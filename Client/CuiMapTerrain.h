#pragma once

#include <algorithm>
#include <array>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <map>
#include <string>

namespace mir3_cui {

enum class GroundCell : unsigned char { Outside, Walkable, Blocked, DoorOpen, DoorClosed };

class MapTerrain {
public:
    static constexpr int ViewColumns = 41;
    static constexpr int ViewRows = 9;
    using View = std::array<GroundCell, ViewColumns * ViewRows>;
    static constexpr int LightColumns = 58, LightRows = 59;
    using LightView = std::array<unsigned, LightColumns * LightRows>;

    struct Graphic {
        int package = 255, frame = 65535, animation = 255;
    };
    struct GraphicsView {
        static constexpr int FirstColumn = -5, LastColumn = 21;
        static constexpr int FirstRow = -4, LastRow = 31;
        std::array<Graphic, 100> tiles{};
        using Objects = std::array<Graphic, 2>;
        std::array<Objects, (LastColumn-FirstColumn)*(LastRow-FirstRow)> objects{};
    };

    bool Open(const std::string& id, const std::filesystem::path& directory = "Map")
    {
        ++revision_;
        file_ = std::ifstream(directory / (id + ".map"), std::ios::binary | std::ios::ate);
        width_ = height_ = 0;
        doors_.clear();
        if (!file_ || file_.tellg() < HeaderSize) return false;
        const auto length = file_.tellg();
        std::array<unsigned char, HeaderSize> header{};
        file_.seekg(0);
        if (!file_.read(reinterpret_cast<char*>(header.data()), header.size())) return false;
        const int width = header[22] | (header[23] << 8);
        const int height = header[24] | (header[25] << 8);
        if (width < 2 || height < 2 || width > 8192 || height > 8192 || width % 2 || height % 2) return false;
        const auto cells = std::int64_t(width) * height;
        const auto start = HeaderSize + (cells / 4) * 3;
        if (length != start + cells * CellSize) return false;
        width_ = width;
        height_ = height;
        start_ = start;
        return true;
    }

    bool Ready() const { return width_ != 0; }
    std::uint64_t Revision() const { return revision_; }
    bool Contains(int x, int y) const { return x >= 0 && y >= 0 && x < width_ && y < height_; }

    bool BaseWalkable(int x,int y)
    {
        std::array<unsigned char,CellSize> cell{};
        return Contains(x,y) && ReadColumn(x,y,1,cell.data()) && (cell[0]&1);
    }

    unsigned DoorAt(int x, int y)
    {
        if (!Contains(x, y)) return 0;
        std::array<unsigned char, CellSize> cell{};
        return ReadColumn(x, y, 1, cell.data()) ? DoorId(cell.data()) : 0;
    }

    View ReadView(int x, int y)
    {
        View view{};
        const int left = x - ViewColumns / 2;
        const int top = y - ViewRows / 2;
        const int first = (std::max)(top, 0);
        const int last = (std::min)(top + ViewRows, height_);
        if (last <= first) return view;

        std::array<unsigned char, ViewRows * CellSize> column{};
        for (int cx = (std::max)(left, 0); cx < (std::min)(left + ViewColumns, width_); ++cx) {
            if (!ReadColumn(cx, first, last - first, column.data())) continue;
            for (int cy = first; cy < last; ++cy) {
                const auto* row = column.data() + (cy - first) * CellSize;
                auto& cell = view[(cy - top) * ViewColumns + cx - left];
                if (DoorId(row)) {
                    const auto override = doors_.find(Index(cx, cy));
                    const bool open = override != doors_.end() ? override->second : (row[11] & 128) != 0;
                    cell = open && (row[0] & 1) ? GroundCell::DoorOpen : GroundCell::DoorClosed;
                } else {
                    cell = (row[0] & 1) ? GroundCell::Walkable : GroundCell::Blocked;
                }
            }
        }
        return view;
    }

    LightView ReadLights(int left, int top)
    {
        LightView view{};
        const int first=(std::max)(0,top), last=(std::min)(height_,top+LightRows);
        if (last<=first) return view;
        std::array<unsigned char,LightRows*CellSize> cells{};
        for (int x=(std::max)(0,left);x<(std::min)(width_,left+LightColumns);++x) {
            if (!ReadColumn(x,first,last-first,cells.data())) continue;
            for (int y=first;y<last;++y) {
                const unsigned packed=Word(cells.data()+(y-first)*CellSize+12);
                if (((packed>>4)&255)<10) view[(x-left)*LightRows+y-top]=packed;
            }
        }
        return view;
    }

    bool UpdateDoor(int x, int y, bool open)
    {
        if (!Contains(x, y)) return false;
        std::array<unsigned char, CellSize> cell{};
        if (!ReadColumn(x, y, 1, cell.data())) return false;
        const auto id = DoorId(cell.data());
        if (!id) return false;
        ++revision_;
        const int first = (std::max)(0, y - 8);
        const int last = (std::min)(height_, y + 10);
        std::array<unsigned char, 18 * CellSize> column{};
        for (int cx = (std::max)(0, x - 8); cx < (std::min)(width_, x + 10); ++cx) {
            if (!ReadColumn(cx, first, last - first, column.data())) continue;
            for (int cy = first; cy < last; ++cy) {
                const auto* row = column.data() + (cy - first) * CellSize;
                if ((row[9] & 127) == id) doors_[Index(cx, cy)] = open;
            }
        }
        return true;
    }

    GraphicsView ReadGraphics(int x, int y)
    {
        GraphicsView view;
        if (!Ready()) return view;
        constexpr int tilePitch=3;
        const int tileY=y/2;
        const int firstTile=(std::max)(0,tileY), lastTile=(std::min)(height_/2,tileY+10);
        std::array<unsigned char,10*tilePitch> tiles{};
        for (int column=0;column<10 && firstTile<lastTile;++column) {
            const int tileX=x/2+column;
            if (tileX<0 || tileX>=width_/2) continue;
            file_.clear();
            file_.seekg(HeaderSize+(std::int64_t(tileX)*(height_/2)+firstTile)*tilePitch);
            if (!file_.read(reinterpret_cast<char*>(tiles.data()),(lastTile-firstTile)*tilePitch)) continue;
            for (int row=firstTile;row<lastTile;++row) {
                const auto* tile=tiles.data()+(row-firstTile)*tilePitch;
                view.tiles[(row-tileY)*10+column]={tile[0],Word(tile+1),255};
            }
        }
        constexpr int columns=GraphicsView::LastColumn-GraphicsView::FirstColumn;
        const int top=y+GraphicsView::FirstRow;
        const int first=(std::max)(0,top), last=(std::min)(height_,y+GraphicsView::LastRow);
        std::array<unsigned char,(GraphicsView::LastRow-GraphicsView::FirstRow)*CellSize> cells{};
        for (int column=0;column<columns && first<last;++column) {
            const int cx=x+GraphicsView::FirstColumn+column;
            if (cx<0 || cx>=width_ || !ReadColumn(cx,first,last-first,cells.data())) continue;
            for (int cy=first;cy<last;++cy) {
                const auto* row=cells.data()+(cy-first)*CellSize;
                auto& objects=view.objects[(cy-top)*columns+column];
                const auto door=doors_.find(Index(cx,cy));
                const bool open=door!=doors_.end() ? door->second : (row[11]&128)!=0;
                for (int layer=0;layer<2;++layer) {
                    int id=Word(row+5+2*layer);
                    if (id!=65535 && (row[9]&127) && open) id+=row[11]&127;
                    objects[layer]={row[4-layer],id,row[1+layer]};
                }
            }
        }
        return view;
    }

private:
    static constexpr int HeaderSize = 28;
    static constexpr int CellSize = 14;
    std::ifstream file_;
    int width_ = 0, height_ = 0;
    std::int64_t start_ = 0;
    std::uint64_t revision_ = 0;
    std::map<int, bool> doors_;

    int Index(int x, int y) const { return x * height_ + y; }
    static unsigned DoorId(const unsigned char* cell) { return cell[9] & 128 ? cell[9] & 127 : 0; }
    static int Word(const unsigned char* bytes) { return bytes[0] | (bytes[1]<<8); }

    bool ReadColumn(int x, int y, int count, unsigned char* data)
    {
        file_.clear();
        file_.seekg(start_ + std::int64_t(Index(x, y)) * CellSize);
        return bool(file_.read(reinterpret_cast<char*>(data), count * CellSize));
    }
};

} // namespace mir3_cui

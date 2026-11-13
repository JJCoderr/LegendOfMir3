#pragma once
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <utility>
#include <vector>

namespace mir3 {
class MapTerrain {
public:
    bool open(const std::filesystem::path& path) {
        std::ifstream file(path,std::ios::binary);
        std::vector<std::uint8_t> data{std::istreambuf_iterator<char>(file),{}};
        if (data.size()<28) return false;
        const int w=data[22]|(data[23]<<8), h=data[24]|(data[25]<<8);
        if (w<2 || h<2 || w>8192 || h>8192 || w%2 || h%2) return false;
        const auto cells=std::size_t(w)*h;
        if (data.size()!=28+cells/4*3+cells*14) return false;
        cell_start=28+cells/4*3;
        tiles=std::move(data);
        width=w;
        height=h;
        return true;
    }

    std::pair<int,int> tile(int x,int y) const {
        if (x<0 || y<0 || x>=width || y>=height) return {255,65535};
        const auto at=28+(std::size_t(x/2)*(height/2)+y/2)*3;
        return {tiles[at],tiles[at+1]|(tiles[at+2]<<8)};
    }

    bool walkable(int x,int y) const {
        return x>=0 && y>=0 && x<width && y<height &&
            (tiles[cell_start+(std::size_t(x)*height+y)*14]&1)!=0;
    }

    struct Object { int package=255, frame=65535, animation=255; };
    Object object(int x,int y,int layer) const {
        if (x<0 || y<0 || x>=width || y>=height || layer<0 || layer>1) return {};
        const auto at=cell_start+(std::size_t(x)*height+y)*14;
        const auto frame=at+5+2*layer;
        return {tiles[at+4-layer],tiles[frame]|(tiles[frame+1]<<8),tiles[at+1+layer]};
    }

private:
    std::size_t cell_start=0;
    int width=0,height=0;
    std::vector<std::uint8_t> tiles;
};
}

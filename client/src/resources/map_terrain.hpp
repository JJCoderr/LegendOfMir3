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
        data.resize(28+cells/4*3);
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

private:
    int width=0,height=0;
    std::vector<std::uint8_t> tiles;
};
}

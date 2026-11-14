#pragma once
#include <cstdint>
#include <algorithm>
#include <array>
#include <cstdlib>
#include <deque>
#include <queue>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <utility>
#include <vector>

namespace mir3 {
using MapCell=std::array<int,2>;
inline constexpr std::array<MapCell,8> map_directions{{
    {0,-1},{1,-1},{1,0},{1,1},{0,1},{-1,1},{-1,0},{-1,-1}
}};

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

    bool contains(int x,int y) const { return x>=0 && y>=0 && x<width && y<height; }
    bool walkable(int x,int y) const {
        return x>=0 && y>=0 && x<width && y<height &&
            (tiles[cell_start+(std::size_t(x)*height+y)*14]&1)!=0;
    }

    template<class Occupied>
    std::deque<MapCell> route(MapCell start,MapCell target,Occupied occupied) const {
        const auto inside=[&](MapCell cell) {
            return cell[0]>=0 && cell[1]>=0 && cell[0]<width && cell[1]<height;
        };
        if (!inside(start) || !inside(target) || start==target) return {};
        const auto traversable=[&](MapCell cell) { return walkable(cell[0],cell[1]) && !occupied(cell); };
        if (!traversable(target)) {
            for (const auto [dx,dy]:map_directions) {
                const MapCell neighbor{target[0]+dx,target[1]+dy};
                if (traversable(neighbor)) { target=neighbor; break; }
            }
        }
        if (!traversable(target) || start==target) return {};
        const auto distance=[&](MapCell cell) {
            const int dx=std::abs(target[0]-cell[0]),dy=std::abs(target[1]-cell[1]);
            return 10*std::max(dx,dy)+4*std::min(dx,dy);
        };
        const int margin=distance(start)>320?std::max(width,height):16;
        const int left=std::max(0,std::min(start[0],target[0])-margin);
        const int top=std::max(0,std::min(start[1],target[1])-margin);
        const int right=std::min(width-1,std::max(start[0],target[0])+margin);
        const int bottom=std::min(height-1,std::max(start[1],target[1])+margin);
        const int stride=right-left+1;
        const auto index=[&](MapCell cell) { return (cell[1]-top)*stride+cell[0]-left; };
        const auto point=[&](int id) { return MapCell{left+id%stride,top+id/stride}; };
        struct Visit { int priority,cost,id; };
        const auto later=[](Visit a,Visit b) { return a.priority>b.priority; };
        std::priority_queue<Visit,std::vector<Visit>,decltype(later)> frontier(later);
        std::vector<int> costs(stride*(bottom-top+1),-1), parents(costs.size(),-1);
        const int first=index(start),last=index(target);
        costs[first]=0;
        frontier.push({distance(start),0,first});
        while (!frontier.empty()) {
            const auto visit=frontier.top();
            frontier.pop();
            if (visit.cost!=costs[visit.id]) continue;
            if (visit.id==last) {
                std::deque<MapCell> path;
                for (int id=last;id!=first;id=parents[id]) path.push_front(point(id));
                return path;
            }
            const auto [x,y]=point(visit.id);
            for (const auto [dx,dy]:map_directions) {
                const MapCell next{x+dx,y+dy};
                if (next[0]<left || next[0]>right || next[1]<top || next[1]>bottom || !traversable(next)) continue;
                const int id=index(next),cost=visit.cost+(dx && dy?14:10);
                if (costs[id]>=0 && costs[id]<=cost) continue;
                costs[id]=cost;
                parents[id]=visit.id;
                frontier.push({cost+distance(next),cost,id});
            }
        }
        return {};
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

#pragma once
#include "inventory_grid.hpp"
namespace mir3 {
// 41C7E0: 每方12条记录，5列40行占位；当前数组以锚点格索引保存记录。
struct TradeGrid {
    using Items=std::array<BagItem,200>;
    static int count(const Items& items) { int n=0;for(const auto& item:items) if(item) ++n;return n; }
    static bool fits(const Items& items,int slot,const BagItem& item) {
        if(slot<0 || slot>=200 || item.width<1 || item.height<1 || slot%5+item.width>5 || slot/5+item.height>40) return false;
        const int x=slot%5,y=slot/5;
        for(int i=0;i<200;++i) if(items[i] && x<i%5+items[i].width && x+item.width>i%5 && y<i/5+items[i].height && y+item.height>i/5) return false;
        return true;
    }
    static int insert(Items& items,const BagItem& item,int preferred=-1) {
        if(!item || count(items)>=12) return -1;
        if(fits(items,preferred,item)) {items[preferred]=item;return preferred;}
        for(int i=0;i<200;++i) if(fits(items,i,item)) {items[i]=item;return i;}
        return -1;
    }
};
}

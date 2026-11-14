#pragma once
#include "quick_items.hpp"
#include <algorithm>
#include <span>
#include <vector>
namespace mir3 {
// 43EC60/43E6F0：Data/<角色名>.itms，仅用持久化身份与格位匹配新下发物品。
struct ItemLayout {
    static constexpr int bag_stride=3120,quick_stride=3112;
    static constexpr int cells_offset=46*bag_stride,quick_offset=cells_offset+1200;
    static constexpr int file_size=quick_offset+6*quick_stride;
    static unsigned word(std::span<const std::uint8_t> data,int at) {return data[at]|(unsigned(data[at+1])<<8);}
    static unsigned dword(std::span<const std::uint8_t> data,int at) {return word(data,at)|(word(data,at+2)<<16);}
    static void put_word(std::vector<std::uint8_t>& data,int at,unsigned value) {data[at]=std::uint8_t(value);data[at+1]=std::uint8_t(value>>8);}
    static bool same_name(std::span<const std::uint8_t> saved,int offset,const BagItem& item) {
        for(int i=0;i<30;++i) {
            if(saved[offset+i]!=item.server_record[i]) return false;
            if(!saved[offset+i]) return true;
        }
        return true;
    }
    static std::vector<std::uint8_t> encode(const InventoryGrid& bag,const QuickItems& quick) {
        std::vector<std::uint8_t> data(file_size);
        for(int i=0;i<46;++i) if(bag.items[i]) {
            const int at=i*bag_stride;data[at]=1;
            put_word(data,at+4,bag.items[i].width);put_word(data,at+8,bag.items[i].height);
            // 原文件其它字段包含旧纹理/绘制缓存，恢复时均被当前服务器记录替换。
            std::copy(bag.items[i].server_record.begin(),bag.items[i].server_record.end(),data.begin()+at+16);
        }
        for(int i=0;i<600;++i) put_word(data,cells_offset+i*2,std::uint16_t(bag.cells[i]));
        for(int i=0;i<6;++i) if(quick.slots[i]) {
            const int at=quick_offset+i*quick_stride;data[at]=1;
            std::copy(quick.slots[i].server_record.begin(),quick.slots[i].server_record.end(),data.begin()+at+8);
        }
        return data;
    }
    static bool restore(std::span<const std::uint8_t> data,InventoryGrid& bag,QuickItems& quick) {
        if(data.size()!=file_size) return false;
        for(int cell=0;cell<600;++cell) {
            const auto code=word(data,cells_offset+cell*2);
            if(code!=65535 && !(code<46 || (code>=1000 && code<1046))) return false;
        }
        auto next=bag;auto references=quick;
        // 同名快捷项取服务器列表中最后一个匹配项，不恢复磁盘中的数量或编号。
        for(int slot=0;slot<6;++slot) {
            const int at=quick_offset+slot*quick_stride;
            if(!dword(data,at)) continue;
            for(const auto& item:bag.items) if(item && same_name(data,at+8,item)) references.slots[slot]=item;
        }
        for(int i=0;i<46;++i) next.take(i);
        std::array<bool,46> consumed{};
        for(int saved=0;saved<46;++saved) {
            const int at=saved*bag_stride;
            if(!dword(data,at)) continue;
            int current=-1;
            for(int i=0;i<46;++i) {
                const auto& item=bag.items[i];
                if(item && !consumed[i] && same_name(data,at+16,item) &&
                   dword(data,at+78)==item.make_index && word(data,at+82)==word(item.server_record,66) &&
                   word(data,at+84)==word(item.server_record,68)) {current=i;break;}
            }
            if(current<0) continue;
            const auto& item=bag.items[current];
            int anchor=-1,cell_count=0;
            for(int cell=0;cell<600;++cell) {
                const auto code=word(data,cells_offset+cell*2);
                if(code==unsigned(1000+saved)) {if(anchor>=0) {anchor=-2;break;} anchor=cell;}
                if(code!=65535 && code%1000==unsigned(saved)) ++cell_count;
            }
            if(anchor<0 || cell_count!=item.width*item.height || !next.fits(anchor%6,anchor/6,item)) continue;
            bool valid=true;
            for(int y=0;y<item.height;++y) for(int x=0;x<item.width;++x) {
                const int cell=anchor+y*6+x;
                if(word(data,cells_offset+cell*2)!=unsigned(saved+(x==0 && y==0?1000:0))) valid=false;
            }
            if(!valid) continue;
            next.items[saved]=item;next.anchors[saved]=anchor;consumed[current]=true;
            for(int y=0;y<item.height;++y) for(int x=0;x<item.width;++x) next.cells[anchor+y*6+x]=short(saved);
            next.cells[anchor]=short(1000+saved);
        }
        // 43E6F0最后将未匹配物品依原列表顺序放回首个可用格。
        for(int i=0;i<46;++i) if(bag.items[i] && !consumed[i] && !next.insert(bag.items[i])) return false;
        bag=std::move(next);quick=std::move(references);return true;
    }
};
}

// 测试内容：原版布局文件字节结构、身份与耐久匹配、数量权威、同名快捷末项、损坏文件原子拒绝、多格布局和原始文件只读兼容。
#pragma once
#include "inventory/item_layout.hpp"
#include "protocol/inventory_wire.hpp"
#include <fstream>
#include <filesystem>
#include <stdexcept>
inline void layout_require(bool condition,const char* message) {if(!condition) throw std::runtime_error(message);}
inline mir3::BagItem layout_item(unsigned id,const std::string& name,int count=3) {
    mir3::BagItem item{-1,301,count,1,1,0,id,name,3};
    std::copy(name.begin(),name.end(),item.server_record.begin());
    for(int i=0;i<4;++i) item.server_record[62+i]=std::uint8_t(id>>(8*i));
    item.server_record[66]=7;item.server_record[68]=10;item.set_count(count);return item;
}
inline void item_layout_checks() {
    using namespace mir3;
    InventoryGrid saved;QuickItems shortcuts;
    const auto first=layout_item(100,"Potion");
    saved.insert(first,20,17+2*38+10,60+3*38+10);shortcuts.slots[4]=first;
    auto wide=layout_item(200,"Sword");wide.width=2;wide.height=3;
    saved.insert(wide);const auto bytes=ItemLayout::encode(saved,shortcuts);
    layout_require(bytes.size()==163392 && ItemLayout::dword(bytes,78)==100 && bytes[144720+4*3112+8]=='P',"legacy offsets");
    InventoryGrid incoming;QuickItems restored;
    incoming.insert(wide);incoming.insert(layout_item(100,"Potion",9));incoming.insert(layout_item(300,"Potion",11));
    layout_require(ItemLayout::restore(bytes,incoming,restored),"restore layout");
    layout_require(incoming.items[0].make_index==100 && incoming.anchors[0]==20 && incoming.items[0].count==9,"identity keeps anchor but uses new count");
    layout_require(restored.slots[4].make_index==300 && restored.slots[4].count==11,"shortcut last name match ignores old identity");
    layout_require(incoming.anchors[1]==saved.anchors[1] && incoming.items[1].width==2,"multi-cell rectangle restored");
    auto broken=bytes;broken.resize(100);
    layout_require(!ItemLayout::restore(broken,incoming,restored) && incoming.items[0].count==9,"short file leaves state unchanged");
    broken=bytes;ItemLayout::put_word(broken,143520,999);
    layout_require(!ItemLayout::restore(broken,incoming,restored) && restored.slots[4].make_index==300,"bad occupancy rejected atomically");
    InventoryGrid worn;QuickItems worn_quick;
    auto changed=first;changed.server_record[66]=8;worn.insert(changed);
    layout_require(ItemLayout::restore(bytes,worn,worn_quick) && worn.anchors[0]==0 && worn_quick.slots[4].make_index==100,"durability mismatch falls back but shortcut matches name");
    InventoryGrid absent;QuickItems absent_quick;absent.insert(layout_item(999,"New"));
    layout_require(ItemLayout::restore(bytes,absent,absent_quick) && absent.items[0].make_index==999 && !absent_quick.slots[4],"stale saved items never resurrect");
}
inline int item_layout_original_checks(const std::filesystem::path& directory) {
    using namespace mir3;int count=0;
    for(const auto& entry:std::filesystem::directory_iterator(directory)) {
        if(entry.path().extension()!=".itms") continue;
        std::ifstream input(entry.path(),std::ios::binary);
        const std::vector<std::uint8_t> bytes{std::istreambuf_iterator<char>(input),{}};
        layout_require(bytes.size()==163392,"original file size");
        InventoryGrid bag;QuickItems quick;
        for(int i=45;i>=0;--i) if(ItemLayout::dword(bytes,i*3120)) {
            InventoryRecord record;std::copy_n(bytes.begin()+i*3120+16,100,record.raw.begin());
            BagItem item{-1,record.icon(),record.count(),int(ItemLayout::dword(bytes,i*3120+4)),int(ItemLayout::dword(bytes,i*3120+8)),record.mode(),record.identity(),record.name(),record.stack_mode(),record.raw};
            layout_require(bag.insert(item),"load original authoritative fixture");
        }
        layout_require(ItemLayout::restore(bytes,bag,quick),"read original layout");
        for(int i=0;i<600;++i) {
            const auto old=ItemLayout::word(bytes,143520+2*i);
            layout_require(std::uint16_t(bag.cells[i])==old,"original multi-cell occupancy restored");
        }
        ++count;
    }
    layout_require(count>0,"original fixture required");return count;
}

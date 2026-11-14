// 测试内容：快捷引用绑定与互换、同名拒绝、背包回放防复制、两档冷却边界与回绕、成功数量同步及失败恢复。
#pragma once
#include "inventory/quick_items.hpp"
#include <stdexcept>
inline void quick_items_checks() {
    using namespace mir3;
    auto require=[](bool value,const char* message){if(!value) throw std::runtime_error(message);};
    InventoryGrid bag;QuickItems quick;
    BagItem potion{-1,20,3,1,1,0,100,"Potion",3};
    bag.held=potion;
    require(quick.release(0,bag,"")==1068 && !bag.held && bag.items[0].count==3,"bind returns owned item");
    require(quick.release(0,bag,"")==0 && bag.held_quick_slot==0 && bag.items[0].count==3,"pickup reference only");
    bag.release(36,79,0,0);
    require(!bag.held && bag.held_quick_slot<0 && bag.items[0].count==3 && !bag.items[1],"occupied bag drop must not duplicate or swap");
    bag.held=bag.take(0);quick.release(0,bag,"");
    require(!quick.activate(0,bag,1000,"").request,"fast cooldown inclusive boundary");
    require(quick.activate(0,bag,1001,"").request,"fast cooldown after boundary");
    require(!quick.activate(1,bag,5000,"").request,"pending prevents another use");
    require(quick.reply(protocol::SM_EAT_OK,2,bag) && quick.slots[0].count==2 && bag.items[0].count==2,"authoritative shared count");
    require(bag.items[0].server_record[71]==2 && quick.slots[0].server_record[71]==2,"raw count synchronized");
    require(!quick.reply(protocol::SM_EAT_OK,1,bag),"duplicate reply rejected");
    require(quick.activate(0,bag,3002,"").request,"activate again");
    require(quick.reply(protocol::SM_EAT_FAIL,0,bag) && quick.slots[0].count==2 && !bag.items[1],"failure restores reference only");
    bag.held=potion;bag.held.make_index=200;
    require(quick.release(1,bag,"")==1067 && bag.held.make_index==200,"duplicate names across slots");
    require(quick.release(0,bag,"")==1069 && bag.held.make_index==200,"same-name target unchanged");
    bag.held={};quick.slots[0].std_mode=3;bag.last_use=100;
    require(!quick.activate(0,bag,2100,"").request,"normal cooldown inclusive boundary");
    require(quick.activate(0,bag,2101,"").request,"normal cooldown after boundary");
    quick.reply(protocol::SM_EAT_FAIL,0,bag);
    bag.last_use=0xfffffff0u;
    require(quick.activate(0,bag,2000,"").request,"unsigned timer wrap");
    quick.reply(protocol::SM_EAT_OK,1,bag);
    require(quick.activate(0,bag,5000,"").request,"last item activation");
    require(quick.reply(protocol::SM_EAT_OK,77,bag) && !quick.slots[0] && !bag.items[0],"last item ignores contradictory remaining");
    require(!quick.activate(5,bag,6000,"").request,"empty slot no request");
    bag.held=potion;quick.release(0,bag,"");quick.release(0,bag,"");
    require(quick.release(2,bag,"")==0 && quick.slots[2] && !bag.items[1],"shortcut movement no duplicate");
    bag.held=potion;bag.held.name="Other";bag.held.make_index=300;
    require(quick.release(2,bag,"")==0 && bag.held.name=="Potion" && quick.slots[2].name=="Other" && bag.items[1].name=="Other","occupied slot swap returns incoming to bag");
    require(!quick.mouse_activate(2,bag,5500,"").request,"mouse cooldown uses held item");
    require(quick.mouse_activate(2,bag,6001,"").request,"mouse fast use");
}

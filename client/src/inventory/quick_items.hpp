#pragma once
#include "inventory_grid.hpp"
#include <string_view>
namespace mir3 {
// 419D50/419F80：六个独立快捷引用，不拥有背包物品。
struct QuickItems {
    std::array<BagItem,6> slots{};
    struct Activation { bool request=false;int binding_notice=0,notice=0; };
    int find(std::string_view name) const {
        for(int i=0;i<6;++i) if(slots[i] && slots[i].name==name) return i;
        return -1;
    }
    static bool accepts(const InventoryGrid& bag,std::string_view gold_name) {
        return bag.held && !bag.pending_use && bag.held_equipment_slot<0 &&
            bag.held.name!=gold_name && (bag.held.std_mode==0 || bag.held.std_mode==3);
    }
    // 返回0表示操作完成，正数为原资源提示，-1表示没有操作。
    int release(int slot,InventoryGrid& bag,std::string_view gold_name) {
        if(slot<0 || slot>=6 || bag.pending_use) return -1;
        if(!bag.held) {
            if(!slots[slot]) return -1;
            bag.held=slots[slot];slots[slot]={};bag.held_quick_slot=slot;
            bag.held_slot=slot;bag.held_equipment_slot=-1;return 0;
        }
        if(!accepts(bag,gold_name)) return -1;
        const bool from_quick=bag.held_quick_slot>=0;
        if(slots[slot]) {
            if(slots[slot].name==bag.held.name) return 1069;
            auto next=bag;
            if(!from_quick && !next.insert(next.held)) return -1;
            auto previous=slots[slot];slots[slot]=bag.held;
            bag=std::move(next);bag.held=std::move(previous);
            bag.held_quick_slot=slot;bag.held_slot=slot;return 0;
        }
        if(find(bag.held.name)>=0) return 1067;
        auto next=bag;
        // 41A8A0将背包来源的物品放回，再保留快捷副本。
        if(!from_quick && !next.insert(next.held,slot)) return -1;
        slots[slot]=bag.held;bag=std::move(next);bag.held={};bag.held_quick_slot=-1;bag.held_slot=-1;
        return from_quick?0:1068;
    }
    Activation activate(int slot,InventoryGrid& bag,std::uint32_t now,std::string_view gold_name) {
        Activation result;
        if(slot<0 || slot>=6) return result;
        const auto& target=slots[slot];
        const auto elapsed=std::uint32_t(now-bag.last_use);
        const bool medicine=!target || target.std_mode==0 || target.std_mode==3;
        const bool fast=target.std_mode==0 && (target.icon==20 || target.icon==21 || target.icon==70);
        if((medicine && elapsed<=2000 && !(fast && elapsed>1000)) || bag.pending_use || bag.held_equipment_slot>=0) {
            result.notice=target?1051:1066;return result;
        }
        // 41A3D0按数字键移动到空槽时先删除原有同名快捷引用。
        if(!target && accepts(bag,gold_name)) {
            const int duplicate=find(bag.held.name);
            if(duplicate>=0) slots[duplicate]={};
        }
        const int changed=release(slot,bag,gold_name);
        if(changed>0) result.binding_notice=changed;
        if(!bag.held || bag.pending_use || bag.held_equipment_slot>=0 || bag.held.name==gold_name) {
            result.notice=slots[slot]?1051:1066;return result;
        }
        bag.pending_use=true;
        if(bag.held.std_mode==0 || bag.held.std_mode==3) bag.last_use=now;
        result.request=true;return result;
    }
    Activation mouse_activate(int slot,InventoryGrid& bag,std::uint32_t now,std::string_view gold_name) {
        Activation result;
        if(slot<0 || slot>=6) return result;
        const auto elapsed=std::uint32_t(now-bag.last_use);
        const bool fast=bag.held.std_mode==0 && (bag.held.icon==20 || bag.held.icon==21 || bag.held.icon==70);
        if(accepts(bag,gold_name) && (elapsed>2000 || (fast && elapsed>1000))) {
            bag.pending_use=true;bag.last_use=now;result.request=true;
        } else result.notice=bag.held?1051:1066;
        return result;
    }
    bool reply(int opcode,int remaining,InventoryGrid& bag) {
        if(!bag.pending_use || !bag.held || bag.held_quick_slot<0 || remaining<0 || remaining>65535 ||
            (opcode!=protocol::SM_EAT_OK && opcode!=protocol::SM_EAT_FAIL)) return false;
        auto next=bag;auto references=slots;
        const auto used=next.held;
        bool restore=opcode==protocol::SM_EAT_FAIL;
        if(opcode==protocol::SM_EAT_OK && used.stack_mode==3) {
            if(used.count<=1) {
                for(int i=0;i<InventoryGrid::capacity;++i) if(next.items[i] && next.items[i].make_index==used.make_index && next.items[i].name==used.name) {next.take(i);break;}
                for(auto& item:references) if(item && item.name==used.name) {item={};break;}
            } else {
                for(auto& item:next.items) if(item && item.make_index==used.make_index) item.set_count(remaining);
                next.held.set_count(remaining);restore=true;
            }
        }
        if(restore) {
            int destination=next.held_quick_slot;
            if(references[destination]) {
                destination=-1;for(int i=0;i<6;++i) if(!references[i]) {destination=i;break;}
                if(destination<0) return false;
            }
            references[destination]=next.held;
        }
        next.held={};next.held_quick_slot=-1;next.held_slot=-1;next.pending_use=false;
        bag=std::move(next);slots=std::move(references);return true;
    }
    void update_count(std::string_view name,int count) { const int slot=find(name);if(slot>=0) slots[slot].set_count(count); }
    void remove(std::string_view name) {const int slot=find(name);if(slot>=0) slots[slot]={};}
};
}

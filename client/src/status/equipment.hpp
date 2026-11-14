#pragma once
#include "protocol/message_ids.hpp"
#include "inventory/inventory_grid.hpp"

namespace mir3 {
struct EquipmentRequest {
    int identifier = 0, slot = -1;
    std::uint32_t make_index = 0;
    std::string name;

};
struct EquipmentState {
    using Rect = std::array<int,4>;
    // 0x4710F0 updates these on every draw. Constructor values differ for10/11.
    static constexpr std::array<Rect,12> bounds{{
        {95,114,50,90},{40,70,50,95},{90,265,38,38},{170,88,38,38},
        {95,70,38,34},{12,155,38,38},{170,155,38,38},{12,195,38,38},
        {170,195,38,38},{130,265,38,38},{12,255,38,38},{170,265,38,38}}};
    static int category(int mode) { // 0x4420F0 payload byte0
        switch(mode) {
        case 5: case 6: case 54: case 55: case 56:return 1;
        case 10: case 11:return 0;
        case 15:return 4;
        case 19: case 20: case 21: case 57:return 3;
        case 22: case 23:return 7;
        case 24: case 26:return 5;
        case 25:return 9;
        case 30:return 2;
        case 53:return 10;
        case 60:return 11;
        default:return -1;
        }
    }
    static bool accepts(const BagItem& item,int slot) { // 0x4762C0
        const int type=category(item.std_mode);
        if (!item || slot<0 || slot>=12 || type<0) return false;
        if (type==5 || type==6 || type==9)
            return item.std_mode==25 ? slot==5 || slot==9 : slot==5 || slot==6;
        if (type==7 || type==8) return slot==7 || slot==8;
        return type==slot;
    }
    static bool uses_paperdoll(int slot) { return slot==0 || slot==1 || slot==4; }
    static std::array<int,2> draw_position(int slot,int width,int height,int offset_x,int offset_y) {
        if (slot<0 || slot>=12) return {};
        if (uses_paperdoll(slot)) return {82+offset_x,199+offset_y}; // 0x442200 a4=1
        const auto r=bounds[slot];return {r[0]+19-width/2,r[1]+19-height/2};
    }
    static int preview_frame(int mode,int slot) { // 0x4710F0 switch, not acceptance rules
        switch (mode) {
        case 19: case 20: case 21: case 57: return slot==3 ? 1235 : -1;
        case 22: case 23: return slot==7 || slot==8 ? 1235 : -1;
        case 24: case 26: return slot==5 || slot==6 ? 1235 : -1;
        case 25: return slot==9 ? 1235 : -1; // accepted slot5 is NOT highlighted
        case 30: return slot==2 ? 1235 : -1;
        case 53: return slot==10 ? 1236 : -1;
        case 60: return slot==11 ? 1235 : -1;
        default:return -1;
        }
    }
    static std::array<int,3> preview(int mode,int slot) {
        const int frame=preview_frame(mode,slot);
        if (frame<0) return {-1,0,0};
        const auto r=bounds[slot];
        return {frame,r[0]-2,r[1]-(mode==53 ? 21 : 2)};
    }
    static int hit(int x,int y,int ox,int oy) { // 0x476250 first matching RECT
        for (int i=0;i<12;++i) {
            const auto r=bounds[i];
            if (x>=ox+r[0] && y>=oy+r[1] && x<ox+r[0]+r[2] && y<oy+r[1]+r[3]) return i;
        }
        return -1;
    }
    std::array<BagItem,12> slots{};
    BagItem incoming;
    EquipmentRequest request;
    bool pending=false;
    int last_reply=0;
    std::uint32_t last_release=0; // status+0xA360, unsigned GetTickCount subtraction
    bool quick_request_ready=false;
    bool use_rejected_notice=false;
    bool release_action(int slot,InventoryGrid& bag,std::uint32_t now) {
        use_rejected_notice=false;
        const auto elapsed=std::uint32_t(now-last_release);
        last_release=now; // 0x476793 / 0x4767D5 both paths update
        if (elapsed<300 && bag.held && !pending && !bag.pending_use) {
            const int mode=bag.held.std_mode;
            if (mode==5 || mode==6 || mode==10 || mode==11 || mode==15 || mode==53) {
                use_rejected_notice=true; // CMList2413, then ordinary release
            } else {
                // 0x476980 requires equipment source; no ordinary fallthrough.
                if (bag.held_equipment_slot<0 || !accepts(bag.held,slot)) return false;
                int destination=slot;
                if (slots[destination]) { // 0x476440 first free fallback
                    destination=-1;
                    for (int i=0;i<12;++i) if (!slots[i]) { destination=i;break; }
                    if (destination<0) return false; // local no-loss protection
                }
                slots[destination]=bag.held;
                bag.held={};bag.held_equipment_slot=-1;
                request={mir3::protocol::CM_EQUIPMENT_QUICK_ACTION,slot,0,{}};last_reply=0;
                quick_request_ready=true; // not a pending1003/1004 transaction
                return true;
            }
        }
        return release(slot,bag);
    }
    bool release(int slot,InventoryGrid& bag) {
        if (slot<0 || slot>=12 || pending || bag.pending_use) return false;
        if (!bag.held) { // 0x476570: remove locally, mark equipment source
            if (!slots[slot]) return false;
            bag.held=slots[slot]; slots[slot]={}; bag.held_equipment_slot=slot; bag.held_slot=slot;
            return true;
        }
        if (!accepts(bag.held,slot)) return false;
        if (bag.held_equipment_slot>=0) {
            if (slots[slot]) return false;
            slots[bag.held_equipment_slot]=bag.held; bag.held={}; bag.held_equipment_slot=-1;
            return true; // 0x476740 restores original equipment slot
        }
        incoming=bag.held; // 0x476690 pending record at panel+38272
        bag.held=slots[slot]; slots[slot]={};
        bag.held_equipment_slot=bag.held ? slot : -1;
        request={mir3::protocol::CM_TAKEONITEM,slot,incoming.make_index,incoming.name}; pending=true;last_reply=0;
        return true;
    }
    bool takeoff(InventoryGrid& bag) { // bag0x441310 -> 0x47BE40(1004)
        if (pending || !bag.held || bag.held_equipment_slot<0) return false;
        request={mir3::protocol::CM_TAKEOFFITEM,bag.held_equipment_slot,bag.held.make_index,bag.held.name};
        pending=true;last_reply=0;return true;
    }
    void restore_slot(int target,const BagItem& item) {
        if (!slots[target]) { slots[target]=item;return; }
        for (auto& slot:slots) if (!slot) { slot=item;return; }
    }
    bool reply(int id,InventoryGrid& bag) {
        if (!pending) return false;
        if (request.identifier==mir3::protocol::CM_TAKEONITEM) {
            if (id!=mir3::protocol::SM_TAKEON_OK && id!=mir3::protocol::SM_TAKEON_FAIL) return false;
            BagItem returned;
            if (id==mir3::protocol::SM_TAKEON_OK) {
                restore_slot(request.slot,incoming); // 0x4766F0；被替换物品由独立200回包返还
            } else {
                returned=incoming; // 616 auto-placement, not source slot
                if (bag.held) restore_slot(request.slot,bag.held);
            }
            incoming={};bag.held={};
            // Local capacity protection: never silently discard a returned item.
            if (returned && !bag.insert(returned)) bag.held=returned;
        } else {
            if (id!=mir3::protocol::SM_TAKEOFF_OK && id!=mir3::protocol::SM_TAKEOFF_FAIL) return false;
            if (id==mir3::protocol::SM_TAKEOFF_OK) {
                bag.held={}; // 619只清除持有物品；背包增加等待独立200消息
            } else {
                restore_slot(request.slot,bag.held); // 620 -> 0x476630
                bag.held={};
            }
        }
        bag.held_equipment_slot=-1;pending=false;last_reply=id;return true;
    }
};
}

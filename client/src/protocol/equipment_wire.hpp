#pragma once
#include "inventory_wire.hpp"
#include <charconv>
namespace mir3 {
struct EquipmentWire {
    struct Entry { int slot;InventoryRecord item; };
    std::vector<Entry> records;
    static std::optional<EquipmentWire> parse(const ReceiveHeader& header,std::span<const std::uint8_t> body) {
        if (header.opcode!=protocol::SM_SENDUSEITEMS) return {};
        EquipmentWire result;
        while (!body.empty() && result.records.size()<12) {
            const auto slash=std::find(body.begin(),body.end(),'/');
            if (slash==body.end() || slash==body.begin()) return {};
            const auto first=reinterpret_cast<const char*>(body.data());
            const auto last=first+(slash-body.begin());
            int slot=-1;const auto parsed=std::from_chars(first,last,slot);
            if (parsed.ec!=std::errc{} || parsed.ptr!=last || slot<0 || slot>=12) return {};
            body=body.subspan(slash-body.begin()+1);
            const auto end=std::find(body.begin(),body.end(),'/');
            if (end==body.end()) return {};
            const auto decoded=decode_six_bit(body.data(),end-body.begin());
            if (!decoded || decoded->size()!=100) return {};
            InventoryRecord item;std::copy(decoded->begin(),decoded->end(),item.raw.begin());
            result.records.push_back({slot,item});
            body=body.subspan(end-body.begin()+1);
        }
        return result;
    }
};
struct EquipmentDurability {
    int slot;
    std::uint16_t current,maximum;
    std::int32_t recognition;
    static std::optional<EquipmentDurability> parse(const ReceiveHeader& header) {
        if (header.opcode!=protocol::SM_DURACHANGE) return {};
        return EquipmentDurability{header.parameters[0],std::uint16_t(header.recognition),header.parameters[1],std::int32_t(header.recognition)};
    }
    bool warns(int mode) const {
        // 4017A0: 完整有符号recognition除以单位，加0.5后截断；不是存储的WORD耐久。
        const int unit=(mode==54 || mode==55)?100:1000;
        return int(double(recognition)/unit+0.5)<=0;
    }
};
}

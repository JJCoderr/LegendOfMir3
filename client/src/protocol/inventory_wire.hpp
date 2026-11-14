#pragma once
#include "receive_message.hpp"
#include "message_ids.hpp"
#include <algorithm>
#include <vector>
namespace mir3 {
// 1.45 CLIENTITEMRCD为100字节；开源早期版本布局不同，按4420F0/440420恢复。
struct InventoryRecord {
    std::array<std::uint8_t,100> raw{};
    std::uint16_t word(int offset) const { return raw[offset]|(std::uint16_t(raw[offset+1])<<8); }
    std::uint32_t identity() const { return word(62)|(std::uint32_t(word(64))<<16); }
    std::string name() const {
        const auto end=std::find(raw.begin(),raw.begin()+30,0);
        return {reinterpret_cast<const char*>(raw.data()),std::size_t(end-raw.begin())};
    }
    int icon() const { return word(36); }
    int count() const { return word(71); }
    int mode() const { return raw[30]; }
    int stack_mode() const { return raw[61]; }
};
struct InventoryWire {
    std::vector<InventoryRecord> records;
    static std::optional<InventoryWire> parse(const ReceiveHeader& header,std::span<const std::uint8_t> body) {
        using namespace protocol;
        const bool full=header.opcode==SM_BAGITEMS;
        if (!full && header.opcode!=SM_ADDITEM && header.opcode!=SM_DELITEM && header.opcode!=SM_UPDATEITEM) return {};
        const int count=full?header.parameters[2]:1;
        if (count>46) return {};
        InventoryWire result;
        if (full && !count && body.empty()) return result;
        for (int i=0;i<std::max(1,count);++i) {
            std::size_t length=body.size();
            if (full) {
                length=0;while(length<body.size() && body[length]!='/') ++length;
                if (length==body.size()) return {};
            }
            const auto decoded=decode_six_bit(body.data(),length);
            if (!decoded || decoded->size()!=100) return {};
            InventoryRecord item;std::copy(decoded->begin(),decoded->end(),item.raw.begin());
            if (item.name().empty()) return {};
            result.records.push_back(item);
            if (full) body=body.subspan(length+1);
        }
        return result;
    }
};
}

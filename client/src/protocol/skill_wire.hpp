#pragma once
#include "receive_message.hpp"
#include "message_ids.hpp"
#include <vector>
namespace mir3 {
// 44F4F0/44F730：列表每条独立编码并以斜杠终结，新增只有一条编码记录。
struct SkillWire {
    static constexpr int record_size=74;
    std::vector<std::uint8_t> records;
    int count=0;
    static std::optional<SkillWire> parse(const ReceiveHeader& header,std::span<const std::uint8_t> body) {
        const bool full=header.opcode==protocol::SM_SENDMYMAGIC;
        if (!full && header.opcode!=protocol::SM_ADDMAGIC) return {};
        SkillWire result;
        result.count=full?(header.parameters[2]&255):1;
        for (int i=0;i<result.count;++i) {
            auto length=body.size();
            if (full) {
                length=0;
                while (length<body.size() && body[length]!='/') ++length;
                if (length==body.size()) return {};
            }
            const auto record=decode_six_bit(body.data(),length);
            if (!record || record->size()!=record_size) return {};
            result.records.insert(result.records.end(),record->begin(),record->end());
            if (full) body=body.subspan(length+1);
        }
        return result;
    }
};
}

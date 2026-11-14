#pragma once
#include "default_message.hpp"
#include "message_ids.hpp"
#include <bit>
#include <cstdint>
#include <string>

namespace mir3 {
// 47CAB0：技能编号和目标编号分开传输，正文保留有符号 DWORD 时间文本。
struct SpellRequest {
    std::uint16_t skill=0;
    std::uint16_t x=0,y=0;
    std::uint32_t target=0,tick=0;

    protocol::DefaultMessage header() const {
        return {std::uint32_t(x)|(std::uint32_t(y)<<16),protocol::CM_SPELL,
            {std::uint16_t(target),skill,std::uint16_t(target>>16)}};
    }
    std::string body() const {
        return std::to_string(std::bit_cast<std::int32_t>(tick));
    }
};
}

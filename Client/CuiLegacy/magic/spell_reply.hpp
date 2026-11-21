#pragma once

#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace mir3 {
struct SpellReply {
    std::uint16_t skill = 0, x = 0, y = 0;
    bool multiple = false;
    std::vector<std::uint32_t> targets;

    static std::optional<SpellReply> parse(std::uint16_t x,std::uint16_t y,std::uint16_t series,
        bool multiple,std::string_view body)
    {
        SpellReply reply{std::uint16_t(series&255),x,y,multiple,{}};
        if (!multiple) {
            if (body.size()!=4) return {};
            std::uint32_t target=0;
            for (int i=0;i<4;++i) target|=std::uint32_t(std::uint8_t(body[i]))<<(8*i);
            reply.targets.push_back(target);
            return reply;
        }
        if (body.size()>1024) return {};
        body=body.substr(0,body.find('\0'));
        for (;;) {
            const auto end=body.find('/');
            const std::string token(body.substr(0,end));
            const auto id=std::clamp(std::strtoll(token.c_str(),nullptr,10),-2147483648LL,2147483647LL);
            reply.targets.push_back(static_cast<std::uint32_t>(id));
            if (end==body.npos) break;
            body.remove_prefix(end+1);
        }
        return reply;
    }
};
}

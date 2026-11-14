#pragma once
#include <array>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
namespace mir3 {
inline bool valid_map_id(std::string_view id) {
    if (id.empty() || id.size()>64) return false;
    for (const unsigned char c:id)
        if (!((c>='0' && c<='9') || (c>='A' && c<='Z') || (c>='a' && c<='z') || c=='_' || c=='-')) return false;
    return true;
}
struct MapChangeMessage {
    int opcode,x,y;
    std::uint8_t ambient,series;
    std::string map_id;
    static std::optional<MapChangeMessage> parse(int opcode,const std::array<std::uint16_t,3>& parameters,std::string body) {
        if (opcode!=51 && opcode!=634) return {};
        // Original decoder consumes an ANSI NUL-terminated map filename.
        body.resize(body.find(char(0))==std::string::npos?body.size():body.find(char(0)));
        if (!valid_map_id(body)) return {};
        return MapChangeMessage{opcode,parameters[0],parameters[1],
            std::uint8_t(parameters[2]),std::uint8_t(parameters[2]>>8),std::move(body)};
    }
};
}

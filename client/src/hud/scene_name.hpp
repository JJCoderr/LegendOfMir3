#pragma once
#include "protocol/message_ids.hpp"
#include <array>
#include <string>
#include <string_view>
#include <cstdint>
namespace mir3 {
// mir3_1.45 ActorIdentity / ActorName: 0x408450, 0x401590, 0x40F210.
struct SceneName {
    std::array<std::string,3> lines;
    unsigned color=0xffffff;
    static unsigned colorref(std::uint8_t index) {
        switch (index) {
        case 47:return 3761061; case 69:return 27631;
        case 125:case 249:return 255; case 147:return 15718028;
        case 154:return 15709497; case 168:return 14580480;
        case 180:case 252:return 16711680; case 221:return 37888;
        case 229:return 16746615; case 251:return 0xffff;
        case 254:return 16776960; case 255:return 0xffffff;
        default:return 0;
        }
    }
    bool update(int opcode,std::uint8_t index,std::string_view ansi) {
        if (opcode!=mir3::protocol::SM_USERNAME && opcode!=mir3::protocol::SM_CHANGENAMECOLOR) return false;
        if (opcode==mir3::protocol::SM_USERNAME) {
            std::array<std::string,3> parsed;
            ansi=ansi.substr(0,ansi.find(char(0)));
            for (auto& line:parsed) {
                const auto cr=ansi.find('\r');
                auto part=ansi.substr(0,cr);
                if (cr==ansi.npos) part=part.substr(0,part.find('/'));
                if (part.size()>63) return false;
                line=part;
                if (cr==ansi.npos) break;
                ansi.remove_prefix(cr+1);
            }
            lines=std::move(parsed);
        }
        color=colorref(index);
        return true;
    }
    static std::array<int,4> rect(int x,int y,int width,int line) {
        const int left=x+(48-width)/2, right=x+(48+width)/2;
        return {left,line==0?y-30:y-15+(line-1)*15,right-left,15};
    }
};
}

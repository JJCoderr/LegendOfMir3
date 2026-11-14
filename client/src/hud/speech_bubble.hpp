#pragma once
#include "protocol/message_ids.hpp"
#include <array>
#include <cstdint>
#include <string>
#include <string_view>
#include <vector>
namespace mir3 {
struct SpeechReply { int identifier=mir3::protocol::SM_HEAR; std::uint32_t actor_id=0; std::string text; }; // decoded UTF-8 adapter
struct SpeechLine { std::string text; int width=0; };
std::vector<SpeechLine> wrap_speech(std::string_view text);
struct SpeechBubble {
    std::uint32_t actor_id=0;
    std::vector<SpeechLine> lines;
    double elapsed=0;
    bool receive(const SpeechReply& reply) {
        // 0x42F380 only40/1501 populate actor text; other channels log only.
        if (reply.actor_id!=actor_id || (reply.identifier!=mir3::protocol::SM_HEAR && reply.identifier!=mir3::protocol::SM_MONSTERSAY)) return false;
        auto wrapped=wrap_speech(reply.text);
        if (!reply.text.empty() && wrapped.empty()) return false;
        lines=std::move(wrapped);elapsed=0;return true;
    }
    void advance(double ms) {
        if (lines.empty()) return;
        elapsed+=ms;
        if (elapsed>3000) { lines.clear();elapsed=0; } // strict >,0x40FAC1
    }
    std::array<int,4> background(int x,int y) const {
        if (lines.empty()) return {};
        if (lines.size()==1) return {x-lines[0].width/2+28,y-73,lines[0].width,18};
        return {x-44,y-55-14*int(lines.size())-4,144,14*int(lines.size())+4};
    }
    std::array<int,2> position(int row,int x,int y) const {
        if (row<0 || row>=int(lines.size())) return {};
        int left=lines.size()==1?x-lines[0].width/2+28:x-44;
        if (lines.size()!=1 && row==int(lines.size())-1) left+=(144-lines[row].width)/2;
        return {left,y-55+14*(row-int(lines.size()))};
    }
};
}

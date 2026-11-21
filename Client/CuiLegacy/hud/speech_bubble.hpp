#pragma once
#include <array>
#include <cstdint>
#include <string>
#include <string_view>
#include <vector>
namespace mir3 {
struct SpeechLine { std::string text; int width=0; };
std::vector<SpeechLine> wrap_speech(std::string_view text);
struct SpeechBubble {
    std::vector<SpeechLine> lines;
    std::uint64_t expiresAt=0;
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

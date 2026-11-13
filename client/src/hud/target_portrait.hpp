#pragma once
#include <cstdint>
#include <array>
#include <string>
#include <utility>
namespace mir3 {
struct TargetAttributes {
    int category=-1, state=-1, icon=-1, element=0;
    std::array<int,7> modifiers{};
    bool visible() const { return category!=-1 && state!=-1 && icon!=-1; }
    std::array<int,7> order() const {
        std::array<int,7> indices{0,1,2,3,4,5,6};
        for (int end=6;end>0;--end)
            for (int i=end;i>0;--i)
                if (modifiers[indices[i]]>modifiers[indices[i-1]])
                    std::swap(indices[i],indices[i-1]);
        for (int i=0;i<7;++i) {
            if (modifiers[indices[i]]) continue;
            for (int next=i+1;next<7;++next) {
                if (!modifiers[indices[next]]) continue;
                std::swap(indices[i],indices[next]);
                break;
            }
        }
        return indices;
    }
};
// 0x438000 actor fields: +69 type, +71/+72 shape, +400432 exclusion.
struct TargetPortrait {
    static bool special_shape(std::uint16_t shape) { return shape>=530 && shape<=533; }
    // 0x43F300: actor+68 profession, actor+69 human type (0 or 1).
    static int human_frame(std::uint8_t profession,std::uint8_t type) {
        if (type>1) return -1;
        return profession<=2?650+2*profession+(type!=0):0;
    }
    static int monster_frame(std::uint8_t type, std::uint16_t shape, bool excluded) {
        if (type != 3 || excluded || shape == 127 || shape == 153) return -1;
        return shape >= 2000 || special_shape(shape) ? shape : shape & 255;
    }
    static int fallback_frame(int frame, bool available) {
        return frame < 0 ? -1 : (available ? frame : 1999);
    }
    // 0x438310: +400660 == 1 selects 635; special 530..533 omit badge.
    static int badge(std::uint16_t shape, int state) {
        return special_shape(shape) ? -1 : (state == 1 ? 635 : 634);
    }
};
// actor+0x61C38 points to stats: percent +9, maximum u16 +10, current u16 +12.
// 0x4387FC..0x438A17, checked against assembly (decompiler loses RECT arguments).
struct TargetPortraitVitals {
    std::uint8_t percent = 0;
    std::uint16_t maximum = 0, current = 0;
    int bar_width() const {
        // x87 loads 0x4AF49C as float and truncates via 0x49FE1C.
        return static_cast<int>(double(percent) * double(0.01f) * 84.0);
    }
    std::string text(int part) const {
        if (part == 1) return "/";
        if (part != 0 && part != 2) return {};
        return current ? std::to_string(part == 0 ? current : maximum) : "?";
    }
    static std::array<int,4> text_rect(int part) {
        constexpr std::array<std::array<int,4>,3> rects{{
            {532,570,30,15}, {562,570,5,15}, {565,570,30,15}
        }};
        return part >= 0 && part < 3 ? rects[part] : std::array<int,4>{};
    }
};
// 0x42488B..0x424950: current +0x374674, displayed +0x374678,
// previous +0x37467C; tick +0x438270. Identity zero denotes no actor.
class PortraitFocus {
public:
    void update(std::uint32_t current, std::uint32_t now) {
        if (current) {
            shown = current;
            if (current != previous) changed_at = now;
            previous = current;
        } else if (std::uint32_t(now - changed_at) > 800) shown = 0;
    }
    void clear() { shown = previous = changed_at = 0; }
    std::uint32_t displayed() const { return shown; }
private:
    std::uint32_t shown = 0, previous = 0, changed_at = 0;
};
}

#pragma once
#include <array>
#include <cstdint>
#include <string>
#include <string_view>

namespace mir3 {
// Original client 800x600 space. 0x435820: HUD this+0xC5C/+0xC60,
// y=601-GameInter[1160].height=467 for the supplied packed resources.
// Button object stride 180; RECT at +4, draw x/y at +40/+44 (0x41FA00).
struct HudLayout {
    using Point = std::array<int, 2>;
    // 0x435820: RECT at this+0xC5C; supplied GameInter[1160] is 134px high.
    static constexpr Point origin{0, 601 - 134};
    // 0x438000 local custom portrait and remote human custom portrait blits.
    static constexpr std::array<int,4> custom_portrait_rect(bool remote) {
        return {origin[0]+534,origin[1]+(remote?13:20),remote?84:82,remote?103:94};
    }
    // 0x438652..0x4386DE; raw frame origin, first dye RGB, second dye white.
    static constexpr Point decoration_position{origin[0]-10,origin[1]-107};
    static constexpr int decoration_frame(std::uint8_t selector) { return 1689+selector; }
    // 0x438000 portrait; 0x4390E0 orbs, meters and stat labels.
    // Preserve raw image origins (these calls do not add WIL frame offsets).
    enum class Image { skin, portrait, hp, mp, experience, weight, left_stat, right_stat, target_badge, target_hp_border, target_hp_fill };
    static constexpr Point image_position(Image image) {
        constexpr std::array<Point, 11> offsets{{
            {0,0}, {534,20}, {33,8}, {73,8},
            {152,26}, {165,26}, {635,105}, {715,105}, {595,94}, {533,116}, {534,117}
        }};
        const auto index = static_cast<unsigned>(image);
        if (index >= offsets.size()) return {};
        return {origin[0] + offsets[index][0], origin[1] + offsets[index][1]};
    }
    bool expanded = false;
    bool repositioned = false;
    void toggle() { expanded = !expanded; repositioned = true; }
    using Rect = std::array<int, 4>; // x, y, width, height
    // 0x436A30 positions object +0x6960 at the HUD origin plus (535,100).
    static constexpr Point portrait_entry_position{origin[0]+535,origin[1]+100};
    static constexpr int portrait_entry_frame(bool pressed) { return pressed?2097:2096; }
    // 0x43849D..0x4384BD: float origin/extent, drawn BEFORE GameInter[1160].
    static constexpr Rect chat_backing{185,317,433,213};
    // 0x438599/0x4385E1: offsets from HUD origin; sizes from packed frames.
    static constexpr Rect chat_frame(bool expanded) {
        return {origin[0]+178,origin[1]-(expanded?186:19),456,expanded?320:152};
    }
    // 0x435820 this+0xDDC/+0xDEC; 0x436DE0/0x436E90 TextOutA rows.
    static constexpr Rect chat_line(bool expanded) { return {189,expanded?320:492,expanded?429:341,15}; }
    static constexpr int chat_rows(bool expanded) { return expanded?14:5; }
    static constexpr std::array<std::array<int, 3>, 23> buttons{{
        {1170,637,473}, {1172,637,507}, {1174,637,542},
        {1176,7,539}, {1178,108,539}, {1180,706,475},
        {1182,674,478}, {1184,739,500}, {1186,742,478},
        {1188,706,539}, {1190,742,528}, {1192,674,528},
        {1194,682,500}, {1196,711,498}, {1168,619,472},
        {1205,150,469}, {1330,186,557}, {1333,226,557},
        {1335,265,556}, {1337,305,556}, {1338,346,556},
        {1340,386,556}, {1342,426,556}
    }};
    // 0x435820 explicit SetRect overrides for buttons 5..15.
    static constexpr std::array<Rect, 11> hit_overrides{{
        {708,476,34,19}, {674,475,29,25}, {740,502,28,29},
        {742,478,35,24}, {709,539,35,20}, {744,531,36,24},
        {675,532,34,22}, {682,504,30,27}, {712,498,28,37},
        {619,472,13,17}, {150,469,24,15}
    }};
    static int count(bool expanded) { return expanded ? 23 : 16; }
    static std::array<int, 3> button(int index, bool expanded) {
        if (index < 0 || index >= count(expanded)) return {};
        auto value = buttons[index];
        if (index == 14 && expanded) value[2] = 305; // 0x43EF10
        return value;
    }
    static Rect hit_rect(int index, bool expanded, int frame_width, int frame_height, bool repositioned = false) {
        if (index < 0 || index >= count(expanded)) return {};
        if (index >= 5 && index <= 15) {
            auto rect = hit_overrides[index - 5];
            if (index == 14 && expanded) rect[1] = 305;
            // 0x43EF10 calls 0x41FD20 after SetRect. The latter overwrites
            // the bounds with frame dimensions, including after collapsing.
            if (index == 14 && repositioned && frame_width > 0 && frame_height > 0) {
                rect[2] = frame_width; rect[3] = frame_height;
            }
            return rect;
        }
        const auto position = button(index, expanded);
        // 0x41FA00 gets initial bounds from the first frame's raw dimensions.
        // 0x435820 overrides all seven expanded-mode buttons to 32x32.
        return {position[1], position[2], index >= 16 ? 32 : frame_width,
                index >= 16 ? 32 : frame_height};
    }
    // 0x4390E0: draw RECTs; these are not button hit regions.
    static constexpr Rect coordinates{6,575,133,17};
    // 0x4390E0 -> 0x48BD63: COLORREF (BGR), not an RGB literal.
    // Element 0: coordinates, 1: level, 2/3: left/right stat.
    static constexpr unsigned text_color(int element) {
        return element == 0 ? 0xC8FFFFu : (element == 1 ? 0x00E1E1u : 0x32C8FFu);
    }
    static constexpr unsigned coordinate_shadow_color = 0x0A0A0A;
    static constexpr std::array<Point, 4> coordinate_shadows{{{-1,-1},{1,-1},{-1,1},{1,1}}};
    static Rect level(bool expanded) { return {393, expanded ? 298 : 466,16,16}; }
    // 0x43914A..0x43926B: selector 0x824750 compared in this exact order
    // against 0x8F1CD4, 0x8F1CD0, 0x8F1CD8, 0x8F1CDC. Their semantic
    // meaning is not yet recovered. No match uses LOWORD(experience).
    static constexpr int level_emblem_frame(std::uint32_t selector,
            const std::array<std::uint32_t,4>& references, std::uint32_t experience) {
        constexpr std::array<int,4> frames{1201,1200,1203,1202};
        for (unsigned i=0;i<references.size();++i)
            if (selector==references[i]) return frames[i];
        return static_cast<std::uint16_t>(experience);
    }
    static constexpr Point level_emblem_position(bool expanded) {
        return {origin[0]+(expanded?384:385),origin[1]-(expanded?178:11)};
    }
    // 0x435820: RECT fields +0xC6C/+0xC7C/+0xC8C/+0xC9C.
    // 0x4390E0 tests these via PtInRect (exclusive right/bottom).
    static constexpr int tooltip_at(Point p, bool shared_hp) {
        constexpr std::array<Rect,4> areas{{{33,475,36,88},{73,475,36,88},
            {148,490,12,96},{161,490,12,96}}};
        for (int i=0;i<4;++i) {
            const auto& r=areas[i];
            if (p[0]>=r[0] && p[0]<r[0]+r[2] && p[1]>=r[1] && p[1]<r[1]+r[3])
                return i<2 ? (shared_hp?2:i) : (i==2?4:3);
        }
        return -1;
    }
    // Kind: HP, MP, shared HP, weight, experience. Original integer SIZE/2
    // truncates before SetRect; odd glyph extents must not be rounded up.
    static constexpr Rect tooltip_rect(int kind, Point text_size) {
        constexpr std::array<Point,5> centers{{{55,528},{105,528},{70,528},{210,535},{152,544}}};
        if (kind<0 || kind>=5) return {};
        const auto c=centers[kind];
        const int x=text_size[0]/2,y=text_size[1]/2;
        return {c[0]-x-5,c[1]-y-1,2*x+10,2*y+2};
    }
    static Rect stat(int side) { return side == 0 ? Rect{663,570,50,24} : Rect{742,570,56,24}; }
};
// 0x41F840: button RECT right/bottom at +0x0C/+0x10, caption at +0x34.
// Global shared state: 0x4B5644=1, 0x4B5648=6, previous caption 0x903884.
struct HudButtonTooltip {
    std::string previous;
    bool expanding = true;
    int height = 6;
    HudLayout::Rect draw(std::string_view caption, HudLayout::Point text_size,
                         HudLayout::Point anchor) {
        if (caption.empty()) return {};
        const int width = text_size[0] + 12, full_height = text_size[1] + 4;
        int left = anchor[0];
        if (left + width > 800) left = 800 - width;
        // Caption changes do not reset an in-progress height counter.
        if (previous != caption) { previous = caption; expanding = true; }
        const int drawn_height = expanding ? std::min(++height, full_height) : full_height;
        if (expanding && height >= full_height) { expanding = false; height = 6; }
        return {left, anchor[1] - drawn_height, width, drawn_height};
    }
};

}

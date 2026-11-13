#pragma once
#include <array>
namespace mir3 {
struct ButtonSkin {
    int normal;          // +24
    int pressed;         // +28
    int overlay = -1;    // +32
    bool hover_draw = false; // +48
};
inline std::array<int, 2> button_frames(ButtonSkin skin, int state, bool enabled = true)
{
    if (state == 2) return {skin.pressed, -1};
    if (!enabled) return {-1, -1};
    if (state == 1) return {skin.hover_draw ? skin.normal : -1, skin.overlay};
    return {skin.overlay, -1};
}
// 0x435820 initial configuration; background GameInter[1160/1162] already
// contains the idle icons. Dynamic toggles can override these skins later.
inline std::array<int, 2> main_button_frames(int index, bool hovered, bool pressed)
{
    constexpr std::array<int, 23> normal{1170,1172,1174,1176,1178,1180,
        1182,1184,1186,1188,1190,1192,1194,1196,1168,1205,
        1330,1333,1335,1337,1338,1340,1342};
    if (index < 0 || index >= static_cast<int>(normal.size())) return {-1, -1};
    const bool pair = index >= 5 && index <= 13;
    return button_frames({normal[index], normal[index] + (pair ? 1 : 0), -1,
        index >= 5 && index <= 14}, pressed ? 2 : (hovered ? 1 : 0));
}
}

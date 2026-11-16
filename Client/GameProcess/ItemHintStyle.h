#pragma once
// MIR3 448770 renderer and 440B30 / 4784F0 callers.
namespace ItemHintStyle {
constexpr int PointerOffset=10;
constexpr int MaleArmor=10, FemaleArmor=11;
constexpr int Padding=5, BottomPadding=2, LineHeight=15;
constexpr int ViewWidth=800, IconClipHeight=530;
inline bool ShopIcon(int mode) { return mode==MaleArmor || mode==FemaleArmor; }
}

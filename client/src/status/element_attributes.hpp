#pragma once
#include <array>
#include <cstdint>
#include <string>
namespace mir3 {
// 0x4710F0: attack bytes 0x8246B8+2*i, strength 0x8246C5+i,
// weakness 0x8246CC+i. Preserve raw bytes and original signed comparisons.
struct ElementAttributes {
    std::array<std::array<std::uint8_t,7>,3> bytes{};
    std::array<std::uint16_t,7> attack_words{};
    static bool valid(int row,int column) { return row>=0 && row<3 && column>=0 && column<7; }
    bool set(int row,int column,int raw) {
        if (!valid(row,column) || raw<0 || raw>255) return false;
        bytes[row][column]=static_cast<std::uint8_t>(raw);
        if (row==0) attack_words[column]=std::uint16_t((attack_words[column]&255)|(unsigned(raw)<<8));
        return true;
    }
    int value(int row,int column) const {
        if (!valid(row,column)) return 0;
        const int v=bytes[row][column];
        return row==0 || v<128 ? v : v-256;
    }
    bool visible(int row,int column) const { return value(row,column)>0; }
    std::string text(int row,int column) const {
        if (!visible(row,column)) return {};
        // Raw literals 0x4B5DBC="+%d", 0x4B5DB8="x%d" (not pointers).
        return std::string(row==0 ? "+" : "x")+std::to_string(value(row,column));
    }
};
}

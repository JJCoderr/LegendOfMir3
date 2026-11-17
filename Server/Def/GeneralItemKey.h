#pragma once
#include <cstdint>
#include <cstdio>

// Database/GameSvr key, not the client wire identity. Keep decimal legacy keys
// when possible; G~ + ten base32 digits carries three full 16-bit fields.
namespace GeneralItemKey {
constexpr int Size = 12;
constexpr char Alphabet[] = "0123456789ABCDEFGHIJKLMNOPQRSTUV";
struct Fields { int standard = 0, durability = 0, maximum = 0; };
inline void Encode(char* key, std::uint16_t standard, std::uint16_t durability, std::uint16_t maximum)
{
    if (standard <= 999 && durability <= 9999 && maximum <= 9999) {
        std::sprintf(key, "G%03u%04u%04u", unsigned(standard), unsigned(durability), unsigned(maximum));
        return;
    }
    std::uint64_t bits = (std::uint64_t(standard) << 32) | (std::uint64_t(durability) << 16) | maximum;
    key[0] = 'G'; key[1] = '~'; key[Size] = 0;
    for (int at = Size - 1; at >= 2; --at) { key[at] = Alphabet[bits & 31]; bits >>= 5; }
}
inline bool Decode(const char* key, Fields& fields)
{
    if (key[0] != 'G') return false;
    if (key[1] == '~') {
        std::uint64_t bits = 0;
        for (int at = 2; at < Size; ++at) {
            const char c = key[at];
            const int digit = c >= '0' && c <= '9' ? c - '0' : c >= 'A' && c <= 'V' ? c - 'A' + 10 : -1;
            if (digit < 0 || (at == 2 && digit > 7)) return false;
            bits = (bits << 5) | digit;
        }
        fields = {int(bits >> 32), int((bits >> 16) & 65535), int(bits & 65535)};
    } else {
        Fields result;
        for (int at = 1; at < Size; ++at) {
            if (key[at] < '0' || key[at] > '9') return false;
            int& value = at < 4 ? result.standard : at < 8 ? result.durability : result.maximum;
            value = value * 10 + key[at] - '0';
        }
        fields = result;
    }
    return true;
}
}

#pragma once
#include <cstdint>
namespace HumanFeature {
constexpr unsigned HairBits=6;
constexpr std::uint8_t HairMask=(1u<<HairBits)-1;
constexpr std::uint8_t Hair(std::uint8_t value) { return value&HairMask; }
constexpr std::uint8_t Profession(std::uint8_t value) { return value>>HairBits; }
constexpr std::uint8_t Pack(std::uint8_t hair,std::uint8_t profession) {
    return Hair(hair)|(profession<<HairBits);
}
}

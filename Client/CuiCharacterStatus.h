#pragma once

#include <array>
#include <cstdint>
#include <string>

namespace mir3_cui {

struct CharacterAppearance {
    bool available = false;
    unsigned body = 0;
    unsigned weapon = 0;
    unsigned clothing = 0;
    unsigned hair = 0;
    unsigned profession = 0;

    bool Apply(std::uint32_t feature)
    {
        const auto type = feature & 0xff;
        const auto dress = (feature >> 16) & 0xff;
        if (type > 1 || dress >= 9) return false;

        const auto weaponId = (feature >> 8) & 0xff;
        const auto hairId = (feature >> 24) & 0x3f;
        available = true;
        body = type;
        weapon = weaponId < 50 ? weaponId : 0;
        clothing = dress;
        hair = hairId < 30 ? hairId : 0;
        profession = feature >> 30;
        return true;
    }
};

struct CharacterStatus {
    using Range = std::array<unsigned, 2>;

    bool received = false;
    unsigned level = 0;
    std::uint32_t experience = 0;
    std::uint32_t nextExperience = 0;
    Range hp{}, mp{}, weight{}, wear{}, hand{};
    std::array<Range, 5> combat{};
    std::array<std::array<unsigned, 7>, 3> elements{};
    std::array<std::uint16_t,7> attackElements{};
    unsigned accuracy = 0;
    unsigned agility = 0;
    std::uint16_t weaponTraining = 0;

    bool ApplyAbility(const std::string& bytes, unsigned profession)
    {
        if (bytes.size() != 63 || profession > 2) return false;
        const auto byte = [&](int offset) { return static_cast<unsigned char>(bytes[offset]); };
        const auto word = [&](int offset) { return unsigned(byte(offset)) | (unsigned(byte(offset + 1)) << 8); };
        const auto dword = [&](int offset) { return std::uint32_t(word(offset)) | (std::uint32_t(word(offset + 2)) << 16); };

        received = true;
        level = byte(0);
        combat = {{{byte(5), byte(6)}, {byte(7), byte(8)}, {byte(9), byte(10)},
            {byte(1), byte(2)}, {byte(3), byte(4)}}};
        hp = {word(11), word(15)};
        mp = {word(13), word(17)};
        experience = dword(19);
        nextExperience = dword(23);
        weight = {word(27), word(29)};
        wear = {byte(31), byte(32)};
        hand = {byte(33), byte(34)};
        for (int column = 0; column < 7; ++column) {
            attackElements[column] = static_cast<std::uint16_t>(word(35 + 2 * column));
            elements[0][column] = byte(36 + 2 * column);
            elements[1][column] = byte(49 + column);
            elements[2][column] = byte(56 + column);
        }
        return true;
    }

    double ExperiencePercent() const
    {
        return nextExperience ? 100.0 * experience / nextExperience : 0.0;
    }

    unsigned DisplayedElement(int row, int column) const
    {
        const auto raw = elements[row][column];
        return row == 0 || raw < 128 ? raw : 0;
    }
};

} // namespace mir3_cui

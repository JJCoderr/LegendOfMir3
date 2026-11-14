#pragma once
#include "protocol/message_ids.hpp"
#include <array>
#include <cstdint>
namespace mir3 {
// mir3_1.45 chat_message.hpp: 0x401430, not the actor-name palette.
constexpr std::uint32_t chat_palette(std::uint8_t index) {
    constexpr std::array<std::uint32_t,8> colors{0,657930,0xffffff,255,65280,16750230,0x50ffff,33023};
    return index<colors.size()?colors[index]:0;
}
constexpr bool chat_opcode(int opcode) { return opcode==mir3::protocol::SM_HEAR || opcode==mir3::protocol::SM_MONSTERSAY || (opcode>=mir3::protocol::SM_SYSMESSAGE && opcode<=mir3::protocol::SM_GUILDMESSAGE); }
}

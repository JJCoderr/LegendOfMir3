#pragma once
#include "default_message.hpp"
#include <array>
#include <cstdint>
#include <optional>
#include <string>
#include <span>
namespace mir3 {
inline std::optional<std::string> decode_six_bit(const std::uint8_t* encoded,std::size_t size) {
    if (size%4==1) return {};
    std::string decoded;
    decoded.reserve(size*3/4);
    unsigned accumulator=0;
    int bits=0;
    for (std::size_t i=0;i<size;++i) {
        const auto byte=encoded[i];
        if (byte<60 || byte>123) return {};
        accumulator=(accumulator<<6)|(byte-60);bits+=6;
        if (bits>=8) { bits-=8;decoded+=char((accumulator>>bits)&255); }
    }
    if (bits && (accumulator&((1u<<bits)-1))) return {};
    return decoded;
}
struct ReceiveHeader {
    static constexpr std::size_t encoded_size=16;
    std::uint32_t recognition=0;
    std::uint16_t opcode=0;
    std::array<std::uint16_t,3> parameters{};
    // 0x47D890: login/selection headers have no scene permutation or key.
    static std::optional<ReceiveHeader> decode_plain(std::span<const std::uint8_t> message) {
        if (message.size()<encoded_size) return {};
        const auto bytes=decode_six_bit(message.data(),encoded_size);
        if (!bytes) return {};
        const auto header=protocol::DefaultMessage::decode({reinterpret_cast<const std::uint8_t*>(bytes->data()),bytes->size()});
        if (!header) return {};
        return ReceiveHeader{header->recognition,header->identifier,header->parameters};
    }
    // 0x47D890 and 0x428308..0x428399, already-framed receive message.
    static std::optional<ReceiveHeader> decode(std::span<const std::uint8_t> message,std::span<const std::uint8_t> key) {
        if (message.size()<16 || key.size()!=4) return {};
        const auto bytes=decode_six_bit(message.data(),16);
        if (!bytes || bytes->size()!=12) return {};
        const auto b=[&](int i) { return std::uint32_t(std::uint8_t((*bytes)[i])); };
        return ReceiveHeader{(b(5)^key[3])|(b(7)<<8)|(b(9)<<16)|(b(11)<<24),
            std::uint16_t((b(0)^key[0])|((b(4)^key[2]^key[1])<<8)),
            {std::uint16_t(b(1)|(b(6)<<8)),std::uint16_t(b(2)|(b(8)<<8)),std::uint16_t(b(3)|(b(10)<<8))}};
    }
};
}

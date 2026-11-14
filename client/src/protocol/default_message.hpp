#pragma once
#include <array>
#include <cstdint>
#include <optional>
#include <span>
namespace mir3::protocol {
// 参考 Common/EnDecode.h 的 TDEFAULTMESSAGE：nRecog、wIdent、wParam、wTag、wSeries。
// 显式小端序列化，避免依赖宿主字节序、int 宽度或结构体对齐。
struct DefaultMessage {
    static constexpr std::size_t wire_size=12;
    std::uint32_t recognition=0;
    std::uint16_t identifier=0;
    std::array<std::uint16_t,3> parameters{};
    std::array<std::uint8_t,wire_size> encode() const {
        std::array<std::uint8_t,wire_size> bytes{};
        for (unsigned i=0;i<4;++i) bytes[i]=std::uint8_t(recognition>>(8*i));
        const auto word=[&](unsigned offset,std::uint16_t value) {
            bytes[offset]=std::uint8_t(value);bytes[offset+1]=std::uint8_t(value>>8);
        };
        word(4,identifier);
        for (unsigned i=0;i<parameters.size();++i) word(6+2*i,parameters[i]);
        return bytes;
    }
    static std::optional<DefaultMessage> decode(std::span<const std::uint8_t> bytes) {
        if (bytes.size()!=wire_size) return {};
        const auto word=[&](unsigned offset) { return std::uint16_t(bytes[offset]|(std::uint16_t(bytes[offset+1])<<8)); };
        std::uint32_t recognition=0;
        for (unsigned i=0;i<4;++i) recognition|=std::uint32_t(bytes[i])<<(8*i);
        return DefaultMessage{recognition,word(4),{word(6),word(8),word(10)}};
    }
};
}

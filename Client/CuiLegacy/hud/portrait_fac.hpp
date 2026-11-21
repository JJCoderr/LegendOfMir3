#pragma once
#include <array>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>

namespace mir3 {
// 0x43F3B0 supplies these two values to 0x47D450 before decoding a FAC.
struct PortraitFac {
    static constexpr std::uint32_t length_key=static_cast<std::uint32_t>(-1901381136);
    static constexpr std::uint32_t checksum_key=static_cast<std::uint32_t>(-1612834157);
    static constexpr std::size_t max_payload=4095;

    static std::optional<std::string> encode(std::string_view jpeg) {
        if (jpeg.size()>max_payload) return std::nullopt;
        std::string file(8,'\0');
        for (unsigned i=0;i<4;++i)
            file[i]=char(((jpeg.size()>>(24-8*i))^(length_key>>(8*i)))&255);
        file.append(jpeg);
        transform(file); // 0x47D4C0: XOR four incrementing byte streams.
        const auto check=checksum(std::string_view(file).substr(8));
        for (unsigned i=0;i<4;++i) file[4+i]=char((check>>(8*i))&255);
        return file;
    }

    static std::optional<std::string> decode(std::string_view file) {
        if (file.size()<8 || file.size()>8+max_payload) return std::nullopt;
        std::uint32_t length=0, check=0;
        for (unsigned i=0;i<4;++i) {
            length=(length<<8)|(std::uint8_t(file[i])^std::uint8_t(length_key>>(8*i)));
            check|=std::uint32_t(std::uint8_t(file[4+i]))<<(8*i);
        }
        // 0x47D560 verifies checksum before reversing XOR. Also bound the declared length.
        if (length!=file.size()-8 || check!=checksum(file.substr(8))) return std::nullopt;
        std::string decoded(file);
        transform(decoded);
        return decoded.substr(8);
    }

private:
    static std::uint32_t checksum(std::string_view body) {
        std::uint32_t sum=0;
        for (std::size_t i=0;i<body.size();++i)
            sum+=static_cast<std::uint32_t>(i)*(std::uint8_t(body[i])+1u);
        return sum^checksum_key;
    }
    static void transform(std::string& file) {
        for (std::size_t i=0;i<file.size()-8;++i)
            for (unsigned j=0;j<4;++j)
                file[i+8]=char(std::uint8_t(file[i+8])^std::uint8_t(std::uint8_t(file[j])+i));
    }
};
}

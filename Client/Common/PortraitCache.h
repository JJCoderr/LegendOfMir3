#pragma once
#include <cstdint>
#include <vector>
// MIR3 0x47D4C0 / 0x47D560; fixed portrait keys set by 0x43F3B0.
namespace PortraitCache {
constexpr std::uint32_t key=std::uint32_t(-1901381136),checkKey=std::uint32_t(-1612834157);
inline std::uint32_t Checksum(const std::vector<std::uint8_t>& bytes) {
    std::uint32_t sum=0;
    for(std::size_t i=8;i<bytes.size();++i) sum+=std::uint32_t(i-8)*(std::uint32_t(bytes[i])+1);
    return sum^checkKey;
}
inline std::vector<std::uint8_t> Encode(const std::vector<std::uint8_t>& jpeg) {
    if(jpeg.empty() || jpeg.size()>4096) return {};
    std::vector<std::uint8_t> bytes(8+jpeg.size());
    for(int i=0;i<4;++i) bytes[i]=std::uint8_t((key>>(8*i))^(std::uint32_t(jpeg.size())>>(24-8*i)));
    for(std::size_t i=0;i<jpeg.size();++i) {
        bytes[i+8]=jpeg[i];
        for(int j=0;j<4;++j) bytes[i+8]^=std::uint8_t(bytes[j]+i);
    }
    const auto checksum=Checksum(bytes);for(int i=0;i<4;++i) bytes[i+4]=std::uint8_t(checksum>>(8*i));
    return bytes;
}
inline std::vector<std::uint8_t> Decode(const std::vector<std::uint8_t>& bytes) {
    if(bytes.size()<9 || bytes.size()>4104) return {};
    std::uint32_t size=0,checksum=0;
    for(int i=0;i<4;++i) {
        size=(size<<8)|std::uint8_t(bytes[i]^(key>>(8*i)));
        checksum|=std::uint32_t(bytes[i+4])<<(8*i);
    }
    if(size!=bytes.size()-8 || checksum!=Checksum(bytes)) return {};
    std::vector<std::uint8_t> jpeg(bytes.begin()+8,bytes.end());
    for(std::size_t i=0;i<jpeg.size();++i) for(int j=3;j>=0;--j) jpeg[i]^=std::uint8_t(bytes[j]+i);
    return jpeg;
}
}

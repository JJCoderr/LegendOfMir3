#pragma once
#include <cstdint>
#include <span>
#include <map>
#include <string>
namespace mir3 {
// LegendOfMir3_Client/Sound/TypeDef.h: packed 58-byte header, 16-byte entries.
class SoundList {
public:
    bool parse(std::span<const std::uint8_t> bytes) {
        if (bytes.size()<58) return false;
        const auto count=std::uint32_t(bytes[54])|(std::uint32_t(bytes[55])<<8)|
            (std::uint32_t(bytes[56])<<16)|(std::uint32_t(bytes[57])<<24);
        if (count>(bytes.size()-58)/16) return false;
        std::map<int,std::string> loaded;
        for (std::size_t i=0;i<count;++i) {
            const auto entry=bytes.subspan(58+i*16,16);
            const int id=entry[0]|(int(entry[1])<<8);
            if (!id) continue;
            std::string name;
            for (int j=2;j<16 && entry[j];++j) name+=char(entry[j]);
            // ReadWavFileList trims stale bytes after the wav suffix.
            if (const auto end=name.find("wav");end!=std::string::npos) name.resize(end+3);
            loaded.try_emplace(id,std::move(name));
        }
        names=std::move(loaded);
        return true;
    }
    std::string file(int id) const {
        const auto it=names.find(id);
        return it==names.end()?std::string{}:it->second;
    }
private:
    std::map<int,std::string> names;
};
}

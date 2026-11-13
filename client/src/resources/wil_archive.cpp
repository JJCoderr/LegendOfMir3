#include "wil_archive.hpp"
#include <iterator>

namespace mir3 {
namespace {
unsigned u16(const std::uint8_t* p) { return p[0] | (unsigned(p[1])<<8); }
std::uint32_t u32(const std::uint8_t* p) { return u16(p) | (std::uint32_t(u16(p+2))<<16); }
std::filesystem::path find_file(std::filesystem::path stem, std::string extension) {
    auto expected=stem; expected+=extension;
    if (std::filesystem::exists(expected)) return expected;
    // Original installations mix GameInter.WIL / gameinter.wix casing.
    auto name=expected.filename().string();
    for (auto& c:name) c=char(std::tolower(static_cast<unsigned char>(c)));
    std::error_code error;
    for (const auto& entry:std::filesystem::directory_iterator(stem.parent_path(),error)) {
        auto candidate=entry.path().filename().string();
        for (auto& c:candidate) c=char(std::tolower(static_cast<unsigned char>(c)));
        if (candidate==name) return entry.path();
    }
    return expected;
}
}
bool WilArchive::open(const std::filesystem::path& stem) {
    archive.close(); archive.clear(); index.clear(); index_start=header_size=0;
    archive.open(find_file(stem,".wil"),std::ios::binary|std::ios::ate);
    if (!archive || archive.tellg()<28) return false;
    length=static_cast<std::uint64_t>(archive.tellg());
    std::uint8_t version_bytes[2]{};
    archive.seekg(22); archive.read(reinterpret_cast<char*>(version_bytes),2);
    const auto version=u16(version_bytes);
    index_start=version==17?24:version==5000?28:version==6000?32:0;
    header_size=version==17?17:21;
    if (!index_start) return false;
    std::ifstream wix(find_file(stem,".wix"),std::ios::binary);
    index.assign(std::istreambuf_iterator<char>(wix),{});
    return index.size()>=static_cast<std::size_t>(index_start);
}
std::optional<WilFrame> WilArchive::frame(int id) {
    if (id<0 || !index_start) return {};
    const auto entry=std::uint64_t(index_start)+std::uint64_t(id)*4;
    if (entry+4>index.size()) return {};
    const auto offset=u32(index.data()+entry);
    if (!offset || std::uint64_t(offset)+header_size>length) return {};
    std::uint8_t header[21]{};
    archive.clear(); archive.seekg(offset); archive.read(reinterpret_cast<char*>(header),header_size);
    WilFrame result;
    result.width=static_cast<std::int16_t>(u16(header));
    result.height=static_cast<std::int16_t>(u16(header+2));
    result.offset_x=static_cast<std::int16_t>(u16(header+4));
    result.offset_y=static_cast<std::int16_t>(u16(header+6));
    const auto bytes=std::uint64_t(u32(header+13))*2;
    if (!archive || result.width<=0 || result.height<=0 || result.width>4096 || result.height>4096 || bytes>length-offset-header_size) return {};
    std::vector<std::uint8_t> source(static_cast<std::size_t>(bytes));
    archive.read(reinterpret_cast<char*>(source.data()),source.size());
    if (!archive || !decode_wil_pixels(source,result.width,result.height,result.pixels)) return {};
    return result;
}
}

#include "messages.hpp"
#include <array>
#include <string_view>
#include <cstdlib>

namespace mir3 {
std::optional<std::string> decode_legacy_text(std::span<const std::uint8_t> file)
{
    // 0x480570 -> 0x47D450 / 0x47D560. Keep decoding exclusively in memory.
    if (file.size() < 8) return {};
    constexpr std::array<std::uint8_t, 4> key{0xf0, 0x39, 0xab, 0x8e};
    std::uint32_t length = 0, checksum = 0, stored = 0;
    for (int i = 0; i < 4; ++i) {
        length = (length << 8) | (file[i] ^ key[i]);
        stored |= std::uint32_t(file[i + 4]) << (8 * i);
    }
    if (length != file.size() - 8) return {};
    for (std::uint32_t i = 0; i < length; ++i) checksum += i * (std::uint32_t(file[i + 8]) + 1);
    if ((checksum ^ std::uint32_t(-1612834157)) != stored) return {};
    std::string plain(reinterpret_cast<const char*>(file.data() + 8), length);
    for (int j = 3; j >= 0; --j)
        for (std::uint32_t i = 0; i < length; ++i)
            plain[i] = char(std::uint8_t(plain[i]) ^ std::uint8_t(file[j] + i));
    return plain;
}
bool Messages::load(std::span<const std::uint8_t> file)
{
    const auto decoded=decode_legacy_text(file);if (!decoded) return false;
    const auto& plain=*decoded;
    std::unordered_map<std::string, std::string> parsed;
    std::size_t start = 0;
    while (start < plain.size()) {
        const auto end = plain.find("\r\n", start);
        if (end == std::string::npos) break;
        const auto line = std::string_view(plain).substr(start, end - start);
        start = end + 2;
        if (line.empty() || line.find(';') != std::string_view::npos || line.front() != '#') continue;
        // The original tree key is a string; section markers such as
        // #Messenger.cpp and records with an empty value are legal.
        const char* next = line.data() + 1;
        while (next != line.data() + line.size() && *next != ' ' && *next != '\t') ++next;
        const std::string key_name(line.data() + 1, next);
        const char* text = next;
        while (text != line.data() + line.size() && (*text == ' ' || *text == '\t')) ++text;
        if (!parsed.emplace(key_name, std::string(text, line.data() + line.size())).second) return false;
    }
    if (parsed.empty()) return false;
    values = std::move(parsed);
    return true;
}
const std::string& Messages::get(int id) const
{
    static const std::string empty;
    const auto found = values.find(std::to_string(id));
    return found == values.end() ? empty : found->second;
}
}

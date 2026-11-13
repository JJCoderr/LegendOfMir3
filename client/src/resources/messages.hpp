#pragma once
#include <cstdint>
#include <span>
#include <string>
#include <unordered_map>
#include <optional>
#include <vector>

namespace mir3 {
std::optional<std::string> decode_legacy_text(std::span<const std::uint8_t> file);
class Messages {
public:
    bool load(std::span<const std::uint8_t> file);
    const std::string& get(int id) const;
private:
    std::unordered_map<std::string, std::string> values;
};
}

#pragma once
#include "wil_pixels.hpp"
#include <filesystem>
#include <fstream>
#include <optional>

namespace mir3 {
struct WilFrame {
    int width=0, height=0, offset_x=0, offset_y=0;
    WilPixels pixels;
    int shadow_mode=48, shadow_x=0, shadow_y=0;
};
// Adapted from mir3_1.45/wil.gd. Archives are read in place; no exported frames.
class WilArchive {
public:
    bool open(const std::filesystem::path& stem);
    std::optional<WilFrame> frame(int id);
private:
    std::ifstream archive;
    std::vector<std::uint8_t> index;
    std::uint64_t length=0;
    int index_start=0, header_size=0;
};
}

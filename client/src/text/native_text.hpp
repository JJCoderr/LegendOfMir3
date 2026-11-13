#pragma once
#include <cstdint>
#include <string>
#include <string_view>
#include <vector>
namespace mir3 {
std::string edit_multiline_ansi(std::string_view previous_ansi,std::string_view proposed_utf8);
struct NativeText {
    std::vector<std::uint8_t> rgba;
    std::string face;
    int dpi = 0;
    unsigned code_page = 0;
    int text_width = 0;
    int text_height = 0;
};
// 0x48BB04 / 0x48BB90 / 0x48BD63 / 0x48BCAF, original ANSI GDI path.
struct DescriptionRow { std::string text; int width=0; bool title=false; };
std::vector<DescriptionRow> wrap_skill_description(const std::vector<std::string>& lines);
enum class TextFont { normal, level, gold, character_name, dialog, skill_level, skill_title };
NativeText raster_text(std::string_view utf8, int width, int height, bool centered,
    TextFont font, std::string_view localized_face = {}, bool inverted_vertical = false);
std::string ansi_to_utf8(std::string_view text);
}

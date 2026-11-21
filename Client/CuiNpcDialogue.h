#pragma once

#include <cstdint>
#include <limits>
#include <string>
#include <utility>
#include <vector>

namespace mir3_cui {

struct NpcDialogue {
    struct Row {
        std::string label;
        std::string command;
        unsigned color;
        int line = 0;
    };
    static constexpr unsigned DefaultColor = (std::numeric_limits<unsigned>::max)();
    std::uint32_t actor = 0;
    unsigned originX = 0, originY = 0;
    bool canClose = true;
    bool resetScroll = false;
    std::vector<Row> rows;
    int explicitBreaks = 0;
    Row prompt{};
    char answer[361]{};
    int editor = 0;
    bool answerTooLong = false;

    bool Apply(std::uint32_t id, const std::string& script)
    {
        const auto prefix = script.find('/');
        if (prefix == std::string::npos) return false;
        auto closeAllowed = id != actor || canClose;
        std::vector<Row> parsed;
        unsigned color = DefaultColor;
        int line = 0;
        std::size_t at = prefix + 1;
        while (at < script.size()) {
            const auto next = script.find_first_of("\\<{", at);
            if (next == std::string::npos) {
                parsed.push_back({script.substr(at), "", color, line});
                break;
            }
            if (next != at) parsed.push_back({script.substr(at, next - at), "", color, line});
            if (script[next] == '\\') {
                if (next == at && next > prefix + 1 && script[next - 1] == '\\') parsed.push_back({" ", "", color, line});
                ++line;
                at = next + 1;
                continue;
            }
            const bool link = script[next] == '<';
            const auto end = script.find(link ? '>' : '}', next + 1);
            if (end == std::string::npos) break;
            const auto tag = script.substr(next + 1, end - next - 1);
            const auto slash = tag.find('/');
            if (slash != std::string::npos) {
                const auto label = tag.substr(0, slash);
                const auto value = tag.substr(slash + 1);
                if (link && !label.empty()) parsed.push_back({label, value, color, line});
                else if (!link && label == "NOTCLOSE") closeAllowed = false;
                else if (!link && label == "FCOLOR" && !value.empty() && value.size() <= 2) {
                    unsigned index = 0;
                    bool valid = true;
                    for (const char digit : value) {
                        if (digit < '0' || digit > '9') { valid = false; break; }
                        index = index * 10 + digit - '0';
                    }
                    if (valid && index < 17) color = index;
                }
            }
            at = end + 1;
        }
        const auto nextEditor = editor + 1;
        *this = {};
        actor = id;
        canClose = closeAllowed;
        resetScroll = true;
        rows = std::move(parsed);
        explicitBreaks = line;
        editor = nextEditor;
        return true;
    }
};

} // namespace mir3_cui

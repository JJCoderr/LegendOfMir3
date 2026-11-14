#pragma once
#include <array>
#include <cstdint>
#include <optional>
#include <string_view>

namespace mir3 {
enum class ActionReply { unknown, good, fail };
inline ActionReply parse_action_reply(std::string_view text) {
    if (text.starts_with("+GOOD")) return ActionReply::good;
    if (text.starts_with("+FAIL")) return ActionReply::fail;
    return ActionReply::unknown;
}
struct ActionPrediction {
    struct Origin { std::array<int,2> cell; int facing; };
    std::optional<Origin> saved;
    bool waiting=false;
    std::uint32_t elapsed=0;
    void begin(std::array<int,2> cell,int facing) { saved=Origin{cell,facing};waiting=true;elapsed=0; }
    void confirm() { waiting=false; }
    std::optional<Origin> reject(bool dead) {
        if (dead || !saved) return {};
        waiting=false;elapsed=0;
        return saved;
    }
    void advance(std::uint32_t ms) {
        if (!waiting) return;
        if (std::uint64_t(elapsed)+ms>8000) { waiting=false;elapsed=0; }
        else elapsed+=ms;
    }
};
}

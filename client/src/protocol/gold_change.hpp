#pragma once
#include "receive_message.hpp"
#include "message_ids.hpp"
#include <bit>
namespace mir3 {
struct GoldChange {
    std::uint32_t balance=0;
    // 42AE12先做DWORD减法，42AE14另行比较结果与零，溢出按32位回绕。
    std::int32_t gain(std::uint32_t previous) const { return std::bit_cast<std::int32_t>(balance-previous); }
    static std::optional<GoldChange> parse(const ReceiveHeader& header) {
        if (header.opcode!=protocol::SM_GOLDCHANGED) return {};
        return GoldChange{header.recognition};
    }
    std::string caption(std::string text,std::uint32_t previous) const {
        const auto at=text.find("%d");
        if (at!=std::string::npos) text.replace(at,2,std::to_string(gain(previous)));
        return text;
    }
};
}

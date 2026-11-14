#pragma once
#include "receive_message.hpp"
#include "message_ids.hpp"
#include <string_view>
namespace mir3 {
// 原版4282C0；参考旧Godot Gameplay::receive_local_experience的独立功能。
struct ActorExperience {
    std::uint32_t total=0;
    std::uint16_t gained=0,training=0;
    bool replace_training=false,extra_notice=false;
    static std::optional<ActorExperience> parse(const ReceiveHeader& header) {
        if (header.opcode!=protocol::SM_WINEXP) return {};
        return ActorExperience{header.recognition,header.parameters[0],header.parameters[1],
            (header.parameters[2]&255)!=0,(header.parameters[2]>>8)!=0};
    }
    std::uint16_t apply_training(std::uint16_t previous) const {
        if (!training) return previous;
        return replace_training?training:std::uint16_t(previous+training);
    }
    int message_id() const { return training?1459:1460; }
    std::string caption(std::string_view pattern) const {
        std::string result(pattern);
        const std::array<unsigned,2> values{gained,training};
        std::size_t begin=0;
        for (int i=0;i<(training?2:1);++i) {
            const auto at=result.find("%d",begin);
            if (at==std::string::npos) break;
            const auto value=std::to_string(values[i]);result.replace(at,2,value);begin=at+value.size();
        }
        return result;
    }
};
}

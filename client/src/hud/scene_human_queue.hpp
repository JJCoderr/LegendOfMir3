#pragma once
#include <cstdint>
#include <deque>
#include <optional>
#include "human_appearance.hpp"
namespace mir3 {
// Decoded subset of the original actor queue (0x481A30 / 0x413F90).
// Wire header/body decoding and appearance updates remain in the transport layer.
struct SceneHumanMessage {
    int opcode=0,x=0,y=0,facing=0,hit_weapon=0;
    std::uint32_t flags=0;
    std::optional<HumanAppearance> appearance;
    static bool accepts(int opcode) {
        switch (opcode) {
        case 7: case 10: case 11: case 13: case 14: case 15: case 32: case 34: case 1316: case 1321:
        case 1306: case 1307: case 1308: case 1309: case 1319: return true;
        default: return false;
        }
    }
};
class SceneHumanQueue {
public:
    bool push(SceneHumanMessage message) {
        if (!SceneHumanMessage::accepts(message.opcode) || message.facing<0 || message.facing>=8 ||
            message.hit_weapon<0 || message.hit_weapon>=50 || pending.size()>=99999) return false;
        pending.push_back(message);
        return true;
    }
    bool catching_up() const { return pending.size()>=3; }
    std::size_t size() const { return pending.size(); }
    std::optional<SceneHumanMessage> take() {
        if (pending.empty()) return {};
        const auto message=pending.front();
        pending.pop_front();
        return message;
    }
private:
    std::deque<SceneHumanMessage> pending;
};
}

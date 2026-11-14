#pragma once
#include <cstdint>
#include <span>
#include <string>
#include <vector>
namespace mir3 {
// 0x47C590 / 0x4043B0：接收块剔除心跳，再按 #...! 拼接完整消息。
struct LoginStreamResult {
    bool heartbeat=false,overflow=false;
    std::vector<std::string> messages;
};
class LoginStream {
    std::string partial;
public:
    void clear() { partial.clear(); }
    LoginStreamResult receive(std::span<const std::uint8_t> bytes) {
        LoginStreamResult result;
        for (const auto value:bytes) {
            if (value=='*') { result.heartbeat=true;continue; }
            if (value=='#') { partial="#";continue; }
            if (partial.empty()) continue;
            if (value=='!') {
                result.messages.push_back(partial.substr(1));partial.clear();continue;
            }
            if (partial.size()>=65536) {
                partial.clear();result.overflow=true;continue;
            }
            partial+=char(value);
        }
        return result;
    }
};
}

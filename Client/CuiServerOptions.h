#pragma once

#include <cstddef>
#include <cstdint>
#include <string>

namespace mir3_cui {

constexpr unsigned ClientOptions = 1401;

struct ServerOption {
    const char* label;
    std::uint32_t mask;
    const char* command;
};

// These are protocol identifiers carried as GBK text in CM_SAY.
constexpr ServerOption ServerOptionList[] = {
    {"Block resurrection", 0x40000000u, "@\xbe\xdc\xbe\xf8\xbb\xd8\xc9\xfa\xca\xf5"},
    {"Block group teleport", 0x80000000u, "@\xbe\xdc\xbe\xf8\xcc\xec\xb5\xd8\xba\xcf\xd2\xbb"},
    {"Block trades", 0x08000000u, "@\xbe\xdc\xbe\xf8\xbd\xbb\xd2\xd7"},
    {"Block private messages", 0x04000000u, "@\xbe\xdc\xbe\xf8\xcb\xbd\xc1\xc4"},
    {"Block guild chat", 0x02000000u, "@\xbe\xdc\xbe\xf8\xd0\xd0\xbb\xe1\xc1\xc4\xcc\xec"},
    {"Allow joining a guild", 0x01000000u, "@\xbc\xd3\xc8\xeb\xd0\xd0\xbb\xe1"},
    {"Block all chat", 0x00800000u, "@\xbe\xdc\xbe\xf8\xcb\xf9\xd3\xd0\xc8\xcb\xc1\xc4\xcc\xec"}
};
constexpr std::size_t ServerOptionCount = sizeof ServerOptionList / sizeof *ServerOptionList;
constexpr char AttackModeCommand[] = "@\xb9\xa5\xbb\xf7\xb7\xbd\xca\xbd";

struct ServerOptions {
    std::uint32_t confirmed = 0;
    std::uint32_t displayed = 0;
    std::uint16_t attackMode = 0;
    bool attackModePending = false;
    bool open = false;
    std::string error;

    void Request(const ServerOption& option)
    {
        displayed = (displayed & ~option.mask) | (~confirmed & option.mask);
    }

    std::uint32_t Apply(std::uint32_t flags, std::uint16_t mode)
    {
        const auto changed = confirmed ^ flags;
        confirmed = flags;
        displayed = (displayed & ~changed) | (flags & changed);
        attackMode = mode;
        attackModePending = false;
        return changed;
    }
};

} // namespace mir3_cui

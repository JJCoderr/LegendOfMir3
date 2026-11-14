#pragma once
#include <cstdint>
namespace mir3::protocol {
enum MessageId : std::uint16_t {
#define MIR3_MESSAGE(name,value) name=value,
#include "message_ids.inc"
#undef MIR3_MESSAGE
};
}

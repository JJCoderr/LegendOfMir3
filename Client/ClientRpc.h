#pragma once
#include <string>
namespace mir3 {
std::string ClientState();
std::string ClientRpc(const std::string& method, const std::string& params);
}

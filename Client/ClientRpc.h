#pragma once
#include <string>
namespace mir3 {
std::string ClientState();
void ClientRpcTick();
void DrawClientTestStep();
void ClientRpcPacket(const char* direction,int opcode,int bytes);
std::string ClientRpc(const std::string& method, const std::string& params);
}

#pragma once

#include "../../Shared/DebugState.h"
#include <iphlpapi.h>
#include <vector>

#pragma comment(lib, "iphlpapi.lib")

namespace mir3 {

inline std::string ServiceTcpState()
{
    ULONG size = 0;
    if (GetTcpTable2(nullptr, &size, FALSE) != ERROR_INSUFFICIENT_BUFFER)
        return ",\"tcp_available\":false";
    std::vector<DWORD> buffer((size + sizeof(DWORD) - 1) / sizeof(DWORD));
    auto table = reinterpret_cast<PMIB_TCPTABLE2>(buffer.data());
    if (GetTcpTable2(table, &size, FALSE) != NO_ERROR)
        return ",\"tcp_available\":false";

    const auto pid = GetCurrentProcessId();
    std::string ports, peers;
    for (DWORD i = 0; i < table->dwNumEntries; ++i) {
        const auto& row = table->table[i];
        if (row.dwOwningPid != pid) continue;
        if (row.dwState == MIB_TCP_STATE_LISTEN) {
            if (!ports.empty()) ports += ',';
            ports += std::to_string(ntohs(static_cast<u_short>(row.dwLocalPort)));
        } else if (row.dwState == MIB_TCP_STATE_ESTAB) {
            if (!peers.empty()) peers += ',';
            peers += '[' + std::to_string(ntohs(static_cast<u_short>(row.dwLocalPort)))
                + ',' + std::to_string(ntohs(static_cast<u_short>(row.dwRemotePort))) + ']';
        }
    }
    return ",\"tcp_available\":true,\"listening_ports\":[" + ports
        + "],\"established_port_pairs\":[" + peers + ']';
}

}

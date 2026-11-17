"""Probe a local test GameSvr: invalid user indexes must not crash its worker.

Run against a deliberately started local integration server, not production.
The probe opens its own gate connection and creates no player/session records.
"""
import argparse
from pathlib import Path
import re
import socket
import struct


PROTOCOL = Path(__file__).resolve().parents[1] / "Def/Protocol.h"
HEADER = struct.Struct("<IiHHHHi")
GATE_SIGNATURE = 0xAA55AA55
INVALID_USER_INDEX = 0xFFFF


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--port", type=int, default=5000)
    args = parser.parse_args()
    definitions = dict(re.findall(r"^#define\s+(GM_\w+)\s+(\d+)",
                                 PROTOCOL.read_text(encoding="cp936"), re.M))

    def packet(name, index=0):
        return HEADER.pack(GATE_SIGNATURE, 0, 0, int(definitions[name]), index, 0, 0)

    with socket.create_connection(("127.0.0.1", args.port), timeout=3) as connection:
        for name in ("GM_DATA", "GM_CLOSE"):
            connection.sendall(packet(name, INVALID_USER_INDEX))
        connection.sendall(packet("GM_CHECKCLIENT"))
        response = b""
        while len(response) < HEADER.size:
            part = connection.recv(HEADER.size - len(response))
            if not part:
                raise AssertionError("GameSvr closed the probe connection")
            response += part
        header = HEADER.unpack(response)
        assert header[0] == GATE_SIGNATURE, header
        assert header[3] == int(definitions["GM_CHECKSERVER"]), header
    print("PASS: invalid GM_DATA/GM_CLOSE indexes rejected; same connection remains responsive")


if __name__ == "__main__":
    main()

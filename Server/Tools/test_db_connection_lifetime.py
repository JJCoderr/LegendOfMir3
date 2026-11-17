"""Exercise queued read requests while local GameSvr-to-DBSvr peers disconnect.

Uses only DB_LOADHUMANRCD for a missing test character; no database writes.
"""
import argparse
from pathlib import Path
import re
import socket
import struct


def encode(data):
    accumulator = bits = 0
    result = bytearray()
    for value in data:
        accumulator = (accumulator << 8) | value
        bits += 8
        while bits >= 6:
            bits -= 6
            result.append(((accumulator >> bits) & 63) + 60)
    if bits:
        result.append(((accumulator << (6 - bits)) & 63) + 60)
    return bytes(result)


def decode(data):
    accumulator = bits = 0
    result = bytearray()
    for value in data:
        accumulator = (accumulator << 6) | (value - 60)
        bits += 6
        if bits >= 8:
            bits -= 8
            result.append((accumulator >> bits) & 255)
    return bytes(result)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--port", type=int, default=6000)
    args = parser.parse_args()
    source = (Path(__file__).resolve().parents[1] / "Def/Protocol.h").read_text(encoding="cp936")
    definitions = dict(re.findall(r"^#define\s+(DB\w+)\s+(\d+)", source, re.M))
    header = encode(struct.pack("<iHHHH", 1, int(definitions["DB_LOADHUMANRCD"]), 0, 0, 0))
    body = encode(struct.pack("<16s20s15si", b"probe", b"__missing_probe__", b"127.0.0.1", 0))
    checksum_size = 6
    checksum = encode(struct.pack("<HH", 0xAA, len(header) + len(body) + checksum_size))
    request = b"#0/" + header + body + checksum + b"!"
    for _ in range(20):
        with socket.create_connection(("127.0.0.1", args.port), timeout=3) as connection:
            connection.sendall(request * 8)
            # Close without waiting: queued reads must retain their connection object.
    with socket.create_connection(("127.0.0.1", args.port), timeout=5) as connection:
        connection.sendall(request)
        response = b""
        while b"!" not in response:
            part = connection.recv(4096)
            if not part:
                raise AssertionError("DBSvr closed the final read connection")
            response += part
        payload = response.split(b"/", 1)[1]
        result = struct.unpack("<iHHHH", decode(payload[:16]))
        assert result[0] == 1 and result[1] == int(definitions["DBR_FAIL"]), result
    print("PASS: 160 queued reads across 20 disconnects; final missing-character response received")


if __name__ == "__main__":
    main()

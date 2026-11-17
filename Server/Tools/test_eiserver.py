"""Smoke test the real console host: listeners, GUI isolation, and login protocol."""
import argparse
import ctypes
from ctypes import wintypes
import socket
import struct
import subprocess
import time
from pathlib import Path

def encode(data):
    accumulator = bits = 0
    output = bytearray()
    for value in data:
        accumulator = (accumulator << 8) | value
        bits += 8
        while bits >= 6:
            bits -= 6
            output.append(((accumulator >> bits) & 63) + 60)
    if bits:
        output.append(((accumulator << (6 - bits)) & 63) + 60)
    return bytes(output)

def decode(data):
    accumulator = bits = 0
    output = bytearray()
    for value in data:
        accumulator = (accumulator << 6) | (value - 60)
        bits += 6
        if bits >= 8:
            bits -= 8
            output.append((accumulator >> bits) & 255)
    return bytes(output)

def exchange(connection, sequence, opcode, parameter, body):
    connection.sendall(b"#" + str(sequence).encode() +
        encode(struct.pack("<iHHHH", 0, opcode, parameter, 0, 0)) + encode(body) + b"!")
    data = b""
    while b"!" not in data:
        part = connection.recv(8192)
        if not part:
            raise RuntimeError("Connection closed before reply")
        data += part
    frame = data.split(b"#", 1)[1].split(b"!", 1)[0]
    return struct.unpack("<iHHHH", decode(frame[:16])), decode(frame[16:])

def assert_no_windows(pid):
    windows = []
    user32 = ctypes.windll.user32
    callback_type = ctypes.WINFUNCTYPE(wintypes.BOOL, wintypes.HWND, wintypes.LPARAM)
    @callback_type
    def visit(window, _):
        owner = wintypes.DWORD()
        user32.GetWindowThreadProcessId(window, ctypes.byref(owner))
        if owner.value == pid:
            windows.append(window)
        return True
    user32.EnumWindows(visit, 0)
    assert not windows, f"Service created top-level windows: {windows}"

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("directory", type=Path)
    args = parser.parse_args()
    folder = args.directory.resolve()
    with (folder / "eiserver-smoke.log").open("wb") as log:
        process = subprocess.Popen([str(folder / "EIServer.exe")], stdout=log, stderr=log,
            creationflags=subprocess.CREATE_NO_WINDOW)
        try:
            deadline = time.monotonic() + 60
            while time.monotonic() < deadline:
                assert process.poll() is None, f"Host exited: {process.returncode}"
                if b"All services ready" in (folder / "eiserver-smoke.log").read_bytes():
                    break
                time.sleep(0.2)
            else:
                raise AssertionError("Startup timed out")
            assert_no_windows(process.pid)
            with socket.create_connection(("127.0.0.1", 7000), 5) as connection:
                header, _ = exchange(connection, 1, 2001, 0, b"mirtest/test123")
                assert header[1] == 529, header
                header, _ = exchange(connection, 2, 104, 1, b"Mir3Local")
                assert header[1] == 530, header
                certification = header[2]
            with socket.create_connection(("127.0.0.1", 7100), 5) as connection:
                header, _ = exchange(connection, 1, 100, 0, b"mirtest/" + str(certification).encode())
                assert header[1] == 520, header
            command = f"(Get-NetTCPConnection -State Listen -OwningProcess {process.pid}).LocalPort | ConvertTo-Json -Compress"
            import json
            ports = json.loads(subprocess.check_output(["powershell", "-NoProfile", "-Command", command]))
            assert {5000, 5100, 5500, 6000, 7000, 7100, 7200} <= set(ports), ports
            print(f"PASS: PID {process.pid}, seven listeners, no top-level windows, login/select/list")
        finally:
            if process.poll() is None:
                process.terminate()
            process.wait(timeout=10)

if __name__ == "__main__":
    main()

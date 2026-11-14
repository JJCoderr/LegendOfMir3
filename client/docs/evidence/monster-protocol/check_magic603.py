"""只在内存中解析原始资源表，记录603缺项；不提取资源文件。"""
import hashlib
import json
import re
from pathlib import Path

source = Path("G:/游戏/宝藏传奇3客户端/MInfo.dat")
raw = source.read_bytes()
key = (0xF0, 0x39, 0xAB, 0x8E)
length = int.from_bytes(bytes(raw[i] ^ key[i] for i in range(4)), "big")
assert length == len(raw) - 8
checksum = sum(i * (value + 1) for i, value in enumerate(raw[8:])) & 0xFFFFFFFF
assert checksum ^ (-1612834157 & 0xFFFFFFFF) == int.from_bytes(raw[4:8], "little")
plain = bytearray(raw[8:])
for j in range(3, -1, -1):
    for i in range(length):
        plain[i] ^= (raw[j] + i) & 255
rows = {}
section = ""
for line in plain.decode("latin1").split("\r\n"):
    if line.startswith("#"):
        section = line.split()[0]
        rows.setdefault(section, [])
    if "[" in line and "]" in line:
        # 与原版strtol一致：每个逗号字段只转换开头整数，末尾无逗号字段不转换。
        fields = line.split("[", 1)[1].split("]", 1)[0].split(",")[:-1][:19]
        columns = [int(match.group()) if (match := re.match(r"\s*[+-]?\d+", value)) else 0 for value in fields]
        if len(columns) > 4 and columns[4] == 603:
            rows[section].append(columns)
result = {"source": str(source), "sha256": hashlib.sha256(raw).hexdigest(),
          "rows_603": rows, "magic_603_present": bool(rows.get("#MAGIC")),
          "ida": ["4495D0:46EF60 lookup, null returns 0", "46EF60:MAGIC table linear search, no SPELL fallback"],
          "scope": "disk resource and static initializer; runtime extension patching not verified"}
Path(__file__).with_name("magic603-resource-proof.json").write_text(json.dumps(result, ensure_ascii=False, indent=2)+"\n", encoding="utf8")
print("MAGIC603 present:", result["magic_603_present"], "SHA256:", result["sha256"])

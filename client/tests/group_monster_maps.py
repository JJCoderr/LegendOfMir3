"""按 Mapinfo 的区域入口与地图系列归类；不再使用刷怪文件名推断区域。"""
import json
import re
from pathlib import Path

PATH = Path(__file__).with_name("monster_maps.json")
# 区域边界为展示规则；洞穴归属依据同包 Mapinfo.txt 的入口/NOReConnect。
REGIONS = [
    ("bichon", "比奇地区", r"0|01|02|D00\d.*|D01\d.*|D4[01]\d.*|D400_.*|D13\d.*|D14\d.*|E00[12]|E402"),
    ("dogwan", "道馆·沃玛神殿", r"1|D0[2345]\d.*"),
    ("phantom", "灌木林·赤月山谷", r"12|122|125|D10\d.*"),
    ("snake", "毒蛇山谷", r"2|D4[2345]\d.*|D420_.*"),
    ("desert", "沙漠·绿洲", r"4|42|43|44|5|6|71|72|73|76|77|78|D8\d.*"),
    ("noma", "诺玛地区", r"41|D16\d.*"),
    ("mongchon", "盟重·石阁·虫谷", r"74|75|D6\d.*|D7\d.*"),
    ("panya", "潘夜岛", r"8|D11\d.*|D121\d.*"),
    ("exile", "流放岛·罪孽洞穴", r"D120\d.*"),
    ("paradise", "失乐园·真天宫", r"9|D15\d.*"),
    ("zuma", "祖玛神殿", r"D5\d.*"),
    ("ship", "神舰", r"D9\d.*"),
    ("special", "特殊任务区域", r"B\d.*"),
]

def main():
    data = json.loads(PATH.read_text(encoding="utf-8"))
    groups = {}
    for row in data["maps"]:
        matches = [(key, name) for key, name, pattern in REGIONS
                   if re.fullmatch(pattern, row["map"], re.IGNORECASE)]
        if len(matches) != 1:
            raise ValueError(f"地图必须且只能归属一个区域: {row['map']} {matches}")
        row["family"], row["family_name"] = matches[0]
        groups.setdefault(row["family_name"], []).append(row["map"])
    data["grouping_source"] = "group_monster_maps.py: explicit display regions; same-package Mapinfo entrances and dungeon series"
    PATH.write_text(json.dumps(data, ensure_ascii=False, indent=2)+"\n", encoding="utf-8")
    for name, maps in groups.items():
        print(f"{name}: {len(maps)} maps")

if __name__ == "__main__":
    main()

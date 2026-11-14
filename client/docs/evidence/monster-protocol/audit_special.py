"""生成逐外观待验收清单，不把发现分支或存在实现视为验收通过。"""
import json
import re
from pathlib import Path

ROOT = Path(__file__).resolve().parent
PROJECT = ROOT.parents[3]
catalog = json.loads((PROJECT / "client/tests/monster_cases.json").read_text(encoding="utf-8"))
# 外层case缩进和shape偏移已经人工核对；嵌套方向/随机分支不计为外观号。
switches = {
    "attack_effect": ("0x40CBF0.c", 6, 0),
    "ranged_attack": ("0x40B0A0.c", 4, 7),
    "death_effect": ("0x408c10.c", 4, 0),
}
groups = {}
for category, (filename, indent, offset) in switches.items():
    source = (ROOT / filename).read_text(encoding="utf-8")
    groups[category] = {int(match, 0)+offset for match in re.findall(
        rf"^ {{{indent}}}case (0x[0-9A-Fa-f]+|[0-9]+)[uU]?:", source, re.M)}
groups["birth_or_hide"] = {3,4,31,49,56,83,84,85,89,90,530,531,532,533,538}
groups["struck_effect"] = {2,67,104,150,161}
rows = []
for monster in catalog["monsters"]:
    if monster["alias"]:
        continue
    shape = monster["shape"]
    row = {"shape":shape, "name":monster["name"],
           "branch_categories":[name for name, shapes in groups.items() if shape in shapes],
           "status":"pending_full_review", "verified":[], "pending":[
               "birth_idle", "movement_direction", "attacks", "struck", "death_remove", "server_behavior"]}
    if shape in (83,84,85):
        row["verified"] = ["dormant_entry", "wake_animation", "dormant_restore"]
        row["test"] = "monsters.gd/check_zuma_lifecycle; monster-zuma-wire-v2.log"
        row["reference"] = "LegendOfMir3_Src/ObjectMonster.cpp:CScultureObject,CScultureKingObject; Zircon/ZumaGuardian.cs,ZumaKing.cs"
    if shape in groups["birth_or_hide"]:
        row["verified"].append("digup_survives_digdown_removes_or_gate_pose")
        row["burrow_test"] = "monsters.gd/check_burrow_lifecycle; monster-burrow-lifecycle.log (14 cases), monster-burrow-538.log (direction 0)"
        if shape==538:
            row["pending"].append("direction_6_entry_resource_rejected")
    rows.append(row)
output = {"scope":"1.45 catalog, excludes resource aliases",
          "warning":"branch_categories is an initial inventory, not an exhaustive proof; conditions, resource tables, subclass dispatch and server rules still require review",
          "functions":switches, "monsters":rows}
(ROOT / "special-audit.json").write_text(json.dumps(output,ensure_ascii=False,indent=2)+"\n",encoding="utf-8")
print(f"Recorded {len(rows)} appearances; all remain pending full review")

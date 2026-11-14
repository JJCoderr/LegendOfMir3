"""审计地图展示产物；区分遍历完成、行为观察与资源/映射缺项。"""
import argparse
import json
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]


def audit(report, catalog, log):
    expected = {row["family_name"] for row in catalog["maps"]}
    observed = [group["family"] for group in report]
    errors, gaps, unmoving = [], [], []
    if set(observed) != expected or len(observed) != len(set(observed)):
        errors.append("区域报告缺失、重复或出现未知区域")
    if f"MONSTER_MAPS_OBSERVED groups={len(expected)}" not in log:
        errors.append("日志缺少全部区域完成标记")
    previous_skill = None
    delays = []
    for group in report:
        skills = []
        gaps.extend({"family": group["family"], **item} for item in group["missing"])
        for actor in group["actors"]:
            identity = f'{group["family"]}/{actor["shape"]}'
            for key in ("attack_accepted", "struck_accepted", "death_accepted"):
                if not actor.get(key):
                    errors.append(f"{identity}: {key}未通过")
            if actor.get("death_frame", -1) < 0:
                errors.append(f"{identity}: 未观察到尸体")
            delay = actor.get("death_sent_after_ms")
            if delay is None or not 0 <= delay <= 2000:
                errors.append(f"{identity}: 受击至死亡发包{delay}ms超过要求")
            else:
                delays.append(delay)
            skill = actor.get("hit_skill")
            if skill is None or skill == previous_skill:
                errors.append(f"{identity}: 受击技能缺失或连续重复")
            previous_skill = skill
            skills.append(skill)
            if not actor.get("walked"):
                unmoving.append({"actor": identity, "reason": actor.get("walk_unavailable_reason", "not_observed")})
                if actor.get("behavior", {}).get("visible_walk_directions"):
                    errors.append(f"{identity}: 有完整行走资源但未观察到移动")
        if len(skills) <= 61 and len(set(skills)) != len(skills):
            errors.append(f'{group["family"]}: 同轮技能池耗尽前重复')
    return {"regions": len(report), "expected_regions": len(expected),
            "actor_instances": sum(len(group["actors"]) for group in report),
            "errors": errors, "unavailable": gaps, "unmapped_names": catalog["mapping_unresolved"],
            "not_observed_walking": unmoving,
            "death_delay_ms": [min(delays), max(delays)] if delays else [],
            "traversal_verified": not errors,
            "complete_monster_system": False,
            "scope": "区域遍历、消息接受、可见行走资源、死亡和随机技能；不证明全部攻击视觉、特殊AI或外观覆盖"}


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--log", type=Path, required=True)
    args = parser.parse_args()
    result = audit(json.loads((ROOT / "build/monster-map-report.json").read_text(encoding="utf8")),
                   json.loads((ROOT / "tests/monster_maps.json").read_text(encoding="utf8")),
                   args.log.read_text(encoding="utf8", errors="replace"))
    output = ROOT / "build/monster-showcase-audit.json"
    output.write_text(json.dumps(result, ensure_ascii=False, indent=2)+"\n", encoding="utf8")
    print(json.dumps({key: value for key, value in result.items() if key not in ("unavailable", "unmapped_names")}, ensure_ascii=False))
    print("unavailable:", len(result["unavailable"]), "unmapped names:", len(result["unmapped_names"]))
    raise SystemExit(bool(result["errors"]))

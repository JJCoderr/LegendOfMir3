"""客户端唯一测试入口：子系统使用子命令，技能实现统一在 skills.gd。

使用示例（仓库根目录）：
  python client/tests/run_tests.py skills --screenshots  # 技能全集及截图
  python client/tests/run_tests.py skills --skill 47  # 单技能的适用场景
  python client/tests/run_tests.py skills --skills 3,47,48,410  # 混选、去重
  python client/tests/run_tests.py skills common --skills 47,48  # 仅共用施法场景
  python client/tests/run_tests.py skills --list  # 场景与技能编号
  python client/tests/run_tests.py equipment  # 装备全集
  python client/tests/run_tests.py map-events  # 地图事件全集

--godot/--checks 指定程序；也可设置 MIR3_GODOT_EXECUTABLE。
技能图形用例在同一进程运行，背景音乐持续播放，技能声效保留。
无筛选运行全集；调试只选受影响技能。共享模型按场景检查，不等于逐技能图形复测。
入口使用参数数组，适配不同平台路径；Linux 实际验收仍需在对应平台执行。
"""
import argparse
import json
import os
from pathlib import Path
import shutil
import subprocess
import sys

ROOT = Path(__file__).resolve().parents[1]
# 每项只登记已有验收，避免把尚未实现的用例当作通过。
COMMON_CASES = json.loads((ROOT / "tests" / "skill_common_cases.json").read_text(encoding="utf-8"))["cases"]
COMMON_SKILLS = tuple(int(c["id"]) for c in COMMON_CASES)
IMPACT_SKILLS = tuple(int(c["id"]) for c in COMMON_CASES if c["projectile"] and c["explosion"] is not None)
STATIONARY_SKILLS = tuple(int(c["id"]) for c in COMMON_CASES if not c["projectile"] and c["id"] not in (36,74,104,107,108))
SUITES = {
    "skills": {
        "models": (["--spell", "--skill-wire", "--action-reply"], None),
        "bindings": ([], "skills.gd"),
        "casting": ([], "skills.gd"),
        "enchantment": (["--spell-weapon-color"], "skills.gd"),
        "enchantment-visual": ([], "skills.gd"),
        "correction": ([], "skills.gd"),
        "cast-correction": ([], "skills.gd"),
        "ranges": ([], "skills.gd"),
        "lightning-model": (["--spell-lightning"], None),
        "common": (["--spell-common"], "skills.gd"),
        "passive": ([], "skills.gd"),
        "special": (["--spell-special"], "skills.gd"),
        "impact": ([], "skills.gd"),
        "target-loss": ([], "skills.gd"),
        "stationary": ([], "skills.gd"),
        "melee": (["--spell-melee"], "skills.gd"),
        "particle-model": (["--spell-particles"], None),
        "sound": (["--spell-sound"], "skills.gd"),
    },
    "map-events": {"effects": ([], "map_events.gd")},
    "equipment": {
        "wire": (["--equipment-wire"], "equipment_wire.gd"),
        "actions": (["--equipment-actions"], "equipment_actions.gd"),
    },
}
SKILLS = (1, 3, 4, 7, 9, 10, 12, 25, 26, 27, 34, 35, 103, 53, 73, 110, 113, 150, *range(408, 415), *COMMON_SKILLS)


def executable(explicit, candidates):
    """优先使用显式路径，再查构建目录和PATH；不拼接shell命令。"""
    for name in ([explicit] if explicit else candidates):
        path = Path(name).expanduser()
        if path.is_file():
            return str(path.resolve())
        found = shutil.which(str(name))
        if found:
            return found
    raise FileNotFoundError(f"找不到可执行程序：{explicit or candidates}")


def skill_cases(skill):
    """按技能编号归集所有适用场景；全集仍由 SUITES 统一登记。"""
    allowed = ["casting"] + (["enchantment", "enchantment-visual"] if skill >= 408 else [])
    if skill in (3, 4):
        allowed = ["passive"]
    if skill in COMMON_SKILLS:
        allowed = ["common"]
    if skill in (5,13,37,38,39,40,41,67):
        allowed.append("target-loss")
    if skill in STATIONARY_SKILLS:
        allowed.append("stationary")
    if skill in IMPACT_SKILLS:
        allowed.append("impact")
    if skill in (36, 74, 104, 107, 108):
        allowed.append("special")
    if skill in (7, 12, 25, 26, 34, 35, 103):
        allowed = ["melee"]
    if skill == 1:
        allowed.append("cast-correction")
    if skill in (9, 10, 53):
        allowed = ["ranges"]
        if skill == 10:
            allowed.insert(0, "lightning-model")
    if skill in (1, 53):
        allowed.append("sound")
    return allowed


BATCH_CASES = {"common", "enchantment", "melee", "special", "impact", "target-loss", "stationary"}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    subcommands = parser.add_subparsers(dest="suite", required=True, help="选择子系统测试集")
    for name, scenarios in SUITES.items():
        command = subcommands.add_parser(name, help=f"{name}测试全集")
        command.add_argument("scenario", nargs="?", choices=scenarios, help="可选场景子命令；省略时运行全部适用场景")
        command.add_argument("--skill", type=int, choices=SKILLS, help="仅运行对应技能")
        command.add_argument("--skills", help="技能编号集合，逗号分隔；可与场景子命令组合，不指定场景时自动归集")
        command.add_argument("--screenshots", action="store_true", help="保存真实视口截图")
        command.add_argument("--godot", default=os.environ.get("MIR3_GODOT_EXECUTABLE"))
        command.add_argument("--checks", help="hud_checks程序的路径")
        command.add_argument("--build-dir", type=Path, default=ROOT / "build")
        command.add_argument("--config", default="Release", help="多配置生成器的构建配置")
        command.add_argument("--list", action="store_true", help="列出已登记子集，不运行测试")
    args = parser.parse_args()
    suite = SUITES[args.suite]
    if args.list:
        print(f"{args.suite}: {', '.join(suite)}")
        if args.suite == "skills":
            print("skill: " + ", ".join(map(str, SKILLS)))
        return 0
    if args.skill is not None and args.suite != "skills":
        parser.error("--skill仅适用于skills测试集")
    cases = [args.scenario] if args.scenario else list(suite)
    if any(case not in suite for case in cases):
        parser.error(f"无效子集；可选：{', '.join(suite)}")
    selected = None
    if args.skill is not None and args.skills is not None:
        parser.error("--skill与--skills不能同时使用")
    if args.skills is not None:
        values = [value.strip() for value in args.skills.split(",")]
        if args.suite != "skills" or any(not value.isdecimal() or int(value) not in SKILLS for value in values):
            parser.error("--skills仅适用于skills，且必须包含有效技能编号")
        selected = list(dict.fromkeys(map(int, values)))
    elif args.skill is not None:
        selected = [args.skill]
    selected_by_case = {}
    if selected is not None:
        applicable = {skill: skill_cases(skill) for skill in selected}
        if args.scenario:
            if any(not any(case in applicable[skill] for skill in selected) for case in cases):
                parser.error("所选场景没有适用的技能")
            if any(not any(case in applicable[skill] for case in cases) for skill in selected):
                parser.error("所选技能没有适用的场景")
        else:
            cases = [case for case in suite if any(case in applicable[skill] for skill in selected)]
        selected_by_case = {case: [skill for skill in selected if case in applicable[skill]] for case in cases}
    cases = list(dict.fromkeys(cases))
    suffix = ".exe" if os.name == "nt" else ""
    checks = None
    godot = None
    if any(suite[case][0] for case in cases):
        checks = executable(args.checks, [args.build_dir / args.config / f"hud_checks{suffix}",
                                         args.build_dir / f"hud_checks{suffix}"])
    if any(suite[case][1] for case in cases):
        godot = executable(args.godot, ["godot", "godot4", "Godot_v4.7.2-stable_win64_console.exe"])
    skill_plan = []
    for case in cases:
        models, script = suite[case]
        # skills/models的--spell已包含颜色、闪电与声道模型，全量时不重复执行。
        commands = [[checks, model] for model in models
                    if not (args.suite == "skills" and "models" in cases and model in ("--spell-weapon-color", "--spell-lightning", "--spell-sound", "--spell-particles", "--spell-melee", "--spell-common", "--spell-special"))]
        if script:
            ids = selected_by_case.get(case)
            # 支持批量的场景只启动一次；其他场景分别传单技能，避免忽略筛选误跑全集。
            selections = [[]] if ids is None else (
                [["--skills=" + ",".join(map(str, ids))]] if case in BATCH_CASES
                else [[f"--skill={skill}"] for skill in ids])
            for selection in selections:
                command = [godot, "--path", str(ROOT / "godot"), "--script", str(ROOT / "tests" / script), "--", *selection]
                if args.screenshots:
                    command.append("--screenshots")
                if args.suite == "skills":
                    skill_plan.append({"case": case, "args": command[command.index("--") + 1:]})
                else:
                    commands.append(command)
        for command in commands:
            print(f"RUN {args.suite}/{case}: {command}", flush=True)
            result = subprocess.run(command, cwd=ROOT)
            if result.returncode:
                print(f"FAILED {args.suite}/{case}: exit={result.returncode}", file=sys.stderr)
                return result.returncode if result.returncode > 0 else 1
    if skill_plan:
        # 所有技能图形用例合并在 skills.gd，一个进程执行计划，背景音乐保持连续。
        command = [godot, "--path", str(ROOT / "godot"), "--script", str(ROOT / "tests" / "skills.gd"), "--", "--plan=" + json.dumps(skill_plan)]
        if args.screenshots:
            command.append("--screenshots")
        print(f"RUN skills: {command}", flush=True)
        result = subprocess.run(command, cwd=ROOT)
        if result.returncode:
            print(f"FAILED skills: exit={result.returncode}", file=sys.stderr)
            return result.returncode if result.returncode > 0 else 1
    scope = "SUBSET" if args.scenario or selected is not None else "ALL"
    print(f"TEST_SUITE_{scope}_OK suite={args.suite} cases={','.join(cases)} skills={selected}")
    return 0


if __name__ == "__main__":
    try:
        sys.exit(main())
    except (OSError, KeyboardInterrupt) as error:
        print(f"测试入口失败：{error}", file=sys.stderr)
        sys.exit(1)

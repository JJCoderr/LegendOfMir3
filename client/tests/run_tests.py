"""客户端测试总入口：按子系统或单项筛选；无筛选时运行该子系统全集。"""
import argparse
import os
from pathlib import Path
import shutil
import subprocess
import sys

ROOT = Path(__file__).resolve().parents[1]
# 每项只登记已有验收，避免把尚未实现的用例当作通过。
SUITES = {
    "skills": {
        "models": (["--spell", "--skill-wire", "--action-reply"], None),
        "bindings": ([], "skill_wire.gd"),
        "casting": ([], "skill_casting.gd"),
        "enchantment": (["--spell-weapon-color"], "weapon_enchantment.gd"),
        "correction": ([], "forced_motion.gd"),
    },
    "equipment": {
        "wire": (["--equipment-wire"], "equipment_wire.gd"),
        "actions": (["--equipment-actions"], "equipment_actions.gd"),
    },
}
SKILLS = (1, 27, 73, 110, 113, 150, *range(408, 415))


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


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--suite", choices=SUITES, default="skills", help="子系统测试集")
    parser.add_argument("--case", action="append", help="子集名称，可重复指定；--list查看")
    parser.add_argument("--skill", type=int, choices=SKILLS, help="仅运行对应技能")
    parser.add_argument("--screenshots", action="store_true", help="保存真实视口截图")
    parser.add_argument("--godot", default=os.environ.get("MIR3_GODOT_EXECUTABLE"))
    parser.add_argument("--checks", help="hud_checks程序的路径")
    parser.add_argument("--build-dir", type=Path, default=ROOT / "build")
    parser.add_argument("--config", default="Release", help="多配置生成器的构建配置")
    parser.add_argument("--list", action="store_true", help="列出已登记子集，不运行测试")
    args = parser.parse_args()
    suite = SUITES[args.suite]
    if args.list:
        print(f"{args.suite}: {', '.join(suite)}")
        if args.suite == "skills":
            print("skill: " + ", ".join(map(str, SKILLS)))
        return 0
    if args.skill is not None and args.suite != "skills":
        parser.error("--skill仅适用于skills测试集")
    cases = args.case or list(suite)
    if any(case not in suite for case in cases):
        parser.error(f"无效子集；可选：{', '.join(suite)}")
    if args.skill is not None:
        allowed = ["casting"] + (["enchantment"] if args.skill >= 408 else [])
        if args.case and any(case not in allowed for case in cases):
            parser.error("所选子集不支持该技能编号筛选")
        cases = args.case or allowed
    cases = list(dict.fromkeys(cases))
    suffix = ".exe" if os.name == "nt" else ""
    checks = None
    godot = None
    if any(suite[case][0] for case in cases):
        checks = executable(args.checks, [args.build_dir / args.config / f"hud_checks{suffix}",
                                         args.build_dir / f"hud_checks{suffix}"])
    if any(suite[case][1] for case in cases):
        godot = executable(args.godot, ["godot", "godot4", "Godot_v4.7.2-stable_win64_console.exe"])
    for case in cases:
        models, script = suite[case]
        # skills/models的--spell已包含颜色模型，全量时不再重复执行。
        commands = [[checks, model] for model in models
                    if not (args.suite == "skills" and "models" in cases and model == "--spell-weapon-color")]
        if script:
            command = [godot, "--path", str(ROOT / "godot"), "--script", str(ROOT / "tests" / script), "--"]
            if args.skill is not None:
                command.append(f"--skill={args.skill}")
            if args.screenshots:
                command.append("--screenshots")
            commands.append(command)
        for command in commands:
            print(f"RUN {args.suite}/{case}: {command}", flush=True)
            result = subprocess.run(command, cwd=ROOT)
            if result.returncode:
                print(f"FAILED {args.suite}/{case}: exit={result.returncode}", file=sys.stderr)
                return result.returncode if result.returncode > 0 else 1
    scope = "SUBSET" if args.case or args.skill is not None else "ALL"
    print(f"TEST_SUITE_{scope}_OK suite={args.suite} cases={','.join(cases)} skill={args.skill}")
    return 0


if __name__ == "__main__":
    try:
        sys.exit(main())
    except (OSError, KeyboardInterrupt) as error:
        print(f"测试入口失败：{error}", file=sys.stderr)
        sys.exit(1)

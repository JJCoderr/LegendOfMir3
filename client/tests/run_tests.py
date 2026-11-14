"""客户端唯一测试入口：子系统使用子命令，技能实现统一在 skills.gd。

使用示例（仓库根目录）：
  python client/tests/run_tests.py skills --screenshots  # 技能全集及截图
  python client/tests/run_tests.py skills --skill 2 --MP4  # 治愈术MP4，默认build/videos
  python client/tests/run_tests.py skills --MP4 client/build/videos/skills.mp4  # 自定输出路径
  python client/tests/run_tests.py skills --skill 47  # 单技能的适用场景
  python client/tests/run_tests.py skills --skills 3,47,48,410  # 混选、去重
  python client/tests/run_tests.py skills common --skills 47,48  # 仅共用施法场景
  python client/tests/run_tests.py skills summon --skill 17  # 骷髅实体入场、位置更新、离场与清理
  python client/tests/run_tests.py skills persistent --skills 16,22  # 持续地面效果及火墙余烬
  python client/tests/run_tests.py skills --list  # 场景与技能编号
  python client/tests/run_tests.py equipment  # 装备全集
  python client/tests/run_tests.py map-events  # 地图事件全集

--godot/--checks 指定程序；也可设置 MIR3_GODOT_EXECUTABLE。
--MP4 可选录制：H.264 800×600/15fps，CRF24/1600kbps上限，AAC 48kbps；仅捕获游戏视口与混音。
需要PATH中的ffmpeg、--ffmpeg路径或python -m pip install imageio-ffmpeg；不启用则无录制开销。
输出默认build/videos，不覆盖已有文件；多场景子系统指定路径时按场景追加文件名。
技能图形用例在同一进程运行，背景音乐持续播放，技能声效保留。
统一从比奇地图 0 的 (413,410) 开始，各场景相对偏移保持一致。
画面左上显示技能名、编号和施法阶段；未知名称保留未命名，不猜测。
场景销毁间隔保留上一帧地图，逐场景核对过渡像素，避免灰屏；音乐仍连续播放。
无筛选运行全集；调试只选受影响技能。共享模型按场景检查，不等于逐技能图形复测。
入口使用参数数组，适配不同平台路径；Linux 实际验收仍需在对应平台执行。

本轮验收范围：Windows 客户端技能；用户已明确不要求原版同状态对照。
服务端、ASPack 和其他平台实机验证留在后续，不作为本轮完成条件。
后续结果审计：17/30/105 接入怪物实体生命周期；16/22 接入持续地图事件；
8 接收端退步、21/104 接收服务器传送落点。回包由离线夹具提供，不验证服务端判定。
105 使用包内 shape=32 验证外观驱动，不声称该技能固定召唤此形态。
召唤的移动阶段验证位置更新包，不代表已覆盖宠物 AI、攻击或服务器归属。
18/19 隐身、31 护盾、2/29 治疗、77 回生的现有声光测试不等于状态结果验收；
这些技能与状态/生命消息的对应关系仍需证据，不能根据技能名称伪造映射。
63/64/78/121 的抑制或忽略分发也不能视为完整玩法结果已验收。
本轮全集通过：client/build/skill-outcomes-bichon-full.log（18 个图形场景及共享模型）。
随后只保留动画结束等待，5/17 定向复测通过：client/build/skill-wait-subset.log。
"""
import argparse
import json
import os
from pathlib import Path
import shutil
import subprocess
import sys
import socket
import struct
import tempfile
import threading
from datetime import datetime

ROOT = Path(__file__).resolve().parents[1]
# 每项只登记已有验收，避免把尚未实现的用例当作通过。
COMMON_CASES = json.loads((ROOT / "tests" / "skill_common_cases.json").read_text(encoding="utf-8"))["cases"]
COMMON_SKILLS = tuple(int(c["id"]) for c in COMMON_CASES)
IMPACT_SKILLS = tuple(int(c["id"]) for c in COMMON_CASES if c["projectile"] and c["explosion"] is not None)
STATIONARY_SKILLS = tuple(int(c["id"]) for c in COMMON_CASES if not c["projectile"] and c["id"] not in (36,74,104,107,108))
SUITES = {
    "monsters": {"maps": ([], "monsters.gd"), "gallery": ([], "monsters.gd"), "regression": ([], "monsters.gd")},
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
        "summon": ([], "skills.gd"),
        "persistent": ([], "skills.gd"),
        "displacement": ([], "skills.gd"),
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
    if skill in (17,30,105):
        allowed.append("summon")  # 召唤必须覆盖怪物入场、移动、离场，不能只测施法回包。
    if skill in (16,22):
        allowed.append("persistent")  # 困魔咒/火墙持续事件及停止后的清理。
    if skill in (8,21,104):
        allowed.append("displacement")  # 受击位移、传送/换位后的实际位置。
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


BATCH_CASES = {"common", "enchantment", "melee", "special", "impact", "target-loss", "stationary", "summon", "persistent", "displacement"}


def run_graphics(command, args, label):
    """直接录制 Godot 视口与游戏混音，不抓桌面；流式编码避免保存大量截图。"""
    if args.MP4 is None:
        return subprocess.run(command, cwd=ROOT)
    encoder = args.ffmpeg or shutil.which("ffmpeg")
    if not encoder:
        try:
            import imageio_ffmpeg
            encoder = imageio_ffmpeg.get_ffmpeg_exe()
        except ImportError as error:
            raise OSError("MP4需要ffmpeg：使用--ffmpeg指定，或安装imageio-ffmpeg") from error
    output = (Path(args.MP4) if args.MP4 else args.build_dir / "videos" / f"{label}-{datetime.now():%Y%m%d-%H%M%S}.mp4").resolve()
    if args.MP4 and args.suite != "skills" and args.scenario is None:
        output = output.with_name(f"{output.stem}-{label}{output.suffix}")
    if output.suffix.lower() != ".mp4":
        raise OSError("--MP4输出路径必须以.mp4结尾")
    if output.exists():
        raise OSError(f"录制文件已存在，不覆盖：{output}")
    output.parent.mkdir(parents=True, exist_ok=True)
    failures = []
    sample_rate = [48000]
    with tempfile.TemporaryDirectory(prefix="mir3-mp4-",dir=output.parent) as temporary, socket.socket() as listener:
        folder = Path(temporary)
        video, audio = folder / "video.mp4", folder / "audio.f32"
        listener.bind(("127.0.0.1",0)); listener.listen(1); listener.settimeout(0.25)
        stop = threading.Event()
        # H.264上限约1.6Mb/s，加AAC音轨；800x600/15fps，约12MB/分钟以内。
        with (folder / "encoder.log").open("wb") as errors:
            process = subprocess.Popen([encoder,"-hide_banner","-loglevel","error","-f","image2pipe","-vcodec","mjpeg","-framerate","15","-i","pipe:0","-vf","scale=800:600:force_original_aspect_ratio=decrease,pad=800:600:(ow-iw)/2:(oh-ih)/2","-an","-c:v","libx264","-preset","veryfast","-crf","24","-maxrate","1600k","-bufsize","3200k","-pix_fmt","yuv420p",str(video)],stdin=subprocess.PIPE,stderr=errors)
            def receive():
                try:
                    connection = None
                    while not stop.is_set():
                        try:
                            connection,_ = listener.accept(); break
                        except socket.timeout:
                            pass
                    if connection is None:
                        raise OSError("游戏没有连接MP4录制通道")
                    with connection, audio.open("wb") as sound:
                        connection.settimeout(1)
                        def read_exact(count):
                            result = bytearray()
                            while len(result)<count:
                                try: chunk=connection.recv(min(count-len(result),65536))
                                except socket.timeout:
                                    if stop.is_set(): return None
                                    continue
                                if not chunk: return None
                                result.extend(chunk)
                            return result
                        while True:
                            header=read_exact(5)
                            if header is None: break
                            kind,length=struct.unpack("<BI",header)
                            if length>16*1024*1024: raise OSError("录制数据帧超限")
                            payload=read_exact(length)
                            if payload is None: raise OSError("录制数据帧不完整")
                            if kind==1: process.stdin.write(payload)
                            elif kind==2: sound.write(payload)
                            elif kind==3: sample_rate[0]=struct.unpack("<I",payload)[0]
                            else: raise OSError("无效录制数据类型")
                except Exception as error:
                    failures.append(str(error))
                finally:
                    process.stdin.close()
            worker=threading.Thread(target=receive,daemon=True); worker.start()
            try:
                result=subprocess.run([*command,f"--mp4-port={listener.getsockname()[1]}"],cwd=ROOT)
            finally:
                stop.set(); worker.join(5)
                if worker.is_alive():
                    process.kill(); worker.join(2)
                    failures.append("录制通道关闭超时")
                try: code=process.wait(timeout=30)
                except subprocess.TimeoutExpired:
                    process.kill(); process.wait(); raise OSError("MP4编码结束超时")
            if code or failures:
                errors.flush()
                raise OSError("MP4录制失败："+"; ".join(failures)+(folder / "encoder.log").read_text(errors="replace"))
        mux=[encoder,"-hide_banner","-loglevel","error","-n","-i",str(video)]
        if audio.exists() and audio.stat().st_size:
            mux += ["-f","f32le","-ar",str(sample_rate[0]),"-ac","2","-i",str(audio),"-c:a","aac","-b:a","48k","-ar","24000"]
        mux += ["-c:v","copy","-movflags","+faststart",str(output)]
        subprocess.run(mux,check=True)
    print(f"MP4: {output} ({output.stat().st_size/1024/1024:.2f} MiB)",flush=True)
    return result


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    subcommands = parser.add_subparsers(dest="suite", required=True, help="选择子系统测试集")
    for name, scenarios in SUITES.items():
        command = subcommands.add_parser(name, help=f"{name}测试全集")
        command.add_argument("scenario", nargs="?", choices=scenarios, help="可选场景子命令；省略时运行全部适用场景")
        command.add_argument("--skill", type=int, choices=SKILLS, help="仅运行对应技能")
        command.add_argument("--skills", help="技能编号集合，逗号分隔；可与场景子命令组合，不指定场景时自动归集")
        command.add_argument("--shapes", help="怪物shape集合，逗号分隔；默认1.45全部外观")
        command.add_argument("--start-index", type=int, default=1, help="地图展示从第几个开始，保留总进度")
        command.add_argument("--map", help="怪物地图编号，例如0为比奇省")
        command.add_argument("--MP4", nargs="?", const="", help="录制小体积MP4，可选输出路径；默认build/videos")
        command.add_argument("--ffmpeg", help="MP4编码器路径；默认PATH或imageio-ffmpeg")
        command.add_argument("--screenshots", action="store_true", help="保存真实视口截图")
        command.add_argument("--godot", default=os.environ.get("MIR3_GODOT_EXECUTABLE"))
        command.add_argument("--checks", help="hud_checks程序的路径")
        command.add_argument("--build-dir", type=Path, default=ROOT / "build")
        command.add_argument("--config", default="Release", help="多配置生成器的构建配置")
        command.add_argument("--list", action="store_true", help="列出已登记子集，不运行测试")
    args = parser.parse_args()
    suite = SUITES[args.suite]
    if args.start_index < 1:
        parser.error("--start-index必须大于零")
    if args.map is not None or args.start_index != 1:
        if args.suite != "monsters" or args.scenario not in (None, "maps") or args.shapes is not None:
            parser.error("--map/--start-index仅用于怪物maps场景，不能与--shapes混用")
        maps = json.loads((ROOT / "tests/monster_maps.json").read_text(encoding="utf-8"))["maps"]
        if args.map is not None and args.map not in {row["map"] for row in maps}:
            parser.error("所选地图不在怪物刷怪清单中")
        count = 1 if args.map is not None else len({row["family"] for row in maps})
        if args.start_index > count:
            parser.error("--start-index超出所选地图系列范围")
        args.scenario = "maps"
    if args.shapes is not None:
        valid = {row["shape"] for row in json.loads((ROOT / "tests/monster_cases.json").read_text(encoding="utf-8"))["monsters"]}
        if args.suite != "monsters" or any(not v.isdecimal() or int(v) not in valid for v in args.shapes.split(",")):
            parser.error("--shapes仅用于monsters，且必须为清单中的shape")
        if args.scenario not in (None, "gallery"):
            parser.error("--shapes仅用于gallery外观场景")
        args.scenario = "gallery"
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
    if args.MP4 is not None and godot is None:
        parser.error("所选用例没有图形场景，无法录制MP4")
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
                if args.suite == "monsters":
                    command.append(f"--monster-scenario={case}")
                    command.append(f"--start-index={args.start_index}")
                    if args.map is not None: command.append("--monster-map=" + args.map)
                    if args.shapes is not None:
                        command.append("--shapes=" + args.shapes)
                if args.suite == "skills":
                    skill_plan.append({"case": case, "args": command[command.index("--") + 1:]})
                else:
                    commands.append(command)
        for command in commands:
            print(f"RUN {args.suite}/{case}: {command}", flush=True)
            result = run_graphics(command,args,f"{args.suite}-{case}") if "--script" in command else subprocess.run(command, cwd=ROOT)
            if result.returncode:
                print(f"FAILED {args.suite}/{case}: exit={result.returncode}", file=sys.stderr)
                return result.returncode if result.returncode > 0 else 1
    if skill_plan:
        # 所有技能图形用例合并在 skills.gd，一个进程执行计划，背景音乐保持连续。
        command = [godot, "--path", str(ROOT / "godot"), "--script", str(ROOT / "tests" / "skills.gd"), "--", "--plan=" + json.dumps(skill_plan)]
        if args.screenshots:
            command.append("--screenshots")
        display_total = 0
        for item in skill_plan:
            selection = next((value.split("=",1)[1] for value in item["args"] if value.startswith(("--skill=","--skills="))), None)
            display_total += len(selection.split(",")) if selection else sum(item["case"] in skill_cases(skill) for skill in SKILLS)
        command.append(f"--display-total={display_total}")
        print(f"RUN skills: {command}", flush=True)
        result = run_graphics(command,args,"skills")
        if result.returncode:
            print(f"FAILED skills: exit={result.returncode}", file=sys.stderr)
            return result.returncode if result.returncode > 0 else 1
    scope = "SUBSET" if args.scenario or selected is not None or args.shapes is not None else "ALL"
    print(f"TEST_SUITE_{scope}_OK suite={args.suite} cases={','.join(cases)} skills={selected}")
    return 0


if __name__ == "__main__":
    try:
        sys.exit(main())
    except (OSError, subprocess.SubprocessError, KeyboardInterrupt) as error:
        print(f"测试入口失败：{error}", file=sys.stderr)
        sys.exit(1)

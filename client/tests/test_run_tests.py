"""验证统一入口的筛选与失败传播，不启动游戏、不重复已经通过的技能。"""
import contextlib
import io
import json
from pathlib import Path
import unittest
from unittest.mock import patch

import run_tests as runner


class SkillSelectionTests(unittest.TestCase):
    def invoke(self, *args, exit_code=0):
        output = io.StringIO()
        with patch("sys.argv", ["run_tests.py", *(args if args and args[0] in runner.SUITES else ("skills", *args))]), contextlib.redirect_stdout(output), contextlib.redirect_stderr(output), patch.object(runner, "executable", return_value="test-executable"), patch.object(runner.subprocess, "run") as run:
            run.return_value.returncode = exit_code
            result = runner.main()
        return result, [call.args[0] for call in run.call_args_list], output.getvalue()

    def test_mixed_skills_route_without_case(self):
        # 跨被动、定点与附魔类别，去重后各场景仅接收适用编号。
        result, commands, output = self.invoke("--skills", "3,47,48,410,47")
        graphics = [command for command in commands if "--script" in command]
        self.assertEqual(len(graphics), 1)  # 全部技能共用进程，不能重启背景音乐。
        self.assertEqual(Path(graphics[0][graphics[0].index("--script") + 1]).name, "skills.gd")
        plan = json.loads(next(arg[7:] for arg in graphics[0] if arg.startswith("--plan=")))
        self.assertEqual(result, 0)
        self.assertEqual(plan, [
            {"case": "casting", "args": ["--skill=410"]},
            {"case": "enchantment", "args": ["--skills=410"]},
            {"case": "enchantment-visual", "args": ["--skill=410"]},
            {"case": "common", "args": ["--skills=47,48"]},
            {"case": "passive", "args": ["--skill=3"]},
            {"case": "stationary", "args": ["--skills=47,48"]},
        ])
        self.assertIn("TEST_SUITE_SUBSET_OK", output)

    def test_scenario_and_skill_combination(self):
        # 场景子命令可与技能集合组合，只运行指定场景。
        _, commands, _ = self.invoke("common", "--skills", "47,48")
        graphic = next(c for c in commands if "--script" in c)
        plan = json.loads(next(a[7:] for a in graphic if a.startswith("--plan=")))
        self.assertEqual(plan, [{"case": "common", "args": ["--skills=47,48"]}])

    def test_all_has_no_skill_filter(self):
        # 无参数仍执行统一全集，包含模型且不附加任何技能筛选。
        result, commands, output = self.invoke()
        self.assertEqual(result, 0)
        self.assertEqual(sum("--script" in c for c in commands), 1)
        graphic = next(c for c in commands if "--script" in c)
        plan = json.loads(next(a[7:] for a in graphic if a.startswith("--plan=")))
        self.assertEqual(len(plan), 18)
        self.assertTrue(all(item["args"] == [] for item in plan))
        self.assertTrue(all(not any(a.startswith(("--skill=", "--skills=")) for a in c) for c in commands))
        self.assertIn("TEST_SUITE_ALL_OK", output)

    def test_invalid_selection_rejected(self):
        # 无效编号、空集合、不适用场景和参数冲突均应失败，不静默空跑。
        for args in [("--skills", ""), ("--skills", "999"), ("passive", "--skills", "3,47"), ("--skill", "3", "--skills", "4"), ("equipment", "--skills", "3")]:
            with self.subTest(args=args), self.assertRaises(SystemExit) as error:
                self.invoke(*args)
            self.assertEqual(error.exception.code, 2)

    def test_child_failure_stops_suite(self):
        # 下游失败必须立刻停止，不能输出成功标志。
        result, commands, output = self.invoke("--skills", "47,48", exit_code=7)
        self.assertEqual(result, 7)
        self.assertEqual(len(commands), 1)
        self.assertNotIn("TEST_SUITE_SUBSET_OK", output)

    def test_shinsu_includes_monster_entry(self):
        # 召唤神兽不能仅覆盖起手和被忽略的技能回包，必须包含怪物入场场景。
        _, commands, _ = self.invoke("--skill", "30")
        graphic = next(c for c in commands if "--script" in c)
        plan = json.loads(next(a[7:] for a in graphic if a.startswith("--plan=")))
        self.assertEqual([item["case"] for item in plan], ["common", "stationary", "summon"])

    def test_outcomes_route_for_each_skill(self):
        # 单技能必须自动带上后续结果，不依赖手工选择场景。
        for scene, ids in (("summon", (17,30,105)), ("persistent", (16,22)), ("displacement", (8,21,104))):
            for skill in ids:
                with self.subTest(skill=skill):
                    _, commands, _ = self.invoke("--skill", str(skill))
                    graphic = next(c for c in commands if "--script" in c)
                    plan = json.loads(next(a[7:] for a in graphic if a.startswith("--plan=")))
                    outcome = next(item for item in plan if item["case"] == scene)
                    self.assertEqual(outcome["args"], [f"--skills={skill}"])

    def test_summon_filter_does_not_add_other_pets(self):
        # 只选骷髅时不能顺带执行神兽；混选时也只保留召唤适用编号。
        _, commands, _ = self.invoke("--skills", "17,22,105")
        graphic = next(c for c in commands if "--script" in c)
        plan = json.loads(next(a[7:] for a in graphic if a.startswith("--plan=")))
        self.assertEqual(next(item["args"] for item in plan if item["case"] == "summon"), ["--skills=17,105"])

    def test_display_progress_counts_individual_plans_once(self):
        # 多个非批量技能产生多个计划，每个计划只计自己的技能，不能平方累加。
        _, commands, _ = self.invoke("casting", "--skills", "1,27")
        graphic = next(c for c in commands if "--script" in c)
        self.assertIn("--display-total=2", graphic)

    def test_mp4_option_passes_optional_output(self):
        # 大写MP4既可自动命名，也可指定路径；不额外建立测试入口。
        for extra, expected in ((["--MP4"], ""), (["--MP4", "demo.mp4"], "demo.mp4")):
            with self.subTest(extra=extra), patch.object(runner,"run_graphics") as record:
                record.return_value.returncode=0
                result, _, _ = self.invoke("common", "--skill", "2", *extra)
                self.assertEqual(result,0)
                self.assertEqual(record.call_args.args[1].MP4,expected)

    def test_model_only_cannot_record(self):
        # 纯模型没有视口，不能生成假视频或启动无输入编码器。
        with self.assertRaises(SystemExit) as error:
            self.invoke("models","--MP4")
        self.assertEqual(error.exception.code,2)

    def test_monster_filter_routes_to_matching_scenario(self):
        # 地图筛选不能误跑外观全集；shape筛选不能被maps静默忽略。
        for arguments, scenario, flag in [
            (("--map", "0"), "maps", "--monster-map=0"),
            (("--shapes", "20,23"), "gallery", "--shapes=20,23"),
            (("--start-index", "2"), "maps", "--start-index=2"),
        ]:
            result, commands, output = self.invoke("monsters", *arguments)
            self.assertEqual(result, 0)
            self.assertEqual(len(commands), 1)
            self.assertIn("--monster-scenario=" + scenario, commands[0])
            self.assertIn(flag, commands[0])
            self.assertIn("TEST_SUITE_SUBSET_OK", output)

    def test_monster_empty_or_incompatible_selection_fails(self):
        # 无效或互斥筛选必须在启动游戏前失败，不能以零用例成功结束。
        for arguments in [
            ("monsters", "--map", "missing"),
            ("monsters", "--start-index", "9999"),
            ("monsters", "--start-index", "0"),
            ("monsters", "maps", "--shapes", "20"),
            ("monsters", "--map", "0", "--shapes", "20"),
            ("equipment", "--map", "0"),
        ]:
            with self.subTest(arguments=arguments), self.assertRaises(SystemExit) as error:
                self.invoke(*arguments)
            self.assertEqual(error.exception.code, 2)


if __name__ == "__main__":
    unittest.main()

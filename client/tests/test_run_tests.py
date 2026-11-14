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
        self.assertEqual(len(plan), 16)
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


if __name__ == "__main__":
    unittest.main()

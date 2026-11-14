# 技能专项运行方式

先编译当前客户端与模型检查程序：

```powershell
cmake --build client/build --config Release --target mir3_client hud_checks --parallel
```

调试单个技能，例如魄冰刺110：

```sh
python client/tests/run_tests.py --suite skills --skill 110 --screenshots
```

当前有独立断言的编号：1、27、73、110、113、150，以及七系附魔效果408至414。1包含起手、远端回包、超时和拒绝恢复；其他编号只运行对应表现或位移用例。未知编号返回失败，不允许空跑通过。后续新增逐技能用例必须同时加入筛选列表和默认全量路径。

开发和调试时仅运行本次修改影响的单技能用例，不重复跑此前已通过且未受影响的技能。技能子系统最终完成后集中运行一次全量，不传`--skill`或`--case`：

```sh
python client/tests/run_tests.py --suite skills --screenshots
```

全量包含技能模型、技能原包模型、动作确认模型、技能列表/快捷键图形用例、全部技能施放用例、本地武器附魔及强制位移回归。任一失败立即停止并返回非零退出码；只有全部通过才输出`TEST_SUITE_ALL_OK`。单技能或子集运行输出`TEST_SUITE_SUBSET_OK`，不能替代最终全量验收。当前用例集合通过不代表尚未覆盖的全部原版技能已完成还原。

只调试本地附魔410时，无需重复运行远端附魔：

```sh
python client/tests/run_tests.py --suite skills --case enchantment --skill 410 --screenshots
python client/tests/run_tests.py --suite skills --list
python client/tests/run_tests.py --suite equipment --list
```

总入口只使用Python标准库与参数数组启动进程，不依赖PowerShell或shell拼接。用`--godot`或环境变量`MIR3_GODOT_EXECUTABLE`指定Godot程序，`--checks`指定模型检查程序；自动识别Windows的`.exe`及Linux无后缀程序，并查找多配置的`build/Release`或单配置的`build`目录。`--build-dir`、`--config`可覆盖构建位置。Linux也可用`python3`执行；当前实际运行验证在Windows，Linux客户端构建和运行仍需目标平台验证。

截图保存在`client/build/screenshots`，使用真实渲染视口；不要添加`--headless`。怪物等其他子系统后续按同样的全集/单项规则登记，未登记的测试集不会被入口宣称通过。

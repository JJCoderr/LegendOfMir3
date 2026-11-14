# 技能专项运行方式

先编译当前客户端与模型检查程序：

```powershell
cmake --build client/build --config Release --target mir3_client hud_checks --parallel
```

调试单个技能，例如魄冰刺110：

```powershell
./client/tests/run_skills.ps1 -Skill 110 -Screenshots
```

当前有独立断言的编号：1、27、73、110、113、150，以及七系附魔效果408至414。1包含起手、远端回包、超时和拒绝恢复；其他编号只运行对应表现或位移用例。未知编号返回失败，不允许空跑通过。后续新增逐技能用例必须同时加入筛选列表和默认全量路径。

开发和调试时仅运行本次修改影响的单技能用例，不重复跑此前已通过且未受影响的技能。技能子系统最终完成后集中运行一次全量，不传`-Skill`：

```powershell
./client/tests/run_skills.ps1 -Screenshots
```

全量包含技能模型、技能原包模型、动作确认模型、技能列表/快捷键图形用例、全部技能施放用例及强制位移回归。任一失败立即停止；只有全部通过才输出`SKILL_SUITE_ALL_OK`。单技能运行输出`SKILL_SUITE_SINGLE_OK`，不能替代最终全量验收。当前用例集合通过不代表尚未覆盖的全部原版技能已完成还原。

可用`-Godot`指定Godot控制台程序路径。截图保存在`client/build/screenshots`，使用真实渲染视口；不要添加`--headless`。

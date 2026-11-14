# 兼容历史命令；怪物验收实现统一维护在monsters.gd。
extends "monsters.gd"

func run() -> void:
	await run_regression()

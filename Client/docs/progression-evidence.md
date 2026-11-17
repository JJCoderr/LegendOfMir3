# 角色属性增量与升级反馈

## 依据

复用 Godot br_godot_test_261115 的 actor_experience.hpp、actor_increments.hpp、level_up_effect.hpp 规则及 effect_catalog.hpp 的MInfo列映射，参考其docs/evidence/actor-experience.md、actor-increments.md、level-up.md中保存的MIR3 4282C0/42F650/4495D0/44B7E0/44A110证据。运行时完全使用当前CLIENT的窗口、人物、WIL、DirectDraw、照明与音效系统。

## 行为

SM_WINEXP更新32位总经验；wParam是获得经验提示数，wTag非零时更新16位武器修炼值，wSeries低字节控制覆盖或累加回绕，高字节控制先输出1513提示。使用原文案1459/1460及白字红底；完整能力同步不修改修炼缓存。

SM_WEIGHTCHANGED只改变当前负重，保留上限；SM_SUBABILITY按各头字段低/高字节更新附加属性。二者沿用既有字段语义并纳入统一测试。

SM_LEVELUP的wSeries为0时更新本地等级低字节/总经验，动作7、方向4，输出1469白字橙底提示；为1时recognition是远端人物ID，不更新本地属性；其他分支忽略。骑乘人物动作结束恢复骑乘待机。

新增CLevelEffects按MInfo.dat的#MAGIC区读取422记录，并加载Data/MagicEx.wil；当前记录是2020..2030、80ms、库137、模式2。RGB565绘制采用染色后的source + destination*(1-source)，支持多个实例。严格大于间隔才推进一帧，结束保留末帧，光尾收缩后回收；位置固定于触发时格子，切图及离开游戏清空。照明接入当前CLightFog，随特效选项控制。

音效请求使用原版14222。当前SoundList.wwl按58字节头、16字节记录遍历确认没有此ID，因此不能验证可听输出，不以其他声音代替。

## 验证和边界

clean Client 87/87及全部七目标构建通过；新增14项进度/资源/帧间隔/生命周期检查及1项骑乘恢复，相关属性、地图、任务、邮件60项回归通过，共75项。预览191 progression、192 level-tail；out/progression.png和out/level-tail.png已核对。两个效果来自本地/远端分支，预览为了观察固定绘制到两处屏幕位置；实服渲染使用实际地图投影。

运行目录须包含原资源MInfo.dat，本次已从原客户端复制到out/mud3-01。真实服务器时序、移动过程中的全部边界、夜间地图实景与逐像素对比尚未验收；不将离线预览称作实服升级完成。

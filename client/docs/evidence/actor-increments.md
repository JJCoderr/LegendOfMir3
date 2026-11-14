# 人物负重与附加能力增量

## 实现前证据（2026-11-14）

原版4282C0的SM_WEIGHTCHANGED分支（429F8A写入）：recognition低16位写this+3583543，即能力正文+27背包当前负重；参数一低字节写+3583547（正文+31穿戴当前负重），参数二低字节写+3583549（正文+33手持当前负重）。不更新三种负重上限，不处理头中其余位。

原版42EC70将SM_SUBABILITY交给42F650：recognition低字节零扩展为WORD写this+3583579；三个参数逐低/高字节写+3583581..3583586。参考reference/legendOfMir2_src/Mir3EIClientDev/Mir3EIClient/GameProcess/Actor.h的ACTORSUBABILITY及GameProc.cpp消息分支，字段依次为AntiMagic、HitPoint、SpeedPoint、AntiPoison、PoisonRecover、HealthRecover、SpellRecover。不能依赖C++结构体对齐直接拷贝。

原版4710F0状态窗口读取8246D5和8246D6（主对象4B9878加3583581/3583582），作为两行百分比显示，即命中/敏捷附加值。其余字段保存供后续已确认消费者使用，不推断自动回复行为。SM_ABILITY与SM_SUBABILITY写入区域互不重叠，完整能力更新不能清除附加能力。

范围：本批完整接入负重及附加能力两类无正文消息、保留负重上限、刷新状态窗口、换角色清理。经验消息还包含聊天通知、等级消息还包含动画，另批恢复完整行为，不只接数值而漏掉反馈。

## 验证结果

Release构建通过；hud_checks --actor-increments和--actor-ability通过，使用抛异常check保证Release下仍执行断言。actor_increments.gd实际视口测试通过：负重当前值及上限、无关生命金币不变、附加能力互不覆盖、截断拒绝、清零、切换角色清理。截图为build/screenshots/actor-increments-status.png，提交前已在对话展示。仅验证受影响链路；真实服务端联调及其余附加能力消费者尚未验收。

# 经验获得消息

## 实现前核对

原版4282C0 case 44：recognition写this+3583535，能力正文+19经验DWORD。参数一为获得经验的提示数值；参数二非零时使用CMList1459，零时用1460。参数二还更新this+2959663的WORD武器修炼值：参数三低字节非零则覆盖，否则16位累加回绕；参数二零时不写入。参数三高字节非零时先输出CMList1513附加提示。两种聊天都经4366D0，前景/背景来自401430的2/3号色，即白字、COLORREF255背景。

参考mir3_1.45/gameplay.cpp的receive_local_experience及extension.cpp的progression_notices，按功能移植，无模拟服务器依赖；消息名使用reference/legendOfMir2_src/Mir3EIClientDev/Mir3EIClient/Define.h的SM_WINEXP。完整能力包不写入武器修炼值。

另核对case 45：本地分支参数三为0，写经验及等级低字节，调用人物状态7/4并创建世界特效422，提示1469；参数三为1时recognition是远端人物身份，仅对该人物创建特效。当前客户端缺少世界特效绘制，下一批恢复这一完整链路，不以仅改等级代替升级完成。

## 验证

Release构建、hud_checks --actor-experience及--actor-ability通过。actor_experience.gd真实视口通过：经验总量/等级上限隔离、原文案替换与前背景色、附加通知和正文顺序、武器修炼累加/覆盖/零值、截断拒绝、完整能力包保留修炼值、换角色清理。截图actor-experience-status.png已在提交前展示。真实服务端联调与升级场景特效尚未完成。

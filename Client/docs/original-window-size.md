# MIR3.EXE 窗口尺寸：IDA 实证

分析日期：2026-11-17。使用 IDA Professional 9.1 / idalib 会话 e2dc0b07。

## 样本

`G:/传奇3/Mud3/Mir3.exe`，1,064,960 字节。
SHA256：`568453e025653402625254a519007fb0ccb6c72fbb949b29668e9293d5c8222f`。
IDA retrieve_input_file_sha256 与文件哈希一致。

另一个 `G:/传奇3/宝藏传奇3/Mir3.exe` 为300,544字节，SHA256
`fdce0cf98757d64c18eb08f9346842e7028b93070872cd4c50a0bd116fd47195`，
会话 a3390459 显示 UPX0/UPX1/.aspack 段，本次没有完成该版本解包，不能把下述结论当作这份文件的反编译结论。

## 尺寸和调用链

| 场景 | 客户区/画布 | IDA 证据 |
| --- | --- | --- |
| 初始服务器组 | 800×600 | WinMain 0x401B50 调用 0x48AE74，最后参数为1；后者传给0x49359E，取800/600分支 |
| 登录 | 640×480 | WinMain直接登录分支及0x48AAE0从服务器组进入登录，均调用0x48B48C(640,480,16,mode,5) |
| 选角 | 640×480 | 登录到选角0x402F90不重设尺寸；从游戏返回0x421E80明确调用0x48B48C(640,480,16,mode,5) |
| 游戏 | 800×600 | 0x421A5D调用前push 0x258、push 0x320；游戏窗口/全屏切换0x425FC8及0x425FF8也是800×600 |

0x48B48C 将宽、高写入对象+348、+350，随后0x48B2ED创建DirectDraw。
窗口模式进入0x48B586：

```cpp
SetRect(&rect, 0, 0, *(WORD*)(this+348), *(WORD*)(this+350));
AdjustWindowRectEx(&rect, GetWindowLongA(hwnd, GWL_STYLE), GetMenu(hwnd)!=0,
                   GetWindowLongA(hwnd, GWL_EXSTYLE));
SetWindowPos(hwnd, 0, 0, 0, rect.right-rect.left, rect.bottom-rect.top, 0x16);
```

同一宽高也用于离屏缓冲区；全屏分支传给DirectDraw SetDisplayMode。
0x49359E初次创建窗口时，按参数选择640×480或800×600，再加系统边框、标题栏和可选菜单高度。
因此上表不是外框尺寸。外框随系统主题度量变化。

## DPI 与结论边界

本次追踪到的窗口创建/重设调用使用640×480、800×600，未见1024×768尺寸分支或显式DPI乘法；不是对所有二进制常量作全局不存在证明。
现代Windows可能对旧程序进行DPI虚拟化；源码逻辑尺寸与屏幕物理像素必须分开。
此前新客户端9bdb779在144 DPI下主动生成960×720和1200×900，是新增缩放策略，不是IDA证明的原版尺寸。
这次只记录分析结论，未再次更改默认窗口大小。

原始反编译/交叉引用输出位于out/ida-window-{xrefs,sizing,transitions,all-calls,verification}.json。

## 最终显示策略（用户确认）

保留DPI放大。原版尺寸作为内部画布尺寸，默认客户区用
`MulDiv(canvasSize, systemDpi, USER_DEFAULT_SCREEN_DPI)`计算，
系统DPI通过窗口DC的`LOGPIXELSX`读取；不硬编码150%或960×720/1200×900。
当前清单是system DPI aware，因此使用系统DPI；这不是跨显示器逐屏DPI自适应。
显式INI WindowWidth/WindowHeight仍为用户指定的物理尺寸。

## 2026-11-23 样本复核

重新计算 `Mud3/Mir3.exe` 的 SHA256，与本页记录一致。PE 导入表中，USER32 提供
`CreateWindowExA`、`GetWindowLongA`、`AdjustWindowRectEx`、`SetWindowPos`，
DDRAW 提供 `DirectDrawCreate`。对样本本体反汇编时，窗口创建函数
`0x49359E` 的两条分支分别在 `0x493605` 和 `0x493615` 附近把
640×480 与 800×600 加上系统边框度量。登录入口 `0x48AB27` 和游戏返回选角
`0x421ED1` 前传入 640×480，进游戏的 `0x421A5D` 前传入 800×600。

## 新版窗口缩放

窗口模式的客户区按 4:3 缩放。参照 Zircon 的显示规格，以 1024×768 为默认基准和物理像素下限。
默认客户区在各场景统一按系统 DPI 缩放，例如 150% 时为 1536×1152。登录和选角沿用
640×480 内部画布，服务器组和游戏沿用 800×600 内部画布。

INI 中的 `Display/WindowWidth`、`Display/WindowHeight` 以物理像素为单位，窗口取能容纳所填
宽高且满足下限的最小 4:3 尺寸。手动调整的尺寸在本次运行的场景切换中保留。

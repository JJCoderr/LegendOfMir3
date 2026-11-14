# KingMir3.dll 分析副本脱壳（2026-11-14）

用途：Mir3.exe未命中的新增技能/行为的补充证据源，不能替代已确认的原版逻辑。原文件保持不变；本轮没有修改客户端行为或服务端，也未重复技能用例。

## 样本与结果

- 原件：`G:/游戏/宝藏传奇3客户端/KingMir3.dll`，381952字节，SHA256 `2f1809c944804ceea93792e2e18c24382927facc115e102ff744537ad1cbfbd9`；脱壳后复核未变。
- 分析副本：`client/build/king-unpack/KingMir3-analysis.dll`，962560字节，SHA256 `bd628a9dcbb3a96e90d5a41e6f5229d0e440e3520dbcad2aebfcbb002260688b`。
- Unicorn仅执行ASPack加载桩，模拟内存分配和导入解析；在进入原代码段前停止。首个0x401000是临时RET探测，不能误判为原入口；实际停点0x4B0530可反编译为Delphi库初始化。
- 实测509次加载API调用、431个唯一导入符号。重建原.idata中的476个导入槽，18组描述符，21个原导出保留。pefile读取无结构警告。两次生成哈希一致。
- IDA库`mir3-king-unpacked`识别5176个函数（壳样本只有1个）；入口与GetMsgProcess(0x4A92B8)均成功反编译。输出见`client/build/king-unpack/ida-survey.txt`。

## E-imports / E-exports

壳IAT以GetProcAddress/GetModuleHandleA/LoadLibraryA为核心，另保留各模块少量导入。恢复后有kernel32（运行时/文件/线程）、user32/gdi32/comctl32（界面）、advapi32（注册表等）、oleaut32（自动化）、version、shell32、wsock32（网络）、winspool。分类是依赖概况，不等于已执行这些行为。逐符号及模拟调用记录为`emulation.json`，原IAT槽与名称映射为`repair.json`。

业务导出包括GetCharInfo、GetCharInfoEx、SetCharInfo、GetMsgProcess、RegMsgProcess，其余主要为EurekaLog。GetMsgProcess从参数+16取字符串并加入线程对象维护的列表；库初始化创建线程。尚未跟进线程执行函数，不能由导出名或未搜到Magic/MInfo字符串推断不存在技能补丁。

## 复现

在仓库根目录执行（依赖只装入build下独立环境）：

```powershell
python -m venv client/build/unpack-venv
client/build/unpack-venv/Scripts/python.exe -m pip install unicorn==2.1.4 pefile==2024.8.26 capstone==5.0.9
client/build/unpack-venv/Scripts/python.exe client/tools/analysis/unpack_king.py
client/build/unpack-venv/Scripts/python.exe client/tools/analysis/rebuild_king.py
```

脚本对原样本SHA256设硬门槛；只支持已验证布局，未知加载API立即报错，未到原代码则失败。导入桩地址仅用于模拟，分析PE的原IAT已改回名称/序号导入。脚本和证据提交，二进制/内存快照/虚拟环境留在忽略的build目录。

边界：这是已验证可供IDA分析的副本，不是已验证可替换运行的DLL。重定位目录虽可读取，但未做异基址加载/TLS/异常链及原程序运行验收；不得声明完整可运行脱壳。后续首先追踪线程Execute及消息/人物信息分发，与未决技能逐项关联。

## 导出后续追踪

4A92A8的线程Execute指向4A9350（类名TWgCheck），循环Sleep(0x6D60)，枚举进程及模块，解析GetMsgProcess加入的逗号分隔规则，比较模块名、导出函数存在性和指定偏移的内容，触发回调/终止线程。GetCharInfo经4A85D8构造流并编码，GetCharInfoEx经4A8710反向解码；SetCharInfo遍历PE导出名并返回地址/RVA。证据为ida-dispatch.txt及这些导出反编译。已追踪路径未显示MInfo加载、技能范围或声光分发；这是局部职责结论，不能扩展为整个DLL不含补丁。后续技能差异优先返回原版确定的分支，不继续无目标遍历本DLL。

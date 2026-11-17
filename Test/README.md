# CSV 联调场景

Test.exe 是独立控制台程序，读取 CSV，逐行向 Client 或 Server 转发 JSON-RPC，检查响应并记录日志。Client/Server 不读取场景文件。CSV 的一行可以操作客户端，下一行立即查询服务端。

## 构建和运行

在仓库根目录运行：

```powershell
cmake --build build-eiserver --config Release --target Test --parallel 6
build-eiserver/Release/Test.exe Test/Inventory.csv --client-pid 1234 --server-pid 5678 --from 20 --to 40 --log out/item-segment.log --database out/runtime/mir3.sqlite3
```

- 第一个参数指定 CSV。
- `--client-pid` / `--server-pid`：以 `--debug-pipe` 启动的真实进程；只需提供所选步骤使用的目标端。
- `--from` / `--to`：包含边界的 **step 编号**，不是文件行号。省略时执行全部；空范围、逆序范围报错。
- `--log`：UTF-8 日志文件，自动创建父目录，每次运行覆盖；默认当前目录 `test.log`。
- `--timeout-ms`：单步骤 RPC/状态等待期限，默认 65000 毫秒。
- `--database`：Database 断言使用的 SQLite 数据库，只读打开。

中途开始会沿用进程当前状态，不会自动补跑前置步骤或恢复存档。Test.exe 不负责启动或关闭被测进程。

需要自动准备隔离环境并启动两端时，用辅助程序：

```powershell
python Test/launch_csv.py Test/Inventory.csv --log out/items.log
```

辅助程序使用标准库，读取同名 `Inventory.fixture.csv` 的 SQL 数据种子，创建独立 `out/rpc-*` 目录和资源链接，先执行服务端基础检查，再启动客户端并调用 Test.exe。默认程序目录 `build-eiserver/Release`，配置资源目录 `out/debug`，可用 `--bin-dir`、`--config-dir`、`--port-offset` 覆盖。也支持 `--from` / `--to`，但新启动的客户端没有已完成的业务前置状态。结束后只关闭自己启动的进程。

## CSV 格式

固定表头，step 必须为正数且严格递增，可以预留编号间隔：

```csv
step,name,target,action,params,expect
10,登录,Client,session.login,account=mirtest;password=test123,result.status=submitted
20,检查服务器,Server,state.get,,result.ready=true;result.services_ready=6
30,等待登录完成,Client,state.wait,,result.stage=server_list;result.notice=true
```

- target：`Client`、`Server`、`Database`。
- action：通常直接填写 RPC 方法名。
- params：`参数=值;参数=值`；空白表示无参数。
- expect：`响应路径=值;响应路径=值`，支持 `result.equipment.0.name` 等路径；留空表示只要求 RPC 正常返回。预期错误必须明确填写 `error.code`。
- 数字、true/false/null、数组按 JSON 值处理，其余普通文本按字符串处理。数字密码等需要保留字符串类型时使用 JSON 引号，再按 CSV 规则转义，例如单元格 `"password=""123456"""`。
- 含分号的字符串用 JSON 引号包围；含逗号、双引号或换行的 CSV 单元格用双引号包围，内部双引号写两次。支持 UTF-8 BOM、CRLF/LF 和多行单元格。

特殊 action：

- `state.wait`：反复查询该端 `state.get`，直到 expect 满足或超时。业务动作的 submitted 只代表提交，应另写状态/数据库断言确认结果。
- `raw`：params 是原始 JSON-RPC 文本，仅供格式错误、批处理、ID 精度等协议基础测试。
- `wire.expect`：Client 协议计数断言，params 例如 `message=CM_TAKEONITEM;direction=sent;at_least=1`。按源码协议名解析消息码，需要场景文件位于仓库内。
- `Database,query`：params 是只读 SQL，expect 如 `rows=[[0]]`。

`Inventory.csv` 是完整登录、物品、返回选角再进图的长场景，包含双端交替检查、协议计数及数据库断言。`Login.csv` 是独立登录场景，`RpcClient.csv` / `RpcServer.csv` 是基础协议测试。

日志包含时间、步骤编号、目标端、动作、实际响应、PASS/FAIL、总数和耗时。首个失败立即停止，退出码为 1；全部通过为 0。正常请求参数不写日志。辅助程序另存 runtime.log、server-preflight.log、result.json；场景执行到 protocol.get 时生成 protocol-coverage.json。服务端当前 state.get 反映服务和网络连接状态，具体业务持久化仍由数据库断言验证。

测试脚本按子系统组织：登录使用 `Login.csv`，物品使用 `Inventory.csv`；后续角色、聊天、商店、战斗各自新增脚本。目标端由每行 target 决定，不按客户端/服务器分业务目录。RPC 基础测试独立于业务场景。

执行器自身验证：`python Test/verify_runner.py`。Runner 目录含用于验证失败处理的预期失败 CSV，应通过该验证程序执行。

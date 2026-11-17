# JSON-RPC 联调测试

测试编排、JSON 场景、测试数据及断言统一放在本目录。Client/Server 只实现 RPC 入口和业务响应，不读取测试集。

- `Client/*.json`：客户端 RPC 及经客户端发起的完整业务场景。
- `Server/*.json`：服务端 RPC 和服务就绪检查。
- `run_rpc_tests.py`：按 JSON 的 cases 顺序调用、等待状态并断言。
- `rpc_fixture.py`：准备隔离数据库、端口、配置及资源链接。
- `protocol_catalog.py`：统计实际协议收发和未观察的消息。

从仓库根目录执行（Windows，Python 3，仅使用标准库）：

```powershell
python Test/run_rpc_tests.py Test/Client/items.json --launch
python Test/run_rpc_tests.py Test/Client/login.json --launch
python Test/run_rpc_tests.py Test/Server/jsonrpc.json --pid 12345
```

`--launch` 先启动 EIServer 并执行服务端基础测试，再启动客户端执行指定场景。默认使用 `build-eiserver/Release` 的程序和 `out/debug` 的配置、资源，可用 `--bin-dir`、`--config-dir`、`--port-offset` 覆盖。结束后只关闭本次启动的进程。`--pid` 附着到已使用 `--debug-pipe` 启动的进程，角色由 JSON 的 role 决定，不创建业务数据、不执行数据库断言、不关闭该进程。

每次运行产生独立 `out/rpc-*` 目录：

- `test.log`：时间戳、逐项 PASS、失败异常、最终结果及耗时。
- `result.json`：已通过检查、总体状态、错误及耗时；失败退出码非零。
- `runtime.log`：自动启动的客户端和服务端标准输出。
- `protocol-coverage.json`：客户端实际收发统计；消息被观察到不等于业务验证完成。

JSON 使用 `include`（相对当前 JSON 路径）复用场景；`cases` 按顺序执行；`fixture` 准备 SQL 数据；`database_expect` 校验最终数据库；`wire_expect` 校验真实协议收发。动作的 submitted 仅表示请求提交，随后用 await_state 或数据库断言确认结果。日志不转储正常 RPC 请求中的密码。

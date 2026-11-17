# 寄售列表与页面状态

## 原版证据

沿用已确认的 MIR3 1.45 分析镜像，IDA 会话 ed4a8612，只读导出 out/consignment-receive.txt、out/consignment-actions.asm 及 out/extension-dispatch.c。

- 4282C0 消息1355调用455BF0：wSeries为记录条数，wParam当前页，wTag总页。仅窗口未打开时调用43AB90(19)，随后保存recognition为会话标识；已打开时不会关闭或替换该标识。
- 455BF0清列表、hover=-1、selected=0、等待状态=0；count>0时更新页码。编码正文从消息头之后开始，按斜杠分段，每段六位解码312字节。缺少尾分隔符的段不处理。空列表保留旧页码，这里沿用原行为。
- 4546F0绘制服务器字符串0、物品名91、价格DWORD72、卖家20；选中详情绘制卖家20、日期76。455CF0复制91起100字节物品记录，191起描述按CR分段和217像素换行。
- 456334读取记录+0x10作为操作中的商品标识。因而字段范围为服务器0..15、标识16..19、卖家区20..71、价格72..75、日期76..90、物品91..190、描述191..311。字符串按字段边界截取，不对未确认的卖家区内部字段作解释。
- 消息1400令对象DWORD434等于recognition==0，此字段控制4546F0加载动画。

## 实现范围

ConsignmentWire统一消息和字段常量；原生CConsignmentWnd接收列表后使用已有四列表格、物品详情、描述换行及分页显示。有效列表默认选择首条，清空列表时使用selected=-1避免旧选择残留。1355在需要时打开原生窗口，1400更新等待动画。关闭清会话标识。

本批仅恢复寄售列表及页面状态，查询/购买/撤回/登记的发送适配仍待完成，不能以离线预览声称实服交易成功。

## 验证（2026-11-17）

客户端clean构建通过，全部七目标构建通过（out/build-consignment-wire-clean.log、out/build-consignment-wire-all.log）。预览consignment-wire（编号180）先构造原记录、六位编码再走ReceiveList，10项检查全部通过；寄售GUI16项、邮件42项、投石车19项回归全部通过。截图out/consignment-wire.png确认四列、首条选择、日期、CR换行及物品图标。尚未通过完整加密网络包注入验证1355分派，也未连接实服。

## 操作发送恢复

- 47D030/47D060/47D090分别1131下一页、1132上一页、1134刷新，recognition为窗口会话标识，其他参数0。455360按钮分支传入对象+0x7D4。
- 455F80查询调用47D100发送1136：名称查询正文 `*<CR>名称<CR>0`，卖家查询 `卖家<CR>*<CR>0`，空查询为 `*<CR>*<CR>0`。
- 456290购买调用47D0C0发送1133，recognition会话ID，商品ID拆低高WORD到wParam/wTag，正文物品名；4562F0撤销调用47D1D0发送1135，参数相同但正文空。商品ID取列表记录+16。
- 4554F9将浏览窗口会话标识复制到登记窗口。457370登记调用47D140发送1130，recognition会话ID；堆叠模式>=2时WORD参数为数量，非堆叠为0；正文为 `物品名<CR>实例ID<CR>价格<CR>说明`。说明复用原版编辑器CRLF转换，空行保留空格CR。调用后关闭登记窗口，背包不做本地扣减。
- 原版先用160字节栈记录（name0、instance28、price32、description36）暂存再sprintf拼包；不复制其越界写入行为，发送名限27、说明限123字节并保留DBCS边界。输入控件仍保留原版长文本编辑能力。

所有操作的会话标识随Request不可变快照保存，确认后从快照构造报文。原版的重复操作时间限制、完整网络注入及实服交易回执尚待验证，不把成功构造或调用send视为交易成功。只读导出见out/consignment-send.txt、out/consignment-transaction.txt。

### 发送适配验证（2026-11-17）

客户端clean构建及全部七目标构建通过；新增12项报文检查和原有87项回归全部通过。现有GUI检查同时覆盖确认期间会话改变时保留快照、登记关闭并清会话。日志out/build-consignment-send-clean.log、out/build-consignment-send-all.log，报告out/mud3-01/consignment-send-check.txt。截图out/consignment-send-confirm.png逐张检查通过。没有执行实服交易。

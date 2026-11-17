# 地面物品显示迁移审查

基线：5524af8。参考G:\传奇3\Mir3分支br_godot_test_261115的client/src/hud/ground_items.hpp及client/src/godot/hud_view.cpp。状态：下述差异已完成原生迁移与离线验证，见末节。真实服务器拾取尚未验证。

## 已确认的完整功能缺口

- 原生GROUNDITEM仅含id/坐标/图号/40字节名称。SM_ITEMSHOW直接解码入40字节数组后按返回长度写NUL，缺少参考的编码校验、地图边界、正id、首个斜杠分离stack_mode和39字节名称限制。重复id应保持原有记录；SM_ITEMHIDE正文非空应拒绝。
- 原生MEMMAPPEDID::pstItem与人物共用_MAX_CELL_OBJ=10；参考地面物品同格独立上限100，在图片查找前计数，不能直接把人物容量一起改成100。
- 原生RenderObject在地面物品前绘制偏移1像素阴影；参考4253ED/42541C绘制普通图片与同格高亮，按24-width/2、16-height/2居中，无该旧阴影。
- 原生名称只绘同格首条，旧三维灰底。参考draw_ground_name：起点cellY*32-cameraY-4；原始名称测宽；棕底RGB(80,60,40)、alpha150、高15；文字黑影x-1,y+2，正文x,y+1，COLORREF0x7dffff；每条上移16，最多100条。CmList1409名称加一位数字时正文去数字，但仍按完整名称测宽，且停止后续名称绘制。需遵守地图指针可用条件，不穿透窗口。
- 缺少ProgUse40..49闪光：锚点格左上+8,0，忽略资源offset；每实例delay=3000+abs(rand1-rand2)%4001，起始GetTickCount；严格超过delay后进入frame0，累计严格>100ms仅进一帧并清零，达到10重置起始tick/frame=-1，delay保持。
- 参考每帧剔除玩家中心半开区间[-24,24)外物品，包含服务器修正坐标；需先移除再构建单格映射，避免悬空指针。换图/退出清理需释放所有记录。

## 下一完整批次验收

一次完成消息接收、100件单格映射、名称/高亮/闪光、范围及场景清理，再统一干净构建（结构布局变化，旧Ninja头依赖不可靠）。实际01.map预览展示同格多件名称、金币变体和闪光，保存每种截图；验证重复/非法包/39字节边界/100容量/严格时钟边界/半开范围/场景清理，回归原生窗口与拾取路径。测试、截图通过后统一中文提交，未联机不得声称拾取成功。


## 整批实现与验收（2026-11-17）

已完成SM_ITEMSHOW编码范围/地图范围/id/39字节名称验证，分离stack_mode，重复id保持记录；SM_ITEMHIDE拒绝附加正文。地面映射使用独立100容量，不扩大人物/魔法容量。去掉旧投影，使用现有原生HighlightItemPixel算法高亮；名称改为9pt宋体、原色和棕色半透明背景，上移16px排列，金币变体按原完整宽度布局后去掉数字并停止后续标签。

GroundItemDisplay集中100件/39字节/24格/40..49闪光常量及DWORD时钟。RenderScene先剔除半开24格范围外记录，再重建映射；换图与退出释放记录。闪光按格左上+8,0在世界覆盖层绘制，不使用WIL偏移。名称/高亮复用SpellPointerAvailable并屏蔽公告；鼠标格换算补地图滚动偏移，渲染每帧重算以适应摄像机移动。

新增预览241 ground-items、242 ground-coin；实际01.map使用生产资源加载，三件同格名称/金币标签/闪光截图out/ground-items.png、out/ground-coin.png已经查看并展示。预览送入真正OnSocketMessageRecieve，不直接伪造接收结果。

干净Client构建out/ground-items-clean.log [87/87]通过；七目标out/ground-items-all.log [111/111]通过。同版Mir2Ex-ground-review.exe：地面物品24、施法76、键盘34、交易54、装备25、界面音效15，共228项全通过，diff空白检查通过。24项包括时间严格边界/回绕、重复id、错误编码/坐标/id、39/40字节、带正文hide、100容量、半开范围真实RenderScene剔除、换图释放、小地图/聊天遮挡、滚动指针。没有额外声称网络拾取或实际服务器物品持久化成功；该批未更改SendPickUp协议，仍待在线验证。未完成MIR3.EXE同帧逐像素比较；闪光随机源沿用native rand，未统一天气随机序列。

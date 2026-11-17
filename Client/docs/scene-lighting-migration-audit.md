# 场景光照迁移批次

基线2098af4。以Mir3 br_godot_test_261115的scene_lighting.hpp及hud_view.cpp draw_scene_lighting、map_message.hpp为依据。本批已完成原生接入及离线集中验收，详见末节；历史过程记录保留。真实服务器与原版同帧逐像素验收未完成。

已落地（尚未验证）：SceneLighting.h复用参考SceneAmbient/SceneLight/SceneLightGrid；GameProc初始化环境状态，新地图/换地图用series低字节enter，610以外的昼夜消息更新mode，RenderScene推进渐变；旧WM_TIMER淡入淡出停止。参考进入亮度255/15/85，未知10；渐变使用独立half_level初始0，每50ms向510/30/170推进，未知目标0；不能擅自从display_level反推half_level。相同mode进入清fading，更新比较完整WORD后截断byte。

光照网格从18x22顶点改18x19，绘制顶点从0,0起，采样保留+8,+29，参考并无绘制层偏移。Ring内圈是像素半径，外圈是32*格半径；旧实现把内圈乘32且用外圈分母，已替换参考SceneLight::sample。逐光源更新三角对角线，每帧清空对角线为默认false。常规圆光源走Ring；保留未使用到的非1暗度/椭圆旧路径待调用审查。顶点颜色改为D3D的RGB通道排列，避免原COLORREF红蓝反转。

同时核对发现上一批地面闪光混合方式仍不完整：Godot ground_sparkle_layer材质是ONE/INVSRCALPHA且alpha除255。已改现有DrawSelectionFrame Additive、sourceAlpha=1，待此批一起截图回归；此前Normal截图不代表混合已对齐。

剩余必须完成后再统一测试/提交：
- 人物/怪物/NPC灯属性到半径颜色的规则与灯锚点；目前CHero仍旧3格/属性直用，且受bDrawShadow提前返回影响。
- 地图光源颜色、半径、采集范围与添加顺序；魔法灯/人物灯顺序应和参考一致。
- 昼夜消息wParam选基底与wSeries天气共享随机源；目前只接环境渐变，不可宣称整条消息完全迁移。
- 旧调试昼夜开关/命令仍直接改m_fDarkRate，要与新状态统一。
- 同一张地图的白昼/夜间/晨昏/人物灯/彩色法术灯截图，原生顶点数值断言、渐变严格边界及地图切换回归；头文件/布局变化必须clean构建。


## 光源收集继续（未验收）

新增LightSceneMap：按参考camera范围x[-20,+37]/y[-20,+38]读取地图lightEvent，颜色索引(packed>>4)&255<10才接受；4/8/12/20四档半径、参考10色表。去掉RenderObject里统一12半径/固定颜色的旧逻辑，地图灯在本地动作更新后采集，避免摄像机滞后一帧。

新增LightSceneActors：本地灯使用本地绘制锚点；远端人/NPC/怪物用锚点+24,+16及SceneLight::actor规则，死亡人/怪物与地下完成怪物不发灯，可见性检查矩形。移除CHero末尾灯与bDrawShadow的耦合；灯在施法及升级灯之后集中加。尚需进一步核对本帧可见性与分类顺序、武器发光顺序；不能凭静态矩形断言等同参考的每帧bounds。

旧U昼夜调试和@timeshift改用SceneAmbient。46消息选基底临时使用现有m_effectRandom.random，与法术序列相同；下一步天气必须也接同一状态，不能另起PRNG。已读取完整SnowWeather及draw_snow/draw_rain/update_weather_layers：雪花500槽、雨400槽、天气select(type,changing_map)保留雨的特殊语义；雾缩放4000x1200且坐标(-1600+fog_x,-300-fog_y)，不能直接沿用旧Mist宣称相同。天气绘制/状态整批尚待实现，当前仍未构建测试/提交。


## 原生接入与集中验收（2026-11-17）

SceneWeather.h复用完整参考SnowWeather，替代旧Mist/Snow/Rain实例及其创建/销毁；雪/雾受@snow总开关控制，雨独立。46消息wSeries选择天气；地图消息series高字节用changing_map=true，保持参考换图雨状态语义。退出清粒子/雨状态；雾位移、随机流按参考保留。法术粒子/余烬/痕迹与天气、昼夜基底都使用同一m_weather MSVCRT序列，初始化用GetTickCount。

WeatherRenderer.h以现有WIL解码缓存与RGB565回缓冲绘制：ProgUse500雪、500..504/510..514雨、550雾；雪/雨四边形使用参考UV旋转转置；雨按前后位置旋转并处理32x32水花；雾4000x1200缩放与坐标按参考。雪采用源ONE、目标INVSRCALPHA且alpha=1/255；雾和雨用screen混合，雨复用整层绘制前背景快照，不能逐雨滴累积背景。光照乘法在天气之后，HUD在其后。

演员增加m_drawnThisFrame，每帧重置，仅本帧成功绘制的远端实体参与灯光，并按人/NPC/怪物及有符号ID排序匹配参考map迭代；本地灯使用本地屏幕锚点，不依赖阴影开关。地图灯、人施法灯、升级灯、场景法术灯、实体灯分别接入。旧非单位暗度圆形/椭圆接口保留，现有场景调用默认路径已走参考采样。

首轮发现native_grid_triangles失败：Init与ClearSavedLightColor使用不同的等效三角顺序，已统一Init走ClearSavedLightColor。天气首帧数量断言错误地预期5/11，参考空池创建首项后processed>=active即中止，正确首帧是1；只修测试预期，未篡改模型。增加天气Quad测试后首次构建缺WeatherRenderer.h包含导致失败，已补齐。所有失败均修复后重跑。

最终清理旧天气成员后干净构建out/scene-weather-clean-final.log [87/87]与七目标out/scene-weather-all-final.log [109/109]通过。同版out/mud3-01/Mir2Ex-lighting-review.exe集中检查：光照天气29、地面24、施法76、键盘34、交易54、装备25、音效15，总257项全通过。网格检查比较342个顶点色与306个对角线；另检查渐变边界、内圈像素半径、雨背景快照、UV、透明像素、消息分派与粒子状态。diff空白检查通过。

预览243..248：lighting-day/night/dawn、weather-snow/rain/fog；实际01.map六张out同名PNG已经逐张查看。夜间含人物与火/冰光源，雪图包含参考伴随雾，雨图可见斜线与水花阶段。截图仅证明原生离线显示；尚未进行真实服务器天气切换、长期帧率验证或MIR3.EXE同帧像素对照。旧近战/武器光的特殊调用顺序仍待随近战完整迁移核对，不能以本批模型对照宣称所有战斗光效均完全一致。

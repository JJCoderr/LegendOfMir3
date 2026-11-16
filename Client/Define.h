#ifndef _DEFINE_H
#define _DEFINE_H




/******************************************************************************************************************
	#1. File Names
*******************************************************************************************************************/
#define MESSAGE_DLL_1				".\\Mirmg.dll"
#define MIR2_PATCH_FILE_NAME		".\\Mir2Patch.exe"




/******************************************************************************************************************
	#1. Procedeure Define
*******************************************************************************************************************/
#define _LOGIN_PROC					0
#define _SVR_SEL_PROC				1
#define _CHAR_SEL_PROC				2
#define _GAME_PROC					3





/******************************************************************************************************************
	#1. Mouse Wheel Define
*******************************************************************************************************************/
#ifndef WM_MOUSEWHEEL
#define WM_MOUSEWHEEL				WM_MOUSELAST+1 
#endif





/******************************************************************************************************************
	#1. User Define Message
*******************************************************************************************************************/
#define _WM_USER_MSG_INPUTTED		WM_USER + 1005				// 编辑框输入消息. 当消息为命令时，wParam 值被设置为 TRUE。
#define ID_SOCKCLIENT_EVENT_MSG		WM_USER + 1000				// 套接字事件消息.





/******************************************************************************************************************
	#1. Image Define Message
*******************************************************************************************************************/
#define _IMAGE_TILES				0
#define _IMAGE_TILES30				1
#define _IMAGE_TILES5				2
#define _IMAGE_SMTILES				3
#define _IMAGE_HOUSES				4
#define _IMAGE_CLIFFS				5
#define _IMAGE_DUNGEONS				6
#define _IMAGE_INNERS				7
#define _IMAGE_FUNITURES			8
#define _IMAGE_WALLS				9
#define _IMAGE_SMOBJECTS			10
#define _IMAGE_ANIMATIONS			11
#define _IMAGE_OBJECT1				12
#define _IMAGE_OBJECT2				13
#define _IMAGE_INTER				14
#define _IMAGE_M_HUMAN				15
#define _IMAGE_M_HAIR				16
#define _IMAGE_M_WEAPON1			17
#define _IMAGE_M_WEAPON2			18
#define _IMAGE_M_WEAPON3			19
#define _IMAGE_M_WEAPON4			20
#define _IMAGE_WM_HUMAN				21
#define _IMAGE_WM_HAIR				22
#define _IMAGE_WM_WEAPON1			23
#define _IMAGE_WM_WEAPON2			24
#define _IMAGE_WM_WEAPON3			25
#define _IMAGE_WM_WEAPON4			26
#define _IMAGE_MAGIC				27
#define _IMAGE_ITEM_INVENTORY		28
#define _IMAGE_ITEM_EQUIP			29
#define _IMAGE_ITEM_GROUND			30
#define _IMAGE_MICON				31
#define _IMAGE_PROGUSE				32
#define _IMAGE_HORSE				33
#define _IMAGE_MONSTER1				34
#define _IMAGE_MONSTER2				35
#define _IMAGE_MONSTER3				36
#define _IMAGE_MONSTER4				37
#define _IMAGE_MONSTER5				38
#define _IMAGE_MONSTER6				39
#define _IMAGE_MONSTER7				40
#define _IMAGE_MONSTER8				41
#define _IMAGE_MONSTER9				42
#define _IMAGE_MONSTER10			43
#define _IMAGE_MONSTER11			44
#define _IMAGE_MONSTER12			45
#define _IMAGE_SMONSTER1			46
#define _IMAGE_SMONSTER2			47
#define _IMAGE_SMONSTER3			48
#define _IMAGE_SMONSTER4			49
#define _IMAGE_SMONSTER5			50
#define _IMAGE_SMONSTER6			51
#define _IMAGE_SMONSTER7			52	
#define _IMAGE_SMONSTER8			53
#define _IMAGE_SMONSTER9			54
#define _IMAGE_SMONSTER10			55	
#define _IMAGE_SMONSTER11			56
#define _IMAGE_SMONSTER12			57
#define _IMAGE_NPC					58
#define _IMAGE_MONMAGIC				59
#define _IMAGE_INTEREX				60

#define _MAX_MONSTER_IMAGE			12

#define _MAX_IMAGE					61

#define _TEXTR_FILE_MAGIC			0
#define _TEXTR_FILE_MONMAGIC		1
#define _TEXTR_FILE_MAP				2
#define _TEXTR_FILE_INTER			3

#define _MAX_TEXTR_FILE				4




/******************************************************************************************************************
	#1. Item Define
*******************************************************************************************************************/
#define _ITEM_TYPE_INVENTORY		0
#define _ITEM_TYPE_EQUIP			1
#define _ITEM_TYPE_GROUND			2

#define _INVENTORY_CELL_WIDTH		38
#define _INVENTORY_CELL_HEIGHT		38

#define _CHAT_COLOR1				0	//RGB(  0,   0,   0); // 透明色.
#define _CHAT_COLOR2				1	//RGB( 10,  10,  10); // 黑色.
#define _CHAT_COLOR3				2	//RGB(255, 255, 255); // 白色.
#define _CHAT_COLOR4				3	//RGB(255,   0,   0); // 红色.
#define _CHAT_COLOR5				4	//RGB(  0, 255,   0); // 绿色.
#define _CHAT_COLOR6				5	//RGB(  0,   0, 255); // 蓝色
#define _CHAT_COLOR7				6	//RGB(255, 255,   0); // 黄色.
#define _CHAT_COLOR8				7	//RGB(255, 128,   0); // 橙色

#define _USER_NAMECLR1				255	//RGB(255, 255, 255); 白色
#define _USER_NAMECLR2				251	//RGB(255, 255,   0); 黄色
#define _USER_NAMECLR3				249	//RGB(255,   0,   0); 红色
#define _USER_NAMECLR4				125	//RGB(255,   0,   0); ??色(结界内怪物颜色)
#define _USER_NAMECLR5				47	//RGB(165,  99,  57); 棕色
#define _USER_NAMECLR6				180	//RGB(  0,   0, 255); 蓝色
#define _USER_NAMECLR7				69	//RGB(239, 107,   0); 橙色
#define _USER_NAMECLR8				221	//RGB(  0, 148,   0); 绿色
#define _USER_NAMECLR9				254	//RGB(  0, 255, 255); 颜色
#define _USER_NAMECLR10				147	//RGB(140, 214, 239); 颜色
#define _USER_NAMECLR11				154	//RGB( 57, 181, 239); 颜色
#define _USER_NAMECLR12				229	//RGB(119, 136, 255); 颜色
#define _USER_NAMECLR13				168	//RGB(  0, 123, 222); 颜色
#define _USER_NAMECLR14				252	//RGB(  0,   0, 255); 颜色




/******************************************************************************************************************
	#1. Actor Define
*******************************************************************************************************************/
#define _GENDER_MAN					0
#define _GENDER_WOMAN				1
#define _GENDER_NPC					50
#define _GENDER_MON					3//77
#define _MAX_GENDER					4

#define _TARGETRGN_GAPX				10
#define _TARGETRGN_GAPY				8

#define _MAX_HERO_KIND				10
#define _MAX_MON_KIND				255
#define _MAX_NPC_KIND				29

#define _MAX_DIRECTION				8
#define _DIRECTION_LIST_1			0
#define _DIRECTION_LIST_2			1
#define _DIRECTION_LIST_3			2
#define _DIRECTION_LIST_4			3
#define _DIRECTION_LIST_5			4
#define _DIRECTION_LIST_6			5	 
#define _DIRECTION_LIST_7			6
#define _DIRECTION_LIST_8			7	

#define _DEFAULT_SPELLFRAME			10
#define _DEFAULT_DELAYTIME			300

#define _HAIR_NONE					0
#define _MAX_HAIR					7

#define _WEAPON_NONE				0		
#define _MAX_WEAPON					32		

#define _HORSE_NONE					0
#define _MAX_HORSE					4

#define	_MAX_WEAPON_MTN				25
#define	_MAX_WEAPON_FRAME			3000

#define	_START_HORSE_FRAME			2320
#define	_START_HORSE_MTN			29
#define	_MAX_HORSE_FRAME			400

#define _MY_CENTER_XPOS				400
#define _MY_CENTER_YPOS	    		243

#define _MAX_CHAT_LINE				5
#define _CHAT_WIDTH					144
//#define _GAPX_TILE_CHAR_MAP		11
//#define _GAPY_TILE_CHAR_MAP		11 
#define _GAPX_TILE_CHAR_MAP			12
#define _GAPY_TILE_CHAR_MAP			12 
#define _CHAR_CENTER_XPOS			400
#define _CHAR_CENTER_YPOS	    	243

#define _SPEED_WALK					1
#define _SPEED_RUN					2
#define _SPEED_HORSERUN				3

#define _MOTION_LOCKTIME			8000
#define _WARMODE_TIME				5000

#define _JOB_DOSA					2
#define _JOB_SULSA					1
#define _JOB_JUNSA					0

#define _STATE_RED					64690
#define _STATE_GREEN				2016
#define _STATE_BLUE					38079//31
#define _STATE_YELLOW				65504
#define _STATE_FUCHSIA				64511
#define _STATE_GRAY					0
#define _STATE_ABLEND				1
#define _STATE_SHIELDUSE			2
#define _STATE_NOTUSED				0XFFFF

#define _RUN_MINHEALTH				10

#define _DIG_NORMAL					0
#define _DIG_UP						1
#define _DIG_DOWN					2
#define _DIG_DOWNDEL				3


/******************************************************************************************************************
	#1. Sprite Define
*******************************************************************************************************************/
// Hero 的动作定义。(定义游戏内的全部动作。)

#define _MAX_HERO_MTN				33			// 最大动作个数.
#define _MAX_HERO_FRAME				3000		// 切换到下一个 Hero 的最大帧数。
#define	_MAX_HERO_REALFRAME			2640

#define _MT_STAND					0			// 空闲动作。
#define _MT_ARROWATTACK				1			// 弓箭攻击。
#define _MT_SPELL1					2			// 魔法施放1.
#define _MT_SPELL2					3			// 魔法攻击施放2.
#define _MT_HOLD					4			// 忍耐.
#define _MT_PUSHBACK				5			// 被向后推开.
#define _MT_PUSHBACKFLY				6			// 向后被击飞推开。
#define _MT_ATTACKMODE				7			// 战斗模式.
#define _MT_CUT						8			// 切肉.
#define _MT_ONEVSWING				9			// 单手纵斩.(在动作第 9 号到第 14 号中，动作由武器种类决定.)
#define _MT_TWOVSWING				10			// 双手纵斩.
#define _MT_ONEHSWING				11			// 单手横斩.
#define _MT_TWOHSWING				12			// 双手横扫斩.	
#define _MT_SPEARVSWING				13			// 长枪纵向劈砍.
#define _MT_SPEARHSWING				14			// 枪横扫.
#define _MT_HITTED					15			// 受击.
#define _MT_WHEELWIND				16			// 旋转劈砍.
#define _MT_RANDSWING				17			// 乱砍.
#define _MT_BACKDROPKICK			18			// 回身踢.
#define _MT_DIE						19			// 死亡.
#define _MT_SPECIALDIE				20			// 跳起死亡.
#define _MT_WALK					21			// 行走.
#define _MT_RUN						22			// 奔跑.
#define _MT_MOODEPO					23			// 武太步跳跃.
#define _MT_ROLL					24			// 翻滚。
#define _MT_FISHSTAND				25			// 钓鱼模式空闲.
#define _MT_FISHHAND				26			// 钓鱼模式.
#define _MT_FISHTHROW				27			// 抛钓鱼线.
#define _MT_FISHPULL				28			// 拉钓鱼线.
#define _MT_HORSESTAND				29			// 骑马模式空闲.
#define _MT_HORSEWALK				30			// 骑马模式行走.
#define _MT_HORSERUN				31			// 骑马模式奔跑.
#define _MT_HORSEHIT				32			// 骑马模式受击.

// Monster 的动作定义。(定义游戏内的所有动作。)
#define _MAX_MON_MTN				10			// 最大动作个数.
#define _MAX_MON_FRAME				1000		// 切换到下一个 Mon 的最大帧数.

#define _MT_MON_STAND				0			// 空闲动作。
#define _MT_MON_WALK				1			// 行走动作。
#define _MT_MON_ATTACK_A			2			// 普通攻击动作1.
#define _MT_MON_HITTED				3			// 受击动作.
#define _MT_MON_DIE					4			// 死亡动作.
#define _MT_MON_ATTACK_B			5			// 普通攻击动作2。
#define _MT_MON_SPELL_A				6			// 魔法攻击动作1.
#define _MT_MON_SPELL_B				7			// 魔法攻击动作2.
#define _MT_MON_APPEAR				8			// 出现动作.
#define _MT_MON_SPECIAL_MTN			9			// 特殊动作1.


// NPC 的动作定义。(定义游戏内的所有动作。)
// NPC 相关定义.
#define _MAX_NPC_MTN				3			// 最大动作个数.
#define _MAX_NPC_FRAME				100			// 切换到下一个 NPC 的最大帧数.

#define _MT_NPC_STAND				0			// 空闲动作。
#define _MT_NPC_ACT01				1			// 特殊动作1.
#define _MT_NPC_ACT02				2			// 特殊动作2.

// 祈祷.							0			
// 武器铁匠.					1			
// 防具铁匠.					2			
// 客栈员工.						3			
// 计数器.							4			
// 拍卖向导。						5			
// 高级书吏.						6			
// 图书馆守卫.						7			
// 书吏.							8			
// 接待处.							9			
// 药草爷爷.					10			
// 服装商.							11			
// 酒馆NPC.							12			
// 沙漠战士。						13			
// 铁匠铺员工.					14			
// 赌场负责人.					15			
// 旧时铁匠.					16			
// 旧时药房老人.					17			
// 旧书店老人.					18			
// 杂货商人.							19			
// 杂货商.							20			
// 杂货商.							21			
// 杂货商.							22			
// 占卜师.							23			
// 布匹商.							24			
// 旅馆老板娘.						25
// 餐馆老翁.						26
// 肉店老板.						27			
// 行政官.							28			





// 魔法效果相关定义.
#define _MAX_EFFECT					33
#define _MAX_MAGIC					52
#define _MAX_EXPLOSION				8





/******************************************************************************************************************
	#1. Chat Popup Wnd
*******************************************************************************************************************/
#define _MAX_CHATLINE_POPUP			19





/******************************************************************************************************************
	#1. Button Wnd
*******************************************************************************************************************/
#define _BTN_STATE_NORMAL			0
#define _BTN_STATE_FOCUS			1
#define _BTN_STATE_CLICK			2

#define _BTN_TYPE_NOFOCUS			0
#define _BTN_TYPE_FOCUS				1





/******************************************************************************************************************
	#1. Game Procedure
*******************************************************************************************************************/
#define _CLIP_WIDTH					800
#define _CLIP_HEIGHT				492
#define _TIMER_ID					1

#define _ET_DIGOUTZOMBI				1  //僵尸从地里钻出的痕迹
#define _ET_MINE					2  //埋藏有矿石
#define _ET_PILESTONES				3  //石堆
#define _ET_HOLYCURTAIN				4  //结界
#define _ET_FIRE					5
#define _ET_SCULPEICE				6  //走马王的碎石块

#define _MAX_CELL_OBJ				10

// Day Bright.
#define _DAYSTATE_DUSK				0  // 黎明.
#define _DAYSTATE_DAY				1  // 白天.
#define _DAYSTATE_DAWN				2  // 傍晚.
#define _DAYSTATE_NIGHT				3  // 夜晚.

// Fog Bright.
#define _FOGSTATE_DAY				0  // 白天.
#define _FOGSTATE_NIGHT				1  // 夜晚.
#define _FOGSTATE_DAWN				2  // 清晨, 傍晚.





/******************************************************************************************************************
	#1. Interface
*******************************************************************************************************************/
#define _CHAT_COLOR1				0	//RGB(  0,   0,   0); // 透明色.
#define _CHAT_COLOR2				1	//RGB( 10,  10,  10); // 黑色.
#define _CHAT_COLOR3				2	//RGB(255, 255, 255); // 白色.
#define _CHAT_COLOR4				3	//RGB(255,   0,   0); // 红色.
#define _CHAT_COLOR5				4	//RGB(  0, 255,   0); // 绿色.
#define _CHAT_COLOR6				5	//RGB(  0,   0, 255); // 蓝色
#define _CHAT_COLOR7				6	//RGB(255, 255,   0); // 黄色.
#define _CHAT_COLOR8				7	//RGB(255, 128,   0); // 橙色

#define _INPUT_EDITWND_XPOS			185
#define _INPUT_EDITWND_YPOS			575
#define _INPUT_EDITWND_WIDTH		344
#define _INPUT_EDITWND_HEIGHT		15

// 主界面中使用的窗口ID.
#define _WND_ID_INVENTORY			0			//背包窗口.
#define _WND_ID_STATUS				1			//角色信息查看窗口(3个)。
#define _WND_ID_STORE				2			//商店窗口.
#define _WND_ID_EXCHANGE			3			//交易窗口.
#define _WND_ID_GUILD				4			//文元窗口.
#define _WND_ID_GUILDMASTER			5			//门主窗口.
#define _WND_ID_GROUP				6			//组窗口。
#define _WND_ID_GROUPPOP			7			//
#define _WND_ID_CHATPOP				8			//
#define _WND_ID_NPCCHAT				9		
#define _WND_ID_MESSAGEBOX			10			// MessageBox
#define _WND_ID_QUEST				11			// 
#define _WND_ID_OPTION				12			// 
#define _WND_ID_HORSE				13			// 

// 主界面中使用的窗口的图像索引.
#define _WNDIMGIDX_MAIN				1160			//主界面聊天窗口
#define _WNDIMGIDX_INVENTORY	    250			//背包
#define _WNDIMGIDX_ITEMSET		    200			//
#define _WNDIMGIDX_SKILLSET			202			//武功设置窗口
#define _WNDIMGIDX_STATUS			201			//
#define _WNDIMGIDX_CHARINFO			201			//
#define _WNDIMGIDX_STORE			253			//商店购买物品
#define _WNDIMGIDX_EXCHANGE			251			//交易窗口
#define _WNDIMGIDX_GUILDMASTER		600			//门主公告板.
#define _WNDIMGIDX_GUILD			169			//文原公告板。
#define _WNDIMGIDX_GROUP			900			//组队窗口.
#define _WNDIMGIDX_GROUPPOP			145			//组队输入窗口.
#define _WNDIMGIDX_CHATPOP			350			//聊天弹出窗口.
#define _WNDIMGINDX_MSGBOX1			254
#define _WNDIMGINDX_MSGBOX2			255
#define _WNDIMGIDX_NPCCHAT			300			//NPC对话窗口
#define _WNDIMGIDX_QUEST			700			//任务窗口.
#define _WNDIMGIDX_OPTION			750			//选项窗口.
#define _WNDIMGIDX_HORSE			850			//聊天框.
#define _IDX_IMG_NOTICEBOX			0			//公告窗口索引

#define _IMGIDX_BELT				51

// 主界面中使用的按钮 ID.
#define _BTN_ID_CHANGE				0
#define _BTN_ID_MINIMAP				1
#define _BTN_ID_MUGONG				2
#define _BTN_ID_EXIT				3
#define _BTN_ID_LOGOUT				4
#define _BTN_ID_GROUP				5
#define _BTN_ID_GUILD				6
#define _BTN_ID_BELT				7
#define _BTN_ID_MAGIC				8
#define _BTN_ID_POPUP				9
#define _BTN_ID_QUEST				10
#define _BTN_ID_OPTION				11
#define _BTN_ID_HELP				12
#define _BTN_ID_HORSE				13
#define _BTN_ID_BAG					14
#define _BTN_ID_CHAR				15

#define _MAX_INTER_BTN				16
#define _MAX_CHATLINE				5

#define _MAX_SAVECHATLINE			50

#define _BELT_STATE_STAY			0
#define _BELT_STATE_UP				1
#define _BELT_STATE_DOWN			2











/******************************************************************************************************************
	#1. Inventory Window
*******************************************************************************************************************/
#define _INVEN_MAX_CELL				25
#define _BELT_MAX_CELL				6
#define _INVEN_CELL_XCNT			5
#define _INVEN_CELL_YCNT			5
#define _INVEN_MAX_CELL_YCNT		12
#define _INVEN_CELL_XSTART			44
#define _INVEN_CELL_YSTART			55

#define _BELT_CELL_XGAP				40
#define _BELT_CELL_XSTART			18
#define _BELT_CELL_YSTART			220

#define _MAX_INVEN_BTN				1//2
#define _MAX_INVEN_ITEM				60
#define _MAX_BELT_ITEM				6

#define _BTN_ID_INVENCLOSE			0
//#define _BTN_ID_INVENSCRLUP		1
//#define _BTN_ID_INVENSCRLDOWN		2





/******************************************************************************************************************
	#1. Light fog
*******************************************************************************************************************/
#define NUM_CNTX_LIGHTTILE			17
#define NUM_CNTY_LIGHTTILE			21
#define TILE_START_XPOS				-8
#define TILE_START_YPOS				-29

#define TILE_WIDTH					48
#define TILE_HEIGHT					32

#define NUM_LIGHTTILE_INDICES		(NUM_CNTX_LIGHTTILE)*(NUM_CNTY_LIGHTTILE)*3*2
#define NUM_LIGHTTILE_VERTICES		(NUM_CNTX_LIGHTTILE+1)*(NUM_CNTY_LIGHTTILE+1)



/******************************************************************************************************************
	#1. Magic
*******************************************************************************************************************/
#define _MAGIC_ACTIVE				0			// 魔法激活状态。
#define _LIGHT_ACTIVE				1			// 魔法光源激活状态.
#define _MAGIC_FINISH				2			// 魔法消失.

#define _SKILL_FIREBALL				1			// 火焰场.
#define _SKILL_HEALLING				2			// 恢复术.
#define _SKILL_ONESWORD				3			// 外修.
#define _SKILL_ILKWANG				4			// 日光.
#define _SKILL_FIREBALL2			5			// 金刚火焰掌.
#define _SKILL_AMYOUNSUL			6			// 暗然术.
#define _SKILL_YEDO					7			// 锐刀.
#define _SKILL_FIREWIND				8			// 火焰风.
#define _SKILL_FIRE					9			// 盐社长.
#define _SKILL_SHOOTLIGHTEN			10			// 雷印.
#define _SKILL_LIGHTENING			11			// 强击.
#define _SKILL_ERGUM				12			// 御剑术.
#define _SKILL_FIRECHARM			13			// 爆杀系.
#define _SKILL_HANGMAJINBUB			14			// 降魔阵法.
#define _SKILL_DEJIWONHO			15			// 大地援护.
#define _SKILL_HOLYSHIELD			16			// 结界.
#define _SKILL_SKELLETON			17			// 白骨召唤术.
#define _SKILL_CLOAK				18			// 隐身.
#define _SKILL_BIGCLOAK				19			// 大隐身.
#define _SKILL_TAMMING				20			// 雷魂击.
#define _SKILL_SPACEMOVE			21			// 我空行法.
#define _SKILL_EARTHFIRE			22			// 地炎术。
#define _SKILL_FIREBOOM				23			// 爆裂波.
#define _SKILL_LIGHTFLOWER			24			// 雷雪火.
#define _SKILL_BANWOL				25			// 半月.
#define _SKILL_FIRESWORD			26			// 焰火诀.
#define _SKILL_MOOTEBO				27			// 武太步。
#define _SKILL_SHOWHP				28			// 探气破延.
#define _SKILL_BIGHEALLING			29			// 大恢复.
#define _SKILL_SINSU				30			// 神兽召唤.
#define _SKILL_SHIELD				31			// 咒术之幕.
#define _SKILL_KILLUNDEAD			32			// 狮子轮回.
#define _SKILL_SNOWWIND				33			// 冰旋风.

#define _SKILL_JUMPSHOT				33			// 跳跃劈砍 剑气.
#define _SKILL_RANDSWING			34			// 胡乱挥舞。
#define _SKILL_SPACEMOVE2			35			// 阿空行法2。
#define _SKILL_SPACEMOVE3			36			// 阿空行法3.

#define _MONMAGIC_KEPAL				100			// 电击攻击.(在攻击的角色上方)(魔法)
#define _MONMAGIC_EXPLODE			101			// 爆炸魔法.(石头傀儡死亡时, 隆攻击时)
#define _MONMAGIC_BLACK1			102			// 黑魔法师。(魔法(施放))
#define _MONMAGIC_BLACK2			103			// 黑魔法师. (魔法(魔法))
#define _MONMAGIC_DIE				104			// 黑魔法师, 轻甲骑士, 红魔法师, 神机船BOSS变身前死亡.(效果)
#define _MONMAGIC_ANTHEAL			105			// 治愈蚂蚁治疗(发射魔法, 爆炸效果.)
#define _MONMAGIC_GREATANT			106			// 巨大蚂蚁.(攻击的角色上方)(魔法)
#define _MONMAGIC_MAGENUMA			108			// 结界.
#define _MONMAGIC_ANT				109			// 兵蚁攻击.(在攻击的角色上方)(魔法)
#define _MONMAGIC_WORKANT			110			// 工蚁攻击.(在攻击的角色上方)(魔法)
#define _MONMAGIC_RED1				111			// 红魔法师. (魔法(施放))
#define _MONMAGIC_RED2				112			// 红魔法师。(魔法(魔法))
#define _MONMAGIC_COWGHOST			113			// 牛面鬼. (魔法)
#define _MONMAGIC_SINSU				114			// 神兽. (魔法)
#define _MONMAGIC_ZOMBIE			115			// 僵尸. (魔法)
#define _MONMAGIC_JUMAWANG			116			// 走马王. (魔法)
#define _MONMAGIC_CHIM				117
#define _MONMAGIC_ARROW				118
#define _MONMAGIC_MAARROW			119
#define _MONMAGIC_DUALAXE			120
#define _MONMAGIC_COWFLAME			121
#define _MONMAGIC_BIGGINE_ATT		122
#define _MONMAGIC_BIGGINE_CHAR		123
#define _MONMAGIC_SANDFISH			124
#define _MONMAGIC_BAODIE			125
#define _MONMAGIC_SINGI_DIE			126
#define _MONMAGIC_HUSU_DIE			127
#define _MONMAGIC_ZOMBIE_DIE		128
#define _MONMAGIC_SSEGI				129
#define _MONMAGIC_NUMAGUMGI			130
#define _MONMAGIC_EXPLODE1			131

#define _BLEND_NORMAL				0			// 一般绘制。（可应用 alpha 值。）
#define _BLEND_LIGHT				1			// 闪光效果形态的混合.
#define _BLEND_LIGHTINV				2	
#define _BLEND_INVNORMAL			3
#define _BLEND_INVLIGHT				4
#define _BLEND_INVLIGHTINV			5




/******************************************************************************************************************
	#1. Map Handler
*******************************************************************************************************************/

#define _BASETILE_WIDTH				96				// 基本图块宽度。
#define _BASETILE_HEIGHT			64
	
#define _CELL_WIDTH					48				// 默认单元格宽度, 默认图块由 4 个单元格组成, 信息以单元格为单位保存.
#define _CELL_HEIGHT				32

#define _VIEW_CELL_X_COUNT			24				// 视图区域所需的 X 轴单元格数量.((不含零散图块)左侧和上方留 3 个单元格, 右侧和下方各留 2 个单元格的余量.)
#define _VIEW_CELL_Y_COUNT			24			
#define _VIEW_CELL_Y_COUNT_FOR_OBJ	44

#define _VIEW_CELL_X_START			-200			// 用于绘制视图区域的起始单元格 X 坐标。
#define _VIEW_CELL_Y_START			-157
#define _MAX_TILE_FILE				14

// 雾色相关属性。
#define _FOG_COLOR_TYPE0			RGB(0, 0, 0);
#define _FOG_COLOR_TYPE1			RGB(100, 10, 10);
#define _FOG_COLOR_TYPE2			RGB(10, 255, 10);
#define _FOG_COLOR_TYPE3			RGB(10, 10, 255);

// 属性相关定义.
#define _CAN_WALK					0
#define _CAN_NOTWALK	 			1
#define _CAN_FLY					0
#define _CAN_NOTFLY					1

// 光照大小相关定义.
#define _LIGHT_SIZE_TYPE0			3
#define _LIGHT_SIZE_TYPE1			4
#define _LIGHT_SIZE_TYPE2			5
#define _LIGHT_SIZE_TYPE3			6

// 光照颜色相关定义.
#define _LIGHT_COLOR_TYPE0			RGB(255, 0, 0);
#define _LIGHT_COLOR_TYPE1			RGB(0, 255, 0);
#define _LIGHT_COLOR_TYPE2			RGB(0, 0, 255);
#define _LIGHT_COLOR_TYPE3			RGB(255, 0, 255);

// 行会相关定义.
#define _DOOR_ISCLOSED				0
#define _DOOR_ISOPEN				1

#define _TILE_ANI_DELAY_1			150
#define _TILE_ANI_DELAY_2			200
#define _TILE_ANI_DELAY_3			250
#define _TILE_ANI_DELAY_4			300
#define _TILE_ANI_DELAY_5			350
#define _TILE_ANI_DELAY_6			400
#define _TILE_ANI_DELAY_7			420
#define _TILE_ANI_DELAY_8			450






/******************************************************************************************************************
	#1. Status Window
*******************************************************************************************************************/
#define _TYPE_EQUIP					0
#define _TYPE_STATUS				1
#define _TYPE_SETMAGIC				2

#define _EQUIP_MAX_CELL				10
#define _MAGIC_MAX_CELL				5
#define _MAGIC_MAX_KEY				8

#define _MAX_TYPE_WND				3

#define _MAX_STAUTS_BTN				3

#define _BTN_ID_STATUSCLOSE			0
#define _BTN_ID_STATUS				1
#define _BTN_ID_STATUSMAGIC			2

#define _BTN_ID_STATUS_X			187
#define _BTN_ID_STATUS_Y			266
#define _BTN_ID_STATUSMAGIC_X		224
#define _BTN_ID_STATUSMAGIC_Y		98

#define _EQUIP_CHAR_X				100
#define _EQUIP_CHAR_Y				200

#define _U_DRESS					0 
#define _U_WEAPON					1 
#define _U_RIGHTHAND				2 
#define _U_NECKLACE					3 
#define _U_HELMET					4 
#define _U_ARMRINGL					5 
#define _U_ARMRINGR					6                
#define _U_RINGL					7 
#define _U_RINGR					8 
#define _U_CHARM					9
#define _U_UNEQUIP					100

#define _MAX_MAGIC_SKILL			32




/******************************************************************************************************************
	#1. Store Window
*******************************************************************************************************************/
#define _STORE_STATE_SELL			0
#define _STORE_STATE_BUY			1






/******************************************************************************************************************
	#1. Client To Server Message
*******************************************************************************************************************/
#define CM_QUERYUSERNAME			80
#define CM_QUERYBAGITEMS			81
#define CM_DROPITEM					1000
#define CM_PICKUP					1001
#define CM_TAKEONITEM				1003
#define CM_TAKEOFFITEM				1004
#define CM_EAT						1006
#define CM_MAGICKEYCHANGE			1008
#define CM_LOGINNOTICEOK			1018
#define CM_GROUPMODE				1019	// ***************************************
#define CM_CREATEGROUP				1020	// ***************************************
#define CM_ADDGROUPMEMBER			1021	// ***************************************
#define CM_DELGROUPMEMBER			1022	// ***************************************
#define CM_TRUN						3010
#define CM_WALK						3011
#define CM_RUN						3013
#define CM_HIT						3014
#define CM_LONGHIT					3019
#define CM_POWERHIT					3018
#define CM_SPELL					3017	// 发送后，若服务器返回许可则执行动作。法术动作结束后，若收到 sm_magicfire 则把动作切换为站立。
#define CM_WIDEHIT					3024
#define CM_FIREHIT					3025
#define CM_SAY						3030
#define CM_RIDE						3031

#define CM_BUTCH					1007
#define CM_SITDOWN					3012
#define	CM_CLICKNPC					1010
#define	CM_OPENDOOR					1002


//勇气
////////////////////////////////////////////////////////////////////////////////////////////////////////
// 通用
#define	CM_CLICKNPC					1010	// 将此消息发送到服务器，以获取该 MPC 的信息。
#define	CM_MERCHANTDLGSELECT		1011	// 当用户在 NPC 对话中点击存在 Tag 的部分时会发送.
////////////////////////////////////////////////////////////////////////////////////////////////////////
// 出售
#define	CM_MERCHANTQUERYSELLPRICE	1012	// 上架要出售的物品时会得到价格.  
											// 需要修改 (过去: 只显示出售物品的价格, 当前: 显示所有出售物品的价格)
#define CM_USERSELLITEM				1013	// 按下卖出按钮的情况 (过去: 只发送一件要卖出的物品, 当前: 发送全部要卖出的物品
////////////////////////////////////////////////////////////////////////////////////////////////////////
// 购买
#define CM_USERBUYITEM				1014	// 按下购买按钮时
#define CM_USERGETDETAILITEM		1015	// 当在商店列表中点击带有 Sub Menu 的项目时
////////////////////////////////////////////////////////////////////////////////////////////////////////
// 修理
#define	CM_USERREPAIRITEM			1023	// 按下修理按钮时
#define	CM_MERCHANTQUERYREPAIRCOST	1024	// 放入要修理的物品时获得价格.
////////////////////////////////////////////////////////////////////////////////////////////////////////
// 药房
#define CM_USERMAKEDRUGITEM			1034	// 

////////////////////////////////////////////////////////////////////////////////////////////////////////
// 客栈
#define	CM_USERSTORAGEITEM			1031	// 用户按下委托按钮(非文本按钮)时发送。
#define	CM_USERTAKEBACKSTORAGEITEM	1032	// 用户按下查找按钮(非文本按钮)时发送.
////////////////////////////////////////////////////////////////////////////////////////////////////////
// 行会
#define	CM_OPENGUILDDLG				1035	// 当用户想要打开行会窗口时发送。
#define CM_GUILDHOME				1036	// 用户按下行会窗口中的 HOME 按钮时发送。
#define	CM_GUILDMEMBERLIST			1037	// 用户想要查看行会窗口中的成员 LIST 时
#define CM_GUILDADDMEMBER			1038	// 当用户添加行会成员时
#define	CM_GUILDDELMEMBER			1039	// 当用户删除行会成员时
#define	CM_GUILDUPDATENOTICE		1040	// 用户修改了行会公告时
#define	CM_GUILDUPDATERANKINFO		1041	// 用户修改行会成员职位时
#define CM_GUILDMAKEALLY			1044	// 用户进行行会结盟的情况
#define CM_GUILDBREAKALLY			1045	// 用户解除行会结盟时



/******************************************************************************************************************
	#1. Server To Client Message
*******************************************************************************************************************/
#define SM_RUSH						6		// 武太步推开.
#define SM_FIREHIT					8
#define SM_POWERHIT					18
#define SM_BACKSTEP					9
#define SM_TURN						10
#define SM_WALK						11
#define SM_RUN						13
#define SM_HIT						14
#define SM_SPELL					17		// 进行施放。
#define SM_LONGHIT					19
#define SM_DIGUP					20
#define SM_DIGDOWN					21
#define SM_FLYAXE					22
#define SM_LIGHTING					23
#define SM_WIDEHIT					24
#define SM_DISAPPEAR				30
#define SM_STRUCK					31
#define SM_DEATH					32
#define SM_NOWDEATH					34
#define SM_HEAR						40
#define SM_FEATURECHANGED			41
#define SM_USERNAME					42
#define SM_WINEXP					44		
#define SM_LEVELUP					45
#define SM_DAYCHANGING				46
#define SM_LOGON					50
#define SM_NEWMAP					51
#define SM_ABILITY					52 
#define SM_HEALTHSPELLCHANGED		53
#define SM_MAPDESCRIPTION			54

#define SM_SYSMESSAGE				100
#define SM_GROUPMESSAGE				101
#define SM_CRY						102
#define SM_WHISPER					103
#define SM_GUILDMESSAGE				104

#define SM_ADDITEM					200
#define SM_BAGITEMS					201
#define SM_ADDMAGIC					210
#define SM_SENDMYMAGIC				211

#define SM_DROPITEM_SCCESS			600
#define SM_DROPITEM_FAIL			601
#define SM_ITEMSHOW					610
#define SM_ITEMHIDE 				611
#define SM_TAKEON_OK				615
#define SM_TAKEON_FAIL				616
#define SM_DOOROPEN					612
#define SM_TAKEOFF_OK				619
#define SM_TAKEOFF_FAIL				620
#define SM_SENDUSEITEMS				621
#define SM_WEIGHTCHANGED			622
#define SM_CHANGEMAP				634
#define SM_CLEAROBJECT				633
#define SM_EAT_OK					635
#define SM_EAT_FAIL					636
#define SM_MAGICFIRE				638		// 魔法被执行.
#define SM_MAGIC_LVEXP				640
#define SM_DURACHANGE				642
#define SM_GOLDCHANGE				653
#define SM_CHANGELIGHT				654
#define SM_CHANGENAMECOLOR			656
#define SM_CHARSTATUSCHANGE			657
#define SM_SENDNOTICE				658
#define SM_CREATEGROUP_OK			660		// ******************************************************************
#define SM_CREATEGROUP_FAIL			661		// ******************************************************************
#define SM_GROUPCANCEL				666		// ******************************************************************
#define SM_GROUPMEMBERS				667		// ******************************************************************

#define SM_CHANGEGUILDNAME			750
#define SM_SUBABILITY				752

#define SM_SHOWEVENT				804
#define SM_HIDEEVENT				805

#define SM_DELITEM					202
#define SM_DELITEMS					203

#define SM_CLOSEHEALTH				1101
#define SM_OPENHEALTH				1100
#define SM_MONSTERSAY				1501
#define SM_SITDOWN					12
#define SM_BUTCH					637
#define SM_SKELETON					33		// 与 SM_DEATH 相同.
#define SM_CHANGEFACE				1104;

#define SM_OPENDOOR_OK				612
#define SM_OPENDOOR_LOCK			613
#define SM_CLOSEDOOR				614

#define SM_MERCHANTSAY				643
#define	SM_MERCHANTDLGCLOSE			644
#define SM_SENDGOODSLIST			645
#define SM_SENDUSERSELL				646
#define	SM_SENDBUYPRICE				647
#define SM_USERSELLITEM_OK			648
#define SM_USERSELLITEM_FAIL		649
#define	SM_BUYITEM_SUCCESS			650
#define SM_BUYITEM_FAIL				651
#define SM_SENDDETAILGOODSLIST		652
#define	SM_GOLDCHANGED				653
#define	SM_OPENGUILDDLG				753
#define	SM_OPENGUILDDLG_FAIL		754
#define	SM_SENDGUILDHOME			755
#define	SM_SENDGUILDMEMBERLIST		756
#define	SM_GUILDADDMEMBER_OK		757
#define	SM_GUILDADDMEMBER_FAIL		758
#define	SM_GUILDDELMEMBER_OK		759
#define	SM_GUILDDELMEMBER_FAIL		760
#define	SM_GUILDRANKUPDATE_FAIL		761
#define	SM_BUILDGUILD_OK			762
#define	SM_BUILDGUILD_FAIL			763
#define	SM_GUILDMAKEALLY_OK			768
#define	SM_GUILDMAKEALLY_FAIL		769
#define	SM_GUILDBREAKALLY_OK		770
#define	SM_GUILDBREAKALLY_FAIL		771
#define	SM_DLGMSG					772


#define	SM_AREASTATE				708

#define	SM_RIDEHORSE				1300

/******************************************************************************************************************
	Resource Dll string index
*******************************************************************************************************************/
#define PUNISHMENT_ID_USING             1
#define PUNISHMENT_ID_THEFT             2
#define PUNISHMENT_DISTURB_GAME         3
#define PUNISHMENT_BBS_CURSES           4
#define PUNISHMENT_CONFIRMED_CURSES     5
#define PUNISHMENT_USAGE_BUG            6
#define PUNISHMENT_ASSUMED_GAMEMASTER   7
#define PUNISHMENT_SPREAD_FALLACY       8
#define NEW_ACCOUNT_HELP_ID             9
#define NEW_ACCOUNT_HELP_PWD            10
#define NEW_ACCOUNT_HELP_REPWD          11
#define NEW_ACCOUNT_HELP_NAME           12
#define NEW_ACCOUNT_HELP_SSNO           13
#define NEW_ACCOUNT_HELP_BIRTH          14
#define NEW_ACCOUNT_HELP_ZIPCODE        15
#define NEW_ACCOUNT_HELP_ADDRESS        16
#define NEW_ACCOUNT_HELP_PHONE          17
#define NEW_ACCOUNT_HELP_MOBILEPHONE    18
#define NEW_ACCOUNT_HELP_EMAIL          19
#define NEW_ACCOUNT_HELP_Q              20
#define NEW_ACCOUNT_HELP_A              21
#define NEW_ACCOUNT_WELL_DONE           22
#define ERROR_STR_LOGIN_1               23
#define ERROR_STR_LOGIN_2               24
#define ERROR_STR_LOGIN_3               25
#define ERROR_STR_LOGIN_4               26
#define ERROR_STR_NEW_ACCOUNT_1         27
#define ERROR_STR_NEW_ACCOUNT_2         28
#define ERROR_STR_NEW_ACCOUNT_3         29
#define ERROR_STR_NEW_ACCOUNT_4         30
#define ERROR_STR_NEW_ACCOUNT_5         31
#define ERROR_STR_NEW_ACCOUNT_6         32
#define ERROR_STR_NEW_ACCOUNT_7         33
#define ALRAM_CHARGE                    34
#define ALRAM_CHARGE_IP                 35
#define ALRAM_CHARGE_1                  36
#define ALRAM_CHARGE_2                  37
#define ALRAM_CHARGE_3                  38
#define CHANGE_PASSWORD                 39
#define CHANGE_PASSWORD_1               40
#define CHANGE_PASSWORD_2               41
#define CHANGE_PASSWORD_3               42
#define CHANGE_PASSWORD_4               43
#define ERROR_STR_SELECT_SERVER_1       44
#define ERROR_STR_SELECT_SERVER_2       45
#define ERROR_STR_NEWCHR_1              200
#define ERROR_STR_NEWCHR_2              201
#define ERROR_STR_NEWCHR_3              202
#define DELETE_CHARACTER_1              203
#define DELETE_CHARACTER_2              204
#define VERSION_NOT_MATCHED             205
#define ERROR_STR_START_GAME_1          206
#define ERROR_STR_DELCHR                207
#define ERROR_STR_PATCH_1               300
#define ERROR_STR_PATCH_2               301
#define ERROR_STR_SERVER_CONNECTION     302
#define NOTICE_NOT_EXIST_INDEX          400
#define NOTICE_LAST_PAGE                401
#define FATAL_ERROR_STR                 9999
#define SERVER_SELECT_TEST              10000
//////////////////////////////////////////////////////////////////////////////////////////////////////



/*******************************************************************************************************
		Login Procedure
*******************************************************************************************************/
#define _LOGIN_SCREEN_WIDTH		640
#define _LOGIN_SCREEN_HEIGHT	480


/******************************************************************************************************************
	#1. Progress States
*******************************************************************************************************************/
#define PRG_CONNECT					 -1
#define PRG_INTRO					 0
#define PRG_LOGIN					 1
#define PRG_NEW_ACCOUNT				 3
#define PRG_PATCH					 4
#define PRG_CHANGE_PASS				 5
#define	PRG_SERVER_SELE				 6
#define PRG_TO_SELECT_CHR			 7
#define	PRG_CHAR_SELE				 8
#define	PRG_CREATE_CHAR				 9
#define PRG_PLAY_GAME				10

#define PRG_SEL_TO_CREA				30
#define PRG_CREA_TO_SEL				31
#define PRG_SEL_TO_GAME				32

#define PRG_PATCH_FILES				77
#define PRG_NEED_ACCOUNT_CHANGE		88
#define PRG_QUIT					99



#endif // _DEFINE_H
























# 原版游戏消息入口清单

本表由4282C0的Hex-Rays语法树提取opcode switch及显式相等判断；原始结构见[client-entry-inventory.json](client-entry-inventory.json)。只统计消息编号选择器，不把WORD1/WORD2参数子分支误算成消息。

本次取得196个不同消息编号。范围不包括仅由范围条件隐含的分支、登录/选角独立分派或运行时不可达性；因此不是“完整协议已经还原”的证明。

“有引用”只表示当前C++源码出现该命名常量，不证明正确接线或功能完成。无命名项保留编号和原版地址，禁止凭数字猜名称后硬编码实现。

| 编号 | 当前定义名 | 原版入口 | 当前源码线索 |
| --- | --- | --- | --- |
| 6 | SM_RUSH | 0x4283e5（共享分支） | 未见对应命名引用；需核对旧数字分支/缺失处理 |
| 7 | SM_BUMP | 0x4283e5（共享分支） | 有引用：client/src/godot/hud_view.cpp, client/src/hud/scene_human_queue.hpp |
| 8 | SM_FIREHIT | 0x4283e5（共享分支） | 未见对应命名引用；需核对旧数字分支/缺失处理 |
| 9 | SM_BACKSTEP | 0x4283e5（共享分支） | 未见对应命名引用；需核对旧数字分支/缺失处理 |
| 10 | SM_TURN | 0x4283e5（共享分支） | 有引用：client/src/godot/hud_view.cpp, client/src/hud/scene_human_queue.hpp |
| 11 | SM_WALK | 0x4283e5（共享分支） | 有引用：client/src/godot/hud_view.cpp, client/src/hud/scene_human_queue.hpp |
| 12 | SM_SITDOWN | 0x4283e5（共享分支） | 未见对应命名引用；需核对旧数字分支/缺失处理 |
| 13 | SM_RUN | 0x4283e5（共享分支） | 有引用：client/src/godot/hud_view.cpp, client/src/hud/scene_human_queue.hpp |
| 14 | SM_HIT | 0x4283e5（共享分支） | 有引用：client/src/godot/hud_view.cpp, client/src/hud/scene_human_queue.hpp |
| 15 | SM_HIT2 | 0x4283e5（共享分支） | 有引用：client/src/godot/hud_view.cpp, client/src/hud/scene_human_queue.hpp |
| 17 | SM_SPELL | 0x4283e5（共享分支） | 未见对应命名引用；需核对旧数字分支/缺失处理 |
| 18 | SM_POWERHIT | 0x4283e5（共享分支） | 未见对应命名引用；需核对旧数字分支/缺失处理 |
| 19 | SM_LONGHIT | 0x4283e5（共享分支） | 未见对应命名引用；需核对旧数字分支/缺失处理 |
| 20 | SM_DIGUP | 0x4283e5（共享分支） | 未见对应命名引用；需核对旧数字分支/缺失处理 |
| 21 | SM_DIGDOWN | 0x4283e5（共享分支） | 未见对应命名引用；需核对旧数字分支/缺失处理 |
| 22 | SM_FLYAXE | 0x4283e5（共享分支） | 未见对应命名引用；需核对旧数字分支/缺失处理 |
| 23 | SM_LIGHTING | 0x4283e5（共享分支） | 未见对应命名引用；需核对旧数字分支/缺失处理 |
| 24 | SM_WIDEHIT | 0x4283e5（共享分支） | 未见对应命名引用；需核对旧数字分支/缺失处理 |
| 25 | 待定名 | 0x4283e5（共享分支） | 未见对应命名引用；需核对旧数字分支/缺失处理 |
| 27 | 待定名 | 0x4283e5（共享分支） | 未见对应命名引用；需核对旧数字分支/缺失处理 |
| 29 | 待定名 | 0x4283e5（共享分支） | 未见对应命名引用；需核对旧数字分支/缺失处理 |
| 30 | SM_DISAPPEAR | 0x4283e5（共享分支） | 有引用：client/src/godot/hud_view.cpp |
| 31 | SM_STRUCK | 0x4283e5（共享分支） | 有引用：client/src/godot/hud_view.cpp, client/src/hud/scene_human_queue.hpp |
| 32 | SM_DEATH | 0x4283e5（共享分支） | 有引用：client/src/godot/hud_view.cpp, client/src/hud/scene_human_queue.hpp |
| 33 | SM_SKELETON | 0x4283e5（共享分支） | 未见对应命名引用；需核对旧数字分支/缺失处理 |
| 34 | SM_NOWDEATH | 0x4283e5（共享分支） | 有引用：client/src/godot/hud_view.cpp, client/src/hud/scene_human_queue.hpp |
| 40 | SM_HEAR | 0x4283e5（共享分支） | 有引用：client/src/chat/scene_chat.hpp, client/src/godot/hud_view.cpp, client/src/hud/speech_bubble.hpp |
| 41 | SM_FEATURECHANGED | 0x429348 | 未见对应命名引用；需核对旧数字分支/缺失处理 |
| 42 | SM_USERNAME | 0x4293df | 有引用：client/src/godot/hud_view.cpp, client/src/hud/scene_name.hpp |
| 44 | SM_WINEXP | 0x428408 | 有引用：client/src/protocol/actor_experience.hpp |
| 45 | SM_LEVELUP | 0x42884b | 有引用：client/src/godot/hud_view.cpp |
| 46 | SM_DAYCHANGING | 0x4291ac | 有引用：client/src/godot/hud_view.cpp |
| 47 | 待定名 | 0x428a85 | 未见对应命名引用；需核对旧数字分支/缺失处理 |
| 50 | SM_LOGON | 0x4283e5（共享分支） | 有引用：client/src/godot/hud_view.cpp, client/src/protocol/login_actor.hpp |
| 51 | SM_NEWMAP | 0x4283e5（共享分支） | 有引用：client/src/godot/hud_view.cpp, client/src/protocol/map_message.hpp |
| 52 | SM_ABILITY | 0x4283e5（共享分支） | 有引用：client/src/godot/hud_view.cpp, client/src/protocol/actor_ability.hpp |
| 53 | SM_HEALTHSPELLCHANGED | 0x429462 | 有引用：client/src/godot/hud_view.cpp, client/src/hud/scene_health.hpp |
| 54 | SM_MAPDESCRIPTION | 0x4283fe | 有引用：client/src/godot/hud_view.cpp |
| 100 | SM_SYSMESSAGE | 0x4283e5（共享分支） | 有引用：client/src/chat/chat_log.hpp, client/src/chat/scene_chat.hpp |
| 101 | SM_GROUPMESSAGE | 0x4283e5（共享分支） | 未见对应命名引用；需核对旧数字分支/缺失处理 |
| 102 | SM_CRY | 0x4283e5（共享分支） | 未见对应命名引用；需核对旧数字分支/缺失处理 |
| 103 | SM_WHISPER | 0x4283e5（共享分支） | 未见对应命名引用；需核对旧数字分支/缺失处理 |
| 104 | SM_GUILDMESSAGE | 0x4283e5（共享分支） | 有引用：client/src/chat/chat_log.hpp, client/src/chat/scene_chat.hpp, client/src/godot/hud_view.cpp |
| 105 | 待定名 | 0x4292d9 | 未见对应命名引用；需核对旧数字分支/缺失处理 |
| 200 | SM_ADDITEM | 0x428626 | 有引用：client/src/godot/hud_view.cpp, client/src/protocol/inventory_wire.hpp |
| 201 | SM_BAGITEMS | 0x42852f | 有引用：client/src/godot/hud_view.cpp, client/src/protocol/inventory_wire.hpp |
| 202 | SM_DELITEM | 0x4287ca | 有引用：client/src/godot/hud_view.cpp, client/src/protocol/inventory_wire.hpp |
| 203 | SM_DELITEMS / SM_UPDATEITEM | 0x4283c8 | 有引用：client/src/godot/hud_view.cpp, client/src/protocol/inventory_wire.hpp |
| 210 | SM_ADDMAGIC | 0x429642 | 有引用：client/src/godot/hud_view.cpp, client/src/protocol/skill_wire.hpp |
| 211 | SM_SENDMYMAGIC | 0x42962e | 有引用：client/src/godot/hud_view.cpp, client/src/protocol/skill_wire.hpp |
| 212 | SM_DELMAGIC | 0x429653 | 有引用：client/src/godot/hud_view.cpp |
| 600 | SM_DROPITEM_SCCESS / SM_DROPITEM_SUCCESS | 0x429663 | 未见对应命名引用；需核对旧数字分支/缺失处理 |
| 601 | SM_DROPITEM_FAIL | 0x4295fa | 未见对应命名引用；需核对旧数字分支/缺失处理 |
| 610 | SM_ITEMSHOW | 0x4297f2 | 有引用：client/src/godot/hud_view.cpp |
| 611 | SM_ITEMHIDE | 0x4298fd | 有引用：client/src/godot/hud_view.cpp |
| 612 | SM_DOOROPEN / SM_OPENDOOR_OK | 0x42995e | 未见对应命名引用；需核对旧数字分支/缺失处理 |
| 613 | SM_OPENDOOR_LOCK | 0x42998d | 未见对应命名引用；需核对旧数字分支/缺失处理 |
| 614 | SM_CLOSEDOOR | 0x4295e8 | 未见对应命名引用；需核对旧数字分支/缺失处理 |
| 615 | SM_TAKEON_OK | 0x429e37 | 有引用：client/src/godot/hud_view.cpp, client/src/status/equipment.hpp |
| 616 | SM_TAKEON_FAIL | 0x429ea1 | 有引用：client/src/godot/hud_view.cpp, client/src/status/equipment.hpp |
| 619 | SM_TAKEOFF_OK | 0x429f10 | 有引用：client/src/godot/hud_view.cpp, client/src/status/equipment.hpp |
| 620 | SM_TAKEOFF_FAIL | 0x429f49 | 有引用：client/src/godot/hud_view.cpp, client/src/status/equipment.hpp |
| 621 | SM_SENDUSEITEMS | 0x429a18 | 有引用：client/src/godot/hud_view.cpp, client/src/protocol/equipment_wire.hpp |
| 622 | SM_WEIGHTCHANGED | 0x429f7f | 有引用：client/src/protocol/actor_increments.hpp |
| 633 | SM_CLEAROBJECT / SM_CLEAROBJECTS | 0x429a00（共享分支） | 有引用：client/src/godot/hud_view.cpp |
| 634 | SM_CHANGEMAP | 0x429a00（共享分支） | 有引用：client/src/godot/hud_view.cpp, client/src/protocol/map_message.hpp |
| 635 | SM_EAT_OK | 0x429b42 | 有引用：client/src/godot/hud_view.cpp, client/src/inventory/inventory_grid.hpp, client/src/inventory/quick_items.hpp |
| 636 | SM_EAT_FAIL | 0x4283b6 | 有引用：client/src/godot/hud_view.cpp, client/src/inventory/inventory_grid.hpp, client/src/inventory/quick_items.hpp |
| 637 | SM_BUTCH | 0x42a0cc（共享分支） | 未见对应命名引用；需核对旧数字分支/缺失处理 |
| 638 | SM_MAGICFIRE | 0x42a0cc（共享分支） | 未见对应命名引用；需核对旧数字分支/缺失处理 |
| 640 | SM_MAGIC_LVEXP | 0x42abfe | 有引用：client/src/godot/hud_view.cpp |
| 641 | 待定名 | 0x42ac0c | 未见对应命名引用；需核对旧数字分支/缺失处理 |
| 642 | SM_DURACHANGE | 0x42ad10 | 有引用：client/src/protocol/equipment_wire.hpp |
| 643 | SM_MERCHANTSAY | 0x42ac4e | 有引用：client/src/godot/hud_view.cpp |
| 644 | SM_MERCHANTDLGCLOSE | 0x42ace6 | 有引用：client/src/godot/hud_view.cpp |
| 645 | SM_SENDGOODSLIST | 0x42a87f | 有引用：client/src/godot/hud_view.cpp |
| 646 | SM_SENDUSERSELL | 0x42a97f | 未见对应命名引用；需核对旧数字分支/缺失处理 |
| 647 | SM_SENDBUYPRICE | 0x42a9bf | 未见对应命名引用；需核对旧数字分支/缺失处理 |
| 648 | SM_USERSELLITEM_OK | 0x42aa02 | 未见对应命名引用；需核对旧数字分支/缺失处理 |
| 649 | SM_USERSELLITEM_FAIL | 0x42aa86 | 未见对应命名引用；需核对旧数字分支/缺失处理 |
| 650 | SM_BUYITEM_SUCCESS | 0x42a8e5 | 有引用：client/src/godot/hud_view.cpp |
| 651 | SM_BUYITEM_FAIL | 0x42a8f5 | 有引用：client/src/godot/hud_view.cpp |
| 652 | SM_SENDDETAILGOODSLIST | 0x42a8d5 | 有引用：client/src/godot/hud_view.cpp |
| 653 | SM_GOLDCHANGE / SM_GOLDCHANGED | 0x42ae03 | 有引用：client/src/protocol/gold_change.hpp |
| 654 | SM_CHANGELIGHT | 0x42b155 | 未见对应命名引用；需核对旧数字分支/缺失处理 |
| 656 | SM_CHANGENAMECOLOR | 0x42b1c9 | 有引用：client/src/godot/hud_view.cpp, client/src/hud/scene_name.hpp |
| 657 | SM_CHARSTATUSCHANGE / SM_CHARSTATUSCHANGED | 0x42b0be | 未见对应命名引用；需核对旧数字分支/缺失处理 |
| 658 | SM_SENDNOTICE | 0x42a0d9 | 未见对应命名引用；需核对旧数字分支/缺失处理 |
| 660 | SM_CREATEGROUP_OK | 0x42ae60 | 有引用：client/src/protocol/group_message.hpp |
| 661 | SM_CREATEGROUP_FAIL | 0x42ae95 | 有引用：client/src/protocol/group_message.hpp |
| 664 | SM_ADDGROUPMEMBER_FAIL | 0x42afe9 | 有引用：client/src/protocol/group_message.hpp |
| 665 | SM_DELGROUPMEMBER_FAIL | 0x42b06b | 有引用：client/src/protocol/group_message.hpp |
| 666 | SM_GROUPCANCEL | 0x42b0a2 | 有引用：client/src/protocol/group_message.hpp |
| 667 | SM_GROUPMEMBERS | 0x42af41 | 有引用：client/src/protocol/group_message.hpp |
| 668 | 待定名 | 0x42aa9c | 未见对应命名引用；需核对旧数字分支/缺失处理 |
| 669 | 待定名 | 0x42ab37 | 未见对应命名引用；需核对旧数字分支/缺失处理 |
| 670 | 待定名 | 0x42ab9a | 未见对应命名引用；需核对旧数字分支/缺失处理 |
| 671 | 待定名 | 0x42aaec | 未见对应命名引用；需核对旧数字分支/缺失处理 |
| 673 | SM_TRADE_OPEN | 0x42a13f | 有引用：client/src/godot/hud_view.cpp |
| 674 | SM_TRADE_OPEN_FAIL | 0x42a120 | 有引用：client/src/godot/hud_view.cpp |
| 675 | SM_TRADE_ADD_OK | 0x42a3df | 有引用：client/src/godot/hud_view.cpp |
| 676 | SM_TRADE_ADD_FAIL | 0x42a4e9 | 有引用：client/src/godot/hud_view.cpp |
| 677 | SM_TRADE_REMOVE_OK | 0x42a544 | 有引用：client/src/godot/hud_view.cpp |
| 678 | SM_TRADE_REMOVE_FAIL | 0x42a634 | 有引用：client/src/godot/hud_view.cpp |
| 681 | SM_TRADE_CANCELLED | 0x42a1e4 | 有引用：client/src/godot/hud_view.cpp |
| 682 | SM_TRADE_PEER_ADD | 0x42a6d9 | 有引用：client/src/godot/hud_view.cpp |
| 683 | SM_TRADE_PEER_REMOVE | 0x42a7ad | 有引用：client/src/godot/hud_view.cpp |
| 684 | SM_TRADE_GOLD | 0x42a69b | 有引用：client/src/godot/hud_view.cpp |
| 685 | 待定名 | 0x4283a4 | 未见对应命名引用；需核对旧数字分支/缺失处理 |
| 686 | SM_TRADE_PEER_GOLD | 0x42b2b2 | 有引用：client/src/godot/hud_view.cpp |
| 687 | SM_TRADE_COMPLETE | 0x42b2ce | 有引用：client/src/godot/hud_view.cpp |
| 700 | 待定名 | 0x42b533 | 未见对应命名引用；需核对旧数字分支/缺失处理 |
| 701 | 待定名 | 0x42b577 | 未见对应命名引用；需核对旧数字分支/缺失处理 |
| 702 | 待定名 | 0x42b68f | 未见对应命名引用；需核对旧数字分支/缺失处理 |
| 703 | 待定名 | 0x42b6a7 | 未见对应命名引用；需核对旧数字分支/缺失处理 |
| 704 | 待定名 | 0x42b447 | 未见对应命名引用；需核对旧数字分支/缺失处理 |
| 705 | 待定名 | 0x42b48a | 未见对应命名引用；需核对旧数字分支/缺失处理 |
| 706 | 待定名 | 0x42b4ee | 未见对应命名引用；需核对旧数字分支/缺失处理 |
| 707 | 待定名 | 0x42b514 | 未见对应命名引用；需核对旧数字分支/缺失处理 |
| 708 | SM_AREASTATE | 0x42c149 | 未见对应命名引用；需核对旧数字分支/缺失处理 |
| 709 | SM_709 | 0x42b993 | 未见对应命名引用；需核对旧数字分支/缺失处理 |
| 710 | 待定名 | 0x42b6bd | 未见对应命名引用；需核对旧数字分支/缺失处理 |
| 711 | 待定名 | 0x42b73d | 未见对应命名引用；需核对旧数字分支/缺失处理 |
| 712 | 待定名 | 0x42b315 | 未见对应命名引用；需核对旧数字分支/缺失处理 |
| 713 | 待定名 | 0x42b365 | 未见对应命名引用；需核对旧数字分支/缺失处理 |
| 714 | 待定名 | 0x42b394 | 未见对应命名引用；需核对旧数字分支/缺失处理 |
| 750 | SM_CHANGEGUILDNAME | 0x42b85e | 未见对应命名引用；需核对旧数字分支/缺失处理 |
| 751 | 待定名 | 0x42c191 | 未见对应命名引用；需核对旧数字分支/缺失处理 |
| 752 | SM_SUBABILITY | 0x42b29f（共享分支） | 有引用：client/src/protocol/actor_increments.hpp |
| 753 | SM_OPENGUILDDLG | 0x42c1d0 | 有引用：client/src/godot/hud_view.cpp, client/src/guild/guild_overview.hpp |
| 754 | SM_OPENGUILDDLG_FAIL | 0x42c250 | 有引用：client/src/protocol/guild_notice.hpp |
| 756 | SM_SENDGUILDMEMBERLIST | 0x42c270 | 有引用：client/src/godot/hud_view.cpp, client/src/guild/guild_members.hpp |
| 757 | SM_GUILDADDMEMBER_OK | 0x42c293 | 有引用：client/src/protocol/guild_notice.hpp |
| 758 | SM_GUILDADDMEMBER_FAIL | 0x42c2ba | 有引用：client/src/protocol/guild_notice.hpp |
| 759 | SM_GUILDDELMEMBER_OK | 0x42c33e | 有引用：client/src/protocol/guild_notice.hpp |
| 760 | SM_GUILDDELMEMBER_FAIL | 0x42c352 | 有引用：client/src/protocol/guild_notice.hpp |
| 761 | SM_GUILDRANKUPDATE_FAIL | 0x42c3ad | 有引用：client/src/protocol/guild_notice.hpp |
| 762 | SM_BUILDGUILD_OK | 0x42c4f5 | 有引用：client/src/protocol/guild_notice.hpp |
| 763 | SM_BUILDGUILD_FAIL | 0x42c50b | 有引用：client/src/protocol/guild_notice.hpp |
| 768 | SM_GUILDMAKEALLY_OK | 0x42c450 | 有引用：client/src/protocol/guild_notice.hpp |
| 769 | SM_GUILDMAKEALLY_FAIL | 0x42c450 | 有引用：client/src/protocol/guild_notice.hpp |
| 770 | SM_GUILDBREAKALLY_OK | 0x42c4b7 | 有引用：client/src/protocol/guild_notice.hpp |
| 771 | SM_GUILDBREAKALLY_FAIL | 0x42c4b7 | 有引用：client/src/protocol/guild_notice.hpp |
| 773 | SM_GUILD_OVERVIEW | 0x42c229 | 有引用：client/src/godot/hud_view.cpp, client/src/guild/guild_overview.hpp |
| 774 | SM_GUILD_OVERVIEW_UPDATE | 0x42c229 | 有引用：client/src/godot/hud_view.cpp, client/src/guild/guild_overview.hpp |
| 775 | SM_GUILD_MEMBERS_UPDATE | 0x42c270 | 有引用：client/src/godot/hud_view.cpp, client/src/guild/guild_members.hpp |
| 776 | SM_GUILD_MEMBERS_REFRESH | 0x42c270 | 有引用：client/src/godot/hud_view.cpp, client/src/guild/guild_members.hpp |
| 800 | 待定名 | 0x42b29f（共享分支） | 未见对应命名引用；需核对旧数字分支/缺失处理 |
| 801 | SM_801 | 0x42b29f（共享分支） | 未见对应命名引用；需核对旧数字分支/缺失处理 |
| 802 | 待定名 | 0x42b7ab | 未见对应命名引用；需核对旧数字分支/缺失处理 |
| 803 | SM_803 | 0x42c154 | 未见对应命名引用；需核对旧数字分支/缺失处理 |
| 804 | SM_SHOWEVENT | 0x42ba2d | 未见对应命名引用；需核对旧数字分支/缺失处理 |
| 805 | SM_HIDEEVENT | 0x42bec8 | 未见对应命名引用；需核对旧数字分支/缺失处理 |
| 820 | 待定名 | 0x42c5cc | 未见对应命名引用；需核对旧数字分支/缺失处理 |
| 821 | 待定名 | 0x42c5cc | 未见对应命名引用；需核对旧数字分支/缺失处理 |
| 1100 | SM_OPENHEALTH | 0x42c5c1 | 未见对应命名引用；需核对旧数字分支/缺失处理 |
| 1101 | SM_CLOSEHEALTH | 0x42c697 | 未见对应命名引用；需核对旧数字分支/缺失处理 |
| 1105 | 待定名 | 0x42c5af | 未见对应命名引用；需核对旧数字分支/缺失处理 |
| 1200 | 待定名 | 0x42c774 | 未见对应命名引用；需核对旧数字分支/缺失处理 |
| 1300 | SM_RIDEHORSE | 0x42c869 | 有引用：client/src/godot/hud_view.cpp, client/src/hud/mount_panel.hpp |
| 1301 | 待定名 | 0x42c7be | 未见对应命名引用；需核对旧数字分支/缺失处理 |
| 1302 | 待定名 | 0x42c7f9 | 未见对应命名引用；需核对旧数字分支/缺失处理 |
| 1303 | 待定名 | 0x42c59d | 未见对应命名引用；需核对旧数字分支/缺失处理 |
| 1304 | 待定名 | 0x42c9ab | 未见对应命名引用；需核对旧数字分支/缺失处理 |
| 1305 | 待定名 | 0x42cb5f | 未见对应命名引用；需核对旧数字分支/缺失处理 |
| 1306 | SM_HUMAN_CAST | 0x42c951（共享分支） | 有引用：client/src/godot/hud_view.cpp, client/src/hud/scene_human_queue.hpp |
| 1307 | SM_HUMAN_CAST_ALT | 0x42c951（共享分支） | 有引用：client/src/godot/hud_view.cpp, client/src/hud/scene_human_queue.hpp |
| 1308 | SM_HUMAN_ACTION18 | 0x42c951（共享分支） | 有引用：client/src/godot/hud_view.cpp, client/src/hud/scene_human_queue.hpp |
| 1309 | SM_HUMAN_STRUCK | 0x42c951（共享分支） | 有引用：client/src/godot/hud_view.cpp, client/src/hud/scene_human_queue.hpp |
| 1310 | 待定名 | 0x42ca17 | 未见对应命名引用；需核对旧数字分支/缺失处理 |
| 1311 | 待定名 | 0x42c969 | 未见对应命名引用；需核对旧数字分支/缺失处理 |
| 1312 | SM_HEALTH_PERCENT | 0x42cb7c | 有引用：client/src/godot/hud_view.cpp, client/src/hud/scene_health.hpp |
| 1313 | SM_INVENTITEM | 0x42cb55 | 未见对应命名引用；需核对旧数字分支/缺失处理 |
| 1314 | 待定名 | 0x42ca56 | 未见对应命名引用；需核对旧数字分支/缺失处理 |
| 1315 | 待定名 | 0x42ab88 | 未见对应命名引用；需核对旧数字分支/缺失处理 |
| 1318 | SM_1318 | 0x42cbff | 未见对应命名引用；需核对旧数字分支/缺失处理 |
| 1319 | SM_HUMAN_ATTACK | 0x42cbee（共享分支） | 有引用：client/src/godot/hud_view.cpp, client/src/hud/scene_human_queue.hpp |
| 1320 | 待定名 | 0x42cbee（共享分支） | 未见对应命名引用；需核对旧数字分支/缺失处理 |
| 1321 | SM_REVIVE_REFRESH | 0x42cbee（共享分支） | 有引用：client/src/godot/hud_view.cpp, client/src/hud/scene_human_queue.hpp |
| 1322 | 待定名 | 0x42d8cd | 未见对应命名引用；需核对旧数字分支/缺失处理 |
| 1324 | 待定名 | 0x42d984 | 未见对应命名引用；需核对旧数字分支/缺失处理 |
| 1325 | 待定名 | 0x42dfde | 未见对应命名引用；需核对旧数字分支/缺失处理 |
| 1330 | SM_DECODEKEY | 0x42dec8 | 有引用：client/src/godot/hud_view.cpp |
| 1350 | 待定名 | 0x42cc21 | 未见对应命名引用；需核对旧数字分支/缺失处理 |
| 1351 | 待定名 | 0x42cc9c | 未见对应命名引用；需核对旧数字分支/缺失处理 |
| 1352 | 待定名 | 0x42ccb2 | 未见对应命名引用；需核对旧数字分支/缺失处理 |
| 1353 | 待定名 | 0x42d086 | 未见对应命名引用；需核对旧数字分支/缺失处理 |
| 1355 | 待定名 | 0x42d0d3 | 未见对应命名引用；需核对旧数字分支/缺失处理 |
| 1356 | 待定名 | 0x42d105 | 未见对应命名引用；需核对旧数字分支/缺失处理 |
| 1357 | 待定名 | 0x42d156 | 未见对应命名引用；需核对旧数字分支/缺失处理 |
| 1358 | 待定名 | 0x42d1b9 | 未见对应命名引用；需核对旧数字分支/缺失处理 |
| 1400 | 待定名 | 0x42d1cf | 未见对应命名引用；需核对旧数字分支/缺失处理 |
| 1401 | SM_1401 / SM_CLIENT_OPTIONS | 0x42d1e6 | 有引用：client/src/godot/hud_view.cpp |
| 1402 | 待定名 | 0x42d1f2 | 未见对应命名引用；需核对旧数字分支/缺失处理 |
| 1403 | 待定名 | 0x42d30e | 未见对应命名引用；需核对旧数字分支/缺失处理 |
| 1410 | 待定名 | 0x42cbee（共享分支） | 未见对应命名引用；需核对旧数字分支/缺失处理 |

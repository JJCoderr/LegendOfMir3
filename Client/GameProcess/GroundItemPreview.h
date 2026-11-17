#pragma once
namespace GroundItemPreview {
inline void Packet(CGameProcess& game,WORD opcode,int id,WORD x,WORD y,WORD looks,const std::string& name) {
    _TDEFAULTMESSAGE header={};fnMakeDefMessage(&header,opcode,id,x,y,looks);
    char encoded[2048]={};fnEncodeMessage(&header,encoded,sizeof(encoded));
    fnEncode6BitBuf((BYTE*)name.data(),encoded+DEFBLOCKSIZE,int(name.size()),sizeof(encoded)-DEFBLOCKSIZE);
    game.OnSocketMessageRecieve(encoded);
}
inline void Checks(CGameProcess& game) {
    FILE* report=fopen("ground-items-check.txt","wb");
    if(!report) return;
    const auto check=[&](const char* name,bool good){fprintf(report,"%s=%d\n",name,good);};
    using namespace GroundItemDisplay;
    Sparkle clock;clock.Begin(100,10,5);check("random_delay",clock.delay==3005);
    clock.Advance(3105,100);check("delay_boundary",clock.frame==-1);
    clock.Advance(3106,100);check("frame_zero",clock.frame==0);
    clock.Advance(3107,1);check("frame_interval",clock.frame==1 && clock.elapsed==0);
    clock.Advance(3108,1000);check("single_step",clock.frame==2);
    for(int i=0;i<8;++i) clock.Advance(3200+i,101);
    check("cycle",clock.frame==-1 && clock.started==3207 && clock.delay==3005);
    clock.Begin(0xfffffff0,0,0);clock.Advance(0x1000,0);check("tick_wrap",clock.frame==0);
    check("range",Nearby(-24,-24) && Nearby(23,23) && !Nearby(24,0) && !Nearby(0,-25));
    check("coin",CoinVariant("Coin9","Coin") && !CoinVariant("Coin10","Coin") && !CoinVariant("Coinx","Coin"));
    game.m_stMapItemList.ClearAllNodes();
    Packet(game,SM_ITEMSHOW,10,334,272,20,"3/测试物品");
    check("show",game.m_stMapItemList.GetCounter()==1);
    game.m_stMapItemList.MoveCurrentToTop();auto* first=game.m_stMapItemList.GetCurrentData();
    check("name_mode",first->stackMode==3 && !strcmp(first->szItemName,"测试物品"));
    Packet(game,SM_ITEMSHOW,10,335,273,21,"Replacement");
    check("duplicate",game.m_stMapItemList.GetCounter()==1 && first->shTileX==334);
    Packet(game,SM_ITEMSHOW,0,334,272,20,"Bad");
    Packet(game,SM_ITEMSHOW,11,65535,272,20,"Bad");
    Packet(game,SM_ITEMSHOW,12,334,272,20,std::string(40,'x'));
    check("invalid",game.m_stMapItemList.GetCounter()==1);
    Packet(game,SM_ITEMSHOW,13,334,272,20,std::string(39,'x'));
    check("name_boundary",game.m_stMapItemList.GetCounter()==2);
    Packet(game,SM_ITEMHIDE,13,0,0,0,"extra");check("hide_payload",game.m_stMapItemList.GetCounter()==2);
    Packet(game,SM_ITEMHIDE,13,0,0,0,"");check("hide",game.m_stMapItemList.GetCounter()==1);
    for(int i=0;i<100;++i) Packet(game,SM_ITEMSHOW,100+i,334,272,20,"Item");
    game.CheckMappedData(0,FALSE);
    const auto& cell=game.m_stMemMappedID[272-game.m_xMap.m_shStartViewTileY][334-game.m_xMap.m_shStartViewTileX];
    check("cell_capacity",cell.pstItem[99] && cell.pstItem[99]->nRecog==198);
    game.m_stMapItemList.ClearAllNodes();
    Packet(game,SM_ITEMSHOW,1,game.m_xMyHero.m_wPosX+24,game.m_xMyHero.m_wPosY,20,"Outside");
    Packet(game,SM_ITEMSHOW,2,game.m_xMyHero.m_wPosX-24,game.m_xMyHero.m_wPosY,20,"Inside");
    game.RenderScene(0);check("range_prune",game.m_stMapItemList.GetCounter()==1);
    char map[]="01";game.LoadMapChanged(map);check("map_clear",game.m_stMapItemList.GetCounter()==0);
    _TDEFAULTMESSAGE invalidHeader={};fnMakeDefMessage(&invalidHeader,SM_ITEMSHOW,99,334,272,20);
    char invalid[64]={};fnEncodeMessage(&invalidHeader,invalid,sizeof(invalid));invalid[DEFBLOCKSIZE]='!';
    game.OnSocketMessageRecieve(invalid);check("encoding_rejected",game.m_stMapItemList.GetCounter()==0);
    game.m_ptMousePos={600,300};check("world_pointer",game.m_xInterface.SpellPointerAvailable());
    game.m_xInterface.m_minimap.visible=true;check("minimap_blocks",!game.m_xInterface.SpellPointerAvailable());
    game.m_xInterface.m_minimap.visible=false;
    game.m_ptMousePos={200,460};check("chat_blocks",!game.m_xInterface.SpellPointerAvailable());
    game.m_xMap.m_shViewOffsetX=24;game.m_xMap.m_shViewOffsetY=16;
    game.OnMouseMove(0,MAKELPARAM(560,300));
    check("scroll_pointer",game.m_ptCurrTile.x==game.m_xMap.m_shStartViewTileX+(560-_VIEW_CELL_X_START+24)/48 &&
        game.m_ptCurrTile.y==game.m_xMap.m_shStartViewTileY+(300-_VIEW_CELL_Y_START+16)/32);
    game.m_xMap.m_shViewOffsetX=game.m_xMap.m_shViewOffsetY=0;
    fclose(report);
}
inline bool Scene(CGameProcess& game,bool coin) {
    if(!SpellPreview::MapScene(game)) return false;
    game.ClearNativeSpells();game.m_xMyHero.SetMotionFrame(_MT_STAND,2);
    game.m_xMyHero.m_bUseEffect=FALSE;game.m_xMyHero.m_spellLightActive=false;
    Checks(game);
    CLoginMessages messages;messages.Load("CmList.dat");
    const auto coinName=messages.Get(GroundItemDisplay::CoinNameMessage);
    Packet(game,SM_ITEMSHOW,1,334,272,20,coin?coinName+"5":"生命药水");
    Packet(game,SM_ITEMSHOW,2,334,272,30,"魔法药水");
    Packet(game,SM_ITEMSHOW,3,334,272,40,"地面装备");
    game.m_stMapItemList.MoveCurrentToTop();
    game.m_stMapItemList.GetCurrentData()->sparkle.frame=5;
    game.m_ptMousePos={ (334-game.m_xMap.m_shStartViewTileX)*48+_VIEW_CELL_X_START+24,
        (272-game.m_xMap.m_shStartViewTileY)*32+_VIEW_CELL_Y_START+16 };
    return true;
}
}

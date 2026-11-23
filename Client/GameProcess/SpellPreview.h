#pragma once
#include "SpellReply.h"
#include "SpellSpriteRenderer.h"

namespace SpellPreview {
inline void Checks(CGameProcess& game) {
    FILE* report=fopen("spell-check.txt","wb");if(!report) return;
    const auto check=[&](const char* name,bool value){fprintf(report,"%s=%d\n",name,value);};
    using namespace mir3;
    SpellTiming timing;timing.begin_animated(10000,800);
    check("key_boundary",!timing.key_ready(12000) && timing.key_ready(12001));
    check("repeat_slots",!timing.key_ready(12001,1,true) && timing.key_ready(12001,10,true) && timing.key_ready(12001,12,true));
    timing.finish_target(1,0,1099,0);
    check("human_delay",timing.movement_delay==1400 && !timing.movement_ready(11400) && timing.movement_ready(11401));
    timing.finish_target(1,2,1099,0);check("nonhuman_delay",timing.movement_delay==0);
    check("tick_wrap",SpellTiming::elapsed_after(16,0xfffffff0u,31) && !SpellTiming::elapsed_after(16,0xfffffff0u,32));
    LocalActionClock clock;clock.attacked=1000;clock.moved=1200;
    check("shared_wait",clock.shared_wait(1400,1001)==401 && clock.shared_wait(1801,1001)==0);
    check("move_admission",!clock.admit_move(599) && clock.admit_move(600) && !clock.admit_move(1199) && clock.admit_move(1200));
    const auto rays=GroundSpell::segments(MagicEffectId::three_rays,{5,5},{12,5},2);
    check("rays",rays.size()==15 && rays[0].cell==rays[5].cell && rays[5].cell==rays[10].cell && rays[4].delay==600);
    const auto wave=GroundSpell::segments(MagicEffectId::fire_wave,{5,5},{5,5},0);
    check("wave",wave.size()==9 && wave[0].cell==GroundSpell::Cell{5,6} && wave.back().delay==1200);
    auto bump=ForcedMotion::begin(ForcedMotion::Bump,2,11,10);
    check("bump_origin",bump.origin==GroundSpell::Cell{10,10});
    check("forced_clock",!bump.advance(95) && bump.advance(1) && bump.frame==2);
    bump.advance(96);check("bump_return",bump.returned() && bump.cell()==bump.origin && bump.offset()==GroundSpell::Cell{});
    auto back=ForcedMotion::begin(ForcedMotion::Backstep,2,9,10);
    check("backstep",back.origin==GroundSpell::Cell{10,10} && back.direction()==6);
    check("direction",SpellDirection(0,-32)==0 && SpellDirection(48,0)==4 && SpellDirection(0,32)==8 && SpellDirection(-48,0)==12);
    SpellSoundVoices voices;
    const auto voice=voices.select(10011,2000,[](std::int64_t){return false;});
    const auto full=voices.select(10011,2001,[](std::int64_t){return true;});
    check("voice_no_steal",voice && voice->replaced && voice->play && full && !full->play);
    const auto mix=SoundPosition::from_view(376,217);check("sound_center",mix.volume==0 && mix.pan==0);
    SpellParticlePool particles;EffectRandom random;
    particles.emit_lightning({100,100},random);check("lightning_pool",particles.count()==22);
    particles.emit_ice({100,100},random);check("ice_pool",particles.ice_count()==3 && particles.count()==25);
    particles.advance(2000,random);check("particle_cleanup",particles.count()==0);
    auto& hero=game.m_xMyHero;hero.m_dwIdentity=123;hero.m_wPosX=5;hero.m_wPosY=5;
    PACKETMSG packet={};const DWORD target=0xfedcba98;char encoded[32]={};
    fnEncode6BitBuf((BYTE*)&target,encoded,sizeof(target),sizeof(encoded));
    fnMakeDefMessage(&packet.stDefMsg,SpellWire::SingleReply,123,12,5,0xff01);
    strcpy(packet.szEncodeData,encoded);const auto reply=SpellWire::Reply::Parse(packet.stDefMsg,packet.szEncodeData);
    check("single_wire",reply && reply->skill==1 && reply->targets.front()==target);
    check("malformed_wire",!SpellWire::Reply::Parse(packet.stDefMsg,"bad"));
    game.ClearNativeSpells();hero.m_shCurrMagicID=1;packet.stDefMsg.wSeries=104;hero.OnMagicFire(&packet);
    check("ignored_reply",game.m_nativeSpells.empty() && hero.m_shCurrMagicID==1);
    packet.stDefMsg.wSeries=22;hero.OnMagicFire(&packet);
    check("state_reply",game.m_nativeSpells.empty() && hero.m_shCurrMagicID==22);
    packet.stDefMsg.wSeries=1;hero.OnMagicFire(&packet);
    check("fireball_created",game.m_nativeSpells.size()==1 && game.m_nativeSpells[0].playback.flying);
    DWORD flightPlaying=0;LONG flightVolume=0,flightPan=0;
    check("fireball_flight_audio",!game.m_nativeSpells.empty() &&
        SUCCEEDED(g_xSound.InspectSpellAudio(game.m_nativeSpells.front().soundVoice,flightPlaying,flightVolume,flightPan)) && flightPlaying==1);
    const auto oldCount=game.m_nativeSpells.size();game.AdvanceNativeSpells(100,game.m_nativeSpells.empty()?game.m_nextNativeSpell:game.m_nativeSpells.front().serial);
    check("new_frame_boundary",oldCount==1 && game.m_nativeSpells.size()==oldCount && game.m_nativeSpells[0].playback.age==0);
    game.ClearNativeSpells();packet.stDefMsg.wSeries=150;hero.OnMagicFire(&packet);
    check("orbit_created",game.m_nativeSpells.size()==3 && game.m_nativeSpells[1].orbit && game.m_nativeSpells[2].orbit && game.m_nativeSpells[2].playback.delay==30);
    game.ClearNativeSpells();packet.stDefMsg.wSeries=110;hero.m_bCurrDir=2;hero.OnMagicFire(&packet);
    check("rays_dispatch",game.m_nativeSpells.size()==15);
    game.ClearNativeSpells();packet.stDefMsg.wSeries=10;hero.OnMagicFire(&packet);
    check("lightning_dispatch",game.m_nativeSpells.size()==10);
    game.AdvanceNativeSpells(100,game.m_nextNativeSpell);check("lightning_emission",game.m_spellParticles.count()>0);
    game.ClearNativeSpells();check("scene_cleanup",game.m_nativeSpells.empty() && game.m_spellParticles.count()==0 && game.m_spellMarks.empty());
    const char list[]="123//123/-1/";
    fnMakeDefMessage(&packet.stDefMsg,SpellWire::MultipleReply,123,0,0,113);
    fnEncode6BitBuf((BYTE*)list,packet.szEncodeData,sizeof(list)-1,sizeof(packet.szEncodeData));
    const auto multiple=SpellWire::Reply::Parse(packet.stDefMsg,packet.szEncodeData);
    check("list_wire",multiple && multiple->targets==std::vector<DWORD>{123,0,123,0xffffffff,0});
    hero.OnMagicList(&packet);check("list_duplicates",game.m_nativeSpells.size()==2);
    game.ClearNativeSpells();
    check("particle_assets",NativeSpellSprites::Frame(520) && NativeSpellSprites::Frame(530));
    const auto capture=[&](auto action,int skill,POINT position,DWORD identity,WORD opcode=CM_SPELL) {
        SOCKET receiver=socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP),sender=socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP);
        sockaddr_in address={};address.sin_family=AF_INET;address.sin_addr.s_addr=htonl(INADDR_LOOPBACK);
        bind(receiver,(sockaddr*)&address,sizeof(address));int size=sizeof(address);getsockname(receiver,(sockaddr*)&address,&size);
        connect(sender,(sockaddr*)&address,sizeof(address));DWORD timeout=80;
        setsockopt(receiver,SOL_SOCKET,SO_RCVTIMEO,(char*)&timeout,sizeof(timeout));
        const SOCKET saved=g_xClientSocket.m_sockClient;g_xClientSocket.m_sockClient=sender;
        const DWORD before=GetTickCount();action();const DWORD after=GetTickCount();g_xClientSocket.m_sockClient=saved;
        char wire[256]={};const int length=recv(receiver,wire,sizeof(wire)-1,0);closesocket(sender);closesocket(receiver);
        if(!skill) return length==SOCKET_ERROR;
        if(length<19 || wire[0]!='#' || wire[1]<'1' || wire[1]>'9' || wire[length-1]!='!') return false;
        _TDEFAULTMESSAGE header={};fnDecodeMessage(&header,wire+2);wire[length-1]=0;
        if(opcode!=CM_SPELL) return length==19 && header.wIdent==opcode &&
            DWORD(header.nRecog)==DWORD(MAKELONG(position.x,position.y)) && header.wTag==skill && !header.wParam && !header.wSeries;
        char body[32]={};fnDecode6BitBuf(wire+2+DEFBLOCKSIZE,body,sizeof(body));
        char* end=nullptr;const LONG signedTick=strtol(body,&end,10);const DWORD tick=DWORD(signedTick);
        return header.wIdent==CM_SPELL && DWORD(header.nRecog)==DWORD(MAKELONG(position.x,position.y)) &&
            header.wParam==LOWORD(identity) && header.wTag==skill && header.wSeries==HIWORD(identity) &&
            end!=body && *end==0 && DWORD(tick-before)<=DWORD(after-before);
    };
    check("request_wire",capture([&](){g_xClientSocket.SendSpellMsg(113,321,654,int(0xfedcba98));},113,{321,654},0xfedcba98));
    hero.m_pxMap=&game.m_xMap;hero.m_stFeature.bGender=_GENDER_MAN;hero.m_stFeatureEx.bHorse=_HORSE_NONE;
    hero.m_bIsDead=hero.m_bIsMon=FALSE;hero.m_pendingAction.reset();hero.m_forcedMotion.reset();
    hero.m_wPosX=20;hero.m_wPosY=20;hero.m_bCurrDir=4;hero.m_bInputLock=hero.m_bMotionLock=FALSE;
    CLIENTMAGICRCD record={};record.stStdMagic.nDelayTime=800;FEATURE targetFeature={};targetFeature.bGender=2;
    hero.m_shCurrMagicID=1;
    check("animated_request",capture([&](){hero.SetMagic(&record,1,2,0,targetFeature,{25,20});},1,{25,20},0));
    check("animated_prediction",hero.m_spellAwaitingReply && hero.m_bInputLock && hero.m_bMotionLock && hero.m_spellTiming.casting_delay==1000 && hero.m_wOldPosX==20);
    const DWORD castingFrame=hero.m_dwCurrFrame;char good[]="+GOOD",fail[]="+FAIL";
    game.OnSocketMessageRecieve(good);
    check("good_keeps_animation",!hero.m_spellAwaitingReply && !hero.m_bMotionLock && hero.m_bInputLock && hero.m_dwCurrFrame==castingFrame);
    hero.m_wPosX=23;game.OnSocketMessageRecieve(fail);
    check("fail_saved_origin",hero.m_wPosX==20 && hero.m_wPosY==20 && !hero.m_bInputLock && !hero.m_bUseEffect);
    hero.m_shCurrMagicID=102;hero.m_spellTiming.cast_at=GetTickCount()-1001;hero.m_spellTiming.casting_delay=777;
    check("predicted_request",capture([&](){hero.SetMagic(&record,1,2,456,targetFeature,{25,20});},102,{0,0},456));
    check("predicted_delay_preserved",hero.m_spellAwaitingReply && hero.m_spellTiming.casting_delay==777);
    hero.SetMotionFrame(_MT_ATTACKMODE,2);hero.m_spellReplyElapsed=7999;hero.m_dwMotionLockTime=0;
    hero.UpdateMotionState(1,FALSE);check("timeout_boundary",hero.m_spellAwaitingReply);
    hero.UpdateMotionState(1,FALSE);check("timeout_release",!hero.m_spellAwaitingReply && !hero.m_bMotionLock && !hero.m_bInputLock);
    hero.m_bIsDead=TRUE;hero.m_wPosX=24;game.OnSocketMessageRecieve(fail);
    check("dead_no_rollback",hero.m_wPosX==24);hero.m_bIsDead=FALSE;
    hero.SetMotionFrame(_MT_STAND,2);hero.m_bInputLock=hero.m_bMotionLock=FALSE;
    hero.m_shCurrMagicID=12;hero.m_spellTiming.cast_at=GetTickCount();
    check("rapid_suppressed",capture([&](){hero.SetMagic(&record,1,2,456,targetFeature,{25,20});},0,{},0));
    hero.m_spellTiming.cast_at=GetTickCount()-201;
    check("rapid_request",capture([&](){hero.SetMagic(&record,1,2,456,targetFeature,{25,20});},12,{0,0},456));
    check("rapid_no_prediction",!hero.m_spellAwaitingReply && hero.m_spellTiming.casting_delay==0);
    PACKETMSG forced={};fnMakeDefMessage(&forced.stDefMsg,ForcedMotion::Bump,123,25,20,2);
    hero.OnRush(&forced);hero.AdvanceForcedMotion(95);
    check("native_bump_wait",hero.m_forcedMotion && hero.m_forcedMotion->frame==0 && hero.m_wPosX==24);
    hero.AdvanceForcedMotion(1);check("native_bump_forward",hero.m_wPosX==25 && hero.m_forcedMotion->frame==2);
    hero.AdvanceForcedMotion(96);check("native_bump_return",!hero.m_forcedMotion && hero.m_wPosX==24 && !hero.m_bInputLock && !hero.m_spellAwaitingReply);
    hero.m_spellTiming={};hero.m_pendingAction.reset();hero.m_forcedMotion.reset();hero.ConfirmAction();
    g_xSound.ClearSpellVoices();g_xSound.SetEnablePlay();g_xSound.SetInterfaceVolume(false,0);g_xSound.m_reversePan=false;
    constexpr int FireballFlightSound=10011;
    const auto liveVoice=g_xSound.PlaySpellWav(FireballFlightSound,100,200);
    DWORD playing=0;LONG volume=0,pan=0;
    const auto inspect=[&](){return SUCCEEDED(g_xSound.InspectSpellAudio(liveVoice,playing,volume,pan));};
    check("audio_playing",liveVoice && inspect() && playing==1);
    check("audio_position",inspect() && volume==-900 && pan==3600);
    g_xSound.m_reversePan=true;g_xSound.RefreshSpellVoices();check("audio_reverse",inspect() && pan==-3600);
    g_xSound.SetInterfaceVolume(false,-25);check("audio_volume",inspect() && volume==-1900);
    g_xSound.PositionSpellWav(liveVoice,FireballFlightSound+1,376,217);
    check("audio_sample_guard",inspect() && volume==-1900 && pan==-3600);
    g_xSound.PositionSpellWav(liveVoice,FireballFlightSound,376,217);
    check("audio_follow",inspect() && volume==-1000 && pan==0);
    bool distinct=true;
    for(int i=1;i<SpellSoundVoices::voices;++i)
        distinct= g_xSound.PlaySpellWav(FireballFlightSound,376,217)==SpellSoundVoices::key(0,i) && distinct;
    check("audio_ten_voices",distinct);
    const auto saturated=g_xSound.PlaySpellWav(FireballFlightSound,376,217);
    check("audio_saturated",saturated==SpellSoundVoices::key(0,9) && inspect() && playing==1);
    g_xSound.SetDisablePlay();check("audio_muted",inspect() && !playing && !g_xSound.PlaySpellWav(FireballFlightSound,376,217));
    g_xSound.SetEnablePlay();check("audio_reenabled",g_xSound.PlaySpellWav(FireballFlightSound,376,217)!=0);
    game.ClearNativeSpells();check("audio_scene_cleanup",!inspect());
    g_xSound.m_reversePan=false;g_xSound.SetInterfaceVolume(false,0);
    hero.m_nState=0;hero.BeginSpellAction(1,2,_MT_SPELL1);
    const DWORD first=hero.m_dwFstFrame;
    hero.AdvanceSpellAnimation(75);check("animation_boundary",hero.m_dwCurrFrame==first && hero.m_bEffectFrame==0);
    hero.AdvanceSpellAnimation(1);check("animation_first",hero.m_dwCurrFrame==first+1 && hero.m_bEffectFrame==1);
    const int directedFrames[]={1,1,1,1,2,3,4,5};bool directed=true;
    for(const int frame:directedFrames) {hero.AdvanceSpellAnimation(hero.m_wDelay+1);directed=directed && hero.m_dwCurrFrame==first+frame;}
    check("directed_frame_sequence",directed && hero.m_bEffectFrame==9);
    hero.BeginSpellAction(11,2,_MT_SPELL2);const DWORD areaFirst=hero.m_dwFstFrame;bool area=true;
    // Skill11 has a 13-frame visual limit; phase3 repeats through relative11.
    for(int i=1;i<=13;++i) {
        hero.AdvanceSpellAnimation(hero.m_wDelay+1);
        const int frame=i<4?i:i<=11?3:i==12?4:5;
        area=area && hero.m_dwCurrFrame==areaFirst+frame;
    }
    check("area_frame_sequence",area && hero.m_bEffectFrame==13);
    hero.BeginSpellAction(1,2,_MT_SPELL1);hero.m_nState=0x08000000;
    hero.AdvanceSpellAnimation(150);const bool slowed=hero.m_bEffectFrame==0;
    hero.AdvanceSpellAnimation(1);check("animation_status_slow",slowed && hero.m_bEffectFrame==1);
    hero.m_nState=0;hero.BeginSpellAction(1,2,_MT_SPELL1);hero.AdvanceSpellAnimation(65536+75);
    check("animation_dword_clock",hero.m_bEffectFrame==1);
    CHero remoteClock;remoteClock.m_stFeature.bGender=_GENDER_MAN;remoteClock.BeginSpellAction(1,2,_MT_SPELL1);
    remoteClock.m_bMsgHurryCheck=TRUE;remoteClock.AdvanceSpellAnimation(76);
    check("remote_catchup",remoteClock.m_wDelay==37 && !remoteClock.m_bMsgHurryCheck && remoteClock.m_bEffectFrame==1);
    hero.SetMotionFrame(_MT_STAND,2);hero.m_bUseEffect=FALSE;hero.m_spellLightActive=false;
    game.ClearNativeSpells();
    // Load the real map without requesting its graphical tile libraries in this logic preview.
    game.m_xMap.SetStartViewTile(10000,10000);char mapName[]="01";
    const bool mapLoaded=game.m_xMap.LoadMapData(mapName)!=FALSE;check("actual_map_loaded",mapLoaded);
    if(mapLoaded) {
        hero.m_pxMap=&game.m_xMap;hero.m_wPosX=20;hero.m_wPosY=20;hero.m_spellTiming={};hero.m_actionClock={};
        hero.m_bIsDead=FALSE;hero.m_bInputLock=hero.m_bMotionLock=FALSE;hero.ConfirmAction();
        hero.SetMotionFrame(_MT_STAND,2);POINT destination={21,20};
        hero.m_actionClock.move_admitted=GetTickCount();
        const bool admissionQuiet=capture([&](){hero.SetMotionState(_MT_WALK,2,0,FALSE,&destination);},0,{},0);
        check("pending_admission",admissionQuiet && hero.m_pendingAction && !hero.m_pendingAction->waitingSince);
        hero.m_actionClock.move_admitted=GetTickCount()-601;hero.m_actionClock.moved=GetTickCount();
        const bool waitQuiet=capture([&](){hero.AdvancePendingAction();},0,{},0);
        check("pending_shared_wait",waitQuiet && hero.m_pendingAction && hero.m_pendingAction->waitingSince && hero.m_pendingAction->wait>0);
        ++hero.m_wPosX;hero.AdvancePendingAction();check("pending_origin_cancel",!hero.m_pendingAction);
        hero.m_wPosX=20;hero.m_actionClock={};hero.m_spellTiming.cast_at=GetTickCount();hero.m_spellTiming.movement_delay=10000;
        hero.SetMotionState(_MT_WALK,2,0,FALSE,&destination);
        check("pending_spell_reject",!hero.m_pendingAction && hero.m_actionClock.move_admitted!=0);
        hero.m_spellTiming={};hero.m_actionClock={};hero.m_actionClock.moved=GetTickCount();
        hero.SetMotionState(_MT_WALK,2,0,FALSE,&destination);
        if(hero.m_pendingAction) hero.m_pendingAction->waitingSince=GetTickCount()-hero.m_pendingAction->wait;
        check("pending_walk_packet",capture([&](){hero.AdvancePendingAction();},2,destination,0,CM_WALK));
        check("pending_walk_consumed",!hero.m_pendingAction && hero.m_actionClock.moved!=0 && hero.m_bCurrMtn==_MT_WALK);
        hero.m_pendingAction.reset();hero.m_forcedMotion.reset();hero.SetMotionFrame(_MT_STAND,2);hero.ConfirmAction();
        game.m_xMap.FreeMapData();
    }
    auto* queued=new PACKETMSG{};DWORD zeroTarget=0;
    fnMakeDefMessage(&queued->stDefMsg,SpellWire::SingleReply,hero.m_dwIdentity,20,20,22);
    fnEncode6BitBuf((BYTE*)&zeroTarget,queued->szEncodeData,sizeof(zeroTarget),sizeof(queued->szEncodeData));
    hero.m_xPacketQueue.PushQ((BYTE*)queued);hero.BeginSpellAction(1,2,_MT_SPELL1);
    check("cast_preserves_fifo",!hero.UpdatePacketState() && hero.m_xPacketQueue.GetCount()==1 && hero.m_shCurrMagicID==1);
    for(int i=0;i<9;++i) hero.AdvanceSpellAnimation(hero.m_wDelay+1);
    hero.m_bMotionLock=FALSE;hero.UpdateMotionState(0,FALSE);
    const bool consumed=hero.UpdatePacketState()!=FALSE;
    check("cast_releases_fifo",consumed && hero.m_xPacketQueue.GetCount()==0 && hero.m_shCurrMagicID==22);
    hero.SetMotionFrame(_MT_STAND,2);hero.ConfirmAction();game.ClearNativeSpells();
    fclose(report);
}
inline void Scene(CGameProcess& game,int mode) {
    game.ClearNativeSpells();auto& hero=game.m_xMyHero;
    hero.m_wPosX=4;hero.m_wPosY=8;hero.m_bCurrDir=2;
    game.m_xMap.m_shStartViewTileX=0;game.m_xMap.m_shStartViewTileY=0;
    game.m_xMap.m_shViewOffsetX=_VIEW_CELL_X_START;game.m_xMap.m_shViewOffsetY=_VIEW_CELL_Y_START;
    if(mode==0) {
        game.StartGroundSpell(mir3::MagicEffectId::fire_wave,{2,6},{11,6},2);
        game.StartGroundSpell(mir3::MagicEffectId::ice_wave,{2,11},{11,11},2);
    } else if(mode==1) game.StartNativeSpell(hero,WORD(mir3::SkillId::original_150),{14,8},0);
    else {
        game.StartNativeSpell(hero,WORD(mir3::SkillId::original_73),{11,8},0);
        game.StartNativeSpell(hero,WORD(mir3::SkillId::original_74),{9,8},0);
    }
    for(int i=0;i<(mode==0?9:mode==1?5:3);++i) game.AdvanceNativeSpells(81,game.m_nextNativeSpell);
}
inline bool MapScene(CGameProcess& game) {
    game.ClearNativeSpells();game.m_xImage.LoadAllImage();
    if(FILE* report=fopen("scene-resource-check.txt","wb")) {
        for(int index=0;index<_MAX_IMAGE;++index) {
            const bool loaded=game.m_xImage.m_xImageList[index].m_stNewWixImgaeInfo.nIndexCount>0;
            fprintf(report,"library_%d=%d\n",index,index==_IMAGE_INTEREX?!loaded:loaded);
        }
        fclose(report);
    }
    game.m_xMap.LoadMapImage(&game.m_xImage);
    constexpr WORD HeroX=330,HeroY=270;
    game.m_xMap.SetStartViewTile(HeroX-_GAPX_TILE_CHAR_MAP,HeroY-_GAPY_TILE_CHAR_MAP);
    char mapName[]="01";
    if(!game.m_xMap.LoadMapData(mapName)) return false;
    auto& hero=game.m_xMyHero;FEATURE feature={};
    feature.bGender=_GENDER_MAN;feature.bDress=1;feature.bHair=1;
    if(!hero.Create(&game.m_xImage,_MT_STAND,2,HeroX,HeroY,&feature,nullptr)) return false;
    hero.SetMapHandler(&game.m_xMap);hero.m_stAbility.wHP=hero.m_stAbility.wMaxHP=400;
    hero.m_stAbility.wMP=hero.m_stAbility.wMaxMP=300;
    hero.m_bIsDead=FALSE;hero.ConfirmAction();
    hero.BeginSpellAction(1,2,_MT_SPELL1);hero.AdvanceSpellAnimation(76);
    game.StartGroundSpell(mir3::MagicEffectId::fire_wave,{HeroX-5,HeroY-3},{HeroX+4,HeroY-3},2);
    game.StartGroundSpell(mir3::MagicEffectId::ice_wave,{HeroX-5,HeroY+4},{HeroX+4,HeroY+4},2);
    for(int i=0;i<9;++i) game.AdvanceNativeSpells(81,game.m_nextNativeSpell);
    game.StartNativeSpell(hero,WORD(mir3::SkillId::original_150),{HeroX+6,HeroY},0);
    for(int i=0;i<5;++i) game.AdvanceNativeSpells(81,game.m_nextNativeSpell);
    game.m_bRender=TRUE;game.m_ptMousePos={790,430};
    lstrcpyA(game.m_szMapName,"01");return true;
}

}

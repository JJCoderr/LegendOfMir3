#pragma once
#include "WeatherRenderer.h"
namespace LightingPreview {
inline void Checks(CGameProcess& game) {
    FILE* report=fopen("scene-lighting-check.txt","wb");if(!report) return;
    const auto check=[&](const char* key,bool good){fprintf(report,"%s=%d\n",key,good);};
    mir3::SceneAmbient ambient;ambient.enter(1);check("night_entry",ambient.level()==15);
    ambient.enter(2);check("dawn_entry",ambient.level()==85);ambient.enter(7);check("unknown_entry",ambient.level()==10);
    ambient.update(0);ambient.advance(49);check("timer_boundary",ambient.half_level==0 && ambient.level()==10);
    ambient.advance(1);check("timer_first",ambient.half_level==1 && ambient.level()==0);
    ambient.advance(25450);check("fade_reaches",ambient.half_level==510 && ambient.level()==255 && ambient.fading);
    ambient.advance(50);check("fade_stops",!ambient.fading);
    ambient.update(1);ambient.advance(24000);check("fade_night",ambient.level()==15 && ambient.fading);
    ambient.enter(1);check("same_entry_stops",!ambient.fading && ambient.half_level==30);
    ambient.select_base(2,41);check("base_selection",ambient.selected_base==85);
    const auto local=mir3::SceneLight::actor(0,0,8,true),remote=mir3::SceneLight::actor(0,0,8,false);
    check("actor_radii",local.outer_radius_cells==10 && remote.outer_radius_cells==8);
    check("actor_dark",mir3::SceneLight::actor(0,0,0,false).outer==mir3::SceneLight::Rgb{10,5,5});
    mir3::SceneLight light{-8,-29,10,{100,20,10},4,{200,200,200}};
    check("inner_pixel_radius",light.sample(0,0)==mir3::SceneLight::Rgb{100,20,10} && light.sample(1,0)==mir3::SceneLight::Rgb{125,125,125});
    CLightFog fog;fog.Init(RGB(15,15,15));mir3::SceneLightGrid grid(15);
    const mir3::SceneLight lights[]={{300,200,8,{255,10,20},7,{230,50,90}},{440,280,0,{},4,{20,220,70}}};
    for(const auto& source:lights) {
        grid.add(source);fog.SetLightRadiusWithRing(source.x,source.y,source.inner_radius,source.inner[0],source.inner[1],source.inner[2],source.outer_radius_cells,source.outer[0],source.outer[1],source.outer[2]);
    }
    bool colors=true,diagonals=true;
    for(int y=0;y<19;++y) for(int x=0;x<18;++x) {
        const auto c=fog.ColorAt(x,y);const auto& expected=grid.samples[y*18+x];
        colors=colors && int((c>>16)&255)==expected[0] && int((c>>8)&255)==expected[1] && int(c&255)==expected[2];
        if(x<17 && y<18) diagonals=diagonals && fog.DiagonalAt(x,y)==grid.diagonals[y*17+x];
    }
    check("native_grid_colors",colors);check("native_grid_triangles",diagonals);
    fog.ClearSavedLightColor();check("frame_reset",fog.ColorAt(8,6)==0x0f0f0f && !fog.DiagonalAt(8,6));
    mir3::SnowWeather weather;check("random_sequence",weather.random()==41);weather.select(1);weather.advance(33,true);
    check("snow_spawn",weather.active==1 && weather.fog && weather.fog_frame==1);
    weather.advance(33,false);check("snow_disabled",weather.active==1 && weather.fog_frame==1);
    weather.select(3);weather.advance(33,false);check("rain_independent",weather.other && weather.rain_active==1);
    weather.select(0,true);check("map_preserves_rain",weather.other);
    weather.select(0);check("message_clears_rain_mode",!weather.other && weather.rain_active==1);
    weather.clear_rain();check("rain_clear",weather.rain_active==0);
    NativeSpellSprites::Sprite sprite;sprite.width=2;sprite.height=2;
    sprite.pixels={0xf800,0x07e0,0x001f,0xffff};sprite.opaque={1,1,1,1};
    WORD pixels[4]={};DDSURFACEDESC2 surface={};surface.dwWidth=surface.dwHeight=2;surface.lPitch=4;surface.lpSurface=pixels;
    std::vector<WORD> background(4,0);
    NativeWeather::Quad(surface,sprite,1,1,1,1,0,{255,255,255},true,&background);
    check("weather_uv",pixels[0]==0xffff && pixels[1]==0x07e0 && pixels[2]==0x001f && pixels[3]==0xf800);
    background.assign(4,0x001f);sprite.width=sprite.height=1;sprite.pixels={0xf800};sprite.opaque={1};
    NativeWeather::Quad(surface,sprite,1,1,1,1,0,{255,255,255},false,&background);
    check("rain_screen_blend",pixels[0]==0xf81f);
    pixels[0]=0xffff;NativeWeather::Quad(surface,sprite,1,1,1,1,0,{255,255,255},false,&background);
    check("rain_shared_snapshot",pixels[0]==0xf81f);
    sprite.opaque={0};pixels[0]=0x07e0;NativeWeather::Quad(surface,sprite,1,1,1,1,0,{255,255,255},false,&background);
    check("weather_transparency",pixels[0]==0x07e0);
    game.EnterSceneAmbient(0);_TDEFAULTMESSAGE header={};char packet[64]={};
    fnMakeDefMessage(&header,SM_DAYCHANGING,0,2,1,3);fnEncodeMessage(&header,packet,sizeof(packet));
    game.OnSocketMessageRecieve(packet);
    check("day_message",game.m_sceneAmbient.mode==1 && game.m_sceneAmbient.fading && game.m_weather.other);
    packet[DEFBLOCKSIZE]='!';game.m_sceneAmbient.mode=2;game.OnSocketMessageRecieve(packet);
    check("day_payload_rejected",game.m_sceneAmbient.mode==2);
    game.m_weather.select(0);game.m_weather.particles={};game.m_weather.active=0;game.m_weather.clear_rain();
    game.m_sceneAmbient={};game.EnterSceneAmbient(0);fclose(report);
}
inline bool Scene(CGameProcess& game,int mode) {
    if(!SpellPreview::MapScene(game)) return false;
    Checks(game);game.ClearNativeSpells();game.m_weather.random_state=1;
    game.m_xMyHero.SetMotionFrame(_MT_STAND,2);game.m_xMyHero.m_bUseEffect=FALSE;game.m_xMyHero.m_spellLightActive=false;
    game.m_xMyHero.m_bLightSize=8;
    game.EnterSceneAmbient(mode==1?1:mode==2?2:0);
    if(mode==1) {
        game.StartGroundSpell(mir3::MagicEffectId::fire_wave,{325,267},{334,267},2);
        game.StartGroundSpell(mir3::MagicEffectId::ice_wave,{325,274},{334,274},2);
        for(int i=0;i<9;++i) game.AdvanceNativeSpells(81,game.m_nextNativeSpell);
    }
    game.m_weather.select(mode==3?1:mode==4?3:mode==5?2:0);
    if(mode>=3) for(int i=0;i<150;++i) game.m_weather.advance(33,true);
    return true;
}
}

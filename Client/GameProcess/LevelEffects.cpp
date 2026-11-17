#include "StdAfx.h"
#include "Common/LoginMessages.h"
#include "Common/LegacyPanel.h"
#include <array>
#include <algorithm>

bool CLevelEffects::Load()
{
    if(loaded) return true;
    std::string plain;if(!LoadLegacyText("./MInfo.dat",plain)) return false;
    bool magic=false;size_t position=0;
    while(position<plain.size()) {
        const auto cr=plain.find('\r',position);if(cr==plain.npos) break;
        const auto line=plain.substr(position,cr-position);position=cr+2;
        if(line.find("#MAGIC ")!=line.npos) {magic=true;continue;}
        if(line=="}") magic=false;
        const auto bracket=line.find('[');if(!magic || bracket==line.npos) continue;
        const auto close=line.find(']',bracket);if(close==line.npos) continue;
        std::array<int,19> columns={};size_t start=bracket+1;
        for(auto& value:columns) {
            const auto comma=line.find(',',start);if(comma==line.npos || comma>close) break;
            value=std::strtol(line.c_str()+start,nullptr,10);start=comma+1;
        }
        if(columns[4]!=Progression::LevelResource) continue;
        g_xSpriteInfo.SetEffectInfo(&resource,columns[0],columns[1],WORD(columns[2]),WORD(columns[3]),WORD(columns[4]),
            BYTE(columns[5]),BYTE(columns[6]),BYTE(columns[7]),BYTE(columns[8]),BYTE(columns[9]),
            BYTE(columns[10]),BYTE(columns[11]),BYTE(columns[12]),BYTE(columns[13]),BYTE(columns[14]),BYTE(columns[15]),
            BYTE(columns[16]),BYTE(columns[17]),BYTE(columns[18]));
        if(resource.wImgIdx!=Progression::LevelLibrary || resource.bBlendType!=Progression::ScreenBlend || resource.dwEndFrm<=resource.dwFstFrm) return false;
        image.NewLoad("./Data/MagicEx.wil");loaded=image.NewSetIndex(resource.dwFstFrm)!=FALSE;return loaded;
    }
    return false;
}
bool CLevelEffects::Start(POINT cell)
{
    if(!Load()) return false;
    Playback effect;effect.cell=cell;effect.frame=resource.dwFstFrm;
    effect.inner=resource.bLightRadius[0];effect.outer=resource.bLightRadius[1];active.push_back(effect);
    g_xSound.PlayActorWav(cell.x,cell.y,g_xGameProc.m_xMyHero.m_wPosX,g_xGameProc.m_xMyHero.m_wPosY,Progression::LevelSound);
    return true;
}
void CLevelEffects::Advance(DWORD elapsed)
{
    for(auto& effect:active) {
        effect.totalElapsed+=elapsed;effect.frameElapsed+=elapsed;
        if(effect.totalElapsed>Progression::MaximumLifetime) {effect.phase=2;continue;}
        if(effect.frameElapsed>resource.wDelay) {
            effect.frameElapsed=0;
            if(++effect.frame>=resource.dwEndFrm) {effect.frame=resource.dwEndFrm-1;effect.phase=1;}
        }
        effect.lightElapsed+=elapsed;
        if(effect.lightElapsed>Progression::LightInterval) {
            if(!effect.phase) {
                effect.lightElapsed=0;if(++effect.lightPhase>resource.bSwingCnt) effect.lightPhase=0;
            } else {
                effect.lightPhase=0;effect.lightElapsed=Progression::LightTailInterval;
                if(effect.inner<=1 || effect.outer<=effect.inner) effect.phase=2;
                else {--effect.inner;--effect.outer;}
            }
        }
    }
    active.erase(std::remove_if(active.begin(),active.end(),[](const Playback& effect){return effect.phase==2;}),active.end());
}
void CLevelEffects::Light()
{
    if(!g_xGameProc.m_xInterface.m_xOptionWnd.m_values[9]) return;
    for(const auto& effect:active) {
        SHORT x,y;g_xGameProc.m_xMap.GetScrnPosFromTilePos(SHORT(effect.cell.x),SHORT(effect.cell.y),x,y);
        g_xGameProc.m_xLightFog.SetLightRadiusWithRing(x+_CELL_WIDTH/2,y+_CELL_HEIGHT/2,
            effect.inner+effect.lightPhase,resource.bLightColor[0][0],resource.bLightColor[0][1],resource.bLightColor[0][2],
            effect.outer+effect.lightPhase,resource.bLightColor[1][0],resource.bLightColor[1][1],resource.bLightColor[1][2]);
    }
}
void CLevelEffects::DrawAt(const Playback& effect,int x,int y)
{
    DrawSelectionFrame(image,effect.frame,x,y,SelectionBlend::Screen,true,0,nullptr,255,0xffff,0xffff,
        RGB(resource.bMagicColor[0],resource.bMagicColor[1],resource.bMagicColor[2]));
}
void CLevelEffects::Draw()
{
    for(const auto& effect:active) {
        SHORT x,y;g_xGameProc.m_xMap.GetScrnPosFromTilePos(SHORT(effect.cell.x),SHORT(effect.cell.y),x,y);DrawAt(effect,x,y);
    }
}

#pragma once
#include <vector>
namespace Progression {
constexpr int ExperienceCaption=1460,TrainingCaption=1459,ExtraCaption=1513,LevelCaption=1469;
constexpr WORD LevelResource=422,LevelLibrary=137;
constexpr BYTE ScreenBlend=2;
constexpr int LevelSound=10*LevelResource+10002,LevelDirection=4;
constexpr DWORD LightInterval=200,LightTailInterval=150,MaximumLifetime=60000;
}
class CLevelEffects {
public:
    struct Playback {
        POINT cell={};DWORD frame=0,frameElapsed=0,lightElapsed=0,totalElapsed=0;
        BYTE inner=0,outer=0,lightPhase=0;int phase=0;
    };
    EFFECTSPRINFO resource={};
    std::vector<Playback> active;
    bool Load();
    bool Start(POINT cell);
    void Advance(DWORD elapsed);
    void Light();
    void Draw();
    void DrawAt(const Playback& effect,int x,int y);
    void Clear() {active.clear();}
private:
    bool loaded=false;
    CWHWilImageData image;
};

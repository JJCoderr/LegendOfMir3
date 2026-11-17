#pragma once
#include "LegacyMovie.h"

// Native DirectDraw/DirectSound presentation of the reference AVI decoder.
class CNativeMovie {
    friend struct MoviePreview;
    mir3::LegacyMovie m_movie;
    LPDIRECTSOUND m_sound=nullptr;
    LPDIRECTSOUNDBUFFER m_audio=nullptr;
    RECT m_destination={0,0,640,480};
public:
    ~CNativeMovie();
    void InitAvi(LPDIRECTSOUND sound) {m_sound=sound;}
    bool Create(const char* filename,bool loop=false,bool forward=true);
    void SetDestRect(RECT destination) {m_destination=destination;}
    void Start();
    void Stop();
    void ReleaseAvi() {Stop();}
    bool Draw(int milliseconds);
    bool Playing() const {return m_movie.playing();}
};

#include "stdafx.h"
#include "NativeMovie.h"

CNativeMovie::~CNativeMovie() {Stop();}
void CNativeMovie::Stop() {
    if(m_audio) {m_audio->Stop();m_audio->Release();m_audio=nullptr;}
    m_movie.close();
}
bool CNativeMovie::Create(const char* filename,bool loop,bool forward) {
    Stop();
    // All lobby movies play once; the reverse frame order follows MIR3.
    return !loop && m_movie.open(std::filesystem::path(filename),!forward);
}
void CNativeMovie::Start() {
    if(m_audio) return;
    const auto& pcm=m_movie.audio();
    if(!m_movie.playing() || pcm.pcm.empty() || !m_sound || !g_xSound.WavEnabled()) return;
    WAVEFORMATEX format={};format.wFormatTag=WAVE_FORMAT_PCM;
    format.nChannels=WORD(pcm.channels);format.nSamplesPerSec=pcm.rate;format.wBitsPerSample=WORD(pcm.bits);
    format.nBlockAlign=WORD(pcm.channels*pcm.bits/8);format.nAvgBytesPerSec=format.nSamplesPerSec*format.nBlockAlign;
    DSBUFFERDESC description={};description.dwSize=sizeof(description);description.dwFlags=DSBCAPS_CTRLVOLUME;
    description.dwBufferBytes=DWORD(pcm.pcm.size());description.lpwfxFormat=&format;
    if(FAILED(m_sound->CreateSoundBuffer(&description,&m_audio,nullptr))) return;
    void *first=nullptr,*second=nullptr;DWORD firstSize=0,secondSize=0;
    if(FAILED(m_audio->Lock(0,description.dwBufferBytes,&first,&firstSize,&second,&secondSize,0))) {
        m_audio->Release();m_audio=nullptr;return;
    }
    memcpy(first,pcm.pcm.data(),firstSize);
    if(secondSize) memcpy(second,pcm.pcm.data()+firstSize,secondSize);
    m_audio->Unlock(first,firstSize,second,secondSize);
    m_audio->SetVolume(g_xSound.m_lWavMaxVol);m_audio->Play(0,0,0);
}
bool CNativeMovie::Draw(int milliseconds) {
    m_movie.advance(max(0,milliseconds));
    if(!m_movie.playing()) {Stop();return false;}
    if(m_audio) m_audio->SetVolume(g_xSound.WavEnabled()?g_xSound.m_lWavMaxVol:DSBVOLUME_MIN);
    DDSURFACEDESC2 surface={};surface.dwSize=sizeof(surface);auto* buffer=g_xMainWnd.GetBackBuffer();
    if(FAILED(buffer->Lock(nullptr,&surface,DDLOCK_WAIT,nullptr))) return true;
    const int width=m_destination.right-m_destination.left,height=m_destination.bottom-m_destination.top;
    const auto& pixels=m_movie.pixels();
    for(int y=max(0L,m_destination.top);y<min(LONG(surface.dwHeight),m_destination.bottom);++y) {
        WORD* row=(WORD*)((BYTE*)surface.lpSurface+y*surface.lPitch);
        const int sy=(y-m_destination.top)*m_movie.height()/height;
        for(int x=max(0L,m_destination.left);x<min(LONG(surface.dwWidth),m_destination.right);++x) {
            const int sx=(x-m_destination.left)*m_movie.width()/width;
            const auto* pixel=&pixels[(size_t(sy)*m_movie.width()+sx)*4];
            row[x]=WORD(((pixel[0]>>3)<<11)|((pixel[1]>>2)<<5)|(pixel[2]>>3));
        }
    }
    buffer->Unlock(nullptr);return true;
}

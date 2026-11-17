#include "LegacyMovie.h"
#ifndef NOMINMAX
#define NOMINMAX
#endif
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <vfw.h>
#include <algorithm>
#include <cmath>
namespace mir3 {
struct LegacyMovie::State {
    PAVIFILE file=nullptr;
    PAVISTREAM video=nullptr;
    PGETFRAME decoder=nullptr;
    LONG start=0,count=0,current=-1;
    int width=0,height=0;
    double elapsed=0,frame_ms=0;
    bool playing=false,reverse=false;
    std::vector<std::uint8_t> pixels;
    Audio audio;
    State() { AVIFileInit(); }
    ~State() {
        if (decoder) AVIStreamGetFrameClose(decoder);
        if (video) AVIStreamRelease(video);
        if (file) AVIFileRelease(file);
        AVIFileExit();
    }
    bool decode(LONG frame) {
        const auto* dib=static_cast<const BITMAPINFOHEADER*>(AVIStreamGetFrame(decoder,start+frame));
        if (!dib || dib->biSize<sizeof(BITMAPINFOHEADER) || dib->biBitCount!=24 || dib->biCompression!=BI_RGB ||
            dib->biWidth!=width || std::abs(dib->biHeight)!=height) return false;
        const auto* data=reinterpret_cast<const std::uint8_t*>(dib)+dib->biSize+dib->biClrUsed*sizeof(RGBQUAD);
        const auto stride=(std::size_t(width)*3+3)&~std::size_t(3);
        pixels.resize(std::size_t(width)*height*4);
        for (int y=0;y<height;++y) {
            const auto* row=data+(dib->biHeight>0?height-1-y:y)*stride;
            for (int x=0;x<width;++x) {
                const auto at=(std::size_t(y)*width+x)*4;
                pixels[at]=row[x*3+2];pixels[at+1]=row[x*3+1];pixels[at+2]=row[x*3];pixels[at+3]=255;
            }
        }
        current=frame;return true;
    }
    void read_audio() {
        PAVISTREAM stream=nullptr;
        if (AVIFileGetStream(file,&stream,streamtypeAUDIO,0)!=AVIERR_OK) return;
        WAVEFORMATEX format{};LONG size=sizeof(format);
        const LONG first=AVIStreamStart(stream),samples=AVIStreamLength(stream);
        if (samples>0 && AVIStreamReadFormat(stream,first,&format,&size)==AVIERR_OK && format.wFormatTag==WAVE_FORMAT_PCM &&
            (format.nChannels==1 || format.nChannels==2) && (format.wBitsPerSample==8 || format.wBitsPerSample==16) && format.nSamplesPerSec>0) {
            LONG bytes=0,read_samples=0;
            if (AVIStreamRead(stream,first,samples,nullptr,0,&bytes,&read_samples)==AVIERR_OK && bytes>0 && bytes<=256*1024*1024) {
                audio.pcm.resize(bytes);LONG actual=0;
                if (AVIStreamRead(stream,first,samples,audio.pcm.data(),bytes,&actual,&read_samples)==AVIERR_OK && actual>0) {
                    audio.pcm.resize(actual);audio.channels=format.nChannels;audio.rate=int(format.nSamplesPerSec);audio.bits=format.wBitsPerSample;
                } else audio={};
            }
        }
        AVIStreamRelease(stream);
    }
};
LegacyMovie::LegacyMovie()=default;
LegacyMovie::~LegacyMovie()=default;
void LegacyMovie::close() { state.reset(); }
bool LegacyMovie::open(const std::filesystem::path& path,bool reverse) {
    close();auto next=std::make_unique<State>();
    if (AVIFileOpenW(&next->file,path.c_str(),OF_READ|OF_SHARE_DENY_NONE,nullptr)!=AVIERR_OK ||
        AVIFileGetStream(next->file,&next->video,streamtypeVIDEO,0)!=AVIERR_OK) return false;
    AVISTREAMINFOW info{};
    if (AVIStreamInfoW(next->video,&info,sizeof(info))!=AVIERR_OK || !info.dwRate || !info.dwScale) return false;
    next->width=info.rcFrame.right-info.rcFrame.left;next->height=info.rcFrame.bottom-info.rcFrame.top;
    next->start=AVIStreamStart(next->video);next->count=AVIStreamLength(next->video);
    if (next->width<=0 || next->height<=0 || next->width>4096 || next->height>4096 || next->count<=0) return false;
    BITMAPINFOHEADER requested{};requested.biSize=sizeof(requested);requested.biPlanes=1;requested.biBitCount=24;
    requested.biCompression=BI_RGB;requested.biWidth=next->width;requested.biHeight=next->height;
    next->decoder=AVIStreamGetFrameOpen(next->video,&requested);
    if (!next->decoder || !next->decode(0)) return false;
    next->frame_ms=std::max(1.0,std::floor(1000.0*info.dwScale/info.dwRate+0.5));next->reverse=reverse;next->playing=true;
    next->read_audio();state=std::move(next);return true;
}
bool LegacyMovie::advance(double milliseconds) {
    if (!playing() || !std::isfinite(milliseconds) || milliseconds<0) return false;
    state->elapsed+=milliseconds;
    const LONG ticks=state->reverse?std::max<LONG>(1,state->count-1):state->count;
    if (state->elapsed>=state->frame_ms*ticks) { state->playing=false;return false; }
    LONG frame=LONG(state->elapsed/state->frame_ms);
    if (state->reverse) frame=frame?state->count-1-frame:0;
    if (frame==state->current) return false;
    if (!state->decode(frame)) { state->playing=false;return false; }
    return true;
}
bool LegacyMovie::playing() const { return state && state->playing; }
int LegacyMovie::width() const { return state?state->width:0; }
int LegacyMovie::height() const { return state?state->height:0; }
const std::vector<std::uint8_t>& LegacyMovie::pixels() const { static const std::vector<std::uint8_t> empty;return state?state->pixels:empty; }
const LegacyMovie::Audio& LegacyMovie::audio() const { static const Audio empty;return state?state->audio:empty; }
}

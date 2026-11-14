// 测试内容：合成AVI原位解码、像素方向、正放与倒放计时边界、PCM音轨和缺失文件处理。
#include "resources/legacy_movie.hpp"
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <vfw.h>
#include <array>
#include <iostream>
#include <stdexcept>
void require(bool value,const char* label) { if (!value) throw std::runtime_error(label); }
int wmain(int argc,wchar_t** argv) {
    try {
        require(argc==2,"需要构建目录内的样本文件路径");
        const std::filesystem::path path(argv[1]);
        AVIFileInit();PAVIFILE file=nullptr;
        require(AVIFileOpenW(&file,path.c_str(),OF_CREATE|OF_WRITE,nullptr)==AVIERR_OK,"创建AVI样本");
        AVISTREAMINFOW info{};info.fccType=streamtypeVIDEO;info.dwScale=1;info.dwRate=10;
        info.rcFrame={0,0,2,2};PAVISTREAM video=nullptr;
        require(AVIFileCreateStreamW(file,&video,&info)==AVIERR_OK,"创建视频流");
        BITMAPINFOHEADER format{};format.biSize=sizeof(format);format.biWidth=2;format.biHeight=2;
        format.biPlanes=1;format.biBitCount=24;format.biCompression=BI_RGB;format.biSizeImage=16;
        require(AVIStreamSetFormat(video,0,&format,sizeof(format))==AVIERR_OK,"设置像素格式");
        for (int frame=0;frame<4;++frame) {
            std::array<unsigned char,16> pixels{};
            pixels[0]=255;pixels[3]=255; // 底行蓝色，验证DIB上下翻转。
            pixels[8+2]=static_cast<unsigned char>(40+frame*40);pixels[11+2]=pixels[10];
            require(AVIStreamWrite(video,frame,1,pixels.data(),int(pixels.size()),AVIIF_KEYFRAME,nullptr,nullptr)==AVIERR_OK,"写入关键帧");
        }
        AVIStreamRelease(video);
        AVISTREAMINFOW audio_info{};audio_info.fccType=streamtypeAUDIO;audio_info.dwScale=1;audio_info.dwRate=8000;audio_info.dwSampleSize=1;
        PAVISTREAM audio=nullptr;require(AVIFileCreateStreamW(file,&audio,&audio_info)==AVIERR_OK,"创建音轨");
        WAVEFORMATEX wave{};wave.wFormatTag=WAVE_FORMAT_PCM;wave.nChannels=1;wave.nSamplesPerSec=8000;
        wave.nAvgBytesPerSec=8000;wave.nBlockAlign=1;wave.wBitsPerSample=8;
        require(AVIStreamSetFormat(audio,0,&wave,sizeof(wave))==AVIERR_OK,"设置PCM格式");
        std::array<unsigned char,3200> samples{};samples.fill(128);
        require(AVIStreamWrite(audio,0,int(samples.size()),samples.data(),int(samples.size()),0,nullptr,nullptr)==AVIERR_OK,"写入PCM样本");
        AVIStreamRelease(audio);AVIFileRelease(file);AVIFileExit();
        mir3::LegacyMovie movie;require(movie.open(path),"打开合成AVI");
        require(movie.width()==2 && movie.height()==2 && movie.pixels()[0]==40 && movie.pixels()[10]==255,"像素色序及上下方向");
        require(movie.audio().rate==8000 && movie.audio().bits==8 && movie.audio().pcm.size()==3200,"原位读取PCM音轨");
        require(!movie.advance(99) && movie.advance(1) && movie.pixels()[0]==80,"正放100毫秒帧边界");
        require(movie.advance(200) && movie.pixels()[0]==160 && movie.playing(),"正放跨帧推进");
        movie.advance(100);require(!movie.playing(),"正放到帧总数结束");
        require(movie.open(path,true) && movie.pixels()[0]==40,"倒放启动仍解码第0帧");
        require(movie.advance(100) && movie.pixels()[0]==120,"倒放首次定时取末帧前一帧");
        require(movie.advance(100) && movie.pixels()[0]==80,"倒放继续递减");
        movie.advance(100);require(!movie.playing(),"倒放索引到0即结束");
        movie.close();require(!movie.open(path.parent_path()/L"missing-movie.dat") && !movie.playing(),"缺失影片保持关闭");
        std::cout << "MOVIE_CHECKS_OK\n";
    } catch (const std::exception& error) { std::cerr << error.what() << '\n';return 1; }
}

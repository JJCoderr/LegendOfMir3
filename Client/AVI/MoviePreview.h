#pragma once
#include <array>
#include <stdexcept>
struct MoviePreview {
    enum {First=268,Reverse=269,Opening=270,Creation=271,Last=Creation};
    static inline CNativeMovie player;
    static inline int mode=First;
    static constexpr const char* Fixture="movie-preview-fixture.avi";
    static void FixtureFile() {
        const std::filesystem::path path(Fixture);
        const auto require=[](bool value,const char* label) {if(!value) throw std::runtime_error(label);};
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

    }
    static void Reply(WORD opcode,const char* body="") {
        _TDEFAULTMESSAGE header={};fnMakeDefMessage(&header,opcode,0,0,0,0);
        char encoded[512]={},tail[512]={};fnEncodeMessage(&header,encoded,sizeof(encoded));
        fnEncode6BitBuf((unsigned char*)body,tail,int(strlen(body)),sizeof(tail));
        std::string wire=std::string(encoded)+tail;g_xChrSelProc.OnSocketMessageRecieve(&wire[0]);
    }
    static void Checks() {
        FILE* report=fopen("lobby-movie-check.txt","wb");if(!report) return;
        const auto check=[&](const char* name,bool ok) {fprintf(report,"%s=%d\n",name,ok);};
        mir3::LegacyMovie movie;
        check("open",movie.open(Fixture));
        check("dimensions",movie.width()==2 && movie.height()==2);
        check("rgb_and_bottom_up",movie.pixels()[0]==40 && movie.pixels()[10]==255);
        check("pcm",movie.audio().channels==1 && movie.audio().rate==8000 && movie.audio().bits==8 && movie.audio().pcm.size()==3200 && movie.audio().pcm[0]==128);
        check("forward_99",!movie.advance(99) && movie.pixels()[0]==40);
        check("forward_100",movie.advance(1) && movie.pixels()[0]==80);
        check("forward_skip",movie.advance(200) && movie.pixels()[0]==160);
        movie.advance(100);check("forward_end",!movie.playing());
        check("reverse_initial",movie.open(Fixture,true) && movie.pixels()[0]==40);
        check("reverse_first",movie.advance(100) && movie.pixels()[0]==120);
        check("reverse_second",movie.advance(100) && movie.pixels()[0]==80);
        movie.advance(100);check("reverse_end",!movie.playing());
        check("missing_closed",!movie.open("movie-preview-missing.dat") && !movie.playing());
        player.InitAvi(g_xSound.GetSoundObject()->GetDS());
        const bool enabled=g_xSound.WavEnabled();const LONG volume=g_xSound.m_lWavMaxVol;
        g_xSound.SetEnablePlay();g_xSound.m_lWavMaxVol=-1200;
        check("native_open",player.Create(Fixture));player.Start();
        DWORD status=0;LONG actual=0;
        check("pcm_buffer",player.m_audio && SUCCEEDED(player.m_audio->GetStatus(&status)) && (status&DSBSTATUS_PLAYING));
        check("pcm_volume",player.m_audio && SUCCEEDED(player.m_audio->GetVolume(&actual)) && actual==-1200);
        auto* audio=player.m_audio;player.Start();check("start_once",audio==player.m_audio);
        player.Draw(0);g_xSound.SetDisablePlay();player.Draw(0);
        check("mute",player.m_audio && SUCCEEDED(player.m_audio->GetVolume(&actual)) && actual==DSBVOLUME_MIN);
        player.Draw(400);check("end_releases",!player.Playing() && player.m_audio==nullptr);
        player.Create(Fixture);player.Start();check("disabled_no_buffer",player.m_audio==nullptr);player.Stop();
        if(enabled) g_xSound.SetEnablePlay();g_xSound.m_lWavMaxVol=volume;
        auto& login=g_xLoginProc;login.Load(false);login.m_xAvi.SetRenderAviState(CLoginAvi::OpeningMovie);login.m_xLogin.SetEditorVisible(false);
        check("opening_pending",login.m_xAvi.Opening());login.OnKeyDown(VK_RETURN,0);
        check("opening_blocks_submit",login.m_previewRequests==0);
        login.RenderScene(INT_MAX);check("opening_returns",!login.m_xAvi.Opening() && IsWindowVisible(g_xChatEditBox.GetSafehWnd()));
        auto& character=g_xChrSelProc;character.Preview("select");
        character.m_xSelectChr.m_nRenderState=11;character.RenderScene(0);
        check("creation_transition",character.m_Progress==PRG_SEL_TO_CREA);
        character.OnKeyDown(VK_F1,0);check("no_debug_skip",character.m_Progress==PRG_SEL_TO_CREA);
        character.RenderScene(INT_MAX);check("creation_returns",character.m_Progress==PRG_CREATE_CHAR);
        character.OnLButtonUp(0,MAKELPARAM(500,450));
        check("cancel_runs_reverse",character.m_Progress==PRG_CREA_TO_SEL);
        character.OnLButtonUp(0,MAKELPARAM(450,450));check("movie_mouse_blocked",character.m_Progress==PRG_CREA_TO_SEL);
        character.RenderScene(INT_MAX);check("reverse_returns",character.m_Progress==PRG_CHAR_SELE);
        character.m_Progress=PRG_CREATE_CHAR;Reply(SM_NEWCHR_SUCCESS);
        check("success_runs_reverse",character.m_Progress==PRG_CREA_TO_SEL);
        character.RenderScene(INT_MAX);Reply(SM_STARTPLAY,"127.0.0.1/7200/");
        check("entry_message_starts_movie",character.m_Progress==PRG_SEL_TO_GAME && g_nServerPort==7200);
        character.m_xBGround.Render(INT_MAX);
        check("entry_completes",character.m_xBGround.GetRenderState()==_RENDER_STARTGAME);
        character.m_xBGround.Create(&character.m_Image);
        check("reload_resets_background",character.m_xBGround.GetRenderState()==_RENDER_STILL_1);
        character.DeleteProc();fclose(report);
    }
    static const char* Load(int requested) {
        mode=requested;LoginVisual::ResizeCanvas(640,480);FixtureFile();
        if(mode==First) Checks();
        if(mode==Opening) {g_xLoginProc.Load(false);g_xLoginProc.m_xAvi.SetRenderAviState(CLoginAvi::OpeningMovie);g_xLoginProc.RenderScene(INT_MAX);}
        else if(mode==Creation) g_xChrSelProc.Preview("create");
        else {player.InitAvi(g_xSound.GetSoundObject()->GetDS());player.Create(Fixture,false,mode!=Reverse);player.Start();player.Draw(100);}
        static const char* files[]={"movie-forward.bmp","movie-reverse.bmp","movie-opening-return.bmp","movie-creation-return.bmp"};
        return files[mode-First];
    }
    static void Render() {
        if(mode==Opening) g_xLoginProc.RenderScene(0);
        else if(mode==Creation) g_xChrSelProc.RenderScene(0);
        else {player.Draw(0);LoginVisual::Text("SYNTHETIC AVI / PCM TEST",20,20,RGB(255,255,255));}
    }
};

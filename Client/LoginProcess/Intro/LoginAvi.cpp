#include "stdafx.h"

void CLoginAvi::Create(CWHWilImageData* image) {
    StopAllAvis();m_image=image;
    m_movie.InitAvi(g_xSound.GetSoundObject()->GetDS());
}
void CLoginAvi::StopAllAvis() {m_movie.Stop();m_opening=false;}
void CLoginAvi::SetRenderAviState(INT state) {
    StopAllAvis();
    if(state==OpeningMovie) {
        m_opening=true;
        g_xSound.UpdateLobbyAudio(CMirSound::LobbyPage::None,0);
        g_xSound.StopBkMusicMp3();
        if(m_movie.Create("./Data/wemade.dat")) m_movie.Start();
    }
}
void CLoginAvi::Render(INT milliseconds) {
    if(m_opening) {if(!m_movie.Draw(milliseconds)) m_opening=false;return;}
    if(m_image->NewSetIndex(LoginLayout::Background))
        g_xMainWnd.DrawWithImageForComp(0,60,m_image->m_lpstNewCurrWilImageInfo->shWidth,
            m_image->m_lpstNewCurrWilImageInfo->shHeight,(WORD*)m_image->m_pbCurrImage);
}

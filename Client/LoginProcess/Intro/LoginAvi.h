#pragma once
#include "AVI/NativeMovie.h"
class CLoginAvi : public CBMWnd {
    CWHWilImageData* m_image=nullptr;
    CNativeMovie m_movie;
    bool m_opening=false;
public:
    enum State {OpeningMovie=1,StaticBackground=9};
    void Create(CWHWilImageData* image);
    void SetRenderAviState(INT state);
    void StopAllAvis();
    void Render(INT milliseconds) override;
    bool Opening() const {return m_opening;}
};

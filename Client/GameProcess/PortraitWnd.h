#pragma once
#include <string>
#include <vector>
struct IPicture;
class CPortraitWnd : public CGameWnd {
    IPicture* preview=nullptr;
    IPicture* ownPhoto=nullptr;
    std::string ownCachePath,ownCacheKey,versionPath;
    int ReadSelected(std::vector<BYTE>& bytes,IPicture** picture);
    void LoadPreview();
    bool Scroll(int kind,POINT point);
public:
    std::string directory=".";
    std::vector<std::string> names;
    std::vector<BYTE> requested;
    bool requestPending=false,scrolling=false;
    int first=0,selected=-1,hovered=-1,pressed=-1;
    DWORD lastUpload=0,scrollTick=0;
    ~CPortraitWnd();
    void Open();
    void Close();
    void Show();
    int HintAt(POINT point,POINT& anchor);
    int ButtonAt(POINT point);
    static int RowAt(POINT point);
    int ThumbY() const;
    void Wheel(int direction);
    BOOL Pointer(int kind,POINT point);
    void Request(int button,DWORD tick);
    bool SendRequest();
    bool ApplyOwnPhoto(const BYTE* bytes,size_t size);
    bool ShowOwnPhoto();
    bool SetCacheContext(const std::string& server,const std::string& actor,const std::string& root="./Data",const std::string& versions="./UserPic.ini");
    bool SaveOwnVersion(DWORD version);
    bool SaveOwnCache(const std::vector<BYTE>& jpeg);
    bool HasOwnPhoto() const {return ownPhoto!=nullptr;}
    bool HasPreview() const { return preview!=nullptr; }
};

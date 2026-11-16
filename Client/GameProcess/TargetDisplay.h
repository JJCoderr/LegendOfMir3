#pragma once
#include <array>
#include <map>
#include <string>
struct IPicture;
class CTargetDisplay {
    IPicture* customPortrait=nullptr;
    void ClearPortrait();
    CWHWilImageData monsters;
    DWORD shown=0,previous=0,changedAt=0;
    bool human=false,special=false;
    int frame=-1,badge=-1;
public:
    struct Attributes {
        int category=-1,state=-1,icon=-1,element=0;
        std::array<int,7> modifiers{};
        std::array<int,7> Order() const;
    };
    std::map<DWORD,Attributes> attributes;
    BYTE percent=0;WORD maximum=0,current=0;
    ~CTargetDisplay();
    void Init();void Destroy();void Clear();
    bool ApplyPortrait(DWORD actor,const BYTE* bytes,size_t size);
    bool HasPortrait() const { return customPortrait!=nullptr; }
    void Focus(DWORD actor,DWORD now);
    void Remove(DWORD actor);
    DWORD Displayed() const { return shown; }
    bool ChangesTo(DWORD actor) const {return actor && actor!=previous;}
    int Frame() const { return frame; }
    int BarWidth() const { return int(double(percent)*double(0.01f)*84.0); }
    std::string VitalText(int part) const;
    void SetMonster(DWORD actor,BYTE type,WORD shape,bool excluded,int state,WORD hp,WORD maxHp,BYTE ratio,DWORD now);
    void SetHuman(DWORD actor,BYTE type,BYTE profession,DWORD now);
    bool ApplyAttributes(DWORD actor,WORD category,WORD state,const BYTE* body,int length);
    void ShowPortrait(CWHWilImageData& portraits,bool userPhotos);
    void ShowVitals(CWHWilImageData& portraits);
    void ShowInformation(CWHWilImageData& portraits);
};

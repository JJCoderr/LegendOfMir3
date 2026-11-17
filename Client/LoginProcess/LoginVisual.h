#pragma once
extern CWHDXGraphicWindow g_xMainWnd;
#include "Common/LegacyPanel.h"
#include "Common/LoginMessages.h"
#include <vector>
#include <cstdlib>
namespace LoginVisual {
inline HFONT Font(int points=9,bool bold=false) {
    const char song[]={char(0xcb),char(0xce),char(0xcc),char(0xe5),0};
    return g_xMainWnd.CreateGameFont(song,points,0,bold?FW_BOLD:FW_NORMAL);
}
inline void Text(std::string text,int x,int y,COLORREF color,int truncate=0,int points=9,bool bold=false,COLORREF background=0) {
    HDC dc;auto* surface=g_xMainWnd.GetBackBuffer();if(FAILED(surface->GetDC(&dc))) return;
    HFONT font=Font(points,bold);const auto previous=SelectObject(dc,font);
    SIZE size={};GetTextExtentPoint32A(dc,text.c_str(),int(text.size()),&size);
    if(truncate && size.cx>truncate) while(!text.empty() && size.cx>=truncate) {
        text.pop_back();GetTextExtentPoint32A(dc,text.c_str(),int(text.size()),&size);
    }
    SetBkMode(dc,background?OPAQUE:TRANSPARENT);SetBkColor(dc,background);SetTextColor(dc,color);
    TextOutA(dc,x,y,text.c_str(),int(text.size()));
    SelectObject(dc,previous);DeleteObject(font);surface->ReleaseDC(dc);
}
inline COLORREF Palette(BYTE index) {
    constexpr COLORREF colors[]={0,657930,0xffffff,255,65280,16750230,0x50ffff,33023};
    return index<8?colors[index]:0;
}
}
// Mir3 login_announcement.hpp, using native GDI measurement and rendering.
struct LoginAnnouncement {
    struct Row {std::string text;int points=9;bool bold=false;BYTE foreground=2,background=0;};
    std::vector<Row> rows;
    int offset=0;DWORD elapsed=0;
    bool Load(const char* path) {
        std::string plain;if(!LoadLegacyText(path,plain)) return false;
        return Parse(plain);
    }
    bool Parse(std::string plain) {
        rows.clear();offset=0;elapsed=0;
        plain.resize(strlen(plain.c_str()));
        HDC dc=CreateCompatibleDC(nullptr);
        for(size_t start=0;start<plain.size();) {
            const size_t end=plain.find('\n',start);
            const size_t count=min(size_t(259),end==std::string::npos?plain.size()-start:end-start+1);
            std::string line=plain.substr(start,count);start+=count;
            if(line[0]==';') continue;
            const auto cr=line.find('\r');if(cr!=std::string::npos) line.resize(cr);
            else if(!line.empty() && line.back()=='\n') line.pop_back();
            const auto number=[&](char marker,int fallback) {const auto at=line.find(marker);return at==std::string::npos?fallback:atoi(line.c_str()+at+1);};
            Row style;style.points=number('!',9);style.bold=line.find('$')!=std::string::npos;
            style.background=BYTE(number('@',0));style.foreground=BYTE(number('#',2));
            if(style.points<1 || style.points>72) {DeleteDC(dc);rows.clear();return false;}
            const auto marker=line.find('*');if(marker!=std::string::npos) line.erase(0,marker+1);
            HFONT font=LoginVisual::Font(style.points,style.bold);const auto previous=SelectObject(dc,font);
            int rowCount=0;std::string current;
            const auto append=[&]() {auto row=style;row.text=current;rows.push_back(row);current.clear();++rowCount;};
            if(line.empty()) append();
            for(size_t i=0;i<line.size() && rowCount<10;) {
                const size_t length=BYTE(line[i])>=0x80 && i+1<line.size()?2:1;
                const std::string next=current+line.substr(i,length);SIZE size={};
                GetTextExtentPoint32A(dc,next.c_str(),int(next.size()),&size);
                if(!current.empty() && size.cx>224) append();
                if(rowCount==10) break;
                current+=line.substr(i,length);i+=length;
                if(i==line.size()) append();
            }
            SelectObject(dc,previous);DeleteObject(font);
        }
        DeleteDC(dc);return true;
    }
    void Advance(DWORD ms) {
        if(ms<10000) elapsed+=ms;
        if(elapsed<=100) return;
        elapsed=0;if(rows.size()*14<=238) return;
        --offset;if(abs(offset)>=int(rows.size()*14)-14) offset=224;
    }
    void Draw() const {
        if(rows.empty()) return;
        DrawLegacyPanel({18,115,252,353},RGB(75,50,0),100,3296100);
        for(size_t i=0;i<rows.size();++i) {
            const int y=120+14*int(i)+offset;if(y<120 || y+14>=353) continue;
            const auto& row=rows[i];
            LoginVisual::Text(row.text,23,y,LoginVisual::Palette(row.foreground),0,row.points,row.bold,LoginVisual::Palette(row.background));
        }
    }
};

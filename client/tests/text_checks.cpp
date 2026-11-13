#include "text/native_text.hpp"
#include "chat/chat_history.hpp"
#include <algorithm>
#include <iostream>
int main() {
    const auto normal=mir3::raster_text("坐标 : [100,200]",133,17,true,mir3::TextFont::normal);
    const auto level=mir3::raster_text("24",16,16,true,mir3::TextFont::level);
    if (normal.rgba.size()!=133*17*4 || level.rgba.size()!=16*16*4 || normal.dpi!=96 || normal.face.empty()) return 1;
    int visible=0;
    for (std::size_t i=3;i<normal.rgba.size();i+=4) visible+=normal.rgba[i]!=0;
    if (!visible || normal.text_width<=0 || normal.text_height<=0) return 2;
    if (!mir3::raster_text("bad",-1,16,true,mir3::TextFont::normal).rgba.empty()) return 3;
    mir3::ChatHistory history;
    history.post("");history.post(std::string(201,'W'));
    if (!history.lines(false).empty()) return 4;
    history.post(std::string(200,'W'),0xabcdef,0x123456);
    if (history.lines(false).size()<=history.lines(true).size()) return 5;
    for (bool expanded:{false,true}) {
        std::string combined;
        for (const auto& row:history.lines(expanded)) {
            combined+=row.text;
            if (row.foreground!=0xabcdef || row.background!=0x123456 || mir3::raster_text(row.text,500,15,false,mir3::TextFont::normal).text_width>(expanded?429:341)) return 6;
        }
        if (combined!=std::string(200,'W')) return 7;
    }
    mir3::ChatHistory chinese;
    std::string message;
    for (int i=0;i<100;++i) message+="测";
    chinese.post(message);
    for (bool expanded:{false,true}) {
        std::string combined;
        for (const auto& row:chinese.lines(expanded)) combined+=row.text;
        if (combined!=message) return 8;
    }
    mir3::ChatHistory following;
    for (int i=0;i<20;++i) following.post(std::to_string(i));
    if (following.first(false)!=15 || following.first(true)!=6) return 9;
    if (!following.input(0,620,508,false,12,12,100) || following.first(false)!=15 || following.first(true)!=10) return 10;
    const int thumb=mir3::chat_thumb_y(15,20,false);
    if (!following.input(0,620,thumb+1,false,12,12,120) || !following.input(1,620,507,false,12,12,121) || following.first(false)!=0) return 11;
    if (!following.input(2,0,0,false,12,12,122) || following.input(1,620,520,false,12,12,123)) return 12;
    for (int i=0;i<101;++i) history.post(std::to_string(i));
    for (bool expanded:{false,true}) if (history.lines(expanded).size()!=79 || history.lines(expanded).front().text!="22" || history.lines(expanded).back().text!="100") return 13;
    std::cout<<"GDI font="<<normal.face<<" level="<<level.face<<" dpi="<<normal.dpi<<" ACP="<<normal.code_page<<'\n';
}

#include "text/native_text.hpp"
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
    std::cout<<"GDI font="<<normal.face<<" level="<<level.face<<" dpi="<<normal.dpi<<" ACP="<<normal.code_page<<'\n';
}

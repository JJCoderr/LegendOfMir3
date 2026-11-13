#pragma once
#include <array>
#include <algorithm>
#include <cstdint>
#include <string>
#include <vector>

namespace mir3 {
// Panel22: 0x41E540/0x41E680/0x41E790/0x41F100/0x41F340.
// Relative coordinates become absolute by adding panel+24/+28.
struct PortraitPicker {
    using Rect=std::array<int,4>;
    static constexpr Rect initial{0,0,580,404};
    static constexpr int frame=2090, visible_rows=12;
    static constexpr Rect preview{390,50,85,104};
    static constexpr std::array<std::array<int,3>,3> buttons{{
        {1221,539,364},{2092,59,345},{2094,179,345}
    }};
    // Draw repositions close from constructor+(400,267) to +(539,364).
    static constexpr std::array<int,3> scroll{1966,276,30};
    static constexpr int scroll_travel=274;
    static constexpr int scroll_page_rows=12; // 0x41FE80 stores page size at +20.
    static constexpr int scroll_page_midpoint=7; // 0x420200 uses +32 >> 1.
    static constexpr Rect row(int i) { return i>=0 && i<visible_rows?Rect{26,27+24*i,238,21}:Rect{}; }
    static constexpr std::array<int,2> instruction(int i) { return {320,165+15*i}; }
    static constexpr int instruction_message(int i) { return i>=0 && i<12?4702+i:-1; }
    static constexpr bool accepts_size(std::uint64_t size) { return size<=4095; }
    std::vector<std::string> names; // original linked list entries: name at +8.
    int hovered=-1;  // panel+724: row within viewport, retained outside rows.
    int selected=-1; // panel+728: absolute entry index.
    int first=0;     // panel+732.
    std::uint32_t last_upload=0; // panel+1188; close does not reset it.
    bool scrolling=false;
    std::uint32_t scroll_tick=0;
    int thumb_y() const {
        const float ratio=names.size()>1?float(double(first)/(names.size()-1)):0;
        return 30+int(scroll_travel*double(ratio));
    }
    void wheel(int direction) { // 0x41F400/0x41F420; +736 is count+1 after enumeration.
        if (direction>0 && first>0) --first;
        if (direction<0 && first<int(names.size())-1) ++first;
    }
    bool scroll_input(int kind,int x,int y,int width,int height,std::uint32_t now) {
        if (kind==2) { const bool used=scrolling;scrolling=false;scroll_tick=now;return used; }
        if (kind==1) {
            if (!scrolling) return false;
            drag(float(double(y-30)/(scroll_travel+height)));return true;
        }
        if (kind!=0 || width<=0 || height<=0 || x<276 || x>=276+width || y<30 || y>=30+scroll_travel+height) return false;
        if (now-scroll_tick<=10) return true;
        scroll_tick=now;
        const float ratio=names.size()>1?float(double(first)/(names.size()-1)):0;
        // 0x41FF91..0x41FFDF uses the image height for the vertical thumb RECT.
        const int top=thumb_y();
        if (y>=top && y<top+height) {
            scrolling=true;
            // 0x420200 captures the thumb, then 0x41F060 converts its existing
            // ratio to a row using the file count, including at ratio 1.
            drag(ratio);
            return true;
        }
        const float page=names.empty()?0:float(double(scroll_page_rows)/names.size());
        drag(ratio+(y<top+scroll_page_midpoint?-page:page));
        return true;
    }
    static int row_at(int x,int y) {
        for (int i=0;i<visible_rows;++i) {
            const auto r=row(i);
            if (x>=r[0] && x<r[0]+r[2] && y>=r[1] && y<r[1]+r[3]) return i;
        }
        return -1;
    }
    void move(int x,int y) {
        const int i=row_at(x,y);
        if (i>=0) hovered=i; // 0x41F340: no clearing when no RECT matches.
    }
    bool select(int x,int y) {
        const int i=row_at(x,y);
        if (i<0 || first+i>=static_cast<int>(names.size())) return false;
        selected=first+i;return true;
    }
    void drag(float ratio) {
        // 0x41F340 multiplies count, not count-1. At bottom an empty page is possible.
        first=static_cast<int>(names.size()*static_cast<double>(std::clamp(ratio,0.0f,1.0f)));
    }
    int selected_row() const {
        const int i=selected-first;
        return selected>=0 && i>=0 && i<visible_rows?i:-1;
    }
    bool accept_upload_tick(std::uint32_t now) {
        // Called only after JPEG validation and successful file read (0x41ECC0).
        if (selected<0 || selected>=static_cast<int>(names.size()) || now-last_upload<1000) return false;
        last_upload=now;return true;
    }
    void close() {
        names.clear();hovered=-1;selected=-1;first=0;scrolling=false; // 0x41F440.
    }
};
}

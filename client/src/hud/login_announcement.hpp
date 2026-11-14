#pragma once
#include "resources/messages.hpp"
#include <cstdlib>
#include <algorithm>
#include <string_view>
#include <functional>
namespace mir3 {
struct LoginAnnouncement {
    struct Row { std::string ansi;int points=9;bool bold=false;std::uint8_t foreground=2,background=0; };
    std::vector<Row> rows;
    int offset=0;
    std::uint32_t elapsed=0;
    bool load(std::span<const std::uint8_t> bytes,const std::function<int(std::string_view,int,bool)>& measure) {
        const auto decoded=decode_legacy_text(bytes);
        if (!decoded || decoded->size()>1024*1024) return false;
        LoginAnnouncement next;
        std::string_view input(*decoded);input=input.substr(0,input.find(char(0)));
        while (!input.empty()) {
            const auto newline=input.find('\n');
            const auto length=std::min<std::size_t>(259,newline==std::string_view::npos?input.size():newline+1);
            std::string line(input.substr(0,length));input.remove_prefix(length);
            if (!line.empty() && line[0]==';') continue;
            const auto end=line.find('\r');if (end!=std::string::npos) line.resize(end);
            else if (!line.empty() && line.back()=='\n') line.pop_back();
            Row style;
            const auto number=[&](char marker,int fallback) {
                const auto at=line.find(marker);if (at==std::string::npos) return fallback;
                return int(std::strtol(line.c_str()+at+1,nullptr,10));
            };
            style.points=number('!',9);if (style.points<1 || style.points>72) return false;
            style.background=std::uint8_t(number('@',0));style.foreground=std::uint8_t(number('#',2));
            style.bold=line.find('$')!=std::string::npos;
            const auto marker=line.find('*');std::string_view text(line);
            if (marker!=std::string::npos) text.remove_prefix(marker+1);
            std::string current;int count=0;
            const auto append_row=[&]() {
                auto row=style;row.ansi=std::move(current);next.rows.push_back(std::move(row));current.clear();++count;
            };
            if (text.empty()) append_row();
            for (std::size_t i=0;i<text.size() && count<10;) {
                const auto c=std::uint8_t(text[i]);const auto size=c>=0x80 && i+1<text.size()?2u:1u;
                const auto unit=text.substr(i,size);
                // 48C03A：恰好等宽仍留在本行；溢出的完整ANSI字符移到下一行。
                if (!current.empty() && measure(current+std::string(unit),style.points,style.bold)>224) append_row();
                if (count==10) break;
                current.append(unit);i+=size;
                if (i==text.size()) append_row();
            }
        }
        *this=std::move(next);return true;
    }
    void advance(std::uint32_t ms) {
        if (ms<10000) elapsed+=ms;
        if (elapsed<=100) return;
        elapsed=0;
        if (rows.size()*14<=238) return;
        --offset;
        if (std::abs(offset)>=int(rows.size()*14)-14) offset=224;
    }
};
}

#pragma once
#include "Common/LoginMessages.h"
#include <cstdint>
#include <vector>
#include <string>
#include <array>
#include <cstdlib>
#include <string_view>

namespace mir3 {
// 按原版46EE20的参数顺序命名已确认字段，不依赖宿主结构体布局。
struct EffectResource {
    std::uint32_t first=0,end=0;
    std::uint16_t interval=0,library=0,id=0;
    std::uint8_t inner_radius=0,outer_radius=0;
    std::array<std::uint8_t,3> inner_color{},outer_color{},tint{};
    std::uint8_t blend=0,opacity=0,light_steps=0;
    static EffectResource from_columns(const std::array<std::int32_t,19>& columns) {
        EffectResource r;
        r.first=std::uint32_t(columns[0]);r.end=std::uint32_t(columns[1]);
        r.interval=std::uint16_t(columns[2]);r.library=std::uint16_t(columns[3]);r.id=std::uint16_t(columns[4]);
        r.inner_radius=std::uint8_t(columns[5]);r.outer_radius=std::uint8_t(columns[6]);
        for (int i=0;i<3;++i) {
            r.inner_color[i]=std::uint8_t(columns[7+i]);r.outer_color[i]=std::uint8_t(columns[10+i]);r.tint[i]=std::uint8_t(columns[13+i]);
        }
        r.blend=std::uint8_t(columns[16]);r.opacity=std::uint8_t(columns[17]);r.light_steps=std::uint8_t(columns[18]);
        return r;
    }
};
class EffectCatalog {
public:
    enum class Section { spell,magic,explosion };
    static EffectCatalog& shared() {
        static EffectCatalog catalog;
        static const bool loaded=catalog.load("./MInfo.dat");
        return catalog;
    }
    // 原版4703E0逐CRLF扫描，只在逗号处转换数值；闭括号前的末项不转换。
    bool load(const char* path) {
        std::string text;
        if(!LoadLegacyText(path,text)) return false;
        const std::string* decoded=&text;
        std::array<std::vector<EffectResource>,3> pending;
        int section=-1;
        bool inside=false;
        std::size_t used=0,position=0;
        while (position<decoded->size()) {
            const auto cr=decoded->find('\r',position);
            if (cr==std::string::npos) break;
            const std::string line=decoded->substr(position,cr-position);position=cr+2;
            const auto hash=line.find('#');
            if (hash!=std::string::npos) {
                const auto space=line.find(' ',hash);
                if (space==std::string::npos) continue;
                const auto tag=line.substr(hash,space-hash);
                const int selected=tag=="#SPELL"?0:tag=="#MAGIC"?1:tag=="#EXPLOSION"?2:-1;
                if (selected<0) continue;
                const auto count=std::strtol(line.c_str()+space+1,nullptr,10);
                if (count<0 || count>100000) return false;
                section=selected;pending[section].assign(std::size_t(count),{});
            } else if (line=="{") {
                if (section<0) return false;
                inside=true;used=0;
            } else if (line=="}") { inside=false;section=-1; }
            else if (const auto bracket=line.find('[');bracket!=std::string::npos) {
                if (!inside || section<0 || used>=pending[section].size()) return false;
                const auto close=line.find(']',bracket);
                if (close==std::string::npos) return false;
                std::array<std::int32_t,19> columns{};
                std::size_t start=bracket+1;
                for (std::size_t i=0;i<columns.size();++i) {
                    const auto comma=line.find(',',start);
                    if (comma==std::string::npos || comma>close) break;
                    columns[i]=std::int32_t(std::strtol(line.c_str()+start,nullptr,10));start=comma+1;
                }
                pending[section][used++]=EffectResource::from_columns(columns);
            }
        }
        if (inside || section!=-1) return false;
        sections=std::move(pending);return true;
    }
    const EffectResource* find(Section section,std::uint16_t id) const {
        for (const auto& item:sections[std::size_t(section)]) if (item.id==id) return &item;
        return nullptr;
    }
private:
    std::array<std::vector<EffectResource>,3> sections;
};
}

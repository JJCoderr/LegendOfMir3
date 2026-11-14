#pragma once
#include "character_selection.hpp"
namespace mir3 {
struct CreateCharacterPage {
    bool visible=false;
    int selected=0,profession=0;
    std::array<SelectionRole,2> roles;
    std::array<unsigned char,5> buttons{};
    void set_profession(int job) {
        if (job<0 || job>2) return;
        profession=job;buttons={};
        constexpr std::array<std::array<int,2>,6> positions{{{110,110},{110,120},{110,120},{400,160},{420,115},{425,118}}};
        for (int gender=0;gender<2;++gender) {
            auto& role=roles[gender];role={};role.occupied=true;role.profession=job;role.gender=gender;
            const auto position=positions[job+3*gender];role.x=position[0];role.y=position[1];role.start(4);
        }
    }
    void open() { visible=true;set_profession(0); }
    void close() { visible=false;buttons={}; }
    bool select(int gender) {
        if (gender<0 || gender>1 || gender==selected) return false;
        selected=gender;roles[selected].start(4);return true;
    }
    static int name_error(std::string_view name) {
        if (name.size()>13) return 223;
        constexpr std::string_view forbidden=" /@?'\"\\.,:;`~!#$%^&*()-_+=|[{}]<>";
        for (std::size_t i=0;i<name.size();) {
            const unsigned char c=name[i];
            if (!c || forbidden.find(char(c))!=std::string_view::npos) return 225;
            if ((c>='a' && c<='z') || (c>='A' && c<='Z') || (c>='0' && c<='9')) ++i;
            else { if (i+1>=name.size() || !name[i+1]) return 225;i+=2; }
        }
        return 0;
    }
    std::optional<std::string> request(std::string_view account,std::string_view name) const {
        if (!visible || account.empty() || name.empty() || name_error(name)) return {};
        return std::string(account)+"/"+std::string(name)+"/1/"+std::to_string(profession)+"/"+std::to_string(selected);
    }
    static int failure_message(std::uint32_t reason) { return reason==2?224:(reason==3?226:232); }
    void advance(std::uint32_t ms) { if (visible) for (auto& role:roles) role.advance(ms); }
};
}

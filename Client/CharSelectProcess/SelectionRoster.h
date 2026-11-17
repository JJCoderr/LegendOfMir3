#pragma once
#include <array>
#include <string>
#include <string_view>
#include <cstdlib>
#include <cerrno>
#include <optional>

struct SelectionRoleInfo {
    BYTE Class=0,Gender=0,Level=0;
    char ChrName[25]={};
};
struct SelectionRoster {
    static constexpr int Capacity=4,NameBytes=24,Delay=500;
    std::array<SelectionRoleInfo,Capacity> roles={};
    int count=0;
    bool Parse(unsigned announced,std::string_view body) {
        if(announced>INT_MAX || body.size()>255 || body.find(char(0))!=std::string_view::npos) return false;
        SelectionRoster next;next.count=int(min(announced,unsigned(Capacity)));
        for(int i=0;i<next.count;++i) {
            if(!body.empty() && body.front()=='*') body.remove_prefix(1);
            std::array<std::string,5> fields;
            for(int f=0;f<5;++f) {
                const auto end=body.find('/');
                if(end==std::string_view::npos && (f!=4 || i+1!=next.count)) return false;
                fields[f]=body.substr(0,end);body=end==std::string_view::npos?std::string_view{}:body.substr(end+1);
            }
            if(fields[0].empty() || fields[0].size()>NameBytes) return false;
            const auto byte=[](const std::string& field,BYTE& value) {
                errno=0;const long number=strtol(field.c_str(),nullptr,10);
                if(errno==ERANGE) return false;value=BYTE(number);return true;
            };
            auto& role=next.roles[i];
            if(!byte(fields[1],role.Class) || !byte(fields[3],role.Level) || !byte(fields[4],role.Gender) || role.Class>2 || role.Gender>1) return false;
            memcpy(role.ChrName,fields[0].data(),fields[0].size());
        }
        *this=next;return true;
    }
    bool ParseLegacy(unsigned announced,const void* data,int bytes) {
        static_assert(sizeof(CHRINFO)==23,"Legacy character packet changed");
        if(announced>3 || bytes!=int(announced*sizeof(CHRINFO))) return false;
        SelectionRoster next;next.count=int(announced);const auto* packed=static_cast<const CHRINFO*>(data);
        for(int i=0;i<next.count;++i) {
            auto& role=next.roles[i];role.Class=packed[i].Class;role.Gender=packed[i].Gender;role.Level=packed[i].Level;
            if(role.Class>2 || role.Gender>1) return false;
            memcpy(role.ChrName,packed[i].ChrName,sizeof(packed[i].ChrName));role.ChrName[sizeof(packed[i].ChrName)-1]=0;
        }
        *this=next;return true;
    }
};

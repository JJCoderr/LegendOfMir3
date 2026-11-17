#pragma once
#include "Common/LoginConfiguration.h"
#include <optional>
#include <charconv>
#include <string_view>
// Native adaptation of Mir3 hud/server_groups.hpp. Configurations are snapshots:
// selecting a group never rewrites Mir3.ini or the supplied resource directory.
struct ServerGroups {
    struct Server {std::string caption,loginSection,updateSection;std::optional<LoginConfiguration> config;};
    struct Group {std::string label,title;int x=0,y=0;std::vector<Server> servers;};
    std::vector<Group> groups;
    int selected=-1,first=0;
    static constexpr int Limit=40,VisibleRows=20;
    static std::string Get(const char* path,const std::string& section,const std::string& key) {
        char value[32768]={};GetPrivateProfileStringA(section.c_str(),key.c_str(),"",value,sizeof(value),path);return value;
    }
    static std::optional<int> Integer(std::string_view value) {
        while(!value.empty() && (value.front()==' ' || value.front()=='\t')) value.remove_prefix(1);
        while(!value.empty() && (value.back()==' ' || value.back()=='\t')) value.remove_suffix(1);
        int number=0;const auto parsed=std::from_chars(value.data(),value.data()+value.size(),number);
        if(parsed.ec!=std::errc{} || parsed.ptr!=value.data()+value.size()) return {};
        return number;
    }
    static std::vector<std::string> Fields(std::string_view value) {
        std::vector<std::string> result;
        while(true) {const auto end=value.find(',');result.emplace_back(value.substr(0,end));
            if(end==std::string_view::npos) break;value.remove_prefix(end+1);}
        return result;
    }
    static std::optional<LoginConfiguration> ReadLogin(const char* path,const std::string& section) {
        if(section=="empty") return {};
        LoginConfiguration config;config.host=Get(path,section,"ServerAddr");
        const auto port=Integer(Get(path,section,"Param1"));
        if(config.host.empty() || config.host.size()>15 || !port || *port<1 || *port>65535) return {};
        config.port=WORD(*port);config.accountUrl=Get(path,section,"Param2");config.passwordUrl=Get(path,section,"Param3");
        const std::string list=Get(path,section,"ServerList");std::string_view rest=list;
        while(true) {
            const auto open=rest.find('(');if(open==std::string_view::npos) break;
            rest.remove_prefix(open+1);const auto close=rest.find(')'),slash=rest.find('/');
            if(close==std::string_view::npos || slash==std::string_view::npos || slash>close || config.servers.size()>=256) return {};
            config.servers.push_back({std::string(rest.substr(0,slash)),std::string(rest.substr(slash+1,close-slash-1))});
            rest.remove_prefix(close+1);
        }
        return config;
    }
    bool Load(const char* path) {
        ServerGroups next;const auto count=Integer(Get(path,"ServerGroup","Count"));
        if(!count || *count<0 || *count>Limit) return false;
        for(int i=1;i<=*count;++i) {
            const auto fields=Fields(Get(path,"ServerGroup","Server"+std::to_string(i)));
            if(fields.size()!=4) return false;
            const auto x=Integer(fields[2]),y=Integer(fields[3]);
            if(!x || !y || *x<0 || *x>800 || *y<0 || *y>600) return false;
            Group group{fields[0],fields[1],*x,*y,{}};
            const std::string section="ServerGroup"+std::to_string(i);
            const auto servers=Integer(Get(path,section,"Count"));if(!servers || *servers<0 || *servers>Limit) return false;
            for(int j=1;j<=*servers;++j) {
                const auto row=Fields(Get(path,section,"Server"+std::to_string(j)));
                if(row.size()!=3 || row[0].size()>31 || row[1].size()>31 || row[2].size()>31) return false;
                group.servers.push_back({row[0],row[1],row[2],ReadLogin(path,row[1])});
            }
            next.groups.push_back(std::move(group));
        }
        *this=std::move(next);return true;
    }
    bool Choose(int group) {
        if(group<0 || group>=int(groups.size())) return false;
        selected=group;first=0;return true;
    }
    void Scroll(int delta) {
        if(selected<0) return;
        first=(std::max)(0,(std::min)(first+delta,int(groups[selected].servers.size())-1));
    }
    std::optional<LoginConfiguration> Login(int row) const {
        if(selected<0 || row<0 || row>=VisibleRows || first+row>=int(groups[selected].servers.size())) return {};
        return groups[selected].servers[first+row].config;
    }
};

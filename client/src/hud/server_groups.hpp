#pragma once
#include "resources/login_config.hpp"
namespace mir3 {
struct ServerGroups {
    struct Server { std::string caption,login_section,update_section; };
    struct Group { std::string label,title;int x=0,y=0;std::vector<Server> servers; };
    std::vector<Group> groups;
    std::map<std::string,std::string> ini;
    bool visible=false;
    int selected=-1,first=0;
    std::string get(std::string section,std::string key) const {
        const auto it=ini.find(LoginConfig::lower(std::move(section))+"/"+LoginConfig::lower(std::move(key)));
        return it==ini.end()?std::string{}:it->second;
    }
    static std::optional<int> integer(std::string_view value) {
        while (!value.empty() && (value.front()==' ' || value.front()=='\t')) value.remove_prefix(1);
        while (!value.empty() && (value.back()==' ' || value.back()=='\t')) value.remove_suffix(1);
        int number=0;const auto parsed=std::from_chars(value.data(),value.data()+value.size(),number);
        if (parsed.ec!=std::errc{} || parsed.ptr!=value.data()+value.size()) return {};
        return number;
    }
    static std::vector<std::string> fields(std::string_view value) {
        std::vector<std::string> result;
        while (true) {
            const auto end=value.find(',');result.emplace_back(value.substr(0,end));
            if (end==std::string_view::npos) break;value.remove_prefix(end+1);
        }
        return result;
    }
    bool load(const std::filesystem::path& path) {
        const auto values=LoginConfig::read_values(path);if (!values) return false;
        ServerGroups next;next.ini=*values;
        const auto count=integer(next.get("ServerGroup","Count"));if (!count || *count<0 || *count>40) return false;
        for (int i=1;i<=*count;++i) {
            const auto parts=fields(next.get("ServerGroup","Server"+std::to_string(i)));
            if (parts.size()!=4) return false;
            const auto x=integer(parts[2]),y=integer(parts[3]);if (!x || !y || *x<0 || *x>800 || *y<0 || *y>600) return false;
            Group group{parts[0],parts[1],*x,*y,{}};
            const auto section="ServerGroup"+std::to_string(i);
            const auto servers=integer(next.get(section,"Count"));if (!servers || *servers<0 || *servers>40) return false;
            for (int j=1;j<=*servers;++j) {
                const auto row=fields(next.get(section,"Server"+std::to_string(j)));
                if (row.size()!=3 || row[0].size()>31 || row[1].size()>31 || row[2].size()>31) return false;
                group.servers.push_back({row[0],row[1],row[2]});
            }
            next.groups.push_back(std::move(group));
        }
        next.visible=true;*this=std::move(next);return true;
    }
    bool choose_group(int group) {
        if (group<0 || group>=int(groups.size())) return false;
        selected=group;first=0;return true;
    }
    void scroll(int delta) {
        if (selected<0) return;
        first=std::clamp(first+delta,0,std::max(0,int(groups[selected].servers.size())-1));
    }
    std::optional<LoginConfig> login(int row) const {
        if (selected<0 || row<0 || row>=20 || first+row>=int(groups[selected].servers.size())) return {};
        const auto& server=groups[selected].servers[first+row];if (server.login_section=="empty") return {};
        LoginConfig config;const auto& section=server.login_section;
        config.gateway.host=get(section,"ServerAddr");const auto port=integer(get(section,"Param1"));
        if (config.gateway.host.empty() || config.gateway.host.size()>15 || !port || *port<1 || *port>65535) return {};
        config.gateway.port=std::uint16_t(*port);config.account_url=get(section,"Param2");config.password_url=get(section,"Param3");
        const auto list=get(section,"ServerList");std::string_view rest=list;
        while (true) {
            const auto open=rest.find('(');if (open==std::string_view::npos) break;
            rest.remove_prefix(open+1);const auto close=rest.find(')'),slash=rest.find('/');
            if (close==std::string_view::npos || slash==std::string_view::npos || slash>close || config.servers.size()>=256) return {};
            config.servers.push_back({std::string(rest.substr(0,slash)),std::string(rest.substr(slash+1,close-slash-1)),0});
            rest.remove_prefix(close+1);
        }
        return config;
    }
};
}

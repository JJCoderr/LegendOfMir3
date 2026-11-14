#pragma once
#include "hud/login_flow.hpp"
#include <cctype>
#include <filesystem>
#include <fstream>
#include <map>
namespace mir3 {
struct LoginConfig {
    LoginEndpoint gateway{"192.168.0.200",7000};
    std::vector<LoginServer> servers;
    std::string account_url,password_url;
    static std::string trim(std::string_view text) {
        while (!text.empty() && (text.front()==' ' || text.front()=='\t' || text.front()=='\r')) text.remove_prefix(1);
        while (!text.empty() && (text.back()==' ' || text.back()=='\t' || text.back()=='\r')) text.remove_suffix(1);
        return std::string(text);
    }
    static std::string lower(std::string value) {
        for (auto& c:value) if (c>='A' && c<='Z') c=char(c-'A'+'a');
        return value;
    }
    static std::optional<std::map<std::string,std::string>> read_values(const std::filesystem::path& path) {
        std::ifstream file(path,std::ios::binary);
        if (!file) return {};
        std::map<std::string,std::string> values;
        std::string section,line;
        while (std::getline(file,line)) {
            line=trim(line);
            if (line.empty() || line.front()==';') continue;
            if (line.front()=='[' && line.back()==']') { section=lower(trim(std::string_view(line).substr(1,line.size()-2)));continue; }
            const auto split=line.find('=');if (split==std::string::npos) continue;
            const auto key=lower(trim(std::string_view(line).substr(0,split)));
            auto value=trim(std::string_view(line).substr(split+1));
            if (value.size()>=2 && ((value.front()=='"' && value.back()=='"') || (value.front()==char(39) && value.back()==char(39)))) value=value.substr(1,value.size()-2);
            values.try_emplace(section+"/"+key,std::move(value));
        }
        return values;
    }
    bool load(const std::filesystem::path& path) {
        const auto parsed_values=read_values(path);if (!parsed_values) return false;
        const auto& values=*parsed_values;
        LoginConfig next;
        const auto get=[&](const std::string& key,std::string fallback={}) {
            const auto found=values.find(key);return found==values.end()?fallback:found->second;
        };
        const auto integer=[&](const std::string& key,int fallback)->std::optional<int> {
            const auto value=get(key);if (value.empty()) return fallback;
            int result=0;const auto parsed=std::from_chars(value.data(),value.data()+value.size(),result);
            if (parsed.ec!=std::errc{} || parsed.ptr!=value.data()+value.size()) return {};
            return result;
        };
        next.gateway.host=get("initial/serveraddr",next.gateway.host);
        const auto port=integer("initial/param1",7000),count=integer("server/servercount",0);
        if (!port || *port<1 || *port>65535 || !count || *count<0 || *count>256 || next.gateway.host.empty() || next.gateway.host.size()>15) return false;
        next.gateway.port=std::uint16_t(*port);
        for (int i=1;i<=*count;++i) {
            const auto stem="server/server"+std::to_string(i);
            auto caption=get(stem+"caption"),name=get(stem+"name");
            if (caption.size()>259 || name.size()>259) return false;
            next.servers.push_back({std::move(caption),std::move(name),0});
        }
        next.account_url=get("initial/param2","http://account.wemade.com/main01.html");
        next.password_url=get("initial/param3","http://account.wemade.com/main03.html");
        *this=std::move(next);return true;
    }
};
}

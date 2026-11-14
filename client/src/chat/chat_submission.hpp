#pragma once
#include <algorithm>
#include <string>
#include <string_view>
#include <optional>
#include <vector>
namespace mir3 {
// 427400输入框分派；界面按钮不经过这里。
struct ChatSubmission {
    bool save_chat=false,snow=true;
    bool control_down=false,alt_down=false;
    int contact_selected=0,phrase_selected=0;
    std::string private_prefix;
    std::vector<std::string> contacts,phrases;
    struct Result {bool send=false,private_echo=false,delay=false,contacts_changed=false,phrases_changed=false;int dialog=0;};
    static bool remember(std::vector<std::string>& values,std::string value) {
        if(std::find(values.begin(),values.end(),value)!=values.end()) return false;
        values.push_back(std::move(value));if(values.size()>10) values.erase(values.begin());return true;
    }
    Result submit(std::string_view input,bool manager,std::string_view add_member,
                  std::string_view mount,std::string_view unmount,std::string_view recall) {
        Result result;
        if(input.empty()) return result;
        if(input==add_member) {result.dialog=manager?1426:1427;return result;}
        if(input=="@chatsave") save_chat=!save_chat;
        else if(input=="@snow") snow=!snow;
        if(input.front()=='/') {
            result.private_echo=true;
            const auto recipient=input.substr(1,input.find(' ',1)==std::string_view::npos?input.size()-1:input.find(' ',1)-1);
            if(input.size()>1) private_prefix="/"+std::string(recipient)+" ";
            result.contacts_changed=remember(contacts,private_prefix.empty()?std::string{}:private_prefix.substr(1));
        } else if(input.front()=='#') {result.phrases_changed=remember(phrases,std::string(input.substr(1)));return result;}
        result.delay=input==mount || input==unmount || input==recall;
        result.send=true;return result;
    }
    std::optional<std::string> select(bool phrase,int value,bool relative) {
        const auto& values=phrase?phrases:contacts;
        auto& selected=phrase?phrase_selected:contact_selected;
        if(relative) selected=value<0?std::max(0,selected-1):std::min(int(values.size())-1,selected+1);
        else if(value>=0 && value<int(values.size())) selected=value;
        if(selected<0 || selected>=int(values.size())) return {};
        if(!phrase) private_prefix="/"+values[selected];
        return phrase?values[selected]:private_prefix;
    }
    void clear_history() {private_prefix.clear();contacts.clear();phrases.clear();contact_selected=phrase_selected=0;control_down=alt_down=false;}
};
}

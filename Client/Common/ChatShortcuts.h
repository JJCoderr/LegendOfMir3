#pragma once
#include <string>
#include <vector>

// MIR3 427400 and Godot chat_submission.hpp; strings remain native ANSI.
class CChatShortcuts {
    std::string storage;
    void Save(bool phrase) const;
public:
    static constexpr int Capacity=10,PanelLeft=183,PanelHeight=165,RowHeight=16;
    std::vector<std::string> contacts,phrases;
    std::string privatePrefix;
    int contactSelected=0,phraseSelected=0;
    bool controlDown=false,altDown=false;
    void Clear();
    void Load(const std::string& base);
    void SetIdentity(const std::string& character,const std::string& server);
    bool Remember(const std::string& input);
    bool Select(bool phrase,int value,bool relative,std::string& output);
    bool Input(UINT message,WPARAM key);
    void Draw(bool expanded);
};
CChatShortcuts& ChatShortcuts();

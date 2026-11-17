#pragma once
#include <charconv>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

// Adapted from the completed Godot client's protocol/quest_wire.hpp; no Godot runtime.
namespace QuestWire {
constexpr WORD ListMessage=1301,AddedMessage=1302,ChildAddedMessage=1303,ChildrenMessage=1310,TextMessage=1311;
constexpr WORD QueryList=1046,QueryChildren=1048,QueryText=1049;
constexpr int MaxEncodedBytes=10922,MaxDecodedBytes=8191,MaxTextBytes=1023;
constexpr int AddedCaption=1433,ChildAddedCaption=1432,TextWidth=160;
struct Record {
    int id=0;
    std::string title;
    bool completed=false;
    int child_count=0;
    std::string summary;
};

inline std::optional<std::vector<Record>> Parse(WORD opcode,const std::string& decoded,int count,int* parent_id=nullptr) {
    const bool full=opcode==ListMessage,children=opcode==ChildrenMessage,child_added=opcode==ChildAddedMessage;
    std::string_view rest(decoded);
    rest=rest.substr(0,rest.find('\0'));
    if(count>rest.size()) return {};
    auto take=[&](bool allow_end=false)->std::optional<std::string_view> {
        const auto slash=rest.find('/');
        if(slash==rest.npos && !allow_end) return {};
        const auto field=rest.substr(0,slash);
        rest.remove_prefix(slash==rest.npos?rest.size():slash+1);
        return field;
    };
    auto number=[](std::optional<std::string_view> field,int& value) {
        if(!field || field->empty()) return false;
        const auto end=field->data()+field->size();
        const auto result=std::from_chars(field->data(),end,value);
        return result.ec==std::errc{} && result.ptr==end;
    };
    if(child_added && (!parent_id || !number(take(),*parent_id))) return {};
    std::vector<Record> list;
    list.reserve(count);
    for(int i=0;i<count;++i) {
        Record record;
        int completed=0;
        if(!number(take(),record.id)) return {};
        const auto title=take(!full && !children && !child_added && i+1==count);
        if(!title || title->empty() || title->size()>511) return {};
        record.title=*title;
        if(full || child_added) {
            bool has_summary=child_added;
            if(full) {
                if(!number(take(),record.child_count) || record.child_count<0) return {};
                has_summary=rest.find('/')!=rest.npos;
                if(!number(take(i+1==count),completed)) return {};
                record.completed=completed!=0;
            }
            if(has_summary) {
                const auto summary=take(i+1==count);
                if(!summary || summary->size()>1023) return {};
                record.summary=*summary;
            }
        }
        list.push_back(std::move(record));
    }
    if(!rest.empty()) return {};
    return list;
}
}

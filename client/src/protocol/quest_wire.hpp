#pragma once
#include "message_ids.hpp"
#include "receive_message.hpp"
#include <charconv>
#include <vector>

namespace mir3 {
struct QuestRecord {
    int id=0;
    std::string title;
    bool completed=false;
    int child_count=0;
    std::string summary;
};

inline std::optional<std::vector<QuestRecord>> decode_quest_list(
        const ReceiveHeader& header,std::span<const std::uint8_t> body) {
    const bool full=header.opcode==protocol::SM_QUEST_LIST;
    const auto count=header.parameters[full?0:2];
    if(body.size()>10922) return {};
    const auto decoded=decode_six_bit(body.data(),body.size());
    if(!decoded || decoded->size()>8191) return {};
    std::string_view rest(*decoded);
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
    std::vector<QuestRecord> list;
    list.reserve(count);
    for(unsigned i=0;i<count;++i) {
        QuestRecord record;
        int completed=0;
        if(!number(take(),record.id)) return {};
        const auto title=take(!full && i+1==count);
        if(!title || title->empty() || title->size()>511) return {};
        record.title=*title;
        if(full) {
            if(!number(take(),record.child_count) || record.child_count<0) return {};
            const bool has_summary=rest.find('/')!=rest.npos;
            if(!number(take(i+1==count),completed)) return {};
            record.completed=completed!=0;
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

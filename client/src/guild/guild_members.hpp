#pragma once
#include "protocol/message_ids.hpp"
#include "guild_overview.hpp"
#include <algorithm>
#include <cstdlib>
#include <limits>

namespace mir3 {
// 0x434000 rebuilds both lists from each decoded 756/775/776 reply.
struct GuildMembers {
    std::vector<GuildLine> members;
    std::vector<std::string> editable_members;
    bool receive(const GuildReply& reply,bool manager) {
        if ((reply.identifier!=mir3::protocol::SM_SENDGUILDMEMBERLIST && reply.identifier!=mir3::protocol::SM_GUILD_MEMBERS_UPDATE && reply.identifier!=mir3::protocol::SM_GUILD_MEMBERS_REFRESH) || reply.ansi.size()>8191) return false;
        std::string_view text=reply.ansi;text=text.substr(0,text.find(char(0)));
        std::vector<GuildLine> rows{{4003,{}},{0," "},{4007,{}}};
        std::vector<std::string> editing;
        std::string accumulated;
        int rank=0,bytes=0;
        std::size_t cursor=0;
        while (true) {
            const auto end=text.find('/',cursor);if (end==text.npos) break;
            const std::string token(text.substr(cursor,end-cursor));
            if (token.size()>259) return false;
            if (token.starts_with("#")) {
                const long number=std::strtol(token.c_str()+1,nullptr,10);
                rank=int(std::clamp(number,long(std::numeric_limits<int>::min()),long(std::numeric_limits<int>::max())));
                accumulated.clear();bytes=0;
            } else if (token.starts_with("*")) {
                const auto title="<"+token.substr(1)+">";
                rows.push_back({0,(manager ? "("+std::to_string(rank)+") " : "")+title});
                editing.push_back("#"+std::to_string(rank)+" "+title);
                accumulated.clear();bytes=0;
            } else {
                bytes+=int(token.size())+1;
                if (accumulated.size()+token.size()+1>259) return false;
                accumulated+=token+" ";
                const char next=end+1<text.size()?text[end+1]:0;
                if (bytes>60 || next=='#' || next==0) {
                    rows.push_back({0,accumulated});editing.push_back(accumulated);
                    accumulated.clear();bytes=0;
                }
            }
            cursor=end+1;
        }
        members=std::move(rows);editable_members=std::move(editing);return true;
    }
};
}

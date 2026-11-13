#pragma once
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace mir3 {
struct GuildReply { int identifier; std::string ansi; };
struct GuildLine { int message=0; std::string ansi; };

// 0x433DA0: continuation keeps the persistent ANSI accumulation buffer.
struct GuildOverview {
    std::string name,partial;
    bool manager=false;
    int section=0;
    std::vector<GuildLine> overview;
    std::vector<std::string> editable;
    bool receive_continuation(const GuildReply& reply) {
        if ((reply.identifier!=773 && reply.identifier!=774) || reply.ansi.size()>8191) return false;
        std::string_view text=reply.ansi;text=text.substr(0,text.find(char(0)));
        GuildOverview next=*this;
        std::size_t cursor=0;
        while (true) {
            const auto end=text.find('\r',cursor);if (end==text.npos) break;
            const auto line=text.substr(cursor,end-cursor);
            GuildLine row;
            if (line.starts_with("<Notice>")) { next.section=1;row.message=4004; }
            else if (line=="<KillGuilds>") { next.section=2;row.message=4005; }
            else if (line=="<AllyGuilds>") { next.section=3;row.message=4006; }
            else {
                // 0x433EDD..0x433F1F appends to persistent+1976, then copies
                // to display. It does not clear the buffer between lines.
                if (next.partial.size()+line.size()>4095) return false;
                next.partial+=line;row.ansi=next.partial;
                if (next.section==1 || next.section==2) {
                    // 0x433F39..0x433FB7 appends the same current line AGAIN
                    // before creating the editable row. Preserve original quirk.
                    if (next.partial.size()+line.size()>4095) return false;
                    next.partial+=line;next.editable.push_back(next.partial);
                }
            }
            next.overview.push_back(std::move(row));cursor=end+1;
        }
        // No store of an unterminated trailing fragment in0x433DA0.
        *this=std::move(next);return true;
    }
    bool receive(const GuildReply& reply) {
        if (reply.identifier!=753 || reply.ansi.size()>8191) return false;
        std::string_view text=reply.ansi;
        text=text.substr(0,text.find(char(0)));
        const auto first=text.find('\r');if (first==text.npos || first>63) return false;
        const auto second=text.find('\r',first+1);if (second==text.npos) return false;
        const auto third=text.find('\r',second+1);if (third==text.npos) return false;
        // Safe transactional guard around the original fixed name/buffer writes.
        GuildOverview next;
        next.name=text.substr(0,first);
        next.manager=second+1<text.size() && text[second+1]=='1';
        next.overview={{4003,{}},{0," "}};
        std::size_t cursor=third+1;
        while (true) {
            const auto end=text.find('\r',cursor);
            if (end==text.npos) { next.partial=text.substr(cursor);break; }
            const auto line=text.substr(cursor,end-cursor);
            if (line.size()>4095) return false;
            GuildLine entry{0,std::string(line)};
            if (line.starts_with("<Notice>")) { next.section=1;entry={4004,{}}; }
            else if (line=="<KillGuilds>") { next.section=2;entry={4005,{}}; }
            else if (line=="<AllyGuilds>") { next.section=3;entry={4006,{}}; }
            else if (next.section==1 || next.section==2) next.editable.emplace_back(line);
            next.overview.push_back(std::move(entry));cursor=end+1;
        }
        if (next.partial.size()>4095) return false;
        *this=std::move(next);return true;
    }
};
}

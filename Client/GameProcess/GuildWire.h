#pragma once
// Adapted from the completed Godot client; no Godot runtime dependency.
namespace GuildWire {
constexpr int SM_GUILD_OVERVIEW=773,SM_GUILD_OVERVIEW_UPDATE=774;
constexpr int SM_GUILD_MEMBERS_UPDATE=775,SM_GUILD_MEMBERS_REFRESH=776;
enum Caption { GuildHeader=4003,NoticeHeader=4004,EnemyHeader=4005,AllyHeader=4006,MembersHeader=4007,
    OpenDenied=1473,MemberAdded=1474,MemberInvalid=1475,DeleteFailureBase=1478,RankFailureBase=1481,
    AllianceFailureBase=1488,AllianceMissing=1489,BreakFailureBase=1491,
    GroupCreateBase=1494,GroupOtherBase=1498,GroupNoPermission=1499,GroupInvalidName=1500,
    GroupAlreadyJoined=1501,GroupFull=1502,GroupRemoveDenied=1503,GuildCreated=1504 };
constexpr int MaximumDecoded=8191,MaximumEncoded=10922;
}
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace GuildWire {
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
        if ((reply.identifier!=SM_GUILD_OVERVIEW && reply.identifier!=SM_GUILD_OVERVIEW_UPDATE) || reply.ansi.size()>8191) return false;
        std::string_view text=reply.ansi;text=text.substr(0,text.find(char(0)));
        GuildOverview next=*this;
        std::size_t cursor=0;
        while (true) {
            const auto end=text.find('\r',cursor);if (end==text.npos) break;
            const auto line=text.substr(cursor,end-cursor);
            GuildLine row;
            if (line.rfind("<Notice>",0)==0) { next.section=1;row.message=NoticeHeader; }
            else if (line=="<KillGuilds>") { next.section=2;row.message=EnemyHeader; }
            else if (line=="<AllyGuilds>") { next.section=3;row.message=AllyHeader; }
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
        if (reply.identifier!=SM_OPENGUILDDLG || reply.ansi.size()>8191) return false;
        std::string_view text=reply.ansi;
        text=text.substr(0,text.find(char(0)));
        const auto first=text.find('\r');if (first==text.npos || first>63) return false;
        const auto second=text.find('\r',first+1);if (second==text.npos) return false;
        const auto third=text.find('\r',second+1);if (third==text.npos) return false;
        // Safe transactional guard around the original fixed name/buffer writes.
        GuildOverview next;
        next.name=text.substr(0,first);
        next.manager=second+1<text.size() && text[second+1]=='1';
        next.overview={{GuildHeader,{}},{0," "}};
        std::size_t cursor=third+1;
        while (true) {
            const auto end=text.find('\r',cursor);
            if (end==text.npos) { next.partial=text.substr(cursor);break; }
            const auto line=text.substr(cursor,end-cursor);
            if (line.size()>4095) return false;
            GuildLine entry{0,std::string(line)};
            if (line.rfind("<Notice>",0)==0) { next.section=1;entry={NoticeHeader,{}}; }
            else if (line=="<KillGuilds>") { next.section=2;entry={EnemyHeader,{}}; }
            else if (line=="<AllyGuilds>") { next.section=3;entry={AllyHeader,{}}; }
            else if (next.section==1 || next.section==2) next.editable.emplace_back(line);
            next.overview.push_back(std::move(entry));cursor=end+1;
        }
        if (next.partial.size()>4095) return false;
        *this=std::move(next);return true;
    }
};
}
#include <algorithm>
#include <cstdlib>
#include <limits>

namespace GuildWire {
// 0x434000 rebuilds both lists from each decoded 756/775/776 reply.
struct GuildMembers {
    std::vector<GuildLine> members;
    std::vector<std::string> editable_members;
    bool receive(const GuildReply& reply,bool manager) {
        if ((reply.identifier!=SM_SENDGUILDMEMBERLIST && reply.identifier!=SM_GUILD_MEMBERS_UPDATE && reply.identifier!=SM_GUILD_MEMBERS_REFRESH) || reply.ansi.size()>8191) return false;
        std::string_view text=reply.ansi;text=text.substr(0,text.find(char(0)));
        std::vector<GuildLine> rows{{GuildHeader,{}},{0," "},{MembersHeader,{}}};
        std::vector<std::string> editing;
        std::string accumulated;
        int rank=0,bytes=0;
        std::size_t cursor=0;
        while (true) {
            const auto end=text.find('/',cursor);if (end==text.npos) break;
            const std::string token(text.substr(cursor,end-cursor));
            if (token.size()>259) return false;
            if (token.rfind("#",0)==0) {
                const long number=std::strtol(token.c_str()+1,nullptr,10);
                rank=int(number);
                accumulated.clear();bytes=0;
            } else if (token.rfind("*",0)==0) {
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

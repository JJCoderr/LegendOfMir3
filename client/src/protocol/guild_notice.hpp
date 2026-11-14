#pragma once
#include "receive_message.hpp"
#include "message_ids.hpp"
#include <bit>
namespace mir3 {
// 0x4282C0：消息名不能代替原分支，联盟OK/FAIL均按有符号原因处理。
struct GuildNotice {
    int message=0;
    bool refresh_members=false;
    static std::optional<GuildNotice> parse(const ReceiveHeader& header) {
        using namespace protocol;
        GuildNotice value;
        const auto reason=std::bit_cast<std::int32_t>(header.recognition);
        switch (header.opcode) {
        case SM_OPENGUILDDLG_FAIL:value.message=1473;break;
        case SM_GUILDADDMEMBER_OK:value.message=1474;value.refresh_members=true;break;
        case SM_GUILDDELMEMBER_OK:value.refresh_members=true;break;
        case SM_GUILDADDMEMBER_FAIL:
            if (reason>=1 && reason<=5) value.message=1474+reason;break;
        case SM_GUILDDELMEMBER_FAIL:
            if (reason>=1 && reason<=4) value.message=reason==1?1475:1478+reason;break;
        case SM_GUILDRANKUPDATE_FAIL:
            if (reason>=-7 && reason<=-2) value.message=1481-reason;break;
        case SM_BUILDGUILD_OK:value.message=1504;break;
        case SM_BUILDGUILD_FAIL:
            if (reason>=-3 && reason<=-1) value.message=1504-reason;break;
        case SM_GUILDMAKEALLY_OK:case SM_GUILDMAKEALLY_FAIL:
            if (reason>=-4 && reason<=-1) value.message=1488-reason;break;
        case SM_GUILDBREAKALLY_OK:case SM_GUILDBREAKALLY_FAIL:
            if (reason>=-3 && reason<=-1) value.message=reason==-1?1489:1491-reason;break;
        default:return {};
        }
        return value;
    }
};
}

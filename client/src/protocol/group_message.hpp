#pragma once
#include "receive_message.hpp"
#include "message_ids.hpp"
#include <bit>
#include <vector>
namespace mir3 {
struct GroupMessage {
    enum class Kind { allow,clear,members,notice };
    Kind kind=Kind::notice;
    int notice=0;
    std::vector<std::string> members;
    static std::optional<GroupMessage> parse(const ReceiveHeader& header,std::span<const std::uint8_t> body) {
        GroupMessage value;
        const auto reason=std::bit_cast<std::int32_t>(header.recognition);
        switch (header.opcode) {
        case protocol::SM_CREATEGROUP_OK: value.kind=Kind::allow;break;
        case protocol::SM_GROUPCANCEL: value.kind=Kind::clear;break;
        case protocol::SM_GROUPMEMBERS: {
            value.kind=Kind::members;
            const auto text=decode_six_bit(body.data(),body.size());
            if (!text || text->size()>8192 || text->find('\0')!=std::string::npos) return {};
            std::size_t begin=0;
            while (true) {
                const auto slash=text->find('/',begin);if (slash==std::string::npos) break;
                if (slash-begin>31) return {}; // 原版分配32字节名字；拒绝越界，不复制其溢出。
                value.members.push_back(text->substr(begin,slash-begin));begin=slash+1;
            }
            break;
        }
        case protocol::SM_CREATEGROUP_FAIL:
            if (reason>=-4 && reason<=-1) value.notice=1494-reason;
            break;
        case protocol::SM_ADDGROUPMEMBER_FAIL:
            if (reason==-1) value.notice=1499;
            if (reason==-2) value.notice=1500;
            if (reason==-3) value.notice=1501;
            if (reason==-4) value.notice=1498;
            if (reason==-5) value.notice=1502;
            break;
        case protocol::SM_DELGROUPMEMBER_FAIL:
            if (reason>=-3 && reason<=-1) value.notice=reason==-3?1503:1498-reason;
            break;
        default:return {};
        }
        return value;
    }
};
}

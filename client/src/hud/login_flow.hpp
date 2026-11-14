#pragma once
#include "protocol/message_ids.hpp"
#include "protocol/receive_message.hpp"
#include <algorithm>
#include <bit>
#include <charconv>
#include <string_view>
#include <vector>
namespace mir3 {
struct LoginRequest {
    std::uint16_t opcode=0;
    std::string body;
};
struct LoginServer {
    std::string caption,name;
    int load=0;
};
struct LoginNotice {
    int message=0;
    std::optional<int> argument;
};
struct LoginEndpoint {
    std::string host;
    std::uint16_t port=0;
};
struct LoginFlow {
    enum class Phase { credentials,servers,leaving };
    Phase phase=Phase::credentials;
    // 0账号、1密码、2回车提交后隐藏；密码不供快照或日志使用。
    int field=0,pressed_server=-1,billing_type=0;
    bool editor_visible=true;
    std::vector<LoginServer> servers;
    LoginNotice notice;
    LoginEndpoint endpoint,previous_endpoint;
    std::int32_t session=0;
    std::uint32_t leaving_ms=0;
    std::string selected_server;
private:
    std::string account,password;
public:
    const std::string& account_name() const { return account; }
    // 返回目标输入字段内容，仅交给编辑控件，不应写入日志。
    std::string change_field(int target,std::string editor) {
        if (phase!=Phase::credentials || target<0 || target>1) return {};
        if (target==1) account=std::move(editor);else password=std::move(editor);
        field=target;editor_visible=true;
        return field==1?password:account;
    }
    std::optional<LoginRequest> submit(bool mouse,std::string editor) {
        if (phase!=Phase::credentials || notice.message) return {};
        change_field(!field,std::move(editor));
        if (!mouse && (field!=0 || account.empty() || password.empty())) return {};
        if (!mouse) field=2;
        editor_visible=false;
        return LoginRequest{mir3::protocol::CM_IDPASSWORD,account+"/"+password};
    }
    void press_server(int index) {
        if (phase==Phase::servers && !notice.message) pressed_server=index>=0 && index<int(servers.size())?index:-1;
    }
    std::optional<LoginRequest> release_server(int index) {
        const int pressed=pressed_server;pressed_server=-1;
        if (phase!=Phase::servers || notice.message || index<0 || index>=int(servers.size()) || index!=pressed) return {};
        selected_server=servers[index].name;return LoginRequest{mir3::protocol::CM_SELECTSERVER,selected_server};
    }
    void close_notice() {
        notice={};
        if (phase==Phase::credentials) { password.clear();field=1;editor_visible=true; }
    }
    std::string editor_text() const { return field==1?password:account; }
    std::string password_mask() const { return std::string(password.size(),'*'); }
    static int failed_login_message(std::int32_t reason) {
        switch (reason) { case -7:return 0;case -6:return 112;case -5:return 117;case -2:return 114;case -1:return 113;default:return 116; }
    }
    bool receive(std::span<const std::uint8_t> message) {
        const auto header=ReceiveHeader::decode_plain(message);
        if (!header) return false;
        const auto recognition=std::bit_cast<std::int32_t>(header->recognition);
        if (header->opcode==mir3::protocol::SM_CERTIFICATION_FAIL || header->opcode==mir3::protocol::SM_ID_NOTFOUND || header->opcode==mir3::protocol::SM_PASSWD_FAIL || header->opcode==mir3::protocol::SM_OUTOFCONNECTION) {
            const int id=header->opcode==mir3::protocol::SM_CERTIFICATION_FAIL?119:(header->opcode==mir3::protocol::SM_ID_NOTFOUND?112:(header->opcode==mir3::protocol::SM_OUTOFCONNECTION?121:failed_login_message(recognition)));
            if (id) notice={id,{}};
            return true;
        }
        if (header->opcode!=mir3::protocol::SM_PASSOK_SELECTSERVER && header->opcode!=mir3::protocol::SM_SELECTSERVER_OK) return false;
        const auto decoded=decode_six_bit(message.data()+16,message.size()-16);
        if (!decoded || decoded->size()>255 || decoded->find(char(0))!=std::string::npos) return false;
        auto next=*this;
        std::string_view body=*decoded;
        if (header->opcode==mir3::protocol::SM_PASSOK_SELECTSERVER) {
            while (true) {
                const auto end=body.find(';');
                if (end==std::string_view::npos) break;
                const auto entry=body.substr(0,end);
                const auto open=entry.find('('),close=entry.find(')');
                if (open==std::string_view::npos || close==std::string_view::npos || close<open) return false;
                int status=0;const auto number=entry.substr(open+1,close-open-1);
                const auto parsed=std::from_chars(number.data(),number.data()+number.size(),status);
                if (parsed.ec!=std::errc{} || parsed.ptr!=number.data()+number.size()) return false;
                for (auto& server:next.servers) if (server.name==entry.substr(0,open)) server.load=status;
                body.remove_prefix(end+1);
            }
            next.phase=Phase::servers;next.editor_visible=false;
            const auto p0=header->parameters[0],p1=header->parameters[1],p2=header->parameters[2];
            if (p1) { next.billing_type=4;next.notice=p1==1?LoginNotice{102,{}}:LoginNotice{103,int(p1)-1}; }
            else if (p2) { next.billing_type=2;next.notice=p2==1?LoginNotice{105,{}}:LoginNotice{104,int(p2)-1}; }
            else if (p0) { next.billing_type=1;if (p0<=100) next.notice={106,int(p0)}; }
            else if (recognition>0) { next.billing_type=3;next.notice={107,recognition}; }
        } else {
            const auto slash=body.find('/');
            if (slash==std::string_view::npos || slash==0 || slash>15) return false;
            const auto tail=body.substr(slash+1);
            const auto end=tail.find('/');
            if (end==std::string_view::npos) return false;
            const auto port_text=tail.substr(0,end);unsigned port=0;
            const auto parsed=std::from_chars(port_text.data(),port_text.data()+port_text.size(),port);
            if (parsed.ec!=std::errc{} || parsed.ptr!=port_text.data()+port_text.size() || port==0 || port>65535) return false;
            next.previous_endpoint=next.endpoint;next.endpoint={std::string(body.substr(0,slash)),std::uint16_t(port)};
            next.session=recognition;next.phase=Phase::leaving;next.leaving_ms=0;
        }
        *this=std::move(next);return true;
    }
    float brightness() const {
        return phase==Phase::leaving?float((255.0-double(leaving_ms)*255.0*double(0.0005f))*double(1.f/255.f)):1.f;
    }
    bool advance(std::uint32_t ms) {
        if (phase!=Phase::leaving || leaving_ms>=2000) return false;
        leaving_ms=std::uint32_t(std::min<std::uint64_t>(2000,std::uint64_t(leaving_ms)+ms));
        return leaving_ms==2000;
    }
};
}

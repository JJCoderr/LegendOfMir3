// 测试内容：账号字段切换、提交与失败恢复、选服及连接交接、建角校验、入场过渡、半包粘包、密钥挑战和登录音效时序。
#include "hud/login_flow.hpp"
#include "hud/server_groups.hpp"
#include "resources/wil_archive.hpp"
#include "hud/create_character.hpp"
#include "hud/game_entry.hpp"
#include "hud/login_audio.hpp"
#include "protocol/send_message.hpp"
#include "protocol/login_stream.hpp"
#include "protocol/game_connection.hpp"
#include <iostream>
#include <stdexcept>
using namespace mir3;
using namespace mir3::protocol;
void require(bool condition,const char* description) { if (!condition) throw std::runtime_error(description); }
std::span<const std::uint8_t> bytes(const std::string& text) { return {reinterpret_cast<const std::uint8_t*>(text.data()),text.size()}; }
std::string reply(std::uint16_t id,std::uint32_t recognition=0,std::string_view body={}) {
    SendMessages sender;const auto wire=sender.packet(id,recognition,0,0,body);return wire.substr(2,wire.size()-3);
}
#ifdef _WIN32
int wmain(int argc,wchar_t** argv) {
#else
int main(int argc,char** argv) {
#endif
    try {
        if (argc==2) {
            const std::filesystem::path root(argv[1]);ServerGroups groups;
            require(groups.load(root/"SelLogin.ini"),"服务器组配置解析");
            WilArchive archive;require(archive.open(root/"Data"/"LoginInterface"),"服务器组图包打开");
            for (int frame:{0,3,4,7,8,11,12,13,15}) {
                const auto value=archive.frame(frame);
                if (!value) { std::cerr << "缺少服务器组帧 " << frame << '\n';return 1; }
            }
        }
        LoginFlow login;login.endpoint={"127.0.0.1",7000};login.servers={{"测试一区","Zone1",0}};
        require(login.change_field(1,"account").empty() && login.field==1,"账号切换保存并聚焦密码");
        const auto request=login.submit(false,"secret");
        require(request && request->opcode==CM_IDPASSWORD && request->body=="account/secret" && login.field==2 && !login.editor_visible,"回车提交与隐藏编辑框");
        auto packet=reply(SM_PASSWD_FAIL,std::uint32_t(-1));
        require(login.receive(bytes(packet)) && login.notice.message==113,"密码错误提示");
        login.close_notice();require(login.field==1 && login.editor_visible && login.password_mask().empty() && login.account_name()=="account","提示关闭清密码并保留账号");
        packet=reply(SM_PASSOK_SELECTSERVER,0,"Zone1(2);");
        require(login.receive(bytes(packet)) && login.phase==LoginFlow::Phase::servers && login.servers[0].load==2,"选服列表和负载");
        login.press_server(0);require(!login.release_server(-1),"移出后释放不选服");
        login.press_server(0);const auto selection=login.release_server(0);
        require(selection && selection->opcode==CM_SELECTSERVER && selection->body=="Zone1","同项按下释放发送内部服务器名");
        packet=reply(SM_SELECTSERVER_OK,std::uint32_t(-123),"127.0.0.1/7100/");
        require(login.receive(bytes(packet)) && login.session==-123 && login.endpoint.port==7100 && login.previous_endpoint.port==7000,"端点交接和有符号会话");
        require(!login.advance(1999) && login.advance(1) && !login.advance(1),"淡出两秒只完成一次");
        struct BillingCase { std::uint16_t p0,p1,p2;std::uint32_t rec;int message,type,argument; };
        for (const auto value:std::array<BillingCase,7>{{{50,1,1,9,102,4,0},{50,3,1,9,103,4,2},{50,0,1,9,105,2,0},{50,0,3,9,104,2,2},{50,0,0,9,106,1,50},{101,0,0,9,0,1,0},{0,0,0,9,107,3,9}}}) {
            LoginFlow billing;SendMessages sender;
            const auto wire=sender.packet(SM_PASSOK_SELECTSERVER,value.rec,std::uint32_t(value.p0)|(std::uint32_t(value.p1)<<16),value.p2);
            const auto payload=wire.substr(2,wire.size()-3);
            require(billing.receive(bytes(payload)) && billing.notice.message==value.message && billing.billing_type==value.type && billing.notice.argument.value_or(0)==value.argument,"计费参数优先级和提示参数");
        }
        for (const auto value:std::array<std::pair<int,int>,7>{{{-7,0},{-6,112},{-5,117},{-2,114},{-1,113},{0,116},{99,116}}}) {
            LoginFlow rejected;const auto error=reply(SM_PASSWD_FAIL,std::uint32_t(value.first));
            require(rejected.receive(bytes(error)) && rejected.notice.message==value.second,"认证失败原因映射");
        }
        LoginFlow empty;require(!empty.submit(false,"") && empty.submit(true,"").has_value(),"空字段键盘与鼠标提交差异");
        CreateCharacterPage creation;creation.open();creation.set_profession(2);creation.select(1);
        require(creation.request("account","Hero")==std::optional<std::string>("account/Hero/1/2/1"),"创建职业性别和正文顺序");
        require(CreateCharacterPage::name_error("Bad Name")==225 && CreateCharacterPage::name_error(std::string(14,'x'))==223 && !creation.request("account",""),"非法名字和空名拒绝");
        require(CreateCharacterPage::failure_message(2)==224 && CreateCharacterPage::failure_message(3)==226 && CreateCharacterPage::failure_message(0)==232,"创建失败原因映射");
        GameEntry entry;packet=reply(SM_STARTPLAY,0,"127.0.0.1/7200/");
        require(entry.receive(bytes(packet),"Hero") && entry.pending && entry.role=="Hero" && entry.endpoint.port==7200,"开始游戏只准备端点");
        require(entry.finish_transition() && !entry.finish_transition(),"影片结束只交接一次");
        LoginStream stream;auto received=stream.receive(bytes(std::string("junk#fir*")));
        require(received.heartbeat && received.messages.empty(),"心跳剔除并保留半包");
        received=stream.receive(bytes(std::string("st!#second!#old#third!")));
        require(received.messages==std::vector<std::string>{"first","second","third"},"粘包及新起始符重同步");
        const auto encoded=GameConnection::challenge_response(0x12345678,[](int low,int) { return low; });
        require(encoded.size()==40 && GameConnection::challenge_word(std::string_view(encoded).substr(0,20))==0x5678 && GameConnection::challenge_word(std::string_view(encoded).substr(20,20))==0x1234,"挑战响应恢复身份字节顺序");
        GameConnection connection;const auto challenge=encoded.substr(0,20)+encoded.substr(20,20)+encoded.substr(0,20);
        require(connection.change_key(challenge) && connection.key==std::array<std::uint8_t,4>{0x78,0x56,0x78,0x56} && connection.secondary==0x1234,"三个挑战区块映射");
        const auto old_key=connection.key;require(!connection.change_key("bad") && connection.key==old_key,"坏挑战保留现有密钥");
        LoginAudio audio;audio.enter(LoginAudio::Page::credentials);
        require(audio.advance(1200).empty() && audio.advance(1)=="Opening.wav" && audio.advance(10000).empty(),"开场音乐严格大于1200毫秒且只触发一次");
        std::cout << "LOGIN_MODEL_OK\n";
    } catch (const std::exception& error) { std::cerr << error.what() << '\n';return 1; }
}

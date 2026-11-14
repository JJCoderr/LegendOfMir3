// 测试内容：角色查询与进入请求、失败提示、角色列表协议、选中及翻页动画、界面模型、选项确认与聊天联动、场景消息、人物行为、地图及资源解析。
#include "protocol/message_ids.hpp"
#include "hud/system_options.hpp"
#include "hud/exit_guard.hpp"
#include "hud/exit_transition.hpp"
#include "hud/character_selection.hpp"
#include "hud/selection_pixels.hpp"
#include "inventory/inventory_grid.hpp"
#include "hud/hud_model.hpp"
#include "hud/ground_items.hpp"
#include "hud/scene_lighting.hpp"
#include "protocol/receive_message.hpp"
#include "protocol/send_message.hpp"
#include "protocol/map_message.hpp"
#include "hud/map_input_delay.hpp"
#include "chat/scene_chat.hpp"
#include "hud/human_actions.hpp"
#include "hud/scene_human_queue.hpp"
#include "hud/human_life.hpp"
#include "hud/combat_stance.hpp"
#include "hud/human_idle.hpp"
#include "resources/sound_list.hpp"
#include "hud/scene_name.hpp"
#include "hud/scene_health.hpp"
#include "chat/chat_reception.hpp"
#include "resources/wil_archive.hpp"
#include "resources/messages.hpp"
#include <iterator>
#include <iostream>
#include <stdexcept>
void check(bool value,const char* message) { if (!value) throw std::runtime_error(message); }
#ifdef _WIN32
int wmain(int argc,wchar_t** argv) {
#else
int main(int argc,char** argv) {
#endif
    try {
        check(argc==2,"resource root required");
        if (std::filesystem::path(argv[1])=="--selection-requests") {
            mir3::SelectionSession session;
            check(session.configure("account",-123) && session.query_body()=="account/-123" && session.start_body("Hero")=="account/Hero","查询及进入正文顺序，编号有符号格式");
            check(!session.configure("invalid/name",1) && !session.configure("",1) && !session.configure(std::string(28,'a'),1) && !session.configure("other",INT64_MAX) && session.account=="account" && session.number==-123,"无效会话原子拒绝");
            mir3::SendMessages sender;
            for (int opcode:{mir3::protocol::CM_QUERYCHR,mir3::protocol::CM_SELCHR}) {
                const auto packet=sender.packet(opcode,0,0,0,opcode==mir3::protocol::CM_QUERYCHR?session.query_body():session.start_body("Hero"));
                const auto header=mir3::ReceiveHeader::decode_plain({reinterpret_cast<const std::uint8_t*>(packet.data()+2),16});
                const auto body=mir3::decode_six_bit(reinterpret_cast<const std::uint8_t*>(packet.data()+18),packet.size()-19);
                check(header && header->opcode==opcode && header->recognition==0 && header->parameters==std::array<std::uint16_t,3>{0,0,0},"普通请求头参数全部为零");
                check(body && *body==(opcode==mir3::protocol::CM_QUERYCHR?"account/-123":"account/Hero"),"查询和进入分别编码正文");
            }
            const auto fields=sender.packet(mir3::protocol::SM_QUERYCHR,0x12345678,0xabcdef01,0x2345);
            const auto plain=mir3::ReceiveHeader::decode_plain({reinterpret_cast<const std::uint8_t*>(fields.data()+2),16});
            check(plain && plain->recognition==0x12345678 && plain->opcode==mir3::protocol::SM_QUERYCHR && plain->parameters==std::array<std::uint16_t,3>{0xef01,0xabcd,0x2345},"普通消息头全部字段按小端读取，无场景密钥异或");
            mir3::SelectionNotice notice;
            for (int opcode:{mir3::protocol::SM_STARTFAIL,mir3::protocol::SM_QUERYCHR_FAIL,mir3::protocol::SM_OUTOFCONNECTION}) {
                notice.show(mir3::SelectionNotice::reply_message(opcode));
                check(notice.message==(opcode==mir3::protocol::SM_STARTFAIL?221:(opcode==mir3::protocol::SM_QUERYCHR_FAIL?220:222)) && !notice.pressed,"进入失败回包映射原版提示");
                notice.pressed=true;notice.close();check(!notice.message && !notice.pressed,"提示关闭清理按下状态");
            }
            check(!mir3::SelectionNotice::reply_message(mir3::protocol::SM_STARTPLAY),"进入成功不误当失败提示");
            std::cout << "SELECTION_REQUESTS_MODEL_OK\n";return 0;
        }
        if (std::filesystem::path(argv[1])=="--selection-list") {
            std::vector<std::uint8_t> canvas(640*480*4,0);
            mir3::WilFrame pixel;pixel.width=1;pixel.height=1;pixel.offset_x=-1;pixel.offset_y=2;
            pixel.pixels.rgba={248,0,0,255};
            mir3::selection_blit(canvas,pixel,1,0,mir3::SelectionBlend::inactive);
            const auto offset=640*2*4;
            check(canvas[offset]==128 && canvas[offset+1]==128 && canvas[offset+2]==128,"未选中灰度按565通道与0.6增强截断");
            mir3::selection_blit(canvas,pixel,1,0,mir3::SelectionBlend::shadow);
            check(canvas[offset]==184 && canvas[offset+1]==64 && canvas[offset+2]==64,"阴影逐565通道50混合并截断");
            mir3::selection_blit(canvas,pixel,1,0,mir3::SelectionBlend::additive);
            check(canvas[offset]==248 && canvas[offset+1]==64 && canvas[offset+2]==64,"光效加色饱和及帧偏移");
            pixel.pixels.rgba[3]=0;mir3::selection_blit(canvas,pixel,1,0,mir3::SelectionBlend::normal);
            check(canvas[offset]==248,"透明像素保留背景");
            mir3::CharacterSelectionPage page;
            mir3::SendMessages sender;
            const auto packet=sender.packet(mir3::protocol::SM_QUERYCHR,4,0,0,"*One/0/ignored/24/0/Two/1/x/31/1/Three/2/y/256/0/Four/2/z/55/1/");
            const auto bytes=std::span(reinterpret_cast<const std::uint8_t*>(packet.data()+2),packet.size()-3);
            check(page.receive(bytes) && page.count==4 && page.selected==0 && page.page==0 && !page.connecting,"520普通消息头、四槽位及默认选中");
            check(page.roles[0].name=="One" && page.roles[0].x==250 && page.roles[1].x==350 && page.roles[1].y==250,"星号跳过及双角色坐标");
            check(page.roles[2].level==0 && page.roles[3].gender==1,"等级写入BYTE与性别字段");
            check(page.roles[0].frame==260 && page.roles[1].frame==1100 && page.roles[2].frame==1460,"职业性别动作帧表");
            page.advance(120);check(page.roles[0].frame==260,"120毫秒仍不推进");
            page.advance(1);check(page.roles[0].frame==261 && page.roles[0].elapsed==0,"121毫秒前进一帧并清零");
            page.advance(1000);check(page.roles[0].frame==262 && page.roles[2].frame==1460,"长帧不追赶且隐藏页不推进");
            check(page.select(1) && page.roles[0].action==3 && page.roles[1].action==1,"按下新角色触发选中及取消动画");
            page.advance(121);const int selected_frame=page.roles[1].frame;
            check(!page.select(1) && page.roles[1].frame==selected_frame && !page.select(2),"重复选择及隐藏角色不改变动画");
            for (int i=0;i<20;++i) page.advance(121);
            check(page.roles[0].action==0 && page.roles[1].action==2,"取消转待机、选中转循环");
            check(page.turn_page(1) && page.selected==2 && page.roles[1].action==0,"翻页取消旧角色并选择第三槽");
            check(page.select(3) && page.turn_page(0) && page.selected==0 && page.turn_page(1) && page.selected==2,"翻页总选择当前页首槽");
            const auto old_name=page.roles[0].name;
            for (auto body:{"bad", "Name/3/x/1/0", "Name/0/x/1/2", "Name/0/x/99999999999999999999999/0", "/0/x/1/0"})
                check(!page.receive_list(1,body) && page.count==4 && page.selected==2 && page.roles[0].name==old_name,"畸形列表原子拒绝");
            check(!page.receive(bytes.first(15)) && page.count==4,"截断头不清理现有列表");
            const auto wrong=sender.packet(mir3::protocol::SM_NEWCHR_SUCCESS,1,0,0,"Name/0/x/1/0");
            check(!page.receive({reinterpret_cast<const std::uint8_t*>(wrong.data()+2),wrong.size()-3}),"非520回包不作为列表");
            check(page.receive_list(1,"Single/1//33/0") && page.roles[0].x==300 && page.roles[0].y==210 && !page.turn_page(1),"单角色居中、末尾斜杠可省略");
            check(page.receive_list(5,"A/0/x/1/0/B/0/x/1/0/C/0/x/1/0/D/0/x/1/0/ignored") && page.count==4,"超过四角色只取前四条");
            for (int job=0;job<3;++job) for (int gender=0;gender<2;++gender) for (int action=0;action<5;++action) {
                mir3::SelectionRole role;role.profession=job;role.gender=gender;role.start(action);
                check(role.first_frame()==200+60*(action+5*(gender+job*2)),"全部职业性别动作首帧");
                role.frame=role.last_frame();role.advance(121);
                check(role.action==(action==1?2:(action==3?0:action)) && role.frame==role.first_frame(),"全部动作末帧衔接");
            }
            check(page.receive_list(0,"") && page.count==0 && page.selected==-1 && !page.roles[0].occupied,"空列表清理槽位");
            std::cout << "SELECTION_LIST_MODEL_OK\n";return 0;
        }
        if (std::filesystem::path(argv[1])=="--exit-transition") {
            mir3::ExitTransition transition;
            check(!transition.advance(3000) && transition.brightness()==1,"未开始时不离场");
            transition.begin();check(!transition.advance(1250) && transition.brightness()>0.50f && transition.brightness()<0.52f,"中段亮度按原版255减毫秒乘0.1");
            check(!transition.advance(1250) && transition.active && transition.brightness()>0.019f && transition.brightness()<0.020f,"2500毫秒仍显示末帧");
            check(transition.advance(1) && !transition.active && transition.elapsed==0,"2501毫秒离场且复位");
            check(!transition.advance(10000),"离场事件只触发一次");
            transition.begin();transition.advance(1000);transition.begin();check(transition.elapsed==0,"再次确认重启原版计时");
            std::cout << "EXIT_TRANSITION_MODEL_OK\n";return 0;
        }
        if (std::filesystem::path(argv[1])=="--exit-guard") {
            mir3::ExitGuard guard;
            check(!guard.allowed(8000) && guard.allowed(8001),"原版8000毫秒边界仍限制，8001放行");
            for (int i=0;i<3;++i) {
                guard.ticks={0,0,0};guard.ticks[i]=10000;
                check(!guard.allowed(18000) && guard.allowed(18001),"三个时间字段均独立限制退出");
            }
            guard.ticks={0xfffffff0u,0xfffffff0u,0xfffffff0u};
            check(!guard.allowed(7984) && guard.allowed(7985),"DWORD回绕仍保持8000边界");
            guard.action=19;check(guard.allowed(0xfffffff0u),"死亡动作19豁免等待");
            guard.action=18;check(!guard.allowed(0xfffffff0u),"其他动作不套用死亡豁免");
            std::cout << "EXIT_GUARD_MODEL_OK\n";return 0;
        }
        if (std::filesystem::path(argv[1])=="--exit-request") {
            mir3::SendMessages sender;
            const auto logout=sender.packet(mir3::protocol::CM_LOGOUT);
            const auto raw=mir3::decode_six_bit(reinterpret_cast<const std::uint8_t*>(logout.data()+2),16);
            check(logout.size()==19 && logout.substr(0,2)=="#1" && logout.back()=='!' && raw && raw->size()==12,"退出请求只有带边界的编码头");
            for (int i=0;i<12;++i) check(std::uint8_t((*raw)[i])==(i==4?0xf1:(i==5?3:0)),"1009头其余字段全部为零");
            const auto npc=sender.packet(mir3::protocol::CM_MERCHANTDLGSELECT,0x12345678,0xabcdef01,0x2345,std::string_view("abc\0ignored",11));
            const auto nh=mir3::decode_six_bit(reinterpret_cast<const std::uint8_t*>(npc.data()+2),16);
            const auto body=mir3::decode_six_bit(reinterpret_cast<const std::uint8_t*>(npc.data()+18),npc.size()-19);
            check(npc[1]=='2' && nh && std::uint8_t((*nh)[0])==0x78 && std::uint8_t((*nh)[9])==0xab && std::uint8_t((*nh)[11])==0x23 && body && *body=="abc","NPC与退出共用序号，字段和正文编码保持");
            for (int i=3;i<=9;++i) check(sender.packet(mir3::protocol::CM_LOGOUT)[1]=='0'+i,"序号依次增长");
            check(sender.packet(mir3::protocol::CM_LOGOUT)[1]=='1',"序号九后回一");
            std::cout << "EXIT_REQUEST_MODEL_OK\n";return 0;
        }
        if (std::filesystem::path(argv[1])=="--options-input") {
            mir3::SystemOptions options;
            check(options.toggle_frame(7,true)==1292 && options.toggle_frame(7,false)==1295,"普通开关帧对应原版");
            options.motion(7);check(options.button_states[7]==1,"鼠标命中产生悬停态");
            options.press(7);options.motion(19);
            check(options.button_states[7]==2 && options.button_states[19]==1 && options.toggle_frame(7,true)==1295,"按下态保留且绘制反向普通帧");
            options.release(19);check(options.button_states[7]==0 && options.button_states[19]==0,"释放依原版遍历顺序复位");
            options.press(11);options.receive(0x40000000,0);check(options.button_states[11]==0,"状态包换帧会复位对应按钮");
            options.press(20);options.cancel_buttons();check(options.button_states[20]==0,"取消交互清除所有按钮状态");
            std::cout << "OPTIONS_INPUT_MODEL_OK\n";return 0;
        }
        if (std::filesystem::path(argv[1])=="--options-wire") {
            mir3::SystemOptions options;
            mir3::ChatReceptionButtons buttons;
            std::array<bool,3> rejected{};
            check(options.receive(0xCF800000u,4)==0x7f,"七个掩码包含最高位且顺序对应窗口");
            buttons.receive_options(0x7f,rejected);
            check(rejected==std::array{true,true,true} && buttons.frames==std::array{1338,1340,-1},"原版全部开启时的聊天侧效应");
            options.request(0);
            check(options.receive(0xCF800000u,2)==0 && !options.displayed[0] && options.confirmed[0] && options.attack_mode==2,"重复位不撤销预测但更新模式");
            const auto changed=options.receive(0,0);
            buttons.receive_options(changed,rejected);
            check(changed==0x7f && rejected==std::array{false,false,false} && buttons.frames==std::array{-1,-1,1342},"全部关闭按原版顺序更新");
            buttons.receive_options(options.receive(0x00800000u,65535),rejected);
            check(rejected==std::array{false,true,true} && buttons.frames[1]==-1 && options.attack_mode==65535,"所有人聊天交叉写行会变量但不改行会图标，模式保留WORD");
            check(options.receive(0x30800001u,1)==0,"无关标志不修改七项确认");
            std::cout << "OPTIONS_WIRE_MODEL_OK\n";return 0;
        }
        if (std::filesystem::path(argv[1])=="--options-panel") {
            mir3::SystemOptions options;
            check(options.show_mon && options.force_attack && options.user_photos && !options.shadow_blend,"原版新增本地设置默认值");
            options.confirmed[0]=true;
            check(options.request(0)==2123 && !options.displayed[0] && options.confirmed[0],"请求只预测显示而不改变确认值");
            check(options.request(0)==2123 && !options.displayed[0],"重复请求仍依据确认值");
            for(int i=1;i<7;++i) check(options.request(i)==2123+i,"选项命令索引对应原版");
            check(options.request(-1)==-1 && options.request(7)==-1,"非法索引不产生命令");
            std::cout << "OPTIONS_PANEL_MODEL_OK\n";return 0;
        }
        if (std::filesystem::path(argv[1])=="--map-input") {
            mir3::MapInputDelay delay;
            check(!delay.active(),"未换图时左键不延迟");
            delay.enter();delay.advance(1499);check(delay.active() && delay.remaining_ms==1,"1499ms 仍阻止左键按下");
            delay.advance(1);check(!delay.active(),"1500ms 边界解除");
            delay.enter();delay.advance(900);delay.enter();check(delay.remaining_ms==1500,"连续换图重新计时");
            delay.advance(-1);check(delay.remaining_ms==1500,"无效时间不能延长或提前解除");
            delay.advance(2000);check(!delay.active() && delay.remaining_ms==0,"长帧钳制到解除状态");
            std::cout << "MAP_INPUT_MODEL_OK\n";return 0;
        }
        if (std::filesystem::path(argv[1])=="--map-wire") {
            const auto message=mir3::MapChangeMessage::parse(mir3::protocol::SM_CHANGEMAP,{312,310,0x0301},"01");
            check(message && message->x==312 && message->y==310 && message->ambient==1 && message->series==3,"地图消息独立解析坐标、亮度及地图状态");
            check(mir3::MapChangeMessage::parse(mir3::protocol::SM_NEWMAP,{0,0,0},std::string("0\0ignored",9))->map_id=="0","地图文件名按 NUL 终止");
            for (const auto* id:{"","../0","a/b","a\\b","G:0","0.map"})
                check(!mir3::MapChangeMessage::parse(mir3::protocol::SM_NEWMAP,{0,0,0},id),"拒绝空编号、路径及扩展名");
            check(mir3::valid_map_id("A_01-2") && mir3::valid_map_id(std::string(64,'a')) && !mir3::valid_map_id(std::string(65,'a')),"地图编号字符和长度边界");
            check(!mir3::MapChangeMessage::parse(mir3::protocol::SM_MAPDESCRIPTION,{0,0,0},"0"),"名称消息不能作为地图切换");
            std::cout << "MAP_WIRE_MODEL_OK\n";return 0;
        }
        if (std::filesystem::path(argv[1])=="--scene-light") {
            mir3::SceneAmbient light;
            light.update(2);light.advance(49);check(light.half_level==510,"timer waits 50ms");
            light.advance(1);check(light.half_level==509 && light.level()==254,"half-unit timer step truncates");
            light.advance(25);light.update(2);light.advance(25);
            check(light.half_level==508,"same mode does not restart timer");
            light.advance(50000);check(light.level()==85 && !light.fading,"dawn transition clamps at endpoint");
            light.update(1);light.advance(100);check(light.level()==84,"fade toward night");
            light.enter(0);light.advance(1000);check(light.level()==255 && !light.fading,"entry cancels pending fade");
            light.enter(200);check(light.level()==10,"unknown entry fallback");
            light.update(201);light.advance(1000);check(light.level()==0,"unknown update fallback");
            auto actor=mir3::SceneLight::actor(40,3,0,true);
            check(actor.sample(1,1)==mir3::SceneLight::Rgb{100,100,100},"actor light center includes original bias");
            check(actor.sample(17,18)==mir3::SceneLight::Rgb{},"out-of-radius vertex stays dark");
            mir3::SceneLightGrid grid(15);grid.add(actor);
            check(grid.samples[19]==mir3::SceneLight::Rgb{115,115,115},"ambient plus actor contribution");
            grid.add(actor);grid.add(actor);check(grid.samples[19][0]==255,"additive lamps saturate");
            check(actor.diagonal_top_left_to_bottom_right(2,0) && !actor.diagonal_top_left_to_bottom_right(2,2),"original light triangle quadrants");
            check(mir3::SceneLight::actor(0,0,2,true).outer_radius_cells==8 && mir3::SceneLight::actor(0,0,2,false).outer_radius_cells==4,"attribute 2 local/remote radius distinction");
            check(mir3::SceneLight::actor(0,0,8,true).outer_radius_cells==10 && mir3::SceneLight::actor(0,0,8,false).outer_radius_cells==8,"attribute 8 radius distinction");
            check(mir3::SceneLight::actor(0,0,255,true).outer_radius_cells==14 && mir3::SceneLight::actor(0,0,255,false).outer_radius_cells==10,"other byte attributes use bright fallback");
            mir3::SceneHumanQueue queue;
            mir3::SceneHumanMessage update;update.opcode=mir3::protocol::SM_ACTOR_LIGHT;update.light_attribute=8;
            check(queue.push(update),"light update joins actor queue");
            update.light_attribute=256;check(!queue.push(update),"reject invalid light byte");
            queue.push({31});check(queue.fold_health().size()==1 && queue.size()==1 && queue.take()->light_attribute==8,"health folding preserves light records");
            std::cout << "SCENE_LIGHT_MODEL_OK\n";return 0;
        }
        if (std::filesystem::path(argv[1])=="--ground-items") {
            mir3::GroundItems items;
            check(items.show({1,10,20,1,"first"}) && items.show({2,10,20,2,"second"}),"multiple drops share a cell");
            check(items.show({1,30,40,3,"changed"}) && items.items.size()==2 && items.items[0].name=="first" && items.items[0].x==10,"duplicate identity is ignored");
            check(!items.show({3,1,2,0,std::string(40,'x')}),"ground name fits original 40-byte buffer");
            check(mir3::GroundItems::image_offset(17,9)==std::array<int,2>{16,12},"center image using integer halves");
            items.hide(1);items.hide(1);check(items.items.size()==1 && items.items[0].id==2,"idempotent remove retains other drop");
            std::cout << "GROUND_ITEMS_MODEL_OK\n";
            return 0;
        }
        if (std::filesystem::path(argv[1])=="--scene-text") {
            check(mir3::chat_palette(5)==16750230 && mir3::chat_palette(6)==0x50ffff && mir3::chat_palette(7)==33023,"1.45 chat palette differs from reference source");
            check(mir3::chat_palette(255)==0 && mir3::chat_opcode(mir3::protocol::SM_HEAR) && mir3::chat_opcode(mir3::protocol::SM_MONSTERSAY) && mir3::chat_opcode(mir3::protocol::SM_GUILDMESSAGE) && !mir3::chat_opcode(42),"chat dispatch classification");
            std::cout << "SCENE_TEXT_MODEL_OK\n";
            return 0;
        }
        if (std::filesystem::path(argv[1])=="--scene-wire") {
            // 原版428308：偏移4先异或k2；偏移7不参与密钥异或。
            const std::string wire="Ex\\ZnBEy<<<<<<<<";
            const std::array<std::uint8_t,4> key{18,52,86,120};
            const auto header=mir3::ReceiveHeader::decode({reinterpret_cast<const std::uint8_t*>(wire.data()),wire.size()},key);
            check(header && header->recognition==5 && header->opcode==mir3::protocol::SM_HEALTHSPELLCHANGED && header->parameters==std::array<std::uint16_t,3>{200,30,200},"fixed permuted keyed header fixture");
            const std::array<std::uint8_t,2> bad{60,61};
            check(!mir3::decode_six_bit(bad.data(),bad.size()),"reject nonzero six-bit padding");
            check(!mir3::ReceiveHeader::decode({},key),"reject truncated header");
            mir3::SceneHumanQueue queue;
            mir3::SceneHumanMessage hp{mir3::protocol::SM_STRUCK};hp.health=std::array<std::uint16_t,3>{100,200,0};
            queue.push(hp);check(queue.fold_health().empty(),"single health record stays ordered");
            queue.push({14});queue.push(hp);
            check(queue.fold_health().size()==2 && queue.size()==1 && queue.take()->opcode==mir3::protocol::SM_HIT,"fold only health records while preserving actions");
            std::cout << "SCENE_WIRE_MODEL_OK\n";
            return 0;
        }
        if (std::filesystem::path(argv[1])=="--scene-feature") {
            std::vector<std::uint8_t> bytes(21,0);
            bytes[0]=1;bytes[1]=2;bytes[2]=3;bytes[3]=129;bytes[7]=8;bytes[16]=2;
            auto feature=mir3::HumanAppearance::parse(bytes,false);
            check(feature && feature->type==1 && feature->weapon==2 && feature->clothing==3 && feature->hair==1 && feature->profession==2 && feature->mount==2 && feature->flags==0x08000000,"decoded feature fields");
            check(!mir3::HumanAppearance::parse(bytes,true)->mount,"hit preserves mount at dequeue");
            bytes.resize(16);check(mir3::HumanAppearance::parse(bytes,false)->mount==0,"base movement state clears mount");
            bytes[1]=255;bytes[3]=255;
            feature=mir3::HumanAppearance::parse(bytes,false);
            check(feature && feature->weapon==0 && feature->hair==0 && feature->profession==3,"normalize packed appearance");
            bytes[0]=2;check(!mir3::HumanAppearance::parse(bytes,false),"invalid sex rejected");
            bytes[0]=0;bytes[2]=9;check(!mir3::HumanAppearance::parse(bytes,false),"invalid clothing rejected");
            bytes.resize(15);check(!mir3::HumanAppearance::parse(bytes,false),"truncated body rejected");
            mir3::SceneHumanQueue queue;
            queue.push({10,1,2,4,0,0,feature});feature->clothing=0;
            check(queue.take()->appearance->clothing==3,"queued feature is a value snapshot");
            std::cout << "SCENE_FEATURE_MODEL_OK\n";
            return 0;
        }
        if (std::filesystem::path(argv[1])=="--scene-combat") {
            mir3::CombatStance stance;
            check(stance.idle_action(false)==0 && stance.idle_action(true)==29,"normal and mounted standing");
            stance.attack();check(stance.idle_action(false)==7,"attack enables combat stance");
            check(!stance.advance(4000),"stance still active");
            stance.attack();check(stance.remaining_ms==1001,"repeated attacks do not extend active stance");
            check(!stance.advance(1000) && stance.idle_action(false)==7,"strict 5000ms expiry");
            check(stance.advance(1) && stance.idle_action(false)==0,"expiry resets ordinary standing");
            stance.attack();stance.clear();check(stance.idle_action(false)==0,"revival clears combat stance");
            mir3::HumanIdleClock idle;
            for (int i=1;i<=6;++i) { idle.advance(201,3);check(idle.frame()==i%3,"combat idle loops three frames"); }
            check(mir3::SceneHumanMessage::accepts(7) && mir3::HumanActions::frame(23,4,2)==1882,"bump opcode and frame bank");
            std::cout << "SCENE_COMBAT_MODEL_OK\n";
            return 0;
        }
        if (std::filesystem::path(argv[1])=="--scene-life") {
            mir3::HumanLife life;
            check(life.opacity()==1 && !life.dead,"initial live state");
            check(life.die(true) && life.dead && life.dying,"animated death");
            check(!life.die(true) && !life.dying,"duplicate death does not replay");
            life.revive();check(!life.dead && !life.dying && life.opacity()==0,"revival starts transparent");
            life.advance(300);check(life.opacity()==15.f/32,"original quantized fade");
            life.advance(299);check(life.opacity()==30.f/32,"fade before endpoint");
            life.advance(1);check(life.opacity()==1,"600ms fully opaque");
            check(!life.die(false) && life.dead,"immediate corpse");
            check(mir3::SceneHumanMessage::accepts(32) && mir3::SceneHumanMessage::accepts(34) && mir3::SceneHumanMessage::accepts(1316) && mir3::SceneHumanMessage::accepts(1321),"lifecycle queue messages");
            std::cout << "SCENE_LIFE_MODEL_OK\n";
            return 0;
        }
        if (std::filesystem::path(argv[1])=="--scene-queue") {
            mir3::SceneHumanQueue queue;
            check(!queue.take() && !queue.catching_up(),"empty queue");
            check(!queue.push({99}) && !queue.push({14,0,0,8}) && !queue.push({1309,0,0,0,50}),"invalid decoded records rejected");
            check(queue.push({11,1,2,4}) && queue.push({14,3,4,2}) && !queue.catching_up(),"two records do not catch up");
            check(queue.push({1309}) && queue.catching_up(),"three records enable catch-up before dequeue");
            check(queue.take()->opcode==mir3::protocol::SM_WALK && !queue.catching_up() && queue.take()->opcode==mir3::protocol::SM_HIT && queue.take()->opcode==mir3::protocol::SM_HUMAN_STRUCK,"FIFO preserves distinct actions");
            mir3::HumanActionClock clock;
            clock.start(9,true);clock.advance(85);check(clock.frame()==0,"first catch-up interval remains full length");
            clock.advance(1);clock.advance(42);check(clock.frame()==1,"halved interval still strict");
            clock.advance(1);check(clock.frame()==2,"catch-up halves after first frame");
            clock.start(15);clock.advance(200,0x08000000);check(clock.frame()==0,"slow flag doubles finite action interval");
            clock.advance(1,0x08000000);check(clock.frame()==1,"slow action advances past boundary");
            for (int i=0;i<99999;++i) check(queue.push({10}),"queue capacity");
            check(!queue.push({10}) && queue.size()==99999,"sentinel capacity enforced");
            std::cout << "SCENE_QUEUE_MODEL_OK\n";
            return 0;
        }
        if (std::filesystem::path(argv[1])=="--scene-actions") {
            std::vector<std::uint8_t> index(58+32,0);
            index[54]=2;index[58]=140;index[74]=141;
            const std::string first="m34-1.wav",second="m35-1.wav";
            std::copy(first.begin(),first.end(),index.begin()+60);
            std::copy(second.begin(),second.end(),index.begin()+76);
            mir3::SoundList list;
            check(list.parse(index) && list.file(140)==first && list.file(141)==second && list.file(999).empty(),"sound IDs resolve mapped filenames");
            index.pop_back();check(!list.parse(index) && list.file(140)==first,"truncated index preserves previous mapping");
            mir3::HumanActionClock clock;
            for (int action: {9,10,13,15,16,17,18,32}) {
                clock.start(action);
                const auto info=mir3::HumanActions::table[action];
                clock.advance(info.interval_ms);
                check(clock.frame()==0,"strict interval boundary");
                clock.advance(1);check(clock.frame()==1,"first increment");
                clock.advance(10000);check(clock.frame()==2,"one increment per update");
                for (int i=2;i<info.count;++i) clock.advance(info.interval_ms+1);
                check(clock.finished(),"exclusive end completes finite action");
            }
            check(mir3::HumanActions::weapon_attack(2)==13 && mir3::HumanActions::weapon_attack(4)==10 && mir3::HumanActions::weapon_attack(1)==9,"weapon groups");
            check(mir3::HumanActions::frame(32,7,2)==2632,"mounted hit frame bank");
            check(mir3::HumanActions::frame_sound(9,1,2,0,0,0)==0 && mir3::HumanActions::frame_sound(9,2,2,0,0,0)==56,"swing sound only on frame two");
            check(mir3::HumanActions::frame_sound(15,1,1,2,3,0)==82,"hit source weapon and armour select impact");
            check(mir3::HumanActions::frame_sound(15,2,1,2,3,0)==138 && mir3::HumanActions::frame_sound(15,2,1,2,3,1)==139,"sex selects hurt voice");
            check(mir3::HumanActions::frame_sound(17,1,0,0,0,0)==140 && mir3::HumanActions::frame_sound(16,1,0,0,0,0)==141,"special action sounds");
            check(mir3::HumanActions::frame_sound(32,1,0,0,0,0)==0,"mounted hit does not reuse ordinary hit sounds without evidence");
            std::cout << "SCENE_ACTIONS_MODEL_OK\n";
            return 0;
        }
        if (std::filesystem::path(argv[1])=="--scene-damage") {
            mir3::DamageNumbers numbers;
            numbers.changed(0,100);check(numbers.cursor==0,"initial HP does not create healing digits");
            numbers.changed(100,88);
            const auto small=numbers.draw(100,200,true);
            check(small==std::vector<mir3::DamageGlyph>{{1800,112,164},{1791,120,160},{1792,131,160}},"small damage frame and coordinate mapping");
            check(mir3::DamageNumbers::glyphs(5,10,20)==std::vector<mir3::DamageGlyph>{{1821,0,20},{1815,10,20}},"healing frame mapping");
            check(mir3::DamageNumbers::glyphs(-150,10,20).front().frame==1840,"large damage uses alternate bank");
            for (int i=0;i<40;++i) numbers.draw(100,200,true);
            check(numbers.draw(100,200,true).empty(),"digits expire after 41 draws");
            numbers={};for(int i=0;i<8;++i) numbers.push(-1);
            check(numbers.cursor==0 && numbers.draw(0,0,false).empty(),"preserve original eight-slot cursor wrap");
            numbers.enabled=false;numbers.changed(100,90);check(numbers.cursor==0,"disabled option suppresses queue");
            std::cout << "SCENE_DAMAGE_MODEL_OK\n";
            return 0;
        }
        if (std::filesystem::path(argv[1])=="--scene-health") {
            mir3::SceneHealth health;
            check(health.apply(53,75,20,100) && !health.visible(),"health alone does not register remote bar");
            check(health.apply(1312,1,2,3) && health.current==75 && health.visible(),"registration preserves health");
            check(mir3::SceneHealth::width(50)==17 && mir3::SceneHealth::width(100)==35,"original float truncation");
            check(mir3::SceneHealth::percent(300,100)==44 && mir3::SceneHealth::percent(5,0)==0,"low-byte percentage and zero maximum");
            health.advance(4999);check(health.visible(),"bar persists before deadline");
            health.advance(1);check(!health.visible() && health.visible(true),"5000ms expiry does not affect local visibility");
            health.excluded=true;check(!health.visible(true),"exclusion hides local bar too");
            check(!health.apply(53,-1,0,0) && health.current==75,"invalid health remains atomic");
            std::cout << "SCENE_HEALTH_MODEL_OK\n";
            return 0;
        }
        if (std::filesystem::path(argv[1])=="--scene-names") {
            mir3::SceneName name;
            check(name.update(42,255,"name\rguild\rtitle/suffix"),"three-line identity");
            check(name.lines==std::array<std::string,3>{"name","guild","title"},"CR lines and trailing slash suffix");
            check(name.update(656,249,"ignored") && name.color==255 && name.lines[0]=="name","color-only update preserves text");
            check(!name.update(42,255,std::string(64,'a')) && name.lines[0]=="name" && name.color==255,"oversized identity is atomic");
            check(mir3::SceneName::rect(100,200,25,0)==std::array<int,4>{111,170,25,15},"odd-width endpoint rounding");
            std::cout << "SCENE_NAMES_MODEL_OK\n";
            return 0;
        }
        if (std::filesystem::path(argv[1])=="--actor-dyes") {
            std::vector<std::uint8_t> rgba{248,252,248,0, 248,252,248,255, 248,252,248,255, 248,252,248,255};
            const std::array<std::uint8_t,4> regions{0,1,2,3};
            mir3::tint_wil_pixels(rgba,regions,0xF800,0x07E0);
            check(rgba==std::vector<std::uint8_t>{248,252,248,0, 248,252,248,255, 248,0,0,255, 0,252,0,255},"C0/C1 preserved and C2/C3 independently tinted");
            mir3::tint_wil_pixels(rgba,regions,0,0);
            check(rgba[8]==0 && rgba[13]==0 && rgba[11]==255 && rgba[15]==255,"black dye preserves opacity");
            std::cout << "ACTOR_DYES_OK\n";
            return 0;
        }
        mir3::WilArchive archive;
        const auto resource_root=std::filesystem::path(argv[1]);
        check(archive.open(std::filesystem::path(argv[1])/"Data"/"GameInter"),"archive open");
        const auto skin=archive.frame(1160);
        check(skin && skin->width==800 && skin->height==134,"original HUD frame dimensions");
        check(!archive.frame(-1) && !archive.frame(0x7fffffff),"invalid frame bounds");
        mir3::WilArchive portraits;
        check(portraits.open(resource_root/"Data"/"ProgUse"),"portrait archive");
        for (int id=650;id<656;++id) check(portraits.frame(id).has_value(),"portrait frame");
        std::ifstream file(resource_root/"CmList.dat",std::ios::binary);
        std::vector<std::uint8_t> bytes{std::istreambuf_iterator<char>(file),{}};
        mir3::Messages messages;
        check(messages.load(bytes) && !messages.get(2612).empty() && !messages.get(2622).empty(),"HUD captions");
        bytes.back()^=1;
        check(!messages.load(bytes),"caption checksum validation");
        mir3::HudModel model;
        for (int i=0;i<23;++i) {
            auto frame=archive.frame(mir3::HudLayout::buttons[i][0]);
            check(frame.has_value(),"HUD button frame missing");
            model.button_sizes[i]={frame->width,frame->height};
            check(mir3::main_button_frames(i,false,false)==std::array{-1,-1},"idle icons belong to background");
        }
        mir3::HudController controller(model);
        check(controller.hit(682,504)==12,"inventory hit override");
        check(controller.hit(632,472)!=14,"exclusive right boundary");
        controller.pointer(0,0,true);
        check(controller.pointer(620,473,false)==14 && model.layout.expanded,"legacy release-inside behavior");
        check(model.button_rect(14)[1]==305,"expanded button origin");
        controller.pointer(620,306,false);
        check(!model.layout.expanded && model.layout.repositioned,"collapse restores bounds");
        model.hp=1;model.max_hp=3;
        check(model.orb_crop(0,{36,96},{35,80})==std::array{0,63,36,33},"float32 crop precision");
        check(model.orb_crop(1,{36,96},{35,80})==model.orb_crop(0,{36,96},{35,80}),"low-level warrior shared crop");
        model.level=24;model.mp=1;model.max_mp=2;
        check(model.orb_crop(1,{36,96},{35,80})==std::array{0,40,35,40},"independent mana crop");
        controller.pointer(685,510,true);controller.cancel();
        check(model.pressed==-1,"focus loss releases pressed state");
        model.defence={1,2};model.magic_defence={3,4};
        controller.pointer(634,570,true);
        check(model.stat(0)==std::array{1502,3,4},"stat switch on press");
        controller.pointer(634,570,false);
        check(model.stat(0)==std::array{1502,3,4},"release does not switch stat twice");
        check(!model.press_stat(664,570) && !model.press_stat(634,590),"stat half-open edges");
        mir3::HudButtonTooltip hint;
        check(hint.draw("inventory",{31,12},{790,540})==std::array{757,533,43,7},"tooltip right edge and first animation step");
        check(hint.draw("changed",{31,12},{790,540})[3]==8,"tooltip caption change preserves animation");
        check(hint.draw("short",{31,3},{790,540})[3]==7 && !hint.expanding && hint.height==6,
            "shorter tooltip finishes animation within measured height");
        mir3::ChatReceptionButtons reception;
        check(reception.hints==std::array{2637,2639,2643},"initial reception hints");
        const auto original=reception;
        check(reception.press(19,false)==-1 && reception.press(23,true)==-1 && reception.hints==original.hints,"invalid reception button");
        for (int i=0;i<3;++i) {
            const int command=i==0?1070:i==1?1071:1073;
            check(reception.press(20+i,false)==command && reception.frames[i]==1338+2*i && reception.hints[i]==original.hints[i]+1,"predict reject command");
            check(reception.press(20+i,true)==command && reception.frames[i]==-1 && reception.hints[i]==original.hints[i],"predict allow command");
            check(!messages.get(command).empty(),"reception command resource");
        }
        check(messages.get(1073).back()==' ',"preserve trailing space in original all-chat command");
        mir3::InventoryGrid trade_bag;
        mir3::BagItem remainder;
        remainder.icon=301;
        remainder.make_index=98766;
        remainder.name="Trade stack";
        remainder.stack_mode=3;
        remainder.count=3;
        check(trade_bag.insert(remainder),"insert stack remainder");
        auto returned=remainder;
        returned.count=2;
        check(trade_bag.restore_trade_item(returned) && trade_bag.items[0].count==5 &&
              !trade_bag.items[1],"trade return merges with matching remainder");
        returned.make_index++;
        check(trade_bag.restore_trade_item(returned) && trade_bag.items[1].count==2,
              "trade return keeps different item identity separate");
        mir3::InventoryGrid trade_destination;
        check(trade_destination.restore_trade_item(returned,10,185,76) &&
              trade_destination.item_at(10)==0,"trade return uses requested bag slot");
        std::cout<<"HUD resources, hit regions, release behavior and crop precision passed\n";
    } catch (const std::exception& error) { std::cerr<<error.what()<<'\n';return 1; }
}

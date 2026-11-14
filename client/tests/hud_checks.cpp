#include "inventory/inventory_grid.hpp"
#include "hud/hud_model.hpp"
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

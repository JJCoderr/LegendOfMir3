#include "inventory/inventory_grid.hpp"
#include "hud/hud_model.hpp"
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

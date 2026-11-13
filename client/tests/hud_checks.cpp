#include "hud/hud_model.hpp"
#include "resources/wil_archive.hpp"
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
        check(archive.open(std::filesystem::path(argv[1])/"Data"/"GameInter"),"archive open");
        const auto skin=archive.frame(1160);
        check(skin && skin->width==800 && skin->height==134,"original HUD frame dimensions");
        check(!archive.frame(-1) && !archive.frame(0x7fffffff),"invalid frame bounds");
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
        std::cout<<"HUD resources, hit regions, release behavior and crop precision passed\n";
    } catch (const std::exception& error) { std::cerr<<error.what()<<'\n';return 1; }
}

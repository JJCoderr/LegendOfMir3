#pragma once
#include <cstdint>
#include <string_view>
#include <array>
namespace mir3 {
struct LoginAudio {
    enum class Page { none,credentials,selection,creation };
    Page pending=Page::none;
    std::uint64_t elapsed=0;
    void enter(Page page) { pending=page;elapsed=0; }
    std::string_view advance(std::uint32_t ms) {
        if (pending==Page::none) return {};
        elapsed+=ms;
        if (elapsed<=(pending==Page::credentials?1200u:1000u)) return {};
        const auto page=pending;pending=Page::none;elapsed=0;
        return page==Page::credentials?"Opening.wav":page==Page::selection?"SelChr.mp3":"CreateChr.mp3";
    }
    static std::string_view voice(bool creation,int profession,int gender) {
        if (profession<0 || profession>2 || gender<0 || gender>1) return {};
        constexpr std::array<std::string_view,6> create{"JMCre.wav","SMCre.wav","DMCre.wav","JWCre.wav","SWCre.wav","DWCre.wav"};
        constexpr std::array<std::string_view,6> select{"JMMSel.wav","SMMSel.wav","DMMSel.wav","JWMSel.wav","SWMSel.wav","DWMSel.wav"};
        return (creation?create:select)[profession+3*gender];
    }
};
}

#pragma once
#include "protocol/receive_message.hpp"
#include <algorithm>
#include <array>
#include <cstdlib>
#include <cerrno>
#include <string_view>
namespace mir3 {
struct SelectionRole {
    std::string name;
    std::uint8_t profession=0,gender=0,level=0,action=0;
    int x=0,y=0,frame=0;
    std::uint16_t elapsed=0;
    bool occupied=false;
    int animation_index() const { return action+5*(gender+2*profession); }
    int first_frame() const { return 200+animation_index()*60; }
    int last_frame() const {
        static constexpr std::array<int,30> lengths{10,19,11,7,17,10,10,10,11,15,10,11,10,10,14,10,17,10,8,16,10,16,19,11,16,10,16,10,9,14};
        return first_frame()+lengths[animation_index()];
    }
    void start(std::uint8_t value) { action=value;frame=first_frame();elapsed=0; }
    void advance(std::uint32_t ms) {
        elapsed=std::uint16_t(elapsed+ms);
        if (elapsed<=120) return;
        elapsed=0;
        if (++frame<=last_frame()) return;
        if (action==1) start(2);
        else if (action==3) start(0);
        else frame=first_frame();
    }
};
struct CharacterSelectionPage {
    bool visible=false,connecting=true;
    static constexpr int width=640,height=480;
    std::array<SelectionRole,4> roles{};
    int count=0,selected=-1,page=0;
    // 名称保持原始 ANSI 字节，界面显示时再转换为 UTF-8。
    bool receive_list(std::uint32_t announced,std::string_view body) {
        if (announced>INT32_MAX || body.size()>255 || body.find(char(0))!=std::string_view::npos) return false;
        auto next=*this;next.roles={};next.count=int(std::min(announced,4u));next.selected=-1;next.page=0;
        for (int i=0;i<next.count;++i) {
            if (body.starts_with('*')) body.remove_prefix(1);
            std::array<std::string,5> fields;
            for (int f=0;f<5;++f) {
                const auto end=body.find('/');
                if (end==std::string_view::npos && (f!=4 || i+1!=next.count)) return false;
                fields[f]=body.substr(0,end);
                body=end==std::string_view::npos?std::string_view{}:body.substr(end+1);
            }
            if (fields[0].empty() || fields[0].size()>24) return false;
            auto& role=next.roles[i];
            role.name=std::move(fields[0]);
            // 原版 atoi 后写入 BYTE；先限制输入整数，避免宿主 atoi 溢出。
            const auto byte=[](const std::string& field)->std::optional<std::uint8_t> {
                char* end=nullptr;
                errno=0;const auto value=std::strtol(field.c_str(),&end,10);
                if (errno==ERANGE || value<INT32_MIN || value>INT32_MAX) return {};
                return std::uint8_t(value);
            };
            const auto job=byte(fields[1]),level=byte(fields[3]),gender=byte(fields[4]);
            if (!job || !level || !gender || *job>2 || *gender>1) return false;
            role.profession=*job;role.gender=*gender;role.level=*level;role.occupied=true;
            role.x=next.count==1?300:(i%2?350:250);role.y=i%2?250:210;
            role.start(i%2?0:1);
        }
        next.selected=next.count?0:-1;next.connecting=false;*this=std::move(next);return true;
    }
    bool receive(std::span<const std::uint8_t> message) {
        if (message.size()<16) return false;
        const auto header=decode_six_bit(message.data(),16);
        const auto body=decode_six_bit(message.data()+16,message.size()-16);
        if (!header || header->size()!=12 || !body) return false;
        const auto b=[&](int i) { return std::uint32_t(std::uint8_t((*header)[i])); };
        if ((b(4)|(b(5)<<8))!=520) return false;
        return receive_list(b(0)|(b(1)<<8)|(b(2)<<16)|(b(3)<<24),*body);
    }
    bool select(int index) {
        if (connecting || index<page*2 || index>=page*2+2 || index>=count || index==selected) return false;
        roles[index].start(1);
        const int other=index^1;
        if (other<count) roles[other].start(3);
        selected=index;return true;
    }
    bool turn_page(int target) {
        if (connecting || count<=2 || target<0 || target>1 || target==page) return false;
        if (selected>=0) roles[selected].start(0);
        page=target;selected=2*page;roles[selected].start(1);return true;
    }
    void advance(std::uint32_t ms) {
        if (connecting) return;
        for (int i=page*2;i<std::min(count,page*2+2);++i) roles[i].advance(ms);
    }
};
}

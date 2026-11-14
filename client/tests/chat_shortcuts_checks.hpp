// 测试内容：收件人与短语独立选中项、数字越界保留、上下键边界、空列表和私聊前缀更新。
#pragma once
#include "chat/chat_submission.hpp"
#include <stdexcept>
inline void chat_shortcuts_checks() {
    mir3::ChatSubmission state;state.contacts={"One ","Two "};state.phrases={"first","second","third"};
    auto check=[](bool value){if(!value) throw std::runtime_error("chat shortcut selection");};
    check(state.select(false,1,false)=="/Two " && state.private_prefix=="/Two ");
    check(state.select(false,9,false)=="/Two " && state.contact_selected==1);
    check(state.select(false,-1,true)=="/One ");check(state.select(false,-1,true)=="/One ");
    check(state.select(true,2,false)=="third" && state.contact_selected==0);
    check(state.select(true,1,true)=="third");check(state.select(true,-1,true)=="second");
    state.clear_history();check(!state.select(false,1,true) && state.contact_selected==-1);
    check(!state.select(false,-1,true) && state.contact_selected==0);
    state.contacts={"New "};check(state.select(false,9,false)=="/New ");
}

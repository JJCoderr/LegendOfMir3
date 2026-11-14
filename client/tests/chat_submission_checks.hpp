// 测试内容：聊天发送分支、私聊前缀与去重上限、快捷短语截取、本地标志、行会权限和骑马延迟。
#pragma once
#include "chat/chat_submission.hpp"
#include <stdexcept>
inline void chat_submission_checks() {
    mir3::ChatSubmission model;
    auto submit=[&](std::string_view value,bool manager=false){return model.submit(value,manager,"add","mount","unmount","recall");};
    auto require=[](bool value,const char* text){if(!value) throw std::runtime_error(text);};
    require(!submit("").send && submit("hello").send,"ordinary dispatch");
    require(submit("add").dialog==1427 && submit("add",true).dialog==1426,"guild rights");
    require(submit("@chatsave").send && model.save_chat && submit("@snow").send && model.snow,"local flags still send");
    submit("@chatsave");require(!model.save_chat,"toggle exact command");
    auto result=submit("/Bob hello");require(result.send && result.private_echo && model.private_prefix=="/Bob " && model.contacts[0]=="Bob ","private prefix");
    require(!submit("/Bob another").contacts_changed && model.contacts.size()==1,"repeat does not reorder history");
    submit("/");require(model.private_prefix=="/Bob " && model.contacts.size()==1,"empty recipient keeps prefix");
    require(!submit("#hello").send && model.phrases[0]=="hello","phrase local only");
    require(!submit("#hello").phrases_changed,"phrase duplicate");
    for(int i=0;i<12;++i) submit("#line"+std::to_string(i));
    require(model.phrases.size()==10 && model.phrases.front()=="line2" && model.phrases.back()=="line11","evict oldest beyond ten");
    require(submit("mount").delay && submit("unmount").delay && submit("recall").delay && !submit("mount ").delay,"exact resource commands");
}

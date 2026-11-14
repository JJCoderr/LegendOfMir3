#pragma once
#include "hud/action_prediction.hpp"
#include <stdexcept>

// 测试内容：文本前缀、确认不回滚、拒绝恢复、死亡忽略、8000毫秒严格边界和生命周期清理。
inline void action_reply_checks() {
    using namespace mir3;
    const auto require=[](bool ok,const char* message) {if(!ok) throw std::runtime_error(message);};
    require(parse_action_reply("+GOOD/1234")==ActionReply::good,"confirmation prefix");
    require(parse_action_reply("+FAIL/0")==ActionReply::fail,"rejection prefix");
    require(parse_action_reply("+GOODextra")==ActionReply::good,"original prefix semantics");
    require(parse_action_reply("GOOD")==ActionReply::unknown && parse_action_reply("+FAI")==ActionReply::unknown,"unknown control");
    ActionPrediction state;
    require(!state.reject(false),"no origin before action");
    state.begin({12,34},6);state.advance(8000);
    require(state.waiting,"exact timeout still pending");
    state.advance(1);
    require(!state.waiting && state.saved->cell==std::array<int,2>{12,34},"timeout releases without rollback");
    state.begin({13,35},2);state.confirm();
    require(!state.waiting && state.saved->cell==std::array<int,2>{13,35},"good only clears wait");
    state.begin({14,36},4);
    require(!state.reject(true) && state.waiting,"dead actor ignores rollback");
    const auto restored=state.reject(false);
    require(restored && restored->cell==std::array<int,2>{14,36} && restored->facing==4 && !state.waiting,"rollback origin and direction");
    state={};require(!state.saved && !state.waiting && !state.reject(false),"scene reset removes stale origin");
}

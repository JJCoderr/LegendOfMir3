#define PICOJSON_USE_INT64
#include "../Server/Def/picojson.h"
#include <deque>
#include "StdAfx.h"
#include "ClientRpc.h"
#include "../Server/Def/JsonRpc.h"
#include "../Shared/DebugState.h"
#include "Tools/ItemDebugState.h"
namespace mir3 {
namespace {
using Json = picojson::value;
using Object = picojson::object;
using Array = picojson::array;
uint64_t eventSequence = 0;
bool enabled = false;
std::map<std::pair<std::string,int>,uint64_t> packetCounts;
std::string lastState;
std::deque<Json> events;
void Record(const std::string& kind, const Json& data) {
    events.emplace_back(Object{{"seq",Json(int64_t(++eventSequence))},{"kind",Json(kind)},{"data",data}});
    constexpr size_t EventCapacity = 256;
    if (events.size() > EventCapacity) events.pop_front();
}
Json Parse(const std::string& text) {
    Json value;
    if (!picojson::parse(value,text).empty()) throw JsonRpc::Error{-32602,"Invalid params"};
    return value;
}
std::string Text(const Json& params, const char* key) {
    if (!params.get(key).is<std::string>()) throw JsonRpc::Error{-32602,"Missing string parameter"};
    const auto utf8 = params.get(key).get<std::string>();
    if (utf8.find('\0') != std::string::npos) throw JsonRpc::Error{-32602,"Embedded NUL"};
    int count = MultiByteToWideChar(CP_UTF8,MB_ERR_INVALID_CHARS,utf8.data(),int(utf8.size()),nullptr,0);
    if (!count) throw JsonRpc::Error{-32602,"Empty or invalid text"};
    std::wstring wide(count,L'\0');MultiByteToWideChar(CP_UTF8,0,utf8.data(),int(utf8.size()),&wide[0],count);
    BOOL substituted=FALSE;count=WideCharToMultiByte(CP_ACP,WC_NO_BEST_FIT_CHARS,wide.data(),int(wide.size()),nullptr,0,nullptr,&substituted);
    std::string local(count,'\0');WideCharToMultiByte(CP_ACP,WC_NO_BEST_FIT_CHARS,wide.data(),int(wide.size()),&local[0],count,nullptr,&substituted);
    if (substituted) throw JsonRpc::Error{-32602,"Text not representable in client encoding"};
    return local;
}
int Integer(const Json& params,const char* key) {
    const auto value=params.get(key);
    if(!value.is<int64_t>() || value.get<int64_t>()<0 || value.get<int64_t>()>INT_MAX)
        throw JsonRpc::Error{-32602,"Expected nonnegative integer"};
    return int(value.get<int64_t>());
}
bool ItemAction(const std::string& method,const Json& params) {
    auto& ui=g_xGameProc.m_xInterface;auto& bag=ui.m_xInventoryWnd;
    auto& status=ui.m_xStatusWnd;auto& held=ui.m_stCommonItem;
    if(g_xGameProc.InputDelayed() || g_xGameProc.m_xNotice.IsActive() || held.bSetted ||
        status.m_equipmentRequest || bag.m_usePending || bag.m_pendingDrop.bSetted) return false;
    if(method=="item.pickup") {
        g_xClientSocket.SendPickUp(g_xGameProc.m_xMyHero.m_wPosX,g_xGameProc.m_xMyHero.m_wPosY);
        return true;
    }
    if(method=="item.unequip") {
        const int slot=Integer(params,"slot");
        if(slot>=_EQUIP_MAX_CELL || !status.m_stEquipItem[slot].bSetted) return false;
        status.SetCommonFromEquip(slot,&held);
        if(status.RequestTakeOff()) return true;
        status.SetEquipFromCommon(slot,&held);return false;
    }
    const int id=Integer(params,"id");
    const int slot=method=="item.equip"?Integer(params,"slot"):-1;
    for(int i=0;i<_MAX_INVEN_ITEM;++i) {
        const auto& item=bag.m_stInventoryItem[i];
        if(!item.bSetted || item.xItem.m_stItemInfo.nMakeIndex!=id) continue;
        if(method=="item.use" && !bag.UseAllowed(item.xItem,GetTickCount()-bag.m_lastUse)) return false;
        bag.SetCommonFromInven(i,&held);
        bool sent=false;
        if(method=="item.equip") sent=status.RequestTakeOn(slot);
        else if(method=="item.use") sent=bag.RequestUse(GetTickCount());
        else if(method=="item.drop") {bag.RequestDrop();sent=bag.m_pendingDrop.bSetted!=FALSE;}
        if(!sent && held.bSetted && bag.AddInvenItem(held.xItem,i,TRUE)) held=COMMONITEMSET{};
        return sent;
    }
    return false;
}
}

std::string ClientState() {

    const char* scene = "unknown";
    switch (g_bProcState) {
    case _SVR_SEL_PROC: scene = "server_select"; break;
    case _LOGIN_PROC: scene = "login"; break;
    case _CHAR_SEL_PROC: scene = "character_select"; break;
    case _GAME_PROC: scene = "world"; break;
    }
    sockaddr_in peer{};
    int size = sizeof(peer);
    const bool connected = getpeername(g_xClientSocket.m_sockClient,
        reinterpret_cast<sockaddr*>(&peer), &size) == 0;
    auto state = std::string(",\"scene\":") + mir3::QuoteLocalText(scene)
        + ",\"connected\":" + mir3::BoolJson(connected);
    RECT bounds{};
    GetClientRect(g_xMainWnd.GetSafehWnd(), &bounds);
    state += ",\"width\":" + std::to_string(bounds.right)
        + ",\"height\":" + std::to_string(bounds.bottom);
    if (g_bProcState == _LOGIN_PROC) {
        state += ",\"stage\":" + mir3::QuoteLocalText(g_xLoginProc.DebugStage())
            + ",\"notice\":" + mir3::BoolJson(g_xLoginProc.HasNotice());
    } else if (g_bProcState == _CHAR_SEL_PROC && g_xMainWnd.m_pxDefProcess == &g_xChrSelProc) {
        state += ",\"waiting\":" + std::string(mir3::BoolJson(g_xChrSelProc.IsWaiting()))
            + ",\"notice\":" + mir3::BoolJson(g_xChrSelProc.HasNotice());
    }
    if (connected) state += ",\"remote_port\":" + std::to_string(ntohs(peer.sin_port));
    if (g_xMainWnd.m_pxDefProcess == &g_xGameProc && g_xGameProc.m_bRender) {
        auto& game = g_xGameProc;
        const auto& hero = game.m_xMyHero;
        state += ",\"notice\":" + std::string(mir3::BoolJson(g_xGameProc.m_xNotice.IsActive()))
            + ",\"input_delayed\":" + mir3::BoolJson(game.InputDelayed());
        state += ",\"map\":" + mir3::QuoteLocalText(game.m_szMapName)
            + ",\"x\":" + std::to_string(hero.m_wPosX)
            + ",\"y\":" + std::to_string(hero.m_wPosY)
            + ",\"hp\":" + std::to_string(hero.m_stAbility.wHP)
            + ",\"mp\":" + std::to_string(hero.m_stAbility.wMP);
        state += mir3::ItemDebugState(game);
    }
    return state;

}
void ClientRpcTick() {
    enabled = true;
    const auto state = std::string("{\"schema\":1") + ClientState() + "}";
    if (state != lastState) { lastState=state; Record("state",Parse(state)); }
}
void ClientRpcPacket(const char* direction,int opcode,int bytes) {
    if(!enabled) return;
    ++packetCounts[{direction,opcode}];
    Record(std::string("packet.")+direction,Json(Object{{"opcode",Json(double(opcode))},{"bytes",Json(double(bytes))}}));
}
std::string ClientRpc(const std::string& method, const std::string& arguments) {
    ClientRpcTick();
    if (method == "state.get") return lastState;
    if (method == "protocol.get") {
        Array observed;
        for(const auto& entry:packetCounts) observed.emplace_back(Object{
            {"direction",Json(entry.first.first)},{"opcode",Json(double(entry.first.second))},{"count",Json(int64_t(entry.second))}});
        return Json(Object{{"observed",Json(observed)}}).serialize();
    }
    const auto params=Parse(arguments);
    if (!params.is<Object>()) throw JsonRpc::Error{-32602,"Expected named parameters"};
    if (method == "events.wait") {
        if (!params.get("after").is<int64_t>() || params.get("after").get<int64_t>() < 0)
            throw JsonRpc::Error{-32602,"after must be a nonnegative integer"};
        const auto after=uint64_t(params.get("after").get<int64_t>());
        if (after > eventSequence) throw JsonRpc::Error{-32602,"Event sequence is in the future"};
        if (!events.empty() && after+1 < uint64_t(events.front().get("seq").get<int64_t>()))
            throw JsonRpc::Error{-32003,"Event history expired; query current state"};
        Array found;
        for (const auto& event:events) if (uint64_t(event.get("seq").get<int64_t>()) > after) found.push_back(event);
        if (found.empty()) {
            const auto timeout=params.get("timeout_ms");
            if (!timeout.is<int64_t>() || timeout.get<int64_t>()<0 || timeout.get<int64_t>()>30000)
                throw JsonRpc::Error{-32602,"timeout_ms must be 0..30000"};
            throw JsonRpc::Pending{DWORD(timeout.get<int64_t>())};
        }
        return Json(Object{{"sequence",Json(int64_t(eventSequence))},{"events",Json(found)}}).serialize();
    }
    bool accepted=false;
    if (method == "session.login" && g_bProcState == _LOGIN_PROC)
        accepted=g_xLoginProc.SubmitCredentials(Text(params,"account"),Text(params,"password"));
    else if (method == "session.select_server" && g_bProcState == _LOGIN_PROC)
        accepted=g_xLoginProc.SelectServer(Text(params,"name"));
    else if (method == "session.enter" && g_bProcState == _CHAR_SEL_PROC)
        accepted=g_xChrSelProc.EnterCharacter(Text(params,"name"));
    else if (method == "notice.dismiss" && g_bProcState == _LOGIN_PROC) {
        accepted=g_xLoginProc.HasNotice();
        if (accepted) g_xLoginProc.DismissNotice();
    } else if (method == "notice.dismiss" && g_bProcState == _GAME_PROC) {
        accepted=true;
        if(g_xGameProc.m_xNotice.IsActive()) g_xGameProc.m_xNotice.HideMessageBox();
    } else if (method == "session.return" && g_bProcState == _GAME_PROC) {
        accepted=g_xGameProc.ConfirmExit(ExitFlow::CharacterDialog,GetTickCount());
    } else if (g_bProcState == _GAME_PROC && (method=="item.equip" || method=="item.unequip" ||
        method=="item.use" || method=="item.drop" || method=="item.pickup")) accepted=ItemAction(method,params);
    else throw JsonRpc::Error{-32601,"Method unavailable in this scene"};
    if (!accepted) throw JsonRpc::Error{-32002,"Action rejected by client state"};
    Record("submitted",Json(Object{{"method",Json(method)}}));
    return Json(Object{{"status",Json(std::string("submitted"))},{"sequence",Json(int64_t(eventSequence))}}).serialize();
}
}

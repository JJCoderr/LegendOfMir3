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
            + ",\"notice\":" + mir3::BoolJson(g_xChrSelProc.HasNotice())
            + ",\"creation_result\":" + mir3::QuoteLocalText(g_xChrSelProc.CreationResult())
            + ",\"deletion_result\":" + mir3::QuoteLocalText(g_xChrSelProc.DeletionResult());
        const auto roster=g_xChrSelProc.Roster();Array characters;
        for(int i=0;i<roster.count;++i) {
            const auto& role=roster.roles[i];
            characters.emplace_back(Object{{"name",Parse(mir3::QuoteLocalText(role.ChrName))},
                {"job",Json(int64_t(role.Class))},{"gender",Json(int64_t(role.Gender))},{"level",Json(int64_t(role.Level))}});
        }
        state += ",\"characters\":" + Json(characters).serialize();
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
            + ",\"mp\":" + std::to_string(hero.m_stAbility.wMP)
            + ",\"gold\":" + std::to_string(hero.m_nGlod)
            + ",\"level\":" + std::to_string(hero.m_stAbility.bLevel)
            + ",\"experience\":" + std::to_string(hero.m_stAbility.dwExp)
            + ",\"next_experience\":" + std::to_string(hero.m_stAbility.dwMaxExp);
        Array actors;
        game.m_xActorList.MoveCurrentToTop();
        for(int i=0;i<game.m_xActorList.GetCounter();++i) {
            const auto* actor=game.m_xActorList.GetCurrentData();game.m_xActorList.MoveNextNode();
            actors.emplace_back(Object{{"id",Json(int64_t(actor->m_dwIdentity))},
                {"name",Parse(mir3::QuoteLocalText(actor->m_szName))},
                {"x",Json(int64_t(actor->m_wPosX))},{"y",Json(int64_t(actor->m_wPosY))},
                {"npc",Json(actor->m_stFeature.bGender==_GENDER_NPC)},
                {"dead",Json(bool(actor->m_bIsDead))},{"hp",Json(int64_t(actor->m_wHP))},
                {"max_hp",Json(int64_t(actor->m_wMAXHP))}});
        }
        state += ",\"actors\":"+Json(actors).serialize()+",\"actor_count\":"+std::to_string(actors.size());
        state += std::string(",\"return_ready\":")+mir3::BoolJson(!game.m_exitTransition.active &&
            game.m_exitGuard.Allowed(GetTickCount(),hero.m_bCurrMtn));
        state += mir3::ItemDebugState(game);
        auto& npc=game.m_xInterface.m_xNPCWnd;
        Array npcRows;
        for(const auto& row:npc.rows) npcRows.emplace_back(Object{
            {"text",Parse(mir3::QuoteLocalText(row.text.c_str()))},
            {"command",Parse(mir3::QuoteLocalText(row.command.c_str()))},
            {"option",Json(int64_t(row.option))}});
        state += ",\"npc_dialog\":"+Json(Object{{"open",Json(bool(npc.GetGameWndActive()))},
            {"id",Json(int64_t(npc.m_nNpcId))},{"rows",Json(npcRows)},
            {"ready",Json(!npc.lastAction || GetTickCount()-npc.lastAction>1000)}}).serialize();
        auto& shop=game.m_xInterface.m_xStoreWnd;
        Array goods;
        for(const auto& good:shop.goods) goods.emplace_back(Object{
            {"name",Parse(mir3::QuoteLocalText(good.name.c_str()))},
            {"price",Json(int64_t(good.price))},{"stock",Json(int64_t(good.stock))}});
        state += ",\"shop\":"+Json(Object{{"open",Json(bool(shop.GetGameWndActive()))},
            {"actor",Json(int64_t(shop.actor))},{"goods",Json(goods)},
            {"notice",Json(game.m_xInterface.m_xMsgBox.IsActive() && game.m_xInterface.m_nIdxMsgBox==0)}}).serialize();
        Array rejected;
        for(bool value:game.m_xInterface.m_chatRejected) rejected.emplace_back(value);
        state += ",\"chat_rejected\":"+Json(rejected).serialize();
        const auto& chat=game.m_xInterface.m_chatHistory;
        state += ",\"chat_count\":"+std::to_string(chat.size())+
            ",\"chat_last\":"+(chat.empty()?"null":mir3::QuoteLocalText(chat.back().text.c_str()));
        state += ",\"spell_reply_id\":"+std::to_string(hero.m_lastSpellReplyId)
            +",\"cast_ready\":"+mir3::BoolJson(!game.InputDelayed() && !hero.m_pendingAction &&
                !hero.m_bMotionLock && !hero.m_bInputLock && hero.m_spellTiming.key_ready(GetTickCount()));
        const auto& window=game.m_xInterface.m_xSkillWnd;Array skills,shortcuts;
        for(int i=0;i<window.m_bMyMagicCnt;++i) {
            const auto& skill=window.m_pstMyMagic[i];
            skills.emplace_back(Object{{"id",Json(int64_t(skill.stStdMagic.wMagicID))},
                {"name",Parse(mir3::QuoteLocalText(skill.stStdMagic.szMagicName))},
                {"level",Json(int64_t(skill.bLevel))},{"training",Json(int64_t(skill.nCurrTrain))},
                {"key",Json(int64_t(CSkillWnd::KeyNumber(BYTE(skill.bUseKey))))}});
        }
        for(auto skill:window.m_shortcuts) shortcuts.emplace_back(int64_t(skill));
        state += ",\"skills\":"+Json(skills).serialize()+",\"skill_shortcuts\":"+Json(shortcuts).serialize();
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
    else if (method == "character.delete" && g_bProcState == _CHAR_SEL_PROC)
        accepted=g_xChrSelProc.DeleteCharacter(Text(params,"name"),Text(params,"password"));
    else if (method == "character.create" && g_bProcState == _CHAR_SEL_PROC)
        accepted=g_xChrSelProc.CreateCharacter(Text(params,"name"),Integer(params,"job"),Integer(params,"gender"));
    else if (method == "notice.dismiss" && g_bProcState == _CHAR_SEL_PROC)
        accepted=g_xChrSelProc.DismissNotice();
    else if (method == "notice.dismiss" && g_bProcState == _LOGIN_PROC) {
        accepted=g_xLoginProc.HasNotice();
        if (accepted) g_xLoginProc.DismissNotice();
    } else if (method == "notice.dismiss" && g_bProcState == _GAME_PROC) {
        accepted=true;
        if(g_xGameProc.m_xNotice.IsActive()) g_xGameProc.m_xNotice.HideMessageBox();
        auto& ui=g_xGameProc.m_xInterface;
        if(ui.m_xMsgBox.IsActive() && ui.m_nIdxMsgBox==0) ui.FinishNotice(CLICK_OK,false);
    } else if (method == "chat.toggle_reception" && g_bProcState == _GAME_PROC) {
        if(!g_xGameProc.InputDelayed() && !g_xGameProc.m_xNotice.IsActive() && !g_xGameProc.m_xInterface.HasInputDialog())
            accepted=g_xGameProc.m_xInterface.ToggleChatReception(Integer(params,"channel"));
    } else if (method == "chat.send" && g_bProcState == _GAME_PROC) {
        const auto text=Text(params,"text");
        constexpr size_t MaximumChatInputBytes=255; // Server's legacy decoded chat buffer, excluding NUL.
        if(text.size()>MaximumChatInputBytes || text.find_first_of("\r\n")!=std::string::npos)
            throw JsonRpc::Error{-32602,"Chat must be a single line of at most 255 local bytes"};
        if(!g_xGameProc.InputDelayed() && !g_xGameProc.m_xNotice.IsActive() && !g_xGameProc.m_xInterface.HasInputDialog()) {
            memcpy(g_xChatEditBox.m_szInputMsg,text.c_str(),text.size()+1);
            g_xGameProc.OnMsgInputted(text[0]=='@',0);accepted=true;
        }
    } else if (method == "shop.buy" && g_bProcState == _GAME_PROC) {
        const int index=Integer(params,"index"),quantity=Integer(params,"quantity");
        if(quantity<1 || quantity>USHRT_MAX) throw JsonRpc::Error{-32602,"Invalid purchase quantity"};
        auto& shop=g_xGameProc.m_xInterface.m_xStoreWnd;
        if(shop.GetGameWndActive() && !shop.storage && index<int(shop.goods.size()) && shop.goods[index].variants==CStoreWnd::BulkCategory &&
            !g_xGameProc.m_xInterface.m_xMsgBox.IsActive() && !g_xGameProc.InputDelayed() && !g_xGameProc.m_xNotice.IsActive() && !g_xGameProc.m_xInterface.HasInputDialog()) {
            shop.highlighted=index;shop.OpenCategory();
            if(shop.bulk>=0) { shop.BuyBulk(std::to_string(quantity).c_str());accepted=true; }
        }
    } else if (method == "npc.click" && g_bProcState == _GAME_PROC) {
        const int index=Integer(params,"index");auto& actors=g_xGameProc.m_xActorList;
        if(index<actors.GetCounter()) {
            actors.MoveCurrentToTop();for(int i=0;i<index;++i) actors.MoveNextNode();
            accepted=g_xGameProc.ClickNPC(actors.GetCurrentData());
        }
    } else if (method == "npc.select" && g_bProcState == _GAME_PROC) {
        if(!g_xGameProc.InputDelayed() && !g_xGameProc.m_xNotice.IsActive() && !g_xGameProc.m_xInterface.HasInputDialog())
            accepted=g_xGameProc.m_xInterface.m_xNPCWnd.SelectOption(Integer(params,"option"));
    } else if (method == "actor.query_name" && g_bProcState == _GAME_PROC) {
        const int index=Integer(params,"index");
        auto& actors=g_xGameProc.m_xActorList;
        if(index<actors.GetCounter()) {
            actors.MoveCurrentToTop();
            for(int i=0;i<index;++i) actors.MoveNextNode();
            const auto* actor=actors.GetCurrentData();
            g_xClientSocket.SendQueryName(actor->m_dwIdentity,actor->m_wPosX,actor->m_wPosY);
            accepted=true;
        }
    } else if (method == "world.walk" && g_bProcState == _GAME_PROC) {
        const int x=Integer(params,"x"),y=Integer(params,"y");
        if(x<=USHRT_MAX && y<=USHRT_MAX && !g_xGameProc.m_xNotice.IsActive())
            accepted=g_xGameProc.m_xMyHero.WalkAdjacent(POINT{x,y});
    } else if (method == "skill.cast" && g_bProcState == _GAME_PROC) {
        const int key=Integer(params,"key");
        if(params.contains("target")) {
            const int index=Integer(params,"target");auto& actors=g_xGameProc.m_xActorList;
            if(index<actors.GetCounter() && !g_xGameProc.m_xNotice.IsActive()) {
                actors.MoveCurrentToTop();for(int i=0;i<index;++i) actors.MoveNextNode();
                const auto* actor=actors.GetCurrentData();
                if(!actor->m_bIsDead) accepted=g_xGameProc.m_xMyHero.CastShortcut(key,
                    POINT{actor->m_wPosX,actor->m_wPosY},actor->m_dwIdentity,actor->m_stFeature);
            }
        } else {
            const int x=Integer(params,"x"),y=Integer(params,"y");
            if(x<=USHRT_MAX && y<=USHRT_MAX && !g_xGameProc.m_xNotice.IsActive())
                accepted=g_xGameProc.m_xMyHero.CastShortcut(key,POINT{x,y},0,FEATURE{});
        }
    } else if (method == "skill.bind" && g_bProcState == _GAME_PROC) {
        const int id=Integer(params,"id"),key=Integer(params,"key");
        auto& window=g_xGameProc.m_xInterface.m_xSkillWnd;
        if(key<=SkillWire::ShortcutCount) for(int i=0;i<window.m_bMyMagicCnt;++i)
            if(window.m_pstMyMagic[i].stStdMagic.wMagicID==id) {
                window.BindMagic(id,key?CSkillWnd::LegacyKey(key):0);accepted=true;break;
            }
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

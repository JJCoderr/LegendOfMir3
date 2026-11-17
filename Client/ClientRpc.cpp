#include "StdAfx.h"
#include "ClientRpc.h"
#include "../Server/Def/JsonRpc.h"
#include "../Shared/DebugState.h"
#include "Tools/ItemDebugState.h"
namespace mir3 {
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
                const auto& game = g_xGameProc;
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
std::string ClientRpc(const std::string& method, const std::string&) {
    if (method == "state.get") return std::string("{\"schema\":1") + ClientState() + "}";
    throw JsonRpc::Error{-32601, "Method not found"};
}
}

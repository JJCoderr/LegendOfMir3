#pragma once

#define NOMINMAX
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>

#include "cui.h"
#include "CuiAudio.h"
#include "CuiCharacterStatus.h"
#include "CuiChat.h"
#include "CuiEquipment.h"
#include "CuiGuild.h"
#include "CuiInventory.h"
#include "CuiInventoryDimensions.h"
#include "CuiInventoryLayout.h"
#include "CuiMapTerrain.h"
#include "CuiLegacy/hud/ground_items.hpp"
#include "CuiLegacy/hud/casting.hpp"
#include "CuiLegacy/hud/local_action_clock.hpp"
#include "CuiLegacy/hud/melee_cooldown.hpp"
#include "CuiLegacy/magic/melee_spells.hpp"
#include "CuiLegacy/hud/level_up_effect.hpp"
#include "CuiLegacy/magic/spell_color.hpp"
#include "CuiLegacy/magic/spell_effects.hpp"
#include "CuiLegacy/magic/spell_sound_voices.hpp"
#include "CuiLegacy/magic/spell_timing.hpp"
#include "CuiLegacy/hud/sound_position.hpp"
#include "CuiLegacy/resources/sound_list.hpp"
#include <bit>
#include "CuiLegacy/hud/minimap.hpp"
#include "CuiLegacy/hud/damage_numbers.hpp"
#include "CuiLegacy/hud/monster_motion.hpp"
#include "CuiLegacy/hud/mount_panel.hpp"
#include "CuiLegacy/hud/scene_lighting.hpp"
#include "CuiLegacy/hud/scene_name.hpp"
#include "CuiLegacy/hud/speech_bubble.hpp"
#include "CuiLegacy/hud/snow_weather.hpp"
#include "CuiLegacy/hud/skill_panel.hpp"
#include "CuiLegacy/hud/target_portrait.hpp"
#include "CuiNpcDialogue.h"
#include "CuiNpcShop.h"
#include "CuiParty.h"
#include "CuiQuests.h"
#include "CuiServerOptions.h"
#include "CuiTrade.h"
#include "Common/EnDecode.h"
#include "Common/Protocol.h"

#include <cerrno>
#include <charconv>
#include <cmath>
#include <array>
#include <cstdio>
#include <cstring>
#include <cstdint>
#include <cstdlib>
#include <deque>
#include <fstream>
#include <map>
#include <memory>
#include <random>
#include <string>
#include <vector>
#include <variant>

namespace mir3_client {

enum class Stage {
    Groups, Login, Connecting, SigningIn, Servers, Selecting, Handoff,
    ConnectingCharacters, Characters, Complete, ConnectingGame, AuthenticatingGame,
    GameSession, SigningOutGame, GameDisconnected
};
enum class CharacterDialog { None, Create, Delete };
inline constexpr wchar_t LoginMusic[] = L"CuiAudio/Login.wav";
inline constexpr wchar_t SessionMusic[] = L"CuiAudio/Session.wav";
inline constexpr wchar_t ClickSound[] = L"CuiAudio/Click.wav";
inline constexpr WORD GameLogon = 50;
inline constexpr WORD GameNewMap = 51;
inline constexpr WORD GameDayChanging = 46, GameChangeLight = 654, GameActorLight = 1100;
inline constexpr WORD GameSpell = 17, GameMagicFire = 638, GameMagicFireList = 1410, ClientSpell = 3017;
inline constexpr WORD GameShowMapEvent = 804, GameHideMapEvent = 805;
inline constexpr WORD GameMapDescription = 54;
inline constexpr WORD GameMinimap = 710;
inline constexpr WORD GameMinimapUnavailable = 711;
inline constexpr WORD ClientMinimap = 1033;
inline constexpr WORD GameAddSkill = 210, GameSkillList = 211, GameRemoveSkill = 212;
inline constexpr WORD GameSkillProgress = 640, ClientSkillBinding = 1008;
inline constexpr WORD GameRideHorse = 1300;
inline constexpr WORD GameTradeOpen = 673, GameTradeFailed = 674, GameTradeAddOk = 675, GameTradeAddFail = 676;
inline constexpr WORD GameTradeReturnOk = 677, GameTradeReturnFail = 678, GameTradeCancelled = 681;
inline constexpr WORD GameTradePeerAdd = 682, GameTradePeerRemove = 683, GameTradeGold = 684, GameTradePeerGold = 686;
inline constexpr WORD GameTradeComplete = 687;
inline constexpr WORD ClientTradeRequest = 1025, ClientTradeOffer = 1026, ClientTradeReturn = 1027;
inline constexpr WORD ClientTradeGold = 1029, ClientTradeConfirm = 1030;
inline constexpr WORD GameChangeMap = 634;
inline constexpr WORD GameBackstep = 9;
inline constexpr WORD GameTurn = 10;
inline constexpr WORD GameWalk = 11;
inline constexpr WORD GameCrouch = 12, GameButcher = 637;
inline constexpr WORD GameRun = 13;
inline constexpr WORD GameMonsterAttack = 14, GameMonsterAttackSpecial = 1317;
inline constexpr WORD GameFireHit=8, GameMiningHit=15, GamePowerHit=18, GameLongHit=19,
    GameWideHit=24, GameWideHit2=25, GameRoundHit=1306, GameBackHit=1307,
    GameHumanAction18=1308, GameHumanAttack=1319;
inline constexpr WORD GameMonsterDigUp = 20, GameMonsterRise = 27;
inline constexpr WORD GameActorExit = 29;
inline constexpr WORD GameActorDisappear = 30;
inline constexpr WORD GameActorDeath = 32;
inline constexpr WORD GameMonsterSkeleton = 33;
inline constexpr WORD GameMonsterStatus = 657;
inline constexpr WORD GameHumanStatus801=801, GameHumanStatus=807, GameHumanStatus808=808;
inline constexpr WORD GameActorStruck = 31;
inline constexpr WORD GameActorDead = 34;
inline constexpr WORD GameActorName = 42;
inline constexpr WORD GameActorNameColor = 656;
inline constexpr WORD GameTargetPortrait = 1403;
inline constexpr WORD GameTargetAttributes = 1304, ClientTargetAttributes = 1047;
inline constexpr WORD GameActorStruckWithSource = 1309;
inline constexpr WORD GameClearActors = 633;
inline constexpr WORD GameActorRemove = 803;
inline constexpr WORD GameActorRevive = 1316;
inline constexpr WORD GameActorReviveRefresh = 1321;
inline constexpr WORD GameRush = 6, GameBump = 7;
inline constexpr WORD ClientQueryBag = 81;
inline constexpr WORD GameNpcDialogue = 643;
inline constexpr WORD GameNpcClose = 644;
inline constexpr WORD GameShopGoods = 645;
inline constexpr WORD GameShopBought = 650;
inline constexpr WORD GameShopRejected = 651;
inline constexpr WORD GameShopOffers = 652;
inline constexpr WORD ClientClickNpc = 1010;
inline constexpr WORD ClientNpcSelect = 1011;
inline constexpr WORD ClientBuy = 1014;
inline constexpr WORD ClientShopOffers = 1015;
inline constexpr WORD GameOpenSell = 646;
inline constexpr WORD GameSellQuote = 647;
inline constexpr WORD GameSellOk = 648;
inline constexpr WORD GameSellFail = 649;
inline constexpr WORD GameOpenRepair = 668;
inline constexpr WORD GameRepairOk = 669;
inline constexpr WORD GameRepairFail = 670;
inline constexpr WORD GameRepairQuote = 671;
inline constexpr WORD GameSellStackOk = 1314;
inline constexpr WORD GameSellStackFail = 1315;
inline constexpr WORD ClientQuoteSell = 1012;
inline constexpr WORD ClientSell = 1013;
inline constexpr WORD ClientRepair = 1023;
inline constexpr WORD ClientQuoteRepair = 1024;
inline constexpr WORD GameStorageOpen = 700;
inline constexpr WORD GameStorageDepositOk = 701;
inline constexpr WORD GameStorageFull = 702;
inline constexpr WORD GameStorageDepositFail = 703;
inline constexpr WORD GameStorageList = 704;
inline constexpr WORD GameStorageWithdrawOk = 705;
inline constexpr WORD GameStorageWithdrawFail = 706;
inline constexpr WORD GameStorageBagFull = 707;
inline constexpr WORD ClientStorageDeposit = 1031;
inline constexpr WORD ClientStorageWithdraw = 1032;
inline constexpr WORD GameDoorOpen = 612;
inline constexpr WORD GameDoorLocked = 613;
inline constexpr WORD GameDoorClosed = 614;
inline constexpr WORD GameGroundItemShow = 610, GameGroundItemHide = 611;
inline constexpr WORD ClientPickup = 1000;
inline constexpr WORD ClientOpenDoor = 1002;
inline constexpr WORD ClientWalk = 3011;
inline constexpr WORD ClientCrouch = 3012, ClientButcher = 1007;
inline constexpr WORD ClientRun = 3013;
inline constexpr WORD GameLogout = 1009;
inline constexpr WORD GameDecodeKey = 1330;
inline constexpr WORD GameDecodeKeyResponse = 1150;
inline constexpr WORD GameExperience = 44;
inline constexpr WORD GameLevelUp = 45;
inline constexpr WORD GameAbility = 52;
inline constexpr WORD GameVitals = 53;
inline constexpr WORD GameHealthRegistration = 1312;
inline constexpr WORD GameWeight = 622;
inline constexpr WORD GameSubAbility = 752;

struct Gateway {
    std::string host;
    unsigned port = 0;
};

inline constexpr std::uint64_t MonsterMovementTick = 96;
inline constexpr std::uint64_t MonsterReviveDuration = 600;
inline constexpr std::uint64_t HumanReviveDuration = 600;
inline constexpr std::uint64_t CrouchFrameDuration = 400;
inline constexpr int HumanCorpsePose = 1529;

inline int CrouchPose(std::uint64_t since, std::uint64_t now)
{
    if (!since || now-since>=2*CrouchFrameDuration) return -1;
    return 640+static_cast<int>((now-since)/CrouchFrameDuration);
}

struct LocalScene {
    struct RemoteCast { int skill, x, y; };
    struct RemoteMelee { unsigned x,y,facing,element,opcode; bool harvest; };
    struct HumanHit {
        WORD opcode,current,maximum,damage;
        mir3_cui::CharacterAppearance appearance;
        std::uint32_t source=0;
        unsigned variant=0;
    };
    struct HumanHealth {
        WORD current=0,maximum=0;
        mir3::DamageNumbers damage;
        std::uint64_t registeredUntil=0;
        std::optional<std::uint8_t> lastMonster;
    };
    struct HumanStatus {
        std::uint32_t flags;
        std::int16_t speed;
    };
    struct LocalLifeAction {
        WORD opcode,x,y,direction;
        mir3_cui::CharacterAppearance appearance;
        unsigned mount;
        std::uint32_t flags;
    };
    struct LocalStep {
        static constexpr std::uint64_t FrameMs=96;
        int frame=-1,dx=0,dy=0;
        unsigned destinationX=0,destinationY=0;
        WORD opcode=0;
        std::uint64_t frameAt=0;
        bool walking() const { return opcode==GameWalk || opcode==GameRun || opcode==GameActorReviveRefresh; }
        bool predicted() const { return opcode==GameWalk || opcode==GameRun; }
    };
    struct RemoteMovement {
        WORD opcode,x,y,direction;
        mir3_cui::CharacterAppearance appearance;
        unsigned mount=0;
        std::uint32_t flags=0,portrait=0;
        std::string name;
    };
    struct RemoteStep {
        int frame=-1,dx=0,dy=0;
        unsigned destinationX=0,destinationY=0;
        WORD opcode=0;
        bool catchUp=false;
    };
    using LocalAction = std::variant<HumanHit,LocalLifeAction,HumanStatus>;
    using RemoteAction = std::variant<RemoteCast,mir3::SpellReply,RemoteMelee,HumanHit,RemoteMovement,HumanStatus>;
    struct PendingMelee { unsigned x,y,facing; WORD opcode; std::uint64_t at; };
    struct Movement {
        unsigned x = 0, y = 0, facing = 0;
        std::uint64_t deadline = 0;
    };
    struct WalkRequest {
        unsigned x,y,facing,originX,originY;
        std::uint32_t admitted;
        std::uint64_t sendAt;
    };

    struct Player {
        struct MonsterAction {
            unsigned x=0, y=0, facing=0, shape=0;
            std::uint32_t status=0;
            WORD opcode=0, current=0, maximum=0, damage=0;
            int light=-1;
        };
        unsigned x = 0, y = 0, facing = 0;
        unsigned light = 0;
        unsigned mount = 0;
        std::uint64_t levelPoseAt = 0;
        mir3::CastVisual casting;
        HumanHealth health;
        std::deque<RemoteAction> remoteActions;
        RemoteStep remoteStep;
        bool remoteSlowTick=false;
        std::uint32_t actionFlags = 0;
        std::uint64_t combatUntil = 0, combatIdleAt = 0;
        bool dead = false;
        bool dying = false;
        std::uint64_t revivedAt = 0;
        unsigned npcType = 0;
        mir3_cui::CharacterAppearance appearance;
        std::string name;
        mir3::SceneName identity;
        unsigned npcShape = 0;
        mir3::TargetPortraitVitals monsterVitals;
        mir3::DamageNumbers monsterDamage;
        mir3::TargetAttributes monsterAttributes;
        bool monsterAttributesRequested = false;
        std::deque<MonsterAction> monsterActions;
        std::uint32_t monsterStatus = 0;
        std::uint64_t monsterNextStep = 0, monsterIdleAt = 0;
        int monsterWalkFrame = -1;
        bool monsterBackstep = false, monsterCatchUp = false, monsterSkipStep = false;
        mir3::MonsterMotion monsterCombatMotion;
        std::uint64_t monsterCombatAt = 0;
        int monsterCombatFrame = -1;
        int monsterDeathFrame = -1;
        bool monsterSkeleton = false;
        std::uint64_t monsterDeathAt = 0, monsterReviveAt = 0;
        std::uint64_t crouchedAt = 0;
        std::uint32_t portraitVersion = 0;
        std::uint32_t resolvedPortraitVersion = 0;
        std::string portraitJpeg;
        std::uint64_t portraitQueryAt = 0;

        bool IsMonster() const { return npcType == 3; }
        bool IsNpc() const { return npcType == 4 || npcType == 5; }
        bool CanBeButchered() const {
            return dead && !IsNpc() && (!IsMonster() || (!monsterSkeleton &&
                (npcShape==20 || npcShape==21 || npcShape==119)));
        }
        bool MonsterDying() const {
            return monsterDeathFrame>=0 && !monsterSkeleton && monsterDeathFrame<mir3::monster_death_motion(npcShape).frames-1;
        }
        bool MonsterAnimating() const { return monsterWalkFrame>=0 || monsterCombatFrame>=0 || MonsterDying(); }
    };

    Movement pending;
    std::optional<WalkRequest> pendingWalk;
    std::optional<Movement> movementOrigin;
    std::uint64_t walkPointerAt=0;
    bool runPointerCaptured=false;
    std::uint64_t lastMove = 0;
    std::uint64_t crouchedAt = 0, crouchReplyAt = 0, crouchHoldAt = 0;
    bool crouchPointerCaptured = false;
    unsigned x = 0, y = 0, facing = 0;
    std::string mapId;
    std::string mapDescription;
    mir3_cui::MapTerrain terrain;
    mir3_cui::MapTerrain::View ground{};
    mir3::SceneAmbient ambient;
    std::uint64_t ambientTick = 0;
    mir3::SnowWeather weather;
    std::uint64_t weatherTick = 0;
    unsigned light = 0;
    struct LevelPlayback {
        mir3::LevelUpEffect effect;
        std::uint64_t tick;
    };
    std::vector<LevelPlayback> levelEffects;
    std::uint64_t levelPoseAt = 0;
    mir3::CastVisual casting;
    HumanHealth health;
    std::deque<LocalAction> localActions;
    bool dead=false,dying=false;
    LocalStep localStep;
    std::uint64_t remoteMovementAt=0;
    std::uint64_t lastSpecialHit=0;
    unsigned foldedHits=0;
    std::deque<mir3::SpellReply> spellMessages;
    std::unique_ptr<mir3::SpellEffects> spellEffects;
    std::uint64_t spellEffectsTick=0;
    mir3::SpellSoundVoices spellSounds;
    mir3::SpellTiming spellTiming;
    std::uint32_t lockedSpellActor = 0;
    std::uint64_t spellReplyUntil = 0, mapEnteredAt = 0;
    mir3::MeleeSpells meleeSpells;
    mir3::MeleeCooldown meleeCooldown;
    mir3::LocalActionClock actionClock;
    std::optional<PendingMelee> pendingMelee;
    bool meleePointerCaptured = false;
    std::uint32_t actionFlags = 0;
    bool ActionInputBlocked() const {
        return dead || pendingWalk || localStep.frame>=0 || !localActions.empty() || (!casting.clock.finished() &&
            (casting.clock.action()==15 || casting.clock.action()==32));
    }
    unsigned spellOriginalFacing = 0;
    std::map<std::uint32_t, Player> players;
    mir3::GroundItems groundItems;
    std::map<std::uint32_t,mir3::SpeechBubble> speech;
    bool pickupBagFull = false;

    bool InputDelayed(std::uint64_t now) const
    {
        return mapEnteredAt && now-mapEnteredAt<1500;
    }

    bool Occupied(int cellX, int cellY) const
    {
        return std::any_of(players.begin(), players.end(), [&](const auto& entry) {
            const auto& player = entry.second;
            return !player.dead && static_cast<int>(player.x) == cellX && static_cast<int>(player.y) == cellY;
        });
    }

    bool CanButcher(const Player& target) const
    {
        return target.CanBeButchered() && std::abs(int(target.x)-int(x))<=1 && std::abs(int(target.y)-int(y))<=1;
    }

    mir3_cui::GroundCell GroundAt(int nextX, int nextY) const
    {
        constexpr int columns = mir3_cui::MapTerrain::ViewColumns;
        constexpr int rows = mir3_cui::MapTerrain::ViewRows;
        const int column = nextX - static_cast<int>(x) + columns / 2;
        const int row = nextY - static_cast<int>(y) + rows / 2;
        if (column < 0 || column >= columns || row < 0 || row >= rows) return mir3_cui::GroundCell::Outside;
        return ground[row * columns + column];
    }

    void SetCell(unsigned nextX, unsigned nextY)
    {
        if (x == nextX && y == nextY) return;
        x = nextX;
        y = nextY;
        ground = terrain.ReadView(x, y);
    }
};

inline constexpr std::array<std::array<int, 2>, 8> SceneDirections{{
    {0,-1},{1,-1},{1,0},{1,1},{0,1},{-1,1},{-1,0},{-1,-1}
}};
inline constexpr float SceneCellPitch = 24;

inline std::array<int,2> LocalActorPixel(const LocalScene& scene)
{
    std::array pixel{int(scene.x)*48,int(scene.y)*32};
    const auto& step=scene.localStep;
    if (step.frame<0) return pixel;
    const unsigned direction=(scene.facing+(step.opcode==GameBackstep ? 4 : 0))%8;
    const bool committed=step.frame>=(direction==7 ? 4 : 1);
    const int scale=committed ? step.frame-5 : step.frame+1;
    pixel[0]+=step.dx*8*scale;
    pixel[1]+=(committed && direction%2 ? 32*step.dy/6 : step.dy*5)*scale;
    return pixel;
}

inline std::array<int,2> LocalActorCamera(const LocalScene& scene)
{
    const auto& step=scene.localStep;
    if (step.frame<0) return {int(scene.x)*48-376,int(scene.y)*32-227};
    constexpr std::array x{8,16,24,32,40,48},y{5,11,16,21,27,32};
    return {(int(step.destinationX)-step.dx)*48-376+step.dx*x[step.frame],
            (int(step.destinationY)-step.dy)*32-227+step.dy*y[step.frame]};
}

inline mir3::SpellPixel ActorWorldPixel(const LocalScene::Player& actor)
{
    int x=int(actor.x)*48,y=int(actor.y)*32;
    const auto& step=actor.remoteStep;
    if (step.frame>=0) {
        const bool committed=step.frame>=(actor.facing==7 ? 4 : 1);
        const int offset=committed ? step.frame-5 : step.frame+1;
        x+=step.dx*8*offset;
        y+=step.opcode==GameBackstep && !committed ? 32*step.dy*offset/6 : 5*step.dy*offset;
    }
    if (actor.monsterWalkFrame>=0) {
        const auto [dx,dy]=SceneDirections[(actor.facing+(actor.monsterBackstep ? 4 : 0))%8];
        const int step=actor.monsterWalkFrame+1;
        const bool reanchored=actor.monsterBackstep && actor.monsterWalkFrame>=(actor.facing==7 ? 4 : 1);
        x-=dx*8*(6-step);
        y-=dy*(reanchored ? 5*(6-step) : 32-(dx==0 ? 32*step/6 : 5*step));
    }
    return {float(x),float(y)};
}

struct Character {
    std::string name;
    std::string label;
    unsigned profession = 0;
    unsigned level = 0;
    unsigned gender = 0;
    bool operator==(const Character&) const = default;
};

struct Server {
    std::string name;
    std::string label;
    WORD index = 0;
    unsigned load = 0;
};

struct BillingNotice {
    int message = 0;
    int argument = 0;
};

inline BillingNotice ReadBilling(const _TDEFAULTMESSAGE& header)
{
    if (header.wTag) return header.wTag == 1 ? BillingNotice{102} : BillingNotice{103, header.wTag - 1};
    if (header.wSeries) return header.wSeries == 1 ? BillingNotice{105} : BillingNotice{104, header.wSeries - 1};
    if (header.wParam) return header.wParam <= 100 ? BillingNotice{106, header.wParam} : BillingNotice{};
    if (header.nRecog > 0) return {107, header.nRecog};
    return {};
}

inline unsigned Number(const std::string& text, unsigned maximum, unsigned invalid = 0)
{
    if (text.empty()) return invalid;
    unsigned value = 0;
    for (char digit : text) {
        if (digit < '0' || digit > '9' || value > maximum / 10 ||
            (value == maximum / 10 && static_cast<unsigned>(digit - '0') > maximum % 10))
            return invalid;
        value = value * 10 + digit - '0';
    }
    return value;
}

inline std::string ConvertText(const std::string& text, UINT source, UINT destination)
{
    const int length = static_cast<int>(text.size());
    const int wideLength = MultiByteToWideChar(source, MB_ERR_INVALID_CHARS, text.data(), length, nullptr, 0);
    if (!wideLength) return {};

    std::wstring wide(wideLength, L'\0');
    MultiByteToWideChar(source, MB_ERR_INVALID_CHARS, text.data(), length, &wide[0], wideLength);
    BOOL substituted = FALSE;
    BOOL* usedDefault = destination == CP_UTF8 ? nullptr : &substituted;
    const DWORD flags = usedDefault ? WC_NO_BEST_FIT_CHARS : 0;
    const int size = WideCharToMultiByte(destination, flags, wide.data(), wideLength, nullptr, 0, nullptr, usedDefault);
    if (!size || substituted) return {};

    std::string result(size, '\0');
    WideCharToMultiByte(destination, flags, wide.data(), wideLength, &result[0], size, nullptr, usedDefault);
    return result;
}

inline Gateway ReadGateway(const std::string& body, bool requireTerminator)
{
    const auto slash = body.find('/');
    if (slash == std::string::npos || slash > 15) return {};
    const auto end = body.find('/', slash + 1);
    if (requireTerminator && end == std::string::npos) return {};
    const auto port = body.substr(slash + 1, end == std::string::npos ? end : end - slash - 1);
    Gateway gateway{body.substr(0, slash), Number(port, 65535)};
    in_addr address = {};
    if (!gateway.port || InetPtonA(AF_INET, gateway.host.c_str(), &address) != 1) return {};
    return gateway;
}

inline bool ReadCharacters(int count, const std::string& body, std::vector<Character>& result)
{
    if (count < 0 || body.size() > 255) return false;
    if (count > 4) count = 4;
    std::vector<Character> next;
    size_t start = 0;
    for (int role = 0; role < count; ++role) {
        if (start < body.size() && body[start] == '*') ++start;
        std::array<std::string, 5> fields;
        for (size_t i = 0; i < fields.size(); ++i) {
            const auto end = body.find('/', start);
            if (end == std::string::npos && (i != fields.size() - 1 || role + 1 < count)) return false;
            fields[i] = body.substr(start, end - start);
            start = end == std::string::npos ? body.size() : end + 1;
        }
        const auto profession = Number(fields[1], 2, 3);
        const auto level = Number(fields[3], 255, 256);
        const auto gender = Number(fields[4], 1, 2);
        const auto label = ConvertText(fields[0], 936, CP_UTF8);
        if (label.empty() || fields[0].size() > 24 || profession > 2 || level > 255 || gender > 1)
            return false;
        next.push_back({fields[0], label, profession, level, gender});
    }
    result = std::move(next);
    return true;
}

inline bool ValidCharacterName(const std::string& name)
{
    if (name.empty() || name.size() > 13) return false;
    for (size_t i = 0; i < name.size(); ++i) {
        const auto ch = static_cast<unsigned char>(name[i]);
        if (ch >= 0x80 && i + 1 < name.size()) {
            ++i;
        } else if (!(ch >= 'A' && ch <= 'Z') && !(ch >= 'a' && ch <= 'z') &&
                   !(ch >= '0' && ch <= '9')) {
            return false;
        }
    }
    return true;
}

inline std::string TrimIni(const std::string& value)
{
    const auto first = value.find_first_not_of(" \t\r");
    if (first == std::string::npos) return {};
    return value.substr(first, value.find_last_not_of(" \t\r") - first + 1);
}

inline std::string IniKey(std::string value)
{
    for (char& ch : value) {
        if (ch >= 'A' && ch <= 'Z') ch += 'a' - 'A';
    }
    return value;
}

inline std::vector<std::string> CommaFields(const std::string& value)
{
    std::vector<std::string> fields;
    size_t start = 0;
    for (auto end = value.find(','); end != std::string::npos; end = value.find(',', start)) {
        fields.push_back(value.substr(start, end - start));
        start = end + 1;
    }
    fields.push_back(value.substr(start));
    return fields;
}

struct GroupLogin {
    std::string host;
    unsigned port = 0;
    std::string registration;
    std::string recovery;
    std::vector<Server> servers;
};

struct GroupLine {
    std::string caption;
    std::string loginSection;
    unsigned row = 0;
};

struct ServerGroup {
    std::string label;
    std::string title;
    std::vector<GroupLine> lines;
    unsigned x = 0, y = 0, rowCount = 0;
};

class ServerGroups {
public:
    std::vector<ServerGroup> groups;
    int selected = -1;

    bool Load(std::istream& file)
    {
        ServerGroups next;
        std::string section, line;
        while (std::getline(file, line)) {
            line = TrimIni(line);
            if (line.empty() || line.front() == ';') continue;
            if (line.front() == '[' && line.back() == ']') {
                section = IniKey(TrimIni(line.substr(1, line.size() - 2)));
                continue;
            }
            const auto split = line.find('=');
            if (split == std::string::npos) continue;
            auto value = TrimIni(line.substr(split + 1));
            if (value.size() >= 2 && value.front() == value.back() &&
                (value.front() == '"' || value.front() == '\''))
                value = value.substr(1, value.size() - 2);
            next.ini_.emplace(section + '/' + IniKey(TrimIni(line.substr(0, split))), std::move(value));
        }

        const unsigned count = Number(next.Get("ServerGroup", "Count"), 40);
        if (!count) return false;
        for (unsigned i = 1; i <= count; ++i) {
            const auto parts = CommaFields(next.Get("ServerGroup", "Server" + std::to_string(i)));
            if (parts.size() != 4) return false;
            const auto x = Number(TrimIni(parts[2]), 800, 801);
            const auto y = Number(TrimIni(parts[3]), 600, 601);
            if (x > 800 || y > 600) return false;

            const auto groupSection = "ServerGroup" + std::to_string(i);
            const unsigned rows = Number(next.Get(groupSection, "Count"), 40, 41);
            if (rows > 40) return false;
            ServerGroup group{ConvertText(parts[0], 936, CP_UTF8), ConvertText(parts[1], 936, CP_UTF8), {}, x, y, rows};
            for (unsigned j = 1; j <= rows; ++j) {
                const auto row = CommaFields(next.Get(groupSection, "Server" + std::to_string(j)));
                if (row.size() != 3 || row[0].size() > 31 || row[1].size() > 31 || row[2].size() > 31)
                    return false;
                if (row[0] != "empty") group.lines.push_back({ConvertText(row[0], 936, CP_UTF8), row[1], j - 1});
            }
            if (x || y) next.groups.push_back(std::move(group));
        }
        if (next.groups.empty()) return false;
        *this = std::move(next);
        return true;
    }

    bool Login(const GroupLine& line, GroupLogin& output) const
    {
        if (line.loginSection.empty() || IniKey(line.loginSection) == "empty") return false;
        GroupLogin next;
        next.host = Get(line.loginSection, "ServerAddr");
        next.port = Number(Get(line.loginSection, "Param1"), 65535);
        in_addr address = {};
        if (!next.port || next.host.size() > 15 ||
            InetPtonA(AF_INET, next.host.c_str(), &address) != 1)
            return false;
        next.registration = ConvertText(Get(line.loginSection, "Param2"), 936, CP_UTF8);
        next.recovery = ConvertText(Get(line.loginSection, "Param3"), 936, CP_UTF8);
        if (next.registration.size() >= 512 || next.recovery.size() >= 512) return false;

        const auto list = Get(line.loginSection, "ServerList");
        for (size_t open = list.find('('); open != std::string::npos;) {
            const auto close = list.find(')', open + 1);
            const auto slash = list.find('/', open + 1);
            if (close == std::string::npos || slash >= close || slash + 1 == close ||
                next.servers.size() == 256)
                return false;
            const auto name = list.substr(slash + 1, close - slash - 1);
            auto caption = ConvertText(list.substr(open + 1, slash - open - 1), 936, CP_UTF8);
            if (caption.empty()) caption = ConvertText(name, 936, CP_UTF8);
            next.servers.push_back({name, caption});
            open = list.find('(', close + 1);
        }
        output = std::move(next);
        return true;
    }

private:
    std::map<std::string, std::string> ini_;

    std::string Get(const std::string& section, const std::string& key) const
    {
        const auto found = ini_.find(IniKey(section) + '/' + IniKey(key));
        return found == ini_.end() ? std::string{} : found->second;
    }
};

class LoginClient {
public:
    char host[16] = "127.0.0.1";
    char port[6] = "7001";
    char account[65] = {};
    char password[65] = {};
    char registration[512] = {};
    char recovery[512] = {};
    bool legacy = false;
    bool settingsOpen = false;
    unsigned noticeDismissals = 0;
    unsigned noticePasswordLength = 0;
    int loginNoticeId = 0;
    Stage stage = Stage::Login;
    ServerGroups serverGroups;
    std::vector<Server> configuredServers;
    std::vector<Server> servers;
    BillingNotice billing;
    int selectionSession = 0;
    Gateway selectionGateway;
    Gateway gameGateway;
    std::string gameCharacter;
    static constexpr std::uint64_t GameExitFadeMs = 2500;
    bool showSignOut = false;
    bool quitGameRequested = false;
    std::uint64_t returnToCharactersAt = 0;
    std::array<std::uint32_t,3> exitGuardTicks{};
    std::uint8_t exitGuardAction = 0;
    mir3_cui::ChatPanel chat;
    mir3_cui::ServerOptions serverOptions;
    mir3_cui::PartyPanel party;
    mir3_cui::GuildPanel guild;
    mir3_cui::QuestPanel quests;
    mir3_cui::InventoryPanel inventory;
    mir3_cui::MerchantService merchant;
    std::deque<mir3_cui::MerchantResult> merchantResults;
    unsigned merchantRevision = 0;
    mir3_cui::EquipmentPanel equipment;
    mir3_cui::CharacterStatus characterStatus;
    mir3_cui::CharacterAppearance characterAppearance;
    LocalScene scene;
    mir3::SkillPanel skills;
    mir3::MountPanel mount;
    mir3_cui::TradePanel trade;
    std::uint64_t mountInputUntil = 0;
    bool skillShortcutsVisible = false;
    mir3::Minimap minimap;
    WORD minimapReply = 0;
    bool minimapUnavailable = false, minimapShowActors = true;
    bool portraitMapMode = false, portraitMapPending = false, portraitMapReady = false;
    bool forceAttack = true, magicLight = true, damageNumbers = true, shadowBlend = false, userPhotos = true;
    bool snowEnabled = true;
    struct LocalOption {
        bool LoginClient::*value;
        const wchar_t* key;
        bool initial;
        int enabledMessage, disabledMessage;
    };
    static constexpr std::array<LocalOption,6> LocalOptions{{
        {&LoginClient::minimapShowActors,L"ShowMon",true,2646,2647},
        {&LoginClient::forceAttack,L"AttackUser",true,1515,1516},
        {&LoginClient::magicLight,L"MagicLight",true,1518,1517},
        {&LoginClient::damageNumbers,L"BloodNum",true,2122,2121},
        {&LoginClient::shadowBlend,L"ShadowBlend",false,0,0},
        {&LoginClient::userPhotos,L"UserPic",true,2132,2131}
    }};
    std::uint64_t minimapDeadline = 0;
    mir3_cui::NpcDialogue npcDialogue;
    mir3_cui::NpcShop shop;
    mir3_cui::NpcShop::Purchase purchase;
    unsigned shopRevision = 0;
    std::uint64_t lastNpcClick = 0, lastNpcAction = 0;
    mir3_cui::AudioOptions audioOptions;
    mir3_cui::AudioPlayer audio;
    std::wstring audioOptionsPath;
    std::string audioSaveError;
    bool audioOpen = false;
    bool audioDirty = false;
    std::string chatHistoryDirectory = "CuiChatHistory";
    std::string itemShortcutDirectory = "CuiItemShortcuts";
    std::string inventoryLayoutDirectory = "CuiInventoryLayouts";
    std::filesystem::path portraitCacheDirectory;
    std::vector<Character> characters;
    int characterIndex = -1;
    int characterPage = 0;
    int profession = 0;
    int gender = 0;
    int characterEditor = 0;
    bool deleteCancelSelected = false;
    bool deleteTabDown = false;
    char characterInput[361] = {};
    CharacterDialog characterDialog = CharacterDialog::None;
    std::string characterNotice;
    int characterNoticeId = 0;
    unsigned characterListRevision = 0;
    WORD characterRequest = 0;
    std::string status;

    ~LoginClient() { Reset(); }

    std::filesystem::path PortraitCachePath() const
    {
        const auto valid = [](const std::string& value) {
            return !value.empty() && value.find_first_of("/\\:\"<>|?*")==std::string::npos &&
                std::none_of(value.begin(),value.end(),[](unsigned char ch) { return ch<32; });
        };
        const auto server = ConvertText(realm_,936,CP_UTF8);
        const auto role = ConvertText(gameRole_,936,CP_UTF8);
        if (!valid(server) || !valid(role)) return {};
        const auto directory = portraitCacheDirectory.empty() ? resourceRoot_/"Data" : portraitCacheDirectory;
        return directory/std::filesystem::u8path(server+"_"+role+".fac");
    }

    bool SendPortrait(const std::string& jpeg)
    {
        constexpr WORD GamePortraitUpload = 1151;
        if (stage!=Stage::GameSession || jpeg.size()>4095 || output_.size()>=8192) return false;
        Queue(GamePortraitUpload,0,jpeg);
        return true;
    }

    void RequestTargetPortrait(LocalScene::Player& player)
    {
        if (stage!=Stage::GameSession || !userPhotos || player.npcType || player.name.empty() ||
            player.portraitVersion==player.resolvedPortraitVersion || output_.size()>=8192) return;
        const auto now = GetTickCount64();
        if (now-player.portraitQueryAt<=1000) return;
        const auto name = ConvertText(player.name,CP_UTF8,936);
        if (name.empty()) return;
        Queue(1152,0,name);
        player.portraitQueryAt = now;
    }

    void Reset(const char* message = "")
    {
        if (stage == Stage::GameSession || stage == Stage::SigningOutGame) SaveInventoryLayout();
        Close();
        servers.clear();
        billing = {};
        selectionSession = 0;
        selectionGateway = {};
        ClearGameState();
        ClearCharacterSelection();
        signedInAccount_.clear();
        realm_.clear();
        stage = Stage::Login;
        status = message;
        noticePasswordLength = 0;
        loginNoticeId = 0;
        SecureZeroMemory(password, sizeof password);
    }

private:
    void ClearCharacterSelection()
    {
        characters.clear();
        characterIndex = -1;
        characterPage = 0;
        characterRequest = 0;
        characterNotice.clear();
        characterNoticeId = 0;
        CloseCharacterDialog();
    }

    void ClearGameState()
    {
        gameGateway = {};
        gameCharacter.clear();
        gameRole_.clear();
        showSignOut = false;
        quitGameRequested = false;
        returnToCharactersAt = 0;
        exitGuardTicks = {};
        exitGuardAction = 0;
        chat = {};
        serverOptions = {};
        party = {};
        guild = {};
        quests = {};
        skills = {};
        mount = {};
        trade = {};
        mountInputUntil = 0;
        skillShortcutsVisible = false;
        inventory = {};
        itemShortcutFilePresent_ = false;
        equipment = {};
        characterStatus = {};
        characterAppearance = {};
        scene = {};
        audio.ClearSceneEffects();
        ClearMinimap();
        CloseNpcDialogue();
        merchant = {};
        merchantResults.clear();
        purchase = {};
        shopRevision = 0;
        lastNpcClick = lastNpcAction = 0;
        audioOpen = false;
    }

public:
    void DismissNotice()
    {
        status.clear();
        loginNoticeId = 0;
        noticePasswordLength = 0;
        SecureZeroMemory(password, sizeof password);
        ++noticeDismissals;
    }

    void ShowLoginNotice(int id, const char* fallback)
    {
        const auto length = password[0] ? static_cast<unsigned>(ConvertText(password, CP_UTF8, 936).size())
                                        : noticePasswordLength;
        Reset(fallback);
        loginNoticeId = id;
        noticePasswordLength = length;
    }

    bool OpenGroupLine(const GroupLine& line)
    {
        GroupLogin config;
        if (!serverGroups.Login(line, config)) {
            status = "This line has an invalid login configuration.";
            return false;
        }
        Reset();
        strcpy_s(host, config.host.c_str());
        sprintf_s(port, "%u", config.port);
        strcpy_s(registration, config.registration.c_str());
        strcpy_s(recovery, config.recovery.c_str());
        configuredServers = std::move(config.servers);
        settingsOpen = false;
        legacy = false;
        return true;
    }

    bool Active() const { return socket_ != INVALID_SOCKET; }

    void SignIn()
    {
        sockaddr_in address = {};
        address.sin_family = AF_INET;
        address.sin_port = htons(static_cast<u_short>(Number(port, 65535)));
        if (!address.sin_port || InetPtonA(AF_INET, host, &address.sin_addr) != 1) {
            status = "Enter an IPv4 address and a port from 1 to 65535.";
            return;
        }
        const auto user = ConvertText(account, CP_UTF8, 936);
        auto secret = ConvertText(password, CP_UTF8, 936);
        if (user.empty() || secret.empty() || user.find('/') != std::string::npos ||
            secret.find('/') != std::string::npos) {
            if (!secret.empty()) SecureZeroMemory(&secret[0], secret.size());
            status = "Enter an account and password supported by the server. Omit slash characters.";
            return;
        }
        if ((!legacy && user.size() > 27) || (legacy && user.size() + 1 + secret.size() >= 32)) {
            SecureZeroMemory(&secret[0], secret.size());
            status = legacy ? "The legacy gateway accepts up to 31 bytes for account, slash, and password."
                            : "The character gateway accepts account names up to 27 bytes.";
            return;
        }

        const int result = Connect(address);
        if (result == SOCKET_ERROR) {
            SecureZeroMemory(&secret[0], secret.size());
            Reset("Could not connect to the login gateway.");
            return;
        }

        signedInAccount_ = user;
        std::string credentials = user + '/';
        credentials += secret;
        Queue(CM_IDPASSWORD, 0, credentials);
        noticePasswordLength = static_cast<unsigned>(secret.size());
        SecureZeroMemory(&credentials[0], credentials.size());
        SecureZeroMemory(&secret[0], secret.size());
        SecureZeroMemory(password, sizeof password);
        Wait(result == 0 ? Stage::SigningIn : Stage::Connecting, "Contacting the login gateway...");
    }

    void Select(const Server& server)
    {
        if (stage != Stage::Servers || billing.message) return;
        realm_ = server.name;
        Queue(CM_SELECTSERVER, server.index, legacy ? "" : server.name);
        Wait(Stage::Selecting, "Waiting for the selection endpoint...");
    }

    std::uint64_t HandoffRemainingMs() const
    {
        const auto now = GetTickCount64();
        return deadline_ > now ? (std::min)(deadline_ - now, std::uint64_t{2000}) : 0;
    }

    void OpenCharacterDialog(CharacterDialog dialog)
    {
        if (dialog == CharacterDialog::Delete && characterIndex < 0) {
            ShowCharacterNotice(227, "Select a character to delete.");
            return;
        }
        characterDialog = dialog;
        profession = 0;
        gender = 0;
        ++characterEditor;
        deleteCancelSelected = false;
        deleteTabDown = false;
        SecureZeroMemory(characterInput, sizeof characterInput);
    }

    void CloseCharacterDialog()
    {
        characterDialog = CharacterDialog::None;
        SecureZeroMemory(characterInput, sizeof characterInput);
    }

    void SubmitCharacterDialog()
    {
        if (stage != Stage::Characters || characterRequest || !characterNotice.empty()) return;
        auto text = ConvertText(characterInput, CP_UTF8, 936);
        if (characterDialog == CharacterDialog::Create) {
            if (!ValidCharacterName(text)) {
                ShowCharacterNotice(225, "Use letters, digits, or Chinese characters, up to 13 bytes.");
                return;
            }
            RequestCharacter(CM_NEWCHR, signedInAccount_ + '/' + text + "/1/" +
                std::to_string(profession) + '/' + std::to_string(gender));
        } else if (characterDialog == CharacterDialog::Delete && characterIndex >= 0) {
            if ((characterInput[0] && text.empty()) || text.size() > 90 || text.find('/') != std::string::npos) {
                ShowCharacterNotice(227, "Enter confirmation text up to 90 bytes, without slashes.");
                return;
            }
            RequestCharacter(CM_DELCHR, characters[characterIndex].name + '/' + text);
            QueryCharacters();
            CloseCharacterDialog();
            if (!text.empty()) SecureZeroMemory(&text[0], text.size());
        }
    }

    void StartCharacter()
    {
        if (stage != Stage::Characters || characterRequest || !characterNotice.empty()) return;
        if (characterIndex < 0) {
            ShowCharacterNotice(230, "Select a character to continue.");
            return;
        }
        RequestCharacter(CM_SELCHR, signedInAccount_ + '/' + characters[characterIndex].name);
    }

    void RefreshCharacterList()
    {
        if (stage == Stage::Characters && !characterRequest) QueryCharacters();
    }

    void ShowCharacterNotice(int id, const char* fallback)
    {
        characterNoticeId = id;
        characterNotice = fallback;
    }

    bool GameExitAllowed() const
    {
        if (exitGuardAction==19) return true;
        const auto now = GetTickCount();
        return std::none_of(exitGuardTicks.begin(),exitGuardTicks.end(),
            [now](auto tick) { return std::uint32_t(now-tick)<=8000u; });
    }

    void SignOutGame(bool returnToCharacters = false)
    {
        if (stage != Stage::GameSession || (returnToCharacters && !GameExitAllowed())) return;
        showSignOut = false;
        Queue(GameLogout, 0, "");
        Wait(Stage::SigningOutGame, "Signing out...");
        if (returnToCharacters) {
            SaveInventoryLayout();
            if (audioOpen) SaveAudioSettings();
            returnToCharactersAt = GetTickCount64()+GameExitFadeMs+1;
        }
    }

    void SendChat(bool addChannelPrefix = true)
    {
        if (stage != Stage::GameSession || showSignOut || chat.receptionOpen || party.open || party.notice.id ||
            guild.open || guild.requested || guild.notice || guild.action != mir3_cui::GuildAction::None ||
            quests.open || inventory.open || inventory.notice || equipment.open ||
            !chat.draft[0]) return;
        const std::string draft = chat.draft;
        const bool command = draft.front() == '/' || draft.front() == '@' ||
            draft.front() == '#' || draft.front() == '!';
        const std::string text = addChannelPrefix && !command ? chat.ChannelPrefix() + draft : draft;
        const auto wire = ConvertText(text, CP_UTF8, 936);
        if (wire.empty() || wire.size() > 90 || draft.find_first_of("\r\n") != std::string::npos) {
            chat.error = "The message must fit within 90 GBK bytes.";
            return;
        }
        if (draft.front() != '#' && output_.size() >= 8192) {
            chat.error = "The connection is busy. Try sending again.";
            return;
        }
        chat.error.clear();
        chat.draft[0] = '\0';
        if (wire == "@\xcc\xed\xbc\xd3\xb3\xc9\xd4\xb1") {
            BeginGuildAction(mir3_cui::GuildAction::Add);
            return;
        }
        if (draft.front() == '#') {
            if (chat.Remember(chat.phrases, draft.substr(1))) SaveChatHistory();
            return;
        }
        if (draft.front() == '/') {
            const auto end = draft.find(' ');
            if (draft.size() > 1 && end != 1 && chat.Remember(chat.contacts, draft.substr(0, end) + ' '))
                SaveChatHistory();
            chat.Append(103, draft, 16750230);
        }
        if (draft == "@chatsave") chat.logging = !chat.logging;
        if (draft == "@snow") snowEnabled = !snowEnabled;
        Queue(mir3_cui::Say, 0, wire);
    }

    void ToggleChatReception(mir3_cui::ReceptionChannel channel)
    {
        if (SendOptionCommand(mir3_cui::ChatReception::ToggleCommand(channel), chat.reception.error))
            chat.reception.Request(channel);
    }

    void ToggleServerOption(const mir3_cui::ServerOption& option)
    {
        if (SendOptionCommand(option.command, serverOptions.error)) serverOptions.Request(option);
    }

    void RequestAttackMode()
    {
        if (SendOptionCommand(mir3_cui::AttackModeCommand, serverOptions.error))
            serverOptions.attackModePending = true;
    }

    void BindSkill(int id, int key)
    {
        if (stage != Stage::GameSession || showSignOut || key < 1 || key > 12) return;
        std::size_t messages = skills.find(id) ? 1 : 0;
        for (const auto& group : skills.learned)
            messages += std::count_if(group.begin(),group.end(),[key](const auto& skill) { return skill.key == key; });
        if (output_.size() + messages*19 > 8192) return;
        skills.bindings[key] = 0;
        for (auto& group : skills.learned) for (auto& skill : group) {
            if (skill.key != key) continue;
            skill.key = 0;
            Queue(ClientSkillBinding,0,"",skill.id);
        }
        for (auto& skill : skills.learned[mir3::SkillLayout::category(id)]) {
            if (skill.id != id) continue;
            if (skill.key && skills.bindings[skill.key] == (id&255)) skills.bindings[skill.key] = 0;
            skill.key = key;
            skills.bindings[key] = id&255;
            Queue(ClientSkillBinding,key <= 9 ? '0'+key : 'A'+key-10,"",id);
            break;
        }
    }

    void SendMountCommand(const std::string& text)
    {
        if (stage != Stage::GameSession || showSignOut || output_.size() >= 8192) return;
        const auto command = ConvertText(text, CP_UTF8, 936);
        if (command.empty()) return;
        Queue(mir3_cui::Say,0,command);
        mountInputUntil = GetTickCount64()+300;
    }

    void RequestTrade()
    {
        if (stage != Stage::GameSession || showSignOut || output_.size() >= 8192 ||
            scene.facing >= SceneDirections.size()) return;
        const auto [dx,dy] = SceneDirections[scene.facing];
        for (const auto& [id,actor] : scene.players) {
            if (actor.dead || actor.npcType || actor.name.empty() || static_cast<int>(actor.x)!=static_cast<int>(scene.x)+dx ||
                static_cast<int>(actor.y)!=static_cast<int>(scene.y)+dy) continue;
            const auto name = ConvertText(actor.name,CP_UTF8,936);
            if (!name.empty()) Queue(ClientTradeRequest,0,name);
            break;
        }
    }

    int ActivateTradeCell(int slot)
    {
        using namespace mir3_cui;
        if (!trade.visible || trade.confirmed || trade.offerSlot>=0 || trade.returnPending || trade.goldPending || inventory.heldGold) return 0;
        if (const auto* item = HeldItem()) {
            if (!item->key.id || equipment.heldSlot>=0 || inventory.heldShortcut>=0 ||
                inventory.request!=ItemRequest::None || equipment.request!=EquipmentRequest::None || output_.size()>=8192) return 0;
            if (trade.items[0].size()>=TradePanel::Capacity || !TradePanel::Fits(trade.items[0],*item,slot))
                return TradePanel::AtCell(trade.items[0],slot) ? 1306 : 0;
            trade.offerSlot = slot;
            trade.offeredItem = *item;
            if (item->Stacks() && item->count>1) trade.OpenPrompt(TradePrompt::Quantity);
            else SendTradeOffer((std::max)(1u,item->count));
        } else {
            const auto* selected = TradePanel::AtCell(trade.items[0],slot);
            trade.returnSlot = selected && selected->key.id ? selected->cell : -1;
        }
        return 0;
    }

    void BeginTradeGold()
    {
        if (trade.visible && !trade.confirmed && !TradeItemLocked() && !trade.goldPending && !trade.gold[0] &&
            inventory.gold && !inventory.heldGold && !HeldItem()) trade.OpenPrompt(mir3_cui::TradePrompt::Gold);
    }

    void FinishTradePrompt(bool accept)
    {
        const auto prompt = std::exchange(trade.prompt,mir3_cui::TradePrompt::None);
        unsigned amount = 0;
        const auto end = trade.input+std::strlen(trade.input);
        const auto result = std::from_chars(trade.input,end,amount);
        const bool valid = accept && trade.visible && !trade.confirmed && result.ec==std::errc{} && result.ptr==end &&
            amount && output_.size()<8192;
        if (prompt == mir3_cui::TradePrompt::Quantity) {
            const auto* held = HeldItem();
            if (valid && trade.offerSlot>=0 && held && held->key==trade.offeredItem.key && amount<=held->count)
                SendTradeOffer(amount);
            else trade.offerSlot = -1;
        } else if (prompt == mir3_cui::TradePrompt::Gold && valid && !trade.gold[0] && !trade.goldPending && amount<=inventory.gold) {
            trade.goldPending = true;
            Queue(ClientTradeGold,0,"",amount);
        }
    }

    void ConfirmTrade()
    {
        if (!trade.visible || trade.confirmed || TradeItemLocked() || trade.goldPending || output_.size()>=8192) return;
        trade.confirmed = true;
        Queue(ClientTradeConfirm,0,"");
    }

    bool TradeItemLocked() const { return trade.offerSlot>=0 || trade.returnSlot>=0; }

    void CancelTradeReturnSelection()
    {
        if (!trade.returnPending) trade.returnSlot = -1;
    }

    void SetPartyAllowed(bool enabled)
    {
        if (stage != Stage::GameSession || showSignOut) return;
        if (output_.size() >= 8192) {
            party.error = "The connection is busy. Try changing the party setting again.";
            return;
        }
        Queue(mir3_cui::PartyMode, enabled ? 1 : 0, "");
        party.allowed = enabled;
        party.error.clear();
    }

    void OpenQuests()
    {
        if (stage != Stage::GameSession) return;
        quests.open = true;
        if (!quests.initialized && quests.entries.empty()) RequestQuests();
    }

    void OpenInventory()
    {
        if (stage != Stage::GameSession) return;
        inventory.open = true;
        if (!inventory.initialized && !inventory.waiting) RequestInventory();
    }

    void SetResourceRoot(const std::filesystem::path& root)
    {
        resourceRoot_ = root;
        inventoryDimensions_.SetResourceRoot(root);
        const auto read=[](const std::filesystem::path& path) {
            std::ifstream file(path,std::ios::binary|std::ios::ate);
            const auto size=file.tellg();
            if (size<=0 || size>16*1024*1024) return std::vector<std::uint8_t>{};
            std::vector<std::uint8_t> bytes(static_cast<std::size_t>(size));
            file.seekg(0);
            if (!file.read(reinterpret_cast<char*>(bytes.data()),size)) bytes.clear();
            return bytes;
        };
        effectCatalog_={};
        effectCatalog_.load(read(root/"MInfo.dat"));
        sceneMessages_={};
        sceneMessages_.load(read(root/"CmList.dat"));
        sceneSoundList_={};
        sceneSoundList_.parse(read(root/"SoundList.wwl"));
    }

    void LoadMapOptions(const wchar_t* path)
    {
        portraitMapMode = GetPrivateProfileIntW(L"Options",L"InformationMode",0,path) != 0;
        for (const auto& option : LocalOptions)
            this->*option.value = GetPrivateProfileIntW(L"Options",option.key,option.initial,path) != 0;
    }

    void ToggleMinimap(std::uint64_t cooldown)
    {
        if (stage != Stage::GameSession || output_.size() >= 8192) return;
        if (minimap.toggle(GetTickCount64(),cooldown)) RequestMinimap();
    }

    void TogglePortraitMap()
    {
        const auto now=GetTickCount64();
        if (stage != Stage::GameSession || (minimap.reply_time && now-minimap.reply_time<=1000)) return;
        if (!portraitMapMode && output_.size() >= 8192) return;
        portraitMapMode = !portraitMapMode;
        portraitMapPending = portraitMapMode;
        portraitMapReady = false;
        if (portraitMapPending) RequestMinimap();
        if (!audioOptionsPath.empty())
            WritePrivateProfileStringW(L"Options",L"InformationMode",portraitMapMode ? L"1" : L"0",
                audioOptionsPath.c_str());
    }

    void CloseNpcDialogue()
    {
        SecureZeroMemory(npcDialogue.answer, sizeof npcDialogue.answer);
        npcDialogue = {};
        shop = {};
        if (merchant.actor) CloseInventory();
    }

    void CloseInventory()
    {
        inventory.open = false;
        const int notice = merchant.notice;
        merchant = {};
        merchant.notice = notice;
    }

    bool BeginHeldDrop()
    {
        if (TradeItemLocked()) return false;
        if (equipment.heldSlot >= 0 || inventory.heldShortcut >= 0) return false;
        if (inventory.heldGold) {
            BeginInventoryDrop(mir3_cui::DropDialog::Gold,true);
            if (inventory.drop == mir3_cui::DropDialog::Gold) inventory.heldGold = false;
            return true;
        }
        const auto* item = inventory.Find(equipment.held);
        if (!item || !item->key.id || item->IsGold()) return false;
        inventory.selection = item->key;
        BeginInventoryDrop(mir3_cui::DropDialog::Item,true);
        if (inventory.drop != mir3_cui::DropDialog::None && (!item->Stacks() || item->count <= 1))
            SubmitInventoryDrop();
        return true;
    }

    void QuoteMerchantItem(int cell)
    {
        using Service = mir3_cui::MerchantService;
        if (!merchant.actor || !inventory.open || merchant.quantityOpen ||
            merchant.phase == Service::Phase::Quoting || merchant.phase == Service::Phase::Paying ||
            !InventoryCanQueue()) return;
        const auto* item = inventory.AtCell(cell);
        if (!item) return;
        merchant.item = item->key;
        merchant.price = 0;
        if (merchant.kind == Service::Kind::Deposit) {
            merchant.phase = Service::Phase::Quoted;
            return;
        }
        merchant.phase = Service::Phase::Quoting;
        Queue(merchant.kind == Service::Kind::Sell ? ClientQuoteSell : ClientQuoteRepair,
            static_cast<WORD>(item->key.id), item->key.name, merchant.actor, 0, static_cast<WORD>(item->key.id >> 16));
    }

    void ConfirmMerchantItem(unsigned quantity = 0)
    {
        using Service = mir3_cui::MerchantService;
        if (!merchant.actor || !inventory.open || merchant.phase != Service::Phase::Quoted ||
            !InventoryCanQueue()) return;
        const auto* item = inventory.Find(merchant.item);
        if (!item) {
            merchant.phase = Service::Phase::Choosing;
            merchant.price = 0;
            return;
        }
        const bool deposit = merchant.kind == Service::Kind::Deposit;
        const bool selling = merchant.kind == Service::Kind::Sell;
        if ((selling || deposit) && item->Stacks() && item->count > 1 && !quantity) {
            merchant.quantityOpen = true;
            merchant.quantity[0] = '\0';
            ++merchant.editor;
            return;
        }
        if (deposit && item->Stacks() && !quantity) quantity = 1;
        if (quantity > item->count) return;
        merchant.phase = Service::Phase::Paying;
        Queue(deposit ? ClientStorageDeposit : selling ? ClientSell : ClientRepair,
            static_cast<WORD>(item->key.id), item->key.name,
            merchant.actor, static_cast<WORD>(quantity), static_cast<WORD>(item->key.id >> 16));
    }

    void SubmitMerchantQuantity()
    {
        if (!std::exchange(merchant.quantityOpen, false)) return;
        const auto* item = inventory.Find(merchant.item);
        const long requested = ReadMerchantQuantity(merchant.quantity, item ? item->count : 0);
        if (requested > 0 && requested <= 65535) ConfirmMerchantItem(static_cast<unsigned>(requested));
    }

    void WithdrawStorage(unsigned quantity = 0)
    {
        if (stage != Stage::GameSession || !shop.storage || !shop.actor || shop.withdrawing ||
            shop.offer < 0 || shop.offer >= static_cast<int>(shop.offers.size()) || output_.size() >= 8192) return;
        const auto& item = shop.offers[shop.offer];
        if (item.stackMode >= 2 && item.count > 1 && !quantity) {
            shop.withdrawalPrompt = true;
            shop.quantity[0] = '\0';
            return;
        }
        if (item.stackMode >= 2 && !quantity) quantity = 1;
        if (quantity > item.count) return;
        shop.withdrawing = item.id;
        Queue(ClientStorageWithdraw, static_cast<WORD>(item.id), item.name, shop.actor,
            static_cast<WORD>(quantity), static_cast<WORD>(item.id >> 16));
    }

    void SubmitStorageQuantity()
    {
        if (!std::exchange(shop.withdrawalPrompt, false) || shop.offer < 0 ||
            shop.offer >= static_cast<int>(shop.offers.size())) return;
        const long requested = ReadMerchantQuantity(shop.quantity, shop.offers[shop.offer].count);
        if (requested > 0 && requested <= 65535) WithdrawStorage(static_cast<unsigned>(requested));
    }

    void PageStorage(int direction)
    {
        if (!shop.storage || shop.withdrawing) return;
        const int last = shop.offers.empty() ? 0 : static_cast<int>((shop.offers.size()-1)/12);
        shop.page = static_cast<unsigned>(std::clamp(static_cast<int>(shop.page)+direction,0,last));
        shop.offer = -1;
    }

    void RequestShopOffers(int category, unsigned page)
    {
        if (stage != Stage::GameSession || !shop.actor || shop.detailDeadline || purchase.deadline ||
            category < 0 || category >= static_cast<int>(shop.goods.size()) || page > 65535 / 10 ||
            output_.size() >= 8192) return;
        const auto& good = shop.goods[category];
        if (category != shop.category) {
            shop.offers.clear();
            shop.offer = -1;
        }
        shop.category = category;
        shop.page = page;
        shop.bulk = good.mode == 2;
        shop.detailed = !shop.bulk;
        shop.status.clear();
        shop.lastRow = -1;
        if (shop.bulk) {
            shop.offersVisible = false;
            shop.quantity[0] = '\0';
        } else {
            Queue(ClientShopOffers, static_cast<WORD>(page * 10), good.name, shop.actor);
            shop.detailDeadline = GetTickCount64() + 8000;
        }
    }

    void BuyShopItem()
    {
        if (stage != Stage::GameSession || !shop.actor || purchase.deadline || shop.detailDeadline ||
            output_.size() >= 8192) return;
        unsigned id = 0, count = 0, pendingOffer = 0;
        const std::string* name = nullptr;
        if (shop.bulk && shop.category >= 0) {
            const auto& good = shop.goods[shop.category];
            count = shop.Quantity();
            if (!count) {
                shop.status = "Enter a quantity from 1 to 65535.";
                return;
            }
            id = good.stock;
            name = &good.name;
        } else if (shop.detailed && shop.offer >= 0 && shop.offer < static_cast<int>(shop.offers.size())) {
            const auto& offer = shop.offers[shop.offer];
            id = pendingOffer = offer.id;
            name = &offer.name;
        }
        if (!name) return;
        Queue(ClientBuy, static_cast<WORD>(id), *name, shop.actor, static_cast<WORD>(count),
            static_cast<WORD>(id >> 16));
        purchase = {shop.actor, pendingOffer, shop.revision, GetTickCount64() + 8000};
        shop.status.clear();
    }

    bool ClickNpc(int x, int y)
    {
        const auto found = std::find_if(scene.players.begin(), scene.players.end(), [&](const auto& entry) {
            const auto& actor = entry.second;
            return actor.IsNpc() && !actor.dead && static_cast<int>(actor.x) == x && static_cast<int>(actor.y) == y;
        });
        return found != scene.players.end() && ClickNpcActor(found->first);
    }

    bool ClickNpcActor(std::uint32_t id)
    {
        const auto found = scene.players.find(id);
        if (found == scene.players.end() || !found->second.IsNpc() || found->second.dead) return false;
        const auto& npc = found->second;
        const auto now = GetTickCount64();
        if (stage == Stage::GameSession && !scene.dead && (!lastNpcClick || now - lastNpcClick > 1500) &&
            std::abs(static_cast<int>(scene.x) - static_cast<int>(npc.x)) <= 2 &&
            std::abs(static_cast<int>(scene.y) - static_cast<int>(npc.y)) <= 2 &&
            output_.size() < 8192) {
            Queue(ClientClickNpc, 0, "", id);
            lastNpcClick = now;
        }
        return true;
    }

    void RequestMonsterAttributes(std::uint32_t id, LocalScene::Player& monster)
    {
        if (stage != Stage::GameSession || monster.monsterAttributes.icon != -1 || output_.size() >= 8192) return;
        Queue(ClientTargetAttributes, static_cast<WORD>(monster.x), "", id, 0, static_cast<WORD>(monster.y));
        monster.monsterAttributesRequested = true;
    }

    void ChooseNpcOption(const mir3_cui::NpcDialogue::Row& option)
    {
        const auto now = GetTickCount64();
        if (stage != Stage::GameSession || !npcDialogue.actor || option.command.empty() ||
            (lastNpcAction && now - lastNpcAction <= 1000) || output_.size() >= 8192) return;
        const auto command = ConvertText(option.command, CP_UTF8, 936);
        if (command.empty()) return;
        lastNpcAction = now;
        if (option.command.compare(0, 2, "@@") == 0) {
            npcDialogue.prompt = option;
            npcDialogue.answer[0] = '\0';
            npcDialogue.answerTooLong = false;
            ++npcDialogue.editor;
        } else {
            Queue(ClientNpcSelect, 0, command, npcDialogue.actor);
        }
    }

    void EndNpcPrompt(bool submit)
    {
        if (submit) {
            if (stage != Stage::GameSession || output_.size() >= 8192 || !npcDialogue.actor) return;
            const auto& option = npcDialogue.prompt.command;
            const auto command = ConvertText(option, CP_UTF8, 936);
            const auto answer = ConvertText(npcDialogue.answer, CP_UTF8, 936);
            if (answer.size() > NpcAnswerLimit() || std::strpbrk(npcDialogue.answer, "\r\n") ||
                (npcDialogue.answer[0] && answer.empty())) {
                npcDialogue.answerTooLong = true;
                return;
            }
            Queue(ClientNpcSelect, 0, command + '\r' + answer, npcDialogue.actor);
        }
        SecureZeroMemory(npcDialogue.answer, sizeof npcDialogue.answer);
        npcDialogue.prompt = {};
        npcDialogue.answerTooLong = false;
    }

    unsigned NpcAnswerLimit() const
    {
        const auto& option = npcDialogue.prompt.command;
        if (option.starts_with("@@*") &&
            (option.find("enter_stgpasswd") != option.npos || option.find("Input_stgpasswd") != option.npos ||
             option.find("Change_stgpasswd") != option.npos || option.find("confirm_passwd") != option.npos)) {
            const auto delimiter = option.find('_', 3);
            const unsigned limit = delimiter >= 10 ? 10 : Number(option.substr(3, delimiter - 3), 90);
            if (limit) return limit;
        }
        return 90;
    }

    void CrouchInScene(int targetX, int targetY, std::uint32_t corpse)
    {
        const auto now=GetTickCount64();
        if (stage!=Stage::GameSession || showSignOut || !scene.terrain.Ready() || scene.pending.deadline || scene.pendingMelee ||
            scene.ActionInputBlocked() || scene.crouchReplyAt || CrouchPose(scene.crouchedAt,now)>=0 || mount.shape || !characterStatus.hp[0] ||
            output_.size()>8192-2*(DEFBLOCKSIZE+3)) return;
        const int x=int(scene.x), y=int(scene.y), dx=targetX-x, dy=targetY-y;
        const std::array direction{std::abs(dy)>2 && std::abs(dx)<=1 ? 0 : (dx>0)-(dx<0),
                                   std::abs(dx)>2 && std::abs(dy)<=1 ? 0 : (dy>0)-(dy<0)};
        if (const auto heading=std::find(SceneDirections.begin(),SceneDirections.end(),direction);heading!=SceneDirections.end())
            scene.facing=static_cast<unsigned>(heading-SceneDirections.begin());
        scene.crouchedAt=now;
        scene.movementOrigin.reset();
        scene.crouchReplyAt=now+8000;
        Queue(ClientCrouch,0,"",scene.x|(std::uint32_t(scene.y)<<16),0,static_cast<WORD>(scene.facing));
        if (const auto found=scene.players.find(corpse);found!=scene.players.end() && scene.CanButcher(found->second))
            Queue(ClientButcher,static_cast<WORD>(scene.x),"",corpse,static_cast<WORD>(scene.facing),static_cast<WORD>(scene.y));
    }

    void RequestSpell(int slot, int x, int y, std::uint32_t hoveredActor, bool repeat)
    {
        const auto tick=GetTickCount64();
        if (stage!=Stage::GameSession || showSignOut || !scene.terrain.Ready() || mount.shape ||
            (characterStatus.hp[1] && !characterStatus.hp[0]) || scene.pending.deadline || scene.pendingMelee || minimap.visible ||
            scene.ActionInputBlocked() || scene.InputDelayed(tick) || output_.size()>=8192 || slot<1 || slot>12) return;
        auto& timing=scene.spellTiming;
        const auto now=static_cast<std::uint32_t>(tick);
        if (!timing.key_ready(now,slot,repeat)) return;
        const auto* skill=skills.find(skills.bindings[slot]);
        if (!skill) return;
        using Action=mir3::SpellAction;
        using Direct=mir3::DirectSpell;
        const auto& route=mir3::SpellRoutes::get(skill->id);
        if (!route.bypass_mana && characterStatus.mp[0]<unsigned(skill->mana_threshold)+10) {
            const auto notice=ConvertText(sceneMessages_.get(1000),936,CP_UTF8);
            chat.Append(100,notice.empty() ? "Not enough mana." : notice,11861940);
            return;
        }
        auto found=scene.players.find(hoveredActor);
        if (found!=scene.players.end() && found->second.IsNpc()) found=scene.players.end();
        if (found!=scene.players.end()) {
            const auto& actor=found->second;
            if (actor.IsMonster() || (actor.identity.color==27631 && (!actor.dead || actor.appearance.body==1)))
                scene.lockedSpellActor=found->first;
        } else if (UsesLockedSpellTarget(skill->id)) found=scene.players.find(scene.lockedSpellActor);
        std::uint32_t target=0;
        int targetType=-1;
        if (found!=scene.players.end()) {
            x=int(found->second.x); y=int(found->second.y);
            target=found->first;
            targetType=found->second.IsMonster() ? 3 : int(found->second.appearance.body);
        }
        const auto facing=SpellFacing(int(scene.x),int(scene.y),x,y);
        auto followup=target;
        bool send=false,predict=false;
        if (!scene.spellReplyUntil && route.action!=Action::none) {
            if (route.action==Action::rush || route.action==Action::special) {
                auto& cooldown=route.action==Action::rush ? timing.rush_at : timing.special_at;
                if (mir3::SpellTiming::elapsed_after(now,cooldown,3000)) {
                    cooldown=now; send=predict=true; x=int(facing); y=0; target=0;
                }
            } else {
                timing.begin_animated(now,skill->casting_delay);
                if (route.self_target) {
                    x=int(scene.x); y=int(scene.y);
                    followup=target=localActor_;
                }
                send=predict=true;
            }
        }
        std::uint32_t* directTick=nullptr, interval=0;
        switch (route.direct) {
        case Direct::rapid: directTick=&timing.cast_at; interval=200; break;
        case Direct::predicted: directTick=&timing.cast_at; interval=1000; break;
        case Direct::fire_sword: directTick=&timing.fire_sword_at; interval=10000; break;
        case Direct::skill35: directTick=&timing.skill35_at; interval=10000; break;
        case Direct::skill34: directTick=&timing.skill34_at; interval=10000; break;
        case Direct::none: break;
        }
        if (directTick && mir3::SpellTiming::elapsed_after(now,*directTick,interval)) {
            *directTick=now; send=true; x=y=0;
            predict=route.direct==Direct::predicted;
            if (route.direct==Direct::rapid) timing.casting_delay=0;
        }
        if (send) {
            if (predict) {
                scene.movementOrigin.reset();
                scene.spellOriginalFacing=scene.facing;
                scene.spellReplyUntil=tick+8000;
                if (route.action!=Action::none && route.action!=Action::rush) {
                    scene.facing=facing;
                    scene.levelPoseAt=scene.crouchedAt=scene.crouchReplyAt=0;
                    StartCast(scene.casting,skill->id,route.action,0,scene.x,scene.y,facing,tick);
                }
            }
            Queue(ClientSpell,WORD(target),std::to_string(std::bit_cast<std::int32_t>(now)),
                WORD(x)|(std::uint32_t(WORD(y))<<16),WORD(target>>16),WORD(skill->id));
        }
        if (route.restrict_followup) timing.restricted_at=now;
        if (followup && targetType>=0 && targetType<=1) {
            const auto first=static_cast<std::uint32_t>(std::rand()), second=static_cast<std::uint32_t>(std::rand());
            timing.finish_target(followup,targetType,first,second);
        } else timing.movement_delay=0;
    }

    bool MeleeInScene(int x,int y,std::uint32_t target,bool force)
    {
        using Attack=mir3::MeleeSpells;
        const auto weapon=characterAppearance.weapon;
        if (weapon>=34 && weapon<=37) return false;
        if (target) {
            const auto& actor=scene.players.at(target);
            if (actor.IsMonster()) {
                const auto shape=actor.npcShape;
                const auto low=shape&255;
                if (low==26 || low==86 || (low>=113 && low<=115) || low==165 || low==166 ||
                    (shape>=530 && shape<=533 && actor.facing==7)) return true;
            } else if (!force && !forceAttack && actor.identity.color!=255 && actor.identity.color!=27631) return true;
            x=int(actor.x); y=int(actor.y);
        }
        const bool same=x==int(scene.x) && y==int(scene.y);
        const unsigned heading=same ? scene.facing : SpellFacing(int(scene.x),int(scene.y),x,y);
        const bool adjacent=target && std::max(std::abs(x-int(scene.x)),std::abs(y-int(scene.y)))==1;
        const auto blocked=[&](unsigned face) {
            const auto [dx,dy]=SceneDirections[face];
            const int col=int(scene.x)+dx,row=int(scene.y)+dy;
            const auto ground=scene.GroundAt(col,row);
            return (ground!=mir3_cui::GroundCell::Walkable && ground!=mir3_cui::GroundCell::DoorOpen) ||
                scene.Occupied(col,row);
        };
        const bool mining=!target && weapon==7 && (force || (!same && blocked(heading) && blocked(scene.facing)));
        if (!adjacent && !mining && !(force && (target || weapon!=7))) return false;
        const auto now=GetTickCount64();
        if (stage!=Stage::GameSession || showSignOut || scene.InputDelayed(now) || !scene.terrain.Ready() ||
            mount.shape || !characterStatus.hp[0] || scene.pending.deadline || scene.pendingMelee ||
            scene.spellReplyUntil || scene.ActionInputBlocked() || scene.crouchReplyAt || CrouchPose(scene.crouchedAt,now)>=0 ||
            (!scene.casting.clock.finished() && scene.casting.clock.action()!=7) || output_.size()>=8192) return true;
        const auto tick=static_cast<std::uint32_t>(now);
        const auto& hand=characterStatus.hand;
        const auto level=characterStatus.level;
        if (!scene.meleeCooldown.accept(tick,level,hand[0],hand[1])) return true;
        const auto interval=mir3::MeleeCooldown::interval(level,scene.meleeCooldown.speed,hand[0],hand[1]);
        scene.pendingMelee=LocalScene::PendingMelee{scene.x,scene.y,mining && !force ? scene.facing : heading,
            mining ? Attack::forcehit : Attack::hit, now+scene.actionClock.shared_wait(tick,interval)};
        AdvanceMelee(now);
        return true;
    }

    void MoveInScene(int targetX, int targetY, bool run, bool approachActor=false, bool firstRightPress=false)
    {
        using mir3_cui::GroundCell;
        const auto now = GetTickCount64();
        if (stage != Stage::GameSession || showSignOut || !scene.terrain.Ready() || scene.pending.deadline || scene.spellReplyUntil ||
            scene.pendingMelee || scene.ActionInputBlocked() || scene.crouchReplyAt || CrouchPose(scene.crouchedAt,now)>=0 ||
            (!scene.casting.clock.finished() && scene.casting.clock.action()!=7) ||
            (scene.spellTiming.cast_at && !scene.spellTiming.movement_ready(static_cast<std::uint32_t>(now))) ||
            output_.size() >= 8192 || (characterStatus.hp[1] && !characterStatus.hp[0])) return;
        if (!scene.terrain.Contains(targetX,targetY)) return;
        const int x = static_cast<int>(scene.x), y = static_cast<int>(scene.y);
        if (targetX == x && targetY == y) {
            if (run) return;
            const auto& items=scene.groundItems.items;
            const auto item=std::find_if(items.begin(),items.end(),[&](const auto& entry) { return entry.x==x && entry.y==y; });
            if (item==items.end()) return;
            if (inventory.items.size()>=mir3_cui::InventoryPanel::Capacity) {
                const bool fits=item->stack_mode>=2 ? std::any_of(inventory.items.begin(),inventory.items.end(),[&](const auto& entry) {
                    return entry.groundLooks==unsigned(item->looks);
                }) : item->name.starts_with("\xbd\xf0\xb1\xd2");
                if (!fits) { scene.pickupBagFull=true; return; }
            }
            Queue(ClientPickup,static_cast<WORD>(x),"",0,0,static_cast<WORD>(y));
            return;
        }
        const auto [dx,dy]=SceneDirections[SpellFacing(x,y,targetX,targetY)];
        const auto heading = [&](int stepX, int stepY) {
            const std::array<int, 2> step{stepX, stepY};
            return static_cast<unsigned>(std::find(SceneDirections.begin(), SceneDirections.end(), step) -
                SceneDirections.begin());
        };
        if (run && std::abs(targetX - x) <= 1 && std::abs(targetY - y) <= 1) {
            scene.facing = heading(dx, dy);
            return;
        }
        if (scene.lastMove && now - scene.lastMove < 600) return;
        const auto walkable = [&](int cellX, int cellY) {
            const auto ground = scene.GroundAt(cellX, cellY);
            return (ground == GroundCell::Walkable || ground == GroundCell::DoorOpen) &&
                !scene.Occupied(cellX, cellY);
        };
        int nextX = x + dx, nextY = y + dy;
        if (RequestSceneDoor(nextX,nextY)) { scene.lastMove=now; return; }
        if (walkable(nextX, nextY)) {
            if (run && !firstRightPress && characterStatus.hp[0]>=10) {
                RequestSceneDoor(nextX+dx,nextY+dy);
                if (walkable(nextX+dx,nextY+dy) && (!mount.shape || walkable(nextX+2*dx,nextY+2*dy))) {
                    const int more=mount.shape ? 2 : 1;
                    nextX+=more*dx;
                    nextY+=more*dy;
                }
            }
        } else {
            if (approachActor) return;
            nextX = x;
            nextY = y;
            int nearest = 100;
            for (const auto& direction : SceneDirections) {
                const int candidateX = x + direction[0], candidateY = y + direction[1];
                if (!walkable(candidateX, candidateY)) continue;
                const int distanceX = candidateX - targetX, distanceY = candidateY - targetY;
                const int distance = int(std::hypot(distanceX,distanceY));
                if (distance > nearest) continue;
                nearest = distance;
                nextX = candidateX;
                nextY = candidateY;
            }
        }
        if (nextX == x && nextY == y) return;
        const unsigned facing = heading((nextX > x) - (nextX < x), (nextY > y) - (nextY < y));
        const auto tick=static_cast<std::uint32_t>(now);
        const auto& hand=characterStatus.hand;
        const auto interval=mir3::MeleeCooldown::interval(characterStatus.level,scene.meleeCooldown.speed,hand[0],hand[1]);
        scene.pendingWalk=LocalScene::WalkRequest{unsigned(nextX),unsigned(nextY),facing,scene.x,scene.y,tick,
            now+scene.actionClock.shared_wait(tick,interval)};
        scene.lastMove=now;
        AdvanceWalk(now);
    }

    void RequestInventory()
    {
        if (!InventoryCanQueue()) return;
        Queue(81, 0, "");
        inventory.waiting = true;
    }

    void UseInventoryItem()
    {
        const auto* item = inventory.Find(inventory.selection);
        if (item) UseItem(*item);
    }

    void ReleaseBagCell(int cell, bool rightHalf = false, bool bottomHalf = false)
    {
        if (trade.offerSlot>=0) return;
        if (const auto* item = trade.SelectedReturn()) {
            if (!trade.returnPending && output_.size()<8192 && !merchant.actor) {
                Queue(ClientTradeReturn,0,item->key.name,item->key.id);
                trade.returnPending = true;
            }
            return;
        }
        if (inventory.request != mir3_cui::ItemRequest::None ||
            equipment.request != mir3_cui::EquipmentRequest::None) return;
        if (inventory.heldShortcut >= 0) {
            inventory.heldShortcut = -1;
            equipment.ReleaseHeld();
            return;
        }
        auto* target = inventory.AtCell(cell);
        const auto placement = [&](const mir3_cui::InventoryItem& item) {
            constexpr int columns = mir3_cui::InventoryPanel::Columns;
            const int x = cell % columns - item.width / 2 + (item.width % 2 == 0 && rightHalf);
            const int y = cell / columns - item.height / 2 + (item.height % 2 == 0 && bottomHalf);
            return x < 0 || x >= columns || y < 0 ? -1 : y * columns + x;
        };
        if (equipment.heldSlot >= 0) {
            const auto* item = HeldItem();
            if (!item) return;
            const int anchor = mir3_cui::InventoryPanel::FirstFit(inventory.Occupied(), *item, placement(*item));
            if (anchor < 0)
                equipment.status = "There is no room for this item.";
            else UnequipItem(anchor);
            return;
        }
        const auto now = GetTickCount64();
        auto* source = inventory.Find(equipment.held);
        if (!source) {
            if (!target) return;
            inventory.selection = target->key;
            equipment.held = target->key;
            equipment.lastRelease = now;
            return;
        }
        if (source == target && now - equipment.lastRelease < 300 && source->key.id) {
            UseItem(*source);
            return;
        }
        const int origin = source->cell;
        const int anchor = target == source ? origin : placement(*source);
        if (!inventory.Move(*source, target == source ? nullptr : target, anchor)) {
            inventory.status = "The item does not fit there.";
            return;
        }
        inventory.status.clear();
        if (target && target != source) {
            equipment.held = target->key;
            inventory.selection = target->key;
            equipment.lastRelease = now;
        } else {
            inventory.selection = source->key;
            equipment.ReleaseHeld();
        }
        if (origin != source->cell || (target && target != source)) SaveInventoryLayout();
    }

    void OpenEquipment(bool fromInventory = false)
    {
        if (stage != Stage::GameSession) return;
        equipment.open = true;
        const auto* item = inventory.Find(inventory.selection);
        if (fromInventory && item && equipment.heldSlot < 0) {
            for (int slot = 0; slot < mir3_cui::EquipmentSlots; ++slot) {
                if (mir3_cui::EquipmentFits(item->mode, slot)) {
                    equipment.selected = slot;
                    equipment.held = item->key;
                    break;
                }
            }
        }
    }

    void EquipItem()
    {
        if (!EquipmentCanQueue()) return;
        if (equipment.heldSlot >= 0) return;
        const auto* item = inventory.Find(equipment.held.name.empty() ? inventory.selection : equipment.held);
        if (!item || !mir3_cui::EquipmentFits(item->mode, equipment.selected)) return;
        Queue(1003, static_cast<WORD>(equipment.selected), item->key.name, item->key.id);
        equipment.pending = *item;
        equipment.pendingSlot = equipment.selected;
        equipment.previous = equipment.slots[equipment.selected].key;
        equipment.request = mir3_cui::EquipmentRequest::Wear;
        equipment.deadline = GetTickCount64() + 10000;
    }

    void ReleaseEquipmentSlot(int slot)
    {
        equipment.selected = slot;
        if (!EquipmentCanQueue()) return;
        const auto now = GetTickCount64();
        auto& target = equipment.slots[slot];
        if (equipment.held.name.empty()) {
            if (target.key.name.empty()) return;
            equipment.held = target.key;
            equipment.heldSlot = slot;
            equipment.lastRelease = now;
            return;
        }
        if (equipment.heldSlot < 0) {
            const auto* item = inventory.Find(equipment.held);
            if (!item || !mir3_cui::EquipmentFits(item->mode, slot)) {
                equipment.status = "This item does not fit that slot.";
                return;
            }
            EquipItem();
            return;
        }
        auto& source = equipment.slots[equipment.heldSlot];
        if (!(source.key == equipment.held)) {
            equipment.ReleaseHeld();
            return;
        }
        const bool quick = now - equipment.lastRelease < 300;
        equipment.lastRelease = now;
        if (!quick || !mir3_cui::EquipmentActivates(source, slot)) {
            if (slot == equipment.heldSlot ||
                (target.key.name.empty() && mir3_cui::EquipmentFits(source.mode, slot))) equipment.ReleaseHeld();
            return;
        }
        if (slot != equipment.heldSlot) {
            auto* destination = &target;
            if (!target.key.name.empty()) {
                const auto free = std::find_if(equipment.slots.begin(), equipment.slots.end(),
                    [](const mir3_cui::InventoryItem& item) { return item.key.name.empty(); });
                if (free == equipment.slots.end()) return;
                destination = &*free;
            }
            *destination = std::move(source);
            source = {};
        }
        equipment.ReleaseHeld();
        Queue(1124, 0, "", slot);
    }

    void UnequipItem(int cell = -1)
    {
        if (!EquipmentCanQueue()) return;
        const int slot = equipment.heldSlot >= 0 ? equipment.heldSlot : equipment.selected;
        const auto& item = equipment.slots[slot];
        if (item.key.name.empty()) return;
        if (inventory.initialized && inventory.items.size() >= mir3_cui::InventoryPanel::Capacity) {
            equipment.status = "Free a bag slot first.";
            return;
        }
        Queue(1004, static_cast<WORD>(slot), item.key.name, item.key.id);
        equipment.pending = item;
        equipment.pendingSlot = slot;
        equipment.request = mir3_cui::EquipmentRequest::Remove;
        equipment.deadline = GetTickCount64() + 10000;
        inventory.returningEquipment = item.key;
        inventory.returnCell = cell;
    }

    const mir3_cui::InventoryItem* HeldItem()
    {
        if (equipment.held.name.empty()) return nullptr;
        if (equipment.heldSlot < 0) return inventory.Find(equipment.held);
        const auto& item = equipment.slots[equipment.heldSlot];
        return item.key == equipment.held ? &item : nullptr;
    }

    void ActivateEquipment()
    {
        if (!EquipmentCanQueue()) return;
        if ((!HeldItem() || equipment.heldSlot == equipment.selected) &&
            mir3_cui::EquipmentActivates(equipment.slots[equipment.selected], equipment.selected)) {
            equipment.ReleaseHeld();
            Queue(1124, 0, "", equipment.selected);
        }
    }

    void ToggleItemShortcuts()
    {
        inventory.shortcutsVisible = !inventory.shortcutsVisible;
        inventory.clickedShortcut = -1;
        if (inventory.shortcutsVisible && !inventory.initialized && !inventory.waiting) RequestInventory();
    }

    void UseItemShortcut(std::size_t slot)
    {
        const auto* item = inventory.ResolveShortcut(slot);
        if (!item) {
            inventory.shortcutStatus = "No usable item is bound to that slot.";
            if (!inventory.shortcuts[slot].name.empty() && !inventory.initialized && !inventory.waiting)
                RequestInventory();
            return;
        }
        UseItem(*item);
        inventory.shortcutStatus = inventory.status;
    }

    void ReleaseShortcutSlot(std::size_t slot)
    {
        if (trade.returnSlot>=0) { CancelTradeReturnSelection(); return; }
        if (!InventoryCanQueue() || equipment.heldSlot >= 0 || inventory.heldGold) return;
        const auto* target = inventory.ResolveShortcut(slot);
        const auto* held = HeldItem();
        if (!held) {
            if (!target) return;
            const auto key = target->key;
            inventory.shortcuts[slot] = {};
            equipment.held = key;
            inventory.heldShortcut = static_cast<int>(slot);
        } else if (held->QuickEligible()) {
            const auto previous = target ? target->key : mir3_cui::ItemKey{};
            if (previous.name == held->key.name) {
                merchantResults.push_back({1069,held->label,static_cast<unsigned>(slot+1)});
                return;
            }
            if (!target && std::any_of(inventory.shortcuts.begin(),inventory.shortcuts.end(),
                [&](const auto& key) { return key.name == held->key.name; })) {
                merchantResults.push_back({1067,held->label,static_cast<unsigned>(slot+1)});
                return;
            }
            if (inventory.heldShortcut < 0 && !target)
                merchantResults.push_back({1068,held->label,static_cast<unsigned>(slot+1)});
            inventory.shortcuts[slot] = held->key;
            equipment.ReleaseHeld();
            equipment.held = previous;
            inventory.heldShortcut = target ? static_cast<int>(slot) : -1;
        }
        SaveItemShortcuts();
    }

    void ActivateShortcutSlot(std::size_t slot, bool mouse)
    {
        if (!InventoryCanQueue() || equipment.heldSlot >= 0 || inventory.heldGold) return;
        const auto* target = mouse ? HeldItem() : inventory.ResolveShortcut(slot);
        const auto now = GetTickCount64();
        const bool fast = target && target->mode == 0 &&
            (target->icon == 20 || target->icon == 21 || target->icon == 70);
        if (inventory.lastUse && now-inventory.lastUse <= (fast ? 1000u : 2000u)) {
            merchantResults.push_back({target ? 1051 : 1066,"",static_cast<unsigned>(slot+1)});
            return;
        }
        if (!mouse) {
            const auto* held = HeldItem();
            if (!target && held && held->QuickEligible())
                for (auto& key : inventory.shortcuts)
                    if (key.name == held->key.name) key = {};
            ReleaseShortcutSlot(slot);
        }
        if (const auto* held = HeldItem(); held && held->QuickEligible() && inventory.heldShortcut >= 0) {
            const int source = inventory.heldShortcut;
            if (UseItem(*held)) {
                inventory.pendingShortcut = source;
                inventory.heldShortcut = -1;
            }
        } else merchantResults.push_back({1066,"",static_cast<unsigned>(slot+1)});
    }

    void BindItemShortcut(std::size_t slot)
    {
        if (stage != Stage::GameSession || inventory.request != mir3_cui::ItemRequest::None) return;
        const auto* selected = inventory.Find(inventory.selection);
        if (!selected || !selected->QuickEligible() || selected->key.name.find_first_of("\r\n") != std::string::npos)
            return;
        for (auto& key : inventory.shortcuts) if (key.name == selected->key.name) key = {};
        inventory.shortcuts[slot] = selected->key;
        inventory.shortcutsVisible = true;
        inventory.bindingShortcut = false;
        inventory.clickedShortcut = -1;
        inventory.shortcutStatus.clear();
        SaveItemShortcuts();
    }

    void ClearItemShortcut(std::size_t slot)
    {
        if (stage != Stage::GameSession || inventory.request != mir3_cui::ItemRequest::None ||
            inventory.shortcuts[slot].name.empty()) return;
        inventory.shortcuts[slot] = {};
        inventory.clickedShortcut = -1;
        inventory.shortcutStatus.clear();
        SaveItemShortcuts();
    }

    void BeginInventoryDrop(mir3_cui::DropDialog drop, bool sceneDrop = false)
    {
        if (!InventoryCanQueue()) return;
        inventory.drop = drop;
        inventory.sceneDrop = sceneDrop;
        inventory.amount[0] = '\0';
        ++inventory.editor;
    }

    void SubmitInventoryDrop()
    {
        using mir3_cui::DropDialog;
        using mir3_cui::ItemRequest;
        if (!InventoryCanQueue()) return;
        const auto drop = inventory.drop;
        if (inventory.sceneDrop) inventory.drop = DropDialog::None;
        if (drop == DropDialog::Gold) {
            const auto parsed = std::strtol(inventory.amount,nullptr,10);
            const auto amount = inventory.sceneDrop
                ? (parsed > 0 ? static_cast<unsigned>(parsed) : 0u) : Number(inventory.amount, inventory.gold);
            if (!amount || amount >= inventory.gold) {
                inventory.status = "Enter a positive amount below your gold balance.";
                return;
            }
            Queue(1016, 0, "", amount);
            inventory.request = ItemRequest::DropGold;
        } else if (drop == DropDialog::Item) {
            const auto* item = inventory.Find(inventory.selection);
            if (!item) {
                inventory.status = "The item is no longer in the bag.";
                inventory.drop = DropDialog::None;
                return;
            }
            inventory.pending = *item;
            if (item->Stacks() && item->count > 1) {
                const std::string text = inventory.amount;
                const long asked = text.empty() || text == "*" ? static_cast<long>(item->count) :
                    inventory.sceneDrop ? std::strtol(inventory.amount,nullptr,10) : Number(text, 65535);
                const auto quantity = (std::min)(asked,100L);
                if (quantity <= 0 || static_cast<unsigned>(quantity) > item->count) {
                    inventory.status = "Enter a quantity available in the stack.";
                    return;
                }
                inventory.dropCount = quantity;
                Queue(1101, 0, item->key.name, item->key.id, static_cast<WORD>(quantity));
                inventory.request = ItemRequest::DropStack;
            } else {
                const DWORD tick = GetTickCount();
                Queue(1001, static_cast<WORD>(tick), item->key.name, item->key.id, static_cast<WORD>(tick >> 16));
                inventory.request = ItemRequest::Drop;
            }
        } else {
            return;
        }
        if (inventory.sceneDrop && drop == DropDialog::Item) equipment.ReleaseHeld();
        inventory.drop = DropDialog::None;
        inventory.deadline = GetTickCount64() + 10000;
    }

    void RequestQuests()
    {
        if (!QuestCanQueue()) return;
        Queue(mir3_cui::QuestQuery, 0, "");
        quests.waiting = true;
    }

    void SelectQuest(int id, int child = -1)
    {
        quests.parent = id;
        quests.child = child;
        quests.resetDescription = true;
        const auto* summary = quests.Description();
        if (child >= 0 && child <= 65535 && id <= 65535 && summary && summary->empty() && QuestCanQueue())
            Queue(mir3_cui::QuestTextQuery, 0, "", child, static_cast<WORD>(id));
    }

    void ExpandQuest(mir3_cui::QuestEntry& entry, bool toggle = true)
    {
        const int id = entry.record.id;
        quests.expanded = toggle && quests.expanded == id ? -1 : id;
        if (quests.expanded < 0) return;
        if (quests.parent != id || quests.child >= 0) SelectQuest(id);
        if (!entry.childrenLoaded && id <= 65535 && QuestCanQueue())
            Queue(mir3_cui::QuestChildrenQuery, 0, "", 0, static_cast<WORD>(id));
    }

    void RequestGuildPage(int page)
    {
        if (stage != Stage::GameSession || (page != 2 && !GuildCanQueue())) return;
        guild.tab = page;
        guild.first = 0;
        if (page == 0) {
            Queue(mir3_cui::GuildHome, 0, "");
            guild.requested = !guild.open;
        } else if (page == 1) {
            Queue(mir3_cui::GuildMembers, 0, "");
        }
    }

    void BeginGuildAction(mir3_cui::GuildAction action)
    {
        if (!guild.overview.manager) {
            guild.notice = 4000;
            return;
        }
        guild.Begin(action);
        if (!guild.Multiline()) return;
        const auto& rows = action == mir3_cui::GuildAction::Notice ? guild.overview.editable : guild.roster.editable;
        if (action == mir3_cui::GuildAction::Ranks && !guild.roster.received) {
            guild.Begin(mir3_cui::GuildAction::None);
            guild.error = "Load the member list before editing ranks.";
            return;
        }
        std::string text;
        bool first = true;
        for (const auto& row : rows) {
            const auto line = ConvertText(row, 936, CP_UTF8);
            if (text.size() + line.size() + 1 >= sizeof guild.input) {
                guild.error = "This text is too large to edit.";
                return;
            }
            if (!first) text += '\n';
            text += line;
            first = false;
        }
        strcpy_s(guild.input, text.c_str());
    }

    void SubmitGuildAction()
    {
        using mir3_cui::GuildAction;
        if (stage != Stage::GameSession || !guild.overview.manager || guild.action == GuildAction::None ||
            !GuildCanQueue())
            return;
        const auto raw = ConvertText(guild.input, CP_UTF8, 936);
        std::string body;
        if (guild.Multiline()) {
            bool emptyLine = true;
            for (std::size_t i = 0; i < raw.size(); ++i) {
                const char ch = raw[i];
                if (ch == '\r' && i + 1 < raw.size() && raw[i + 1] == '\n') continue;
                if (ch == '\r' || ch == '\n') {
                    if (emptyLine) body += ' ';
                    body += '\r';
                    emptyLine = true;
                } else {
                    body += ch;
                    emptyLine = false;
                }
            }
            if (body.empty() || body.size() > 3999) {
                guild.error = "Enter text that fits within 3999 GBK bytes.";
                return;
            }
        } else {
            body = raw;
            const std::size_t limit = guild.action == GuildAction::BreakAlliance ? 63 : 31;
            if (body.empty() || body.size() > limit || body.find_first_of("/\r\n") != std::string::npos) {
                guild.error = "Enter a name of up to " + std::to_string(limit) + " GBK bytes without a slash.";
                return;
            }
        }

        if (guild.action == GuildAction::BreakAlliance) {
            Queue(mir3_cui::Say, 0, "\xc6\xc6\xbb\xb5\xbd\xe1\xc3\xcb " + body);
            Queue(mir3_cui::GuildHome, 0, "");
        } else {
            Queue(static_cast<WORD>(guild.action), 0, body);
            if (guild.action == GuildAction::Remove) Queue(mir3_cui::GuildMembers, 0, "");
        }
        guild.Begin(GuildAction::None);
    }

    void SubmitPartyMember()
    {
        if (stage != Stage::GameSession || party.action == mir3_cui::PartyAction::None) return;
        const auto name = ConvertText(party.name, CP_UTF8, 936);
        if (name.empty() || name.size() > 31 || name.find_first_of("/\r\n") != std::string::npos) {
            party.error = "Enter a character name of up to 31 GBK bytes without a slash.";
            return;
        }
        if (output_.size() >= 8192) {
            party.error = "The connection is busy. Try submitting the name again.";
            return;
        }
        Queue(static_cast<WORD>(party.action), 0, name);
        party.Begin(mir3_cui::PartyAction::None);
    }

    bool SaveAudioSettings()
    {
        if (!audioDirty) return true;
        if (!audioOptionsPath.empty() && audioOptions.Save(audioOptionsPath.c_str())) {
            audioSaveError.clear();
            audioDirty = false;
            return true;
        }
        audioSaveError = "Could not save audio settings.";
        return false;
    }

    void Poll()
    {
        const auto now = GetTickCount64();
        if (stage==Stage::GameSession) {
            AdvanceMonsterActions(now);
            AdvanceGroundItems(static_cast<std::uint32_t>(now));
            if (scene.spellReplyUntil && now>=scene.spellReplyUntil) scene.spellReplyUntil=0;
            AdvanceMelee(now);
            AdvanceWalk(now);
            AdvanceHumanAction(scene.casting,0,scene.x,scene.y,characterAppearance,now);
            AdvanceLocalLife(now);
            const bool remoteMovementTick=now>=scene.remoteMovementAt;
            if (remoteMovementTick) scene.remoteMovementAt=now+LocalScene::LocalStep::FrameMs;
            for (auto& [id,actor]:scene.players)
                if (remoteMovementTick && actor.remoteStep.frame>=0) AdvanceRemoteMovement(actor,id,now);
            AdvanceSpellEffects(now);
            for (auto& [id,actor]:scene.players) {
                const bool wasActing=!actor.casting.clock.finished();
                const int previousFrame=actor.casting.clock.frame();
                AdvanceHumanAction(actor.casting,id,actor.x,actor.y,actor.appearance,now);
                if (actor.dying) {
                    if (!previousFrame && actor.casting.clock.frame()==1)
                        PlaySceneSound(id,144+actor.appearance.body,actor.x,actor.y);
                    if (actor.casting.clock.frame()>=9) {
                        actor.dying=false;
                        actor.casting={};
                    }
                    continue;
                }
                if (wasActing && actor.casting.clock.finished()) actor.combatIdleAt=now;
                if (actor.remoteStep.frame>=0 || !actor.casting.clock.finished() || actor.remoteActions.empty()) continue;
                auto message=std::move(actor.remoteActions.front());
                const bool catchUp=actor.remoteActions.size()>=3;
                actor.remoteActions.pop_front();
                if (const auto* reply=std::get_if<mir3::SpellReply>(&message)) {
                    PlaySpellEffects(*reply,actor.x,actor.y,actor.facing);
                    continue;
                }
                if (const auto* hit=std::get_if<LocalScene::HumanHit>(&message)) {
                    StartRemoteHit(actor,id,*hit,now,catchUp);
                    continue;
                }
                if (const auto* motion=std::get_if<LocalScene::RemoteMovement>(&message)) {
                    StartRemoteMovement(actor,id,*motion,catchUp);
                    continue;
                }
                if (const auto* status=std::get_if<LocalScene::HumanStatus>(&message)) {
                    actor.actionFlags=status->flags;
                    continue;
                }
                if (actor.dead) continue;
                if (const auto* melee=std::get_if<LocalScene::RemoteMelee>(&message)) {
                    StartRemoteMelee(actor,id,*melee,now,catchUp);
                    continue;
                }
                const auto& next=std::get<LocalScene::RemoteCast>(message);
                actor.facing=SpellFacing(int(actor.x),int(actor.y),next.x,next.y);
                actor.levelPoseAt=actor.crouchedAt=0;
                StartCast(actor.casting,next.skill,mir3::SpellRoutes::remote_action(next.skill),id,
                    actor.x,actor.y,actor.facing,now,catchUp);
            }
            for (auto& playback:scene.levelEffects) {
                playback.effect.advance(static_cast<std::uint32_t>(now-playback.tick));
                playback.tick=now;
            }
            std::erase_if(scene.levelEffects,[](const auto& p) { return p.effect.phase==mir3::LevelUpEffect::Phase::finished; });
            std::erase_if(scene.speech,[&](const auto& entry) { return now>entry.second.expiresAt; });
            if (scene.ambient.fading) {
                scene.ambient.advance(double(now-scene.ambientTick));
                scene.ambientTick=now;
            }
            const auto weatherElapsed=now-scene.weatherTick;
            if ((!snowEnabled || !scene.weather.fog) && !scene.weather.other) scene.weatherTick=now;
            else if (scene.terrain.Ready() && weatherElapsed>=16) {
                scene.weather.advance(int((std::min)(weatherElapsed,std::uint64_t(250))),snowEnabled);
                scene.weatherTick=now;
            }
        }
        if (returnToCharactersAt && now>=returnToCharactersAt && (!Active() || output_.empty() || now>=deadline_)) {
            Close();
            ClearGameState();
            ClearCharacterSelection();
            OpenCharacters();
            return;
        }
        if (minimapDeadline && now>=minimapDeadline) {
            minimapDeadline=0;
            minimap.pending=portraitMapPending=false;
        }
        if (npcDialogue.actor && npcDialogue.originX && npcDialogue.originY &&
            ((std::max)(std::abs(static_cast<int>(scene.x) - static_cast<int>(npcDialogue.originX)),
                        std::abs(static_cast<int>(scene.y) - static_cast<int>(npcDialogue.originY))) > 5))
            CloseNpcDialogue();
        if (shop.detailDeadline && now >= shop.detailDeadline) {
            shop.detailDeadline = 0;
            shop.status = "No response for this page.";
        }
        if (purchase.deadline && now >= purchase.deadline) {
            if (shop.actor == purchase.actor) shop.status = "The purchase request timed out.";
            purchase = {};
        }
        if (scene.pending.deadline && GetTickCount64() >= scene.pending.deadline) scene.pending = {};
        if (scene.crouchReplyAt && now>=scene.crouchReplyAt) scene.crouchReplyAt=0;
        if (equipment.request != mir3_cui::EquipmentRequest::None && GetTickCount64() >= equipment.deadline) {
            if (equipment.request == mir3_cui::EquipmentRequest::Remove &&
                inventory.returningEquipment == equipment.pending.key) {
                inventory.returningEquipment = {};
                inventory.returnCell = -1;
            }
            equipment.request = mir3_cui::EquipmentRequest::None;
            equipment.ReleaseHeld();
            equipment.status = "The equipment request timed out.";
        }
        if (inventory.request != mir3_cui::ItemRequest::None && GetTickCount64() >= inventory.deadline) {
            FinishShortcutUse(true);
            inventory.request = mir3_cui::ItemRequest::None;
            RequestInventory();
            inventory.status = "The item request timed out. Refreshing the bag.";
        }
        if (stage == Stage::Handoff && GetTickCount64() >= deadline_) OpenCharacters();
        if (stage == Stage::ConnectingGame && !Active()) OpenGameGateway();
        if (!Active()) return;
        const bool characterConnection = stage == Stage::Characters || stage == Stage::ConnectingCharacters;
        const bool gameConnection = IsGameConnection();
        if (stage != Stage::Servers && stage != Stage::GameSession &&
            (stage != Stage::Characters || characterRequest) &&
            GetTickCount64() >= deadline_) {
            if (gameConnection) EndGameSession("The game gateway timed out.");
            else Reset(characterConnection ? "The character gateway timed out." : "The login gateway timed out.");
            return;
        }
        if (stage == Stage::Connecting || stage == Stage::ConnectingCharacters || stage == Stage::ConnectingGame) {
            fd_set writable, failed;
            FD_ZERO(&writable);
            FD_ZERO(&failed);
            FD_SET(socket_, &writable);
            FD_SET(socket_, &failed);
            timeval timeout = {};
            if (select(0, nullptr, &writable, &failed, &timeout) == SOCKET_ERROR ||
                FD_ISSET(socket_, &failed)) {
                if (gameConnection) EndGameSession("Could not connect to the game gateway.");
                else Reset(characterConnection ? "Could not connect to the character gateway."
                                               : "Could not connect to the login gateway.");
                return;
            }
            if (!FD_ISSET(socket_, &writable)) return;
            if (gameConnection) {
                AuthenticateGame();
            } else if (characterConnection) {
                stage = Stage::Characters;
                QueryCharacters();
            } else {
                Wait(Stage::SigningIn, "Waiting for sign-in...");
            }
        }

        if (!output_.empty()) {
            const int sent = send(socket_, output_.data() + sent_,
                static_cast<int>(output_.size() - sent_), 0);
            if (sent > 0) {
                sent_ += sent;
                if (sent_ == output_.size()) ClearOutput();
            } else if (sent == SOCKET_ERROR && WSAGetLastError() != WSAEWOULDBLOCK) {
                if (gameConnection) EndGameSession("Could not send to the game gateway.");
                else Reset(characterConnection ? "Could not send to the character gateway."
                                               : "Could not send to the login gateway.");
                return;
            }
        }

        if (stage == Stage::SigningOutGame && output_.empty() && !returnToCharactersAt) {
            EndGameSession("You signed out.");
            return;
        }
        char buffer[4096];
        const int count = recv(socket_, buffer, sizeof buffer, 0);
        if (count == 0 || (count == SOCKET_ERROR && WSAGetLastError() != WSAEWOULDBLOCK)) {
            if (gameConnection) EndGameSession("The game gateway disconnected.");
            else Reset(characterConnection ? "The character gateway disconnected." : "The login gateway disconnected.");
            return;
        }
        for (int i = 0; i < count && Active(); ++i) Receive(buffer[i]);
    }

private:
    bool EquipmentCanQueue()
    {
        if (stage != Stage::GameSession || TradeItemLocked()) return false;
        if (equipment.request != mir3_cui::EquipmentRequest::None ||
            inventory.request != mir3_cui::ItemRequest::None || output_.size() >= 8192) {
            equipment.status = "Wait for the current item request before trying again.";
            return false;
        }
        equipment.status.clear();
        return true;
    }

    bool UseItem(const mir3_cui::InventoryItem& item)
    {
        if (!InventoryCanQueue() || item.IsGold()) return false;
        const auto now = GetTickCount64();
        const bool fast = item.mode == 0 && (item.icon == 20 || item.icon == 21 || item.icon == 70);
        if (item.Medicine() && inventory.lastUse && now - inventory.lastUse <= (fast ? 1000u : 2000u)) {
            inventory.status = "This item is cooling down.";
            return false;
        }
        const auto tick = static_cast<DWORD>(now);
        Queue(1006, static_cast<WORD>(tick), item.key.name, item.key.id, static_cast<WORD>(tick >> 16));
        inventory.pending = item;
        if (equipment.held == item.key && equipment.heldSlot < 0) equipment.ReleaseHeld();
        inventory.request = mir3_cui::ItemRequest::Use;
        inventory.deadline = now + 10000;
        if (item.Medicine()) inventory.lastUse = now;
        return true;
    }

    bool InventoryCanQueue()
    {
        if (stage != Stage::GameSession || TradeItemLocked()) return false;
        if (inventory.request != mir3_cui::ItemRequest::None ||
            equipment.request != mir3_cui::EquipmentRequest::None) {
            inventory.status = "Waiting for the previous item request.";
            return false;
        }
        if (output_.size() >= 8192) {
            inventory.status = "The connection is busy. Try again.";
            return false;
        }
        inventory.status.clear();
        return true;
    }

    bool QuestCanQueue()
    {
        if (stage != Stage::GameSession) return false;
        if (output_.size() >= 8192) {
            quests.error = "The connection is busy. Try again.";
            return false;
        }
        quests.error.clear();
        return true;
    }

    bool GuildCanQueue()
    {
        if (output_.size() < 8192) {
            guild.error.clear();
            return true;
        }
        guild.error = "The connection is busy. Try again.";
        return false;
    }

    bool SendOptionCommand(const char* command, std::string& error)
    {
        if (stage != Stage::GameSession || showSignOut) return false;
        if (output_.size() >= 8192) {
            error = "The connection is busy. Try changing the option again.";
            return false;
        }
        Queue(mir3_cui::Say, 0, command);
        error.clear();
        return true;
    }

    SOCKET socket_ = INVALID_SOCKET;
    unsigned sequence_ = 0;
    ULONGLONG deadline_ = 0;
    bool receiving_ = false;
    std::string input_;
    std::string output_;
    size_t sent_ = 0;
    std::string signedInAccount_;
    std::string gameRole_;
    std::string realm_;
    std::array<unsigned char, 4> sceneKey_ = {};
    std::uint32_t localActor_ = 0;
    bool itemShortcutFilePresent_ = false;
    mir3_cui::InventoryDimensions inventoryDimensions_;
    std::filesystem::path resourceRoot_ = ".";
    mir3::EffectCatalog effectCatalog_;
    mir3::Messages sceneMessages_;
    mir3::SoundList sceneSoundList_;

    std::string CharacterStoragePath(const std::string& directory, const char* extension = ".txt") const
    {
        static const char hex[] = "0123456789abcdef";
        std::string file = directory + '/' + host + '-' + port + '-';
        for (const auto* identity : {&realm_, &gameRole_}) {
            for (unsigned char ch : *identity) {
                file += hex[ch >> 4];
                file += hex[ch & 15];
            }
            file += '-';
        }
        return file + extension;
    }

    void LoadChatHistory()
    {
        std::ifstream file(CharacterStoragePath(chatHistoryDirectory), std::ios::binary);
        if (!file) {
            LoadLegacyChatHistory();
            return;
        }
        std::string line;
        while (std::getline(file, line)) {
            if (line.size() < 2 || line.size() > sizeof chat.draft || line.find('\0') != std::string::npos)
                continue;
            const auto value = line.substr(1);
            if (line.front() == 'P') chat.Remember(chat.phrases, value);
            if (line.front() == 'C') chat.Remember(chat.contacts, value);
        }
    }

    void LoadLegacyChatHistory()
    {
        const auto role = ConvertText(gameRole_,936,CP_UTF8);
        const auto realm = ConvertText(realm_,936,CP_UTF8);
        const auto safe = [](const std::string& name) {
            return !name.empty() && name!="." && name!=".." &&
                name.find_first_of("/\\:\"<>|?*")==std::string::npos &&
                std::none_of(name.begin(),name.end(),[](unsigned char byte) { return byte<32; });
        };
        if (!safe(role) || !safe(realm)) return;
        for (bool phrase : {false,true}) {
            const auto suffix = phrase ? ".mlt" : ".slt";
            const auto path = resourceRoot_/"Data"/std::filesystem::u8path(role+"("+realm+")"+suffix);
            std::ifstream file(path,std::ios::binary);
            if (!file) continue;
            std::array<char,10241> bytes{};
            file.read(bytes.data(),bytes.size());
            if (!file.eof() || file.bad()) continue;
            const std::string_view contents(bytes.data(),static_cast<std::size_t>(file.gcount()));
            auto& history = phrase ? chat.phrases : chat.contacts;
            std::size_t start = 0;
            while (history.size()<10) {
                const auto end = contents.find('/',start);
                if (end==contents.npos || end-start>1023) break;
                auto entry = ConvertText(std::string(contents.substr(start,end-start)),936,CP_UTF8);
                if (!entry.empty() && entry.size()+1<sizeof chat.draft && entry.find_first_of("\r\n")==std::string::npos &&
                    entry.find('\0')==std::string::npos) {
                    if (!phrase) entry.insert(entry.begin(),'/');
                    chat.Remember(history,entry);
                }
                start = end+1;
            }
        }
    }

    void SaveChatHistory()
    {
        const auto path = CharacterStoragePath(chatHistoryDirectory);
        const auto temporary = path + '.' + std::to_string(GetCurrentProcessId()) + ".tmp";
        CreateDirectoryA(chatHistoryDirectory.c_str(), nullptr);
        std::ofstream file(temporary, std::ios::binary | std::ios::trunc);
        for (const auto& line : chat.contacts) file << 'C' << line << '\n';
        for (const auto& line : chat.phrases) file << 'P' << line << '\n';
        file.close();
        if (!file || !MoveFileExA(temporary.c_str(), path.c_str(), MOVEFILE_REPLACE_EXISTING)) {
            DeleteFileA(temporary.c_str());
            chat.error = "Could not save chat shortcuts.";
        }
    }

    void LoadItemShortcuts()
    {
        std::ifstream file(CharacterStoragePath(itemShortcutDirectory), std::ios::binary);
        itemShortcutFilePresent_ = file.is_open();
        std::string name;
        for (auto& key : inventory.shortcuts) {
            if (!std::getline(file, name)) break;
            if (!name.empty() && name.size() <= 30 && name.find('\0') == std::string::npos &&
                name.find('\r') == std::string::npos &&
                std::none_of(inventory.shortcuts.begin(), inventory.shortcuts.end(), [&](const auto& bound) {
                    return bound.name == name;
                }))
                key = {0, name};
        }
    }

    void SaveItemShortcuts()
    {
        const auto path = CharacterStoragePath(itemShortcutDirectory);
        const auto temporary = path + '.' + std::to_string(GetCurrentProcessId()) + ".tmp";
        CreateDirectoryA(itemShortcutDirectory.c_str(), nullptr);
        std::ofstream file(temporary, std::ios::binary | std::ios::trunc);
        for (const auto& key : inventory.shortcuts) file << key.name << '\n';
        file.close();
        if (!file || !MoveFileExA(temporary.c_str(), path.c_str(), MOVEFILE_REPLACE_EXISTING)) {
            DeleteFileA(temporary.c_str());
            inventory.shortcutStatus = "Could not save item shortcuts.";
        }
    }

    void RestoreInventoryLayout(std::vector<mir3_cui::InventoryItem>& records)
    {
        std::ifstream file(CharacterStoragePath(inventoryLayoutDirectory, ".itms"), std::ios::binary);
        if (!file) return;
        std::string data(mir3_cui::InventoryLayout::FileSize, '\0');
        if (!file.read(&data[0], data.size()) || file.peek() != std::char_traits<char>::eof()) return;
        mir3_cui::InventoryLayout::Restore(data, records, itemShortcutFilePresent_ ? nullptr : &inventory.shortcuts);
    }

    void SaveInventoryLayout()
    {
        if (!inventory.initialized || gameRole_.empty()) return;
        const auto data = mir3_cui::InventoryLayout::Encode(inventory);
        const auto path = CharacterStoragePath(inventoryLayoutDirectory, ".itms");
        const auto temporary = path + '.' + std::to_string(GetCurrentProcessId()) + ".tmp";
        CreateDirectoryA(inventoryLayoutDirectory.c_str(), nullptr);
        std::ofstream file(temporary, std::ios::binary | std::ios::trunc);
        file.write(data.data(), data.size());
        file.close();
        if (!file || !MoveFileExA(temporary.c_str(), path.c_str(), MOVEFILE_REPLACE_EXISTING)) {
            DeleteFileA(temporary.c_str());
            inventory.status = "Could not save the bag layout.";
        }
    }

    void RemoveInventoryItem(const mir3_cui::ItemKey& key)
    {
        if (equipment.heldSlot < 0 && equipment.held == key) {
            equipment.ReleaseHeld();
            inventory.heldShortcut = -1;
        }
        // Resolve references before erasing the record they may be bound to.
        bool changed = false;
        for (std::size_t i = 0; i < inventory.shortcuts.size(); ++i) {
            const auto* item = inventory.ResolveShortcut(i);
            if (item && item->key == key) {
                inventory.shortcuts[i] = {};
                changed = true;
            }
        }
        inventory.Remove(key);
        if (changed) SaveItemShortcuts();
    }

    void FinishShortcutUse(bool restore)
    {
        const int slot = std::exchange(inventory.pendingShortcut,-1);
        if (slot < 0 || !restore || !inventory.Find(inventory.pending.key)) return;
        const auto same = std::find_if(inventory.shortcuts.begin(),inventory.shortcuts.end(),
            [&](const auto& item) { return item.name == inventory.pending.key.name; });
        if (same != inventory.shortcuts.end()) return;
        auto target = inventory.shortcuts.begin()+slot;
        if (!target->name.empty())
            target = std::find_if(inventory.shortcuts.begin(),inventory.shortcuts.end(),
                [](const auto& item) { return item.name.empty(); });
        if (target != inventory.shortcuts.end()) {
            *target = inventory.pending.key;
            SaveItemShortcuts();
        }
    }

    bool IsGameConnection() const
    {
        return stage == Stage::ConnectingGame || stage == Stage::AuthenticatingGame ||
            stage == Stage::GameSession || stage == Stage::SigningOutGame;
    }

    void EndGameSession(const char* message)
    {
        if (returnToCharactersAt) {
            Close();
            return;
        }
        if (stage == Stage::GameSession || stage == Stage::SigningOutGame) SaveInventoryLayout();
        Close();
        CloseNpcDialogue();
        merchant = {};
        merchantResults.clear();
        skills = {};
        mount = {};
        trade = {};
        mountInputUntil = 0;
        skillShortcutsVisible = false;
        ClearMinimap();
        stage = Stage::GameDisconnected;
        status = message;
        showSignOut = false;
        quitGameRequested = false;
        if (audioOpen) SaveAudioSettings();
        audioOpen = false;
    }

    void AuthenticateGame()
    {
        sceneKey_ = {};
        localActor_ = 0;
        // The first game frame has a sequence digit and an encoded body, with no default header.
        constexpr std::uint32_t version = 0x0131a4f0u;
        const auto session = static_cast<std::uint32_t>(selectionSession);
        const auto signedText = [](std::uint32_t value) {
            return std::to_string(static_cast<std::int64_t>(value) -
                (value >= 0x80000000u ? 0x100000000LL : 0));
        };
        const std::string body = "**" + signedInAccount_ + '/' + gameRole_ + '/' +
            signedText(session ^ 0x3eb2c5ccu) + '/' +
            signedText(version ^ session ^ 0x5580af27u) + '/' +
            signedText(version ^ 0xfa0280afu) + "/1";
        QueueFrame(nullptr, body);
        Wait(Stage::AuthenticatingGame, "Waiting for game sign-in...");
    }

    void OpenGameGateway()
    {
        sockaddr_in address = {};
        address.sin_family = AF_INET;
        address.sin_port = htons(static_cast<u_short>(gameGateway.port));
        InetPtonA(AF_INET, gameGateway.host.c_str(), &address.sin_addr);
        const int result = Connect(address);
        if (result == SOCKET_ERROR) EndGameSession("Could not connect to the game gateway.");
        else if (result == 0) AuthenticateGame();
        else Wait(Stage::ConnectingGame, "Connecting to the game gateway...");
    }

    int Connect(const sockaddr_in& address)
    {
        Close();
        socket_ = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        u_long nonBlocking = 1;
        if (!Active() || ioctlsocket(socket_, FIONBIO, &nonBlocking) == SOCKET_ERROR)
            return SOCKET_ERROR;
        const int result = connect(socket_, reinterpret_cast<const sockaddr*>(&address), sizeof address);
        return result == SOCKET_ERROR && WSAGetLastError() == WSAEWOULDBLOCK ? 1 : result;
    }

    void RequestCharacter(WORD opcode, const std::string& body)
    {
        Queue(opcode, 0, body);
        characterRequest = opcode;
        deadline_ = GetTickCount64() + 10000;
    }

    void QueryCharacters()
    {
        RequestCharacter(CM_QUERYCHR, signedInAccount_ + '/' + std::to_string(selectionSession));
    }

    void OpenCharacters()
    {
        sockaddr_in address = {};
        address.sin_family = AF_INET;
        address.sin_port = htons(static_cast<u_short>(selectionGateway.port));
        InetPtonA(AF_INET, selectionGateway.host.c_str(), &address.sin_addr);
        const int result = Connect(address);
        if (result == SOCKET_ERROR) {
            Reset("Could not connect to the character gateway.");
        } else if (result == 0) {
            stage = Stage::Characters;
            QueryCharacters();
        } else {
            Wait(Stage::ConnectingCharacters, "Connecting to the character gateway...");
        }
    }

    void HandleCharacterFrame(const _TDEFAULTMESSAGE& header, const std::string& body)
    {
        switch (header.wIdent) {
        case SM_QUERYCHR:
            if (characterRequest != CM_QUERYCHR) break;
            characterRequest = 0;
            if (!ReadCharacters(header.nRecog, body, characters)) {
                ShowCharacterNotice(220, "The character gateway sent an invalid character list.");
                break;
            }
            characterPage = 0;
            characterIndex = characters.empty() ? -1 : 0;
            ++characterListRevision;
            if (characters.empty() && characterNotice.empty()) ShowCharacterNotice(219, "This account has no characters.");
            break;
        case SM_QUERYCHR_FAIL:
            if (characterRequest != CM_QUERYCHR) break;
            characterRequest = 0;
            ShowCharacterNotice(220, "Could not load the character list.");
            break;
        case SM_NEWCHR_SUCCESS:
            if (characterRequest != CM_NEWCHR) break;
            CloseCharacterDialog();
            QueryCharacters();
            break;
        case SM_NEWCHR_FAIL:
            if (characterRequest != CM_NEWCHR) break;
            characterRequest = 0;
            ShowCharacterNotice(header.nRecog == 2 ? 224 : header.nRecog == 3 ? 226 : 232,
                header.nRecog == 2 ? "That character name already exists." :
                header.nRecog == 3 ? "This account has no free character slot." : "Could not create the character.");
            break;
        case SM_DELCHR_FAIL:
            characterNotice = "The character gateway declined to delete the character.";
            break;
        case SM_STARTPLAY:
            if (characterRequest != CM_SELCHR) break;
            characterRequest = 0;
            gameGateway = ReadGateway(body, false);
            if (!gameGateway.port) {
                ShowCharacterNotice(221, "The character gateway returned an invalid game address.");
                break;
            }
            gameRole_ = characters[characterIndex].name;
            gameCharacter = characters[characterIndex].label;
            Close();
            Wait(Stage::ConnectingGame, "Connecting to the game gateway...");
            break;
        case SM_STARTFAIL:
            if (characterRequest != CM_SELCHR) break;
            characterRequest = 0;
            ShowCharacterNotice(221, "The character could not enter the game.");
            break;
        case SM_OUTOFCONNECTION:
            Reset("The character gateway ended the session.");
            break;
        }
    }

    void ClearOutput()
    {
        if (!output_.empty()) SecureZeroMemory(&output_[0], output_.size());
        output_.clear();
        sent_ = 0;
    }

    void Close()
    {
        if (Active()) closesocket(socket_);
        socket_ = INVALID_SOCKET;
        sequence_ = 0;
        receiving_ = false;
        input_.clear();
        ClearOutput();
    }

    void Wait(Stage next, const char* message)
    {
        stage = next;
        status = message;
        deadline_ = GetTickCount64() + 10000;
    }

    void Queue(WORD opcode, WORD parameter, const std::string& body, std::uint32_t recognition = 0, WORD series = 0,
        WORD tag = 0)
    {
        _TDEFAULTMESSAGE header;
        static_assert(sizeof(header) == 12, "Login header wire size");
        fnMakeDefMessage(&header, opcode, static_cast<int>(recognition), parameter, tag, series);
        char encodedHeader[DEFBLOCKSIZE + 1];
        fnEncodeMessage(&header, encodedHeader, sizeof encodedHeader);
        QueueFrame(encodedHeader, body);
    }

    void QueueFrame(const char* encodedHeader, const std::string& body)
    {
        std::vector<char> encodedBody((body.size() * 4 + 2) / 3 + 1);
        fnEncode6BitBuf(reinterpret_cast<unsigned char*>(const_cast<char*>(body.data())),
            encodedBody.data(), static_cast<int>(body.size()), static_cast<int>(encodedBody.size()));
        sequence_ = sequence_ % 9 + 1;
        output_ += '#';
        output_ += static_cast<char>('0' + sequence_);
        if (encodedHeader) output_ += encodedHeader;
        output_ += encodedBody.data();
        output_ += '!';
        SecureZeroMemory(encodedBody.data(), encodedBody.size());
    }

    void Receive(char byte)
    {
        if (byte == '*') {
            if (output_.size() >= 8192) {
                if (IsGameConnection()) EndGameSession("The game gateway sent too many pending heartbeats.");
                else Reset("The gateway sent too many pending heartbeats.");
                return;
            }
            output_ += '*';
        } else if (byte == '#') {
            input_.clear();
            receiving_ = true;
        } else if (receiving_) {
            if (byte == '!') {
                receiving_ = false;
                HandleFrame();
                input_.clear();
            } else {
                const bool game = IsGameConnection();
                if (input_.size() >= (game ? 65536u : 4096u) || (!game && (byte < 60 || byte > 123))) {
                    if (game) EndGameSession("The game gateway sent an oversized reply.");
                    else Reset("The gateway sent an invalid reply.");
                    return;
                }
                input_ += byte;
            }
        }
    }

    void HandleGameFrame()
    {
        if (stage==Stage::GameSession && scene.meleeSpells.receive(input_)) return;
        if (input_.compare(0, 5, "+GOOD") == 0) {
            scene.spellReplyUntil=0;
            scene.crouchReplyAt=0;
            if (scene.pending.deadline && !scene.dead) {
                if (!scene.movementOrigin) {
                    scene.SetCell(scene.pending.x,scene.pending.y);
                    scene.facing=scene.pending.facing;
                }
                scene.pending = {};
            }
            return;
        }
        if (input_.compare(0, 5, "+FAIL") == 0) {
            if (std::exchange(scene.spellReplyUntil,0) && !scene.dead) {
                scene.facing=scene.spellOriginalFacing;
                scene.casting={};
                audio.StopSceneEffect(0);
            }
            if (scene.crouchReplyAt) scene.crouchedAt=scene.crouchReplyAt=0;
            if (scene.movementOrigin && !scene.dead && (!characterStatus.hp[1] || characterStatus.hp[0])) {
                const auto origin=*scene.movementOrigin;
                scene.SetCell(origin.x,origin.y);
                scene.facing=origin.facing;
                scene.localStep={};
                scene.pendingWalk.reset();
                scene.movementOrigin.reset();
                audio.StopSceneEffect(0);
            }
            scene.pending = {};
            return;
        }
        if (input_.size() < DEFBLOCKSIZE) return;
        std::array<char, DEFBLOCKSIZE + 1> encodedHeader = {};
        for (std::size_t i = 0; i < DEFBLOCKSIZE; ++i) {
            if (input_[i] < 60 || input_[i] > 123) return;
            encodedHeader[i] = input_[i];
        }
        std::array<unsigned char, 12> header = {};
        if (fnDecode6BitBuf(encodedHeader.data(), reinterpret_cast<char*>(header.data()), static_cast<int>(header.size())) != 12)
            return;
        const unsigned opcode = (header[0] ^ sceneKey_[0]) |
            (static_cast<unsigned>(header[4] ^ sceneKey_[2] ^ sceneKey_[1]) << 8);
        const std::uint32_t actor = (header[5] ^ sceneKey_[3]) | (std::uint32_t(header[7]) << 8) |
            (std::uint32_t(header[9]) << 16) | (std::uint32_t(header[11]) << 24);
        const auto first = static_cast<WORD>(header[1] | (header[6] << 8));
        const auto second = static_cast<WORD>(header[2] | (header[8] << 8));
        const auto third = static_cast<WORD>(header[3] | (header[10] << 8));
        if (opcode == GameLogon && (stage == Stage::AuthenticatingGame ||
                                    (stage==Stage::GameSession && actor==localActor_))) {
            if (!actor || actor > 0x7fffffffu || input_.size() != DEFBLOCKSIZE + 51 ||
                (third & 0xff) >= 8) return;
            // These sections have independent six-bit padding.
            const auto feature = DecodeGameBytes(input_.substr(DEFBLOCKSIZE, 22));
            const auto extension = DecodeGameBytes(input_.substr(DEFBLOCKSIZE + 22, 7));
            const auto extra = DecodeGameBytes(input_.substr(DEFBLOCKSIZE + 29, 22));
            if (feature.size() != 16 || extension.size() != 5 || extra.size() != 16) return;
            mir3_cui::CharacterAppearance appearance;
            if (!appearance.Apply(PackedAppearance(feature)) || appearance.profession > 2) return;

            const bool reentry=stage==Stage::GameSession;
            if (reentry) {
                using Terrain=mir3_cui::MapTerrain;
                using Cell=mir3_cui::GroundCell;
                const auto view=scene.terrain.ReadView(first,second);
                const auto cell=view[Terrain::ViewRows/2*Terrain::ViewColumns+Terrain::ViewColumns/2];
                if (cell!=Cell::Walkable && cell!=Cell::DoorOpen) return;
            }
            stage = Stage::GameSession;
            scene.dead=scene.dying=false;
            ResetLocalActorActions();
            scene.health={};
            characterAppearance = appearance;
            const auto mountShape = static_cast<unsigned char>(extension[0]);
            mount.shape = mountShape < 4 ? mountShape : 0;
            localActor_ = actor;
            scene.weather.random_state=static_cast<std::uint32_t>(GetTickCount64());
            scene.SetCell(first, second);
            scene.facing = third & 0xff;
            scene.light = third >> 8;
            scene.actionFlags=0;
            for (unsigned i=0;i<4;++i) scene.actionFlags|=std::uint32_t(std::uint8_t(feature[4+i]))<<(8*i);
            if (reentry) Queue(ClientQueryBag,0,"");
            else {
                LoadChatHistory();
                LoadItemShortcuts();
            }
            if (portraitMapMode && !scene.mapId.empty()) {
                portraitMapPending=true;
                RequestMinimap();
            }
            status = "Signed in to the game gateway.";
        } else if (opcode == SM_CERTIFICATION_FAIL || opcode == SM_OUTOFCONNECTION) {
            EndGameSession(opcode == SM_CERTIFICATION_FAIL ? "The game gateway rejected sign-in."
                                                         : "The game gateway ended the session.");
        } else if (opcode == GameDecodeKey && stage == Stage::GameSession) {
            const auto body = ReadGameBody();
            if (body.size() == 60 && body.find('\0') == std::string::npos) ApplySceneKey(body);
        } else if (opcode == GameNewMap || opcode == GameChangeMap || opcode == GameMapDescription) {
            HandleMapUpdate(opcode, first, second, third);
        } else if (stage == Stage::GameSession && opcode == GameDayChanging) {
            if (input_.size()==DEFBLOCKSIZE) {
                scene.ambient.select_base(first&255,scene.weather.random());
                if (second!=scene.ambient.mode) scene.ambientTick=GetTickCount64();
                scene.ambient.update(second);
                scene.weather.select(third);
                scene.weatherTick=GetTickCount64();
            }
        } else if (stage == Stage::GameSession && (opcode == GameChangeLight || opcode == GameActorLight)) {
            if (input_.size()==DEFBLOCKSIZE) {
                if (actor && actor==localActor_ && opcode==GameActorLight) scene.light=first&255;
                else if (const auto player=scene.players.find(actor);player!=scene.players.end() && !player->second.IsNpc() &&
                    (player->second.IsMonster()==(opcode==GameChangeLight))) player->second.light=first&255;
            }
        } else if (stage == Stage::GameSession && (opcode == GameMinimap || opcode == GameMinimapUnavailable)) {
            if (input_.size() != DEFBLOCKSIZE) return;
            if (opcode == GameMinimap) {
                if (first) minimapReply=first;
            } else {
                minimapReply=0;
                minimapDeadline=0;
                minimap.reply_time=GetTickCount64();
                minimap.visible=minimap.pending=minimap.dragging=minimap.release_captured=false;
                portraitMapReady=portraitMapPending=false;
                minimapUnavailable=true;
            }
        } else if (stage == Stage::GameSession && (opcode == GameAddSkill || opcode == GameSkillList)) {
            HandleSkillRecords(opcode == GameSkillList, third & 0xff);
        } else if (stage == Stage::GameSession && opcode == GameRideHorse) {
            if (input_.size() == DEFBLOCKSIZE) {
                mount.apply_reply(actor);
                scene.pending = {};
                scene.pendingWalk.reset();
                scene.movementOrigin.reset();
                scene.lastMove = 0;
            }
        } else if (stage == Stage::GameSession && opcode>=GameTradeOpen && opcode<=GameTradeComplete) {
            HandleTradeReply(opcode,actor,first,second);
        } else if (stage == Stage::GameSession && (opcode == GameRemoveSkill || opcode == GameSkillProgress)) {
            if (actor && actor <= 65535 && input_.size() == DEFBLOCKSIZE) {
                if (opcode == GameRemoveSkill) skills.remove(static_cast<int>(actor));
                else skills.update_progress(static_cast<int>(actor),second | (std::uint32_t(third)<<16),first & 0xff);
            }
        } else if (stage == Stage::GameSession && opcode == GameDoorLocked) {
            if (input_.size()==DEFBLOCKSIZE) {
                const auto label=ConvertText(sceneMessages_.get(1472),936,CP_UTF8);
                chat.Append(100,label.empty() ? "The door is locked." : label,11861940);
            }
        } else if (stage == Stage::GameSession && (opcode == GameDoorOpen || opcode == GameDoorClosed)) {
            if (input_.size() == DEFBLOCKSIZE && scene.terrain.UpdateDoor(first, second, opcode == GameDoorOpen))
                scene.ground = scene.terrain.ReadView(scene.x, scene.y);
        } else if (stage == Stage::GameSession && opcode == GameGroundItemShow) {
            ApplyGroundItem(actor,first,second,third);
        } else if (stage == Stage::GameSession && opcode == GameShowMapEvent) {
            if (input_.size()==DEFBLOCKSIZE+6 && scene.terrain.Contains(second,third) &&
                (first==3 || first==4 || first==5 || first==6 || first==9) && ReadGameBody().size()==4)
                SceneEffects().mapEvents.show(effectCatalog_,actor,first,{second,third},
                    first==9 && scene.terrain.BaseWalkable(second,third),scene.weather);
        } else if (stage == Stage::GameSession && opcode == GameHideMapEvent) {
            if (input_.size()==DEFBLOCKSIZE && scene.spellEffects)
                scene.spellEffects->mapEvents.hide(effectCatalog_,actor,scene.weather);
        } else if (stage == Stage::GameSession && (opcode==GameMagicFire || opcode==GameMagicFireList)) {
            HandleSpellEffects(opcode==GameMagicFireList,actor,first,second,third);
        } else if (stage == Stage::GameSession && opcode == GameSpell && actor!=localActor_) {
            const auto found=scene.players.find(actor);
            if (found!=scene.players.end() && !found->second.IsNpc() && !found->second.IsMonster() &&
                input_.size()<=DEFBLOCKSIZE+16 && found->second.remoteActions.size()<64) {
                const auto* text=input_.c_str()+DEFBLOCKSIZE;
                char* end=nullptr;
                const auto skill=std::strtol(text,&end,10);
                if (end!=text && skill>0 && skill<=65535)
                    QueueHumanAction(found->second,LocalScene::RemoteCast{int(skill),first,second});
            }
        } else if (stage == Stage::GameSession && opcode == GameGroundItemHide) {
            if (actor && actor<=0x7fffffffu && input_.size()==DEFBLOCKSIZE) scene.groundItems.hide(static_cast<int>(actor));
        } else if (stage==Stage::GameSession && localActor_ &&
                   ((actor==localActor_ && (opcode==GameActorDeath || opcode==GameActorDead || opcode==GameBackstep ||
                                            opcode==GameRush || opcode==GameBump)) ||
                    (opcode==GameActorReviveRefresh && (actor==localActor_ || !scene.players.contains(actor))))) {
            HandleLocalLife(opcode,first,second,third);
        } else if (stage == Stage::GameSession &&
                   (opcode == GameBackstep || opcode == GameTurn || opcode == GameWalk || opcode == GameRun || opcode == GameBump)) {
            if (actor == localActor_) HandleLocalMovement(opcode, first, second, third);
            else HandleRemotePlayer(opcode, actor, first, second, third);
        } else if (stage == Stage::GameSession &&
                   (opcode == GameActorDeath || opcode == GameActorDead ||
                    opcode == GameActorRevive || opcode == GameActorReviveRefresh ||
                    opcode == GameMonsterDigUp || opcode == GameMonsterRise || opcode == GameMonsterSkeleton)) {
            if (actor != localActor_) HandleRemotePlayer(opcode, actor, first, second, third);
        } else if (stage == Stage::GameSession && (opcode == GameActorName || opcode == GameActorNameColor)) {
            const auto player = scene.players.find(actor);
            if (player != scene.players.end() && input_.size() <= DEFBLOCKSIZE + 344) {
                const bool noBody=input_.size()==DEFBLOCKSIZE;
                if (opcode==GameActorNameColor && noBody) {
                    player->second.identity.color=mir3::SceneName::colorref(first&255);
                } else if (opcode==GameActorName || player->second.IsMonster()) {
                    auto body=ReadGameBody();
                    if (!body.empty() || noBody) SetRemoteName(player->second,std::move(body),first&255);
                }
            }
        } else if (stage == Stage::GameSession && (opcode == GameCrouch || opcode == GameButcher)) {
            const auto player=scene.players.find(actor);
            const auto direction=third&0xffu;
            if (actor!=localActor_ && player!=scene.players.end() && !player->second.npcType && direction<8 &&
                scene.terrain.Contains(first,second)) {
                QueueRemotePosition(player->second,actor,{WORD(opcode),first,second,WORD(direction)});
            }
        } else if (stage == Stage::GameSession && opcode == GameTargetPortrait) {
            ApplyTargetPortrait(actor,first | (std::uint32_t(second)<<16));
        } else if (stage == Stage::GameSession && opcode == GameTargetAttributes) {
            ApplyMonsterAttributes(actor,first,second);
        } else if (stage == Stage::GameSession && (opcode == GameMonsterStatus || opcode == GameHumanStatus801 ||
                   opcode == GameHumanStatus || opcode == GameHumanStatus808)) {
            const auto found=scene.players.find(actor);
            if (input_.size()!=DEFBLOCKSIZE) return;
            const auto flags=std::uint32_t(first)|(std::uint32_t(second)<<16);
            const LocalScene::HumanStatus status{flags,std::bit_cast<std::int16_t>(third)};
            if (actor==localActor_) {
                if (opcode==GameMonsterStatus) ApplyLocalHumanStatus(status);
                else if (scene.localActions.size()<64) scene.localActions.emplace_back(status);
            } else if (found!=scene.players.end()) {
                if (found->second.IsMonster()) {
                    if (opcode==GameMonsterStatus) found->second.monsterStatus=flags;
                }
                else if (!found->second.IsNpc()) {
                    if (opcode==GameMonsterStatus) found->second.actionFlags=flags;
                    else QueueHumanAction(found->second,status);
                }
            }
        } else if (stage == Stage::GameSession && IsMeleeMessage(opcode)) {
            HandleMeleeMessage(opcode,actor,first,second,third);
        } else if (stage == Stage::GameSession && (opcode == GameActorStruck || opcode == GameActorStruckWithSource)) {
            HandleSceneHit(opcode,actor,first,second,third);
        } else if (stage == Stage::GameSession && opcode == GameMonsterAttackSpecial) {
            HandleMonsterCombat(opcode,actor,first,second,third);
        } else if (stage == Stage::GameSession && opcode == GameNpcDialogue) {
            if (actor && actor <= 0x7fffffffu && input_.size() <= DEFBLOCKSIZE + (8192 * 4 + 2) / 3) {
                auto body = ReadGameBody();
                const auto terminator = body.find('\0');
                if (terminator != std::string::npos) body.resize(terminator);
                if (!body.empty() && body.size() <= 8192) {
                    const auto previous = npcDialogue.actor;
                    if (npcDialogue.Apply(actor, ConvertText(body, 936, CP_UTF8))) {
                        npcDialogue.originX = scene.x;
                        npcDialogue.originY = scene.y;
                        if (previous != actor) shop = {};
                    }
                }
            }
        } else if (stage == Stage::GameSession && opcode == GameNpcClose) {
            if (input_.size() == DEFBLOCKSIZE) CloseNpcDialogue();
        } else if (stage == Stage::GameSession && (opcode == GameShopGoods || opcode == GameShopOffers ||
                   opcode == GameShopBought || opcode == GameShopRejected)) {
            HandleShopReply(opcode, actor, first, second);
        } else if (stage == Stage::GameSession &&
                   ((opcode >= GameOpenSell && opcode <= GameSellFail) ||
                    (opcode >= GameOpenRepair && opcode <= GameRepairQuote) ||
                    opcode == GameSellStackOk || opcode == GameSellStackFail)) {
            HandleMerchantReply(opcode, actor, first, second);
        } else if (stage == Stage::GameSession && opcode >= GameStorageOpen && opcode <= GameStorageBagFull) {
            HandleStorageReply(opcode, actor, first, second, third);
        } else if (stage == Stage::GameSession &&
                   (opcode == GameActorExit || opcode == GameActorDisappear || opcode == GameActorRemove ||
                    opcode == GameClearActors)) {
            if (input_.size() == DEFBLOCKSIZE) {
                if (opcode == GameClearActors) {
                    if (scene.spellEffects) for (const auto& [id,actor]:scene.players)
                        scene.spellEffects->forgetActor(id,ActorWorldPixel(actor));
                    scene.players.clear();
                    scene.lockedSpellActor=0;
                    audio.ClearSceneEffects(true);
                    std::erase_if(scene.speech,[](const auto& entry) { return entry.first!=0; });
                } else {
                    if (scene.spellEffects) scene.spellEffects->forgetActor(actor,SpellTarget(actor));
                    scene.players.erase(actor);
                    if (scene.lockedSpellActor==actor) scene.lockedSpellActor=0;
                    audio.StopSceneEffect(actor);
                    scene.speech.erase(actor);
                }
            }
        } else if ((stage == Stage::GameSession || stage == Stage::AuthenticatingGame) &&
                   (opcode == GameAbility || opcode == GameVitals || opcode == GameHealthRegistration || opcode == GameWeight ||
                    opcode == GameSubAbility || opcode == GameExperience || opcode == GameLevelUp)) {
            HandleCharacterStatus(opcode, actor, first, second, third);
        } else if (opcode == mir3_cui::ClientOptions && stage == Stage::GameSession) {
            if (input_.size() == DEFBLOCKSIZE) {
                const auto changed = serverOptions.Apply(actor, first);
                chat.reception.Apply(actor, changed);
            }
        } else if (stage == Stage::GameSession &&
                   opcode >= static_cast<unsigned>(mir3_cui::PartyReply::Created) &&
                   opcode <= static_cast<unsigned>(mir3_cui::PartyReply::Roster)) {
            HandlePartyReply(static_cast<mir3_cui::PartyReply>(opcode), actor);
        } else if (stage == Stage::GameSession && opcode >= 753 && opcode <= 776) {
            HandleGuildReply(opcode, actor);
        } else if (stage == Stage::GameSession &&
                   ((opcode >= 1301 && opcode <= 1303) || opcode == 1310 || opcode == 1311)) {
            HandleQuestReply(static_cast<mir3_cui::QuestReply>(opcode), first, third);
        } else if (stage == Stage::GameSession &&
                   ((opcode >= 200 && opcode <= 203) || opcode == 600 || opcode == 601 ||
                    opcode == 635 || opcode == 636 || opcode == 653 || opcode == 1313)) {
            HandleInventoryReply(opcode, actor, first, third);
        } else if ((stage == Stage::GameSession || stage == Stage::AuthenticatingGame) &&
                   (opcode == 621 || opcode == 642 || opcode == 615 || opcode == 616 || opcode == 619 || opcode == 620)) {
            HandleEquipmentReply(opcode, actor, first, second);
        } else if (mir3_cui::ChatChannel(opcode) && stage == Stage::GameSession &&
                   input_.size() <= DEFBLOCKSIZE + 1024) {
            auto body = ReadGameBody();
            if (body.empty() && input_.size() != DEFBLOCKSIZE) return;
            const auto terminator = body.find('\0');
            if (terminator != std::string::npos) body.resize(terminator);
            if ((opcode == 40 || opcode == 1501) && body.size() > 259) return;
            const auto text = ConvertText(body,936,CP_UTF8);
            if ((opcode==40 || opcode==1501) && scene.terrain.Ready() &&
                (!actor || actor==localActor_ || scene.players.contains(actor)) && (body.empty() || !text.empty())) {
                const auto speaker=actor==localActor_ ? 0u : actor;
                if (text.empty()) scene.speech.erase(speaker);
                else {
                    auto lines=mir3::wrap_speech(text);
                    if (!lines.empty())
                        scene.speech.insert_or_assign(speaker,mir3::SpeechBubble{std::move(lines),GetTickCount64()+3000});
                }
            }
            // Received chat has two independent append paths in the V1.45 client.
            const int copies = (opcode != 104 ? 1 : 0) + (chat.logging && opcode != 100 ? 1 : 0);
            if (copies) {
                std::ofstream log("Chat.txt", std::ios::binary | std::ios::app);
                for (int i = 0; i < copies && log; ++i) log << body << "\r\n";
            }
            if (text.empty()) return;
            constexpr std::array<unsigned,8> palette{0,657930,0xffffff,255,65280,16750230,0x50ffff,33023};
            const auto color = [&](unsigned index) { return index < palette.size() ? palette[index] : 0u; };
            chat.Append(opcode, text, color(opcode == 104 ? 4 : first & 255u),
                color(opcode == 104 ? 0 : second & 255u));
            if (opcode == 104) {
                for (auto& row : mir3::wrap_system_notice(text))
                    guild.AppendHistory({0,std::move(row)});
            }
        }
    }

    static long ReadMerchantQuantity(const char* value, unsigned available)
    {
        return !std::strcmp(value,"*") ? static_cast<long>(available) : std::strtol(value,nullptr,10);
    }

    void StartMerchantService(std::uint32_t actor, mir3_cui::MerchantService::Kind kind)
    {
        merchant = {};
        merchant.actor = actor;
        merchant.revision = ++merchantRevision;
        merchant.kind = kind;
        inventory.open = true;
    }

    void ApplyBagCount(mir3_cui::InventoryItem& item, WORD count)
    {
        if (count > item.count)
            merchantResults.push_back({1707,item.label,count-item.count});
        item.count = count;
    }

    void HandleStorageReply(unsigned opcode, std::uint32_t actor, WORD first, WORD second, WORD count)
    {
        using Service = mir3_cui::MerchantService;
        if (opcode == GameStorageList) {
            const auto storageActor = shop.storage ? shop.actor : actor;
            constexpr std::size_t itemLength = 134, recordLength = itemLength+1;
            if (!storageActor || storageActor > 0x7fffffffu ||
                input_.size() != DEFBLOCKSIZE + count*recordLength) return;
            std::vector<mir3_cui::NpcShop::Offer> offers;
            offers.reserve(count);
            for (std::size_t at = DEFBLOCKSIZE; at < input_.size(); at += recordLength) {
                mir3_cui::InventoryItem item;
                if (input_[at+itemLength] != '/' ||
                    !DecodeInventoryItem(input_.substr(at,itemLength),item,true) || !item.key.id) return;
                offers.push_back({std::move(item.key.name),std::move(item.label),item.key.id,
                    0,item.icon,item.count,item.stackMode});
            }
            const auto pending = shop.storage ? shop.withdrawing : 0;
            const int notice = merchant.notice;
            shop = {};
            merchant = {};
            merchant.notice = notice;
            shop.actor = storageActor;
            shop.storage = shop.offersVisible = true;
            shop.withdrawing = pending;
            shop.revision = ++shopRevision;
            shop.catalogueRevision = shop.revision;
            shop.offers = std::move(offers);
            shop.offer = shop.offers.empty() ? -1 : 0;
            return;
        }
        if (input_.size() != DEFBLOCKSIZE) return;
        if (opcode == GameStorageOpen) {
            if (!actor) return;
            shop = {};
            StartMerchantService(actor,Service::Kind::Deposit);
            return;
        }
        if (opcode <= GameStorageDepositFail) {
            if (opcode != GameStorageDepositOk) merchant.notice = opcode == GameStorageFull ? 1449 : 1450;
            if (!merchant.actor || merchant.kind != Service::Kind::Deposit || merchant.phase != Service::Phase::Paying)
                return;
            if (opcode == GameStorageDepositOk) {
                merchantResults.push_back({second ? 1447 : 1448,ConvertText(merchant.item.name,936,CP_UTF8),second});
                if (!first) RemoveInventoryItem(merchant.item);
                else if (auto* item = inventory.Find(merchant.item)) ApplyBagCount(*item,first);
            }
            merchant.phase = Service::Phase::Choosing;
            return;
        }
        if (opcode != GameStorageWithdrawOk)
            merchant.notice = opcode == GameStorageWithdrawFail ? 1445 : 1446;
        if (!shop.storage || !shop.withdrawing) return;
        const auto pending = std::exchange(shop.withdrawing,0u);
        if (opcode != GameStorageWithdrawOk) return;
        if (second) {
            const auto bagItem = std::find_if(inventory.items.begin(),inventory.items.end(),
                [&](const auto& item) { return item.key.id == pending; });
            if (bagItem != inventory.items.end()) ApplyBagCount(*bagItem,second);
        }
        const auto storedItem = std::find_if(shop.offers.begin(),shop.offers.end(),
            [&](const auto& item) { return item.id == pending; });
        if (storedItem == shop.offers.end()) return;
        if (first) {
            storedItem->count = first;
        } else {
            shop.offers.erase(storedItem);
            shop.offer = -1;
            const auto last = shop.offers.empty() ? 0u : static_cast<unsigned>((shop.offers.size()-1)/12);
            shop.page = (std::min)(shop.page,last);
        }
    }

    void HandleMerchantReply(unsigned opcode, std::uint32_t actor, WORD first, WORD second)
    {
        using Service = mir3_cui::MerchantService;
        if (input_.size() != DEFBLOCKSIZE) return;
        if (opcode == GameOpenSell || opcode == GameOpenRepair) {
            if (!actor) return;
            StartMerchantService(actor,opcode == GameOpenSell ? Service::Kind::Sell : Service::Kind::Repair);
            return;
        }
        if (opcode == GameSellQuote || opcode == GameRepairQuote) {
            const auto kind = opcode == GameSellQuote ? Service::Kind::Sell : Service::Kind::Repair;
            if (merchant.actor && merchant.kind == kind &&
                merchant.phase != Service::Phase::Choosing && merchant.phase != Service::Phase::Paying) {
                merchant.price = actor;
                merchant.phase = actor ? Service::Phase::Quoted : Service::Phase::Unavailable;
            }
            return;
        }

        const bool stack = opcode == GameSellStackOk;
        const bool repair = opcode == GameRepairOk || opcode == GameRepairFail;
        const bool success = opcode == GameSellOk || stack || opcode == GameRepairOk;
        if (success) inventory.gold = actor;
        if (opcode == GameSellFail || opcode == GameRepairFail) merchant.notice = repair ? 1457 : 1455;
        if (!merchant.actor || merchant.kind != (repair ? Service::Kind::Repair : Service::Kind::Sell) ||
            merchant.phase != Service::Phase::Paying) return;
        if (success) {
            merchantResults.push_back({repair ? 1456 : stack ? 1454 : 1453,
                ConvertText(merchant.item.name, 936, CP_UTF8), second});
            if (repair) {
                if (auto* item = inventory.Find(merchant.item)) {
                    item->durability = first;
                    item->maxDurability = second;
                }
            } else if (stack && first) {
                if (auto* item = inventory.Find(merchant.item)) item->count = first;
            } else if (stack) {
                RemoveInventoryItem(merchant.item);
            } else {
                if (equipment.heldSlot < 0 && equipment.held == merchant.item) {
                    equipment.ReleaseHeld();
                    inventory.heldShortcut = -1;
                }
                inventory.Remove(merchant.item);
            }
        }
        merchant.phase = Service::Phase::Choosing;
        merchant.price = 0;
    }

    void HandleShopReply(unsigned opcode, std::uint32_t actor, WORD count, WORD offset)
    {
        if (opcode == GameShopBought || opcode == GameShopRejected) {
            if (input_.size() != DEFBLOCKSIZE || (opcode == GameShopRejected && (actor < 1 || actor > 3))) return;
            if (opcode == GameShopBought) inventory.gold = actor;
            if (shop.storage) {
                purchase = {};
                return;
            }
            if (purchase.deadline && purchase.actor == shop.actor && purchase.revision == shop.revision) {
                if (purchase.offer && (opcode == GameShopBought || actor == 1)) {
                    const auto item = std::find_if(shop.offers.begin(), shop.offers.end(),
                        [&](const mir3_cui::NpcShop::Offer& offer) { return offer.id == purchase.offer; });
                    if (item != shop.offers.end()) shop.offers.erase(item);
                    if (shop.offer >= static_cast<int>(shop.offers.size()))
                        shop.offer = shop.sourceLayout ? -1 : static_cast<int>(shop.offers.size()) - 1;
                }
                static const char* const reasons[] = {"", "This item is sold out.", "You do not have enough gold.",
                    "The shop could not complete the purchase."};
                shop.status = opcode == GameShopBought ? "Purchase confirmed." : reasons[actor];
            } else if (!purchase.deadline && shop.sourceLayout && shop.offersVisible && shop.offer >= 0 &&
                       shop.offer < static_cast<int>(shop.offers.size()) && (opcode == GameShopBought || actor == 1)) {
                shop.offers.erase(shop.offers.begin() + shop.offer);
                if (shop.offer >= static_cast<int>(shop.offers.size())) shop.offer = -1;
            }
            if (shop.actor && opcode == GameShopRejected) shop.notice = 1436 + static_cast<int>(actor);
            purchase = {};
            return;
        }
        if (!actor || count > 255 || input_.size() > DEFBLOCKSIZE + (10240 * 4 + 2) / 3) return;
        auto body = ReadGameBody();
        if (input_.size() > DEFBLOCKSIZE && body.empty()) return;
        if (opcode == GameShopGoods) {
            if (actor != npcDialogue.actor || body.size() > 10240) return;
            const auto zero = body.find('\0');
            if (zero != std::string::npos) body.resize(zero);
            std::vector<mir3_cui::NpcShop::Good> goods;
            std::size_t start = 0;
            for (WORD index = 0; index < count; ++index) {
                std::array<std::string, 7> fields;
                for (auto& field : fields) {
                    const auto end = body.find('/', start);
                    if (end == std::string::npos || end == start) return;
                    field = body.substr(start, end - start);
                    start = end + 1;
                }
                mir3_cui::NpcShop::Good good;
                good.name = std::move(fields[0]);
                good.label = ConvertText(good.name, 936, CP_UTF8);
                if (good.label.empty()) return;
                unsigned* values[] = {&good.mode, &good.price, &good.stock, &good.icon,
                    &good.requirementType, &good.requirement};
                for (int i = 0; i < 6; ++i) {
                    *values[i] = Number(fields[i + 1], i == 3 ? 65535u : 0x7fffffffu, 0xffffffffu);
                    if (*values[i] == 0xffffffffu) return;
                }
                goods.push_back(std::move(good));
            }
            if (start != body.size()) return;
            shop = {};
            shop.actor = actor;
            shop.revision = ++shopRevision;
            shop.catalogueRevision = shop.revision;
            shop.goods = std::move(goods);
            shop.selected = shop.goods.empty() ? -1 : 0;
            return;
        }
        constexpr std::size_t encodedRecordSize = 134;
        if (actor != shop.actor || !shop.detailed || shop.category < 0 || body.size() > 8196 ||
            body.size() != count * (encodedRecordSize + 1)) return;
        std::vector<mir3_cui::NpcShop::Offer> offers;
        for (std::size_t start = 0; start < body.size(); start += encodedRecordSize + 1) {
            mir3_cui::InventoryItem item;
            if (body[start + encodedRecordSize] != '/' ||
                !DecodeInventoryItem(body.substr(start, encodedRecordSize), item, true) || !item.key.id) return;
            offers.push_back({std::move(item.key.name), std::move(item.label), item.key.id, item.maxDurability, item.icon});
        }
        shop.offers = std::move(offers);
        shop.offer = shop.offers.empty() ? -1 : 0;
        shop.page = offset / 10;
        shop.detailDeadline = 0;
        shop.offersVisible = true;
        shop.revision = ++shopRevision;
        shop.status.clear();
        shop.lastRow = -1;
    }

    static std::uint32_t PackedAppearance(const std::string& feature)
    {
        std::uint32_t packed = 0;
        for (int byte = 0; byte < 4; ++byte)
            packed |= std::uint32_t(static_cast<unsigned char>(feature[byte])) << (8 * byte);
        return packed;
    }

    void SendTradeOffer(unsigned amount)
    {
        trade.offerCount = amount;
        const auto& key = trade.offeredItem.key;
        Queue(ClientTradeOffer,0,key.name,key.id,static_cast<WORD>(amount));
    }

    static bool RestoreTradeItem(mir3_cui::InventoryPanel& bag, mir3_cui::InventoryItem item, int preferred = -1)
    {
        if (item.IsGold() || bag.Find(item.key)) return true;
        if (item.Stacks() && std::any_of(bag.items.begin(),bag.items.end(),[&](const auto& owned) {
            return owned.key.name == item.key.name;
        })) return true;
        if (bag.items.size()>=mir3_cui::InventoryPanel::Capacity) return false;
        item.cell = mir3_cui::InventoryPanel::FirstFit(bag.Occupied(),item,preferred);
        if (item.cell<0) return false;
        bag.items.push_back(std::move(item));
        return true;
    }

    void HandleTradeOfferReply(unsigned opcode, std::uint32_t id, WORD remaining)
    {
        if (trade.offerSlot<0 || !trade.offerCount) return;
        auto item = trade.offeredItem;
        auto nextBag = inventory;
        auto nextOffers = trade.items[0];
        if (opcode==GameTradeAddOk) {
            if (remaining && (remaining>=item.count || !id)) return;
            if (remaining) {
                item.count -= remaining;
                item.key.id = id;
            }
            if (!mir3_cui::TradePanel::Insert(nextOffers,item,trade.offerSlot)) return;
            if (remaining) {
                if (auto* owned=nextBag.Find(trade.offeredItem.key)) owned->count = remaining;
                else {
                    auto leftover = trade.offeredItem;
                    leftover.count = remaining;
                    if (!RestoreTradeItem(nextBag,leftover,leftover.cell)) return;
                }
            } else {
                nextBag.Remove(trade.offeredItem.key);
                for (auto& shortcut : nextBag.shortcuts)
                    if (shortcut.name==item.key.name) shortcut = {};
            }
        } else if (!RestoreTradeItem(nextBag,item,item.cell)) return;
        inventory = std::move(nextBag);
        trade.items[0] = std::move(nextOffers);
        if (opcode==GameTradeAddOk && !remaining) SaveItemShortcuts();
        trade.feedback.push_back({0,0,static_cast<int>(item.mode),static_cast<int>(item.subMode),item.label});
        if (equipment.held==trade.offeredItem.key) equipment.ReleaseHeld();
        trade.offerSlot = -1;
        trade.offerCount = 0;
        trade.offeredItem = {};
    }

    void HandleTradeReply(unsigned opcode, std::uint32_t actor, WORD first, WORD second)
    {
        using namespace mir3_cui;
        if (opcode==GameTradeFailed) {
            if (input_.size()==DEFBLOCKSIZE) trade.OpenPrompt(TradePrompt::Failed);
            return;
        }
        if (opcode==GameTradeOpen) {
            if (!trade.visible) {
                const auto body = ReadGameBody();
                if (body.empty() || body.size()>60 || body.find('\0')!=std::string::npos) return;
                auto partner = ConvertText(body,936,CP_UTF8);
                if (partner.empty()) return;
                trade.Clear();
                trade.partner = std::move(partner);
                trade.visible = true;
                ++trade.openRevision;
            }
            if (!inventory.open) {
                trade.repositionInventory = true;
                OpenInventory();
            }
            return;
        }
        if (!trade.visible) return;
        if (opcode==GameTradePeerAdd || opcode==GameTradePeerRemove) {
            InventoryItem item;
            if (!DecodeInventoryItem(input_.substr(DEFBLOCKSIZE),item) || !inventoryDimensions_.HasFrame(item.icon)) return;
            auto& peer = trade.items[1];
            if (opcode==GameTradePeerAdd) {
                if (!TradePanel::Insert(peer,item)) return;
            } else {
                const auto found = std::find_if(peer.begin(),peer.end(),[&](const auto& other) { return other.key==item.key; });
                if (found!=peer.end()) peer.erase(found);
            }
            trade.feedback.push_back({opcode==GameTradePeerAdd ? 1435 : 1436,0,static_cast<int>(item.mode),
                static_cast<int>(item.subMode),item.label});
            return;
        }
        if (input_.size()!=DEFBLOCKSIZE) return;
        switch (opcode) {
        case GameTradeAddOk:
        case GameTradeAddFail:
            HandleTradeOfferReply(opcode,actor,first);
            break;
        case GameTradeReturnOk:
        case GameTradeReturnFail:
            if (const auto* selected = trade.SelectedReturn(); selected && trade.returnPending) {
                const auto item = *selected;
                if (opcode==GameTradeReturnOk && !RestoreTradeItem(inventory,item)) return;
                auto& offers = trade.items[0];
                std::erase_if(offers,[&](const auto& other) { return other.cell==trade.returnSlot; });
                if (opcode==GameTradeReturnFail) TradePanel::Insert(offers,item);
                trade.returnPending = false;
                trade.returnSlot = -1;
                trade.feedback.push_back({0,0,static_cast<int>(item.mode),static_cast<int>(item.subMode),item.label});
            }
            break;
        case GameTradeGold:
            trade.gold[0] = actor;
            trade.goldPending = false;
            inventory.gold = first | (std::uint32_t(second)<<16);
            trade.feedback.push_back({0,106,-1,0,{}});
            break;
        case GameTradePeerGold:
            trade.gold[1] = actor;
            trade.feedback.push_back({0,106,-1,0,{}});
            break;
        case GameTradeCancelled: {
            auto restored = inventory;
            for (const auto& item : trade.items[0])
                if (!item.Stacks() && !RestoreTradeItem(restored,item)) return;
            if (trade.offerSlot>=0) {
                if (!RestoreTradeItem(restored,trade.offeredItem,trade.offeredItem.cell)) return;
                if (equipment.held==trade.offeredItem.key) equipment.ReleaseHeld();
            }
            restored.gold += trade.gold[0];
            inventory = std::move(restored);
            trade.Clear();
            break;
        }
        case GameTradeComplete:
            if (trade.offerSlot<0 && !trade.returnPending) {
                for (const auto& side : trade.items) for (const auto& item : side) {
                    if (item.stackMode<=2 || std::any_of(inventory.items.begin(),inventory.items.end(),
                        [&](const auto& owned) { return owned.icon==item.icon; })) continue;
                    for (auto& shortcut : inventory.shortcuts)
                        if (shortcut.name==item.key.name) shortcut = {};
                }
                SaveItemShortcuts();
                trade.Clear();
            }
            break;
        }
    }

    void HandleSkillRecords(bool full, int count)
    {
        auto next = skills;
        if (full) for (auto& group : next.learned) group.clear();
        else count = 1;
        std::string_view body(input_.data()+DEFBLOCKSIZE,input_.size()-DEFBLOCKSIZE);
        for (int i=0;i<count;++i) {
            const auto end = full ? body.find('/') : body.size();
            if (end == body.npos || end != 99) return;
            const auto record = DecodeGameBytes(std::string(body.substr(0,end)));
            if (record.size() != 74) return;
            const auto byte = [&](int at) { return static_cast<std::uint8_t>(record[at]); };
            const auto dword = [&](int at) {
                return byte(at) | (std::uint32_t(byte(at+1))<<8) | (std::uint32_t(byte(at+2))<<16) |
                    (std::uint32_t(byte(at+3))<<24);
            };
            const int id = byte(6) | (byte(7)<<8), level = byte(5);
            if (!id) return;
            const auto decimal = std::strtol(record.c_str()+4,nullptr,10);
            const int label = decimal>0 && decimal<10 ? int(decimal) : int(byte(4))-55;
            const int binding = full ? label : static_cast<std::uint8_t>(decimal);
            if (binding>=1 && binding<=12) next.bindings[binding] = byte(6);
            std::string_view name(record.data()+8,66);
            const std::array thresholds{static_cast<int>(dword(31)),static_cast<int>(dword(35)),static_cast<int>(dword(39))};
            next.learned[mir3::SkillLayout::category(id)].push_back({id,level,label>=1 && label<=12 ? label : 0,
                dword(0),level<3 ? thresholds[level] : 0,ConvertText(std::string(name.substr(0,name.find('\0'))),936,CP_UTF8),
                thresholds,dword(49),byte(53)});
            if (full) body.remove_prefix(end+1);
        }
        const int previous = next.tab;
        if (full) {
            next.tab = 0;
            next.menu_target = -1;
        }
        next.select_nonempty();
        if (full && next.learned[next.tab].empty()) next.tab = previous;
        skills = std::move(next);
    }

    void HandleMapUpdate(unsigned opcode, WORD x, WORD y, WORD atmosphere)
    {
        if (input_.size() > DEFBLOCKSIZE + 1024) return;
        auto body = ReadGameBody();
        if (body.empty() && input_.size()!=DEFBLOCKSIZE) return;
        const auto zero = body.find('\0');
        if (zero != std::string::npos) body.resize(zero);
        if (opcode == GameMapDescription) {
            if (body.size() <= 259) scene.mapDescription = ConvertText(body, 936, CP_UTF8);
            return;
        }
        if (body.empty() || body.size() > 64 || !std::all_of(body.begin(), body.end(), [](char c) {
            return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
                c == '_' || c == '-';
        })) return;
        if (!scene.terrain.OpenAtSpawn(body,resourceRoot_/"Map",x,y,scene.ground)) return;
        scene.mapId = std::move(body);
        CloseNpcDialogue();
        scene.pending = {};
        scene.pendingWalk.reset();
        scene.movementOrigin.reset();
        scene.walkPointerAt=0;
        scene.runPointerCaptured=false;
        scene.crouchedAt=scene.crouchReplyAt=scene.crouchHoldAt=0;
        scene.crouchPointerCaptured=false;
        scene.lastMove = 0;
        scene.players.clear();
        scene.groundItems.items.clear();
        scene.speech.clear();
        scene.levelEffects.clear();
        scene.levelPoseAt=0;
        if (!scene.dying) scene.casting={};
        scene.health={};
        scene.localActions.clear();
        scene.localStep={};
        scene.remoteMovementAt=0;
        scene.lastSpecialHit=0;
        scene.foldedHits=0;
        scene.spellMessages.clear();
        scene.spellEffects.reset();
        scene.spellSounds={};
        scene.lockedSpellActor=0;
        scene.spellReplyUntil=0;
        scene.pendingMelee.reset();
        scene.meleePointerCaptured=false;
        audio.ClearSceneEffects();
        scene.pickupBagFull=false;
        scene.x = x;
        scene.y = y;
        scene.mapEnteredAt=scene.weatherTick=GetTickCount64();
        scene.ambient.enter(atmosphere);
        scene.weather.select(atmosphere>>8,true);
        const bool reopen=minimap.visible || minimap.pending;
        const bool large=minimap.large, translucent=minimap.translucent;
        ClearMinimap();
        minimap.large=large;
        minimap.translucent=translucent;
        minimap.pending=reopen;
        portraitMapPending=portraitMapMode;
        if (reopen || portraitMapPending) RequestMinimap();
    }

    void RequestMinimap()
    {
        if (stage!=Stage::GameSession || minimapDeadline || output_.size()>=8192) return;
        Queue(ClientMinimap,0,"");
        minimapDeadline=GetTickCount64()+8000;
    }

    void ApplyGroundItem(std::uint32_t actor, WORD x, WORD y, WORD looks)
    {
        if (!actor || actor>0x7fffffffu || !scene.terrain.Contains(x,y)) return;
        auto name=ReadGameBody();
        if (name.empty() && input_.size()!=DEFBLOCKSIZE) return;
        if (const auto zero=name.find('\0');zero!=std::string::npos) name.resize(zero);
        int mode=0;
        if (const auto slash=name.find('/');slash!=std::string::npos) {
            mode=static_cast<int>(std::strtol(name.c_str(),nullptr,10));
            name.erase(0,slash+1);
        }
        auto& items=scene.groundItems.items;
        const auto size=items.size();
        if (scene.groundItems.show({static_cast<int>(actor),x,y,looks,std::move(name),mode}) && items.size()!=size) {
            auto& item=items.back();
            item.sparkle_started_tick=GetTickCount();
            const int first=std::rand();
            item.sparkle_delay_ms=mir3::GroundItems::sparkle_delay(first,std::rand());
        }
    }

    void AdvanceGroundItems(std::uint32_t now)
    {
        auto& items=scene.groundItems.items;
        std::erase_if(items,[&](const auto& item) {
            const int dx=item.x-int(scene.x), dy=item.y-int(scene.y);
            return dx<-24 || dx>=24 || dy<-24 || dy>=24;
        });
        for (auto& item:items) {
            const auto elapsed=now-item.sparkle_started_tick;
            if (elapsed<=unsigned(item.sparkle_delay_ms)) continue;
            item.sparkle_frame=static_cast<int>((elapsed-item.sparkle_delay_ms-1)/101);
            if (item.sparkle_frame>=mir3::GroundItems::sparkle_frames) {
                item.sparkle_frame=-1;
                item.sparkle_started_tick=now;
            }
        }
    }

    void ClearMinimap()
    {
        minimap={};
        minimapReply=0;
        minimapDeadline=0;
        minimapUnavailable=false;
        portraitMapPending=portraitMapReady=false;
    }

    void HandleLocalMovement(unsigned opcode, WORD x, WORD y, WORD direction)
    {
        const unsigned facing=direction&255;
        if (facing>=8 || (scene.localStep.frame>=0 && !scene.localStep.predicted()) ||
            (scene.terrain.Ready() && !scene.terrain.Contains(x,y))) return;
        const auto size = input_.size() - DEFBLOCKSIZE;
        if (opcode == GameBackstep && size != 29) return;
        if (opcode == GameTurn && (size < 29 || size > 1024)) return;
        if (opcode != GameTurn && size != 0 && size != 22 && size != 29) return;
        if (size) {
            const auto feature = DecodeGameBytes(input_.substr(DEFBLOCKSIZE, 22));
            if (feature.size() != 16) return;
            if (size >= 29 && DecodeGameBytes(input_.substr(DEFBLOCKSIZE + 22, 7)).size() != 5) return;
            if (size > 29 && DecodeGameBytes(input_.substr(DEFBLOCKSIZE + 29)).empty()) return;
            if (!characterAppearance.Apply(PackedAppearance(feature))) return;
        }
        if (scene.localStep.frame>=0 && scene.localStep.predicted() && opcode==scene.localStep.opcode &&
            scene.localStep.destinationX==x && scene.localStep.destinationY==y && scene.facing==facing) {
            scene.pending={};
            scene.light=direction>>8;
            return;
        }
        scene.localStep={};
        scene.pendingWalk.reset();
        scene.movementOrigin.reset();
        scene.SetCell(x, y);
        scene.facing = facing;
        if (opcode!=GameBackstep) scene.light=direction>>8;
        scene.pending = {};
        if (opcode==GameWalk || opcode==GameRun || opcode==GameBackstep) scene.crouchedAt=scene.crouchReplyAt=0;
    }

    void ResetLocalActorActions()
    {
        scene.pending={};
        scene.pendingWalk.reset();
        scene.movementOrigin.reset();
        scene.walkPointerAt=0;
        scene.runPointerCaptured=false;
        scene.localStep={};
        scene.pendingMelee.reset();
        scene.localActions.clear();
        scene.casting={};
        scene.spellMessages.clear();
        scene.spellReplyUntil=scene.levelPoseAt=scene.lastMove=0;
        scene.crouchedAt=scene.crouchReplyAt=scene.crouchHoldAt=0;
        scene.crouchPointerCaptured=scene.meleePointerCaptured=false;
        scene.lastSpecialHit=0;
        scene.foldedHits=0;
        scene.spellTiming={};
        scene.lockedSpellActor=0;
        audio.StopSceneEffect(0);
    }

    void HandleLocalLife(unsigned opcode,WORD x,WORD y,WORD direction)
    {
        const bool death=opcode==GameActorDeath || opcode==GameActorDead;
        const auto size=input_.size()-DEFBLOCKSIZE;
        if ((size!=29 && !(death && size==22)) || (direction&255)>=8 ||
            !scene.terrain.Contains(x,y) || scene.localActions.size()>=64) return;
        const auto feature=DecodeGameBytes(input_.substr(DEFBLOCKSIZE,22));
        const auto extra=size==29 ? DecodeGameBytes(input_.substr(DEFBLOCKSIZE+22,7)) : std::string(5,0);
        LocalScene::LocalLifeAction action{WORD(opcode),x,y,direction};
        if (feature.size()!=16 || extra.size()!=5 || !action.appearance.Apply(PackedAppearance(feature))) return;
        const auto [dx,dy]=SceneDirections[((direction&255)+(opcode==GameBackstep ? 4 : 0))%8];
        if (!death && !scene.terrain.Contains(int(x)-dx,int(y)-dy)) return;
        const unsigned shape=std::uint8_t(extra[0]);
        action.mount=shape<4 ? shape : 0;
        for (unsigned i=0;i<4;++i) action.flags|=std::uint32_t(std::uint8_t(feature[4+i]))<<(8*i);
        scene.localActions.emplace_back(action);
    }

    bool RequestSceneDoor(int x,int y)
    {
        if (scene.GroundAt(x,y)!=mir3_cui::GroundCell::DoorClosed) return false;
        const auto door=scene.terrain.DoorAt(x,y);
        if (!door) return false;
        Queue(ClientOpenDoor,WORD(x),"",door,0,WORD(y));
        return true;
    }

    void AdvanceWalk(std::uint64_t now)
    {
        if (!scene.pendingWalk || now<scene.pendingWalk->sendAt) return;
        const auto request=*scene.pendingWalk;
        scene.pendingWalk.reset();
        if (scene.dead || showSignOut || (characterStatus.hp[1] && !characterStatus.hp[0]) ||
            scene.x!=request.originX || scene.y!=request.originY || output_.size()>=8192) return;
        const int dx=int(request.x)-int(scene.x),dy=int(request.y)-int(scene.y);
        const int distance=std::max(std::abs(dx),std::abs(dy));
        const bool running=distance>1;
        if (!running) {
            for (const auto [x,y]:SceneDirections)
                if (RequestSceneDoor(int(request.x)+x,int(request.y)+y)) break;
        } else {
            const auto [stepX,stepY]=SceneDirections[request.facing];
            const int firstX=int(scene.x)+stepX,firstY=int(scene.y)+stepY;
            for (const auto [x,y]:SceneDirections)
                for (int ray=1;ray<=distance;++ray)
                    if (RequestSceneDoor(firstX+x*ray,firstY+y*ray)) break;
        }
        scene.movementOrigin=LocalScene::Movement{scene.x,scene.y,scene.facing,0};
        scene.facing=request.facing;
        scene.casting={};
        scene.levelPoseAt=scene.crouchedAt=0;
        scene.localStep={0,dx,dy,request.x,request.y,running ? GameRun : GameWalk,now+LocalScene::LocalStep::FrameMs};
        scene.pending={request.x,request.y,request.facing,now+8000};
        scene.actionClock.moved=static_cast<std::uint32_t>(now);
        const auto tick=request.admitted;
        Queue(running ? ClientRun : ClientWalk,WORD(tick),"",request.x|(std::uint32_t(request.y)<<16),
            WORD(tick>>16),WORD(request.facing));
    }

    void AdvanceLocalLife(std::uint64_t now)
    {
        if (scene.dying && scene.casting.clock.frame()>=9) {
            scene.dying=false;
            scene.casting={};
        }
        if (scene.dead && !scene.dying) characterStatus.hp[0]=0;
        auto& step=scene.localStep;
        if (step.frame>=0) {
            if (now<step.frameAt) return;
            step.frameAt=now+LocalScene::LocalStep::FrameMs;
            step.frame+=step.walking() ? 1 : 2;
            if (step.walking()) {
                if (mount.shape && step.opcode==GameRun) {
                    if (step.frame==3) PlaySceneSample(0,"35.wav",int(scene.x),int(scene.y));
                } else if (step.frame==1 || step.frame==4) {
                    const char* sample=!mount.shape ? "1.wav" : step.frame==1 ? "33.wav" : "34.wav";
                    PlaySceneSample(0,sample,int(scene.x),int(scene.y));
                }
            }
            if (step.opcode==GameBump && step.frame>=4) {
                scene.SetCell(int(step.destinationX)-step.dx,int(step.destinationY)-step.dy);
                step={};
                scene.casting.tick=now;
                scene.casting.clock.start(7);
            } else {
                const auto direction=(scene.facing+(step.opcode==GameBackstep ? 4 : 0))%8;
                if (step.frame>=(direction==7 ? 4 : 1)) scene.SetCell(step.destinationX,step.destinationY);
                if (step.frame<6) return;
                step={};
            }
        }
        if (scene.pendingMelee || scene.localActions.empty() ||
            (!scene.casting.clock.finished() && scene.casting.clock.action()!=7)) return;
        if (scene.pending.deadline && !scene.movementOrigin && scene.lastMove &&
            now-scene.lastMove<6*LocalScene::LocalStep::FrameMs) return;
        if (scene.pending.deadline) {
            if (!scene.lastMove) return;
            scene.SetCell(scene.pending.x,scene.pending.y);
            scene.facing=scene.pending.facing;
            scene.pending={};
        }
        auto record=std::move(scene.localActions.front());
        scene.localActions.pop_front();
        if (const auto* hit=std::get_if<LocalScene::HumanHit>(&record)) StartLocalHit(*hit,now);
        else if (const auto* status=std::get_if<LocalScene::HumanStatus>(&record)) ApplyLocalHumanStatus(*status);
        else ApplyLocalLife(std::get<LocalScene::LocalLifeAction>(record),now);
    }

    void ApplyLocalHumanStatus(const LocalScene::HumanStatus& status)
    {
        scene.actionFlags=status.flags;
        scene.meleeCooldown.speed=status.speed;
    }

    void ApplyLocalLife(const LocalScene::LocalLifeAction& action,std::uint64_t now)
    {
        characterAppearance=action.appearance;
        scene.pending={};
        scene.pendingWalk.reset();
        scene.movementOrigin.reset();
        scene.spellReplyUntil=0;
        scene.casting={};
        scene.crouchedAt=scene.crouchReplyAt=scene.levelPoseAt=0;
        scene.facing=action.direction&255;
        scene.light=action.direction>>8;
        scene.actionFlags=action.flags;
        if (action.opcode==GameActorDeath || action.opcode==GameActorDead) {
            scene.dying=!scene.dead && action.opcode==GameActorDead;
            scene.dead=true;
            scene.lockedSpellActor=0;
            scene.SetCell(action.x,action.y);
            if (scene.dying) {
                scene.casting.tick=now;
                scene.casting.clock.start(19);
            } else characterStatus.hp[0]=0;
            return;
        }
        if (action.opcode!=GameRush && action.opcode!=GameBump) mount.shape=action.mount;
        const auto [dx,dy]=SceneDirections[(scene.facing+(action.opcode==GameBackstep ? 4 : 0))%8];
        scene.SetCell(int(action.x)-dx,int(action.y)-dy);
        if (!scene.dead)
            scene.localStep={0,dx,dy,action.x,action.y,action.opcode,now+LocalScene::LocalStep::FrameMs};
    }

    static void SetRemoteName(LocalScene::Player& player, std::string body, int palette=-1)
    {
        const auto zero = body.find('\0');
        if (zero != std::string::npos) body.resize(zero);
        auto identity=player.identity;
        if (!identity.replace(body)) return;
        if (palette<0) {
            if (const auto slash=body.rfind('/');slash!=std::string::npos) {
                const char* start=body.c_str()+slash+1;
                char* end=nullptr;
                const auto value=std::strtol(start,&end,10);
                if (end!=start && !*end && value>=0 && value<=255) palette=static_cast<int>(value);
            }
        }
        const auto name = ConvertText(identity.lines[0],936,CP_UTF8);
        if (!identity.lines[0].empty() && name.empty()) return;
        if (palette>=0) identity.color=mir3::SceneName::colorref(static_cast<std::uint8_t>(palette));
        player.identity=std::move(identity);
        if (player.name!=name) {
            player.name = name;
            player.portraitJpeg.clear();
            player.resolvedPortraitVersion = 0;
            player.portraitQueryAt = 0;
        }
    }

    void ApplyTargetPortrait(std::uint32_t actor, std::uint32_t version)
    {
        const auto player = scene.players.find(actor);
        if (player==scene.players.end() || player->second.npcType) return;
        auto& target = player->second;
        if (!version && input_.size()==DEFBLOCKSIZE) {
            target.portraitJpeg.clear();
            target.resolvedPortraitVersion = target.portraitVersion;
            return;
        }
        if (!version || version!=target.portraitVersion || input_.size()>DEFBLOCKSIZE+5462) return;
        if (target.resolvedPortraitVersion==version && !target.portraitJpeg.empty()) return;
        auto bytes = ReadGameBody();
        if (bytes.size()<2 || bytes.size()>4096 || bytes.compare(0,2,"\xff\xd8")!=0) return;
        target.portraitJpeg = std::move(bytes);
        target.resolvedPortraitVersion = version;
    }

    void ApplyMonsterAttributes(std::uint32_t actor, WORD category, WORD state)
    {
        const auto found=scene.players.find(actor);
        if (found==scene.players.end() || !found->second.IsMonster() || !found->second.monsterAttributesRequested ||
            input_.size()!=DEFBLOCKSIZE+14) return;
        const auto body=ReadGameBody();
        if (body.size()!=10) return;
        auto& attributes=found->second.monsterAttributes;
        attributes.category=category;
        attributes.state=state;
        attributes.icon=static_cast<std::int8_t>(body[0]);
        attributes.element=static_cast<unsigned char>(body[2]);
        for (std::size_t i=0; i<attributes.modifiers.size(); ++i)
            attributes.modifiers[i]=static_cast<std::int8_t>(body[i+3]);
    }

    static void SetMonsterVitals(LocalScene::Player& monster, WORD current, WORD maximum)
    {
        const auto percent=maximum ? (std::min)(unsigned(current)*100/maximum,100u) : 0u;
        monster.monsterVitals={static_cast<std::uint8_t>(percent),maximum,current};
    }

    static unsigned MonsterShape(const std::string& feature)
    {
        return unsigned(static_cast<unsigned char>(feature[2])) |
            (unsigned(static_cast<unsigned char>(feature[3]))<<8);
    }

    static void BeginMonsterDeath(LocalScene::Player& monster, WORD opcode, std::uint64_t now, bool catchUp)
    {
        const bool animate=opcode==GameActorDead && !monster.dead;
        monster.dead=true;
        monster.monsterWalkFrame=monster.monsterCombatFrame=-1;
        monster.monsterSkeleton=opcode==GameMonsterSkeleton;
        monster.monsterDeathFrame=monster.monsterSkeleton || animate ? 0 : mir3::monster_death_motion(monster.npcShape).frames-1;
        monster.monsterDeathAt=now;
        monster.monsterReviveAt=0;
        monster.monsterCatchUp=catchUp;
        monster.monsterVitals.percent=0;
    }

    void StartMonsterAction(LocalScene::Player& monster, const LocalScene::Player::MonsterAction& move,
                            std::uint64_t now, bool catchUp)
    {
        if (move.opcode==GameActorStruck || move.opcode==GameActorStruckWithSource) {
            monster.npcShape=move.shape;
            SetMonsterVitals(monster,move.current,move.maximum);
            monster.monsterDamage.enabled=damageNumbers;
            if (!monster.dead) monster.monsterDamage.push(-int(move.damage));
            if (move.opcode==GameActorStruckWithSource && monster.monsterActions.size()<64) {
                auto relayed=move;
                relayed.opcode=GameActorStruck;
                monster.monsterActions.push_back(relayed);
            }
            if (monster.dead) {
                if (move.damage || !monster.monsterSkeleton) BeginMonsterDeath(monster,GameActorDeath,now,false);
                return;
            }
            if (!move.current) {
                BeginMonsterDeath(monster,GameActorDead,now,catchUp);
                return;
            }
            if (!move.damage) return;
            monster.monsterCombatMotion=mir3::monster_struck_motion(monster.npcShape);
            monster.monsterCombatFrame=0;
            monster.monsterCombatAt=now;
            monster.monsterCatchUp=catchUp;
            return;
        }
        if (move.opcode==GameMonsterAttack || move.opcode==GameMonsterAttackSpecial) {
            if (monster.dead) return;
            const bool special=move.opcode==GameMonsterAttackSpecial;
            if (!special) {
                monster.x=move.x;
                monster.y=move.y;
                monster.facing=move.facing;
            }
            const int action=special || monster.npcShape==141 ? 6 : monster.npcShape==151 ? 5 : 2;
            monster.monsterCombatMotion=mir3::monster_attack_motion(monster.npcShape,action);
            monster.monsterCombatFrame=0;
            monster.monsterCombatAt=now;
            monster.monsterCatchUp=catchUp;
            return;
        }
        if (monster.dead && (move.opcode==GameWalk || move.opcode==GameBackstep)) return;
        monster.x=move.x;
        monster.y=move.y;
        monster.facing=move.facing;
        monster.npcShape=move.shape;
        monster.monsterStatus=move.status;
        if (move.light>=0) monster.light=move.light;
        monster.monsterBackstep=move.opcode==GameBackstep;
        monster.monsterCatchUp=catchUp;
        monster.monsterSkipStep=false;
        monster.monsterCombatFrame=-1;
        if (move.opcode==GameActorDeath || move.opcode==GameActorDead || move.opcode==GameMonsterSkeleton) {
            BeginMonsterDeath(monster,move.opcode,now,catchUp);
        } else if (move.opcode==GameActorRevive || move.opcode==GameActorReviveRefresh || move.opcode==GameMonsterRise) {
            monster.dead=false;
            monster.monsterDeathFrame=-1;
            monster.monsterSkeleton=false;
            if (move.opcode==GameActorRevive) monster.monsterReviveAt=now;
        } else if (monster.dead && move.opcode==GameTurn) {
            BeginMonsterDeath(monster,GameActorDeath,now,false);
        }
        if (move.opcode==GameWalk || monster.monsterBackstep) {
            monster.monsterWalkFrame=0;
            monster.monsterNextStep=now+MonsterMovementTick;
        } else {
            monster.monsterWalkFrame=-1;
            monster.monsterNextStep=0;
            monster.monsterIdleAt=now;
        }
    }

    void QueueMonsterAction(LocalScene::Player& monster, const LocalScene::Player::MonsterAction& move)
    {
        if (monster.monsterActions.size()>1) {
            std::erase_if(monster.monsterActions,[&](const auto& queued) {
                if (queued.opcode!=GameActorStruck) return false;
                SetMonsterVitals(monster,queued.current,queued.maximum);
                return true;
            });
        }
        if (monster.MonsterAnimating() || !monster.monsterActions.empty()) {
            if (monster.monsterActions.size()<64) monster.monsterActions.push_back(move);
        } else StartMonsterAction(monster,move,GetTickCount64(),false);
    }

    void HandleMonsterCombat(unsigned opcode, std::uint32_t actor, WORD first, WORD second, WORD third)
    {
        const auto found=scene.players.find(actor);
        if (found==scene.players.end() || !found->second.IsMonster()) return;
        LocalScene::Player::MonsterAction move{.opcode=static_cast<WORD>(opcode)};
        if (opcode==GameActorStruck || opcode==GameActorStruckWithSource) {
            if (input_.size()!=DEFBLOCKSIZE+22) return;
            const auto feature=ReadGameBody();
            if (feature.size()!=16 || feature[0]!=3) return;
            move.shape=MonsterShape(feature);
            move.current=first;
            move.maximum=second;
            move.damage=third;
        } else if (opcode==GameMonsterAttack) {
            move.x=first;
            move.y=second;
            move.facing=third&0xff;
            if (move.facing>=8 || (scene.terrain.Ready() && !scene.terrain.Contains(first,second))) return;
        } else if (input_.size()!=DEFBLOCKSIZE) return;
        QueueMonsterAction(found->second,move);
    }

    void QueueHumanAction(LocalScene::Player& player,LocalScene::RemoteAction action)
    {
        auto& queue=player.remoteActions;
        if (queue.size()>=2) std::erase_if(queue,[&](const LocalScene::RemoteAction& record) {
            const auto* hit=std::get_if<LocalScene::HumanHit>(&record);
            if (!hit || hit->opcode!=GameActorStruck) return false;
            player.health.current=hit->current;
            player.health.maximum=hit->maximum;
            return true;
        });
        if (queue.size()<64) queue.push_back(std::move(action));
    }

    void HandleSceneHit(unsigned opcode,std::uint32_t actor,WORD hp,WORD maximum,WORD damage)
    {
        const bool local=actor && actor==localActor_;
        const auto found=scene.players.find(actor);
        if (!local && (found==scene.players.end() || found->second.IsNpc())) return;
        if (!local && found->second.IsMonster()) {
            HandleMonsterCombat(opcode,actor,hp,maximum,damage);
            return;
        }
        const bool full=input_.size()==DEFBLOCKSIZE+22;
        if (!full && (local || opcode!=GameActorStruck || input_.size()!=DEFBLOCKSIZE)) return;
        LocalScene::HumanHit hit{WORD(opcode),hp,maximum,damage};
        if (full) {
            const auto feature=ReadGameBody();
            if (feature.size()!=16 || !hit.appearance.Apply(PackedAppearance(feature))) return;
            for (unsigned i=0;i<4;++i) hit.source|=std::uint32_t(std::uint8_t(feature[8+i]))<<(8*i);
            hit.variant=std::uint8_t(feature[14])|(unsigned(std::uint8_t(feature[15]))<<8);
        }
        if (!local) {
            QueueHumanAction(found->second,hit);
            return;
        }
        auto& queue=scene.localActions;
        if (queue.size()>=64) return;
        if (opcode==GameActorStruck && !scene.dead) {
            if (scene.lastSpecialHit && GetTickCount64()-scene.lastSpecialHit<500) {
                characterStatus.hp={hp,maximum};
                if (hp) return;
            }
            scene.foldedHits=0;
            if (queue.size()>1) {
                std::erase_if(queue,[&](const LocalScene::LocalAction& record) {
                    const auto* older=std::get_if<LocalScene::HumanHit>(&record);
                    if (!older || older->opcode!=GameActorStruck) return false;
                    characterStatus.hp={older->current,older->maximum};
                    ++scene.foldedHits;
                    return true;
                });
            }
        }
        queue.push_back(hit);
    }

    unsigned SourceWeapon(std::uint32_t source) const
    {
        if (source && source==localActor_) return characterAppearance.weapon;
        if (const auto found=scene.players.find(source);found!=scene.players.end() && !found->second.npcType)
            return found->second.appearance.weapon;
        return 0;
    }

    void PlayHumanImpact(unsigned effect,std::uint32_t actor,unsigned x,unsigned y)
    {
        if (effect>65535 || !scene.terrain.Ready()) return;
        SceneEffects().followingImpact(effectCatalog_,effect,{int(x),int(y)},actor,
            [&](int sample,mir3::SpellCell cell) { return PlaySpellSound(sample,cell); });
    }

    void PlayHitSource(LocalScene::HumanHealth& health,std::uint32_t actor,std::uint32_t source,unsigned x,unsigned y)
    {
        if (source && source==localActor_) health.lastMonster.reset();
        else if (const auto found=scene.players.find(source);found!=scene.players.end()) {
            if (found->second.IsMonster()) health.lastMonster=std::uint8_t(found->second.npcShape);
            else health.lastMonster.reset();
        }
        if (!health.lastMonster) return;
        switch (*health.lastMonster) {
        case 2: PlayHumanImpact(500,actor,x,y); break;
        case 67: PlayHumanImpact(523,actor,x,y); break;
        case 104: PlayHumanImpact(542,actor,x,y); break;
        case 150: PlayHumanImpact(574,actor,x,y); break;
        }
    }

    void StartLocalHit(LocalScene::HumanHit hit,std::uint64_t now)
    {
        characterStatus.hp={hit.current,hit.maximum};
        characterAppearance=hit.appearance;
        scene.health.damage.enabled=damageNumbers;
        if (!scene.dead) scene.health.damage.push(-int(hit.damage));
        if (!scene.dead && hit.damage) {
            scene.crouchedAt=scene.crouchReplyAt=scene.levelPoseAt=0;
            auto& cast=scene.casting;
            cast={};
            cast.tick=now;
            cast.hitWeapon=SourceWeapon(hit.source);
            cast.clock.start(mount.shape ? 32 : 15);
            cast.clock.set_interval(std::max(80,200-3*int(std::uint8_t(characterStatus.level)))+
                (hit.variant ? 15*scene.foldedHits : 0));
            PlayHumanImpact(400+hit.variant,localActor_,scene.x,scene.y);
        }
        PlayHitSource(scene.health,localActor_,hit.source,scene.x,scene.y);
        if (scene.dead) characterStatus.hp[0]=0;
        if (hit.variant) scene.lastSpecialHit=now;
        if (hit.opcode==GameActorStruckWithSource && scene.localActions.size()<64) {
            hit.opcode=GameActorStruck;
            scene.localActions.push_back(hit);
        }
    }

    void StartRemoteHit(LocalScene::Player& player,std::uint32_t actor,const LocalScene::HumanHit& hit,
                        std::uint64_t now,bool catchUp)
    {
        auto& health=player.health;
        health.current=hit.current;
        health.maximum=hit.maximum;
        if (!hit.appearance.available) return;
        if (hit.opcode==GameActorStruckWithSource && player.remoteActions.size()<64) {
            auto relay=hit;
            relay.opcode=GameActorStruck;
            player.remoteActions.emplace_back(relay);
        }
        PlayHitSource(health,actor,hit.source,player.x,player.y);
        if (player.dead) return;
        player.appearance=hit.appearance;
        health.damage.enabled=damageNumbers;
        health.damage.push(-int(hit.damage));
        if (!hit.damage) return;
        auto& cast=player.casting;
        cast={};
        cast.tick=now;
        cast.hitWeapon=SourceWeapon(hit.source);
        cast.flags=player.actionFlags;
        cast.clock.start(player.mount ? 32 : 15,catchUp);
        player.crouchedAt=player.levelPoseAt=0;
        PlayHumanImpact(400+hit.variant,actor,player.x,player.y);
    }

    void AdvanceMonsterActions(std::uint64_t now)
    {
        for (auto& [id,monster]:scene.players) {
            if (monster.monsterReviveAt && now-monster.monsterReviveAt>=MonsterReviveDuration) monster.monsterReviveAt=0;
            if (monster.MonsterDying()) {
                const auto death=mir3::monster_death_motion(monster.npcShape);
                const bool fast=monster.monsterCatchUp && monster.monsterDeathFrame>0;
                const bool slowed=monster.monsterStatus&0x08000000u;
                const int interval=(fast ? death.interval/2 : death.interval)*(slowed ? 2 : 1);
                if (now-monster.monsterDeathAt<=static_cast<std::uint64_t>(interval)) continue;
                monster.monsterDeathAt=now;
                ++monster.monsterDeathFrame;
                if (monster.MonsterDying()) continue;
            }
            if (monster.monsterWalkFrame>=0) {
                if (now<monster.monsterNextStep) continue;
                monster.monsterNextStep=(now-monster.monsterNextStep>=MonsterMovementTick ? now : monster.monsterNextStep)+MonsterMovementTick;
                const bool slowed=(monster.monsterStatus&0x08000000u) && !monster.monsterBackstep;
                const bool advance=!slowed || !monster.monsterSkipStep;
                monster.monsterSkipStep=slowed && advance;
                if (advance) monster.monsterWalkFrame+=(monster.monsterBackstep || monster.monsterCatchUp) ? 2 : 1;
                if (monster.monsterWalkFrame<6) continue;
                monster.monsterWalkFrame=-1;
                monster.monsterIdleAt=now;
            }
            if (monster.monsterCombatFrame>=0) {
                const auto& motion=monster.monsterCombatMotion;
                const bool fast=monster.monsterCatchUp && monster.monsterCombatFrame>0;
                const bool slowed=monster.monsterStatus&0x08000000u;
                const int interval=(fast ? motion.interval/2 : motion.interval)*(slowed ? 2 : 1);
                if (now-monster.monsterCombatAt<=static_cast<std::uint64_t>(interval)) continue;
                monster.monsterCombatAt=now;
                if (++monster.monsterCombatFrame<motion.frames) continue;
                monster.monsterCombatFrame=-1;
                monster.monsterIdleAt=now;
            }
            if (monster.monsterActions.empty()) continue;
            const bool catchUp=monster.monsterActions.size()>=3;
            const auto move=monster.monsterActions.front();
            monster.monsterActions.pop_front();
            StartMonsterAction(monster,move,now,catchUp);
        }
    }

    void HandleRemotePlayer(unsigned opcode, std::uint32_t actor, WORD x, WORD y, WORD direction)
    {
        const unsigned facing=direction&255;
        if (!actor || actor > 0x7fffffffu || facing >= 8 ||
            (scene.terrain.Ready() && !scene.terrain.Contains(x, y))) return;
        const auto size = input_.size() - DEFBLOCKSIZE;
        if (size > 344) return;
        const auto feature = size >= 22 ? DecodeGameBytes(input_.substr(DEFBLOCKSIZE, 22)) : std::string();
        const unsigned type = feature.size() == 16 ? static_cast<unsigned char>(feature[0]) : 0;
        if (opcode==GameBump && (type>=3 || !scene.players.contains(actor) || scene.players.at(actor).npcType)) return;
        if (type == 3) {
            const auto existing=scene.players.find(actor);
            const bool known=existing!=scene.players.end() && existing->second.IsMonster();
            if ((opcode==GameActorDead || opcode==GameActorRevive || opcode==GameActorReviveRefresh) &&
                !known) return;
            if (opcode==GameRun && known) return;
            const auto shape=MonsterShape(feature);
            std::uint32_t status=0;
            for (unsigned i=0;i<4;++i) status|=std::uint32_t(static_cast<unsigned char>(feature[4+i]))<<(8*i);
            auto name=size>22 ? DecodeGameBytes(input_.substr(DEFBLOCKSIZE+22)) : std::string{};
            if (size>22 && name.empty()) return;
            auto& monster=scene.players[actor];
            if (!known) monster={};
            monster.npcType=type;
            LocalScene::Player::MonsterAction move{x,y,opcode==GameRun ? 0u : facing,shape,status,static_cast<WORD>(opcode)};
            if (opcode!=GameBackstep) move.light=direction>>8;
            QueueMonsterAction(monster,move);
            if (!name.empty()) SetRemoteName(monster,std::move(name));
            return;
        }
        if (opcode==GameMonsterDigUp || opcode==GameMonsterRise || opcode==GameMonsterSkeleton) return;
        if (type == 4 || type == 5) {
            auto& npc = scene.players[actor];
            if (!npc.IsNpc()) npc={};
            npc.x = x;
            npc.y = y;
            npc.facing = facing;
            npc.light = direction >> 8;
            npc.npcType = type;
            npc.npcShape = static_cast<unsigned char>(feature[type == 4 ? 1 : 2]);
            npc.dead = opcode == GameActorDeath || opcode == GameActorDead;
            if (size > 22) {
                auto name = DecodeGameBytes(input_.substr(DEFBLOCKSIZE + 22));
                if (!name.empty()) SetRemoteName(npc, std::move(name));
            }
            return;
        }
        const bool named = opcode == GameTurn || opcode == GameActorRevive || opcode == GameActorReviveRefresh;
        if (named && (size < 29 || size > 344)) return;
        if (opcode == GameBackstep && size != 29) return;
        if (!named && size != 0 && size != 22 && size != 29) return;
        auto found = scene.players.find(actor);
        const bool newPlayer = found == scene.players.end();
        if (!newPlayer && found->second.npcType) return;
        if (newPlayer && size < 29) return;
        LocalScene::RemoteMovement move{WORD(opcode),x,y,direction};
        if (size) {
            if (feature.size()!=16 || !move.appearance.Apply(PackedAppearance(feature))) return;
            for (unsigned i=0;i<4;++i) {
                move.flags|=std::uint32_t(std::uint8_t(feature[4+i]))<<(8*i);
                move.portrait|=std::uint32_t(std::uint8_t(feature[8+i]))<<(8*i);
            }
            const auto extra=size>=29 ? DecodeGameBytes(input_.substr(DEFBLOCKSIZE+22,7)) : std::string{};
            if (size>=29 && extra.size()!=5) return;
            const unsigned shape=extra.empty() ? 0 : std::uint8_t(extra[0]);
            move.mount=shape<4 ? shape : 0;
            if (size > 29) {
                move.name=DecodeGameBytes(input_.substr(DEFBLOCKSIZE+29));
                if (move.name.empty()) return;
            }
        }
        if (newPlayer) {
            LocalScene::Player player;
            if (StartRemoteMovement(player,actor,move,false)) scene.players.emplace(actor,std::move(player));
            return;
        }
        QueueRemotePosition(found->second,actor,std::move(move));
    }

    void QueueRemotePosition(LocalScene::Player& player,std::uint32_t actor,LocalScene::RemoteMovement move)
    {
        if (player.remoteStep.frame>=0 || !player.casting.clock.finished() || !player.remoteActions.empty())
            QueueHumanAction(player,std::move(move));
        else StartRemoteMovement(player,actor,move,false);
    }

    bool StartRemoteMovement(LocalScene::Player& player,std::uint32_t actor,
                             const LocalScene::RemoteMovement& move,bool catchUp)
    {
        const auto opcode=move.opcode;
        const bool moving=opcode==GameWalk || opcode==GameRun || opcode==GameBackstep || opcode==GameBump;
        if (moving && player.dead) return false;
        const unsigned facing=move.direction&255;
        const unsigned mount=move.appearance.available ? move.mount : player.mount;
        const int distance=opcode==GameRun ? (mount ? 3 : 2) : moving ? 1 : 0;
        const auto [dx,dy]=SceneDirections[(facing+(opcode==GameBackstep ? 4 : 0))%8];
        const int originX=int(move.x)-distance*dx,originY=int(move.y)-distance*dy;
        if (moving && !scene.terrain.Contains(originX,originY)) return false;
        const bool death=opcode==GameActorDeath || opcode==GameActorDead;
        if (move.appearance.available) {
            player.appearance=move.appearance;
            player.actionFlags=move.flags;
            if (!death) player.mount=move.mount;
            if (player.portraitVersion!=move.portrait) {
                player.portraitVersion=move.portrait;
                player.resolvedPortraitVersion=0;
                player.portraitJpeg.clear();
                player.portraitQueryAt=0;
            }
            if (!move.name.empty()) SetRemoteName(player,move.name);
        }
        player.facing=facing;
        player.crouchedAt=0;
        player.x=unsigned(originX);
        player.y=unsigned(originY);
        if (opcode==GameCrouch || opcode==GameButcher) {
            if (!player.dead) player.crouchedAt=GetTickCount64();
            return true;
        }
        if (opcode!=GameBump && !death) player.light=move.direction>>8;
        if (death) {
            player.dying=!player.dead && opcode==GameActorDead;
            player.dead=true;
            player.revivedAt=0;
            player.casting={};
            player.combatUntil=player.combatIdleAt=0;
            player.levelPoseAt=0;
            if (scene.lockedSpellActor==actor) scene.lockedSpellActor=0;
            if (player.dying) {
                player.casting.clock.start(19,catchUp);
                player.casting.flags=player.actionFlags;
                player.casting.tick=GetTickCount64();
            }
        } else if (opcode==GameActorRevive || opcode==GameActorReviveRefresh) {
            player.dead=player.dying=false;
            player.revivedAt=GetTickCount64();
            player.combatUntil=player.combatIdleAt=0;
            player.levelPoseAt=0;
        }
        if (moving) {
            player.levelPoseAt=0;
            player.remoteStep={0,distance*dx,distance*dy,move.x,move.y,opcode,catchUp};
        }
        return true;
    }

    void AdvanceRemoteMovement(LocalScene::Player& player,std::uint32_t actor,std::uint64_t now)
    {
        if (player.actionFlags&0x08000000u) {
            player.remoteSlowTick=!player.remoteSlowTick;
            if (player.remoteSlowTick) return;
        }
        auto& step=player.remoteStep;
        if (step.opcode==GameBump || step.opcode==GameBackstep) step.frame+=2;
        else {
            ++step.frame;
            const char* sample=nullptr;
            if (player.mount && step.opcode==GameRun) {
                if (step.frame==3) sample="35.wav";
            } else if (step.frame==1 || step.frame==4) sample=!player.mount ? "1.wav" : step.frame==1 ? "33.wav" : "34.wav";
            if (sample) PlaySceneSample(actor,sample,int(player.x),int(player.y));
            step.frame+=int(step.catchUp);
        }
        if (step.opcode==GameBump && step.frame>=4) {
            player.x=unsigned(int(step.destinationX)-step.dx);
            player.y=unsigned(int(step.destinationY)-step.dy);
            step={};
            player.casting={};
            player.casting.tick=now;
            player.casting.clock.start(7);
            return;
        }
        if (step.frame>=(player.facing==7 ? 4 : 1)) {
            player.x=step.destinationX;
            player.y=step.destinationY;
        }
        if (step.frame>=6) {
            step={};
            player.combatIdleAt=now;
        }
    }

    static unsigned SpellFacing(int sourceX, int sourceY, int x, int y)
    {
        const int dx=x-sourceX, dy=y-sourceY;
        const std::array<int,2> direction{
            std::abs(dy)>2 && std::abs(dx)<=1 ? 0 : (dx>0)-(dx<0),
            std::abs(dx)>2 && std::abs(dy)<=1 ? 0 : (dy>0)-(dy<0)};
        const auto heading=std::find(SceneDirections.begin(),SceneDirections.end(),direction);
        return heading==SceneDirections.end() ? 4 : static_cast<unsigned>(heading-SceneDirections.begin());
    }

    static bool UsesLockedSpellTarget(int skill)
    {
        using mir3::SkillId;
        switch (static_cast<SkillId>(skill)) {
        case SkillId::fireball: case SkillId::fireball2: case SkillId::amyounsul:
        case SkillId::lightening: case SkillId::firecharm: case SkillId::tamming: case SkillId::killundead:
        case SkillId::original_37: case SkillId::original_38: case SkillId::original_39: case SkillId::original_40:
        case SkillId::original_41: case SkillId::original_67: case SkillId::original_74: return true;
        default: return false;
        }
    }

    void HandleSpellEffects(bool multiple,std::uint32_t actor,WORD x,WORD y,WORD series)
    {
        if ((!multiple && input_.size()!=DEFBLOCKSIZE+6) || input_.size()>DEFBLOCKSIZE+1366) return;
        const auto decoded=ReadGameBody();
        if (decoded.empty() && input_.size()!=DEFBLOCKSIZE) return;
        auto reply=mir3::SpellReply::parse(x,y,series,multiple,decoded);
        if (!reply) return;
        if (actor && actor==localActor_) {
            if (scene.spellMessages.size()<64) scene.spellMessages.push_back(std::move(*reply));
        } else if (const auto it=scene.players.find(actor);it!=scene.players.end() &&
            !it->second.IsNpc() && !it->second.IsMonster() && it->second.remoteActions.size()<64) {
            QueueHumanAction(it->second,std::move(*reply));
        }
    }

    std::optional<mir3::SpellPixel> SpellTarget(std::uint32_t actor) const
    {
        if (actor && actor==localActor_) {
            const auto pixel=LocalActorPixel(scene);
            return mir3::SpellPixel{float(pixel[0]),float(pixel[1])};
        }
        if (const auto it=scene.players.find(actor);it!=scene.players.end()) return ActorWorldPixel(it->second);
        return {};
    }

    mir3::SpellEffects& SceneEffects()
    {
        if (!scene.spellEffects) {
            scene.spellEffects=std::make_unique<mir3::SpellEffects>();
            scene.spellEffectsTick=GetTickCount64();
        }
        return *scene.spellEffects;
    }

    void PlaySpellEffects(const mir3::SpellReply& reply,unsigned x,unsigned y,unsigned facing)
    {
        if (!scene.terrain.Ready()) return;
        SceneEffects().play(effectCatalog_,reply,{int(x),int(y)},facing,
            [&](auto actor) { return SpellTarget(actor); },
            [&](int sample,mir3::SpellCell cell) { return PlaySpellSound(sample,cell); },
            [&](mir3::SpellCell cell) { return scene.terrain.BaseWalkable(cell[0],cell[1]); });
    }

    void AdvanceSpellEffects(std::uint64_t now)
    {
        if (scene.spellEffects && now>scene.spellEffectsTick) {
            const auto ms=(std::min)(now-scene.spellEffectsTick,std::uint64_t(250));
            const auto camera=LocalActorCamera(scene);
            scene.spellEffects->advance(effectCatalog_,static_cast<unsigned>(ms),camera,
                [&](auto actor) { return SpellTarget(actor); },
                [&](int sample,mir3::SpellCell cell) { return PlaySpellSound(sample,cell); },
                [&](std::int64_t voice,int sample,mir3::SpellPixel pixel) {
                    using Voices=mir3::SpellSoundVoices;
                    const auto index=Voices::key(0,0)-voice;
                    if (index<0 || index>=Voices::samples*Voices::voices || index%Voices::voices>=5 ||
                        scene.spellSounds.cache[index/Voices::voices].id!=sample) return;
                    const auto mix=mir3::SoundPosition::from_view(int(pixel[0]/48)*48-camera[0],int(pixel[1]/32)*32-camera[1]);
                    audio.MoveSceneEffect(voice,mix.volume,mix.pan,audioOptions);
                },scene.weather);
            scene.spellEffectsTick=now;
        }
        if (scene.spellMessages.empty() || scene.pending.deadline || !scene.casting.clock.finished()) return;
        const auto reply=std::move(scene.spellMessages.front());
        scene.spellMessages.pop_front();
        PlaySpellEffects(reply,scene.x,scene.y,scene.facing);
    }

    std::int64_t PlaySpellSound(int sample,mir3::SpellCell cell)
    {
        if (!audioOptions.effects) return 0;
        const auto sound=sceneSoundList_.file(sample);
        if (sound.empty()) return 0;
        const auto voice=scene.spellSounds.select(sample,GetTickCount(),
            [&](std::int64_t key) { return audio.SceneEffectPlaying(key); });
        if (!voice) return 0;
        if (voice->replaced) for (int i=0;i<mir3::SpellSoundVoices::voices;++i)
            audio.StopSceneEffect(mir3::SpellSoundVoices::key(voice->cache,i));
        if (voice->play) PlaySceneSample(voice->key,sound,cell[0],cell[1]);
        return voice->key;
    }

    void PlaySceneSample(std::int64_t actor,const std::string& sound,int x,int y)
    {
        const auto camera=LocalActorCamera(scene);
        const auto mix=mir3::SoundPosition::from_view(x*48-camera[0],y*32-camera[1]);
        const auto path=resourceRoot_/"Sound"/std::filesystem::u8path(sound).filename();
        audio.PlaySceneEffect(actor,path.c_str(),mix.volume,mix.pan,audioOptions);
    }

    void PlaySceneSound(std::uint32_t actor,int sample,unsigned x,unsigned y)
    {
        const auto sound=sceneSoundList_.file(sample);
        if (sound.empty()) return;
        PlaySceneSample(actor,sound,int(x),int(y));
    }

    static bool IsMeleeMessage(unsigned opcode)
    {
        switch (opcode) {
        case GameFireHit: case GameMonsterAttack: case GameMiningHit: case GamePowerHit: case GameLongHit:
        case GameWideHit: case GameWideHit2: case GameRoundHit: case GameBackHit:
        case GameHumanAction18: case GameHumanAttack: return true;
        default: return false;
        }
    }

    void HandleMeleeMessage(unsigned opcode,std::uint32_t actor,WORD x,WORD y,WORD direction)
    {
        const unsigned facing=direction&255;
        const bool miningBody=opcode==GameMiningHit && input_.size()==DEFBLOCKSIZE+1;
        const bool harvest=miningBody && input_[DEFBLOCKSIZE]!=0;
        const auto found=scene.players.find(actor);
        if (actor!=localActor_ && found!=scene.players.end() && found->second.IsMonster()) {
            if (opcode==GameMonsterAttack) HandleMonsterCombat(opcode,actor,x,y,direction);
            return;
        }
        if (facing>=8 || (input_.size()!=DEFBLOCKSIZE && !miningBody)) return;
        if (actor==localActor_) {
            if (opcode!=GameMiningHit || scene.dead || !characterStatus.hp[0]) return;
            auto& cast=scene.casting;
            const int action=mir3::HumanActions::weapon_attack(characterAppearance.weapon);
            if (cast.clock.finished() || cast.clock.action()!=action || scene.facing!=facing || cast.clock.frame()>=3) {
                scene.pending={};
                scene.pendingMelee.reset();
                scene.facing=facing;
                scene.crouchedAt=scene.levelPoseAt=0;
                StartMelee(cast,action,0,facing,0,scene.actionFlags,0,scene.x,scene.y,GetTickCount64());
            }
            cast.harvest=harvest;
            return;
        }
        if (found==scene.players.end() || found->second.IsNpc()) return;
        if ((opcode==GameRoundHit || opcode==GameBackHit || opcode==GameHumanAction18) &&
            !scene.terrain.BaseWalkable(x,y)) return;
        auto& player=found->second;
        if (scene.terrain.Contains(x,y))
            QueueHumanAction(player,LocalScene::RemoteMelee{x,y,facing,unsigned(direction>>8),opcode,harvest});
    }

    void StartRemoteMelee(LocalScene::Player& player,std::uint32_t actor,const LocalScene::RemoteMelee& next,
        std::uint64_t tick,bool catchUp)
    {
        if (player.mount) return;
        using S=mir3::SkillId;
        int skill=0,action=mir3::HumanActions::weapon_attack(player.appearance.weapon);
        switch (next.opcode) {
        case GameFireHit: skill=int(S::firesword); break;
        case GamePowerHit: skill=int(S::yedo); break;
        case GameLongHit: skill=int(S::ergum); break;
        case GameWideHit: case GameWideHit2:
            skill=int(next.opcode==GameWideHit ? S::banwol : S::original_103);
            action=mir3::HumanActions::weapon_attack(player.appearance.weapon,false);
            break;
        case GameRoundHit: skill=int(S::randswing); action=17; break;
        case GameBackHit: skill=int(S::spacemove2); action=16; break;
        case GameHumanAction18: action=18; break;
        case GameHumanAttack: action=9; break;
        }
        if (next.opcode!=GameBackHit) player.facing=next.facing;
        player.x=next.x; player.y=next.y;
        player.crouchedAt=player.levelPoseAt=0;
        if (tick>=player.combatUntil) player.combatUntil=tick+5001;
        const auto* resource=effectCatalog_.find(mir3::EffectCatalog::Section::spell,WORD(skill));
        const int sound=resource && skill ? mir3::MeleeSpells::frame_sound(skill,player.appearance.body) : 0;
        const int effect=next.opcode==GameMonsterAttack ? mir3::SpellColor::weapon_effect(next.element) : skill;
        StartMelee(player.casting,action,effect,next.opcode==GameWideHit2 ? 0 : next.facing,sound,player.actionFlags,
            actor,player.x,player.y,tick,next.element,catchUp);
        player.casting.harvest=next.harvest;
    }

    void AdvanceMelee(std::uint64_t now)
    {
        if (!scene.pendingMelee || now<scene.pendingMelee->at) return;
        const auto pending=*scene.pendingMelee;
        scene.pendingMelee.reset();
        if (showSignOut || scene.dead || mount.shape || !characterStatus.hp[0] || !scene.terrain.Ready() ||
            scene.x!=pending.x || scene.y!=pending.y || output_.size()>=8192) return;
        const auto tick=static_cast<std::uint32_t>(now);
        const auto [dx,dy]=SceneDirections[pending.facing];
        const int x=int(scene.x),y=int(scene.y);
        using Attack=mir3::MeleeSpells;
        const auto weapon=characterAppearance.weapon;
        Attack::Attack selected{0,pending.opcode,mir3::HumanActions::weapon_attack(weapon),int(pending.facing),0};
        if (pending.opcode==Attack::hit)
            selected=scene.meleeSpells.select(weapon,int(pending.facing),characterStatus.mp[0],
                scene.Occupied(x+2*dx,y+2*dy),characterAppearance.body);
        scene.movementOrigin.reset();
        scene.spellOriginalFacing=scene.facing;
        scene.spellReplyUntil=now+8000;
        scene.facing=pending.facing;
        scene.levelPoseAt=scene.crouchedAt=scene.crouchReplyAt=0;
        scene.actionClock.attacked=tick;
        const auto& record=equipment.slots[1].serverRecord;
        const int effect=selected.skill ? selected.skill : mir3::SpellColor::local_weapon_effect(record);
        StartMelee(scene.casting,selected.action,effect,selected.facing,selected.sound,scene.actionFlags,0,scene.x,scene.y,now);
        Queue(WORD(selected.opcode),WORD(tick),"",scene.x|(std::uint32_t(scene.y)<<16),WORD(tick>>16),WORD(scene.facing));
    }

    void StartMelee(mir3::CastVisual& cast,int action,int effect,unsigned facing,int sound,std::uint32_t flags,
        std::uint32_t actor,unsigned x,unsigned y,std::uint64_t tick,unsigned element=255,bool catchUp=false)
    {
        cast={};
        cast.weapon=true;
        cast.clock.start(action,catchUp);
        cast.flags=flags;
        cast.tick=tick;
        cast.direction=int(facing);
        cast.meleeSound=sound;
        if (effect) if (const auto* resource=effectCatalog_.find(mir3::EffectCatalog::Section::spell,WORD(effect))) {
            cast.resource=*resource;
            cast.tint=resource->tint;
            if (!actor) cast.tint=mir3::SpellColor::local_weapon_color(equipment.slots[1].serverRecord,characterStatus.attackElements,cast.tint);
            else if (element<mir3::SpellColor::palette.size()) cast.tint=mir3::SpellColor::palette[element];
            PlaySpellSound(10*(effect+1000),{int(x),int(y)});
        }
    }

    void AdvanceHumanAction(mir3::CastVisual& cast,std::uint32_t actor,unsigned x,unsigned y,
                            const mir3_cui::CharacterAppearance& appearance,std::uint64_t now)
    {
        const int previous=cast.clock.frame();
        cast.advance(now);
        const int frame=cast.clock.frame();
        if (previous==frame) return;
        const int action=cast.clock.action();
        if (action==15)
            PlaySceneSound(actor,mir3::HumanActions::hit_sound(frame,cast.hitWeapon,appearance.clothing,appearance.body),x,y);
        if (!cast.weapon) return;
        if (frame==1 && cast.meleeSound) PlaySpellSound(cast.meleeSound,{int(x),int(y)});
        else if (frame==1 && (action==16 || action==17)) PlaySceneSound(actor,action==16 ? 141 : 140,x,y);
        if (frame==2 && action>=9 && action<=14)
            PlaySceneSound(actor,mir3::HumanActions::weapon_swing_sound(appearance.weapon),x,y);
        if (frame==3 && cast.harvest)
            SceneEffects().harvest(effectCatalog_,{int(x),int(y)},[this](int sound,mir3::SpellCell cell) {
                return PlaySpellSound(sound,cell);
            });
    }

    void StartCast(mir3::CastVisual& cast,int skill,mir3::SpellAction action,std::uint32_t actor,
        unsigned x,unsigned y,unsigned facing,std::uint64_t tick,bool catchUp=false)
    {
        cast={};
        cast.tick=tick;
        if (action==mir3::SpellAction::special) {
            cast.clock.start(int(action));
            PlaySceneSound(actor,142,x,y);
            return;
        }
        const auto& route=mir3::SpellRoutes::get(skill);
        cast.clock.start_spell(int(action),route.action_limit,catchUp);
        if (route.action_interval) cast.clock.set_interval(route.action_interval);
        cast.direction=route.directional_effect ? int(facing) : 0;
        if (const auto* effect=effectCatalog_.find(mir3::EffectCatalog::Section::spell,WORD(skill))) {
            cast.resource=*effect;
            cast.tint=effect->tint;
            if (!actor) cast.tint=mir3::SpellColor::strongest(characterStatus.attackElements,cast.tint);
            PlaySceneSound(actor,10*(skill+1000),x,y);
        }
    }

    void HandleLevelUp(std::uint32_t recognition, WORD first, WORD branch)
    {
        if (input_.size()!=DEFBLOCKSIZE || branch>1) return;
        const auto now=GetTickCount64();
        unsigned x=scene.x, y=scene.y;
        if (branch==0) {
            characterStatus.level=first&255;
            characterStatus.experience=recognition;
            scene.facing=4;
            scene.pending={};
            scene.crouchedAt=scene.crouchReplyAt=0;
            if (!scene.dead && scene.localStep.frame<0) {
                scene.levelPoseAt=now;
                scene.casting={};
            }
            const auto text=ConvertText(sceneMessages_.get(1469),936,CP_UTF8);
            chat.Append(100,text.empty() ? "Level up: "+std::to_string(characterStatus.level) : text,0xffffff,33023);
        } else {
            const auto found=scene.players.find(recognition);
            if (found==scene.players.end() || found->second.IsNpc() || found->second.IsMonster()) return;
            auto& player=found->second;
            x=player.x; y=player.y;
            player.facing=4;
            player.crouchedAt=0;
            player.levelPoseAt=now;
            player.casting={};
        }
        if (!scene.terrain.Ready()) return;
        using Effect=mir3::LevelUpEffect;
        const auto* resource=effectCatalog_.find(mir3::EffectCatalog::Section::magic,Effect::resource_id);
        if (!resource || resource->library!=137 || resource->blend!=2 || resource->end<=resource->first ||
            resource->end>INT_MAX || scene.levelEffects.size()>=256) return;
        scene.levelEffects.push_back({Effect(*resource,int(x),int(y)),now});
        PlaySceneSound(branch==0 ? 0 : recognition,Effect::sound_id,x,y);
    }

    void HandleCharacterStatus(unsigned opcode, std::uint32_t actor, WORD first, WORD second, WORD third)
    {
        auto& stats = characterStatus;
        if (opcode == GameAbility) {
            if (stats.ApplyAbility(ReadGameBody(), first & 0xff)) inventory.gold = actor;
            return;
        }
        if (input_.size() != DEFBLOCKSIZE) return;
        switch (opcode) {
        case GameVitals:
            if (actor == 0 || actor == localActor_) {
                scene.health.damage.enabled=damageNumbers;
                scene.health.damage.changed(stats.hp[0],first);
                stats.hp = {first, third};
                stats.mp[0] = second;
                if (third && !first) {
                    scene.pending = {};
                    scene.crouchedAt=scene.crouchReplyAt=0;
                }
            } else if (const auto found=scene.players.find(actor);found!=scene.players.end() && !found->second.IsNpc()) {
                auto& player=found->second;
                if (player.IsMonster()) {
                    player.monsterDamage.enabled=damageNumbers;
                    player.monsterDamage.changed(player.monsterVitals.current,first);
                    SetMonsterVitals(player,first,third);
                } else {
                    auto& health=player.health;
                    health.damage.enabled=damageNumbers;
                    health.damage.changed(health.current,first);
                    health.current=first;
                    health.maximum=third;
                }
            }
            break;
        case GameHealthRegistration:
            if (const auto found=scene.players.find(actor);found!=scene.players.end() && !found->second.npcType)
                found->second.health.registeredUntil=GetTickCount64()+5000;
            break;
        case GameWeight:
            stats.weight[0] = actor & 0xffff;
            stats.wear[0] = first & 0xff;
            stats.hand[0] = second & 0xff;
            break;
        case GameSubAbility:
            stats.accuracy = first & 0xff;
            stats.agility = first >> 8;
            break;
        case GameExperience:
            stats.experience = actor;
            if (second)
                stats.weaponTraining = (third & 0xff) ? second : static_cast<WORD>(stats.weaponTraining + second);
            if (third >> 8) chat.Append(100, "Bonus experience.");
            if (second) chat.Append(100, "Experience +" + std::to_string(first) + "  Weapon training +" + std::to_string(second));
            else chat.Append(100, "Experience +" + std::to_string(first));
            break;
        case GameLevelUp:
            HandleLevelUp(actor,first,third);
            break;
        }
    }

    void HandleEquipmentReply(unsigned opcode, std::uint32_t actor, WORD first, WORD second)
    {
        using mir3_cui::EquipmentRequest;
        if (opcode == 621) {
            auto slots = equipment.slots;
            std::size_t cursor = DEFBLOCKSIZE;
            for (int entries = 0; cursor < input_.size(); ++entries) {
                if (entries >= mir3_cui::EquipmentSlots) return;
                const auto slash = input_.find('/', cursor);
                if (slash == std::string::npos || slash - cursor > 2) return;
                const auto slot = Number(input_.substr(cursor, slash - cursor), mir3_cui::EquipmentSlots - 1,
                    mir3_cui::EquipmentSlots);
                if (slot >= mir3_cui::EquipmentSlots || slash + 135 >= input_.size() ||
                    input_[slash + 135] != '/' || !DecodeInventoryItem(input_.substr(slash + 1, 134), slots[slot]))
                    return;
                cursor = slash + 136;
            }
            equipment.slots = std::move(slots);
            if (!HeldItem()) {
                equipment.ReleaseHeld();
                inventory.heldShortcut = -1;
            }
            return;
        }
        if (input_.size() != DEFBLOCKSIZE) return;
        if (opcode == 642) {
            if (first >= equipment.slots.size()) return;
            auto& item = equipment.slots[first];
            if (item.key.name.empty()) return;
            item.durability = actor & 0xffff;
            item.maxDurability = second;
            const auto signedValue = static_cast<std::int64_t>(actor) - (actor >= 0x80000000u ? 0x100000000LL : 0);
            const int threshold = (item.mode == 54 || item.mode == 55) ? 50 : 500;
            if (signedValue < threshold) chat.Append(100, "Low durability: " + item.label);
            return;
        }

        const bool wearing = equipment.request == EquipmentRequest::Wear;
        if (equipment.request == EquipmentRequest::None ||
            (wearing && opcode != 615 && opcode != 616) || (!wearing && opcode != 619 && opcode != 620))
            return;
        auto& slot = equipment.slots[equipment.pendingSlot];
        if (opcode == 615) {
            const auto worn = std::find_if(equipment.slots.begin(), equipment.slots.end(),
                [&](const mir3_cui::InventoryItem& item) { return item.key == equipment.pending.key; });
            if (worn == equipment.slots.end()) {
                if (slot.key == equipment.previous || slot.key.name.empty()) slot = equipment.pending;
                else {
                    const auto free = std::find_if(equipment.slots.begin(), equipment.slots.end(),
                        [](const mir3_cui::InventoryItem& item) { return item.key.name.empty(); });
                    if (free != equipment.slots.end()) *free = equipment.pending;
                }
            }
            RemoveInventoryItem(equipment.pending.key);
            characterAppearance.Apply(actor);
            equipment.status.clear();
        } else if (opcode == 619) {
            for (auto& worn : equipment.slots) {
                if (worn.key == equipment.pending.key) worn = {};
            }
            characterAppearance.Apply(actor);
            equipment.status.clear();
        } else {
            equipment.status = wearing ? "The server declined to equip this item."
                                       : "The server declined to remove this item.";
            if (!wearing && inventory.returningEquipment == equipment.pending.key) {
                inventory.returningEquipment = {};
                inventory.returnCell = -1;
            }
        }
        equipment.request = EquipmentRequest::None;
        equipment.ReleaseHeld();
    }

    void HandleInventoryReply(unsigned opcode, std::uint32_t actor, WORD first, WORD third)
    {
        using mir3_cui::ItemRequest;
        if (opcode >= 200 && opcode <= 203) {
            ApplyBagRecords(opcode, third);
            return;
        }
        if (opcode == 601) {
            auto name = ReadGameBody();
            const auto zero = name.find('\0');
            if (zero != std::string::npos) name.resize(zero);
            if ((inventory.request == ItemRequest::Drop || inventory.request == ItemRequest::DropStack) &&
                actor == inventory.pending.key.id && name == inventory.pending.key.name) {
                inventory.request = ItemRequest::None;
                inventory.status = "The server declined the item drop.";
            }
            return;
        }
        if (input_.size() != DEFBLOCKSIZE) return;
        switch (opcode) {
        case 600:
            if ((inventory.request == ItemRequest::Drop || inventory.request == ItemRequest::DropStack) &&
                (!actor || actor == inventory.pending.key.id)) {
                if (inventory.request == ItemRequest::Drop || inventory.dropCount >= inventory.pending.count)
                    RemoveInventoryItem(inventory.pending.key);
                inventory.request = ItemRequest::None;
                inventory.status.clear();
            }
            break;
        case 635:
        case 636:
            if (inventory.request != ItemRequest::Use) break;
            inventory.request = ItemRequest::None;
            if (opcode == 636) {
                FinishShortcutUse(true);
                inventory.status = "The server declined to use " + inventory.pending.label + ".";
                inventory.shortcutStatus = inventory.status;
                inventory.notice = inventory.pending.mode == 51;
                break;
            }
            if (inventory.pendingShortcut >= 0 && inventory.pending.stackMode != 3) {
                FinishShortcutUse(false);
            } else if (inventory.pending.stackMode == 3 && inventory.pending.count > 1 &&
                (first || inventory.pendingShortcut >= 0)) {
                if (auto* item = inventory.Find(inventory.pending.key)) item->count = first;
                FinishShortcutUse(true);
            } else {
                RemoveInventoryItem(inventory.pending.key);
                FinishShortcutUse(false);
            }
            inventory.status = "Used " + inventory.pending.label + ".";
            inventory.shortcutStatus = inventory.status;
            break;
        case 653:
            inventory.gold = actor;
            if (inventory.request == ItemRequest::DropGold) {
                inventory.request = ItemRequest::None;
                inventory.status.clear();
            }
            break;
        case 1313:
            for (auto& item : inventory.items) {
                if (item.key.id == actor) ApplyBagCount(item,first);
            }
            if (!first) {
                std::vector<mir3_cui::ItemKey> emptyStacks;
                for (const auto& item : inventory.items) {
                    if (item.key.id == actor && item.Stacks()) emptyStacks.push_back(item.key);
                }
                for (const auto& key : emptyStacks) RemoveInventoryItem(key);
            }
            if (inventory.request == ItemRequest::DropStack && inventory.pending.key.id == actor) {
                inventory.request = ItemRequest::None;
                inventory.status.clear();
            }
            break;
        }
    }

    bool DecodeInventoryItem(const std::string& encoded, mir3_cui::InventoryItem& item, bool requireTerminator = false)
    {
        const auto record = DecodeGameBytes(encoded);
        if (record.size() != 100) return false;
        const auto word = [&](std::size_t offset) {
            return static_cast<unsigned char>(record[offset]) |
                (static_cast<unsigned>(static_cast<unsigned char>(record[offset + 1])) << 8);
        };
        auto name = record.substr(0, 30);
        const auto end = name.find('\0');
        if (requireTerminator && end == std::string::npos) return false;
        if (end != std::string::npos) name.resize(end);
        const auto label = ConvertText(name, 936, CP_UTF8);
        if (label.empty()) return false;
        std::copy(record.begin(),record.end(),item.serverRecord.begin());
        item.key = {word(62) | (std::uint32_t(word(64)) << 16), std::move(name)};
        item.label = label;
        item.icon = word(36);
        item.groundLooks = word(48);
        const auto dimensions = inventoryDimensions_.Get(item.icon);
        item.width = dimensions[0];
        item.height = dimensions[1];
        item.count = word(71);
        item.durability = word(66);
        item.maxDurability = word(68);
        item.mode = static_cast<unsigned char>(record[30]);
        item.subMode = static_cast<unsigned char>(record[31]);
        item.stackMode = static_cast<unsigned char>(record[61]);
        return true;
    }

    void ApplyBagRecords(unsigned opcode, WORD count)
    {
        const bool full = opcode == 201;
        if (!full) count = 1;
        if (count > 46) return;
        const std::size_t encodedRecordSize = 134;
        const auto size = input_.size() - DEFBLOCKSIZE;
        if (size != count * (encodedRecordSize + (full ? 1 : 0))) return;

        std::vector<mir3_cui::InventoryItem> records;
        std::size_t start = DEFBLOCKSIZE;
        for (WORD i = 0; i < count; ++i) {
            mir3_cui::InventoryItem item;
            if ((full && input_[start + encodedRecordSize] != '/') ||
                !DecodeInventoryItem(input_.substr(start, encodedRecordSize), item)) return;
            records.push_back(std::move(item));
            start += encodedRecordSize + (full ? 1 : 0);
        }
        if (full) {
            if (!inventory.initialized) RestoreInventoryLayout(records);
            inventory.Replace(std::move(records));
            inventory.initialized = true;
            inventory.waiting = false;
            if (!inventory.Find(inventory.selection)) inventory.selection = {};
            if (!HeldItem()) {
                equipment.ReleaseHeld();
                inventory.heldShortcut = -1;
            }
            return;
        }

        auto& item = records.front();
        if (opcode == 202 || opcode == 203) {
            for (auto& slot : equipment.slots) {
                if (!(slot.key == item.key)) continue;
                slot = {};
                if (opcode == 203) {
                    for (auto& freeSlot : equipment.slots) {
                        if (!freeSlot.key.name.empty()) continue;
                        freeSlot = item;
                        break;
                    }
                }
                break;
            }
        }
        auto* current = inventory.Find(item.key);
        // The replaced item can arrive before the wear confirmation frees its bag slot.
        const bool earlyReturn = equipment.request == mir3_cui::EquipmentRequest::Wear &&
            item.key == equipment.previous;
        const bool returnedGear = item.key == inventory.returningEquipment;
        if (opcode == 202) RemoveInventoryItem(item.key);
        else if (current) {
            item.cell = mir3_cui::InventoryPanel::FirstFit(inventory.Occupied(current), item, current->cell);
            *current = std::move(item);
        } else if (opcode == 200 && (inventory.items.size() < mir3_cui::InventoryPanel::Capacity || earlyReturn)) {
            item.cell = mir3_cui::InventoryPanel::FirstFit(inventory.Occupied(), item,
                returnedGear ? inventory.returnCell : -1);
            inventory.items.push_back(std::move(item));
        }
        if (opcode == 200 && returnedGear) {
            inventory.returningEquipment = {};
            inventory.returnCell = -1;
        }
        if (!HeldItem()) {
            equipment.ReleaseHeld();
            inventory.heldShortcut = -1;
        }
    }

    void HandleQuestReply(mir3_cui::QuestReply reply, WORD first, WORD third)
    {
        using mir3_cui::QuestReply;
        const bool text = reply == QuestReply::Text;
        const auto size = input_.size() - DEFBLOCKSIZE;
        if (size > (text ? 1366u : 10922u)) return;
        auto body = ReadGameBody();
        if (size && body.empty()) return;
        const auto zero = body.find('\0');
        if (zero != std::string::npos) body.resize(zero);
        if (text) {
            if (body.size() > 1023) return;
            auto* entry = quests.Find(third);
            auto* child = entry ? entry->FindChild(first) : nullptr;
            if (!child) return;
            const auto decoded = ConvertText(body, 936, CP_UTF8);
            if (decoded.empty() && !body.empty()) return;
            child->summary = decoded;
            return;
        }

        const bool full = reply == QuestReply::Full;
        const bool children = reply == QuestReply::Children;
        int parentId = third;
        std::vector<mir3_cui::QuestRecord> records;
        if (!mir3_cui::DecodeQuestRecords(reply, body, full || children ? first : third, parentId, records)) return;
        for (auto& record : records) {
            record.title = ConvertText(record.title, 936, CP_UTF8);
            const auto summary = ConvertText(record.summary, 936, CP_UTF8);
            if (record.title.empty() || (summary.empty() && !record.summary.empty())) return;
            record.summary = summary;
        }
        const bool childAdded = reply == QuestReply::ChildAdded;
        if (childAdded && records.empty()) return;
        if (childAdded || reply == QuestReply::Added) {
            if (quests.notices.size() == 79) quests.notices.pop_front();
            quests.notices.push_back(childAdded ? 1432 : 1433);
        }
        if (!full && !quests.initialized) {
            RequestQuests();
            return;
        }
        if (full || reply == QuestReply::Added) {
            quests.Sync(records, full);
            return;
        }
        auto* entry = quests.Find(parentId);
        if (!entry || records.empty()) return;
        if (children) {
            for (auto& record : records) record.completed = entry->record.completed;
            entry->children = std::move(records);
            entry->childrenLoaded = true;
            if (quests.parent == parentId) {
                quests.child = -1;
                quests.resetDescription = true;
            }
        } else {
            auto* duplicateParent = quests.Find(first);
            for (auto& record : records) {
                if ((duplicateParent && duplicateParent->FindChild(record.id)) || entry->children.size() >= 2048)
                    break;
                record.completed = entry->record.completed;
                entry->children.push_back(std::move(record));
                entry->childrenLoaded = true;
            }
        }
    }

    void HandleGuildReply(unsigned opcode, std::uint32_t value)
    {
        const bool overview = opcode == 753 || opcode == 773 || opcode == 774;
        const bool roster = opcode == 756 || opcode == 775 || opcode == 776;
        if (overview || roster) {
            const auto size = input_.size() - DEFBLOCKSIZE;
            if (size > (8191 * 4 + 2) / 3) return;
            auto body = ReadGameBody();
            if (size && body.empty()) return;
            const auto zero = body.find('\0');
            if (zero != std::string::npos) body.resize(zero);
            if (overview) {
                const auto previous = guild.overview.name;
                if (guild.overview.Apply(body, opcode == 753) && opcode == 753) {
                    if (guild.overview.name != previous) {
                        guild.roster = {};
                        guild.Begin(mir3_cui::GuildAction::None);
                    } else if (!guild.overview.manager) {
                        guild.Begin(mir3_cui::GuildAction::None);
                    }
                    guild.open = true;
                    guild.requested = false;
                    guild.tab = 0;
                    guild.first = 0;
                    ++guild.overviewRevision;
                }
            } else {
                guild.roster.Apply(body, guild.overview.manager);
            }
            return;
        }
        if (input_.size() != DEFBLOCKSIZE) return;
        if (opcode == 754) guild.requested = false;
        if ((opcode == 757 || opcode == 759) && GuildCanQueue()) Queue(mir3_cui::GuildMembers, 0, "");
        const auto reason = static_cast<std::int64_t>(value) - (value >= 0x80000000u ? 0x100000000LL : 0);
        const int notice = mir3_cui::GuildNoticeId(opcode, reason);
        if (notice) {
            guild.Begin(mir3_cui::GuildAction::None);
            guild.notice = notice;
            if (!guild.overview.name.empty()) guild.AppendHistory({notice,{}});
        }
    }

    void HandlePartyReply(mir3_cui::PartyReply reply, std::uint32_t value)
    {
        using mir3_cui::PartyReply;
        if (reply == PartyReply::Roster) {
            const auto size = input_.size() - DEFBLOCKSIZE;
            if (size > (8192 * 4 + 2) / 3) return;
            const auto body = ReadGameBody();
            if ((size && body.empty()) || body.size() > 8192 || body.find('\0') != std::string::npos)
                return;
            std::vector<std::string> members;
            size_t start = 0;
            for (auto end = body.find('/'); end != std::string::npos; end = body.find('/', start)) {
                if (end - start > 31) return;
                const auto name = body.substr(start, end - start);
                auto label = ConvertText(name, 936, CP_UTF8);
                if (label.empty() && !name.empty()) return;
                members.push_back(std::move(label));
                start = end + 1;
            }
            party.members = std::move(members);
            return;
        }
        if (input_.size() != DEFBLOCKSIZE) return;
        if (reply == PartyReply::Created) party.allowed = true;
        else if (reply == PartyReply::Cancelled) party.members.clear();
        else {
            const auto notice = mir3_cui::PartyFailure(reply, value);
            if (notice.id) {
                party.Begin(mir3_cui::PartyAction::None);
                party.notice = notice;
            }
        }
    }

    std::string ReadGameBody()
    {
        return DecodeGameBytes(input_.substr(DEFBLOCKSIZE));
    }

    static std::string DecodeGameBytes(std::string encoded)
    {
        const size_t encodedSize = encoded.size();
        if (!encodedSize || encodedSize % 4 == 1) return {};
        for (char byte : encoded) if (byte < 60 || byte > 123) return {};
        const unsigned padding = static_cast<unsigned>((encodedSize * 6) % 8);
        if ((encoded.back() - 60) & ((1u << padding) - 1)) return {};
        std::string body(encodedSize * 3 / 4, '\0');
        body.resize(fnDecode6BitBuf(&encoded[0], &body[0], static_cast<int>(body.size())));
        return body;
    }

    void ApplySceneKey(const std::string& challenge)
    {
        const auto decodeWord = [&](size_t start) {
            unsigned bits = 0;
            for (size_t group = 0; group < 2; ++group) {
                const size_t at = start + group * 10;
                const unsigned threshold = static_cast<unsigned char>(challenge[at]) +
                    static_cast<unsigned char>(challenge[at + 9]);
                for (size_t bit = 0; bit < 8; ++bit) {
                    if (static_cast<unsigned char>(challenge[at + bit + 1]) >= threshold)
                        bits |= 0x8000u >> (group * 8 + bit);
                }
            }
            return ((bits & 255) ^ 0x87u) << 8 | ((bits >> 8) ^ 0x87u);
        };
        const unsigned identity = decodeWord(0);
        const unsigned message = decodeWord(40);
        std::random_device random;
        const auto pick = [&](int high) { return std::uniform_int_distribution<int>(0, high)(random); };
        std::string response(40, '\0');
        for (size_t group = 0; group < 4; ++group) {
            const auto value = ((localActor_ >> (group * 8)) & 255u) ^ 0x87u;
            const int base = 10 + pick(49);
            const int threshold = 10 + pick(19);
            const auto at = group * 10;
            response[at] = static_cast<char>(base);
            response[at + 9] = static_cast<char>(threshold);
            for (size_t bit = 0; bit < 8; ++bit)
                response[at + bit + 1] = static_cast<char>(base + pick(threshold - 1) +
                    ((value & (0x80u >> bit)) ? threshold : 0));
        }
        Queue(GameDecodeKeyResponse, 0, response);
        sceneKey_ = {static_cast<unsigned char>(message), static_cast<unsigned char>(message >> 8),
            static_cast<unsigned char>(identity), static_cast<unsigned char>(identity >> 8)};
    }

    void HandleFrame()
    {
        if (IsGameConnection()) {
            HandleGameFrame();
            return;
        }
        _TDEFAULTMESSAGE header = {};
        if (input_.size() < DEFBLOCKSIZE || fnDecodeMessage(&header, &input_[0]) != sizeof header) {
            Reset("The login gateway sent an invalid reply.");
            return;
        }
        const bool characterFrame = stage == Stage::Characters;
        if (!characterFrame && header.wIdent == SM_CERTIFICATION_FAIL) {
            ShowLoginNotice(119, "The login gateway rejected authentication.");
            return;
        }
        if (!characterFrame && header.wIdent == SM_ID_NOTFOUND) {
            ShowLoginNotice(112, "The account was not found.");
            return;
        }
        if (!characterFrame && header.wIdent == SM_PASSWD_FAIL) {
            switch (header.nRecog) {
            case -7: break;
            case -6: ShowLoginNotice(112, "The account was not found."); break;
            case -5: ShowLoginNotice(117, "The account is unavailable."); break;
            case -2: ShowLoginNotice(114, "The account is already signed in."); break;
            case -1: ShowLoginNotice(113, "The password is incorrect."); break;
            default: ShowLoginNotice(116, "The login gateway rejected sign-in."); break;
            }
            return;
        }
        if (!characterFrame && header.wIdent == SM_OUTOFCONNECTION) {
            ShowLoginNotice(121, "The login gateway ended the session.");
            return;
        }
        if (!characterFrame && header.wIdent != SM_PASSOK_SELECTSERVER && header.wIdent != SM_SELECTSERVER_OK)
            return;

        const size_t size = input_.size() - DEFBLOCKSIZE;
        std::string body(size, '\0');
        if (size) {
            const int decoded = fnDecode6BitBuf(&input_[DEFBLOCKSIZE], &body[0], static_cast<int>(size));
            body.resize(decoded);
        }
        if (size % 4 == 1 || body.find('\0') != std::string::npos) {
            Reset("The login gateway sent invalid selection data.");
            return;
        }
        if (characterFrame) {
            HandleCharacterFrame(header, body);
        } else if (header.wIdent == SM_PASSOK_SELECTSERVER && stage == Stage::SigningIn) {
            if (!ReadServers(body)) {
                Reset("The login gateway sent an invalid server list.");
                return;
            }
            billing = legacy ? BillingNotice{} : ReadBilling(header);
            stage = Stage::Servers;
            status = servers.empty() ? "The gateway returned no selectable servers." : "";
        } else if (header.wIdent == SM_SELECTSERVER_OK &&
            (stage == Stage::Selecting || stage == Stage::Servers)) {
            selectionGateway = ReadGateway(body, true);
            if (!selectionGateway.port) {
                Reset("The gateway returned an invalid selection endpoint.");
                return;
            }
            status = selectionGateway.host + ':' + std::to_string(selectionGateway.port);
            selectionSession = header.nRecog;
            Close();
            stage = legacy ? Stage::Complete : Stage::Handoff;
            deadline_ = GetTickCount64() + 2000;
        }
    }

    bool ReadServers(const std::string& body)
    {
        servers = legacy ? std::vector<Server>{} : configuredServers;
        size_t start = 0;
        const char separator = legacy ? ',' : ';';
        while (start < body.size()) {
            const size_t end = body.find(separator, start);
            if (end == std::string::npos) break;
            auto name = body.substr(start, end - start);
            start = end + 1;
            WORD index = 0;
            unsigned load = 0;
            if (legacy) {
                const size_t nameEnd = body.find(',', start);
                if (nameEnd == std::string::npos) break;
                const unsigned parsed = Number(name, 65535);
                if (!parsed && name != "0") break;
                index = static_cast<WORD>(parsed);
                name = body.substr(start, nameEnd - start);
                start = nameEnd + 1;
            } else {
                const size_t open = name.rfind('(');
                if (open == std::string::npos || name.back() != ')') return false;
                const auto value = name.substr(open + 1, name.size() - open - 2);
                char* end = nullptr;
                errno = 0;
                const long parsed = std::strtol(value.c_str(), &end, 10);
                if (value.empty() || (value.front() != '-' && (value.front() < '0' || value.front() > '9')) ||
                    *end || errno == ERANGE)
                    return false;
                if (parsed >= 1 && parsed <= 3) load = static_cast<unsigned>(parsed);
                name.resize(open);
                if (!configuredServers.empty()) {
                    for (auto& server : servers) {
                        if (server.name == name) server.load = load;
                    }
                    continue;
                }
            }
            const std::string label = ConvertText(name, 936, CP_UTF8);
            if (!label.empty() && servers.size() < 256) servers.push_back({name, label, index, load});
        }
        return true;
    }
};

} // namespace mir3_client

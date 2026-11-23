// The local debug pipe queues showcase actions on the monster worker.
namespace {
using ShowcaseJson = picojson::value;
using ShowcaseObject = picojson::object;
using ShowcaseArray = picojson::array;
std::mutex showcaseMutex;
std::packaged_task<std::string()> showcaseTask;
std::vector<CMonsterObject*> showcaseMonsters;

constexpr POINT ShowcaseCells[] = {{1,0},{1,-2},{1,2}};
constexpr int ShowcaseDropRadius = 6;
constexpr char ShowcaseCharacter[] = "MirProbe01";
constexpr char ShowcaseDrop[] = "Showcase potion";

void PollMonsterShowcase()
{
    std::packaged_task<std::string()> task;
    {
        std::lock_guard<std::mutex> lock(showcaseMutex);
        task.swap(showcaseTask);
    }
    if (task.valid()) task();
}

std::string RunMonsterShowcase(const std::string& action, const std::vector<int>& ids, bool natural)
{
    auto* player = FindPlayerByName(ShowcaseCharacter);
    if (!player) throw mir3::JsonRpc::Error{-32002, "Showcase character is outside the world"};
    auto* map = player->m_pMap;
    if (action == "spawn") {
        if (!showcaseMonsters.empty())
            throw mir3::JsonRpc::Error{-32002, "Clear the previous showcase group"};
        for (int id : ids)
            if (!GetMonRaceInfo(id)) throw mir3::JsonRpc::Error{-32602, "Unknown monster index"};

        if (natural) player->m_fIsNeverDie = TRUE;
        ShowcaseArray actors;
        for (size_t i = 0; i < ids.size(); ++i) {
            const auto* race = GetMonRaceInfo(ids[i]);
            const auto cell = ShowcaseCells[i];
            auto* monster = AddCreature(map, player->m_nCurrX + cell.x, player->m_nCurrY + cell.y, race->nIndex, TRUE);
            g_pMonGenInfo[g_nNumOfMonGenInfo].xMonsterObjList.AddNewNode(monster);
            showcaseMonsters.push_back(monster);
            monster->m_fNoAttackMode = !natural;
            monster->m_dwWalkTime = GetTickCount();
            monster->m_nDirection = monster->GetNextDirection(player->m_nCurrX, player->m_nCurrY);
            actors.emplace_back(ShowcaseObject{
                {"monster", ShowcaseJson(int64_t(ids[i]))},
                {"id", ShowcaseJson(int64_t(reinterpret_cast<DWORD>(monster)))}
            });
        }
        return ShowcaseJson(ShowcaseObject{
            {"status", ShowcaseJson(action)}, {"actors", ShowcaseJson(actors)}
        }).serialize();
    }
    if (showcaseMonsters.empty()) throw mir3::JsonRpc::Error{-32002, "There is no active showcase group"};
    if (action == "attack") {
        for (auto* monster : showcaseMonsters)
            monster->AddRefMsg(RM_HIT, monster->m_nDirection, monster->m_nCurrX, monster->m_nCurrY, 0, NULL);
    } else if (action == "defeat") {
        for (auto* monster : showcaseMonsters) {
            if (monster->m_fIsDead) continue;
            monster->DamageHealth(monster->m_WAbility.HP);
            monster->Die();
        }
    } else if (action == "clear") {
        const int centerX = player->m_nCurrX, centerY = player->m_nCurrY;
        auto& monsters = g_pMonGenInfo[g_nNumOfMonGenInfo].xMonsterObjList;
        for (auto node = monsters.GetHead(); node; node = monsters.GetNext(node)) {
            auto* monster = monsters.GetData(node);
            if (!monster->m_fIsGhost) monster->MakeGhost();
        }
        showcaseMonsters.clear();
        for (int x = centerX - ShowcaseDropRadius; x <= centerX + ShowcaseDropRadius; ++x) {
            for (int y = centerY - ShowcaseDropRadius; y <= centerY + ShowcaseDropRadius; ++y) {
                auto* item = map->GetItem(x, y);
                if (!item || strcmp(item->szName, ShowcaseDrop)) continue;
                if (map->RemoveObject(x, y, OS_ITEMOBJECT, item)) {
                    player->AddRefMsg(RM_ITEMHIDE, 0, (int)item, x, y, NULL);
                    delete reinterpret_cast<_LPTGENERALITEMRCD>(item->pItem);
                    delete item;
                }
            }
        }
    }
    return ShowcaseJson(ShowcaseObject{{"status", ShowcaseJson(action)}}).serialize();
}
}

std::string MonsterShowcase(const std::string& params)
{
    ShowcaseJson request;
    if (!picojson::parse(request, params).empty() || !request.is<ShowcaseObject>() ||
        !request.get("action").is<std::string>())
        throw mir3::JsonRpc::Error{-32602, "Expected a showcase action"};
    const auto action = request.get("action").get<std::string>();
    std::vector<int> ids;
    if (action == "spawn") {
        const auto& list = request.get("ids");
        if (!list.is<ShowcaseArray>() || list.get<ShowcaseArray>().empty() ||
            list.get<ShowcaseArray>().size() > _countof(ShowcaseCells))
            throw mir3::JsonRpc::Error{-32602, "Expected one to three monster indexes"};
        for (const auto& value : list.get<ShowcaseArray>()) {
            if (!value.is<int64_t>() || value.get<int64_t>() <= 0 || value.get<int64_t>() > INT_MAX)
                throw mir3::JsonRpc::Error{-32602, "Invalid monster index"};
            ids.push_back(int(value.get<int64_t>()));
        }
    } else if (action != "attack" && action != "defeat" && action != "clear") {
        throw mir3::JsonRpc::Error{-32602, "Unknown showcase action"};
    }
    const bool natural = request.get("natural").is<bool>() && request.get("natural").get<bool>();
    std::packaged_task<std::string()> task([action, ids, natural] { return RunMonsterShowcase(action, ids, natural); });
    auto response = task.get_future();
    {
        std::lock_guard<std::mutex> lock(showcaseMutex);
        showcaseTask = std::move(task);
    }
    if (response.wait_for(std::chrono::seconds(5)) != std::future_status::ready)
        throw mir3::JsonRpc::Error{-32001, "Monster worker timed out"};
    return response.get();
}

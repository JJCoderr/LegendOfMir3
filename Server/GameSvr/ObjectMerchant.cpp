#include "stdafx.h"
#include <string>
#include <cerrno>
#include "../Def/MerchantPricing.h"

CMerchantObject::CMerchantObject() : CCharObject(NULL)
{
	m_fHideMode = FALSE;

	m_dwRunTime			= GetTickCount();
	m_dwSearchTime		= GetTickCount();

	m_dwRunNextTick		= 250;
	m_dwSearchTick		= 2000 + rand() % 2000;
}

void CMerchantObject::RunRace()
{
    {
        std::lock_guard<std::mutex> lock(m_goodsMutex);
        constexpr ULONGLONG RefillCheckMilliseconds = 30000;
        const ULONGLONG now = GetTickCount64();
        if (now-m_lastRefillCheck > RefillCheckMilliseconds) {
            m_lastRefillCheck = now;
            for (auto& good : m_goods) if (now-good.lastRefill > good.refillInterval) {
                good.lastRefill = now;
                good.stock = good.targetStock;
                if (good.returnedItems.size()>size_t(good.stock)) good.returnedItems.resize(good.stock);
                FillStock(good);
            }
        }
    }
	if ((rand() % 40) == 0)
		TurnTo(rand() % 8);
	else if ((rand() % 50) == 0)
		AddRefMsg(RM_HIT, m_nDirection, m_nCurrX, m_nCurrY, 0, NULL);
}

namespace {
bool LoadMerchantLines(const std::string& path, std::vector<std::string>& lines)
{
    FILE* input = fopen(path.c_str(), "rb");
    if (!input) return false;
    std::string source;
    char chunk[1024];
    while (const size_t count = fread(chunk, 1, sizeof(chunk), input)) source.append(chunk, count);
    fclose(input);
    size_t position = 0;
    while (position < source.size()) {
        const size_t end = source.find('\n', position);
        std::string line = source.substr(position, end == std::string::npos ? end : end-position);
        position = end == std::string::npos ? source.size() : end+1;
        const auto first = line.find_first_not_of(" \t\r");
        if (first == std::string::npos || line[first] == ';') continue;
        lines.push_back(line.substr(first, line.find_last_not_of(" \t\r") - first + 1));
    }
    return true;
}

bool FindMerchantFile(const std::vector<std::string>& files, const std::string& section, std::string& path)
{
    const std::string heading = "[" + section + "]";
    for (auto file = files.rbegin(); file != files.rend(); ++file) {
        std::vector<std::string> lines;
        if (!LoadMerchantLines(*file, lines)) continue;
        for (const auto& line : lines) if (!_stricmp(line.c_str(), heading.c_str())) {
            path = *file;
            return true;
        }
    }
    return false;
}

// Text-only script evaluation: reject unsupported executed instructions before sending anything.
bool ReadMerchantPage(const std::string& path, const std::string& section,
    int level, std::string& body, int& remaining, std::vector<std::string>& files, int depth = 0, bool textOnly = false)
{
    constexpr int MaximumCallDepth = 32;
    if (depth >= MaximumCallDepth) return false;
    std::vector<std::string> lines;
    if (!LoadMerchantLines(path, lines)) return false;
    const std::string heading = "[" + section + "]";
    size_t index = 0;
    while (index < lines.size() && _stricmp(lines[index].c_str(), heading.c_str())) ++index;
    if (index == lines.size()) return false;
    enum class Mode { None, Condition, Say, Act };
    Mode mode = textOnly ? Mode::Say : Mode::None;
    bool condition = true, hasCondition = false, active = true;
    for (++index; index < lines.size() && lines[index][0] != '['; ++index) {
        if (--remaining < 0) return false;
        const auto& line = lines[index];
        if (line == "{" || line == "}") continue;
        const bool isCall = line.size()>5 && !_strnicmp(line.c_str(), "#CALL", 5) && (line[5]==' ' || line[5]=='\t');
        const bool isInclude = line.size()>8 && !_strnicmp(line.c_str(), "#INCLUDE", 8) && (line[8]==' ' || line[8]=='\t');
        if (textOnly && line[0]=='#' && !isInclude) return false;
        if (!_stricmp(line.c_str(), "#IF")) {
            mode = Mode::Condition; condition = true; hasCondition = true; active = true;
        } else if (!_stricmp(line.c_str(), "#SAY") || !_stricmp(line.c_str(), "#ACT")) {
            mode = !_stricmp(line.c_str(), "#SAY") ? Mode::Say : Mode::Act;
            active = condition;
        } else if (!_stricmp(line.c_str(), "#ELSESAY") || !_stricmp(line.c_str(), "#ELSEACT")) {
            if (!hasCondition) return false;
            mode = !_stricmp(line.c_str(), "#ELSESAY") ? Mode::Say : Mode::Act;
            active = !condition;
        } else if (mode == Mode::Condition) {
            const auto space = line.find_first_of(" \t");
            if (space == std::string::npos || _stricmp(line.substr(0, space).c_str(), "checklevel")) return false;
            const char* number = line.c_str()+space;
            char* end;
            const long minimum = strtol(number, &end, 10);
            if (end == number || *end || minimum < 0 || minimum > UCHAR_MAX) return false;
            condition = condition && level >= minimum;
        } else if (!active) {
            continue;
        } else if (isCall || isInclude) {
            const bool include = isInclude;
            if (include && mode != Mode::Say) return false;
            if (textOnly && !include) return false;
            const auto open = line.find('['), close = line.find(']', open);
            if (open == std::string::npos || close == std::string::npos) return false;
            const auto label = line.find_first_not_of(" \t", close+1);
            if (label == std::string::npos || line[label] != '@') return false;
            std::string relative = line.substr(open+1, close-open-1);
            if (relative.empty()) return false;
            if (relative[0] == '\\' || relative[0] == '/') relative.erase(0, 1);
            const std::string target = (include ? "Envir/Market_Def/" : "Envir/QuestDiary/") + relative;
            if (!include) {
                bool known = false;
                for (const auto& file : files) if (file == target) known = true;
                if (!known) files.push_back(target);
            }
            if (!ReadMerchantPage(target, line.substr(label), level, body, remaining, files, depth+1, include)) return false;
        } else if (mode == Mode::Say && line[0] != '#') {
            body += line;
        } else if (mode == Mode::Act) {
            if (!_stricmp(line.c_str(), "break")) return true;
            const auto space = line.find_first_of(" \t");
            if (space == std::string::npos || _stricmp(line.substr(0, space).c_str(), "goto")) return false;
            const auto label = line.find_first_not_of(" \t", space);
            if (label == std::string::npos || line[label] != '@') return false;
            // Imported scripts can jump back to labels in their merchant caller.
            std::string target = path;
            const std::string targetHeading = "[" + line.substr(label) + "]";
            bool found = false;
            for (const auto& entry : lines) if (!_stricmp(entry.c_str(), targetHeading.c_str())) found = true;
            if (!found) found = FindMerchantFile(files, line.substr(label), target);
            if (!found || !ReadMerchantPage(target, line.substr(label), level, body, remaining, files, depth+1)) return false;
        } else return false;
    }
    return true;
}
}

void CMerchantObject::Say(CPlayerObject* player, const char* section)
{
    const auto& info = g_pMerchantInfo[m_nIndex];
    const std::string name = info.scriptName.empty() ? m_szName : info.scriptName;
    const std::string path = "Envir/Market_Def/" + name + "-" + info.szMapName + ".txt";
    constexpr int MaximumScriptInstructions = 4096;
    int remaining = MaximumScriptInstructions;
    std::string body;
    auto files = player->m_npcScriptFiles;
    if (files.empty()) files.push_back(path);
    std::string target = path;
    if (!FindMerchantFile(files, section, target) || !ReadMerchantPage(target, section, player->m_Ability.Level, body, remaining, files) || body.empty()) return;
    // SM_MERCHANTSAY carries a speaker prefix separated from the script by '/'.
    const std::string text = std::string(m_szName) + "/" + body;
    char encoded[8192];
    if (text.size() * 4 / 3 + 4 >= sizeof(encoded)) return;
    const int length = fnEncode6BitBufA((unsigned char*)text.data(), encoded, int(text.size()), sizeof(encoded)-1);
    encoded[length] = 0;
    _TDEFAULTMESSAGE message;
    fnMakeDefMessage(&message, SM_MERCHANTSAY, (int)this, 0, 0, 1);
    // Commit options with the successfully rendered page; failed pages keep the old context.
    std::vector<std::string> options;
    size_t position = 0;
    while ((position = body.find_first_of("<{\\", position)) != std::string::npos) {
        if (body[position]=='\\') { ++position; continue; }
        const bool optionTag = body[position]=='<';
        const auto close = body.find(optionTag ? '>' : '}', position+1);
        if (close == std::string::npos) break;
        const auto slash = body.find('/', position+1);
        if (optionTag && slash != std::string::npos && slash+1 < close && body[slash+1]=='@')
            options.push_back(body.substr(slash+1, close-slash-1));
        position = close+1;
    }
    player->m_npcOptions = options;
    player->m_npcScriptFiles = files;
    player->SendSocket(&message, encoded);
    if (!_stricmp(section, "@buy")) { player->m_sellMerchant = NULL; player->m_shopMerchant = this; ShowGoods(player); }
    if (!_stricmp(section, "@sell")) {
        player->m_shopMerchant = NULL; player->m_sellMerchant = this;
        fnMakeDefMessage(&message, SM_SENDUSERSELL, (int)this, 0, 0, 0);
        player->SendSocket(&message, NULL);
    }
}

bool CMerchantObject::LoadGoods()
{
    if (!m_goodsLoaded) {
        const auto& info = g_pMerchantInfo[m_nIndex];
        const std::string name = info.scriptName.empty() ? m_szName : info.scriptName;
        std::vector<std::string> lines;
        if (!LoadMerchantLines("Envir/Market_Def/"+name+"-"+info.szMapName+".txt", lines)) return false;
        constexpr int MinimumPricePercent = 55;
        int percent = 100;
        for (const auto& line : lines) {
            if (line[0]=='[') break;
            if (line[0]!='%' && line[0]!='+') continue;
            char* end;
            const char* number = line.c_str()+1;
            errno = 0;
            const long parsed = strtol(number, &end, 10);
            if (end==number || *end || errno==ERANGE) continue;
            if (line[0]=='+' && parsed>=0) m_purchaseTypes.push_back(int(parsed));
            if (line[0]=='%' && parsed>=MinimumPricePercent) percent = int(parsed);
        }
        m_pricePercent = percent;
        bool goods = false;
        for (const auto& line : lines) {
            if (line[0]=='[') { if (goods) break; goods = !_stricmp(line.c_str(), "[Goods]"); continue; }
            if (!goods) continue;
            char item[20], extra; int count, hours;
            if (sscanf(line.c_str(), "%19s %d %d %c", item, &count, &hours, &extra)!=3 || count<=0 || count>USHRT_MAX || hours<0) continue;
            for (int i=0; i<g_nStdItemEtc; ++i) if (!strcmp(item, g_pStdItemEtc[i].szName)) {
                const long long price = MerchantPricing::Purchase(int(g_pStdItemEtc[i].dwPrice), percent);
                constexpr ULONGLONG MillisecondsPerHour = 60ULL*60*1000;
                if (price>=0 && price<=INT_MAX)
                    {
                    m_goods.push_back({i,count,int(price),count,ULONGLONG(hours)*MillisecondsPerHour,GetTickCount64()});
                    FillStock(m_goods.back());
                }
                break;
            }
        }
        m_goodsLoaded = true;
    }
    return true;
}

void CMerchantObject::FillStock(Good& good)
{
    const auto& standard = g_pStdItemEtc[good.standard];
    if (standard.stackMode>=2) return;
    if (good.items.size()>size_t(good.stock)) good.items.resize(good.stock);
    while (good.items.size()<size_t(good.stock)) {
        _TGENERALITEMRCD record{}; record.nStdIndex=good.standard;
        standard.InitDurability(&record);
        good.items.push_back({m_nextStockId++,record});
    }
}

void CMerchantObject::Details(CPlayerObject* player, const char* name, DWORD offset)
{
    std::lock_guard<std::mutex> lock(m_goodsMutex);
    constexpr unsigned ItemsPerPage=10;
    std::string body; WORD count=0;
    for (const auto& good : m_goods) {
        auto& standard=g_pStdItemEtc[good.standard];
        if (strcmp(name,standard.szName) || standard.stackMode>=2) continue;
        if (offset>=good.items.size()) offset=good.items.size()>ItemsPerPage ? DWORD(good.items.size()-ItemsPerPage) : 0;
        for (size_t i=offset; i<good.items.size() && i<offset+ItemsPerPage; ++i) {
            const auto& offer=good.items[i];
            const int base=MerchantPricing::Base(int(standard.dwPrice),standard.wStandardDurability,
                offer.record.nDura,offer.record.nDuraMax,standard.wStdMode,standard.stackMode);
            const long long price=base<0 ? -1 : MerchantPricing::Purchase(base,m_pricePercent);
            // The native detail record carries its quote in the 16-bit maximum slot.
            if (price<0 || price>USHRT_MAX) continue;
            _TCLIENTITEMRCD legacy{}; standard.GetStandardItem(&legacy);
            legacy.nDura=WORD(offer.record.nDura); legacy.nDuraMax=WORD(price);
            auto record=Mir3Item::FromLegacy(legacy,offer.id);
            record[Mir3Item::StackMode]=standard.stackMode;
            char encoded[Mir3Item::EncodedSize+1]{};
            const int size=fnEncode6BitBufA(record.data(),encoded,int(record.size()),sizeof(encoded));
            body.append(encoded,size); body+='/'; ++count;
        }
        break;
    }
    _TDEFAULTMESSAGE message;
    fnMakeDefMessage(&message,SM_SENDDETAILGOODSLIST,(int)this,count,WORD(offset),0);
    char encoded[2048];
    const int size=fnEncode6BitBufA((unsigned char*)body.data(),encoded,int(body.size()),sizeof(encoded)-1);
    encoded[size]=0;
    player->SendSocket(&message,encoded);
}

void CMerchantObject::ShowGoods(CPlayerObject* player)
{
    std::lock_guard<std::mutex> lock(m_goodsMutex);
    if (LoadGoods()) SendGoods(player);
}

void CMerchantObject::SendGoods(CPlayerObject* player)
{
    constexpr int BulkGoods = 2, LastUniformMode = 4, BundleMode = 31, ReservedUniformMode = 42;
    std::string text;
    for (const auto& good : m_goods) {
        const auto& item = g_pStdItemEtc[good.standard];
        text += std::string(item.szName)+"/"+std::to_string(item.stackMode>=2 ? BulkGoods : int(item.wStdMode>LastUniformMode && item.wStdMode!=BundleMode && item.wStdMode!=ReservedUniformMode))+"/"+std::to_string(good.price)+"/"+
            std::to_string(good.stock)+"/"+std::to_string(item.dwLooks)+"/0/0/";
    }
    char encoded[8192];
    if (m_goods.size()>255 || text.size()*4/3+4>=sizeof(encoded)) return;
    const int length = fnEncode6BitBufA((unsigned char*)text.data(), encoded, int(text.size()), sizeof(encoded)-1);
    encoded[length] = 0;
    _TDEFAULTMESSAGE message;
    fnMakeDefMessage(&message, SM_SENDGOODSLIST, (int)this, WORD(m_goods.size()), 0, 0);
    player->SendSocket(&message, encoded);
}

void CMerchantObject::Buy(CPlayerObject* player, const char* name, int quantity, DWORD instance)
{
    std::lock_guard<std::mutex> lock(m_goodsMutex);
    enum Failure { Unavailable = 1, CannotCarry = 2, NotEnoughGold = 3 };
    int failure = Unavailable;
    CUserInfo* owner = player->m_pUserInfo;
    for (auto& good : m_goods) {
        auto& item = g_pStdItemEtc[good.standard];
        if (strcmp(name, item.szName)) continue;
        if (item.stackMode<2) {
            if (quantity) break;
            for (auto offer=good.items.begin(); offer!=good.items.end(); ++offer) {
                if (offer->id!=instance) continue;
                if (!owner->IsEnoughBag() || ULONGLONG(owner->CalcBagWeight())+item.wWeight>player->m_WAbility.MaxWeight) {
                    failure=CannotCarry; break;
                }
                const int base=MerchantPricing::Base(int(item.dwPrice),item.wStandardDurability,
                    offer->record.nDura,offer->record.nDuraMax,item.wStdMode,item.stackMode);
                const long long price=base<0 ? -1 : MerchantPricing::Purchase(base,m_pricePercent);
                if (price<=0 || price>USHRT_MAX || price>owner->GetGold()) { failure=NotEnoughGold; break; }
                auto* record=new _TGENERALITEMRCD(offer->record);
                owner->DecGold(int(price)); owner->MakeGenItem(record);
                good.items.erase(offer); --good.stock;
                player->WeightChanged(); failure=0; break;
            }
            break;
        }
        if (quantity<=0 || quantity>good.stock) break;
        if (!owner->IsEnoughBag(quantity) ||
            static_cast<unsigned long long>(owner->CalcBagWeight())+static_cast<unsigned long long>(item.wWeight)*quantity>player->m_WAbility.MaxWeight) {
            failure = CannotCarry; break;
        }
        const unsigned long long price = static_cast<unsigned long long>(good.price)*quantity;
        if (!price || price>owner->GetGold() || price>INT_MAX) { failure = NotEnoughGold; break; }
        owner->DecGold(int(price));
        for (int i=0; i<quantity; ++i) {
            auto* record = new _TGENERALITEMRCD{};
            record->nStdIndex = good.standard;
            item.InitDurability(record);
            if (good.stock-i<=int(good.returnedItems.size())) {
                *record = good.returnedItems.front();
                good.returnedItems.erase(good.returnedItems.begin());
            }
            owner->MakeGenItem(record);
        }
        good.stock -= quantity;
        player->WeightChanged();
        failure = 0;
        break;
    }
    _TDEFAULTMESSAGE message;
    fnMakeDefMessage(&message, failure ? SM_BUYITEM_FAIL : SM_BUYITEM_SUCCESS,
        failure ? failure : owner->GetGold(), 0, 0, 0);
    player->SendSocket(&message, NULL);
    if (!failure) SendGoods(player);
}

void CMerchantObject::Sale(CPlayerObject* player, _LPTGENERALITEMRCD item, bool commit, int quantity)
{
    std::lock_guard<std::mutex> lock(m_goodsMutex);
    auto* owner = player->m_pUserInfo;
    int price = -1;
    if (item && LoadGoods()) {
        const auto& standard = g_pStdItemEtc[item->nStdIndex];
        bool allowed = false;
        for (int mode : m_purchaseTypes) if (mode==standard.wStdMode) allowed = true;
        if (allowed && standard.dwPrice<=INT_MAX) {
            const int base = MerchantPricing::Base(int(standard.dwPrice),standard.wStandardDurability,
                item->nDura,item->nDuraMax,standard.wStdMode,standard.stackMode);
            if (base>=0) price = base/2 + ((base%2 && (base/2)%2) ? 1 : 0);
        }
    }
    constexpr int PowderMode=25, CandleMode=30, MinimumSaleDurability=4000;
    const bool lowQuality = item && (g_pStdItemEtc[item->nStdIndex].wStdMode==PowderMode ||
        g_pStdItemEtc[item->nStdIndex].wStdMode==CandleMode) && item->nDura<MinimumSaleDurability;
    bool sold = false;
    if (commit && !lowQuality && price>=0 && quantity==0 && ULONGLONG(owner->GetGold())+price<=BAGGOLD) {
        Good* stock = NULL;
        if (item->nDura>0) {
            for (auto& good : m_goods) if (good.standard==item->nStdIndex) stock = &good;
            if (!stock) {
                const auto buyPrice = MerchantPricing::Purchase(int(g_pStdItemEtc[item->nStdIndex].dwPrice),m_pricePercent);
                if (buyPrice>=0 && buyPrice<=INT_MAX) {
                    m_goods.push_back({item->nStdIndex,0,int(buyPrice),0,~ULONGLONG(0),GetTickCount64()});
                    stock = &m_goods.back();
                }
            }
        }
        if ((!item->nDura || (stock && stock->stock<USHRT_MAX)) && owner->IncGold(price)) {
            if (stock) {
                if (g_pStdItemEtc[item->nStdIndex].stackMode>=2) stock->returnedItems.push_back(*item);
                else stock->items.push_back({m_nextStockId++,*item});
                ++stock->stock;
            }
            owner->RemoveGenItem(item);
            player->WeightChanged();
            sold = true;
        }
    }
    _TDEFAULTMESSAGE message;
    fnMakeDefMessage(&message, commit ? (sold ? SM_USERSELLITEM_OK : SM_USERSELLITEM_FAIL) : SM_SENDBUYPRICE,
        commit ? (sold ? owner->GetGold() : 0) : (price<0 ? 0 : price), 0, 0, 0);
    player->SendSocket(&message,NULL);
}

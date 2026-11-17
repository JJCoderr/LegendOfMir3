#include "stdafx.h"
#include <string>

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
    const std::string path = std::string("Envir/Market_Def/") + m_szName + "-" +
        g_pMerchantInfo[m_nIndex].szMapName + ".txt";
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
    player->m_npcScriptFiles = files;
    player->SendSocket(&message, encoded);
}

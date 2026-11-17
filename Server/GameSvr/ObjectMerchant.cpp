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
// Text-only script evaluation: reject unsupported executed instructions before sending anything.
bool ReadMerchantPage(const std::vector<std::string>& lines, const std::string& section,
    int level, std::string& body, int& remaining, int depth = 0)
{
    constexpr int MaximumCallDepth = 32;
    if (depth >= MaximumCallDepth) return false;
    const std::string heading = "[" + section + "]";
    size_t index = 0;
    while (index < lines.size() && _stricmp(lines[index].c_str(), heading.c_str())) ++index;
    if (index == lines.size()) return false;
    enum class Mode { None, Condition, Say, Act };
    Mode mode = Mode::None;
    bool condition = true, hasCondition = false, active = true;
    for (++index; index < lines.size() && lines[index][0] != '['; ++index) {
        if (--remaining < 0) return false;
        const auto& line = lines[index];
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
        } else if (mode == Mode::Say && line[0] != '#') {
            body += line;
        } else if (mode == Mode::Act) {
            if (!_stricmp(line.c_str(), "break")) return true;
            const auto space = line.find_first_of(" \t");
            if (space == std::string::npos || _stricmp(line.substr(0, space).c_str(), "goto")) return false;
            const auto label = line.find_first_not_of(" \t", space);
            if (label == std::string::npos || line[label] != '@' ||
                !ReadMerchantPage(lines, line.substr(label), level, body, remaining, depth+1)) return false;
        } else return false;
    }
    return true;
}
}

void CMerchantObject::Say(CPlayerObject* player, const char* section)
{
    const std::string path = std::string("Envir/Market_Def/") + m_szName + "-" +
        g_pMerchantInfo[m_nIndex].szMapName + ".txt";
    FILE* input = fopen(path.c_str(), "rb");
    if (!input) return;
    std::string source;
    char chunk[1024];
    while (const size_t count = fread(chunk, 1, sizeof(chunk), input)) source.append(chunk, count);
    fclose(input);
    std::vector<std::string> lines;
    size_t position = 0;
    while (position < source.size()) {
        const size_t end = source.find('\n', position);
        std::string line = source.substr(position, end == std::string::npos ? end : end-position);
        position = end == std::string::npos ? source.size() : end+1;
        const auto first = line.find_first_not_of(" \t\r");
        if (first == std::string::npos || line[first] == ';') continue;
        lines.push_back(line.substr(first, line.find_last_not_of(" \t\r") - first + 1));
    }
    constexpr int MaximumScriptInstructions = 4096;
    int remaining = MaximumScriptInstructions;
    std::string body;
    if (!ReadMerchantPage(lines, section, player->m_Ability.Level, body, remaining) || body.empty()) return;
    // SM_MERCHANTSAY carries a speaker prefix separated from the script by '/'.
    const std::string text = std::string(m_szName) + "/" + body;
    char encoded[8192];
    if (text.size() * 4 / 3 + 4 >= sizeof(encoded)) return;
    const int length = fnEncode6BitBufA((unsigned char*)text.data(), encoded, int(text.size()), sizeof(encoded)-1);
    encoded[length] = 0;
    _TDEFAULTMESSAGE message;
    fnMakeDefMessage(&message, SM_MERCHANTSAY, (int)this, 0, 0, 1);
    player->SendSocket(&message, encoded);
}

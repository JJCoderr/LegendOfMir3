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

// Basic legacy Market_Def pages. Unsupported executable directives must not be ignored.
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
    const std::string heading = std::string("[") + section + "]";
    std::string line, body;
    bool selected = false, say = false;
    size_t position = 0;
    while (position < source.size()) {
        const size_t end = source.find('\n', position);
        line = source.substr(position, end == std::string::npos ? end : end-position);
        position = end == std::string::npos ? source.size() : end+1;
        const auto first = line.find_first_not_of(" \t\r");
        if (first == std::string::npos || line[first] == ';') continue;
        line = line.substr(first, line.find_last_not_of(" \t\r") - first + 1);
        if (line[0] == '[') {
            if (selected) break;
            selected = line == heading;
            continue;
        }
        if (!selected) continue;
        if (line == "#SAY") { say = true; continue; }
        if (!say || line[0] == '#') return;
        body += line;
    }
    if (!say || body.empty()) return;
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

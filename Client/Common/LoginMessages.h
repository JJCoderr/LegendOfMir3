#pragma once
#include <map>
#include <string>

// CmList.dat format follows Mir3 client/src/resources/messages.cpp.
class CLoginMessages {
public:
    bool Load(const char* path);
    const std::string& Get(int id) const;
private:
    std::map<std::string, std::string> m_values;
};

// Shared encrypted text container used by CmList.dat and Magic.exp.
bool LoadLegacyText(const char* path,std::string& plain);

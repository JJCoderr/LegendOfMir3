#include "LoginMessages.h"
#include <cstdint>
#include <fstream>
#include <iterator>
#include <vector>

bool CLoginMessages::Load(const char* path)
{
    std::ifstream stream(path, std::ios::binary);
    const std::vector<unsigned char> file{std::istreambuf_iterator<char>(stream), {}};
    if (file.size() < 8) return false;
    const unsigned char key[] = {0xf0, 0x39, 0xab, 0x8e};
    std::uint32_t length = 0, stored = 0, checksum = 0;
    for (int i = 0; i < 4; ++i) {
        length = (length << 8) | (file[i] ^ key[i]);
        stored |= std::uint32_t(file[i + 4]) << (8 * i);
    }
    if (length != file.size() - 8) return false;
    for (std::uint32_t i = 0; i < length; ++i) checksum += i * (std::uint32_t(file[i + 8]) + 1);
    if ((checksum ^ std::uint32_t(-1612834157)) != stored) return false;
    std::string plain(reinterpret_cast<const char*>(file.data() + 8), length);
    for (int j = 3; j >= 0; --j)
        for (std::uint32_t i = 0; i < length; ++i)
            plain[i] = char(static_cast<unsigned char>(plain[i]) ^ static_cast<unsigned char>(file[j] + i));
    std::map<std::string, std::string> values;
    std::size_t start = 0;
    while (start < plain.size()) {
        const auto end = plain.find("\r\n", start);
        if (end == std::string::npos) break;
        const std::string line = plain.substr(start, end - start);
        start = end + 2;
        if (line.empty() || line[0] != '#' || line.find(';') != std::string::npos) continue;
        auto split = line.find_first_of(" \t", 1);
        if (split == std::string::npos) split = line.size();
        auto text = line.find_first_not_of(" \t", split);
        if (text == std::string::npos) text = line.size();
        if (!values.emplace(line.substr(1, split - 1), line.substr(text)).second) return false;
    }
    if (values.empty()) return false;
    m_values.swap(values);
    return true;
}

const std::string& CLoginMessages::Get(int id) const
{
    static const std::string empty;
    const auto found = m_values.find(std::to_string(id));
    return found == m_values.end() ? empty : found->second;
}

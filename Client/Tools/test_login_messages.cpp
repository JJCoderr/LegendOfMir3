#include "../Common/LoginMessages.h"
#include <fstream>
#include <iostream>
#include <iterator>
#include <vector>

int main(int argc, char** argv) {
    if (argc != 3) return 2; // resource path, temporary corrupt-fixture path
    CLoginMessages messages;
    if (!messages.Load(argv[1])) return 3;
    for (int id : {112,113,114,116,117,119,121,126,127,128,129,130,131,216,217,218})
        if (messages.Get(id).empty()) { std::cerr << "Missing message " << id; return 4; }
    const auto original = messages.Get(112);
    std::ifstream input(argv[1], std::ios::binary);
    std::vector<char> bytes{std::istreambuf_iterator<char>(input), {}};
    bytes.back() ^= 1;
    { std::ofstream output(argv[2], std::ios::binary); output.write(bytes.data(), bytes.size()); }
    if (messages.Load(argv[2]) || messages.Get(112) != original) return 5;
    { std::ofstream output(argv[2], std::ios::binary); output.write(bytes.data(), 7); }
    if (messages.Load(argv[2]) || messages.Get(112) != original) return 6;
    std::cout << "LOGIN_MESSAGES_OK: real captions, checksum, truncation, retained state\n";
}

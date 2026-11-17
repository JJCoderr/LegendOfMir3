#include <windows.h>
#include "../Def/Protocol.h"
#include "../Def/Mir3Item.h"
#include <cstdio>

int main()
{
    _TCLIENTITEMRCD item = {};
    std::strcpy(item.tStdItem.szName, "Test sword");
    item.tStdItem.btStdMode = 5; item.tStdItem.btShape = 1;
    item.tStdItem.btWeight = 7; item.tStdItem.btAniCount = 8;
    item.tStdItem.btSource = 9; item.tStdItem.btNeedIdentify = 10;
    item.tStdItem.wLooks = 0x1234; item.tStdItem.wDuraMax = 0x2345;
    item.tStdItem.wAC = 0x0201; item.tStdItem.wMAC = 0x0403; item.tStdItem.wDC = 0x0605;
    item.tStdItem.btNeed = 1; item.tStdItem.btNeedLevel = 15;
    item.tStdItem.nPrice = 0x89abcdef;
    item.nDura = 1234; item.nDuraMax = 5678;
    auto record = Mir3Item::FromLegacy(item, 0xfedcba98);
    unsigned char expected[100] = {};
    std::memcpy(expected, "Test sword", 10);
    const unsigned char standard[] = {5,1,7,8,9,10,0x34,0x12,0x45,0x23,1,2,3,4,5,6};
    std::memcpy(expected + 30, standard, sizeof(standard));
    const unsigned char instance[] = {1,15,0xef,0xcd,0xab,0x89,0,0,0x98,0xba,0xdc,0xfe,0xd2,4,0x2e,0x16,0,1,0};
    std::memcpy(expected + 54, instance, sizeof(instance));
    if (std::memcmp(record.data(), expected, sizeof(expected))) return 1;

    Mir3Item::Identities identities;
    const char first[12] = {'A','2','6','1','1','1','7','1',0,0,0,0};
    char second[12]; std::memcpy(second, first, sizeof(second)); second[11] = 1;
    const auto firstId = identities.Get(first), secondId = identities.Get(second);
    char key[13] = {};
    if (!firstId || firstId == secondId || identities.Get(first) != firstId) return 2;
    if (!identities.Resolve(secondId, key) || std::memcmp(key, second, 12) || key[12]) return 3;
    if (identities.Resolve(0, key) || identities.Resolve(secondId + 1, key)) return 4;
    identities.Clear();
    if (identities.Resolve(firstId, key)) return 5;
    std::puts("PASS: 100-byte golden item, full-width fields, binary legacy keys, reverse lookup and reset");
    return 0;
}

#include "../../Server/Def/GateFrames.h"
#include <iostream>
#include <vector>

int main() {
    const std::vector<std::string> expected={"%776/#first!","%+-","%788/#second!"};
    const std::string wire=std::string("%776/#first!$")+char(0)+"%+-$"+char(0)+"%788/#second!$"+char(0);
    unsigned checks=0;
    auto verify=[&](const std::vector<size_t>& sizes) {
        GateFrames frames;std::vector<std::string> actual;size_t offset=0;
        for (size_t size:sizes) {
            frames.Append(wire.data()+offset,size,[&](std::string frame) {actual.push_back(frame);});
            offset+=size;
        }
        ++checks;
        return actual==expected;
    };
    for (size_t split=0;split<=wire.size();++split)
        if (!verify({split,wire.size()-split})) return 1;
    if (!verify(std::vector<size_t>(wire.size(),1))) return 2;
    GateFrames frames;std::vector<std::string> actual;
    const std::string partial="%776/#incomplete";
    frames.Append(partial.data(),partial.size(),[&](std::string s){actual.push_back(s);});
    if (!actual.empty()) return 3;
    frames.Append("!$",2,[&](std::string s){actual.push_back(s);});
    if (actual!=std::vector<std::string>{partial+"!"}) return 4;
    std::cout<<"PASS gate frames checks="<<checks<<" plus incomplete frame retention\n";
}

#pragma once

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace mir3_test {

struct Action {
    std::string step, kind, value;
};

struct Scenario {
    std::string id;
    std::vector<Action> actions;
};

inline std::vector<Scenario> ReadPlan(const std::filesystem::path& path)
{
    std::ifstream file(path);
    std::string row;
    std::getline(file,row);
    if (row.starts_with("\xef\xbb\xbf")) row.erase(0,3);
    const bool legacy=row=="step,action,value";
    if (!legacy && row!="scenario,step,action,value")
        throw std::runtime_error("expected CSV header: scenario,step,action,value");

    std::vector<Scenario> scenarios;
    if (legacy) scenarios.emplace_back();
    int line=1;
    while (std::getline(file,row)) {
        ++line;
        if (row.empty()) continue;
        const auto fail = [&](const char* message) {
            throw std::runtime_error("CSV line "+std::to_string(line)+": "+message);
        };
        std::size_t start=0;
        const auto field = [&] {
            const auto end=row.find(',',start);
            if (end==std::string::npos) fail("missing column");
            auto value=row.substr(start,end-start);
            start=end+1;
            return value;
        };
        bool newScenario=false;
        if (!legacy) {
            auto id=field();
            if (id.empty() || id.find_first_not_of("abcdefghijklmnopqrstuvwxyz0123456789-")!=id.npos)
                fail("invalid scenario id");
            newScenario=scenarios.empty() || scenarios.back().id!=id;
            if (newScenario) {
                if (std::ranges::any_of(scenarios,[&](const Scenario& other) { return other.id==id; }))
                    fail("rows for a scenario must be adjacent");
                scenarios.push_back({std::move(id),{}});
            }
        }
        auto step=field();
        auto kind=field();
        auto value=row.substr(start);
        if (value.starts_with('"')) {
            if (value.size()<2 || value.back()!='"') fail("unterminated CSV value");
            std::string unquoted;
            for (std::size_t i=1;i+1<value.size();++i) {
                if (value[i]=='"') {
                    ++i;
                    if (i+1>=value.size() || value[i]!='"') fail("expected escaped CSV quote");
                }
                unquoted+=value[i];
            }
            value=std::move(unquoted);
        } else if (value.find_first_of(",\"")!=value.npos) fail("quote values containing a comma or quote");
        if (newScenario && kind=="run" && step.empty() && value.empty()) continue;
        if (step.empty() || (kind!="click" && kind!="move" && kind!="press" && kind!="release" &&
            kind!="wheel" && kind!="key" && kind!="text")) fail("invalid step or action");
        scenarios.back().actions.push_back({std::move(step),std::move(kind),std::move(value)});
    }
    if (!file.eof()) throw std::runtime_error("could not read CSV plan");
    if (scenarios.empty()) throw std::runtime_error("CSV plan has no scenarios");
    return scenarios;
}
}

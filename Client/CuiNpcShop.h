#pragma once

#include <cstdint>
#include <cstdlib>
#include <string>
#include <vector>

namespace mir3_cui {

struct NpcShop {
    struct Good {
        std::string name, label;
        unsigned mode = 0, price = 0, stock = 0, icon = 0, requirementType = 0, requirement = 0;
    };
    struct Offer {
        std::string name, label;
        std::uint32_t id = 0;
        unsigned price = 0, icon = 0, count = 0, stackMode = 0;
    };
    struct Purchase {
        std::uint32_t actor = 0, offer = 0;
        unsigned revision = 0;
        std::uint64_t deadline = 0;
        bool crafting = false;
    };

    std::uint32_t actor = 0;
    unsigned revision = 0;
    unsigned catalogueRevision = 0;
    std::vector<Good> goods;
    std::vector<Offer> offers;
    int selected = -1, category = -1, offer = -1;
    unsigned page = 0;
    bool detailed = false, bulk = false, offersVisible = false, sourceLayout = false;
    bool storage = false, withdrawalPrompt = false, crafting = false;
    std::uint32_t withdrawing = 0;
    int notice = 0;
    std::uint64_t detailDeadline = 0;
    int lastRow = -1;
    std::uint64_t lastClick = 0;
    char quantity[12] = "1";
    std::string status;

    unsigned Quantity() const
    {
        const auto count = quantity[0] ? std::strtol(quantity, nullptr, 10) : 1;
        return count > 0 && count <= 65535 ? static_cast<unsigned>(count) : 0;
    }
};

} // namespace mir3_cui

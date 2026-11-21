#pragma once

#include <array>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <map>
#include <optional>

namespace mir3_cui {

// Read frame headers only. The CUI grid does not depend on decoded sprite pixels.
class InventoryDimensions {
public:
    void SetResourceRoot(const std::filesystem::path& root)
    {
        const auto directory = root / "Data";
        if (directory==data_) return;
        data_ = directory;
        archive_ = {};
        index_ = {};
        archiveSize_ = indexSize_ = 0;
        tableOffset_ = headerSize_ = 0;
        attempted_ = false;
        entries_.clear();
    }

    std::array<int, 2> Get(unsigned icon)
    {
        return Find(icon).value_or(std::array{1,1});
    }

    bool HasFrame(unsigned icon) { return Find(icon).has_value(); }

private:
    std::optional<std::array<int,2>> Find(unsigned icon)
    {
        if (!attempted_) Open();
        if (!headerSize_) return {};
        const auto cached = entries_.find(icon);
        if (cached != entries_.end()) return cached->second;
        return entries_.emplace(icon, Read(icon)).first->second;
    }

    std::filesystem::path data_ = "Data";
    std::ifstream archive_, index_;
    std::uint64_t archiveSize_ = 0, indexSize_ = 0;
    unsigned tableOffset_ = 0, headerSize_ = 0;
    bool attempted_ = false;
    std::map<unsigned, std::optional<std::array<int, 2>>> entries_;

    static unsigned Word(const unsigned char* bytes)
    {
        return bytes[0] | (unsigned(bytes[1]) << 8);
    }

    void Open()
    {
        if (attempted_) return;
        attempted_ = true;
        archive_.open(data_ / "Inventory.wil", std::ios::binary | std::ios::ate);
        index_.open(data_ / "Inventory.wix", std::ios::binary | std::ios::ate);
        if (!archive_ || !index_ || archive_.tellg() < 28 || index_.tellg() < 24) return;
        archiveSize_ = static_cast<std::uint64_t>(archive_.tellg());
        indexSize_ = static_cast<std::uint64_t>(index_.tellg());
        unsigned char version[2]{};
        archive_.seekg(22);
        archive_.read(reinterpret_cast<char*>(version), sizeof version);
        if (!archive_) return;
        switch (Word(version)) {
        case 17: tableOffset_ = 24; headerSize_ = 17; break;
        case 5000: tableOffset_ = 28; headerSize_ = 21; break;
        case 6000: tableOffset_ = 32; headerSize_ = 21; break;
        }
    }

    std::optional<std::array<int,2>> Read(unsigned icon)
    {
        const auto entry = std::uint64_t(tableOffset_) + std::uint64_t(icon) * 4;
        if (entry + 4 > indexSize_) return {};
        unsigned char bytes[4]{};
        index_.clear();
        index_.seekg(entry);
        if (!index_.read(reinterpret_cast<char*>(bytes), sizeof bytes)) return {};
        const auto offset = Word(bytes) | (std::uint64_t(Word(bytes + 2)) << 16);
        if (!offset || offset + headerSize_ > archiveSize_) return {};
        archive_.clear();
        archive_.seekg(offset);
        if (!archive_.read(reinterpret_cast<char*>(bytes), sizeof bytes)) return {};
        const auto width = Word(bytes), height = Word(bytes + 2);
        if (!width || !height || width > 6 * 38 || height > 100 * 38) return {};
        return std::array{int((width + 37) / 38), int((height + 37) / 38)};
    }
};

} // namespace mir3_cui

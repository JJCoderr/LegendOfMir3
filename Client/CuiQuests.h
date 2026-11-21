#pragma once

#include <algorithm>
#include <cerrno>
#include <cstdlib>
#include <deque>
#include <string>
#include <utility>
#include <vector>

namespace mir3_cui {

constexpr unsigned QuestQuery = 1046;
constexpr unsigned QuestChildrenQuery = 1048;
constexpr unsigned QuestTextQuery = 1049;
enum class QuestReply : unsigned { Full = 1301, Added = 1302, ChildAdded = 1303, Children = 1310, Text = 1311 };

struct QuestRecord {
    int id = 0;
    std::string title;
    int childCount = 0;
    bool completed = false;
    std::string summary;
};

struct QuestEntry {
    QuestRecord record;
    std::vector<QuestRecord> children;
    bool childrenLoaded = false;

    QuestRecord* FindChild(int id)
    {
        for (auto& child : children) if (child.id == id) return &child;
        return nullptr;
    }
};

inline bool DecodeQuestRecords(QuestReply reply, const std::string& body, unsigned count,
    int& parent, std::vector<QuestRecord>& result)
{
    if (count > body.size() || body.size() > 8191) return false;
    const bool full = reply == QuestReply::Full;
    const bool children = reply == QuestReply::Children;
    const bool childAdded = reply == QuestReply::ChildAdded;
    std::size_t next = 0;
    const auto take = [&](std::string& field, bool allowEnd = false) {
        const auto slash = body.find('/', next);
        if (slash == std::string::npos && !allowEnd) return false;
        field = body.substr(next, slash == std::string::npos ? slash : slash - next);
        next = slash == std::string::npos ? body.size() : slash + 1;
        return true;
    };
    const auto number = [&](int& output, bool allowEnd = false) {
        std::string field;
        if (!take(field, allowEnd) || field.empty() ||
            (field.front() != '-' && (field.front() < '0' || field.front() > '9'))) return false;
        char* end = nullptr;
        errno = 0;
        const auto value = std::strtol(field.c_str(), &end, 10);
        if (*end || errno == ERANGE) return false;
        output = static_cast<int>(value);
        return true;
    };
    if (childAdded && !number(parent)) return false;

    std::vector<QuestRecord> records;
    records.reserve(count);
    for (unsigned i = 0; i < count; ++i) {
        QuestRecord record;
        const bool last = i + 1 == count;
        if (!number(record.id) || record.id < 0 ||
            !take(record.title, last && !full && !children && !childAdded) ||
            record.title.empty() || record.title.size() > 511)
            return false;
        bool hasSummary = childAdded;
        if (full) {
            if (!number(record.childCount) || record.childCount < 0) return false;
            hasSummary = body.find('/', next) != std::string::npos;
            int completed = 0;
            if (!number(completed, last)) return false;
            record.completed = completed != 0;
        }
        if (hasSummary && (!take(record.summary, last) || record.summary.size() > 1023))
            return false;
        records.push_back(std::move(record));
    }
    if (next != body.size()) return false;
    result = std::move(records);
    return true;
}

struct QuestPanel {
    bool open = false;
    bool initialized = false;
    bool waiting = false;
    int parent = -1;
    int child = -1;
    int expanded = -1;
    bool resetScroll = false;
    bool resetDescription = false;
    std::vector<QuestEntry> entries;
    std::deque<int> notices;
    std::string error;

    QuestEntry* Find(int id)
    {
        for (auto& entry : entries) if (entry.record.id == id) return &entry;
        return nullptr;
    }

    const std::string* Description()
    {
        auto* entry = Find(parent);
        if (!entry) return nullptr;
        if (child < 0) return &entry->record.summary;
        const auto* record = entry->FindChild(child);
        return record ? &record->summary : nullptr;
    }

    void Sync(std::vector<QuestRecord>& records, bool full)
    {
        if (full) {
            entries.clear();
            initialized = !records.empty();
            waiting = false;
        }
        for (auto& record : records) {
            if (!full && Find(record.id)) break;
            if (entries.size() >= 2048) break;
            entries.push_back({std::move(record)});
        }
        std::stable_sort(entries.begin(), entries.end(), [](const QuestEntry& a, const QuestEntry& b) {
            return a.record.id < b.record.id;
        });
        parent = child = expanded = -1;
        resetScroll = resetDescription = true;
    }
};

} // namespace mir3_cui

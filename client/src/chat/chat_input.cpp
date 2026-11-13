#include "chat_input.hpp"
#include "text/native_text.hpp"
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <algorithm>
#include <vector>
namespace mir3 {
namespace {
std::string encode(const wchar_t* text, int size, UINT page) {
    const int count = WideCharToMultiByte(page, 0, text, size, nullptr, 0, nullptr, nullptr);
    std::string result(count, '\0');
    if (count) WideCharToMultiByte(page, 0, text, size, result.data(), count, nullptr, nullptr);
    return result;
}
}

std::string limit_chat_input(std::string_view utf8, int byte_limit)
{
    if (utf8.empty() || utf8.size() > 1024 * 1024) return {};
    const int length = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, utf8.data(),
        static_cast<int>(utf8.size()), nullptr, 0);
    std::wstring wide(length, L'\0');
    if (length) MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, utf8.data(),
        static_cast<int>(utf8.size()), wide.data(), length);
    auto ansi = encode(wide.data(), length, CP_ACP);
    // Legacy ANSI EDIT EM_LIMITTEXT=90. Never retain half of a DBCS glyph.
    std::size_t end = 0;
    while (end < ansi.size() && ansi[end]) {
        const std::size_t next = end + (IsDBCSLeadByteEx(CP_ACP, static_cast<BYTE>(ansi[end])) ? 2 : 1);
        if (next > ansi.size() || next > static_cast<std::size_t>(byte_limit)) break;
        end = next;
    }
    return ansi_to_utf8(std::string_view(ansi).substr(0, end));
}

namespace {
std::vector<std::string> ansi_characters(std::string_view utf8) {
    const int length = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, utf8.data(),
        static_cast<int>(utf8.size()), nullptr, 0);
    std::wstring wide(length, L'\0');
    if (length) MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, utf8.data(),
        static_cast<int>(utf8.size()), wide.data(), length);
    const auto ansi = encode(wide.data(), length, CP_ACP);
    std::vector<std::string> characters;
    for (std::size_t i = 0; i < ansi.size() && ansi[i];) {
        const auto size = IsDBCSLeadByteEx(CP_ACP, static_cast<BYTE>(ansi[i])) ? 2u : 1u;
        if (i + size > ansi.size()) break;
        characters.push_back(ansi.substr(i, size));
        i += size;
    }
    return characters;
}
}

const std::string& ChatInput::set(std::string_view text) {
    value = limit_chat_input(text, byte_limit);
    return value;
}

ChatInputEdit ChatInput::edit(std::string_view candidate, int caret) {
    if (candidate.size() > 1024 * 1024) return {value, 0};
    const auto old = ansi_characters(value), next = ansi_characters(candidate);
    std::size_t prefix = 0, suffix = 0;
    // Repeated glyphs make the edit ambiguous from text alone. The post-edit
    // caret bounds the insertion point (e.g. typing A in the middle of AAA).
    const auto prefix_limit = static_cast<std::size_t>(std::max(0,
        caret - std::max(0, static_cast<int>(next.size()) - static_cast<int>(old.size()))));
    while (prefix < old.size() && prefix < next.size() && prefix < prefix_limit && old[prefix] == next[prefix]) ++prefix;
    while (suffix < old.size() - prefix && suffix < next.size() - prefix &&
        old[old.size() - suffix - 1] == next[next.size() - suffix - 1]) ++suffix;
    std::string head, tail;
    for (std::size_t i = 0; i < prefix; ++i) head += next[i];
    for (std::size_t i = next.size() - suffix; i < next.size(); ++i) tail += next[i];
    const auto end = next.size() - suffix;
    std::size_t accepted = prefix;
    // Limit the insertion, preserving the existing suffix and any selection deletion.
    while (accepted < end && head.size() + tail.size() + next[accepted].size() <= static_cast<std::size_t>(byte_limit))
        head += next[accepted++];
    if (accepted == prefix && end > prefix) {
        // ANSI EDIT rejects the entire replacement if no inserted character fits.
        // The old selection remains available, including its original contents.
        return {value, static_cast<int>(old.size() - suffix), static_cast<int>(prefix),
            static_cast<int>(old.size() - suffix)};
    }
    const int removed = static_cast<int>(end - accepted);
    const int old_caret = std::clamp(caret, 0, static_cast<int>(next.size()));
    const int new_caret = old_caret <= static_cast<int>(accepted) ? old_caret :
        (old_caret <= static_cast<int>(end) ? static_cast<int>(accepted) : old_caret - removed);
    value = ansi_to_utf8(head + tail);
    return {value, new_caret};
}

}

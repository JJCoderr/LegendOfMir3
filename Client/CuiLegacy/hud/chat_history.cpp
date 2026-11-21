#include "text/native_text.hpp"
#include "chat_history.hpp"
#include "speech_bubble.hpp"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <algorithm>
#include <cstdio>
#include <cstring>

namespace mir3 {
namespace {
struct Surface {
    // The original PE has no DPI-awareness manifest. Isolate legacy GDI from
    // Godot's per-monitor DPI context without changing the window's awareness.
    DPI_AWARENESS_CONTEXT previous_dpi = SetThreadDpiAwarenessContext(DPI_AWARENESS_CONTEXT_UNAWARE);
    HDC dc = CreateCompatibleDC(nullptr);
    HBITMAP bitmap = nullptr;
    HFONT font = nullptr;
    HGDIOBJ old_bitmap = nullptr;
    HGDIOBJ old_font = nullptr;
    ~Surface() {
        if (old_font) SelectObject(dc, old_font);
        if (old_bitmap) SelectObject(dc, old_bitmap);
        if (font) DeleteObject(font);
        if (bitmap) DeleteObject(bitmap);
        if (dc) DeleteDC(dc);
        if (previous_dpi) SetThreadDpiAwarenessContext(previous_dpi);
    }
};
std::string encode(const wchar_t* text, int size, UINT page) {
    const int count = WideCharToMultiByte(page, 0, text, size, nullptr, 0, nullptr, nullptr);
    std::string result(count, '\0');
    if (count) WideCharToMultiByte(page, 0, text, size, result.data(), count, nullptr, nullptr);
    return result;
}
}

namespace {
std::string wrapped_ansi(HDC dc, const std::string& ansi, int width, int& count) {
    std::string buffer;
    int start = 0, high_bytes = 0; count = 1;
    for (int i = 0; i < static_cast<int>(ansi.size()); ++i) {
        SIZE extent{};
        GetTextExtentPoint32A(dc, ansi.data() + start, i - start + 1, &extent);
        const bool high = static_cast<signed char>(ansi[i]) < 0;
        if (extent.cx <= width) {
            if (high) ++high_bytes;
            buffer += ansi[i];
        } else if (high && high_bytes % 2) {
            buffer.back() = '`';
            start = i - 1;
            i -= 2;
            --high_bytes;
            ++count;
        } else {
            buffer += '`';
            start = i;
            --i;
            ++count;
        }
    }
    return buffer;
}
}

//0x44FD40: wrap with normal9pt font; even title width is measured in it.
std::vector<DescriptionRow> wrap_skill_description(const std::vector<std::string>& lines) {
    std::vector<DescriptionRow> out;
    Surface surface;if (!surface.dc) return out;
    const char face[]={char(0xcb),char(0xce),char(0xcc),char(0xe5),0};
    surface.font=CreateFontA(-MulDiv(9,GetDeviceCaps(surface.dc,LOGPIXELSY),72),0,0,0,400,0,0,0,1,0,0,0,0,face);
    if (!surface.font) return out;
    surface.old_font=SelectObject(surface.dc,surface.font);
    for (const auto& line:lines) {
        if (line.size()>1299) continue; // original fixed line buffer
        int count=0;const auto buffer=wrapped_ansi(surface.dc,line,165,count);
        std::size_t pos=0;bool failed=false;
        for (int i=0;i<std::min(count,20);++i) {
            std::string row;
            if (!failed) {
                const auto end=buffer.find('`',pos);
                const auto length=(end==buffer.npos?buffer.size():end)-pos;
                if (!length) failed=true;
                else {
                    row=buffer.substr(pos,std::min<std::size_t>(length,259));
                    pos=end==buffer.npos?buffer.size():end+1;
                    while (pos<buffer.size() && (buffer[pos]==' ' || buffer[pos]=='\t' || buffer[pos]=='\r' || buffer[pos]=='\n')) ++pos;
                    if (end==buffer.npos) failed=true;
                }
            }
            SIZE size{};GetTextExtentPoint32A(surface.dc,row.data(),int(row.size()),&size);
            out.push_back({ansi_to_utf8(row),int(size.cx),!row.empty() && row[0]=='['});
        }
    }
    return out;
}

std::vector<std::string> wrap_system_notice(std::string_view utf8)
{
    if (utf8.empty() || utf8.size()>4096) return {};
    auto ansi = utf8_to_ansi(utf8);
    if (const auto zero=ansi.find('\0'); zero!=ansi.npos) ansi.resize(zero);
    if (ansi.empty() || ansi.size()>1023) return {};

    Surface surface;
    if (!surface.dc) return {};
    const char face[]={char(0xcb),char(0xce),char(0xcc),char(0xe5),0};
    surface.font=CreateFontA(-MulDiv(9,GetDeviceCaps(surface.dc,LOGPIXELSY),72),
        0,0,0,400,0,0,0,DEFAULT_CHARSET,0,0,0,0,face);
    if (!surface.font) return {};
    surface.old_font=SelectObject(surface.dc,surface.font);
    int count=0;
    const auto buffer=wrapped_ansi(surface.dc,ansi,514,count);
    char rows[5][260]{};
    std::sscanf(buffer.c_str(),"%259[^`]%*c %259[^`]%*c %259[^`]%*c %259[^`]%*c %259[^`]%*c",
        rows[0],rows[1],rows[2],rows[3],rows[4]);
    std::vector<std::string> result;
    for (int i=0;i<std::min(count,5);++i) result.push_back(ansi_to_utf8(rows[i]));
    return result;
}

// 0x4366D0 inserts into separately wrapped compact and expanded histories.
void ChatHistory::post(std::string_view utf8, std::uint32_t foreground, std::uint32_t background)
{
    if (utf8.empty() || utf8.size() > 4096) return;
    const int length = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, utf8.data(),
        static_cast<int>(utf8.size()), nullptr, 0);
    std::wstring wide(length, L'\0');
    if (length) MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, utf8.data(),
        static_cast<int>(utf8.size()), wide.data(), length);
    auto ansi = encode(wide.data(), length, CP_ACP);
    ansi.resize(ansi.find('\0') == std::string::npos ? ansi.size() : ansi.find('\0'));
    if (ansi.empty() || ansi.size() > 200) return; // 0x4366D0: strlen, not UTF-8 length.
    Surface surface;
    if (!surface.dc) return;
    const char face[] = {char(0xcb), char(0xce), char(0xcc), char(0xe5), 0};
    surface.font = CreateFontA(-MulDiv(9, GetDeviceCaps(surface.dc, LOGPIXELSY), 72),
        0, 0, 0, 400, 0, 0, 0, DEFAULT_CHARSET, 0, 0, 0, 0, face);
    if (!surface.font) return;
    surface.old_font = SelectObject(surface.dc, surface.font);
    for (int mode = 0; mode < 2; ++mode) {
        // 0x48C03A deliberately measures successive ANSI byte prefixes. Preserve
        // its signed-byte parity/backtracking, rather than Unicode word wrapping.
        int count=0;
        const auto buffer=wrapped_ansi(surface.dc,ansi,mode?429:341,count);
        // Original sscanf uses backticks and consumes whitespace after separators.
        // Existing backticks in a message therefore have the same legacy behavior.
        char rows[5][260]{};
        std::sscanf(buffer.c_str(), "%259[^`]%*c %259[^`]%*c %259[^`]%*c %259[^`]%*c %259[^`]%*c",
            rows[0], rows[1], rows[2], rows[3], rows[4]);
        for (int i = 0; i < std::min(count, mode ? 3 : 5); ++i) {
            // 0x43682E / 0x436902: expanded index checked in BOTH insertion loops.
            if (static_cast<int>(logs[1].size()) - scrolls[1].position() == 14)
                scrolls[1].set_position(scrolls[1].position() + 1);
            if (!mode && static_cast<int>(logs[0].size()) - scrolls[0].position() == 5)
                scrolls[0].set_position(scrolls[0].position() + 1);
            logs[mode].push_back({ansi_to_utf8(rows[i]), foreground, background});
        }
    }
    // 0x436991: trim only after both insertion loops have finished.
    for (int mode = 0; mode < 2; ++mode) {
        while (logs[mode].size() >= 80) {
            logs[mode].pop_front();
            if (scrolls[mode].position() > 0)
                scrolls[mode].set_position(scrolls[mode].position() - 1);
        }
    }
}

std::vector<SpeechLine> wrap_speech(std::string_view utf8)
{
    if (utf8.empty() || utf8.size()>4096) return {};
    const int length=MultiByteToWideChar(CP_UTF8,MB_ERR_INVALID_CHARS,utf8.data(),static_cast<int>(utf8.size()),nullptr,0);
    if (!length) return {};
    std::wstring wide(length,L'\0');
    MultiByteToWideChar(CP_UTF8,MB_ERR_INVALID_CHARS,utf8.data(),static_cast<int>(utf8.size()),wide.data(),length);
    auto ansi=encode(wide.data(),length,CP_ACP);
    const auto nul=ansi.find(char(0));if (nul!=ansi.npos) ansi.resize(nul);
    if (ansi.empty() || ansi.size()>259) return {}; // original actor buffer260
    Surface surface;
    if (!surface.dc) return {};
    const char face[]={char(0xcb),char(0xce),char(0xcc),char(0xe5),0};
    surface.font=CreateFontA(-MulDiv(9,GetDeviceCaps(surface.dc,LOGPIXELSY),72),0,0,0,400,0,0,0,DEFAULT_CHARSET,0,0,0,0,face);
    if (!surface.font) return {};
    surface.old_font=SelectObject(surface.dc,surface.font);
    int count=0;
    const auto buffer=wrapped_ansi(surface.dc,ansi,144,count);
    if (count>8) return {}; // reject legacy fixed-buffer overflow
    char rows[8][260]{};
    std::sscanf(buffer.c_str(),"%259[^`]%*c %259[^`]%*c %259[^`]%*c %259[^`]%*c %259[^`]%*c %259[^`]%*c %259[^`]%*c %259[^`]%*c",rows[0],rows[1],rows[2],rows[3],rows[4],rows[5],rows[6],rows[7]);
    std::vector<SpeechLine> result;
    for (int i=0;i<count;++i) {
        SIZE size{};GetTextExtentPoint32A(surface.dc,rows[i],static_cast<int>(std::strlen(rows[i])),&size);
        result.push_back({ansi_to_utf8(rows[i]),size.cx});
    }
    return result;
}

}

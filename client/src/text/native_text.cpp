#include "native_text.hpp"
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

std::string utf8_to_ansi(std::string_view text) {
    if (text.size()>1024*1024) return {};
    const int count=MultiByteToWideChar(CP_UTF8,MB_ERR_INVALID_CHARS,text.data(),int(text.size()),nullptr,0);
    std::wstring wide(count,L'\0');
    if (count) MultiByteToWideChar(CP_UTF8,MB_ERR_INVALID_CHARS,text.data(),int(text.size()),wide.data(),count);
    return encode(wide.data(),count,CP_ACP);
}

std::string multiline_to_ansi(std::string_view proposed_utf8) {
    const auto proposed=utf8_to_ansi(proposed_utf8);
    std::string next;
    for (std::size_t i=0;i<proposed.size() && proposed[i];++i) {
        if (proposed[i]=='\n' && (i==0 || proposed[i-1]!='\r')) next+='\r';
        next+=proposed[i];
    }
    return next;
}

std::string edit_multiline_ansi(std::string_view previous_ansi,std::string_view proposed_utf8) {
    previous_ansi=previous_ansi.substr(0,previous_ansi.find(char(0)));
    const auto next=multiline_to_ansi(proposed_utf8);
    const auto boundary=[](std::string_view value,std::size_t index) {
        std::size_t i=0;
        while (i<index) i+=IsDBCSLeadByteEx(CP_ACP,static_cast<BYTE>(value[i])) && i+1<value.size()?2:1;
        return i==index;
    };
    std::size_t prefix=0,suffix=0;
    while (prefix<previous_ansi.size() && prefix<next.size() && previous_ansi[prefix]==next[prefix]) ++prefix;
    while (prefix && (!boundary(previous_ansi,prefix) || !boundary(next,prefix))) --prefix;
    while (suffix<previous_ansi.size()-prefix && suffix<next.size()-prefix && previous_ansi[previous_ansi.size()-1-suffix]==next[next.size()-1-suffix]) ++suffix;
    while (suffix && (!boundary(previous_ansi,previous_ansi.size()-suffix) || !boundary(next,next.size()-suffix))) --suffix;
    const auto retained=prefix+suffix;
    const auto available=retained<4000?4000-retained:0;
    auto inserted=std::min(next.size()-retained,available);
    while (inserted && !boundary(next,prefix+inserted)) --inserted;
    return next.substr(0,prefix)+next.substr(prefix,inserted)+next.substr(next.size()-suffix);
}

std::string ansi_to_utf8(std::string_view text)
{
    const int count = MultiByteToWideChar(CP_ACP, 0, text.data(), static_cast<int>(text.size()), nullptr, 0);
    std::wstring wide(count, L'\0');
    if (count) MultiByteToWideChar(CP_ACP, 0, text.data(), static_cast<int>(text.size()), wide.data(), count);
    return encode(wide.data(), count, CP_UTF8);
}

std::vector<std::string> wrap_quest_detail(std::string_view utf8) {
    if (utf8.empty()) return {std::string()};
    const auto ansi=utf8_to_ansi(utf8);
    if (ansi.empty()) return {};

    Surface surface;
    if (!surface.dc) return {};
    const char face[]={char(0xcb),char(0xce),char(0xcc),char(0xe5),0};
    surface.font=CreateFontA(-MulDiv(9,GetDeviceCaps(surface.dc,LOGPIXELSY),72),
        0,0,0,400,0,0,0,DEFAULT_CHARSET,0,0,0,0,face);
    if (!surface.font) return {};
    surface.old_font=SelectObject(surface.dc,surface.font);
    std::vector<std::string> rows;
    std::size_t start=0,end=0;
    while (end<ansi.size()) {
        end+=end+1<ansi.size() && IsDBCSLeadByteEx(CP_ACP,static_cast<BYTE>(ansi[end]))?2:1;
        SIZE extent{};
        GetTextExtentPoint32A(surface.dc,ansi.data()+start,int(end-start),&extent);
        // 0x46C3C7 keeps the character that meets or crosses 170 pixels.
        if (extent.cx>=170 || end==ansi.size()) {
            rows.push_back(ansi_to_utf8(std::string_view(ansi).substr(start,end-start)));
            start=end;
        }
    }
    return rows;
}

NativeText raster_text(std::string_view utf8, int width, int height, bool centered,
    TextFont font, std::string_view localized_face, bool inverted_vertical)
{
    NativeText result;
    if (width <= 0 || height <= 0 || width > 1024 || height > 1024 || utf8.size() > 4096) return result;
    Surface surface;
    if (!surface.dc) return result;
    result.dpi = GetDeviceCaps(surface.dc, LOGPIXELSY);
    result.code_page = GetACP();
    // Preserve bytes passed to CreateFontA, including the special level face.
    const char normal_face[] = {char(0xcb), char(0xce), char(0xcc), char(0xe5), 0};
    const char level_face[] = {char(0xb1), char(0xbc), char(0xb8), char(0xb2), 0};
    const std::string caption_face(localized_face);
    const bool use_caption_face = font == TextFont::gold || font == TextFont::localized_normal;
    if (font == TextFont::gold && caption_face.empty()) return result;
    const int points = (font == TextFont::gold || font == TextFont::skill_title) ? 10 : ((font == TextFont::level || font == TextFont::skill_level) ? 8 : 9);
    surface.font = CreateFontA(-MulDiv(points, result.dpi, 72), 0, 0, 0,
        (font == TextFont::normal || font == TextFont::dialog || font == TextFont::skill_level || font == TextFont::npc_link || font == TextFont::localized_normal) ? 400 : 700, 0, font == TextFont::npc_link, 0, DEFAULT_CHARSET, 0, 0, 0, 0,
        use_caption_face ? caption_face.c_str() :
        (font == TextFont::level || font == TextFont::skill_level || font == TextFont::skill_title || font == TextFont::character_name ? level_face : normal_face));
    if (!surface.font) return result;
    surface.old_font = SelectObject(surface.dc, surface.font);
    wchar_t face[LF_FACESIZE]{};
    GetTextFaceW(surface.dc, LF_FACESIZE, face);
    result.face = encode(face, static_cast<int>(wcslen(face)), CP_UTF8);

    BITMAPINFO info{};
    info.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    info.bmiHeader.biWidth = width;
    info.bmiHeader.biHeight = -height;
    info.bmiHeader.biPlanes = 1;
    info.bmiHeader.biBitCount = 32;
    info.bmiHeader.biCompression = BI_RGB;
    void* bits = nullptr;
    surface.bitmap = CreateDIBSection(surface.dc, &info, DIB_RGB_COLORS, &bits, nullptr, 0);
    if (!surface.bitmap || !bits) return result;
    surface.old_bitmap = SelectObject(surface.dc, surface.bitmap);
    PatBlt(surface.dc, 0, 0, width, height, BLACKNESS);
    SetBkMode(surface.dc, TRANSPARENT);
    SetTextColor(surface.dc, RGB(255, 255, 255));
    const int length = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, utf8.data(),
        static_cast<int>(utf8.size()), nullptr, 0);
    std::wstring wide(length, L'\0');
    if (length) MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, utf8.data(),
        static_cast<int>(utf8.size()), wide.data(), length);
    auto ansi = encode(wide.data(), length, CP_ACP);
    SIZE extent{};
    GetTextExtentPoint32A(surface.dc, ansi.data(), static_cast<int>(ansi.size()), &extent);
    result.text_width = extent.cx;
    result.text_height = extent.cy;
    RECT rect{0, 0, width, height};
    if (inverted_vertical) { rect.top=height;rect.bottom=0; }
    if (font==TextFont::dialog && extent.cx>=width) {
        RECT measured=rect;
        const int block=DrawTextA(surface.dc,ansi.data(),int(ansi.size()),&measured,1040);
        rect.top=height/2-block/2;rect.bottom=rect.top+block;
        DrawTextA(surface.dc,ansi.data(),int(ansi.size()),&rect,16);
    }
    else if (centered) DrawTextA(surface.dc, ansi.data(), static_cast<int>(ansi.size()), &rect, 37);
    else TextOutA(surface.dc, 0, 0, ansi.data(), static_cast<int>(ansi.size()));
    GdiFlush();
    result.rgba.resize(static_cast<std::size_t>(width) * height * 4);
    const auto* source = static_cast<const std::uint8_t*>(bits);
    for (std::size_t i = 0; i < result.rgba.size(); i += 4) {
        result.rgba[i] = result.rgba[i + 1] = result.rgba[i + 2] = 255;
        result.rgba[i + 3] = std::max({source[i], source[i + 1], source[i + 2]});
    }
    return result;
}
}

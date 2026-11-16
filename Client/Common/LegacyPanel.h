#pragma once

// Same premultiplied rectangle operation as Godot selection_pixels.hpp.
inline void DrawLegacyPanel(RECT box, COLORREF fill, BYTE alpha, COLORREF border, bool outlined = true)
{
    DDSURFACEDESC2 surface = {}; surface.dwSize = sizeof(surface);
    LPDIRECTDRAWSURFACE7 buffer = g_xMainWnd.GetBackBuffer();
    if (FAILED(buffer->Lock(NULL, &surface, DDLOCK_WAIT, NULL))) return;
    for (int y = max(0L, box.top); y < min((LONG)surface.dwHeight, box.bottom); ++y) {
        WORD* row = (WORD*)((BYTE*)surface.lpSurface + y * surface.lPitch);
        for (int x = max(0L, box.left); x < min((LONG)surface.dwWidth, box.right); ++x) {
            const bool edge = outlined && (x == box.left || x == box.right-1 || y == box.top || y == box.bottom-1);
            const WORD old = row[x];
            const int r = edge ? GetRValue(border) : min(255, GetRValue(fill) + (((old >> 11) * 255 / 31) * (255-alpha) + 127) / 255);
            const int g = edge ? GetGValue(border) : min(255, GetGValue(fill) + ((((old >> 5) & 63) * 255 / 63) * (255-alpha) + 127) / 255);
            const int b = edge ? GetBValue(border) : min(255, GetBValue(fill) + (((old & 31) * 255 / 31) * (255-alpha) + 127) / 255);
            row[x] = WORD(((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3));
        }
    }
    buffer->Unlock(NULL);
}

// RGB565 adaptation of Mir3's Godot selection_pixels.hpp.
enum class SelectionBlend { Normal, Inactive, Shadow, Additive, Alpha, ItemHighlight };

// MIR3 48FA0F: dye C2/C3 first, then floor(c + 1.5*c*c/max).
inline WORD HighlightItemPixel(WORD pixel,WORD dye)
{
    const auto channel=[](int source,int color,int maximum) {
        const int tinted=source*color/maximum;
        return min(maximum,tinted+(3*tinted*tinted)/(2*maximum));
    };
    return WORD((channel(pixel>>11,dye>>11,31)<<11) |
        (channel((pixel>>5)&63,(dye>>5)&63,63)<<5) | channel(pixel&31,dye&31,31));
}

inline RECT DrawSelectionFrame(CWHWilImageData& image, int frame, int x, int y,
    SelectionBlend blend, bool useOffset = true, BYTE sourceAlpha = 0, const RECT* clip = nullptr, BYTE tint = 255, WORD firstDye = 0xffff, WORD secondDye = 0xffff)
{
    if (!image.NewSetIndex(frame)) return {};
    const auto& info = *image.m_lpstNewCurrWilImageInfo;
    if (useOffset) { x += info.shPX; y += info.shPY; }
    const RECT bounds = {x, y, x + info.shWidth, y + info.shHeight};
    DDSURFACEDESC2 surface = {}; surface.dwSize = sizeof(surface);
    auto* buffer = g_xMainWnd.GetBackBuffer();
    if (FAILED(buffer->Lock(NULL, &surface, DDLOCK_WAIT, NULL))) return bounds;
    const auto* next = reinterpret_cast<const WORD*>(image.m_pbCurrImage);
    for (int sy = 0; sy < info.shHeight; ++sy) {
        const WORD* run = next + 1;
        const WORD* end = run + *next;
        next = end;
        const int dy = y + sy;
        if (dy < 0 || dy >= int(surface.dwHeight) || (clip && (dy<clip->top || dy>=clip->bottom))) continue;
        auto* row = reinterpret_cast<WORD*>(static_cast<BYTE*>(surface.lpSurface) + dy * surface.lPitch);
        int dx = x;
        while (run + 1 < end) {
            const WORD tag = *run++, count = *run++;
            if (tag == 0xc0) { dx += count; continue; }
            if (tag < 0xc1 || tag > 0xc3 || run + count > end) break;
            for (int i = 0; i < count; ++i, ++dx) {
                if (dx < 0 || dx >= int(surface.dwWidth) || (clip && (dx<clip->left || dx>=clip->right))) continue;
                const WORD pixel = run[i];
                int r = pixel >> 11, g = (pixel >> 5) & 63, b = pixel & 31;
                if (blend == SelectionBlend::ItemHighlight) {
                    const WORD highlighted=HighlightItemPixel(pixel,tag==0xc2?firstDye:tag==0xc3?secondDye:0xffff);
                    r=highlighted>>11;g=(highlighted>>5)&63;b=highlighted&31;
                } else if (blend == SelectionBlend::Inactive) {
                    const int gray = (r + (g >> 1) + b) / 3;
                    r = b = min(31, gray + gray * 3 / 5);
                    g = r * 2;
                } else if (blend == SelectionBlend::Shadow) {
                    const WORD dst = row[dx];
                    r = (r + (dst >> 11)) / 2;
                    g = (g + ((dst >> 5) & 63)) / 2;
                    b = (b + (dst & 31)) / 2;
                } else if (blend == SelectionBlend::Alpha) {
                    const WORD dst = row[dx];
                    r = (r*tint*sourceAlpha+(dst >> 11)*(255-sourceAlpha)*255+32512)/65025;
                    g = (g*tint*sourceAlpha+((dst >> 5)&63)*(255-sourceAlpha)*255+32512)/65025;
                    b = (b*tint*sourceAlpha+(dst&31)*(255-sourceAlpha)*255+32512)/65025;
                } else if (blend == SelectionBlend::Additive) {
                    const WORD dst = row[dx];
                    r = min(31, r + ((dst >> 11)*(255-sourceAlpha)+127)/255);
                    g = min(63, g + (((dst >> 5) & 63)*(255-sourceAlpha)+127)/255);
                    b = min(31, b + ((dst & 31)*(255-sourceAlpha)+127)/255);
                }
                row[dx] = WORD((r << 11) | (g << 5) | b);
            }
            run += count;
        }
    }
    buffer->Unlock(NULL);
    return bounds;
}

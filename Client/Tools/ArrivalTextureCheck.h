#pragma once

// Pixel-level check of the actual DirectDraw textures used by the arrival effect.
inline int CheckArrivalTextures()
{
    FILE* report = fopen("arrival-texture-check.txt", "wb");
    if (!report) return 1;
    CWHWilImageData image;
    char path[] = "./Data/Magic.wil";
    if (!image.NewLoad(path, TRUE, TRUE)) { fclose(report); return 1; }
    const auto* effect = g_xSpriteInfo.GetMagicInfo(_SKILL_SPACEMOVE2);
    int checked = 0, failures = 0;
    for (DWORD frame = effect->dwFstFrm; frame < effect->dwEndFrm; ++frame) {
        if (!image.NewSetIndex(frame)) continue;
        const auto info = *image.m_lpstNewCurrWilImageInfo;
        if (!info.shWidth || !info.shHeight) continue;
        std::vector<WORD> expected(size_t(info.shWidth) * info.shHeight, 0);
        const WORD* next = reinterpret_cast<const WORD*>(image.m_pbCurrImage);
        for (int y = 0; y < info.shHeight; ++y) {
            const WORD* run = next + 1;
            const WORD* end = run + *next;
            next = end;
            int x = 0;
            while (run + 1 < end) {
                const WORD tag = *run++, count = *run++;
                if (tag == 0xc0) { x += count; continue; }
                if (tag < 0xc1 || tag > 0xc3 || run + count > end || x + count > info.shWidth) {
                    fclose(report); return 1;
                }
                for (int i = 0; i < count; ++i) expected[y * info.shWidth + x++] = *run++;
            }
        }
        D3DWILTextr_CreateEmptyTexture(path, frame, info.shWidth, info.shHeight,
            reinterpret_cast<WORD*>(image.m_pbCurrImage), D3DTEXTR_TRANSPARENTBLACK);
        auto* surface = D3DWILTextr_RestoreEx(path, frame, g_xMainWnd.Get3DDevice());
        if (!surface) { fprintf(report, "frame=%lu upload_failed\n", frame); ++failures; continue; }
        DDSURFACEDESC2 desc{}; desc.dwSize = sizeof(desc);
        if (FAILED(surface->Lock(nullptr, &desc, DDLOCK_WAIT | DDLOCK_READONLY, nullptr))) {
            fclose(report); return 1;
        }
        fprintf(report, "bpp=%lu masks=%08lx/%08lx/%08lx pitch=%ld\n", desc.ddpfPixelFormat.dwRGBBitCount, desc.ddpfPixelFormat.dwRBitMask, desc.ddpfPixelFormat.dwGBitMask, desc.ddpfPixelFormat.dwBBitMask, desc.lPitch);
        const bool format = desc.ddpfPixelFormat.dwRGBBitCount == 16 &&
            desc.ddpfPixelFormat.dwRBitMask == 0xf800 && desc.ddpfPixelFormat.dwGBitMask == 0x07e0 &&
            desc.ddpfPixelFormat.dwBBitMask == 0x001f;
        int mismatch = 0, transparent = 0;
        const bool dimensions = desc.dwWidth == info.shWidth && desc.dwHeight == info.shHeight;
        if (format && dimensions) {
            for (int y = 0; y < info.shHeight; ++y) {
                const WORD* row = reinterpret_cast<const WORD*>(static_cast<const BYTE*>(desc.lpSurface) + y * desc.lPitch);
                for (int x = 0; x < info.shWidth; ++x) {
                    const WORD pixel = expected[y * info.shWidth + x];
                    transparent += pixel == 0;
                    mismatch += row[x] != pixel;
                }
            }
        }
        surface->Unlock(nullptr);
        fprintf(report, "frame=%lu rgb565=%d dimensions=%d transparent=%d mismatch=%d\n",
            frame, format, dimensions, transparent, mismatch);
        failures += !format || !dimensions || mismatch != 0;
        ++checked;
        D3DWILTextr_DestroyTexture(path, frame);
    }
    fprintf(report, "checked=%d failures=%d\n", checked, failures);
    fclose(report);
    return checked && !failures ? 0 : 1;
}

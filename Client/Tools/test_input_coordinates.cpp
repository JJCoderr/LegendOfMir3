#include "../WindHorn/InputCoordinates.h"
#include <cstdio>

int main()
{
    SetProcessDPIAware();
    HWND window = CreateWindowW(L"STATIC", L"Coordinate test", WS_POPUP,
        0, 0, 1200, 900, nullptr, nullptr, GetModuleHandleW(nullptr), nullptr);
    if (!window) return 1;
    int failures = 0;
    const auto check = [&](POINT physical, POINT expected) {
        const POINT actual = mir3::ClientToCanvas(window, physical, 800, 600);
        if (actual.x != expected.x || actual.y != expected.y) ++failures;
    };
    // A 150% client: center, east tile, and a captured pointer outside the window.
    check({600, 450}, {400, 300});
    check({672, 360}, {448, 240});
    check({-72, -48}, {-48, -32});
    SetWindowPos(window, nullptr, 0, 0, 1600, 900, SWP_NOMOVE | SWP_NOZORDER);
    check({896, 360}, {448, 240}); // independent horizontal/vertical scaling
    SetWindowPos(window, nullptr, 0, 0, 800, 600, SWP_NOMOVE | SWP_NOZORDER);
    check({448, 240}, {448, 240});
    DestroyWindow(window);
    std::printf("input coordinates: failures=%d\n", failures);
    return failures ? 1 : 0;
}

#pragma once

#include <windows.h>

namespace mir3 {

inline POINT ClientToCanvas(HWND window, POINT point, int width, int height)
{
    RECT client{};
    GetClientRect(window, &client);
    if (client.right && client.bottom) {
        point.x = MulDiv(point.x, width, client.right);
        point.y = MulDiv(point.y, height, client.bottom);
    }
    return point;
}

}

#pragma once
namespace GoldDrop {
    constexpr int Dialog=102,Prompt=1415,Request=1016,Icon=124;
    inline RECT InventoryBounds(const RECT& window) { return {window.left+15,window.top+390,window.left+57,window.top+417}; }
}

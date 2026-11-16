#pragma once
#include <windows.h>
#include <vector>

// Disk records use byte packing. V1.45 stores separate object file indices.
#pragma pack(push, 1)
struct Mir145Cell {
    BYTE flag, animation1, animation2, file2, file1;
    WORD object1, object2;
    BYTE doorIndex, doorOffset, reserved;
    WORD lightEvent;
};
#pragma pack(pop)
static_assert(sizeof(Mir145Cell) == 14, "Unexpected map cell layout");

inline bool ValidateMirMapSize(HANDLE file, int width, int height)
{
    if (width <= 0 || height <= 0 || (width & 1) || (height & 1)) return false;
    ULONGLONG count = static_cast<ULONGLONG>(width) * height;
    ULONGLONG tilesEnd = 28 + count / 4 * 3;
    DWORD size = GetFileSize(file, NULL);
    if (size == INVALID_FILE_SIZE || size < tilesEnd) return false;
    ULONGLONG remaining = size - tilesEnd;
    return remaining == count * 10 || remaining == count * 12 || remaining == count * 14;
}

inline bool ReadMir145Cells(HANDLE file, int count, std::vector<Mir145Cell>& cells)
{
    if (count <= 0) return false;
    DWORD position = SetFilePointer(file, 0, NULL, FILE_CURRENT);
    DWORD size = GetFileSize(file, NULL);
    ULONGLONG bytes = static_cast<ULONGLONG>(count) * sizeof(Mir145Cell);
    if (position == INVALID_SET_FILE_POINTER || size == INVALID_FILE_SIZE ||
        position > size) return false;
    DWORD remaining = size - position;
    if (remaining == static_cast<DWORD>(count) * 10 || remaining == static_cast<DWORD>(count) * 12) {
        DWORD stride = remaining / count;
        std::vector<BYTE> legacy(remaining);
        DWORD read = 0;
        if (!ReadFile(file, legacy.data(), remaining, &read, NULL) || read != remaining) return false;
        cells.resize(count);
        for (int i = 0; i < count; ++i) {
            const BYTE* source = &legacy[i * stride];
            Mir145Cell& cell = cells[i];
            ZeroMemory(&cell, sizeof(cell));
            cell.flag = source[0]; cell.animation1 = source[1]; cell.animation2 = source[2];
            cell.file1 = source[3] >> 4; cell.file2 = source[3] & 15;
            memcpy(&cell.object1, source + 4, 4);
            if (stride == 12) { cell.doorIndex = source[8]; cell.doorOffset = source[9]; }
            memcpy(&cell.lightEvent, source + stride - 2, 2);
        }
        return true;
    }
    if (bytes != remaining) return false;
    cells.resize(count);
    DWORD read = 0;
    return ReadFile(file, cells.data(), static_cast<DWORD>(bytes), &read, NULL)
        && read == bytes;
}

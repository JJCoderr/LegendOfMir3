#include "stdafx.h"
#include <string>
#include <vector>
#include <set>
#include <errno.h>

// Legacy entry points are retained for the existing configuration dialogs.
// Configuration is stored exclusively in server.ini in the working directory.
#ifdef MIR3_EISERVER
static const TCHAR* ConfigFile = _T(".\\EIServer.ini");
#else
static const TCHAR* ConfigFile = _T(".\\server.ini");
#endif

static std::basic_string<TCHAR> Section(LPCTSTR key)
{
    const TCHAR* prefix = _T("Software\\LegendOfMir\\");
    return _tcsnicmp(key, prefix, _tcslen(prefix)) == 0 ? key + _tcslen(prefix) : key;
}

static bool IsAddress(LPCTSTR name)
{
    return _tcscmp(name, _T("RemoteIP")) == 0 || _tcscmp(name, _T("DBServerIP")) == 0 ||
        _tcscmp(name, _T("PrivateIP")) == 0 || _tcscmp(name, _T("PublicIP")) == 0;
}

static bool IsText(LPCTSTR name)
{
    return _tcscmp(name, _T("Device")) == 0 || _tcscmp(name, _T("MapFileLoc")) == 0 ||
        _tcscmp(name, _T("Title")) == 0 || _tcscmp(name, _T("Gate")) == 0;
}

BOOL jRegSetKey(LPCTSTR section, LPCTSTR name, DWORD type, LPBYTE data, DWORD size)
{
    if (!section || !name || !data) return FALSE;
    TCHAR text[1024] = {};
    if (type == REG_SZ) {
        if (size < sizeof(TCHAR) || size > sizeof(text) || size % sizeof(TCHAR)) return FALSE;
        memcpy(text, data, size);
        text[size / sizeof(TCHAR) - 1] = 0;
    } else {
        DWORD value = 0;
        if (type == REG_BINARY && size == 1) value = *data;
        else if (type == REG_DWORD && size == sizeof(DWORD)) memcpy(&value, data, sizeof(value));
        else return FALSE;
        if (IsAddress(name))
            _stprintf_s(text, _T("%u.%u.%u.%u"), value >> 24, (value >> 16) & 255, (value >> 8) & 255, value & 255);
        else _stprintf_s(text, _T("%lu"), value);
    }
    return WritePrivateProfileString(Section(section).c_str(), name, text, ConfigFile);
}

BOOL jRegGetKeyBuf(LPCTSTR section, LPCTSTR name, LPBYTE data, DWORD size)
{
    if (!section || !name || !data || !size) return FALSE;
    TCHAR text[1024] = {};
    DWORD length = GetPrivateProfileString(Section(section).c_str(), name, _T(""), text, 1024, ConfigFile);
    if (!length || length >= 1023) return FALSE;
    if (IsText(name)) {
        if ((length + 1) * sizeof(TCHAR) > size) return FALSE;
        memcpy(data, text, (length + 1) * sizeof(TCHAR));
        return TRUE;
    }
    DWORD value = 0;
    if (IsAddress(name)) {
        unsigned a, b, c, d;
        TCHAR extra;
        if (_stscanf(text, _T("%u.%u.%u.%u%c"), &a, &b, &c, &d, &extra) != 4 ||
            a > 255 || b > 255 || c > 255 || d > 255) return FALSE;
        value = (a << 24) | (b << 16) | (c << 8) | d;
    } else {
        TCHAR* end = NULL;
        errno = 0;
        unsigned long parsed = _tcstoul(text, &end, 10);
        if (errno || end == text || *end || text[0] == _T('-')) return FALSE;
        value = parsed;
    }
    if (_tcscmp(name, _T("Installed")) == 0) {
        if (value > 1) return FALSE;
        *data = static_cast<BYTE>(value);
    } else {
        if (size < sizeof(value)) return FALSE;
        memcpy(data, &value, sizeof(value));
    }
    return TRUE;
}

BOOL jRegGetKey(LPCTSTR section, LPCTSTR name, LPBYTE data)
{
    // String callers use the sized API; this wrapper handles legacy scalar calls.
    if (IsText(name)) return FALSE;
    return jRegGetKeyBuf(section, name, data, _tcscmp(name, _T("Installed")) == 0 ? 1 : sizeof(DWORD));
}

LONG jRegEnumKey(LPCTSTR section, int index, LPTSTR output, int capacity)
{
    if (!section || !output || index < 0 || capacity <= 0) return ERROR_INVALID_PARAMETER;
    std::vector<TCHAR> names(32768);
    DWORD length = GetPrivateProfileSectionNames(names.data(), static_cast<DWORD>(names.size()), ConfigFile);
    if (length >= names.size() - 2) return ERROR_MORE_DATA;
    std::basic_string<TCHAR> prefix = Section(section) + _T("\\");
    std::set<std::basic_string<TCHAR>> children;
    for (const TCHAR* p = names.data(); *p; p += _tcslen(p) + 1) {
        if (_tcsnicmp(p, prefix.c_str(), prefix.size()) != 0) continue;
        std::basic_string<TCHAR> child = p + prefix.size();
        child = child.substr(0, child.find(_T('\\')));
        if (!child.empty()) children.insert(child);
    }
    if (static_cast<size_t>(index) >= children.size()) return ERROR_NO_MORE_ITEMS;
    auto item = children.begin();
    std::advance(item, index);
    if (item->size() + 1 > static_cast<size_t>(capacity)) return ERROR_MORE_DATA;
    _tcscpy_s(output, capacity, item->c_str());
    return ERROR_SUCCESS;
}

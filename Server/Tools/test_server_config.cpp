#include <windows.h>
#include <tchar.h>
#include <cstdio>
#include <cstdlib>
BOOL jRegSetKey(LPCTSTR, LPCTSTR, DWORD, LPBYTE, DWORD);
BOOL jRegGetKey(LPCTSTR, LPCTSTR, LPBYTE);
BOOL jRegGetKeyBuf(LPCTSTR, LPCTSTR, LPBYTE, DWORD);
LONG jRegEnumKey(LPCTSTR, int, LPTSTR, int);
static void Check(bool value) { if (!value) { puts("INI test failed"); exit(1); } }
int main()
{
    TCHAR previous[MAX_PATH], directory[MAX_PATH], temporary[MAX_PATH];
    Check(GetCurrentDirectory(MAX_PATH, previous) > 0);
    Check(GetTempPath(MAX_PATH, directory) > 0);
    Check(GetTempFileName(directory, _T("m3i"), 0, temporary) != 0);
    Check(DeleteFile(temporary) && CreateDirectory(temporary, NULL) && SetCurrentDirectory(temporary));
    LPCTSTR section = _T("Software\\LegendOfMir\\LoginSvr\\Servers\\1");
    DWORD address = 0x7f000001, result = 0;
    Check(jRegSetKey(section, _T("PrivateIP"), REG_DWORD, (LPBYTE)&address, sizeof(address)));
    Check(jRegGetKey(section, _T("PrivateIP"), (LPBYTE)&result) && address == result);
    TCHAR title[] = _T("\x4f20\x5947");
    Check(jRegSetKey(section, _T("Title"), REG_SZ, (LPBYTE)title, sizeof(title)));
    TCHAR read[32] = {};
    Check(jRegGetKeyBuf(section, _T("Title"), (LPBYTE)read, sizeof(read)) && _tcscmp(title, read) == 0);
    Check(!jRegGetKeyBuf(section, _T("Title"), (LPBYTE)read, sizeof(TCHAR)));
    Check(jRegEnumKey(_T("Software\\LegendOfMir\\LoginSvr\\Servers"), 0, read, 32) == ERROR_SUCCESS && _tcscmp(read, _T("1")) == 0);
    Check(jRegEnumKey(_T("Software\\LegendOfMir\\LoginSvr\\Servers"), 1, read, 32) == ERROR_NO_MORE_ITEMS);
    Check(!jRegGetKey(section, _T("LocalPort"), (LPBYTE)&result));
    Check(WritePrivateProfileString(_T("LoginSvr\\Servers\\1"), _T("PrivateIP"), _T("999.0.0.1"), _T(".\\server.ini")));
    Check(!jRegGetKey(section, _T("PrivateIP"), (LPBYTE)&result));
    Check(DeleteFile(_T("server.ini")));
    Check(SetCurrentDirectory(previous) && RemoveDirectory(temporary));
    puts("INI configuration tests passed");
}

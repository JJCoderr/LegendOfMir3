// ****************************************************************************
//
//		Registry Handler 
//
//      All written by Jeong, Euy-heon
//      Copyright(C) 1997, 1998 Euy-heon, Jeong. All right reserved.
//
// ****************************************************************************

#include "stdafx.h"

// ****************************************************************************
//
//      jRegGetKey/jRegSetKey
//
//      Last update : 1997.07.16 (Euy-heon, Jeong)
//      Purpose     : Get and set Windows null-terminate string regstry keys. 
//
// ****************************************************************************

BOOL jRegSetKey(LPCTSTR pSubKeyName, LPCTSTR pValueName, DWORD dwFlags, 
						LPBYTE pValue, DWORD nValueSize)
{
	HKEY	hk = NULL;
	DWORD	dwDisposition = 0;

	if (RegCreateKeyEx(HKEY_LOCAL_MACHINE, pSubKeyName, 0, NULL, 
					REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, 
					&hk, &dwDisposition) == ERROR_SUCCESS)
	{
		if (RegSetValueEx(hk, pValueName, 0, dwFlags, pValue, nValueSize) != ERROR_SUCCESS)
		{
			RegCloseKey(hk);
			return FALSE;
		}

		RegCloseKey(hk);

		return TRUE;
	}

	return FALSE;
}

// 打开 HKLM 下的键：先按进程默认视图打开，失败再按 64 / 32 位视图各试一次。
// 32 位程序跑在 64 位系统上时，regedit 默认写的是 64 位视图、程序读的是
// Wow6432Node —— 两边错位就读不到配置，枚举也会一并失败。
static BOOL jRegOpenKeyRead(LPCTSTR pSubKeyName, HKEY *phk)
{
	const REGSAM	extra[] = { 0, KEY_WOW64_64KEY, KEY_WOW64_32KEY };

	if (!phk)
		return FALSE;

	*phk = NULL;

	for (int i = 0; i < (int)(sizeof(extra) / sizeof(extra[0])); i++)
	{
		if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, pSubKeyName, 0,
						 KEY_READ | extra[i], phk) == ERROR_SUCCESS)
			return TRUE;
	}

	*phk = NULL;

	return FALSE;
}

BOOL jRegGetKey(LPCTSTR pSubKeyName, LPCTSTR pValueName, LPBYTE pValue)
{
	HKEY	hk = NULL;
	DWORD	dwFlags = 0, dwValueSize = 0;

    if (jRegOpenKeyRead(pSubKeyName, &hk))
	{
		if (RegQueryValueEx(hk, pValueName, NULL, &dwFlags, NULL, &dwValueSize) != ERROR_SUCCESS)
		{
			RegCloseKey(hk);
			return FALSE;
		}

		RegQueryValueEx(hk, pValueName, NULL, &dwFlags, pValue, &dwValueSize);
		RegCloseKey(hk);

		return TRUE;
	}

	return FALSE;
}

// 枚举子键：键打不开时返回 ERROR_NO_MORE_ITEMS（而不是 -1），
// 调用方 "while (返回值 == ERROR_SUCCESS)" 的循环就一定能正常结束。
LONG jRegEnumKey(LPCTSTR pSubKeyName, int nIndex, LPTSTR lpSubKey, int nSubkeyLen)
{
	HKEY		hk = NULL;
	DWORD		dwSubKeyLen = (DWORD)nSubkeyLen;
	FILETIME	ft;
	LONG		lRet = ERROR_NO_MORE_ITEMS;

	if (!jRegOpenKeyRead(pSubKeyName, &hk))
		return ERROR_NO_MORE_ITEMS;

	lRet = RegEnumKeyEx(hk, (DWORD)nIndex, lpSubKey, &dwSubKeyLen, NULL, NULL, NULL, &ft);

	RegCloseKey(hk);

	return lRet;
}

// 带缓冲区大小的读值版本：注册表里的值再长也只会截断到调用方给的缓冲区内，
// 不会像 jRegGetKey 那样按查询到的大小整块写入（历史写法会写爆调用方的栈缓冲）。
BOOL jRegGetKeyBuf(LPCTSTR pSubKeyName, LPCTSTR pValueName, LPBYTE pValue, DWORD nValueSize)
{
	HKEY	hk = NULL;
	DWORD	dwType = 0, dwSize = nValueSize;
	LONG	lRet;

	if (!pValue || nValueSize == 0)
		return FALSE;

	if (!jRegOpenKeyRead(pSubKeyName, &hk))
		return FALSE;

	lRet = RegQueryValueEx(hk, pValueName, NULL, &dwType, pValue, &dwSize);

	RegCloseKey(hk);

	return (lRet == ERROR_SUCCESS);
}

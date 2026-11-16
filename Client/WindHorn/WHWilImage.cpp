#include "StdAfx.h"



CWHWilImageData::CWHWilImageData()
{
	Init();
}


CWHWilImageData::~CWHWilImageData()
{
	Destroy();
}


VOID CWHWilImageData::Init()
{
	m_bIsMemMapped		= FALSE;
	m_bIsCompressed		= FALSE;
	m_pbStartData		= NULL;	

	ZeroMemory(&m_stWixImgaeInfo, sizeof(WIXIMAGEINFO));

	ZeroMemory(&m_stNewWixImgaeInfo, sizeof(NEWWIXIMAGEINFO));

	ZeroMemory(&m_szWilFileName, sizeof(m_szWilFileName));

	m_nCurrImageIdx		= -1;								// 初始设置值未设定的状态。
	m_pbCurrImage		= NULL;
	m_lpstCurrWilImageInfo = NULL;

	m_lpstNewCurrWilImageInfo = NULL;
}


VOID CWHWilImageData::Destroy()
{
	if ( m_pbStartData ) 
	{
		if ( m_bIsMemMapped )
		{
			UnmapViewOfFile(m_pbStartData);
			m_pbStartData = NULL;
		}
		else
		{
			delete[] m_pbStartData;
			m_pbStartData = NULL;
		}
	}

	delete[] m_stWixImgaeInfo.pnPosition;
	ZeroMemory(&m_stWixImgaeInfo, sizeof(WIXIMAGEINFO));

	delete[] m_stNewWixImgaeInfo.pnPosition;
	ZeroMemory(&m_stNewWixImgaeInfo, sizeof(NEWWIXIMAGEINFO));

	Init();
}



BOOL CWHWilImageData::Load(CHAR* szWilFile, BOOL bIsMemMapped, BOOL bComp)
{
	m_bIsMemMapped	= bIsMemMapped;
	m_bIsCompressed	= bComp;

	HANDLE	hWilFile;
	HANDLE	hWixFile;
	HANDLE	hWilMappedFile;
	DWORD	dwReadLen;
	CHAR	szWixFile[MAX_PATH];
	CHAR	szMsg[MAX_PATH];

	strcpy(szWixFile, szWilFile);
	strcpy(szWixFile+strlen(szWixFile)-3, "wix");
	strcpy(m_szWilFileName, szWilFile);

	hWixFile= CreateFile(szWixFile, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if ( hWixFile )
	{
		ReadFile(hWixFile, &m_stWixImgaeInfo, sizeof(WIXIMAGEINFO)-sizeof(INT*), &dwReadLen, NULL);

		m_stWixImgaeInfo.pnPosition = new INT[m_stWixImgaeInfo.nIndexCount];
		ReadFile(hWixFile, m_stWixImgaeInfo.pnPosition, sizeof(INT)*m_stWixImgaeInfo.nIndexCount, &dwReadLen, NULL);

		hWilFile = CreateFile(szWilFile, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );

		if ( hWilFile != INVALID_HANDLE_VALUE )
		{
			// 内存映射形态时.
			if ( m_bIsMemMapped )
			{
				hWilMappedFile = CreateFileMapping(hWilFile, NULL, PAGE_READONLY, NULL, NULL, NULL);

				m_pbStartData = (BYTE*)MapViewOfFile(hWilMappedFile, FILE_MAP_READ, NULL, NULL, NULL);

				if ( !m_pbStartData )
				{
					// Debug
					MessageBox(NULL, "CreateFileMapping Failed", "Mir2EX :: Error", MB_ICONERROR | MB_OK);
					CloseHandle(hWilFile);
					CloseHandle(hWixFile);
					return FALSE;
				}
				CloseHandle(hWilMappedFile);
			}
			else
			{
				DWORD dwFileSize;
				dwFileSize = GetFileSize(hWilFile, NULL);
				m_pbStartData = new BYTE[dwFileSize];
				ReadFile(hWilFile, m_pbStartData, dwFileSize, &dwReadLen, NULL);
			}

			SetIndex(0);						// 设置为索引 0.
			CloseHandle(hWilFile);
			CloseHandle(hWixFile);
			return TRUE;
		}

		wsprintf(szMsg, "Cannot find %s file", szWixFile);
		// Debug
		MessageBox(NULL, szMsg, "Mir2EX :: Error", MB_ICONERROR | MB_OK);
		CloseHandle(hWixFile);
		return FALSE;
	}

	wsprintf(szMsg, "Cannot find %s file", szWixFile);
	// Debug
	MessageBox(NULL, szMsg, "Mir2EX :: Error", MB_ICONERROR | MB_OK);

	return FALSE;

}



VOID CWHWilImageData::SetIndex(INT nIndex)
{
	if ( m_nCurrImageIdx == nIndex )	return;				// 若新设置的 Index 没有变化.

	m_nCurrImageIdx = nIndex;

	BYTE* pbTmpData = (BYTE*)(m_pbStartData + m_stWixImgaeInfo.pnPosition[nIndex]);

	m_lpstCurrWilImageInfo = (LPWILIMAGEINFO)pbTmpData;

	if ( m_bIsCompressed )
		m_pbCurrImage = (BYTE*)(pbTmpData + sizeof(WILIMAGEINFO));
	else
		m_pbCurrImage = (BYTE*)(pbTmpData + sizeof(WILIMAGEINFO) - sizeof(DWORD));
}


BOOL CWHWilImageData::NewSetIndex(DWORD dwIndex)
{
//	if ( m_nCurrImageIdx == nIndex || !m_stNewWixImgaeInfo.pnPosition[nIndex] )	return;				// 如果新 Setting 的 Index 没有变化.

	m_nCurrImageIdx = dwIndex;

	if ( m_stNewWixImgaeInfo.pnPosition && dwIndex < (DWORD)m_stNewWixImgaeInfo.nIndexCount && m_stNewWixImgaeInfo.pnPosition[dwIndex] != 0 )
	{
		BYTE* pbTmpData = (BYTE*)(m_pbStartData + m_stNewWixImgaeInfo.pnPosition[dwIndex]);

		m_lpstNewCurrWilImageInfo = (LPNEWWILIMAGEINFO)pbTmpData;

		if ( m_bIsCompressed )
			m_pbCurrImage = (BYTE*)(pbTmpData + sizeof(NEWWILIMAGEINFO));
		else
			m_pbCurrImage = (BYTE*)(pbTmpData + sizeof(NEWWILIMAGEINFO) - sizeof(DWORD));

		return TRUE;
	}
	return FALSE;
}


BOOL CWHWilImageData::NewLoad(CHAR* szWilFile, BOOL bIsMemMapped, BOOL bComp)
{
	m_bIsMemMapped	= bIsMemMapped;
	m_bIsCompressed	= bComp;

	HANDLE	hWilFile;
	HANDLE	hWixFile;
	HANDLE	hWilMappedFile;
	DWORD	dwReadLen;
	CHAR	szWixFile[MAX_PATH];
	CHAR	szMsg[MAX_PATH];

	strcpy(szWixFile, szWilFile);
	strcpy(szWixFile+strlen(szWixFile)-3, "wix");
	strcpy(m_szWilFileName, szWilFile);

	hWixFile= CreateFile(szWixFile, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if ( hWixFile != INVALID_HANDLE_VALUE )
	{
        const DWORD headerSize = sizeof(NEWWIXIMAGEINFO) - sizeof(INT*);
        DWORD indexSize = GetFileSize(hWixFile, NULL);
        if (!ReadFile(hWixFile, &m_stNewWixImgaeInfo, headerSize, &dwReadLen, NULL) ||
            dwReadLen != headerSize || m_stNewWixImgaeInfo.nIndexCount <= 0) {
            CloseHandle(hWixFile);
            return FALSE;
        }
        ULONGLONG expected = headerSize + (ULONGLONG)m_stNewWixImgaeInfo.nIndexCount * sizeof(INT);
        // V1.45 adds a four-byte field after the index count.
        if (indexSize == expected + 4)
            SetFilePointer(hWixFile, 4, NULL, FILE_CURRENT);
        else if (indexSize != expected) {
            CloseHandle(hWixFile);
            return FALSE;
        }

		m_stNewWixImgaeInfo.pnPosition = new INT[m_stNewWixImgaeInfo.nIndexCount];
		ReadFile(hWixFile, m_stNewWixImgaeInfo.pnPosition, sizeof(INT)*m_stNewWixImgaeInfo.nIndexCount, &dwReadLen, NULL);

		hWilFile = CreateFile(szWilFile, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );

		if ( hWilFile != INVALID_HANDLE_VALUE )
		{
			// 内存映射形态时.
			if ( m_bIsMemMapped )
			{
				hWilMappedFile = CreateFileMapping(hWilFile, NULL, PAGE_READONLY, NULL, NULL, NULL);

				m_pbStartData = (BYTE*)MapViewOfFile(hWilMappedFile, FILE_MAP_READ, NULL, NULL, NULL);

				if ( !m_pbStartData )
				{
					// Debug
					MessageBox(NULL, "CreateFileMapping Failed", "Mir2EX :: Error", MB_ICONERROR | MB_OK);
					CloseHandle(hWilFile);
					CloseHandle(hWixFile);
					return FALSE;
				}
				CloseHandle(hWilMappedFile);
			}
			else
			{
				DWORD dwFileSize;
				dwFileSize = GetFileSize(hWilFile, NULL);
				m_pbStartData = new BYTE[dwFileSize];
				ReadFile(hWilFile, m_pbStartData, dwFileSize, &dwReadLen, NULL);
			}

			NewSetIndex(0);						// 设置为索引 0.
			CloseHandle(hWilFile);
			CloseHandle(hWixFile);
			return TRUE;
		}

		wsprintf(szMsg, "Cannot find %s file", szWixFile);
		// Debug
		MessageBox(NULL, szMsg, "Mir2EX :: Error", MB_ICONERROR | MB_OK);
		CloseHandle(hWixFile);
		return FALSE;
	}

	wsprintf(szMsg, "Cannot find %s file", szWixFile);
	// Debug
	MessageBox(NULL, szMsg, "Mir2EX :: Error", MB_ICONERROR | MB_OK);

	return FALSE;

}

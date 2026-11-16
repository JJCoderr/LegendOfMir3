// ConsoleSocket.cpp : implementation file
//

#include "stdafx.h"

#include "ManConsole.h"
#include "ConsoleSocket.h"

/////////////////////////////////////////////////////////////////////////////
// CConsoleSocket

CConsoleSocket::CConsoleSocket()
{
	m_hSocket = INVALID_SOCKET;
}

CConsoleSocket::~CConsoleSocket()
{
	Close();
}

/////////////////////////////////////////////////////////////////////////////
// CConsoleSocket member functions

BOOL CConsoleSocket::Create(UINT nSocketPort, int nSocketType, LPCTSTR lpszSocketAddress)
{
	Close();

	m_hSocket = socket(AF_INET, nSocketType, 0);

	if (m_hSocket == INVALID_SOCKET)
		return FALSE;

	if (nSocketPort == 0 && lpszSocketAddress == NULL)
		return TRUE;

	SOCKADDR_IN	addr;

	ZeroMemory(&addr, sizeof(addr));

	addr.sin_family	= AF_INET;
	addr.sin_port	= htons((u_short)nSocketPort);

	if (lpszSocketAddress != NULL)
	{
		if (InetPton(AF_INET, lpszSocketAddress, &addr.sin_addr) != 1)
		{
			Close();
			return FALSE;
		}
	}
	else
	{
		addr.sin_addr.s_addr = htonl(INADDR_ANY);
	}

	if (bind(m_hSocket, (SOCKADDR*)&addr, sizeof(addr)) == SOCKET_ERROR)
	{
		Close();
		return FALSE;
	}

	return TRUE;
}

BOOL CConsoleSocket::Connect(LPCTSTR lpszHostAddress, UINT nHostPort)
{
	ADDRINFOW		hints;
	PADDRINFOW		pResult = NULL;
	TCHAR			szPort[16];

	ZeroMemory(&hints, sizeof(hints));

	hints.ai_family		= AF_INET;
	hints.ai_socktype	= SOCK_STREAM;

	_stprintf(szPort, _T("%u"), nHostPort);

	if (GetAddrInfoW(lpszHostAddress, szPort, &hints, &pResult) != 0)
		return FALSE;

	if (m_hSocket == INVALID_SOCKET)
		m_hSocket = socket(pResult->ai_family, pResult->ai_socktype, pResult->ai_protocol);

	BOOL bResult = (m_hSocket != INVALID_SOCKET)
				&& (connect(m_hSocket, pResult->ai_addr, (int)pResult->ai_addrlen) != SOCKET_ERROR);

	FreeAddrInfoW(pResult);

	if (!bResult)
		Close();

	return bResult;
}

BOOL CConsoleSocket::Connect(const SOCKADDR* lpSockAddr, int nSockAddrLen)
{
	if (m_hSocket == INVALID_SOCKET)
		m_hSocket = socket(AF_INET, SOCK_STREAM, 0);

	if (m_hSocket == INVALID_SOCKET)
		return FALSE;

	if (connect(m_hSocket, lpSockAddr, nSockAddrLen) == SOCKET_ERROR)
	{
		Close();
		return FALSE;
	}

	return TRUE;
}

BOOL CConsoleSocket::Bind(const SOCKADDR* lpSockAddr, int nSockAddrLen)
{
	return (bind(m_hSocket, lpSockAddr, nSockAddrLen) != SOCKET_ERROR);
}

BOOL CConsoleSocket::Listen(int nConnectionBacklog)
{
	return (listen(m_hSocket, nConnectionBacklog) != SOCKET_ERROR);
}

BOOL CConsoleSocket::Accept(CConsoleSocket& rConnectedSocket)
{
	SOCKADDR_IN	addr;
	int			nSockAddrLen = sizeof(addr);

	SOCKET hSocket = accept(m_hSocket, (SOCKADDR*)&addr, &nSockAddrLen);

	if (hSocket == INVALID_SOCKET)
		return FALSE;

	rConnectedSocket.Attach(hSocket);

	return TRUE;
}

BOOL CConsoleSocket::Attach(SOCKET hSocket)
{
	Close();

	m_hSocket = hSocket;

	return (m_hSocket != INVALID_SOCKET);
}

SOCKET CConsoleSocket::Detach()
{
	SOCKET hSocket = m_hSocket;

	m_hSocket = INVALID_SOCKET;

	return hSocket;
}

void CConsoleSocket::Close()
{
	if (m_hSocket != INVALID_SOCKET)
	{
		closesocket(m_hSocket);

		m_hSocket = INVALID_SOCKET;
	}
}

int CConsoleSocket::Receive(void* lpBuf, int nBufLen, int nFlags)
{
	return recv(m_hSocket, (char*)lpBuf, nBufLen, nFlags);
}

int CConsoleSocket::Send(const void* lpBuf, int nBufLen, int nFlags)
{
	return send(m_hSocket, (const char*)lpBuf, nBufLen, nFlags);
}

BOOL CConsoleSocket::ShutDown(int nHow)
{
	return (shutdown(m_hSocket, nHow) != SOCKET_ERROR);
}

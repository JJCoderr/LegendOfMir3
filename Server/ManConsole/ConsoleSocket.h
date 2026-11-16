#if !defined(AFX_CONSOLESOCKET_H__C2E973DB_90BA_4535_9633_38278EAB8EDB__INCLUDED_)
#define AFX_CONSOLESOCKET_H__C2E973DB_90BA_4535_9633_38278EAB8EDB__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ConsoleSocket.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CConsoleSocket
//
// Blocking socket wrapper used by the console to talk to the game servers.

class CConsoleSocket
{
// Attributes
public:
	SOCKET	GetSocket() const						{ return m_hSocket; }
	BOOL	IsValid() const							{ return (m_hSocket != INVALID_SOCKET); }
	operator SOCKET() const							{ return m_hSocket; }

// Operations
public:
	CConsoleSocket();
	virtual ~CConsoleSocket();

	BOOL	Create(UINT nSocketPort = 0, int nSocketType = SOCK_STREAM, LPCTSTR lpszSocketAddress = NULL);
	BOOL	Connect(LPCTSTR lpszHostAddress, UINT nHostPort);
	BOOL	Connect(const SOCKADDR* lpSockAddr, int nSockAddrLen);
	BOOL	Bind(const SOCKADDR* lpSockAddr, int nSockAddrLen);
	BOOL	Listen(int nConnectionBacklog = 5);
	BOOL	Accept(CConsoleSocket& rConnectedSocket);
	BOOL	Attach(SOCKET hSocket);
	SOCKET	Detach();
	void	Close();

	int		Receive(void* lpBuf, int nBufLen, int nFlags = 0);
	int		Send(const void* lpBuf, int nBufLen, int nFlags = 0);
	BOOL	ShutDown(int nHow = SD_SEND);

// Implementation
protected:
	SOCKET	m_hSocket;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CONSOLESOCKET_H__C2E973DB_90BA_4535_9633_38278EAB8EDB__INCLUDED_)

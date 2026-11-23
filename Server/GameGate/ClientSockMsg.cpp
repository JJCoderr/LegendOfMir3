#include "stdafx.h"
#include "../Service/PacketLog.inl"

DWORD WINAPI	ThreadFuncForMsg(LPVOID lpParameter);
BOOL			CheckSocketError(LPARAM lParam);
VOID WINAPI		OnTimerProc(HWND hWnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime);

UINT WINAPI		ClientWorkerThread(LPVOID lpParameter);

extern SOCKET					g_csock;

extern HWND						g_hMainWnd;
extern HWND						g_hStatusBar;

extern CWHDynamicArray<CSessionInfo>	g_UserInfoArray;


HANDLE							g_hThreadForComm = NULL;
HANDLE							g_hSvrMsgEvnt = NULL;

char							g_szRemainBuff[DATA_BUFSIZE * 2];
int								g_nRemainBuffLen = 0;

//CWHQueue						g_SendToUserQ;

WSAEVENT						g_ClientIoEvent;

void ProcMakeSocketStr(char *lpMsg)
{
    const auto message = reinterpret_cast<_LPTMSGHEADER>(lpMsg);
    auto session = g_UserInfoArray.GetData(message->wUserGateIndex);
    if (!session) return;
    const char* body = lpMsg + sizeof(_TMSGHEADER);
    size_t bodySize;
    char header[_DEFBLOCKSIZE + 1]{};
    size_t headerSize = 0;
    if (message->nLength < 0) {
        bodySize = static_cast<size_t>(-static_cast<long long>(message->nLength));
    } else {
        if (message->nLength < sizeof(_TDEFAULTMESSAGE)) return;
        headerSize = fnEncodeMessageA(reinterpret_cast<_LPTDEFAULTMESSAGE>(const_cast<char*>(body)), header, sizeof(header));
        body += sizeof(_TDEFAULTMESSAGE);
        bodySize = message->nLength - sizeof(_TDEFAULTMESSAGE);
    }
    AcquireSRWLockExclusive(&session->sendLock);
    if (session->sock == SOCKET(message->nSocket)) {
        const size_t start = session->SendBuffer.size();
        if (gatewire::AppendFrame(session->SendBuffer, header, headerSize, body, bodySize)) {
            TracePacket(L"TX", session->sock, session->SendBuffer.c_str() + start + 1);
        } else {
            mir3::ConsoleLog(L"GameGate", L"Client send queue limit exceeded; disconnecting.");
            shutdown(session->sock, SD_BOTH);
        }
    }
    ReleaseSRWLockExclusive(&session->sendLock);
}

void ProcReceiveBuffer(char *pszPacket, int nRecv)
{
	int				nLen = nRecv;
	int				nNext = 0;
	char			szBuff[sizeof(g_szRemainBuff)];
	char			*pszData = &szBuff[0];
	_LPTMSGHEADER	lpMsgHeader;

	if (g_nRemainBuffLen > 0)
		memmove(szBuff, g_szRemainBuff, g_nRemainBuffLen);

	memmove(&szBuff[g_nRemainBuffLen], pszPacket, nLen);

	nLen += g_nRemainBuffLen;

	while (nLen >= sizeof(_TMSGHEADER))
	{
		lpMsgHeader = (_LPTMSGHEADER)pszData;

		if (nLen < (int)(sizeof(_TMSGHEADER) + lpMsgHeader->nLength)) break;

		if (lpMsgHeader->nCode == 0xAA55AA55)
		{
			switch (lpMsgHeader->wIdent)
			{
				case GM_CHECKSERVER:
						// Received keep alive check code from game server
					break;
				case GM_SERVERUSERINDEX:
				{
					CSessionInfo* pSessionInfo = g_UserInfoArray.GetData(lpMsgHeader->wUserGateIndex);

					if (pSessionInfo && pSessionInfo->sock == (SOCKET)lpMsgHeader->nSocket)
					{
						pSessionInfo->nServerUserIndex = lpMsgHeader->wUserListIndex;
						pSessionInfo->Recv();
					}

					break;
				}
				case GM_RECEIVE_OK:
					SendSocketMsgS(GM_RECEIVE_OK, 0, 0, 0, 0, NULL);
					break;
				case GM_DATA:
					ProcMakeSocketStr(pszData);
					break;
				case GM_TEST:
					break;
			}

			pszData += sizeof(_TMSGHEADER) + abs(lpMsgHeader->nLength);
			nLen -= sizeof(_TMSGHEADER) + abs(lpMsgHeader->nLength);
		}
		else
		{
			pszData++;
			nLen--;
		}
	} // while

	if (nLen > 0)
	{
		memmove(g_szRemainBuff, pszData, nLen);
		g_nRemainBuffLen = nLen;
#ifdef _DEBUG
	_RPT2(_CRT_WARN, "REMAIN:%d, %s\n", g_nRemainBuffLen, pszData);
#endif
	}
	else
	{
		g_nRemainBuffLen = 0;
	}
}

/*
BOOL InitServerThreadForComm()
{
	DWORD	dwThreadIDForComm = 0;

	if (!g_hSvrMsgEvnt)
		g_hSvrMsgEvnt = CreateEvent(NULL, FALSE, FALSE, NULL);

	if (!g_hThreadForComm)
	{
		g_hThreadForComm	= CreateThread(NULL, 0, ThreadFuncForComm,	NULL, 0, &dwThreadIDForComm);

		if (g_hThreadForComm)
			return TRUE;
	}

	return FALSE;
}
*/
BOOL InitServerThreadForMsg()
{
	DWORD	dwThreadIDForMsg = 0;

	HANDLE hThreadForMsg	= CreateThread(NULL, 0, ThreadFuncForMsg,	NULL, 0, &dwThreadIDForMsg);

	if (hThreadForMsg)
	{
		CloseHandle(hThreadForMsg);
	
		return TRUE;
	}

	return FALSE;
}

LPARAM OnClientSockMsg(WPARAM wParam, LPARAM lParam)
{
	switch (WSAGETSELECTEVENT(lParam))
	{
		case FD_CONNECT:
		{
			if (CheckSocketError(lParam))
			{
				if (InitServerThreadForMsg())
				{
					g_nRemainBuffLen = 0;

					KillTimer(g_hMainWnd, _ID_TIMER_CONNECTSERVER);
					
					SetTimer(g_hMainWnd, _ID_TIMER_KEEPALIVE, 50000, (TIMERPROC)OnTimerProc);
//					SetTimer(g_hMainWnd, _ID_TIMER_KEEPALIVE, 1000, (TIMERPROC)OnTimerProc);

					InsertLogMsg(IDS_CONNECT_LOGINSERVER);


					//
					UINT			dwThreadIDForMsg = 0;
					unsigned long	hThreadForMsg = 0;

					g_ClientIoEvent = WSACreateEvent();

					hThreadForMsg = _beginthreadex(NULL, 0, ClientWorkerThread, NULL, 0, &dwThreadIDForMsg);
				}
			}
			else
			{
				closesocket(g_csock);
				g_csock = INVALID_SOCKET;

				SetTimer(g_hMainWnd, _ID_TIMER_CONNECTSERVER, 10000, (TIMERPROC)OnTimerProc);
			}

			break;
		}
/*		case FD_CLOSE:
		{
			closesocket(g_csock);
			g_csock = INVALID_SOCKET;

			KillTimer(g_hMainWnd, _ID_TIMER_KEEPALIVE);
			
			SetTimer(g_hMainWnd, _ID_TIMER_CONNECTSERVER, 10000, (TIMERPROC)OnTimerProc);

			InsertLogMsg(IDS_DISCONNECT_LOGINSERVER);


			break;
		}
		case FD_READ:
		{
			char	szPacket[1024];
	
			int nRecv = recv((SOCKET)wParam, szPacket, sizeof(szPacket), 0);

			szPacket[nRecv] = '\0';

			ProcReceiveBuffer(szPacket, nRecv);

			break;
		}
*/	}

	return 0L;
}

UINT WINAPI		ClientWorkerThread(LPVOID lpParameter)
{
	_TOVERLAPPEDEX		ClientOverlapped;
	DWORD				dwIndex;
	DWORD				dwBytesTransferred;
	DWORD				dwFlags;
	DWORD				dwRecvBytes;
		
//	char				*pszPos;
//	int					nSocket;

	ZeroMemory(&ClientOverlapped.Overlapped, sizeof(WSAOVERLAPPED));

	ClientOverlapped.Overlapped.hEvent = g_ClientIoEvent;

	ClientOverlapped.DataBuf.len	= DATA_BUFSIZE;
	ClientOverlapped.DataBuf.buf	= &ClientOverlapped.Buffer[0];

	dwFlags = 0;

	if (WSARecv(g_csock, &(ClientOverlapped.DataBuf), 1, &dwRecvBytes, &dwFlags, &(ClientOverlapped.Overlapped), NULL) == SOCKET_ERROR)
	{
		if (WSAGetLastError() != ERROR_IO_PENDING)
			return 0;		
	}

	while (TRUE)
	{
		dwIndex = WSAWaitForMultipleEvents(1, &g_ClientIoEvent, FALSE, WSA_INFINITE, FALSE);

		WSAResetEvent(g_ClientIoEvent);

		WSAGetOverlappedResult(g_csock, &ClientOverlapped.Overlapped, &dwBytesTransferred, FALSE, &dwFlags);

		if (dwBytesTransferred == 0)
			break;

		ClientOverlapped.DataBuf.buf[dwBytesTransferred] = '\0';

/*#ifdef _DEBUG
		_RPT1(_CRT_WARN, "%d:", dwBytesTransferred);

		for (int i = 0; i < dwBytesTransferred; i++)
			_RPT1(_CRT_WARN, "%c", ClientOverlapped.DataBuf.buf[i]);
		_RPT0(_CRT_WARN, "\n");
#endif*/

		ProcReceiveBuffer(ClientOverlapped.DataBuf.buf, dwBytesTransferred);

		dwFlags = 0;

		ZeroMemory(&(ClientOverlapped.Overlapped), sizeof(OVERLAPPED));

		ClientOverlapped.DataBuf.len = DATA_BUFSIZE;
		ClientOverlapped.DataBuf.buf = &ClientOverlapped.Buffer[0];

		ClientOverlapped.Overlapped.hEvent = g_ClientIoEvent;

		if (WSARecv(g_csock, &(ClientOverlapped.DataBuf), 1, &dwRecvBytes, &dwFlags, &(ClientOverlapped.Overlapped), NULL) == SOCKET_ERROR)
		{
			if (WSAGetLastError() != ERROR_IO_PENDING)
				break;
		}
	}

	return 0L;
}

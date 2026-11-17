#include "ClientRpc.h"
#include "stdafx.h"

CWHQueue PacketQ;

CClientSocket::CClientSocket()
{
	m_sockClient = INVALID_SOCKET;
	m_pxDefProc	 = NULL;

	m_nSync = 1;
}


CClientSocket::~CClientSocket()
{
	m_sockClient = INVALID_SOCKET;
	m_pxDefProc	 = NULL;
}


unsigned long CClientSocket::GetINetAddr(CHAR* pszDotAddress)
{
	if ( inet_addr(pszDotAddress) == -1L )
	{
		struct hostent* pHostEnt;

		pHostEnt = gethostbyname(pszDotAddress);
		return *(unsigned long*)pHostEnt->h_addr_list[0];
	}
	
	return inet_addr(pszDotAddress);
}


BOOL CClientSocket::ConnectToServer(HWND window, CHAR* host, INT port)
{
    if(m_sockClient!=INVALID_SOCKET) return TRUE;
    m_connectionWindow=window;m_connectionHost=host;m_connectionPort=port;m_retry={};
    m_sockClient=socket(AF_INET,SOCK_STREAM,0);
    if(m_sockClient==INVALID_SOCKET) {ConnectionFailed();return FALSE;}
    SOCKADDR_IN address={};address.sin_family=AF_INET;address.sin_port=htons(port);address.sin_addr.s_addr=inet_addr(host);
    if(WSAAsyncSelect(m_sockClient,window,ID_SOCKCLIENT_EVENT_MSG,FD_CONNECT|FD_READ|FD_CLOSE)==SOCKET_ERROR) {
        ConnectionFailed();return FALSE;
    }
    if(connect(m_sockClient,(const sockaddr*)&address,sizeof(address))==SOCKET_ERROR && WSAGetLastError()!=WSAEWOULDBLOCK) {
        ConnectionFailed();return FALSE;
    }
    m_nSync=1;return TRUE;
}
void CClientSocket::ConnectionFailed()
{
    DisconnectToServer();m_retry.Fail();
}
void CClientSocket::AnswerConnection(bool retry,DWORD now)
{
    m_retry.Answer(retry,now);
    if(retry) EnableWindow(m_connectionWindow,FALSE);
    else PostMessage(m_connectionWindow,WM_CLOSE,0,0);
}
bool CClientSocket::PollConnection(DWORD now)
{
    if(m_retry.phase==ConnectionRetry::Phase::Prompt) {
        static CLoginMessages messages;static const bool loaded=messages.Load("./CmList.dat");
        m_retryFocus=GetFocus();
        const int answer=MessageBoxA(m_connectionWindow,messages.Get(ConnectionRetry::Message).c_str(),ConnectionRetry::Title,MB_YESNO);
        AnswerConnection(answer==IDYES,GetTickCount());
        return true;
    }
    if(m_retry.Due(now)) {
        EnableWindow(m_connectionWindow,TRUE);
        const std::string host=m_connectionHost;
        ConnectToServer(m_connectionWindow,const_cast<char*>(host.c_str()),m_connectionPort);
        if(IsWindow(m_retryFocus)) SetFocus(m_retryFocus);
        return true;
    }
    return m_retry.Blocks();
}

BOOL CClientSocket::DisconnectToServer()
{
    if(m_retry.phase==ConnectionRetry::Phase::Waiting) EnableWindow(m_connectionWindow,TRUE);
    m_retry={};
	if ( m_sockClient != INVALID_SOCKET )
	{
        // Cancel queued notifications before Windows can reuse this socket handle.
        WSAAsyncSelect(m_sockClient,g_xMainWnd.GetSafehWnd(),ID_SOCKCLIENT_EVENT_MSG,0);
        MSG pending;
        while(PeekMessage(&pending,g_xMainWnd.GetSafehWnd(),ID_SOCKCLIENT_EVENT_MSG,ID_SOCKCLIENT_EVENT_MSG,PM_REMOVE)) {}
		closesocket(m_sockClient);
		m_sockClient = INVALID_SOCKET;

		return TRUE;
	}

	return FALSE;
}

/*
VOID CClientSocket::SendToServer(CHAR *pszPacket)
{
	CHAR				szPacket[1024];
	static INT nCode = 1;

	wsprintf(szPacket, "#%d%s!", nCode, pszPacket);

	if (nCode >= 9) nCode = 1;
	else
		nCode++;

	send(m_sockClient, szPacket, lstrlen(szPacket), 0);
}
*/

VOID CClientSocket::SendActMsg(WORD wIdent, INT nX, INT nY, INT nDir)
{
	fnMakeDefMessage(&DefMsg, wIdent, MAKELONG(nX, nY), 0, nDir, 0);
	SendPacket(&DefMsg, NULL);
}

VOID CClientSocket::SendHitMsg(WORD wIdent, INT nX, INT nY, INT nDir, WORD wHitStyle)
{
	fnMakeDefMessage(&DefMsg, wIdent, MAKELONG(nX, nY), 0, nDir, 0);
	SendPacket(&DefMsg, NULL);
}

VOID CClientSocket::SendRideMsg(WORD wIdent, INT nDir, BOOL bRide)
{
	// nDir  = wParam;
	// bRide = lParam1;
	fnMakeDefMessage(&DefMsg, wIdent, 0, nDir, bRide, 0);
	SendPacket(&DefMsg, NULL);
}


VOID CClientSocket::SendRunLogin(CHAR* szLoginID, CHAR* szCharName, INT nCertification, INT VersionNum)
{
	CHAR				szEncodeMsg[512];
	CHAR				szLoginInfo[512];
	CHAR				szPacket[512];

	wsprintf(szLoginInfo, "**%s/%s/%d/%d/1", szLoginID, szCharName, nCertification, VersionNum);

	int nPos = fnEncode6BitBuf((BYTE*)szLoginInfo, szEncodeMsg, lstrlen(szLoginInfo), sizeof(szEncodeMsg));
	szEncodeMsg[nPos] = '\0';

	((m_nSync >= 9) ? m_nSync = 1 : m_nSync++);
	wsprintf(szPacket, "#%d%s!", m_nSync, szEncodeMsg);

	send(m_sockClient, szPacket, strlen(szPacket), 0);
}

BOOL CClientSocket::CheckSocketError(LPARAM parameter)
{
    return WSAGETSELECTERROR(parameter)==0;
}

VOID CClientSocket::SendQueryMsg(WORD wIdent)
{
	fnMakeDefMessage(&DefMsg, wIdent, 0, 0, 0, 0);
	SendPacket(&DefMsg, NULL);
}


VOID CClientSocket::SendTakeOnOffItem(WORD wIdent, BYTE bEquipNum, CHAR* szItemName, INT nItemIdx)
{
	fnMakeDefMessage(&DefMsg, wIdent, nItemIdx, bEquipNum, 0, 0);
	SendPacket(&DefMsg, szItemName);
}


VOID CClientSocket::SendQueryName(INT nRecog, INT nX, INT nY)
{
	fnMakeDefMessage(&DefMsg, CM_QUERYUSERNAME, nRecog, nX, nY, 0);
	SendPacket(&DefMsg, NULL);
}


VOID CClientSocket::SendNPCClick(INT nRecog)
{
	fnMakeDefMessage(&DefMsg, CM_CLICKNPC, nRecog, 0, 0, 0);
	SendPacket(&DefMsg, NULL);
}


VOID CClientSocket::SendMagicKeyChange(INT nKeyNum, INT nMagicID)
{
	fnMakeDefMessage(&DefMsg, CM_MAGICKEYCHANGE, nMagicID, nKeyNum, 0, 0);
	SendPacket(&DefMsg, NULL);
}


VOID CClientSocket::SendItemIndex(WORD wIdent, INT nItemIndex, CHAR* pszItemName)
{
	fnMakeDefMessage(&DefMsg, wIdent, nItemIndex, 0, 0, 0);
	SendPacket(&DefMsg, pszItemName);
}

VOID CClientSocket::SendPickUp(INT nX, INT nY)
{
	fnMakeDefMessage(&DefMsg, CM_PICKUP, 0, nX, nY, 0);
	SendPacket(&DefMsg, NULL);
}

VOID CClientSocket::SendButchAnimal(INT nX, INT nY, INT nDir, INT nActorID)
{
	fnMakeDefMessage(&DefMsg, CM_BUTCH, nActorID, nX, nY, nDir);
	SendPacket(&DefMsg, NULL);
}

LRESULT CClientSocket::OnSocketMessage(WPARAM wParam, LPARAM lParam)
{
    if(m_sockClient==INVALID_SOCKET || (SOCKET)wParam!=m_sockClient) return FALSE;
	if ( !CheckSocketError(lParam) )
	{
		ConnectionFailed();
		return FALSE;
	}

	switch ( WSAGETSELECTEVENT(lParam) )
	{
		case FD_CONNECT:
		{
			ZeroMemory(m_szBuff, sizeof(m_szBuff));
			m_nCurRecv = 0;

			if (m_pxDefProc)
				m_pxDefProc->OnConnectToServer();

			break;
		}
		case FD_CLOSE:
		{
			DisconnectToServer();
			
			break;
		}
		case FD_READ:
		{
            int nRecv=0;
            if (ioctlsocket((SOCKET)wParam,FIONREAD,(u_long*)&nRecv)==SOCKET_ERROR) {
                ConnectionFailed();break;
            }
            if (nRecv<=0) break;
            if (g_bProcState==_GAME_PROC) {
                char* packet=new char[nRecv+1];
                const int received=recv((SOCKET)wParam,packet,nRecv,0);
                if (received<=0) {
                    delete[] packet;
                    if (!received) DisconnectToServer();
                    else if (WSAGetLastError()!=WSAEWOULDBLOCK) ConnectionFailed();
                    break;
                }
                packet[received]='\0';
#ifdef _DEBUG
                _RPT1(_CRT_WARN, "RECV:%s\n", packet);
#endif
                if (!PacketQ.PushQ((BYTE*)packet)) delete[] packet;
            } else {
                nRecv=min(nRecv,int(sizeof(m_szBuff))-m_nCurRecv-1);
                if (nRecv<=0) {DisconnectToServer();break;}
                // recv may return fewer bytes than FIONREAD reported. WSAAsyncSelect
                // schedules another FD_READ for remaining data after this recv.
                const int received=recv((SOCKET)wParam,m_szBuff+m_nCurRecv,nRecv,0);
                if (received<=0) {
                    if (!received) DisconnectToServer();
                    else if (WSAGetLastError()!=WSAEWOULDBLOCK) ConnectionFailed();
                    break;
                }
                m_nCurRecv+=received;
                m_szBuff[m_nCurRecv]='\0';
                if (m_pxDefProc) {
                    char* remaining=m_pxDefProc->OnMessageReceive(m_szBuff);
                    m_nCurRecv=remaining?int(strlen(remaining)):0;
                    if (remaining) memmove(m_szBuff,remaining,m_nCurRecv);
                    m_szBuff[m_nCurRecv]='\0';
                }
            }

			break;
		}
	}

	return 0L;
}

VOID CClientSocket::SendPacket(_TDEFAULTMESSAGE* lpDefMsg, char *pszData, int dataLength)
{
	fnEncodeMessage(lpDefMsg, m_szEncodeDefMsg, sizeof(m_szEncodeDefMsg));

	((m_nSync >= 9) ? m_nSync = 1 : m_nSync++);

	if (pszData)
	{
		fnEncode6BitBuf((unsigned char *)pszData, m_szEncodeBody, dataLength < 0 ? strlen(pszData) : dataLength, sizeof(m_szEncodeBody));
		sprintf(m_szPacket, "#%d%s%s!", m_nSync, m_szEncodeDefMsg, m_szEncodeBody);
	}
	else
		sprintf(m_szPacket, "#%d%s!", m_nSync, m_szEncodeDefMsg);

    const int length=int(strlen(m_szPacket));
    const int sent=send(m_sockClient, m_szPacket, length, 0);
    mir3::ClientRpcPacket(sent==length?"sent":"send_failed",lpDefMsg->wIdent,sent);
}

// **************************************************************************************
//
//		For Login Process
//
// **************************************************************************************

VOID CClientSocket::OnLogin(char *pszID, char *pszPassword)
{
	char				szPacket[64];
	
	fnMakeDefMessage(&DefMsg, CM_IDPASSWORD, 0, 0, 0, 0);
	wsprintf(szPacket, "%s/%s", pszID, pszPassword);

	SendPacket(&DefMsg, szPacket);
}

VOID CClientSocket::OnSelectServer(char *pszServerName)
{
    WORD legacyIndex=0;
    const int count=GetPrivateProfileIntA("Server","ServerCount",0,"./Mir3.ini");
    for(int i=1;i<=count;++i) {
        char key[32],name[256];sprintf(key,"Server%dName",i);
        GetPrivateProfileStringA("Server",key,"",name,sizeof(name),"./Mir3.ini");
        if(strcmp(name,pszServerName)==0) {
            sprintf(key,"Server%dIndex",i);legacyIndex=WORD(GetPrivateProfileIntA("Server",key,i,"./Mir3.ini"));break;
        }
    }
    selectedServerName=pszServerName;
    fnMakeDefMessage(&DefMsg,CM_SELECTSERVER,0,legacyIndex,0,0);
    SendPacket(&DefMsg,pszServerName);
}

VOID CClientSocket::OnChangePassword(char *pszID, char *pszOldPassword, char *pszNewPassword)
{
	char				szPacket[64];
	char				cDivider;

	cDivider = 0x09;
	fnMakeDefMessage(&DefMsg, CM_CHANGEPASSWORD, 0, 0, 0, 0);

	wsprintf(szPacket, "%s%c%s%c%s", pszID, cDivider, pszOldPassword, cDivider, pszNewPassword);

	SendPacket(&DefMsg, szPacket);
}

VOID CClientSocket::OnNewAccount(char *pszNewAccount)
{
	fnMakeDefMessage(&DefMsg, CM_ADDNEWUSER, 0, 0, 0, 0);
	SendPacket(&DefMsg, pszNewAccount);
}

// **************************************************************************************
//
//		For Character Select Process
//
// **************************************************************************************

VOID CClientSocket::OnQueryChar()
{
	char szPacket[32];

	fnMakeDefMessage(&DefMsg, CM_QUERYCHR, 0, 0, 0, 0);

	wsprintf(szPacket, "%s/%d", g_szUserID, g_nCertifyCode);

	SendPacket(&DefMsg, szPacket);
}

VOID CClientSocket::OnNewChar(char *pszId, int nHair, int nJob, int nSex)
{
    // DBSvr decodes a packed 43-byte character/account record, including zeros.
    _TCREATECHR character = {};
    static_assert(sizeof(character) == 43, "Character creation wire layout changed");
    character.Class = (BYTE)nJob;
    character.Gender = (BYTE)nSex;
    character.Level = 1;
    lstrcpynA(character.ChrName, pszId, sizeof(character.ChrName));
    lstrcpynA(character.szID, g_szUserID, sizeof(character.szID));
    fnMakeDefMessage(&DefMsg, CM_NEWCHR, 0, 0, 0, 0);
    SendPacket(&DefMsg, (char*)&character, sizeof(character));
}

VOID CClientSocket::OnDelChar(const char* name,const char* confirmation)
{
    const std::string body=std::string(name)+"/"+confirmation;
    fnMakeDefMessage(&DefMsg,CM_DELCHR,0,0,0,0);
    SendPacket(&DefMsg,const_cast<char*>(body.c_str()));
}


VOID CClientSocket::OnSelChar(char* name)
{
    const std::string body=std::string(g_szUserID)+"/"+name;
    fnMakeDefMessage(&DefMsg,CM_SELCHR,0,0,0,0);
    SendPacket(&DefMsg,const_cast<char*>(body.c_str()));
}



// **************************************************************************************
//
//		For Game Playing
//
// **************************************************************************************

void CClientSocket::SendNoticeOK()
{
	fnMakeDefMessage(&DefMsg, CM_LOGINNOTICEOK, 0, 0, 0, 0);
	SendPacket(&DefMsg, NULL);
}

VOID CClientSocket::SendSay(CHAR *pszSay)
{
	fnMakeDefMessage(&DefMsg, CM_SAY, 0, 0, 0, 0);
	SendPacket(&DefMsg, pszSay);
}

VOID CClientSocket::SendSpellMsg(INT nMagicID, INT nTargetX, INT nTargetY, INT nTargetID)
{
//wParam  : 魔法 ID(wtag)
//lParam1 : 目标X
//lParam2 : 目标Y
//lParam3 : 目标对象 id(wTag)
//lParam4 : ...(wSeries)
	fnMakeDefMessage(&DefMsg, CM_SPELL, MAKELONG(nTargetX, nTargetY), LOWORD(nTargetID), nMagicID, HIWORD(nTargetID));
    // 47CAB0: DWORD tick is formatted as signed decimal before common encoding.
    const DWORD tick=GetTickCount();
    LONG signedTick;memcpy(&signedTick,&tick,sizeof(signedTick));
    char body[16];sprintf(body,"%ld",signedTick);
    SendPacket(&DefMsg,body);
}

VOID CClientSocket::SendOpenDoor(INT nX, INT nY, INT nIdx)
{
	fnMakeDefMessage(&DefMsg, CM_OPENDOOR, nIdx, nX, nY, 0);
	SendPacket(&DefMsg, NULL);
}

void CClientSocket::SendNPCMessage(WORD wIdent, DWORD dwNpcId, INT reg, char *pszMsg)
{
	fnMakeDefMessage(&DefMsg, wIdent, dwNpcId, 0, 0, reg);
	SendPacket(&DefMsg, pszMsg);
}

void CClientSocket::SendGetDetailItem(DWORD dwNpcId, INT nIndex, CHAR* szItemName)
{
	fnMakeDefMessage(&DefMsg, CM_USERGETDETAILITEM, dwNpcId,nIndex,0,0);
	SendPacket(&DefMsg, szItemName);
}

void CClientSocket::SendBuyItem(DWORD dwNpcId, CHAR* szItemName, CHAR* szItemIndex)
{
	fnMakeDefMessage(&DefMsg, CM_USERBUYITEM, dwNpcId, 0, 0, 0);
	SendPacket(&DefMsg, szItemName);
}

// *** For Group
void CClientSocket::SendCreateGroup(char *pszWho)
{
	fnMakeDefMessage(&DefMsg, CM_CREATEGROUP, 0, 0, 0, 0);
	SendPacket(&DefMsg, pszWho);
}

void CClientSocket::SendGroupMode(BOOL fFlag)
{
	if (fFlag)
		fnMakeDefMessage(&DefMsg, CM_GROUPMODE, 0, 1, 0, 0); // On
	else
		fnMakeDefMessage(&DefMsg, CM_GROUPMODE, 0, 0, 0, 0); // Off

	SendPacket(&DefMsg, NULL);
}

void CClientSocket::SendAddGroupMember(char *pszWho)
{
	fnMakeDefMessage(&DefMsg, CM_ADDGROUPMEMBER, 0, 0, 0, 0);
	SendPacket(&DefMsg, pszWho);
}

void CClientSocket::SendDelGroupMember(char *pszWho)
{
	fnMakeDefMessage(&DefMsg, CM_DELGROUPMEMBER, 0, 0, 0, 0);
	SendPacket(&DefMsg, pszWho);
}

// *** For Guild
void CClientSocket::SendGuildHome()
{
	fnMakeDefMessage(&DefMsg, CM_GUILDHOME, 0, 0, 0, 0);
	SendPacket(&DefMsg, NULL);
}

void CClientSocket::SendGuildMemberRemoval(char* name)
{
    fnMakeDefMessage(&DefMsg,CM_GUILDDELMEMBER,0,0,0,0);
    SendPacket(&DefMsg,name);
    fnMakeDefMessage(&DefMsg,CM_GUILDMEMBERLIST,0,0,0,0);
    SendPacket(&DefMsg,NULL);
}

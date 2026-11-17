#ifdef MIR3_EISERVER
#define MIR3_DATABASE_CONFIG ".\\EISERVER.ini"
#else
#define MIR3_DATABASE_CONFIG ".\\database.ini"
#endif
#include "stdafx.h"
#include "dbsvr.h"
#include "globalUserList.h"
#include "../def/dbmgr.h"
#include <stdio.h>
#include "tableList.h"
#include "processdbmsg.h"

void GetLoadHumanRcd(CServerInfo* pServerInfo, _LPTLOADHUMAN lpLoadHuman, int nRecog);

extern CWHList<CServerInfo*>		g_xServerList;
extern CWHList< GAMESERVERINFO * >	g_xGameServerList;

void CGateInfo::ReceiveOpenUser(char *pszPacket)
{
	char	*pszPos;
	int		nSocket;
	int		nLen = memlen(pszPacket);

	if (pszPos = (char *)memchr(pszPacket, '/', nLen))
	{
		nSocket = AnsiStrToVal(pszPacket);

		pszPos++;

		CUserInfo* pUserInfo = new CUserInfo;

		if (pUserInfo)
		{
			MultiByteToWideChar(CP_ACP, 0, pszPacket, -1, pUserInfo->szSockHandle, sizeof(pUserInfo->szSockHandle)/sizeof(TCHAR));
			MultiByteToWideChar(CP_ACP, 0, pszPos, -1, pUserInfo->szAddress, sizeof(pUserInfo->szAddress)/sizeof(TCHAR));

			pUserInfo->sock					= nSocket;
			pUserInfo->nCertification		= 0;
			pUserInfo->nClientVersion		= 0;

			ZeroMemory(pUserInfo->szUserID, sizeof(pUserInfo->szUserID));

			xUserInfoList.AddNewNode(pUserInfo);

			InsertLogMsgParam(IDS_OPEN_USER, pUserInfo->szAddress);
		}
	} 
}

void CGateInfo::ReceiveCloseUser(char *pszPacket)
{
	int nSocket = AnsiStrToVal(pszPacket);
/*
	map<SOCKET, CUserInfo, less<SOCKET> >::iterator it = pGateInfo->UserInfoMap.find((SOCKET)nSocket);

	if (it != pGateInfo->UserInfoMap.end())
	{
		InsertLogMsgParam(IDS_CLOSE_USER, it->second.szAddress);

		pGateInfo->UserInfoMap.erase(it);
	} */
}

void CGateInfo::ReceiveSendUser(char *pszPacket)
{
	char	*pszPos;//, *pszPos2;
	int		nSocket;
	int		nLen = memlen(pszPacket);

	if ((pszPos = (char *)memchr(pszPacket, '/', nLen)))// && (pszPos2 = (char *)memchr(pszPacket, '$', nLen)))
	{
		nSocket = AnsiStrToVal(pszPacket);

		pszPos++;

		_LPTGATESENDBUFF lpSendUserData = new _TGATESENDBUFF;

		lpSendUserData->sock		= (SOCKET)nSocket;

		memmove(lpSendUserData->szData, pszPos, memlen(pszPos));

		m_GateQ.PushQ((BYTE *)lpSendUserData);
	}
}

void CGateInfo::SendToGate(SOCKET cSock, char *pszPacket)
{
	char	szData[256];
	WSABUF	buf;
	DWORD	dwSendBytes;
	
	int nLen = memlen(pszPacket) - 1;

	szData[0] = '%';
	
	char *pszNext = ValToAnsiStr((int)cSock, &szData[1]);
	
	*pszNext++ = '/';
	*pszNext++ = '#';

	memmove(pszNext, pszPacket, nLen);

	pszNext += nLen;

	*pszNext++ = '!';
	*pszNext++ = '$';
	*pszNext++ = '\0';

	buf.len = pszNext - szData;
	buf.buf = szData;

	WSASend(sock, &buf, 1, &dwSendBytes, 0, NULL, NULL);
}

void CGateInfo::QueryCharacter(SOCKET s, char *pszPacket)
{
	_TQUERYCHR			tQueryChr[3];
	char				szDecodeMsg[128];
	int					nCnt = 0;
	char				szQuery[256];

	ZeroMemory(tQueryChr, sizeof(tQueryChr));

	int nPos = fnDecode6BitBufA(pszPacket, szDecodeMsg, sizeof(szDecodeMsg));
	szDecodeMsg[nPos] = '\0';

	char *pszDevide = (char *)memchr(szDecodeMsg, '/', nPos);
	
	if (pszDevide)
	{
		*pszDevide++ = '\0';

		sprintf( szQuery, "SELECT * FROM TBL_CHARACTER WHERE FLD_LOGINID='%s'", szDecodeMsg );

		CRecordset *pRec = GetDBManager()->CreateRecordset();
		
		if (pRec->Execute( szQuery ))
		{
			while (pRec->Fetch() && nCnt < 3)
			{
				tQueryChr[nCnt].btClass	 = atoi( pRec->Get( "FLD_JOB" ) );
				tQueryChr[nCnt].btLevel = atoi(pRec->Get("FLD_LEVEL"));
				tQueryChr[nCnt].btGender = atoi( pRec->Get( "FLD_GENDER" ) );
				strcpy( tQueryChr[nCnt].szName, pRec->Get( "FLD_CHARNAME" ) );
				ChangeSpaceToNull( tQueryChr[nCnt].szName );

				nCnt++;
			}
		}

		GetDBManager()->DestroyRecordset( pRec );

		_TDEFAULTMESSAGE	DefaultMsg;
		char				szEncodeMsg[32];
		char				szEncodeData[256];
		char				szEncodePacket[256];
		
		if (nCnt >= 0 && nCnt <= 3)
		{
			fnMakeDefMessageA(&DefaultMsg, SM_QUERYCHR, 0, nCnt, 0, 0);
			nPos = fnEncodeMessageA(&DefaultMsg, szEncodeMsg, sizeof(szEncodeMsg));
			int nPos2 = fnEncode6BitBufA((unsigned char *)tQueryChr, szEncodeData, sizeof(_TQUERYCHR) * nCnt, sizeof(szEncodeData));
			
			memmove(szEncodePacket, szEncodeMsg, nPos);
			memmove(&szEncodePacket[nPos], szEncodeData, nPos2);
			szEncodePacket[nPos + nPos2] = '\0';
				
			SendToGate(s, szEncodePacket);
		}
		else
		{
			fnMakeDefMessageA(&DefaultMsg, SM_QUERYCHR_FAIL, 0, 0, 0, 0);
			nPos = fnEncodeMessageA(&DefaultMsg, szEncodeMsg, sizeof(szEncodeMsg));
			szEncodeMsg[nPos] = '\0';
			
			SendToGate(s, szEncodeMsg);
		}
	}
}

void CGateInfo::DeleteExistCharacter(SOCKET s, _LPTCREATECHR lpTCreateChr)
{
	_TDEFAULTMESSAGE	DefaultMsg;
	char				szEncodeMsg[32];
	char				szQuery[256];
	CRecordset			*pRec;

	sprintf( szQuery, "DELETE FROM TBL_CHARACTER WHERE FLD_LOGINID='%s' AND FLD_CHARNAME='%s'", lpTCreateChr->szID, lpTCreateChr->szName );

	pRec = GetDBManager()->CreateRecordset();
	pRec->Execute( szQuery );
	GetDBManager()->DestroyRecordset( pRec );

	sprintf( szQuery, "DELETE FROM TBL_CHARACTER_GENITEM WHERE FLD_LOGINID='%s' AND FLD_CHARNAME='%s'", lpTCreateChr->szID, lpTCreateChr->szName );

	pRec = GetDBManager()->CreateRecordset();
	pRec->Execute( szQuery );
	GetDBManager()->DestroyRecordset( pRec );

	sprintf( szQuery, "DELETE FROM TBL_CHARACTER_ITEM WHERE FLD_LOGINID='%s' AND FLD_CHARNAME='%s'", lpTCreateChr->szID, lpTCreateChr->szName );

	pRec = GetDBManager()->CreateRecordset();
	pRec->Execute( szQuery );
	GetDBManager()->DestroyRecordset( pRec );

	sprintf( szQuery, "DELETE FROM TBL_CHARACTER_MAGIC WHERE FLD_LOGINID='%s' AND FLD_CHARNAME='%s'", lpTCreateChr->szID, lpTCreateChr->szName );

	pRec = GetDBManager()->CreateRecordset();
	pRec->Execute( szQuery );
	GetDBManager()->DestroyRecordset( pRec );

	fnMakeDefMessageA(&DefaultMsg, SM_DELCHR_SUCCESS, 0, 4, 0, 0);
	int nPos = fnEncodeMessageA(&DefaultMsg, szEncodeMsg, sizeof(szEncodeMsg));
	szEncodeMsg[nPos] = '\0';
	
	SendToGate(s, szEncodeMsg);
}

void CGateInfo::MakeNewCharacter(SOCKET s, _LPTCREATECHR lpTCreateChr)
{
	//ERROR: 1=> Exist Charname, 2=>Wrong Name, 3=>Not enough Space, 4=>Error
	_TDEFAULTMESSAGE	DefaultMsg;
	char				szEncodeMsg[32];
	int					nPos;
	char				szQuery[2048];

	sprintf( szQuery, "SELECT FLD_CHARNAME FROM TBL_CHARACTER WHERE FLD_CHARNAME='%s'", lpTCreateChr->szName );

	CRecordset *pRec = GetDBManager()->CreateRecordset();

	pRec->Execute( szQuery );

	if (pRec->Fetch())
	{
		fnMakeDefMessageA(&DefaultMsg, SM_NEWCHR_FAIL, 0, 1, 0, 0);
		nPos = fnEncodeMessageA(&DefaultMsg, szEncodeMsg, sizeof(szEncodeMsg));
		szEncodeMsg[nPos] = '\0';
		
		SendToGate(s, szEncodeMsg);

		GetDBManager()->DestroyRecordset( pRec );

		return;
	}
	
	GetDBManager()->DestroyRecordset( pRec );

	sprintf( szQuery, "SELECT COUNT(FLD_CHARNAME) AS FLD_COUNT FROM TBL_CHARACTER WHERE FLD_LOGINID='%s'", lpTCreateChr->szID );

	pRec = GetDBManager()->CreateRecordset();

	if (pRec->Execute( szQuery ) && pRec->Fetch() )
	{
		if (atoi(pRec->Get( "FLD_COUNT" )) >= 3)
		{
			fnMakeDefMessageA(&DefaultMsg, SM_NEWCHR_FAIL, 0, 3, 0, 0);
			nPos = fnEncodeMessageA(&DefaultMsg, szEncodeMsg, sizeof(szEncodeMsg));
			szEncodeMsg[nPos] = '\0';
			
			SendToGate(s, szEncodeMsg);

			GetDBManager()->DestroyRecordset( pRec );

			return;
		}

		GetDBManager()->DestroyRecordset( pRec );

		char startMap[15] = {};
        GetPrivateProfileStringA("Game", "StartMap", "4", startMap, sizeof(startMap), MIR3_DATABASE_CONFIG);
        CTblStartPoint::TABLE *table = GetTblStartPoint()->Get(startMap);
        if (!table) {
            fnMakeDefMessageA(&DefaultMsg, SM_NEWCHR_FAIL, 0, 4, 0, 0);
            nPos = fnEncodeMessageA(&DefaultMsg, szEncodeMsg, sizeof(szEncodeMsg));
            szEncodeMsg[nPos] = '\0';
            SendToGate(s, szEncodeMsg);
            return;
        }

		pRec = GetDBManager()->CreateRecordset();

		// 添加 TBL_CHARACTER 表

		sprintf(szQuery, "INSERT INTO TBL_CHARACTER ("
						"FLD_LOGINID, FLD_CHARNAME, FLD_JOB, FLD_GENDER, FLD_LEVEL, FLD_DIRECTION, "
						"FLD_ATTACKMODE, FLD_CX, FLD_CY, FLD_MAPNAME, FLD_GOLD, FLD_HAIR, "
						"FLD_DRESS_ID, FLD_WEAPON_ID, FLD_LEFTHAND_ID, FLD_RIGHTHAND_ID, FLD_HELMET_ID, "
						"FLD_NECKLACE_ID, FLD_ARMRINGL_ID, FLD_ARMRINGR_ID, FLD_RINGL_ID, "
						"FLD_RINGR_ID, FLD_EXP) VALUES ( "
						"'%s', '%s', %d, %d, 1, 4, "
						"1, %d, %d, '%s', 0, 0, "
						"'0', '0', '0', '0', '0', "
						"'0', '0', '0', '0', "
						"'0', 0 )",
						lpTCreateChr->szID, lpTCreateChr->szName, lpTCreateChr->btClass, lpTCreateChr->btGender,
						table->posX, table->posY, table->mapName);
		pRec->Execute( szQuery );

		sprintf(szQuery, "INSERT INTO TBL_CHARACTER_GENITEM (FLD_LOGINID, FLD_CHARNAME, FLD_ITEMINDEX) VALUES ('%s', '%s', 'G00080008000')",
							lpTCreateChr->szID, lpTCreateChr->szName);
		pRec->Execute( szQuery );
		
		GetDBManager()->DestroyRecordset( pRec );
		
		_TLOADHUMAN		human;
		_TMAKEITEMRCD	makeItem;
		memset( &human, 0, sizeof( human ) );
		memset( &makeItem, 0, sizeof( makeItem ) );

		strcpy( human.szUserID, lpTCreateChr->szID );
		strcpy( human.szCharName, lpTCreateChr->szName );

		// 添加便服 (0: 男, 1: 女)
		makeItem.szStdType	= 'B';
		makeItem.nStdIndex	= lpTCreateChr->btGender ? 34 : 33;
		makeItem.nDura		= 5000;
		makeItem.nDuraMax	= 5000;
		MakeNewItem( NULL, &human, &makeItem, 0 );

		// 添加木剑
		makeItem.szStdType	= 'A';
		makeItem.nStdIndex	= 7;
		makeItem.nDura		= 4000;
		makeItem.nDuraMax	= 4000;
		MakeNewItem( NULL, &human, &makeItem, 0 );
		
		fnMakeDefMessageA(&DefaultMsg, SM_NEWCHR_SUCCESS, 0, 0, 0, 0);
		nPos = fnEncodeMessageA(&DefaultMsg, szEncodeMsg, sizeof(szEncodeMsg));
		szEncodeMsg[nPos] = '\0';
		
		SendToGate(s, szEncodeMsg);

		return;
	}

	fnMakeDefMessageA(&DefaultMsg, SM_NEWCHR_FAIL, 0, 4, 0, 0);
	nPos = fnEncodeMessageA(&DefaultMsg, szEncodeMsg, sizeof(szEncodeMsg));
	szEncodeMsg[nPos] = '\0';
	
	SendToGate(s, szEncodeMsg);

	GetDBManager()->DestroyRecordset( pRec );
}

void CGateInfo::GetSelectCharacter(SOCKET s, char *pszPacket)
{
    ServerConnectionReadLock connectionLock;
	char				szDecodeMsg[128];
	char				szServerIP[32];
	char				szEncodeMsg[32];
	char				szEncodeData[64];
	char				szEncodePacket[256];
	_TDEFAULTMESSAGE	DefaultMsg;
	
	// ORZ: Load Balancing, 选择连接数最少的网关服务器 IP
	GAMESERVERINFO *pBestServer = NULL;
	GAMESERVERINFO *pTemp;

//	EnterCriticalSection( &g_xGameServerList.m_cs );
	for ( PLISTNODE pNode = g_xGameServerList.GetHead();pNode; pNode = g_xGameServerList.GetNext( pNode ) )
	{
		pTemp = g_xGameServerList.GetData( pNode );
		
		if ( !pBestServer || pTemp->connCnt < pBestServer->connCnt )
		{
			pBestServer = pTemp;
			continue;
		}
	}

    if (!pBestServer || !g_xServerList.GetHead()) {
        fnMakeDefMessageA(&DefaultMsg, SM_STARTFAIL, 0, 0, 0, 0);
        int length = fnEncodeMessageA(&DefaultMsg, szEncodeMsg, sizeof(szEncodeMsg));
        szEncodeMsg[length] = 0;
        SendToGate(s, szEncodeMsg);
        return;
    }
	pBestServer->connCnt++;
//	LeaveCriticalSection( &g_xGameServerList.m_cs );

	strcpy( szServerIP, pBestServer->ip );
	// ORZ: from here

	int nPos = fnDecode6BitBufA(pszPacket, szDecodeMsg, sizeof(szDecodeMsg));
	szDecodeMsg[nPos] = '\0';

	char *pszDevide = (char *)memchr(szDecodeMsg, '/', nPos);
	
	if (pszDevide)
	{
		*pszDevide++ = '\0';

		// 修改为可选择服务器
		_TLOADHUMAN		tLoadHuman = {};
		CServerInfo*	pServerInfo = NULL;

		memcpy(tLoadHuman.szUserID, szDecodeMsg, memlen(szDecodeMsg));
		memcpy(tLoadHuman.szCharName, pszDevide, memlen(pszDevide));
		ZeroMemory(tLoadHuman.szUserAddr, sizeof(tLoadHuman.szUserAddr));
		tLoadHuman.nCertification = 0;

		PLISTNODE pListNode = g_xServerList.GetHead();

		if (pListNode)
			pServerInfo = g_xServerList.GetData(pListNode);

		GetLoadHumanRcd(pServerInfo, &tLoadHuman, 0);

		fnMakeDefMessageA(&DefaultMsg, SM_STARTPLAY, 0, 0, 0, 0);
		nPos = fnEncodeMessageA(&DefaultMsg, szEncodeMsg, sizeof(szEncodeMsg));
		int nPos2 = fnEncode6BitBufA((unsigned char *)szServerIP, szEncodeData, memlen(szServerIP) -1, sizeof(szEncodeData));
		
		memmove(szEncodePacket, szEncodeMsg, nPos);
		memmove(&szEncodePacket[nPos], szEncodeData, nPos2);
		szEncodePacket[nPos + nPos2] = '\0';
		
		// ORZ: 添加到全部列表中.
		// 可能因相同账号ID已存在或内存不足等原因而失败。
//		if ( GetGlobalUserList()->Insert( tLoadHuman.szCharName, szServerIP ) )
			SendToGate(s, szEncodePacket);
//		else
//		{
//			fnMakeDefMessageA(&DefaultMsg, SM_STARTFAIL, 0, 0, 0, 0);
//			nPos = fnEncodeMessageA(&DefaultMsg, szEncodeMsg, sizeof(szEncodeMsg));
//			szEncodeMsg[nPos] = '\0';
			
//			SendToGate(pGateInfo->sock, s, szEncodeMsg);
//		}
	}
}


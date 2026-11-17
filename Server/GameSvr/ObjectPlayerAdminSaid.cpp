#include "stdafx.h"

#define _MAX_ADMINCOMMAND_LIST	(IDS_COMMAND_HAIRSTYLE - IDS_COMMAND_MONGEN + 1)

void InitAdminCommandList()
{
	TCHAR	szCommand[64];
	int		nLen;

	HINSTANCE hInstance = GetModuleHandle(NULL);

	if (hInstance)
	{
		for (int i = 0; i < _MAX_ADMINCOMMAND_LIST; i++)
		{
			LoadAdminString(hInstance, IDS_COMMAND_MONGEN + i, szCommand, sizeof(szCommand)/sizeof(TCHAR));

			nLen = lstrlen(szCommand) * sizeof(TCHAR) + 1;

			char *pszAdminCommand = new char[nLen];

			if (pszAdminCommand)
			{
				WideCharToMultiByte(CP_ACP, 0, szCommand, -1, pszAdminCommand, nLen, NULL, NULL);
				g_xAdminCommandList.AddNewNode(pszAdminCommand);
			}
		}

		InsertLogMsgParam(IDS_LOAD_ADMINCOMMAND, &i, LOGPARAM_INT);


	}
}

void UnInitAdminCommandList()
{
	PLISTNODE pListNode = g_xAdminCommandList.GetHead();

	while (pListNode)
	{
		delete [] g_xAdminCommandList.GetData(pListNode);

		pListNode = g_xAdminCommandList.RemoveNode(pListNode);
	}
}

BOOL CPlayerObject::ProcessForAdminSaid(char *pszMsg)
{
	if (pszMsg[0] == '@')
	{
		PLISTNODE	pListNode	= g_xAdminCommandList.GetHead();
		int			nCommand	= 0;

		char		*pszParam1 = NULL, *pszParam2 = NULL, *pszParam3 = NULL, *pszParam4 = NULL;

		if ((pszParam1 = strchr(&pszMsg[1], ' ')))
		{
			*pszParam1 = '\0';
			pszParam1++;
		}

		char *pszData;

		while (pListNode)
		{
			pszData = (char *)g_xAdminCommandList.GetData(pListNode);

			if (memcmp(&pszMsg[1], pszData, memlen(pszData)) == 0)
				break;

			nCommand++;

			pListNode = g_xAdminCommandList.GetNext(pListNode);
		}

		if (nCommand >= _MAX_ADMINCOMMAND_LIST) return FALSE;

		if (pszParam1)
		{
			if (pszParam2 = strchr(pszParam1, ' '))
			{
				*pszParam2 = '\0';
				pszParam2++;

				if (pszParam3 = strchr(pszParam2, ' '))
				{
					*pszParam3 = '\0';
					pszParam3++;

					if (pszParam4 = strchr(pszParam3, ' '))
					{
						*pszParam4 = '\0';
						pszParam4++;
					}
				}
			}
		}

		switch (IDS_COMMAND_MONGEN + nCommand)
		{
			case IDS_COMMAND_MONGEN:						//"怪物刷新"
				if (pszParam1 && pszParam2)
					CmdCallMakeMonster(pszParam1, pszParam2);
				break;
			case IDS_COMMAND_INSPECTOR:						//"观察者"
			{
				TCHAR	wszMsg[64];
				char	szMsg[64];

				m_fInspector = ~m_fInspector;

				if (m_fInspector)
					LoadString(g_hInst, IDS_INSPECTOR_ON, wszMsg, sizeof(wszMsg)/sizeof(TCHAR));
				else
					LoadString(g_hInst, IDS_INSPECTOR_OFF, wszMsg, sizeof(wszMsg)/sizeof(TCHAR));

				WideCharToMultiByte(CP_ACP, 0, wszMsg, -1, szMsg, sizeof(szMsg), NULL, NULL);
				SysMsg(szMsg, 1);

				break;
			}
			case IDS_COMMAND_ADMIN:							//"管理员"
				break;
			case IDS_COMMAND_INVINCIBILITY:					//"无敌"
			{
				TCHAR	wszMsg[64];
				char	szMsg[64];

				m_fIsNeverDie = ~m_fIsNeverDie;

				if (m_fIsNeverDie)
					LoadString(g_hInst, IDS_NEVERDIE_ON, wszMsg, sizeof(wszMsg)/sizeof(TCHAR));
				else
					LoadString(g_hInst, IDS_NEVERDIE_OFF, wszMsg, sizeof(wszMsg)/sizeof(TCHAR));

				WideCharToMultiByte(CP_ACP, 0, wszMsg, -1, szMsg, sizeof(szMsg), NULL, NULL);
				SysMsg(szMsg, 1);

				break;
			}
			case IDS_COMMAND_ATTACK:						//"攻击"
				break;
			case IDS_COMMAND_MOVE:							//"移动"
				if (pszParam1)
					CmdRandomSpaceMove(pszParam1);
				break;
			case IDS_COMMAND_CHANGELEVEL:					//"等级调整"
			{
				if (pszParam1)
				{
					m_Ability.Level = m_pUserInfo->m_THumanRcd.szLevel = _MIN(40, AnsiStrToVal(pszParam1));				
					HasLevelUp(1);
				}
				break;
			}
			case IDS_COMMAND_MOVEFREESTYLE:					//"自由移动"
				if (pszParam1 && pszParam2 && pszParam3)
					CmdFreeSpaceMove(pszParam1, pszParam2, pszParam3);
				break;
			case IDS_COMMAND_MAP:							//"地图"
			{
				char szMsg[64];

				memcpy(szMsg, "MAP : ", 6);
				memcpy(&szMsg[6], m_pMap->m_szMapTextName, memlen(m_pMap->m_szMapTextName));

				SysMsg(szMsg, 0);
				
				break;
			}
			case IDS_COMMAND_LEVEL:							//"等级"
				break;
			case IDS_COMMAND_MONLEVEL:						//"怪物等级"
				CmdSendMonsterLevelInfos();
				break;
			case IDS_COMMAND_RECALL:						//"召唤"
				break;
			case IDS_COMMAND_NUMOFMON:						//"怪物数量"
				break;
			case IDS_COMMAND_NUMOFHUMAN:					//"人数"
				break;
			case IDS_COMMAND_SABUKMONEY:					//"沙北城钱"
				break;
			case IDS_COMMAND_KICK:							//"Kick"
				break;
			case IDS_COMMAND_KICK2:							//"踢"
				break;
			case IDS_COMMAND_TING:							//"叮"
				break;
			case IDS_COMMAND_KINGTING:						//"王廷"
				break;
			case IDS_COMMAND_LUCKYVALUE:					//"幸运值"
				break;
			case IDS_COMMANT_SHUTUP:						//"采金"
				break;
			case IDS_COMMAND_TALK:							//"采矿解除"
				break;
			case IDS_COMMAND_SHUTUPMAN:						//"采金者"
				break;
			case IDS_COMMAND_RELOADADMIN:					//"ReloadAdmin"
				break;
			case IDS_COMMAND_RELOADABUSIVE:					//"辱骂信息读取"
				break;
			case IDS_COMMAND_MOOTAEBO:						//"武太保"
				break;
			case IDS_COMMAND_BACKWALK:						//"后退"
				break;
			case IDS_COMMAND_GAIN:							//"生产"
				if (pszParam1 && pszParam2)
					CmdMakeItem(pszParam1, pszParam2);
				break;
			case IDS_COMMAND_DELGOLD:						//"金钱删除"
				break;
			case IDS_COMMAND_ADDGOLD:						//"增加金钱"
				break;
			case IDS_COMMAND_GOOD:							//"物品"
				break;
			case IDS_COMMAND_OTHERDELGOLD:					//"删除他人金钱"
				break;
			case IDS_COMMAND_OTHERADDGOLD:					//"添加他人金钱"
				break;
			case IDS_COMMAND_OTHERLEVELUP:					//"他人等级调整"
				break;
			case IDS_COMMAND_OTHEREXPUP:					//"他人经验值调整"
				break;
			case IDS_COMMAND_WEAPONDUR:						//"武器耐久调整"
				break;
			case IDS_COMMAND_PARDON:						//"免罪"
				break;
			case IDS_COMMAND_PKPOINT:						//"PK分数"
				break;
			case IDS_COMMAND_PKPOINTUP:						//"PK分数增加"
				break;
			case IDS_COMMAND_CHANGELUCKY:					//"幸运变更"
				break;
			case IDS_COMMAND_SKILLUP:						//"武功修炼"
				if (pszParam1 && pszParam2)
					CmdMakeFullSkill(pszParam1, pszParam2);
				break;
			case IDS_COMMAND_OTHERSKILLUP:					//"他人武功修炼"
				break;
			case IDS_COMMAND_DELSKILL:						//"删除武功"
				break;
			case IDS_COMMAND_OTHERDELSKILL:					//"删除他人武功"
				break;
			case IDS_COMMAND_CHGJOB:						//"转职"
			{
				if (pszParam1)
				{
					if (CmdChangeJob(pszParam1))
						HasLevelUp(1);
				}
			
				break;
			}
			case IDS_COMMAND_CHGGENDER:						//"跨性别者"
			{
				if (m_pUserInfo->m_THumanRcd.btGender == 0) m_pUserInfo->m_THumanRcd.btGender = m_tFeature.btGender = 1;
				else m_pUserInfo->m_THumanRcd.btGender = m_tFeature.btGender = 0;

				SysMsg(pszData, 1);

				break;
			}
			case IDS_COMMAND_COLOR:							//"颜色"
				break;
			case IDS_COMMAND_MONRECALL:						//"怪物召唤"
			{
				if (pszParam1 && pszParam2)
					CmdCallMakeSlaveMonster(pszParam1, pszParam2);

				break;
			}
			case IDS_COMMAND_MISSION:						//"任务"
				break;
			case IDS_COMMAND_GENPOS:						//"刷新点"
			{
				if (pszParam1 && pszParam2 && pszParam3 && pszParam4)
					CmdCallMakeMonsterXY(pszParam1, pszParam2, pszParam3, pszParam4);
				break;
			}
			case IDS_COMMAND_TRANSPARENCY:					//"透明"
				break;
			case IDS_COMMAND_CIVILWAR:						//"强制攻城战"
				break;
			case IDS_COMMAND_DYEINGHAIR:
				if (pszParam1)
					CmdDyeingHair(pszParam1);
				break;
			case IDS_COMMAND_DYEINGWEAR:
				if (pszParam1)
					CmdDyeingWear(pszParam1);
				break;
			case IDS_COMMAND_HAIRSTYLE:
				if (pszParam1)
					CmdHairStyle(pszParam1);
				break;
			case IDS_COMMAND_RESERVED3:						// 物品名称
				if (pszParam1 && pszParam2)
					CmdChangeItemPrefix(pszParam1, pszParam2);
				break;
			case IDS_COMMAND_RESERVED4:						// 仓库存取
			{
				TCHAR	wszMsg[64];
				char	szMsg[64];

				m_fIsCapture = ~m_fIsCapture;

				if (m_fIsCapture)
				{
					LoadString(g_hInst, IDS_CAPTURE_ON, wszMsg, sizeof(wszMsg)/sizeof(TCHAR));

					m_hCaptureFile = fopen("capture.txt", "w+");
				}
				else
				{
					LoadString(g_hInst, IDS_CAPTURE_OFF, wszMsg, sizeof(wszMsg)/sizeof(TCHAR));
				
					fclose(m_hCaptureFile);
				}

				WideCharToMultiByte(CP_ACP, 0, wszMsg, -1, szMsg, sizeof(szMsg), NULL, NULL);
				SysMsg(szMsg, 1);

				break;
			}
			case IDS_COMMAND_RESERVED5:
			{
				int nX, nY;
				CScripterObject* pScripter = new CScripterObject;

				if (pScripter)
				{
					GetFrontPosition(nX, nY);

					pScripter->Create(nX, nY, m_pMap);

					g_xScripterList.AddNewNode(pScripter);
				}

				break;
			}
		}
	}

	return FALSE;
}

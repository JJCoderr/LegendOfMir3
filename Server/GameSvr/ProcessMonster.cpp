#include "stdafx.h"

CMirMap*					GetMap(char *pszMapName);

CMonsterObject* AddCreature(CMirMap* pMap, int nX, int nY, int nMonRace, BOOL fSearch)
{
	CMonsterObject* pMonsterObject = NULL;

    const auto& race = g_pMonRaceInfo[nMonRace];
    if (race.wRaceIndex == LightningZombieRace && race.nAppear == LightningZombieShape)
        pMonsterObject = new CLightingZombiObject;
    else switch (race.wRaceIndex)
	{
		case _ANIMAL_DEER:
			pMonsterObject = (CMonsterObject*)new CAnimalObject; break;
		case _MONSTER_STICK:		// 食人草
			pMonsterObject = (CMonsterObject*)new CStickObject; break;
		case _MONSTER_SANDMOB:		// 石匠人
			pMonsterObject = (CMonsterObject*)new CSandMobObject; break;
		case _MONSTER_RON:			// 罗恩
			pMonsterObject = (CMonsterObject*)new CRonObject; break;
		case ZumaStatueRace:
		case _MONSTER_SCULTURE1:	// 走马神掌、走马护法
		case _MONSTER_SCULTURE2:
			pMonsterObject = (CMonsterObject*)new CScultureObject; break;
		case _MONSTER_BEEQUEEN:		// 飞膜圆虫
			pMonsterObject = (CMonsterObject*)new CBeeQueen; break;
		case ZumaLordRace:
		case _MONSTER_SCULTUREKING: // 走马王
			pMonsterObject = (CMonsterObject*)new CScultureKingObject; break;
		case _MONSTER_DUALAXE:		// 双斧骷髅
			pMonsterObject = (CMonsterObject*)new CDualAxeObject; break;
		case _MONSTER_MAGUNGSA:		// 魔弓士
			pMonsterObject = (CMonsterObject*)new CDualAxeObject(6); break;
		case _MONSTER_THONEDARK:	// 暗黑
			pMonsterObject = (CMonsterObject*)new CDualAxeObject(3); break;
		case _MONSTER_LIGHTZOMBI:
			pMonsterObject = (CMonsterObject*)new CLightingZombiObject; break;
		case _MONSTER_ROCKMAN:		// 沙魁
			pMonsterObject = (CMonsterObject*)new CRockManObject; break;
		case _MONSTER_MINORNUMA:
			pMonsterObject = (CMonsterObject*)new CMinorNumaObject; break;
		case _MONSTER_WHITESKELETON:	// 白骨
			pMonsterObject = (CMonsterObject*)new CWhiteSkeleton; break;
		case _MONSTER_ELFMONSTER:		// 神兽变身之前 
			pMonsterObject = (CMonsterObject*)new CElfMonster; break;			
		case _MONSTER_ELFWARRIOR:		// 神兽变身之后 
			pMonsterObject = (CMonsterObject*)new CElfWarriorMonster; break;
		case _MONSTER_SPITSPIDER:		// 唾液蜘蛛
			pMonsterObject = (CMonsterObject*)new CSpitSpiderMonster; break;
		case _MONSTER_OMA:
		case _MONSTER_OMAKNIGHT:
		case _MONSTER_OMAWARRIOR:
			pMonsterObject = (CMonsterObject*)new COmaObject; break;
		default:
			pMonsterObject = (CMonsterObject*)new CGenneralMonObject;
//			pMonsterObject = (CMonsterObject*)new CChickenObject;
			break;
	}

	if (pMonsterObject)
	{
		if (fSearch)
			pMonsterObject->GetAvailablePosition(pMap, nX, nY, 10);

		pMonsterObject->m_nCurrX			= nX;
		pMonsterObject->m_nCurrY			= nY;
		pMonsterObject->m_nDirection		= rand() % 8;
		pMonsterObject->m_pMap				= pMap;
		pMonsterObject->m_nRace				= nMonRace;

		pMonsterObject->Initialize();

		pMap->AddNewObject(nX, nY, OS_MOVINGOBJECT, pMonsterObject);

		if (!pMonsterObject->m_fHideMode)
			pMonsterObject->AddRefMsg(RM_TURN, pMonsterObject->m_nDirection, nX, nY, 0, pMonsterObject->m_szName);

		return pMonsterObject;
	}

	return NULL;
}

BOOL RegenMonster(CMonsterGenInfo* pMonGenInfo, int nCount)
{
	DWORD dwStart	= GetTickCount();
    CMirMap* map=GetMap(pMonGenInfo->szMapName);
    if(!map || pMonGenInfo->nMonIndex<0 || pMonGenInfo->nMonIndex>=g_nNumOfMonRaceInfo) return FALSE;

	for (int i = 0; i < nCount; i++)
	{
		int nX = pMonGenInfo->nX - pMonGenInfo->nAreaX + (rand() % (pMonGenInfo->nAreaX * 2 + 1));
		int nY = pMonGenInfo->nY - pMonGenInfo->nAreaY + (rand() % (pMonGenInfo->nAreaY * 2 + 1));

		if (CMonsterObject* pMonsterObject = AddCreature(map, nX, nY, pMonGenInfo->nMonIndex, TRUE))
			pMonGenInfo->xMonsterObjList.AddNewNode(pMonsterObject);

		if (GetTickCount() - dwStart > 3000/*ZenLimitTime*/)
			break;
	}

	return TRUE;
}

DWORD GetZenTime(DWORD dwZenTime)
{
	if (dwZenTime < 30 * 60 * 1000)
	{
		return dwZenTime;
/*      if ztime < 30 * 60 * 1000 then begin
         r := (GetUserCount - UserFullCount) / ZenFastStep;  //每增加 200 人就把怪物刷新量提高 10%
         if r > 0 then begin
            if r > 6 then r := 6;
            Result := ztime - Round ((ztime/10) * r);
         end else
            Result := ztime;
      end else
         Result := ztime; */
	}

	return dwZenTime;
}

UINT WINAPI ProcessMonster(LPVOID lpParameter)
{
	int		nCurrMonGen		= 0;
	DWORD	dwCurrentTick	= GetTickCount();
    DWORD dwLastGenTick=dwCurrentTick;
    constexpr DWORD GenerationIntervalMs=200;

	while (TRUE)
	{
		if (g_fTerminated) 
		{
			return 0L;
		}

		// Generate configured monsters.
		if (g_nNumOfMonGenInfo > 0 && GetTickCount() - dwLastGenTick > GenerationIntervalMs)
		{
			dwLastGenTick = GetTickCount();

			if (nCurrMonGen < g_nNumOfMonGenInfo - 1) 
				nCurrMonGen++;
			else
				nCurrMonGen = 0;

			CMonsterGenInfo* pMonGenInfo = &g_pMonGenInfo[nCurrMonGen];

			if (pMonGenInfo)
			{
				if ((pMonGenInfo->dwStartTime == 0) || ((dwLastGenTick - pMonGenInfo->dwStartTime) > GetZenTime(pMonGenInfo->dwZenTime)))
				{
                    int living = 0;
                    auto& monsters = pMonGenInfo->xMonsterObjList;
                    for (auto node = monsters.GetHead(); node; node = monsters.GetNext(node)) {
                        const auto monster = monsters.GetData(node);
                        if (monster && !monster->m_fIsDead && !monster->m_fIsGhost) ++living;
                    }
                    if (pMonGenInfo->nCount > living)
                    {
                        if (RegenMonster(pMonGenInfo, pMonGenInfo->nCount - living))
							pMonGenInfo->dwStartTime = GetTickCount();
					}
				}
			}
		}

		// Execute Monster A.I
		// 刷怪组有上千条，整轮遍历会让单次循环远超 250ms 的行动间隔，
		// 多数怪物几秒才轮到一次。改成每轮只处理固定数量的组，用游标轮转。
		static int nAiCursor = 0;
		constexpr int AiGroupsPerPass = 128;
		const int nAiGroups = (g_nNumOfMonGenInfo > 0) ? (g_nNumOfMonGenInfo + 1) : 1;
		for (int nAiStep = 0; nAiStep < AiGroupsPerPass && nAiStep < nAiGroups; nAiStep++)
		{
			const int i = nAiCursor % nAiGroups;
			++nAiCursor;
			CMonsterGenInfo* pMonGenInfo = &g_pMonGenInfo[i];

			if (pMonGenInfo)
			{
				if (pMonGenInfo->xMonsterObjList.GetCount())
				{
					PLISTNODE pListNode = pMonGenInfo->xMonsterObjList.GetHead();

					while (pListNode)
					{
						CMonsterObject* pMonsterObject = pMonGenInfo->xMonsterObjList.GetData(pListNode);

						if (pMonsterObject)
						{
							dwCurrentTick = GetTickCount();

							if (!pMonsterObject->m_fIsGhost)
							{
								if (dwCurrentTick - pMonsterObject->m_dwRunTime >= pMonsterObject->m_dwRunNextTick)
								{
									pMonsterObject->m_dwRunTime = dwCurrentTick;

									if (dwCurrentTick - pMonsterObject->m_dwSearchTime >= pMonsterObject->m_dwSearchTick)
									{
										pMonsterObject->m_dwSearchTime = dwCurrentTick;
										pMonsterObject->SearchViewRange();
									}

									pMonsterObject->Operate();
								}
							}
							else
							{
								if (GetTickCount() - pMonsterObject->m_dwGhostTime > 3 * 60 * 1000)
								{
									delete pMonsterObject;
									pMonsterObject = NULL;

									pListNode = pMonGenInfo->xMonsterObjList.RemoveNode(pListNode);
									continue;
								}
							}
						}

						pListNode = pMonGenInfo->xMonsterObjList.GetNext(pListNode);
					} // while (pListNode)
				}
			} // if (pMonGenInfo)
		}

		SleepEx(1, TRUE);
	}

	return 0L;
}

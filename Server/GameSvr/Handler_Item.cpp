#include "stdafx.h"

int CItem::GetUpgrade(int nCount, int nRandom)
{
	int nResult = 0;

	for (int i = 0; i < nCount; i++)
	{
		if ((rand() % nRandom) == 0)
			nResult++;
		else
			break;
	}

	return nResult;
}

void CStdItem::GetStandardItem(_LPTCLIENTITEMRCD lpClientItemRcd)
{
	strcpy(lpClientItemRcd->tStdItem.szName, szName);

	lpClientItemRcd->tStdItem.btStdMode		= (BYTE)wStdMode;
	lpClientItemRcd->tStdItem.btShape		= (BYTE)wShape;
	lpClientItemRcd->tStdItem.btWeight		= (BYTE)wWeight;
	lpClientItemRcd->tStdItem.btAniCount	= 0;
	lpClientItemRcd->tStdItem.btSource		= 0;
	lpClientItemRcd->tStdItem.wLooks		= (WORD)dwLooks;
	lpClientItemRcd->tStdItem.wDuraMax		= (WORD)wDuraMax;
	lpClientItemRcd->tStdItem.btNeed		= 0;
	lpClientItemRcd->tStdItem.btNeedLevel	= 0;
	lpClientItemRcd->tStdItem.nPrice		= (UINT)dwPrice;
}

CStdItemSpecial::CStdItemSpecial()
{
}

void CStdItemSpecial::GetStandardItem(_LPTCLIENTITEMRCD lpClientItemRcd)
{
	memcpy(lpClientItemRcd->tStdItem.szName, szName, memlen(szName));

	lpClientItemRcd->tStdItem.btStdMode			= (BYTE)wStdMode;
	lpClientItemRcd->tStdItem.btShape			= (BYTE)wShape;
	lpClientItemRcd->tStdItem.btWeight			= (BYTE)wWeight;
	lpClientItemRcd->tStdItem.btAniCount		= (BYTE)wAniCount;
	lpClientItemRcd->tStdItem.btSource			= (BYTE)wSource;
//	lpClientItemRcd->tStdItem.btNeedIdentify	= (BYTE)g_pStdItemWeapon[nIdx].wNeedIdentify;
	lpClientItemRcd->tStdItem.wLooks			= (WORD)dwLooks;
	lpClientItemRcd->tStdItem.wDuraMax			= (WORD)wDuraMax;
	lpClientItemRcd->tStdItem.btNeed			= (BYTE)wNeed;
	lpClientItemRcd->tStdItem.btNeedLevel		= (BYTE)wNeedLevel;
	lpClientItemRcd->tStdItem.nPrice			= (UINT)dwPrice;
}

void CStdItemSpecial::GetUpgradeStdItem(_LPTCLIENTITEMRCD lpClientItemRcd, _LPTUSERITEMRCD lpUserItemRcd)
{
/*	switch (lpUserItemRcd->szMakeIndex[0])
	{
		case 'A': // Weapon
		{
			lpClientItemRcd->tStdItem.wDC		= MAKEWORD(g_pStdItemWeapon[nIdx].wDC, g_pStdItemWeapon[nIdx].wDC2 + lpUserItemRcd->btValue[0]);
			lpClientItemRcd->tStdItem.wMC		= MAKEWORD(g_pStdItemWeapon[nIdx].wMC, g_pStdItemWeapon[nIdx].wMC2 + lpUserItemRcd->btValue[1]);
			lpClientItemRcd->tStdItem.wSC		= MAKEWORD(g_pStdItemWeapon[nIdx].wSC, g_pStdItemWeapon[nIdx].wSC2 + lpUserItemRcd->btValue[2]);										//3:幸运, 4:诅咒
			lpClientItemRcd->tStdItem.wAC		= MAKEWORD(g_pStdItemWeapon[nIdx].wAC + lpUserItemRcd->btValue[3], g_pStdItemWeapon[nIdx].wAC2 + lpUserItemRcd->btValue[5]);			//准确
			lpClientItemRcd->tStdItem.wMAC		= MAKEWORD(g_pStdItemWeapon[nIdx].wMAC + lpUserItemRcd->btValue[4], g_pStdItemWeapon[nIdx].wMAC2 + lpUserItemRcd->btValue[6]);		//攻击速度(-/+)
			lpClientItemRcd->tStdItem.btSource	= (BYTE)lpUserItemRcd->btValue[7];

			if (lpUserItemRcd->btValue[10]) lpClientItemRcd->tStdItem.btNeedIdentify = 0x01;

			break;
		}
		case 'B': // Armor
		{
			lpClientItemRcd->tStdItem.wAC	= MAKEWORD(g_pStdItemArmor[nIdx].wAC, g_pStdItemArmor[nIdx].wAC2 + lpUserItemRcd->btValue[0]);
			lpClientItemRcd->tStdItem.wDC	= MAKEWORD(g_pStdItemArmor[nIdx].wDC, g_pStdItemArmor[nIdx].wDC2 + lpUserItemRcd->btValue[2]);
			lpClientItemRcd->tStdItem.wMAC	= MAKEWORD(g_pStdItemArmor[nIdx].wMAC, g_pStdItemArmor[nIdx].wMAC2 + lpUserItemRcd->btValue[1]);
			lpClientItemRcd->tStdItem.wMC	= MAKEWORD(g_pStdItemArmor[nIdx].wMC, g_pStdItemArmor[nIdx].wMC2 + lpUserItemRcd->btValue[3]);
			lpClientItemRcd->tStdItem.wSC	= MAKEWORD(g_pStdItemArmor[nIdx].wSC, g_pStdItemArmor[nIdx].wSC2 + lpUserItemRcd->btValue[4]);

			break;
		}
		case 'C': // Accessory
		{
			lpClientItemRcd->tStdItem.wAC  = MAKEWORD(g_pStdItemAccessory[nIdx].wAC, g_pStdItemAccessory[nIdx].wAC2 + lpUserItemRcd->btValue[0]);
			lpClientItemRcd->tStdItem.wMAC = MAKEWORD(g_pStdItemAccessory[nIdx].wMAC,g_pStdItemAccessory[nIdx].wMAC2+ lpUserItemRcd->btValue[1]);
			lpClientItemRcd->tStdItem.wDC  = MAKEWORD(g_pStdItemAccessory[nIdx].wDC, g_pStdItemAccessory[nIdx].wDC2 + lpUserItemRcd->btValue[2]);
			lpClientItemRcd->tStdItem.wMC  = MAKEWORD(g_pStdItemAccessory[nIdx].wMC, g_pStdItemAccessory[nIdx].wMC2 + lpUserItemRcd->btValue[3]);
			lpClientItemRcd->tStdItem.wSC  = MAKEWORD(g_pStdItemAccessory[nIdx].wSC, g_pStdItemAccessory[nIdx].wSC2 + lpUserItemRcd->btValue[4]);

			break;
		}
	}
*/	
	switch (btType)
	{
		case 0:
		{
			lpClientItemRcd->tStdItem.wDC		= MAKEWORD(wDC, wDC2 + lpUserItemRcd->btValue[0]);
			lpClientItemRcd->tStdItem.wMC		= MAKEWORD(wMC, wMC2 + lpUserItemRcd->btValue[1]);
			lpClientItemRcd->tStdItem.wSC		= MAKEWORD(wSC, wSC2 + lpUserItemRcd->btValue[2]); //3:幸运, 4:诅咒
			lpClientItemRcd->tStdItem.wAC		= MAKEWORD(wAC + lpUserItemRcd->btValue[3], wAC2 + lpUserItemRcd->btValue[5]);  //准确
			lpClientItemRcd->tStdItem.wMAC		= MAKEWORD(wMAC + lpUserItemRcd->btValue[4], wMAC2 + lpUserItemRcd->btValue[6]);  //攻击速度(-/+)
			lpClientItemRcd->tStdItem.btSource	= lpUserItemRcd->btValue[7];
		//	if (lpMakeItemRcd->btValue[10])
		//		lptWeaponItem->NeedIdentify := 0x01;
			break;
		}
		case 1:
		{
			lpClientItemRcd->tStdItem.wAC	= MAKEWORD(wAC, wAC2 + lpUserItemRcd->btValue[0]);
			lpClientItemRcd->tStdItem.wMAC	= MAKEWORD(wMAC, wMAC2 + lpUserItemRcd->btValue[1]);
			lpClientItemRcd->tStdItem.wDC	= MAKEWORD(wDC, wDC2 + lpUserItemRcd->btValue[2]);
			lpClientItemRcd->tStdItem.wMC	= MAKEWORD(wMC, wMC2 + lpUserItemRcd->btValue[3]);
			lpClientItemRcd->tStdItem.wSC	= MAKEWORD(wSC, wSC2 + lpUserItemRcd->btValue[4]);

			break;
		}
		case 2:
		{
			lpClientItemRcd->tStdItem.wAC  = MAKEWORD(wAC, wAC2 + lpUserItemRcd->btValue[0]);
			lpClientItemRcd->tStdItem.wMAC = MAKEWORD(wMAC,wMAC2+ lpUserItemRcd->btValue[1]);
			lpClientItemRcd->tStdItem.wDC  = MAKEWORD(wDC, wDC2 + lpUserItemRcd->btValue[2]);
			lpClientItemRcd->tStdItem.wMC  = MAKEWORD(wMC, wMC2 + lpUserItemRcd->btValue[3]);
			lpClientItemRcd->tStdItem.wSC  = MAKEWORD(wSC, wSC2 + lpUserItemRcd->btValue[4]);

			break;
		}
		default:
		{
			lpClientItemRcd->tStdItem.wAC		= 0;
			lpClientItemRcd->tStdItem.wMAC		= 0;
			lpClientItemRcd->tStdItem.wDC		= 0;
			lpClientItemRcd->tStdItem.wMC		= 0;
			lpClientItemRcd->tStdItem.wSC		= 0;
			lpClientItemRcd->tStdItem.btSource	= 0;

			break;
		}
	}
}

void CStdItemSpecial::UpgradeRandomItem(BYTE* btValue, WORD &nDura, WORD &nDuraMax)//_LPTMAKEITEMRCD lpMakeItemRcd)
{
	int nUpgrade, nIncp, nVal;

	switch (btType)
	{
		case 0:	// 武器
		{
			// 破坏选项
			nUpgrade = GetUpgrade(12, 15);
			if (rand() % 15 == 0) 
				btValue[0] = 1 + nUpgrade;	// DC

			// 攻击速度
			nUpgrade = GetUpgrade(12, 15);
			if (rand() % 20 == 0)
			{
				nIncp = (int)((1 + nUpgrade) / 3);

				if (nIncp > 0)
				{
					if (rand() % 3 != 0) btValue[6] = nIncp;	// 攻击速度 (-)
					else
						btValue[6] = 10 + nIncp; // 攻击速度 (+)
				}
			}

			// 魔法力
			nUpgrade = GetUpgrade(12, 15);
			if (rand() % 15 == 0) 
				btValue[1] = 1 + nUpgrade;	// MC

			// 道力
			nUpgrade = GetUpgrade(12, 15);
			if (rand() % 15 == 0) 
				btValue[2] = 1 + nUpgrade;	// SC

			// 准确
			nUpgrade = GetUpgrade(12, 15);
			if (rand() % 24 == 0) 
				btValue[5] = 1 + ((int)nUpgrade / 2);	// 准确 (+)

			// 耐久
			nUpgrade = GetUpgrade(12, 12);
			if (rand() % 3 < 2)
			{
				nVal = (1 + nUpgrade) * 2000;
				
				nDuraMax	= _MIN(65000, nDuraMax + nVal);
				nDura		= _MIN(65000, nDura + nVal);
			}

			// 强度
			nUpgrade = GetUpgrade(12, 15);
			if (rand() % 10 == 0) 
				btValue[7] = 1 + ((int)nUpgrade / 2);

			break;
		}
		case 1:	// 防具类
		{
			//防御
			nUpgrade = GetUpgrade(6, 15);
			if (rand() % 40 == 0) 
				btValue[0] = 1 + nUpgrade;	// AC

			//魔抗
			nUpgrade = GetUpgrade(6, 15);
			if (rand() % 40 == 0) 
				btValue[1] = 1 + nUpgrade;	// MAC

			//破坏
			nUpgrade = GetUpgrade(6, 20);
			if (rand() % 40 == 0) 
				btValue[2] = 1 + nUpgrade;	// DC

			//魔法
			nUpgrade = GetUpgrade(6, 20);
			if (rand() % 40 == 0) 
				btValue[3] = 1 + nUpgrade;	// MC

			//道力
			nUpgrade = GetUpgrade(6, 20);
			if (rand() % 40 == 0) 
				btValue[4] = 1 + nUpgrade;	// SC

			//耐久
			nUpgrade = GetUpgrade(6, 10);
			if (rand() % 8 < 6)
			{
				nVal = (1 + nUpgrade) * 2000;
				
				nDuraMax	= _MIN(65000, nDuraMax + nVal);
				nDura		= _MIN(65000, nDura + nVal);
			}

			break;
		}
		case 2:	// 戒指, 手镯, 项链
		{
			// 准确
			nUpgrade = GetUpgrade(6, 30);
			if (rand() % 60 == 0) 
				btValue[0] = 1 + nUpgrade;	// AC(HIT)

			//敏捷
			nUpgrade = GetUpgrade(6, 30);
			if (rand() % 60 == 0) 
				btValue[1] = 1 + nUpgrade;	// MAC(SPEED)

			//破坏
			nUpgrade = GetUpgrade(6, 20);
			if (rand() % 30 == 0) 
				btValue[2] = 1 + nUpgrade;	// DC

			//魔法
			nUpgrade = GetUpgrade(6, 20);
			if (rand() % 30 == 0) 
				btValue[3] = 1 + nUpgrade;	// MC

			//道力
			nUpgrade = GetUpgrade(6, 20);
			if (rand() % 30 == 0) 
				btValue[4] = 1 + nUpgrade;	// SC

			//耐久
			nUpgrade = GetUpgrade(6, 12);
			if (rand() % 20 < 15)
			{
				nVal = (1 + nUpgrade) * 1000;
				
				nDuraMax	= _MIN(65000, nDuraMax + nVal);
				nDura		= _MIN(65000, nDura + nVal);
			}

			break;
		}
	}
}

void CStdItemSpecial::ApplyItemParameters(CObjectAddAbility *m_pAddAbility)
{
	switch (btType)
	{
		case 0:
		{
            m_pAddAbility->HIT			= m_pAddAbility->HIT + wAC2;

            if (wMAC2 > 10)
               m_pAddAbility->HitSpeed	= m_pAddAbility->HitSpeed + wMAC2 - 10; //攻击速度 (+)
            else
               m_pAddAbility->HitSpeed	= m_pAddAbility->HitSpeed - wMAC2; //攻击速度 (-)

            m_pAddAbility->Luck			= m_pAddAbility->Luck + wAC;   //通过游戏内的活动附加
            m_pAddAbility->UnLuck		= m_pAddAbility->UnLuck + wMAC;  //通过游戏内事件附加(PK)

			break;
		}
		case 1:
		{															   
			m_pAddAbility->AC	= MAKEWORD(LOBYTE(m_pAddAbility->AC) + wAC, HIBYTE(m_pAddAbility->AC) + wAC2);
			m_pAddAbility->MAC	= MAKEWORD(LOBYTE(m_pAddAbility->MAC) + wMAC, HIBYTE(m_pAddAbility->MAC) + wMAC2);

			break;
		}
		case 2:
		{
			switch (wStdMode)
			{
				case 21: //项链
					m_pAddAbility->AntiMagic	+= wAC2;
					m_pAddAbility->UnLuck		+= wMAC;
					m_pAddAbility->Luck			+= wMAC2;
					break;
				case 22: //项链
				case 41: //手镯
					m_pAddAbility->HIT			+= wAC2;
					m_pAddAbility->SPEED		+= wMAC2;
					break;
				case 23: //项链
					m_pAddAbility->HealthRecover	+= wAC2;
					m_pAddAbility->SpellRecover		+= wMAC2;
					m_pAddAbility->HitSpeed			+= wAC;
					m_pAddAbility->HitSpeed			-= wMAC;
					break;
				case 32: //戒指
					m_pAddAbility->AntiPoison		+= wAC2;
					m_pAddAbility->PoisonRecover	+= wMAC2;
					m_pAddAbility->HitSpeed			+= wAC;
					m_pAddAbility->HitSpeed			-= wMAC;
					break;
				default:
					m_pAddAbility->AC	= MAKEWORD(LOBYTE(m_pAddAbility->AC) + wAC, HIBYTE(m_pAddAbility->AC) + wAC2);
					m_pAddAbility->MAC	= MAKEWORD(LOBYTE(m_pAddAbility->MAC) + wMAC, HIBYTE(m_pAddAbility->MAC) + wMAC2);
					break;
			}

			break;
		}
	}

	m_pAddAbility->DC = MAKEWORD(LOBYTE(m_pAddAbility->DC) + wDC, HIBYTE(m_pAddAbility->DC) + wDC2);
	m_pAddAbility->MC = MAKEWORD(LOBYTE(m_pAddAbility->MC) + wMC, HIBYTE(m_pAddAbility->MC) + wMC2);
	m_pAddAbility->SC = MAKEWORD(LOBYTE(m_pAddAbility->SC) + wSC, HIBYTE(m_pAddAbility->SC) + wSC2);
}

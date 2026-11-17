#include "stdafx.h"
#include "MagicWire.h"
#include "../../Shared/Mir3ChatOptions.h"
#include "../Def/Mir3Ability.h"

namespace {
std::string ItemSqlText(const char* text) {
    std::string result="'";
    for (;*text;++text) { result+=*text; if (*text=='\'') result+='\''; }
    return result+"'";
}
bool BuildSpecialItem(CUserInfo& owner, const _TUSERITEMRCD& item, Mir3Item::Record& record)
{
    if (item.nStdIndex >= g_nStdItemSpecial) return false;
    _TCLIENTITEMRCD legacy = {};
    auto& standard = g_pStdItemSpecial[item.nStdIndex];
    standard.GetStandardItem(&legacy);
    legacy.nDura = item.nDura; legacy.nDuraMax = item.nDuraMax;
    record = Mir3Item::FromLegacy(legacy, owner.m_itemIdentities.Get(item.szMakeIndex));
    Mir3Item::ApplyUpgrades(record,item.btValue);
    return true;
}
bool BuildGeneralItem(CUserInfo& owner, const _TGENERALITEMRCD& item, Mir3Item::Record& record)
{
    if (item.nStdIndex < 0 || item.nStdIndex >= g_nStdItemEtc) return false;
    _TCLIENTITEMRCD legacy = {};
    auto& standard = g_pStdItemEtc[item.nStdIndex];
    standard.GetStandardItem(&legacy);
    legacy.nDura = item.nDura; legacy.nDuraMax = item.nDuraMax;
    record = Mir3Item::FromLegacy(legacy, owner.m_itemIdentities.Get(item.szMakeIndex, &item));
    record[Mir3Item::StackMode] = standard.stackMode;
    if (standard.wStdMode == 0) {
        Mir3Item::Word(record, Mir3Item::Defence, standard.wDuraMax);
        Mir3Item::Word(record, Mir3Item::MagicDefence, standard.dwRSource);
    }
    return true;
}
void AppendItem(std::string& body, const Mir3Item::Record& record)
{
    char encoded[Mir3Item::EncodedSize + 1] = {};
    const int size = fnEncode6BitBufA(const_cast<unsigned char*>(record.data()), encoded, record.size(), sizeof(encoded));
    body.append(encoded, size); body += '/';
}
bool IsEquipped(const CUserInfo& owner, const char* key)
{
    for (const auto& equipped : owner.m_THumanRcd.szTakeItem)
        if (!memcmp(equipped, key, Mir3Item::LegacyKeySize)) return true;
    return false;
}
}

#define _MSG_GOOD		"+GOOD/"
#define _MSG_FAIL		"+FAIL/"

CMagicInfo*		GetMagicInfo(int nMagicID);
int				GetBack(int nDirection);
CMirMap*		GetMap(char *pszMapName);

CPlayerObject*	GetUserHuman(char *pszCharName);

void SendRDBSocket(int nCertification, char *pszData1, char *pszData2, int nData2Len);

void CPlayerObject::Constructor()
{
	m_bEmpty				= true;
	m_wObjectType			= _OBJECT_HUMAN;

	m_nViewRange			= 12;
	m_dwSearchTime			= GetTickCount();
	m_dwSearchTick			= 1000;

	m_fIsDead				= FALSE;

	m_btLight				= 2;

	m_fAdminMode			= FALSE;
	
	m_dwLastCryTime			= 0;
    m_clientOptions = 0;
    m_dialogMerchant = NULL;
    m_shopMerchant = NULL;
    m_sellMerchant = NULL;
    m_npcScriptFiles.clear();
    m_npcOptions.clear();

	m_fIsCapture			= FALSE;
}

bool CPlayerObject::IsEmpty()
{
	return m_bEmpty;
}

CPlayerObject::CPlayerObject(): CCharObject(NULL)
{
	Constructor();
}

CPlayerObject::CPlayerObject(CUserInfo* pUserInfo): CCharObject(pUserInfo)
{
	Constructor();
}

BOOL CPlayerObject::IsProperTarget(CCharObject* pTargetObject)
{
	BOOL fFlag = FALSE;

	if (!pTargetObject)
		return fFlag;

	int nMode = m_pUserInfo->GetAttackMode();

	switch (nMode)
	{
		case HAM_ALL:
			if (!(pTargetObject->m_wObjectType & _OBJECT_NPC))
				fFlag = TRUE;
			break;
		case HAM_PEACE:
			if (pTargetObject->m_wObjectType & _OBJECT_ANIMAL)
				fFlag = TRUE;
			break;
		case HAM_GROUP:
/*                  if not ((target.RaceServer >= RC_NPC) and (target.RaceServer <= RC_PEACENPC)) then
             Result := TRUE;
          if target.RaceServer = RC_USERHUMAN then
             if IsGroupMember (target) then
                Result := FALSE; */
			break;
		case HAM_GUILD:
/*                  if not ((target.RaceServer >= RC_NPC) and (target.RaceServer <= RC_PEACENPC)) then
             Result := TRUE;
          if target.RaceServer = RC_USERHUMAN then
             if MyGuild <> nil then begin
                if TGuild(MyGuild).IsMember(target.UserName) then
                   Result := FALSE;
                if BoGuildWarArea and (target.MyGuild <> nil) then begin  //处于行会战,攻城战地区
                   if TGuild(MyGuild).IsAllyGuild(TGuild(target.MyGuild)) then
                      Result := FALSE;
                end;
             end;
       end; */
			break;
		case HAM_PKATTACK:
/*
          if not ((target.RaceServer >= RC_NPC) and (target.RaceServer <= RC_PEACENPC)) then
             Result := TRUE;
          if target.RaceServer = RC_USERHUMAN then
             if self.PKLevel >= 2 then begin  //攻击者是红名
                if target.PKLevel < 2 then Result := TRUE
                else Result := FALSE;
             end else begin
                //攻击者是白名
                if target.PKLevel >= 2 then Result := TRUE
                else Result := FALSE;
             end;
       end;
*/			break;
	}

/*			if target.RaceServer = RC_USERHUMAN then begin
        if (InSafeZone) or (target.InSafeZone) then begin
           Result := FALSE;
        end;

        if not target.BoInFreePKArea then begin  //攻城战区域除外。
           if (PKLevel >= 2) and (Abil.Level > 10) then begin //高等级红名
              if (target.Abil.Level <= 10) and (target.PKLevel < 2) then //无法攻击低等级善良新手。
                 Result := FALSE;
           end;
           if (Abil.Level <= 10) and (PKLevel < 2) then begin //低等级善良新手
              if (target.PKLevel >= 2) and (target.Abil.Level > 10) then //无法攻击高等级红名。
                 Result := FALSE;
           end;
        end;

        //地图移动后 3 秒内无法攻击，也不会被攻击
        if (GetTickCount - MapMoveTime < 3000) or (GetTickCount - target.MapMoveTime < 3000) then
           Result := FALSE;

     end else begin
        if target.Master = self then begin  //我的部下
           if HumAttackMode <> HAM_ALL then  //仅在全部攻击模式下部下才会攻击
              Result := FALSE;
        end else begin
           if target.Master <> nil then
              Result := IsProperTarget(target.Master);
        end;
     end;	*/

	return fFlag;
}

void CPlayerObject::GetCharName(char *pszCharName)
{ 
	strcpy(pszCharName, m_szName); 
}

BOOL CPlayerObject::ReadBook(char *pszMakeIndex)
{
	if (!m_pUserInfo) return FALSE;

	int nMagicID = m_pUserInfo->GetMagicID(pszMakeIndex);

	if (m_pUserInfo->IsMyMagic(nMagicID)) return FALSE;

	CMagicInfo*	pMagicInfo	= GetMagicInfo(nMagicID);

	if (pMagicInfo)
	{
		if (((pMagicInfo->sJob == m_pUserInfo->GetJob()) || pMagicInfo->sJob == 99) && m_Ability.Level >= pMagicInfo->sNeed[0])
		{
			char				szEncodeMsg[256];
			_TDEFAULTMESSAGE	DefMsg;
			_LPTHUMANMAGICRCD	lpHumanMagicRcd = new _THUMANMAGICRCD;

			if (lpHumanMagicRcd)
			{
				lpHumanMagicRcd->btMagicID		= nMagicID;
				lpHumanMagicRcd->btLevel		= 0;
				lpHumanMagicRcd->btUseKey		= 0;
				lpHumanMagicRcd->nCurrTrain		= 0;

				m_pUserInfo->m_lpTMagicRcd.AddNewNode(lpHumanMagicRcd);
				
				RecalcAbilitys();

				Mir3Magic::Record tClientMagicRcd{};

                if(!ClientMagicRecord(*lpHumanMagicRcd,tClientMagicRcd)) return FALSE;

				fnMakeDefMessage(&DefMsg, SM_ADDMAGIC, 0, 0, 0, 1);
				int nPos = fnEncode6BitBufA((unsigned char *)&tClientMagicRcd, szEncodeMsg, sizeof(tClientMagicRcd), sizeof(szEncodeMsg));
				szEncodeMsg[nPos] = '\0';
				SendSocket(&DefMsg, szEncodeMsg);

				return TRUE;
			}
		}
	}

	return FALSE;
}

BOOL CPlayerObject::EatItem(char *pszMakeIndex)
{
	int nStdIndex = m_pUserInfo->GetGenItemStdIdx(pszMakeIndex);

	if (g_pStdItemEtc[nStdIndex].wStdMode != 0) return FALSE;

	if (g_pStdItemEtc[nStdIndex].wShape == 1)
		IncHealthSpell(g_pStdItemEtc[nStdIndex].wDuraMax/*+hp*/, g_pStdItemEtc[nStdIndex].dwRSource/*+mp*/);
	else
	{
		if ((m_IncHealth + g_pStdItemEtc[nStdIndex].wDuraMax < 500) && (g_pStdItemEtc[nStdIndex].wDuraMax > 0))
			m_IncHealth += (WORD)g_pStdItemEtc[nStdIndex].wDuraMax;
		
		if ((m_IncSpell + g_pStdItemEtc[nStdIndex].dwRSource < 500) && (g_pStdItemEtc[nStdIndex].dwRSource > 0))
			m_IncSpell += (WORD)g_pStdItemEtc[nStdIndex].dwRSource;
	}

	return TRUE;
}

BOOL CPlayerObject::ServerGetEatItem(int nItemIndex, char *pszMakeIndex, DWORD instance)
{
	if (!m_pUserInfo) return FALSE;

	_TDEFAULTMESSAGE	DefMsg;
	BOOL				fFlag = FALSE;

	_LPTGENERALITEMRCD lptGenItem = m_pUserInfo->GetUseGenItem(pszMakeIndex, instance);

	if (lptGenItem)
	{
		switch (g_pStdItemEtc[nItemIndex].wStdMode)
		{
			case 0:
			{
				if (EatItem(pszMakeIndex))
				{
					m_pUserInfo->RemoveGenItem(lptGenItem);
					fFlag = TRUE;
				}

				break;
			}
			case 3:
			{
				break;
			}
			case 4:
			{
				if (ReadBook(pszMakeIndex))
				{
					m_pUserInfo->RemoveGenItem(lptGenItem);

					if (m_pUserInfo->m_lpTMagicErgumSkill)
					{
						if (!m_pUserInfo->m_fLongHitSkill)
						{
							m_pUserInfo->AllowLongHitSkill(TRUE);
							SendSocket(NULL, "+LNG");
						}
					}

					if (m_pUserInfo->m_lpTMagicBanwolSkill)
					{
						if (!m_pUserInfo->m_fWideHitSkill)
						{
							m_pUserInfo->AllowWideHitSkill(TRUE);
							SendSocket(NULL, "+WID");
						}
					}

					fFlag = TRUE;
				}

				break;
			}
		}
	}

	if (fFlag)
	{
 		fnMakeDefMessage(&DefMsg, SM_EAT_OK, 0, 0, 0, 0);
		SendSocket(&DefMsg, NULL);
	}
	else
	{
 		fnMakeDefMessage(&DefMsg, SM_EAT_FAIL, 0, 0, 0, 0);
		SendSocket(&DefMsg, NULL);
	}

	return fFlag;
}

void CPlayerObject::SendAddItem(_LPTUSERITEMRCD item)
{
    Mir3Item::Record record;
    bool built = false;
    if (item->szMakeIndex[0] == 'G') {
        SendAddGeneralItem(m_pUserInfo->GetUseGenItem(item->szMakeIndex));
        return;
    }
    built = BuildSpecialItem(*m_pUserInfo, *item, record);
    if (!built) return;
    char encoded[Mir3Item::EncodedSize + 1] = {};
    fnEncode6BitBufA(const_cast<unsigned char*>(record.data()), encoded, record.size(), sizeof(encoded));
    _TDEFAULTMESSAGE message;
    fnMakeDefMessage(&message, SM_ADDITEM, (int)this, 0, 0, 1);
    SendSocket(&message, encoded);
}

void CPlayerObject::SendAddGeneralItem(_LPTGENERALITEMRCD item)
{
    Mir3Item::Record record;
    if (!item || !BuildGeneralItem(*m_pUserInfo, *item, record)) return;
    char encoded[Mir3Item::EncodedSize + 1] = {};
    fnEncode6BitBufA(record.data(), encoded, record.size(), sizeof(encoded));
    _TDEFAULTMESSAGE message;
    fnMakeDefMessage(&message, SM_ADDITEM, (int)this, 0, 0, 1);
    SendSocket(&message, encoded);
}

BOOL CPlayerObject::PickUp()
{
	CMapItem* pMapItem = m_pMap->GetItem(m_nCurrX, m_nCurrY);

	if (pMapItem)
	{
		if (memcmp(pMapItem->szName, g_szGoldName, memlen(g_szGoldName) - 1) == 0)
		{
			if (m_pMap->RemoveObject(m_nCurrX, m_nCurrY, OS_ITEMOBJECT, (VOID *)pMapItem))
			{
				if (m_pUserInfo->IncGold(pMapItem->nCount))
				{
					AddRefMsg(RM_ITEMHIDE, 0, (int)pMapItem, m_nCurrX, m_nCurrY, NULL);

					GoldChanged();
					
					delete pMapItem;
					pMapItem = NULL;

					return TRUE;
				}
				else
					m_pMap->AddNewObject(m_nCurrX, m_nCurrY, OS_ITEMOBJECT, (VOID *)pMapItem);
			}
		}
		else
		{
			if (m_pUserInfo->IsEnoughBag())
			{
				if (m_pMap->RemoveObject(m_nCurrX, m_nCurrY, OS_ITEMOBJECT, (VOID *)pMapItem))
				{
                    const auto key = reinterpret_cast<const char*>(pMapItem->pItem);
                    if (key[0] == 'G') {
                        const auto general = reinterpret_cast<_LPTGENERALITEMRCD>(pMapItem->pItem);
                        m_pUserInfo->m_lpTGenItemRcd.AddNewNode(general);
                        SendAddGeneralItem(general);
                    } else {
                        const auto item = reinterpret_cast<_LPTUSERITEMRCD>(pMapItem->pItem);
                        const int updated = UpdateItemToDB(item, _ITEM_ACTION_PICKUP);
                        if (!updated) {
                            m_pMap->AddNewObject(m_nCurrX, m_nCurrY, OS_ITEMOBJECT, pMapItem);
                            return FALSE;
                        }
                        if (updated == 2) {
                            m_pUserInfo->m_lpTItemRcd.AddNewNode(item);
                            SendAddItem(item);
                        }
                    }
                    AddRefMsg(RM_ITEMHIDE, 0, (int)pMapItem, m_nCurrX, m_nCurrY, NULL);
                    delete pMapItem;
                    WeightChanged();
                    return TRUE;
				}
			}
		}
	}

	return FALSE;
}
/*function  TCreature.PickUp: Boolean;
var
   i: integer;
   pu: PTUserItem;
   pmi: PTMapItem;
   ps: PTStdItem;
   hum: TUserHuman;
begin
   Result := FALSE;
   if BoDealing then exit;  //交易中无法拾取物品.
   pmi := PEnvir.GetItem (CX, CY);
   if pmi <> nil then begin
      if CompareText (pmi.Name, '金钱') = 0 then begin
         if PEnvir.DeleteFromMap (CX, CY, OS_ITEMOBJECT, TObject(pmi)) = 1 then begin
            if IncGold (pmi.Count) then begin
               SendRefMsg (RM_ITEMHIDE, 0, integer(pmi), CX, CY, '');
               //记录留痕
               AddUserLog ('拾取_' +
                           MapName + '_' +
                           IntToStr(CX) + '_' +
                           IntToStr(CY) + '_' +
                           UserName + '_' +
                           '金钱_' +
                           IntToStr(pmi.count));
               GoldChanged;
               Dispose (pmi);
            end else
               PEnvir.AddToMap (CX, CY, OS_ITEMOBJECT, TObject(pmi));
         end;
      end else
         if IsEnoughBag then begin
            if PEnvir.DeleteFromMap (CX, CY, OS_ITEMOBJECT, TObject(pmi)) = 1 then begin
               new (pu);
               pu^ := pmi.UserItem;
               ps := UserEngine.GetStdItem (pu.Index);
               if (ps <> nil) and IsAddWeightAvailable (UserEngine.GetStdItemWeight(pu.Index)) then begin
                  SendMsg (self, RM_ITEMHIDE, 0, integer(pmi), CX, CY, '');
                  AddItem (pu);
                  //记录留痕
                  if not IsCheapStuff (ps.StdMode) then
                     AddUserLog ('拾取_' +
                                 MapName + '_' +
                                 IntToStr(CX) + '_' +
                                 IntToStr(CY) + '_' +
                                 UserName + '_' +
                                 UserEngine.GetStdItemName (pu.Index) + '_' +
                                 IntToStr(pu.MakeIndex));
                  Dispose (pmi);
                  if RaceServer = RC_USERHUMAN then begin
                     if self is TUserHuman then begin
                        hum := TUserHuman(self);
                        TUserHuman(hum).SendAddItem (pu^);
                     end;
                  end;
                  Result := TRUE;
               end else begin
                  Dispose (pu);
                  PEnvir.AddToMap (CX, CY, OS_ITEMOBJECT, TObject(pmi));
               end;
            end;
         end;
   end;
end;
*/

int CPlayerObject::UpdateItemToDB(_LPTUSERITEMRCD item, int action)
{
    if (!g_pConnGame->BeginTransaction()) return 0;
    auto candidate=*item;
    const int result=WriteItemRecord(&candidate,action);
    if (!g_pConnGame->EndTransaction(result!=0) || !result) return 0;
    *item=candidate;
    return result;
}

bool CPlayerObject::TradeItemToDB(_LPTUSERITEMRCD item, int action, int gold)
{
    if (!g_pConnGame->BeginTransaction()) return false;
    auto candidate=*item;
    bool saved=WriteItemRecord(&candidate,action)!=0;
    if (saved) {
        auto* record=g_pConnGame->CreateRecordset();
        std::string query="UPDATE TBL_CHARACTER SET FLD_GOLD="+std::to_string(gold)+
            " WHERE FLD_LOGINID="+ItemSqlText(m_pUserInfo->m_szUserID)+" AND FLD_CHARNAME="+ItemSqlText(m_pUserInfo->m_szCharName);
        saved=record && record->Execute(&query[0]) && record->GetRowCount()==1;
        if (record) g_pConnGame->DestroyRecordset(record);
    }
    saved=g_pConnGame->EndTransaction(saved) && saved;
    if (saved) *item=candidate;
    return saved;
}

int CPlayerObject::WriteItemRecord(_LPTUSERITEMRCD lpMakeItemRcd, int nAction)
{
	char	szQuery[1024];

	char	szMakeDay[7];
	char	szMakeIndex[6];

	ZeroMemory(szMakeDay, sizeof(szMakeDay));
	ZeroMemory(szMakeIndex, sizeof(szMakeIndex));

	memcpy(szMakeDay, &lpMakeItemRcd->szMakeIndex[1], 6);
	memcpy(szMakeIndex, &lpMakeItemRcd->szMakeIndex[7], 5);

    // Commit the new identity before PickUp completes the transfer or notifies the client.
    if (atoi(szMakeDay) == 0 && atoi(szMakeIndex) == 0) {
        SYSTEMTIME now;GetLocalTime(&now);
        char date[7];sprintf(date,"%02u%02u%02u",now.wYear%100,now.wMonth,now.wDay);
        char category[2]={lpMakeItemRcd->szMakeIndex[0],0};
        auto* record=g_pConnGame->CreateRecordset();
        if(!record) return 0;
        std::string query="SELECT MAX(CAST(FLD_MAKEINDEX AS INT)) AS LAST_INDEX FROM TBL_CHARACTER_ITEM WHERE FLD_STDTYPE="+
            ItemSqlText(category)+" AND FLD_MAKEDATE="+ItemSqlText(date);
        bool saved=record->Execute(&query[0]) && record->Fetch();
        const int next=saved?atoi(record->Get("LAST_INDEX"))+1:0;
        constexpr int MaximumDailyItemIndex=99999;
        char serial[6]={};
        if(saved && next<=MaximumDailyItemIndex) {
            sprintf(serial,"%05d",next);
            query="INSERT INTO TBL_CHARACTER_ITEM (FLD_LOGINID,FLD_CHARNAME,FLD_STDTYPE,FLD_MAKEDATE,FLD_MAKEINDEX,FLD_STDINDEX,FLD_DURA,FLD_DURAMAX,FLD_PREFIXNAME,FLD_LASTOWNER,FLD_LASTACTION";
            for(int i=1;i<=_countof(lpMakeItemRcd->btValue);++i) query+=",FLD_VALUE"+std::to_string(i);
            query+=") VALUES ("+ItemSqlText(m_pUserInfo->m_szUserID)+","+ItemSqlText(m_pUserInfo->m_szCharName)+","+
                ItemSqlText(category)+","+ItemSqlText(date)+","+ItemSqlText(serial)+","+std::to_string(lpMakeItemRcd->nStdIndex)+","+
                std::to_string(lpMakeItemRcd->nDura)+","+std::to_string(lpMakeItemRcd->nDuraMax)+","+
                ItemSqlText(lpMakeItemRcd->szPrefixName)+","+ItemSqlText(m_pUserInfo->m_szCharName)+","+std::to_string(_ITEM_ACTION_PICKUP);
            for(auto value:lpMakeItemRcd->btValue) query+=","+std::to_string(value);
            query+=")";
            saved=record->Execute(&query[0]) && record->GetRowCount()==1;
        } else saved=false;
        g_pConnGame->DestroyRecordset(record);
        if(!saved) return 0;
        memcpy(lpMakeItemRcd->szMakeIndex+1,date,6);memcpy(lpMakeItemRcd->szMakeIndex+7,serial,5);
        return 2;
    }

	switch (nAction)
	{
		case _ITEM_ACTION_CREATE:
		{
/*			sprintf(szQuery, "INSERT TBL_CHARACTER_ITEM (FLD_LOGINID, FLD_CHARNAME, FLD_STDTYPE, FLD_MAKEDATE, FLD_MAKEINDEX, "
								"FLD_STDINDEX, FLD_DURA, FLD_DURAMAX, FLD_VALUE1, FLD_VALUE2, FLD_VALUE3, FLD_VALUE4, FLD_VALUE5, "
								"FLD_VALUE6, FLD_VALUE7, FLD_VALUE8, FLD_VALUE9, FLD_VALUE10, FLD_VALUE11, FLD_VALUE12, FLD_VALUE13, "
								"FLD_VALUE14, FLD_LASTOWNER, FLD_LASTACTION) "
								"VALUES( '%s', '%s', '%c', '%s', '%05d', %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %s, %d )", 
								pszUserID, pszCharName, lpMakeItemRcd->szMakeIndex[0], g_szYesterDay, g_nItemIndexCnt,
								lpMakeItemRcd->nStdIndex, lpMakeItemRcd->nDura,	lpMakeItemRcd->nDuraMax, 
								lpMakeItemRcd->btValue[0], lpMakeItemRcd->btValue[1], lpMakeItemRcd->btValue[2], lpMakeItemRcd->btValue[3], 
								lpMakeItemRcd->btValue[4], lpMakeItemRcd->btValue[5], lpMakeItemRcd->btValue[6], lpMakeItemRcd->btValue[7],
								lpMakeItemRcd->btValue[8], lpMakeItemRcd->btValue[9], lpMakeItemRcd->btValue[10], lpMakeItemRcd->btValue[11], 
								lpMakeItemRcd->btValue[12], lpMakeItemRcd->btValue[13], pszCharName, _ITEM_ACTION_CREATE);
*/			break;
		}
		case _ITEM_ACTION_THROW:
		{
			sprintf(szQuery, "UPDATE TBL_CHARACTER_ITEM SET FLD_LOGINID='%s', FLD_CHARNAME='%s', "
								"FLD_DURA=%d, FLD_DURAMAX=%d, FLD_VALUE1=%d, FLD_VALUE2=%d, FLD_VALUE3=%d, FLD_VALUE4=%d, FLD_VALUE5=%d, "
								"FLD_VALUE6=%d, FLD_VALUE7=%d, FLD_VALUE8=%d, FLD_VALUE9=%d, FLD_VALUE10=%d, FLD_VALUE11=%d, FLD_VALUE12=%d, FLD_VALUE13=%d, "
								"FLD_VALUE14=%d, FLD_LASTOWNER='%s', FLD_LASTACTION=%d "
								"WHERE FLD_STDTYPE='%c' AND FLD_MAKEDATE='%s' AND FLD_MAKEINDEX='%s' AND FLD_STDINDEX=%d",
								"0", "WEMADE", lpMakeItemRcd->nDura, lpMakeItemRcd->nDuraMax, 
								lpMakeItemRcd->btValue[0], lpMakeItemRcd->btValue[1], lpMakeItemRcd->btValue[2], lpMakeItemRcd->btValue[3], 
								lpMakeItemRcd->btValue[4], lpMakeItemRcd->btValue[5], lpMakeItemRcd->btValue[6], lpMakeItemRcd->btValue[7],
								lpMakeItemRcd->btValue[8], lpMakeItemRcd->btValue[9], lpMakeItemRcd->btValue[10], lpMakeItemRcd->btValue[11], 
								lpMakeItemRcd->btValue[12], lpMakeItemRcd->btValue[13], m_pUserInfo->m_szCharName, _ITEM_ACTION_THROW,
								lpMakeItemRcd->szMakeIndex[0], szMakeDay, szMakeIndex, lpMakeItemRcd->nStdIndex);
			break;
		}
		case _ITEM_ACTION_PICKUP:
		{
			sprintf(szQuery, "UPDATE TBL_CHARACTER_ITEM SET FLD_LOGINID='%s', FLD_CHARNAME='%s', "
								"FLD_DURA=%d, FLD_DURAMAX=%d, FLD_VALUE1=%d, FLD_VALUE2=%d, FLD_VALUE3=%d, FLD_VALUE4=%d, FLD_VALUE5=%d, "
								"FLD_VALUE6=%d, FLD_VALUE7=%d, FLD_VALUE8=%d, FLD_VALUE9=%d, FLD_VALUE10=%d, FLD_VALUE11=%d, FLD_VALUE12=%d, FLD_VALUE13=%d, "
								"FLD_VALUE14=%d, FLD_LASTOWNER='%s', FLD_LASTACTION=%d "
								"WHERE FLD_STDTYPE='%c' AND FLD_MAKEDATE='%s' AND FLD_MAKEINDEX='%s' AND FLD_STDINDEX=%d",
								m_pUserInfo->m_szUserID, m_pUserInfo->m_szCharName, lpMakeItemRcd->nDura, lpMakeItemRcd->nDuraMax, 
								lpMakeItemRcd->btValue[0], lpMakeItemRcd->btValue[1], lpMakeItemRcd->btValue[2], lpMakeItemRcd->btValue[3], 
								lpMakeItemRcd->btValue[4], lpMakeItemRcd->btValue[5], lpMakeItemRcd->btValue[6], lpMakeItemRcd->btValue[7],
								lpMakeItemRcd->btValue[8], lpMakeItemRcd->btValue[9], lpMakeItemRcd->btValue[10], lpMakeItemRcd->btValue[11], 
								lpMakeItemRcd->btValue[12], lpMakeItemRcd->btValue[13], "WEMADE", _ITEM_ACTION_PICKUP,
								lpMakeItemRcd->szMakeIndex[0], szMakeDay, szMakeIndex, lpMakeItemRcd->nStdIndex);
			break;
		}
		case _ITEM_ACTION_UPDATE:
		{
			sprintf(szQuery, "UPDATE TBL_CHARACTER_ITEM SET "
								"FLD_DURA=%d, FLD_DURAMAX=%d, FLD_VALUE1=%d, FLD_VALUE2=%d, FLD_VALUE3=%d, FLD_VALUE4=%d, FLD_VALUE5=%d, "
								"FLD_VALUE6=%d, FLD_VALUE7=%d, FLD_VALUE8=%d, FLD_VALUE9=%d, FLD_VALUE10=%d, FLD_VALUE11=%d, FLD_VALUE12=%d, FLD_VALUE13=%d, "
								"FLD_VALUE14=%d, FLD_LASTOWNER='%s', FLD_LASTACTION=%d, FLD_PREFIXNAME='%s' "
								"WHERE FLD_STDTYPE='%c' AND FLD_MAKEDATE='%s' AND FLD_MAKEINDEX='%s' AND FLD_STDINDEX=%d",
								lpMakeItemRcd->nDura, lpMakeItemRcd->nDuraMax, 
								lpMakeItemRcd->btValue[0], lpMakeItemRcd->btValue[1], lpMakeItemRcd->btValue[2], lpMakeItemRcd->btValue[3], 
								lpMakeItemRcd->btValue[4], lpMakeItemRcd->btValue[5], lpMakeItemRcd->btValue[6], lpMakeItemRcd->btValue[7],
								lpMakeItemRcd->btValue[8], lpMakeItemRcd->btValue[9], lpMakeItemRcd->btValue[10], lpMakeItemRcd->btValue[11], 
								lpMakeItemRcd->btValue[12], lpMakeItemRcd->btValue[13], m_pUserInfo->m_szCharName, _ITEM_ACTION_UPDATE, lpMakeItemRcd->szPrefixName,
								lpMakeItemRcd->szMakeIndex[0], szMakeDay, szMakeIndex, lpMakeItemRcd->nStdIndex);
			break;
		}

		default:
			return 0;
	}

	CRecordset *pRec = g_pConnGame->CreateRecordset();

	if ( !pRec->Execute( szQuery ) || pRec->GetRowCount() <= 0 )
	{
		InsertLogMsg(_T("UpdateItemToDB : update error"));
		g_pConnGame->DestroyRecordset( pRec );

		return 0;
	}

	g_pConnGame->DestroyRecordset( pRec );

	return 2;
}

void CPlayerObject::WeightChanged()
{
	if (m_pUserInfo)
	{
		m_WAbility.Weight = m_pUserInfo->CalcBagWeight();
		UpdateProcess(this, RM_WEIGHTCHANGED, 0, 0, 0, 0, NULL); 
	}
}

void CPlayerObject::Whisper(char *pszChar, char *pszMsg)
{
	char	szChatMsg[512];
	int		nLen;

	CPlayerObject* pPlayerObject = GetUserHuman(pszChar);

	if (pPlayerObject)
	{
        if (pPlayerObject->m_clientOptions & Mir3ChatOptions::RejectWhisper) {
            SysMsg("Private messages are disabled by the recipient.", 0);
            return;
        }
		nLen = memlen(m_pUserInfo->m_szCharName);

		memmove(szChatMsg, m_pUserInfo->m_szCharName, (nLen - 1));
		memmove(&szChatMsg[nLen - 1], "=> ", 3);
		memmove(&szChatMsg[nLen + 2], pszMsg, memlen(pszMsg));

		pPlayerObject->AddProcess(this, RM_WHISPER, 0, 0, 0, 0, szChatMsg);
	}
	else
	{
		TCHAR	wszMsg[64];

		nLen = memlen(pszChar);

		LoadString(g_hInst, IDS_CANTFIND_HUMAN, wszMsg, sizeof(wszMsg)/sizeof(TCHAR));

		memmove(szChatMsg, pszChar, (nLen - 1));
		WideCharToMultiByte(CP_ACP, 0, wszMsg, -1, &szChatMsg[nLen - 1], sizeof(szChatMsg) - nLen, NULL, NULL);

		SysMsg(szChatMsg, 0);
	}
}
/*procedure TUserHuman.Whisper (whostr, saystr: string);
var
   hum: TUserHuman;
   svidx: integer;
begin
   hum := TUserHuman (UserEngine.GetUserHuman (whostr));
   if hum <> nil then begin
      if not hum.ReadyRun then begin
         SysMsg (whostr + '无法向该用户传递消息。', 0);
         exit;
      end;
      if not hum.BoHearWhisper or hum.IsBlockWhisper (UserName) then begin
         SysMsg (whostr + '该用户正在拒绝私聊。', 0);
         exit;
      end;
      hum.SendMsg (self, RM_WHISPER, 0, 0, 0, 0, UserName + '=> ' + saystr);
   end else begin
      if UserEngine.FindOtherServerUser (whostr, svidx) then begin
         UserEngine.SendInterMsg (ISM_WHISPER, svidx, whostr + '/' + UserName + '=> ' + saystr);
      end else
         SysMsg (whostr + '找不到该用户。', 0);
   end;
end;
}
*/

int CPlayerObject::CalcGetExp(int nTagLevel, int nTagHP)
{
	int nExp;

	if (m_Ability.Level < nTagLevel + 10) nExp = nTagHP;
	else
		nExp = nTagHP - ROUND((nTagHP / 15) * (m_Ability.Level - (nTagLevel + 10)));

	if (nExp <= 0)
		nExp = 1;

	return nExp;
}

void CPlayerObject::AddBodyLuck(double fLuck)
{
/*	if (fLuck > 0) && (
   if (r > 0) and (BodyLuck < 5 * BODYLUCKUNIT) then
      BodyLuck := BodyLuck + r;
   if (r < 0) and (BodyLuck > -(5 * BODYLUCKUNIT)) then
      BodyLuck := BodyLuck + r;

   n := Trunc (BodyLuck / BODYLUCKUNIT);
   if n > 5 then n := 5;
   if n < -10 then n := -10;
   BodyLuckLevel := n;
end;
*/
}

void CPlayerObject::WinExp(int nExp)
{
	m_Ability.Exp += nExp;

	AddBodyLuck (nExp * 0.002);

	AddProcess(this, RM_WINEXP, 0, nExp, 0, 0);

	while (m_Ability.Level + 1 < MAXLEVEL && m_Ability.MaxExp > 0 && m_Ability.Exp >= m_Ability.MaxExp)
	{
		m_Ability.Exp	-= m_Ability.MaxExp;
		m_Ability.Level++;
		HasLevelUp(m_Ability.Level - 1);

		AddBodyLuck (100);
/*      AddUserLog ('升级_' +
                  UserName + '_' +
                  IntToStr(Abil.Level) + '_' +
                  IntToStr(Abil.Exp)); */
	}
}

void CPlayerObject::GainExp(int nExp)
{
	WinExp(nExp);
}
/*
procedure TCreature.GainExp (exp: longword);
var
   i, n, sumlv, dexp: integer;
   cret: TCreature;
const
   bonus: array[0..GROUPMAX] of Real = (1, 1.2, 1.3, 1.4, 1.5, 1.6, 1.7, 1.8, 1.9, 2, 2.1, 2.2);
begin
   try
      if GroupOwner <> nil then begin
         sumlv := 0;
         n := 0;
         for i:=0 to GroupOwner.GroupMembers.Count-1 do begin
            cret := TCreature(GroupOwner.GroupMembers.Objects[i]);
            if not cret.Death and (PEnvir = cret.PEnvir) and (abs(CX-cret.CX) <= 12) and (abs(CY-cret.CY) <= 12) then begin
               sumlv := sumlv + cret.Abil.Level;
               Inc (n);
            end;
         end;
         if (sumlv > 0) and (n > 1) then begin
            if n in [0..GROUPMAX] then
               exp := Round (exp * bonus[n]);
            for i:=0 to GroupOwner.GroupMembers.Count-1 do begin
               cret := TCreature(GroupOwner.GroupMembers.Objects[i]);
               if not cret.Death and (PEnvir = cret.PEnvir) and (abs(CX-cret.CX) <= 12) and (abs(CY-cret.CY) <= 12) then
                  cret.WinExp (Round (exp / sumlv * cret.Abil.Level));
            end;
         end else
            WinExp (exp);
      end else
         WinExp (exp);
   except
      MainOutMessage ('[Exception] TCreature.GainExp');
   end;
end;
*/

void CPlayerObject::HasLevelUp(int nPrevLevel)
{
   m_Ability.MaxExp = NEEDEXPS[m_Ability.Level];

   RecalcLevelAbilitys();
/*
{$IFDEF FOR_ABIL_POINT}
//从 4/16 起适用
   if prevlevel + 1 = Abil.Level then begin
      BonusPoint := BonusPoint + GetBonusPoint (Job, Abil.Level);  //升级带来的奖励
      SendMsg (self, RM_ADJUST_BONUS, 0, 0, 0, 0, '');
   end else begin
      if prevlevel <> Abil.Level then begin
         //从头重新计算奖励点数。
         BonusPoint := GetLevelBonusSum (Job, Abil.Level);
         FillChar (BonusAbil, sizeof(TNakedAbility), #0);
         FillChar (CurBonusAbil, sizeof(TNakedAbility), #0);
         //if prevlevel <> 0 then begin
         RecalcLevelAbilitys;  //计算随等级变化的能力值。
         //end else begin
         //   RecalcLevelAbilitys_old;
         //   BonusPoint := 0;
         //end;
         SendMsg (self, RM_ADJUST_BONUS, 0, 0, 0, 0, '');
      end;
   end;
{$ENDIF}
*/
	RecalcAbilitys();
	
	AddProcess(this, RM_LEVELUP, 0, m_Ability.Exp, 0, 0, NULL);
}

void CPlayerObject::RecalcHitSpeed()
{
	m_nHitDouble = 0;

	/*HitPoint := DEFHIT + BonusAbil.Hit;*/
	m_btHitPoint = 5;

   /*case Job of
      2: SpeedPoint := DEFSPEED + BonusAbil.Speed + 3;  //道士的默认敏捷较高.
      else SpeedPoint := DEFSPEED + BonusAbil.Speed;
   end;*/
	if (m_pUserInfo->GetJob() == 2)
		m_btSpeedPoint = DEFSPEED + 3;
	else
		m_btSpeedPoint = DEFSPEED;

	m_pUserInfo->m_lpTMagicErgumSkill		= NULL;
	m_pUserInfo->m_lpTMagicBanwolSkill		= NULL;
	m_pUserInfo->m_lpTMagicFireSwordSkill	= NULL;
	m_pUserInfo->m_lpTMagicOneSwordSkill	= NULL;
	m_pUserInfo->m_lpTMagicIlkwangSkill		= NULL;
	m_pUserInfo->m_lpTMagicPowerHitSkill	= NULL;

	PLISTNODE pListNode = m_pUserInfo->m_lpTMagicRcd.GetHead();

	while (pListNode)
	{
		_LPTHUMANMAGICRCD	lptUserMagicRcd = m_pUserInfo->m_lpTMagicRcd.GetData(pListNode);

		switch (lptUserMagicRcd->btMagicID)
		{
			case _SKILL_ILKWANG:
			{
				m_pUserInfo->m_lpTMagicIlkwangSkill = lptUserMagicRcd;

				if (lptUserMagicRcd->btLevel > 0)
					m_btHitPoint += ROUND(8 / 3 * lptUserMagicRcd->btLevel);

				break;
			}
			case _SKILL_ERGUM:
				m_pUserInfo->m_lpTMagicErgumSkill = lptUserMagicRcd;
				break;
			case _SKILL_BANWOL:
				m_pUserInfo->m_lpTMagicBanwolSkill = lptUserMagicRcd;
				break;
			case _SKILL_FIRESWORD:
			{
				m_pUserInfo->m_lpTMagicFireSwordSkill = lptUserMagicRcd;
				m_nHitDouble = 4 + lptUserMagicRcd->btLevel * 4;  //+40% ~ +160%
				break;
			}
			case _SKILL_ONESWORD:
			{
				m_pUserInfo->m_lpTMagicOneSwordSkill = lptUserMagicRcd;

				if (lptUserMagicRcd->btLevel > 0)
					m_btHitPoint += ROUND(9 / 3 * lptUserMagicRcd->btLevel);

				break;
			}
			case _SKILL_YEDO:
			{
				m_pUserInfo->m_lpTMagicPowerHitSkill = lptUserMagicRcd;

				if (lptUserMagicRcd->btLevel > 0)
					m_btHitPoint += ROUND(3 / 3 * lptUserMagicRcd->btLevel);

				m_btHitPlus = 5 + lptUserMagicRcd->btLevel;

				m_pUserInfo->m_btAttackSkillCount		= 7 - lptUserMagicRcd->btLevel;
				m_pUserInfo->m_btAttackSkillPointCount	= rand() % m_pUserInfo->m_btAttackSkillCount;

				break;
			}
		}

		pListNode = m_pUserInfo->m_lpTMagicRcd.GetNext(pListNode);
	}
}

void CPlayerObject::RecalcAbilitys()
{
    const WORD currentHP = m_WAbility.HP, currentMP = m_WAbility.MP;
	BOOL fCGHI[4];

	m_btAntiPoison		= 0;
	m_btPoisonRecover	= 0;
	m_btHealthRecover	= 0;
	m_btSpellRecover	= 0;
	m_btAntiMagic		= 1;   //基本 10%
	
	m_sHitSpeed			= 0;

	fCGHI[0] = FALSE;
	fCGHI[1] = FALSE;
	fCGHI[2] = FALSE;
	fCGHI[3] = FALSE;

	BOOL fOldHumHideMode = m_fHumHideMode;

	ZeroMemory(&m_AddAbility, sizeof(m_AddAbility));

	m_WAbility = m_Ability;

	m_WAbility.Weight		= 0;
	m_WAbility.HandWeight	= 0;
	m_WAbility.WearWeight	= 0;

	_LPTUSERITEMRCD lpUserItemRcd = NULL;

	for (int i = U_DRESS; i <= U_RINGR; i++)
	{
		if (lpUserItemRcd = m_pUserInfo->GetAccessory(i))
		{
			g_pStdItemSpecial[lpUserItemRcd->nStdIndex].ApplyItemParameters(&m_AddAbility);

			if ( i == U_RIGHTHAND || i == U_WEAPON )
				m_WAbility.HandWeight += g_pStdItemSpecial[lpUserItemRcd->nStdIndex].wWeight;
			else
				m_WAbility.WearWeight += g_pStdItemSpecial[lpUserItemRcd->nStdIndex].wWeight; 

			if ( i == U_WEAPON )
				m_AddAbility.WeaponStrong = (BYTE)g_pStdItemSpecial[lpUserItemRcd->nStdIndex].dwRSource; // 武器的强度. 强度越高越不容易损坏
		}
	}

	m_WAbility.Weight	= m_pUserInfo->CalcBagWeight();

	// BEGIN:隐身术
	if ((m_fFixedHideMode) && (m_wStatusArr[STATE_TRANSPARENT] > 0))
		m_fHumHideMode = TRUE;

	if (m_fHumHideMode)
	{
		if (!fOldHumHideMode)
		{
			m_nCharStatus = GetCharStatus();
			AddRefMsg(RM_CHARSTATUSCHANGED, m_sHitSpeed/*wparam*/, m_nCharStatus, 0, 0, NULL);
		}
	}
	else
	{
		if (fOldHumHideMode)
		{
			m_wStatusArr[STATE_TRANSPARENT] = 0;
			m_nCharStatus = GetCharStatus();
			AddRefMsg(RM_CHARSTATUSCHANGED, m_sHitSpeed/*wparam*/, m_nCharStatus, 0, 0, NULL);
		}
	}
	// END:隐身术

	RecalcHitSpeed();

	// BEGIN:Light (Candle)
	BYTE btOldLight = m_btLight;

	_LPTGENERALITEMRCD lpTItemRcd = m_pUserInfo->GetRightHand();

	if (lpTItemRcd)
	{
		if (lpTItemRcd->nDura > 0)
			m_btLight = 8;
		else
			m_btLight = 2;
	}
	else
		m_btLight = 2;

	if (btOldLight != m_btLight)
		AddRefMsg(RM_CHANGELIGHT, 0, 0, 0, 0, NULL);
	// END
	
	m_btAntiMagic		+= m_AddAbility.AntiMagic;
	m_btHitPoint		+= m_AddAbility.HIT;
	m_btSpeedPoint		+= m_AddAbility.SPEED;
	m_btAntiPoison		+= m_AddAbility.AntiPoison;
	m_btPoisonRecover	+= m_AddAbility.PoisonRecover;
	m_btHealthRecover	+= m_AddAbility.HealthRecover;
	m_btSpellRecover	+= m_AddAbility.SpellRecover;
	m_sHitSpeed			+= m_AddAbility.HitSpeed;
//   Luck := Luck + AddAbil.Luck;
//   Luck := Luck - AddAbil.UnLuck;

	m_WAbility.MaxHP	= m_Ability.MaxHP + m_AddAbility.HP;
	m_WAbility.MaxMP	= m_Ability.MaxMP + m_AddAbility.MP;
    m_WAbility.HP = _MIN(currentHP, m_WAbility.MaxHP);
    m_WAbility.MP = _MIN(currentMP, m_WAbility.MaxMP);
	m_WAbility.AC		= MAKEWORD(LOBYTE(m_AddAbility.AC)  + LOBYTE(m_Ability.AC),  HIBYTE(m_AddAbility.AC)  + HIBYTE(m_Ability.AC));
	m_WAbility.MAC		= MAKEWORD(LOBYTE(m_AddAbility.MAC) + LOBYTE(m_Ability.MAC), HIBYTE(m_AddAbility.MAC) + HIBYTE(m_Ability.MAC));
	m_WAbility.DC		= MAKEWORD(LOBYTE(m_AddAbility.DC)  + LOBYTE(m_Ability.DC),  HIBYTE(m_AddAbility.DC)  + HIBYTE(m_Ability.DC));
	m_WAbility.MC		= MAKEWORD(LOBYTE(m_AddAbility.MC)  + LOBYTE(m_Ability.MC),  HIBYTE(m_AddAbility.MC)  + HIBYTE(m_Ability.MC));
	m_WAbility.SC		= MAKEWORD(LOBYTE(m_AddAbility.SC)  + LOBYTE(m_Ability.SC),  HIBYTE(m_AddAbility.SC)  + HIBYTE(m_Ability.SC));

	if (m_wStatusArr[STATE_DEFENCEUP] > 0) // 防御力上升
		m_WAbility.AC = MAKEWORD(LOBYTE(m_WAbility.AC), HIBYTE(m_WAbility.AC) + 2 + (m_Ability.Level / 7));

	if (m_wStatusArr[STATE_MAGDEFENCEUP] > 0) // 魔抗上升
		m_WAbility.MAC = MAKEWORD(LOBYTE(m_WAbility.MAC), HIBYTE(m_WAbility.MAC) + 2 + (m_Ability.Level / 7));

	UpdateProcess(this, RM_CHARSTATUSCHANGED, m_sHitSpeed/*wparam*/, m_nCharStatus, 0, 0, NULL);
}

void CPlayerObject::ServerGetMagicKeyChange(int nMagicID, int nUseKey)
{
	PLISTNODE pListNode = m_pUserInfo->m_lpTMagicRcd.GetHead();

	while (pListNode)
	{
		_LPTHUMANMAGICRCD	lptUserMagicRcd = m_pUserInfo->m_lpTMagicRcd.GetData(pListNode);

		if (lptUserMagicRcd)
		{
			if (lptUserMagicRcd->btMagicID == nMagicID)
			{
				lptUserMagicRcd->btUseKey	= nUseKey;
				break;
			}
		}

		pListNode = m_pUserInfo->m_lpTMagicRcd.GetNext(pListNode);
	}
}

void CPlayerObject::SendMyMagics()
{
	char				szEncodeMsg[4096];
	int					nPos = 0;

	_TDEFAULTMESSAGE	DefMsg;

	int nCnt = m_pUserInfo->EncodeMyMagic(szEncodeMsg, sizeof(szEncodeMsg), nPos);

	szEncodeMsg[nPos] = '\0';

	fnMakeDefMessage(&DefMsg, SM_SENDMYMAGIC, 0, 0, 0, nCnt);

	SendSocket(&DefMsg, szEncodeMsg);
}

void CPlayerObject::SendBagItems()
{
    std::string body; int count = 0;
    for (PLISTNODE node = m_pUserInfo->m_lpTItemRcd.GetHead(); node; node = m_pUserInfo->m_lpTItemRcd.GetNext(node)) {
        const auto item = m_pUserInfo->m_lpTItemRcd.GetData(node);
        Mir3Item::Record record;
        if (item && !IsEquipped(*m_pUserInfo, item->szMakeIndex) && BuildSpecialItem(*m_pUserInfo, *item, record)) {
            AppendItem(body, record); ++count;
        }
    }
    for (PLISTNODE node = m_pUserInfo->m_lpTGenItemRcd.GetHead(); node; node = m_pUserInfo->m_lpTGenItemRcd.GetNext(node)) {
        const auto item = m_pUserInfo->m_lpTGenItemRcd.GetData(node);
        Mir3Item::Record record;
        if (item && !IsEquipped(*m_pUserInfo, item->szMakeIndex) && BuildGeneralItem(*m_pUserInfo, *item, record)) {
            AppendItem(body, record); ++count;
        }
    }
    _TDEFAULTMESSAGE message;
    fnMakeDefMessage(&message, SM_BAGITEMS, (int)this, 0, 0, count);
    SendSocket(&message, const_cast<char*>(body.c_str()));
}

void CPlayerObject::SendEquipmentItems()
{
    std::string body;
    for (int slot = 0; slot < sizeof(m_pUserInfo->m_THumanRcd.szTakeItem) / Mir3Item::LegacyKeySize; ++slot) {
        char* key = m_pUserInfo->m_THumanRcd.szTakeItem[slot];
        Mir3Item::Record record;
        bool present = false;
        if (key[0] == 'G') {
            const auto item = m_pUserInfo->GetUseGenItem(key);
            present = item && BuildGeneralItem(*m_pUserInfo, *item, record);
        } else {
            const auto item = m_pUserInfo->GetItem(key);
            present = item && BuildSpecialItem(*m_pUserInfo, *item, record);
        }
        if (present) { body += std::to_string(slot) + '/'; AppendItem(body, record); }
    }
    _TDEFAULTMESSAGE message;
    fnMakeDefMessage(&message, SM_SENDUSEITEMS, 0, 0, 0, 0);
    SendSocket(&message, const_cast<char*>(body.c_str()));
}

void CPlayerObject::RecalcLevelAbilitys()
{
	int n;
	float nLevel = m_Ability.Level;

	switch (m_pUserInfo->m_THumanRcd.btJob)
	{
		case 2:		// 道士
		{
            m_Ability.MaxHP			= 14 + ROUND((nLevel / 6 + 2.5) * nLevel);
            m_Ability.MaxMP			= 13 + ROUND((nLevel / 8)* 2.2 * nLevel);
            m_Ability.MaxWeight		= 50 + ROUND((nLevel / 4) * nLevel);
            m_Ability.MaxWearWeight = 15 + ROUND((nLevel / 50) * nLevel);
            m_Ability.MaxHandWeight = 12 + ROUND((nLevel / 42) * nLevel);

            n = (int)nLevel / 7;
            
			m_Ability.DC			= MAKEWORD(_MAX(n-1, 0), _MAX(1, n));
            m_Ability.MC			= 0;
            m_Ability.SC			= MAKEWORD(_MAX(n-1, 0), _MAX(1, n));
            m_Ability.AC			= 0;
            
			n = ROUND(nLevel / 6);

            m_Ability.MAC			= MAKEWORD(n / 2, n + 1);
		
			break;
		}
		case 1:		// 术士
		{
            m_Ability.MaxHP			= 14 + ROUND((nLevel / 15 + 1.8) * nLevel);
            m_Ability.MaxMP			= 13 + ROUND((nLevel / 5 + 2) * 2.2 * nLevel);
            m_Ability.MaxWeight		= 50 + ROUND((nLevel / 5) * nLevel);
            m_Ability.MaxWearWeight = 15 + ROUND((nLevel / 100) * nLevel);
            m_Ability.MaxHandWeight = 12 + ROUND((nLevel / 90) * nLevel);

            n = (int)nLevel / 7;

			m_Ability.DC			= MAKEWORD(_MAX(n-1, 0), _MAX(1, n));
            m_Ability.MC			= MAKEWORD(_MAX(n-1, 0), _MAX(1, n));
            m_Ability.SC			= 0;
            m_Ability.AC			= 0;
            m_Ability.MAC			= 0;

			break;
		}
		case 0:		// 战士
		{
            m_Ability.MaxHP			= 14 + ROUND((nLevel / 4 + 4) * nLevel);
            m_Ability.MaxMP			= 11 + (int)(nLevel * 2);
            m_Ability.MaxWeight		= 50 + ROUND((nLevel / 3) * nLevel);
            m_Ability.MaxWearWeight = 15 + ROUND((nLevel / 20) * nLevel);
            m_Ability.MaxHandWeight = 12 + ROUND((nLevel / 13) * nLevel);
            m_Ability.DC			= MAKEWORD(_MAX((int)(nLevel / 7) - 1, 1), _MAX(1, int(nLevel / 5)));
            m_Ability.SC			= 0;
            m_Ability.MC			= 0;
            m_Ability.AC			= MAKEWORD(0, int(nLevel / 7));
            m_Ability.MAC			= 0;

			break;
		}
	}

	if (m_Ability.HP > m_Ability.MaxHP) m_Ability.HP = m_Ability.MaxHP;
	if (m_Ability.MP > m_Ability.MaxMP) m_Ability.MP = m_Ability.MaxMP;
}

void CPlayerObject::Initialize()
{
	if (!m_pUserInfo) return;

	m_dwLatestHitTime		= 0;
	m_nHitTimeOverCount		= 0;

	m_nCurrX		= m_pUserInfo->m_THumanRcd.nCX;
	m_nCurrY		= m_pUserInfo->m_THumanRcd.nCY;
	m_nDirection	= m_pUserInfo->m_THumanRcd.nDirection;
	m_pMap			= GetMap(m_pUserInfo->m_THumanRcd.szMapName);

	memcpy(m_szName, m_pUserInfo->m_szCharName, memlen(m_pUserInfo->m_szCharName));

	if (m_pMap)
	{
		AddProcess(this, RM_LOGON, 0, 0, 0, 0, NULL); 

		m_pMap->AddNewObject(m_nCurrX, m_nCurrY, OS_MOVINGOBJECT, this);

		AddRefMsg(RM_TURN, m_nDirection, m_nCurrX, m_nCurrY, 0, NULL);

		m_Ability.Level		= m_pUserInfo->m_THumanRcd.szLevel;
		m_Ability.Exp		= m_pUserInfo->m_THumanRcd.nExp;

		m_Ability.MaxExp	= NEEDEXPS[m_Ability.Level];

		RecalcLevelAbilitys();
        // The legacy character record stores no current HP/MP. Initialize this login explicitly.
        m_WAbility.HP = m_Ability.MaxHP;
        m_WAbility.MP = m_Ability.MaxMP;
		RecalcAbilitys();

		m_btBright = g_nMirDayTime;

		AddProcess(this, RM_ABILITY, 0, 0, 0, 0, NULL);
		AddProcess(this, RM_SUBABILITY, 0, 0, 0, 0, NULL);
		AddProcess(this, RM_DAYCHANGING, 0, 0, 0, 0, NULL);
		AddProcess(this, RM_SENDUSEITEMS, 0, 0, 0, 0, NULL);
		AddProcess(this, RM_SENDMYMAGIC, 0, 0, 0, 0, NULL);

		// BEGIN : Send Attack Mode Msg
		TCHAR	wszMsg[128];
		char	szMsg[128];

		LoadString(g_hInst, m_pUserInfo->GetAttackMode() + IDS_ATTACKMODE_ALL, wszMsg, sizeof(wszMsg)/sizeof(TCHAR));
		WideCharToMultiByte(CP_ACP, 0, wszMsg, -1, szMsg, sizeof(szMsg), NULL, NULL);
		SysMsg(szMsg, 1);

		LoadString(g_hInst, IDS_ATTACKMODE_CHGMODE, wszMsg, sizeof(wszMsg)/sizeof(TCHAR));
		WideCharToMultiByte(CP_ACP, 0, wszMsg, -1, szMsg, sizeof(szMsg), NULL, NULL);
		SysMsg(szMsg, 1);
		// END : Send Attack Mode Msg
	}
}
/*
void CPlayerObject::SendSocket(_LPTDEFAULTMESSAGE lpDefMsg, char *pszPacket)
{
	DWORD					dwBytesSends = 0;
	_TMSGHEADER				MsgHdr;

	MsgHdr.nCode			= 0xAA55AA55;
	MsgHdr.wIdent			= GM_DATA;
	MsgHdr.wUserGateIndex	= m_pUserInfo->m_nUserGateIndex;
	MsgHdr.wUserListIndex	= m_pUserInfo->m_nUserServerIndex;
	MsgHdr.nSocket			= m_pUserInfo->m_sock;

	if (lpDefMsg)
	{
		if (pszPacket)
		{
			int nLength = memlen(pszPacket);

			if (nLength >= 256) return;				// If pszPacket is too long.

			MsgHdr.nLength	= sizeof(_TDEFAULTMESSAGE) + nLength;

			m_pUserInfo->m_pGateInfo->OverlappedEx[1].DataBuf.len	= sizeof(_TMSGHEADER) + MsgHdr.nLength;

			memmove(m_pUserInfo->m_pGateInfo->OverlappedEx[1].Buffer, (char *)&MsgHdr, sizeof(_TMSGHEADER));
			memmove(&m_pUserInfo->m_pGateInfo->OverlappedEx[1].Buffer[sizeof(_TMSGHEADER)], lpDefMsg, sizeof(_TDEFAULTMESSAGE));
			memmove(&m_pUserInfo->m_pGateInfo->OverlappedEx[1].Buffer[sizeof(_TMSGHEADER) + sizeof(_TDEFAULTMESSAGE)], pszPacket, MsgHdr.nLength - sizeof(_TDEFAULTMESSAGE));
			
			m_pUserInfo->m_pGateInfo->OverlappedEx[1].Buffer[m_pUserInfo->m_pGateInfo->OverlappedEx[1].DataBuf.len] = '\0';
		}
		else
		{
			MsgHdr.nLength		= sizeof(_TDEFAULTMESSAGE);

			m_pUserInfo->m_pGateInfo->OverlappedEx[1].DataBuf.len = sizeof(_TMSGHEADER) + MsgHdr.nLength;

			memmove(m_pUserInfo->m_pGateInfo->OverlappedEx[1].Buffer, (char *)&MsgHdr, sizeof(_TMSGHEADER));
			memmove(&m_pUserInfo->m_pGateInfo->OverlappedEx[1].Buffer[sizeof(_TMSGHEADER)], lpDefMsg, sizeof(_TDEFAULTMESSAGE));
			m_pUserInfo->m_pGateInfo->OverlappedEx[1].Buffer[m_pUserInfo->m_pGateInfo->OverlappedEx[1].DataBuf.len] = '\0';
		}
	}
	else
	{
		int nLen		= (memlen(pszPacket) - 1);
		
		MsgHdr.nLength	= -(nLen);

		m_pUserInfo->m_pGateInfo->OverlappedEx[1].DataBuf.len = sizeof(_TMSGHEADER) + nLen;

		memmove(m_pUserInfo->m_pGateInfo->OverlappedEx[1].Buffer, (char *)&MsgHdr, sizeof(_TMSGHEADER));
		memmove(&m_pUserInfo->m_pGateInfo->OverlappedEx[1].Buffer[sizeof(_TMSGHEADER)], pszPacket, nLen);
		m_pUserInfo->m_pGateInfo->OverlappedEx[1].Buffer[m_pUserInfo->m_pGateInfo->OverlappedEx[1].DataBuf.len] = '\0';
	}

	m_pUserInfo->m_pGateInfo->OverlappedEx[1].nOvFlag		= OVERLAPPED_SEND;
	m_pUserInfo->m_pGateInfo->OverlappedEx[1].DataBuf.buf	= m_pUserInfo->m_pGateInfo->OverlappedEx[1].Buffer;

	WSASend(m_pUserInfo->m_pGateInfo->m_sock, &m_pUserInfo->m_pGateInfo->OverlappedEx[1].DataBuf, 1, &dwBytesSends, 0, (OVERLAPPED *)&m_pUserInfo->m_pGateInfo->OverlappedEx[1], NULL);
}
*/
BOOL CPlayerObject::DoRideHorse(int nDir, int fRideFlag)
{
//	if (fRideFlag == m_xAbility.m_fRideHorse)
//	{
/*		if (m_xAbility.m_fRideHorse)
			AddRefMsg(RM_RIDE, 0, 0, 0, 0, NULL);
		else
			AddRefMsg(RM_RIDE, 1, 0, 0, 0, NULL);
*/
//		return TRUE;
//	}

	return FALSE;
}

void CPlayerObject::MakeFeature()
{
	m_tFeature.btGender			= m_pUserInfo->GetGenderFeature();
	m_tFeature.btWear			= m_pUserInfo->GetDressFeature();
	m_tFeature.btHair			= m_pUserInfo->GetHairFeature();
	m_tFeature.btWeapon			= m_pUserInfo->GetWeaponFeature();

	m_tFeatureEx.btHorse		= m_pUserInfo->GetHorseFeatureEx();
	m_tFeatureEx.dwHairColor	= 0xFFFF;
	m_tFeatureEx.dwWearColor	= 0xFFFF;
}

BOOL CPlayerObject::CheckTakeOnItem(WORD wWhere, _LPTUSERITEMRCD lpTItemRcd)
{
	TCHAR	wszMsg[64];
	char	szMsg[64];

	int		nNeed		= g_pStdItemSpecial[lpTItemRcd->nStdIndex].wNeed;
	int		nNeedLevel	= g_pStdItemSpecial[lpTItemRcd->nStdIndex].wNeedLevel;;
	int		nWeight		= g_pStdItemSpecial[lpTItemRcd->nStdIndex].wWeight;

	if (wWhere == U_DRESS)
	{
		if (lpTItemRcd->szMakeIndex[0] != 'B') return FALSE;

		if (g_pStdItemSpecial[lpTItemRcd->nStdIndex].wStdMode == 10)	// 男装
		{
			if (m_tFeature.btGender != 0)
			{
				LoadString(g_hInst, IDS_MALE_WEAR, wszMsg, sizeof(wszMsg)/sizeof(TCHAR));
				WideCharToMultiByte(CP_ACP, 0, wszMsg, -1, szMsg, sizeof(szMsg), NULL, NULL);
				SysMsg(szMsg, 0);

				return FALSE;
			}
		}
		else if (g_pStdItemSpecial[lpTItemRcd->nStdIndex].wStdMode == 11)	// 女装
		{
			if (m_tFeature.btGender != 1)
			{
				LoadString(g_hInst, IDS_FEMALE_WEAR, wszMsg, sizeof(wszMsg)/sizeof(TCHAR));
				WideCharToMultiByte(CP_ACP, 0, wszMsg, -1, szMsg, sizeof(szMsg), NULL, NULL);
				SysMsg(szMsg, 0);

				return FALSE;
			}
		}
	}
	   
	if ((wWhere == U_WEAPON) || (wWhere == U_RIGHTHAND))
	{
		if (m_WAbility.MaxHandWeight < nWeight)
		{
			LoadString(g_hInst, IDS_NOTENOUGH_VALUE, wszMsg, sizeof(wszMsg)/sizeof(TCHAR));
			WideCharToMultiByte(CP_ACP, 0, wszMsg, -1, szMsg, sizeof(szMsg), NULL, NULL);
			SysMsg(szMsg, 0);

			return FALSE;
		}
	}
	else
	{
		if (m_WAbility.MaxWearWeight < nWeight + m_pUserInfo->CalcWearWeightEx(wWhere))
		{
			LoadString(g_hInst, IDS_NOTENOUGH_WEIGHT, wszMsg, sizeof(wszMsg)/sizeof(TCHAR));
			WideCharToMultiByte(CP_ACP, 0, wszMsg, -1, szMsg, sizeof(szMsg), NULL, NULL);
			SysMsg(szMsg, 0);

			return FALSE;
		}
	}

	switch (nNeed)
	{
		case 0:
			if (m_WAbility.Level < nNeedLevel) goto _NOTENOUGH_VALUE;
			break;
		case 1:
			if (HIBYTE(m_WAbility.DC) < nNeedLevel) goto _NOTENOUGH_VALUE;
			break;
		case 2:
			if (HIBYTE(m_WAbility.MC) < nNeedLevel) goto _NOTENOUGH_VALUE;
			break;
		case 3:
			if (HIBYTE(m_WAbility.SC) < nNeedLevel) goto _NOTENOUGH_VALUE;
			break;
	}

	return TRUE;

_NOTENOUGH_VALUE:

	LoadString(g_hInst, IDS_NOTENOUGH_VALUE, wszMsg, sizeof(wszMsg)/sizeof(TCHAR));
	WideCharToMultiByte(CP_ACP, 0, wszMsg, -1, szMsg, sizeof(szMsg), NULL, NULL);
	SysMsg(szMsg, 0);

	return FALSE;
}

void CPlayerObject::ServerGetTakeOnGenItem(WORD wWhere, char *pszItemIndex)
{
	_LPTGENERALITEMRCD	lpTGenItemRcd = NULL;
	_TDEFAULTMESSAGE	DefMsg;

	if (m_pUserInfo->m_lpTGenItemRcd.GetCount())
	{
		PLISTNODE pListNode = m_pUserInfo->m_lpTGenItemRcd.GetHead();

		while (pListNode)
		{
			lpTGenItemRcd = m_pUserInfo->m_lpTGenItemRcd.GetData(pListNode);
		
			if (memcmp(pszItemIndex, lpTGenItemRcd->szMakeIndex, _MAKEITEMINDEX) == 0)
			{
				memcpy(m_pUserInfo->m_THumanRcd.szTakeItem[wWhere], lpTGenItemRcd->szMakeIndex, _MAKEITEMINDEX);

				RecalcAbilitys();

				LONG lFeature = GetFeatureToLong();

				AddProcess(this, RM_ABILITY, 0, 0, 0, 0, NULL);
				AddProcess(this, RM_SUBABILITY, 0, 0, 0, 0, NULL);

				fnMakeDefMessage(&DefMsg, SM_TAKEON_OK, lFeature, 0, 0, 0);
				SendSocket(&DefMsg, NULL);

				AddRefMsg(RM_FEATURECHANGED, 0, lFeature, 0, 0, NULL);

				return;
			}

			pListNode = m_pUserInfo->m_lpTGenItemRcd.GetNext(pListNode);
		}
	}

	fnMakeDefMessage(&DefMsg, SM_TAKEON_FAIL, 2, 0, 0, 0);
	SendSocket(&DefMsg, NULL);
}

void CPlayerObject::ServerGetTakeOnItem(WORD wWhere, char *pszItemIndex)
{
	_TDEFAULTMESSAGE	DefMsg;

	_LPTUSERITEMRCD lpTItemRcd = m_pUserInfo->GetItem(pszItemIndex);
	
	if (lpTItemRcd)
	{
		if (CheckTakeOnItem(wWhere, lpTItemRcd))
		{
			memcpy(m_pUserInfo->m_THumanRcd.szTakeItem[wWhere], lpTItemRcd->szMakeIndex, _MAKEITEMINDEX);

			switch (wWhere)
			{
				case U_DRESS:
					m_tFeature.btWear = m_pUserInfo->GetDressFeature();
					break;
				case U_WEAPON:
					m_tFeature.btWeapon	= m_pUserInfo->GetWeaponFeature();
					break;
			}

			RecalcAbilitys();

			LONG lFeature = GetFeatureToLong();

			AddProcess(this, RM_ABILITY, 0, 0, 0, 0, NULL);
			AddProcess(this, RM_SUBABILITY, 0, 0, 0, 0, NULL);

			fnMakeDefMessage(&DefMsg, SM_TAKEON_OK, lFeature, 0, 0, 0);
			SendSocket(&DefMsg, NULL);

			AddRefMsg(RM_FEATURECHANGED, 0, lFeature, 0, 0, NULL);

			return;
		}
		else
		{
			fnMakeDefMessage(&DefMsg, SM_TAKEON_FAIL, 1, 0, 0, 0);
			SendSocket(&DefMsg, NULL);

			return;
		}
	}

	fnMakeDefMessage(&DefMsg, SM_TAKEON_FAIL, 2, 0, 0, 0);
	SendSocket(&DefMsg, NULL);
}

void CPlayerObject::ServerGetTakeOffItem(WORD wWhere, char *pszItemIndex)
{
	_LPTUSERITEMRCD		lpTItemRcd = NULL;
	_TDEFAULTMESSAGE	DefMsg;

//		PLISTNODE pListNode = m_pUserInfo->m_lpTItemRcd.GetHead();

//		while (pListNode)
//		{
//			lpTItemRcd = m_pUserInfo->m_lpTItemRcd.GetData(pListNode);
		
	if (m_pUserInfo->IsEnoughBag() &&
        memcmp(m_pUserInfo->m_THumanRcd.szTakeItem[wWhere], pszItemIndex, _MAKEITEMINDEX) == 0)
	{
		ZeroMemory(m_pUserInfo->m_THumanRcd.szTakeItem[wWhere], _MAKEITEMINDEX);
		memcpy(m_pUserInfo->m_THumanRcd.szTakeItem[wWhere], "0", 2);

		switch (wWhere)
		{
			case U_DRESS:
				m_tFeature.btWear = m_pUserInfo->GetDressFeature();
				break;
			case U_WEAPON:
				m_tFeature.btWeapon	= m_pUserInfo->GetWeaponFeature();
				break;
		}

		RecalcAbilitys();

		LONG lFeature = GetFeatureToLong();

		AddProcess(this, RM_ABILITY, 0, 0, 0, 0, NULL);
		AddProcess(this, RM_SUBABILITY, 0, 0, 0, 0, NULL);

		fnMakeDefMessage(&DefMsg, SM_TAKEOFF_OK, lFeature, 0, 0, 0);
		SendSocket(&DefMsg, NULL);

		AddRefMsg(RM_FEATURECHANGED, 0, lFeature, 0, 0, NULL);
	}
	else
	{
		fnMakeDefMessage(&DefMsg, SM_TAKEOFF_FAIL, 1, 0, 0, 0);
		SendSocket(&DefMsg, NULL);
	}
}

void CPlayerObject::SendMapName()
{
	if (m_pMap)
	{
		_TDEFAULTMESSAGE	DefMsg;
		char				szEncodeMsg[256];

		fnMakeDefMessage(&DefMsg, SM_MAPDESCRIPTION, 0, 0, 0, 0);

		int nPos = fnEncode6BitBufA((unsigned char *)m_pMap->m_szMapTextName, szEncodeMsg, memlen(m_pMap->m_szMapTextName) - 1, sizeof(szEncodeMsg));
		szEncodeMsg[nPos] = '\0';

		SendSocket(&DefMsg, szEncodeMsg);
	}
}

void CPlayerObject::SendNewMap()
{
	if (m_pMap)
	{
		_TDEFAULTMESSAGE	DefMsg;
		char				szEncodeMsg[256];
																			
		fnMakeDefMessage(&DefMsg, SM_NEWMAP, (int)this, m_nCurrX, m_nCurrY, MAKEWORD(DayBright(), m_pMap->m_btSeries));

		int nPos = fnEncode6BitBufA((unsigned char *)m_pMap->m_szMapName, szEncodeMsg, memlen(m_pMap->m_szMapName) - 1, sizeof(szEncodeMsg));
		szEncodeMsg[nPos] = '\0';

		SendSocket(&DefMsg, szEncodeMsg);
	}
}

BYTE CPlayerObject::DayBright()
{
	int n;
//               begin
//                  if PEnvir.Darkness then n := 1
//                  else
	if (m_btBright == 1)
		n = 0;
	else if (m_btBright == 3)
		n = 1;
	else
		n = 2;

/*                  if PEnvir.DayLight then n := 0; */

	return n;
}

void CPlayerObject::Run()
{
	if (RestoreHealSpell())
		Die();

	// 召唤对象检查
	if (m_xSlaveObjList.GetCount())
	{
		PLISTNODE pListNode = m_xSlaveObjList.GetHead();
	
		while (pListNode)
		{
			CCharObject* pCharObject = m_xSlaveObjList.GetData(pListNode);

			if (pCharObject)
			{
				if (pCharObject->m_fIsDead || pCharObject->m_fIsGhost || pCharObject->m_pMasterObject != this)
					pListNode = m_xSlaveObjList.RemoveNode(pListNode);
				else
					pListNode = m_xSlaveObjList.GetNext(pListNode);
			}
		}
	}

	CCharObject::Run();
}

WORD CPlayerObject::GetThisCharColor()
{
	return _CHAT_COLOR3;
}

void CPlayerObject::ServerGetButch(CCharObject* pCharObject, int nX, int nY, int nDir)
{
	if ((abs(nX - m_nCurrX) <= 2) && (abs(nY - m_nCurrY) <= 2))
	{
		if (m_pMap->IsValidObject(nX, nY, 2, pCharObject))
		{
			CAnimalObject* pAnimalObject = dynamic_cast<CAnimalObject*>(pCharObject);

			if (pAnimalObject)
			{
				if (pAnimalObject->m_fIsDead && !pAnimalObject->m_fSkeleton)
				{
					int n = 5 + rand() % 16;
					int m = 100 + rand() % 201;
												
					pAnimalObject->m_nMeatQuality	-= n;
					pAnimalObject->m_nBodyLeathery	-= m;

					if (pAnimalObject->m_nMeatQuality < 0) 
						pAnimalObject->m_nMeatQuality = 0;

					if (pAnimalObject->m_nBodyLeathery <= 0)
					{
	//               if (animal.RaceServer >= RC_ANIMAL) and (animal.RaceServer < RC_MONSTER) then begin  //像鹿一样给肉的才变成骷髅
	//                  animal.BoSkeleton := TRUE;
	//                  animal.ApplyMeatQuality;
	//                  animal.SendRefMsg (RM_SKELETON, animal.Dir, animal.CX, animal.CY, 0, '')
	//               end;
						pAnimalObject->m_fSkeleton = TRUE;
						pAnimalObject->AddRefMsg(RM_SKELETON, pAnimalObject->m_nDirection, pAnimalObject->m_nCurrX, pAnimalObject->m_nCurrY, 0, NULL);

//						if (!pAnimalObject->TakeCretBagItems())
//							SysMag("什么也没有得到。", 0);

						pAnimalObject->m_nBodyLeathery = 50;
					}

					m_dwDeathTime = GetTickCount();
				}
			}
		}

		m_nDirection = nDir;
	}

	AddRefMsg(RM_BUTCH, nDir, m_nCurrX, m_nCurrY, 0, NULL);
}

BOOL CPlayerObject::SpellXY(int nMagicID, int nTargetX, int nTargetY, int nTargetObj)
{
	if (!m_pUserInfo) return FALSE;
    constexpr int TargetPositionTolerance=2;
    if(nTargetObj && !m_pMap->IsValidObject(nTargetX,nTargetY,TargetPositionTolerance,(CCharObject*)nTargetObj)) return FALSE;

	_LPTHUMANMAGICRCD	lptMagicRcd = m_pUserInfo->GetMagicRcdByID(nMagicID);
	
	if (lptMagicRcd)
	{
		CMagicInfo*	pMagicInfo	= GetMagicInfo(lptMagicRcd->btMagicID);

		if (pMagicInfo)
		{
			int nSpellPoint = pMagicInfo->GetSpellPoint(lptMagicRcd->btLevel);

			switch (lptMagicRcd->btMagicID)
			{
				case _SKILL_ERGUM:
				{
					if (!m_pUserInfo->m_fLongHitSkill)
					{
						m_pUserInfo->AllowLongHitSkill(TRUE);
						SendSocket(NULL, "+LNG");
					}
					else
					{
						m_pUserInfo->AllowLongHitSkill(FALSE);
						SendSocket(NULL, "+ULNG");
					}

					return TRUE;
				}
				case _SKILL_BANWOL:
				{
					if (!m_pUserInfo->m_fWideHitSkill)
					{
						m_pUserInfo->AllowWideHitSkill(TRUE);
						SendSocket(NULL, "+WID");
					}
					else
					{
						m_pUserInfo->AllowWideHitSkill(FALSE);
						SendSocket(NULL, "+UWID");
					}

					return TRUE;
				}
				case _SKILL_FIRESWORD:
				{
					if (m_pUserInfo->AllowFireHitSkill())
					{
						DamageSpell(nSpellPoint);
						HealthSpellChanged();

						SendSocket(NULL, "+FIR");
					}

					return TRUE;
				}
			}

			if (m_WAbility.MP >= nSpellPoint)
			{
				DamageSpell(nSpellPoint);
				HealthSpellChanged();

				GetNextDirection(nTargetX, nTargetY);

				switch (lptMagicRcd->btMagicID)
				{
					case _SKILL_HANGMAJINBUB:
					case _SKILL_DEJIWONHO:
					case _SKILL_FIRECHARM:
					case _SKILL_FIRE:
					case _SKILL_FIREBALL2:
					case _SKILL_FIREBALL:
					case _SKILL_BIGCLOAK:
					case _SKILL_CLOAK:
						AddRefMsg(RM_SPELL2, pMagicInfo->sEffect, nTargetX, nTargetY, pMagicInfo->nIndex, NULL);
						break;
					case _SKILL_MOOTEBO:
						DoMotaebo();
						return TRUE;
					default:
						AddRefMsg(RM_SPELL, pMagicInfo->sEffect, nTargetX, nTargetY, pMagicInfo->nIndex, NULL);
						break;
				}

				return DoSpell(lptMagicRcd, nTargetX, nTargetY, (CCharObject*)nTargetObj);
			}
		} // if (pMagicInfo)
	}

	return FALSE;	
}

BOOL CPlayerObject::DoSpell(_LPTHUMANMAGICRCD	lptMagicRcd, int TargetX, int TargetY, CCharObject* pTargetObject)
{
	BOOL fTrain = FALSE;

	CMagicInfo*	pMagicInfo	= GetMagicInfo(lptMagicRcd->btMagicID);

	if (!pMagicInfo) return FALSE;

	switch (lptMagicRcd->btMagicID)
	{
		case _SKILL_SHOWHP:			// 探气破连
		{
			if (!DoShowHP(pTargetObject, pMagicInfo, lptMagicRcd->btLevel))
				return FALSE;
		
			break;
		}
		case _SKILL_HEALLING:		// 恢复术
		{
			if (!pTargetObject)
			{
				pTargetObject = (CCharObject*)this;

				TargetX = m_nCurrX;
				TargetY = m_nCurrY;
			}

			int nPwr = GetAttackPower(pMagicInfo->GetPower(pMagicInfo->MPow() + LOBYTE(m_WAbility.SC) * 2, lptMagicRcd->btLevel), 
										(HIBYTE(m_WAbility.SC) - LOBYTE(m_WAbility.SC)) * 2 + 1);

			if (pTargetObject->m_WAbility.HP < pTargetObject->m_WAbility.MaxHP)
			{
				pTargetObject->AddDelayProcess(this, RM_MAGHEALING, 0, nPwr, 0, 0, NULL, 800);
#ifdef _DEBUG
				char szMsg[64];

				sprintf(szMsg, "%s healed - PW:%d HP:%d", pTargetObject->m_szName, nPwr, pTargetObject->m_WAbility.HP);
				SysMsg(szMsg, 0);
#endif
			}

			break;
		}
		case _SKILL_BIGHEALLING:	// 大恢复术
		{
			int nPwr = GetAttackPower(pMagicInfo->GetPower(pMagicInfo->MPow() + LOBYTE(m_WAbility.SC) * 2, lptMagicRcd->btLevel), 
										(HIBYTE(m_WAbility.SC) - LOBYTE(m_WAbility.SC)) * 2 + 1);
			
			if (MagBigHealing(nPwr, TargetX, TargetY))
				fTrain = TRUE;
			
			break;
		}
		case _SKILL_LIGHTENING:		// 强击
		{
			if (IsProperTarget(pTargetObject))
			{
				if (pTargetObject->m_btAntiMagic <= (rand() % 10))
				{
					int nPwr = GetAttackPower(pMagicInfo->GetPower(pMagicInfo->MPow(), lptMagicRcd->btLevel) + LOBYTE(m_WAbility.MC), 
												HIBYTE(m_WAbility.MC) - LOBYTE(m_WAbility.MC) + 1);

					if (pTargetObject->m_wObjectType & _OBJECT_ANIMAL)
					{
						fTrain = TRUE;

						if (((CMonsterObject*)pTargetObject)->m_btLifeAttrib == LA_UNDEAD)
							nPwr = ROUND(nPwr * 1.5);
					}

					AddDelayProcess(this, RM_DELAYMAGIC, nPwr, MAKELONG(TargetX, TargetY), 2, (int)pTargetObject, NULL, 1400);
				}
				else
					pTargetObject = NULL;
			}

			break;
		}
		case _SKILL_FIREBALL:		// 火焰场
		case _SKILL_FIREBALL2:		// 金刚火焰掌 
		{
			int nPwr = GetAttackPower(pMagicInfo->GetPower(pMagicInfo->MPow(), lptMagicRcd->btLevel) + LOBYTE(m_WAbility.MC), 
										HIBYTE(m_WAbility.MC) - LOBYTE(m_WAbility.MC) + 1);

			AddDelayProcess(this, RM_DELAYMAGIC, nPwr, MAKELONG(TargetX, TargetY), 2, (int)pTargetObject, NULL, 900);
/*         if user.MagCanHitTarget (user.CX, user.CY, target) then begin
    if user.IsProperTarget (target) then begin
       if (target.AntiMagic <= Random(10)) and (abs(target.CX-xx) <= 1) and (abs(target.CY-yy) <= 1) then begin
          with user do begin
             pwr := GetAttackPower (
                         GetPower (MPow(pum)) + Lobyte(WAbil.MC),
                         ShortInt(Hibyte(WAbil.MC)-Lobyte(WAbil.MC)) + 1
                      );
             //pwr := GetPower (MPow(pum)) + (Lobyte(WAbil.MC) + Random(Hibyte(WAbil.MC)-Lobyte(WAbil.MC) + 1));
             //目标被击中, 之后出现效果
             //target.SendDelayMsg (user, RM_MAGSTRUCK, 0, pwr, 0, 0, '', 1200 + _MAX(Abs(CX-xx),Abs(CY-yy)) * 50 );
          end;
          //在 rm-delaymagic 中处理 selecttarget。
          user.SendDelayMsg (user, RM_DELAYMAGIC, pwr, MakeLong(xx, yy), 2, integer(target), '', 600);
          if (target.RaceServer >= RC_ANIMAL) then train := TRUE;
       end else
          target := nil;
    end else
       target := nil;
 end else
    target := nil; */
			break;
		}
		case _SKILL_FIREWIND:		// 火焰风
		{
			if (MagPushArround(3) > 0) fTrain = TRUE;

			break;
		}
		case _SKILL_FIRE:			// 焰蛇将
		{
			int nX = 0, nY = 0;

			int nDir = GetNextDirection(TargetX, TargetY);

			if (GetNextPosition(nDir, 1, nX, nY))
			{
				GetNextPosition(nDir, 5, TargetX, TargetY);

				int nPwr = GetAttackPower(pMagicInfo->GetPower(pMagicInfo->MPow(), lptMagicRcd->btLevel) + LOBYTE(m_WAbility.MC), 
											HIBYTE(m_WAbility.MC) - LOBYTE(m_WAbility.MC) + 1);
				
				if (MagPassThroughMagic(nX, nY, TargetX, TargetY, nDir, nPwr, FALSE) > 0)
					fTrain = TRUE;
			}

			break;
		}
		case _SKILL_SHOOTLIGHTEN:	// 雷印掌
		{
			int nX = 0, nY = 0;

			int nDir = GetNextDirection(TargetX, TargetY);

			if (GetNextPosition(nDir, 1, nX, nY))
			{
				GetNextPosition(nDir, 8, TargetX, TargetY);

				int nPwr = GetAttackPower(pMagicInfo->GetPower(pMagicInfo->MPow(), lptMagicRcd->btLevel) + LOBYTE(m_WAbility.MC), 
											HIBYTE(m_WAbility.MC) - LOBYTE(m_WAbility.MC) + 1);
				
				if (MagPassThroughMagic(nX, nY, TargetX, TargetY, nDir, nPwr, TRUE) > 0)
					fTrain = TRUE;
			}

			break;
		}
		case _SKILL_SHIELD:		// 咒术之幕
		{
			if (MagBubbleDefenceUp(lptMagicRcd->btLevel, pMagicInfo->GetPower(15 + GetRPow(m_WAbility.MC), lptMagicRcd->btLevel)))
				fTrain = TRUE;

			break;
		}
		case _SKILL_EATTHFIRE:	// 地炎术
		{
			int nPwr = GetAttackPower(pMagicInfo->GetPower(pMagicInfo->MPow(), lptMagicRcd->btLevel) + LOBYTE(m_WAbility.MC), 
										HIBYTE(m_WAbility.MC) - LOBYTE(m_WAbility.MC) + 1);

			if (MagMakeFireCross(nPwr, pMagicInfo->GetPower(10, lptMagicRcd->btLevel) + GetRPow(m_WAbility.MC) / 2, TargetX, TargetY))
				fTrain = TRUE;

			break;
		}
		case _SKILL_FIREBOOM:	// 爆裂波
		case _SKILL_SNOWWIND:	// 冰雪风
		{
			if (MagBigExplosion(GetAttackPower(pMagicInfo->GetPower(pMagicInfo->MPow(), lptMagicRcd->btLevel) + LOBYTE(m_WAbility.MC), 
												HIBYTE(m_WAbility.MC) - LOBYTE(m_WAbility.MC) + 1), 
								TargetX, TargetY, 1))
				fTrain = TRUE;

			break;
		}
		case _SKILL_AMYOUNSUL:
		case _SKILL_HANGMAJINBUB:
		case _SKILL_DEJIWONHO:
		case _SKILL_CLOAK:
		case _SKILL_FIRECHARM:
		case _SKILL_HOLYSHIELD:
		case _SKILL_BIGCLOAK:
		case _SKILL_SKELLETON:
		{
			if (_LPTGENERALITEMRCD lptItem = m_pUserInfo->CanUseBujuk())
			{
				if (lptItem->nDura >= 1)
				{
					lptItem->nDura--;

					AddProcess(this, RM_DURACHANGE, U_ARMRINGL, lptItem->nDura, lptItem->nDuraMax, 0);

					switch (lptMagicRcd->btMagicID)
					{
						case _SKILL_HANGMAJINBUB:	// 降魔阵法
						{
							int nPwr = GetAttackPower(pMagicInfo->GetPower13(60, lptMagicRcd->btLevel) + 5 * LOBYTE(m_WAbility.SC),
														5 * (HIBYTE(m_WAbility.SC) - LOBYTE(m_WAbility.SC)) + 1);

							if (MagMakeDefenceArea(TargetX, TargetY, 3, nPwr, STATE_MAGDEFENCEUP))
								fTrain = TRUE;

							break;
						}
						case _SKILL_DEJIWONHO:		// 大地援护
						{
							int nPwr = GetAttackPower(pMagicInfo->GetPower13(60, lptMagicRcd->btLevel) + 5 * LOBYTE(m_WAbility.SC),
														5 * (HIBYTE(m_WAbility.SC) - LOBYTE(m_WAbility.SC)) + 1);

							if (MagMakeDefenceArea(TargetX, TargetY, 3, nPwr, STATE_DEFENCEUP))
								fTrain = TRUE;

							break;
						}
						case _SKILL_CLOAK:			// 隐身术
						{
							if (MagMakePrivateTransparent(pMagicInfo->GetPower13(30, lptMagicRcd->btLevel) + 3 * GetRPow(m_WAbility.SC)))
								fTrain = TRUE;

							break;
						}
						case _SKILL_BIGCLOAK:		// 大隐身术
						{
							if (MagMakeGroupTransparent(TargetX, TargetY, pMagicInfo->GetPower13(30, lptMagicRcd->btLevel) + 3 * GetRPow(m_WAbility.SC)))
								fTrain = TRUE;

							break;
						}
						case _SKILL_FIRECHARM:		// 爆杀系
						{
							int nPwr = GetAttackPower(pMagicInfo->GetPower(pMagicInfo->MPow(), lptMagicRcd->btLevel) + LOBYTE(m_WAbility.SC), 
														HIBYTE(m_WAbility.SC) - LOBYTE(m_WAbility.SC) + 1);

							AddDelayProcess(this, RM_DELAYMAGIC, nPwr, MAKELONG(TargetX, TargetY), 2, (int)pTargetObject, NULL, 900);

							break;
						}
						case _SKILL_HOLYSHIELD:		// 结界
						{
							if (MagMakeHolyCurtain(pMagicInfo->GetPower13(40, lptMagicRcd->btLevel) + 3 * GetRPow(m_WAbility.SC), TargetX, TargetY) > 0)
								fTrain = TRUE;

							break;
						}
						case _SKILL_SKELLETON:		// 白骨召唤术
						{
							int nX, nY;

							GetFrontPosition(nX, nY);

							if (MakeSlave(94, nX, nY, lptMagicRcd->btLevel, 1, 48 * 60 * 60))
								fTrain = TRUE;

							break;
						}
					}
				}
				else
					return FALSE;
			}
			else
				return FALSE;

			break;
		}
		case _SKILL_SINSU:			// 神兽召唤
		{
			if (_LPTGENERALITEMRCD lptItem = m_pUserInfo->CanUseBujuk())
			{
				if (lptItem->nDura >= 5)
				{
					lptItem->nDura -= 5;

					AddProcess(this, RM_DURACHANGE, U_ARMRINGL, lptItem->nDura, lptItem->nDuraMax, 0);

					if (GetAvailablePosition(m_pMap, TargetX, TargetY, 3))
					{
						AddDelayProcess(this, RM_MAKESLAVE, 161, TargetX, TargetY, lptMagicRcd->btLevel, NULL, 1200);
						fTrain = TRUE;
					}
					else
						return FALSE;
				}
				else
					return FALSE;
			}
			else
				return FALSE;

			break;
		}
		case _SKILL_KILLUNDEAD:
		{
			if (pTargetObject)
			{
				if (IsProperTarget(pTargetObject))
				{
	//				if (MagTurnUnDead(pTargetObject, TargetX, TargetY, lptMagicRcd->btLevel))
					MagTurnUndead(pTargetObject, TargetX, TargetY, lptMagicRcd->btLevel);
						fTrain = TRUE;
				}
			}

			break;
		}
		case _SKILL_TAMMING:
			break;
		case _SKILL_LIGHTFLOWER:
		{
			if (MagElecBlizzard(GetAttackPower(pMagicInfo->GetPower(pMagicInfo->MPow(), lptMagicRcd->btLevel) + LOBYTE(m_WAbility.MC),
										(HIBYTE(m_WAbility.MC) - LOBYTE(m_WAbility.MC)) + 1)))
				fTrain = TRUE;

			break;
		}
	}

/*   if not nofire then begin
      with user do begin
         if needfire then
            SendRefMsg (RM_MAGICFIRE, 0, MakeWord(pum.pDef.EffectType, pum.pDef.Effect), MakeLong(xx, yy), integer(target), '');
         //被击中的对方
         if (pum.Level < 3) and train then
            if Abil.Level >= pum.pDef.NeedLevel[pum.Level] then begin
               //达到修炼等级的情况
               user.TrainSkill (pum, 1 + Random(3));
               if not CheckMagicLevelup (pum) then
                  SendDelayMsg (user, RM_MAGIC_LVEXP, 0, pum.pDef.MagicId, pum.Level, pum.CurTrain, '', 1000);
            end;
      end;
      Result := TRUE;
   end; */

	AddRefMsg(RM_MAGICFIRE, 0, lptMagicRcd->btMagicID, MAKELONG(TargetX, TargetY), (LONG)pTargetObject, NULL);

	if (lptMagicRcd->btLevel < 3)
	{
		if (m_Ability.Level >= pMagicInfo->sNeed[lptMagicRcd->btLevel])
			TrainSkill(lptMagicRcd, 1 + (rand() % 3));
	}

	return TRUE;
}

void CPlayerObject::TrainSkill(_LPTHUMANMAGICRCD lptMagicRcd, int nTrain)
{
	if (m_fFastTrain)
		lptMagicRcd->nCurrTrain += (nTrain * 3);
	else
		lptMagicRcd->nCurrTrain += nTrain;
}

BOOL CPlayerObject::MakeSlave(int nMonRace, int nX, int nY, int nLevel, int nMax, DWORD dwRoyaltySec)
{
	CMonRaceInfo* pMonRaceInfo = &g_pMonRaceInfo[nMonRace];

	if (pMonRaceInfo)
	{
		CMonsterObject* pMonsterObject = (CMonsterObject*)AddCreatureSysop(nX, nY, pMonRaceInfo, FALSE);

		if (pMonsterObject)
		{
			pMonsterObject->m_pMasterObject = this;
		
			m_xSlaveObjList.AddNewNode(pMonsterObject);
/*         mon.MasterRoyaltyTime := GetTickCount + longword(royaltysec) * 1000;
         mon.SlaveMakeLevel := slevel;
         mon.SlaveExpLevel := slevel;
         mon.RecalcAbilitys; //ApplySlaveLevelAbilitys;
         if mon.WAbil.HP < mon.WAbil.MaxHP then begin
            mon.WAbil.HP := mon.WAbil.HP + (mon.WAbil.MaxHP - mon.WAbil.HP) div 2;
         end;
         mon.ChangeNameColor;
         SlaveList.Add (mon);
         Result := mon;
 */
			return TRUE;
		}
	}

	return FALSE;
}

void CPlayerObject::Operate()
{
	if (!m_pUserInfo) return;

	if (m_pUserInfo->m_fFireHitSkill)
	{
		if (GetTickCount() - m_pUserInfo->m_dwLatestFireHitTime > 20 * 1000)
		{
			TCHAR	szText[64];
			char	szMsg[64];

			m_pUserInfo->m_fFireHitSkill = FALSE;

			LoadString(g_hInst, IDS_TIMEOUT_FIREHIT, szText, sizeof(szText)/sizeof(TCHAR));
			WideCharToMultiByte(CP_ACP, 0, szText, -1, szMsg, sizeof(szMsg), NULL, NULL);
			SysMsg(szMsg, 0);
			
			SendSocket(NULL, "+UFIR");
		}
	}

	if (m_nEvent >= 0)
	{
		if ((m_nEvent >= 0) && (m_nEvent < g_nNumOfMoveMapEventInfo))
		{
			CMirMap* pMirMap = GetMap(g_pMoveMapEventInfo[m_nEvent].szDMapFileName);

			if (pMirMap)
				SpaceMove(g_pMoveMapEventInfo[m_nEvent].nDX, g_pMoveMapEventInfo[m_nEvent].nDY, pMirMap);

			m_nEvent = -1;
		}
	}

	Run();

	int		nCount = m_DelayProcessQ.GetCount();
	int		nPos;

	_TDEFAULTMESSAGE	DefMsg;
	_LPTPROCESSMSG		lpProcessMsg;

	if (nCount)
	{
		for (int i = 0; i < nCount; i++)
		{
			lpProcessMsg = (_LPTPROCESSMSG)m_DelayProcessQ.PopQ();

			if (lpProcessMsg)
			{
				if (GetTickCount() < lpProcessMsg->dwDeliveryTime)
				{
					m_DelayProcessQ.PushQ((BYTE *)lpProcessMsg);
					continue;
				}

				switch (lpProcessMsg->wIdent)
				{
					case RM_MAGIC_LVEXP:
					{						
						fnMakeDefMessage(&DefMsg, SM_MAGIC_LVEXP, lpProcessMsg->lParam3, 
											(WORD)lpProcessMsg->lParam2, (WORD)lpProcessMsg->lParam1, 0);
						SendSocket(&DefMsg, NULL);

						break;
					}
					case RM_STRUCK:
					{
						lpProcessMsg->pCharObject->AddRefMsg(RM_STRUCK, lpProcessMsg->wParam, lpProcessMsg->lParam1, 
																		lpProcessMsg->lParam2, lpProcessMsg->lParam3, NULL);
						break;
					}
					case RM_DOOPENHEALTH:
					{
						MakeOpenHealth();
						break;
					}
					case RM_MAGHEALING:
					{
						if (m_IncHealing + lpProcessMsg->lParam1 < 300)
						{
							m_IncHealing	+= (BYTE)lpProcessMsg->lParam1;
							m_btPerHealing	= 5;
						}
						else
						{
							m_IncHealing	= 300;
						}

						break;
					}
					case RM_DELAYMAGIC:
					{
						CCharObject* pTargetObject = (CCharObject*)lpProcessMsg->lParam3;
						int nMagPower = lpProcessMsg->wParam;

						if (pTargetObject && m_pMap->IsValidObject(LOWORD(lpProcessMsg->lParam1),HIWORD(lpProcessMsg->lParam1),int(lpProcessMsg->lParam2),pTargetObject))
						{
							int n = pTargetObject->GetMagStruckDamage(nMagPower);

							if (n > 0)
							{
								SelectTarget(pTargetObject);

								if (pTargetObject->m_wObjectType & _OBJECT_ANIMAL)
									nMagPower = ROUND(nMagPower * 1.2);

								// Debug Code
								char szMsg[64];

								sprintf(szMsg, "%s hit by magic - PW:%d", pTargetObject->m_szName, nMagPower);
								SysMsg(szMsg, 0);
								// Debug Code

								if ((abs(LOWORD(lpProcessMsg->lParam1) - pTargetObject->m_nCurrX) <= (int)lpProcessMsg->lParam2) &&
									(abs(HIWORD(lpProcessMsg->lParam1) - pTargetObject->m_nCurrY) <= (int)lpProcessMsg->lParam2))
									pTargetObject->AddProcess(this, RM_MAGSTRUCK, 0, nMagPower, 0, 0, NULL);
							}
						}

						break;
					}
					case RM_MAGSTRUCK:
					{
						AddProcess(lpProcessMsg->pCharObject, lpProcessMsg->wIdent, lpProcessMsg->wParam, lpProcessMsg->lParam1, 
										lpProcessMsg->lParam2, lpProcessMsg->lParam3, NULL);
						break;
					}
					case RM_TRANSPARENT:
					{
						lpProcessMsg->pCharObject->MagMakePrivateTransparent(lpProcessMsg->lParam1);
						break;
					}
					case RM_MAKESLAVE:
					{
						MakeSlave(lpProcessMsg->wParam, lpProcessMsg->lParam1, lpProcessMsg->lParam2, lpProcessMsg->lParam3, 1, 48 * 60 * 60);

						break;
					}
				} // switch

				delete lpProcessMsg;
				lpProcessMsg = NULL;
			}
		}
	}

	nCount = m_ProcessQ.GetCount();

	if (nCount)
	{
		char				szEncodeMsg[256];
		_TMESSAGEBODYWL		MsgBodyWL;
		_TSHORTMSSEAGE		tsMsg;
		_TCHARDESC			CharDesc;

		for (int i = 0; i < nCount; i++)
		{
//			EnterCriticalSection(&m_cs);
			lpProcessMsg = (_LPTPROCESSMSG)m_ProcessQ.PopQ();
//			LeaveCriticalSection(&m_cs);

//#ifdef _DEBUG
//			_RPT4(_CRT_WARN, "CPlayerObject::Operate - %d, %d, %d, %d", lpProcessMsg->wIdent, lpProcessMsg->wParam, lpProcessMsg->lParam1, lpProcessMsg->lParam2);
//			_RPT1(_CRT_WARN, " %d \n", lpProcessMsg->lParam3);
//#endif
			if (lpProcessMsg)
			{
				switch (lpProcessMsg->wIdent)
				{
					case CM_TURN:
					{
						if (!m_fIsDead)
						{
							if (TurnXY(lpProcessMsg->lParam1, lpProcessMsg->lParam2, lpProcessMsg->wParam)) // x, y, dir
								memmove(szEncodeMsg, _MSG_GOOD, sizeof(_MSG_GOOD));
							else
								memmove(szEncodeMsg, _MSG_FAIL, sizeof(_MSG_FAIL));
						}
						else
							memmove(szEncodeMsg, _MSG_FAIL, sizeof(_MSG_FAIL));

						ValToAnsiStr(GetTickCount(), &szEncodeMsg[sizeof(_MSG_GOOD) - 1]);
						SendSocket(NULL, szEncodeMsg);

						break;
					}
					case CM_WALK:
					{
						if (!m_fIsDead)
						{
//							if (WalkXY(lpProcessMsg->lParam1, lpProcessMsg->lParam2, lpProcessMsg->wParam))
							if (WalkTo(lpProcessMsg->wParam)) // dir
								memmove(szEncodeMsg, _MSG_GOOD, sizeof(_MSG_GOOD));
							else
								memmove(szEncodeMsg, _MSG_FAIL, sizeof(_MSG_FAIL));
						}
						else
							memmove(szEncodeMsg, _MSG_FAIL, sizeof(_MSG_FAIL));

						ValToAnsiStr(GetTickCount(), &szEncodeMsg[sizeof(_MSG_GOOD) - 1]);
						SendSocket(NULL, szEncodeMsg);

						break;
					}
					case CM_RUN:
					{
						if (!m_fIsDead)
						{
//							if (RunXY(lpProcessMsg->lParam1, lpProcessMsg->lParam2, lpProcessMsg->wParam))
							if (RunTo(lpProcessMsg->wParam)) // dir
								memmove(szEncodeMsg, _MSG_GOOD, sizeof(_MSG_GOOD));
							else
								memmove(szEncodeMsg, _MSG_FAIL, sizeof(_MSG_FAIL));
						}
						else
							memmove(szEncodeMsg, _MSG_FAIL, sizeof(_MSG_FAIL));

						ValToAnsiStr(GetTickCount(), &szEncodeMsg[sizeof(_MSG_GOOD) - 1]);
						SendSocket(NULL, szEncodeMsg);

						break;
					}
					case CM_HIT:
					case CM_LONGHIT:
					case CM_WIDEHIT:
					case CM_FIREHIT:
					case CM_POWERHIT:
					{
						if (!m_fIsDead)
						{
							if (HitXY(lpProcessMsg->wIdent, lpProcessMsg->lParam1, lpProcessMsg->lParam2, lpProcessMsg->wParam, lpProcessMsg->lParam3))
								memmove(szEncodeMsg, _MSG_GOOD, sizeof(_MSG_GOOD));
							else
								memmove(szEncodeMsg, _MSG_FAIL, sizeof(_MSG_FAIL));
						}
						else
							memmove(szEncodeMsg, _MSG_FAIL, sizeof(_MSG_FAIL));

						ValToAnsiStr(GetTickCount(), &szEncodeMsg[sizeof(_MSG_GOOD) - 1]);
						SendSocket(NULL, szEncodeMsg);

						break;
					}
					case CM_RIDE:
					{
						if (!m_fIsDead)
						{
							if (DoRideHorse(lpProcessMsg->wParam, lpProcessMsg->lParam1))
								memmove(szEncodeMsg, _MSG_GOOD, sizeof(_MSG_GOOD));
							else
								memmove(szEncodeMsg, _MSG_FAIL, sizeof(_MSG_FAIL));
						}
						else
							memmove(szEncodeMsg, _MSG_FAIL, sizeof(_MSG_FAIL));

						ValToAnsiStr(GetTickCount(), &szEncodeMsg[sizeof(_MSG_GOOD) - 1]);
						SendSocket(NULL, szEncodeMsg);

						break;
					}
					case CM_SAY:
					{
						if (lpProcessMsg->pszData)
							ProcessForUserSaid(lpProcessMsg->pszData);
						break;
					}
					case CM_SPELL:
					{
						if (!m_fIsDead)
						{
							if (SpellXY(lpProcessMsg->wParam, lpProcessMsg->lParam1, lpProcessMsg->lParam2, lpProcessMsg->lParam3))
								memmove(szEncodeMsg, _MSG_GOOD, sizeof(_MSG_GOOD));
							else
								memmove(szEncodeMsg, _MSG_FAIL, sizeof(_MSG_FAIL));
						}
						else
							memmove(szEncodeMsg, _MSG_FAIL, sizeof(_MSG_FAIL));

						ValToAnsiStr(GetTickCount(), &szEncodeMsg[sizeof(_MSG_GOOD) - 1]);
						SendSocket(NULL, szEncodeMsg);

						break;
					}
					case CM_SITDOWN:
					{
						if (!m_fIsDead)
							memmove(szEncodeMsg, _MSG_GOOD, sizeof(_MSG_GOOD));
						else
							memmove(szEncodeMsg, _MSG_FAIL, sizeof(_MSG_FAIL));

						ValToAnsiStr(GetTickCount(), &szEncodeMsg[sizeof(_MSG_GOOD) - 1]);
						SendSocket(NULL, szEncodeMsg);

						break;
					}
                    case CM_TAKEONITEM:
                    case CM_TAKEOFFITEM:
                    {
                        char key[Mir3Item::LegacyKeySize + 1] = {}; int standard = 0;
                        const int slot = lpProcessMsg->lParam2;
                        const bool takingOn = lpProcessMsg->wIdent == CM_TAKEONITEM;
                        if (slot < 0 || slot >= sizeof(m_pUserInfo->m_THumanRcd.szTakeItem) / Mir3Item::LegacyKeySize ||
                            !m_pUserInfo->ResolveItemRequest(DWORD(lpProcessMsg->lParam1), lpProcessMsg->pszData, key, standard)) {
                            fnMakeDefMessage(&DefMsg, takingOn ? SM_TAKEON_FAIL : SM_TAKEOFF_FAIL, 2, 0, 0, 0);
                            SendSocket(&DefMsg, NULL);
                        } else {
                            char previous[Mir3Item::LegacyKeySize + 1] = {};
                            memcpy(previous, m_pUserInfo->m_THumanRcd.szTakeItem[slot], Mir3Item::LegacyKeySize);
                            if (!takingOn) ServerGetTakeOffItem(WORD(slot), key);
                            else if (key[0] == 'G') ServerGetTakeOnGenItem(WORD(slot), key);
                            else ServerGetTakeOnItem(WORD(slot), key);
                            // MIR3 clears the held item on success; return the removed record afterwards.
                            const bool changed = memcmp(previous, m_pUserInfo->m_THumanRcd.szTakeItem[slot], Mir3Item::LegacyKeySize) != 0;
                            if (changed) {
                                if (previous[0] == 'G') {
                                    const auto item = m_pUserInfo->GetUseGenItem(previous);
                                    if (item) {
                                        _TUSERITEMRCD returned = {};
                                        memcpy(returned.szMakeIndex, item->szMakeIndex, Mir3Item::LegacyKeySize);
                                        returned.nStdIndex = item->nStdIndex;
                                        returned.nDura = item->nDura; returned.nDuraMax = item->nDuraMax;
                                        SendAddItem(&returned);
                                    }
                                } else {
                                    const auto item = m_pUserInfo->GetItem(previous);
                                    if (item) SendAddItem(item);
                                }
                            }
                        }
                        break;
                    }
                    case CM_MERCHANTQUERYSELLPRICE:
                    case CM_USERSELLITEM:
                    {
                        const bool commit = lpProcessMsg->wIdent==CM_USERSELLITEM;
                        bool handled = false;
                        for (auto node=g_xMerchantObjList.GetHead(); node; node=g_xMerchantObjList.GetNext(node)) {
                            auto* npc=g_xMerchantObjList.GetData(node);
                            if ((int)npc!=lpProcessMsg->lParam1) continue;
                            if (m_fIsDead || npc!=m_dialogMerchant || npc!=m_sellMerchant || npc->m_pMap!=m_pMap ||
                                abs(npc->m_nCurrX-m_nCurrX)>m_nViewRange || abs(npc->m_nCurrY-m_nCurrY)>m_nViewRange) break;
                            char key[Mir3Item::LegacyKeySize+1]={}; int standard=0;
                            const DWORD instance=DWORD(lpProcessMsg->lParam2);
                            _LPTGENERALITEMRCD item=NULL;
                            _LPTUSERITEMRCD equipment=NULL;
                            if (m_pUserInfo->ResolveItemRequest(instance,lpProcessMsg->pszData,key,standard) && !IsEquipped(*m_pUserInfo,key)) {
                                if (key[0]=='G') item=m_pUserInfo->GetUseGenItem(key,instance);
                                else equipment=m_pUserInfo->GetItem(key);
                            }
                            npc->Sale(this,item,commit,lpProcessMsg->wParam,equipment);
                            handled=true; break;
                        }
                        if (!handled) {
                            fnMakeDefMessage(&DefMsg,commit ? SM_USERSELLITEM_FAIL : SM_SENDBUYPRICE,0,0,0,0);
                            SendSocket(&DefMsg,NULL);
                        }
                        break;
                    }
                    case CM_USERBUYITEM:
                    case CM_USERGETDETAILITEM:
                    case CM_CLICKNPC:
                    case CM_MERCHANTDLGSELECT:
                    {
                        // Resolve the wire identity against live merchants before dereferencing it.
                        PLISTNODE node = g_xMerchantObjList.GetHead();
                        while (node) {
                            CMerchantObject* npc = g_xMerchantObjList.GetData(node);
                            node = g_xMerchantObjList.GetNext(node);
                            if ((int)npc != lpProcessMsg->lParam1) continue;
                            if (m_fIsDead || npc->m_pMap != m_pMap ||
                                abs(npc->m_nCurrX-m_nCurrX)>m_nViewRange ||
                                abs(npc->m_nCurrY-m_nCurrY)>m_nViewRange) break;
                            if (lpProcessMsg->wIdent == CM_CLICKNPC) {
                                m_dialogMerchant = npc;
                                m_shopMerchant = NULL;
                                m_sellMerchant = NULL;
                                m_npcScriptFiles.clear();
                                m_npcOptions.clear();
                            } else if (m_dialogMerchant != npc) break;
                            if (lpProcessMsg->wIdent == CM_USERBUYITEM || lpProcessMsg->wIdent == CM_USERGETDETAILITEM) {
                                if (m_shopMerchant != npc || !lpProcessMsg->pszData) break;
                                char name[128];
                                const int length = fnDecode6BitBufA(lpProcessMsg->pszData, name, sizeof(name)-1);
                                name[length] = 0;
                                if (lpProcessMsg->wIdent == CM_USERBUYITEM) npc->Buy(this, name, lpProcessMsg->wParam, DWORD(lpProcessMsg->lParam2));
                                else npc->Details(this, name, DWORD(lpProcessMsg->lParam2));
                                break;
                            }
                            char section[256] = "@main";
                            if (lpProcessMsg->wIdent == CM_MERCHANTDLGSELECT) {
                                if (!lpProcessMsg->pszData) break;
                                const int size = fnDecode6BitBufA(lpProcessMsg->pszData, section, sizeof(section)-1);
                                section[size] = 0;
                                if (section[0] != '@') break;
                                // Only options in the last delivered page may invoke script labels.
                                bool allowed = !_stricmp(section, "@main") || !_stricmp(section, "@exit");
                                for (const auto& option : m_npcOptions) if (option == section) allowed = true;
                                if (!allowed) break;
                            }
                            if (!_stricmp(section, "@exit")) {
                                fnMakeDefMessage(&DefMsg, SM_MERCHANTDLGCLOSE, (int)npc, 0, 0, 0);
                                SendSocket(&DefMsg, NULL);
                                m_dialogMerchant = NULL;
                                m_shopMerchant = NULL;
                                m_sellMerchant = NULL;
                                m_npcScriptFiles.clear();
                                m_npcOptions.clear();
                            } else npc->Say(this, section);
                            break;
                        }
                        break;
                    }
					case CM_QUERYUSERNAME:
					{
						GetQueryUserName((CCharObject*)lpProcessMsg->lParam1, lpProcessMsg->lParam2/*x*/, lpProcessMsg->lParam3/*y*/);
						break;
					}
					case CM_QUERYBAGITEMS:
					{
						SendBagItems();

						break;
					}
                    case CM_EAT:
                    {
                        char key[Mir3Item::LegacyKeySize + 1] = {}; int standard = 0;
                        if (m_pUserInfo->ResolveItemRequest(DWORD(lpProcessMsg->lParam1), lpProcessMsg->pszData, key, standard) && key[0] == 'G') {
                            ServerGetEatItem(standard, key, DWORD(lpProcessMsg->lParam1));
                        } else {
                            fnMakeDefMessage(&DefMsg, SM_EAT_FAIL, 0, 0, 0, 0);
                            SendSocket(&DefMsg, NULL);
                        }
                        break;
                    }
					case CM_MAGICKEYCHANGE:
					{
						ServerGetMagicKeyChange(lpProcessMsg->wParam, lpProcessMsg->lParam1);
						break;
					}
                    case CM_DROPITEM:
                    {
                        char key[Mir3Item::LegacyKeySize + 1] = {}; int standard = 0;
                        const DWORD instance = DWORD(lpProcessMsg->lParam1);
                        bool dropped = false;
                        if (m_pUserInfo->ResolveItemRequest(instance, lpProcessMsg->pszData, key, standard) && !IsEquipped(*m_pUserInfo, key)) {
                            dropped = key[0] == 'G' ? m_pUserInfo->UserDropGenItem(standard, key, instance) : m_pUserInfo->UserDropItem(standard, key);
                        }
                        fnMakeDefMessage(&DefMsg, dropped ? SM_DROPITEM_SUCCESS : SM_DROPITEM_FAIL, instance, 0, 0, 0);
                        SendSocket(&DefMsg, dropped ? NULL : lpProcessMsg->pszData);
                        break;
                    }
					case CM_PICKUP:
					{
						if ((m_nCurrX == (int)lpProcessMsg->lParam3) && (m_nCurrY == (int)lpProcessMsg->lParam2))
								PickUp();

						break;
					}
					case CM_BUTCH:
					{
						ServerGetButch((CCharObject*)lpProcessMsg->lParam1, lpProcessMsg->lParam2/*x*/, lpProcessMsg->lParam3/*y*/, lpProcessMsg->wParam); 
						break;
					}
					case RM_LOGON:
					{
						SendNewMap();

						fnMakeDefMessage(&DefMsg, SM_LOGON, (int)this, 
											(unsigned short)m_nCurrX, (unsigned short)m_nCurrY, MAKEWORD(m_nDirection, lpProcessMsg->pCharObject->m_btLight));

                        const LONG feature = lpProcessMsg->pCharObject->GetFeatureToLong();
                        nPos = fnEncode6BitBufA((unsigned char *)&feature, szEncodeMsg, sizeof(feature), sizeof(szEncodeMsg));
						nPos +=	fnEncode6BitBufA((unsigned char *)&((CPlayerObject *)lpProcessMsg->pCharObject)->m_tFeatureEx, &szEncodeMsg[nPos], sizeof(_TOBJECTFEATUREEX), sizeof(szEncodeMsg) - nPos);
						szEncodeMsg[nPos] = '\0';

						SendSocket(&DefMsg, szEncodeMsg);

						GetQueryUserName(this, m_nCurrX, m_nCurrY);

						SendMapName();

						break;
					}
					case RM_HEAR:
					case RM_CRY:
					case RM_WHISPER:
					case RM_SYSMESSAGE:
					case RM_SYSMESSAGE2:
					case RM_GROUPMESSAGE:
					case RM_GUILDMESSAGE:
					case RM_MERCHANTSAY:
					case RM_MONSTERSAY:
					{
						if (lpProcessMsg->pszData)
						{
							switch (lpProcessMsg->wIdent)
							{
								case RM_HEAR:
									fnMakeDefMessage(&DefMsg, SM_HEAR, (int)lpProcessMsg->pCharObject, _CHAT_COLOR3, _CHAT_COLOR1, 0);
									break;
								case RM_CRY:
									fnMakeDefMessage(&DefMsg, SM_CRY, (int)lpProcessMsg->pCharObject, _CHAT_COLOR1, _CHAT_COLOR7, 0);
									break;
								case RM_WHISPER:
									fnMakeDefMessage(&DefMsg, SM_WHISPER, (int)lpProcessMsg->pCharObject, 7, _CHAT_COLOR1, 0);
									break;
								case RM_SYSMESSAGE:
									fnMakeDefMessage(&DefMsg, SM_SYSMESSAGE, (int)lpProcessMsg->pCharObject, _CHAT_COLOR3, _CHAT_COLOR8, 0);
									break;
								case RM_SYSMESSAGE2:
									fnMakeDefMessage(&DefMsg, SM_SYSMESSAGE, (int)lpProcessMsg->pCharObject, _CHAT_COLOR3, _CHAT_COLOR4, 0);
									break;
								case RM_GROUPMESSAGE:
									fnMakeDefMessage(&DefMsg, SM_SYSMESSAGE, (int)lpProcessMsg->pCharObject, _CHAT_COLOR3, _CHAT_COLOR1, 0);
									break;
								case RM_GUILDMESSAGE:
									fnMakeDefMessage(&DefMsg, SM_GUILDMESSAGE, (int)lpProcessMsg->pCharObject, _CHAT_COLOR3, _CHAT_COLOR1, 0);
									break;
								case RM_MERCHANTSAY:
									fnMakeDefMessage(&DefMsg, SM_MERCHANTSAY, (int)lpProcessMsg->pCharObject, 0, 0, 1);
									break;
								case RM_MONSTERSAY:
									fnMakeDefMessage(&DefMsg, SM_MONSTERSAY, (int)lpProcessMsg->pCharObject, _CHAT_COLOR3, _CHAT_COLOR1, 0);
									break;
							}

							nPos = fnEncode6BitBufA((unsigned char *)lpProcessMsg->pszData, szEncodeMsg, memlen(lpProcessMsg->pszData), sizeof(szEncodeMsg));
							szEncodeMsg[nPos] = '\0';
							
							SendSocket(&DefMsg, szEncodeMsg);
						}

						break;
					}
					case RM_RIDE:
					{
						break;
					}
					case RM_TURN:
					{
						if (lpProcessMsg->pCharObject != this)
						{
							fnMakeDefMessage(&DefMsg, SM_TURN, (int)lpProcessMsg->pCharObject, 
												(unsigned short)lpProcessMsg->lParam1, (unsigned short)lpProcessMsg->lParam2, MAKEWORD(lpProcessMsg->wParam, lpProcessMsg->pCharObject->m_btLight));
							
							CharDesc.Feature	= lpProcessMsg->pCharObject->GetFeatureToLong();
							CharDesc.Status		= lpProcessMsg->pCharObject->m_nCharStatus;

							nPos = 	fnEncode6BitBufA((unsigned char *)&CharDesc, szEncodeMsg, sizeof(_TCHARDESC), sizeof(szEncodeMsg));

							if (lpProcessMsg->pCharObject->m_wObjectType & _OBJECT_HUMAN)
								nPos +=	fnEncode6BitBufA((unsigned char *)&((CPlayerObject *)lpProcessMsg->pCharObject)->m_tFeatureEx, &szEncodeMsg[nPos], sizeof(_TOBJECTFEATUREEX), sizeof(szEncodeMsg) - nPos);
							
							if (lpProcessMsg->pszData)
								nPos += fnEncode6BitBufA((unsigned char *)lpProcessMsg->pszData, &szEncodeMsg[nPos], memlen(lpProcessMsg->pszData) - 1, sizeof(szEncodeMsg) - nPos);

							szEncodeMsg[nPos] = '\0';

							SendSocket(&DefMsg, szEncodeMsg);
						}

						break;
					}
					case RM_WALK:
					{
						if (lpProcessMsg->pCharObject != this)
						{
							fnMakeDefMessage(&DefMsg, SM_WALK, (int)lpProcessMsg->pCharObject, 
												(unsigned short)lpProcessMsg->lParam1, (unsigned short)lpProcessMsg->lParam2, MAKEWORD(lpProcessMsg->wParam, lpProcessMsg->pCharObject->m_btLight));
							
							CharDesc.Feature	= lpProcessMsg->pCharObject->GetFeatureToLong();
							CharDesc.Status		= lpProcessMsg->pCharObject->m_nCharStatus;

							nPos = 	fnEncode6BitBufA((unsigned char *)&CharDesc, szEncodeMsg, sizeof(_TCHARDESC), sizeof(szEncodeMsg));

							if (lpProcessMsg->pCharObject->m_wObjectType & _OBJECT_HUMAN)
								nPos +=	fnEncode6BitBufA((unsigned char *)&((CPlayerObject *)lpProcessMsg->pCharObject)->m_tFeatureEx, &szEncodeMsg[nPos], sizeof(_TOBJECTFEATUREEX), sizeof(szEncodeMsg) - nPos);
							
							szEncodeMsg[nPos] = '\0';

							SendSocket(&DefMsg, szEncodeMsg);
//							 cdesc.Feature := TCreature(msg.sender).Feature;
//							 cdesc.Status := TCreature(msg.sender).CharStatus;
//							 SendSocket (@Def, EncodeBuffer (@cdesc, sizeof(TCharDesc)));
//#ifdef _DEBUG
//							_RPT4(_CRT_WARN, "SM_WALK[%x]:%x, %d-%d\n", ,(int)this, (int)lpProcessMsg->pCharObject, lpProcessMsg->lParam1, lpProcessMsg->lParam2);
//#endif
						}

						break;
					}
					case RM_RUN:
					{
						if (lpProcessMsg->pCharObject != this)
						{
							fnMakeDefMessage(&DefMsg, SM_RUN, (int)lpProcessMsg->pCharObject, 
												(unsigned short)lpProcessMsg->lParam1, (unsigned short)lpProcessMsg->lParam2, MAKEWORD(lpProcessMsg->wParam, lpProcessMsg->pCharObject->m_btLight));
							
							CharDesc.Feature	= lpProcessMsg->pCharObject->GetFeatureToLong();
							CharDesc.Status		= lpProcessMsg->pCharObject->m_nCharStatus;

							nPos = 	fnEncode6BitBufA((unsigned char *)&CharDesc, szEncodeMsg, sizeof(_TCHARDESC), sizeof(szEncodeMsg));

							if (lpProcessMsg->pCharObject->m_wObjectType & _OBJECT_HUMAN)
								nPos +=	fnEncode6BitBufA((unsigned char *)&((CPlayerObject *)lpProcessMsg->pCharObject)->m_tFeatureEx, &szEncodeMsg[nPos], sizeof(_TOBJECTFEATUREEX), sizeof(szEncodeMsg) - nPos);
							
							szEncodeMsg[nPos] = '\0';

							SendSocket(&DefMsg, szEncodeMsg);
						}

						break;
					}
					case RM_BUTCH:
					{
						if (lpProcessMsg->pCharObject != this)
						{
							fnMakeDefMessage(&DefMsg, SM_BUTCH, (int)lpProcessMsg->pCharObject, 
												(unsigned short)lpProcessMsg->lParam1/*x*/, (unsigned short)lpProcessMsg->lParam2/*y*/, lpProcessMsg->wParam/*Dir*/);
						
							SendSocket(&DefMsg, NULL);
						}

						break;
					}
					case RM_HIT:
					{
						if (lpProcessMsg->pCharObject != this)
						{
							fnMakeDefMessage(&DefMsg, SM_HIT, (int)lpProcessMsg->pCharObject, 
												(unsigned short)lpProcessMsg->lParam1, (unsigned short)lpProcessMsg->lParam2, MAKEWORD(LOBYTE(lpProcessMsg->wParam), lpProcessMsg->lParam3));//lpProcessMsg->pCharObject->m_nLight);
						
							SendSocket(&DefMsg, NULL);
						}

						break;
					}
					case RM_LONGHIT:
					{
						if (lpProcessMsg->pCharObject != this)
						{
							fnMakeDefMessage(&DefMsg, SM_LONGHIT, (int)lpProcessMsg->pCharObject, 
												(unsigned short)lpProcessMsg->lParam1, (unsigned short)lpProcessMsg->lParam2, lpProcessMsg->wParam);//lpProcessMsg->pCharObject->m_nLight);
						
							SendSocket(&DefMsg, NULL);
						}

						break;
					}
					case RM_WIDEHIT:
					{
						if (lpProcessMsg->pCharObject != this)
						{
							fnMakeDefMessage(&DefMsg, SM_WIDEHIT, (int)lpProcessMsg->pCharObject, 
												(unsigned short)lpProcessMsg->lParam1, (unsigned short)lpProcessMsg->lParam2, lpProcessMsg->wParam);//lpProcessMsg->pCharObject->m_nLight);
						
							SendSocket(&DefMsg, NULL);
						}

						break;
					}
					case RM_FIREHIT:
					{
						if (lpProcessMsg->pCharObject != this)
						{
							fnMakeDefMessage(&DefMsg, SM_FIREHIT, (int)lpProcessMsg->pCharObject, 
												(unsigned short)lpProcessMsg->lParam1, (unsigned short)lpProcessMsg->lParam2, lpProcessMsg->wParam);//lpProcessMsg->pCharObject->m_nLight);
						
							SendSocket(&DefMsg, NULL);
						}

						break;
					}
					case RM_POWERHIT:
					{
						if (lpProcessMsg->pCharObject != this)
						{
							fnMakeDefMessage(&DefMsg, SM_POWERHIT, (int)lpProcessMsg->pCharObject, 
												(unsigned short)lpProcessMsg->lParam1, (unsigned short)lpProcessMsg->lParam2, lpProcessMsg->wParam);//lpProcessMsg->pCharObject->m_nLight);
						
							SendSocket(&DefMsg, NULL);
						}

						break;
					}
					case RM_SPELL:
					case RM_SPELL2:
					{
						if (lpProcessMsg->pCharObject != this)
						{
							WORD wIdent;
							char szMagicID[8];

							if (lpProcessMsg->wIdent == RM_SPELL)
								wIdent = SM_SPELL;
							else
								wIdent = SM_SPELL2;

							fnMakeDefMessage(&DefMsg, wIdent, (int)lpProcessMsg->pCharObject, 
												(unsigned short)lpProcessMsg->lParam1/*x*/, (unsigned short)lpProcessMsg->lParam2/*y*/, lpProcessMsg->wParam/*effect*/);
							ValToAnsiStr(lpProcessMsg->lParam3, szMagicID); // MagicID							
							nPos = fnEncode6BitBufA((unsigned char *)szMagicID, szEncodeMsg, memlen(szMagicID) - 1, sizeof(szEncodeMsg));
							szEncodeMsg[nPos] = '\0';
							SendSocket(&DefMsg, szEncodeMsg);
						}

						break;
					}
					case RM_MAGICFIRE:
					{
//						char szMagicID[8];

						fnMakeDefMessage(&DefMsg, SM_MAGICFIRE, (int)lpProcessMsg->pCharObject, 
											LOWORD(lpProcessMsg->lParam2)/*x*/, HIWORD(lpProcessMsg->lParam2)/*y*/, (WORD)lpProcessMsg->lParam1);
						nPos = fnEncode6BitBufA((unsigned char *)&lpProcessMsg->lParam3, szEncodeMsg, sizeof(DWORD), sizeof(szEncodeMsg));
						szEncodeMsg[nPos] = '\0';

//						ValToAnsiStr(lpProcessMsg->lParam3, szMagicID); // MagicID							
						SendSocket(&DefMsg, szEncodeMsg);

						break;
					}
					case RM_STRUCK:
					case RM_STRUCK_MAG:
					{
						if (lpProcessMsg->wParam > 0)
						{
							if (lpProcessMsg->pCharObject)
							{
								if (lpProcessMsg->pCharObject == this)
								{
									m_dwHealthTick	= 0;
									m_dwSpellTick	= 0;
									m_btPerHealth--;
									m_btPerSpell--;
								}

								fnMakeDefMessage(&DefMsg, SM_STRUCK, (int)lpProcessMsg->pCharObject, 
															lpProcessMsg->pCharObject->m_WAbility.HP/*HP*/, lpProcessMsg->pCharObject->m_WAbility.MaxHP/*MaxHP*/, lpProcessMsg->wParam);
								
								MsgBodyWL.lParam1	= lpProcessMsg->pCharObject->GetFeatureToLong();
								MsgBodyWL.lParam2	= lpProcessMsg->pCharObject->m_nCharStatus;

								MsgBodyWL.nTag1		= lpProcessMsg->lParam3;

								nPos = fnEncode6BitBufA((unsigned char *)&MsgBodyWL, szEncodeMsg, sizeof(MsgBodyWL), sizeof(szEncodeMsg));
								szEncodeMsg[nPos] = '\0';
								
								m_dwHealthTick	= 0;
								m_dwSpellTick	= 0;

								SendSocket(&DefMsg, szEncodeMsg);
							}
						}

						break;
					}
					case RM_RUSH:
					case RM_PUSH:
					{
//						if (lpProcessMsg->pCharObject != this)
//						{
							if (lpProcessMsg->wIdent == RM_PUSH)
								fnMakeDefMessage(&DefMsg, SM_BACKSTEP, (int)lpProcessMsg->pCharObject, (unsigned short)lpProcessMsg->lParam1/*x*/, 
														(unsigned short)lpProcessMsg->lParam2/*y*/, lpProcessMsg->wParam);
							else if (lpProcessMsg->wIdent == RM_RUSH)
								fnMakeDefMessage(&DefMsg, SM_RUSH, (int)lpProcessMsg->pCharObject, (unsigned short)lpProcessMsg->lParam1/*x*/, 
														(unsigned short)lpProcessMsg->lParam2/*y*/, lpProcessMsg->wParam);
//						}

						SendSocket(&DefMsg, NULL);

						break;
					}
					case RM_DISAPPEAR:
					{
						fnMakeDefMessage(&DefMsg, SM_DISAPPEAR, (int)lpProcessMsg->pCharObject, 0, 0, 0);
						SendSocket(&DefMsg, NULL);
						break;
					}
					case RM_CLEAROBJECTS:
					{
						fnMakeDefMessage(&DefMsg, SM_CLEAROBJECTS, 0, 0, 0, 0);
						SendSocket(&DefMsg, NULL);
						break;
					}
					case RM_CHANGEMAP:
					{
 						fnMakeDefMessage(&DefMsg, SM_CHANGEMAP, (int)this, m_nCurrX, m_nCurrY, MAKEWORD(DayBright(), m_pMap->m_btSeries));
						nPos = fnEncode6BitBufA((unsigned char *)m_pMap->m_szMapName, szEncodeMsg, memlen(m_pMap->m_szMapName) - 1, sizeof(szEncodeMsg));
						szEncodeMsg[nPos] = '\0';
						SendSocket(&DefMsg, szEncodeMsg);

						SendMapName();

						break;
					}
					case RM_SPACEMOVE_SHOW:
					{
						break;
					}
					case RM_DOOROPEN:
					{
 						fnMakeDefMessage(&DefMsg, SM_DOOROPEN, 1, (unsigned short)lpProcessMsg->lParam1, 0, 0);
						SendSocket(&DefMsg, NULL);

						break;
					}
					case RM_ABILITY:
					{
						fnMakeDefMessage(&DefMsg, SM_ABILITY, m_pUserInfo->m_THumanRcd.dwGold, m_pUserInfo->m_THumanRcd.btJob, 0, 0);
						const auto abilityRecord = Mir3Ability::FromAbility(m_WAbility);
						nPos = fnEncode6BitBufA((unsigned char *)&abilityRecord, szEncodeMsg, sizeof(abilityRecord), sizeof(szEncodeMsg));
						szEncodeMsg[nPos] = '\0';
						SendSocket(&DefMsg, szEncodeMsg);

						break;
					}
					case RM_SUBABILITY:
					{
						fnMakeDefMessage(&DefMsg, SM_SUBABILITY, MAKELONG(MAKEWORD(m_btAntiMagic, 0), 0), MAKEWORD(m_btHitPoint, m_btSpeedPoint), MAKEWORD(m_btAntiPoison, m_btPoisonRecover), MAKEWORD(m_btHealthRecover, m_btSpellRecover));
						SendSocket(&DefMsg, NULL);
						break;
					}
					case RM_DEATH:
					{
						if (lpProcessMsg->lParam3 == 1)
							fnMakeDefMessage(&DefMsg, SM_NOWDEATH, (int)lpProcessMsg->pCharObject, (WORD)lpProcessMsg->lParam1, (WORD)lpProcessMsg->lParam2, lpProcessMsg->wParam);
						else
							fnMakeDefMessage(&DefMsg, SM_DEATH, (int)lpProcessMsg->pCharObject, (WORD)lpProcessMsg->lParam1, (WORD)lpProcessMsg->lParam2, lpProcessMsg->wParam);
							
						CharDesc.Feature	= lpProcessMsg->pCharObject->GetFeatureToLong();
						CharDesc.Status		= lpProcessMsg->pCharObject->m_nCharStatus;

						nPos = fnEncode6BitBufA((unsigned char *)&CharDesc, szEncodeMsg, sizeof(CharDesc), sizeof(szEncodeMsg));
						szEncodeMsg[nPos] = '\0';

						SendSocket(&DefMsg, szEncodeMsg);

						break;
					}
					case RM_SKELETON:
					{
						fnMakeDefMessage(&DefMsg, SM_SKELETON, (int)lpProcessMsg->pCharObject, (WORD)lpProcessMsg->lParam1, (WORD)lpProcessMsg->lParam2, lpProcessMsg->wParam);
							
						CharDesc.Feature	= lpProcessMsg->pCharObject->GetFeatureToLong();
						CharDesc.Status		= lpProcessMsg->pCharObject->m_nCharStatus;

						nPos = fnEncode6BitBufA((unsigned char *)&CharDesc, szEncodeMsg, sizeof(CharDesc), sizeof(szEncodeMsg));
						szEncodeMsg[nPos] = '\0';

						SendSocket(&DefMsg, szEncodeMsg);

						break;
					}
					case RM_LEVELUP:
					{
						fnMakeDefMessage(&DefMsg, SM_LEVELUP, m_Ability.Exp, m_Ability.Level, 0, 0);
						SendSocket(&DefMsg, NULL);

						fnMakeDefMessage(&DefMsg, SM_ABILITY, m_pUserInfo->m_THumanRcd.dwGold, m_pUserInfo->m_THumanRcd.btJob, 0, 0);
						const auto abilityRecord = Mir3Ability::FromAbility(m_WAbility);
						nPos = fnEncode6BitBufA((unsigned char *)&abilityRecord, szEncodeMsg, sizeof(abilityRecord), sizeof(szEncodeMsg));
						szEncodeMsg[nPos] = '\0';
						SendSocket(&DefMsg, szEncodeMsg);

						fnMakeDefMessage(&DefMsg, SM_SUBABILITY, MAKELONG(MAKEWORD(m_btAntiMagic, 0), 0), MAKEWORD(m_btHitPoint, m_btSpeedPoint), MAKEWORD(m_btAntiPoison, m_btPoisonRecover), MAKEWORD(m_btHealthRecover, m_btSpellRecover));
						SendSocket(&DefMsg, NULL);

						break;
					}
					case RM_HEALTHSPELLCHANGED:
					{
						fnMakeDefMessage(&DefMsg, SM_HEALTHSPELLCHANGED, (int)lpProcessMsg->pCharObject, m_WAbility.HP, m_WAbility.MP, m_WAbility.MaxHP);
						SendSocket(&DefMsg, NULL);

						break;
					}
					case RM_FEATURECHANGED:
					{
						fnMakeDefMessage(&DefMsg, SM_FEATURECHANGED, (int)lpProcessMsg->pCharObject, LOWORD(lpProcessMsg->lParam1), HIWORD(lpProcessMsg->lParam1), 0);
						SendSocket(&DefMsg, NULL);

						break;
					}
					case RM_OPENHEALTH:
					{
						fnMakeDefMessage(&DefMsg, SM_OPENHEALTH, (int)lpProcessMsg->pCharObject, lpProcessMsg->pCharObject->m_WAbility.HP
											, lpProcessMsg->pCharObject->m_WAbility.MaxHP, 0);
						SendSocket(&DefMsg, NULL);

						break;
					}
					case RM_CLOSEHEALTH:
					{
						fnMakeDefMessage(&DefMsg, SM_CLOSEHEALTH, (int)lpProcessMsg->pCharObject, 0, 0, 0);
						SendSocket(&DefMsg, NULL);

						break;
					}
                    case RM_SENDUSEITEMS:
                        SendEquipmentItems();
                        break;
					case RM_SENDMYMAGIC:
					{
						SendMyMagics();

						break;
					}
					case RM_DAYCHANGING:
					{
						fnMakeDefMessage(&DefMsg, SM_DAYCHANGING, 0, m_btBright, DayBright(), 0);
						SendSocket(&DefMsg, NULL);

						break;
					}
					case RM_MAGSTRUCK:
					case RM_MAGSTRUCK_MINE:
					{
						int nVal = GetMagStruckDamage(lpProcessMsg->lParam1);

						if (nVal > 0)
						{
							StruckDamage(nVal);
							HealthSpellChanged();
							AddRefMsg(RM_STRUCK_MAG, nVal, m_WAbility.HP, m_WAbility.MP, (int)lpProcessMsg->pCharObject, NULL);
						}

						break;
					}
					case RM_CHARSTATUSCHANGED:
					{
						fnMakeDefMessage(&DefMsg, SM_CHARSTATUSCHANGED, (int)lpProcessMsg->pCharObject, 
											LOWORD(lpProcessMsg->lParam1), HIWORD(lpProcessMsg->lParam1), lpProcessMsg->wParam);
						SendSocket(&DefMsg, NULL);

						break;
					}
					case RM_CHANGELIGHT:
					{
						fnMakeDefMessage(&DefMsg, SM_CHANGELIGHT, (int)lpProcessMsg->pCharObject, 
											lpProcessMsg->pCharObject->m_btLight, 0, 0);
						SendSocket(&DefMsg, NULL);
						break;
					}
					case RM_MAGIC_LVEXP:
					{						
						fnMakeDefMessage(&DefMsg, SM_MAGIC_LVEXP, lpProcessMsg->lParam3, 
											(WORD)lpProcessMsg->lParam2, (WORD)lpProcessMsg->lParam1, 0);
						SendSocket(&DefMsg, NULL);
						break;
					}
					case RM_DIGUP:
					{
						fnMakeDefMessage(&DefMsg, SM_DIGUP, (int)lpProcessMsg->pCharObject, 
													(WORD)lpProcessMsg->lParam1, (WORD)lpProcessMsg->lParam2, MAKEWORD(lpProcessMsg->wParam, lpProcessMsg->pCharObject->m_btLight));

						MsgBodyWL.lParam1	= lpProcessMsg->pCharObject->GetFeatureToLong();
						MsgBodyWL.lParam2	= lpProcessMsg->pCharObject->m_nCharStatus;
						MsgBodyWL.nTag1		= lpProcessMsg->lParam3;

						nPos = fnEncode6BitBufA((unsigned char *)&MsgBodyWL, szEncodeMsg, sizeof(MsgBodyWL), sizeof(szEncodeMsg));
						szEncodeMsg[nPos] = '\0';
						
						SendSocket(&DefMsg, szEncodeMsg);

						break;
					}
					case RM_DIGDOWN:
					{
						fnMakeDefMessage(&DefMsg, SM_DIGDOWN, (int)lpProcessMsg->pCharObject, 
											(WORD)lpProcessMsg->lParam1, (WORD)lpProcessMsg->lParam2, 0);
						SendSocket(&DefMsg, NULL);

						break;
					}
					case RM_ITEMSHOW:
					{
						fnMakeDefMessage(&DefMsg, SM_ITEMSHOW, lpProcessMsg->lParam1, 
											(WORD)lpProcessMsg->lParam2, (WORD)lpProcessMsg->lParam3, lpProcessMsg->wParam);

						nPos = fnEncode6BitBufA((unsigned char *)lpProcessMsg->pszData, szEncodeMsg, memlen(lpProcessMsg->pszData) - 1, sizeof(szEncodeMsg));
						szEncodeMsg[nPos] = '\0';
						
						SendSocket(&DefMsg, szEncodeMsg);
						
						break;
					}
					case RM_ITEMHIDE:
					{
						fnMakeDefMessage(&DefMsg, SM_ITEMHIDE, lpProcessMsg->lParam1, 
											(WORD)lpProcessMsg->lParam2, (WORD)lpProcessMsg->lParam3, 0);
						SendSocket(&DefMsg, NULL);
						
						break;
					}
					case RM_DURACHANGE:
					{
						fnMakeDefMessage(&DefMsg, SM_DURACHANGE, lpProcessMsg->lParam1, 
											(WORD)lpProcessMsg->wParam, LOWORD(lpProcessMsg->lParam2), HIWORD(lpProcessMsg->lParam2));
						SendSocket(&DefMsg, NULL);
						
						break;
					}
					case RM_WEIGHTCHANGED:
					{
						fnMakeDefMessage(&DefMsg, SM_WEIGHTCHANGED, m_WAbility.Weight, m_WAbility.WearWeight, m_WAbility.HandWeight, 0); 

						SendSocket(&DefMsg, NULL);
						
						break;
					}
					case RM_GOLDCHANGED:
					{
						fnMakeDefMessage(&DefMsg, SM_GOLDCHANGED, (int)m_pUserInfo->GetGold(), 0, 0, 0); 

						SendSocket(&DefMsg, NULL);
						
						break;
					}
					case RM_WINEXP:
					{
						fnMakeDefMessage(&DefMsg, SM_WINEXP, m_Ability.Exp, (WORD)lpProcessMsg->lParam1, 0, 0); 

						SendSocket(&DefMsg, NULL);
						
						break;
					}
					case RM_SHOWEVENT:
					{
						tsMsg.wIdent	= HIWORD(lpProcessMsg->lParam2);
						tsMsg.wMsg		= 0;

						fnMakeDefMessage(&DefMsg, SM_SHOWEVENT, (int)lpProcessMsg->lParam1, lpProcessMsg->wParam, LOWORD(lpProcessMsg->lParam2), (WORD)lpProcessMsg->lParam3); 

						nPos = fnEncode6BitBufA((unsigned char *)&tsMsg, szEncodeMsg, sizeof(_TSHORTMSSEAGE), sizeof(szEncodeMsg));
						szEncodeMsg[nPos] = '\0';
						
						SendSocket(&DefMsg, szEncodeMsg);

						break;
					}
					case RM_HIDEEVENT:
					{
						fnMakeDefMessage(&DefMsg, SM_HIDEEVENT, (int)lpProcessMsg->lParam1, lpProcessMsg->wParam, (WORD)lpProcessMsg->lParam2, (WORD)lpProcessMsg->lParam3); 

						SendSocket(&DefMsg, NULL);
						
						break;
					}
					case RM_CHANGENAMECOLOR:
					{
						fnMakeDefMessage(&DefMsg, SM_CHANGENAMECOLOR, (int)lpProcessMsg->pCharObject, lpProcessMsg->pCharObject->GetThisCharColor(), 0, 0); 

						SendSocket(&DefMsg, NULL);
						
						break;
					}																									   
					case RM_FLYAXE:
					{
						if (lpProcessMsg->lParam3 != 0)
						{
							MsgBodyWL.lParam1	= ((CCharObject*)lpProcessMsg->lParam3)->m_nCurrX;
							MsgBodyWL.lParam2	= ((CCharObject*)lpProcessMsg->lParam3)->m_nCurrY;
							MsgBodyWL.nTag1		= LOWORD(lpProcessMsg->lParam3);
							MsgBodyWL.nTag2		= HIWORD(lpProcessMsg->lParam3);

							fnMakeDefMessage(&DefMsg, SM_FLYAXE, (int)lpProcessMsg->pCharObject, (WORD)lpProcessMsg->lParam1, (WORD)lpProcessMsg->lParam2, lpProcessMsg->wParam/*Dir*/);
							nPos = fnEncode6BitBufA((unsigned char *)&MsgBodyWL, szEncodeMsg, sizeof(MsgBodyWL), sizeof(szEncodeMsg));
							szEncodeMsg[nPos] = '\0';
							
							SendSocket(&DefMsg, szEncodeMsg);

						}

						break;
					}
					case RM_LIGHTING:
					{
						if (lpProcessMsg->lParam3 != 0)
						{
							MsgBodyWL.lParam1	= ((CCharObject*)lpProcessMsg->lParam3)->m_nCurrX;
							MsgBodyWL.lParam2	= ((CCharObject*)lpProcessMsg->lParam3)->m_nCurrY;
							MsgBodyWL.nTag1		= lpProcessMsg->lParam3;
							MsgBodyWL.nTag2		= lpProcessMsg->wParam;

							fnMakeDefMessage(&DefMsg, SM_LIGHTING, (int)lpProcessMsg->pCharObject, (WORD)lpProcessMsg->lParam1, (WORD)lpProcessMsg->lParam2, lpProcessMsg->pCharObject->m_nDirection);
							nPos = fnEncode6BitBufA((unsigned char *)&MsgBodyWL, szEncodeMsg, sizeof(MsgBodyWL), sizeof(szEncodeMsg));
							szEncodeMsg[nPos] = '\0';
							
							SendSocket(&DefMsg, szEncodeMsg);

						}

						break;
					}
					case RM_ITEMUPDATE:
					{
						fnMakeDefMessage(&DefMsg, SM_ITEMUPDATE, (int)lpProcessMsg->pCharObject, (WORD)lpProcessMsg->wParam, (WORD)0, 0);
						nPos = fnEncode6BitBufA((unsigned char *)lpProcessMsg->pszData, szEncodeMsg, memlen(lpProcessMsg->pszData), sizeof(szEncodeMsg));
						szEncodeMsg[nPos] = '\0';

						SendSocket(&DefMsg, szEncodeMsg);

						break;
					}
					case RM_USERNAME:
					{
						fnMakeDefMessage(&DefMsg, SM_USERNAME, (int)lpProcessMsg->pCharObject, lpProcessMsg->pCharObject->GetThisCharColor(), (WORD)0, 0);
						nPos = fnEncode6BitBufA((unsigned char *)lpProcessMsg->pszData, szEncodeMsg, memlen(lpProcessMsg->pszData), sizeof(szEncodeMsg));
						szEncodeMsg[nPos] = '\0';

						SendSocket(&DefMsg, szEncodeMsg);

						break;
					}
					case RM_CHANGEFACE:
					{
						if (lpProcessMsg->lParam1 && lpProcessMsg->lParam2)
						{
							fnMakeDefMessage(&DefMsg, SM_CHANGEFACE, (int)lpProcessMsg->lParam1, LOWORD(lpProcessMsg->lParam2), HIWORD(lpProcessMsg->lParam2), 0);

							CharDesc.Feature	= lpProcessMsg->pCharObject->GetFeatureToLong();
							CharDesc.Status		= lpProcessMsg->pCharObject->m_nCharStatus;

							nPos = fnEncode6BitBufA((unsigned char *)&CharDesc, szEncodeMsg, sizeof(CharDesc), sizeof(szEncodeMsg));
							szEncodeMsg[nPos] = '\0';

							SendSocket(&DefMsg, szEncodeMsg);
						}

						break;
					}
				} // switch

				if (lpProcessMsg->pszData)
				{
					delete [] lpProcessMsg->pszData;
					lpProcessMsg->pszData = NULL;
				}

				delete lpProcessMsg;
				lpProcessMsg = NULL;
			} // if
		} // for loop
	}
}


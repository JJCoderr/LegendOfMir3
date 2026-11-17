#include "stdafx.h"

int CMagicInfo::GetPower13(int nPwr, int nLevel)
{
	float p1, p2;

	p1 = (float)nPwr / 3;
	p2 = (float)nPwr - p1;

	if ((sDefMaxPower - sDefPower))
		return ROUND(p1 + p2 / (3/*MaxTrainLevel*/ + 1) * (nLevel + 1) + (sDefPower + rand() % (sDefMaxPower - sDefPower))); 
	else
		return ROUND(p1 + p2 / (3/*MaxTrainLevel*/ + 1) * (nLevel + 1) + sDefPower);
}

BOOL CMagicInfo::CheckMagicLevelup(_LPTHUMANMAGICRCD record)
{
    constexpr BYTE MaximumSkillLevel=3;
    if (record->btLevel>=MaximumSkillLevel || record->nCurrTrain<nTrain[record->btLevel]) return FALSE;
    record->nCurrTrain-=nTrain[record->btLevel];
    ++record->btLevel;
    return TRUE;
}

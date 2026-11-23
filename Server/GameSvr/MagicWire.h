#pragma once
// MIR3 1.45 client record. The database/server skill record keeps its own layout.
namespace Mir3Magic {
constexpr int LeftTalismanSlot = 6; // FLD_ARMRINGL_ID in the persisted equipment array.
#pragma pack(push,1)
struct Standard {
    WORD id;
    char name[13];
    BYTE effectType,effect;
    WORD spell,minPower;
    BYTE need[4];
    int train[4];
    BYTE maxTrainLevel,job;
    int delay;
    BYTE defSpell,defMinPower;
    WORD maxPower;
    BYTE defMaxPower;
    char description[16];
};
struct Record { DWORD training; BYTE key,level; Standard standard; };
#pragma pack(pop)
static_assert(sizeof(Record)==74,"MIR3 skill record size");
}
CMagicInfo* GetMagicInfo(int id);
inline bool ClientMagicRecord(const _THUMANMAGICRCD& source,Mir3Magic::Record& record) {
    const auto* info=GetMagicInfo(source.btMagicID);
    if(!info) return false;
    record={};record.training=source.nCurrTrain;record.level=BYTE(source.btLevel);record.key=BYTE(source.btUseKey);
    auto& standard=record.standard;
    standard.id=source.btMagicID;lstrcpynA(standard.name,info->szName,sizeof(standard.name));
    standard.effectType=BYTE(info->sEffectType);standard.effect=BYTE(info->sEffect);
    standard.spell=WORD(info->sSpell);standard.minPower=WORD(info->sPower);standard.maxPower=WORD(info->sMaxPower);
    standard.defSpell=BYTE(info->sDefSpell);standard.defMinPower=BYTE(info->sDefPower);standard.defMaxPower=BYTE(info->sDefMaxPower);
    standard.job=BYTE(info->sJob);standard.maxTrainLevel=3;standard.delay=info->sDelay;
    for(int i=0;i<3;++i) {standard.need[i]=BYTE(info->sNeed[i]);standard.train[i]=info->nTrain[i];}
    return true;
}

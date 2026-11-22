"""把 mir2x 的 King_StdItems.csv / King_Magic.csv 转换成 LoM3 的物品与技能表。

King_StdItems.csv 的列与 TBL_STDITEM 基本一一对应（Idx/Name/Stdmode/Shape/...
Looks/DuraMax/Ac/Mac/Dc/Mc/Sac/Need/NeedLevel/Price/Stock）。
King_Magic.csv 对应 TBL_MAGIC（MagID/MagName/EffectType/Effect/Spell/Power/
MaxPower/DefSpell/DefPower/DefMaxPower/Job/NeedL1..3/L1..3Train/Delay）。

用法:
  python Server/Tools/import_king_tables.py --items <King_StdItems.csv> \
      --magic <King_Magic.csv> --out-dir Server/SQL_Scripts
"""
import argparse
import csv
from pathlib import Path

ITEM_COLUMNS = (
    'FLD_INDEX,FLD_NAME,FLD_STDMODE,FLD_TYPE,FLD_SHAPE,FLD_WEIGHT,FLD_ANICOUNT,FLD_SOURCE,'
    'FLD_LOOKS,FLD_DURAMAX,FLD_AC,FLD_AC2,FLD_MAC,FLD_MAC2,FLD_DC,FLD_DC2,'
    'FLD_MC,FLD_MC2,FLD_SC,FLD_SC2,FLD_NEED,FLD_NEEDLEVEL,FLD_PRICE,FLD_STOCK')

MAGIC_COLUMNS = (
    'FLD_INDEX,FLD_NAME,FLD_EFFECTTYPE,FLD_EFFECT,FLD_SPELL,FLD_POWER,FLD_MAXPOWER,'
    'FLD_DEFSPELL,FLD_DEFPOWER,FLD_DEFMAXPOWER,FLD_JOB,FLD_NEEDL1,FLD_L1TRAIN,'
    'FLD_NEEDL2,FLD_L2TRAIN,FLD_NEEDL3,FLD_L3TRAIN,FLD_DELAY,FLD_DESC,FLD_DESCRIPTION')


def q(text):
    return "'" + ' '.join(str(text).split()).replace("'", "''") + "'"


def as_int(row, key, default=0):
    try:
        return int(float(row.get(key) or default))
    except (TypeError, ValueError):
        return default


def load(path):
    with path.open(encoding='utf-8-sig', errors='replace', newline='') as handle:
        return list(csv.DictReader(handle))


def build_items(rows):
    statements = ['DELETE FROM TBL_STDITEM;', 'DELETE FROM TBL_STDITEM_ETC;']
    equip = etc = 0
    for row in rows:
        name = (row.get('Name') or '').strip()
        if not name or name.startswith('测试'):
            continue
        index = as_int(row, 'Idx')
        if index <= 0:
            continue
        stdmode = as_int(row, 'Stdmode')
        shape = as_int(row, 'Shape')
        values = (
            index, q(name), stdmode, stdmode, shape, as_int(row, 'Weight'), as_int(row, 'Anicount'),
            as_int(row, 'Source'), as_int(row, 'Looks'), as_int(row, 'DuraMax'),
            as_int(row, 'Ac'), as_int(row, 'Ac2'), as_int(row, 'Mac'), as_int(row, 'Mac2'),
            as_int(row, 'Dc'), as_int(row, 'Dc2'), as_int(row, 'Mc'), as_int(row, 'Mc2'),
            as_int(row, 'Sac'), as_int(row, 'Sac2'), as_int(row, 'Need'), as_int(row, 'NeedLevel'),
            as_int(row, 'Price'), as_int(row, 'Stock'))
        if stdmode >= 5:
            statements.append(f'INSERT INTO TBL_STDITEM ({ITEM_COLUMNS}) VALUES '
                              '(' + ','.join(str(v) for v in values) + ');')
            equip += 1
        else:
            statements.append(
                'INSERT INTO TBL_STDITEM_ETC (FLD_INDEX,FLD_NAME,FLD_STDMODE,FLD_SHAPE,FLD_LOOKS,'
                'FLD_WEIGHT,FLD_PRICE,FLD_DURAMAX,FLD_STACKMODE,FLD_VAL1,FLD_VAL2) VALUES '
                f'({index},{q(name)},{stdmode},{shape},{as_int(row, "Looks")},{as_int(row, "Weight")},'
                f'{as_int(row, "Price")},{as_int(row, "DuraMax")},{as_int(row, "StackSize", 0)},'
                f'{as_int(row, "Func")},0);')
            etc += 1
    return statements, equip, etc


def build_magic(rows):
    statements = ['DELETE FROM TBL_MAGIC;']
    count = 0
    for row in rows:
        name = (row.get('MagName') or '').strip()
        if not name:
            continue
        index = as_int(row, 'MagID')
        if index <= 0:
            continue
        statements.append(
            f'INSERT INTO TBL_MAGIC ({MAGIC_COLUMNS}) VALUES '
            f'({index},{q(name)},{as_int(row, "EffectType")},{as_int(row, "Effect")},'
            f'{as_int(row, "Spell")},{as_int(row, "Power")},{as_int(row, "MaxPower")},'
            f'{as_int(row, "DefSpell")},{as_int(row, "DefPower")},{as_int(row, "DefMaxPower")},'
            f'{as_int(row, "Job")},{as_int(row, "NeedL1")},{as_int(row, "L1Train")},'
            f'{as_int(row, "NeedL2")},{as_int(row, "L2Train")},{as_int(row, "NeedL3")},{as_int(row, "L3Train")},'
            f'{as_int(row, "Delay")},{q(name)},{q(row.get("OrgMagName") or name)});')
        count += 1
    return statements, count


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--items', type=Path, required=True)
    parser.add_argument('--magic', type=Path, required=True)
    parser.add_argument('--out-dir', type=Path, required=True)
    args = parser.parse_args()

    items, equip, etc = build_items(load(args.items))
    magic, magic_count = build_magic(load(args.magic))
    (args.out_dir / 'king_items.sql').write_text(
        '\n'.join(['-- 由 Server/Tools/import_king_tables.py 生成。'] + items) + '\n', encoding='utf-8')
    (args.out_dir / 'king_magic.sql').write_text(
        '\n'.join(['-- 由 Server/Tools/import_king_tables.py 生成。'] + magic) + '\n', encoding='utf-8')
    print(f'物品 {equip + etc} 条（装备 {equip}、其他 {etc}），技能 {magic_count} 条')


if __name__ == '__main__':
    main()

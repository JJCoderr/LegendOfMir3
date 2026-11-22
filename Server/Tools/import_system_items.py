"""把 System.db 导出的物品表导入服务端 TBL_STDITEM / TBL_STDITEM_ETC。

用法: python Server/Tools/import_system_items.py --xlsx <xlsx> --out Server/SQL_Scripts/item_data.sql
      python Server/Tools/import_system_items.py --xlsx <xlsx> --sqlite <mir3.sqlite3>

物品图标以宝藏传奇3 的 Inventory.wil 为准，取 Shape 作为 LOOKS。
ItemType 到 MIR3 stdmode 只做大类映射（药水/武器/衣服/头盔/项链/手镯/戒指/矿/肉/书/蜡烛），
其余先归 0，后续按原版 stdmode 逐项校正。
"""
import argparse
import sqlite3
from pathlib import Path

import openpyxl

STAT = {'Health': 2, 'Mana': 3, 'MinAC': 4, 'MaxAC': 5, 'MinMR': 6, 'MaxMR': 7,
        'MinDC': 8, 'MaxDC': 9, 'MinMC': 10, 'MaxMC': 11, 'MinSC': 12, 'MaxSC': 13}

EQUIP_TYPES = {2, 3, 4, 5, 6, 7, 8, 9, 27}
STDMODE = {0: 0, 1: 0, 2: 5, 3: 10, 4: 30, 5: 15, 6: 19, 7: 20, 8: 21,
           9: 0, 10: 0, 11: 0, 12: 1, 13: 3, 14: 4, 15: 0, 16: 0, 17: 0,
           18: 0, 19: 0, 20: 0, 21: 0, 22: 0, 23: 0, 24: 0, 25: 0, 26: 0, 27: 0, 28: 0}

STAT_ORDER = ('MinAC', 'MaxAC', 'MinMR', 'MaxMR', 'MinDC', 'MaxDC', 'MinMC', 'MaxMC', 'MinSC', 'MaxSC')


def q(text):
    # 名称里可能带换行或制表符，统一压成空格，避免 SQL 语句被拆行。
    cleaned = ' '.join(str(text).split())
    return "'" + cleaned.replace("'", "''") + "'"


def as_int(value, default=0):
    try:
        return int(float(value))
    except (TypeError, ValueError):
        return default


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--xlsx', type=Path, required=True)
    parser.add_argument('--out', type=Path)
    parser.add_argument('--sqlite', type=Path)
    args = parser.parse_args()

    workbook = openpyxl.load_workbook(args.xlsx, read_only=True)
    rows = workbook['ItemInfo'].iter_rows(values_only=True)
    header = list(next(rows))
    at = {name: header.index(name) for name in header}
    stats = {}
    for row in list(workbook['ItemInfoStat'].iter_rows(values_only=True))[1:]:
        if row[0] is None or row[1] is None:
            continue
        stats.setdefault(as_int(row[0]), {})[as_int(row[1])] = as_int(row[2])

    equip, etc = [], []
    for row in rows:
        index = as_int(row[at['Index']])
        name = row[at['ItemName']]
        if not name or index <= 0:
            continue
        item_type = as_int(row[at['ItemType']])
        shape = as_int(row[at['Shape']])
        weight = as_int(row[at['Weight']])
        price = as_int(row[at['Price']])
        durability = as_int(row[at['Durability']])
        mode = STDMODE.get(item_type, 0)
        own = stats.get(index, {})
        if item_type in EQUIP_TYPES:
            values = ','.join(str(own.get(STAT[key], 0)) for key in STAT_ORDER)
            equip.append(
                'INSERT INTO TBL_STDITEM (FLD_INDEX,FLD_NAME,FLD_STDMODE,FLD_TYPE,FLD_SHAPE,FLD_WEIGHT,'
                'FLD_ANICOUNT,FLD_SOURCE,FLD_LOOKS,FLD_DURAMAX,FLD_AC,FLD_AC2,FLD_MAC,FLD_MAC2,FLD_DC,FLD_DC2,'
                'FLD_MC,FLD_MC2,FLD_SC,FLD_SC2,FLD_NEED,FLD_NEEDLEVEL,FLD_PRICE,FLD_STOCK) VALUES '
                '(%d,%s,%d,%d,%d,%d,1,0,%d,%d,%s,0,%d,%d,0);'
                % (index, q(name), mode, item_type, shape, weight, shape, durability, values,
                   as_int(row[at['RequiredAmount']]), price))
        else:
            stack = 1 if as_int(row[at['StackSize']], 1) > 1 else 0
            etc.append(
                'INSERT INTO TBL_STDITEM_ETC (FLD_INDEX,FLD_NAME,FLD_STDMODE,FLD_SHAPE,FLD_LOOKS,FLD_WEIGHT,'
                'FLD_PRICE,FLD_DURAMAX,FLD_STACKMODE,FLD_VAL1,FLD_VAL2) VALUES '
                '(%d,%s,%d,%d,%d,%d,%d,%d,%d,%d,0);'
                % (index, q(name), mode, shape, shape, weight, price, durability, stack,
                   as_int(row[at['Effect']])))

    statements = ['-- 由 Server/Tools/import_system_items.py 生成，请勿手改。',
                  'DELETE FROM TBL_STDITEM;', 'DELETE FROM TBL_STDITEM_ETC;'] + equip + etc
    if args.out:
        args.out.write_text('\n'.join(statements) + '\n', encoding='utf-8')
        print('SQL:', args.out, len(statements), 'statements, equip', len(equip), 'etc', len(etc))
    if args.sqlite:
        with sqlite3.connect(args.sqlite) as database:
            database.execute('BEGIN')
            for statement in statements:
                if statement.startswith('--'):
                    continue
                database.execute(statement)
            database.commit()
            print('TBL_STDITEM', database.execute('SELECT COUNT(*) FROM TBL_STDITEM').fetchone()[0],
                  'TBL_STDITEM_ETC', database.execute('SELECT COUNT(*) FROM TBL_STDITEM_ETC').fetchone()[0])


if __name__ == '__main__':
    main()

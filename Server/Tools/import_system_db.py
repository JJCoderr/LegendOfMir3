"""把 System.db 导出（xlsx）转成服务端 TBL_* 数据。

数据源: System.db导出-物理列序.xlsx（zircon-legend 的 Library.SystemModels 导出）
目标  : Server/SQL_Scripts/game_data.sql，或直接写入 SQLite 库。

用法:
  python Server/Tools/import_system_db.py --xlsx <xlsx路径> --sqlite <mir3.sqlite3>
  python Server/Tools/import_system_db.py --xlsx <xlsx路径> --out <sql路径>
"""
import argparse
import sqlite3
from pathlib import Path

import openpyxl

# Stat 枚举下标（Library/Library/Stat.cs，0 起连续）
STAT = {
    'BaseHealth': 0, 'BaseMana': 1, 'Health': 2, 'Mana': 3,
    'MinAC': 4, 'MaxAC': 5, 'MinMR': 6, 'MaxMR': 7,
    'MinDC': 8, 'MaxDC': 9, 'MinMC': 10, 'MaxMC': 11,
    'MinSC': 12, 'MaxSC': 13, 'Accuracy': 14, 'Agility': 15,
    'AttackSpeed': 16, 'Light': 17,
}


def truth(value):
    return 1 if str(value).strip().upper() == 'TRUE' else 0


def as_int(value, default=0):
    try:
        return int(float(value))
    except (TypeError, ValueError):
        return default


def as_text(value):
    return '' if value is None else str(value)


def read_sheet(workbook, name):
    rows = workbook[name].iter_rows(values_only=True)
    header = list(next(rows))
    return header, [dict(zip(header, row)) for row in rows if any(v is not None for v in row)]


def build(xlsx):
    workbook = openpyxl.load_workbook(xlsx, read_only=True)
    sql = ['-- 由 Server/Tools/import_system_db.py 从 System.db 导出生成，请勿手改。']

    # --- 怪物属性 ---
    _, monsters = read_sheet(workbook, 'MonsterInfo')
    stats = {}
    for row in read_sheet(workbook, 'MonsterInfoStat')[1]:
        if row['Monster'] is None or row['Stat'] is None:
            continue
        stats.setdefault(as_int(row['Monster']), {})[as_int(row['Stat'])] = as_int(row['Amount'])

    sql.append('DELETE FROM TBL_MONSTER;')
    for m in monsters:
        index = as_int(m['Index'])
        if index <= 0:
            continue
        own = stats.get(index, {})
        attack_delay = as_int(m['AttackDelay'])
        move_delay = as_int(m['MoveDelay'])
        sql.append(
            "INSERT INTO TBL_MONSTER (FLD_INDEX,FLD_NAME,FLD_RACE,FLD_RACEIMG,FLD_APPR,FLD_LEVEL,"
            "FLD_UNDEAD,FLD_EXP,FLD_HP,FLD_MP,FLD_AC,FLD_MAXAC,FLD_MAC,FLD_MAXMAC,FLD_DC,FLD_MAXDC,"
            "FLD_SPEED,FLD_HIT,FLD_WALKSPEED,FLD_ATTACKSPEED,FLD_VIEWRANGE,FLD_RACEINDEX,FLD_ESCAPE) VALUES "
            f"({index},{q(as_text(m['MonsterName']))},0,{as_int(m['Image'])},{as_int(m['Image'])},"
            f"{as_int(m['Level'])},{truth(m['Undead'])},{as_int(m['Experience'])},"
            f"{max(1, own.get(STAT['Health'], 0))},{own.get(STAT['Mana'], 0)},"
            f"{own.get(STAT['MinAC'], 0)},{own.get(STAT['MaxAC'], 0)},"
            f"{own.get(STAT['MinMR'], 0)},{own.get(STAT['MaxMR'], 0)},"
            f"{own.get(STAT['MinDC'], 0)},{own.get(STAT['MaxDC'], 0)},"
            f"0,{own.get(STAT['Accuracy'], 0)},{move_delay},{attack_delay},"
            f"{as_int(m['ViewRange'], 10)},{as_int(m['AI'])},0);")
    return sql


def q(text):
    return "'" + str(text).replace("'", "''") + "'"


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--xlsx', type=Path, required=True)
    parser.add_argument('--out', type=Path)
    parser.add_argument('--sqlite', type=Path)
    args = parser.parse_args()
    sql = build(args.xlsx)
    if args.out:
        args.out.write_text('\n'.join(sql) + '\n', encoding='utf-8')
        print('SQL:', args.out, len(sql), 'statements')
    if args.sqlite:
        with sqlite3.connect(args.sqlite) as database:
            database.execute('BEGIN')
            for statement in sql:
                if statement.startswith('--'):
                    continue
                database.execute(statement)
            database.commit()
            for table in ('TBL_MAPINFO', 'TBL_MONSTER', 'TBL_MONGEN'):
                count = database.execute(f'SELECT COUNT(*) FROM {table}').fetchone()[0]
                print(table, count)


if __name__ == '__main__':
    main()
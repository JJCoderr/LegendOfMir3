"""把 Mud3 的 v1.45 服务端文本配置转成服务端 TBL_* 数据。

数据源: Mud3/Envir（Mapinfo.txt、StartPoint.txt、MonGen.txt、Mon_Def/*.gen）
        怪物属性与编号来自 System.db 导出（MonsterInfo）。

用法:
  python Server/Tools/import_mud3_envir.py --envir <Mud3/Envir> --monsters <xlsx> --sqlite <mir3.sqlite3>
  python Server/Tools/import_mud3_envir.py --envir <Mud3/Envir> --monsters <xlsx> --out <sql路径>
"""
import argparse
import re
import sqlite3
from pathlib import Path

import openpyxl

MAP_LINE = re.compile(r'^\s*\[([^\]\s]+)\s+([^\]\s]+)\s*(\d*)\]\s*(.*)$')
GEN_LINE = re.compile(r'^\s*(\S+)\s+(-?\d+)\s+(-?\d+)\s+(\S+)\s+(\d+)\s+(\d+)\s+(\d+)\s*$')
START_LINE = re.compile(r'^\s*(\S+)\s+(\d+)\s+(\d+)\s*$')
LOADGEN = re.compile(r'^\s*loadgen\s+"([^"]+)"')
MERCHANT_LINE = re.compile(r'^(\S+)\s+(\S+)\s+(-?\d+)\s+(-?\d+)\s+(\S+)\s+(\d+)\s+(\d+)')


def q(text):
    return "'" + str(text).replace("'", "''") + "'"


def normalize(name):
    return re.sub(r'\d+$', '', name).strip()


def read_lines(path):
    return path.read_text(encoding='gbk', errors='ignore').splitlines()


def monster_indexes(xlsx):
    workbook = openpyxl.load_workbook(xlsx, read_only=True)
    rows = workbook['MonsterInfo'].iter_rows(values_only=True)
    header = list(next(rows))
    name_at, index_at = header.index('MonsterName'), header.index('Index')
    table = {}
    for row in rows:
        name = row[name_at]
        if not name:
            continue
        table[normalize(str(name))] = int(row[index_at])
    return table


def map_records(envir):
    records = {}
    for line in read_lines(envir / 'Mapinfo.txt'):
        if line.lstrip().startswith(';'):
            continue
        found = MAP_LINE.match(line)
        if not found:
            continue
        file_name, map_name, _, flags = found.groups()
        records.setdefault(file_name, (file_name, map_name, flags.upper()))
    return list(records.values())


def start_points(envir):
    records = {}
    for line in read_lines(envir / 'StartPoint.txt'):
        if line.lstrip().startswith(';'):
            continue
        found = START_LINE.match(line)
        if found:
            records.setdefault(found.group(1), (found.group(1), int(found.group(2)), int(found.group(3))))
    return list(records.values())


def gen_records(envir):
    records = []
    names = []
    for line in read_lines(envir / 'MonGen.txt'):
        found = LOADGEN.match(line)
        if found:
            names.append(found.group(1))
    for name in names:
        path = envir / 'Mon_Def' / name
        if not path.is_file():
            continue
        for line in read_lines(path):
            if line.lstrip().startswith(';') or line.lstrip().startswith('['):
                continue
            found = GEN_LINE.match(line)
            if found:
                records.append((found.group(1), int(found.group(2)), int(found.group(3)),
                                found.group(4), int(found.group(5)), int(found.group(6)),
                                int(found.group(7))))
    return records


def merchant_records(envir):
    records = []
    for line in read_lines(envir / 'merchant.txt'):
        if line.lstrip().startswith(';'):
            continue
        found = MERCHANT_LINE.match(line)
        if found:
            records.append(found.groups())
    return records


def build(envir, xlsx):
    monsters = monster_indexes(xlsx)
    sql = ['-- 由 Server/Tools/import_mud3_envir.py 从 Mud3/Envir 生成，请勿手改。']

    sql.append('DELETE FROM TBL_MAPINFO;')
    for file_name, map_name, flags in map_records(envir):
        series = 1 if 'NIGHT' in flags else 0
        sql.append(
            "INSERT INTO TBL_MAPINFO (FLD_MAPFILENAME,FLD_MAPNAME,FLD_SERVERINDEX,FLD_ATTRIBUTE,"
            "FLD_RECALLMAPFNAME,FLD_DESCRIPTION,FLD_MAPSERIES,FLD_MAPSERIESVAL) VALUES "
            f"({q(file_name)},{q(map_name)},1,0,'',{q(map_name)},{series},0);")

    sql.append('DELETE FROM TBL_STARTPOINT;')
    for index, (map_name, x, y) in enumerate(start_points(envir), start=1):
        sql.append(
            "INSERT INTO TBL_STARTPOINT (FLD_INDEX,FLD_MAPNAME,FLD_POSX,FLD_POSY,FLD_DESCRIPTION) VALUES "
            f"({index},{q(map_name)},{x},{y},'Mud3 StartPoint');")

    sql.append('DELETE FROM TBL_MONGEN;')
    missing = {}
    index = 0
    for map_name, x, y, monster, area, count, gentime in gen_records(envir):
        monster_id = monsters.get(normalize(monster))
        if monster_id is None:
            missing[monster] = missing.get(monster, 0) + 1
            continue
        index += 1
        sql.append(
            "INSERT INTO TBL_MONGEN (FLD_INDEX,FLD_MAPNAME,FLD_X,FLD_Y,FLD_MONNAME,FLD_AREA,FLD_COUNT,"
            "FLD_GENTIME,FLD_SMALLGENRATE,FLD_DESCRIPTION,FLD_MONINDEX,FLD_AREA_X,FLD_AREA_Y) VALUES "
            f"({index},{q(map_name)},{x},{y},{q(normalize(monster))},{area},{count},"
            f"{gentime},0,'',{monster_id},{area},{area});")

    sql.append('DELETE FROM TBL_MERCHANT;')
    for index, (script, map_name, x, y, name, face, body) in enumerate(merchant_records(envir), start=1):
        sql.append(
            "INSERT INTO TBL_MERCHANT (FLD_ID,FLD_MAPNAME,FLD_POSX,FLD_POSY,FLD_NPCNAME,FLD_SCRIPT,"
            "FLD_FACE,FLD_BODY,FLD_GENDER,FLD_DESCRIPTION) VALUES "
            f"({index},{q(map_name)},{int(x)},{int(y)},{q(name)},{q(script)},"
            f"{int(face)},{int(body)},0,'Mud3 merchant');")
    return sql, missing


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--envir', type=Path, required=True)
    parser.add_argument('--monsters', type=Path, required=True)
    parser.add_argument('--out', type=Path)
    parser.add_argument('--sqlite', type=Path)
    args = parser.parse_args()
    sql, missing = build(args.envir, args.monsters)
    if args.out:
        args.out.write_text('\n'.join(sql) + '\n', encoding='utf-8')
        print('SQL:', args.out, len(sql), 'statements')
    if missing:
        print('unmapped monster names:', len(missing), sorted(missing)[:10])
    if args.sqlite:
        with sqlite3.connect(args.sqlite) as database:
            database.execute('BEGIN')
            for statement in sql:
                if statement.startswith('--'):
                    continue
                database.execute(statement)
            database.commit()
            for table in ('TBL_MAPINFO', 'TBL_STARTPOINT', 'TBL_MONGEN'):
                print(table, database.execute(f'SELECT COUNT(*) FROM {table}').fetchone()[0])


if __name__ == '__main__':
    main()
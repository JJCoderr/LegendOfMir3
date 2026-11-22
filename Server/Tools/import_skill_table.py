"""把 GODOT 分支 docs/Skills.md 的 1.45 技能表与 System.db 的 MagicInfo 合并成 TBL_MAGIC。

Skills.md 提供 1.45 技能 ID、中文名与对应的 Zircon ID；MagicInfo 提供该技能的
消耗、等级与修炼数据。MIcon 索引由客户端按 2*(ID-1) 计算，无需入库。

用法:
  python Server/Tools/import_skill_table.py --skills docs/Skills.md --xlsx <System.db导出> \
      --out Server/SQL_Scripts/magic_data.sql
"""
import argparse
import re
from pathlib import Path

import openpyxl

ROW = re.compile(r'^\|\s*(\d+)\s*\|\s*([^|]*)\|\s*([^|]*)\|\s*([^|]*)\|\s*([^|]*)\|')


def q(text):
    cleaned = ' '.join(str(text).split())
    return "'" + cleaned.replace("'", "''") + "'"


def as_int(value, default=0):
    try:
        return int(float(value))
    except (TypeError, ValueError):
        return default


def parse_skills(path):
    records = []
    for line in path.read_text(encoding='utf-8').splitlines():
        found = ROW.match(line)
        if not found:
            continue
        skill_id = int(found.group(1))
        name = found.group(2).strip()
        zircon = found.group(4).strip()
        records.append((skill_id, name, as_int(zircon, -1) if zircon not in ('—', '-', '') else -1))
    return records


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--skills', type=Path, required=True)
    parser.add_argument('--xlsx', type=Path, required=True)
    parser.add_argument('--out', type=Path)
    args = parser.parse_args()

    skills = parse_skills(args.skills)
    workbook = openpyxl.load_workbook(args.xlsx, read_only=True)
    rows = workbook['MagicInfo'].iter_rows(values_only=True)
    header = list(next(rows))
    at = {name: header.index(name) for name in header}
    by_magic = {}
    for row in rows:
        magic = as_int(row[at['Magic']], -1)
        if magic >= 0:
            by_magic[magic] = row

    statements = ['-- 由 Server/Tools/import_skill_table.py 生成，请勿手改。', 'DELETE FROM TBL_MAGIC;']
    matched = 0
    for skill_id, name, zircon in skills:
        source = by_magic.get(zircon) if zircon >= 0 else None
        if source is not None:
            matched += 1
        power = as_int(source[at['MinBasePower']]) if source is not None else 0
        max_power = as_int(source[at['MaxBasePower']]) if source is not None else 0
        job = as_int(source[at['Class']]) if source is not None else 0
        school = as_int(source[at['School']]) if source is not None else 0
        description = source[at['Description']] if source is not None else ''
        statements.append(
            'INSERT INTO TBL_MAGIC (FLD_INDEX,FLD_NAME,FLD_EFFECTTYPE,FLD_EFFECT,FLD_SPELL,FLD_POWER,'
            'FLD_MAXPOWER,FLD_DEFSPELL,FLD_DEFPOWER,FLD_DEFMAXPOWER,FLD_JOB,FLD_NEEDL1,FLD_L1TRAIN,'
            'FLD_NEEDL2,FLD_L2TRAIN,FLD_NEEDL3,FLD_L3TRAIN,FLD_DELAY,FLD_DESC,FLD_DESCRIPTION) VALUES '
            '(%d,%s,%d,0,0,%d,%d,0,0,0,%d,%d,%d,%d,%d,%d,%d,%d,%s,%s);'
            % (skill_id, q(name), school, power, max_power, job,
               as_int(source[at['NeedLevel1']]) if source is not None else 0,
               as_int(source[at['Experience1']]) if source is not None else 0,
               as_int(source[at['NeedLevel2']]) if source is not None else 0,
               as_int(source[at['Experience2']]) if source is not None else 0,
               as_int(source[at['NeedLevel3']]) if source is not None else 0,
               as_int(source[at['Experience3']]) if source is not None else 0,
               as_int(source[at['Delay']]) if source is not None else 0,
               q(name), q(description)))

    if args.out:
        args.out.write_text('\n'.join(statements) + '\n', encoding='utf-8')
        print('skills', len(skills), 'matched zircon', matched, '->', args.out)
        print('unmatched:')
        for skill_id, name, zircon in skills:
            if zircon < 0 or zircon not in by_magic:
                print('  %d %s zircon=%s' % (skill_id, name, zircon))


if __name__ == '__main__':
    main()

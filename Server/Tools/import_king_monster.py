"""把 mir2x 的 King_Monster.csv（V1.45 原版怪物表）转换成 TBL_MONSTER。

用法:
  python Server/Tools/import_king_monster.py --csv <King_Monster.csv> \
      --out Server/SQL_Scripts/king_monster.sql
"""
import argparse
import csv
from pathlib import Path

DEFAULT_VIEW_RANGE = 10


def q(text):
    return "'" + ' '.join(str(text).split()).replace("'", "''") + "'"


def as_int(value, default=0):
    try:
        return int(float(value))
    except (TypeError, ValueError):
        return default


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--csv', type=Path, required=True)
    parser.add_argument('--out', type=Path)
    args = parser.parse_args()

    with args.csv.open(encoding='utf-8-sig', errors='replace', newline='') as handle:
        rows = list(csv.DictReader(handle))

    statements = ['-- 由 Server/Tools/import_king_monster.py 生成，数据来自 mir2x 的 King_Monster.csv。',
                  'DELETE FROM TBL_MONSTER;']
    index = 0
    for row in rows:
        name = (row.get('Name') or '').strip()
        if not name:
            continue
        index += 1
        race = as_int(row.get('Race'))
        appr = as_int(row.get('Appr'))
        statements.append(
            'INSERT INTO TBL_MONSTER (FLD_INDEX,FLD_NAME,FLD_RACE,FLD_RACEIMG,FLD_APPR,FLD_LEVEL,'
            'FLD_UNDEAD,FLD_EXP,FLD_HP,FLD_MP,FLD_AC,FLD_MAXAC,FLD_MAC,FLD_MAXMAC,FLD_DC,FLD_MAXDC,'
            'FLD_SPEED,FLD_HIT,FLD_WALKSPEED,FLD_ATTACKSPEED,FLD_VIEWRANGE,FLD_RACEINDEX,FLD_ESCAPE) VALUES '
            f'({index},{q(name)},{race},{as_int(row.get("RaceImg"))},{appr},{as_int(row.get("Lvl"))},'
            f'{as_int(row.get("Undead"))},{as_int(row.get("Exp"))},{as_int(row.get("HP"))},{as_int(row.get("MP"))},'
            f'{as_int(row.get("Ac"))},{as_int(row.get("Ac"))},{as_int(row.get("FireMAC"))},{as_int(row.get("FireMAC"))},'
            f'{as_int(row.get("Dc"))},{as_int(row.get("DcMax"))},{as_int(row.get("Speed"))},{as_int(row.get("Hit"))},'
            f'{as_int(row.get("WALK_SPD"))},{as_int(row.get("ATTACK_SPD"))},{DEFAULT_VIEW_RANGE},{race},0);')

    if args.out:
        args.out.write_text('\n'.join(statements) + '\n', encoding='utf-8')
        print(f'怪物 {index} 条 -> {args.out}')


if __name__ == '__main__':
    main()

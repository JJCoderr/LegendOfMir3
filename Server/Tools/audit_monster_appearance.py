"""核对怪物外观是否落在当前客户端资源（宝藏传奇3/Data）的 Mon-*.wil 库范围内。

用法: python Server/Tools/audit_monster_appearance.py --data <Data目录> --xlsx <System.db导出>
"""
import argparse
import re
import sqlite3
from pathlib import Path

import openpyxl


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--data', type=Path, required=True)
    parser.add_argument('--xlsx', type=Path, required=True)
    args = parser.parse_args()

    libraries = set()
    for path in args.data.glob('Mon-*.wil'):
        found = re.match(r'Mon-(\d+)\.wil$', path.name, re.IGNORECASE)
        if found:
            libraries.add(int(found.group(1)))
    print('Mon 库:', sorted(libraries))

    workbook = openpyxl.load_workbook(args.xlsx, read_only=True)
    rows = workbook['MonsterInfo'].iter_rows(values_only=True)
    header = list(next(rows))
    name_at, image_at, index_at = header.index('MonsterName'), header.index('Image'), header.index('Index')
    missing = []
    total = 0
    for row in rows:
        image = row[image_at]
        if image is None:
            continue
        total += 1
        library = int(image) // 10 + 1
        if library not in libraries:
            missing.append((int(row[index_at]), str(row[name_at]), int(image), library))
    print(f'怪物总数 {total}，外观库缺失 {len(missing)}')
    for entry in missing[:15]:
        print('  缺失 index=%d name=%s image=%d 需要 Mon-%d.wil' % entry)


if __name__ == '__main__':
    main()
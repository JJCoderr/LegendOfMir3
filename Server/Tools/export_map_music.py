"""从 System.db 导出生成 MapMusic.ini（地图文件名 → 背景音乐索引）。

用法: python Server/Tools/export_map_music.py --xlsx <xlsx> --out Server/GameData/MapMusic.ini
"""
import argparse
from pathlib import Path

import openpyxl


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--xlsx', type=Path, required=True)
    parser.add_argument('--out', type=Path, required=True)
    args = parser.parse_args()
    workbook = openpyxl.load_workbook(args.xlsx, read_only=True)
    rows = workbook['MapInfo'].iter_rows(values_only=True)
    header = list(next(rows))
    name_at, music_at = header.index('FileName'), header.index('Music')
    lines = ['[MapMusic]', '; 由 Server/Tools/export_map_music.py 生成，值为背景音乐索引，0 表示无音乐。']
    for row in rows:
        name = row[name_at]
        if not name:
            continue
        lines.append(f'{name}={int(row[music_at] or 0)}')
    args.out.parent.mkdir(parents=True, exist_ok=True)
    args.out.write_text('\n'.join(lines) + '\n', encoding='gbk')
    print('written', args.out, len(lines) - 2, 'maps')


if __name__ == '__main__':
    main()
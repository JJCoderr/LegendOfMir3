"""核对客户端 WIL 帧数，以及物品/技能图标索引是否落在帧范围内。

用法: python Server/Tools/audit_wil_references.py --data <Data目录> [--xlsx <System.db导出>]
"""
import argparse
import struct
from pathlib import Path

WIL_HEADER_VERSION_OFFSET = 22
WIL_HEADER_COUNT_OFFSET = 24


def wil_frames(path):
    raw = path.read_bytes()[:32]
    version, = struct.unpack_from('<h', raw, WIL_HEADER_VERSION_OFFSET)
    count, = struct.unpack_from('<i', raw, WIL_HEADER_COUNT_OFFSET)
    return version, count


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--data', type=Path, required=True)
    parser.add_argument('--xlsx', type=Path)
    args = parser.parse_args()

    frames = {}
    for path in sorted(args.data.glob('*.wil')):
        version, count = wil_frames(path)
        frames[path.name.lower()] = count
        print(f'{path.name:24s} ver={version:5d} frames={count}')
    for name in ('inventory.wil', 'micon.wil', 'magic.wil', 'monmagic.wil'):
        print(f'关键库 {name}: {frames.get(name)}')

    if not args.xlsx:
        return
    import openpyxl
    workbook = openpyxl.load_workbook(args.xlsx, read_only=True)

    inventory = frames.get('inventory.wil', 0)
    micon = frames.get('micon.wil', 0)
    magic_rows = list(workbook['MagicInfo'].iter_rows(values_only=True))
    magic_header = magic_rows[0]
    index_at = magic_header.index('Index')
    magic_ids = [int(row[index_at]) for row in magic_rows[1:] if row[index_at] is not None]
    if magic_ids:
        worst = max(2 * value - 2 for value in magic_ids)
        print(f'技能数 {len(magic_ids)}，图标帧 2*id-2 最大 {worst}（MIcon {micon}）',
              'OK' if worst < micon else '越界')

    item_rows = list(workbook['ItemInfo'].iter_rows(values_only=True))
    item_header = item_rows[0]
    shape_at = item_header.index('Shape')
    look_alike = [int(row[shape_at]) for row in item_rows[1:] if row[shape_at] is not None]
    if look_alike:
        worst = max(look_alike)
        print(f'物品数 {len(look_alike)}，Shape 最大 {worst}（Inventory {inventory}）',
              'OK' if worst < inventory else '越界（需按 wLooks 映射）')


if __name__ == '__main__':
    main()
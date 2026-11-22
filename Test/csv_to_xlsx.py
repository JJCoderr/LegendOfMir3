"""把现有 CSV 场景转换成 Excel（.xlsx），供 xlsx_scenario.py 驱动。

用例表放步骤，fixture 表放初始化 SQL，files 表嵌入同名 .files 的资源。

用法:
  python Test/csv_to_xlsx.py Test/Inventory/Inventory.csv
  python Test/csv_to_xlsx.py Test/Shop/ShopBuy.csv --out Test/场景-商店购买.xlsx
"""
import argparse
import base64
import csv
from pathlib import Path

import openpyxl

WIDTHS = (30, 8, 24, 10, 22, 34, 80)


def convert(source, target):
    workbook = openpyxl.Workbook()
    sheet = workbook.active
    sheet.title = source.stem
    with source.open(encoding='utf-8-sig', newline='') as handle:
        rows = list(csv.reader(handle))
    sheet.append(['编号', *rows[0]])
    for row in rows[1:]:
        sheet.append([f'{source.stem}.{row[0]}', *row])
    for column, width in zip('ABCDEFG', WIDTHS):
        sheet.column_dimensions[column].width = width

    fixture = source.with_suffix('.fixture.csv')
    if fixture.exists():
        data = workbook.create_sheet('fixture')
        with fixture.open(encoding='utf-8-sig', newline='') as handle:
            for row in csv.reader(handle):
                data.append(row)

    assets = source.with_suffix('.files')
    if assets.is_dir():
        data = workbook.create_sheet('files')
        data.append(['case', 'path', 'base64'])
        for path in sorted(assets.rglob('*')):
            if path.is_file():
                encoded = base64.b64encode(path.read_bytes()).decode('ascii')
                relative = path.relative_to(assets).as_posix()
                for offset in range(0, max(1, len(encoded)), 24000):
                    data.append([source.stem, relative, encoded[offset:offset+24000]])
    target.parent.mkdir(parents=True, exist_ok=True)
    workbook.save(target)
    return len(rows) - 1


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('source', type=Path, nargs='+')
    parser.add_argument('--out', type=Path)
    args = parser.parse_args()

    for source in args.source:
        target = args.out if (args.out and len(args.source) == 1) else Path('Test') / ('场景-' + source.stem + '.xlsx')
        steps = convert(source, target)
        print(f'{source} -> {target}: {steps} steps')


if __name__ == '__main__':
    main()

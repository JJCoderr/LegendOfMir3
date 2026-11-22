"""把现有 CSV 场景转换成 Excel（.xlsx），供 xlsx_scenario.py 驱动。

CSV 与 .xlsx 内容一一对应：scenario 表放步骤，fixture 表放初始化 SQL，
同名 .files 目录原样复制。

用法:
  python Test/csv_to_xlsx.py Test/Inventory/Inventory.csv
  python Test/csv_to_xlsx.py Test/Shop/ShopBuy.csv --out Test/Scenarios/ShopBuy.xlsx
"""
import argparse
import csv
import shutil
from pathlib import Path

import openpyxl

WIDTHS = (8, 24, 10, 22, 34, 80)


def convert(source, target):
    workbook = openpyxl.Workbook()
    sheet = workbook.active
    sheet.title = 'scenario'
    with source.open(encoding='utf-8-sig', newline='') as handle:
        rows = list(csv.reader(handle))
    for row in rows:
        sheet.append(row)
    for column, width in zip('ABCDEF', WIDTHS):
        sheet.column_dimensions[column].width = width

    fixture = source.with_suffix('.fixture.csv')
    if fixture.exists():
        data = workbook.create_sheet('fixture')
        with fixture.open(encoding='utf-8-sig', newline='') as handle:
            for row in csv.reader(handle):
                data.append(row)

    target.parent.mkdir(parents=True, exist_ok=True)
    workbook.save(target)
    assets = source.with_suffix('.files')
    if assets.is_dir():
        shutil.copytree(assets, target.with_suffix('.files'), dirs_exist_ok=True)
    return len(rows) - 1


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('source', type=Path, nargs='+')
    parser.add_argument('--out', type=Path)
    args = parser.parse_args()

    for source in args.source:
        target = args.out if (args.out and len(args.source) == 1) else Path('Test/Scenarios') / (source.stem + '.xlsx')
        steps = convert(source, target)
        print(f'{source} -> {target}: {steps} steps')


if __name__ == '__main__':
    main()

"""把现有 CSV 场景转换成 Excel（.xlsx），供 xlsx_scenario.py 驱动。

用例表放步骤，fixture 表放初始化 SQL，files 表嵌入同名 .files 的资源。

用法:
  python Test/csv_to_xlsx.py Test/Shop/ShopBuy.csv --function-id 201 --out out/ShopBuy.xlsx
"""
import argparse
import base64
import csv
from pathlib import Path

import openpyxl

from xlsx_scenario import STEP_BASE

WIDTHS = (16, 24, 10, 22, 34, 80)


def convert(source, target, function_id):
    workbook = openpyxl.Workbook()
    sheet = workbook.active
    sheet.title = source.stem
    with source.open(encoding='utf-8-sig', newline='') as handle:
        rows = list(csv.reader(handle))
    sheet.append(['编号', *rows[0][1:]])
    for step, *row in rows[1:]:
        if not 0 < int(step) < STEP_BASE:
            raise SystemExit(f'步骤号超出编号末四位的范围：{step}')
        sheet.append([function_id * STEP_BASE + int(step), *row])
    for column, width in zip('ABCDEF', WIDTHS):
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
    parser.add_argument('source', type=Path)
    parser.add_argument('--function-id', type=int, required=True, help='已分配的数字功能 ID')
    parser.add_argument('--out', type=Path)
    args = parser.parse_args()

    if not 0 < args.function_id < 10**11:
        parser.error('功能 ID 必须为正整数，且完整编号不得超过 Excel 的 15 位精度')
    target = args.out or Path('Test') / (args.source.stem + '.xlsx')
    steps = convert(args.source, target, args.function_id)
    print(f'{args.source} -> {target}: {steps} steps')


if __name__ == '__main__':
    main()

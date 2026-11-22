"""用 Excel（.xlsx）编写操作脚本，对运行中的客户端注入鼠标与键盘。

两种模式：
  --pid <客户端进程号>   直接对已运行的游戏发操作，不新建环境（默认用途）。
  省略 --pid             退回隔离环境模式，交给 launch_csv.py 建库起进程。

xlsx 结构：
  scenario 表（或第一张表）：step | name | target | action | params | expect
  fixture 表（可选，仅隔离环境模式）：单列 sql
"""
import argparse
import csv
import shutil
import subprocess
import sys
import tempfile
from pathlib import Path

import openpyxl

COLUMNS = ('step', 'name', 'target', 'action', 'params', 'expect')


def read_scenario(path):
    workbook = openpyxl.load_workbook(path, read_only=True, data_only=True)
    sheet = workbook['scenario'] if 'scenario' in workbook.sheetnames else workbook.worksheets[0]
    rows = list(sheet.iter_rows(values_only=True))
    header = [str(cell).strip().lower() if cell is not None else '' for cell in rows[0]]
    missing = [name for name in COLUMNS if name not in header]
    if missing:
        raise SystemExit(f'{path}: 缺少列 {missing}')
    position = {name: header.index(name) for name in COLUMNS}
    records = []
    for row in rows[1:]:
        if row is None or all(cell is None for cell in row):
            continue
        record = {name: ('' if row[index] is None else str(row[index]))
                  for name, index in position.items() if index < len(row)}
        if not record.get('step', '').strip():
            continue
        records.append(record)
    fixtures = []
    if 'fixture' in workbook.sheetnames:
        for row in list(workbook['fixture'].iter_rows(values_only=True))[1:]:
            if row and row[0] is not None and str(row[0]).strip():
                fixtures.append({'sql': str(row[0])})
    workbook.close()
    return records, fixtures


def write_scenario(records, fixtures, directory, name):
    scenario = directory / f'{name}.csv'
    with scenario.open('w', encoding='utf-8-sig', newline='') as target:
        writer = csv.DictWriter(target, fieldnames=COLUMNS)
        writer.writeheader()
        writer.writerows(records)
    if fixtures:
        with (directory / f'{name}.fixture.csv').open('w', encoding='utf-8-sig', newline='') as target:
            writer = csv.DictWriter(target, fieldnames=('sql',))
            writer.writeheader()
            writer.writerows(fixtures)
    return scenario


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('scenario', type=Path)
    parser.add_argument('--pid', type=int, help='运行中的客户端进程号，直接注入操作')
    parser.add_argument('--bin-dir', type=Path, default=Path('build-cui/Release'))
    parser.add_argument('--clients', type=int, default=1, choices=(1, 2, 3))
    parser.add_argument('--port-offset', type=int, default=20000, help='隔离环境端口偏移')
    parser.add_argument('--from', dest='first', type=int)
    parser.add_argument('--to', dest='last', type=int)
    parser.add_argument('--timeout-ms', type=int)
    parser.add_argument('--log', type=Path)
    args = parser.parse_args()

    records, fixtures = read_scenario(args.scenario)
    print(f'scenario: {len(records)} steps', flush=True)
    repo = Path(__file__).resolve().parents[1]
    output = repo / 'out'
    output.mkdir(exist_ok=True)
    args.log = args.log or output / f'{args.scenario.stem}.log'
    args.log.parent.mkdir(parents=True, exist_ok=True)
    args.log.write_text('', encoding='utf-8')
    # wire.expect 从场景文件向上查找仓库内的协议头文件。
    with tempfile.TemporaryDirectory(prefix='xlsx-scenario-', dir=output) as directory:
        generated = write_scenario(records, fixtures, Path(directory), args.scenario.stem)
        assets = args.scenario.with_suffix('.files')
        if assets.is_dir():
            shutil.copytree(assets, generated.with_suffix('.files'))
        if args.pid:
            if fixtures:
                raise SystemExit('直接注入模式不使用 fixture，请把数据准备放在游戏里')
            command = [str(args.bin_dir / 'Test.exe'), str(generated), '--client-pid', str(args.pid)]
            if args.first is not None:
                command.extend(['--from', str(args.first)])
            if args.last is not None:
                command.extend(['--to', str(args.last)])
            if args.timeout_ms is not None:
                command.extend(['--timeout-ms', str(args.timeout_ms)])
            if args.log is not None:
                args.log.parent.mkdir(parents=True, exist_ok=True)
                command.extend(['--log', str(args.log)])
            # 业务步骤不可自动整段重试，否则可能重复购买或删除角色。
            return subprocess.call(command)
        command = [sys.executable, str(Path(__file__).with_name('launch_csv.py')), str(generated),
                   '--bin-dir', str(args.bin_dir), '--clients', str(args.clients),
                   '--port-offset', str(args.port_offset)]
        if args.first is not None:
            command.extend(['--from', str(args.first)])
        if args.last is not None:
            command.extend(['--to', str(args.last)])
        if args.timeout_ms is not None:
            command.extend(['--timeout-ms', str(args.timeout_ms)])
        if args.log is not None:
            args.log.parent.mkdir(parents=True, exist_ok=True)
            command.extend(['--log', str(args.log)])
        return subprocess.call(command)


if __name__ == '__main__':
    sys.exit(main())

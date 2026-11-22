"""用 Excel（.xlsx）编写操作脚本，对运行中的客户端注入鼠标与键盘。

两种模式：
  --pid <客户端进程号>   直接对已运行的游戏发操作，不新建环境（默认用途）。
  省略 --pid             退回隔离环境模式，交给 launch_csv.py 建库起进程。

xlsx 结构：
  每个用例一张表：编号 | step | name | target | action | params | expect
  fixture 表（可选，仅隔离环境模式）：case | sql
  files 表（可选）：case | path | base64
"""
import argparse
import base64
import csv
import shutil
import subprocess
import sys
import tempfile
from pathlib import Path

import openpyxl

COLUMNS = ('step', 'name', 'target', 'action', 'params', 'expect')
SUPPORT_SHEETS = {'fixture', 'files'}


def read_scenario(workbook, case):
    sheet = workbook[case]
    rows = list(sheet.iter_rows(values_only=True))
    header = [str(cell).strip().lower() if cell is not None else '' for cell in rows[0]]
    missing = [name for name in COLUMNS if name not in header]
    if missing:
        raise SystemExit(f'{case}: 缺少列 {missing}')
    position = {name: header.index(name) for name in COLUMNS}
    identifier = header.index('编号') if '编号' in header else None
    records = []
    for row in rows[1:]:
        if row is None or all(cell is None for cell in row):
            continue
        record = {name: ('' if row[index] is None else str(row[index]))
                  for name, index in position.items() if index < len(row)}
        if not record.get('step', '').strip():
            continue
        if identifier is not None and row[identifier] != f"{case}.{record['step']}":
            raise SystemExit(f"{case}: 步骤 {record['step']} 的编号应为 {case}.{record['step']}")
        records.append(record)
    fixtures = []
    if 'fixture' in workbook.sheetnames:
        rows = workbook['fixture'].values
        if next(rows, ('',))[0] == 'case':
            fixtures = [{'sql': str(sql)} for name, sql in rows if name == case and sql]
        else:
            fixtures = [{'sql': str(row[0])} for row in rows if row[0]]
    return records, fixtures


def extract_assets(workbook, case, directory):
    if 'files' not in workbook.sheetnames:
        return
    for name, relative, encoded in workbook['files'].iter_rows(min_row=2, values_only=True):
        if name != case:
            continue
        relative = Path(relative)
        if relative.anchor or '..' in relative.parts:
            raise SystemExit(f'无效的资源路径：{relative}')
        target = directory / relative
        target.parent.mkdir(parents=True, exist_ok=True)
        with target.open('ab') as stream:
            stream.write(base64.b64decode(encoded or '', validate=True))


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
    parser.add_argument('--case', help='子系统工作簿的用例工作表名')
    parser.add_argument('--list', action='store_true', help='列出工作簿中的用例')
    parser.add_argument('--pid', type=int, help='运行中的客户端进程号，直接注入操作')
    parser.add_argument('--bin-dir', type=Path, default=Path('build-cui/Release'))
    parser.add_argument('--data-sql', type=Path, action='append',
        help='隔离环境额外导入的数据脚本（可重复），例如 King 系列的怪物/物品/技能')
    parser.add_argument('--clients', type=int, default=1, choices=(1, 2, 3))
    parser.add_argument('--port-offset', type=int, default=20000, help='隔离环境端口偏移')
    parser.add_argument('--from', dest='first', type=int)
    parser.add_argument('--to', dest='last', type=int)
    parser.add_argument('--timeout-ms', type=int)
    parser.add_argument('--log', type=Path)
    args = parser.parse_args()

    workbook = openpyxl.load_workbook(args.scenario, read_only=True, data_only=True)
    cases = [name for name in workbook.sheetnames if name not in SUPPORT_SHEETS]
    if args.list:
        print('\n'.join(cases))
        workbook.close()
        return 0
    case = args.case or ('scenario' if 'scenario' in cases else cases[0] if len(cases) == 1 else None)
    if case not in cases:
        workbook.close()
        parser.error('请使用 --case 选择用例：' + ', '.join(cases))
    records, fixtures = read_scenario(workbook, case)
    print(f'scenario: {len(records)} steps', flush=True)
    repo = Path(__file__).resolve().parents[1]
    output = repo / 'out'
    output.mkdir(exist_ok=True)
    name = args.scenario.stem if case == 'scenario' else case
    args.log = args.log or output / f'{name}.log'
    args.log.parent.mkdir(parents=True, exist_ok=True)
    args.log.write_text('', encoding='utf-8')
    # wire.expect 从场景文件向上查找仓库内的协议头文件。
    with tempfile.TemporaryDirectory(prefix='xlsx-scenario-', dir=output) as directory:
        generated = write_scenario(records, fixtures, Path(directory), name)
        assets = args.scenario.with_suffix('.files')
        if 'files' in workbook.sheetnames:
            extract_assets(workbook, case, generated.with_suffix('.files'))
        elif assets.is_dir():
            shutil.copytree(assets, generated.with_suffix('.files'))
        workbook.close()
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
        for script in (args.data_sql or ()):
            command.extend(['--data-sql', str(script)])
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

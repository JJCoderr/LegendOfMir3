"""用 Excel（.xlsx）编写操作脚本，对运行中的客户端注入鼠标与键盘。

两种模式：
  --pid <客户端进程号>   直接对已运行的游戏发操作，不新建环境（默认用途）。
  省略 --pid             退回隔离环境模式，交给 launch_csv.py 建库起进程。

xlsx 结构：
  Rules 表或独立对话框表：编号 | name | target | action | params | expect
  编号末四位是步骤号，前面的数字是功能编号。
  Cases 表将功能编号映射到可独立执行的规则段。
  fixture 表（可选，仅隔离环境模式）：case | sql
  files 表（可选）：case | path | encoding | text
"""
import argparse
import csv
import subprocess
import sys
import tempfile
from pathlib import Path

import openpyxl

COLUMNS = ('step', 'name', 'target', 'action', 'params', 'expect')
SUPPORT_SHEETS = {'fixture', 'files', 'Cases'}
STEP_BASE = 10_000


def decode_number(value):
    number = str(value)
    if not number.isascii() or not number.isdecimal():
        raise SystemExit(f'编号必须是纯数字：{number}')
    function_id, step = divmod(int(number), STEP_BASE)
    if not function_id or not step:
        raise SystemExit(f'功能编号或步骤号无效：{number}')
    return function_id, step


def read_rule_cases(workbook):
    index = dict(workbook['Cases'].iter_rows(min_row=2, values_only=True))
    rows = workbook['Rules'].values
    header = next(rows)
    identifier = header.index('编号')
    cases = {name: [header] for name in index.values()}
    for row in rows:
        if all(cell is None for cell in row):
            continue
        function_id, _ = decode_number(row[identifier])
        if function_id not in index:
            raise SystemExit(f'Cases 中缺少功能编号：{function_id}')
        cases[index[function_id]].append(row)
    return cases


def read_scenario(workbook, case, rule_cases=None):
    rows = rule_cases[case] if rule_cases is not None else list(workbook[case].values)
    header = [str(cell).strip().lower() if cell is not None else '' for cell in rows[0]]
    encoded = 'step' not in header
    required = ('编号', *COLUMNS[1:]) if encoded else COLUMNS
    missing = [name for name in required if name not in header]
    if missing:
        raise SystemExit(f'{case}: 缺少列 {missing}')
    position = {name: header.index(name) for name in COLUMNS if name in header}
    identifier = header.index('编号') if '编号' in header else None
    records = []
    function_id = None
    for row in rows[1:]:
        if row is None or all(cell is None for cell in row):
            continue
        record = {name: ('' if row[index] is None else str(row[index]))
                  for name, index in position.items() if index < len(row)}
        if encoded:
            prefix, step = decode_number(row[identifier])
            if function_id is not None and prefix != function_id:
                raise SystemExit(f'{case}: 同一规则段包含多个功能编号')
            function_id = prefix
            record['step'] = str(step)
        if not record.get('step', '').strip():
            continue
        if not encoded and identifier is not None and row[identifier] != f"{case}.{record['step']}":
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
    for name, relative, encoding, text in workbook['files'].iter_rows(min_row=2, values_only=True):
        if name != case:
            continue
        relative = Path(relative)
        if relative.anchor or '..' in relative.parts:
            raise SystemExit(f'无效的资源路径：{relative}')
        target = directory / relative
        target.parent.mkdir(parents=True, exist_ok=True)
        target.write_bytes((text or '').replace('\n', '\r\n').encode(encoding))


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


def materialize_case(workbook, case, directory, rule_cases=None, name=None):
    records, fixtures = read_scenario(workbook, case, rule_cases)
    scenario = write_scenario(records, fixtures, directory, name or case)
    extract_assets(workbook, case, scenario.with_suffix('.files'))
    return scenario, records, fixtures


def export_case(workbook_path, case, directory):
    workbook = openpyxl.load_workbook(workbook_path, read_only=True, data_only=True)
    try:
        rules = read_rule_cases(workbook) if 'Cases' in workbook.sheetnames else None
        return materialize_case(workbook, case, directory, rules)
    finally:
        workbook.close()


def run_case(args, workbook, case, rule_cases):
    repo = Path(__file__).resolve().parents[1]
    output = repo / 'out'
    output.mkdir(exist_ok=True)
    name = args.scenario.stem if case == 'scenario' else case
    log = args.log or output / f'{name}.log'
    log.parent.mkdir(parents=True, exist_ok=True)
    # wire.expect 从场景文件向上查找仓库内的协议头文件。
    with tempfile.TemporaryDirectory(prefix='xlsx-scenario-', dir=output) as directory:
        generated, records, fixtures = materialize_case(workbook, case, Path(directory), rule_cases, name)
        print(f'{case}: {len(records)} steps', flush=True)
        if args.pid:
            if fixtures:
                raise SystemExit('直接注入模式不使用 fixture，请把数据准备放在游戏里')
            command = [str(args.bin_dir / 'Test.exe'), str(generated), '--client-pid', str(args.pid)]
        else:
            clients = args.clients or max(({'Client2': 2, 'Client3': 3}.get(r['target'], 1) for r in records), default=1)
            command = [sys.executable, str(Path(__file__).with_name('launch_csv.py')), str(generated),
                       '--bin-dir', str(args.bin_dir), '--clients', str(clients),
                       '--port-offset', str(args.port_offset)]
            for script in (args.data_sql or ()):
                command.extend(['--data-sql', str(script)])
            if args.data_then_fixture:
                command.append('--data-then-fixture')
            for flag, value in (('--resume-runtime', args.resume_runtime), ('--config-dir', args.config_dir)):
                if value is not None:
                    command.extend([flag, str(value)])
        if args.first is not None:
            command.extend(['--from', str(args.first)])
        if args.last is not None:
            command.extend(['--to', str(args.last)])
        if args.timeout_ms is not None:
            command.extend(['--timeout-ms', str(args.timeout_ms)])
        command.extend(['--log', str(log)])
        return subprocess.call(command)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('scenario', type=Path)
    parser.add_argument('--case', help='只运行指定规则段或对话框表')
    parser.add_argument('--list', action='store_true', help='列出规则段或对话框表')
    parser.add_argument('--pid', type=int, help='运行中的客户端进程号，直接注入操作')
    parser.add_argument('--bin-dir', type=Path, default=Path('build-cui/Release'))
    parser.add_argument('--data-sql', type=Path, action='append', help='隔离环境额外导入的数据脚本（可重复）')
    parser.add_argument('--data-then-fixture', action='store_true', help='先导入数据脚本，再应用 fixture')
    parser.add_argument('--config-dir', type=Path, help='隔离运行环境的配置模板')
    parser.add_argument('--resume-runtime', type=Path, help='复用前一阶段的隔离运行环境')
    parser.add_argument('--clients', type=int, choices=(1, 2, 3))
    parser.add_argument('--port-offset', type=int, default=20000, help='隔离环境端口偏移')
    parser.add_argument('--from', dest='first', type=int)
    parser.add_argument('--to', dest='last', type=int)
    parser.add_argument('--timeout-ms', type=int)
    parser.add_argument('--log', type=Path)
    args = parser.parse_args()

    workbook = openpyxl.load_workbook(args.scenario, read_only=True, data_only=True)
    try:
        rule_cases = read_rule_cases(workbook) if 'Cases' in workbook.sheetnames else None
        cases = list(rule_cases) if rule_cases is not None else [name for name in workbook.sheetnames if name not in SUPPORT_SHEETS]
        if args.list:
            print('\n'.join(cases))
            return 0
        case = args.case or ('scenario' if 'scenario' in cases else cases[0] if len(cases) == 1 else None)
        if case not in cases:
            parser.error('请使用 --case 选择规则段：' + ', '.join(cases))
        return run_case(args, workbook, case, rule_cases)
    finally:
        workbook.close()


if __name__ == '__main__':
    sys.exit(main())

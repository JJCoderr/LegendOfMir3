"""Prepare an isolated runtime and run a CSV scenario with the native Test.exe."""
import argparse
import csv
import ctypes
import json
import subprocess
import shutil
import time
from pathlib import Path
from rpc_fixture import prepare
from protocol_catalog import definitions, coverage


def main():
    repo = Path(__file__).resolve().parents[1]
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('scenario', type=Path)
    parser.add_argument('--bin-dir', type=Path, default=repo/'build-eiserver/Release')
    parser.add_argument('--config-dir', type=Path, default=repo/'out/debug')
    parser.add_argument('--port-offset', type=int, default=20000)
    parser.add_argument('--from', dest='first', type=int, default=1)
    parser.add_argument('--to', dest='last', type=int)
    parser.add_argument('--log', type=Path)
    parser.add_argument('--resume-runtime', type=Path, help='Reuse an isolated runtime without reapplying fixtures')
    parser.add_argument('--clients', type=int, choices=(1,2,3), default=1)
    parser.add_argument('--timeout-ms', type=int, help='RPC/state wait timeout per step; forwarded to Test.exe')
    args = parser.parse_args()
    binaries = args.bin_dir.resolve()
    if not (binaries/'Test.exe').is_file():
        raise FileNotFoundError(binaries/'Test.exe')
    if args.resume_runtime:
        root = args.resume_runtime.resolve(strict=True)
    else:
        fixture = args.scenario.with_suffix('.fixture.csv')
        statements = []
        if fixture.exists():
            with fixture.open(encoding='utf-8-sig', newline='') as source:
                statements = list(csv.DictReader(source))
        root = prepare(repo, binaries, args.config_dir.resolve(), {'fixture':statements}, args.port_offset)
        assets = args.scenario.with_suffix('.files')
        if assets.is_dir(): shutil.copytree(assets, root, dirs_exist_ok=True)
    log_path = args.log.resolve() if args.log else root/'test.log'
    print('Runtime:', root, flush=True)
    processes = []
    started = time.monotonic()
    result = {'status':'running', 'log':str(log_path)}
    try:
        with (root/'runtime.log').open('wb') as output:
            def start(role, binary):
                process = subprocess.Popen([str(binary), '--debug-pipe'], cwd=root, stdout=output, stderr=output,
                    creationflags=subprocess.CREATE_NO_WINDOW if role=='Server' else 0)
                processes.append(process)
                deadline = time.monotonic()+20
                while not ctypes.windll.kernel32.WaitNamedPipeW(rf'\\.\pipe\Mir3.{role}.{process.pid}', 100):
                    if process.poll() is not None: raise RuntimeError(f'{role} exited {process.returncode}')
                    if time.monotonic()>=deadline: raise TimeoutError(f'{role} pipe startup')
                    time.sleep(.05)
                return process.pid
            server = start('Server', binaries/'EIServer.exe')
            subprocess.run([str(binaries/'Test.exe'), str(repo/'Test/Rpc/RpcServer.csv'),
                '--server-pid', str(server), '--log', str(root/'server-preflight.log')], check=True)
            client = start('Client', root/'Mir2Ex.exe')
            command = [str(binaries/'Test.exe'), str(args.scenario.resolve()), '--client-pid', str(client),
                '--server-pid', str(server), '--database', str(root/'mir3.sqlite3'),
                '--from', str(args.first), '--log', str(log_path)]
            for index in range(2,args.clients+1):
                command.extend([f'--client{index}-pid', str(start('Client', root/'Mir2Ex.exe'))])
            if args.last is not None: command.extend(['--to', str(args.last)])
            if args.timeout_ms is not None: command.extend(['--timeout-ms', str(args.timeout_ms)])
            subprocess.run(command, check=True)
        result['status'] = 'passed'
    except Exception as error:
        result.update(status='failed', error=str(error))
        raise
    finally:
        # Stop the server first: client teardown must not trigger a disconnect save.
        for process in processes:
            if process.poll() is None: process.terminate()
            process.wait(timeout=5)
        result['elapsed_seconds'] = round(time.monotonic()-started, 3)
        if log_path.exists():
            lines = log_path.read_text(encoding='utf-8').splitlines()
            result['passed_checks'] = sum(' PASS ' in line for line in lines)
            for line in reversed(lines):
                if ' response=' not in line: continue
                response = json.loads(line.split(' response=', 1)[1])
                if isinstance(response, dict) and 'observed' in response.get('result', {}):
                    report = coverage(definitions(repo), response['result']['observed'])
                    (root/'protocol-coverage.json').write_text(json.dumps(report, ensure_ascii=False, indent=2), encoding='utf-8')
                    break
        (root/'result.json').write_text(json.dumps(result, ensure_ascii=False, indent=2), encoding='utf-8')


if __name__ == '__main__':
    main()

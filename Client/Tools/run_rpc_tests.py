"""Run JSON test cases against a real process started with --debug-pipe."""
import argparse
import ctypes
import json
import msvcrt
import queue
import threading
import time
import subprocess
import sqlite3
from pathlib import Path
from ctypes import wintypes

def load_suite(path):
    path = Path(path)
    suite = json.loads(path.read_text(encoding='utf-8'))
    inherited = [load_suite(path.parent/name) for name in suite.get('include', [])]
    for key in ('cases','fixture','database_expect','wire_expect'):
        suite[key] = [entry for base in inherited for entry in base.get(key, [])] + suite.get(key, [])
    return suite

def run(suite_path, pid, report=None):
    suite = load_suite(suite_path)
    name = rf'\\.\pipe\Mir3.{suite["role"]}.{pid}'
    if not ctypes.windll.kernel32.WaitNamedPipeW(name, 10000):
        raise ctypes.WinError()
    with open(name, 'r+b', buffering=0) as pipe:
        replies = queue.Queue()
        def read():
            try:
                line = bytearray()
                while chunk := pipe.read(1):
                    if chunk == b'\n':
                        replies.put(json.loads(line)); return
                    else:
                        line.extend(chunk)
                replies.put(EOFError('RPC pipe closed before a response'))
            except Exception as error:
                replies.put(error)
        def exchange(request):
            pipe.write((request + '\n').encode('utf-8'))
            reader = threading.Thread(target=read, daemon=True)
            reader.start()
            try:
                response = replies.get(timeout=65)
            except queue.Empty:
                ctypes.windll.kernel32.CancelIoEx(wintypes.HANDLE(msvcrt.get_osfhandle(pipe.fileno())), None)
                reader.join(timeout=1)
                raise TimeoutError('RPC response')
            if isinstance(response, Exception): raise response
            return response
        def matches(response, expected):
            for key, value in expected.items():
                actual = response
                try:
                    for part in key.split('.'):
                        actual = actual[int(part)] if isinstance(actual, list) else actual[part]
                except (KeyError, IndexError, TypeError): return False
                if actual != value: return False
            return True
        after = 0
        for case in suite['cases']:
            if 'await_state' in case:
                deadline = time.monotonic() + 30
                current = exchange(json.dumps({'jsonrpc':'2.0','id':case['name'],'method':'state.get'}))
                satisfied = matches(current.get('result', {}), case['await_state'])
                while not satisfied:
                    response = exchange(json.dumps({'jsonrpc':'2.0','id':case['name'], 'method':'events.wait',
                        'params':{'after':after,'timeout_ms':10000}}))
                    assert 'result' in response, (case['name'], response)
                    after = response['result']['sequence']
                    states = [e['data'] for e in response['result']['events'] if e['kind']=='state']
                    if states and matches(states[-1], case['await_state']): break
                    assert time.monotonic() < deadline, (case['name'], response)
            else:
                response = exchange(case.get('raw', json.dumps(case.get('request'), ensure_ascii=False)))
                assert matches(response, case['expect']), (case['name'], case['expect'], response)
                if isinstance(response, dict) and 'sequence' in response.get('result', {}):
                    after = response['result']['sequence']
            print('PASS:', suite['role'], case['name'], flush=True)
            if report is not None: report.append({'role':suite['role'],'case':case['name'],'status':'passed'})
        if suite['role']=='Client':
            from protocol_catalog import definitions, number, coverage
            entries = definitions(Path(__file__).resolve().parents[2])
            wire = exchange(json.dumps({'jsonrpc':'2.0','id':'wire coverage','method':'protocol.get'}))
            observed = wire['result']['observed']
            for check in suite.get('wire_expect', []):
                opcode = number(entries, check['message'])
                count = sum(e['count'] for e in observed if e['direction']==check['direction'] and e['opcode']==opcode)
                assert count >= check.get('at_least',1), (check,count)
                if report is not None: report.append({'role':'Wire','case':check['message'],'status':'passed','count':count})
                print('PASS: Wire',check['direction'],check['message'],count,flush=True)
            return coverage(entries,observed)

def launch(args):
    from rpc_fixture import prepare
    repo = Path(__file__).resolve().parents[2]
    suite = load_suite(args.suite)
    root = prepare(repo,args.bin_dir.resolve(),args.config_dir.resolve(),suite,args.port_offset)
    print('Runtime:',root,flush=True)
    processes, report = [], []
    started = time.monotonic()
    result = {'status':'running','cases':report}
    try:
        with (root/'runtime.log').open('wb') as log:
            for role, binary, cases in (
                ('Server',args.bin_dir.resolve()/'EIServer.exe',repo/'Server/Test/jsonrpc.json'),
                ('Client',root/'Mir2Ex.exe',args.suite)):
                process = subprocess.Popen([str(binary),'--debug-pipe'],cwd=root,stdout=log,stderr=log,
                    creationflags=subprocess.CREATE_NO_WINDOW if role=='Server' else 0)
                processes.append(process)
                deadline = time.monotonic()+20
                while not ctypes.windll.kernel32.WaitNamedPipeW(rf'\\.\pipe\Mir3.{role}.{process.pid}',100):
                    if process.poll() is not None: raise RuntimeError(f'{role} exited {process.returncode}')
                    if time.monotonic()>=deadline: raise TimeoutError(f'{role} debug pipe startup')
                    time.sleep(.05)
                wire = run(cases,process.pid,report)
                if wire is not None:
                    (root/'protocol-coverage.json').write_text(json.dumps(wire,ensure_ascii=False,indent=2),encoding='utf-8')
            with sqlite3.connect(root/'mir3.sqlite3') as db:
                for check in suite.get('database_expect', []):
                    rows = [list(row) for row in db.execute(check['sql'],check.get('params', []))]
                    assert rows == check['rows'], (check['name'],rows,check['rows'])
                    report.append({'role':'Database','case':check['name'],'status':'passed'})
                    print('PASS: Database',check['name'],flush=True)
        result['status']='passed'
    except Exception as error:
        result['status']='failed'
        result['error']=str(error)
        raise
    finally:
        for process in reversed(processes):
            if process.poll() is None: process.terminate()
            process.wait(timeout=5)
        result['elapsed_seconds']=round(time.monotonic()-started,3)
        (root/'result.json').write_text(json.dumps(result,ensure_ascii=False,indent=2),encoding='utf-8')

if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('suite', type=Path)
    mode = parser.add_mutually_exclusive_group(required=True)
    mode.add_argument('--pid', type=int)
    mode.add_argument('--launch', action='store_true')
    repo = Path(__file__).resolve().parents[2]
    parser.add_argument('--bin-dir',type=Path,default=repo/'build-eiserver/Release')
    parser.add_argument('--config-dir',type=Path,default=repo/'out/debug')
    parser.add_argument('--port-offset',type=int,default=20000)
    args = parser.parse_args()
    if args.launch: launch(args)
    else: run(args.suite, args.pid)

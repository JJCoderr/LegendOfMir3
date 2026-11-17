"""Run JSON test cases against a real process started with --debug-pipe."""
import argparse
import ctypes
import json
import msvcrt
import queue
import threading
import time
import subprocess
from pathlib import Path
from ctypes import wintypes

def run(suite_path, pid, report=None):
    suite = json.loads(Path(suite_path).read_text(encoding='utf-8'))
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
                while True:
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
            print('PASS:', suite['role'], case['name'], flush=True)
            if report is not None: report.append({'role':suite['role'],'case':case['name'],'status':'passed'})

def launch(args):
    from rpc_fixture import prepare
    repo = Path(__file__).resolve().parents[2]
    suite = json.loads(args.suite.read_text(encoding='utf-8'))
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
                run(cases,process.pid,report)
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

"""Run JSON test cases against a real process started with --debug-pipe."""
import argparse
import ctypes
import json
import msvcrt
import queue
import threading
from pathlib import Path
from ctypes import wintypes

def run(suite_path, pid):
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
        for case in suite['cases']:
            request = case.get('raw', json.dumps(case.get('request'), ensure_ascii=False))
            pipe.write((request + '\n').encode('utf-8'))
            reader = threading.Thread(target=read, daemon=True)
            reader.start()
            try:
                response = replies.get(timeout=10)
            except queue.Empty:
                ctypes.windll.kernel32.CancelIoEx(wintypes.HANDLE(msvcrt.get_osfhandle(pipe.fileno())), None)
                reader.join(timeout=1)
                raise TimeoutError(case['name'])
            if isinstance(response, Exception): raise response
            for key, expected in case['expect'].items():
                actual = response
                for part in key.split('.'):
                    actual = actual[int(part)] if isinstance(actual, list) else actual[part]
                assert actual == expected, (case['name'], key, expected, response)
            print('PASS:', suite['role'], case['name'], flush=True)

if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('suite', type=Path)
    parser.add_argument('--pid', type=int, required=True)
    args = parser.parse_args()
    run(args.suite, args.pid)

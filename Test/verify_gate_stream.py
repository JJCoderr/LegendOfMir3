"""Run a real CSV scenario through deliberately fragmented gateway replies."""
import argparse
import configparser
import csv
import select
import shutil
import socket
import subprocess
import sys
import threading
import time
from pathlib import Path
from rpc_fixture import prepare


class FragmentProxy:
    def __init__(self, upstream):
        self.upstream = upstream
        self.listener = socket.socket()
        self.listener.bind(('127.0.0.1', 0))
        self.listener.listen(1)
        self.listener.settimeout(.2)
        self.port = self.listener.getsockname()[1]
        self.stop = threading.Event()
        self.error = None
        self.fragments = 0
        self.thread = threading.Thread(target=self.run, daemon=True)
        self.thread.start()

    def run(self):
        try:
            while not self.stop.is_set():
                try:
                    downstream, _ = self.listener.accept()
                    break
                except socket.timeout:
                    continue
            else:
                return
            with downstream, socket.create_connection(('127.0.0.1', self.upstream), timeout=5) as upstream:
                downstream.setsockopt(socket.IPPROTO_TCP, socket.TCP_NODELAY, 1)
                while not self.stop.is_set():
                    ready, _, _ = select.select([downstream, upstream], [], [], .2)
                    for source in ready:
                        data = source.recv(8192)
                        if not data:
                            return
                        if source is downstream:
                            upstream.sendall(data)
                        else:
                            # Include the server's NUL padding, even when it arrives alone.
                            for value in data:
                                if self.stop.is_set(): return
                                downstream.sendall(bytes([value]))
                                self.fragments += 1
                                time.sleep(.002)
        except ConnectionResetError:
            # launch_csv terminates the server before closing its clients.
            return
        except OSError as error:
            if not self.stop.is_set(): self.error = str(error)

    def close(self):
        self.stop.set()
        self.thread.join(timeout=6)
        self.listener.close()


def main():
    repo = Path(__file__).resolve().parents[1]
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--scenario', type=Path, default=repo/'Test/Login/Login.csv')
    parser.add_argument('--clients', type=int, default=1, choices=(1, 2, 3))
    parser.add_argument('--timeout-ms', type=int, default=15000)
    parser.add_argument('--log-dir', type=Path, default=repo/'out/gate-stream')
    args = parser.parse_args()
    args.log_dir.mkdir(parents=True, exist_ok=True)
    with args.scenario.with_suffix('.fixture.csv').open(encoding='utf-8-sig', newline='') as source:
        fixture = list(csv.DictReader(source))
    root = prepare(repo, repo/'build-eiserver/Release', repo/'out/debug', {'fixture': fixture}, 20000)
    assets = args.scenario.with_suffix('.files')
    if assets.is_dir(): shutil.copytree(assets, root, dirs_exist_ok=True)
    config = configparser.ConfigParser(interpolation=None)
    config.optionxform = str
    config.read(root/'EIServer.ini', encoding='utf-8')
    proxies = []
    try:
        for section in ('LoginGate', 'SelGate'):
            proxy = FragmentProxy(int(config[section]['RemotePort']))
            proxies.append((section, proxy))
            config[section]['RemotePort'] = str(proxy.port)
        with (root/'EIServer.ini').open('w', encoding='utf-8') as target:
            config.write(target, space_around_delimiters=False)
        with (args.log_dir/'launcher.log').open('w', encoding='utf-8') as output:
            result = subprocess.run([sys.executable, str(repo/'Test/launch_csv.py'), str(args.scenario.resolve()),
                '--resume-runtime', str(root), '--clients', str(args.clients), '--timeout-ms', str(args.timeout_ms),
                '--log', str((args.log_dir/'test.log').resolve())], stdout=output, stderr=subprocess.STDOUT)
    finally:
        for _, proxy in proxies: proxy.close()
    for section, proxy in proxies:
        print(f'{section}: fragments={proxy.fragments} error={proxy.error}')
    print('Runtime:', root)
    print('Logs:', args.log_dir)
    if result.returncode or any(p.error or not p.fragments for _, p in proxies): return 1
    print('PASS: real client/server scenario with fragmented login and selection replies')
    return 0


if __name__ == '__main__':
    sys.exit(main())

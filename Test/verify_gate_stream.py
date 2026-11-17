"""Run a real CSV scenario through deliberately fragmented gateway traffic."""
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
    def __init__(self, upstream, direction):
        self.upstream = upstream
        self.direction = direction
        self.listener = socket.socket()
        self.listener.bind(('127.0.0.1', 0))
        self.listener.listen(1)
        self.listener.settimeout(.2)
        self.port = self.listener.getsockname()[1]
        self.stop = threading.Event()
        self.error = None
        self.fragments = {'requests': 0, 'replies': 0}
        self.workers = []
        self.thread = threading.Thread(target=self.run, daemon=True)
        self.thread.start()

    def run(self):
        while not self.stop.is_set():
            try:
                downstream, _ = self.listener.accept()
            except socket.timeout:
                continue
            worker = threading.Thread(target=self.transfer, args=(downstream,), daemon=True)
            self.workers.append(worker)
            worker.start()

    def transfer(self, downstream):
        try:
            with downstream, socket.create_connection(('127.0.0.1', self.upstream), timeout=5) as upstream:
                downstream.setsockopt(socket.IPPROTO_TCP, socket.TCP_NODELAY, 1)
                upstream.setsockopt(socket.IPPROTO_TCP, socket.TCP_NODELAY, 1)
                while not self.stop.is_set():
                    ready, _, _ = select.select([downstream, upstream], [], [], .2)
                    for source in ready:
                        data = source.recv(8192)
                        if not data:
                            return
                        direction = 'requests' if source is downstream else 'replies'
                        destination = upstream if source is downstream else downstream
                        if self.direction not in (direction, 'both'):
                            destination.sendall(data)
                        else:
                            # Preserve padding and framing bytes in both directions.
                            for value in data:
                                if self.stop.is_set(): return
                                destination.sendall(bytes([value]))
                                self.fragments[direction] += 1
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
        for worker in self.workers: worker.join(timeout=6)


def main():
    repo = Path(__file__).resolve().parents[1]
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--scenario', type=Path, default=repo/'Test/Login/Login.csv')
    parser.add_argument('--edge', choices=('gateway', 'client'), default='gateway')
    parser.add_argument('--direction', choices=('requests', 'replies', 'both'), default='replies')
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
    client_config = configparser.ConfigParser(interpolation=None)
    client_config.optionxform = str
    client_config.read(root/'Mir3.ini', encoding='utf-8')
    proxies = []
    try:
        for section, client_key in (('LoginGate', 'Param1'), ('SelGate', 'SelectGatePort')):
            port_key = 'RemotePort' if args.edge == 'gateway' else 'LocalPort'
            proxy = FragmentProxy(int(config[section][port_key]), args.direction)
            proxies.append((section, proxy))
            if args.edge == 'gateway': config[section]['RemotePort'] = str(proxy.port)
            else: client_config['Initial'][client_key] = str(proxy.port)
        with (root/'Mir3.ini').open('w', encoding='utf-8') as target:
            client_config.write(target, space_around_delimiters=False)
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
    directions = ('requests', 'replies') if args.direction == 'both' else (args.direction,)
    if result.returncode or any(p.error or any(not p.fragments[d] for d in directions) for _, p in proxies): return 1
    print('PASS: real client/server scenario with fragmented login and selection traffic')
    return 0


if __name__ == '__main__':
    sys.exit(main())

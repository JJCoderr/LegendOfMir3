"""Run a workbook scenario through deliberately fragmented gateway traffic."""
import argparse
import configparser
import os
import select
import socket
import subprocess
import sys
import threading
import time
from pathlib import Path
from rpc_fixture import prepare
from xlsx_scenario import extract_assets, read_rule_cases, read_scenario
import openpyxl


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
    parser.add_argument('--scenario', type=Path, default=repo/'Test/Login.xlsx')
    parser.add_argument('--case', default='Login')
    parser.add_argument('--bin-dir', type=Path, default=repo/'build-cui/Release')
    parser.add_argument('--port-offset', type=int, default=20000)
    parser.add_argument('--game', action='store_true', help='Also proxy the client game connection (requires --edge client)')
    parser.add_argument('--edge', choices=('gateway', 'client'), default='gateway')
    parser.add_argument('--direction', choices=('requests', 'replies', 'both'), default='replies')
    parser.add_argument('--clients', type=int, default=1, choices=(1, 2, 3))
    parser.add_argument('--timeout-ms', type=int, default=15000)
    parser.add_argument('--log-dir', type=Path, default=repo/'out/gate-stream')
    args = parser.parse_args()
    if args.game and args.edge != 'client': parser.error('--game requires --edge client')
    args.log_dir.mkdir(parents=True, exist_ok=True)
    with args.scenario.open('rb') as source:
        workbook = openpyxl.load_workbook(source, read_only=True, data_only=True)
        rules = read_rule_cases(workbook) if 'Rules' in workbook.sheetnames else None
        _, fixture = read_scenario(workbook, args.case, rules)
        root = prepare(repo, args.bin_dir.resolve(), repo/'out/debug', {'fixture': fixture}, args.port_offset)
        extract_assets(workbook, args.case, root)
        workbook.close()
    config = configparser.ConfigParser(interpolation=None)
    config.optionxform = str
    config.read(root/'EIServer.ini', encoding='utf-8')
    client_config = configparser.ConfigParser(interpolation=None)
    client_config.optionxform = str
    client_config.read(root/'Mir3.ini', encoding='utf-8')
    proxies = []
    try:
        endpoints = [('LoginGate', 'Param1'), ('SelGate', 'SelectGatePort')]
        if args.game: endpoints.append(('GameGate', 'GameGatePort'))
        for section, client_key in endpoints:
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
            result = subprocess.run([sys.executable, str(repo/'Test/xlsx_scenario.py'), str(args.scenario.resolve()),
                '--case', args.case, '--bin-dir', str(args.bin_dir.resolve()),
                '--resume-runtime', str(root), '--clients', str(args.clients), '--timeout-ms', str(args.timeout_ms),
                '--log', str((args.log_dir/'test.log').resolve())], stdout=output, stderr=subprocess.STDOUT,
                env=dict(os.environ, PYTHONIOENCODING='utf-8'))
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

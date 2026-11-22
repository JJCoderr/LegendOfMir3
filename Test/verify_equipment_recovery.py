"""Verify committed equipment trades after terminating both test processes before character save."""
import argparse
import os
from pathlib import Path
import subprocess
import sys


def main():
    repo = Path(__file__).resolve().parents[1]
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--log-dir', type=Path, default=repo/'out/equipment-recovery')
    parser.add_argument('--port-offset', type=int, default=20000)
    args = parser.parse_args()
    logs = args.log_dir.resolve()
    logs.mkdir(parents=True, exist_ok=True)

    def run(name, scenario, *options):
        command = [sys.executable, str(repo/'Test/xlsx_scenario.py'), str(repo/'Test/Shop.xlsx'), '--case', scenario,
                   '--port-offset', str(args.port_offset),
                   '--timeout-ms', '15000', '--log', str(logs/(name+'.log')), *options]
        result = subprocess.run(command, cwd=repo, capture_output=True, text=True,
                                encoding='utf-8', env=dict(os.environ, PYTHONIOENCODING='utf-8'))
        report = logs/(name+'.launcher.log')
        report.write_text(result.stdout+result.stderr, encoding='utf-8')
        if result.returncode:
            raise RuntimeError(f'{name} failed; see {report}')
        runtime = next(line.removeprefix('Runtime: ') for line in result.stdout.splitlines()
                       if line.startswith('Runtime: '))
        print(name, (logs/(name+'.log')).read_text(encoding='utf-8').splitlines()[-1], flush=True)
        return runtime

    # launch_csv terminates its own server before its clients in finally. These cutoffs occur
    # before session.return, so neither phase relies on the character-save request.
    cases = [(name, 'ShopEquipment', last, 'ShopEquipmentRecovery'+name)
             for name, last in [('Sold', '17'), ('Rebuy', '32'), ('New', '41')]]
    cases += [('MixedBuy', 'ShopMixedTrade', None, 'ShopMixedRecovery'),
              ('MixedSale', 'ShopMixedSale', None, 'ShopMixedSaleRecovery')]
    for name, scenario, last, recovery in cases:
        runtime = run(name+'-before', scenario, *(['--to', last] if last else []))
        run(name+'-after', recovery, '--resume-runtime', runtime)
    print('PASS: five trade scenarios survive server termination before character save')


if __name__ == '__main__':
    main()

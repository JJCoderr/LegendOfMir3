"""Compare live training notifications, saved records and reloaded client state."""
import argparse
import json
import os
from pathlib import Path
import sqlite3
import subprocess
import sys


def main():
    repo=Path(__file__).resolve().parents[1]
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--log-dir',type=Path,default=repo/'out/skill-training')
    parser.add_argument('--port-offset', type=int, default=20000)
    args=parser.parse_args();logs=args.log_dir.resolve();logs.mkdir(parents=True,exist_ok=True)
    (logs/'comparison.log').write_text('',encoding='utf-8')
    for suffix in ('','High','Progress','Cap'):
        name='SkillsTraining'+suffix
        log=logs/(name+'.log')
        result=subprocess.run([sys.executable,str(repo/'Test/xlsx_scenario.py'),str(repo/'Test/Skills.xlsx'),'--case',name,
            '--port-offset',str(args.port_offset),
            '--timeout-ms','15000','--log',str(log)],cwd=repo,capture_output=True,text=True,encoding='utf-8',
            env=dict(os.environ,PYTHONIOENCODING='utf-8'))
        (logs/(name+'.launcher.log')).write_text(result.stdout+result.stderr,encoding='utf-8')
        if result.returncode: raise RuntimeError(name+' failed; see '+str(log))
        runtime=Path(next(line.removeprefix('Runtime: ') for line in result.stdout.splitlines() if line.startswith('Runtime: ')))
        states={}
        for line in log.read_text(encoding='utf-8').splitlines():
            for step in (180,250):
                if f' PASS {step} response=' in line:
                    skill=json.loads(line.split(' response=',1)[1])['result']['skills'][0]
                    states[step]=(skill['id'],skill['level'],skill['training'])
        with sqlite3.connect(runtime/'mir3.sqlite3') as db:
            saved=db.execute("SELECT FLD_MAGICID,FLD_LEVEL,FLD_CURRTRAIN FROM TBL_CHARACTER_MAGIC WHERE FLD_CHARNAME='MirProbe01'").fetchone()
        if states[180]!=saved or states[250]!=saved:
            raise AssertionError((name,states,saved))
        message=f'PASS {name}: live = database = reload = {saved}'
        print(message,flush=True)
        with (logs/'comparison.log').open('a',encoding='utf-8') as report: report.write(message+'\n')


if __name__=='__main__': main()

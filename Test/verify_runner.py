"""Verify CSV parsing, inclusive step ranges, exact assertions and failure exit codes."""
import argparse
import sqlite3
import subprocess
import tempfile
from pathlib import Path
from xlsx_scenario import export_case

repo = Path(__file__).resolve().parents[1]
parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument('--exe',type=Path,default=repo/'build-cui/Release/Test.exe')
args = parser.parse_args()
(repo/'out').mkdir(exist_ok=True)
root = Path(tempfile.mkdtemp(prefix='csv-runner-',dir=repo/'out'))
with sqlite3.connect(root/'test.sqlite3') as db:
    db.execute('CREATE TABLE fixture(value)')
checks = [
    ('wait','wait.csv',['--from','1','--to','1'],0,['PASS 1'],[]),
    *[(f'wait-invalid-{step}','wait.csv',['--from',str(step),'--to',str(step)],1,
       ['Runner wait requires milliseconds'],['PASS']) for step in (2,3,4,5)],
    ('all','csv.csv',[],0,['PASS 1','PASS 2','PASS 3'],[]),
    ('range','csv.csv',['--from','2','--to','2'],0,['PASS 2'],['STEP 1','STEP 3']),
    ('start','csv.csv',['--from','2'],0,['PASS 2','PASS 3'],['STEP 1']),
    ('end','csv.csv',['--to','2'],0,['PASS 1','PASS 2'],['STEP 3']),
    ('empty','csv.csv',['--from','4','--to','5'],1,['No steps'],['STEP']),
    ('reversed','csv.csv',['--from','3','--to','1'],1,['Invalid execution range'],['STEP']),
    ('precision','wrong_integer.csv',[],1,['expected [[9007199254740992]], actual [[9007199254740993]]'],['PASS']),
    ('stop','stop_on_failure.csv',[],1,['FAIL step=1'],['STEP 2']),
]
for name,scenario,options,code,required,forbidden in checks:
    log = root/(name+'.log')
    fixture, _, _ = export_case(repo/'Test/Runner.xlsx', Path(scenario).stem, root)
    result = subprocess.run([str(args.exe),str(fixture),'--database',str(root/'test.sqlite3'),
        '--log',str(log),*options],stdout=subprocess.DEVNULL)
    text = log.read_text(encoding='utf-8')
    assert result.returncode==code, (name,result.returncode,text)
    assert all(value in text for value in required), (name,text)
    assert not any(value in text for value in forbidden), (name,text)
    for line in text.splitlines():
        for marker in (' STEP ', ' PASS '):
            if marker in line:
                step = line.split(marker, 1)[1].split()[0]
                assert f'[id={Path(scenario).stem}.{step}]' in line, line
        if ' FAIL step=' in line:
            step = line.split(' FAIL step=', 1)[1].split()[0]
            if step != '0':
                assert f'[id={Path(scenario).stem}.{step}]' in line, line
    print('PASS',name)
print('Logs:',root)

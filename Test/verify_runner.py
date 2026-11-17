"""Verify CSV parsing, inclusive step ranges, exact assertions and failure exit codes."""
import argparse
import sqlite3
import subprocess
import tempfile
from pathlib import Path

repo = Path(__file__).resolve().parents[1]
parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument('--exe',type=Path,default=repo/'build-eiserver/Release/Test.exe')
args = parser.parse_args()
(repo/'out').mkdir(exist_ok=True)
root = Path(tempfile.mkdtemp(prefix='csv-runner-',dir=repo/'out'))
with sqlite3.connect(root/'test.sqlite3') as db:
    db.execute('CREATE TABLE fixture(value)')
checks = [
    ('all','csv.csv',[],0,['PASS 10','PASS 20','PASS 30'],[]),
    ('range','csv.csv',['--from','20','--to','20'],0,['PASS 20'],['STEP 10','STEP 30']),
    ('start','csv.csv',['--from','20'],0,['PASS 20','PASS 30'],['STEP 10']),
    ('end','csv.csv',['--to','20'],0,['PASS 10','PASS 20'],['STEP 30']),
    ('empty','csv.csv',['--from','21','--to','29'],1,['No steps'],['STEP']),
    ('reversed','csv.csv',['--from','30','--to','10'],1,['Invalid execution range'],['STEP']),
    ('precision','wrong_integer.csv',[],1,['expected [[9007199254740992]], actual [[9007199254740993]]'],['PASS']),
    ('stop','stop_on_failure.csv',[],1,['FAIL step=10'],['STEP 20']),
]
for name,scenario,options,code,required,forbidden in checks:
    log = root/(name+'.log')
    result = subprocess.run([str(args.exe),str(repo/'Test/Runner'/scenario),'--database',str(root/'test.sqlite3'),
        '--log',str(log),*options],stdout=subprocess.DEVNULL)
    text = log.read_text(encoding='utf-8')
    assert result.returncode==code, (name,result.returncode,text)
    assert all(value in text for value in required), (name,text)
    assert not any(value in text for value in forbidden), (name,text)
    print('PASS',name)
print('Logs:',root)

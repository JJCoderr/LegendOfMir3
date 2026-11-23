"""Isolated runtime preparation for the real client/server JSON test suites."""
import configparser
import os
from pathlib import Path
import shutil
import socket
import sqlite3
import subprocess
import tempfile

def prepare(repo, binaries, resources, suite, offset):
    ports = {p:p+offset for p in (5000,5100,5500,6000,7000,7100,7200)}
    for port in ports.values():
        if not 1024 <= port <= 65535: raise ValueError('Invalid test port')
        with socket.socket() as probe:
            probe.setsockopt(socket.SOL_SOCKET, socket.SO_EXCLUSIVEADDRUSE, 1)
            probe.bind(('127.0.0.1',port))
    for name in ('EIServer.exe','Mir2Ex.exe'):
        if not (binaries/name).is_file(): raise FileNotFoundError(binaries/name)
    (repo/'out').mkdir(exist_ok=True)
    root = Path(tempfile.mkdtemp(prefix='rpc-',dir=repo/'out'))
    shutil.copy2(binaries/'Mir2Ex.exe',root/'Mir2Ex.exe')
    for name in ('SoundList.wwl', 'CmList.dat', 'MInfo.dat'):
        source = resources/name
        if not source.is_file(): source = (resources/'Data').resolve().parent/name
        if source.is_file(): os.link(source, root/name)
    for name in ('EIServer.ini','Mir3.ini'):
        config = configparser.ConfigParser(interpolation=None)
        config.optionxform = str
        with (resources/name).open(encoding='utf-8-sig') as source: config.read_file(source)
        if name == 'EIServer.ini':
            for section,key,port in (
                ('DatabaseSvr','LocalPort',6000),('DatabaseSvr','GatePort',5100),
                ('GameServer','LocalPort',5000),('GameServer','DBServerPort',6000),
                ('LoginSvr','LocalPort',5500),('LoginGate','LocalPort',7000),
                ('LoginGate','RemotePort',5500),('SelGate','LocalPort',7100),
                ('SelGate','RemotePort',5100),('GameGate','LocalPort',7200),('GameGate','RemotePort',5000)):
                config[section][key] = str(ports[port])
            config['GameServer']['DBServerIP'] = '127.0.0.1'
            config['GameServer']['MapFileLoc'] = 'Map'
            for section in ('LoginGate','SelGate','GameGate'): config[section]['RemoteIP'] = '127.0.0.1'
            server = config[r'LoginSvr\Servers\1']
            server['Gate'] = f'127.0.0.1:{ports[7100]}'
            server['PrivateIP'] = server['PublicIP'] = '127.0.0.1'
            for key in ('Mir2_Account','Mir2_Common','Mir2_Game'): config['SQLite'][key] = 'mir3.sqlite3'
            config['Game']['StartMap'] = '01'
        else:
            for key,port in (('Param1',7000),('SelectGatePort',7100),('GameGatePort',7200)):
                config['Initial'][key] = str(ports[port])
            config['Initial']['ServerAddr'] = '127.0.0.1'
        with (root/name).open('w',encoding='utf-8') as target: config.write(target,space_around_delimiters=False)
    for name in ('Data','Map','Sound'):
        target = (resources/name).resolve()
        if not target.is_dir(): raise FileNotFoundError(target)
        env = dict(os.environ,MIR3_TEST_LINK=str(root/name),MIR3_TEST_TARGET=str(target))
        subprocess.run(['powershell','-NoProfile','-Command',
            '$ErrorActionPreference="Stop"; New-Item -ItemType Junction -Path $env:MIR3_TEST_LINK -Target $env:MIR3_TEST_TARGET | Out-Null'],env=env,check=True)
    with sqlite3.connect(root/'mir3.sqlite3') as db:
        db.executescript((repo/'Server/SQL_Scripts/sqlite_minimal.sql').read_text(encoding='utf-8-sig'))
        for statement in suite.get('fixture',[]): db.execute(statement['sql'],statement.get('params',[]))
    return root

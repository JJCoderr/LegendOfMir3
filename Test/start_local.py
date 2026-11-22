"""固定端口、固定目录的本地启动脚本。

每次运行都会先关掉上一套客户端/服务端，再在 out/run 这一套固定目录里启动：
端口沿用 EIServer.ini / Mir3.ini 的默认值（DBSvr 6000、GameSvr 5000、LoginGate 7000、
SelGate 7100、GameGate 7200），不再每次换端口偏移。

用法:
  python Test/start_local.py                # 首次会建库并准备资源
  python Test/start_local.py --with-data    # 额外导入 King 系列正式数据
"""
import argparse
import shutil
import sqlite3
import subprocess
import sys
import time
from pathlib import Path

REPO = Path(__file__).resolve().parent.parent
RUNTIME = REPO / 'out/run'
BINARIES = REPO / 'build-cui/Release'
ASSETS = Path(r'G:/传奇3/宝藏传奇3')
DATA_SCRIPTS = ('king_monster.sql', 'king_items.sql', 'king_magic.sql')


def kill_previous():
    subprocess.run(['taskkill', '/F', '/IM', 'Mir2Ex.exe'], capture_output=True)
    subprocess.run(['taskkill', '/F', '/IM', 'EIServer.exe'], capture_output=True)
    print('已关闭上一套进程')


def ensure_link(link, target):
    if link.exists() or link.is_symlink():
        return
    if target.exists():
        subprocess.run(['cmd', '/c', 'mklink', '/J', str(link), str(target)], check=True, capture_output=True)


def ensure_runtime(with_data):
    RUNTIME.mkdir(parents=True, exist_ok=True)
    for exe in ('EIServer.exe', 'Mir2Ex.exe', 'Test.exe'):
        source = BINARIES / exe
        if source.exists():
            shutil.copy2(source, RUNTIME / exe)
    for ini in ('Mir3.ini', 'EIServer.ini'):
        if not (RUNTIME / ini).exists():
            shutil.copy2(REPO / 'out/debug' / ini, RUNTIME / ini)

    ensure_link(RUNTIME / 'Data', ASSETS / 'Data')
    ensure_link(RUNTIME / 'Map', ASSETS / 'Map')
    ensure_link(RUNTIME / 'Sound', ASSETS / 'Sound')

    if not (RUNTIME / 'Envir').is_dir():
        shutil.copytree(REPO / 'Server/GameData/Envir', RUNTIME / 'Envir')
    for name in ('BgmList.wwl', 'MapMusic.ini'):
        source = (ASSETS / name) if name == 'BgmList.wwl' else (REPO / 'Server/GameData' / name)
        if source.exists() and not (RUNTIME / name).exists():
            shutil.copy2(source, RUNTIME / name)

    database = RUNTIME / 'mir3.sqlite3'
    if not database.exists():
        connection = sqlite3.connect(database)
        connection.executescript((REPO / 'Server/SQL_Scripts/sqlite_minimal.sql').read_text(encoding='utf-8-sig'))
        if with_data:
            for script in DATA_SCRIPTS:
                path = REPO / 'Server/SQL_Scripts' / script
                for line in path.read_text(encoding='utf-8').splitlines():
                    if line.strip() and not line.startswith('--'):
                        connection.execute(line)
        connection.commit()
        connection.close()
        print('已建库:', database, '(含正式数据)' if with_data else '(最小种子)')

    subprocess.run([sys.executable, str(REPO / 'Test/prepare_client_ini.py'), '--runtime', str(RUNTIME)],
                   check=True, capture_output=True)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--with-data', action='store_true')
    parser.add_argument('--no-client', action='store_true')
    args = parser.parse_args()

    kill_previous()
    ensure_runtime(args.with_data)

    subprocess.Popen([str(RUNTIME / 'EIServer.exe')], cwd=RUNTIME)
    time.sleep(6)
    if not args.no_client:
        subprocess.Popen([str(RUNTIME / 'Mir2Ex.exe')], cwd=RUNTIME)
        time.sleep(10)
    print('运行目录:', RUNTIME)
    print('端口: DBSvr 6000 / GameSvr 5000 / LoginGate 7000 / SelGate 7100 / GameGate 7200')


if __name__ == '__main__':
    main()

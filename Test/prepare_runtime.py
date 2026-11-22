"""把手动启动用的运行目录补齐到可玩状态。

补三样东西（缺哪个补哪个）：
  1. Envir 目录（服务端读 MiniMap.txt、Market_Def、MonItems 用）；
  2. MapMusic.ini（地图背景音乐表）；
  3. Data / Map / Sound 资源链接与 Mir3.ini 端口（与 EIServer.ini 对齐）。

用法:
  python Test/prepare_runtime.py --runtime out/rpc-xxxx
  python Test/prepare_runtime.py --runtime out/rpc-xxxx --assets "G:/传奇3/宝藏传奇3"
"""
import argparse
import shutil
import subprocess
import sys
from pathlib import Path

REPO = Path(__file__).resolve().parent.parent
DEFAULT_ASSETS = Path(r'G:/传奇3/宝藏传奇3')


def ensure_directory_link(link, target):
    if link.exists() or link.is_symlink():
        return False
    if not target.exists():
        print(f'  skip {link.name}: 资源目录不存在 {target}')
        return False
    subprocess.run(['cmd', '/c', 'mklink', '/J', str(link), str(target)],
                   check=True, capture_output=True)
    return True


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--runtime', type=Path, required=True)
    parser.add_argument('--assets', type=Path, default=DEFAULT_ASSETS)
    args = parser.parse_args()

    runtime = args.runtime
    if not runtime.is_dir():
        raise SystemExit(f'运行目录不存在: {runtime}')

    envir = runtime / 'Envir'
    if not envir.is_dir():
        shutil.copytree(REPO / 'Server/GameData/Envir', envir)
        print('  复制 Envir:', sum(1 for _ in envir.rglob('*') if _.is_file()), '个文件')

    music = runtime / 'MapMusic.ini'
    if not music.exists():
        shutil.copy2(REPO / 'Server/GameData/MapMusic.ini', music)
        print('  复制 MapMusic.ini')

    for name, sub in (('Data', 'Data'), ('Map', 'Map'), ('Sound', 'Sound')):
        if ensure_directory_link(runtime / name, args.assets / sub):
            print(f'  建立 {name} 链接')

    subprocess.run([sys.executable, str(REPO / 'Test/prepare_client_ini.py'), '--runtime', str(runtime)],
                   check=True)
    print('运行目录已就绪:', runtime)


if __name__ == '__main__':
    main()

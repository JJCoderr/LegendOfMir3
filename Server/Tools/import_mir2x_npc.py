"""把 mir2x 的 NPC 脚本目录转换成服务端 TBL_MERCHANT。

mir2x 的 server/script/npc 下每个 NPC 用一个空 lua 文件占位，文件名本身编码了
全部信息：<地图名>_<地图号>.<NPC名>_<序号>.GLOC_<x>_<y>_<z>.LOOK_<外观>.lua

用法:
  python Server/Tools/import_mir2x_npc.py --npc <mir2x/server/script/npc> \
      --out Server/SQL_Scripts/mir2x_npc.sql
"""
import argparse
import re
from pathlib import Path

PATTERN = re.compile(
    r'^(?P<container>[^.]+)\.(?P<name>[^_]+)_(?P<seq>\d+)'
    r'\.GLOC_(?P<x>-?\d+)_(?P<y>-?\d+)_(?P<z>\d+)\.LOOK_(?P<look>\d+)\.lua$')


def q(text):
    return "'" + ' '.join(str(text).split()).replace("'", "''") + "'"


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--npc', type=Path, required=True)
    parser.add_argument('--out', type=Path)
    args = parser.parse_args()

    rows = []
    skipped = []
    for path in sorted(args.npc.glob('*.lua')):
        found = PATTERN.match(path.name)
        if not found:
            skipped.append(path.name)
            continue
        info = found.groupdict()
        container = info['container']
        mapname = container.rsplit('_', 1)[-1] if '_' in container else container
        rows.append((mapname, int(info['x']), int(info['y']), info['name'], int(info['look'])))

    statements = ['-- 由 Server/Tools/import_mir2x_npc.py 生成，请勿手改。', 'DELETE FROM TBL_MERCHANT;']
    for index, (mapname, x, y, name, look) in enumerate(rows, start=1):
        script = f'{name}-{mapname}.txt'
        statements.append(
            'INSERT INTO TBL_MERCHANT (FLD_ID,FLD_MAPNAME,FLD_POSX,FLD_POSY,FLD_NPCNAME,FLD_SCRIPT,'
            'FLD_FACE,FLD_BODY,FLD_GENDER,FLD_DESCRIPTION) VALUES '
            f'({index},{q(mapname)},{x},{y},{q(name)},{q(script)},0,{look},0,{q(name)});')

    if args.out:
        args.out.write_text('\n'.join(statements) + '\n', encoding='utf-8')
        print(f'NPC {len(rows)} 条 -> {args.out}，未解析 {len(skipped)}')
        for name in skipped[:5]:
            print('  skip', name)


if __name__ == '__main__':
    main()

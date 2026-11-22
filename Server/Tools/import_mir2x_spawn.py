"""把 mir2x 的地图脚本（刷怪配置）转换成 TBL_MONGEN 插入语句。

mir2x 的 server/script/map 下每个地图一个 lua，内容是 addmonster.monGener(...) 的
表结构：

    { name = '蜈蚣', loc = { {x = 100, y = 100, w = 100, h = 100, count = 40, time = 600} } }

文件名是 <地图中文名>_<地图号>.lua，地图号即 TBL_MAPINFO 的文件名。

用法:
  python Server/Tools/import_mir2x_spawn.py --map <mir2x/server/script/map> \
      --out Server/SQL_Scripts/mir2x_spawn.sql
"""
import argparse
import re
from pathlib import Path

MONSTER = re.compile(r"name\s*=\s*'([^']+)'\s*,\s*loc\s*=\s*\{(.*?)\n\s*\}", re.S)
SPOT = re.compile(
    r"x\s*=\s*(-?\d+)\s*,\s*y\s*=\s*(-?\d+)\s*,\s*w\s*=\s*(\d+)\s*,\s*h\s*=\s*(\d+)"
    r"\s*,\s*count\s*=\s*(\d+)\s*,\s*time\s*=\s*(\d+)")

SKIP = {'addmonster.lua', 'default.lua'}


def q(text):
    return "'" + ' '.join(str(text).split()).replace("'", "''") + "'"


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--map', type=Path, required=True)
    parser.add_argument('--out', type=Path)
    args = parser.parse_args()

    rows = []
    files = 0
    for path in sorted(args.map.glob('*.lua')):
        if path.name in SKIP:
            continue
        stem = path.stem
        if '_' not in stem:
            continue
        mapname = stem.rsplit('_', 1)[-1]
        text = path.read_text(encoding='utf-8', errors='ignore')
        files += 1
        for name, body in MONSTER.findall(text):
            for x, y, w, h, count, gentime in SPOT.findall(body):
                rows.append((mapname, name, int(x), int(y), int(w), int(h), int(count), int(gentime)))

    statements = ['-- 由 Server/Tools/import_mir2x_spawn.py 生成，请勿手改。',
                  '-- 导入前请确认 TBL_MONSTER 里有同名怪物，FLD_MONINDEX 需要按名字回填。',
                  'DELETE FROM TBL_MONGEN;']
    for index, (mapname, name, x, y, w, h, count, gentime) in enumerate(rows, start=1):
        statements.append(
            'INSERT INTO TBL_MONGEN (FLD_INDEX,FLD_MAPNAME,FLD_X,FLD_Y,FLD_MONNAME,FLD_AREA,'
            'FLD_COUNT,FLD_GENTIME,FLD_SMALLGENRATE,FLD_DESCRIPTION,FLD_MONINDEX,FLD_AREA_X,FLD_AREA_Y) '
            f'VALUES ({index},{q(mapname)},{x},{y},{q(name)},{w},{count},{gentime},0,{q(name)},0,{w},{h});')

    if args.out:
        args.out.write_text('\n'.join(statements) + '\n', encoding='utf-8')
        print(f'地图脚本 {files} 个，刷怪条目 {len(rows)} -> {args.out}')


if __name__ == '__main__':
    main()

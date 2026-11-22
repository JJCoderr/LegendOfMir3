"""统计各 WIL 的空帧分布，用于发现"帧索引指向空帧"这类回退缺陷。

用法: python Test/audit_wil_frames.py --data "G:/传奇3/宝藏传奇3/Data"
"""
import argparse
import struct
from pathlib import Path


def load_frames(data, name):
    wil_path = data / f'{name}.wil'
    wix_path = data / f'{name}.wix'
    if not (wil_path.exists() and wix_path.exists()):
        return None
    wil = wil_path.read_bytes()
    wix = wix_path.read_bytes()
    count = struct.unpack_from('<i', wix, 20)[0]
    if count <= 0:
        return None
    positions = struct.unpack_from(f'<{count}i', wix, 24)
    empty = [index for index, pos in enumerate(positions) if pos <= 0 or pos + 4 > len(wil)]
    sizes = {}
    for index, pos in enumerate(positions):
        if pos <= 0 or pos + 4 > len(wil):
            continue
        sizes[index] = struct.unpack_from('<hh', wil, pos)
    return count, empty, sizes


def compress(indices, limit=6):
    """把连续区间压成 3-5、7 这样的形式，便于阅读。"""
    if not indices:
        return ''
    spans = []
    start = prev = indices[0]
    for value in indices[1:]:
        if value == prev + 1:
            prev = value
            continue
        spans.append((start, prev))
        start = prev = value
    spans.append((start, prev))
    parts = []
    for begin, end in spans[:limit]:
        parts.append(f'{begin}' if begin == end else f'{begin}-{end}')
    if len(spans) > limit:
        parts.append(f'…(共 {len(spans)} 段)')
    return ','.join(parts)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--data', type=Path, required=True)
    args = parser.parse_args()

    for wil_path in sorted(args.data.glob('*.wil')):
        result = load_frames(args.data, wil_path.stem)
        if not result:
            continue
        count, empty, sizes = result
        head = f'{wil_path.name:20s} 帧数={count:6d} 空帧={len(empty):5d}'
        print(head, '空帧区间:', compress(empty) if empty else '无')

    print()
    print('关键帧检查（ProgUse.wil）:')
    result = load_frames(args.data, 'ProgUse')
    if result:
        count, empty, sizes = result
        for frame in (2, 3, 41, 49, 50):
            state = 'empty' if frame in empty else f'{sizes.get(frame)}'
            print(f'  frame {frame}: {state}')


if __name__ == '__main__':
    main()

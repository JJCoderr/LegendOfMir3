"""Inspect V1.45 WIL/WIX frames and optionally export a contact sheet.

Example: python inspect_wil.py Data/Interface1c.wil --frames 0:110 --output ui.png
Contact sheets require Pillow; listing metadata uses only the standard library.
"""
import argparse
from pathlib import Path
import struct


def read_frames(path):
    data = path.read_bytes()
    index = path.with_suffix('.wix').read_bytes()
    count = struct.unpack_from('<i', index, 20)[0]
    header = len(index) - count * 4
    if count < 0 or header not in (24, 28):
        raise ValueError('Unsupported WIX layout')
    for number in range(count):
        offset = struct.unpack_from('<i', index, header + number * 4)[0]
        if offset == 0:
            continue
        if offset < 0 or offset + 17 > len(data):
            raise ValueError(f'Invalid frame offset: {number}')
        width, height = struct.unpack_from('<hh', data, offset)
        if width > 0 and height > 0:
            yield number, width, height, data, offset + 17


def decode(width, height, data, offset):
    from PIL import Image
    image = Image.new('RGB', (width, height))
    pixels = image.load()
    for y in range(height):
        length = struct.unpack_from('<H', data, offset)[0]
        end = offset + length * 2
        cursor, x = offset + 2, 0
        while cursor < end:
            tag, count = struct.unpack_from('<HH', data, cursor)
            cursor += 4
            if x + count > width:
                raise ValueError('Run exceeds frame width')
            if tag == 0xC0:
                x += count
            elif tag in (0xC1, 0xC2, 0xC3):
                # Dye runs are shown in their stored base color.
                if cursor + count * 2 > end + 2:
                    raise ValueError('Run exceeds row length')
                for value in struct.unpack_from('<' + 'H' * count, data, cursor):
                    pixels[x, y] = ((value >> 11) * 255 // 31,
                                    ((value >> 5) & 63) * 255 // 63,
                                    (value & 31) * 255 // 31)
                    x += 1
                cursor += count * 2
            else:
                raise ValueError(f'Unknown run marker: {tag:#x}')
        offset = end + 2
    return image


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('wil', type=Path)
    parser.add_argument('--frames', default='0:100', help='Start:end (end exclusive)')
    parser.add_argument('--output', type=Path)
    args = parser.parse_args()
    start, end = map(int, args.frames.split(':'))
    frames = [f for f in read_frames(args.wil) if start <= f[0] < end]
    for number, width, height, _, _ in frames:
        print(f'{number}: {width} x {height}')
    if args.output and frames:
        from PIL import Image, ImageDraw
        sheet = Image.new('RGB', (800, 160 * ((len(frames) + 3) // 4)), (40, 40, 40))
        draw = ImageDraw.Draw(sheet)
        for slot, (number, width, height, data, offset) in enumerate(frames):
            image = decode(width, height, data, offset)
            image.thumbnail((192, 130))
            x, y = slot % 4 * 200, slot // 4 * 160
            sheet.paste(image, (x, y + 24))
            draw.text((x + 4, y + 4), f'{number}: {width}x{height}', fill='white')
        sheet.save(args.output)


if __name__ == '__main__':
    main()

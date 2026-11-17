"""Build a local visual review index from fresh native CLIENT captures."""
import html
import json
import sys
from pathlib import Path

from PIL import Image, ImageDraw

root = Path(sys.argv[1]).resolve()
manifest = json.loads((root / 'manifest.json').read_text(encoding='utf-8-sig'))
cards = []
images = []
for entry in manifest['captures']:
    source = root / entry['file']
    with Image.open(source) as capture:
        capture.load()
        image = capture.convert('RGB')
    name = source.with_suffix('.png')
    image.save(name)
    mode = entry['mode']
    cards.append(f'<figure id="{mode}"><figcaption>{html.escape(mode)}</figcaption>'
                 f'<a href="{name.name}"><img loading="lazy" src="{name.name}" alt="{mode}"></a>'
                 f'<small>{image.width} × {image.height} · {entry["captured"]}</small></figure>')
    images.append((mode, image))

for start in range(0, len(images), 6):
    sheet = Image.new('RGB', (1600, 1872), '#15191d')
    draw = ImageDraw.Draw(sheet)
    for index, (mode, image) in enumerate(images[start:start + 6]):
        x, y = index % 2 * 800, index // 2 * 624
        draw.text((x + 10, y + 5), mode, fill='white')
        sheet.paste(image, (x, y + 24))
    sheet.save(root / f'contact-{start // 6 + 1:02}.png')

page = '''<!doctype html><meta charset="utf-8"><title>CLIENT GUI 视觉核查</title>
<style>body{background:#15191d;color:#eee;font:16px system-ui;margin:24px}header{max-width:1000px}
main{display:grid;grid-template-columns:repeat(auto-fit,minmax(400px,1fr));gap:24px}
figure{margin:0;background:#242a30;padding:12px}img{width:100%;height:auto}figcaption{margin-bottom:10px}
small{display:block;color:#b8c0c8;margin-top:8px}a{color:#9cd2ff}input{padding:12px;width:360px;margin:20px 0}</style>
<header><h1>CLIENT GUI 当前截图索引</h1><p>真实原生控件、离线测试数据。点击图片查看原始分辨率。
这些截图用于核查，不代表已与 MIR3.EXE 逐像素比对通过。</p>'''
page += f'<p>源码：{manifest["revision"]}<br>二进制 SHA256：{manifest["binaryHash"]}<br>截图数：{len(cards)}</p></header>'
page += '<input placeholder="输入页面名筛选" aria-label="筛选页面"><main>' + ''.join(cards) + '</main>'
page += '''<script>document.querySelector('input').oninput=e=>document.querySelectorAll('figure').forEach(f=>f.hidden=!f.id.includes(e.target.value.trim().toLowerCase()));</script>'''
(root / 'index.html').write_text(page, encoding='utf-8')
print(f'{len(cards)} captures: {root / "index.html"}')

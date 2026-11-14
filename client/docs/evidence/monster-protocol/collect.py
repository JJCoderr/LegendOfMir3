"""只读提取原版怪物分派与处理函数；不会修改或保存 IDB。"""
import json, urllib.request
from pathlib import Path
out = Path(__file__).resolve().parent
code = '''import ida_hexrays, ida_lines, ida_bytes, idautils, json
result={"functions":{},"vtable":{}}
for ea in [0x40D8C0,0x42EC70,0x42E9A0,0x4282C0,0x407B70,0x407BE0,0x407C00,0x407C20,0x407C50,0x407C60,0x407CC0,0x4081F0,0x4082F0,0x4083D0,0x4085A0,0x408720,0x408B70,0x4093B0,0x4094A0,0x409500,0x409600,0x409DB0,0x40CBF0,0x40B0A0,0x40E7F0,0x40E9F0,0x406750,0x408C10,0x406E60,0x407FC0,0x405C00,0x46F690]:
 try:
  c=ida_hexrays.decompile(ea)
  result["functions"][hex(ea)]="\\n".join(ida_lines.tag_remove(x.line) for x in c.get_pseudocode())
 except Exception as e: result["functions"][hex(ea)]=str(e)
for offset in range(0,128,4): result["vtable"][str(offset)]=hex(ida_bytes.get_dword(0x4AF3D4+offset))
from pathlib import Path
Path(r'G:/mir3/Mir3/client/docs/evidence/monster-protocol/export.json').write_text(json.dumps(result),encoding='utf-8')
print('saved')'''
req = urllib.request.Request('http://127.0.0.1:13337/mcp', json.dumps({'jsonrpc':'2.0','id':1,'method':'tools/call','params':{'name':'py_eval','arguments':{'database':'mir3-special-monsters','code':code}}}).encode(), {'Content-Type':'application/json'})
raw=json.load(urllib.request.urlopen(req,timeout=55))
out.joinpath('raw.json').write_text(json.dumps(raw,ensure_ascii=False,indent=2),encoding='utf-8')
r=raw['result']; d=r.get('structuredContent') or json.loads(r['content'][0]['text'])
data=json.loads(out.joinpath('export.json').read_text(encoding='utf-8'))
for addr,txt in data['functions'].items(): out.joinpath(addr+'.c').write_text(txt,encoding='utf-8')
out.joinpath('vtable.json').write_text(json.dumps(data['vtable'],indent=2),encoding='utf-8')
print('Exported',len(data['functions']),'functions')

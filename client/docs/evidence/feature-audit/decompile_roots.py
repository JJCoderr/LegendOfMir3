import ida_hexrays,ida_lines,idautils,ida_funcs,json
from pathlib import Path
out=Path(r'G:\mir3\Mir3\client\docs\evidence\feature-audit')
roots=[0x401B50,0x434AC0,0x436A30,0x43AB90,0x43D760,0x425F00,0x427400,0x4282C0,0x42EC70,0x4610D0,0x4602B0,0x46E400,0x46E6E0,0x48A830,0x48A9D0,0x4644F0,0x476B80,0x478000,0x470A30,0x470E60]
result=[]
for ea in roots:
 try:
  c=ida_hexrays.decompile(ea)
  txt='\n'.join(ida_lines.tag_remove(x.line) for x in c.get_pseudocode())
  out.joinpath(f'{ea:08X}.c').write_text(txt,encoding='utf-8')
  result.append({'addr':hex(ea),'status':'ok','lines':len(txt.splitlines())})
 except Exception as e:result.append({'addr':hex(ea),'status':'error','error':str(e)})
out.joinpath('decompile-results.json').write_text(json.dumps(result,indent=2),encoding='utf-8')
print(json.dumps(result))

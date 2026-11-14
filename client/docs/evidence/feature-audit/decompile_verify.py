import ida_hexrays,ida_lines,idautils,ida_funcs,json
from pathlib import Path
out=Path(r'G:\mir3\Mir3\client\docs\evidence\feature-audit')
roots=[0x478850,0x479100,0x47CCF0,0x47CD30,0x47CFA0,0x47CFD0,0x47D000,0x47D030,0x47BF30,0x470C30,0x4710F0,0x471FA0,0x430180,0x435EA0,0x43A840,0x425FF0,0x426040]
result=[]
for ea in roots:
 try:
  c=ida_hexrays.decompile(ea)
  txt='\n'.join(ida_lines.tag_remove(x.line) for x in c.get_pseudocode())
  out.joinpath(f'{ea:08X}.c').write_text(txt,encoding='utf-8')
  result.append({'addr':hex(ea),'status':'ok','lines':len(txt.splitlines())})
 except Exception as e:result.append({'addr':hex(ea),'status':'error','error':str(e)})
out.joinpath('verification-results.json').write_text(json.dumps(result,indent=2),encoding='utf-8')
print(json.dumps(result))

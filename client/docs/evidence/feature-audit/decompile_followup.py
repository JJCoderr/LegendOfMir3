import ida_hexrays,ida_lines,idautils,ida_funcs,json
from pathlib import Path
out=Path(r'G:\mir3\Mir3\client\docs\evidence\feature-audit')
roots=[0x421070,0x423E90,0x47C2E0,0x46E510,0x46EA10,0x46EC00,0x43A5D0,0x43C130,0x477780,0x4782E0,0x4614D0,0x420540,0x420850,0x435180]
result=[]
for ea in roots:
 try:
  c=ida_hexrays.decompile(ea)
  txt='\n'.join(ida_lines.tag_remove(x.line) for x in c.get_pseudocode())
  out.joinpath(f'{ea:08X}.c').write_text(txt,encoding='utf-8')
  result.append({'addr':hex(ea),'status':'ok','lines':len(txt.splitlines())})
 except Exception as e:result.append({'addr':hex(ea),'status':'error','error':str(e)})
out.joinpath('followup-results.json').write_text(json.dumps(result,indent=2),encoding='utf-8')
print(json.dumps(result))

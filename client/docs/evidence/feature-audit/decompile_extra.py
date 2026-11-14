import ida_hexrays,ida_lines,idautils,ida_funcs,json
from pathlib import Path
out=Path(r'G:\mir3\Mir3\client\docs\evidence\feature-audit')
roots=[4330656, 4341072, 4341392, 4341840, 4342400, 4344528, 4347568, 4351184, 4352800, 4353312, 4353600, 4353776, 4357200, 4646288, 4646336, 4646624, 4647504, 4647840, 4648192, 4648320, 4648480, 4648672, 4648736, 4648800, 4677008, 4680752, 4687808, 4692992, 4706464]
result=[]
for ea in roots:
 try:
  c=ida_hexrays.decompile(ea)
  txt='\n'.join(ida_lines.tag_remove(x.line) for x in c.get_pseudocode())
  out.joinpath(f'{ea:08X}.c').write_text(txt,encoding='utf-8')
  result.append({'addr':hex(ea),'status':'ok','lines':len(txt.splitlines())})
 except Exception as e:result.append({'addr':hex(ea),'status':'error','error':str(e)})
out.joinpath('extra-results.json').write_text(json.dumps(result,indent=2),encoding='utf-8')
print(json.dumps(result))

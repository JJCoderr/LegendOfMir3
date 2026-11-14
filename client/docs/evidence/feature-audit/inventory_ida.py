import ida_funcs,ida_bytes,idautils,ida_name,json
strings=[]
for s in idautils.Strings():
 if s.ea>=0x4B5000 and s.ea<0x4B725C:
  refs=[]
  for x in idautils.XrefsTo(s.ea):
   f=ida_funcs.get_func(x.frm)
   refs.append({'at':hex(x.frm),'function':hex(f.start_ea) if f else None})
  strings.append({'addr':hex(s.ea),'text':str(s),'refs':refs})
funcs=[]
for ea in idautils.Functions():
 f=ida_funcs.get_func(ea)
 if ea>=0x48A000:continue
 calls=[]
 for item in idautils.FuncItems(ea):
  for to in idautils.CodeRefsFrom(item,False):
   g=ida_funcs.get_func(to)
   if g and g.start_ea!=ea:calls.append(hex(g.start_ea))
 funcs.append({'addr':hex(ea),'name':ida_name.get_name(ea),'size':f.end_ea-ea,'callees':sorted(set(calls))})
from pathlib import Path
Path(r'G:\mir3\Mir3\client\docs\evidence\feature-audit\inventory.json').write_text(json.dumps({'strings':strings,'functions':funcs},ensure_ascii=False,indent=2),encoding='utf-8')
print(len(strings),len(funcs))

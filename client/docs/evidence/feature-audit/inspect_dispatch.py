import ida_bytes,ida_funcs,ida_name,idautils,idc,json
from pathlib import Path
out=Path(r'G:\mir3\Mir3\client\docs\evidence\feature-audit')
vtable={}
for base in [0x4AFBFC,0x4AF77C,0x4AF760,0x4AF744]:
 vtable[hex(base)]=[{'slot':i,'addr':hex(ida_bytes.get_dword(base+i*4)),'name':ida_name.get_name(ida_bytes.get_dword(base+i*4))} for i in range(16)]
refs=[]
for ea in idautils.Functions(0x421000,0x4282C0):
 for item in idautils.FuncItems(ea):
  for i in range(3):
   v=idc.get_operand_value(item,i)
   if 1106080*4<=v<=1106200*4:refs.append({'func':hex(ea),'at':hex(item),'line':idc.generate_disasm_line(item,0)})
out.joinpath('dispatch-extra.json').write_text(json.dumps({'vtable':vtable,'refs':refs},indent=2),encoding='utf-8')
print(json.dumps({'vtable':vtable,'refs':refs[:15],'total_refs':len(refs)}))

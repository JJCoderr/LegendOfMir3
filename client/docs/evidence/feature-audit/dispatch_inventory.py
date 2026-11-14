import ida_hexrays,ida_lines,ida_bytes,idautils,idc,json
from pathlib import Path
out=Path(r'G:\mir3\Mir3\client\docs\evidence\feature-audit')
class V(ida_hexrays.ctree_visitor_t):
 def __init__(self,c):
  ida_hexrays.ctree_visitor_t.__init__(self,ida_hexrays.CV_FAST);self.c=c;self.switches=[];self.comparisons=[]
 def visit_insn(self,i):
  if i.op==ida_hexrays.cit_switch:
   sw=i.cswitch; selector=ida_lines.tag_remove(sw.expr.print1(self.c))
   cases=[]
   for case in sw.cases:cases.append({'target':hex(case.ea),'values':[int(x) for x in case.values]})
   self.switches.append({'at':hex(i.ea),'selector':selector,'cases':cases})
  return 0
 def visit_expr(self,e):
  if e.op in [ida_hexrays.cot_eq,ida_hexrays.cot_ne,ida_hexrays.cot_uge,ida_hexrays.cot_ugt,ida_hexrays.cot_ule,ida_hexrays.cot_ult,ida_hexrays.cot_sge,ida_hexrays.cot_sgt,ida_hexrays.cot_sle,ida_hexrays.cot_slt]:
   t=ida_lines.tag_remove(e.print1(self.c))
   if 'v358' in t:self.comparisons.append({'at':hex(e.ea),'text':t})
  return 0
result=[]
for ea in [0x4282C0,0x43AB90,0x43C130,0x43D760,0x401040]:
 try:
  c=ida_hexrays.decompile(ea);v=V(c);v.apply_to(c.body,None)
  result.append({'function':hex(ea),'switches':v.switches,'comparisons':v.comparisons})
  if ea==0x401040:out.joinpath('00401040.c').write_text('\n'.join(ida_lines.tag_remove(x.line) for x in c.get_pseudocode()),encoding='utf-8')
 except Exception as e:result.append({'function':hex(ea),'error':str(e)})
asm=[]
for ea in idautils.Heads(0x436A30,0x436D80):asm.append({'at':hex(ea),'asm':idc.generate_disasm_line(ea,0)})
out.joinpath('dispatch-inventory.json').write_text(json.dumps({'functions':result,'window_disasm':asm},ensure_ascii=False,indent=2),encoding='utf-8')
print(json.dumps([{'function':r['function'],'switches':len(r.get('switches',[])),'comparisons':len(r.get('comparisons',[])),'error':r.get('error')} for r in result]))

"""只在Unicorn内存中执行ASPack加载桩；到原代码段立即停止，不调用宿主DLL。"""
from pathlib import Path
import json,struct,hashlib
import pefile
from unicorn import Uc,UC_ARCH_X86,UC_MODE_32,UC_HOOK_BLOCK,UC_HOOK_CODE,UcError
from unicorn.x86_const import UC_X86_REG_ESP,UC_X86_REG_EIP,UC_X86_REG_EAX
source=Path('G:/游戏/宝藏传奇3客户端/KingMir3.dll')
out=Path(__file__).resolve().parents[2]/'build'/'king-unpack';out.mkdir(exist_ok=True)
assert hashlib.sha256(source.read_bytes()).hexdigest()=='2f1809c944804ceea93792e2e18c24382927facc115e102ff744537ad1cbfbd9', '样本指纹不符，须重新核对布局'
p=pefile.PE(str(source));base=p.OPTIONAL_HEADER.ImageBase
u=Uc(UC_ARCH_X86,UC_MODE_32)
size=(p.OPTIONAL_HEADER.SizeOfImage+4095)&~4095
u.mem_map(base,size);u.mem_write(base,p.get_memory_mapped_image())
u.mem_map(0x70000000,0x100000);u.mem_map(0x20000000,0x200000)
sp=0x201ff000
u.mem_write(sp,struct.pack('<IIII',0x700ff000,base,1,0));u.reg_write(UC_X86_REG_ESP,sp)
api={};names={};modules={};events=[];next_alloc=0x10000000;reached=[]
def dword(a):return struct.unpack('<I',u.mem_read(a,4))[0]
def string(a):
 data=bytearray()
 while a and len(data)<1024:
  c=u.mem_read(a+len(data),1)[0]
  if not c:break
  data.append(c)
 return data.decode('ascii',errors='replace')
def stub(module,name):
 key=(module,name)
 if key not in names:
  a=0x70000000+len(names)*16;names[key]=a;api[a]=key;u.mem_write(a,b'\xc3')
 return names[key]
for dll in p.DIRECTORY_ENTRY_IMPORT:
 module=dll.dll.decode().lower()
 for item in dll.imports:u.mem_write(item.address,struct.pack('<I',stub(module,item.name.decode() if item.name else '#'+str(item.ordinal))))
def on_api(uc,a,sz,data):
 global next_alloc
 if a not in api:raise RuntimeError('Unknown API PC '+hex(a))
 module,name=api[a];esp=u.reg_read(UC_X86_REG_ESP);args=[dword(esp+4+i*4) for i in range(4)]
 if name in ('LoadLibraryA','GetModuleHandleA'):
  key=string(args[0]).lower() if args[0] else 'kingmir3.dll'
  if key not in modules:modules[key]=0x60000000+len(modules)*0x10000
  value=modules[key];argc=1
 elif name=='GetProcAddress':
  mod=next((k for k,v in modules.items() if v==args[0]),hex(args[0]))
  sym=string(args[1]) if args[1]>65535 else '#'+str(args[1])
  value=stub(mod,sym);argc=2
 elif name=='VirtualAlloc':
  amount=(args[1]+4095)&~4095;value=args[0] or next_alloc;next_alloc=value+amount+0x10000
  u.mem_map(value,amount);argc=4
 elif name=='VirtualFree':value=1;argc=3
 elif name=='VirtualProtect':
  if args[3]:u.mem_write(args[3],struct.pack('<I',0x40))
  value=1;argc=4
 else:raise RuntimeError('Unsupported loader API '+module+'!'+name)
 events.append({'api':name,'args':args,'result':value})
 u.reg_write(UC_X86_REG_EAX,value);u.reg_write(UC_X86_REG_EIP,dword(esp));u.reg_write(UC_X86_REG_ESP,esp+4+argc*4)
def block(uc,a,sz,data):
 if base+0x1000<=a<base+0xb1000 and u.mem_read(a,1)!=b'\xc3':
  reached.append(a);u.emu_stop()
u.hook_add(UC_HOOK_CODE,on_api,begin=0x70000000,end=0x700fffff)
u.hook_add(UC_HOOK_BLOCK,block)
error=None
try:u.emu_start(base+p.OPTIONAL_HEADER.AddressOfEntryPoint,0,timeout=30000000,count=100000000)
except Exception as e:error=str(e)
report={'source_sha256':hashlib.sha256(source.read_bytes()).hexdigest(),'oep':reached,'pc':hex(u.reg_read(UC_X86_REG_EIP)),'error':error,'events':events,'imports':[{'address':a,'module':m,'name':n} for a,(m,n) in api.items()]}
(out/'emulation.json').write_text(json.dumps(report,indent=2),encoding='utf-8')
(out/'image.bin').write_bytes(u.mem_read(base,size))
print(json.dumps({k:v for k,v in report.items() if k not in ('events','imports')}));print('API calls',len(events),'symbols',len(api))
if error or not reached:raise SystemExit('加载桩未完成，不能重建为成功副本')

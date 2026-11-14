"""从模拟器快照重建供IDA分析的PE；不作为可运行替换DLL发布。"""
from pathlib import Path
import json,struct,hashlib
import pefile
folder=Path(__file__).resolve().parents[2]/'build'/'king-unpack'
r=json.loads((folder/'emulation.json').read_text());assert r['oep'] and r['error'] is None
b=bytearray((folder/'image.bin').read_bytes());p=pefile.PE(data=bytes(b[:1024]),fast_load=True)
lookup={x['address']:(x['module'],x['name']) for x in r['imports']}
groups=[]
for a in range(0xc4000,0xc7000,4):
 v=struct.unpack_from('<I',b,a)[0]
 if v not in lookup:continue
 module,name=lookup[v]
 if groups and groups[-1]['module']==module and groups[-1]['slots'][-1][0]+4==a:groups[-1]['slots'].append((a,name))
 else:groups.append({'module':module,'slots':[(a,name)]})
assert sum(len(g['slots']) for g in groups)>300
start=0xe8000;blob=bytearray(20*(len(groups)+1))
def add(data,align=1):
 blob.extend(b'\0'*((-len(blob))%align));rva=start+len(blob);blob.extend(data);return rva
for i,g in enumerate(groups):
 mod=add(g['module'].encode()+b'\0');thunks=[]
 for slot,name in g['slots']:
  v=(0x80000000|int(name[1:])) if name.startswith('#') else add(b'\0\0'+name.encode()+b'\0',2)
  thunks.append(v);struct.pack_into('<I',b,slot,v)
 int_rva=add(struct.pack('<'+'I'*(len(thunks)+1),*thunks,0),4)
 struct.pack_into('<IIIII',blob,i*20,int_rva,0,0,mod,g['slots'][0][0])
size=(start+len(blob)+4095)&~4095
b.extend(b'\0'*(size-len(b)));b[start:start+len(blob)]=blob
p.OPTIONAL_HEADER.AddressOfEntryPoint=r['oep'][0]-p.OPTIONAL_HEADER.ImageBase
p.OPTIONAL_HEADER.SizeOfImage=size;p.OPTIONAL_HEADER.CheckSum=0
p.OPTIONAL_HEADER.DATA_DIRECTORY[1].VirtualAddress=start;p.OPTIONAL_HEADER.DATA_DIRECTORY[1].Size=20*(len(groups)+1)
for i in [4,11]:p.OPTIONAL_HEADER.DATA_DIRECTORY[i].VirtualAddress=0;p.OPTIONAL_HEADER.DATA_DIRECTORY[i].Size=0
for s in p.sections:
 s.PointerToRawData=s.VirtualAddress;s.SizeOfRawData=s.Misc_VirtualSize
 if s.Name.startswith(b'.adata'):
  s.Misc_VirtualSize=size-start;s.SizeOfRawData=size-start;s.Characteristics=0xc0000040
headers=p.write()[:1024];b[:len(headers)]=headers
output=folder/'KingMir3-analysis.dll';output.write_bytes(b)
check=pefile.PE(str(output));count=sum(len(d.imports) for d in check.DIRECTORY_ENTRY_IMPORT)
print('imports',count,'groups',len(groups),'exports',len(check.DIRECTORY_ENTRY_EXPORT.symbols),'entry',hex(check.OPTIONAL_HEADER.AddressOfEntryPoint))
print('sha256',hashlib.sha256(b).hexdigest(),'size',len(b))
(folder/'repair.json').write_text(json.dumps({'analysis_only':True,'imports':count,'groups':groups,'sha256':hashlib.sha256(b).hexdigest()},indent=2),encoding='utf-8')

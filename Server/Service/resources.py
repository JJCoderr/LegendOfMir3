from pathlib import Path
import re,sys
server=Path(__file__).resolve().parent.parent
result=['#pragma code_page(65001)','LANGUAGE 9, 1','STRINGTABLE','BEGIN']
for index,role in enumerate(['DBSvr','LoginSvr','GameSvr','LoginGate','SelGate','GameGate','AdminCmd'],1):
    header=(server/role/'resource.h').read_text(encoding='gbk',errors='replace')
    ids=dict(re.findall(r'#define\s+(\w+)\s+(\d+)',header))
    rc=(server/role/(role+'.rc')).read_text(encoding='gbk',errors='replace')
    for block in re.findall(r'STRINGTABLE[^\n]*\s+BEGIN(.*?)END',rc,re.S):
        for name,text in re.findall(r'(\w+)\s+("[^\n]+")',block):
            if name in ids:result.append(f'{int(ids[name])+index*8192} {text}')
result.append('END')
Path(sys.argv[1]).write_text('\n'.join(result),encoding='utf-8')

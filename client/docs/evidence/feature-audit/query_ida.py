"""只读调用现有IDA分析库，保存原始MCP结果；不修改或保存IDB。"""
import sys,json,urllib.request
from pathlib import Path
name=sys.argv[1]; args=json.loads(sys.argv[2]); args['database']='mir3-map-review'
r=urllib.request.Request('http://127.0.0.1:13337/mcp',json.dumps({'jsonrpc':'2.0','id':1,'method':'tools/call','params':{'name':name,'arguments':args}}).encode(),{'Content-Type':'application/json'})
v=json.load(urllib.request.urlopen(r,timeout=55))['result']
d=v.get('structuredContent') or json.loads(v['content'][0]['text'])
Path(sys.argv[3]).write_text(json.dumps(d,ensure_ascii=False,indent=2),encoding='utf-8')
if 'code' in d: print(d.get('code') or d)
else: print(json.dumps(d,ensure_ascii=True))

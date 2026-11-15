"""只读提取指定宝藏版IDA会话；每次保留请求及原始结果，不默认回退Mir3.exe。"""
import argparse
import json
import urllib.request
from pathlib import Path

p = argparse.ArgumentParser()
p.add_argument('database')
p.add_argument('tool')
p.add_argument('arguments', type=Path)
p.add_argument('output', type=Path)
a = p.parse_args()
arguments = json.loads(a.arguments.read_text(encoding='utf-8-sig'))
arguments['database'] = a.database
request = {'jsonrpc': '2.0', 'id': 1, 'method': 'tools/call',
           'params': {'name': a.tool, 'arguments': arguments}}
req = urllib.request.Request('http://127.0.0.1:13337/mcp', json.dumps(request).encode(),
                             {'Content-Type': 'application/json'})
with urllib.request.urlopen(req, timeout=55) as response:
    result = json.load(response)
a.output.write_text(json.dumps({'request': request, 'response': result}, ensure_ascii=False, indent=2), encoding='utf-8')
payload = result.get('result', {})
print(json.dumps(payload.get('structuredContent', payload), ensure_ascii=True))

"""生成全量移植清单；声明/方法存在仅表示源码覆盖，不表示运行接入或验证通过。"""
import json
import re
from pathlib import Path

root = Path(__file__).resolve().parents[4]
models = root / 'reference/Zircon/ServerLibrary/Models'
header = (root / 'client/src/world/Monster.hpp').read_text(encoding='utf-8-sig')
source = (root / 'client/src/world/Monster.cpp').read_text(encoding='utf-8-sig')
factory = (models / 'MonsterObject.cs').read_text(encoding='utf-8-sig').split('public static MonsterObject GetMonster', 1)[1].split('public MonsterObject()', 1)[0]
branches = []
for match in re.finditer(r'case (-?\d+):([\s\S]*?)(?=\n\s*case |\n\s*default:)', factory):
    kind = re.search(r'return new (?:Monsters\.)?(\w+)', match[2])
    if kind:
        branches.append({'ai': int(match[1]), 'class': kind[1], 'initializer': match[2].strip()})
rows = []
for path in sorted((models / 'Monsters').glob('*.cs')):
    text = path.read_text(encoding='utf-8-sig')
    parent = re.search(r'class\s+\w+\s*:\s*(\w+)', text)
    name = path.stem
    rows.append({'class': name, 'parent': parent[1] if parent else None,
                 'factory_ai': [b['ai'] for b in branches if b['class'] == name],
                 'declared': bool(re.search(r'\bclass\s+'+name+r'\b', header)),
                 'state_struct': bool(re.search(r'\bstruct\s+'+name+r'State\b', header)),
                 'methods': sorted(set(re.findall(r'zircon::'+name+r'::(\w+)\(', source))),
                 'reference_lines': len(text.splitlines()),
                 'world_status': 'pending_individual_audit', 'baozang_status': 'pending_individual_evidence'})
out = Path(__file__).with_name('reference-inventory.json')
out.write_text(json.dumps({'classes': rows, 'factory': branches}, ensure_ascii=False, indent=2), encoding='utf-8')
print(json.dumps({'reference_classes':len(rows),'factory_branches':len(branches),
                  'missing_classes':[r['class'] for r in rows if not r['declared']]},ensure_ascii=False))

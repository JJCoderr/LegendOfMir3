"""Inventory declared CM/SM messages; wire observation is not semantic coverage."""
from pathlib import Path
import re

SOURCES = ('Client/Common/Protocol.h','Client/Define.h','Server/Def/Protocol.h')

def definitions(repo):
    entries = []
    for source in SOURCES:
        for line, text in enumerate((repo/source).read_text(encoding='utf-8',errors='replace').splitlines(),1):
            match = re.match(r'\s*#define\s+((?:CM|SM)_\w+)\s+(0[xX][0-9a-fA-F]+|[0-9]+)\b', text)
            if match:
                name, value = match.groups()
                entries.append({'name':name,'opcode':int(value,0) if value.lower().startswith('0x') else int(value),
                    'direction':'sent' if name.startswith('CM_') else 'received','source':source,'line':line})
    return entries

def number(entries, name):
    client = [e['opcode'] for e in entries if e['name']==name and e['source'].startswith('Client/')]
    values = set(client or [e['opcode'] for e in entries if e['name']==name])
    if len(values)!=1: raise ValueError(f'Undefined or ambiguous protocol: {name}: {values}')
    return values.pop()

def coverage(entries, observed):
    counts = {(e['direction'],e['opcode']):e['count'] for e in observed}
    grouped = {}
    for entry in entries:
        key = (entry['direction'],entry['opcode'])
        record = grouped.setdefault(key,{'direction':key[0],'opcode':key[1],'definitions':[],
            'observed_count':counts.get(key,0),'semantic_coverage':'not_determined_by_packet_count'})
        record['definitions'].append(entry)
    known = set(grouped)
    for key,count in counts.items():
        if key not in known:
            grouped[key]={'direction':key[0],'opcode':key[1],'definitions':[],'observed_count':count,
                'semantic_coverage':'not_determined_by_packet_count'}
    return {'scope':list(SOURCES),'limitation':'Namespace constants and literal opcodes outside these headers require further inventory.',
        'messages':list(grouped.values())}

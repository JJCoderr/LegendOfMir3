"""Expand a workbook row into a single-pass monster showcase."""
import random
import sqlite3


GROUP_SIZE = 3
SMOKE_GROUPS = 2
DROP_NAME = 'Showcase potion'
POTION_INDEX = 0
POTION_LOOKS = 3
GENERAL_ITEM = 3
FIREBALL = 1
MONSTER_HP = 100
ATTACK_DISPLAY_MS = 550
MAGIC_DISPLAY_MS = 650
INJURY_DISPLAY_MS = 700
DROP_DISPLAY_MS = 550


def expand_catalog(repo, prefix, smoke=False):
    source = repo / 'Server/SQL_Scripts/king_monster.sql'
    statements = [line for line in source.read_text(encoding='utf-8-sig').splitlines()
                  if line.strip() and not line.startswith('--')]
    with sqlite3.connect(':memory:') as database:
        database.executescript((repo / 'Server/SQL_Scripts/sqlite_minimal.sql').read_text(encoding='utf-8-sig'))
        for statement in statements:
            database.execute(statement)
        catalog = database.execute('SELECT FLD_INDEX, FLD_NAME FROM TBL_MONSTER ORDER BY FLD_INDEX').fetchall()
    random.shuffle(catalog)
    if smoke:
        catalog = catalog[:GROUP_SIZE * SMOKE_GROUPS]

    statements.extend([
        'DELETE FROM TBL_MONGEN',
        'DELETE FROM TBL_MONITEM',
        f'UPDATE TBL_MONSTER SET FLD_RACE=0,FLD_RACEINDEX=0,FLD_HP={MONSTER_HP},FLD_EXP=0,'
        'FLD_AC=0,FLD_MAXAC=0,FLD_MAC=0,FLD_MAXMAC=0,FLD_DC=0,FLD_MAXDC=0,'
        'FLD_VIEWRANGE=0,FLD_WALKSPEED=600000',
        f"UPDATE TBL_STDITEM_ETC SET FLD_NAME='{DROP_NAME}',FLD_LOOKS={POTION_LOOKS} WHERE FLD_INDEX={POTION_INDEX}",
        'INSERT INTO TBL_MONITEM (FLD_MONSTER,FLD_TYPE,FLD_ITEM,FLD_PROBABLY,FLD_COUNT) '
        f'SELECT FLD_INDEX,{GENERAL_ITEM},{POTION_INDEX},1,1 FROM TBL_MONSTER',
    ])
    records = []

    def add(group, name, target, action, params='', expect=''):
        records.append(dict(step=f'{prefix}.{group}.{len(records)+1}', name=name,
                            target=target, action=action, params=params, expect=expect))

    def pause(group, name, milliseconds):
        add(group, name, 'Runner', 'wait', f'milliseconds={milliseconds}', 'result.status=waited')

    for start in range(0, len(catalog), GROUP_SIZE):
        group = start // GROUP_SIZE + 1
        batch = catalog[start:start + GROUP_SIZE]
        ids = ','.join(str(index) for index, _ in batch)
        matching_ids = ';'.join(f'result.actors.{position}.monster={index}'
                                for position, (index, _) in enumerate(batch))
        add(group, f'[{group}] [#{batch[0][0]}] ' + '、'.join(name for _, name in batch),
            'Server', 'test.monsters', f'action=spawn;ids=[{ids}]', f'result.status=spawn;{matching_ids}')
        add(group, '确认本组全部怪物出场', 'Client', 'state.wait', expect=f'result.actor_count={len(batch)}')
        add(group, '展示怪物的普攻', 'Server', 'test.monsters', 'action=attack', 'result.status=attack')
        pause(group, '播放普攻动作', ATTACK_DISPLAY_MS)
        add(group, '核对普攻回包', 'Client', 'wire.expect',
            'message=SM_HIT;direction=received', f'count={start+len(batch)}')
        add(group, '展示怪物的施法', 'Client', 'test.monster_magic', expect=f'result.effects={len(batch)}')
        pause(group, '播放怪物法术', MAGIC_DISPLAY_MS)
        add(group, '等待玩家可以施法', 'Client', 'state.wait', expect='result.cast_ready=true')
        add(group, '玩家用火球术攻击第一只怪物', 'Client', 'skill.cast', 'key=1;target=0', 'result.status=submitted')
        add(group, '核对火球术造成的中间血量', 'Client', 'state.wait',
            expect=f'result.actors.0.max_hp={MONSTER_HP};result.actors.0.injured=true;'
                   f'result.actors.0.health_bar_visible=true;result.spell_reply_id={FIREBALL}')
        pause(group, '展示受伤后的血条', INJURY_DISPLAY_MS)
        add(group, '触发服务端死亡和掉落', 'Server', 'test.monsters', 'action=defeat', 'result.status=defeat')
        deaths = ';'.join(f'result.actors.{position}.dead=true;result.actors.{position}.health_bar_visible=false'
                          for position in range(len(batch)))
        add(group, '核对死亡与物品掉落', 'Client', 'state.wait',
            expect=f'{deaths};result.ground_items.{len(batch)-1}.name={DROP_NAME}')
        pause(group, '展示落地物品', DROP_DISPLAY_MS)
        add(group, '清理本组怪物和物品', 'Server', 'test.monsters', 'action=clear', 'result.status=clear')
        add(group, '确认场地已清空', 'Client', 'state.wait', expect='result.actor_count=0;result.ground_items=[]')
    group_count = (len(catalog) + GROUP_SIZE - 1) // GROUP_SIZE
    add(group_count + 1, '核对每组使用一次火球术', 'Client', 'wire.expect',
        'message=CM_SPELL;direction=sent', f'count={group_count}')
    return records, [{'sql': statement} for statement in statements]

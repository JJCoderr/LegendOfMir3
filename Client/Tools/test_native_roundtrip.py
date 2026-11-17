"""Real Windows client login/movement/return/relogin regression; no screenshots.

Keeps its isolated database and logs under out/native-roundtrip-*.
Requires the standard development INI files and Data/Map/Sound resource links.
"""
import argparse
import configparser
import ctypes as c
import json
import os
import sqlite3
import subprocess
import tempfile
import time
import shutil
from ctypes import wintypes as w
from pathlib import Path

WM_SETTEXT = 0x000C
WM_GETTEXTLENGTH = 0x000E
WM_KEYDOWN = 0x0100
WM_KEYUP = 0x0101
WM_SYSKEYDOWN = 0x0104
WM_MOUSEMOVE = 0x0200
WM_LBUTTONDOWN = 0x0201
WM_LBUTTONUP = 0x0202
EM_GETPASSWORDCHAR = 0x00D2
VK_RETURN = 13
VK_LBUTTON = 1
VK_RBUTTON = 2
MOUSEEVENTF_LEFTDOWN = 0x0002
MOUSEEVENTF_LEFTUP = 0x0004
MK_LBUTTON = 1
ALT_CONTEXT = 1 << 29

u=c.windll.user32
cb=c.WINFUNCTYPE(w.BOOL,w.HWND,w.LPARAM)
u.SetProcessDPIAware()
repo = Path(__file__).resolve().parents[2]
parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument('--bin-dir', type=Path, default=repo / 'out/debug')
parser.add_argument('--config-dir', type=Path, default=repo / 'out/debug')
parser.add_argument('--port-offset', type=int, default=20000)
parser.add_argument('--hold-mouse', action='store_true', help='Exercise real held mouse input; requires an unlocked foreground desktop')
parser.add_argument('--equipment', action='store_true', help='Exercise armor equip/unequip and persistence through native panels')
args = parser.parse_args()
exe = args.bin_dir.resolve()
config_dir = args.config_dir.resolve()
ports = {p: p + args.port_offset for p in (5000,5100,5500,6000,7000,7100,7200)}
if any(p < 1024 or p > 65535 for p in ports.values()):
    parser.error('Test ports must be between 1024 and 65535')
for name in ('EIServer.exe', 'Mir2Ex.exe'):
    if not (exe/name).is_file():parser.error(f'Missing binary: {exe/name}')
(repo/'out').mkdir(exist_ok=True)
root=Path(tempfile.mkdtemp(prefix='native-roundtrip-',dir=repo/'out'))
# EIServer uses cwd; Mir2Ex changes cwd to its executable directory.
# Keep the listening server binary stable and isolate the outbound-only client.
shutil.copy2(exe/"Mir2Ex.exe", root/"Mir2Ex.exe")
# Optional sound index shares the configured resource without copying it.
if (config_dir/'SoundList.wwl').is_file():
    os.link(config_dir/'SoundList.wwl', root/'SoundList.wwl')
# Override endpoint and DB keys explicitly so test writes stay in the fixture.
for name in ('EIServer.ini', 'Mir3.ini'):
    config = configparser.ConfigParser(interpolation=None)
    config.optionxform = str
    with (config_dir/name).open(encoding='utf-8-sig') as source:
        config.read_file(source)
    if name == 'EIServer.ini':
        for section, key, port in (
            ('DatabaseSvr', 'LocalPort', 6000), ('DatabaseSvr', 'GatePort', 5100),
            ('GameServer', 'LocalPort', 5000), ('GameServer', 'DBServerPort', 6000),
            ('LoginSvr', 'LocalPort', 5500), ('LoginGate', 'LocalPort', 7000),
            ('LoginGate', 'RemotePort', 5500), ('SelGate', 'LocalPort', 7100),
            ('SelGate', 'RemotePort', 5100), ('GameGate', 'LocalPort', 7200),
            ('GameGate', 'RemotePort', 5000)):
            config[section][key] = str(ports[port])
        config['GameServer']['DBServerIP'] = '127.0.0.1'
        config['GameServer']['MapFileLoc'] = 'Map'
        for section in ('LoginGate', 'SelGate', 'GameGate'):
            config[section]['RemoteIP'] = '127.0.0.1'
        server = config[r'LoginSvr\Servers\1']
        server['Gate'] = f'127.0.0.1:{ports[7100]}'
        server['PrivateIP'] = server['PublicIP'] = '127.0.0.1'
        for key in ('Mir2_Account', 'Mir2_Common', 'Mir2_Game'):
            config['SQLite'][key] = 'mir3.sqlite3'
        config['Game']['StartMap'] = '01'
    else:
        for key, port in (('Param1', 7000), ('SelectGatePort', 7100), ('GameGatePort', 7200)):
            config['Initial'][key] = str(ports[port])
        config['Initial']['ServerAddr'] = '127.0.0.1'
    with (root/name).open('w', encoding='utf-8') as output:
        config.write(output, space_around_delimiters=False)
for name in ('Data','Map','Sound'):
    target=(config_dir/name).resolve()
    if not target.is_dir():parser.error(f'Missing resource directory: {target}')
    link_env=dict(os.environ,MIR3_TEST_LINK=str(root/name),MIR3_TEST_TARGET=str(target))
    subprocess.run(['powershell','-NoProfile','-Command',
        '$ErrorActionPreference="Stop"; New-Item -ItemType Junction -Path $env:MIR3_TEST_LINK -Target $env:MIR3_TEST_TARGET | Out-Null'],env=link_env,check=True)
with sqlite3.connect(root/'mir3.sqlite3') as db:
    db.executescript((repo/'Server/SQL_Scripts/sqlite_minimal.sql').read_text(encoding='utf-8-sig'))
    db.execute("INSERT INTO TBL_CHARACTER (FLD_LOGINID,FLD_CHARNAME,FLD_JOB,FLD_GENDER,FLD_LEVEL,FLD_MAPNAME,FLD_CX,FLD_CY) VALUES ('mirtest','MirProbe01','0','0',1,'01',459,261)")
if args.equipment:
    with sqlite3.connect(root/'mir3.sqlite3') as db:
        db.execute("UPDATE TBL_STDITEM SET FLD_AC=2,FLD_AC2=5,FLD_MAC=1,FLD_MAC2=3 WHERE FLD_INDEX=33")
        db.execute("INSERT INTO TBL_CHARACTER_ITEM (FLD_LOGINID,FLD_CHARNAME,FLD_STDTYPE,FLD_MAKEDATE,FLD_MAKEINDEX,FLD_STDINDEX,FLD_DURA,FLD_DURAMAX) VALUES ('mirtest','MirProbe01','B','261117','00001',33,5000,5000)")

env=dict(os.environ,MIR3_DEBUG_STATE_DIR=str(root))
processes=[]
print(root,flush=True)
def launch(role,**kw):
    binary=(exe if role=='EIServer' else root)/(role+'.exe')
    p=subprocess.Popen([str(binary)],cwd=root,env=env,**kw)
    processes.append(p);return p,root/f'{role}-{p.pid}.json'
def wait(path,accept,seconds=12):
    data={}
    owner=next(p for p in processes if str(p.pid)==path.stem.rsplit("-",1)[1])
    for _ in range(seconds*10):
        if owner.poll() is not None:raise RuntimeError(f"{path.stem} exited {owner.returncode}; logs: {root}")
        try:data=json.loads(path.read_bytes())
        except (OSError,ValueError):pass
        if path.exists() and time.time()-path.stat().st_mtime<2 and accept(data):return data
        time.sleep(.1)
    raise RuntimeError(data)
def windows(pid):
    result={}
    @cb
    def visit(h,_):
        owner=w.DWORD();u.GetWindowThreadProcessId(h,c.byref(owner))
        if owner.value==pid:
            name=c.create_unicode_buffer(100);u.GetClassNameW(h,name,100);result[name.value]=h
        return True
    u.EnumWindows(visit,0)
    return result
def click(h,x,y):
    x=round(x*u.GetDpiForSystem()/96);y=round(y*u.GetDpiForSystem()/96)
    pos=(y<<16)|x
    for msg,wp in ((WM_MOUSEMOVE,0),(WM_LBUTTONDOWN,MK_LBUTTON),(WM_LBUTTONUP,0)):u.PostMessageW(h,msg,wp,pos)
def equipment_roundtrip(h, state_path):
    def equipped(d):
        return len(d.get('equipment', [])) == 1 and not d.get('bag') and not d.get('held_item') and not d.get('equipment_pending')
    for key in ('Q', 'W'):
        u.PostMessageW(h, WM_KEYDOWN, ord(key), 0)
        u.PostMessageW(h, WM_KEYUP, ord(key), 0)
    initial = wait(state_path, lambda d:d.get('bag_panel', {}).get('open') and d.get('equipment_panel', {}).get('open') and len(d.get('bag', [])) == 1)
    bag = initial['bag_panel']; status = initial['equipment_panel']
    bag_point = (bag['x'] + 36, bag['y'] + 79)
    armor_point = (status['x'] + 120, status['y'] + 159)
    assert initial['bag'][0]['looks']==940 and initial['bag'][0]['shape']==1,initial
    item_id = initial['bag'][0]['id']
    expected_defence = [value + bonus for value, bonus in zip(initial['defence'], (2, 5))]
    expected_magic = [value + bonus for value, bonus in zip(initial['magic_defence'], (1, 3))]
    for cycle in range(3):
        click(h, *bag_point)
        wait(state_path, lambda d:d.get('held_item', {}) and d['held_item']['id'] == item_id)
        click(h, *armor_point)
        worn = wait(state_path, lambda d:equipped(d) and d['defence']==expected_defence and d['magic_defence']==expected_magic and d['dress']==1)
        assert worn['equipment'][0]['slot'] == 0 and worn['equipment'][0]['id'] == item_id, worn
        assert worn['equipment_panel'] == status and worn['bag_panel'] == bag, worn
        assert (worn['hp'], worn['mp']) == (initial['hp'], initial['mp']), worn
        print('armor equipped', cycle, worn, flush=True)
        if cycle == 2:break
        click(h, *armor_point)
        wait(state_path, lambda d:d.get('held_item', {}) and d['held_item']['id'] == item_id)
        click(h, *bag_point)
        removed = wait(state_path, lambda d:len(d.get('bag', [])) == 1 and not d.get('equipment') and not d.get('held_item') and not d.get('equipment_pending') and d['defence']==initial['defence'] and d['magic_defence']==initial['magic_defence'] and d['dress']==0)
        assert removed['bag'][0]['id'] == item_id, removed
        print('armor removed', cycle, removed, flush=True)
    for key in ('Q', 'W'):
        u.PostMessageW(h, WM_KEYDOWN, ord(key), 0)
        u.PostMessageW(h, WM_KEYUP, ord(key), 0)
    wait(state_path, lambda d:not d['bag_panel']['open'] and not d['equipment_panel']['open'])

def hold_mouse(h, x, y, seconds=1.8):
    if any(u.GetAsyncKeyState(key) & 0x8000 for key in (VK_LBUTTON, VK_RBUTTON)):
        raise RuntimeError('A mouse button is already held; refusing to interfere')
    foreground_thread = u.GetWindowThreadProcessId(u.GetForegroundWindow(), None)
    current_thread = c.windll.kernel32.GetCurrentThreadId()
    attached = u.AttachThreadInput(current_thread, foreground_thread, True)
    try:
        u.SetForegroundWindow(h)
    finally:
        if attached:
            u.AttachThreadInput(current_thread, foreground_thread, False)
    for _ in range(20):
        if u.GetForegroundWindow() == h:
            break
        time.sleep(.1)
    else:
        raise RuntimeError(f'Cannot activate client {h}; foreground={u.GetForegroundWindow()}')
    previous = w.POINT()
    u.GetCursorPos(c.byref(previous))
    width, height = size(h)
    point = w.POINT(round(x * width / 800), round(y * height / 600))
    if not u.ClientToScreen(h, c.byref(point)):
        raise RuntimeError('Cannot locate the client on screen')
    try:
        if not u.SetCursorPos(point.x, point.y):
            raise RuntimeError('Cannot position the mouse')
        u.mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0)
        time.sleep(seconds)
    finally:
        u.mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0)
        u.SetCursorPos(previous.x, previous.y)

def size(h):
    r=w.RECT();u.GetClientRect(h,c.byref(r));return r.right,r.bottom
with (root/'server.log').open('wb') as log:
    try:
        s,ss=launch('EIServer',stdout=log,stderr=log,creationflags=subprocess.CREATE_NO_WINDOW)
        print('server',wait(ss,lambda d:d.get('ready')),flush=True)
        for iteration in range(2):
            p,cs=launch('Mir2Ex')
            login=wait(cs,lambda d:d.get('connected'))
            assert login.get('remote_port')==ports[7000],login
            print('login',login,flush=True)
            ws=windows(p.pid)
            print(ws,flush=True)
            edit=ws.pop('Edit');h=ws['WH GEngine']
            assert size(h)==tuple(round(v*u.GetDpiForSystem()/96) for v in (640,480)),size(h)
            for _ in range(100):
                if u.IsWindowVisible(edit):break
                time.sleep(.1)
            assert u.IsWindowVisible(edit),'login editor not shown'
            for x,word in ((150,'mirtest'),(350,'test123')):
                click(h,x,446);time.sleep(.25)
                assert u.SendMessageW(edit,WM_SETTEXT,0,c.c_wchar_p(word))
                print('editor length/mask',u.SendMessageW(edit,WM_GETTEXTLENGTH,0,0),u.SendMessageW(edit,EM_GETPASSWORDCHAR,0,0),flush=True)
            click(h,466,442)
            login=wait(cs,lambda d:d.get('stage')=='server_list' or d.get('notice'))
            print('authenticated',login,flush=True)
            assert login.get('stage')=='server_list',login
            if login.get('notice'):
                u.PostMessageW(h,WM_KEYDOWN,VK_RETURN,0);u.PostMessageW(h,WM_KEYUP,VK_RETURN,0)
                wait(cs,lambda d:not d.get('notice'))
            click(h,60,80)
            print('character',wait(cs,lambda d:d.get('scene')=='character_select'),size(h),flush=True)
            assert size(h)==tuple(round(v*u.GetDpiForSystem()/96) for v in (640,480)),size(h)
            time.sleep(3)
            click(h,275,61)
            world=wait(cs,lambda d:d.get('map') and d.get('remote_port')==ports[7200],20)
            print('world',world,size(h),flush=True)
            assert size(h)==tuple(round(v*u.GetDpiForSystem()/96) for v in (800,600)),size(h)
            if iteration==0:
                # Acknowledge the arrival notice before using the map.
                u.PostMessageW(h,WM_KEYDOWN,VK_RETURN,0);u.PostMessageW(h,WM_KEYUP,VK_RETURN,0)
                print('input ready',wait(cs,lambda d:not d.get('notice') and not d.get('input_delayed')),flush=True)
                if args.equipment:
                    equipment_roundtrip(h,cs)
                if args.hold_mouse:
                    hold_mouse(h,448,240)
                else:
                    click(h,448,240)
                moved=wait(cs,lambda d:d.get('scene')=='world' and (d.get('x'),d.get('y'))!=(459,261))
                print('moved',moved,flush=True)
                time.sleep(1)
                moved=json.loads(cs.read_bytes())
                print('settled before logout',moved,flush=True)
                if args.hold_mouse:
                    assert moved['x'] >= world['x'] + 2 and moved['y'] == world['y'], ('held input must continue east', moved, world)
            else:
                assert (world['x'],world['y'])==saved, (world,saved)
                if args.equipment:
                    assert world['equipment'][0]['looks']==940 and world['dress']==1,world
                    assert world['defence']==[2,5] and world['magic_defence']==[1,3],world
                    assert len(world['equipment'])==1 and world['equipment'][0]['name']=='Test male armor' and not world['bag'],world
            if iteration==0:
                u.PostMessageW(h,WM_SYSKEYDOWN,ord('X'),ALT_CONTEXT)
                time.sleep(.5)
                u.PostMessageW(h,WM_KEYDOWN,VK_RETURN,0);u.PostMessageW(h,WM_KEYUP,VK_RETURN,0)
                returned=wait(cs,lambda d:d.get('scene')=='character_select' and d.get('connected') and d.get('remote_port')==ports[7100],20)
                print('returned',returned,flush=True)
                assert size(h)==tuple(round(v*u.GetDpiForSystem()/96) for v in (640,480)),size(h)
                wait(cs,lambda d:not d.get('waiting') and not d.get('notice'))
                with sqlite3.connect(root/'mir3.sqlite3') as db:
                    persisted=db.execute("SELECT FLD_CX,FLD_CY FROM TBL_CHARACTER WHERE FLD_CHARNAME='MirProbe01'").fetchone()
                print('saved before reentry',persisted,flush=True)
                assert persisted==(moved['x'],moved['y']),(persisted,moved)
                click(h,275,61)
                restored=wait(cs,lambda d:d.get('scene')=='world' and d.get('map') and d.get('remote_port')==ports[7200],20)
                print('restored',restored,flush=True)
                if args.equipment:
                    assert restored['equipment'][0]['looks']==940 and restored['dress']==1,restored
                    assert restored['defence']==[2,5] and restored['magic_defence']==[1,3],restored
                    assert len(restored['equipment'])==1 and restored['equipment'][0]['name']=='Test male armor' and not restored['bag'],restored
                assert (restored['x'],restored['y'])==(moved['x'],moved['y']),(restored,moved)
                wait(cs,lambda d:not d.get('input_delayed'))
                # Make the second save distinct from the earlier return-to-selection save.
                click(h,448,240)
                wait(cs,lambda d:(d.get('x'),d.get('y'))!=(restored['x'],restored['y']))
                time.sleep(1)
            # Alt+Q and Enter exercise the client's normal confirmed exit.
            u.PostMessageW(h,WM_SYSKEYDOWN,ord('Q'),ALT_CONTEXT)
            dialog_opened=time.time()
            before_exit=wait(cs,lambda d:cs.stat().st_mtime>dialog_opened and d.get('scene')=='world')
            final_position=(before_exit['x'],before_exit['y'])
            print('position before confirmed exit',final_position,flush=True)
            u.PostMessageW(h,WM_KEYDOWN,VK_RETURN,0);u.PostMessageW(h,WM_KEYUP,VK_RETURN,0)
            exit_code=p.wait(timeout=15)
            assert exit_code==0,exit_code
            print('client exit',exit_code,flush=True)
            if iteration==0:
                for attempt in range(100):
                    with sqlite3.connect(root/'mir3.sqlite3') as db:
                        saved=db.execute("SELECT FLD_CX,FLD_CY FROM TBL_CHARACTER WHERE FLD_CHARNAME='MirProbe01'").fetchone()
                    if saved==final_position:break
                    time.sleep(.1)
                assert saved==final_position,(saved,final_position)
                print('persisted',saved,flush=True)

    finally:
        for p in reversed(processes):
            if p.poll() is None:p.terminate()
            p.wait(timeout=5)
print('PASS: native login, movement, return, reentry and persisted position after client restart',flush=True)

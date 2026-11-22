"""按服务端 EIServer.ini 的端口修正客户端 Mir3.ini，并补齐自动登录段。

手动启动一套运行目录时使用：把 Mir3.ini 的登录/选服/游戏端口对齐到
EIServer.ini，避免客户端连到默认端口导致「连接失败」。

用法: python Test/prepare_client_ini.py --runtime out/rpc-xxxx
      python Test/prepare_client_ini.py --runtime out/rpc-xxxx --account mirtest --password test123
"""
import argparse
import configparser
from pathlib import Path


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--runtime', type=Path, required=True)
    parser.add_argument('--account', default='mirtest')
    parser.add_argument('--password', default='test123')
    parser.add_argument('--server', default='Mir3Local')
    parser.add_argument('--character', default='')
    args = parser.parse_args()

    server = configparser.ConfigParser(interpolation=None)
    server.optionxform = str
    server.read(args.runtime / 'EIServer.ini', encoding='gbk')

    client_path = args.runtime / 'Mir3.ini'
    client = configparser.ConfigParser(interpolation=None)
    client.optionxform = str
    client.read(client_path, encoding='utf-8')
    if not client.has_section('Initial'):
        client.add_section('Initial')
    client['Initial']['ServerAddr'] = '127.0.0.1'
    client['Initial']['Param1'] = server['LoginGate']['LocalPort']
    client['Initial']['SelectGatePort'] = server['SelGate']['LocalPort']
    client['Initial']['GameGatePort'] = server['GameGate']['LocalPort']
    if not client.has_section('AutoLogin'):
        client.add_section('AutoLogin')
    client['AutoLogin']['Account'] = args.account
    client['AutoLogin']['Password'] = args.password
    client['AutoLogin']['Server'] = args.server
    client['AutoLogin']['Character'] = args.character
    with client_path.open('w', encoding='utf-8') as target:
        client.write(target, space_around_delimiters=False)
    print(f'{client_path}: Param1={server["LoginGate"]["LocalPort"]} '
          f'SelectGatePort={server["SelGate"]["LocalPort"]} '
          f'GameGatePort={server["GameGate"]["LocalPort"]}')


if __name__ == '__main__':
    main()

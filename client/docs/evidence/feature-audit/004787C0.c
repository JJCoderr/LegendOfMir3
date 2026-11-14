BOOL __thiscall sub_4787B0(int this, int a2, int a3)
{
  char v3; // al
  int v4; // edx
  int v5; // eax
  int v6; // ecx
  int v7; // eax
  int v8; // eax
  int v9; // edx
  int v10; // eax
  int v12; // [esp+0h] [ebp-10h] BYREF
  int v13; // [esp+4h] [ebp-Ch]
  int v14; // [esp+8h] [ebp-8h]
  int v15; // [esp+Ch] [ebp-4h]

  v3 = *(_BYTE *)(this + 1524);
  if ( v3 == 1 )
  {
    v4 = *(_DWORD *)(this + 28);
    v12 = *(_DWORD *)(this + 24) + 300;
    v5 = *(_DWORD *)(this + 32);
    v6 = *(_DWORD *)(this + 36);
    v13 = v4 + 208;
    v14 = v5;
    v15 = v6;
    v7 = dword_4AF2AC(&v12, a2, a3);
  }
  else
  {
    if ( v3 != 4 )
      return 1;
    v8 = *(_DWORD *)(this + 28);
    v12 = *(_DWORD *)(this + 24) + 300;
    v9 = *(_DWORD *)(this + 32);
    v13 = v8 + 100;
    v10 = *(_DWORD *)(this + 36);
    v14 = v9;
    v15 = v10;
    v7 = dword_4AF2AC(&v12, a2, a3);
  }
  return !v7;
}
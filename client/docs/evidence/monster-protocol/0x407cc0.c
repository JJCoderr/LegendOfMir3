int __thiscall sub_407CC0(_DWORD *this, int a2)
{
  char *v3; // eax
  char *v4; // edi
  char v5; // al
  int v6; // edi
  bool v7; // zf
  __int16 v8; // bp
  char *v9; // eax
  char *v10; // edi
  void (__thiscall **v11)(_DWORD, _DWORD); // edx
  _DWORD *v12; // eax
  _DWORD *v13; // eax
  char *v14; // eax
  void (__thiscall **v15)(_DWORD, _DWORD); // edx
  int v17; // [esp-4h] [ebp-448h]
  int v18; // [esp-4h] [ebp-448h]
  int v19; // [esp+0h] [ebp-444h]
  int v20; // [esp+0h] [ebp-444h]
  char v21; // [esp+24h] [ebp-420h]
  int v22; // [esp+28h] [ebp-41Ch] BYREF
  int v23; // [esp+2Ch] [ebp-418h]
  char v24[16]; // [esp+38h] [ebp-40Ch] BYREF
  int v25; // [esp+440h] [ebp-4h]

  this[33] = *(unsigned __int16 *)(a2 + 6);
  this[34] = *(unsigned __int16 *)(a2 + 8);
  v21 = *(_BYTE *)(a2 + 10);
  *(_BYTE *)(this[100110] + 20) = *(_BYTE *)(a2 + 11);
  sub_47D780((const char *)(a2 + 12), (int)&v22, 16);
  sub_4832D0(109, this[33], this[34], 0);
  if ( strlen((const char *)(a2 + 12)) > 0x16 )
  {
    v24[sub_47D780((const char *)(a2 + 12), (int)v24, 1024)] = 0;
    sub_408450(this, v24);
    v3 = MEMORY[0x4A09E0](v24, 47);
    v4 = v3;
    if ( v3 )
    {
      v5 = MEMORY[0x49FAF5](v3 + 1);
      this[100107] = sub_401590(v5);
      *v4 = 0;
    }
  }
  v6 = v22;
  v7 = (v23 & 2) == 0;
  this[100099] = v23;
  if ( v7 )
  {
    *(_DWORD *)(this[100110] + 16) = 0;
  }
  else
  {
    *(_DWORD *)(this[100110] + 16) = MEMORY[0x49FDAE]();
    this[100785] = *(_DWORD *)(this[100110] + 16);
  }
  sub_4064B0((int)this, v6);
  v8 = *(_WORD *)(a2 + 4);
  if ( v8 == 801 )
  {
    v9 = (char *)MEMORY[0x4A01AA](0x240u);
    v25 = 0;
    if ( v9 )
      v10 = sub_449480(v9);
    else
      v10 = 0;
    v11 = *(void (__thiscall ***)(_DWORD, _DWORD))v10;
    v19 = this[34];
    v17 = this[33];
    v25 = -1;
    if ( ((int (__thiscall *)(char *, int, int, int, int, int, _DWORD, _DWORD, _DWORD))v11[1])(
           v10,
           321,
           v17,
           v19,
           v17,
           v19,
           0,
           0,
           0) )
    {
      if ( !dword_59A9F8 )
      {
        v12 = MEMORY[0x4A01AA](0x10u);
        if ( v12 )
        {
          *v12 = &off_4AF460;
          v12[1] = 0;
          v12[3] = 0;
          v12[2] = 0;
          goto LABEL_16;
        }
        goto LABEL_15;
      }
      goto LABEL_18;
    }
    if ( !v10 )
      return (*(int (__thiscall **)(_DWORD *, _DWORD, char, _DWORD))(*this + 16))(this, 0, v21, 0);
LABEL_32:
    (**(void (__thiscall ***)(char *, int))v10)(v10, 1);
    return (*(int (__thiscall **)(_DWORD *, _DWORD, char, _DWORD))(*this + 16))(this, 0, v21, 0);
  }
  if ( v8 != 808 )
    return (*(int (__thiscall **)(_DWORD *, _DWORD, char, _DWORD))(*this + 16))(this, 0, v21, 0);
  v14 = (char *)MEMORY[0x4A01AA](0x240u);
  v25 = 1;
  if ( v14 )
    v10 = sub_449480(v14);
  else
    v10 = 0;
  v15 = *(void (__thiscall ***)(_DWORD, _DWORD))v10;
  v20 = this[34];
  v18 = this[33];
  v25 = -1;
  if ( !((int (__thiscall *)(char *, int, int, int, int, int, _DWORD, _DWORD, _DWORD))v15[1])(
          v10,
          323,
          v18,
          v20,
          v18,
          v20,
          0,
          0,
          0) )
  {
    if ( !v10 )
      return (*(int (__thiscall **)(_DWORD *, _DWORD, char, _DWORD))(*this + 16))(this, 0, v21, 0);
    goto LABEL_32;
  }
  if ( !dword_59A9F8 )
  {
    v12 = MEMORY[0x4A01AA](0x10u);
    if ( v12 )
    {
      *v12 = &off_4AF460;
      v12[1] = 0;
      v12[3] = 0;
      v12[2] = 0;
LABEL_16:
      dword_59A9F8 = (int)v12;
      if ( v12 )
      {
        v12[1] = v10;
        v13 = (_DWORD *)dword_59A9F8;
LABEL_20:
        dword_59A9FC = (int)v13;
        ++dword_59AA08;
        return (*(int (__thiscall **)(_DWORD *, _DWORD, char, _DWORD))(*this + 16))(this, 0, v21, 0);
      }
      return (*(int (__thiscall **)(_DWORD *, _DWORD, char, _DWORD))(*this + 16))(this, 0, v21, 0);
    }
LABEL_15:
    v12 = 0;
    goto LABEL_16;
  }
LABEL_18:
  v13 = MEMORY[0x4A01AA](0x10u);
  if ( v13 )
  {
    *v13 = &off_4AF460;
    v13[1] = 0;
    v13[3] = 0;
    v13[2] = 0;
    v13[1] = v10;
    v13[3] = 0;
    v13[2] = dword_59A9FC;
    *(_DWORD *)(dword_59A9FC + 12) = v13;
    goto LABEL_20;
  }
  return (*(int (__thiscall **)(_DWORD *, _DWORD, char, _DWORD))(*this + 16))(this, 0, v21, 0);
}
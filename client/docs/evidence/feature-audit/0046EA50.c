int __thiscall sub_46EA50(_DWORD *this, int a2, int a3, int a4)
{
  __int16 v5; // ax
  const char *v6; // eax
  int v8; // [esp+0h] [ebp-10h]
  int v9; // [esp+4h] [ebp-Ch]

  if ( *(_DWORD *)a2 )
  {
    if ( !*(_DWORD *)(a2 + 4) && !*(_DWORD *)(a2 + 8) )
    {
      v5 = *(_WORD *)(a2 + 64);
      if ( v5 == 310 || v5 == 311 )
      {
        v6 = (const char *)sub_480550(4304);
        sub_420540((int)dword_82B050, (int)dword_5A041C, 112, 1, v6, 1, -1, -1, 0xFFFF, 0, 0, v8, v9);
        return 0;
      }
    }
  }
  if ( (*(int (__thiscall **)(_DWORD *, int, int, int))(this[21] + 16))(this + 21, a3, a4, 1) )
  {
    this[203] = 0;
    this[204] = 0;
    return 1;
  }
  if ( (*(int (__thiscall **)(_DWORD *, int, int, int))(this[66] + 16))(this + 66, a3, a4, 1) )
  {
    if ( this[203] && this[204] )
    {
      sub_47CFA0(*((unsigned __int16 *)this + 406) | (*((unsigned __int16 *)this + 408) << 16));
      return 0;
    }
  }
  else
  {
    if ( (*(int (__thiscall **)(_DWORD *, int, int, int))(this[111] + 16))(this + 111, a3, a4, 1) )
    {
      sub_47D000(dword_8FCDD8);
      return 0;
    }
    if ( (*(int (__thiscall **)(_DWORD *, int, int, int))(this[156] + 16))(this + 156, a3, a4, 1) )
    {
      this[203] = 0;
      this[204] = 0;
      sub_47CFD0(dword_8FCDD8);
    }
  }
  return 0;
}
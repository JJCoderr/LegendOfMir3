int __thiscall sub_408B70(char *this, int a2)
{
  unsigned __int16 v2; // dx
  char v3; // al
  unsigned __int16 v4; // ax
  int v5; // eax

  *((_DWORD *)this + 33) = *(unsigned __int16 *)(a2 + 6);
  v2 = *(_WORD *)(a2 + 8);
  LOBYTE(a2) = this[121];
  v3 = this[69];
  *((_DWORD *)this + 34) = v2;
  *((_DWORD *)this + 100098) = 1;
  if ( v3 == 3 )
    v4 = *(_WORD *)(this + 71);
  else
    v4 = (unsigned __int8)this[71];
  if ( v4 > 0x1Fu )
  {
    if ( v4 >= 0x212u && v4 <= 0x215u )
    {
      LOBYTE(a2) = 0;
      *((_DWORD *)this + 100098) = 1;
    }
  }
  else if ( v4 == 31 )
  {
    LOBYTE(a2) = 0;
    *((_DWORD *)this + 100098) = 0;
  }
  else if ( v4 == 3 )
  {
    *((_DWORD *)this + 100098) = 0;
  }
  v5 = *(_DWORD *)this;
  this[400444] = 2;
  return (*(int (__stdcall **)(int, int, _DWORD))(v5 + 16))(8, a2, 0);
}
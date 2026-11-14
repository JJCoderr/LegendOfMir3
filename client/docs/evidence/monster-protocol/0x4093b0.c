int __thiscall sub_4093B0(_DWORD *this, int a2)
{
  int v2; // edi
  char v4; // al
  __int16 v5; // di
  int result; // eax
  int v7; // edx
  int v8; // [esp-4h] [ebp-Ch]

  v2 = a2;
  v8 = a2;
  this[33] = *(unsigned __int16 *)(a2 + 6);
  this[34] = *(unsigned __int16 *)(a2 + 8);
  v4 = *(_BYTE *)(a2 + 11);
  LOBYTE(a2) = *(_BYTE *)(a2 + 10);
  *(_BYTE *)(this[100110] + 20) = v4;
  *(_BYTE *)(this[100110] + 9) = 0;
  (*(void (__thiscall **)(_DWORD *, int))(*this + 36))(this, v8);
  if ( dword_82DEF8 && **(_DWORD **)(dword_82DEF8 + 400440) == *(_DWORD *)v2 )
    dword_82DEF8 = 0;
  v5 = *(_WORD *)(v2 + 4);
  if ( v5 == 32 )
  {
    result = (*(int (__thiscall **)(_DWORD *, int, int, _DWORD))(*this + 16))(this, 4, a2, 0);
    v7 = this[29];
    this[100108] = 1;
    this[31] = v7 - 1;
  }
  else if ( v5 == 33 )
  {
    result = (*(int (__thiscall **)(_DWORD *, int, int, _DWORD))(*this + 16))(this, 10, a2, 0);
    this[100108] = 1;
  }
  else
  {
    result = (*(int (__thiscall **)(_DWORD *, int, int, _DWORD))(*this + 16))(this, 4, a2, 0);
    if ( *((_BYTE *)this + 69) == 3 )
      return sub_408C10(*(unsigned __int16 *)((char *)this + 71));
  }
  return result;
}
__int16 __thiscall sub_4082F0(_DWORD *this, int a2)
{
  int v2; // edi
  int v4; // edx
  char v5; // cl
  int v6; // ebx
  bool v7; // zf
  int v9; // [esp+Ch] [ebp-10h] BYREF
  int v10; // [esp+10h] [ebp-Ch]

  v2 = a2;
  v4 = this[100110];
  v5 = *(_BYTE *)(a2 + 11);
  LOBYTE(a2) = *(_BYTE *)(a2 + 10);
  *(_BYTE *)(v4 + 20) = v5;
  sub_47D780((const char *)(v2 + 12), (int)&v9, 16);
  v6 = v9;
  v7 = (v10 & 2) == 0;
  this[100099] = v10;
  if ( v7 )
  {
    *(_DWORD *)(this[100110] + 16) = 0;
  }
  else
  {
    *(_DWORD *)(this[100110] + 16) = MEMORY[0x49FDAE]();
    this[100785] = *(_DWORD *)(this[100110] + 16);
  }
  if ( (_BYTE)v6 == 3 )
    sub_4064B0((int)this, v6);
  (*(void (__thiscall **)(_DWORD *, int, int, _DWORD))(*this + 16))(this, 1, a2, 0);
  *(_BYTE *)(this[100110] + 4) = 1;
  sub_453450(
    *(unsigned __int16 *)(v2 + 6),
    *(unsigned __int16 *)(v2 + 8),
    a2,
    this + 33,
    this + 34,
    *(unsigned __int8 *)(this[100110] + 4));
  return sub_407120((int)this);
}
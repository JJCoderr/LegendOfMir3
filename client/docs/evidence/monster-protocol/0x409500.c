void __thiscall sub_409500(unsigned __int8 *this, int a2)
{
  unsigned __int8 v4; // al
  unsigned __int8 v5; // al
  int v6; // ebx
  bool v7; // zf
  unsigned __int8 v8; // al
  unsigned __int8 v9; // al
  int v10; // ecx
  unsigned __int8 v11; // [esp+8h] [ebp-14h]
  int v12; // [esp+Ch] [ebp-10h] BYREF
  int v13; // [esp+10h] [ebp-Ch]
  unsigned __int8 v14; // [esp+20h] [ebp+4h]

  v4 = *(_BYTE *)(a2 + 10);
  v11 = v4;
  if ( v4 >= 4u )
    v5 = v4 - 4;
  else
    v5 = v4 + 4;
  v14 = v5;
  *(_BYTE *)(*((_DWORD *)this + 100110) + 20) = *(_BYTE *)(a2 + 11);
  sub_47D780((const char *)(a2 + 12), (int)&v12, 16);
  v6 = v12;
  v7 = (v13 & 2) == 0;
  *((_DWORD *)this + 100099) = v13;
  if ( v7 )
  {
    *(_DWORD *)(*((_DWORD *)this + 100110) + 16) = 0;
  }
  else
  {
    *(_DWORD *)(*((_DWORD *)this + 100110) + 16) = MEMORY[0x49FDAE]();
    *((_DWORD *)this + 100785) = *(_DWORD *)(*((_DWORD *)this + 100110) + 16);
  }
  if ( (_BYTE)v6 == 3 )
    sub_4064B0((int)this, v6);
  (*(void (__thiscall **)(unsigned __int8 *, int, unsigned __int8, _DWORD))(*(_DWORD *)this + 16))(this, 1, v11, 0);
  v8 = this[121];
  if ( v8 >= 4u )
    v9 = v8 - 4;
  else
    v9 = v8 + 4;
  v10 = *((_DWORD *)this + 100110);
  this[122] = v9;
  *(_BYTE *)(v10 + 4) = 1;
  this[400204] = 0;
  this[400205] = 6;
  sub_453450(
    *(unsigned __int16 *)(a2 + 6),
    *(unsigned __int16 *)(a2 + 8),
    v14,
    (_DWORD *)this + 33,
    (_DWORD *)this + 34,
    1);
  sub_4075B0((int)this);
}
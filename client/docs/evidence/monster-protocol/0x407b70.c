int __thiscall sub_407B70(_DWORD *this, int a2)
{
  int result; // eax
  int v4; // ebx
  bool v5; // zf
  int v6; // [esp+8h] [ebp-10h] BYREF
  int v7; // [esp+Ch] [ebp-Ch]

  sub_47D780((const char *)(a2 + 12), (int)&v6, 16);
  result = v7;
  v4 = v6;
  v5 = (v7 & 2) == 0;
  this[100099] = v7;
  if ( v5 )
  {
    *(_DWORD *)(this[100110] + 16) = 0;
  }
  else
  {
    *(_DWORD *)(this[100110] + 16) = MEMORY[0x49FDAE]();
    result = this[100110];
    this[100785] = *(_DWORD *)(result + 16);
  }
  if ( (_BYTE)v4 == 3 )
    return sub_4064B0((int)this, v4);
  return result;
}
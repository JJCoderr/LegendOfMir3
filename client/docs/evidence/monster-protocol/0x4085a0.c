int __thiscall sub_4085A0(int this, int a2)
{
  int v2; // ebp
  char *v4; // edi
  char v5; // al
  int v6; // ebx
  bool v7; // zf
  int result; // eax
  int v9; // eax
  int v10; // [esp+10h] [ebp-50h] BYREF
  int v11; // [esp+14h] [ebp-4Ch]
  char Str[64]; // [esp+20h] [ebp-40h] BYREF

  v2 = a2;
  *(_DWORD *)(this + 132) = *(unsigned __int16 *)(a2 + 6);
  *(_DWORD *)(this + 136) = *(unsigned __int16 *)(a2 + 8);
  LOBYTE(a2) = *(_BYTE *)(a2 + 10);
  *(_BYTE *)(*(_DWORD *)(this + 400440) + 20) = *(_BYTE *)(v2 + 11);
  sub_47D780((const char *)(v2 + 12), (int)&v10, 16);
  if ( *(_WORD *)(v2 + 4) == 1316 )
  {
    *(_DWORD *)(this + 400380) = 600;
    *(_DWORD *)(this + 400432) = 0;
    *(_DWORD *)(this + 400384) = 0;
  }
  if ( strlen((const char *)(v2 + 12)) > 0x16 )
  {
    Str[sub_47D780((const char *)(v2 + 34), (int)Str, 64)] = 0;
    sub_408450((_DWORD *)this, Str);
    v4 = MEMORY[0x4A09E0](Str, 47);
    if ( v4 )
    {
      v5 = MEMORY[0x49FAF5](v4 + 1);
      *(_DWORD *)(this + 400428) = sub_401590(v5);
      *v4 = 0;
    }
  }
  v6 = v10;
  v7 = (v11 & 2) == 0;
  *(_DWORD *)(this + 400396) = v11;
  if ( v7 )
  {
    *(_DWORD *)(*(_DWORD *)(this + 400440) + 16) = 0;
  }
  else
  {
    *(_DWORD *)(*(_DWORD *)(this + 400440) + 16) = MEMORY[0x49FDAE]();
    *(_DWORD *)(this + 403140) = *(_DWORD *)(*(_DWORD *)(this + 400440) + 16);
  }
  if ( (_BYTE)v6 == 3 )
    sub_4064B0(this, v6);
  result = (*(int (__thiscall **)(int, _DWORD, int, _DWORD))(*(_DWORD *)this + 16))(this, 0, a2, 0);
  if ( (*(_BYTE *)(this + 400396) & 1) != 0 )
  {
    if ( *(_BYTE *)(this + 69) == 3 )
      LOWORD(result) = *(_WORD *)(this + 71);
    else
      LOWORD(result) = *(unsigned __int8 *)(this + 71);
    result = (unsigned __int16)result;
    if ( (unsigned __int16)result >= 0x53u && (unsigned __int16)result <= 0x55u )
    {
      (*(void (__thiscall **)(int, int, _DWORD, _DWORD))(*(_DWORD *)this + 16))(this, 8, 0, 0);
      v9 = *(_DWORD *)(this + 112);
      *(_BYTE *)(this + 120) = 0;
      result = v9 + 1;
      *(_DWORD *)(this + 116) = result;
    }
  }
  return result;
}
int __thiscall sub_470E60(int this)
{
  int v2; // eax
  int v3; // edx
  int v4; // eax
  void (__stdcall *v5)(int *, int, int, int, int); // edi
  int v6; // eax
  int v8; // [esp+F0h] [ebp-10h] BYREF
  int v9; // [esp+F4h] [ebp-Ch]
  int v10; // [esp+F8h] [ebp-8h]
  int v11; // [esp+FCh] [ebp-4h]

  sub_4710F0(this);
  if ( *(_BYTE *)(this + 84) != 1 )
  {
    v2 = *(_DWORD *)(this + 28);
    if ( byte_82452B )
    {
      v3 = v2 + 34;
      v4 = v2 + 14;
    }
    else
    {
      v3 = v2 + 50;
      v4 = v2 + 30;
    }
    v5 = (void (__stdcall *)(int *, int, int, int, int))dword_4AF2A8;
    dword_4AF2A8(&v8, *(_DWORD *)(this + 24) + 100, v4, *(_DWORD *)(this + 24) + 230, v3);
    v6 = sub_48BB04((int)&dword_8FCC50, (int)dword_4B582C, 9, 0, 700, 0, 0, 0, 1);
    sub_48BD63(&dword_8FCC50, 0, v8, v9, v10, v11, 14483420, 0, (int)byte_8244E9, v6);
    v5(
      &v8,
      *(_DWORD *)(this + 24) + 82,
      *(_DWORD *)(this + 28) + 14,
      *(_DWORD *)(this + 24) + 254,
      *(_DWORD *)(this + 28) + 34);
    if ( byte_82452B )
    {
      v9 += 15;
      v11 += 15;
      sub_48BD63(&dword_8FCC50, 0, v8, v9, v10, v11, 11860735, 0, (int)&byte_82452B, 0);
      if ( byte_82456E )
      {
        v9 += 14;
        v11 += 14;
        sub_48BD63(&dword_8FCC50, 0, v8, v9, v10, v11, (int)&off_96C8FF, 0, (int)&byte_82456E, 0);
      }
    }
    if ( byte_7C1E24 )
    {
      v5(
        &v8,
        *(_DWORD *)(this + 24) + 200,
        *(_DWORD *)(this + 28) + 66,
        *(_DWORD *)(this + 24) + 305,
        *(_DWORD *)(this + 28) + 81);
      sub_48BD63(&dword_8FCC50, 0, v8, v9, v10, v11, 15417076, 0, (int)&byte_7C1E24, 0);
      if ( sub_491C70(1298) )
        sub_48D5F8(
          v8 - 19,
          v9 + 1,
          *(__int16 *)dword_5A0458,
          *(__int16 *)(dword_5A0458 + 2),
          dword_5A045C,
          0xFFFF,
          0xFFFF);
    }
  }
  if ( *(_BYTE *)(this + 84) == 1 )
    nullsub_5();
  if ( *(_BYTE *)(this + 84) == 1 )
    sub_41FD20((_DWORD *)(this + 88), *(_DWORD *)(this + 24) + 201, *(_DWORD *)(this + 28) + 422);
  else
    sub_41FD20((_DWORD *)(this + 88), *(_DWORD *)(this + 24) + 289, *(_DWORD *)(this + 28) + 14);
  return (*(int (__thiscall **)(int))(*(_DWORD *)(this + 88) + 4))(this + 88);
}
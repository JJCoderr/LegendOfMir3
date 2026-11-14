int __thiscall sub_420840(int this, int a2, int a3)
{
  int v4; // ebx
  int v5; // esi

  if ( !*(_DWORD *)(this + 40) )
    return 0;
  if ( sub_491C70(*(_DWORD *)(this + 4)) )
    sub_48D9F7(
      *(_DWORD *)(this + 8),
      *(_DWORD *)(this + 12),
      **(__int16 **)(*(_DWORD *)(this + 1116) + 60),
      *(__int16 *)(*(_DWORD *)(*(_DWORD *)(this + 1116) + 60) + 2),
      *(_DWORD *)(*(_DWORD *)(this + 1116) + 64),
      800,
      600,
      0xFFFF,
      0xFFFF);
  sub_48BE0F(
    &dword_8FCC50,
    0,
    *(_DWORD *)(this + 24),
    *(_DWORD *)(this + 28),
    *(_DWORD *)(this + 32),
    *(_DWORD *)(this + 36),
    13171455,
    0,
    (const char *)(this + 44),
    0);
  v4 = 0;
  v5 = this + 604;
  do
  {
    if ( *(_DWORD *)(v5 - 40) )
    {
      if ( !dword_4AF2AC(v5 - 32, a2, a3) )
        *(_BYTE *)(v5 + 1) = 0;
      (*(void (__thiscall **)(int))(*(_DWORD *)(v5 - 36) + 4))(v5 - 36);
      if ( v4 == *(unsigned __int8 *)(this + 1122) && *(_BYTE *)(v5 + 1) != 2 )
      {
        if ( sub_491C70(*(_DWORD *)(v5 - 12)) )
          sub_48D9F7(
            *(_DWORD *)(v5 - 32),
            *(_DWORD *)(v5 - 28),
            **(__int16 **)(*(_DWORD *)(this + 1116) + 60),
            *(__int16 *)(*(_DWORD *)(*(_DWORD *)(this + 1116) + 60) + 2),
            *(_DWORD *)(*(_DWORD *)(this + 1116) + 64),
            800,
            600,
            0xFFFF,
            0xFFFF);
      }
    }
    ++v4;
    v5 += 184;
  }
  while ( v4 < 3 );
  return 1;
}
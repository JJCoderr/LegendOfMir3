int __thiscall sub_44A110(int this, char *a2, int a3)
{
  int v4; // ebx
  __int16 v5; // ax
  char v6; // al
  unsigned __int8 v7; // cl
  unsigned __int8 v8; // dl
  unsigned __int8 v9; // al
  unsigned __int8 v10; // cl
  char v11; // al
  unsigned __int8 v12; // cl
  unsigned __int8 v13; // dl
  unsigned __int8 v14; // al
  unsigned __int8 v15; // cl
  unsigned __int8 v16; // cl
  unsigned __int8 v17; // cl
  unsigned __int8 v18; // cl
  unsigned __int8 v19; // cl
  int v21; // [esp+8h] [ebp-4h] BYREF

  v4 = a3 + *(_DWORD *)(this + 220);
  v5 = *(_WORD *)(this + 16);
  *(_DWORD *)(this + 220) = v4;
  if ( v5 == 22
    || v5 == 16
    || v5 == 9
    || v5 == 30
    || v5 == 11
    || v5 == 330
    || v5 == 331
    || v5 == 332
    || v5 == 320
    || v5 == 321
    || v5 == 333
    || v5 == 334
    || v5 == 10
    || v5 == 538
    || v5 == 539
    || v5 == 322
    || v5 == 53
    || v5 == 63
    || v5 == 104
    || v5 == 336
    || v5 == 420
    || v5 == 421
    || v5 == 110
    || v5 == 323 )
  {
    v11 = *(_BYTE *)(this + 4);
    if ( v11 )
    {
      if ( v11 == 1 )
      {
        v14 = *(_BYTE *)(this + 234) - 2;
        *(_BYTE *)(this + 289) = 0;
        *(_BYTE *)(this + 234) = v14;
        if ( v14 <= 2u )
          *(_BYTE *)(this + 234) = 2;
        v15 = *(_BYTE *)(this + 235) - 2;
        *(_BYTE *)(this + 235) = v15;
        if ( v15 <= 2u )
          *(_BYTE *)(this + 235) = 2;
        v16 = *(_BYTE *)(this + 236) - 2;
        *(_BYTE *)(this + 236) = v16;
        if ( v16 <= 2u )
          *(_BYTE *)(this + 236) = 2;
        v17 = *(_BYTE *)(this + 237) - 2;
        *(_BYTE *)(this + 237) = v17;
        if ( v17 <= 2u )
          *(_BYTE *)(this + 237) = 2;
        v18 = *(_BYTE *)(this + 238) - 2;
        *(_BYTE *)(this + 238) = v18;
        if ( v18 <= 2u )
          *(_BYTE *)(this + 238) = 2;
        v19 = *(_BYTE *)(this + 239) - 2;
        *(_BYTE *)(this + 239) = v19;
        if ( v19 <= 2u )
          *(_BYTE *)(this + 239) = 2;
        if ( *(_BYTE *)(this + 234) == 2
          && *(_BYTE *)(this + 235) == 2
          && *(_BYTE *)(this + 236) == 2
          && *(_BYTE *)(this + 237) == 2
          && *(_BYTE *)(this + 238) == 2
          && *(_BYTE *)(this + 239) == 2 )
        {
          *(_BYTE *)(this + 4) = 2;
        }
      }
    }
    else if ( v4 > 200 )
    {
      v12 = *(_BYTE *)(this + 258);
      v13 = *(_BYTE *)(this + 289) + 1;
      *(_DWORD *)(this + 220) = 0;
      *(_BYTE *)(this + 289) = v13;
      if ( v13 > v12 )
        *(_BYTE *)(this + 289) = 0;
    }
  }
  else if ( v4 > 200 )
  {
    v6 = *(_BYTE *)(this + 4);
    if ( v6 )
    {
      if ( v6 == 1 )
      {
        v9 = *(_BYTE *)(this + 232);
        *(_BYTE *)(this + 289) = 0;
        *(_DWORD *)(this + 220) = 150;
        if ( v9 <= 1u || (v10 = *(_BYTE *)(this + 233), v10 <= v9) )
        {
          *(_BYTE *)(this + 4) = 2;
        }
        else
        {
          *(_BYTE *)(this + 232) = v9 - 1;
          *(_BYTE *)(this + 233) = v10 - 1;
        }
      }
    }
    else
    {
      v7 = *(_BYTE *)(this + 258);
      v8 = *(_BYTE *)(this + 289) + 1;
      *(_DWORD *)(this + 220) = 0;
      *(_BYTE *)(this + 289) = v8;
      if ( v8 > v7 )
        *(_BYTE *)(this + 289) = 0;
    }
  }
  if ( *(_DWORD *)(this + 292) && *(_BYTE *)(this + 4) <= 1u && dword_8F1D48 )
  {
    if ( *(_DWORD *)(this + 12) )
    {
      sub_453B60(&dword_5B03B0, *(_DWORD *)(this + 176), *(_DWORD *)(this + 180), &v21, &a3);
      sub_448C30(
        a2,
        v21 + 24,
        a3 + 16,
        *(unsigned __int8 *)(this + 289) + *(unsigned __int8 *)(this + 232),
        *(unsigned __int8 *)(this + 234),
        *(unsigned __int8 *)(this + 235),
        *(unsigned __int8 *)(this + 236),
        *(unsigned __int8 *)(this + 289) + *(unsigned __int8 *)(this + 233),
        *(unsigned __int8 *)(this + 237),
        *(unsigned __int8 *)(this + 238),
        *(unsigned __int8 *)(this + 239));
    }
    else
    {
      sub_448C30(
        a2,
        *(_DWORD *)(this + 160) + 24,
        *(_DWORD *)(this + 164) + 16,
        *(unsigned __int8 *)(this + 289) + *(unsigned __int8 *)(this + 232),
        *(unsigned __int8 *)(this + 234),
        *(unsigned __int8 *)(this + 235),
        *(unsigned __int8 *)(this + 236),
        *(unsigned __int8 *)(this + 289) + *(unsigned __int8 *)(this + 233),
        *(unsigned __int8 *)(this + 237),
        *(unsigned __int8 *)(this + 238),
        *(unsigned __int8 *)(this + 239));
    }
  }
  return 1;
}
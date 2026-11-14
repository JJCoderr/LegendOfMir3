int __thiscall sub_478850(int this, int a2, int a3)
{
  char v4; // al
  char v6; // al
  int i; // eax
  void *v8; // edi
  int v9; // ecx
  int v10; // eax
  char v11; // al
  char v12; // al
  int v13; // eax
  char v14; // al
  const char *v15; // eax
  char v16; // al
  int v17; // eax
  const char *v18; // eax
  int v19; // eax
  int v20; // eax
  int v21; // eax
  int v22; // eax
  int v23; // ecx
  int v24; // ebx
  int v25; // eax
  int v26; // [esp+0h] [ebp-10h]
  int v27; // [esp+4h] [ebp-Ch]

  v4 = *(_BYTE *)(this + 1524);
  if ( (!v4 || v4 == 4 || v4 == 1 || v4 == 3)
    && (*(int (__thiscall **)(int, int, int, int))(*(_DWORD *)(this + 84) + 16))(this + 84, a2, a3, 1) )
  {
    return 1;
  }
  v6 = *(_BYTE *)(this + 1524);
  if ( v6 == 1 || v6 == 4 || v6 == 2 )
  {
    if ( (*(int (__thiscall **)(int, int, int, int))(*(_DWORD *)(this + 444) + 16))(this + 444, a2, a3, 1) )
    {
      if ( *(_BYTE *)(this + 1524) == 2 )
        return 1;
      for ( i = *(_DWORD *)(this + 1768); i; i = *(_DWORD *)(this + 1768) )
      {
        v8 = *(void **)(i + 4);
        if ( v8 )
        {
          nullsub_5();
          MEMORY[0x4939F6](v8);
        }
        v9 = *(_DWORD *)(this + 1768);
        v10 = *(_DWORD *)(v9 + 12);
        *(_DWORD *)(this + 1768) = v10;
        if ( v10 )
          *(_DWORD *)(v10 + 8) = 0;
        if ( v9 )
          (**(void (__thiscall ***)(int, int))v9)(v9, 1);
      }
      *(_DWORD *)(this + 1768) = 0;
      *(_DWORD *)(this + 1772) = 0;
      *(_DWORD *)(this + 1784) = 0;
      *(_DWORD *)(this + 1980) = 0;
      *(_BYTE *)(this + 1524) = 0;
      sub_4317B0((_DWORD *)this, 1260, 0, 225, 292, 224, v26, v27);
      return 0;
    }
    if ( (*(int (__thiscall **)(int, int, int, int))(*(_DWORD *)(this + 1164) + 16))(this + 1164, a2, a3, 1) )
    {
      sub_4317B0((_DWORD *)this, 1260, 0, 225, 292, 224, v26, v27);
      *(_DWORD *)(this + 2024) = 0;
      *(_BYTE *)(this + 1524) = 0;
      *(_BYTE *)(this + 1988) = 0;
      return 0;
    }
  }
  v11 = *(_BYTE *)(this + 1524);
  if ( (!v11 || v11 == 4 || v11 == 1 || v11 == 3) && sub_420360((_DWORD *)(this + 1528), a2, a3) )
    return 0;
  v12 = *(_BYTE *)(this + 1524);
  if ( !v12 || v12 == 4 || v12 == 1 || v12 == 3 )
  {
    if ( (*(int (__thiscall **)(int, int, int, int))(*(_DWORD *)(this + 264) + 16))(this + 264, a2, a3, 1) )
    {
      if ( *(_DWORD *)(this + 1760) != -1 )
      {
        v13 = sub_478570((_DWORD *)this, *(_DWORD *)(this + 1760));
        if ( v13 )
        {
          if ( *(_DWORD *)(v13 + 32) == 2 )
          {
            v14 = *(_BYTE *)(this + 1524);
            if ( v14 )
            {
              if ( v14 == 3 )
                *(_BYTE *)(this + 1988) = 1;
            }
            else
            {
              *(_BYTE *)(this + 1988) = 0;
            }
            *(_BYTE *)(this + 1524) = 4;
            sub_4317B0((_DWORD *)this, 1262, 0, 225, 516, 224, v26, v27);
            *(_DWORD *)(this + 2024) = sub_478570((_DWORD *)this, *(_DWORD *)(this + 1760));
            return 0;
          }
          else
          {
            sub_47CC30(*(_DWORD *)(this + 2028), 0, v13);
            return 0;
          }
        }
      }
    }
  }
  if ( *(_BYTE *)(this + 1524) == 4
    && (*(int (__thiscall **)(int, int, int, int))(*(_DWORD *)(this + 1344) + 16))(this + 1344, a2, a3, 1)
    && *(_DWORD *)(this + 2024) )
  {
    if ( *(_BYTE *)(this + 1988) == 1 )
    {
      sub_47CCF0(*(_DWORD *)(this + 2028), *(_DWORD *)(this + 2024));
    }
    else
    {
      v15 = (const char *)sub_480550(2302);
      sub_420540((int)dword_82B050, (int)dword_5A041C, 110, 1, v15, 1, -1, -1, 0xFFFF, 0, 0, v26, v27);
    }
    return 0;
  }
  v16 = *(_BYTE *)(this + 1524);
  if ( v16 != 1 && v16 != 2
    || !(*(int (__thiscall **)(int, int, int, int))(*(_DWORD *)(this + 624) + 16))(this + 624, a2, a3, 1)
    || *(_DWORD *)(this + 2032) == -1
    || (v17 = sub_478670((_DWORD *)this, *(_DWORD *)(this + 2032))) == 0 )
  {
    if ( *(_BYTE *)(this + 1524) == 1
      && (*(int (__thiscall **)(int, int, int, int))(*(_DWORD *)(this + 804) + 16))(this + 804, a2, a3, 1) )
    {
      v19 = *(_DWORD *)(this + 1984);
      if ( v19 > 0 )
      {
        *(_DWORD *)(this + 1984) = v19 - 1;
        if ( *(_DWORD *)(this + 1760) != -1 )
        {
          v20 = sub_478570((_DWORD *)this, *(_DWORD *)(this + 1760));
          if ( v20 )
          {
LABEL_68:
            sub_47CC30(*(_DWORD *)(this + 2028), 10 * *(_DWORD *)(this + 1984), v20);
            *(_DWORD *)(this + 1984) = -1;
            return 0;
          }
        }
      }
    }
    else if ( *(_BYTE *)(this + 1524) == 2
           && (*(int (__thiscall **)(int, int, int, int))(*(_DWORD *)(this + 804) + 16))(this + 804, a2, a3, 1) )
    {
      v21 = *(_DWORD *)(this + 1980);
      if ( v21 > 0 )
      {
        *(_DWORD *)(this + 1980) = v21 - 1;
        return 0;
      }
    }
    else if ( *(_BYTE *)(this + 1524) == 1
           && (*(int (__thiscall **)(int, int, int, int))(*(_DWORD *)(this + 984) + 16))(this + 984, a2, a3, 1) )
    {
      v22 = *(_DWORD *)(this + 1760);
      ++*(_DWORD *)(this + 1984);
      if ( v22 != -1 )
      {
        v20 = sub_478570((_DWORD *)this, v22);
        if ( v20 )
          goto LABEL_68;
      }
    }
    else if ( *(_BYTE *)(this + 1524) == 2
           && ((v23 = *(_DWORD *)(this + 1784), !(v23 % 12)) ? (v24 = v23 / 12) : (v24 = v23 / 12 + 1),
               (*(int (__thiscall **)(int, int, int, int))(*(_DWORD *)(this + 984) + 16))(this + 984, a2, a3, 1)) )
    {
      v25 = *(_DWORD *)(this + 1980);
      if ( v25 != -1 && v25 < v24 - 1 )
      {
        *(_DWORD *)(this + 1980) = v25 + 1;
        return 0;
      }
    }
    else
    {
      if ( sub_4785A0((_DWORD *)this, a2, a3) != -1 )
        *(_DWORD *)(this + 1760) = sub_4785A0((_DWORD *)this, a2, a3);
      if ( sub_4786A0((_DWORD *)this, a2, a3) != -1 )
        *(_DWORD *)(this + 2032) = sub_4786A0((_DWORD *)this, a2, a3);
    }
    return 0;
  }
  if ( *(_BYTE *)(this + 1524) == 1 )
  {
    sub_47CC70(*(_DWORD *)(this + 2028), v17 + 4, *(_DWORD *)(v17 + 66));
    return 0;
  }
  if ( *(_BYTE *)(v17 + 65) < 2u )
  {
    sub_47CD30(*(_DWORD *)(this + 2028), v17 + 4, *(_DWORD *)(v17 + 66), 0);
  }
  else
  {
    if ( *(_WORD *)(v17 + 75) > 1u )
    {
      v18 = (const char *)sub_480550(2301);
      sub_420540((int)dword_82B050, (int)dword_5A041C, 107, 1, v18, 1, -1, -1, 0xFFFF, 0, 0, v26, v27);
      return 0;
    }
    sub_47CD30(*(_DWORD *)(this + 2028), v17 + 4, *(_DWORD *)(v17 + 66), 1);
  }
  return 0;
}
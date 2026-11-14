int __thiscall sub_479100(int this, int __return_address, int a3)
{
  char v4; // al
  int v5; // eax
  int v6; // eax
  char v7; // al
  int v9; // eax
  int v10; // ecx
  int (__stdcall *v11)(_DWORD *, int, int); // ebx
  int v12; // eax
  char v13; // al
  int v14; // eax
  int v15; // eax
  const char *v16; // eax
  int v17; // [esp+4h] [ebp-30h]
  int v18; // [esp+8h] [ebp-2Ch]
  _DWORD v19[4]; // [esp+14h] [ebp-20h] BYREF
  _DWORD v20[4]; // [esp+24h] [ebp-10h] BYREF

  v4 = *(_BYTE *)(this + 1524);
  if ( (!v4 || v4 == 4 || v4 == 1 || v4 == 3)
    && sub_4785A0((_DWORD *)this, __return_address, a3) != -1
    && (v5 = sub_4785A0((_DWORD *)this, __return_address, a3),
        *(_DWORD *)(this + 1760) = v5,
        (v6 = sub_478570((_DWORD *)this, v5)) != 0) )
  {
    if ( *(_DWORD *)(v6 + 32) == 2 )
    {
      v7 = *(_BYTE *)(this + 1524);
      if ( v7 )
      {
        if ( v7 == 3 )
          *(_BYTE *)(this + 1988) = 1;
      }
      else
      {
        *(_BYTE *)(this + 1988) = 0;
      }
      *(_BYTE *)(this + 1524) = 4;
      sub_4317B0((_DWORD *)this, 1262, 0, 225, 516, 224, v17, v18);
      *(_DWORD *)(this + 2024) = sub_478570((_DWORD *)this, *(_DWORD *)(this + 1760));
      return 0;
    }
    else
    {
      sub_47CC30(dword_8FCDD8, *(_DWORD *)(this + 2028), 0, (const char *)v6);
      return 0;
    }
  }
  else
  {
    if ( *(_BYTE *)(this + 1524) != 4 )
      goto LABEL_22;
    v9 = *(_DWORD *)(this + 24);
    v10 = *(_DWORD *)(this + 28);
    v19[0] = v9 + *(_DWORD *)(this + 1992);
    v19[1] = v10 + *(_DWORD *)(this + 1996);
    v19[2] = v9 + *(_DWORD *)(this + 2000);
    v19[3] = v10 + *(_DWORD *)(this + 2004);
    dword_4AF2A8(
      v20,
      v9 + *(_DWORD *)(this + 2008),
      v10 + *(_DWORD *)(this + 2012),
      v9 + *(_DWORD *)(this + 2016),
      v10 + *(_DWORD *)(this + 2020));
    v11 = (int (__stdcall *)(_DWORD *, int, int))dword_4AF2AC;
    if ( !dword_4AF2AC(v19, __return_address, a3) && !v11(v20, __return_address, a3) )
      goto LABEL_22;
    v12 = *(_DWORD *)(this + 2024);
    if ( v12 )
    {
      if ( *(_BYTE *)(this + 1988) == 1 )
        sub_47CCF0(dword_8FCDD8, *(_DWORD *)(this + 2028), *(const char **)(this + 2024));
      else
        sub_47CCB0(*(_DWORD *)(this + 2028), v12, *(_DWORD *)(v12 + 40), 1);
      return 0;
    }
    else
    {
LABEL_22:
      v13 = *(_BYTE *)(this + 1524);
      if ( (v13 == 1 || v13 == 2) && sub_4786A0((_DWORD *)this, __return_address, a3) != -1 )
      {
        v14 = sub_4786A0((_DWORD *)this, __return_address, a3);
        *(_DWORD *)(this + 2032) = v14;
        v15 = sub_478670((_DWORD *)this, v14);
        if ( v15 )
        {
          if ( *(_BYTE *)(this + 1524) == 1 )
          {
            sub_47CC70(dword_8FCDD8, *(_DWORD *)(this + 2028), (const char *)(v15 + 4), *(_DWORD *)(v15 + 66));
            return 0;
          }
          if ( *(_BYTE *)(v15 + 65) < 2u )
          {
            sub_47CD30(*(_DWORD *)(this + 2028), v15 + 4, *(_DWORD *)(v15 + 66), 0);
          }
          else
          {
            if ( *(_WORD *)(v15 + 75) > 1u )
            {
              v16 = (const char *)sub_480550(2301);
              sub_420540((int)dword_82B050, (int)dword_5A041C, 107, 1, v16, 1, -1, -1, 0xFFFF, 0, 0, v17, v18);
              return 0;
            }
            sub_47CD30(*(_DWORD *)(this + 2028), v15 + 4, *(_DWORD *)(v15 + 66), 1);
          }
        }
      }
      return 0;
    }
  }
}
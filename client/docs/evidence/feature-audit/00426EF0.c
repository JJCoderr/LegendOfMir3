int __thiscall sub_426EF0(int this, int a2, int a3)
{
  int v4; // ebx
  int v5; // edi
  int v6; // ebp
  int v7; // edi
  int result; // eax
  int v9; // eax
  int v10; // eax
  char v11; // cl
  unsigned __int16 (__stdcall *v12)(int); // ebx
  char v13; // al
  int v14; // eax
  int v15; // edi
  int v16; // ebx
  const char *v17; // eax
  int v18; // eax
  int v19; // eax
  unsigned int v20; // kr10_4
  unsigned int v21; // kr14_4
  int v22; // ecx
  unsigned __int8 v23; // al
  int v24; // [esp+14h] [ebp-4h]

  v4 = 0;
  v5 = 0;
  v6 = 0;
  LOBYTE(v24) = 0;
  if ( *(_DWORD *)(this + 3614112) )
  {
    v7 = this + 3614084;
    if ( (*(int (__thiscall **)(int, int, int))(*(_DWORD *)(this + 3614084) + 36))(this + 3614084, a2, a3) )
    {
      (*(void (__thiscall **)(int))(*(_DWORD *)v7 + 20))(this + 3614084);
      *(_DWORD *)(this + 84) = 1;
      *(_BYTE *)(this + 4424760) = 1;
      *(_DWORD *)(this + 4424764) = 0;
      return 0;
    }
    return 0;
  }
  if ( sub_420A10(this + 3610584, 0, a2, a3) || !*(_DWORD *)(this + 3583700) || sub_43D760(this + 2780620, a2, a3) )
    return 0;
  if ( *(_DWORD *)(this + 3622536) && sub_430050(a2) )
  {
    v9 = *(_DWORD *)(this + 3622536);
    v6 = *(_DWORD *)(v9 + 132);
    v4 = *(_DWORD *)(v9 + 136);
    v5 = **(_DWORD **)(v9 + 400440);
    v24 = *(_DWORD *)(v9 + 69);
  }
  v10 = *(_DWORD *)(this + 3622516);
  if ( v10 )
  {
    if ( *(_DWORD *)(this + 92) )
    {
      v11 = *(_BYTE *)(v10 + 69);
      if ( v11 == 3 || (!*(_DWORD *)(v10 + 400432) && !v11 || v11 == 1) && *(_DWORD *)(v10 + 400428) == 27631 )
        *(_DWORD *)(this + 3622536) = v10;
    }
    v6 = *(_DWORD *)(v10 + 132);
    v4 = *(_DWORD *)(v10 + 136);
    v5 = **(_DWORD **)(v10 + 400440);
    v24 = *(_DWORD *)(v10 + 69);
  }
  sub_4197C0(this + 3179944, v6, a2, a2, a3, *(_DWORD *)(this + 3583684), *(_DWORD *)(this + 3583688), v6, v4, v5, v24);
  v12 = (unsigned __int16 (__stdcall *)(int))dword_4AF230;
  if ( (unsigned __int16)dword_4AF230(77) >> 8 && ((v13 = *(_BYTE *)(this + 3180064)) == 0 || v13 == 29) )
  {
    if ( *(_BYTE *)(this + 3583288) )
      v14 = sub_480550(1419);
    else
      v14 = sub_480550(1418);
    sub_47CA80((char *)dword_8FCDD8, v14);
    *(_DWORD *)(this + 4422616) = 1000;
    *(_DWORD *)(this + 4422620) = 0;
    return 0;
  }
  else
  {
    switch ( a2 )
    {
      case 13:
      case 32:
        sub_47B3D0(&dword_8FB928);
        dword_4AF2B0(dword_8FB92C);
        dword_4AF2A0(dword_8FB92C, 5);
        v19 = *(_DWORD *)(this + 2809892);
        if ( v19 == 2 )
        {
          dword_4AF26C(dword_8FB92C, asc_4B560C);
          v20 = strlen(asc_4B560C) + 1;
          dword_4AF2B4(dword_8FB92C, 177, v20 - 1, v20 - 1);
        }
        else if ( v19 == 1 )
        {
          dword_4AF26C(dword_8FB92C, asc_4B5610);
          v21 = strlen(asc_4B5610) + 1;
          dword_4AF2B4(dword_8FB92C, 177, v21 - 1, v21 - 1);
        }
        sub_427400((char *)this, 0, 0);
        return 0;
      case 65:
      case 97:
        if ( !(v12(17) >> 8) )
          return 0;
        v18 = sub_480550(1423);
        goto LABEL_36;
      case 72:
      case 104:
        if ( !(v12(17) >> 8) )
          return 0;
        v18 = sub_480550(1422);
LABEL_36:
        sub_47CA80((char *)dword_8FCDD8, v18);
        return 0;
      case 85:
        if ( *(_DWORD *)(this + 60) )
        {
          v22 = 0;
          v23 = *(_BYTE *)(this + 3583708) + 1;
          *(_BYTE *)(this + 3583708) = v23;
          if ( v23 > 2u )
            *(_BYTE *)(this + 3583708) = 0;
          if ( *(_BYTE *)(this + 3583708) )
          {
            if ( *(_BYTE *)(this + 3583708) == 1 )
            {
              *(_DWORD *)(this + 4422624) = 1097859072;
              v22 = 986895;
            }
            else if ( *(_BYTE *)(this + 3583708) == 2 )
            {
              *(_DWORD *)(this + 4422624) = 1118437376;
              v22 = (int)byte_555555;
            }
          }
          else
          {
            *(_DWORD *)(this + 4422624) = 1132396544;
            v22 = 0xFFFFFF;
          }
          sub_448B90((_DWORD *)(this + 3583712), v22);
        }
        return 0;
      case 187:
        *(_DWORD *)(this + 4424912) = *(_DWORD *)(this + 4424912) == 0;
        v15 = sub_401430(2);
        v16 = sub_401430(7);
        if ( *(_DWORD *)(this + 4424912) )
          v17 = (const char *)sub_480550(1518);
        else
          v17 = (const char *)sub_480550(1517);
        goto LABEL_30;
      case 220:
        *(_DWORD *)(this + 4424908) = *(_DWORD *)(this + 4424908) == 0;
        v15 = sub_401430(2);
        v16 = sub_401430(7);
        if ( *(_DWORD *)(this + 4424908) )
          v17 = (const char *)sub_480550(1515);
        else
          v17 = (const char *)sub_480550(1516);
LABEL_30:
        sub_4366D0((_DWORD *)(this + 2780620), v15, v16, v17);
        result = 0;
        break;
      default:
        return 0;
    }
  }
  return result;
}
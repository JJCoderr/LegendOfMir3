int __thiscall sub_43D760(int this, int a2, int a3)
{
  int result; // eax
  unsigned __int16 (__stdcall *v5)(int); // ebx
  int v6; // ecx
  int v7; // eax
  int v8; // esi
  int v9; // edi
  int v10; // eax
  int v11; // eax
  const char *v12; // eax
  const char *v13; // eax
  const char *v14; // eax
  int v15; // edx
  int v16; // eax
  unsigned __int8 v17; // cl
  int v18; // esi
  unsigned int *v19; // edx
  int v20; // eax
  int *v21; // esi
  unsigned int v22; // eax
  int v23; // eax
  int v24; // eax
  int v25; // esi
  int v26; // eax
  int v27; // eax
  int v28; // esi
  int v29; // esi
  int v30; // eax
  int v31; // esi
  int v32; // eax
  unsigned int v33; // kr24_4
  char v34[260]; // [esp+2Ch] [ebp-208h] BYREF
  _BYTE v35[260]; // [esp+130h] [ebp-104h] BYREF

  if ( *(_DWORD *)(this + 398660) )
    return 1;
  if ( *(_DWORD *)(this + 357736) )
    return 1;
  if ( *(_DWORD *)(this + 344504) && sub_43BA50((_DWORD *)this, 8) && sub_41BD30(a2, a3) )
    return 1;
  if ( *(_DWORD *)(this + 378136) && sub_43BA50((_DWORD *)this, 19) && sub_455A30(a2, a3) )
    return 1;
  if ( *(_DWORD *)(this + 387096) && sub_43BA50((_DWORD *)this, 21) && sub_45BA30(a2, a3) )
    return 1;
  if ( *(_DWORD *)(this + 32) && *(_DWORD *)(this + 36) )
    return 1;
  v5 = (unsigned __int16 (__stdcall *)(int))dword_4AF230;
  if ( (unsigned __int16)dword_4AF230(87) >> 8 )
  {
    if ( sub_46B6E0(this) )
    {
      sub_43AB90((_DWORD *)this, 0);
      nullsub_5();
      sub_441FD0((_BYTE *)(this + 29440), 0);
    }
    goto LABEL_105;
  }
  if ( v5(81) >> 8 )
  {
    if ( sub_46B6E0(this) )
    {
      sub_43AB90((_DWORD *)this, 1);
      nullsub_5();
    }
    goto LABEL_105;
  }
  if ( v5(69) >> 8 )
  {
    sub_43AB90((_DWORD *)this, 14);
    *(_DWORD *)(this + 357652) = 0;
    goto LABEL_105;
  }
  if ( v5(82) >> 8 )
  {
    sub_43EF10((_DWORD *)this);
    goto LABEL_105;
  }
  if ( v5(83) >> 8 )
  {
    sub_43AB90((_DWORD *)this, 13);
    goto LABEL_105;
  }
  if ( v5(74) >> 8 )
  {
    if ( (unsigned int)(((int (__cdecl *)())dword_4AF2FC)() - *(_DWORD *)(this + 27320)) > 0x3E8 )
    {
      if ( *(_DWORD *)(this + 29276) )
      {
        *(_DWORD *)(this + 29264) = 0;
        *(_DWORD *)(this + 29276) = 0;
      }
      else
      {
        sub_47BF30(dword_8FCDD8);
        v6 = *(_DWORD *)(this + 29276);
        *(_DWORD *)(this + 29368) = 1;
        *(_DWORD *)(this + 29276) = v6 == 0;
      }
    }
    if ( dword_4AF110(260, v34) )
    {
      if ( !v34[0] )
        return 1;
      strcat(v34, MEMORY[0x4B5864]);
      MEMORY[0x49FA18](v35, "%d", *(_DWORD *)(this + 29276));
      dword_4AF0C4(MEMORY[0x4B584C], MEMORY[0x4B5854], v35, v34);
    }
    goto LABEL_105;
  }
  if ( v5(79) >> 8 )
  {
    dword_8F1D30 = dword_8F1D30 == 0;
    if ( dword_4AF110(260, v34) )
    {
      if ( !v34[0] )
        return 1;
      strcat(v34, MEMORY[0x4B5864]);
      MEMORY[0x49FA18](v35, "%d", dword_8F1D30);
      dword_4AF0C4(MEMORY[0x4B584C], MEMORY[0x4B5884], v35, v34);
    }
    goto LABEL_105;
  }
  if ( v5(73) >> 8 )
  {
    sub_401430(2);
    sub_401430(7);
    dword_76740C = dword_76740C == 0;
    if ( dword_4AF110(260, v34) )
    {
      if ( !v34[0] )
        return 1;
      strcat(v34, MEMORY[0x4B5864]);
      MEMORY[0x49FA18](v35, "%d", dword_76740C);
      dword_4AF0C4(MEMORY[0x4B584C], MEMORY[0x4B587C], v35, v34);
    }
    goto LABEL_105;
  }
  if ( v5(75) >> 8 )
  {
    v7 = *(_DWORD *)(this + 29272) + 1;
    *(_DWORD *)(this + 29272) = v7;
    if ( v7 > 2 )
      *(_DWORD *)(this + 29272) = 0;
    v8 = sub_401430(2);
    v9 = sub_401430(7);
    v10 = *(_DWORD *)(this + 29272);
    if ( v10 )
    {
      v11 = v10 - 1;
      if ( v11 )
      {
        if ( v11 == 1 )
        {
          v12 = (const char *)sub_480550(2628);
          sub_4366D0((_DWORD *)this, v8, v9, v12);
          sub_41FD70(1334, 1334, 1334);
          sub_41FD70(1337, 1337, 1337);
        }
        goto LABEL_105;
      }
      v13 = (const char *)sub_480550(2627);
      sub_4366D0((_DWORD *)this, v8, v9, v13);
      sub_41FD70(1335, 1335, 1335);
    }
    else
    {
      v14 = (const char *)sub_480550(2626);
      sub_4366D0((_DWORD *)this, v8, v9, v14);
      sub_41FD70(1334, 1334, 1334);
    }
    sub_41FD70(1336, 1336, 1336);
LABEL_105:
    switch ( a2 )
    {
      case 9:
        dword_8F1D34 = 1;
        return sub_43A840((_DWORD *)this, dword_82473C, dword_824740) == 14
            && sub_450510(this + 353908, a2, a3, dword_82473C, dword_824740);
      case 13:
        return 0;
      case 27:
        if ( !*(_DWORD *)(this + 3696) )
          return sub_43A840((_DWORD *)this, dword_82473C, dword_824740) == 14
              && sub_450510(this + 353908, a2, a3, dword_82473C, dword_824740);
        v18 = dword_8FCC58;
        if ( dword_4AF248() != v18 )
          return sub_43A840((_DWORD *)this, dword_82473C, dword_824740) == 14
              && sub_450510(this + 353908, a2, a3, dword_82473C, dword_824740);
        v19 = *(unsigned int **)(this + 3688);
        v20 = *(_DWORD *)(this + 3696) - 1;
        v21 = (int *)v19;
        *(_DWORD *)(this + 3684) = v19;
        *(_DWORD *)(this + 3692) = v20;
        v22 = *v19;
        if ( *v19 == 9 && dword_8F1B10 )
        {
          v23 = *(_DWORD *)(this + 3692);
          *(_DWORD *)(this + 3684) = v19[2];
          v24 = v23 <= 1 ? 1 : v23 - 1;
          *(_DWORD *)(this + 3692) = v24;
          v21 = *(int **)(this + 3684);
          v22 = *v21;
          if ( *v21 == 9 )
            return 1;
        }
        if ( !v21 )
          return 1;
        if ( v22 == 9 )
        {
          sub_424AD0((int)dword_4B9878);
          return 1;
        }
        else if ( v22 == 21 )
        {
          sub_45C450(this + 387048);
          return 1;
        }
        else if ( v22 < 2 )
        {
          if ( sub_46B6E0(this) )
          {
            sub_43AB90((_DWORD *)this, *v21);
            nullsub_5();
          }
          return 1;
        }
        else
        {
          sub_43AB90((_DWORD *)this, v22);
          return 1;
        }
      case 38:
        if ( dword_82B078 || (v25 = dword_8FB92C, dword_4AF248() != v25) )
        {
          sub_43D310(this);
        }
        else
        {
          if ( (v5(17) & 0x80u) != 0 )
          {
            v26 = *(_DWORD *)(this + 29408) - 1;
            *(_DWORD *)(this + 29408) = v26;
            if ( v26 < 0 )
              *(_DWORD *)(this + 29408) = 0;
LABEL_134:
            sub_437240(this);
            return 1;
          }
          if ( (v5(18) & 0x80u) != 0 )
          {
            v27 = *(_DWORD *)(this + 29436) - 1;
            *(_DWORD *)(this + 29436) = v27;
            if ( v27 < 0 )
              *(_DWORD *)(this + 29436) = 0;
LABEL_138:
            sub_4378E0(this);
            return 1;
          }
        }
        return 1;
      case 40:
        if ( dword_82B078 || (v28 = dword_8FB92C, dword_4AF248() != v28) )
        {
          sub_43D490(this);
        }
        else
        {
          if ( (v5(17) & 0x80u) != 0 )
          {
            v29 = *(_DWORD *)(this + 29408) + 1;
            *(_DWORD *)(this + 29408) = v29;
            v30 = *(_DWORD *)(this + 29404);
            if ( v29 >= v30 )
              *(_DWORD *)(this + 29408) = v30 - 1;
            goto LABEL_134;
          }
          if ( (v5(18) & 0x80u) != 0 )
          {
            v31 = *(_DWORD *)(this + 29436) + 1;
            *(_DWORD *)(this + 29436) = v31;
            v32 = *(_DWORD *)(this + 29432);
            if ( v31 >= v32 )
              *(_DWORD *)(this + 29436) = v32 - 1;
            goto LABEL_138;
          }
        }
        return 1;
      case 49:
      case 52:
        result = sub_41A3D0(a2, a3);
        if ( result == 1 )
          return result;
        return sub_43A840((_DWORD *)this, dword_82473C, dword_824740) == 14
            && sub_450510(this + 353908, a2, a3, dword_82473C, dword_824740);
      case 50:
      case 53:
        result = sub_41A3D0(a2, a3);
        if ( result == 1 )
          return result;
        return sub_43A840((_DWORD *)this, dword_82473C, dword_824740) == 14
            && sub_450510(this + 353908, a2, a3, dword_82473C, dword_824740);
      case 51:
      case 54:
        result = sub_41A3D0(a2, a3);
        if ( result == 1 )
          return result;
        return sub_43A840((_DWORD *)this, dword_82473C, dword_824740) == 14
            && sub_450510(this + 353908, a2, a3, dword_82473C, dword_824740);
      case 191:
        dword_4AF2B0(dword_8FB92C);
        dword_4AF2A0(dword_8FB92C, 5);
        if ( !*(_BYTE *)(this + 399056) )
          strcpy((char *)(this + 399056), asc_4B561C);
        dword_4AF26C(dword_8FB92C, this + 399056);
        v33 = strlen((const char *)(this + 399056)) + 1;
        dword_4AF2B4(dword_8FB92C, 177, v33 - 1, v33 - 1);
        sub_47B3D0(&dword_8FB928);
        return sub_43A840((_DWORD *)this, dword_82473C, dword_824740) == 14
            && sub_450510(this + 353908, a2, a3, dword_82473C, dword_824740);
      case 192:
        dword_8F1D3C = 1;
        return sub_43A840((_DWORD *)this, dword_82473C, dword_824740) == 14
            && sub_450510(this + 353908, a2, a3, dword_82473C, dword_824740);
      default:
        return sub_43A840((_DWORD *)this, dword_82473C, dword_824740) == 14
            && sub_450510(this + 353908, a2, a3, dword_82473C, dword_824740);
    }
  }
  if ( v5(68) >> 8 )
  {
    sub_43AB90((_DWORD *)this, 11);
    goto LABEL_105;
  }
  if ( v5(76) >> 8 )
  {
    dword_4B9880 = dword_4B9880 == 0;
    if ( dword_4AF110(260, v34) )
    {
      if ( !v34[0] )
        return 1;
      strcat(v34, MEMORY[0x4B5864]);
      MEMORY[0x49FA18](v35, "%d", dword_4B9880);
      dword_4AF0C4(MEMORY[0x4B584C], MEMORY[0x4B5870], v35, v34);
    }
    goto LABEL_105;
  }
  if ( !(v5(80) >> 8) )
  {
    if ( v5(90) >> 8 )
    {
      sub_41A3D0(a2, a3);
    }
    else if ( v5(67) >> 8 )
    {
      LOBYTE(v15) = byte_7C1E99;
      v16 = sub_4281B0(dword_7C1EA4, dword_7C1EA8, v15, 1);
      if ( v16 )
        sub_47C2E0(v16 + 403145);
    }
    else if ( v5(86) >> 8 )
    {
      if ( (unsigned int)(((int (__cdecl *)())dword_4AF2FC)() - *(_DWORD *)(this + 27320)) > 0x3E8 )
      {
        if ( *(_DWORD *)(this + 29260) )
        {
          *(_DWORD *)(this + 29260) = 0;
        }
        else
        {
          sub_47BF30(dword_8FCDD8);
          *(_DWORD *)(this + 29364) = 1;
        }
      }
    }
    else if ( v5(66) >> 8 )
    {
      *(_DWORD *)(this + 27308) = *(_DWORD *)(this + 27308) == 0;
    }
    else if ( v5(71) >> 8 )
    {
      sub_43AB90((_DWORD *)this, 6);
    }
    else if ( v5(70) >> 8 )
    {
      sub_47CE80(dword_8FCDD8);
    }
    else if ( v5(78) >> 8 )
    {
      sub_43AB90((_DWORD *)this, 12);
    }
    else if ( v5(84) >> 8 )
    {
      if ( *(_DWORD *)(this + 29260) == 1 || *(_DWORD *)(this + 29264) == 1 )
      {
        v17 = *(_BYTE *)(this + 27992) + 1;
        *(_BYTE *)(this + 27992) = v17;
        if ( v17 > 1u )
          *(_BYTE *)(this + 27992) = 0;
        switch ( *(_BYTE *)(this + 27992) )
        {
          case 0:
            sub_45CC20((_DWORD *)(this + 27324), dword_8FCC64, 128, 128);
            break;
          case 1:
            sub_45CC20((_DWORD *)(this + 27324), dword_8FCC64, 256, 256);
            break;
          case 2:
            sub_45CC20((_DWORD *)(this + 27324), dword_8FCC64, 512, 512);
            break;
          case 3:
            sub_45CC20((_DWORD *)(this + 27324), dword_8FCC64, 1024, 1024);
            break;
          default:
            goto LABEL_105;
        }
      }
    }
    else if ( v5(89) >> 8 && *(_DWORD *)(this + 29260) == 1 )
    {
      *(_DWORD *)(this + 27988) = *(_DWORD *)(this + 27988) == 0;
    }
    goto LABEL_105;
  }
  if ( *(_DWORD *)(this + 29372) && sub_43AB90((_DWORD *)this, 18) == 1 )
    sub_47BF30(dword_8FCDD8);
  return 0;
}
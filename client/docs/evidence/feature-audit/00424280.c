char __userpurge sub_424280@<al>(int a1@<ecx>, int a2@<ebx>, int a3@<ebp>, int a4@<esi>, int a5, int a6)
{
  int v8; // ebx
  unsigned int v9; // edx
  unsigned int v10; // eax
  const char *v11; // eax
  int v12; // ecx
  int v13; // ebp
  int v14; // edi
  __int16 v15; // ax
  int v16; // edx
  int v17; // edx
  int v18; // edx
  int i; // ebp
  int **v20; // edi
  int j; // edi
  int v22; // ecx
  int v23; // eax
  int v24; // ecx
  int v25; // eax
  int v26; // eax
  int v27; // eax
  __int64 v28; // rax
  const char *v29; // eax
  char result; // al
  const char *v31; // [esp+90h] [ebp-124h]
  int v32; // [esp+94h] [ebp-120h]
  int v33; // [esp+94h] [ebp-120h]
  int v34; // [esp+98h] [ebp-11Ch]
  int v35; // [esp+98h] [ebp-11Ch]
  int v37; // [esp+9Ch] [ebp-118h]
  int v38; // [esp+A0h] [ebp-114h]
  int v39; // [esp+A4h] [ebp-110h]
  int v40; // [esp+B0h] [ebp-104h]
  int v41; // [esp+B4h] [ebp-100h] BYREF
  int v42; // [esp+B8h] [ebp-FCh]
  int v43; // [esp+BCh] [ebp-F8h]

  v34 = a3;
  v32 = a4;
  dword_903A84 = 0;
  v8 = MEMORY[0x49FE1C]((double)a5 * *(float *)(a1 + 4424784));
  if ( v8 < 10 )
    v8 = a6;
  dword_903A88 += v8;
  v9 = v8 + *(_DWORD *)((char *)&loc_437BDA + a1 + 2);
  *(_DWORD *)((char *)&loc_437BDA + a1 + 2) = v9;
  if ( v9 >= *(_DWORD *)((char *)&loc_437BD5 + a1 + 3) )
  {
    *(_DWORD *)((char *)&loc_437BDA + a1 + 2) = 0;
    *(_DWORD *)((char *)&loc_437BD5 + a1 + 3) = 0;
  }
  if ( (unsigned int)dword_903A88 > 0x5F )
  {
    v40 = 1;
    dword_903A88 = 0;
  }
  if ( *(_DWORD *)((char *)&loc_438275 + a1 + 3) )
  {
    v10 = v8 + *(_DWORD *)((char *)&loc_43827B + a1 + 1);
    *(_DWORD *)((char *)&loc_43827B + a1 + 1) = v10;
    if ( v10 > 0x1388 )
    {
      if ( !*(_DWORD *)(a1 + 76) )
      {
        v31 = (const char *)sub_480550(1411);
        v11 = (const char *)sub_480550(1412);
        MEMORY[0x49FA18](&v41, "%s %s", v11, v31);
        sub_420540(a1 + 3610584, (int)dword_5A041C, 111, 0, (const char *)&v41, 0, -1, -1, 0xFFFF, 0, 0, v32, a3);
      }
      sub_483560(dword_8FC5D8, a1 + 1010492, 1, 0);
      dword_4AF0CC(30);
      *(_DWORD *)((char *)&loc_438275 + a1 + 3) = 0;
      *(_DWORD *)((char *)&loc_43827B + a1 + 1) = 0;
    }
  }
  sub_423BC0(a1);
  sub_42E780((_DWORD *)a1);
  sub_42EC70(a1, v8, 0);
  sub_491A4E();
  sub_47F8D0(a1 + 1010300);
  if ( *(_DWORD *)(a1 + 3583700) )
  {
    sub_451AD0(*(_DWORD *)(a1 + 3180076), *(_DWORD *)(a1 + 3180080));
    sub_448BD0(a1 + 3583712);
    (*(void (__thiscall **)(int, int, int, int, int, int))(*(_DWORD *)(a1 + 3179944) + 28))(
      a1 + 3179944,
      v8,
      v40,
      v32,
      v34,
      a2);
    if ( *(_DWORD *)(a1 + 64) )
      sub_422EA0(a1);
    else
      sub_4229E0(a1);
    if ( *(_DWORD *)(a1 + 1010892) )
    {
      sub_48C4A4(
        *(_DWORD *)(a1 + 1010812) + *(_DWORD *)(a1 + 1010804) - *(_DWORD *)(a1 + 1010796),
        *(_DWORD *)(a1 + 1010816) + *(_DWORD *)(a1 + 1010808) - *(_DWORD *)(a1 + 1010800),
        *(_DWORD *)(a1 + 1010820) - *(_DWORD *)(a1 + 1010812),
        *(_DWORD *)(a1 + 1010824) - *(_DWORD *)(a1 + 1010816),
        a1 + 1010930,
        800,
        530);
    }
    else
    {
      sub_491932(0);
      sub_452C80((_DWORD *)(a1 + 1010488));
    }
    v12 = v43;
    *(_DWORD *)(a1 + 3622516) = 0;
    sub_421FF0((_DWORD *)a1, v8, v12);
    sub_424D40(a1, v8);
    v13 = *(_DWORD *)(a1 + 921984);
    if ( v13 )
    {
      do
      {
        v14 = *(_DWORD *)(v13 + 4);
        v43 = *(_DWORD *)(v13 + 12);
        if ( v14 )
        {
          v15 = *(_WORD *)(v14 + 16);
          if ( v15 == 150 )
          {
            (*(void (__thiscall **)(int, int))(*(_DWORD *)v14 + 12))(v14, v8);
            if ( *(_BYTE *)(v14 + 4) == 2 )
              goto LABEL_28;
          }
          else if ( v15 == 337 )
          {
            (*(void (__thiscall **)(int, int))(*(_DWORD *)v14 + 12))(v14, v8);
            if ( *(_BYTE *)(v14 + 4) == 2 )
              goto LABEL_28;
          }
          else
          {
            (*(void (__thiscall **)(int, int))(*(_DWORD *)v14 + 12))(v14, v8);
            if ( *(_BYTE *)(v14 + 4) == 2 )
            {
LABEL_28:
              (*(void (__thiscall **)(int, int))(*(_DWORD *)(a1 + 921980) + 20))(a1 + 921980, v13);
              (**(void (__thiscall ***)(int, int))v14)(v14, 1);
            }
          }
        }
        v13 = v43;
      }
      while ( v43 );
    }
    if ( *(_DWORD *)(a1 + 40) )
    {
      if ( *(_DWORD *)(a1 + 44) )
        sub_46B980();
      if ( *(_DWORD *)(a1 + 40) && *(_DWORD *)(a1 + 36) )
      {
        v16 = *(_DWORD *)(a1 + 3609436);
        v41 = 0;
        v42 = 0;
        (*(void (__thiscall **)(int, int, _DWORD, _DWORD, _DWORD))(v16 + 20))(a1 + 3609436, v8, 0, 0, 0);
        (*(void (__thiscall **)(int, int))(*(_DWORD *)(a1 + 3609436) + 16))(a1 + 3609436, dword_8FCC64);
      }
    }
    if ( *(_DWORD *)(a1 + 48) )
    {
      v17 = *(_DWORD *)(a1 + 3609820);
      v41 = 0;
      v42 = 0;
      (*(void (__thiscall **)(int, int, _DWORD, _DWORD, _DWORD))(v17 + 20))(a1 + 3609820, v8, 0, 0, 0);
      (*(void (__thiscall **)(int, int))(*(_DWORD *)(a1 + 3609820) + 16))(a1 + 3609820, dword_8FCC64);
    }
    v18 = *(_DWORD *)(a1 + 3609632);
    v41 = 0;
    v33 = 0;
    v42 = 0;
    v35 = 0;
    v37 = 0;
    (*(void (__thiscall **)(int, int))(v18 + 20))(a1 + 3609632, v8);
    (*(void (__thiscall **)(int, int))(*(_DWORD *)(a1 + 3609632) + 16))(a1 + 3609632, dword_8FCC64);
    (*(void (__thiscall **)(int, int, _DWORD, _DWORD, _DWORD))(*(_DWORD *)(a1 + 3610020) + 20))(
      a1 + 3610020,
      v8,
      0,
      0,
      0);
    (*(void (__thiscall **)(int, int))(*(_DWORD *)(a1 + 3610020) + 16))(a1 + 3610020, dword_8FCC64);
    (*(void (__thiscall **)(int, int, _DWORD, _DWORD, _DWORD))(*(_DWORD *)(a1 + 3610208) + 20))(
      a1 + 3610208,
      v8,
      0,
      0,
      0);
    (*(void (__thiscall **)(int, int))(*(_DWORD *)(a1 + 3610208) + 16))(a1 + 3610208, dword_8FCC64);
    v39 = 0;
    (*(void (__thiscall **)(int, int, _DWORD, _DWORD, _DWORD))(*(_DWORD *)(a1 + 3610396) + 20))(
      a1 + 3610396,
      v8,
      0,
      0,
      0);
    (*(void (__thiscall **)(int, int))(*(_DWORD *)(a1 + 3610396) + 16))(a1 + 3610396, dword_8FCC64);
    sub_40E1F0(a1 + 3179944);
    sub_40FA90(a1 + 3179944, v8);
    (*(void (__thiscall **)(int, int, int))(*(_DWORD *)(a1 + 3179944) + 128))(a1 + 3179944, -1, -1);
    (*(void (__thiscall **)(int))(*(_DWORD *)(a1 + 3179944) + 132))(a1 + 3179944);
    for ( i = *(_DWORD *)(a1 + 921960); i; i = *(_DWORD *)(i + 12) )
    {
      v20 = *(int ***)(i + 4);
      if ( v20 )
      {
        switch ( *((_BYTE *)v20 + 69) )
        {
          case 0:
          case 1:
            sub_40E1F0(*(_DWORD *)(i + 4));
            sub_40FA90((int)v20, v8);
            ((void (__thiscall *)(int **))(*v20)[33])(v20);
            if ( sub_4280E0((_DWORD *)a1, *v20[100110]) )
              goto LABEL_42;
            break;
          case 3:
            sub_40E1F0(*(_DWORD *)(i + 4));
            ((void (__thiscall *)(int **))(*v20)[33])(v20);
            if ( sub_4280E0((_DWORD *)a1, *v20[100110]) )
LABEL_42:
              ((void (__thiscall *)(int **, int, int))(*v20)[32])(v20, -1, -1);
            break;
          case 5:
            sub_40FA90((int)v20, v8);
            sub_430180((_DWORD *)a1, (int)v20);
            break;
          default:
            continue;
        }
      }
    }
    sub_423710((_DWORD *)a1);
    sub_4492D0((void *)(a1 + 3583712), COERCE_FLOAT(400), COERCE_FLOAT(300));
    for ( j = *(_DWORD *)(a1 + 921984); j; j = *(_DWORD *)(j + 12) )
    {
      v22 = *(_DWORD *)(j + 4);
      if ( v22 )
        (*(void (__thiscall **)(int))(*(_DWORD *)v22 + 16))(v22);
    }
    v23 = *(_DWORD *)(a1 + 3622516);
    if ( v23 )
    {
      v24 = *(_DWORD *)(a1 + 3622524);
      *(_DWORD *)(a1 + 3622520) = v23;
      if ( v24 != v23 )
      {
        *(_DWORD *)((char *)&loc_43826F + a1 + 1) = ((int (__cdecl *)(_DWORD, _DWORD, _DWORD))dword_4AF2FC)(0, 0, 0);
        v25 = *(_DWORD *)(a1 + 3622516);
        if ( !*(_BYTE *)(v25 + 403145) )
          sub_47BE80(dword_8FCDD8, **(_DWORD **)(v25 + 400440), *(_DWORD *)(v25 + 132), *(_DWORD *)(v25 + 136));
        if ( *(_DWORD *)(a1 + 80) )
        {
          v26 = *(_DWORD *)(a1 + 3622520);
          if ( v26 )
          {
            if ( *(_BYTE *)(v26 + 400664) == 0xFF )
              sub_47BEC0(**(_DWORD **)(v26 + 400440), *(_DWORD *)(v26 + 132), *(_DWORD *)(v26 + 136));
          }
        }
      }
      (*(void (__thiscall **)(_DWORD))(**(_DWORD **)(a1 + 3622516) + 136))(*(_DWORD *)(a1 + 3622516));
      *(_DWORD *)(a1 + 3622524) = *(_DWORD *)(a1 + 3622516);
    }
    else if ( (unsigned int)(((int (__cdecl *)(_DWORD, _DWORD, _DWORD))dword_4AF2FC)(0, 0, 0)
                           - *(_DWORD *)((char *)&loc_43826F + a1 + 1)) > 0x320 )
    {
      *(_DWORD *)(a1 + 3622520) = 0;
    }
    v27 = *(_DWORD *)((char *)&loc_43827F + a1 + 1);
    if ( v27 )
    {
      if ( *(_DWORD *)((char *)&loc_438281 + a1 + 3) )
      {
        if ( (int)abs32(v27 - *(_DWORD *)(a1 + 3180076)) > 5
          || (v28 = *(_DWORD *)((char *)&loc_438281 + a1 + 3) - *(_DWORD *)(a1 + 3180080),
              (int)((HIDWORD(v28) ^ v28) - HIDWORD(v28)) > 5) )
        {
          sub_424AD0(a1);
        }
      }
    }
    sub_424B90(v8);
    sub_428110(a1);
    if ( *(_DWORD *)((char *)&loc_43847B + a1 + 1) )
      sub_42F800((char *)a1);
    sub_438000((_DWORD *)(a1 + 2780620), v8, *(_DWORD *)(a1 + 3583684), *(_DWORD *)(a1 + 3583688));
    sub_423470(v33, v35, v37, v38, v39, 0, 0, 0, v41, v42);
    sub_424050(a1, v8);
  }
  else
  {
    dword_903A84 += v8;
    sub_491932(0);
    if ( !*(_DWORD *)(a1 + 84) && (unsigned int)dword_903A84 > 0x186A0 )
    {
      v29 = (const char *)sub_480550(1512);
      sub_420540(a1 + 3610584, (int)dword_5A041C, 100, 0, v29, 0, -1, -1, 0xFFFF, 0, 0, v32, v34);
    }
  }
  sub_420840(a1 + 3610584, *(_DWORD *)(a1 + 3583684), *(_DWORD *)(a1 + 3583688));
  (*(void (__thiscall **)(int, int))(*(_DWORD *)(a1 + 3614084) + 40))(a1 + 3614084, v8);
  result = sub_423D50((char *)a1, *(float *)&v8);
  if ( *(_DWORD *)((char *)&loc_4384BD + a1 + 7) )
    result = sub_42FEE0(a1);
  if ( !*(_DWORD *)((char *)&loc_4384C5 + a1 + 3) && *(_DWORD *)(a1 + 2809896) == 1 )
  {
    result = sub_47BF30(dword_8FCDD8);
    *(_DWORD *)(a1 + 2809988) = 1;
    *(_DWORD *)((char *)&loc_4384C5 + a1 + 3) = 1;
  }
  return result;
}
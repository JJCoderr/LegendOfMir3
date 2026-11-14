int __thiscall sub_4264D0(int this, int a2, unsigned int a3)
{
  int result; // eax
  int v5; // eax
  int v6; // ebx
  unsigned int v7; // edi
  const char *v8; // eax
  const char *v9; // eax
  int v10; // ebp
  int *v11; // ecx
  int v12; // edi
  int v13; // edx
  int v14; // eax
  int v15; // eax
  char v16; // al
  unsigned __int16 (__stdcall *v17)(int); // ebp
  int v18; // eax
  int v19; // ecx
  int v20; // ebx
  int *v21; // eax
  const char *v22; // eax
  char *v23; // eax
  int v24; // ecx
  int v25; // edi
  char v26; // al
  __int16 v27; // dx
  unsigned __int8 v28; // al
  int v29; // edi
  int v30; // eax
  int (__cdecl *v31)(int, int, int, int, int); // edi
  int v32; // eax
  int v33; // edx
  int v34; // [esp-10h] [ebp-30h]
  int v35; // [esp-Ch] [ebp-2Ch]
  int v36; // [esp-Ch] [ebp-2Ch]
  int v37; // [esp-Ch] [ebp-2Ch]
  int v38; // [esp+0h] [ebp-20h]
  int v39; // [esp+4h] [ebp-1Ch]
  int v40; // [esp+4h] [ebp-1Ch]
  int v41; // [esp+8h] [ebp-18h]
  int v42; // [esp+8h] [ebp-18h]
  int v43; // [esp+Ch] [ebp-14h]
  int v44; // [esp+10h] [ebp-10h]
  void *v45; // [esp+14h] [ebp-Ch]
  int v46; // [esp+14h] [ebp-Ch]
  _DWORD v47[2]; // [esp+18h] [ebp-8h] BYREF
  int v48; // [esp+28h] [ebp+8h]

  *(_DWORD *)(this + 3583684) = (unsigned __int16)a3;
  *(_DWORD *)(this + 3583688) = HIWORD(a3);
  if ( *(_DWORD *)(this + 3614112) )
  {
    (*(void (__thiscall **)(int, _DWORD, _DWORD))(*(_DWORD *)(this + 3614084) + 28))(
      this + 3614084,
      *(_DWORD *)(this + 3583684),
      *(_DWORD *)(this + 3583688));
    return 0;
  }
  v48 = this + 3610584;
  if ( sub_4209A0(this + 3610584, *(_DWORD *)(this + 3583684), *(_DWORD *)(this + 3583688)) )
    return 0;
  if ( !*(_DWORD *)(this + 3583700) )
    return 0;
  if ( *(_DWORD *)(this + 4422616) )
    return 0;
  if ( *(_DWORD *)(this + 4424344) )
    return 0;
  v5 = *(_DWORD *)(this + 3583688) - *(_DWORD *)(this + 1010808);
  v6 = *(_DWORD *)(this + 1010792);
  *(_DWORD *)(this + 3583692) = *(_DWORD *)(this + 1010788)
                              + (*(_DWORD *)(this + 3583684) - *(_DWORD *)(this + 1010804)) / 48;
  v47[0] = this + 2780620;
  v38 = *(_DWORD *)(this + 3583688);
  *(_DWORD *)(this + 3583696) = v6 + v5 / 32;
  if ( sub_43BAD0(this + 2780620, *(_DWORD *)(this + 3583684), v38)
    || dword_4AF2AC(this + 2783784, *(_DWORD *)(this + 3583684), *(_DWORD *)(this + 3583688)) )
  {
    return 0;
  }
  v45 = (void *)(this + 2780652);
  if ( *(_DWORD *)(this + 2780652) && !*(_DWORD *)(this + 2780656) && !*(_DWORD *)(this + 2780660) )
  {
    v7 = *(unsigned __int16 *)(this + 2780751);
    if ( !strcmp((const char *)(this + 2780680), (const char *)sub_480550(1050)) )
    {
      v8 = (const char *)sub_480550(1415);
      sub_420540(v48, (int)dword_5A041C, 102, 1, v8, 1, -1, -1, 0xFFFF, 0, 0, v39, v41);
      result = 0;
      memset(v45, 0, 0xC3Cu);
      return result;
    }
    if ( *(_BYTE *)(this + 2780741) < 2u || v7 <= 1 )
    {
      if ( *(_DWORD *)(this + 2780664) )
      {
        sub_41DB70(0, 0, v45);
        return 0;
      }
      if ( !*(_DWORD *)(this + 2780668) )
      {
        sub_47C0E0((char *)dword_8FCDD8, 1001, *(_DWORD *)(this + 2780742), this + 2780680);
        *(_DWORD *)(this + 2780656) = 1;
        return 0;
      }
    }
    else if ( !*(_DWORD *)(this + 2780668) )
    {
      v9 = (const char *)sub_480550(1416);
      sub_420540(v48, (int)dword_5A041C, 104, 1, v9, 1, -1, -1, 0xFFFF, 0, 0, v39, v41);
      return 0;
    }
    return 0;
  }
  v10 = 0;
  v11 = (int *)(1600
              * (*(_DWORD *)(this + 3583692)
               - *(_DWORD *)(this + 1010788)
               + 24 * (*(_DWORD *)(this + 3583696) - *(_DWORD *)(this + 1010792)))
              + this
              + 1156);
  do
  {
    v12 = *v11;
    if ( !*v11 )
      break;
    v13 = *(_DWORD *)(v12 + 4);
    if ( v13 == *(_DWORD *)(this + 3583692) )
    {
      v14 = *(_DWORD *)(v12 + 8);
      if ( v14 == *(_DWORD *)(this + 3583696)
        && v13 == *(_DWORD *)(this + 3180076)
        && v14 == *(_DWORD *)(this + 3180080) )
      {
        v20 = 0;
        v21 = dword_7680B8;
        do
        {
          if ( *v21 )
            ++v20;
          v21 += 780;
        }
        while ( (int)v21 < (int)dword_78B158 );
        v22 = (const char *)sub_480550(1050);
        if ( !MEMORY[0x4A0D80]((const char *)(v12 + 34), v22, 4) )
          goto LABEL_48;
        if ( v20 >= 46 )
        {
          if ( *(int *)(v12 + 14) < 2 )
          {
LABEL_47:
            v23 = (char *)sub_480550(1417);
            sub_41C120(v23, 11861940);
            return 0;
          }
LABEL_48:
          if ( v20 >= 46 && *(int *)(v12 + 14) >= 2 && !sub_4411A0(*(_WORD *)(v12 + 12)) )
            goto LABEL_47;
        }
        sub_47C1C0(*(_DWORD *)(v12 + 4), *(_DWORD *)(v12 + 8));
        return 0;
      }
    }
    ++v10;
    ++v11;
  }
  while ( v10 < 100 );
  v15 = *(_DWORD *)(this + 3622516);
  if ( !v15 || *(_DWORD *)(v15 + 400432) )
  {
    v33 = *(_DWORD *)(this + 3583684);
    v37 = *(_DWORD *)(this + 3583688);
    *(_DWORD *)(this + 3622528) = 0;
    sub_417E10((_DWORD *)(this + 3179944), v33, v37, -1, 0, 0);
    return 0;
  }
  *(_DWORD *)(this + 3622528) = v15;
  v47[0] = *(_DWORD *)(v15 + 132);
  v47[1] = *(_DWORD *)(v15 + 136);
  v16 = *(_BYTE *)(v15 + 69);
  if ( v16 == 5 || v16 == 4 )
  {
    v31 = dword_4AF2FC;
    if ( (unsigned int)(*(_DWORD *)(this + 4424308) - ((int (__cdecl *)())dword_4AF2FC)()) > 0x1F4 )
    {
      if ( !*(_DWORD *)(this + 3127528) && *(_DWORD *)(this + 3127564) == -1 )
      {
        v32 = *(_DWORD *)(this + 3622516);
        *(_DWORD *)(this + 3127564) = **(_DWORD **)(v32 + 400440);
        sub_47BF00(**(_DWORD **)(v32 + 400440));
      }
      *(_DWORD *)(this + 4424308) = v31(v40, v42, v43, v44, v46);
    }
    *(_DWORD *)(this + 3622528) = 0;
    return 0;
  }
  v17 = (unsigned __int16 (__stdcall *)(int))dword_4AF230;
  if ( (unsigned __int16)dword_4AF230(16) >> 8
    || (v18 = *(_DWORD *)(this + 3622516), *(_BYTE *)(v18 + 69) == 3)
    || (v19 = *(_DWORD *)(v18 + 400428), v19 == 255)
    || v19 == 27631
    || *(_DWORD *)(this + 4424908) )
  {
    v24 = *(_DWORD *)(this + 3622516);
    v25 = 0;
    v26 = *(_BYTE *)(v24 + 71);
    if ( v26 == 26 || v26 == 86 || v26 == 113 || v26 == 114 || v26 == 115 || v26 == -91 || v26 == -90 )
      v25 = 1;
    v27 = *(_WORD *)(v24 + 71);
    if ( (v27 == 530 || v27 == 531 || v27 == 532 || v27 == 533) && *(_BYTE *)(v24 + 121) == 7 || v25 )
    {
      v30 = *(_DWORD *)(this + 3583684);
      v36 = *(_DWORD *)(this + 3583688);
      *(_DWORD *)(this + 3622528) = 0;
      sub_417E10((_DWORD *)(this + 3179944), v30, v36, -1, 0, 0);
      return 0;
    }
    v28 = *(_BYTE *)(v24 + 69);
    v29 = 0;
    if ( v28 < 2u )
    {
      if ( *(_BYTE *)(v24 + 120) != 19 && !*(_DWORD *)(v24 + 400432) )
        goto LABEL_76;
    }
    else
    {
      if ( v28 != 3 )
      {
LABEL_76:
        sub_417E10(
          (_DWORD *)(this + 3179944),
          *(_DWORD *)(this + 3583684),
          *(_DWORD *)(this + 3583688),
          **(_DWORD **)(*(_DWORD *)(this + 3622516) + 400440),
          v29,
          v47);
        return 0;
      }
      if ( *(_BYTE *)(v24 + 120) != 4 && !*(_DWORD *)(v24 + 400432) )
      {
        if ( v17(16) >> 8 )
          *(_DWORD *)(this + 3622528) = 0;
        goto LABEL_76;
      }
    }
    v29 = 1;
    goto LABEL_76;
  }
  if ( v18 != this + 3179944 )
    return 0;
  v35 = *(_DWORD *)(this + 3583688);
  v34 = *(_DWORD *)(this + 3583684);
  *(_DWORD *)(this + 3622528) = 0;
  sub_417E10((_DWORD *)(this + 3179944), v34, v35, -1, 0, 0);
  return 0;
}
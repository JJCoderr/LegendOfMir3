int __thiscall sub_4256B0(int this, int a2, const char *a3)
{
  const char *v4; // eax
  int v5; // eax
  int v6; // edx
  unsigned int v7; // edi
  int v8; // ebp
  int v9; // eax
  int v10; // eax
  unsigned int v11; // edi
  int v12; // eax
  bool v13; // zf
  int v14; // eax
  unsigned int v15; // edi
  int v16; // ebp
  int v17; // eax
  int v18; // ebp
  unsigned int v19; // edi
  int v20; // eax
  unsigned int v21; // edi
  int v22; // esi
  unsigned int v23; // eax
  unsigned int v24; // eax
  unsigned int v25; // edi
  int v26; // eax
  int v28[783]; // [esp-C2Ch] [ebp-D40h] BYREF
  _BYTE v29[260]; // [esp+10h] [ebp-104h] BYREF

  switch ( HIBYTE(a2) )
  {
    case 'd':
      if ( BYTE2(a2) == 1 && sub_421F60(this) )
        goto LABEL_10;
      goto LABEL_82;
    case 'e':
      if ( !BYTE2(a2) && sub_421F60(this) )
      {
        sub_47BE10(1009);
        sub_43EC60((char *)(this + 2780620), this + 3583089);
        *(_DWORD *)(this + 4424764) = 0;
        *(_BYTE *)(this + 4424760) = 2;
      }
      goto LABEL_82;
    case 'f':
      if ( !BYTE2(a2) )
      {
        v5 = MEMORY[0x49FAF5](a3);
        if ( v5 < *(_DWORD *)(this + 3583508) )
          sub_47C150(v5);
      }
      goto LABEL_82;
    case 'g':
      if ( (_WORD)a2 == 1038 && !BYTE2(a2) )
        sub_47CEE0(a3);
      goto LABEL_82;
    case 'h':
      if ( !BYTE2(a2) )
      {
        v11 = *(unsigned __int16 *)(this + 2780751);
        if ( !strcmp(a3, dword_902E34) )
          v12 = *(unsigned __int16 *)(this + 2780751);
        else
          v12 = MEMORY[0x49FAF5](a3);
        if ( v12 >= 100 )
          v12 = 100;
        v13 = v12 == v11;
        if ( v12 < v11 )
        {
          if ( v12 > 0 )
          {
            sub_47C180(*(_DWORD *)(this + 2780742), v12, this + 2780680);
            goto LABEL_34;
          }
          v13 = v12 == v11;
        }
        if ( v13 )
          sub_47C180(*(_DWORD *)(this + 2780742), v12, this + 2780680);
      }
      goto LABEL_82;
    case 'i':
      if ( BYTE2(a2) )
        goto LABEL_82;
      v6 = 3120 * *(_DWORD *)(this + 2955497);
      v7 = *(unsigned __int16 *)(v6 + this + 2812055);
      v8 = v6 + this;
      if ( !strcmp(a3, dword_902E34) )
      {
        v9 = v7;
      }
      else
      {
        v9 = MEMORY[0x49FAF5](a3);
        if ( v9 > v7 )
          goto LABEL_82;
      }
      if ( v9 > 0 )
        sub_47BF60(*(_DWORD *)(v8 + 2812046), *(_DWORD *)(this + 2955504), v8 + 2811984, v9);
      goto LABEL_82;
    case 'j':
      if ( BYTE2(a2) )
        goto LABEL_82;
      v14 = 3120 * *(_DWORD *)(this + 2955497);
      v15 = *(unsigned __int16 *)(v14 + this + 2812055);
      v16 = v14 + this;
      if ( !strcmp(a3, dword_902E34) )
      {
        v17 = v15;
      }
      else
      {
        v17 = MEMORY[0x49FAF5](a3);
        if ( v17 > v15 )
          goto LABEL_82;
      }
      if ( v17 > 0 )
        sub_47C060(*(_DWORD *)(v16 + 2812046), *(_DWORD *)(this + 2955504), v16 + 2811984, v17);
      goto LABEL_82;
    case 'k':
      if ( BYTE2(a2) )
        goto LABEL_82;
      v18 = sub_478670((_DWORD *)(this + 2997392), *(_DWORD *)(this + 2999424));
      v19 = *(unsigned __int16 *)(v18 + 75);
      if ( !strcmp(a3, dword_902E34) )
      {
        v20 = *(unsigned __int16 *)(v18 + 75);
      }
      else
      {
        v20 = MEMORY[0x49FAF5](a3);
        if ( v20 > v19 )
          goto LABEL_82;
      }
      if ( v20 > 0 )
        sub_47CD30(*(_DWORD *)(this + 2999420), v18 + 4, *(_DWORD *)(v18 + 66), v20);
      goto LABEL_82;
    case 'l':
      if ( BYTE2(a2) )
        goto LABEL_82;
      v21 = *(unsigned __int16 *)(this + 2780751);
      if ( !strcmp(a3, dword_902E34) )
      {
        v22 = *(unsigned __int16 *)(this + 2780751);
      }
      else
      {
        v22 = MEMORY[0x49FAF5](a3);
        if ( v22 > v21 )
          goto LABEL_82;
      }
      if ( v22 > 0 )
      {
        sub_41DBD0(*(_DWORD *)(this + 3078756), *(_DWORD *)(this + 3078752));
        sub_47C310(this + 3075492, *(_DWORD *)(this + 3075554), v22);
        *(_DWORD *)(*(_DWORD *)(this + 3078756) + 4) = 1;
      }
      goto LABEL_82;
    case 'm':
      if ( !BYTE2(a2) && sub_421F60(this) )
      {
        v4 = (const char *)sub_480550(1077);
        MEMORY[0x49FA18](v29, "%s", v4);
        dword_4AF21C(0, 0, v29, 0, 0, 1);
        dword_4AF2B4(dword_8FCC58, 2, 0, 0);
      }
      goto LABEL_82;
    case 'n':
      if ( BYTE2(a2) )
        goto LABEL_82;
      if ( !strcmp(a3, dword_902E34) )
      {
        LOWORD(v10) = 1;
      }
      else
      {
        v10 = MEMORY[0x49FAF5](a3);
        if ( v10 <= 0 )
          goto LABEL_82;
      }
      sub_47CCB0(
        dword_8FCDD8,
        *(_DWORD *)(this + 2999420),
        *(const char **)(this + 2999416),
        *(_DWORD *)(*(_DWORD *)(this + 2999416) + 40),
        v10);
      goto LABEL_82;
    case 'o':
LABEL_10:
      dword_4AF2B4(dword_8FCC58, 2, 0, 0);
      goto LABEL_82;
    case 'p':
      if ( BYTE2(a2) )
        goto LABEL_82;
      v25 = *(unsigned __int16 *)(this + 2780751);
      if ( !strcmp(a3, dword_902E34) )
      {
        v26 = *(unsigned __int16 *)(this + 2780751);
      }
      else
      {
        v26 = MEMORY[0x49FAF5](a3);
        if ( v26 > v25 )
          goto LABEL_82;
      }
      if ( v26 > 0 )
      {
        sub_47C390(this + 2780680, v26, *(_DWORD *)(this + 2780742));
LABEL_34:
        v28[778] = 0;
        v28[777] = 0;
        qmemcpy(v28, (const void *)(this + 2780676), 0xC24u);
        sub_440CB0(
          (_WORD *)(this + 2810060),
          v28[0],
          v28[1],
          v28[2],
          v28[3],
          v28[4],
          v28[5],
          v28[6],
          v28[7],
          v28[8],
          v28[9],
          v28[10],
          v28[11],
          v28[12],
          v28[13],
          v28[14],
          v28[15],
          v28[16],
          v28[17],
          v28[18],
          v28[19],
          v28[20],
          v28[21],
          v28[22],
          v28[23],
          v28[24],
          v28[25],
          v28[26],
          v28[27],
          v28[28],
          v28[29],
          v28[30],
          v28[31],
          v28[32],
          v28[33],
          v28[34],
          v28[35],
          v28[36],
          v28[37],
          v28[38],
          v28[39],
          v28[40],
          v28[41],
          v28[42],
          v28[43],
          v28[44],
          v28[45],
          v28[46],
          v28[47],
          v28[48],
          v28[49],
          v28[50],
          v28[51],
          v28[52],
          v28[53],
          v28[54],
          v28[55],
          v28[56],
          v28[57],
          v28[58],
          v28[59],
          v28[60],
          v28[61]);
        memset((void *)(this + 2780652), 0, 0xC3Cu);
      }
LABEL_82:
      sub_43E5E0(a2, (int)a3);
      return 0;
    case 'r':
      if ( BYTE2(a2) )
        goto LABEL_60;
      v23 = MEMORY[0x49FAF5](a3);
      if ( v23 <= 0x5F5E100 )
      {
        if ( v23 )
          *(_DWORD *)(this + 3167632) = v23;
LABEL_60:
        *(_DWORD *)(this + 3167660) = 0;
      }
      goto LABEL_82;
    case 's':
      if ( !BYTE2(a2) )
        sub_456290(this + 3158708);
      goto LABEL_82;
    case 't':
      if ( !BYTE2(a2) )
        sub_4562F0(this + 3158708);
      goto LABEL_82;
    case 'u':
      if ( !BYTE2(a2) )
        sub_457370(this + 3163868);
      goto LABEL_82;
    case 'v':
      if ( BYTE2(a2) )
      {
        *(_DWORD *)(this + 3178032) = 0;
      }
      else
      {
        v24 = MEMORY[0x49FAF5](a3);
        if ( v24 <= 0x5F5E100 )
        {
          if ( v24 )
            *(_DWORD *)(this + 3178028) = v24;
          *(_DWORD *)(this + 3178032) = 1;
        }
      }
      goto LABEL_82;
    default:
      goto LABEL_82;
  }
}
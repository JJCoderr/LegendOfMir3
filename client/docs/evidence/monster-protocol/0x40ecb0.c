int __thiscall sub_40ECB0(int this, _DWORD *a2, int a3, int a4, int a5, int a6)
{
  int v7; // edx
  char v8; // al
  int v9; // eax
  __int16 *v10; // eax
  int v11; // eax
  int v12; // edi
  int v13; // ecx
  int v14; // edi
  int v15; // eax
  int v16; // ecx
  int v17; // edi
  int v18; // edx
  int v19; // ebp
  int v20; // ebx
  unsigned __int16 v21; // ax
  int v22; // edi
  bool v23; // zf
  int v24; // eax
  __int16 *v25; // eax
  int v26; // edx
  int v27; // ecx
  int v28; // edx
  char v29; // cl
  int v30; // edx
  int v32; // [esp-20h] [ebp-40h]
  int v33; // [esp-1Ch] [ebp-3Ch]
  int v34; // [esp-18h] [ebp-38h]
  int v35; // [esp-14h] [ebp-34h]
  int v36; // [esp-10h] [ebp-30h]
  __int16 v37; // [esp+14h] [ebp-Ch]
  unsigned __int16 v38; // [esp+14h] [ebp-Ch]
  unsigned __int16 v39; // [esp+18h] [ebp-8h]
  char v40; // [esp+1Ch] [ebp-4h]
  int v41; // [esp+24h] [ebp+4h]
  __int16 v42; // [esp+30h] [ebp+10h]
  char v43; // [esp+30h] [ebp+10h]

  if ( a5 )
  {
    v7 = *(_DWORD *)(this + 136);
    *(_DWORD *)(this + 156) = *(_DWORD *)(this + 140) + a2[79] + 48 * (*(_DWORD *)(this + 132) - a2[75]) - a2[77];
    *(_DWORD *)(this + 160) = *(_DWORD *)(this + 144) + a2[80] + 32 * (v7 - a2[76]) - a2[78];
  }
  if ( !*(_DWORD *)(this + 76) )
    return 0;
  v8 = *(_BYTE *)(this + 400205);
  if ( *(_DWORD *)(this + 400392) )
  {
    if ( v8 )
    {
      v9 = *(_DWORD *)(this + 124);
      goto LABEL_10;
    }
  }
  else if ( !v8 )
  {
    v9 = *(_DWORD *)(this + 124);
    goto LABEL_10;
  }
  v9 = *(_DWORD *)(this + 116) - *(_DWORD *)(this + 124) + *(_DWORD *)(this + 112) - 1;
LABEL_10:
  v41 = v9;
  if ( !sub_491C70(v9) )
    return 0;
  v10 = *(__int16 **)(*(_DWORD *)(this + 76) + 60);
  v40 = *((_BYTE *)v10 + 8);
  v42 = *(__int16 *)((char *)v10 + 9);
  v37 = *(__int16 *)((char *)v10 + 11);
  dword_4AF2A8(
    this + 80,
    *(_DWORD *)(this + 156) + v10[2],
    *(_DWORD *)(this + 160) + v10[3],
    *(_DWORD *)(this + 156) + v10[2] + *v10,
    *(_DWORD *)(this + 160) + v10[3] + v10[1]);
  v11 = *(_DWORD *)(this + 88);
  v12 = *(_DWORD *)(this + 80);
  v13 = v11 - v12;
  if ( v11 - v12 <= 58 )
  {
    *(_DWORD *)(this + 96) = v12;
  }
  else
  {
    *(_DWORD *)(this + 96) = v12 + (v13 - 58) / 2;
    v11 = v12 + (v13 + 58) / 2;
  }
  v14 = *(_DWORD *)(this + 84);
  *(_DWORD *)(this + 104) = v11;
  v15 = *(_DWORD *)(this + 92);
  v16 = v15 - v14;
  if ( v15 - v14 <= 40 )
  {
    *(_DWORD *)(this + 100) = v14;
  }
  else
  {
    *(_DWORD *)(this + 100) = v14 + (v16 - 40) / 2;
    v15 = v14 + (v16 + 40) / 2;
  }
  v17 = *(_DWORD *)(this + 156);
  v18 = *(_DWORD *)(this + 160);
  *(_DWORD *)(this + 108) = v15;
  v19 = v17 + v42;
  v20 = v18 + v37;
  v43 = 13;
  v38 = sub_405C40(this);
  v21 = *(_WORD *)(this + 71);
  v39 = v21;
  if ( a6 && v21 != 530 && v21 != 531 && v21 != 532 && v21 != 533 )
  {
    if ( *(_BYTE *)(this + 69) == 3 && v21 >= 0x7D0u )
      v22 = 328 * *(unsigned __int8 *)(this + 73) + 5876660;
    else
      v22 = 328 * *(unsigned __int8 *)(this + 73) + 5882892;
    v23 = sub_491C70(v41) == 0;
    v24 = *(_DWORD *)(this + 400380);
    if ( v23 )
    {
      if ( v24 || v38 == 1 || (v26 = a4) != 0 )
        v26 = 1;
      if ( !*(_DWORD *)(this + 400432) || v39 == 127 || v39 == 153 )
        sub_48F4AB(
          v19,
          v20,
          **(__int16 **)(*(_DWORD *)(this + 76) + 60),
          *(__int16 *)(*(_DWORD *)(*(_DWORD *)(this + 76) + 60) + 2),
          *(_DWORD *)(*(_DWORD *)(this + 76) + 64),
          800,
          530,
          dword_8F1468,
          v26,
          v40,
          13);
      else
        sub_48F4AB(
          *(_DWORD *)(this + 80) + 3,
          *(_DWORD *)(this + 84) + 2,
          **(__int16 **)(*(_DWORD *)(this + 76) + 60),
          *(__int16 *)(*(_DWORD *)(*(_DWORD *)(this + 76) + 60) + 2),
          *(_DWORD *)(*(_DWORD *)(this + 76) + 64),
          800,
          530,
          dword_8F1468,
          v26,
          50,
          13);
    }
    else if ( v24 || v38 == 1 || a4 )
    {
      sub_48EF15(
        (int)&dword_8FCC50,
        *(_DWORD *)(this + 156) + *(__int16 *)(*(_DWORD *)(v22 + 60) + 4),
        *(_DWORD *)(this + 160) + *(__int16 *)(*(_DWORD *)(v22 + 60) + 6),
        **(__int16 **)(v22 + 60),
        *(__int16 *)(*(_DWORD *)(v22 + 60) + 2),
        *(unsigned __int16 **)(v22 + 64),
        800,
        530,
        0xFFFFu,
        0xFFFFu,
        13);
    }
    else
    {
      v25 = *(__int16 **)(v22 + 60);
      v36 = *(_DWORD *)(v22 + 64);
      v35 = v25[1];
      v34 = *v25;
      v33 = *(_DWORD *)(this + 160) + v25[3];
      v32 = *(_DWORD *)(this + 156) + v25[2];
      if ( a3 )
        sub_48FA0F(v32, v33, v34, v35, v36, 800, 530, 0xFFFF, 0xFFFF);
      else
        sub_48D9F7(v32, v33, v34, v35, v36, 800, 530, 0xFFFF, 0xFFFF);
    }
  }
  v27 = *(_DWORD *)(this + 400380);
  if ( v27 <= 0 )
    goto LABEL_47;
  v28 = *(_DWORD *)(this + 400384);
  if ( !*(_DWORD *)(this + 400388) )
  {
    v43 = 31 * v28 / v27;
LABEL_47:
    v29 = v43;
    goto LABEL_48;
  }
  v29 = 31 - 31 * v28 / v27;
  v43 = v29;
LABEL_48:
  HIWORD(v30) = HIWORD(a3);
  if ( a3 )
  {
    if ( *(_DWORD *)(this + 400388) )
      v29 -= 5;
    else
      v29 += 5;
    v43 = v29;
  }
  if ( (unsigned __int8)v29 > 0x1Fu )
    v43 = 0;
  LOWORD(v30) = *(_WORD *)(this + 403020);
  sub_405CB0(
    (_DWORD *)this,
    *(_DWORD *)(this + 80),
    *(_DWORD *)(this + 84),
    **(__int16 **)(*(_DWORD *)(this + 76) + 60),
    *(__int16 *)(*(_DWORD *)(*(_DWORD *)(this + 76) + 60) + 2),
    *(unsigned __int16 **)(*(_DWORD *)(this + 76) + 64),
    v30,
    0xFFFF,
    a3,
    v43,
    v38);
  return 1;
}
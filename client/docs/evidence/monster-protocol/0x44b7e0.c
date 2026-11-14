int __thiscall sub_44B7E0(unsigned __int16 *this, int a2)
{
  int v2; // ebx
  unsigned int v4; // esi
  int result; // eax
  unsigned __int16 v6; // ax
  unsigned __int16 *v7; // edi
  int v8; // esi
  int v9; // eax
  unsigned int v10; // ecx
  int v11; // eax
  unsigned int v12; // eax
  _DWORD *v13; // eax
  int v14; // edx
  int v15; // eax
  int v16; // ebx
  int v17; // ecx
  signed int v18; // edi
  int v19; // edx
  int v20; // eax
  signed int v21; // esi
  int v22; // edx
  int v23; // edx
  double v24; // st7
  int v25; // esi
  int v26; // eax
  int v27; // ecx
  int v28; // eax
  double v29; // st7
  __int16 v30; // ax
  unsigned __int16 v31; // ax
  int v32; // ecx
  __int64 v33; // rax
  int v34; // eax
  _DWORD *v35; // eax
  int v36; // ecx
  unsigned __int16 *v37; // ebx
  int v38; // ecx
  int v39; // ecx
  int v40; // eax
  int v41; // ecx
  int v42; // ecx
  int v43; // ecx
  int v44; // eax
  int v45; // eax
  int v46; // edx
  unsigned __int16 v47; // ax
  int v48; // ecx
  unsigned __int16 v49; // ax
  int v50; // eax
  int v51; // edx
  int v52; // eax
  int v53; // esi
  __int16 v54; // [esp-4h] [ebp-34h]
  _DWORD v55[8]; // [esp+10h] [ebp-20h] BYREF
  unsigned __int16 *v56; // [esp+34h] [ebp+4h]

  v2 = a2;
  *((_DWORD *)this + 54) += a2;
  v4 = a2 + *((_DWORD *)this + 57);
  *((_DWORD *)this + 57) = v4;
  if ( v4 < this[148] )
  {
    *((_DWORD *)this + 54) = 0;
    sub_453B60(&dword_5B03B0, *((_DWORD *)this + 48), *((_DWORD *)this + 49), (_DWORD *)this + 46, (int *)this + 47);
    sub_453B60(&dword_5B03B0, *((_DWORD *)this + 52), *((_DWORD *)this + 53), (_DWORD *)this + 50, (int *)this + 51);
    *((_DWORD *)this + 40) = *((_DWORD *)this + 46);
    *((_DWORD *)this + 41) = *((_DWORD *)this + 47);
    result = 0;
    *((float *)this + 42) = (float)*((int *)this + 46);
    *((float *)this + 43) = (float)*((int *)this + 47);
    return result;
  }
  if ( v4 > 0xEA60 )
  {
    *((_BYTE *)this + 4) = 2;
    return 0;
  }
  if ( *((_DWORD *)this + 54) > *((_DWORD *)this + 56) )
  {
    ++*((_DWORD *)this + 63);
    *((_DWORD *)this + 54) = 0;
    v6 = this[8];
    if ( (v6 == 420 || v6 == 421) && *((_DWORD *)this + 63) == *((_DWORD *)this + 62) - 1 )
    {
      v7 = this + 158;
      if ( this != (unsigned __int16 *)-316 )
      {
        v8 = sub_401430(3);
        v9 = sub_401430(2);
        sub_4366D0(dword_760644, v8, v9, (const char *)this + 316);
        memset(v7, 0, 0x104u);
      }
    }
    if ( this[8] == 30 )
    {
      v10 = *((_DWORD *)this + 63);
      if ( v10 < 0x93A || v10 > 0x945 )
        *((_DWORD *)this + 56) = 110;
      else
        *((_DWORD *)this + 56) = 30;
    }
    v11 = *((_DWORD *)this + 76);
    if ( v11 < 5 && !*((_DWORD *)this + 3) )
      sub_483450(10 * this[8] + 10001, v11, *((_DWORD *)this + 44), *((_DWORD *)this + 45));
    v12 = *((_DWORD *)this + 62);
    if ( *((_DWORD *)this + 63) >= v12 )
    {
      if ( *((_DWORD *)this + 2) )
      {
        *((_DWORD *)this + 63) = *((_DWORD *)this + 61);
      }
      else
      {
        *((_DWORD *)this + 63) = v12 - 1;
        *((_BYTE *)this + 4) = 1;
      }
    }
  }
  v13 = (_DWORD *)*((_DWORD *)this + 6);
  if ( *((_DWORD *)this + 3) )
  {
    if ( v13 )
    {
      v41 = v13[33];
      *((_DWORD *)this + 52) = v41;
      *((_DWORD *)this + 44) = v41;
      v42 = v13[34];
      *((_DWORD *)this + 53) = v42;
      *((_DWORD *)this + 45) = v42;
      v43 = v13[39];
      *((_DWORD *)this + 50) = v43;
      *((_DWORD *)this + 40) = v43;
      v44 = v13[40];
      *((_DWORD *)this + 51) = v44;
    }
    else
    {
      *((_DWORD *)this + 44) = *((_DWORD *)this + 52);
      *((_DWORD *)this + 45) = *((_DWORD *)this + 53);
      sub_453B60(&dword_5B03B0, *((_DWORD *)this + 52), *((_DWORD *)this + 53), (_DWORD *)this + 50, (int *)this + 51);
      v44 = *((_DWORD *)this + 51);
      *((_DWORD *)this + 40) = *((_DWORD *)this + 50);
    }
    *((_DWORD *)this + 41) = v44;
    goto LABEL_68;
  }
  if ( v13 )
  {
    *((_DWORD *)this + 52) = v13[33];
    *((_DWORD *)this + 53) = v13[34];
    *((_DWORD *)this + 50) = v13[39];
    v14 = v13[40];
    v15 = *((_DWORD *)this + 48);
    *((_DWORD *)this + 51) = v14;
    *((_DWORD *)this + 44) = v15;
    *((_DWORD *)this + 45) = *((_DWORD *)this + 49);
    v16 = *((_DWORD *)this + 40);
    v17 = *((_DWORD *)this + 50);
    if ( v16 == v17 )
      v18 = 1;
    else
      v18 = abs32(v17 - v16);
    v19 = *((_DWORD *)this + 41);
    v20 = *((_DWORD *)this + 51);
    if ( v19 == v20 )
      v21 = 1;
    else
      v21 = abs32(v20 - v19);
    if ( !v18 )
      v18 = 1;
    if ( !v21 )
      v21 = 1;
    if ( v18 <= v21 )
    {
      v23 = *((_DWORD *)this + 41);
      *((_DWORD *)this + 68) = 500 / v21 * (v17 - v16);
      *((_DWORD *)this + 69) = 500 / v21 * (*((_DWORD *)this + 51) - v23);
    }
    else
    {
      v22 = *((_DWORD *)this + 51);
      *((_DWORD *)this + 68) = 500 / v18 * (v17 - v16);
      *((_DWORD *)this + 69) = 500 / v18 * (v22 - *((_DWORD *)this + 41));
    }
    v24 = (double)a2;
    *((float *)this + 42) = (double)*((int *)this + 68) * flt_4AF950 * v24 + *((float *)this + 42);
    *((float *)this + 43) = (double)*((int *)this + 69) * flt_4AF950 * v24 + *((float *)this + 43);
    *((_DWORD *)this + 40) = MEMORY[0x49FE1C](*((float *)this + 42));
    v25 = MEMORY[0x49FE1C](*((float *)this + 43));
    v26 = *((_DWORD *)this + 50);
    *((_DWORD *)this + 41) = v25;
    v27 = abs32(v26 - *((_DWORD *)this + 40));
    v28 = abs32(*((_DWORD *)this + 51) - v25);
    if ( (v27 > 15 || v28 > 15) && (v27 < *((_DWORD *)this + 70) || v28 < *((_DWORD *)this + 71)) )
    {
      v2 = a2;
      *((_DWORD *)this + 70) = v27;
      *((_DWORD *)this + 71) = v28;
LABEL_68:
      (*(void (__thiscall **)(unsigned __int16 *, int *, int))(*(_DWORD *)this + 20))(this, dword_824758, v2);
      return 1;
    }
  }
  else
  {
    v29 = (double)a2;
    *((float *)this + 42) = (double)*((int *)this + 68) * flt_4AF950 * v29 + *((float *)this + 42);
    *((float *)this + 43) = (double)*((int *)this + 69) * flt_4AF950 * v29 + *((float *)this + 43);
    *((_DWORD *)this + 40) = (__int16)MEMORY[0x49FE1C](*((float *)this + 42));
    v30 = MEMORY[0x49FE1C](*((float *)this + 43));
    *((_DWORD *)this + 41) = v30;
    sub_453BB0(&dword_5B03B0, *((_DWORD *)this + 40), v30, (_DWORD *)this + 44, (int *)this + 45);
    v31 = this[8];
    if ( v31 == 13
      || v31 == 5
      || v31 == 1
      || v31 == 37
      || v31 == 39
      || v31 == 40
      || v31 == 41
      || v31 == 67
      || v31 == 416
      || v31 == 417
      || v31 == 418
      || v31 == 419
      || v31 == 113 )
    {
      if ( *((_DWORD *)this + 57) < 0xBB8u )
        goto LABEL_68;
      *((_BYTE *)this + 4) = 2;
    }
    else
    {
      v32 = abs32(*((_DWORD *)this + 50) - *((_DWORD *)this + 40));
      v33 = *((_DWORD *)this + 51) - *((_DWORD *)this + 41);
      v34 = (HIDWORD(v33) ^ v33) - HIDWORD(v33);
      if ( (v32 > 15 || v34 > 15) && (v32 < *((_DWORD *)this + 70) || v34 < *((_DWORD *)this + 71)) )
      {
        *((_DWORD *)this + 70) = v32;
        *((_DWORD *)this + 71) = v34;
        goto LABEL_68;
      }
    }
  }
  v35 = (_DWORD *)*((_DWORD *)this + 6);
  if ( v35 )
  {
    v36 = v35[33];
    *((_DWORD *)this + 52) = v36;
    v37 = this + 90;
    *((_DWORD *)this + 44) = v36;
    v38 = v35[34];
    *((_DWORD *)this + 53) = v38;
    *((_DWORD *)this + 45) = v38;
    v39 = v35[39];
    v56 = this + 88;
    *((_DWORD *)this + 50) = v39;
    *((_DWORD *)this + 40) = v39;
    v40 = v35[40];
    *((_DWORD *)this + 51) = v40;
  }
  else
  {
    v37 = this + 90;
    *((_DWORD *)this + 44) = *((_DWORD *)this + 52);
    v56 = this + 88;
    *((_DWORD *)this + 45) = *((_DWORD *)this + 53);
    sub_453B60(&dword_5B03B0, *((_DWORD *)this + 52), *((_DWORD *)this + 53), (_DWORD *)this + 50, (int *)this + 51);
    v40 = *((_DWORD *)this + 51);
    *((_DWORD *)this + 40) = *((_DWORD *)this + 50);
  }
  *((_DWORD *)this + 41) = v40;
  *((_DWORD *)this + 3) = 1;
  v54 = this[8];
  *((_DWORD *)this + 2) = 0;
  v45 = sub_46EF20(dword_8FBA50, v54);
  if ( v45 )
  {
    v46 = *(_DWORD *)v45;
    *((_DWORD *)this + 61) = *(_DWORD *)v45;
    *((_DWORD *)this + 62) = *(_DWORD *)(v45 + 4);
    *((_DWORD *)this + 63) = v46;
    *((_DWORD *)this + 56) = *(unsigned __int16 *)(v45 + 8);
    *((_BYTE *)this + 232) = *(_BYTE *)(v45 + 14);
    *((_BYTE *)this + 233) = *(_BYTE *)(v45 + 15);
    *((_BYTE *)this + 234) = *(_BYTE *)(v45 + 16);
    *((_BYTE *)this + 235) = *(_BYTE *)(v45 + 17);
    *((_BYTE *)this + 236) = *(_BYTE *)(v45 + 18);
    *((_BYTE *)this + 237) = *(_BYTE *)(v45 + 19);
    *((_BYTE *)this + 238) = *(_BYTE *)(v45 + 20);
    *((_BYTE *)this + 239) = *(_BYTE *)(v45 + 21);
    *((_BYTE *)this + 240) = *(_BYTE *)(v45 + 22);
    *((_BYTE *)this + 241) = *(_BYTE *)(v45 + 23);
    *((_BYTE *)this + 242) = *(_BYTE *)(v45 + 24);
    *((_BYTE *)this + 256) = *(_BYTE *)(v45 + 25);
    *((_BYTE *)this + 257) = *(_BYTE *)(v45 + 26);
    *((_BYTE *)this + 258) = *(_BYTE *)(v45 + 27);
    v47 = *(_WORD *)(v45 + 10);
    this[150] = v47;
    v48 = 328 * v47 + 5876332;
    v49 = this[150];
    *((_DWORD *)this + 39) = v48;
    if ( v49 == 81 || v49 == 137 || v49 == 149 )
    {
      this[149] = 0;
    }
    else if ( v49 == 129 || v49 == 138 || v49 == 140 || v49 == 143 )
    {
      this[149] = 1;
    }
    v55[0] = -1090519040;
    v55[1] = 1056964608;
    memset(&v55[2], 0, 12);
    v55[5] = -1082130432;
    v55[6] = 0;
    v55[7] = 0;
    v50 = this[8];
    qmemcpy(this + 14, v55, 0x20u);
    v55[0] = -1090519040;
    v55[1] = -1090519040;
    memset(&v55[2], 0, 12);
    v55[5] = -1082130432;
    v55[6] = 0;
    v55[7] = 1065353216;
    v51 = 5 * v50;
    v52 = *((_DWORD *)this + 6);
    qmemcpy(this + 30, v55, 0x20u);
    v55[0] = 1056964608;
    v55[1] = 1056964608;
    memset(&v55[2], 0, 12);
    v55[5] = -1082130432;
    v55[6] = 1065353216;
    v55[7] = 0;
    qmemcpy(this + 46, v55, 0x20u);
    v55[0] = 1056964608;
    v55[1] = -1090519040;
    memset(&v55[2], 0, 12);
    v55[5] = -1082130432;
    v55[6] = 1065353216;
    v55[7] = 1065353216;
    qmemcpy(this + 62, v55, 0x20u);
    v53 = 2 * v51 + 10002;
    if ( v52 )
      sub_4832D0(v53, *(_DWORD *)v56, *(_DWORD *)v37, 0);
    if ( this[8] == 594 )
      sub_4832D0(v53, *(_DWORD *)v56, *(_DWORD *)v37, 0);
    return 0;
  }
  else
  {
    *((_BYTE *)this + 4) = 2;
    return 0;
  }
}
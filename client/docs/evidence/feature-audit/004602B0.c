int __thiscall sub_4602B0(_DWORD *this)
{
  int v2; // edi
  int v3; // esi
  int v4; // eax
  int v5; // ebx
  int *v6; // eax
  int v7; // ecx
  int v8; // eax
  unsigned int v9; // kr08_4
  int v10; // edi
  signed int v11; // edx
  int v12; // ecx
  char v13; // al
  int *v14; // eax
  char v15; // cl
  int v16; // ebx
  int v17; // eax
  int v18; // eax
  int v19; // ecx
  char v20; // cl
  int v21; // eax
  int v22; // ecx
  char v23; // cl
  int v24; // edi
  int v25; // ecx
  int v26; // eax
  int *v27; // eax
  int v28; // ebx
  int v29; // eax
  int v30; // ebx
  unsigned int v31; // kr0C_4
  signed int v32; // edx
  int v33; // ecx
  char v34; // al
  int *v35; // eax
  char v36; // cl
  int v37; // edi
  int v38; // eax
  int v39; // eax
  int v40; // ecx
  int v41; // eax
  int v42; // eax
  char v43; // cl
  int v44; // eax
  int v45; // eax
  int v46; // eax
  int v47; // eax
  char v48; // cl
  int v49; // eax
  int v50; // eax
  int v51; // eax
  int v52; // eax
  int v53; // edx
  int result; // eax
  char v55; // al
  int v56; // [esp+A8h] [ebp-878h]
  int v57; // [esp+ACh] [ebp-874h]
  int v58; // [esp+B0h] [ebp-870h]
  signed int v59; // [esp+B4h] [ebp-86Ch]
  int v60; // [esp+B4h] [ebp-86Ch]
  int v61; // [esp+B8h] [ebp-868h]
  int v62; // [esp+B8h] [ebp-868h]
  int v63; // [esp+B8h] [ebp-868h]
  int v64; // [esp+BCh] [ebp-864h]
  signed int v65; // [esp+BCh] [ebp-864h]
  int v66; // [esp+C0h] [ebp-860h]
  int v67; // [esp+C4h] [ebp-85Ch]
  int v68; // [esp+D0h] [ebp-850h]
  int v69; // [esp+D4h] [ebp-84Ch]
  int v70; // [esp+D4h] [ebp-84Ch]
  int v71; // [esp+D4h] [ebp-84Ch]
  const char *v72; // [esp+DCh] [ebp-844h]
  int v73; // [esp+E0h] [ebp-840h]
  int v74; // [esp+E0h] [ebp-840h]
  int v75; // [esp+E4h] [ebp-83Ch]
  int v76; // [esp+F8h] [ebp-828h] BYREF
  int v77; // [esp+100h] [ebp-820h] BYREF
  int v78; // [esp+108h] [ebp-818h] BYREF
  int v79; // [esp+110h] [ebp-810h] BYREF
  int v80; // [esp+118h] [ebp-808h] BYREF
  char v81[1024]; // [esp+120h] [ebp-800h] BYREF
  char v82[1024]; // [esp+520h] [ebp-400h] BYREF

  v2 = 0;
  v3 = 0;
  v56 = 0;
  v66 = this[6];
  v67 = this[7];
  (*(void (__thiscall **)(_DWORD *))(*this + 12))(this);
  v4 = dword_903AD4;
  v68 = dword_4B5BF8;
  v75 = dword_903AD4;
  if ( dword_903AD4 )
  {
    while ( 1 )
    {
      v5 = *(_DWORD *)(v4 + 4);
      v57 = v5;
      switch ( *(_BYTE *)v5 )
      {
        case 1:
          v6 = sub_48BF93(&dword_8FCC50, &v80, 0, 0, *(const char **)(v5 + 4));
          v7 = *v6;
          v8 = v6[1];
          v64 = v2 + v7;
          v73 = v3 >= 7 ? 305 : 198;
          if ( v2 + v7 <= v73 )
          {
            v23 = *((_BYTE *)this + 1434);
            if ( v23 != 1 || v3 >= 7 || *((_BYTE *)this + 1444) )
            {
              v24 = this[240];
              if ( v3 >= v24 && v3 <= v24 + this[363] )
              {
                if ( *((_BYTE *)this + 1444) == 1 && v23 == 1 )
                  v25 = v3 - v24 + 7;
                else
                  v25 = v3 - v24;
                sub_48BCAF(&dword_8FCC50, 0, v66 + v56 + 35, (v8 + 5) * v25 + v67 + 40, v68, 0, *(char **)(v5 + 4), 0);
              }
            }
            else
            {
              sub_48BCAF(
                &dword_8FCC50,
                0,
                v66 + v2 + 150,
                (v8 + 5) * (v3 - this[240]) + v67 + 40,
                v68,
                0,
                *(char **)(v5 + 4),
                0);
            }
            v56 = v64;
          }
          else
          {
            v9 = strlen(*(const char **)(v5 + 4)) + 1;
            v10 = 0;
            v59 = 0;
            if ( (int)(v9 - 1) > 0 )
            {
              do
              {
                v11 = v59;
                v12 = *(_DWORD *)(v57 + 4);
                v13 = *(_BYTE *)(v12 + v59);
                v82[v10] = v13;
                if ( v13 < 0 )
                {
                  ++v10;
                  ++v59;
                  v82[v10] = *(_BYTE *)(v12 + v11 + 1);
                }
                v82[v10 + 1] = 0;
                v14 = sub_48BF93(&dword_8FCC50, &v77, 0, 0, v82);
                v15 = *((_BYTE *)this + 1434);
                v69 = *v14;
                v16 = v14[1];
                if ( v15 != 1 || v3 >= 7 || v56 + *v14 < v73 || *((_BYTE *)this + 1444) )
                {
                  if ( v56 + v69 < 305 )
                  {
                    ++v10;
                  }
                  else
                  {
                    v18 = this[240];
                    if ( v3 >= v18 && v3 <= v18 + this[363] )
                    {
                      if ( *((_BYTE *)this + 1444) == 1 && v15 == 1 )
                        v19 = v3 - v18 + 7;
                      else
                        v19 = v3 - v18;
                      sub_48BCAF(&dword_8FCC50, 0, v66 + v56 + 35, (v16 + 5) * v19 + v67 + 40, v68, 0, v82, 0);
                    }
                    ++v3;
                    v10 = 0;
                    v56 = 0;
                  }
                }
                else
                {
                  v17 = this[240];
                  if ( v3 >= v17 && v3 <= v17 + this[363] )
                    sub_48BCAF(&dword_8FCC50, 0, v66 + v56 + 150, (v16 + 5) * (v3 - v17) + v67 + 40, v68, 0, v82, 0);
                  ++v3;
                  v10 = 0;
                  v56 = 0;
                }
                ++v59;
              }
              while ( v59 < (int)(v9 - 1) );
              if ( v10 )
              {
                v20 = *((_BYTE *)this + 1434);
                if ( v20 != 1 || v3 >= 7 || *((_BYTE *)this + 1444) )
                {
                  v21 = this[240];
                  if ( v3 >= v21 && v3 <= v21 + this[363] )
                  {
                    if ( *((_BYTE *)this + 1444) == 1 && v20 == 1 )
                      v22 = v3 - v21 + 7;
                    else
                      v22 = v3 - v21;
                    sub_48BCAF(&dword_8FCC50, 0, v66 + v56 + 35, (v16 + 5) * v22 + v67 + 40, v68, 0, v82, 0);
                  }
                }
                else
                {
                  sub_48BCAF(&dword_8FCC50, 0, v66 + v56 + 150, (v16 + 5) * (v3 - this[240]) + v67 + 40, v68, 0, v82, 0);
                }
                v56 += v69;
              }
            }
          }
          goto LABEL_124;
        case 2:
          v58 = 0;
          v60 = 0xFFFF;
          if ( *(_BYTE *)(v5 + 93) )
          {
            v26 = sub_480550(1074);
            v58 = sub_48BB04((int)&dword_8FCC50, v26, 9, 0, 400, 0, 1, 0, 1);
          }
          if ( *(_BYTE *)(v5 + 92) )
            v60 = 255;
          v27 = sub_48BF93(&dword_8FCC50, &v78, 0, 0, *(const char **)(v5 + 4));
          v70 = *v27;
          v28 = v27[1];
          v29 = (v3 >= 7) - 1;
          LOBYTE(v29) = v29 & 0x95;
          v74 = v29 + 305;
          if ( v56 + v70 <= v29 + 305 )
          {
            v48 = *((_BYTE *)this + 1434);
            if ( v48 != 1 || v3 >= 7 || *((_BYTE *)this + 1444) )
            {
              v51 = this[240];
              if ( v3 >= v51 && v3 <= v51 + this[363] )
              {
                if ( *((_BYTE *)this + 1444) == 1 && v48 == 1 )
                  v63 = (v28 + 5) * (v3 - v51 + 7) + v67 + 40;
                else
                  v63 = (v28 + 5) * (v3 - v51) + v67 + 40;
                if ( *(_BYTE *)(v57 + 93) )
                {
                  v52 = sub_480550(1074);
                  v58 = sub_48BB04((int)&dword_8FCC50, v52, 9, 0, 400, 0, 1, 0, 1);
                }
                sub_48BCAF(&dword_8FCC50, 0, v66 + v56 + 35, v63, v60, 0, *(char **)(v57 + 4), v58);
                sub_461B70(v57, v66 + v56 + 35, v63, v70 + v66 + v56 + 35, v63 + v28);
              }
            }
            else
            {
              if ( *(_BYTE *)(v57 + 93) )
              {
                v49 = sub_480550(1074);
                v58 = sub_48BB04((int)&dword_8FCC50, v49, 9, 0, 400, 0, 1, 0, 1);
              }
              sub_48BCAF(
                &dword_8FCC50,
                0,
                v66 + v56 + 150,
                (v28 + 5) * (v3 - this[240]) + v67 + 40,
                v60,
                0,
                *(char **)(v57 + 4),
                v58);
              v50 = (v28 + 5) * (v3 - this[240]) + v67 + 40;
              sub_461B70(v57, v66 + v56 + 150, v50, v66 + v56 + 150 + v70, v50 + v28);
            }
            v56 += v70;
LABEL_118:
            dword_4AF064(v58);
            goto LABEL_124;
          }
          v30 = 0;
          v31 = strlen(*(const char **)(v57 + 4)) + 1;
          v65 = 0;
          if ( (int)(v31 - 1) <= 0 )
            goto LABEL_118;
          do
          {
            v32 = v65;
            v33 = *(_DWORD *)(v57 + 4);
            v34 = *(_BYTE *)(v33 + v65);
            v81[v30] = v34;
            if ( v34 < 0 )
            {
              ++v30;
              ++v65;
              v81[v30] = *(_BYTE *)(v33 + v32 + 1);
            }
            v81[v30 + 1] = 0;
            v35 = sub_48BF93(&dword_8FCC50, &v76, 0, 0, v81);
            v36 = *((_BYTE *)this + 1434);
            v71 = *v35;
            v37 = v35[1];
            if ( v36 != 1 || v3 >= 7 || v56 + *v35 < v74 || *((_BYTE *)this + 1444) )
            {
              if ( v56 + v71 < 305 )
              {
                ++v30;
              }
              else
              {
                v41 = this[240];
                if ( v3 >= v41 && v3 <= v41 + this[363] )
                {
                  if ( *((_BYTE *)this + 1444) == 1 && v36 == 1 )
                    v61 = (v37 + 5) * (v3 - v41 + 7) + v67 + 40;
                  else
                    v61 = (v37 + 5) * (v3 - v41) + v67 + 40;
                  if ( *(_BYTE *)(v57 + 93) )
                  {
                    v42 = sub_480550(1074);
                    v58 = sub_48BB04((int)&dword_8FCC50, v42, 9, 0, 400, 0, 1, 0, 1);
                  }
                  sub_48BCAF(&dword_8FCC50, 0, v66 + v56 + 35, v61, v60, 0, v81, v58);
                  sub_461B70(v57, v66 + v56 + 35, v61, v66 + v56 + 35 + v71, v37 + v61);
                }
                ++v3;
                v30 = 0;
                v56 = 0;
              }
            }
            else
            {
              if ( *(_BYTE *)(v57 + 93) )
              {
                v38 = sub_480550(1074);
                v58 = sub_48BB04((int)&dword_8FCC50, v38, 9, 0, 400, 0, 1, 0, 1);
              }
              v39 = this[240];
              if ( v3 >= v39 && v3 <= v39 + this[363] )
                sub_48BCAF(&dword_8FCC50, 0, v66 + v56 + 150, (v37 + 5) * (v3 - v39) + v67 + 40, v60, 0, v81, v58);
              v40 = (v37 + 5) * (v3 - this[240]) + v67 + 40;
              sub_461B70(v57, v66 + v56 + 150, v40, v66 + v56 + 150 + v71, v37 + v40);
              v30 = 0;
              sub_48BF93(&dword_8FCC50, &v79, 0, 0, v81);
              ++v3;
              v56 = 0;
            }
            ++v65;
          }
          while ( v65 < (int)(v31 - 1) );
          if ( !v30 )
            goto LABEL_118;
          v43 = *((_BYTE *)this + 1434);
          if ( v43 != 1 || v3 >= 7 || *((_BYTE *)this + 1444) )
          {
            v46 = this[240];
            if ( v3 >= v46 && v3 <= v46 + this[363] )
            {
              if ( *((_BYTE *)this + 1444) == 1 && v43 == 1 )
                v62 = (v37 + 5) * (v3 - v46 + 7) + v67 + 40;
              else
                v62 = (v37 + 5) * (v3 - v46) + v67 + 40;
              if ( *(_BYTE *)(v57 + 93) )
              {
                v47 = sub_480550(1074);
                v58 = sub_48BB04((int)&dword_8FCC50, v47, 9, 0, 400, 0, 1, 0, 1);
              }
              sub_48BCAF(&dword_8FCC50, 0, v66 + v56 + 35, v62, v60, 0, v81, v58);
              sub_461B70(v57, v66 + v56 + 35, v62, v71 + v66 + v56 + 35, v62 + v37);
            }
          }
          else
          {
            if ( *(_BYTE *)(v57 + 93) )
            {
              v44 = sub_480550(1074);
              v58 = sub_48BB04((int)&dword_8FCC50, v44, 9, 0, 400, 0, 1, 0, 1);
            }
            sub_48BCAF(&dword_8FCC50, 0, v66 + v56 + 150, (v37 + 5) * (v3 - this[240]) + v67 + 40, v60, 0, v81, v58);
            v45 = (v37 + 5) * (v3 - this[240]) + v67 + 40;
            sub_461B70(v57, v66 + v56 + 150, v45, v66 + v56 + 150 + v71, v45 + v37);
          }
          v56 += v71;
          dword_4AF064(v58);
LABEL_124:
          v75 = *(_DWORD *)(v75 + 12);
          v4 = v75;
          if ( !v75 )
            goto LABEL_125;
          v2 = v56;
          break;
        case 3:
          ++v3;
          v56 = 0;
          goto LABEL_124;
        case 4:
          v72 = *(const char **)(v5 + 4);
          if ( !strcmp(v72, MEMORY[0x4B5C34]) )
          {
            v68 = dword_4B5BC8[MEMORY[0x49FAF5](*(_DWORD *)(v5 + 8))];
          }
          else if ( strcmp(*(const char **)(v5 + 4), MEMORY[0x4B5C2C]) && !strcmp(v72, MEMORY[0x4B5C20]) )
          {
            this[157] = 0;
          }
          goto LABEL_124;
        default:
          goto LABEL_124;
      }
    }
  }
LABEL_125:
  v53 = this[22];
  this[241] = v3;
  result = (*(int (__thiscall **)(_DWORD *))(v53 + 4))(this + 22);
  if ( *((_BYTE *)this + 1444) == 1 )
  {
    if ( this[241] - this[240] <= this[363] )
    {
      (*(void (__thiscall **)(_DWORD *))(this[67] + 4))(this + 67);
    }
    else
    {
      if ( *((_BYTE *)this + 1445) == 1 )
        (*(void (__thiscall **)(_DWORD *))(this[67] + 4))(this + 67);
      if ( this[362] == 16 )
      {
        v55 = *((_BYTE *)this + 1445);
        this[362] = 0;
        *((_BYTE *)this + 1445) = v55 != 1;
      }
      ++this[362];
    }
    return (*(int (__thiscall **)(_DWORD *))(this[112] + 4))(this + 112);
  }
  return result;
}
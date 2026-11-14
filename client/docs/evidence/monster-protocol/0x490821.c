int __thiscall sub_490821(
        int this,
        int a2,
        int a3,
        unsigned __int16 *a4,
        int a5,
        int a6,
        unsigned __int16 a7,
        unsigned __int16 a8,
        __int16 a9,
        int a10,
        int a11)
{
  unsigned __int16 *v11; // edx
  int v13; // edi
  bool v14; // zf
  int v15; // edx
  int v16; // eax
  int v17; // ebx
  int v18; // edx
  int v19; // ecx
  double v20; // st7
  int v21; // edx
  char *v22; // eax
  __int16 v23; // cx
  int v24; // edx
  int v25; // ecx
  char *v26; // eax
  int v27; // eax
  unsigned int v28; // edi
  unsigned int v29; // edx
  unsigned int v30; // eax
  char v31; // bl
  __int64 v32; // rax
  unsigned __int8 v33; // bl
  unsigned int v34; // edi
  char *v35; // ecx
  unsigned int v36; // edx
  int v37; // edx
  char v38; // bl
  unsigned int v39; // edi
  int v40; // eax
  int v41; // et2
  char v42; // cl
  unsigned __int16 *v43; // ecx
  char *v44; // eax
  unsigned int v45; // edi
  unsigned int v46; // edx
  unsigned int v47; // eax
  char v48; // bl
  __int64 v49; // rax
  unsigned __int8 v50; // bl
  int v51; // ecx
  unsigned int v52; // edx
  int v53; // edx
  char v54; // bl
  int v55; // eax
  int v56; // et2
  unsigned int v57; // edi
  char v58; // cl
  int v59; // edx
  unsigned __int16 *v60; // eax
  unsigned int v61; // edi
  unsigned int v62; // edx
  unsigned int v63; // eax
  char v64; // bl
  __int64 v65; // rax
  unsigned __int8 v66; // bl
  unsigned int v67; // edi
  int v68; // ecx
  unsigned int v69; // edx
  int v70; // edx
  char v71; // bl
  unsigned int v72; // edi
  int v73; // eax
  int v74; // et2
  char v75; // cl
  unsigned __int16 *v76; // ecx
  int v77; // edi
  _DWORD v79[4]; // [esp+Ch] [ebp-11Ch] BYREF
  int v80; // [esp+1Ch] [ebp-10Ch]
  int v81; // [esp+30h] [ebp-F8h]
  char *v82; // [esp+90h] [ebp-98h]
  int v83; // [esp+94h] [ebp-94h]
  int v84; // [esp+98h] [ebp-90h]
  int v85; // [esp+9Ch] [ebp-8Ch]
  unsigned int v86; // [esp+A0h] [ebp-88h]
  int v87; // [esp+A4h] [ebp-84h]
  unsigned int v88; // [esp+A8h] [ebp-80h]
  int v89; // [esp+ACh] [ebp-7Ch]
  int v90; // [esp+B0h] [ebp-78h]
  int v91; // [esp+B4h] [ebp-74h]
  int v92; // [esp+B8h] [ebp-70h]
  int v93; // [esp+BCh] [ebp-6Ch]
  unsigned int v94; // [esp+C0h] [ebp-68h]
  int v95; // [esp+C4h] [ebp-64h]
  unsigned int v96; // [esp+C8h] [ebp-60h]
  int v97; // [esp+CCh] [ebp-5Ch]
  unsigned int v98; // [esp+D0h] [ebp-58h]
  int v99; // [esp+D4h] [ebp-54h]
  unsigned int v100; // [esp+D8h] [ebp-50h]
  int v101; // [esp+DCh] [ebp-4Ch]
  int v102; // [esp+E0h] [ebp-48h]
  int v103; // [esp+E4h] [ebp-44h]
  int v104; // [esp+E8h] [ebp-40h]
  int v105; // [esp+ECh] [ebp-3Ch]
  unsigned __int16 *v106; // [esp+F0h] [ebp-38h]
  int v107; // [esp+F4h] [ebp-34h]
  float v108; // [esp+F8h] [ebp-30h]
  int v109; // [esp+FCh] [ebp-2Ch]
  unsigned int v110; // [esp+100h] [ebp-28h]
  unsigned int v111; // [esp+104h] [ebp-24h]
  char *v112; // [esp+108h] [ebp-20h]
  unsigned int v113; // [esp+10Ch] [ebp-1Ch]
  char *v114; // [esp+110h] [ebp-18h]
  int v115; // [esp+114h] [ebp-14h]
  int v116; // [esp+118h] [ebp-10h]
  int v117; // [esp+11Ch] [ebp-Ch]
  unsigned __int16 *v118; // [esp+120h] [ebp-8h]
  unsigned __int16 *v119; // [esp+124h] [ebp-4h]
  char *v120; // [esp+134h] [ebp+Ch]
  char *v121; // [esp+134h] [ebp+Ch]
  int v122; // [esp+134h] [ebp+Ch]
  int v123; // [esp+144h] [ebp+1Ch]
  char v124; // [esp+147h] [ebp+1Fh]
  unsigned __int8 v125; // [esp+147h] [ebp+1Fh]
  char v126; // [esp+147h] [ebp+1Fh]
  char v127; // [esp+147h] [ebp+1Fh]
  unsigned __int8 v128; // [esp+147h] [ebp+1Fh]
  char v129; // [esp+147h] [ebp+1Fh]
  char v130; // [esp+147h] [ebp+1Fh]
  unsigned __int8 v131; // [esp+147h] [ebp+1Fh]
  char v132; // [esp+147h] [ebp+1Fh]
  int v133; // [esp+148h] [ebp+20h]
  unsigned __int8 v134; // [esp+14Bh] [ebp+23h]
  char v135; // [esp+14Bh] [ebp+23h]
  unsigned __int8 v136; // [esp+14Bh] [ebp+23h]
  char v137; // [esp+14Bh] [ebp+23h]
  unsigned __int8 v138; // [esp+14Bh] [ebp+23h]
  char v139; // [esp+14Bh] [ebp+23h]

  v11 = a4;
  v107 = a5;
  v13 = 0;
  v14 = *(_DWORD *)(this + 28) == 0;
  v119 = a4;
  v115 = 0;
  if ( v14 )
    return 0;
  if ( a2 < 0 )
  {
    v115 = -a2;
    v119 = (unsigned __int16 *)((char *)a4 + a2);
    v11 = a4;
  }
  if ( (int)v11 + a2 - 1 > a7 - 1 )
    v119 = (unsigned __int16 *)(a7 - v115 - a2);
  if ( a3 >= 0 )
  {
    v15 = v107;
  }
  else
  {
    v13 = -a3;
    v15 = a5 + a3;
  }
  if ( a3 + a5 - 1 > a8 - 1 )
    v15 = a8 - v13 - a3;
  if ( (int)v119 <= 0 || v15 <= 0 )
    goto LABEL_90;
  v83 = v13 + v15;
  v82 = (char *)v119 + v115;
  MEMORY[0x4A1630](v79, 0, 124);
  v16 = *(_DWORD *)(this + 28);
  v79[0] = 124;
  if ( (*(int (__stdcall **)(int, _DWORD, _DWORD *, int, _DWORD))(*(_DWORD *)v16 + 100))(v16, 0, v79, 1, 0) < 0 )
    return 0;
  v17 = a6;
  v18 = 0;
  v19 = v80 >> 1;
  v104 = v81;
  v102 = v80 >> 1;
  v123 = 0;
  v117 = 0;
  if ( v13 > 0 )
  {
    v133 = v13;
    do
    {
      v14 = v133-- == 1;
      v18 += *(unsigned __int16 *)(a6 + 2 * v123) + 1;
      v123 = v18;
    }
    while ( !v14 );
  }
  v105 = v13;
  if ( v13 >= v83 )
  {
LABEL_90:
    v77 = 1;
    goto LABEL_91;
  }
  v20 = flt_4AFECC;
  v109 = v19 * (v13 + a3);
  v106 = (unsigned __int16 *)(v81 + 2 * (a2 + v115 + v109));
  while ( 1 )
  {
    v21 = *(unsigned __int16 *)(v17 + 2 * v123) + v18;
    v22 = (char *)(v123 + 1);
    v107 = v21;
    if ( v123 + 1 < v21 )
      break;
LABEL_89:
    v117 = 0;
    v109 += v19;
    v18 = v21 + 1;
    ++v105;
    v123 = v18;
    v106 += v19;
    if ( v105 >= v83 )
      goto LABEL_90;
  }
  while ( 1 )
  {
    v23 = *(_WORD *)(v17 + 2 * (_DWORD)v22);
    if ( v23 == 192 )
    {
      v117 += *(unsigned __int16 *)(v17 + 2 * (_DWORD)v22 + 2);
      v120 = v22 + 2;
      goto LABEL_87;
    }
    if ( v23 != 193 && v23 != 194 && v23 != 195 )
      break;
    v24 = *(unsigned __int16 *)(v17 + 2 * (_DWORD)v22 + 2);
    v25 = v117;
    v117 += v24;
    v26 = v22 + 2;
    v112 = (char *)v24;
    v121 = v26;
    v103 = v25;
    if ( v115 > v117 || (int)v82 < v25 )
    {
      v44 = &v26[v24];
LABEL_86:
      v120 = v44;
      goto LABEL_87;
    }
    if ( v25 < v115 )
    {
      v119 = 0;
      v27 = (int)&v26[v115 - v25];
      v122 = v27;
      v116 = v117 - v115;
      if ( v117 - v115 > 0 )
      {
        v118 = v106;
        v112 = (char *)(v17 + 2 * v27);
        do
        {
          v28 = *(unsigned __int16 *)v112;
          v29 = (v28 & *(_DWORD *)(this + 376)) >> *(_BYTE *)(this + 365);
          v30 = (v28 & *(_DWORD *)(this + 372)) >> *(_BYTE *)(this + 364);
          if ( a9 )
          {
            v34 = *(_DWORD *)(this + 376) >> *(_BYTE *)(this + 365);
            v35 = (char *)(unsigned __int8)v29;
            v36 = *(_DWORD *)(this + 372);
            v114 = v35;
            LOBYTE(v35) = *(_BYTE *)(this + 364);
            v113 = v34;
            v94 = v34;
            v114 = (char *)(unsigned __int8)v30;
            v95 = 0;
            v85 = 0;
            v37 = v36 >> (char)v35;
            LOBYTE(v35) = *(_BYTE *)(this + 363);
            v84 = v37;
            v100 = *(_DWORD *)(this + 368) >> (char)v35;
            v101 = 0;
            v111 = v100;
            v110 = v37;
            v108 = (double)(unsigned __int8)v30 / (double)v37;
            v38 = MEMORY[0x49FE1C](v20);
            v33 = MEMORY[0x49FE1C](v20) + v38;
            v126 = MEMORY[0x49FE1C](v20);
            v125 = MEMORY[0x49FE1C](v20) + v126;
            v135 = MEMORY[0x49FE1C](v20);
            v32 = MEMORY[0x49FE1C](v20);
            v134 = v32 + v135;
            if ( v33 > v113 )
              v33 = v113;
            if ( v125 > v110 )
              v125 = v110;
            WORD1(v32) = HIWORD(v111);
            if ( v134 > v111 )
              v134 = v111;
          }
          else
          {
            v31 = (unsigned __int8)(v29
                                  + ((v28 & *(_DWORD *)(this + 368)) >> *(_BYTE *)(this + 363))
                                  + ((unsigned __int8)v30 >> 1))
                / 3;
            v32 = MEMORY[0x49FE1C](v20);
            v124 = v32 + v31;
            if ( (unsigned __int8)(v32 + v31) > 0x1Fu )
              v124 = 31;
            v134 = v124;
            v33 = v124;
            v125 = 2 * v124;
          }
          if ( a11 )
          {
            v39 = *(unsigned __int16 *)(v104 + 2 * ((_DWORD)v119 + v109 + v103 + a2));
            v33 = 50 * (v33 + (unsigned __int8)((v39 & *(_DWORD *)(this + 376)) >> *(_BYTE *)(this + 365))) / 100;
            v125 = 50 * (v125 + (unsigned __int8)((v39 & *(_DWORD *)(this + 372)) >> *(_BYTE *)(this + 364))) / 100;
            v40 = v134 + (unsigned __int8)((v39 & *(_DWORD *)(this + 368)) >> *(_BYTE *)(this + 363));
            v41 = 50 * v40 % 100;
            LODWORD(v32) = 50 * v40 / 100;
            HIWORD(v32) = HIWORD(v41);
          }
          else
          {
            LOBYTE(v32) = v134;
          }
          WORD2(v32) = v125;
          LOWORD(v32) = (unsigned __int8)v32;
          HIDWORD(v32) <<= *(_BYTE *)(this + 364);
          v42 = *(_BYTE *)(this + 365);
          LOWORD(v32) = WORD2(v32) | ((_DWORD)v32 << *(_BYTE *)(this + 363));
          v112 += 2;
          WORD2(v32) = v33;
          HIDWORD(v32) <<= v42;
          v43 = v118++;
          v119 = (unsigned __int16 *)((char *)v119 + 1);
          *v43 = WORD2(v32) | v32;
        }
        while ( (int)v119 < v116 );
        v27 = v122;
      }
      v44 = (char *)(v117 - v115 + v27);
      goto LABEL_86;
    }
    if ( v25 > (int)v82 || (int)v82 >= v117 )
    {
      if ( v24 > 0 )
      {
        v119 = (unsigned __int16 *)(v104 + 2 * (a2 + v25 + v109));
        v118 = (unsigned __int16 *)(a6 + 2 * (_DWORD)v26);
        v114 = v112;
        do
        {
          v61 = *v118;
          v62 = (v61 & *(_DWORD *)(this + 376)) >> *(_BYTE *)(this + 365);
          v63 = (v61 & *(_DWORD *)(this + 372)) >> *(_BYTE *)(this + 364);
          if ( a9 )
          {
            v67 = *(_DWORD *)(this + 376) >> *(_BYTE *)(this + 365);
            v68 = (unsigned __int8)v62;
            v69 = *(_DWORD *)(this + 372);
            v116 = v68;
            LOBYTE(v68) = *(_BYTE *)(this + 364);
            v113 = v67;
            v96 = v67;
            v116 = (unsigned __int8)v63;
            v97 = 0;
            v93 = 0;
            v70 = v69 >> v68;
            LOBYTE(v68) = *(_BYTE *)(this + 363);
            v92 = v70;
            v88 = *(_DWORD *)(this + 368) >> v68;
            v89 = 0;
            v111 = v88;
            v110 = v70;
            v108 = (double)(unsigned __int8)v63 / (double)v70;
            v71 = MEMORY[0x49FE1C](v20);
            v66 = MEMORY[0x49FE1C](v20) + v71;
            v132 = MEMORY[0x49FE1C](v20);
            v131 = MEMORY[0x49FE1C](v20) + v132;
            v139 = MEMORY[0x49FE1C](v20);
            v65 = MEMORY[0x49FE1C](v20);
            v138 = v65 + v139;
            if ( v66 > v113 )
              v66 = v113;
            if ( v131 > v110 )
              v131 = v110;
            WORD1(v65) = HIWORD(v111);
            if ( v138 > v111 )
              v138 = v111;
          }
          else
          {
            v64 = (unsigned __int8)(v62
                                  + ((v61 & *(_DWORD *)(this + 368)) >> *(_BYTE *)(this + 363))
                                  + ((unsigned __int8)v63 >> 1))
                / 3;
            v65 = MEMORY[0x49FE1C](v20);
            v130 = v65 + v64;
            if ( (unsigned __int8)(v65 + v64) > 0x1Fu )
              v130 = 31;
            v138 = v130;
            v66 = v130;
            v131 = 2 * v130;
          }
          if ( a11 )
          {
            v72 = *v119;
            v66 = 50 * (v66 + (unsigned __int8)((v72 & *(_DWORD *)(this + 376)) >> *(_BYTE *)(this + 365))) / 100;
            v131 = 50 * (v131 + (unsigned __int8)((v72 & *(_DWORD *)(this + 372)) >> *(_BYTE *)(this + 364))) / 100;
            v73 = v138 + (unsigned __int8)((v72 & *(_DWORD *)(this + 368)) >> *(_BYTE *)(this + 363));
            v74 = 50 * v73 % 100;
            LODWORD(v65) = 50 * v73 / 100;
            HIWORD(v65) = HIWORD(v74);
          }
          else
          {
            LOBYTE(v65) = v138;
          }
          WORD2(v65) = v131;
          LOWORD(v65) = (unsigned __int8)v65;
          HIDWORD(v65) <<= *(_BYTE *)(this + 364);
          v75 = *(_BYTE *)(this + 365);
          LODWORD(v65) = HIDWORD(v65) | ((_DWORD)v65 << *(_BYTE *)(this + 363));
          ++v118;
          WORD2(v65) = v66;
          HIDWORD(v65) <<= v75;
          v76 = v119++;
          v14 = v114-- == (char *)1;
          *v76 = WORD2(v65) | v65;
        }
        while ( !v14 );
      }
      v120 = &v121[(_DWORD)v112];
    }
    else
    {
      if ( (int)&v82[-v25] > 0 )
      {
        v114 = &v82[-v25];
        v119 = (unsigned __int16 *)(v104 + 2 * (a2 + v25 + v109));
        v118 = (unsigned __int16 *)(a6 + 2 * (_DWORD)v26);
        do
        {
          v45 = *v118;
          v46 = (v45 & *(_DWORD *)(this + 376)) >> *(_BYTE *)(this + 365);
          v47 = (v45 & *(_DWORD *)(this + 372)) >> *(_BYTE *)(this + 364);
          if ( a9 )
          {
            v45 = *(_DWORD *)(this + 376) >> *(_BYTE *)(this + 365);
            v51 = (unsigned __int8)v46;
            v52 = *(_DWORD *)(this + 372);
            v116 = v51;
            LOBYTE(v51) = *(_BYTE *)(this + 364);
            v113 = v45;
            v86 = v45;
            HIWORD(v45) = 0;
            v116 = (unsigned __int8)v47;
            v87 = 0;
            v91 = 0;
            v53 = v52 >> v51;
            LOBYTE(v51) = *(_BYTE *)(this + 363);
            v90 = v53;
            v98 = *(_DWORD *)(this + 368) >> v51;
            v99 = 0;
            v111 = v98;
            v110 = v53;
            v108 = (double)(unsigned __int8)v47 / (double)v53;
            v54 = MEMORY[0x49FE1C](v20);
            v50 = MEMORY[0x49FE1C](v20) + v54;
            v129 = MEMORY[0x49FE1C](v20);
            v128 = MEMORY[0x49FE1C](v20) + v129;
            v137 = MEMORY[0x49FE1C](v20);
            v49 = MEMORY[0x49FE1C](v20);
            v136 = v49 + v137;
            if ( v50 > v113 )
              v50 = v113;
            if ( v128 > v110 )
              v128 = v110;
            WORD1(v49) = HIWORD(v111);
            if ( v136 > v111 )
              v136 = v111;
          }
          else
          {
            v48 = (unsigned __int8)(v46
                                  + ((v45 & *(_DWORD *)(this + 368)) >> *(_BYTE *)(this + 363))
                                  + ((unsigned __int8)v47 >> 1))
                / 3;
            v49 = MEMORY[0x49FE1C](v20);
            v127 = v49 + v48;
            if ( (unsigned __int8)(v49 + v48) > 0x1Fu )
              v127 = 31;
            v136 = v127;
            v50 = v127;
            v128 = 2 * v127;
          }
          if ( a11 )
          {
            v45 = *v119;
            v50 = 50 * (v50 + (unsigned __int8)((v45 & *(_DWORD *)(this + 376)) >> *(_BYTE *)(this + 365))) / 100;
            v128 = 50 * (v128 + (unsigned __int8)((v45 & *(_DWORD *)(this + 372)) >> *(_BYTE *)(this + 364))) / 100;
            v55 = v136 + (unsigned __int8)((v45 & *(_DWORD *)(this + 368)) >> *(_BYTE *)(this + 363));
            v56 = 50 * v55 % 100;
            LODWORD(v49) = 50 * v55 / 100;
            HIWORD(v49) = HIWORD(v56);
          }
          else
          {
            LOBYTE(v49) = v136;
          }
          LOWORD(v45) = v128;
          WORD2(v49) = v50;
          HIDWORD(v49) <<= *(_BYTE *)(this + 363);
          v57 = v45 << *(_BYTE *)(this + 364);
          v58 = *(_BYTE *)(this + 365);
          LOWORD(v49) = (unsigned __int8)v49;
          ++v118;
          v59 = ((_DWORD)v49 << v58) | v57 | HIDWORD(v49);
          v60 = v119++;
          v14 = v114-- == (char *)1;
          *v60 = v59;
        }
        while ( !v14 );
        v24 = (int)v112;
      }
      v120 = &v121[v24];
    }
LABEL_87:
    if ( (int)v120 >= v107 )
    {
      v17 = a6;
      v19 = v102;
      v21 = v107;
      goto LABEL_89;
    }
    v22 = v120;
    v17 = a6;
  }
  v77 = 0;
LABEL_91:
  (*(void (__stdcall **)(_DWORD, _DWORD))(**(_DWORD **)(this + 28) + 128))(*(_DWORD *)(this + 28), 0);
  return v77;
}
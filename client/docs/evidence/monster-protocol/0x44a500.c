int __thiscall sub_44A500(int this)
{
  int v3; // edx
  int v4; // eax
  int v5; // eax
  int v6; // ecx
  __int16 v7; // ax
  int v8; // ecx
  int v9; // eax
  int v10; // ecx
  int v11; // eax
  int v12; // ecx
  int v13; // eax
  int v14; // ecx
  int v15; // eax
  int v16; // ecx
  int v17; // eax
  int v18; // eax
  int v19; // eax
  int v20; // eax
  int v21; // ecx
  int v22; // eax
  int v23; // ecx
  int v24; // eax
  int v25; // ecx
  int v26; // eax
  int v27; // ecx
  int v28; // eax
  __int16 v29; // ax
  __int16 *v30; // eax
  int v31; // edx
  __int16 *v32; // eax
  double v33; // st7
  float v34; // [esp+0h] [ebp-2F8h]
  float v35; // [esp+4h] [ebp-2F4h]
  float v36; // [esp+8h] [ebp-2F0h]
  float v37; // [esp+20h] [ebp-2D8h]
  float v38; // [esp+24h] [ebp-2D4h]
  float v39; // [esp+28h] [ebp-2D0h]
  float v40; // [esp+28h] [ebp-2D0h]
  float v41; // [esp+28h] [ebp-2D0h]
  float v42; // [esp+28h] [ebp-2D0h]
  float v43; // [esp+28h] [ebp-2D0h]
  float v44; // [esp+28h] [ebp-2D0h]
  float v45; // [esp+28h] [ebp-2D0h]
  float v46; // [esp+28h] [ebp-2D0h]
  float v47; // [esp+28h] [ebp-2D0h]
  float v48; // [esp+28h] [ebp-2D0h]
  float v49; // [esp+28h] [ebp-2D0h]
  float v50; // [esp+28h] [ebp-2D0h]
  float v51; // [esp+28h] [ebp-2D0h]
  float v52; // [esp+28h] [ebp-2D0h]
  float v53; // [esp+28h] [ebp-2D0h]
  float v54; // [esp+28h] [ebp-2D0h]
  float v55; // [esp+28h] [ebp-2D0h]
  float v56; // [esp+28h] [ebp-2D0h]
  float v57; // [esp+28h] [ebp-2D0h]
  float v58; // [esp+2Ch] [ebp-2CCh]
  float v59; // [esp+2Ch] [ebp-2CCh]
  float v60; // [esp+2Ch] [ebp-2CCh]
  float v61; // [esp+2Ch] [ebp-2CCh]
  float v62; // [esp+2Ch] [ebp-2CCh]
  float v63; // [esp+2Ch] [ebp-2CCh]
  float v64; // [esp+2Ch] [ebp-2CCh]
  float v65; // [esp+2Ch] [ebp-2CCh]
  float v66; // [esp+2Ch] [ebp-2CCh]
  float v67; // [esp+2Ch] [ebp-2CCh]
  float v68; // [esp+2Ch] [ebp-2CCh]
  float v69; // [esp+2Ch] [ebp-2CCh]
  float v70; // [esp+2Ch] [ebp-2CCh]
  float v71; // [esp+2Ch] [ebp-2CCh]
  float v72; // [esp+2Ch] [ebp-2CCh]
  float v73; // [esp+2Ch] [ebp-2CCh]
  float v74; // [esp+2Ch] [ebp-2CCh]
  int v75; // [esp+34h] [ebp-2C4h] BYREF
  int v76; // [esp+38h] [ebp-2C0h]
  int v77; // [esp+3Ch] [ebp-2BCh]
  int v78; // [esp+40h] [ebp-2B8h]
  int v79; // [esp+44h] [ebp-2B4h]
  int v80; // [esp+48h] [ebp-2B0h]
  int v81; // [esp+4Ch] [ebp-2ACh]
  int v82; // [esp+50h] [ebp-2A8h]
  int v83; // [esp+54h] [ebp-2A4h]
  int v84; // [esp+58h] [ebp-2A0h]
  int v85; // [esp+5Ch] [ebp-29Ch]
  int v86; // [esp+60h] [ebp-298h]
  int v87; // [esp+64h] [ebp-294h]
  int v88; // [esp+68h] [ebp-290h]
  int v89; // [esp+6Ch] [ebp-28Ch]
  int v90; // [esp+70h] [ebp-288h]
  int v91; // [esp+74h] [ebp-284h]
  int v92; // [esp+78h] [ebp-280h] BYREF
  int v93; // [esp+7Ch] [ebp-27Ch]
  int v94; // [esp+80h] [ebp-278h]
  int v95; // [esp+84h] [ebp-274h]
  int v96; // [esp+88h] [ebp-270h]
  int v97; // [esp+8Ch] [ebp-26Ch]
  int v98; // [esp+90h] [ebp-268h]
  int v99; // [esp+94h] [ebp-264h]
  int v100; // [esp+98h] [ebp-260h]
  int v101; // [esp+9Ch] [ebp-25Ch]
  int v102; // [esp+A0h] [ebp-258h]
  int v103; // [esp+A4h] [ebp-254h]
  int v104; // [esp+A8h] [ebp-250h]
  int v105; // [esp+ACh] [ebp-24Ch]
  int v106; // [esp+B0h] [ebp-248h]
  int v107; // [esp+B4h] [ebp-244h]
  int v108; // [esp+B8h] [ebp-240h]
  int v109; // [esp+BCh] [ebp-23Ch]
  int v110; // [esp+C0h] [ebp-238h]
  int v111; // [esp+C4h] [ebp-234h]
  int v112; // [esp+C8h] [ebp-230h]
  int v113; // [esp+CCh] [ebp-22Ch]
  int v114; // [esp+D0h] [ebp-228h]
  int v115; // [esp+D4h] [ebp-224h]
  int v116; // [esp+D8h] [ebp-220h]
  int v117; // [esp+DCh] [ebp-21Ch]
  int v118; // [esp+E0h] [ebp-218h]
  int v119; // [esp+E4h] [ebp-214h]
  int v120; // [esp+E8h] [ebp-210h]
  int v121; // [esp+ECh] [ebp-20Ch]
  int v122; // [esp+F0h] [ebp-208h]
  int v123; // [esp+F4h] [ebp-204h]
  int v124; // [esp+F8h] [ebp-200h] BYREF
  _DWORD v125[2]; // [esp+FCh] [ebp-1FCh]
  int v126; // [esp+104h] [ebp-1F4h]
  int v127; // [esp+108h] [ebp-1F0h]
  int v128; // [esp+10Ch] [ebp-1ECh]
  int v129; // [esp+110h] [ebp-1E8h]
  int v130; // [esp+114h] [ebp-1E4h]
  int v131; // [esp+118h] [ebp-1E0h]
  int v132; // [esp+11Ch] [ebp-1DCh]
  int v133; // [esp+120h] [ebp-1D8h]
  int v134; // [esp+124h] [ebp-1D4h]
  int v135; // [esp+128h] [ebp-1D0h]
  int v136; // [esp+12Ch] [ebp-1CCh]
  int v137; // [esp+130h] [ebp-1C8h]
  int v138; // [esp+134h] [ebp-1C4h]
  int v139; // [esp+138h] [ebp-1C0h]
  int v140; // [esp+13Ch] [ebp-1BCh]
  int v141; // [esp+140h] [ebp-1B8h]
  int v142; // [esp+144h] [ebp-1B4h]
  int v143; // [esp+148h] [ebp-1B0h]
  int v144; // [esp+14Ch] [ebp-1ACh]
  int v145; // [esp+150h] [ebp-1A8h]
  int v146; // [esp+154h] [ebp-1A4h]
  int v147; // [esp+158h] [ebp-1A0h]
  int v148; // [esp+15Ch] [ebp-19Ch]
  int v149; // [esp+160h] [ebp-198h]
  int v150; // [esp+164h] [ebp-194h]
  int v151; // [esp+168h] [ebp-190h]
  int v152; // [esp+16Ch] [ebp-18Ch]
  int v153; // [esp+170h] [ebp-188h]
  int v154; // [esp+174h] [ebp-184h]
  int v155; // [esp+178h] [ebp-180h] BYREF
  _DWORD v156[31]; // [esp+17Ch] [ebp-17Ch]
  int v157; // [esp+1F8h] [ebp-100h] BYREF
  _DWORD v158[31]; // [esp+1FCh] [ebp-FCh]
  int v159; // [esp+278h] [ebp-80h]
  _DWORD v160[31]; // [esp+27Ch] [ebp-7Ch]

  if ( *(_DWORD *)(this + 228) < (unsigned int)*(unsigned __int16 *)(this + 296) )
  {
    *(_DWORD *)(this + 216) = 0;
    return 0;
  }
  v156[12] = 36;
  v156[24] = 36;
  v156[0] = 34;
  v156[4] = 33;
  v156[6] = 31;
  v156[8] = 31;
  v156[10] = 33;
  v156[19] = 43;
  v156[27] = 43;
  v156[30] = 34;
  v124 = 34;
  v132 = 34;
  v134 = 43;
  v139 = 34;
  v146 = 43;
  v156[29] = 46;
  v149 = 46;
  v151 = 46;
  v155 = 40;
  v156[1] = 42;
  v156[2] = 35;
  v156[3] = 49;
  v156[5] = 59;
  v156[7] = 65;
  v156[9] = 62;
  v156[11] = 54;
  v156[13] = 41;
  v156[14] = 44;
  v156[15] = 47;
  v156[16] = 48;
  v156[17] = 47;
  v156[18] = 45;
  v156[20] = 37;
  v156[21] = 41;
  v156[22] = 38;
  v156[23] = 40;
  v156[25] = 42;
  v156[26] = 37;
  v156[28] = 35;
  v125[0] = 37;
  v125[1] = 47;
  v126 = 38;
  v127 = 62;
  v128 = 48;
  v129 = 73;
  v130 = 41;
  v131 = 76;
  v133 = 72;
  v135 = 58;
  v136 = 53;
  v137 = 50;
  v138 = 48;
  v140 = 49;
  v141 = 45;
  v142 = 49;
  v143 = 55;
  v144 = 49;
  v145 = 56;
  v147 = 48;
  v148 = 35;
  v150 = 40;
  v152 = 47;
  v153 = 47;
  v154 = 38;
  v92 = 6;
  v93 = 4;
  v94 = 26;
  v95 = 0;
  v96 = 44;
  HIWORD(v4) = 0;
  v104 = 48;
  v107 = 48;
  v109 = 49;
  v111 = 47;
  v160[11] = 48;
  v160[16] = 49;
  v160[18] = 47;
  HIWORD(v3) = 0;
  v97 = 2;
  v98 = 58;
  v99 = 6;
  v100 = 56;
  v101 = 6;
  v102 = 52;
  v103 = 10;
  v105 = 30;
  v106 = 36;
  v108 = 10;
  v110 = 6;
  v112 = 4;
  v113 = 30;
  v114 = 2;
  v115 = 20;
  v116 = 0;
  v117 = 10;
  v118 = 2;
  v119 = 4;
  v120 = 3;
  v121 = 0;
  v122 = 4;
  v123 = -1;
  v159 = 14;
  v160[0] = 4;
  v160[1] = 28;
  v160[2] = 0;
  v160[3] = 44;
  v160[4] = 4;
  v160[5] = 58;
  v160[6] = 6;
  v160[7] = 56;
  v160[8] = 12;
  v160[9] = 52;
  v160[10] = 22;
  v160[12] = 32;
  v160[13] = 36;
  v160[14] = 40;
  v160[15] = 18;
  v160[17] = 2;
  v160[19] = 1;
  v160[20] = 30;
  v160[21] = 2;
  v160[22] = 20;
  v160[23] = 0;
  v160[24] = 14;
  v160[25] = 2;
  v160[26] = 8;
  v160[27] = 3;
  v160[28] = 6;
  v160[29] = 1;
  v160[30] = -1;
  v157 = 30;
  v158[0] = 4;
  v158[1] = 50;
  v158[2] = 15;
  v158[3] = 60;
  v158[4] = 15;
  v158[5] = 68;
  v158[6] = 20;
  v158[7] = 75;
  v158[8] = 25;
  v158[9] = 62;
  v158[10] = 45;
  v158[11] = 53;
  v158[12] = 42;
  v158[13] = 36;
  v158[14] = 52;
  v158[15] = 32;
  v158[16] = 80;
  v158[17] = 18;
  v85 = 2;
  LOBYTE(v4) = *(_BYTE *)(this + 4);
  v158[18] = 80;
  v158[19] = 11;
  v158[20] = 60;
  v158[21] = 12;
  v158[22] = 40;
  v158[23] = 20;
  v158[24] = 24;
  v158[25] = -2;
  v158[26] = 8;
  v158[27] = 3;
  v158[28] = 6;
  v158[29] = 20;
  v158[30] = 10;
  v75 = 8;
  v76 = 8;
  v77 = 1;
  v78 = 1;
  v79 = 1;
  v80 = 1;
  v81 = 1;
  v82 = 1;
  v83 = 1;
  v84 = 1;
  v86 = 3;
  v87 = 1;
  v88 = 1;
  v89 = 1;
  v90 = 1;
  if ( (_BYTE)v4 )
    return 0;
  LOWORD(v3) = *(_WORD *)(this + 300);
  LOWORD(v4) = *(_WORD *)(this + 298);
  v5 = sub_47F3A0(v4, v3, *(_DWORD *)(this + 252));
  v6 = *(_DWORD *)(this + 252);
  v91 = v5;
  if ( !sub_491C70(v6) )
    return 0;
  if ( !*(_DWORD *)(this + 12) )
  {
    v7 = *(_WORD *)(this + 16);
    switch ( v7 )
    {
      case 5:
      case 113:
        v25 = *(_DWORD *)(*(_DWORD *)(this + 156) + 60);
        v26 = 2 * *(unsigned __int8 *)(this + 288);
        v49 = (float)(*(_DWORD *)(this + 160) + v156[v26 - 1] + *(__int16 *)(v25 + 4));
        v68 = (float)(*(_DWORD *)(this + 164) + v156[v26] + *(__int16 *)(v25 + 6));
        sub_4685F0(LODWORD(v49), LODWORD(v68), 0);
        v27 = *(_DWORD *)(*(_DWORD *)(this + 156) + 60);
        v28 = 2 * *(unsigned __int8 *)(this + 288);
        v50 = (float)(*(_DWORD *)(this + 160) + v156[v28 - 1] + *(__int16 *)(v27 + 4));
        v69 = (float)(*(_DWORD *)(this + 164) + v156[v28] + *(__int16 *)(v27 + 6));
        sub_469C70(LODWORD(v50), LODWORD(v69), 0);
        break;
      case 1:
        v8 = *(_DWORD *)(*(_DWORD *)(this + 156) + 60);
        v9 = 2 * *(unsigned __int8 *)(this + 288);
        v39 = (float)(*(_DWORD *)(this + 160) + v125[v9 - 1] + *(__int16 *)(v8 + 4));
        v58 = (float)(*(_DWORD *)(this + 164) + v125[v9] + *(__int16 *)(v8 + 6));
        sub_4685F0(LODWORD(v39), LODWORD(v58), 0);
        v10 = *(_DWORD *)(*(_DWORD *)(this + 156) + 60);
        v11 = 2 * *(unsigned __int8 *)(this + 288);
        v40 = (float)(*(_DWORD *)(this + 160) + v125[v11 - 1] + *(__int16 *)(v10 + 4));
        v59 = (float)(*(_DWORD *)(this + 164) + v125[v11] + *(__int16 *)(v10 + 6));
        sub_469C70(LODWORD(v40), LODWORD(v59), 0);
        break;
      case 67:
        v12 = *(_DWORD *)(*(_DWORD *)(this + 156) + 60);
        v13 = 2 * *(unsigned __int8 *)(this + 288);
        v41 = (float)(*(_DWORD *)(this + 160) + *(__int16 *)(v12 + 4) + v158[v13 - 1]);
        v60 = (float)(*(_DWORD *)(this + 164) + *(__int16 *)(v12 + 6) + v158[v13]);
        sub_468C40(LODWORD(v41), LODWORD(v60), 0);
        v14 = *(_DWORD *)(*(_DWORD *)(this + 156) + 60);
        v15 = 2 * *(unsigned __int8 *)(this + 288);
        v42 = (float)(*(_DWORD *)(this + 160) + *(__int16 *)(v14 + 4) + v158[v15 - 1]);
        v61 = (float)(*(_DWORD *)(this + 164) + *(__int16 *)(v14 + 6) + v158[v15]);
        sub_46AC00(LODWORD(v42), LODWORD(v61), 0);
        break;
      case 39:
        v16 = *(_DWORD *)(*(_DWORD *)(this + 156) + 60);
        v17 = 8 * *(unsigned __int8 *)(this + 288);
        v43 = (float)(*(_DWORD *)(this + 160) + *(__int16 *)(v16 + 4) + *(int *)((char *)&v92 + v17));
        v62 = (float)(*(_DWORD *)(this + 164) + *(__int16 *)(v16 + 6) + *(int *)((char *)&v93 + v17));
        sub_46A120(LODWORD(v43), LODWORD(v62), 0);
        break;
      case 40:
        v18 = *(_DWORD *)(*(_DWORD *)(this + 156) + 60);
        v44 = (float)(*(__int16 *)(v18 + 4) + *(_DWORD *)(this + 160) + 6);
        v63 = (float)(*(__int16 *)(v18 + 6) + *(_DWORD *)(this + 164) + 4);
        sub_46A2C0(LODWORD(v44), LODWORD(v63), 0);
        v19 = *(_DWORD *)(*(_DWORD *)(this + 156) + 60);
        v45 = (float)(*(__int16 *)(v19 + 4) + *(_DWORD *)(this + 160) + 6);
        v64 = (float)(*(__int16 *)(v19 + 6) + *(_DWORD *)(this + 164) + 4);
        sub_468670(v45, v64, 0);
        break;
      case 37:
        v20 = *(_DWORD *)(*(_DWORD *)(this + 156) + 60);
        v46 = (float)(*(__int16 *)(v20 + 4) + *(_DWORD *)(this + 160) + 40);
        v65 = (float)(*(__int16 *)(v20 + 6) + *(_DWORD *)(this + 164) + 40);
        sub_468870(v46, v65, 0);
        break;
      case 41:
        v21 = *(_DWORD *)(*(_DWORD *)(this + 156) + 60);
        v22 = 2 * *(unsigned __int8 *)(this + 288);
        v47 = (float)(*(_DWORD *)(this + 160) + *(__int16 *)(v21 + 4) + v160[v22 - 1]);
        v66 = (float)(*(_DWORD *)(this + 164) + *(__int16 *)(v21 + 6) + v160[v22]);
        sub_46A460(LODWORD(v47), LODWORD(v66), 0);
        break;
      case 594:
        v23 = *(_DWORD *)(*(_DWORD *)(this + 156) + 60);
        v24 = 8 * *(unsigned __int8 *)(this + 288);
        v48 = (float)(*(_DWORD *)(this + 160) + *(__int16 *)(v23 + 4) + *(int *)((char *)&v75 + v24));
        v67 = (float)(*(_DWORD *)(this + 164) + *(__int16 *)(v23 + 6) + *(int *)((char *)&v76 + v24));
        sub_46B270(LODWORD(v48), LODWORD(v67), 0);
        break;
    }
  }
  v29 = *(_WORD *)(this + 16);
  switch ( v29 )
  {
    case 'H':
      if ( *(_DWORD *)(this + 252) < (unsigned int)(*(_DWORD *)(this + 248) - 5) )
      {
        v38 = (double)sub_401710(0, 20) + (double)(*(_DWORD *)(this + 164) + 16);
        v51 = (double)sub_401710(-10, 10) + (double)(*(_DWORD *)(this + 160) + 24);
        sub_46A910(LODWORD(v51), LODWORD(v38), 0);
        v52 = (float)(*(_DWORD *)(this + 160) + 24);
        v70 = (float)(*(_DWORD *)(this + 164) + 16);
        sub_467CC0(LODWORD(v52), LODWORD(v70), 0);
      }
      break;
    case 'J':
      if ( *(_DWORD *)(this + 252) < (unsigned int)(*(_DWORD *)(this + 248) - 5) )
      {
        v53 = (float)(*(_DWORD *)(this + 160) + 24);
        v71 = (float)(*(_DWORD *)(this + 164) + 16);
        sub_46B0E0(LODWORD(v53), LODWORD(v71), 0);
        v54 = (float)(*(_DWORD *)(this + 160) + 24);
        v72 = (float)(*(_DWORD *)(this + 164) + 16);
        sub_467F00(LODWORD(v54), LODWORD(v72), 0);
      }
      break;
    case 'I':
      v37 = (double)sub_401710(0, 20) + (double)(*(_DWORD *)(this + 164) + 16);
      v55 = (double)sub_401710(-10, 10) + (double)(*(_DWORD *)(this + 160) + 24);
      sub_46AF50(LODWORD(v55), LODWORD(v37), 0);
      v56 = (float)(*(_DWORD *)(this + 160) + 24);
      v73 = (float)(*(_DWORD *)(this + 164) + 16);
      sub_467F80(LODWORD(v56), LODWORD(v73), 0);
      break;
  }
  if ( !dword_8FCC64 )
    return 0;
  v36 = (double)*(unsigned __int8 *)(this + 242) * flt_4AF3AC;
  v35 = (double)*(unsigned __int8 *)(this + 241) * flt_4AF3AC;
  v34 = (double)*(unsigned __int8 *)(this + 240) * flt_4AF3AC;
  sub_49239B((float *)&v124, v34, v35, v36, 1.0);
  *(float *)&v126 = (double)*(unsigned __int8 *)(this + 257) * flt_4AF3AC;
  (*(void (__stdcall **)(int, int *))(*(_DWORD *)dword_8FCC64 + 64))(dword_8FCC64, &v124);
  v30 = *(__int16 **)(*(_DWORD *)(this + 156) + 60);
  v57 = (double)*v30 * flt_4AF358 + (double)v30[2] + (double)*(int *)(this + 160) - flt_4AF48C;
  v74 = flt_4AF488 - ((double)v30[1] * flt_4AF358 + (double)v30[3] + (double)*(int *)(this + 164));
  if ( (*(int (__stdcall **)(int))(*(_DWORD *)dword_8FCC64 + 20))(dword_8FCC64) < 0 )
    return 0;
  (*(void (__stdcall **)(int, int, int *))(*(_DWORD *)dword_8FCC64 + 48))(dword_8FCC64, 1, &v157);
  v31 = *(_DWORD *)(this + 156);
  v82 = 0;
  *(float *)&v87 = v57;
  *(float *)&v88 = v74;
  v81 = 0;
  v79 = 0;
  v78 = 0;
  v77 = 0;
  v76 = 0;
  v86 = 0;
  v84 = 0;
  v83 = 0;
  v90 = 1065353216;
  v85 = 1065353216;
  v80 = 1065353216;
  v75 = 1065353216;
  v89 = 0;
  v32 = *(__int16 **)(v31 + 60);
  v33 = (double)v32[1];
  *(float *)&v92 = (float)*v32;
  v99 = 0;
  v98 = 0;
  *(float *)&v97 = v33;
  v96 = 0;
  v95 = 0;
  v94 = 0;
  v93 = 0;
  v106 = 0;
  v105 = 0;
  v104 = 0;
  v103 = 0;
  v101 = 0;
  v100 = 0;
  v107 = 1065353216;
  v102 = 1065353216;
  sub_492A9B(&v155, &v92, &v75);
  (*(void (__stdcall **)(int, int, int *))(*(_DWORD *)dword_8FCC64 + 44))(dword_8FCC64, 1, &v155);
  sub_492788((int *)dword_8FCC64, *(_BYTE *)(this + 256), (int)&v124);
  (*(void (__stdcall **)(int, _DWORD, int))(*(_DWORD *)dword_8FCC64 + 140))(dword_8FCC64, 0, v91);
  (*(void (__stdcall **)(int, int, int, int, int, _DWORD))(*(_DWORD *)dword_8FCC64 + 100))(
    dword_8FCC64,
    5,
    274,
    this + 28,
    4,
    0);
  sub_49295D(dword_8FCC64);
  (*(void (__stdcall **)(int, int, int *))(*(_DWORD *)dword_8FCC64 + 44))(dword_8FCC64, 1, &v157);
  (*(void (__stdcall **)(int))(*(_DWORD *)dword_8FCC64 + 24))(dword_8FCC64);
  return 1;
}
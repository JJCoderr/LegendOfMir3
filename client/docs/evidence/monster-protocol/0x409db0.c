_DWORD *__thiscall sub_409DB0(char *this, int a2)
{
  _DWORD *result; // eax
  int v4; // ecx
  int v5; // ebp
  char *v6; // eax
  char *v7; // ebx
  int (__thiscall **v8)(_DWORD, _DWORD); // edx
  _DWORD *v9; // eax
  int v10; // eax
  __int16 v11; // ax
  char *v12; // eax
  char *v13; // ebx
  int (__thiscall **v14)(_DWORD, _DWORD); // edx
  _DWORD *v15; // eax
  int v16; // eax
  char *v17; // eax
  char *v18; // ebx
  int (__thiscall **v19)(_DWORD, _DWORD); // edx
  char *v20; // eax
  int (__thiscall **v21)(_DWORD, _DWORD); // edx
  char *v22; // eax
  int (__thiscall **v23)(_DWORD, _DWORD); // edx
  char *v24; // eax
  int (__thiscall **v25)(_DWORD, _DWORD); // edx
  int v26; // eax
  unsigned __int16 v27; // ax
  char *v28; // eax
  char *v29; // ebx
  int (__thiscall **v30)(_DWORD, _DWORD); // edx
  _DWORD *v31; // eax
  char *v32; // eax
  int (__thiscall **v33)(_DWORD, _DWORD); // edx
  char *v34; // eax
  int (__thiscall **v35)(_DWORD, _DWORD); // edx
  char *v36; // eax
  int (__thiscall **v37)(_DWORD, _DWORD); // edx
  int v38; // ecx
  int v39; // eax
  char *v40; // eax
  int (__thiscall **v41)(_DWORD, _DWORD); // edx
  char *v42; // eax
  int (__thiscall **v43)(_DWORD, _DWORD); // edx
  char *v44; // eax
  int (__thiscall **v45)(_DWORD, _DWORD); // edx
  char *v46; // eax
  char *v47; // ebx
  int (__thiscall **v48)(_DWORD, _DWORD); // edx
  _DWORD *v49; // eax
  int v50; // eax
  int v51; // eax
  char *v52; // eax
  int (__thiscall **v53)(_DWORD, _DWORD); // edx
  int v54; // ecx
  int v55; // eax
  char *v56; // eax
  int (__thiscall **v57)(_DWORD, _DWORD); // edx
  char *v58; // eax
  int (__thiscall **v59)(_DWORD, _DWORD); // edx
  char *v60; // eax
  int (__thiscall **v61)(_DWORD, _DWORD); // edx
  int v62; // eax
  char *v63; // eax
  int (__thiscall **v64)(_DWORD, _DWORD); // edx
  char *v65; // eax
  int (__thiscall **v66)(_DWORD, _DWORD); // edx
  int v67; // ecx
  char *v68; // eax
  int (__thiscall **v69)(_DWORD, _DWORD); // eax
  char *v70; // eax
  int (__thiscall **v71)(_DWORD, _DWORD); // edx
  int v72; // [esp-Ch] [ebp-48h]
  int v73; // [esp-Ch] [ebp-48h]
  int v74; // [esp-Ch] [ebp-48h]
  int v75; // [esp-Ch] [ebp-48h]
  int v76; // [esp-Ch] [ebp-48h]
  int v77; // [esp-Ch] [ebp-48h]
  int v78; // [esp-Ch] [ebp-48h]
  int v79; // [esp-Ch] [ebp-48h]
  int v80; // [esp-Ch] [ebp-48h]
  int v81; // [esp-Ch] [ebp-48h]
  int v82; // [esp-Ch] [ebp-48h]
  int v83; // [esp-Ch] [ebp-48h]
  int v84; // [esp-Ch] [ebp-48h]
  int v85; // [esp-Ch] [ebp-48h]
  int v86; // [esp-Ch] [ebp-48h]
  int v87; // [esp-8h] [ebp-44h]
  int v88; // [esp-8h] [ebp-44h]
  int v89; // [esp-8h] [ebp-44h]
  int v90; // [esp-8h] [ebp-44h]
  int v91; // [esp-8h] [ebp-44h]
  int v92; // [esp-8h] [ebp-44h]
  int v93; // [esp-8h] [ebp-44h]
  int v94; // [esp-8h] [ebp-44h]
  int v95; // [esp-8h] [ebp-44h]
  int v96; // [esp-8h] [ebp-44h]
  int v97; // [esp-8h] [ebp-44h]
  int v98; // [esp-8h] [ebp-44h]
  int v99; // [esp-8h] [ebp-44h]
  int v100; // [esp-8h] [ebp-44h]
  int v101; // [esp-8h] [ebp-44h]
  int v102; // [esp-4h] [ebp-40h]
  int v103; // [esp-4h] [ebp-40h]
  int v104; // [esp-4h] [ebp-40h]
  int v105; // [esp-4h] [ebp-40h]
  int v106; // [esp-4h] [ebp-40h]
  int v107; // [esp-4h] [ebp-40h]
  int v108; // [esp-4h] [ebp-40h]
  int v109; // [esp+0h] [ebp-3Ch]
  int v110; // [esp+0h] [ebp-3Ch]
  int v111; // [esp+0h] [ebp-3Ch]
  int v112; // [esp+0h] [ebp-3Ch]
  int v113; // [esp+0h] [ebp-3Ch]
  int v114; // [esp+0h] [ebp-3Ch]
  int v115; // [esp+0h] [ebp-3Ch]
  int v116; // [esp+8h] [ebp-34h]
  int v117; // [esp+8h] [ebp-34h]
  int v118; // [esp+8h] [ebp-34h]
  int v119; // [esp+20h] [ebp-1Ch] BYREF
  int v120; // [esp+24h] [ebp-18h]
  int v121; // [esp+28h] [ebp-14h]
  _DWORD *v122; // [esp+2Ch] [ebp-10h]
  int v123; // [esp+38h] [ebp-4h]

  sub_47D780((const char *)(a2 + 12), (int)&v119, 16);
  result = v122;
  v4 = v121;
  v5 = v121;
  if ( v122 == (_DWORD *)1 )
  {
    *((_DWORD *)this + 33) = *(unsigned __int16 *)(a2 + 6);
    *((_DWORD *)this + 34) = *(unsigned __int16 *)(a2 + 8);
    this[121] = *(_BYTE *)(a2 + 10);
    v6 = (char *)MEMORY[0x4A01AA](0x240u);
    v123 = 0;
    if ( v6 )
      v7 = sub_449480(v6);
    else
      v7 = 0;
    v8 = *(int (__thiscall ***)(_DWORD, _DWORD))v7;
    v109 = *((_DWORD *)this + 34);
    v102 = *((_DWORD *)this + 33);
    v123 = -1;
    v9 = (_DWORD *)((int (__thiscall *)(char *, int, int, int, int, int, char *, int, _DWORD))v8[1])(
                     v7,
                     515,
                     v102,
                     v109,
                     v102,
                     v109,
                     this,
                     v5,
                     0);
    if ( v9 )
    {
      if ( dword_59A9F8 )
      {
        v9 = MEMORY[0x4A01AA](0x10u);
        if ( v9 )
        {
          *v9 = &off_4AF460;
          v9[1] = 0;
          v9[3] = 0;
          v9[2] = 0;
          v9[1] = v7;
          v9[3] = 0;
          v9[2] = dword_59A9FC;
          *(_DWORD *)(dword_59A9FC + 12) = v9;
          goto LABEL_14;
        }
      }
      else
      {
        v9 = MEMORY[0x4A01AA](0x10u);
        if ( v9 )
        {
          *v9 = &off_4AF460;
          v9[1] = 0;
          v9[3] = 0;
          v9[2] = 0;
        }
        else
        {
          v9 = 0;
        }
        dword_59A9F8 = (int)v9;
        if ( v9 )
        {
          v9[1] = v7;
          v9 = (_DWORD *)dword_59A9F8;
LABEL_14:
          dword_59A9FC = (int)v9;
          v10 = ++dword_59AA08;
          LOBYTE(v10) = this[121];
          return (_DWORD *)(*(int (__thiscall **)(char *, int, int, _DWORD))(*(_DWORD *)this + 16))(this, 2, v10, 0);
        }
      }
    }
    else if ( v7 )
    {
      v9 = (_DWORD *)(**(int (__thiscall ***)(char *, int))v7)(v7, 1);
    }
    LOBYTE(v9) = this[121];
    return (_DWORD *)(*(int (__thiscall **)(char *, int, _DWORD *, _DWORD))(*(_DWORD *)this + 16))(this, 2, v9, 0);
  }
  if ( v122 == (_DWORD *)2 )
  {
    *((_DWORD *)this + 33) = *(unsigned __int16 *)(a2 + 6);
    *((_DWORD *)this + 34) = *(unsigned __int16 *)(a2 + 8);
    this[121] = *(_BYTE *)(a2 + 10);
    if ( this[69] == 3 )
      v11 = *(_WORD *)(this + 71);
    else
      v11 = (unsigned __int8)this[71];
    if ( v11 == 13 )
    {
      v12 = (char *)MEMORY[0x4A01AA](0x240u);
      v123 = 1;
      if ( v12 )
        v13 = sub_449480(v12);
      else
        v13 = 0;
      v14 = *(int (__thiscall ***)(_DWORD, _DWORD))v13;
      v87 = *((_DWORD *)this + 34);
      v72 = *((_DWORD *)this + 33);
      v123 = -1;
      v15 = (_DWORD *)((int (__thiscall *)(char *, int, int, int, int, int, char *, int, int))v14[1])(
                        v13,
                        538,
                        v72,
                        v87,
                        v119,
                        v120,
                        this,
                        v5,
                        500);
      if ( v15 )
      {
        if ( !dword_59A9F8 )
        {
          v15 = MEMORY[0x4A01AA](0x10u);
          if ( v15 )
          {
            *v15 = &off_4AF460;
            v15[1] = 0;
            v15[3] = 0;
            v15[2] = 0;
          }
          else
          {
            v15 = 0;
          }
          dword_59A9F8 = (int)v15;
          if ( v15 )
          {
            v15[1] = v13;
            dword_59A9FC = dword_59A9F8;
            v16 = ++dword_59AA08;
            LOBYTE(v16) = this[121];
            return (_DWORD *)(*(int (__thiscall **)(char *, int, int, _DWORD))(*(_DWORD *)this + 16))(this, 6, v16, 0);
          }
          goto LABEL_185;
        }
LABEL_180:
        v15 = MEMORY[0x4A01AA](0x10u);
        if ( v15 )
        {
          *v15 = &off_4AF460;
          v15[1] = 0;
          v15[3] = 0;
          v15[2] = 0;
          v15[1] = v13;
          v15[3] = 0;
          v15[2] = dword_59A9FC;
          *(_DWORD *)(dword_59A9FC + 12) = v15;
          goto LABEL_182;
        }
        goto LABEL_185;
      }
LABEL_183:
      if ( v13 )
        v15 = (_DWORD *)(**(int (__thiscall ***)(char *, int))v13)(v13, 1);
      goto LABEL_185;
    }
    v17 = (char *)MEMORY[0x4A01AA](0x240u);
    v123 = 2;
    if ( v17 )
      v18 = sub_449480(v17);
    else
      v18 = 0;
    v19 = *(int (__thiscall ***)(_DWORD, _DWORD))v18;
    v88 = *((_DWORD *)this + 34);
    v73 = *((_DWORD *)this + 33);
    v123 = -1;
    result = (_DWORD *)((int (__thiscall *)(char *, int, int, int, int, int, char *, int, _DWORD))v19[1])(
                         v18,
                         538,
                         v73,
                         v88,
                         v119,
                         v120,
                         this,
                         v5,
                         0);
    if ( result )
    {
LABEL_37:
      if ( !dword_59A9F8 )
      {
        result = MEMORY[0x4A01AA](0x10u);
        if ( result )
        {
          *result = &off_4AF460;
          result[1] = 0;
          result[3] = 0;
          result[2] = 0;
          goto LABEL_201;
        }
        goto LABEL_200;
      }
      goto LABEL_203;
    }
LABEL_40:
    if ( v18 )
      return (_DWORD *)(**(int (__thiscall ***)(char *, int))v18)(v18, 1);
    return result;
  }
  if ( v122 == (_DWORD *)3 )
  {
    *((_DWORD *)this + 33) = *(unsigned __int16 *)(a2 + 6);
    *((_DWORD *)this + 34) = *(unsigned __int16 *)(a2 + 8);
    this[121] = *(_BYTE *)(a2 + 10);
    v20 = (char *)MEMORY[0x4A01AA](0x240u);
    v123 = 3;
    if ( v20 )
      v18 = sub_449480(v20);
    else
      v18 = 0;
    v21 = *(int (__thiscall ***)(_DWORD, _DWORD))v18;
    v110 = *((_DWORD *)this + 34);
    v103 = *((_DWORD *)this + 33);
    v123 = -1;
    result = (_DWORD *)((int (__thiscall *)(char *, int, int, int, int, int, char *, _DWORD, _DWORD))v21[1])(
                         v18,
                         539,
                         v103,
                         v110,
                         v103,
                         v110,
                         this,
                         0,
                         0);
LABEL_47:
    if ( !result )
      goto LABEL_40;
    if ( !dword_59A9F8 )
    {
      result = MEMORY[0x4A01AA](0x10u);
      if ( result )
      {
        *result = &off_4AF460;
        result[1] = 0;
        result[3] = 0;
        result[2] = 0;
      }
      else
      {
        result = 0;
      }
      dword_59A9F8 = (int)result;
      if ( !result )
        return result;
      result[1] = v18;
      result = (_DWORD *)dword_59A9F8;
LABEL_205:
      dword_59A9FC = (int)result;
      return (_DWORD *)++dword_59AA08;
    }
    goto LABEL_203;
  }
  if ( v122 == (_DWORD *)4 )
  {
    this[121] = *(_BYTE *)(a2 + 10);
    v22 = (char *)MEMORY[0x4A01AA](0x240u);
    v123 = 4;
    if ( v22 )
      v18 = sub_449480(v22);
    else
      v18 = 0;
    v23 = *(int (__thiscall ***)(_DWORD, _DWORD))v18;
    v89 = *((_DWORD *)this + 34);
    v74 = *((_DWORD *)this + 33);
    v123 = -1;
    result = (_DWORD *)((int (__thiscall *)(char *, int, int, int, int, int, char *, int, _DWORD))v23[1])(
                         v18,
                         543,
                         v74,
                         v89,
                         v119,
                         v120,
                         this,
                         v5,
                         0);
    if ( !result )
      goto LABEL_40;
    if ( !dword_59A9F8 )
    {
      result = MEMORY[0x4A01AA](0x10u);
      if ( result )
      {
        *result = &off_4AF460;
        result[1] = 0;
        result[3] = 0;
        result[2] = 0;
        goto LABEL_201;
      }
      goto LABEL_200;
    }
LABEL_203:
    result = MEMORY[0x4A01AA](0x10u);
    if ( !result )
      return result;
    *result = &off_4AF460;
    result[1] = 0;
    result[3] = 0;
    result[2] = 0;
    result[1] = v18;
    result[3] = 0;
    result[2] = dword_59A9FC;
    *(_DWORD *)(dword_59A9FC + 12) = result;
    goto LABEL_205;
  }
  if ( v122 == (_DWORD *)5 )
  {
    *((_DWORD *)this + 33) = *(unsigned __int16 *)(a2 + 6);
    *((_DWORD *)this + 34) = *(unsigned __int16 *)(a2 + 8);
    this[121] = *(_BYTE *)(a2 + 10);
    v24 = (char *)MEMORY[0x4A01AA](0x240u);
    v123 = 5;
    if ( v24 )
      v18 = sub_449480(v24);
    else
      v18 = 0;
    v25 = *(int (__thiscall ***)(_DWORD, _DWORD))v18;
    v90 = *((_DWORD *)this + 34);
    v75 = *((_DWORD *)this + 33);
    v123 = -1;
    result = (_DWORD *)((int (__thiscall *)(char *, int, int, int, int, int, char *, int, _DWORD))v25[1])(
                         v18,
                         23,
                         v75,
                         v90,
                         v119,
                         v120,
                         this,
                         v5,
                         0);
    if ( result )
      goto LABEL_37;
    goto LABEL_40;
  }
  if ( v122 == (_DWORD *)6 )
  {
    v26 = v120;
    this[121] = *(_BYTE *)(a2 + 10);
    return (_DWORD *)sub_40DD80((int *)this, 9, v4, v119, v26);
  }
  if ( v122 == (_DWORD *)7 )
  {
    v27 = *(_WORD *)(a2 + 6);
    *((_DWORD *)this + 33) = v27;
    *((_DWORD *)this + 34) = *(unsigned __int16 *)(a2 + 8);
    LOBYTE(v27) = *(_BYTE *)(a2 + 10);
    this[121] = v27;
    sub_40DB10((int)this, (int)dword_59AA6C, 550, v27, -1);
    *((_DWORD *)this + 100060) = 1;
    v28 = (char *)MEMORY[0x4A01AA](0x240u);
    v123 = 6;
    if ( v28 )
      v29 = sub_449480(v28);
    else
      v29 = 0;
    v30 = *(int (__thiscall ***)(_DWORD, _DWORD))v29;
    v91 = *((_DWORD *)this + 34);
    v76 = *((_DWORD *)this + 33);
    v123 = -1;
    v31 = (_DWORD *)((int (__thiscall *)(char *, int, int, int, int, int, char *, int, int))v30[1])(
                      v29,
                      576,
                      v76,
                      v91,
                      v119,
                      v120,
                      this,
                      v5,
                      500);
    if ( v31 )
    {
      if ( !dword_59A9F8 )
      {
        v31 = MEMORY[0x4A01AA](0x10u);
        if ( v31 )
        {
          *v31 = &off_4AF460;
          v31[1] = 0;
          v31[3] = 0;
          v31[2] = 0;
          goto LABEL_79;
        }
        goto LABEL_78;
      }
LABEL_165:
      v31 = MEMORY[0x4A01AA](0x10u);
      if ( v31 )
      {
        *v31 = &off_4AF460;
        v31[1] = 0;
        v31[3] = 0;
        v31[2] = 0;
        v31[1] = v29;
        v31[3] = 0;
        v31[2] = dword_59A9FC;
        *(_DWORD *)(dword_59A9FC + 12) = v31;
        dword_59A9FC = (int)v31;
        ++dword_59AA08;
      }
      goto LABEL_222;
    }
LABEL_167:
    if ( v29 )
      goto LABEL_221;
    goto LABEL_222;
  }
  if ( v122 == (_DWORD *)8 )
  {
    *((_DWORD *)this + 33) = *(unsigned __int16 *)(a2 + 6);
    *((_DWORD *)this + 34) = *(unsigned __int16 *)(a2 + 8);
    this[121] = *(_BYTE *)(a2 + 10);
    v32 = (char *)MEMORY[0x4A01AA](0x240u);
    v123 = 7;
    if ( v32 )
      v29 = sub_449480(v32);
    else
      v29 = 0;
    v33 = *(int (__thiscall ***)(_DWORD, _DWORD))v29;
    v92 = *((_DWORD *)this + 34);
    v77 = *((_DWORD *)this + 33);
    v123 = -1;
    v31 = (_DWORD *)((int (__thiscall *)(char *, int, int, int, int, int, char *, int, int))v33[1])(
                      v29,
                      538,
                      v77,
                      v92,
                      v119,
                      v120,
                      this,
                      v5,
                      500);
    goto LABEL_86;
  }
  if ( v122 == (_DWORD *)9 )
  {
    *((_DWORD *)this + 33) = *(unsigned __int16 *)(a2 + 6);
    *((_DWORD *)this + 34) = *(unsigned __int16 *)(a2 + 8);
    this[121] = *(_BYTE *)(a2 + 10);
    v34 = (char *)MEMORY[0x4A01AA](0x240u);
    v123 = 8;
    if ( v34 )
      v18 = sub_449480(v34);
    else
      v18 = 0;
    v35 = *(int (__thiscall ***)(_DWORD, _DWORD))v18;
    v93 = *((_DWORD *)this + 34);
    v78 = *((_DWORD *)this + 33);
    v123 = -1;
    result = (_DWORD *)((int (__thiscall *)(char *, int, int, int, int, int, char *, int, _DWORD))v35[1])(
                         v18,
                         555,
                         v78,
                         v93,
                         v119,
                         v120,
                         this,
                         v5,
                         0);
    if ( result )
      goto LABEL_37;
    goto LABEL_40;
  }
  if ( v122 == (_DWORD *)10 || v122 == (_DWORD *)11 )
  {
    *((_DWORD *)this + 33) = *(unsigned __int16 *)(a2 + 6);
    *((_DWORD *)this + 34) = *(unsigned __int16 *)(a2 + 8);
    this[121] = *(_BYTE *)(a2 + 10);
    v70 = (char *)MEMORY[0x4A01AA](0x240u);
    v123 = 9;
    if ( v70 )
      v29 = sub_449480(v70);
    else
      v29 = 0;
    v71 = *(int (__thiscall ***)(_DWORD, _DWORD))v29;
    v101 = *((_DWORD *)this + 34);
    v86 = *((_DWORD *)this + 33);
    v123 = -1;
    v31 = (_DWORD *)((int (__thiscall *)(char *, int, int, int, int, int, char *, int, int))v71[1])(
                      v29,
                      590,
                      v86,
                      v101,
                      v119,
                      v120,
                      this,
                      v5,
                      400);
    if ( !v31 )
    {
      if ( v29 )
LABEL_221:
        v31 = (_DWORD *)(**(int (__thiscall ***)(char *, int))v29)(v29, 1);
LABEL_222:
      LOBYTE(v31) = this[121];
      return (_DWORD *)(*(int (__thiscall **)(char *, int, _DWORD *, _DWORD))(*(_DWORD *)this + 16))(this, 5, v31, 0);
    }
    if ( dword_59A9F8 )
      goto LABEL_165;
    v31 = MEMORY[0x4A01AA](0x10u);
    if ( !v31 )
      goto LABEL_78;
    *v31 = &off_4AF460;
    v31[1] = 0;
    v31[3] = 0;
    v31[2] = 0;
    goto LABEL_79;
  }
  if ( v122 == (_DWORD *)12 )
  {
    *((_DWORD *)this + 33) = *(unsigned __int16 *)(a2 + 6);
    *((_DWORD *)this + 34) = *(unsigned __int16 *)(a2 + 8);
    this[121] = *(_BYTE *)(a2 + 10);
    v36 = (char *)MEMORY[0x4A01AA](0x240u);
    v123 = 10;
    if ( v36 )
      v29 = sub_449480(v36);
    else
      v29 = 0;
    v37 = *(int (__thiscall ***)(_DWORD, _DWORD))v29;
    v94 = *((_DWORD *)this + 34);
    v79 = *((_DWORD *)this + 33);
    v123 = -1;
    v31 = (_DWORD *)((int (__thiscall *)(char *, int, int, int, int, int, char *, int, _DWORD))v37[1])(
                      v29,
                      581,
                      v79,
                      v94,
                      v119,
                      v120,
                      this,
                      v5,
                      0);
    if ( !v31 )
      goto LABEL_167;
    if ( dword_59A9F8 )
      goto LABEL_165;
    v31 = MEMORY[0x4A01AA](0x10u);
    if ( !v31 )
      goto LABEL_78;
    *v31 = &off_4AF460;
    v31[1] = 0;
    v31[3] = 0;
    v31[2] = 0;
LABEL_79:
    dword_59A9F8 = (int)v31;
    if ( v31 )
    {
      v31[1] = v29;
      v31 = (_DWORD *)(dword_59AA08 + 1);
      dword_59A9FC = dword_59A9F8;
      ++dword_59AA08;
    }
    goto LABEL_222;
  }
  if ( v122 == (_DWORD *)13 )
  {
    *((_DWORD *)this + 33) = *(unsigned __int16 *)(a2 + 6);
    *((_DWORD *)this + 34) = *(unsigned __int16 *)(a2 + 8);
    v116 = *(unsigned __int8 *)(a2 + 10);
    this[121] = v116;
    sub_40DB10((int)this, (int)dword_59AA6C, 579, v116, -1);
    LOBYTE(v38) = this[121];
    v39 = *(_DWORD *)this;
    *((_DWORD *)this + 100060) = 1;
    return (_DWORD *)(*(int (__thiscall **)(char *, int, int, _DWORD))(v39 + 16))(this, 6, v38, 0);
  }
  if ( v122 == (_DWORD *)14 )
  {
    *((_DWORD *)this + 33) = *(unsigned __int16 *)(a2 + 6);
    *((_DWORD *)this + 34) = *(unsigned __int16 *)(a2 + 8);
    this[121] = *(_BYTE *)(a2 + 10);
    v40 = (char *)MEMORY[0x4A01AA](0x240u);
    v123 = 11;
    if ( v40 )
      v13 = sub_449480(v40);
    else
      v13 = 0;
    v41 = *(int (__thiscall ***)(_DWORD, _DWORD))v13;
    v111 = *((_DWORD *)this + 34);
    v104 = *((_DWORD *)this + 33);
    v123 = -1;
    v15 = (_DWORD *)((int (__thiscall *)(char *, int, int, int, int, int, char *, _DWORD, _DWORD))v41[1])(
                      v13,
                      524,
                      v104,
                      v111,
                      v104,
                      v111,
                      this,
                      0,
                      0);
    if ( !v15 )
      goto LABEL_183;
    if ( dword_59A9F8 )
      goto LABEL_180;
    v15 = MEMORY[0x4A01AA](0x10u);
    if ( v15 )
    {
      *v15 = &off_4AF460;
      v15[1] = 0;
      v15[3] = 0;
      v15[2] = 0;
      goto LABEL_178;
    }
    goto LABEL_177;
  }
  if ( v122 == (_DWORD *)15 )
  {
    *((_DWORD *)this + 33) = *(unsigned __int16 *)(a2 + 6);
    *((_DWORD *)this + 34) = *(unsigned __int16 *)(a2 + 8);
    this[121] = *(_BYTE *)(a2 + 10);
    v42 = (char *)MEMORY[0x4A01AA](0x240u);
    v123 = 12;
    if ( v42 )
      v29 = sub_449480(v42);
    else
      v29 = 0;
    v43 = *(int (__thiscall ***)(_DWORD, _DWORD))v29;
    v95 = *((_DWORD *)this + 34);
    v80 = *((_DWORD *)this + 33);
    v123 = -1;
    v31 = (_DWORD *)((int (__thiscall *)(char *, int, int, int, int, int, char *, int, _DWORD))v43[1])(
                      v29,
                      592,
                      v80,
                      v95,
                      v119,
                      v120,
                      this,
                      v5,
                      0);
    goto LABEL_86;
  }
  if ( v122 == (_DWORD *)16 )
  {
    *((_DWORD *)this + 33) = *(unsigned __int16 *)(a2 + 6);
    *((_DWORD *)this + 34) = *(unsigned __int16 *)(a2 + 8);
    this[121] = *(_BYTE *)(a2 + 10);
    v44 = (char *)MEMORY[0x4A01AA](0x240u);
    v123 = 13;
    if ( v44 )
      v13 = sub_449480(v44);
    else
      v13 = 0;
    v45 = *(int (__thiscall ***)(_DWORD, _DWORD))v13;
    v112 = *((_DWORD *)this + 34);
    v105 = *((_DWORD *)this + 33);
    v123 = -1;
    v15 = (_DWORD *)((int (__thiscall *)(char *, int, int, int, int, int, char *, int, _DWORD))v45[1])(
                      v13,
                      584,
                      v105,
                      v112,
                      v105,
                      v112,
                      this,
                      -1,
                      0);
    if ( !v15 )
      goto LABEL_183;
    if ( dword_59A9F8 )
      goto LABEL_180;
    v15 = MEMORY[0x4A01AA](0x10u);
    if ( v15 )
    {
      *v15 = &off_4AF460;
      v15[1] = 0;
      v15[3] = 0;
      v15[2] = 0;
      goto LABEL_178;
    }
    goto LABEL_177;
  }
  if ( v122 == (_DWORD *)17 )
  {
    *((_DWORD *)this + 33) = *(unsigned __int16 *)(a2 + 6);
    *((_DWORD *)this + 34) = *(unsigned __int16 *)(a2 + 8);
    this[121] = *(_BYTE *)(a2 + 10);
    v46 = (char *)MEMORY[0x4A01AA](0x240u);
    v123 = 14;
    if ( v46 )
      v47 = sub_449480(v46);
    else
      v47 = 0;
    v48 = *(int (__thiscall ***)(_DWORD, _DWORD))v47;
    v96 = *((_DWORD *)this + 34);
    v81 = *((_DWORD *)this + 33);
    v123 = -1;
    v49 = (_DWORD *)((int (__thiscall *)(char *, int, int, int, int, int, char *, int, int))v48[1])(
                      v47,
                      576,
                      v81,
                      v96,
                      v119,
                      v120,
                      this,
                      v5,
                      500);
    if ( v49 )
    {
      if ( dword_59A9F8 )
      {
        v49 = MEMORY[0x4A01AA](0x10u);
        if ( v49 )
        {
          *v49 = &off_4AF460;
          v49[1] = 0;
          v49[3] = 0;
          v49[2] = 0;
          v49[1] = v47;
          v49[3] = 0;
          v49[2] = dword_59A9FC;
          *(_DWORD *)(dword_59A9FC + 12) = v49;
          dword_59A9FC = (int)v49;
          v51 = ++dword_59AA08;
          LOBYTE(v51) = this[121];
          return (_DWORD *)(*(int (__thiscall **)(char *, int, int, _DWORD))(*(_DWORD *)this + 16))(this, 7, v51, 0);
        }
      }
      else
      {
        v49 = MEMORY[0x4A01AA](0x10u);
        if ( v49 )
        {
          *v49 = &off_4AF460;
          v49[1] = 0;
          v49[3] = 0;
          v49[2] = 0;
        }
        else
        {
          v49 = 0;
        }
        dword_59A9F8 = (int)v49;
        if ( v49 )
        {
          v49[1] = v47;
          dword_59A9FC = dword_59A9F8;
          v50 = ++dword_59AA08;
          LOBYTE(v50) = this[121];
          return (_DWORD *)(*(int (__thiscall **)(char *, int, int, _DWORD))(*(_DWORD *)this + 16))(this, 7, v50, 0);
        }
      }
    }
    else if ( v47 )
    {
      v49 = (_DWORD *)(**(int (__thiscall ***)(char *, int))v47)(v47, 1);
    }
    LOBYTE(v49) = this[121];
    return (_DWORD *)(*(int (__thiscall **)(char *, int, _DWORD *, _DWORD))(*(_DWORD *)this + 16))(this, 7, v49, 0);
  }
  if ( v122 == (_DWORD *)18 )
  {
    *((_DWORD *)this + 33) = *(unsigned __int16 *)(a2 + 6);
    *((_DWORD *)this + 34) = *(unsigned __int16 *)(a2 + 8);
    this[121] = *(_BYTE *)(a2 + 10);
    v52 = (char *)MEMORY[0x4A01AA](0x240u);
    v123 = 15;
    if ( v52 )
      v13 = sub_449480(v52);
    else
      v13 = 0;
    v53 = *(int (__thiscall ***)(_DWORD, _DWORD))v13;
    v113 = *((_DWORD *)this + 34);
    v106 = *((_DWORD *)this + 33);
    v123 = -1;
    v15 = (_DWORD *)((int (__thiscall *)(char *, int, int, int, int, int, char *, _DWORD, _DWORD))v53[1])(
                      v13,
                      588,
                      v106,
                      v113,
                      v106,
                      v113,
                      this,
                      0,
                      0);
    if ( !v15 )
      goto LABEL_183;
    if ( dword_59A9F8 )
      goto LABEL_180;
    v15 = MEMORY[0x4A01AA](0x10u);
    if ( v15 )
    {
      *v15 = &off_4AF460;
      v15[1] = 0;
      v15[3] = 0;
      v15[2] = 0;
      goto LABEL_178;
    }
    goto LABEL_177;
  }
  if ( v122 == (_DWORD *)19 )
  {
    *((_DWORD *)this + 33) = *(unsigned __int16 *)(a2 + 6);
    *((_DWORD *)this + 34) = *(unsigned __int16 *)(a2 + 8);
    v117 = *(unsigned __int8 *)(a2 + 10);
    this[121] = v117;
    sub_40DB10((int)this, (int)dword_59AA6C, 587, v117, -1);
    LOBYTE(v54) = this[121];
    v55 = *(_DWORD *)this;
    *((_DWORD *)this + 100060) = 1;
    return (_DWORD *)(*(int (__thiscall **)(char *, int, int, _DWORD))(v55 + 16))(this, 5, v54, 0);
  }
  if ( v122 == (_DWORD *)20 )
  {
    *((_DWORD *)this + 33) = *(unsigned __int16 *)(a2 + 6);
    *((_DWORD *)this + 34) = *(unsigned __int16 *)(a2 + 8);
    this[121] = *(_BYTE *)(a2 + 10);
    v56 = (char *)MEMORY[0x4A01AA](0x240u);
    v123 = 16;
    if ( v56 )
      v29 = sub_449480(v56);
    else
      v29 = 0;
    v57 = *(int (__thiscall ***)(_DWORD, _DWORD))v29;
    v97 = *((_DWORD *)this + 34);
    v82 = *((_DWORD *)this + 33);
    v123 = -1;
    v31 = (_DWORD *)((int (__thiscall *)(char *, int, int, int, int, int, char *, int, int))v57[1])(
                      v29,
                      597,
                      v82,
                      v97,
                      v119,
                      v120,
                      this,
                      v5,
                      800);
    goto LABEL_86;
  }
  if ( v122 == (_DWORD *)21 )
  {
    *((_DWORD *)this + 33) = *(unsigned __int16 *)(a2 + 6);
    *((_DWORD *)this + 34) = *(unsigned __int16 *)(a2 + 8);
    this[121] = *(_BYTE *)(a2 + 10);
    v58 = (char *)MEMORY[0x4A01AA](0x240u);
    v123 = 17;
    if ( v58 )
      v29 = sub_449480(v58);
    else
      v29 = 0;
    v59 = *(int (__thiscall ***)(_DWORD, _DWORD))v29;
    v98 = *((_DWORD *)this + 34);
    v83 = *((_DWORD *)this + 33);
    v123 = -1;
    v31 = (_DWORD *)((int (__thiscall *)(char *, int, int, int, int, int, char *, int, int))v59[1])(
                      v29,
                      603,
                      v83,
                      v98,
                      v119,
                      v120,
                      this,
                      v5,
                      400);
LABEL_86:
    if ( !v31 )
      goto LABEL_167;
    if ( dword_59A9F8 )
      goto LABEL_165;
    v31 = MEMORY[0x4A01AA](0x10u);
    if ( v31 )
    {
      *v31 = &off_4AF460;
      v31[1] = 0;
      v31[3] = 0;
      v31[2] = 0;
      goto LABEL_79;
    }
LABEL_78:
    v31 = 0;
    goto LABEL_79;
  }
  if ( v122 == (_DWORD *)22 )
  {
    *((_DWORD *)this + 33) = *(unsigned __int16 *)(a2 + 6);
    *((_DWORD *)this + 34) = *(unsigned __int16 *)(a2 + 8);
    this[121] = *(_BYTE *)(a2 + 10);
    v60 = (char *)MEMORY[0x4A01AA](0x240u);
    v123 = 18;
    if ( v60 )
      v13 = sub_449480(v60);
    else
      v13 = 0;
    v61 = *(int (__thiscall ***)(_DWORD, _DWORD))v13;
    v114 = *((_DWORD *)this + 34);
    v107 = *((_DWORD *)this + 33);
    v123 = -1;
    v15 = (_DWORD *)((int (__thiscall *)(char *, int, int, int, int, int, char *, int, _DWORD))v61[1])(
                      v13,
                      606,
                      v107,
                      v114,
                      v107,
                      v114,
                      this,
                      -1,
                      0);
    if ( !v15 )
      goto LABEL_183;
    if ( dword_59A9F8 )
      goto LABEL_180;
    v15 = MEMORY[0x4A01AA](0x10u);
    if ( v15 )
    {
      *v15 = &off_4AF460;
      v15[1] = 0;
      v15[3] = 0;
      v15[2] = 0;
      goto LABEL_178;
    }
LABEL_177:
    v15 = 0;
LABEL_178:
    dword_59A9F8 = (int)v15;
    if ( v15 )
    {
      v15[1] = v13;
      v15 = (_DWORD *)dword_59A9F8;
LABEL_182:
      dword_59A9FC = (int)v15;
      v62 = ++dword_59AA08;
      LOBYTE(v62) = this[121];
      return (_DWORD *)(*(int (__thiscall **)(char *, int, int, _DWORD))(*(_DWORD *)this + 16))(this, 6, v62, 0);
    }
LABEL_185:
    LOBYTE(v15) = this[121];
    return (_DWORD *)(*(int (__thiscall **)(char *, int, _DWORD *, _DWORD))(*(_DWORD *)this + 16))(this, 6, v15, 0);
  }
  if ( v122 == (_DWORD *)23 )
  {
    *((_DWORD *)this + 33) = *(unsigned __int16 *)(a2 + 6);
    *((_DWORD *)this + 34) = *(unsigned __int16 *)(a2 + 8);
    this[121] = *(_BYTE *)(a2 + 10);
    v63 = (char *)MEMORY[0x4A01AA](0x240u);
    v123 = 19;
    if ( v63 )
      v18 = sub_449480(v63);
    else
      v18 = 0;
    v64 = *(int (__thiscall ***)(_DWORD, _DWORD))v18;
    v99 = *((_DWORD *)this + 34);
    v84 = *((_DWORD *)this + 33);
    v123 = -1;
    result = (_DWORD *)((int (__thiscall *)(char *, int, int, int, int, int, char *, int, int))v64[1])(
                         v18,
                         2,
                         v84,
                         v99,
                         v119,
                         v120,
                         this,
                         v5,
                         700);
    if ( result )
      goto LABEL_37;
    goto LABEL_40;
  }
  if ( v122 == (_DWORD *)24 )
  {
    *((_DWORD *)this + 33) = *(unsigned __int16 *)(a2 + 6);
    *((_DWORD *)this + 34) = *(unsigned __int16 *)(a2 + 8);
    this[121] = *(_BYTE *)(a2 + 10);
    v65 = (char *)MEMORY[0x4A01AA](0x240u);
    v123 = 20;
    if ( v65 )
      v18 = sub_449480(v65);
    else
      v18 = 0;
    v66 = *(int (__thiscall ***)(_DWORD, _DWORD))v18;
    v100 = *((_DWORD *)this + 34);
    v85 = *((_DWORD *)this + 33);
    v123 = -1;
    result = (_DWORD *)((int (__thiscall *)(char *, int, int, int, int, int, _DWORD, int, _DWORD))v66[1])(
                         v18,
                         29,
                         v85,
                         v100,
                         v119,
                         v120,
                         0,
                         v5,
                         0);
    if ( result )
    {
      if ( !dword_59A9F8 )
      {
        result = MEMORY[0x4A01AA](0x10u);
        if ( result )
        {
          *result = &off_4AF460;
          result[1] = 0;
          result[3] = 0;
          result[2] = 0;
          goto LABEL_201;
        }
LABEL_200:
        result = 0;
LABEL_201:
        dword_59A9F8 = (int)result;
        if ( result )
        {
          result[1] = v18;
          result = (_DWORD *)(dword_59AA08 + 1);
          dword_59A9FC = dword_59A9F8;
          ++dword_59AA08;
        }
        return result;
      }
      goto LABEL_203;
    }
    goto LABEL_40;
  }
  if ( v122 == (_DWORD *)25 )
  {
    *((_DWORD *)this + 33) = *(unsigned __int16 *)(a2 + 6);
    v67 = v120;
    *((_DWORD *)this + 34) = *(unsigned __int16 *)(a2 + 8);
    v118 = v119;
    this[121] = *(_BYTE *)(a2 + 10);
    return (_DWORD *)sub_40DD80((int *)this, 53, 0, v118, v67);
  }
  if ( v122 == (_DWORD *)26 )
  {
    *((_DWORD *)this + 33) = *(unsigned __int16 *)(a2 + 6);
    *((_DWORD *)this + 34) = *(unsigned __int16 *)(a2 + 8);
    this[121] = *(_BYTE *)(a2 + 10);
    v68 = (char *)MEMORY[0x4A01AA](0x240u);
    v123 = 21;
    if ( v68 )
      v18 = sub_449480(v68);
    else
      v18 = 0;
    v69 = *(int (__thiscall ***)(_DWORD, _DWORD))v18;
    v115 = *((_DWORD *)this + 34);
    v108 = *((_DWORD *)this + 33);
    v123 = -1;
    result = (_DWORD *)((int (__thiscall *)(char *, int, int, int, int, int, char *, int, int))v69[1])(
                         v18,
                         120,
                         v119,
                         v120,
                         v108,
                         v115,
                         this,
                         v5,
                         700);
    goto LABEL_47;
  }
  return result;
}
int __thiscall sub_40B0A0(char *this, int a2)
{
  unsigned __int16 v3; // bx
  unsigned __int16 v4; // bp
  unsigned __int16 v5; // ax
  int result; // eax
  char *v7; // eax
  void (__thiscall **v8)(_DWORD, _DWORD); // eax
  int v9; // edx
  _DWORD *v10; // eax
  int v11; // ecx
  _DWORD *v12; // eax
  char *v13; // eax
  void (__thiscall **v14)(_DWORD, _DWORD); // edx
  int v15; // eax
  char *v16; // eax
  void (__thiscall **v17)(_DWORD, _DWORD); // edx
  int v18; // eax
  char *v19; // eax
  void (__thiscall **v20)(_DWORD, _DWORD); // edx
  int v21; // eax
  char *v22; // eax
  void (__thiscall **v23)(_DWORD, _DWORD); // edx
  int v24; // eax
  char *v25; // eax
  void (__thiscall **v26)(_DWORD, _DWORD); // edx
  int v27; // eax
  char *v28; // eax
  void (__thiscall **v29)(_DWORD, _DWORD); // edx
  int v30; // eax
  char *v31; // eax
  void (__thiscall **v32)(_DWORD, _DWORD); // edx
  int v33; // eax
  char *v34; // eax
  void (__thiscall **v35)(_DWORD, _DWORD); // edx
  int v36; // eax
  char *v37; // eax
  int v38; // eax
  int v39; // ecx
  _DWORD *v40; // eax
  _DWORD *v41; // eax
  char *v42; // eax
  int (__thiscall **v43)(_DWORD, _DWORD); // eax
  int v44; // edx
  char *v45; // eax
  void (__thiscall **v46)(_DWORD, _DWORD); // eax
  int v47; // edx
  char *v48; // eax
  void (__thiscall **v49)(_DWORD, _DWORD); // edx
  int v50; // eax
  char *v51; // ecx
  int v52; // ecx
  int v53; // ebp
  int v54; // ebx
  char *v55; // eax
  char *v56; // eax
  int v57; // edx
  int v58; // ecx
  _DWORD *v59; // eax
  _DWORD *v60; // eax
  int v61; // ebp
  char *v62; // eax
  char *v63; // ebx
  void (__thiscall **v64)(_DWORD, _DWORD); // edx
  _DWORD *v65; // eax
  char *v66; // eax
  void (__thiscall **v67)(_DWORD, _DWORD); // eax
  int v68; // eax
  _DWORD *v69; // eax
  char *v70; // eax
  void (__thiscall **v71)(_DWORD, _DWORD); // edx
  char *v72; // eax
  void (__thiscall **v73)(_DWORD, _DWORD); // edx
  char *v74; // eax
  void (__thiscall **v75)(_DWORD, _DWORD); // edx
  char *v76; // eax
  void (__thiscall **v77)(_DWORD, _DWORD); // edx
  _DWORD *v78; // eax
  char *v79; // eax
  void (__thiscall **v80)(_DWORD, _DWORD); // eax
  _DWORD *v81; // eax
  char *v82; // eax
  void (__thiscall **v83)(_DWORD, _DWORD); // edx
  char *v84; // eax
  void (__thiscall **v85)(_DWORD, _DWORD); // edx
  int v86; // [esp+18h] [ebp-4Ch]
  int v87; // [esp+18h] [ebp-4Ch]
  int v88; // [esp+18h] [ebp-4Ch]
  int v89; // [esp+18h] [ebp-4Ch]
  int v90; // [esp+18h] [ebp-4Ch]
  int v91; // [esp+18h] [ebp-4Ch]
  int v92; // [esp+18h] [ebp-4Ch]
  int v93; // [esp+18h] [ebp-4Ch]
  int v94; // [esp+18h] [ebp-4Ch]
  int v95; // [esp+1Ch] [ebp-48h]
  int v96; // [esp+1Ch] [ebp-48h]
  int v97; // [esp+1Ch] [ebp-48h]
  int v98; // [esp+1Ch] [ebp-48h]
  int v99; // [esp+1Ch] [ebp-48h]
  int v100; // [esp+1Ch] [ebp-48h]
  int v101; // [esp+1Ch] [ebp-48h]
  int v102; // [esp+1Ch] [ebp-48h]
  int v103; // [esp+1Ch] [ebp-48h]
  int v104; // [esp+1Ch] [ebp-48h]
  int v105; // [esp+1Ch] [ebp-48h]
  int v106; // [esp+1Ch] [ebp-48h]
  int v107; // [esp+1Ch] [ebp-48h]
  int v108; // [esp+1Ch] [ebp-48h]
  int v109; // [esp+1Ch] [ebp-48h]
  int v110; // [esp+1Ch] [ebp-48h]
  int v111; // [esp+1Ch] [ebp-48h]
  int v112; // [esp+1Ch] [ebp-48h]
  int v113; // [esp+1Ch] [ebp-48h]
  int v114; // [esp+1Ch] [ebp-48h]
  int v115; // [esp+1Ch] [ebp-48h]
  int v116; // [esp+1Ch] [ebp-48h]
  char *v117; // [esp+44h] [ebp-20h]
  char *v118; // [esp+44h] [ebp-20h]
  char *v119; // [esp+44h] [ebp-20h]
  int v120; // [esp+44h] [ebp-20h]
  _WORD v121[2]; // [esp+4Ch] [ebp-18h] BYREF
  int v122; // [esp+50h] [ebp-14h]
  int v123; // [esp+60h] [ebp-4h]
  int v124; // [esp+68h] [ebp+4h]
  char *v125; // [esp+68h] [ebp+4h]

  *((_DWORD *)this + 33) = *(unsigned __int16 *)(a2 + 6);
  *((_DWORD *)this + 34) = *(unsigned __int16 *)(a2 + 8);
  this[121] = *(_BYTE *)(a2 + 10);
  sub_47D780((const char *)(a2 + 12), (int)v121, 8);
  v3 = v121[1];
  v4 = v121[0];
  v124 = v122;
  if ( this[69] == 3 )
    v5 = *(_WORD *)(this + 71);
  else
    v5 = (unsigned __int8)this[71];
  result = v5 - 7;
  switch ( result )
  {
    case 0:
      v16 = (char *)MEMORY[0x4A01AA](0x240u);
      v123 = 2;
      if ( v16 )
        v117 = sub_449480(v16);
      else
        v117 = 0;
      v17 = *(void (__thiscall ***)(_DWORD, _DWORD))v117;
      v97 = *((_DWORD *)this + 34);
      v18 = *((_DWORD *)this + 33);
      v123 = -1;
      if ( !((int (__thiscall *)(char *, int, int, int, _DWORD, _DWORD, char *, int, int))v17[1])(
              v117,
              505,
              v18,
              v97,
              v4,
              v3,
              this,
              v124,
              500) )
      {
        v11 = (int)v117;
        if ( v117 )
          (**(void (__thiscall ***)(char *, int))v117)(v117, 1);
        goto LABEL_224;
      }
      if ( dword_59A9F8 )
      {
        v12 = MEMORY[0x4A01AA](0x10u);
        if ( !v12 )
          goto LABEL_224;
        goto LABEL_195;
      }
      v10 = MEMORY[0x4A01AA](0x10u);
      if ( v10 )
      {
        *v10 = &off_4AF460;
        v10[1] = 0;
        v10[3] = 0;
        v10[2] = 0;
      }
      else
      {
        v10 = 0;
      }
      dword_59A9F8 = (int)v10;
      if ( !v10 )
        goto LABEL_224;
      goto LABEL_193;
    case 12:
      v7 = (char *)MEMORY[0x4A01AA](0x240u);
      v123 = 0;
      if ( v7 )
        v117 = sub_449480(v7);
      else
        v117 = 0;
      v8 = *(void (__thiscall ***)(_DWORD, _DWORD))v117;
      v95 = *((_DWORD *)this + 34);
      v9 = *((_DWORD *)this + 33);
      v123 = -1;
      if ( !((int (__thiscall *)(char *, int, int, int, _DWORD, _DWORD, char *, int, int))v8[1])(
              v117,
              511,
              v9,
              v95,
              v4,
              v3,
              this,
              v124,
              500) )
      {
        v11 = (int)v117;
        if ( v117 )
          (**(void (__thiscall ***)(char *, int))v117)(v117, 1);
        goto LABEL_224;
      }
      if ( dword_59A9F8 )
      {
        v12 = MEMORY[0x4A01AA](0x10u);
        if ( !v12 )
          goto LABEL_224;
        goto LABEL_195;
      }
      v10 = MEMORY[0x4A01AA](0x10u);
      if ( v10 )
      {
        *v10 = &off_4AF460;
        v10[1] = 0;
        v10[3] = 0;
        v10[2] = 0;
      }
      else
      {
        v10 = 0;
      }
      dword_59A9F8 = (int)v10;
      if ( !v10 )
        goto LABEL_224;
      goto LABEL_193;
    case 26:
      v19 = (char *)MEMORY[0x4A01AA](0x240u);
      v123 = 3;
      if ( v19 )
        v117 = sub_449480(v19);
      else
        v117 = 0;
      v20 = *(void (__thiscall ***)(_DWORD, _DWORD))v117;
      v98 = *((_DWORD *)this + 34);
      v21 = *((_DWORD *)this + 33);
      v123 = -1;
      if ( !((int (__thiscall *)(char *, int, int, int, _DWORD, _DWORD, char *, int, int))v20[1])(
              v117,
              520,
              v21,
              v98,
              v4,
              v3,
              this,
              v124,
              365) )
      {
        v11 = (int)v117;
        if ( v117 )
          (**(void (__thiscall ***)(char *, int))v117)(v117, 1);
        goto LABEL_224;
      }
      if ( dword_59A9F8 )
      {
        v12 = MEMORY[0x4A01AA](0x10u);
        if ( !v12 )
          goto LABEL_224;
        goto LABEL_195;
      }
      v10 = MEMORY[0x4A01AA](0x10u);
      if ( v10 )
      {
        *v10 = &off_4AF460;
        v10[1] = 0;
        v10[3] = 0;
        v10[2] = 0;
      }
      else
      {
        v10 = 0;
      }
      dword_59A9F8 = (int)v10;
      if ( !v10 )
        goto LABEL_224;
      goto LABEL_193;
    case 39:
      v22 = (char *)MEMORY[0x4A01AA](0x240u);
      v123 = 4;
      if ( v22 )
        v117 = sub_449480(v22);
      else
        v117 = 0;
      v23 = *(void (__thiscall ***)(_DWORD, _DWORD))v117;
      v99 = *((_DWORD *)this + 34);
      v24 = *((_DWORD *)this + 33);
      v123 = -1;
      if ( !((int (__thiscall *)(char *, int, int, int, _DWORD, _DWORD, char *, int, int))v23[1])(
              v117,
              517,
              v24,
              v99,
              v4,
              v3,
              this,
              v124,
              365) )
      {
        v11 = (int)v117;
        if ( v117 )
          (**(void (__thiscall ***)(char *, int))v117)(v117, 1);
        goto LABEL_224;
      }
      if ( dword_59A9F8 )
      {
        v12 = MEMORY[0x4A01AA](0x10u);
        if ( !v12 )
          goto LABEL_224;
        goto LABEL_195;
      }
      v10 = MEMORY[0x4A01AA](0x10u);
      if ( v10 )
      {
        *v10 = &off_4AF460;
        v10[1] = 0;
        v10[3] = 0;
        v10[2] = 0;
      }
      else
      {
        v10 = 0;
      }
      dword_59A9F8 = (int)v10;
      if ( !v10 )
        goto LABEL_224;
      goto LABEL_193;
    case 75:
    case 137:
      v25 = (char *)MEMORY[0x4A01AA](0x240u);
      v123 = 5;
      if ( v25 )
        v117 = sub_449480(v25);
      else
        v117 = 0;
      v26 = *(void (__thiscall ***)(_DWORD, _DWORD))v117;
      v100 = *((_DWORD *)this + 34);
      v27 = *((_DWORD *)this + 33);
      v123 = -1;
      if ( !((int (__thiscall *)(char *, int, int, int, _DWORD, _DWORD, char *, int, int))v26[1])(
              v117,
              519,
              v27,
              v100,
              v4,
              v3,
              this,
              v124,
              365) )
      {
        v11 = (int)v117;
        if ( v117 )
          (**(void (__thiscall ***)(char *, int))v117)(v117, 1);
        goto LABEL_224;
      }
      if ( dword_59A9F8 )
      {
        v12 = MEMORY[0x4A01AA](0x10u);
        if ( !v12 )
          goto LABEL_224;
        goto LABEL_195;
      }
      v10 = MEMORY[0x4A01AA](0x10u);
      if ( v10 )
      {
        *v10 = &off_4AF460;
        v10[1] = 0;
        v10[3] = 0;
        v10[2] = 0;
      }
      else
      {
        v10 = 0;
      }
      dword_59A9F8 = (int)v10;
      if ( !v10 )
        goto LABEL_224;
      goto LABEL_193;
    case 79:
      v28 = (char *)MEMORY[0x4A01AA](0x240u);
      v123 = 6;
      if ( v28 )
        v117 = sub_449480(v28);
      else
        v117 = 0;
      v29 = *(void (__thiscall ***)(_DWORD, _DWORD))v117;
      v101 = *((_DWORD *)this + 34);
      v30 = *((_DWORD *)this + 33);
      v123 = -1;
      if ( !((int (__thiscall *)(char *, int, int, int, _DWORD, _DWORD, char *, int, int))v29[1])(
              v117,
              518,
              v30,
              v101,
              v4,
              v3,
              this,
              v124,
              365) )
      {
        v11 = (int)v117;
        if ( v117 )
          (**(void (__thiscall ***)(char *, int))v117)(v117, 1);
        goto LABEL_224;
      }
      if ( dword_59A9F8 )
      {
        v12 = MEMORY[0x4A01AA](0x10u);
        if ( !v12 )
          goto LABEL_224;
        goto LABEL_195;
      }
      v10 = MEMORY[0x4A01AA](0x10u);
      if ( v10 )
      {
        *v10 = &off_4AF460;
        v10[1] = 0;
        v10[3] = 0;
        v10[2] = 0;
      }
      else
      {
        v10 = 0;
      }
      dword_59A9F8 = (int)v10;
      if ( !v10 )
        goto LABEL_224;
      goto LABEL_193;
    case 88:
      v31 = (char *)MEMORY[0x4A01AA](0x240u);
      v123 = 7;
      if ( v31 )
        v117 = sub_449480(v31);
      else
        v117 = 0;
      v32 = *(void (__thiscall ***)(_DWORD, _DWORD))v117;
      v102 = *((_DWORD *)this + 34);
      v33 = *((_DWORD *)this + 33);
      v123 = -1;
      if ( !((int (__thiscall *)(char *, int, int, int, _DWORD, _DWORD, char *, int, int))v32[1])(
              v117,
              545,
              v33,
              v102,
              v4,
              v3,
              this,
              v124,
              700) )
      {
        v11 = (int)v117;
        if ( v117 )
          (**(void (__thiscall ***)(char *, int))v117)(v117, 1);
        goto LABEL_224;
      }
      if ( dword_59A9F8 )
      {
        v12 = MEMORY[0x4A01AA](0x10u);
        if ( !v12 )
          goto LABEL_224;
        goto LABEL_195;
      }
      v10 = MEMORY[0x4A01AA](0x10u);
      if ( v10 )
      {
        *v10 = &off_4AF460;
        v10[1] = 0;
        v10[3] = 0;
        v10[2] = 0;
      }
      else
      {
        v10 = 0;
      }
      dword_59A9F8 = (int)v10;
      if ( !v10 )
        goto LABEL_224;
      goto LABEL_193;
    case 89:
      v13 = (char *)MEMORY[0x4A01AA](0x240u);
      v123 = 1;
      if ( v13 )
        v117 = sub_449480(v13);
      else
        v117 = 0;
      v14 = *(void (__thiscall ***)(_DWORD, _DWORD))v117;
      v96 = *((_DWORD *)this + 34);
      v15 = *((_DWORD *)this + 33);
      v123 = -1;
      if ( !((int (__thiscall *)(char *, int, int, int, _DWORD, _DWORD, char *, int, int))v14[1])(
              v117,
              544,
              v15,
              v96,
              v4,
              v3,
              this,
              v124,
              500) )
      {
        v11 = (int)v117;
        if ( v117 )
          (**(void (__thiscall ***)(char *, int))v117)(v117, 1);
        goto LABEL_224;
      }
      if ( dword_59A9F8 )
      {
        v12 = MEMORY[0x4A01AA](0x10u);
        if ( !v12 )
          goto LABEL_224;
        goto LABEL_195;
      }
      v10 = MEMORY[0x4A01AA](0x10u);
      if ( v10 )
      {
        *v10 = &off_4AF460;
        v10[1] = 0;
        v10[3] = 0;
        v10[2] = 0;
      }
      else
      {
        v10 = 0;
      }
      dword_59A9F8 = (int)v10;
      if ( !v10 )
        goto LABEL_224;
      goto LABEL_193;
    case 117:
    case 118:
      v37 = (char *)MEMORY[0x4A01AA](0x240u);
      v123 = 9;
      if ( v37 )
        v118 = sub_449480(v37);
      else
        v118 = 0;
      v38 = *((_DWORD *)this + 34);
      v39 = **((_DWORD **)this + 100110);
      v123 = -1;
      if ( (*(int (__thiscall **)(char *, int, _DWORD, int, _DWORD, int, char *, int, _DWORD))(*(_DWORD *)v118 + 4))(
             v118,
             569,
             *((_DWORD *)this + 33),
             v38,
             *((_DWORD *)this + 33),
             v38,
             this,
             v39,
             0) )
      {
        if ( dword_59A9F8 )
        {
          v41 = MEMORY[0x4A01AA](0x10u);
          if ( !v41 )
            goto LABEL_155;
          *v41 = &off_4AF460;
          v41[1] = 0;
          v41[3] = 0;
          v41[2] = 0;
          v41[1] = v118;
          v41[3] = 0;
          v41[2] = dword_59A9FC;
          *(_DWORD *)(dword_59A9FC + 12) = v41;
          goto LABEL_152;
        }
        v40 = MEMORY[0x4A01AA](0x10u);
        if ( v40 )
        {
          *v40 = &off_4AF460;
          v40[1] = 0;
          v40[3] = 0;
          v40[2] = 0;
        }
        else
        {
          v40 = 0;
        }
        dword_59A9F8 = (int)v40;
        if ( v40 )
        {
          v40[1] = v118;
          v41 = (_DWORD *)dword_59A9F8;
LABEL_152:
          dword_59A9FC = (int)v41;
          ++dword_59AA08;
        }
      }
      else if ( v118 )
      {
        (**(void (__thiscall ***)(char *, int))v118)(v118, 1);
      }
LABEL_155:
      v42 = (char *)MEMORY[0x4A01AA](0x240u);
      v123 = 10;
      if ( v42 )
        v119 = sub_449480(v42);
      else
        v119 = 0;
      v43 = *(int (__thiscall ***)(_DWORD, _DWORD))v119;
      v104 = *((_DWORD *)this + 34);
      v44 = *((_DWORD *)this + 33);
      v123 = -1;
      result = ((int (__thiscall *)(char *, int, int, int, _DWORD, _DWORD, char *, int, int))v43[1])(
                 v119,
                 2,
                 v44,
                 v104,
                 v4,
                 v3,
                 this,
                 v124,
                 700);
      if ( result )
      {
        if ( dword_59A9F8 )
        {
          result = (int)MEMORY[0x4A01AA](0x10u);
          if ( result )
          {
            *(_DWORD *)result = &off_4AF460;
            *(_DWORD *)(result + 4) = 0;
            *(_DWORD *)(result + 12) = 0;
            *(_DWORD *)(result + 8) = 0;
            *(_DWORD *)(result + 4) = v119;
            *(_DWORD *)(result + 12) = 0;
            *(_DWORD *)(result + 8) = dword_59A9FC;
            *(_DWORD *)(dword_59A9FC + 12) = result;
            dword_59A9FC = result;
            ++dword_59AA08;
          }
        }
        else
        {
          result = (int)MEMORY[0x4A01AA](0x10u);
          if ( result )
          {
            *(_DWORD *)result = &off_4AF460;
            *(_DWORD *)(result + 4) = 0;
            *(_DWORD *)(result + 12) = 0;
            *(_DWORD *)(result + 8) = 0;
          }
          else
          {
            result = 0;
          }
          dword_59A9F8 = result;
          if ( result )
          {
            *(_DWORD *)(result + 4) = v119;
            result = dword_59AA08 + 1;
            dword_59A9FC = dword_59A9F8;
            ++dword_59AA08;
          }
        }
      }
      else if ( v119 )
      {
        return (**(int (__thiscall ***)(char *, int))v119)(v119, 1);
      }
      return result;
    case 138:
    case 139:
      v34 = (char *)MEMORY[0x4A01AA](0x240u);
      v123 = 8;
      if ( v34 )
        v117 = sub_449480(v34);
      else
        v117 = 0;
      v35 = *(void (__thiscall ***)(_DWORD, _DWORD))v117;
      v103 = *((_DWORD *)this + 34);
      v36 = *((_DWORD *)this + 33);
      v123 = -1;
      if ( !((int (__thiscall *)(char *, int, int, int, _DWORD, _DWORD, char *, int, _DWORD))v35[1])(
              v117,
              573,
              v36,
              v103,
              v4,
              v3,
              this,
              v124,
              0) )
      {
        v11 = (int)v117;
        if ( v117 )
          (**(void (__thiscall ***)(char *, int))v117)(v117, 1);
        goto LABEL_224;
      }
      if ( dword_59A9F8 )
      {
        v12 = MEMORY[0x4A01AA](0x10u);
        if ( !v12 )
          goto LABEL_224;
        goto LABEL_195;
      }
      v10 = MEMORY[0x4A01AA](0x10u);
      if ( v10 )
      {
        *v10 = &off_4AF460;
        v10[1] = 0;
        v10[3] = 0;
        v10[2] = 0;
      }
      else
      {
        v10 = 0;
      }
      dword_59A9F8 = (int)v10;
      if ( !v10 )
        goto LABEL_224;
      goto LABEL_193;
    case 149:
      v45 = (char *)MEMORY[0x4A01AA](0x240u);
      v123 = 11;
      if ( v45 )
        v117 = sub_449480(v45);
      else
        v117 = 0;
      v46 = *(void (__thiscall ***)(_DWORD, _DWORD))v117;
      v105 = *((_DWORD *)this + 34);
      v47 = *((_DWORD *)this + 33);
      v123 = -1;
      if ( !((int (__thiscall *)(char *, int, int, int, _DWORD, _DWORD, char *, int, int))v46[1])(
              v117,
              578,
              v47,
              v105,
              v4,
              v3,
              this,
              v124,
              700) )
      {
        v11 = (int)v117;
        if ( v117 )
          (**(void (__thiscall ***)(char *, int))v117)(v117, 1);
        goto LABEL_224;
      }
      if ( dword_59A9F8 )
      {
        v12 = MEMORY[0x4A01AA](0x10u);
        if ( !v12 )
          goto LABEL_224;
        goto LABEL_195;
      }
      v10 = MEMORY[0x4A01AA](0x10u);
      if ( v10 )
      {
        *v10 = &off_4AF460;
        v10[1] = 0;
        v10[3] = 0;
        v10[2] = 0;
      }
      else
      {
        v10 = 0;
      }
      dword_59A9F8 = (int)v10;
      if ( !v10 )
        goto LABEL_224;
      goto LABEL_193;
    case 160:
      switch ( this[121] )
      {
        case 0:
          v53 = *((_DWORD *)this + 33);
          v54 = *((_DWORD *)this + 34) - 20;
          break;
        case 1:
          v53 = *((_DWORD *)this + 33) + 20;
          v54 = *((_DWORD *)this + 34) - 30;
          break;
        case 2:
          v53 = *((_DWORD *)this + 33) + 20;
          v54 = *((_DWORD *)this + 34) - 5;
          break;
        case 3:
          v53 = *((_DWORD *)this + 33) + 20;
          v54 = *((_DWORD *)this + 34) + 10;
          break;
        case 4:
          v53 = *((_DWORD *)this + 33);
          v54 = *((_DWORD *)this + 34) + 20;
          break;
        case 5:
          v53 = *((_DWORD *)this + 33) - 20;
          v54 = *((_DWORD *)this + 34) + 10;
          break;
        case 6:
          v53 = *((_DWORD *)this + 33) - 20;
          v54 = *((_DWORD *)this + 34) - 5;
          break;
        case 7:
          v53 = *((_DWORD *)this + 33) - 20;
          v54 = *((_DWORD *)this + 34) - 30;
          break;
        default:
          v53 = v122;
          v54 = v53;
          break;
      }
      v55 = (char *)MEMORY[0x4A01AA](0x240u);
      v123 = 13;
      if ( v55 )
      {
        v56 = sub_449480(v55);
        v125 = v56;
      }
      else
      {
        v125 = 0;
        v56 = 0;
      }
      v57 = *(_DWORD *)v56;
      v107 = *((_DWORD *)this + 34);
      v58 = *((_DWORD *)this + 33);
      v123 = -1;
      if ( (*(int (__thiscall **)(char *, int, int, int, int, int, char *, _DWORD, int))(v57 + 4))(
             v56,
             593,
             v58,
             v107,
             v53,
             v54,
             this,
             0,
             700) )
      {
        if ( dword_59A9F8 )
        {
          v60 = MEMORY[0x4A01AA](0x10u);
          if ( !v60 )
            goto LABEL_224;
          *v60 = &off_4AF460;
          v60[1] = 0;
          v60[3] = 0;
          v60[2] = 0;
          v60[1] = v125;
          v60[3] = 0;
          v60[2] = dword_59A9FC;
          v11 = dword_59A9FC;
          *(_DWORD *)(dword_59A9FC + 12) = v60;
          goto LABEL_221;
        }
        v59 = MEMORY[0x4A01AA](0x10u);
        if ( v59 )
        {
          *v59 = &off_4AF460;
          v59[1] = 0;
          v59[3] = 0;
          v59[2] = 0;
        }
        else
        {
          v59 = 0;
        }
        dword_59A9F8 = (int)v59;
        if ( v59 )
        {
          v59[1] = v125;
          v60 = (_DWORD *)dword_59A9F8;
LABEL_221:
          dword_59A9FC = (int)v60;
          ++dword_59AA08;
          LOBYTE(v11) = this[121];
          return (*(int (__thiscall **)(char *, int, int, _DWORD))(*(_DWORD *)this + 16))(this, 2, v11, 0);
        }
      }
      else
      {
        v11 = (int)v125;
        if ( v125 )
          (**(void (__thiscall ***)(char *, int))v125)(v125, 1);
      }
LABEL_224:
      LOBYTE(v11) = this[121];
      return (*(int (__thiscall **)(char *, int, int, _DWORD))(*(_DWORD *)this + 16))(this, 2, v11, 0);
    case 162:
      result = (unsigned __int8)this[121];
      switch ( this[121] )
      {
        case 0:
          v124 = *((_DWORD *)this + 33);
          v61 = *((_DWORD *)this + 34) - 20;
          break;
        case 1:
          result = *((_DWORD *)this + 33) + 20;
          v61 = *((_DWORD *)this + 34) - 30;
          v124 = result;
          break;
        case 2:
          result = *((_DWORD *)this + 33) + 30;
          v61 = *((_DWORD *)this + 34) - 8;
          v124 = result;
          break;
        case 3:
          result = *((_DWORD *)this + 33) + 20;
          v61 = *((_DWORD *)this + 34) + 12;
          v124 = result;
          break;
        case 4:
          result = *((_DWORD *)this + 33);
          v124 = result;
          v61 = *((_DWORD *)this + 34) + 20;
          break;
        case 5:
          result = *((_DWORD *)this + 33) - 20;
          v61 = *((_DWORD *)this + 34) + 12;
          v124 = result;
          break;
        case 6:
          result = *((_DWORD *)this + 33) - 30;
          v61 = *((_DWORD *)this + 34) - 8;
          v124 = result;
          break;
        case 7:
          result = *((_DWORD *)this + 33) - 20;
          v61 = *((_DWORD *)this + 34) - 30;
          v124 = result;
          break;
        default:
          v61 = v122;
          break;
      }
      v120 = 0;
      while ( 2 )
      {
        LOBYTE(result) = this[121];
        (*(void (__thiscall **)(char *, int, int, _DWORD))(*(_DWORD *)this + 16))(this, 2, result, 0);
        switch ( v120 )
        {
          case 0:
            v62 = (char *)MEMORY[0x4A01AA](0x240u);
            v123 = 14;
            if ( v62 )
              v63 = sub_449480(v62);
            else
              v63 = 0;
            v64 = *(void (__thiscall ***)(_DWORD, _DWORD))v63;
            v108 = *((_DWORD *)this + 34);
            v86 = *((_DWORD *)this + 33);
            v123 = -1;
            if ( !((int (__thiscall *)(char *, int, int, int, int, int, char *, _DWORD, _DWORD))v64[1])(
                    v63,
                    595,
                    v86,
                    v108,
                    v124,
                    v61,
                    this,
                    0,
                    0) )
              goto LABEL_276;
            if ( dword_59A9F8 )
              goto LABEL_274;
            v65 = MEMORY[0x4A01AA](0x10u);
            if ( !v65 )
              goto LABEL_271;
            *v65 = &off_4AF460;
            v65[1] = 0;
            v65[3] = 0;
            v65[2] = 0;
            goto LABEL_272;
          case 1:
            v66 = (char *)MEMORY[0x4A01AA](0x240u);
            v123 = 15;
            if ( v66 )
              v63 = sub_449480(v66);
            else
              v63 = 0;
            v67 = *(void (__thiscall ***)(_DWORD, _DWORD))v63;
            v109 = *((_DWORD *)this + 34);
            v87 = *((_DWORD *)this + 33);
            v123 = -1;
            v68 = ((int (__thiscall *)(char *, int, int, int, int, int, char *, _DWORD, int))v67[1])(
                    v63,
                    595,
                    v87,
                    v109,
                    v124 + 2,
                    v61,
                    this,
                    0,
                    70);
            goto LABEL_248;
          case 2:
            v70 = (char *)MEMORY[0x4A01AA](0x240u);
            v123 = 16;
            if ( v70 )
              v63 = sub_449480(v70);
            else
              v63 = 0;
            v71 = *(void (__thiscall ***)(_DWORD, _DWORD))v63;
            v110 = *((_DWORD *)this + 34);
            v88 = *((_DWORD *)this + 33);
            v123 = -1;
            v68 = ((int (__thiscall *)(char *, int, int, int, int, int, char *, _DWORD, int))v71[1])(
                    v63,
                    595,
                    v88,
                    v110,
                    v124,
                    v61 + 2,
                    this,
                    0,
                    140);
            goto LABEL_248;
          case 3:
            v72 = (char *)MEMORY[0x4A01AA](0x240u);
            v123 = 17;
            if ( v72 )
              v63 = sub_449480(v72);
            else
              v63 = 0;
            v73 = *(void (__thiscall ***)(_DWORD, _DWORD))v63;
            v111 = *((_DWORD *)this + 34);
            v89 = *((_DWORD *)this + 33);
            v123 = -1;
            v68 = ((int (__thiscall *)(char *, int, int, int, int, int, char *, _DWORD, int))v73[1])(
                    v63,
                    595,
                    v89,
                    v111,
                    v124 + 2,
                    v61 + 2,
                    this,
                    0,
                    210);
            goto LABEL_248;
          case 4:
            v74 = (char *)MEMORY[0x4A01AA](0x240u);
            v123 = 18;
            if ( v74 )
              v63 = sub_449480(v74);
            else
              v63 = 0;
            v75 = *(void (__thiscall ***)(_DWORD, _DWORD))v63;
            v112 = *((_DWORD *)this + 34);
            v90 = *((_DWORD *)this + 33);
            v123 = -1;
            v68 = ((int (__thiscall *)(char *, int, int, int, int, int, char *, _DWORD, int))v75[1])(
                    v63,
                    595,
                    v90,
                    v112,
                    v124 + 3,
                    v61 + 2,
                    this,
                    0,
                    280);
            goto LABEL_248;
          case 5:
            v76 = (char *)MEMORY[0x4A01AA](0x240u);
            v123 = 19;
            if ( v76 )
              v63 = sub_449480(v76);
            else
              v63 = 0;
            v77 = *(void (__thiscall ***)(_DWORD, _DWORD))v63;
            v113 = *((_DWORD *)this + 34);
            v91 = *((_DWORD *)this + 33);
            v123 = -1;
            if ( !((int (__thiscall *)(char *, int, int, int, int, int, char *, _DWORD, int))v77[1])(
                    v63,
                    595,
                    v91,
                    v113,
                    v124 - 2,
                    v61,
                    this,
                    0,
                    350) )
            {
LABEL_276:
              if ( !v63 )
                goto LABEL_306;
              goto LABEL_305;
            }
            if ( !dword_59A9F8 )
            {
              v65 = MEMORY[0x4A01AA](0x10u);
              if ( v65 )
              {
                *v65 = &off_4AF460;
                v65[1] = 0;
                v65[3] = 0;
                v65[2] = 0;
              }
              else
              {
LABEL_271:
                v65 = 0;
              }
LABEL_272:
              dword_59A9F8 = (int)v65;
              if ( v65 )
              {
                v65[1] = v63;
                dword_59A9FC = dword_59A9F8;
                ++dword_59AA08;
              }
              goto LABEL_306;
            }
LABEL_274:
            v78 = MEMORY[0x4A01AA](0x10u);
            if ( !v78 )
              goto LABEL_306;
            goto LABEL_296;
          case 6:
            v79 = (char *)MEMORY[0x4A01AA](0x240u);
            v123 = 20;
            if ( v79 )
              v63 = sub_449480(v79);
            else
              v63 = 0;
            v80 = *(void (__thiscall ***)(_DWORD, _DWORD))v63;
            v114 = *((_DWORD *)this + 34);
            v92 = *((_DWORD *)this + 33);
            v123 = -1;
            if ( !((int (__thiscall *)(char *, int, int, int, int, int, char *, _DWORD, int))v80[1])(
                    v63,
                    595,
                    v92,
                    v114,
                    v124,
                    v61 - 2,
                    this,
                    0,
                    420) )
              goto LABEL_298;
            if ( dword_59A9F8 )
              goto LABEL_295;
            v81 = MEMORY[0x4A01AA](0x10u);
            if ( v81 )
            {
              *v81 = &off_4AF460;
              v81[1] = 0;
              v81[3] = 0;
              v81[2] = 0;
            }
            else
            {
              v81 = 0;
            }
            dword_59A9F8 = (int)v81;
            if ( !v81 )
              goto LABEL_306;
            v81[1] = v63;
            v78 = (_DWORD *)dword_59A9F8;
            goto LABEL_297;
          case 7:
            v82 = (char *)MEMORY[0x4A01AA](0x240u);
            v123 = 21;
            if ( v82 )
              v63 = sub_449480(v82);
            else
              v63 = 0;
            v83 = *(void (__thiscall ***)(_DWORD, _DWORD))v63;
            v115 = *((_DWORD *)this + 34);
            v93 = *((_DWORD *)this + 33);
            v123 = -1;
            v68 = ((int (__thiscall *)(char *, int, int, int, int, int, char *, _DWORD, int))v83[1])(
                    v63,
                    595,
                    v93,
                    v115,
                    v124 - 2,
                    v61 - 2,
                    this,
                    0,
                    490);
LABEL_248:
            if ( v68 )
              goto LABEL_249;
LABEL_298:
            if ( v63 )
              goto LABEL_305;
            goto LABEL_306;
          case 8:
            v84 = (char *)MEMORY[0x4A01AA](0x240u);
            v123 = 22;
            if ( v84 )
              v63 = sub_449480(v84);
            else
              v63 = 0;
            v85 = *(void (__thiscall ***)(_DWORD, _DWORD))v63;
            v116 = *((_DWORD *)this + 34);
            v94 = *((_DWORD *)this + 33);
            v123 = -1;
            if ( ((int (__thiscall *)(char *, int, int, int, int, int, char *, _DWORD, int))v85[1])(
                   v63,
                   595,
                   v94,
                   v116,
                   v124 - 3,
                   v61 - 2,
                   this,
                   0,
                   560) )
            {
LABEL_249:
              if ( dword_59A9F8 )
              {
LABEL_295:
                v78 = MEMORY[0x4A01AA](0x10u);
                if ( v78 )
                {
LABEL_296:
                  *v78 = &off_4AF460;
                  v78[1] = 0;
                  v78[3] = 0;
                  v78[2] = 0;
                  v78[1] = v63;
                  v78[3] = 0;
                  v78[2] = dword_59A9FC;
                  *(_DWORD *)(dword_59A9FC + 12) = v78;
LABEL_297:
                  dword_59A9FC = (int)v78;
                  ++dword_59AA08;
                }
              }
              else
              {
                v69 = MEMORY[0x4A01AA](0x10u);
                if ( v69 )
                {
                  *v69 = &off_4AF460;
                  v69[1] = 0;
                  v69[3] = 0;
                  v69[2] = 0;
                }
                else
                {
                  v69 = 0;
                }
                dword_59A9F8 = (int)v69;
                if ( v69 )
                {
                  v69[1] = v63;
                  dword_59A9FC = dword_59A9F8;
                  ++dword_59AA08;
                }
              }
            }
            else if ( v63 )
            {
LABEL_305:
              (**(void (__thiscall ***)(char *, int))v63)(v63, 1);
            }
LABEL_306:
            result = ++v120;
            if ( v120 >= 9 )
              return result;
            continue;
          default:
            goto LABEL_306;
        }
      }
    case 176:
      v48 = (char *)MEMORY[0x4A01AA](0x240u);
      v123 = 12;
      if ( v48 )
        v117 = sub_449480(v48);
      else
        v117 = 0;
      v49 = *(void (__thiscall ***)(_DWORD, _DWORD))v117;
      v106 = *((_DWORD *)this + 34);
      v50 = *((_DWORD *)this + 33);
      v123 = -1;
      if ( !((int (__thiscall *)(char *, int, int, int, _DWORD, _DWORD, char *, int, int))v49[1])(
              v117,
              23,
              v50,
              v106,
              v4,
              v3,
              this,
              v124,
              700) )
      {
        v11 = (int)v117;
        if ( v117 )
          (**(void (__thiscall ***)(char *, int))v117)(v117, 1);
        goto LABEL_224;
      }
      if ( !dword_59A9F8 )
      {
        v10 = MEMORY[0x4A01AA](0x10u);
        if ( v10 )
        {
          *v10 = &off_4AF460;
          v10[1] = 0;
          v10[3] = 0;
          v10[2] = 0;
        }
        else
        {
          v10 = 0;
        }
        dword_59A9F8 = (int)v10;
        if ( v10 )
        {
LABEL_193:
          v51 = v117;
          v10[1] = v117;
          dword_59A9FC = dword_59A9F8;
          ++dword_59AA08;
          LOBYTE(v51) = this[121];
          return (*(int (__thiscall **)(char *, int, char *, _DWORD))(*(_DWORD *)this + 16))(this, 2, v51, 0);
        }
        goto LABEL_224;
      }
      v12 = MEMORY[0x4A01AA](0x10u);
      if ( !v12 )
        goto LABEL_224;
LABEL_195:
      *v12 = &off_4AF460;
      v12[1] = 0;
      v12[3] = 0;
      v12[2] = 0;
      v12[1] = v117;
      v12[3] = 0;
      v12[2] = dword_59A9FC;
      v52 = dword_59A9FC;
      *(_DWORD *)(dword_59A9FC + 12) = v12;
      dword_59A9FC = (int)v12;
      ++dword_59AA08;
      LOBYTE(v52) = this[121];
      return (*(int (__thiscall **)(char *, int, int, _DWORD))(*(_DWORD *)this + 16))(this, 2, v52, 0);
    default:
      return result;
  }
}
char __thiscall sub_40CBF0(char *this, int a2)
{
  int v2; // eax
  unsigned __int16 v4; // dx
  int v5; // edx
  _DWORD *v6; // eax
  char *v7; // eax
  char *v8; // ebx
  int (__thiscall **v9)(_DWORD, _DWORD); // edx
  char *v10; // eax
  int (__thiscall **v11)(_DWORD, _DWORD); // edx
  char *v12; // eax
  int (__thiscall **v13)(_DWORD, _DWORD); // edx
  char *v14; // eax
  int (__thiscall **v15)(_DWORD, _DWORD); // edx
  char *v16; // eax
  int (__thiscall **v17)(_DWORD, _DWORD); // edx
  char *v18; // eax
  int (__thiscall **v19)(_DWORD, _DWORD); // edx
  char *v20; // eax
  int (__thiscall **v21)(_DWORD, _DWORD); // edx
  char *v22; // eax
  int (__thiscall **v23)(_DWORD, _DWORD); // edx
  char *v24; // eax
  int (__thiscall **v25)(_DWORD, _DWORD); // edx
  char *v26; // eax
  int (__thiscall **v27)(_DWORD, _DWORD); // edx
  char *v28; // eax
  int (__thiscall **v29)(_DWORD, _DWORD); // edx
  char *v30; // eax
  int (__thiscall **v31)(_DWORD, _DWORD); // edx
  char *v32; // eax
  int (__thiscall **v33)(_DWORD, _DWORD); // edx
  char *v34; // eax
  int (__thiscall **v35)(_DWORD, _DWORD); // edx
  char *v36; // eax
  char *v37; // ebx
  void (__thiscall **v38)(_DWORD, _DWORD); // edx
  _DWORD *v39; // eax
  _DWORD *v40; // eax
  int v42; // [esp-14h] [ebp-40h]
  int v43; // [esp-14h] [ebp-40h]
  int v44; // [esp-14h] [ebp-40h]
  int v45; // [esp-14h] [ebp-40h]
  int v46; // [esp-14h] [ebp-40h]
  int v47; // [esp-14h] [ebp-40h]
  int v48; // [esp-14h] [ebp-40h]
  int v49; // [esp-14h] [ebp-40h]
  int v50; // [esp-14h] [ebp-40h]
  int v51; // [esp-14h] [ebp-40h]
  int v52; // [esp-14h] [ebp-40h]
  int v53; // [esp-14h] [ebp-40h]
  int v54; // [esp-14h] [ebp-40h]
  int v55; // [esp-14h] [ebp-40h]
  int v56; // [esp-10h] [ebp-3Ch]
  int v57; // [esp-10h] [ebp-3Ch]
  int v58; // [esp-10h] [ebp-3Ch]
  int v59; // [esp-10h] [ebp-3Ch]
  int v60; // [esp-10h] [ebp-3Ch]
  int v61; // [esp-10h] [ebp-3Ch]
  int v62; // [esp-10h] [ebp-3Ch]
  int v63; // [esp-10h] [ebp-3Ch]
  int v64; // [esp-10h] [ebp-3Ch]
  int v65; // [esp-10h] [ebp-3Ch]
  int v66; // [esp-10h] [ebp-3Ch]
  int v67; // [esp-10h] [ebp-3Ch]
  int v68; // [esp-10h] [ebp-3Ch]
  int v69; // [esp-10h] [ebp-3Ch]
  int v70; // [esp+10h] [ebp-1Ch] BYREF
  int v71; // [esp+14h] [ebp-18h]
  int v72; // [esp+18h] [ebp-14h] BYREF
  int v73; // [esp+28h] [ebp-4h]

  v2 = a2;
  v4 = *(_WORD *)(a2 + 6);
  LOBYTE(a2) = *(_BYTE *)(a2 + 10);
  *((_DWORD *)this + 33) = v4;
  v5 = *(_DWORD *)this;
  *((_DWORD *)this + 34) = *(unsigned __int16 *)(v2 + 8);
  (*(void (__thiscall **)(char *, int, int, _DWORD))(v5 + 16))(this, 2, a2, 0);
  LOBYTE(v6) = this[69];
  if ( (_BYTE)v6 == 3 )
  {
    LOWORD(v6) = *(_WORD *)(this + 71) - 5;
    switch ( *(_WORD *)(this + 71) )
    {
      case 5:
        v10 = (char *)MEMORY[0x4A01AA](0x240u);
        v73 = 1;
        if ( v10 )
          v8 = sub_449480(v10);
        else
          v8 = 0;
        v11 = *(int (__thiscall ***)(_DWORD, _DWORD))v8;
        v57 = *((_DWORD *)this + 34);
        v43 = *((_DWORD *)this + 33);
        v73 = -1;
        v6 = (_DWORD *)((int (__thiscall *)(char *, int, int, int, int, int, char *, _DWORD, _DWORD))v11[1])(
                         v8,
                         502,
                         v43,
                         v57,
                         v43,
                         v57,
                         this,
                         0,
                         0);
        if ( !v6 )
          goto LABEL_107;
        if ( dword_59A9F8 )
          goto LABEL_104;
        v6 = MEMORY[0x4A01AA](0x10u);
        if ( !v6 )
          goto LABEL_101;
        *v6 = &off_4AF460;
        v6[1] = 0;
        v6[3] = 0;
        v6[2] = 0;
        goto LABEL_102;
      case 9:
        v7 = (char *)MEMORY[0x4A01AA](0x240u);
        v73 = 0;
        if ( v7 )
          v8 = sub_449480(v7);
        else
          v8 = 0;
        v9 = *(int (__thiscall ***)(_DWORD, _DWORD))v8;
        v56 = *((_DWORD *)this + 34);
        v42 = *((_DWORD *)this + 33);
        v73 = -1;
        v6 = (_DWORD *)((int (__thiscall *)(char *, int, int, int, int, int, char *, _DWORD, _DWORD))v9[1])(
                         v8,
                         501,
                         v42,
                         v56,
                         v42,
                         v56,
                         this,
                         0,
                         0);
        if ( !v6 )
          goto LABEL_107;
        if ( dword_59A9F8 )
          goto LABEL_104;
        v6 = MEMORY[0x4A01AA](0x10u);
        if ( !v6 )
          goto LABEL_101;
        *v6 = &off_4AF460;
        v6[1] = 0;
        v6[3] = 0;
        v6[2] = 0;
        goto LABEL_102;
      case 0x13:
        v12 = (char *)MEMORY[0x4A01AA](0x240u);
        v73 = 2;
        if ( v12 )
          v8 = sub_449480(v12);
        else
          v8 = 0;
        v13 = *(int (__thiscall ***)(_DWORD, _DWORD))v8;
        v58 = *((_DWORD *)this + 34);
        v44 = *((_DWORD *)this + 33);
        v73 = -1;
        v6 = (_DWORD *)((int (__thiscall *)(char *, int, int, int, int, int, char *, _DWORD, _DWORD))v13[1])(
                         v8,
                         511,
                         v44,
                         v58,
                         v44,
                         v58,
                         this,
                         0,
                         0);
        if ( !v6 )
          goto LABEL_107;
        if ( dword_59A9F8 )
          goto LABEL_104;
        v6 = MEMORY[0x4A01AA](0x10u);
        if ( !v6 )
          goto LABEL_101;
        *v6 = &off_4AF460;
        v6[1] = 0;
        v6[3] = 0;
        v6[2] = 0;
        goto LABEL_102;
      case 0x25:
        v28 = (char *)MEMORY[0x4A01AA](0x240u);
        v73 = 10;
        if ( v28 )
          v8 = sub_449480(v28);
        else
          v8 = 0;
        v29 = *(int (__thiscall ***)(_DWORD, _DWORD))v8;
        v66 = *((_DWORD *)this + 34);
        v52 = *((_DWORD *)this + 33);
        v73 = -1;
        v6 = (_DWORD *)((int (__thiscall *)(char *, int, int, int, int, int, char *, int, int))v29[1])(
                         v8,
                         535,
                         v52,
                         v66,
                         v52,
                         v66,
                         this,
                         -1,
                         300);
        if ( !v6 )
          goto LABEL_107;
        if ( dword_59A9F8 )
          goto LABEL_104;
        v6 = MEMORY[0x4A01AA](0x10u);
        if ( !v6 )
          goto LABEL_101;
        *v6 = &off_4AF460;
        v6[1] = 0;
        v6[3] = 0;
        v6[2] = 0;
        goto LABEL_102;
      case 0x2B:
        v18 = (char *)MEMORY[0x4A01AA](0x240u);
        v73 = 5;
        if ( v18 )
          v8 = sub_449480(v18);
        else
          v8 = 0;
        v19 = *(int (__thiscall ***)(_DWORD, _DWORD))v8;
        v61 = *((_DWORD *)this + 34);
        v47 = *((_DWORD *)this + 33);
        v73 = -1;
        v6 = (_DWORD *)((int (__thiscall *)(char *, int, int, int, int, int, char *, _DWORD, _DWORD))v19[1])(
                         v8,
                         521,
                         v47,
                         v61,
                         v47,
                         v61,
                         this,
                         0,
                         0);
        if ( !v6 )
          goto LABEL_107;
        if ( dword_59A9F8 )
          goto LABEL_104;
        v6 = MEMORY[0x4A01AA](0x10u);
        if ( !v6 )
          goto LABEL_101;
        *v6 = &off_4AF460;
        v6[1] = 0;
        v6[3] = 0;
        v6[2] = 0;
        goto LABEL_102;
      case 0x2D:
        v14 = (char *)MEMORY[0x4A01AA](0x240u);
        v73 = 3;
        if ( v14 )
          v8 = sub_449480(v14);
        else
          v8 = 0;
        v15 = *(int (__thiscall ***)(_DWORD, _DWORD))v8;
        v59 = *((_DWORD *)this + 34);
        v45 = *((_DWORD *)this + 33);
        v73 = -1;
        v6 = (_DWORD *)((int (__thiscall *)(char *, int, int, int, int, int, char *, _DWORD, _DWORD))v15[1])(
                         v8,
                         513,
                         v45,
                         v59,
                         v45,
                         v59,
                         this,
                         0,
                         0);
        if ( !v6 )
          goto LABEL_107;
        if ( dword_59A9F8 )
          goto LABEL_104;
        v6 = MEMORY[0x4A01AA](0x10u);
        if ( !v6 )
          goto LABEL_101;
        *v6 = &off_4AF460;
        v6[1] = 0;
        v6[3] = 0;
        v6[2] = 0;
        goto LABEL_102;
      case 0x2F:
        v26 = (char *)MEMORY[0x4A01AA](0x240u);
        v73 = 9;
        if ( v26 )
          v8 = sub_449480(v26);
        else
          v8 = 0;
        v27 = *(int (__thiscall ***)(_DWORD, _DWORD))v8;
        v65 = *((_DWORD *)this + 34);
        v51 = *((_DWORD *)this + 33);
        v73 = -1;
        v6 = (_DWORD *)((int (__thiscall *)(char *, int, int, int, int, int, char *, int, int))v27[1])(
                         v8,
                         536,
                         v51,
                         v65,
                         v51,
                         v65,
                         this,
                         -1,
                         300);
        if ( !v6 )
          goto LABEL_107;
        if ( dword_59A9F8 )
          goto LABEL_104;
        v6 = MEMORY[0x4A01AA](0x10u);
        if ( !v6 )
          goto LABEL_101;
        *v6 = &off_4AF460;
        v6[1] = 0;
        v6[3] = 0;
        v6[2] = 0;
        goto LABEL_102;
      case 0x43:
        v22 = (char *)MEMORY[0x4A01AA](0x240u);
        v73 = 7;
        if ( v22 )
          v8 = sub_449480(v22);
        else
          v8 = 0;
        v23 = *(int (__thiscall ***)(_DWORD, _DWORD))v8;
        v63 = *((_DWORD *)this + 34);
        v49 = *((_DWORD *)this + 33);
        v73 = -1;
        v6 = (_DWORD *)((int (__thiscall *)(char *, int, int, int, int, int, char *, int, _DWORD))v23[1])(
                         v8,
                         522,
                         v49,
                         v63,
                         v49,
                         v63,
                         this,
                         200,
                         0);
        if ( !v6 )
          goto LABEL_107;
        if ( dword_59A9F8 )
          goto LABEL_104;
        v6 = MEMORY[0x4A01AA](0x10u);
        if ( !v6 )
          goto LABEL_101;
        *v6 = &off_4AF460;
        v6[1] = 0;
        v6[3] = 0;
        v6[2] = 0;
        goto LABEL_102;
      case 0x4B:
        v24 = (char *)MEMORY[0x4A01AA](0x240u);
        v73 = 8;
        if ( v24 )
          v8 = sub_449480(v24);
        else
          v8 = 0;
        v25 = *(int (__thiscall ***)(_DWORD, _DWORD))v8;
        v64 = *((_DWORD *)this + 34);
        v50 = *((_DWORD *)this + 33);
        v73 = -1;
        v6 = (_DWORD *)((int (__thiscall *)(char *, int, int, int, int, int, char *, int, int))v25[1])(
                         v8,
                         529,
                         v50,
                         v64,
                         v50,
                         v64,
                         this,
                         -1,
                         400);
        if ( !v6 )
          goto LABEL_107;
        if ( dword_59A9F8 )
          goto LABEL_104;
        v6 = MEMORY[0x4A01AA](0x10u);
        if ( !v6 )
          goto LABEL_101;
        *v6 = &off_4AF460;
        v6[1] = 0;
        v6[3] = 0;
        v6[2] = 0;
        goto LABEL_102;
      case 0x55:
        v20 = (char *)MEMORY[0x4A01AA](0x240u);
        v73 = 6;
        if ( v20 )
          v8 = sub_449480(v20);
        else
          v8 = 0;
        v21 = *(int (__thiscall ***)(_DWORD, _DWORD))v8;
        v62 = *((_DWORD *)this + 34);
        v48 = *((_DWORD *)this + 33);
        v73 = -1;
        v6 = (_DWORD *)((int (__thiscall *)(char *, int, int, int, int, int, char *, _DWORD, _DWORD))v21[1])(
                         v8,
                         516,
                         v48,
                         v62,
                         v48,
                         v62,
                         this,
                         0,
                         0);
        if ( !v6 )
          goto LABEL_107;
        if ( dword_59A9F8 )
          goto LABEL_104;
        v6 = MEMORY[0x4A01AA](0x10u);
        if ( !v6 )
          goto LABEL_101;
        *v6 = &off_4AF460;
        v6[1] = 0;
        v6[3] = 0;
        v6[2] = 0;
        goto LABEL_102;
      case 0x5A:
        v16 = (char *)MEMORY[0x4A01AA](0x240u);
        v73 = 4;
        if ( v16 )
          v8 = sub_449480(v16);
        else
          v8 = 0;
        v17 = *(int (__thiscall ***)(_DWORD, _DWORD))v8;
        v60 = *((_DWORD *)this + 34);
        v46 = *((_DWORD *)this + 33);
        v73 = -1;
        v6 = (_DWORD *)((int (__thiscall *)(char *, int, int, int, int, int, char *, _DWORD, int))v17[1])(
                         v8,
                         514,
                         v46,
                         v60,
                         v46,
                         v60,
                         this,
                         0,
                         900);
        if ( !v6 )
          goto LABEL_107;
        if ( dword_59A9F8 )
          goto LABEL_104;
        v6 = MEMORY[0x4A01AA](0x10u);
        if ( !v6 )
          goto LABEL_101;
        *v6 = &off_4AF460;
        v6[1] = 0;
        v6[3] = 0;
        v6[2] = 0;
        goto LABEL_102;
      case 0x5E:
        v30 = (char *)MEMORY[0x4A01AA](0x240u);
        v73 = 11;
        if ( v30 )
          v8 = sub_449480(v30);
        else
          v8 = 0;
        v31 = *(int (__thiscall ***)(_DWORD, _DWORD))v8;
        v67 = *((_DWORD *)this + 34);
        v53 = *((_DWORD *)this + 33);
        v73 = -1;
        v6 = (_DWORD *)((int (__thiscall *)(char *, int, int, int, int, int, char *, _DWORD, _DWORD))v31[1])(
                         v8,
                         524,
                         v53,
                         v67,
                         v53,
                         v67,
                         this,
                         0,
                         0);
        if ( !v6 )
          goto LABEL_107;
        if ( dword_59A9F8 )
          goto LABEL_104;
        v6 = MEMORY[0x4A01AA](0x10u);
        if ( !v6 )
          goto LABEL_101;
        *v6 = &off_4AF460;
        v6[1] = 0;
        v6[3] = 0;
        v6[2] = 0;
        goto LABEL_102;
      case 0x61:
        sub_453590(&dword_5B03B0, *((_DWORD *)this + 33), *((_DWORD *)this + 34), this[121], 1, &v70);
        v32 = (char *)MEMORY[0x4A01AA](0x240u);
        v73 = 12;
        if ( v32 )
          v8 = sub_449480(v32);
        else
          v8 = 0;
        v33 = *(int (__thiscall ***)(_DWORD, _DWORD))v8;
        v73 = -1;
        v6 = (_DWORD *)((int (__thiscall *)(char *, int, int, int, int, int, char *, int, int))v33[1])(
                         v8,
                         543,
                         v70,
                         v71,
                         v70,
                         v71,
                         this,
                         -1,
                         600);
        if ( !v6 )
          goto LABEL_107;
        if ( dword_59A9F8 )
          goto LABEL_104;
        v6 = MEMORY[0x4A01AA](0x10u);
        if ( !v6 )
          goto LABEL_101;
        *v6 = &off_4AF460;
        v6[1] = 0;
        v6[3] = 0;
        v6[2] = 0;
        goto LABEL_102;
      case 0x62:
        LOBYTE(v6) = sub_40DB10((int)this, (int)dword_59AA6C, 530, a2, -1);
        goto LABEL_136;
      case 0x71:
        LOBYTE(v6) = sub_40DB10((int)this, (int)dword_59AA6C, 537, a2, -1);
        goto LABEL_136;
      case 0x78:
        LOBYTE(v6) = sub_40DB10((int)this, (int)dword_59AA6C, 532, a2, -1);
        goto LABEL_136;
      case 0x82:
        LOBYTE(v6) = sub_40DB10((int)this, (int)dword_59AA6C, 546, a2, -1);
        goto LABEL_136;
      case 0x83:
        LOBYTE(v6) = sub_40DB10((int)this, (int)dword_59AA6C, 548, a2, -1);
        goto LABEL_136;
      case 0x84:
        sub_453590(&dword_5B03B0, *((_DWORD *)this + 33), *((_DWORD *)this + 34), this[121], 1, &v70);
        v34 = (char *)MEMORY[0x4A01AA](0x240u);
        v73 = 13;
        if ( v34 )
          v8 = sub_449480(v34);
        else
          v8 = 0;
        v35 = *(int (__thiscall ***)(_DWORD, _DWORD))v8;
        v68 = *((_DWORD *)this + 34);
        v54 = *((_DWORD *)this + 33);
        v73 = -1;
        v6 = (_DWORD *)((int (__thiscall *)(char *, int, int, int, int, int, char *, int, int))v35[1])(
                         v8,
                         551,
                         v54,
                         v68,
                         v54,
                         v68,
                         this,
                         -1,
                         600);
        if ( v6 )
        {
          if ( dword_59A9F8 )
          {
LABEL_104:
            v6 = MEMORY[0x4A01AA](0x10u);
            if ( !v6 )
              return (char)v6;
            *v6 = &off_4AF460;
            v6[1] = 0;
            v6[3] = 0;
            v6[2] = 0;
            v6[1] = v8;
            v6[3] = 0;
            v6[2] = dword_59A9FC;
            *(_DWORD *)(dword_59A9FC + 12) = v6;
            goto LABEL_106;
          }
          v6 = MEMORY[0x4A01AA](0x10u);
          if ( v6 )
          {
            *v6 = &off_4AF460;
            v6[1] = 0;
            v6[3] = 0;
            v6[2] = 0;
          }
          else
          {
LABEL_101:
            v6 = 0;
          }
LABEL_102:
          dword_59A9F8 = (int)v6;
          if ( v6 )
          {
            v6[1] = v8;
            v6 = (_DWORD *)dword_59A9F8;
LABEL_106:
            dword_59A9FC = (int)v6;
            LOBYTE(v6) = ++dword_59AA08;
            return (char)v6;
          }
        }
        else
        {
LABEL_107:
          if ( v8 )
            LOBYTE(v6) = (**(int (__thiscall ***)(char *, int))v8)(v8, 1);
        }
        return (char)v6;
      case 0x8B:
        LOBYTE(v6) = sub_40DB10((int)this, (int)dword_59AA6C, 554, a2, -1);
        goto LABEL_136;
      case 0x8D:
        LOBYTE(v6) = (*(int (__thiscall **)(char *, int, int, _DWORD))(*(_DWORD *)this + 16))(this, 6, a2, 0);
        return (char)v6;
      case 0x96:
        LOBYTE(v6) = sub_40DB10((int)this, (int)dword_59AA6C, 570, a2, -1);
        goto LABEL_136;
      case 0x97:
        sub_453590(&dword_5B03B0, *((_DWORD *)this + 33), *((_DWORD *)this + 34), this[121], 1, &v72);
        v36 = (char *)MEMORY[0x4A01AA](0x240u);
        v73 = 14;
        if ( v36 )
          v37 = sub_449480(v36);
        else
          v37 = 0;
        v38 = *(void (__thiscall ***)(_DWORD, _DWORD))v37;
        v69 = *((_DWORD *)this + 34);
        v55 = *((_DWORD *)this + 33);
        v73 = -1;
        if ( ((int (__thiscall *)(char *, int, int, int, int, int, char *, int, _DWORD))v38[1])(
               v37,
               575,
               v55,
               v69,
               v55,
               v69,
               this,
               -1,
               0) )
        {
          if ( dword_59A9F8 )
          {
            v40 = MEMORY[0x4A01AA](0x10u);
            if ( v40 )
            {
              *v40 = &off_4AF460;
              v40[1] = 0;
              v40[3] = 0;
              v40[2] = 0;
              v40[1] = v37;
              v40[3] = 0;
              v40[2] = dword_59A9FC;
              *(_DWORD *)(dword_59A9FC + 12) = v40;
              goto LABEL_130;
            }
          }
          else
          {
            v39 = MEMORY[0x4A01AA](0x10u);
            if ( v39 )
            {
              *v39 = &off_4AF460;
              v39[1] = 0;
              v39[3] = 0;
              v39[2] = 0;
            }
            else
            {
              v39 = 0;
            }
            dword_59A9F8 = (int)v39;
            if ( v39 )
            {
              v39[1] = v37;
              v40 = (_DWORD *)dword_59A9F8;
LABEL_130:
              dword_59A9FC = (int)v40;
              ++dword_59AA08;
              LOBYTE(v6) = (*(int (__thiscall **)(char *, int, int, _DWORD))(*(_DWORD *)this + 16))(this, 5, a2, 0);
              goto LABEL_136;
            }
          }
        }
        else if ( v37 )
        {
          (**(void (__thiscall ***)(char *, int))v37)(v37, 1);
        }
        LOBYTE(v6) = (*(int (__thiscall **)(char *, int, int, _DWORD))(*(_DWORD *)this + 16))(this, 5, a2, 0);
LABEL_136:
        *((_DWORD *)this + 100060) = 1;
        break;
      case 0x98:
        LOBYTE(v6) = sub_40DB10((int)this, (int)dword_59AA6C, 572, a2, -1);
        goto LABEL_136;
      case 0xA4:
        LOBYTE(v6) = sub_40DB10((int)this, (int)dword_59AA6C, 591, a2, -1);
        goto LABEL_136;
      case 0xBA:
        LOBYTE(v6) = sub_40DB10((int)this, (int)dword_59AA6C, 608, a2, -1);
        goto LABEL_136;
      default:
        return (char)v6;
    }
  }
  return (char)v6;
}
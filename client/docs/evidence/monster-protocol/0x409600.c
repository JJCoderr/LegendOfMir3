char __thiscall sub_409600(char *this, const char *a2)
{
  double v2; // st7
  unsigned __int16 v4; // bx
  double v5; // st7
  int v6; // eax
  int v7; // eax
  char *v8; // eax
  char *v9; // ebx
  void (__thiscall **v10)(_DWORD, _DWORD); // edx
  _DWORD *v11; // eax
  _DWORD *v12; // eax
  int v13; // eax
  int v14; // ecx
  __int16 v15; // cx
  char *v16; // eax
  char *v17; // ebx
  int (__thiscall **v18)(_DWORD, _DWORD); // edx
  _DWORD *v19; // eax
  _DWORD *v20; // eax
  _DWORD *v21; // ebx
  char *v22; // eax
  int (__thiscall **v23)(_DWORD, _DWORD); // edx
  char *v24; // eax
  int (__thiscall **v25)(_DWORD, _DWORD); // edx
  char *v26; // eax
  int (__thiscall **v27)(_DWORD, _DWORD); // edx
  int v29; // [esp+20h] [ebp-40h]
  int v30; // [esp+20h] [ebp-40h]
  int v31; // [esp+20h] [ebp-40h]
  int v32; // [esp+20h] [ebp-40h]
  int v33; // [esp+20h] [ebp-40h]
  int v34; // [esp+24h] [ebp-3Ch]
  int v35; // [esp+24h] [ebp-3Ch]
  int v36; // [esp+24h] [ebp-3Ch]
  int v37; // [esp+24h] [ebp-3Ch]
  int v38; // [esp+24h] [ebp-3Ch]
  int v39; // [esp+2Ch] [ebp-34h]
  int v40; // [esp+44h] [ebp-1Ch] BYREF
  int v41; // [esp+4Ch] [ebp-14h]
  unsigned int v42; // [esp+50h] [ebp-10h]
  int v43; // [esp+5Ch] [ebp-4h]

  v2 = flt_4AF468;
  if ( *((_WORD *)a2 + 4) )
    v2 = (double)*((unsigned __int16 *)a2 + 3) / (double)*((unsigned __int16 *)a2 + 4);
  v4 = *((_WORD *)a2 + 5);
  v5 = v2 * flt_4AF464;
  *(_WORD *)(*((_DWORD *)this + 100110) + 12) = *((_WORD *)a2 + 3);
  *(_WORD *)(*((_DWORD *)this + 100110) + 10) = *((_WORD *)a2 + 4);
  *(_BYTE *)(*((_DWORD *)this + 100110) + 9) = MEMORY[0x49FE1C](v5);
  if ( dword_8F1D30 )
  {
    if ( this[120] != 4 )
    {
      *(_DWORD *)&this[4 * *((_DWORD *)this + 100780) + 403024] = -v4;
      *(_DWORD *)&this[8 * *((_DWORD *)this + 100780) + 403056] = *((_DWORD *)this + 39);
      *(_DWORD *)&this[8 * *((_DWORD *)this + 100780) + 403060] = *((_DWORD *)this + 40);
      *(_WORD *)&this[2 * *((_DWORD *)this + 100780) + 403124] = 0;
      v6 = *((_DWORD *)this + 100780) + 1;
      *((_DWORD *)this + 100780) = v6;
      if ( v6 > 7 )
        *((_DWORD *)this + 100780) = 0;
    }
  }
  sub_47D780(a2 + 12, (int)&v40, 16);
  v7 = v40;
  if ( (_BYTE)v40 == 3 )
    v7 = sub_4064B0((int)this, v40);
  if ( *(_WORD *)(*((_DWORD *)this + 100110) + 12) || *((_DWORD *)this + 100108) )
  {
    if ( v4 )
    {
      LOBYTE(v7) = this[121];
      (*(void (__thiscall **)(char *, int, int, _DWORD))(*(_DWORD *)this + 16))(this, 3, v7, 0);
    }
  }
  else
  {
    LOBYTE(v7) = this[121];
    (*(void (__thiscall **)(char *, int, int, _DWORD))(*(_DWORD *)this + 16))(this, 4, v7, 0);
    sub_408C10(*(unsigned __int16 *)(this + 71));
  }
  if ( !*((_DWORD *)this + 100108) && v4 )
  {
    v8 = (char *)MEMORY[0x4A01AA](0x240u);
    v43 = 0;
    if ( v8 )
      v9 = sub_449480(v8);
    else
      v9 = 0;
    v10 = *(void (__thiscall ***)(_DWORD, _DWORD))v9;
    v39 = **((_DWORD **)this + 100110);
    v34 = *((_DWORD *)this + 34);
    v29 = *((_DWORD *)this + 33);
    v43 = -1;
    if ( ((int (__thiscall *)(char *, unsigned int, int, int, int, int, char *, int, _DWORD))v10[1])(
           v9,
           HIWORD(v42) + 400,
           v29,
           v34,
           v29,
           v34,
           this,
           v39,
           0) )
    {
      if ( dword_59A9F8 )
      {
        v12 = MEMORY[0x4A01AA](0x10u);
        if ( v12 )
        {
          *v12 = &off_4AF460;
          v12[1] = 0;
          v12[3] = 0;
          v12[2] = 0;
          v12[1] = v9;
          v12[3] = 0;
          v12[2] = dword_59A9FC;
          *(_DWORD *)(dword_59A9FC + 12) = v12;
          dword_59A9FC = (int)v12;
          ++dword_59AA08;
        }
      }
      else
      {
        v11 = MEMORY[0x4A01AA](0x10u);
        if ( v11 )
        {
          *v11 = &off_4AF460;
          v11[1] = 0;
          v11[3] = 0;
          v11[2] = 0;
        }
        else
        {
          v11 = 0;
        }
        dword_59A9F8 = (int)v11;
        if ( v11 )
        {
          v11[1] = v9;
          dword_59A9FC = dword_59A9F8;
          ++dword_59AA08;
        }
      }
    }
    else if ( v9 )
    {
      (**(void (__thiscall ***)(char *, int))v9)(v9, 1);
    }
  }
  v13 = *(unsigned __int16 *)(this + 71);
  if ( (_WORD)v13 == 161 )
  {
    LOBYTE(v13) = this[121];
    LOBYTE(v13) = sub_40DB10((int)this, (int)dword_59AA6C, 585, v13, -1);
    *((_DWORD *)this + 100060) = 1;
  }
  if ( v41 == *(_DWORD *)dword_823A58 )
  {
    *((_DWORD *)this + 100109) = dword_7C1E65;
  }
  else
  {
    v13 = dword_59A9E0;
    if ( dword_59A9E0 )
    {
      while ( 1 )
      {
        v14 = *(_DWORD *)(v13 + 4);
        if ( v14 )
        {
          if ( **(_DWORD **)(v14 + 400440) == v41 )
            break;
        }
        v13 = *(_DWORD *)(v13 + 12);
        if ( !v13 )
          goto LABEL_40;
      }
      v13 = *(_DWORD *)(v14 + 69);
      *((_DWORD *)this + 100109) = v13;
    }
  }
LABEL_40:
  if ( this[400436] == 3 )
  {
    v15 = *((_WORD *)this + 200219);
    LOBYTE(v13) = v15 - 2;
    switch ( v15 )
    {
      case 2:
        v22 = (char *)MEMORY[0x4A01AA](0x240u);
        v43 = 2;
        if ( v22 )
          v17 = sub_449480(v22);
        else
          v17 = 0;
        v23 = *(int (__thiscall ***)(_DWORD, _DWORD))v17;
        v36 = *((_DWORD *)this + 34);
        v31 = *((_DWORD *)this + 33);
        v43 = -1;
        v13 = ((int (__thiscall *)(char *, int, int, int, int, int, char *, _DWORD, _DWORD))v23[1])(
                v17,
                500,
                v31,
                v36,
                v31,
                v36,
                this,
                0,
                0);
        if ( !v13 )
          goto LABEL_83;
        if ( dword_59A9F8 )
          goto LABEL_80;
        v13 = (int)MEMORY[0x4A01AA](0x10u);
        if ( !v13 )
          goto LABEL_70;
        *(_DWORD *)v13 = &off_4AF460;
        *(_DWORD *)(v13 + 4) = 0;
        *(_DWORD *)(v13 + 12) = 0;
        *(_DWORD *)(v13 + 8) = 0;
        goto LABEL_71;
      case 67:
        v24 = (char *)MEMORY[0x4A01AA](0x240u);
        v43 = 3;
        if ( v24 )
          v17 = sub_449480(v24);
        else
          v17 = 0;
        v25 = *(int (__thiscall ***)(_DWORD, _DWORD))v17;
        v37 = *((_DWORD *)this + 34);
        v32 = *((_DWORD *)this + 33);
        v43 = -1;
        v13 = ((int (__thiscall *)(char *, int, int, int, int, int, char *, _DWORD, _DWORD))v25[1])(
                v17,
                523,
                v32,
                v37,
                v32,
                v37,
                this,
                0,
                0);
        if ( !v13 )
          goto LABEL_83;
        if ( dword_59A9F8 )
          goto LABEL_80;
        v13 = (int)MEMORY[0x4A01AA](0x10u);
        if ( !v13 )
          goto LABEL_70;
        *(_DWORD *)v13 = &off_4AF460;
        *(_DWORD *)(v13 + 4) = 0;
        *(_DWORD *)(v13 + 12) = 0;
        *(_DWORD *)(v13 + 8) = 0;
        goto LABEL_71;
      case 104:
        v16 = (char *)MEMORY[0x4A01AA](0x240u);
        v43 = 1;
        if ( v16 )
          v17 = sub_449480(v16);
        else
          v17 = 0;
        v18 = *(int (__thiscall ***)(_DWORD, _DWORD))v17;
        v35 = *((_DWORD *)this + 34);
        v30 = *((_DWORD *)this + 33);
        v43 = -1;
        v13 = ((int (__thiscall *)(char *, int, int, int, int, int, char *, _DWORD, _DWORD))v18[1])(
                v17,
                542,
                v30,
                v35,
                v30,
                v35,
                this,
                0,
                0);
        if ( v13 )
        {
          if ( dword_59A9F8 )
          {
            v20 = MEMORY[0x4A01AA](0x10u);
            if ( !v20 )
              goto LABEL_55;
            *v20 = &off_4AF460;
            v20[1] = 0;
            v20[3] = 0;
            v20[2] = 0;
            v20[1] = v17;
            v20[3] = 0;
            v20[2] = dword_59A9FC;
            *(_DWORD *)(dword_59A9FC + 12) = v20;
          }
          else
          {
            v19 = MEMORY[0x4A01AA](0x10u);
            if ( v19 )
            {
              *v19 = &off_4AF460;
              v19[1] = 0;
              v19[3] = 0;
              v19[2] = 0;
            }
            else
            {
              v19 = 0;
            }
            dword_59A9F8 = (int)v19;
            if ( !v19 )
            {
LABEL_55:
              v21 = MEMORY[0x4A01AA](0x20u);
              v21[1] = 0;
              *v21 = 0;
              v21[3] = ((int (__cdecl *)())dword_4AF2FC)();
              v21[5] = *((_DWORD *)this + 33);
              v21[6] = *((_DWORD *)this + 34);
              v21[4] = 15000;
              *((_WORD *)v21 + 4) = 250;
              *((_WORD *)v21 + 5) = 420;
              v21[7] = 0;
              if ( dword_59AA58 )
              {
                v13 = (int)MEMORY[0x4A01AA](0x10u);
                if ( v13 )
                {
                  *(_DWORD *)v13 = &off_4AF46C;
                  *(_DWORD *)(v13 + 4) = 0;
                  *(_DWORD *)(v13 + 12) = 0;
                  *(_DWORD *)(v13 + 8) = 0;
                  *(_DWORD *)(v13 + 4) = v21;
                  *(_DWORD *)(v13 + 12) = 0;
                  *(_DWORD *)(v13 + 8) = dword_59AA5C;
                  *(_DWORD *)(dword_59AA5C + 12) = v13;
                  dword_59AA5C = v13;
                  ++dword_59AA68;
                }
              }
              else
              {
                v13 = (int)MEMORY[0x4A01AA](0x10u);
                if ( v13 )
                {
                  *(_DWORD *)v13 = &off_4AF46C;
                  *(_DWORD *)(v13 + 4) = 0;
                  *(_DWORD *)(v13 + 12) = 0;
                  *(_DWORD *)(v13 + 8) = 0;
                }
                else
                {
                  v13 = 0;
                }
                dword_59AA58 = v13;
                if ( v13 )
                {
                  *(_DWORD *)(v13 + 4) = v21;
                  LOBYTE(v13) = dword_59AA68 + 1;
                  dword_59AA5C = dword_59AA58;
                  ++dword_59AA68;
                }
              }
              return v13;
            }
            v19[1] = v17;
            v20 = (_DWORD *)dword_59A9F8;
          }
          dword_59A9FC = (int)v20;
          ++dword_59AA08;
          goto LABEL_55;
        }
LABEL_83:
        if ( !v17 )
          return v13;
        goto LABEL_93;
      case 150:
        v26 = (char *)MEMORY[0x4A01AA](0x240u);
        v43 = 4;
        if ( v26 )
          v17 = sub_449480(v26);
        else
          v17 = 0;
        v27 = *(int (__thiscall ***)(_DWORD, _DWORD))v17;
        v38 = *((_DWORD *)this + 34);
        v33 = *((_DWORD *)this + 33);
        v43 = -1;
        v13 = ((int (__thiscall *)(char *, int, int, int, int, int, char *, _DWORD, _DWORD))v27[1])(
                v17,
                574,
                v33,
                v38,
                v33,
                v38,
                this,
                0,
                0);
        if ( v13 )
        {
          if ( dword_59A9F8 )
          {
LABEL_80:
            v13 = (int)MEMORY[0x4A01AA](0x10u);
            if ( !v13 )
              return v13;
            *(_DWORD *)v13 = &off_4AF460;
            *(_DWORD *)(v13 + 4) = 0;
            *(_DWORD *)(v13 + 12) = 0;
            *(_DWORD *)(v13 + 8) = 0;
            *(_DWORD *)(v13 + 4) = v17;
            *(_DWORD *)(v13 + 12) = 0;
            *(_DWORD *)(v13 + 8) = dword_59A9FC;
            *(_DWORD *)(dword_59A9FC + 12) = v13;
            goto LABEL_82;
          }
          v13 = (int)MEMORY[0x4A01AA](0x10u);
          if ( v13 )
          {
            *(_DWORD *)v13 = &off_4AF460;
            *(_DWORD *)(v13 + 4) = 0;
            *(_DWORD *)(v13 + 12) = 0;
            *(_DWORD *)(v13 + 8) = 0;
          }
          else
          {
LABEL_70:
            v13 = 0;
          }
LABEL_71:
          dword_59A9F8 = v13;
          if ( v13 )
          {
            *(_DWORD *)(v13 + 4) = v17;
            v13 = dword_59A9F8;
LABEL_82:
            dword_59A9FC = v13;
            LOBYTE(v13) = ++dword_59AA08;
            return v13;
          }
        }
        else if ( v17 )
        {
LABEL_93:
          LOBYTE(v13) = (**(int (__thiscall ***)(char *, int))v17)(v17, 1);
        }
        break;
      default:
        return v13;
    }
  }
  return v13;
}
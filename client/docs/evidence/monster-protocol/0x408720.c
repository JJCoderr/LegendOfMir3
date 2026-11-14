int __thiscall sub_408720(char *this, int a2)
{
  int v2; // eax
  char v4; // al
  unsigned __int16 v5; // ax
  _DWORD *v6; // esi
  _DWORD *v7; // eax
  _DWORD *v8; // eax
  _DWORD *v9; // eax
  char *v10; // eax
  char *v11; // esi
  _DWORD *v12; // eax
  char *v13; // eax
  _DWORD *v14; // eax
  int v15; // edx
  int v17; // [esp-4h] [ebp-20h]

  v2 = a2;
  v17 = a2;
  *((_DWORD *)this + 33) = *(unsigned __int16 *)(a2 + 6);
  *((_DWORD *)this + 34) = *(unsigned __int16 *)(a2 + 8);
  LOBYTE(a2) = *(_BYTE *)(a2 + 10);
  *(_BYTE *)(*((_DWORD *)this + 100110) + 20) = *(_BYTE *)(v2 + 11);
  (*(void (__thiscall **)(char *, int))(*(_DWORD *)this + 36))(this, v17);
  v4 = this[69];
  *((_DWORD *)this + 100098) = 0;
  if ( v4 == 3 )
    v5 = *(_WORD *)(this + 71);
  else
    v5 = (unsigned __int8)this[71];
  if ( v5 > 0x38u )
  {
    if ( v5 > 0x59u )
    {
      if ( v5 >= 0x212u && v5 <= 0x215u )
      {
        LOBYTE(a2) = 0;
        *((_DWORD *)this + 100098) = 0;
      }
      goto LABEL_53;
    }
    if ( v5 != 89 )
    {
      if ( v5 == 85 )
        LOBYTE(a2) = 0;
      goto LABEL_53;
    }
    v13 = (char *)MEMORY[0x4A01AA](0x240u);
    if ( v13 )
      v11 = sub_449480(v13);
    else
      v11 = 0;
    if ( (*(int (__thiscall **)(char *, int, _DWORD, _DWORD, _DWORD, _DWORD, _DWORD, _DWORD, _DWORD))(*(_DWORD *)v11 + 4))(
           v11,
           30,
           *((_DWORD *)this + 33),
           *((_DWORD *)this + 34),
           *((_DWORD *)this + 33),
           *((_DWORD *)this + 34),
           0,
           0,
           0) )
    {
      if ( !dword_59A9F8 )
      {
        v12 = MEMORY[0x4A01AA](0x10u);
        if ( v12 )
        {
          *v12 = &off_4AF460;
          v12[1] = 0;
          v12[3] = 0;
          v12[2] = 0;
          goto LABEL_43;
        }
LABEL_42:
        v12 = 0;
        goto LABEL_43;
      }
LABEL_45:
      v14 = MEMORY[0x4A01AA](0x10u);
      if ( !v14 )
        goto LABEL_53;
      *v14 = &off_4AF460;
      v14[1] = 0;
      v14[3] = 0;
      v14[2] = 0;
      v14[1] = v11;
      v14[3] = 0;
      v14[2] = dword_59A9FC;
      *(_DWORD *)(dword_59A9FC + 12) = v14;
      goto LABEL_47;
    }
LABEL_48:
    if ( v11 )
      (**(void (__thiscall ***)(char *, int))v11)(v11, 1);
    goto LABEL_53;
  }
  if ( v5 == 56 )
  {
    v10 = (char *)MEMORY[0x4A01AA](0x240u);
    if ( v10 )
      v11 = sub_449480(v10);
    else
      v11 = 0;
    if ( (*(int (__thiscall **)(char *, int, _DWORD, _DWORD, _DWORD, _DWORD, _DWORD, _DWORD, _DWORD))(*(_DWORD *)v11 + 4))(
           v11,
           17,
           *((_DWORD *)this + 33),
           *((_DWORD *)this + 34),
           *((_DWORD *)this + 33),
           *((_DWORD *)this + 34),
           0,
           0,
           0) )
    {
      if ( !dword_59A9F8 )
      {
        v12 = MEMORY[0x4A01AA](0x10u);
        if ( v12 )
        {
          *v12 = &off_4AF460;
          v12[1] = 0;
          v12[3] = 0;
          v12[2] = 0;
LABEL_43:
          dword_59A9F8 = (int)v12;
          if ( !v12 )
            goto LABEL_53;
          v12[1] = v11;
          v14 = (_DWORD *)dword_59A9F8;
LABEL_47:
          dword_59A9FC = (int)v14;
          ++dword_59AA08;
          goto LABEL_53;
        }
        goto LABEL_42;
      }
      goto LABEL_45;
    }
    goto LABEL_48;
  }
  switch ( v5 )
  {
    case 3u:
      goto LABEL_23;
    case 4u:
      v6 = MEMORY[0x4A01AA](0x20u);
      v6[1] = 0;
      *v6 = 0;
      v6[3] = ((int (__cdecl *)())dword_4AF2FC)();
      v6[5] = *((_DWORD *)this + 33);
      v6[6] = *((_DWORD *)this + 34);
      v6[4] = 60000;
      *((_WORD *)v6 + 4) = (unsigned __int8)a2 + 200;
      *((_WORD *)v6 + 5) = 1200;
      v6[7] = 0;
      if ( dword_59AA58 )
        goto LABEL_19;
      v7 = MEMORY[0x4A01AA](0x10u);
      if ( v7 )
      {
        *v7 = &off_4AF46C;
        v7[1] = 0;
        v7[3] = 0;
        v7[2] = 0;
      }
      else
      {
        v7 = 0;
      }
      dword_59AA58 = (int)v7;
      if ( v7 )
      {
        v7[1] = v6;
        dword_59AA5C = dword_59AA58;
        ++dword_59AA68;
      }
      break;
    case 0x1Fu:
      LOBYTE(a2) = 0;
LABEL_23:
      *((_DWORD *)this + 100098) = 1;
      break;
    case 0x31u:
      v6 = MEMORY[0x4A01AA](0x20u);
      v6[1] = 0;
      *v6 = 0;
      v6[3] = ((int (__cdecl *)())dword_4AF2FC)();
      v6[5] = *((_DWORD *)this + 33);
      v6[6] = *((_DWORD *)this + 34);
      v6[4] = 60000;
      *((_WORD *)v6 + 4) = (unsigned __int8)a2 + 240;
      *((_WORD *)v6 + 5) = 1200;
      v6[7] = 0;
      if ( dword_59AA58 )
      {
LABEL_19:
        v9 = MEMORY[0x4A01AA](0x10u);
        if ( !v9 )
          break;
        *v9 = &off_4AF46C;
        v9[1] = 0;
        v9[3] = 0;
        v9[2] = 0;
        v9[1] = v6;
        v9[3] = 0;
        v9[2] = dword_59AA5C;
        *(_DWORD *)(dword_59AA5C + 12) = v9;
        goto LABEL_21;
      }
      v8 = MEMORY[0x4A01AA](0x10u);
      if ( v8 )
      {
        *v8 = &off_4AF46C;
        v8[1] = 0;
        v8[3] = 0;
        v8[2] = 0;
      }
      else
      {
        v8 = 0;
      }
      dword_59AA58 = (int)v8;
      if ( v8 )
      {
        v8[1] = v6;
        v9 = (_DWORD *)dword_59AA58;
LABEL_21:
        dword_59AA5C = (int)v9;
        ++dword_59AA68;
        break;
      }
      break;
    default:
      break;
  }
LABEL_53:
  v15 = *(_DWORD *)this;
  this[400444] = 1;
  return (*(int (__thiscall **)(char *, int, int, _DWORD))(v15 + 16))(this, 8, a2, 0);
}
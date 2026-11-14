__int16 __thiscall sub_407FC0(_DWORD *this, int a2)
{
  _DWORD *v3; // eax
  char *v4; // eax
  char *v5; // ebx
  char *v6; // eax

  sub_4832D0(110, this[33], this[34], 0);
  LOWORD(v3) = *(_WORD *)(a2 + 4);
  if ( (_WORD)v3 == 800 )
  {
    v4 = (char *)MEMORY[0x4A01AA](0x240u);
    if ( v4 )
      v5 = sub_449480(v4);
    else
      v5 = 0;
    v3 = (_DWORD *)(*(int (__thiscall **)(char *, int, _DWORD, _DWORD, _DWORD, _DWORD, _DWORD, _DWORD, _DWORD))(*(_DWORD *)v5 + 4))(
                     v5,
                     322,
                     this[33],
                     this[34],
                     this[33],
                     this[34],
                     0,
                     0,
                     0);
    if ( v3 )
    {
      if ( !dword_59A9F8 )
      {
        v3 = MEMORY[0x4A01AA](0x10u);
        if ( v3 )
        {
          *v3 = &off_4AF460;
          v3[1] = 0;
          v3[3] = 0;
          v3[2] = 0;
          goto LABEL_10;
        }
        goto LABEL_9;
      }
      goto LABEL_12;
    }
    if ( !v5 )
      return (__int16)v3;
LABEL_26:
    LOWORD(v3) = (**(int (__thiscall ***)(char *, int))v5)(v5, 1);
    return (__int16)v3;
  }
  if ( (_WORD)v3 != 806 )
    return (__int16)v3;
  v6 = (char *)MEMORY[0x4A01AA](0x240u);
  if ( v6 )
    v5 = sub_449480(v6);
  else
    v5 = 0;
  v3 = (_DWORD *)(*(int (__thiscall **)(char *, int, _DWORD, _DWORD, _DWORD, _DWORD, _DWORD, _DWORD, _DWORD))(*(_DWORD *)v5 + 4))(
                   v5,
                   320,
                   this[33],
                   this[34],
                   this[33],
                   this[34],
                   0,
                   0,
                   0);
  if ( !v3 )
  {
    if ( !v5 )
      return (__int16)v3;
    goto LABEL_26;
  }
  if ( !dword_59A9F8 )
  {
    v3 = MEMORY[0x4A01AA](0x10u);
    if ( v3 )
    {
      *v3 = &off_4AF460;
      v3[1] = 0;
      v3[3] = 0;
      v3[2] = 0;
LABEL_10:
      dword_59A9F8 = (int)v3;
      if ( v3 )
      {
        v3[1] = v5;
        v3 = (_DWORD *)dword_59A9F8;
LABEL_14:
        dword_59A9FC = (int)v3;
        LOWORD(v3) = ++dword_59AA08;
        return (__int16)v3;
      }
      return (__int16)v3;
    }
LABEL_9:
    v3 = 0;
    goto LABEL_10;
  }
LABEL_12:
  v3 = MEMORY[0x4A01AA](0x10u);
  if ( v3 )
  {
    *v3 = &off_4AF460;
    v3[1] = 0;
    v3[3] = 0;
    v3[2] = 0;
    v3[1] = v5;
    v3[3] = 0;
    v3[2] = dword_59A9FC;
    *(_DWORD *)(dword_59A9FC + 12) = v3;
    goto LABEL_14;
  }
  return (__int16)v3;
}
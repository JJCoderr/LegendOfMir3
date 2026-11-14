_DWORD *__thiscall sub_408C10(int this, unsigned __int16 a2)
{
  _DWORD *result; // eax
  int v4; // ecx
  char *v5; // eax
  char *v6; // ebx
  char *v7; // eax
  char *v8; // eax
  char *v9; // eax
  char *v10; // eax
  char *v11; // eax
  char *v12; // eax
  char *v13; // eax
  char *v14; // eax
  char *v15; // eax
  char *v16; // eax
  char *v17; // eax
  char *v18; // eax

  result = (_DWORD *)(a2 - 4);
  v4 = 0;
  switch ( a2 )
  {
    case 4u:
      v5 = (char *)MEMORY[0x4A01AA](0x240u);
      if ( v5 )
        v6 = sub_449480(v5);
      else
        v6 = 0;
      result = (_DWORD *)(*(int (__thiscall **)(char *, int, _DWORD, _DWORD, _DWORD, _DWORD, int, _DWORD, _DWORD))(*(_DWORD *)v6 + 4))(
                           v6,
                           531,
                           *(_DWORD *)(this + 132),
                           *(_DWORD *)(this + 136),
                           *(_DWORD *)(this + 132),
                           *(_DWORD *)(this + 136),
                           this,
                           0,
                           0);
      if ( !result )
        goto LABEL_75;
      if ( dword_59A9F8 )
        goto LABEL_72;
      result = MEMORY[0x4A01AA](0x10u);
      if ( !result )
        goto LABEL_9;
      *result = &off_4AF460;
      result[1] = 0;
      result[3] = 0;
      result[2] = 0;
      goto LABEL_10;
    case 5u:
    case 6u:
    case 0xBu:
    case 0x13u:
      v7 = (char *)MEMORY[0x4A01AA](0x240u);
      if ( v7 )
        v6 = sub_449480(v7);
      else
        v6 = 0;
      result = (_DWORD *)(*(int (__thiscall **)(char *, int, _DWORD, _DWORD, _DWORD, _DWORD, int, _DWORD, _DWORD))(*(_DWORD *)v6 + 4))(
                           v6,
                           526,
                           *(_DWORD *)(this + 132),
                           *(_DWORD *)(this + 136),
                           *(_DWORD *)(this + 132),
                           *(_DWORD *)(this + 136),
                           this,
                           0,
                           0);
      goto LABEL_16;
    case 0x20u:
    case 0x21u:
    case 0x22u:
    case 0x23u:
    case 0x24u:
      v9 = (char *)MEMORY[0x4A01AA](0x240u);
      if ( v9 )
        v6 = sub_449480(v9);
      else
        v6 = 0;
      result = (_DWORD *)(*(int (__thiscall **)(char *, int, _DWORD, _DWORD, _DWORD, _DWORD, int, _DWORD, _DWORD))(*(_DWORD *)v6 + 4))(
                           v6,
                           534,
                           *(_DWORD *)(this + 132),
                           *(_DWORD *)(this + 136),
                           *(_DWORD *)(this + 132),
                           *(_DWORD *)(this + 136),
                           this,
                           0,
                           0);
      goto LABEL_16;
    case 0x28u:
      v8 = (char *)MEMORY[0x4A01AA](0x240u);
      if ( v8 )
        v6 = sub_449480(v8);
      else
        v6 = 0;
      result = (_DWORD *)(*(int (__thiscall **)(char *, int, _DWORD, _DWORD, _DWORD, _DWORD, int, _DWORD, _DWORD))(*(_DWORD *)v6 + 4))(
                           v6,
                           527,
                           *(_DWORD *)(this + 132),
                           *(_DWORD *)(this + 136),
                           *(_DWORD *)(this + 132),
                           *(_DWORD *)(this + 136),
                           this,
                           0,
                           0);
      if ( !result )
        goto LABEL_75;
      if ( dword_59A9F8 )
        goto LABEL_72;
      result = MEMORY[0x4A01AA](0x10u);
      if ( !result )
        goto LABEL_9;
      *result = &off_4AF460;
      result[1] = 0;
      result[3] = 0;
      result[2] = 0;
      goto LABEL_10;
    case 0x30u:
      v10 = (char *)MEMORY[0x4A01AA](0x240u);
      if ( v10 )
        v6 = sub_449480(v10);
      else
        v6 = 0;
      result = (_DWORD *)(*(int (__thiscall **)(char *, int, _DWORD, _DWORD, _DWORD, _DWORD, int, _DWORD, _DWORD))(*(_DWORD *)v6 + 4))(
                           v6,
                           528,
                           *(_DWORD *)(this + 132),
                           *(_DWORD *)(this + 136),
                           *(_DWORD *)(this + 132),
                           *(_DWORD *)(this + 136),
                           this,
                           0,
                           0);
      goto LABEL_16;
    case 0x63u:
      v11 = (char *)MEMORY[0x4A01AA](0x240u);
      if ( v11 )
        v6 = sub_449480(v11);
      else
        v6 = 0;
      result = (_DWORD *)(*(int (__thiscall **)(char *, int, _DWORD, _DWORD, _DWORD, _DWORD, int, _DWORD, _DWORD))(*(_DWORD *)v6 + 4))(
                           v6,
                           525,
                           *(_DWORD *)(this + 132),
                           *(_DWORD *)(this + 136),
                           *(_DWORD *)(this + 132),
                           *(_DWORD *)(this + 136),
                           this,
                           0,
                           0);
      goto LABEL_16;
    case 0x65u:
      v15 = (char *)MEMORY[0x4A01AA](0x240u);
      if ( v15 )
        v6 = sub_449480(v15);
      else
        v6 = 0;
      result = (_DWORD *)(*(int (__thiscall **)(char *, int, _DWORD, _DWORD, _DWORD, _DWORD, int, _DWORD, _DWORD))(*(_DWORD *)v6 + 4))(
                           v6,
                           541,
                           *(_DWORD *)(this + 132),
                           *(_DWORD *)(this + 136),
                           *(_DWORD *)(this + 132),
                           *(_DWORD *)(this + 136),
                           this,
                           0,
                           0);
      goto LABEL_16;
    case 0x69u:
      v14 = (char *)MEMORY[0x4A01AA](0x240u);
      if ( v14 )
        v6 = sub_449480(v14);
      else
        v6 = 0;
      result = (_DWORD *)(*(int (__thiscall **)(char *, int, _DWORD, _DWORD, _DWORD, _DWORD, int, _DWORD, _DWORD))(*(_DWORD *)v6 + 4))(
                           v6,
                           540,
                           *(_DWORD *)(this + 132),
                           *(_DWORD *)(this + 136),
                           *(_DWORD *)(this + 132),
                           *(_DWORD *)(this + 136),
                           this,
                           0,
                           0);
      goto LABEL_16;
    case 0x78u:
      v12 = (char *)MEMORY[0x4A01AA](0x240u);
      if ( v12 )
        v6 = sub_449480(v12);
      else
        v6 = 0;
      result = (_DWORD *)(*(int (__thiscall **)(char *, int, _DWORD, _DWORD, _DWORD, _DWORD, int, _DWORD, _DWORD))(*(_DWORD *)v6 + 4))(
                           v6,
                           533,
                           *(_DWORD *)(this + 132),
                           *(_DWORD *)(this + 136),
                           *(_DWORD *)(this + 132),
                           *(_DWORD *)(this + 136),
                           this,
                           0,
                           0);
      goto LABEL_16;
    case 0x82u:
      result = (_DWORD *)sub_40DB10(this, (int)dword_59AA6C, 547, 0, -1);
      goto LABEL_63;
    case 0x83u:
      result = (_DWORD *)sub_40DB10(this, (int)dword_59AA6C, 549, 0, -1);
      goto LABEL_63;
    case 0x8Au:
      result = (_DWORD *)sub_40DB10(this, (int)dword_59AA6C, 553, 0, -1);
      goto LABEL_63;
    case 0x8Cu:
      LOBYTE(v4) = *(_BYTE *)(this + 121);
      result = (_DWORD *)sub_40DB10(this, (int)dword_59AA6C, 557, v4, -1);
      goto LABEL_63;
    case 0x8Eu:
    case 0x8Fu:
    case 0x90u:
      result = (_DWORD *)sub_40DB10(this, (int)dword_59AA6C, 556, 0, -1);
      goto LABEL_63;
    case 0x9Fu:
      result = (_DWORD *)sub_40DB10(this, (int)dword_59AA6C, 582, 0, -1);
      goto LABEL_63;
    case 0xA0u:
      result = (_DWORD *)sub_40DB10(this, (int)dword_59AA6C, 583, 0, -1);
      goto LABEL_63;
    case 0xA1u:
      result = (_DWORD *)sub_40DB10(this, (int)dword_59AA6C, 586, 0, -1);
LABEL_63:
      *(_DWORD *)(this + 400240) = 1;
      return result;
    case 0xA4u:
      v16 = (char *)MEMORY[0x4A01AA](0x240u);
      if ( v16 )
        v6 = sub_449480(v16);
      else
        v6 = 0;
      result = (_DWORD *)(*(int (__thiscall **)(char *, int, _DWORD, _DWORD, _DWORD, _DWORD, int, _DWORD, _DWORD))(*(_DWORD *)v6 + 4))(
                           v6,
                           589,
                           *(_DWORD *)(this + 132),
                           *(_DWORD *)(this + 136),
                           *(_DWORD *)(this + 132),
                           *(_DWORD *)(this + 136),
                           this,
                           0,
                           0);
      goto LABEL_16;
    case 0xB6u:
    case 0xB9u:
      v18 = (char *)MEMORY[0x4A01AA](0x240u);
      if ( v18 )
        v6 = sub_449480(v18);
      else
        v6 = 0;
      result = (_DWORD *)(*(int (__thiscall **)(char *, int, _DWORD, _DWORD, _DWORD, _DWORD, int, _DWORD, _DWORD))(*(_DWORD *)v6 + 4))(
                           v6,
                           605,
                           *(_DWORD *)(this + 132),
                           *(_DWORD *)(this + 136),
                           *(_DWORD *)(this + 132),
                           *(_DWORD *)(this + 136),
                           this,
                           0,
                           0);
      if ( result )
      {
        if ( dword_59A9F8 )
          goto LABEL_72;
        result = MEMORY[0x4A01AA](0x10u);
        if ( result )
        {
          *result = &off_4AF460;
          result[1] = 0;
          result[3] = 0;
          result[2] = 0;
          goto LABEL_10;
        }
        goto LABEL_9;
      }
      if ( !v6 )
        return result;
      return (_DWORD *)(**(int (__thiscall ***)(char *, int))v6)(v6, 1);
    case 0xB8u:
      v17 = (char *)MEMORY[0x4A01AA](0x240u);
      if ( v17 )
        v6 = sub_449480(v17);
      else
        v6 = 0;
      result = (_DWORD *)(*(int (__thiscall **)(char *, int, _DWORD, _DWORD, _DWORD, _DWORD, int, _DWORD, int))(*(_DWORD *)v6 + 4))(
                           v6,
                           602,
                           *(_DWORD *)(this + 132),
                           *(_DWORD *)(this + 136),
                           *(_DWORD *)(this + 132),
                           *(_DWORD *)(this + 136),
                           this,
                           0,
                           800);
      goto LABEL_16;
    case 0xBAu:
      v13 = (char *)MEMORY[0x4A01AA](0x240u);
      if ( v13 )
        v6 = sub_449480(v13);
      else
        v6 = 0;
      result = (_DWORD *)(*(int (__thiscall **)(char *, int, _DWORD, _DWORD, _DWORD, _DWORD, int, _DWORD, int))(*(_DWORD *)v6 + 4))(
                           v6,
                           610,
                           *(_DWORD *)(this + 132),
                           *(_DWORD *)(this + 136),
                           *(_DWORD *)(this + 132),
                           *(_DWORD *)(this + 136),
                           this,
                           0,
                           800);
LABEL_16:
      if ( result )
      {
        if ( dword_59A9F8 )
        {
LABEL_72:
          result = MEMORY[0x4A01AA](0x10u);
          if ( !result )
            return result;
          *result = &off_4AF460;
          result[1] = 0;
          result[3] = 0;
          result[2] = 0;
          result[1] = v6;
          result[3] = 0;
          result[2] = dword_59A9FC;
          *(_DWORD *)(dword_59A9FC + 12) = result;
          goto LABEL_74;
        }
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
LABEL_9:
          result = 0;
        }
LABEL_10:
        dword_59A9F8 = (int)result;
        if ( result )
        {
          result[1] = v6;
          result = (_DWORD *)dword_59A9F8;
LABEL_74:
          dword_59A9FC = (int)result;
          return (_DWORD *)++dword_59AA08;
        }
      }
      else
      {
LABEL_75:
        if ( v6 )
          return (_DWORD *)(**(int (__thiscall ***)(char *, int))v6)(v6, 1);
      }
      return result;
    default:
      return result;
  }
}
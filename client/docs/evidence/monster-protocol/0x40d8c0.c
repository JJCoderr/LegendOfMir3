int __thiscall sub_40D8C0(_BYTE *this)
{
  int v2; // eax
  _BYTE *v3; // ecx
  unsigned __int16 *v4; // ebp
  int v5; // eax
  FILE *v6; // eax
  int v7; // eax
  int v8; // edx

  if ( this[120] && !*((_DWORD *)this + 100108) )
    return 0;
  v2 = *((_DWORD *)this + 41);
  v3 = this + 164;
  if ( v2 <= 0 )
    return 0;
  *((_DWORD *)this + 100050) = (unsigned int)v2 >= 3;
  v4 = (unsigned __int16 *)sub_481AF0(v3);
  if ( !v4 )
    return 0;
  v5 = v4[2];
  if ( (unsigned __int16)v5 <= 0x1Bu )
  {
    if ( (unsigned __int16)v5 == 27 )
    {
      (*(void (__thiscall **)(_BYTE *, unsigned __int16 *))(*(_DWORD *)this + 88))(this, v4);
    }
    else
    {
      switch ( v4[2] )
      {
        case 9u:
          (*(void (__thiscall **)(_BYTE *, unsigned __int16 *))(*(_DWORD *)this + 92))(this, v4);
          break;
        case 0xAu:
          goto LABEL_38;
        case 0xBu:
          (*(void (__thiscall **)(_BYTE *, unsigned __int16 *))(*(_DWORD *)this + 68))(this, v4);
          break;
        case 0xEu:
          (*(void (__thiscall **)(_BYTE *, unsigned __int16 *))(*(_DWORD *)this + 100))(this, v4);
          break;
        case 0x14u:
          (*(void (__thiscall **)(_BYTE *, unsigned __int16 *))(*(_DWORD *)this + 76))(this, v4);
          break;
        case 0x15u:
          (*(void (__thiscall **)(_BYTE *, unsigned __int16 *))(*(_DWORD *)this + 80))(this, v4);
          break;
        case 0x16u:
          (*(void (__thiscall **)(_BYTE *, unsigned __int16 *))(*(_DWORD *)this + 104))(this, v4);
          break;
        case 0x17u:
          (*(void (__thiscall **)(_BYTE *, unsigned __int16 *))(*(_DWORD *)this + 108))(this, v4);
          break;
        default:
          goto LABEL_41;
      }
    }
    goto LABEL_41;
  }
  if ( v4[2] <= 0x326u )
  {
    if ( v5 == 806 )
    {
LABEL_30:
      (*(void (__thiscall **)(_BYTE *, unsigned __int16 *))(*(_DWORD *)this + 120))(this, v4);
      goto LABEL_41;
    }
    if ( v4[2] <= 0x320u )
    {
      if ( v5 != 800 )
      {
        if ( v5 == 31 )
        {
          (*(void (__thiscall **)(_BYTE *, unsigned __int16 *))(*(_DWORD *)this + 96))(this, v4);
        }
        else if ( v4[2] > 0x1Fu && v4[2] <= 0x22u )
        {
          (*(void (__thiscall **)(_BYTE *, unsigned __int16 *))(*(_DWORD *)this + 84))(this, v4);
        }
        goto LABEL_41;
      }
      goto LABEL_30;
    }
    if ( v5 != 801 )
      goto LABEL_41;
LABEL_37:
    (*(void (__thiscall **)(_BYTE *, unsigned __int16 *))(*(_DWORD *)this + 116))(this, v4);
    goto LABEL_41;
  }
  if ( v4[2] > 0x524u )
  {
    if ( v5 == 1317 )
    {
      LOBYTE(v5) = this[121];
      (*(void (__thiscall **)(_BYTE *, int, int, _DWORD))(*(_DWORD *)this + 16))(this, 6, v5, 0);
    }
    goto LABEL_41;
  }
  if ( v5 == 1316 )
  {
LABEL_38:
    (*(void (__thiscall **)(_BYTE *, unsigned __int16 *))(*(_DWORD *)this + 72))(this, v4);
    goto LABEL_41;
  }
  if ( v4[2] >= 0x327u )
  {
    if ( v4[2] > 0x328u )
    {
      if ( v5 == 1309 )
      {
        (*(void (__thiscall **)(_BYTE *, unsigned __int16 *))(*(_DWORD *)this + 96))(this, v4);
        v6 = (FILE *)MEMORY[0x4A01AA](0x40Cu);
        qmemcpy(v6, v4, 0x40Cu);
        LOWORD(v6->_cnt) = 31;
        MEMORY[0x481A30](v6);
      }
      goto LABEL_41;
    }
    goto LABEL_37;
  }
LABEL_41:
  MEMORY[0x4939F6](v4);
  v7 = *((_DWORD *)this + 100108);
  if ( v7 )
  {
    v8 = *(_DWORD *)this;
    LOBYTE(v7) = this[121];
    if ( this[120] == 10 )
    {
      (*(void (__thiscall **)(_BYTE *, int, int, _DWORD))(v8 + 16))(this, 10, v7, 0);
      return 1;
    }
    (*(void (__thiscall **)(_BYTE *, int, int, _DWORD))(v8 + 16))(this, 4, v7, 0);
    *((_DWORD *)this + 31) = *((_DWORD *)this + 29) - 1;
  }
  return 1;
}
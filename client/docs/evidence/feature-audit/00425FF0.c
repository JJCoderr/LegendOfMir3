int __thiscall sub_425F80(_DWORD *this, int a2, int a3)
{
  int v3; // edi
  int v5; // edi
  int v6; // ebp
  int v7; // ebx
  int v8; // eax
  int v9; // eax
  char v10; // cl
  const char *v11; // eax
  int result; // eax
  const char *v13; // eax
  int v14; // [esp+1Ch] [ebp-Ch]
  int v15; // [esp+20h] [ebp-8h]
  int v16; // [esp+24h] [ebp-4h]

  v3 = a2;
  if ( a2 == 13 && this[15] )
  {
    this[895925] = 0;
    dword_8FCD98 = 0;
    if ( (byte_8FCC78 & 1) != 0 )
    {
      if ( (int)sub_48B48C(800, 600, 16, 2, 5) < 0 )
        goto LABEL_7;
    }
    else if ( (int)sub_48B48C(800, 600, 16, 1, 5) < 0 )
    {
LABEL_7:
      dword_4AF284(dword_8FCC58, MEMORY[0x4B56AC], MEMORY[0x4B56E4], 16);
    }
    dword_4AF2B0(dword_8FCC58);
    sub_45D960(dword_8FCC64);
    sub_47F3E0(this + 230525);
    dword_8FCD98 = 1;
    this[895925] = 1;
  }
  if ( this[794820] || this[902656] )
    return 0;
  if ( a2 == 121 )
  {
    if ( !sub_43E570(this + 695155, 121, a3) )
    {
      v5 = 0;
      v6 = 0;
      v7 = 0;
      LOBYTE(v16) = 0;
      if ( this[905634] && sub_430050(121) )
      {
        v8 = this[905634];
        v6 = *(_DWORD *)(v8 + 132);
        v7 = *(_DWORD *)(v8 + 136);
        v5 = **(_DWORD **)(v8 + 400440);
        v16 = *(_DWORD *)(v8 + 69);
      }
      v9 = this[905629];
      if ( v9 )
      {
        if ( this[23] )
        {
          v10 = *(_BYTE *)(v9 + 69);
          if ( v10 == 3 || (!*(_DWORD *)(v9 + 400432) && !v10 || v10 == 1) && *(_DWORD *)(v9 + 400428) == 27631 )
            this[905634] = v9;
        }
        v6 = *(_DWORD *)(v9 + 132);
        v7 = *(_DWORD *)(v9 + 136);
        v5 = **(_DWORD **)(v9 + 400440);
        v16 = *(_DWORD *)(v9 + 69);
      }
      sub_4199A0((int)(this + 794986), v7, v6, v5, 121, a3, this[895921], this[895922], v6, v7, v5, v16);
      v3 = a2;
      goto LABEL_26;
    }
    return 0;
  }
LABEL_26:
  switch ( v3 )
  {
    case 'Q':
    case 'q':
      if ( sub_421F60(dword_4B9878) )
      {
        v13 = (const char *)sub_480550(2636);
        sub_420540((int)dword_82B050, (int)dword_5A041C, 109, 1, v13, 0, -1, -1, 0xFFFF, 0, 0, v14, v15);
      }
      return 0;
    case 'X':
    case 'x':
      if ( !sub_421F60(dword_4B9878) )
        return 0;
      v11 = (const char *)sub_480550(1065);
      sub_420540((int)(this + 902646), (int)(this + 236265), 101, 1, v11, 0, -1, -1, 0xFFFF, 0, 0, v14, v15);
      result = 0;
      break;
    default:
      return 0;
  }
  return result;
}
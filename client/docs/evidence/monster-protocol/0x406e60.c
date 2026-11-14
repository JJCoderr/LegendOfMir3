int __thiscall sub_406E60(int this)
{
  unsigned __int16 v2; // si
  int v3; // ebp
  int v4; // eax
  int v5; // ecx
  int v6; // eax
  int v7; // eax
  int result; // eax
  int v9; // eax

  if ( *(_BYTE *)(this + 69) == 3 )
    v2 = *(_WORD *)(this + 71);
  else
    v2 = *(unsigned __int8 *)(this + 71);
  v3 = *(_DWORD *)(this + 112);
  v4 = *(_DWORD *)(this + 124);
  v5 = v3 + 1;
  if ( v4 != v3 + 1 )
    goto LABEL_20;
  if ( *(_BYTE *)(this + 120) == 8 )
  {
    v6 = 10 * (v2 + 20);
    goto LABEL_40;
  }
  if ( v4 != v5 )
    goto LABEL_20;
  if ( *(_BYTE *)(this + 120) <= 1u )
  {
    v7 = MEMORY[0x49FDAE]();
    if ( !*(_DWORD *)(this + 400456) || *(_DWORD *)(this + 400432) )
    {
      if ( v7 % 3 || *(_DWORD *)(this + 400432) )
        goto LABEL_42;
      *(_DWORD *)(this + 400456) = 1;
      v6 = 10 * v2 + 201;
    }
    else
    {
      if ( v7 % 25 )
        goto LABEL_42;
      v6 = 10 * v2 + 201;
    }
    goto LABEL_40;
  }
  if ( v4 == v5 && *(_BYTE *)(this + 120) == 2 )
  {
    v6 = 10 * v2 + 202;
  }
  else
  {
LABEL_20:
    if ( v4 == v3 + 2 && *(_BYTE *)(this + 120) == 2 )
    {
      v6 = 10 * v2 + 203;
    }
    else if ( v4 == v5 && *(_BYTE *)(this + 120) == 3 )
    {
      v6 = 10 * v2 + 204;
    }
    else if ( v4 == v3 + 2 && *(_BYTE *)(this + 120) == 4 )
    {
      v6 = 10 * v2 + 205;
    }
    else if ( v4 == v3 + 3 && *(_BYTE *)(this + 120) == 4 )
    {
      v6 = 10 * v2 + 206;
    }
    else
    {
      if ( v4 != v5 )
        goto LABEL_42;
      if ( *(_BYTE *)(this + 120) == 5 )
      {
        v6 = 10 * v2 + 207;
      }
      else
      {
        if ( v4 != v5 )
          goto LABEL_42;
        if ( *(_BYTE *)(this + 120) == 6 )
        {
          v6 = 10 * v2 + 208;
        }
        else
        {
          if ( v4 != v5 || *(_BYTE *)(this + 120) != 7 )
            goto LABEL_42;
          v6 = 10 * v2 + 209;
        }
      }
    }
  }
LABEL_40:
  if ( v6 != -1 )
    sub_4832D0(v6, *(_DWORD *)(this + 132), *(_DWORD *)(this + 136), 0);
LABEL_42:
  result = *(_DWORD *)(this + 124);
  if ( result == *(_DWORD *)(this + 112) + 1 && *(_BYTE *)(this + 120) == 3 && *(_BYTE *)(this + 400436) < 2u )
  {
    switch ( *(_BYTE *)(this + 400437) )
    {
      case 1:
      case 4:
      case 0xB:
      case 0xD:
      case 0xE:
      case 0x14:
      case 0x19:
      case 0x1D:
      case 0x1F:
      case 0x21:
      case 0x26:
      case 0x28:
      case 0x2D:
      case 0x2E:
        v9 = 63;
        break;
      case 2:
      case 3:
      case 6:
      case 7:
      case 0x10:
      case 0x11:
      case 0x13:
      case 0x18:
        v9 = 72;
        break;
      case 5:
      case 0xA:
      case 0xC:
        v9 = 64;
        break;
      case 8:
      case 9:
        v9 = 61;
        break;
      case 0xF:
        v9 = 65;
        break;
      case 0x12:
      case 0x16:
      case 0x17:
      case 0x1A:
      case 0x1B:
      case 0x1C:
      case 0x1E:
      case 0x20:
        v9 = 62;
        break;
      case 0x15:
        v9 = 60;
        break;
      default:
        v9 = 73;
        break;
    }
    return sub_4832D0(v9, *(_DWORD *)(this + 132), *(_DWORD *)(this + 136), 0);
  }
  return result;
}
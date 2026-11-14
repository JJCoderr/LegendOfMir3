int __thiscall sub_43A840(_DWORD *this, int a2, int a3)
{
  int v4; // eax
  int v5; // ecx
  int v6; // ebx
  int v7; // edi
  _DWORD *v8; // edx
  int v9; // eax
  int v10; // eax
  _DWORD v12[4]; // [esp+10h] [ebp-10h] BYREF

  if ( !this[924] )
    return -1;
  v4 = this[922];
  v5 = this[924] - 1;
  this[921] = v4;
  this[923] = v5;
  v6 = this[924] - 1;
  if ( v6 < 0 )
    return -1;
  while ( 1 )
  {
    v7 = this[921];
    switch ( *(_DWORD *)v7 )
    {
      case 0:
        v8 = this + 7366;
        goto LABEL_23;
      case 1:
        v8 = this + 43737;
        goto LABEL_23;
      case 2:
        v8 = this + 54199;
        goto LABEL_23;
      case 3:
        v8 = this + 54708;
        goto LABEL_23;
      case 4:
        v8 = this + 74541;
        goto LABEL_23;
      case 6:
        v8 = this + 76060;
        goto LABEL_23;
      case 7:
        v8 = this + 76313;
        goto LABEL_23;
      case 8:
        v8 = this + 86120;
        goto LABEL_23;
      case 9:
        v8 = this + 86721;
        goto LABEL_23;
      case 0xB:
        v8 = this + 87088;
        goto LABEL_23;
      case 0xC:
        v8 = this + 87251;
        goto LABEL_23;
      case 0xD:
        v8 = this + 88237;
        goto LABEL_23;
      case 0xE:
        v8 = this + 88483;
        goto LABEL_23;
      case 0x10:
        v8 = this + 89545;
        goto LABEL_23;
      case 0x12:
        v8 = this + 94304;
        goto LABEL_23;
      case 0x13:
        v8 = this + 94528;
        goto LABEL_23;
      case 0x14:
        v8 = this + 95818;
        goto LABEL_23;
      case 0x15:
        v8 = this + 96768;
        goto LABEL_23;
      case 0x16:
        v8 = this + 99361;
LABEL_23:
        v12[0] = *v8;
        v12[1] = v8[1];
        v12[2] = v8[2];
        v12[3] = v8[3];
        break;
      default:
        break;
    }
    if ( dword_4AF2AC(v12, a2, a3) )
      return *(_DWORD *)v7;
    v9 = this[923];
    this[921] = *(_DWORD *)(this[921] + 8);
    if ( v9 <= 1 )
      v10 = 1;
    else
      v10 = v9 - 1;
    --v6;
    this[923] = v10;
    if ( v6 < 0 )
      return -1;
  }
}
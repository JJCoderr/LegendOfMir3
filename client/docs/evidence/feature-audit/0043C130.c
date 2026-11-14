int __userpurge sub_43C130@<eax>(_DWORD *this@<ecx>, int a2@<ebx>, int a3, int a4)
{
  int result; // eax
  int v6; // edi
  int v7; // eax
  int v8; // esi
  int v9; // eax
  int v10; // eax
  const char *v11; // eax
  int v12; // esi
  const char *v13; // eax
  int v14; // esi
  int v15; // eax
  unsigned int v16; // kr08_4
  unsigned int v17; // kr0C_4
  int v18; // esi
  int v19; // edi
  const char *v20; // eax
  int v21; // eax
  int v22; // eax
  const char *v23; // eax
  int v24; // eax
  int v25; // esi
  int v26; // edi
  const char *v27; // eax
  int v28; // eax
  int v29; // eax
  const char *v30; // eax
  int v31; // eax
  int v32; // eax
  int v33; // eax
  int v34; // eax
  int v35; // eax
  int (__stdcall *v36)(_DWORD *, int, int); // edi
  int v37; // eax
  int v38; // [esp+Ch] [ebp-220h]
  int v39; // [esp+10h] [ebp-21Ch]
  int v40; // [esp+14h] [ebp-218h]
  int v41; // [esp+18h] [ebp-214h]
  int v42; // [esp+1Ch] [ebp-210h]
  int v43; // [esp+20h] [ebp-20Ch]
  char v44[260]; // [esp+24h] [ebp-208h] BYREF
  _BYTE v45[260]; // [esp+128h] [ebp-104h] BYREF

  this[925] = 0;
  if ( this[99665] )
  {
    if ( sub_420F60(a3, a4) )
      dword_4AF2B4(dword_8FCC58, 2, 0, 0);
    return 1;
  }
  if ( this[89434] )
  {
    if ( sub_45F100((int)(this + 89422), a2, a3, a4) )
      sub_43AB90(this, 15);
    return 1;
  }
  v6 = a3;
  if ( sub_420360(this + 6789, a3, a4) )
    return 1;
  if ( sub_420360(this + 6808, a3, a4) )
    return 1;
  v7 = sub_43A840(this, a3, a4);
  v8 = v7;
  if ( v7 != 2 )
  {
    if ( v7 == -1 )
      goto LABEL_15;
LABEL_21:
    switch ( v8 )
    {
      case 0:
        if ( !sub_441310((int)(this + 7360), this + 8, a3, a4) || !sub_46B6E0(this) )
          goto LABEL_62;
        sub_43AB90(this, 0);
        nullsub_5();
        result = 1;
        break;
      case 1:
        if ( !sub_476740(this + 8, a3, a4) || !sub_46B6E0(this) )
          goto LABEL_62;
        sub_43AB90(this, 1);
        nullsub_5();
        result = 1;
        break;
      case 2:
        if ( !sub_478850(a3, a4) )
          goto LABEL_62;
        sub_43AB90(this, 2);
        result = 1;
        break;
      case 3:
        if ( !sub_41DD00(this + 8, a3, a4) )
          goto LABEL_62;
        sub_43AB90(this, 3);
        result = 1;
        break;
      case 4:
        if ( !sub_433420((int)(this + 74535), a3, a4) )
          goto LABEL_62;
        sub_43AB90(this, 4);
        result = 1;
        break;
      case 6:
        if ( !sub_431F20(this + 76054, a3, a4) )
          goto LABEL_62;
        sub_43AB90(this, 6);
        result = 1;
        break;
      case 7:
        if ( !sub_47B150(a3, a4) )
          goto LABEL_62;
        sub_43AB90(this, 7);
        result = 1;
        break;
      case 8:
        if ( !sub_41B8C0(a3, a4) )
          goto LABEL_62;
        sub_43AB90(this, 8);
        result = 1;
        break;
      case 9:
        if ( !sub_4610D0((int)(this + 86715), (int)this, a3, a4) )
          goto LABEL_62;
        sub_424AD0(dword_4B9878);
        result = 1;
        break;
      case 11:
        if ( !sub_46C960(a3, a4) )
          goto LABEL_62;
        sub_43AB90(this, 11);
        result = 1;
        break;
      case 12:
        if ( !sub_463500(this + 87245, a3, a4) )
          goto LABEL_62;
        sub_43AB90(this, 12);
        result = 1;
        break;
      case 13:
        if ( !sub_434910(a3, a4) )
          goto LABEL_62;
        sub_43AB90(this, 13);
        result = 1;
        break;
      case 14:
        if ( !sub_450860((int)(this + 88477), a3, a4) )
          goto LABEL_62;
        sub_43AB90(this, 14);
        result = 1;
        break;
      case 16:
        if ( !sub_41A0A0(this + 89539, (int)(this + 8), a3, a4) )
          goto LABEL_62;
        sub_43AB90(this, 16);
        result = 1;
        break;
      case 18:
        if ( !sub_46EA50(this + 8, a3, a4) )
          goto LABEL_62;
        sub_43AB90(this, 18);
        result = 1;
        break;
      case 19:
        if ( !sub_455360(this + 8, a3, a4) )
          goto LABEL_62;
        sub_43AB90(this, 19);
        result = 1;
        break;
      case 20:
        if ( !sub_456EC0(this + 8, a3, a4) )
          goto LABEL_62;
        sub_43AB90(this, 20);
        result = 1;
        break;
      case 21:
        if ( !sub_45A110(this + 8, a3, a4) )
          goto LABEL_62;
        sub_43AB90(this, 21);
        result = 1;
        break;
      case 22:
        if ( sub_41ECC0(this + 99355, a3, a4) )
          sub_43AB90(this, 22);
        goto LABEL_62;
      default:
LABEL_62:
        result = 1;
        break;
    }
    return result;
  }
  if ( sub_4787B0((int)(this + 54193), a3, a4) )
    goto LABEL_21;
LABEL_15:
  v42 = 0;
  v9 = -(this[7317] != 1);
  LOBYTE(v9) = v9 & 0xF9;
  v43 = v9 + 23;
  if ( v9 + 23 > 0 )
  {
    do
    {
      if ( (*(int (__thiscall **)(_DWORD *, int, int, int))(this[45 * v42 + 5619] + 16))(
             &this[45 * v42 + 5619],
             v6,
             a4,
             1) )
      {
        v10 = v42;
        switch ( v42 )
        {
          case 0:
            LOBYTE(v10) = byte_7C1E99;
            v12 = sub_4281B0(dword_7C1EA4, dword_7C1EA8, v10, 1);
            if ( v12 )
            {
              sub_41FD70(1392, 1396, 1392);
              sub_47C2E0(v12 + 403145);
            }
            break;
          case 1:
            if ( (unsigned int)(dword_4AF2FC(v38, v39, v40, v41, v42) - this[6830]) > 0x64 )
            {
              if ( this[7315] )
              {
                this[7315] = 0;
              }
              else
              {
                sub_47BF30(dword_8FCDD8);
                this[7341] = 1;
              }
            }
            break;
          case 2:
            this[6827] = this[6827] == 0;
            break;
          case 3:
            if ( sub_421F60(dword_4B9878) )
            {
              v13 = (const char *)sub_480550(2636);
              sub_420540((int)dword_82B050, (int)dword_5A041C, 109, 1, v13, 0, -1, -1, 0xFFFF, 0, 0, v38, v39);
            }
            break;
          case 4:
            if ( sub_421F60(dword_4B9878) )
            {
              v11 = (const char *)sub_480550(1065);
              sub_420540((int)dword_82B050, (int)dword_5A041C, 101, 1, v11, 0, -1, -1, 0xFFFF, 0, 0, v38, v39);
            }
            break;
          case 5:
            sub_43AB90(this, 6);
            break;
          case 6:
            sub_47CE80(dword_8FCDD8);
            break;
          case 7:
            sub_43AB90(this, 14);
            break;
          case 8:
            sub_43AB90(this, 11);
            break;
          case 9:
            sub_43AB90(this, 12);
            break;
          case 11:
            sub_43AB90(this, 13);
            break;
          case 12:
            if ( sub_46B6E0(this) )
            {
              sub_43AB90(this, 0);
              nullsub_5();
              sub_441FD0((_BYTE *)this + 29440, 0);
            }
            break;
          case 13:
            if ( sub_46B6E0(this) )
            {
              sub_43AB90(this, 1);
              nullsub_5();
            }
            break;
          case 14:
            sub_43EF10(this);
            break;
          case 15:
            sub_43AB90(this, 16);
            break;
          case 16:
            dword_4AF2B0(dword_8FB92C);
            dword_4AF2A0(dword_8FB92C, 5);
            v14 = dword_8FB92C;
            v15 = sub_480550(1072);
            dword_4AF26C(v14, v15);
            v16 = strlen((const char *)sub_480550(1072)) + 1;
            dword_4AF2B4(dword_8FB92C, 177, v16 - 1, v16 - 1);
            goto LABEL_85;
          case 17:
            dword_4AF2B0(dword_8FB92C);
            dword_4AF2A0(dword_8FB92C, 5);
            dword_4AF26C(dword_8FB92C, asc_4B5614);
            v17 = strlen(asc_4B5614) + 1;
            dword_4AF2B4(dword_8FB92C, 177, v17 - 1, v17 - 1);
LABEL_85:
            sub_47B3D0(&dword_8FB928);
            v6 = a3;
            break;
          case 18:
            v18 = sub_401430(2);
            v19 = sub_401430(7);
            if ( this[7318] != 1 )
            {
              v20 = (const char *)sub_480550(2633);
              sub_4366D0(this, v18, v19, v20);
              this[7318] = 1;
              v21 = sub_480550(2619);
              sub_41FDD0(v21);
              v22 = sub_480550(2631);
              sub_41FDD0(v22);
              sub_41FD70(1335, 1335, 1335);
              goto LABEL_89;
            }
            v23 = (const char *)sub_480550(2626);
            sub_4366D0(this, v18, v19, v23);
            this[7318] = 0;
            v24 = sub_480550(2618);
            sub_41FDD0(v24);
            sub_41FD70(1334, 1334, 1334);
            v6 = a3;
            break;
          case 19:
            v25 = sub_401430(2);
            v26 = sub_401430(7);
            if ( this[7318] == 2 )
            {
              v30 = (const char *)sub_480550(2626);
              sub_4366D0(this, v25, v26, v30);
              this[7318] = 0;
              v31 = sub_480550(2619);
              sub_41FDD0(v31);
LABEL_89:
              sub_41FD70(1336, 1336, 1336);
              v6 = a3;
            }
            else
            {
              v27 = (const char *)sub_480550(2635);
              sub_4366D0(this, v25, v26, v27);
              this[7318] = 2;
              v28 = sub_480550(2618);
              sub_41FDD0(v28);
              v29 = sub_480550(2630);
              sub_41FDD0(v29);
              sub_41FD70(1334, 1334, 1334);
              sub_41FD70(1337, 1337, 1337);
              v6 = a3;
            }
            break;
          case 20:
            if ( this[7338] == 1 )
            {
              sub_41FD70(-1, -1, -1);
              v32 = sub_480550(2637);
            }
            else
            {
              sub_41FD70(1338, 1338, 1338);
              v32 = sub_480550(2638);
            }
            sub_41FDD0(v32);
            v33 = sub_480550(1070);
            goto LABEL_106;
          case 21:
            if ( this[7339] == 1 )
            {
              sub_41FD70(-1, -1, -1);
              v34 = sub_480550(2639);
            }
            else
            {
              sub_41FD70(1340, 1340, 1340);
              v34 = sub_480550(2640);
            }
            sub_41FDD0(v34);
            v33 = sub_480550(1071);
            goto LABEL_106;
          case 22:
            if ( this[7340] == 1 )
            {
              sub_41FD70(-1, -1, -1);
              v35 = sub_480550(2643);
            }
            else
            {
              sub_41FD70(1342, 1342, 1342);
              v35 = sub_480550(2644);
            }
            sub_41FDD0(v35);
            v33 = sub_480550(1073);
LABEL_106:
            sub_47CA80((char *)dword_8FCDD8, v33);
            break;
          default:
            break;
        }
      }
      ++v42;
    }
    while ( v42 < v43 );
  }
  if ( this[7343] == 1 && (*(int (__thiscall **)(_DWORD *, int, int, int))(this[6654] + 16))(this + 6654, v6, a4, 1) )
  {
    if ( sub_43AB90(this, 18) == 1 )
      sub_47BF30(dword_8FCDD8);
    return 0;
  }
  v36 = (int (__stdcall *)(_DWORD *, int, int))dword_4AF2AC;
  if ( dword_4AF2AC(this + 7324, a3, a4)
    && !this[7317]
    && (*(int (__thiscall **)(_DWORD *, int, int, int))(this[6744] + 16))(this + 6744, a3, a4, 1) )
  {
    sub_43AB90(this, 22);
    return 0;
  }
  if ( !v36(this + 7324, a3, a4) || this[7317] )
    return 0;
  if ( (unsigned int)(dword_4AF2FC(v38, v39, v40, v41, v42) - this[6830]) > 0x3E8 )
  {
    if ( this[7319] )
    {
      this[7316] = 0;
      this[7319] = 0;
    }
    else
    {
      sub_47BF30(dword_8FCDD8);
      v37 = this[7319];
      this[7342] = 1;
      this[7319] = v37 == 0;
    }
    if ( dword_4AF110(260, v44) )
    {
      if ( !v44[0] )
        return 1;
      strcat(v44, MEMORY[0x4B5864]);
      MEMORY[0x49FA18](v45, "%d", this[7319]);
      dword_4AF0C4(MEMORY[0x4B584C], MEMORY[0x4B5854], v45, v44);
    }
  }
  return 1;
}
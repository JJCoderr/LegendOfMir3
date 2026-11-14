int __userpurge sub_406750@<eax>(int a1@<ecx>, int a2@<edi>, unsigned __int8 a3, int a4, int a5)
{
  int v6; // eax
  int v7; // ecx
  int v8; // ecx
  unsigned __int16 v9; // bp
  unsigned __int8 v10; // al
  char *v11; // eax
  char *v12; // edi
  _DWORD *v13; // eax
  _DWORD *v14; // eax
  int v15; // ebx
  int v16; // ebp
  int v17; // ecx
  int v18; // eax
  int v19; // eax
  int v20; // eax
  int v21; // edi
  int v22; // eax
  unsigned __int8 v23; // cl
  int v24; // edx
  int v25; // eax
  int v26; // ecx
  int v27; // ecx
  unsigned __int16 v29; // [esp+20h] [ebp-14h]

  if ( (unsigned __int8)a4 >= 8u || *(_BYTE *)(a1 + 73) >= 0x96u )
    return 0;
  if ( !*(_DWORD *)(a1 + 76) )
    *(_DWORD *)(a1 + 76) = 328 * *(unsigned __int8 *)(a1 + 73) + 5876332;
  switch ( *(_BYTE *)(a1 + 69) )
  {
    case 0:
    case 1:
      if ( *(_BYTE *)(a1 + 71) >= 9u || a3 >= 0x21u )
        return 0;
      v6 = 3 * a3;
      v7 = (unsigned __int16)word_8FBA68[v6] + 10 * ((unsigned __int8)a4 + 300 * *(unsigned __int8 *)(a1 + 71));
      *(_DWORD *)(a1 + 112) = v7;
      *(_DWORD *)(a1 + 116) = v7 + (unsigned __int16)word_8FBA6A[v6];
      v8 = (unsigned __int16)word_8FBA6C[v6];
      goto LABEL_108;
    case 3:
      v9 = *(_WORD *)(a1 + 71);
      v10 = a3;
      v29 = v9;
      if ( a3 >= 0xBu )
        return 0;
      if ( v9 == 31 )
      {
        if ( !a3 )
          LOBYTE(a4) = 0;
        goto LABEL_29;
      }
      if ( v9 == 10 )
      {
        if ( a3 != 6 )
          goto LABEL_29;
        v11 = (char *)MEMORY[0x4A01AA](0x240u);
        if ( v11 )
          v12 = sub_449480(v11);
        else
          v12 = 0;
        if ( !(*(int (__thiscall **)(char *, int, _DWORD, _DWORD, _DWORD, _DWORD, int, _DWORD, _DWORD))(*(_DWORD *)v12 + 4))(
                v12,
                501,
                *(_DWORD *)(a1 + 132),
                *(_DWORD *)(a1 + 136),
                *(_DWORD *)(a1 + 132),
                *(_DWORD *)(a1 + 136),
                a1,
                0,
                0) )
        {
          if ( v12 )
            (**(void (__thiscall ***)(char *, int))v12)(v12, 1);
          goto LABEL_28;
        }
        if ( dword_59A9F8 )
        {
          v14 = MEMORY[0x4A01AA](0x10u);
          if ( !v14 )
          {
LABEL_28:
            v10 = 6;
            goto LABEL_29;
          }
          *v14 = &off_4AF460;
          v14[1] = 0;
          v14[3] = 0;
          v14[2] = 0;
          v14[1] = v12;
          v14[3] = 0;
          v14[2] = dword_59A9FC;
          *(_DWORD *)(dword_59A9FC + 12) = v14;
        }
        else
        {
          v13 = MEMORY[0x4A01AA](0x10u);
          if ( v13 )
          {
            *v13 = &off_4AF460;
            v13[1] = 0;
            v13[3] = 0;
            v13[2] = 0;
          }
          else
          {
            v13 = 0;
          }
          dword_59A9F8 = (int)v13;
          if ( !v13 )
            goto LABEL_28;
          v13[1] = v12;
          v14 = (_DWORD *)dword_59A9F8;
        }
        dword_59A9FC = (int)v14;
        ++dword_59AA08;
        goto LABEL_28;
      }
      if ( (v9 == 530 || v9 == 531 || v9 == 532 || v9 == 533 || v9 == 538) && a3 == 4 )
        LOBYTE(a4) = 0;
LABEL_29:
      v15 = v9;
      if ( v9 <= 0x8Du )
      {
        if ( v9 != 141 )
        {
          v16 = v9 - 67;
          if ( (unsigned int)(v15 - 67) <= 0x3C )
          {
            switch ( byte_406E20[v16] )
            {
              case 0:
                LOBYTE(a4) = v10 == 2;
                break;
              case 1:
                v9 = v29;
                goto LABEL_64;
              case 2:
                LOBYTE(a4) = 1;
                break;
              case 3:
                break;
            }
          }
          v9 = v29;
          goto LABEL_45;
        }
        sub_40DB10(a1, (int)dword_59AA6C, v10 + 559, a4, -1);
LABEL_57:
        *(_DWORD *)(a1 + 400240) = 1;
        goto LABEL_45;
      }
      if ( v9 <= 0xB8u )
      {
        if ( v9 == 184 )
        {
          if ( a3 == 4 )
            goto LABEL_45;
          sub_40DB10(a1, (int)dword_59AA6C, v10 + 598, a4, -1);
          goto LABEL_57;
        }
        if ( v9 != 153 && v9 != 179 )
          goto LABEL_45;
LABEL_64:
        LOBYTE(a4) = 0;
        goto LABEL_45;
      }
      if ( v9 > 0x219u )
        goto LABEL_45;
      if ( v9 >= 0x216u )
        goto LABEL_64;
      if ( v9 >= 0xBBu && v9 <= 0xBDu )
        LOBYTE(a4) = 0;
LABEL_45:
      sub_46F690(dword_8FBA50, v9);
      if ( v9 >= 0x3E8u )
      {
        v20 = (unsigned __int16)word_8FBB2E[3 * a3] + 10 * ((unsigned __int8)a4 + 100 * ((v15 - 2000) / 10));
        *(_DWORD *)(a1 + 112) = v20;
        *(_DWORD *)(a1 + 116) = v20 + (unsigned __int16)word_8FBB30[3 * a3];
        *(_DWORD *)(*(_DWORD *)(a1 + 400440) + 5) = (unsigned __int16)word_8FBB32[3 * a3];
      }
      else
      {
        v17 = 3 * a3;
        v18 = (unsigned __int16)word_8FBB2E[v17] + 10 * ((unsigned __int8)a4 + 100 * (v15 % 10));
        *(_DWORD *)(a1 + 112) = v18;
        *(_DWORD *)(a1 + 116) = v18 + (unsigned __int16)word_8FBB30[v17];
        *(_DWORD *)(*(_DWORD *)(a1 + 400440) + 5) = (unsigned __int16)word_8FBB32[v17];
        if ( v29 == 141 && a3 == 4 )
        {
          v19 = (unsigned __int16)word_8FBB46 + 4 * (5 * (unsigned __int8)a4 + 250);
          *(_DWORD *)(a1 + 112) = v19;
          *(_DWORD *)(a1 + 116) = v19 + (unsigned __int16)dword_8FBB48;
        }
      }
LABEL_109:
      if ( a5 )
        *(_DWORD *)(*(_DWORD *)(a1 + 400440) + 5) = a5;
      v27 = *(_DWORD *)(a1 + 112);
      *(_BYTE *)(a1 + 120) = a3;
      *(_BYTE *)(a1 + 121) = a4;
      *(_BYTE *)(a1 + 122) = a4;
      *(_DWORD *)(a1 + 124) = v27;
      *(_DWORD *)(a1 + 128) = 0;
      if ( (_BYTE)a4 == 7 )
        *(_BYTE *)(a1 + 400445) = 2;
      else
        *(_BYTE *)(a1 + 400445) = 5;
      if ( (*(_DWORD *)(a1 + 400396) & 0x8000000) != 0 )
        *(_DWORD *)(*(_DWORD *)(a1 + 400440) + 5) *= 2;
      return 1;
    case 4:
      LOWORD(a2) = *(unsigned __int8 *)(a1 + 70);
      *(_BYTE *)(a1 + 73) = -114;
      a3 = 0;
      LOBYTE(a4) = 0;
      sub_46F3B0(a2 + 77);
      v21 = (unsigned __int16)dword_8FBB70 + 100 * (unsigned __int16)a2;
      v22 = *(_DWORD *)(a1 + 400440);
      *(_DWORD *)(a1 + 112) = v21;
      *(_DWORD *)(a1 + 116) = v21 + HIWORD(dword_8FBB70);
      *(_DWORD *)(v22 + 5) = (unsigned __int16)dword_8FBB74;
      goto LABEL_109;
    case 5:
      v23 = *(_BYTE *)(a1 + 71);
      if ( v23 >= 0x64u || a3 >= 3u )
        return 0;
      v24 = (unsigned __int8)a4 % 3;
      LOBYTE(a4) = (unsigned __int8)a4 % 3;
      if ( a3 == 1
        && (v23 == 24
         || v23 == 25
         || v23 == 34
         || v23 == 35
         || v23 == 43
         || v23 == 44
         || v23 == 45
         || v23 == 46
         || v23 == 47
         || v23 == 48
         || v23 == 49
         || v23 == 50
         || v23 == 58
         || v23 == 76
         || v23 == 69
         || v23 == 80
         || v23 == 82
         || v23 == 83
         || v23 == 84
         || v23 == 85
         || v23 == 86) )
      {
        a3 = 0;
      }
      if ( v23 == 40 || v23 == 56 || v23 == 57 )
        LOBYTE(a4) = 0;
      if ( v23 == 51 || v23 == 52 || v23 == 53 || v23 == 54 || v23 == 55 || v23 == 59 || v23 == 64 || v23 == 65 )
      {
        a3 = 0;
        LOBYTE(a4) = 0;
      }
      if ( v23 == 87 )
      {
        sub_40DB10(a1, (int)dword_59AA6C, 607, a4, -1);
        *(_DWORD *)(a1 + 400240) = 1;
      }
      else
      {
        LOWORD(v24) = v23;
        sub_46F3B0(v24);
        v25 = 6 * a3;
        v26 = *(unsigned __int16 *)((char *)&dword_8FBB70 + v25)
            + 10 * ((unsigned __int8)a4 + 10 * *(unsigned __int8 *)(a1 + 71));
        *(_DWORD *)(a1 + 112) = v26;
        *(_DWORD *)(a1 + 116) = v26 + *(unsigned __int16 *)((char *)&dword_8FBB70 + v25 + 2);
        v8 = *(unsigned __int16 *)((char *)&dword_8FBB74 + v25);
LABEL_108:
        *(_DWORD *)(*(_DWORD *)(a1 + 400440) + 5) = v8;
      }
      goto LABEL_109;
    default:
      return 0;
  }
}
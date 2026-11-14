int __userpurge sub_405E50@<eax>(
        unsigned __int8 *a1@<ecx>,
        int a2@<esi>,
        int a3,
        int a4,
        unsigned __int8 a5,
        unsigned __int8 a6,
        int a7,
        int a8)
{
  unsigned __int8 *v8; // ebp
  int v9; // eax
  int v10; // ecx
  int v11; // ecx
  unsigned __int16 v12; // ax
  unsigned __int16 v13; // di
  int v14; // esi
  unsigned __int8 v15; // al
  int v16; // ecx
  int v17; // edx
  int v18; // eax
  int v19; // edi
  unsigned __int8 v20; // al
  int v21; // eax
  int v22; // ecx
  int v23; // eax
  char v24; // al
  unsigned __int8 v25; // al
  unsigned __int8 v26; // cl
  int v27; // eax
  int v28; // ecx
  int v29; // eax
  int v30; // edx
  int v31; // eax
  int v32; // edx
  int v33; // esi
  int result; // eax
  _DWORD v35[8]; // [esp+10h] [ebp-20h] BYREF

  v8 = a1;
  if ( a6 >= 8u )
    return 0;
  switch ( *(_BYTE *)a4 )
  {
    case 0:
      if ( *(_BYTE *)(a4 + 2) >= 9u || a5 >= 0x21u )
        return 0;
      a1[73] = 71;
      goto LABEL_9;
    case 1:
      if ( *(_BYTE *)(a4 + 2) >= 9u || a5 >= 0x21u )
        return 0;
      a1[73] = 76;
LABEL_9:
      v9 = 3 * a5;
      v10 = (unsigned __int16)word_8FBA68[v9] + 10 * (a6 + 300 * *(unsigned __int8 *)(a4 + 2));
      *((_DWORD *)v8 + 28) = v10;
      *((_DWORD *)v8 + 29) = v10 + (unsigned __int16)word_8FBA6A[v9];
      v11 = (unsigned __int16)word_8FBA6C[v9];
      goto LABEL_55;
    case 3:
      v12 = *(_WORD *)(a4 + 2);
      v13 = v12;
      if ( a5 >= 0xBu )
        return 0;
      v14 = v12;
      if ( v12 > 0x7Fu )
      {
        if ( v12 > 0xBDu )
        {
          if ( v12 < 0x216u || v12 > 0x219u )
            goto LABEL_23;
        }
        else if ( v12 < 0xBBu && v12 != 153 && v12 != 179 )
        {
          goto LABEL_23;
        }
LABEL_22:
        a6 = 0;
        goto LABEL_23;
      }
      if ( v12 == 127 )
        goto LABEL_22;
      switch ( v12 )
      {
        case 0x1Fu:
        case 0x43u:
        case 0x49u:
        case 0x68u:
          goto LABEL_22;
        case 0x6Au:
          a6 = 1;
          break;
        default:
          break;
      }
LABEL_23:
      sub_46F690(v12);
      if ( v13 < 0x7D0u )
      {
        v15 = v13 / 10 + 88;
        v8[73] = v15;
        if ( v15 < 0x96u )
        {
          v16 = 3 * a5;
          v17 = v13 % 10;
          goto LABEL_26;
        }
        return 0;
      }
      v19 = v13 - 2000;
      v20 = (v14 - 2000) / 100 - 121;
      v8[73] = v20;
      if ( v20 >= 0x96u )
        return 0;
      *((_WORD *)v8 + 201510) = sub_4014A0(v14 % 10);
      v16 = 3 * a5;
      v17 = v19 / 10;
LABEL_26:
      v18 = (unsigned __int16)word_8FBB2E[v16] + 10 * (a6 + 100 * v17);
      *((_DWORD *)v8 + 28) = v18;
      *((_DWORD *)v8 + 29) = v18 + (unsigned __int16)word_8FBB30[v16];
      *(_DWORD *)(*((_DWORD *)v8 + 100110) + 5) = (unsigned __int16)word_8FBB32[v16];
LABEL_56:
      v8[120] = a5;
      *((_DWORD *)v8 + 33) = a7;
      v8[121] = a6;
      v8[122] = a6;
      *((_DWORD *)v8 + 34) = a8;
      sub_4064B0((int)v8, *(_DWORD *)a4);
      v29 = v8[73];
      v35[0] = -1090519040;
      v35[1] = 1056964608;
      memset(&v35[2], 0, 12);
      v35[5] = -1082130432;
      v35[6] = 0;
      v35[7] = 0;
      *((_DWORD *)v8 + 19) = a3 + 328 * v29;
      qmemcpy(v8 + 400252, v35, 0x20u);
      v30 = *((_DWORD *)v8 + 28);
      LOBYTE(v29) = v8[121];
      v35[0] = -1090519040;
      v35[1] = -1090519040;
      memset(&v35[2], 0, 12);
      v35[5] = -1082130432;
      v35[6] = 0;
      v35[7] = 1065353216;
      *((_DWORD *)v8 + 31) = v30;
      qmemcpy(v8 + 400284, v35, 0x20u);
      v35[0] = 1056964608;
      v35[1] = 1056964608;
      memset(&v35[2], 0, 12);
      v35[5] = -1082130432;
      v35[6] = 1065353216;
      v35[7] = 0;
      v8[122] = v29;
      v31 = *((_DWORD *)v8 + 33);
      qmemcpy(v8 + 400316, v35, 0x20u);
      v35[0] = 1056964608;
      v35[1] = -1090519040;
      memset(&v35[2], 0, 12);
      v35[5] = -1082130432;
      v35[6] = 1065353216;
      v35[7] = 1065353216;
      qmemcpy(v8 + 400348, v35, 0x20u);
      v32 = *((_DWORD *)v8 + 34);
      v33 = *((_DWORD *)v8 + 36);
      *((_DWORD *)v8 + 39) = dword_5B04EC + *((_DWORD *)v8 + 35) + 48 * (v31 - dword_5B04DC) - dword_5B04E4;
      result = 1;
      *((_DWORD *)v8 + 40) = dword_5B04F0 + v33 + 32 * (v32 - dword_5B04E0) - dword_5B04E8;
      return result;
    case 4:
      LOWORD(a2) = a1[70];
      a5 = 0;
      a6 = 0;
      a1[73] = -114;
      sub_46F3B0(a2 + 77);
      v21 = 100 * (unsigned __int16)a2 + (unsigned __int16)dword_8FBB70;
      *((_DWORD *)v8 + 28) = v21;
      v22 = v21 + HIWORD(dword_8FBB70);
      v23 = *((_DWORD *)v8 + 100110);
      *((_DWORD *)v8 + 29) = v22;
      *(_DWORD *)(v23 + 5) = (unsigned __int16)dword_8FBB74;
      goto LABEL_56;
    case 5:
      if ( *(_BYTE *)(a4 + 2) >= 0x64u || a5 >= 3u )
        return 0;
      a1[73] = 0x80;
      a6 %= 3;
      if ( a5 == 1 )
      {
        v24 = *(_BYTE *)(a4 + 2);
        if ( v24 == 24
          || v24 == 25
          || v24 == 34
          || v24 == 35
          || v24 == 43
          || v24 == 44
          || v24 == 45
          || v24 == 46
          || v24 == 47
          || v24 == 48
          || v24 == 49
          || v24 == 50 )
        {
          a5 = 0;
        }
      }
      v25 = *(_BYTE *)(a4 + 2);
      if ( v25 == 40 )
        a6 = 0;
      if ( v25 == 51 || v25 == 52 || (v26 = a1[71], v26 == 53) || v26 == 54 || v26 == 55 )
      {
        a5 = 0;
        a6 = 0;
      }
      LOWORD(a1) = v25;
      sub_46F3B0(a1);
      v27 = 6 * a5;
      v28 = *(unsigned __int16 *)((char *)&dword_8FBB70 + v27) + 10 * (a6 + 10 * *(unsigned __int8 *)(a4 + 2));
      *((_DWORD *)v8 + 28) = v28;
      *((_DWORD *)v8 + 29) = v28 + *(unsigned __int16 *)((char *)&dword_8FBB70 + v27 + 2);
      v11 = *(unsigned __int16 *)((char *)&dword_8FBB74 + v27);
LABEL_55:
      *(_DWORD *)(*((_DWORD *)v8 + 100110) + 5) = v11;
      goto LABEL_56;
    default:
      return 0;
  }
}
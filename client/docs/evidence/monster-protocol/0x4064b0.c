int __thiscall sub_4064B0(int this, int a2)
{
  int v2; // edi
  int v4; // edx
  __int16 v5; // dx
  int v6; // eax
  int v7; // ecx
  int v8; // eax
  int v9; // ecx
  unsigned __int16 v10; // cx
  unsigned __int16 v11; // di
  unsigned __int8 v12; // al
  int v13; // ecx
  int v14; // edx
  int v15; // eax
  int v16; // edi
  unsigned __int8 v17; // al
  int v18; // edx
  int v19; // eax
  int v20; // edx
  int result; // eax

  HIWORD(v2) = HIWORD(a2);
  if ( !sub_406460(a2) )
    return 0;
  *(_DWORD *)(this + 69) = a2;
  switch ( (char)a2 )
  {
    case 0:
      v5 = HIWORD(a2);
      if ( BYTE2(a2) >= 9u )
        return 0;
      *(_BYTE *)(this + 73) = 71;
      goto LABEL_7;
    case 1:
      v5 = HIWORD(a2);
      if ( BYTE2(a2) >= 9u )
        return 0;
      *(_BYTE *)(this + 73) = 76;
LABEL_7:
      v6 = 3 * *(unsigned __int8 *)(this + 120);
      v7 = (unsigned __int16)word_8FBA68[v6] + 10 * (*(unsigned __int8 *)(this + 121) + 300 * (unsigned __int8)v5);
      *(_DWORD *)(this + 112) = v7;
      *(_DWORD *)(this + 116) = v7 + (unsigned __int16)word_8FBA6A[v6];
      *(_DWORD *)(*(_DWORD *)(this + 400440) + 5) = (unsigned __int16)word_8FBA6C[v6];
      *(_BYTE *)(this + 72) = HIBYTE(v5) & 0x3F;
      *(_BYTE *)(this + 68) = HIBYTE(v5) >> 6;
      goto LABEL_18;
    case 3:
      LOBYTE(v10) = 0;
      HIBYTE(v10) = HIBYTE(a2);
      LOWORD(v4) = BYTE2(a2);
      v11 = BYTE2(a2) | v10;
      sub_46F690(v4 | v10);
      if ( v11 >= 0x3E8u )
      {
        v16 = v11 - 2000;
        v17 = v16 / 100 - 121;
        *(_BYTE *)(this + 73) = v17;
        if ( v17 >= 0x96u )
          return 0;
        v13 = 3 * *(unsigned __int8 *)(this + 120);
        v14 = v16 / 10;
      }
      else
      {
        v12 = v11 / 10 + 88;
        *(_BYTE *)(this + 73) = v12;
        if ( v12 >= 0x96u )
          return 0;
        v13 = 3 * *(unsigned __int8 *)(this + 120);
        v14 = v11 % 10;
      }
      v15 = (unsigned __int16)word_8FBB2E[v13] + 10 * (*(unsigned __int8 *)(this + 121) + 100 * v14);
      *(_DWORD *)(this + 112) = v15;
      *(_DWORD *)(this + 116) = v15 + (unsigned __int16)word_8FBB30[v13];
      *(_DWORD *)(*(_DWORD *)(this + 400440) + 5) = (unsigned __int16)word_8FBB32[v13];
LABEL_18:
      v20 = 41 * *(unsigned __int8 *)(this + 73);
      *(_DWORD *)(this + 124) = *(_DWORD *)(this + 112);
      *(_DWORD *)(this + 76) = 8 * v20 + 5876332;
      result = 1;
      break;
    case 4:
      LOWORD(v2) = *(unsigned __int8 *)(this + 70);
      *(_BYTE *)(this + 73) = -114;
      *(_BYTE *)(this + 120) = 0;
      *(_BYTE *)(this + 121) = 0;
      sub_46F3B0(v2 + 77);
      v8 = 6 * *(unsigned __int8 *)(this + 120);
      v9 = *(unsigned __int16 *)((char *)&dword_8FBB70 + v8)
         + 10 * (*(unsigned __int8 *)(this + 121) + 10 * (unsigned __int16)v2);
      goto LABEL_17;
    case 5:
      if ( BYTE2(a2) >= 0x64u )
        return 0;
      v18 = *(unsigned __int8 *)(this + 121);
      v19 = *(unsigned __int8 *)(this + 120);
      *(_BYTE *)(this + 73) = 0x80;
      v8 = 6 * v19;
      v9 = *(unsigned __int16 *)((char *)&dword_8FBB70 + v8) + 10 * (v18 + 10 * BYTE2(a2));
LABEL_17:
      *(_DWORD *)(this + 112) = v9;
      *(_DWORD *)(this + 116) = v9 + *(unsigned __int16 *)(v8 + 9419634);
      *(_DWORD *)(*(_DWORD *)(this + 400440) + 5) = *(unsigned __int16 *)((char *)&dword_8FBB74 + v8);
      goto LABEL_18;
    default:
      goto LABEL_18;
  }
  return result;
}
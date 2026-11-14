char __thiscall sub_423D50(char *this, float a2)
{
  double v2; // st7
  int v4; // ebx
  char result; // al
  __int64 v6; // rcx
  __int64 v7; // [esp+8h] [ebp-5Ch] BYREF
  int v8; // [esp+10h] [ebp-54h]
  float v9[3]; // [esp+14h] [ebp-50h] BYREF
  float v10[17]; // [esp+20h] [ebp-44h] BYREF
  float v11; // [esp+68h] [ebp+4h]

  v2 = flt_4AF468;
  v4 = LODWORD(a2) + *(_DWORD *)&this[(_DWORD)&loc_43843B + 1];
  result = this[(_DWORD)&loc_438437 + 1];
  *(_DWORD *)&this[(_DWORD)&loc_43843B + 1] = v4;
  v6 = (unsigned int)v4;
  switch ( result )
  {
    case 0:
      *(_DWORD *)&this[(_DWORD)&loc_43843B + 1] = 0;
      return result;
    case 1:
      if ( (unsigned int)v6 > 0x9C4 )
      {
        this[(_DWORD)&loc_438437 + 1] = 0;
        *(_DWORD *)&this[(_DWORD)&loc_43843B + 1] = 0;
        return result;
      }
      v7 = v6;
      v2 = (double)v6 * flt_4AF79C;
      break;
    case 2:
      if ( (unsigned int)v6 > 0x9C4 )
      {
        this[(_DWORD)&loc_438437 + 1] = BYTE4(v6);
        *(_DWORD *)&this[(_DWORD)&loc_43843B + 1] = HIDWORD(v6);
        return sub_421E80(this);
      }
      v7 = v6;
      v2 = flt_4AF3B4 - (double)v6 * flt_4AF79C;
      break;
  }
  memset(v9, 0, sizeof(v9));
  v7 = 0x4416000044480000LL;
  v11 = v2 * flt_4AF3AC;
  v8 = 0;
  sub_49239B(v10, v11, v11, v11, 1.0);
  v10[3] = 0.39215687;
  return sub_47F3F0(this + 922100, (int *)dword_8FCC64, v9, (float *)&v7, (int)v10, 0, 6u);
}
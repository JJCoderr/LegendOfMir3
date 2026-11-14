void __thiscall sub_424050(int this, int a2)
{
  unsigned int v3; // ecx
  unsigned int v4; // eax
  _DWORD *v5; // ecx
  int v6; // eax
  int v7; // ecx
  double v8; // st7
  int v9; // edx
  int v10; // eax
  int v11; // edi
  _DWORD *v12; // esi
  int v13; // edx
  int v14; // eax
  float v15[3]; // [esp+10h] [ebp-5Ch] BYREF
  float v16[3]; // [esp+1Ch] [ebp-50h] BYREF
  float v17[17]; // [esp+28h] [ebp-44h] BYREF

  if ( *(_DWORD *)((char *)&loc_438297 + this + 1) )
  {
    v3 = *(_DWORD *)((char *)&loc_438299 + this + 3);
    v4 = a2 + *(_DWORD *)((char *)&loc_43829F + this + 1);
    *(_DWORD *)((char *)&loc_43829F + this + 1) = v4;
    if ( v4 <= v3 )
    {
      v7 = *(_DWORD *)((char *)&loc_43828B + this + 1);
      v6 = *(_DWORD *)((char *)&loc_438287 + this + 1);
      v8 = (double)v6;
      v9 = *(_DWORD *)((char *)&loc_43828D + this + 3) - v6;
      v10 = *(_DWORD *)((char *)&loc_438293 + this + 1);
      v16[0] = v8;
      v16[1] = (float)v7;
      v15[0] = (float)v9;
      v16[2] = 0.0;
      v15[2] = 0.0;
      v15[1] = (float)(v10 - v7);
      sub_49239B(v17, 0.039215688, 0.039215688, 0.039215688, 1.0);
      v17[3] = 0.39215687;
      sub_47F3F0((char *)(this + 922100), (int *)dword_8FCC64, v16, v15, (int)v17, 0, 0);
      sub_48C359(&dword_8FCC50, (char *)&loc_438287 + this + 1, 0, (int)dword_505050, 1);
      v11 = 0;
      v12 = (_DWORD *)((char *)&loc_4383B7 + this + 1);
      do
      {
        if ( !*(v12 - 4) )
          break;
        *(v12 - 1) += a2;
        v13 = a2 + *v12;
        *v12 = v13;
        if ( v13 <= *(v12 - 2) )
        {
          if ( *(v12 - 1) > v12[1] )
          {
            *(v12 - 1) = 0;
            v14 = *(v12 - 3) + 1;
            *(v12 - 3) = v14;
            if ( v14 > 153 )
              *(v12 - 3) = 150;
          }
        }
        else
        {
          *(v12 - 3) = v12[2] + 139;
        }
        if ( dword_5A137C )
        {
          if ( sub_491C70(*(v12 - 3)) )
            sub_48D9F7(
              v12[3],
              v12[4],
              *(__int16 *)dword_5A13B8,
              *(__int16 *)(dword_5A13B8 + 2),
              dword_5A13BC,
              800,
              530,
              0xFFFF,
              0xFFFF);
        }
        ++v11;
        v12 += 9;
      }
      while ( v11 < 4 );
    }
    else
    {
      *(_DWORD *)((char *)&loc_438297 + this + 1) = 0;
      *(_DWORD *)((char *)&loc_437BD5 + this + 3) = 1000;
      *(_DWORD *)((char *)&loc_437BDA + this + 2) = 0;
      *(_DWORD *)((char *)&loc_438299 + this + 3) = 0;
      *(_DWORD *)((char *)&loc_43829F + this + 1) = 0;
      memset((char *)&loc_4383A3 + this + 5, 0, 0x90u);
      v5 = (_DWORD *)((char *)&loc_438287 + this + 1);
      *v5 = 0;
      v5[1] = 0;
      v5[2] = 0;
      v5[3] = 0;
      sub_47CBF0(1011, *(_DWORD *)(this + 3127564), 0, (char *)&loc_4382A4 + this);
      *(_DWORD *)((char *)&loc_4382A4 + this) = 0;
    }
  }
}
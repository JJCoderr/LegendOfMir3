int __thiscall sub_43A5D0(_DWORD *this)
{
  int result; // eax
  int v3; // edi
  _BYTE *v4; // esi
  int v5; // ebp
  double v6; // st7
  int v7; // eax
  int v8; // edi
  int v9; // edi
  __int16 *v10; // eax
  double v11; // st7
  int v12; // edi
  int v13; // [esp+24h] [ebp-98h]
  int v14; // [esp+24h] [ebp-98h]
  int v15; // [esp+28h] [ebp-94h]
  float v16; // [esp+2Ch] [ebp-90h] BYREF
  int v17; // [esp+30h] [ebp-8Ch]
  int v18; // [esp+34h] [ebp-88h]
  float v19; // [esp+38h] [ebp-84h] BYREF
  float v20; // [esp+3Ch] [ebp-80h]
  float v21; // [esp+40h] [ebp-7Ch]
  float v22; // [esp+44h] [ebp-78h]
  float v23; // [esp+48h] [ebp-74h]
  float v24; // [esp+4Ch] [ebp-70h]
  float v25; // [esp+50h] [ebp-6Ch]
  int v26; // [esp+54h] [ebp-68h]
  float v27; // [esp+58h] [ebp-64h]
  float v28; // [esp+5Ch] [ebp-60h]
  int v29; // [esp+60h] [ebp-5Ch]
  float v30; // [esp+64h] [ebp-58h]
  float v31; // [esp+68h] [ebp-54h]
  float v32; // [esp+6Ch] [ebp-50h]
  int v33; // [esp+70h] [ebp-4Ch]
  int v34; // [esp+74h] [ebp-48h]
  float v35[3]; // [esp+78h] [ebp-44h] BYREF
  int v36; // [esp+84h] [ebp-38h]

  result = this[6827];
  if ( result )
  {
    v3 = -354707 - (_DWORD)this;
    v15 = 0;
    v4 = (char *)this + 356157;
    v34 = -354707 - (_DWORD)this;
    v5 = -356157 - (_DWORD)this;
    while ( 1 )
    {
      if ( &v4[v5] == (_BYTE *)4 || &v4[v5] == (_BYTE *)8 )
        v15 += 40;
      if ( *v4 && sub_491C70((unsigned __int8)*v4 + 999) )
      {
        v29 = 0x40000000;
        v30 = 0.0;
        v28 = (float)v15;
        v17 = 0x40000000;
        v16 = v28;
        v18 = 0;
        v24 = 1.0;
        v6 = (double)*(__int16 *)dword_5A1790;
        v13 = *(__int16 *)(dword_5A1790 + 2);
        v21 = 1.0;
        v22 = v6;
        v19 = v22;
        v7 = (unsigned __int8)*v4;
        v23 = (float)v13;
        v20 = v23;
        v8 = sub_47F3A0(3, 85, v7 + 999);
        sub_49239B(v35, 1.0, 1.0, 1.0, 1.0);
        v36 = 1061734601;
        result = sub_47F3F0((char *)dword_59AA6C, (int *)dword_8FCC64, &v16, &v19, (int)v35, v8, 0);
      }
      else
      {
        v9 = (int)&v4[v3];
        result = sub_491C70(v9);
        if ( result )
        {
          v26 = -1053818880;
          v27 = 0.0;
          v17 = -1053818880;
          v18 = 0;
          v33 = 1065353216;
          v25 = (double)v15 - flt_4AF918;
          v16 = v25;
          v10 = *(__int16 **)(this[7] + 60);
          v11 = (double)*v10;
          v14 = v10[1];
          v21 = 1.0;
          v31 = v11;
          v19 = v31;
          v32 = (float)v14;
          v20 = v32;
          v12 = sub_47F3A0(3, 70, v9);
          sub_49239B(v35, 0.58823532, 0.58823532, 0.58823532, 1.0);
          v36 = 1058444951;
          result = sub_47F3F0((char *)dword_59AA6C, (int *)dword_8FCC64, &v16, &v19, (int)v35, v12, 0);
        }
      }
      ++v4;
      v15 += 40;
      if ( (int)&v4[v5] >= 12 )
        break;
      v3 = v34;
    }
  }
  return result;
}
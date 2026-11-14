int __thiscall sub_477780(int this)
{
  void (__stdcall *v1)(int *, int, int, int, int); // ebx
  int v3; // eax
  _DWORD *v4; // edi
  int v5; // ebp
  int v6; // eax
  int v7; // ecx
  int v8; // ecx
  int v9; // eax
  int v10; // eax
  int v11; // eax
  int v12; // eax
  int v13; // eax
  int v14; // ecx
  int v15; // edi
  int v16; // eax
  int v17; // ecx
  int v18; // eax
  int v19; // ecx
  int v21; // eax
  int v22; // ecx
  char *v23; // [esp+F8h] [ebp-19Ch]
  int v24; // [esp+110h] [ebp-184h] BYREF
  int v25; // [esp+114h] [ebp-180h]
  int v26; // [esp+118h] [ebp-17Ch]
  int v27; // [esp+11Ch] [ebp-178h]
  int v28; // [esp+120h] [ebp-174h]
  int v29; // [esp+124h] [ebp-170h]
  int v30; // [esp+128h] [ebp-16Ch]
  int v31; // [esp+12Ch] [ebp-168h]
  float v32; // [esp+130h] [ebp-164h] BYREF
  float v33; // [esp+134h] [ebp-160h]
  int v34; // [esp+138h] [ebp-15Ch]
  int v35; // [esp+13Ch] [ebp-158h]
  float v36[3]; // [esp+140h] [ebp-154h] BYREF
  float v37[17]; // [esp+14Ch] [ebp-148h] BYREF
  char v38[64]; // [esp+190h] [ebp-104h] BYREF

  v1 = (void (__stdcall *)(int *, int, int, int, int))dword_4AF2A8;
  v3 = *(_DWORD *)(this + 1768);
  v28 = 0;
  v29 = v3;
  v31 = 0;
  if ( v3 )
  {
    v4 = (_DWORD *)(this + 1796);
    do
    {
      if ( v28 >= 12 * *(_DWORD *)(this + 1980) )
      {
        v5 = *(_DWORD *)(v29 + 4);
        if ( v5 && sub_491C70(*(unsigned __int16 *)(v5 + 40)) )
        {
          v6 = *(_DWORD *)(this + 24);
          v24 = *(v4 - 2) + v6;
          v7 = *(_DWORD *)(this + 28);
          v25 = *(v4 - 1) + v7;
          v8 = v4[1] + v7;
          v26 = *v4 + v6;
          v27 = v8;
          if ( v28 == *(_DWORD *)(this + 2032) )
          {
            sub_49239B(v37, 0.19607843, 0.58823532, 0.58823532, 1.0);
            v32 = (float)v24;
            v33 = (float)v25;
            v30 = v27 - v25;
            v36[0] = (float)(v26 - v24);
            v36[1] = (float)(v27 - v25);
            v37[3] = 0.0039215689;
            v34 = 0;
            v36[2] = 1.0;
            sub_47F3F0((char *)dword_59AA6C, (int *)dword_8FCC64, &v32, v36, (int)v37, 0, 0);
            sub_48C359(&dword_8FCC50, &v24, 0, 11842740, 1);
          }
          sub_442200(v5, *(v4 - 2) + *(_DWORD *)(this + 24), *(v4 - 1) + *(_DWORD *)(this + 28), 3, 1);
          if ( *(_BYTE *)(this + 1524) != 2 )
          {
            MEMORY[0x49FA18](v38, "%d", *(unsigned __int16 *)(v5 + 72));
            v1(
              &v24,
              *(_DWORD *)(this + 24) + *(v4 - 2) - 1,
              *(_DWORD *)(this + 28) + *(v4 - 1) + 20,
              *(_DWORD *)(this + 24) + *v4 - 1,
              v4[1] + *(_DWORD *)(this + 28));
            v9 = sub_480550(1074);
            v10 = sub_48BB04((int)&dword_8FCC50, v9, 8, 0, 400, 0, 0, 0, 1);
            sub_48BD63(&dword_8FCC50, 0, v24, v25, v26, v27, 657930, 0, (int)v38, v10);
            v1(
              &v24,
              *(v4 - 2) + *(_DWORD *)(this + 24),
              *(_DWORD *)(this + 28) + *(v4 - 1) + 21,
              *v4 + *(_DWORD *)(this + 24),
              *(_DWORD *)(this + 28) + v4[1] + 1);
            v11 = sub_480550(1074);
            v12 = sub_48BB04((int)&dword_8FCC50, v11, 8, 0, 400, 0, 0, 0, 1);
            sub_48BD63(&dword_8FCC50, 0, v24, v25, v26, v27, 0xFFFFFF, 0, (int)v38, v12);
          }
        }
        v4 += 4;
        if ( ++v31 >= 12 )
          break;
      }
      v13 = *(_DWORD *)(v29 + 12);
      ++v28;
      v29 = v13;
    }
    while ( v13 );
  }
  if ( *(_BYTE *)(this + 1524) == 2 )
  {
    v14 = *(_DWORD *)(this + 1784);
    if ( v14 % 12 )
    {
      v15 = v14 / 12 + 1;
    }
    else if ( v14 )
    {
      v15 = v14 / 12;
    }
    else
    {
      v15 = 1;
    }
    v16 = *(_DWORD *)(this + 28) + *(_DWORD *)(this + 1792);
    v17 = *(_DWORD *)(this + 24) + *(_DWORD *)(this + 1788);
    v1(&v24, v17 + 30, v16 - 37, v17 + 70, v16 - 7);
    v18 = *(_DWORD *)(this + 28) + *(_DWORD *)(this + 1792);
    v19 = *(_DWORD *)(this + 24) + *(_DWORD *)(this + 1788);
    v1((int *)&v32, v19 + 60, v18 - 37, v19 + 110, v18 - 7);
    MEMORY[0x49FA18](v38, "%d", *(_DWORD *)(this + 1980) + 1);
    sub_48BD63(&dword_8FCC50, 0, v24, v25, v26, v27, (int)&off_96C8FF, 0, (int)v38, 0);
    MEMORY[0x49FA18](v38, "/    %d", v15);
    return sub_48BD63(&dword_8FCC50, 0, SLOBYTE(v32), SLODWORD(v33), v34, v35, (int)&off_96C8FF, 0, (int)v38, 0);
  }
  else
  {
    v21 = *(_DWORD *)(this + 28) + *(_DWORD *)(this + 1792);
    v22 = *(_DWORD *)(this + 1788) + *(_DWORD *)(this + 24);
    v1(&v24, v22 - 5, v21 - 45, v22 + 158, v21 - 1);
    MEMORY[0x49FA18](v38, "%d", *(_DWORD *)(this + 1984));
    if ( *(_DWORD *)(this + 1984) == -1 )
      v23 = MEMORY[0x4B57F8];
    else
      v23 = v38;
    return sub_48BD63(&dword_8FCC50, 0, v24, v25, v26, v27, (int)&off_96C8FF, 1973790, (int)v23, 0);
  }
}
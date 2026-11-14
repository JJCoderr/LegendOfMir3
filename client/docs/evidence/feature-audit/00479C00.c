int __thiscall sub_479C00(int this, char *Str)
{
  int v3; // eax
  _DWORD *v4; // edi
  int v5; // ecx
  int v6; // eax
  int j; // eax
  void *v8; // edi
  int v9; // ecx
  int v10; // eax
  int v11; // ebx
  int v12; // edi
  int k; // esi
  const char *v14; // ebx
  const char *v16; // edi
  char *v17; // eax
  char *v18; // eax
  char *v19; // eax
  char *v20; // eax
  char *v21; // eax
  char *v22; // eax
  char *v23; // eax
  char *v24; // ebp
  int v25; // [esp+Ch] [ebp-28h]
  int v26; // [esp+10h] [ebp-24h]
  char *v27; // [esp+1Ch] [ebp-18h]
  char *v28; // [esp+20h] [ebp-14h]
  char *v29; // [esp+24h] [ebp-10h]
  char *String; // [esp+28h] [ebp-Ch]
  char *v31; // [esp+2Ch] [ebp-8h]
  char *i; // [esp+30h] [ebp-4h]
  char *v33; // [esp+38h] [ebp+4h]

  v3 = *(_DWORD *)(this + 1608);
  v4 = (_DWORD *)(this + 1604);
  for ( i = (char *)(this + 1604); v3; v3 = v4[1] )
  {
    MEMORY[0x4939F6](*(void **)(v3 + 4));
    v5 = v4[1];
    v6 = *(_DWORD *)(v5 + 12);
    v4[1] = v6;
    if ( v6 )
      *(_DWORD *)(v6 + 8) = 0;
    if ( v5 )
      (**(void (__thiscall ***)(int, int))v5)(v5, 1);
  }
  v4[1] = 0;
  v4[2] = 0;
  v4[5] = 0;
  *(_DWORD *)(this + 1756) = 0;
  *(_DWORD *)(this + 1760) = 0;
  for ( j = *(_DWORD *)(this + 1768); j; j = *(_DWORD *)(this + 1768) )
  {
    v8 = *(void **)(j + 4);
    if ( v8 )
    {
      nullsub_5();
      MEMORY[0x4939F6](v8);
    }
    v9 = *(_DWORD *)(this + 1768);
    v10 = *(_DWORD *)(v9 + 12);
    *(_DWORD *)(this + 1768) = v10;
    if ( v10 )
      *(_DWORD *)(v10 + 8) = 0;
    if ( v9 )
      (**(void (__thiscall ***)(int, int))v9)(v9, 1);
  }
  *(_DWORD *)(this + 1768) = 0;
  *(_DWORD *)(this + 1772) = 0;
  *(_DWORD *)(this + 1784) = 0;
  *(_DWORD *)(this + 1980) = 0;
  sub_4317B0((_DWORD *)this, 1260, 0, 225, 292, 224, v25, v26);
  *(_BYTE *)(this + 1524) = 3;
  v11 = 43;
  v12 = this + 1788;
  do
  {
    for ( k = 323; k < 475; k += 38 )
    {
      dword_4AF2A8(v12, k, v11, k + 37, v11 + 37);
      v12 += 16;
    }
    v11 += 38;
  }
  while ( v11 < 157 );
  v14 = Str;
  if ( !strlen(Str) )
    return 0;
  if ( *Str )
  {
    while ( 1 )
    {
      v16 = v14;
      v17 = MEMORY[0x49FFB0](v14, 47);
      if ( !v17 )
        break;
      *v17 = 0;
      v33 = v17 + 1;
      v18 = MEMORY[0x49FFB0](v17 + 1, 47);
      if ( !v18 )
        break;
      *v18 = 0;
      v27 = v18 + 1;
      v19 = MEMORY[0x49FFB0](v18 + 1, 47);
      if ( !v19 )
        break;
      *v19 = 0;
      v28 = v19 + 1;
      v20 = MEMORY[0x49FFB0](v19 + 1, 47);
      if ( !v20 )
        break;
      *v20 = 0;
      v29 = v20 + 1;
      v21 = MEMORY[0x49FFB0](v20 + 1, 47);
      if ( !v21 )
        break;
      *v21 = 0;
      String = v21 + 1;
      v22 = MEMORY[0x49FFB0](v21 + 1, 47);
      if ( !v22 )
        break;
      *v22 = 0;
      v31 = v22 + 1;
      v23 = MEMORY[0x49FFB0](v22 + 1, 47);
      if ( !v23 )
        break;
      *v23 = 0;
      v14 = v23 + 1;
      v24 = (char *)MEMORY[0x4A01AA](0x3Cu);
      strcpy(v24, v16);
      *((_DWORD *)v24 + 8) = MEMORY[0x49FAF5](v33);
      *((_DWORD *)v24 + 9) = MEMORY[0x49FAF5](v27);
      *((_DWORD *)v24 + 10) = MEMORY[0x49FAF5](v28);
      *((_DWORD *)v24 + 11) = 0;
      *((_DWORD *)v24 + 12) = MEMORY[0x49FAF5](v29);
      *((_DWORD *)v24 + 13) = MEMORY[0x49FAF5](String);
      *((_DWORD *)v24 + 14) = MEMORY[0x49FAF5](v31);
      (*(void (__thiscall **)(char *, char *))(*(_DWORD *)i + 4))(i, v24);
      if ( !*v14 )
        return 1;
    }
    return 0;
  }
  return 1;
}
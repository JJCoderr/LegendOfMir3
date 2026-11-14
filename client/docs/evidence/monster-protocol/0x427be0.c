int __thiscall sub_427BE0(_DWORD *this, const void *a2, int a3)
{
  int v3; // ebx
  int v4; // edi
  _BYTE *v6; // esi
  const void *v7; // esi
  const void *v8; // eax

  v3 = a3;
  v4 = (int)a2;
  v6 = (_BYTE *)MEMORY[0x4A0930](a2, 33, a3);
  if ( !v6 )
    return v4;
  while ( 1 )
  {
    *v6 = 0;
    v7 = v6 + 1;
    sub_4282C0(this, v3, (unsigned int)this, v4, (int)v7, (_BYTE *)(v4 + 1));
    v3 += v4 - (_DWORD)v7;
    v8 = (const void *)MEMORY[0x4A0930](v7, 35, v3);
    v4 = (int)v8;
    if ( !v8 )
      break;
    v6 = (_BYTE *)MEMORY[0x4A0930](v8, 33, v3);
    if ( !v6 )
      return v4;
  }
  return 0;
}
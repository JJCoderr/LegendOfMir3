int __thiscall sub_46EF60(_DWORD *this, __int16 a2)
{
  int v2; // esi
  int result; // eax
  int v4; // edx
  int v5; // edi
  _WORD *i; // ecx

  v2 = this[1];
  result = 0;
  v4 = 0;
  if ( v2 > 0 )
  {
    v5 = this[4];
    for ( i = (_WORD *)(v5 + 12); a2 != *i; i += 24 )
    {
      if ( ++v4 >= v2 )
        return result;
    }
    return v5 + 48 * v4;
  }
  return result;
}
int __thiscall sub_46EEE0(int *this, __int16 a2)
{
  int v2; // esi
  int result; // eax
  int v4; // edx
  int v5; // edi
  _WORD *i; // ecx

  v2 = *this;
  result = 0;
  v4 = 0;
  if ( *this > 0 )
  {
    v5 = this[3];
    for ( i = (_WORD *)(v5 + 12); a2 != *i; i += 24 )
    {
      if ( ++v4 >= v2 )
        return result;
    }
    return v5 + 48 * v4;
  }
  return result;
}
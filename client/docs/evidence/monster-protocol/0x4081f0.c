int __thiscall sub_4081F0(_DWORD *this, unsigned __int16 *a2)
{
  unsigned __int16 v3; // cx
  int v4; // eax
  int v5; // eax
  double v6; // st7
  double v7; // st7
  int result; // eax

  v3 = *(_WORD *)(this[100110] + 12);
  v4 = a2[3] - v3;
  if ( v4 )
  {
    if ( v3 )
    {
      if ( dword_8F1D30 )
      {
        this[this[100780] + 100756] = v4;
        this[2 * this[100780] + 100764] = this[39];
        this[2 * this[100780] + 100765] = this[40];
        *((_WORD *)this + this[100780] + 201562) = 0;
        v5 = this[100780] + 1;
        this[100780] = v5;
        if ( v5 > 7 )
          this[100780] = 0;
      }
    }
  }
  v6 = flt_4AF468;
  if ( a2[5] )
    v6 = (double)a2[3] / (double)a2[5];
  v7 = v6 * flt_4AF464;
  *(_WORD *)(this[100110] + 12) = a2[3];
  *(_WORD *)(this[100110] + 14) = a2[4];
  *(_WORD *)(this[100110] + 10) = a2[5];
  result = MEMORY[0x49FE1C](v7);
  *(_BYTE *)(this[100110] + 9) = result;
  return result;
}
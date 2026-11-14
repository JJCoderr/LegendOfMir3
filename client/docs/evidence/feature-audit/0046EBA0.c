int __thiscall sub_46EBA0(_DWORD *this, int a2, int a3)
{
  _DWORD *v4; // edi
  int (__stdcall *v5)(_DWORD *, int, int); // edi
  int v6; // esi
  int v7; // edi
  const char *v8; // eax
  int v10; // [esp+44h] [ebp-4h]

  dword_4AF2A8(this + 217, this[213], this[214], this[215], this[216]);
  v10 = 0;
  v4 = this + 21;
  do
  {
    if ( (*(int (__thiscall **)(_DWORD *, int, int))(*v4 + 12))(v4, a2, a3) )
      return 1;
    v4 += 45;
    ++v10;
  }
  while ( v10 < 4 );
  v5 = (int (__stdcall *)(_DWORD *, int, int))dword_4AF2AC;
  if ( dword_4AF2AC(this + 209, a2, a3) && !v5(this + 213, a2, a3) && v5(this + 205, a2, a3) )
  {
    if ( v5(this + 213, a2, a3) )
    {
      v6 = sub_401430(2);
      v7 = sub_401430(7);
      v8 = (const char *)sub_480550(4303);
      sub_4366D0(dword_760644, v6, v7, v8);
      return 0;
    }
    this[203] = MEMORY[0x49FE1C](((double)dword_767850 * flt_4AFA6C + (double)a2 - (double)(int)this[205]) * flt_4AFC14);
    this[204] = MEMORY[0x49FE1C](((double)dword_767854 * flt_4AFA64 + (double)a3 - (double)(int)this[206]) * flt_4AFC10);
  }
  return 0;
}
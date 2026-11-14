int __thiscall sub_46E500(_DWORD *this)
{
  void (__stdcall *v2)(_DWORD *, _DWORD, _DWORD, _DWORD, _DWORD); // edi
  int v3; // esi
  int v4; // edi
  int result; // eax

  sub_4315D0(this);
  v2 = (void (__stdcall *)(_DWORD *, _DWORD, _DWORD, _DWORD, _DWORD))dword_4AF2A8;
  this[203] = 0;
  this[204] = 0;
  this[201] = 0;
  this[202] = 0;
  this[223] = 0;
  v2(this + 205, 0, 0, 0, 0);
  v2(this + 213, 0, 0, 0, 0);
  v2(this + 209, 0, 0, 0, 0);
  v2(this + 217, 0, 0, 0, 0);
  this[221] = 1;
  this[222] = 0;
  v3 = (int)(this + 21);
  v4 = 4;
  do
  {
    result = sub_41FAA0(v3);
    v3 += 180;
    --v4;
  }
  while ( v4 );
  return result;
}
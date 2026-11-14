int __thiscall sub_4094A0(_DWORD *this, int a2)
{
  int v2; // eax
  int v4; // edx
  int v6; // [esp-4h] [ebp-8h]

  v2 = a2;
  v6 = a2;
  this[33] = *(unsigned __int16 *)(a2 + 6);
  this[34] = *(unsigned __int16 *)(a2 + 8);
  LOBYTE(a2) = *(_BYTE *)(a2 + 10);
  *(_BYTE *)(this[100110] + 20) = *(_BYTE *)(v2 + 11);
  (*(void (__thiscall **)(_DWORD *, int))(*this + 36))(this, v6);
  v4 = *this;
  this[100098] = 0;
  this[100108] = 0;
  return (*(int (__thiscall **)(_DWORD *, int, int, _DWORD))(v4 + 16))(this, 8, a2, 0);
}
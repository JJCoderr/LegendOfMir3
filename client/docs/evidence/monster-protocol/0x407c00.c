int __thiscall sub_407C00(_DWORD *this, int a2)
{
  int result; // eax

  result = this[100110];
  *(_BYTE *)(result + 20) = *(_BYTE *)(a2 + 6);
  return result;
}
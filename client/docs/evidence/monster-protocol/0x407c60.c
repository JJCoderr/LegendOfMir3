int __thiscall sub_407C60(_DWORD *this, int a2)
{
  int result; // eax

  result = *(_DWORD *)(a2 + 6);
  this[100099] = result;
  if ( (result & 2) != 0 )
  {
    *(_DWORD *)(this[100110] + 16) = MEMORY[0x49FDAE]();
    result = *(_DWORD *)(this[100110] + 16);
    this[100785] = result;
  }
  else
  {
    *(_DWORD *)(this[100110] + 16) = 0;
  }
  return result;
}
int __thiscall sub_407C20(_DWORD *this, int a2)
{
  int result; // eax

  *(_DWORD *)(this[100110] + 16) = MEMORY[0x49FDAE]();
  result = *(_DWORD *)(this[100110] + 16);
  this[100785] = result;
  return result;
}
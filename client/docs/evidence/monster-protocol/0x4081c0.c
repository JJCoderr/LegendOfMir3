int __thiscall sub_4081C0(void *this, int a2)
{
  int result; // eax

  result = *(_DWORD *)(a2 + 6);
  if ( (_BYTE)result == 3 || (_BYTE)result == 4 )
    return sub_4064B0((int)this, *(_DWORD *)(a2 + 6));
  return result;
}
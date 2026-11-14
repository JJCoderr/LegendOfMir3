int __thiscall sub_46ED80(_WORD *this)
{
  int result; // eax

  memset(this + 12, 0, 0xC4u);
  this[110] = 0;
  memset(this + 111, 0, 0x40u);
  this[143] = 0;
  result = 0;
  *((_DWORD *)this + 72) = 0;
  *((_DWORD *)this + 73) = 0;
  *((_DWORD *)this + 74) = 0;
  *((_DWORD *)this + 75) = 0;
  this[152] = 0;
  *((_DWORD *)this + 3) = 0;
  *((_DWORD *)this + 4) = 0;
  *((_DWORD *)this + 5) = 0;
  *(_DWORD *)this = 0;
  *((_DWORD *)this + 1) = 0;
  *((_DWORD *)this + 2) = 0;
  return result;
}
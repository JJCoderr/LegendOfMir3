int __thiscall sub_426E40(_DWORD *this, int a2, unsigned int a3)
{
  int v3; // edx

  v3 = this[895925];
  this[895922] = HIWORD(a3);
  this[895921] = (unsigned __int16)a3;
  if ( v3 && !this[1106086] )
  {
    this[895923] = this[252697] + ((unsigned __int16)a3 - this[252701]) / 48;
    this[895924] = this[252698] + (int)(HIWORD(a3) - this[252702]) / 32;
  }
  return 0;
}
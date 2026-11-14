int __thiscall sub_46E590(void *this)
{
  int v2; // esi
  int v3; // edi

  sub_431620(this);
  v2 = (int)this + 84;
  v3 = 4;
  do
  {
    sub_41FAE0(v2);
    v2 += 180;
    --v3;
  }
  while ( v3 );
  return (*(int (__thiscall **)(void *))(*(_DWORD *)this + 4))(this);
}
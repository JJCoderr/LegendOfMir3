int __thiscall sub_405C00(_DWORD *this)
{
  _DWORD *v2; // esi
  int v3; // ebx
  void *v4; // eax

  v2 = this + 41;
  if ( (int)this[41] > 0 )
  {
    v3 = this[41];
    do
    {
      v4 = (void *)sub_481AF0(v2);
      if ( v4 )
        MEMORY[0x4939F6](v4);
      --v3;
    }
    while ( v3 );
  }
  return (*(int (__thiscall **)(_DWORD *))(*this + 4))(this);
}
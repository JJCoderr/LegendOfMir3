char __thiscall sub_424AD0(int this)
{
  int v2; // eax

  if ( *(_DWORD *)(this + 3127528) )
  {
    sub_43AA30((_DWORD *)(this + 2780620), 9);
    (*(void (__thiscall **)(int, _DWORD))(*(_DWORD *)(this + 3127480) + 16))(this + 3127480, 0);
    *(_DWORD *)((char *)&loc_438281 + this + 3) = 0;
    *(_DWORD *)((char *)&loc_43827F + this + 1) = 0;
  }
  if ( *(_DWORD *)(this + 2997440) )
  {
    sub_43AA30((_DWORD *)(this + 2780620), 2);
    (*(void (__thiscall **)(int, _DWORD))(*(_DWORD *)(this + 2997392) + 16))(this + 2997392, 0);
  }
  v2 = *(_DWORD *)(this + 2810108);
  if ( v2 )
  {
    LOBYTE(v2) = *(_BYTE *)(this + 2810144);
    if ( (_BYTE)v2 )
    {
      sub_43AA30((_DWORD *)(this + 2780620), 0);
      (*(void (__thiscall **)(int, _DWORD))(*(_DWORD *)(this + 2810060) + 16))(this + 2810060, 0);
      sub_441FD0((_BYTE *)(this + 2810060), 0);
      *(_DWORD *)(this + 2955504) = 0;
      LOBYTE(v2) = sub_4411F0(this + 2810060);
    }
  }
  return v2;
}
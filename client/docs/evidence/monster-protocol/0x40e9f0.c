int __thiscall sub_40E9F0(_BYTE *this, int a2)
{
  unsigned __int8 v3; // al
  unsigned int v4; // edx
  unsigned __int8 v5; // cl
  int v6; // ecx
  int v7; // edx
  int v8; // eax
  int v9; // edx

  if ( this[120] != 1 )
    return 0;
  *((_DWORD *)this + 32) = 0;
  if ( a2 )
  {
    v3 = this[400205];
    if ( v3 )
    {
      *((_DWORD *)this + 31) += 2;
      v5 = this[400204] + 2;
      this[400204] = v5;
      if ( v5 >= v3 )
        this[400204] = v3;
      goto LABEL_13;
    }
    if ( (*((_DWORD *)this + 100099) & 0x8000000) == 0 )
      goto LABEL_11;
    v4 = *((_DWORD *)this + 100113) + 1;
    *((_DWORD *)this + 100113) = v4;
    if ( v4 > 0x186A0 )
      *((_DWORD *)this + 100113) = 1;
    if ( (this[400452] & 1) == 0 )
    {
LABEL_11:
      v6 = *((_DWORD *)this + 100050);
      v7 = *((_DWORD *)this + 31) + 1;
      *((_DWORD *)this + 31) = v7;
      if ( v6 )
        *((_DWORD *)this + 31) = v7 + 1;
LABEL_13:
      if ( *((_DWORD *)this + 31) >= *((_DWORD *)this + 29) - (unsigned int)(unsigned __int8)this[400445]
        && !*((_DWORD *)this + 100112) )
      {
        sub_407A40((int)this);
        *((_DWORD *)this + 100112) = 1;
      }
      goto LABEL_16;
    }
    return 0;
  }
LABEL_16:
  if ( *((_DWORD *)this + 31) < *((_DWORD *)this + 29) )
  {
    if ( this[400205] )
      sub_4075B0((int)this);
    else
      sub_407120((int)this);
    return 1;
  }
  else
  {
    v8 = *((_DWORD *)this + 28);
    v9 = *(_DWORD *)this;
    *((_DWORD *)this + 31) = v8;
    LOBYTE(v8) = this[121];
    *((_DWORD *)this + 37) = 0;
    *((_DWORD *)this + 38) = 0;
    *((_DWORD *)this + 35) = 0;
    *((_DWORD *)this + 36) = 0;
    *((_DWORD *)this + 100052) = 0;
    *((_DWORD *)this + 100053) = 0;
    *((_DWORD *)this + 100054) = 0;
    this[400220] = 0;
    *((_DWORD *)this + 100059) = 0;
    *((_DWORD *)this + 100060) = 0;
    this[400204] = 0;
    this[400205] = 0;
    *((_DWORD *)this + 100112) = 0;
    (*(void (__thiscall **)(_BYTE *, _DWORD, int, _DWORD))(v9 + 16))(this, 0, v8, 0);
    return 1;
  }
}
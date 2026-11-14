int __thiscall sub_426B20(int this, int a2, unsigned int a3)
{
  int v4; // eax
  int v5; // eax
  unsigned __int8 v6; // cl
  int v7; // eax
  int v8; // eax
  int v9; // edx
  int result; // eax

  *(_DWORD *)(this + 3583688) = HIWORD(a3);
  *(_DWORD *)(this + 3583684) = (unsigned __int16)a3;
  if ( !sub_420A00((_DWORD *)(this + 3610584), (unsigned __int16)a3, HIWORD(a3)) && *(_DWORD *)(this + 3583700) )
  {
    v4 = *(_DWORD *)(this + 3583688) - *(_DWORD *)(this + 1010808);
    *(_DWORD *)(this + 3583692) = *(_DWORD *)(this + 1010788)
                                + (*(_DWORD *)(this + 3583684) - *(_DWORD *)(this + 1010804)) / 48;
    *(_DWORD *)(this + 3583696) = *(_DWORD *)(this + 1010792) + v4 / 32;
    v5 = *(_DWORD *)(this + 3622528);
    if ( v5 )
    {
      v6 = *(_BYTE *)(v5 + 69);
      if ( v6 == 3 || v6 <= 1u && ((v7 = *(_DWORD *)(v5 + 400428), v7 == 255) || v7 == 27631) )
        *(_DWORD *)(this + 3622528) = 0;
    }
    if ( !sub_43BA70((_DWORD *)(this + 2780620), *(_DWORD *)(this + 3583684), *(_DWORD *)(this + 3583688))
      && !dword_4AF2AC(this + 2783784, *(_DWORD *)(this + 3583684), *(_DWORD *)(this + 3583688))
      && !*(_DWORD *)(this + 4424344) )
    {
      if ( (dword_4AF230(17) & 0x80u) != 0 )
      {
        v8 = *(_DWORD *)(this + 3622516);
        if ( v8 )
        {
          v9 = **(_DWORD **)(v8 + 400440);
          if ( v9 != **(_DWORD **)(this + 3580384) && *(_BYTE *)(v8 + 69) <= 1u && !*(_DWORD *)(v8 + 400432) )
          {
            sub_47CE40(v9, *(_DWORD *)(v8 + 132), *(_DWORD *)(v8 + 136));
            return 0;
          }
        }
      }
      result = strcmp((const char *)(this + 2780680), (const char *)sub_480550(1050));
      if ( !result )
      {
        memset((void *)(this + 2780652), 0, 0xC3Cu);
        return result;
      }
      sub_418660((_DWORD *)(this + 3179944), *(_DWORD *)(this + 3583684), *(_DWORD *)(this + 3583688));
    }
  }
  return 0;
}
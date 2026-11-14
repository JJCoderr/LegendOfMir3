int __stdcall sub_48A9D0(int a1)
{
  _DWORD v2[4]; // [esp+0h] [ebp-158h] BYREF
  _DWORD v3[17]; // [esp+10h] [ebp-148h] BYREF
  char v4[260]; // [esp+54h] [ebp-104h] BYREF

  if ( a1 )
  {
    if ( byte_4B9798 == 1 )
    {
      dword_4AF110(260, v4);
      v3[0] = 68;
      strcat(v4, MEMORY[0x4B7038]);
      memset(&v3[1], 0, 64);
      memset(v2, 0, sizeof(v2));
      if ( !dword_4AF0AC(v4, 0, 0, 0, 0, 0, 0, 0, v3, v2) )
        dword_4AF284(0, MEMORY[0x4B702C], MEMORY[0x4B5180], 16);
      return dword_4AF2B4(dword_8FCC58, 2, 0, 0);
    }
    else
    {
      return sub_48AAE0();
    }
  }
  else
  {
    sub_4891A0();
    return dword_4AF2B4(dword_8FCC58, 2, 0, 0);
  }
}
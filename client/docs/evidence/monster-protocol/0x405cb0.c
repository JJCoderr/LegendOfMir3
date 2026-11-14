int __thiscall sub_405CB0(
        _DWORD *this,
        int a2,
        int a3,
        int a4,
        int a5,
        unsigned __int16 *a6,
        int a7,
        int a8,
        int a9,
        char a10,
        __int16 a11)
{
  if ( (unsigned __int16)a11 <= 0x94BFu )
  {
    if ( (unsigned __int16)a11 != 38079 && a11 )
    {
      if ( a11 == 1 )
        return sub_48EF15((int)&dword_8FCC50, a2, a3, a4, a5, a6, 800, 530, a7, a8, 8);
      if ( a11 != 2016 )
        goto LABEL_11;
    }
    return sub_490821(a2, a3, a4, a5, a6, 800, 530, a11, a9, this[100095] != 0);
  }
  if ( (unsigned __int16)a11 == 64511 || (unsigned __int16)a11 == 64690 || (unsigned __int16)a11 == 65504 )
    return sub_490821(a2, a3, a4, a5, a6, 800, 530, a11, a9, this[100095] != 0);
LABEL_11:
  if ( this[100095] )
    return sub_48EF15((int)&dword_8FCC50, a2, a3, a4, a5, a6, 800, 530, a7, a8, a10);
  if ( a9 )
    return sub_48FA0F(a2, a3, a4, a5, a6, 800, 530, a7, a8);
  return sub_48D9F7(a2, a3, a4, a5, a6, 800, 530, a7, a8);
}
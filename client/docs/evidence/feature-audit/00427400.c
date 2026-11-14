int __thiscall sub_427400(char *this, int a2, int a3)
{
  char *v4; // edi
  const char *v5; // eax
  int v6; // esi
  int v7; // eax
  const char *v9; // [esp-1Ch] [ebp-130h]
  int v10; // [esp+0h] [ebp-114h]
  int v11; // [esp+4h] [ebp-110h]
  char Buffer[260]; // [esp+10h] [ebp-104h] BYREF

  v4 = this + 2780620;
  if ( !sub_43E5B0((_DWORD *)this + 695155, a2, a3) && byte_8FB948 )
  {
    if ( !strcmp(&byte_8FB948, (const char *)sub_480550(1425)) )
    {
      if ( *((_DWORD *)this + 769727) )
      {
        memset(Buffer, 0, sizeof(Buffer));
        v5 = (const char *)sub_480550(1426);
        MEMORY[0x49FA18](Buffer, v5, this + 3078844);
        sub_420540((int)(this + 3610584), (int)dword_5A041C, 103, 1, Buffer, 1, -1, -1, 1038, 0, 0, v10, v11);
      }
      else
      {
        v9 = (const char *)sub_480550(1427);
        sub_420540((int)(this + 3610584), (int)dword_5A041C, 103, 0, v9, 0, -1, -1, 0xFFFF, 0, 0, v10, v11);
      }
    }
    else
    {
      if ( !strcmp(&byte_8FB948, aChatsave) )
      {
        *((_DWORD *)this + 18) = *((_DWORD *)this + 18) == 0;
      }
      else if ( !strcmp(&byte_8FB948, MEMORY[0x4B56EC]) )
      {
        *((_DWORD *)this + 10) = *((_DWORD *)this + 10) == 0;
      }
      if ( byte_8FB948 == 47 )
      {
        v6 = sub_401430(0);
        v7 = sub_401430(5);
        sub_4366D0(v4, v7, v6, &byte_8FB948);
        sub_436F10(&byte_8FB948);
        sub_437350(this + 3179677);
      }
      else if ( byte_8FB948 == 35 )
      {
        sub_4379D0(byte_8FB949);
        return 0;
      }
      if ( !strcmp(&byte_8FB948, (const char *)sub_480550(1418))
        || !strcmp(&byte_8FB948, (const char *)sub_480550(1428))
        || !strcmp(&byte_8FB948, (const char *)sub_480550(1419)) )
      {
        *((_DWORD *)this + 1105654) = 300;
        *((_DWORD *)this + 1105655) = 0;
      }
      sub_47CA80((char *)dword_8FCDD8, (int)&byte_8FB948);
    }
    memset(&byte_8FB948, 0, 0x104u);
  }
  return 0;
}
int __thiscall sub_4614D0(int this, int a2, const char *a3)
{
  char Buffer[260]; // [esp+4h] [ebp-104h] BYREF

  if ( !BYTE2(a2) && (_WORD)a2 == 1011 )
  {
    MEMORY[0x49FA18](Buffer, "%s\r%s", (const char *)(this + 1044), a3);
    sub_47CBF0(1011, *(_DWORD *)(this + 84), 0, Buffer);
    memset((void *)(this + 1044), 0, 0x104u);
  }
  sub_47B960(90);
  return 0;
}
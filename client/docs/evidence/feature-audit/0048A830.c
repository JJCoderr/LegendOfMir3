int __thiscall sub_48A830(char *this)
{
  void (__stdcall *v2)(char *, char *, char *, _BYTE *); // esi
  _BYTE v4[256]; // [esp+58h] [ebp-100h] BYREF

  dword_4AF110(256, v4);
  dword_4AF0A0(v4, MEMORY[0x4B6FFC]);
  v2 = (void (__stdcall *)(char *, char *, char *, _BYTE *))dword_4AF0C4;
  dword_4AF0C4(MEMORY[0x4B6FF0], MEMORY[0x4B6F7C], this + 1824, v4);
  v2(MEMORY[0x4B6FF0], aBasedirectory, this + 1952, v4);
  v2(MEMORY[0x4B6FF0], MEMORY[0x4B6FE4], a21, v4);
  v2(MEMORY[0x4B6FF0], MEMORY[0x4B6FD0], aAnonymous, v4);
  v2(MEMORY[0x4B6FF0], MEMORY[0x4B6FB8], MEMORY[0x4B6FC0], v4);
  return ((int (__stdcall *)(char *, char *, char *, _BYTE *))v2)(
           MEMORY[0x4B6FF0],
           MEMORY[0x4B6FA0],
           MEMORY[0x4B6FAC],
           v4);
}
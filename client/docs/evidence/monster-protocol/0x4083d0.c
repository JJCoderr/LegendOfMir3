unsigned int __thiscall sub_4083D0(int this, int a2)
{
  unsigned int result; // eax
  char Str[1024]; // [esp+8h] [ebp-400h] BYREF

  Str[sub_47D780((const char *)(a2 + 12), (int)Str, 1024)] = 0;
  sub_408450((_DWORD *)this, Str);
  *(_DWORD *)(this + 400428) = sub_401590(*(_BYTE *)(a2 + 6));
  result = strlen((const char *)(this + 400464)) + 1;
  qmemcpy((void *)(this + 403145), (const void *)(this + 400464), result);
  return result;
}
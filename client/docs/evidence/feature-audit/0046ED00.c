int __thiscall sub_46ED00(char *this, int a2, int a3)
{
  char *v3; // esi
  int v4; // edi
  int result; // eax

  v3 = this + 84;
  v4 = 4;
  do
  {
    result = (*(int (__thiscall **)(char *, int, int))(*(_DWORD *)v3 + 8))(v3, a2, a3);
    v3 += 180;
    --v4;
  }
  while ( v4 );
  return result;
}
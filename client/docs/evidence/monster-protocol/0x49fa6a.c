int __cdecl MEMORY[0x49FA6A](unsigned __int8 *a1)
{
  int v3; // esi
  unsigned __int8 *v4; // edi
  int v5; // ebp
  int v6; // ebx
  int result; // eax

  while ( MEMORY[0x4B88EC] <= 1 ? off_4B86E0[0][*a1] & 8 : MEMORY[0x4A2C2B](*a1, 8) )
    ++a1;
  v3 = *a1;
  v4 = a1 + 1;
  v5 = v3;
  if ( v3 == 45 || v3 == 43 )
    v3 = *v4++;
  v6 = 0;
  while ( MEMORY[0x4B88EC] <= 1 ? off_4B86E0[0][v3] & 4 : MEMORY[0x4A2C2B](v3, 4) )
  {
    v6 = v3 + 10 * v6 - 48;
    v3 = *v4++;
  }
  result = v6;
  if ( v5 == 45 )
    return -v6;
  return result;
}
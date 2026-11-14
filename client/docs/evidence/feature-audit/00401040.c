char sub_401040()
{
  int v0; // edi
  int v1; // ebp
  int v2; // edx
  int i; // ebx
  _BYTE *v4; // eax
  int v5; // ecx
  int v7; // edx
  const char *v8; // eax
  const char *v9; // edi
  int v10; // [esp-8h] [ebp-418h]
  int v11; // [esp-4h] [ebp-414h]
  int v12; // [esp+0h] [ebp-410h]
  int v13; // [esp+4h] [ebp-40Ch]
  char Buffer[1024]; // [esp+10h] [ebp-400h] BYREF

  v0 = dword_96991C;
  v1 = 1;
  if ( dword_96991C > 1 )
  {
    v2 = dword_969920;
    for ( i = 2; ; ++i )
    {
      v4 = *(_BYTE **)(v2 + 4 * v1);
      if ( *v4 == 45 || *v4 == 47 )
      {
        switch ( v4[1] )
        {
          case 'C':
          case 'c':
            byte_4B9799 = 1;
            break;
          case 'M':
          case 'm':
            byte_4B979A = 0;
            break;
          case 'P':
          case 'p':
            if ( i >= v0 )
              break;
            v5 = *(_DWORD *)(v2 + 4 * v1++ + 4);
            ++i;
            dword_4B97B0 = MEMORY[0x49FAF5](v5);
            goto LABEL_11;
          case 'R':
          case 'r':
            if ( i >= v0 )
              break;
            v7 = *(_DWORD *)(v2 + 4 * v1++ + 4);
            ++i;
            dword_4B979C = MEMORY[0x49FAF5](v7);
            if ( (unsigned __int8)sub_401200(800, 600, 16, dword_4B979C, v11, v12, v13) )
              goto LABEL_11;
            v10 = dword_4B979C;
            v8 = (const char *)sub_480550(10000);
            MEMORY[0x49FA18](Buffer, v8, v10, &byte_4B9680);
            if ( dword_4AF284(0, Buffer, MEMORY[0x4B5180], 68) == 7 )
              return 0;
            dword_4B979C = 0;
LABEL_11:
            v2 = dword_969920;
LABEL_12:
            v0 = dword_96991C;
            break;
          case 'S':
          case 's':
            if ( i >= v0 )
              break;
            v9 = *(const char **)(v2 + 4 * v1++ + 4);
            ++i;
            strcpy(&byte_4B97A0, v9);
            goto LABEL_12;
          case 'U':
          case 'u':
            byte_4B9798 = 0;
            break;
          default:
            break;
        }
      }
      if ( ++v1 >= v0 )
        return 1;
    }
  }
  return 1;
}
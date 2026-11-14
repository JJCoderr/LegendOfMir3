int __userpurge sub_4610D0@<eax>(int a1@<ecx>, int a2@<ebp>, int String, int a4)
{
  int v6; // eax
  int v7; // eax
  int v8; // esi
  int v9; // ebp
  int v10; // esi
  int v11; // edi
  _BYTE *v12; // eax
  const char *v13; // eax
  int v14; // eax
  int v15; // esi
  const char *v16; // eax
  int v17; // edx
  int v18; // [esp+4h] [ebp-20h]
  int v19; // [esp+4h] [ebp-20h]
  int v20; // [esp+8h] [ebp-1Ch]
  int v21; // [esp+Ch] [ebp-18h]
  int v22; // [esp+10h] [ebp-14h]
  int v23; // [esp+14h] [ebp-10h]
  int v24; // [esp+18h] [ebp-Ch] BYREF
  int v25; // [esp+1Ch] [ebp-8h] BYREF

  if ( (*(int (__thiscall **)(int, int, int, int))(*(_DWORD *)(a1 + 88) + 16))(a1 + 88, String, a4, 1)
    && *(_DWORD *)(a1 + 628) )
  {
    return 1;
  }
  if ( *(int *)(a1 + 960) > 0
    && *(_BYTE *)(a1 + 1444) == 1
    && (*(int (__thiscall **)(int, int, int, int))(*(_DWORD *)(a1 + 448) + 16))(a1 + 448, String, a4, 1) )
  {
    v6 = *(_DWORD *)(a1 + 960) - 1;
    *(_DWORD *)(a1 + 960) = v6;
    if ( v6 < 0 )
      *(_DWORD *)(a1 + 960) = 0;
LABEL_9:
    sub_461C20(a1);
    return 0;
  }
  if ( *(_DWORD *)(a1 + 960) < *(_DWORD *)(a1 + 964)
    && *(_BYTE *)(a1 + 1444) == 1
    && (*(int (__thiscall **)(int, int, int, int))(*(_DWORD *)(a1 + 268) + 16))(a1 + 268, String, a4, 1) )
  {
    v7 = *(_DWORD *)(a1 + 964);
    v8 = *(_DWORD *)(a1 + 960) + 1;
    *(_DWORD *)(a1 + 960) = v8;
    if ( v8 > v7 )
      *(_DWORD *)(a1 + 960) = v7;
    goto LABEL_9;
  }
  if ( !sub_420360((_DWORD *)(a1 + 968), String, a4) )
  {
    v24 = dword_903AD4;
    if ( dword_903AD4 )
    {
      v20 = a2;
      do
      {
        v9 = *(_DWORD *)(v24 + 4);
        if ( *(_BYTE *)v9 != 2 )
          goto LABEL_51;
        *(_BYTE *)(v9 + 92) = 0;
        v10 = v9 + 12;
        v11 = 5;
        do
        {
          if ( dword_4AF2AC(v10, String, a4) )
            *(_BYTE *)(v9 + 92) = 1;
          v10 += 16;
          --v11;
        }
        while ( v11 );
        if ( *(_BYTE *)(v9 + 92) != 1
          || (unsigned int)(dword_4AF2FC(v20, v21, v22, v23, v24) - *(_DWORD *)(a1 + 1304)) <= 0x3E8 )
        {
          goto LABEL_51;
        }
        v12 = *(_BYTE **)(v9 + 8);
        if ( *v12 != 64 )
          goto LABEL_49;
        if ( v12[1] != 64 || v12[2] != 42 )
        {
          if ( v12[1] == 64 )
          {
            memset((void *)(a1 + 1044), 0, 0x104u);
            strcpy((char *)(a1 + 1044), *(const char **)(v9 + 8));
            if ( !strcmp(*(const char **)(v9 + 8), aBuildguildnow) )
              v16 = (const char *)sub_480550(2000);
            else
              v16 = *(const char **)(v9 + 4);
            sub_420540((int)dword_82B050, (int)dword_5A041C, 9, 1, v16, 1, -1, -1, 1011, 0, 0, v20, v21);
            goto LABEL_50;
          }
LABEL_49:
          sub_47CBF0(1011, *(_DWORD *)(a1 + 84), 0, *(_DWORD *)(v9 + 8));
          goto LABEL_50;
        }
        memset((void *)(a1 + 1044), 0, 0x104u);
        strcpy((char *)(a1 + 1044), *(const char **)(v9 + 8));
        if ( !strcmp(*(const char **)(v9 + 8), aBuildguildnow) )
        {
          v18 = 0;
          v13 = (const char *)sub_480550(2000);
        }
        else
        {
          if ( MEMORY[0x4A1220](*(const char **)(v9 + 8), SubStr)
            || MEMORY[0x4A1220](*(const char **)(v9 + 8), MEMORY[0x4B5C74])
            || MEMORY[0x4A1220](*(const char **)(v9 + 8), aChangeStgpassw)
            || MEMORY[0x4A1220](*(const char **)(v9 + 8), aConfirmPasswd) )
          {
            v14 = *(_DWORD *)(v9 + 8);
            v15 = 3;
            if ( *(_BYTE *)(v14 + 3) == 95 )
              goto LABEL_43;
            do
            {
              if ( v15 >= 11 )
                break;
              MEMORY[0x49FA18]((char *)&v24 + v15 + 1, "%c", *(char *)(v14 + v15));
              v14 = *(_DWORD *)(v9 + 8);
              ++v15;
            }
            while ( *(_BYTE *)(v14 + v15) != 95 );
            if ( v15 < 10 )
            {
LABEL_43:
              *((_BYTE *)&v24 + v15 + 1) = 0;
              v19 = MEMORY[0x49FAF5](&v25);
              sub_420540(
                (int)dword_82B050,
                (int)dword_5A041C,
                9,
                1,
                *(const char **)(v9 + 4),
                1,
                -1,
                -1,
                1011,
                1,
                v19,
                v20,
                v21);
              goto LABEL_50;
            }
            v18 = 10;
          }
          else
          {
            v18 = 0;
          }
          v13 = *(const char **)(v9 + 4);
        }
        sub_420540((int)dword_82B050, (int)dword_5A041C, 9, 1, v13, 1, -1, -1, 1011, 1, v18, v20, v21);
LABEL_50:
        *(_DWORD *)(a1 + 1304) = dword_4AF2FC(v20, v21, v22, v23, v24);
LABEL_51:
        v17 = v24;
        *(_BYTE *)(v9 + 92) = 0;
        v24 = *(_DWORD *)(v17 + 12);
      }
      while ( v24 );
    }
  }
  return 0;
}
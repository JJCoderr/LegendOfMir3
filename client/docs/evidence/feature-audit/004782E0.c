void __stdcall sub_4782E0(int a1, int a2, unsigned int a3, int a4)
{
  int v4; // esi
  int v5; // eax
  char *v6; // eax
  int v7; // eax
  const char *v8; // eax
  char v9[260]; // [esp+4h] [ebp-104h] BYREF

  if ( a3 < 0xC )
  {
    v4 = 0;
    switch ( a3 )
    {
      case 0u:
        if ( (unsigned __int8)dword_824694 >= a4 )
          goto LABEL_33;
        break;
      case 1u:
        if ( HIBYTE(word_824699) >= a4 )
          goto LABEL_33;
        break;
      case 2u:
        v4 = 1;
        v5 = 8537783;
        do
        {
          if ( *(unsigned __int8 *)(v5 + 1) < a4 )
            goto LABEL_19;
          v5 += 2;
        }
        while ( v5 < (int)&byte_8246BF );
        break;
      case 3u:
        v4 = 1;
        v6 = &byte_8246BF;
        do
        {
          if ( (unsigned __int8)v6[1] < a4 )
            goto LABEL_19;
          v6 += 2;
        }
        while ( (int)v6 < (int)&byte_8246C5 );
        break;
      case 4u:
        v4 = 1;
        v7 = 8537783;
        while ( *(unsigned __int8 *)(v7 + 1) >= a4 )
        {
          v7 += 2;
          if ( v7 >= (int)&byte_8246C5 )
            goto LABEL_34;
        }
LABEL_19:
        v4 = 0;
        break;
      case 5u:
        if ( (unsigned __int8)byte_8246B8 >= a4 )
          goto LABEL_33;
        break;
      case 6u:
        if ( (unsigned __int8)byte_8246BA >= a4 )
          goto LABEL_33;
        break;
      case 7u:
        if ( (unsigned __int8)byte_8246BC >= a4 )
          goto LABEL_33;
        break;
      case 8u:
        if ( (unsigned __int8)byte_8246BE >= a4 )
          goto LABEL_33;
        break;
      case 9u:
        if ( (unsigned __int8)byte_8246C0 >= a4 )
          goto LABEL_33;
        break;
      case 0xAu:
        if ( (unsigned __int8)byte_8246C2 >= a4 )
          goto LABEL_33;
        break;
      case 0xBu:
        if ( (unsigned __int8)byte_8246C4 >= a4 )
LABEL_33:
          v4 = 1;
        break;
      default:
        break;
    }
LABEL_34:
    v8 = (const char *)sub_480550(a3 + 5140);
    MEMORY[0x49FA18](v9, "%s %d", v8, a4);
    if ( v4 )
      sub_48BCAF(&dword_8FCC50, 0, a1, a2 + 20, 16773360, 0, v9, 0);
    else
      sub_48BCAF(&dword_8FCC50, 0, a1, a2 + 20, 1974015, 0, v9, 0);
  }
}
unsigned int __usercall sub_42EC70@<eax>(int a1@<ecx>, int a2@<ebx>, int a3@<edi>)
{
  unsigned int result; // eax
  unsigned int *v5; // ebp
  int v6; // eax
  int v7; // edi
  __int16 v8; // ax
  __int16 v9; // ax
  int v10; // ecx
  __int16 v11; // ax
  int v12; // edx
  int v13; // eax
  int v14; // ebx
  int v15; // [esp-8h] [ebp-14h]
  int v16; // [esp-4h] [ebp-10h]
  int v17; // [esp+0h] [ebp-Ch]
  int v18; // [esp+4h] [ebp-8h]
  unsigned int v19; // [esp+8h] [ebp-4h]

  v19 = 0;
  result = *(_DWORD *)(a1 + 3622540);
  v5 = (unsigned int *)(a1 + 3622540);
  if ( result )
  {
    v16 = a2;
    v15 = a3;
    do
    {
      v6 = sub_481AF0(v5);
      v7 = v6;
      if ( v6 )
      {
        v8 = *(_WORD *)(v6 + 4);
        switch ( v8 )
        {
          case 51:
          case 634:
            sub_42EE00(a1, v7);
            break;
          case 50:
            sub_42F190((void *)v7);
            break;
          case 52:
            sub_42F5A0((void *)v7);
            break;
          case 752:
            sub_42F650((void *)v7);
            break;
          case 1321:
            goto LABEL_13;
          default:
            if ( *(_DWORD *)v7 == **(_DWORD **)(a1 + 3580384) )
            {
              if ( v8 == 31 )
              {
                if ( (unsigned int)(dword_4AF2FC(v15, v16, v17, v18, v19) - *(_DWORD *)(a1 + 3583624)) < 0x1F4 )
                {
                  v9 = *(_WORD *)(v7 + 6);
                  v10 = *(_DWORD *)(a1 + 3580384);
                  *(_WORD *)(a1 + 3583527) = v9;
                  *(_WORD *)(v10 + 12) = v9;
                  v11 = *(_WORD *)(v7 + 8);
                  v12 = *(_DWORD *)(a1 + 3580384);
                  *(_WORD *)(a1 + 3583531) = v11;
                  *(_WORD *)(v12 + 10) = v11;
                  if ( *(_WORD *)(*(_DWORD *)(a1 + 3580384) + 12) )
                    goto LABEL_18;
                }
                sub_40D7F0(a1 + 3179944);
              }
LABEL_13:
              MEMORY[0x481A30]((FILE *)v7);
              break;
            }
            v13 = *(_DWORD *)(a1 + 921960);
            if ( v13 )
            {
              while ( 1 )
              {
                v14 = *(_DWORD *)(v13 + 4);
                if ( v14 )
                {
                  if ( **(_DWORD **)(v14 + 400440) == *(_DWORD *)v7 )
                    break;
                }
                v13 = *(_DWORD *)(v13 + 12);
                if ( !v13 )
                  goto LABEL_25;
              }
              sub_40D7F0(*(_DWORD *)(v13 + 4));
              MEMORY[0x481A30]((FILE *)v7);
              break;
            }
LABEL_25:
            if ( !sub_42E9A0((_DWORD *)a1, (FILE *)v7) )
LABEL_18:
              MEMORY[0x4939F6]((void *)v7);
            break;
        }
      }
      result = ++v19;
    }
    while ( v19 < *v5 );
  }
  return result;
}
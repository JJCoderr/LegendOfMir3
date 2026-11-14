int __userpurge sub_420540@<eax>(
        int a1@<ecx>,
        int a2,
        int a3,
        int a4,
        const char *a5,
        int a6,
        int a7,
        int a8,
        __int16 a9,
        int a10,
        int a11,
        int a12,
        int a13)
{
  int result; // eax
  int v15; // eax
  int v16; // ecx
  __int16 *v17; // esi
  int *v18; // esi
  int (__stdcall *v19)(int, int, int); // edi
  int v20; // esi
  int v21; // eax
  int v22; // [esp+38h] [ebp-8h]

  sub_4204B0(a1);
  *(_DWORD *)(a1 + 1116) = a2;
  if ( a4 == 2 )
    *(_DWORD *)(a1 + 4) = 1248;
  else
    *(_DWORD *)(a1 + 4) = 1240;
  result = sub_491C70(*(_DWORD *)(a1 + 4));
  if ( result )
  {
    *(_DWORD *)a1 = a3;
    strcpy((char *)(a1 + 44), a5);
    v15 = a8;
    v16 = a7;
    if ( a7 == -1 && a8 == -1 )
    {
      v17 = *(__int16 **)(*(_DWORD *)(a1 + 1116) + 60);
      v16 = 400 - *v17 / 2;
      v15 = 246 - v17[1] / 2;
    }
    v18 = (int *)(a1 + 8);
    v19 = (int (__stdcall *)(int, int, int))dword_4AF2A8;
    dword_4AF2A8(
      a1 + 8,
      v16,
      v15,
      v16 + **(__int16 **)(*(_DWORD *)(a1 + 1116) + 60),
      v15 + *(__int16 *)(*(_DWORD *)(*(_DWORD *)(a1 + 1116) + 60) + 2));
    if ( a4 )
    {
      if ( a4 == 1 )
      {
        *(_DWORD *)(a1 + 748) = 0;
        *(_DWORD *)(a1 + 564) = 1;
        *(_DWORD *)(a1 + 932) = 1;
        *(_BYTE *)(a1 + 1122) = 0;
      }
      else if ( a4 == 2 )
      {
        *(_DWORD *)(a1 + 564) = 0;
        *(_DWORD *)(a1 + 748) = 0;
        *(_DWORD *)(a1 + 932) = 0;
      }
    }
    else
    {
      *(_DWORD *)(a1 + 564) = 0;
      *(_DWORD *)(a1 + 748) = 1;
      *(_DWORD *)(a1 + 932) = 0;
      *(_BYTE *)(a1 + 1122) = 1;
    }
    sub_41FA00(a1 + 568, *(_DWORD *)(a1 + 1116), 1241, 1242, *v18 + 41, *(_DWORD *)(a1 + 12) + 188, 0, 1, 1241, 0);
    sub_41FA00(a1 + 752, *(_DWORD *)(a1 + 1116), 1243, 1244, *v18 + 137, *(_DWORD *)(a1 + 12) + 188, 0, 1, -1, 0);
    sub_41FA00(a1 + 936, *(_DWORD *)(a1 + 1116), 1245, 1246, *v18 + 234, *(_DWORD *)(a1 + 12) + 188, 0, 1, 1245, 0);
    if ( a8 )
    {
      ((void (__stdcall *)(int, int, int, int, int, int))v19)(
        a1 + 24,
        *v18 + 43,
        *(_DWORD *)(a1 + 12) + 84,
        *v18 + 321,
        *(_DWORD *)(a1 + 12) + 159,
        v22);
      dword_4AF234(dword_8FB92C, dword_8FCD9C + *v18 + 24, *(_DWORD *)(a1 + 12) + dword_8FCDA0 + 228, 309, 15, 1);
      if ( a13 )
        dword_4AF2B4(dword_8FB92C, 204, 42, 0);
      else
        dword_4AF2B4(dword_8FB92C, 204, 0, 0);
      dword_4AF26C(dword_8FB92C, dword_902E34);
      dword_4AF2B0(dword_8FB92C);
      dword_4AF2A0(dword_8FB92C, 5);
      if ( a11 )
        result = sub_47B960(a11);
      else
        result = sub_47B960(90);
      dword_8FB938 = 0;
    }
    else
    {
      v20 = *v18;
      v21 = *(_DWORD *)(a1 + 12);
      if ( a6 == 2 )
        result = v19(a1 + 24, v20 + 27, v21 + 35);
      else
        result = v19(a1 + 24, v20 + 23, v21 + 84);
    }
    *(_DWORD *)(a1 + 40) = 1;
    *(_WORD *)(a1 + 1120) = a9;
  }
  return result;
}
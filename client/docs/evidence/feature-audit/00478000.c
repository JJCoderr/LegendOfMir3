char __usercall sub_478000@<al>(int a1@<ecx>, int a2@<ebp>)
{
  char v3; // al
  int v4; // ecx
  char v5; // al
  char v6; // al
  char result; // al

  (*(void (__thiscall **)(int))(*(_DWORD *)a1 + 12))(a1);
  v3 = *(_BYTE *)(a1 + 1524);
  if ( v3 && v3 != 4 && v3 != 1 && v3 != 3 )
  {
    sub_41FD20((_DWORD *)(a1 + 444), *(_DWORD *)(a1 + 24) + 449, *(_DWORD *)(a1 + 28) + 150);
    sub_41FD20((_DWORD *)(a1 + 624), *(_DWORD *)(a1 + 24) + 358, *(_DWORD *)(a1 + 28) + 154);
    sub_41FD20((_DWORD *)(a1 + 804), *(_DWORD *)(a1 + 24) + 316, *(_DWORD *)(a1 + 28) + 154);
    v4 = *(_DWORD *)(a1 + 28) + 154;
LABEL_8:
    sub_41FD20((_DWORD *)(a1 + 984), *(_DWORD *)(a1 + 24) + 406, v4);
    goto LABEL_11;
  }
  sub_41FD20((_DWORD *)(a1 + 84), *(_DWORD *)(a1 + 24) + 258, *(_DWORD *)(a1 + 28) + 183);
  sub_41FD20((_DWORD *)(a1 + 264), *(_DWORD *)(a1 + 24) + 106, *(_DWORD *)(a1 + 28) + 186);
  v5 = *(_BYTE *)(a1 + 1524);
  if ( v5 == 1 )
  {
    sub_41FD20((_DWORD *)(a1 + 444), *(_DWORD *)(a1 + 24) + 449, *(_DWORD *)(a1 + 28) + 159);
    sub_41FD20((_DWORD *)(a1 + 624), *(_DWORD *)(a1 + 24) + 358, *(_DWORD *)(a1 + 28) + 163);
    sub_41FD20((_DWORD *)(a1 + 804), *(_DWORD *)(a1 + 24) + 316, *(_DWORD *)(a1 + 28) + 163);
    v4 = *(_DWORD *)(a1 + 28) + 163;
    goto LABEL_8;
  }
  if ( v5 == 4 )
  {
    sub_41FD20((_DWORD *)(a1 + 1164), *(_DWORD *)(a1 + 24) + 475, *(_DWORD *)(a1 + 28) + 56);
    sub_41FD20((_DWORD *)(a1 + 1344), *(_DWORD *)(a1 + 24) + 367, *(_DWORD *)(a1 + 28) + 59);
  }
LABEL_11:
  v6 = *(_BYTE *)(a1 + 1524);
  if ( !v6 || v6 == 4 || v6 == 1 || v6 == 3 )
  {
    sub_477160(a1, a2);
    (*(void (__thiscall **)(int))(*(_DWORD *)(a1 + 84) + 4))(a1 + 84);
    (*(void (__thiscall **)(int))(*(_DWORD *)(a1 + 264) + 4))(a1 + 264);
  }
  result = *(_BYTE *)(a1 + 1524);
  if ( result == 1 || result == 2 )
  {
    sub_477780(a1);
    (*(void (__thiscall **)(int))(*(_DWORD *)(a1 + 444) + 4))(a1 + 444);
    (*(void (__thiscall **)(int))(*(_DWORD *)(a1 + 624) + 4))(a1 + 624);
    (*(void (__thiscall **)(int))(*(_DWORD *)(a1 + 804) + 4))(a1 + 804);
    result = (*(int (__thiscall **)(int))(*(_DWORD *)(a1 + 984) + 4))(a1 + 984);
  }
  if ( *(_BYTE *)(a1 + 1524) == 4 )
  {
    sub_477C80(a1);
    (*(void (__thiscall **)(int))(*(_DWORD *)(a1 + 84) + 4))(a1 + 84);
    (*(void (__thiscall **)(int))(*(_DWORD *)(a1 + 264) + 4))(a1 + 264);
    (*(void (__thiscall **)(int))(*(_DWORD *)(a1 + 1164) + 4))(a1 + 1164);
    return (*(int (__thiscall **)(int))(*(_DWORD *)(a1 + 1344) + 4))(a1 + 1344);
  }
  return result;
}
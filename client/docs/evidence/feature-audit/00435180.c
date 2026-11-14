int __thiscall sub_435180(#22 *this)
{
  int v2; // eax
  void (__stdcall *v3)(char *, int, int, int, int); // ebp
  int v4; // edx
  char *v5; // edi
  int v6; // edi
  int i; // eax
  int v8; // ecx
  int v9; // eax
  int j; // eax
  int v11; // ecx
  int v12; // eax
  _DWORD *v13; // edx
  int result; // eax
  _DWORD *v15; // esi
  _DWORD *v16; // [esp+3Ch] [ebp-18h]
  int v17; // [esp+40h] [ebp-14h]
  int v18; // [esp+44h] [ebp-10h]
  int v19; // [esp+48h] [ebp-Ch]
  int v20; // [esp+4Ch] [ebp-8h]
  int v21; // [esp+50h] [ebp-4h]
  int v22; // [esp+50h] [ebp-4h]

  v2 = ((int (__cdecl *)())dword_4AF2FC)();
  v3 = (void (__stdcall *)(char *, int, int, int, int))dword_4AF2A8;
  *((_DWORD *)this + 6829) = v2;
  *((_DWORD *)this + 6828) = 0;
  *((_DWORD *)this + 895) = 0;
  *((_DWORD *)this + 896) = 0;
  *((_WORD *)this + 1852) = 0;
  *((_DWORD *)this + 6827) = 0;
  *((_DWORD *)this + 925) = 0;
  *((_BYTE *)this + 3706) = 0;
  *((_DWORD *)this + 7317) = 0;
  v3((char *)this + 3164, 0, 0, 0, 0);
  v3((char *)this + 3548, 0, 0, 0, 0);
  v3((char *)this + 3564, 0, 0, 0, 0);
  v3((char *)this + 3636, 0, 0, 0, 0);
  memset((char *)this + 32, 0, 0xC28u);
  MEMORY[0x41C0B0](this);
  v4 = *((_DWORD *)this + 43731);
  *((_DWORD *)this + 7) = 0;
  *((_DWORD *)this + 99829) = -1;
  (*(void (__thiscall **)(char *))(v4 + 4))((char *)this + 174924);
  (*(void (__thiscall **)(char *))(*((_DWORD *)this + 54193) + 4))((char *)this + 216772);
  (*(void (__thiscall **)(char *))(*((_DWORD *)this + 74535) + 4))((char *)this + 298140);
  (*(void (__thiscall **)(char *))(*((_DWORD *)this + 76054) + 4))((char *)this + 304216);
  (*(void (__thiscall **)(char *))(*((_DWORD *)this + 86114) + 4))((char *)this + 344456);
  (*(void (__thiscall **)(char *))(*((_DWORD *)this + 54702) + 4))((char *)this + 218808);
  (*(void (__thiscall **)(char *))(*((_DWORD *)this + 76307) + 4))((char *)this + 305228);
  (*(void (__thiscall **)(char *))(*((_DWORD *)this + 7360) + 4))((char *)this + 29440);
  (*(void (__thiscall **)(char *))(*((_DWORD *)this + 89422) + 4))((char *)this + 357688);
  (*(void (__thiscall **)(char *))(*((_DWORD *)this + 89539) + 4))((char *)this + 358156);
  memset((char *)this + 399056, 0, 0x104u);
  *((_DWORD *)this + 7315) = 0;
  *((_DWORD *)this + 7316) = 0;
  *((_DWORD *)this + 6830) = dword_4AF2FC(v17, v18, v19, v20, v21);
  v5 = (char *)this + 22476;
  v22 = 23;
  do
  {
    sub_41FAA0((int)v5);
    v5 += 180;
    --v22;
  }
  while ( v22 );
  *((_DWORD *)this + 6790) = 0;
  *((_DWORD *)this + 6791) = 0;
  *((_DWORD *)this + 6793) = 0;
  *((_DWORD *)this + 6792) = 0;
  *((_DWORD *)this + 6794) = 0;
  *((_DWORD *)this + 6795) = 0;
  *((_WORD *)this + 13592) = 0;
  *((_WORD *)this + 13593) = 0;
  *((_WORD *)this + 13594) = 0;
  v3((char *)this + 27192, 0, 0, 0, 0);
  v3((char *)this + 27208, 0, 0, 0, 0);
  *((_DWORD *)this + 6809) = 0;
  *((_DWORD *)this + 6810) = 0;
  *((_DWORD *)this + 6812) = 0;
  *((_DWORD *)this + 6811) = 0;
  *((_DWORD *)this + 6813) = 0;
  *((_DWORD *)this + 6814) = 0;
  *((_WORD *)this + 13630) = 0;
  *((_WORD *)this + 13631) = 0;
  *((_WORD *)this + 13632) = 0;
  v3((char *)this + 27268, 0, 0, 0, 0);
  v3((char *)this + 27284, 0, 0, 0, 0);
  if ( *((int *)this + 924) > 0 )
  {
    v6 = 0;
    do
    {
      v16 = (_DWORD *)*((_DWORD *)this + 920);
      *((_DWORD *)this + 920) = v16[1];
      MEMORY[0x4939F6](v16);
      ++v6;
    }
    while ( v6 < *((_DWORD *)this + 924) );
  }
  *((_DWORD *)this + 924) = 0;
  *((_DWORD *)this + 920) = 0;
  *((_DWORD *)this + 922) = 0;
  *((_DWORD *)this + 921) = 0;
  *((_DWORD *)this + 923) = 0;
  for ( i = *((_DWORD *)this + 7347); i; i = *((_DWORD *)this + 7347) )
  {
    MEMORY[0x4939F6](*(void **)(i + 4));
    v8 = *((_DWORD *)this + 7347);
    v9 = *(_DWORD *)(v8 + 12);
    *((_DWORD *)this + 7347) = v9;
    if ( v9 )
      *(_DWORD *)(v9 + 8) = 0;
    if ( v8 )
      (**(void (__thiscall ***)(int, int))v8)(v8, 1);
  }
  *((_DWORD *)this + 7347) = 0;
  *((_DWORD *)this + 7348) = 0;
  *((_DWORD *)this + 7351) = 0;
  *((_DWORD *)this + 7352) = 0;
  for ( j = *((_DWORD *)this + 7354); j; j = *((_DWORD *)this + 7354) )
  {
    MEMORY[0x4939F6](*(void **)(j + 4));
    v11 = *((_DWORD *)this + 7354);
    v12 = *(_DWORD *)(v11 + 12);
    *((_DWORD *)this + 7354) = v12;
    if ( v12 )
      *(_DWORD *)(v12 + 8) = 0;
    if ( v11 )
      (**(void (__thiscall ***)(int, int))v11)(v11, 1);
  }
  *((_DWORD *)this + 7354) = 0;
  *((_DWORD *)this + 7355) = 0;
  *((_DWORD *)this + 7358) = 0;
  *((_DWORD *)this + 7359) = 0;
  *((_DWORD *)this + 7318) = 0;
  *((_DWORD *)this + 7319) = 0;
  *((_DWORD *)this + 7321) = 0;
  *((_DWORD *)this + 7322) = 1;
  *((_DWORD *)this + 7336) = 1;
  *((_DWORD *)this + 7337) = 1;
  *((_DWORD *)this + 7338) = 0;
  *((_DWORD *)this + 7339) = 0;
  *((_DWORD *)this + 7340) = 0;
  *((_DWORD *)this + 7341) = 0;
  *((_DWORD *)this + 7342) = 0;
  *((_DWORD *)this + 7343) = 0;
  *((_DWORD *)this + 7344) = 0;
  *((_DWORD *)this + 7323) = 0;
  *((_DWORD *)this + 7345) = 1;
  v3((char *)this + 29312, 634, 570, 664, 590);
  v3((char *)this + 29328, 717, 570, 747, 590);
  v3((char *)this + 29296, 0, 0, 0, 0);
  memset((char *)this + 3708, 0, 0x60u);
  memset((char *)this + 3804, 0, 0x48F0u);
  *((_DWORD *)this + 795) = 0;
  *((_DWORD *)this + 796) = 0;
  *((_DWORD *)this + 797) = 0;
  *((_DWORD *)this + 798) = 0;
  *((_DWORD *)this + 799) = 0;
  *((_DWORD *)this + 800) = 0;
  *((_DWORD *)this + 801) = 0;
  *((_DWORD *)this + 802) = 0;
  v13 = (_DWORD *)((char *)this + 3212);
  result = 0;
  v15 = (_DWORD *)((char *)this + 3228);
  *v13 = 0;
  v13[1] = 0;
  v13[2] = 0;
  v13[3] = 0;
  *v15 = 0;
  v15[1] = 0;
  v15[2] = 0;
  v15[3] = 0;
  return result;
}
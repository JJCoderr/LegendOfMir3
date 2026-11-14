int __thiscall sub_4214A0(int this)
{
  _DWORD *v2; // ecx
  int i; // eax
  int v4; // ecx
  int v5; // eax
  _DWORD *v6; // eax

  *(_DWORD *)(this + 8) = 0;
  *(_DWORD *)(this + 16) = 0;
  *(_DWORD *)(this + 24) = 0;
  *(_DWORD *)(this + 28) = 0;
  *(_DWORD *)(this + 20) = 0;
  *(_DWORD *)(this + 12) = 0;
  *(_DWORD *)(this + 60) = 0;
  *(_DWORD *)(this + 32) = 0;
  *(_DWORD *)(this + 3583700) = 0;
  *(_DWORD *)(this + 36) = 0;
  *(_DWORD *)(this + 40) = 1;
  *(_DWORD *)(this + 44) = 0;
  *(_DWORD *)(this + 48) = 0;
  *(_DWORD *)(this + 52) = 0;
  *(_DWORD *)(this + 84) = 0;
  *(_DWORD *)(this + 64) = 0;
  *(_DWORD *)(this + 68) = 0;
  *(_DWORD *)(this + 72) = 0;
  *(_DWORD *)(this + 76) = 0;
  *(_DWORD *)(this + 80) = 1;
  *(_DWORD *)(this + 92) = 1;
  *(_DWORD *)(this + 88) = 1;
  *((_BYTE *)&loc_43846D + this + 1) = 0;
  *(_DWORD *)((char *)&loc_43846D + this + 3) = 0;
  *(_DWORD *)((char *)&loc_43846D + this + 7) = 0;
  *(_DWORD *)((char *)&loc_4384B5 + this + 7) = 0;
  *(_DWORD *)((char *)&loc_43843F + this + 1) = 0;
  *(_DWORD *)((char *)&loc_438442 + this + 2) = 0;
  *(_DWORD *)((char *)&loc_438450 + this) = 1065353216;
  *(_DWORD *)((char *)&loc_438451 + this + 3) = 0;
  *(_DWORD *)((char *)&loc_438445 + this + 3) = 0;
  *(_DWORD *)((char *)&loc_438449 + this + 3) = 0;
  *(_DWORD *)(this + 56) = 0;
  *(_DWORD *)((char *)&loc_4384B5 + this + 3) = 1;
  v2 = (_DWORD *)(this + 3583684);
  *(_DWORD *)((char *)&loc_4384C5 + this + 7) = 0;
  *(_DWORD *)((char *)&loc_4384CD + this + 3) = 1;
  *v2 = 0;
  v2[1] = 0;
  *(_DWORD *)(this + 3583692) = 0;
  *(_DWORD *)(this + 3583696) = 0;
  memset((void *)(this + 356), 0, 0xE1000u);
  *(_DWORD *)(this + 3583704) = 0;
  *(_BYTE *)(this + 3583708) = 1;
  *(_DWORD *)(this + 3622516) = 0;
  *(_DWORD *)(this + 3622524) = 0;
  *(_DWORD *)(this + 3622528) = 0;
  *(_DWORD *)(this + 3622532) = 0;
  *(_DWORD *)(this + 3622520) = 0;
  *(_DWORD *)(this + 3622536) = 0;
  *(_DWORD *)((char *)&loc_437BE4 + this) = 1132396544;
  *(_DWORD *)((char *)&loc_437BE4 + this + 4) = 1130758144;
  *(_DWORD *)((char *)&loc_437BE9 + this + 3) = 1127481344;
  *(_DWORD *)((char *)&loc_437BDE + this + 2) = 0;
  *(_DWORD *)((char *)&loc_437BD5 + this + 3) = 0;
  *(_DWORD *)((char *)&loc_437BDA + this + 2) = 0;
  *(_DWORD *)((char *)&loc_43827F + this + 1) = 0;
  *(_DWORD *)((char *)&loc_438281 + this + 3) = 0;
  *(_DWORD *)((char *)&loc_437BD0 + this + 4) = 0;
  *((_BYTE *)&loc_438437 + this + 1) = 0;
  *(_DWORD *)((char *)&loc_43843B + this + 1) = 0;
  *((_BYTE *)&loc_438468 + this) = 0;
  *((_BYTE *)&loc_438468 + this + 1) = 0;
  *(_WORD *)((char *)&loc_438468 + this + 2) = 0;
  *(_WORD *)((char *)&loc_438468 + this + 4) = 0;
  memset((char *)&loc_437BED + this + 3, 0, 0x320u);
  *(_DWORD *)((char *)&loc_43826F + this + 1) = 0;
  *(_DWORD *)((char *)&loc_438273 + this + 1) = 0;
  *(_DWORD *)((char *)&loc_43827B + this + 1) = 0;
  *(_DWORD *)((char *)&loc_438275 + this + 3) = 0;
  *(_DWORD *)((char *)&loc_4384AD + this + 7) = 0;
  *(_DWORD *)((char *)&loc_4384BD + this + 3) = 0;
  *(_DWORD *)((char *)&loc_4384BD + this + 7) = 0;
  *(_DWORD *)((char *)&loc_4384C5 + this + 3) = 0;
  dword_4AF2A8((char *)&loc_438480 + this, 0, 0, 300, 200);
  for ( i = *(_DWORD *)((char *)&loc_438493 + this + 1); i; i = *(_DWORD *)((char *)&loc_438493 + this + 1) )
  {
    MEMORY[0x4939F6](*(void **)(i + 4));
    v4 = *(_DWORD *)((char *)&loc_438493 + this + 1);
    v5 = *(_DWORD *)(v4 + 12);
    *(_DWORD *)((char *)&loc_438493 + this + 1) = v5;
    if ( v5 )
      *(_DWORD *)(v5 + 8) = 0;
    if ( v4 )
      (**(void (__thiscall ***)(int, int))v4)(v4, 1);
  }
  *(_DWORD *)((char *)&loc_438493 + this + 1) = 0;
  *(_DWORD *)((char *)&loc_438497 + this + 1) = 0;
  *(_DWORD *)((char *)&loc_43849D + this + 7) = 0;
  v6 = (_DWORD *)((char *)&loc_438287 + this + 1);
  *(_DWORD *)((char *)&loc_4384A5 + this + 7) = 0;
  *(_DWORD *)((char *)&loc_4384A5 + this + 3) = 0;
  *(_DWORD *)((char *)&loc_43847B + this + 1) = 0;
  *(_DWORD *)((char *)&loc_438475 + this + 3) = -1;
  *(_DWORD *)((char *)&loc_43845C + this) = 0;
  *(_DWORD *)((char *)&loc_438455 + this + 3) = 0;
  *(_DWORD *)((char *)&loc_438460 + this) = 0;
  *(_DWORD *)((char *)&loc_438462 + this + 2) = 0;
  *(_DWORD *)((char *)&loc_438297 + this + 1) = 0;
  *(_DWORD *)((char *)&loc_438299 + this + 3) = 0;
  *(_DWORD *)((char *)&loc_43829F + this + 1) = 0;
  *(_DWORD *)((char *)&loc_4382A4 + this) = 0;
  *v6 = 0;
  v6[1] = 0;
  v6[2] = 0;
  v6[3] = 0;
  memset((char *)&loc_4383A3 + this + 5, 0, 0x90u);
  memset((void *)(this + 96), 0, 0x104u);
  sub_47DA90((char *)(this + 922100));
  sub_435180((#22 *)(this + 2780620));
  (*(void (__thiscall **)(int))(*(_DWORD *)(this + 3179944) + 4))(this + 3179944);
  sub_451870((_DWORD *)(this + 1010488));
  sub_448A80((_DWORD *)(this + 3583712), 0xFFFFFF);
  nullsub_5();
  (*(void (__thiscall **)(int))(*(_DWORD *)(this + 3610020) + 4))(this + 3610020);
  (*(void (__thiscall **)(int))(*(_DWORD *)(this + 3610208) + 4))(this + 3610208);
  (*(void (__thiscall **)(int))(*(_DWORD *)(this + 3610396) + 4))(this + 3610396);
  (*(void (__thiscall **)(int))(*(_DWORD *)(this + 3609436) + 4))(this + 3609436);
  (*(void (__thiscall **)(int))(*(_DWORD *)(this + 3609632) + 4))(this + 3609632);
  (*(void (__thiscall **)(int))(*(_DWORD *)(this + 3609820) + 4))(this + 3609820);
  return sub_46B520(this + 3611716);
}
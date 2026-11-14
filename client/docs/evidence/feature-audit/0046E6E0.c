int __thiscall sub_46E6E0(int *this)
{
  void (__stdcall *v2)(int *, int, int, int, int); // edi
  int *v3; // ebp
  _DWORD *v4; // eax
  _DWORD *v5; // ebx
  int v6; // eax
  int v7; // eax
  int v8; // ebp
  int v9; // ebx
  _DWORD *v10; // eax
  int v11; // ebp
  int v12; // ebp
  int v13; // ebx
  _DWORD *v14; // ebx
  int v15; // esi
  int result; // eax
  int v17; // [esp+98h] [ebp-124h]
  int v18; // [esp+9Ch] [ebp-120h]
  int v19; // [esp+A0h] [ebp-11Ch]
  int v20; // [esp+A8h] [ebp-114h] BYREF
  int v21; // [esp+ACh] [ebp-110h]
  int v22; // [esp+B0h] [ebp-10Ch]
  int v23; // [esp+B4h] [ebp-108h]
  char v24[84]; // [esp+B8h] [ebp-104h] BYREF

  v2 = (void (__stdcall *)(int *, int, int, int, int))dword_4AF2A8;
  v3 = this + 205;
  dword_4AF2A8(this + 205, this[6] + 6, this[7] + 65, this[6] + 186, this[7] + 250);
  if ( this[12] )
  {
    v4 = sub_4280E0(dword_4B9878, this[222]);
    v5 = v4;
    if ( v4 )
    {
      sub_45E430(v4[1], v4[2]);
      v6 = ((int (__cdecl *)())dword_4AF2FC)();
      sub_45E660(dword_767578, v6);
      v7 = this[223];
      if ( v7 == 1 )
      {
        v8 = MEMORY[0x49FE1C]((double)(int)v5[1] * flt_4AFA70 + (double)*v3 - (double)dword_767850 * flt_4AFA6C - flt_4AF6E4);
        v9 = MEMORY[0x49FE1C]((double)(int)v5[2] * flt_4AFA68 + (double)this[206] - (double)dword_767854 * flt_4AFA64 - flt_4AF6E4);
        v2(this + 213, v8 - 55, v9 - 34, v8 + 55, v9 + 34);
        sub_48C359(&dword_8FCC50, this + 213, 0, (int)dword_6464C8, 1);
        v19 = v9 + 50;
        v18 = v8 + 80;
        v10 = this + 209;
        v17 = v9 - 50;
        v11 = v8 - 80;
LABEL_7:
        ((void (__stdcall *)(_DWORD *, int))v2)(v10, v11);
        sub_48C359(&dword_8FCC50, this + 209, 0, 3289850, 1);
        goto LABEL_8;
      }
      if ( v7 > 0 )
      {
        v12 = MEMORY[0x49FE1C]((double)(int)v5[1] * flt_4AFA70 + (double)*v3 - (double)dword_767850 * flt_4AFA6C - flt_4AF6E4);
        v13 = MEMORY[0x49FE1C]((double)(int)v5[2] * flt_4AFA68 + (double)this[206] - (double)dword_767854 * flt_4AFA64 - flt_4AF6E4);
        v2(this + 213, v12 - 28, v13 - 16, v12 + 28, v13 + 16);
        sub_48C359(&dword_8FCC50, this + 213, 0, (int)dword_6464C8, 1);
        v19 = v13 + 29;
        v18 = v12 + 50;
        v10 = this + 209;
        v17 = v13 - 29;
        v11 = v12 - 50;
        goto LABEL_7;
      }
    }
  }
LABEL_8:
  (*(void (__thiscall **)(int *, int, int, int))(*this + 12))(this, v17, v18, v19);
  v14 = this + 21;
  sub_41FD20(this + 21, this[6] + 153, this[7] + 241);
  sub_41FD20(this + 66, this[6] + 17, this[7] + 231);
  sub_41FD20(this + 111, this[6] + 67, this[7] + 231);
  sub_41FD20(this + 156, this[6] + 117, this[7] + 231);
  v2(&v20, this[6] + 14, this[7] + 22, this[6] + 90, this[7] + 37);
  MEMORY[0x49FA18](v24, "%d", this[201]);
  sub_48BD63(&dword_8FCC50, 0, v20, v21, v22, v23, (int)dword_64C8F8, 0, (int)v24, 0);
  v2(&v20, this[6] + 100, this[7] + 22, this[6] + 180, this[7] + 37);
  MEMORY[0x49FA18](v24, "%d", this[202]);
  sub_48BD63(&dword_8FCC50, 0, v20, v21, v22, v23, (int)dword_64C8F8, 0, (int)v24, 0);
  v15 = 4;
  do
  {
    result = (*(int (__thiscall **)(_DWORD *))(*v14 + 4))(v14);
    v14 += 45;
    --v15;
  }
  while ( v15 );
  return result;
}
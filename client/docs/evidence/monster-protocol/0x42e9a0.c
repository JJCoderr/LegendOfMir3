int __thiscall sub_42E9A0(_DWORD *this, FILE *a2)
{
  FILE *v2; // ebx
  __int16 cnt; // ax
  _DWORD *v4; // esi
  unsigned __int16 cnt_high; // di
  unsigned __int16 base; // bp
  _DWORD *v7; // eax
  _DWORD *v8; // edi
  int v9; // edx
  _DWORD *v10; // eax
  _DWORD *v11; // esi
  void (__thiscall **v12)(_DWORD *, int); // eax
  _DWORD *v13; // eax
  int v15; // [esp+18h] [ebp-54h]
  int v16; // [esp+20h] [ebp-4Ch]
  int v17; // [esp+38h] [ebp-34h]
  char *ptr; // [esp+3Ch] [ebp-30h]
  _DWORD v20[2]; // [esp+48h] [ebp-24h] BYREF
  FILE *v21; // [esp+50h] [ebp-1Ch] BYREF
  int v22; // [esp+68h] [ebp-4h]

  v2 = a2;
  cnt = a2->_cnt;
  v4 = 0;
  v17 = 0;
  if ( cnt != 10
    && cnt != 13
    && cnt != 11
    && cnt != 9
    && cnt != 32
    && cnt != 33
    && cnt != 20
    && cnt != 27
    && cnt != 801
    && cnt != 807
    && cnt != 808 )
  {
    return v17;
  }
  cnt_high = HIWORD(a2->_cnt);
  base = (unsigned __int16)a2->_base;
  ptr = a2->_ptr;
  sub_47D780((const char *)&a2->_flag, (int)&v21, 16);
  a2 = v21;
  if ( (unsigned __int8)v21 >= 2u )
  {
    if ( (_BYTE)v21 == 5 || (_BYTE)v21 == 4 )
    {
      v10 = MEMORY[0x4A01AA](0x62790u);
      v11 = v10;
      v22 = 1;
      if ( v10 )
      {
        sub_405760(v10);
        *v11 = &off_4AF7A8;
      }
      else
      {
        v11 = 0;
      }
      *(_DWORD *)v11[100110] = ptr;
      v12 = (void (__thiscall **)(_DWORD *, int))*v11;
      v22 = -1;
      if ( ((int (__thiscall *)(_DWORD *, _DWORD *, FILE **, _DWORD, _DWORD, _DWORD, _DWORD))v12[3])(
             v11,
             this + 230525,
             &a2,
             0,
             0,
             cnt_high,
             base) )
      {
        (*(void (__thiscall **)(_DWORD *, _DWORD *))(this[230489] + 4))(this + 230489, v11);
        MEMORY[0x481A30](v2);
        v17 = 1;
        v11[100095] = 300;
        v11[100096] = 0;
      }
      else if ( v11 )
      {
        (*(void (__thiscall **)(_DWORD *, int))*v11)(v11, 1);
      }
      return v17;
    }
    v7 = MEMORY[0x4A01AA](0x62790u);
    v22 = 2;
    if ( v7 )
      v4 = sub_405760(v7);
    v16 = cnt_high;
    v8 = this;
    *(_DWORD *)v4[100110] = ptr;
    LOBYTE(a2) = 3;
    v9 = *v4;
    v22 = -1;
    if ( !(*(int (__thiscall **)(_DWORD *, _DWORD *, FILE **, _DWORD, _DWORD, int, _DWORD))(v9 + 12))(
            v4,
            this + 230525,
            &a2,
            0,
            0,
            v16,
            base) )
    {
      if ( v4 )
        goto LABEL_32;
      return v17;
    }
LABEL_30:
    (*(void (__thiscall **)(_DWORD *, _DWORD *))(v8[230489] + 4))(v8 + 230489, v4);
    MEMORY[0x481A30](v2);
    v17 = 1;
    v4[100095] = 300;
    v4[100096] = 0;
    return v17;
  }
  v13 = MEMORY[0x4A01AA](0x6284Cu);
  v22 = 0;
  if ( v13 )
    v4 = sub_410550(v13);
  *(_DWORD *)v4[100110] = ptr;
  v22 = -1;
  sub_47D780((const char *)&v2[1]._ptr + 2, (int)v20, 5);
  v15 = cnt_high;
  v8 = this;
  if ( (*(int (__thiscall **)(_DWORD *, _DWORD *, _DWORD, _DWORD, int, _DWORD, FILE **, _DWORD *))(*v4 + 144))(
         v4,
         this + 230525,
         0,
         0,
         v15,
         base,
         &a2,
         v20) )
  {
    goto LABEL_30;
  }
  if ( v4 )
LABEL_32:
    (*(void (__thiscall **)(_DWORD *, int))*v4)(v4, 1);
  return v17;
}
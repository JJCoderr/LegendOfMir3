int __thiscall sub_43AB90(_DWORD *this, int a2)
{
  int v3; // edi
  int result; // eax
  int v5; // edx
  _BYTE *v6; // esi
  int v7; // eax
  int v8; // edx
  _DWORD *v9; // esi
  int v10; // eax
  int v11; // eax

  v3 = 0;
  sub_43B880(this);
  switch ( a2 )
  {
    case 0:
      if ( this[7372] )
      {
        sub_43AA30(this, 0);
        (*(void (__thiscall **)(_DWORD *, _DWORD))(this[7360] + 16))(this + 7360, 0);
        result = 0;
      }
      else
      {
        sub_43AA10(0);
        v5 = this[7360];
        v6 = this + 7360;
        (*(void (__thiscall **)(_BYTE *, int))(v5 + 16))(v6, 1);
        sub_441FD0(v6, 0);
        sub_4411F0(v6);
        result = 1;
      }
      break;
    case 1:
      if ( this[43743] )
      {
        sub_43AA30(this, 1);
        (*(void (__thiscall **)(_DWORD *, _DWORD))(this[43731] + 16))(this + 43731, 0);
        result = 0;
      }
      else
      {
        sub_43AA10(1);
        (*(void (__thiscall **)(_DWORD *, int))(this[43731] + 16))(this + 43731, 1);
        result = 1;
      }
      break;
    case 2:
      if ( this[54205] )
      {
        sub_43AA30(this, 2);
        (*(void (__thiscall **)(_DWORD *, _DWORD))(this[54193] + 16))(this + 54193, 0);
        result = 0;
      }
      else
      {
        sub_43AA10(2);
        (*(void (__thiscall **)(_DWORD *, int))(this[54193] + 16))(this + 54193, 1);
        result = 1;
      }
      break;
    case 3:
      if ( this[54714] )
      {
        sub_43AA30(this, 3);
        (*(void (__thiscall **)(_DWORD *, _DWORD))(this[54702] + 16))(this + 54702, 0);
        result = 0;
      }
      else
      {
        sub_43AA10(3);
        (*(void (__thiscall **)(_DWORD *, int))(this[54702] + 16))(this + 54702, 1);
        result = 1;
      }
      break;
    case 4:
      if ( this[74547] )
      {
        sub_43AA30(this, 4);
        (*(void (__thiscall **)(_DWORD *, _DWORD))(this[74535] + 16))(this + 74535, 0);
        result = 0;
      }
      else
      {
        sub_47CE80(dword_8FCDD8);
        sub_43AA10(4);
        (*(void (__thiscall **)(_DWORD *, int))(this[74535] + 16))(this + 74535, 1);
        result = 1;
      }
      break;
    case 6:
      if ( this[76066] )
      {
        sub_43AA30(this, 6);
        (*(void (__thiscall **)(_DWORD *, _DWORD))(this[76054] + 16))(this + 76054, 0);
        result = 0;
      }
      else
      {
        sub_43AA10(6);
        (*(void (__thiscall **)(_DWORD *, int))(this[76054] + 16))(this + 76054, 1);
        if ( this[76306] )
          sub_41FD70(1370, 1370, 1370);
        else
          sub_41FD70(1371, 1371, 1371);
        result = 1;
      }
      break;
    case 7:
      if ( this[76319] )
      {
        sub_43AA30(this, 7);
        (*(void (__thiscall **)(_DWORD *, _DWORD))(this[76307] + 16))(this + 76307, 0);
        result = 0;
      }
      else
      {
        sub_43AA10(7);
        (*(void (__thiscall **)(_DWORD *, int))(this[76307] + 16))(this + 76307, 1);
        result = 1;
      }
      break;
    case 9:
      if ( this[86727] )
      {
        sub_43AA30(this, 9);
        (*(void (__thiscall **)(_DWORD *, _DWORD))(this[86715] + 16))(this + 86715, 0);
        result = 0;
      }
      else
      {
        sub_43AA10(9);
        (*(void (__thiscall **)(_DWORD *, int))(this[86715] + 16))(this + 86715, 1);
        result = 1;
      }
      break;
    case 11:
      if ( this[87094] )
      {
        sub_43AA30(this, 11);
        (*(void (__thiscall **)(_DWORD *, _DWORD))(this[87082] + 16))(this + 87082, 0);
        result = 0;
      }
      else
      {
        sub_43AA10(11);
        (*(void (__thiscall **)(_DWORD *, int))(this[87082] + 16))(this + 87082, 1);
        result = 1;
      }
      break;
    case 12:
      if ( this[87257] )
      {
        sub_43AA30(this, 12);
        (*(void (__thiscall **)(_DWORD *, _DWORD))(this[87245] + 16))(this + 87245, 0);
        result = 0;
      }
      else
      {
        sub_43AA10(12);
        (*(void (__thiscall **)(_DWORD *, int))(this[87245] + 16))(this + 87245, 1);
        result = 1;
      }
      break;
    case 13:
      if ( this[88243] )
      {
        sub_43AA30(this, 13);
        (*(void (__thiscall **)(_DWORD *, _DWORD))(this[88231] + 16))(this + 88231, 0);
        result = 0;
      }
      else
      {
        sub_43AA10(13);
        (*(void (__thiscall **)(_DWORD *, int))(this[88231] + 16))(this + 88231, 1);
        result = 1;
      }
      break;
    case 14:
      if ( this[88489] )
      {
        sub_43AA30(this, 14);
        (*(void (__thiscall **)(_DWORD *, _DWORD))(this[88477] + 16))(this + 88477, 0);
        result = 0;
      }
      else
      {
        sub_43AA10(14);
        (*(void (__thiscall **)(_DWORD *, int))(this[88477] + 16))(this + 88477, 1);
        result = 1;
      }
      break;
    case 15:
      if ( !this[89434] )
      {
        sub_43AA10(15);
        (*(void (__thiscall **)(_DWORD *, int))(this[89422] + 16))(this + 89422, 1);
        v11 = this[89537];
        if ( !v11 )
          goto LABEL_72;
        dword_4AF2A0(v11, 5);
        dword_4AF250(this[89537]);
        dword_4AF234(
          this[89537],
          dword_8FCD9C + this[89533],
          dword_8FCDA0 + this[89534],
          this[89535] - this[89533],
          this[89536] - this[89534],
          1);
        v8 = this[89537];
        goto LABEL_71;
      }
      if ( dword_767898 )
        dword_4AF234(dword_8FB92C, dword_8FCD9C + 185, dword_8FCDA0 + 534, 432, 15, 1);
      else
        dword_4AF234(dword_8FB92C, dword_8FCD9C + 185, dword_8FCDA0 + 575, 344, 15, 1);
      sub_43AA30(this, 15);
      (*(void (__thiscall **)(_DWORD *, _DWORD))(this[89422] + 16))(this + 89422, 0);
      v10 = this[89537];
      if ( v10 )
      {
        dword_4AF2A0(v10, 0);
        dword_4AF26C(this[89537], dword_902E34);
      }
      dword_4AF2B0(dword_8FCC58);
      result = 0;
      dword_8FB938 = 1;
      break;
    case 16:
      if ( this[89551] )
      {
        sub_43AA30(this, 16);
        (*(void (__thiscall **)(_DWORD *, _DWORD))(this[89539] + 16))(this + 89539, 0);
        result = 0;
      }
      else
      {
        sub_43AA10(16);
        (*(void (__thiscall **)(_DWORD *, int))(this[89539] + 16))(this + 89539, 1);
        result = 1;
      }
      break;
    case 18:
      if ( this[94310] )
      {
        sub_43AA30(this, 18);
        (*(void (__thiscall **)(_DWORD *, _DWORD))(this[94298] + 16))(this + 94298, 0);
        result = 0;
      }
      else
      {
        sub_43AA10(18);
        (*(void (__thiscall **)(_DWORD *, int))(this[94298] + 16))(this + 94298, 1);
        result = 1;
      }
      break;
    case 19:
      if ( this[94534] )
      {
        sub_456120(this + 94522);
        result = 0;
      }
      else
      {
        sub_43AA10(19);
        (*(void (__thiscall **)(_DWORD *, int))(this[94522] + 16))(this + 94522, 1);
        result = 1;
      }
      break;
    case 20:
      if ( !this[95824] )
      {
        if ( !this[7372] )
          sub_43AB90(this, 0);
        sub_43AA10(20);
        (*(void (__thiscall **)(_DWORD *, int))(this[95812] + 16))(this + 95812, 1);
        v7 = this[96755];
        if ( v7 )
        {
          dword_4AF2A0(v7, 5);
          dword_4AF250(this[96755]);
          dword_4AF234(
            this[96755],
            dword_8FCD9C + this[96756],
            dword_8FCDA0 + this[96757],
            this[96758] - this[96756],
            this[96759] - this[96757],
            1);
          v8 = this[96755];
LABEL_71:
          dword_4AF2B0(v8);
        }
LABEL_72:
        dword_8FB938 = 0;
        v3 = 1;
        goto LABEL_73;
      }
      sub_4572B0(this + 95812);
      result = 0;
      break;
    case 21:
      if ( this[96774] )
      {
        sub_43AA30(this, 21);
        (*(void (__thiscall **)(_DWORD *, _DWORD))(this[96762] + 16))(this + 96762, 0);
        result = 0;
      }
      else
      {
        sub_43AA10(21);
        (*(void (__thiscall **)(_DWORD *, int))(this[96762] + 16))(this + 96762, 1);
        result = 1;
      }
      break;
    case 22:
      if ( this[99367] )
      {
        sub_41F440(this + 99355);
        result = 0;
      }
      else
      {
        sub_43AA10(22);
        v9 = this + 99355;
        sub_41F4F0(v9);
        (*(void (__thiscall **)(_DWORD *, int))(*v9 + 16))(v9, 1);
        result = 1;
      }
      break;
    default:
LABEL_73:
      result = v3;
      break;
  }
  return result;
}
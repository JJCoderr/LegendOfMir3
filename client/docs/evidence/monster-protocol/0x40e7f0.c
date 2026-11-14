int __thiscall sub_40E7F0(char *this, int a2, int a3)
{
  int v4; // edx
  int v5; // ecx
  int v6; // ecx
  int v7; // eax
  unsigned int v8; // eax
  unsigned int v9; // ecx
  int v10; // edx
  unsigned int v11; // eax
  int v12; // ecx
  char v13; // cl
  int v14; // edx
  unsigned __int16 v15; // ax
  int v16; // eax

  v4 = *((_DWORD *)this + 100096);
  v5 = *((_DWORD *)this + 100095);
  *((_DWORD *)this + 100096) = a2 + v4;
  if ( a2 + v4 >= v5 )
  {
    *((_DWORD *)this + 100096) = 0;
    *((_DWORD *)this + 100095) = 0;
    *((_DWORD *)this + 100097) = 0;
  }
  if ( this[120] == 4 && *((_DWORD *)this + 31) >= (unsigned int)(*((_DWORD *)this + 29) - 1) )
    *((_DWORD *)this + 100108) = 1;
  if ( !(*(int (__thiscall **)(char *, int))(*(_DWORD *)this + 32))(this, a3) )
  {
    v6 = *((_DWORD *)this + 100110);
    v7 = a2 + *((_DWORD *)this + 32);
    *((_DWORD *)this + 32) = v7;
    if ( v7 > *(_DWORD *)(v6 + 5) )
    {
      v8 = *((_DWORD *)this + 31);
      v9 = *((_DWORD *)this + 29);
      *((_DWORD *)this + 32) = 0;
      if ( v8 < v9 )
      {
        v10 = *(_DWORD *)this;
        *((_DWORD *)this + 31) = v8 + 1;
        (*(void (__thiscall **)(char *))(v10 + 20))(this);
        if ( *((_DWORD *)this + 100050) )
        {
          *(int *)(*((_DWORD *)this + 100110) + 5) /= 2;
          *((_DWORD *)this + 100050) = 0;
        }
      }
    }
    v11 = *((_DWORD *)this + 31);
    if ( v11 >= *((_DWORD *)this + 29) )
    {
      v12 = *((_DWORD *)this + 28);
      *((_DWORD *)this + 37) = 0;
      *((_DWORD *)this + 31) = v12;
      v13 = this[120];
      *((_DWORD *)this + 38) = 0;
      *((_DWORD *)this + 35) = 0;
      *((_DWORD *)this + 36) = 0;
      *((_DWORD *)this + 100098) = 0;
      *((_DWORD *)this + 100052) = 0;
      *((_DWORD *)this + 100053) = 0;
      *((_DWORD *)this + 100054) = 0;
      this[400220] = 0;
      this[400221] = 10;
      *((_DWORD *)this + 100059) = 0;
      *((_DWORD *)this + 100060) = 0;
      if ( v13 == 8 )
      {
        v11 = *(unsigned __int16 *)(this + 71);
        if ( this[69] == 3
          && ((_WORD)v11 == 530 || (_WORD)v11 == 531 || (_WORD)v11 == 532 || (_WORD)v11 == 533 || (_WORD)v11 == 538) )
        {
          if ( this[400444] == 1 )
          {
            this[121] = 7;
          }
          else
          {
            this[400444] = 1;
            this[121] = 0;
          }
        }
      }
      if ( *((_DWORD *)this + 100108) )
      {
        LOBYTE(v11) = this[121];
        v14 = *(_DWORD *)this;
        if ( v13 == 10 )
        {
          (*(void (__thiscall **)(char *, int, unsigned int, _DWORD))(v14 + 16))(this, 10, v11, 0);
        }
        else
        {
          (*(void (__thiscall **)(char *, int, unsigned int, _DWORD))(v14 + 16))(this, 4, v11, 0);
          *((_DWORD *)this + 31) = *((_DWORD *)this + 29) - 1;
        }
      }
      else
      {
        LOBYTE(v11) = this[121];
        (*(void (__thiscall **)(char *, _DWORD, unsigned int, _DWORD))(*(_DWORD *)this + 16))(this, 0, v11, 0);
      }
      if ( (this[400396] & 1) != 0 )
      {
        v15 = this[69] == 3 ? *(_WORD *)(this + 71) : (unsigned __int8)this[71];
        if ( v15 >= 0x53u && v15 <= 0x55u )
        {
          (*(void (__thiscall **)(char *, int, _DWORD, _DWORD))(*(_DWORD *)this + 16))(this, 8, 0, 0);
          v16 = *((_DWORD *)this + 28);
          this[120] = 0;
          *((_DWORD *)this + 29) = v16 + 1;
        }
      }
      if ( this[400444] == 2 )
        this[400444] = 3;
    }
  }
  return (*(int (__thiscall **)(char *))(*(_DWORD *)this + 24))(this);
}
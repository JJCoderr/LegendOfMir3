char __thiscall sub_4644F0(_DWORD *this)
{
  int v2; // eax
  void (__stdcall *v3)(char *, char *, char *, _BYTE *, int, char *); // esi
  int v4; // eax
  int v5; // eax
  int v6; // eax
  int v7; // eax
  int v8; // eax
  int v9; // eax
  _BYTE v11[260]; // [esp+180h] [ebp-208h] BYREF
  char v12[260]; // [esp+284h] [ebp-104h] BYREF

  memset(v12, 0, sizeof(v12));
  memset(v11, 0, sizeof(v11));
  v2 = dword_4AF110(260, v12);
  if ( v2 )
  {
    LOBYTE(v2) = v12[0];
    if ( v12[0] )
    {
      strcat(v12, MEMORY[0x4B5864]);
      v3 = (void (__stdcall *)(char *, char *, char *, _BYTE *, int, char *))dword_4AF130;
      LOBYTE(v2) = dword_4AF130(MEMORY[0x4B584C], MEMORY[0x4B5D34], MEMORY[0x4B5D44], v11, 260, v12);
      if ( v11 )
      {
        v4 = MEMORY[0x49FAF5](v11);
        this[22] = v4;
        if ( v4 )
        {
          sub_4834C0(dword_8FC5D8);
        }
        else
        {
          sub_4834E0(dword_8FC5D8);
          sub_4834D0(dword_8FC5D8);
        }
        v3(MEMORY[0x4B584C], MEMORY[0x4B5D20], MEMORY[0x4B5D40], v11, 260, v12);
        Value = MEMORY[0x49FAF5](v11);
        v3(MEMORY[0x4B584C], (char *)&off_4B5D1C, MEMORY[0x4B5D44], v11, 260, v12);
        v5 = MEMORY[0x49FAF5](v11);
        this[21] = v5;
        if ( v5 )
        {
          sub_483700(dword_8FC5D8);
        }
        else
        {
          sub_4836E0(dword_8FC5D8);
          sub_483720(dword_8FC5D8);
        }
        v3(MEMORY[0x4B584C], MEMORY[0x4B587C], MEMORY[0x4B5D44], v11, 260, v12);
        dword_76740C = MEMORY[0x49FAF5](v11);
        v3(MEMORY[0x4B584C], MEMORY[0x4B5CD0], MEMORY[0x4B5D44], v11, 260, v12);
        dword_8F1D44 = MEMORY[0x49FAF5](v11);
        v3(MEMORY[0x4B584C], MEMORY[0x4B5CC4], MEMORY[0x4B5D44], v11, 260, v12);
        dword_8F1D48 = MEMORY[0x49FAF5](v11);
        v3(MEMORY[0x4B584C], MEMORY[0x4B5CB8], MEMORY[0x4B5D44], v11, 260, v12);
        dword_8F1D30 = MEMORY[0x49FAF5](v11);
        v3(MEMORY[0x4B584C], MEMORY[0x4B5D10], MEMORY[0x4B5D40], v11, 260, v12);
        v6 = MEMORY[0x49FAF5](v11);
        dword_8FC5F8 = v6;
        if ( v6 > 0 || v6 < -100 )
          dword_8FC5F8 = 0;
        v7 = Value;
        if ( Value > 0 || Value < -100 )
        {
          v7 = 0;
          Value = 0;
        }
        this[39] = MEMORY[0x49FE1C]((double)(v7 + 100) * dbl_4AFAD0);
        this[37] = MEMORY[0x49FE1C]((double)(dword_8FC5F8 + 100) * dbl_4AFAD0);
        v3(MEMORY[0x4B584C], MEMORY[0x4B5D04], MEMORY[0x4B5D40], v11, 260, v12);
        this[23] = MEMORY[0x49FAF5](v11);
        v3(MEMORY[0x4B584C], MEMORY[0x4B5CF8], MEMORY[0x4B5D40], v11, 260, v12);
        v8 = MEMORY[0x49FAF5](v11);
        this[24] = v8;
        dword_4B98AC = v8;
        v3(MEMORY[0x4B584C], MEMORY[0x4B5CEC], MEMORY[0x4B5D40], v11, 260, v12);
        v9 = MEMORY[0x49FAF5](v11);
        this[25] = v9;
        dword_8FCC4C = v9;
        v3(MEMORY[0x4B584C], MEMORY[0x4B5CE4], MEMORY[0x4B5D44], v11, 260, v12);
        dword_7678E4 = MEMORY[0x49FAF5](v11);
        v3(MEMORY[0x4B584C], MEMORY[0x4B5CDC], MEMORY[0x4B5D44], v11, 260, v12);
        dword_7678E8 = MEMORY[0x49FAF5](v11);
        v3(MEMORY[0x4B584C], MEMORY[0x4B5870], MEMORY[0x4B5D40], v11, 260, v12);
        dword_4B9880 = MEMORY[0x49FAF5](v11);
        v3(MEMORY[0x4B584C], MEMORY[0x4B5854], MEMORY[0x4B5D40], v11, 260, v12);
        dword_7678A0 = MEMORY[0x49FAF5](v11);
        v3(MEMORY[0x4B584C], MEMORY[0x4B5748], MEMORY[0x4B5D44], v11, 260, v12);
        v2 = MEMORY[0x49FAF5](v11);
        dword_767908 = v2;
      }
    }
  }
  return v2;
}
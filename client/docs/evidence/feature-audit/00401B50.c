int __stdcall WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
  int v4; // ebp
  int v6; // eax
  __time32_t v7; // eax
  int v8; // ebp
  BOOL v9; // eax
  void (__stdcall *v10)(int *); // edi
  int v11; // eax
  bool v12; // zf
  int v13; // esi
  int v14; // [esp+68h] [ebp-230h]
  int v15; // [esp+6Ch] [ebp-22Ch]
  int v16; // [esp+70h] [ebp-228h]
  int v17; // [esp+74h] [ebp-224h]
  int v18; // [esp+78h] [ebp-220h] BYREF
  unsigned __int16 v19; // [esp+7Eh] [ebp-21Ah]
  unsigned __int16 v20; // [esp+80h] [ebp-218h]
  unsigned __int16 v21; // [esp+82h] [ebp-216h]
  int v22; // [esp+84h] [ebp-214h]
  _BYTE v23[8]; // [esp+88h] [ebp-210h] BYREF
  int v24; // [esp+90h] [ebp-208h]
  char v25[48]; // [esp+A4h] [ebp-1F4h] BYREF
  _BYTE v26[400]; // [esp+108h] [ebp-190h] BYREF

  sub_480570(dword_4B9840);
  dword_4B97D8 = 20030704;
  dword_4B983C = 0;
  strcpy((char *)dword_4B9820, dword_902E34);
  dword_4B97DC = 0;
  dword_4B97C4 = 0;
  byte_4B9798 = 1;
  strcpy((char *)dword_4B9804, dword_902E34);
  strcpy((char *)dword_4B97F0, dword_902E34);
  strcpy((char *)dword_4B97E0, dword_902E34);
  strcpy((char *)dword_4B97C8, dword_902E34);
  if ( !(unsigned __int8)sub_401040() )
    return -1;
  dword_4B97C0 = 1;
  dword_8FCDB4 = dword_4B979C;
  if ( dword_4AF2A4(0, MEMORY[0x4B5180]) )
    return -1;
  if ( byte_4B979A )
    v6 = sub_48AE74(hInstance, MEMORY[0x4B5180], 0, 102, 2, 5, 1);
  else
    v6 = sub_48AE74(hInstance, MEMORY[0x4B5180], 0, 102, 1, 5, 1);
  if ( !v6 )
    return -1;
  sub_47B880(&dword_8FB928, dword_8FCC54, dword_8FCC58, 0, 0, 0, 0);
  dword_4AF2A0(dword_8FB92C, 0);
  if ( (int)sub_482BC0(dword_8FCC58) >= 0 )
  {
    dword_8FC5E0 = 1;
    sub_4830B0(dword_8FC5D8, (int)MEMORY[0x4B53BC]);
    sub_4831A0(dword_8FC5D8, (int)MEMORY[0x4B53B0]);
  }
  else
  {
    dword_8FC5E0 = 0;
  }
  if ( !sub_46EFA0((char *)dword_8FBA50) )
    return -1;
  v7 = MEMORY[0x4A0263](0);
  sub_49FDA4(v7);
  if ( sub_493960(514, v26) )
    return -1;
  if ( byte_4B9799 == 1 )
  {
    dword_8FCDCC = (int)dword_8FA568;
    dword_902E20 = (int)dword_8FA568;
    sub_402C50(dword_8FA568);
    LOBYTE(dword_902E2C) = 0;
    if ( byte_4B979A )
      sub_48B48C(640, 480, 16, 2, 5);
    else
      sub_48B48C(640, 480, 16, 1, 5);
  }
  else
  {
    dword_8FCDCC = (int)dword_8F4408;
    dword_902E20 = (int)dword_8F4408;
    sub_488F20();
    LOBYTE(dword_902E2C) = 4;
  }
  v14 = v4;
  v8 = 0;
  v9 = sub_401740();
  v10 = (void (__stdcall *)(int *))dword_4AF124;
  dword_902E28 = v9;
  while ( 1 )
  {
    while ( !dword_4AF29C(v23, 0, 0, 0, 0) )
    {
      v11 = dword_4AF2FC(v14, v15, v16, v17, v18);
      v13 = v11 - v8;
      v12 = v11 == v8;
      v8 = v11;
      if ( !v12 && dword_8FCD98 )
      {
        switch ( (char)dword_902E2C )
        {
          case 0:
            sub_403220(*(float *)&v13);
            break;
          case 2:
            sub_485AD0(v13);
            break;
          case 3:
            if ( dword_902E24 == dword_4AF28C(dword_8FCC58, -4) )
            {
              if ( dword_8FCD94 )
              {
                sub_424280((int)dword_4B9878, 0, v8, (int)v10, v13, v13, v14);
              }
              else
              {
                sub_423BC0(dword_4B9878);
                sub_423C80(dword_4B9878);
              }
            }
            break;
          case 4:
            sub_4891C0(dword_8F4408, v13);
            break;
          default:
            break;
        }
        if ( (unsigned __int16)dword_4AF288(44) )
        {
          v10(&v18);
          MEMORY[0x49FA18](
            v25,
            "[%sServer %s] - %d/%d/%d %d:%d:%d",
            (const char *)dword_4B97F0,
            (const char *)dword_4B9804,
            (unsigned __int16)v18,
            HIWORD(v18),
            v19,
            v20,
            v21,
            (unsigned __int16)v22);
          sub_48BCAF(&dword_8FCC50, 0, 0, 0, 657930, 0, v25, 0);
          sub_48BCAF(&dword_8FCC50, 0, 1, 1, (int)&off_96C8FF, 0, v25, 0);
          sub_481CB0(dword_8FCC6C, 0, v14, v15, v16);
        }
        if ( sub_4914DD(&dword_8FCC50) < 0 )
          sub_49153B(&dword_8FCC50);
      }
    }
    if ( !dword_4AF298(v23, 0, 0, 0) )
      return v24;
    dword_4AF294(v23);
    dword_4AF290(v23);
  }
}
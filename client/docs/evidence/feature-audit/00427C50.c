int __thiscall sub_427C50(int this, char *Str)
{
  char *v3; // eax
  int v4; // edi
  int v5; // ecx
  int v6; // eax
  int v7; // ecx
  int v8; // esi
  double v9; // st7
  int v10; // eax
  int v11; // esi
  int v12; // edi
  const char *v13; // eax
  int v14; // eax
  int result; // eax
  int v16; // [esp+10h] [ebp-10h]
  int v17; // [esp+14h] [ebp-Ch]

  v3 = MEMORY[0x49FFB0](Str, 47);
  if ( v3 )
  {
    v4 = MEMORY[0x49FAF5](v3 + 1);
    v5 = ((int (__cdecl *)())dword_4AF2FC)();
    v6 = *(_DWORD *)(this + 4424772);
    v17 = v5;
    if ( v6 )
    {
      v7 = v5 - *(_DWORD *)(this + 4424768);
      v8 = *(_DWORD *)(this + 4424780) + 1;
      *(_DWORD *)(this + 4424780) = v8;
      *(float *)(this + 4424776) = fabs((double)(unsigned int)(v4 - v6) / (double)v7) + *(float *)(this + 4424776);
      if ( v8 >= sub_401710(15, 25) )
      {
        v9 = *(float *)(this + 4424776) / (double)v8;
        *(float *)(this + 4424784) = v9;
        if ( v9 > flt_4AF6E4 )
          *(_DWORD *)((char *)&loc_438450 + this) = 1065353216;
        if ( *(float *)(this + 4424784) >= (double)flt_4AF834 )
        {
          v14 = *(_DWORD *)(this + 4424788) - 1;
          *(_DWORD *)(this + 4424788) = v14;
          if ( v14 < 0 )
            *(_DWORD *)(this + 4424788) = 0;
        }
        else
        {
          v10 = *(_DWORD *)(this + 4424788) + 1;
          *(_DWORD *)(this + 4424788) = v10;
          if ( v10 > 2 )
          {
            v11 = sub_401430(2);
            v12 = sub_401430(3);
            v13 = (const char *)sub_480550(1431);
            sub_4366D0((_DWORD *)(this + 2780620), v11, v12, v13);
            v4 = v16;
            *(_DWORD *)(this + 4424788) = 0;
          }
        }
        *(_DWORD *)(this + 4424780) = 0;
        *(_DWORD *)(this + 4424776) = 0;
      }
    }
    *(_DWORD *)(this + 4424772) = v4;
    *(_DWORD *)(this + 4424768) = v17;
  }
  result = 0;
  if ( !memcmp(Str, MEMORY[0x4B519C], strlen(MEMORY[0x4B519C])) )
  {
    *(_DWORD *)(this + 3583488) = 0;
  }
  else if ( !memcmp(Str, MEMORY[0x4B51A4], strlen(MEMORY[0x4B51A4])) )
  {
    return sub_415E60((_DWORD *)(this + 3179944));
  }
  else
  {
    result = 0;
    if ( !memcmp(Str, MEMORY[0x4B51AC], strlen(MEMORY[0x4B51AC])) )
    {
      *(_DWORD *)(this + 3583308) = 1;
    }
    else
    {
      result = 0;
      if ( !memcmp(Str, MEMORY[0x4B51B0], strlen(MEMORY[0x4B51B0])) )
      {
        *(_DWORD *)(this + 3583308) = 0;
      }
      else
      {
        result = 0;
        if ( !memcmp(Str, MEMORY[0x4B51B8], strlen(MEMORY[0x4B51B8])) )
        {
          *(_DWORD *)(this + 3583300) = 1;
          *(_DWORD *)(this + 3583304) = 0;
        }
        else
        {
          result = 0;
          if ( !memcmp(Str, MEMORY[0x4B51BC], strlen(MEMORY[0x4B51BC])) )
          {
            *(_DWORD *)(this + 3583300) = 0;
          }
          else
          {
            result = 0;
            if ( !memcmp(Str, &off_4B51C4, strlen((const char *)&off_4B51C4)) )
            {
              *(_BYTE *)(this + 3583299) = 1;
            }
            else
            {
              result = 0;
              if ( !memcmp(Str, MEMORY[0x4B51C8], strlen(MEMORY[0x4B51C8])) )
              {
                *(_BYTE *)(this + 3583299) = 0;
              }
              else
              {
                result = 0;
                if ( !memcmp(Str, &dword_4B51D8, strlen((const char *)&dword_4B51D8)) )
                {
                  *(_BYTE *)(this + 3583297) = 1;
                }
                else
                {
                  result = 0;
                  if ( !memcmp(Str, MEMORY[0x4B51DC], strlen(MEMORY[0x4B51DC])) )
                  {
                    *(_BYTE *)(this + 3583297) = 0;
                  }
                  else
                  {
                    result = 0;
                    if ( !memcmp(Str, &off_4B51E4, strlen((const char *)&off_4B51E4)) )
                    {
                      *(_BYTE *)(this + 3583298) = 1;
                    }
                    else
                    {
                      result = 0;
                      if ( !memcmp(Str, MEMORY[0x4B51E8], strlen(MEMORY[0x4B51E8])) )
                      {
                        *(_BYTE *)(this + 3583298) = 0;
                      }
                      else
                      {
                        result = 0;
                        if ( !memcmp(Str, &off_4B51D0, strlen((const char *)&off_4B51D0)) )
                        {
                          *(_BYTE *)(this + 3583296) = 1;
                        }
                        else
                        {
                          result = 0;
                          if ( !memcmp(Str, &dword_4B51D4, strlen((const char *)&dword_4B51D4)) )
                          {
                            *(_DWORD *)(this + 3583400) = 1;
                          }
                          else
                          {
                            result = 0;
                            if ( !memcmp(Str, &off_4B51F0, strlen((const char *)&off_4B51F0)) )
                            {
                              *(_DWORD *)(this + 3583304) = 1;
                              *(_DWORD *)(this + 3583300) = 0;
                            }
                            else
                            {
                              result = 0;
                              if ( !memcmp(Str, MEMORY[0x4B51F4], strlen(MEMORY[0x4B51F4])) )
                                *(_DWORD *)(this + 3583304) = 0;
                            }
                          }
                        }
                      }
                    }
                  }
                }
              }
            }
          }
        }
      }
    }
  }
  return result;
}
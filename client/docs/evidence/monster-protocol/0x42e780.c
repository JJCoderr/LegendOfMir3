unsigned int __thiscall sub_42E780(_DWORD *this)
{
  unsigned int result; // eax
  _DWORD *v3; // ecx
  int v4; // eax
  _DWORD *v5; // ecx
  __int16 v6; // ax
  int v7; // ebp
  int v8; // esi
  void *v9; // eax
  int v10; // ecx
  int v11; // eax
  _DWORD *v12; // [esp+10h] [ebp-Ch]
  int v13; // [esp+14h] [ebp-8h]
  unsigned int v14; // [esp+18h] [ebp-4h]

  v14 = 0;
  result = this[1005644];
  v3 = this + 1005644;
  if ( !result )
    return result;
  while ( 1 )
  {
    v4 = sub_481AF0(v3);
    v5 = (_DWORD *)v4;
    v12 = (_DWORD *)v4;
    if ( v4 )
    {
      v6 = *(_WORD *)(v4 + 4);
      if ( v6 == 30 || v6 == 29 || v6 == 803 )
      {
        v7 = this[230490];
        if ( v7 )
        {
          while ( 1 )
          {
            v8 = *(_DWORD *)(v7 + 4);
            if ( v8 )
            {
              if ( **(_DWORD **)(v8 + 400440) == *v5 )
                break;
            }
            v7 = *(_DWORD *)(v7 + 12);
            if ( !v7 )
              goto LABEL_30;
          }
          sub_423CF0(*(_DWORD *)(v7 + 4));
          if ( v8 == this[905629] )
            this[905629] = 0;
          if ( v8 == this[905630] )
            this[905630] = 0;
          if ( v8 == this[905631] )
            this[905631] = 0;
          if ( v8 == this[905632] )
            this[905632] = 0;
          if ( *(int *)(v8 + 164) > 0 )
          {
            v13 = *(_DWORD *)(v8 + 164);
            do
            {
              v9 = (void *)sub_481AF0(v8 + 164);
              if ( v9 )
                MEMORY[0x4939F6](v9);
              --v13;
            }
            while ( v13 );
          }
          switch ( *(_BYTE *)(v8 + 69) )
          {
            case 0:
            case 1:
            case 4:
            case 5:
              goto LABEL_28;
            case 3:
              if ( *(_BYTE *)(v8 + 400444) != 2 )
              {
LABEL_28:
                (*(void (__thiscall **)(_DWORD *, int))(this[230489] + 20))(this + 230489, v7);
                (**(void (__thiscall ***)(int, int))v8)(v8, 1);
              }
              break;
            default:
              break;
          }
          v5 = v12;
        }
LABEL_30:
        MEMORY[0x4939F6](v5);
        goto LABEL_31;
      }
      if ( v6 == 633 )
        break;
    }
LABEL_31:
    v3 = this + 1005644;
    result = ++v14;
    if ( v14 >= this[1005644] )
      return result;
  }
  if ( v5 )
    MEMORY[0x4939F6](v5);
  sub_421DD0(this);
  for ( result = this[230502]; result; result = this[230502] )
  {
    MEMORY[0x4939F6](*(void **)(result + 4));
    v10 = this[230502];
    v11 = *(_DWORD *)(v10 + 12);
    this[230502] = v11;
    if ( v11 )
      *(_DWORD *)(v11 + 8) = 0;
    if ( v10 )
      (**(void (__thiscall ***)(int, int))v10)(v10, 1);
  }
  this[230502] = 0;
  this[230503] = 0;
  this[230506] = 0;
  this[905629] = 0;
  this[905630] = 0;
  this[905631] = 0;
  this[905632] = 0;
  return result;
}
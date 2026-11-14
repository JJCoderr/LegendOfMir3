int __userpurge sub_426D20@<eax>(int this@<ecx>, int a2@<ebx>, int a3, unsigned int a4)
{
  int v6; // edi
  int v7; // [esp-8h] [ebp-10h]
  int v8; // [esp-4h] [ebp-Ch]

  *(_DWORD *)(this + 3583684) = (unsigned __int16)a4;
  *(_DWORD *)(this + 3583688) = HIWORD(a4);
  if ( *(_DWORD *)(this + 3614112) )
  {
    if ( (*(int (__thiscall **)(int, _DWORD, _DWORD))(*(_DWORD *)(this + 3614084) + 32))(
           this + 3614084,
           (unsigned __int16)a4,
           *(_DWORD *)(this + 3583688)) )
    {
      (*(void (__thiscall **)(int))(*(_DWORD *)(this + 3614084) + 20))(this + 3614084);
      *(_DWORD *)(this + 4424764) = 0;
      *(_DWORD *)(this + 84) = 1;
      *(_BYTE *)(this + 4424760) = 1;
      return 0;
    }
  }
  else if ( !sub_420BB0(this + 3610584, this, *(_DWORD *)(this + 3583684), *(_DWORD *)(this + 3583688))
         && *(_DWORD *)(this + 3583700)
         && !*(_DWORD *)(this + 4424344) )
  {
    v8 = *(_DWORD *)(this + 3583688);
    v6 = *(_DWORD *)(this + 1010792);
    *(_DWORD *)(this + 3583692) = *(_DWORD *)(this + 1010788)
                                + (*(_DWORD *)(this + 3583684) - *(_DWORD *)(this + 1010804)) / 48;
    v7 = *(_DWORD *)(this + 3583684);
    *(_DWORD *)(this + 3583696) = v6 + (v8 - *(_DWORD *)(this + 1010808)) / 32;
    sub_43C130((_DWORD *)(this + 2780620), a2, v7, v8);
  }
  return 0;
}
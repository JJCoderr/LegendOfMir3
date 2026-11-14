int __thiscall sub_423E90(_DWORD *this, int __return_address)
{
  int v3; // ebx
  int v4; // eax
  int *v5; // edi
  _DWORD *v6; // esi
  int v7; // eax
  unsigned int v8; // eax
  int result; // eax
  int v10; // [esp+14h] [ebp-24h]
  _DWORD v11[4]; // [esp+18h] [ebp-20h] BYREF
  _BYTE v12[16]; // [esp+28h] [ebp-10h] BYREF

  sub_47D780((const char *)(__return_address + 12), (int)v12, 16);
  v3 = 0;
  *(_DWORD *)((char *)&loc_4382A4 + (_DWORD)this) = 0;
  *((_BYTE *)&loc_4382A4
  + (_DWORD)this
  + sub_47D780((const char *)(__return_address + 34), (int)&loc_4382A4 + (_DWORD)this, 260)) = 0;
  v11[0] = v12[0];
  v11[1] = v12[1];
  v4 = *(unsigned __int16 *)(__return_address + 6);
  v11[2] = v12[2];
  v11[3] = v12[3];
  if ( (unsigned __int16)v4 < 4u )
    memset(&v11[v4], 0, 4 * (4 - v4));
  this[1106086] = 1;
  this[1106087] = 0;
  this[1106088] = 0;
  memset(this + 1106154, 0, 0x90u);
  this[1106082] = 0;
  this[1106083] = 0;
  this[1106084] = 0;
  this[1106085] = 0;
  dword_4AF2A8(
    this + 1106082,
    400 - 48 * *(unsigned __int16 *)(__return_address + 6) / 2,
    210,
    48 * *(unsigned __int16 *)(__return_address + 6) / 2 + 400,
    270);
  v10 = 0;
  if ( *(_WORD *)(__return_address + 6) )
  {
    v5 = v11;
    v6 = this + 1106155;
    do
    {
      *(v6 - 1) = 1;
      *v6 = 150;
      v6[1] = sub_401710(1500, 2500);
      v6[2] = 0;
      v6[4] = sub_401710(80, 120);
      v7 = *v5;
      v6[3] = 0;
      v6[5] = v7;
      v6[6] = this[1106082] + v3 + 12;
      v6[7] = this[1106083] + 17;
      v8 = v6[1];
      if ( v8 > this[1106087] )
        this[1106087] = v8;
      ++v5;
      v3 += 48;
      v6 += 9;
      ++v10;
    }
    while ( v10 < *(unsigned __int16 *)(__return_address + 6) );
  }
  result = this[1106087] + 2000;
  this[1106087] = result;
  return result;
}
_DWORD *__thiscall sub_476B80(_DWORD *this)
{
  *this = &off_4AF664;
  sub_4315D0(this);
  MEMORY[0x4A033F](
    this + 21,
    0xB4u,
    8,
    (void (__thiscall *)(void *))sub_4052D0,
    (void (__thiscall *)(void *))&loc_4052F0);
  sub_41FE00(this + 382);
  this[401] = &off_4AFC6C;
  this[402] = 0;
  this[403] = 0;
  this[404] = 0;
  this[405] = 0;
  this[406] = 0;
  this[441] = &off_4AFC50;
  this[442] = 0;
  this[443] = 0;
  this[444] = 0;
  this[445] = 0;
  this[446] = 0;
  *this = &off_4AFC3C;
  sub_476D10(this);
  return this;
}
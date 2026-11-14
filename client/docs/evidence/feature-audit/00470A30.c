_DWORD *__thiscall sub_470A30(_DWORD *this)
{
  *this = &off_4AF664;
  sub_4315D0(this);
  MEMORY[0x4A033F](
    this + 22,
    0xB4u,
    2,
    (void (__thiscall *)(void *))sub_4052D0,
    (void (__thiscall *)(void *))&loc_4052F0);
  MEMORY[0x4A033F](
    this + 232,
    0xC28u,
    12,
    (void (__thiscall *)(void *))sub_419E00,
    (void (__thiscall *)(void *))sub_419E10);
  sub_4420D0(this + 9569);
  *this = &off_4AFC18;
  sub_470B90(this);
  return this;
}
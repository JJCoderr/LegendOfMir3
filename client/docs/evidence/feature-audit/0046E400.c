_DWORD *__thiscall sub_46E400(_DWORD *this)
{
  *this = &off_4AF664;
  sub_4315D0(this);
  MEMORY[0x4A033F](
    this + 21,
    0xB4u,
    4,
    (void (__thiscall *)(void *))sub_4052D0,
    (void (__thiscall *)(void *))&loc_4052F0);
  *this = &off_4AFBFC;
  sub_46E500(this);
  return this;
}
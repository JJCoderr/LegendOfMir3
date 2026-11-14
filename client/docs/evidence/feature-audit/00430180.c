int __thiscall sub_430180(_DWORD *this, int a2)
{
  __int64 v3; // rax
  __int64 v4; // rax
  int *v5; // edx
  int result; // eax
  __int64 v7; // rax
  __int64 v8; // rax

  if ( **(_DWORD **)(a2 + 400440) == this[790178] )
  {
    v3 = *(_DWORD *)(a2 + 132) - this[795019];
    if ( (int)((HIDWORD(v3) ^ v3) - HIDWORD(v3)) > 10
      || (v4 = *(_DWORD *)(a2 + 136) - this[795020], (int)((HIDWORD(v4) ^ v4) - HIDWORD(v4)) > 10) )
    {
      sub_456120(this + 789677);
    }
  }
  v5 = *(int **)(a2 + 400440);
  result = *v5;
  if ( *v5 == this[791909] )
  {
    v7 = *(_DWORD *)(a2 + 132) - this[795019];
    if ( (int)((HIDWORD(v7) ^ v7) - HIDWORD(v7)) > 10 )
      return sub_4572B0(this + 790967);
    v8 = *(_DWORD *)(a2 + 136) - this[795020];
    result = (HIDWORD(v8) ^ v8) - HIDWORD(v8);
    if ( result > 10 )
      return sub_4572B0(this + 790967);
  }
  return result;
}
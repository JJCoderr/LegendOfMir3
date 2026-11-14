void __thiscall sub_421FF0(_DWORD *this, int a2, int a3)
{
  int v4; // eax
  int v5; // ebx
  int v6; // ecx
  int v7; // esi
  int v8; // edi
  int v9; // eax
  int v10; // ecx
  int v11; // eax
  int v12; // ecx
  int v13; // edi
  _DWORD *v14; // eax
  int v15; // edx
  int v16; // eax
  int v17; // ecx
  _DWORD *v18; // eax
  int *v19; // esi
  __int16 v20; // ax
  int v21; // edx
  int v22; // ecx
  int v23; // edi
  int v24; // ebx
  int v25; // eax
  int v26; // ecx
  int v27; // edx
  int v28; // eax
  _DWORD *v29; // ecx
  int v30; // edi
  int v31; // edi
  int v32; // ebx
  int v33; // eax
  int v34; // ecx
  _DWORD *v35; // ecx
  int v36; // edi
  int v37; // ebx
  int v38; // eax
  int v39; // ecx
  _DWORD *v40; // ecx
  int v41; // edi
  int v42; // ebx
  int v43; // eax
  int v44; // ecx
  _DWORD *v45; // ecx
  int v46; // edx
  int v47; // edi
  int v48; // eax
  int v49; // ecx
  int v50; // ebx
  int *v51; // ecx
  int v52; // eax
  int v53; // edi
  int v54; // ebx
  int v55; // eax
  int v56; // ecx
  _DWORD *v57; // ecx
  int v58; // edi
  _DWORD *v59; // esi
  int v60; // ebx
  int v61; // edi
  int v62; // ebx
  int v63; // eax
  int v64; // ecx
  int v65; // edx
  int v66; // eax
  _DWORD *v67; // ecx
  int v68; // esi
  _DWORD *v69; // ebx
  int v70; // edi
  int v71; // esi
  int v72; // edi
  int v73; // eax
  int v74; // ecx
  int v75; // edx
  int v76; // eax
  _DWORD *v77; // ecx
  int v78; // [esp+20h] [ebp-1C8h]
  int v79; // [esp+24h] [ebp-1C4h]
  int v80; // [esp+28h] [ebp-1C0h]
  int v81; // [esp+2Ch] [ebp-1BCh]
  int v82; // [esp+30h] [ebp-1B8h]
  int v83; // [esp+34h] [ebp-1B4h]
  int v84; // [esp+38h] [ebp-1B0h]
  int v85; // [esp+38h] [ebp-1B0h]
  int v86; // [esp+38h] [ebp-1B0h]
  int v87; // [esp+38h] [ebp-1B0h]
  int v88; // [esp+3Ch] [ebp-1ACh]
  int v89; // [esp+40h] [ebp-1A8h]
  int v90; // [esp+44h] [ebp-1A4h]
  _DWORD v91[4]; // [esp+48h] [ebp-1A0h] BYREF
  _DWORD v92[100]; // [esp+58h] [ebp-190h] BYREF

  memset(this + 89, 0, 0xE1000u);
  v4 = this[795019];
  v5 = this[230490];
  v91[0] = v4 - 24;
  v6 = this[795020];
  v90 = 0;
  v88 = 0;
  v89 = 0;
  v84 = 0;
  v91[1] = v6 - 24;
  v91[2] = v4 + 24;
  v91[3] = v6 + 24;
  if ( !v5 )
    goto LABEL_43;
  do
  {
    v7 = *(_DWORD *)(v5 + 4);
    if ( v7 )
    {
      switch ( *(_BYTE *)(v7 + 69) )
      {
        case 0:
        case 1:
        case 4:
        case 5:
          (*(void (__thiscall **)(int, int, int))(*(_DWORD *)v7 + 28))(v7, a2, a3);
          goto LABEL_5;
        case 3:
          (*(void (__thiscall **)(int, int, int))(*(_DWORD *)v7 + 28))(v7, a2, a3);
          if ( *(_BYTE *)(v7 + 400444) != 3 )
            goto LABEL_5;
LABEL_7:
          sub_423CF0(v7);
          v8 = *(_DWORD *)(v5 + 12);
          (*(void (__thiscall **)(_DWORD *, int))(this[230489] + 20))(this + 230489, v5);
          v5 = v8;
          (**(void (__thiscall ***)(int, int))v7)(v7, 1);
          continue;
        default:
LABEL_5:
          if ( !dword_4AF2AC(v91, *(_DWORD *)(v7 + 132), *(_DWORD *)(v7 + 136)) )
          {
            switch ( *(_BYTE *)(v7 + 69) )
            {
              case 0:
              case 1:
              case 3:
              case 4:
              case 5:
                goto LABEL_7;
              default:
                break;
            }
          }
          v9 = *(_DWORD *)(v7 + 132);
          v10 = this[252697];
          if ( v9 < v10 )
            goto LABEL_40;
          if ( v9 >= v10 + 24 )
            goto LABEL_40;
          v11 = *(_DWORD *)(v7 + 136);
          v12 = this[252698];
          if ( v11 < v12 || v11 >= v12 + 24 )
            goto LABEL_40;
          v13 = 0;
          *(_DWORD *)(*(_DWORD *)(v7 + 400440) + 16) = 0;
          if ( sub_4280E0(this, **(_DWORD **)(v7 + 400440)) )
          {
            *(_DWORD *)(*(_DWORD *)(v7 + 400440) + 16) = MEMORY[0x49FDAE]();
            *(_DWORD *)(v7 + 403140) = *(_DWORD *)(*(_DWORD *)(v7 + 400440) + 16);
          }
          v14 = &this[400 * *(_DWORD *)(v7 + 132)
                    + 89
                    + 400 * (24 * (*(_DWORD *)(v7 + 136) - this[252698]) - this[252697])];
          break;
      }
      while ( *v14 )
      {
        ++v13;
        ++v14;
        if ( v13 >= 100 )
          goto LABEL_40;
      }
      if ( v7 == this[905632] )
        v90 = 1;
      if ( this[23] && v7 == this[905634] )
        v89 = 1;
      if ( v7 == this[905630] )
        v88 = 1;
      if ( this[905629] )
      {
        if ( dword_4AF2AC(v7 + 96, this[895921], this[895922]) )
        {
          if ( *(_DWORD *)(v7 + 400432) )
            v84 = v7;
          else
            this[905629] = v7;
        }
      }
      else
      {
        v79 = this[895922];
        v78 = this[895921];
        if ( *(_DWORD *)(v7 + 400432) )
        {
          if ( dword_4AF2AC(v7 + 96, v78, v79) )
          {
            v84 = v7;
            this[905629] = v7;
          }
        }
        else if ( dword_4AF2AC(v7 + 80, v78, v79) )
        {
          this[905629] = v7;
        }
      }
      if ( *(_DWORD *)(v7 + 400432) )
      {
        v15 = this[252697];
        v16 = *(_DWORD *)(v7 + 136) - this[252698];
        v17 = *(_DWORD *)(v7 + 132);
        v92[0] = v7;
        v18 = &this[400 * v17 + 89 + 400 * (24 * v16 - v15)];
        qmemcpy(&v92[1], v18, 0x18Cu);
        qmemcpy(v18, v92, 0x190u);
      }
      else
      {
        this[400 * *(_DWORD *)(v7 + 132) + 89 + 400 * (24 * (*(_DWORD *)(v7 + 136) - this[252698]) - this[252697]) + v13] = v7;
      }
LABEL_40:
      v5 = *(_DWORD *)(v5 + 12);
    }
  }
  while ( v5 );
  if ( !v90 )
LABEL_43:
    this[905632] = 0;
  if ( !v88 )
    this[905630] = 0;
  if ( this[23] && !v89 )
    this[905634] = 0;
  if ( !this[905629] && v84 )
    this[905629] = v84;
  v85 = this[230508];
  while ( v85 )
  {
    v19 = *(int **)(v85 + 4);
    if ( v19 )
    {
      v20 = *((_WORD *)v19 + 8);
      if ( v20 != 16 && v20 != 22 && v20 != 63 && v20 != 330 && v20 != 331 && v20 != 332 && v20 != 333 && v20 != 334 )
      {
        if ( v20 == 9 || v20 == 53 || v20 == 336 )
        {
          (*(void (__thiscall **)(int *, int))(*v19 + 12))(v19, a2);
          if ( *((_BYTE *)v19 + 4) != 2 )
          {
            v41 = v19[52];
            v42 = this[252697];
            if ( v41 >= v42 && v41 < v42 + 24 )
            {
              v43 = v19[53];
              v44 = this[252698];
              if ( v43 >= v44 && v43 < v44 + 24 )
              {
                v27 = 0;
                v28 = v41 + 24 * (v43 - v44) - v42;
                v45 = &this[400 * v28 + 189];
                while ( *v45 )
                {
                  ++v27;
                  ++v45;
                  if ( v27 >= 100 )
                    goto LABEL_131;
                }
                goto LABEL_130;
              }
            }
            goto LABEL_131;
          }
        }
        else if ( v20 == 72 || v20 == 73 || v20 == 74 )
        {
          (*(void (__thiscall **)(int *, int))(*v19 + 12))(v19, a2);
          if ( *((_BYTE *)v19 + 4) != 2 )
          {
            v36 = v19[52];
            v37 = this[252697];
            if ( v36 >= v37 && v36 < v37 + 24 )
            {
              v38 = v19[53];
              v39 = this[252698];
              if ( v38 >= v39 && v38 < v39 + 24 )
              {
                v27 = 0;
                v28 = v36 + 24 * (v38 - v39) - v37;
                v40 = &this[400 * v28 + 189];
                while ( *v40 )
                {
                  ++v27;
                  ++v40;
                  if ( v27 >= 100 )
                    goto LABEL_131;
                }
                goto LABEL_130;
              }
            }
            goto LABEL_131;
          }
        }
        else
        {
          v21 = *v19;
          v22 = *(_DWORD *)(v85 + 4);
          if ( v20 == 140 )
          {
            (*(void (__thiscall **)(int, int))(v21 + 12))(v22, a2);
            if ( *((_BYTE *)v19 + 4) != 2 )
            {
              v23 = v19[52];
              v24 = this[252697];
              if ( v23 >= v24 && v23 < v24 + 24 )
              {
                v25 = v19[53];
                v26 = this[252698];
                if ( v25 >= v26 && v25 < v26 + 24 )
                {
                  v27 = 0;
                  v28 = v23 + 24 * (v25 - v26) - v24;
                  v29 = &this[400 * v28 + 189];
                  while ( *v29 )
                  {
                    ++v27;
                    ++v29;
                    if ( v27 >= 100 )
                      goto LABEL_131;
                  }
                  goto LABEL_130;
                }
              }
              goto LABEL_131;
            }
          }
          else
          {
            (*(void (__thiscall **)(int, int))(v21 + 12))(v22, a2);
            if ( *((_BYTE *)v19 + 4) != 2 )
            {
              v31 = v19[52];
              v32 = this[252697];
              if ( v31 >= v32 && v31 < v32 + 24 )
              {
                v33 = v19[53];
                v34 = this[252698];
                if ( v33 >= v34 && v33 < v34 + 24 )
                {
                  v27 = 0;
                  v28 = v31 + 24 * (v33 - v34) - v32;
                  v35 = &this[400 * v28 + 189];
                  while ( *v35 )
                  {
                    ++v27;
                    ++v35;
                    if ( v27 >= 100 )
                      goto LABEL_131;
                  }
                  goto LABEL_130;
                }
              }
              goto LABEL_131;
            }
          }
        }
LABEL_79:
        v30 = *(_DWORD *)(v85 + 12);
        (*(void (__thiscall **)(_DWORD *, int))(this[230507] + 20))(this + 230507, v85);
        v85 = v30;
        (*(void (__thiscall **)(int *, int))*v19)(v19, 1);
        continue;
      }
      v46 = v19[52];
      v19[73] = 1;
      v47 = this[252697];
      if ( v46 >= v47 && v46 < v47 + 24 )
      {
        v48 = v19[53];
        v49 = this[252698];
        if ( v48 >= v49 && v48 < v49 + 24 )
        {
          v50 = 0;
          v51 = &this[400 * v46 + 189 + 400 * (24 * (v48 - v49) - v47)];
          while ( 1 )
          {
            v52 = *v51;
            if ( *v51 )
            {
              if ( !*(_BYTE *)(v52 + 4)
                && *(_WORD *)(v52 + 16) == *((_WORD *)v19 + 8)
                && *(_DWORD *)(v52 + 176) == v19[44]
                && *(_DWORD *)(v52 + 180) == v19[45] )
              {
                break;
              }
            }
            ++v50;
            ++v51;
            if ( v50 >= 100 )
              goto LABEL_119;
          }
          v19[73] = 0;
        }
      }
LABEL_119:
      (*(void (__thiscall **)(int *, int))(*v19 + 12))(v19, a2);
      if ( *((_BYTE *)v19 + 4) == 2 )
        goto LABEL_79;
      v53 = v19[52];
      v54 = this[252697];
      if ( v53 < v54 )
        goto LABEL_131;
      if ( v53 >= v54 + 24 )
        goto LABEL_131;
      v55 = v19[53];
      v56 = this[252698];
      if ( v55 < v56 || v55 >= v56 + 24 )
        goto LABEL_131;
      if ( v19[73] )
      {
        v27 = 0;
        v28 = v53 + 24 * (v55 - v56) - v54;
        v57 = &this[400 * v28 + 189];
        while ( *v57 )
        {
          ++v27;
          ++v57;
          if ( v27 >= 100 )
            goto LABEL_131;
        }
LABEL_130:
        this[400 * v28 + 189 + v27] = v19;
LABEL_131:
        v85 = *(_DWORD *)(v85 + 12);
        continue;
      }
      v85 = *(_DWORD *)(v85 + 12);
    }
  }
  v58 = this[230514];
  v86 = v58;
  if ( v58 )
  {
    while ( 1 )
    {
      v59 = *(_DWORD **)(v58 + 4);
      if ( v59 )
      {
        if ( !dword_4AF2AC(v91, v59[1], v59[2]) )
        {
          v60 = *(_DWORD *)(v58 + 12);
          (*(void (__thiscall **)(_DWORD *, int))(this[230513] + 20))(this + 230513, v58);
          v86 = v60;
          MEMORY[0x4939F6](v59);
          goto LABEL_151;
        }
        v61 = v59[1];
        v62 = this[252697];
        if ( v61 >= v62 && v61 < v62 + 24 )
        {
          v63 = v59[2];
          v64 = this[252698];
          if ( v63 >= v64 && v63 < v64 + 24 )
          {
            v65 = 0;
            v66 = v61 + 24 * (v63 - v64) - v62;
            v67 = &this[400 * v66 + 289];
            while ( *v67 )
            {
              ++v65;
              ++v67;
              if ( v65 >= 100 )
                goto LABEL_148;
            }
            this[400 * v66 + 289 + v65] = v59;
          }
        }
LABEL_148:
        v58 = v86;
      }
      if ( !v58 )
        break;
      v86 = *(_DWORD *)(v58 + 12);
LABEL_151:
      if ( !v86 )
        break;
      v58 = v86;
    }
  }
  v87 = this[230520];
  if ( v87 )
  {
    v68 = this[230520];
    while ( 1 )
    {
      v69 = *(_DWORD **)(v68 + 4);
      if ( !v69 )
        goto LABEL_166;
      if ( (unsigned int)(dword_4AF2FC(v80, v81, v82, v83, v87) - v69[3]) <= v69[4] )
        break;
      v70 = *(_DWORD *)(v68 + 12);
      (*(void (__thiscall **)(_DWORD *, int))(this[230519] + 20))(this + 230519, v68);
      v68 = v70;
      v87 = v70;
      MEMORY[0x4939F6](v69);
LABEL_168:
      if ( !v68 )
        return;
    }
    v71 = v69[5];
    v72 = this[252697];
    if ( v71 >= v72 && v71 < v72 + 24 )
    {
      v73 = v69[6];
      v74 = this[252698];
      if ( v73 >= v74 && v73 < v74 + 24 )
      {
        v75 = 0;
        v76 = v71 + 24 * (v73 - v74) - v72;
        v77 = &this[400 * v76 + 389];
        while ( *v77 )
        {
          ++v75;
          ++v77;
          if ( v75 >= 100 )
            goto LABEL_166;
        }
        this[400 * v76 + 389 + v75] = v69;
      }
    }
LABEL_166:
    if ( !v87 )
      return;
    v68 = *(_DWORD *)(v87 + 12);
    v87 = v68;
    goto LABEL_168;
  }
}
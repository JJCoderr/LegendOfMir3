int __thiscall sub_4495D0(int this, __int16 a2, int a3, int a4, int a5, int a6, int a7, int a8, __int16 a9)
{
  __int16 v10; // ax
  int v12; // edi
  int v13; // eax
  int v14; // ecx
  _DWORD *v15; // ebx
  int v16; // esi
  int v17; // ecx
  int v18; // eax
  signed int v19; // eax
  double v20; // st7
  int v21; // eax
  unsigned __int8 v22; // al
  int v23; // edx
  int v24; // edx
  int v25; // eax
  int v26; // edx
  int v27; // edx
  char v28; // al
  int v29; // edx
  int v30; // eax
  int v31; // eax
  int v32; // edx
  int v33; // eax
  unsigned __int16 v34; // di
  __int16 v35; // ax
  unsigned __int16 v36; // ax
  BOOL v37; // ecx
  int v38; // eax
  int v39; // [esp+4h] [ebp-3Ch]
  int v40; // [esp+4h] [ebp-3Ch]
  int v41; // [esp+4h] [ebp-3Ch]
  _DWORD v42[8]; // [esp+20h] [ebp-20h] BYREF
  int v43; // [esp+4Ch] [ebp+Ch]
  float v44; // [esp+4Ch] [ebp+Ch]
  int v45; // [esp+60h] [ebp+20h]

  if ( a3 == a5 )
  {
    v10 = a2;
    if ( a4 == a6
      && (a2 == 13
       || a2 == 1
       || a2 == 5
       || a2 == 37
       || a2 == 39
       || a2 == 40
       || a2 == 41
       || a2 == 67
       || a2 == 416
       || a2 == 417
       || a2 == 418
       || a2 == 419
       || a2 == 150) )
    {
      return 0;
    }
  }
  else
  {
    v10 = a2;
  }
  v12 = sub_46EF60(dword_8FBA50, v10);
  if ( !v12 )
    return 0;
  *(_DWORD *)(this + 24) = 0;
  if ( a8 )
  {
    if ( a8 == *(_DWORD *)dword_823A58 )
    {
      *(_DWORD *)(this + 24) = &dword_7C1E20;
    }
    else
    {
      v13 = dword_59A9E0;
      if ( dword_59A9E0 )
      {
        while ( 1 )
        {
          v14 = *(_DWORD *)(v13 + 4);
          if ( v14 )
          {
            if ( **(_DWORD **)(v14 + 400440) == a8 )
              break;
          }
          v13 = *(_DWORD *)(v13 + 12);
          if ( !v13 )
            goto LABEL_28;
        }
        *(_DWORD *)(this + 24) = v14;
      }
    }
  }
LABEL_28:
  *(_DWORD *)(this + 192) = a3;
  v15 = (_DWORD *)(this + 188);
  *(_DWORD *)(this + 196) = a4;
  sub_453B60(&dword_5B03B0, *(_DWORD *)(this + 192), a4, (_DWORD *)(this + 184), (int *)(this + 188));
  *(_DWORD *)(this + 208) = a5;
  *(_DWORD *)(this + 212) = a6;
  sub_453B60(&dword_5B03B0, *(_DWORD *)(this + 208), a6, (_DWORD *)(this + 200), (int *)(this + 204));
  *(_DWORD *)(this + 176) = *(_DWORD *)(this + 192);
  *(_DWORD *)(this + 180) = *(_DWORD *)(this + 196);
  *(_DWORD *)(this + 160) = *(_DWORD *)(this + 184);
  *(float *)(this + 168) = (float)*(int *)(this + 184);
  *(_DWORD *)(this + 164) = *(_DWORD *)(this + 188);
  *(float *)(this + 172) = (float)*(int *)(this + 188);
  *(_WORD *)(this + 296) = a9;
  *(_BYTE *)(this + 240) = -1;
  *(_BYTE *)(this + 241) = -1;
  *(_BYTE *)(this + 242) = -1;
  v16 = *(_DWORD *)(this + 184);
  v17 = *(_DWORD *)(this + 200);
  if ( v16 == v17 )
    v45 = 1;
  else
    v45 = abs32(v17 - v16);
  v18 = *(_DWORD *)(this + 204);
  if ( *v15 == v18 )
    v19 = 1;
  else
    v19 = abs32(v18 - *v15);
  v43 = v19;
  if ( !v45 )
    v45 = 1;
  if ( !v19 )
  {
    v19 = 1;
    v43 = 1;
  }
  v20 = flt_4AF94C / (double)v45;
  v44 = flt_4AF94C / (double)v43;
  if ( v45 <= v19 )
  {
    *(_DWORD *)(this + 272) = MEMORY[0x49FE1C]((double)(v17 - v16) * v44);
    v21 = MEMORY[0x49FE1C]((double)(*(_DWORD *)(this + 204) - *v15) * v44);
  }
  else
  {
    *(_DWORD *)(this + 272) = MEMORY[0x49FE1C](v20);
    v21 = MEMORY[0x49FE1C](v20);
  }
  *(_DWORD *)(this + 276) = v21;
  v22 = sub_453C00(*(_DWORD *)(this + 192), *(_DWORD *)(this + 196), a5, a6);
  v23 = a7;
  *(_BYTE *)(this + 288) = v22;
  *(_WORD *)(this + 16) = a2;
  *(_DWORD *)(this + 20) = a7;
  *(_BYTE *)(this + 4) = 0;
  *(_DWORD *)(this + 12) = 1;
  *(_DWORD *)(this + 8) = 0;
  *(_DWORD *)(this + 228) = 0;
  *(_BYTE *)(this + 289) = 0;
  *(_DWORD *)(this + 220) = 0;
  if ( (unsigned __int16)a2 > 0x30u )
  {
    if ( (unsigned __int16)a2 > 0x207u )
    {
      switch ( a2 )
      {
        case 520:
          v26 = *(_DWORD *)(this + 192);
          *(_DWORD *)(this + 8) = 1;
          v40 = *(_DWORD *)(this + 196);
          *(_DWORD *)(this + 12) = 0;
          *(_BYTE *)(this + 288) = sub_453D20(v26, v40, a5, a6);
          *(_DWORD *)(this + 292) = 0;
          goto LABEL_76;
        case 521:
        case 529:
        case 535:
        case 536:
        case 551:
        case 552:
        case 572:
        case 574:
        case 575:
        case 580:
        case 584:
        case 585:
        case 602:
        case 603:
        case 605:
        case 606:
          v23 = a7;
LABEL_68:
          if ( v23 )
            *(_BYTE *)(this + 288) = *(_BYTE *)(v23 + 121);
          *(_DWORD *)(this + 8) = 0;
          *(_DWORD *)(this + 12) = 1;
          *(_DWORD *)(this + 24) = 0;
          *(_DWORD *)(this + 292) = 0;
          goto LABEL_76;
        case 522:
        case 523:
        case 524:
        case 525:
        case 526:
        case 527:
        case 528:
        case 531:
        case 533:
        case 534:
        case 540:
        case 541:
        case 542:
        case 543:
        case 555:
        case 577:
        case 589:
LABEL_73:
          *(_DWORD *)(this + 8) = 0;
          *(_DWORD *)(this + 12) = 1;
          *(_BYTE *)(this + 288) = 0;
          *(_DWORD *)(this + 292) = 0;
          goto LABEL_76;
        case 538:
          goto LABEL_71;
        case 539:
        case 569:
        case 588:
          goto LABEL_72;
        case 544:
        case 545:
          *(_DWORD *)(this + 8) = 1;
          *(_DWORD *)(this + 12) = 0;
          *(_BYTE *)(this + 288) = 0;
          *(_DWORD *)(this + 292) = 0;
          goto LABEL_76;
        case 558:
        case 573:
        case 576:
        case 581:
        case 592:
          goto LABEL_63;
        case 578:
        case 590:
        case 597:
          v29 = *(_DWORD *)(this + 196);
          v30 = *(_DWORD *)(this + 192);
          *(_DWORD *)(this + 8) = 1;
          *(_DWORD *)(this + 12) = 0;
          *(_DWORD *)(this + 292) = 0;
          v28 = sub_453D20(v30, v29, a5, a6);
          goto LABEL_75;
        case 593:
        case 594:
        case 595:
        case 596:
          v27 = *(_DWORD *)(this + 192);
          *(_DWORD *)(this + 8) = 1;
          v41 = *(_DWORD *)(this + 196);
          *(_DWORD *)(this + 12) = 0;
          v28 = sub_453D20(v27, v41, a5, a6);
LABEL_75:
          *(_BYTE *)(this + 288) = v28;
          break;
        default:
          goto LABEL_76;
      }
      goto LABEL_76;
    }
    if ( (unsigned __int16)a2 >= 0x205u )
    {
LABEL_63:
      *(_DWORD *)(this + 8) = 1;
      *(_DWORD *)(this + 12) = 0;
      *(_DWORD *)(this + 292) = 0;
      goto LABEL_76;
    }
    if ( (unsigned __int16)a2 <= 0x7Bu )
    {
      if ( (unsigned __int16)a2 >= 0x7Au )
      {
LABEL_72:
        *(_DWORD *)(this + 8) = 0;
        *(_DWORD *)(this + 12) = 1;
        *(_BYTE *)(this + 288) = 0;
      }
      else
      {
        switch ( a2 )
        {
          case '5':
            goto LABEL_59;
          case '?':
          case '@':
          case 'B':
          case 'H':
          case 'I':
          case 'J':
          case 'M':
          case 'o':
            goto LABEL_71;
          case 'C':
          case 'q':
            goto LABEL_51;
          case 'U':
          case 'V':
          case 'W':
          case 'X':
          case 'Y':
          case 'Z':
          case '[':
          case '\\':
          case ']':
          case '^':
          case '_':
          case '`':
          case 'a':
          case 'b':
          case 'c':
          case 'd':
          case 'e':
LABEL_50:
            *(_DWORD *)(this + 24) = 0;
            goto LABEL_51;
          case 'f':
          case 'j':
          case 'p':
            goto LABEL_73;
          case 'h':
          case 'i':
          case 'k':
          case 'x':
            goto LABEL_72;
          case 'n':
            *(_BYTE *)(this + 288) = 0;
            *(_DWORD *)(this + 8) = 0;
            *(_DWORD *)(this + 12) = 0;
            *(_DWORD *)(this + 24) = 0;
            break;
          default:
            goto LABEL_76;
        }
      }
      goto LABEL_76;
    }
    if ( (unsigned __int16)a2 > 0x1A2u )
    {
      switch ( a2 )
      {
        case 419:
LABEL_61:
          *(_DWORD *)(this + 8) = 1;
          *(_DWORD *)(this + 12) = 0;
          *(_BYTE *)(this + 288) = 0;
          break;
        case 420:
        case 421:
          goto LABEL_72;
        case 422:
        case 500:
        case 501:
        case 502:
        case 503:
        case 505:
        case 506:
        case 508:
        case 509:
        case 510:
        case 511:
        case 512:
          goto LABEL_73;
        case 513:
        case 514:
        case 515:
        case 516:
          goto LABEL_68;
        default:
          goto LABEL_76;
      }
      goto LABEL_76;
    }
    if ( (unsigned __int16)a2 >= 0x1A0u )
    {
LABEL_51:
      *(_DWORD *)(this + 8) = 1;
      *(_DWORD *)(this + 12) = 0;
      goto LABEL_76;
    }
    if ( (unsigned __int16)a2 > 0x14Eu )
    {
      switch ( a2 )
      {
        case 335:
          goto LABEL_68;
        case 336:
        case 337:
LABEL_59:
          *(_BYTE *)(this + 288) = 0;
          *(_DWORD *)(this + 8) = 0;
          *(_DWORD *)(this + 12) = 1;
          *(_DWORD *)(this + 24) = 0;
          break;
        case 400:
        case 401:
        case 402:
        case 403:
        case 404:
        case 405:
        case 406:
        case 407:
          goto LABEL_73;
        default:
          goto LABEL_76;
      }
      goto LABEL_76;
    }
    if ( (unsigned __int16)a2 < 0x14Au )
    {
      switch ( a2 )
      {
        case 140:
          goto LABEL_71;
        case 150:
          goto LABEL_51;
        case 320:
        case 321:
        case 322:
        case 323:
          goto LABEL_72;
        default:
          goto LABEL_76;
      }
      goto LABEL_76;
    }
  }
  else if ( (unsigned __int16)a2 < 0x2Fu )
  {
    switch ( a2 )
    {
      case 1:
      case 5:
      case 13:
      case 39:
      case 41:
        goto LABEL_51;
      case 2:
      case 6:
      case 17:
      case 20:
      case 27:
      case 28:
      case 30:
      case 31:
      case 32:
        goto LABEL_72;
      case 9:
        goto LABEL_59;
      case 10:
        v24 = *(_DWORD *)(this + 192);
        *(_DWORD *)(this + 12) = 1;
        v39 = *(_DWORD *)(this + 196);
        *(_DWORD *)(this + 8) = 0;
        *(_BYTE *)(this + 288) = sub_453D20(v24, v39, a5, a6);
        *(_DWORD *)(this + 24) = 0;
        *(_DWORD *)(this + 208) = *(_DWORD *)(this + 192);
        v25 = *(_DWORD *)(this + 196);
        *(_DWORD *)(this + 212) = v25;
        sub_453B60(&dword_5B03B0, *(_DWORD *)(this + 208), v25, (_DWORD *)(this + 200), (int *)(this + 204));
        break;
      case 11:
      case 16:
      case 22:
      case 23:
      case 24:
      case 29:
      case 33:
        goto LABEL_71;
      case 14:
      case 15:
      case 19:
        goto LABEL_50;
      case 37:
      case 38:
      case 40:
        goto LABEL_61;
      default:
        goto LABEL_76;
    }
    goto LABEL_76;
  }
LABEL_71:
  *(_BYTE *)(this + 288) = 0;
  *(_DWORD *)(this + 8) = 0;
  *(_DWORD *)(this + 12) = 1;
  *(_DWORD *)(this + 24) = 0;
LABEL_76:
  v31 = 10 * *(unsigned __int8 *)(this + 288);
  *(_DWORD *)(this + 244) = *(_DWORD *)v12 + v31;
  v32 = v31 + *(_DWORD *)(v12 + 4);
  v33 = *(_DWORD *)(this + 244);
  *(_DWORD *)(this + 248) = v32;
  *(_DWORD *)(this + 252) = v33;
  *(_DWORD *)(this + 224) = *(unsigned __int16 *)(v12 + 8);
  *(_BYTE *)(this + 232) = *(_BYTE *)(v12 + 14);
  *(_BYTE *)(this + 233) = *(_BYTE *)(v12 + 15);
  *(_BYTE *)(this + 234) = *(_BYTE *)(v12 + 16);
  *(_BYTE *)(this + 235) = *(_BYTE *)(v12 + 17);
  *(_BYTE *)(this + 236) = *(_BYTE *)(v12 + 18);
  *(_BYTE *)(this + 237) = *(_BYTE *)(v12 + 19);
  *(_BYTE *)(this + 238) = *(_BYTE *)(v12 + 20);
  *(_BYTE *)(this + 239) = *(_BYTE *)(v12 + 21);
  *(_BYTE *)(this + 240) = *(_BYTE *)(v12 + 22);
  *(_BYTE *)(this + 241) = *(_BYTE *)(v12 + 23);
  *(_BYTE *)(this + 242) = *(_BYTE *)(v12 + 24);
  *(_BYTE *)(this + 256) = *(_BYTE *)(v12 + 25);
  *(_BYTE *)(this + 257) = *(_BYTE *)(v12 + 26);
  *(_BYTE *)(this + 258) = *(_BYTE *)(v12 + 27);
  v34 = *(_WORD *)(v12 + 10);
  *(_WORD *)(this + 300) = v34;
  *(_DWORD *)(this + 156) = 328 * v34 + 5876332;
  v35 = *(_WORD *)(this + 300);
  if ( v35 == 81 || v35 == 137 || v35 == 149 )
  {
    *(_WORD *)(this + 298) = 0;
  }
  else if ( v35 == 129 || v35 == 138 || v35 == 140 || v35 == 143 )
  {
    *(_WORD *)(this + 298) = 1;
  }
  v42[0] = -1090519040;
  v42[1] = 1056964608;
  memset(&v42[2], 0, 12);
  v42[5] = -1082130432;
  v42[6] = 0;
  v42[7] = 0;
  v36 = *(_WORD *)(this + 16);
  qmemcpy((void *)(this + 28), v42, 0x20u);
  v42[0] = -1090519040;
  v42[1] = -1090519040;
  memset(&v42[2], 0, 12);
  v42[5] = -1082130432;
  v42[6] = 0;
  v42[7] = 1065353216;
  qmemcpy((void *)(this + 60), v42, 0x20u);
  v42[0] = 1056964608;
  v42[1] = 1056964608;
  memset(&v42[2], 0, 12);
  v42[5] = -1082130432;
  v42[6] = 1065353216;
  v42[7] = 0;
  qmemcpy((void *)(this + 92), v42, 0x20u);
  v42[0] = 1056964608;
  v42[1] = -1090519040;
  memset(&v42[2], 0, 12);
  v42[5] = -1082130432;
  v42[6] = 1065353216;
  v42[7] = 1065353216;
  qmemcpy((void *)(this + 124), v42, 0x20u);
  v37 = v36 == 22;
  if ( *(_DWORD *)(this + 12) )
    v38 = sub_4832D0(10 * v36 + 10002, *(_DWORD *)(this + 208), *(_DWORD *)(this + 212), v37);
  else
    v38 = sub_4832D0(10 * v36 + 10001, *(_DWORD *)(this + 176), *(_DWORD *)(this + 180), v37);
  *(_DWORD *)(this + 304) = v38;
  return 1;
}
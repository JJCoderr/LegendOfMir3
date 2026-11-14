_WORD *__thiscall sub_46F690(_WORD *this, unsigned __int16 a2)
{
  if ( a2 > 0x9Eu )
  {
    if ( a2 > 0x215u )
    {
      if ( a2 != 538 )
      {
LABEL_15:
        sub_4872D0(this + 111, 0, 4, 300);
        goto LABEL_17;
      }
    }
    else if ( a2 < 0x212u )
    {
      if ( a2 == 161 || a2 > 0xA4u && a2 <= 0xA6u )
      {
        sub_4872D0(this + 111, 0, 4, 250);
        goto LABEL_17;
      }
      goto LABEL_15;
    }
    sub_4872D0(this + 111, 0, 1, 200);
    goto LABEL_17;
  }
  if ( a2 == 158 )
  {
    sub_4872D0(this + 111, 0, 4, 200);
  }
  else
  {
    switch ( a2 )
    {
      case 0u:
      case 1u:
      case 2u:
      case 3u:
      case 4u:
      case 5u:
      case 6u:
      case 7u:
      case 8u:
      case 9u:
        sub_4872D0(this + 111, 0, 4, 250);
        goto LABEL_5;
      case 0x5Fu:
LABEL_5:
        sub_4872D0(this + 111, 0, 1, 250);
        break;
      case 0x91u:
      case 0x92u:
      case 0x93u:
        sub_4872D0(this + 111, 0, 4, 150);
        break;
      default:
        goto LABEL_15;
    }
  }
LABEL_17:
  sub_4872D0(this + 114, 80, 6, 90);
  switch ( a2 )
  {
    case 0u:
    case 1u:
    case 2u:
    case 0xCu:
    case 0x63u:
      sub_4872D0(this + 117, 160, 6, 100);
      break;
    case 3u:
    case 5u:
    case 6u:
    case 7u:
    case 8u:
    case 9u:
    case 0xFu:
    case 0x13u:
    case 0x5Du:
    case 0x60u:
    case 0x61u:
    case 0x62u:
    case 0x9Fu:
    case 0xA0u:
    case 0xA1u:
      sub_4872D0(this + 117, 160, 6, 140);
      break;
    case 4u:
    case 0xAu:
    case 0xBu:
    case 0xDu:
    case 0x11u:
    case 0x12u:
    case 0x5Eu:
    case 0x91u:
    case 0x92u:
    case 0x95u:
    case 0x9Cu:
    case 0x9Du:
    case 0xA5u:
    case 0xA6u:
      sub_4872D0(this + 117, 160, 6, 150);
      break;
    case 0xEu:
    case 0x10u:
    case 0x5Cu:
    case 0x93u:
    case 0x9Au:
    case 0x9Bu:
    case 0xA2u:
    case 0xA3u:
      sub_4872D0(this + 117, 160, 6, 120);
      break;
    case 0x21u:
    case 0x2Eu:
    case 0x52u:
      sub_4872D0(this + 117, 160, 6, 90);
      break;
    case 0x5Au:
      sub_4872D0(this + 117, 160, 6, 160);
      break;
    case 0x5Fu:
    case 0x78u:
      sub_4872D0(this + 117, 160, 10, 90);
      break;
    case 0xA4u:
      sub_4872D0(this + 117, 160, 9, 120);
      break;
    case 0xBAu:
      sub_4872D0(this + 117, 400, 8, 90);
      break;
    default:
      sub_4872D0(this + 117, 160, 6, 130);
      break;
  }
  if ( a2 > 0x9Du )
  {
    if ( a2 > 0xA6u )
    {
      if ( a2 < 0x212u || a2 > 0x215u && a2 != 538 )
      {
LABEL_42:
        sub_4872D0(this + 120, 240, 2, 180);
        goto LABEL_43;
      }
    }
    else if ( a2 < 0xA5u && a2 != 158 )
    {
      goto LABEL_36;
    }
    sub_4872D0(this + 120, 240, 2, 200);
    goto LABEL_43;
  }
  if ( a2 >= 0x9Au )
  {
LABEL_36:
    sub_4872D0(this + 120, 240, 2, 300);
    goto LABEL_43;
  }
  switch ( a2 )
  {
    case 0x74u:
    case 0x75u:
    case 0x76u:
      sub_4872D0(this + 120, 240, 3, 150);
      break;
    case 0x91u:
    case 0x92u:
    case 0x93u:
      sub_4872D0(this + 120, 240, 2, 150);
      break;
    case 0x95u:
      goto LABEL_36;
    default:
      goto LABEL_42;
  }
LABEL_43:
  if ( a2 <= 0x99u )
  {
    if ( a2 != 153 )
    {
      switch ( a2 )
      {
        case 8u:
        case 0x56u:
        case 0x5Eu:
        case 0x69u:
          sub_4872D0(this + 123, 320, 10, 130);
          break;
        case 0x1Eu:
        case 0x25u:
          sub_4872D0(this + 123, 320, 4, 220);
          break;
        case 0x68u:
          sub_4872D0(this + 123, 320, 20, 120);
          break;
        case 0x7Fu:
          goto LABEL_63;
        case 0x8Du:
          sub_4872D0(this + 123, 720, 20, 120);
          break;
        case 0x91u:
        case 0x92u:
        case 0x93u:
          goto LABEL_54;
        case 0x95u:
          goto LABEL_50;
        default:
          goto LABEL_65;
      }
      goto LABEL_67;
    }
LABEL_63:
    sub_4872D0(this + 123, 320, 9, 120);
    goto LABEL_67;
  }
  if ( a2 > 0xB3u )
  {
    if ( a2 > 0x219u )
    {
      if ( a2 != 538 )
        goto LABEL_65;
    }
    else
    {
      if ( a2 >= 0x216u )
        goto LABEL_63;
      if ( a2 < 0x212u )
      {
        if ( a2 >= 0xBBu && a2 <= 0xBDu )
          goto LABEL_63;
LABEL_65:
        sub_4872D0(this + 123, 320, 10, 140);
        goto LABEL_67;
      }
    }
    sub_4872D0(this + 123, 320, 8, 120);
    goto LABEL_67;
  }
  if ( a2 == 179 )
    goto LABEL_63;
  switch ( a2 )
  {
    case 0x9Au:
    case 0x9Bu:
LABEL_54:
      sub_4872D0(this + 123, 320, 10, 95);
      break;
    case 0x9Cu:
    case 0x9Du:
      sub_4872D0(this + 123, 320, 10, 145);
      break;
    case 0x9Eu:
    case 0xA5u:
    case 0xA6u:
      sub_4872D0(this + 123, 320, 10, 150);
      break;
    case 0x9Fu:
    case 0xA0u:
    case 0xA1u:
      sub_4872D0(this + 123, 320, 10, 110);
      break;
    case 0xA2u:
    case 0xA3u:
    case 0xA4u:
LABEL_50:
      sub_4872D0(this + 123, 320, 10, 120);
      break;
    default:
      goto LABEL_65;
  }
LABEL_67:
  switch ( a2 )
  {
    case 0x9Fu:
    case 0xA0u:
      sub_4872D0(this + 126, 400, 6, 140);
      break;
    case 0xA4u:
      sub_4872D0(this + 126, 400, 9, 120);
      break;
    default:
      sub_4872D0(this + 126, 400, 6, 150);
      break;
  }
  switch ( a2 )
  {
    case 0xAu:
    case 0xB5u:
      sub_4872D0(this + 129, 400, 6, 150);
      break;
    case 0xDu:
    case 0x9Du:
    case 0xB6u:
    case 0xB9u:
      sub_4872D0(this + 129, 480, 6, 150);
      break;
    case 0x8Bu:
    case 0x9Eu:
      sub_4872D0(this + 129, 480, 6, 200);
      break;
    case 0x8Du:
      sub_4872D0(this + 129, 480, 6, 120);
      break;
    case 0xA1u:
      sub_4872D0(this + 129, 480, 6, 140);
      break;
    case 0xA4u:
    case 0xBAu:
      sub_4872D0(this + 129, 480, 8, 150);
      break;
    default:
      sub_4872D0(this + 129, 480, 10, 150);
      break;
  }
  if ( a2 == 141 )
    sub_4872D0(this + 132, 560, 6, 120);
  else
    sub_4872D0(this + 132, 560, 10, 150);
  if ( a2 > 0x55u )
  {
    if ( a2 > 0x215u )
    {
      if ( a2 != 538 )
      {
LABEL_85:
        sub_4872D0(this + 135, 640, 10, 120);
        goto LABEL_95;
      }
    }
    else if ( a2 < 0x212u )
    {
      if ( a2 == 90 )
      {
        sub_4872D0(this + 135, 640, 10, 100);
        goto LABEL_95;
      }
      goto LABEL_85;
    }
    sub_4872D0(this + 135, 640, 7, 120);
    goto LABEL_95;
  }
  if ( a2 == 85 )
  {
    sub_4872D0(this + 135, 640, 20, 120);
  }
  else
  {
    switch ( a2 )
    {
      case 'S':
      case 'T':
        sub_4872D0(this + 135, 640, 6, 120);
        break;
      default:
        goto LABEL_85;
    }
  }
LABEL_95:
  sub_4872D0(this + 138, 720, 6, 150);
  return sub_4872D0(this + 141, 880, 1, 500);
}
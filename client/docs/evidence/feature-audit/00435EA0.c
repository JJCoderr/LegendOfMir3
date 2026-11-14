#19 *__thiscall sub_435820(_DWORD *this, int a2)
{
  int v3; // eax
  void (__stdcall *v4)(_DWORD *, int, int, int, int); // edi
  int v5; // eax
  _DWORD *v6; // ebp
  int v7; // ebx
  int v8; // eax
  _DWORD *v9; // ebp
  int v10; // ebx
  const char *v11; // eax
  const char *v12; // eax
  const char *v13; // eax
  const char *v14; // eax
  const char *v15; // eax
  const char *v16; // eax
  const char *v17; // eax
  const char *v18; // eax
  const char *v19; // eax
  const char *v20; // eax
  const char *v21; // eax
  const char *v22; // eax
  const char *v23; // eax
  const char *v24; // eax
  const char *v25; // eax
  const char *v26; // eax
  const char *v27; // eax
  const char *v28; // eax
  const char *v29; // eax
  const char *v30; // eax
  const char *v31; // eax
  const char *v32; // eax
  const char *v33; // eax
  const char *v34; // eax
  const char *v35; // eax
  const char *v36; // eax
  int v37; // ebx
  _DWORD *v38; // ebp

  this[7] = a2 + 22960;
  dword_4AF234(dword_8FB92C, dword_8FCD9C + 185, dword_8FCDA0 + 575, 344, 15, 1);
  v3 = sub_491C70(1160);
  v4 = (void (__stdcall *)(_DWORD *, int, int, int, int))dword_4AF2A8;
  if ( v3 )
    dword_4AF2A8(this + 791, 0, 601 - *(__int16 *)(*(_DWORD *)(this[7] + 60) + 2), **(__int16 **)(this[7] + 60), 600);
  v4(this + 887, 189, 492, 530, 566);
  v4(this + 891, 189, 320, 618, 566);
  v4(this + 909, 176, 297, 635, 552);
  v5 = 492;
  v6 = this + 811;
  do
  {
    v7 = v5 + 15;
    v4(v6, 189, v5, 530, v5 + 15);
    v5 = v7;
    v6 += 4;
  }
  while ( v7 < 567 );
  v8 = 320;
  v9 = this + 831;
  do
  {
    v10 = v8 + 15;
    v4(v9, 189, v8, 530, v8 + 15);
    v8 = v10;
    v9 += 4;
  }
  while ( v10 < 530 );
  v4(this + 795, 33, 475, 69, 563);
  v4(this + 799, 73, 475, 109, 563);
  v4(this + 803, 148, 490, 160, 586);
  v4(this + 807, 161, 490, 173, 586);
  v4(this + 7324, 535, 486, 616, 580);
  v11 = (const char *)sub_480550(2600);
  sub_41FA00((int)(this + 5619), this[7], 1170, 1170, this[791] + 637, this[792] + 6, v11, 1, -1, 0);
  v12 = (const char *)sub_480550(2601);
  sub_41FA00((int)(this + 5664), this[7], 1172, 1172, this[791] + 637, this[792] + 40, v12, 1, -1, 0);
  v13 = (const char *)sub_480550(2602);
  sub_41FA00((int)(this + 5709), this[7], 1174, 1174, this[791] + 637, this[792] + 75, v13, 1, -1, 0);
  v14 = (const char *)sub_480550(2603);
  sub_41FA00((int)(this + 5754), this[7], 1176, 1176, this[791] + 7, this[792] + 72, v14, 1, -1, 0);
  v15 = (const char *)sub_480550(2604);
  sub_41FA00((int)(this + 5799), this[7], 1178, 1178, this[791] + 108, this[792] + 72, v15, 1, -1, 0);
  v16 = (const char *)sub_480550(2605);
  sub_41FA00((int)(this + 5844), this[7], 1180, 1181, this[791] + 706, this[792] + 8, v16, 1, -1, 1);
  v4(this + 5845, 708, 476, 742, 495);
  v17 = (const char *)sub_480550(2606);
  sub_41FA00((int)(this + 5889), this[7], 1182, 1183, this[791] + 674, this[792] + 11, v17, 1, -1, 1);
  v4(this + 5890, 674, 475, 703, 500);
  v18 = (const char *)sub_480550(2607);
  sub_41FA00((int)(this + 5934), this[7], 1184, 1185, this[791] + 739, this[792] + 33, v18, 1, -1, 1);
  v4(this + 5935, 740, 502, 768, 531);
  v19 = (const char *)sub_480550(2608);
  sub_41FA00((int)(this + 5979), this[7], 1186, 1187, this[791] + 742, this[792] + 11, v19, 1, -1, 1);
  v4(this + 5980, 742, 478, 777, 502);
  v20 = (const char *)sub_480550(2609);
  sub_41FA00((int)(this + 6024), this[7], 1188, 1189, this[791] + 706, this[792] + 72, v20, 1, -1, 1);
  v4(this + 6025, 709, 539, 744, 559);
  v21 = (const char *)sub_480550(2610);
  sub_41FA00((int)(this + 6069), this[7], 1190, 1191, this[791] + 742, this[792] + 61, v21, 1, -1, 1);
  v4(this + 6070, 744, 531, 780, 555);
  v22 = (const char *)sub_480550(2611);
  sub_41FA00((int)(this + 6114), this[7], 1192, 1193, this[791] + 674, this[792] + 61, v22, 1, -1, 1);
  v4(this + 6115, 675, 532, 709, 554);
  v23 = (const char *)sub_480550(2612);
  sub_41FA00((int)(this + 6159), this[7], 1194, 1195, this[791] + 682, this[792] + 33, v23, 1, -1, 1);
  v4(this + 6160, 682, 504, 712, 531);
  v24 = (const char *)sub_480550(2613);
  sub_41FA00((int)(this + 6204), this[7], 1196, 1197, this[791] + 711, this[792] + 31, v24, 1, -1, 1);
  v4(this + 6205, 712, 498, 740, 535);
  v25 = (const char *)sub_480550(2614);
  sub_41FA00((int)(this + 6249), this[7], 1168, 1168, this[791] + 619, this[792] + 5, v25, 1, -1, 1);
  v4(this + 6250, 619, 472, 632, 489);
  v26 = (const char *)sub_480550(2615);
  sub_41FA00((int)(this + 6294), this[7], 1205, 1205, this[791] + 150, this[792] + 2, v26, 1, -1, 0);
  v4(this + 6295, 150, this[792] + 2, 174, this[792] + 17);
  v27 = (const char *)sub_480550(2616);
  sub_41FA00((int)(this + 6339), this[7], 1330, 1330, this[791] + 186, this[792] + 90, v27, 1, -1, 0);
  v4(this + 6340, 186, this[792] + 90, 218, this[792] + 122);
  v28 = (const char *)sub_480550(2617);
  sub_41FA00((int)(this + 6384), this[7], 1333, 1333, this[791] + 226, this[792] + 90, v28, 1, -1, 0);
  v4(this + 6385, 226, this[792] + 90, 258, this[792] + 122);
  v29 = (const char *)sub_480550(2618);
  sub_41FA00((int)(this + 6429), this[7], 1335, 1335, this[791] + 265, this[792] + 89, v29, 1, -1, 0);
  v4(this + 6430, 265, this[792] + 89, 297, this[792] + 121);
  v30 = (const char *)sub_480550(2619);
  sub_41FA00((int)(this + 6474), this[7], 1337, 1337, this[791] + 305, this[792] + 89, v30, 1, -1, 0);
  v4(this + 6475, 305, this[792] + 89, 337, this[792] + 121);
  v31 = (const char *)sub_480550(2620);
  sub_41FA00((int)(this + 6519), this[7], 1338, 1338, this[791] + 346, this[792] + 89, v31, 1, -1, 0);
  v4(this + 6520, 346, this[792] + 89, 378, this[792] + 121);
  v32 = (const char *)sub_480550(2621);
  sub_41FA00((int)(this + 6564), this[7], 1340, 1340, this[791] + 386, this[792] + 89, v32, 1, -1, 0);
  v4(this + 6565, 386, this[792] + 89, 418, this[792] + 121);
  v33 = (const char *)sub_480550(2643);
  sub_41FA00((int)(this + 6609), this[7], 1342, 1342, this[791] + 426, this[792] + 89, v33, 1, -1, 0);
  v4(this + 6610, 426, this[792] + 89, 458, this[792] + 121);
  v34 = (const char *)sub_480550(2645);
  sub_41FA00((int)(this + 6654), this[7], 1570, 1572, this[791] + 757, this[792] - 20, v34, 1, 1570, 1);
  v35 = (const char *)sub_480550(4622);
  sub_41FA00((int)(this + 6699), this[7], 2072, 2074, this[791] + 757, this[792] - 20, v35, 1, 2072, 1);
  v36 = (const char *)sub_480550(4715);
  sub_41FA00((int)(this + 6744), this[7], 2096, 2097, this[791] + 757, this[792] - 20, v36, 1, 2096, 1);
  sub_41FE80((int)(this + 6789), this[7], 1207, 5, 12, 43, 12, 0);
  sub_41FE80((int)(this + 6808), this[7], 1207, 14, 12, 203, 12, 0);
  sub_43FA00((_BYTE *)this + 29440, 0, this[7], 1220, 518, 0, 282, 466, 1);
  sub_470C50((_BYTE *)this + 174924, 1, this[7], 1280, 0, 0, 328, 466, 1);
  sub_476ED0((_BYTE *)this + 216772, 2, this[7], 1260, 0, 0, 292, 224, 0);
  sub_419EE0(16, this[7], 1210, 73, 413, 280, 48, 1);
  sub_462000(this + 87245, 12, this[7], 1290, 145, 30, 510, 468, 1, 3);
  sub_4327E0(this + 74535, 4, this[7], 1300, 102, 22, 594, 444, 1);
  sub_45EDA0(this + 89422, 15, this[7], 1326, 107, 110, 584, 252, 0, 3);
  sub_45FDD0((_BYTE *)this + 346860, 9, this[7], 1350, 0, 0, 386, 160, 0);
  sub_431B80(this + 76054, 6, this[7], 1360, 272, 123, 258, 244, 1);
  sub_41C860(3, this[7], 1390, 0, 0, 494, 330, 1);
  sub_434700(13, this[7], 1400, 0, 0, 276, 322, 1);
  sub_46BCE0(11, this[7], 1420, 0, 0, 340, 440, 1);
  sub_47A4B0(7, this[7], 1288, 516, 0, 284, 348, 1);
  sub_44E750((char *)this + 353908, 14, this[7], 1620, 348, 0, 360, 488, 1, 3);
  sub_46E5C0(this + 94298, 18, this[7], 1590, 100, 100, 194, 285, 1);
  sub_454410(19, this[7], 1890, 60, 20, 720, 440, 0, 3);
  sub_4567A0(this + 95812, 20, this[7], 1910, 0, 10, 408, 310, 0, 3);
  sub_457F50(this + 96762, 21, this[7], 1960, 0, 0, 440, 310, 0, 3);
  sub_41E680(this + 99355, 22, this[7], 2090, 0, 0, 580, 404, 0, 3);
  sub_41B1A0(8, this[7], 350, 114, 76, 572, 388, 1, 0);
  v37 = 279;
  v38 = this + 927;
  do
  {
    v4(v38, v37, 432, v37 + 38, 470);
    v37 += 40;
    v38 += 4;
  }
  while ( v37 < 519 );
  sub_45CA90(dword_8FCC64);
  sub_45E240(dword_8FCC64);
  sub_437680(this);
  return sub_437DD0(this);
}
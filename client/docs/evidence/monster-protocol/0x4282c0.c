void __userpurge sub_4282C0(
        _DWORD *a1@<ecx>,
        int a2@<ebx>,
        unsigned int a3@<ebp>,
        int a4@<edi>,
        int a5@<esi>,
        _BYTE *a6)
{
  struct _EXCEPTION_REGISTRATION_RECORD *ExceptionList; // eax
  void *v7; // esp
  unsigned __int16 v9; // ax
  unsigned __int16 v10; // dx
  unsigned int v11; // eax
  __int16 v12; // bx
  unsigned __int16 v13; // dx
  _BYTE *v14; // edi
  char v15; // si
  int v16; // edi
  int v17; // ebx
  const char *v18; // eax
  const char *v19; // eax
  const char *v20; // eax
  int v21; // esi
  int v22; // eax
  const char *v23; // esi
  char *v24; // eax
  int m; // ebp
  const char *v26; // eax
  const char *v27; // eax
  const char *v28; // eax
  const char *v29; // eax
  char v30; // dl
  int v31; // eax
  char *v32; // eax
  char *v33; // esi
  _DWORD *v34; // eax
  _DWORD *v35; // eax
  char *v36; // eax
  int v37; // eax
  int v38; // esi
  char *v39; // eax
  char *v40; // edi
  _DWORD *v41; // eax
  _DWORD *v42; // eax
  char *v43; // eax
  char *v44; // esi
  _DWORD *v45; // eax
  int v46; // eax
  int v47; // esi
  char *v48; // eax
  char *v49; // eax
  int v50; // eax
  int v51; // esi
  char *v52; // eax
  char *v53; // eax
  int v54; // eax
  int v55; // esi
  char *v56; // eax
  int v57; // edx
  char *v58; // eax
  int v59; // eax
  int v60; // ecx
  int v61; // edx
  int v62; // edi
  _DWORD *v63; // esi
  char *v64; // eax
  int v65; // eax
  int v66; // ecx
  _DWORD *v67; // esi
  int v68; // edx
  char *v69; // eax
  int v70; // eax
  int v71; // ecx
  int v72; // edi
  int v73; // eax
  int v74; // esi
  char *v75; // eax
  int v76; // esi
  int v77; // ecx
  int v78; // eax
  int v79; // eax
  __int16 v80; // ax
  int v81; // edx
  char v82; // al
  _BYTE *v83; // ecx
  int v84; // eax
  int v85; // eax
  int v86; // eax
  int v87; // eax
  int v88; // ecx
  _DWORD *v89; // edx
  char *v90; // eax
  int v91; // eax
  char *v92; // eax
  char *v93; // esi
  int v94; // eax
  char *v95; // ecx
  char *v96; // edi
  char *v97; // esi
  int k; // edx
  _DWORD *v99; // esi
  int v100; // eax
  int v101; // eax
  char *v102; // eax
  char *v103; // edi
  __int16 *v104; // eax
  const void *v105; // esi
  int v106; // edx
  int v107; // eax
  int v108; // edx
  char *v109; // eax
  bool v110; // cc
  const char *v111; // eax
  const char *v112; // eax
  const char *v113; // eax
  const char *v114; // eax
  const char *v115; // eax
  int v116; // edx
  __int16 v117; // si
  int v118; // eax
  int v119; // eax
  int v120; // eax
  int v121; // esi
  char v122; // dl
  char v123; // al
  int v124; // ecx
  const char *v125; // eax
  int v126; // eax
  int v127; // ecx
  _DWORD *v128; // esi
  _DWORD *v129; // esi
  __int16 v130; // bx
  _DWORD *v131; // esi
  int v132; // eax
  int v133; // eax
  int v134; // edx
  int v135; // ecx
  _DWORD *v136; // ebp
  _WORD *v137; // ecx
  int v138; // ecx
  int v139; // ecx
  int v140; // edx
  const char *v141; // eax
  const char *v142; // eax
  int v143; // eax
  unsigned __int16 v144; // dx
  bool v145; // zf
  unsigned int v146; // eax
  unsigned int v147; // eax
  _BYTE *v148; // esi
  int v149; // ecx
  const char *v150; // eax
  int v151; // eax
  _DWORD *v152; // eax
  char *v153; // eax
  const char *v154; // eax
  int v155; // eax
  int v156; // eax
  int v157; // ecx
  int v158; // eax
  char *v159; // eax
  const char *v160; // esi
  char v161; // al
  const char *v162; // eax
  int v163; // edx
  const char *v164; // eax
  int v165; // eax
  char *v166; // edi
  char *i; // ebp
  const char *v168; // ebp
  char *v169; // edx
  _BYTE *v170; // ecx
  int v171; // eax
  int v172; // eax
  unsigned __int16 v173; // dx
  int v174; // eax
  int v175; // eax
  int v176; // eax
  unsigned int v177; // esi
  _DWORD *v178; // edi
  const char *v179; // eax
  const char *v180; // eax
  int v181; // eax
  int v182; // eax
  unsigned __int16 v183; // cx
  int v184; // esi
  int v185; // edi
  const char *v186; // eax
  _DWORD *v187; // eax
  int v188; // eax
  char *v189; // eax
  char *v190; // esi
  int v191; // edi
  int v192; // eax
  int v193; // eax
  char *v194; // eax
  unsigned int v195; // kr80_4
  char *v196; // eax
  char *v197; // edx
  char *v198; // edi
  char *v199; // ebp
  const char *v200; // esi
  char *v201; // esi
  const char *v202; // edi
  char *v203; // eax
  char *v204; // ebp
  char *v205; // eax
  int v206; // ebp
  int v207; // ecx
  int v208; // esi
  int v209; // ebp
  int v210; // eax
  char *v211; // eax
  _DWORD *v212; // eax
  _DWORD *v213; // eax
  char *v214; // eax
  _DWORD *v215; // eax
  char *v216; // eax
  _DWORD *v217; // esi
  int v218; // edx
  int v219; // edx
  _DWORD *v220; // eax
  int v221; // edx
  int v222; // esi
  int v223; // ebp
  int v224; // ecx
  int v225; // ebp
  int v226; // edi
  char *v227; // eax
  char *v228; // esi
  _DWORD *v229; // eax
  _DWORD *v230; // eax
  _DWORD *v231; // eax
  char *v232; // eax
  int v233; // eax
  int v234; // eax
  int v235; // eax
  int v236; // eax
  _DWORD *v237; // edi
  int v238; // ecx
  int v239; // eax
  int v240; // esi
  int v241; // edi
  const char *v242; // eax
  int v243; // eax
  _DWORD *v244; // esi
  char v245; // cl
  int v246; // eax
  int v247; // ecx
  int v248; // eax
  int v249; // esi
  int v250; // edi
  const char *v251; // eax
  int v252; // eax
  unsigned __int16 v253; // cx
  int v254; // edi
  int v255; // edi
  int v256; // eax
  unsigned __int16 v257; // dx
  int v258; // ecx
  unsigned __int16 v259; // cx
  unsigned int v260; // kr00_4
  int v261; // eax
  const char *v262; // eax
  int v263; // eax
  _DWORD *v264; // esi
  int v265; // eax
  char *v266; // eax
  unsigned int v267; // eax
  unsigned __int16 v268; // dx
  unsigned __int16 v269; // cx
  unsigned int v270; // ebp
  int v271; // edi
  int v272; // esi
  char *v273; // eax
  char *v274; // eax
  int v275; // edx
  _DWORD *v276; // eax
  char *v277; // ecx
  char *v278; // edi
  char *v279; // ebp
  char *v280; // eax
  char *v281; // esi
  char *v282; // eax
  char *v283; // esi
  char *v284; // edx
  int v285; // edi
  int v286; // esi
  char *v287; // ebp
  char *v288; // eax
  _DWORD *v289; // eax
  _DWORD *v290; // eax
  int v291; // ecx
  void *v292; // edi
  __int16 v293; // ax
  int v294; // esi
  int v295; // edi
  const char *v296; // eax
  int v297; // edx
  FILE *v298; // esi
  FILE *v299; // edi
  FILE *v300; // edi
  int v301; // ebp
  FILE *v302; // esi
  FILE *v303; // esi
  int v304; // edi
  char *v305; // ebp
  FILE *v306; // eax
  FILE *v307; // esi
  char v308; // cl
  int v309; // ebx
  int v310; // edx
  int *v311; // eax
  const char *v312; // eax
  int v313; // esi
  int v314; // eax
  _DWORD *v315; // edi
  char *v316; // eax
  char *v317; // ebp
  int v318; // eax
  int v319; // esi
  int v320; // ecx
  _DWORD *v321; // eax
  _DWORD *v322; // eax
  char *v323; // eax
  char *v324; // esi
  _DWORD *v325; // eax
  _DWORD *v326; // eax
  double v327; // st7
  _DWORD *v328; // edi
  char *v329; // eax
  char *v330; // ebp
  int v331; // eax
  int v332; // ecx
  void (__thiscall **v333)(_DWORD *, int); // edx
  _DWORD *v334; // eax
  _DWORD *v335; // eax
  int v336; // eax
  char *v337; // eax
  int v338; // eax
  int v339; // ecx
  int v340; // edx
  int v341; // eax
  _BYTE *v342; // edx
  __int16 v343; // si
  __int16 v344; // di
  __int16 v345; // ax
  int v346; // eax
  char *v347; // esi
  int v348; // edi
  _DWORD *v349; // edi
  int v350; // ecx
  _BYTE v351[3116]; // [esp-BE8h] [ebp-4448h] BYREF
  int v352; // [esp+44h] [ebp-381Ch]
  int v353; // [esp+48h] [ebp-3818h]
  unsigned int v354; // [esp+4Ch] [ebp-3814h]
  int v355; // [esp+50h] [ebp-3810h]
  int v356; // [esp+54h] [ebp-380Ch]
  _DWORD *v357; // [esp+58h] [ebp-3808h] BYREF
  __int64 v358; // [esp+5Ch] [ebp-3804h]
  int v359; // [esp+64h] [ebp-37FCh]
  char *j; // [esp+68h] [ebp-37F8h]
  int v361; // [esp+6Ch] [ebp-37F4h]
  __int16 *v362; // [esp+70h] [ebp-37F0h] BYREF
  int v363; // [esp+74h] [ebp-37ECh] BYREF
  unsigned int v364; // [esp+78h] [ebp-37E8h]
  char v365; // [esp+7Ch] [ebp-37E4h]
  char v366; // [esp+7Dh] [ebp-37E3h]
  char v367; // [esp+7Eh] [ebp-37E2h]
  char v368; // [esp+7Fh] [ebp-37E1h]
  float v369[5]; // [esp+80h] [ebp-37E0h] BYREF
  char v370; // [esp+94h] [ebp-37CCh]
  char v371[100]; // [esp+98h] [ebp-37C8h] BYREF
  char Str[260]; // [esp+FCh] [ebp-3764h] BYREF
  char v373[24]; // [esp+200h] [ebp-3660h] BYREF
  char v374[260]; // [esp+218h] [ebp-3648h] BYREF
  char v375[1308]; // [esp+31Ch] [ebp-3544h] BYREF
  char v376[260]; // [esp+838h] [ebp-3028h] BYREF
  char v377[260]; // [esp+93Ch] [ebp-2F24h] BYREF
  _BYTE v378[260]; // [esp+A40h] [ebp-2E20h] BYREF
  char v379[10240]; // [esp+B44h] [ebp-2D1Ch] BYREF
  char FileName[260]; // [esp+3344h] [ebp-51Ch] BYREF
  unsigned int v381; // [esp+3448h] [ebp-418h] BYREF
  __int64 v382; // [esp+344Ch] [ebp-414h]
  char v383[1024]; // [esp+3454h] [ebp-40Ch] BYREF
  struct _EXCEPTION_REGISTRATION_RECORD *v384; // [esp+3854h] [ebp-Ch]
  int (*v385)(); // [esp+3858h] [ebp-8h]
  int v386; // [esp+385Ch] [ebp-4h]

  v386 = -1;
  ExceptionList = NtCurrentTeb()->NtTib.ExceptionList;
  v385 = sub_4ACA36;
  v384 = ExceptionList;
  v7 = alloca(14336);
  v355 = a2;
  v354 = a3;
  v353 = a5;
  v352 = a4;
  if ( *a6 == 43 )
  {
    sub_427C50((int)a1, a6 + 1);
    return;
  }
  sub_47D890((int)&v363, a6);
  LOWORD(v364) = *((_WORD *)a1 + 2212405) ^ v364;
  HIBYTE(v9) = v368;
  LOBYTE(v9) = v366;
  HIBYTE(v10) = HIBYTE(v364);
  LOBYTE(v10) = BYTE1(v364);
  v11 = v10 | (v9 << 16);
  HIBYTE(v10) = *((_BYTE *)a1 + 4424809) ^ v364;
  HIBYTE(v12) = v365;
  LOBYTE(v10) = v363 ^ *((_BYTE *)&loc_438468 + (_DWORD)a1);
  LOWORD(v358) = v10;
  v357 = (_DWORD *)v11;
  LOBYTE(v13) = BYTE1(v363);
  HIBYTE(v13) = BYTE2(v364);
  LOBYTE(v12) = BYTE2(v363);
  WORD1(v358) = v13;
  v14 = (_BYTE *)v358;
  WORD2(v358) = v12;
  LOBYTE(v12) = HIBYTE(v363);
  HIBYTE(v12) = v367;
  HIWORD(v358) = v12;
  if ( (unsigned __int16)v358 > 0x2ADu )
  {
    if ( (unsigned __int16)v358 <= 0x328u )
    {
      if ( (unsigned __int16)v358 < 0x326u )
      {
        switch ( (__int16)v358 )
        {
          case 686:
            sub_4832D0(106, 0, 0, 0);
            a1[768870] = v357;
            return;
          case 687:
            sub_41E2D0(a1 + 749857);
            sub_41C7E0(a1 + 749857);
            if ( a1[749869] )
            {
              sub_43AA30(a1 + 695155, 3);
              (*(void (__thiscall **)(_DWORD *, _DWORD))(a1[749857] + 16))(a1 + 749857, 0);
            }
            return;
          case 700:
            if ( !a1[702527] )
              sub_43AB90(a1 + 695155, 0);
            if ( a1[749360] )
              sub_43AB90(a1 + 695155, 2);
            v148 = a1 + 702515;
            *(_DWORD *)&v351[3112] = 3;
LABEL_356:
            sub_441FD0(v148, v351[3112]);
            a1[738876] = v357;
LABEL_357:
            sub_4318D0(v148, 519, 0, 1);
            return;
          case 701:
            if ( !a1[738872] )
              goto LABEL_366;
            v177 = WORD1(v358);
            v178 = &a1[780 * *(_DWORD *)((char *)a1 + 2955497)];
            if ( WORD2(v358) )
            {
              v179 = (const char *)sub_480550(1447);
              MEMORY[0x49FA18](Str, v179, v178 + 702996, WORD2(v358));
            }
            else
            {
              v180 = (const char *)sub_480550(1448);
              MEMORY[0x49FA18](Str, v180, v178 + 702996);
            }
            sub_41C120(Str, 11861940);
            if ( WORD1(v358) )
              goto LABEL_464;
            qmemcpy(&v351[8], v178 + 702995, 0xC24u);
            v181 = sub_41AD30(dword_7B7D50);
            if ( v181 != -1 )
              sub_41AE40(v181);
            goto LABEL_467;
          case 702:
            *(_DWORD *)&v351[3112] = 0;
            *(_DWORD *)&v351[3108] = 0;
            *(_DWORD *)&v351[3104] = 0xFFFF;
            *(_DWORD *)&v351[3100] = -1;
            *(_DWORD *)&v351[3096] = -1;
            *(_DWORD *)&v351[3092] = 0;
            *(_DWORD *)&v351[3088] = 1449;
            goto LABEL_368;
          case 703:
            *(_DWORD *)&v351[3112] = 0;
            *(_DWORD *)&v351[3108] = 0;
            *(_DWORD *)&v351[3104] = 0xFFFF;
            *(_DWORD *)&v351[3100] = -1;
            *(_DWORD *)&v351[3096] = -1;
            *(_DWORD *)&v351[3092] = 0;
            *(_DWORD *)&v351[3088] = 1450;
LABEL_368:
            v154 = (const char *)sub_480550(*(int *)&v351[3088]);
            sub_420540(
              (int)(a1 + 902646),
              (int)dword_5A041C,
              0,
              0,
              v154,
              *(int *)&v351[3092],
              *(int *)&v351[3096],
              *(int *)&v351[3100],
              *(__int16 *)&v351[3104],
              *(int *)&v351[3108],
              *(int *)&v351[3112],
              v352,
              v353);
            goto LABEL_366;
          case 704:
            sub_4798D0(a6 + 16, HIWORD(v358));
            v145 = a1[749360] == 0;
LABEL_444:
            if ( v145 )
              goto LABEL_436;
            return;
          case 705:
            if ( WORD2(v358) )
            {
              v175 = sub_478670(a1 + 749348, a1[749856]);
              sub_441DD0((char *)a1 + 2810060, *(_DWORD *)(v175 + 66), WORD2(v358));
            }
            if ( WORD1(v358) )
            {
              v176 = sub_478670(a1 + 749348, a1[749856]);
              if ( v176 )
                *(_WORD *)(v176 + 75) = WORD1(v358);
            }
            else
            {
              sub_478750(a1 + 749348);
            }
            return;
          case 706:
            *(_DWORD *)&v351[3112] = 0;
            *(_DWORD *)&v351[3108] = 0;
            *(_DWORD *)&v351[3104] = 0xFFFF;
            *(_DWORD *)&v351[3100] = -1;
            *(_DWORD *)&v351[3096] = -1;
            *(_DWORD *)&v351[3092] = 0;
            *(_DWORD *)&v351[3088] = sub_480550(1445);
            *(_DWORD *)&v351[3084] = 0;
            *(_DWORD *)&v351[3080] = 2;
            goto LABEL_630;
          case 707:
            *(_DWORD *)&v351[3112] = 0;
            *(_DWORD *)&v351[3108] = 0;
            *(_DWORD *)&v351[3104] = 0xFFFF;
            *(_DWORD *)&v351[3100] = -1;
            *(_DWORD *)&v351[3096] = -1;
            *(_DWORD *)&v351[3092] = 0;
            *(_DWORD *)&v351[3088] = sub_480550(1446);
            *(_DWORD *)&v351[3084] = 0;
            *(_DWORD *)&v351[3080] = 2;
            goto LABEL_630;
          case 708:
            a1[1105653] = v11;
            return;
          case 709:
            v356 = 0;
            v379[sub_47D780(a6 + 16, (int)v379, 10240)] = 0;
            v201 = v379;
            v362 = (__int16 *)(a1 + 702515);
            j = (char *)(a1 + 738886);
            do
            {
              v202 = v201;
              v203 = MEMORY[0x49FFB0](v201, 47);
              *(_DWORD *)&v351[3104] = 47;
              *v203 = 0;
              v204 = v203 + 1;
              v205 = MEMORY[0x49FFB0](v203 + 1, *(int *)&v351[3104]);
              *(_DWORD *)&v351[3096] = v204;
              *v205 = 0;
              v201 = v205 + 1;
              v206 = MEMORY[0x49FAF5](*(_DWORD *)&v351[3096]);
              sub_440C00(v362, v206, v202);
              sub_4763B0(v206, v202);
              ++v356;
            }
            while ( v356 < HIWORD(v358) && v201 );
            return;
          case 710:
            v182 = dword_4AF2FC(v352, v353, v354, v355, v356);
            v183 = WORD1(v358);
            a1[701985] = v182;
            sub_45CDB0(v183 - 1);
            sub_45E370(WORD1(v358) - 1);
            if ( a1[702496] == 1 )
            {
              a1[702470] = 1;
              a1[702496] = 0;
            }
            if ( a1[702497] == 1 )
            {
              a1[702471] = 1;
              a1[702474] = 1;
              a1[702497] = 0;
            }
            return;
          case 711:
            v351[3112] = 2;
            a1[701985] = dword_4AF2FC(v352, v353, v354, v355, v356);
            v184 = sub_401430(v351[3112]);
            v185 = sub_401430(3);
            v186 = (const char *)sub_480550(1458);
            sub_4366D0(a1 + 695155, v184, v185, v186);
            a1[702470] = 0;
            a1[702496] = 0;
            if ( a1[702497] == 1 )
            {
              a1[702474] = 1;
              a1[702471] = 0;
              a1[702497] = 0;
            }
            return;
          case 712:
            v174 = sub_47D780(a6 + 16, (int)v379, 10240);
            *(_DWORD *)&v351[3112] = v379;
            v379[v174] = 0;
            sub_479C00(*(char **)&v351[3112]);
            if ( !a1[749360] )
            {
LABEL_436:
              sub_43AB90(a1 + 695155, 2);
              a1[749855] = v357;
            }
            return;
          case 713:
            *(_DWORD *)&v351[3112] = 0;
            *(_DWORD *)&v351[3108] = 0;
            *(_DWORD *)&v351[3104] = 0xFFFF;
            *(_DWORD *)&v351[3100] = -1;
            *(_DWORD *)&v351[3096] = -1;
            *(_DWORD *)&v351[3092] = 0;
            *(_DWORD *)&v351[3088] = 1440;
            a1[895877] = v11;
            *(_DWORD *)&v351[3088] = sub_480550(*(int *)&v351[3088]);
            *(_DWORD *)&v351[3084] = 0;
            *(_DWORD *)&v351[3080] = 2;
            goto LABEL_630;
          case 714:
            switch ( v11 )
            {
              case 1u:
                *(_DWORD *)&v351[3112] = 0;
                *(_DWORD *)&v351[3108] = 0;
                *(_DWORD *)&v351[3104] = 0xFFFF;
                *(_DWORD *)&v351[3100] = -1;
                *(_DWORD *)&v351[3096] = -1;
                *(_DWORD *)&v351[3092] = 0;
                *(_DWORD *)&v351[3088] = sub_480550(1441);
                *(_DWORD *)&v351[3084] = 0;
                *(_DWORD *)&v351[3080] = 2;
                goto LABEL_630;
              case 2u:
                *(_DWORD *)&v351[3112] = 0;
                *(_DWORD *)&v351[3108] = 0;
                *(_DWORD *)&v351[3104] = 0xFFFF;
                *(_DWORD *)&v351[3100] = -1;
                *(_DWORD *)&v351[3096] = -1;
                *(_DWORD *)&v351[3092] = 0;
                *(_DWORD *)&v351[3088] = sub_480550(1442);
                *(_DWORD *)&v351[3084] = 0;
                *(_DWORD *)&v351[3080] = 2;
                goto LABEL_630;
              case 3u:
                *(_DWORD *)&v351[3112] = 0;
                *(_DWORD *)&v351[3108] = 0;
                *(_DWORD *)&v351[3104] = 0xFFFF;
                *(_DWORD *)&v351[3100] = -1;
                *(_DWORD *)&v351[3096] = -1;
                *(_DWORD *)&v351[3092] = 0;
                *(_DWORD *)&v351[3088] = sub_480550(1443);
                *(_DWORD *)&v351[3084] = 0;
                *(_DWORD *)&v351[3080] = 2;
                goto LABEL_630;
              case 4u:
                *(_DWORD *)&v351[3112] = 0;
                *(_DWORD *)&v351[3108] = 0;
                *(_DWORD *)&v351[3104] = 0xFFFF;
                *(_DWORD *)&v351[3100] = -1;
                *(_DWORD *)&v351[3096] = -1;
                *(_DWORD *)&v351[3092] = 0;
                *(_DWORD *)&v351[3088] = sub_480550(1444);
                *(_DWORD *)&v351[3084] = 0;
                *(_DWORD *)&v351[3080] = 2;
                goto LABEL_630;
              default:
                return;
            }
            return;
          case 750:
            v193 = sub_47D780(a6 + 16, (int)v375, 1024);
            *(_DWORD *)&v351[3112] = 12074;
            *(_DWORD *)&v351[3108] = v375;
            v375[v193] = 0;
            v194 = MEMORY[0x49FFB0](*(const char **)&v351[3108], *(int *)&v351[3112]);
            if ( v194 )
            {
              *v194 = 0;
              v195 = strlen(v375) + 1;
              if ( (int)(v195 - 1) > 0 )
                v374[v195 + 258] = 0;
              memset(a1 + 794987, 0, 0x3Cu);
              MEMORY[0x49FA18](a1 + 794987, MEMORY[0x4B54AC], v194 + 1);
            }
            else
            {
              memset(a1 + 794987, 0, 0x3Cu);
            }
            v196 = MEMORY[0x49FFB0](v375, 47);
            v197 = (char *)a1 + 3583155;
            v198 = (char *)a1 + 3583155;
            v199 = (char *)a1 + 3583222;
            memset(v198, 0, 0x3Cu);
            memset(v199, 0, 0x3Cu);
            if ( v196 )
            {
              *v196 = 0;
              v200 = v196 + 1;
              v356 = strlen(v375);
              if ( (int)(v356 + strlen(v196 + 1)) > 40 )
                v200[-v356 + 40] = 0;
              memset(v197, 0, 0x3Cu);
              memset(v199, 0, 0x3Cu);
              MEMORY[0x49FA18](v197, "%s", v375);
              MEMORY[0x49FA18](v199, "%s", v200);
            }
            return;
          case 751:
            sub_47D780(a6 + 16, (int)v375, 1308);
            if ( !sub_42E740(a1 + 771462) )
              sub_43AB90(a1 + 695155, 7);
            sub_47AE30(v375);
            return;
          case 752:
          case 800:
          case 801:
            goto LABEL_876;
          case 753:
            if ( !sub_42E740(a1 + 769690) )
              sub_43AB90(a1 + 695155, 4);
            v379[sub_47D780(a6 + 16, (int)v379, 0x2000)] = 0;
            sub_433A30((int)(a1 + 769690), v379);
            return;
          case 754:
            *(_DWORD *)&v351[3112] = 0;
            *(_DWORD *)&v351[3108] = 0;
            *(_DWORD *)&v351[3104] = 0xFFFF;
            *(_DWORD *)&v351[3100] = -1;
            *(_DWORD *)&v351[3096] = -1;
            *(_DWORD *)&v351[3092] = 0;
            *(_DWORD *)&v351[3088] = 1473;
            goto LABEL_629;
          case 756:
          case 775:
          case 776:
            sub_434000(a1 + 769690, a6 + 16);
            return;
          case 757:
            sub_47CEB0(dword_8FCDD8);
            *(_DWORD *)&v351[3112] = 0;
            *(_DWORD *)&v351[3108] = 0;
            *(_DWORD *)&v351[3104] = 0xFFFF;
            *(_DWORD *)&v351[3100] = -1;
            *(_DWORD *)&v351[3096] = -1;
            *(_DWORD *)&v351[3092] = 0;
            *(_DWORD *)&v351[3088] = 1474;
            goto LABEL_629;
          case 758:
            switch ( v11 )
            {
              case 1u:
                goto LABEL_596;
              case 2u:
                *(_DWORD *)&v351[3112] = 0;
                *(_DWORD *)&v351[3108] = 0;
                *(_DWORD *)&v351[3104] = 0xFFFF;
                *(_DWORD *)&v351[3100] = -1;
                *(_DWORD *)&v351[3096] = -1;
                *(_DWORD *)&v351[3092] = 0;
                *(_DWORD *)&v351[3088] = 1476;
                goto LABEL_629;
              case 3u:
                *(_DWORD *)&v351[3112] = 0;
                *(_DWORD *)&v351[3108] = 0;
                *(_DWORD *)&v351[3104] = 0xFFFF;
                *(_DWORD *)&v351[3100] = -1;
                *(_DWORD *)&v351[3096] = -1;
                *(_DWORD *)&v351[3092] = 0;
                *(_DWORD *)&v351[3088] = 1477;
                goto LABEL_629;
              case 4u:
                *(_DWORD *)&v351[3112] = 0;
                *(_DWORD *)&v351[3108] = 0;
                *(_DWORD *)&v351[3104] = 0xFFFF;
                *(_DWORD *)&v351[3100] = -1;
                *(_DWORD *)&v351[3096] = -1;
                *(_DWORD *)&v351[3092] = 0;
                *(_DWORD *)&v351[3088] = 1478;
                goto LABEL_629;
              case 5u:
                *(_DWORD *)&v351[3112] = 0;
                *(_DWORD *)&v351[3108] = 0;
                *(_DWORD *)&v351[3104] = 0xFFFF;
                *(_DWORD *)&v351[3100] = -1;
                *(_DWORD *)&v351[3096] = -1;
                *(_DWORD *)&v351[3092] = 0;
                *(_DWORD *)&v351[3088] = 1479;
                goto LABEL_629;
              default:
                return;
            }
            return;
          case 759:
            sub_47CEB0(dword_8FCDD8);
            return;
          case 760:
            switch ( v11 )
            {
              case 1u:
LABEL_596:
                *(_DWORD *)&v351[3112] = 0;
                *(_DWORD *)&v351[3108] = 0;
                *(_DWORD *)&v351[3104] = 0xFFFF;
                *(_DWORD *)&v351[3100] = -1;
                *(_DWORD *)&v351[3096] = -1;
                *(_DWORD *)&v351[3092] = 0;
                *(_DWORD *)&v351[3088] = 1475;
                goto LABEL_629;
              case 2u:
                *(_DWORD *)&v351[3112] = 0;
                *(_DWORD *)&v351[3108] = 0;
                *(_DWORD *)&v351[3104] = 0xFFFF;
                *(_DWORD *)&v351[3100] = -1;
                *(_DWORD *)&v351[3096] = -1;
                *(_DWORD *)&v351[3092] = 0;
                *(_DWORD *)&v351[3088] = 1480;
                goto LABEL_629;
              case 3u:
                *(_DWORD *)&v351[3112] = 0;
                *(_DWORD *)&v351[3108] = 0;
                *(_DWORD *)&v351[3104] = 0xFFFF;
                *(_DWORD *)&v351[3100] = -1;
                *(_DWORD *)&v351[3096] = -1;
                *(_DWORD *)&v351[3092] = 0;
                *(_DWORD *)&v351[3088] = 1481;
                goto LABEL_629;
              case 4u:
                *(_DWORD *)&v351[3112] = 0;
                *(_DWORD *)&v351[3108] = 0;
                *(_DWORD *)&v351[3104] = 0xFFFF;
                *(_DWORD *)&v351[3100] = -1;
                *(_DWORD *)&v351[3096] = -1;
                *(_DWORD *)&v351[3092] = 0;
                *(_DWORD *)&v351[3088] = 1482;
                goto LABEL_629;
              default:
                return;
            }
            return;
          case 761:
            switch ( v11 )
            {
              case 0xFFFFFFF9:
                *(_DWORD *)&v351[3112] = 0;
                *(_DWORD *)&v351[3108] = 0;
                *(_DWORD *)&v351[3104] = 0xFFFF;
                *(_DWORD *)&v351[3100] = -1;
                *(_DWORD *)&v351[3096] = -1;
                *(_DWORD *)&v351[3092] = 0;
                *(_DWORD *)&v351[3088] = 1488;
                goto LABEL_629;
              case 0xFFFFFFFA:
                *(_DWORD *)&v351[3112] = 0;
                *(_DWORD *)&v351[3108] = 0;
                *(_DWORD *)&v351[3104] = 0xFFFF;
                *(_DWORD *)&v351[3100] = -1;
                *(_DWORD *)&v351[3096] = -1;
                *(_DWORD *)&v351[3092] = 0;
                *(_DWORD *)&v351[3088] = 1487;
                goto LABEL_629;
              case 0xFFFFFFFB:
                *(_DWORD *)&v351[3112] = 0;
                *(_DWORD *)&v351[3108] = 0;
                *(_DWORD *)&v351[3104] = 0xFFFF;
                *(_DWORD *)&v351[3100] = -1;
                *(_DWORD *)&v351[3096] = -1;
                *(_DWORD *)&v351[3092] = 0;
                *(_DWORD *)&v351[3088] = 1486;
                goto LABEL_629;
              case 0xFFFFFFFC:
                *(_DWORD *)&v351[3112] = 0;
                *(_DWORD *)&v351[3108] = 0;
                *(_DWORD *)&v351[3104] = 0xFFFF;
                *(_DWORD *)&v351[3100] = -1;
                *(_DWORD *)&v351[3096] = -1;
                *(_DWORD *)&v351[3092] = 0;
                *(_DWORD *)&v351[3088] = 1485;
                goto LABEL_629;
              case 0xFFFFFFFD:
                *(_DWORD *)&v351[3112] = 0;
                *(_DWORD *)&v351[3108] = 0;
                *(_DWORD *)&v351[3104] = 0xFFFF;
                *(_DWORD *)&v351[3100] = -1;
                *(_DWORD *)&v351[3096] = -1;
                *(_DWORD *)&v351[3092] = 0;
                *(_DWORD *)&v351[3088] = 1484;
                goto LABEL_629;
              case 0xFFFFFFFE:
                *(_DWORD *)&v351[3112] = 0;
                *(_DWORD *)&v351[3108] = 0;
                *(_DWORD *)&v351[3104] = 0xFFFF;
                *(_DWORD *)&v351[3100] = -1;
                *(_DWORD *)&v351[3096] = -1;
                *(_DWORD *)&v351[3092] = 0;
                *(_DWORD *)&v351[3088] = 1483;
                goto LABEL_629;
              default:
                return;
            }
            return;
          case 762:
            *(_DWORD *)&v351[3112] = 0;
            *(_DWORD *)&v351[3108] = 0;
            *(_DWORD *)&v351[3104] = 0xFFFF;
            *(_DWORD *)&v351[3100] = -1;
            *(_DWORD *)&v351[3096] = -1;
            *(_DWORD *)&v351[3092] = 0;
            *(_DWORD *)&v351[3088] = 1504;
            goto LABEL_629;
          case 763:
            switch ( v11 )
            {
              case 0xFFFFFFFD:
                *(_DWORD *)&v351[3112] = 0;
                *(_DWORD *)&v351[3108] = 0;
                *(_DWORD *)&v351[3104] = 0xFFFF;
                *(_DWORD *)&v351[3100] = -1;
                *(_DWORD *)&v351[3096] = -1;
                *(_DWORD *)&v351[3092] = 0;
                *(_DWORD *)&v351[3088] = 1507;
                break;
              case 0xFFFFFFFE:
                *(_DWORD *)&v351[3112] = 0;
                *(_DWORD *)&v351[3108] = 0;
                *(_DWORD *)&v351[3104] = 0xFFFF;
                *(_DWORD *)&v351[3100] = -1;
                *(_DWORD *)&v351[3096] = -1;
                *(_DWORD *)&v351[3092] = 0;
                *(_DWORD *)&v351[3088] = 1506;
                break;
              case 0xFFFFFFFF:
                *(_DWORD *)&v351[3112] = 0;
                *(_DWORD *)&v351[3108] = 0;
                *(_DWORD *)&v351[3104] = 0xFFFF;
                *(_DWORD *)&v351[3100] = -1;
                *(_DWORD *)&v351[3096] = -1;
                *(_DWORD *)&v351[3092] = 0;
                *(_DWORD *)&v351[3088] = 1505;
                break;
              default:
                return;
            }
            goto LABEL_629;
          case 768:
          case 769:
            switch ( v11 )
            {
              case 0xFFFFFFFC:
                *(_DWORD *)&v351[3112] = 0;
                *(_DWORD *)&v351[3108] = 0;
                *(_DWORD *)&v351[3104] = 0xFFFF;
                *(_DWORD *)&v351[3100] = -1;
                *(_DWORD *)&v351[3096] = -1;
                *(_DWORD *)&v351[3092] = 0;
                *(_DWORD *)&v351[3088] = 1492;
                goto LABEL_629;
              case 0xFFFFFFFD:
                *(_DWORD *)&v351[3112] = 0;
                *(_DWORD *)&v351[3108] = 0;
                *(_DWORD *)&v351[3104] = 0xFFFF;
                *(_DWORD *)&v351[3100] = -1;
                *(_DWORD *)&v351[3096] = -1;
                *(_DWORD *)&v351[3092] = 0;
                *(_DWORD *)&v351[3088] = 1491;
                goto LABEL_629;
              case 0xFFFFFFFE:
                *(_DWORD *)&v351[3112] = 0;
                *(_DWORD *)&v351[3108] = 0;
                *(_DWORD *)&v351[3104] = 0xFFFF;
                *(_DWORD *)&v351[3100] = -1;
                *(_DWORD *)&v351[3096] = -1;
                *(_DWORD *)&v351[3092] = 0;
                *(_DWORD *)&v351[3088] = 1490;
                goto LABEL_629;
              case 0xFFFFFFFF:
                goto LABEL_614;
              default:
                return;
            }
            return;
          case 770:
          case 771:
            switch ( v11 )
            {
              case 0xFFFFFFFD:
                *(_DWORD *)&v351[3112] = 0;
                *(_DWORD *)&v351[3108] = 0;
                *(_DWORD *)&v351[3104] = 0xFFFF;
                *(_DWORD *)&v351[3100] = -1;
                *(_DWORD *)&v351[3096] = -1;
                *(_DWORD *)&v351[3092] = 0;
                *(_DWORD *)&v351[3088] = 1494;
                break;
              case 0xFFFFFFFE:
                *(_DWORD *)&v351[3112] = 0;
                *(_DWORD *)&v351[3108] = 0;
                *(_DWORD *)&v351[3104] = 0xFFFF;
                *(_DWORD *)&v351[3100] = -1;
                *(_DWORD *)&v351[3096] = -1;
                *(_DWORD *)&v351[3092] = 0;
                *(_DWORD *)&v351[3088] = 1493;
                break;
              case 0xFFFFFFFF:
LABEL_614:
                *(_DWORD *)&v351[3112] = 0;
                *(_DWORD *)&v351[3108] = 0;
                *(_DWORD *)&v351[3104] = 0xFFFF;
                *(_DWORD *)&v351[3100] = -1;
                *(_DWORD *)&v351[3096] = -1;
                *(_DWORD *)&v351[3092] = 0;
                *(_DWORD *)&v351[3088] = 1489;
                break;
              default:
                return;
            }
LABEL_629:
            *(_DWORD *)&v351[3088] = sub_480550(*(int *)&v351[3088]);
            *(_DWORD *)&v351[3084] = 0;
            *(_DWORD *)&v351[3080] = 4;
LABEL_630:
            sub_420540(
              (int)(a1 + 902646),
              (int)dword_5A041C,
              *(int *)&v351[3080],
              *(int *)&v351[3084],
              *(const char **)&v351[3088],
              *(int *)&v351[3092],
              *(int *)&v351[3096],
              *(int *)&v351[3100],
              *(__int16 *)&v351[3104],
              *(int *)&v351[3108],
              *(int *)&v351[3112],
              v352,
              v353);
            return;
          case 773:
          case 774:
            v233 = sub_47D780(a6 + 16, (int)v379, 0x2000);
            *(_DWORD *)&v351[3112] = v379;
            v379[v233] = 0;
            sub_433DA0((int)(a1 + 769690), *(char **)&v351[3112]);
            return;
          case 802:
            v187 = (_DWORD *)a1[895096];
            *(_DWORD *)&v351[3112] = 0x2000;
            *(_DWORD *)&v351[3108] = v379;
            *(_DWORD *)&v351[3104] = a6 + 16;
            *v187 = 0;
            v188 = sub_47D780(*(const char **)&v351[3104], *(int *)&v351[3108], *(int *)&v351[3112]);
            *(_DWORD *)&v351[3112] = 47;
            *(_DWORD *)&v351[3108] = v379;
            v379[v188] = 0;
            v189 = MEMORY[0x49FFB0](*(const char **)&v351[3108], *(int *)&v351[3112]);
            v190 = v189;
            if ( v189 )
            {
              *v189 = 0;
              sub_47BBB0(dword_8FCDD8);
              *((_BYTE *)&loc_438468 + (_DWORD)a1) = 0;
              *((_BYTE *)a1 + 4424809) = 0;
              *((_WORD *)a1 + 2212405) = 0;
              *((_WORD *)a1 + 2212406) = 0;
              a1[1105654] = 500;
              a1[1105655] = 0;
              v191 = dword_8FCC58;
              v192 = MEMORY[0x49FAF5](v190 + 1);
              sub_47BAB0(v191, v379, v192);
            }
            return;
          case 803:
LABEL_584:
            v232 = (char *)MEMORY[0x4A01AA](0xCu);
            *(_DWORD *)v232 = v357;
            *(_DWORD *)&v351[3112] = v232;
            *(_QWORD *)(v232 + 4) = v358;
            goto LABEL_877;
          case 804:
            v356 = 0;
            sub_47D780(a6 + 16, (int)&v362, 4);
            v207 = a1[230508];
            if ( v207 )
            {
              do
              {
                v208 = *(_DWORD *)(v207 + 4);
                if ( v208 )
                {
                  switch ( *(_WORD *)(v208 + 16) )
                  {
                    case 0x10:
                    case 0x16:
                    case 0x3F:
                    case 0x8C:
                      if ( *(_DWORD **)(v208 + 580) == v357 )
                        v356 = 1;
                      break;
                    default:
                      break;
                  }
                }
                v207 = *(_DWORD *)(v207 + 12);
              }
              while ( v207 );
              if ( v356 )
                return;
            }
            v209 = a1[230520];
            if ( !v209 )
              goto LABEL_502;
            do
            {
              v210 = *(_DWORD *)(v209 + 4);
              if ( v210 && *(_DWORD **)(v210 + 4) == v357 )
                return;
              v209 = *(_DWORD *)(v209 + 12);
            }
            while ( v209 );
LABEL_502:
            if ( WORD1(v358) == 5 )
            {
              v211 = (char *)MEMORY[0x4A01AA](0x248u);
              v44 = v211;
              v356 = (int)v211;
              v386 = 12;
              if ( v211 )
              {
                sub_449480(v211);
                *(_DWORD *)v44 = &off_4AF868;
                *((_DWORD *)v44 + 145) = 0;
                *((_DWORD *)v44 + 144) = 0;
              }
              else
              {
                v44 = 0;
              }
              v386 = -1;
              if ( sub_44D0B0(v357, 22, 0, 0, WORD2(v358), HIWORD(v358), -1, 0, 0) )
              {
                *((_DWORD *)v44 + 63) += sub_401710(1, 8);
                *(_DWORD *)&v351[3112] = 16;
                if ( !dword_59AA28 )
                {
                  v212 = MEMORY[0x4A01AA](*(unsigned int *)&v351[3112]);
                  if ( v212 )
                  {
                    *v212 = &off_4AF460;
                    v212[1] = 0;
                    v212[3] = 0;
                    v212[2] = 0;
                    goto LABEL_511;
                  }
                  goto LABEL_510;
                }
LABEL_513:
                v213 = MEMORY[0x4A01AA](*(unsigned int *)&v351[3112]);
                if ( !v213 )
                  return;
LABEL_514:
                *v213 = &off_4AF460;
                v213[1] = 0;
                v213[3] = 0;
                v213[2] = 0;
                v213[1] = v44;
                v213[3] = 0;
                v213[2] = dword_59AA2C;
                *(_DWORD *)(dword_59AA2C + 12) = v213;
                goto LABEL_515;
              }
              goto LABEL_516;
            }
            if ( WORD1(v358) != 9 )
            {
              if ( WORD1(v358) != 4 )
              {
                if ( WORD1(v358) == 6 )
                {
                  v217 = MEMORY[0x4A01AA](0x20u);
                  *v217 = 6;
                  v217[1] = v357;
                  v217[3] = dword_4AF2FC(v352, v353, v354, v355, v356);
                  v217[5] = WORD2(v358);
                  v218 = HIWORD(v358);
                  v217[4] = 0xFFFF;
                  v217[6] = v218;
                  *((_WORD *)v217 + 4) = 210;
                }
                else
                {
                  if ( WORD1(v358) != 3 )
                    return;
                  v217 = MEMORY[0x4A01AA](0x20u);
                  *v217 = 3;
                  v217[1] = v357;
                  v217[3] = dword_4AF2FC(v352, v353, v354, v355, v356);
                  v217[5] = WORD2(v358);
                  v219 = HIWORD(v358);
                  v217[4] = 0xFFFF;
                  v217[6] = v219;
                  *((_WORD *)v217 + 4) = 230;
                }
                *((_WORD *)v217 + 5) = 0;
                v217[7] = 0;
LABEL_546:
                *(_DWORD *)&v351[3112] = 16;
                if ( dword_59AA58 )
                {
                  v231 = MEMORY[0x4A01AA](*(unsigned int *)&v351[3112]);
                  if ( !v231 )
                    return;
                  *v231 = &off_4AF46C;
                  v231[1] = 0;
                  v231[3] = 0;
                  v231[2] = 0;
                  v231[1] = v217;
                  v231[3] = 0;
                  v231[2] = dword_59AA5C;
                  *(_DWORD *)(dword_59AA5C + 12) = v231;
                }
                else
                {
                  v220 = MEMORY[0x4A01AA](*(unsigned int *)&v351[3112]);
                  if ( v220 )
                  {
                    *v220 = &off_4AF46C;
                    v220[1] = 0;
                    v220[3] = 0;
                    v220[2] = 0;
                  }
                  else
                  {
                    v220 = 0;
                  }
                  dword_59AA58 = (int)v220;
                  if ( !v220 )
                    return;
                  v220[1] = v217;
                  v231 = (_DWORD *)dword_59AA58;
                }
                dword_59AA5C = (int)v231;
                ++dword_59AA68;
                return;
              }
              v216 = (char *)MEMORY[0x4A01AA](0x248u);
              v44 = v216;
              v356 = (int)v216;
              v386 = 14;
              if ( v216 )
              {
                sub_449480(v216);
                *(_DWORD *)v44 = &off_4AF868;
                *((_DWORD *)v44 + 145) = 0;
                *((_DWORD *)v44 + 144) = 0;
              }
              else
              {
                v44 = 0;
              }
              v386 = -1;
              if ( sub_44D0B0(v357, 16, 0, 0, WORD2(v358), HIWORD(v358), -1, 0, 0) )
              {
                *((_DWORD *)v44 + 63) += sub_401710(1, 8);
                *(_DWORD *)&v351[3112] = 16;
                if ( !dword_59AA28 )
                {
                  v212 = MEMORY[0x4A01AA](*(unsigned int *)&v351[3112]);
                  if ( v212 )
                  {
                    *v212 = &off_4AF460;
                    v212[1] = 0;
                    v212[3] = 0;
                    v212[2] = 0;
LABEL_511:
                    dword_59AA28 = (int)v212;
                    if ( v212 )
                    {
                      v212[1] = v44;
                      dword_59AA2C = dword_59AA28;
                      ++dword_59AA38;
                    }
                    return;
                  }
LABEL_510:
                  v212 = 0;
                  goto LABEL_511;
                }
                goto LABEL_513;
              }
LABEL_516:
              if ( !v44 )
                return;
              goto LABEL_517;
            }
            v214 = (char *)MEMORY[0x4A01AA](0x248u);
            v44 = v214;
            v356 = (int)v214;
            v386 = 13;
            if ( v214 )
            {
              sub_449480(v214);
              *(_DWORD *)v44 = &off_4AF850;
            }
            else
            {
              v44 = 0;
            }
            v386 = -1;
            if ( !sub_44D0B0(v357, 140, 0, 0, WORD2(v358), HIWORD(v358), -1, 0, 0) )
              goto LABEL_531;
            *(_DWORD *)&v351[3112] = 16;
            if ( dword_59AA28 )
              goto LABEL_529;
            v215 = MEMORY[0x4A01AA](*(unsigned int *)&v351[3112]);
            if ( v215 )
            {
              *v215 = &off_4AF460;
              v215[1] = 0;
              v215[3] = 0;
              v215[2] = 0;
              goto LABEL_527;
            }
            break;
          case 805:
            v221 = a1[230508];
            if ( !v221 )
              goto LABEL_554;
            do
            {
              v222 = *(_DWORD *)(v221 + 4);
              if ( v222 )
              {
                switch ( *(_WORD *)(v222 + 16) )
                {
                  case 0x10:
                  case 0x16:
                  case 0x3F:
                  case 0x8C:
                    if ( *(_DWORD *)(v222 + 580) != v11 )
                      goto LABEL_553;
                    *(_DWORD *)(v222 + 576) = 0;
                    if ( *(_WORD *)(v222 + 16) != 22 )
                      return;
                    v225 = sub_401710(20000, 30000);
                    if ( *(_WORD *)(v222 + 16) != 22 )
                      return;
                    v226 = sub_401710(330, 334);
                    v227 = (char *)MEMORY[0x4A01AA](0x248u);
                    v228 = v227;
                    v356 = (int)v227;
                    v386 = 15;
                    if ( v227 )
                    {
                      sub_449480(v227);
                      *(_DWORD *)v228 = &off_4AF868;
                      *((_DWORD *)v228 + 145) = 0;
                      *((_DWORD *)v228 + 144) = 0;
                    }
                    else
                    {
                      v228 = 0;
                    }
                    v386 = -1;
                    if ( sub_44D0B0(v357, v226, 0, 0, WORD2(v358), HIWORD(v358), v225, 0, 0) )
                    {
                      *(_DWORD *)&v351[3112] = 16;
                      if ( dword_59AA28 )
                      {
                        v230 = MEMORY[0x4A01AA](*(unsigned int *)&v351[3112]);
                        if ( v230 )
                        {
                          *v230 = &off_4AF460;
                          v230[1] = 0;
                          v230[3] = 0;
                          v230[2] = 0;
                          v230[1] = v228;
                          v230[3] = 0;
                          v230[2] = dword_59AA2C;
                          *(_DWORD *)(dword_59AA2C + 12) = v230;
                          dword_59AA2C = (int)v230;
                          ++dword_59AA38;
                        }
                      }
                      else
                      {
                        v229 = MEMORY[0x4A01AA](*(unsigned int *)&v351[3112]);
                        if ( v229 )
                        {
                          *v229 = &off_4AF460;
                          v229[1] = 0;
                          v229[3] = 0;
                          v229[2] = 0;
                        }
                        else
                        {
                          v229 = 0;
                        }
                        dword_59AA28 = (int)v229;
                        if ( v229 )
                        {
                          v229[1] = v228;
                          dword_59AA2C = dword_59AA28;
                          ++dword_59AA38;
                        }
                      }
                    }
                    else if ( v228 )
                    {
                      (**(void (__thiscall ***)(char *, int))v228)(v228, 1);
                    }
                    v217 = MEMORY[0x4A01AA](0x20u);
                    *v217 = 0;
                    v217[1] = 0;
                    v217[3] = dword_4AF2FC(v352, v353, v354, v355, v356);
                    v217[5] = WORD2(v358);
                    v217[6] = HIWORD(v358);
                    v217[4] = v225 + 800;
                    *((_WORD *)v217 + 4) = 220;
                    *((_WORD *)v217 + 5) = 0;
                    v217[7] = 1;
                    break;
                  default:
                    goto LABEL_553;
                }
                goto LABEL_546;
              }
LABEL_553:
              v221 = *(_DWORD *)(v221 + 12);
            }
            while ( v221 );
LABEL_554:
            v223 = a1[230520];
            if ( v223 )
            {
              while ( 1 )
              {
                v224 = *(_DWORD *)(v223 + 4);
                if ( v224 )
                {
                  if ( *(_DWORD *)(v224 + 4) == v11 )
                    break;
                }
                v223 = *(_DWORD *)(v223 + 12);
                if ( !v223 )
                  return;
              }
              *(_DWORD *)(v224 + 16) = 0;
            }
            return;
          default:
            return;
        }
        goto LABEL_526;
      }
LABEL_876:
      v349 = MEMORY[0x4A01AA](0x40Cu);
      *v349 = v357;
      v349[1] = v358;
      v350 = HIDWORD(v358);
      *(_DWORD *)&v351[3112] = a6 + 16;
      *((_BYTE *)v349 + 12) = 0;
      v349[2] = v350;
      dword_4AF0C0(v349 + 3, *(_DWORD *)&v351[3112]);
      *(_DWORD *)&v351[3112] = v349;
LABEL_877:
      MEMORY[0x481A30](*(FILE **)&v351[3112]);
      return;
    }
    if ( (unsigned __int16)v358 <= 0x525u )
    {
      if ( (unsigned __int16)v358 < 0x524u )
      {
        if ( (unsigned __int16)v358 > 0x517u )
        {
          switch ( (__int16)v358 )
          {
            case 1304:
              v254 = sub_4280B0(a1, v11);
              sub_47D780(a6 + 16, (int)v369, 10);
              if ( v254 )
              {
                *(_DWORD *)(v254 + 400656) = WORD1(v358);
                *(_DWORD *)(v254 + 400660) = WORD2(v358);
                v255 = v254 + 400664;
                *(float *)v255 = v369[0];
                *(float *)(v255 + 4) = v369[1];
                *(_WORD *)(v255 + 8) = LOWORD(v369[2]);
              }
              return;
            case 1305:
LABEL_674:
              sub_42F380((int)a1, (int)&v357, a6 + 16);
              return;
            case 1306:
            case 1307:
            case 1308:
            case 1309:
              goto LABEL_876;
            case 1310:
              v256 = sub_47D780(a6 + 16, (int)v379, 0x2000);
              v257 = HIWORD(v358);
              v258 = WORD1(v358);
              v379[v256] = 0;
              sub_46D570(v379, v257, v258);
              return;
            case 1311:
              v252 = sub_47D780(a6 + 16, (int)v379, 0x2000);
              v253 = HIWORD(v358);
              v379[v252] = 0;
              sub_46D7F0(v379, v253, WORD1(v358));
              return;
            case 1312:
              v264 = sub_4280E0(a1, v11);
              if ( !v264 )
              {
                v264 = MEMORY[0x4A01AA](0x14u);
                (*(void (__thiscall **)(_DWORD *, _DWORD *))(a1[230501] + 4))(a1 + 230501, v264);
              }
              v264[4] = dword_4AF2FC(v352, v353, v354, v355, v356);
              *v264 = v357;
              v264[1] = WORD1(v358);
              v264[2] = WORD2(v358);
              v264[3] = HIWORD(v358);
              return;
            case 1313:
              sub_441DD0((char *)a1 + 2810060, v11, WORD1(v358));
              return;
            case 1314:
              v260 = *(_DWORD *)((char *)&v358 + 2);
              v259 = HIWORD(v260);
              LOWORD(v177) = v260;
              a1[895877] = v11;
              v177 = (unsigned __int16)v177;
              if ( !a1[738872] )
                goto LABEL_366;
              v261 = *(_DWORD *)((char *)a1 + 2955497);
              *(_DWORD *)&v351[3112] = v259;
              v178 = &a1[780 * v261];
              v262 = (const char *)sub_480550(1454);
              MEMORY[0x49FA18](Str, v262, v178 + 702996, *(_DWORD *)&v351[3112]);
              sub_41C120(Str, 11861940);
              if ( (_WORD)v177 )
              {
LABEL_464:
                sub_441DD0((char *)a1 + 2810060, *(_DWORD *)((char *)v178 + 2812046), v177);
                sub_4411F0(a1 + 702515);
              }
              else
              {
                qmemcpy(&v351[8], v178 + 702995, 0xC24u);
                v263 = sub_41AD30(dword_7B7D50);
                if ( v263 != -1 )
                  sub_41AE40(v263);
LABEL_467:
                sub_440BA0((__int16 *)a1 + 1405030, *(_DWORD *)((char *)a1 + 2955497));
                sub_4411F0(a1 + 702515);
              }
              break;
            case 1315:
LABEL_366:
              sub_4411F0(a1 + 702515);
              break;
            default:
              return;
          }
        }
        else if ( (unsigned __int16)v358 == 1303 )
        {
          v248 = sub_47D780(a6 + 16, (int)v379, 0x2000);
          v351[3112] = 2;
          v379[v248] = 0;
          v249 = sub_401430(v351[3112]);
          v250 = sub_401430(3);
          v251 = (const char *)sub_480550(1432);
          sub_4366D0(a1 + 695155, v249, v250, v251);
          sub_46DDF0(v379, WORD1(v358), HIWORD(v358));
        }
        else if ( (unsigned __int16)v358 > 0x451u )
        {
          switch ( (__int16)v358 )
          {
            case 1200:
              v237 = MEMORY[0x4A01AA](0x40Cu);
              *v237 = v357;
              v237[1] = v358;
              v238 = HIDWORD(v358);
              *(_DWORD *)&v351[3112] = a6 + 16;
              *((_BYTE *)v237 + 12) = 0;
              v237[2] = v238;
              dword_4AF0C0(v237 + 3, *(_DWORD *)&v351[3112]);
              sub_423E90(v237);
              break;
            case 1300:
              *((_BYTE *)a1 + 3583288) = (_BYTE)v357;
              v243 = a1[794986];
              v244 = a1 + 794986;
              v245 = *((_BYTE *)a1 + 3583292);
              *(_DWORD *)&v351[3108] = a1[895822];
              v351[3112] = v245;
              v246 = (*(int (__thiscall **)(_DWORD *, _DWORD, _DWORD, _DWORD, int, int, unsigned int))(v243 + 140))(
                       a1 + 794986,
                       *(_DWORD *)((char *)a1 + 3180013),
                       *(_DWORD *)&v351[3108],
                       *(_DWORD *)&v351[3112],
                       v352,
                       v353,
                       v354);
              if ( *((_BYTE *)a1 + 3583288) )
              {
                LOBYTE(v247) = *((_BYTE *)a1 + 3180065);
                (*(void (__thiscall **)(_DWORD *, int, int, _DWORD))(*v244 + 16))(a1 + 794986, 29, v247, 0);
              }
              else
              {
                LOBYTE(v246) = *((_BYTE *)a1 + 3180065);
                (*(void (__thiscall **)(_DWORD *, _DWORD, int, _DWORD))(*v244 + 16))(a1 + 794986, 0, v246, 0);
              }
              break;
            case 1301:
              v379[sub_47D780(a6 + 16, (int)v379, 0x2000)] = 0;
              sub_46D330(v379, WORD1(v358));
              break;
            case 1302:
              v239 = sub_47D780(a6 + 16, (int)v379, 0x2000);
              v351[3112] = 2;
              v379[v239] = 0;
              v240 = sub_401430(v351[3112]);
              v241 = sub_401430(3);
              v242 = (const char *)sub_480550(1433);
              sub_4366D0(a1 + 695155, v240, v241, v242);
              sub_46DC30(v379, HIWORD(v358));
              break;
            default:
              return;
          }
        }
        else if ( (unsigned __int16)v358 == 1105 )
        {
          v236 = sub_42E730(&dword_8FB928);
          dword_4AF2B4(v236, 204, 42, 0);
          dword_8FB944 = 1;
        }
        else if ( (unsigned __int16)v358 > 0x44Cu )
        {
          if ( (unsigned __int16)v358 == 1101 )
          {
            *(_DWORD *)v375 = v11;
            *(_QWORD *)&v375[4] = v358;
            v375[12] = 0;
            dword_4AF0C0(&v375[12], a6 + 16);
            if ( v357 == *(_DWORD **)a1[895096] )
            {
              (*(void (__thiscall **)(_DWORD *, char *))(a1[794986] + 56))(a1 + 794986, v375);
            }
            else
            {
              v235 = sub_4280B0(a1, (int)v357);
              if ( v235 )
                (*(void (__thiscall **)(int, char *))(*(_DWORD *)v235 + 56))(v235, v375);
            }
          }
        }
        else if ( (unsigned __int16)v358 == 1100 )
        {
          *(_DWORD *)v375 = v11;
          *(_QWORD *)&v375[4] = v358;
          v375[12] = 0;
          dword_4AF0C0(&v375[12], a6 + 16);
          if ( v357 == *(_DWORD **)a1[895096] )
          {
            (*(void (__thiscall **)(_DWORD *, char *))(a1[794986] + 52))(a1 + 794986, v375);
          }
          else
          {
            v234 = sub_4280B0(a1, (int)v357);
            if ( v234 )
              (*(void (__thiscall **)(int, char *))(*(_DWORD *)v234 + 52))(v234, v375);
          }
        }
        else if ( (unsigned __int16)v358 == 820 || (unsigned __int16)v358 == 821 )
        {
          sub_479A90(a6 + 16, HIWORD(v358));
        }
        return;
      }
      goto LABEL_876;
    }
    switch ( (__int16)v358 )
    {
      case 1318:
        a1[19] = 1;
        if ( v13 == 1212 )
          a1[19] = 1;
        return;
      case 1319:
      case 1320:
      case 1321:
      case 1410:
        goto LABEL_876;
      case 1322:
        v308 = BYTE2(v358);
        v309 = 0;
        *(_WORD *)((char *)a1 + 2959663) = 0;
        *((_BYTE *)a1 + 2959662) = v308;
        v310 = 0;
        v311 = dword_4B52A8;
        while ( *(v311 - 1) > WORD1(v358) || *v311 <= WORD1(v358) )
        {
          ++v311;
          ++v309;
          if ( (int)v311 >= (int)&dword_4B5334 )
            goto LABEL_795;
        }
        v310 = 1;
LABEL_795:
        if ( v309 == 34 && v310 == 1 )
          v309 = 35;
        if ( v309 + 1 != WORD2(v358) )
        {
          v312 = (const char *)sub_480550(1462);
          MEMORY[0x49FA18](v376, "%s %d -> %d", v312, v309, v309 + 1);
          v313 = sub_401430(2);
          v314 = sub_401430(3);
          sub_4366D0(a1 + 695155, v313, v314, v376);
        }
        return;
      case 1324:
        *(_DWORD *)&v351[3112] = v11;
        if ( v13 )
        {
          v315 = (_DWORD *)sub_4280B0(a1, *(int *)&v351[3112]);
          if ( !v315 )
          {
            if ( v357 != *(_DWORD **)a1[895096] )
              return;
            v315 = a1 + 794986;
            if ( a1 == (_DWORD *)-3179944 )
              return;
          }
          v316 = (char *)MEMORY[0x4A01AA](0x240u);
          v356 = (int)v316;
          v386 = 4;
          if ( v316 )
            v317 = sub_449480(v316);
          else
            v317 = 0;
          v386 = -1;
          v318 = sub_47D780(a6 + 16, (int)v375, 1024);
          v356 = v318;
          if ( v318 >= 1024 )
          {
            v318 = 1024;
            v356 = 1024;
          }
          v319 = (int)v357;
          v320 = v315[33];
          v375[v318] = 0;
          if ( (*(int (__thiscall **)(char *, int, int, _DWORD, int, _DWORD, _DWORD *, int, _DWORD, int, int, unsigned int, int, int, _DWORD *, _DWORD, _DWORD, int, char *, int))(*(_DWORD *)v317 + 4))(
                 v317,
                 420,
                 v320,
                 v315[34],
                 v320,
                 v315[34],
                 v315,
                 v319,
                 0,
                 v352,
                 v353,
                 v354,
                 v355,
                 v356,
                 v357,
                 v358,
                 HIDWORD(v358),
                 v359,
                 j,
                 v361) )
          {
            if ( LODWORD(v369[0]) )
              MEMORY[0x49FA18](v317 + 316, &v375[44]);
            v361 = 16;
            if ( dword_59AA28 )
            {
              v322 = MEMORY[0x4A01AA](v361);
              if ( v322 )
              {
                *v322 = &off_4AF460;
                v322[1] = 0;
                v322[3] = 0;
                v322[2] = 0;
                v322[1] = v317;
                v322[3] = 0;
                v322[2] = dword_59AA2C;
                *(_DWORD *)(dword_59AA2C + 12) = v322;
                dword_59AA2C = (int)v322;
                ++dword_59AA38;
              }
            }
            else
            {
              v321 = MEMORY[0x4A01AA](v361);
              if ( v321 )
              {
                *v321 = &off_4AF460;
                v321[1] = 0;
                v321[3] = 0;
                v321[2] = 0;
              }
              else
              {
                v321 = 0;
              }
              dword_59AA28 = (int)v321;
              if ( v321 )
              {
                v321[1] = v317;
                dword_59AA2C = dword_59AA28;
                ++dword_59AA38;
              }
            }
          }
          else if ( v317 )
          {
            (**(void (__thiscall ***)(char *, int))v317)(v317, 1);
          }
          v323 = (char *)MEMORY[0x4A01AA](0x240u);
          v324 = v323;
          LODWORD(v369[0]) = v323;
          if ( v323 )
          {
            sub_449480(v323);
            *(_DWORD *)v324 = &off_4AF838;
          }
          else
          {
            v324 = 0;
          }
          if ( !(*(int (__thiscall **)(char *, int, _DWORD, _DWORD, _DWORD, _DWORD, _DWORD, _DWORD, _DWORD))(*(_DWORD *)v324 + 4))(
                  v324,
                  337,
                  v315[33],
                  v315[34],
                  v315[33],
                  v315[34],
                  0,
                  0,
                  0) )
          {
            if ( v324 )
              (**(void (__thiscall ***)(char *, int))v324)(v324, 1);
            goto LABEL_837;
          }
          v354 = 16;
          if ( dword_59A9F8 )
          {
            v326 = MEMORY[0x4A01AA](v354);
            if ( !v326 )
            {
LABEL_837:
              (*(void (__thiscall **)(_DWORD *, int, int, _DWORD))(*v315 + 16))(v315, 7, 4, 0);
              v315[100061] = 1;
              v315[100062] = 0;
              dword_8F1450 = 2000;
              dword_8F1454 = 0;
              v327 = (double)(v315[39] - 2);
              v356 = v315[40] - 30;
              v369[2] = 0.0;
              v369[0] = v327;
              v369[1] = (float)v356;
              sub_465CA0(v369[0], v369[1], 0);
              return;
            }
            *v326 = &off_4AF460;
            v326[1] = 0;
            v326[3] = 0;
            v326[2] = 0;
            v326[1] = v324;
            v326[3] = 0;
            v326[2] = dword_59A9FC;
            *(_DWORD *)(dword_59A9FC + 12) = v326;
          }
          else
          {
            v325 = MEMORY[0x4A01AA](v354);
            if ( v325 )
            {
              *v325 = &off_4AF460;
              v325[1] = 0;
              v325[3] = 0;
              v325[2] = 0;
            }
            else
            {
              v325 = 0;
            }
            dword_59A9F8 = (int)v325;
            if ( !v325 )
              goto LABEL_837;
            v325[1] = v324;
            v326 = (_DWORD *)dword_59A9F8;
          }
          dword_59A9FC = (int)v326;
          ++dword_59AA08;
          goto LABEL_837;
        }
        v328 = (_DWORD *)sub_4280B0(a1, *(int *)&v351[3112]);
        if ( !v328 )
        {
          if ( v357 != *(_DWORD **)a1[895096] )
            return;
          v328 = a1 + 794986;
          if ( a1 == (_DWORD *)-3179944 )
            return;
        }
        v329 = (char *)MEMORY[0x4A01AA](0x240u);
        v330 = v329;
        v356 = (int)v329;
        v386 = 6;
        if ( v329 )
        {
          sub_449480(v329);
          *(_DWORD *)v330 = &off_4AF838;
        }
        else
        {
          v330 = 0;
        }
        v331 = v328[34];
        v332 = v328[33];
        v333 = *(void (__thiscall ***)(_DWORD *, int))v330;
        v386 = -1;
        if ( ((int (__thiscall *)(char *, int, int, int, int, int, _DWORD, _DWORD, _DWORD, int, int, unsigned int, int, int, _DWORD *, _DWORD, _DWORD))v333[1])(
               v330,
               337,
               v332,
               v331,
               v332,
               v331,
               0,
               0,
               0,
               v352,
               v353,
               v354,
               v355,
               v356,
               v357,
               v358,
               HIDWORD(v358)) )
        {
          HIDWORD(v358) = 16;
          if ( dword_59A9F8 )
          {
            v335 = MEMORY[0x4A01AA](HIDWORD(v358));
            if ( !v335 )
              goto LABEL_856;
            *v335 = &off_4AF460;
            v335[1] = 0;
            v335[3] = 0;
            v335[2] = 0;
            v335[1] = v330;
            v335[3] = 0;
            v335[2] = dword_59A9FC;
            *(_DWORD *)(dword_59A9FC + 12) = v335;
            goto LABEL_853;
          }
          v334 = MEMORY[0x4A01AA](HIDWORD(v358));
          if ( v334 )
          {
            *v334 = &off_4AF460;
            v334[1] = 0;
            v334[3] = 0;
            v334[2] = 0;
          }
          else
          {
            v334 = 0;
          }
          dword_59A9F8 = (int)v334;
          if ( v334 )
          {
            v334[1] = v330;
            v335 = (_DWORD *)dword_59A9F8;
LABEL_853:
            dword_59A9FC = (int)v335;
            ++dword_59AA08;
          }
        }
        else if ( v330 )
        {
          (**(void (__thiscall ***)(char *, int))v330)(v330, 1);
        }
LABEL_856:
        v336 = sub_47D780(a6 + 16, (int)&v375[32], 1024);
        v363 = v336;
        if ( v336 >= 1024 )
        {
          v336 = 1024;
          v363 = 1024;
        }
        HIDWORD(v358) = 576;
        v375[v336 + 32] = 0;
        v337 = (char *)MEMORY[0x4A01AA](HIDWORD(v358));
        LODWORD(v369[4]) = v337;
        if ( v337 )
          v44 = sub_449480(v337);
        else
          v44 = 0;
        v338 = v328[34];
        v339 = v328[33];
        v340 = *(_DWORD *)v44;
        v358 = v364 | 0x3E800000000LL;
        v357 = v328;
        v356 = v338;
        if ( !(*(int (__thiscall **)(char *, int, int, int, int))(v340 + 4))(v44, 421, v339, v338, v339) )
          return;
        if ( v356 )
          MEMORY[0x49FA18](v44 + 316, v375);
        (*(void (__thiscall **)(_DWORD *, int, int, _DWORD))(*v328 + 16))(v328, 7, 4, 0);
        v328[100061] = 1;
        v328[100062] = 0;
        dword_8F1450 = 2000;
        dword_8F1454 = 0;
        *(_DWORD *)&v351[3112] = 16;
        if ( dword_59AA28 )
        {
LABEL_529:
          v213 = MEMORY[0x4A01AA](*(unsigned int *)&v351[3112]);
          if ( !v213 )
            return;
          goto LABEL_514;
        }
        v215 = MEMORY[0x4A01AA](*(unsigned int *)&v351[3112]);
        if ( v215 )
        {
          *v215 = &off_4AF460;
          v215[1] = 0;
          v215[3] = 0;
          v215[2] = 0;
          goto LABEL_527;
        }
LABEL_526:
        v215 = 0;
LABEL_527:
        dword_59AA28 = (int)v215;
        if ( !v215 )
          return;
        v215[1] = v44;
        v213 = (_DWORD *)dword_59AA28;
LABEL_515:
        dword_59AA2C = (int)v213;
        ++dword_59AA38;
        return;
      case 1325:
        v346 = sub_47D780(a6 + 16, (int)v375, 1024);
        if ( v346 >= 1024 )
          v346 = 1024;
        v375[v346] = 0;
        v347 = (char *)a1 + 2956553;
        v348 = 12;
        do
        {
          if ( *(_DWORD **)(v347 - 11) == v357 )
            MEMORY[0x49FA18](v347, v375);
          v347 += 3112;
          --v348;
        }
        while ( v348 );
        return;
      case 1330:
        v341 = sub_47D780(a6 + 16, (int)v379, 0x2000);
        v342 = (_BYTE *)a1[895096];
        v379[v341] = 0;
        memset(v375, 0, 0x400u);
        sub_430230(v342, (int)v375);
        sub_42F6B0(v375);
        sub_47D330(v375);
        if ( strlen(v379) == 60 )
        {
          qmemcpy(v369, v379, sizeof(v369));
          qmemcpy(v373, &v379[20], 0x14u);
          v370 = 0;
          qmemcpy(v371, &v379[40], 0x14u);
          v373[20] = 0;
          v371[20] = 0;
          v343 = sub_42F760((const char *)v369);
          v344 = sub_42F760(v373);
          v345 = sub_42F760(v371);
          *((_BYTE *)&loc_438468 + (_DWORD)a1) = v345;
          *((_BYTE *)a1 + 4424809) = HIBYTE(v345);
          *((_WORD *)a1 + 2212405) = v343;
          *((_WORD *)a1 + 2212406) = v344;
        }
        return;
      case 1350:
        v265 = sub_47D780(a6 + 16, (int)v379, 0x2000);
        *(_DWORD *)&v351[3112] = 9;
        v379[v265] = 0;
        v266 = MEMORY[0x49FFB0](v379, *(int *)&v351[3112]);
        if ( v266 )
        {
          *v266 = 0;
          a1[789675] = MEMORY[0x49FAF5](v266 + 1);
        }
        v267 = (unsigned int)v357;
        v268 = WORD1(v358);
        v269 = (unsigned __int16)v357;
        a1[702498] = 1;
        a1[789654] = v269;
        a1[789655] = HIWORD(v267);
        a1[789676] = v268;
        return;
      case 1351:
        a1[789654] = (unsigned __int16)v11;
        a1[789655] = HIWORD(v11);
        return;
      case 1352:
        v270 = HIWORD(v11);
        v271 = (unsigned __int16)v11;
        if ( WORD2(v358) != 10 )
        {
          if ( WORD2(v358) == 20 )
          {
            j = 0;
            v379[sub_47D780(a6 + 16, (int)v379, 0x2000)] = 0;
            v278 = v379;
            v279 = &v371[4];
            do
            {
              v280 = MEMORY[0x49FFB0](v278, 9);
              v281 = v280;
              if ( v280 )
              {
                *(_DWORD *)&v351[3112] = v278;
                *v280 = 0;
                *((_DWORD *)v279 - 1) = MEMORY[0x49FAF5](*(_DWORD *)&v351[3112]);
                v278 = v281 + 1;
              }
              v282 = MEMORY[0x49FFB0](v278, 9);
              v283 = v282;
              if ( v282 )
              {
                *v282 = 0;
                v283 = v282 + 1;
                *(_DWORD *)v279 = MEMORY[0x49FAF5](v278);
                v278 = v283;
              }
              v279 += 8;
              v284 = ++j;
            }
            while ( v283 );
            v362 = 0;
            j = v284 - 1;
            if ( (int)(v284 - 1) > 0 )
            {
              v285 = v356;
              v286 = v361;
              v287 = v371;
              do
              {
                switch ( WORD1(v358) )
                {
                  case 0:
                    v285 = *(_DWORD *)v287;
                    v286 = *((_DWORD *)v287 + 1) + 20;
                    break;
                  case 1:
                    v285 = *(_DWORD *)v287 - 20;
                    v286 = *((_DWORD *)v287 + 1) + 20;
                    break;
                  case 2:
                  case 3:
                    v285 = *(_DWORD *)v287 - 20;
                    goto LABEL_727;
                  case 4:
                    v285 = *(_DWORD *)v287;
                    goto LABEL_727;
                  case 5:
                  case 6:
                    v285 = *(_DWORD *)v287 + 20;
LABEL_727:
                    v286 = *((_DWORD *)v287 + 1) - 20;
                    break;
                  case 7:
                    v285 = *(_DWORD *)v287 + 20;
                    v286 = *((_DWORD *)v287 + 1) + 20;
                    break;
                  default:
                    break;
                }
                v288 = (char *)MEMORY[0x4A01AA](0x240u);
                v356 = (int)v288;
                v386 = 3;
                if ( v288 )
                  v361 = (int)sub_449480(v288);
                else
                  v361 = 0;
                v386 = -1;
                if ( (*(int (__thiscall **)(int, int, int, int, _DWORD, _DWORD, _DWORD, _DWORD, int))(*(_DWORD *)v361 + 4))(
                       v361,
                       596,
                       v285,
                       v286,
                       *(_DWORD *)v287,
                       *((_DWORD *)v287 + 1),
                       0,
                       0,
                       70 * (_DWORD)v362) )
                {
                  *(_DWORD *)&v351[3112] = 16;
                  if ( dword_59A9F8 )
                  {
                    v290 = MEMORY[0x4A01AA](*(unsigned int *)&v351[3112]);
                    if ( v290 )
                    {
                      v291 = v361;
                      *v290 = &off_4AF460;
                      v290[1] = 0;
                      v290[3] = 0;
                      v290[2] = 0;
                      v290[1] = v291;
                      v290[3] = 0;
                      v290[2] = dword_59A9FC;
                      *(_DWORD *)(dword_59A9FC + 12) = v290;
                      dword_59A9FC = (int)v290;
                      ++dword_59AA08;
                    }
                  }
                  else
                  {
                    v289 = MEMORY[0x4A01AA](*(unsigned int *)&v351[3112]);
                    if ( v289 )
                    {
                      *v289 = &off_4AF460;
                      v289[1] = 0;
                      v289[3] = 0;
                      v289[2] = 0;
                    }
                    else
                    {
                      v289 = 0;
                    }
                    dword_59A9F8 = (int)v289;
                    if ( v289 )
                    {
                      v289[1] = v361;
                      dword_59A9FC = dword_59A9F8;
                      ++dword_59AA08;
                    }
                  }
                }
                else if ( v361 )
                {
                  (**(void (__thiscall ***)(int, int))v361)(v361, 1);
                }
                v287 += 8;
                v362 = (__int16 *)((char *)v362 + 1);
              }
              while ( (int)v362 < (int)j );
            }
          }
          return;
        }
        switch ( WORD1(v358) )
        {
          case 0:
            v272 = (unsigned __int16)v11;
            v361 = v270 + 20;
            break;
          case 1:
            v272 = (unsigned __int16)v11 - 20;
            v361 = v270 + 10;
            break;
          case 2:
            v272 = (unsigned __int16)v11 - 20;
            v361 = v270 - 10;
            break;
          case 3:
            v272 = (unsigned __int16)v11 - 20;
            v361 = v270 - 30;
            break;
          case 4:
            v272 = (unsigned __int16)v11;
            v361 = v270 - 20;
            break;
          case 5:
            v272 = (unsigned __int16)v11 + 20;
            v361 = v270 - 30;
            break;
          case 6:
            v272 = (unsigned __int16)v11 + 20;
            v361 = v270 - 10;
            break;
          case 7:
            v272 = (unsigned __int16)v11 + 20;
            v361 = v270 + 10;
            break;
          default:
            v272 = v356;
            break;
        }
        v273 = (char *)MEMORY[0x4A01AA](0x240u);
        v356 = (int)v273;
        v386 = 2;
        if ( v273 )
        {
          v274 = sub_449480(v273);
          j = v274;
        }
        else
        {
          j = 0;
          v274 = 0;
        }
        v275 = *(_DWORD *)v274;
        v386 = -1;
        if ( !(*(int (__thiscall **)(char *, int, int, int, int, unsigned int, _DWORD, _DWORD, _DWORD))(v275 + 4))(
                v274,
                594,
                v272,
                v361,
                v271,
                v270,
                0,
                0,
                0) )
        {
          if ( j )
            (**(void (__thiscall ***)(char *, int))j)(j, 1);
          return;
        }
        *(_DWORD *)&v351[3112] = 16;
        if ( !dword_59A9F8 )
        {
          v276 = MEMORY[0x4A01AA](*(unsigned int *)&v351[3112]);
          if ( v276 )
          {
            *v276 = &off_4AF460;
            v276[1] = 0;
            v276[3] = 0;
            v276[2] = 0;
          }
          else
          {
            v276 = 0;
          }
          dword_59A9F8 = (int)v276;
          if ( v276 )
          {
            v276[1] = j;
            v42 = (_DWORD *)dword_59A9F8;
            goto LABEL_708;
          }
          return;
        }
        v42 = MEMORY[0x4A01AA](*(unsigned int *)&v351[3112]);
        if ( !v42 )
          return;
        v277 = j;
        *v42 = &off_4AF460;
        v42[1] = 0;
        v42[3] = 0;
        v42[2] = 0;
        v42[1] = v277;
        v42[3] = 0;
        v42[2] = dword_59A9FC;
        *(_DWORD *)(dword_59A9FC + 12) = v42;
        goto LABEL_708;
      case 1353:
        a1[702498] = 0;
        if ( a1[789465] )
          sub_43AB90(a1 + 695155, 18);
        return;
      case 1355:
        sub_455BF0(a6, HIWORD(v358), WORD1(v358), WORD2(v358));
        if ( !a1[789689] )
        {
          sub_43AB90(a1 + 695155, 19);
          a1[790178] = v357;
        }
        return;
      case 1356:
        if ( v13 )
        {
          if ( v13 == 2 )
            sub_45BBA0(a6, HIWORD(v358));
        }
        else
        {
          sub_45BA50(a6, HIWORD(v358));
        }
        return;
      case 1357:
        v292 = MEMORY[0x4A01AA](0x265u);
        sub_47D780(a6 + 16, (int)v292, 613);
        v293 = WORD1(v358);
        v145 = WORD1(v358) == 0;
        a1[792703] = 1;
        if ( v145 )
        {
          sub_45BB20(v292);
          a1[794413] = 0;
        }
        else if ( v293 == 2 )
        {
          sub_45BB20(v292);
          a1[794413] = 2;
        }
        return;
      case 1358:
        a1[702499] = 1;
        return;
      case 1400:
        a1[790111] = v11 == 0;
        return;
      case 1401:
        sub_464A00(a1 + 782400, v11, WORD1(v358));
        return;
      case 1402:
        v356 = v11;
        if ( v11 )
        {
          v294 = sub_401430(2);
          v295 = sub_401430(3);
          v296 = (const char *)sub_480550(1519);
          sub_4366D0(a1 + 695155, v294, v295, v296);
          if ( dword_4AF110(260, Str) )
          {
            if ( Str[0] )
            {
              strcat(Str, MEMORY[0x4B5758]);
              MEMORY[0x49FA18](v374, "%d", v356);
              MEMORY[0x49FA18](v378, "%s_%s", (const char *)dword_4B97F0, (const char *)a1 + 3583089);
              dword_823A30 = v356;
              dword_4AF0C4(MEMORY[0x4B5748], v378, v374, Str);
            }
          }
        }
        return;
      case 1403:
        v297 = *(_DWORD *)a1[895096];
        v361 = *(_DWORD *)((char *)&v358 + 2);
        if ( v11 == v297 )
        {
          if ( dword_4AF110(260, v374) && v374[0] )
          {
            v356 = sub_47D780(a6 + 16, (int)v379, 4096);
            MEMORY[0x49FA18](v378, "%s\\Data", v374);
            dword_4AF0C8(v378);
            *(_DWORD *)&v351[3112] = *(_DWORD *)((char *)&v358 + 2);
            v362 = (__int16 *)((char *)a1 + 3583089);
            MEMORY[0x49FA18](Str, "%s_%s.jpg", (const char *)dword_4B97F0, (const char *)a1 + 3583089);
            v298 = (FILE *)MEMORY[0x4A0250](Str, MEMORY[0x4B5444]);
            MEMORY[0x4A07ED](v379, 1, v356, v298);
            MEMORY[0x4A0154](v298);
            v299 = (FILE *)MEMORY[0x4A0250](Str, Mode);
            v356 = MEMORY[0x4A01B8](v299->_file);
            j = (char *)(v356 + 8);
            v99 = MEMORY[0x4A01AA](v356 + 8);
            if ( v99 )
            {
              MEMORY[0x4A006C](v99 + 2, 1, v356, v299);
              MEMORY[0x4A0154](v299);
              if ( sub_47D4C0((int)(v99 + 2), v356) )
              {
                MEMORY[0x49FA18](FileName, "%s_%s.fac", (const char *)dword_4B97F0, (const char *)v362);
                v300 = (FILE *)MEMORY[0x4A0250](FileName, MEMORY[0x4B5444]);
                if ( v300 )
                {
                  MEMORY[0x4A07ED](v99, 1, j, v300);
                  MEMORY[0x4A0154](v300);
                  dword_4AF128(Str);
                  dword_4AF0C8(v374);
                  a1[702477] = 1;
                  if ( dword_4AF110(260, v374) )
                  {
                    if ( v374[0] )
                    {
                      strcat(v374, MEMORY[0x4B5758]);
                      MEMORY[0x49FA18](v377, "%d", v361);
                      MEMORY[0x49FA18](v376, "%s_%s", (const char *)dword_4B97F0, (const char *)v362);
                      dword_4AF0C4(MEMORY[0x4B5748], v376, v377, v374);
                    }
                  }
                }
              }
              else
              {
LABEL_228:
                MEMORY[0x4939F6](v99);
              }
            }
            else
            {
              MEMORY[0x4A0154](v299);
            }
          }
          return;
        }
        j = (char *)sub_4280B0(a1, v11);
        if ( !j )
          return;
        if ( !(WORD1(v358) | (WORD2(v358) << 16)) )
        {
          if ( dword_4AF110(260, v374) && v374[0] )
          {
            strcat(v374, MEMORY[0x4B5758]);
            MEMORY[0x49FA18](v376, aD_4, 0);
            MEMORY[0x49FA18](v377, "%s_%s", (const char *)dword_4B97F0, j + 403145);
            dword_4AF0C4(MEMORY[0x4B5748], v377, v376, v374);
          }
          return;
        }
        v301 = sub_47D780(a6 + 16, (int)v379, 4096);
        if ( !dword_4AF110(260, v374) )
          goto LABEL_786;
        if ( !v374[0] )
          return;
        MEMORY[0x49FA18](v376, "%s\\Data", v374);
        dword_4AF0C8(v376);
        *(_DWORD *)&v351[3112] = *(_DWORD *)((char *)&v358 + 2);
        MEMORY[0x49FA18](Str, "%s_%s.jpg", (const char *)dword_4B97F0, j + 403145);
        v302 = (FILE *)MEMORY[0x4A0250](Str, MEMORY[0x4B5444]);
        MEMORY[0x4A07ED](v379, 1, v301, v302);
        MEMORY[0x4A0154](v302);
        v303 = (FILE *)MEMORY[0x4A0250](Str, Mode);
        v304 = MEMORY[0x4A01B8](v303->_file);
        v356 = v304 + 8;
        v305 = (char *)MEMORY[0x4A01AA](v304 + 8);
        if ( !v305 )
        {
          MEMORY[0x4A0154](v303);
          return;
        }
        MEMORY[0x4A006C](v305 + 8, 1, v304, v303);
        MEMORY[0x4A0154](v303);
        if ( !sub_47D4C0((int)(v305 + 8), v304) )
        {
          MEMORY[0x4939F6](v305);
          return;
        }
        MEMORY[0x49FA18](v377, "%s_%s.fac", (const char *)dword_4B97F0, j + 403145);
        v306 = (FILE *)MEMORY[0x4A0250](v377, MEMORY[0x4B5444]);
        v307 = v306;
        if ( v306 )
        {
          MEMORY[0x4A07ED](v305, 1, v356, v306);
          MEMORY[0x4A0154](v307);
          dword_4AF128(Str);
          dword_4AF0C8(v374);
LABEL_786:
          if ( dword_4AF110(260, v374) && v374[0] )
          {
            strcat(v374, MEMORY[0x4B5758]);
            MEMORY[0x49FA18](v376, "%d", v361);
            MEMORY[0x49FA18](v377, "%s_%s", (const char *)dword_4B97F0, j + 403145);
            dword_4AF0C4(MEMORY[0x4B5748], v377, v376, v374);
            *((_DWORD *)j + 100104) = 1;
          }
        }
        return;
      default:
        return;
    }
  }
  if ( (unsigned __int16)v358 == 685 )
  {
    v173 = WORD2(v358);
    a1[768869] = v11;
    a1[895877] = WORD1(v358) | (v173 << 16);
    return;
  }
  if ( (unsigned __int16)v358 > 0x27Cu )
  {
    switch ( (__int16)v358 )
    {
      case 637:
      case 638:
        goto LABEL_876;
      case 640:
        sub_44F870(v11, SHIDWORD(v358), SBYTE2(v358));
        return;
      case 641:
        if ( v11 )
        {
          v155 = sub_4280B0(a1, v11);
          if ( v155 )
            (*(void (__thiscall **)(int))(*(_DWORD *)v155 + 20))(v155);
        }
        else
        {
          sub_4832D0(WORD1(v358), 0, 0, 0);
        }
        return;
      case 642:
        if ( v13 < 0xCu && a1[778 * WORD1(v358) + 739118] )
        {
          HIWORD(a1[778 * WORD1(v358) + 739136]) = (_WORD)v357;
          LOWORD(a1[778 * WORD1(v358) + 739137]) = WORD2(v358);
          v161 = BYTE2(a1[778 * WORD1(v358) + 739127]);
          if ( v161 == 54 || v161 == 55 )
          {
            *(_DWORD *)&v351[3112] = 100;
            *(_DWORD *)&v351[3108] = v357;
          }
          else
          {
            *(_DWORD *)&v351[3112] = 1000;
            *(_DWORD *)&v351[3108] = v357;
          }
          if ( (int)sub_4017A0(*(_DWORD *)&v351[3108], *(_DWORD *)&v351[3112]) <= 0 )
          {
            v162 = (const char *)sub_480550(1470);
            MEMORY[0x49FA18](Str, v162, &a1[778 * WORD1(v358) + 739120]);
            sub_41C120(Str, 16777010);
          }
        }
        return;
      case 643:
        sub_461620(a1 + 781870);
        v156 = a1[795019];
        v157 = a1[795020];
        *(_DWORD *)&v351[3112] = 0x2000;
        *(_DWORD *)&v351[3108] = v379;
        *(_DWORD *)&v351[3104] = a6 + 16;
        a1[1106080] = v156;
        a1[1106081] = v157;
        v158 = sub_47D780(*(const char **)&v351[3104], *(int *)&v351[3108], *(int *)&v351[3112]);
        *(_DWORD *)&v351[3112] = 47;
        v379[v158] = 0;
        v159 = MEMORY[0x49FFB0](v379, *(int *)&v351[3112]);
        if ( v159 )
        {
          *v159 = 0;
          v160 = v159 + 1;
          if ( v159 != (char *)-1 )
          {
            sub_461730((int)(a1 + 781870), v159 + 1);
            sub_45F5F0(a1 + 781870, v160);
          }
        }
        if ( !sub_42E740(a1 + 781870) )
        {
          sub_43AB90(a1 + 695155, 9);
          sub_42E750(v357);
        }
        return;
      case 644:
        sub_424AD0((int)a1);
        return;
      case 645:
        v143 = sub_47D780(a6 + 16, (int)v379, 10240);
        v144 = WORD1(v358);
        v379[v143] = 0;
        sub_479410((int)(a1 + 749348), v379, v144);
        v145 = a1[749360] == 0;
        goto LABEL_444;
      case 646:
        if ( !a1[702527] )
          sub_43AB90(a1 + 695155, 0);
        v148 = a1 + 702515;
        sub_441FD0((_BYTE *)a1 + 2810060, 2u);
        a1[738876] = v357;
        goto LABEL_357;
      case 647:
        if ( !a1[738872] )
          return;
        v149 = *(_DWORD *)((char *)a1 + 2955497);
        a1[738882] = v11;
        *(_DWORD *)&v351[3112] = v11;
        if ( BYTE1(a1[780 * v149 + 703011]) < 2u )
          goto LABEL_360;
        *(_DWORD *)&v351[3108] = sub_480550(1451);
        MEMORY[0x49FA18](a1 + 738877, *(const char **)&v351[3108], *(_DWORD *)&v351[3112]);
        goto LABEL_361;
      case 648:
        a1[895877] = v11;
        if ( !a1[738872] )
          goto LABEL_366;
        *(_DWORD *)&v351[3112] = &a1[780 * *(_DWORD *)((char *)a1 + 2955497) + 702996];
        v150 = (const char *)sub_480550(1453);
        MEMORY[0x49FA18](Str, v150, *(_DWORD *)&v351[3112]);
        sub_41C120(Str, 11861940);
        sub_440BA0((__int16 *)a1 + 1405030, *(_DWORD *)((char *)a1 + 2955497));
        sub_4411F0(a1 + 702515);
        return;
      case 649:
        *(_DWORD *)&v351[3112] = 0;
        *(_DWORD *)&v351[3108] = 0;
        *(_DWORD *)&v351[3104] = 0xFFFF;
        *(_DWORD *)&v351[3100] = -1;
        *(_DWORD *)&v351[3096] = -1;
        *(_DWORD *)&v351[3092] = 0;
        *(_DWORD *)&v351[3088] = 1455;
        goto LABEL_368;
      case 650:
        a1[895877] = v11;
        sub_478750(a1 + 749348);
        return;
      case 651:
        v146 = v11 - 1;
        if ( v146 )
        {
          v147 = v146 - 1;
          if ( v147 )
          {
            if ( v147 != 1 )
              return;
            *(_DWORD *)&v351[3112] = 0;
            *(_DWORD *)&v351[3108] = 0;
            *(_DWORD *)&v351[3104] = 0xFFFF;
            *(_DWORD *)&v351[3100] = -1;
            *(_DWORD *)&v351[3096] = -1;
            *(_DWORD *)&v351[3092] = 0;
            *(_DWORD *)&v351[3088] = sub_480550(1439);
            *(_DWORD *)&v351[3084] = 0;
            *(_DWORD *)&v351[3080] = 2;
          }
          else
          {
            *(_DWORD *)&v351[3112] = 0;
            *(_DWORD *)&v351[3108] = 0;
            *(_DWORD *)&v351[3104] = 0xFFFF;
            *(_DWORD *)&v351[3100] = -1;
            *(_DWORD *)&v351[3096] = -1;
            *(_DWORD *)&v351[3092] = 0;
            *(_DWORD *)&v351[3088] = sub_480550(1438);
            *(_DWORD *)&v351[3084] = 0;
            *(_DWORD *)&v351[3080] = 2;
          }
        }
        else
        {
          sub_478750(a1 + 749348);
          *(_DWORD *)&v351[3112] = 0;
          *(_DWORD *)&v351[3108] = 0;
          *(_DWORD *)&v351[3104] = 0xFFFF;
          *(_DWORD *)&v351[3100] = -1;
          *(_DWORD *)&v351[3096] = -1;
          *(_DWORD *)&v351[3092] = 0;
          *(_DWORD *)&v351[3088] = sub_480550(1437);
          *(_DWORD *)&v351[3084] = 0;
          *(_DWORD *)&v351[3080] = 2;
        }
        goto LABEL_630;
      case 652:
        sub_4796A0((int)(a1 + 749348), a6 + 16, WORD1(v358), WORD2(v358));
        return;
      case 653:
        sub_4832D0(106, 0, 0, 0);
        v163 = a1[895877];
        if ( (int)v357 - v163 > 0 )
        {
          *(_DWORD *)&v351[3112] = (char *)v357 - v163;
          v164 = (const char *)sub_480550(1471);
          MEMORY[0x49FA18](Str, v164, *(_DWORD *)&v351[3112]);
          sub_41C120(Str, 11861940);
        }
        a1[895877] = v357;
        return;
      case 654:
        *(_DWORD *)v375 = v11;
        *(_QWORD *)&v375[4] = v358;
        v375[12] = 0;
        dword_4AF0C0(&v375[12], a6 + 16);
        if ( v357 == *(_DWORD **)a1[895096] )
        {
          (*(void (__thiscall **)(_DWORD *, char *))(a1[794986] + 48))(a1 + 794986, v375);
        }
        else
        {
          v171 = sub_4280B0(a1, (int)v357);
          if ( v171 )
            (*(void (__thiscall **)(int, char *))(*(_DWORD *)v171 + 48))(v171, v375);
        }
        return;
      case 656:
        *(_DWORD *)v375 = v11;
        *(_QWORD *)&v375[4] = v358;
        v375[12] = 0;
        dword_4AF0C0(&v375[12], a6 + 16);
        if ( v357 == *(_DWORD **)a1[895096] )
        {
          (*(void (__thiscall **)(_DWORD *, char *))(a1[794986] + 44))(a1 + 794986, v375);
        }
        else
        {
          v172 = sub_4280B0(a1, (int)v357);
          if ( v172 )
            (*(void (__thiscall **)(int, char *))(*(_DWORD *)v172 + 44))(v172, v375);
        }
        return;
      case 657:
        *(_DWORD *)v375 = v11;
        *(_QWORD *)&v375[4] = v358;
        v375[12] = 0;
        dword_4AF0C0(&v375[12], a6 + 16);
        if ( v357 == *(_DWORD **)a1[895096] )
        {
          (*(void (__thiscall **)(_DWORD *, char *))(a1[794986] + 112))(a1 + 794986, v375);
        }
        else
        {
          v170 = (_BYTE *)sub_4280B0(a1, (int)v357);
          if ( v170 )
          {
            switch ( v170[69] )
            {
              case 0:
              case 1:
              case 3:
              case 4:
              case 5:
                (*(void (__thiscall **)(_BYTE *, char *))(*(_DWORD *)v170 + 112))(v170, v375);
                break;
              default:
                return;
            }
          }
        }
        return;
      case 658:
        sub_4204B0((int)(a1 + 902646));
        v379[sub_47D780(a6 + 16, (int)v379, 0x2000)] = 0;
        (*(void (__thiscall **)(_DWORD *, char *, int))(a1[903521] + 16))(a1 + 903521, v379, 1250);
        return;
      case 660:
        *(_DWORD *)&v351[3112] = 920;
        *(_DWORD *)&v351[3108] = 921;
        *(_DWORD *)&v351[3104] = 920;
        a1[771461] = 1;
        sub_41FD70(*(_DWORD *)&v351[3104], *(_DWORD *)&v351[3108], *(_DWORD *)&v351[3112]);
        return;
      case 661:
        switch ( v11 )
        {
          case 0xFFFFFFFC:
            *(_DWORD *)&v351[3112] = 0;
            *(_DWORD *)&v351[3108] = 0;
            *(_DWORD *)&v351[3104] = 0xFFFF;
            *(_DWORD *)&v351[3100] = -1;
            *(_DWORD *)&v351[3096] = -1;
            *(_DWORD *)&v351[3092] = 0;
            *(_DWORD *)&v351[3088] = sub_480550(1498);
            *(_DWORD *)&v351[3084] = 0;
            *(_DWORD *)&v351[3080] = 6;
            goto LABEL_630;
          case 0xFFFFFFFD:
            *(_DWORD *)&v351[3112] = 0;
            *(_DWORD *)&v351[3108] = 0;
            *(_DWORD *)&v351[3104] = 0xFFFF;
            *(_DWORD *)&v351[3100] = -1;
            *(_DWORD *)&v351[3096] = -1;
            *(_DWORD *)&v351[3092] = 0;
            *(_DWORD *)&v351[3088] = sub_480550(1497);
            *(_DWORD *)&v351[3084] = 0;
            *(_DWORD *)&v351[3080] = 6;
            goto LABEL_630;
          case 0xFFFFFFFE:
            *(_DWORD *)&v351[3112] = 0;
            *(_DWORD *)&v351[3108] = 0;
            *(_DWORD *)&v351[3104] = 0xFFFF;
            *(_DWORD *)&v351[3100] = -1;
            *(_DWORD *)&v351[3096] = -1;
            *(_DWORD *)&v351[3092] = 0;
            *(_DWORD *)&v351[3088] = sub_480550(1496);
            *(_DWORD *)&v351[3084] = 0;
            *(_DWORD *)&v351[3080] = 6;
            goto LABEL_630;
          case 0xFFFFFFFF:
            *(_DWORD *)&v351[3112] = 0;
            *(_DWORD *)&v351[3108] = 0;
            *(_DWORD *)&v351[3104] = 0xFFFF;
            *(_DWORD *)&v351[3100] = -1;
            *(_DWORD *)&v351[3096] = -1;
            *(_DWORD *)&v351[3092] = 0;
            *(_DWORD *)&v351[3088] = sub_480550(1495);
            *(_DWORD *)&v351[3084] = 0;
            *(_DWORD *)&v351[3080] = 6;
            goto LABEL_630;
          default:
            return;
        }
        return;
      case 664:
        switch ( v11 )
        {
          case 0xFFFFFFFB:
            *(_DWORD *)&v351[3112] = 0;
            *(_DWORD *)&v351[3108] = 0;
            *(_DWORD *)&v351[3104] = 0xFFFF;
            *(_DWORD *)&v351[3100] = -1;
            *(_DWORD *)&v351[3096] = -1;
            *(_DWORD *)&v351[3092] = 0;
            *(_DWORD *)&v351[3088] = 1502;
            goto LABEL_629;
          case 0xFFFFFFFC:
            *(_DWORD *)&v351[3112] = 0;
            *(_DWORD *)&v351[3108] = 0;
            *(_DWORD *)&v351[3104] = 0xFFFF;
            *(_DWORD *)&v351[3100] = -1;
            *(_DWORD *)&v351[3096] = -1;
            *(_DWORD *)&v351[3092] = 0;
            *(_DWORD *)&v351[3088] = 1498;
            goto LABEL_629;
          case 0xFFFFFFFD:
            *(_DWORD *)&v351[3112] = 0;
            *(_DWORD *)&v351[3108] = 0;
            *(_DWORD *)&v351[3104] = 0xFFFF;
            *(_DWORD *)&v351[3100] = -1;
            *(_DWORD *)&v351[3096] = -1;
            *(_DWORD *)&v351[3092] = 0;
            *(_DWORD *)&v351[3088] = 1501;
            goto LABEL_629;
          case 0xFFFFFFFE:
            goto LABEL_405;
          case 0xFFFFFFFF:
            goto LABEL_404;
          default:
            return;
        }
        return;
      case 665:
        switch ( v11 )
        {
          case 0xFFFFFFFD:
            *(_DWORD *)&v351[3112] = 0;
            *(_DWORD *)&v351[3108] = 0;
            *(_DWORD *)&v351[3104] = 0xFFFF;
            *(_DWORD *)&v351[3100] = -1;
            *(_DWORD *)&v351[3096] = -1;
            *(_DWORD *)&v351[3092] = 0;
            *(_DWORD *)&v351[3088] = 1503;
            goto LABEL_629;
          case 0xFFFFFFFE:
LABEL_405:
            *(_DWORD *)&v351[3112] = 0;
            *(_DWORD *)&v351[3108] = 0;
            *(_DWORD *)&v351[3104] = 0xFFFF;
            *(_DWORD *)&v351[3100] = -1;
            *(_DWORD *)&v351[3096] = -1;
            *(_DWORD *)&v351[3092] = 0;
            *(_DWORD *)&v351[3088] = 1500;
            goto LABEL_629;
          case 0xFFFFFFFF:
LABEL_404:
            *(_DWORD *)&v351[3112] = 0;
            *(_DWORD *)&v351[3108] = 0;
            *(_DWORD *)&v351[3104] = 0xFFFF;
            *(_DWORD *)&v351[3100] = -1;
            *(_DWORD *)&v351[3096] = -1;
            *(_DWORD *)&v351[3092] = 0;
            *(_DWORD *)&v351[3088] = 1499;
            goto LABEL_629;
        }
        return;
      case 666:
        sub_4308C0(1, 0);
        return;
      case 667:
        v356 = (int)(a1 + 771230);
        sub_4308C0(1, 0);
        v165 = sub_47D780(a6 + 16, (int)v379, 0x2000);
        *(_DWORD *)&v351[3112] = 47;
        *(_DWORD *)&v351[3108] = v379;
        v379[v165] = 0;
        v166 = v379;
        for ( i = MEMORY[0x49FFB0](*(const char **)&v351[3108], *(int *)&v351[3112]); i; i = MEMORY[0x49FFB0](v168, 47) )
        {
          *i = 0;
          v168 = i + 1;
          v169 = (char *)MEMORY[0x4A01AA](0x20u);
          if ( v169 )
          {
            strcpy(v169, v166);
            (*(void (__thiscall **)(int, char *))(*(_DWORD *)v356 + 4))(v356, v169);
          }
          v166 = (char *)v168;
        }
        return;
      case 668:
        if ( !a1[702527] )
          sub_43AB90(a1 + 695155, 0);
        v148 = a1 + 702515;
        *(_DWORD *)&v351[3112] = 1;
        goto LABEL_356;
      case 669:
        a1[895877] = v11;
        if ( a1[738872] )
        {
          v151 = *(_DWORD *)((char *)a1 + 2955497);
          *(_DWORD *)&v351[3112] = 11861940;
          v152 = &a1[780 * v151];
          *(_DWORD *)&v351[3108] = 1456;
          *((_WORD *)v152 + 1406025) = v13;
          *((_WORD *)v152 + 1406026) = WORD2(v358);
          v153 = (char *)sub_480550(*(int *)&v351[3108]);
          sub_41C120(v153, *(int *)&v351[3112]);
        }
        goto LABEL_366;
      case 670:
        *(_DWORD *)&v351[3112] = 0;
        *(_DWORD *)&v351[3108] = 0;
        *(_DWORD *)&v351[3104] = 0xFFFF;
        *(_DWORD *)&v351[3100] = -1;
        *(_DWORD *)&v351[3096] = -1;
        *(_DWORD *)&v351[3092] = 0;
        *(_DWORD *)&v351[3088] = 1457;
        goto LABEL_368;
      case 671:
        if ( !a1[738872] )
          return;
        a1[738882] = v11;
        *(_DWORD *)&v351[3112] = v11;
LABEL_360:
        *(_DWORD *)&v351[3108] = sub_480550(1452);
        MEMORY[0x49FA18](a1 + 738877, *(const char **)&v351[3108], *(_DWORD *)&v351[3112]);
LABEL_361:
        if ( v357 )
          *((_BYTE *)a1 + 2955492) = 2;
        else
          *((_BYTE *)a1 + 2955492) = 0;
        return;
      case 673:
        if ( !a1[749869] )
        {
          sub_43AB90(a1 + 695155, 3);
          sub_41FD70(1392, 1396, 1392);
          v371[sub_47D780(a6 + 16, (int)v371, 60)] = 0;
          strcpy((char *)a1 + 3075412, v371);
        }
        if ( !a1[702527] )
        {
          sub_43AB90(a1 + 695155, 0);
          sub_4318D0(a1 + 702515, 518, 0, 1);
        }
        return;
      case 674:
        *(_DWORD *)&v351[3112] = 0;
        *(_DWORD *)&v351[3108] = 0;
        *(_DWORD *)&v351[3104] = 0xFFFF;
        *(_DWORD *)&v351[3100] = -1;
        *(_DWORD *)&v351[3096] = -1;
        *(_DWORD *)&v351[3092] = 0;
        *(_DWORD *)&v351[3088] = sub_480550(1434);
        *(_DWORD *)&v351[3084] = 0;
        *(_DWORD *)&v351[3080] = 2;
        goto LABEL_630;
      case 675:
        v356 = v11;
        v130 = WORD1(v358);
        v131 = a1 + 768872;
        if ( !WORD1(v358) )
        {
          sub_43ED40((char *)a1 + 3075488);
          qmemcpy(&v351[8], v131, 0xC24u);
          v132 = sub_41AD30(dword_7B7D50);
          if ( v132 != -1 )
            sub_41AE40(v132);
          sub_41DC30(a1 + 749857);
          goto LABEL_315;
        }
        qmemcpy((void *)(a1[769689] + 24), v131, 0xC24u);
        *(_WORD *)(a1[769689] + 99) = WORD1(v358);
        qmemcpy(&v351[8], (const void *)(a1[769689] + 24), 0xC24u);
        v133 = sub_41AD30(dword_7B7D50);
        if ( v133 != -1 )
          word_7B7F0B[1556 * v133] = v130;
        v134 = v356;
        *(_DWORD *)a1[769689] = 1;
        *(_WORD *)((char *)a1 + 3075563) -= v130;
        *(_DWORD *)((char *)a1 + 3075554) = v134;
        sub_4410D0(0, a1[769689]);
        sub_41DC30(a1 + 749857);
        memset(a1 + 768871, 0, 0xC28u);
        return;
      case 676:
        if ( a1[768871] )
        {
          sub_43ED40((char *)a1 + 3075488);
          *(_DWORD *)&v351[3112] = 0;
          *(_DWORD *)&v351[3108] = 0;
          qmemcpy(v351, a1 + 768872, 0xC24u);
          sub_440CB0(
            (_WORD *)a1 + 1405030,
            *(__int16 *)v351,
            *(int *)&v351[4],
            *(int *)&v351[8],
            *(int *)&v351[12],
            *(int *)&v351[16],
            *(int *)&v351[20],
            *(int *)&v351[24],
            *(int *)&v351[28],
            *(int *)&v351[32],
            *(int *)&v351[36],
            *(int *)&v351[40],
            *(int *)&v351[44],
            *(int *)&v351[48],
            *(int *)&v351[52],
            *(int *)&v351[56],
            *(int *)&v351[60],
            *(int *)&v351[64],
            *(int *)&v351[68],
            *(int *)&v351[72],
            *(int *)&v351[76],
            *(int *)&v351[80],
            *(int *)&v351[84],
            *(int *)&v351[88],
            *(int *)&v351[92],
            *(int *)&v351[96],
            *(int *)&v351[100],
            *(int *)&v351[104],
            *(int *)&v351[108],
            *(int *)&v351[112],
            *(int *)&v351[116],
            *(int *)&v351[120],
            *(int *)&v351[124],
            *(int *)&v351[128],
            *(int *)&v351[132],
            *(int *)&v351[136],
            *(int *)&v351[140],
            *(int *)&v351[144],
            *(int *)&v351[148],
            *(int *)&v351[152],
            *(int *)&v351[156],
            *(int *)&v351[160],
            *(int *)&v351[164],
            *(int *)&v351[168],
            *(int *)&v351[172],
            *(int *)&v351[176],
            *(int *)&v351[180],
            *(int *)&v351[184],
            *(int *)&v351[188],
            *(int *)&v351[192],
            *(int *)&v351[196],
            *(int *)&v351[200],
            *(int *)&v351[204],
            *(int *)&v351[208],
            *(int *)&v351[212],
            *(int *)&v351[216],
            *(int *)&v351[220],
            *(int *)&v351[224],
            *(int *)&v351[228],
            *(int *)&v351[232],
            *(int *)&v351[236],
            *(int *)&v351[240],
            *(int *)&v351[244]);
        }
        memset(a1 + 768871, 0, 0xC28u);
        memset(a1 + 695163, 0, 0xC3Cu);
        return;
      case 677:
        v135 = a1[695163];
        v361 = (int)(a1 + 695163);
        if ( !v135 )
          return;
        j = (char *)(a1 + 695169);
        sub_43ED40((char *)a1 + 2780676);
        if ( !strcmp((const char *)a1 + 2780680, (const char *)sub_480550(1050)) )
          goto LABEL_280;
        if ( *((_BYTE *)a1 + 2780741) < 2u )
        {
          *(_DWORD *)&v351[3112] = 0;
          *(_DWORD *)&v351[3108] = 0;
          qmemcpy(v351, j, 0xC24u);
          v137 = a1 + 702515;
        }
        else
        {
          v136 = a1 + 702515;
          qmemcpy(&v351[8], j, 0xC24u);
          if ( sub_441EE0(v136) )
            goto LABEL_280;
          *(_DWORD *)&v351[3112] = 0;
          *(_DWORD *)&v351[3108] = 0;
          qmemcpy(v351, j, 0xC24u);
          v137 = v136;
        }
        sub_440CB0(
          v137,
          *(__int16 *)v351,
          *(int *)&v351[4],
          *(int *)&v351[8],
          *(int *)&v351[12],
          *(int *)&v351[16],
          *(int *)&v351[20],
          *(int *)&v351[24],
          *(int *)&v351[28],
          *(int *)&v351[32],
          *(int *)&v351[36],
          *(int *)&v351[40],
          *(int *)&v351[44],
          *(int *)&v351[48],
          *(int *)&v351[52],
          *(int *)&v351[56],
          *(int *)&v351[60],
          *(int *)&v351[64],
          *(int *)&v351[68],
          *(int *)&v351[72],
          *(int *)&v351[76],
          *(int *)&v351[80],
          *(int *)&v351[84],
          *(int *)&v351[88],
          *(int *)&v351[92],
          *(int *)&v351[96],
          *(int *)&v351[100],
          *(int *)&v351[104],
          *(int *)&v351[108],
          *(int *)&v351[112],
          *(int *)&v351[116],
          *(int *)&v351[120],
          *(int *)&v351[124],
          *(int *)&v351[128],
          *(int *)&v351[132],
          *(int *)&v351[136],
          *(int *)&v351[140],
          *(int *)&v351[144],
          *(int *)&v351[148],
          *(int *)&v351[152],
          *(int *)&v351[156],
          *(int *)&v351[160],
          *(int *)&v351[164],
          *(int *)&v351[168],
          *(int *)&v351[172],
          *(int *)&v351[176],
          *(int *)&v351[180],
          *(int *)&v351[184],
          *(int *)&v351[188],
          *(int *)&v351[192],
          *(int *)&v351[196],
          *(int *)&v351[200],
          *(int *)&v351[204],
          *(int *)&v351[208],
          *(int *)&v351[212],
          *(int *)&v351[216],
          *(int *)&v351[220],
          *(int *)&v351[224],
          *(int *)&v351[228],
          *(int *)&v351[232],
          *(int *)&v351[236],
          *(int *)&v351[240],
          *(int *)&v351[244]);
        goto LABEL_280;
      case 678:
        v138 = a1[695163];
        v361 = (int)(a1 + 695163);
        if ( v138 )
        {
          sub_43ED40((char *)a1 + 2780676);
          *(_DWORD *)&v351[3112] = 0;
          *(_DWORD *)&v351[3108] = 0;
          qmemcpy(v351, a1 + 695169, 0xC24u);
          sub_41CFF0(a1 + 749857, 0);
          memset((void *)v361, 0, 0xC3Cu);
        }
        return;
      case 681:
        v356 = 12;
        v361 = (int)a1 + 3000609;
        do
        {
          if ( *(_DWORD *)(v361 - 77) )
          {
            if ( *(_BYTE *)v361 < 2u )
            {
              *(_DWORD *)&v351[3112] = 0;
              *(_DWORD *)&v351[3108] = 0;
              qmemcpy(v351, (const void *)(v361 - 65), 0xC24u);
              sub_440CB0(
                (_WORD *)a1 + 1405030,
                *(__int16 *)v351,
                *(int *)&v351[4],
                *(int *)&v351[8],
                *(int *)&v351[12],
                *(int *)&v351[16],
                *(int *)&v351[20],
                *(int *)&v351[24],
                *(int *)&v351[28],
                *(int *)&v351[32],
                *(int *)&v351[36],
                *(int *)&v351[40],
                *(int *)&v351[44],
                *(int *)&v351[48],
                *(int *)&v351[52],
                *(int *)&v351[56],
                *(int *)&v351[60],
                *(int *)&v351[64],
                *(int *)&v351[68],
                *(int *)&v351[72],
                *(int *)&v351[76],
                *(int *)&v351[80],
                *(int *)&v351[84],
                *(int *)&v351[88],
                *(int *)&v351[92],
                *(int *)&v351[96],
                *(int *)&v351[100],
                *(int *)&v351[104],
                *(int *)&v351[108],
                *(int *)&v351[112],
                *(int *)&v351[116],
                *(int *)&v351[120],
                *(int *)&v351[124],
                *(int *)&v351[128],
                *(int *)&v351[132],
                *(int *)&v351[136],
                *(int *)&v351[140],
                *(int *)&v351[144],
                *(int *)&v351[148],
                *(int *)&v351[152],
                *(int *)&v351[156],
                *(int *)&v351[160],
                *(int *)&v351[164],
                *(int *)&v351[168],
                *(int *)&v351[172],
                *(int *)&v351[176],
                *(int *)&v351[180],
                *(int *)&v351[184],
                *(int *)&v351[188],
                *(int *)&v351[192],
                *(int *)&v351[196],
                *(int *)&v351[200],
                *(int *)&v351[204],
                *(int *)&v351[208],
                *(int *)&v351[212],
                *(int *)&v351[216],
                *(int *)&v351[220],
                *(int *)&v351[224],
                *(int *)&v351[228],
                *(int *)&v351[232],
                *(int *)&v351[236],
                *(int *)&v351[240],
                *(int *)&v351[244]);
            }
            else
            {
              memset(a1 + 695163, 0, 0xC3Cu);
            }
          }
          v145 = v356 == 1;
          v361 += 3120;
          --v356;
        }
        while ( !v145 );
        if ( !a1[695163] || !a1[695166] )
          goto LABEL_303;
        if ( !strcmp((const char *)a1 + 2780680, (const char *)sub_480550(1050)) )
          goto LABEL_302;
        if ( *((_BYTE *)a1 + 2780741) < 2u )
        {
          *(_DWORD *)&v351[3112] = 0;
          *(_DWORD *)&v351[3108] = 0;
          v128 = a1 + 695169;
        }
        else
        {
          qmemcpy(&v351[8], a1 + 695169, 0xC24u);
          if ( sub_441EE0(a1 + 702515) )
            goto LABEL_302;
          *(_DWORD *)&v351[3112] = 0;
          *(_DWORD *)&v351[3108] = 0;
          v128 = a1 + 695169;
        }
        qmemcpy(v351, v128, 0xC24u);
        sub_440CB0(
          (_WORD *)a1 + 1405030,
          *(__int16 *)v351,
          *(int *)&v351[4],
          *(int *)&v351[8],
          *(int *)&v351[12],
          *(int *)&v351[16],
          *(int *)&v351[20],
          *(int *)&v351[24],
          *(int *)&v351[28],
          *(int *)&v351[32],
          *(int *)&v351[36],
          *(int *)&v351[40],
          *(int *)&v351[44],
          *(int *)&v351[48],
          *(int *)&v351[52],
          *(int *)&v351[56],
          *(int *)&v351[60],
          *(int *)&v351[64],
          *(int *)&v351[68],
          *(int *)&v351[72],
          *(int *)&v351[76],
          *(int *)&v351[80],
          *(int *)&v351[84],
          *(int *)&v351[88],
          *(int *)&v351[92],
          *(int *)&v351[96],
          *(int *)&v351[100],
          *(int *)&v351[104],
          *(int *)&v351[108],
          *(int *)&v351[112],
          *(int *)&v351[116],
          *(int *)&v351[120],
          *(int *)&v351[124],
          *(int *)&v351[128],
          *(int *)&v351[132],
          *(int *)&v351[136],
          *(int *)&v351[140],
          *(int *)&v351[144],
          *(int *)&v351[148],
          *(int *)&v351[152],
          *(int *)&v351[156],
          *(int *)&v351[160],
          *(int *)&v351[164],
          *(int *)&v351[168],
          *(int *)&v351[172],
          *(int *)&v351[176],
          *(int *)&v351[180],
          *(int *)&v351[184],
          *(int *)&v351[188],
          *(int *)&v351[192],
          *(int *)&v351[196],
          *(int *)&v351[200],
          *(int *)&v351[204],
          *(int *)&v351[208],
          *(int *)&v351[212],
          *(int *)&v351[216],
          *(int *)&v351[220],
          *(int *)&v351[224],
          *(int *)&v351[228],
          *(int *)&v351[232],
          *(int *)&v351[236],
          *(int *)&v351[240],
          *(int *)&v351[244]);
LABEL_302:
        memset(a1 + 695163, 0, 0xC3Cu);
LABEL_303:
        a1[895877] += a1[768869];
        if ( !a1[768871] )
          goto LABEL_309;
        if ( *((_BYTE *)a1 + 3075553) < 2u )
        {
          *(_DWORD *)&v351[3112] = 0;
          *(_DWORD *)&v351[3108] = 0;
          v129 = a1 + 768872;
        }
        else
        {
          qmemcpy(&v351[8], a1 + 768872, 0xC24u);
          if ( sub_441EE0(a1 + 702515) )
            goto LABEL_309;
          *(_DWORD *)&v351[3112] = 0;
          *(_DWORD *)&v351[3108] = 0;
          v129 = a1 + 768872;
        }
        qmemcpy(v351, v129, 0xC24u);
        sub_440CB0(
          (_WORD *)a1 + 1405030,
          *(__int16 *)v351,
          *(int *)&v351[4],
          *(int *)&v351[8],
          *(int *)&v351[12],
          *(int *)&v351[16],
          *(int *)&v351[20],
          *(int *)&v351[24],
          *(int *)&v351[28],
          *(int *)&v351[32],
          *(int *)&v351[36],
          *(int *)&v351[40],
          *(int *)&v351[44],
          *(int *)&v351[48],
          *(int *)&v351[52],
          *(int *)&v351[56],
          *(int *)&v351[60],
          *(int *)&v351[64],
          *(int *)&v351[68],
          *(int *)&v351[72],
          *(int *)&v351[76],
          *(int *)&v351[80],
          *(int *)&v351[84],
          *(int *)&v351[88],
          *(int *)&v351[92],
          *(int *)&v351[96],
          *(int *)&v351[100],
          *(int *)&v351[104],
          *(int *)&v351[108],
          *(int *)&v351[112],
          *(int *)&v351[116],
          *(int *)&v351[120],
          *(int *)&v351[124],
          *(int *)&v351[128],
          *(int *)&v351[132],
          *(int *)&v351[136],
          *(int *)&v351[140],
          *(int *)&v351[144],
          *(int *)&v351[148],
          *(int *)&v351[152],
          *(int *)&v351[156],
          *(int *)&v351[160],
          *(int *)&v351[164],
          *(int *)&v351[168],
          *(int *)&v351[172],
          *(int *)&v351[176],
          *(int *)&v351[180],
          *(int *)&v351[184],
          *(int *)&v351[188],
          *(int *)&v351[192],
          *(int *)&v351[196],
          *(int *)&v351[200],
          *(int *)&v351[204],
          *(int *)&v351[208],
          *(int *)&v351[212],
          *(int *)&v351[216],
          *(int *)&v351[220],
          *(int *)&v351[224],
          *(int *)&v351[228],
          *(int *)&v351[232],
          *(int *)&v351[236],
          *(int *)&v351[240],
          *(int *)&v351[244]);
LABEL_309:
        sub_41C7E0(a1 + 749857);
        if ( a1[749869] )
        {
          sub_43AA30(a1 + 695155, 3);
          (*(void (__thiscall **)(_DWORD *, _DWORD))(a1[749857] + 16))(a1 + 749857, 0);
        }
        break;
      case 682:
        sub_47D780(a6 + 16, (int)v371, 100);
        sub_4420D0(v379);
        v386 = 0;
        qmemcpy(&v351[3012], v371, 0x64u);
        sub_4420F0(
          v379,
          v351[3012],
          *(int *)&v351[3016],
          *(int *)&v351[3020],
          *(int *)&v351[3024],
          *(int *)&v351[3028],
          *(int *)&v351[3032],
          *(int *)&v351[3036],
          *(int *)&v351[3040],
          *(int *)&v351[3044],
          *(int *)&v351[3048],
          *(int *)&v351[3052],
          *(int *)&v351[3056],
          *(int *)&v351[3060],
          *(int *)&v351[3064],
          *(int *)&v351[3068],
          *(int *)&v351[3072],
          *(int *)&v351[3076],
          *(int *)&v351[3080],
          *(int *)&v351[3084],
          *(int *)&v351[3088],
          *(int *)&v351[3092],
          *(int *)&v351[3096],
          *(int *)&v351[3100],
          *(int *)&v351[3104],
          *(int *)&v351[3108],
          0);
        *(_DWORD *)&v351[3112] = 0;
        *(_DWORD *)&v351[3108] = 0;
        qmemcpy(v351, v379, 0xC24u);
        sub_41CFF0(a1 + 749857, 1);
        sub_43ED40(v379);
        *(_DWORD *)&v351[3112] = &v379[4];
        v141 = (const char *)sub_480550(1435);
        MEMORY[0x49FA18](Str, v141);
        sub_41C120(Str, 11861940);
        v386 = -1;
        nullsub_5();
        return;
      case 683:
        sub_47D780(a6 + 16, (int)v371, 100);
        sub_4420D0(v379);
        v386 = 1;
        qmemcpy(&v351[3012], v371, 0x64u);
        sub_4420F0(
          v379,
          v351[3012],
          *(int *)&v351[3016],
          *(int *)&v351[3020],
          *(int *)&v351[3024],
          *(int *)&v351[3028],
          *(int *)&v351[3032],
          *(int *)&v351[3036],
          *(int *)&v351[3040],
          *(int *)&v351[3044],
          *(int *)&v351[3048],
          *(int *)&v351[3052],
          *(int *)&v351[3056],
          *(int *)&v351[3060],
          *(int *)&v351[3064],
          *(int *)&v351[3068],
          *(int *)&v351[3072],
          *(int *)&v351[3076],
          *(int *)&v351[3080],
          *(int *)&v351[3084],
          *(int *)&v351[3088],
          *(int *)&v351[3092],
          *(int *)&v351[3096],
          *(int *)&v351[3100],
          *(int *)&v351[3104],
          *(int *)&v351[3108],
          0);
        sub_43ED40(v379);
        sub_41CF60(1, *(_DWORD *)&v371[62], v371);
        *(_DWORD *)&v351[3112] = v371;
        v142 = (const char *)sub_480550(1436);
        MEMORY[0x49FA18](Str, v142);
        sub_41C120(Str, 11861940);
        v386 = -1;
        nullsub_5();
        return;
      case 684:
        sub_4832D0(106, 0, 0, 0);
        v139 = WORD2(v358) << 16;
        v140 = WORD1(v358);
        a1[768869] = v357;
        a1[895877] = v140 | v139;
        return;
      default:
        return;
    }
    return;
  }
  if ( (unsigned __int16)v358 == 636 )
  {
    v124 = a1[695163];
    v361 = (int)(a1 + 695163);
    if ( v124 )
    {
      if ( *((_BYTE *)a1 + 2780710) == 51 )
      {
        v125 = (const char *)sub_480550(1414);
        sub_420540((int)(a1 + 902646), (int)dword_5A041C, 103, 0, v125, 0, -1, -1, 0xFFFF, 0, 0, v352, v353);
      }
      if ( !a1[695167] && strcmp((const char *)a1 + 2780680, (const char *)sub_480550(1050)) )
      {
        v126 = *((__int16 *)a1 + 1390339);
        *(_DWORD *)&v351[3112] = 1;
        *(_DWORD *)&v351[3108] = v126;
        qmemcpy(v351, a1 + 695169, 0xC24u);
        sub_440CB0(
          (_WORD *)a1 + 1405030,
          *(__int16 *)v351,
          *(int *)&v351[4],
          *(int *)&v351[8],
          *(int *)&v351[12],
          *(int *)&v351[16],
          *(int *)&v351[20],
          *(int *)&v351[24],
          *(int *)&v351[28],
          *(int *)&v351[32],
          *(int *)&v351[36],
          *(int *)&v351[40],
          *(int *)&v351[44],
          *(int *)&v351[48],
          *(int *)&v351[52],
          *(int *)&v351[56],
          *(int *)&v351[60],
          *(int *)&v351[64],
          *(int *)&v351[68],
          *(int *)&v351[72],
          *(int *)&v351[76],
          *(int *)&v351[80],
          *(int *)&v351[84],
          *(int *)&v351[88],
          *(int *)&v351[92],
          *(int *)&v351[96],
          *(int *)&v351[100],
          *(int *)&v351[104],
          *(int *)&v351[108],
          *(int *)&v351[112],
          *(int *)&v351[116],
          *(int *)&v351[120],
          *(int *)&v351[124],
          *(int *)&v351[128],
          *(int *)&v351[132],
          *(int *)&v351[136],
          *(int *)&v351[140],
          *(int *)&v351[144],
          *(int *)&v351[148],
          *(int *)&v351[152],
          *(int *)&v351[156],
          *(int *)&v351[160],
          *(int *)&v351[164],
          *(int *)&v351[168],
          *(int *)&v351[172],
          *(int *)&v351[176],
          *(int *)&v351[180],
          *(int *)&v351[184],
          *(int *)&v351[188],
          *(int *)&v351[192],
          *(int *)&v351[196],
          *(int *)&v351[200],
          *(int *)&v351[204],
          *(int *)&v351[208],
          *(int *)&v351[212],
          *(int *)&v351[216],
          *(int *)&v351[220],
          *(int *)&v351[224],
          *(int *)&v351[228],
          *(int *)&v351[232],
          *(int *)&v351[236],
          *(int *)&v351[240],
          *(int *)&v351[244]);
        memset((void *)v361, 0, 0xC3Cu);
        return;
      }
      v127 = *((__int16 *)a1 + 1390339);
      *(_DWORD *)&v351[3112] = 1;
      *(_DWORD *)&v351[3108] = v127;
      qmemcpy(v351, a1 + 695169, 0xC24u);
      sub_41AC50(a1 + 784694);
    }
LABEL_280:
    memset((void *)v361, 0, 0xC3Cu);
    return;
  }
  if ( (unsigned __int16)v358 > 0xCBu )
  {
    if ( (unsigned __int16)v358 > 0x266u )
    {
      switch ( (__int16)v358 )
      {
        case 615:
          v121 = v11;
          sub_43ED40((char *)a1 + 2993820);
          dword_8245B1 = *(_DWORD *)((char *)&v358 + 2);
          sub_414650((int)&dword_7C1E20, v121, unk_8245B0, SBYTE5(v358));
          sub_4766F0((__int16 *)a1 + 1477772);
          memset(a1 + 695163, 0, 0xC3Cu);
          return;
        case 616:
          sub_43ED40((char *)a1 + 2993820);
          if ( a1[748454] )
          {
            *(_DWORD *)&v351[3112] = 0;
            *(_DWORD *)&v351[3108] = 0;
            qmemcpy(v351, a1 + 748455, 0xC24u);
            sub_440CB0(
              (_WORD *)a1 + 1405030,
              *(__int16 *)v351,
              *(int *)&v351[4],
              *(int *)&v351[8],
              *(int *)&v351[12],
              *(int *)&v351[16],
              *(int *)&v351[20],
              *(int *)&v351[24],
              *(int *)&v351[28],
              *(int *)&v351[32],
              *(int *)&v351[36],
              *(int *)&v351[40],
              *(int *)&v351[44],
              *(int *)&v351[48],
              *(int *)&v351[52],
              *(int *)&v351[56],
              *(int *)&v351[60],
              *(int *)&v351[64],
              *(int *)&v351[68],
              *(int *)&v351[72],
              *(int *)&v351[76],
              *(int *)&v351[80],
              *(int *)&v351[84],
              *(int *)&v351[88],
              *(int *)&v351[92],
              *(int *)&v351[96],
              *(int *)&v351[100],
              *(int *)&v351[104],
              *(int *)&v351[108],
              *(int *)&v351[112],
              *(int *)&v351[116],
              *(int *)&v351[120],
              *(int *)&v351[124],
              *(int *)&v351[128],
              *(int *)&v351[132],
              *(int *)&v351[136],
              *(int *)&v351[140],
              *(int *)&v351[144],
              *(int *)&v351[148],
              *(int *)&v351[152],
              *(int *)&v351[156],
              *(int *)&v351[160],
              *(int *)&v351[164],
              *(int *)&v351[168],
              *(int *)&v351[172],
              *(int *)&v351[176],
              *(int *)&v351[180],
              *(int *)&v351[184],
              *(int *)&v351[188],
              *(int *)&v351[192],
              *(int *)&v351[196],
              *(int *)&v351[200],
              *(int *)&v351[204],
              *(int *)&v351[208],
              *(int *)&v351[212],
              *(int *)&v351[216],
              *(int *)&v351[220],
              *(int *)&v351[224],
              *(int *)&v351[228],
              *(int *)&v351[232],
              *(int *)&v351[236],
              *(int *)&v351[240],
              *(int *)&v351[244]);
            memset(a1 + 748454, 0, 0xC28u);
          }
          if ( !a1[695163] )
            return;
          *(_DWORD *)&v351[3112] = a1 + 695163;
          break;
        case 619:
          sub_414650((int)&dword_7C1E20, v11, unk_8245B0, SHIBYTE(dword_8245B1));
          sub_43ED40((char *)a1 + 2780676);
          memset(a1 + 695163, 0, 0xC3Cu);
          return;
        case 620:
          if ( !a1[695163] )
            return;
          sub_43ED40((char *)a1 + 2780676);
          *(_DWORD *)&v351[3112] = a1 + 695163;
          break;
        case 621:
          memset(v375, 0, 0x64u);
          j = a6 + 16;
          v356 = 0;
          v361 = (int)v375;
          do
          {
            v102 = MEMORY[0x49FFB0](j, 47);
            if ( !v102 )
              break;
            *v102 = 0;
            v103 = v102 + 1;
            v104 = (__int16 *)MEMORY[0x49FFB0](v102 + 1, 47);
            v362 = v104;
            if ( !v104 )
              break;
            v105 = (const void *)v361;
            *(_DWORD *)&v351[3112] = 100;
            *(_DWORD *)&v351[3108] = v361;
            *(_DWORD *)&v351[3104] = v103;
            *(_BYTE *)v104 = 0;
            sub_47D780(*(const char **)&v351[3104], *(int *)&v351[3108], *(int *)&v351[3112]);
            sub_4420D0(v379);
            v386 = 9;
            qmemcpy(&v351[3012], v105, 0x64u);
            sub_4420F0(
              v379,
              v351[3012],
              *(int *)&v351[3016],
              *(int *)&v351[3020],
              *(int *)&v351[3024],
              *(int *)&v351[3028],
              *(int *)&v351[3032],
              *(int *)&v351[3036],
              *(int *)&v351[3040],
              *(int *)&v351[3044],
              *(int *)&v351[3048],
              *(int *)&v351[3052],
              *(int *)&v351[3056],
              *(int *)&v351[3060],
              *(int *)&v351[3064],
              *(int *)&v351[3068],
              *(int *)&v351[3072],
              *(int *)&v351[3076],
              *(int *)&v351[3080],
              *(int *)&v351[3084],
              *(int *)&v351[3088],
              *(int *)&v351[3092],
              *(int *)&v351[3096],
              *(int *)&v351[3100],
              *(int *)&v351[3104],
              *(int *)&v351[3108],
              0);
            v106 = 389 * MEMORY[0x49FAF5](j);
            *(_DWORD *)&v351[3108] = j;
            qmemcpy(&a1[2 * v106 + 739119], v379, 0xC24u);
            v107 = MEMORY[0x49FAF5](*(_DWORD *)&v351[3108]);
            v386 = -1;
            v108 = 389 * v107;
            v109 = (char *)v362 + 1;
            a1[2 * v108 + 739118] = 1;
            j = v109;
            nullsub_5();
            v110 = ++v356 < 12;
            v361 += 100;
          }
          while ( v110 );
          return;
        case 622:
          v122 = BYTE2(v358);
          v123 = BYTE4(v358);
          *(_WORD *)((char *)a1 + 3583543) = (_WORD)v357;
          *((_BYTE *)a1 + 3583547) = v122;
          *((_BYTE *)a1 + 3583549) = v123;
          return;
        case 633:
          goto LABEL_584;
        case 634:
          goto LABEL_876;
        case 635:
          memset(Str, 0, sizeof(Str));
          for ( j = 0; (int)j < 10; ++j )
          {
            v111 = (const char *)sub_480550(1409);
            MEMORY[0x49FA18](v369, "%s%d", v111, j);
            if ( !strcmp((const char *)a1 + 2780680, (const char *)v369) )
            {
              v112 = (const char *)sub_480550(1409);
              MEMORY[0x49FA18](Str, v112);
            }
          }
          if ( !Str[0] )
            MEMORY[0x49FA18](Str, (const char *)a1 + 2780680);
          v113 = (const char *)sub_480550(1464);
          MEMORY[0x49FA18](v374, v113, Str);
          sub_41C120(v374, 11861940);
          sub_43EED0((int)(a1 + 695169));
          if ( a1[695167] )
          {
            if ( WORD1(v358) )
            {
              v114 = (const char *)sub_480550(1467);
              MEMORY[0x49FA18](v374, v114, Str, WORD1(v358));
            }
            else
            {
              v115 = (const char *)sub_480550(1468);
              MEMORY[0x49FA18](v374, v115, Str);
            }
            sub_41C120(v374, 11861940);
          }
          if ( *((_BYTE *)a1 + 2780741) != 3 )
            goto LABEL_315;
          if ( *(_WORD *)((char *)a1 + 2780751) <= 1u )
          {
            qmemcpy(&v351[8], a1 + 695169, 0xC24u);
            v120 = sub_41AD30(a1 + 784694);
            if ( v120 != -1 )
              sub_41AE40(v120);
            sub_440C00((__int16 *)a1 + 1405030, *(_DWORD *)((char *)a1 + 2780742), (const char *)a1 + 2780680);
            memset(a1 + 695163, 0, 0xC3Cu);
          }
          else
          {
            v116 = *(_DWORD *)((char *)a1 + 2780742);
            v117 = WORD1(v358);
            j = (char *)WORD1(v358);
            sub_441DD0((char *)a1 + 2810060, v116, WORD1(v358));
            if ( !a1[695167] )
            {
              qmemcpy(&v351[8], a1 + 695169, 0xC24u);
              v118 = sub_41AD30(a1 + 784694);
              if ( v118 != -1 )
                *(_WORD *)((char *)&a1[778 * v118 + 784804] + 3) = (_WORD)j;
              v117 = (__int16)j;
            }
            *(_WORD *)((char *)a1 + 2780751) = v117;
            if ( a1[695167] )
            {
              sub_41A8A0(*((__int16 *)a1 + 1390339), a1 + 695163);
              memset(a1 + 695163, 0, 0xC3Cu);
            }
            else
            {
              v119 = *((__int16 *)a1 + 1390339);
              *(_DWORD *)&v351[3112] = 1;
              *(_DWORD *)&v351[3108] = v119;
              qmemcpy(v351, a1 + 695169, 0xC24u);
              sub_440CB0(
                (_WORD *)a1 + 1405030,
                *(__int16 *)v351,
                *(int *)&v351[4],
                *(int *)&v351[8],
                *(int *)&v351[12],
                *(int *)&v351[16],
                *(int *)&v351[20],
                *(int *)&v351[24],
                *(int *)&v351[28],
                *(int *)&v351[32],
                *(int *)&v351[36],
                *(int *)&v351[40],
                *(int *)&v351[44],
                *(int *)&v351[48],
                *(int *)&v351[52],
                *(int *)&v351[56],
                *(int *)&v351[60],
                *(int *)&v351[64],
                *(int *)&v351[68],
                *(int *)&v351[72],
                *(int *)&v351[76],
                *(int *)&v351[80],
                *(int *)&v351[84],
                *(int *)&v351[88],
                *(int *)&v351[92],
                *(int *)&v351[96],
                *(int *)&v351[100],
                *(int *)&v351[104],
                *(int *)&v351[108],
                *(int *)&v351[112],
                *(int *)&v351[116],
                *(int *)&v351[120],
                *(int *)&v351[124],
                *(int *)&v351[128],
                *(int *)&v351[132],
                *(int *)&v351[136],
                *(int *)&v351[140],
                *(int *)&v351[144],
                *(int *)&v351[148],
                *(int *)&v351[152],
                *(int *)&v351[156],
                *(int *)&v351[160],
                *(int *)&v351[164],
                *(int *)&v351[168],
                *(int *)&v351[172],
                *(int *)&v351[176],
                *(int *)&v351[180],
                *(int *)&v351[184],
                *(int *)&v351[188],
                *(int *)&v351[192],
                *(int *)&v351[196],
                *(int *)&v351[200],
                *(int *)&v351[204],
                *(int *)&v351[208],
                *(int *)&v351[212],
                *(int *)&v351[216],
                *(int *)&v351[220],
                *(int *)&v351[224],
                *(int *)&v351[228],
                *(int *)&v351[232],
                *(int *)&v351[236],
                *(int *)&v351[240],
                *(int *)&v351[244]);
              memset(a1 + 695163, 0, 0xC3Cu);
            }
          }
          return;
        default:
          return;
      }
      sub_476630(a1 + 738886, *((__int16 *)a1 + 1390339), *(char **)&v351[3112]);
    }
    else if ( (unsigned __int16)v358 == 614 )
    {
      v101 = sub_452A40(WORD1(v358), WORD2(v358));
      if ( v101 )
        sub_452BC0(WORD1(v358), WORD2(v358), v101);
    }
    else if ( (unsigned __int16)v358 > 0x259u )
    {
      switch ( (__int16)v358 )
      {
        case 610:
          v88 = a1[230514];
          if ( v88 )
          {
            while ( 1 )
            {
              v89 = *(_DWORD **)(v88 + 4);
              if ( v89 )
              {
                if ( *v89 == v11 )
                  break;
              }
              v88 = *(_DWORD *)(v88 + 12);
              if ( !v88 )
                goto LABEL_218;
            }
          }
          else
          {
LABEL_218:
            v90 = (char *)MEMORY[0x4A01AA](0x4Au);
            *(_DWORD *)v90 = v357;
            j = v90;
            *((_DWORD *)v90 + 1) = WORD1(v358);
            *(_DWORD *)&v351[3112] = 40;
            *((_DWORD *)v90 + 2) = WORD2(v358);
            *((_WORD *)v90 + 6) = HIWORD(v358);
            *(_DWORD *)(v90 + 14) = 0;
            v91 = sub_47D780(a6 + 16, (int)v371, *(int *)&v351[3112]);
            *(_DWORD *)&v351[3112] = 47;
            *(_DWORD *)&v351[3108] = v371;
            v371[v91] = 0;
            v92 = MEMORY[0x49FFB0](*(const char **)&v351[3108], *(int *)&v351[3112]);
            v93 = v92;
            if ( v92 )
            {
              *v92 = 0;
              v94 = MEMORY[0x49FAF5](v371);
              v95 = j;
              v96 = v93 + 1;
              *(_DWORD *)(j + 14) = v94;
            }
            else
            {
              v95 = j;
              v96 = v371;
            }
            strcpy(v95 + 34, v96);
            v97 = j;
            *(_DWORD *)&v351[3112] = 7000;
            *(_DWORD *)&v351[3108] = 3000;
            *(_DWORD *)(j + 18) = dword_4AF2FC(v352, v353, v354, v355, v356);
            *(_DWORD *)(v97 + 26) = 0;
            *(_DWORD *)(v97 + 30) = 0;
            *(_DWORD *)(v97 + 22) = sub_401710(*(int *)&v351[3108], *(int *)&v351[3112]);
            (*(void (__thiscall **)(_DWORD *, char *))(a1[230513] + 4))(a1 + 230513, v97);
          }
          break;
        case 611:
          for ( k = a1[230514]; k; k = *(_DWORD *)(k + 12) )
          {
            v99 = *(_DWORD **)(k + 4);
            if ( v99 && *v99 == v11 )
            {
              (*(void (__thiscall **)(_DWORD *, int))(a1[230513] + 20))(a1 + 230513, k);
              goto LABEL_228;
            }
          }
          break;
        case 612:
          v100 = sub_452A40(WORD1(v358), WORD2(v358));
          if ( v100 )
            sub_452B00(WORD1(v358), WORD2(v358), v100);
          break;
        case 613:
          *(_DWORD *)&v351[3112] = 11861940;
          *(_DWORD *)&v351[3108] = 1472;
LABEL_52:
          v36 = (char *)sub_480550(*(int *)&v351[3108]);
          sub_41C120(v36, *(int *)&v351[3112]);
          break;
        default:
          return;
      }
    }
    else if ( (unsigned __int16)v358 == 601 )
    {
      *((_BYTE *)v369 + sub_47D780(a6 + 16, (int)v369, 15)) = 0;
      if ( strcmp((const char *)v369, (const char *)a1 + 2780680) || *(_DWORD **)((char *)a1 + 2780742) == v357 )
      {
        if ( !a1[695167] && strcmp((const char *)a1 + 2780680, (const char *)sub_480550(1050)) )
        {
          *(_DWORD *)&v351[3112] = 0;
          *(_DWORD *)&v351[3108] = 0;
          qmemcpy(v351, a1 + 695169, 0xC24u);
          sub_440CB0(
            (_WORD *)a1 + 1405030,
            *(__int16 *)v351,
            *(int *)&v351[4],
            *(int *)&v351[8],
            *(int *)&v351[12],
            *(int *)&v351[16],
            *(int *)&v351[20],
            *(int *)&v351[24],
            *(int *)&v351[28],
            *(int *)&v351[32],
            *(int *)&v351[36],
            *(int *)&v351[40],
            *(int *)&v351[44],
            *(int *)&v351[48],
            *(int *)&v351[52],
            *(int *)&v351[56],
            *(int *)&v351[60],
            *(int *)&v351[64],
            *(int *)&v351[68],
            *(int *)&v351[72],
            *(int *)&v351[76],
            *(int *)&v351[80],
            *(int *)&v351[84],
            *(int *)&v351[88],
            *(int *)&v351[92],
            *(int *)&v351[96],
            *(int *)&v351[100],
            *(int *)&v351[104],
            *(int *)&v351[108],
            *(int *)&v351[112],
            *(int *)&v351[116],
            *(int *)&v351[120],
            *(int *)&v351[124],
            *(int *)&v351[128],
            *(int *)&v351[132],
            *(int *)&v351[136],
            *(int *)&v351[140],
            *(int *)&v351[144],
            *(int *)&v351[148],
            *(int *)&v351[152],
            *(int *)&v351[156],
            *(int *)&v351[160],
            *(int *)&v351[164],
            *(int *)&v351[168],
            *(int *)&v351[172],
            *(int *)&v351[176],
            *(int *)&v351[180],
            *(int *)&v351[184],
            *(int *)&v351[188],
            *(int *)&v351[192],
            *(int *)&v351[196],
            *(int *)&v351[200],
            *(int *)&v351[204],
            *(int *)&v351[208],
            *(int *)&v351[212],
            *(int *)&v351[216],
            *(int *)&v351[220],
            *(int *)&v351[224],
            *(int *)&v351[228],
            *(int *)&v351[232],
            *(int *)&v351[236],
            *(int *)&v351[240],
            *(int *)&v351[244]);
          memset(a1 + 695163, 0, 0xC3Cu);
          return;
        }
        v87 = *((__int16 *)a1 + 1390339);
        *(_DWORD *)&v351[3112] = 1;
        *(_DWORD *)&v351[3108] = v87;
        qmemcpy(v351, a1 + 695169, 0xC24u);
        sub_41AC50(a1 + 784694);
      }
      memset(a1 + 695163, 0, 0xC3Cu);
    }
    else if ( (unsigned __int16)v358 > 0xD4u )
    {
      if ( (unsigned __int16)v358 == 600 )
      {
        if ( *((_BYTE *)a1 + 2780741) > 2u )
        {
          qmemcpy(&v351[8], a1 + 695169, 0xC24u);
          v86 = sub_41AD30(a1 + 784694);
          if ( v86 != -1 )
            sub_41AE40(v86);
        }
LABEL_315:
        memset(a1 + 695163, 0, 0xC3Cu);
      }
    }
    else
    {
      switch ( (unsigned __int16)v358 )
      {
        case 0xD4u:
          sub_44F670((unsigned __int8 *)a1 + 3134528, v11);
          break;
        case 0xD2u:
          sub_44F730((unsigned __int8 *)a1 + 3134528, a6 + 16);
          break;
        case 0xD3u:
          sub_44F4F0((unsigned __int8 *)a1 + 3134528, BYTE6(v358), a6 + 16);
          break;
      }
    }
    return;
  }
  if ( (unsigned __int16)v358 == 203 )
  {
    sub_47D780(a6 + 16, (int)v371, 100);
    sub_4420D0(v379);
    v386 = 11;
    qmemcpy(&v351[3012], v371, 0x64u);
    sub_4420F0(
      v379,
      v351[3012],
      *(int *)&v351[3016],
      *(int *)&v351[3020],
      *(int *)&v351[3024],
      *(int *)&v351[3028],
      *(int *)&v351[3032],
      *(int *)&v351[3036],
      *(int *)&v351[3040],
      *(int *)&v351[3044],
      *(int *)&v351[3048],
      *(int *)&v351[3052],
      *(int *)&v351[3056],
      *(int *)&v351[3060],
      *(int *)&v351[3064],
      *(int *)&v351[3068],
      *(int *)&v351[3072],
      *(int *)&v351[3076],
      *(int *)&v351[3080],
      *(int *)&v351[3084],
      *(int *)&v351[3088],
      *(int *)&v351[3092],
      *(int *)&v351[3096],
      *(int *)&v351[3100],
      *(int *)&v351[3104],
      *(int *)&v351[3108],
      0);
    if ( sub_440C00((__int16 *)a1 + 1405030, *(int *)&v379[66], &v379[4]) == 1 )
    {
      *(_DWORD *)&v351[3112] = 0;
      *(_DWORD *)&v351[3108] = 0;
      qmemcpy(v351, v379, 0xC24u);
      sub_440CB0(
        (_WORD *)a1 + 1405030,
        *(__int16 *)v351,
        *(int *)&v351[4],
        *(int *)&v351[8],
        *(int *)&v351[12],
        *(int *)&v351[16],
        *(int *)&v351[20],
        *(int *)&v351[24],
        *(int *)&v351[28],
        *(int *)&v351[32],
        *(int *)&v351[36],
        *(int *)&v351[40],
        *(int *)&v351[44],
        *(int *)&v351[48],
        *(int *)&v351[52],
        *(int *)&v351[56],
        *(int *)&v351[60],
        *(int *)&v351[64],
        *(int *)&v351[68],
        *(int *)&v351[72],
        *(int *)&v351[76],
        *(int *)&v351[80],
        *(int *)&v351[84],
        *(int *)&v351[88],
        *(int *)&v351[92],
        *(int *)&v351[96],
        *(int *)&v351[100],
        *(int *)&v351[104],
        *(int *)&v351[108],
        *(int *)&v351[112],
        *(int *)&v351[116],
        *(int *)&v351[120],
        *(int *)&v351[124],
        *(int *)&v351[128],
        *(int *)&v351[132],
        *(int *)&v351[136],
        *(int *)&v351[140],
        *(int *)&v351[144],
        *(int *)&v351[148],
        *(int *)&v351[152],
        *(int *)&v351[156],
        *(int *)&v351[160],
        *(int *)&v351[164],
        *(int *)&v351[168],
        *(int *)&v351[172],
        *(int *)&v351[176],
        *(int *)&v351[180],
        *(int *)&v351[184],
        *(int *)&v351[188],
        *(int *)&v351[192],
        *(int *)&v351[196],
        *(int *)&v351[200],
        *(int *)&v351[204],
        *(int *)&v351[208],
        *(int *)&v351[212],
        *(int *)&v351[216],
        *(int *)&v351[220],
        *(int *)&v351[224],
        *(int *)&v351[228],
        *(int *)&v351[232],
        *(int *)&v351[236],
        *(int *)&v351[240],
        *(int *)&v351[244]);
    }
    if ( sub_4763B0(*(_DWORD *)&v379[66], &v379[4]) == 1 )
    {
      *(_DWORD *)&v351[3112] = 0;
      *(_DWORD *)&v351[3108] = -1;
      qmemcpy(v351, v379, 0xC24u);
      sub_476440(
        (int)(a1 + 738886),
        *(int *)v351,
        *(int *)&v351[4],
        *(int *)&v351[8],
        *(int *)&v351[12],
        *(int *)&v351[16],
        *(int *)&v351[20],
        *(int *)&v351[24],
        *(int *)&v351[28],
        *(int *)&v351[32],
        *(int *)&v351[36],
        *(int *)&v351[40],
        *(int *)&v351[44],
        *(int *)&v351[48],
        *(int *)&v351[52],
        *(int *)&v351[56],
        *(int *)&v351[60],
        *(int *)&v351[64],
        *(int *)&v351[68],
        *(int *)&v351[72],
        *(int *)&v351[76],
        *(int *)&v351[80],
        *(int *)&v351[84],
        *(int *)&v351[88],
        *(int *)&v351[92],
        *(int *)&v351[96],
        *(int *)&v351[100],
        *(int *)&v351[104],
        *(int *)&v351[108],
        *(int *)&v351[112],
        *(int *)&v351[116],
        *(int *)&v351[120],
        *(int *)&v351[124],
        *(int *)&v351[128],
        *(int *)&v351[132],
        *(int *)&v351[136],
        *(int *)&v351[140],
        *(int *)&v351[144],
        *(int *)&v351[148],
        *(int *)&v351[152],
        *(int *)&v351[156],
        *(int *)&v351[160],
        *(int *)&v351[164],
        *(int *)&v351[168],
        *(int *)&v351[172],
        *(int *)&v351[176],
        *(int *)&v351[180],
        *(int *)&v351[184],
        *(int *)&v351[188],
        *(int *)&v351[192],
        *(int *)&v351[196],
        *(int *)&v351[200],
        *(int *)&v351[204],
        *(int *)&v351[208],
        *(int *)&v351[212],
        *(int *)&v351[216],
        *(int *)&v351[220],
        *(int *)&v351[224],
        *(int *)&v351[228],
        *(int *)&v351[232],
        *(int *)&v351[236],
        *(int *)&v351[240],
        *(int *)&v351[244]);
    }
    sub_43ED40(v379);
    v386 = -1;
    nullsub_5();
    return;
  }
  switch ( (__int16)v358 )
  {
    case 6:
    case 7:
    case 8:
    case 9:
    case 10:
    case 11:
    case 12:
    case 13:
    case 14:
    case 15:
    case 17:
    case 18:
    case 19:
    case 20:
    case 21:
    case 22:
    case 23:
    case 24:
    case 25:
    case 27:
    case 31:
    case 32:
    case 33:
    case 34:
    case 50:
    case 51:
    case 52:
      goto LABEL_876;
    case 29:
    case 30:
      goto LABEL_584;
    case 40:
    case 100:
    case 101:
    case 102:
    case 103:
    case 104:
      goto LABEL_674;
    case 41:
      *(_DWORD *)v375 = v11;
      *(_QWORD *)&v375[4] = v358;
      v375[12] = 0;
      dword_4AF0C0(&v375[12], a6 + 16);
      if ( v357 == *(_DWORD **)a1[895096] )
      {
        (*(void (__thiscall **)(_DWORD *, char *))(a1[794986] + 60))(a1 + 794986, v375);
      }
      else
      {
        v83 = (_BYTE *)sub_4280B0(a1, (int)v357);
        if ( v83 )
        {
          switch ( v83[69] )
          {
            case 0:
            case 1:
            case 3:
            case 4:
            case 5:
              (*(void (__thiscall **)(_BYTE *, char *))(*(_DWORD *)v83 + 60))(v83, v375);
              break;
            default:
              return;
          }
        }
      }
      return;
    case 42:
      *(_DWORD *)v375 = v11;
      *(_QWORD *)&v375[4] = v358;
      v375[12] = 0;
      dword_4AF0C0(&v375[12], a6 + 16);
      if ( v357 == *(_DWORD **)a1[895096] )
      {
        (*(void (__thiscall **)(_DWORD *, char *))(a1[794986] + 40))(a1 + 794986, v375);
      }
      else
      {
        v84 = sub_4280B0(a1, (int)v357);
        if ( v84 )
          (*(void (__thiscall **)(int, char *))(*(_DWORD *)v84 + 40))(v84, v375);
      }
      return;
    case 44:
      v15 = BYTE6(v358);
      *(_DWORD *)((char *)a1 + 3583535) = v11;
      if ( HIBYTE(v358) )
      {
        v16 = sub_401430(2);
        v17 = sub_401430(3);
        v18 = (const char *)sub_480550(1513);
        MEMORY[0x49FA18](Str, v18);
        sub_4366D0(a1 + 695155, v16, v17, Str);
      }
      if ( WORD2(v358) )
      {
        v19 = (const char *)sub_480550(1459);
        MEMORY[0x49FA18](Str, v19, WORD1(v358), WORD2(v358));
        if ( v15 )
          *(_WORD *)((char *)a1 + 2959663) = WORD2(v358);
        else
          *(_WORD *)((char *)a1 + 2959663) += WORD2(v358);
      }
      else
      {
        v20 = (const char *)sub_480550(1460);
        MEMORY[0x49FA18](Str, v20, WORD1(v358));
      }
      v21 = sub_401430(2);
      v22 = sub_401430(3);
      sub_4366D0(a1 + 695155, v21, v22, Str);
      return;
    case 45:
      if ( !HIWORD(v358) )
      {
        v30 = BYTE2(v358);
        *(_DWORD *)((char *)a1 + 3583535) = v11;
        *(_DWORD *)&v351[3112] = 0;
        v31 = a1[794986];
        *(_DWORD *)&v351[3108] = 4;
        *(_DWORD *)&v351[3104] = 7;
        *((_BYTE *)a1 + 3583516) = v30;
        (*(void (__stdcall **)(_DWORD, _DWORD, _DWORD, int, int, unsigned int, int, int, _DWORD *, _DWORD, _DWORD))(v31 + 16))(
          *(_DWORD *)&v351[3104],
          *(_DWORD *)&v351[3108],
          *(_DWORD *)&v351[3112],
          v352,
          v353,
          v354,
          v355,
          v356,
          v357,
          v358,
          HIDWORD(v358));
        v32 = (char *)MEMORY[0x4A01AA](0x240u);
        v363 = (int)v32;
        if ( v32 )
          v33 = sub_449480(v32);
        else
          v33 = 0;
        if ( !(*(int (__thiscall **)(char *, int, _DWORD, _DWORD, _DWORD, _DWORD, _DWORD, _DWORD, _DWORD))(*(_DWORD *)v33 + 4))(
                v33,
                422,
                a1[795019],
                a1[795020],
                a1[795019],
                a1[795020],
                0,
                *(_DWORD *)a1[895096],
                0) )
        {
          if ( v33 )
            (**(void (__thiscall ***)(char *, int))v33)(v33, 1);
          goto LABEL_51;
        }
        *(_DWORD *)&v351[3112] = 16;
        if ( dword_59A9F8 )
        {
          v35 = MEMORY[0x4A01AA](*(unsigned int *)&v351[3112]);
          if ( !v35 )
          {
LABEL_51:
            *(_DWORD *)&v351[3112] = 11861940;
            *(_DWORD *)&v351[3108] = 1469;
            goto LABEL_52;
          }
          *v35 = &off_4AF460;
          v35[1] = 0;
          v35[3] = 0;
          v35[2] = 0;
          v35[1] = v33;
          v35[3] = 0;
          v35[2] = dword_59A9FC;
          *(_DWORD *)(dword_59A9FC + 12) = v35;
        }
        else
        {
          v34 = MEMORY[0x4A01AA](*(unsigned int *)&v351[3112]);
          if ( v34 )
          {
            *v34 = &off_4AF460;
            v34[1] = 0;
            v34[3] = 0;
            v34[2] = 0;
          }
          else
          {
            v34 = 0;
          }
          dword_59A9F8 = (int)v34;
          if ( !v34 )
            goto LABEL_51;
          v34[1] = v33;
          v35 = (_DWORD *)dword_59A9F8;
        }
        dword_59A9FC = (int)v35;
        ++dword_59AA08;
        goto LABEL_51;
      }
      if ( HIWORD(v358) != 1 )
        return;
      v37 = sub_4280B0(a1, v11);
      v38 = v37;
      if ( !v37 )
        return;
      (*(void (__thiscall **)(int, int, int, _DWORD, int, int, unsigned int, int, int, _DWORD *, _DWORD, _DWORD))(*(_DWORD *)v37 + 16))(
        v37,
        7,
        4,
        0,
        v352,
        v353,
        v354,
        v355,
        v356,
        v357,
        v358,
        HIDWORD(v358));
      v39 = (char *)MEMORY[0x4A01AA](0x240u);
      v363 = (int)v39;
      if ( v39 )
        v40 = sub_449480(v39);
      else
        v40 = 0;
      if ( !(*(int (__thiscall **)(char *, int, _DWORD, _DWORD, _DWORD, _DWORD, _DWORD, _DWORD, _DWORD))(*(_DWORD *)v40 + 4))(
              v40,
              422,
              *(_DWORD *)(v38 + 132),
              *(_DWORD *)(v38 + 136),
              *(_DWORD *)(v38 + 132),
              *(_DWORD *)(v38 + 136),
              0,
              **(_DWORD **)(v38 + 400440),
              0) )
        goto LABEL_64;
      *(_DWORD *)&v351[3112] = 16;
      if ( !dword_59A9F8 )
      {
        v41 = MEMORY[0x4A01AA](*(unsigned int *)&v351[3112]);
        if ( !v41 )
          goto LABEL_89;
        *v41 = &off_4AF460;
        v41[1] = 0;
        v41[3] = 0;
        v41[2] = 0;
        goto LABEL_90;
      }
LABEL_62:
      v42 = MEMORY[0x4A01AA](*(unsigned int *)&v351[3112]);
      if ( !v42 )
        return;
      *v42 = &off_4AF460;
      v42[1] = 0;
      v42[3] = 0;
      v42[2] = 0;
      v42[1] = v40;
      goto LABEL_80;
    case 46:
      v76 = BYTE2(v358);
      v77 = MEMORY[0x49FDAE]();
      *(_DWORD *)((char *)&loc_43845C + (_DWORD)a1) = v77;
      v78 = (v77 + 37) / 2;
      a1[1106198] = v78;
      *(_DWORD *)((char *)&loc_438460 + (_DWORD)a1) = v78 + 46;
      a1[1106201] = v78 + 75;
      switch ( v76 )
      {
        case 0:
          a1[895926] = v77;
          break;
        case 1:
          a1[895926] = v78;
          break;
        case 2:
          a1[895926] = v78 + 46;
          break;
        case 3:
          a1[895926] = v78 + 75;
          break;
        default:
          break;
      }
      if ( *((unsigned __int8 *)a1 + 3583708) != WORD2(v358) )
      {
        v79 = sub_42E720(&dword_8FCC50, 1, 50, 0);
        dword_4AF23C(v79, *(_DWORD *)&v351[3104], *(_DWORD *)&v351[3108], *(_DWORD *)&v351[3112]);
      }
      v80 = HIWORD(v358);
      a1[9] = 0;
      a1[11] = 0;
      a1[12] = 0;
      if ( v80 == 1 )
      {
        v81 = a1[902359];
        a1[9] = 1;
        a1[11] = 1;
        (*(void (__thiscall **)(_DWORD *, int, int, unsigned int, int))(v81 + 12))(a1 + 902359, v352, v353, v354, v355);
        (*(void (__thiscall **)(_DWORD *))(a1[902359] + 4))(a1 + 902359);
        (*(void (__thiscall **)(_DWORD *, int, int, int))(a1[902359] + 28))(a1 + 902359, 500, 1145569280, 1142292480);
        *((_BYTE *)a1 + 3583708) = BYTE4(v358);
      }
      else if ( v80 == 2 )
      {
        v82 = BYTE4(v358);
        a1[11] = 1;
        *((_BYTE *)a1 + 3583708) = v82;
      }
      else
      {
        if ( v80 == 3 )
          a1[12] = 1;
        *((_BYTE *)a1 + 3583708) = BYTE4(v358);
      }
      return;
    case 47:
      if ( v13 != 102 )
      {
        if ( v13 == 112 )
        {
          if ( v11 != *(_DWORD *)a1[895096] )
          {
            v50 = sub_4280B0(a1, v11);
            v51 = v50;
            if ( !v50 )
              return;
            (*(void (__thiscall **)(int, int, int, int, int, int, unsigned int, int, int, _DWORD *, _DWORD, _DWORD))(*(_DWORD *)v50 + 16))(
              v50,
              7,
              4,
              400,
              v352,
              v353,
              v354,
              v355,
              v356,
              v357,
              v358,
              HIDWORD(v358));
            v52 = (char *)MEMORY[0x4A01AA](0x240u);
            v363 = (int)v52;
            if ( v52 )
              v40 = sub_449480(v52);
            else
              v40 = 0;
            if ( (*(int (__thiscall **)(char *, int, _DWORD, _DWORD, _DWORD, _DWORD, _DWORD, _DWORD, _DWORD))(*(_DWORD *)v40 + 4))(
                   v40,
                   112,
                   *(_DWORD *)(v51 + 132),
                   *(_DWORD *)(v51 + 136),
                   *(_DWORD *)(v51 + 132),
                   *(_DWORD *)(v51 + 136),
                   0,
                   **(_DWORD **)(v51 + 400440),
                   0) )
            {
              *(_DWORD *)&v351[3112] = 16;
              if ( dword_59A9F8 )
                goto LABEL_62;
              v41 = MEMORY[0x4A01AA](*(unsigned int *)&v351[3112]);
              if ( v41 )
              {
                *v41 = &off_4AF460;
                v41[1] = 0;
                v41[3] = 0;
                v41[2] = 0;
                goto LABEL_90;
              }
              goto LABEL_89;
            }
            goto LABEL_92;
          }
          (*(void (__thiscall **)(_DWORD *, int, int, int, int, int, unsigned int, int, int, _DWORD *, _DWORD, _DWORD))(a1[794986] + 16))(
            a1 + 794986,
            7,
            4,
            400,
            v352,
            v353,
            v354,
            v355,
            v356,
            v357,
            v358,
            HIDWORD(v358));
          v49 = (char *)MEMORY[0x4A01AA](0x240u);
          v363 = (int)v49;
          if ( v49 )
            v44 = sub_449480(v49);
          else
            v44 = 0;
          if ( (*(int (__thiscall **)(char *, int, _DWORD, _DWORD, _DWORD, _DWORD, _DWORD, _DWORD, _DWORD))(*(_DWORD *)v44 + 4))(
                 v44,
                 112,
                 a1[795019],
                 a1[795020],
                 a1[795019],
                 a1[795020],
                 0,
                 *(_DWORD *)a1[895096],
                 0) )
          {
            *(_DWORD *)&v351[3112] = 16;
            if ( !dword_59A9F8 )
            {
              v45 = MEMORY[0x4A01AA](*(unsigned int *)&v351[3112]);
              if ( v45 )
              {
                *v45 = &off_4AF460;
                v45[1] = 0;
                v45[3] = 0;
                v45[2] = 0;
                goto LABEL_76;
              }
              goto LABEL_75;
            }
            goto LABEL_78;
          }
LABEL_531:
          if ( !v44 )
            return;
LABEL_517:
          (**(void (__thiscall ***)(char *, int))v44)(v44, 1);
          return;
        }
        if ( v13 == 106 )
        {
          if ( v11 == *(_DWORD *)a1[895096] )
          {
            (*(void (__thiscall **)(_DWORD *, int, int, int, int, int, unsigned int, int, int, _DWORD *, _DWORD, _DWORD))(a1[794986] + 16))(
              a1 + 794986,
              7,
              4,
              400,
              v352,
              v353,
              v354,
              v355,
              v356,
              v357,
              v358,
              HIDWORD(v358));
            v53 = (char *)MEMORY[0x4A01AA](0x240u);
            v363 = (int)v53;
            if ( v53 )
              v44 = sub_449480(v53);
            else
              v44 = 0;
            if ( (*(int (__thiscall **)(char *, int, _DWORD, _DWORD, _DWORD, _DWORD, _DWORD, _DWORD, _DWORD))(*(_DWORD *)v44 + 4))(
                   v44,
                   106,
                   a1[795019],
                   a1[795020],
                   a1[795019],
                   a1[795020],
                   0,
                   *(_DWORD *)a1[895096],
                   0) )
            {
              *(_DWORD *)&v351[3112] = 16;
              if ( !dword_59A9F8 )
              {
                v45 = MEMORY[0x4A01AA](*(unsigned int *)&v351[3112]);
                if ( v45 )
                {
                  *v45 = &off_4AF460;
                  v45[1] = 0;
                  v45[3] = 0;
                  v45[2] = 0;
                  goto LABEL_76;
                }
                goto LABEL_75;
              }
LABEL_78:
              v42 = MEMORY[0x4A01AA](*(unsigned int *)&v351[3112]);
              if ( !v42 )
                return;
              *v42 = &off_4AF460;
              v42[1] = 0;
              v42[3] = 0;
              v42[2] = 0;
              v42[1] = v44;
LABEL_80:
              v42[3] = 0;
              v42[2] = dword_59A9FC;
              *(_DWORD *)(dword_59A9FC + 12) = v42;
              goto LABEL_708;
            }
            goto LABEL_531;
          }
          v54 = sub_4280B0(a1, v11);
          v55 = v54;
          if ( !v54 )
            return;
          (*(void (__thiscall **)(int, int, int, int, int, int, unsigned int, int, int, _DWORD *, _DWORD, _DWORD))(*(_DWORD *)v54 + 16))(
            v54,
            7,
            4,
            400,
            v352,
            v353,
            v354,
            v355,
            v356,
            v357,
            v358,
            HIDWORD(v358));
          v56 = (char *)MEMORY[0x4A01AA](0x240u);
          v363 = (int)v56;
          if ( v56 )
            v40 = sub_449480(v56);
          else
            v40 = 0;
          if ( (*(int (__thiscall **)(char *, int, _DWORD, _DWORD, _DWORD, _DWORD, _DWORD, _DWORD, _DWORD))(*(_DWORD *)v40 + 4))(
                 v40,
                 106,
                 *(_DWORD *)(v55 + 132),
                 *(_DWORD *)(v55 + 136),
                 *(_DWORD *)(v55 + 132),
                 *(_DWORD *)(v55 + 136),
                 0,
                 **(_DWORD **)(v55 + 400440),
                 0) )
          {
            *(_DWORD *)&v351[3112] = 16;
            if ( dword_59A9F8 )
              goto LABEL_62;
            v41 = MEMORY[0x4A01AA](*(unsigned int *)&v351[3112]);
            if ( v41 )
            {
              *v41 = &off_4AF460;
              v41[1] = 0;
              v41[3] = 0;
              v41[2] = 0;
              goto LABEL_90;
            }
            goto LABEL_89;
          }
          goto LABEL_92;
        }
        v145 = v13 == 123;
        v57 = *(_DWORD *)a1[895096];
        if ( v145 )
        {
          if ( v11 == v57 )
          {
            v58 = (char *)MEMORY[0x4A01AA](0x240u);
            v356 = (int)v58;
            v386 = 24;
            if ( v58 )
              v44 = sub_449480(v58);
            else
              v44 = 0;
            v59 = a1[795020];
            v60 = a1[795019];
            v61 = *(_DWORD *)v44;
            v62 = *(_DWORD *)a1[895096];
            *(_DWORD *)&v351[3112] = 0;
            *(_DWORD *)&v351[3108] = v62;
            *(_DWORD *)&v351[3104] = 0;
            *(_DWORD *)&v351[3100] = v59;
            *(_DWORD *)&v351[3096] = v60;
            *(_DWORD *)&v351[3092] = v59;
            *(_DWORD *)&v351[3088] = v60;
            v386 = -1;
            *(_DWORD *)&v351[3084] = 123;
            goto LABEL_145;
          }
          v63 = (_DWORD *)sub_4280B0(a1, v11);
          if ( !v63 )
            return;
          v64 = (char *)MEMORY[0x4A01AA](0x240u);
          v356 = (int)v64;
          v386 = 25;
          if ( v64 )
            v40 = sub_449480(v64);
          else
            v40 = 0;
          v65 = v63[34];
          v66 = v63[33];
          v67 = (_DWORD *)v63[100110];
          v68 = *(_DWORD *)v40;
          v386 = -1;
          if ( !(*(int (__thiscall **)(char *, int, int, int, int, int, _DWORD, _DWORD, _DWORD))(v68 + 4))(
                  v40,
                  123,
                  v66,
                  v65,
                  v66,
                  v65,
                  0,
                  *v67,
                  0) )
          {
LABEL_92:
            if ( !v40 )
              return;
LABEL_65:
            (**(void (__thiscall ***)(char *, int))v40)(v40, 1);
            return;
          }
        }
        else
        {
          if ( v11 == v57 )
          {
            (*(void (__thiscall **)(_DWORD *, int, int, _DWORD))(a1[794986] + 16))(a1 + 794986, 7, 4, 0);
            v69 = (char *)MEMORY[0x4A01AA](0x240u);
            v356 = (int)v69;
            v386 = 28;
            if ( v69 )
              v44 = sub_449480(v69);
            else
              v44 = 0;
            v70 = a1[795020];
            v71 = a1[795019];
            v61 = *(_DWORD *)v44;
            v72 = *(_DWORD *)a1[895096];
            *(_DWORD *)&v351[3112] = 0;
            *(_DWORD *)&v351[3108] = v72;
            *(_DWORD *)&v351[3104] = 0;
            *(_DWORD *)&v351[3100] = v70;
            *(_DWORD *)&v351[3096] = v71;
            *(_DWORD *)&v351[3092] = v70;
            *(_DWORD *)&v351[3088] = v71;
            v386 = -1;
            *(_DWORD *)&v351[3084] = 423;
LABEL_145:
            if ( (*(int (__thiscall **)(char *, _DWORD, _DWORD, _DWORD, _DWORD, _DWORD, _DWORD, _DWORD, _DWORD))(v61 + 4))(
                   v44,
                   *(_DWORD *)&v351[3084],
                   *(_DWORD *)&v351[3088],
                   *(_DWORD *)&v351[3092],
                   *(_DWORD *)&v351[3096],
                   *(_DWORD *)&v351[3100],
                   *(_DWORD *)&v351[3104],
                   *(_DWORD *)&v351[3108],
                   *(_DWORD *)&v351[3112]) )
            {
              sub_430AF0(v44);
              return;
            }
            goto LABEL_531;
          }
          v73 = sub_4280B0(a1, v11);
          v74 = v73;
          if ( !v73 )
            return;
          (*(void (__thiscall **)(int, int, int, _DWORD, int, int, unsigned int, int, int, _DWORD *, _DWORD, _DWORD))(*(_DWORD *)v73 + 16))(
            v73,
            7,
            4,
            0,
            v352,
            v353,
            v354,
            v355,
            v356,
            v357,
            v358,
            HIDWORD(v358));
          v75 = (char *)MEMORY[0x4A01AA](0x240u);
          v363 = (int)v75;
          if ( v75 )
            v40 = sub_449480(v75);
          else
            v40 = 0;
          if ( !(*(int (__thiscall **)(char *, int, _DWORD, _DWORD, _DWORD, _DWORD, _DWORD, _DWORD, _DWORD))(*(_DWORD *)v40 + 4))(
                  v40,
                  423,
                  *(_DWORD *)(v74 + 132),
                  *(_DWORD *)(v74 + 136),
                  *(_DWORD *)(v74 + 132),
                  *(_DWORD *)(v74 + 136),
                  0,
                  **(_DWORD **)(v74 + 400440),
                  0) )
          {
LABEL_64:
            if ( !v40 )
              return;
            goto LABEL_65;
          }
        }
        sub_430AF0(v40);
        return;
      }
      if ( v11 == *(_DWORD *)a1[895096] )
      {
        (*(void (__thiscall **)(_DWORD *, int, int, int, int, int, unsigned int, int, int, _DWORD *, _DWORD, _DWORD))(a1[794986] + 16))(
          a1 + 794986,
          7,
          4,
          400,
          v352,
          v353,
          v354,
          v355,
          v356,
          v357,
          v358,
          HIDWORD(v358));
        v43 = (char *)MEMORY[0x4A01AA](0x240u);
        v363 = (int)v43;
        if ( v43 )
          v44 = sub_449480(v43);
        else
          v44 = 0;
        if ( (*(int (__thiscall **)(char *, int, _DWORD, _DWORD, _DWORD, _DWORD, _DWORD, _DWORD, _DWORD))(*(_DWORD *)v44 + 4))(
               v44,
               102,
               a1[795019],
               a1[795020],
               a1[795019],
               a1[795020],
               0,
               *(_DWORD *)a1[895096],
               0) )
        {
          *(_DWORD *)&v351[3112] = 16;
          if ( !dword_59A9F8 )
          {
            v45 = MEMORY[0x4A01AA](*(unsigned int *)&v351[3112]);
            if ( v45 )
            {
              *v45 = &off_4AF460;
              v45[1] = 0;
              v45[3] = 0;
              v45[2] = 0;
LABEL_76:
              dword_59A9F8 = (int)v45;
              if ( !v45 )
                return;
              v45[1] = v44;
              v42 = (_DWORD *)dword_59A9F8;
              goto LABEL_708;
            }
LABEL_75:
            v45 = 0;
            goto LABEL_76;
          }
          goto LABEL_78;
        }
        goto LABEL_531;
      }
      v46 = sub_4280B0(a1, v11);
      v47 = v46;
      if ( !v46 )
        return;
      (*(void (__thiscall **)(int, int, int, int, int, int, unsigned int, int, int, _DWORD *, _DWORD, _DWORD))(*(_DWORD *)v46 + 16))(
        v46,
        7,
        4,
        400,
        v352,
        v353,
        v354,
        v355,
        v356,
        v357,
        v358,
        HIDWORD(v358));
      v48 = (char *)MEMORY[0x4A01AA](0x240u);
      v363 = (int)v48;
      if ( v48 )
        v40 = sub_449480(v48);
      else
        v40 = 0;
      if ( !(*(int (__thiscall **)(char *, int, _DWORD, _DWORD, _DWORD, _DWORD, _DWORD, _DWORD, _DWORD))(*(_DWORD *)v40 + 4))(
              v40,
              102,
              *(_DWORD *)(v47 + 132),
              *(_DWORD *)(v47 + 136),
              *(_DWORD *)(v47 + 132),
              *(_DWORD *)(v47 + 136),
              0,
              **(_DWORD **)(v47 + 400440),
              0) )
        goto LABEL_92;
      *(_DWORD *)&v351[3112] = 16;
      if ( dword_59A9F8 )
        goto LABEL_62;
      v41 = MEMORY[0x4A01AA](*(unsigned int *)&v351[3112]);
      if ( v41 )
      {
        *v41 = &off_4AF460;
        v41[1] = 0;
        v41[3] = 0;
        v41[2] = 0;
        goto LABEL_90;
      }
LABEL_89:
      v41 = 0;
LABEL_90:
      dword_59A9F8 = (int)v41;
      if ( v41 )
      {
        v41[1] = v40;
        v42 = (_DWORD *)dword_59A9F8;
LABEL_708:
        dword_59A9FC = (int)v42;
        ++dword_59AA08;
      }
      return;
    case 53:
      v381 = v11;
      v382 = v358;
      v383[0] = 0;
      dword_4AF0C0(v383, a6 + 16);
      if ( v357 == *(_DWORD **)a1[895096] )
      {
        (*(void (__thiscall **)(_DWORD *, unsigned int *))(a1[794986] + 64))(a1 + 794986, &v381);
      }
      else
      {
        v85 = sub_4280B0(a1, (int)v357);
        if ( v85 )
          (*(void (__thiscall **)(int, unsigned int *))(*(_DWORD *)v85 + 64))(v85, &v381);
      }
      return;
    case 54:
      *((_BYTE *)a1 + sub_47D780(a6 + 16, (int)(a1 + 24), 260) + 96) = 0;
      return;
    case 105:
      v379[sub_47D780(a6 + 16, (int)v379, 0x2000)] = 0;
      if ( strlen(v379) )
      {
        *(_DWORD *)&v351[3112] = 200;
        v145 = WORD1(v358) == 0;
        a1[1106206] = WORD1(v358) - 1;
        if ( v145 )
          *(_DWORD *)&v351[3108] = 390;
        else
          *(_DWORD *)&v351[3108] = 300;
        dword_4AF2A8((char *)&loc_438480 + (_DWORD)a1, 0, 0, *(_DWORD *)&v351[3108], *(_DWORD *)&v351[3112]);
        sub_42FA80(v379);
      }
      return;
    case 200:
      sub_47D780(a6 + 16, (int)v371, 100);
      sub_4420D0(v379);
      v386 = 10;
      qmemcpy(&v351[3012], v371, 0x64u);
      sub_4420F0(
        v379,
        v351[3012],
        *(int *)&v351[3016],
        *(int *)&v351[3020],
        *(int *)&v351[3024],
        *(int *)&v351[3028],
        *(int *)&v351[3032],
        *(int *)&v351[3036],
        *(int *)&v351[3040],
        *(int *)&v351[3044],
        *(int *)&v351[3048],
        *(int *)&v351[3052],
        *(int *)&v351[3056],
        *(int *)&v351[3060],
        *(int *)&v351[3064],
        *(int *)&v351[3068],
        *(int *)&v351[3072],
        *(int *)&v351[3076],
        *(int *)&v351[3080],
        *(int *)&v351[3084],
        *(int *)&v351[3088],
        *(int *)&v351[3092],
        *(int *)&v351[3096],
        *(int *)&v351[3100],
        *(int *)&v351[3104],
        *(int *)&v351[3108],
        0);
      *(_DWORD *)&v351[3112] = 0;
      *(_DWORD *)&v351[3108] = 0;
      qmemcpy(v351, v379, 0xC24u);
      sub_440CB0(
        (_WORD *)a1 + 1405030,
        *(__int16 *)v351,
        *(int *)&v351[4],
        *(int *)&v351[8],
        *(int *)&v351[12],
        *(int *)&v351[16],
        *(int *)&v351[20],
        *(int *)&v351[24],
        *(int *)&v351[28],
        *(int *)&v351[32],
        *(int *)&v351[36],
        *(int *)&v351[40],
        *(int *)&v351[44],
        *(int *)&v351[48],
        *(int *)&v351[52],
        *(int *)&v351[56],
        *(int *)&v351[60],
        *(int *)&v351[64],
        *(int *)&v351[68],
        *(int *)&v351[72],
        *(int *)&v351[76],
        *(int *)&v351[80],
        *(int *)&v351[84],
        *(int *)&v351[88],
        *(int *)&v351[92],
        *(int *)&v351[96],
        *(int *)&v351[100],
        *(int *)&v351[104],
        *(int *)&v351[108],
        *(int *)&v351[112],
        *(int *)&v351[116],
        *(int *)&v351[120],
        *(int *)&v351[124],
        *(int *)&v351[128],
        *(int *)&v351[132],
        *(int *)&v351[136],
        *(int *)&v351[140],
        *(int *)&v351[144],
        *(int *)&v351[148],
        *(int *)&v351[152],
        *(int *)&v351[156],
        *(int *)&v351[160],
        *(int *)&v351[164],
        *(int *)&v351[168],
        *(int *)&v351[172],
        *(int *)&v351[176],
        *(int *)&v351[180],
        *(int *)&v351[184],
        *(int *)&v351[188],
        *(int *)&v351[192],
        *(int *)&v351[196],
        *(int *)&v351[200],
        *(int *)&v351[204],
        *(int *)&v351[208],
        *(int *)&v351[212],
        *(int *)&v351[216],
        *(int *)&v351[220],
        *(int *)&v351[224],
        *(int *)&v351[228],
        *(int *)&v351[232],
        *(int *)&v351[236],
        *(int *)&v351[240],
        *(int *)&v351[244]);
      v356 = (int)(a1 + 695155);
      sub_43ED40(v379);
      for ( m = 0; m < 10; ++m )
      {
        *(_DWORD *)&v351[3112] = m;
        v26 = (const char *)sub_480550(1409);
        MEMORY[0x49FA18](v369, "%s%d", v26, *(_DWORD *)&v351[3112]);
        if ( !strcmp(&v379[4], (const char *)v369) )
        {
          v28 = (const char *)sub_480550(1463);
          MEMORY[0x49FA18](Str, v28);
          goto LABEL_33;
        }
      }
      if ( v379[65] <= 2u )
      {
        *(_DWORD *)&v351[3112] = &v379[4];
        v27 = (const char *)sub_480550(1465);
      }
      else
      {
        *(_DWORD *)&v351[3112] = *(unsigned __int16 *)&v379[75];
        *(_DWORD *)&v351[3108] = &v379[4];
        v27 = (const char *)sub_480550(1461);
      }
      MEMORY[0x49FA18](Str, v27);
LABEL_33:
      sub_41C120(Str, 11861940);
      v386 = -1;
      nullsub_5();
      return;
    case 201:
      v23 = a6 + 16;
      v361 = 0;
      sub_440C90(a1 + 702515);
      do
      {
        v24 = MEMORY[0x49FFB0](v23, 47);
        j = v24;
        if ( v24 )
        {
          sub_4420D0(v379);
          *(_DWORD *)&v351[3112] = 100;
          *(_DWORD *)&v351[3108] = v371;
          *(_DWORD *)&v351[3104] = v23;
          v386 = 8;
          *j = 0;
          sub_47D780(*(const char **)&v351[3104], *(int *)&v351[3108], *(int *)&v351[3112]);
          qmemcpy(&v351[3012], v371, 0x64u);
          sub_4420F0(
            v379,
            v351[3012],
            *(int *)&v351[3016],
            *(int *)&v351[3020],
            *(int *)&v351[3024],
            *(int *)&v351[3028],
            *(int *)&v351[3032],
            *(int *)&v351[3036],
            *(int *)&v351[3040],
            *(int *)&v351[3044],
            *(int *)&v351[3048],
            *(int *)&v351[3052],
            *(int *)&v351[3056],
            *(int *)&v351[3060],
            *(int *)&v351[3064],
            *(int *)&v351[3068],
            *(int *)&v351[3072],
            *(int *)&v351[3076],
            *(int *)&v351[3080],
            *(int *)&v351[3084],
            *(int *)&v351[3088],
            *(int *)&v351[3092],
            *(int *)&v351[3096],
            *(int *)&v351[3100],
            *(int *)&v351[3104],
            *(int *)&v351[3108],
            0);
          *(_DWORD *)&v351[3112] = 0;
          *(_DWORD *)&v351[3108] = 0;
          qmemcpy(v351, v379, 0xC24u);
          v14 = &v351[3108];
          sub_440CB0(
            (_WORD *)a1 + 1405030,
            *(__int16 *)v351,
            *(int *)&v351[4],
            *(int *)&v351[8],
            *(int *)&v351[12],
            *(int *)&v351[16],
            *(int *)&v351[20],
            *(int *)&v351[24],
            *(int *)&v351[28],
            *(int *)&v351[32],
            *(int *)&v351[36],
            *(int *)&v351[40],
            *(int *)&v351[44],
            *(int *)&v351[48],
            *(int *)&v351[52],
            *(int *)&v351[56],
            *(int *)&v351[60],
            *(int *)&v351[64],
            *(int *)&v351[68],
            *(int *)&v351[72],
            *(int *)&v351[76],
            *(int *)&v351[80],
            *(int *)&v351[84],
            *(int *)&v351[88],
            *(int *)&v351[92],
            *(int *)&v351[96],
            *(int *)&v351[100],
            *(int *)&v351[104],
            *(int *)&v351[108],
            *(int *)&v351[112],
            *(int *)&v351[116],
            *(int *)&v351[120],
            *(int *)&v351[124],
            *(int *)&v351[128],
            *(int *)&v351[132],
            *(int *)&v351[136],
            *(int *)&v351[140],
            *(int *)&v351[144],
            *(int *)&v351[148],
            *(int *)&v351[152],
            *(int *)&v351[156],
            *(int *)&v351[160],
            *(int *)&v351[164],
            *(int *)&v351[168],
            *(int *)&v351[172],
            *(int *)&v351[176],
            *(int *)&v351[180],
            *(int *)&v351[184],
            *(int *)&v351[188],
            *(int *)&v351[192],
            *(int *)&v351[196],
            *(int *)&v351[200],
            *(int *)&v351[204],
            *(int *)&v351[208],
            *(int *)&v351[212],
            *(int *)&v351[216],
            *(int *)&v351[220],
            *(int *)&v351[224],
            *(int *)&v351[228],
            *(int *)&v351[232],
            *(int *)&v351[236],
            *(int *)&v351[240],
            *(int *)&v351[244]);
          v23 = j + 1;
          ++v361;
          v386 = -1;
          nullsub_5();
          v24 = j;
        }
      }
      while ( v361 < HIWORD(v358) && v24 );
      sub_43E6F0((const char *)a1 + 2780620, (int)v14, (int)a1 + 3583089);
      return;
    case 202:
      sub_47D780(a6 + 16, (int)v371, 100);
      sub_440C00((__int16 *)a1 + 1405030, *(int *)&v371[62], v371);
      sub_4763B0(*(_DWORD *)&v371[62], v371);
      v29 = (const char *)sub_480550(1466);
      MEMORY[0x49FA18](Str, v29, v371);
      sub_41C120(Str, 11861940);
      return;
    default:
      return;
  }
}
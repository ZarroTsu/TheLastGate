/*************************************************************************

   This file is part of 'Mercenaries of Astonia v2'
   Copyright (c) 1997-2001 Daniel Brockhaus (joker@astonia.com)
   All rights reserved.

 **************************************************************************/

#define CT_BRAVER_M		   2
#define CT_BRAVER_F		   3

#define CT_TEMP_M		   4
#define CT_TEMP_F		   5
#define CT_MERC_M		   6
#define CT_MERC_F		   7
#define CT_HARA_M		   8
#define CT_HARA_F		   9

#define CT_SEYANDU_M	  10
#define CT_SEYANDU_F	  11

#define CT_ARCHTEMP_M	  12
#define CT_ARCHTEMP_F	  13
#define CT_PUGILIST_M	  14
#define CT_PUGILIST_F	  15

#define CT_WARRIOR_M	  16
#define CT_WARRIOR_F	  17
#define CT_SORCERER_M	  18
#define CT_SORCERER_F	  19

#define CT_SUMMONER_M	  20
#define CT_SUMMONER_F	  21
#define CT_ARCHHARA_M	  22
#define CT_ARCHHARA_F	  23

#define CT_COMPANION	  24
#define CT_ARCHCOMP  	  25
#define CT_CASTERCOMP	  26
#define CT_ARCHCASTER  	  27

#define CT_INNKEEPER   	  28
#define CT_BISHOP    	  29
#define CT_PRIEST    	  30
#define CT_GATEKEEPER	 848
#define CT_HERBCOLL		 703
#define CT_HERBCOLL2	1205
#define CT_SHIVA_I		 664

#define CT_RD_SKELE		  31
#define CT_RD_GHOST		  32
#define CT_RD_GROLM		  33
#define CT_RD_GARG		  34
#define CT_RD_ICEY		  35

#define CT_PEACEKEEPER	  36

#define CT_UNDEAD		  71
#define CT_VAMPIRE		  72
#define CT_RATLING		  83
#define CT_RATKING		  91
#define CT_GREENLING	  92
#define CT_GREENKING	 100
#define CT_DREADLING	 101
#define CT_DREADKING	 109
#define CT_SPIDER		 110
#define CT_LIZEMPEROR	1200

#define CT_STONEGOLEM	 723
#define CT_SEASONSMOB	 724
#define CT_ASCENTMOB	 728
#define CT_CANYONBNDT	 731
#define CT_MUDGOLEM		 732
#define CT_SWAMPLIZ		 733
#define CT_MOUNTVIK		 734
#define CT_GARGNEST		 735
#define CT_XECKO		1007
#define CT_ICENEST		1009
#define CT_CULTIST		1023
#define CT_THUGS		1060
#define CT_LIZARDKIN	1101
#define CT_BASALT		1255
#define CT_TOWER		1260

#define CT_GOLEMTRAP	  39
#define CT_GARGTRAP		  40

#define CT_GARGSTAT		 666
#define CT_GROLMSTA		 665

#define CT_LAB_1_BOSS	 518
#define CT_LAB_2_BOSS	 540
#define CT_LAB_3_BOSS	 543
#define CT_LAB_4_BOSS	 555
#define CT_LAB_5_BOSS	 592
#define CT_LAB_6_BOSS	 641
//
#define CT_LAB_7_BOSS	 850	// Skeleton
#define CT_LAB_8_BOSS	 723	// Golem
#define CT_LAB_9_BOSS	 900	// Librarian
#define CT_LAB10_BOSS	 923	// Golemancer
#define CT_LAB11_BOSS	 946	// Time Mage
#define CT_LAB12_BOSS	 728	// Barbarian

#define CT_LAB13_KEEP	 849

#define CT_ARENA_FIRST	  42
#define CT_ARENA_LAST	  70

#define CT_PENTMOB		  42

#define CT_BSMAGE1		1004
#define CT_BSMAGE2		1005
#define CT_BSMAGE3		1006
#define CT_TACTICIAN	 372

#define CT_JESSICA		1314	// Casino Rewards
#define CT_KAIBA		1312	// Casino Tokens
#define CT_YUGI			1313	// Casino Games

#define CT_OSIRIS		 348
#define CT_CONTRACTOR	 349
#define CT_ADHERENT		 351

#define BS_HOUR 		3600
#define BS_MINS 		(BS_HOUR/60)
#define BS_RC			  20
#define BS_COUNTER		  18

// -----

#define COMPANION_TIMEOUT (5 * 60 * TICKS)

// -----

#define IT_SEYANSWORD 	 682
#define IT_DEADSEYSWORD	 683

#define IT_ARENANOTE	 687

#define IT_TOMBSTONE 	 170
#define IT_LAGSCROLL 	 500

#define IT_RPOT			 267
#define IT_GPOT			 833
#define IT_SPOT			1479

#define IT_HERBA		1798
#define IT_HERBB		1799
#define IT_HERBC		1800
#define IT_HERBD		1801
#define IT_HERBE		1965

#define IT_GREENPILL	 648
#define IT_BLUEPILL		 649

#define IT_BSCOIN_STAR	 761
#define IT_BSCOIN_MOON	 762
#define IT_BSCOIN_NULL	 763

// Unique Amulets
#define IT_ANKHAMULET	 768
#define IT_UNS_AMBER	 702
#define IT_UNS_TURQU	 703
#define IT_UNS_GARNE	 115
#define IT_AMBERANKH	 704
#define IT_TURQUANKH	 705
#define IT_GARNEANKH	 706
#define IT_TRUEANKH		2388
#define IT_BREATHAMMY	1275
#define IT_AM_BLOODS	21050
#define IT_AM_VERDANT	21060
#define IT_AM_SEABREZ	21070
#define IT_AM_OBSIDI	2108
#define IT_AM_TRUESUN	2295
#define IT_AM_FALMOON	2296
#define IT_AM_ECLIPSE	2297

// Unique Belts
#define IT_BL_SOLAR		2395
#define IT_BL_LUNAR		2396
#define IT_BL_ECLIPSE	2397

// Tarot Cards - used with WN_CHARM
#define IT_CH_FOOL		1513
#define IT_CH_MAGI		1514
#define IT_CH_PREIST	1515
#define IT_CH_EMPRESS   1516
#define IT_CH_EMPEROR	1517
#define IT_CH_HEIROPH	1518
#define IT_CH_LOVERS	1519
#define IT_CH_CHARIOT	1520
#define IT_CH_STRENGTH	1521
#define IT_CH_HERMIT	1522
#define IT_CH_WHEEL		1523
#define IT_CH_JUSTICE	1524
#define IT_CH_HANGED	1525
#define IT_CH_DEATH		1526
#define IT_CH_TEMPER	1527
#define IT_CH_DEVIL		1528
#define IT_CH_TOWER		1529
#define IT_CH_STAR		1530
#define IT_CH_MOON		1531
#define IT_CH_SUN		1532
#define IT_CH_JUDGE		1533
#define IT_CH_WORLD		1534

#define IT_CH_FOOL_R	2231
#define IT_CH_MAGI_R	2232
#define IT_CH_PREIST_R	2233
#define IT_CH_EMPRES_R	2234
#define IT_CH_EMPERO_R	2235
#define IT_CH_HEIROP_R	2236
#define IT_CH_LOVERS_R	2237
#define IT_CH_CHARIO_R	2238
#define IT_CH_STRENG_R	2239
#define IT_CH_HERMIT_R	2240
#define IT_CH_WHEEL_R	2241
#define IT_CH_JUSTIC_R	2242
#define IT_CH_HANGED_R	2243
#define IT_CH_DEATH_R	2244
#define IT_CH_TEMPER_R	2245
#define IT_CH_DEVIL_R	2246
#define IT_CH_TOWER_R	2247
#define IT_CH_STAR_R	2248
#define IT_CH_MOON_R	2249
#define IT_CH_SUN_R		2250
#define IT_CH_JUDGE_R	2251
#define IT_CH_WORLD_R	2252

// TOWER items
#define IT_TW_CROWN		 981
#define IT_TW_CLOAK		 982
#define IT_TW_DREAD		 983
#define IT_TW_DOUSER	2394
#define IT_TW_MARCH		 984
#define IT_TW_BBELT		 985
#define IT_TW_OUTSIDE	1197
#define IT_TW_PROPHET	1201
#define IT_TW_HEAVENS	1202
//
#define IT_TW_ACEDIA	2401
#define IT_TW_IRA		2402
#define IT_TW_INVIDIA	2403
#define IT_TW_GULA		2404
#define IT_TW_LUXURIA	2405
#define IT_TW_AVARITIA	2406
#define IT_TW_SUPERBIA	2407
#define IT_TW_SINBIND	2400
#define IT_TW_SINSPR	879

// Special Glove items
#define IT_GL_SERPENT	 707
#define IT_GL_CHILLED	 708
#define IT_GL_CURSED	1116
#define IT_GL_TITANS	1117
#define IT_GL_BURNING	1661
#define IT_GL_SHADOW	 709
#define IT_GL_BLVIPER	2043

// Special Boot items
#define IT_BT_NATURES	 711

// Unique weapons
#define IT_WP_LIFESPRIG	  	2512
#define IT_WP_BLOODLET	  	2514
#define IT_WP_GEMCUTTER	  	2515
#define IT_WP_STARLIGHT	  	2519
#define IT_WP_KELPTRID	  	2520
#define IT_WP_PHALANX	  	2526 
#define IT_WP_LAMEDARG	  	2529
#define IT_WP_WHITEODA	  	2530
#define IT_WP_EXCALIBUR	  	2532
#define IT_WP_FELLNIGHT	  	2534
#define IT_WP_BLACKTAC	  	2535
#define IT_WP_CRIMRIP	  	2539
#define IT_WP_CRESSUN	  	2543
#define IT_WP_GILDSHINE	  	2545
#define IT_WP_BRONCHIT	  	2551
#define IT_WP_VOLCANF	  	2552

#define REQ_LAME		   10000

// Other

#define IT_SILVAMMY			2104
#define IT_SILVBELT			2109
#define IT_MISERRING		2482
#define IT_FORTERING		2483
#define IT_GAMBLERFAL		2484
#define IT_WHITEBELT		2486
#define IT_BONEARMOR		2481

// -----

#define IT_FLASK		 100

// Standard flowers; Red, Green, Purple, Yellow, Blue, White
#define IT_FLO_R		  18
#define IT_FLO_G		 142
#define IT_FLO_P		  46
#define IT_FLO_Y		 140
#define IT_FLO_B		 141
#define IT_FLO_W		1957

// Jungle flowers outside lab 2; Magenta, Cyan, Verdant
#define IT_FLO_M		1961
#define IT_FLO_C		1962
#define IT_FLO_V		1963

// Special Flowers; Tulip, Orchid
#define IT_FLO_T		1540
#define IT_FLO_O		1542

// Jungle flowers in lab 2 (1 is Magenta, 2 is Cyan, 3 is Verdant)
#define IT_FLO_1		 197
#define IT_FLO_2		 198
#define IT_FLO_3		 199

// Flower mixtures
#define IT_MIX_W_		 145
#define IT_MIX_Y_		 143
#define IT_MIX_B_		 144
#define IT_MIX_M_		1966
#define IT_MIX_C_		1967
#define IT_MIX_V_		1968

#define IT_MIX_WY		 146
#define IT_MIX_BW		 147
#define IT_MIX_MW		 148
#define IT_MIX_VW		 149
#define IT_MIX_CW		 158
#define IT_MIX_BY		 161
#define IT_MIX_MY		1972
#define IT_MIX_VY		1973
#define IT_MIX_CY		1974
#define IT_MIX_BM		1975
#define IT_MIX_BV		1976
#define IT_MIX_BC		1977
#define IT_MIX_CM		1969
#define IT_MIX_CV		1970
#define IT_MIX_MV		1971

// Lab 2 mixtures
#define IT_MIX_1_		 218
#define IT_MIX_2_		 219
#define IT_MIX_3_		 220
#define IT_MIX_12		 221
#define IT_MIX_23		 222
#define IT_MIX_13		 223

// Deadly Poison
#define IT_MIX_3____	2580
#define IT_MIX_32___	2581
#define IT_MIX_321__	2582
#define IT_MIX_3213_	2583
#define IT_POT_DEATH	2584

// Potions
#define IT_POT_M_HP		2000
#define IT_POT_N_HP		 101
#define IT_POT_G_HP		 127
#define IT_POT_H_HP		2001
#define IT_POT_S_HP		2002
#define IT_POT_C_HP		2003
#define IT_POT_L_HP		2004
#define IT_POT_D_HP		2005

#define IT_POT_M_EN		2006
#define IT_POT_N_EN		 125
#define IT_POT_G_EN		 273
#define IT_POT_H_EN		2007
#define IT_POT_S_EN		2008
#define IT_POT_C_EN		2009
#define IT_POT_L_EN		2010
#define IT_POT_D_EN		2011

#define IT_POT_M_MP		2012
#define IT_POT_N_MP		 102
#define IT_POT_G_MP		 131
#define IT_POT_H_MP		2013
#define IT_POT_S_MP		2014
#define IT_POT_C_MP		2015
#define IT_POT_L_MP		2016
#define IT_POT_D_MP		2017

#define IT_POT_VITA		 293
#define IT_POT_CLAR		 294
#define IT_POT_SAGE		 295

#define IT_POT_RAIN		 274
#define IT_POT_LIFE		 181
#define IT_POT_DEAD		  99

#define IT_POT_GOLEM	 449

#define IT_POT_T		1543
#define IT_POT_O		1544
#define IT_POT_PT		1545
#define IT_POT_PO		1546

#define IT_POT_LAB2		 224
#define IT_POT_BRV		1978
#define IT_POT_WIL		1979
#define IT_POT_INT		1980
#define IT_POT_AGL		1981
#define IT_POT_STR		 135

#define IT_POT_EXHP		1982
#define IT_POT_EXEN		1983
#define IT_POT_EXMP		1984

#define IT_POT_PRE		1985
#define IT_POT_EVA		1986
#define IT_POT_MOB		1987
#define IT_POT_FRE		1988
#define IT_POT_MAR		1989
#define IT_POT_IMM		1990
#define IT_POT_CLA		1991
#define IT_POT_THO		1992
#define IT_POT_BRU		1993
#define IT_POT_RES		1994
#define IT_POT_APT		1995
#define IT_POT_OFF		1996
#define IT_POT_DEF		1997
#define IT_POT_PER		1998
#define IT_POT_STE		1999

// -----

#define IT_PICKAXE		 458

#define IT_SILV			1342
#define IT_GOLD			1343
#define IT_PLAT			1344

#define IT_SILV_RING	1345
#define IT_GOLD_RING	1346
#define IT_PLAT_RING	1347

// Mine gemstones
#define IT_S_SA			1348
#define IT_S_RU			1349
#define IT_S_AM			1350
#define IT_S_TO			1351
#define IT_S_EM			1352
#define IT_S_DI			1353

#define IT_M_SA			1354
#define IT_M_RU			1355
#define IT_M_AM			1356
#define IT_M_TO			1357
#define IT_M_EM			1358
#define IT_M_DI			1359

#define IT_B_SA			1360
#define IT_B_RU			1361
#define IT_B_AM			1362
#define IT_B_TO			1363
#define IT_B_EM			1364
#define IT_B_DI			1365

#define IT_H_SA			1366
#define IT_H_RU			1367
#define IT_H_AM			1368
#define IT_H_TO			1369
#define IT_H_EM			1370
#define IT_H_DI			1371

#define IT_F_SA			1372
#define IT_F_RU			1373
#define IT_F_AM			1374
#define IT_F_TO			1375
#define IT_F_EM			1376
#define IT_F_DI			1377

// New Gemstones
#define IT_B_SP			2057
#define IT_B_CI			2058
#define IT_B_OP			2059
#define IT_B_AQ			2060

#define IT_H_SP			2061
#define IT_H_CI			2062
#define IT_H_OP			2063
#define IT_H_AQ			2064
#define IT_H_BE			2065
#define IT_H_ZI			2066

#define IT_F_SP			2067
#define IT_F_CI			2068
#define IT_F_OP			2069
#define IT_F_AQ			2070
#define IT_F_BE			2071
#define IT_F_ZI			2072

// Mine enemies
#define IT_E_WK			1465
#define IT_E_SK			 359
#define IT_E_GO			 371
#define IT_E_GA			 372
#define IT_E_MA			1466

// -----

#define IT_GAXE_STEL	 292
#define IT_THSW_STEL	 291
#define IT_AXXE_STEL	 290
#define IT_SHIE_STEL	 287
#define IT_SWOR_STEL	 288
#define IT_DUAL_STEL	 289
#define IT_DAGG_STEL	 284
#define IT_STAF_STEL	 285
#define IT_SPEA_STEL	 286
#define IT_CLAW_STEL	1779
#define IT_GAXE_GOLD	 531
#define IT_THSW_GOLD	 530
#define IT_AXXE_GOLD	 529
#define IT_SHIE_GOLD	 526
#define IT_SWOR_GOLD	 527
#define IT_DUAL_GOLD	 528
#define IT_DAGG_GOLD	 523
#define IT_STAF_GOLD	 524
#define IT_SPEA_GOLD	 525
#define IT_CLAW_GOLD	1780
#define IT_GAXE_EMER	 540
#define IT_THSW_EMER	 539
#define IT_AXXE_EMER	 538
#define IT_SHIE_EMER	 535
#define IT_SWOR_EMER	 536
#define IT_DUAL_EMER	 537
#define IT_DAGG_EMER	 532
#define IT_STAF_EMER	 533
#define IT_SPEA_EMER	 534
#define IT_CLAW_EMER	1781
#define IT_GAXE_CRYS	 549
#define IT_THSW_CRYS	 548
#define IT_AXXE_CRYS	 547
#define IT_SHIE_CRYS	 544
#define IT_SWOR_CRYS	 545
#define IT_DUAL_CRYS	 546
#define IT_DAGG_CRYS	 541
#define IT_STAF_CRYS	 542
#define IT_SPEA_CRYS	 543
#define IT_CLAW_CRYS	1782
#define IT_GAXE_TITN	 580
#define IT_THSW_TITN	 579
#define IT_AXXE_TITN	 578
#define IT_SHIE_TITN	 575
#define IT_SWOR_TITN	 576
#define IT_DUAL_TITN	 577
#define IT_DAGG_TITN	 572
#define IT_STAF_TITN	 573
#define IT_SPEA_TITN	 574
#define IT_CLAW_TITN	1783
#define IT_GAXE_ADAM	 701
#define IT_THSW_ADAM	 700
#define IT_AXXE_ADAM	 699
#define IT_SHIE_ADAM	 696
#define IT_SWOR_ADAM	 697
#define IT_DUAL_ADAM	 698
#define IT_DAGG_ADAM	 693
#define IT_STAF_ADAM	 694
#define IT_SPEA_ADAM	 695
#define IT_CLAW_ADAM	1784
#define IT_GAXE_DAMA	2052
#define IT_THSW_DAMA	2051
#define IT_AXXE_DAMA	2050
#define IT_SHIE_DAMA	2047
#define IT_SWOR_DAMA	2048
#define IT_DUAL_DAMA	2049
#define IT_DAGG_DAMA	2044
#define IT_STAF_DAMA	2045
#define IT_SPEA_DAMA	2046
#define IT_CLAW_DAMA	2053
//
#define IT_HELM_BRNZ	  51
#define IT_BODY_BRNZ	  53
#define IT_HELM_STEL	  56
#define IT_BODY_STEL	  58
#define IT_HELM_GOLD	  61
#define IT_BODY_GOLD	  63
#define IT_HELM_EMER	  66
#define IT_BODY_EMER	  68
#define IT_HELM_CRYS	  71
#define IT_BODY_CRYS	  73
#define IT_HELM_TITN	  76
#define IT_BODY_TITN	  78
#define IT_HELM_ADAM	  94
#define IT_BODY_ADAM	  96
#define IT_HELM_CAST	 342
#define IT_BODY_CAST	 344
#define IT_HELM_ADEP	 347
#define IT_BODY_ADEP	 349
#define IT_HELM_WIZR	 352
#define IT_BODY_WIZR	 354
#define IT_HELM_DAMA	2028
#define IT_BODY_DAMA	2030

// ----- Books

#define IT_BOOK_ALCH	1648
#define IT_BOOK_HOLY	1649
#define IT_BOOK_ADVA	1651
#define IT_BOOK_TRAV	1652
#define IT_BOOK_SWOR	1654
#define IT_BOOK_DAMO	1655
#define IT_BOOK_SHIV	1656
#define IT_BOOK_PROD	1657
#define IT_BOOK_VENO	1658
#define IT_BOOK_NECR	1659
#define IT_BOOK_BISH	2055
#define IT_BOOK_GREA	2056

// ----- Casino

#define IT_CASI_POTI	2476
#define IT_CASI_SCRO	2477
#define IT_CASI_TARO	2478
#define IT_CASI_RING	2479


// -------- pop_create_bonus -------- //

#define IT_POP_SPELL	1314 // +0 - +6
#define IT_POP_ASPEL	1321 // +0 - +6
#define IT_POP_ISPEL	1328 // +0 - +6
#define IT_POP_SSPEL	1335 // +0 - +6

#define IT_RD_HEAL		101
#define IT_RD_MANA		102
#define IT_RD_END		125
#define IT_RD_GHEAL		127
#define IT_RD_GMANA		131
#define IT_RD_GEND		273

#define IT_RD_AMMYONE	192
#define IT_RD_BELTONE	193
#define IT_RD_AMMYTWO	1793
#define IT_RD_BELTTWO	1794
#define IT_RD_AMMYTHR	1795
#define IT_RD_BELTTHR	2114
#define IT_RD_AMMYFOU	2123
#define IT_RD_BELTFOU	1797

#define IT_RD_BRV		182
#define IT_RD_WIL		183
#define IT_RD_INT		184
#define IT_RD_AGL		185
#define IT_RD_STR		186
#define IT_RD_HP		187
#define IT_RD_EN		188
#define IT_RD_MP		189

#define IT_OS_BRV		2496
#define IT_OS_WIL		2497
#define IT_OS_INT		2498
#define IT_OS_AGL		2499
#define IT_OS_STR		2500
#define IT_OS_SK		2501

#define IT_RD_SILV1		1390 // +RANDOM(6)
#define IT_RD_SILV2		2073 // +RANDOM(4)
#define IT_RD_GOLD1		1402 // +RANDOM(6)
#define IT_RD_GOLD2		2077 // +RANDOM(4)
#define IT_RD_PLAT1		1414 // +RANDOM(6)
#define IT_RD_PLAT2		2087 // +RANDOM(4)

#define IT_EXPS			2124
#define IT_LUKS			2125

#define IT_SOULSTONE	1146
#define IT_SOULCATAL	2502
#define IT_SOULFOCUS	2503

// -------- Buff/Debuff Sprites -------- //

#define BUF_SPR_GENERIC		93
#define BUF_SPR_EXHAUST		97
#define BUF_SPR_LIGHT		85
#define BUF_SPR_PROTECT		86
#define BUF_SPR_ENHANCE		87
#define BUF_SPR_BLESS		88
#define BUF_SPR_MSHIELD		95
#define BUF_SPR_HASTE		94
#define BUF_SPR_HEALSICK	119
#define BUF_SPR_CURSE		89
#define BUF_SPR_SLOW		149
#define BUF_SPR_POISON		178
#define BUF_SPR_VENOM		1011
#define BUF_SPR_RECALL		90
#define BUF_SPR_SHADOW		179
#define BUF_SPR_REND		224
#define BUF_SPR_WARCRY		325
#define BUF_SPR_WARCRY2		91
#define BUF_SPR_WARCRY3		822
//
#define BUF_SPR_RAGE		700
#define BUF_SPR_ARIA		701
#define BUF_SPR_ARIA2		702
#define BUF_SPR_LETHARGY	703
#define BUF_SPR_AGGRAVATE	1009
//
#define BUF_SPR_BLEED		411
#define BUF_SPR_REND2		329
#define BUF_SPR_SCORCH		365
#define BUF_SPR_CURSE2		225
#define BUF_SPR_SLOW2		319
#define BUF_SPR_MSHELL		219
#define BUF_SPR_REGEN		98
//
#define BUF_SPR_HEATSTR		419
#define BUF_SPR_FIRE		487
//
#define BUF_SPR_IMMUNI		497
#define BUF_SPR_INNOCU		498
#define BUF_SPR_BLIND		338
#define BUF_SPR_PULSE		485
#define BUF_SPR_TAUNT		489
#define BUF_SPR_GUARD		518
#define BUF_SPR_ZEPHYR		519
#define BUF_SPR_ZEPHYR2		617
#define BUF_SPR_DOUSE		618
//
#define BUF_SPR_POTION		619
#define BUF_SPR_POME		790
#define BUF_SPR_STARL		895
#define BUF_SPR_FROSTB		471
#define BUF_SPR_PHALANX	  16864
//
#define BUF_SPR_DEBIL	  16860
#define BUF_SPR_FRAGI	  16861
#define BUF_SPR_STIGM	  16862
#define BUF_SPR_HYPRT	  16863
//
#define BUF_SPR_ACCEL	  16764
#define BUF_SPR_SHARP	  16761
#define BUF_SPR_FORTI	  16762
#define BUF_SPR_BLSSN	  16760
#define BUF_SPR_WISDM	  16763

// -----

#define BUF_IT_PIGS			1131
#define BUF_IT_AALE			1141
#define BUF_IT_DRAG			1142
#define BUF_IT_MANA			1143

// -------- BLACK STRONGHOLD CANDLES -------- //

#define IT_BS_CAN1		 740
#define IT_BS_CAN2		1510
#define IT_BS_CAN3		1512


					//   X     Y
#define BS_CAN_1_1		554 + 283 * MAPX
#define BS_CAN_1_2		552 + 284 * MAPX
#define BS_CAN_1_3		556 + 284 * MAPX
#define BS_CAN_1_4		550 + 285 * MAPX
#define BS_CAN_1_5		558 + 285 * MAPX

#define BS_CAN_2_1		576 + 283 * MAPX
#define BS_CAN_2_2		574 + 284 * MAPX
#define BS_CAN_2_3		578 + 284 * MAPX
#define BS_CAN_2_4		572 + 285 * MAPX
#define BS_CAN_2_5		580 + 285 * MAPX

#define BS_CAN_3_1		598 + 283 * MAPX
#define BS_CAN_3_2		596 + 284 * MAPX
#define BS_CAN_3_3		600 + 284 * MAPX
#define BS_CAN_3_4		594 + 285 * MAPX
#define BS_CAN_3_5		602 + 285 * MAPX

//

#define EP_GOLEM1X		926
#define EP_GOLEM1Y		399
#define EP_GOLEM2X		930
#define EP_GOLEM2Y		399
#define EP_GOLEM3X		936
#define EP_GOLEM3Y		410
#define EP_GOLEM4X		936
#define EP_GOLEM4Y		414

//

// Map tile sizes
#define MAP_TILE_SIZE	   7
#define MAP_CELL_SIZE	   7

// X/Y for the target where a map is built
#define MM_TARGETX		  22
#define MM_TARGETY		1027


// Offset for additional maps - padded by 15 to avoid hearing others in adjacent maps
#define MM_TARG_OF		(MAP_TILE_SIZE*MAP_CELL_SIZE+15)

// Map special object template numbers
#define MM_CHEST		2489	// dr 107
#define MM_HOLE_S		2490
#define MM_HOLE_W		2491
#define MM_LADD_N		2492
#define MM_LADD_E		2493
#define MM_SHRINE		2494	// dr 108
#define MM_EXIT 		2495	// dr 109

// 16812 Red
// 16813 Blue
// 16814 Green

#define NUM_MAP_ENEM	  35
#define NUM_LEG_ENEM	   7

// Floor sprites
#define MFL_MINE1		 116
#define MFL_MINE2		 117
#define MFL_MINE3		 118
#define MFL_SAND		 130
#define MFL_DIRT		 131
#define MFL_WOOD		 133
#define MFL_SANDSTONE	 142
#define MFL_GRANITE		 170
#define MFL_DARKSTONE	 500
#define MFL_ROAD		 542
#define MFL_LGRASS		 551
#define MFL_BRICK		 558
#define MFL_LAVA		 704
#define MFL_GOLEM		 808
#define MFL_BSROUGH		 950
#define MFL_BSSMOOTH	 959
#define MFL_BEACH		1002
#define MFL_JUNGLE		1003
#define MFL_BASIC		1008
#define MFL_EARTH		1014
#define MFL_DGREY		1052
#define MFL_LGREY		1100
#define MFL_DGRASS		1118
#define MFL_MARBLE		1158
#define MFL_EMERALD		2658
#define MFL_SNOW		3020
#define MFL_LAUTUMN		2828
#define MFL_DAUTUMN		2832
#define MFL_CANYON		3538
#define MFL_LIZARD		5047
#define MFL_TOWER		5585
#define MFL_ICE		   16670
#define MFL_SEWER	   16728

// Wall Templates
#define MWL_GREY		   2
#define MWL_MARBLE		   3
#define MWL_BMARBLE		 122
#define MWL_EARTH		 150
#define MWL_SEWER		 249
#define MWL_RED			 297
#define MWL_MINE1		 437
#define MWL_MINE2		 438
#define MWL_MINE3		 439
#define MWL_BLACK		 721
#define MWL_ICE			 807
#define MWL_GREEN		 821
#define MWL_TTREE		1467
#define MWL_FTREE		1468
#define MWL_SAND	   91551
#define MWL_GBRICK	    1664
#define MWL_TBRICK	   91666
#define MWL_RBRICK	   91670
#define MWL_CANYON		1674
#define MWL_SNOW		1675
#define MWL_DTREE		1684
#define MWL_SNTREE		1688
#define MWL_ATTREE		1695
#define MWL_AFTREE		1696
#define MWL_RCANYON		1707
#define MWL_JTREE		1902
#define MWL_EMERALD		2208
#define MWL_LIZARD		2214

// Torch Templates
#define MTR_GROLM		 171
#define MTR_DGREY		 250
#define MTR_GOLD		 268
#define MTR_WHITE		 296
#define MTR_FIRE		 522
#define MTR_LGREY		 650
#define MTR_BOWL		 741
#define MTR_BLUE		 764
#define MTR_ICE			 812
#define MTR_GREEN		 822
#define MTR_MARBLE		1055
#define MTR_LAMP		1132
#define MTR_TRASH		1147
#define MTR_BLACK		1645
#define MTR_FPIT		1886
#define MTR_EMERALD		2210

// Decoration Templates
#define MAP_NUM_DECO	   4
#define MDE_MPILLAR		   4
#define MDE_BMARBLE		 123
#define MDE_EARTH	   90153
#define MDE_RUBBLE	   90251
#define MDE_LAVA	   90517
#define MDE_ICE		   90809
#define MDE_BPILLAR		 729
#define MDE_MTABLE		1052
#define MDE_FRUIT	   91434
#define MDE_SBUSH		1440
#define MDE_SPILLAR		1554
#define MDE_SAND	   91555
#define MDE_TBUSH		1591
#define MDE_BARREL	   91711
#define MDE_EMERALD		2209
#define MDE_ESPIKE		2211
#define MDE_CACTUS	   92567

// Scenery Templates
#define MAP_NUM_SCEN	   4
#define MSC_MOSS	   90017
#define MSC_FLOWER	   90016
#define MSC_EARTHR	   90156
#define MSC_JFLOWER	   90201
#define MSC_JFAUNA	   90232
#define MSC_AQUATIC	   90230
#define MSC_LGRASS	   91558
#define MSC_DGRASS	   91583
#define MSC_YGRASS	   91586
#define MSC_PEBBLES		1589
#define MSC_APLANT	   91689
#define MSC_SPLANT	   91692
#define MSC_EMERALD	   92212

// Door Templates
#define MDR_WOOD_N		  22
#define MDR_WOOD_E		  84
#define MDR_DIRT_N		 159
#define MDR_DIRT_E		 160
#define MDR_RED_N		 301
#define MDR_RED_E		 300
#define MDR_IRON_N		 776
#define MDR_IRON_E		 773
#define MDR_MARBLE_N	1062
#define MDR_MARBLE_E	1063
#define MDR_ICE_N		1170
#define MDR_ICE_E		1171
#define MDR_SAND_N		1561
#define MDR_SAND_E		1562
#define MDR_GREEN_N		2216
#define MDR_GREEN_E		2215

// Contract sprites
#define MSP_Y			5602
#define MSP_G			5603
#define MSP_B			5604
#define MSP_R			5601

// Contract Templates
#define MCT_CONTRACT	2507
#define MCT_QUILL_Y		2508
#define MCT_QUILL_G		2509
#define MCT_QUILL_B		2510
#define MCT_QUILL_R		2511

#define MCT_ARTIFACT	2504
#define MCT_PENTIGRAM	2506

// to self: add sprites 760 - 762 from OLD for the map pents

/*************************************************************************

   This file is part of 'Mercenaries of Astonia v2'
   Copyright (c) 1997-2001 Daniel Brockhaus (joker@astonia.com)
   All rights reserved.

 **************************************************************************/

#define CT_GOD_M		   2
#define CT_GOD_F		   3

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
#define CT_HERBCOLL		 703

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

#define CT_STONEGOLEM	 723
#define CT_SEASONSMOB	 724
#define CT_ASCENTMOB	 728
#define CT_CANYONBNDT	 731
#define CT_MUDGOLEM		 732
#define CT_SWAMPLIZ		 733
#define CT_MOUNTVIK		 734
#define CT_GARGNEST		 735

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

#define IT_HERBA		1798
#define IT_HERBB		1799
#define IT_HERBC		1800
#define IT_HERBD		1801

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
#define IT_BREATHAMMY	1275
#define IT_AM_BLOODS	   1
#define IT_AM_VERDANT	   1
#define IT_AM_SEABREZ	   1
#define IT_AM_OBSIDI	   1

// Unique Belts
#define IT_BL_SOLAR		   1
#define IT_BL_LUNAR		   1
#define IT_BL_ECLIPSE	   1

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
#define IT_CH_FOOL_R	   1
#define IT_CH_MAGI_R	   1
#define IT_CH_PREIST_R	   1
#define IT_CH_EMPRES_R     1
#define IT_CH_EMPERO_R	   1
#define IT_CH_HEIROP_R	   1
#define IT_CH_LOVERS_R	   1
#define IT_CH_CHARIO_R	   1
#define IT_CH_STRENG_R	   1
#define IT_CH_HERMIT_R	   1
#define IT_CH_WHEEL_R	   1
#define IT_CH_JUSTIC_R	   1
#define IT_CH_HANGED_R	   1
#define IT_CH_DEATH_R	   1
#define IT_CH_TEMPER_R	   1
#define IT_CH_DEVIL_R	   1
#define IT_CH_TOWER_R	   1
#define IT_CH_STAR_R	   1
#define IT_CH_MOON_R	   1
#define IT_CH_SUN_R		   1
#define IT_CH_JUDGE_R	   1
#define IT_CH_WORLD_R	   1

// TOWER items
#define IT_TW_CROWN		 981
#define IT_TW_CLOAK		 982
#define IT_TW_DREAD		 983
#define IT_TW_DOUSER	   1
#define IT_TW_ROOTS		 984
#define IT_TW_BBELT		 985
#define IT_TW_OUTSIDE	1197
#define IT_TW_PROPHET	1201
#define IT_TW_HEAVENS	1202

// Special Glove items
#define IT_GL_SERPENT	 707
#define IT_GL_SPIDER	 708
#define IT_GL_CURSED	1116
#define IT_GL_TITANS	1117
#define IT_GL_BURNING	1661
#define IT_GL_SHADOW	   1
#define IT_GL_BLVIPER	   1

// Special Boot items
#define IT_BT_NATURES	   1

// Unique weapons
#define IT_WP_LIFESPRIG	   1
#define IT_WP_BLOODLET	   1
#define IT_WP_GEMCUTTER	   1
#define IT_WP_STARLIGHT	   1
#define IT_WP_KELPTRID	   1
#define IT_WP_PHALLENX	   1
#define IT_WP_LAMEDARG	   1
#define IT_WP_WHITEODA	   1
#define IT_WP_EXCALIBUR	   1
#define IT_WP_FELLNIGHT	   1
#define IT_WP_BLACKTAC	   1
#define IT_WP_CRIMRIP	   1
#define IT_WP_CRESSUN	   1
#define IT_WP_GILDSHINE	   1
#define IT_WP_BRONCHIT	   1
#define IT_WP_VOLCANF	   1

// -----

// Standard flowers; Red, Green, Purple, Yellow, Blue
#define IT_FLOWER_R		  18
#define IT_FLOWER_G		 142
#define IT_FLOWER_P		  46
#define IT_FLOWER_Y		 140
#define IT_FLOWER_B		 141

// Jungle flowers; Pink, Cyan, Verdant
#define IT_FLOWER_JP	 197
#define IT_FLOWER_JC	 198
#define IT_FLOWER_JV	 199

#define IT_FLASK		 100

// Standard flower mixtures
#define IT_MIX_Y___		 143
#define IT_MIX_B___		 144
#define IT_MIX_YR__		 145
#define IT_MIX_YG__		 146
#define IT_MIX_YP__		 147
#define IT_MIX_YRG_		 148
#define IT_MIX_YGP_		 149
#define IT_MIX_YPR_		 158
#define IT_MIX_YRGP		 161

// Jungle flower mixtures
#define IT_MIX_JP__		 218
#define IT_MIX_JC__		 219
#define IT_MIX_JV__		 220
#define IT_MIX_JPJC		 221
#define IT_MIX_JCJV		 222
#define IT_MIX_JVJP		 223

// Special Flowers
#define IT_TULIP		1540
#define IT_ROSE			1542

// Potions
#define IT_POT_L_HP		 101
#define IT_POT_L_EN		 125
#define IT_POT_L_MP		 102

#define IT_POT_RAIN		 274

#define IT_POT_G_HP		 127
#define IT_POT_G_EN		 273
#define IT_POT_G_MP		 131

#define IT_POT_HPEN		 293
#define IT_POT_ENMP		 294
#define IT_POT_MPHP		 295

#define IT_POT_LIFE		 181

#define IT_POT_DEAD		  99

#define IT_POT_STRE		 135
#define IT_POT_AGIL		 224

#define IT_POT_GOLM		 449

#define IT_POT_TULI		1543
#define IT_POT_ROSE		1544

// -----

#define IT_PICKAXE		 458

#define IT_SILV			1342
#define IT_GOLD			1343
#define IT_PLAT			1344

#define IT_SILV_RING	1345
#define IT_GOLD_RING	1346
#define IT_PLAT_RING	1347

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

#define IT_E_WK			1465
#define IT_E_SK			359
#define IT_E_GO			371
#define IT_E_GA			372
#define IT_E_MA			1466

// -----

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
#define IT_BOOK_BISH	   1
#define IT_BOOK_GREA	   1


// -------- pop_create_bonus -------- //

#define IT_POP_REDFL	  18
#define IT_POP_PURFL	  46
#define IT_POP_GREFL	 142
#define IT_POP_YELFL	 140
#define IT_POP_BLUFL	 141
#define IT_POP_FLASK	 100
#define IT_POP_HEALP	 101
#define IT_POP_ENDUP	 125
#define IT_POP_MANAP	 102
#define IT_POP_RAINP	 274
#define IT_POP_GHEAL	 127
#define IT_POP_GENDU	 273
#define IT_POP_GMANA	 131
#define IT_POP_VITPT	 293
#define IT_POP_CLAPT	 294
#define IT_POP_SAGPT	 295
#define IT_POP_LIFEP	 181
#define IT_POP_SRING	1345
#define IT_POP_GRING	1346
#define IT_POP_PRING	1347
#define IT_POP_GEMSM	1348 // +0 - +5
#define IT_POP_GEMMD	1354 // +0 - +5
#define IT_POP_GEMBG	1360 // +0 - +5
#define IT_POP_GEMHU	1366 // +0 - +5
#define IT_POP_GEMFL	1372 // +0 - +5
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
#define IT_RD_BELTTHR	1796
#define IT_RD_BELTFOU	1797

#define IT_RD_BRV		182
#define IT_RD_WIL		183
#define IT_RD_INT		184
#define IT_RD_AGL		185
#define IT_RD_STR		186
#define IT_RD_HP		187
#define IT_RD_EN		188
#define IT_RD_MP		189

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
#define BUF_SPR_RECALL		90
#define BUF_SPR_SHADOW		179
#define BUF_SPR_REND		224
#define BUF_SPR_WARCRY		325
#define BUF_SPR_WARCRY2		91
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
#define BUF_SPR_BLIND		93
#define BUF_SPR_PULSE		93
#define BUF_SPR_TAUNT		93
#define BUF_SPR_GUARD		93
#define BUF_SPR_RAZOR		93
#define BUF_SPR_RAZOR2		93
#define BUF_SPR_DOUSE		93

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


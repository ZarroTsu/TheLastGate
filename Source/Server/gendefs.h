/*************************************************************************

   This file is part of 'Mercenaries of Astonia v2'
   Copyright (c) 1997-2001 Daniel Brockhaus (joker@astonia.com)
   All rights reserved.

 **************************************************************************/

#define VERSION				0x000902
#define MINVERSION			0x000902

#define DATDIR				".dat"

#define TICKS				20
#define TICK				(1000000/TICKS)

#define MAPX				1024
#define MAPY				2048
#define MAXCHARS			8192
#define MAXBUFF				(40*MAXCHARS)
#define MAXITEM				(96*1024)
#define MAXEFFECT			4096
#define MAXMISSION			1024
#define MAXSKILL			  50
#define MAXBUFFS			  40
#define MAXWPS				  27

#define MAXTCHARS			4548
#define MAXTITEM			4548

#define LIGHTDIST			  10
#define LENDESC				 200

#define HOME_START_X		 768
#define HOME_START_Y		 256

#define HOME_TEMPLE_X		 512
#define HOME_TEMPLE_Y		 512

#define HOME_PURPLE_X		 569
#define HOME_PURPLE_Y		 455

#define HOME_KWAI_X			 707
#define HOME_KWAI_Y			 856

#define HOME_GORN_X			 795
#define HOME_GORN_Y			 788

#define HOME_STAFF_X		 512
#define HOME_STAFF_Y		 512

#define LAB_ARENA_SPAWN_X 	 142
#define LAB_ARENA_SPAWN_Y 	 119

#define LAB_ARENA_BOSS_X 	 142
#define LAB_ARENA_BOSS_Y 	 125

#define LAB_ARENA_TOP_X 	 134
#define LAB_ARENA_TOP_Y 	 114

#define LAB_ARENA_BOTTOM_X 	 150
#define LAB_ARENA_BOTTOM_Y 	 130

#define OS_WARP_X			 758
#define OS_WARP_Y			 981

#define BRAVER_WARP_X		 959
#define BRAVER_WARP_Y		 519

#define COLOSSEUM1_X		 751
#define COLOSSEUM1_Y		 933
#define COLOS1_X1			 733
#define COLOS1_Y1			 915
#define COLOS1_X2			 769
#define COLOS1_Y2			 951

#define COLOSSEUM2_X		 718
#define COLOSSEUM2_Y		 908
#define COLOS2_X1			 700
#define COLOS2_Y1			 890
#define COLOS2_X2			 736
#define COLOS2_Y2			 926

#define COLOSSEUM3_X		 784
#define COLOSSEUM3_Y		 908
#define COLOS3_X1			 766
#define COLOS3_Y1			 890
#define COLOS3_X2			 802
#define COLOS3_Y2			 926

#define COLOSSEUM4_X		 718
#define COLOSSEUM4_Y		 958
#define COLOS4_X1			 700
#define COLOS4_Y1			 940
#define COLOS4_X2			 736
#define COLOS4_Y2			 976

#define COLOSSEUM5_X		 784
#define COLOSSEUM5_Y		 958
#define COLOS5_X1			 766
#define COLOS5_Y1			 940
#define COLOS5_X2			 802
#define COLOS5_Y2			 976

#define PANDI_X1			 275
#define PANDI_Y1			 981
#define PANDI_X2			 291
#define PANDI_Y2			 997
#define PANDI_MIDX			 283
#define PANDI_MIDY			 989

#define LAB_I_X				  74
#define LAB_I_Y				  25
#define LAB_II_X			  25
#define LAB_II_Y			 127
#define LAB_III_X			  48
#define LAB_III_Y			 173
#define LAB_IV_X			  39
#define LAB_IV_Y			 250
#define LAB_V_X				  30
#define LAB_V_Y				 379
#define LAB_VI_X			  77
#define LAB_VI_Y			 462
#define LAB_VII_X			  37
#define LAB_VII_Y			 494
#define LAB_VIII_X			  25
#define LAB_VIII_Y			 576
#define LAB_IX_X			 121
#define LAB_IX_Y			 661
#define LAB_X_X				 137
#define LAB_X_Y				 785
#define LAB_XI_X			  25
#define LAB_XI_Y			 902
#define LAB_XII_X			 152
#define LAB_XII_Y			 993
#define LAB_XIII_X			 142
#define LAB_XIII_Y			 160

#define CNTSAY				(TICKS)
#define MAXSAY				(TICKS*7)

#define GODPASSWORD			"xxxxxxxxxxxx"

// wear positions
#define WN_HEAD			0
#define WN_NECK			1
#define WN_BODY			2
#define WN_ARMS			3
#define WN_BELT			4
#define WN_CHARM		5
#define WN_FEET			6
#define WN_LHAND		7		// shield
#define WN_RHAND		8		// weapon
#define WN_CLOAK		9
#define WN_LRING		10
#define WN_RRING		11
#define WN_CHARM2		12

// Sneaky Packet Hacks		   buf + 5	    buf + 7
#define WN_SPEED		13  // Speed 	and Attack Speed
#define WN_SPMOD		14  // Spellmod	and Spellapt
#define WN_CRIT			15  // Crit Chc	and Crit Mult
#define WN_TOP			16  // TopDmg	and Reflection
#define WN_HITPAR		17  // Hit 		and Parry
#define WN_CLDWN		18  // Cooldown and Cast Speed
#define WN_FLAGS		19  // Special flags

// placement bits
#define PL_HEAD			1
#define PL_NECK			2
#define PL_BODY			4
#define PL_ARMS			8
#define PL_BELT			32
#define PL_CHARM		64
#define PL_FEET			128
#define PL_WEAPON		256
#define PL_SHIELD		512		// not usable with two-handed weapons
#define PL_CLOAK		1024
#define PL_TWOHAND		2048
#define PL_RING			4096
#define PL_SOULSTONED	16384
#define PL_ENCHANTED	32768

#define DX_RIGHT		1		// South
#define DX_LEFT			2		// North
#define DX_UP			3		// West
#define DX_DOWN			4		// East
#define DX_LEFTUP		5
#define DX_LEFTDOWN		6
#define DX_RIGHTUP		7
#define DX_RIGHTDOWN	8

// driver stuff
#define DR_IDLE			0
#define DR_DROP			1
#define DR_PICKUP		2
#define DR_GIVE			3
#define DR_USE			4
#define DR_BOW			5
#define DR_WAVE			6
#define DR_TURN			7
#define DR_SINGLEBUILD	8
#define DR_AREABUILD1	9
#define DR_AREABUILD2	10

#define NT_NONE			0
#define NT_GOTHIT		1
#define NT_GOTMISS		2
#define NT_DIDHIT		3
#define NT_DIDMISS		4
#define NT_DIDKILL		5
#define NT_GOTEXP		6
#define NT_SEEHIT		7
#define NT_SEEMISS		8
#define NT_SEEKILL		9
#define NT_GIVE			11
#define NT_SEE			12
#define NT_DIED			13
#define NT_SHOUT		14
#define NT_HITME		15

#define SP_LIGHT	(1u<<0)
#define SP_PROTECT	(1u<<1)
#define SP_ENHANCE	(1u<<2)
#define SP_BLESS	(1u<<3)
#define SP_HEAL		(1u<<4)
#define SP_CURSE	(1u<<5)
#define SP_SLOW		(1u<<6)
#define SP_DISPEL	(1u<<7)
#define SP_WARCRY	(1u<<8)
#define SP_REND		(1u<<9)
#define SP_POISON	(1u<<10)
#define SP_HASTE	(1u<<11)
#define SP_BLIND	(1u<<12)
#define SP_PULSE	(1u<<13)
#define SP_TAUNT	(1u<<14)
#define SP_ZEPHYR	(1u<<15)
#define SP_MSHIELD	(1u<<16)
#define SP_RAGE		(1u<<17)
#define SP_LETHARGY	(1u<<18)

// -------- Spell Cost values -------- //
#define SP_COST_BASE		 5
#define SP_COST_LIGHT		 5
#define SP_COST_PROTECT		15
#define SP_COST_ENHANCE		15
#define SP_COST_BLESS		35
#define SP_COST_MSHIELD		25
#define SP_COST_HASTE		35
#define SP_COST_HEAL		25
#define SP_COST_CURSE		30
#define SP_COST_SLOW		20
#define SP_COST_POISON		30
#define SP_COST_IDENT		20
#define SP_COST_RECALL		10
#define SP_COST_DISPEL		40
#define SP_COST_GHOST		45
#define SP_COST_SHADOW		50
#define SP_COST_PULSE		50
#define SP_COST_ZEPHYR		45
#define SP_COST_LETHARGY	40

#define SP_COST_WEAKEN		20
#define SP_COST_BLIND		20
#define SP_COST_TAUNT		15
#define SP_COST_WARCRY		25
#define SP_COST_RAGE		20

#define FIVE_PERC_FAIL		18
#define TEN_PERC_FAIL		17

// -------- Debuff Multipliers -------- //

#define SP_MULT_CURSE		100
#define SP_MULT_CURSE2		105
#define SP_MULT_SLOW		120
#define SP_MULT_SLOW2		115
#define SP_MULT_POISON		115
#define SP_MULT_POISON2		110
#define SP_MULT_IDENTIFY	180
#define SP_MULT_DISPEL		 80
#define SP_MULT_DISPEL2		 75
#define SP_MULT_WEAKEN		100
#define SP_MULT_WEAKEN2		 95
#define SP_MULT_WARCRY		 95
#define SP_MULT_TAUNT		 80
#define SP_MULT_BLIND		100

// -------- Damage Multipliers -------- //

#define DAM_MULT_HIT		 250
#define DAM_MULT_BLAST		 625
#define DAM_MULT_HOLYW		 750
#define DAM_MULT_THORNS		1000
#define DAM_MULT_CLEAVE		 500
#define DAM_MULT_PULSE		 100
#define DAM_MULT_ZEPHYR		  50
#define DAM_MULT_LEAP		 250

// -------- Buff Duration values -------- //
#define SP_DUR_LIGHT		( TICKS * 60 * 30                 )
#define SP_DUR_MONSTERS		( TICKS * 60 * 20                 )
#define SP_DUR_PROTECT		( TICKS * 60 * 10                 )
#define SP_DUR_ENHANCE		( TICKS * 60 * 10                 )
#define SP_DUR_BLESS		( TICKS * 60 * 10                 )
#define SP_DUR_MSHELL(a)	( a * 128                         )
#define SP_DUR_MSHIELD(a)	( a * 256                         )
#define SP_DUR_HASTE		( TICKS * 60 * 10                 )
#define SP_DUR_REGEN		( TICKS * 20                      )
#define SP_DUR_HEAL			( TICKS * 60                      )
#define SP_DUR_CURSE2		( TICKS * 40                      )
#define SP_DUR_CURSE		( TICKS * 60 * 2                  )
#define SP_DUR_SLOW2(a)		( TICKS * 30 + TICKS * a / 2      )
#define SP_DUR_SLOW(a)		( TICKS * 15 + TICKS * a / 4      )
#define SP_DUR_POISON		( TICKS * 30                      )
#define SP_DUR_WARCRY2(a)	( TICKS + TICKS * a / 75          )
#define SP_DUR_WARCRY		( TICKS * 60                      )
#define SP_DUR_RALLY		( TICKS * 60 * 5                  )
#define SP_DUR_SCORCH		( TICKS * 20                      )
#define SP_DUR_RECALL(a)	( max(TICKS / 3, 66 - (a / 5))    )
#define SP_DUR_SHADOW(a)	( TICKS * 20 + TICKS * a / 5      )
#define SP_DUR_BLEED		( TICKS * 15                      )
#define SP_DUR_WEAKEN		( TICKS * 60                      )
#define SP_DUR_RAGE			( TICKS * 60 * 60 * 2             )
#define SP_DUR_ARIA			( TICKS * 5                       )
#define SP_DUR_LETHARGY		( TICKS * 60 * 60 * 2             )
#define SP_DUR_AGGRAVATE	( TICKS * 20                      )

#define SP_DUR_DISPEL(a)	( TICKS * 15 + TICKS * a / 10     )
#define SP_DUR_BLIND		( TICKS * 60 * 2                  )
#define SP_DUR_TAUNT		( TICKS *  5                      )
#define SP_DUR_GUARD		( TICKS * 30                      )
#define SP_DUR_PULSE		( TICKS * 60 * 5                  )
#define SP_DUR_ZEPHYR		( TICKS * 60 * 5                  )
#define SP_DUR_DOUSE		( TICKS * 20                      )
#define SP_DUR_FROSTB		( TICKS * 20                      )
#define SP_DUR_POME			( TICKS *  5                      )
#define SP_DUR_SOL			( TICKS *  5                      )
#define SP_DUR_STARL		( TICKS * 60 * 5                  )
#define SP_DUR_PHALANX		( TICKS * 60                      )

#define SP_DUR_SHRINE		( TICKS * 60 * 30                 )
#define SP_DUR_MAPMOD		( TICKS * 60 * 120                )

// -------- Skill Exhaustion values -------- //
#define SK_EXH_MAXIMUM	 	(TICKS * 8)
#define SK_EXH_LIGHT		(TICKS / 4)
#define SK_EXH_PROTECT		(TICKS / 2)
#define SK_EXH_ENHANCE		(TICKS / 2)
#define SK_EXH_BLESS		(TICKS)
#define SK_EXH_MSHIELD		(TICKS * 3)
#define SK_EXH_HASTE		(TICKS * 3)
#define SK_EXH_HEAL			(TICKS * 3)
#define SK_EXH_CURSE		(TICKS * 4)
#define SK_EXH_SLOW			(TICKS * 4)
#define SK_EXH_POISON		(TICKS * 5)
#define SK_EXH_WARCRY		(TICKS * 3)
#define SK_EXH_IDENT		(TICKS * 2)
#define SK_EXH_BLAST		(TICKS * 6)
#define SK_EXH_RECALL		(TICKS)
#define SK_EXH_DISPEL		(TICKS * 4)
#define SK_EXH_GHOST		(TICKS * 8)
#define SK_EXH_SHADOW		(TICKS * 4)
#define SK_EXH_CLEAVE		(TICKS * 5)
#define SK_EXH_WEAKEN		(TICKS * 3)
#define SK_EXH_LETHARGY		(TICKS * 4)

#define SK_EXH_PULSE		(TICKS * 6)
#define SK_EXH_ZEPHYR		(TICKS * 4)
#define SK_EXH_BLIND		(TICKS * 3)
#define SK_EXH_TAUNT		(TICKS * 3)
#define SK_EXH_LEAP			(TICKS * 5)
#define SK_EXH_RAGE			(TICKS * 4)

#define GLVDICE		4

extern char *at_name[];

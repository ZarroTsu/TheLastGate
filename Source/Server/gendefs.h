/*************************************************************************

   This file is part of 'Mercenaries of Astonia v2'
   Copyright (c) 1997-2001 Daniel Brockhaus (joker@astonia.com)
   All rights reserved.

 **************************************************************************/

#define max(a, b) ((a)>(b) ? (a) : (b))
#define min(a, b) ((a)<(b) ? (a) : (b))
#define RANDOM(a) (a>0?random()%(a):0)
#define DATDIR ".dat"

#define VERSION    0x000604
#define MINVERSION 0x000604

#define TICKS 18
#define TICK  (1000000/TICKS)

#define MAPX       1024
#define MAPY       1024
#define MAXCHARS   8192
#define MAXBUFF    65535
#define MAXITEM    (96*1024)
#define MAXEFFECT  4096
#define MAXMISSION 1024
#define MAXSKILL   50
#define MAXBUFFS   40
#define MAXWPS     20

#define MAXTCHARS 4548
#define MAXTITEM  4548

#define LIGHTDIST 10
#define LENDESC   200

#define HOME_START_X		 768
#define HOME_START_Y		 256

#define HOME_TEMPLE_X		 512
#define HOME_TEMPLE_Y		 512

#define HOME_PURPLE_X		 569
#define HOME_PURPLE_Y		 455

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

#define LAB_I_X				  74 // R74C25
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
//
#define LAB_VII_X			  37 // R37C494
#define LAB_VII_Y			 494
#define LAB_VIII_X			  25 // R25C576
#define LAB_VIII_Y			 576
#define LAB_IX_X			 121 // R121C661
#define LAB_IX_Y			 661
#define LAB_X_X				 137 // R137C785
#define LAB_X_Y				 785
#define LAB_XI_X			  25 // R25C902
#define LAB_XI_Y			 902
#define LAB_XII_X			 152 // R152C993
#define LAB_XII_Y			 993
//
#define LAB_XIII_X			 142
#define LAB_XIII_Y			 160


#define CNTSAY (TICKS)
#define MAXSAY (TICKS*7)

#define GODPASSWORD "xxxxxxxxxxxx"

// wear positions
#define WN_HEAD  	0
#define WN_NECK  	1
#define WN_BODY  	2
#define WN_ARMS  	3
#define WN_BELT  	4
#define WN_CHARM 	5
#define WN_FEET  	6
#define WN_LHAND 	7		// shield
#define WN_RHAND 	8		// weapon
#define WN_CLOAK 	9
#define WN_LRING 	10
#define WN_RRING 	11
#define WN_CHARM2 	12

// Sneaky Packet Hacks		   buf + 5	    buf + 7
#define WN_SPEED		13  // Speed 	and Attack Speed
#define WN_SPMOD		14  // Spellmod	and Spellapt
#define WN_CRIT			15  // Crit Chc	and Crit Mult
#define WN_TOP			16  // TopDmg	and Reflection
#define WN_HITPAR		17  // Hit 		and Parry
#define WN_CLDWN		18  // Cooldown and Cast Speed
#define WN_FLAGS		19  // Special flags

// placement bits
#define PL_HEAD       1
#define PL_NECK       2
#define PL_BODY       4
#define PL_ARMS       8
#define PL_BELT      32
#define PL_CHARM     64
#define PL_FEET     128
#define PL_WEAPON   256
#define PL_SHIELD   512					// not usable with two-handed weapons
#define PL_CLOAK   1024
#define PL_TWOHAND 2048
#define PL_RING    4096

#define DX_RIGHT     1
#define DX_LEFT      2
#define DX_UP        3
#define DX_DOWN      4
#define DX_LEFTUP    5
#define DX_LEFTDOWN  6
#define DX_RIGHTUP   7
#define DX_RIGHTDOWN 8

// driver stuff
#define DR_IDLE        0
#define DR_DROP        1
#define DR_PICKUP      2
#define DR_GIVE        3
#define DR_USE         4
#define DR_BOW         5
#define DR_WAVE        6
#define DR_TURN        7
#define DR_SINGLEBUILD 8
#define DR_AREABUILD1  9
#define DR_AREABUILD2  10

#define NT_NONE    0
#define NT_GOTHIT  1
#define NT_GOTMISS 2
#define NT_DIDHIT  3
#define NT_DIDMISS 4
#define NT_DIDKILL 5
#define NT_GOTEXP  6
#define NT_SEEHIT  7
#define NT_SEEMISS 8
#define NT_SEEKILL 9
#define NT_GIVE    11
#define NT_SEE     12
#define NT_DIED    13
#define NT_SHOUT   14
#define NT_HITME   15

#define SP_LIGHT   (1u<<0)
#define SP_PROTECT (1u<<1)
#define SP_ENHANCE (1u<<2)
#define SP_BLESS   (1u<<3)
#define SP_HEAL    (1u<<4)
#define SP_CURSE   (1u<<5)
#define SP_SLOW    (1u<<6)
#define SP_DISPEL  (1u<<7)
#define SP_WARCRY  (1u<<8)
#define SP_REND    (1u<<9)
#define SP_POISON  (1u<<10)
#define SP_HASTE   (1u<<11)
#define SP_BLIND   (1u<<12)
#define SP_PULSE   (1u<<13)
#define SP_TAUNT   (1u<<14)
#define SP_ZEPHYR   (1u<<15)
#define SP_MSHIELD (1u<<16)

// -------- Spell Cost values -------- //
#define SP_COST_BASE		 5
#define SP_COST_LIGHT		 5
#define SP_COST_PROTECT		15
#define SP_COST_ENHANCE		15
#define SP_COST_BLESS		35
#define SP_COST_MSHIELD		25
#define SP_COST_HASTE		75
#define SP_COST_HEAL		25
#define SP_COST_CURSE		35
#define SP_COST_SLOW		20
#define SP_COST_POISON		40
#define SP_COST_IDENT		20
#define SP_COST_RECALL		15
#define SP_COST_DISPEL		45
#define SP_COST_GHOST		45
#define SP_COST_SHADOW		50
#define SP_COST_PULSE		60
#define SP_COST_ZEPHYR		20

#define SP_COST_WEAKEN		10
#define SP_COST_BLIND		10
#define SP_COST_TAUNT		15
#define SP_COST_WARCRY		25

#define FIVE_PERC_FAIL		18
#define TEN_PERC_FAIL		17

// -------- Buff Duration values -------- //
#define SP_DUR_LIGHT		( TICKS * 60 * 30                 )
#define SP_DUR_MONSTERS		( TICKS * 60 * 20                 )
#define SP_DUR_PROTECT		( TICKS * 60 * 10                 )
#define SP_DUR_ENHANCE		( TICKS * 60 * 10                 )
#define SP_DUR_BLESS		( TICKS * 60 * 10                 )
#define SP_DUR_MSHELL(a)	( a * 128                         )
#define SP_DUR_MSHIELD(a)	( a * 256                         )
#define SP_DUR_HASTE		( TICKS * 60 * 10                 )
#define SP_DUR_REGEN		( TICKS * 15                      )
#define SP_DUR_HEAL			( TICKS * 60                      )
#define SP_DUR_CURSE2		( TICKS * 40                      )
#define SP_DUR_CURSE		( TICKS * 60 * 2                  )
#define SP_DUR_SLOW2(a)		( TICKS * 30 + TICKS * a / 2      )
#define SP_DUR_SLOW(a)		( TICKS * 15 + TICKS * a / 4      )
#define SP_DUR_POISON		( TICKS * 30                      )
#define SP_DUR_WARCRY2(a)	( TICKS + TICKS * a / 80          )
#define SP_DUR_WARCRY		( TICKS * 60                      )
#define SP_DUR_SCORCH		( TICKS * 20                      )
#define SP_DUR_RECALL(a)	( max(TICKS / 2, 60 - (a / 4))    )
#define SP_DUR_SHADOW(a)	( TICKS * 10 + TICKS * a / 6      )
#define SP_DUR_BLEED		( TICKS * 10                      )
#define SP_DUR_WEAKEN		( TICKS * 60                      )

#define SP_DUR_DISPEL		( TICKS * 15                      )
#define SP_DUR_BLIND		( TICKS * 60 * 2                  )
#define SP_DUR_TAUNT		( TICKS *  5                      )
#define SP_DUR_GUARD		( TICKS * 15                      )
#define SP_DUR_PULSE		( TICKS * 60 * 1                  )
#define SP_DUR_ZEPHYR		( TICKS * 60 * 5                  )
#define SP_DUR_DOUSE		( TICKS * 20                      )
#define SP_DUR_FROSTB		( TICKS * 20                      )

// -------- Skill Exhaustion values -------- //
#define SK_EXH_MAXIMUM	 	(TICKS * 6)
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
#define SK_EXH_GHOST		(TICKS * 4)
#define SK_EXH_SHADOW		(TICKS * 4)
#define SK_EXH_CLEAVE		(TICKS * 5)
#define SK_EXH_WEAKEN		(TICKS * 3)

#define SK_EXH_PULSE		(TICKS * 5)
#define SK_EXH_ZEPHYR		(TICKS * 5)
#define SK_EXH_BLIND		(TICKS * 3)
#define SK_EXH_TAUNT		(TICKS * 3)
#define SK_EXH_LEAP(a)		(TICKS * 6 + TICKS * a / 2)



extern char *at_name[];

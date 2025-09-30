/*************************************************************************

   This file is part of 'Mercenaries of Astonia v2'
   Copyright (c) 1997-2001 Daniel Brockhaus (joker@astonia.com)
   All rights reserved.

 **************************************************************************/

#define VERSION				0x000D01
#define MINVERSION			0x000D00

#define DATDIR				".dat"

#define TICKS				20
#define TICK				(1000000/TICKS)

#define GAMEYEAR			  40

#define MAPX				1024
#define MAPY				2048
#define MAXCHARS			(16*1024)
#define MAXBUFF				(40*MAXCHARS)
#define MAXITEM				(64*1024*4)
#define MAXEFFECT			( 8*1024)
#define MAXMISSION			1024
#define MAXSKILL			  50
#define MAXITEMS			  60
#define MAXGSLOTS			  13
#define MAXBUFFS			  40
#define MAXWPS				  27

#define MAXTCHARS			( 8*1024)
#define MAXTITEM			( 8*1024)

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

#define LYCAN_WARP_X		 889
#define LYCAN_WARP_Y		 485

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

#define AAPPROACH_X1		 421
#define AAPPROACH_Y1		 793
#define AAPPROACH_X2		 465
#define AAPPROACH_Y2		 832

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
//
#define LAB_XIV_X			 118
#define LAB_XIV_Y			1095
#define LAB_XV_X			 133
#define LAB_XV_Y			1152
#define LAB_XVI_X			 145
#define LAB_XVI_Y			1276
#define LAB_XVII_X			 163
#define LAB_XVII_Y			1398
#define LAB_XVIII_X			  98
#define LAB_XVIII_Y			1478
#define LAB_XIX_X			 256
#define LAB_XIX_Y			1665
#define LAB_XX_X			 204
#define LAB_XX_Y			1481
//
#define TLG_X1				 194
#define TLG_Y1				1544
#define TLG_X2				 214
#define TLG_Y2				1564
#define TLG_MIDX			 204
#define TLG_MIDY			1554

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
#define PL_CORRUPTED	8192
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
#define SP_WEAKEN	(1u<<9)
#define SP_POISON	(1u<<10)
#define SP_HASTE	(1u<<11)
#define SP_BLIND	(1u<<12)
#define SP_PULSE	(1u<<13)
#define SP_TAUNT	(1u<<14)
#define SP_ZEPHYR	(1u<<15)
#define SP_MSHIELD	(1u<<16)
#define SP_RAGE		(1u<<17)
#define SP_LETHARGY	(1u<<18)
#define SP_WEAKEN2	(1u<<19)
#define SP_CURSE2	(1u<<20)
#define SP_SLOW2	(1u<<21)
#define SP_DOUSE	(1u<<22)
#define SP_MSHELL	(1u<<23)
#define SP_WARCRY3	(1u<<24)
#define SP_DISPEL2	(1u<<25)
#define SP_VENOM	(1u<<26)
#define SP_PULSE2	(1u<<27)
#define SP_REGEN	(1u<<28)
#define SP_IMMOLATE	(1u<<29)

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
#define SP_MULT_DISPEL		 90
#define SP_MULT_DISPEL2		 80
#define SP_MULT_WEAKEN		110
#define SP_MULT_WEAKEN2		105
#define SP_MULT_WARCRY		 95
#define SP_MULT_TAUNT		100
#define SP_MULT_BLIND		125

// -------- Damage Multipliers -------- //

#define DAM_MULT_HIT		 250
#define DAM_MULT_BLAST		 625
#define DAM_MULT_HOLYW		 750
#define DAM_MULT_THORNS		1000
#define DAM_MULT_CLEAVE		 500
#define DAM_MULT_PULSE		 100
#define DAM_MULT_ZEPHYR		  50
#define DAM_MULT_LEAP		 375
#define DAM_MULT_RLEAP		 150

// -------- Buff Duration values -------- //
#define SP_DUR_MAPMOD		( TICKS * 60 * 60 * 2             )
#define SP_DUR_SHRINE		( TICKS * 60 * 30                 )
#define SP_DUR_LIGHT		( TICKS * 60 * 30                 )
#define SP_DUR_MONSTERS		( TICKS * 60 * 30                 )
#define SP_DUR_PROTECT		( TICKS * 60 * 10                 )
#define SP_DUR_ENHANCE		( TICKS * 60 * 10                 )
#define SP_DUR_BLESS		( TICKS * 60 * 10                 )
#define SP_DUR_HASTE		( TICKS * 60 * 10                 )
#define SP_DUR_RALLY		( TICKS * 60 * 10                 )
#define SP_DUR_PULSE		( TICKS * 60 * 10                 )
#define SP_DUR_ZEPHYR		( TICKS * 60 * 10                 )
#define SP_DUR_STARL		( TICKS * 60 * 10                 )
#define SP_DUR_PHALANX		( TICKS * 60 * 10                 )

#define SP_DUR_CURSE		( TICKS * 60 *  2                 )
#define SP_DUR_PLAGUE		( TICKS * 60 *  2                 )
#define SP_DUR_WARCRY		( TICKS * 60 *  2                 )
#define SP_DUR_WEAKEN		( TICKS * 60 *  2                 )
#define SP_DUR_BLIND		( TICKS * 60 *  2                 )
#define SP_DUR_SHOCK		( TICKS * 60 *  2                 )
#define SP_DUR_DOUSE		( TICKS * 60 *  2                 )
#define SP_DUR_FROSTB		( TICKS * 60 *  2                 )

#define SP_DUR_CURSE2		( TICKS * 60                      )
#define SP_DUR_HEAL			( TICKS * 60                      )
#define SP_DUR_POISON		( TICKS * 30                      )
#define SP_DUR_GUARD		( TICKS * 30                      )
#define SP_DUR_SCORCH		( TICKS * 20                      )
#define SP_DUR_AGGRAVATE	( TICKS * 20                      )
#define SP_DUR_REGEN		( TICKS * 20                      )
#define SP_DUR_BLEED		( TICKS * 15                      )
#define SP_DUR_POME			( TICKS *  5                      )
#define SP_DUR_SOL			( TICKS *  5                      )
#define SP_DUR_ARIA			( TICKS *  5                      )
#define SP_DUR_RAGE			( TICKS *  5                      )
#define SP_DUR_LETHARGY		( TICKS *  5                      )

#define SP_DUR_MSHELL(a)	( a * 128                         )
#define SP_DUR_MSHIELD(a)	( a * 256                         )
#define SP_DUR_SLOW2(a)		( TICKS * 30 + TICKS * a / 2      )
#define SP_DUR_SLOW(a)		( TICKS * 15 + TICKS * a / 4      )
#define SP_DUR_WARCRY2(a)	( TICKS + TICKS * a / 75          )
#define SP_DUR_RECALL(a)	( max(TICKS / 3, 66 - (a / 5))    )
#define SP_DUR_SHADOW(a)	( TICKS * 20 + TICKS * a / 5      )
#define SP_DUR_DISPEL(a)	( TICKS * 15 + TICKS * a / 5      )

// -------- Skill Exhaustion values -------- //
#define SK_EXH_MAXIMUM	 	(TICKS *10)
#define SK_EXH_LIGHT		(TICKS / 2)
#define SK_EXH_PROTECT		(TICKS / 2)
#define SK_EXH_ENHANCE		(TICKS / 2)
#define SK_EXH_BLESS		(TICKS / 2)
#define SK_EXH_HASTE		(TICKS / 2)
#define SK_EXH_RECALL		(TICKS / 2)
#define SK_EXH_ZEPHYR		(TICKS / 2)
#define SK_EXH_RAGE			(TICKS / 2)
#define SK_EXH_PULSE		(TICKS / 2)
#define SK_EXH_IDENT		(TICKS / 2)
#define SK_EXH_LETHARGY		(TICKS * 2)
#define SK_EXH_MSHIELD		(TICKS * 2)
#define SK_EXH_HEAL			(TICKS * 2)
#define SK_EXH_WEAKEN		(TICKS * 3)
#define SK_EXH_WARCRY		(TICKS * 3)
#define SK_EXH_BLIND		(TICKS * 3)
#define SK_EXH_TAUNT		(TICKS * 3)
#define SK_EXH_CURSE		(TICKS * 4)
#define SK_EXH_SLOW			(TICKS * 4)
#define SK_EXH_DISPEL		(TICKS * 4)
#define SK_EXH_SHADOW		(TICKS * 4)
#define SK_EXH_CLEAVE		(TICKS * 5)
#define SK_EXH_POISON		(TICKS * 5)
#define SK_EXH_LEAP			(TICKS * 6)
#define SK_EXH_BLAST		(TICKS * 6)
#define SK_EXH_GHOST		(TICKS * 8)
#define SK_EXH_SHIFT		(TICKS * 8)

#define GLVDICE		4

extern char *at_name[];

// -------- Contract Names and Modifiers -------- //

#define MSN_CN			"CONTRACT: "
#define MSN_00			"Find the exit!"
#define MSN_01			"Defeat all enemies!"
#define MSN_02			"Defeat all divine enemies!"
#define MSN_03			"Defeat all cruel enemies!"
#define MSN_04			"Find the artifact!"
#define MSN_05			"Find all the shrines!"
#define MSN_06			"Find all the chests!"
#define MSN_07			"Touch all the pentagrams!"
#define MSN_08			"Defeat the timid enemy!"
#define MSN_09			"Defeat the unique enemy!"

// 						"!        .         .   |     .         .        !"
#define CFL_P_CHST		"  Area has %d additional chests\n"
#define CFL_P_SHRN		"  Area has %d additional shrines\n"
#define CFL_P_XEXP		"  Exit grants %d%% more exp\n"
#define RATE_P_XEXP		25
#define CFL_P_XLUK		"  Exit grants %d%% of exp as luck\n"
#define RATE_P_XLUK		5
#define CFL_P_XBSP		"  Exit grants %d%% of exp as stronghold pts\n"
#define RATE_P_XBSP		10
#define CFL_P_XOSP		"  Exit grants %d%% of exp as Osiris pts\n"
#define RATE_P_XOSP		10
#define CFL_P_PLXP		"  Players earn %d%% more exp from enemies\n"
#define RATE_P_PLXP		20
#define CFL_P_ENBS		"  Enemies grant %d%% of exp as stronghold pts\n"
#define RATE_P_ENBS		10
#define CFL_P_ENOS		"  Enemies grant %d%% of exp as Osiris pts\n"
#define RATE_P_ENOS		10
#define CFL_P_ENGL		"  Enemies drop %d%% of exp as gold\n"
#define RATE_P_ENGL		5
#define CFL_P_ARGL		"  Area contains %s piles of gold\n"
#define CFL_P_AREQ		"  Area contains %s discarded equipment\n"
#define CFL_P_ARPT		"  Area contains %s discarded potions\n"
#define CFL_P_DRGM		"  %d enemies drop an additional huge gem\n"
#define CFL_P_RANK		"  Contract rank is increased by %d\n"
// 						"!        .         .   |     .         .        !"
#define CFL_N_EXTY		"  Area has %d additional enemy spawns\n"
#define CFL_N_EXDV		"  Area has %d additional divine enemy spawns\n"
#define CFL_N_ARUW		"  %d%% of area is underwater\n"
#define CFL_N_ENUN		"  Enemies are %d%% undead\n"
#define CFL_N_EXEN		"  Exit is guarded by %d cruel enemies\n"
#define CFL_N_ENRO		"  Enemies roam %d%% farther\n"
#define CFL_N_ENRS		"  Enemies are %d%% more resistant\n"
#define CFL_N_ENSK		"  Enemies are %d%% more skillful\n"
#define CFL_N_ENSH		"  Enemies have %d%% more weapon value\n"
#define CFL_N_ENFO		"  Enemies have %d%% more armor value\n"
#define CFL_N_ENFS		"  Enemies are %d%% faster\n"
#define CFL_N_ENWI		"  Enemies have %d%% more spellmod\n"
#define CFL_N_PLDB		"  Players are %d%% debilitated\n"
#define CFL_N_PLFR		"  Players are %d%% more fragile\n"
#define CFL_N_PLST		"  Players are %d%% stigmatic\n"
#define CFL_N_PLHY		"  Players are %d%% hyperthermic\n"
#define CFL_N_ENTR		"  Enemies use %s tarot cards.\n"
#define CFL_N_ARSP		"  Area contains %s spike traps\n"
#define CFL_N_ARDT		"  Area contains %s hidden dart traps\n"
#define CFL_N_ARFL		"  Area contains %s open flames\n"
// 						"!        .         .   |     .         .        !"

// -------- Special Item Descriptions -------- //

#define FN_TOWER		7
#define DESC_TW_CROWN		"When equipped, 25%% of skill costs are nullified, 25%% of mana costs from spells are taken from endurance, and 25%% of endurance costs from skills are taken from mana. You no longer lose focus.\n"
#define DESC_TW_CLOAK		"When equipped, 10%% of damage taken from hits is dealt to Endurance instead, and 10%% of damage taken from damage over time is dealt to Mana instead.\n"
#define DESC_TW_DREAD		"When equipped, your ghost companion is replaced with a spellcaster companion.\n"
#define DESC_TW_DOUSER		"When equipped, grants a 25%% chance on skill hit and a 5%% chance on melee hit to inflict douse, reducing enemy spell modifier.\n"
#define DESC_TW_MARCH		"When equipped, Stun is turned into a speed reduction of 1.50, and all other speed reductions are reduced by one third.\n"
#define DESC_TW_BBELT		"When equipped, get WV from your Hand to Hand skill and +2%% base crit if your main hand is empty, half of shield parry bonus if your off-hand is empty, and 5%% total base crit if both hands are empty.\n"
#define DESC_TW_OUTSIDE		"When equipped, enemies beside and behind you no longer gain a bonus to hitting you.\n"
#define DESC_TW_HEAVENS		"Uses your highest weapon skill for hit and parry scores. When equipped, your highest attribute score is increased by 20%%.\n"

#define FN_ABYSS		4
#define DESC_TW_ACEDIA		"When equipped in your main-hand, you get 25%% less skill cooldown, but 25%% less buff and debuff duration. In your off-hand, you get 50%% more buff and debuff duration, but 50%% more skill cooldown.\n"
#define DESC_TW_IRA			"When equipped, your critical hit chance and half of critical multiplier effect your Blast spell, but you lose mana over time and cannot deal melee critical hits.\n"
#define DESC_TW_INVIDIA		"When equipped, your ghost companion has surround hit AoE, and your shadow copy teleports to new targets, but each cannot regenerate health.\n"
#define DESC_TW_GULA		"When equipped, you restore health upon hitting, cleaving or blasting an enemy, but you lose hitpoints over time.\n"
#define DESC_TW_LUXURIA		"When equipped, grants a 25%% chance on skill hit and a 5%% chance on melee hit to inflict stun.\n"
#define DESC_TW_AVARITIA	"When equipped, twice your lowest attribute increases critical hit chance and half your highest attribute increases critical hit multiplier, but you lose endurance over time.\n"
#define DESC_TW_SUPERBIA	"When equipped, the effects of your Heal or Regen spell is halved, and your total spell aptitude is reduced by 90%%.\n"
#define DESC_TW_SINBIND		"You may only equip one Sinbinder at a time.\n"

#define FN_O			4
#define DESC_O				"You may only equip one %s at a time.\n", it[in].name

#define FN_SIGNT		7
#define DESC_SIGN_SKUA		"When equipped, all negative values on gear become positive.\n"
#define DESC_SIGN_SHOU		"When equipped, your Warcry shocks enemies, and your Rally charges allies.\n"
#define DESC_SIGN_SLAY		"When equipped, your Leap repeats an additional time. If Leap no longer repeats, it instead gains +1 AoE per two repeats it would have made.\n"
#define DESC_SIGN_STOR		"When equipped, your Blind and Douse also applies a stack of Zephyr every 3 seconds.\n"
#define DESC_SIGN_SICK		"When equipped, your Poisons last twice as long and reduce enemy cooldown rate by 10%%.\n"
#define DESC_SIGN_SHAD		"When equipped, your Shadow Copy can summon Shadow Copies.\n"
#define DESC_SIGN_SPAR		"When equipped, your Blast hits an additional nearby target when its cooldown expires.\n"
#define DESC_SIGN_SYMM		"When equipped, this ring will copy the effects of your other ring slot.\n"
#define DESC_SIGN_SONG		"When equipped, your Aria additionally grants nearby allies 10%% of your Armor Value.\n"
#define DESC_SIGN_SCRE		"When equipped, your Curse also inflicts Aggravate.\n"

#define FN_EX			5
#define DESC_GL_SERPENT		"When equipped, grants a 25%% chance on skill hit and a 5%% chance on melee hit to inflict poison, dealing damage over time.\n"
#define DESC_GL_BURNING		"When equipped, grants a 25%% chance on skill hit and a 5%% chance on melee hit to inflict scorch, causing enemies to take additional damage.\n"
#define DESC_GL_SHADOW		"When equipped, grants a 25%% chance on skill hit and a 5%% chance on melee hit to inflict blind, reducing enemy perception, hit and parry.\n"
#define DESC_GL_CHILLED		"When equipped, grants a 25%% chance on skill hit and a 5%% chance on melee hit to inflict slow, reducing enemy action speed.\n"
#define DESC_GL_CURSED		"When equipped, grants a 25%% chance on skill hit and a 5%% chance on melee hit to inflict curse, reducing enemy attributes.\n"
#define DESC_GL_TITANS		"When equipped, grants a 25%% chance on skill hit and a 5%% chance on melee hit to inflict weaken, reducing enemy weapon value.\n"
#define DESC_GL_BLVIPER		"When equipped, grants a 25%% chance on skill hit and a 5%% chance on melee hit to inflict frostburn, draining enemy endurance and mana.\n"

#define DESC_BT_NATURES		"When equipped, grants 33%% more regeneration rates while idle.\n"
#define DESC_RINGWARMTH		"When equipped and activated, grants additional health, endurance, and mana regeneration.\n"
#define DESC_LIZCROWN		"When equipped, your maximum hitpoints and maximum mana are swapped. Overcap bonuses occur after this swap.\n"

#define DESC_ANKHAMULET		"When equipped, grants additional passive regeneration bonuses.\n"
#define DESC_AMBERANKH		"When equipped, grants additional passive regeneration bonuses. Grants improved life regeneration.\n"
#define DESC_TURQUANKH		"When equipped, grants additional passive regeneration bonuses. Grants improved endurance regeneration.\n"
#define DESC_GARNEANKH		"When equipped, grants additional passive regeneration bonuses. Grants improved mana regeneration.\n"
#define DESC_TRUEANKH		"When equipped, grants very strong passive regeneration bonuses.\n"
#define DESC_AM_BLOODS		"When equipped, penalizes meditate, but greatly improves the effect of regenerate.\n"
#define DESC_AM_VERDANT		"When equipped, penalizes regenerate, but greatly improves the effect of rest.\n"
#define DESC_AM_SEABREZ		"When equipped, penalizes rest, but greatly improves the effect of meditate.\n"
#define DESC_AM_TRUESUN		"When equipped, you are immune to areas that would disable your magic.\n"
#define DESC_AM_FALMOON		"When equipped, you cannot cast or receive spells of any kind.\n"
#define DESC_AM_ECLIPSE		"When equipped, your immunity has 25%% increased effectiveness.\n"
#define DESC_GAMBLERFAL		"When equipped, critical hits award the active stats on this amulet for 5 seconds.\n"

#define DESC_BL_ECLIPSE		"Grants passive bonuses during the day (6:00 to 18:00), and active bonuses during the night (18:00 to 6:00).\n"
#define DESC_WHITEBELT		"When equipped, get WV from your Hand to Hand skill and +2%% base crit if your main hand and off hand are empty.\n"

#define DESC_MISERRING		"When equipped and activated, grants 50%% more gold from enemies you kill.\n"
#define DESC_FORTERING		"When equipped and activated, grants 25%% more EXP from enemies you kill.\n"

#define DESC_IT_PIGS		"Grants 15%% additional maximum hitpoints for its duration.\n"
#define DESC_IT_AALE		"Grants 15%% reduced endurance costs for its duration.\n"
#define DESC_IT_DRAG		"Grants 1 reduced maximum healing sickness stack for its duration.\n"
#define DESC_IT_MANA		"Grants 15%% reduced mana costs for its duration.\n"

#define DESC_BOOK_ALCH		"When equipped, healing items are 50%% more effective.\n"
#define DESC_BOOK_HOLY		"When equipped, maximum healing sickness you can inflict is reduced by 1 stack.\n"
#define DESC_BOOK_ADVA		"When equipped, debuffs you cast are more likely to pass resistance checks.\n"
#define DESC_BOOK_TRAV		"When equipped, secondary effects of Braveness and Agility are equal to the higher of the two.\n"
#define DESC_BOOK_DAMO		"When equipped, increases cooldown recovery speed by 11%%.\n"
#define DESC_BOOK_SHIV		"When equipped, casting Curse will immediately cast Slow as well. These are less likely to pass resistance checks.\n"
#define DESC_BOOK_PROD		"When equipped, improves the mana effectiveness of the Economize skill.\n"
#define DESC_BOOK_VENO		"When equipped, Poisons and Venoms you inflict deal damage 25%% faster.\n"
#define DESC_BOOK_NECR		"When equipped, newly casted Ghost Companions and Shadow Copies become undead monsters.\n"
#define DESC_BOOK_BISH		"When equipped, casting Bless will immediately cast Protect and Enhance as well.\n"
#define DESC_BOOK_GREA		"When equipped, damage dealt to the duration of your Magic Shield and Magic Shell is halved.\n"
#define DESC_BOOK_DEVI		"When equipped, this book can be used to freely cast Shadow Copy. Shadow copies created this way are a thrall of the last enemy you've killed.\n"
#define DESC_BOOK_BURN		"When equipped, your Immolate spell gains 1 additional base power per 20 uncapped hitpoints.\n"
#define DESC_BOOK_VERD		"When equipped, your Dispel spell loses power less quickly and has no removal limit, but no longer immunizes or inoculates.\n"
#define DESC_BOOK_MALT		"When equipped, you no longer lose focus.\n"
#define DESC_BOOK_GRAN		"When equipped, your highest attribute score is increased by 10%%.\n"

#define FN_WO			9
#define DESC_STONEDAGG		"When equipped, this weapon can be used to cast 'Sacrifice', converting 50% of your current Hitpoints into half as much Mana.\n"
#define DESC_LIFESPRIG		"When equipped, mana spent is restored as life for 5 seconds. This effect is overwritten by stronger sources.\n"
#define DESC_SPIDERFANG		"When equipped, this weapon can be used to cast Venom, with power equal to 1/5 of your uncapped mana.\n"
#define DESC_MAGEMASH		"When equipped, this weapon can be used to cast Dispel. Dispel cast in this way does not affect you or your allies, and instead removes enemy buffs.\n"
#define DESC_BLOODLET		"When equipped, this weapon can be used to cast 'Bloodletting', costing 1/3 of uncapped hitpoints for power. Bloodletting inflicts bleeding on surrounding enemies.\n"
#define DESC_GEMCUTTER		"When equipped, stats granted by your ring slots are improved by 25%%.\n"
#define DESC_QUICKSILV		"When equipped, this weapon grants full damage and shield parry while in the offhand.\n"
#define DESC_JANESOBLIT		"When equipped, this weapon can be used to cast 'Obliterate', costing 1/3 of your uncapped mana for power. Obliterate has a base radius of 4, centered at your target's location.\n"
#define DESC_RATTANBO		"When equipped, this weapon can be used to cast a buff version of Zephyr, with power equal to your Staff skill modifier.\n"
#define DESC_PUTRIDIRE		"When equipped, this weapon can be used to freely cast Poison, with power equal to 1/3 of your uncapped mana.\n"
#define DESC_STARLIGHT		"When equipped, this weapon can be used to cast 'Starlight', costing 1/3 of uncapped mana. Starlight grants 1 additional spell modifier per 50 mana spent.\n"
#define DESC_BLOODYSCY		"When equipped, this weapon can be used to freely cast Cleave, with power equal to 1/2 of your uncapped hitpoints.\n"
#define DESC_GOLDGLAIVE		"When equipped, your Ghost Companion will use Warcry instead of Taunt, based on its Taunt skill modifier.\n"
#define DESC_KELPTRID		"When equipped, grants +30 to action speed while underwater.\n"
#define DESC_FLAGBEAR		"When equipped, this weapon can be used to freely cast Rally, with power equal to your uncapped endurance.\n"
#define DESC_FROSTGLASS		"When equipped, this shield can be used to cast Slow, with power equal to your total Shield skill modifier.\n"
#define DESC_PHALANX		"When equipped, this shield can be used to cast 'Phalanx', costing 1/3 of uncapped endurance. Phalanx grants 2%% damage reduction per 5 endurance spent.\n"
#define DESC_RISINGPHO		"When equipped, this shield can be used to freely cast Immolate, with power equal to 1/3 of your uncapped hitpoints, ignoring spell modifier.\n"
#define DESC_THEWALL		"When equipped, your Shield skill becomes Shield Bash. Shield Bash inflicts Stun and deals damage based on your Armor Value.\n"
#define DESC_BARBSWORD		"When equipped, your active melee skills gain an additional 5%% Strength bonus.\n"
#define DESC_LAMEDARG		"Thine worth shall be proven. %d remain.\n", max(0, REQ_LAME-it[in].data[0])
#define DESC_LAMEFULL		"Thou art worthy. Use me when ready.\n"
#define DESC_WHITEODA		"When equipped, grants additional armor value based on total spell modifier.\n"
#define DESC_EXCALIBUR		"When equipped, 20%% of uncapped attack speed is granted as additional weapon value.\n"
#define DESC_BEINESTOC		"When equipped, your Hit and Parry scores are the higher of the two.\n"
#define DESC_BLACKTAC		"When equipped, grants additional weapon value based on total spell modifier.\n"
#define DESC_EVERGREEN		"When equipped, grants 1 additional weapon value per 10 Agility, and 1 additional armor value per 10 Strength.\n"
#define DESC_LIONSPAWS		"When equipped, your passive skills gain an additional 10%% Braveness bonus.\n"
#define DESC_CRIMRIP		"When equipped, this weapon can be used to cast 'Bloodletting', costing 1/3 of uncapped hitpoints. Bloodletting inflicts bleeding on surrounding enemies.\n"
#define DESC_SOVERIGNS		"When equipped, %s and %s become the average of the two, plus 10.\n", skilltab[it[in].data[1]].name, skilltab[it[in].data[2]].name
#define DESC_CRESSUN		"When equipped, endurance spent is restored as life for 5 seconds. This effect is overwritten by stronger sources.\n"
#define DESC_MJOLNIR		"When equipped, this weapon can be used to cast Blast, with power based on your WV and top damage, ignoring spell modifier. Blasts cast this way will inflict a stack of Shock, and grant you a stack of Charge.\n"
#define DESC_LAVA2HND		"Has a 25%% chance on skill hit and a 5%% chance on melee hit to inflict Weaken, reducing enemy weapon value.\n"
#define DESC_GILDSHINE		"When equipped, grants additional critical hit multiplier based off total Economize score.\n"
#define DESC_BURN2HND		"Has a 25%% chance on skill hit and a 5%% chance on melee hit to inflict Scorch, causing enemies to take additional damage.\n"
#define DESC_ICE2HND		"Has a 25%% chance on skill hit and a 5%% chance on melee hit to inflict Slow, reducing enemy action speed.\n"
#define DESC_COLDSTEEL		"When equipped, your active melee skills gain an additional 10%% Agility bonus.\n"
#define DESC_CROSSBLAD		"When equipped, Surround Hit has a base radius of 4.\n"
#define DESC_BRONCHIT		"When equipped, 20%% of Cleave's damage is also dealt to the target's mana.\n"
#define DESC_VOLCANF		"When equipped, you cannot naturally deal critical hits. If your enemy is scorched, the scorch is removed to guarantee a critical hit.\n"
#define DESC_VIKINGMALT		"When equipped, this weapon can be used to cast 'Slam', dealing damage based on Strength around you and stunning enemies with a base radius of 2 around your target. Stun duration with Slam is determined by your crit multiplier.\n"
#define DESC_GUNGNIR		"When equipped, bleeding you inflict deals its damage three times faster.\n"

#define DESC_BONEARMOR		"When equipped, 30%% of damage taken from hits is instead taken as damage over time, over 5 seconds.\n"

// -------- Tarot Cards -------- //

#define FN_TAROT		8
#define DESC_FOOL			"Grants applied soulstone and enchantment while equipped. Soulstones applied to this card behave strangely.\n"
#define DESC_MAGI			"When equipped, secondary effects of Intuition and Strength are equal to the higher of the two.\n"
#define DESC_PREIST			"When equipped, 20%% of damage taken from hits is dealt to Mana instead.\n"
#define DESC_EMPRESS		"When equipped, your Magic Shield spell is replaced with Magic Shell. Magic Shell grants a temporary Resistance and Immunity bonus.\n"
#define DESC_EMPEROR		"When equipped, your Slow spell is replaced with Greater Slow. Greater Slow no longer decays and has an increased duration.\n"
#define DESC_HEIROPH		"When equipped, Immunize and Inoculate from your Dispel spell lasts four times as long, but your Dispel spell can only remove a single buff or debuff at a time.\n"
#define DESC_LOVERS			"When equipped, your Weapon Value and Armor Value become the average of your Weapon Value and Armor Value.\n"
#define DESC_CHARIOT		"When equipped, your Blind skill is replaced with Douse. Douse reduces your target's stealth and spell modifier.\n"
#define DESC_STRENGTH		"When equipped, reduces your attack speed, cast speed, and cooldown recovery speed by 15%%, but grants 20%% more damage with hits.\n"
#define DESC_HERMIT			"When equipped, you have 15%% more Armor Value, but 20%% less Resistance and Immunity.\n"
#define DESC_WHEEL			"When equipped, your critical hit chance is reduced by 33%%, but you have 33%% more critical hit multiplier.\n"
#define DESC_JUSTICE		"When equipped, your Cleave skill no longer inflicts a Bleeding, and instead inflicts Aggravate, causing the target to take additional damage for 20 seconds.\n"
#define DESC_HANGED			"When equipped, 33%% of your Resistance is instead used to reduce the strength of incoming enemy spells.\n"
#define DESC_DEATH			"When equipped, your Weaken skill is replaced with Crush. Crush reduces a target's Armor Value, but no longer reduces enemy Weapon Value.\n"
#define DESC_TEMPER			"When equipped, your Taunt skill grants 100%% more Guard power, but Guard duration is halved.\n"
#define DESC_DEVIL			"When equipped, 33%% of all skill and spell costs are instead taken from your Hitpoints.\n"
#define DESC_TOWER			"When equipped, your Curse spell is replaced with Greater Curse. Greater Curse has increased effect, but decays over time and has a reduced duration.\n"
#define DESC_STAR			"When equipped, your Heal spell is replaced with Regen. Regen grants a buff which regenerates the target's Hitpoints over 20 seconds.\n"
#define DESC_MOON			"When equipped, life regeneration is instead applied as mana regeneration while not at full mana.\n"
#define DESC_SUN			"When equipped, endurance regeneration is instead applied as life regeneration while not at full life.\n"
#define DESC_JUDGE			"When equipped, your Blast spell deals 15%% less damage but inflicts Scorch, causing the target to take additional damage for 20 seconds.\n"
#define DESC_WORLD			"When equipped, mana regeneration is instead applied as endurance regeneration while not at full endurance.\n"

#define DESC_FOOL_R			"When equipped, your attributes become the average of all attributes, plus 8%%.\n"
#define DESC_MAGI_R			"When equipped, your Economize skill no longer reduces the Mana cost of skills, and instead increases your Cooldown rate.\n"
#define DESC_PREIST_R		"When equipped, your Magic Shield and Magic Shell are now passive and regenerate quickly, but are more fragile. You cannot gain Magic Shield or Magic Shell from other sources.\n"
#define DESC_EMPRES_R		"When equipped, your Lethargy skill costs life over time instead of mana over time.\n"
#define DESC_EMPERO_R		"When equipped, your Warcry skill is replaced with Rally. Rally grants nearby allies a buff which improves Hit Score and Parry Score.\n"
#define DESC_HEIROP_R		"When equipped, your Ghost Companion has 12%% more Weapon Value and Armor Value, but has a 20%% chance to miss when it should have hit.\n"
#define DESC_LOVERS_R		"When equipped, your Hit Score and Parry Score become the average of your Hit Score and Parry Score.\n"
#define DESC_CHARIO_R		"When equipped, your debuffs ignore 25%% of target resistance and immunity, but are 20%% weaker once applied.\n"
#define DESC_STRENG_R		"When equipped, you have 20%% more Weapon Value, but 20%% less hit score.\n"
#define DESC_HERMIT_R		"When equipped, your Rage and Calm skills instead cost endurance over time.\n"
#define DESC_WHEEL_R		"When equipped, you take 20%% less damage from melee attacks, but have a 25%% chance to be hit when you would have parried.\n"
#define DESC_JUSTIC_R		"When equipped, your Leap skill no longer repeats. Leap now always deals a critical hit and stuns everything it hits.\n"
#define DESC_HANGED_R		"When equipped, you have 24%% more Top Damage, but 12%% less Weapon Value.\n"
#define DESC_DEATH_R		"When equipped, your Zephyr skill no longer triggers on hit, and instead triggers when an enemy attacks you.\n"
#define DESC_TEMPER_R		"When equipped, you gain 6.25%% more Weapon Value per stack of Healing Sickness on you. The maximum healing sickness you can receive is increased by 1 stack.\n"
#define DESC_DEVIL_R		"When equipped, your Shadow Copy deals 25%% more damage and takes 25%% less damage, but while your Shadow Copy is active you deal 20%% less damage and take 20%% more damage.\n"
#define DESC_TOWER_R		"When equipped, your Poison spell is replaced with Venom. Venom deals half as much damage, but it reduces enemy Immunity and can stack up to three times.\n"
#define DESC_STAR_R			"When equipped, your base Spell Modifier is 0.90. Your Spell Modifier no longer effects spell power and instead effects skill power.\n"
#define DESC_MOON_R			"When equipped, your Tactics skill has 1%% increased effect per 50 uncapped mana, but you lose 0.2%% of current mana per second per 50 uncapped mana.\n"
#define DESC_SUN_R			"When equipped, the effectiveness of your Regenerate, Rest, and Meditate skills behave as if stationary while fighting, but as if fighting while stationary.\n"
#define DESC_JUDGE_R		"When equipped, your Pulse spell no longer deals damage to enemies and instead heals allies with each pulse. It inflicts Charge instead of Shock to allies, granting them additional damage and damage reduction.\n"
#define DESC_WORLD_R		"When equipped, 50%% of damage taken is dealt to Endurance instead. All Endurance costs instead use Mana, and all skills grant Endurance on use. You lose 40%% of current endurance per second, mitigated by your Rest skill.\n"

enum _sk_tree_skills {
	TSK_SEYA_ACCU,	TSK_SEYA_EXPE,	TSK_SEYA_AVOI,	TSK_SEYA_ABSO,	TSK_SEYA_RIGO,	TSK_SEYA_SCOR,
	TSK_SEYA_DETE,	TSK_SEYA_JACK,	TSK_SEYA_INDI,	TSK_SEYA_ENIG,	TSK_SEYA_FLEX,	TSK_SEYA_PENA,
	TSK_ARTM_RAVA,	TSK_ARTM_MIGH,	TSK_ARTM_TOUG,	TSK_ARTM_BULW,	TSK_ARTM_VANQ,	TSK_ARTM_IMPA,
	TSK_ARTM_BARB,	TSK_ARTM_STRE,	TSK_ARTM_OVER,	TSK_ARTM_RAMP,	TSK_ARTM_UNBR,	TSK_ARTM_TEMP,
	TSK_SKAL_DECI,	TSK_SKAL_DEXT,	TSK_SKAL_WALL,	TSK_SKAL_LITH,	TSK_SKAL_BRUT,	TSK_SKAL_CRUS,
	TSK_SKAL_NOCT,	TSK_SKAL_AGIL,	TSK_SKAL_CELE,	TSK_SKAL_GUAR,	TSK_SKAL_SANC,	TSK_SKAL_BAST,
	TSK_WARR_RAPI,	TSK_WARR_RUFF,	TSK_WARR_STAM,	TSK_WARR_DISM,	TSK_WARR_SWIF,	TSK_WARR_FLAS,
	TSK_WARR_SLAY,	TSK_WARR_HARR,	TSK_WARR_ANTA,	TSK_WARR_CHAM,	TSK_WARR_PERS,	TSK_WARR_TENA,
	TSK_SORC_PASS,	TSK_SORC_POTE,	TSK_SORC_QUIC,	TSK_SORC_INTR,	TSK_SORC_ZEAL,	TSK_SORC_REWI,
	TSK_SORC_TOXI,	TSK_SORC_PRAG,	TSK_SORC_HEXM,	TSK_SORC_FAST,	TSK_SORC_FLEE,	TSK_SORC_DODG,
	TSK_SUMM_NIMB,	TSK_SUMM_WISD,	TSK_SUMM_BARR,	TSK_SUMM_TACT,	TSK_SUMM_SPEL,	TSK_SUMM_STRA,
	TSK_SUMM_MYST,	TSK_SUMM_WILL,	TSK_SUMM_SHAP,	TSK_SUMM_DIVI,	TSK_SUMM_CONS,	TSK_SUMM_NECR,
	TSK_ARHR_COMP,	TSK_ARHR_INTE,	TSK_ARHR_WELL,	TSK_ARHR_MALI,	TSK_ARHR_SERE,	TSK_ARHR_DEST,
	TSK_ARHR_PSYC,	TSK_ARHR_INTU,	TSK_ARHR_CONC,	TSK_ARHR_FLOW,	TSK_ARHR_PERP,	TSK_ARHR_RESO,
	TSK_BRAV_MUSC,	TSK_BRAV_BOLD,	TSK_BRAV_MIND,	TSK_BRAV_PERF,	TSK_BRAV_VALO,	TSK_BRAV_PRES,
	TSK_BRAV_VIRT,	TSK_BRAV_BRAV,	TSK_BRAV_ALAC,	TSK_BRAV_SPEL,	TSK_BRAV_WIZA,	TSK_BRAV_RESI,
	TSK_LYCA_EXPA,	TSK_LYCA_FEAS,	TSK_LYCA_SHAR,	TSK_LYCA_SICK,	TSK_LYCA_PRID,	TSK_LYCA_GREE,
	TSK_LYCA_LUST,	TSK_LYCA_GLUT,	TSK_LYCA_WRAT,	TSK_LYCA_SLOT,	TSK_LYCA_ENVY,	TSK_LYCA_SERR,
	
	TSK_CORR_ACCU,	TSK_CORR_EXPE,	TSK_CORR_AVOI,	TSK_CORR_ABSO,	TSK_CORR_RIGO,	TSK_CORR_SCOR,
	TSK_CORR_DETE,	TSK_CORR_MAST,	TSK_CORR_INDI,	TSK_CORR_ENIG,	TSK_CORR_FLEX,	TSK_CORR_PENA,
	TSK_CORR_RAVA,	TSK_CORR_MIGH,	TSK_CORR_TOUG,	TSK_CORR_BULW,	TSK_CORR_VANQ,	TSK_CORR_GOLI,
	TSK_CORR_BARB,	TSK_CORR_FSTR,	TSK_CORR_OVER,	TSK_CORR_FULL,	TSK_CORR_UNBR,	TSK_CORR_TEMP,
	TSK_CORR_DECI,	TSK_CORR_DEXT,	TSK_CORR_WALL,	TSK_CORR_LITH,	TSK_CORR_BRUT,	TSK_CORR_MONK,
	TSK_CORR_NOCT,	TSK_CORR_FAGL,	TSK_CORR_CELE,	TSK_CORR_TOWE,	TSK_CORR_SANC,	TSK_CORR_BAST,
	TSK_CORR_RAPI,	TSK_CORR_RUFF,	TSK_CORR_STAM,	TSK_CORR_DISM,	TSK_CORR_SWIF,	TSK_CORR_AXEM,
	TSK_CORR_SLAY,	TSK_CORR_HARR,	TSK_CORR_ANTA,	TSK_CORR_SEVE,	TSK_CORR_PERS,	TSK_CORR_TENA,
	TSK_CORR_PASS,	TSK_CORR_POTE,	TSK_CORR_QUIC,	TSK_CORR_INTR,	TSK_CORR_ZEAL,	TSK_CORR_ASSA,
	TSK_CORR_TOXI,	TSK_CORR_PRAG,	TSK_CORR_HEXM,	TSK_CORR_SKIP,	TSK_CORR_FLEE,	TSK_CORR_DODG,
	TSK_CORR_NIMB,	TSK_CORR_WISD,	TSK_CORR_BARR,	TSK_CORR_HARP,	TSK_CORR_SPEL,	TSK_CORR_STRA,
	TSK_CORR_MYST,	TSK_CORR_FWIL,	TSK_CORR_SHAP,	TSK_CORR_WRAI,	TSK_CORR_CONS,	TSK_CORR_NECR,
	TSK_CORR_COMP,	TSK_CORR_INTE,	TSK_CORR_WELL,	TSK_CORR_REPU,	TSK_CORR_SERE,	TSK_CORR_WARL,
	TSK_CORR_PSYC,	TSK_CORR_FINT,	TSK_CORR_CONC,	TSK_CORR_FLOW,	TSK_CORR_PERP,	TSK_CORR_RESO,
	TSK_CORR_MUSC,	TSK_CORR_BOLD,	TSK_CORR_MIND,	TSK_CORR_PERF,	TSK_CORR_VALO,	TSK_CORR_MADN,
	TSK_CORR_VIRT,	TSK_CORR_FBRV,	TSK_CORR_ALAC,	TSK_CORR_SWOR,	TSK_CORR_WIZA,	TSK_CORR_RESI,
	TSK_CORR_EXPA,	TSK_CORR_FEAS,	TSK_CORR_SHAR,	TSK_CORR_MART,	TSK_CORR_PRID,	TSK_CORR_GREE,
	TSK_CORR_LUST,	TSK_CORR_GLUT,	TSK_CORR_WRAT,	TSK_CORR_SLOT,	TSK_CORR_ENVY,	TSK_CORR_WILD,
	
	TSK_MAX
};

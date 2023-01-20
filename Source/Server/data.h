/*************************************************************************

   This file is part of 'Mercenaries of Astonia v2'
   Copyright (c) 1997-2001 Daniel Brockhaus (joker@astonia.com)
   All rights reserved.

 **************************************************************************/

// For all .used:
#define USE_EMPTY     ((unsigned char)0)
#define USE_ACTIVE    ((unsigned char)1)
#define USE_NONACTIVE ((unsigned char)2)

/*********/
/* Globs */
/*********/

#define MD_HOUR (TICKS*60*5) // One hour per 5 IRL minutes
#define MD_MIN	(MD_HOUR/60)
#define MD_DAY  (MD_HOUR*24)
#define MD_YEAR 300

// 02162020 - Added for larger client render
#define VISI_SIZE 60

/* CS, 991113: SIZEs in one header */
#define GLOBSIZE (sizeof(struct global))

#define GF_LOOTING    (1<<0)
#define GF_MAYHEM     (1<<1)
#define GF_CLOSEENEMY (1<<2)
#define GF_CAP        (1<<3)
#define GF_SPEEDY     (1<<4)
#define GF_DIRTY      (1<<5)
#define GF_NEWBS      (1<<6)
#define GF_DISCORD    (1<<7)
#define GF_STRONGHOLD (1<<8)

struct global
{
	int mdtime;
	int mdday;
	int mdyear;
	int dlight;

	int players_created;
	int npcs_created;
	int players_died;
	int npcs_died;

	int character_cnt;
	int item_cnt;
	int effect_cnt;

	int expire_cnt;
	int expire_run;

	int gc_cnt;
	int gc_run;

	int lost_cnt;
	int lost_run;

	int reset_char;
	int reset_item;

	int ticker;

	long long total_online_time;
	long long online_per_hour[24];

	int flags;

	long long uptime;
	long long uptime_per_hour[24];

	int awake;
	int body;

	int players_online;
	int queuesize;

	long long recv;
	long long send;

	int transfer_reset_time;
	int load_avg;

	long load;

	int max_online;
	int max_online_per_hour[24];

	char fullmoon;
	char newmoon;

	unsigned long long unique;

	int cap;
	unsigned int top_ps[3];
	unsigned int top_pg[3];
};


/*******/
/* Map */
/*******/

// 32 bits for flag (static)
#define MF_MOVEBLOCK  (1ULL<<0)
#define MF_SIGHTBLOCK (1ULL<<1)
#define MF_INDOORS    (1ULL<<2)
#define MF_UWATER     (1ULL<<3)
#define MF_NOLAG      (1ULL<<4)
#define MF_NOMONST    (1ULL<<5)
#define MF_BANK       (1ULL<<6)
#define MF_TAVERN     (1ULL<<7)
#define MF_NOMAGIC    (1ULL<<8)
#define MF_DEATHTRAP  (1ULL<<9)
#define MF_NOPLAYER   (1ULL<<10)
#define MF_ARENA      (1ULL<<11)

#define MF_NOEXPIRE   (1ULL<<13)
#define MF_NOFIGHT    (1Ull<<14)

// plus 32 bits for dynamic flags
#define MF_GFX_INJURED  (1ULL<<32)
#define MF_GFX_INJURED1 (1ULL<<33)
#define MF_GFX_INJURED2 (1ULL<<34)

#define MF_GFX_TOMB   ((1ULL<<35)|(1ULL<<36)|(1ULL<<37)|(1ULL<<38)|(1ULL<<39))
#define MF_GFX_TOMB1  (1ULL<<35)
#define MF_GFX_DEATH  ((1ULL<<40)|(1ULL<<41)|(1ULL<<42)|(1ULL<<43)|(1ULL<<44))
#define MF_GFX_DEATH1 (1ULL<<40)

#define MF_GFX_EMAGIC  ((1ULL<<45)|(1ULL<<46)|(1ULL<<47))
#define MF_GFX_EMAGIC1 (1ULL<<45)
#define MF_GFX_GMAGIC  ((1ULL<<48)|(1ULL<<49)|(1ULL<<50))
#define MF_GFX_GMAGIC1 (1ULL<<48)
#define MF_GFX_CMAGIC  ((1ULL<<51)|(1ULL<<52)|(1ULL<<53))
#define MF_GFX_CMAGIC1 (1ULL<<51)

#define MF_GFX_CRIT    (1ULL<<54)

/* CS, 991113: SIZEs in one header */
#define MAPSIZE (sizeof(struct map)*MAPX*MAPY)

struct map_old
{
	unsigned short sprite;          // background image
	unsigned short fsprite;         // foreground sprite

	// for fast access to objects & characters
	unsigned short ch, to_ch;
	unsigned short it;

	unsigned short dlight;          // percentage of dlight
	short light;                    // strength of light (objects only, daylight is computed independendly)

	unsigned long long flags;       // s.a.

}
__attribute__ ((packed));

struct map
{
	unsigned short sprite;          // background image
	unsigned short fsprite;         // foreground sprite

	// for fast access to objects & characters
	unsigned int ch, to_ch;
	unsigned int it;

	unsigned short dlight;          // percentage of dlight
	short light;                    // strength of light (objects only, daylight is computed independendly)

	unsigned long long flags;       // s.a.

}
__attribute__ ((packed));

/* CS, 991113: Support for new pathfinder */
extern unsigned int mapmark[MAPX][MAPY];
extern unsigned int mapmarker;

/**************/
/* Characters */
/**************/

#define KIN_MERCENARY   (1u<< 0)
#define KIN_SEYAN_DU    (1u<< 1)
#define KIN_PURPLE      (1u<< 2)
#define KIN_MONSTER     (1u<< 3)
#define KIN_TEMPLAR     (1u<< 4)
#define KIN_ARCHTEMPLAR (1u<< 5)
#define KIN_HARAKIM     (1u<< 6)
#define KIN_CLANKWAI    (1u<< 7)
#define KIN_FEMALE      (1u<< 8)
#define KIN_ARCHHARAKIM (1u<< 9)
#define KIN_WARRIOR     (1u<<10)  // arch-merc, warrior
#define KIN_SORCERER    (1u<<11)  // arch-merc, sorcerer
#define KIN_SKALD		(1u<<12)
#define KIN_SUMMONER    (1u<<13)
#define KIN_BRAVER      (1u<<14)
#define KIN_CLANGORN    (1u<<15)

#define CF_IMMORTAL		(1ull<< 0)  // will not suffer any damage
#define CF_GOD			(1ull<< 1)  // may issue #god commands
#define CF_CREATOR		(1ull<< 2)  // may use #build
#define CF_BUILDMODE	(1ull<< 3)  // does use #build
#define CF_RESPAWN		(1ull<< 4)  // will respawn after death - not for players
#define CF_PLAYER		(1ull<< 5)  // is a player
#define CF_NEWUSER		(1ull<< 6)  // new account created. player may change name
#define CF_NOTELL		(1ull<< 8)  // tell will only work on him if used by a god
#define CF_NOSHOUT		(1ull<< 9)  // shout will only work in him if used by a god
#define CF_MERCHANT		(1ull<<10)  // will sell his inventory if looked at
#define CF_STAFF		(1ull<<11)  // member of the staff
#define CF_NOHPREG		(1ull<<12)  // no hp regeneration
#define CF_NOENDREG		(1ull<<13)  // no end regeneration
#define CF_NOMANAREG	(1ull<<14)  // no mana regeneration
#define CF_INVISIBLE	(1ull<<15)  // character is completely invisible
#define CF_INFRARED		(1ull<<16)  // sees in the dark
#define CF_BODY			(1ull<<17)  // dead body
#define CF_NOSLEEP		(1ull<<18)  // stay awake all the time
#define CF_UNDEAD		(1ull<<19)  // is undead, can be killed with holy water
#define CF_NOMAGIC		(1ull<<20)  // no magic zone
#define CF_STONED		(1ull<<21)  // turned to stone due to lag
#define CF_USURP		(1ull<<22)  // NPC is being played by player
#define CF_IMP			(1ull<<23)  // may impersonate monsters
#define CF_SHUTUP		(1ull<<24)  // player is unable to talk till next day
#define CF_NODESC		(1ull<<25)  // player cannot change his description
#define CF_PROF			(1ull<<26)  // profiler listing
#define CF_SIMPLE		(1ull<<27)  // uses simple animation system (move, turn, 1 attack)
#define CF_KICKED		(1ull<<28)  // player got kicked, may not login again for a certain time
#define CF_NOLIST		(1ull<<29)  // dont list character in top ten
#define CF_NOWHO		(1ull<<30)  // don't list character in #WHO
#define CF_SPELLIGNORE	(1ull<<31)  // ignore spells cast on me
#define CF_CCP			(1ull<<32)  // Computer Controlled Player, does NOT log out and may have some extra logic
#define CF_SAFE			(1ull<<33)  // safety measures for gods
#define CF_NOSTAFF		(1ull<<34)  // #stell will only work if flag off
#define CF_POH			(1ull<<35)  // clan purples of honor
#define CF_POH_LEADER	(1ull<<36)  // clan purples of honor
#define CF_THRALL		(1ull<<37)  // is enthralled NPC
#define CF_LABKEEPER	(1ull<<38)  // is labkeeper
#define CF_ISLOOTING	(1ull<<39)  // is currently looting a grave
#define CF_GOLDEN		(1ull<<40)  // is on "golden list" aka good player
#define CF_BLACK		(1ull<<41)  // is on "black list" aka bad player
#define CF_PASSWD		(1ull<<42)  // has passwd set
#define CF_UPDATE		(1ull<<43)  // client side update needed
#define CF_SAVEME		(1ull<<44)  // save this player to disk
#define CF_GREATERGOD	(1ull<<45)  // greater god
#define CF_GREATERINV	(1ull<<46)  // no one sees me, ever
#define CF_LOCKPICK		(1ull<<47)  // Ability to use lockpicks and evaluate doors
#define CF_SHADOWCOPY	(1ull<<48)  // Special flag for shadow copies so you cannot spell them.
#define CF_SILENCE		(1ull<<49)  // Shuts up NPC greetings so pents isn't as spammy
#define CF_GCTOME		(1ull<<50)  // Ghost Companion (and Shadow Copy) will automatically teleport with the player
#define CF_EXTRAEXP		(1ull<<51)  // NPC gives extra exp! Used for STRONG mobs and grind spots
#define CF_EXTRACRIT	(1ull<<52)  // Flag to determine if non-players can crit
#define CF_APPRAISE		(1ull<<53)  // Can see item value
#define CF_APPR_OFF		(1ull<<54)  // Toggle Appraisal
#define CF_AREA_OFF		(1ull<<55)  // Toggle AoE Skills
#define CF_SENSEOFF		(1ull<<56)  // Shuts up NPC sense-magic messages
#define CF_AUTOLOOT		(1ull<<57)  // Automatic looting
#define CF_BSPOINTS		(1ull<<58)  // Black Stronghold point merchant
#define CF_SYS_OFF		(1ull<<59)  // Turns off all system-related messages
#define CF_OVERRIDE		(1ull<<60)  // Allow overriding friendly spells on self
#define CF_ALW_SPECT	(1ull<<61)  // Allow spectating
#define CF_SENSE		(1ull<<62)  // Sense Magic Flag		(ch[co].flags & CF_SENSE)

#define AT_CAP				300
#define SPEED_CAP 			300
#define C_AT_CAP(cn, n)		(AT_CAP + ch[(cn)].limit_break[(n)][0] + ch[(cn)].limit_break[(n)][1])

#define PROX_MULTI		(AT_CAP/ 2)
#define PROX_CAP		(AT_CAP/10) // 12
#define PROX_HIT		(AT_CAP/25) // 30
#define PROX_POW		(AT_CAP/30) // 36

#define PRXW_RAD		6
#define PRXW_CAP		(AT_CAP/ 4)
#define PRXW_HIT		(AT_CAP/20)
#define PRXW_POW		(AT_CAP/24)

#define PRXB_RAD		4
#define PRXB_CAP		(AT_CAP/ 5)
#define PRXB_HIT		(AT_CAP/15)
#define PRXB_POW		(AT_CAP/18)

#define PRXP_RAD		3

#define PRXA_RAD		5

#define PREC_CAP		(AT_CAP)

// Attribute Definitions
#define AT_BRV 			 0
#define AT_WIL 			 1
#define AT_INT 			 2
#define AT_AGL 			 3
#define AT_STR 			 4

// Skill Definitions    //
#define SK_HAND			 0
#define SK_PRECISION	 1
#define SK_DAGGER		 2
#define SK_SWORD		 3
#define SK_AXE			 4
#define SK_STAFF		 5
#define SK_TWOHAND		 6
#define SK_ZEPHYR		 7
#define SK_STEALTH		 8
#define SK_PERCEPT		 9
//////////////////////////
#define SK_SWIM			10
#define SK_MSHIELD		11
#define SK_BARTER		12
#define SK_REPAIR		13
#define SK_RAGE			14
#define SK_LETHARGY		15
#define SK_SHIELD		16
#define SK_PROTECT		17
#define SK_ENHANCE		18
#define SK_SLOW			19
//////////////////////////
#define SK_CURSE		20
#define SK_BLESS		21
#define SK_IDENT		22
#define SK_RESIST		23
#define SK_BLAST		24
#define SK_DISPEL		25
#define SK_HEAL			26
#define SK_GHOST		27
#define SK_REGEN		28
#define SK_REST			29
//////////////////////////
#define SK_MEDIT		30
#define SK_ARIA			31
#define SK_IMMUN		32
#define SK_SURROUND		33
#define SK_CONCEN		34
#define SK_WARCRY		35
#define SK_DUAL			36
#define SK_BLIND		37
#define SK_GEARMAST		38
#define SK_SAFEGRD		39
//////////////////////////
#define SK_CLEAVE		40
#define SK_WEAKEN		41
#define SK_POISON		42
#define SK_PULSE		43
#define SK_PROX			44
#define SK_GCMASTERY	45
#define SK_SHADOW		46
#define SK_HASTE		47
#define SK_TAUNT		48
#define SK_LEAP			49
//////////////////////////
#define SK_LIGHT		50
#define SK_RECALL		51
//////////////////////////
// Defines for Ailments // - These are OK to match existing skill numbers; see splog[] in skill_driver.c
#define SK_EXHAUST  	 1
#define SK_BLEED		 2
#define SK_WEAKEN2		 3
#define SK_SCORCH		 4
#define SK_CURSE2		 5 
#define SK_SLOW2		 6
#define SK_ZEPHYR2		 8
#define SK_DOUSE		10
#define SK_MSHELL		12
#define SK_GUARD		16
#define SK_VENOM		29
#define SK_WARCRY3  	30
#define SK_DISPEL2		32
#define SK_WARCRY2  	36
#define SK_AGGRAVATE	38
#define SK_ARIA2		39
//////////////////////////
#define SK_BLOODLET		 9
#define SK_POME			 9
#define SK_STARLIGHT	23
#define SK_PHALANX		34
#define SK_SOL			33
#define SK_IMMOLATE		44
#define SK_IMMOLATE2	45
#define SK_FROSTB		52
#define SK_SLOW3		53
#define SK_OPPRESSION	54
#define SK_OPPRESSED	55


/*
	*** ch.data[] definitions ***
	(this list is growing very slowly;
	see definitions at the beginning of driver.c for NPCs and
	README for player characters.
*/

// Generic CHD values
#define CHD_GROUP			42
#define CHD_MASTER			63
#define CHD_CORPSEOWNER		66

// Player-only values (PCD_)
#define PCD_AFK				 0
#define PCD_MINGROUP		 1
#define PCD_MAXGROUP		 9
#define PCD_FIGHTBACK		11
#define PCD_COMPANION		64
#define PCD_ALLOW			65
#define PCD_RIDDLER			67
#define PCD_ATTACKTIME		68
#define PCD_ATTACKVICT		69
#define PCD_SHADOWCOPY		95

// Monster-only values (MCD_)
#define MCD_DESTTIMER		64
#define MCD_TALKATIVE		71
#define MCD_ENEMY1ST		80
#define MCD_ENEMYZZZ		91

//
#define RANKS 25

/* level differences permitted for attack / group */
#define ATTACK_RANGE 3
#define GROUP_RANGE  3

/* CS, 991113: SIZEs in one header */
#define CHARSIZE  (sizeof(struct character)*MAXCHARS)
#define TCHARSIZE (sizeof(struct character)*MAXTCHARS)

struct character
{
	unsigned char used;             // 1
	// general

	char name[40];                  // 41
	char reference[40];             // 81
	char description[LENDESC];      // 281

	int kindred;                    // 285

	int player;                     // 289
	unsigned int pass1, pass2;      // 297

	unsigned short sprite;          // 299, sprite base value, 1024 dist
	unsigned short sound;           // 301, sound base value, 64 dist

	unsigned long long flags;       // 309

	short int alignment;            // 311

	unsigned short temple_x;        // 313, position of temple for recall and dying
	unsigned short temple_y;        // 315

	unsigned short tavern_x;        // 317, position of last temple for re-login
	unsigned short tavern_y;        // 319

	unsigned short temp;            // 321, created from template n

	// character stats
	// [0]=bare value, 0=unknown
	// [1]=preset modifier, is race/npc dependend
	// [2]=race specific maximum
	// [3]=race specific difficulty to raise (0=not raisable, 1=easy ... 10=hard)
	// [4]=dynamic modifier, depends on equipment and spells (this one is currently not used)
	// [5]=total value

	unsigned char attrib[5][6];     // 351

	unsigned short hp[6];           // 363
	unsigned short end[6];          // 375
	unsigned short mana[6];         // 387

	unsigned char skill[50][6];     // 687

	unsigned char weapon_bonus;
	unsigned char armor_bonus;

	// temporary attributes
	int a_hp;
	int a_end;
	int a_mana;

	unsigned char light;    // strength of lightsource
	unsigned char mode;     // 0 = slow, 1 = medium, 2 = fast
	short int speed;

	int points;
	int points_tot;

	// summary of weapons + armor
	short int armor;
	short int weapon;

	// map stuff
	short int x, y;          // current position x,y NOTE: x=-1, y=-1 = void
	short int tox, toy;      // target coordinated, where the char will be next turn
	short int frx, fry;      // where the char was last turn
	short int status;       // what the character is doing, animation-wise
	short int status2;      // for plr_misc(): what is misc?
	unsigned char dir;      // direction character is facing

	// posessions
	int gold;

	// items carried
	unsigned int item[40];

	// items worn
	unsigned int worn[20];

	// spells active on character
	unsigned short spell[MAXBUFFS];

	// item currently in hand (mouse cursor)
	unsigned int citem;

	time_t creation_date;
	time_t login_date;

	unsigned int addr;

	// misc
	unsigned int current_online_time;
	unsigned int total_online_time;
	unsigned int comp_volume;
	unsigned int raw_volume;
	unsigned int idle;

	// generic driver data
	unsigned short attack_cn;       // target for attacks, will attack if set (prio 4)
	unsigned short skill_nr;        // skill to use/spell to cast, will cast if set (prio 2)
	unsigned short skill_target1;   // target for skills/spells
	unsigned short skill_target2;   // target for skills/spells
	unsigned short goto_x;          // will goto x,y if set (prio 3)
	unsigned short goto_y;
	unsigned short use_nr;          // will use worn item nr if set (prio 1)

	unsigned short misc_action;     // drop, pickup, use, whatever (prio 5)
	unsigned short misc_target1;    // item for misc_action
	unsigned short misc_target2;    // location for misc_action

	unsigned short cerrno;          // error/success indicator for last action (svr_act level)

	unsigned short escape_timer;    // can try again to escape in X ticks
	unsigned short enemy[4];        // currently being fought against by these
	unsigned short current_enemy;   // currently fighting against X

	unsigned short retry;           // retry current action X times

	unsigned short stunned;         // is stunned for X ticks

	// misc stuff added later:
	char speed_mod;                 // race dependent speed modification
	char last_action;               // last action was success/failure (driver_generic level)
	char unused;
	char depot_sold;                // items from depot where sold to pay for the rent

	unsigned char gethit_dam;       // damage for attacker when hitting this char
	char gethit_bonus;              // race specific bonus for above

	unsigned char light_bonus;      // char emits light all the time

	char passwd[16];

	char lastattack;                // neater display: remembers the last attack animation
	
	char move_speed;		// Bonus to movement speed (added to speed_mod)
	char atk_speed;		// Bonus to attacking speed (added to speed_mod)
	char cast_speed;		// Bonus to casting speed (added to speed_mod)
	
	unsigned short spell_mod;       // race dependent spell modifier - two digit decimal accuracy (1.00)
	unsigned short spell_apt;       // aptitude for receiving spells
	unsigned short cool_bonus;      // exhaust cooldown rate
	
	unsigned short crit_chance;     // chance to deal a critical hit (100 = 1%)
	unsigned short crit_multi;		// multiplier for critical hits
	
	unsigned short to_hit;       	// final chance to hit
	unsigned short to_parry;       	// final chance to parry
	unsigned short top_damage;		// Maximum damage score (normally handled by STR/2)
	
	unsigned short taunted;         // has been taunted by this template
	
	int bs_points;					// Black Stronghold points

	short int sprite_override;

	unsigned int alt_worn[12];		// Alternative gear set in storage for swapping
	
	int os_points;					// Osiris points (for maps?)
	
	char gcm;						// Stores current GC 'mode'
	
	char aoe_bonus;					// Total AoE bonus from gear
	
	char colosseum;					// Colosseum mode check, resets every 1st
	char spellfail;					// check for 'Suppression' from NPCs
	
	unsigned short dmg_bonus;		// Damage multiplier. Factor of 1:100, where 10000 is the median 100% dealt value.
	unsigned short dmg_reduction;	// Damage reduction.  Factor of 1:100, where 10000 is the median 100% taken value.
	
	char limit_break[6][2];			// [0]-[4] for BWAIS, [5] for all skills; [0] is base, [1] for effects from items or (de)buffs
	unsigned char pandium_floor[3];	// [0] = Solo  [1] = Group  [2] = reward tier obtained
	
	unsigned short mana_cost;		// Mana cost multiplier.  Factor of 1:100, where 10000 is the median 100%.
	
	unsigned short tree_points;		// Points allocated (upper 12 bits) and available (lower 4 bits)
	
	char future2[19];				// space for future expansion

	unsigned int depot[62];

	int depot_cost;

	int luck;

	int unreach, unreachx, unreachy;

	int class;                      // monster class
	
	char item_lock[40];				// Used for locking items on the client so /sort ignores them
	
	int waypoints;					// known waypoint flags
	
	int tokens;						// for gambling

	time_t logout_date;

	// driver data
	int data[100];
	char text[10][160];
}
__attribute__ ((packed));

/* CS, 991113: SIZEs in one header */
#define NEWCHARSIZE  (sizeof(struct newcharacter)*MAXCHARS)
#define NEWTCHARSIZE (sizeof(struct newcharacter)*MAXTCHARS)

struct newcharacter
{
	unsigned char used;					//           1
	
	// general
	char name[40];						// +40,     41
	char reference[40];					// +40,     81
	char description[LENDESC];			// +200,   281
	int kindred;						// +4,     285
	int player;							// +4,     289
	unsigned int pass1, pass2;			// +4*2,   297
	unsigned short sprite;				// +2,     299, sprite base value, 1024 dist
	unsigned short sound;				// +2,     301, sound base value, 64 dist
	unsigned long long flags;			// +8,     309
	short int alignment;				// +2,     311
	unsigned short temple_x, temple_y;	// +2*2,   315, position of temple for recall and dying
	unsigned short tavern_x, tavern_y;	// +2*2,   319, position of last temple for re-login
	unsigned short temp;				// +2,     321, created from template n

	// character stats
	// [0]=bare value, 0=unknown
	// [1]=preset modifier, is race/npc dependend
	// [2]=race specific maximum
	// [3]=race specific difficulty to raise (0=not raisable, 1=easy ... 10=hard)
	// [4]=dynamic modifier, depends on equipment and spells (this one is currently not used)
	// [5]=total value

	unsigned char attrib[5][6];			// +5*6,   351
	unsigned short hp[6];				// +2*6,   363
	unsigned short end[6];				// +2*6,   375
	unsigned short mana[6];				// +2*6,   387
	unsigned char skill[50][6];			// +50*6,  687
	unsigned char weapon_bonus;			// +1,     688
	unsigned char armor_bonus;			// +1,     689
	int a_hp, a_end, a_mana;			// +4*3,   701, temporary hp, endurance, mana
	unsigned char light;				// +1,     702, strength of lightsource
	unsigned char mode;					// +1,     703, 0 = slow, 1 = medium, 2 = fast
	short int speed;					// +4,     707, character base action speed
	int points;							// +4,     711, experience points unspent
	int points_tot;						// +4,     715, experience points total
	short int armor, weapon;			// +4*2,   723, summary of weapons + armor

	// map stuff
	short int x, y;						// +4*2,   731, current position x,y NOTE: x=-1, y=-1 = void
	short int tox, toy;					// +4*2,   739, target coordinated, where the char will be next turn
	short int frx, fry;					// +4*2,   747, where the char was last turn
	short int status;					// +4*2,   755, what the character is doing, animation-wise
	short int status2;					// +4*2,   763, for plr_misc(): what is misc?
	unsigned char dir;					// +1,     764, direction character is facing

	// posessions
	int gold;							// +4,     768, carried gold
	unsigned int item[60];				// +4*60, 1008, items carried
	unsigned int worn[20];				// +4*20, 1088, items worn
	unsigned short spell[MAXBUFFS];		// +2*40, 1168, active buffs/debuffs
	unsigned int citem;					// +4,    1172, item currently in hand (mouse cursor)

	time_t creation_date;				// +8,    1180
	time_t login_date;					// +8,    1188

	unsigned int addr;					// +4,    1192

	// misc
	unsigned int current_online_time;	// +4,    1196
	unsigned int total_online_time;		// +4,    1200
	unsigned int comp_volume;			// +4,    1204
	unsigned int raw_volume;			// +4,    1208
	unsigned int idle;					// +4,    1212

	// generic driver data
	unsigned short attack_cn;			// +2,    1214, target for attacks, will attack if set (prio 4)
	unsigned short skill_nr;			// +2,    1216, skill to use/spell to cast, will cast if set (prio 2)
	unsigned short skill_target1;		// +2,    1218, target for skills/spells
	unsigned short skill_target2;		// +2,    1220, target for skills/spells
	unsigned short goto_x, goto_y;		// +2*2,  1224, will goto x,y if set (prio 3)
	unsigned short use_nr;				// +2,    1226, will use worn item nr if set (prio 1)

	unsigned short misc_action;			// +2,    1228, drop, pickup, use, whatever (prio 5)
	unsigned short misc_target1;		// +2,    1230, item for misc_action
	unsigned short misc_target2;		// +2,    1232, location for misc_action

	unsigned short cerrno;				// +2,    1234, error/success indicator for last action (svr_act level)

	unsigned short escape_timer;		// +2,    1236, can try again to escape in X ticks
	unsigned short enemy[4];			// +2*4,  1244, currently being fought against by these
	unsigned short current_enemy;		// +2,    1246, currently fighting against X

	unsigned short retry;				// +2,    1248, retry current action X times

	unsigned short stunned;				// +2,    1250, is stunned for X ticks

	// misc stuff added later:
	char speed_mod;						// +1,    1251, race dependent speed modification
	char last_action;					// +1,    1252, last action was success/failure (driver_generic level)

	unsigned char gethit_dam;			// +1,    1253, damage for attacker when hitting this char
	char gethit_bonus;					// +1,    1254, race specific bonus for above

	unsigned char light_bonus;			// +1,    1255, char emits light all the time

	char passwd[16];					// +16,   1271

	char lastattack;					// +1,    1272, neater display: remembers the last attack animation
	
	char move_speed;					// +1,    1273, Bonus to movement speed (added to speed_mod)
	char atk_speed;						// +1,    1274, Bonus to attacking speed (added to speed_mod)
	char cast_speed;					// +1,    1275, Bonus to casting speed (added to speed_mod)
	
	unsigned short spell_mod;			// +2,    1277, race dependent spell modifier - two digit decimal accuracy (1.00)
	unsigned short spell_apt;			// +2,    1279, aptitude for receiving spells
	unsigned short cool_bonus;			// +2,    1281, exhaust cooldown rate
	
	unsigned short crit_chance;			// +2,    1283, chance to deal a critical hit (100 = 1%)
	unsigned short crit_multi;			// +2,    1285, multiplier for critical hits
	
	unsigned short to_hit;				// +2,    1287, final chance to hit
	unsigned short to_parry;			// +2,    1289, final chance to parry
	unsigned short top_damage;			// +2,    1291, Maximum damage score (normally handled by STR/2)
	
	unsigned short taunted;				// +2,    1293, has been taunted by this template
	
	int bs_points;						// +4,    1297, Black Stronghold points

	short int sprite_override;			// +4,    1301

	unsigned int alt_worn[12];			// +4*12, 1349, Alternative gear set in storage for swapping
	unsigned int alt_worn2[12];			// +4*12, 1397, Second alternative gear set
	
	int os_points;						// +4,    1401, Osiris points (for maps?)
	
	char gcm;							// +1,    1402, Stores current GC 'mode'
	
	char aoe_bonus;						// +1,    1403, Total AoE bonus from gear
	
	char colosseum;						// +1,    1404, Colosseum mode check, resets every 1st
	char spellfail;						// +1,    1405, check for 'Suppression' from NPCs
	
	unsigned short dmg_bonus;			// +2,    1407, Damage multiplier.    Factor of 1:100, where 10000 is the median 100% dealt value
	unsigned short dmg_reduction;		// +2,    1409, Damage reduction.     Factor of 1:100, where 10000 is the median 100% taken value
	unsigned short mana_cost;			// +2,    1411, Mana cost multiplier. Factor of 1:100, where 10000 is the median 100%
	
	char limit_break[6][2];				// +6*2,  1423, [0]-[4] for BWAIS, [5] for all skills; [0] is base, [1] for effects from items or (de)buffs
	unsigned char pandium_floor[3];		// +3,    1426, [0] = Solo  [1] = Group  [2] = reward tier obtained
	
	int waypoints;						// +4,    1430, unlocked waypoint flags
	int tokens;							// +4,    1434, tokens for casino
	
	unsigned int depot[62];				// +4*62, 1682, character storage
	
	char future[290];					// +388,  2068, space for future expansion

	int luck;							// +4,    2072, character luck

	int unreach, unreachx, unreachy;	// +4*3,  2084, ???

	int class;							// +4,    2088, monster class id number

	time_t logout_date;					// +8,    2096

	int data[100];						// +4*100, 2496, driver data
	char text[10][160];					// +10*160, 4096, npc text data
}
__attribute__ ((packed));

/*********/
/* Items */
/*********/

#define IF_MOVEBLOCK     (1ull<<0)
#define IF_SIGHTBLOCK    (1ull<<1)
#define IF_TAKE          (1ull<<2)
#define IF_MONEY         (1ull<<3)
#define IF_LOOK          (1ull<<4)
#define IF_LOOKSPECIAL   (1ull<<5)
#define IF_SPELL         (1ull<<6)
#define IF_NOREPAIR      (1ull<<7)
#define IF_ARMOR         (1ull<<8)      // is a piece of armor
#define IF_USE           (1ull<<9)
#define IF_USESPECIAL    (1ull<<10)
#define IF_SINGLEAGE     (1ull<<11)     // don't use age[1] even if it is active
#define IF_SHOPDESTROY   (1ull<<12)
#define IF_UPDATE        (1ull<<13)
#define IF_ALWAYSEXP1    (1ull<<14)     // expire even if not laying in the open and when non-active
#define IF_ALWAYSEXP2    (1ull<<15)     // expire ... when active
#define IF_WP_SWORD      (1ull<<16)     // is a weapon - sword
#define IF_WP_DAGGER     (1ull<<17)     // is a weapon - dagger
#define IF_WP_AXE        (1ull<<18)     // is a weapon - axe
#define IF_WP_STAFF      (1ull<<19)     // is a weapon - staff
#define IF_WP_TWOHAND    (1ull<<20)     // is a weapon - two-handed sword
#define IF_USEDESTROY    (1ull<<21)     // using it destroys the object
#define IF_USEACTIVATE   (1ull<<22)     // may be turned on (activated)
#define IF_USEDEACTIVATE (1ull<<23)     // may be turned off (deactivated)
#define IF_MAGIC         (1ull<<24)     // is magical
#define IF_MISC          (1ull<<25)     // is neither weapon nor armor nor magical
#define IF_REACTIVATE    (1ull<<26)     // reactive item whenever it expires
#define IF_PERMSPELL     (1ull<<27)     // permanent spell (may take mana to keep up)
#define IF_UNIQUE        (1ull<<28)     // unique item
#define IF_DONATE        (1ull<<29)     // auto-donate this item
#define IF_LABYDESTROY   (1ull<<30)     // destroy when leaving labyrinth
#define IF_NOMARKET      (1ull<<31)     // dont change the price for this item
#define IF_HIDDEN        (1ull<<32)     // hard to see, uses data[9] for difficulty
#define IF_STEPACTION    (1ull<<33)     // special routine to call when stepped on
#define IF_NODEPOT       (1ull<<34)     // not storable in depot
#define IF_LIGHTAGE      (1ull<<35)     // ages faster when exposed to light
#define IF_EXPIREPROC    (1ull<<36)     // special procedure for expire
#define IF_IDENTIFIED    (1ull<<37)     // item has been identified
#define IF_NOEXPIRE      (1ull<<38)     // dont expire item
#define IF_SOULSTONE     (1ull<<39)     // item was enhanced by a soulstone
#define IF_OF_DUALSW     (1ull<<40)     // off-handed item is a dual sword
#define IF_OF_SHIELD     (1ull<<41)     // off-handed item is a shield
#define IF_KWAI_UNI      (1ull<<42)     // unique check for Kwai 
#define IF_GORN_UNI      (1ull<<43)     // unique check for Gorn 
#define IF_WP_CLAW       (1ull<<44)     // is a weapon - claw/unarmed
#define IF_BOOK          (1ull<<45)     // is a book
#define IF_JEWELERY      (1ull<<46)     // is an accessory
#define IF_ITEMLOCK      (1ull<<47)     // locked by client - ignore in sort
#define IF_STACKABLE     (1ull<<48)     // item can be stacked
#define IF_PURP_UNI      (1ull<<49)     // unique check for Purple 
#define IF_GEMSTONE      (1ull<<50)     // is a gem
#define IF_IS_KEY        (1ull<<51)     // is a key and decays on rugs
#define IF_AUGMENTED     (1ull<<52)     // item was enhanced by a shrine
#define IF_DIMINISHED    (1ull<<53)     // item was enhanced by a shrine
#define IF_EASEUSE       (1ull<<54)     // item was enhanced by a shrine
#define IF_SOULSPLIT     (1ull<<55)     // item was enhanced by a shrine
#define IF_LEGACY        (1ull<<56)     // item was enhanced by a shrine
#define IF_DUPLICATED    (1ull<<57)     // item was enhanced by a shrine
#define IF_CAN_SS		 (1ull<<58)		// item CAN be soulstoned
#define IF_CAN_EN		 (1ull<<59)		// item CAN be enchanted (talisman)
#define IF_ENCHANTED	 (1ull<<60)		// item has been enchanted

#define IF_WEAPON        (IF_WP_SWORD|IF_WP_DAGGER|IF_WP_AXE|IF_WP_STAFF|IF_WP_TWOHAND|IF_OF_DUALSW|IF_WP_CLAW)
#define IF_ARMORS	     (IF_ARMOR|IF_OF_SHIELD)
#define IF_SELLABLE      (IF_WEAPON|IF_MISC|IF_MAGIC|IF_ARMORS|IF_BOOK|IF_JEWELERY|IF_GEMSTONE)

#define BUFFSIZE         (sizeof(struct item)*MAXBUFF)
#define ITEMSIZE         (sizeof(struct item)*MAXITEM)
#define TITEMSIZE        (sizeof(struct item)*MAXTITEM)

#define EXP_RATE 16
#define EXP_TIME (MAPY/EXP_RATE)

struct item
{
	unsigned char used;             // 1
	char name[40];                  // 41
	char reference[40];             // 81, a pair of boots
	char description[200];          // 281, A pair of studded leather boots.
	
	unsigned long long flags;       // 289, s.a.
	
	unsigned int value;             // 293, value to a merchant
	unsigned short placement;       // 295, see constants above
	
	unsigned short temp;            // 297, created from template temp
	
	unsigned char damage_state;     // 298, has reached damage level X of 5, 0=OK, 4=almost destroyed, 5=destroyed
	
	// states for non-active [0] and active[1]
	unsigned int max_age[2];        // 306, maximum age per state
	unsigned int current_age[2];    // 314, current age in current state
	
	unsigned int max_damage;        // 318, maximum damage per state
	unsigned int current_damage;    // 322, current damage in current state
	
	// modifiers - modifiers apply only when the item is being
	// worn (wearable objects) or when spell is cast. After duration expires,
	// the effects are removed.
	
	// modifiers - modifier [0] applies when the item is being
	// worn (wearable objects) or is added to the powers (spells) for permanent spells
	// modifier [1] applies when it is active
	// modifier [2] is not a modifier but the minimum value that attibute/skill must have to wear or use
	// the item
	
	char attrib[5][3];              // 337
	
	short hp[3];                    // 343
	short end[3];                   // 349
	short mana[3];                  // 355
	
	char skill[50][3];              // 505
	
	char armor[2];                  // 506
	char weapon[2];                 // 507
	
	short light[2];                 // 511
	
	unsigned int duration;          // 515
	unsigned int cost;              // 519
	unsigned int power;             // 523
	unsigned int active;            // 527
	
	// map stuff
	unsigned short int x, y;		// 531, current position        NOTE: x=0, y=0 = void
	unsigned short carried;         // 533, carried by character carried
	unsigned short sprite_override; // 535, used for potions/spells which change the character sprite
	
	short int sprite[2];            // 543
	unsigned char status[2];        // 545
	
	char gethit_dam[2];             // 547, damage for hitting this item
	
	char min_rank;                  // minimum rank to wear the item
	
	char aoe_bonus[2];				// Area of Effect bonus
	
	char move_speed[2];				// movement speed bonus
	char atk_speed[2];				// attack speed bonus
	char cast_speed[2];				// Cast speed bonus
	
	char spell_mod[2];				// spell mod bonus
	char spell_apt[2];				// spell aptitude bonus
	char cool_bonus[2];				// cooldown bonus
	
	char crit_chance[2];			// Crit chance bonus
	char crit_multi[2];				// Crit multiplier bonus
	
	char to_hit[2];					// Hit bonus
	char to_parry[2];				// Parry bonus
	
	char top_damage[2];				// Maximum damage bonus for hits
	
	unsigned char stack;			// Item stacking
	
	unsigned short orig_temp;       // Original template value, for restoration by repair
	
	char ss_armor;					// Used to store AV on SS gear so it repairs correctly
	char ss_weapon;					// Used to store WV on SS gear so it repairs correctly
	
	unsigned char base_crit;		// base crit chance for weapons
	
	short int speed[2];				// base action speed modifier (for haste and slow)
	
	char enchantment;				// Special effect via talismans
	
	char dmg_bonus[2];				// Damage multiplier bonus - Each 1 point is 0.5% bonus.     At 100 points it is a 50% bonus.
	char dmg_reduction[2];			// Damage reduction bonus  - Each 1 point is 0.5% reduction. At 100 points it is a 50% reduction.
	
	int t_bought;                   // 591
	int t_sold;                     // 595
	
	unsigned char driver;           // 596, special routines for LOOKSPECIAL and USESPECIAL
	unsigned int data[10];          // 634, driver data

}
__attribute__ ((packed));

/***********/
/* Effects */
/***********/

#define EF_MOVEBLOCK  1
#define EF_SIGHTBLOCK 2

#define FX_INJURED 1

/* CS, 991113: SIZEs in one header */
#define EFFECTSIZE (sizeof(struct effect)*MAXEFFECT)

struct effect
{
	unsigned char used;
	unsigned char flags;

	unsigned char type;             // what type of effect (FX_)

	unsigned int duration;          // time effect will stay

	unsigned int data[10];          // some data
}
__attribute__ ((packed));

struct s_skilltab
{
	int nr;
	char sortkey;
	
	char name[40];
	char desc[200];
	
	char alt_a[200]; // If something changes the name
	char alt_b[200]; // If something changes the desc
	char alt_c[200]; // Extra slot for proximity desc

	int attrib[3];
};

struct see_map
{
	int x, y;
	char vis[VISI_SIZE * VISI_SIZE]; // 02162020 - updated from 40x40 to 60x60 for larger client render
};

struct s_splog
{
	int nr;
	
	char name[40];		// log name, 		ie. "Curse"
	char ref[40];		// log reference, 	ie. "curse"
	char act[40];		// log action, 		ie. "cursing"
	
	char self[100];		// log self,		ie. "You have been cursed."
	char other[100];	// log other,		ie. " was cursed."
	char sense[100];	// log sense,		ie. " cast curse on you."
	
	char selfaoe[100];	// log self aoe,	ie. "You unleash a powerful mass-curse."
	char otheraoe[100]; // log other aoe,   ie. "You feel a wiked power emanate from somewhere."
	char senseaoe[100];	// log sense aoe,	ie. " tried to include you in a mass-curse but failed."
};

#define MAX_MAPED_QUEUE         16000
#define MAPED_QUEUE_SIZE        (sizeof(struct mapedit_queue)*MAX_MAPED_QUEUE)

#define MAPED_PLACEITEM         0
#define MAPED_RMVITEM           1
#define MAPED_SETFLOOR          2

struct mapedit_queue
{
        unsigned char used;
        int x, y;
        char op_type;   // 0-place item, 1-remove item, 2-change floor
        int it_temp;
};

struct waypoint
{
	int x, y;
	char *name;
	char *desc;
};

struct sk_tree
{
	char *name;
	char *desc;
};

#define N_SOULBONUS		 50
#define N_SOULMAX		 24
#define N_SOULFACTOR	  2

#define MM_M_DFAE		 1
#define MM_M_DADE		 2
#define MM_M_DACE		 3
#define MM_M_FTAR		 4
#define MM_M_FASH		 5
#define MM_M_FACH		 6
#define MM_M_TAPN		 7
#define MM_M_DFTC		 8
#define MM_M_DFTU		 9
#define MM_M_FTEX		10

#define MM_P_CHST		 0
#define MM_P_SHRN		 1
#define MM_P_XEXP		 2
#define MM_P_XLUK		 3
#define MM_P_XBSP		 4
#define MM_P_XOSP		 5
#define MM_P_PLXP		 6
#define MM_P_ENBS		 7
#define MM_P_ENOS		 8
#define MM_P_ENGL		 9
#define MM_P_ARGL		10
#define MM_P_AREQ		11
#define MM_P_ARPT		12
#define MM_P_DRGM		13
#define MM_P_RANK		14
#define NUM_MAP_POS		15

#define MM_N_EXTY		 0 + NUM_MAP_POS
#define MM_N_EXDV		 1 + NUM_MAP_POS
#define MM_N_ARUW		 2 + NUM_MAP_POS
#define MM_N_ENUN		 3 + NUM_MAP_POS
#define MM_N_EXEN		 4 + NUM_MAP_POS
#define MM_N_ENRO		 5 + NUM_MAP_POS
#define MM_N_ENRS		 6 + NUM_MAP_POS
#define MM_N_ENSK		 7 + NUM_MAP_POS
#define MM_N_ENSH		 8 + NUM_MAP_POS
#define MM_N_ENFO		 9 + NUM_MAP_POS
#define MM_N_ENFS		10 + NUM_MAP_POS
#define MM_N_ENWI		11 + NUM_MAP_POS
#define MM_N_PLDB		12 + NUM_MAP_POS
#define MM_N_PLFR		13 + NUM_MAP_POS
#define MM_N_PLST		14 + NUM_MAP_POS
#define MM_N_PLHY		15 + NUM_MAP_POS
#define MM_N_ENTR		16 + NUM_MAP_POS
#define MM_N_ARSP		17 + NUM_MAP_POS
#define MM_N_ARDT		18 + NUM_MAP_POS
#define MM_N_ARFL		19 + NUM_MAP_POS
#define NUM_MAP_NEG		20

extern struct s_skilltab skilltab[MAXSKILL+2];
extern struct s_splog splog[52];
extern struct global *globs;
extern struct map *map;
extern struct character *ch;
extern struct item *bu;
extern struct item *it;
extern struct character *ch_temp;
extern struct item *it_temp;
extern struct effect *fx;
extern struct see_map *  see;
extern struct mapedit_queue *maped_queue;
extern struct waypoint waypoint[MAXWPS];
extern struct sk_tree sk_tree[8][12];

extern struct newcharacter *ch_new;
extern struct newcharacter *ch_temp_new;

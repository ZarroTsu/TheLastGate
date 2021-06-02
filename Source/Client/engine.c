#include <alloc.h>
#include <windows.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#pragma hdrstop
#include "dd.h"
#include "common.h"
#include "inter.h"

int init_done=0;
int frame=0;
extern int mx,my;

extern int ticker;

int pskip=0,pidle=0;
extern int t_size;

extern int cursor_type;
extern HCURSOR cursor[10];

extern int screen_width, screen_height, screen_tilexoff, screen_tileyoff;
//extern int screen_overlay_sprite;
extern short screen_windowed;
extern short screen_renderdist;

// from dd.c
int copysprite(int nr,int effect,int x,int y,int xoff,int yoff);
void dd_flip(void);
void dd_flip_windowed(void);
void dd_showbar(int xf,int yf,int xs,int ys,unsigned short col);
void copyspritex(int nr,int xpos,int ypos,int effect);
void dd_showbox(int xf,int yf,int xs,int ys,unsigned short col);
void dd_alphaeffect_magic(int nr,int str,int xpos,int ypos,int xoff,int yoff);
int get_avgcol(int nr);

char *lookup(int nr,unsigned short id);

extern char input[];
extern int in_len;
extern int cur_pos;
extern int view_pos;
extern int logstart;
extern int logtimer;
extern int RED,GREEN,BLUE;
int tput=0;

extern int do_shadow;

void do_msg(void);

char *at_name[5]={
	"Braveness",
	"Willpower",
	"Intuition",
	"Agility",
	"Strength"
};

#define AT_BRV		0
#define AT_WIL		1
#define AT_INT		2
#define AT_AGL		3
#define AT_STR		4

struct skilltab *skilltab;
struct skilltab _skilltab[50]={
//		sortkey
//	{ nr, '/', "name[40]////////",	"desc[200]/////////////////////////////////////////",			{ //////, //////, ////// }}, //
	{  0, 'C', "Hand to Hand", 		"Your ability to hit and parry while unarmed.", 				{ AT_AGL, AT_AGL, AT_STR }},
	{  1, 'F', "Precision", 		"Improves your ability to inflict critical hits.", 				{ AT_BRV, AT_AGL, AT_AGL }},
	{  2, 'D', "Dagger", 			"Your ability to hit and parry with daggers.", 					{ AT_BRV, AT_INT, AT_AGL }},
	{  3, 'D', "Sword", 			"Your ability to hit and parry with one-handed swords.",		{ AT_BRV, AT_AGL, AT_STR }},
	{  4, 'D', "Axe", 				"Your ability to hit and parry with axes.", 					{ AT_AGL, AT_STR, AT_STR }},
	{  5, 'D', "Staff", 			"Your ability to hit and parry with staffs.", 					{ AT_BRV, AT_WIL, AT_STR }},
	{  6, 'D', "Two-Handed", 		"Your ability to hit and parry with two-handed weapons.", 		{ AT_AGL, AT_AGL, AT_STR }},
	{  7, 'H', "Focus", 			"You cannot lose focus. Grants a bonus to total spellpower.",	{ AT_BRV, AT_BRV, AT_BRV }},
	{  8, 'R', "Stealth", 			"Your ability to stay hidden from others' sight.", 				{ AT_INT, AT_AGL, AT_AGL }},
	{  9, 'R', "Perception", 		"Your ability to see and hear your surroundings.", 				{ AT_WIL, AT_INT, AT_AGL }},
//	{ //, '/', "////////////////",	"//////////////////////////////////////////////////",			{ //////, //////, ////// }}, //
	{ 10, 'U', "Swimming", 			"Prevents the loss of hitpoints while moving underwater.", 		{ AT_WIL, AT_AGL, AT_STR }},
	{ 11, 'O', "Magic Shield", 		"Use: Applies a buff to yourself, granting temporary armor.", 	{ AT_BRV, AT_WIL, AT_WIL }},
	{ 12, 'U', "Bartering", 		"Getting good prices while buying or selling.", 				{ AT_BRV, AT_INT, AT_INT }},
	{ 13, 'P', "Repair", 			"Use: You will try to repair the item under your cursor.", 		{ AT_INT, AT_AGL, AT_STR }},
	{ 14, 'O', "Light", 			"Use: Make yourself or your target glow in the dark.", 			{ AT_BRV, AT_WIL, AT_INT }},
	{ 15, 'O', "Recall", 			"Use: Warp yourself to a safe place.", 							{ AT_BRV, AT_WIL, AT_INT }},
	{ 16, 'D', "Shield", 			"Your ability to score bonus parries while using a shield.", 	{ AT_BRV, AT_WIL, AT_STR }},
	{ 17, 'O', "Protect", 			"Use: Applies a buff, raising your target's armor value.", 		{ AT_BRV, AT_WIL, AT_WIL }},
	{ 18, 'O', "Enhance", 			"Use: Applies a buff, raising your target's weapon value.", 	{ AT_BRV, AT_WIL, AT_WIL }},
	{ 19, 'K', "Slow", 				"Use: Applies a debuff, reducing your target's action speed.", 	{ AT_BRV, AT_INT, AT_INT }},
//	{ //, '/', "////////////////",	"//////////////////////////////////////////////////",			{ //////, //////, ////// }}, //
	{ 20, 'K', "Curse", 			"Use: Applies a debuff, reducing your target's attributes.", 	{ AT_BRV, AT_INT, AT_INT }},
	{ 21, 'O', "Bless", 			"Use: Applies a buff, raising your target's attributes.", 		{ AT_BRV, AT_WIL, AT_WIL }},
	{ 22, 'O', "Identify", 			"Use: Identify the properties of a target or an item.", 		{ AT_BRV, AT_WIL, AT_INT }},
	{ 23, 'Q', "Resistance", 		"Resists against enemy negative spells.", 						{ AT_BRV, AT_WIL, AT_STR }},
	{ 24, 'I', "Blast", 			"Use: Damages the selected target and any around you.", 		{ AT_BRV, AT_INT, AT_INT }},
	{ 25, 'O', "Dispel", 			"Use: Removes debuffs from your target.", 						{ AT_BRV, AT_WIL, AT_INT }},
	{ 26, 'O', "Heal", 				"Use: Heals your target. Repeat uses lose their full effect.", 	{ AT_BRV, AT_INT, AT_STR }},
	{ 27, 'M', "Ghost Companion", 	"Use: Summons a companion to follow you and your commands.", 	{ AT_BRV, AT_WIL, AT_INT }},
	{ 28, 'A', "Regenerate", 		"Regenerating hitpoints, usually while out-of-combat.", 		{ AT_STR, AT_STR, AT_STR }},
	{ 29, 'A', "Rest", 				"Regenerating endurance, usually while out-of-combat.", 		{ AT_AGL, AT_AGL, AT_AGL }},
//	{ //, '/', "////////////////",	"//////////////////////////////////////////////////",			{ //////, //////, ////// }}, //
	{ 30, 'B', "Meditate", 			"Regenerating mana, usually while out-of-combat.", 				{ AT_INT, AT_INT, AT_INT }},
	{ 31, 'U', "Sense Magic", 		"Sensing who or what did something to you.", 					{ AT_BRV, AT_WIL, AT_STR }},
	{ 32, 'Q', "Immunity", 			"Reduces the strength of enemy negative spells.", 				{ AT_BRV, AT_AGL, AT_STR }},
	{ 33, 'S', "Surround Hit", 		"Hitting all surrounding enemies, including your front.", 		{ AT_BRV, AT_AGL, AT_STR }},
	{ 34, 'H', "Concentrate", 		"Reduces the mana cost of spells and abilities.", 				{ AT_WIL, AT_WIL, AT_WIL }},
	{ 35, 'G', "Warcry", 			"Use: Shout to stun and strike fear into all nearby enemies.", 	{ AT_BRV, AT_STR, AT_STR }},
	{ 36, 'D', "Dual Wield", 		"Your ability to score bonus hits while dual-wielding.", 		{ AT_BRV, AT_AGL, AT_STR }},
	{ 37, 'F', "Combat Mastery", 	"Improves your ability to hit and parry with any weapon.", 		{ AT_BRV, AT_AGL, AT_STR }},
	{ 38, 'D', "Weapon Mastery", 	"Improves the weapon value for your equipped weapon.", 			{ AT_BRV, AT_AGL, AT_STR }},
	{ 39, 'E', "Armor Mastery", 	"Improves the armor value for your equipped armor.", 			{ AT_BRV, AT_AGL, AT_STR }},
//	{ //, '/', "////////////////",	"//////////////////////////////////////////////////",			{ //////, //////, ////// }}, //
	{ 40, 'G', "Cleave", 			"Use: Strike your foe and deal damage to surrounding enemies.", { AT_BRV, AT_STR, AT_STR }},
	{ 41, 'G', "Weaken", 			"Use: Reduce your foe's weapon and armor values.", 				{ AT_BRV, AT_AGL, AT_AGL }},
	{ 42, 'I', "Poison", 			"Use: Poison the selected target and any around you.", 			{ AT_BRV, AT_INT, AT_INT }},
	{ 43, 'J', "Damage Proximity", 	"Increases the area-of-effect of both Blast and Poison.", 		{ AT_WIL, AT_INT, AT_INT }},
	{ 44, 'L', "Hex Proximity", 	"Increases the area-of-effect of Poison, Curse and Slow.", 		{ AT_WIL, AT_INT, AT_INT }},
	{ 45, 'N', "Companion Mastery", "Improves the power and abilities of your ghost companion.", 	{ AT_BRV, AT_WIL, AT_INT }},
	{ 46, 'N', "Shadow Copy", 		"Use: Summon a doppelganger to attack your enemies with you.", 	{ AT_WIL, AT_AGL, AT_AGL }},
	{ 47, 'O', "Haste", 			"Use: Applies a buff to yourself, increasing action speed.",	{ AT_BRV, AT_WIL, AT_WIL }},
	{ 48, 'T', "Surround Area",		"Improves the area-of-effect for Surround Hit.", 				{ AT_BRV, AT_STR, AT_STR }},
	{ 49, 'T', "Surround Rate", 	"Improves how often Surround Hit is triggered.", 				{ AT_BRV, AT_AGL, AT_STR }}
//	{ //, '/', "////////////////",	"//////////////////////////////////////////////////",			{ //////, //////, ////// }}, //
};

struct wpslist wpslist[MAXWPS]={
//    nr, "123456789012345678901234567890",	"\"2345678901234567890123456789\""
	{  0, "Lynbore, Tavern", 				"\"Humble beginnings.\"" },
	{  1, "Lynbore, East", 					"\"Ghosts and spiders duel.\"" },
	{  2, "Aston, South", 					"\"The Weeping Woods.\"" },
	{  3, "Aston, Crossroads", 				"\"Between ivory and violet.\"" },
	{  4, "Aston, West", 					"\"The Strange Forest.\"" },
	{  5, "Aston, East", 					"\"The Autumn Meadow.\"" },
	{  6, "Forgotten Canyon", 				"\"Old secrets echo.\"" },
	{  7, "Lizard Temple, North", 			"\"The Beryl Jungle.\"" },
	{  8, "Temple of Osiris", 				"\"Gods toy with greed.\"" },
	{  9, "Neiseer, West", 					"\"The Basalt Desert.\"" },
	{ 10, "Neiseer", 						"\"Twilit stars sing sweetly.\"" },
	{ 11, "Neiseer, North", 				"\"The Burning Plains.\"" },
	{ 12, "Mausoleum, Basement", 			"\"An accursed tomb.\"" },
	{ 13, "Pentagram Quest, Novice", 		"\"Endless adventure begins.\"" },
	{ 14, "Pentagram Quest, Earth", 		"\"Smells of dirt and soot.\"" },
	{ 15, "Pentagram Quest, Fire", 			"\"Boiling and burning.\"" },
	{ 16, "Pentagram Quest, Jungle", 		"\"A verdant expanse.\"" },
	{ 17, "Pentagram Quest, Ice", 			"\"Frozen still for eternity.\"" },
	{ 18, "Pentagram Quest, Underwater",	"\"Embrace foul waters.\"" },
	{ 19, "Pentagram Quest, Onyx", 			"\"Depths known only by fear.\"" }
//    nr, "123456789012345678901234567890",	"123456789012345678901234567890"
};

int skill_cmp(const void *a,const void *b)
{
	const struct skilltab *c,*d;
	int m1,m2;

	c=a; d=b;

	m1=c->nr; m2=d->nr;
	
	if (m1==99 && m2!=99) return 1;
	if (m2==99 && m1!=99) return -1;
	
	if (pl.skill[m1][0]==0 && pl.skill[m2][0]!=0) return 1;
	if (pl.skill[m2][0]==0 && pl.skill[m1][0]!=0) return -1;

	// (m1==8||m1==23||m1==31||m1==32) // Stealth, Resistance, Sense-Magic, Immunity -- these are all active even if you don't know them.
	if (pl.skill[m1][0]==0 && pl.skill[m2][0]==0 && (m1==8||m1==23||m1==31||m1==32) && (m2!=8&&m2!=23&&m2!=31&&m2!=32)) return -1;
	if (pl.skill[m2][0]==0 && pl.skill[m1][0]==0 && (m2==8||m2==23||m2==31||m2==32) && (m1!=8&&m1!=23&&m1!=31&&m1!=32)) return 1;

	if (c->sortkey>d->sortkey) return 1;
	if (c->sortkey<d->sortkey) return -1;

	return strcmp(c->name,d->name);
}

// from main.c
extern int quit;

int idle=0;
int ttime=0,xtime=0;
int ctick=0;

int do_exit=0;

int xoff=0,yoff=0;

extern int selected_char;

struct look look={0,{0,0,0,0,0,0,0,0,0,0},0,0,"",0,0,0,0};

// ************* CHARACTER ****************

struct cplayer pl;

static char *rank[25]={
	"Private",
	"Private First Class",
	"Lance Corporal",
	"Corporal",
	"Sergeant",
	"Staff Sergeant",
	"Master Sergeant",
	"First Sergeant",
	"Sergeant Major",
	"Second Lieutenant",
	"First Lieutenant",
	"Captain",
	"Major",
	"Lieutenant Colonel",
	"Colonel",
	"Brigadier General",
	"Major General",
	"Lieutenant General",
	"General",
	"Field Marshal",
	"Knight",
	"Baron",
	"Earl",
	"Marquess",
	"Warlord"
};
static int rank_sprite[25]={
	   10,    11,    12,    13,    14, 
	   15,    16,    17,    18,    19, 
	   20,    21,    22,    23,    24, 
	   25,    26,    27,    28,    29,
	   30,     6,     7,     8,     9
};

int stat_raised[108]={0,0,0,0,0,0,0,0,0,0,0,0,0,};
int stat_points_used=0;

int points2rank(int v)
{
	if (v<      250)	return( 0); // Private
	if (v<     1750)	return( 1); // Private FIrst Class
	if (v<     7000)	return( 2); // Lance Corporal
	if (v<    21000)	return( 3); // Corporal
	if (v<    52500)	return( 4); // Sergeant
	if (v<   115500)	return( 5); // Staff Sergeant
	if (v<   231000)	return( 6); // Master Sergeant
	if (v<   429000)	return( 7); // First Sergeant
	if (v<   750750)	return( 8); // Sergeant Major
	if (v<  1251250)	return( 9); // Second Lieutenant
	if (v<  2002000)	return(10); // First Lieutenant
	if (v<  3094000)	return(11); // Captain
	if (v<  4641000)	return(12); // Major
	if (v<  6783000)	return(13); // Lieutenant Colonel
	if (v<  9690000)	return(14); // Colonel
	if (v< 13566000)	return(15); // Brigadier General
	if (v< 18653250)	return(16); // Major General
	if (v< 25236750)	return(17); // Lieutenant General
	if (v< 33649000)	return(18); // General
	if (v< 44275000)	return(19); // Field Marshal
	if (v< 57557500)	return(20); // Knight
	if (v< 74002500)	return(21); // Baron
	if (v< 94185000)	return(22); // Earl
	if (v<118755000)	return(23); // Marquess
						return(24); // Warlord
}

int rank2points(int v)
{
	if (v==1)	return 250;
	if (v==2)	return 1750;
	if (v==3)	return 7000;
	if (v==4)	return 21000;
	if (v==5)	return 52500;
	if (v==6)	return 115500;
	if (v==7)	return 231000;
	if (v==8)	return 429000;
	if (v==9)	return 750750;
	if (v==10)	return 1251250;
	if (v==11)	return 2002000;
	if (v==12)	return 3094000;
	if (v==13)	return 4641000;
	if (v==14)	return 6783000;
	if (v==15)	return 9690000;
	if (v==16)	return 13566000;
	if (v==17)	return 18653250;
	if (v==18)	return 25236750;
	if (v==19)	return 33649000;
	if (v==20)	return 44275000;
	if (v==21)	return 57557500;
	if (v==22)	return 74002500;
	if (v==23)	return 94185000;
	if (v==24)	return 118755000;
				return 0;
}

/* Calculates experience to next level from current experience and the
   points2rank() function. As no inverse function is supplied we use a
   binary search to determine the experience for the next level.
   If the given number of points corresponds to the highest level,
   return 0. */
int points_tolevel(int curr_exp)
{
        int curr_level, next_level, r, j;  //, p0, p5, p9;

		if (!curr_exp) return 50;	//0 exp
        curr_level = points2rank(curr_exp);
        if (curr_level == 24) return 0;
        next_level = curr_level + 1;

		r = rank2points(next_level);
		j = r-curr_exp;
		
		return j;
}

int attrib_needed(int n,int v)
{
	if (v>=pl.attrib[n][2])	return HIGH_VAL;

	return v*v*v*pl.attrib[n][3]/20;
}

int hp_needed(int v)
{
	if (v>=pl.hp[2]) return HIGH_VAL;

	return v*pl.hp[3];
}

int end_needed(int v)
{
	if (v>=pl.end[2]) return HIGH_VAL;

	return v*pl.end[3]/2;
}

int mana_needed(int v)
{
	if (v>=pl.mana[2]) return HIGH_VAL;

	return v*pl.mana[3];
}

int skill_needed(int n,int v)
{
	if (v>=pl.skill[n][2]) return HIGH_VAL;

	return max(v,v*v*v*pl.skill[n][3]/40);
}



// ************* MAP **********************

struct cmap *map=NULL;

void eng_init_map(void)
{
	int n;

	map=calloc(screen_renderdist*screen_renderdist*sizeof(struct cmap),1);

	for (n=0; n<screen_renderdist*screen_renderdist; n++)	map[n].ba_sprite=SPR_EMPTY;
}

void eng_init_player(void)
{
	memset(&pl,0,sizeof(struct cplayer));
}

// ************* DISPLAY ******************

unsigned int    inv_pos=0,show_shop=0,waypoints=0,show_wps=0;

unsigned int    show_motd=0,show_newp=0,show_tuto=0,tuto_page=0,tuto_max=0;

unsigned int    skill_pos=0,wps_pos=0,mm_magnify=1;

unsigned int   show_look=0,
look_nr=0,			// look at char/item nr
look_type=0,		// 1=char, 2=item
look_timer=0;		// look_timer

unsigned char   inv_block[20]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

extern int inv_object;			// object carried with the mouse cursor
extern int mouse_x,mouse_y;	// current mouse coordinates

#define XS	49
#define LL	22
#define XLL (22*10)
static char logtext[XLL][60];
static char logfont[XLL];

#define MAXTS            20

// 2020 - Zarrotsu
#define GUI_HP_COUNT_X	  9
#define GUI_HP_COUNT_Y	288
#define GUI_EN_COUNT_X	  9
#define GUI_EN_COUNT_Y	302
#define GUI_MP_COUNT_X	  9
#define GUI_MP_COUNT_Y	316

#define GUI_MONEY_X		442
#define GUI_MONEY_Y		114

#define GUI_UPDATE_X	144
#define GUI_UPDATE_Y	260

#define GUI_UPOINTS_X	189
#define GUI_UPOINTS_Y	260

//					   HEAD,NECK,BODY,ARMS,BELT,CHRM,FEET,LHND,RHND,CLOK,LRNG,RRNG,CHRM2
int gui_equ_x[]		= { 738, 700, 738, 704, 738, 777, 738, 806, 670, 772, 776, 700, 801 };
int gui_equ_y[]		= {   5,  18,  39,  56,  73,  17, 107,  56,  56,  56,  94,  94,  17 };

#define GUI_BAR_X		845
#define GUI_BAR_HP		131
#define GUI_BAR_EN		138
#define GUI_BAR_MP		145

#define GUI_F_COL1		1084
#define GUI_F_COL2		1132
#define GUI_F_COL3		1180
#define GUI_F_COL4		1228
#define GUI_F_ROW1		 664
#define GUI_F_ROW2		 679
#define GUI_F_ROW3		 694

#define GUI_WV_X		1112
#define GUI_WV_Y		 261

#define GUI_XPBAR_X		1109
#define GUI_XPBAR_Y		 300
#define GUI_XPBAR_W		 165

#define GUI_LOG_X		977

#define GUI_DPS_X		 265
#define GUI_DPS_Y		 120


//#define GUI_SHOP_X	220
//#define GUI_SHOP_Y	260
#define GUI_SHOP_X		((1280/2)-(320/2))
#define GUI_SHOP_Y		((736/2)-(320/2)+72)

#define GUI_BAR_BLU		0x00B0
#define GUI_BAR_GRE		0x0B00
#define GUI_BAR_RED		0xB000

#define GUI_BAR_EXP		0xBB00

#define CHAR_BAR_HP		0x0B00
#define CHAR_BAR_RD		0xA000
#define CHAR_BAR_BL		0x0020

//

int load=0;

void display_meta_from_ls()
{
	int pl_speed, pl_atksp, pl_spmod, pl_spapt;
	int pl_critc, pl_critm, pl_topdm, pl_reflc;
	int pl_hitsc, pl_parry, pl_coold, pl_casts;
	int pl_flags, pl_flagb, pl_basel, pl_dmgml;
	int pl_dlow,  pl_dhigh, pl_dps,   pl_cdrate;
	int moonmult = 20;
	int hpmult, endmult, manamult;
	int race_reg = 0, race_res = 0, race_med = 0;
	int sk_proxi, sk_immun, sk_resis, sk_conce, sk_ghost, sk_poiso, sk_slowv, sk_curse;
	int sk_blast, sk_scorc, sk_douse, sk_pulse, sk_pucnt, sk_razor, sk_trice, sk_blind;
	int sk_water, sk_cleav, sk_taunt, sk_weake, sk_warcr, sk_regen, sk_restv, sk_medit;
	
	// Player Speed and Attack Speed - WN_SPEED
	pl_speed = SPEED_CAP - pl.worn[WN_SPEED]; if (pl_speed > SPEED_CAP) pl_speed = SPEED_CAP;
	pl_atksp = pl_speed + pl.worn_p[WN_SPEED]; if (pl_atksp > SPEED_CAP) pl_atksp = SPEED_CAP;

	// Player Spell Mod and Spell Apt - WN_SPMOD
	pl_spmod = pl.worn[WN_SPMOD];
	pl_spapt = pl.worn_p[WN_SPMOD];
	
	// Player Crit chance and Crit Multiplier - WN_CRIT
	pl_critc = pl.worn[WN_CRIT];
	pl_critm = pl.worn_p[WN_CRIT];
	
	// Player Top damage and Reflection - WN_TOP
	pl_topdm = pl.worn[WN_TOP];
	pl_reflc = pl.worn_p[WN_TOP];
	
	// Player Hit and Parry - WN_HITPAR
	pl_hitsc = pl.worn[WN_HITPAR];
	pl_parry = pl.worn_p[WN_HITPAR];
	
	// Player Cooldown and Cast Speed - WN_CLDWN
	pl_coold = pl.worn[WN_CLDWN];
	pl_casts = pl_speed + pl.worn_p[WN_CLDWN]; if (pl_casts > SPEED_CAP) pl_casts = SPEED_CAP;
	
	// Player Flags from special items
	pl_flags = pl.worn[WN_FLAGS];
	pl_flagb = pl.worn_p[WN_FLAGS];
	pl_basel = 100;
	pl_dmgml = 100;
	
	// Book - Damor's Grudge (cooldown bonus)
	if (pl_flags & (1 <<  0))
		pl_basel =  90;
	
	// Tarot - Strength (damage multiplier)
	if (pl_flags & (1 <<  1))
		pl_dmgml = 125;
	
	// Player DPS - pl_dlow, pl_dhigh, pl_dps
	pl_dlow  = (pl.weapon*pl_dmgml/100)/4;
	pl_dhigh = pl.weapon+8+pl_topdm+6;
	pl_dhigh = ((pl_dhigh+pl_dhigh*pl_critc*pl_critm/1000000)*pl_dmgml/100)/4;
	pl_dps   = ((pl_dlow+pl_dhigh)/2)*pl_atksp;
	
	// Player cooldown rate - pl_cdrate
	pl_cdrate = 100 * pl_basel / max(100, pl_coold);
	
	/*
		Moon multiplier adjustments
	*/
	// FULL MOON
	if (pl_flagb & (1 <<  8))
	{
		moonmult = 30;
	}
	// NEW MOON
	if (pl_flagb & (1 <<  9))
	{
		moonmult = 40;
	}
	hpmult = endmult = manamult = moonmult;
	
	/*
		Additional skill bonuses for GUI
	*/
	sk_proxi = pl.skill[44][5] / (300/12);
	sk_ghost = (pl.skill[27][5]*pl_spmod/100) * 5 / 11;
	sk_poiso = (pl.skill[42][5]*pl_spmod/100) * 15 / 3 * 18;
	sk_blast = (pl.skill[24][5]*pl_spmod/100) * 2 * 750/1000;
	sk_scorc = (1000 + (pl.skill[24][5]*pl_spmod/100));
	sk_douse = -(pl.skill[24][5]*pl_spmod/100);
	sk_pulse = (pl.skill[43][5]*pl_spmod/100) * 2 * 75/1000;
	sk_pucnt = 60 / (6 * pl_basel / max(100, pl_coold));
	sk_razor = (pl.skill[7][5]+pl.weapon/4) * 2 * 75/1000;
	sk_trice = (pl.skill[49][5]+max(0,(pl_atksp-120))/4) * 750/1000;
	sk_blind = -(pl.skill[37][5] / 6 + 3);
	sk_water = 25 * 18;
	sk_cleav = (pl.skill[40][5]+pl.weapon/4) * 2 * 750/1000;
	sk_taunt = (1000 - pl.skill[48][5]);
	sk_warcr = -(2+(pl.skill[35][5]/(10/3)) / 5);
	
	// Tarot - Hanged Man (immunity&resistance)
	if (pl_flags & (1 <<  2))
	{
		sk_immun = pl.skill[32][5] + pl.skill[23][5] * 30/100;
		sk_resis = pl.skill[23][5] * 70/100;
	}
	else
	{
		sk_immun = pl.skill[32][5];
		sk_resis = pl.skill[23][5];
	}
	
	// Book - Great Prodigy (concentrate bonus)
	if (pl_flags & (1 <<  3))
	{
		sk_conce = max(1, 100 - 100 * pl.skill[34][5] / 300);
	}
	else
	{
		sk_conce = max(1, 100 - 100 * pl.skill[34][5] / 400);
	}
	
	// Book - Venom Compendium (poison bonus)
	if (pl_flags & (1 <<  4))
	{
		sk_poiso *= 5 / 4;
	}
	
	// Tarot - Emperor (slow bonus)
	if (pl_flags & (1 <<  5))
	{
		sk_slowv = -(30 + (pl.skill[19][5]*pl_spmod/100)/(8/3));
	}
	else
	{
		sk_slowv = -(30 + (pl.skill[19][5]*pl_spmod/100)/2);
	}
	
	// Tarot - Tower (curse bonus)
	if (pl_flags & (1 <<  6))
	{
		sk_curse = -(2 + (((pl.skill[20][5]*pl_spmod/100)*4/3)-4)/5);
	}
	else
	{
		sk_curse = -(2 + ((pl.skill[20][5]*pl_spmod/100)-4)/5);
	}
	
	// Swimming skill
	if (pl.skill[10][0])
	{
		sk_water = (250 - pl.skill[10][5]/3*2) * 18/10;
	}
	
	// Amulet - Water breathing (degen/2)
	if (pl_flagb & (1 <<  4))
	{
		sk_water /= 2;
	}
	
	// Tarot - Justice (Cleave reduction)
	if (pl_flags & (1 <<  8))
	{
		sk_cleav *= 70 / 100;
	}
	
	// Tarot - Temperance (Taunt reduction)
	if (pl_flags & (1 <<  9))
	{
		sk_taunt *= 70 / 100;
	}
	
	// Tarot - Death (Weaken bonus)
	if (pl_flags & (1 << 10))
	{
		sk_weake = -(pl.skill[41][5] / 6 + 2);
	}
	else
	{
		sk_weake = -(pl.skill[41][5] / 8 + 2);
	}
	
	/*
		Regeneration stats
	*/
	race_reg = pl.skill[28][5] * moonmult / 15;
	race_res = pl.skill[29][5] * moonmult / 15;
	race_med = pl.skill[30][5] * moonmult / 15;
	
	// Tarot - Moon :: While not full mana, life regen is mana regen
	if ((pl_flags & (1 << 11)) && (pl.a_mana<pl.mana[5]))
	{
		race_med += race_reg;	race_reg -= race_reg;
		manamult += hpmult;		hpmult   -= hpmult;
	}
	// Tarot - Sun :: While not full life, end regen is life regen
	if ((pl_flags & (1 << 12)) && (pl.a_hp<pl.hp[5]))
	{
		race_reg += race_res;	race_res -= race_res;
		hpmult   += endmult;	endmult  -= endmult;
	}
	// Tarot - World :: While not full end, mana regen is end regen
	if ((pl_flags & (1 << 13)) && (pl.a_end<pl.end[5]))
	{
		race_res += race_med;	race_med -= race_med;
		endmult  += manamult;	manamult -= manamult;
	}
	
	// Amulet - Bloodstone
	if (pl_flagb & (1 <<  5))
	{
		race_med /= 2;
		race_reg *= 2;
	}
	// Amulet - Verdant
	if (pl_flagb & (1 <<  6))
	{
		race_reg /= 2;
		race_res *= 2;
	}
	// Amulet - Sea
	if (pl_flagb & (1 <<  7))
	{
		race_res /= 2;
		race_med *= 2;
	}
		
	sk_regen = (pl.skill[28][0]?race_reg:0) + hpmult   * 2;
	sk_restv = (pl.skill[29][0]?race_res:0) + endmult  * 4;
	sk_medit = (pl.skill[30][0]?race_med:0) + manamult * 1;
	
	// Amulet - Standard Ankh
	if (pl_flagb & (1 <<  0))
	{
		sk_regen += pl.skill[28][0]?race_reg/12:0;
		sk_restv += pl.skill[29][0]?race_res/12:0;
		sk_medit += pl.skill[30][0]?race_med/12:0;
	}
	// Amulet - Amber Ankh (Life)
	if (pl_flagb & (1 <<  1))
	{
		sk_regen += pl.skill[28][0]?race_reg/ 6:0;
		sk_restv += pl.skill[29][0]?race_res/24:0;
		sk_medit += pl.skill[30][0]?race_med/24:0;
	}
	// Amulet - Turquoise Ankh (End)
	if (pl_flagb & (1 <<  2))
	{
		sk_regen += pl.skill[28][0]?race_reg/24:0;
		sk_restv += pl.skill[29][0]?race_res/ 6:0;
		sk_medit += pl.skill[30][0]?race_med/24:0;
	}
	// Amulet - Garnet Ankh (Mana)
	if (pl_flagb & (1 <<  3))
	{
		sk_regen += pl.skill[28][0]?race_reg/24:0;
		sk_restv += pl.skill[29][0]?race_res/24:0;
		sk_medit += pl.skill[30][0]?race_med/ 6:0;
	}
	
	sk_regen *= 18/10;
	sk_restv *= 18/10;
	sk_medit *= 18/10;

	// DRAW THE GUI INFO
	switch (last_skill)
	{												// ".............." // 
		case 50: // Braveness - 1. Cast Speed 	2. Crit Chanc	3. Crit Multi	4. Prox Bonus 	5. eImmunity	6. eResist
			dd_xputtext(GUI_DPS_X,    GUI_DPS_Y+14*0,1,"Cast Speed");
			dd_xputtext(GUI_DPS_X+103,GUI_DPS_Y+14*0,1,"%6d.%02d",pl_casts/100,pl_casts%100);
			dd_xputtext(GUI_DPS_X,    GUI_DPS_Y+14*1,1,"Crit Chance");
			dd_xputtext(GUI_DPS_X+103,GUI_DPS_Y+14*1,1,"%5d.%02d%%",pl_critc/100,pl_critc%100);
			dd_xputtext(GUI_DPS_X,    GUI_DPS_Y+14*2,1,"Crit Multi");
			dd_xputtext(GUI_DPS_X+103,GUI_DPS_Y+14*2,1,"%8d%%",pl_critm);
			dd_xputtext(GUI_DPS_X,    GUI_DPS_Y+14*3,1,"E.Immunity");
			dd_xputtext(GUI_DPS_X+103,GUI_DPS_Y+14*3,1,"%9d",sk_immun);
			dd_xputtext(GUI_DPS_X,    GUI_DPS_Y+14*4,1,"E.Resist");
			dd_xputtext(GUI_DPS_X+103,GUI_DPS_Y+14*4,1,"%9d",sk_resis);
				if (pl_reflc>0) {
			dd_xputtext(GUI_DPS_X,    GUI_DPS_Y+14*5,1,"Damage Reflect");
			dd_xputtext(GUI_DPS_X+103,GUI_DPS_Y+14*5,1,"%9d",pl_reflc);
			}
			break;									// ".............." // 
		case 51: // Willpower - 1. Apt Bonus, 	2. Mana reduce, 3. GC Power		4. Poison DPS	5. Slow Pow 	6. Curse Pow
			dd_xputtext(GUI_DPS_X,    GUI_DPS_Y+14*0,1,"Aptitude Bonus");
			dd_xputtext(GUI_DPS_X+103,GUI_DPS_Y+14*0,1,"%9d",pl.attrib[AT_WIL][5]/4);
				if (pl.skill[34][0]) {
			dd_xputtext(GUI_DPS_X,    GUI_DPS_Y+14*1,1,"Mana Cost %");
			dd_xputtext(GUI_DPS_X+103,GUI_DPS_Y+14*1,1,"%6d.%02d",sk_conce/100,sk_conce%100);
			}	if (pl.skill[27][0]) {
			dd_xputtext(GUI_DPS_X,    GUI_DPS_Y+14*2,1,"Ghost Potency");
			dd_xputtext(GUI_DPS_X+103,GUI_DPS_Y+14*2,1,"%9d",sk_ghost);
			}	if (pl.skill[42][0]) {
			dd_xputtext(GUI_DPS_X,    GUI_DPS_Y+14*3,1,"Poison Degen/s");
			dd_xputtext(GUI_DPS_X+103,GUI_DPS_Y+14*3,1,"%6d.%02d",sk_poiso/100,sk_poiso%100);
			}	if (pl.skill[19][0]) {
			dd_xputtext(GUI_DPS_X,    GUI_DPS_Y+14*4,1,"Slow Effect");
			dd_xputtext(GUI_DPS_X+103,GUI_DPS_Y+14*4,1,"%9d",sk_slowv);
			}	if (pl.skill[20][0]) {
			dd_xputtext(GUI_DPS_X,    GUI_DPS_Y+14*5,1,"Curse Effect");
			dd_xputtext(GUI_DPS_X+103,GUI_DPS_Y+14*5,1,"%9d",sk_curse);
			}
			break;									// ".............." // 
		case 52: // Intuition - 1. Cooldown, 	2. Medit Rate, 	3. Blast DPH	4. Scorch Pow	5. Pulse DPH	6. Pulse Count
			dd_xputtext(GUI_DPS_X,    GUI_DPS_Y+14*0,1,"Cooldown Rate");
			dd_xputtext(GUI_DPS_X+103,GUI_DPS_Y+14*0,1,"%6d.%02d",pl_cdrate/100,pl_cdrate%100);
			dd_xputtext(GUI_DPS_X,    GUI_DPS_Y+14*1,1,"Mana Regen/s");
			dd_xputtext(GUI_DPS_X+103,GUI_DPS_Y+14*1,1,"%6d.%02d",sk_medit/100,sk_medit%100);
				if (pl.skill[24][0]) {
			dd_xputtext(GUI_DPS_X,    GUI_DPS_Y+14*2,1,"Blast DPH");
			dd_xputtext(GUI_DPS_X+103,GUI_DPS_Y+14*2,1,"%9d",sk_blast);
				// Tarot - Judgement (blast change)
				if (pl_flags & (1 <<  7)) {
			dd_xputtext(GUI_DPS_X,    GUI_DPS_Y+14*3,1,"Douse Effect");
			dd_xputtext(GUI_DPS_X+103,GUI_DPS_Y+14*3,1,"%5d.%03d",sk_douse/1000,sk_douse%1000);
			}	else {
			dd_xputtext(GUI_DPS_X,    GUI_DPS_Y+14*3,1,"Scorch Effect");
			dd_xputtext(GUI_DPS_X+103,GUI_DPS_Y+14*3,1,"%5d.%03d",sk_scorc/1000,sk_scorc%1000);
			} }	if (pl.skill[43][0]) {
			dd_xputtext(GUI_DPS_X,    GUI_DPS_Y+14*4,1,"Pulse DPH");
			dd_xputtext(GUI_DPS_X+103,GUI_DPS_Y+14*4,1,"%9d",sk_pulse);
			dd_xputtext(GUI_DPS_X,    GUI_DPS_Y+14*5,1,"Pulse Count");
			dd_xputtext(GUI_DPS_X+103,GUI_DPS_Y+14*5,1,"%9d",sk_pucnt);
			}
			break;									// ".............." // 
		case 53: // Agility - 	1. Atk Speed, 	2. Rest Rate, 	3. Razor DPH	4. Trice DPH	5. Blind Pow	6. Water Degen
			dd_xputtext(GUI_DPS_X,    GUI_DPS_Y+14*0,1,"Attack Speed");
			dd_xputtext(GUI_DPS_X+103,GUI_DPS_Y+14*0,1,"%6d.%02d",pl_atksp/100,pl_atksp%100);
			dd_xputtext(GUI_DPS_X,    GUI_DPS_Y+14*1,1,"Endur. Regen/s");
			dd_xputtext(GUI_DPS_X+103,GUI_DPS_Y+14*1,1,"%6d.%02d",sk_restv/100,sk_restv%100);
				if (pl.skill[7][0]) {
			dd_xputtext(GUI_DPS_X,    GUI_DPS_Y+14*2,1,"Razor DPH");
			dd_xputtext(GUI_DPS_X+103,GUI_DPS_Y+14*2,1,"%9d",sk_razor);
			}	if (pl.skill[49][0]) {
			dd_xputtext(GUI_DPS_X,    GUI_DPS_Y+14*3,1,"Trice DPH");
			dd_xputtext(GUI_DPS_X+103,GUI_DPS_Y+14*3,1,"%9d",sk_trice);
			}	if (pl.skill[37][0]) {
			dd_xputtext(GUI_DPS_X,    GUI_DPS_Y+14*4,1,"Blind Effect");
			dd_xputtext(GUI_DPS_X+103,GUI_DPS_Y+14*4,1,"%9d",sk_blind);
			}
			dd_xputtext(GUI_DPS_X,    GUI_DPS_Y+14*5,1,"UWater Degen/s");
			dd_xputtext(GUI_DPS_X+103,GUI_DPS_Y+14*5,1,"%6d.%02d",sk_water/100,sk_water%100);
			break;									// ".............." // 
		case 54: // Strength - 	1. Melee DPH, 	2. Regen Rate, 	3. Cleave DPH	4. Taunt Pow	5. Weaken Pow	6. Warcry Pow
			dd_xputtext(GUI_DPS_X,    GUI_DPS_Y+14*0,1,"Est. Melee DPH");
			dd_xputtext(GUI_DPS_X+103,GUI_DPS_Y+14*0,1,"%3d - %3d",pl_dlow,pl_dhigh);
			dd_xputtext(GUI_DPS_X,    GUI_DPS_Y+14*1,1,"Health Regen/s");
			dd_xputtext(GUI_DPS_X+103,GUI_DPS_Y+14*1,1,"%6d.%02d",sk_regen/100,sk_regen%100);
				if (pl.skill[40][0]) {
			dd_xputtext(GUI_DPS_X,    GUI_DPS_Y+14*2,1,"Cleave DPH");
			dd_xputtext(GUI_DPS_X+103,GUI_DPS_Y+14*2,1,"%9d",sk_cleav);
			}	if (pl.skill[48][0]) {
			dd_xputtext(GUI_DPS_X,    GUI_DPS_Y+14*3,1,"Taunt Guard");
			dd_xputtext(GUI_DPS_X+103,GUI_DPS_Y+14*3,1,"%5d.%03d",sk_taunt/1000,sk_taunt%1000);
			}	if (pl.skill[41][0]) {
			dd_xputtext(GUI_DPS_X,    GUI_DPS_Y+14*4,1,"Weaken Effect");
			dd_xputtext(GUI_DPS_X+103,GUI_DPS_Y+14*4,1,"%9d",sk_weake);
			}	if (pl.skill[35][0]) {
			dd_xputtext(GUI_DPS_X,    GUI_DPS_Y+14*5,1,"Warcry Effect");
			dd_xputtext(GUI_DPS_X+103,GUI_DPS_Y+14*5,1,"%9d",sk_warcr);
			}
			break;									// ".............." // 
		default: // Default -	1. Melee DPS,	2. Hit, 		3. Parry, 		 4. Spell Mod	5. Spell Apt 	6. Act Speed
			dd_xputtext(GUI_DPS_X,    GUI_DPS_Y+14*0,1,"Est. Melee DPS");
			dd_xputtext(GUI_DPS_X+103,GUI_DPS_Y+14*0,1,"%6d.%02d",pl_dps/100,pl_dps%100);
			dd_xputtext(GUI_DPS_X,    GUI_DPS_Y+14*1,1,"Hit Score");
			dd_xputtext(GUI_DPS_X+103,GUI_DPS_Y+14*1,1,"%9d",pl_hitsc);
			dd_xputtext(GUI_DPS_X,    GUI_DPS_Y+14*2,1,"Parry Score");
			dd_xputtext(GUI_DPS_X+103,GUI_DPS_Y+14*2,1,"%9d",pl_parry);
			dd_xputtext(GUI_DPS_X,    GUI_DPS_Y+14*3,1,"Spell Modifier");
			dd_xputtext(GUI_DPS_X+103,GUI_DPS_Y+14*3,1,"%6d.%02d",pl_spmod/100,pl_spmod%100);
			dd_xputtext(GUI_DPS_X,    GUI_DPS_Y+14*4,1,"Spell Aptitude");
			dd_xputtext(GUI_DPS_X+103,GUI_DPS_Y+14*4,1,"%9d",pl_spapt);
			dd_xputtext(GUI_DPS_X,    GUI_DPS_Y+14*5,1,"Action Speed");
			dd_xputtext(GUI_DPS_X+103,GUI_DPS_Y+14*5,1,"%6d.%02d",pl_speed/100,pl_speed%100);
			break;									// ".............." // 
	}
	
	// Draw red rectangle around selected last_skill, for context
	if (last_skill >= 50)
	{
		dd_showbox(6,6+19*(last_skill-50),129,11,(unsigned short)(RED));
	}
	
	// Draw a moon icon?  261 212
	/*
	if (moonmult==40)
	{
		// New Moon
	}
	if (moonmult==30)
	{
		// Full Moon
	}
	else
	{
		// Standard Moon
	}
	*/
	
}

void eng_display_win(int plr_sprite,int init)
{
	int y,n,m;
	char *tmp,buf[50];
	int pl_flags, pl_flagb;

	//if (load) dd_xputtext(670,300+MAXTS,1,"%3d%%",load);

	if (init) {
		reset_block();

		// Draw red rectangles around lower right gui component toggles
		if (pl.mode==2)			dd_showbox(GUI_F_COL1,GUI_F_ROW1,45,12,(unsigned short)(RED));
		else if (pl.mode==1)	dd_showbox(GUI_F_COL2,GUI_F_ROW1,45,12,(unsigned short)(RED));
		else if (pl.mode==0)	dd_showbox(GUI_F_COL3,GUI_F_ROW1,45,12,(unsigned short)(RED));
		if (pdata.show_proz)	dd_showbox(GUI_F_COL4,GUI_F_ROW1,45,12,(unsigned short)(RED));
		//
		if (pdata.show_stats)	dd_showbox(GUI_F_COL1,GUI_F_ROW2,45,12,(unsigned short)(RED));
		if (pdata.hide)			dd_showbox(GUI_F_COL2,GUI_F_ROW2,45,12,(unsigned short)(RED));
		if (pdata.show_names)	dd_showbox(GUI_F_COL3,GUI_F_ROW2,45,12,(unsigned short)(RED));
		if (pdata.show_bars)	dd_showbox(GUI_F_COL4,GUI_F_ROW2,45,12,(unsigned short)(RED));

		// inventory    251  6
		for (n=0; n<30; n++) {
			// Draw inventory items
			if (pl.item[n+inv_pos]) {
				if (hightlight==HL_BACKPACK && hightlight_sub==n+(signed)inv_pos)
					copyspritex(pl.item[n+inv_pos],261+(n%10)*34,6+(n/10)*34,16);
				else
					copyspritex(pl.item[n+inv_pos],261+(n%10)*34,6+(n/10)*34,0);
				// Draw stack count 
				if (pl.item_s[n+inv_pos]>0)
				{
					copyspritex(4000+pl.item_s[n+inv_pos],261+(n%10)*34,6+(n/10)*34,0);
				}
				// Draw lock icon for locked items
				if (pl.item_l[n+inv_pos])
				{
					copyspritex(4000,261+(n%10)*34,6+(n/10)*34,0);
				}
			}
			// Draw shortcut key names
			for (m=0; m<16; m++)
			{
				if (pdata.xbutton[m].skill_nr==100+n+inv_pos)
				{
					copyspritex(4011+m,261+(n%10)*34,6+(n/10)*34,0);
				}
			}
		}

		// spells
		for (n=0; n<MAXBUFFS; n++) {
			if (pl.spell[n]) {
				copyspritex(pl.spell[n],848+(n%6)*20,8+(n/6)*23,15-min(15,pl.active[n]));
			}
		}

		// Scroll Bars for Skills and Inventory
		dd_showbar(234,152+(skill_pos*58)/40+(skill_pos>25?1:0), 11,11,(unsigned short)GUI_BAR_GRE);
		dd_showbar(601, 36+(inv_pos * 18)/10, 11,22,(unsigned short)GUI_BAR_GRE);

		// display info-texts
		// HP, EN, MA below the skill list
		dd_xputtext(GUI_HP_COUNT_X,	GUI_HP_COUNT_Y,	1,"Hitpoints         %3d %3d",pl.a_hp,pl.hp[5]);
		dd_xputtext(GUI_EN_COUNT_X,	GUI_EN_COUNT_Y,	1,"Endurance         %3d %3d",pl.a_end,pl.end[5]);
		dd_xputtext(GUI_MP_COUNT_X,	GUI_MP_COUNT_Y,	1,"Mana              %3d %3d",pl.a_mana,pl.mana[5]);
		
		// display meta stats
		display_meta_from_ls();
		
		// Money, Update, Points
		dd_xputtext(GUI_MONEY_X,	GUI_MONEY_Y,	1,"Money");
		dd_xputtext(GUI_MONEY_X,	GUI_MONEY_Y+14,	1,"%9dG %2dS",pl.gold/100,pl.gold%100);
		dd_xputtext(GUI_UPDATE_X,	GUI_UPDATE_Y,	1,"Update");
		dd_xputtext(GUI_UPOINTS_X,	GUI_UPOINTS_Y,	1,"%7d",pl.points-stat_points_used);

		// WV, AV, EXP
		dd_xputtext(GUI_WV_X,   GUI_WV_Y,1,   "Weapon Value");
		dd_xputtext(GUI_WV_X+92,GUI_WV_Y,1,   "%11d",pl.weapon);
		dd_xputtext(GUI_WV_X,   GUI_WV_Y+14,1,"Armor Value");
		dd_xputtext(GUI_WV_X+92,GUI_WV_Y+14,1,"%11d",pl.armor);
		dd_xputtext(GUI_WV_X,   GUI_WV_Y+28,1,"Experience");
		dd_xputtext(GUI_WV_X+92,GUI_WV_Y+28,1,"%11d",pl.points_tot);

		// display spell shortcut buttons
		for (n=0; n<16; n++) {
			dd_xputtext(1086+(n%4)*49,591+(n/4)*15,1,pdata.xbutton[n].name);
		}

		for (n=0; n<5; n++) {
			dd_xputtext(9,8+n*14,1,"%-20.20s",at_name[n]);
			//
			if (pdata.show_stats) dd_xputtext(117,(8+n*14),3,"%3d",pl.attrib[n][0]+stat_raised[n]);
			dd_xputtext(140,(8+n*14),1,"%3d",pl.attrib[n][5]+stat_raised[n]);
			//
			if (attrib_needed(n,pl.attrib[n][0]+stat_raised[n])<=pl.points-stat_points_used) 
				dd_putc(163,8+n*14,1,'+');
			if (stat_raised[n]>0) 
				dd_putc(177,8+n*14,1,'-');
			if (attrib_needed(n,pl.attrib[n][0]+stat_raised[n])!=HIGH_VAL) 
				dd_xputtext(189,8+n*14,1,"%7d",attrib_needed(n,pl.attrib[n][0]+stat_raised[n]));
		}
		
		dd_xputtext(9,8+5*14,1,"Hitpoints");
		//
		if (pdata.show_stats) dd_xputtext(117,(8+5*14),3,"%3d",pl.hp[0]+stat_raised[5]);
		dd_xputtext(140,(8+5*14),1,"%3d",pl.hp[5]+stat_raised[5]);
		//
		if (hp_needed(pl.hp[0]+stat_raised[5])<=pl.points-stat_points_used)	
			dd_putc(163,8+5*14,1,'+');
		if (stat_raised[5]>0) 
			dd_putc(177,8+5*14,1,'-');
		if (hp_needed(pl.hp[0]+stat_raised[5])!=HIGH_VAL) 
			dd_xputtext(189,8+5*14,1,"%7d",hp_needed(pl.hp[0]+stat_raised[5]));
		
		/*
		dd_xputtext(9, 88,1,"Endurance         %3d",pl.end[5]+stat_raised[6]);
		if (end_needed(pl.end[0]+stat_raised[6])<=pl.points-stat_points_used) dd_putc(136,88,1,'+');
		if (stat_raised[6]>0) dd_putc(150,88,1,'-');
		if (end_needed(pl.end[0]+stat_raised[6])!=HIGH_VAL)	dd_xputtext(162,88,1,"%7d",end_needed(pl.end[0]+stat_raised[6]));
		*/
		
		dd_xputtext(9,8+6*14,1,"Mana");
		//
		if (pdata.show_stats) dd_xputtext(117,(8+6*14),3,"%3d",pl.mana[0]+stat_raised[7]);
		dd_xputtext(140,(8+6*14),1,"%3d",pl.mana[5]+stat_raised[7]);
		//
		if (mana_needed(pl.mana[0]+stat_raised[7])<=pl.points-stat_points_used)	
			dd_putc(163,8+6*14,1,'+');
		if (stat_raised[7]>0) 
			dd_putc(177,8+6*14,1,'-');
		if (mana_needed(pl.mana[0]+stat_raised[7])!=HIGH_VAL) 
			dd_xputtext(189,8+6*14,1,"%7d",mana_needed(pl.mana[0]+stat_raised[7]));
		
		// Player Flags from special items
		pl_flags = pl.worn[WN_FLAGS];
		pl_flagb = pl.worn_p[WN_FLAGS];
		
		for (n=0; n<10; n++) {
			m=skilltab[n+skill_pos].nr;
			if (!pl.skill[m][0]) {
				if (m==8||m==23||m==31||m==32) // Stealth, Resistance, Sense-Magic, Immunity -- these are all active even if you don't know them.
				{
					dd_xputtext(9,(8+8*14)+n*14,0,"%-20.20s",skilltab[n+skill_pos].name);
					dd_xputtext(140,(8+8*14)+n*14,0,"%3d",pl.skill[m][5]);
				}
				else
					dd_xputtext(9,(8+8*14)+n*14,1,"-");
				continue;
			}
			if (m==11&&(pl_flagb & (1 << 10))) 			// Magic Shield -> Magic Shell
				dd_xputtext(9,(8+8*14)+n*14,1,"%-20.20s","Magic Shell");
			else if (m==19&&(pl_flags & (1 <<  5)))	// Slow -> Greater Slow
				dd_xputtext(9,(8+8*14)+n*14,1,"%-20.20s","Greater Slow");
			else if (m==20&&(pl_flags & (1 <<  6)))	// Curse -> Greater Curse
				dd_xputtext(9,(8+8*14)+n*14,1,"%-20.20s","Greater Curse");
			else if (m==26&&(pl_flags & (1 << 14)))	// Heal -> Regen
				dd_xputtext(9,(8+8*14)+n*14,1,"%-20.20s","Regen");
			else if (m==41&&(pl_flags & (1 << 10)))	// Weaken -> Greater Weaken
				dd_xputtext(9,(8+8*14)+n*14,1,"%-20.20s","Greater Weaken");
			else
				dd_xputtext(9,(8+8*14)+n*14,1,"%-20.20s",skilltab[n+skill_pos].name);
			if (pdata.show_stats) dd_xputtext(117,(8+8*14)+n*14,3,"%3d",pl.skill[m][0]+stat_raised[n+8+skill_pos]);
			dd_xputtext(140,(8+8*14)+n*14,1,"%3d",pl.skill[m][5]+stat_raised[n+8+skill_pos]);
			if (skill_needed(m,pl.skill[m][0]+stat_raised[n+8+skill_pos])<=pl.points-stat_points_used) 
				dd_putc(163,(8+8*14)+n*14,1,'+');
			if (stat_raised[n+8+skill_pos]>0) 
				dd_putc(177,(8+8*14)+n*14,1,'-');
			if (skill_needed(m,pl.skill[m][0]+stat_raised[n+8+skill_pos])!=HIGH_VAL)
				dd_xputtext(189,(8+8*14)+n*14,1,"%7d",skill_needed(m,pl.skill[m][0]+stat_raised[n+8+skill_pos]));
		}
	}

	// experience bar
	if (pl.points_tot>0 && points2rank(pl.points_tot)<24) 
	{
		n=min(GUI_XPBAR_W,(
			pl.points_tot-rank2points(points2rank(pl.points_tot))
			)*GUI_XPBAR_W/(
			pl.points_tot+points_tolevel(pl.points_tot)-rank2points(points2rank(pl.points_tot))
			));
	}
	else 
	{
		n=0;
	}
	if (points2rank(pl.points_tot)==24)
		dd_showbar(GUI_XPBAR_X,GUI_XPBAR_Y,GUI_XPBAR_W,6,(unsigned short)GUI_BAR_EXP);
	else
	{
		dd_showbar(GUI_XPBAR_X,GUI_XPBAR_Y,GUI_XPBAR_W,6,(unsigned short)GUI_BAR_BLU);
		dd_showbar(GUI_XPBAR_X,GUI_XPBAR_Y,n,          6,(unsigned short)GUI_BAR_EXP);
	}
	//

	// logtext
	if (logtimer) logtimer--;
	else logstart=0;

	for (y=0; y<LL; y++) {
		dd_puttext(GUI_LOG_X,8+y*10,logfont[LL-y-1+logstart],logtext[LL-y-1+logstart]);
	}

	input[in_len]=0;
	if (cur_pos-view_pos>45) view_pos=cur_pos-45;
	if (cur_pos-5<view_pos)	view_pos=max(0,cur_pos-5);
	memcpy(buf,input+view_pos,48);
	buf[48]=0;

	dd_puttext(GUI_LOG_X,13+10*LL,1,buf);
	dd_putc(GUI_LOG_X+6*(cur_pos-view_pos),13+10*LL,1,127);

	if (init) {
		if (show_shop || show_wps || show_motd || show_newp || show_tuto) show_look=0;
		if (!show_look) {
			/*
			for (n=0; n<12; n++) 
			{
				if (pl.worn[wntab[n]]) 
				{
					if (hightlight==HL_EQUIPMENT && hightlight_sub==wntab[n])
						copyspritex(pl.worn[wntab[n]],303+(n%2)*35,2+(n/2)*35,16);
					else
						copyspritex(pl.worn[wntab[n]],303+(n%2)*35,2+(n/2)*35,0);
				}
				if (inv_block[wntab[n]]) copyspritex(4,303+(n%2)*35,2+(n/2)*35,0);
			}
			*/
			
			if (pl.worn[WN_CHARM2])
			{
				copyspritex(3+do_darkmode?18000:0, 775, 15,  0);
			}
			
			// Show your own gear
			for (n = 0; n < 13; n++)
			{
				if (pl.worn[n])
				{
					if (hightlight==HL_EQUIPMENT && hightlight_sub==n)
						copyspritex(pl.worn[n], gui_equ_x[n]+1, gui_equ_y[n]+1, 16);
					else
						copyspritex(pl.worn[n], gui_equ_x[n]+1, gui_equ_y[n]+1,  0);
					
					if (inv_block[n] && n==WN_CHARM2)
						copyspritex(4,          gui_equ_x[n]+1, gui_equ_y[n]+1,  0); 
				}
				// Get [X] sprite if we can't equip the slot
				if (inv_block[n] && n!=WN_CHARM2)
						copyspritex(4,          gui_equ_x[n]+1, gui_equ_y[n]+1,  0); 
			}
			//

			if (selected_char) tmp=lookup(selected_char,0);
			else tmp=pl.name;
			dd_xputtext(846+(125-strlen(tmp)*6)/2,32,1,tmp);


			// Bar for HP
			if (pl.hp[5]>0)	n=min(124,pl.hp[5]*62/pl.hp[5]);
			else n=0;
			dd_showbar(GUI_BAR_X,GUI_BAR_HP,n,6,(unsigned short)GUI_BAR_BLU);
			if (pl.hp[5]>0)	n=min(124,pl.a_hp*62/pl.hp[5]);
			else n=0;
			dd_showbar(GUI_BAR_X,GUI_BAR_HP,n,6,(unsigned short)GUI_BAR_GRE);

			// Bar for EN
			if (pl.end[5]>0) n=min(124,pl.end[5]*62/pl.end[5]);
			else n=0;
			dd_showbar(GUI_BAR_X,GUI_BAR_EN,n,6,(unsigned short)GUI_BAR_BLU);
			if (pl.end[5]>0) n=min(124,pl.a_end*62/pl.end[5]);
			else n=0;
			dd_showbar(GUI_BAR_X,GUI_BAR_EN,n,6,(unsigned short)GUI_BAR_GRE);

			// Bar for MP
			if (pl.mana[5]>0) n=min(124,pl.mana[5]*62/pl.mana[5]);
			else n=0;
			dd_showbar(GUI_BAR_X,GUI_BAR_MP,n,6,(unsigned short)GUI_BAR_BLU);
			if (pl.mana[5]>0) n=min(124,pl.a_mana*62/pl.mana[5]);
			else n=0;
			dd_showbar(GUI_BAR_X,GUI_BAR_MP,n,6,(unsigned short)GUI_BAR_GRE);
			
			if (!show_shop) {
				copyspritex(rank_sprite[points2rank(pl.points_tot)],935,42,0);
				copyspritex(plr_sprite,935-61,36,0);
				dd_xputtext(846+(125-strlen(pl.name)*6)/2,157,1,pl.name);
				dd_xputtext(846+(125-strlen(rank[points2rank(pl.points_tot)])*6)/2,176,1,rank[points2rank(pl.points_tot)]);
			}

		} else {
			if (look.worn[WN_CHARM2])
			{
				copyspritex(3+do_darkmode?18000:0, 775, 15,  0);
			}
			// Look at target gear
			for (n = 0; n < 13; n++)
			{
				if (look.worn[n]) copyspritex(look.worn[n], gui_equ_x[n]+1, gui_equ_y[n]+1, 0);
			}
			
			if (look.sprite) copyspritex(look.sprite,935-61,36,0);

			dd_xputtext(846+(125-strlen(rank[points2rank(look.points)])*6)/2,176,1,rank[points2rank(look.points)]);
			dd_xputtext(846+(125-strlen(look.name)*6)/2,157,1,look.name);
			
			// Bar for HP
			if (pl.hp[5]) n=min(124,look.hp*62/pl.hp[5]);
			else n=0;
			dd_showbar(GUI_BAR_X,GUI_BAR_HP,n,6,(unsigned short)GUI_BAR_BLU);
			if (pl.hp[5]) n=min(124,look.a_hp*62/pl.hp[5]);
			else n=0;
			dd_showbar(GUI_BAR_X,GUI_BAR_HP,n,6,(unsigned short)GUI_BAR_RED);
			
			// Bar for EN
			if (pl.end[5]) n=min(124,look.end*62/pl.end[5]);
			else n=0;
			dd_showbar(GUI_BAR_X,GUI_BAR_EN,n,6,(unsigned short)GUI_BAR_BLU);
			if (pl.end[5]) n=min(124,look.a_end*62/pl.end[5]);
			else n=0;
			dd_showbar(GUI_BAR_X,GUI_BAR_EN,n,6,(unsigned short)GUI_BAR_RED);

			// Bar for MP
			if (pl.mana[5])	n=min(124,look.mana*62/pl.mana[5]);
			else n=0;
			dd_showbar(GUI_BAR_X,GUI_BAR_MP,n,6,(unsigned short)GUI_BAR_BLU);
			if (pl.mana[5])	n=min(124,look.a_mana*62/pl.mana[5]);
			else n=0;
			dd_showbar(GUI_BAR_X,GUI_BAR_MP,n,6,(unsigned short)GUI_BAR_RED);

			copyspritex(rank_sprite[points2rank(look.points)],935,42,0);
		}

		if (show_shop) 
		{
			copyspritex(92+do_darkmode?18000:0,GUI_SHOP_X,GUI_SHOP_Y,0); // GUI element
			for (n=0; n<62; n++) {
				if (!shop.item[n]) continue;
				if (hightlight==HL_SHOP && hightlight_sub==n) 
				{
					copyspritex(shop.item[n],GUI_SHOP_X+2+(n%8)*35,GUI_SHOP_Y+2+(n/8)*35,16);
					if (shop.price[n]) 
					{
						if (show_shop==1)
							dd_xputtext(GUI_SHOP_X+5,GUI_SHOP_Y+299,1,"Sell: %dG %dS",shop.price[n]/100,shop.price[n]%100);
					}
				} 
				else copyspritex(shop.item[n],GUI_SHOP_X+2+(n%8)*35,GUI_SHOP_Y+2+(n/8)*35,0);
			}
			if (pl.citem && shop.pl_price)
			{
				if (show_shop==1)
					dd_xputtext(GUI_SHOP_X+5,GUI_SHOP_Y+299,1,"Buy:  %dG %dS",shop.pl_price/100,shop.pl_price%100);
				if (show_shop==2) // Black Stronghold
					dd_xputtext(GUI_SHOP_X+5,GUI_SHOP_Y+299,1,"Buy:  %d Points",shop.pl_price);
			}

			if (shop.sprite) copyspritex(shop.sprite,935-61,36,0);
			copyspritex(rank_sprite[points2rank(shop.points)],935,42,0);
			dd_xputtext(846+(125-strlen(rank[points2rank(shop.points)])*6)/2,176,1,rank[points2rank(shop.points)]);
			dd_xputtext(846+(125-strlen(shop.name)*6)/2,157,1,shop.name);
		}
		
		if (show_wps)
		{
			copyspritex(5+do_darkmode?18000:0,GUI_SHOP_X,GUI_SHOP_Y,0); // GUI element
			//wps_pos
			for (n=0; n<8; n++) {
				m=n+wps_pos;
				if (hightlight==HL_WAYPOINT && hightlight_sub==n)
					copyspritex((waypoints&(1<<m))?4500+m:4533+m,GUI_SHOP_X+2,GUI_SHOP_Y+2+n*35,16);
				else
					copyspritex((waypoints&(1<<m))?4500+m:4533+m,GUI_SHOP_X+2,GUI_SHOP_Y+2+n*35, 0);
				if (waypoints&(1<<m))
				{
					dd_xputtext(GUI_SHOP_X+74,GUI_SHOP_Y+ 7+n*35,1,wpslist[m].name);
					dd_xputtext(GUI_SHOP_X+74,GUI_SHOP_Y+18+n*35,1,wpslist[m].desc);
				}
				else
				{
					dd_xputtext(GUI_SHOP_X+74,GUI_SHOP_Y+ 7+n*35,1,"Unknown Location");
					dd_xputtext(GUI_SHOP_X+74,GUI_SHOP_Y+18+n*35,1,"\"???\"");
				}
			}
			// Scroll bar
			dd_showbar(GUI_SHOP_X+269, GUI_SHOP_Y+36+(wps_pos*176)/(MAXWPS-8),11,33,(unsigned short)GUI_BAR_GRE);
		}
	}
}

struct looks {
	char known;
	char name[21];
	char proz;
	unsigned short id;
};

struct looks *looks=NULL;
int lookmax=0;
int lookat=0;

char *lookup(int nr,unsigned short id)
{
	static char buf[40];
	int n;

	if (nr>=lookmax) {
		looks=realloc(looks,sizeof(struct looks)*(nr+10));
		for (n=lookmax; n<nr+10; n++) {
			strcpy(looks[n].name,"");
			looks[n].known=0;
			looks[n].proz=0;
		}
		lookmax=nr+10;
	}

	if (id && id!=looks[nr].id) {
		looks[nr].known=0;
		looks[nr].name[0]=0;
		looks[nr].proz=0;
		looks[nr].id=id;
	}

	if (!looks[nr].known) lookat=nr;

	if (!id) return looks[nr].name;

	if (pdata.show_names && pdata.show_proz) 
	{
		if (looks[nr].proz) 
		{
			sprintf(buf,"%s %d%%",looks[nr].name,looks[nr].proz);
			return buf;
		} 
		else return looks[nr].name;
	} 
	else if (pdata.show_names) return looks[nr].name;
	else if (pdata.show_proz) 
	{
		if (looks[nr].proz) 
		{
			sprintf(buf,"%d%%",looks[nr].proz);
			return buf;
		} 
		else return "";
	} 
	else return "";
}

void add_look(unsigned short nr,char *name,unsigned short id)
{
	int n;

	if (nr>=lookmax) {
		looks=realloc(looks,sizeof(struct looks)*(nr+10));
		for (n=lookmax; n<nr+10; n++) {
			strcpy(looks[n].name,"");
			looks[n].known=0;
			looks[n].proz=0;
		}
		lookmax=nr+10;
	}

	if (id!=looks[nr].id) {
		looks[nr].known=0;
		looks[nr].name[0]=0;
		looks[nr].proz=0;
	}

	strncpy(looks[nr].name,name,16);
	looks[nr].name[16]=0;
	looks[nr].known=1;
	looks[nr].proz=0;
	looks[nr].id=id;
}

void set_look_proz(unsigned short nr,unsigned short id,int proz)
{
	int n;

	if (nr>=lookmax) {
		looks=realloc(looks,sizeof(struct looks)*(nr+10));
		for (n=lookmax; n<nr+10; n++) {
			strcpy(looks[n].name,"");
			looks[n].known=0;
		}
		lookmax=nr+10;
	}
	if (id!=looks[nr].id) {
		looks[nr].known=0;
		looks[nr].name[0]=0;
		looks[nr].proz=0;
		looks[nr].id=id;
	}
	looks[nr].proz=(unsigned char)proz;
}

int tile_x=-1,tile_y=-1,tile_type=-1;

void dd_show_map(unsigned short *src,int xo,int yo,int magnify);

int autohide(int x,int y)
{
	if (x>=(screen_renderdist/2) || (y<=screen_renderdist/2)) return 0;
	return 1;
}

int facing(int x,int y,int dir)
{
	if (dir==1 && x==screen_renderdist/2+1 && y==screen_renderdist/2) return 1;
	if (dir==2 && x==screen_renderdist/2-1 && y==screen_renderdist/2) return 1;
	if (dir==4 && x==screen_renderdist/2 && y==screen_renderdist/2+1) return 1;
	if (dir==3 && x==screen_renderdist/2 && y==screen_renderdist/2-1) return 1;

	return 0;
}

int mapxy_rand(int x,int y,int dur)
{
    int val,tim;

    val=(x*43+y*77+x*y+x*24+y*39)%666;
    tim=(ticker/dur)%666;

    if (tim==val) return ticker%dur;
    else return 0;
}

void display_floortile(int tile,int light,int x,int y,int xoff,int yoff,int mx,int my)
{
    switch(tile) {
	case 16980:	tile+=mapxy_rand(mx,my,10)/2; break;
    }
    copysprite(tile,light,x,y,xoff,yoff);
}

void eng_display(int init)	// optimize me!!!!!
{
	int x,y,rx,ry,m,plr_sprite,tmp,mapx,mapy,selected_visible=0,alpha,alphastr,txtclr=1;
	extern int dd_cache_hit,dd_cache_miss,swap,MAXCACHE;
	static unsigned short xmap[MAPX_MAX*MAPY_MAX];
	static xm_flag=1;

	if (xm_flag) {
		for (m=0; m<MAPX_MAX*MAPY_MAX; m++)	xmap[m]=0;
		xm_flag=0;
	}

	// check if we're visible. If not, just leave.
	if (!dd_isvisible()) return;

	mouse(mx,my,0);
	SetCursor(cursor[cursor_type]);

	// *******
	// * map *
	// *******

	if (init) {
		if (do_shadow) dd_shadow_clear();
		xoff=-map[(screen_renderdist/2)+(screen_renderdist/2)*screen_renderdist].obj_xoff-176; //-176;
		yoff=-map[(screen_renderdist/2)+(screen_renderdist/2)*screen_renderdist].obj_yoff; //-176;
		plr_sprite=map[(screen_renderdist/2)+(screen_renderdist/2)*screen_renderdist].obj2;

		mapx=map[(screen_renderdist/2)+(screen_renderdist/2)*screen_renderdist].x;
		mapy=map[(screen_renderdist/2)+(screen_renderdist/2)*screen_renderdist].y;

		for (y=screen_renderdist-1; y>=0; y--) {
			for (x=0; x<screen_renderdist; x++) {

				// background
				m=x+y*screen_renderdist;

				if (hightlight==HL_MAP && tile_type==0 && tile_x==x && tile_y==y) tmp=16;
				else tmp=0;
				if (map[m].flags&INVIS)	tmp|=64;
				if (map[m].flags&INFRARED) tmp|=256;
				if (map[m].flags&UWATER) tmp|=512;

				display_floortile(map[m].back,map[m].light|tmp,x*32,y*32,xoff,yoff,map[x+y*screen_renderdist].x,map[x+y*screen_renderdist].y);

				if (map[m].x<MAPX_MAX && map[m].y<MAPY_MAX && !(map[m].flags&INVIS)) {
					if (!xmap[map[m].y+map[m].x*MAPX_MAX] || xmap[map[m].y+map[m].x*MAPX_MAX]==0xffff)

						xmap[map[m].y+map[m].x*MAPX_MAX]=(unsigned short)get_avgcol(map[m].back);
				}

				if (pl.goto_x==map[m].x && pl.goto_y==map[m].y)
					copysprite(31,0,x*32,y*32,xoff,yoff);
			}
		}

		for (y=screen_renderdist-1; y>=0; y--) {
			for (x=0; x<screen_renderdist; x++) {

				if (	(y <= 5 && x >= screen_renderdist-9) 
					|| 	(y <= 6 && x >= screen_renderdist-8)
					|| 	(y <= 7 && x >= screen_renderdist-7)
					|| 	(y <= 8 && x >= screen_renderdist-6)
					|| 	(y <= 9 && x >= screen_renderdist-5)) 
				continue; // x+, y-

				m=x+y*screen_renderdist;

				if (map[x+y*screen_renderdist].flags&INVIS) continue; //tmp=128;
				else tmp=0;

				if (map[m].flags&INFRARED) tmp|=256;
				if (map[m].flags&UWATER) tmp|=512;

				// object
                if (pdata.hide==0 || (map[m].flags&ISITEM) || autohide(x,y)) {
					int tmp2;

					if (map[m].obj1>16335 && map[m].obj1<16422 && map[m].obj1!=16357 &&
						map[m].obj1!=16365 && map[m].obj1!=16373 && map[m].obj1!=16381 &&
						map[m].obj1!=16357 && map[m].obj1!=16389 && map[m].obj1!=16397 &&
						map[m].obj1!=16405 && map[m].obj1!=16413 && map[m].obj1!=16421 &&
						!facing(x,y,pl.dir) && !autohide(x,y) && pdata.hide) { // mine hack

						if (map[m].obj1<16358) tmp2=457;
						else if (map[m].obj1<16366)	tmp2=456;
						else if (map[m].obj1<16374)	tmp2=455;
						else if (map[m].obj1<16382)	tmp2=466;
						else if (map[m].obj1<16390)	tmp2=459;
						else if (map[m].obj1<16398)	tmp2=458;
						else if (map[m].obj1<16398)	tmp2=449;
						else if (map[m].obj1<16406)	tmp2=468;
						else tmp2=467;

						if (hightlight==HL_MAP && tile_type==1 && tile_x==x && tile_y==y) 
							copysprite(tmp2,map[m].light|16|tmp,x*32,y*32,xoff,yoff);
						else 
							copysprite(tmp2,map[m].light|tmp,x*32,y*32,xoff,yoff);
					} else {
						if (hightlight==HL_MAP && tile_type==1 && tile_x==x && tile_y==y) 
							copysprite(map[m].obj1,map[m].light|16|tmp,x*32,y*32,xoff,yoff);
						else 
							copysprite(map[m].obj1,map[m].light|tmp,x*32,y*32,xoff,yoff);
					}					

				} else if (map[m].obj1) {					
					copysprite(map[m].obj1+1,map[m].light|tmp,x*32,y*32,xoff,yoff);					
				}

				if (map[m].obj1 && map[m].x<MAPX_MAX && map[m].y<MAPY_MAX) {
					xmap[map[m].y+map[m].x*MAPX_MAX]=(unsigned short)get_avgcol(map[m].obj1);
				}

				// character
				if (tile_type==2 && tile_x==x && tile_y==y)	tmp=16;
				else tmp=0;
				if (map[m].ch_nr==selected_char) {
					tmp|=32; selected_visible=1;
				}
				if (map[m].flags&INVIS)	tmp|=64;
				if (map[m].flags&STONED) tmp|=128;
				if (map[m].flags&INFRARED) tmp|=256;
				if (map[m].flags&UWATER) tmp|=512;

				if (do_shadow) dd_shadow(map[m].obj2,x*32,y*32,xoff+map[m].obj_xoff,yoff+map[m].obj_yoff+4);
				copysprite(map[m].obj2,map[m].light|tmp,x*32,y*32,xoff+map[m].obj_xoff,yoff+map[m].obj_yoff);				

				if (pl.attack_cn && pl.attack_cn==map[m].ch_nr)
					copysprite(34,0,x*32,y*32,xoff+map[m].obj_xoff,yoff+map[m].obj_yoff);

				if (pl.misc_action==DR_GIVE && pl.misc_target1==map[m].ch_id)
					copysprite(45,0,x*32,y*32,xoff+map[m].obj_xoff,yoff+map[m].obj_yoff);
				
				if ((pdata.show_names|pdata.show_proz) && map[m].ch_nr) 
				{
					set_look_proz(map[m].ch_nr,map[m].ch_id,map[m].ch_proz);
					if (map[m].ch_fontcolor>=0 && map[m].ch_fontcolor<=3)
					{
						// Colors 1 and 0 are flip-flopped in case the color can't be pulled from the server.
						if (map[m].ch_fontcolor==0)
							txtclr = 1;
						else if (map[m].ch_fontcolor==1)
							txtclr = 0;
						else
							txtclr = map[m].ch_fontcolor;
					}
					dd_gputtext(x*32,y*32,txtclr,lookup(map[m].ch_nr,map[m].ch_id),xoff+map[m].obj_xoff,yoff+map[m].obj_yoff-8);
				}
				
				// Healthbar over characters
				if (pdata.show_bars && map[m].ch_nr) 
				{
					set_look_proz(map[m].ch_nr,map[m].ch_id,map[m].ch_proz);
					rx=((x*32)/2)+((y*32)/2)+32-HPBAR_WIDTH/2+screen_tilexoff-((screen_renderdist-34)/2*32)+xoff+map[m].obj_xoff;
					ry=((x*32)/4)-((y*32)/4)+screen_tileyoff-60+yoff+map[m].obj_yoff;
					
					if (looks[map[m].ch_nr].proz) 
					{
						dd_showbar(rx-1,ry-1,HPBAR_WIDTH+2,4,(unsigned short)CHAR_BAR_BL);
						dd_showbar(rx,ry,HPBAR_WIDTH,2,(unsigned short)CHAR_BAR_RD);
					}
					dd_showbar(rx,ry,(int)(HPBAR_WIDTH*((float)looks[map[m].ch_nr].proz/100.0)),2,(unsigned short)CHAR_BAR_HP);
				}
				
				if (pl.misc_action==DR_DROP && pl.misc_target1==map[m].x && pl.misc_target2==map[m].y)
					copysprite(32,0,x*32,y*32,xoff,yoff);
				if (pl.misc_action==DR_PICKUP && pl.misc_target1==map[m].x && pl.misc_target2==map[m].y)
					copysprite(33,0,x*32,y*32,xoff,yoff);
				if (pl.misc_action==DR_USE && pl.misc_target1==map[m].x && pl.misc_target2==map[m].y)
					copysprite(45,0,x*32,y*32,xoff,yoff);

				// effects
				if (map[m].flags2&MF_MOVEBLOCK)	copysprite(55,0,x*32,y*32,xoff,yoff);
				if (map[m].flags2&MF_SIGHTBLOCK) copysprite(84,0,x*32,y*32,xoff,yoff);
				if (map[m].flags2&MF_INDOORS) copysprite(56,0,x*32,y*32,xoff,yoff);
				if (map[m].flags2&MF_UWATER) copysprite(75,0,x*32,y*32,xoff,yoff);
				if (map[m].flags2&MF_NOFIGHT) copysprite(58,0,x*32,y*32,xoff,yoff);
				if (map[m].flags2&MF_NOMONST) copysprite(59,0,x*32,y*32,xoff,yoff);
				if (map[m].flags2&MF_BANK) copysprite(60,0,x*32,y*32,xoff,yoff);
				if (map[m].flags2&MF_TAVERN) copysprite(61,0,x*32,y*32,xoff,yoff);
				if (map[m].flags2&MF_NOMAGIC) copysprite(62,0,x*32,y*32,xoff,yoff);
				if (map[m].flags2&MF_DEATHTRAP)	copysprite(73,0,x*32,y*32,xoff,yoff);
				if (map[m].flags2&MF_NOLAG)	copysprite(57,0,x*32,y*32,xoff,yoff);
				if (map[m].flags2&MF_ARENA)	copysprite(76,0,x*32,y*32,xoff,yoff);
//				if (map[m].flags2&MF_TELEPORT2) copysprite(77,0,x*32,y*32,xoff,yoff);
				if (map[m].flags2&MF_NOEXPIRE) copysprite(82,0,x*32,y*32,xoff,yoff);
				if (map[m].flags2&0x80000000) copysprite(72,0,x*32,y*32,xoff,yoff);

				if ((map[m].flags&(INJURED|INJURED1|INJURED2))==INJURED)
					copysprite(1079,0,x*32,y*32,xoff+map[m].obj_xoff,yoff+map[m].obj_yoff);
				if ((map[m].flags&(INJURED|INJURED1|INJURED2))==(INJURED|INJURED1))
					copysprite(1080,0,x*32,y*32,xoff+map[m].obj_xoff,yoff+map[m].obj_yoff);
				if ((map[m].flags&(INJURED|INJURED1|INJURED2))==(INJURED|INJURED2))
					copysprite(1081,0,x*32,y*32,xoff+map[m].obj_xoff,yoff+map[m].obj_yoff);
				if ((map[m].flags&(INJURED|INJURED1|INJURED2))==(INJURED|INJURED1|INJURED2))
					copysprite(1082,0,x*32,y*32,xoff+map[m].obj_xoff,yoff+map[m].obj_yoff);

				if (map[m].flags&DEATH) {
					if (map[m].obj2) copysprite(280+((map[m].flags&DEATH)>>17)-1,0,x*32,y*32,xoff+map[m].obj_xoff,yoff+map[m].obj_yoff);
					else copysprite(280+((map[m].flags&DEATH)>>17)-1,0,x*32,y*32,xoff,yoff);
				}
				if (map[m].flags&TOMB) {
					copysprite(240+((map[m].flags&TOMB)>>12)-1,map[m].light,x*32,y*32,xoff,yoff);
				}

				alpha=0; alphastr=0;

				if (map[m].flags&EMAGIC) {
					alpha|=1;
					alphastr=max((unsigned)alphastr,((map[m].flags&EMAGIC)>>22));
				}

				if (map[m].flags&GMAGIC) {
					alpha|=2;
					alphastr=max((unsigned)alphastr,((map[m].flags&GMAGIC)>>25));
				}

				if (map[m].flags&CMAGIC) {
					alpha|=4;
					alphastr=max((unsigned)alphastr,((map[m].flags&CMAGIC)>>28));
				}
				if (alpha) dd_alphaeffect_magic(alpha,alphastr,x*32,y*32,xoff+map[m].obj_xoff,yoff+map[m].obj_yoff);
			}
		}
	} else {
		for (y=screen_renderdist-1; y>=0; y--) {
			for (x=0; x<screen_renderdist; x++) {
				// background
				copysprite(SPR_EMPTY,0,x*32,y*32,-176,0);
			}
		}
	}

	if (!selected_visible) selected_char=0;

	copyspritex(1+do_darkmode?18000:0,0,0,0);

	if (init) {
		// Store your position on the map as a white pixel
		xmap[mapy+mapx*MAPX_MAX]=0xffff;
		
		// offsets the minimap draw, so that it begins drawing from 64 tiles to the upper-left of your position,
		//   drawing 128 x 128 tiles as pixels in dd_show_map.
		// Added to this, mm_magnify carries a magnification value, which shrinks the tile offsets
		//   and increases the size of the drawn 'pixels' to fill the 128x128 space.
		mapx=mapx-(64/max(1,mm_magnify));
		if (mapx<0)	mapx=0;
		if (mapx>MAPX_MAX-((128/max(1,mm_magnify))+1)) mapx=MAPX_MAX-((128/max(1,mm_magnify))+1);

		mapy=mapy-(64/max(1,mm_magnify));
		if (mapy<0)	mapy=0;
		if (mapy>MAPY_MAX-((128/max(1,mm_magnify))+1)) mapy=MAPY_MAX-((128/max(1,mm_magnify))+1);

		dd_show_map(xmap,mapy,mapx,max(1,mm_magnify));
	}

	eng_display_win(plr_sprite,init);

	// ********************
	// display cursors etc.
	// ********************

	if (init && pl.citem) {
		if (cursor_type==CT_DROP || cursor_type==CT_SWAP || cursor_type==CT_USE)
			copyspritex(pl.citem,mouse_x-16,mouse_y-16,16);
		else
			copyspritex(pl.citem,mouse_x-16,mouse_y-16,0);
	}
}

// DISPLAY: TEXT OUTPUT

void tlog(char *text,char font)
{
	int n,panic=0;
	static int flag=0;

	if (!flag) {
		for (n=0; n<XLL*60; n++) {
			logtext[0][n]=0;
		}
		for (n=0; n<XLL; n++) {
			logfont[n]=0;
		}
		flag=1;
	}

	if (strlen(text)<1)	return;

	while (panic++<XLL) {
		do_msg();
		memmove(logtext[1],logtext[0],XLL*60-60);
		memmove(&logfont[1],&logfont[0],XLL-1);
		memcpy(logtext[0],text,min(60-1,strlen(text)+1));
		logfont[0]=font;
		logtext[0][60-1]=0;
		if (strlen(text)<XS-1) return;
		for (n=XS-1; n>0; n--) if (logtext[0][n]==' ') break;
		if (n!=0) {
			logtext[0][n]=0; text+=n+1;
		} else text+=XS-1;
	}
}

void xlog(char font,char *format,...)
{
	va_list args;
	char buf[1024];

	va_start(args,format);
	vsprintf(buf,format,args);
	tlog(buf,font);
	va_end(args);
}


// ************* MAIN *********************

void init_engine(void)
{
	eng_init_map();
	eng_init_player();
}

void do_msg(void)
{
	MSG msg;

	if (PeekMessage(&msg,NULL,0,0,PM_REMOVE)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}

void eng_flip(unsigned int t)
{
	int diff;
	MSG msg;

	diff=t-GetTickCount();
	if (diff>0)	idle+=diff;

	do {
		if (PeekMessage(&msg,NULL,0,0,PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		} else Sleep(1);
	} while (t>GetTickCount());

	if (screen_windowed == 1) {
		dd_flip_windowed();
	} else {
		dd_flip();
	}

	frame++;
}

// Ultra big speed table. I recommend just collapsing this bracket (if you can stand the lag when scrolling), it is massive.
unsigned char speedtab[300][200]=
{
//   1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0,1,2,3,4... etc
	{1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2},
	{1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,1,2},
	{1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,1,2},
	{1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,1,2},
	{1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,1,2},
	{1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,1,2},
	{1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,1,2},
	{1,2,1,2,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,2,1,2,1,1,2},
	{1,2,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,2,1,1,2},
	{1,2,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,2,1,1,2},
	{1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,1,2},
	{1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,1,2},
	{1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,1,2},
	{1,2,1,2,1,2,1,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,1,2,1,2,1,2,1,1,2},
	{1,2,1,2,1,2,1,1,2,1,2,1,2,1,1,2,1,2,1,2,1,1,2,1,2,1,2,1,1,2,1,2,1,2,1,1,2,1,2,1,2,1,1,2,1,2,1,2,1,1,2,1,2,1,2,1,1,2,1,2,1,2,1,1,2,1,2,1,2,1,1,2,1,2,1,2,1,1,2,1,2,1,2,1,1,2,1,2,1,2,1,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,1,2,1,2,1,2,1,1,2,1,2,1,2,1,1,2,1,2,1,2,1,1,2,1,2,1,2,1,1,2,1,2,1,2,1,1,2,1,2,1,2,1,1,2,1,2,1,2,1,1,2,1,2,1,2,1,1,2,1,2,1,2,1,1,2,1,2,1,2,1,1,2,1,2,1,2,1,1,2,1,2,1,2,1,1,2},
	{1,2,1,2,1,1,2,1,2,1,2,1,1,2,1,2,1,2,1,1,2,1,2,1,2,1,1,2,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,2,1,1,2,1,2,1,2,1,1,2,1,2,1,2,1,1,2,1,2,1,2,1,1,2,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,2,1,1,2,1,2,1,2,1,1,2,1,2,1,2,1,1,2,1,2,1,2,1,1,2,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,2,1,1,2,1,2,1,2,1,1,2,1,2,1,2,1,1,2,1,2,1,2,1,1,2,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,2,1,1,2,1,2,1,2,1,1,2,1,2,1,2,1,1,2,1,2,1,2,1,1,2,1,2,1,2,1,1,2,1,2,1,1,2},
	{1,2,1,2,1,1,2,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,2,1,1,2,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,2,1,1,2,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,2,1,1,2,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,2,1,1,2,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,2,1,1,2,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,2,1,1,2,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,2,1,1,2,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,2,1,1,2,1,2,1,1,2},
	{1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2},
	{1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2},
	{1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2},
	{1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2},
	{1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,1,2},
	{1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,1,2},
	{1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,1,2},
	{1,2,1,1,2,1,2,1,1,2,1,1,2,1,2,1,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,1,2,1,2,1,1,2,1,1,2,1,2,1,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,1,2,1,2,1,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,1,2,1,2,1,1,2,1,1,2,1,2,1,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,1,2,1,2,1,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,1,2,1,2,1,1,2,1,1,2,1,2,1,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,1,2,1,2,1,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,1,2,1,2,1,1,2,1,1,2,1,2,1,1,2,1,1,2},
	{1,2,1,1,2,1,1,2,1,2,1,1,2,1,1,2,1,2,1,1,2,1,1,2,1,2,1,1,2,1,1,2,1,2,1,1,2,1,1,2,1,2,1,1,2,1,1,2,1,2,1,1,2,1,1,2,1,2,1,1,2,1,1,2,1,2,1,1,2,1,1,2,1,2,1,1,2,1,1,2,1,2,1,1,2,1,1,2,1,2,1,1,2,1,1,2,1,2,1,1,2,1,1,2,1,2,1,1,2,1,1,2,1,2,1,1,2,1,1,2,1,2,1,1,2,1,1,2,1,2,1,1,2,1,1,2,1,2,1,1,2,1,1,2,1,2,1,1,2,1,1,2,1,2,1,1,2,1,1,2,1,2,1,1,2,1,1,2,1,2,1,1,2,1,1,2,1,2,1,1,2,1,1,2,1,2,1,1,2,1,1,2,1,2,1,1,2,1,1,2},
	{1,2,1,1,2,1,1,2,1,2,1,1,2,1,1,2,1,2,1,1,2,1,1,2,1,1,2,1,2,1,1,2,1,1,2,1,2,1,1,2,1,1,2,1,2,1,1,2,1,1,2,1,1,2,1,2,1,1,2,1,1,2,1,2,1,1,2,1,1,2,1,2,1,1,2,1,1,2,1,1,2,1,2,1,1,2,1,1,2,1,2,1,1,2,1,1,2,1,1,2,1,2,1,1,2,1,1,2,1,2,1,1,2,1,1,2,1,2,1,1,2,1,1,2,1,1,2,1,2,1,1,2,1,1,2,1,2,1,1,2,1,1,2,1,2,1,1,2,1,1,2,1,1,2,1,2,1,1,2,1,1,2,1,2,1,1,2,1,1,2,1,2,1,1,2,1,1,2,1,1,2,1,2,1,1,2,1,1,2,1,2,1,1,2,1,1,2,1,1,2},
	{1,2,1,1,2,1,1,2,1,2,1,1,2,1,1,2,1,1,2,1,2,1,1,2,1,1,2,1,1,2,1,2,1,1,2,1,1,2,1,1,2,1,2,1,1,2,1,1,2,1,1,2,1,2,1,1,2,1,1,2,1,1,2,1,2,1,1,2,1,1,2,1,2,1,1,2,1,1,2,1,1,2,1,2,1,1,2,1,1,2,1,1,2,1,2,1,1,2,1,1,2,1,1,2,1,2,1,1,2,1,1,2,1,1,2,1,2,1,1,2,1,1,2,1,1,2,1,2,1,1,2,1,1,2,1,2,1,1,2,1,1,2,1,1,2,1,2,1,1,2,1,1,2,1,1,2,1,2,1,1,2,1,1,2,1,1,2,1,2,1,1,2,1,1,2,1,1,2,1,2,1,1,2,1,1,2,1,1,2,1,2,1,1,2,1,1,2,1,1,2},
	{1,2,1,1,2,1,1,2,1,1,2,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,2,1,1,2,1,1,2,1,1,2,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,2,1,1,2,1,1,2,1,1,2,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,2,1,1,2,1,1,2,1,1,2,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,2,1,1,2,1,1,2,1,1,2,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,2,1,1,2,1,1,2,1,1,2,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,2,1,1,2,1,1,2,1,1,2,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,2,1,1,2,1,1,2,1,1,2,1,2,1,1,2,1,1,2,1,1,2,1,1,2},
	{1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2},
	{1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2},
	{1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2},
	{1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2},
	{1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2},
	{1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,1,2},
	{1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,1,2},
	{1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,1,2},
	{1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,1,2},
	{1,1,2,1,1,2,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,2,1,1,2,1,1,1,2},
	{1,1,2,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,2,1,1,1,2},
	{1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,2,1,1,1,2},
	{1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,1,2},
	{1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,1,2},
	{1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,1,2},
	{1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,1,2},
	{1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,1,2},
	{1,1,2,1,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2},
	{1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2},
	{1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2},
	{1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2},
	{1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2},
	{1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,1,2},
	{1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,1,2},
	{1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,1,2},
	{1,1,1,2,1,1,1,2,1,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,1,2,1,1,1,2,1,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,1,2,1,1,1,2,1,1,1,1,2},
	{1,1,1,2,1,1,1,2,1,1,1,1,2,1,1,1,2,1,1,1,1,2,1,1,1,2,1,1,1,1,2,1,1,1,2,1,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,1,2,1,1,1,2,1,1,1,1,2,1,1,1,2,1,1,1,1,2,1,1,1,2,1,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,1,2,1,1,1,2,1,1,1,1,2,1,1,1,2,1,1,1,1,2,1,1,1,2,1,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,1,2,1,1,1,2,1,1,1,1,2,1,1,1,2,1,1,1,1,2,1,1,1,2,1,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,1,2,1,1,1,2,1,1,1,1,2,1,1,1,2,1,1,1,1,2,1,1,1,2,1,1,1,1,2},
	{1,1,1,2,1,1,1,1,2,1,1,1,2,1,1,1,1,2,1,1,1,2,1,1,1,1,2,1,1,1,2,1,1,1,1,2,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,2,1,1,1,1,2,1,1,1,2,1,1,1,1,2,1,1,1,2,1,1,1,1,2,1,1,1,2,1,1,1,1,2,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,2,1,1,1,1,2,1,1,1,2,1,1,1,1,2,1,1,1,2,1,1,1,1,2,1,1,1,2,1,1,1,1,2,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,2,1,1,1,1,2,1,1,1,2,1,1,1,1,2,1,1,1,2,1,1,1,1,2,1,1,1,2,1,1,1,1,2,1,1,1,2,1,1,1,1,2,1,1,1,1,2},
	{1,1,1,2,1,1,1,1,2,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,2,1,1,1,1,2,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,2,1,1,1,1,2,1,1,1,1,2},
	{1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,1,2},
	{1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,1,2},
	{1,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,1,2},
	{1,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,1,1,2},
	{1,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,1,1,2},
	{1,1,1,1,2,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,2,1,1,1,1,1,2},
	{1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,2},
	{1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,2},
	{1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,2},
	{1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,1,2},
	{1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,1,2},
	{1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,1,2},
	{1,1,1,1,1,2,1,1,1,1,1,1,2,1,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,1,2,1,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,1,2,1,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,1,2,1,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,1,2,1,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,1,2,1,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,1,2,1,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,1,2,1,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,1,2,1,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,1,2,1,1,1,1,1,1,2},
	{1,1,1,1,1,2,1,1,1,1,1,1,2,1,1,1,1,1,1,2,1,1,1,1,1,1,2,1,1,1,1,1,1,2,1,1,1,1,1,1,2,1,1,1,1,1,1,2,1,1,1,1,1,1,2,1,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,1,2,1,1,1,1,1,1,2,1,1,1,1,1,1,2,1,1,1,1,1,1,2,1,1,1,1,1,1,2,1,1,1,1,1,1,2,1,1,1,1,1,1,2,1,1,1,1,1,1,2,1,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,1,2,1,1,1,1,1,1,2,1,1,1,1,1,1,2,1,1,1,1,1,1,2,1,1,1,1,1,1,2,1,1,1,1,1,1,2,1,1,1,1,1,1,2,1,1,1,1,1,1,2,1,1,1,1,1,1,2},
	{1,1,1,1,1,1,2,1,1,1,1,1,1,2,1,1,1,1,1,1,2,1,1,1,1,1,1,2,1,1,1,1,1,1,2,1,1,1,1,1,1,2,1,1,1,1,1,1,1,2,1,1,1,1,1,1,2,1,1,1,1,1,1,2,1,1,1,1,1,1,2,1,1,1,1,1,1,2,1,1,1,1,1,1,2,1,1,1,1,1,1,2,1,1,1,1,1,1,1,2,1,1,1,1,1,1,2,1,1,1,1,1,1,2,1,1,1,1,1,1,2,1,1,1,1,1,1,2,1,1,1,1,1,1,2,1,1,1,1,1,1,2,1,1,1,1,1,1,1,2,1,1,1,1,1,1,2,1,1,1,1,1,1,2,1,1,1,1,1,1,2,1,1,1,1,1,1,2,1,1,1,1,1,1,2,1,1,1,1,1,1,2,1,1,1,1,1,1,1,2},
	{1,1,1,1,1,1,2,1,1,1,1,1,1,2,1,1,1,1,1,1,1,2,1,1,1,1,1,1,2,1,1,1,1,1,1,1,2,1,1,1,1,1,1,2,1,1,1,1,1,1,2,1,1,1,1,1,1,1,2,1,1,1,1,1,1,2,1,1,1,1,1,1,1,2,1,1,1,1,1,1,2,1,1,1,1,1,1,2,1,1,1,1,1,1,1,2,1,1,1,1,1,1,2,1,1,1,1,1,1,1,2,1,1,1,1,1,1,2,1,1,1,1,1,1,2,1,1,1,1,1,1,1,2,1,1,1,1,1,1,2,1,1,1,1,1,1,1,2,1,1,1,1,1,1,2,1,1,1,1,1,1,2,1,1,1,1,1,1,1,2,1,1,1,1,1,1,2,1,1,1,1,1,1,1,2,1,1,1,1,1,1,2,1,1,1,1,1,1,1,2},
	{1,1,1,1,1,1,2,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,2,1,1,1,1,1,1,2,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,2,1,1,1,1,1,1,2,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,2,1,1,1,1,1,1,2,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,2,1,1,1,1,1,1,2,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,2,1,1,1,1,1,1,2,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,2,1,1,1,1,1,1,2,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,2,1,1,1,1,1,1,2,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,2},
	{1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,2},
	{1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,2},
	{1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,2},
	{1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,2},
	{1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,2},
	{1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,2},
	{1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,2},
	{1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,2},
	{1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,2},
	{1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,2},
	{1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,2},
	{1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2},
	{1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2},
	{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2},
	{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2},
	{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2},
	{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2},
	{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2},
	{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2},
	{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2},
	{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2},
	{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2},
	{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2},
	{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2},
	{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2},
	{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
	{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1},
	{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1},
	{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1},
	{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1},
	{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1},
	{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1},
	{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1},
	{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1},
	{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1},
	{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1},
	{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1},
	{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1},
	{1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1},
	{1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1},
	{1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,0,1},
	{1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,0,1},
	{1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,0,1},
	{1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,0,1},
	{1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,0,1},
	{1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,0,1},
	{1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,0,1},
	{1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,0,1},
	{1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,0,1},
	{1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,0,1},
	{1,1,1,1,1,1,0,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,0,1},
	{1,1,1,1,1,1,0,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,0,1,1,1,1,1,1,0,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,0,1,1,1,1,1,1,0,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,0,1,1,1,1,1,1,0,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,0,1,1,1,1,1,1,0,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,0,1,1,1,1,1,1,0,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,0,1,1,1,1,1,1,0,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,0,1,1,1,1,1,1,0,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,0,1,1,1,1,1,1,0,1},
	{1,1,1,1,1,1,0,1,1,1,1,1,1,0,1,1,1,1,1,1,1,0,1,1,1,1,1,1,0,1,1,1,1,1,1,1,0,1,1,1,1,1,1,0,1,1,1,1,1,1,0,1,1,1,1,1,1,1,0,1,1,1,1,1,1,0,1,1,1,1,1,1,1,0,1,1,1,1,1,1,0,1,1,1,1,1,1,0,1,1,1,1,1,1,1,0,1,1,1,1,1,1,0,1,1,1,1,1,1,1,0,1,1,1,1,1,1,0,1,1,1,1,1,1,0,1,1,1,1,1,1,1,0,1,1,1,1,1,1,0,1,1,1,1,1,1,1,0,1,1,1,1,1,1,0,1,1,1,1,1,1,0,1,1,1,1,1,1,1,0,1,1,1,1,1,1,0,1,1,1,1,1,1,1,0,1,1,1,1,1,1,0,1,1,1,1,1,1,0,1},
	{1,1,1,1,1,1,0,1,1,1,1,1,1,0,1,1,1,1,1,1,0,1,1,1,1,1,1,0,1,1,1,1,1,1,0,1,1,1,1,1,1,0,1,1,1,1,1,1,0,1,1,1,1,1,1,1,0,1,1,1,1,1,1,0,1,1,1,1,1,1,0,1,1,1,1,1,1,0,1,1,1,1,1,1,0,1,1,1,1,1,1,0,1,1,1,1,1,1,0,1,1,1,1,1,1,1,0,1,1,1,1,1,1,0,1,1,1,1,1,1,0,1,1,1,1,1,1,0,1,1,1,1,1,1,0,1,1,1,1,1,1,0,1,1,1,1,1,1,0,1,1,1,1,1,1,1,0,1,1,1,1,1,1,0,1,1,1,1,1,1,0,1,1,1,1,1,1,0,1,1,1,1,1,1,0,1,1,1,1,1,1,0,1,1,1,1,1,1,0,1},
	{1,1,1,1,1,0,1,1,1,1,1,1,0,1,1,1,1,1,1,0,1,1,1,1,1,1,0,1,1,1,1,1,1,0,1,1,1,1,1,1,0,1,1,1,1,1,1,0,1,1,1,1,1,1,0,1,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,1,0,1,1,1,1,1,1,0,1,1,1,1,1,1,0,1,1,1,1,1,1,0,1,1,1,1,1,1,0,1,1,1,1,1,1,0,1,1,1,1,1,1,0,1,1,1,1,1,1,0,1,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,1,0,1,1,1,1,1,1,0,1,1,1,1,1,1,0,1,1,1,1,1,1,0,1,1,1,1,1,1,0,1,1,1,1,1,1,0,1,1,1,1,1,1,0,1,1,1,1,1,1,0,1,1,1,1,1,0,1},
	{1,1,1,1,1,0,1,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,1,0,1,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,1,0,1,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,1,0,1,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,1,0,1,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,1,0,1,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,1,0,1,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,1,0,1,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,1,0,1,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,1,0,1,1,1,1,1,1,0,1,1,1,1,1,0,1},
	{1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1},
	{1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1},
	{1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1},
	{1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,0,1},
	{1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,0,1},
	{1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,0,1},
	{1,1,1,1,0,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1},
	{1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1},
	{1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1},
	{1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1},
	{1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,0,1},
	{1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,0,1},
	{1,1,1,0,1,1,1,1,0,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,0,1,1,1,1,0,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,0,1,1,1,1,0,1,1,1,0,1},
	{1,1,1,0,1,1,1,1,0,1,1,1,0,1,1,1,1,0,1,1,1,0,1,1,1,1,0,1,1,1,0,1,1,1,1,0,1,1,1,0,1,1,1,1,0,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,0,1,1,1,1,0,1,1,1,0,1,1,1,1,0,1,1,1,0,1,1,1,1,0,1,1,1,0,1,1,1,1,0,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,0,1,1,1,1,0,1,1,1,0,1,1,1,1,0,1,1,1,0,1,1,1,1,0,1,1,1,0,1,1,1,1,0,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,0,1,1,1,1,0,1,1,1,0,1,1,1,1,0,1,1,1,0,1,1,1,1,0,1,1,1,0,1,1,1,1,0,1,1,1,0,1},
	{1,1,1,0,1,1,1,0,1,1,1,1,0,1,1,1,0,1,1,1,1,0,1,1,1,0,1,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,1,0,1,1,1,0,1,1,1,1,0,1,1,1,0,1,1,1,1,0,1,1,1,0,1,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,1,0,1,1,1,0,1,1,1,1,0,1,1,1,0,1,1,1,1,0,1,1,1,0,1,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,1,0,1,1,1,0,1,1,1,1,0,1,1,1,0,1,1,1,1,0,1,1,1,0,1,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,1,0,1,1,1,0,1,1,1,1,0,1,1,1,0,1,1,1,1,0,1,1,1,0,1,1,1,1,0,1,1,1,0,1,1,1,0,1},
	{1,1,1,0,1,1,1,0,1,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,1,0,1,1,1,0,1,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,1,0,1,1,1,0,1,1,1,0,1},
	{1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1},
	{1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1},
	{1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1},
	{1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1},
	{1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,0,1},
	{1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,0,1},
	{1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,0,1},
	{1,1,0,1,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,1,0,1,1,0,1},
	{1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,0,1},
	{1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,0,1},
	{1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,0,1},
	{1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,0,1},
	{1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,0,1},
	{1,1,0,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,0,1},
	{1,1,0,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,0,1,1,0,1},
	{1,1,0,1,1,0,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1},
	{1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1},
	{1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1},
	{1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1},
	{1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1},
	{1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,0,1},
	{1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,0,1},
	{1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,0,1},
	{1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,0,1},
	{1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,0,1},
	{1,0,1,1,0,1,1,0,1,1,0,1,0,1,1,0,1,1,0,1,1,0,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,0,1,1,0,1,1,0,1,1,0,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,0,1,1,0,1,1,0,1,1,0,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,0,1,1,0,1,1,0,1,1,0,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,0,1,1,0,1,1,0,1,1,0,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,0,1,1,0,1,1,0,1,1,0,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,0,1,1,0,1,1,0,1,1,0,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,0,1,1,0,1,1,0,1,1,0,1,0,1},
	{1,0,1,1,0,1,1,0,1,0,1,1,0,1,1,0,1,1,0,1,0,1,1,0,1,1,0,1,1,0,1,0,1,1,0,1,1,0,1,1,0,1,0,1,1,0,1,1,0,1,1,0,1,0,1,1,0,1,1,0,1,1,0,1,0,1,1,0,1,1,0,1,0,1,1,0,1,1,0,1,1,0,1,0,1,1,0,1,1,0,1,1,0,1,0,1,1,0,1,1,0,1,1,0,1,0,1,1,0,1,1,0,1,1,0,1,0,1,1,0,1,1,0,1,1,0,1,0,1,1,0,1,1,0,1,0,1,1,0,1,1,0,1,1,0,1,0,1,1,0,1,1,0,1,1,0,1,0,1,1,0,1,1,0,1,1,0,1,0,1,1,0,1,1,0,1,1,0,1,0,1,1,0,1,1,0,1,1,0,1,0,1,1,0,1,1,0,1,0,1},
	{1,0,1,1,0,1,1,0,1,0,1,1,0,1,1,0,1,0,1,1,0,1,1,0,1,1,0,1,0,1,1,0,1,1,0,1,0,1,1,0,1,1,0,1,0,1,1,0,1,1,0,1,1,0,1,0,1,1,0,1,1,0,1,0,1,1,0,1,1,0,1,0,1,1,0,1,1,0,1,1,0,1,0,1,1,0,1,1,0,1,0,1,1,0,1,1,0,1,0,1,1,0,1,1,0,1,1,0,1,0,1,1,0,1,1,0,1,0,1,1,0,1,1,0,1,1,0,1,0,1,1,0,1,1,0,1,0,1,1,0,1,1,0,1,0,1,1,0,1,1,0,1,1,0,1,0,1,1,0,1,1,0,1,0,1,1,0,1,1,0,1,0,1,1,0,1,1,0,1,1,0,1,0,1,1,0,1,1,0,1,0,1,1,0,1,1,0,1,0,1},
	{1,0,1,1,0,1,0,1,1,0,1,1,0,1,0,1,1,0,1,1,0,1,0,1,1,0,1,1,0,1,0,1,1,0,1,1,0,1,0,1,1,0,1,1,0,1,0,1,1,0,1,1,0,1,0,1,1,0,1,1,0,1,0,1,1,0,1,1,0,1,0,1,1,0,1,1,0,1,0,1,1,0,1,1,0,1,0,1,1,0,1,1,0,1,0,1,1,0,1,1,0,1,0,1,1,0,1,1,0,1,0,1,1,0,1,1,0,1,0,1,1,0,1,1,0,1,0,1,1,0,1,1,0,1,0,1,1,0,1,1,0,1,0,1,1,0,1,1,0,1,0,1,1,0,1,1,0,1,0,1,1,0,1,1,0,1,0,1,1,0,1,1,0,1,0,1,1,0,1,1,0,1,0,1,1,0,1,1,0,1,0,1,1,0,1,1,0,1,0,1},
	{1,0,1,1,0,1,0,1,1,0,1,1,0,1,0,1,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,1,0,1,0,1,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,1,0,1,0,1,1,0,1,1,0,1,0,1,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,1,0,1,0,1,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,1,0,1,0,1,1,0,1,1,0,1,0,1,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,1,0,1,0,1,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,1,0,1,0,1,1,0,1,1,0,1,0,1,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,1,0,1,0,1,1,0,1,1,0,1,0,1,1,0,1,0,1},
	{1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1},
	{1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1},
	{1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1},
	{1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1},
	{1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,0,1},
	{1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,0,1},
	{1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,0,1},
	{1,0,1,0,1,1,0,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,0,1,1,0,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,0,1,1,0,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,0,1,1,0,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,0,1,1,0,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,0,1,1,0,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,0,1,1,0,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,0,1,1,0,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,0,1,1,0,1,0,1,0,1},
	{1,0,1,0,1,1,0,1,0,1,0,1,1,0,1,0,1,0,1,1,0,1,0,1,0,1,1,0,1,0,1,0,1,1,0,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,0,1,1,0,1,0,1,0,1,1,0,1,0,1,0,1,1,0,1,0,1,0,1,1,0,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,0,1,1,0,1,0,1,0,1,1,0,1,0,1,0,1,1,0,1,0,1,0,1,1,0,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,0,1,1,0,1,0,1,0,1,1,0,1,0,1,0,1,1,0,1,0,1,0,1,1,0,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,0,1,1,0,1,0,1,0,1,1,0,1,0,1,0,1,1,0,1,0,1,0,1,1,0,1,0,1,0,1},
	{1,0,1,0,1,0,1,1,0,1,0,1,0,1,1,0,1,0,1,0,1,1,0,1,0,1,0,1,1,0,1,0,1,0,1,1,0,1,0,1,0,1,1,0,1,0,1,0,1,1,0,1,0,1,0,1,1,0,1,0,1,0,1,1,0,1,0,1,0,1,1,0,1,0,1,0,1,1,0,1,0,1,0,1,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,1,0,1,0,1,0,1,1,0,1,0,1,0,1,1,0,1,0,1,0,1,1,0,1,0,1,0,1,1,0,1,0,1,0,1,1,0,1,0,1,0,1,1,0,1,0,1,0,1,1,0,1,0,1,0,1,1,0,1,0,1,0,1,1,0,1,0,1,0,1,1,0,1,0,1,0,1,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1},
	{1,0,1,0,1,0,1,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1},
	{1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1},
	{1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,0,1},
	{1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,0,1},
	{1,0,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,0,1,0,1},
	{1,0,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,0,1,0,1},
	{1,0,1,0,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1},
	{1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1},
	{1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1},
	{1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1},
	{1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1},
	{1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1},
	{1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1},
	{0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1},
	{0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,0,1},
	{0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,0,1},
	{0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,0,1},
	{0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,0,1},
	{0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,0,1},
	{0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,0,1},
	{0,1,0,1,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,1,0,1,0,0,1},
	{0,1,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,1,0,0,1},
	{0,1,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,1,0,0,1},
	{0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,0,1},
	{0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,0,1},
	{0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,0,1},
	{0,1,0,1,0,1,0,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,0,1,0,1,0,1,0,0,1},
	{0,1,0,1,0,1,0,0,1,0,1,0,1,0,0,1,0,1,0,1,0,0,1,0,1,0,1,0,0,1,0,1,0,1,0,0,1,0,1,0,1,0,0,1,0,1,0,1,0,0,1,0,1,0,1,0,0,1,0,1,0,1,0,0,1,0,1,0,1,0,0,1,0,1,0,1,0,0,1,0,1,0,1,0,0,1,0,1,0,1,0,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,0,1,0,1,0,1,0,0,1,0,1,0,1,0,0,1,0,1,0,1,0,0,1,0,1,0,1,0,0,1,0,1,0,1,0,0,1,0,1,0,1,0,0,1,0,1,0,1,0,0,1,0,1,0,1,0,0,1,0,1,0,1,0,0,1,0,1,0,1,0,0,1,0,1,0,1,0,0,1,0,1,0,1,0,0,1},
	{0,1,0,1,0,0,1,0,1,0,1,0,0,1,0,1,0,1,0,0,1,0,1,0,1,0,0,1,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,1,0,0,1,0,1,0,1,0,0,1,0,1,0,1,0,0,1,0,1,0,1,0,0,1,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,1,0,0,1,0,1,0,1,0,0,1,0,1,0,1,0,0,1,0,1,0,1,0,0,1,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,1,0,0,1,0,1,0,1,0,0,1,0,1,0,1,0,0,1,0,1,0,1,0,0,1,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,1,0,0,1,0,1,0,1,0,0,1,0,1,0,1,0,0,1,0,1,0,1,0,0,1,0,1,0,1,0,0,1,0,1,0,0,1},
	{0,1,0,1,0,0,1,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,1,0,0,1,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,1,0,0,1,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,1,0,0,1,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,1,0,0,1,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,1,0,0,1,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,1,0,0,1,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,1,0,0,1,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,1,0,0,1,0,1,0,0,1},
	{0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1},
	{0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1},
	{0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1},
	{0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1},
	{0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,0,1},
	{0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,0,1},
	{0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,0,1},
	{0,1,0,0,1,0,1,0,0,1,0,0,1,0,1,0,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,0,1,0,1,0,0,1,0,0,1,0,1,0,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,0,1,0,1,0,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,0,1,0,1,0,0,1,0,0,1,0,1,0,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,0,1,0,1,0,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,0,1,0,1,0,0,1,0,0,1,0,1,0,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,0,1,0,1,0,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,0,1,0,1,0,0,1,0,0,1,0,1,0,0,1,0,0,1},
	{0,1,0,0,1,0,0,1,0,1,0,0,1,0,0,1,0,1,0,0,1,0,0,1,0,1,0,0,1,0,0,1,0,1,0,0,1,0,0,1,0,1,0,0,1,0,0,1,0,1,0,0,1,0,0,1,0,1,0,0,1,0,0,1,0,1,0,0,1,0,0,1,0,1,0,0,1,0,0,1,0,1,0,0,1,0,0,1,0,1,0,0,1,0,0,1,0,1,0,0,1,0,0,1,0,1,0,0,1,0,0,1,0,1,0,0,1,0,0,1,0,1,0,0,1,0,0,1,0,1,0,0,1,0,0,1,0,1,0,0,1,0,0,1,0,1,0,0,1,0,0,1,0,1,0,0,1,0,0,1,0,1,0,0,1,0,0,1,0,1,0,0,1,0,0,1,0,1,0,0,1,0,0,1,0,1,0,0,1,0,0,1,0,1,0,0,1,0,0,1},
	{0,1,0,0,1,0,0,1,0,1,0,0,1,0,0,1,0,1,0,0,1,0,0,1,0,0,1,0,1,0,0,1,0,0,1,0,1,0,0,1,0,0,1,0,1,0,0,1,0,0,1,0,0,1,0,1,0,0,1,0,0,1,0,1,0,0,1,0,0,1,0,1,0,0,1,0,0,1,0,0,1,0,1,0,0,1,0,0,1,0,1,0,0,1,0,0,1,0,0,1,0,1,0,0,1,0,0,1,0,1,0,0,1,0,0,1,0,1,0,0,1,0,0,1,0,0,1,0,1,0,0,1,0,0,1,0,1,0,0,1,0,0,1,0,1,0,0,1,0,0,1,0,0,1,0,1,0,0,1,0,0,1,0,1,0,0,1,0,0,1,0,1,0,0,1,0,0,1,0,0,1,0,1,0,0,1,0,0,1,0,1,0,0,1,0,0,1,0,0,1},
	{0,1,0,0,1,0,0,1,0,1,0,0,1,0,0,1,0,0,1,0,1,0,0,1,0,0,1,0,0,1,0,1,0,0,1,0,0,1,0,0,1,0,1,0,0,1,0,0,1,0,0,1,0,1,0,0,1,0,0,1,0,0,1,0,1,0,0,1,0,0,1,0,1,0,0,1,0,0,1,0,0,1,0,1,0,0,1,0,0,1,0,0,1,0,1,0,0,1,0,0,1,0,0,1,0,1,0,0,1,0,0,1,0,0,1,0,1,0,0,1,0,0,1,0,0,1,0,1,0,0,1,0,0,1,0,1,0,0,1,0,0,1,0,0,1,0,1,0,0,1,0,0,1,0,0,1,0,1,0,0,1,0,0,1,0,0,1,0,1,0,0,1,0,0,1,0,0,1,0,1,0,0,1,0,0,1,0,0,1,0,1,0,0,1,0,0,1,0,0,1},
	{0,1,0,0,1,0,0,1,0,0,1,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,1,0,0,1,0,0,1,0,0,1,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,1,0,0,1,0,0,1,0,0,1,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,1,0,0,1,0,0,1,0,0,1,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,1,0,0,1,0,0,1,0,0,1,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,1,0,0,1,0,0,1,0,0,1,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,1,0,0,1,0,0,1,0,0,1,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,1,0,0,1,0,0,1,0,0,1,0,1,0,0,1,0,0,1,0,0,1,0,0,1},
	{0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1},
	{0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1},
	{0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1},
	{0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1},
	{0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1},
	{0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,0,1},
	{0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,0,1},
	{0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,0,1},
	{0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,0,1},
	{0,0,1,0,0,1,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,1,0,0,1,0,0,0,1},
	{0,0,1,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,1,0,0,0,1},
	{0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,1,0,0,0,1},
	{0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,0,1},
	{0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,0,1},
	{0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,0,1},
	{0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,0,1},
	{0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,0,1},
	{0,0,1,0,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1},
	{0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1},
	{0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1},
	{0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1},
	{0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1},
	{0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,0,1},
	{0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,0,1},
	{0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,0,1},
	{0,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0,1,0,0,0,0,1},
	{0,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0,1,0,0,0,0,1},
	{0,0,0,1,0,0,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0,0,1},
	{0,0,0,1,0,0,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0,0,1},
	{0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1},
	{0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1},
	{0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1},
	{0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,0,1},
	{0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,0,1},
	{0,0,0,0,1,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,1,0,0,0,0,0,1},
	{0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,1},
	{0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,1},
	{0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,1},
	{0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,0,1},
	{0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,0,1},
	{0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,0,1},
	{0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,1},
	{0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,1},
	{0,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1},
	{0,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1},
	{0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1},
	{0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1},
	{0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,1},
	{0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,1},
	{0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,1},
	{0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,1},
	{0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,1},
	{0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,1},
	{0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,1},
	{0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,1},
	{0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,1},
	{0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
	{0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1}
};

/*
unsigned char speedsteptab[20][20]=
{
//  1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0
	{4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4},	//20
	{4,4,4,4,4,4,4,4,4,2,4,4,4,4,4,4,4,4,4,4},	//19
	{4,4,4,4,4,2,4,4,4,4,4,4,4,4,2,4,4,4,4,4},	//18
	{4,4,4,2,4,4,4,4,4,2,4,4,4,4,4,4,2,4,4,4},	//17
	{4,4,2,4,4,4,4,2,4,4,4,4,2,4,4,4,4,2,4,4},	//16
	{4,4,2,4,4,4,2,4,4,4,2,4,4,4,2,4,4,4,2,4},	//15
	{4,2,4,4,2,4,4,4,2,4,4,2,4,4,2,4,4,2,4,4},	//14
	{4,2,4,4,2,4,4,2,4,4,2,4,4,2,4,4,2,4,4,2},	//13
	{2,4,4,2,4,4,2,4,2,4,4,2,4,4,2,4,2,4,2,4},	//12
	{2,4,2,4,2,4,2,4,4,2,4,2,4,2,4,2,4,4,2,4},	//11
	{4,2,4,2,4,2,4,2,4,2,4,2,4,2,4,2,4,2,4,2},	//10
	{4,2,4,2,4,2,4,1,3,4,2,4,2,4,2,4,1,2,4,2},	//9
	{4,1,2,4,1,3,4,2,4,1,2,4,1,3,4,2,4,2,4,2},	//8
	{2,4,1,3,4,1,3,4,1,3,4,1,3,4,1,3,4,1,3,4},	//7
	{3,4,1,3,4,1,2,3,4,1,3,4,1,3,4,1,3,4,1,2},	//6
	{2,3,4,1,2,3,4,1,2,3,4,1,2,3,4,1,2,3,4,1},	//5
	{2,3,4,1,1,2,3,4,1,1,2,3,4,1,1,2,3,4,1,1},	//4
	{2,3,4,4,1,1,2,2,3,4,1,1,2,2,3,4,4,1,1,2},	 //3
	{2,3,3,4,4,4,1,1,1,2,2,2,3,3,4,1,1,1,2,2},	//2
	{3,3,3,3,3,4,4,4,4,4,1,1,1,1,1,2,2,2,2,2}		//1
};
*/

int speedo(int n)
{
	int moveSpeedValue;
	
	moveSpeedValue = map[n].ch_speed - map[n].ch_movespd;
	if (moveSpeedValue < 0)
	{
		moveSpeedValue = 0;
	}
	return speedtab[moveSpeedValue][ctick];
}

int speedoMisc(int n)
{
	int miscSpeedValue;
	
	miscSpeedValue = map[n].ch_speed;
	
	switch(map[n].ch_stat_off)
	{
		// 0, 5, 6 == Attack
		case    0:
		case    5:
		case    6:
			miscSpeedValue -= map[n].ch_atkspd;
			if (miscSpeedValue < 0) 
			{
				miscSpeedValue = 0;
			}
			return(speedtab[miscSpeedValue][ctick]);
			
		// 9 == Use skill, mostly casting
		case    9:
			miscSpeedValue -= map[n].ch_castspd;
			if (miscSpeedValue < 0) 
			{
				miscSpeedValue = 0;
			}
			return(speedtab[miscSpeedValue][ctick]);
			
		// Default - Shouldn't happen but here as a redundancy
		default:
			return(speedtab[miscSpeedValue][ctick]);
	}
}

int speedstep(int n,int d,int s,int update)
{
	int hard_step;
	int soft_step;
	int total_step;
	int speed;
	int dist;
	int z,m;

	speed=map[n].ch_speed;
	hard_step=map[n].ch_status-d;

	if (!update) return 32*hard_step/s;

	z=ctick;
	soft_step=0;
	m=hard_step;

	while (m) {
		z--;
		if (z<0) z=199;	// ctick extended from 20 to 24 to 200
		soft_step++;
		if (speedtab[speed][z])	m--;
	}
	while (1) {
		z--;
		if (z<0) z=199;	// ctick extended from 20 to 24 to 200
		if (speedtab[speed][z])	break;
		soft_step++;
	}

	z=ctick;
	total_step=soft_step;
	m=s-hard_step;

	while (1) {
		if (speedtab[speed][z])	m--;
		if (m<1) break;
		z++;
		if (z>199) z=0;	// ctick extended from 20 to 24 to 200
		total_step++;
	}
	dist=32*(soft_step)/(total_step+1);

	return dist;
}
//  0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10
static int stattab[]={ 0, 1, 1, 6, 6, 2, 3, 4, 5, 7, 4};

#define do_idle(ani,sprite)  (sprite==22480 ? ani : 0)

int eng_char(int n)
{
	int tmp,update=1;

	if (map[n].flags&STUNNED) update=0;

	switch (map[n].ch_status) {
		// idle up
		case    0:    map[n].obj_xoff=0; map[n].obj_yoff=0;
			map[n].idle_ani++; if (map[n].idle_ani>7) map[n].idle_ani=0;
			return map[n].ch_sprite+0+do_idle(map[n].idle_ani,map[n].ch_sprite);
			// idle down
		case    1:    map[n].obj_xoff=0; map[n].obj_yoff=0;
			if (speedo(n) && update) {
				map[n].idle_ani++; if (map[n].idle_ani>7) map[n].idle_ani=0;
			}
			return map[n].ch_sprite+8+do_idle(map[n].idle_ani,map[n].ch_sprite);
			// idle left
		case    2:    map[n].obj_xoff=0; map[n].obj_yoff=0;
			if (speedo(n) && update) {
				map[n].idle_ani++; if (map[n].idle_ani>7) map[n].idle_ani=0;
			}
			return map[n].ch_sprite+16+do_idle(map[n].idle_ani,map[n].ch_sprite);
			// idle right
		case    3:    map[n].obj_xoff=0; map[n].obj_yoff=0;
			if (speedo(n) && update) {
				map[n].idle_ani++; if (map[n].idle_ani>7) map[n].idle_ani=0;
			}
			return map[n].ch_sprite+24+do_idle(map[n].idle_ani,map[n].ch_sprite);

			// idle left-up
		case    4:    map[n].obj_xoff=0; map[n].obj_yoff=0;
			if (speedo(n) && update) {
				map[n].idle_ani++; if (map[n].idle_ani>7) map[n].idle_ani=0;
			}
			return map[n].ch_sprite+32+do_idle(map[n].idle_ani,map[n].ch_sprite);
			// idle left-down
		case    5:    map[n].obj_xoff=0; map[n].obj_yoff=0;
			if (speedo(n) && update) {
				map[n].idle_ani++; if (map[n].idle_ani>7) map[n].idle_ani=0;
			}
			return map[n].ch_sprite+40+do_idle(map[n].idle_ani,map[n].ch_sprite);
			// idle right-up
		case    6:    map[n].obj_xoff=0; map[n].obj_yoff=0;
			if (speedo(n) && update) {
				map[n].idle_ani++; if (map[n].idle_ani>7) map[n].idle_ani=0;
			}
			return map[n].ch_sprite+48+do_idle(map[n].idle_ani,map[n].ch_sprite);
			// idle right-down
		case    7:    map[n].obj_xoff=0; map[n].obj_yoff=0;
			if (speedo(n) && update) {
				map[n].idle_ani++; if (map[n].idle_ani>7) map[n].idle_ani=0;
			}
			return map[n].ch_sprite+56+do_idle(map[n].idle_ani,map[n].ch_sprite);

			// walk up
		case    16:
		case    17:
		case    18:
		case    19:
		case    20:
		case    21: map[n].obj_xoff=-speedstep(n,16,8,update)/2;
			map[n].obj_yoff=speedstep(n,16,8,update)/4;
			tmp=map[n].ch_sprite+(map[n].ch_status-16)+64;
			if (speedo(n) && update) map[n].ch_status+=speedo(n);
			return tmp;
		case    22: map[n].obj_xoff=-speedstep(n,16,8,update)/2;
			map[n].obj_yoff=speedstep(n,16,8,update)/4;
			tmp=map[n].ch_sprite+(map[n].ch_status-16)+64;
			if (speedo(n)==1 && update) map[n].ch_status++;
			else if (speedo(n)>1 && update) map[n].ch_status=16;
			return tmp;
		case    23:   map[n].obj_xoff=-speedstep(n,16,8,update)/2;
			map[n].obj_yoff=speedstep(n,16,8,update)/4;
			tmp=map[n].ch_sprite+(map[n].ch_status-16)+64;
			if (speedo(n) && update) map[n].ch_status=16+((speedo(n)>1)?1:0);
			return tmp;

			// walk down
		case    24:
		case    25:
		case    26:
		case    27:
		case    28:
		case    29: map[n].obj_xoff=speedstep(n,24,8,update)/2;
			map[n].obj_yoff=-speedstep(n,24,8,update)/4;
			tmp=map[n].ch_sprite+(map[n].ch_status-24)+72;
			if (speedo(n) && update) map[n].ch_status+=speedo(n);
			return tmp;
		case    30: map[n].obj_xoff=speedstep(n,24,8,update)/2;
			map[n].obj_yoff=-speedstep(n,24,8,update)/4;
			tmp=map[n].ch_sprite+(map[n].ch_status-24)+72;
			if (speedo(n)==1 && update) map[n].ch_status++;
			else if (speedo(n)>1 && update) map[n].ch_status=24;
			return tmp;
		case    31:   map[n].obj_xoff=speedstep(n,24,8,update)/2;
			map[n].obj_yoff=-speedstep(n,24,8,update)/4;
			tmp=map[n].ch_sprite+(map[n].ch_status-24)+72;
			if (speedo(n) && update) map[n].ch_status=24+((speedo(n)>1)?1:0);
			return tmp;

			// walk left
		case    32:
		case    33:
		case    34:
		case    35:
		case    36:
		case    37: map[n].obj_xoff=-speedstep(n,32,8,update)/2;
			map[n].obj_yoff=-speedstep(n,32,8,update)/4;
			tmp=map[n].ch_sprite+(map[n].ch_status-32)+80;
			if (speedo(n) && update) map[n].ch_status+=speedo(n);
			return tmp;
		case    38: map[n].obj_xoff=-speedstep(n,32,8,update)/2;
			map[n].obj_yoff=-speedstep(n,32,8,update)/4;
			tmp=map[n].ch_sprite+(map[n].ch_status-32)+80;
			if (speedo(n)==1 && update) map[n].ch_status++;
			else if (speedo(n)>1 && update) map[n].ch_status=32;
			return tmp;
		case    39:   map[n].obj_xoff=-speedstep(n,32,8,update)/2;
			map[n].obj_yoff=-speedstep(n,32,8,update)/4;
			tmp=map[n].ch_sprite+(map[n].ch_status-32)+80;
			if (speedo(n) && update) map[n].ch_status=32+((speedo(n)>1)?1:0);
			return tmp;

			// walk right
		case    40:
		case    41:
		case    42:
		case    43:
		case    44:
		case    45: map[n].obj_xoff=speedstep(n,40,8,update)/2;
			map[n].obj_yoff=speedstep(n,40,8,update)/4;
			tmp=map[n].ch_sprite+(map[n].ch_status-40)+88;
			if (speedo(n) && update) map[n].ch_status+=speedo(n);
			return tmp;
		case    46: map[n].obj_xoff=speedstep(n,40,8,update)/2;
			map[n].obj_yoff=speedstep(n,40,8,update)/4;
			tmp=map[n].ch_sprite+(map[n].ch_status-40)+88;
			if (speedo(n)==1 && update) map[n].ch_status++;
			else if (speedo(n)>1 && update) map[n].ch_status=40;
			return tmp;
		case    47:   map[n].obj_xoff=speedstep(n,40,8,update)/2;
			map[n].obj_yoff=speedstep(n,40,8,update)/4;
			tmp=map[n].ch_sprite+(map[n].ch_status-40)+88;
			if (speedo(n) && update) map[n].ch_status=40+((speedo(n)>1)?1:0);
			return tmp;


			// left+up
		case    48:
		case    49:
		case    50:
		case    51:
		case    52:
		case    53:
		case    54:
		case    55:
		case    56:
		case    57:   map[n].obj_xoff=-speedstep(n,48,12,update);
			map[n].obj_yoff=0;
			tmp=map[n].ch_sprite+(map[n].ch_status-48)*8/12+96;
			if (speedo(n) && update) map[n].ch_status+=speedo(n);
			return tmp;
		case    58:   map[n].obj_xoff=-speedstep(n,48,12,update);
			map[n].obj_yoff=0;
			tmp=map[n].ch_sprite+(map[n].ch_status-48)*8/12+96;
			if (speedo(n)==1 && update) map[n].ch_status++;
			else if (speedo(n)>1 && update) map[n].ch_status=48;
			return tmp;
		case    59:   map[n].obj_xoff=-speedstep(n,48,12,update);
			map[n].obj_yoff=0;
			tmp=map[n].ch_sprite+(map[n].ch_status-48)*8/12+96;
			if (speedo(n) && update) map[n].ch_status=48+((speedo(n)>1)?1:0);
			return tmp;


			// left+down
		case    60:
		case    61:
		case    62:
		case    63:
		case    64:
		case    65:
		case    66:
		case    67:
		case    68:
		case    69:   map[n].obj_xoff=0;
			map[n].obj_yoff=-speedstep(n,60,12,update)/2;
			tmp=map[n].ch_sprite+(map[n].ch_status-60)*8/12+104;
			if (speedo(n) && update) map[n].ch_status+=speedo(n);
			return tmp;
		case    70:   map[n].obj_xoff=0;
			map[n].obj_yoff=-speedstep(n,60,12,update)/2;
			tmp=map[n].ch_sprite+(map[n].ch_status-60)*8/12+104;
			if (speedo(n)==1 && update) map[n].ch_status++;
			else if (speedo(n)>1 && update) map[n].ch_status=60;
			return tmp;
		case    71:   map[n].obj_xoff=0;
			map[n].obj_yoff=-speedstep(n,60,12,update)/2;
			tmp=map[n].ch_sprite+(map[n].ch_status-60)*8/12+104;
			if (speedo(n) && update) map[n].ch_status=60+((speedo(n)>1)?1:0);
			return tmp;


			// right+up
		case    72:
		case    73:
		case    74:
		case    75:
		case    76:
		case    77:
		case    78:
		case    79:
		case    80:
		case    81:   map[n].obj_xoff=0;
			map[n].obj_yoff=speedstep(n,72,12,update)/2;
			tmp=map[n].ch_sprite+(map[n].ch_status-72)*8/12+112;
			if (speedo(n) && update) map[n].ch_status+=speedo(n);
			return tmp;
		case    82:   map[n].obj_xoff=0;
			map[n].obj_yoff=speedstep(n,72,12,update)/2;
			tmp=map[n].ch_sprite+(map[n].ch_status-72)*8/12+112;
			if (speedo(n)==1 && update) map[n].ch_status++;
			else if (speedo(n)>1 && update) map[n].ch_status=72;
			return tmp;
		case    83:   map[n].obj_xoff=0;
			map[n].obj_yoff=speedstep(n,72,12,update)/2;
			tmp=map[n].ch_sprite+(map[n].ch_status-72)*8/12+112;
			if (speedo(n) && update) map[n].ch_status=72+((speedo(n)>1)?1:0);
			return tmp;

			// right+down
		case    84:
		case    85:
		case    86:
		case    87:
		case    88:
		case    89:
		case    90:
		case    91:
		case    92:
		case    93:   map[n].obj_xoff=speedstep(n,84,12,update);
			map[n].obj_yoff=0;
			tmp=map[n].ch_sprite+(map[n].ch_status-84)*8/12+120;
			if (speedo(n) && update) map[n].ch_status+=speedo(n);
			return tmp;
		case    94:   map[n].obj_xoff=speedstep(n,84,12,update);
			map[n].obj_yoff=0;
			tmp=map[n].ch_sprite+(map[n].ch_status-84)*8/12+120;
			if (speedo(n)==1 && update) map[n].ch_status++;
			else if (speedo(n)>1 && update) map[n].ch_status=84;
			return tmp;
		case    95:   map[n].obj_xoff=speedstep(n,84,12,update);
			map[n].obj_yoff=0;
			tmp=map[n].ch_sprite+(map[n].ch_status-84)*8/12+120;
			if (speedo(n) && update) map[n].ch_status=84+((speedo(n)>1)?1:0);
			return tmp;

			// turn up to left-up
		case    96:
		case    97: map[n].obj_xoff=0; map[n].obj_yoff=0;
			tmp=map[n].ch_sprite+(map[n].ch_status-96)+128;
			if (speedo(n) && update) map[n].ch_status+=speedo(n);
			return tmp;
		case    98: map[n].obj_xoff=0; map[n].obj_yoff=0;
			tmp=map[n].ch_sprite+(map[n].ch_status-96)+128;
			if (speedo(n)==1 && update) map[n].ch_status++;
			else if (speedo(n)>1 && update) map[n].ch_status=96;
			return tmp;
		case    99: map[n].obj_xoff=0; map[n].obj_yoff=0;
			tmp=map[n].ch_sprite+(map[n].ch_status-96)+128;
			if (speedo(n) && update) map[n].ch_status=96+((speedo(n)>1)?1:0);
			return tmp;

			// turn left-up to up
		case 100:
		case 101:   map[n].obj_xoff=0; map[n].obj_yoff=0;
			tmp=map[n].ch_sprite+(map[n].ch_status-100)+132;
			if (speedo(n) && update) map[n].ch_status+=speedo(n);
			return tmp;
		case 102:   map[n].obj_xoff=0; map[n].obj_yoff=0;
			tmp=map[n].ch_sprite+(map[n].ch_status-100)+132;
			if (speedo(n)==1 && update) map[n].ch_status++;
			else if (speedo(n)>1 && update) map[n].ch_status=100;
			return tmp;
		case 103:   map[n].obj_xoff=0; map[n].obj_yoff=0;
			tmp=map[n].ch_sprite+(map[n].ch_status-100)+132;
			if (speedo(n) && update) map[n].ch_status=100+((speedo(n)>1)?1:0);
			return tmp;

			// turn up to right-up
		case 104:
		case 105:   map[n].obj_xoff=0; map[n].obj_yoff=0;
			tmp=map[n].ch_sprite+(map[n].ch_status-104)+136;
			if (speedo(n) && update) map[n].ch_status+=speedo(n);
			return tmp;
		case 106:   map[n].obj_xoff=0; map[n].obj_yoff=0;
			tmp=map[n].ch_sprite+(map[n].ch_status-104)+136;
			if (speedo(n)==1 && update) map[n].ch_status++;
			else if (speedo(n)>1 && update) map[n].ch_status=104;
			return tmp;
		case 107:   tmp=map[n].ch_sprite+(map[n].ch_status-104)+136;
			if (speedo(n) && update) map[n].ch_status=104+((speedo(n)>1)?1:0);
			return tmp;

			// turn right-up to right
		case 108:
		case 109:   map[n].obj_xoff=0; map[n].obj_yoff=0;
			tmp=map[n].ch_sprite+(map[n].ch_status-108)+140;
			if (speedo(n) && update) map[n].ch_status+=speedo(n);
			return tmp;
		case 110:   map[n].obj_xoff=0; map[n].obj_yoff=0;
			tmp=map[n].ch_sprite+(map[n].ch_status-108)+140;
			if (speedo(n)==1 && update) map[n].ch_status++;
			else if (speedo(n)>1 && update) map[n].ch_status=108;
			return tmp;
		case 111:   tmp=map[n].ch_sprite+(map[n].ch_status-108)+140;
			if (speedo(n) && update) map[n].ch_status=108+((speedo(n)>1)?1:0);
			return tmp;

			// turn down to left-down
		case 112:
		case 113:   map[n].obj_xoff=0; map[n].obj_yoff=0;
			tmp=map[n].ch_sprite+(map[n].ch_status-112)+144;
			if (speedo(n) && update) map[n].ch_status+=speedo(n);
			return tmp;
		case 114:   map[n].obj_xoff=0; map[n].obj_yoff=0;
			tmp=map[n].ch_sprite+(map[n].ch_status-112)+144;
			if (speedo(n)==1 && update) map[n].ch_status++;
			else if (speedo(n)>1 && update) map[n].ch_status=112;
			return tmp;
		case 115:   tmp=map[n].ch_sprite+(map[n].ch_status-112)+144;
			if (speedo(n) && update) map[n].ch_status=112+((speedo(n)>1)?1:0);
			return tmp;

			// turn left-down to left
		case 116:
		case 117:   map[n].obj_xoff=0; map[n].obj_yoff=0;
			tmp=map[n].ch_sprite+(map[n].ch_status-116)+148;
			if (speedo(n) && update) map[n].ch_status+=speedo(n);
			return tmp;
		case 118:   map[n].obj_xoff=0; map[n].obj_yoff=0;
			tmp=map[n].ch_sprite+(map[n].ch_status-116)+148;
			if (speedo(n)==1 && update) map[n].ch_status++;
			else if (speedo(n)>1 && update) map[n].ch_status=116;
			return tmp;
		case 119:   map[n].obj_xoff=0; map[n].obj_yoff=0;
			tmp=map[n].ch_sprite+(map[n].ch_status-116)+148;
			if (speedo(n) && update) map[n].ch_status=116+((speedo(n)>1)?1:0);
			return tmp;

			// turn down to right-down
		case 120:
		case 121:   map[n].obj_xoff=0; map[n].obj_yoff=0;
			tmp=map[n].ch_sprite+(map[n].ch_status-120)+152;
			if (speedo(n) && update) map[n].ch_status+=speedo(n);
			return tmp;
		case 122:   map[n].obj_xoff=0; map[n].obj_yoff=0;
			tmp=map[n].ch_sprite+(map[n].ch_status-120)+152;
			if (speedo(n)==1 && update) map[n].ch_status++;
			else if (speedo(n)>1 && update) map[n].ch_status=120;
			return tmp;
		case 123:   tmp=map[n].ch_sprite+(map[n].ch_status-120)+152;
			if (speedo(n) && update) map[n].ch_status=120+((speedo(n)>1)?1:0);
			return tmp;

			// turn right-down to down
		case 124:
		case 125:   map[n].obj_xoff=0; map[n].obj_yoff=0;
			tmp=map[n].ch_sprite+(map[n].ch_status-124)+156;
			if (speedo(n) && update) map[n].ch_status+=speedo(n);
			return tmp;
		case 126:   map[n].obj_xoff=0; map[n].obj_yoff=0;
			tmp=map[n].ch_sprite+(map[n].ch_status-124)+156;
			if (speedo(n)==1 && update) map[n].ch_status++;
			else if (speedo(n)>1 && update) map[n].ch_status=124;
			return tmp;
		case 127:   map[n].obj_xoff=0; map[n].obj_yoff=0;
			tmp=map[n].ch_sprite+(map[n].ch_status-124)+156;
			if (speedo(n) && update) map[n].ch_status=124+((speedo(n)>1)?1:0);
			return tmp;

			// turn left to left-up
		case 128:
		case 129:   map[n].obj_xoff=0; map[n].obj_yoff=0;
			tmp=map[n].ch_sprite+(map[n].ch_status-128)+160;
			if (speedo(n) && update) map[n].ch_status+=speedo(n);
			return tmp;
		case 130:   map[n].obj_xoff=0; map[n].obj_yoff=0;
			tmp=map[n].ch_sprite+(map[n].ch_status-128)+160;
			if (speedo(n)==1 && update) map[n].ch_status++;
			else if (speedo(n)>1 && update) map[n].ch_status=128;
			return tmp;
		case 131:   tmp=map[n].ch_sprite+(map[n].ch_status-128)+160;
			if (speedo(n) && update) map[n].ch_status=128+((speedo(n)>1)?1:0);
			return tmp;

			// turn left-up to up
		case 132:
		case 133:   map[n].obj_xoff=0; map[n].obj_yoff=0;
			tmp=map[n].ch_sprite+(map[n].ch_status-132)+164;
			if (speedo(n) && update) map[n].ch_status+=speedo(n);
			return tmp;
		case 134:   map[n].obj_xoff=0; map[n].obj_yoff=0;
			tmp=map[n].ch_sprite+(map[n].ch_status-132)+164;
			if (speedo(n)==1 && update) map[n].ch_status++;
			else if (speedo(n)>1 && update) map[n].ch_status=132;
			return tmp;
		case 135:   map[n].obj_xoff=0; map[n].obj_yoff=0;
			tmp=map[n].ch_sprite+(map[n].ch_status-132)+164;
			if (speedo(n) && update) map[n].ch_status=132+((speedo(n)>1)?1:0);
			return tmp;

			// turn left to left-down
		case 136:
		case 137:   map[n].obj_xoff=0; map[n].obj_yoff=0;
			tmp=map[n].ch_sprite+(map[n].ch_status-136)+168;
			if (speedo(n) && update) map[n].ch_status+=speedo(n);
			return tmp;
		case 138:   map[n].obj_xoff=0; map[n].obj_yoff=0;
			tmp=map[n].ch_sprite+(map[n].ch_status-136)+168;
			if (speedo(n)==1 && update) map[n].ch_status++;
			else if (speedo(n)>1 && update) map[n].ch_status=136;
			return tmp;
		case 139:   tmp=map[n].ch_sprite+(map[n].ch_status-136)+168;
			if (speedo(n) && update) map[n].ch_status=136+((speedo(n)>1)?1:0);
			return tmp;

			// turn left-down to down
		case 140:
		case 141:   map[n].obj_xoff=0; map[n].obj_yoff=0;
			tmp=map[n].ch_sprite+(map[n].ch_status-140)+172;
			if (speedo(n) && update) map[n].ch_status+=speedo(n);
			return tmp;
		case 142:   map[n].obj_xoff=0; map[n].obj_yoff=0;
			tmp=map[n].ch_sprite+(map[n].ch_status-140)+172;
			if (speedo(n)==1 && update) map[n].ch_status++;
			else if (speedo(n)>1 && update) map[n].ch_status=140;
			return tmp;
		case 143:   map[n].obj_xoff=0; map[n].obj_yoff=0;
			tmp=map[n].ch_sprite+(map[n].ch_status-140)+172;
			if (speedo(n) && update) map[n].ch_status=140+((speedo(n)>1)?1:0);
			return tmp;

			// turn right to right-up
		case 144:
		case 145:   map[n].obj_xoff=0; map[n].obj_yoff=0;
			tmp=map[n].ch_sprite+(map[n].ch_status-144)+176;
			if (speedo(n) && update) map[n].ch_status+=speedo(n);
			return tmp;
		case 146:   map[n].obj_xoff=0; map[n].obj_yoff=0;
			tmp=map[n].ch_sprite+(map[n].ch_status-144)+176;
			if (speedo(n)==1 && update) map[n].ch_status++;
			else if (speedo(n)>1 && update) map[n].ch_status=144;
			return tmp;
		case 147:   tmp=map[n].ch_sprite+(map[n].ch_status-144)+176;
			if (speedo(n) && update) map[n].ch_status=144+((speedo(n)>1)?1:0);
			return tmp;

			// turn right-up to up
		case 148:
		case 149:   map[n].obj_xoff=0; map[n].obj_yoff=0;
			tmp=map[n].ch_sprite+(map[n].ch_status-148)+180;
			if (speedo(n) && update) map[n].ch_status+=speedo(n);
			return tmp;
		case 150:   map[n].obj_xoff=0; map[n].obj_yoff=0;
			tmp=map[n].ch_sprite+(map[n].ch_status-148)+180;
			if (speedo(n)==1 && update) map[n].ch_status++;
			else if (speedo(n)>1 && update) map[n].ch_status=148;
			return tmp;
		case 151:   map[n].obj_xoff=0; map[n].obj_yoff=0;
			tmp=map[n].ch_sprite+(map[n].ch_status-148)+180;
			if (speedo(n) && update) map[n].ch_status=148+((speedo(n)>1)?1:0);
			return tmp;

			// turn right to right-down
		case 152:
		case 153:   map[n].obj_xoff=0; map[n].obj_yoff=0;
			tmp=map[n].ch_sprite+(map[n].ch_status-152)+184;
			if (speedo(n) && update) map[n].ch_status+=speedo(n);
			return tmp;
		case 154:   map[n].obj_xoff=0; map[n].obj_yoff=0;
			tmp=map[n].ch_sprite+(map[n].ch_status-152)+184;
			if (speedo(n)==1 && update) map[n].ch_status++;
			else if (speedo(n)>1 && update) map[n].ch_status=152;
			return tmp;
		case 155:   tmp=map[n].ch_sprite+(map[n].ch_status-152)+184;
			if (speedo(n) && update) map[n].ch_status=152+((speedo(n)>1)?1:0);
			return tmp;

			// turn right-down to down
		case 156:
		case 157:   map[n].obj_xoff=0; map[n].obj_yoff=0;
			tmp=map[n].ch_sprite+(map[n].ch_status-156)+188;
			if (speedo(n) && update) map[n].ch_status+=speedo(n);
			return tmp;
		case 158:   map[n].obj_xoff=0; map[n].obj_yoff=0;
			tmp=map[n].ch_sprite+(map[n].ch_status-156)+188;
			if (speedo(n)==1 && update) map[n].ch_status++;
			else if (speedo(n)>1 && update) map[n].ch_status=156;
			return tmp;
		case 159:   map[n].obj_xoff=0; map[n].obj_yoff=0;
			tmp=map[n].ch_sprite+(map[n].ch_status-156)+188;
			if (speedo(n) && update) map[n].ch_status=156+((speedo(n)>1)?1:0);
			return tmp;

			// misc up
		case 160:
		case 161:
		case 162:
		case 163:
		case 164:
		case 165:   map[n].obj_xoff=0; map[n].obj_yoff=0;
			tmp=map[n].ch_sprite+(map[n].ch_status-160)+192+((int)(stattab[map[n].ch_stat_off])<<5);
			if (speedoMisc(n) && update) map[n].ch_status+=speedoMisc(n);
			return tmp;
		case 166:   map[n].obj_xoff=0; map[n].obj_yoff=0;
			tmp=map[n].ch_sprite+(map[n].ch_status-160)+192+((int)(stattab[map[n].ch_stat_off])<<5);
			if (speedoMisc(n)==1 && update) map[n].ch_status++;
			else if (speedoMisc(n)>1 && update) map[n].ch_status=160;
			return tmp;
		case 167:   map[n].obj_xoff=0; map[n].obj_yoff=0;
			tmp=map[n].ch_sprite+(map[n].ch_status-160)+192+((int)(stattab[map[n].ch_stat_off])<<5);
			if (speedoMisc(n) && update) map[n].ch_status=160+((speedoMisc(n)>1)?1:0);
			return tmp;

			// misc down
		case 168:
		case 169:
		case 170:
		case 171:
		case 172:
		case 173:   map[n].obj_xoff=0; map[n].obj_yoff=0;
			tmp=map[n].ch_sprite+(map[n].ch_status-168)+200+((int)(stattab[map[n].ch_stat_off])<<5);
			if (speedoMisc(n) && update) map[n].ch_status+=speedoMisc(n);
			return tmp;
		case 174:   map[n].obj_xoff=0; map[n].obj_yoff=0;
			tmp=map[n].ch_sprite+(map[n].ch_status-168)+200+((int)(stattab[map[n].ch_stat_off])<<5);
			if (speedoMisc(n)==1 && update) map[n].ch_status++;
			else if (speedoMisc(n)>1 && update) map[n].ch_status=168;
			return tmp;
		case 175:   map[n].obj_xoff=0; map[n].obj_yoff=0;
			tmp=map[n].ch_sprite+(map[n].ch_status-168)+200+((int)(stattab[map[n].ch_stat_off])<<5);
			if (speedoMisc(n) && update) map[n].ch_status=168+((speedoMisc(n)>1)?1:0);
			return tmp;

			// misc left
		case 176:
		case 177:
		case 178:
		case 179:
		case 180:
		case 181:   map[n].obj_xoff=0; map[n].obj_yoff=0;
			tmp=map[n].ch_sprite+(map[n].ch_status-176)+208+((int)(stattab[map[n].ch_stat_off])<<5);
			if (speedoMisc(n) && update) map[n].ch_status+=speedoMisc(n);
			return tmp;
		case 182:   map[n].obj_xoff=0; map[n].obj_yoff=0;
			tmp=map[n].ch_sprite+(map[n].ch_status-176)+208+((int)(stattab[map[n].ch_stat_off])<<5);
			if (speedoMisc(n)==1 && update) map[n].ch_status++;
			else if (speedoMisc(n)>1 && update) map[n].ch_status=176;
			return tmp;
		case 183:   map[n].obj_xoff=0; map[n].obj_yoff=0;
			tmp=map[n].ch_sprite+(map[n].ch_status-176)+208+((int)(stattab[map[n].ch_stat_off])<<5);
			if (speedoMisc(n) && update) map[n].ch_status=176+((speedoMisc(n)>1)?1:0);
			return tmp;

			// misc right
		case 184:
		case 185:
		case 186:
		case 187:
		case 188:
		case 189:   map[n].obj_xoff=0; map[n].obj_yoff=0;
			tmp=map[n].ch_sprite+(map[n].ch_status-184)+216+((int)(stattab[map[n].ch_stat_off])<<5);
			if (speedoMisc(n) && update) map[n].ch_status+=speedoMisc(n);
			return tmp;
		case 190:   map[n].obj_xoff=0; map[n].obj_yoff=0;
			tmp=map[n].ch_sprite+(map[n].ch_status-184)+216+((int)(stattab[map[n].ch_stat_off])<<5);
			if (speedoMisc(n)==1 && update) map[n].ch_status++;
			else if (speedoMisc(n)>1 && update) map[n].ch_status=184;
			return tmp;
		case 191:   map[n].obj_xoff=0; map[n].obj_yoff=0;
			tmp=map[n].ch_sprite+(map[n].ch_status-184)+216+((int)(stattab[map[n].ch_stat_off])<<5);
			if (speedoMisc(n) && update) map[n].ch_status=184+((speedoMisc(n)>1)?1:0);
			return tmp;

		default:        xlog(0,"Unknown ch_status %d",map[n].ch_status);
			return map[n].ch_sprite;
	}
}

int eng_item(int n)
{
	switch (map[n].it_status) {
		case    0:      return map[n].it_sprite;
		case    1:      return map[n].it_sprite;

			// four sprite animation, 2-step
		case    2:    if (speedtab[200][ctick]) map[n].it_status++;	// adjusted these speedtab values from 10 to 24 to 200
			return map[n].it_sprite;

		case    3:    if (speedtab[200][ctick]) map[n].it_status++;
			return map[n].it_sprite+2;

		case    4:    if (speedtab[200][ctick]) map[n].it_status++;
			return map[n].it_sprite+4;

		case    5:    if (speedtab[200][ctick]) map[n].it_status=2;
			return map[n].it_sprite+6;

			// two sprite animation, 1-step
		case    6:    if (speedtab[200][ctick]) map[n].it_status++;
			return map[n].it_sprite;

		case    7:    if (speedtab[200][ctick]) map[n].it_status=6;
			return map[n].it_sprite+1;

			// eight sprite animation, 1-step
		case    8:    if (speedtab[200][ctick]) map[n].it_status++;
			return map[n].it_sprite;

		case    9:    if (speedtab[200][ctick]) map[n].it_status++;
			return map[n].it_sprite+1;

		case    10:   if (speedtab[200][ctick]) map[n].it_status++;
			return map[n].it_sprite+2;

		case    11:   if (speedtab[200][ctick]) map[n].it_status++;
			return map[n].it_sprite+3;

		case   12:   if (speedtab[200][ctick]) map[n].it_status++;
			return map[n].it_sprite+4;

		case    13:   if (speedtab[200][ctick]) map[n].it_status++;
			return map[n].it_sprite+5;

		case    14:   if (speedtab[200][ctick]) map[n].it_status++;
			return map[n].it_sprite+6;

		case    15:   if (speedtab[200][ctick]) map[n].it_status=8;
			return map[n].it_sprite+7;

			// five sprite animation, 1-step, random
		case    16:   if (speedtab[200][ctick]) map[n].it_status++;
			return map[n].it_sprite;

		case    17:   if (speedtab[200][ctick]) map[n].it_status++;
				return map[n].it_sprite+1;

		case    18:   	if (speedtab[200][ctick]) map[n].it_status++;
				return map[n].it_sprite+2;

		case    19:   	if (speedtab[200][ctick]) map[n].it_status++;
				return map[n].it_sprite+3;

		case   20:   	if (speedtab[200][ctick]) map[n].it_status=16;
				return map[n].it_sprite+4;

		case   21:  return map[n].it_sprite+(ticker&63);

		default:        xlog(0,"Unknown it_status");
				return map[n].it_sprite;
	}
}

void engine_tick(void)
{
	int n,tmp;

	ticker++;

	for (n=0; n<screen_renderdist*screen_renderdist; n++) {
		map[n].back=0;
		map[n].obj1=0;
		map[n].obj2=0;
		map[n].ovl_xoff=0;
		map[n].ovl_yoff=0;
	}

	for (n=0; n<screen_renderdist*screen_renderdist; n++) {

		map[n].back=map[n].ba_sprite;

		// item
		if (map[n].it_sprite) {
			tmp=eng_item(n);
			map[n].obj1=tmp;
		}

		// character
		if (map[n].ch_sprite) {
			tmp=eng_char(n);
			map[n].obj2=tmp;
		}
	}
}

void send_opt(void)
{
	static int state=0;
	unsigned char buf[16];
	int n;

	buf[0]=CL_CMD_SETUSER;

	switch (state) {
		case    0:  buf[1]=0; buf[2]=0; for (n=0; n<13; n++) buf[n+3]=pdata.cname[n];
			xlog(1,"Transfering user data..."); break;
		case    1: buf[1]=0; buf[2]=13; for (n=0; n<13; n++) buf[n+3]=pdata.cname[n+13]; break;
		case    2:  buf[1]=0; buf[2]=26; for (n=0; n<13; n++) buf[n+3]=pdata.cname[n+26]; break;
		case    3:  buf[1]=0; buf[2]=39; for (n=0; n<13; n++) buf[n+3]=pdata.cname[n+39]; break;
		case    4: buf[1]=0; buf[2]=52; for (n=0; n<13; n++) buf[n+3]=pdata.cname[n+52]; break;
		case    5: buf[1]=0; buf[2]=65; for (n=0; n<13; n++) buf[n+3]=pdata.cname[n+65]; break;

		case  6:    buf[1]=1; buf[2]=0; for (n=0; n<13; n++) buf[n+3]=pdata.desc[n]; break;
		case  7: buf[1]=1; buf[2]=13; for (n=0; n<13; n++) buf[n+3]=pdata.desc[n+13]; break;
		case  8:    buf[1]=1; buf[2]=26; for (n=0; n<13; n++) buf[n+3]=pdata.desc[n+26]; break;
		case  9:    buf[1]=1; buf[2]=39; for (n=0; n<13; n++) buf[n+3]=pdata.desc[n+39]; break;
		case 10: buf[1]=1; buf[2]=52; for (n=0; n<13; n++) buf[n+3]=pdata.desc[n+52]; break;
		case 11: buf[1]=1; buf[2]=65; for (n=0; n<13; n++) buf[n+3]=pdata.desc[n+65]; break;

		case 12:    buf[1]=2; buf[2]=0; for (n=0; n<13; n++) buf[n+3]=pdata.desc[n+78]; break;
		case 13: buf[1]=2; buf[2]=13; for (n=0; n<13; n++) buf[n+3]=pdata.desc[n+91]; break;
		case 14:    buf[1]=2; buf[2]=26; for (n=0; n<13; n++) buf[n+3]=pdata.desc[n+104]; break;
		case 15:    buf[1]=2; buf[2]=39; for (n=0; n<13; n++) buf[n+3]=pdata.desc[n+117]; break;
		case 16: buf[1]=2; buf[2]=52; for (n=0; n<13; n++) buf[n+3]=pdata.desc[n+130]; break;
		case 17: buf[1]=2; buf[2]=65; for (n=0; n<13; n++) buf[n+3]=pdata.desc[n+143];
			pdata.changed=0; save_options();
			xlog(1,"Transfer done."); break;
	}
	xsend(buf);
	state++;
}

int firstquit=0;
int wantquit=0;
int maynotquit=TICKS*5/TICKMULTI;

void cmd_exit(void)
{
	if (do_exit || !maynotquit) {
		quit=1;
		return;
	}
	if (!firstquit) {
		xlog(0," ");
		xlog(0,"(F12) Leaving the game without entering a tavern will cause you to take damage, and possibly lose your life. Click again if you need to leave immediately.");
		xlog(0,"A tavern will be located near where you entered the game.");
		firstquit=1;
		return;
	}

	if (!wantquit) {
		cmd1(CL_CMD_EXIT,0);
		wantquit=1;
		xlog(0," ");
		xlog(0,"(F12) Exit request acknowledged. Please wait a moment...");
	}
}

int noshop=0;
extern int xmove;
extern int do_ticker;

void engine(void)
{
	int tmp,tick,init=0;
	int step=0,skip=0,lookstep=0,optstep=0,skipinrow=0,n,panic,xtimer=0;
	extern int cmd_count,tick_count;
	unsigned int t;

	skilltab=malloc(sizeof(struct skilltab)*MAXSKILL);
	for (n=0; n<MAXSKILL; n++) {
		skilltab[n]=_skilltab[n];
		skilltab[n].attrib[0]=1;
	}

	init_done=1;

	t=GetTickCount();

	while (!quit) {
		do_msg();
		if (wantquit && maynotquit)	maynotquit--;

		if (do_ticker && (ticker&15)==0) cmd1s(CL_CMD_CTICK,ticker);

		if (step++>16) {
			pskip=100.0*(float)skip/(float)frame;
			pidle=100.0*(float)idle/(float)xtime;
            skip=frame=0;
			idle=xtime=0;
			step=0;
		}

		frame++;

		lookstep++;
		if (lookat && lookstep>QSIZE*3) {
			if (lookat>=lookmax || looks[lookat].known==0)
				cmd1s(CL_CMD_AUTOLOOK,lookat);
			lookat=0;
			lookstep=0;
		}

		if (look_timer)	look_timer--;
		else show_look=0;

		if ((show_shop || show_wps || show_motd || show_newp || show_tuto) && lookstep>QSIZE) {
			cmd1s(CL_CMD_LOOK,shop.nr);
			lookstep=0;
		}

		optstep++;
		if (optstep>4 && pdata.changed) {
			send_opt();
			optstep=0;
		}

		if (xtime>0) xtimer--;

		if (xmove && xtimer<1) {
			switch (xmove) {
				case  1:     cmds(CL_CMD_MOVE,map[(RENDERDIST/2-7)+screen_renderdist*RENDERDIST/2].x,map[(RENDERDIST/2-7)+screen_renderdist*RENDERDIST/2].y); break;
				case  3:     cmds(CL_CMD_MOVE,map[(RENDERDIST/2+7)+screen_renderdist*RENDERDIST/2].x,map[(RENDERDIST/2+7)+screen_renderdist*RENDERDIST/2].y); break;
				case  2:     cmds(CL_CMD_MOVE,map[RENDERDIST/2+screen_renderdist*(RENDERDIST/2-7)].x,map[RENDERDIST/2+screen_renderdist*(RENDERDIST/2-7)].y); break;
				case  4:     cmds(CL_CMD_MOVE,map[RENDERDIST/2+screen_renderdist*(RENDERDIST/2+7)].x,map[RENDERDIST/2+screen_renderdist*(RENDERDIST/2+7)].y); break;
			}
			xtimer=4;
		}

		panic=0;
		do {
			do_msg();
			tmp=game_loop();
			panic++;
		} while (tmp && panic<8192);

		tmp=tick_do();
        if (tmp) init=1;
		if (do_exit) init=0;

		do_msg();

		if (noshop>0) {
			noshop--;
			show_shop=0;
			show_wps=0;
			show_motd=0;
			show_newp=0;
			show_tuto=0;
		}
		if (t>GetTickCount() || skipinrow>100) {	// display frame only if we've got enough time
			eng_display(init);
			eng_flip(t);
			skipinrow=0;
		} else {
			skip++; skipinrow++;
		}

		do_msg();

		if (t_size) tick=TICK*QSIZE/t_size;
		else tick=TICK;

		t+=tick; ttime+=tick; xtime+=tick;
	}
}

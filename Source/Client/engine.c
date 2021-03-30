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
extern int screen_overlay_sprite;
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
	"Strength"};

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

static char *rank[24]={
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
	"Warlord"
};

int stat_raised[108]={0,0,0,0,0,0,0,0,0,0,0,0,0,};
int stat_points_used=0;

int points2rank(int v)
{
	if (v<     250)	return( 0); // Private
	if (v<    1750)	return( 1); // Private FIrst Class
	if (v<    7000)	return( 2); // Lance Corporal
	if (v<   21000)	return( 3); // Corporal
	if (v<   52500)	return( 4); // Sergeant
	if (v<  115500)	return( 5); // Staff Sergeant
	if (v<  231000)	return( 6); // Master Sergeant
	if (v<  429000)	return( 7); // First Sergeant
	if (v<  750750)	return( 8); // Sergeant Major
	if (v< 1251250)	return( 9); // Second Lieutenant
	if (v< 2002000)	return(10); // First Lieutenant
	if (v< 3094000)	return(11); // Captain
	if (v< 4641000)	return(12); // Major
	if (v< 6783000)	return(13); // Lieutenant Colonel
	if (v< 9690000)	return(14); // Colonel
	if (v<13566000)	return(15); // Brigadier General
	if (v<18653250)	return(16); // Major General
	if (v<25236750)	return(17); // Lieutenant General
	if (v<33649000)	return(18); // General
	if (v<44275000)	return(19); // Field Marshal
	if (v<57557500)	return(20); // Knight
	if (v<74002500)	return(21); // Baron
	if (v<94185000)	return(22); // Earl
					return(23); // Warlord
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
				return 0;
}

int points2EL(int v)
{
	if (v<	525)			return 1;
	if (v<	1760)			return 2;
	if (v<	3781)			return 3;
	if (v<	7184)			return 4;
	if (v<	12186)			return 5;
	if (v<	19324)			return 6;
	if (v<	29377)			return 7;
	if (v<	43181)			return 8;
	if (v<	61693)			return 9;
	if (v<	85990)			return 10;
	if (v<	117506)			return 11;
	if (v<	157384)			return 12;
	if (v<	207736)			return 13;
	if (v<	269997)			return 14;
	if (v<	346462)			return 15;
	if (v<	439268)			return 16;
	if (v<	551295)			return 17;
	if (v<	685171)			return 18;
	if (v<	843709)			return 19;
	if (v<	1030734)		return 20;
	if (v<	1249629)		return 21;
	if (v<	1504995)		return 22;
	if (v<	1800847)		return 23;
	if (v<	2142652)		return 24;
	if (v<	2535122)		return 25;
	if (v<	2984677)		return 26;
	if (v<	3496798)		return 27;
	if (v<	4080655)		return 28;
	if (v<	4742836)		return 29;
	if (v<	5490247)		return 30;
	if (v<	6334393)		return 31;
	if (v<	7283446)		return 32;
	if (v<	8384398)		return 33;
	if (v<	9541110)		return 34;
	if (v<	10874351)		return 35;
	if (v<	12361842)		return 36;
	if (v<	14018289)		return 37;
	if (v<	15859432)		return 38;
	if (v<	17905634)		return 39;
	if (v<	20171471)		return 40;
	if (v<	22679999)		return 41;
	if (v<	25456123)		return 42;
	if (v<	28517857)		return 43;
	if (v<	31897771)		return 44;
	if (v<	35621447)		return 45;
	if (v<	39721017)		return 46;
	if (v<	44225461)		return 47;
	if (v<	49176560)		return 48;
	if (v<	54607467)		return 49;
	if (v<	60565335)		return 50;
	if (v<	67094245)		return 51;
	if (v<	74247659)		return 52;
	if (v<	82075627)		return 53;
	if (v<	90631041)		return 54;
	if (v<	99984974)		return 55;
	if (v<	110197515)		return 56;
	if (v<	121340161)		return 57;
	if (v<	133497202)		return 58;
	if (v<	146749362)		return 59;
	if (v<	161191120)		return 60;
	if (v<	176922628)		return 61;
	if (v<	194049893)		return 62;
	if (v<	212684946)		return 63;
	if (v<	232956711)		return 64;
	if (v<	255001620)		return 65;
	if (v<	278952403)		return 66;
	if (v<	304972236)		return 67;
	if (v<	333233648)		return 68;
	if (v<	363906163)		return 69;
	if (v<	397194041)		return 70;
	if (v<	433312945)		return 71;
	if (v<	472476370)		return 72;
	if (v<	514937180)		return 73;
	if (v<	560961898)		return 74;
	if (v<	610815862)		return 75;
	if (v<	664824416)		return 76;
	if (v<	723298169)		return 77;
	if (v<	786612664)		return 78;
	if (v<	855129128)		return 79;
	if (v<	929261318)		return 80;
	if (v<	1009443795)		return 81;
	if (v<	1096169525)		return 82;
	if (v<	1189918242)		return 83;
	if (v<	1291270350)		return 84;
	if (v<	1400795257)		return 85;
	if (v<	1519130326)		return 86;
	if (v<	1646943474)		return 87;
	if (v<	1784977296)		return 88;
	if (v<	1934009687)		return 89;
	
	return 90;
}

int EL2points(int v)
{
	if (v==	2)		return 525;
	if (v==	3)		return 1760;
	if (v==	4)		return 3781;
	if (v==	5)		return 7184;
	if (v==	6)		return 12186;
	if (v==	7)		return 19324;
	if (v==	8)		return 29377;
	if (v==	9)		return 43181;
	if (v==	10)		return 61693;
	if (v==	11)		return 85990;
	if (v==	12)		return 117506;
	if (v==	13)		return 157384;
	if (v==	14)		return 207736;
	if (v==	15)		return 269997;
	if (v==	16)		return 346462;
	if (v==	17)		return 439268;
	if (v==	18)		return 551295;
	if (v==	19)		return 685171;
	if (v==	20)		return 843709;
	if (v==	21)		return 1030734;
	if (v==	22)		return 1249629;
	if (v==	23)		return 1504995;
	if (v==	24)		return 1800847;
	if (v==	25)		return 2142652;
	if (v==	26)		return 2535122;
	if (v==	27)		return 2984677;
	if (v==	28)		return 3496798;
	if (v==	29)		return 4080655;
	if (v==	30)		return 4742836;
	if (v==	31)		return 5490247;
	if (v==	32)		return 6334393;
	if (v==	33)		return 7283446;
	if (v==	34)		return 8384398;
	if (v==	35)		return 9541110;
	if (v==	36)		return 10874351;
	if (v==	37)		return 12361842;
	if (v==	38)		return 14018289;
	if (v==	39)		return 15859432;
	if (v==	40)		return 17905634;
	if (v==	41)		return 20171471;
	if (v==	42)		return 22679999;
	if (v==	43)		return 25456123;
	if (v==	44)		return 28517857;
	if (v==	45)		return 31897771;
	if (v==	46)		return 35621447;
	if (v==	47)		return 39721017;
	if (v==	48)		return 44225461;
	if (v==	49)		return 49176560;
	if (v==	50)		return 54607467;
	if (v==	51)		return 60565335;
	if (v==	52)		return 67094245;
	if (v==	53)		return 74247659;
	if (v==	54)		return 82075627;
	if (v==	55)		return 90631041;
	if (v==	56)		return 99984974;
	if (v==	57)		return 110197515;
	if (v==	58)		return 121340161;
	if (v==	59)		return 133497202;
	if (v==	60)		return 146749362;
	if (v==	61)		return 161191120;
	if (v==	62)		return 176922628;
	if (v==	63)		return 194049893;
	if (v==	64)		return 212684946;
	if (v==	65)		return 232956711;
	if (v==	66)		return 255001620;
	if (v==	67)		return 278952403;
	if (v==	68)		return 304972236;
	if (v==	69)		return 333233648;
	if (v==	70)		return 363906163;
	if (v==	71)		return 397194041;
	if (v==	72)		return 433312945;
	if (v==	73)		return 472476370;
	if (v==	74)		return 514937180;
	if (v==	75)		return 560961898;
	if (v==	76)		return 610815862;
	if (v==	77)		return 664824416;
	if (v==	78)		return 723298169;
	if (v==	79)		return 786612664;
	if (v==	80)		return 855129128;
	if (v==	81)		return 929261318;
	if (v==	82)		return 1009443795;
	if (v==	83)		return 1096169525;
	if (v==	84)		return 1189918242;
	if (v==	85)		return 1291270350;
	if (v==	86)		return 1400795257;
	if (v==	87)		return 1519130326;
	if (v==	88)		return 1646943474;
	if (v==	89)		return 1784977296;
	if (v==	90)		return 1934009687;

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
        if (curr_level == 23) return 0;
        next_level = curr_level + 1;

		r = rank2points(next_level);
		j = r-curr_exp;
		
		return j;

        /*p0 = 1;
        p5 = 1;
        p9 = 20 * curr_exp;
        for (j=0; p0<p9 && j<100; j++) {
                p5 = (p0 + p9) / 2;
                r = points2rank(curr_exp + p5);
                if (r < next_level) {
                        p0 = p5 + 1;
                } else {
                        p9 = p5 - 1;
                }
        }
        if (p0 > (20*curr_exp)) return 0;       // Can't do it
        p5++;
        return p5;*/
}

int points_tolevel_EL(int curr_exp)
{
        int curr_level, next_level, r, j;  //, p0, p5, p9;

		if (!curr_exp) return 525;	//0 exp
        curr_level = points2EL(curr_exp);
        if (curr_level == 90) return 0;
        next_level = curr_level + 1;

		r = EL2points(next_level);
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

unsigned int    inv_pos=0,			// scrolling position of small inventory
show_shop=0;

unsigned int    skill_pos=0;

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

void eng_display_win(int plr_sprite,int init)
{
	int pl_speed, pl_atksp; //, pl_cstsp;
	int pl_spmod, pl_crit, pl_dps, pl_cool, pl_baselen;
	int pl_tarot = 100;
	int y,n,m;
	char *tmp,buf[50];
	//static int wntab[20]={
    //    WN_HEAD, WN_CLOAK,WN_BODY, WN_ARMS,
	//	WN_NECK, WN_BELT, WN_RHAND,WN_LHAND,
	//	WN_RRING,WN_LRING,WN_LEGS, WN_FEET,
	//	0,0,0,0,0,0,0,0};

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
			if (pl.item[n+inv_pos]) {
				if (hightlight==HL_BACKPACK && hightlight_sub==n+(signed)inv_pos)
					copyspritex(pl.item[n+inv_pos],261+(n%10)*34,6+(n/10)*34,16);
				else
					copyspritex(pl.item[n+inv_pos],261+(n%10)*34,6+(n/10)*34,0);
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
		
		// Hacky stuff
		
		// Player Speed - WN_SPEED
		// Between 0 and 35, adding 1 so it is between 1 and 36.
		// Final value will be x/24 to get the APS between 0.04 and 1.50
		pl_speed = 36-pl.worn[WN_SPEED];
		pl_atksp = pl_speed + pl.attrib[AT_AGL][5]/30; if (pl_atksp > 36) pl_atksp = 36;
		
		// Player Spell Mod - WN_SPMOD
		// Should be regular spell mod times 100. Simply divide by 100 for the mod value.
		pl_spmod = pl.worn[WN_SPMOD];
		
		// Player Crit mod - WN_CRIT
		// Crit mod is going to be 1.06 on average. It becomes 1.12 if the player is using a dagger or twohander
		pl_crit = pl.worn[WN_CRIT];

		if (pl.worn[WN_CHARM]==2808||pl.worn[WN_CHARM2]==2808) // Strength - gotta use the sprite here.
			pl_tarot = 125;

		pl_baselen = 100;
		
		if (pl.worn[WN_LHAND]==2970) // Book: Damor's Grudge
			pl_baselen = 90;

		// Player DPS
		// Gets the average of the low and high Damage Per Hit value. Multiplies by 100 so it can be rounded to the 2nd decimal
		if (pl.worn[WN_CHARM]==2801||pl.worn[WN_CHARM2]==2801) // Magician
		{
			pl_dps  = ((((pl.weapon+((pl.weapon+8+pl.attrib[AT_INT][5]/2+6)*pl_crit/10000))/2)*100*pl_atksp/12)*pl_tarot/100)/4;
			pl_cool = 10000/(pl_baselen-pl.attrib[AT_STR][5]/4);
			dd_xputtext(GUI_DPS_X+103,GUI_DPS_Y+14*1,1,"%3d - %3d",
				((pl.weapon)*pl_tarot/100)/4,
				(((pl.weapon+8+pl.attrib[AT_INT][5]/2+6)*pl_crit/10000)*pl_tarot/100)/4);
			dd_xputtext(GUI_DPS_X+103,GUI_DPS_Y+14*5,1,"%9d",(pl.attrib[AT_WIL][0]+pl.attrib[AT_STR][0])*pl_spmod/100);
		}
		else
		{
			pl_dps  = ((((pl.weapon+((pl.weapon+8+pl.attrib[AT_STR][5]/2+6)*pl_crit/10000))/2)*100*pl_atksp/12)*pl_tarot/100)/4;
			pl_cool = 10000/(pl_baselen-pl.attrib[AT_INT][5]/4);
			dd_xputtext(GUI_DPS_X+103,GUI_DPS_Y+14*1,1,"%3d - %3d",
				((pl.weapon)*pl_tarot/100)/4,
				(((pl.weapon+8+pl.attrib[AT_STR][5]/2+6)*pl_crit/10000)*pl_tarot/100)/4);
			dd_xputtext(GUI_DPS_X+103,GUI_DPS_Y+14*5,1,"%9d",(pl.attrib[AT_WIL][0]+pl.attrib[AT_INT][0])*pl_spmod/100);
		}
		//
		
		// Hidden values
		dd_xputtext(GUI_DPS_X,    GUI_DPS_Y+14*0,1,"Damage per Sec");
		dd_xputtext(GUI_DPS_X+103,GUI_DPS_Y+14*0,1,"%6d.%02d",pl_dps/100,pl_dps%100);
		dd_xputtext(GUI_DPS_X,    GUI_DPS_Y+14*1,1,"Damage per Hit");
		dd_xputtext(GUI_DPS_X,    GUI_DPS_Y+14*2,1,"Attack Speed");
		dd_xputtext(GUI_DPS_X+103,GUI_DPS_Y+14*2,1,"%6d.%02d",pl_atksp/12,pl_atksp%12*100/12);
		dd_xputtext(GUI_DPS_X,    GUI_DPS_Y+14*3,1,"Cooldown Rate");
		dd_xputtext(GUI_DPS_X+103,GUI_DPS_Y+14*3,1,"%6d.%02d",pl_cool/100,pl_cool%100);
		dd_xputtext(GUI_DPS_X,    GUI_DPS_Y+14*4,1,"Spell Modifier");
		dd_xputtext(GUI_DPS_X+103,GUI_DPS_Y+14*4,1,"%6d.%02d",pl_spmod/100,pl_spmod%100);
		dd_xputtext(GUI_DPS_X,    GUI_DPS_Y+14*5,1,"Spell Aptitude");
		/*
		#ifdef HOMECOPY
		dd_xputtext(GUI_DPS_X,    GUI_DPS_Y+14*6,1,"PL_CRIT Value");
		dd_xputtext(GUI_DPS_X+103,GUI_DPS_Y+14*6,1,"%8d",pl_crit);
		#endif
		*/
	
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
	if (pl.points_tot>0) 
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
	dd_showbar(GUI_XPBAR_X,GUI_XPBAR_Y,GUI_XPBAR_W,6,(unsigned short)GUI_BAR_BLU);
	dd_showbar(GUI_XPBAR_X,GUI_XPBAR_Y,n,          6,(unsigned short)GUI_BAR_EXP);
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
		if (show_shop) show_look=0;
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
				copyspritex(3, 775, 15,  0);
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
				copyspritex(10+min(20,points2rank(pl.points_tot)),935,42,0);
				copyspritex(plr_sprite,935-61,36,0);
				dd_xputtext(846+(125-strlen(pl.name)*6)/2,157,1,pl.name);
				dd_xputtext(846+(125-strlen(rank[points2rank(pl.points_tot)])*6)/2,176,1,rank[points2rank(pl.points_tot)]);
			}

		} else {
			if (look.worn[WN_CHARM2])
			{
				copyspritex(3, 775, 15,  0);
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

			copyspritex(10+min(20,points2rank(look.points)),935,42,0);
		}

		if (show_shop) 
		{
			copyspritex(92,GUI_SHOP_X,GUI_SHOP_Y,0);
			for (n=0; n<62; n++) {
				if (!shop.item[n]) continue;
				if (hightlight==HL_SHOP && hightlight_sub==n) 
				{
					copyspritex(shop.item[n],GUI_SHOP_X+2+(n%8)*35,GUI_SHOP_Y+2+(n/8)*35,16);
					if (shop.price[n]) 
						dd_xputtext(GUI_SHOP_X+5,GUI_SHOP_Y+299,1,"Sell: %dG %dS",shop.price[n]/100,shop.price[n]%100);
				} 
				else copyspritex(shop.item[n],GUI_SHOP_X+2+(n%8)*35,GUI_SHOP_Y+2+(n/8)*35,0);
			}
			if (pl.citem && shop.pl_price)
				dd_xputtext(GUI_SHOP_X+5,GUI_SHOP_Y+299,1,"Buy:  %dG %dS",shop.pl_price/100,shop.pl_price%100);

			if (shop.sprite) copyspritex(shop.sprite,935-61,36,0);
			copyspritex(10+min(20,points2rank(shop.points)),935,42,0);
			dd_xputtext(846+(125-strlen(rank[points2rank(shop.points)])*6)/2,176,1,rank[points2rank(shop.points)]);
			dd_xputtext(846+(125-strlen(shop.name)*6)/2,157,1,shop.name);
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

void dd_show_map(unsigned short *src,int xo,int yo);

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
	int x,y,rx,ry,m,plr_sprite,tmp,mapx,mapy,selected_visible=0,alpha,alphastr;
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
					dd_gputtext(x*32,y*32,1,lookup(map[m].ch_nr,map[m].ch_id),xoff+map[m].obj_xoff,yoff+map[m].obj_yoff-8);
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

	copyspritex(screen_overlay_sprite,0,0,0);

	if (init) {
		xmap[mapy+mapx*MAPX_MAX]=0xffff;

		mapx-=64;
		if (mapx<0)	mapx=0;
		if (mapx>MAPX_MAX-129) mapx=MAPX_MAX-129;

		mapy=mapy-64;
		if (mapy<0)	mapy=0;
		if (mapy>MAPY_MAX-129) mapy=MAPY_MAX-129;

		dd_show_map(xmap,mapy,mapx);
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

unsigned char speedtab[36][24]=
{
//   1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0,1,2,3,4
	{2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1},	// 36
	{1,1,2,1,2,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,2},	// 35
	{2,1,2,1,2,1,1,2,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,1},	// 34
	{1,1,2,1,1,2,1,2,1,1,2,1,1,2,1,2,1,1,2,1,1,2,1,2},	// 33
	{1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1},	// 32
	{2,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,1},	// 31
	{1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1},	// 30
	{1,1,1,2,1,1,1,1,2,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1},	// 29
	{1,1,2,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1},	// 28
	{1,1,1,2,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,2,1,1,1,1},	// 27
	{1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1},	// 26
	{1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1},	// 25
	{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},	// 24 - Previously 20
	{1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1},	// 23
	{1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1},	// 22
	{1,1,1,0,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,0,1,1,1,1},	// 21
	{1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1},	// 20
	{1,1,1,0,1,1,1,1,0,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1},	// 19
	{1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1},	// 18
	{0,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,1},	// 17
	{1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1},	// 16
	{1,1,0,1,1,0,1,0,1,1,0,1,1,0,1,0,1,1,0,1,1,0,1,0},	// 15
	{0,1,0,1,0,1,1,0,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,1},	// 14
	{1,1,0,1,0,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,0},	// 13
	{1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0},	// 12
	{0,0,1,0,1,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,1},	// 11
	{1,0,1,0,1,0,0,1,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0},	// 10
	{0,0,1,0,0,1,0,1,0,0,1,0,0,1,0,1,0,0,1,0,0,1,0,1},	//  9
	{0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0},	//  8
	{1,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,0},	//  7
	{0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0},	//  6
	{0,0,0,1,0,0,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0},	//  5
	{0,0,1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0},	//  4
	{0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0},	//  3
	{0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0},	//  2
	{0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0}	//  1
};

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

int speedo(int n)
{
	return speedtab[map[n].ch_speed][ctick];
}

// Feb 2020 - better FIGHT speed with better agl, better CAST speed with better int
int speedoMisc(int n)
{
	int miscSpeedValue = map[n].ch_speed;
	
	switch(map[n].ch_stat_off)
	{
		// 0, 5, 6 == Attack
		case    0:
		case    5:
		case    6:
			miscSpeedValue -= map[n].ch_agl;
			if (miscSpeedValue < 0) miscSpeedValue = 0;
			return(speedtab[miscSpeedValue][ctick]);
			
		// 9 == Use skill, mostly casting
		case    9:
			miscSpeedValue -= map[n].ch_brv;
			if (miscSpeedValue < 0) miscSpeedValue = 0;
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
		if (z<0) z=23;	// Feb 2020 - ctick extended from 20 to 24
		soft_step++;
		if (speedtab[speed][z])	m--;
	}
	while (1) {
		z--;
		if (z<0) z=23;	// Feb 2020 - ctick extended from 20 to 24
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
		if (z>23) z=0;	// Feb 2020 - ctick extended from 20 to 24
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
		case    2:    if (speedtab[24][ctick]) map[n].it_status++;	// Feb 2020 - adjusted these speedtab values from '10' to '24' (same internal array)
			return map[n].it_sprite;

		case    3:    if (speedtab[24][ctick]) map[n].it_status++;
			return map[n].it_sprite+2;

		case    4:    if (speedtab[24][ctick]) map[n].it_status++;
			return map[n].it_sprite+4;

		case    5:    if (speedtab[24][ctick]) map[n].it_status=2;
			return map[n].it_sprite+6;

			// two sprite animation, 1-step
		case    6:    if (speedtab[24][ctick]) map[n].it_status++;
			return map[n].it_sprite;

		case    7:    if (speedtab[24][ctick]) map[n].it_status=6;
			return map[n].it_sprite+1;

			// eight sprite animation, 1-step
		case    8:    if (speedtab[24][ctick]) map[n].it_status++;
			return map[n].it_sprite;

		case    9:    if (speedtab[24][ctick]) map[n].it_status++;
			return map[n].it_sprite+1;

		case    10:   if (speedtab[24][ctick]) map[n].it_status++;
			return map[n].it_sprite+2;

		case    11:   if (speedtab[24][ctick]) map[n].it_status++;
			return map[n].it_sprite+3;

		case   12:   if (speedtab[24][ctick]) map[n].it_status++;
			return map[n].it_sprite+4;

		case    13:   if (speedtab[24][ctick]) map[n].it_status++;
			return map[n].it_sprite+5;

		case    14:   if (speedtab[24][ctick]) map[n].it_status++;
			return map[n].it_sprite+6;

		case    15:   if (speedtab[24][ctick]) map[n].it_status=8;
			return map[n].it_sprite+7;

			// five sprite animation, 1-step, random
		case    16:   if (speedtab[24][ctick]) map[n].it_status++;
			return map[n].it_sprite;

		case    17:   if (speedtab[24][ctick]) map[n].it_status++;
				return map[n].it_sprite+1;

		case    18:   	if (speedtab[24][ctick]) map[n].it_status++;
				return map[n].it_sprite+2;

		case    19:   	if (speedtab[24][ctick]) map[n].it_status++;
				return map[n].it_sprite+3;

		case   20:   	if (speedtab[24][ctick]) map[n].it_status=16;
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
int maynotquit=TICKS*5;

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

		if (show_shop && lookstep>QSIZE) {
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

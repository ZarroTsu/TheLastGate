// **************************************************************** 
//
//  The Last Gate - SDL2 Client by Zachary "ZarroTsu" Baillie
//  Based on the Mercenaries of Astonia engine by Daniel Brockhaus
//  
//  common.h
//  Common variable defininitions and structs,
//   includes network-related functionality.
//
// **************************************************************** 
#ifndef COMMON_H
#define COMMON_H

// Core file inclusions
#include "SDL2/SDL.h"
#include "SDL2/SDL_image.h"
#include "SDL2/SDL_ttf.h"
#include <fcntl.h>
#include <stdio.h>
#include <windows.h>

// Comment or un-comment this define to toggle internal and external network connection
#define HOMECOPY

// Used for debugging client - causes a CMD to open with the client to write output to
#define DEBUGMODE

#define TICKS			60
#define TICK			(1000/TICKS)

#define MAXSKILL		(50+2)	// must match server!
#define MAXBUFFS		40		// must match server!
#define MAXWPS			24

#define QSIZE			8

#define CLICKVOL		100

//
#define MAXSPRITE 2000+(128*1024)

#define MAPX_MAX		1024
#define MAPY_MAX		2048

#define MS_MOVE		    0
#define MS_LB_DOWN	    1
#define MS_RB_DOWN	    2
#define MS_LB_UP		3
#define MS_RB_UP		4

// Item worn positions & packet hacks
#define WN_HEAD			0
#define WN_NECK			1
#define WN_BODY			2
#define WN_ARMS			3
#define WN_BELT			4
#define WN_CHARM		5
#define WN_FEET			6
#define WN_LHAND		7	// shield
#define WN_RHAND		8	// weapon
#define WN_CLOAK		9
#define WN_LRING		10
#define WN_RRING		11
#define WN_CHARM2		12
#define WN_SPEED		13  // Speed 	and Attack Speed
#define WN_SPMOD		14  // Spellmod	and Spellapt
#define WN_CRIT			15  // Crit Chc	and Crit Mult
#define WN_TOP			16  // TopDmg	and Luck
#define WN_HITPAR		17  // Hit 		and Parry
#define WN_CLDWN		18  // Cooldown and Cast Speed
#define WN_FLAGS		19  // Special flags

// Item placement bits
#define PL_HEAD			1
#define PL_NECK			2
#define PL_BODY			4
#define PL_ARMS			8
//
#define PL_BELT			32
#define PL_CHARM		64
#define PL_FEET			128
#define PL_WEAPON		256
#define PL_SHIELD		512
#define PL_CLOAK		1024
#define PL_TWOHAND		2048
#define PL_RING			4096
//
#define PL_SOULSTONED	16384
#define PL_ENCHANTED	32768

// Map animation flags
#define INJURED			(1u<<0)
#define INJURED1		(1u<<1)
#define INJURED2		(1u<<2)
#define STONED			(1u<<3)
#define INFRARED		(1u<<4)
#define UWATER			(1u<<5)
//
#define ISUSABLE		(1u<<7)
#define ISITEM			(1u<<8)
#define ISCHAR			(1u<<9)
#define INVIS			(1u<<10)
#define STUNNED			(1u<<11)
#define TOMB			((1u<<12)|(1u<<13)|(1u<<14)|(1u<<15)|(1u<<16))
#define DEATH			((1u<<17)|(1u<<18)|(1u<<19)|(1u<<20)|(1u<<21))
#define EMAGIC			((1U<<22)|(1U<<23)|(1U<<24))
#define GMAGIC			((1U<<25)|(1U<<26)|(1U<<27))
#define CMAGIC			((1U<<28)|(1U<<29)|(1U<<30))
#define CRITTED			(1U<<31)

// Map tile flags
#define MF_MOVEBLOCK	(1U<<0)
#define MF_SIGHTBLOCK	(1U<<1)
#define MF_INDOORS		(1U<<2)
#define MF_UWATER		(1U<<3)
#define MF_NOLAG		(1U<<4)
#define MF_NOMONST		(1U<<5)
#define MF_BANK			(1U<<6)
#define MF_TAVERN		(1U<<7)
#define MF_NOMAGIC		(1U<<8)
#define MF_DEATHTRAP	(1U<<9)
//
#define MF_ARENA		(1U<<11)
//
#define MF_NOEXPIRE		(1U<<13)
#define MF_NOFIGHT		(1U<<14)

// Sprite animation frames
#define SPF_IDLE_UP					   0
#define SPF_IDLE_DOWN				   8
#define SPF_IDLE_LEFT				  16
#define SPF_IDLE_RIGHT				  24
#define SPF_IDLE_LEFTUP				  32
#define SPF_IDLE_LEFTDOWN			  40
#define SPF_IDLE_RIGHTUP			  48
#define SPF_IDLE_RIGHTDOWN			  56
#define SPF_WALK_UP					  64
#define SPF_WALK_DOWN				  72
#define SPF_WALK_LEFT				  80
#define SPF_WALK_RIGHT				  88
#define SPF_WALK_LEFTUP				  96
#define SPF_WALK_LEFTDOWN			 104
#define SPF_WALK_RIGHTUP			 112
#define SPF_WALK_RIGHTDOWN			 120
#define SPF_TURN_UPLEFTUP			 128
#define SPF_TURN_LEFTUPUP			 132
#define SPF_TURN_UPRIGHTUP			 136
#define SPF_TURN_RIGHTUPRIGHT		 140
#define SPF_TURN_DOWNLEFTDOWN		 144
#define SPF_TURN_LEFTDOWNLEFT		 148
#define SPF_TURN_DOWNRIGHTDOWN		 152
#define SPF_TURN_RIGHTDOWNDOWN		 156
#define SPF_TURN_LEFTLEFTUP			 160
#define SPF_TURN_LEFTUPLEFT			 164
#define SPF_TURN_LEFTLEFTDOWN		 168
#define SPF_TURN_LEFTDOWNDOWN		 172
#define SPF_TURN_RIGHTRIGHTUP		 176
#define SPF_TURN_RIGHTUPUP			 180
#define SPF_TURN_RIGHTRIGHTDOWN		 184
#define SPF_TURN_RIGHTDOWNRIGHT		 188
#define SPF_MISC_UP					 192
#define SPF_MISC_DOWN				 200
#define SPF_MISC_LEFT				 208
#define SPF_MISC_RIGHT				 216

// Default option settings
#define DEF_FULLSCREEN		0
#define DEF_MAGNIFY			0
#define DEF_MUSIC			0
#define DEF_SOUND			1
#define DEF_ALPHA			2
#define DEF_SHADOWS			1
#define DEF_DARKMODE		0

// Colors
#define COLOR_GUI_RED	0xFF0000FF
#define COLOR_GUI_GRE	0x00FF00FF
#define COLOR_GUI_BLU	0x0000FFFF
#define COLOR_GUI_YEL	0xFFFF00FF

// Game Directions
#define DX_RIGHT		1
#define DX_LEFT			2
#define DX_UP			3
#define DX_DOWN			4

// Game Enumerators
typedef enum
{
	SCENE_OPENER,
    SCENE_TITLE,
	SCENE_CHARACTER,
	SCENE_GAMELOOP
	
} SCE_Scene;

// Game option struct - for loading/storing/saving local game variables
struct Opts {
	char fullscreen;
	char magnify;
	char music;
	char sound;
	char alpha;
	char shadows;
	char darkmode;
} options;

// Local character file struct - for storing and saving local character files
struct Key {
	unsigned int usnr;
	unsigned int pass1,pass2;
	char name[40];
	int race;
} okey;

struct look
{
	unsigned char autoflag;
	unsigned short worn[20];
	unsigned short sprite;
	unsigned int points;
	char name[40];
	unsigned int hp;
	unsigned int end;
	unsigned int mana;
	unsigned int a_hp;
	unsigned int a_end;
	unsigned int a_mana;
	unsigned short nr;
	unsigned short id;
	unsigned char extended;
	unsigned short item[62];
	unsigned int price[62];
	unsigned int pl_price;
};

struct skilltab
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

struct wpslist
{
	int nr;
	
	char name[30];
	char desc[30];
};

struct xbutton
{
   char name[8];
   int skill_nr;
};

struct pdata
{
	char cname[80];
	char ref[80];
	char desc[160];

	char changed;

	int hide;
	int show_names;
	int show_proz;
	int show_bars;
	int show_stats;
	struct xbutton xbutton[16];
} pdata;

struct cmap {
	// common:

	unsigned short x,y;				// position

	// for background
	short int ba_sprite;			// background image
	unsigned char light;
	unsigned int flags;
	unsigned int flags2;

	// for character
	unsigned short ch_sprite;		// basic sprite of character
	unsigned char ch_status;		// what the character is doing, animation-wise
	unsigned char ch_stat_off;
	short int ch_speed;				// speed of animation
	unsigned short ch_nr;			// character id
	unsigned short ch_id;			// character 'crc'
	unsigned char ch_proz;
	short int ch_castspd;			// animation speed bonus for misc. actions (casting)
	short int ch_atkspd;			// animation speed bonus for attacking
	short int ch_movespd;			// animation speed bonus for walking
	unsigned char ch_fontcolor;		// overhead text font color. 1 = red, 0 = yellow, green/blue same as server

	// for item
	short int it_sprite;			// basic sprite of item
	unsigned char it_status;		// for items with animation (burning torches etc)

	// for local computation -- client only:
	int back;	// background
	int obj1;	// item
	int obj2;	// character
//	int obj2f;	// character animation frame

	int obj_xoff,obj_yoff;
	int ovl_xoff,ovl_yoff;

	int idle_ani;
};

struct cplayer {
	// informative stuff
	char name[40];

	int mode;	// 0 = slow, 1 = medium, 2 = fast

	// character attributes+abilities
	// [0]=bare value, [1]=modifier, [2]=total value
	int attrib[5][6];
	
	int hp[6];
	int end[6];
	int mana[6];
	
	int skill[100][6];

	// temporary attributes
	int a_hp;
	int a_end;
	int a_mana;

	int points;
	int points_tot;
	int kindred;

	// possessions
	int gold;
	int bs_points;  // Point total for Black Stronghold
	int os_points;  // Points for future content
	int tokens; 	// Points for gambling
	int waypoints;

	// items carried
	int item[40];
	int item_p[40];
	char item_s[40];  // Stack size of given item (0 - 10)
	char item_l[40];  // Whether or not the given item is locked or stoned

	// items worn
	int worn[20];
	int worn_p[20];

	// spells ready
	short spell[MAXBUFFS];
	char active[MAXBUFFS];

	int armor;
	int weapon;

	int citem, citem_p;
	char citem_s;  // Stack size of given item (0 - 10)

	int attack_cn;
	int goto_x,goto_y;
	int misc_action,misc_target1,misc_target2;
	int dir;
} pl;

extern int ctick;

#include "funcs.h"

#endif // COMMON_H
/* END OF FILE */
/*************************************************************************

   This file is part of 'Mercenaries of Astonia v2'
   Copyright (c) 1997-2001 Daniel Brockhaus (joker@astonia.com)
   All rights reserved.

 **************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <unistd.h>
#include <sys/mman.h>
#include <errno.h>
#include "cgi-lib.h"
#include "html-lib.h"
#include "gendefs.h"
#include "data.h"
#include "macros.h"

struct character *ch_temp;
struct character *ch;
struct item *it;
struct item *it_temp;
struct global *globs;

#if 0
unsigned long long atoll(char *string)
{
	unsigned long long val = 0;

	while (isspace(*string)) string++;

	while (isdigit(*string))
	{
		val *= 10;
		val += *string - '0';
		string++;
	}
	return(val);
}
#endif

// Attribute nameplates
char *at_name[5] = {
	"Braveness", "Willpower", "Intuition", "Agility", "Strength"
};

// The order to display skill fields
static int skillslots[MAXSKILL] = {
	28,29,30, 0,12, 4, 2,36,16, 5,
	 3, 6,37,40,49,22,13,48,35,41,
	24,21,20,25,18,27,47,26,15,11,
	42,17,43,46,19,31,45,34,14,38,
	32,10, 9, 1,44,23,39, 8,33, 7
};

// The order to display item slot fields
static int itemslots[MAXGSLOTS] = { 
	 8, 7, 0, 9, 2, 3, 6, 1, 4,11,10, 5,12
};

// Slot nameplates
static char *weartext[MAXGSLOTS] = {
	"Head",
	"Neck",
	"Body",
	"Arms",
	"Belt",
	"Charm",
	"Feet",
	"Left Hand",
	"Right Hand",
	"Cloak",
	"Left Ring",
	"Right Ring",
	"Charm 2"
};

// Text use nameplates
static char *text_name[10] = {
	"Killed Enemy $1",				//  0
	"Will attack $1",				//  1
	"Greeting $1",					//  2
	"Killed by $1",					//  3
	"Shout Help against $1",		//  4
	"Going to Help $1",				//  5
	"Keyword",						//  6
	"Reaction to keyword",			//  7
	"Warning about attack",			//  8
	"Unused",						//  9
};

// Player text nameplates
static char *player_text_name[10] = {
	"(Temporary) AFK Message",		//  0
	"(Temporary)",					//  1
	"(Temporary)",					//  2
	"Mark Message",					//  3
	"Unused",						//  4
	"Unused",						//  5
	"Unused",						//  6
	"Unused",						//  7
	"Unused",						//  8
	"Unused",						//  9
};

// Player data nameplates
static char *player_data_name[100] = {
	"Away from Keyboard",			//  0
	"Group With (1)",				//  1
	"Group With (2)",				//  2
	"Group With (3)",				//  3
	"Group With (4)",				//  4
	"Group With (5)",				//  5
	"Group With (6)",				//  6
	"Group With (7)",				//  7
	"Group With (8)",				//  8
	"Group With (9)",				//  9
	"Following",					// 10
	"Petrified Blood DoT",			// 11
	"Follow Timeout",				// 12
	"Money in Bank",				// 13
	"Number of Deaths",				// 14
	"Killed By X",					// 15
	"Date",							// 16
	"Area",							// 17
	"Current Pent Exp",				// 18
	"Lag Timer",					// 19
	"Highest Lab Gorge",			// 20
	"Seyan Shrines Used",			// 21
	"Current Arena Tier",			// 22
	"Overall Kill Counter",			// 23
	"EXP Pole Flags (6)",			// 24
	"Underdark Entry 'M'",			// 25
	"BS Kill Counter",				// 26
	"Gambling: Player Hand",		// 27
	"Gambling: Dealer Hand",		// 28
	"Other Players Killed",			// 29
	"Soft Ignore List (1)",			// 30
	"Soft Ignore List (2)",			// 31
	"Soft Ignore List (3)",			// 32
	"Soft Ignore List (4)",			// 33
	"Soft Ignore List (5)",			// 34
	"Soft Ignore List (6)",			// 35
	"Soft Ignore List (7)",			// 36
	"Soft Ignore List (8)",			// 37
	"Soft Ignore List (9)",			// 38
	"Soft Ignore List (X)",			// 39
	"Shops Killed",					// 40
	"Contract Mission",				// 41
	"Group",						// 42
	"Contract Item ID",				// 43
	"# Saved by the Gods",			// 44
	"Level X HP/EN/MA Bonus",		// 45
	"EXP Pole Flags (1)",			// 46
	"EXP Pole Flags (2)",			// 47
	"EXP Pole Flags (3)",			// 48
	"EXP Pole Flags (4)",			// 49
	"Hard Ignore List (1)",			// 50
	"Hard Ignore List (2)",			// 51
	"Hard Ignore List (3)",			// 52
	"Hard Ignore List (4)",			// 53
	"Hard Ignore List (5)",			// 54
	"Hard Ignore List (6)",			// 55
	"Hard Ignore List (7)",			// 56
	"Hard Ignore List (8)",			// 57
	"Hard Ignore List (9)",			// 58
	"Hard Ignore List (X)",			// 59
	"First Kill Flags (1)",			// 60
	"First Kill Flags (2)",			// 61
	"First Kill Flags (3)",			// 62
	"First Kill Flags (4)",			// 63
	"Current Ghost Comp",			// 64
	"Player #ALLOWed",				// 65
	"Corpse's Owner",				// 66
	"First Kill Flags (8)",			// 67
	"Date of Last Pl Attack",		// 68
	"Last Player Attacked",			// 69
	"First Kill Flags (5)",			// 70
	"Number of Actions",			// 71
	"Quest Flags (1)",				// 72
	"First Kill Flags (7)",			// 73
	"NPC Spell Delays (1)",			// 74
	"NPC Spell Delays (2)",			// 75
	"Waypoint Flags",				// 76
	"Pent Kill Streak",				// 77
	"Highest Pent",					// 78
	"Latest Version MotD",			// 79
	"Last Login From (1)",			// 80
	"Last Login From (2)",			// 81
	"Last Login From (3)",			// 82
	"Last Login From (4)",			// 83
	"Last Login From (5)",			// 84
	"Last Login From (6)",			// 85
	"Last Login From (7)",			// 86
	"Last Login From (8)",			// 87
	"Last Login From (9)",			// 88
	"Last Login From (X)",			// 89
	"Number in Database",			// 90
	"EXP Pole Flags (5)",			// 91
	"Sleep Timer",					// 92
	"First Kill Flags (6)",			// 93
	"Quest Flags (2)",				// 94
	"Current Shadow Copy",			// 95
	"Queued Spells",				// 96
	"Comp Time Last Action",		// 97
	"Comp Time Used",				// 98
	"Used by Populate"				// 99
};

static char *data_name[100] = {
	"Generic 1",					//  0
	"Generic 2",					//  1
	"Generic 3",					//  2
	"Generic 4",					//  3
	"Generic 5",					//  4
	"Generic 6",					//  5
	"Generic 7",					//  6
	"Generic 8",					//  7
	"Generic 9",					//  8
	"Generic 10",					//  9
	"Patrol",						// 10
	"Patrol",						// 11
	"Patrol",						// 12
	"Patrol",						// 13
	"Patrol",						// 14
	"Patrol",						// 15
	"Patrol",						// 16
	"Patrol",						// 17
	"Patrol",						// 18
	"Reserved",						// 19
	"Close Door",					// 20
	"Close Door",					// 21
	"Close Door",					// 22
	"Close Door",					// 23
	"Prevent Fights",				// 24
	"Special Driver",				// 25
	"Special Sub-Driver",			// 26
	"Reserved",						// 27
	"Unused",						// 28
	"Resting Position",				// 29
	"Resting Dir",					// 30
	"Protect Templ",				// 31
	"Activate Light",				// 32
	"Activate Light",				// 33
	"Activate Light",				// 34
	"Activate Light",				// 35
	"Reserved",						// 36
	"Greet everyone",				// 37
	"Reserved",						// 38
	"Reserved",						// 39
	"Reserved",						// 40
	"Street Light Templ.",			// 41
	"Group",						// 42
	"Attack all but Group",			// 43
	"Attack all but Group",			// 44
	"Attack all but Group",			// 45
	"Attack all but Group",			// 46
	"Donate Trash to pl #",			// 47
	"Prob. of dying msg",			// 48
	"Wants item #",					// 49
	"Teaches Skill #",				// 50
	"Gives # EXPs",					// 51
	"Shout help/code #",			// 52
	"Come help/code #",				// 53
	"Reserved",						// 54
	"Reserved",						// 55
	"Reserved",						// 56
	"Reserved",						// 57
	"Reserved",						// 58
	"Protect Group",				// 59
	"Random Walk Time",				// 60
	"Reserved",						// 61
	"Create Light",					// 62
	"Reserved",						// 63
	"Self Destruct",				// 64
	"Reserved",						// 65
	"Gives Item #",					// 66
	"Reserved",						// 67
	"Level of Knowledge",			// 68
	"Accepts Money",				// 69
	"Reserved",						// 70
	"Talkativity",					// 71
	"Area of Knowledge",			// 72
	"Random Walk Max Dist",			// 73
	"Reserved",						// 74
	"Reserved",						// 75
	"Reserved",						// 76
	"Reserved",						// 77
	"Reserved",						// 78
	"Rest between patrol",			// 79
	"Reserved",						// 80
	"Reserved",						// 81
	"Reserved",						// 82
	"Reserved",						// 83
	"Reserved",						// 84
	"Reserved",						// 85
	"Reserved",						// 86
	"Reserved",						// 87
	"Reserved",						// 88
	"Reserved",						// 89
	"Reserved",						// 90
	"Reserved",						// 91
	"Reserved",						// 92
	"Attack distance",				// 93
	"Reserved",						// 94
	"Keyword Action",				// 95
	"Reserved",						// 96
	"Unused",						// 97
	"Unused",						// 98
	"Reserved"						// 99
};

int extend(int handle, long sizereq, size_t sizeone, void*templ)
{
	long length;
	void *buffer;
	int success;
	
	length = lseek(handle, 0L, SEEK_END);
	if (length < sizereq)
	{
		fprintf(stderr, "Current size = %ldK, extending to %ldK", length / 1024, sizereq / 1024);
		buffer = templ;
		if (buffer == NULL) buffer = calloc(1, sizeone);		// automatically clears memory
		if (buffer == NULL) return 0;							// calloc() failure
		for (; length < sizereq; length += sizeone) success = write(handle, buffer, sizeone);
		if (templ == NULL) free(buffer);
	}
	return 1; // success
}

int web_load_err(int v, char *wlv)
{
	switch (v)
	{
		case  1: fprintf(stderr, "Unable to load %s",   wlv); break;
		case  2: fprintf(stderr, "Unable to extend %s", wlv); break;
		case  3: fprintf(stderr, "Unable to mmap %s",   wlv); break;
		default: break;
	}
	return -1;
}

static int web_load(void)
{
	int handle = 0;
	
	// (ch) CHAR
	fprintf(stderr, "Loading CHAR: Obj size=%d, file size=%dK", sizeof(struct character), CHARSIZE >> 10);
	handle = open(DATDIR "/char.dat", O_RDWR);
	if (handle==-1) return web_load_err(1, "CHAR");
	if (!extend(handle, CHARSIZE, sizeof(struct character), NULL)) return web_load_err(2, "CHAR");
	ch = mmap(NULL, CHARSIZE, PROT_READ | PROT_WRITE, MAP_SHARED, handle, 0);
	if (ch==(void*)-1) return web_load_err(3, "CHAR");
	close(handle);
	
	// (ch_temp) TCHAR
	fprintf(stderr, "Loading TCHAR: Obj size=%d, file size=%dK", sizeof(struct character), TCHARSIZE >> 10);
	handle = open(DATDIR "/tchar.dat", O_RDWR);
	if (handle==-1) return web_load_err(1, "TCHAR");
	if (!extend(handle, TCHARSIZE, sizeof(struct character), NULL)) return web_load_err(2, "TCHAR");
	ch_temp = mmap(NULL, TCHARSIZE, PROT_READ | PROT_WRITE, MAP_SHARED, handle, 0);
	if (ch_temp==(void*)-1) return web_load_err(3, "TCHAR");
	close(handle);
	
	// (it) ITEM
	fprintf(stderr, "Loading ITEM: Obj size=%d, file size=%dK", sizeof(struct item), ITEMSIZE >> 10);
	handle = open(DATDIR "/item.dat", O_RDWR);
	if (handle==-1) return web_load_err(1, "ITEM");
	if (!extend(handle, ITEMSIZE, sizeof(struct item), NULL)) return web_load_err(2, "ITEM");
	it = mmap(NULL, ITEMSIZE, PROT_READ | PROT_WRITE, MAP_SHARED, handle, 0);
	if (it==(void*)-1) return web_load_err(3, "ITEM");
	close(handle);
	
	// (it_temp) TITEM
	fprintf(stderr, "Loading TITEM: Obj size=%d, file size=%dK", sizeof(struct item), TITEMSIZE >> 10);
	handle = open(DATDIR "/titem.dat", O_RDWR);
	if (handle==-1) return web_load_err(1, "TITEM");
	if (!extend(handle, TITEMSIZE, sizeof(struct item), NULL)) return web_load_err(2, "TITEM");
	it_temp = mmap(NULL, TITEMSIZE, PROT_READ | PROT_WRITE, MAP_SHARED, handle, 0);
	if (it_temp==(void*)-1) return web_load_err(3, "TITEM");
	close(handle);
	
	// (globs) GLOBS
	fprintf(stderr, "Loading GLOBS: Obj size=%d, file size=%db", sizeof(struct global), sizeof(struct global));
	handle = open(DATDIR "/global.dat", O_RDWR);
	if (handle==-1) return web_load_err(1, "GLOBS");
	if (!extend(handle, GLOBSIZE, sizeof(struct global), NULL)) return web_load_err(2, "GLOBS");
	globs = mmap(NULL, sizeof(struct global), PROT_READ | PROT_WRITE, MAP_SHARED, handle, 0);
	if (globs==(void*)-1) return web_load_err(3, "GLOBS");
	close(handle);
	
	return 0;
}

void web_unload_err(char *wuv)
{
	fprintf(stderr, "ERROR: munmap(%s) %s", wuv, strerror(errno));
}

static void web_unload(void)
{
	fprintf(stderr, "Unloading data files");
	if (munmap(ch, CHARSIZE)) 					web_unload_err("ch");
	if (munmap(ch_temp, TCHARSIZE)) 			web_unload_err("ch_temp");
	if (munmap(it, ITEMSIZE)) 					web_unload_err("it");
	if (munmap(it_temp, TITEMSIZE)) 			web_unload_err("it_temp");
	if (munmap(globs, sizeof(struct global))) 	web_unload_err("globs");
}

void phtml_home(void)            { printf("<center><a href=/cgi-imp/acct.cgi>Home</a></center><br><br>\n"); }
void phtml_backhome(int v)       { printf("<center><a href=/cgi-imp/acct.cgi?step=%d>Back</a>|<a href=/cgi-imp/acct.cgi>Home</a></center><br><br>\n", v); }
void phtml_formpost(void)        { printf("<form method=post action=/cgi-imp/acct.cgi>\n"); }
void phtml_tab_title(char *v)    { printf("<tr><td valign=top>%s:</td><td>\n", v); }
void phtml_tab_open(void)        { printf("<table>\n"); }
void phtml_tab_close(void)       { printf("</table>\n"); }
void phtml_tab_close_break(void) { printf("</table><br>\n"); }
void phtml_row_player(int n)     { printf("<tr><td>%d:</td><td><a href=/cgi-imp/acct.cgi?step=20&cn=%d>%s</a></td></tr>", n, n, ch[n].name); }
void phtml_row_ctemplate(int n)
{
	printf("<tr><td>%d:</td><td><a href=/cgi-imp/acct.cgi?step=13&cn=%d>"
		"%s%30.30s%s</a></td><td>Pos: %d,%d</td><td>EXP: %'9d</td><td>Alignment: %d</td>"
		"<td>Group: %d</td><td>%4.4s %4.4s</td><td><a href=/cgi-imp/acct.cgi?step=15&cn=%d>Copy</a>"
		"</td><td><a href=/cgi-imp/acct.cgi?step=12&cn=%d>Delete</a></td></tr>\n",
		n, n, (ch_temp[n].flags & CF_RESPAWN) ? "<b>" : "", ch_temp[n].name, (ch_temp[n].flags & CF_RESPAWN) ? "</b>" : "",
		ch_temp[n].x, ch_temp[n].y, ch_temp[n].points_tot, ch_temp[n].alignment, ch_temp[n].data[CHD_GROUP], 
		(ch_temp[n].flags & CF_EXTRAEXP) ? "+EXP" : "", (ch_temp[n].flags & CF_EXTRACRIT) ? "CRIT" : "", n, n);
}
void phtml_form_textboxs(char *title, char *bname, char *value, int size, int maxlen)
{ 
	printf("<tr><td>%s:</td><td><input type=text name=%s value=\"%s\" size=%d maxlength=%d></td></tr>\n", title, bname, value, size, maxlen);
}
void phtml_form_textbox(char *title, char *bname, int v, int size, int maxlen)
{ 
	printf("<tr><td>%s:</td><td><input type=text name=%s value=\"%d\" size=%d maxlength=%d></td></tr>\n", title, bname, v, size, maxlen);
}
void phtml_form_textboxd(char *title, char *bname, int v, int size, int maxlen)
{ 
	printf("<tr><td>%s:</td><td><input type=text name=%s value=\"%d\" size=%d maxlength=%d> (1s 2n 3w 4e)</td></tr>\n", title, bname, v, size, maxlen);
}
void phtml_form_textbin(char *title, char *bname, int v, int size, int maxlen)
{ 
	printf("<tr><td>%s:</td><td><input type=text name=%s value=\"%d\" size=%d maxlength=%d> (%s)</td></tr>\n", title, bname, v, size, maxlen, v ? it_temp[v].name : "none");
}
void phtml_form_checkbox(char *bname, unsigned int fl, unsigned int v, char *title)
{
	printf("<input type=checkbox name=%s value=%d %s>%s<br>\n", bname, v, (fl & v) ? "checked" : "", title);
}
void phtml_form_checklong(char *bname, unsigned long long fl, unsigned long long v, char *title)
{
	printf("<input type=checkbox name=%s value=%Lu %s>%s<br>\n", bname, v, (fl & v) ? "checked" : "", title);
}

void list_all_player_characters()
{
	int n;
	phtml_home();
	phtml_tab_open();
	for (n = 1; n<MAXCHARS; n++)
	{
		if (!(ch[n].flags & (CF_PLAYER))) continue;
		if (ch[n].used==USE_EMPTY) continue;
		phtml_row_player(n);
	}
	phtml_tab_close();
	phtml_home();
}

#define IS_EMPTY_OR_NOT_PLAYER(n) (IS_EMPTYCHAR(n) || !IS_PLAYER(n))

void list_all_player_characters_by_class()
{
	int n, m;
	char cname[20];
	phtml_home();
	
	for (m = 0; m < 13; m++)
	{
		switch (m)
		{
			case  0: strncpy(cname, "Templar",      19); break;
			case  1: strncpy(cname, "Mercenaries",  19); break;
			case  2: strncpy(cname, "Harakim",      19); break;
			case  3: strncpy(cname, "Seyan'du",     19); break;
			case  4: strncpy(cname, "Arch Templar", 19); break;
			case  5: strncpy(cname, "Skalds",       19); break;
			case  6: strncpy(cname, "Warriors",     19); break;
			case  7: strncpy(cname, "Sorcerers",    19); break;
			case  8: strncpy(cname, "Summoners",    19); break;
			case  9: strncpy(cname, "Arch Harakim", 19); break;
			case 10: strncpy(cname, "Bravers",      19); break;
			case 11: strncpy(cname, "Lycanthropes", 19); break;
			default: strncpy(cname, "REBIRTH",      19); break;
		}
		printf("%s:<br><table>\n", cname);
		for (n = 1; n<MAXCHARS; n++)
		{	
			if (IS_EMPTY_OR_NOT_PLAYER(n))     continue;
			if (m ==  0 && !IS_TEMPLAR(n))     continue;
			if (m ==  1 && !IS_MERCENARY(n))   continue;
			if (m ==  2 && !IS_HARAKIM(n))     continue;
			if (m ==  3 && !IS_SEYAN_DU(n))    continue;
			if (m ==  4 && !IS_ARCHTEMPLAR(n)) continue;
			if (m ==  5 && !IS_SKALD(n))       continue;
			if (m ==  6 && !IS_WARRIOR(n))     continue;
			if (m ==  7 && !IS_SORCERER(n))    continue;
			if (m ==  8 && !IS_SUMMONER(n))    continue;
			if (m ==  9 && !IS_ARCHHARAKIM(n)) continue;
			if (m == 10 && !IS_BRAVER(n))      continue;
			if (m == 11 && !IS_LYCANTH(n))     continue;
			if (m == 12 && !IS_RB(n))          continue;
			phtml_row_player(n);
		}
		phtml_tab_close_break();
	}
	phtml_home();
}

void list_all_player_characters_by_pandium()
{
	int n, m;
	char cname[20];
	
	for (m = 0; m < 6; m++)
	{
		switch (m)
		{
			case  0: strncpy(cname, "Cleared Depth 40", 19); break;
			case  1: strncpy(cname, "Cleared Depth 30", 19); break;
			case  2: strncpy(cname, "Cleared Depth 20", 19); break;
			case  3: strncpy(cname, "Cleared Depth 10", 19); break;
			case  4: strncpy(cname, "Cleared Depth 1",  19); break;
			default: strncpy(cname, "Uncleared",        19); break;
		}
		printf("%s:<br><table>\n", cname);
		for (n = 1; n<MAXCHARS; n++)
		{	
			if (IS_EMPTY_OR_NOT_PLAYER(n))            continue;
			if (m ==  0 && ch[n].pandium_floor[2]!=5) continue;
			if (m ==  1 && ch[n].pandium_floor[2]!=4) continue;
			if (m ==  2 && ch[n].pandium_floor[2]!=3) continue;
			if (m ==  3 && ch[n].pandium_floor[2]!=2) continue;
			if (m ==  4 && ch[n].pandium_floor[2]!=1) continue;
			if (m ==  5 && ch[n].pandium_floor[2]!=0) continue;
			phtml_row_player(n);
		}
		phtml_tab_close_break();
	}
	phtml_home();
}

#define P_ERR_NAN 0
#define P_ERR_OOB 1
#define P_ERR_MAXTC 2
#define P_ERR_MAXTI 3

void page_err(int v)
{
	switch (v)
	{
		case  0: printf("No number specified.\n");  return;
		case  1: printf("Number out of bounds.\n"); return;
		case  2: printf("MAXTCHARS reached!\n");    return;
		case  3: printf("MAXTITEM reached!\n");     return;
		default: break;
	}
}

void copy_character_template(LIST *head)
{
	int cn, n;
	char *tmp;

	tmp = find_val(head, "cn");
	if (tmp) cn = atoi(tmp);
	else return page_err(P_ERR_NAN);
	if (cn < 0 || cn >= MAXTCHARS) return page_err(P_ERR_OOB);
	for (n = 1; n < MAXTCHARS; n++) if (IS_EMPTY(ch_temp[n])) break;
	if (n==MAXTCHARS) return page_err(P_ERR_MAXTC);

	ch_temp[n] = ch_temp[cn];

	printf("Done. Here is your new, copied monster\n");
	phtml_tab_open(); // show copied monster for direct access.
	phtml_row_ctemplate(n);
	phtml_tab_close();
}

void view_character_template(LIST *head)
{
	int cn, n, m, t;
	char *tmp;
	
	tmp = find_val(head, "cn");
	if (tmp) cn = atoi(tmp);
	else return page_err(P_ERR_NAN);
	if (cn<0 || cn>=MAXTCHARS) return page_err(P_ERR_OOB);
	
	phtml_backhome(11);
	phtml_formpost();
	phtml_tab_open();
	phtml_form_textboxs("Name",        "name",        ch_temp[cn].name,        35,  35);
	phtml_form_textboxs("Reference",   "reference",   ch_temp[cn].reference,   35,  35);
	phtml_form_textboxs("Description", "description", ch_temp[cn].description, 35, 195);
	phtml_tab_title("Kindred");
	phtml_form_checkbox("kindred", ch_temp[cn].kindred, KIN_FEMALE,      "Female");
	phtml_form_checkbox("kindred", ch_temp[cn].kindred, KIN_TEMPLAR,     "Templar");
	phtml_form_checkbox("kindred", ch_temp[cn].kindred, KIN_MERCENARY,   "Mercenary");
	phtml_form_checkbox("kindred", ch_temp[cn].kindred, KIN_HARAKIM,     "Harakim");
	phtml_form_checkbox("kindred", ch_temp[cn].kindred, KIN_SEYAN_DU,    "Seyan'Du");
	phtml_form_checkbox("kindred", ch_temp[cn].kindred, KIN_ARCHTEMPLAR, "Arch-Templar");
	phtml_form_checkbox("kindred", ch_temp[cn].kindred, KIN_SKALD,       "Skald");
	phtml_form_checkbox("kindred", ch_temp[cn].kindred, KIN_WARRIOR,     "Warrior");
	phtml_form_checkbox("kindred", ch_temp[cn].kindred, KIN_SORCERER,    "Sorcerer");
	phtml_form_checkbox("kindred", ch_temp[cn].kindred, KIN_SUMMONER,    "Summoner");
	phtml_form_checkbox("kindred", ch_temp[cn].kindred, KIN_ARCHHARAKIM, "Arch-Harakim");
	phtml_form_checkbox("kindred", ch_temp[cn].kindred, KIN_BRAVER,      "Braver");
	phtml_form_checkbox("kindred", ch_temp[cn].kindred, KIN_LYCANTH,     "Lycanthrope");
	phtml_form_checkbox("kindred", ch_temp[cn].kindred, KIN_MONSTER,     "Monster");
	phtml_form_checkbox("kindred", ch_temp[cn].kindred, KIN_PURPLE,      "Purple");
	phtml_form_checkbox("kindred", ch_temp[cn].kindred, KIN_CLANKWAI,    "Clan Kwai");
	phtml_form_checkbox("kindred", ch_temp[cn].kindred, KIN_CLANGORN,    "Clan Gorn");
	phtml_form_checkbox("kindred", ch_temp[cn].kindred, KIN_SHADOW,      "Shadow");
	phtml_form_checkbox("kindred", ch_temp[cn].kindred, KIN_BLOODY,      "Bloody");
	printf("</td></tr>\n");
	phtml_form_textbox("Sprite base", "sprite", ch_temp[cn].sprite, 10, 10);
	phtml_form_textbox("Sound base",  "sound",  ch_temp[cn].sound,  10, 10);
	phtml_form_textbox("Class",       "class",  ch_temp[cn].class,  10, 10);
	phtml_tab_title("Flags");
	phtml_form_checklong("flags",  ch_temp[cn].flags,   CF_INFRARED,     "Infrared");
	phtml_form_checklong("flags",  ch_temp[cn].flags,   CF_UNDEAD,       "Undead");
	phtml_form_checklong("flags",  ch_temp[cn].flags,   CF_RESPAWN,      "Respawn");
	phtml_form_checklong("flags",  ch_temp[cn].flags,   CF_NOSLEEP,      "No-Sleep");
	phtml_form_checklong("flags",  ch_temp[cn].flags,   CF_MERCHANT,     "Merchant");
	phtml_form_checklong("flags",  ch_temp[cn].flags,   CF_BSPOINTS,     "BS Points");
	phtml_form_checklong("flags",  ch_temp[cn].flags,   CF_SIMPLE,       "Simple Animation");
	phtml_form_checklong("flags",  ch_temp[cn].flags,   CF_EXTRAEXP,     "Extra Kill EXP (2x exp)");
	phtml_form_checklong("flags",  ch_temp[cn].flags,   CF_EXTRACRIT,    "Extra Crit Chance (+2%% base, 1.5x exp)");
	printf("</td></tr>\n");
	phtml_form_textbox("Alignment", "alignment", ch_temp[cn].alignment, 10, 10);
	
	printf("<tr><td colspan=2><table>\n");
	printf("<tr><td>Name</td><td>Start Value</td><td>Race Max</td><td>Difficulty</td></tr>\n");
	for (n = 0; n<5; n++)
	{
		printf("<tr><td>%s</td>\n", at_name[n]);
		for (m=0;m<4;m++) { if (m==1) continue; printf("<td><input type=text name=attrib%d_%d value=\"%d\" size=10 maxlength=10></td>\n", n, m, ch_temp[cn].attrib[n][m]); }
		printf("</tr>\n");
	}
	printf("<tr><td>Hitpoints</td>\n");
	for (m=0;m<4;m++) { if (m==1) continue; printf("<td><input type=text name=hp_%d value=\"%d\" size=10 maxlength=10></td>\n", m, ch_temp[cn].hp[m]); }
	printf("</tr>\n");
	printf("<tr><td>Endurance</td>\n");
	for (m=0;m<4;m++) { if (m==1) continue; printf("<td><input type=text name=end_%d value=\"%d\" size=10 maxlength=10></td>\n", m, ch_temp[cn].end[m]); }
	printf("</tr>\n");
	printf("<tr><td>Mana</td>\n");
	for (m=0;m<4;m++) { if (m==1) continue; printf("<td><input type=text name=mana_%d value=\"%d\" size=10 maxlength=10></td>\n", m, ch_temp[cn].mana[m]); }
	printf("</tr>\n");
	for (n = 0; n<MAXSKILL; n++)
	{
		t = skillslots[n];
		if (skilltab[t].name[0]==0) continue;
		printf("<tr><td>%s</td>\n", skilltab[t].name);
		for (m=0;m<4;m++) { if (m==1) continue; printf("<td><input type=text name=skill%d_%d value=\"%d\" size=10 maxlength=10></td>\n", t, m, ch_temp[cn].skill[t][m]); }
		printf("</tr>\n");
	}
	printf("</table></td></tr>\n");
	
	phtml_form_textbox("Speed Mod",        "speed_mod",        ch_temp[cn].speed_mod,        10,  10);
	phtml_form_textbox("Weapon Bonus",        "weapon_bonus",        ch_temp[cn].weapon_bonus,        10,  10);
	phtml_form_textbox("Armor Bonus",        "armor_bonus",        ch_temp[cn].armor_bonus,        10,  10);
	phtml_form_textbox("Thorns Bonus",        "gethit_bonus",        ch_temp[cn].gethit_bonus,        10,  10);
	phtml_form_textbox("Light Bonus",        "light_bonus",        ch_temp[cn].light_bonus,        10,  10);
	phtml_form_textbox("EXP left",        "points",        ch_temp[cn].points,        10,  10);
	phtml_form_textbox("EXP total",        "points_tot",        ch_temp[cn].points_tot,        10,  10);
	
	phtml_form_textbox("Position X",        "x",        ch_temp[cn].x,        10,  10);
	phtml_form_textbox("Position Y",        "y",        ch_temp[cn].y,        10,  10);
	phtml_form_textboxd("Direction",        "dir",        ch_temp[cn].dir,        10,  10);
	phtml_form_textbox("Gold",        "gold",        ch_temp[cn].gold,        10,  10);
	
	if (!IS_SANEITEM(ch_temp[cn].citem)) ch_temp[cn].citem = 0;
	phtml_form_textbin("Current Item",        "citem",        ch_temp[cn].citem,        10,  10);
	
	for (n = 0; n<MAXGSLOTS; n++)
	{
		m = itemslots[n];
		if (!IS_SANEITEM(ch_temp[cn].worn[m])) ch_temp[cn].worn[m] = 0;
		printf("<tr><td>%s</td><td><input type=text name=worn%d value=\"%d\" size=10 maxlength=10> (%s)</td></tr>\n",
				weartext[m], m, ch_temp[cn].worn[m], ch_temp[cn].worn[m] ? it_temp[ch_temp[cn].worn[m]].name : "none");
	}
	for (n = 0; n<MAXITEMS; n++)
	{
		if (!IS_SANEITEM(ch_temp[cn].item[n])) ch_temp[cn].item[n] = 0;
		printf("<tr><td>Item %d</td><td><input type=text name=item%d value=\"%d\" size=10 maxlength=10> (%s)</td></tr>\n",
				n, n, ch_temp[cn].item[n], ch_temp[cn].item[n] ? it_temp[ch_temp[cn].item[n]].name : "none");
	}

	printf("<tr><td>Driver Data:</td></tr>\n");
	for (n = 0; n<100; n++)
	{
		if (strcmp(data_name[n], "Unused")==0 || strcmp(data_name[n], "Reserved")==0) continue;
		printf("<tr><td>[%3d] %s</td><td><input type=text name=drdata%d value=\"%d\" size=35 maxlength=75> %s%s%s</td></tr>\n",
				n, data_name[n], n, ch_temp[cn].data[n], 
				(n<10&&ch_temp[cn].data[n]) ? "(" : "",
				(n<10&&ch_temp[cn].data[n]) ? it_temp[ch_temp[cn].data[n]].name : "",
				(n<10&&ch_temp[cn].data[n]) ? ")" : "");
	}

	printf("<tr><td>Driver Texts:</td></tr>\n");
	for (n = 0; n<10; n++)
	{
		if (strcmp(text_name[n], "Unused")==0 || strcmp(text_name[n], "Reserved")==0) continue;
		printf("<tr><td>%s</td><td><input type=text name=text_%d value=\"%s\" size=35 maxlength=158></td></tr>\n",
				text_name[n], n, ch_temp[cn].text[n]);
	}

	printf("<tr><td><input type=submit value=Update></td><td> </td></tr>\n");
	phtml_tab_close();
	printf("<input type=hidden name=step value=14>\n");
	printf("<input type=hidden name=cn value=%d>\n", cn);
	printf("</form>\n");
	phtml_backhome(11);
}

void view_character_player(LIST *head)
{
	int cn, n;
	char *tmp;
	int m;

	tmp = find_val(head, "cn");
	if (tmp) cn = atoi(tmp);
	else return page_err(P_ERR_NAN);
	if (cn<0 || cn>=MAXCHARS) return page_err(P_ERR_OOB);
	
	phtml_backhome(19);
	printf("<a href=/cgi-imp/acct.cgi?step=29&cn=%d>Save MOA</a><br>", cn);
	printf("<form method=post action=/cgi-imp/acct.cgi>\n");
	phtml_tab_open();
	printf("<tr><td>Name:</td><td><input type=text name=name value=\"%s\" size=35 maxlength=35></td></tr>\n",
			ch[cn].name);
	printf("<tr><td>Reference:</td><td><input type=text name=reference value=\"%s\" size=35 maxlength=35></td></tr>\n",
			ch[cn].reference);
	printf("<tr><td>Description:</td><td><input type=text name=description value=\"%s\" size=35 maxlength=195></td></tr>\n",
			ch[cn].description);
	printf("<tr><td>Player:</td><td><input type=text name=player value=\"%d\" size=10 maxlength=10></td></tr>\n",
			ch[cn].player);
	printf("<tr><td>Pass1:</td><td><input type=text name=pass1 value=\"%d\" size=10 maxlength=10></td></tr>\n",
			ch[cn].pass1);
	printf("<tr><td>Pass2:</td><td><input type=text name=pass2 value=\"%d\" size=10 maxlength=10></td></tr>\n",
			ch[cn].pass2);

	printf("<tr><td valign=top>Kindred:</td><td>\n");
	printf("<input type=checkbox name=kindred value=%d %s>Mercenary<br>\n",
			KIN_MERCENARY, IS_MERCENARY(cn) ? "checked" : "");
	printf("<input type=checkbox name=kindred value=%d %s>Sorcerer-Mercenary<br>\n\n",
			KIN_SORCERER, IS_SORCERER(cn) ? "checked" : "");
	printf("<input type=checkbox name=kindred value=%d %s>Warrior-Mercenary<br>\n",
			KIN_WARRIOR, IS_WARRIOR(cn) ? "checked" : "");
	printf("<input type=checkbox name=kindred value=%d %s>Templar<br>\n",
			KIN_TEMPLAR, IS_TEMPLAR(cn) ? "checked" : "");
	printf("<input type=checkbox name=kindred value=%d %s>Arch-Templar<br>\n",
			KIN_ARCHTEMPLAR, IS_ARCHTEMPLAR(cn) ? "checked" : "");
	printf("<input type=checkbox name=kindred value=%d %s>Harakim<br>\n",
			KIN_HARAKIM, IS_HARAKIM(cn) ? "checked" : "");
	printf("<input type=checkbox name=kindred value=%d %s>Arch-Harakim<br>\n",
			KIN_ARCHHARAKIM, IS_ARCHHARAKIM(cn) ? "checked" : "");
	printf("<input type=checkbox name=kindred value=%d %s>Seyan'Du<br>\n",
			KIN_SEYAN_DU, IS_SEYAN_DU(cn) ? "checked" : "");
	printf("<input type=checkbox name=kindred value=%d %s>Braver<br>\n",
			KIN_BRAVER, IS_BRAVER(cn) ? "checked" : "");
	printf("<input type=checkbox name=kindred value=%d %s>Lycanthrope<br>\n",
			KIN_LYCANTH, IS_LYCANTH(cn) ? "checked" : "");
	printf("<input type=checkbox name=kindred value=%d %s>Purple<br>\n",
			KIN_PURPLE, IS_PURPLE(cn) ? "checked" : "");
	printf("<input type=checkbox name=kindred value=%d %s>Monster<br>\n",
			KIN_MONSTER, IS_MONSTER(cn) ? "checked" : "");
	printf("<input type=checkbox name=kindred value=%d %s>Female<br>\n",
			KIN_FEMALE, IS_FEMALE(cn) ? "checked" : "");
	printf("<input type=checkbox name=kindred value=%d %s>Clan Kwai<br>\n",
			KIN_CLANKWAI, IS_CLANKWAI(cn) ? "checked" : "");
	printf("<input type=checkbox name=kindred value=%d %s>Clan Gorn<br>\n",
			KIN_CLANGORN, IS_CLANGORN(cn) ? "checked" : "");
	printf("</td></tr>\n");

	printf("<tr><td valign=top>Sprite base:</td><td><input type=text name=sprite value=\"%d\" size=10 maxlength=10></td></tr>\n",
			ch[cn].sprite);

	printf("<tr><td valign=top>Sound base:</td><td><input type=text name=sound value=\"%d\" size=10 maxlength=10></td></tr>\n",
			ch[cn].sound);

	printf("<tr><td valign=top>Class:</td><td><input type=text name=class value=\"%d\" size=10 maxlength=10></td></tr>\n",
			ch[cn].class);

	printf("<tr><td valign=top>Flags:</td><td>\n");
	printf("<input type=checkbox name=flags value=%Lu %s>Player<br>\n", CF_PLAYER, (ch[cn].flags & CF_PLAYER) ? "checked" : "");
	printf("<input type=checkbox name=flags value=%Lu %s>Can Lockpick<br>\n", CF_LOCKPICK, (ch[cn].flags & CF_LOCKPICK) ? "checked" : "");
	printf("<input type=checkbox name=flags value=%Lu %s>Staff<br>\n", CF_STAFF, (ch[cn].flags & CF_STAFF) ? "checked" : "");
	printf("<input type=checkbox name=flags value=%Lu %s>Imp<br>\n", CF_IMP, (ch[cn].flags & CF_IMP) ? "checked" : "");
	printf("<input type=checkbox name=flags value=%Lu %s>Creator<br>\n", CF_CREATOR, (ch[cn].flags & CF_CREATOR) ? "checked" : "");
	printf("<input type=checkbox name=flags value=%Lu %s>Build Mode<br>\n", CF_BUILDMODE, (ch[cn].flags & CF_BUILDMODE) ? "checked" : "");
	printf("<input type=checkbox name=flags value=%Lu %s>God<br>\n", CF_GOD, (ch[cn].flags & CF_GOD) ? "checked" : "");
	printf("<input type=checkbox name=flags value=%Lu %s>Greater God<br>\n", CF_GREATERGOD, (ch[cn].flags & CF_GREATERGOD) ? "checked" : "");
	printf("<input type=checkbox name=flags value=%Lu %s>Infrared<br>\n", CF_INFRARED, (ch[cn].flags & CF_INFRARED) ? "checked" : "");
	printf("<input type=checkbox name=flags value=%Lu %s>Immortal<br>\n", CF_IMMORTAL, (ch[cn].flags & CF_IMMORTAL) ? "checked" : "");
	printf("<input type=checkbox name=flags value=%Lu %s>Invisible<br>\n", CF_INVISIBLE, (ch[cn].flags & CF_INVISIBLE) ? "checked" : "");
	printf("<input type=checkbox name=flags value=%Lu %s>Greater Invisibility<br>\n", CF_GREATERINV, (ch[cn].flags & CF_GREATERINV) ? "checked" : "");
	printf("<input type=checkbox name=flags value=%Lu %s>Safety Measures for Gods<br>\n", CF_SAFE, (ch[cn].flags & CF_SAFE) ? "checked" : "");
	printf("<input type=checkbox name=flags value=%Lu %s>New User<br>\n", CF_NEWUSER, (ch[cn].flags & CF_NEWUSER) ? "checked" : "");
	printf("<input type=checkbox name=flags value=%Lu %s>No Tell<br>\n", CF_NOTELL, (ch[cn].flags & CF_NOTELL) ? "checked" : "");
	printf("<input type=checkbox name=flags value=%Lu %s>No Shout<br>\n", CF_NOSHOUT, (ch[cn].flags & CF_NOSHOUT) ? "checked" : "");
	printf("<input type=checkbox name=flags value=%Lu %s>Silence<br>\n", CF_SILENCE, (ch[cn].flags & CF_SILENCE) ? "checked" : "");
	printf("<input type=checkbox name=flags value=%Lu %s>GC-to-Me<br>\n", CF_GCTOME, (ch[cn].flags & CF_GCTOME) ? "checked" : "");
	printf("<input type=checkbox name=flags value=%Lu %s>No HP Regen<br>\n", CF_NOHPREG, (ch[cn].flags & CF_NOHPREG) ? "checked" : "");
	printf("<input type=checkbox name=flags value=%Lu %s>No END Regen<br>\n", CF_NOENDREG, (ch[cn].flags & CF_NOENDREG) ? "checked" : "");
	printf("<input type=checkbox name=flags value=%Lu %s>No MANA Regen<br>\n", CF_NOMANAREG, (ch[cn].flags & CF_NOMANAREG) ? "checked" : "");
	printf("<input type=checkbox name=flags value=%Lu %s>Dead Body<br>\n", CF_BODY, (ch[cn].flags & CF_BODY) ? "checked" : "");
	printf("<input type=checkbox name=flags value=%Lu %s>Stay Awake<br>\n", CF_NOSLEEP, (ch[cn].flags & CF_NOSLEEP) ? "checked" : "");
	printf("<input type=checkbox name=flags value=%Lu %s>No Magic<br>\n", CF_NOMAGIC, (ch[cn].flags & CF_NOMAGIC) ? "checked" : "");
	printf("<input type=checkbox name=flags value=%Lu %s>Stoned<br>\n", CF_STONED, (ch[cn].flags & CF_STONED) ? "checked" : "");
	printf("<input type=checkbox name=flags value=%Lu %s>Usurped<br>\n", CF_USURP, (ch[cn].flags & CF_USURP) ? "checked" : "");
	printf("<input type=checkbox name=flags value=%Lu %s>Shutup<br>\n", CF_SHUTUP, (ch[cn].flags & CF_SHUTUP) ? "checked" : "");
	printf("<input type=checkbox name=flags value=%Lu %s>No Description<br>\n", CF_NODESC, (ch[cn].flags & CF_NODESC) ? "checked" : "");
	printf("<input type=checkbox name=flags value=%Lu %s>Profiler<br>\n", CF_PROF, (ch[cn].flags & CF_PROF) ? "checked" : "");
	printf("<input type=checkbox name=flags value=%Lu %s>Simple Animation<br>\n", CF_SIMPLE, (ch[cn].flags & CF_SIMPLE) ? "checked" : "");
	printf("<input type=checkbox name=flags value=%Lu %s>Kicked<br>\n", CF_KICKED, (ch[cn].flags & CF_KICKED) ? "checked" : "");
	printf("<input type=checkbox name=flags value=%Lu %s>Do Not List<br>\n", CF_NOLIST, (ch[cn].flags & CF_NOLIST) ? "checked" : "");
	printf("<input type=checkbox name=flags value=%Lu %s>Do Not List on Who<br>\n", CF_NOWHO, (ch[cn].flags & CF_NOWHO) ? "checked" : "");
	printf("<input type=checkbox name=flags value=%Lu %s>Spell Ignore<br>\n", CF_SPELLIGNORE, (ch[cn].flags & CF_SPELLIGNORE) ? "checked" : "");
	printf("<input type=checkbox name=flags value=%Lu %s>No Staff Tells<br>\n", CF_NOSTAFF, (ch[cn].flags & CF_NOSTAFF) ? "checked" : "");
	printf("<input type=checkbox name=flags value=%Lu %s>Is POH<br>\n", KIN_POH, (ch[cn].kindred & KIN_POH) ? "checked" : "");
	printf("<input type=checkbox name=flags value=%Lu %s>Is POH Leader<br>\n", KIN_POH_LEADER, (ch[cn].kindred & KIN_POH_LEADER) ? "checked" : "");
	printf("<input type=checkbox name=flags value=%Lu %s>Is Looting<br>\n", CF_ISLOOTING, (ch[cn].flags & CF_ISLOOTING) ? "checked" : "");
	printf("<input type=checkbox name=flags value=%Lu %s>Gold List<br>\n", CF_GOLDEN, (ch[cn].flags & CF_GOLDEN) ? "checked" : "");
	printf("<input type=checkbox name=flags value=%Lu %s>Black List<br>\n", CF_BLACK, (ch[cn].flags & CF_BLACK) ? "checked" : "");
	printf("<input type=checkbox name=flags value=%Lu %s>Has Password<br>\n", CF_PASSWD, (ch[cn].flags & CF_PASSWD) ? "checked" : "");
	printf("<input type=checkbox name=flags value=%Lu %s>Client Side Update Needed<br>\n", CF_UPDATE, (ch[cn].flags & CF_UPDATE) ? "checked" : "");
	printf("<input type=checkbox name=flags value=%Lu %s>Save Player To Disk<br>\n", CF_SAVEME, (ch[cn].flags & CF_SAVEME) ? "checked" : "");
	printf("</td></tr>\n");

	printf("<tr><td valign=top>Alignment</td><td><input type=text name=alignment value=\"%d\" size=10 maxlength=10></td></tr>\n",
			ch[cn].alignment);

	printf("<tr><td colspan=2><table>\n");

	printf("<tr><td>Name</td><td>Start Value</td><td>Race Mod</td><td>Race Max</td><td>Difficulty</td></tr>\n");

	for (n = 0; n<5; n++)
	{
		printf("<tr><td>%s</td>\n", at_name[n]);
		printf("<td><input type=text name=attrib%d_0 value=\"%d\" size=10 maxlength=10></td>\n", n, B_AT(cn, n));
		printf("<td><input type=text name=attrib%d_1 value=\"%d\" size=10 maxlength=10></td>\n", n, ch[cn].attrib[n][1]);
		printf("<td><input type=text name=attrib%d_2 value=\"%d\" size=10 maxlength=10></td>\n", n, ch[cn].attrib[n][2]);
		printf("<td><input type=text name=attrib%d_3 value=\"%d\" size=10 maxlength=10></td>\n", n, ch[cn].attrib[n][3]);
		printf("</tr>\n");
	}

	printf("<tr><td>Hitpoints</td>\n");
	printf("<td><input type=text name=hp_0 value=\"%d\" size=10 maxlength=10></td>\n", ch[cn].hp[0]);
	printf("<td><input type=text name=hp_1 value=\"%d\" size=10 maxlength=10></td>\n", ch[cn].hp[1]);
	printf("<td><input type=text name=hp_2 value=\"%d\" size=10 maxlength=10></td>\n", ch[cn].hp[2]);
	printf("<td><input type=text name=hp_3 value=\"%d\" size=10 maxlength=10></td>\n", ch[cn].hp[3]);
	printf("</tr>\n");

	printf("<tr><td>Endurance</td>\n");
	printf("<td><input type=text name=end_0 value=\"%d\" size=10 maxlength=10></td>\n", ch[cn].end[0]);
	printf("<td><input type=text name=end_1 value=\"%d\" size=10 maxlength=10></td>\n", ch[cn].end[1]);
	printf("<td><input type=text name=end_2 value=\"%d\" size=10 maxlength=10></td>\n", ch[cn].end[2]);
	printf("<td><input type=text name=end_3 value=\"%d\" size=10 maxlength=10></td>\n", ch[cn].end[3]);
	printf("</tr>\n");

	printf("<tr><td>Mana</td>\n");
	printf("<td><input type=text name=mana_0 value=\"%d\" size=10 maxlength=10></td>\n", ch[cn].mana[0]);
	printf("<td><input type=text name=mana_1 value=\"%d\" size=10 maxlength=10></td>\n", ch[cn].mana[1]);
	printf("<td><input type=text name=mana_2 value=\"%d\" size=10 maxlength=10></td>\n", ch[cn].mana[2]);
	printf("<td><input type=text name=mana_3 value=\"%d\" size=10 maxlength=10></td>\n", ch[cn].mana[3]);
	printf("</tr>\n");

	for (n = 0; n<MAXSKILL; n++)
	{
		m = skillslots[n];
		if (skilltab[m].name[0]==0)	continue;
		printf("<tr><td>%s</td>\n", skilltab[m].name);
		printf("<td><input type=text name=skill%d_0 value=\"%d\" size=10 maxlength=10></td>\n", m, B_SK(cn, m));
		printf("<td><input type=text name=skill%d_1 value=\"%d\" size=10 maxlength=10></td>\n", m, ch[cn].skill[m][1]);
		printf("<td><input type=text name=skill%d_2 value=\"%d\" size=10 maxlength=10></td>\n", m, ch[cn].skill[m][2]);
		printf("<td><input type=text name=skill%d_3 value=\"%d\" size=10 maxlength=10></td>\n", m, ch[cn].skill[m][3]);
		printf("</tr>\n");
	}

	printf("</table></td></tr>\n");

	printf("<tr><td valign=top>Speed Mod</td><td><input type=text name=speed_mod value=\"%d\" size=10 maxlength=10></td></tr>\n",
			ch[cn].speed_mod);
	printf("<tr><td valign=top>Weapon Bonus</td><td><input type=text name=weapon_bonus value=\"%d\" size=10 maxlength=10></td></tr>\n",
			ch[cn].weapon_bonus);
	printf("<tr><td valign=top>Armor Bonus</td><td><input type=text name=armor_bonus value=\"%d\" size=10 maxlength=10></td></tr>\n",
			ch[cn].armor_bonus);
	printf("<tr><td valign=top>Gethit Bonus</td><td><input type=text name=gethit_bonus value=\"%d\" size=10 maxlength=10></td></tr>\n",
			ch[cn].gethit_bonus);
	printf("<tr><td valign=top>Light Bonus</td><td><input type=text name=light_bonus value=\"%d\" size=10 maxlength=10></td></tr>\n",
			ch[cn].light_bonus);

	printf("<tr><td>EXP left:</td><td><input type=text name=points value=\"%d\" size=10 maxlength=10></td></tr>\n",
			ch[cn].points);
	printf("<tr><td>EXP total:</td><td><input type=text name=points_tot value=\"%d\" size=10 maxlength=10></td></tr>\n",
			ch[cn].points_tot);

	printf("<tr><td>Position X:</td><td><input type=text name=x value=\"%d\" size=10 maxlength=10></td></tr>\n",
			ch[cn].x);
	printf("<tr><td>Position Y:</td><td><input type=text name=y value=\"%d\" size=10 maxlength=10></td></tr>\n",
			ch[cn].y);
	printf("<tr><td>Direction:</td><td><input type=text name=dir value=\"%d\" size=10 maxlength=10> 1s 2n 3w 4e</td></tr>\n",
			ch[cn].dir);
	printf("<tr><td>Temple X:</td><td><input type=text name=temple_x value=\"%d\" size=10 maxlength=10></td></tr>\n",
			ch[cn].temple_x);
	printf("<tr><td>Temple Y:</td><td><input type=text name=temple_y value=\"%d\" size=10 maxlength=10></td></tr>\n",
			ch[cn].temple_y);
	printf("<tr><td>Tavern X:</td><td><input type=text name=tavern_x value=\"%d\" size=10 maxlength=10></td></tr>\n",
			ch[cn].tavern_x);
	printf("<tr><td>Tavern Y:</td><td><input type=text name=tavern_y value=\"%d\" size=10 maxlength=10></td></tr>\n",
			ch[cn].tavern_y);
	printf("<tr><td>Temp:</td><td><input type=text name=temp value=\"%d\" size=10 maxlength=10></td></tr>\n",
			ch[cn].temp);

	printf("<tr><td>To X:</td><td><input type=text name=tox value=\"%d\" size=10 maxlength=10></td></tr>\n",
			ch[cn].tox);
	printf("<tr><td>To Y:</td><td><input type=text name=toy value=\"%d\" size=10 maxlength=10></td></tr>\n",
			ch[cn].toy);
	printf("<tr><td>From X:</td><td><input type=text name=frx value=\"%d\" size=10 maxlength=10></td></tr>\n",
			ch[cn].frx);
	printf("<tr><td>From Y:</td><td><input type=text name=fry value=\"%d\" size=10 maxlength=10></td></tr>\n",
			ch[cn].fry);
	printf("<tr><td>Status:</td><td><input type=text name=status value=\"%d\" size=10 maxlength=10></td></tr>\n",
			ch[cn].status);
	printf("<tr><td>Status2:</td><td><input type=text name=status2 value=\"%d\" size=10 maxlength=10></td></tr>\n",
			ch[cn].status2);


	printf("<tr><td>Gold:</td><td><input type=text name=gold value=\"%d\" size=10 maxlength=10></td></tr>\n",
			ch[cn].gold);

	if (ch[cn].citem<0 || ch[cn].citem>=MAXITEM) ch[cn].citem = 0;

	printf("<tr><td>Current Item</td><td><input type=text name=citem value=\"%d\" size=10 maxlength=10> (%s)</td></tr>\n",
			ch[cn].citem, ch[cn].citem ? it[ch[cn].citem].name : "none");

	for (n = 0; n<13; n++)
	{
		m = itemslots[n];
		if (ch[cn].worn[m]>=MAXITEM) ch[cn].worn[m] = 0;
		printf("<tr><td>%s</td><td><input type=text name=worn%d value=\"%d\" size=10 maxlength=10> (%s)</td></tr>\n",
				weartext[m], m, ch[cn].worn[m], ch[cn].worn[m] ? it[ch[cn].worn[m]].name : "none");
	}

	for (n = 0; n<MAXITEMS; n++)
	{
		if (ch[cn].item[n]<0 || ch[cn].item[n]>=MAXITEM)
		{
			ch[cn].item[n] = 0;
			ch[cn].item_lock[n] = 0;
		}
		printf("<tr><td>Item %d</td><td><input type=text name=item%d value=\"%d\" size=10 maxlength=10> (%s)</td></tr>\n",
				n, n, ch[cn].item[n], ch[cn].item[n] ? it[ch[cn].item[n]].name : "none");
	}

	printf("<tr><td>Driver Data:</td></tr>\n");
	for (n = 0; n<100; n++)
	{
		if (strcmp(player_data_name[n], "Unused")==0 || strcmp(player_data_name[n], "Reserved")==0)
		{
			continue;
		}
		printf("<tr><td>[%3d] %s</td><td><input type=text name=drdata%d value=\"%d\" size=35 maxlength=75></td></tr>\n",
				n, player_data_name[n], n, ch[cn].data[n]);
	}

	printf("<tr><td>Driver Texts:</td></tr>\n");
	for (n = 0; n<10; n++)
	{
		if (strcmp(player_text_name[n], "Unused")==0 || strcmp(player_text_name[n], "Reserved")==0)
		{
			continue;
		}
		printf("<tr><td>%s</td><td><input type=text name=text_%d value=\"%s\" size=35 maxlength=158></td></tr>\n",
				player_text_name[n], n, ch[cn].text[n]);
	}

	printf("<tr><td><input type=submit value=Update></td><td> </td></tr>\n");
	phtml_tab_close();
	printf("<input type=hidden name=step value=26>\n");
	printf("<input type=hidden name=cn value=%d>\n", cn);
	printf("</form>\n");
	phtml_backhome(19);
}

void save_character_player(LIST *head)
{
	int cn, n;
	char *tmp;

	tmp = find_val(head, "cn");
	if (tmp) cn = atoi(tmp);
	else return page_err(P_ERR_NAN);
	if (cn<0 || cn>=MAXCHARS) return page_err(P_ERR_OOB);

	int handle;
	char buf[80];
	int result = chdir("/home/merc");

	if(result != 0)
	{
		printf("Unable to find /home/merc");
		exit(1);
	}
	sprintf(buf, ".save/%s.moa", ch[cn].name);

	handle = open(buf, O_WRONLY | O_TRUNC | O_CREAT, 0600);
	if (handle==-1)
	{
		printf("Could not open file.\n");
		perror(buf);
		return;
	}
	write(handle, &cn, 4);
	write(handle, &ch[cn].pass1, 4);
	write(handle, &ch[cn].pass2, 4);
	write(handle, ch[cn].name, 40);
	write(handle, &ch[cn].temp, 4);
	close(handle);
	printf("Saved as %s.moa.\n", ch[cn].name);
	return;
}

void copy_object(LIST *head)
{
	int in, n;
	char *tmp;

	tmp = find_val(head, "in");
	if (tmp) in = atoi(tmp);
	else return page_err(P_ERR_NAN);
	if (in<1 || in>=MAXTCHARS) return page_err(P_ERR_OOB);
	for (n = 1; n<MAXTITEM; n++) if (IS_EMPTY(it_temp[n])) break;
	if (n==MAXTITEM) return page_err(P_ERR_MAXTI);
	it_temp[n] = it_temp[in];
	printf("Done.\n");
}

void view_object(LIST *head)
{
	int in, n;
	char *tmp;
	int m;

	tmp = find_val(head, "in");
	if (tmp) in = atoi(tmp);
	else return page_err(P_ERR_NAN);
	if (in<1 || in>=MAXTITEM) return page_err(P_ERR_OOB);
	
	phtml_backhome(21);
	printf("<form method=post action=/cgi-imp/acct.cgi>\n");
	phtml_tab_open();
	printf("<tr><td valign=top>Name:</td><td><input type=text name=name value=\"%s\" size=35 maxlength=35></td></tr>\n",
			it_temp[in].name);
	printf("<tr><td>Reference:</td><td><input type=text name=reference value=\"%s\" size=35 maxlength=35></td></tr>\n",
			it_temp[in].reference);
	printf("<tr><td>Description:</td><td><input type=text name=description value=\"%s\" size=35 maxlength=195></td></tr>\n",
			it_temp[in].description);

	printf("<tr><td valign=top>Flags:</td><td>\n");
	printf("<input type=checkbox name=flags value=%Lu %s>Moveblock<br>\n",
			IF_MOVEBLOCK, (it_temp[in].flags & IF_MOVEBLOCK) ? "checked" : "");
	printf("<input type=checkbox name=flags value=%Lu %s>Sightblock<br>\n",
			IF_SIGHTBLOCK, (it_temp[in].flags & IF_SIGHTBLOCK) ? "checked" : "");
	printf("<input type=checkbox name=flags value=%Lu %s>Take-Able<br>\n",
			IF_TAKE, (it_temp[in].flags & IF_TAKE) ? "checked" : "");
	printf("<input type=checkbox name=flags value=%Lu %s>Look-Able<br>\n",
			IF_LOOK, (it_temp[in].flags & IF_LOOK) ? "checked" : "");
	printf("<input type=checkbox name=flags value=%Lu %s>Look-Special<br>\n",
			IF_LOOKSPECIAL, (it_temp[in].flags & IF_LOOKSPECIAL) ? "checked" : "");
	printf("<input type=checkbox name=flags value=%Lu %s>Use<br>\n",
			IF_USE, (it_temp[in].flags & IF_USE) ? "checked" : "");
	printf("<input type=checkbox name=flags value=%Lu %s>Use-Special<br>\n",
			IF_USESPECIAL, (it_temp[in].flags & IF_USESPECIAL) ? "checked" : "");
	printf("<input type=checkbox name=flags value=%Lu %s>Use-Destroy<br>\n",
			IF_USEDESTROY, (it_temp[in].flags & IF_USEDESTROY) ? "checked" : "");
	printf("<input type=checkbox name=flags value=%Lu %s>Use-Activate<br>\n",
			IF_USEACTIVATE, (it_temp[in].flags & IF_USEACTIVATE) ? "checked" : "");
	printf("<input type=checkbox name=flags value=%Lu %s>Use-Deactivate<br>\n",
			IF_USEDEACTIVATE, (it_temp[in].flags & IF_USEDEACTIVATE) ? "checked" : "");
	printf("<input type=checkbox name=flags value=%Lu %s>Re-Activate<br>\n",
			IF_REACTIVATE, (it_temp[in].flags & IF_REACTIVATE) ? "checked" : "");
	printf("<input type=checkbox name=flags value=%Lu %s>No-Repair<br>\n",
			IF_NOREPAIR, (it_temp[in].flags & IF_NOREPAIR) ? "checked" : "");
	printf("<input type=checkbox name=flags value=%Lu %s>Stackable<br>\n",
			IF_STACKABLE, (it_temp[in].flags & IF_STACKABLE) ? "checked" : "");

	printf("<input type=checkbox name=flags value=%Lu %s>Hidden (data[9])<br>\n",
			IF_HIDDEN, (it_temp[in].flags & IF_HIDDEN) ? "checked" : "");
	printf("<input type=checkbox name=flags value=%Lu %s>Step-Action<br>\n",
			IF_STEPACTION, (it_temp[in].flags & IF_STEPACTION) ? "checked" : "");
	printf("<input type=checkbox name=flags value=%Lu %s>Expire-Proc<br>\n",
			IF_EXPIREPROC, (it_temp[in].flags & IF_EXPIREPROC) ? "checked" : "");

	printf("<input type=checkbox name=flags value=%Lu %s>Fast Light Age<br>\n",
			IF_LIGHTAGE, (it_temp[in].flags & IF_LIGHTAGE) ? "checked" : "");

	printf("<input type=checkbox name=flags value=%Lu %s>Unique<br>\n",
			IF_UNIQUE, (it_temp[in].flags & IF_UNIQUE) ? "checked" : "");

	printf("<input type=checkbox name=flags value=%Lu %s>Shop-Destroy (quest items)<br>\n",
			IF_SHOPDESTROY, (it_temp[in].flags & IF_SHOPDESTROY) ? "checked" : "");
	printf("<input type=checkbox name=flags value=%Lu %s>Laby-Destroy (quest items)<br>\n",
			IF_LABYDESTROY, (it_temp[in].flags & IF_LABYDESTROY) ? "checked" : "");
	printf("<input type=checkbox name=flags value=%Lu %s>No-Depot (quest items)<br>\n",
			IF_NODEPOT, (it_temp[in].flags & IF_NODEPOT) ? "checked" : "");
	printf("<input type=checkbox name=flags value=%Lu %s>No Market (no price change)<br>\n",
			IF_NOMARKET, (it_temp[in].flags & IF_NOMARKET) ? "checked" : "");
	printf("<input type=checkbox name=flags value=%Lu %s>Donate (cheap items)<br>\n",
			IF_DONATE, (it_temp[in].flags & IF_DONATE) ? "checked" : "");
	printf("<input type=checkbox name=flags value=%Lu %s>Single-Age<br>\n",
			IF_SINGLEAGE, (it_temp[in].flags & IF_SINGLEAGE) ? "checked" : "");
	printf("<input type=checkbox name=flags value=%Lu %s>Always expire when inactive<br>\n",
			IF_ALWAYSEXP1, (it_temp[in].flags & IF_ALWAYSEXP1) ? "checked" : "");
	printf("<input type=checkbox name=flags value=%Lu %s>Always expire when active<br>\n",
			IF_ALWAYSEXP2, (it_temp[in].flags & IF_ALWAYSEXP2) ? "checked" : "");
	printf("<input type=checkbox name=flags value=%Lu %s>Armor<br>\n",
			IF_ARMOR, (it_temp[in].flags & IF_ARMOR) ? "checked" : "");
	printf("<input type=checkbox name=flags value=%Lu %s>Jewelery<br>\n",
			IF_JEWELERY, (it_temp[in].flags & IF_JEWELERY) ? "checked" : "");
	printf("<input type=checkbox name=flags value=%Lu %s>Book<br>\n",
			IF_BOOK, (it_temp[in].flags & IF_BOOK) ? "checked" : "");
	printf("<input type=checkbox name=flags value=%Lu %s>Magic<br>\n",
			IF_MAGIC, (it_temp[in].flags & IF_MAGIC) ? "checked" : "");
	printf("<input type=checkbox name=flags value=%Lu %s>Gemstone<br>\n",
			IF_GEMSTONE, (it_temp[in].flags & IF_GEMSTONE) ? "checked" : "");
	printf("<input type=checkbox name=flags value=%Lu %s>Is a Key<br>\n",
			IF_IS_KEY, (it_temp[in].flags & IF_IS_KEY) ? "checked" : "");
	printf("<input type=checkbox name=flags value=%Lu %s>Misc<br>\n",
			IF_MISC, (it_temp[in].flags & IF_MISC) ? "checked" : "");
	printf("<br>\n");
	printf("<input type=checkbox name=flags value=%Lu %s>Can be Soulstoned<br>\n",
			IF_CAN_SS, (it_temp[in].flags & IF_CAN_SS) ? "checked" : "");
	printf("<input type=checkbox name=flags value=%Lu %s>Can be Enchanted<br>\n",
			IF_CAN_EN, (it_temp[in].flags & IF_CAN_EN) ? "checked" : "");
	printf("<br>\n");
	printf("<input type=checkbox name=flags value=%Lu %s>Weapon: Claw<br>\n",
			IF_WP_CLAW, (it_temp[in].flags & IF_WP_CLAW) ? "checked" : "");
	printf("<input type=checkbox name=flags value=%Lu %s>Weapon: Dagger<br>\n",
			IF_WP_DAGGER, (it_temp[in].flags & IF_WP_DAGGER) ? "checked" : "");
	printf("<input type=checkbox name=flags value=%Lu %s>Weapon: Staff<br>\n",
			IF_WP_STAFF, (it_temp[in].flags & IF_WP_STAFF) ? "checked" : "");
	printf("<input type=checkbox name=flags value=%Lu %s>Weapon: Sword<br>\n",
			IF_WP_SWORD, (it_temp[in].flags & IF_WP_SWORD) ? "checked" : "");
	printf("<input type=checkbox name=flags value=%Lu %s>Weapon: Axe<br>\n",
			IF_WP_AXE, (it_temp[in].flags & IF_WP_AXE) ? "checked" : "");
	printf("<input type=checkbox name=flags value=%Lu %s>Weapon: Two-Handed<br>\n",
			IF_WP_TWOHAND, (it_temp[in].flags & IF_WP_TWOHAND) ? "checked" : "");
	printf("<input type=checkbox name=flags value=%Lu %s>Offhand: Dual Sword<br>\n",
			IF_OF_DUALSW, (it_temp[in].flags & IF_OF_DUALSW) ? "checked" : "");
	printf("<input type=checkbox name=flags value=%Lu %s>Offhand: Shield<br>\n",
			IF_OF_SHIELD, (it_temp[in].flags & IF_OF_SHIELD) ? "checked" : "");
	printf("</td></tr>\n");

	printf("<tr><td>Value:</td><td><input type=text name=value value=\"%d\" size=10 maxlength=10></td></tr>\n",
			it_temp[in].value);

	printf("<tr><td valign=top>Placement:</td><td>\n");
	printf("<input type=checkbox name=placement value=%d %s>Right Hand<br>\n",
			PL_WEAPON, (it_temp[in].placement & PL_WEAPON) ? "checked" : "");
	printf("<input type=checkbox name=placement value=%d %s>Two-Handed<br>\n",
			PL_TWOHAND, (it_temp[in].placement & PL_TWOHAND) ? "checked" : "");
	printf("<input type=checkbox name=placement value=%d %s>Left Hand<br>\n",
			PL_SHIELD, (it_temp[in].placement & PL_SHIELD) ? "checked" : "");
	printf("<input type=checkbox name=placement value=%d %s>Head<br>\n",
			PL_HEAD, (it_temp[in].placement & PL_HEAD) ? "checked" : "");
	printf("<input type=checkbox name=placement value=%d %s>Cloak<br>\n",
			PL_CLOAK, (it_temp[in].placement & PL_CLOAK) ? "checked" : "");
	printf("<input type=checkbox name=placement value=%d %s>Body<br>\n",
			PL_BODY, (it_temp[in].placement & PL_BODY) ? "checked" : "");
	printf("<input type=checkbox name=placement value=%d %s>Arms<br>\n",
			PL_ARMS, (it_temp[in].placement & PL_ARMS) ? "checked" : "");
	printf("<input type=checkbox name=placement value=%d %s>Feet<br>\n",
			PL_FEET, (it_temp[in].placement & PL_FEET) ? "checked" : "");
	printf("<input type=checkbox name=placement value=%d %s>Neck<br>\n",
			PL_NECK, (it_temp[in].placement & PL_NECK) ? "checked" : "");
	printf("<input type=checkbox name=placement value=%d %s>Belt<br>\n",
			PL_BELT, (it_temp[in].placement & PL_BELT) ? "checked" : "");
	printf("<input type=checkbox name=placement value=%d %s>Ring<br>\n",
			PL_RING, (it_temp[in].placement & PL_RING) ? "checked" : "");
	printf("<input type=checkbox name=placement value=%d %s>Charm<br>\n",
			PL_CHARM, (it_temp[in].placement & PL_CHARM) ? "checked" : "");
	printf("</td></tr>\n");


	printf("<tr><td colspan=2><table>\n");

	printf("<tr><td>Name</td><td>Inactive Mod</td><td>Active Mod</td><td>Min to wear</td><td>\n");

	for (n = 0; n<5; n++)
	{
		printf("<tr><td>%s</td>\n", at_name[n]);
		printf("<td><input type=text name=attrib%d_0 value=\"%d\" size=10 maxlength=10></td>\n", n, it_temp[in].attrib[n][I_I]);
		printf("<td><input type=text name=attrib%d_1 value=\"%d\" size=10 maxlength=10></td>\n", n, it_temp[in].attrib[n][I_A]);
		printf("<td><input type=text name=attrib%d_2 value=\"%d\" size=10 maxlength=10></td>\n", n, it_temp[in].attrib[n][I_R]);
		printf("</tr>\n");
	}

	printf("<tr><td>Hitpoints</td>\n");
	printf("<td><input type=text name=hp_0 value=\"%d\" size=10 maxlength=10></td>\n", it_temp[in].hp[I_I]);
	printf("<td><input type=text name=hp_1 value=\"%d\" size=10 maxlength=10></td>\n", it_temp[in].hp[I_A]);
	printf("<td><input type=text name=hp_2 value=\"%d\" size=10 maxlength=10></td>\n", it_temp[in].hp[I_R]);
	printf("</tr>\n");

	printf("<tr><td>Endurance</td>\n");
	printf("<td><input type=text name=end_0 value=\"%d\" size=10 maxlength=10></td>\n", it_temp[in].end[I_I]);
	printf("<td><input type=text name=end_1 value=\"%d\" size=10 maxlength=10></td>\n", it_temp[in].end[I_A]);
	printf("<td><input type=text name=end_2 value=\"%d\" size=10 maxlength=10></td>\n", it_temp[in].end[I_R]);
	printf("</tr>\n");

	printf("<tr><td>Mana</td>\n");
	printf("<td><input type=text name=mana_0 value=\"%d\" size=10 maxlength=10></td>\n", it_temp[in].mana[I_I]);
	printf("<td><input type=text name=mana_1 value=\"%d\" size=10 maxlength=10></td>\n", it_temp[in].mana[I_A]);
	printf("<td><input type=text name=mana_2 value=\"%d\" size=10 maxlength=10></td>\n", it_temp[in].mana[I_R]);
	printf("</tr>\n");

	for (n = 0; n<MAXSKILL; n++)
	{
		m = skillslots[n];
		if (skilltab[m].name[0]==0) continue;
		printf("<tr><td>%s</td>\n", skilltab[m].name);
		printf("<td><input type=text name=skill%d_0 value=\"%d\" size=10 maxlength=10></td>\n", m, it_temp[in].skill[m][I_I]);
		printf("<td><input type=text name=skill%d_1 value=\"%d\" size=10 maxlength=10></td>\n", m, it_temp[in].skill[m][I_A]);
		printf("<td><input type=text name=skill%d_2 value=\"%d\" size=10 maxlength=10></td>\n", m, it_temp[in].skill[m][I_R]);
		printf("</tr>\n");
	}

	printf("<tr><td>Armor:</td><td><input type=text name=armor_1 value=\"%d\" size=10 maxlength=10></td><td><input type=text name=armor_2 value=\"%d\" size=10 maxlength=10></td></tr>\n",
			it_temp[in].armor[I_I], it_temp[in].armor[I_A]);
	printf("<tr><td>Weapon:</td><td><input type=text name=weapon_1 value=\"%d\" size=10 maxlength=10></td><td><input type=text name=weapon_2 value=\"%d\" size=10 maxlength=10></td></tr>\n",
			it_temp[in].weapon[I_I], it_temp[in].weapon[I_A]);
	printf("<tr><td>Base Crit:</td><td><input type=text name=base_crit_1 value=\"%d\" size=10 maxlength=10></td><td><input type=text name=base_crit_2 value=\"%d\" size=10 maxlength=10></td></tr>\n",
			it_temp[in].base_crit[I_I], it_temp[in].base_crit[I_A]);
			
	// New Meta Stuff
	printf("<tr><td>Base Speed:</td><td><input type=text name=speed_1 value=\"%d\" size=10 maxlength=10></td><td><input type=text name=speed_2 value=\"%d\" size=10 maxlength=10></td></tr>\n",
			it_temp[in].speed[I_I], it_temp[in].speed[I_A]);
	printf("<tr><td>Move Speed:</td><td><input type=text name=move_speed_1 value=\"%d\" size=10 maxlength=10></td><td><input type=text name=move_speed_2 value=\"%d\" size=10 maxlength=10></td></tr>\n",
			it_temp[in].move_speed[I_I], it_temp[in].move_speed[I_A]);
	printf("<tr><td>Attack Speed:</td><td><input type=text name=atk_speed_1 value=\"%d\" size=10 maxlength=10></td><td><input type=text name=atk_speed_2 value=\"%d\" size=10 maxlength=10></td></tr>\n",
			it_temp[in].atk_speed[I_I], it_temp[in].atk_speed[I_A]);
	printf("<tr><td>Cast Speed:</td><td><input type=text name=cast_speed_1 value=\"%d\" size=10 maxlength=10></td><td><input type=text name=cast_speed_2 value=\"%d\" size=10 maxlength=10></td></tr>\n",
			it_temp[in].cast_speed[I_I], it_temp[in].cast_speed[I_A]);
			
	printf("<tr><td>Spell Mod:</td><td><input type=text name=spell_mod_1 value=\"%d\" size=10 maxlength=10></td><td><input type=text name=spell_mod_2 value=\"%d\" size=10 maxlength=10></td></tr>\n",
			it_temp[in].spell_mod[I_I], it_temp[in].spell_mod[I_A]);
	printf("<tr><td>Spell Aptitude:</td><td><input type=text name=spell_apt_1 value=\"%d\" size=10 maxlength=10></td><td><input type=text name=spell_apt_2 value=\"%d\" size=10 maxlength=10></td></tr>\n",
			it_temp[in].spell_apt[I_I], it_temp[in].spell_apt[I_A]);
	printf("<tr><td>Cooldown Bonus:</td><td><input type=text name=cool_bonus_1 value=\"%d\" size=10 maxlength=10></td><td><input type=text name=cool_bonus_2 value=\"%d\" size=10 maxlength=10></td></tr>\n",
			it_temp[in].cool_bonus[I_I], it_temp[in].cool_bonus[I_A]);
			
	printf("<tr><td>Crit Chance:</td><td><input type=text name=crit_chance_1 value=\"%d\" size=10 maxlength=10></td><td><input type=text name=crit_chance_2 value=\"%d\" size=10 maxlength=10></td></tr>\n",
			it_temp[in].crit_chance[I_I], it_temp[in].crit_chance[I_A]);
	printf("<tr><td>Crit Multi:</td><td><input type=text name=crit_multi_1 value=\"%d\" size=10 maxlength=10></td><td><input type=text name=crit_multi_2 value=\"%d\" size=10 maxlength=10></td></tr>\n",
			it_temp[in].crit_multi[I_I], it_temp[in].crit_multi[I_A]);
			
	printf("<tr><td>Hit Bonus:</td><td><input type=text name=to_hit_1 value=\"%d\" size=10 maxlength=10></td><td><input type=text name=to_hit_2 value=\"%d\" size=10 maxlength=10></td></tr>\n",
			it_temp[in].to_hit[I_I], it_temp[in].to_hit[I_A]);
	printf("<tr><td>Parry Bonus:</td><td><input type=text name=to_parry_1 value=\"%d\" size=10 maxlength=10></td><td><input type=text name=to_parry_2 value=\"%d\" size=10 maxlength=10></td></tr>\n",
			it_temp[in].to_parry[I_I], it_temp[in].to_parry[I_A]);
			
	printf("<tr><td>Top Dmg Bonus:</td><td><input type=text name=top_damage_1 value=\"%d\" size=10 maxlength=10></td><td><input type=text name=top_damage_2 value=\"%d\" size=10 maxlength=10></td></tr>\n",
			it_temp[in].top_damage[I_I], it_temp[in].top_damage[I_A]);
	//

	printf("<tr><td>Thorns:</td><td><input type=text name=gethit_dam_1 value=\"%d\" size=10 maxlength=10></td><td><input type=text name=gethit_dam_2 value=\"%d\" size=10 maxlength=10></td></tr>\n",
			it_temp[in].gethit_dam[I_I], it_temp[in].gethit_dam[I_A]);
	
	printf("<tr><td>AoE Bonus:</td><td><input type=text name=area_1 value=\"%d\" size=10 maxlength=10></td><td><input type=text name=area_2 value=\"%d\" size=10 maxlength=10></td></tr>\n",
			it_temp[in].aoe_bonus[I_I], it_temp[in].aoe_bonus[I_A]);
	
	printf("<tr><td>Dmg Bonus:</td><td><input type=text name=dmg_bonus_1 value=\"%d\" size=10 maxlength=10></td><td><input type=text name=dmg_bonus_2 value=\"%d\" size=10 maxlength=10></td></tr>\n",
			it_temp[in].dmg_bonus[I_I], it_temp[in].dmg_bonus[I_A]);
	
	printf("<tr><td>Dmg Reduction:</td><td><input type=text name=dmg_reduc_1 value=\"%d\" size=10 maxlength=10></td><td><input type=text name=dmg_reduc_2 value=\"%d\" size=10 maxlength=10></td></tr>\n",
			it_temp[in].dmg_reduction[I_I], it_temp[in].dmg_reduction[I_A]);

	printf("<tr><td>Max Age:</td><td><input type=text name=max_age_1 value=\"%d\" size=10 maxlength=10></td><td><input type=text name=max_age_2 value=\"%d\" size=10 maxlength=10></td></tr>\n",
			it_temp[in].max_age[I_I], it_temp[in].max_age[I_A]);

	printf("<tr><td>Light</td><td><input type=text name=light_1 value=\"%d\" size=10 maxlength=10></td>\n",
			it_temp[in].light[I_I]);
	printf("<td><input type=text name=light_2 value=\"%d\" size=10 maxlength=10></td></tr>\n",
			it_temp[in].light[I_A]);

	printf("<tr><td valign=top>Sprite:</td><td><input type=text name=sprite_1 value=\"%d\" size=10 maxlength=10></td>\n",
			it_temp[in].sprite[I_I]);
	printf("<td><input type=text name=sprite_2 value=\"%d\" size=10 maxlength=10></td></tr>\n",
			it_temp[in].sprite[I_A]);

	printf("<tr><td>Animation-Status:</td><td><input type=text name=status_1 value=\"%d\" size=10 maxlength=10></td>\n",
			it_temp[in].status[I_I]);
	printf("<td><input type=text name=status_2 value=\"%d\" size=10 maxlength=10></td></tr>\n",
			it_temp[in].status[I_A]);

	printf("</table></td></tr>\n");

	printf("<tr><td>Max Damage:</td><td><input type=text name=max_damage value=\"%d\" size=10 maxlength=10></td></tr>\n",
			it_temp[in].max_damage);

	printf("<tr><td>Duration:</td><td><input type=text name=duration value=\"%d\" size=10 maxlength=10> (in 1/18 of a sec)</td></tr>\n",
			it_temp[in].duration);
	printf("<tr><td>Cost:</td><td><input type=text name=cost value=\"%d\" size=10 maxlength=10></td></tr>\n",
			it_temp[in].cost);
	printf("<tr><td>Power:</td><td><input type=text name=power value=\"%d\" size=10 maxlength=10></td></tr>\n",
			it_temp[in].power);
	printf("<tr><td>Sprite Overr.:</td><td><input type=text name=spr_ovr value=\"%d\" size=10 maxlength=10></td></tr>\n",
			it_temp[in].sprite_override);

	printf("<tr><td>Min. Rank:</td><td><input type=text name=min_rank value=\"%d\" size=10 maxlength=10></td></tr>\n",
			it_temp[in].min_rank);

	printf("<tr><td valign=top>Driver:</td><td>\n");
	printf("<input type=text name=driver value=%d>", it_temp[in].driver);
	printf("</td></tr>\n");

	printf("<tr><td valign=top>Driver Data:</td><td>\n");
	printf("The way the data is used depends on the driver type!<br>\n");

	for (n = 0; n<10; n++)
	{
		printf("<tr><td>Drdata %d:</td><td><input type=text name=drdata%d value=\"%d\" size=11 maxlength=11></td></tr>\n",
				n, n, it_temp[in].data[n]);
	}

	printf("<tr><td><input type=submit value=Update></td><td> </td></tr>\n");
	phtml_tab_close();
	printf("<input type=hidden name=step value=24>\n");
	printf("<input type=hidden name=in value=%d>", in);
	printf("</form>\n");
	phtml_backhome(21);
}

void view_item(LIST *head)
{
	int in, n;
	char *tmp;
	int m;

	tmp = find_val(head, "in");
	if (tmp) in = atoi(tmp);
	else return page_err(P_ERR_NAN);
	if (in<1 || in>=MAXITEM) return page_err(P_ERR_OOB);
	
	phtml_backhome(27);
	printf("<form method=post action=/cgi-imp/acct.cgi>\n");
	phtml_tab_open();
	printf("<tr><td valign=top>Name:</td><td><input type=text name=name value=\"%s\" size=35 maxlength=35></td></tr>\n",
			it[in].name);
	printf("<tr><td>Reference:</td><td><input type=text name=reference value=\"%s\" size=35 maxlength=35></td></tr>\n",
			it[in].reference);
	printf("<tr><td>Description:</td><td><input type=text name=description value=\"%s\" size=35 maxlength=195></td></tr>\n",
			it[in].description);
	printf("<tr><td>Carried By:</td><td><input type=text name=carried value=\"%d\" size=10 maxlength=10>(%s)</td></tr>\n",
			it[in].carried, ch[it[in].carried].name);
	printf("<tr><td>X:</td><td><input type=text name=x value=\"%d\" size=10 maxlength=10></td></tr>\n",
			it[in].x);
	printf("<tr><td>Y:</td><td><input type=text name=y value=\"%d\" size=10 maxlength=10></td></tr>\n",
			it[in].y);

	printf("<tr><td valign=top>Flags:</td><td>\n");
	printf("<input type=checkbox name=flags value=%Lu %s>Moveblock<br>\n",
			IF_MOVEBLOCK, (it[in].flags & IF_MOVEBLOCK) ? "checked" : "");
	printf("<input type=checkbox name=flags value=%Lu %s>Sightblock<br>\n",
			IF_SIGHTBLOCK, (it[in].flags & IF_SIGHTBLOCK) ? "checked" : "");
	printf("<input type=checkbox name=flags value=%Lu %s>Take-Able<br>\n",
			IF_TAKE, (it[in].flags & IF_TAKE) ? "checked" : "");
	printf("<input type=checkbox name=flags value=%Lu %s>Look-Able<br>\n",
			IF_LOOK, (it[in].flags & IF_LOOK) ? "checked" : "");
	printf("<input type=checkbox name=flags value=%Lu %s>Look-Special<br>\n",
			IF_LOOKSPECIAL, (it[in].flags & IF_LOOKSPECIAL) ? "checked" : "");
	printf("<input type=checkbox name=flags value=%Lu %s>Use<br>\n",
			IF_USE, (it[in].flags & IF_USE) ? "checked" : "");
	printf("<input type=checkbox name=flags value=%Lu %s>Use-Special<br>\n",
			IF_USESPECIAL, (it[in].flags & IF_USESPECIAL) ? "checked" : "");
	printf("<input type=checkbox name=flags value=%Lu %s>Use-Destroy<br>\n",
			IF_USEDESTROY, (it[in].flags & IF_USEDESTROY) ? "checked" : "");
	printf("<input type=checkbox name=flags value=%Lu %s>Use-Activate<br>\n",
			IF_USEACTIVATE, (it[in].flags & IF_USEACTIVATE) ? "checked" : "");
	printf("<input type=checkbox name=flags value=%Lu %s>Use-Deactivate<br>\n",
			IF_USEDEACTIVATE, (it[in].flags & IF_USEDEACTIVATE) ? "checked" : "");
	printf("<input type=checkbox name=flags value=%Lu %s>Re-Activate<br>\n",
			IF_REACTIVATE, (it[in].flags & IF_REACTIVATE) ? "checked" : "");
	printf("<input type=checkbox name=flags value=%Lu %s>No-Repair<br>\n",
			IF_NOREPAIR, (it[in].flags & IF_NOREPAIR) ? "checked" : "");
	printf("<input type=checkbox name=flags value=%Lu %s>Stackable<br>\n",
			IF_STACKABLE, (it[in].flags & IF_STACKABLE) ? "checked" : "");

	printf("<input type=checkbox name=flags value=%Lu %s>Hidden (data[9])<br>\n",
			IF_HIDDEN, (it[in].flags & IF_HIDDEN) ? "checked" : "");
	printf("<input type=checkbox name=flags value=%Lu %s>Step-Action<br>\n",
			IF_STEPACTION, (it[in].flags & IF_STEPACTION) ? "checked" : "");
	printf("<input type=checkbox name=flags value=%Lu %s>Expire-Proc<br>\n",
			IF_EXPIREPROC, (it[in].flags & IF_EXPIREPROC) ? "checked" : "");

	printf("<input type=checkbox name=flags value=%Lu %s>Fast Light Age<br>\n",
			IF_LIGHTAGE, (it[in].flags & IF_LIGHTAGE) ? "checked" : "");

	printf("<input type=checkbox name=flags value=%Lu %s>Unique<br>\n",
			IF_UNIQUE, (it[in].flags & IF_UNIQUE) ? "checked" : "");

	printf("<input type=checkbox name=flags value=%Lu %s>Shop-Destroy (quest items)<br>\n",
			IF_SHOPDESTROY, (it[in].flags & IF_SHOPDESTROY) ? "checked" : "");
	printf("<input type=checkbox name=flags value=%Lu %s>Laby-Destroy (quest items)<br>\n",
			IF_LABYDESTROY, (it[in].flags & IF_LABYDESTROY) ? "checked" : "");
	printf("<input type=checkbox name=flags value=%Lu %s>No-Depot (quest items)<br>\n",
			IF_NODEPOT, (it[in].flags & IF_NODEPOT) ? "checked" : "");
	printf("<input type=checkbox name=flags value=%Lu %s>No Market (no price change)<br>\n",
			IF_NOMARKET, (it[in].flags & IF_NOMARKET) ? "checked" : "");
	printf("<input type=checkbox name=flags value=%Lu %s>Donate (cheap items)<br>\n",
			IF_DONATE, (it[in].flags & IF_DONATE) ? "checked" : "");
	printf("<input type=checkbox name=flags value=%Lu %s>Single-Age<br>\n",
			IF_SINGLEAGE, (it[in].flags & IF_SINGLEAGE) ? "checked" : "");
	printf("<input type=checkbox name=flags value=%Lu %s>Always expire when inactive<br>\n",
			IF_ALWAYSEXP1, (it[in].flags & IF_ALWAYSEXP1) ? "checked" : "");
	printf("<input type=checkbox name=flags value=%Lu %s>Always expire when active<br>\n",
			IF_ALWAYSEXP2, (it[in].flags & IF_ALWAYSEXP2) ? "checked" : "");
	printf("<input type=checkbox name=flags value=%Lu %s>Armor<br>\n",
			IF_ARMOR, (it[in].flags & IF_ARMOR) ? "checked" : "");
	printf("<input type=checkbox name=flags value=%Lu %s>Jewelery<br>\n",
			IF_JEWELERY, (it[in].flags & IF_JEWELERY) ? "checked" : "");
	printf("<input type=checkbox name=flags value=%Lu %s>Book<br>\n",
			IF_BOOK, (it[in].flags & IF_BOOK) ? "checked" : "");
	printf("<input type=checkbox name=flags value=%Lu %s>Magic<br>\n",
			IF_MAGIC, (it[in].flags & IF_MAGIC) ? "checked" : "");
	printf("<input type=checkbox name=flags value=%Lu %s>Gemstone<br>\n",
			IF_GEMSTONE, (it[in].flags & IF_GEMSTONE) ? "checked" : "");
	printf("<input type=checkbox name=flags value=%Lu %s>Is a Key<br>\n",
			IF_IS_KEY, (it[in].flags & IF_IS_KEY) ? "checked" : "");
	printf("<input type=checkbox name=flags value=%Lu %s>Misc<br>\n",
			IF_MISC, (it[in].flags & IF_MISC) ? "checked" : "");
	printf("<br>\n");
	printf("<input type=checkbox name=flags value=%Lu %s>Can be Soulstoned<br>\n",
			IF_CAN_SS, (it[in].flags & IF_CAN_SS) ? "checked" : "");
	printf("<input type=checkbox name=flags value=%Lu %s>Can be Enchanted<br>\n",
			IF_CAN_EN, (it[in].flags & IF_CAN_EN) ? "checked" : "");
	printf("<br>\n");
	printf("<input type=checkbox name=flags value=%Lu %s>Weapon: Claw<br>\n",
			IF_WP_CLAW, (it[in].flags & IF_WP_CLAW) ? "checked" : "");
	printf("<input type=checkbox name=flags value=%Lu %s>Weapon: Dagger<br>\n",
			IF_WP_DAGGER, (it[in].flags & IF_WP_DAGGER) ? "checked" : "");
	printf("<input type=checkbox name=flags value=%Lu %s>Weapon: Staff<br>\n",
			IF_WP_STAFF, (it[in].flags & IF_WP_STAFF) ? "checked" : "");
	printf("<input type=checkbox name=flags value=%Lu %s>Weapon: Sword<br>\n",
			IF_WP_SWORD, (it[in].flags & IF_WP_SWORD) ? "checked" : "");
	printf("<input type=checkbox name=flags value=%Lu %s>Weapon: Axe<br>\n",
			IF_WP_AXE, (it[in].flags & IF_WP_AXE) ? "checked" : "");
	printf("<input type=checkbox name=flags value=%Lu %s>Weapon: Two-Handed<br>\n",
			IF_WP_TWOHAND, (it[in].flags & IF_WP_TWOHAND) ? "checked" : "");
	printf("<input type=checkbox name=flags value=%Lu %s>Offhand: Dual Sword<br>\n",
			IF_OF_DUALSW, (it[in].flags & IF_OF_DUALSW) ? "checked" : "");
	printf("<input type=checkbox name=flags value=%Lu %s>Offhand: Shield<br>\n",
			IF_OF_SHIELD, (it[in].flags & IF_OF_SHIELD) ? "checked" : "");
	printf("</td></tr>\n");

	printf("<tr><td>Value:</td><td><input type=text name=value value=\"%d\" size=10 maxlength=10></td></tr>\n",
			it[in].value);

	printf("<tr><td valign=top>Placement:</td><td>\n");
	printf("<input type=checkbox name=placement value=%d %s>Right Hand<br>\n",
			PL_WEAPON, (it[in].placement & PL_WEAPON) ? "checked" : "");
	printf("<input type=checkbox name=placement value=%d %s>Two-Handed<br>\n",
			PL_TWOHAND, (it[in].placement & PL_TWOHAND) ? "checked" : "");
	printf("<input type=checkbox name=placement value=%d %s>Left Hand<br>\n",
			PL_SHIELD, (it[in].placement & PL_SHIELD) ? "checked" : "");
	printf("<input type=checkbox name=placement value=%d %s>Head<br>\n",
			PL_HEAD, (it[in].placement & PL_HEAD) ? "checked" : "");
	printf("<input type=checkbox name=placement value=%d %s>Cloak<br>\n",
			PL_CLOAK, (it[in].placement & PL_CLOAK) ? "checked" : "");
	printf("<input type=checkbox name=placement value=%d %s>Body<br>\n",
			PL_BODY, (it[in].placement & PL_BODY) ? "checked" : "");
	printf("<input type=checkbox name=placement value=%d %s>Arms<br>\n",
			PL_ARMS, (it[in].placement & PL_ARMS) ? "checked" : "");
	printf("<input type=checkbox name=placement value=%d %s>Feet<br>\n",
			PL_FEET, (it[in].placement & PL_FEET) ? "checked" : "");
	printf("<input type=checkbox name=placement value=%d %s>Neck<br>\n",
			PL_NECK, (it[in].placement & PL_NECK) ? "checked" : "");
	printf("<input type=checkbox name=placement value=%d %s>Belt<br>\n",
			PL_BELT, (it[in].placement & PL_BELT) ? "checked" : "");
	printf("<input type=checkbox name=placement value=%d %s>Ring<br>\n",
			PL_RING, (it[in].placement & PL_RING) ? "checked" : "");
	printf("<input type=checkbox name=placement value=%d %s>Charm<br>\n",
			PL_CHARM, (it[in].placement & PL_CHARM) ? "checked" : "");
	printf("</td></tr>\n");


	printf("<tr><td colspan=2><table>\n");

	printf("<tr><td>Name</td><td>Inactive Mod</td><td>Active Mod</td><td>Min to wear</td><td>\n");

	for (n = 0; n<5; n++)
	{
		printf("<tr><td>%s</td>\n", at_name[n]);
		printf("<td><input type=text name=attrib%d_0 value=\"%d\" size=10 maxlength=10></td>\n", n, it[in].attrib[n][I_I]);
		printf("<td><input type=text name=attrib%d_1 value=\"%d\" size=10 maxlength=10></td>\n", n, it[in].attrib[n][I_A]);
		printf("<td><input type=text name=attrib%d_2 value=\"%d\" size=10 maxlength=10></td>\n", n, it[in].attrib[n][I_R]);
		printf("</tr>\n");
	}

	printf("<tr><td>Hitpoints</td>\n");
	printf("<td><input type=text name=hp_0 value=\"%d\" size=10 maxlength=10></td>\n", it[in].hp[I_I]);
	printf("<td><input type=text name=hp_1 value=\"%d\" size=10 maxlength=10></td>\n", it[in].hp[I_A]);
	printf("<td><input type=text name=hp_2 value=\"%d\" size=10 maxlength=10></td>\n", it[in].hp[I_R]);
	printf("</tr>\n");

	printf("<tr><td>Endurance</td>\n");
	printf("<td><input type=text name=end_0 value=\"%d\" size=10 maxlength=10></td>\n", it[in].end[I_I]);
	printf("<td><input type=text name=end_1 value=\"%d\" size=10 maxlength=10></td>\n", it[in].end[I_A]);
	printf("<td><input type=text name=end_2 value=\"%d\" size=10 maxlength=10></td>\n", it[in].end[I_R]);
	printf("</tr>\n");

	printf("<tr><td>Mana</td>\n");
	printf("<td><input type=text name=mana_0 value=\"%d\" size=10 maxlength=10></td>\n", it[in].mana[I_I]);
	printf("<td><input type=text name=mana_1 value=\"%d\" size=10 maxlength=10></td>\n", it[in].mana[I_A]);
	printf("<td><input type=text name=mana_2 value=\"%d\" size=10 maxlength=10></td>\n", it[in].mana[I_R]);
	printf("</tr>\n");

	for (n = 0; n<MAXSKILL; n++)
	{
		m = skillslots[n];
		if (skilltab[m].name[0]==0) continue;
		printf("<tr><td>%s</td>\n", skilltab[m].name);
		printf("<td><input type=text name=skill%d_0 value=\"%d\" size=10 maxlength=10></td>\n", m, it[in].skill[m][I_I]);
		printf("<td><input type=text name=skill%d_1 value=\"%d\" size=10 maxlength=10></td>\n", m, it[in].skill[m][I_A]);
		printf("<td><input type=text name=skill%d_2 value=\"%d\" size=10 maxlength=10></td>\n", m, it[in].skill[m][I_R]);
		printf("</tr>\n");
	}

	printf("<tr><td>Armor:</td><td><input type=text name=armor_1 value=\"%d\" size=10 maxlength=10></td><td><input type=text name=armor_2 value=\"%d\" size=10 maxlength=10></td></tr>\n",
			it[in].armor[I_I], it[in].armor[I_A]);
	printf("<tr><td>Weapon:</td><td><input type=text name=weapon_1 value=\"%d\" size=10 maxlength=10></td><td><input type=text name=weapon_2 value=\"%d\" size=10 maxlength=10></td></tr>\n",
			it[in].weapon[I_I], it[in].weapon[I_A]);
	printf("<tr><td>Base Crit:</td><td><input type=text name=base_crit_1 value=\"%d\" size=10 maxlength=10></td><td><input type=text name=base_crit_2 value=\"%d\" size=10 maxlength=10></td></tr>\n",
			it[in].base_crit[I_I], it[in].base_crit[I_A]);
			
	// New Meta Stuff
	printf("<tr><td>Base Speed:</td><td><input type=text name=speed_1 value=\"%d\" size=10 maxlength=10></td><td><input type=text name=speed_2 value=\"%d\" size=10 maxlength=10></td></tr>\n",
			it[in].speed[I_I], it[in].speed[I_A]);
	printf("<tr><td>Move Speed:</td><td><input type=text name=move_speed_1 value=\"%d\" size=10 maxlength=10></td><td><input type=text name=move_speed_2 value=\"%d\" size=10 maxlength=10></td></tr>\n",
			it[in].move_speed[I_I], it[in].move_speed[I_A]);
	printf("<tr><td>Attack Speed:</td><td><input type=text name=atk_speed_1 value=\"%d\" size=10 maxlength=10></td><td><input type=text name=atk_speed_2 value=\"%d\" size=10 maxlength=10></td></tr>\n",
			it[in].atk_speed[I_I], it[in].atk_speed[I_A]);
	printf("<tr><td>Cast Speed:</td><td><input type=text name=cast_speed_1 value=\"%d\" size=10 maxlength=10></td><td><input type=text name=cast_speed_2 value=\"%d\" size=10 maxlength=10></td></tr>\n",
			it[in].cast_speed[I_I], it[in].cast_speed[I_A]);
			
	printf("<tr><td>Spell Mod:</td><td><input type=text name=spell_mod_1 value=\"%d\" size=10 maxlength=10></td><td><input type=text name=spell_mod_2 value=\"%d\" size=10 maxlength=10></td></tr>\n",
			it[in].spell_mod[I_I], it[in].spell_mod[I_A]);
	printf("<tr><td>Spell Aptitude:</td><td><input type=text name=spell_apt_1 value=\"%d\" size=10 maxlength=10></td><td><input type=text name=spell_apt_2 value=\"%d\" size=10 maxlength=10></td></tr>\n",
			it[in].spell_apt[I_I], it[in].spell_apt[I_A]);
	printf("<tr><td>Cooldown Bonus:</td><td><input type=text name=cool_bonus_1 value=\"%d\" size=10 maxlength=10></td><td><input type=text name=cool_bonus_2 value=\"%d\" size=10 maxlength=10></td></tr>\n",
			it[in].cool_bonus[I_I], it[in].cool_bonus[I_A]);
			
	printf("<tr><td>Crit Chance:</td><td><input type=text name=crit_chance_1 value=\"%d\" size=10 maxlength=10></td><td><input type=text name=crit_chance_2 value=\"%d\" size=10 maxlength=10></td></tr>\n",
			it[in].crit_chance[I_I], it[in].crit_chance[I_A]);
	printf("<tr><td>Crit Multi:</td><td><input type=text name=crit_multi_1 value=\"%d\" size=10 maxlength=10></td><td><input type=text name=crit_multi_2 value=\"%d\" size=10 maxlength=10></td></tr>\n",
			it[in].crit_multi[I_I], it[in].crit_multi[I_A]);
			
	printf("<tr><td>Hit Bonus:</td><td><input type=text name=to_hit_1 value=\"%d\" size=10 maxlength=10></td><td><input type=text name=to_hit_2 value=\"%d\" size=10 maxlength=10></td></tr>\n",
			it[in].to_hit[I_I], it[in].to_hit[I_A]);
	printf("<tr><td>Parry Bonus:</td><td><input type=text name=to_parry_1 value=\"%d\" size=10 maxlength=10></td><td><input type=text name=to_parry_2 value=\"%d\" size=10 maxlength=10></td></tr>\n",
			it[in].to_parry[I_I], it[in].to_parry[I_A]);
			
	printf("<tr><td>Top Dmg Bonus:</td><td><input type=text name=top_damage_1 value=\"%d\" size=10 maxlength=10></td><td><input type=text name=top_damage_2 value=\"%d\" size=10 maxlength=10></td></tr>\n",
			it[in].top_damage[I_I], it[in].top_damage[I_A]);
	//

	printf("<tr><td>Thorns:</td><td><input type=text name=gethit_dam_1 value=\"%d\" size=10 maxlength=10></td><td><input type=text name=gethit_dam_2 value=\"%d\" size=10 maxlength=10></td></tr>\n",
			it[in].gethit_dam[I_I], it[in].gethit_dam[I_A]);
			
	printf("<tr><td>AoE Bonus:</td><td><input type=text name=area_1 value=\"%d\" size=10 maxlength=10></td><td><input type=text name=area_2 value=\"%d\" size=10 maxlength=10></td></tr>\n",
			it[in].aoe_bonus[I_I], it[in].aoe_bonus[I_A]);
			
	printf("<tr><td>Dmg Bonus:</td><td><input type=text name=dmg_bonus_1 value=\"%d\" size=10 maxlength=10></td><td><input type=text name=dmg_bonus_2 value=\"%d\" size=10 maxlength=10></td></tr>\n",
			it[in].dmg_bonus[I_I], it[in].dmg_bonus[I_A]);
			
	printf("<tr><td>Dmg Reduction:</td><td><input type=text name=dmg_reduc_1 value=\"%d\" size=10 maxlength=10></td><td><input type=text name=dmg_reduc_2 value=\"%d\" size=10 maxlength=10></td></tr>\n",
			it[in].dmg_reduction[I_I], it[in].dmg_reduction[I_A]);

	printf("<tr><td>Max Age:</td><td><input type=text name=max_age_1 value=\"%d\" size=10 maxlength=10></td><td><input type=text name=max_age_2 value=\"%d\" size=10 maxlength=10></td></tr>\n",
			it[in].max_age[I_I], it[in].max_age[I_A]);

	printf("<tr><td>Light</td><td><input type=text name=light_1 value=\"%d\" size=10 maxlength=10></td>\n",
			it[in].light[I_I]);
	printf("<td><input type=text name=light_2 value=\"%d\" size=10 maxlength=10></td></tr>\n",
			it[in].light[I_A]);

	printf("<tr><td valign=top>Sprite:</td><td><input type=text name=sprite_1 value=\"%d\" size=10 maxlength=10></td>\n",
			it[in].sprite[I_I]);
	printf("<td><input type=text name=sprite_2 value=\"%d\" size=10 maxlength=10></td></tr>\n",
			it[in].sprite[I_A]);

	printf("<tr><td>Animation-Status:</td><td><input type=text name=status_1 value=\"%d\" size=10 maxlength=10></td>\n",
			it[in].status[I_I]);
	printf("<td><input type=text name=status_2 value=\"%d\" size=10 maxlength=10></td></tr>\n",
			it[in].status[I_A]);

	printf("</table></td></tr>\n");

	printf("<tr><td>Max Damage:</td><td><input type=text name=max_damage value=\"%d\" size=10 maxlength=10></td></tr>\n",
			it[in].max_damage);

	printf("<tr><td>Duration:</td><td><input type=text name=duration value=\"%d\" size=10 maxlength=10> (in 1/18 of a sec)</td></tr>\n",
			it[in].duration);
	printf("<tr><td>Cost:</td><td><input type=text name=cost value=\"%d\" size=10 maxlength=10></td></tr>\n",
			it[in].cost);
	printf("<tr><td>Power:</td><td><input type=text name=power value=\"%d\" size=10 maxlength=10></td></tr>\n",
			it[in].power);
	printf("<tr><td>Sprite Overr.:</td><td><input type=text name=spr_ovr value=\"%d\" size=10 maxlength=10></td></tr>\n",
			it[in].sprite_override);

	printf("<tr><td>Min. Rank:</td><td><input type=text name=min_rank value=\"%d\" size=10 maxlength=10></td></tr>\n",
			it[in].min_rank);

	printf("<tr><td valign=top>Driver:</td><td>\n");
	printf("<input type=text name=driver value=%d>", it[in].driver);
	printf("</td></tr>\n");

	printf("<tr><td valign=top>Driver Data:</td><td>\n");
	printf("The way the data is used depends on the driver type!<br>\n");

	for (n = 0; n<10; n++)
	{
		printf("<tr><td>Drdata %d:</td><td><input type=text name=drdata%d value=\"%d\" size=10 maxlength=10></td></tr>\n",
				n, n, it[in].data[n]);
	}

	printf("<tr><td><input type=submit value=Update></td><td> </td></tr>\n");
	phtml_tab_close();
	printf("<input type=hidden name=step value=24>\n");
	printf("<input type=hidden name=in value=%d>", in);
	printf("</form>\n");
	phtml_backhome(27);
}

void u_err(char *mdv)
{
	printf("%s not specified.\n", mdv);
}

void delete_character_template(LIST *head)
{
	int cn;
	char *tmp;

	tmp = find_val(head, "cn");
	if (tmp) cn = atoi(tmp); else return u_err("CN");
	ch_temp[cn].used = USE_EMPTY;
	if (ch_temp[cn].flags & CF_RESPAWN) globs->reset_char = cn;

	printf("Done.\n");
}

void delete_object(LIST *head)
{
	int in;
	char *tmp;

	tmp = find_val(head, "in");
	if (tmp) in = atoi(tmp); else return u_err("IN");
	it_temp[in].used = USE_EMPTY;

	printf("Done.\n");
}

void update_character_player(LIST *head)
{
	int cn, cnt, val, n;
	unsigned long long lval;
	char *tmp, **tmps, buf[80];
	
	tmp = find_val(head, "cn");
	if (tmp) cn = atoi(tmp);
	else return u_err("CN");
	
	bzero(&ch[cn], sizeof(struct character));
	ch[cn].used = USE_ACTIVE;
	
	tmp = find_val(head, "name");
	if (tmp) { strncpy(ch[cn].name, tmp, 35); ch[cn].name[35] = 0; }
	else return u_err("NAME");
	
	tmp = find_val(head, "reference");
	if (tmp) { strncpy(ch[cn].reference, tmp, 35); ch[cn].reference[35] = 0; }
	else return u_err("REFERENCE");
	
	tmp = find_val(head, "description");
	if (tmp) { strncpy(ch[cn].description, tmp, 195); ch[cn].description[195] = 0; }
	else return u_err("DESCRIPTION");
	
	tmp = find_val(head, "player");
	if (tmp) ch[cn].player = atoi(tmp);
	else return u_err("PLAYER");
	
	tmp = find_val(head, "pass1");
	if (tmp) ch[cn].pass1 = atoi(tmp);
	else return u_err("PASS1");
	
	tmp = find_val(head, "pass2");
	if (tmp) ch[cn].pass2 = atoi(tmp);
	else return u_err("PASS2");

	cnt = find_val_multi(head, "kindred", &tmps);
	if (cnt)
	{
		for (n = val = 0; n<cnt; n++)
		{
			val |= atoi(tmps[n]);
		}
		ch[cn].kindred = val;
	}

	tmp = find_val(head, "sprite");
	if (tmp)
	{
		ch[cn].sprite = atoi(tmp);
	}
	else
	{
		printf("SPRITE not specified.\n");
		return;
	}

	tmp = find_val(head, "sound");
	if (tmp)
	{
		ch[cn].sound = atoi(tmp);
	}
	else
	{
		printf("SOUND not specified.\n");
		return;
	}

	tmp = find_val(head, "class");
	if (tmp)
	{
		ch[cn].class = atoi(tmp);
	}
	else
	{
		printf("CLASS not specified.\n");
		return;
	}

	cnt  = find_val_multi(head, "flags", &tmps);
	lval = 0;
	if (cnt)
	{
		for (n = 0; n<cnt; n++)
		{
			lval |= atoll(tmps[n]);
		}
	}
	ch[cn].flags = lval;

	tmp = find_val(head, "alignment");
	if (tmp)
	{
		ch[cn].alignment = atoi(tmp);
	}
	else
	{
		printf("ALIGNMENT not specified.\n");
		return;
	}

	for (n = 0; n<100; n++)
	{
		sprintf(buf, "drdata%d", n);
		tmp = find_val(head, buf);
		if (tmp)
		{
			ch[cn].data[n] = atoi(tmp);
		}
		else
		{
			ch[cn].data[n] = 0;
		}
	}

	for (n = 0; n<10; n++)
	{
		sprintf(buf, "text_%d", n);
		tmp = find_val(head, buf);
		if (tmp)
		{
			strncpy(ch[cn].text[n], tmp, 158);
		}
		else
		{
			ch[cn].text[n][0] = 0;
		}
		ch[cn].text[n][159] = 0;
	}

	for (n = 0; n<5; n++)
	{
		sprintf(buf, "attrib%d_0", n);
		tmp = find_val(head, buf);
		if (tmp)
		{
			B_AT(cn, n) = atoi(tmp);
		}
		else
		{
			printf("ATTRIB%d_0 not specified.", n);
			return;
		}

		sprintf(buf, "attrib%d_1", n);
		tmp = find_val(head, buf);
		if (tmp)
		{
			ch[cn].attrib[n][1] = atoi(tmp);
		}
		else
		{
			ch[cn].attrib[n][1] = 0;
			//printf("ATTRIB%d_1 not specified.", n);
			//return;
		}

		sprintf(buf, "attrib%d_2", n);
		tmp = find_val(head, buf);
		if (tmp)
		{
			ch[cn].attrib[n][2] = atoi(tmp);
		}
		else
		{
			printf("ATTRIB%d_2 not specified.", n);
			return;
		}

		sprintf(buf, "attrib%d_3", n);
		tmp = find_val(head, buf);
		if (tmp)
		{
			ch[cn].attrib[n][3] = atoi(tmp);
		}
		else
		{
			printf("ATTRIB%d_3 not specified.", n);
			return;
		}
	}

	tmp = find_val(head, "hp_0");
	if (tmp)
	{
		ch[cn].hp[0] = atoi(tmp);
	}
	else
	{
		printf("HP_0 not specified.\n");
		return;
	}

	tmp = find_val(head, "hp_1");
	if (tmp)
	{
		ch[cn].hp[1] = atoi(tmp);
	}
	else
	{
		ch[cn].hp[1] = 0;
		//printf("HP_1 not specified.\n");
		//return;
	}

	tmp = find_val(head, "hp_2");
	if (tmp)
	{
		ch[cn].hp[2] = atoi(tmp);
	}
	else
	{
		printf("HP_2 not specified.\n");
		return;
	}

	tmp = find_val(head, "hp_3");
	if (tmp)
	{
		ch[cn].hp[3] = atoi(tmp);
	}
	else
	{
		printf("HP_3 not specified.\n");
		return;
	}

	tmp = find_val(head, "end_0");
	if (tmp)
	{
		ch[cn].end[0] = atoi(tmp);
	}
	else
	{
		printf("END_0 not specified.\n");
		return;
	}

	tmp = find_val(head, "end_1");
	if (tmp)
	{
		ch[cn].end[1] = atoi(tmp);
	}
	else
	{
		ch[cn].end[1] = 0;
		//printf("END_1 not specified.\n");
		//return;
	}

	tmp = find_val(head, "end_2");
	if (tmp)
	{
		ch[cn].end[2] = atoi(tmp);
	}
	else
	{
		printf("END_2 not specified.\n");
		return;
	}

	tmp = find_val(head, "end_3");
	if (tmp)
	{
		ch[cn].end[3] = atoi(tmp);
	}
	else
	{
		printf("END_3 not specified.\n");
		return;
	}

	tmp = find_val(head, "mana_0");
	if (tmp)
	{
		ch[cn].mana[0] = atoi(tmp);
	}
	else
	{
		printf("MANA_0 not specified.\n");
		return;
	}

	tmp = find_val(head, "mana_1");
	if (tmp)
	{
		ch[cn].mana[1] = atoi(tmp);
	}
	else
	{
		ch[cn].mana[1] = 0;
		//printf("MANA_1 not specified.\n");
		//return;
	}

	tmp = find_val(head, "mana_2");
	if (tmp)
	{
		ch[cn].mana[2] = atoi(tmp);
	}
	else
	{
		printf("MANA_2 not specified.\n");
		return;
	}

	tmp = find_val(head, "mana_3");
	if (tmp)
	{
		ch[cn].mana[3] = atoi(tmp);
	}
	else
	{
		printf("MANA_3 not specified.\n");
		return;
	}


	for (n = 0; n<MAXSKILL; n++)
	{
		sprintf(buf, "skill%d_0", n);
		tmp = find_val(head, buf);
		if (tmp)
		{
			B_SK(cn, n) = atoi(tmp);
		}
		else
		{
			B_SK(cn, n) = 0;
		}

		sprintf(buf, "skill%d_1", n);
		tmp = find_val(head, buf);
		if (tmp)
		{
			ch[cn].skill[n][1] = atoi(tmp);
		}
		else
		{
			ch[cn].skill[n][1] = 0;
		}

		sprintf(buf, "skill%d_2", n);
		tmp = find_val(head, buf);
		if (tmp)
		{
			ch[cn].skill[n][2] = atoi(tmp);
		}
		else
		{
			ch[cn].skill[n][2] = 0;
		}

		sprintf(buf, "skill%d_3", n);
		tmp = find_val(head, buf);
		if (tmp)
		{
			ch[cn].skill[n][3] = atoi(tmp);
		}
		else
		{
			ch[cn].skill[n][3] = 0;
		}
	}

	tmp = find_val(head, "speed_mod");
	if (tmp)
	{
		ch[cn].speed_mod = atoi(tmp);
	}
	else
	{
		printf("SPEED_MOD not specified.\n");
		return;
	}

	tmp = find_val(head, "weapon_bonus");
	if (tmp)
	{
		ch[cn].weapon_bonus = atoi(tmp);
	}
	else
	{
		printf("WEAPON_BONUS not specified.\n");
		return;
	}

	tmp = find_val(head, "light_bonus");
	if (tmp)
	{
		ch[cn].light_bonus = atoi(tmp);
	}
	else
	{
		printf("LIGHT_BONUS not specified.\n");
		return;
	}

	tmp = find_val(head, "armor_bonus");
	if (tmp)
	{
		ch[cn].armor_bonus = atoi(tmp);
	}
	else
	{
		printf("ARMOR_BONUS not specified.\n");
		return;
	}

	tmp = find_val(head, "points");
	if (tmp)
	{
		ch[cn].points = atoi(tmp);
	}
	else
	{
		printf("POINTS not specified.\n");
		return;
	}

	tmp = find_val(head, "points_tot");
	if (tmp)
	{
		ch[cn].points_tot = atoi(tmp);
	}
	else
	{
		printf("POINTS_TOT not specified.\n");
		return;
	}

	tmp = find_val(head, "x");
	if (tmp)
	{
		ch[cn].x = atoi(tmp);
	}
	else
	{
		printf("X not specified.\n");
		return;
	}

	tmp = find_val(head, "y");
	if (tmp)
	{
		ch[cn].y = atoi(tmp);
	}
	else
	{
		printf("Y not specified.\n");
		return;
	}

	tmp = find_val(head, "dir");
	if (tmp)
	{
		ch[cn].dir = atoi(tmp);
	}
	else
	{
		printf("DIR not specified.\n");
		return;
	}


	tmp = find_val(head, "temple_x");
	if (tmp)
	{
		ch[cn].temple_x = atoi(tmp);
	}
	else
	{
		printf("temple_x not specified.\n");
		return;
	}

	tmp = find_val(head, "temple_y");
	if (tmp)
	{
		ch[cn].temple_y = atoi(tmp);
	}
	else
	{
		printf("temple_y not specified.\n");
		return;
	}

	tmp = find_val(head, "tavern_x");
	if (tmp)
	{
		ch[cn].tavern_x = atoi(tmp);
	}
	else
	{
		printf("tavern_x not specified.\n");
		return;
	}

	tmp = find_val(head, "tavern_y");
	if (tmp)
	{
		ch[cn].tavern_y = atoi(tmp);
	}
	else
	{
		printf("tavern_y not specified.\n");
		return;
	}

	tmp = find_val(head, "temp");
	if (tmp)
	{
		ch[cn].temp = atoi(tmp);
	}
	else
	{
		printf("temp not specified.\n");
		return;
	}


	tmp = find_val(head, "tox");
	if (tmp)
	{
		ch[cn].tox = atoi(tmp);
	}
	else
	{
		printf("tox not specified.\n");
		return;
	}
	tmp = find_val(head, "toy");
	if (tmp)
	{
		ch[cn].toy = atoi(tmp);
	}
	else
	{
		printf("toy not specified.\n");
		return;
	}
	tmp = find_val(head, "frx");
	if (tmp)
	{
		ch[cn].frx = atoi(tmp);
	}
	else
	{
		printf("frx not specified.\n");
		return;
	}
	tmp = find_val(head, "fry");
	if (tmp)
	{
		ch[cn].fry = atoi(tmp);
	}
	else
	{
		printf("fry not specified.\n");
		return;
	}
	tmp = find_val(head, "status");
	if (tmp)
	{
		ch[cn].status = atoi(tmp);
	}
	else
	{
		printf("status not specified.\n");
		return;
	}
	tmp = find_val(head, "status2");
	if (tmp)
	{
		ch[cn].status2 = atoi(tmp);
	}
	else
	{
		printf("status2 not specified.\n");
		return;
	}


	tmp = find_val(head, "gold");
	if (tmp)
	{
		ch[cn].gold = atoi(tmp);
	}
	else
	{
		printf("GOLD not specified.\n");
		return;
	}

	for (n = 0; n<20; n++)
	{
		sprintf(buf, "worn%d", n);
		tmp = find_val(head, buf);
		if (tmp)
		{
			ch[cn].worn[n] = atoi(tmp);
		}
	}
	
	for (n = 0; n<12; n++)
	{
		sprintf(buf, "alt_worn%d", n);
		tmp = find_val(head, buf);
		if (tmp)
		{
			ch[cn].alt_worn[n] = atoi(tmp);
		}
	}

	for (n = 0; n<MAXITEMS; n++)
	{
		sprintf(buf, "item%d", n);
		tmp = find_val(head, buf);
		if (tmp)
		{
			ch[cn].item[n] = atoi(tmp);
		}
	}

	tmp = find_val(head, "citem");
	if (tmp)
	{
		ch[cn].citem = atoi(tmp);
	}
	else
	{
		printf("CITEM not specified.\n");
		return;
	}

	printf("Done.\n");
}

void update_character_template(LIST *head)
{
	int cn, cnt, val, n;
	unsigned long long lval;
	char *tmp, **tmps, buf[80];

	tmp = find_val(head, "cn");
	if (tmp)
	{
		cn = atoi(tmp);
	}
	else
	{
		printf("CN not specified.\n");
		return;
	}

	bzero(&ch_temp[cn], sizeof(struct character));
	ch_temp[cn].used = USE_ACTIVE;

	tmp = find_val(head, "name");
	if (tmp)
	{
		strncpy(ch_temp[cn].name, tmp, 35);
		ch_temp[cn].name[35] = 0;
	}
	else
	{
		printf("NAME not specified.\n");
		return;
	}

	tmp = find_val(head, "reference");
	if (tmp)
	{
		strncpy(ch_temp[cn].reference, tmp, 35);
		ch_temp[cn].reference[35] = 0;
	}
	else
	{
		printf("REFERENCE not specified.\n");
		return;
	}

	tmp = find_val(head, "description");
	if (tmp)
	{
		strncpy(ch_temp[cn].description, tmp, 195);
		ch_temp[cn].description[195] = 0;
	}
	else
	{
		printf("DESCRIPTION not specified.\n");
		return;
	}

	cnt = find_val_multi(head, "kindred", &tmps);
	if (cnt)
	{
		for (n = val = 0; n<cnt; n++)
		{
			val |= atoi(tmps[n]);
		}
		ch_temp[cn].kindred = val;
	}

	tmp = find_val(head, "sprite");
	if (tmp)
	{
		ch_temp[cn].sprite = atoi(tmp);
	}
	else
	{
		printf("SPRITE not specified.\n");
		return;
	}

	tmp = find_val(head, "sound");
	if (tmp)
	{
		ch_temp[cn].sound = atoi(tmp);
	}
	else
	{
		printf("SOUND not specified.\n");
		return;
	}

	tmp = find_val(head, "class");
	if (tmp)
	{
		ch_temp[cn].class = atoi(tmp);
	}
	else
	{
		printf("CLASS not specified.\n");
		return;
	}

	cnt  = find_val_multi(head, "flags", &tmps);
	lval = 0;
	if (cnt)
	{
		for (n = 0; n<cnt; n++)
		{
			lval |= atoll(tmps[n]);
		}
	}
	ch_temp[cn].flags = lval;

	tmp = find_val(head, "alignment");
	if (tmp)
	{
		ch_temp[cn].alignment = atoi(tmp);
	}
	else
	{
		printf("ALIGNMENT not specified.\n");
		return;
	}

	for (n = 0; n<100; n++)
	{
		sprintf(buf, "drdata%d", n);
		tmp = find_val(head, buf);
		if (tmp)
		{
			ch_temp[cn].data[n] = atoi(tmp);
		}
		else
		{
			ch_temp[cn].data[n] = 0;
		}
	}

	for (n = 0; n<10; n++)
	{
		sprintf(buf, "text_%d", n);
		tmp = find_val(head, buf);
		if (tmp)
		{
			strncpy(ch_temp[cn].text[n], tmp, 158);
		}
		else
		{
			ch_temp[cn].text[n][0] = 0;
		}
		ch_temp[cn].text[n][159] = 0;
	}

	for (n = 0; n<5; n++)
	{
		sprintf(buf, "attrib%d_0", n);
		tmp = find_val(head, buf);
		if (tmp)
		{
			ch_temp[cn].attrib[n][0] = atoi(tmp);
		}
		else
		{
			printf("ATTRIB%d_0 not specified.", n);
			return;
		}

		sprintf(buf, "attrib%d_1", n);
		tmp = find_val(head, buf);
		if (tmp)
		{
			ch_temp[cn].attrib[n][1] = atoi(tmp);
		}
		else
		{
			ch_temp[cn].attrib[n][1] = 0;
			//printf("ATTRIB%d_1 not specified.", n);
			//return;
		}

		sprintf(buf, "attrib%d_2", n);
		tmp = find_val(head, buf);
		if (tmp)
		{
			ch_temp[cn].attrib[n][2] = atoi(tmp);
		}
		else
		{
			printf("ATTRIB%d_2 not specified.", n);
			return;
		}

		sprintf(buf, "attrib%d_3", n);
		tmp = find_val(head, buf);
		if (tmp)
		{
			ch_temp[cn].attrib[n][3] = atoi(tmp);
		}
		else
		{
			printf("ATTRIB%d_3 not specified.", n);
			return;
		}
	}

	tmp = find_val(head, "hp_0");
	if (tmp)
	{
		ch_temp[cn].hp[0] = atoi(tmp);
	}
	else
	{
		printf("HP_0 not specified.\n");
		return;
	}

	tmp = find_val(head, "hp_1");
	if (tmp)
	{
		ch_temp[cn].hp[1] = atoi(tmp);
	}
	else
	{
		ch_temp[cn].hp[1] = 0;
		//printf("HP_1 not specified.\n");
		//return;
	}

	tmp = find_val(head, "hp_2");
	if (tmp)
	{
		ch_temp[cn].hp[2] = atoi(tmp);
	}
	else
	{
		printf("HP_2 not specified.\n");
		return;
	}

	tmp = find_val(head, "hp_3");
	if (tmp)
	{
		ch_temp[cn].hp[3] = atoi(tmp);
	}
	else
	{
		printf("HP_3 not specified.\n");
		return;
	}

	tmp = find_val(head, "end_0");
	if (tmp)
	{
		ch_temp[cn].end[0] = atoi(tmp);
	}
	else
	{
		printf("END_0 not specified.\n");
		return;
	}

	tmp = find_val(head, "end_1");
	if (tmp)
	{
		ch_temp[cn].end[1] = atoi(tmp);
	}
	else
	{
		ch_temp[cn].end[1] = 0;
		//printf("END_1 not specified.\n");
		//return;
	}

	tmp = find_val(head, "end_2");
	if (tmp)
	{
		ch_temp[cn].end[2] = atoi(tmp);
	}
	else
	{
		printf("END_2 not specified.\n");
		return;
	}

	tmp = find_val(head, "end_3");
	if (tmp)
	{
		ch_temp[cn].end[3] = atoi(tmp);
	}
	else
	{
		printf("END_3 not specified.\n");
		return;
	}

	tmp = find_val(head, "mana_0");
	if (tmp)
	{
		ch_temp[cn].mana[0] = atoi(tmp);
	}
	else
	{
		printf("MANA_0 not specified.\n");
		return;
	}

	tmp = find_val(head, "mana_1");
	if (tmp)
	{
		ch_temp[cn].mana[1] = atoi(tmp);
	}
	else
	{
		ch_temp[cn].mana[1] = 0;
		//printf("MANA_1 not specified.\n");
		//return;
	}

	tmp = find_val(head, "mana_2");
	if (tmp)
	{
		ch_temp[cn].mana[2] = atoi(tmp);
	}
	else
	{
		printf("MANA_2 not specified.\n");
		return;
	}

	tmp = find_val(head, "mana_3");
	if (tmp)
	{
		ch_temp[cn].mana[3] = atoi(tmp);
	}
	else
	{
		printf("MANA_3 not specified.\n");
		return;
	}


	for (n = 0; n<MAXSKILL; n++)
	{
		sprintf(buf, "skill%d_0", n);
		tmp = find_val(head, buf);
		if (tmp)
		{
			ch_temp[cn].skill[n][0] = atoi(tmp);
		}
		else
		{
			ch_temp[cn].skill[n][0] = 0;
		}

		sprintf(buf, "skill%d_1", n);
		tmp = find_val(head, buf);
		if (tmp)
		{
			ch_temp[cn].skill[n][1] = atoi(tmp);
		}
		else
		{
			ch_temp[cn].skill[n][1] = 0;
		}

		sprintf(buf, "skill%d_2", n);
		tmp = find_val(head, buf);
		if (tmp)
		{
			ch_temp[cn].skill[n][2] = atoi(tmp);
		}
		else
		{
			ch_temp[cn].skill[n][2] = 0;
		}

		sprintf(buf, "skill%d_3", n);
		tmp = find_val(head, buf);
		if (tmp)
		{
			ch_temp[cn].skill[n][3] = atoi(tmp);
		}
		else
		{
			ch_temp[cn].skill[n][3] = 0;
		}
	}

	tmp = find_val(head, "speed_mod");
	if (tmp)
	{
		ch_temp[cn].speed_mod = atoi(tmp);
	}
	else
	{
		printf("SPEED_MOD not specified.\n");
		return;
	}

	tmp = find_val(head, "weapon_bonus");
	if (tmp)
	{
		ch_temp[cn].weapon_bonus = atoi(tmp);
	}
	else
	{
		printf("WEAPON_BONUS not specified.\n");
		return;
	}

	tmp = find_val(head, "light_bonus");
	if (tmp)
	{
		ch_temp[cn].light_bonus = atoi(tmp);
	}
	else
	{
		printf("LIGHT_BONUS not specified.\n");
		return;
	}

	tmp = find_val(head, "armor_bonus");
	if (tmp)
	{
		ch_temp[cn].armor_bonus = atoi(tmp);
	}
	else
	{
		printf("ARMOR_BONUS not specified.\n");
		return;
	}

	tmp = find_val(head, "points");
	if (tmp)
	{
		ch_temp[cn].points = atoi(tmp);
	}
	else
	{
		printf("POINTS not specified.\n");
		return;
	}

	tmp = find_val(head, "points_tot");
	if (tmp)
	{
		ch_temp[cn].points_tot = atoi(tmp);
	}
	else
	{
		printf("POINTS_TOT not specified.\n");
		return;
	}

	tmp = find_val(head, "x");
	if (tmp)
	{
		ch_temp[cn].x = atoi(tmp);
	}
	else
	{
		printf("X not specified.\n");
		return;
	}

	tmp = find_val(head, "y");
	if (tmp)
	{
		ch_temp[cn].y = atoi(tmp);
	}
	else
	{
		printf("Y not specified.\n");
		return;
	}

	tmp = find_val(head, "dir");
	if (tmp)
	{
		ch_temp[cn].dir = atoi(tmp);
	}
	else
	{
		printf("DIR not specified.\n");
		return;
	}

	tmp = find_val(head, "gold");
	if (tmp)
	{
		ch_temp[cn].gold = atoi(tmp);
	}
	else
	{
		printf("GOLD not specified.\n");
		return;
	}

	for (n = 0; n<20; n++)
	{
		sprintf(buf, "worn%d", n);
		tmp = find_val(head, buf);
		if (tmp)
		{
			ch_temp[cn].worn[n] = atoi(tmp);
		}
	}

	for (n = 0; n<MAXITEMS; n++)
	{
		sprintf(buf, "item%d", n);
		tmp = find_val(head, buf);
		if (tmp)
		{
			ch_temp[cn].item[n] = atoi(tmp);
		}
	}

	tmp = find_val(head, "citem");
	if (tmp)
	{
		ch_temp[cn].citem = atoi(tmp);
	}
	else
	{
		printf("CITEM not specified.\n");
		return;
	}

	if (ch_temp[cn].flags & CF_RESPAWN)
	{
		globs->reset_char = cn;
	}

	if (!ch_temp[cn].data[29] && (ch_temp[cn].flags & CF_RESPAWN))
	{
		ch_temp[cn].data[29] = ch_temp[cn].x + ch_temp[cn].y * MAPX;
	}

	printf("Done.\n");
}


void update_object(LIST *head)
{
	int in, cnt, val, n;
	unsigned long long lval;
	char *tmp, **tmps, buf[80];
	
	tmp = find_val(head, "in");
	if (tmp) in = atoi(tmp); else return u_err("IN");

	bzero(&it_temp[in], sizeof(struct item));
	it_temp[in].used = USE_ACTIVE;

	tmp = find_val(head, "name");
	if (tmp) { strncpy(it_temp[in].name, tmp, 35); it_temp[in].name[35] = 0; } else return u_err("NAME");

	tmp = find_val(head, "reference");
	if (tmp) { strncpy(it_temp[in].reference, tmp, 35); it_temp[in].reference[35] = 0; } else return u_err("REFERENCE");

	tmp = find_val(head, "description");
	if (tmp) { strncpy(it_temp[in].description, tmp, 195); it_temp[in].description[195] = 0; } else return u_err("DESCRIPTION");

	cnt  = find_val_multi(head, "flags", &tmps);
	lval = 0;
	if (cnt) for (n = 0; n<cnt; n++) lval |= atoll(tmps[n]);
	it_temp[in].flags = lval;

	tmp = find_val(head, "driver");
	if (tmp) it_temp[in].driver = atoi(tmp); else return u_err("DRIVER");

	for (n = 0; n<100; n++)	{ sprintf(buf, "drdata%d", n); tmp = find_val(head, buf); if (tmp) it_temp[in].data[n] = atoi(tmp); }

	tmp = find_val(head, "light_1");
	if (tmp)
	{
		it_temp[in].light[I_I] = atoi(tmp);
	}
	else
	{
		printf("LIGHT_1 not specified.\n");
		return;
	}

	tmp = find_val(head, "light_2");
	if (tmp)
	{
		it_temp[in].light[I_A] = atoi(tmp);
	}
	else
	{
		printf("LIGHT_2 not specified.\n");
		return;
	}

	tmp = find_val(head, "value");
	if (tmp)
	{
		it_temp[in].value = atoi(tmp);
	}
	else
	{
		printf("VALUE not specified.\n");
		return;
	}

	cnt = find_val_multi(head, "placement", &tmps);
	if (cnt)
	{
		for (n = val = 0; n<cnt; n++)
		{
			val |= atoi(tmps[n]);
		}
		it_temp[in].placement = val;
	}

	for (n = 0; n<5; n++)
	{
		sprintf(buf, "attrib%d_0", n);
		tmp = find_val(head, buf);
		if (tmp)
		{
			it_temp[in].attrib[n][I_I] = atoi(tmp);
		}
		else
		{
			printf("ATTRIB%d_0 not specified.", n);
			return;
		}

		sprintf(buf, "attrib%d_1", n);
		tmp = find_val(head, buf);
		if (tmp)
		{
			it_temp[in].attrib[n][I_A] = atoi(tmp);
		}
		else
		{
			printf("ATTRIB%d_1 not specified.", n);
			return;
		}

		sprintf(buf, "attrib%d_2", n);
		tmp = find_val(head, buf);
		if (tmp)
		{
			it_temp[in].attrib[n][I_R] = atoi(tmp);
		}
		else
		{
			printf("ATTRIB%d_2 not specified.", n);
			return;
		}
	}

	tmp = find_val(head, "hp_0");
	if (tmp) { it_temp[in].hp[I_I] = atoi(tmp); }
	else { printf("HP_0 not specified.\n"); return; }

	tmp = find_val(head, "hp_1");
	if (tmp) { it_temp[in].hp[I_A] = atoi(tmp); }
	else { printf("HP_1 not specified.\n"); return; }

	tmp = find_val(head, "hp_2");
	if (tmp) { it_temp[in].hp[I_R] = atoi(tmp); }
	else { printf("HP_2 not specified.\n"); return; }

	tmp = find_val(head, "end_0");
	if (tmp) { it_temp[in].end[I_I] = atoi(tmp); }
	else { printf("END_0 not specified.\n"); return; }

	tmp = find_val(head, "end_1");
	if (tmp) { it_temp[in].end[I_A] = atoi(tmp); }
	else { printf("END_1 not specified.\n"); return; }

	tmp = find_val(head, "end_2");
	if (tmp) { it_temp[in].end[I_R] = atoi(tmp); }
	else { printf("END_2 not specified.\n"); return; }

	tmp = find_val(head, "mana_0");
	if (tmp) { it_temp[in].mana[I_I] = atoi(tmp); }
	else { printf("MANA_0 not specified.\n"); return; }

	tmp = find_val(head, "mana_1");
	if (tmp) { it_temp[in].mana[I_A] = atoi(tmp); }
	else { printf("MANA_1 not specified.\n"); return; }

	tmp = find_val(head, "mana_2");
	if (tmp) { it_temp[in].mana[I_R] = atoi(tmp); }
	else { printf("MANA_2 not specified.\n"); return; }

	for (n = 0; n<MAXSKILL; n++)
	{
		sprintf(buf, "skill%d_0", n);
		tmp = find_val(head, buf);
		if (tmp)
		{
			it_temp[in].skill[n][I_I] = atoi(tmp);
		}
		else
		{
			it_temp[in].skill[n][I_I] = 0;
		}

		sprintf(buf, "skill%d_1", n);
		tmp = find_val(head, buf);
		if (tmp)
		{
			it_temp[in].skill[n][I_A] = atoi(tmp);
		}
		else
		{
			it_temp[in].skill[n][I_A] = 0;
		}

		sprintf(buf, "skill%d_2", n);
		tmp = find_val(head, buf);
		if (tmp)
		{
			it_temp[in].skill[n][I_R] = atoi(tmp);
		}
		else
		{
			it_temp[in].skill[n][I_R] = 0;
		}
	}

	tmp = find_val(head, "armor_1");
	if (tmp)
	{
		it_temp[in].armor[I_I] = atoi(tmp);
	}
	else
	{
		printf("ARMOR_1 not specified.\n");
		return;
	}

	tmp = find_val(head, "armor_2");
	if (tmp)
	{
		it_temp[in].armor[I_A] = atoi(tmp);
	}
	else
	{
		printf("ARMOR_2 not specified.\n");
		return;
	}

	tmp = find_val(head, "weapon_1");
	if (tmp)
	{
		it_temp[in].weapon[I_I] = atoi(tmp);
	}
	else
	{
		printf("WEAPON_1 not specified.\n");
		return;
	}

	tmp = find_val(head, "weapon_2");
	if (tmp)
	{
		it_temp[in].weapon[I_A] = atoi(tmp);
	}
	else
	{
		printf("WEAPON_2 not specified.\n");
		return;
	}
	
	// New Meta Stuff
	tmp = find_val(head, "base_crit_1");
	if (tmp)
	{
		it_temp[in].base_crit[I_I] = atoi(tmp);
	}
	else
	{
		printf("BASE_CRIT_1 not specified.\n");
		return;
	}
	tmp = find_val(head, "base_crit_2");
	if (tmp)
	{
		it_temp[in].base_crit[I_A] = atoi(tmp);
	}
	else
	{
		printf("BASE_CRIT_2 not specified.\n");
		return;
	}
	
	tmp = find_val(head, "speed_1");
	if (tmp)
	{
		it_temp[in].speed[I_I] = atoi(tmp);
	}
	else
	{
		printf("speed_1 not specified.\n");
		return;
	}
	
	tmp = find_val(head, "speed_2");
	if (tmp)
	{
		it_temp[in].speed[I_A] = atoi(tmp);
	}
	else
	{
		printf("speed_2 not specified.\n");
		return;
	}
	
	tmp = find_val(head, "move_speed_1");
	if (tmp)
	{
		it_temp[in].move_speed[I_I] = atoi(tmp);
	}
	else
	{
		printf("move_speed_1 not specified.\n");
		return;
	}
	
	tmp = find_val(head, "move_speed_2");
	if (tmp)
	{
		it_temp[in].move_speed[I_A] = atoi(tmp);
	}
	else
	{
		printf("move_speed_2 not specified.\n");
		return;
	}
	
	tmp = find_val(head, "atk_speed_1");
	if (tmp)
	{
		it_temp[in].atk_speed[I_I] = atoi(tmp);
	}
	else
	{
		printf("atk_speed_1 not specified.\n");
		return;
	}
	
	tmp = find_val(head, "atk_speed_2");
	if (tmp)
	{
		it_temp[in].atk_speed[I_A] = atoi(tmp);
	}
	else
	{
		printf("atk_speed_2 not specified.\n");
		return;
	}
	
	tmp = find_val(head, "cast_speed_1");
	if (tmp)
	{
		it_temp[in].cast_speed[I_I] = atoi(tmp);
	}
	else
	{
		printf("cast_speed_1 not specified.\n");
		return;
	}
	
	tmp = find_val(head, "cast_speed_2");
	if (tmp)
	{
		it_temp[in].cast_speed[I_A] = atoi(tmp);
	}
	else
	{
		printf("cast_speed_2 not specified.\n");
		return;
	}
	
	tmp = find_val(head, "spell_mod_1");
	if (tmp)
	{
		it_temp[in].spell_mod[I_I] = atoi(tmp);
	}
	else
	{
		printf("spell_mod_1 not specified.\n");
		return;
	}
	
	tmp = find_val(head, "spell_mod_2");
	if (tmp)
	{
		it_temp[in].spell_mod[I_A] = atoi(tmp);
	}
	else
	{
		printf("spell_mod_2 not specified.\n");
		return;
	}
	
	tmp = find_val(head, "spell_apt_1");
	if (tmp)
	{
		it_temp[in].spell_apt[I_I] = atoi(tmp);
	}
	else
	{
		printf("spell_apt_1 not specified.\n");
		return;
	}
	
	tmp = find_val(head, "spell_apt_2");
	if (tmp)
	{
		it_temp[in].spell_apt[I_A] = atoi(tmp);
	}
	else
	{
		printf("spell_apt_2 not specified.\n");
		return;
	}
	
	tmp = find_val(head, "cool_bonus_1");
	if (tmp)
	{
		it_temp[in].cool_bonus[I_I] = atoi(tmp);
	}
	else
	{
		printf("cool_bonus_1 not specified.\n");
		return;
	}
	
	tmp = find_val(head, "cool_bonus_2");
	if (tmp)
	{
		it_temp[in].cool_bonus[I_A] = atoi(tmp);
	}
	else
	{
		printf("cool_bonus_2 not specified.\n");
		return;
	}
	
	tmp = find_val(head, "crit_chance_1");
	if (tmp)
	{
		it_temp[in].crit_chance[I_I] = atoi(tmp);
	}
	else
	{
		printf("crit_chance_1 not specified.\n");
		return;
	}
	
	tmp = find_val(head, "crit_chance_2");
	if (tmp)
	{
		it_temp[in].crit_chance[I_A] = atoi(tmp);
	}
	else
	{
		printf("crit_chance_2 not specified.\n");
		return;
	}
	
	tmp = find_val(head, "crit_multi_1");
	if (tmp)
	{
		it_temp[in].crit_multi[I_I] = atoi(tmp);
	}
	else
	{
		printf("crit_multi_1 not specified.\n");
		return;
	}
	
	tmp = find_val(head, "crit_multi_2");
	if (tmp)
	{
		it_temp[in].crit_multi[I_A] = atoi(tmp);
	}
	else
	{
		printf("crit_multi_2 not specified.\n");
		return;
	}
	
	tmp = find_val(head, "to_hit_1");
	if (tmp)
	{
		it_temp[in].to_hit[I_I] = atoi(tmp);
	}
	else
	{
		printf("to_hit_1 not specified.\n");
		return;
	}
	
	tmp = find_val(head, "to_hit_2");
	if (tmp)
	{
		it_temp[in].to_hit[I_A] = atoi(tmp);
	}
	else
	{
		printf("to_hit_2 not specified.\n");
		return;
	}
	
	tmp = find_val(head, "to_parry_1");
	if (tmp)
	{
		it_temp[in].to_parry[I_I] = atoi(tmp);
	}
	else
	{
		printf("to_parry_1 not specified.\n");
		return;
	}
	
	tmp = find_val(head, "to_parry_2");
	if (tmp)
	{
		it_temp[in].to_parry[I_A] = atoi(tmp);
	}
	else
	{
		printf("to_parry_2 not specified.\n");
		return;
	}
	
	tmp = find_val(head, "top_damage_1");
	if (tmp)
	{
		it_temp[in].top_damage[I_I] = atoi(tmp);
	}
	else
	{
		printf("top_damage_1 not specified.\n");
		return;
	}
	
	tmp = find_val(head, "top_damage_2");
	if (tmp)
	{
		it_temp[in].top_damage[I_A] = atoi(tmp);
	}
	else
	{
		printf("top_damage_2 not specified.\n");
		return;
	}
	//

	tmp = find_val(head, "gethit_dam_1");
	if (tmp)
	{
		it_temp[in].gethit_dam[I_I] = atoi(tmp);
	}
	else
	{
		printf("GETHIT_DAM_1 not specified.\n");
		return;
	}

	tmp = find_val(head, "gethit_dam_2");
	if (tmp)
	{
		it_temp[in].gethit_dam[I_A] = atoi(tmp);
	}
	else
	{
		printf("GETHIT_DAM_2 not specified.\n");
		return;
	}
	
	tmp = find_val(head, "area_1");
	if (tmp)
	{
		it_temp[in].aoe_bonus[I_I] = atoi(tmp);
	}
	else
	{
		printf("area_1 not specified.\n");
		return;
	}
	
	tmp = find_val(head, "area_2");
	if (tmp)
	{
		it_temp[in].aoe_bonus[I_A] = atoi(tmp);
	}
	else
	{
		printf("area_2 not specified.\n");
		return;
	}
	
	tmp = find_val(head, "dmg_bonus_1");
	if (tmp)
	{
		it_temp[in].dmg_bonus[I_I] = atoi(tmp);
	}
	else
	{
		printf("dmg_bonus_1 not specified.\n");
		return;
	}
	
	tmp = find_val(head, "dmg_bonus_2");
	if (tmp)
	{
		it_temp[in].dmg_bonus[I_A] = atoi(tmp);
	}
	else
	{
		printf("dmg_bonus_2 not specified.\n");
		return;
	}
	
	tmp = find_val(head, "dmg_reduc_1");
	if (tmp)
	{
		it_temp[in].dmg_reduction[I_I] = atoi(tmp);
	}
	else
	{
		printf("dmg_reduc_1 not specified.\n");
		return;
	}
	
	tmp = find_val(head, "dmg_reduc_2");
	if (tmp)
	{
		it_temp[in].dmg_reduction[I_A] = atoi(tmp);
	}
	else
	{
		printf("dmg_reduc_2 not specified.\n");
		return;
	}

	tmp = find_val(head, "max_age_1");
	if (tmp)
	{
		it_temp[in].max_age[I_I] = atoi(tmp);
	}
	else
	{
		printf("MAX_AGE_1 not specified.\n");
		return;
	}

	tmp = find_val(head, "max_age_2");
	if (tmp)
	{
		it_temp[in].max_age[I_A] = atoi(tmp);
	}
	else
	{
		printf("MAX_AGE_2 not specified.\n");
		return;
	}

	tmp = find_val(head, "max_damage");
	if (tmp)
	{
		it_temp[in].max_damage = atoi(tmp);
	}
	else
	{
		printf("MAX_DAMAGE not specified.\n");
		return;
	}

	tmp = find_val(head, "duration");
	if (tmp)
	{
		it_temp[in].duration = atoi(tmp);
	}
	else
	{
		printf("DURATION not specified.\n");
		return;
	}

	tmp = find_val(head, "cost");
	if (tmp)
	{
		it_temp[in].cost = atoi(tmp);
	}
	else
	{
		printf("COST not specified.\n");
		return;
	}

	tmp = find_val(head, "power");
	if (tmp)
	{
		it_temp[in].power = atoi(tmp);
	}
	else
	{
		printf("POWER not specified.\n");
		return;
	}

	tmp = find_val(head, "spr_ovr");
	if (tmp)
	{
		it_temp[in].sprite_override = atoi(tmp);
	}
	else
	{
		printf("SPR_OVR not specified.\n");
		return;
	}

	tmp = find_val(head, "min_rank");
	if (tmp)
	{
		it_temp[in].min_rank = atoi(tmp);
	}
	else
	{
		printf("MIN_RANK not specified.\n");
		return;
	}

	tmp = find_val(head, "sprite_1");
	if (tmp)
	{
		it_temp[in].sprite[I_I] = atoi(tmp);
	}
	else
	{
		printf("SPRITE_1 not specified.\n");
		return;
	}

	tmp = find_val(head, "sprite_2");
	if (tmp)
	{
		it_temp[in].sprite[I_A] = atoi(tmp);
	}
	else
	{
		printf("SPRITE_2 not specified.\n");
		return;
	}

	tmp = find_val(head, "status_1");
	if (tmp)
	{
		it_temp[in].status[I_I] = atoi(tmp);
	}
	else
	{
		printf("STATUS_1 not specified.\n");
		return;
	}

	tmp = find_val(head, "status_2");
	if (tmp)
	{
		it_temp[in].status[I_A] = atoi(tmp);
	}
	else
	{
		printf("STATUS_2 not specified.\n");
		return;
	}

	globs->reset_item = in;

	printf("Done.\n");
}


void list_characters_template(LIST *head)    // excludes grolms, gargs, icegargs. decides by sprite-nr
{
	int n;
	phtml_home();
	phtml_tab_open();

	for (n = 0; n<MAXTCHARS; n++)
	{
		if (ch_temp[n].used==USE_EMPTY)
		{
			continue;
		}
		if (ch_temp[n].sprite==12240 || ch_temp[n].sprite==18384 || ch_temp[n].sprite==21456)
		{
			continue;
		}
		phtml_row_ctemplate(n);
	}

	phtml_tab_close();
	phtml_home();
}

void list_characters2_template(LIST *head)		// listing grolms,gargs,icegargs only, desides by sprite-nr
{
	int n;
	phtml_home();
	phtml_tab_open();

	for (n = 0; n<MAXTCHARS; n++)
	{
		if (ch_temp[n].used==USE_EMPTY)
		{
			continue;
		}
		if (!(ch_temp[n].sprite==12240 || ch_temp[n].sprite==18384 || ch_temp[n].sprite==21456))
		{
			continue;
		}
		phtml_row_ctemplate(n);
	}

	phtml_tab_close();
	phtml_home();
}

void list_characters_template_pugilism(LIST *head)    // has pugilism greater than 0
{
	int n;
	phtml_home();
	phtml_tab_open();

	for (n = 0; n<MAXTCHARS; n++)
	{
		if (ch_temp[n].used==USE_EMPTY)
		{
			continue;
		}
		if (ch_temp[n].skill[1][0] < 1)
		{
			continue;
		}
		phtml_row_ctemplate(n);
	}

	phtml_tab_close();
	phtml_home();
}

void list_characters_template_align(LIST *head, int flag)    // excludes grolms, gargs, icegargs. decides by sprite-nr
{
	int n;
	phtml_home();
	phtml_tab_open();

	for (n = 0; n<MAXTCHARS; n++)
	{
		if (ch_temp[n].used==USE_EMPTY)
		{
			continue;
		}
		if (ch_temp[n].sprite==12240 || ch_temp[n].sprite==18384 || ch_temp[n].sprite==21456)
		{
			continue;
		}
		if ((flag && ch_temp[n].alignment < 0) || (!flag && ch_temp[n].alignment >= 0))
		{
			continue;
		}
		phtml_row_ctemplate(n);
	}

	phtml_tab_close();
	phtml_home();
}

void list_characters_template_non_monster(LIST *head)    // excludes grolms, gargs, icegargs. decides by sprite-nr
{
	int n;
	phtml_home();
	phtml_tab_open();

	for (n = 0; n<MAXTCHARS; n++)
	{
		if (ch_temp[n].used==USE_EMPTY)
		{
			continue;
		}
		if (ch_temp[n].kindred & KIN_MONSTER)
		{
			continue;
		}
		phtml_row_ctemplate(n);
	}

	phtml_tab_close();
	phtml_home();
}



void list_named_characters_template(LIST *head)    // excludes grolms, gargs, icegargs. decides by sprite-nr
{
	int n;
	int i;
	phtml_tab_open();

	for (n = 0; n<MAXTCHARS; n++)
	{
		if (ch_temp[n].used==USE_EMPTY)
		{
			continue;
		}
		if (ch_temp[n].sprite==12240 ||
		    ch_temp[n].sprite==18384 ||
		    ch_temp[n].sprite==21456 ||
		    ch_temp[n].sprite==14288 ||
		    ch_temp[n].sprite==31696)
		{
			continue;
		}
		phtml_row_ctemplate(n);
	}

	phtml_tab_close();
}

void list_new_characters_template(LIST *head, int flag)		 // listing characters with high IDs
{
	int n, m=1;
	
	if (flag==1)
	{
		for (n = 1; n<MAXTCHARS; n++)
		{
			if (ch_temp[n].used==USE_EMPTY) break;
		}
		m = max(1, n - 1000);
	}
	phtml_tab_open();
	for (n = m; n<MAXTCHARS; n++)
	{
		if (ch_temp[n].used==USE_EMPTY)
		{
			continue;
		}
		phtml_row_ctemplate(n);
	}
	phtml_tab_close();
}

int startsWith(const char *a, const char *b)
{
   if (strncmp(a, b, strlen(b)) == 0) return 1;
   return 0;
}

void list_objects(LIST *head, int flag)
{
	int n, m=1;
	if (flag==4)
	{
		for (n = 1; n<MAXTITEM; n++)
		{
			if (it_temp[n].used==USE_EMPTY) break;
		}
		m = max(1, n - 1000);
	}
	phtml_home();
	printf("<center><table>\n");
	for (n = m; n<MAXTITEM; n++)
	{
		if (it_temp[n].used==USE_EMPTY) continue;
		if (flag==5 && !(it_temp[n].flags & IF_MAGIC)) continue;
		if (flag==1 && !(it_temp[n].flags & IF_SELLABLE)) continue;
		if (flag==2 && !(it_temp[n].flags & IF_JEWELERY)) continue;
		if (flag==3 && !(it_temp[n].duration)) continue;
		if (flag==8)
		{
			if (!(startsWith(it_temp[n].reference, "a ") || startsWith(it_temp[n].reference, "an ") || startsWith(it_temp[n].reference, "the "))) continue;
		}
		printf("<tr><td style=\"text-align:right;\">%d:&ensp;</td><td><a href=/cgi-imp/acct.cgi?step=23&in=%d>%30.30s</a></td>\n"
				"<td><font size=2>p:%d.%02dG</font></td><td><font size=2>t:%dm%02ds(%dt)</font></td><td><font size=2>d0:%d</font></td><td><font size=2>d1:%d</font></td><td><font size=2>d2:%d</font></td><td><font size=2>d3: %d</font></td><td><font size=2>d4: %d</font></td>\n"
				"<td><a href=/cgi-imp/acct.cgi?step=25&in=%d>Copy</a></td><td><a href=/cgi-imp/acct.cgi?step=22&in=%d>Delete</a></td><td>&nbsp;:%d</td></tr>\n",
				n, n, it_temp[n].name,
				it_temp[n].value / 100, it_temp[n].value % 100, 
				it_temp[n].duration!=-1?((it_temp[n].duration / TICKS) / 60):0, 
				it_temp[n].duration!=-1?((it_temp[n].duration / TICKS) % 60):0, 
				it_temp[n].duration,
				it_temp[n].data[0], it_temp[n].data[1], it_temp[n].data[2], it_temp[n].data[3], it_temp[n].data[4],
				n, n, n);
	}
	printf("</table></center>\n");
	phtml_home();
}

void list_items(LIST *head)
{
	int n;
	phtml_home();

	printf("<center><table>\n");

	for (n = 1; n<MAXITEM; n++)
	{
		if (it[n].used==USE_EMPTY)
		{
			continue;
		}
		if(it[n].x==0 && it[n].y==0 && it[n].carried==0)
		{
			continue;
		}

//				  if (it_temp[n].driver!=23) continue;
//		if (!(it_temp[n].flags&IF_TAKE)) continue;

		printf("<tr><td>%d:</td><td><a href=/cgi-imp/acct.cgi?step=28&in=%d>%30.30s</a></td>\n"
				"<td><font size=2>pr: %dG, %dS</font></td><td><font size=2>da0: %d</font></td><td><font size=2>da1: %d</font></td><td><font size=2>da2: %d</font></td><td><font size=2>da3: %d</font></td><td><font size=2>da4: %d</font></td>\n"
				/*"<td><a href=/cgi-imp/acct.cgi?step=25&in=%d>Copy</a></td><td><a href=/cgi-imp/acct.cgi?step=22&in=%d>Delete</a></td></tr>\n"*/,
				n, n, it[n].name,
				it[n].value / 100, it[n].value % 100, it[n].data[0], it[n].data[1], it[n].data[2], it[n].data[3], it[n].data[4]/*,
																		     n, n*/);
	}

	printf("</table></center>\n");
	phtml_home();

}

void list_object_drivers(LIST *head)
{
	int n, nd;
	int ndrivers = 0;
	int found;

	// Find out how many drivers there are
	for (n = 1; n<MAXTITEM; n++)
	{
		if (it_temp[n].driver > ndrivers)
		{
			ndrivers = it_temp[n].driver;
		}
	}
	printf("<ul compact>\n");
	for (nd = 1; nd<=ndrivers; nd++)
	{
		found = 0;
		for (n = 1; n<MAXTITEM; n++)
		{
			if (it_temp[n].used==USE_EMPTY)
			{
				continue;
			}
			if (it_temp[n].driver == nd)
			{
				if (!found)
				{
					printf("<li>Driver #%d:"
							"<dir compact>", nd);
					found = 1;
				}
				printf("<li>"
						"<a href=/cgi-imp/acct.cgi?step=23&in=%d>%d:</a>"
						"&nbsp;&nbsp;&nbsp;%s"
						"\n</li>\n", n, n, it_temp[n].name);
			}
		}
		if (found)
		{
			printf("</dir></li>\n");
		}
	}
	printf("\n</ul>\n");
}

int main(int argc, char *args[])
{
	int step = 0;
	int n __attribute__ ((unused));
	char *tmp;
	LIST *head;
	head = is_form_empty() ? NULL : cgi_input_parse();

	int result = chdir("/home/merc");

	if(result != 0)
	{
		printf("Unable to find /home/merc");
		exit(1);
	}

	printf("Content-Type: text/html\n\n");
	printf("<html><head><title>World Builder</title><META HTTP-EQUIV=\"PRAGMA\" CONTENT=\"NO-CACHE\"></head>\n");
	#if defined(SUSE)
	printf("<BODY TEXT=#D7D700 BGCOLOR=#264A9F LINK=#FFFFBB VLINK=#CCCC00 ALINK=#FFFF9D\n");
	#else
	printf("<BODY TEXT=#D7D700 BGCOLOR=#264A9F LINK=#FFFFBB VLINK=#CCCC00 ALINK=#FFFF9D background=/gfx/back4.gif>\n");
	#endif
	printf("<center>\n");
	printf("<table width=\"100%%\"><tr>\n");
	printf("<td align=center>\n");
	printf("<a href=\"https://github.com/dylanyaga/openMerc\"><img src=\"/gfx/logo.gif\" width=100 height=60 border=0></a>\n");
	printf("</td><td align=center>\n");
	printf("<h2>Server Info</h2>\n");
	printf("</td><td align=center>\n");
	printf("<a href=\"https://github.com/dylanyaga/openMerc\"><img src=\"/gfx/logo.gif\" width=100 height=60 border=0></a>");
	printf("</td></tr></table>\n");
	printf("<hr width=80%% color=\"#808000\"><br>\n");
	printf("<table width=\"95%%\"><tr><td>\n");

	web_load();

	#ifdef RESET_CHAR_KLUDGE
	n = atoi(args[1]);
	globs->reset_char = n;
	web_unload();
	exit(0);
	#endif

	if (head)
	{
		tmp = find_val(head, "step");
		if (tmp)
		{
			step = atoi(tmp);
		}
	}

	ch_temp[1].used = USE_ACTIVE;
	strcpy(ch_temp[1].name, "Blank Template");

	it_temp[1].used = USE_ACTIVE;
	strcpy(it_temp[1].name, "Blank Template");

	/* for (n=1; n<MAXTCHARS; n++) {
			 if (ch_temp[n].used==USE_EMPTY) continue;
	   } */

/*      for (n=1; n<MAXTITEM; n++) {
				  if (it_temp[n].used==USE_EMPTY) continue;
				  it_temp[n].value/=5;
		 } */

	switch(step)
	{
	case 11:
		list_characters_template(head);
		break;
	case 12:
		delete_character_template(head);
		break;
	case 13:
		view_character_template(head);
		break;
	case 14:
		update_character_template(head);
		break;
	case 15:
		copy_character_template(head);
		break;
	case 16:
		list_characters_template_align(head, 1);
		break;
	case 17:
		list_named_characters_template(head);
		break;
	case 18:
		list_characters_template_non_monster(head);
		break;
	case 19:
		list_all_player_characters();
		break;
	case 20:
		view_character_player(head);
		break;
	case 21:
		list_objects(head, 0);
		break;
	case 22:
		delete_object(head);
		break;
	case 23:
		view_object(head);
		break;
	case 24:
		update_object(head);
		break;
	case 25:
		copy_object(head);
		break;
	case 26:
		update_character_player(head);
		break;
	case 27:
		list_items(head);
		break;
	case 28:
		view_item(head);
		break;
	case 29:
		save_character_player(head);
		break;
	case 30:
		list_objects(head, 1);
		break;
	case 31:
		list_object_drivers(head);
		break;
	case 32:
		list_all_player_characters_by_class();
		break;
	case 33:
		list_objects(head, 2);
		break;
	case 34:
		list_objects(head, 3);
		break;
	case 35:
		list_characters_template_align(head, 0);
		break;
	case 36:
		list_all_player_characters_by_pandium();
		break;
	case 37:
		list_objects(head, 4);
		break;
	case 38:
		list_objects(head, 5);
		break;
	case 41:
		list_characters2_template(head);
		break;
	case 51:
		list_new_characters_template(head, 0);
		break;
	case 52:
		list_new_characters_template(head, 1);
		break;
	default:
		printf("</td></tr></table>\n");
		printf("<table width=\"80%%\"><tr><td>\n");
		printf("Player Web Editing (Experimental)<br>\n");
		printf("<a href=/cgi-imp/acct.cgi?step=19>Player Characters by ID</a><br>\n");
		printf("<a href=/cgi-imp/acct.cgi?step=32>Player Characters by Class</a><br><br>\n");
		printf("<a href=/cgi-imp/acct.cgi?step=36>Player Characters by Pandium Clears</a><br><br>\n");
		printf("Together those lists include all character-templates<br>\n");
		printf("<a href=/cgi-imp/acct.cgi?step=11>Characters (without Grolms, Gargoyles, Icegargs)</a><br>\n");
		printf("<a href=/cgi-imp/acct.cgi?step=16>Characters (only with Positive Alignment) </a><br>\n");
		printf("<a href=/cgi-imp/acct.cgi?step=35>Characters (only with Negative Alignment) </a><br>\n");
		printf("<a href=/cgi-imp/acct.cgi?step=17>Characters (only Named) </a><br>\n");
		printf("<a href=/cgi-imp/acct.cgi?step=18>Characters (Non Monster) </a><br>\n");
		printf("<a href=/cgi-imp/acct.cgi?step=41>Characters (only Grolms, Gargoyles, Icegargs) </a><br><br>\n");
		printf("This list includes only characters with high IDs for fast access<br>\n");
		printf("<a href=/cgi-imp/acct.cgi?step=51>Character Templates (All)</a><br><br>\n");
		printf("<a href=/cgi-imp/acct.cgi?step=52>Character Templates (New)</a><br><br>\n");
		printf("<a href=/cgi-imp/acct.cgi?step=21>Object Templates (All)</a><br>\n");
		printf("<a href=/cgi-imp/acct.cgi?step=37>Object Templates (New)</a><br>\n");
		printf("<a href=/cgi-imp/acct.cgi?step=30>Object Templates (Sellable)</a><br>\n");
		printf("<a href=/cgi-imp/acct.cgi?step=33>Object Templates (Jewelery)</a><br>\n");
		printf("<a href=/cgi-imp/acct.cgi?step=38>Object Templates (Magic)</a><br>\n");
		printf("<a href=/cgi-imp/acct.cgi?step=34>Object Templates (Duration)</a><br>\n");
		printf("<a href=/cgi-imp/acct.cgi?step=31>Object Driver List</a><br><br>\n");
		printf("Show All Items<br>\n");
		printf("<a href=/cgi-imp/acct.cgi?step=27>Item List</a><br><br>\n");
		printf("Ab Aeterno's super cool map editor<br>\n");
		printf("<a href=/cgi-imp/mapper.cgi>Online Map Editor</a><br>");
		break;
	}

	web_unload();

	printf("</td></tr></table>\n");

	printf("<hr width=80%% color=\"#808000\"><br>\n");
	printf("<a href=/cgi-imp/acct.cgi>Back to main page</a>\n");
	printf("<hr width=80%% color=\"#808000\"><br>\n");
	printf("<font size=-1>All material on this server is based on the Mercenaries of Astonia engine by Daniel Brockhaus.</font></center>\n");
	printf("</body></html>\n");


	return 0;
}

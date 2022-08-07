/*************************************************************************

   This file is part of 'Mercenaries of Astonia v2'
   Copyright (c) 1997-2001 Daniel Brockhaus (joker@astonia.com)
   All rights reserved.

 **************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <fcntl.h>
#include <unistd.h>

#include "server.h"
#include "driver.h"

static char *mkp(void)
{
	static char buf[40];
	static char *syl1[] = {
		"thi", "ar",  "an",   "un",  "iss", "ish", "urs",  "ur",
		"ent", "esh", "ash",  "jey", "jay", "joh", "jan",  "jess",
		"dur", "lon", "lan",  "len", "lun", "leg", "less", "lar",
		"so",  "lur", "gar",  "cry", "au",  "dau", "dei",  "das", 
		"zir", "zil", "sol",  "luc", "nis", "bus", "bat",  "bul", 
		"mid", "err", "doo",  "do",  "al",  "ea",  "jac",  "ta",
		"bi",  "vae", "rif",  "tol", "nim", "ru",  "li",   "rid", 
		"fro", "sam", "beut", "bil", "ga",  "gu",  "gi",  "guy", 
		"nee", "ara", "rho",  "dan", "va",  "lan", "lay", "larn",
		"cec", "cic", "cac",  "cuc", "ix",  "vea", "cya", "nya", 
		"hie", "bo",  "ni",   "do",  "sar", "phe", "ho",  "pho", 
		"cos", "sin", "tan",  "mul", "har", "gur", "tar", "mar",
		"a",   "e",   "i",    "o",   "u",   "je",  "ho",  "if",
		"jai", "coy", "ya",   "pa",  "pul", "pil", "poh", "ruf",
		"rez", "rel", "rar",  "dom", "rom", "tom", "mom", "dad",
		"ar",  "ur",  "ir",   "er",  "yr",  "li",  "la",  "lu", "lo"
	};
	static char *syl2[] = {
		"tar", "tur", "kar", "kur", "kan", "tan", "gar", "gur", 
		"run", "sun", "bun", "pun", "won", "gur", "bar", "bas",
		"ruu", "tsu", "sue", "blu", "sno", "th",  "sh",  "ck"
	};
	static char *syl3[] = {
		"a", "e", "i", "o", "u"
	};

	int n;

	buf[0] = 0;

	n = random() % max(1,(sizeof(syl1) / sizeof(char *)));
	strcat(buf, syl1[n]);
	buf[0] = toupper(buf[0]);

	n = random() % max(1,(sizeof(syl2) / sizeof(char *)));
	strcat(buf, syl2[n]);

	if (random() % 2)
	{
		return( buf);
	}

	n = random() % max(1,(sizeof(syl3) / sizeof(char *)));
	strcat(buf, syl3[n]);

	return(buf);
}

#define MAXFREEBUFF 32
#define MAXFREEITEM 32
static int free_buff_list[MAXFREEBUFF];
static int free_item_list[MAXFREEITEM];

void god_init_freelist(void)
{
	int n, m;

	bzero(free_buff_list, sizeof(free_buff_list));

	for (m = 0, n = 1; n<MAXBUFF; n++)
	{
		if (it[n].used==USE_EMPTY)
		{
			free_buff_list[m++] = n;
			if (m>=MAXFREEBUFF)
			{
				break;
			}
		}
	}
	
	bzero(free_item_list, sizeof(free_item_list));

	for (m = 0, n = 1; n<MAXITEM; n++)
	{
		if (it[n].used==USE_EMPTY)
		{
			free_item_list[m++] = n;
			if (m>=MAXFREEITEM)
			{
				break;
			}
		}
	}
}


int get_free_buff(void)
{
	int n, in, m;

	for (n = 0; n<MAXFREEBUFF; n++)
	{
		if ((in = free_buff_list[n]) && bu[in].used==USE_EMPTY)
		{
			break;
		}
	}
	if (n<MAXFREEBUFF)
	{
		free_buff_list[n] = 0;
		return(in);
	}

	for (m = in = 0, n = 1; n<MAXBUFF; n++)
	{
		if (bu[n].used==USE_EMPTY)
		{
			in = free_buff_list[m++] = n;
			if (m>=MAXFREEBUFF)
			{
				break;
			}
		}
	}

	return(in);
}

int god_create_buff(void)
{
	int n, m;
	unsigned long long prof;

	prof = prof_start();

	n = get_free_buff();
	if (!n)
	{
		xlog("god_create_buff (svr_god.c): MAXBUFF reached");
		prof_stop(46, prof);
		return 0;
	}

	bu[n] = it_temp[1];
	bu[n].temp = 1;

	prof_stop(46, prof);

	return(n);
}

int get_free_item(void)
{
	int n, in, m;

	for (n = 0; n<MAXFREEITEM; n++)
	{
		if ((in = free_item_list[n]) && it[in].used==USE_EMPTY)
		{
			break;
		}
	}
	if (n<MAXFREEITEM)
	{
		free_item_list[n] = 0;
		return( in);
	}

	for (m = in = 0, n = 1; n<MAXITEM; n++)
	{
		if (it[n].used==USE_EMPTY)
		{
			in = free_item_list[m++] = n;
			if (m>=MAXFREEITEM)
			{
				break;
			}
		}
	}

	return(in);
}

int god_create_item(int temp)
{
	int n, m;
	unsigned long long prof;

	if (!IS_SANEITEMPLATE(temp))
	{
		return 0;
	}

	prof = prof_start();

	if (it_temp[temp].used == USE_EMPTY)
	{
		xlog("god_create_item(): unused template.");
		prof_stop(23, prof);
		return 0;
	}

	n = get_free_item();
	if (!n)
	{
		xlog("god_create_item (svr_god.c): MAXITEM reached");
		prof_stop(23, prof);
		return 0;
	}

	it[n] = it_temp[temp];
	it[n].temp = temp;
	
	// Special case to set fresh stackable items to at least 1 stack
	if (it[n].flags & IF_STACKABLE)
	{
		it[n].stack = 1;
	}
	
	// Special case to set stacks for spell scrolls
	if (it[n].driver==48)
	{
		it[n].stack = it[n].data[2];
	}

	prof_stop(23, prof);

	return(n);
}

int god_create_char(int temp, int withitems)
{
	int n, m, tmp, flag = 0;

	for (n = 1; n<MAXCHARS; n++)
	{
		if (ch[n].used==USE_EMPTY)
		{
			break;
		}
	}

	if (n==MAXCHARS)
	{
		xlog("god_create_char (svr_god.c): MAXCHARS reached!");
		return 0;
	}

	ch[n] = ch_temp[temp];
	ch[n].pass1 = RANDOM(0x3fffffff);
	ch[n].pass2 = RANDOM(0x3fffffff);
	ch[n].temp  = temp;

	while (1)
	{
		strcpy(ch[n].name, mkp());
		for (m = 1; m<MAXCHARS; m++)
		{
			if (m==n)
			{
				continue;
			}
			if (!strcmp(ch[n].name, ch[m].name))
			{
				break;
			}
		}
		if (m==MAXCHARS)
		{
			break;
		}
	}
	strcpy(ch[n].reference, ch[n].name);

	sprintf(ch[n].description, "%s is a %s%s%s%s%s. %s%s%s looks somewhat nondescript.",
		ch[n].name,
		IS_TEMPLAR(n) ? "Templar" : "",
		IS_MERCENARY(n) ? "Mercenary" : "",
		IS_HARAKIM(n) ? "Harakim" : "",
		IS_SEYAN_DU(n) ? "Seyan'Du" : "",
		!(IS_TEMPLAR(n) || IS_MERCENARY(n) || IS_HARAKIM(n) || IS_SEYAN_DU(n)) ? "Monster" : "",
		!IS_FEMALE(n) ? "He" : "",
		IS_FEMALE(n) ? "She" : "",
		IS_MONSTER(n) ? "It" : "");

	for (m = 0; m<100; m++)
	{
		ch[n].data[m] = 0;
	}
	ch[n].attack_cn = 0;
	ch[n].skill_nr  = 0;
	ch[n].goto_x = 0;
	ch[n].use_nr = 0;
	ch[n].misc_action = 0;
	ch[n].stunned = 0;
	ch[n].taunted = 0;
	ch[n].retry = 0;
	ch[n].dir = DX_DOWN;

	for (m = 0; m<40; m++)
	{
		if ((tmp = ch[n].item[m])!=0)
		{
			if (withitems)
			{
				tmp = god_create_item(tmp);
				if (!tmp)
				{
					flag = 1;
				}
				it[tmp].carried = n;
			}
			else
			{
				tmp = 0;
			}
			ch[n].item[m] = tmp;
		}
	}

	for (m = 0; m<20; m++)
	{
		if ((tmp = ch[n].worn[m])!=0)
		{
			if (withitems)
			{
				tmp = god_create_item(tmp);
				if (!tmp)
				{
					flag = 1;
				}
				it[tmp].carried = n;
			}
			else
			{
				tmp = 0;
			}
			ch[n].worn[m] = tmp;
		}
	}
	
	for (m = 0; m<12; m++)
	{
		if ((tmp = ch[n].alt_worn[m])!=0)
		{
			if (withitems)
			{
				tmp = god_create_item(tmp);
				if (!tmp)
				{
					flag = 1;
				}
				it[tmp].carried = n;
			}
			else
			{
				tmp = 0;
			}
			ch[n].alt_worn[m] = tmp;
		}
	}

	for (m = 0; m<MAXBUFFS; m++)
	{
		if (ch[n].spell[m]!=0)
		{
			ch[n].spell[m] = 0;
		}
	}

	if ((tmp = ch[n].citem)!=0)
	{
		if (withitems)
		{
			tmp = god_create_item(tmp);
			if (!tmp)
			{
				flag = 1;
			}
			it[tmp].carried = n;
		}
		else
		{
			tmp = 0;
		}
		ch[n].citem = tmp;
	}

	if (flag)
	{
		god_destroy_items(n);
		ch[n].used = USE_EMPTY;
		return 0;
	}

	ch[n].a_end  = 1000000;
	ch[n].a_hp   = 1000000;
	ch[n].a_mana = 1000000;

	do_update_char(n);

	return(n);
}

int god_change_pass(int cn, int co, char *pass)
{
	int hash, n;

	if (co<1 || co>=MAXCHARS)
	{
		do_char_log(cn, 0, "Character out of bounds.\n");
		return 0;
	}
	if (!pass || !*pass)
	{
		if (cn!=co)
		{
			do_char_log(cn, 1, "Removed %s's password.\n", ch[co].name);
		}
		else
		{
			do_char_log(cn, 1, "Removed password.\n");
		}
		chlog(cn, "Removed password");
		ch[co].flags &= ~CF_PASSWD;
		bzero(ch[co].passwd, sizeof(ch[co].passwd));
		return 1;
	}
	memcpy(ch[co].passwd, pass, 15);
	ch[co].passwd[15] = 0;

	if (cn!=co)
	{
		do_char_log(cn, 1, "Set %s's password to \"%s\".\n", ch[co].name, ch[co].passwd);
	}
	else
	{
		do_char_log(cn, 1, "Set your password to \"%s\".\n", ch[co].passwd);
	}

	for (n = hash = 0; n<15 && ch[co].passwd[n]; n++)
	{
		hash ^= (ch[co].passwd[n] << (n * 2));
	}

	chlog(cn, "Set %s's (%d) password to hash %u", ch[co].name, co, hash);

	ch[co].flags |= CF_PASSWD;

	return 1;
}

int god_drop_item(int nr, int x, int y)
{
	int m;

	if (!SANEXY(x, y))
	{
		return 0;
	}
	m = XY2M(x, y);
	if (map[m].ch || map[m].to_ch || map[m].it || (map[m].flags & (MF_MOVEBLOCK | MF_DEATHTRAP)) || map[m].fsprite)
	{
		return 0;
	}

	map[m].it = nr;

	it[nr].x = (short)x;
	it[nr].y = (short)y;
	it[nr].carried = 0;

	if (it[nr].active)
	{
		if (it[nr].light[1])
		{
			do_add_light(x, y, it[nr].light[1]);
		}
	}
	else
	{
		if (it[nr].light[0])
		{
			do_add_light(x, y, it[nr].light[0]);
		}
	}

	return 1;
}

int god_remove_item(int cn, int nr)
{
	int in;

	in = ch[cn].worn[nr];
	if (!in)
	{
		return 0;
	}

	it[in].x = 0;
	it[in].y = 0;
	it[in].carried = 0;

	ch[cn].worn[nr] = 0;

	do_update_char(cn);

	return(in);
}

int god_drop_item_fuzzy(int nr, int x, int y)
{
	if (god_drop_item(nr, x, y))
	{
		return 1;
	}

	if (can_go(x, y, x + 1, y) && god_drop_item(nr, x + 1, y))
	{
		return 1;
	}
	if (can_go(x, y, x - 1, y) && god_drop_item(nr, x - 1, y))
	{
		return 1;
	}
	if (can_go(x, y, x, y + 1) && god_drop_item(nr, x, y + 1))
	{
		return 1;
	}
	if (can_go(x, y, x, y - 1) && god_drop_item(nr, x, y - 1))
	{
		return 1;
	}

	if (can_go(x, y, x + 1, y + 1) && god_drop_item(nr, x + 1, y + 1))
	{
		return 1;
	}
	if (can_go(x, y, x + 1, y - 1) && god_drop_item(nr, x + 1, y - 1))
	{
		return 1;
	}
	if (can_go(x, y, x - 1, y + 1) && god_drop_item(nr, x - 1, y + 1))
	{
		return 1;
	}
	if (can_go(x, y, x - 1, y - 1) && god_drop_item(nr, x - 1, y - 1))
	{
		return 1;
	}

	if (can_go(x, y, x + 2, y - 2) && god_drop_item(nr, x + 2, y - 2))
	{
		return 1;
	}
	if (can_go(x, y, x + 2, y - 1) && god_drop_item(nr, x + 2, y - 1))
	{
		return 1;
	}
	if (can_go(x, y, x + 2, y + 0) && god_drop_item(nr, x + 2, y + 0))
	{
		return 1;
	}
	if (can_go(x, y, x + 2, y + 1) && god_drop_item(nr, x + 2, y + 1))
	{
		return 1;
	}
	if (can_go(x, y, x + 2, y + 2) && god_drop_item(nr, x + 2, y + 2))
	{
		return 1;
	}
	if (can_go(x, y, x - 2, y - 2) && god_drop_item(nr, x - 2, y - 2))
	{
		return 1;
	}
	if (can_go(x, y, x - 2, y - 1) && god_drop_item(nr, x - 2, y - 1))
	{
		return 1;
	}
	if (can_go(x, y, x - 2, y + 0) && god_drop_item(nr, x - 2, y + 0))
	{
		return 1;
	}
	if (can_go(x, y, x - 2, y + 1) && god_drop_item(nr, x - 2, y + 1))
	{
		return 1;
	}
	if (can_go(x, y, x - 2, y + 2) && god_drop_item(nr, x - 2, y + 2))
	{
		return 1;
	}
	if (can_go(x, y, x - 1, y + 2) && god_drop_item(nr, x - 1, y + 2))
	{
		return 1;
	}
	if (can_go(x, y, x + 0, y + 2) && god_drop_item(nr, x + 0, y + 2))
	{
		return 1;
	}
	if (can_go(x, y, x + 1, y + 2) && god_drop_item(nr, x + 1, y + 2))
	{
		return 1;
	}
	if (can_go(x, y, x - 1, y - 2) && god_drop_item(nr, x - 1, y - 2))
	{
		return 1;
	}
	if (can_go(x, y, x + 0, y - 2) && god_drop_item(nr, x + 0, y - 2))
	{
		return 1;
	}
	if (can_go(x, y, x + 1, y - 2) && god_drop_item(nr, x + 1, y - 2))
	{
		return 1;
	}

	return 0;
}

int god_drop_char(int cn, int x, int y)
{
	int m, in;

	if (x<1 || x>MAPX - 2 || y<1 || y>MAPY - 2)
	{
		return 0;
	}

	m = x + y * MAPX;
	if (map[m].ch || map[m].to_ch || ((in = map[m].it)!=0 && (it[in].flags & IF_MOVEBLOCK)) || (map[m].flags & MF_MOVEBLOCK) ||
	    (map[m].flags & MF_TAVERN) || (map[m].flags & MF_DEATHTRAP))
	{
		return 0;
	}

	ch[cn].x = (unsigned short)x;
	ch[cn].y = (unsigned short)y;
	ch[cn].tox = (unsigned short)x;
	ch[cn].toy = (unsigned short)y;

	plr_map_set(cn);

	return 1;
}

int god_drop_char_fuzzy(int nr, int x, int y)
{
	if (god_drop_char(nr, x, y))
	{
		return 1;
	}

	if (can_go(x, y, x + 1, y) && god_drop_char(nr, x + 1, y))
	{
		return 1;
	}
	if (can_go(x, y, x - 1, y) && god_drop_char(nr, x - 1, y))
	{
		return 1;
	}
	if (can_go(x, y, x, y + 1) && god_drop_char(nr, x, y + 1))
	{
		return 1;
	}
	if (can_go(x, y, x, y - 1) && god_drop_char(nr, x, y - 1))
	{
		return 1;
	}

	if (can_go(x, y, x + 1, y + 1) && god_drop_char(nr, x + 1, y + 1))
	{
		return 1;
	}
	if (can_go(x, y, x + 1, y - 1) && god_drop_char(nr, x + 1, y - 1))
	{
		return 1;
	}
	if (can_go(x, y, x - 1, y + 1) && god_drop_char(nr, x - 1, y + 1))
	{
		return 1;
	}
	if (can_go(x, y, x - 1, y - 1) && god_drop_char(nr, x - 1, y - 1))
	{
		return 1;
	}

	if (can_go(x, y, x + 2, y - 2) && god_drop_char(nr, x + 2, y - 2))
	{
		return 1;
	}
	if (can_go(x, y, x + 2, y - 1) && god_drop_char(nr, x + 2, y - 1))
	{
		return 1;
	}
	if (can_go(x, y, x + 2, y + 0) && god_drop_char(nr, x + 2, y + 0))
	{
		return 1;
	}
	if (can_go(x, y, x + 2, y + 1) && god_drop_char(nr, x + 2, y + 1))
	{
		return 1;
	}
	if (can_go(x, y, x + 2, y + 2) && god_drop_char(nr, x + 2, y + 2))
	{
		return 1;
	}
	if (can_go(x, y, x - 2, y - 2) && god_drop_char(nr, x - 2, y - 2))
	{
		return 1;
	}
	if (can_go(x, y, x - 2, y - 1) && god_drop_char(nr, x - 2, y - 1))
	{
		return 1;
	}
	if (can_go(x, y, x - 2, y + 0) && god_drop_char(nr, x - 2, y + 0))
	{
		return 1;
	}
	if (can_go(x, y, x - 2, y + 1) && god_drop_char(nr, x - 2, y + 1))
	{
		return 1;
	}
	if (can_go(x, y, x - 2, y + 2) && god_drop_char(nr, x - 2, y + 2))
	{
		return 1;
	}
	if (can_go(x, y, x - 1, y + 2) && god_drop_char(nr, x - 1, y + 2))
	{
		return 1;
	}
	if (can_go(x, y, x + 0, y + 2) && god_drop_char(nr, x + 0, y + 2))
	{
		return 1;
	}
	if (can_go(x, y, x + 1, y + 2) && god_drop_char(nr, x + 1, y + 2))
	{
		return 1;
	}
	if (can_go(x, y, x - 1, y - 2) && god_drop_char(nr, x - 1, y - 2))
	{
		return 1;
	}
	if (can_go(x, y, x + 0, y - 2) && god_drop_char(nr, x + 0, y - 2))
	{
		return 1;
	}
	if (can_go(x, y, x + 1, y - 2) && god_drop_char(nr, x + 1, y - 2))
	{
		return 1;
	}

	return 0;
}

int god_drop_char_fuzzy_large(int nr, int x, int y, int xs, int ys)
{
	if (can_go(xs, ys, x, y) && god_drop_char(nr, x, y))
	{
		return 1;
	}

	if (can_go(xs, ys, x + 1, y) && god_drop_char(nr, x + 1, y))
	{
		return 1;
	}
	if (can_go(xs, ys, x - 1, y) && god_drop_char(nr, x - 1, y))
	{
		return 1;
	}
	if (can_go(xs, ys, x, y + 1) && god_drop_char(nr, x, y + 1))
	{
		return 1;
	}
	if (can_go(xs, ys, x, y - 1) && god_drop_char(nr, x, y - 1))
	{
		return 1;
	}

	if (can_go(xs, ys, x + 1, y + 1) && god_drop_char(nr, x + 1, y + 1))
	{
		return 1;
	}
	if (can_go(xs, ys, x + 1, y - 1) && god_drop_char(nr, x + 1, y - 1))
	{
		return 1;
	}
	if (can_go(xs, ys, x - 1, y + 1) && god_drop_char(nr, x - 1, y + 1))
	{
		return 1;
	}
	if (can_go(xs, ys, x - 1, y - 1) && god_drop_char(nr, x - 1, y - 1))
	{
		return 1;
	}

	if (can_go(xs, ys, x + 2, y - 2) && god_drop_char(nr, x + 2, y - 2))
	{
		return 1;
	}
	if (can_go(xs, ys, x + 2, y - 1) && god_drop_char(nr, x + 2, y - 1))
	{
		return 1;
	}
	if (can_go(xs, ys, x + 2, y + 0) && god_drop_char(nr, x + 2, y + 0))
	{
		return 1;
	}
	if (can_go(xs, ys, x + 2, y + 1) && god_drop_char(nr, x + 2, y + 1))
	{
		return 1;
	}
	if (can_go(xs, ys, x + 2, y + 2) && god_drop_char(nr, x + 2, y + 2))
	{
		return 1;
	}
	if (can_go(xs, ys, x - 2, y - 2) && god_drop_char(nr, x - 2, y - 2))
	{
		return 1;
	}
	if (can_go(xs, ys, x - 2, y - 1) && god_drop_char(nr, x - 2, y - 1))
	{
		return 1;
	}
	if (can_go(xs, ys, x - 2, y + 0) && god_drop_char(nr, x - 2, y + 0))
	{
		return 1;
	}
	if (can_go(xs, ys, x - 2, y + 1) && god_drop_char(nr, x - 2, y + 1))
	{
		return 1;
	}
	if (can_go(xs, ys, x - 2, y + 2) && god_drop_char(nr, x - 2, y + 2))
	{
		return 1;
	}
	if (can_go(xs, ys, x - 1, y + 2) && god_drop_char(nr, x - 1, y + 2))
	{
		return 1;
	}
	if (can_go(xs, ys, x + 0, y + 2) && god_drop_char(nr, x + 0, y + 2))
	{
		return 1;
	}
	if (can_go(xs, ys, x + 1, y + 2) && god_drop_char(nr, x + 1, y + 2))
	{
		return 1;
	}
	if (can_go(xs, ys, x - 1, y - 2) && god_drop_char(nr, x - 1, y - 2))
	{
		return 1;
	}
	if (can_go(xs, ys, x + 0, y - 2) && god_drop_char(nr, x + 0, y - 2))
	{
		return 1;
	}
	if (can_go(xs, ys, x + 1, y - 2) && god_drop_char(nr, x + 1, y - 2))
	{
		return 1;
	}

	return 0;
}

int god_give_char(int in, int cn)
{
	int n, tmp, stacksize;

	if (!IS_SANEITEM(in) || !IS_LIVINGCHAR(cn))
	{
		return 0;
	}
	if (IS_PLAYER(cn)) for (n = 0; n<40; n++)
	{
		// Find a stackable item of the same template
		if ((it[in].flags & IF_STACKABLE) && it[in].temp == it[ch[cn].item[n]].temp && it[ch[cn].item[n]].stack<10)
		{
			if (it[ch[cn].item[n]].stack<1)	it[ch[cn].item[n]].stack=1;
			if (it[in].stack>1)
			{
				tmp 		= it[ch[cn].item[n]].value / it[ch[cn].item[n]].stack;
				stacksize 	= it[ch[cn].item[n]].stack + it[in].stack;
				
				if (stacksize > 10)
				{
					it[ch[cn].item[n]].stack = 10;
					it[ch[cn].item[n]].value = tmp * it[ch[cn].item[n]].stack;
					it[in].stack 	= stacksize-10;
					it[in].value 	= tmp * it[in].stack;
					break;
				}
				else
				{
					it[ch[cn].item[n]].stack += it[in].stack;
					it[ch[cn].item[n]].value = tmp * it[ch[cn].item[n]].stack;
				}
			}
			else
			{
				tmp = it[ch[cn].item[n]].value / it[ch[cn].item[n]].stack;
				it[ch[cn].item[n]].stack++;
				it[ch[cn].item[n]].value = tmp * it[ch[cn].item[n]].stack;
			}
			do_update_char(cn);
			return 1;
		}
	}
	for (n = 0; n<40; n++)
	{
		// Find an empty inventory slot
		if (!ch[cn].item[n])
		{
			break;
		}
	}
	if (n==40)
	{
		if (ch[cn].citem)
		{
			return 0;
		}
		ch[cn].citem = in;
	}
	else
	{
		ch[cn].item[n] = in;
	}

	it[in].x = 0;
	it[in].y = 0;
	it[in].carried = cn;

	do_update_char(cn);
	return 1;
}

int god_take_from_char(int in, int cn)
{
	int n;

	if (ch[cn].citem==in)
	{
		ch[cn].citem = 0;
	}
	else
	{
		for (n = 0; n<40; n++)
		{
			if (ch[cn].item[n]==in)
			{
				break;
			}
		}
		if (n<40)
		{
			ch[cn].item[n] = 0;
			ch[cn].item_lock[n] = 0;
		}
		else
		{
			for (n = 0; n<20; n++)
			{
				if (ch[cn].worn[n]==in)
				{
					break;
				}
			}
			if (n<20)
			{
				ch[cn].worn[n] = 0;
			}
			else
			{
				for (n = 0; n<12; n++)
				{
					if (ch[cn].alt_worn[n]==in)
					{
						break;
					}
				}
				if (n==12)
				{
					return 0;
				}
				ch[cn].alt_worn[n] = 0;
			}
		}
	}

	it[in].x = 0;
	it[in].y = 0;
	it[in].carried = 0;

	do_update_char(cn);

	return 1;
}

int god_transfer_char(int cn, int x, int y)
{
	int ret = 0, gc, sc;
	
	if (!IS_SANECHAR(cn) || !SANEXY(x, y))
	{
		return 0;
	}
	
	gc = ch[cn].data[PCD_COMPANION];
	sc = ch[cn].data[PCD_SHADOWCOPY];
	
	ch[cn].status = 0;
	ch[cn].attack_cn = 0;
	ch[cn].skill_nr  = 0;
	ch[cn].goto_x = 0;

	plr_map_remove(cn);
	
	if (god_drop_char_fuzzy_large(cn, x, y, x, y)) 		ret = 1;
	else if (god_drop_char_fuzzy_large(cn, x + 3, y, x, y))	ret = 1;
	else if (god_drop_char_fuzzy_large(cn, x - 3, y, x, y))	ret = 1;
	else if (god_drop_char_fuzzy_large(cn, x, y + 3, x, y))	ret = 1;
	else if (god_drop_char_fuzzy_large(cn, x, y - 3, x, y))	ret = 1;
	
	if (ret)
	{
		if (ch[cn].flags & CF_GCTOME)
		{
			if (IS_SANECHAR(gc) && ch[gc].data[CHD_MASTER]==cn) god_transfer_char(gc, x, y);
			if (IS_SANECHAR(sc) && ch[sc].data[CHD_MASTER]==cn) god_transfer_char(sc, x, y);
		}
		return 1;
	}
	
	plr_map_set(cn);
	
	return 0;
}

/* CS, 991128: GOTO n|e|s|w <nnn> */
void god_goto(int cn, int co, char *cx, char *cy)
{
	int xo, yo;

	xo = ch[co].x;
	yo = ch[co].y;

	if (!*cy)   // GOTO to a character
	{
		int who = dbatoi(cx);
		if (!IS_SANECHAR(who) || ((ch[who].flags & CF_GOD) && !(ch[cn].flags & CF_GOD)))
		{
			do_char_log(cn, 0, "Bad character: %s!\n", cx);
			return;
		}
		else if (god_transfer_char(co, ch[who].x, ch[who].y))
		{
			chlog(cn, "IMP: transferred %s to %s (%s).", ch[co].name, ch[who].name, get_area_m(ch[who].x, ch[who].y, 0));
			if (!(ch[co].flags & CF_INVISIBLE))
			{
				fx_add_effect(12, 0, xo, yo, 0);
				fx_add_effect(12, 0, ch[co].x, ch[co].y, 0);
			}
			return;
		}
	}
	else     // x,y coordinates
	{
		int x, y;

		x = ch[co].x;
		y = ch[co].y;
		if      (!strcasecmp(cx, "n"))
		{
			x -= atoi(cy);
		}
		else if (!strcasecmp(cx, "e"))
		{
			y += atoi(cy);
		}
		else if (!strcasecmp(cx, "s"))
		{
			x += atoi(cy);
		}
		else if (!strcasecmp(cx, "w"))
		{
			y -= atoi(cy);
		}
		else
		{
			x = atoi(cx);
			y = atoi(cy);
		}
		if (!SANEXY(x, y))
		{
			do_char_log(cn, 0, "Bad coordinates(%d %d)\n", x, y);
			return;
		}
		if (god_transfer_char(co, x, y))
		{
			chlog(cn, "IMP: transferred %s to %d,%d (%s).", ch[co].name, x, y, get_area_m(x, y, 0));
			if (!(ch[co].flags & CF_INVISIBLE))
			{
				fx_add_effect(12, 0, xo, yo, 0);
				fx_add_effect(12, 0, ch[co].x, ch[co].y, 0);
			}
			return;
		}
	}
	do_char_log(cn, 0, "GOTO failed. Solid object there.\n");
	return;
}

void god_info(int cn, int co)
{
	int t;
	/* CS, 991128: Support for different levels of #INFO */
	char cnum_str[20], pos_str[30], temp_str[20], need_str[20];
	int pos_x, pos_y;
	int pts;

	if (!IS_SANECHAR(co))
	{
		do_char_log(cn, 0, "There's no such character.\n");
		return;
	}

	if (IS_SANENPC(co) && !(IS_GOD(cn) | IS_IMP(cn) | IS_USURP(cn)))
	{
		do_char_log(cn, 0, "Access denied.\n");
		return;
	}

	if ((ch[co].flags & CF_GOD) && !(ch[cn].flags & CF_GOD))
	{
		do_char_log(cn, 0, "Access denied.\n");
		return;
	}

	char_info(cn, co);
	/* CS, 991127: No position for invis Imps, no char num to staff */
	if (IS_IMP(cn)||IS_USURP(cn))
	{
		sprintf(cnum_str, " (%d)", co);
	}
	else
	{
		cnum_str[0] = '\0';
	}
	pos_x = ch[co].x;
	pos_y = ch[co].y;
	if (!IS_IMP(cn) && !IS_USURP(cn) && ((ch[co].flags & (CF_INVISIBLE | CF_NOWHO)) && invis_level(co) > invis_level(cn)))
	{
		pos_x = pos_y = 0;
	}
	if (pos_x != 0 || pos_y != 0)
	{
		sprintf(pos_str, " Pos=%d,%d.", pos_x, pos_y);
	}
	else
	{
		pos_str[0] = '\0';
	}
	/* CS, 991127: Display exp/exp to next level or template number */
	pts = ch[co].points_tot;
	strcpy(need_str, int2str(points_tolevel(pts)));
	if (IS_PLAYER(co))
	{
		do_char_log(cn, 2, "%s %s%s%s Pts/need=%s/%s.\n",
		            who_rank_name[points2rank(pts)],
		            ch[co].name,
		            cnum_str, pos_str,
		            int2str(pts), need_str);
	}
	else
	{
		if (IS_IMP(cn) || IS_USURP(cn))
		{
			sprintf(temp_str, " Temp=%d", ch[co].temp);
		}
		else
		{
			temp_str[0] = '\0';
		}
		do_char_log(cn, 2, "%s %s%s%s%s.\n",
		            who_rank_name[points2rank(pts)],
		            ch[co].name,
		            cnum_str, pos_str, temp_str);
	}
	do_char_log(cn, 2, "HP=%d/%d, End=%d/%d, Mana=%d/%d.\n",
	            ch[co].a_hp / 1000, ch[co].hp[5],
	            ch[co].a_end / 1000, ch[co].end[5],
	            ch[co].a_mana / 1000, ch[co].mana[5]);
	do_char_log(cn, 2, "Speed=%d. Gold=%d.%02dG (%d.%02dG).\n",
	            ch[co].speed,
	            ch[co].gold / 100, ch[co].gold % 100,
	            ch[co].data[13] / 100, ch[co].data[13] % 100);
	if (IS_PLAYER(co) && (IS_CLANKWAI(co) || IS_CLANGORN(co)) && ch[co].data[PCD_ATTACKTIME])
	{
		if (IS_IMP(cn) && IS_SANEPLAYER(ch[co].data[PCD_ATTACKVICT]))
		{
			do_char_log(cn, 0, "Last PvP attack: %s, against %s.\n",
			            ago_string(globs->ticker - ch[co].data[PCD_ATTACKTIME]),
			            ch[ch[co].data[PCD_ATTACKVICT]].name);
		}
		else
		{
			do_char_log(cn, 0, "Last PvP attack: %s.\n",
			            ago_string(globs->ticker - ch[co].data[PCD_ATTACKTIME]));
		}
	}
	if (IS_IMP(cn) || IS_USURP(cn))
	{
		if (IS_PLAYER(co))
		{
			do_char_log(cn, 3, "Killed %d NPCs below rank, %d NPCs at rank, %d NPCs above rank.\n",
			            ch[co].data[23], ch[co].data[24], ch[co].data[25]);
			do_char_log(cn, 3, "Killed %d players outside arena, killed %d shopkeepers.\n",
			            ch[co].data[29], ch[co].data[40]);
			do_char_log(cn, 3, "Stronghold: %d total points\n", ch[co].bs_points);
		}
		do_char_log(cn, 1, "Drivers [%d,%d,%d,%d,%d,%d,%d,%d,%d,%d].\n",
			            ch[co].data[0], ch[co].data[1], ch[co].data[2], ch[co].data[3], ch[co].data[4],
			            ch[co].data[5], ch[co].data[6], ch[co].data[7], ch[co].data[8], ch[co].data[9]);
		do_char_log(cn, 1, "CN %d, CS %d, CG %d, CP %d.\n",
			            CONT_NUM(co), CONT_SCEN(co), CONT_GOAL(co), CONT_PROG(co));
		//do_char_log(cn, 2, "Current location=%d, Home coords: %d.\n",
		//	            ch[co].x + ch[co].y * MAPX, ch[co].data[29]);
		//do_char_log(cn, 1, "Rwalk=%d, Rwalk tm=%d, Rwalk ds=%d.\n",
		//	            ch[co].data[60], ch[co].data[61], ch[co].data[73]);
		do_char_log(cn, 3, "Armor=%d, Weapon=%d. Alignment=%d.\n", ch[co].armor, ch[co].weapon, ch[co].alignment);
		do_char_log(cn, 3, "Group=%d (%d), Single Awake=%d, Spells=%d.\n", ch[co].data[CHD_GROUP], group_active(co), ch[co].data[92], ch[co].data[96]);
		do_char_log(cn, 3, "Luck=%d, Gethit_Dam=%d.\n", ch[co].luck, ch[co].gethit_dam);
		do_char_log(cn, 3, "Current Online Time: %dd %dh %dm %ds, Total Online Time: %dd %dh %dm %ds.\n",
		            ch[co].current_online_time / (TICKS * 60 * 60 * 24),
		            ch[co].current_online_time / (TICKS * 60 * 60) % 24,
		            ch[co].current_online_time / (TICKS * 60) % 60,
		            ch[co].current_online_time / (TICKS) % 60,
		            ch[co].total_online_time / (TICKS * 60 * 60 * 24),
		            ch[co].total_online_time / (TICKS * 60 * 60) % 24,
		            ch[co].total_online_time / (TICKS * 60) % 60,
		            ch[co].total_online_time / (TICKS) % 60);

		if (IS_SANENPC(co) && ch[co].data[64])
		{
			t  = ch[co].data[64] - globs->ticker;
			t /= TICKS;
			do_char_log(cn, 3, "Will self destruct in %dm %ds.\n", t / 60, t % 60);
		}
	}
	return;
}

void god_iinfo(int cn, int in)
{
	if (!IS_SANEITEM(in))
	{
		do_char_log(cn, 0, "Argument out of bounds.\n");
		return;
	}
	if (!IS_USEDITEM(in))
	{
		do_char_log(cn, 0, "There's no such item.\n");
		return;
	}

	item_info(cn, in, 0);
	do_char_log(cn, 1, "Owner: %s (%d), Position %d,%d.\n",
	            ch[it[in].carried].name, it[in].carried, it[in].x, it[in].y);
	return;
}

void god_tinfo(int cn, int temp)
{
	int in;

	if (!IS_SANEITEMPLATE(temp) || !it_temp[temp].used)
	{
		do_char_log(cn, 0, "There's no such item.\n");
		return;
	}

	for (in = 1; in<MAXITEM; in++)
	{
		if (it[in].used==USE_EMPTY)
		{
			continue;
		}
		if (it[in].temp==temp)
		{
			break;
		}
	}

	if (in==MAXITEM)
	{
		do_char_log(cn, 1, "No %s is in the game.\n", it_temp[temp].name);
		return;
	}

	item_info(cn, in, 0);
	do_char_log(cn, 1, "Owner: %s (%d), Position %d,%d (%d).\n",
	            ch[it[in].carried].name, it[in].carried, it[in].x, it[in].y, in);
}

void god_unique(int cn)
{
	static int unique[60] = {
	//	280, 281, 282, 283, 284, 285, 286, 287, 288, 289, 290, 291, 292, 525, 526,
	//	527, 528, 529, 530, 531, 532, 533, 534, 535, 536, 537, 538, 539, 540, 541,
	//	542, 543, 544, 545, 546, 547, 548, 549, 550, 551, 552, 553, 554, 555, 556,
	//	572, 573, 574, 575, 576, 577, 578, 579, 580, 581, 582, 583, 584, 585, 586
	};
	int owner[60];
	int n, in;

	for (n = 0; n<60; n++)
	{
		owner[n] = 0;
	}

	for (in = 1; in<MAXITEM; in++)
	{
		if (it[in].used==USE_EMPTY)
		{
			continue;
		}
		for (n = 0; n<60; n++)
		{
			if (it[in].temp==unique[n])
			{
				owner[n] = it[in].carried;
			}
		}
	}

	for (n = 0; n<60; n++)
	{
		do_char_log(cn, 2, "%-25.25s: %.10s (%d)\n",
		            it_temp[unique[n]].name, ch[owner[n]].name, owner[n]);
	}
}

char *int2str(int val)
{
	static char buf[256];

	if (val<99 * 1000)
	{
		sprintf(buf, "%d", val);
	}
	else if (val<99 * 1000 * 1000)
	{
		sprintf(buf, "%dK", val / 1000);
	}
	else
	{
		sprintf(buf, "%dM", val / 1000000);
	}

	return(buf);
}

void god_who(int cn)
{
	int n, m, players, font, showarea;

	players = 0;
	do_char_log(cn, 3, "-----------------------------------------------\n");
	for (n = 1; n<MAXCHARS; n++)
	{
		if (!IS_ACTIVECHAR(n))
		{
			continue;
		}
		else if (!IS_PLAYER(n))
		{
			if (!IS_USURP(n))
			{
				continue;
			}
			if (!(IS_GOD(cn)||IS_IMP(cn)) )
			{
				continue;
			}
			font = 3; // Usurped mobs are blue for imps
		}
		else if (IS_INVISIBLE(n))
		{
			if (invis_level(cn)<invis_level(n))
			{
				continue;
			}
			font = 0; // Invisible chars are red for highers
		}
		else if (ch[n].flags & CF_NOWHO)
		{
			if (!(ch[cn].flags & (CF_IMP | CF_GOD)))
			{
				continue;
			}
			// only imps below this point
			font = 3; // NOWHO'd chars are blue for imps
		}
		else if (IS_STAFF(n) || IS_GOD(n))
		{
			// staff and gods are green for everyone
			font = 2;
		}
		else
		{
			font = 1; // all others yellow
		}
		players++;

		showarea = 1;
		if ((ch[n].flags & CF_GOD) && !(ch[cn].flags & CF_GOD))
		{
			showarea = 0;
		}
		if (IS_OPP_CLAN(cn, n) && !(ch[cn].flags & (CF_GOD | CF_IMP | CF_USURP)))
		{
			showarea = 0;
		}

		do_char_log(cn, font, "%4d: %-10.10s%c%c %-8.8s %.19s\n",
		            n, ch[n].name,
		            IS_CLANKWAI(n) ? 'K' : (IS_CLANGORN(n) ? 'G' : ' '),
		            IS_PURPLE(n) ? '*' : ' ',
		            int2str(ch[n].points_tot),
		            !showarea ? "--------" : get_area(n, 0));
	}
	/* list player's GC and thralls, if any -- limited to 10 entries as to not flood /who by a country mile */
	for (n = 1, m = 0; n<MAXCHARS; n++)
	{
		if (!IS_ACTIVECHAR(n))				continue;
		if ((ch[n].flags & (CF_PLAYER)))	continue;
		if (ch[n].data[CHD_MASTER] != cn)	continue;
		
		do_char_log(cn, 3, "%4d: %-10.10s@ %-8.8s %.20s\n",
		            n, ch[n].name,
		            int2str(ch[n].points_tot),
		            get_area(n, 0));
		/*
		do_char_log(cn, 3, "%.5s %-10.10s%c%c%c %.23s\n",
		            who_rank_name[points2rank(ch[n].points_tot)],
		            ch[n].name,
		            IS_PURPLE(n) ? '*' : ' ',
		            (ch[n].flags & CF_POH) ? '+' : ' ',
		            (ch[n].flags & CF_POH_LEADER) ? '+' : ' ',
		            get_area(n, 0));
		*/
		m++;
		if (m>10) break;
	}
	do_char_log(cn, 3, "-----------------------------------------------\n");
	do_char_log(cn, 1, "%3d player%s online.\n",
	            players, (players > 1) ? "s" : "");
}

void god_implist(int cn)
{
	int n, players, showarea;

	players = 0;
	do_char_log(cn, 3, "-----------------------------------------------\n");
	for (n = 1; n<MAXCHARS; n++)
	{
		if (!IS_USEDCHAR(n))
		{
			continue;
		}
		if (!IS_PLAYER(n))
		{
			continue;
		}
		if (!IS_IMP(n))
		{
			continue;
		}
		players++;

		showarea = 1;
		if ((ch[n].flags & CF_GOD) && !(ch[cn].flags & CF_GOD))
		{
			showarea = 0;
		}
		if (IS_OPP_CLAN(cn, n) && !(ch[cn].flags & (CF_GOD | CF_IMP | CF_USURP)))
		{
			showarea = 0;
		}

		do_char_log(cn, 1, "%4d: %-10.10s%c%c %-8.8s %.19s\n",
		            n, ch[n].name,
		            IS_CLANKWAI(n) ? 'K' : (IS_CLANGORN(n) ? 'G' : ' '),
		            IS_PURPLE(n) ? '*' : ' ',
		            int2str(ch[n].points_tot),
		            !showarea ? "--------" : get_area(n, 0));
	}
	do_char_log(cn, 3, "-----------------------------------------------\n");
	do_char_log(cn, 1, "%3d imp%s.\n",
	            players, (players > 1) ? "s" : "");
}

void user_who(int cn)
{
	int n, players, gc, font, showarea;

	do_char_log(cn, 1, "-----------------------------------------------\n");
	players = 0;
	/* list players */
	for (n = 1; n<MAXCHARS; n++)
	{
		if (!(ch[n].flags & (CF_PLAYER)))
		{
			continue;
		}
		if (ch[n].used!=USE_ACTIVE || (ch[n].flags & (CF_INVISIBLE | CF_NOWHO)))
		{
			continue;
		}
		players++;
		/* color staff and gods green */
		font = (IS_STAFF(n) || IS_GOD(n)) ? 2 : 1;

		showarea = 1;
		if ((ch[n].flags & CF_GOD) && !(ch[cn].flags & CF_GOD))
		{
			showarea = 0;
		}
		if (IS_OPP_CLAN(cn, n) && !(ch[cn].flags & (CF_GOD | CF_IMP | CF_USURP)))
		{
			showarea = 0;
		}

		do_char_log(cn, font, "%.5s %-10.10s%c%c %.24s\n",
		            who_rank_name[points2rank(ch[n].points_tot)],
		            ch[n].name,
		            IS_CLANKWAI(n) ? 'K' : (IS_CLANGORN(n) ? 'G' : ' '),
		            IS_PURPLE(n) ? '*' : ' ',
		            !showarea ? "--------" : get_area(n, 0));
	}
	if ((gc = ch[cn].data[PCD_COMPANION]) && IS_SANECHAR(gc))
	{
		do_char_log(cn, 3, "%.5s %-10.10s%c%c %.24s\n",
		            who_rank_name[points2rank(ch[gc].points_tot)],
		            ch[gc].name,
		            IS_CLANKWAI(n) ? 'K' : (IS_CLANGORN(n) ? 'G' : ' '),
		            IS_PURPLE(n) ? '*' : ' ',
		            get_area(gc, 0));
		/*
		do_char_log(cn, 3, "%4d: %-10.10s@ %-8.8s %.20s\n",
		            gc, ch[gc].name,
		            int2str(ch[gc].points_tot),
		            get_area(gc, 0));
		*/
	}
	do_char_log(cn, 1, "-----------------------------------------------\n");
	do_char_log(cn, 1, "%3d player%s online.\n",
	            players, (players > 1) ? "s" : "");
}

void god_top(int cn)
{
	do_char_log(cn, 1, "Only partially implemented.\n");

	do_char_log(cn, 1, "Today is the %d%s%s%s%s day of the Year %d. It is %d:%02d Astonian Standard Time.\n",
	            globs->mdday,
	            (globs->mdday==1 ? "st" : ""),
	            (globs->mdday==2 ? "nd" : ""),
	            (globs->mdday==3 ? "rd" : ""),
	            (globs->mdday>3 ? "th" : ""),
	            globs->mdyear, globs->mdtime / MD_HOUR, (globs->mdtime / 60) % 60);

	if ((globs->mdday % 28) + 1==1)
	{
		do_char_log(cn, 1, "New Moon tonight!\n");
	}
	else if ((globs->mdday % 28) + 1<15)
	{
		do_char_log(cn, 1, "The Moon is growing.\n");
	}
	else if ((globs->mdday % 28) + 1==15)
	{
		do_char_log(cn, 1, "Full Moon tonight!\n");
	}
	else
	{
		do_char_log(cn, 1, "The moon is dwindling.\n");
	}
}

void god_create(int cn, int x, int gen_a, int gen_b, int gen_c)
{
	int in, bonus = 1;
	char *gend, *godn, name[60], refer[60], descr[220];

	if (x == 0)
	{
		do_char_log(cn, 0, "No such item.\n");
		return;
	}
	else if (!IS_SANEITEM(x))
	{
		do_char_log(cn, 0, "Bad item number: %d.\n", x);
		return;
	}
	if (!(it_temp[x].flags & IF_TAKE))
	{
		do_char_log(cn, 0, "item is not take-able.\n");
		return;
	}
	if (!gen_a)
	{
		in = god_create_item(x);
	}
	else
	{
		if (it_temp[x].flags & IF_STACKABLE)
		{
			in = god_create_item(x);
			
			if (gen_a > 10) gen_a = 10;
			if (gen_a < 1) gen_a = 1;
			it[in].stack = gen_a;
			it[in].value = it[in].value * it[in].stack;
		}
		else if (gen_a > 100 && is_unique_able(x))
		{
			gen_a -= 100;
			in = god_create_item(x); 			// create the item
			if (in)								// assure the item is created
			{
				if (IS_TWOHAND(in))
				{
					bonus = 2;
				}
				
				switch (gen_a)
				{
					case 1:
						gend = " god ";
						godn = "Skua";
						it[in].flags |= IF_KWAI_UNI | IF_GORN_UNI;
						it[in].speed[0]      +=  8 * bonus;
						break;
					case 2:
						gend = " goddess ";
						godn = "Kwai";
						it[in].flags |= IF_KWAI_UNI;
						it[in].to_hit[0]     +=  4 * bonus;
						it[in].to_parry[0]   +=  4 * bonus;
						break;
					case 3:
						gend = " god ";
						godn = "Gorn";
						it[in].flags |= IF_GORN_UNI;
						it[in].spell_mod[0]  +=  2 * bonus;
						break;
					default:
						gend = " ";
						godn = "Purple One";
						it[in].flags |= IF_PURP_UNI;
						it[in].top_damage[0] += 20 * bonus;
						break;
				}
				
				if (it[in].flags & IF_OF_SHIELD)
				{
					it[in].armor[0]  += 4 * bonus;
					//it[in].max_damage = 5000 * it[in].armor[0]/2;
				}
				else
				{
					it[in].weapon[0] += 4 * bonus;
					//it[in].max_damage = 5000 * it[in].weapon[0]/2;
				}
				it[in].orig_temp = it[in].temp;
				it[in].temp = 0;
				it[in].flags |= IF_SINGLEAGE | IF_SHOPDESTROY | IF_NOMARKET | IF_UNIQUE | IF_NOREPAIR;
				strcpy(name, it[in].name);
				strcpy(refer, it[in].reference);
				strcpy(descr, it[in].description);
				sprintf(it[in].name, "%s's %s", godn, name);
				sprintf(it[in].reference, "%s's %s", godn, refer);
				sprintf(it[in].description, "%s It has been blessed by the%s%s.", descr, gend, godn);
				
				if (is_unique_able(x) > 54) // Claws
				{
					if (gen_a==1)
						it[in].sprite[0] = 3715 + is_unique_able(x)-55;
					else if (gen_a==2)
						it[in].sprite[0] = 3721 + is_unique_able(x)-55;
					else if (gen_a==3)
						it[in].sprite[0] = 3727 + is_unique_able(x)-55;
					else
						it[in].sprite[0] = 4944 + is_unique_able(x)-55;
				}
				else if (is_unique_able(x) > 45)
				{
					if (gen_a==1)
						it[in].sprite[0] = 2602 + is_unique_able(x)-46;
					else if (gen_a==2)
						it[in].sprite[0] = 2611 + is_unique_able(x)-46;
					else if (gen_a==3)
						it[in].sprite[0] = 2620 + is_unique_able(x)-46;
					else
						it[in].sprite[0] = 4935 + is_unique_able(x)-46;
				}
				else
				{
					if (gen_a==1)
						it[in].sprite[0] =  730 + is_unique_able(x)-1;
					else if (gen_a==2)
						it[in].sprite[0] = 2512 + is_unique_able(x)-1;
					else if (gen_a==3)
						it[in].sprite[0] = 2557 + is_unique_able(x)-1;
					else
						it[in].sprite[0] = 4890 + is_unique_able(x)-1;
				}
			}
		}
		else
		{
			in = get_special_item(x, gen_a, gen_b, gen_c);
		}
	}
	if (in==0)
	{
		do_char_log(cn, 0, "god_create_item() failed.\n");
		return;
	}
	if (!god_give_char(in, cn))
	{
		do_char_log(cn, 0, "Your inventory is full!\n");
		return;
	}
	do_char_log(cn, 1, "created one %s.\n", it[in].name);
	chlog(cn, "IMP: created one %s.", it[in].name);
	return;
}

void god_cleanslots(int cn)
{
	int n, m;
	
	do_char_log(cn, 1, "Now cleaning all character dummy slots...\n");
	for (n = 1; n<MAXCHARS; n++)
	{
		if (!(ch[n].flags & (CF_PLAYER)))
		{
			continue;
		}
		if (ch[n].used==USE_EMPTY)
		{
			continue;
		}
		for (m=12;m<20;m++)
		{
			ch[n].worn[m] = 0;
		}
	}
	do_char_log(cn, 0, "Done.\n");
	return;
}

void god_reset_items(int cn)
{
	int n;
	
	do_char_log(cn, 1, "Now resetting all items...\n");
	for (n = 1; n<MAXTITEM; n++)
	{
		if (it_temp[n].used==USE_EMPTY) continue;
		reset_item(n);
	}
	do_char_log(cn, 0, "Done.\n");
}

void god_reset_player(int cn, int co)
{
	if (co<1 || co>=MAXCHARS)
	{
		do_char_log(cn, 0, "That template number is a bit strange, don't you think so, dude?\n");
		return;
	}
	
	do_char_log(cn, 1, "Now resetting player %s (%d)\n", ch[co].name, co);
	god_racechange(co, ch[co].temp, 1);
	do_char_log(cn, 0, "Done.\n");
	return;
}

void god_reset_players(int cn)
{
	int n;
	do_char_log(cn, 1, "Now resetting all players...\n");
	for (n=2;n<MAXCHARS;n++)
	{
		if (ch[n].used==USE_EMPTY)
		{
			continue;
		}
		if (!IS_SANEPLAYER(n))
		{
			continue;
		}
		god_racechange(n, ch[n].temp, 1);
	}
	do_char_log(cn, 0, "Done.\n");
	return;
}

void god_reset_npcs(int cn)
{
	int n;
	
	if (cn) 
		do_char_log(cn, 1, "Now wiping all npcs...\n");
	else 
		xlog("Now wiping all npcs...");
	for (n = 1; n<MAXCHARS; n++)
	{
		if (ch[n].used==USE_EMPTY)
		{
			continue;
		}
		if (IS_SANEPLAYER(n))
		{
			continue;
		}
		god_destroy_items(n);
		if (ch[n].used==USE_ACTIVE)
		{
			plr_map_remove(n);
		}
		ch[n].flags = 0;
		ch[n].used = USE_EMPTY;
	}
	if (cn) 
		do_char_log(cn, 1, "Now resetting npcs...\n");
	else
		xlog("Now resetting npcs...");
	for (n = 1; n<MAXTCHARS; n++)
	{
		if (ch_temp[n].flags & CF_RESPAWN)
		{
			reset_char(n);
		}
	}
	globs->reset_char = 0;
	if (cn)
		do_char_log(cn, 0, "Done.\n");
	else
		xlog("Done.");
	return;
}

void god_reset_ticker(int cn)
{
	do_char_log(cn, 1, "The ticker was %d\n", globs->ticker);
	globs->ticker 	= 0;
	globs->mdtime 	= 0;
	globs->mdday 	= 0;
	globs->mdyear 	= 0;
	do_char_log(cn, 0, "The ticker is now %d\nCheck #time for further details.\n", globs->ticker);
	return;
}

int find_next_char(int startcn, char *spec1, char *spec2)
{
	int n;
	int rank;

	if (isdigit(*spec2))
	{
		rank = atoi(spec2);
	}
	else
	{
		rank = -1;
	}
	for (n = startcn + 1; n<MAXCHARS; n++)
	{
		if (!IS_USEDCHAR(n))
		{
			continue;
		}
		if (!strstr(ch[n].name, spec1))
		{
			continue;
		}
		if (rank == -1)
		{
			if (strstr(ch[n].name, spec2))
			{
				return( n);
			}
		}
		else
		{
			if (points2rank(ch[n].points_tot) == rank)
			{
				return( n);
			}
		}
	}
	return 0;
}

int invis(int looker, int target)
{
	if (!(ch[target].flags & CF_INVISIBLE))
	{
		return 0;
	}
	if (invis_level(looker)>=invis_level(target))
	{
		return 0;
	}

	return 1;
}


void god_summon(int cn, char *spec1, char *spec2, char *spec3)
{
	int co;
	int x, y, xo, yo;
	int which = 1, count = 0;

	if (!*spec1)            // no arguments
	{
		do_char_log(cn, 0, "summon whom?\n");
		return;
	}
	if (!*spec2)    // only one arg
	{
		co = dbatoi(spec1);
		if (!IS_SANEUSEDCHAR(co) || invis(cn, co))       // bad character number
		{
			do_char_log(cn, 0, "No such character.\n");
			return;
		}
		if (ch[co].flags & CF_BODY)     // dead character
		{
			do_char_log(cn, 0, "Character recently deceased; try %d.\n",
			            ch[co].data[CHD_CORPSEOWNER]);
			return;
		}
		if (co == cn)                   // self
		{
			do_char_log(cn, 0, "You can't summon yourself!\n");
			return;
		}
	}
	else                    // at least 2 args
	{
		co = 0;
		if (isdigit(*spec2))
		{
			if (atoi(spec2) < 0 || atoi(spec2) >= RANKS)
			{
				do_char_log(cn, 0, "No such rank: %s\n", spec2);
				return;
			}
		}
		else
		{
			titlecase_str(spec2);
		}
		titlecase_str(spec1);
		if (*spec3)     // 3 args
		{
			which = max(atoi(spec3), 1);
		}
		while (count < which)
		{
			co = find_next_char(co, spec1, spec2);
			if (co == 0)
			{
				break;
			}
			if (co == cn)
			{
				continue;                       // ignore self
			}
			if (ch[co].flags & CF_BODY)
			{
				continue;                       // ignore bodies
			}
			if (IS_PLAYER(co) && !IS_ACTIVECHAR(co))
			{
				continue;                                  // ignore sleeping players
			}
			if (invis(cn, co))
			{
				continue;               // ignore whom we cant see
			}
			count++;
		}
		if (co == 0)
		{
			if (!isdigit(*spec2))
			{
				if (count == 0)
				{
					do_char_log(cn, 0, "Couldn't find a %s %s.\n", spec1, spec2);
				}
				else
				{
					do_char_log(cn, 0, "Only found %d %s %s.\n", count, spec1, spec2);
				}
			}
			else
			{
				if (count == 0)
				{
					do_char_log(cn, 0, "Couldn't find a %s %s.\n", spec1, rank_name[atoi(spec2)]);
				}
				else
				{
					do_char_log(cn, 0, "Only found %d %s %s.\n", count, spec1, rank_name[atoi(spec2)]);
				}
			}
			return;
		}
	}

	x = ch[cn].x;
	y = ch[cn].y;
	/* try to transfer char to in front of summoner */
	switch (ch[cn].dir)
	{
	case DX_RIGHT:
		x++;
		break;
	case DX_RIGHTUP:
		x++;
		y--;
		break;
	case DX_UP:
		y--;
		break;
	case DX_LEFTUP:
		x--;
		y--;
		break;
	case DX_LEFT:
		x--;
		break;
	case DX_LEFTDOWN:
		x--;
		y++;
		break;
	case DX_DOWN:
		y++;
		break;
	case DX_RIGHTDOWN:
		y++;
		x++;
		break;
	}

	xo = ch[co].x;
	yo = ch[co].y;

	if (!god_transfer_char(co, x, y))
	{
		do_char_log(cn, 0, "god_transfer_char() failed.\n");

		fx_add_effect(12, 0, xo, yo, 0);
		fx_add_effect(12, 0, ch[co].x, ch[co].y, 0);

		return;
	}
	do_char_log(cn, 1, "%s was summoned.\n", ch[co].name);
	chlog(cn, "IMP: summoned %s.", ch[co].name);
	return;
}

/*   creates a mirror-enemy in front of the target (*spec1) with the skills of target
     with a skillbonus of (*spec2) , no equipment, same spells active              */

void god_mirror(int cn, char *spec1, char *spec2)
{
	int cc, co, i, bonus = 0;

	co = dbatoi(spec1);
	if (!*spec1)                            // no arguments
	{
		do_char_log(cn, 0, "create mirror-enemy of whom?\n");
		return;
	}
	if (!*spec2)                            // only one arg
	{
		bonus = 0;
		if (!IS_SANEUSEDCHAR(co))       // bad character number
		{
			do_char_log(cn, 0, "No such character.\n");
			return;
		}
		if (ch[co].flags & CF_BODY)     // dead character
		{
			do_char_log(cn, 0, "Character recently deceased. \n");
			return;
		}
		if (!IS_PLAYER(co))             // only mirror players (should we allow usurps?)
		{
			do_char_log(cn, 0, "%s is not a player, and you can't mirror monsters!\n",
			            ch[co].name);
			return;
		}
		if (co == cn)                   // self
		{
			do_char_log(cn, 0, "You want an enemy? Here it is...!\n");
		}
	}
	else if (isdigit(*spec2))
	{
		bonus = atoi(spec2);
	}
	;
	;                                                       // 2 arguments

//	cc=pop_create_char(968,0);
	if (!(cc = god_create_char(CT_UNDEAD, 0)))
	{
		do_char_log(cn, 0, "god_create_char() failed.\n");
		return;
	}


	strcpy(ch[cc].name, ch[co].name);
	ch[cc].sprite = ch[co].sprite;
	for (i = 0; i<5; i++)
	{
		B_AT(cc, i) = B_AT(co, i);
	}
	ch[cc].hp[0]   = ch[co].hp[0];
	ch[cc].end[0]  = ch[co].end[0];
	ch[cc].mana[0] = ch[co].mana[0];
	for (i = 1; i<35; i++)
	{
		B_SK(cc, i) = B_SK(co, i);
	}

	if      (ch[co].kindred & (KIN_TEMPLAR | KIN_ARCHTEMPLAR | KIN_SEYAN_DU))         // TH -> hand2hand (str,str,agi)
	{
		B_SK(co, 0) = B_SK(co, 6) + bonus + (B_AT(co, 4) - B_AT(co, 0)) / 5;
	}
	else if (ch[co].kindred & (KIN_HARAKIM | KIN_ARCHHARAKIM ))                       // Dag-> hand2hand (wil,agi,int)
	{
		B_SK(co, 0) = B_SK(co, 2) + bonus + (B_AT(co, 2) - B_AT(co, 4)) / 5;
	}
	else if (ch[co].kindred & (KIN_MERCENARY | KIN_SORCERER | KIN_WARRIOR ))          // Swo-> hand2hand (wil,agi,str)
	{
		B_SK(co, 0) = B_SK(co, 3) + bonus;
	}

	ch[co].weapon = ch[cn].weapon;
	ch[co].armor  = ch[cn].armor;
	do_update_char(cc);

	god_drop_char_fuzzy(cc, ch[cn].x, ch[cn].y); // to be replaced later by: _fuzzy(cc,ch[co].x,ch[co].y);
//        if (!god_transfer_char(cc, ch[cn].x, ch[cn].y))  {
//                do_char_log(cn,0,"god_transfer_char() failed.\n");
//                return;
//        }


	npc_add_enemy(cc, co, 1);
	do_char_log(cn, 0, "Mirror of %s active (bonus: %s) \n", spec1, spec2);
	chlog(cn, "IMP: created mirror against %s.", ch[co].name);
	return;
}


int god_thrall(int cn, char *spec1, char *spec2, int offset)
{
	int co, ct, x, y, n, in;

	if (!*spec1)            // no arguments
	{
		do_char_log(cn, 0, "enthrall whom?\n");
		return 0;
	}
	if (!*spec2)            // only one arg
	{
		co = dbatoi(spec1);
		if (!IS_SANEUSEDCHAR(co))       // bad character number
		{
			do_char_log(cn, 0, "No such character.\n");
			return 0;
		}
		if (ch[co].flags & CF_BODY)     // dead character
		{
			do_char_log(cn, 0, "Character recently deceased; try %d.\n",
			            ch[co].data[CHD_CORPSEOWNER]);
			return 0;
		}
		if (co == cn)                   // self
		{
			do_char_log(cn, 0, "You can't enthrall yourself!\n");
			return 0;
		}
	}
	else                    // at least 2 args
	{
		co = 0;
		if (isdigit(*spec2))
		{
			if (atoi(spec2) < 0 || atoi(spec2) >= RANKS)
			{
				do_char_log(cn, 0, "No such rank: %s\n", spec2);
				return 0;
			}
		}
		else
		{
			titlecase_str(spec2);
		}
		titlecase_str(spec1);
		while (1)
		{
			co = find_next_char(co, spec1, spec2);
			if (co == 0)
			{
				break;
			}
			if (co == cn)
			{
				continue;                       // ignore self
			}
			if (ch[co].flags & CF_BODY)
			{
				continue;                       // ignore bodies
			}
			break;
		}
		if (co == 0)
		{
			if (!isdigit(*spec2))
			{
				do_char_log(cn, 0, "Couldn't find a %s %s.\n", spec1, spec2);
			}
			else
			{
				do_char_log(cn, 0, "Couldn't find a %s %s.\n", spec1, rank_name[atoi(spec2)]);
			}
			return 0;
		}
	}

	if (IS_PLAYER(co))
	{
		do_char_log(cn, 0, "%s is a player, and you can't enthrall players!\n",
		            ch[co].name);
		return 0;
	}

	if (IS_COMPANION(co))
	{
		do_char_log(cn, 0, "%s is a companion/thrall, and you can't enthrall them!\n",
		            ch[co].name);
		return 0;
	}

	x = ch[cn].x;
	y = ch[cn].y;
	/* try to transfer char to in front of summoner */
	switch (ch[cn].dir)
	{
	case DX_RIGHT:
		x++; x+=offset;
		break;
	case DX_RIGHTUP:
		x++; x+=offset;
		y--; y-=offset;
		break;
	case DX_UP:
		y--; y-=offset;
		break;
	case DX_LEFTUP:
		x--; x-=offset;
		y--; y-=offset;
		break;
	case DX_LEFT:
		x--; x-=offset;
		break;
	case DX_LEFTDOWN:
		x--; x-=offset;
		y++; y+=offset;
		break;
	case DX_DOWN:
		y++; y+=offset;
		break;
	case DX_RIGHTDOWN:
		y++; y+=offset;
		x++; x+=offset;
		break;
	}

	if (!(ct = god_create_char(ch[co].temp, 1)))
	{
		do_char_log(cn, 0, "god_create_char() failed.\n");
		return -1;
	}

	strcpy(ch[ct].name, ch[co].name);
	strcpy(ch[ct].reference, ch[co].reference);
	strcpy(ch[ct].description, ch[co].description);

	/* tricky: make thrall act like a ghost companion */
	ch[ct].temp = CT_COMPANION;
	ch[ct].data[64] = globs->ticker + 7 * MD_DAY * TICKS;		// die in one week if not otherwise
	ch[ct].data[CHD_GROUP] = 65500;								// set group
	ch[ct].data[59] = 65500;									// protect all other members of this group

	/* make thrall harmless */
	ch[ct].data[24] = 0;				// do not interfere in fights
	ch[ct].data[36] = 0;				// no walking around
	ch[ct].data[43] = 0;				// don't attack anyone
	ch[ct].data[80] = 0;				// no enemies
	ch[ct].data[CHD_MASTER] = cn;		// obey and protect enthraller
	ch[cn].data[72] = 99;				// grants special talk.c properties

	ch[ct].flags |= CF_SHUTUP | CF_THRALL;
	ch[ct].alignment = -999;
	
	if (offset) 
	{
		ch[ct].text[1][0] = 0;  		// removes attack message entirely for "army" thralls
		ch[ct].data[43] = 65500;		// on second thought, *do* attack players if we're part of an army...
	}

	for (n = 0; n<20; n++)
	{
		if ((in = ch[ct].worn[n]) && (it[in].flags & IF_LABYDESTROY))
		{
			it[in].used = 0;
			ch[ct].worn[n] = 0;
		}
	}
	
	for (n = 0; n<12; n++)
	{
		if ((in = ch[ct].alt_worn[n]) && (it[in].flags & IF_LABYDESTROY))
		{
			it[in].used = 0;
			ch[ct].alt_worn[n] = 0;
		}
	}

	for (n = 0; n<40; n++)
	{
		if ((in = ch[ct].item[n]) && (it[in].flags & IF_LABYDESTROY))
		{
			it[in].used = 0;
			ch[ct].item[n] = 0;
		}
	}

	if ((in = ch[ct].citem) && (it[in].flags & IF_LABYDESTROY))
	{
		it[in].used  = 0;
		ch[ct].citem = 0;
	}

	if (!god_drop_char_fuzzy(ct, x, y))
	{
		do_char_log(cn, 0, "god_drop_char_fuzzy() called from god_thrall() failed.\n");
		god_destroy_items(ct);
		ch[ct].used = USE_EMPTY;
		return -1;
	}
	if (!offset)
	{
		do_char_log(cn, 1, "%s was enthralled.\n", ch[co].name);
		chlog(cn, "IMP: enthralled %s.", ch[co].name);
	}
	return ct;
}

int god_army(int cn, int nmbr, char *spec1, char *spec2)
{
	int n, gt=0, offset=0;
	
	if (nmbr < 1)
	{
		do_char_log(cn, 0, "...How many?\n");
		return 0;
	}
	if (nmbr > 25)
	{
		do_char_log(cn, 0, "That might be TOO many...\n");
		return 0;
	}
	
	if (nmbr > 1) offset++;
	if (nmbr > 9) offset++;
	
	for (n=0;n<nmbr;n++)
	{
		if (god_thrall(cn, spec1, spec2, offset))
			gt++;
		else 
			return 0;
	}
	
	if (gt)
	{
		do_char_log(cn, 1, "Enthralled %d of those.\n", gt);
		chlog(cn, "IMP: enthralled %d monsters.", gt);
	}
}


void god_tavern(int cn)
{
	// DB: tavern does nothing to NPCs... But better be safe than sorry ;-)
	if (IS_USURP(cn))
	{
		do_char_log(cn, 0, "Not with NPCs!\n");
		return;
	}
	if (IS_BUILDING(cn))
	{
		god_build(cn, 0);
	}
	ch[cn].tavern_x = ch[cn].x;
	ch[cn].tavern_y = ch[cn].y;
	chlog(cn, "Entered tavern");
	plr_logout(cn, ch[cn].player, LO_TAVERN);
}

int god_build_start(int cn)
{
	int co, n, in;

	if ((co = ch[cn].data[PCD_COMPANION]))
	{
		do_char_log(cn, 0, "Get rid of %s first.\n",
		            ch[co].name);
		return 0;
	}
	/* Create char to hold inventory */
	co = god_create_char(1, 0);
	if (!co)
	{
		do_char_log(cn, 0, "Could not create item holder for you.\n");
		return 0;
	}
	ch[cn].data[PCD_COMPANION] = co;

	/* Transfer inventory */
	for (n = 0; n<40; n++)
	{
		in = ch[cn].item[n];
		if (in)
		{
			ch[co].item[n] = in;
			ch[cn].item[n] = 0;
			ch[cn].item_lock[n] = 0;
			it[in].carried = co;
		}
	}
	ch[co].citem = ch[cn].citem;
	ch[cn].citem = 0;

	sprintf(ch[co].name, "%s's holder", ch[cn].name);
	god_drop_char(co, 10, 10);

	/* Set build mode flag */
	ch[cn].flags |= CF_BUILDMODE;
	do_update_char(cn);

	return 1;
}

void god_build_stop(int cn)
{
	int co, n, in;

	/* Empty builder's inventory */
	for (n = 0; n<40; n++)
	{
		ch[cn].item[n] = 0;
		ch[cn].item_lock[n] = 0;
	}
	ch[cn].citem = 0;

	/* Reset build mode */
	ch[cn].flags &= ~(CF_BUILDMODE);
	ch[cn].misc_action = DR_IDLE;
	do_char_log(cn, 3, "Now out of build mode.\n");

	/* Retrieve inventory from item holder */
	co = ch[cn].data[PCD_COMPANION];
	if (!co)
	{
		do_char_log(cn, 0, "Could not find your item holder!\n");
		return;
	}

	/* Transfer inventory */
	for (n = 0; n<40; n++)
	{
		in = ch[co].item[n];
		if (in)
		{
			ch[cn].item[n] = in;
			ch[co].item[n] = 0;
			it[in].carried = cn;
		}
	}
	ch[cn].citem = ch[co].citem;
	ch[co].citem = 0;

	/* Destroy item holder */
	plr_map_remove(co);
	ch[co].used = USE_EMPTY;
	ch[cn].data[PCD_COMPANION] = 0;

	do_update_char(cn);
}

void god_build_equip(int cn, int x)
{
	int n, m;

	m = 0;
	switch (x)
	{
	case 0:
		ch[cn].item[m++] = 0x40000000 | MF_MOVEBLOCK;
		ch[cn].item[m++] = 0x40000000 | MF_SIGHTBLOCK;
		ch[cn].item[m++] = 0x40000000 | MF_INDOORS;
		ch[cn].item[m++] = 0x40000000 | MF_ARENA;
		ch[cn].item[m++] = 0x40000000 | MF_NOMONST;
		ch[cn].item[m++] = 0x40000000 | MF_BANK;
		ch[cn].item[m++] = 0x40000000 | MF_TAVERN;
		ch[cn].item[m++] = 0x40000000 | MF_NOMAGIC;
		ch[cn].item[m++] = 0x40000000 | MF_DEATHTRAP;
		ch[cn].item[m++] = 0x40000000 | MF_UWATER;
		
		ch[cn].item[m++] = 0x40000000 | MF_NOLAG;
		ch[cn].item[m++] = 0x40000000 | MF_NOFIGHT;
		ch[cn].item[m++] = 0x40000000 | MF_NOEXPIRE;
		ch[cn].item[m++] = 0x20000000 | SPR_DESERT_GROUND;	//  1002
		ch[cn].item[m++] = 0x20000000 | SPR_GROUND1;		//  1008
		ch[cn].item[m++] = 0x20000000 | SPR_WOOD_GROUND;	//  1013
		ch[cn].item[m++] = 0x20000000 | SPR_TAVERN_GROUND;	//  1034
		ch[cn].item[m++] = 0x20000000 | SPR_STONE_GROUND1;	//  1010
		ch[cn].item[m++] = 0x20000000 | SPR_STONE_GROUND2;	//  1052
		ch[cn].item[m++] = 0x20000000 | 1100;
		
		ch[cn].item[m++] = 0x20000000 | 1099;
		ch[cn].item[m++] = 0x20000000 | 1012;
		ch[cn].item[m++] = 0x20000000 | 1109;
		ch[cn].item[m++] = 0x20000000 | 1118;
		ch[cn].item[m++] = 0x20000000 | 1141;
		ch[cn].item[m++] = 0x20000000 | 1158;
		ch[cn].item[m++] = 0x20000000 | 1145;
		ch[cn].item[m++] = 0x20000000 | 1014;
		ch[cn].item[m++] = 0x20000000 | 1003;
		ch[cn].item[m++] = 0x20000000 | 1005;
		
		ch[cn].item[m++] = 0x20000000 | 1006;
		ch[cn].item[m++] = 0x20000000 | 1007;
		ch[cn].item[m++] = 0x20000000 | 402;
		ch[cn].item[m++] = 0x20000000 | 500;
		ch[cn].item[m++] = 0x20000000 | 558;
		ch[cn].item[m++] = 0x20000000 | 596;
		//
		//
		//
		//
		break;
	case 1:
		ch[cn].item[m++] = 0x20000000 | 520;
		ch[cn].item[m++] = 0x20000000 | 521;
		ch[cn].item[m++] = 0x20000000 | 522;
		ch[cn].item[m++] = 0x20000000 | 523;
		ch[cn].item[m++] = 0x20000000 | 524;
		ch[cn].item[m++] = 0x20000000 | 525;
		ch[cn].item[m++] = 0x20000000 | 526;
		ch[cn].item[m++] = 0x20000000 | 527;
		ch[cn].item[m++] = 0x20000000 | 528;
		ch[cn].item[m++] = 0x20000000 | 529;
		ch[cn].item[m++] = 0x20000000 | 530;
		//
		ch[cn].item[m++] = 0x20000000 | 531;
		ch[cn].item[m++] = 0x20000000 | 532;
		ch[cn].item[m++] = 0x20000000 | 533;
		ch[cn].item[m++] = 0x20000000 | 534;
		ch[cn].item[m++] = 0x20000000 | 535;
		ch[cn].item[m++] = 0x20000000 | 536;
		ch[cn].item[m++] = 0x20000000 | 537;
		ch[cn].item[m++] = 0x20000000 | 538;
		ch[cn].item[m++] = 0x20000000 | 539;
		ch[cn].item[m++] = 0x20000000 | 540;
		ch[cn].item[m++] = 0x20000000 | 541;
		//
		ch[cn].item[m++] = 0x20000000 | 5034;
		ch[cn].item[m++] = 0x20000000 | 5035;
		ch[cn].item[m++] = 0x20000000 | 5036;
		ch[cn].item[m++] = 0x20000000 | 5037;
		ch[cn].item[m++] = 0x20000000 | 5038;
		ch[cn].item[m++] = 0x20000000 | 5039;
		ch[cn].item[m++] = 0x20000000 | 5040;
		ch[cn].item[m++] = 0x20000000 | 5041;
		ch[cn].item[m++] = 0x20000000 | 5042;
		ch[cn].item[m++] = 0x20000000 | 5043;
		ch[cn].item[m++] = 0x20000000 | 5044;
		break;
	case 2:
		ch[cn].item[m++] = 0x20000000 | 542;
		ch[cn].item[m++] = 0x20000000 | 543;
		ch[cn].item[m++] = 0x20000000 | 544;
		ch[cn].item[m++] = 0x20000000 | 545;
		ch[cn].item[m++] = 0x20000000 | 546;
		ch[cn].item[m++] = 0x20000000 | 547;
		ch[cn].item[m++] = 0x20000000 | 548;
		ch[cn].item[m++] = 0x20000000 | 549;
		ch[cn].item[m++] = 0x20000000 | 550;
		ch[cn].item[m++] = 0x20000000 | 551;
		ch[cn].item[m++] = 0x20000000 | 552;
		ch[cn].item[m++] = 0x20000000 | 553;
		ch[cn].item[m++] = 0x20000000 | 554;
		break;
	case 3:
		ch[cn].item[m++] = 0x20000000 | 130;
		ch[cn].item[m++] = 0x20000000 | 131;
		ch[cn].item[m++] = 0x20000000 | 132;
		ch[cn].item[m++] = 0x20000000 | 133;
		ch[cn].item[m++] = 0x20000000 | 134;
		ch[cn].item[m++] = 0x20000000 | 135;
		ch[cn].item[m++] = 0x20000000 | 136;
		ch[cn].item[m++] = 0x20000000 | 137;
		ch[cn].item[m++] = 0x20000000 | 138;
		ch[cn].item[m++] = 0x20000000 | 139;
		ch[cn].item[m++] = 0x20000000 | 140;
		ch[cn].item[m++] = 0x20000000 | 141;
		ch[cn].item[m++] = 0x20000000 | 142;
		ch[cn].item[m++] = 0x20000000 | 143;
		ch[cn].item[m++] = 0x20000000 | 144;
		ch[cn].item[m++] = 0x20000000 | 145;
		break;
	case 4:
		ch[cn].item[m++] = 0x20000000 | 170;
		ch[cn].item[m++] = 0x20000000 | 171;
		ch[cn].item[m++] = 0x20000000 | 172;
		ch[cn].item[m++] = 0x20000000 | 173;
		ch[cn].item[m++] = 0x20000000 | 174;
		ch[cn].item[m++] = 0x20000000 | 175;
		break;
	case 11:	// Grass to Dark Grass
		ch[cn].item[m++] = 0x20000000 | 551;
		ch[cn].item[m++] = 0x20000000 | 1118;
		ch[cn].item[m++] = 0x20000000 | 2721;
		ch[cn].item[m++] = 0x20000000 | 2722;
		ch[cn].item[m++] = 0x20000000 | 2723;
		ch[cn].item[m++] = 0x20000000 | 2724;
		ch[cn].item[m++] = 0x20000000 | 2725;
		ch[cn].item[m++] = 0x20000000 | 2726;
		ch[cn].item[m++] = 0x20000000 | 2727;
		ch[cn].item[m++] = 0x20000000 | 2728;
		ch[cn].item[m++] = 0x20000000 | 2729;
		ch[cn].item[m++] = 0x20000000 | 2730;
		ch[cn].item[m++] = 0x20000000 | 2731;
		ch[cn].item[m++] = 0x20000000 | 2732;
		break;
	case 12:	// Grass to Sand
		ch[cn].item[m++] = 0x20000000 | 551;	// Grass
		ch[cn].item[m++] = 0x20000000 | 130;	// Sand
		ch[cn].item[m++] = 0x20000000 | 2741;	// Grass N, Sand S
		ch[cn].item[m++] = 0x20000000 | 2742;	// Grass NE (U)
		ch[cn].item[m++] = 0x20000000 | 2743;	// Grass E, Sand W
		ch[cn].item[m++] = 0x20000000 | 2744;	// Grass SE (U)
		ch[cn].item[m++] = 0x20000000 | 2745;	// Grass S, Sand N
		ch[cn].item[m++] = 0x20000000 | 2746;	// Grass SW (U)
		ch[cn].item[m++] = 0x20000000 | 2747;	// Grass W, Sand E
		ch[cn].item[m++] = 0x20000000 | 2748;	// Grass NW (U)
		ch[cn].item[m++] = 0x20000000 | 2749;	// Sand NE (U)
		ch[cn].item[m++] = 0x20000000 | 2750;	// Sand SE (U)
		ch[cn].item[m++] = 0x20000000 | 2751;	// Sand SW (U)
		ch[cn].item[m++] = 0x20000000 | 2752;	// Sand NW (U)
		break;
	case 13:	// Black Marble Stuff
		ch[cn].item[m++] = 0x20000000 | 2717;
		ch[cn].item[m++] = 0x20000000 | 531;
		ch[cn].item[m++] = 0x20000000 | 532;
		ch[cn].item[m++] = 0x20000000 | 533;
		ch[cn].item[m++] = 0x20000000 | 534;
		ch[cn].item[m++] = 0x20000000 | 535;
		ch[cn].item[m++] = 0x20000000 | 536;
		ch[cn].item[m++] = 0x20000000 | 537;
		ch[cn].item[m++] = 0x20000000 | 538;
		ch[cn].item[m++] = 0x20000000 | 539;
		//
		ch[cn].item[m++] = 0x20000000 | 540;
		ch[cn].item[m++] = 0x20000000 | 541;
		ch[cn].item[m++] = 0x20000000 | 5864;
		ch[cn].item[m++] = 0x20000000 | 5865;
		ch[cn].item[m++] = 122;
		ch[cn].item[m++] = 123;
		ch[cn].item[m++] = 2646;				// Red Marble
		ch[cn].item[m++] = 2647;
		ch[cn].item[m++] = 2648;
		ch[cn].item[m++] = 2649;
		//
		ch[cn].item[m++] = 2650;
		ch[cn].item[m++] = 2651;
		ch[cn].item[m++] = 2652;				// Green Marble
		ch[cn].item[m++] = 2653;
		ch[cn].item[m++] = 2654;
		ch[cn].item[m++] = 2655;
		ch[cn].item[m++] = 2656;
		ch[cn].item[m++] = 2657;
		break;
	case 14:	// Grass to Dirt
		ch[cn].item[m++] = 0x20000000 | 551;	// Grass
		ch[cn].item[m++] = 0x20000000 | 131;	// Dirt
		ch[cn].item[m++] = 0x20000000 | 3570;	// Grass N, Dirt S
		ch[cn].item[m++] = 0x20000000 | 3571;	// Grass NE (U)
		ch[cn].item[m++] = 0x20000000 | 3572;	// Grass E, Dirt W
		ch[cn].item[m++] = 0x20000000 | 3573;	// Grass SE (U)
		ch[cn].item[m++] = 0x20000000 | 3574;	// Grass S, Dirt N
		ch[cn].item[m++] = 0x20000000 | 3575;	// Grass SW (U)
		ch[cn].item[m++] = 0x20000000 | 3576;	// Grass W, Dirt E
		ch[cn].item[m++] = 0x20000000 | 3577;	// Grass NW (U)
		ch[cn].item[m++] = 0x20000000 | 3578;	// Dirt NE (U)
		ch[cn].item[m++] = 0x20000000 | 3579;	// Dirt SE (U)
		ch[cn].item[m++] = 0x20000000 | 3580;	// Dirt SW (U)
		ch[cn].item[m++] = 0x20000000 | 3581;	// Dirt NW (U)
		break;
	case 15:	// Yellow Grass to Dark Yellow Grass
		ch[cn].item[m++] = 0x20000000 | 2828;	// Grass
		ch[cn].item[m++] = 0x20000000 | 2832;	// DarkG
		ch[cn].item[m++] = 0x20000000 | 2833;	// Grass N, DarkG S
		ch[cn].item[m++] = 0x20000000 | 2834;	// Grass NE (U)
		ch[cn].item[m++] = 0x20000000 | 2835;	// Grass E, DarkG W
		ch[cn].item[m++] = 0x20000000 | 2836;	// Grass SE (U)
		ch[cn].item[m++] = 0x20000000 | 2837;	// Grass S, DarkG N
		ch[cn].item[m++] = 0x20000000 | 2838;	// Grass SW (U)
		ch[cn].item[m++] = 0x20000000 | 2839;	// Grass W, DarkG E
		ch[cn].item[m++] = 0x20000000 | 2840;	// Grass NW (U)
		//
		ch[cn].item[m++] = 0x20000000 | 2841;	// DarkG NE (U)
		ch[cn].item[m++] = 0x20000000 | 2842;	// DarkG SE (U)
		ch[cn].item[m++] = 0x20000000 | 2843;	// DarkG SW (U)
		ch[cn].item[m++] = 0x20000000 | 2844;	// DarkG NW (U)
		ch[cn].item[m++] = 1689;				// ** Deco
		ch[cn].item[m++] = 1690;				// ** Deco
		ch[cn].item[m++] = 1691;				// ** Deco
		break;	
	case 16:	// Snowy & Emerald
		ch[cn].item[m++] = 0x20000000 | 808;	// Grey Stone Floor
		ch[cn].item[m++] = 0x20000000 | 2822;	// Dark Sand
		ch[cn].item[m++] = 0x20000000 | 2823;	// Snow 1
		ch[cn].item[m++] = 0x20000000 | 2827;	// Snow 2
		ch[cn].item[m++] = 0x20000000 | 3021;	// Snow 3
		ch[cn].item[m++] = 0x20000000 | 5965;	// Cold Stone
		ch[cn].item[m++] = 0x20000000 | 5966;	// Cold Stone
		ch[cn].item[m++] = 1675;				// ** Wall
		ch[cn].item[m++] = 1692;				// ** Deco
		ch[cn].item[m++] = 1693;				// ** Deco
		ch[cn].item[m++] = 1694;				// ** Deco
		ch[cn].item[m++] = 1688;				// ** Tree
		ch[cn].item[m++] = 0x20000000 | 5047;	// Emerald Brick Floor
		ch[cn].item[m++] = 0x20000000 | 2658;	// Emerald Cave Floor
		ch[cn].item[m++] = 2208;				// ** 
		ch[cn].item[m++] = 2209;				// ** 
		ch[cn].item[m++] = 2210;				// ** 
		ch[cn].item[m++] = 2211;				// ** 
		ch[cn].item[m++] = 2212;				// ** 
		ch[cn].item[m++] = 2213;				// ** 
		ch[cn].item[m++] = 2214;				// ** 
		ch[cn].item[m++] = 2215;				// ** 
		ch[cn].item[m++] = 2216;				// ** 
		break;	
	case 17:	// Canyon
		ch[cn].item[m++] = 0x40000000 | MF_INDOORS;
		ch[cn].item[m++] = 1674;				// ** Wall
		ch[cn].item[m++] = 1710;				// ** Rope
		ch[cn].item[m++] = 0x20000000 | 3614;
		ch[cn].item[m++] = 0x20000000 | 3615;
		ch[cn].item[m++] = 0x20000000 | 3538;
		ch[cn].item[m++] = 0x20000000 | 3539;
		ch[cn].item[m++] = 0x20000000 | 3540;
		ch[cn].item[m++] = 0x20000000 | 3541;
		ch[cn].item[m++] = 0x20000000 | 3542;
		ch[cn].item[m++] = 0x20000000 | 3543;
		ch[cn].item[m++] = 0x20000000 | 3544;
		ch[cn].item[m++] = 0x20000000 | 3545;
		ch[cn].item[m++] = 0x20000000 | 3546;
		ch[cn].item[m++] = 0x20000000 | 3547;
		ch[cn].item[m++] = 0x20000000 | 3548;
		ch[cn].item[m++] = 0x20000000 | 3549;
		ch[cn].item[m++] = 0x20000000 | 3550;
		ch[cn].item[m++] = 0x20000000 | 3551;
		ch[cn].item[m++] = 0x20000000 | 3552;
		ch[cn].item[m++] = 0x20000000 | 3553;
		ch[cn].item[m++] = 0x20000000 | 3554;
		ch[cn].item[m++] = 0x20000000 | 3555;
		ch[cn].item[m++] = 0x20000000 | 3556;
		ch[cn].item[m++] = 0x20000000 | 3557;
		ch[cn].item[m++] = 0x20000000 | 3558;
		ch[cn].item[m++] = 0x20000000 | 3559;
		ch[cn].item[m++] = 0x20000000 | 3560;
		ch[cn].item[m++] = 0x20000000 | 3561;
		ch[cn].item[m++] = 0x20000000 | 3562;
		ch[cn].item[m++] = 0x20000000 | 3563;
		ch[cn].item[m++] = 0x20000000 | 3564;
		ch[cn].item[m++] = 0x20000000 | 3565;
		ch[cn].item[m++] = 0x20000000 | 3566;
		ch[cn].item[m++] = 0x20000000 | 3567;
		ch[cn].item[m++] = 0x20000000 | 3568;
		ch[cn].item[m++] = 0x20000000 | 3569;
		break;	
	case 18:	// Water border tiles etc
		ch[cn].item[m++] = 0x20000000 | 16933;
		ch[cn].item[m++] = 0x20000000 | 3582;
		ch[cn].item[m++] = 0x20000000 | 3583;
		ch[cn].item[m++] = 0x20000000 | 3584;
		ch[cn].item[m++] = 0x20000000 | 3585;
		ch[cn].item[m++] = 0x20000000 | 3586;
		ch[cn].item[m++] = 0x20000000 | 3587;
		ch[cn].item[m++] = 0x20000000 | 3588;
		ch[cn].item[m++] = 0x20000000 | 3589;
		ch[cn].item[m++] = 0x20000000 | 3590;
		ch[cn].item[m++] = 0x20000000 | 3591;
		ch[cn].item[m++] = 0x20000000 | 3592;
		ch[cn].item[m++] = 0x20000000 | 3600;
		ch[cn].item[m++] = 0x20000000 | 3601;
		ch[cn].item[m++] = 0x20000000 | 3610;
		ch[cn].item[m++] = 1806;				// ** Edge piece
		ch[cn].item[m++] = 1807;				// ** Edge piece
		ch[cn].item[m++] = 1808;				// ** Edge piece
		ch[cn].item[m++] = 1809;				// ** Edge piece
		ch[cn].item[m++] = 1810;				// ** Edge piece
		ch[cn].item[m++] = 1811;				// ** Edge piece
		ch[cn].item[m++] = 1812;				// ** Edge piece
		break;
	case 19:	// Tower
		ch[cn].item[m++] = 3;					// ** Marble
		ch[cn].item[m++] = 8;					// ** Marble Window
		ch[cn].item[m++] = 9;					// ** Marble Window
		ch[cn].item[m++] = 2408;				// ** Marble Window
		ch[cn].item[m++] = 2409;				// ** Marble Window
		ch[cn].item[m++] = 0x20000000 | 5524;
		ch[cn].item[m++] = 0x20000000 | 5525;
		ch[cn].item[m++] = 0x20000000 | 5526;
		ch[cn].item[m++] = 0x20000000 | 5527;
		ch[cn].item[m++] = 0x20000000 | 5528;
		ch[cn].item[m++] = 0x20000000 | 5529;
		ch[cn].item[m++] = 0x20000000 | 5530;
		ch[cn].item[m++] = 0x20000000 | 5531;
		ch[cn].item[m++] = 0x20000000 | 5532;
		ch[cn].item[m++] = 0x20000000 | 5533;
		ch[cn].item[m++] = 0x20000000 | 5534;
		ch[cn].item[m++] = 0x20000000 | 5535;
		ch[cn].item[m++] = 0x20000000 | 5536;
		ch[cn].item[m++] = 0x20000000 | 5537;
		ch[cn].item[m++] = 0x20000000 | 5538;
		ch[cn].item[m++] = 0x20000000 | 5539;
		ch[cn].item[m++] = 0x20000000 | 5540;
		ch[cn].item[m++] = 0x20000000 | 5541;
		ch[cn].item[m++] = 0x20000000 | 5542;
		ch[cn].item[m++] = 0x20000000 | 5543;
		ch[cn].item[m++] = 0x20000000 | 5544;
		ch[cn].item[m++] = 0x20000000 | 5545;
		ch[cn].item[m++] = 0x20000000 | 5546;
		ch[cn].item[m++] = 0x20000000 | 5547;
		ch[cn].item[m++] = 0x20000000 | 5548;
		ch[cn].item[m++] = 0x20000000 | 5549;
		ch[cn].item[m++] = 0x20000000 | 5585;
		break;
	case 20:	// New Brick Walls
		ch[cn].item[m++] = 0x40000000 | MF_INDOORS;
		ch[cn].item[m++] = 0x20000000 | 3021;
		ch[cn].item[m++] = 1662;				// ** Grey
		ch[cn].item[m++] = 1663;				// ** Grey window
		ch[cn].item[m++] = 1664;				// ** M.Grey
		ch[cn].item[m++] = 1665;				// ** M.Grey window
		ch[cn].item[m++] = 1666;				// ** Tan
		ch[cn].item[m++] = 1667;				// ** Tan window
		ch[cn].item[m++] = 1668;				// ** M.Tan
		ch[cn].item[m++] = 1669;				// ** M.Tan window
		ch[cn].item[m++] = 1670;				// ** Red
		ch[cn].item[m++] = 1671;				// ** Red window
		ch[cn].item[m++] = 1672;				// ** M.Red
		ch[cn].item[m++] = 1673;				// ** M.Red window
		break;	
	
	case 25:	// Tables and table-candles
		ch[cn].item[m++] = 82; 	// Rough Table  w/ leg
		ch[cn].item[m++] = 83; 	// Rough Table  no leg
		ch[cn].item[m++] = 81; 	// Rough G.Candle  w/ leg
		ch[cn].item[m++] = 85; 	// Rough G.Candle  no leg
		ch[cn].item[m++] = 175; // Rough B.Candle  w/ leg
		ch[cn].item[m++] = 176; // Rough B.Candle  no leg
		ch[cn].item[m++] = 298; // Smooth Table  w/ leg
		ch[cn].item[m++] = 299; // Smooth Table  no leg
		ch[cn].item[m++] = 26; 	// Smooth G.Candle  w/ leg
		ch[cn].item[m++] = 88; 	// Smooth G.Candle  no leg
		ch[cn].item[m++] = 177; // Smooth B.Candle  w/ leg
		ch[cn].item[m++] = 178; // Smooth B.Candle  no leg
		break;
		
	case 300:
		ch[cn].item[m++] = 0x40000000 | MF_INDOORS;
		ch[cn].item[m++] = 0x20000000 | 116;
		ch[cn].item[m++] = 0x20000000 | 117;
		ch[cn].item[m++] = 0x20000000 | 118;
		ch[cn].item[m++] = 0x20000000 | 704;
		//
		ch[cn].item[m++] = 1465;
		ch[cn].item[m++] = 1441;
		ch[cn].item[m++] = 1449;
		ch[cn].item[m++] = 1457;
		ch[cn].item[m++] = 437;
		//
		ch[cn].item[m++] = 359;
		ch[cn].item[m++] = 332;
		ch[cn].item[m++] = 373;
		ch[cn].item[m++] = 381;
		ch[cn].item[m++] = 443;
		//
		ch[cn].item[m++] = 371;
		ch[cn].item[m++] = 389;
		ch[cn].item[m++] = 397;
		ch[cn].item[m++] = 405;
		ch[cn].item[m++] = 438;
		//
		ch[cn].item[m++] = 372;
		ch[cn].item[m++] = 413;
		ch[cn].item[m++] = 421;
		ch[cn].item[m++] = 429;
		ch[cn].item[m++] = 439;
		//
		ch[cn].item[m++] = 443;
		ch[cn].item[m++] = 444;
		ch[cn].item[m++] = 445;
		ch[cn].item[m++] = 446;
		ch[cn].item[m++] = 447;
		//
		ch[cn].item[m++] = 448;
		ch[cn].item[m++] = 450;
		ch[cn].item[m++] = 451;
		break;
	case 301:
		ch[cn].item[m++] = 0x20000000 | 704;
		ch[cn].item[m++] = 0x20000000 | 705;
		ch[cn].item[m++] = 0x20000000 | 706;
		ch[cn].item[m++] = 0x20000000 | 707;
		ch[cn].item[m++] = 0x20000000 | 708;
		ch[cn].item[m++] = 0x20000000 | 709;
		ch[cn].item[m++] = 0x20000000 | 710;
		ch[cn].item[m++] = 0x20000000 | 711;
		ch[cn].item[m++] = 0x20000000 | 712;
		ch[cn].item[m++] = 0x20000000 | 713;
		//
		ch[cn].item[m++] = 0x20000000 | 714;
		ch[cn].item[m++] = 0x20000000 | 715;
		ch[cn].item[m++] = 0x20000000 | 716;
		ch[cn].item[m++] = 0x20000000 | 717;
		ch[cn].item[m++] = 0x20000000 | 718;
		ch[cn].item[m++] = 0x20000000 | 719;
		ch[cn].item[m++] = 0x20000000 | 720;
		ch[cn].item[m++] = 0x20000000 | 721;
		ch[cn].item[m++] = 0x20000000 | 722;
		ch[cn].item[m++] = 0x20000000 | 723;
		//
		ch[cn].item[m++] = 0x20000000 | 724;
		ch[cn].item[m++] = 0x20000000 | 725;
		ch[cn].item[m++] = 0x20000000 | 726;
		ch[cn].item[m++] = 0x20000000 | 727;
		ch[cn].item[m++] = 0x20000000 | 728;
		break;
	case 302:		// Jungle-fied lava
		ch[cn].item[m++] = 0x20000000 | 2845;
		ch[cn].item[m++] = 0x20000000 | 2846;
		ch[cn].item[m++] = 0x20000000 | 2847;
		ch[cn].item[m++] = 0x20000000 | 2848;
		ch[cn].item[m++] = 0x20000000 | 2849;
		ch[cn].item[m++] = 0x20000000 | 2850;
		ch[cn].item[m++] = 0x20000000 | 2851;
		ch[cn].item[m++] = 0x20000000 | 2852;
		ch[cn].item[m++] = 0x20000000 | 2853;
		ch[cn].item[m++] = 0x20000000 | 2854;
		ch[cn].item[m++] = 0x20000000 | 2855;
		ch[cn].item[m++] = 0x20000000 | 2856;
		ch[cn].item[m++] = 0x20000000 | 2857;
		ch[cn].item[m++] = 0x20000000 | 2858;
		ch[cn].item[m++] = 0x20000000 | 2859;
		ch[cn].item[m++] = 0x20000000 | 2860;
		ch[cn].item[m++] = 0x20000000 | 2861;
		ch[cn].item[m++] = 0x20000000 | 2862;
		ch[cn].item[m++] = 0x20000000 | 2863;
		break;
	case 700:       // black stronghold
		ch[cn].item[m++] = 0x40000000 | MF_INDOORS;
		ch[cn].item[m++] = 0x20000000 | 950;
		ch[cn].item[m++] = 0x20000000 | 959;
		ch[cn].item[m++] = 0x20000000 | 16652;
		ch[cn].item[m++] = 0x20000000 | 16653;
		ch[cn].item[m++] = 0x20000000 | 16654;
		ch[cn].item[m++] = 0x20000000 | 16655;
		break;
	case 701:
		for (n = 0; n<40; n++)
		{
			ch[cn].item[m++] = 0x20000000 | (n + 16430);
		}
		break;
	case 702:
		for (n = 40; n<78; n++)
		{
			ch[cn].item[m++] = 0x20000000 | (n + 16430);
		}
		break;
	case 703:
		for (n = 16584; n<16599; n++)
		{
			ch[cn].item[m++] = 0x20000000 | n;
		}
		break;
	case 704:
		for (n = 985; n<989; n++)
		{
			ch[cn].item[m++] = 0x20000000 | n;
		}
		break;
	case 705:
		ch[cn].item[m++] = 0x20000000 | 1118;
		ch[cn].item[m++] = 0x20000000 | 989;
		for (n = 16634; n<16642; n++)
		{
			ch[cn].item[m++] = 0x20000000 | n;
		}
		break;
	case 819:
		ch[cn].item[m++] = 0x40000000 | MF_INDOORS;
		ch[cn].item[m++] = 0x20000000 | 16728;
		break;
	case 900:       // graveyard quest
		ch[cn].item[m++] = 0x20000000 | 16933; // lost souls tile
		ch[cn].item[m++] = 0x20000000 | 16934; // grave
		ch[cn].item[m++] = 0x20000000 | 16937; // grave, other dir
		break;
	case 1000:
		ch[cn].item[m++] = 0x40000000 | MF_INDOORS;
		ch[cn].item[m++] = 0x20000000 | 1014;
		ch[cn].item[m++] = 0x20000000 | 704;
		ch[cn].item[m++] = 0x20000000 | 662;
		ch[cn].item[m++] = 0x20000000 | 16689;
		//ch[cn].item[m++]=150;
		//ch[cn].item[m++]=171;
		//ch[cn].item[m++]=152;
		//ch[cn].item[m++]=153;
		//ch[cn].item[m++]=154;
		//ch[cn].item[m++]=155;
		//ch[cn].item[m++]=156;
		//ch[cn].item[m++]=157;
		//ch[cn].item[m++]=520;
		//ch[cn].item[m++]=521;
		//ch[cn].item[m++]=504;
		//ch[cn].item[m++]=505;
		//ch[cn].item[m++]=506;
		//ch[cn].item[m++]=507;
		ch[cn].item[m++] = 508;
		ch[cn].item[m++] = 509;
		ch[cn].item[m++] = 510;
		ch[cn].item[m++] = 511;
		ch[cn].item[m++] = 512;
		ch[cn].item[m++] = 513;
		ch[cn].item[m++] = 514;
		ch[cn].item[m++] = 515;
		ch[cn].item[m++] = 516;
		ch[cn].item[m++] = 517;
		ch[cn].item[m++] = 518;
		ch[cn].item[m++] = 519;
		ch[cn].item[m++] = 522;
		break;
	case 1001:
		ch[cn].item[m++] = 0x40000000 | MF_INDOORS; // Indoor flag
		ch[cn].item[m++] = 0x20000000 | 551;		// Light Grass
		ch[cn].item[m++] = 0x20000000 | 1118;		// Dark Grass
		ch[cn].item[m++] = 0x20000000 | 5852;		// Light Grass
		ch[cn].item[m++] = 0x20000000 | 5856;		// Dark Grass
		ch[cn].item[m++] = 0x20000000 | 5848;		// Cherry Grass
		ch[cn].item[m++] = 1467;	// Random Town Tree
		ch[cn].item[m++] = 1468;	// Random Forest Tree
		ch[cn].item[m++] = 17;		// Light moss
		ch[cn].item[m++] = 47;		// Dark moss
		//
		ch[cn].item[m++] = 1558;	// Tall Grass I
		ch[cn].item[m++] = 1559;	// Tall Grass II
		ch[cn].item[m++] = 1560;	// Tall Grass III
		ch[cn].item[m++] = 1583;	// Tall Grass IV
		ch[cn].item[m++] = 1584;	// Tall Grass V
		ch[cn].item[m++] = 1585;	// Tall Grass VI
		ch[cn].item[m++] = 1589;	// Pebbles
		ch[cn].item[m++] = 16;		// Red Flower
		ch[cn].item[m++] = 45;		// Purple Flower
		ch[cn].item[m++] = 139;		// Green Flower
		//
		ch[cn].item[m++] = 1956;	// White Flower
		ch[cn].item[m++] = 137;		// Yellow Flower
		ch[cn].item[m++] = 138;		// Blue Flower
		ch[cn].item[m++] = 19;		// Tree I
		ch[cn].item[m++] = 20;		// Tree II
		ch[cn].item[m++] = 48;		// Tree III
		ch[cn].item[m++] = 49;		// Tree IV
		ch[cn].item[m++] = 606;		// Tree V
		ch[cn].item[m++] = 607;		// Tree VI
		ch[cn].item[m++] = 608;		// Tree VII
		//
		ch[cn].item[m++] = 609;		// Tree VIII
		ch[cn].item[m++] = 2645;	// Cherry Tree
		ch[cn].item[m++] = 1434;	// Apple Tree
		ch[cn].item[m++] = 1436;	// Pear Tree
		ch[cn].item[m++] = 1438;	// Berry Bush
		ch[cn].item[m++] = 1440;	// Bush
		ch[cn].item[m++] = 1591;	// Tall Bush
		break;
	case 1002:   // ice penta
		ch[cn].item[m++] = 0x40000000 | MF_INDOORS;
		ch[cn].item[m++] = 0x20000000 | 16670;
		ch[cn].item[m++] = 800;
		ch[cn].item[m++] = 801;
		ch[cn].item[m++] = 802;
		ch[cn].item[m++] = 803;
		ch[cn].item[m++] = 804;
		ch[cn].item[m++] = 805;
		ch[cn].item[m++] = 806;
		ch[cn].item[m++] = 807;
		ch[cn].item[m++] = 808;
		ch[cn].item[m++] = 809;
		ch[cn].item[m++] = 810;
		ch[cn].item[m++] = 811;
		ch[cn].item[m++] = 812;
		break;
	case    1003:
		ch[cn].item[m++] = 0x20000000 | 16980;
		break;
	case    1004:
		ch[cn].item[m++] = 0x40000000 | MF_INDOORS; // Indoor flag
		ch[cn].item[m++] = 0x20000000 | 2828;		// Light Grass
		ch[cn].item[m++] = 0x20000000 | 2832;		// Dark Grass
		ch[cn].item[m++] = 1695;	// Random Town Tree
		ch[cn].item[m++] = 1696;	// Random Forest Tree
		ch[cn].item[m++] = 1689;
		ch[cn].item[m++] = 1690;
		ch[cn].item[m++] = 1691;
		ch[cn].item[m++] = 1680;
		ch[cn].item[m++] = 1681;
		//
		ch[cn].item[m++] = 1682;
		ch[cn].item[m++] = 1683;
		ch[cn].item[m++] = 1684;
		ch[cn].item[m++] = 1685;
		ch[cn].item[m++] = 1686;
		ch[cn].item[m++] = 1687;
		break;
	case 1005:
		ch[cn].item[m++] = 0x40000000 | MF_INDOORS; // Indoor flag
		ch[cn].item[m++] = 0x20000000 | 5860;		// Violet Jungle Floor
		ch[cn].item[m++] = 2658;
		ch[cn].item[m++] = 2659;
		ch[cn].item[m++] = 2660;
		ch[cn].item[m++] = 2661;
		ch[cn].item[m++] = 2662;
		ch[cn].item[m++] = 2663;
		ch[cn].item[m++] = 2664;
		ch[cn].item[m++] = 2665;
		//
		ch[cn].item[m++] = 2666;
		ch[cn].item[m++] = 2667;
		ch[cn].item[m++] = 2668;
		break;
	case    1140:
		ch[cn].item[m++] = 0x20000000 | 17064;
		ch[cn].item[m++] = 0x20000000 | 17065;
		ch[cn].item[m++] = 0x20000000 | 17066;
		ch[cn].item[m++] = 0x20000000 | 17067;
		break;
	}

	if (x<1)
	{
		x = 1;
	}

	/* fill inventory with other stuff upward from last item */
	for (n = x; n<MAXTITEM && m<40; n++)
	{
		if (it_temp[n].used==USE_EMPTY)
		{
			continue;
		}
		if (it_temp[n].flags & IF_TAKE)
		{
			continue;
		}
		if (it_temp[n].driver==25 && it_temp[n].data[3]==0)
		{
			continue;
		}
		if (it_temp[n].driver==22)
		{
			continue;
		}
		ch[cn].item[m] = n;
		m++;
	}

	chlog(cn, "Now in build mode %d", x);
	do_char_log(cn, 3, "Build mode %d\n", x);
}

void god_build(int cn, int x)
{

	if (!IS_BUILDING(cn))
	{
		// Remove GC-to-me to prevent weird holder teleport behavior
		if (ch[cn].flags & CF_GCTOME)
		{
			ch[cn].flags ^= CF_GCTOME;
		}
		
		// build from non-build mode: start
		if (god_build_start(cn))
		{
			god_build_equip(cn, x);
		}
	}
	else if (!x)
	{
		// build 0 from build mode: stop
		god_build_stop(cn);
	}
	else
	{
		// switch to mode x
		god_build_equip(cn, x);
	}
}

void god_raise_char(int cn, int co, int v, int bsp)
{
	if (co == 0)
	{
		do_char_log(cn, 0, "No such character.\n");
		return;
	}
	else if (!IS_SANECHAR(co))
	{
		do_char_log(cn, 0, "Bad character number: %d\n", co);
		return;
	}
	else if (v < 0)
	{
		do_char_log(cn, 0, "Raising by a negative amount??\n");
		return;
	}
	
	if (bsp)
	{
		ch[co].bs_points += v;

		do_char_log(cn, 1, "Awarded %s %d Stronghold Points.\n", ch[co].name, v);
		chlog(cn, "IMP: Awarded %s %d Stronghold Points.", ch[co].name, v);
		do_char_log(co, 0, "You have been rewarded by the gods. You received %d black stronghold points.\n", v);
	}
	else
	{
		ch[co].points += v;
		ch[co].points_tot += v;

		do_char_log(cn, 1, "Raised %s by %d.\n", ch[co].name, v);
		chlog(cn, "IMP: Raised %s by %d.", ch[co].name, v);
		do_char_log(co, 0, "You have been rewarded by the gods. You received %d experience points.\n", v);

		do_check_new_level(co, 1);
	}
}

void god_lower_char(int cn, int co, int v)
{
	if (co == 0)
	{
		do_char_log(cn, 0, "No such character.\n");
		return;
	}
	else if (!IS_SANECHAR(co))
	{
		do_char_log(cn, 0, "Bad character number: %d\n", co);
		return;
	}
	else if (v < 0)
	{
		do_char_log(cn, 0, "Lowering by a negative amount??\n");
	}
	ch[co].points_tot -= v;
	ch[co].points -= v;

	do_char_log(cn, 1, "Lowered %s by %d.\n", ch[co].name, v);
	chlog(cn, "IMP: Lowered %s by %d.", ch[co].name, v);
	do_char_log(co, 0, "You have been punished by the gods. You lost %d experience points.\n", v);
}

/* CS, 991127: Now takes either <silver> or <gold> <silver> */
void god_gold_char(int cn, int co, int v, char *silver)
{
	if (co == 0)
	{
		do_char_log(cn, 0, "No such character.\n");
		return;
	}
	else if (!IS_SANECHAR(co))
	{
		do_char_log(cn, 0, "Bad character number: %d\n", co);
		return;
	}
	else if (v < 0)
	{
		do_char_log(cn, 0, "Handing out negative gold?\n");
		return;
	}
	else if (*silver && (atoi(silver) < 0))
	{
		do_char_log(cn, 0, "Handing out negative silver?\n");
		return;
	}
	if (*silver)
	{
		v = v * 100 + atoi(silver);
	}
	ch[co].gold += v;

	do_char_log(cn, 1, "Added %dG %dS to %s.\n", v / 100, v % 100, ch[co].name);
	chlog(cn, "IMP: Added %dG %dS to %s.", v / 100, v % 100, ch[co].name);
	do_char_log(co, 0, "You have been rewarded by the gods. You received %dG %dS.\n", v / 100, v % 100);

	do_update_char(co);
}

void god_erase(int cn, int co, int erase_player)
{
	if (co == 0)
	{
		do_char_log(cn, 0, "No such character.\n");
		return;
	}
	else if (!IS_SANECHAR(co))
	{
		do_char_log(cn, 0, "Bad character number: %d\n", co);
		return;
	}
	else if (!IS_USEDCHAR(co))
	{
		do_char_log(cn, 0, "Character %d is unused anyway.\n", co);
		return;
	}

	if ((ch[co].flags & (CF_PLAYER | CF_USURP)) && !erase_player)
	{
		do_char_log(cn, 0, "%-.20s is a player or QM; use #PERASE if you insist.\n", ch[co].name);
		return;
	}
	if (erase_player && !(ch[co].flags & (CF_PLAYER | CF_USURP)))
	{
		do_char_log(cn, 0, "%-.20s is not a player; use #ERASE for NPCs.\n", ch[co].name);
		return;
	}
	if (erase_player)
	{
		if (ch[co].player)
		{
			plr_logout(co, ch[co].player, LO_SHUTDOWN);
		}
		ch[co].used = USE_EMPTY;
		chlog(cn, "IMP: Erased player %d (%-.20s).", co, ch[co].name);
		do_char_log(cn, 1, "Player %d (%-.20s) is no more.\n", co, ch[co].name);
	}
	else
	{
		do_char_killed(0, co, 0);
		chlog(cn, "IMP: Erased NPC %d (%-.20s).", co, ch[co].name);
		do_char_log(cn, 1, "NPC %d (%-.20s) is no more.\n", co, ch[co].name);
	}
}

void god_kick(int cn, int co)
{
	if (co == 0)
	{
		do_char_log(cn, 0, "No such character.\n");
		return;
	}
	else if (!IS_SANECHAR(co))
	{
		do_char_log(cn, 0, "Bad character number: %d\n", co);
		return;
	}
	else if (!IS_USEDCHAR(co) || !IS_PLAYER(co))
	{
		do_char_log(cn, 0, "Character %d is not a player!\n", co);
		return;
	}
	plr_logout(co, ch[co].player, LO_IDLE);
	do_char_log(cn, 1, "Kicked %s.\n", ch[co].name);
	chlog(cn, "IMP: kicked %s (%d)", ch[co].name, co);
	ch[co].flags |= CF_KICKED;
}

void god_skill(int cn, int co, int n, int val)
{
	if (n == -1)   // bad skill number
	{
		return;
	}
	else if (co == 0)
	{
		do_char_log(cn, 0, "No such character.\n");
		return;
	}
	else if (!IS_SANECHAR(co))
	{
		do_char_log(cn, 0, "Bad character number: %d\n", co);
		return;
	}
	else if (!IS_USEDCHAR(co))
	{
		do_char_log(cn, 0, "Character %d is not in the game!\n", co);
		return;
	}
	else if (!IS_SANESKILL(n))
	{
		do_char_log(cn, 0, "Skill number %d out of range.\n", n);
		return;
	}
	else if (val<0 || val>250)
	{
		do_char_log(cn, 0, "Skill amount %d out of range.\n", val);
		return;
	}
	B_SK(co, n) = val;
	do_update_char(co);

	do_char_log(cn, 1, "Set %s of %s to %d.\n", skilltab[n].name, ch[co].name, val);
	chlog(cn, "IMP: Set %s of %s to %d.", skilltab[n].name, ch[co].name, val);
}

void god_donate_item(int in, int place)
{
	static int don_x[] = {760, 505, 579}; // Tavern X, Skua X, Purple X
	static int don_y[] = {261, 512, 455}; // Tavern Y, Skua Y, Purple Y
	int x, y;

	if (!IS_SANEUSEDITEM(in))
	{
		xlog("Attempt to god_donate_item %d", in);
		return;
	}
	if (place<1 || place>3)
	{
		place = RANDOM(2)+1;
	}

	x = don_x[place];
	y = don_y[place];

	if (!god_drop_item_fuzzy(in, x, y))
	{
		it[in].used = USE_EMPTY;
	}
}

void god_set_flag(int cn, int co, unsigned long long flag)
{
	char *ptr = "unknown";

	if (!co || !IS_SANECHAR(co))
	{
		do_char_log(cn, 0, "Character %d does not exist, dude!\n", co);
		return;
	}

	ch[co].flags ^= flag;

	switch (flag)
	{
	case    CF_IMMORTAL:
		ptr = "immortal";
		break;
	case    CF_GOD:
		ptr = "god";
		break;
	case    CF_CREATOR:
		ptr = "creator";
		break;
	case    CF_BUILDMODE:
		ptr = "buildmode";
		break;
	case    CF_RESPAWN:
		ptr = "respawn";
		break;
	case    CF_PLAYER:
		ptr = "player";
		break;
	case    CF_LOCKPICK:
		ptr = "lockpick";
		break;
	case    CF_CCP:
		ptr = "ccp";
		break;
	case    CF_NEWUSER:
		ptr = "newuser";
		break;
	case    CF_NOTELL:
		ptr = "notell";
		break;
	case    CF_NOSHOUT:
		ptr = "noshout";
		break;
	case    CF_MERCHANT:
		ptr = "merchant";
		break;
	case    CF_STAFF:
		ptr = "staff";
		break;
	case    CF_NOHPREG:
		ptr = "nohpreg";
		break;
	case    CF_NOENDREG:
		ptr = "noendreg";
		break;
	case    CF_NOMANAREG:
		ptr = "nomanareg";
		break;
	case    CF_INVISIBLE:
		ptr = "invisible";
		fx_add_effect(12, 0, ch[co].x, ch[co].y, 0);
		break;
	case    CF_INFRARED:
		ptr = "infrared";
		break;
	case    CF_BODY:
		ptr = "body";
		break;
	case    CF_UNDEAD:
		ptr = "undead";
		break;
	case    CF_NOMAGIC:
		ptr = "nomagic";
		break;
	case    CF_STONED:
		ptr = "stoned";
		break;
	case    CF_USURP:
		ptr = "usurp";
		break;
	case    CF_IMP:
		ptr = "imp";
		break;
	case    CF_SHUTUP:
		ptr = "shutup";
		break;
	case    CF_NODESC:
		ptr = "nodesc";
		break;
	case    CF_PROF:
		ptr = "prof";
		break;
	case    CF_NOLIST:
		ptr = "nolist";
		break;
	case    CF_NOWHO:
		ptr = "nowho";
		break;
	case    CF_SAFE:
		ptr = "safe";
		break;
	case    CF_POH:
		ptr = "purple of honor";
		break;
	case    CF_POH_LEADER:
		ptr = "poh leader";
		break;
	case    CF_GOLDEN:
		ptr = "golden list";
		break;
	case    CF_BLACK:
		ptr = "black list";
		break;
	case    CF_GREATERGOD:
		ptr = "greater god";
		break;
	case    CF_GREATERINV:
		ptr = "greater inv";
		if (ch[cn].flags & CF_GREATERINV)
		{
			ch[cn].flags |= CF_INVISIBLE;
			ptr = "greater inv & invisible";
		}
		break;

	default:
		ptr = "unknown";
		break;
	}

	chlog(cn, "IMP: set %s (%llX) on %s (%d) to %s", ptr, flag, ch[co].name, co, (ch[co].flags & flag) ? "on" : "off");
	do_char_log(cn, 3, "Set %s on %s to %s.\n", ptr, ch[co].name, (ch[co].flags & flag) ? "on" : "off");

	if (flag==CF_STAFF && ch[co].temple_x!=HOME_START_X)
	{
		if (IS_PURPLE(co))
		{
			ch[co].temple_x = ch[co].tavern_x = HOME_PURPLE_X;
			ch[co].temple_y = ch[co].tavern_y = HOME_PURPLE_Y;
		}
		else if (ch[co].flags & CF_STAFF)
		{
			ch[co].temple_x = ch[co].tavern_x = HOME_STAFF_X;
			ch[co].temple_y = ch[co].tavern_y = HOME_STAFF_Y;
		}
		else
		{
			ch[co].temple_x = ch[co].tavern_x = HOME_START_X;
			ch[co].temple_y = ch[co].tavern_y = HOME_START_Y;
		}
	}
}

void god_set_gflag(int cn, int flag)
{
	char *ptr = "unknown";

	globs->flags ^= flag;

	switch(flag)
	{
	case    GF_LOOTING:
		ptr = "looting";
		break;
	case    GF_MAYHEM:
		ptr = "mayhem";
		break;
	case    GF_CAP:
		ptr = "cap";
		break;
	case    GF_SPEEDY:
		ptr = "speedy";
		break;
	case    GF_NEWBS:
		ptr = "newbs";
		break;
	case    GF_DISCORD:
		ptr = "discord integration";
		break;
	case	GF_STRONGHOLD:
		ptr = "stronghold";
		break;

	default:
		ptr = "unknown";
		break;
	}

	chlog(cn, "IMP: set %s (%X) to %s", ptr, flag, (globs->flags & flag) ? "on" : "off");
	do_char_log(cn, 3, "Set %s to %s.\n", ptr, (globs->flags & flag) ? "on" : "off");
}

/* toggle purple (PK) status */
void god_set_purple(int cn, int co)
{
	if (co == 0)
	{
		do_char_log(cn, 0, "No such character.\n");
		return;
	}
	else if (!IS_SANECHAR(co))
	{
		do_char_log(cn, 0, "Bad character number: %d\n", co);
		return;
	}
	ch[co].kindred ^= KIN_PURPLE;
	do_char_log(cn, 1, "%s purple: %s.\n", ch[co].name, (IS_PURPLE(co)) ? "on" : "off");
	chlog(cn, "IMP: Set purple status of %s to %s.", ch[co].name, (IS_PURPLE(co)) ? "on" : "off");
	/* CS, 000209: Reset last kill time and victim */
	if (!IS_PURPLE(co))
	{
		//ch[co].data[PCD_ATTACKTIME] = 0;
		//ch[co].data[PCD_ATTACKVICT] = 0;
		ch[co].temple_x = HOME_TEMPLE_X;
		ch[co].temple_y = HOME_TEMPLE_Y;
	}
	else
	{
		ch[co].temple_x = HOME_PURPLE_X;
		ch[co].temple_y = HOME_PURPLE_Y;
	}
}

void god_set_kwai(int cn, int co)
{
	if (co == 0)
	{
		do_char_log(cn, 0, "No such character.\n");
		return;
	}
	else if (!IS_SANECHAR(co))
	{
		do_char_log(cn, 0, "Bad character number: %d\n", co);
		return;
	}
	ch[co].kindred ^= KIN_CLANKWAI;
	do_char_log(cn, 1, "%s kwai: %s.\n", ch[co].name, (IS_CLANKWAI(co)) ? "on" : "off");
	chlog(cn, "IMP: Set kwai status of %s to %s.", ch[co].name, (IS_CLANKWAI(co)) ? "on" : "off");
	if (!IS_CLANKWAI(co))
	{
		ch[co].data[PCD_ATTACKTIME] = 0;
		ch[co].data[PCD_ATTACKVICT] = 0;
		ch[co].temple_x = HOME_TEMPLE_X;
		ch[co].temple_y = HOME_TEMPLE_Y;
	}
	else // change this to kwai temple later
	{
		ch[co].temple_x = HOME_TEMPLE_X;
		ch[co].temple_y = HOME_TEMPLE_Y;
	}
}

void god_set_gorn(int cn, int co)
{
	if (co == 0)
	{
		do_char_log(cn, 0, "No such character.\n");
		return;
	}
	else if (!IS_SANECHAR(co))
	{
		do_char_log(cn, 0, "Bad character number: %d\n", co);
		return;
	}
	ch[co].kindred ^= KIN_CLANGORN;
	do_char_log(cn, 1, "%s gorn: %s.\n", ch[co].name, (IS_CLANGORN(co)) ? "on" : "off");
	chlog(cn, "IMP: Set gorn status of %s to %s.", ch[co].name, (IS_CLANGORN(co)) ? "on" : "off");
	if (!IS_CLANGORN(co))
	{
		ch[co].data[PCD_ATTACKTIME] = 0;
		ch[co].data[PCD_ATTACKVICT] = 0;
		ch[co].temple_x = HOME_TEMPLE_X;
		ch[co].temple_y = HOME_TEMPLE_Y;
	}
	else // change this to kwai temple later
	{
		ch[co].temple_x = HOME_TEMPLE_X;
		ch[co].temple_y = HOME_TEMPLE_Y;
	}
}

void god_destroy_items(int cn)
{
	int n, in;

	for (n = 0; n<40; n++)
	{
		if ((in = ch[cn].item[n])!=0)
		{
			ch[cn].item[n] = 0;
			ch[cn].item_lock[n] = 0;
			if (in>0 && in<MAXITEM)
			{
				it[in].used = USE_EMPTY;
			}
		}
	}
	for (n = 0; n<20; n++)
	{
		if ((in = ch[cn].worn[n])!=0)
		{
			ch[cn].worn[n] = 0;
			if (in>0 && in<MAXITEM)
			{
				it[in].used = USE_EMPTY;
			}
		}
	}
	for (n = 0; n<12; n++)
	{
		if ((in = ch[cn].alt_worn[n])!=0)
		{
			ch[cn].alt_worn[n] = 0;
			if (in>0 && in<MAXITEM)
			{
				it[in].used = USE_EMPTY;
			}
		}
	}
	for (n = 0; n<MAXBUFFS; n++)
	{
		if ((in = ch[cn].spell[n])!=0)
		{
			ch[cn].spell[n] = 0;
			if (in>0 && in<MAXITEM)
			{
				bu[in].used = USE_EMPTY;
			}
		}
	}
	if ((in = ch[cn].citem)!=0)
	{
		ch[cn].citem = 0;
		if (in>0 && in<MAXITEM)
		{
			it[in].used = USE_EMPTY;
		}
	}
	if (ch[cn].flags & CF_PLAYER)
	{
		for (n = 0; n<62; n++)
		{
			if ((in = ch[cn].depot[n])!=0)
			{
				ch[cn].depot[n] = 0;
				if (in>0 && in<MAXITEM)
				{
					it[in].used = USE_EMPTY;
				}
			}
		}
	}
	do_update_char(cn);
}

void god_racechange(int co, int temp, int keepstuff)
{
	int n, rank;
	struct character old, dpt;

	if (!IS_SANEUSEDCHAR(co) || !IS_PLAYER(co))
	{
		return;
	}

	if (!keepstuff)
	{
		dpt = ch[co];
		
		for (n = 0; n<62; n++)
		{
			ch[co].depot[n] = 0;
		}
		
		god_destroy_items(co);
		
		for (n = 0; n<62; n++)
		{
			ch[co].depot[n] = dpt.depot[n];
			it[ch[co].depot[n]].carried = co;
		}
	}
	
	old = ch[co];
	ch[co] = ch_temp[temp];

	ch[co].temp      = temp;
	ch[co].pass1     = old.pass1;
	ch[co].pass2     = old.pass2;
	ch[co].gold      = old.gold;
	ch[co].bs_points = old.bs_points;
	ch[co].os_points = old.os_points;
	ch[co].tokens    = old.tokens;

	strcpy(ch[co].name, old.name);
	strcpy(ch[co].reference, old.name);
	strcpy(ch[co].description, old.description);

	ch[co].dir = old.dir;

	ch[co].temple_x = ch[co].tavern_x = HOME_TEMPLE_X;
	ch[co].temple_y = ch[co].tavern_y = HOME_TEMPLE_Y;

	ch[co].creation_date = old.creation_date;
	ch[co].login_date = old.login_date;
	ch[co].flags = old.flags;
	if (old.kindred & KIN_PURPLE)
	{
		ch[co].kindred |= KIN_PURPLE;
		ch[co].temple_x = HOME_PURPLE_X;
		ch[co].temple_y = HOME_PURPLE_Y;
	}
	ch[co].total_online_time = old.total_online_time;
	ch[co].current_online_time = old.current_online_time;
	ch[co].comp_volume = old.comp_volume;
	ch[co].raw_volume  = old.raw_volume;
	ch[co].idle = old.idle;
	
	ch[co].x = old.x;
	ch[co].y = old.y;
	ch[co].tox = old.tox;
	ch[co].toy = old.toy;
	ch[co].frx = old.frx;
	ch[co].fry = old.fry;

	ch[co].mode = old.mode;
	ch[co].used = USE_ACTIVE;
	ch[co].player = old.player;
	ch[co].alignment = 0;
	ch[co].luck    = old.luck;
	ch[co].light   = old.light;
	ch[co].status  = old.status;
	ch[co].status2 = old.status2;

	for (n = 0; n<MAXBUFFS; n++) ch[co].spell[n] = 0;
	for (n = 0; n<100; n++) ch[co].data[n] = old.data[n];
	for (n = 0; n<62; n++) { ch[co].depot[n] = old.depot[n]; it[ch[co].depot[n]].carried = co; }

	if (keepstuff)
	{
		int hp = 0, mana = 0, attri = 0, exp = 0;
		
		exp = old.points_tot;
		do_char_log(co, 2, "Old exp was %d.\n", exp);
		ch[co].points_tot = exp;
		ch[co].points     = exp;
		rank = points2rank(exp);
		
		if (IS_ANY_MERC(co) || IS_SEYAN_DU(co) || IS_BRAVER(co))
		{
			hp   = 10;
			mana = 10;
		}
		else if (IS_ANY_TEMP(co))
		{
			hp   = 15;
			mana = 5;
		}
		else if (IS_ANY_HARA(co))
		{
			hp   = 5;
			mana = 15;
		}
		if (rank >= 20)
		{
			attri = 1;
		}
		
		ch[co].data[45] = rank;
		
		ch[co].hp[1]   = hp * min(20,rank);
		ch[co].mana[1] = mana * min(20,rank);
		if (attri)
		{
			temp = ch[co].temp;
			for (n = 0; n<5; n++) 
				ch[co].attrib[n][2] = ch_temp[temp].attrib[n][2] + attri * min(5, max(0,rank-19));
		}
		
		for (n = 0; n<40; n++) {ch[co].item[n] = old.item[n]; it[ch[co].item[n]].carried = co;}
		for (n = 0; n<20; n++) {ch[co].worn[n] = old.worn[n]; it[ch[co].worn[n]].carried = co;}
		for (n = 0; n<12; n++) {ch[co].alt_worn[n] = old.alt_worn[n]; it[ch[co].alt_worn[n]].carried = co;}
		
		// Re-acquire previously learned skills
		if (old.skill[ 5][0] && ch[co].skill[ 5][2]) ch[co].skill[ 5][0] = 1; // Staff
		if (old.skill[10][0] && ch[co].skill[10][2]) ch[co].skill[10][0] = 1; // Swimming
		if (old.skill[12][0] && ch[co].skill[12][2]) ch[co].skill[12][0] = 1; // Barter
		if (old.skill[13][0] && ch[co].skill[13][2]) ch[co].skill[13][0] = 1; // Repair
		if (old.skill[48][0] && ch[co].skill[48][2]) ch[co].skill[48][0] = 1; // Taunt
		if (old.skill[42][0] && ch[co].skill[42][2]) ch[co].skill[42][0] = 1; // Poison
		if (old.skill[34][0] && ch[co].skill[34][2]) ch[co].skill[34][0] = 1; // Concentrate
		if (old.skill[16][0] && ch[co].skill[16][2]) ch[co].skill[16][0] = 1; // Shield
		if (old.skill[18][0] && ch[co].skill[18][2]) ch[co].skill[18][0] = 1; // Enhance
		if ((IS_ANY_TEMP(co) || IS_BRAVER(co)) && 
			old.skill[11][0] && ch[co].skill[11][2]) ch[co].skill[11][0] = 1; // MShield (For T/B)
		if (old.skill[19][0] && ch[co].skill[19][2]) ch[co].skill[19][0] = 1; // Slow
		if (old.skill[20][0] && ch[co].skill[20][2]) ch[co].skill[20][0] = 1; // Curse
		if (old.skill[21][0] && ch[co].skill[21][2]) ch[co].skill[21][0] = 1; // Bless
		if (old.skill[22][0] && ch[co].skill[22][2]) ch[co].skill[22][0] = 1; // Identify
		if (old.skill[23][0] && ch[co].skill[23][2]) ch[co].skill[23][0] = 1; // Resistance
		if (old.skill[25][0] && ch[co].skill[25][2]) ch[co].skill[25][0] = 1; // Dispel
		if (old.skill[26][0] && ch[co].skill[26][2]) ch[co].skill[26][0] = 1; // Heal
		if (old.skill[29][0] && ch[co].skill[29][2]) ch[co].skill[29][0] = 1; // Rest
		if (old.skill[32][0] && ch[co].skill[32][2]) ch[co].skill[32][0] = 1; // Immunity
		if (old.skill[33][0] && ch[co].skill[33][2]) ch[co].skill[33][0] = 1; // Surround Hit
		if (old.skill[41][0] && ch[co].skill[41][2]) ch[co].skill[41][0] = 1; // Weaken
		
		if (old.skill[ 7][0] && ch[co].skill[ 7][2]) ch[co].skill[ 7][0] = 1; // Zephyr
		if (old.skill[35][0] && ch[co].skill[35][2]) ch[co].skill[35][0] = 1; // Warcry
		if (old.skill[15][0] && ch[co].skill[15][2]) ch[co].skill[15][0] = 1; // Lethargy
		if (old.skill[43][0] && ch[co].skill[43][2]) ch[co].skill[43][0] = 1; // Pulse
		if (old.skill[46][0] && ch[co].skill[46][2]) ch[co].skill[46][0] = 1; // Shadow
		if (old.skill[49][0] && ch[co].skill[49][2]) ch[co].skill[49][0] = 1; // Leap
		if (old.skill[14][0] && ch[co].skill[14][2]) ch[co].skill[14][0] = 1; // Rage
		
		if (old.flags & CF_SENSE)    ch[co].flags |= CF_SENSE;
		if (old.flags & CF_APPRAISE) ch[co].flags |= CF_APPRAISE;
		if (old.flags & CF_LOCKPICK) ch[co].flags |= CF_LOCKPICK;
		
		ch[co].bs_points = old.bs_points;
		ch[co].os_points = old.os_points;
		ch[co].tokens    = old.tokens;
		
		ch[co].waypoints = old.waypoints;
		god_transfer_char(co, 512, 512);
	}
	else
	{
		for (n = 0; n<40; n++) ch[co].item[n] 		= 0;
		for (n = 0; n<20; n++) ch[co].worn[n] 		= 0;
		for (n = 0; n<12; n++) ch[co].alt_worn[n] 	= 0;
		
		ch[co].data[18] = 0;      // pentagram experience
		ch[co].data[20] = 0;      // highest gorge solved
		ch[co].data[21] = 0;      // seyan'du sword bits
		ch[co].data[22] = 0;      // arena monster reset
		ch[co].data[45] = 0;      // current rank
		
		// Reset BS flags etc.
		ch[co].bs_points = 0;
		ch[co].os_points = 0;
		ch[co].tokens = 0;
		ch[co].data[26] = 0;
		ch[co].data[27] = 0;
		ch[co].data[28] = 0;
		
		// Reset Pole flags
		ch[co].data[46] = 0;
		ch[co].data[47] = 0;
		ch[co].data[48] = 0;
		ch[co].data[49] = 0;
		ch[co].data[91] = 0;
		
		// Reset FK flags
		ch[co].data[60] = 0;
		ch[co].data[61] = 0;
		ch[co].data[62] = 0;
		ch[co].data[CHD_MASTER] = 0;
		ch[co].data[70] = 0;
		ch[co].data[93] = 0;
		ch[co].data[73] = 0;
		
		// Reset quest flags
		ch[co].data[72] = 0;
		ch[co].data[94] = 0;
		
		// Remove learned flags
		ch[co].flags &= ~(CF_APPRAISE | CF_LOCKPICK | CF_SENSE);
	}
	
	ch[co].a_end  = 999999;
	ch[co].a_hp   = 999999;
	ch[co].a_mana = 999999;

	do_update_char(co);
}

int god_save(int cn, int co)
{
	int handle;
	char buf[80];

	if (co == 0)
	{
		do_char_log(cn, 0, "No such character.\n");
		return 0;
	}
	else if (!IS_SANECHAR(co))
	{
		do_char_log(cn, 0, "Bad character number: %d\n", co);
		return 0;
	}
	else if (!IS_PLAYER(co))
	{
		do_char_log(cn, 0, "Character %d is not a player.\n", co);
		return 0;
	}

	sprintf(buf, ".save/%s.moa", ch[co].name);
	handle = open(buf, O_WRONLY | O_TRUNC | O_CREAT, 0600);
	if (handle==-1)
	{
		do_char_log(cn, 0, "Could not open file.\n");
		perror(buf);
		return 0;
	}
	write(handle, &co, 4);
	write(handle, &ch[co].pass1, 4);
	write(handle, &ch[co].pass2, 4);
	write(handle, ch[co].name, 40);
	write(handle, &ch[co].temp, 4);
	close(handle);
	do_char_log(cn, 1, "Saved as %s.moa.\n", ch[co].name);
	chlog(cn, "IMP: Saved %s.", ch[co].name);

	return 1;
}

void god_mail_pass(int cn, int co)
{
	char buf[256];

	if (!god_save(cn, co))
	{
		return;
	}

	sprintf(buf, "uuencode %s.moa </home/merc/.save/%s.moa | mail -s \"%s savefile\" %s",
	        ch[co].name,
	        ch[co].name,
	        ch[co].name,
	        "admin@astonia.com");

	system(buf);
}

void god_slap(int cn, int co)
{
	// DB: why else ... else .. else ?
	if (co == 0)
	{
		do_char_log(cn, 0, "No such character.\n");
		return;
	}
	else if (!IS_SANECHAR(co))
	{
		do_char_log(cn, 0, "Bad character number: %d\n", co);
		return;
	}
	else if (!IS_USEDCHAR(co))
	{
		do_char_log(cn, 0, "Character %d is not active.\n", co);
		return;
	}
	else if (!IS_ACTIVECHAR(co) || IS_GOD(co))
	{
		do_char_log(cn, 0, "%s is not available to feel your punishment.\n", ch[co].name);
		return;
	}

	if (ch[co].a_hp>10000)
	{
		ch[co].a_hp -= 5000;
	}
	if (ch[co].a_end>10000)
	{
		ch[co].a_end -= 5000;
	}
	if (ch[co].a_mana>10000)
	{
		ch[co].a_mana -= 5000;
	}

	if (cn)
	{
		do_char_log(cn, 1, "Slapped %s.\n", ch[co].reference);
	}
	do_char_log(co, 0, "A god reaches down and slaps you in the face.\n");
	do_area_log(cn, co, ch[co].x, ch[co].y, 1, "A god reaches down and slaps %s in the face.\n", ch[co].reference);
	chlog(cn, "IMP: slapped %s (%d)", ch[co].name, co);
}

void god_spritechange(int cn, int co, int sprite)
{
	if (co == 0)
	{
		do_char_log(cn, 0, "No such character.\n");
		return;
	}
	else if (!IS_SANECHAR(co))
	{
		do_char_log(cn, 0, "Bad character number: %d\n", co);
		return;
	}
	else if (!IS_USEDCHAR(co))
	{
		do_char_log(cn, 0, "Character %d is not active.\n", co);
		return;
	}
	else if (sprite>31 || sprite<0)
	{
		do_char_log(cn, 0, "Sprite base out of bounds.\n");
		return;
	}
	else if ((ch[co].flags & CF_PLAYER) && !(ch[cn].flags & CF_GOD))
	{
		do_char_log(cn, 0, "Sorry, you cannot change a player's sprite.\n");
		return;
	}

	chlog(cn, "IMP: changed sprite of %s (%d) from %d to %d", ch[co].name, co, (ch[co].sprite - 2000) / 1024, sprite);
	do_char_log(cn, 0, "%s former sprite base was %d.\n", ch[co].name, (ch[co].sprite - 2000) / 1024);
	ch[co].sprite = sprite * 1024 + 2000;
}

void god_luck(int cn, int co, int val)
{
	if (co == 0)
	{
		do_char_log(cn, 0, "No such character.\n");
		return;
	}
	else if (!IS_SANECHAR(co))
	{
		do_char_log(cn, 0, "Bad character number: %d\n", co);
		return;
	}
	else if (!IS_PLAYER(co))
	{
		do_char_log(cn, 0, "Character %d is not a player.\n", co);
		return;
	}
	ch[co].luck = val;

	do_char_log(cn, 2, "Set %s luck to %d.\n", ch[co].name, val);
	chlog(cn, "IMP: Set %s luck to %d.", ch[co].name, val);

}

void god_reset_description(int cn, int co)
{
	char old_desc[LENDESC];

	if (co == 0)
	{
		do_char_log(cn, 0, "No such character.\n");
		return;
	}
	else if (!IS_SANECHAR(co))
	{
		do_char_log(cn, 0, "Bad character number: %d\n", co);
		return;
	}
	else if (!IS_PLAYER(co) && !IS_USURP(co))
	{
		do_char_log(cn, 0, "Character %d is not a player.\n", co);
		return;
	}

	if (ch[co].flags & CF_NODESC)   // is currently hindered from describing self
	{
		ch[co].flags &= ~CF_NODESC;
		do_char_log(cn, 2, "%s is again able to change his/her description.\n", ch[co].name);
		do_char_log(co, 0, "The gods have restored your ability to describe yourself.\n");
		chlog(cn, "IMP: reset NODESC flag on %s", ch[co].name);
	}
	else
	{
		strncpy(old_desc, ch[co].description, LENDESC);
		sprintf(ch[co].description, "%s chose an indecent description and it was removed by the gods.", ch[co].name);
		ch[co].flags |= CF_NODESC;
		do_char_log(cn, 2, "Reset description of %s (%d).\n", ch[co].name, co);
		do_char_log(co, 0, "A god has removed your inappropriate description.\n");
		chlog(cn, "IMP: reset description of %s (%d)", ch[co].name, co);
		chlog(co, "previous description: %-.*s", LENDESC, old_desc);
	}
}

void god_set_name(int cn, int co, char *name)
{
	if (co == 0)
	{
		do_char_log(cn, 0, "No such character.\n");
		return;
	}
	else if (!IS_SANECHAR(co))
	{
		do_char_log(cn, 0, "Bad character number: %d\n", co);
		return;
	}
	else if (!IS_GOD(cn) && ch[co].data[CHD_MASTER]!=cn)
	{
		do_char_log(cn, 0, "Character %d isn't one of your thralls.\n", co);
		return;
	}

	if (!name || strlen(name)<3 || strlen(name)>35)
	{
		do_char_log(cn, 0, "Name too short or long.\n");
		return;
	}
	//titlecase_str(name);

	chlog(cn, "IMP: changed name %s (%d) to %s", ch[co].name, co, name);

	strcpy(ch[co].name, name);
	strcpy(ch[co].reference, name);
	do_char_log(cn, 1, "Done.\n");

	do_update_char(co);
}

void god_usurp(int cn, int co)
{
	int nr;

	if (co == 0)
	{
		do_char_log(cn, 0, "No such character.\n");
		return;
	}
	else if (!IS_SANECHAR(co))
	{
		do_char_log(cn, 0, "Bad character number: %d\n", co);
		return;
	}
	else if (!IS_USEDCHAR(co) || !IS_SANENPC(co))
	{
		do_char_log(cn, 0, "Character %d is not an NPC.\n", co);
		return;
	}

	chlog(cn, "IMP: Usurping %s (%d, t=%d)", ch[co].name, co, ch[co].temp);

	ch[co].flags |= CF_USURP;
	nr = ch[cn].player;

	ch[co].player = nr;
	player[nr].usnr = co;

	if (ch[cn].flags & CF_USURP)
	{
		ch[co].data[97] = ch[cn].data[97];
		ch[cn].data[97] = 0;
	}
	else
	{
		ch[co].data[97] = cn;
		ch[cn].flags |= CF_CCP;
	}

	if (ch[cn].flags & (CF_PLAYER))
	{
		ch[cn].tavern_x = ch[cn].x;
		ch[cn].tavern_y = ch[cn].y;
		god_transfer_char(cn, 10, 10);
		if (!ch[cn].data[PCD_AFK])
		{
			do_afk(cn, NULL);
		}
	}
	plr_logout(cn, nr, LO_USURP);

	do_update_char(co);
}

void god_exit_usurp(int cn)
{
	int co, nr;

	ch[cn].flags &= ~(CF_USURP | CF_STAFF | CF_IMMORTAL | CF_GOD | CF_CREATOR);
	co = ch[cn].data[97];
	ch[co].flags &= ~CF_CCP;

	nr = ch[cn].player;

	ch[co].player = nr;
	player[nr].usnr = co;
	god_transfer_char(co, 512, 512); // simulate recall
	do_afk(co, NULL);

	do_update_char(cn);
}

void god_grolm(int cn)
{
	int co;

	co = pop_create_char(386, 1);
	if (co)
	{
		chlog(cn, "IMP: is now playing %s (%d)", ch[co].name, co);
		god_usurp(cn, co);
	}
}

void god_grolm_info(int cn)
{
	static char *states[3] = {"at_home", "moving_out", "moving_in"};
	int co;

	for (co = 1; co<MAXCHARS; co++)
	{
		if (ch[co].temp==478)
		{
			break;
		}
	}

	if (co==MAXCHARS || ch[co].used!=USE_ACTIVE || (ch[co].flags & CF_BODY))
	{
		do_char_log(cn, 1, "Grolmy is dead.\n");
		//return;
	}
	else
	{
		do_char_log(cn, 2, "Current state=%s, runs=%d, timer=%2.2fm, id=%d.\n",
			(ch[co].data[22]>=0 || ch[co].data[22]<=2) ? states[ch[co].data[22]] : "unknown",
			ch[co].data[40],
			(double)(globs->ticker - ch[co].data[23]) / (TICKS * 60.0),
			co);
	}
				
	for (co = 1; co<MAXCHARS; co++)
	{
		if (ch[co].temp==1321)
		{
			break;
		}
	}

	if (co==MAXCHARS || ch[co].used!=USE_ACTIVE || (ch[co].flags & CF_BODY))
	{
		do_char_log(cn, 1, "SeaGrolmy is dead.\n");
		return;
	}

	do_char_log(cn, 2, "Current state=%s, runs=%d, timer=%2.2fm, id=%d.\n",
	            (ch[co].data[22]>=0 || ch[co].data[22]<=2) ? states[ch[co].data[22]] : "unknown",
	            ch[co].data[40],
	            (double)(globs->ticker - ch[co].data[23]) / (TICKS * 60.0),
	            co);
}

void god_grolm_start(int cn)
{
	int co;

	for (co = 1; co<MAXCHARS; co++) if (ch[co].temp==478) break;
	if (co==MAXCHARS || ch[co].used!=USE_ACTIVE || (ch[co].flags & CF_BODY))
	{
		do_char_log(cn, 1, "Grolmy is dead.\n");
		//return;
	}
	else if (ch[co].data[22]!=0)
	{
		do_char_log(cn, 1, "Grolmy is already moving.\n");
		//return;
	}
	else
	{
		ch[co].data[22] = 1;
	}
	
	for (co = 1; co<MAXCHARS; co++) if (ch[co].temp==1321) break;
	if (co==MAXCHARS || ch[co].used!=USE_ACTIVE || (ch[co].flags & CF_BODY))
	{
		do_char_log(cn, 1, "SeaGrolmy is dead.\n");
		return;
	}
	else if (ch[co].data[22]!=0)
	{
		do_char_log(cn, 1, "SeaGrolmy is already moving.\n");
		return;
	}
	else
	{
		ch[co].data[22] = 1;
	}
}

void god_gargoyle(int cn)
{
	int co;

	co = pop_create_char(495, 1);
	if (co)
	{
		chlog(cn, "is now playing %s (%d)", ch[co].name, co);
		god_usurp(cn, co);
	}
}

void god_minor_racechange(int cn, int t) // note: cannot deal with values which are already higher than the new max!
{
	int n, tempkin;

	if (!IS_SANECHAR(cn) || !IS_SANECTEMPLATE(t))
	{
		return;
	}

	chlog(cn, "Changed into %s", ch_temp[t].name);

	ch[cn].hp[1] = ch_temp[t].hp[1];
	ch[cn].hp[2] = ch_temp[t].hp[2];
	ch[cn].hp[3] = ch_temp[t].hp[3];

	ch[cn].end[1] = ch_temp[t].end[1];
	ch[cn].end[2] = ch_temp[t].end[2];
	ch[cn].end[3] = ch_temp[t].end[3];

	ch[cn].mana[1] = ch_temp[t].mana[1];
	ch[cn].mana[2] = ch_temp[t].mana[2];
	ch[cn].mana[3] = ch_temp[t].mana[3];

	ch[cn].sprite = ch_temp[t].sprite;
	
	tempkin = ch[cn].kindred;
	
	ch[cn].kindred = ch_temp[t].kindred;
	
	if (tempkin & KIN_PURPLE) ch[cn].kindred |= KIN_PURPLE;
	
	if (tempkin & KIN_CLANKWAI) ch[cn].kindred |= KIN_CLANKWAI;
	else if (tempkin & KIN_CLANGORN) ch[cn].kindred |= KIN_CLANGORN;

	ch[cn].temp = t;
	ch[cn].weapon_bonus = ch_temp[t].weapon_bonus;
	ch[cn].armor_bonus  = ch_temp[t].armor_bonus;
	ch[cn].gethit_bonus = ch_temp[t].gethit_bonus;

	// character flags??

	for (n = 0; n<5; n++)
	{
		ch[cn].attrib[n][1] = ch_temp[t].attrib[n][1];
		ch[cn].attrib[n][2] = ch_temp[t].attrib[n][2];
		ch[cn].attrib[n][3] = ch_temp[t].attrib[n][3];
	}

	for (n = 0; n<MAXSKILL; n++)
	{
		if (B_SK(cn, n)==0 && ch_temp[t].skill[n][0])
		{
			B_SK(cn, n) = ch_temp[t].skill[n][0];
			xlog("added %s to %s", skilltab[n].name, ch[cn].name);
		}
		ch[cn].skill[n][1] = ch_temp[t].skill[n][1];
		ch[cn].skill[n][2] = ch_temp[t].skill[n][2];
		ch[cn].skill[n][3] = ch_temp[t].skill[n][3];
	}

	ch[cn].data[45] = 0;      // reset level
	do_check_new_level(cn, 0);
}

void god_force(int cn, char *whom, char *text)
{
	int co;

	if (cn <= 0)
	{
		return;
	}

	if (!*whom)
	{
		do_char_log(cn, 0, "#FORCE whom?\n");
		return;
	}
	co = dbatoi_self(cn, whom);
	if (co <= 0)
	{
		do_char_log(cn, 0, "No such character.\n");
		return;
	}
	if (!IS_USEDCHAR(co))
	{
		do_char_log(cn, 0, "Character is not active.\n");
		return;
	}
	if (IS_PLAYER(co) && !IS_GOD(cn))
	{
		do_char_log(cn, 0, "Not allowed to #FORCE players.\n");
		return;
	}
	if (!text || !*text)
	{
		do_char_log(cn, 0, "#FORCE %s to what?\n", ch[co].name);
		return;
	}
	chlog(cn, "IMP: Forced %s (%d) to \"%s\"", ch[co].name, co, text);
	do_say(co, text);
	do_char_log(cn, 2, "%s was forced.\n", ch[co].name);
}

/* CS, 991205: #ENEMY <NPC> <char> */
void do_enemy(int cn, char *npc, char *victim)
{
	int co, cv, cc;

	if (!*npc)
	{
		do_char_log(cn, 0, "Make whom the enemy of whom?\n");
		return;
	}
	co = dbatoi(npc);
	if (co == 0)
	{
		do_char_log(cn, 0, "No such character: '%s'.\n", npc);
		return;
	}
	if (!IS_USEDCHAR(co))
	{
		do_char_log(cn, 0, "That character is currently not in use.\n");
		return;
	}
	if (IS_PLAYER(co))
	{
		do_char_log(cn, 0, "#ENEMY only works on NPCs; %s is a player.\n", ch[co].name);
		return;
	}
	if (!*victim)
	{
		npc_list_enemies(co, cn);
		return;
	}
	cv = dbatoi(victim);
	if (cv == 0)
	{
		do_char_log(cn, 0, "No such character: '%s'.\n", victim);
		return;
	}
	if (!IS_USEDCHAR(cv))
	{
		do_char_log(cn, 0, "That character is currently not in use.\n");
		return;
	}
	if (npc_is_enemy(co, cv))
	{
		if (!npc_remove_enemy(co, cv))
		{
			do_char_log(cn, 0, "Can't remove %s from %s's enemy list!\n", ch[cv].name, ch[co].name);
			xlog("#ENEMY failed to remove %s from %s's enemy list.", ch[cv].name, ch[co].name);
		}
		else
		{
			do_char_log(cn, 0, "Removed %s from %s's enemy list.\n", ch[cv].name, ch[co].name);
			chlog(cn, "IMP: Removed %s from %s's enemy list.\n", ch[cv].name, ch[co].name);
		}
		return;
	}
	if (ch[co].data[CHD_GROUP] == ch[cv].data[CHD_GROUP])
	{
		do_char_log(cn, 0, "%s refuses to fight %s.\n", ch[co].name, ch[cv].name);
		return;
	}
	if (!npc_add_enemy(co, cv, 1))
	{
		do_char_log(cn, 0, "%s can't handle any more enemies.\n", ch[co].name);
		return;
	}
	if (ch[cn].text[1][0])
	{
		if (!(IS_SANECHAR(cc = ch[cn].data[CHD_MASTER]) && (ch[cc].flags & CF_SILENCE)))
			do_sayx(co, ch[co].text[1], ch[cv].name);
	}
	chlog(cn, "IMP: Made %s an enemy of %s", ch[cv].name, ch[co].name);
	chlog(co, "Added %s to kill list (#ENEMY by %s)", ch[cv].name, ch[cn].name);
	do_char_log(cn, 2, "%s is now an enemy of %s.\n", ch[cv].name, ch[co].name);
}


#define MAXBAN 250

struct ban
{
	char creator[80];
	char victim[80];

	unsigned int addr;
};

struct ban ban[MAXBAN];
int maxban = 0;

int god_read_banlist(void)
{
	int handle;

	handle = open("banlist.dat", O_RDONLY);
	if (handle==-1)
	{
		return 0;
	}

	read(handle, &maxban, sizeof(maxban));
	read(handle, ban, sizeof(ban));

	close(handle);

	return 1;
}

int god_write_banlist(void)
{
	int handle;

	handle = open("banlist.dat", O_WRONLY | O_CREAT | O_TRUNC, 0600);
	if (handle==-1)
	{
		return 0;
	}

	write(handle, &maxban, sizeof(maxban));
	write(handle, ban, sizeof(ban));

	close(handle);

	return 1;
}

int god_is_banned(int addr)
{
	int n;

	for (n = 0; n<maxban; n++)
	{
		if ((ban[n].addr & 0x00ffffff)==(addr & 0x00ffffff))
		{
			return 1;
		}
	}

	return 0;
}

int god_add_single_ban(int cn, int co, unsigned int addr)
{
	int n;

	for (n = 0; n<maxban; n++)
	{
		if ((ban[n].addr & 0x00ffffff)==(addr & 0x00ffffff))
		{
			do_char_log(cn, 2, "%s is already banned\n", ch[co].name);
			return 1;
		}
	}
	for (n = 0; n<maxban; n++)
	{
		if (!ban[n].addr)
		{
			break;
		}
	}
	if (n + 1==MAXBAN)
	{
		do_char_log(cn, 2, "Sorry, ban list is full.\n");
		return 1;
	}
	strcpy(ban[n].creator, ch[cn].name);
	strcpy(ban[n].victim, ch[co].name);
	ban[n].addr = addr;
	if (n==maxban)
	{
		maxban = n + 1;
	}

	do_char_log(cn, 2, "Banned %s (%d.%d.%d.%d)\n", ch[co].name, addr & 255, (addr >> 8) & 255, (addr >> 16) & 255, addr >> 24);

	return 1;
}

void god_add_ban(int cn, int co)
{
	int n;

	for (n = 80; n<90; n++)
	{
		if (ch[co].data[n])
		{
			if (!god_add_single_ban(cn, co, ch[co].data[n]))
			{
				break;
			}
		}
	}

	chlog(cn, "IMP: banned %s (%d)", ch[co].name, co);

	god_write_banlist();
}

void god_del_ban(int cn, int nr)
{
	if (nr>maxban || !ban[nr].addr)
	{
		do_char_log(cn, 2, "Number out of bounds.\n");
		return;
	}
	ban[nr].addr = 0;
	do_char_log(cn, 2, "Done.\n");

	god_write_banlist();
}

void god_list_ban(int cn)
{
	int n;

	for (n = 0; n<maxban; n++)
	{
		if (!ban[n].addr)
		{
			continue;
		}
		do_char_log(cn, 2, "%03d: %-10.10s %d.%d.%d.%d (%s)\n",
		            n, ban[n].victim,
		            ban[n].addr & 255, (ban[n].addr >> 8) & 255, (ban[n].addr >> 16) & 255, ban[n].addr >> 24,
		            ban[n].creator);
	}
}

static char **badname = NULL;
static int cursize = 0, maxsize = 0;

void god_init_badnames(void)
{
	char buf[80], *dst, *src;
	FILE *fp;

	fp = fopen("badnames.txt", "r");
	if (!fp)
	{
		return;
	}

	if (badname)
	{
		free(badname);
		badname = NULL;
		cursize = maxsize = 0;
	}

	while (fgets(buf, 79, fp))
	{
		buf[79] = 0; // fgets is silly.

		dst = src = buf;

		while (*src)
		{
			if (isspace(*src))
			{
				src++;
				continue;
			}
			if (*src=='\r' || *src=='\n')
			{
				break;
			}
			*dst++ = *src++;
		}
		*dst = 0;

		if (strlen(buf)<3)
		{
			continue;
		}

		if (cursize>=maxsize)
		{
			maxsize += 256;
			badname  = realloc(badname, maxsize * sizeof(char**));
		}
		badname[cursize++] = strdup(buf);
	}

	fclose(fp);
}

int god_is_badname(char *name)
{
	int n;

	if (strlen(name)>15)
	{
		return 1;
	}

	for (n = 0; n<cursize; n++)
	{
		if (strstr(name, badname[n]))
		{
			return 1;
		}
	}
	return 0;
}

void god_shutup(int cn, int co)
{
	god_set_flag(cn, co, CF_SHUTUP);
	if (ch[cn].flags & (CF_IMP | CF_GOD))
	{
		return;
	}

	do_imp_log(2, "%s set %s %s shutup (just for info).\n",
	           ch[cn].name, ch[co].name,
	           (ch[co].flags & CF_SHUTUP) ? "on" : "off");
}

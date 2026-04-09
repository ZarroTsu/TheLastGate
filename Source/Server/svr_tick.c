/*************************************************************************

   This file is part of 'Mercenaries of Astonia v2'
   Copyright (c) 1997-2001 Daniel Brockhaus (joker@astonia.com)
   All rights reserved.

 **************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#ifdef ONLINE
#include <crypt.h>
#endif

#define TMIDDLEX (TILEX/2)
#define TMIDDLEY (TILEY/2)

#include "server.h"

int ctick = 0;

static char intro_msg1_font = 1;
static char intro_msg1[] = {"Welcome to The Last Gate, based on the Mercenaries of Astonia engine by Daniel Brockhaus!\n"};
static char intro_msg2_font = 1;
static char intro_msg2[] = {"May your visit here be... interesting.\n"};
static char intro_msg3_font = 3;
static char intro_msg3[] = {"Current client/server version is 0.14.3\n"};
static char intro_msg4_font = 0;
static char intro_msg4[] = {"Skill trees have been overhauled and deallocated. Remember to reallocate before adventuring!\n"};
static char intro_msg5_font = 2;
static char intro_msg5[] = {"For patch notes and changes, please visit our Discord using the Discord button on the load menu.\n"};

static char newbi_msg1_font = 1;
static char newbi_msg1[] = {"Welcome to The Last Gate, based on the Mercenaries of Astonia engine by Daniel Brockhaus!\n"};
static char newbi_msg2_font = 2;
static char newbi_msg2[] = {"New to the the game? Click on the TUTORIAL button to take a tour!\n"};
static char newbi_msg3_font = 1;
static char newbi_msg3[] = {"You can use /help (or #help) to get a listing of helpful text commands.\n"};
static char newbi_msg4_font = 3;
static char newbi_msg4[] = {"If you need assistance, try /shout before you type. This will notify the entire server.\n"};
static char newbi_msg5_font = 2;
static char newbi_msg5[] = {"If you need further help or an admin, feel free to join our Discord server using the Discord button on the load menu.\n"};

static inline unsigned int _mcmp(unsigned char *a, unsigned char *b, unsigned int len)
{
	// align a
	while (len>0 && ((int)(a) & 3))
	{
		if (*a!=*b)
		{
			return 1;
		}
		a++;
		b++;
		len--;
	}
	// compare as much as possible with 32 bit cmds
	while (len>3)
	{
		if (*(unsigned long *)a!=*(unsigned long *)b)
		{
			return 1;
		}
		a += 4;
		b += 4;
		len -= 4;
	}
	// compare remaining 0-3 bytes
	while (len>0)
	{
		if (*a!=*b)
		{
			return 1;
		}
		a++;
		b++;
		len--;
	}
	return 0;
}

// some magic to avoid a lot of casts
static inline unsigned int mcmp(void *a, void *b, unsigned int len)
{
	return(_mcmp(a, b, len));
}

static inline void *_fdiff(unsigned char *a, unsigned char *b, unsigned int len)
{
	// align a
	while (len>0 && ((int)(a) & 3))
	{
		if (*a!=*b)
		{
			return( a);
		}
		a++;
		b++;
		len--;
	}
	// compare as much as possible with 32 bit cmds
	while (len>3)
	{
		if (*(unsigned long *)a!=*(unsigned long *)b)
		{
			while (*a==*b)
			{
				a++;
				b++;
			}
			return(a);
		}
		a += 4;
		b += 4;
		len -= 4;
	}
	// compare remaining 0-3 bytes
	while (len>0)
	{
		if (*a!=*b)
		{
			return( a);
		}
		a++;
		b++;
		len--;
	}
	return 0;
}

// some magic to avoid a lot of casts
static inline void *fdiff(void *a, void *b, unsigned int len)
{
	return(_fdiff(a, b, len));
}

static inline unsigned int _mcpy(unsigned char *a, unsigned char *b, unsigned int len)
{
	// align a
	while (len>0 && ((int)(a) & 3))
	{
		*a = *b;
		a++;
		b++;
		len--;
	}
	// compare as much as possible with 32 bit cmds
	while (len>3)
	{
		*(unsigned long *)a = *(unsigned long *)b;
		a += 4;
		b += 4;
		len -= 4;
	}
	// compare remaining 0-3 bytes
	while (len>0)
	{
		*a = *b;
		a++;
		b++;
		len--;
	}
	return 0;
}

// some magic to avoid a lot of casts
static inline unsigned int mcpy(void *a, void *b, unsigned int len)
{
	return(_mcpy(a, b, len));
}


static char secret[256] = {"\
Ifhjf64hH8sa,-#39ddj843tvxcv0434dvsdc40G#34Trefc349534Y5#34trecerr943\
5#erZt#eA534#5erFtw#Trwec,9345mwrxm gerte-534lMIZDN(/dn8sfn8&DBDB/D&s\
8efnsd897)DDzD'D'D''Dofs,t0943-rg-gdfg-gdf.t,e95.34u.5retfrh.wretv.56\
9v4#asf.59m(D)/ND/DDLD;gd+dsa,fw9r,x  OD(98snfsfa"};

unsigned int xcrypt(unsigned int val)
{
	unsigned int res = 0;

	res += (unsigned int)(secret[ val & 255]);
	res += (unsigned int)(secret[(val >> 8 ) & 255]) << 8;
	res += (unsigned int)(secret[(val >> 16) & 255]) << 16;
	res += (unsigned int)(secret[(val >> 24) & 255]) << 24;

	res ^= 0x5a7ce52e;

	return(res);
}

void send_mod(int nr)
{
	int n;
	unsigned char buf[16];
	extern char mod[];

	for (n = 0; n<8; n++)
	{
		buf[0] = SV_MOD1 + n;
		memcpy(buf + 1, mod + n * 15, 15);
		csend(nr, buf, 16);
	}
}

void plr_challenge_newlogin(int nr)
{
	unsigned char buf[16];
	int tmp;

	tmp = RANDOM(0x3fffffff);
	if (tmp==0)
	{
		tmp = 42;
	}

	player[nr].challenge = tmp;
	player[nr].state = ST_NEW_CHALLENGE;
	player[nr].lasttick = globs->ticker;

	buf[0] = SV_CHALLENGE;
	*(unsigned long *)(buf + 1) = tmp;

	csend(nr, buf, 16);

	send_mod(nr);
}

void plr_challenge_login(int nr)
{
	unsigned char buf[16];
	int tmp, cn;

	plog(nr, "challenge_login");

	tmp = RANDOM(0x3fffffff);
	if (tmp==0)
	{
		tmp = 42;
	}

	player[nr].challenge = tmp;
	player[nr].state = ST_LOGIN_CHALLENGE;
	player[nr].lasttick = globs->ticker;

	buf[0] = SV_CHALLENGE;
	*(unsigned long *)(buf + 1) = tmp;

	csend(nr, buf, 16);

	cn = *(unsigned long*)(player[nr].inbuf + 1);
	if (cn<1 || cn>=MAXCHARS)
	{
		plog(nr, "sent wrong cn %d in challenge login", cn);
		plr_logout(0, nr, LO_CHALLENGE);
		return;
	}
	player[nr].usnr  = cn;
	player[nr].pass1 = *(unsigned long*)(player[nr].inbuf + 5);
	player[nr].pass2 = *(unsigned long*)(player[nr].inbuf + 9);

	send_mod(nr);
}

void plr_challenge(int nr)
{
	unsigned int tmp;

	tmp = *(unsigned long*)(player[nr].inbuf + 1);
	player[nr].version = *(unsigned long*)(player[nr].inbuf + 5);
	player[nr].race = *(unsigned long*)(player[nr].inbuf + 9);

	if (tmp!=xcrypt(player[nr].challenge))
	{
		plog(nr, "Challenge failed");
		plr_logout(player[nr].usnr, nr, LO_CHALLENGE);
		return;
	}
	switch(player[nr].state)
	{
	case ST_NEW_CHALLENGE:
		player[nr].state = ST_NEWLOGIN;
		player[nr].lasttick = globs->ticker;
		break;
	case ST_LOGIN_CHALLENGE:
		player[nr].state = ST_LOGIN;
		player[nr].lasttick = globs->ticker;
		break;
	case ST_CHALLENGE:
		player[nr].state = ST_NORMAL;
		player[nr].lasttick = globs->ticker;
		player[nr].ltick = 0;
		break;
	default:
		plog(nr, "Challenge reply at unexpected state");
	}

	plog(nr, "Challenge ok");
}

void plr_perf_report(int nr)
{
//	int ticksize, _idle, skip; //,cn;
//	float kbps;
	
//	ticksize = *(unsigned short*)(player[nr].inbuf + 1);
//	skip  = *(unsigned short*)(player[nr].inbuf + 3);
//	_idle = *(unsigned short*)(player[nr].inbuf + 5);
//	kbps=*(float *)(player[nr].inbuf+7);
	
	player[nr].lasttick = globs->ticker;      // update timeout
	
//	plog(nr,"ticksize=%3d, %2d%% skip, %2d%% idle, %2.2fkBps",
//		ticksize,skip,_idle,kbps);

/*	cn=player[nr].usnr;
	if (cn) {
		chlog(cn,"HP=%d/%d End=%d/%d, Mana=%d/%d, Exp=%d/%d, TS=%d, SK=%d%% ID=%d%%",
			ch[cn].a_hp/1000,ch[cn].hp[5],
			ch[cn].a_end/1000,ch[cn].end[5],
			ch[cn].a_mana/1000,ch[cn].mana[5],
			ch[cn].points,ch[cn].points_tot,
			ticksize,skip,_idle);
	}*/
}

void plr_unique(int nr)
{
	char buf[16];

	player[nr].unique = *(unsigned long long*)(player[nr].inbuf + 1);
	plog(nr, "received unique %llX", player[nr].unique);

	if (!player[nr].unique)
	{
		globs->unique++;
		player[nr].unique = globs->unique;

		buf[0] = SV_UNIQUE;
		*(unsigned long long*)(buf + 1) = player[nr].unique;
		xsend(nr, buf, 9);

		plog(nr, "sent unique %llX", player[nr].unique);
	}
}

void plr_passwd(int nr)
{
	int n, hash;

	memcpy(player[nr].passwd, player[nr].inbuf + 1, 15);
	player[nr].passwd[15] = 0;

	for (n = hash = 0; n<15 && player[nr].passwd[n]; n++)
	{
		hash ^= (player[nr].passwd[n] << (n * 2));
	}

	plog(nr, "Received passwd hash %u", hash);
}

void plr_cmd_move(int nr)
{
	int x, y;
	int cn;

	x = *(unsigned short*)(player[nr].inbuf + 1);
	y = *(unsigned short*)(player[nr].inbuf + 3);
	
	if (player[nr].spectating) return;
	cn = player[nr].usnr;

	ch[cn].attack_cn = 0;
	ch[cn].goto_x = x;
	ch[cn].goto_y = y;
	ch[cn].misc_action = 0;
	ch[cn].cerrno = 0;
	ch[cn].data[12] = globs->ticker;
}

void plr_cmd_reset(int nr)
{
	int cn;
	
	if (player[nr].spectating) return;
	cn = player[nr].usnr;

	ch[cn].use_nr = 0;
	ch[cn].skill_nr  = 0;
	ch[cn].attack_cn = 0;
	ch[cn].goto_x = 0;
	ch[cn].goto_y = 0;
	ch[cn].misc_action = 0;
	ch[cn].cerrno = 0;
	ch[cn].data[12] = globs->ticker;
}

void plr_cmd_turn(int nr)
{
	int x, y;
	int cn;

	x = *(unsigned short*)(player[nr].inbuf + 1);
	y = *(unsigned short*)(player[nr].inbuf + 3);
	
	if (player[nr].spectating) return;
	cn = player[nr].usnr;

	if (IS_BUILDING(cn))
	{
		do_char_log(cn, 3, "x=%d, y=%d, m=%d,light=%d, indoors=%lld, dlight=%d.\n",
		            x, y, x + y * MAPX, map[x + y * MAPX].light, map[x + y * MAPX].flags & MF_INDOORS, map[x + y * MAPX].dlight);

		do_char_log(cn, 3, "ch=%d, to_ch=%d, it=%d.\n",
		            map[x + y * MAPX].ch, map[x + y * MAPX].to_ch, map[x + y * MAPX].it);
		do_char_log(cn, 3, "flags=%04X %04X\n",
		            (unsigned int) (map[x + y * MAPX].flags >> 32),
		            (unsigned int) (map[x + y * MAPX].flags & 0xFFFF));

		do_char_log(cn, 3, "sprite=%d, fsprite=%d\n",
		            map[x + y * MAPX].sprite, map[x + y * MAPX].fsprite);
	}


	ch[cn].attack_cn = 0;
	ch[cn].goto_x = 0;
	ch[cn].misc_action  = DR_TURN;
	ch[cn].misc_target1 = x;
	ch[cn].misc_target2 = y;
	ch[cn].cerrno = 0;
	ch[cn].data[12] = globs->ticker;
}

// inventory manipulation... moves citem from/to item or worn - will be done at once
void plr_cmd_inv(int nr)
{
	int what, n, m, tmp, tmpv, cn, in, co;
	
	if (player[nr].spectating) return;
	cn = player[nr].usnr;

	what = *(unsigned long*)(player[nr].inbuf + 1);
	n  = *(unsigned long*)(player[nr].inbuf + 5);
	co = *(unsigned long*)(player[nr].inbuf + 9);
	if (co<1 || co>=MAXCHARS)
	{
		co = 0;
	}

	if (what==0) // 0 - Inventory management - swapping items with other item slots
	{
		if (n<0 || n>(MAXITEMS-1))
		{
			return;                 // sanity check
		}
		if (ch[cn].stunned==1)
		{
			return;
		}

		tmp = ch[cn].item[n];

		if (IS_SANEITEM(tmp) && (it[tmp].temp == IT_LAGSCROLL))
			// || ch[cn].item_lock[n])) // Cannot pick up locked items
		{
			return;
		}
		/*
	//	else if (!IS_SANEITEM(tmp) && ch[cn].item_lock[n]) // Clean bad item lock if the item isn't valid
		{
		//	ch[cn].item_lock[n] = 0;
		}
		*/

		do_update_char(cn);
		
		if (ch[cn].citem & 0x80000000)
		{
			tmp = ch[cn].citem & 0x7fffffff;
			if (tmp>0)
			{
				ch[cn].gold += tmp;
			}
			ch[cn].citem = 0;
			return;
		}
		else
		{
			if (!IS_BUILDING(cn))
			{
				ch[cn].item[n] = ch[cn].citem;
			}
			else
			{
				ch[cn].misc_action = DR_SINGLEBUILD;
				do_char_log(cn, 3, "Single mode\n");
			}
		}
		ch[cn].citem = tmp;
		return;
	}
	if (what==1) // 1 - Swap held item with gear slot
	{
		if (ch[cn].stunned==1)
		{
			return;
		}

		do_swap_item(cn, n);
		return;
	}
	if (what==2) // 2 - Grab money from purse via coin button(s)
	{
		if (ch[cn].stunned==1)
		{
			return;
		}

		if (ch[cn].citem)
		{
			return;
		}
		if (n>ch[cn].gold)
		{
			return;
		}
		if (n<=0)
		{
			return;
		}
		ch[cn].citem = 0x80000000 | n;
		ch[cn].gold -= n;

		do_update_char(cn);
		return;
	}
	if (what==3) // 3 - Push or pull item stacks (CTRL+Left)
	{
		if (n<0 || n>(MAXITEMS-1) || IS_BUILDING(cn))
		{
			return;                             // sanity check
		}
		if (ch[cn].stunned==1)
		{
			return;
		}
		
		in  = ch[cn].item[n];
		tmp = ch[cn].citem;
		
		// One of the items is not stackable, abort.
		if ((in && !(it[in].flags & IF_STACKABLE)) || (tmp && !(it[tmp].flags & IF_STACKABLE)))
		{
			return;
		}
		
		// Both items are sane and stackable, and both items are the same template
		// In this instance we just subtract 1 from held item and add 1 to slot item
		if (in && tmp && (it[in].flags & IF_STACKABLE) && it[in].temp == it[tmp].temp) 
		{
			// Sanity checks
			if (it[in].stack==10)  return;
			if (it[tmp].stack < 1) it[tmp].stack = 1;
			if (it[in].stack < 1)  it[in].stack  = 1;
			
			if (it[tmp].stack > 1)
			{
				tmpv = it[tmp].value / it[tmp].stack;
				it[tmp].stack--;
				it[tmp].value = tmpv * it[tmp].stack;
			}
			else
			{
				god_take_from_char(tmp, cn);
			}
			tmpv = it[in].value / it[in].stack;
			it[in].stack++;
			it[in].value = tmpv * it[in].stack;
			
			do_update_char(cn);
		}
		// Target slot is sane but held item is not - take from slot stack
		else if (in && !tmp) 
		{
			/*
		//	if (IS_SANEITEM(in) && ch[cn].item_lock[n]) // Cannot pick up locked items
			{
				return;
			}
			*/
			if ((it[in].flags & IF_STACKABLE) && it[in].stack > 1)
			{
				if (it[in].temp)
				{
					tmp = god_create_item(it[in].temp);
				}
				else if (it[in].orig_temp)
				{
					tmp = god_create_item(it[in].orig_temp);
					it[tmp].orig_temp = tmp;
					it[tmp].temp = 0;
				}
				else if (IS_SOULCAT(in))
				{
					m = it[in].data[4]-1;
					tmp = god_create_item(IT_SOULCATAL);
					
					sprintf(it[tmp].name, "Soul Catalyst (%s)", skilltab[m].name);
					sprintf(it[tmp].reference, "soul catalyst (%s)", skilltab[m].name);
					sprintf(it[tmp].description, "A soul catalyst. Can be used on a soulstone to grant it static properties.");
					
					it[tmp].temp          = 0;
					it[tmp].driver        = 93;
					it[tmp].data[4]       = m+1;
					it[tmp].flags        |= IF_IDENTIFIED | IF_STACKABLE;
					it[tmp].stack         = 1;
					
					for (m=0; m<50; m++)
					{
						it[tmp].skill[m][I_I] = it[in].skill[m][I_I];
						it[tmp].skill[m][I_A] = it[in].skill[m][I_A];
					}
				}
				else
				{
					return;
				}
				
				tmpv = it[in].value / it[in].stack;
				it[in].stack--;
				it[in].value = tmpv * it[in].stack;
				
				if (IS_GSCROLL(in))
				{
					it[tmp].data[0] = it[in].data[0];
					it[tmp].data[1] = it[in].data[1];
				}
				if (IS_CORRUPTOR(in))
				{
					it[tmp].data[0] = it[in].data[0];
					it[tmp].flags   = it[in].flags;
				}
				
				//it[tmp].x = 0;
				//it[tmp].y = 0;
				it[tmp].carried = cn;
				ch[cn].citem = tmp;
				
				do_update_char(cn);
			}
			else
			{
				ch[cn].item[n] = tmp;
				ch[cn].citem = in;
			}
		}
		// Target slot is not valid but held item is - drop from held stack
		else if (!in && tmp) 
		{
			if ((it[tmp].flags & IF_STACKABLE) && it[tmp].stack > 1)
			{
				tmpv = it[tmp].value / it[tmp].stack;
				it[tmp].stack--;
				it[tmp].value = tmpv * it[tmp].stack;
				
				in = god_create_item(it[tmp].temp);
				
				//it[in].x = 0;
				//it[in].y = 0;
				it[in].carried = cn;
				ch[cn].item[n] = in;
				
				do_update_char(cn);
			}
			else
			{
				ch[cn].citem = in;
				ch[cn].item[n] = tmp;
			}
		//	ch[cn].item_lock[n] = 0;
		}
		return;
	}
	/*
	if (what==4) // 4 - Lock item in place in inventory so /sort doesn't move it (CTRL+Right)
	{
		if (n<0 || n>(MAXITEMS-1) || IS_BUILDING(cn))
		{
			return;                             // sanity check
		}
		
		if ((in = ch[cn].item[n])!=0)
		{
		//	if (ch[cn].item_lock[n])
			{
		//		ch[cn].item_lock[n] = 0;
				do_char_log(cn, 1, "%s, now unlocked.\n", it[in].name);
			}
			else
			{
		//		ch[cn].item_lock[n] = 1;
				do_char_log(cn, 1, "%s, now locked.\n", it[in].name);
			}
			
			do_update_char(cn);
		}
		
		return;
	}
	*/
	if (what==5) // 5 - Use a given gear piece while it is equipped, such as rings
	{
		if (n<0 || n>19 || IS_BUILDING(cn))
		{
			return;                             // sanity check
		}
		ch[cn].use_nr = n;
		ch[cn].skill_target1 = co;
		return;
	}
	if (what==6) // 6 - Use an item in your inventory
	{
		if (n<0 || n>(MAXITEMS-1) || IS_BUILDING(cn))
		{
			return;                             // sanity check
		}
		ch[cn].use_nr = n + 20;
		ch[cn].skill_target1 = co;
		return;
	}
	if (what==7) // 7 - Look at your equipment
	{
		if (n<0 || n>19 || IS_BUILDING(cn))
		{
			return;                             // sanity check
		}
		if ((in = ch[cn].worn[n])!=0)
		{
			do_look_item(cn, in);
		}
		return;
	}
	if (what==8) // 8 - Look at your items
	{
		if (n<0 || n>(MAXITEMS-1) || IS_BUILDING(cn))
		{
			return;                             // sanity check
		}
		if ((in = ch[cn].item[n])!=0)
		{
			do_look_item(cn, in);
		}
		return;
	}
	if (what==9) // 9 - Process special command.  0 = Trash,  1 = Swap
	{
		if (ch[cn].stunned==1)
		{
			return;
		}
		if (n==0)
		{
			do_trash(cn);
		}
		if (n==1)
		{
			do_swap_gear(cn);
		}
		return;
	}
	plog(nr, "Unknown CMD-INV-what %d", what);
}

void plr_cmd_inv_look(int nr)
{
	int n, cn, in;
	
	if (player[nr].spectating) return;
	cn = player[nr].usnr;

	n = *(unsigned short*)(player[nr].inbuf + 1);

	if (n<0 || n>(MAXITEMS-1))
	{
		return;                 // sanity check
	}
	if (IS_BUILDING(cn))
	{
		ch[cn].citem = ch[cn].item[n];
		do_char_log(cn, 3, "Area mode\n");
		ch[cn].misc_action = DR_AREABUILD1;
		return;
	}
	if ((in = ch[cn].item[n])!=0)
	{
		do_look_item(cn, in);
	}
}

void plr_cmd_mode(int nr)       // speed change, done at once
{
	int cn, mode;
	static char *speedname[3] = {"Slow", "Normal", "Fast"};
	
	if (player[nr].spectating) return;
	cn = player[nr].usnr;

	mode = *(unsigned short*)(player[nr].inbuf + 1);
	if (mode<0 || mode>2)
	{
		return;
	}

	ch[cn].mode = (unsigned char)mode;

	do_update_char(cn);

	plog(nr, "Speed mode: %d (%s)", mode, speedname[mode]);
}

void plr_cmd_drop(int nr)
{
	int x, y, xs, ys, xe, ye;
	int cn;

	x = *(unsigned short*)(player[nr].inbuf + 1);
	y = *(unsigned short*)(player[nr].inbuf + 3);
	
	if (player[nr].spectating) return;
	cn = player[nr].usnr;

	if (IS_BUILDING(cn))
	{
		if (ch[cn].misc_action==DR_AREABUILD2)
		{

			do_char_log(cn, 3, "Areaend: %d,%d\n", x, y);
			xs = x;
			ys = y;
			xe = ch[cn].misc_target1;
			ye = ch[cn].misc_target2;
			if (xs>xe)
			{
				x  = xe;
				xe = xs;
				xs = x;
			}
			if (ys>ye)
			{
				y  = ye;
				ye = ys;
				ys = y;
			}
			do_char_log(cn, 3, "Area: %d,%d - %d,%d\n",
			            xs, ys, xe, ye);

			for (x = xs; x<=xe; x++)
			{
				for (y = ys; y<=ye; y++)
				{
					build_drop(x, y, ch[cn].citem);
				}
			}

			ch[cn].misc_action = DR_AREABUILD1;
		}
		else if (ch[cn].misc_action==DR_AREABUILD1)
		{
			ch[cn].misc_action  = DR_AREABUILD2;
			ch[cn].misc_target1 = x;
			ch[cn].misc_target2 = y;
			do_char_log(cn, 3, "Areastart: %d,%d\n", x, y);
		}
		else if (ch[cn].misc_action==DR_SINGLEBUILD)
		{
			build_drop(x, y, ch[cn].citem);
		}
		return;
	}

	ch[cn].attack_cn = 0;
	ch[cn].goto_x = 0;
	ch[cn].misc_action  = DR_DROP;
	ch[cn].misc_target1 = x;
	ch[cn].misc_target2 = y;
	ch[cn].cerrno = 0;
	ch[cn].data[12] = globs->ticker;
}

void plr_cmd_give(int nr)
{
	int cn, co;

	co = *(unsigned int*)(player[nr].inbuf + 1);

	if (co<0 || co>=MAXCHARS)
	{
		return;
	}
	
	if (player[nr].spectating) return;
	cn = player[nr].usnr;

	ch[cn].attack_cn = 0;
	ch[cn].goto_x = 0;
	ch[cn].misc_action  = DR_GIVE;
	ch[cn].misc_target1 = co;
	ch[cn].cerrno = 0;
	ch[cn].data[12] = globs->ticker;
}

void plr_cmd_pickup(int nr)
{
	int x, y;
	int cn;

	x = *(unsigned short*)(player[nr].inbuf + 1);
	y = *(unsigned short*)(player[nr].inbuf + 3);
	
	if (player[nr].spectating) return;
	cn = player[nr].usnr;

	if (IS_BUILDING(cn))
	{
		build_remove(x, y);
		return;
	}

	ch[cn].attack_cn = 0;
	ch[cn].goto_x = 0;
	ch[cn].misc_action  = DR_PICKUP;
	ch[cn].misc_target1 = x;
	ch[cn].misc_target2 = y;
	ch[cn].cerrno = 0;
	ch[cn].data[12] = globs->ticker;
}

void plr_cmd_use(int nr)
{
	int x, y;
	int cn;
	
	if (player[nr].spectating) return;
	cn = player[nr].usnr;

	x = *(unsigned short*)(player[nr].inbuf + 1);
	y = *(unsigned short*)(player[nr].inbuf + 3);

	ch[cn].attack_cn = 0;
	ch[cn].goto_x = 0;
	ch[cn].misc_action  = DR_USE;
	ch[cn].misc_target1 = x;
	ch[cn].misc_target2 = y;
	ch[cn].cerrno = 0;
	ch[cn].data[12] = globs->ticker;
}

void plr_cmd_attack(int nr)
{
	int cn, co;

	co = *(unsigned int*)(player[nr].inbuf + 1);
	if (co<0 || co>=MAXCHARS)
	{
		return;
	}
	
	if (player[nr].spectating) return;
	cn = player[nr].usnr;
	
	ch[cn].attack_cn = co;
	ch[cn].goto_x = 0;
	ch[cn].misc_action = 0;
	ch[cn].cerrno = 0;
	ch[cn].data[12] = globs->ticker;

	plog(nr, "Trying to attack %s (%d)", ch[co].name, co);
	remember_pvp(cn, co);
}

void plr_cmd_look(int nr, int autoflag)
{
	int cn, co;

	co = *(unsigned short*)(player[nr].inbuf + 1);
	
	if (player[nr].spectating && !autoflag) return;
		cn = player[nr].usnr;

	if ((co & 0x8000) && !player[nr].spectating)
	{
		do_look_depot(cn, co & 0x7fff);
	}
	else
	{
		do_look_char(cn, co, 0, autoflag, 0);
	}
}

void plr_cmd_shop(int nr)
{
	int cn, co, n;

	co = *(unsigned short*)(player[nr].inbuf + 1);
	n = *(unsigned short*)(player[nr].inbuf + 3);
	cn = player[nr].usnr;

	if (co & 0x8000)
	{
		do_depot_char(cn, co & 0x7fff, n);
	}
	else
	{
		do_shop_char(cn, co, n);
	}
}

void plr_cmd_smith(int nr)
{
	int cn, co, n;

	co = *(unsigned short*)(player[nr].inbuf + 1);
	n  = *(unsigned short*)(player[nr].inbuf + 3);
	cn = player[nr].usnr;
	
	do_update_char(cn);

	do_blacksmith(cn, co, n);
}

void plr_cmd_qshop(int nr)
{
	int cn, co, cc, n, tmp;
	
	co = *(unsigned long*)(player[nr].inbuf + 1);
	n  = *(unsigned long*)(player[nr].inbuf + 5);
	cc = *(unsigned long*)(player[nr].inbuf + 9);

	cn = player[nr].usnr;
	
	if (n<0 || n>(MAXITEMS-1)) return;
	if (ch[cn].stunned==1) return;
	if (IS_BUILDING(cn)) return;
	if (ch[cn].citem) return;
	
	tmp = ch[cn].item[n];
	
	if (!IS_SANEITEM(tmp) || (IS_SANEITEM(tmp) && (it[tmp].temp == IT_LAGSCROLL))) return;
	
	do_update_char(cn);
	
	ch[cn].item[n] = 0;
	ch[cn].citem = tmp;

	if (co & 0x8000)
	{
		do_depot_char(cn, co & 0x7fff, n+cc*64);
	}
	else
	{
		do_shop_char(cn, co, n);
	}
}

void plr_cmd_wps(int nr)
{
	int cn, n;

	n = *(unsigned short*)(player[nr].inbuf + 1);

	cn = player[nr].usnr;

	do_waypoint(cn, n);
}

void plr_cmd_tree(int nr)
{
	int cn, n;

	n = *(unsigned short*)(player[nr].inbuf + 1);

	cn = player[nr].usnr;

	do_treeupdate(cn, n);
}

void plr_cmd_motd(int nr)
{
	int cn, n;

	n = *(unsigned short*)(player[nr].inbuf + 1);

	cn = player[nr].usnr;
	
	// this command sets tutorial step in .data[76] - use (ch[cn].data[76]&(1<<n)) to check elsewhere
	// if this is 1<<16 it skips future tutorials
	ch[cn].data[76] |= (1<<n);
}

void plr_cmd_bsshop(int nr)
{
//	int cn, co, n;
//	
//	co = *(unsigned short*)(player[nr].inbuf + 1);
//	n = *(unsigned short*)(player[nr].inbuf + 3);
//	
//	cn = player[nr].usnr;
//	
//	/* do_shop_char(cn, co, n); */
}

void plr_cmd_look_item(int nr)
{
	int cn, in, x, y;

	x = *(unsigned short*)(player[nr].inbuf + 1);
	y = *(unsigned short*)(player[nr].inbuf + 3);

	if (x<0 || x>=MAPX || y<0 || y>=MAPY)
	{
		return;
	}

	in = map[x + y * MAPX].it;

	if (player[nr].spectating) return;
	cn = player[nr].usnr;

	do_look_item(cn, in);
}

void plr_cmd_stat(int nr)
{
	int n, v, cn;

	if (player[nr].spectating) return;
	cn = player[nr].usnr;

	n = *(unsigned short*)(player[nr].inbuf + 1);
	v = *(unsigned short*)(player[nr].inbuf + 3);

	if (n<0 || n>107)
	{
		return;
	}
	if (v<0 || v>900)
	{
		return;				// sanity check
	}
	if (n<5)
	{
		while (v--)
		{
			do_raise_attrib(cn, n);
		}
	}
	else if (n==5)
	{
		while (v--)
		{
			do_raise_hp(cn);
		}
	}
	/*
	else if (n==6)
	{
		while (v--)
		{
			do_raise_end(cn);
		}
	}
	*/
	else if (n==7)
	{
		while (v--)
		{
			do_raise_mana(cn);
		}
	}
	else
	{
		if (v>135)
		{
			return;				// sanity check
		}
		while (v--)
		{
			do_raise_skill(cn, n - 8);
		}
	}

	do_update_char(cn);
}

void plr_cmd_skill(int nr)
{
	int n, cn, co;
	
	if (player[nr].spectating) return;
	cn = player[nr].usnr;

	n  = *(unsigned long*)(player[nr].inbuf + 1);
	co = *(unsigned long*)(player[nr].inbuf + 5);

	if (n<0 || n>99)                                               return; // sanity checks
	if (co<0 || co>=MAXCHARS)                                      return;
	if (n!=50 && n!=51 && n!=52 && n!=53 && n!=54 && !B_SK(cn, n)) return;

	ch[cn].skill_nr = n;
	ch[cn].skill_target1 = co;
}

void plr_cmd_input1(int nr)
{
	int n;
	for (n = 0; n<15; n++) player[nr].input[n] = player[nr].inbuf[n + 1];
}

void plr_cmd_input2(int nr)
{
	int n;
	for (n = 0; n<15; n++) player[nr].input[n + 15] = player[nr].inbuf[n + 1];
}

void plr_cmd_input3(int nr)
{
	int n;
	for (n = 0; n<15; n++) player[nr].input[n + 30] = player[nr].inbuf[n + 1];
}

void plr_cmd_input4(int nr)
{
	int n;
	for (n = 0; n<15; n++) player[nr].input[n + 45] = player[nr].inbuf[n + 1];
}

void plr_cmd_input5(int nr)
{
	int n;
	for (n = 0; n<15; n++) player[nr].input[n + 60] = player[nr].inbuf[n + 1];
}

void plr_cmd_input6(int nr)
{
	int n;
	for (n = 0; n<15; n++) player[nr].input[n + 75] = player[nr].inbuf[n + 1];
}

void plr_cmd_input7(int nr)
{
	int n;
	for (n = 0; n<15; n++) player[nr].input[n + 90] = player[nr].inbuf[n + 1];
}

void plr_cmd_input8(int nr)
{
	int n;
	for (n = 0; n<15; n++) player[nr].input[n + 105] = player[nr].inbuf[n + 1];
	
	player[nr].input[105 + 14] = 0;
	
	do_say(player[nr].usnr, player[nr].input);
}

void plr_cmd_setuser(int nr)
{
	int n, cn, pos, flag;
	char *reason = NULL;
	char *race_name;
	
	if (player[nr].spectating) return;
	cn = player[nr].usnr;

	pos = player[nr].inbuf[2];
	if (pos<0 || pos>65)
	{
		return;
	}

	switch(player[nr].inbuf[1])
	{
		case    0:
			for (n = 0; n<13; n++)
				ch[cn].text[0][n + pos] = player[nr].inbuf[n + 3];
			break;
		case    1:
			for (n = 0; n<13; n++)
				ch[cn].text[1][n + pos] = player[nr].inbuf[n + 3];
			break;
		case    2:
			for (n = 0; n<13; n++)
				ch[cn].text[2][n + pos] = player[nr].inbuf[n + 3];
			if (pos!=65)
				break;
			
			if (strlen(ch[cn].text[0])>2 &&
				strlen(ch[cn].text[0])<38 &&
				(ch[cn].flags & CF_NEWUSER))
			{

				flag = 0;

				for (n = 0; ch[cn].text[0][n]; n++)
				{
					if (!isalpha(ch[cn].text[0][n]))
					{
						flag = 1;
						break;
					}
					ch[cn].text[0][n] = tolower(ch[cn].text[0][n]);
				}
				if (flag==1)
				{
					reason = "contains non-letters. Please choose a more normal-looking name.";
				}

				// check for bad names here when the string is all lowercase
				if (god_is_badname(ch[cn].text[0]))
				{
					flag = 3;
				}

				ch[cn].text[0][0] = toupper(ch[cn].text[0][0]);

				/* CS, 991030: Reserve "Self" for self-reference. */
				if (!strcmp(ch[cn].text[0], "Self"))
				{
					flag = 2;
				}
				for (n = 1; !flag && n<MAXCHARS; n++)
				{
					if (ch[n].used!=USE_EMPTY && strcmp(ch[cn].text[0], ch[n].name)==0)
					{
						flag = 2;
						break;
					}
				}
				/* CS, 000301: Check for names of mobs in templates */
				for (n = 1; !flag && n<MAXTCHARS; n++) if (!strcmp(ch[cn].text[0], ch_temp[n].name))
				{
					flag = 2;
					break;
				}
				
				if (flag==2)
					reason = "is already in use. Please try to choose another name.";
				
				if (flag==3)
					reason = "is deemed inappropriate. Please try to choose another name.";
				
				if (flag)
					do_char_log(cn, 0, "The name \"%s\" you have chosen for your character %s\n",
								ch[cn].text[0], reason);
				
				else
				{
					strcpy(ch[cn].name, ch[cn].text[0]);
					strcpy(ch[cn].reference, ch[cn].text[0]);
					ch[cn].flags &= ~CF_NEWUSER;
				}
			}
			
			strcpy(ch[cn].description, ch[cn].text[1]);
			
			if (strlen(ch[cn].description)>77)
				strcat(ch[cn].description, ch[cn].text[2]);
			
			reason = NULL;
			
			if (strlen(ch[cn].description)<10)
				reason = "is too short";
			
			/*else if (strstr(ch[cn].description, ch[cn].name)==NULL)
				reason = "does not contain your name";*/
			
			else if (strchr(ch[cn].description, '\"'))
				reason = "contains a double quote";
			
			else if (ch[cn].flags & CF_NODESC)
				reason = "was blocked because you have been known to enter inappropriate descriptions";
			
			if (reason != NULL)
			{
				if (IS_TEMPLAR(cn))          race_name = "a Templar";
				else if (IS_HARAKIM(cn))     race_name = "a Harakim";
				else if (IS_MERCENARY(cn))   race_name = "a Mercenary";
				else if (IS_SEYAN_DU(cn))    race_name = "a Seyan'Du";
				else if (IS_ARCHHARAKIM(cn)) race_name = "an Arch Harakim";
				else if (IS_ARCHTEMPLAR(cn)) race_name = "an Arch Templar";
				else if (IS_WARRIOR(cn))     race_name = "a Warrior";
				else if (IS_SORCERER(cn))    race_name = "a Sorcerer";
				else if (IS_SKALD(cn))       race_name = "a Skald";
				else if (IS_SUMMONER(cn))    race_name = "a Summoner";
				else if (IS_BRAVER(cn))      race_name = "a Braver";
				else if (IS_LYCANTH(cn))     race_name = "a Lycanthrope";
				else                         race_name = "a strange figure";
				
				do_char_log(cn, 0, "The description you entered for your character %s, so it has been rejected.\n", reason);
				sprintf(ch[cn].description, "%s is %s. %s looks somewhat nondescript.", ch[cn].name, race_name, HE_SHE_CAPITAL(cn));
			}
			do_char_log(cn, 1, "Account data received.\n");
			plog(nr, "Account data received");
			do_update_char(cn);
			break;
		default:
			plog(nr, "Unknown setuser subtype %d", player[nr].inbuf[1]);
			break;
	}
}

void plr_idle(int nr)
{
	if (player[nr].spectating) return;
	if (IS_SANECHAR(player[nr].usnr))
	{
		if (IS_BUILDING(player[nr].usnr)) return;
		if (IS_IN_TEMPLE(ch[player[nr].usnr].x, ch[player[nr].usnr].y)) return;
	}
	
	if (globs->ticker - player[nr].lasttick>TICKS * 60)
	{
		plog(nr, "Idle too long (protocol level)");
		plr_logout(player[nr].usnr, nr, LO_IDLE);
	}
	if (player[nr].state==ST_EXIT)
	{
		return;
	}

	if (globs->ticker - player[nr].lasttick2>TICKS * 60 * 15)
	{
		plog(nr, "Idle too long (player level)");
		plr_logout(player[nr].usnr, nr, LO_IDLE);
	}
}

void plr_cmd_exit(int nr)
{
	plog(nr, "Pressed F12");
	plr_logout(player[nr].usnr, nr, LO_EXIT);
}

void plr_cmd_ctick(int nr)
{
	player[nr].rtick = *(unsigned long*)(player[nr].inbuf + 1);
	player[nr].lasttick = globs->ticker;
}

static unsigned int clcmd[255];

void cl_list(void)
{
	int n, m = 0, tot = 0;

	for (n = 0; n<255; n++)
	{
		tot += clcmd[n];
		if (clcmd[n]>m)
		{
			m = clcmd[n];
		}
	}

	for (n = 0; n<255; n++)
	{
		if (clcmd[n]>m / 16)
		{
			xlog("cl type %2d: %5d (%.2f%%)", n, clcmd[n], 100.0 / tot * clcmd[n]);
		}
	}
}

void plr_update_all_meta_terminology(int nr);

// dispatch command.
void plr_cmd(int nr)
{
	int cn;
	
	clcmd[player[nr].inbuf[0]]++;
	
	switch(player[nr].inbuf[0])
	{
		case CL_NEWLOGIN:   plr_challenge_newlogin(nr); break;
		case CL_CHALLENGE:  plr_challenge(nr);          break;
		case CL_LOGIN:      plr_challenge_login(nr);    break;
		case CL_CMD_UNIQUE: plr_unique(nr);             return;
		case CL_PASSWD:     plr_passwd(nr);             break;
		default: break;
	}
	if (player[nr].state!=ST_NORMAL) return;
	
	if (player[nr].inbuf[0] != CL_CMD_AUTOLOOK &&
		player[nr].inbuf[0] != CL_PERF_REPORT  &&
		player[nr].inbuf[0] != CL_CMD_CTICK    )
		player[nr].lasttick2 = globs->ticker;
	
	switch(player[nr].inbuf[0])
	{
		case CL_PERF_REPORT:  plr_perf_report(nr); return;
		case CL_CMD_LOOK:     plr_cmd_look(nr, 0); return;
		case CL_CMD_AUTOLOOK: plr_cmd_look(nr, 1); return;
		case CL_CMD_SETUSER:  plr_cmd_setuser(nr); return;
		case CL_CMD_STAT:     plr_cmd_stat(nr);    return;
		case CL_CMD_INPUT1:   plr_cmd_input1(nr);  return;
		case CL_CMD_INPUT2:   plr_cmd_input2(nr);  return;
		case CL_CMD_INPUT3:   plr_cmd_input3(nr);  return;
		case CL_CMD_INPUT4:   plr_cmd_input4(nr);  return;
		case CL_CMD_INPUT5:   plr_cmd_input5(nr);  return;
		case CL_CMD_INPUT6:   plr_cmd_input6(nr);  return;
		case CL_CMD_INPUT7:   plr_cmd_input7(nr);  return;
		case CL_CMD_INPUT8:   plr_cmd_input8(nr);  return;
		case CL_CMD_CTICK:    plr_cmd_ctick(nr);   return;
		default: break;
	}
	
	cn = player[nr].usnr;
	if (ch[cn].stunned==1) do_char_log(cn, 2, "You have been stunned. You cannot move.\n");
	
	switch(player[nr].inbuf[0])
	{
		case CL_CMD_LOOK_ITEM: plr_cmd_look_item(nr);               return;
		case CL_CMD_GIVE:      plr_cmd_give(nr);                    return;
		case CL_CMD_TURN:      plr_cmd_turn(nr);                    return;
		case CL_CMD_DROP:      plr_cmd_drop(nr);                    return;
		case CL_CMD_PICKUP:    plr_cmd_pickup(nr);                  return;
		case CL_CMD_ATTACK:    plr_cmd_attack(nr);                  return;
		case CL_CMD_MODE:      plr_cmd_mode(nr);                    return;
		case CL_CMD_MOVE:      plr_cmd_move(nr);                    return;
		case CL_CMD_RESET:     plr_cmd_reset(nr);                   return;
		case CL_CMD_SKILL:     plr_cmd_skill(nr);                   return;
		case CL_CMD_INV_LOOK:  plr_cmd_inv_look(nr);                return;
		case CL_CMD_USE:       plr_cmd_use(nr);                     return;
		case CL_CMD_INV:       plr_cmd_inv(nr);                     return;
		case CL_CMD_META:      plr_update_all_meta_terminology(nr); break;
		case CL_CMD_EXIT:      plr_cmd_exit(nr);                    return;
		default: break;
	}
	
	if (ch[cn].stunned==1) return;
	
	switch(player[nr].inbuf[0])
	{
		case CL_CMD_SHOP:   plr_cmd_shop(nr);   break;
		case CL_CMD_SMITH:  plr_cmd_smith(nr);  break;
		case CL_CMD_QSHOP:  plr_cmd_qshop(nr);  break;
		case CL_CMD_WPS:    plr_cmd_wps(nr);    break;
		case CL_CMD_TREE:   plr_cmd_tree(nr);   break;
		case CL_CMD_MOTD:   plr_cmd_motd(nr);   break;
		case CL_CMD_BSSHOP: plr_cmd_bsshop(nr); break;
		default: plog(nr, "Unknown CL: %d", player[nr].inbuf[0]); break;
	}
}

void char_add_net(int cn, unsigned int net)
{
	int n, m;
	
	for (n = 80; n<89; n++) if ((ch[cn].data[n] & 0x00ffffff) == (net & 0x00ffffff))
		break;
	
	for (m = n; m>80; m--) ch[cn].data[m] = ch[cn].data[m - 1];
	
	ch[cn].data[80] = net;
}

void char_remove_net(int cn, int co)
{
	int n;
	
	if (co<=0 || co>=MAXCHARS || !IS_SANECHAR(co))
	{
		do_char_log(cn, 0, "Error: Bad ID.\n");
		return;
	}
	
	for (n = 80; n<89; n++)
	{
		ch[cn].data[n] = 0;
	}
	
	do_char_log(cn, 1, "Done.\n");
}

void char_remove_same_nets(int cn, int co)
{
	int cc, n, m;
	
	if (co<=0 || co>=MAXCHARS || !IS_SANECHAR(co))
	{
		do_char_log(cn, 0, "Error: Bad ID.\n");
		return;
	}
	
	for (cc = 1; cc<MAXCHARS; cc++)
	{
		if (ch[cc].used==USE_EMPTY || !IS_SANEPLAYER(cc)) continue;
		for (n = 80; n<89; n++)
		{
			if (ch[cc].data[n]==0) continue;
			for (m = 80; m<89; m++)
			{
				if (ch[co].data[m]==0) continue;
				if (ch[cc].data[n]==ch[co].data[m])
				{
					ch[cc].data[n] = 0;
				}
			}
		}
	}
	
	do_char_log(cn, 1, "Done.\n");
}

void plr_update_treenode_terminology(int nr, int tn, int n)
{
	unsigned char buf[256];
	int cn = player[nr].usnr, m = 0, j;
	
	if (tn < 0) return;
	if (tn > 9) return;
	
	if (tn == 9)
	{
		buf[0] = SV_TERM_CTREE;
	}
	else
	{
		buf[0] = SV_TERM_STREE;
		m = ch[cn].tree_node[n];
	}
	
	buf[2] = n;
	
	buf[1] = ST_TREE_ICON;
	if (m) *(unsigned short*)(buf + 3) = (unsigned short)(sk_corrupt[m-1].icon);
	else   *(unsigned short*)(buf + 3) = (unsigned short)(sk_tree[tn][n].icon);
	xsend(nr, buf,  5);
	
	for (j=0; j<3; j++)
	{
		buf[1] = ST_TREE_NAME+j;
		if (m) mcpy(buf+3, sk_corrupt[m-1].name+j*10, 10);
		else   mcpy(buf+3,  sk_tree[tn][n].name+j*10, 10);
		xsend(nr, buf, 13);
	}
	
	for (j=0; j<5; j++)
	{
		buf[1] = ST_TREE_DESC1+j;
		if (m) mcpy(buf+3, sk_corrupt[m-1].dsc1+j*10, 10);
		else   mcpy(buf+3,  sk_tree[tn][n].dsc1+j*10, 10);
		xsend(nr, buf, 13);
	}
	
	for (j=0; j<5; j++)
	{
		buf[1] = ST_TREE_DESC2+j;
		if (m) mcpy(buf+3, sk_corrupt[m-1].dsc2+j*10, 10);
		else   mcpy(buf+3,  sk_tree[tn][n].dsc2+j*10, 10);
		xsend(nr, buf, 13);
	}
}

void plr_update_tree_terminology(int nr, int val)
{
	int tn = -1, n = 0;
	int cn = player[nr].usnr;
	
	if (val==SV_TERM_STREE)
	{
		     if (IS_SEYAN_DU(cn))    tn = 0;
		else if (IS_ARCHTEMPLAR(cn)) tn = 1;
		else if (IS_SKALD(cn))       tn = 2;
		else if (IS_WARRIOR(cn))     tn = 3;
		else if (IS_SORCERER(cn))    tn = 4;
		else if (IS_SUMMONER(cn))    tn = 5;
		else if (IS_ARCHHARAKIM(cn)) tn = 6;
		else if (IS_BRAVER(cn))      tn = 7;
		else if (IS_LYCANTH(cn))     tn = 8;
	}
	if (val==SV_TERM_CTREE) tn = 9;
	
	if (tn >= 0)
	{
		for (n = 0; n < 12; n++)
			plr_update_treenode_terminology(nr, tn, n);
	}
}

char get_known_player_skill(int cn, int n)
{
	if (n<0 || n>=55) return 0; // 0 = not known
	
	if (n>=50) // Orange meta skill
	{
		if ((n==52) && !(ch[cn].kindred & KIN_IDENTIFY)) return 0; // 0 = not known
		if ((n==53||n==54) && !IS_LYCANTH(cn)) return 0; // 0 = not known
		
		return 2; //  2 = show in orange
	}
	
	if (!B_SK(cn, n)) // We don't know this skill
	{	// Stealth, Resist, Regen, Rest, Medit, Immun -- these are active even if you don't know them.
		if (n==8||n==23||n==28||n==29||n==30||n==32) return 4; //  4 = show in red
		if (n==44 && IS_SEYAN_DU(cn)) return 4; //  4 = show in red
		
		return 0; // 0 = not known
	}
	
	return 1; //  1 = show in yellow
}

int ch_get_skill_attrib(int cn, int sk, int n)
{
	int atr;
	
	atr = skilltab[sk].attrib[n];
	
	if ((sk == SK_BLESS || sk == SK_ENHANCE || sk == SK_HASTE || sk == SK_PROTECT || sk == SK_MSHIELD) && IS_LYCANTH(cn))
	{
		if (n == 0) atr = AT_BRV;
		if (n == 1) atr = AT_WIL;
		if (n == 2) atr = AT_AGL;
	}
	
	return atr;
}

void plr_update_skill_terminology(int nr, int n)
{
	unsigned char buf[256];
	int cn = player[nr].usnr;
	int alt = 0, m;
	char known;
	
	known = get_known_player_skill(cn, n);
	
	buf[0] = SV_TERM_SKILLS;
	buf[2] = n;
	
	buf[1] = ST_SKILLS_SORT;
	*(unsigned char*)(buf + 3) = (unsigned char)skilltab[n].sortkey;
	*(unsigned char*)(buf + 4) = (unsigned char)(GET_AT(cn, n, 0));
	*(unsigned char*)(buf + 5) = (unsigned char)(GET_AT(cn, n, 1));
	*(unsigned char*)(buf + 6) = (unsigned char)(GET_AT(cn, n, 2));
	*(unsigned char*)(buf + 7) = (unsigned char)known;
	xsend(nr, buf,  8);
	
	if (n==11 && do_get_iflag(cn, SF_EMPRESS))    alt = 1; // Magic Shield -> Magic Shell
	if (n==12 && do_get_iflag(cn, SF_PREIST_R))   alt = 1; // Tactics invert
	if (n==16 && do_get_iflag(cn, SF_SHIELDBASH)) alt = 1; // Shield -> Shield Bash
	if (n==19 && do_get_iflag(cn, SF_EMPEROR))    alt = 1; // Slow -> Greater Slow
	if (n==20 && do_get_iflag(cn, SF_TOWER))      alt = 1; // Curse -> Greater Curse
	if (n==22 && do_get_iflag(cn, SF_HERMIT_R))   alt = 1; // Pact HP -> Mana
	if (n==24 && do_get_iflag(cn, SF_JUDGE))      alt = 1; // Blast -> +Scorch
	if (n==26 && do_get_iflag(cn, SF_STAR))       alt = 1; // Heal -> Regen
	if (n==35 && do_get_iflag(cn, SF_EMPERO_R))   alt = 1; // Warcry -> Rally
	if (n==37 && do_get_iflag(cn, SF_CHARIOT))    alt = 1; // Blind -> Douse
	if (n==40 && do_get_iflag(cn, SF_JUSTICE))    alt = 1; // Cleave -> +Aggravate
	if (n==41 && do_get_iflag(cn, SF_DEATH))      alt = 1; // Weaken -> Greater Weaken
	if (n==42 && do_get_iflag(cn, SF_TOWER_R))    alt = 1; // Poison -> Venom
	if (n==43 && do_get_iflag(cn, SF_JUDGE_R))    alt = 1; // Pulse -> Healing Pulses
	if (n==49 && do_get_iflag(cn, SF_JUSTIC_R))   alt = 1; // Leap
	
	for (m=0; m<3; m++)
	{
		buf[1] = ST_SKILLS_NAME+m;
		if (alt) mcpy(buf+3, skilltab[n].alt_name+m*10, 10);
		else     mcpy(buf+3, skilltab[n].name+m*10,     10);
		xsend(nr, buf, 13);
	}
	
	for (m=0; m<12; m++)
	{
		buf[1] = ST_SKILLS_DESC+m;
		if (alt) mcpy(buf+3, skilltab[n].alt_desc+m*10, 10);
		else     mcpy(buf+3, skilltab[n].desc+m*10,     10);
		xsend(nr, buf, 13);
	}
}

void ch_update_skill_terminology(int cn, int n)
{
	plr_update_skill_terminology(ch[cn].player, n);
}

void plr_update_all_skill_terminology(int nr)
{
	for (int n=0; n<(MAXSKILL+5); n++) plr_update_skill_terminology(nr, n);
}

int get_meta_stat_value(int cn, int n)
{
	int m, in = 0, power=0, durat=0, value = -1, cdlen = 100;
	int hpmult, endmult, manamult, moonmult = 20;
	int race_reg = 0, race_res = 0, race_med = 0;
	int dmg_wpn=0, dmg_low=0, dmg_hgh=0, dmg_top=0;
	int dmg_hit=0, dmg_dps=0, dmg_bns=0, dmg_str=0;
	int regen = 0, restn = 0, medit = 0;
	int en_dam, mp_dam;
	
	switch (n) // Regen set
	{
		case 51: case 52: case 53:
			if (IS_GLOB_MAYHEM)				moonmult = 10;
			if (globs->fullmoon)			moonmult = 30;
			if (globs->newmoon)				moonmult = 40;
			
			hpmult = endmult = manamult = moonmult;
			
			race_reg = M_SK(cn, SK_REGEN) * moonmult / 20 + M_SK(cn, SK_REGEN) * ch[cn].hp[5]  /2000;
			race_res = M_SK(cn, SK_REST)  * moonmult / 20 + M_SK(cn, SK_REST)  * ch[cn].end[5] /1000;
			race_med = M_SK(cn, SK_MEDIT) * moonmult / 20 + M_SK(cn, SK_MEDIT) * ch[cn].mana[5]/2000;
			
			if (do_get_iflag(cn, SF_MOON)  && (ch[cn].a_mana < ch[cn].mana[5] * 1000)) // Tarot - Moon
			{ race_med += race_reg;  race_reg -= race_reg;  manamult += hpmult;    hpmult   -= hpmult; }
			if (do_get_iflag(cn, SF_SUN)   && (ch[cn].a_hp   < ch[cn].hp[5]   * 1000)) // Tarot - Sun
			{ race_reg += race_res;  race_res -= race_res;  hpmult   += endmult;   endmult  -= endmult; }
			if (do_get_iflag(cn, SF_WORLD) && (ch[cn].a_end  < ch[cn].end[5]  * 1000)) // Tarot - World
			{ race_res += race_med;  race_med -= race_med;  endmult  += manamult;  manamult -= manamult; }
			
			if (do_get_iflag(cn, SF_EN_MEDIREGN)) // Meditate added to Hitpoints
			{ race_reg += race_med/2;  hpmult   += manamult/2; }
			if (do_get_iflag(cn, SF_EN_RESTMEDI)) // Rest added to mana
			{ race_med += race_res/2;  manamult += endmult/2; }
			
			regen = race_reg + hpmult   * 2;
			restn = race_res + endmult  * 3;
			medit = race_med + manamult * 1;
			
			if ((in = ch[cn].worn[WN_NECK])) switch (it[in].temp)
			{
				case IT_ANKHAMULET: regen += (race_reg/ 8); restn += (race_res/ 8); medit += (race_med/ 8); break;
				case IT_AMBERANKH:  regen += (race_reg/ 4); restn += (race_res/12); medit += (race_med/12); break;
				case IT_TURQUANKH:  regen += (race_reg/12); restn += (race_res/ 4); medit += (race_med/12); break;
				case IT_GARNEANKH:  regen += (race_reg/12); restn += (race_res/12); medit += (race_med/ 4); break;
				case IT_TRUEANKH:   regen += (race_reg/ 4); restn += (race_res/ 4); medit += (race_med/ 4); break;
				default: break;
			}
			if ((in = get_gear(cn, IT_RINGWARMTH)) && it[in].active)
			{ regen += (race_reg/ 8); restn += (race_res/ 8); medit += (race_med/ 8); }
			break;
		default: break;
	}
	
	switch (n) // Melee set
	{
		case  9: case 10: case 13: case 14: case 17: case 58:
			dmg_wpn = ch[cn].weapon;
			dmg_top = ch[cn].top_damage;
			dmg_str = do_get_iflag(cn, SF_STRENGTH)?120:100;
			dmg_bns = ch[cn].dmg_bonus;
			//
			dmg_low = (  dmg_wpn          * dmg_str/100 * dmg_bns/10000 ) / 4;
			dmg_hgh = ( (dmg_wpn+dmg_top) * dmg_str/100 * dmg_bns/10000 );
			if (T_ARTM_SK(cn, 6))  // (ArTm) Impact
				dmg_low = dmg_hgh/4;
			dmg_hgh = ( dmg_hgh + dmg_hgh*ch[cn].crit_chance*ch[cn].crit_multi/1000000 ) / 4;
			dmg_hit = (dmg_low+dmg_hgh+1)/2;
			dmg_dps = dmg_hit*max(0, min(SPEED_CAP, SPEED_BASE+GET_SPD_ATK(cn)));
			break;
		default: break;
	}
	
	switch (n) // Cooldown set
	{
		case 0: case 26: case 28: case 29: case 33: case 36: case 38: 
		case 39: case 44: case 47: case 75: case 77: case 79: case 81: 
		case 83: case 85: case 88: case 91: case 97: case 99: case 101:
		case 104: case 106:
			cdlen = 100 * (do_get_iflag(cn, SF_BOOK_DAMO)?90:100) / max(25, ch[cn].cool_bonus);
			if (IS_IT_TEMP(ch[cn].worn[WN_RHAND], IT_TW_ACEDIA)) cdlen = cdlen * 3/4; // Acedia less
			if (IS_IT_TEMP(ch[cn].worn[WN_LHAND], IT_TW_ACEDIA)) cdlen = cdlen * 6/4; // Acedia more
			break;
		default: break;
	}
	
	switch (n) // M.Shield and M.Shell duration
	{
		case 68: case 69: case 93: case 94:
			power = spell_multiplier(M_SK(cn, SK_REGEN), cn);
			durat = do_get_iflag(cn, SF_EMPRESS)?SP_DUR_MSHELL(power):SP_DUR_MSHIELD(power);
			break;
		default: break;
	}
	
	switch (n) // Pact power from hp/en/mp
	{
		case 30: case 71:
			power = skill_multiplier(M_SK(cn, SK_PACT), cn);
			if (do_get_iflag(cn, SF_HERMIT_R))
				power = has_buff(cn, SK_PACT)?ch[cn].reserve[2]:min(95, min(80, 15+power/5)+ch[cn].reserve[2]);
			else
				power = has_buff(cn, SK_PACT)?ch[cn].reserve[0]:min(95, min(80, 15+power/5)+ch[cn].reserve[0]);
			
			power = more(power, ch[cn].hp[5] * (T_LYCA_SK(cn,9)*2 + TC_SK(cn,105)), 100);  // (Lyca) Wrath
			
			break;
		default: break;
	}
	
	switch (n)
	{
		case  0: // Cooldown Duration				Decimal, 0.00 x
			value = cdlen;
			break;
		case  1: // Spell Aptitude
			value = ch[cn].spell_apt;
			break;
		case  2: case 89: // Spell Bonus
			value = ch[cn].spell_pow;
			break;
		case  3: // Spell Multiplier				Decimal, 0.00 x
			value = ch[cn].spell_mod;
			break;
		case  4: // Base Action Speed				Decimal, 0.00
			value = max(0, min(SPEED_CAP, (SPEED_CAP-ch[cn].speed)));
			break;
		case  5: // Hit Score
			value = ch[cn].to_hit;
			break;
		case  6: // Parry Score
			value = ch[cn].to_parry;
			break;
		//
		case  8: // Damage Multiplier				Decimal, 0.00 %
			value = ch[cn].dmg_bonus;
			break;
		case  9: // Est. Melee DPS					Decimal, 0.00
			value = dmg_dps;
			break;
		case 10: // Est. Melee Hit Dmg
			value = dmg_hit;
			break;
		case 11: // Critical Multiplier
			value = ch[cn].crit_multi;
			break;
		case 12: // Critical Chance					Decimal, 0.00 %
			value = ch[cn].crit_chance;
			break;
		case 13: // Melee Ceiling Damage
			value = dmg_hgh+2;
			break;
		case 14: // Melee  Floor  Damage
			value = dmg_low;
			break;
		case 15: // Attack Speed					Decimal, 0.00
			value = max(0, min(SPEED_CAP, SPEED_BASE+GET_SPD_ATK(cn)));
			break;
		case 56: // Movement Speed					Decimal, 0.00
			value = max(0, min(SPEED_CAP, SPEED_BASE+GET_SPD_MOV(cn)));
			break;
		case 16: case 57: //   Cast Speed			Decimal, 0.00
			value = max(0, min(SPEED_CAP, SPEED_BASE+GET_SPD_CAS(cn)));
			break;
		case 17: case 58: // Thorns Score
			value = ch[cn].gethit_dam * dmg_str/100 * dmg_bns/10000;
			break;
		case 18: case 59: // Mana Cost Multiplier	Decimal, 0.00 %
			value = ch[cn].mana_cost;
			break;
		case 19: case 60: // Total AoE Bonus		Decimal, 0.00 %
			value = ch[cn].aoe_bonus*50 + ch[cn].aoe_bonus*50 * GET_PROX(cn)/400;
			break;
		//
		case 24: // Cleave Hit Damage
			power = skill_multiplier(M_SK(cn, SK_CLEAVE) + ch[cn].weapon/4 + ch[cn].top_damage/4, cn)*2;
			power = more(power, M_AT(cn, AT_STR) * TC_SK(cn,112), 20);  // (Corr) Conquest
			value = power * DAM_MULT_CLEAVE/1000;
			break;
		case 25: // Cleave Bleed Degen				Decimal, 0.00 /s
			power = skill_multiplier(M_SK(cn, SK_CLEAVE) + ch[cn].weapon/4 + ch[cn].top_damage/4, cn)*2;
			power = more(power, M_AT(cn, AT_STR) * TC_SK(cn,112), 20);  // (Corr) Conquest
			if (do_get_iflag(cn, SF_EN_MOREBLEE)) power = more(power, 20, 1);                  // [Ench] More Bleed
			if (T_LYCA_SK(cn, 12))                power = more(power, ch[cn].gethit_dam, 1);   // (Lyca) Serration
			durat = 0;
			durat += TC_SK(cn,113)*15; // (Corr) Torment
			durat = SP_DUR_BLEED * 100 / (100 + durat);
			value = BLEEDFORM(power, (do_get_iflag(cn, SF_GUNGNIR)?durat/3:durat));
			break;
		case 26: // Cleave Cooldown					Decimal, 0.00 Seconds
			value = 5 * cdlen;
			break;
		case 27: // Leap Hit Damage
			power = skill_multiplier(M_SK(cn, SK_LEAP) + ch[cn].weapon/4 + ch[cn].top_damage/4, cn) * 2;
			power = more(power, M_AT(cn, AT_STR) * (T_WARR_SK(cn, 7)*2+TC_SK(cn, 43)), 20);  // (Warr) Slayer
			value = power * DAM_MULT_RLEAP/1000;
			if (do_get_iflag(cn, SF_JUSTIC_R)) value = power * ch[cn].crit_multi / 100 * DAM_MULT_LEAP/1000;
			break;
		case 28: // Leap # of Repeats
			value = max(0, GET_SPD_ATK(cn)/60);
			if (do_get_iflag(cn, SF_SIGN_SLAY)) value++;
			if (T_WARR_SK(cn,  6))              value++;  // (Warr) Flash Step
			break;
		case 29: // Leap Cooldown					Decimal, 0.00 Seconds
			value = 5 * cdlen;
			break;
		case 30: // Pact Dmg Bonus								Decimal, 0.00 %
			if (do_get_iflag(cn, SF_HERMIT_R)) value = 10000+(power*2/6)*50;
			else                               value = 10000+(power*2/3)*50;
			break;
		case 31: case 72: case 102: // Pact HP/MP Reserve
			power = skill_multiplier(M_SK(cn, SK_PACT), cn);
			value = min(80, 15+power/5);
			break;
		case 32: // Blast Hit Damage
			power = spell_multiplier(M_SK(cn, SK_BLAST), cn) * 2;
			if (do_get_iflag(cn, SF_TW_IRA))   in = power * ch[cn].crit_multi / 100 - power;
			if (do_get_iflag(cn, SF_JUDGE)) power = (power+max(0, in))*85/100;
			else                            power =  power+max(0, in);
			value = power * DAM_MULT_BLAST/1000;
			break;
		case 33: // Blast Cooldown					Decimal, 0.00 Seconds
			value = 6 * cdlen;
			break;
		case 34: // Lethargy Effect											// Flipped to Positive
			power = spell_multiplier(M_SK(cn, SK_LETHARGY), cn);
			power = more(power, M_AT(cn, AT_WIL) * (T_SORC_SK(cn, 9)*2+TC_SK(cn, 57)), 20);  // (Sorc) Hex Master
			if (IS_SEYAN_DU(cn)) value = (bu[in].power/6);
			else                 value = (bu[in].power/4);
			break;
		case 35: case 90: // Poison/Venom Degen		Decimal, 0.00 /s
			power = spell_multiplier(M_SK(cn, SK_POISON), cn);
			power = more(power, M_AT(cn, AT_INT) * (T_SORC_SK(cn, 7)*2+TC_SK(cn, 55)), 20);  // (Sorc) Toxins
			if (do_get_iflag(cn, SF_EN_MOREPOIS)) power = power*6/5;
			durat = 0;
			durat += TC_SK(cn,113)*15; // (Corr) Torment
			if (do_get_iflag(cn, SF_BOOK_VENO)) durat += 25;  // [Book] Venom Compendium
			durat = SP_DUR_POISON * 100 / (100 + durat);
			value = PL_POISFORM(power, durat);
			if (do_get_iflag(cn, SF_TOWER_R))     value /= 2;
			break;
		case 36: case 91: // Poison/Venom Cooldn	Decimal, 0.00 Seconds
			value = 5 * cdlen;
			break;
		case 37: case 92: // Pulse Hit Damage/Heal	Decimal, 0.00
			power = M_SK(cn, SK_PULSE);
			power = more(power, M_AT(cn, AT_INT) * (T_ARHR_SK(cn,  7)*2+TC_SK(cn, 79)), 20);  // (ArHr) Psychosis
			power = spell_multiplier(power, cn);
			if (n==37) value = power * 2;
			else       value = power * DAM_MULT_PULSE / 20;
			break;
		case 38: // Pulse Count
			value = 60*2*100 / max(1, 3 * cdlen);
			break;
		case 39: // Pulse Cooldown					Decimal, 0.00 Seconds
			value = 6 * cdlen;
			break;
		case 40: // Zephyr Hit Damage				Decimal, 0.00
			power = spell_multiplier(M_SK(cn, SK_ZEPHYR) + ch[cn].spell_pow, cn) + ch[cn].weapon / 8 + ch[cn].top_damage / 8;
			power = more(power, M_AT(cn, AT_BRV)*(T_BRAV_SK(cn, 9)*2+TC_SK(cn, 93)), 20);  // (Brav) Alacrity
			value = power * DAM_MULT_ZEPHYR/10;
			break;
		case 41: // Immolate Degen					Decimal, 0.00 /s
			power = ch[cn].hp[4] * 30 / 100;  // 30% uncapped HP as power
			power = more(power*3, (TC_SK(cn, 34)*15), 1);
			value = max(100, 100 + power*4);
			value = do_immol_leech_dam(cn, value) * 20 / 10;
			break;
		case 43: case 84: // Ghost Comp Potency
			power = spell_multiplier(M_SK(cn, SK_GHOST), cn);
			value = power * 5 / 11;
			break;
		case 44: case 85: // Ghost Comp Cooldown	Decimal, 0.00 Seconds
			value = 8 * cdlen;
			break;
		case 45: case 86: // Shadow Copy Potency
			power = spell_multiplier(M_SK(cn, SK_SHADOW), cn);
			value = power;
			break;
		case 46: case 87: // Shadow Copy Duration	Decimal, 0.00 Seconds
			power = spell_multiplier(M_SK(cn, SK_SHADOW), cn);
			value = SP_DUR_SHADOW(power);
			break;
		case 47: case 88: // Shadow Copy Cooldown	Decimal, 0.00 Seconds
			value = 4 * cdlen;
			break;
		//
		case 49: // Damage Reduction				Decimal, 0.00 %
			value = ch[cn].dmg_reduction;
			break;
		case 50: // Effective Hitpoints
			value = HP_SOFTCAP(cn)*10 / max(1, ch[cn].dmg_reduction);
			en_dam = mp_dam = 0;
			if (T_WARR_SK(cn, 12))                 en_dam +=   30;    // (Warr) Tenacity
			if ((m=TC_SK(cn, 48)))                 en_dam += m*15;
			if (do_get_iflag(cn, SF_TW_CLOAK))     en_dam +=   15;    // [Gear] Cloak of Shadows
			if (do_get_iflag(cn, SF_EN_TAKEASEN))  en_dam +=   15;    // [Ench] *DoT* damage taken as endurance
			if (T_ARHR_SK(cn, 12))                 mp_dam +=   30;    // (ArHr) Resourcefulness
			if ((m=TC_SK(cn, 84)))                 mp_dam += m*15;
			if (do_get_iflag(cn, SF_PREIST))       mp_dam +=   30;    // [Taro] Priestess
			if (do_get_iflag(cn, SF_EN_TAKEASMA))  mp_dam +=   15;    // [Ench] *Hit* damage taken as mana
			if ((m = en_dam + mp_dam))
			{
				en_dam = ( (en_dam * 100)/m * min(90, m) )/100;
				mp_dam = ( (mp_dam * 100)/m * min(90, m) )/100;
			}
			value = value + en_dam*EN_SOFTCAP(cn)/(ch[cn].dmg_reduction*10);
			value = value + mp_dam*MP_SOFTCAP(cn)/(ch[cn].dmg_reduction*10);
			break;
		case 51: // Health Regen Rate				Decimal, 0.00 /s
			value = regen * 20/10;
			value = more(value, T_LYCA_SK(cn, 7)*50+TC_SK(cn, 103)*25, 1);  // (Lyca) Lust
			if (do_get_iflag(cn, SF_WORLD_R)) value = less(value, 50, 1);  // [Taro] World.R
			break;
		case 52: // Endurance Regen Rate			Decimal, 0.00 /s
			value = restn * 20/10;
			value = more(value, T_LYCA_SK(cn, 7)*50+TC_SK(cn, 103)*25, 1);  // (Lyca) Lust
			if (do_get_iflag(cn, SF_WORLD_R)) value = less(value, 50, 1);  // [Taro] World.R
			break;
		case 53: // Mana Regen Rate					Decimal, 0.00 /s
			value = medit * 20/10;
			value = more(value, T_LYCA_SK(cn, 7)*50+TC_SK(cn, 103)*25, 1);  // (Lyca) Lust
			if (do_get_iflag(cn, SF_WORLD_R)) value = less(value, 50, 1);  // [Taro] World.R
			break;
		case 54: // Effective Immunity
			value = M_SK(cn, SK_IMMUN);
			if (do_get_iflag(cn, SF_HANGED)) value += M_SK(cn, SK_RESIST)/3;
			value = more(value, (SPEED_BASE+GET_SPD_ATK(cn))*(T_WARR_SK(cn, 4)*2+TC_SK(cn, 40)), 20);  // (Warr) Dismissal
			if (do_get_iflag(cn, SF_AM_ECLIPSE)) value = more(n, 25, 1);
			break;
		case 55: // Effective Resistance
			value = M_SK(cn, SK_RESIST);
			if (do_get_iflag(cn, SF_HANGED)) value -= M_SK(cn, SK_RESIST)/3;
			value = more(value, (SPEED_BASE+GET_SPD_MOV(cn))*(T_SORC_SK(cn, 12)*2+TC_SK(cn, 60)), 20);  // (Sorc) Dodging
			break;
		case 61: // Buffing Apt Bonus
			value = M_AT(cn, AT_WIL)/4;
			break;
		case 62: // Underwater Degen				Decimal, 0.00 /s
			value = spell_metabolism(250, get_target_metabolism(cn)) * 20/10;
			if (do_get_iflag(cn, SF_WBREATH)) value /= 4;
			break;
		case 65: // Bless Effect
			power = spell_multiplier(M_SK(cn, SK_BLESS), cn);
			power = more(power, M_AT(cn, AT_BRV)*TC_SK(cn, 96), 20);  // (Corr) Guardian Angel
			value = min(127, (power*2/3) / 5 + 3);
			break;
		case 66: // Enhance Effect
			power = spell_multiplier(M_SK(cn, SK_ENHANCE), cn);
			if (IS_SEYA_OR_BRAV(cn)) value = min(127, power / 6 + 3);
			else                     value = min(127, power / 4 + 4);
			break;
		case 67: // Protect Effect
			power = spell_multiplier(M_SK(cn, SK_PROTECT), cn);
			if (IS_SEYA_OR_BRAV(cn)) value = min(127, power / 6 + 3);
			else                     value = min(127, power / 4 + 4);
			break;
		case 68: case 93: // M.Shield/Shell Effect
			if (do_get_iflag(cn, SF_EMPRESS)) value = min(127, durat / (IS_SEYA_OR_BRAV(cn)? 768: 512) + 1);
			else                              value = min(127, durat / (IS_SEYA_OR_BRAV(cn)?1536:1024) + 1);
			break;
		case 69: case 94: // M.Shield/Shell Dur		Decimal, 0.00 Seconds
			value = durat / 20;
			break;
		case 70: // Haste Effect
			power = spell_multiplier(M_SK(cn, SK_HASTE), cn);
			if (T_SORC_SK(cn, 10))    // (Sorc) Fast Forward
			{
				power = more(power, (TC_SK(cn, 34)*15), 1);  // (Corr) Towering Presence
				power = less(power, 25, 1);
			}
			value = min(300,10+(power)/6)+min(127,5+(power+6)/12);
			break;
		case 71: // Pact Dmg Taken								Decimal, 0.00 %
			if (do_get_iflag(cn, SF_HERMIT_R)) value = 10000-(power*4/6)*50;
			else                               value = 10000-(power*4/3)*50;
			break;
		case 73: case 95: // Heal/Regen Effect
			power = M_SK(cn, SK_HEAL);
			if (do_get_iflag(cn, SF_EN_MOREHEAL)) power = more(power, 20, 1);
			if (do_get_iflag(cn, SF_TW_SUPERBIA)) power = less(power, 50, 1);
			if (do_get_iflag(cn, SF_STAR))        value = spell_multiplier(power, cn)*1875/max(1, SP_DUR_REGEN * 20);
			else                                  value = spell_multiplier(power * 4/5, cn);
			break;
		case 74: // Blind Effect											// Flipped to Positive
			power = skill_multiplier(M_SK(cn, SK_BLIND), cn);
			power = more(power, M_AT(cn, AT_AGL)*(T_WARR_SK(cn, 9)*2+TC_SK(cn, 45)), 20);  // (Warr) Antagonizer
			if (do_get_iflag(cn, SF_EN_MOREBLIN)) power = more(power, 20, 1);
			if (IS_ANY_MERC(cn)) value = min(127, (power/6 + 2));
			else                 value = min(127, (power/8 + 1));
			break;
		case 75: case 97: // Blind/Douse Cooldown					Decimal, 0.00 Seconds
			value = 3 * cdlen;
			break;
		case 76: // Warcry Effect											// Flipped to Positive
			power = skill_multiplier(M_SK(cn, SK_WARCRY), cn);
			power = more(power, M_AT(cn, AT_STR)*(T_ARTM_SK(cn, 9)*2+TC_SK(cn, 21)), 20);  // (ArTm) Overlord
			if (IS_ARCHTEMPLAR(cn)) value = (4+(power*5/8) / 5);
			else                    value = (3+(power  /2) / 5);
			break;
		case 77: case 99: // Warcry/Rally Cooldn	Decimal, 0.00 Seconds
			value = 3 * cdlen;
			break;
		case 78: case 100: // Weaken/Crush Effect							// Flipped to Positive
			power = skill_multiplier(M_SK(cn, SK_WEAKEN), cn);
			power = more(power, M_AT(cn, AT_AGL) * TC_SK(cn,1111), 20);  // (Corr) Burden
			if (do_get_iflag(cn, SF_EN_MOREWEAK)) power = more(power, 20, 1);
			if (T_LYCA_SK(cn,  4))    // (Lyca) Sickness
			{
				power = more(power, (TC_SK(cn, 34)*15), 1);  // (Corr) Towering Presence
				power = less(power, 25, 1);
			}
			value = min(127, (power / 4 + 4));
			break;
		case 79: case 101: // Weaken/Crush Cooldn	Decimal, 0.00 Seconds
			value = 3 * cdlen;
			break;
		case 80: case 103: // Curse/Focus Effect									// Flipped to Positive
			power = spell_multiplier(M_SK(cn, SK_CURSE), cn);
			power = more(power, M_AT(cn, AT_INT) * TC_SK(cn,110), 20);  // (Corr) Famine
			if (do_get_iflag(cn, SF_EN_MORECURS)) power = more(power, 20, 1);
			if (T_BRAV_SK(cn,  6))    // (Brav) Presence
			{
				power = more(power, (TC_SK(cn, 34)*15), 1);  // (Corr) Towering Presence
				power = less(power, 25, 1);
			}
			if (do_get_iflag(cn, SF_TOWER))       value = min(127, power/4 + 1);
			else                                  value = (3 + (power - 4) / 5);
			break;
		case 81: case 104: // Curse/Stymie Cooldown					Decimal, 0.00 Seconds
			value = 4 * cdlen;
			break;
		case 82: case 105: // Slow/Stymie Effect										// Flipped to Positive
			power = spell_multiplier(M_SK(cn, SK_SLOW), cn);
			power = more(power, M_AT(cn, AT_WIL) * TC_SK(cn,109), 20);  // (Corr) Shackle
			if (do_get_iflag(cn, SF_EN_MORESLOW)) power = more(power, 20, 1);
			if (T_SORC_SK(cn,  6))    // (Sorc) Rewind
			{
				power = more(power, (TC_SK(cn, 34)*15), 1);  // (Corr) Towering Presence
				power = less(power, 25, 1);
			}
			if (do_get_iflag(cn, SF_EMPEROR))     value = max(-127, -(power/4 + 1));
			else                                  value = (min(300, 30 + SLOWFORM(power)));
			break;
		case 83: case 106: // Slow/Focus Cooldown					Decimal, 0.00 Seconds
			value = 4 * cdlen;
			break;
		//
		case 96: // Douse Effect					Decimal, 0.00 %			// Flipped to Positive
			power = skill_multiplier(M_SK(cn, SK_BLIND), cn);
			power = more(power, M_AT(cn, AT_AGL)*(T_WARR_SK(cn,  9)*2+TC_SK(cn, 45)), 20);  // (Warr) Antagonizer
			if (do_get_iflag(cn, SF_EN_MOREBLIN)) power = more(power, 20, 1);
			if (IS_ANY_MERC(cn)) value = min(127, (power/6 + 2));
			else                 value = min(127, (power/8 + 1));
			break;
		case 98: // Rally Effect
			power = skill_multiplier(M_SK(cn, SK_WARCRY), cn);
			power = more(power, M_AT(cn, AT_STR)*(T_ARTM_SK(cn, 9)*2+TC_SK(cn, 21)), 20);  // (ArTm) Overlord
			value = min(127, (power*2/3) / 5 + 3);
			break;
		//
		default: break;
	}
	
	switch (n) // Global damage multiplier
	{
		case 24: case 27: case 32: case 37: case 40:
			value = value * ((do_get_iflag(cn, SF_STRENGTH)?120:100)/100)*(ch[cn].dmg_bonus/10000);
			break;
		case 25: case 35: case 41: case 90:
			value = value * ((do_get_iflag(cn, SF_STRENGTH)?120:100)/100)*(ch[cn].dmg_bonus/10000) / 20;
			break;
		default: break;
	}
	
	if (value > 65535) value = 65535;
	if (value <     0) value =     0;
	
	return value;
}

int ch_get_meta_alternative_value(int cn, int n)
{
	if (n ==  2 && do_get_iflag(cn, SF_STAR_R))   return  89; // Spell Bonus -> Skill Bonus
	if (n == 35 && do_get_iflag(cn, SF_TOWER_R))  return  90; // Poison -> Venom
	if (n == 36 && do_get_iflag(cn, SF_TOWER_R))  return  91; // Poison -> Venom
	if (n == 37 && do_get_iflag(cn, SF_JUDGE_R))  return  92; // Pulse Hit Damage -> Pulse Hit Heal
	if (n == 68 && do_get_iflag(cn, SF_EMPRESS))  return  93; // M.Shield -> M.Shell
	if (n == 69 && do_get_iflag(cn, SF_EMPRESS))  return  94; // M.Shield -> M.Shell
	if (n == 73 && do_get_iflag(cn, SF_STAR))     return  95; // Heal -> Regen
	if (n == 74 && do_get_iflag(cn, SF_CHARIOT))  return  96; // Blind -> Douse
	if (n == 75 && do_get_iflag(cn, SF_CHARIOT))  return  97; // Blind -> Douse
	if (n == 76 && do_get_iflag(cn, SF_EMPERO_R)) return  98; // Warcry -> Rally
	if (n == 77 && do_get_iflag(cn, SF_EMPERO_R)) return  99; // Warcry -> Rally
	if (n == 78 && do_get_iflag(cn, SF_DEATH))    return 100; // Weaken -> Crush
	if (n == 79 && do_get_iflag(cn, SF_DEATH))    return 101; // Weaken -> Crush
	if (n == 31 && do_get_iflag(cn, SF_HERMIT_R)) return 102; // Pact HP -> Mana
	if (n == 72 && do_get_iflag(cn, SF_HERMIT_R)) return 102; // Pact HP -> Mana
	if (n == 80 && do_get_iflag(cn, SF_TOWER))    return 103; // Curse -> Stymie
	if (n == 81 && do_get_iflag(cn, SF_TOWER))    return 104; // Curse -> Stymie
	if (n == 82 && do_get_iflag(cn, SF_EMPEROR))  return 105; // Slow -> Focus
	if (n == 83 && do_get_iflag(cn, SF_EMPEROR))  return 106; // Slow -> Focus
	
	return n;
}

void plr_update_meta_stat_values(int nr, int n)
{
	unsigned char buf[256];
	int cn = player[nr].usnr;
	int v, m = 0;
	
	buf[0] = SV_TERM_META;
	buf[2] = n;
	
	n = ch_get_meta_alternative_value(cn, n);
	v = get_meta_stat_value(cn, n);
	
	buf[1] = ST_META_VALUES;
	*(unsigned short*)(buf+ 3) = (unsigned short)(v);
	*(unsigned char*)(buf + 5) = (unsigned char)metaStats[n].flag;
	mcpy(buf+6, metaStats[n].affix, 8);
	*(unsigned char*)(buf +14) = (unsigned char)metaStats[n].font;
	
	if (metaStats[n].sknum == -2) m = 0;
	else if (metaStats[n].sknum == -1) m = 1;
	else if (metaStats[n].sknum == SK_IMMOLATE) { if (IS_WEARINGPHOENIX(cn)) m = 1; else m = 0; }
	else if (metaStats[n].sknum >=0 && metaStats[n].sknum < 50 && B_SK(cn, (int)(metaStats[n].sknum))) m = 1;
	
	*(unsigned char*)(buf +15) = (unsigned char)(m);
	
	xsend(nr, buf, 16);
}

void plr_update_all_meta_stat_values(int nr)
{
	for (int n=0; n<89; n++) plr_update_meta_stat_values(nr, n);
}

void plr_update_meta_terminology(int nr, int n)
{
	unsigned char buf[256];
	int m;
	
	buf[0] = SV_TERM_META;
	buf[2] = n;
	
	n = ch_get_meta_alternative_value(player[nr].usnr, n);
	
	for (m=0; m<3; m++)
	{
		buf[1] = ST_META_NAME+m;
		mcpy(buf+3, metaStats[n].name+m*10, 10);
		xsend(nr, buf, 13);
	}
	
	for (m=0; m<12; m++)
	{
		buf[1] = ST_META_DESC+m;
		mcpy(buf+3, metaStats[n].desc+m*10, 10);
		xsend(nr, buf, 13);
	}
}

void plr_update_all_meta_terminology(int nr)
{
	for (int n=0; n<89; n++) plr_update_meta_terminology(nr, n);
}

void plr_newlogin(int nr)
{
	int cn, temp, tmp, in;
	unsigned char buf[16];

	if (player[nr].version<MINVERSION)
	{
		plog(nr, "Client too old (%X). Logout demanded", player[nr].version);
		plr_logout(0, nr, LO_VERSION);
		return;
	}
	if (god_is_banned(player[nr].addr))
	{
		plog(nr, "Banned, sent away");
		plr_logout(0, nr, LO_KICKED);
		return;
	}
	if ((tmp = cap(0, nr)))
	{
		plog(nr, "Reached player cap, returned queue place %d, prio=%d", tmp, player[nr].prio);

		buf[0] = SV_CAP;
		*(unsigned int*)(buf + 1) = tmp;
		*(unsigned int*)(buf + 5) = player[nr].prio;
		csend(nr, buf, 16);

		player[nr].state = ST_NEWCAP;
		player[nr].lasttick  = globs->ticker;
		player[nr].lasttick2 = globs->ticker;
		return;
	}
	temp = player[nr].race;
	
	if (temp!=CT_TEMP_M && temp!=CT_TEMP_F 
	 && temp!=CT_MERC_M && temp!=CT_MERC_F 
	 && temp!=CT_HARA_M && temp!=CT_HARA_F)
	{
		temp = CT_MERC_M;
	}

	cn = god_create_char(temp, 1);
	ch[cn].player = nr;

	ch[cn].temple_x = ch[cn].tavern_x = HOME_START_X;
	ch[cn].temple_y = ch[cn].tavern_y = HOME_START_Y;

	ch[cn].points = 0;
	ch[cn].points_tot = 0;
	ch[cn].luck = 205;
	
	globs->players_created++;

	if (!god_drop_char_fuzzy_large(cn, ch[cn].temple_x, ch[cn].temple_y-1, ch[cn].temple_x, ch[cn].temple_y-1))
	{
		if (!god_drop_char_fuzzy_large(cn, ch[cn].temple_x + 3, ch[cn].temple_y, ch[cn].temple_x, ch[cn].temple_y))
		{
			if (!god_drop_char_fuzzy_large(cn, ch[cn].temple_x, ch[cn].temple_y + 3, ch[cn].temple_x, ch[cn].temple_y))
			{
				plog(nr, "plr_newlogin(): could not drop new character");
				plr_logout(cn, nr, LO_NOROOM);
				ch[cn].used = 0;
				return;
			}
		}
	}

	ch[cn].creation_date = time(NULL);
	ch[cn].login_date = time(NULL);
	ch[cn].flags |= CF_NEWUSER | CF_PLAYER;
	ch[cn].addr   = player[nr].addr;
	char_add_net(cn, ch[cn].addr);
	ch[cn].mode = 1;
	do_update_char(cn);

	player[nr].usnr  = cn;
	player[nr].pass1 = ch[cn].pass1;
	player[nr].pass2 = ch[cn].pass2;

	buf[0] = SV_NEWPLAYER;
	*(unsigned long*)(buf + 1)  = cn;
	*(unsigned long*)(buf + 5)  = ch[cn].pass1;
	*(unsigned long*)(buf + 9)  = ch[cn].pass2;
	*(unsigned char*)(buf + 13) = VERSION & 255;
	*(unsigned char*)(buf + 14) = (VERSION >> 8) & 255;
	*(unsigned char*)(buf + 15) = VERSION >> 16;
	csend(nr, buf, 16);

	player[nr].state = ST_NORMAL;
	player[nr].lasttick = globs->ticker;
	player[nr].ltick = 0;
	player[nr].ticker_started = 1;
	player[nr].spectating = 0;

	buf[0] = SV_TICK;
	*(unsigned char*)(buf + 1) = (unsigned char)ctick;
	xsend(nr, buf, 2);
	
	// grant additional minor potions to start
	if (temp==CT_TEMP_M || temp==CT_TEMP_F)
	{
		in = god_create_item(IT_POT_M_HP); god_give_char(in, cn);
		in = god_create_item(IT_POT_M_HP); god_give_char(in, cn);
		in = god_create_item(IT_POT_M_HP); god_give_char(in, cn);
		in = god_create_item(IT_POT_M_EN); god_give_char(in, cn);
		in = god_create_item(IT_POT_M_EN); god_give_char(in, cn);
		in = god_create_item(IT_POT_M_EN); god_give_char(in, cn);
	}
	else if (temp==CT_MERC_M || temp==CT_MERC_F)
	{
		in = god_create_item(IT_POT_M_HP); god_give_char(in, cn);
		in = god_create_item(IT_POT_M_HP); god_give_char(in, cn);
		in = god_create_item(IT_POT_M_EN); god_give_char(in, cn);
		in = god_create_item(IT_POT_M_EN); god_give_char(in, cn);
		in = god_create_item(IT_POT_M_MP); god_give_char(in, cn);
		in = god_create_item(IT_POT_M_MP); god_give_char(in, cn);
	}
	else if (temp==CT_HARA_M || temp==CT_HARA_F)
	{
		in = god_create_item(IT_POT_M_HP); god_give_char(in, cn);
		in = god_create_item(IT_POT_M_HP); god_give_char(in, cn);
		in = god_create_item(IT_POT_M_HP); god_give_char(in, cn);
		in = god_create_item(IT_POT_M_MP); god_give_char(in, cn);
		in = god_create_item(IT_POT_M_MP); god_give_char(in, cn);
		in = god_create_item(IT_POT_M_MP); god_give_char(in, cn);
	}
	
	plog(nr, "Created new character");

	do_char_motd(cn, newbi_msg1_font, newbi_msg1);
	do_char_motd(cn, 1, " \n");
	do_char_motd(cn, 1, " \n");
	do_char_motd(cn, newbi_msg2_font, newbi_msg2);
	do_char_motd(cn, 1, " \n");
	do_char_motd(cn, 1, " \n");
	do_char_motd(cn, newbi_msg3_font, newbi_msg3);
	do_char_motd(cn, 1, " \n");
	do_char_motd(cn, 1, " \n");
	do_char_motd(cn, newbi_msg4_font, newbi_msg4);
	do_char_motd(cn, 1, " \n");
	do_char_motd(cn, 1, " \n");
	do_char_motd(cn, newbi_msg5_font, newbi_msg5);
	
	buf[0] = SV_SHOWMOTD;
	*(unsigned char*)(buf + 1) = 99;
	xsend(nr, buf, 2);

	if (player[nr].passwd[0] && !(ch[cn].flags & CF_PASSWD))
	{
		god_change_pass(cn, cn, player[nr].passwd);
	}
	
	ch[cn].goto_x = HOME_START_X;
	ch[cn].goto_y = HOME_START_Y;
	
	plr_update_all_skill_terminology(nr);
	
	// do_staff_log(2,"New player %s entered the game!\n",ch[cn].name);
	do_announce(cn, 0, "A new player has entered the game.\n", ch[cn].name);
}


void plr_login(int nr)
{
	int cn, tmp, in, n;
	unsigned char buf[16];

	if (player[nr].version<MINVERSION)
	{
		plog(nr, "Client too old (%X). Logout demanded", player[nr].version);
		plr_logout(0, nr, LO_VERSION);
		return;
	}

	cn = player[nr].usnr;

	if (cn<=0 || cn>=MAXCHARS)
	{
		plog(nr, "Login as %d denied (illegal cn)", cn);
		plr_logout(0, nr, LO_PARAMS);
		return;
	}

	if (ch[cn].pass1!=player[nr].pass1 || ch[cn].pass2!=player[nr].pass2)
	{
		plog(nr, "Login as %s denied (pass1/pass2)", ch[cn].name);
		plr_logout(0, nr, LO_PASSWORD);
		return;
	}
	if ((ch[cn].flags & CF_PASSWD) && strcmp(ch[cn].passwd, player[nr].passwd))
	{
		plog(nr, "Login as %s denied (password)", ch[cn].name);
		plr_logout(0, nr, LO_PASSWORD);
		return;
	}

	if (ch[cn].used==USE_EMPTY)
	{
		plog(nr, "Login as %s denied (deleted)", ch[cn].name);
		plr_logout(0, nr, LO_PASSWORD);
		return;
	}

	if (ch[cn].used!=USE_NONACTIVE && !(ch[cn].flags & CF_CCP))
	{
		plog(nr, "Login as %s who is already active", ch[cn].name);
		plr_logout(cn, ch[cn].player, LO_IDLE);
	}
	
	/*
	if (ch[cn].flags & CF_KICKED)
	{
		plog(nr, "Login as %s denied (kicked)", ch[cn].name);
		plr_logout(0, nr, LO_KICKED);
		return;
	}
	*/

	if (!(ch[cn].flags & CF_GOD) && god_is_banned(player[nr].addr))
	{
		chlog(cn, "Banned, sent away");
		plr_logout(0, nr, LO_KICKED);
		return;
	}

	if ((tmp = cap(cn, nr)))
	{
		chlog(cn, "Reached player cap, returned queue place %d, prio=%d", tmp, player[nr].prio);

		buf[0] = SV_CAP;
		*(unsigned int*)(buf + 1) = tmp;
		*(unsigned int*)(buf + 5) = player[nr].prio;
		csend(nr, buf, 16);

		player[nr].state = ST_CAP;
		player[nr].lasttick  = globs->ticker;
		player[nr].lasttick2 = globs->ticker;
		return;
	}

	ch[cn].player = nr;

	if (!(ch[cn].flags & CF_CCP) && (ch[cn].flags & CF_GOD))
	{
		ch[cn].flags |= CF_INVISIBLE;
	}

	player[nr].state = ST_NORMAL;
	player[nr].lasttick = globs->ticker;
	player[nr].ltick = 0;
	player[nr].ticker_started = 1;
	player[nr].spectating = 0;

	buf[0] = SV_LOGIN_OK;
	*(unsigned long*)(buf + 1) = VERSION;
	csend(nr, buf, 16);

	buf[0] = SV_TICK;
	*(unsigned char*)(buf + 1) = (unsigned char)ctick;
	xsend(nr, buf, 2);

	ch[cn].used = USE_ACTIVE;
	ch[cn].login_date = time(NULL);
	ch[cn].addr = player[nr].addr;
	char_add_net(cn, ch[cn].addr);
	ch[cn].current_online_time = 0;

	player[nr].cpl.mode = -1; // "impossible" client default will force change

	if (!(ch[cn].flags & CF_CCP))
	{
		if (!god_drop_char_fuzzy_large(cn, ch[cn].tavern_x, ch[cn].tavern_y, ch[cn].tavern_x, ch[cn].tavern_y))
		{
			if (!god_drop_char_fuzzy_large(cn, ch[cn].tavern_x + 3, ch[cn].tavern_y, ch[cn].tavern_x, ch[cn].tavern_y))
			{
				if (!god_drop_char_fuzzy_large(cn, ch[cn].tavern_x, ch[cn].tavern_y + 3, ch[cn].tavern_x, ch[cn].tavern_y))
				{
					if (!god_drop_char_fuzzy_large(cn, ch[cn].temple_x, ch[cn].temple_y, ch[cn].temple_x, ch[cn].temple_y))
					{
						if (!god_drop_char_fuzzy_large(cn, ch[cn].temple_x + 3, ch[cn].temple_y, ch[cn].temple_x, ch[cn].temple_y))
						{
							if (!god_drop_char_fuzzy_large(cn, ch[cn].temple_x, ch[cn].temple_y + 3, ch[cn].temple_x, ch[cn].temple_y))
							{
								plog(nr, "plr_login(): could not drop new character");
								plr_logout(cn, nr, LO_NOROOM);
								return;
							}
						}
					}
				}
			}
		}
	}

	for (n = 0; n<MAXBUFFS; n++)
	{
		if ((in = ch[cn].spell[n]))
		{
			if (bu[in].temp==SK_RECALL)
			{
				bu[in].used = 0;
				ch[cn].spell[n] = 0;
				chlog(cn, "CHEATER: removed active teleport");
			}
		}
	}

	do_update_char(cn);
	ch[cn].tavern_x = ch[cn].temple_x;
	ch[cn].tavern_y = ch[cn].temple_y;

	plog(nr, "Login successful");
	
	plr_update_tree_terminology(nr, SV_TERM_STREE);
	plr_update_tree_terminology(nr, SV_TERM_CTREE);
	
	plr_update_all_skill_terminology(nr);
	
	if (ch[cn].data[79] != VERSION)
	{
		do_char_motd(cn, intro_msg1_font, intro_msg1);
		do_char_motd(cn, 1, " \n");
		do_char_motd(cn, 1, " \n");
		do_char_motd(cn, intro_msg2_font, intro_msg2);
		do_char_motd(cn, 1, " \n");
		do_char_motd(cn, 1, " \n");
		do_char_motd(cn, intro_msg3_font, intro_msg3);
		do_char_motd(cn, 1, " \n");
		do_char_motd(cn, 1, " \n");
		do_char_motd(cn, intro_msg4_font, intro_msg4);
		do_char_motd(cn, 1, " \n");
		do_char_motd(cn, 1, " \n");
		do_char_motd(cn, intro_msg5_font, intro_msg5);
		
		buf[0] = SV_SHOWMOTD;
		*(unsigned char*)(buf + 1) = 0;
		xsend(nr, buf, 2);
		
		ch[cn].data[79] = VERSION;
	}
	
	// Kill the tutorial if we're a returning player with more than 10k exp
	if (ch[cn].points_tot>10000 && ch[cn].data[76]<(1<<16))
	{
		ch[cn].data[76] |= (1<<16);
	}

	if (player[nr].passwd[0] && !(ch[cn].flags & CF_PASSWD))
	{
		god_change_pass(cn, cn, player[nr].passwd);
	}

	if (!(ch[cn].flags & CF_CCP) && (ch[cn].flags & CF_GOD))
	{
		do_char_log(cn, 0, "Remember, you are invisible!\n");
	}

	do_announce(cn, 0, "%s entered the game.\n", ch[cn].name);
}

void plr_logout(int cn, int nr, int reason)
{
	int n, in, co;
	unsigned char buf[16];
	
	if (nr<0 || nr>=MAXPLAYER) nr = 0;
	if (cn>0 && cn<MAXCHARS && (ch[cn].player==nr || nr==0) && (ch[cn].flags & CF_USURP))
	{
		ch[cn].flags &= ~(CF_CCP | CF_USURP | CF_STAFF | CF_IMMORTAL | CF_GOD | CF_CREATOR);
		co = ch[cn].data[97];
		plr_logout(co, 0, LO_SHUTDOWN);
	}
	
	if (cn>0 && cn<MAXCHARS && (ch[cn].player==nr || nr==0) && (ch[cn].flags & (CF_PLAYER)) && !(ch[cn].flags & CF_CCP))
	{
		if (reason==LO_EXIT)
		{
			chlog(cn, "Punished for leaving the game with F12");
			
			do_char_log(cn, 0, " \n");
			do_char_log(cn, 0, "You are being punished for leaving the game without entering a tavern.\n");
			do_char_log(cn, 0, "If you are out of combat, this punishment will be minimal. Otherwise...\n");
			do_char_log(cn, 0, " \n");
			do_char_log(cn, 0, "You have been hit by a demon. You lost %d HP.\n", ch[cn].hp[5] * 5 / 10);
			
			ch[cn].a_hp -= ch[cn].hp[5] * 500;
			
			if (ch[cn].a_hp<500)
			{
				do_char_log(cn, 0, " \n");
				do_char_log(cn, 0, "Oh dear, the demon killed you.\n");
				do_char_log(cn, 0, "Your belongings dropped to the ground where you stood.\n");
				do_char_killed(0, cn, 0);
			}
			
			do_area_log(cn, 0, ch[cn].x, ch[cn].y, 2, "%s left the game without saying goodbye.\n", ch[cn].name);
		}
		else
		{
			for (n = 0; n<MAXBUFFS; n++) // clear poison to avoid dying offline
			{
				if ((in = ch[cn].spell[n])==0) continue;
				if (bu[in].temp==SK_VENOM || bu[in].temp==SK_POISON || bu[in].temp==SK_BLEED || bu[in].temp==SK_IMMOLATE2)
				{
					bu[in].used = USE_EMPTY; 
					ch[cn].spell[n] = 0;
				}
			}
		}
		
		if (map[ch[cn].x + ch[cn].y * MAPX].ch==cn)
		{
			map[ch[cn].x + ch[cn].y * MAPX].ch = 0;
			if (ch[cn].light)
				do_add_light(ch[cn].x, ch[cn].y, -ch[cn].light);
		}
		
		if (map[ch[cn].tox + ch[cn].toy * MAPX].to_ch==cn)
			map[ch[cn].tox + ch[cn].toy * MAPX].to_ch = 0;
		
		remove_enemy(cn);
		
		if (reason==LO_IDLE || reason==LO_SHUTDOWN || reason==0)   // give lag scroll to player
		{
			if (abs(ch[cn].x - ch[cn].temple_x) + abs(ch[cn].y - ch[cn].temple_y)>10 && !(map[ch[cn].x + ch[cn].y * MAPX].flags & MF_NOLAG))
			{
				for (n=0;n<MAXITEMS;n++)
				{
					if (!(in = ch[cn].item[n])) continue;
					if (it[in].temp==IT_LAGSCROLL)
					{
						it[in].used = USE_EMPTY;
						ch[cn].item[n] = 0;
					}
				}
				
				in = god_create_item(IT_LAGSCROLL);
				it[in].data[0] = ch[cn].x;
				it[in].data[1] = ch[cn].y;
				it[in].data[2] = globs->ticker;
				
				if (in) god_give_char(in, cn);
			}
		}
		
		for (n=1;n<MAXCHARS;n++)
		{
			if (ch[n].used==USE_EMPTY) continue;
			
			if (IS_SANEPLAYER(n) && ch[n].player && player[ch[n].player].spectating == cn)
				player[ch[n].player].spectating = 0;
			
			if (IS_PLAYER_COMP(n) && CN_OWNER(n) == cn && !(reason==LO_IDLE || reason==LO_SHUTDOWN || reason==0))
			{
				do_sayx(n, "Goodbye, %s.", ch[cn].name);
				do_give_exp(cn, ch[n].data[28], 1, -1, 0);
				fx_add_effect(7, 0, ch[n].x, ch[n].y, 0);
				die_companion(n);
			}
		}
		
		ch[cn].x = ch[cn].y = ch[cn].tox = ch[cn].toy = ch[cn].frx = ch[cn].fry = 0;
		
		ch[cn].player       = 0;
		ch[cn].status       = 0;
		ch[cn].status2      = 0;
		ch[cn].dir          = 1;
		ch[cn].escape_timer = 0;
		
		for (n = 0; n<4; n++) ch[cn].enemy[n] = 0;
		
		ch[cn].attack_cn    = 0;
		ch[cn].skill_nr     = 0;
		ch[cn].goto_x       = 0;
		ch[cn].use_nr       = 0;
		ch[cn].misc_action  = 0;
		ch[cn].stunned      = 0;
		ch[cn].taunted      = 0;
		ch[cn].retry        = 0;
		for (n = 0; n<13; n++)      // reset afk, group and follow
		{
			if (n==11) continue;    // leave fightback set
			ch[cn].data[n]  = 0;
		}
		ch[cn].data[96]     = 0;    // Reset Queued spell
		
		clear_map_buffs(cn, 1);
		
		ch[cn].used = USE_NONACTIVE;
		ch[cn].logout_date = time(NULL);
		
		ch[cn].flags |= CF_SAVEME;
		ch[cn].flags &= ~CF_ALW_SPECT;
		
		if (IS_BUILDING(cn)) god_build(cn, 0);
		
		do_announce(cn, 0, "%s left the game.\n", ch[cn].name);
	}
	
	if (nr && reason && reason!=LO_USURP)
	{
		buf[0] = SV_EXIT;
		*(unsigned int*)(buf + 1) = reason;
		
		if (player[nr].state==ST_NORMAL) xsend(nr, buf, 16);
		else csend(nr, buf, 16);
		
		player_exit(nr);
	}
}

void plr_state(int nr)
{
	if (globs->ticker - player[nr].lasttick>TICKS * 15 && player[nr].state==ST_EXIT)
	{
		close(player[nr].sock);
		plog(nr, "Connection closed (ST_EXIT)");
		player[nr].sock = 0;
		deflateEnd(&player[nr].zs);
	}

	if (!player[nr].spectating && !IS_BUILDING(player[nr].usnr) && !IS_IN_TEMPLE(ch[player[nr].usnr].x, ch[player[nr].usnr].y))
	{
		if (globs->ticker - player[nr].lasttick>TICKS * 60)
		{
			plog(nr, "Idle timeout");
			plr_logout(0, nr, LO_IDLE);
		}
	}

	switch(player[nr].state)
	{
	case    ST_NEWLOGIN:
		plr_newlogin(nr);
		break;
	case    ST_LOGIN:
		plr_login(nr);
		break;

	case    ST_NEWCAP:
		if (globs->ticker - player[nr].lasttick>TICKS * 10)
		{
			player[nr].state = ST_NEWLOGIN;
		}
		break;
	case    ST_CAP:
		if (globs->ticker - player[nr].lasttick>TICKS * 10)
		{
			player[nr].state = ST_LOGIN;
		}
		break;

	case    ST_NEW_CHALLENGE:
	case    ST_LOGIN_CHALLENGE:
	case    ST_CONNECT:
		break;
	case    ST_EXIT:
		break;

	default:
		plog(nr, "UNKNOWN ST: %d", player[nr].state);
		break;
	}
}

void inline plr_change_stat(int nr, unsigned char *a, unsigned char *b, unsigned char code, unsigned char n)
{
	unsigned char buf[16];

	//if (a[0]!=b[0] || a[1]!=b[1] || a[2]!=b[2] || a[3]!=b[3] || a[4]!=b[4] || a[5]!=b[5]) {
	if (*(unsigned long*)a!=*(unsigned long*)b ||
	    *(unsigned short*)(a + 4)!=*(unsigned short*)(b + 4))
	{
		buf[0] = code;
		buf[1] = n;

		buf[2] = b[0];
		buf[3] = b[1];
		buf[4] = b[2];
		buf[5] = b[3];
		buf[6] = b[4];
		buf[7] = b[5];

		xsend(nr, buf, 8);
		mcpy(a, b, 6);
	}
}

void inline plr_change_power(int nr, unsigned short *a, unsigned short *b, unsigned char code)
{
	unsigned char buf[16];
	int cn = player[nr].usnr;
	int v = 0;
	
	if (code == SV_SETCHAR_HP)    v = ch[cn].reserve[0];
	if (code == SV_SETCHAR_ENDUR) v = ch[cn].reserve[1];
	if (code == SV_SETCHAR_MANA)  v = ch[cn].reserve[2];
	
	if (mcmp(a, b, 12))
	{
		buf[0] = code;
		*(unsigned short*)(buf + 1)  = b[0];
		*(unsigned short*)(buf + 3)  = b[1];
		*(unsigned short*)(buf + 5)  = b[2];
		*(unsigned short*)(buf + 7)  = b[3];
		*(unsigned short*)(buf + 9)  = v;
		*(unsigned short*)(buf + 11) = b[5];
		
		xsend(nr, buf, 13);
		mcpy(a, b, 12);
	}
}

inline int ch_base_status(int n)
{
	if (n<4)
	{
		return( n);
	}

	if (n<16)
	{
		return( n);     // unassigned!!
	}
	if (n<24)
	{
		return( 16);    // walk up
	}
	if (n<32)
	{
		return( 24);    // walk down
	}
	if (n<40)
	{
		return( 32);    // walk left
	}
	if (n<48)
	{
		return( 40);    // walk right
	}
	if (n<60)
	{
		return( 48);    // walk left+up
	}
	if (n<72)
	{
		return( 60);    // walk left+down
	}
	if (n<84)
	{
		return( 72);    // walk right+up
	}
	if (n<96)
	{
		return( 84);    // walk right+down
	}
	if (n<100)
	{
		return( 96);
	}
	if (n<104)
	{
		return( 100);   // turn up to left
	}
	if (n<108)
	{
		return( 104);   // turn up to right
	}
	if (n<112)
	{
		return( 108);
	}
	if (n<116)
	{
		return( 112);
	}
	if (n<120)
	{
		return( 116);   // turn down to left
	}
	if (n<124)
	{
		return( 120);
	}
	if (n<128)
	{
		return( 124);   // turn down to right
	}
	if (n<132)
	{
		return( 128);
	}
	if (n<136)
	{
		return( 132);   // turn left to up
	}
	if (n<140)
	{
		return( 136);
	}
	if (n<144)
	{
		return( 140);   // turn left to down
	}
	if (n<148)
	{
		return( 144);
	}
	if (n<152)
	{
		return( 148);   // turn right to up
	}
	if (n<156)
	{
		return( 152);
	}
	if (n<160)
	{
		return( 160);   // turn right to down
	}
	if (n<164)
	{
		return( 160);
	}

	if (n<168)
	{
		return( 160);   // attack up
	}
	if (n<176)
	{
		return( 168);   // attack down
	}
	if (n<184)
	{
		return( 176);   // attack left
	}
	if (n<192)
	{
		return( 184);   // attack right

	}
	if (n<200)
	{
		return( 192);   // misc up
	}
	if (n<208)
	{
		return( 200);   // misc down
	}
	if (n<216)
	{
		return( 208);   // misc left
	}
	if (n<224)
	{
		return( 216);   // misc right

	}
	// remainder is unassigned

	return(n);
}

static inline int it_base_status(int n)
{
	if (n==0)
	{
		return 0;
	}
	if (n==1)
	{
		return 1;
	}

	if (n<6)
	{
		return( 2);
	}
	if (n<8)
	{
		return( 6);
	}
	if (n<16)
	{
		return( 8);
	}
	if (n<21)
	{
		return( 16);
	}

	return(n);
}

int cl_light_26(int n, int dosend, struct cmap *cmap, struct cmap *smap)
{
	char buf[16], p;
	int  m, l = 0;

	if (!dosend)
	{
		for (m = n; m<n + 27 && m<TILEX * TILEY; m++)
		{
			if (cmap[m].light!=smap[m].light)
			{
				l++;
			}
		}
		return(VISI_SIZE * l / 16);
	}
	else
	{

		buf[0] = SV_SETMAP3;
		*(unsigned int*)(buf+1)=n;
		*(unsigned char*)(buf+5)=smap[n].light;
		//*(unsigned short*)(buf + 1) = (unsigned short)(n | ((unsigned short)(smap[n].light) << 12));
		cmap[n].light = smap[n].light;

		for (m = n + 2, p = 6; m<min(n + 20 + 2, TILEX * TILEY); m += 2, p++) // 02162020 - changed 26 to 20
		{
			*(unsigned char*)(buf + p) = smap[m].light | (smap[m - 1].light << 4);
			cmap[m].light = smap[m].light;
			cmap[m - 1].light = smap[m - 1].light;
		}
		xsend(dosend, buf, 16);
		return 1;
	}
}
int cl_light_one(int n, int dosend, struct cmap *cmap, struct cmap *smap)
{
	char buf[16];

	if (!dosend)
	{
		return(VISI_SIZE * 1 / 3);
	}
	else
	{

		buf[0] = SV_SETMAP4;
		*(unsigned int*)(buf+1)=n;
		*(unsigned char*)(buf+5)=smap[n].light;
		//*(unsigned short*)(buf + 1) = (unsigned short)(n | ((unsigned short)(smap[n].light) << 12));
		cmap[n].light = smap[n].light;
		xsend(dosend, buf, 6); // 02162020 - changed 3 to 6
		return 1;
	}
}
int cl_light_three(int n, int dosend, struct cmap *cmap, struct cmap *smap)
{
	char buf[16], p;
	int  m, l = 0;

	if (!dosend)
	{
		for (m = n; m<n + 3 && m<TILEX * TILEY; m++)
		{
			if (cmap[m].light!=smap[m].light)
			{
				l++;
			}
		}
		return(VISI_SIZE * l / 4);
	}
	else
	{
		buf[0] = SV_SETMAP5;
		*(unsigned int*)(buf+1)=n;
		*(unsigned char*)(buf+5)=smap[n].light;
		//*(unsigned short*)(buf + 1) = (unsigned short)(n | ((unsigned short)(smap[n].light) << 12));
		cmap[n].light = smap[n].light;

		for (m = n + 2, p = 6; m<min(n + 2 + 2, TILEX * TILEY); m += 2, p++)
		{
			*(unsigned char*)(buf + p) = smap[m].light | (smap[m - 1].light << 4);
			cmap[m].light = smap[m].light;
			cmap[m - 1].light = smap[m - 1].light;
		}
		xsend(dosend, buf, 7); // 02162020 - changed 4 to 7
		return 1;
	}
	return 0;
}
int cl_light_seven(int n, int dosend, struct cmap *cmap, struct cmap *smap)
{
	char buf[16], p;
	int  m, l = 0;

	if (!dosend)
	{
		for (m = n; m<n + 7 && m<TILEX * TILEY; m++)
		{
			if (cmap[m].light!=smap[m].light)
			{
				l++;
			}
		}
		return(VISI_SIZE * l / 6);
	}
	else
	{

		buf[0] = SV_SETMAP6;
		*(unsigned int*)(buf+1)=n;
		*(unsigned char*)(buf+5)=smap[n].light;
		//*(unsigned short*)(buf + 1) = (unsigned short)(n | ((unsigned short)(smap[n].light) << 12));
		cmap[n].light = smap[n].light;

		for (m = n + 2, p = 6; m<min(n + 6 + 2, TILEX * TILEY); m += 2, p++)
		{
			*(unsigned char*)(buf + p) = smap[m].light | (smap[m - 1].light << 4);
			cmap[m].light = smap[m].light;
			cmap[m - 1].light = smap[m - 1].light;
		}
		xsend(dosend, buf, 9); // 02162020 - changed 6 to 9
		return 1;
	}
}

// check for any changed values in player data and send them to the client
void plr_change(int nr)
{
	int cn, n, in, p, lastn = -1;
	struct cplayer *cpl;
	struct cmap *cmap;
	struct cmap *smap;
	unsigned char buf[256];
	int spr, ss, en, cr, stack;
	char areaname[40];

	cn   = player[nr].usnr;
	cpl  = &player[nr].cpl;
	cmap = player[nr].cmap;
	smap = player[nr].smap;

	if ((ch[cn].flags & CF_UPDATE) || (cn & 15)==(globs->ticker & 15))
	{
		// cplayer - stats
		if (strcmp(cpl->name, ch[cn].name))
		{
			buf[0] = SV_SETCHAR_NAME1;
			mcpy(buf + 1, ch[cn].name, 15);
			xsend(nr, buf, 16);
			buf[0] = SV_SETCHAR_NAME2;
			mcpy(buf + 1, ch[cn].name + 15, 15);
			xsend(nr, buf, 16);
			buf[0] = SV_SETCHAR_NAME3;
			mcpy(buf + 1, ch[cn].name + 30, 10);
			*(unsigned long*)(buf + 11) = (unsigned long)(ch[cn].temp);
			xsend(nr, buf, 16);
			mcpy(cpl->name, ch[cn].name, 40);
		}
		
		if (cpl->mode!=ch[cn].mode)
		{
			buf[0] = SV_SETCHAR_MODE;
			buf[1] = ch[cn].mode;
			xsend(nr, buf, 2);

			cpl->mode = ch[cn].mode;
		}
		
		for (n = 0; n<5; n++)
		{
			plr_change_stat(nr, cpl->attrib[n], ch[cn].attrib[n], SV_SETCHAR_ATTRIB, n);
		}

		plr_change_power(nr, cpl->hp, ch[cn].hp, SV_SETCHAR_HP);
		plr_change_power(nr, cpl->end, ch[cn].end, SV_SETCHAR_ENDUR);
		plr_change_power(nr, cpl->mana, ch[cn].mana, SV_SETCHAR_MANA);
		
		for (n = 0; n<MAXSKILL; n++)
		{
			plr_change_stat(nr, cpl->skill[n], ch[cn].skill[n], SV_SETCHAR_SKILL, n);
		}
		
		for (n = 0; n<MAXITEMS; n++)
		{
			in = ch[cn].item[n];
			if (cpl->item[n]!=in || (!IS_BUILDING(cn) && cpl->item_s[n]!=it[in].stack) || (!IS_BUILDING(cn) && it[in].flags & IF_UPDATE))
			{
				buf[0] = SV_SETCHAR_ITEM;
				*(unsigned long*)(buf + 1) = n;
				if (in)
				{
					if (IS_BUILDING(cn))
					{
						if (in & 0x40000000)
						{
							switch(in & 0xfffffff)
							{
							case MF_MOVEBLOCK:	*(short int*)(buf + 5) = 47; break;
							case MF_SIGHTBLOCK:	*(short int*)(buf + 5) = 83; break;
							case MF_INDOORS:	*(short int*)(buf + 5) = 48; break;
							case MF_UWATER:		*(short int*)(buf + 5) = 50; break;
							case MF_NOFIGHT:	*(short int*)(buf + 5) = 36; break;
							case MF_NOMONST:	*(short int*)(buf + 5) = 51; break;
							case MF_BANK:		*(short int*)(buf + 5) = 52; break;
							case MF_TAVERN:		*(short int*)(buf + 5) = 53; break;
							case MF_NOMAGIC:	*(short int*)(buf + 5) = 54; break;
							case MF_DEATHTRAP:	*(short int*)(buf + 5) = 74; break;
							case MF_ARENA:		*(short int*)(buf + 5) = 78; break;
							case MF_NOEXPIRE:	*(short int*)(buf + 5) = 81; break;
							case MF_NOLAG:		*(short int*)(buf + 5) = 49; break;
							case MF_NOPLAYER:	*(short int*)(buf + 5) = 79; break;
							default:			*(short int*)(buf + 5) =  0; break;
							}
							*(short int*)(buf + 7) = 0;
						}
						else if (in & 0x20000000)
						{
							*(short int*)(buf + 5) = in & 0xfffffff;
							*(short int*)(buf + 7) = 0;
						}
						else
						{
							*(short int*)(buf + 5) = it_temp[in].sprite[I_I];
							*(short int*)(buf + 7) = 0;
						}
						*(unsigned char*)(buf + 9) = 0;
						*(unsigned char*)(buf + 10) = 0;
					}
					else
					{
						if (it[in].active)
						{
							*(short int*)(buf + 5) = it[in].sprite[I_A];
						}
						else
						{
							*(short int*)(buf + 5) = it[in].sprite[I_I];
						}
						*(short int*)(buf + 7) = 0;
						if (IS_SOULCAT(in))   *(short int*)(buf + 7) = it[in].data[4];
						if (IS_CORRUPTOR(in)) *(short int*)(buf + 7) = it[in].data[0];
						*(unsigned char*)(buf + 9)  = (unsigned char)(it[in].stack);
						*(unsigned char*)(buf + 10) = (unsigned char)((ch[cn].item_lock[n]?0:0)+((it[in].flags&IF_SOULSTONE)?2:0)+((it[in].flags&IF_ENCHANTED)?4:0)+((it[in].flags&IF_CORRUPTED)?8:0));

						it[in].flags &= ~IF_UPDATE;
					}
				}
				else
				{
					*(short int*)(buf + 5) = 0;
					*(short int*)(buf + 7) = 0;
					*(unsigned char*)(buf + 9) = 0;
					*(unsigned char*)(buf + 10) = 0;
				}
				xsend(nr, buf, 11);

				cpl->item[n] = in;
				if (!IS_BUILDING(cn))
				{
					cpl->item_s[n]=it[in].stack;
					cpl->item_l[n]=(ch[cn].item_lock[n]?0:0)+((it[in].flags&IF_SOULSTONE)?2:0)+((it[in].flags&IF_ENCHANTED)?4:0)+((it[in].flags&IF_CORRUPTED)?8:0);
				}
			}
		}

		for (n = 0; n<13; n++)
		{
			if (cpl->worn[n]!=(in = ch[cn].worn[n]) || (it[in].flags & IF_UPDATE))
			{
				buf[0] = SV_SETCHAR_WORN;
				*(unsigned long*)(buf + 1) = n;
				if (in)
				{
					if (it[in].active) *(short int*)(buf + 5) = it[in].sprite[I_A];
					else               *(short int*)(buf + 5) = it[in].sprite[I_I];
					
					*(short int*)(buf + 7) = it[in].placement;
					
					if (IS_TWOHAND(in) && do_get_iflag(cn, SF_WORLD_R))    // [Taro] World.R
						*(short int*)(buf + 7) &= ~PL_TWOHAND;
					
					if ((it[in].flags & IF_OF_SHIELD) && IS_ARCHTEMPLAR(cn))
						*(short int*)(buf + 7) |= PL_WEAPON;
					
					if (it[in].flags & IF_SOULSTONE)
						*(short int*)(buf + 7) |= PL_SOULSTONED;
					
					if (it[in].flags & IF_ENCHANTED)
						*(short int*)(buf + 7) |= PL_ENCHANTED;
					
					if (it[in].flags & IF_CORRUPTED)
						*(short int*)(buf + 7) |= PL_CORRUPTED;
					
					*(unsigned char*)(buf + 9) = it[in].stack;
				}
				else
				{
					*(short int*)(buf + 5) = 0;
					*(short int*)(buf + 7) = 0;
					*(unsigned char*)(buf + 9) = 0;
				}
				xsend(nr, buf, 10);

				cpl->worn[n]  = in;
				it[in].flags &= ~IF_UPDATE;
			}
		}

		for (n = 0; n<MAXBUFFS; n++)
		{
			if (cpl->spell[n]!=(in = ch[cn].spell[n]) ||
			    (cpl->active[n]!=bu[in].active * 16 / max(1, bu[in].duration)) || (bu[in].flags & BF_UPDATE))
			{
				buf[0] = SV_SETCHAR_SPELL;
				*(unsigned long*)(buf + 1) = n;
				if (in)
				{
					*(short int*)(buf + 5) = bu[in].sprite;
					*(short int*)(buf + 7) = bu[in].active * 16 / max(1, bu[in].duration);
					cpl->spell[n]  = in;
					cpl->active[n] = (bu[in].active * 16 / max(1, bu[in].duration));
					bu[in].flags &= ~BF_UPDATE;
				}
				else
				{
					*(short int*)(buf + 5) = 0;
					*(short int*)(buf + 7) = 0;
					cpl->spell[n]  = 0;
					cpl->active[n] = 0;
				}
				xsend(nr, buf, 9);
			}
		}
		
		in = ch[cn].citem;
		if (cpl->citem!=in || 
			(!IS_BUILDING(cn) && !(in & 0x80000000) && cpl->citem_s != it[in].stack) || 
			(!IS_BUILDING(cn) && !(in & 0x80000000) && it[in].flags & IF_UPDATE))
		{
			buf[0] = SV_SETCHAR_OBJ;
			if (in & 0x80000000)
			{
				if ((in & 0x7fffffff)>999999)     *(short int*)(buf + 1) = 121;
				else if ((in & 0x7fffffff)>99999) *(short int*)(buf + 1) = 120;
				else if ((in & 0x7fffffff)>9999)  *(short int*)(buf + 1) =  41;
				else if ((in & 0x7fffffff)>999)   *(short int*)(buf + 1) =  40;
				else if ((in & 0x7fffffff)>99)    *(short int*)(buf + 1) =  39;
				else if ((in & 0x7fffffff)>9)     *(short int*)(buf + 1) =  38;
				else                              *(short int*)(buf + 1) =  37;
				*(short int*)(buf + 3) = 0;
				*(unsigned char*)(buf + 5) = 0;
			}
			else if (in)
			{
				if (IS_BUILDING(cn))
				{
					*(short int*)(buf + 1)     = 46;
					*(short int*)(buf + 3)     =  0;
					*(unsigned char*)(buf + 5) =  0;
				}
				else
				{
					if (it[in].active) *(short int*)(buf + 1) = it[in].sprite[I_A];
					else               *(short int*)(buf + 1) = it[in].sprite[I_I];
					
					*(short int*)(buf + 3) = it[in].placement;
					
					if (IS_TWOHAND(in) && do_get_iflag(cn, SF_WORLD_R))    // [Taro] World.R
						*(short int*)(buf + 3) &= ~PL_TWOHAND;
					
					if ((it[in].flags & IF_OF_SHIELD) && IS_ARCHTEMPLAR(cn))
						*(short int*)(buf + 3) |= PL_WEAPON;
					
					if (it[in].flags & IF_SOULSTONE)
						*(short int*)(buf + 3) |= PL_SOULSTONED;
					
					if (it[in].flags & IF_ENCHANTED)
						*(short int*)(buf + 3) |= PL_ENCHANTED;
					
					if (it[in].flags & IF_CORRUPTED)
						*(short int*)(buf + 3) |= PL_CORRUPTED;
					
					*(unsigned char*)(buf + 5) = it[in].stack;
					
					it[in].flags &= ~IF_UPDATE;
				}
			}
			else
			{
				*(short int*)(buf + 1)     = 0;
				*(short int*)(buf + 3)     = 0;
				*(unsigned char*)(buf + 5) = 0;
			}
			xsend(nr, buf, 6);
			
			cpl->citem = in;
			
			if (!IS_BUILDING(cn) && !(in & 0x80000000))
				cpl->citem_s = it[in].stack;
		}
		
		if (strcmp(cpl->location, get_area_truncated(cn)))
		{
			strcpy(areaname, get_area_truncated(cn));
			
			buf[0] = SV_SETCHAR_LOCA1;
			mcpy(buf + 1, areaname, 10);
			xsend(nr, buf, 11);
			
			buf[0] = SV_SETCHAR_LOCA2;
			mcpy(buf + 1, areaname + 10, 10);
			xsend(nr, buf, 11);
			
			mcpy(cpl->location, areaname, 20);
		}
	}

	if (cpl->a_hp!=(ch[cn].a_hp + 500) / 1000)
	{
		buf[0] = SV_SETCHAR_AHP;
		*(unsigned short*)(buf + 1) = (unsigned short)((ch[cn].a_hp + 500) / 1000);
		xsend(nr, buf, 3);

		cpl->a_hp = (ch[cn].a_hp + 500) / 1000;
	}

	if (cpl->a_end!=(ch[cn].a_end + 500) / 1000)
	{
		buf[0] = SV_SETCHAR_AEND;
		*(unsigned short*)(buf + 1) = (unsigned short)((ch[cn].a_end + 500) / 1000);
		xsend(nr, buf, 3);

		cpl->a_end = (ch[cn].a_end + 500) / 1000;
	}

	if (cpl->a_mana!=(ch[cn].a_mana + 500) / 1000)
	{
		buf[0] = SV_SETCHAR_AMANA;
		*(unsigned short*)(buf + 1) = (unsigned short)((ch[cn].a_mana + 500) / 1000);
		xsend(nr, buf, 3);

		cpl->a_mana = (ch[cn].a_mana + 500) / 1000;
	}

	if (cpl->dir!=ch[cn].dir)
	{
		buf[0] = SV_SETCHAR_DIR;
		*(unsigned char*)(buf + 1) = ch[cn].dir;
		xsend(nr, buf, 2);

		cpl->dir = ch[cn].dir;
	}

	if (cpl->points!=ch[cn].points ||
	    cpl->points_tot!=ch[cn].points_tot ||
	    cpl->kindred!=ch[cn].kindred)
	{
		buf[0] = SV_SETCHAR_PTS;
		*(unsigned long*)(buf + 1) = (unsigned long)(ch[cn].points);
		*(unsigned long*)(buf + 5) = (unsigned long)(ch[cn].points_tot);
		*(unsigned int*)(buf + 9)  = (unsigned int)(ch[cn].kindred);
		xsend(nr, buf, 13);

		cpl->points = ch[cn].points;
		cpl->points_tot = ch[cn].points_tot;
		cpl->kindred = ch[cn].kindred;
	}
	
	// waypoints, bspoints, ospoints
	if (cpl->waypoints != ch[cn].waypoints ||
		cpl->bs_points != ch[cn].bs_points ||
		cpl->os_points != ch[cn].os_points)
	{
		buf[0] = SV_SETCHAR_WPS;
		*(unsigned int*)(buf + 1) = (unsigned int)(ch[cn].waypoints);
		*(unsigned int*)(buf + 5) = (unsigned int)(ch[cn].bs_points);
		*(unsigned int*)(buf + 9) = (unsigned int)(ch[cn].os_points);
		xsend(nr, buf, 13);
		
		cpl->waypoints = ch[cn].waypoints;
		cpl->bs_points = ch[cn].bs_points;
		cpl->os_points = ch[cn].os_points;
	}
	
	// casino tokens
	if (cpl->tokens != ch[cn].tokens || cpl->tree_points != ch[cn].tree_points || cpl->os_tree != ch[cn].os_tree)
	{
		buf[0] = SV_SETCHAR_TOK;
		*(unsigned int*)(buf + 1)   = (unsigned int)(ch[cn].tokens);
		*(unsigned short*)(buf + 5) = (unsigned short)(ch[cn].tree_points);
		*(unsigned short*)(buf + 7) = (unsigned short)(ch[cn].os_tree);
		xsend(nr, buf, 9);
		
		cpl->tokens      = ch[cn].tokens;
		cpl->tree_points = ch[cn].tree_points;
		cpl->os_tree     = ch[cn].os_tree;
	}
	
	// tree nodes
	for (n = 0; n<12; n++)
	{
		if (cpl->tree_node[n] != ch[cn].tree_node[n])
		{
			buf[0] = SV_SETCHAR_TRE;
			*(unsigned char*)(buf + 1) = n;
			*(unsigned char*)(buf + 2) = (unsigned char)(ch[cn].tree_node[n]);
			xsend(nr, buf, 3);
			
			cpl->tree_node[n] = ch[cn].tree_node[n];
		}
	}
	
	// Blacksmith slots
	for (n = 0; n<4; n++)
	{
		if (cpl->sitem[n] == (in = ch[cn].blacksmith[n])) continue;
		if (in)
		{
			spr = it[in].sprite[I_I];
			if (it[in].flags & IF_SOULSTONE) ss = 1; else ss = 0;
			if (it[in].flags & IF_ENCHANTED) en = 2; else en = 0;
			if (it[in].flags & IF_CORRUPTED) cr = 4; else cr = 0;
			if (it[in].stack) stack = it[in].stack;  else stack = 0;
		}
		else
		{
			spr = ss = en = cr = stack = 0;
		}
		buf[0] = SV_LOOK8;
		*(unsigned char *)(buf + 1) = n;
		*(unsigned short*)(buf + 2) = (unsigned short)(ch[cn].smithnum);
		*(unsigned short*)(buf + 4) = spr;
		*(unsigned char *)(buf + 6) = stack;
		*(unsigned char *)(buf + 7) = ss + en + cr;
		xsend(nr, buf, 8);
		
		if (n==0 && IS_SANEITEM(ch[cn].blacksmith[0]))
		{
			buf[0] = SV_LOOK8;
			if (it[ch[cn].blacksmith[0]].flags & IF_ARMORS)
				*(unsigned char *)(buf + 1) = 5;
			else
				*(unsigned char *)(buf + 1) = 4;
			*(unsigned short*)(buf + 2) = (unsigned short)(ch[cn].smithnum);
			*(unsigned short*)(buf + 4) = 0;
			*(unsigned char *)(buf + 6) = 0;
			*(unsigned char *)(buf + 7) = 0;
			xsend(nr, buf, 8);
		}
		cpl->sitem[n] = ch[cn].blacksmith[n];
	}

	if (cpl->gold!=ch[cn].gold || cpl->armor!=ch[cn].armor || cpl->weapon!=ch[cn].weapon)
	{
		buf[0] = SV_SETCHAR_GOLD;
		*(unsigned long*)(buf + 1) = (unsigned long)(ch[cn].gold);
		*(unsigned long*)(buf + 5) = (unsigned long)(ch[cn].armor);
		*(unsigned long*)(buf + 9) = (unsigned long)(ch[cn].weapon);
		xsend(nr, buf, 13);

		cpl->gold = ch[cn].gold;
		cpl->armor  = ch[cn].armor;
		cpl->weapon = ch[cn].weapon;
	}

	if ((ch[cn].flags & CF_GOD) && (globs->ticker & 31)==0)
	{
		buf[0] = SV_LOAD;
		*(unsigned int*)(buf + 1) = globs->load;
		xsend(nr, buf, 5);
	}

	// cplayer - map

	if (cpl->x!=ch[cn].x || cpl->y!=ch[cn].y)
	{
		if (cpl->x==ch[cn].x - 1 && cpl->y==ch[cn].y)
		{
			buf[0] = SV_SCROLL_RIGHT;
			xsend(nr, buf, 1);

			memmove(cmap, cmap + 1, sizeof(struct cmap) * (TILEX * TILEY - 1));
		}
		else if (cpl->x==ch[cn].x + 1 && cpl->y==ch[cn].y)
		{
			buf[0] = SV_SCROLL_LEFT;
			xsend(nr, buf, 1);

			memmove(cmap + 1, cmap, sizeof(struct cmap) * (TILEX * TILEY - 1));
		}
		else if (cpl->x==ch[cn].x && cpl->y==ch[cn].y - 1)
		{
			buf[0] = SV_SCROLL_DOWN;
			xsend(nr, buf, 1);

			memmove(cmap, cmap + TILEX, sizeof(struct cmap) * (TILEX * TILEY - TILEX));
		}
		else if (cpl->x==ch[cn].x && cpl->y==ch[cn].y + 1)
		{
			buf[0] = SV_SCROLL_UP;
			xsend(nr, buf, 1);

			memmove(cmap + TILEX, cmap, sizeof(struct cmap) * (TILEX * TILEY - TILEX));
		}
		else if (cpl->x==ch[cn].x + 1 && cpl->y==ch[cn].y + 1)
		{
			buf[0] = SV_SCROLL_LEFTUP;
			xsend(nr, buf, 1);

			memmove(cmap + TILEX + 1, cmap, sizeof(struct cmap) * (TILEX * TILEY - TILEX - 1));
		}
		else if (cpl->x==ch[cn].x + 1 && cpl->y==ch[cn].y - 1)
		{
			buf[0] = SV_SCROLL_LEFTDOWN;
			xsend(nr, buf, 1);

			memmove(cmap, cmap + TILEX - 1, sizeof(struct cmap) * (TILEX * TILEY - TILEX + 1));
		}
		else if (cpl->x==ch[cn].x - 1 && cpl->y==ch[cn].y + 1)
		{
			buf[0] = SV_SCROLL_RIGHTUP;
			xsend(nr, buf, 1);

			memmove(cmap + TILEX - 1, cmap, sizeof(struct cmap) * (TILEX * TILEY - TILEX + 1));
		}
		else if (cpl->x==ch[cn].x - 1 && cpl->y==ch[cn].y - 1)
		{
			buf[0] = SV_SCROLL_RIGHTDOWN;
			xsend(nr, buf, 1);

			memmove(cmap, cmap + TILEX + 1, sizeof(struct cmap) * (TILEX * TILEY - TILEX - 1));
		}

		cpl->x = ch[cn].x;
		cpl->y = ch[cn].y;

		buf[0] = SV_SETORIGIN;
		*(short*)(buf + 1) = ch[cn].x - TMIDDLEX; //smap[0].x;
		*(short*)(buf + 3) = ch[cn].y - TMIDDLEY; //smap[0].y;
		xsend(nr, buf, 5);
	}

	// light as special case which sends multiple tiles at once
	for (n = 0; n<TILEX * TILEY; n++)
	{
		if (cmap[n].light!=smap[n].light)
		{
			int bp = 0, l, bl = 0, pe;
			static int (*lfunc[])(int, int, struct cmap*, struct cmap*) =
			{cl_light_one, cl_light_three, cl_light_seven, cl_light_26};

			for (l = 0; l<sizeof(lfunc) / sizeof(int*); l++)
			{
				pe = lfunc[l](n, 0, cmap, smap);
				if (pe>=bp)
				{
					bp = pe;
					bl = l;
				}
			}

			lfunc[bl](n, nr, cmap, smap);
		}
	}

	// everything else
	//for (n=0; n<TILEX*TILEY; n++) {
	//      if (!fdiff(&cmap[n],&smap[n],sizeof(struct cmap))) continue;

	n = 0;
	while (1)
	{
		unsigned char *tmp;

		tmp = fdiff(cmap + n, smap + n, sizeof(struct cmap) * (TILEX * TILEY - n));   // find address of first difference
		if (!tmp)
		{
			break;                                                  // no difference found? then we're done

		}
		// calculate index of difference found
		n = (tmp - (unsigned char*)(cmap + n)) / sizeof(struct cmap) + n;

		//for (f=0; (n=player[nr].changed_field[f])!=-1; f++) {
		if (n>lastn && n - lastn<127)
		{
			buf[0] = SV_SETMAP | (n - lastn);
			buf[1] = 0;
			p = 2;
		}
		else
		{
			buf[0] = SV_SETMAP;
			buf[1] = 0;
			*(unsigned short*)(buf + 2) = (short)n;
			p = 4;
		}

		if (cmap[n].ba_sprite!=smap[n].ba_sprite)
		{
			buf[1] |= 1;
			*(unsigned short*)(buf + p) = smap[n].ba_sprite;
			p += 2;

		}
		if (cmap[n].flags!=smap[n].flags)
		{
			buf[1] |= 2;
			*(unsigned int*)(buf + p) = smap[n].flags;
			p += 4;
		}
		if (cmap[n].flags2!=smap[n].flags2)
		{
			buf[1] |= 4;
			*(unsigned int*)(buf + p) = smap[n].flags2;
			p += 4;
		}
		if (cmap[n].it_sprite!=smap[n].it_sprite)
		{
			buf[1] |= 8;
			*(unsigned short*)(buf + p) = smap[n].it_sprite;
			p += 2;
		}
		if (cmap[n].it_status!=smap[n].it_status && it_base_status(cmap[n].it_status)!=it_base_status(smap[n].it_status))
		{
			buf[1] |= 16;
			*(unsigned char*)(buf + p) = smap[n].it_status;
			p += 1;
		}
		if (cmap[n].ch_sprite!=smap[n].ch_sprite ||
		    (cmap[n].ch_status!=smap[n].ch_status && ch_base_status(cmap[n].ch_status)!=ch_base_status(smap[n].ch_status)) ||
		    cmap[n].ch_status2!=smap[n].ch_status2)
		{
			buf[1] |= 32;
			*(unsigned short*)(buf + p) = smap[n].ch_sprite;
			p += 2;
			*(unsigned char*)(buf + p) = smap[n].ch_status;
			p += 1;
			*(unsigned char*)(buf + p) = smap[n].ch_status2;
			p += 1;
		}
		if (cmap[n].ch_speed!=smap[n].ch_speed ||
		    cmap[n].ch_nr!=smap[n].ch_nr ||
		    cmap[n].ch_id!=smap[n].ch_id)
		{
			buf[1] |= 64;
			*(unsigned short*)(buf + p) = smap[n].ch_nr;
			p += 2;
			*(unsigned short*)(buf + p) = smap[n].ch_id;
			p += 2;
			*(short int*)(buf + p) = smap[n].ch_speed;
			p += 2;
		}

		if (cmap[n].ch_proz!=smap[n].ch_proz || cmap[n].ch_castspd!=smap[n].ch_castspd || 
			cmap[n].ch_atkspd!=smap[n].ch_atkspd || cmap[n].ch_movespd!=smap[n].ch_movespd)
		{
			buf[1] |= 128;
			*(unsigned char*)(buf + p) = smap[n].ch_proz;
			p += 1;
			*(short int*)(buf + p) = smap[n].ch_castspd;
			p += 2;
			*(short int*)(buf + p) = smap[n].ch_atkspd;
			p += 2;
			*(short int*)(buf + p) = smap[n].ch_movespd;
			p += 2;
			*(unsigned char*)(buf + p) = smap[n].ch_fontcolor;
			p += 1;
		}

		if (buf[1])   // we found a change (no change found can happen since it_status & ch_status are not transmitted all the time)
		{
			xsend(nr, buf, p);
			lastn = n;
		}
		mcpy(&cmap[n], &smap[n], sizeof(struct cmap));

		n++;                            // done with this field
		if (n>=TILEX * TILEY)
		{
			break;                  // finished?
		}
	}

	if (ch[cn].attack_cn!=cpl->attack_cn ||
	    ch[cn].goto_x!=cpl->goto_x ||
	    ch[cn].goto_y!=cpl->goto_y ||
	    ch[cn].misc_action!=cpl->misc_action ||
	    ch[cn].misc_target1!=cpl->misc_target1 ||
	    ch[cn].misc_target2!=cpl->misc_target2)
	{
		buf[0] = SV_SETTARGET;
		cpl->attack_cn = *(unsigned short*)(buf + 1) = ch[cn].attack_cn;
		cpl->goto_x = *(unsigned short*)(buf + 3) = ch[cn].goto_x;
		cpl->goto_y = *(unsigned short*)(buf + 5) = ch[cn].goto_y;
		cpl->misc_action  = *(unsigned short*)(buf + 7) = ch[cn].misc_action;
		cpl->misc_target1 = *(unsigned short*)(buf + 9) = ch[cn].misc_target1;
		cpl->misc_target2 = *(unsigned short*)(buf + 11) = ch[cn].misc_target2;
		xsend(nr, buf, 13);
	}
}

void char_play_sound(int cn, int sound, int vol, int pan)
{
	unsigned char buf[16];
	int nr;

	nr = ch[cn].player;

	if (!nr)
	{
		return;
	}

	buf[0] = SV_PLAYSOUND;
	*(int*)(buf + 1) = sound;
	*(int*)(buf + 5) = vol;
	*(int*)(buf + 9) = pan;

	xsend(nr, buf, 13);
}

int do_char_calc_light(int cn, int light)
{
	int val, infv = 5;
	
	if (IS_BUILDING(cn))
	{
		light = 64;
	}
	
	if (IS_IN_DW(ch[cn].x, ch[cn].y))
	{
		val = light * min(M_SK(cn, SK_PERCEPT)/3*2, 10) / 10;
		if (val>255)
		{
			val = 255;
		}
		return val;
	}
	else if (IS_GLOB_MAYHEM)
	{
		if (light==0 && M_SK(cn, SK_PERCEPT)>225 && !has_buff(cn, 215) && !(IS_IN_XVIII(ch[cn].x, ch[cn].y) && has_item(cn, IT_COMMAND3)))
		{
			light = 1;
		}
		val = light * min(M_SK(cn, SK_PERCEPT)/3*2, 10) / 10;
	}
	else
	{
		if (light==0 && M_SK(cn, SK_PERCEPT)>150 && !has_buff(cn, 215) && !(IS_IN_XVIII(ch[cn].x, ch[cn].y) && has_item(cn, IT_COMMAND3)))
		{
			light = 1;
		}
		val = light * min(M_SK(cn, SK_PERCEPT), 10) / 10;
	}

	if (val>255)
	{
		val = 255;
	}
	
	if (ch[cn].flags & CF_INFRARED)
	{
		if (M_SK(cn, SK_PERCEPT)> 20) infv = 4;
		if (M_SK(cn, SK_PERCEPT)> 40) infv = 3;
		if (M_SK(cn, SK_PERCEPT)> 60) infv = 2;
		if (M_SK(cn, SK_PERCEPT)> 80) infv = 1;
		if (M_SK(cn, SK_PERCEPT)>150) infv = 0;
	}
	if ((ch[cn].flags & CF_INFRARED) && val<infv)
	{
		val = infv;
	}

	return (val);
}

inline int check_dlight(int x, int y)
{
	int m;

	m = x + y * MAPX;

	if (!(map[m].flags & MF_INDOORS))
	{
		if (IS_IN_DW(x, y))
		{
			if  (map[m].flags & MF_TOUCHED) return (200 * map[m].dlight) / 256;
			return 0;
		}
		return globs->dlight;
	}

	return (globs->dlight * map[m].dlight) / 256;
}

inline int check_dlightm(int m)
{
	if (!(map[m].flags & MF_INDOORS))
	{
		if (IS_IN_DW(M2X(m), M2Y(m)))
		{
			if  (map[m].flags & MF_TOUCHED) return (200 * map[m].dlight) / 256;
			return 0;
		}
		return globs->dlight;
	}

	return (globs->dlight * map[m].dlight) / 256;
}

static void inline empty_field(struct cmap *smap, int n)
{
	smap[n].ba_sprite = SPR_EMPTY;
	smap[n].flags  = 0;
	smap[n].flags2 = 0;
	smap[n].light  = 0;

	smap[n].ch_sprite  = 0;
	smap[n].ch_status  = 0;
	smap[n].ch_status2 = 0;
	smap[n].ch_speed = 0;
	smap[n].ch_nr = 0;
	smap[n].ch_id = 0;
	smap[n].ch_proz = 0;
	smap[n].ch_castspd = 0;
	smap[n].ch_atkspd = 0;
	smap[n].ch_movespd = 0;
	smap[n].ch_fontcolor = 0;

	smap[n].it_sprite = 0;
	smap[n].it_status = 0;
}

/* static int inline tmp_vis(int x,int y)
   {
        int in;

        if (map[x+y*MAPX].flags&MF_SIGHTBLOCK) return 0;
        if ((in=map[x+y*MAPX].it) && (it[in].flags&IF_SIGHTBLOCK)) return 0;
        if (map[x+y*MAPX].ch) return 0;

        return 1;
   }

   static int tmp_see(int frx,int fry,int tox,int toy)
   {
        long long fx,fy,tx,ty,x,y;
        long long dx,dy,panic=0;
        //long long visi=16384;

        x=fx=(((long long)(frx))<<16)+32768; y=fy=(((long long)(fry))<<16)+32768;
        tx=(((long long)(tox))<<16)+32768; ty=(((long long)(toy))<<16)+32768;

        dx=tox-frx;
        dy=toy-fry;

        if (!dx && !dy) return 65536;

        if (abs(dx)>abs(dy)) { dy*=65536; dy/=abs(dx); if (dx>0) dx=65536; else dx=-65536; }
        else { dx*=65536; dx/=abs(dy); if (dy>0) dy=65536; else dy=-65536; }

        while (1) {
                x+=dx; y+=dy;
                if ((x>>16)==(tx>>16) && (y>>16)==(ty>>16)) return 65536; //return visi;
                //if (!tmp_vis(x>>16,y>>16)) visi-=abs(32768-(x&0xffff))+abs(32768-(y&0xffff));
                //if (visi<0) return 0;

                if (!tmp_vis(x>>16,y>>16)) return 0;

                if (panic++>25) {
                        xlog("f=%d,%d t=%d,%d (%d,%d) d=%d,%d",(int)(fx>>16),(int)(fy>>16),(int)(tx>>16),(int)(ty>>16),(int)(x>>16),(int)(y>>16),(int)dx,(int)dy);
                        return 1;
                }
        }
   }


   static int tmp_see1(int frx,int fry,int tox,int toy)
   {
        long long fx,fy,tx,ty,x,y;
        long long dx,dy,panic=0;

        x=fx=(((long long)(frx))<<16)+60000; y=fy=(((long long)(fry))<<16)+60000;
        tx=(((long long)(tox))<<16)+60000; ty=(((long long)(toy))<<16)+60000;

        dx=tox-frx;
        dy=toy-fry;

        if (!dx && !dy) return 1;

        if (abs(dx)>abs(dy)) { dy*=65536; dy/=abs(dx); if (dx>0) dx=65536; else dx=-65536; }
        else { dx*=65536; dx/=abs(dy); if (dy>0) dy=65536; else dy=-65536; }

        while (1) {
                x+=dx; y+=dy;
                if ((x>>16)==(tx>>16) && (y>>16)==(ty>>16)) return 1;
                if (!tmp_vis(x>>16,y>>16)) return 0;

                if (panic++>25) {
                        xlog("f=%d,%d t=%d,%d (%d,%d) d=%d,%d",(int)(fx>>16),(int)(fy>>16),(int)(tx>>16),(int)(ty>>16),(int)(x>>16),(int)(y>>16),(int)dx,(int)dy);
                        return 1;
                }
        }
   }

   static int tmp_see2(int frx,int fry,int tox,int toy)
   {
        long long fx,fy,tx,ty,x,y;
        long long dx,dy,panic=0;

        x=fx=(((long long)(frx))<<16)+5000; y=fy=(((long long)(fry))<<16)+5000;
        tx=(((long long)(tox))<<16)+5000; ty=(((long long)(toy))<<16)+5000;

        dx=tox-frx;
        dy=toy-fry;

        if (!dx && !dy) return 1;

        if (abs(dx)>abs(dy)) { dy*=65536; dy/=abs(dx); if (dx>0) dx=65536; else dx=-65536; }
        else { dx*=65536; dx/=abs(dy); if (dy>0) dy=65536; else dy=-65536; }

        while (1) {
                x+=dx; y+=dy;
                if ((x>>16)==(tx>>16) && (y>>16)==(ty>>16)) return 1;
                if (!tmp_vis(x>>16,y>>16)) return 0;

                if (panic++>25) {
                        xlog("f=%d,%d t=%d,%d (%d,%d) d=%d,%d",(int)(fx>>16),(int)(fy>>16),(int)(tx>>16),(int)(ty>>16),(int)(x>>16),(int)(y>>16),(int)dx,(int)dy);
                        return 1;
                }
        }
   }*/

#define YSCUT 3         //3
#define YECUT 1         //1
#define XSCUT 2         //2
#define XECUT 2         //2

void plr_getmap_complete(int nr)
{
	int cn, co, light, tmp, visible, infra = 0, infv = 5;
	int x, y, n, m;
	int ys, ye, xs, xe;
	unsigned char do_all = 0;
	struct cmap * smap;

	if (player[nr].spectating) 
		cn = player[nr].spectating;
	else 
		cn = player[nr].usnr;
	smap = player[nr].smap;

	ys = ch[cn].y - (TILEY / 2) + YSCUT;
	ye = ch[cn].y + (TILEY / 2) - YECUT;
	xs = ch[cn].x - (TILEX / 2) + XSCUT;
	xe = ch[cn].x + (TILEX / 2) - XECUT;

	// trigger recomputation of visibility map
	can_see(cn, ch[cn].x, ch[cn].y, ch[cn].x + 1, ch[cn].y + 1, TILEX/2);

	// check if visibility changed
	if (player[nr].vx!=see[cn].x || player[nr].vy!=see[cn].y || 
		mcmp(player[nr].visi, see[cn].vis, VISI_SIZE * VISI_SIZE))
	{
		mcpy(player[nr].visi, see[cn].vis, VISI_SIZE * VISI_SIZE);
		player[nr].vx = see[cn].x;
		player[nr].vy = see[cn].y;
		do_all = 1;
	}

	if (IS_BUILDING(cn))
	{
		do_all = 1;
	}

	for (n = YSCUT * TILEX + XSCUT, m = xs + ys * MAPX, y = ys; y<ye; 
			y++, m += MAPX - TILEX + XSCUT + XECUT, n += XSCUT + XECUT)
	{
		for (x = xs; x<xe; x++, n++, m++)
		{
			if (do_all || map[m].it || map[m].ch || 
				mcmp(&player[nr].xmap[n], &map[m], sizeof(struct map)))		// any change on map data?
			{
				mcpy(&player[nr].xmap[n], &map[m], sizeof(struct map));		// remember changed values
			}
			else
			{
				continue;
			}

			//player[nr].changed_field[p++]=n;

			// field outside of map? then display empty one.
			if (x<0 || y<0 || x>=MAPX || y>=MAPY)
			{
				empty_field(smap, n);
				continue;
			}

			// calculate light
			tmp = check_dlightm(m);

			light = max(map[m].light, tmp);
			light = do_char_calc_light(cn, light);
			if (ch[cn].flags & CF_INFRARED)
			{
				if (M_SK(cn, SK_PERCEPT)> 20) infv = 4;
				if (M_SK(cn, SK_PERCEPT)> 40) infv = 3;
				if (M_SK(cn, SK_PERCEPT)> 60) infv = 2;
				if (M_SK(cn, SK_PERCEPT)> 80) infv = 1;
				if (M_SK(cn, SK_PERCEPT)>150) infv = 0;
			}
			if (light<=infv && (ch[cn].flags & CF_INFRARED) && !IS_IN_DW(ch[cn].x, ch[cn].y))
			{
				infra = 1;
			}
			else
			{
				infra = 0;
			}

			// everybody sees himself
			if (light==0 && map[m].ch==cn)
			{
				light = 1;
			}

			// no light, nothing visible
			if (light==0)
			{
				empty_field(smap, n);
				continue;
			}

			// --- Begin of Flags -----
			smap[n].flags = 0;

			if (map[m].flags & (MF_GFX_INJURED |
			                    MF_GFX_INJURED1 |
			                    MF_GFX_INJURED2 |
								MF_GFX_CRIT |
			                    MF_GFX_DEATH |
			                    MF_GFX_TOMB |
			                    MF_GFX_EMAGIC |
			                    MF_GFX_GMAGIC |
			                    MF_GFX_CMAGIC |
			                    MF_UWATER))
			{
				if (map[m].flags & MF_GFX_CRIT)
					smap[n].flags |= CRITTED;
				
				if (map[m].flags & MF_GFX_INJURED)
					smap[n].flags |= INJURED;
				if (map[m].flags & MF_GFX_INJURED1)
					smap[n].flags |= INJURED1;
				if (map[m].flags & MF_GFX_INJURED2)
					smap[n].flags |= INJURED2;

				if (map[m].flags & MF_GFX_DEATH)
					smap[n].flags |= (map[m].flags & MF_GFX_DEATH) >> 23;
				if (map[m].flags & MF_GFX_TOMB)
					smap[n].flags |= (map[m].flags & MF_GFX_TOMB) >> 23;
				if (map[m].flags & MF_GFX_EMAGIC)
					smap[n].flags |= (map[m].flags & MF_GFX_EMAGIC) >> 23;
				if (map[m].flags & MF_GFX_GMAGIC)
					smap[n].flags |= (map[m].flags & MF_GFX_GMAGIC) >> 23;
				if (map[m].flags & MF_GFX_CMAGIC)
					smap[n].flags |= (map[m].flags & MF_GFX_CMAGIC) >> 23;

				if (map[m].flags & MF_UWATER)
					smap[n].flags |= UWATER;
			}

			if (infra)
				smap[n].flags |= INFRARED;

			if (IS_BUILDING(cn))
				smap[n].flags2 = (unsigned int)map[m].flags;
			else
				smap[n].flags2 = 0;

			tmp = (x - ch[cn].x + VISI_SIZE/2) + (y - ch[cn].y + VISI_SIZE/2) * VISI_SIZE;

			if (see[cn].vis[tmp + 0 + 0] ||
			    see[cn].vis[tmp + 0 + VISI_SIZE] ||
			    see[cn].vis[tmp + 0 - VISI_SIZE] ||
			    see[cn].vis[tmp + 1 + 0] ||
			    see[cn].vis[tmp + 1 + VISI_SIZE] ||
			    see[cn].vis[tmp + 1 - VISI_SIZE] ||
			    see[cn].vis[tmp - 1 + 0] ||
			    see[cn].vis[tmp - 1 + VISI_SIZE] ||
			    see[cn].vis[tmp - 1 - VISI_SIZE])
			{
				visible = 1;
			}
			else
			{
				visible = 0;
			}

			if (!visible)
			{
				smap[n].flags |= INVIS;
			}

			// --- End of Flags ----- - more flags set in character part further down

			// --- Begin of Light -----
			if (light>64)		smap[n].light = 0;
			else if (light>52)	smap[n].light = 1;
			else if (light>40)	smap[n].light = 2;
			else if (light>32)	smap[n].light = 3;
			else if (light>28)	smap[n].light = 4;
			else if (light>24)	smap[n].light = 5;
			else if (light>20)	smap[n].light = 6;
			else if (light>16)	smap[n].light = 7;
			else if (light>14)	smap[n].light = 8;
			else if (light>12)	smap[n].light = 9;
			else if (light>10)	smap[n].light = 10;
			else if (light>8)	smap[n].light = 11;
			else if (light>6)	smap[n].light = 12;
			else if (light>4)	smap[n].light = 13;
			else if (light>2)	smap[n].light = 14;
			else				smap[n].light = 15;
			// --- End of Light -----

			// --- Begin of ba_sprite  -----
			smap[n].ba_sprite = map[m].sprite;
			// --- End of ba_sprite -----

			// --- Begin of Character ----- - also sets flags
			if (visible && (co = map[m].ch)!=0 && (tmp = do_char_can_see(cn, co, 0))!=0)
			{
				if (!ch[co].sprite_override)
				{
					smap[n].ch_sprite = ch[co].sprite;
				}
				else
				{
					smap[n].ch_sprite = ch[co].sprite_override;
				}
				smap[n].ch_status  = ch[co].status;
				smap[n].ch_status2 = ch[co].status2;
				smap[n].ch_speed = ch[co].speed;
				smap[n].ch_nr = co;
				smap[n].ch_id = (unsigned short)char_id(co);
				if (tmp<=75 && ch[co].hp[5]>0)
				{
					smap[n].ch_proz = ((ch[co].a_hp + 5) / 10) / ch[co].hp[5];
				}
				else
				{
					smap[n].ch_proz = 0;
				}
				// Sending packets for speed bonuses.
				// Used by local client to render frames correctly.
				smap[n].ch_atkspd  = ch[co].atk_speed;
				smap[n].ch_castspd = ch[co].cast_speed;
				smap[n].ch_movespd = ch[co].move_speed;
				// Inform the client that this character is special in some way by changing the overhead font color
				if (IS_PLAYER(co))														// Players are Orange
					smap[n].ch_fontcolor = 5;
				else if ((ch[co].flags & CF_EXTRAEXP) && (ch[co].flags & CF_EXTRACRIT))	// Extra EXP AND CRIT are Violet
					smap[n].ch_fontcolor = 8;
				else if (ch[co].flags & CF_EXTRAEXP)									// Extra EXP are Red
					smap[n].ch_fontcolor = 1;
				else if (ch[co].flags & CF_EXTRACRIT)									// Extra Crit are Blue
					smap[n].ch_fontcolor = 3;
				else																	// Otherwise yellow
					smap[n].ch_fontcolor = 0;
				//
				smap[n].flags |= ISCHAR;
				if (ch[co].stunned==1)
				{
					smap[n].flags |= STUNNED;
				}
				if (ch[co].flags & CF_STONED)
				{
					smap[n].flags |= STUNNED | STONED;
				}
				if (IS_SHADOW(co))
				{
					smap[n].flags |= STONED;
				}
				if (IS_BLOODY(co))
				{
					smap[n].flags |= BLOODY;
				}
			}
			else
			{
				smap[n].ch_sprite  = 0;
				smap[n].ch_status  = 0;
				smap[n].ch_status2 = 0;
				smap[n].ch_speed = 0;
				smap[n].ch_nr = 0;
				smap[n].ch_id = 0;
				smap[n].ch_proz = 0;
			}
			// --- End of Character -----

			// --- Begin of Item -----
			if (map[m].fsprite)
			{
				smap[n].it_sprite = map[m].fsprite;
				smap[n].it_status = 0;
			}
			else if (IS_SANEITEM(co = map[m].it) && (!(it[co].flags & (IF_TAKE | IF_HIDDEN)) || 
				(visible && do_char_can_see_item(cn, co))))
			{
				if (it[co].active)
				{
					smap[n].it_sprite = it[co].sprite[I_A];
					smap[n].it_status = it[co].status[I_A];
				}
				else
				{
					smap[n].it_sprite = it[co].sprite[I_I];
					smap[n].it_status = it[co].status[I_I];
				}
				if ((it[co].flags & IF_LOOK) || (it[co].flags & IF_LOOKSPECIAL))
				{
					smap[n].flags |= ISITEM;
				}
				if (!(it[co].flags & IF_TAKE) && it[co].flags & (IF_USE | IF_USESPECIAL))
				{
					smap[n].flags |= ISUSABLE;
				}
			}
			else
			{
				smap[n].it_sprite = 0;
				smap[n].it_status = 0;
			}
			// --- Begin of Item -----
		}
	}
	//player[nr].changed_field[p++]=-1;

	player[nr].vx = see[cn].x;
	player[nr].vy = see[cn].y;
}

#define YSCUTF (YSCUT+3)        //3
#define YECUTF (YECUT)          //1
#define XSCUTF (XSCUT)          //2
#define XECUTF (XECUT+3)        //2

void plr_getmap_fast(int nr)
{
	int cn, co, light, tmp, visible, infra = 0, infv = 5;
	int x, y, n, m;
	int ys, ye, xs, xe;
	unsigned char do_all = 0;
	struct cmap * smap;
	
	if (player[nr].spectating) 
		cn = player[nr].spectating;
	else 
		cn = player[nr].usnr;
	
	smap = player[nr].smap;

	ys = ch[cn].y - (TILEY / 2) + YSCUTF;
	ye = ch[cn].y + (TILEY / 2) - YECUTF;
	xs = ch[cn].x - (TILEX / 2) + XSCUTF;
	xe = ch[cn].x + (TILEX / 2) - XECUTF;

	// trigger recomputation of visibility map
	can_see(cn, ch[cn].x, ch[cn].y, ch[cn].x + 1, ch[cn].y + 1, 16);

	// check if visibility changed
	if (player[nr].vx!=see[cn].x || player[nr].vy!=see[cn].y || 
		mcmp(player[nr].visi, see[cn].vis, VISI_SIZE * VISI_SIZE))
	{
		mcpy(player[nr].visi, see[cn].vis, VISI_SIZE * VISI_SIZE);
		player[nr].vx = see[cn].x;
		player[nr].vy = see[cn].y;
		do_all = 1;
	}

	if (IS_BUILDING(cn))
	{
		do_all = 1;
	}

	for (n = YSCUTF * TILEX + XSCUTF, m = xs + ys * MAPX, y = ys; y<ye; 
			y++, m += MAPX - TILEX + XSCUTF + XECUTF, n += XSCUTF + XECUTF)
	{
		for (x = xs; x<xe; x++, n++, m++)
		{
			if (do_all || map[m].it || map[m].ch || 
				mcmp(&player[nr].xmap[n], &map[m], sizeof(struct map)))		// any change on map data?
			{
				mcpy(&player[nr].xmap[n], &map[m], sizeof(struct map));		// remember changed values
			}
			else
			{
				continue;
			}

			// field outside of map? then display empty one.
			if (x<0 || y<0 || x>=MAPX || y>=MAPY)
			{
				empty_field(smap, n);
				continue;
			}

			// calculate light
			tmp = check_dlightm(m);

			light = max(map[m].light, tmp);
			light = do_char_calc_light(cn, light);
			if (ch[cn].flags & CF_INFRARED)
			{
				if (M_SK(cn, SK_PERCEPT)> 20) infv = 4;
				if (M_SK(cn, SK_PERCEPT)> 40) infv = 3;
				if (M_SK(cn, SK_PERCEPT)> 60) infv = 2;
				if (M_SK(cn, SK_PERCEPT)> 80) infv = 1;
				if (M_SK(cn, SK_PERCEPT)>150) infv = 0;
			}
			if (light<=infv && (ch[cn].flags & CF_INFRARED) && !IS_IN_DW(ch[cn].x, ch[cn].y))
			{
				infra = 1;
			}
			else
			{
				infra = 0;
			}

			// everybody sees himself
			if (light==0 && map[m].ch==cn)
			{
				light = 1;
			}

			// no light, nothing visible
			if (light==0)
			{
				empty_field(smap, n);
				continue;
			}

			// Flags
			smap[n].flags  = 0;
			smap[n].flags2 = 0;

			if (map[m].flags & (MF_GFX_INJURED |
			                    MF_GFX_INJURED1 |
			                    MF_GFX_INJURED2 |
								MF_GFX_CRIT |
			                    MF_GFX_DEATH |
			                    MF_GFX_TOMB |
			                    MF_GFX_EMAGIC |
			                    MF_GFX_GMAGIC |
			                    MF_GFX_CMAGIC))
			{
				if (map[m].flags & MF_GFX_CRIT)
					smap[n].flags |= CRITTED;
				
				if (map[m].flags & MF_GFX_INJURED)
					smap[n].flags |= INJURED;
				if (map[m].flags & MF_GFX_INJURED1)
					smap[n].flags |= INJURED1;
				if (map[m].flags & MF_GFX_INJURED2)
					smap[n].flags |= INJURED2;

				if (map[m].flags & MF_GFX_DEATH)
					smap[n].flags |= (map[m].flags & MF_GFX_DEATH) >> 23;
				if (map[m].flags & MF_GFX_TOMB)
					smap[n].flags |= (map[m].flags & MF_GFX_TOMB) >> 23;
				if (map[m].flags & MF_GFX_EMAGIC)
					smap[n].flags |= (map[m].flags & MF_GFX_EMAGIC) >> 23;
				if (map[m].flags & MF_GFX_GMAGIC)
					smap[n].flags |= (map[m].flags & MF_GFX_GMAGIC) >> 23;
				if (map[m].flags & MF_GFX_CMAGIC)
					smap[n].flags |= (map[m].flags & MF_GFX_CMAGIC) >> 23;
			}

			if (map[m].flags & MF_UWATER)
				smap[n].flags |= UWATER;

			if (infra)
				smap[n].flags |= INFRARED;

			if (IS_BUILDING(cn))
				smap[n].flags2 = (unsigned int)map[m].flags;

			tmp = (x - ch[cn].x + VISI_SIZE/2) + (y - ch[cn].y + VISI_SIZE/2) * VISI_SIZE;

			if (see[cn].vis[tmp + 0 + 0] ||
			    see[cn].vis[tmp + 0 + VISI_SIZE] ||
			    see[cn].vis[tmp + 0 - VISI_SIZE] ||
			    see[cn].vis[tmp + 1 + 0] ||
			    see[cn].vis[tmp + 1 + VISI_SIZE] ||
			    see[cn].vis[tmp + 1 - VISI_SIZE] ||
			    see[cn].vis[tmp - 1 + 0] ||
			    see[cn].vis[tmp - 1 + VISI_SIZE] ||
			    see[cn].vis[tmp - 1 - VISI_SIZE])
			{
				visible = 1;
			}
			else
			{
				visible = 0;
			}

			if (!visible)
			{
				smap[n].flags |= INVIS;
			}

			if (light>64)		smap[n].light = 0;
			else if (light>52)	smap[n].light = 1;
			else if (light>40)	smap[n].light = 2;
			else if (light>32)	smap[n].light = 3;
			else if (light>28)	smap[n].light = 4;
			else if (light>24)	smap[n].light = 5;
			else if (light>20)	smap[n].light = 6;
			else if (light>16)	smap[n].light = 7;
			else if (light>14)	smap[n].light = 8;
			else if (light>12)	smap[n].light = 9;
			else if (light>10)	smap[n].light = 10;
			else if (light>8)	smap[n].light = 11;
			else if (light>6)	smap[n].light = 12;
			else if (light>4)	smap[n].light = 13;
			else if (light>2)	smap[n].light = 14;
			else				smap[n].light = 15;

			// background
			smap[n].ba_sprite = map[m].sprite;

			// character
			if (visible && (co = map[m].ch)!=0 && (tmp = do_char_can_see(cn, co, 0))!=0)
			{
				if (!ch[co].sprite_override)
				{
					smap[n].ch_sprite = ch[co].sprite;
				}
				else
				{
					smap[n].ch_sprite = ch[co].sprite_override;
				}
				smap[n].ch_status  = ch[co].status;
				smap[n].ch_status2 = ch[co].status2;
				smap[n].ch_speed = ch[co].speed;
				smap[n].ch_nr = co;
				smap[n].ch_id = (unsigned short)char_id(co);
				if (tmp<=75 && ch[co].hp[5]>0)
				{
					smap[n].ch_proz = ((ch[co].a_hp + 5) / 10) / ch[co].hp[5];
				}
				else
				{
					smap[n].ch_proz = 0;
				}
				// Sending packets for speed bonuses.
				// Used by local client to render frames correctly.
				smap[n].ch_atkspd  = ch[co].atk_speed;
				smap[n].ch_castspd = ch[co].cast_speed;
				smap[n].ch_movespd = ch[co].move_speed;
				if (IS_PLAYER(co))
				{
					smap[n].ch_fontcolor = 5;
				}
				else if (ch[co].flags & CF_EXTRAEXP)
				{
					// Inform the client this is a 'special' enemy by turning their font red
					smap[n].ch_fontcolor = 1;
				}
				else
				{
					// Otherwise just display normal yellow font
					smap[n].ch_fontcolor = 0;
				}
				//
				smap[n].flags |= ISCHAR;
				if (ch[co].stunned==1)
				{
					smap[n].flags |= STUNNED;
				}
				if (ch[co].flags & CF_STONED)
				{
					smap[n].flags |= STUNNED | STONED;
				}
				if (IS_SHADOW(co))
				{
					smap[n].flags |= STONED;
				}
				if (IS_BLOODY(co))
				{
					smap[n].flags |= BLOODY;
				}
			}
			else
			{
				smap[n].ch_sprite  = 0;
				smap[n].ch_status  = 0;
				smap[n].ch_status2 = 0;
				smap[n].ch_speed = 0;
				smap[n].ch_nr = 0;
				smap[n].ch_id = 0;
				smap[n].ch_proz = 0;
			}

			// item
			if (map[m].fsprite)
			{
				smap[n].it_sprite = map[m].fsprite;
				smap[n].it_status = 0;
			}
			else if ((co = map[m].it)!=0 && (!(it[co].flags & (IF_TAKE | IF_HIDDEN)) || 
				(visible && do_char_can_see_item(cn, co))))
			{
				if (it[co].active)
				{
					smap[n].it_sprite = it[co].sprite[I_A];
					smap[n].it_status = it[co].status[I_A];
				}
				else
				{
					smap[n].it_sprite = it[co].sprite[I_I];
					smap[n].it_status = it[co].status[I_I];
				}
				if ((it[co].flags & IF_LOOK) || (it[co].flags & IF_LOOKSPECIAL))
				{
					smap[n].flags |= ISITEM;
				}
				if (!(it[co].flags & IF_TAKE) && it[co].flags & (IF_USE | IF_USESPECIAL))
				{
					smap[n].flags |= ISUSABLE;
				}
			}
			else
			{
				smap[n].it_sprite = 0;
				smap[n].it_status = 0;
			}
		}
	}
	player[nr].vx = see[cn].x;
	player[nr].vy = see[cn].y;
}

void plr_clear_map(void)
{
	int n;

	for (n = 1; n<MAXPLAYER; n++)
	{
		bzero(player[n].smap, sizeof(player[n].smap));
		player[n].vx = 0; // force do_all in plr_getmap
	}
}

void plr_getmap(int nr)
{
	static int mode = 0;

	if (globs->load_avg>8000 && mode==0 && (globs->flags & GF_SPEEDY))
	{
		mode = 1;
		plr_clear_map();
		do_announce(0, 0, "Entered speed savings mode. Display will be imperfect.\n");
	}
	if ((!(globs->flags & GF_SPEEDY) || globs->load_avg<6500) && mode!=0)
	{
		mode = 0;
		do_announce(0, 0, "Left speed savings mode.\n");
	}

	if (mode==0)
	{
		plr_getmap_complete(nr);
	}
	else
	{
		plr_getmap_fast(nr);
	}
}

void stone_gc(int cn, int mode)
{
	int co;

	if (!(ch[cn].flags & CF_PLAYER))
	{
		return;
	}
	if (!(co = ch[cn].data[PCD_COMPANION]))
	{
		return;
	}
	if (!IS_ACTIVECHAR(co))
	{
		return;
	}
	if (ch[co].data[CHD_MASTER]!=cn)
	{
		return;
	}
	if (mode)
	{
		ch[co].flags |= CF_STONED;
	}
	else
	{
		ch[co].flags &= ~CF_STONED;
	}
}

void plr_tick(int nr)
{
	int cn;

	player[nr].ltick++;

	if (player[nr].state==ST_NORMAL)
	{
		cn = player[nr].usnr;

		if (cn && ch[cn].data[19] && (ch[cn].flags & CF_PLAYER) && !(map[ch[cn].x + ch[cn].y * MAPX].flags & MF_NOLAG))
		{
			if (player[nr].ltick>player[nr].rtick + ch[cn].data[19] && !(ch[cn].flags & CF_STONED))
			{
				chlog(cn, "Turned to stone due to lag (%.2fs)", (player[nr].ltick - player[nr].rtick) / 20.0);
				ch[cn].flags |= CF_STONED;
				stone_gc(cn, 1);
			}
			else if (player[nr].ltick<player[nr].rtick + ch[cn].data[19] - TICKS && (ch[cn].flags & CF_STONED))
			{
				chlog(cn, "Unstoned, lag is gone");
				ch[cn].flags &= ~CF_STONED;
				stone_gc(cn, 0);
			}
		}
	}
}

int check_valid(int cn)
{
	int n, in;

	if (ch[cn].x<1 || ch[cn].y<1 || ch[cn].x>MAPX - 2 || ch[cn].y>MAPY - 2)
	{
		chlog(cn, "Killed character %s (%d) for invalid data", ch[cn].name, cn);
		do_char_killed(0, cn, 0);
		return 0;
	}

	n = ch[cn].x + ch[cn].y * MAPX;
	if (map[n].ch!=cn)
	{
		chlog(cn, "Not on map (%d)!", map[n].ch);
		if (map[n].ch)
		{
			chlog(cn, "drop=%d", god_drop_char_fuzzy_large(cn, 900, 900, 900, 900));
		}
		else
		{
			map[n].ch = cn;
		}
	}

	if (IS_BUILDING(cn))
	{
		return 1;
	}

	for (n = 0; n<MAXITEMS; n++)
	{
		if ((in = ch[cn].item[n]))
		{
			if (it[in].carried!=cn || it[in].used!=USE_ACTIVE)
			{
				xlog("Reset item %d (%s,%d) from char %d (%s)", in, it[in].name, it[in].used, cn, ch[cn].name);
				ch[cn].item[n] = 0;
			//	ch[cn].item_lock[n] = 0;
			}
		}
	}
	for (n = 0; n<ST_PAGES*ST_SLOTS; n++)
	{
		if ((in = st[cn].depot[n/ST_SLOTS][n%ST_SLOTS]))
		{
			if (it[in].carried!=cn || it[in].used!=USE_ACTIVE)
			{
				xlog("Reset depot item %d (%s,%d) from char %d (%s)", in, it[in].name, it[in].used, cn, ch[cn].name);
				st[cn].depot[n/ST_SLOTS][n%ST_SLOTS] = 0;
			}
		}
	}

	for (n = 0; n<20; n++)
	{
		if ((in = ch[cn].worn[n]))
		{
			if (it[in].carried!=cn || it[in].used!=USE_ACTIVE)
			{
				xlog("Reset worn item %d (%s,%d) from char %d (%s)", in, it[in].name, it[in].used, cn, ch[cn].name);
				ch[cn].worn[n] = 0;
			}
		}
	}
	
	for (n = 0; n<12; n++)
	{
		if ((in = ch[cn].alt_worn[n]))
		{
			if (it[in].carried!=cn || it[in].used!=USE_ACTIVE)
			{
				xlog("Reset alt_worn item %d (%s,%d) from char %d (%s)", in, it[in].name, it[in].used, cn, ch[cn].name);
				ch[cn].alt_worn[n] = 0;
			}
		}
	}
	
	for (n = 0; n<4; n++)
	{
		if ((in = ch[cn].blacksmith[n]))
		{
			if (it[in].carried!=cn || it[in].used!=USE_ACTIVE)
			{
				xlog("Reset blacksmith item %d (%s,%d) from char %d (%s)", in, it[in].name, it[in].used, cn, ch[cn].name);
				ch[cn].blacksmith[n] = 0;
			}
		}
	}
	
	for (n = 0; n<MAXBUFFS; n++)
	{
		if ((in = ch[cn].spell[n]))
		{
			if (bu[in].carried!=cn || bu[in].used!=USE_ACTIVE)
			{
				xlog("Reset spell buff %d (%s,%d,%d) from char %d (%s)", in, bu[in].name, bu[in].carried, bu[in].used, cn, ch[cn].name);
				ch[cn].spell[n] = 0;
			}
		}
	}

	if ((ch[cn].flags & CF_STONED) && !(ch[cn].flags & CF_PLAYER))
	{
		int co;

		if (!(co = ch[cn].data[CHD_MASTER]) || !IS_ACTIVECHAR(co))
		{
			ch[cn].flags &= ~CF_STONED;
			chlog(cn, "oops, stoned removed");
		}
	}

	return 1;
}

void check_expire(int cn)
{
	int erase = 0, t, /*year,*/ week, day;
	
	day   = 60*60*24;
	week  = day*7;
//	year  = day*365;

	t = time(NULL);
	
	if (!IS_SEYA_OR_BRAV(cn) && !IS_LYCANTH(cn) && !IS_RB(cn))
	{
		if (ch[cn].points_tot==0)
		{
			if (ch[cn].login_date +   3 * day < t) 
				erase = 1;
		}
		else if (ch[cn].points_tot < 52500) // Staff Sergeant
		{
			if (ch[cn].login_date +   3 * week < t)	
				erase = 1;
		}
	}
	
	if (erase)
	{
		if (ch[cn].flags & (CF_IMP | CF_GOD))
		{
			xlog("Player %s, %d exp has expired, but will not be deleted (Imp/God).", ch[cn].name, ch[cn].points_tot);
		}
		else
		{
			xlog("Player %s, %d exp has expired, and has been deleted.", ch[cn].name, ch[cn].points_tot);
			god_destroy_items(cn);
			ch[cn].used=USE_EMPTY;
		}
	}
}

inline int group_active(int cn)
{
	if ((ch[cn].flags & (CF_PLAYER | CF_USURP | CF_NOSLEEP)) && ch[cn].used==USE_ACTIVE)
	{
		return 1;
	}

	if (ch[cn].data[92])
	{
		return 1;
	}

	return 0;
}

char *strnchr(char *ptr, char c, int len)
{
	while (len)
	{
		if (*ptr==c)
		{
			return( ptr);
		}
		ptr++;
		len--;
	}
	return(NULL);
}

static long cl = 0;
static int  cltick = 0;
static int  wakeup = 1;

void tick(void)
{
	int n, cnt, hour, awake, body, online = 0, plon;
	unsigned long long prof;
	time_t t;
	struct tm *tm;

	prof_tick();

	t  = time(NULL);
	tm = localtime(&t);
	hour = tm->tm_hour;

	globs->ticker++;
	globs->uptime++;

	globs->uptime_per_hour[hour]++;
	
	if ((globs->ticker & 31)==0)
	{
		pop_save_char(globs->ticker % MAXCHARS);
	}
	
	// Process map editor queue
	int queue_prev = 20; // Process this many after finding the first USE_EMPTY instruction
	for (int i=0; i<MAX_MAPED_QUEUE; i++) {
			if (maped_queue[i].used == USE_EMPTY) {
					queue_prev--;
					if (queue_prev <= 0) break;
					continue;
			}

			xlog("processed maped queue: type=%d, x=%d, y=%d, it_temp=%d",
					maped_queue[i].op_type, maped_queue[i].x, maped_queue[i].y, maped_queue[i].it_temp);

			switch(maped_queue[i].op_type) {
					case MAPED_PLACEITEM: // Place item
							if (maped_queue[i].it_temp > 0) {
									build_drop(maped_queue[i].x, maped_queue[i].y, maped_queue[i].it_temp);
							}
					break;

					case MAPED_RMVITEM: // Remove item
							build_remove(maped_queue[i].x, maped_queue[i].y);
					break;

					case MAPED_SETFLOOR: // Change floor
							map[maped_queue[i].x + maped_queue[i].y * MAPX].sprite = maped_queue[i].it_temp;
					break;
			}
			maped_queue[i].used = USE_EMPTY;
			queue_prev = 20;
	}

	// send tick to players
	for (n = 1; n<MAXPLAYER; n++)
	{
		if (!player[n].sock)
		{
			continue;
		}
		if (player[n].state!=ST_NORMAL && player[n].state!=ST_EXIT)
		{
			continue;
		}
		plr_tick(n);
		if (player[n].state==ST_NORMAL)
		{
			online++;
		}
	}

	if (online>globs->max_online)
	{
		globs->max_online = online;
	}
	if (online>globs->max_online_per_hour[hour])
	{
		globs->max_online_per_hour[hour] = online;
	}

	// check for player commands and translate to character commands, also does timeout checking
	for (n = 1; n<MAXPLAYER; n++)
	{
		if (!player[n].sock)
		{
			continue;
		}
		while (player[n].in_len>=16)
		{
			plr_cmd(n);
			player[n].in_len -= 16;
			memmove(player[n].inbuf, player[n].inbuf + 16, 240);
		}
		plr_idle(n);
	}

	// do login stuff
	for (n = 1; n<MAXPLAYER; n++)
	{
		if (!player[n].sock)
		{
			continue;
		}
		if (player[n].state==ST_NORMAL)
		{
			continue;
		}
		plr_state(n);
	}
	
	// send changes to players
	for (n = 1; n<MAXPLAYER; n++)
	{
		if (!player[n].sock)
		{
			continue;
		}
		if (player[n].state!=ST_NORMAL)
		{
			continue;
		}
//#define SPEEDTEST
#ifdef SPEEDTEST
		{
			int zz;

			for (zz = 0; zz<80; zz++)
			{
				prof = prof_start();
				plr_getmap(n);
				prof_stop(10, prof);
				prof = prof_start();
				plr_change(n);
				prof_stop(11, prof);
			}
		}
#else
		prof = prof_start();
		plr_getmap(n);
		prof_stop(10, prof);
		prof = prof_start();
		plr_change(n);
		prof_stop(11, prof);
#endif
	}

	// let characters act
	cnt = awake = body = plon = 0;

	if ((globs->ticker & 63)==0)
	{
		if (wakeup>=MAXCHARS)
		{
			wakeup = 1;
		}
		ch[wakeup++].data[92] = TICKS * 60;
	}

	for (n = 1; n<MAXCHARS; n++)
	{
		if (ch[n].used==USE_EMPTY)
		{
			continue;
		}
		cnt++;

		if (ch[n].flags & CF_UPDATE)
		{
			really_update_char(n);
			ch[n].flags &= ~CF_UPDATE;
		}

		if (ch[n].used==USE_NONACTIVE && (n & 1023)==(globs->ticker & 1023))
		{
			// Check if we delete the character
			check_expire(n);
		}
		if (ch[n].flags & CF_BODY)
		{
			if (!(ch[n].flags & CF_PLAYER) && ch[n].data[98]++>TICKS * 60 * 30)
			{
				chlog(n, "removing lost body");
				god_destroy_items(n);
				ch[n].used = USE_EMPTY;
				continue;
			}
			body++;
			continue;
		}

		if (!IS_PLAYER(n) && ch[n].data[92]>0)
		{
			ch[n].data[92]--;               // reduce single awake
		}
		if (ch[n].status<8 && !group_active(n))
		{
			// Character is idle/standing still and not a player/usurp/nosleep
			continue;
		}

		awake++;

		if (ch[n].used==USE_ACTIVE)
		{
			if ((n & 1023)==(globs->ticker & 1023) && !check_valid(n))
			{
				continue;
			}
			ch[n].current_online_time++;
			ch[n].total_online_time++;
			if (ch[n].flags & (CF_PLAYER | CF_USURP))
			{
				globs->total_online_time++;
				globs->online_per_hour[hour]++;
				if ((ch[n].flags & CF_PLAYER) && ch[n].data[71]>0)
				{
					ch[n].data[71]--;
				}
				if ((ch[n].flags & CF_PLAYER) && !(ch[n].flags & CF_INVISIBLE))
				{
					plon++;
				}
			}
			prof = prof_start();
			plr_act(n);
			prof_stop(12, prof);
		}
		do_regenerate(n);
	}
	globs->character_cnt = cnt;
	globs->awake = awake;
	globs->body  = body;
	globs->players_online = plon;

	prof = prof_start();
	pop_tick();
	prof_stop(13, prof);
	prof = prof_start();
	effect_tick();
	prof_stop(14, prof);
	prof = prof_start();
	item_tick();
	prof_stop(15, prof);

	// do global updates like time of day, weather etc.
	prof = prof_start();
	global_tick();
	prof_stop(26, prof);

	ctick++;
	if (ctick>199)	// Feb 2020 - extended from 20 to 24 to 200
	{
		ctick = 0;
	}

	cltick++;
	if (cltick>17)
	{
		cltick = 0;
		n = clock() / (CLOCKS_PER_SEC / 100);
		if (cl && n>cl)
		{
			globs->load = n - cl;
			globs->load_avg = (int)((globs->load_avg * 0.0099 + globs->load * 0.01) * 100);
		}
		cl = n;
	}
}

void player_exit(int nr)
{
	int cn;

	player[nr].state = ST_EXIT;
	player[nr].lasttick = globs->ticker;

	cn = player[nr].usnr;
	if (cn && cn>0 && cn<MAXCHARS && ch[cn].player==nr)
	{
		ch[cn].player = 0;
	}
}

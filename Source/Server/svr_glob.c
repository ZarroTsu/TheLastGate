/*************************************************************************

   This file is part of 'Mercenaries of Astonia v2'
   Copyright (c) 1997-2001 Daniel Brockhaus (joker@astonia.com)
   All rights reserved.

 **************************************************************************/

#include <stdio.h>

#include "server.h"

FILE *discordFile;

void sanguine_spawners(int cn)
{
	int in, n = 0, m = 0, j = 0;
	int ina[999] = {0};
	int inb[999] = {0};
	int x=0, y=0, rank=0;
	
	if (cn)
	{
		rank = getrank(cn) + (IS_RB(cn)?1:0);
		y = 1028 + (rank-5)*30;
	}
	
	for (in = 1; in<MAXITEM; in++)
	{
		if (it[in].used==USE_EMPTY) continue;
		if (it[in].temp==IT_SANGCRYS4)
		{
			if (cn && it[in].y < y)  continue;
			inb[m] = in;
			m++;
			continue;
		}
		if (cn) continue;
		if (it[in].temp==IT_SANGCRYS1)
		{
			j++;
			continue;
		}
		if (it[in].temp==IT_SANGCRYS3)
		{
			ina[n] = in;
			n++;
			continue;
		}
	}
	
	if (m > 0)
	{
		m = RANDOM(m);
		in = inb[m];
		if (in)
		{
			x = it[in].x;
			y = it[in].y;
			reset_go(x, y); remove_lights(x, y);
			build_remove(x, y);
			build_drop(x, y, IT_SANGCRYS2);
			reset_go(x, y); add_lights(x, y);
		}
	}
	
	if (cn || j > 13) return;
	
	if (n > 0)
	{
		n = RANDOM(n);
		in = ina[n];
		if (in)
		{
			x = it[in].x;
			y = it[in].y;
			reset_go(x, y); remove_lights(x, y);
			build_remove(x, y);
			build_drop(x, y, IT_SANGCRYS1);
			reset_go(x, y); add_lights(x, y);
		}
	}
}

void do_misc(void)
{
	int cn;

	for (cn = 1; cn<MAXCHARS; cn++)
	{
		if (ch[cn].used==USE_EMPTY)
		{
			continue;
		}
		if (!(ch[cn].flags & (CF_PLAYER)))
		{
			continue;
		}

		if (count_uniques(cn)>2)
		{
			if (ch[cn].used==USE_ACTIVE)
			{
				ch[cn].luck -= 5;
				chlog(cn, "reduced luck by 5 to %d for having more than one unique", ch[cn].luck);
			}
		}
		else
		{
			if (ch[cn].luck<0)
			{
				ch[cn].luck++;
			}
			if (ch[cn].luck<0)
			{
				ch[cn].luck++;
			}
		}
		ch[cn].flags &= ~(CF_SHUTUP | CF_NODESC | CF_KICKED);
		if (!(globs->mdday % 28)) ch[cn].colosseum = 0;
	}
}

void init_lights_check(void)
{
	int n, players = 0;
	for (n = 1; n<MAXCHARS; n++)
	{
		if (!(ch[n].flags & (CF_PLAYER)))	continue;
		if (ch[n].used!=USE_ACTIVE)			continue;
		players++;
	}
	if (!players) 
	{
		god_reset_npcs(0);
		init_lights();
	}
}

void global_pandium_rewards(void)
{
	int n;
	
	xlog("Attemping Pandium Rewards");
	
	for (n=0;n<3;n++)
	{
		if (IS_SANEPLAYER(globs->top_ps[n])) god_give_char(god_create_item(2974), globs->top_ps[n]);
		if (IS_SANEPLAYER(globs->top_pg[n])) god_give_char(god_create_item(2974), globs->top_pg[n]);
		globs->top_ps[n] = 0;
		globs->top_pg[n] = 0;
	}
	
	xlog("Pandium Rewards complete.");
}

void reset_daily_shops(void)
{
	int n;
	for (n = 1; n<MAXCHARS; n++)
	{
		if (ch[n].used!=USE_ACTIVE) continue;
		if (ch[n].temp==CT_NULLAN)  update_shop(n);
	}
}

void global_tick(void)
{
	int tmp;

	globs->mdtime++;
	if (globs->mdtime>=MD_DAY)
	{
		globs->mdday++;
		globs->mdtime = 0;
		xlog("day %d of the year %d begins", globs->mdday, globs->mdyear);
		reset_daily_shops();
		//sanguine_spawners(0);
		do_misc();
		if (!(globs->mdday % 28)) init_lights_check();
	}

	if (globs->mdday>=MD_YEAR)
	{
		char message[100];
		globs->mdyear++;
		globs->mdday = 1;
		
		if (globs->mdyear%5==0)
		{
			sprintf(message, "A new year begins in Astonia! Archon records have been reset, and rewards have been distributed!");
			if (globs->flags & GF_DISCORD) discord_ranked(message);
			global_pandium_rewards();
			if (globs->mdyear%10==0) 
			{
				sprintf(message, "A new year begins in Astonia! Archon records have been reset, and rewards have been distributed!");
				god_reset_all_depths(0);
			}
			else
			{
				sprintf(message, "A new year begins in Astonia! Archon rewards have been distributed!");
			}
		}
		else
		{
			sprintf(message, "A new year begins in Astonia!");
			if (globs->flags & GF_DISCORD) discord_ranked(message);
		}
	}
	
	if (IS_GLOB_MAYHEM)
	{
		globs->dlight = 0;
	}
	else
	{
		if (globs->mdtime<MD_HOUR * 6)
		{
			globs->dlight = 0;
		}
		else if (globs->mdtime<MD_HOUR * 7)
		{
			globs->dlight = (globs->mdtime - MD_HOUR * 6) * 255 / MD_HOUR;
		}
		else if (globs->mdtime<MD_HOUR * 22)
		{
			globs->dlight = 255;
		}
		else if (globs->mdtime<MD_HOUR * 23)
		{
			globs->dlight = (MD_HOUR * 23 - globs->mdtime) * 255 / MD_HOUR;
		}
		else
		{
			globs->dlight = 0;
		}
	}

	tmp = globs->mdday % 28 + 1;

	globs->newmoon  = 0;
	globs->fullmoon = 0;
	
	if (globs->flags & GF_DISCORD) 
	{
		if (!(globs->mdtime % TICKS*10)) discord_who();
		discord_shout_in();
	}
	
	if (IS_GLOB_MAYHEM)
	{
		return;
	}
	
	if (tmp==1)
	{
		globs->newmoon = 1;
		return;
	}
	if (tmp==15)
	{
		globs->fullmoon = 1;
	}
	
	if (tmp>14)
	{
		tmp = 28 - tmp;
	}
	if (tmp>globs->dlight)
	{
		globs->dlight = tmp;
	}
}

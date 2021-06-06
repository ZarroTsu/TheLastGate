/*************************************************************************

   This file is part of 'Mercenaries of Astonia v2'
   Copyright (c) 1997-2001 Daniel Brockhaus (joker@astonia.com)
   All rights reserved.

 **************************************************************************/

#include <malloc.h>
#include <string.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <fcntl.h>
#include <stdarg.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "server.h"

//static int ox=0,oy=0;
//static char visi[40*40];

static char *visi;
static char  _visi[VISI_SIZE * VISI_SIZE]; // 02162020 - updated from 40x40 to 60x60 for larger client render
static int   ox = 0, oy = 0;
static char  ismonster = 0;
static char  isbuilding = 0;

static inline void add_vis(int x, int y, int v)
{
	// 02162020 - updated for larger client render
	if (!visi[(x - ox + VISI_SIZE/2) + (y - oy + VISI_SIZE/2) * VISI_SIZE])
	{
		visi[(x - ox + VISI_SIZE/2) + (y - oy + VISI_SIZE/2) * VISI_SIZE] = v;
	}
}

static inline int check_map_see(int x, int y)
{
	int m;

	if (x<=0 || x>=MAPX || y<=0 || y>=MAPY)
	{
		return( 0);
	}

	m = x + y * MAPX;
	
	if (isbuilding)
	{
		return(1);
	}
	
	if (ismonster)
	{
		if (map[m].flags & (MF_SIGHTBLOCK | MF_NOMONST))
		{
			return( 0);
		}
	}
	else
	{
		if (map[m].flags & MF_SIGHTBLOCK)
		{
			return( 0);
		}
	}
	if (map[m].it && (it[map[m].it].flags & IF_SIGHTBLOCK))
	{
		return( 0);
	}

	return(1);
}

static inline int check_map_go(int x, int y)
{
	int m;

	if (x<=0 || x>=MAPX || y<=0 || y>=MAPY)
	{
		return( 0);
	}

	m = x + y * MAPX;

	if (map[m].flags & MF_MOVEBLOCK)
	{
		return( 0);
	}

	if (map[m].it && (it[map[m].it].flags & IF_MOVEBLOCK))
	{
		return( 0);
	}

	return(1);
}

static inline int close_vis_see(int x, int y, int v)
{
	// 02162020 - updated 20's and 40's for larger client render
	if (!check_map_see(x, y))
	{
		return( 0);
	}

	x = x - ox + VISI_SIZE/2;
	y = y - oy + VISI_SIZE/2;

	if (visi[(x + 1) + (y) * VISI_SIZE]==v)
	{
		return( 1);
	}
	if (visi[(x - 1) + (y) * VISI_SIZE]==v)
	{
		return( 1);
	}
	if (visi[(x) + (y + 1) * VISI_SIZE]==v)
	{
		return( 1);
	}
	if (visi[(x) + (y - 1) * VISI_SIZE]==v)
	{
		return( 1);
	}
	if (visi[(x + 1) + (y + 1) * VISI_SIZE]==v)
	{
		return( 1);
	}
	if (visi[(x + 1) + (y - 1) * VISI_SIZE]==v)
	{
		return( 1);
	}
	if (visi[(x - 1) + (y + 1) * VISI_SIZE]==v)
	{
		return( 1);
	}
	if (visi[(x - 1) + (y - 1) * VISI_SIZE]==v)
	{
		return( 1);
	}

	return(0);
}

static inline int close_vis_go(int x, int y, int v)
{
	// 02162020 - updated 20's and 40's for larger client render
	if (!check_map_go(x, y))
	{
		return( 0);
	}

	x = x - ox + VISI_SIZE/2;
	y = y - oy + VISI_SIZE/2;

	if (visi[(x + 1) + (y) * VISI_SIZE]==v)
	{
		return( 1);
	}
	if (visi[(x - 1) + (y) * VISI_SIZE]==v)
	{
		return( 1);
	}
	if (visi[(x) + (y + 1) * VISI_SIZE]==v)
	{
		return( 1);
	}
	if (visi[(x) + (y - 1) * VISI_SIZE]==v)
	{
		return( 1);
	}
	if (visi[(x + 1) + (y + 1) * VISI_SIZE]==v)
	{
		return( 1);
	}
	if (visi[(x + 1) + (y - 1) * VISI_SIZE]==v)
	{
		return( 1);
	}
	if (visi[(x - 1) + (y + 1) * VISI_SIZE]==v)
	{
		return( 1);
	}
	if (visi[(x - 1) + (y - 1) * VISI_SIZE]==v)
	{
		return( 1);
	}

	return(0);
}

static inline int check_vis(int x, int y)
{
	// 02162020 - updated 20's and 40's for larger client render
	int best = 99;

	x = x - ox + VISI_SIZE/2;
	y = y - oy + VISI_SIZE/2;

	if (visi[(x + 1) + (y + 0) * VISI_SIZE] && visi[(x + 1) + (y + 0) * VISI_SIZE]<best)
	{
		best = visi[(x + 1) + (y + 0) * VISI_SIZE];
	}
	if (visi[(x - 1) + (y + 0) * VISI_SIZE] && visi[(x - 1) + (y + 0) * VISI_SIZE]<best)
	{
		best = visi[(x - 1) + (y + 0) * VISI_SIZE];
	}
	if (visi[(x + 0) + (y + 1) * VISI_SIZE] && visi[(x + 0) + (y + 1) * VISI_SIZE]<best)
	{
		best = visi[(x + 0) + (y + 1) * VISI_SIZE];
	}
	if (visi[(x + 0) + (y - 1) * VISI_SIZE] && visi[(x + 0) + (y - 1) * VISI_SIZE]<best)
	{
		best = visi[(x + 0) + (y - 1) * VISI_SIZE];
	}
	if (visi[(x + 1) + (y + 1) * VISI_SIZE] && visi[(x + 1) + (y + 1) * VISI_SIZE]<best)
	{
		best = visi[(x + 1) + (y + 1) * VISI_SIZE];
	}
	if (visi[(x + 1) + (y - 1) * VISI_SIZE] && visi[(x + 1) + (y - 1) * VISI_SIZE]<best)
	{
		best = visi[(x + 1) + (y - 1) * VISI_SIZE];
	}
	if (visi[(x - 1) + (y + 1) * VISI_SIZE] && visi[(x - 1) + (y + 1) * VISI_SIZE]<best)
	{
		best = visi[(x - 1) + (y + 1) * VISI_SIZE];
	}
	if (visi[(x - 1) + (y - 1) * VISI_SIZE] && visi[(x - 1) + (y - 1) * VISI_SIZE]<best)
	{
		best = visi[(x - 1) + (y - 1) * VISI_SIZE];
	}

	if (best==99)
	{
		return( 0);
	}
	else
	{
		return( best);
	}
}

static void can_map_see(int _fx, int _fy, int maxdist)
{
	// 02162020 - updated 20's and 40's for larger client render
	int xc, yc, x, y, dist;

	bzero(visi, sizeof(char) * VISI_SIZE * VISI_SIZE);

	ox = _fx;
	oy = _fy;
	xc = _fx;
	yc = _fy;

	add_vis(_fx, _fy, 1);

	for (dist = 1; dist<maxdist + 1; dist++)
	{
		for (x = xc - dist; x<=xc + dist; x++)
		{
			if (close_vis_see(x, yc - dist, dist))
			{
				add_vis(x, yc - dist, dist + 1);
			}
			if (close_vis_see(x, yc + dist, dist))
			{
				add_vis(x, yc + dist, dist + 1);
			}
		}
		for (y = yc - dist + 1; y<=yc + dist - 1; y++)
		{
			if (close_vis_see(xc - dist, y, dist))
			{
				add_vis(xc - dist, y, dist + 1);
			}
			if (close_vis_see(xc + dist, y, dist))
			{
				add_vis(xc + dist, y, dist + 1);
			}
		}
	}
}

static void can_map_go(int _fx, int _fy, int maxdist)
{
	// 02162020 - updated 20's and 40's for larger client render
	int xc, yc, x, y, dist;

	bzero(visi, sizeof(char) * VISI_SIZE * VISI_SIZE);

	ox = _fx;
	oy = _fy;
	xc = _fx;
	yc = _fy;

	add_vis(_fx, _fy, 1);

	for (dist = 1; dist<maxdist + 1; dist++)
	{
		for (x = xc - dist; x<=xc + dist; x++)
		{
			if (close_vis_go(x, yc - dist, dist))
			{
				add_vis(x, yc - dist, dist + 1);
			}
			if (close_vis_go(x, yc + dist, dist))
			{
				add_vis(x, yc + dist, dist + 1);
			}
		}
		for (y = yc - dist + 1; y<=yc + dist - 1; y++)
		{
			if (close_vis_go(xc - dist, y, dist))
			{
				add_vis(xc - dist, y, dist + 1);
			}
			if (close_vis_go(xc + dist, y, dist))
			{
				add_vis(xc + dist, y, dist + 1);
			}
		}
	}
}

void reset_go(int xc, int yc)
{
	int x, y, cn;

	for (y = max(0, yc - 18); y<min(MAPY - 1, yc + 18); y++)
	{
		for (x = max(0, xc - 18); x<min(MAPX - 1, xc + 18); x++)
		{
			if ((cn = map[x + y * MAPX].ch)!=0)
			{
				see[cn].x = see[cn].y = 0;
			}
		}
	}

	ox = oy = 0;
}

int can_see(int cn, int _fx, int _fy, int tx, int ty, int maxdist)
{
	int tmp;
	extern int see_hit, see_miss;
	unsigned long long prof;

	prof = prof_start();

	if (cn)
	{
		visi = see[cn].vis;
		if (_fx!=see[cn].x || _fy!=see[cn].y)
		{
			if ((ch[cn].kindred & KIN_MONSTER) && !(ch[cn].flags & (CF_USURP | CF_THRALL)))
			{
				ismonster = 1;
			}
			else
			{
				ismonster = 0;
			}
			if (IS_BUILDING(cn))
			{
				isbuilding = 1;
			}
			else
			{
				isbuilding = 0;
			}
			can_map_see(_fx, _fy, maxdist);
			see[cn].x = _fx;
			see[cn].y = _fy;
			see_miss++;
		}
		else
		{
			see_hit++;
			ox = _fx;
			oy = _fy;
		}
	}
	else
	{
		if (visi!=_visi)
		{
			visi = _visi;
			ox = oy = 0;
		}
		if (ox!=_fx || oy!=_fy)
		{
			ismonster = 0;
			can_map_see(_fx, _fy, maxdist);
		}
	}

	tmp = check_vis(tx, ty);

	prof_stop(16, prof);

	return(tmp);
}

int can_go(int _fx, int _fy, int tx, int ty)
{
	int tmp;
	unsigned long long prof;

	prof = prof_start();

	if (visi!=_visi)
	{
		visi = _visi;
		ox = oy = 0;
	}
	if (ox!=_fx || oy!=_fy)
	{
		can_map_go(_fx, _fy, 15);
	}
	tmp = check_vis(tx, ty);

	prof_stop(17, prof);

	return(tmp);
}

/* moved to svr_tick for speed reasons
   int check_dlight(int x,int y)
   {
        int m;

        m=x+y*MAPX;

        if (!(map[m].flags&MF_INDOORS)) return globs->dlight;

        return (globs->dlight*map[m].dlight)/256;
   } */

void compute_dlight(int xc, int yc)
{
	int xs, ys, xe, ye, x, y, v, d, best = 0, m;
	unsigned long long prof;

	prof = prof_start();

	xs = max(0, xc - LIGHTDIST);
	ys = max(0, yc - LIGHTDIST);
	xe = min(MAPX - 1, xc + 1 + LIGHTDIST);
	ye = min(MAPY - 1, yc + 1 + LIGHTDIST);

	for (y = ys; y<ye; y++)
	{
		m = y * MAPX + xs;
		for (x = xs; x<xe; x++, m++)
		{
			if ((xc - x) * (xc - x) + (yc - y) * (yc - y)>(LIGHTDIST * LIGHTDIST + 1))
			{
				continue;
			}
			if (!(map[m].flags & MF_INDOORS))
			{
				if ((v = can_see(0, xc, yc, x, y, LIGHTDIST))==0)
				{
					continue;
				}
				d = 256 / (v * (abs(xc - x) + abs(yc - y)));
				if (d>best)
				{
					best = d;
				}
			}
		}
	}
	if (best>256)
	{
		best = 256;
	}
	map[xc + yc * MAPX].dlight = best;

	prof_stop(18, prof);
}

void remove_lights(int x, int y)
{
	int xs, ys, xe, ye, in, cn, v, m;
	unsigned long long prof;

	prof = prof_start();

	xs = max(1, x - LIGHTDIST);
	ys = max(1, y - LIGHTDIST);
	xe = min(MAPX - 2, x + 1 + LIGHTDIST);
	ye = min(MAPY - 2, y + 1 + LIGHTDIST);

	for (y = ys; y<ye; y++)
	{
		m = y * MAPX + xs;
		for (x = xs; x<xe; x++, m++)
		{
			if ((in = map[m].it)!=0)
			{
				if (it[in].active)
				{
					if ((v = it[in].light[1])!=0)
					{
						do_add_light(x, y, -v);
					}
				}
				else
				{
					if ((v = it[in].light[0])!=0)
					{
						do_add_light(x, y, -v);
					}
				}
			}

			if ((cn = map[m].ch)!=0)
			{
				if ((v = ch[cn].light)!=0)
				{
					do_add_light(x, y, -v);
				}
			}
			map[m].dlight = 0;
		}
	}

	prof_stop(19, prof);
}

void add_lights(int x, int y)
{
	int xs, ys, xe, ye, in, cn, v, m;
	unsigned long long prof;

	prof = prof_start();

	xs = max(1, x - LIGHTDIST);
	ys = max(1, y - LIGHTDIST);
	xe = min(MAPX - 2, x + 1 + LIGHTDIST);
	ye = min(MAPY - 2, y + 1 + LIGHTDIST);

	for (y = ys; y<ye; y++)
	{
		m = y * MAPX + xs;
		for (x = xs; x<xe; x++, m++)
		{
			if ((in = map[m].it)!=0)
			{
				if (it[in].active)
				{
					if ((v = it[in].light[1])!=0)
					{
						do_add_light(x, y, v);
					}
				}
				else
				{
					if ((v = it[in].light[0])!=0)
					{
						do_add_light(x, y, v);
					}
				}
			}
			if ((cn = map[m].ch)!=0)
			{
				if ((v = ch[cn].light)!=0)
				{
					do_add_light(x, y, v);
				}
			}
			if (map[m].flags & MF_INDOORS)
			{
				compute_dlight(x, y);
			}
		}
	}

	prof_stop(20, prof);
}

int char_id(int cn)
{
	int id = 0, n;

	for (n = 0; n<40; n += sizeof(int))
	{
		id ^= *(unsigned int*)(&ch[cn].name[n]);
	}

	id ^= ch[cn].pass1;
	id ^= ch[cn].pass2;

	return(id);
}

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
	switch(v)
	{
		case  0:	return       250;
		case  1:	return      1750;
		case  2:	return      7000;
		case  3:	return     21000;
		case  4:	return     52500;
		case  5:	return    115500;
		case  6:	return    231000;
		case  7:	return    429000;
		case  8:	return    750750;
		case  9:	return   1251250;
		case 10:	return   2002000;
		case 11:	return   3094000;
		case 12:	return   4641000;
		case 13:	return   6783000;
		case 14:	return   9690000;
		case 15:	return  13566000;
		case 16:	return  18653250;
		case 17:	return  25236750;
		case 18:	return  33649000;
		case 19:	return  44275000;
		case 20:	return  57557500;
		case 21:	return  74002500;
		case 22:	return  94185000;
		default:	return 118755000;
	}
}

/* Calculates experience to next level from current experience and the
   points2rank() function. As no inverse function is supplied we use a
   binary search to determine the experience for the next level.
   If the given number of points corresponds to the highest level,
   return 0. */
int points_tolevel(int curr_exp)
{
	int curr_level, next_level, p0, p5, p9, r, j;

	curr_level = points2rank(curr_exp);
	if (curr_level >= RANKS-1)
	{
		return( 0);
	}
	next_level = curr_level + 1;

	p0 = 1;
	p5 = 1;
	p9 = 20 * curr_exp;
	for (j = 0; p0<p9 && j<100; j++)
	{
		p5 = (p0 + p9) / 2;
		r  = points2rank(curr_exp + p5);
		if (r < next_level)
		{
			p0 = p5 + 1;
		}
		else
		{
			p9 = p5 - 1;
		}
	}
	if (p0 > (20 * curr_exp))
	{
		return( 0);                     // Can't do it
	}
	p5++;
	return(p5);
}

int rankdiff(int cn, int co)
{
	return(points2rank(ch[co].points_tot) - points2rank(ch[cn].points_tot));
}

int absrankdiff(int cn, int co)
{
	int rd = rankdiff(cn, co);
	return (rd < 0 ? -rd : rd);
}

int in_attackrange(int cn, int co)
{
	return (absrankdiff(cn, co) <= ATTACK_RANGE);
}

int in_grouprange(int cn, int co)
{
	return (absrankdiff(cn, co) <= GROUP_RANGE);
}

int scale_exps2(int cn, int co_rank, int exp)
{
	static float scale_tab[RANKS*2+1] = {
		0.04,					// -25
		0.04, 0.04, 0.04, 0.04, // -24, -23, -22, -21
		0.04, 0.04, 0.04, 0.04, // -20, -19, -18, -17
		0.12, 0.19, 0.26, 0.33, // -16, -15, -14, -13
		0.40, 0.46, 0.52, 0.58, // -12, -11, -10, - 9
		0.64, 0.69, 0.74, 0.79, // - 8, - 7, - 6, - 5
		0.84, 0.88, 0.92, 0.96, // - 4, - 3, - 2, - 1
		
		1.00,					// +- 0
		
		1.02, 1.06, 1.12, 1.20, // + 1, + 2, + 3, + 4
		1.30, 1.42, 1.56, 1.72, // + 5, + 6, + 7, + 8
		1.90, 2.10, 2.32, 2.56, // + 9, +10, +11, +12
		2.82, 3.10, 3.40, 3.72, // +13, +14, +15, +16
		4.00, 4.00, 4.00, 4.00, // +17, +18, +19, +20
		4.00, 4.00, 4.00, 4.00, // +21, +22, +23, +24
		4.00					// +25
	};
	int diff;

	diff = co_rank - points2rank(ch[cn].points_tot);

	diff += RANKS;
	if (diff<0)
	{
		diff = 0;
	}
	if (diff>RANKS*2)
	{
		diff = RANKS*2;
	}

//      xlog("scale %d to %d: diff=%d, scale=%f",points2rank(ch[cn].points_tot),co_rank,diff,scale_tab[diff]);

	return((int)(exp * scale_tab[diff]));

}

int scale_exps(int cn, int co, int exp)
{
	return(scale_exps2(cn, points2rank(ch[co].points_tot), exp));
}

/* CS, 991128: Ranks rearranged for clarity */
char *rank_name[RANKS] = {
	"Private",           	"Private First Class", "Lance Corporal",		//  0  1  2
	"Corporal",          	"Sergeant", 			"Staff Sergeant",		//  3  4  5
	"Master Sergeant",   	"First Sergeant", 		"Sergeant Major",		//  6  7  8
	"Second Lieutenant", 	"First Lieutenant", 	"Captain",				//  9 10 11
	"Major",             	"Lieutenant Colonel", 	"Colonel",				// 12 13 14
	"Brigadier General", 	"Major General", 		"Lieutenant General",	// 15 16 17
	"General",           	"Field Marshal", 		"Knight of Astonia",	// 18 19 20
	"Baron of Astonia", 	"Earl of Astonia", 		"Marquess of Astonia", 	// 21 22 23
	"Warlord of Astonia"  													// 24
};

char *who_rank_name[RANKS] = {
	" Pvt ", " PFC ", " LCp ",
	" Cpl ", " Sgt ", " SSg ",
	" MSg ", " 1Sg ", " SgM ",
	"2Lieu", "1Lieu", "Captn",
	"Major", "LtCol", "Colnl",
	"BrGen", "MaGen", "LtGen",
	"Genrl", "FDMAR", "KNIGT",
	"BARON", " EARL", "MARQS",
	"WARLD"
};

int create_special_item(int temp)
{
	int in, mul = 1, spr, roll=21, legendary = 0, n, prev=-1;
	char *pref, *suffix, name[40], newname[60];

	in = god_create_item(temp, 0);
	if (!in)
	{
		return( 0);
	}
	
	it[in].temp = 0;
	
	// Special 'Legendary' affixes
	if (!RANDOM(15))
	{
		legendary = 1;
		roll = 14;
	}
	
	// Pick a prefix
	switch(RANDOM(roll))
	{
		case  0: 
		case  1: 
			if (legendary) pref = "Leo "; else pref = "Lion's ";
			it[in].attrib[AT_BRV][0] += 3; 
			break;
		case  2: 
		case  3: 
			if (legendary) pref = "Anguis "; else pref = "Snake's "; 
			it[in].attrib[AT_WIL][0] += 3; 
			break;
		case  4: 
		case  5: 
			if (legendary) pref = "Ibis "; else pref = "Owl's "; 
			it[in].attrib[AT_INT][0] += 3; 
			break;
		case  6: 
		case  7: 
			if (legendary) pref = "Mus "; else pref = "Weasel's "; 
			it[in].attrib[AT_AGL][0] += 3; 
			break;
		case  8: 
		case  9: 
			if (legendary) pref = "Ursa "; else pref = "Bear's "; 
			it[in].attrib[AT_STR][0] += 3; 
			break;
		case 10: 
		case 11: 
			if (legendary) pref = "Angelus "; else pref = "Angelic "; 
			it[in].attrib[AT_BRV][0] += 1; 
			it[in].attrib[AT_WIL][0] += 1; 
			it[in].attrib[AT_INT][0] += 1; 
			it[in].attrib[AT_AGL][0] += 1; 
			it[in].attrib[AT_STR][0] += 1; 
			break;
		case 12: 
			if (legendary) pref = "Lux "; else pref = "Glowing "; 
			it[in].light[0] += 20; 
			break;
		case 13: 
			if (legendary) pref = "Deus "; else pref = "Godly "; 
			mul = 2; 
			break;
		default: 
			pref = ""; 
			break;
	}
	
	roll=RANDOM(42);

	// Pick a suffix, or two if legendary
	for (n=0;n<=legendary;n++)
	{
		while (roll==prev) // avoid picking the same thing twice if legendary
		{
			roll=RANDOM(42);
		}
		prev = roll;
		
		switch(roll)
		{
			case  0:
			case  1:
				if (legendary) suffix = "Fortem"; else suffix = " of Braveness";
				it[in].attrib[AT_BRV][0] += 4 * mul;
				break;
			case  2:
			case  3:
				if (legendary) suffix = "Autem"; else suffix = " of Willpower";
				it[in].attrib[AT_WIL][0] += 4 * mul;
				break;
			case  4:
			case  5:
				if (legendary) suffix = "Intuitio"; else suffix = " of Intuition";
				it[in].attrib[AT_INT][0] += 4 * mul;
				break;
			case  6:
			case  7:
				if (legendary) suffix = "Agilitas"; else suffix = " of Agility";
				it[in].attrib[AT_AGL][0] += 4 * mul;
				break;
			case  8:
			case  9:
				if (legendary) suffix = "Viribus"; else suffix = " of Strength";
				it[in].attrib[AT_STR][0] += 4 * mul;
				break;
			case 10:
				if (legendary) suffix = "Callidus"; else suffix = " of the Adept";
				it[in].attrib[AT_BRV][0] += 2 * mul; 
				it[in].attrib[AT_WIL][0] += 2 * mul; 
				it[in].attrib[AT_INT][0] += 2 * mul; 
				it[in].attrib[AT_AGL][0] += 2 * mul; 
				it[in].attrib[AT_STR][0] += 2 * mul; 
				break;
			case 11:
			case 12:
				if (legendary) suffix = "Salutem"; else suffix = " of Hitpoints";
				it[in].hp[0] += 40 * mul;
				break;
			case 13:
			case 14:
				if (legendary) suffix = "Pati"; else suffix = " of Endurance";
				it[in].end[0] += 20 * mul;
				break;
			case 15:
			case 16:
				if (legendary) suffix = "Magus"; else suffix = " of Mana";
				it[in].mana[0] += 40 * mul;
				break;
			case 17:
				if (legendary) suffix = "Impetus"; else suffix = " of Offense";
				it[in].weapon[0] += 3 * mul;
				break;
			case 18:
				if (legendary) suffix = "Defendere"; else suffix = " of Defense";
				it[in].armor[0] += 3 * mul;
				break;
			case 19:
				if (legendary) suffix = "Immunis"; else suffix = " of Immunity";
				it[in].skill[SK_IMMUN][0] += 3 * mul;
				break;
			case 20:
				if (legendary) suffix = "Resistere"; else suffix = " of Resistance";
				it[in].skill[SK_RESIST][0] += 3 * mul;
				break;
			case 21:
			case 22:
				if (
					temp==IT_HELM_CAST || temp==IT_BODY_CAST || 
					temp==IT_HELM_ADEP || temp==IT_BODY_ADEP || 
					temp==IT_HELM_WIZR || temp==IT_BODY_WIZR ||
					((temp==IT_HELM_DAMA || temp==IT_BODY_DAMA) && RANDOM(2))
				)
				{
					if (legendary) suffix = "Pugione"; else suffix = " of the Dagger";
					it[in].skill[SK_DAGGER][0] += 3 * mul;
				}
				else
				{
					if (legendary) suffix = "Manibus"; else suffix = " of the Unarmed";
					it[in].skill[SK_HAND][0] += 3 * mul;
				}
				break;
			case 23:
			case 24:
				if (
					temp==IT_HELM_CAST || temp==IT_BODY_CAST || 
					temp==IT_HELM_ADEP || temp==IT_BODY_ADEP || 
					temp==IT_HELM_WIZR || temp==IT_BODY_WIZR ||
					((temp==IT_HELM_DAMA || temp==IT_BODY_DAMA) && RANDOM(2))
				)
				{
					if (legendary) suffix = "Virgam"; else suffix = " of the Staff";
					it[in].skill[SK_STAFF][0] += 3 * mul;
				}
				else
				{
					if (legendary) suffix = "Gladio"; else suffix = " of the Sword";
					it[in].skill[SK_SWORD][0] += 3 * mul;
				}
				break;
			case 25:
			case 26:
				if (
					temp==IT_HELM_CAST || temp==IT_BODY_CAST || 
					temp==IT_HELM_ADEP || temp==IT_BODY_ADEP || 
					temp==IT_HELM_WIZR || temp==IT_BODY_WIZR ||
					((temp==IT_HELM_DAMA || temp==IT_BODY_DAMA) && RANDOM(2))
				)
				{
					if (legendary) suffix = "Benedicite"; else suffix = " of Blessing";
					it[in].skill[SK_BLESS][0] += 4 * mul;
				}
				else
				{
					if (legendary) suffix = "Securis"; else suffix = " of the Axe";
					it[in].skill[SK_AXE][0] += 3 * mul;
				}
				break;
			case 27:
			case 28:
				if (
					temp==IT_HELM_CAST || temp==IT_BODY_CAST || 
					temp==IT_HELM_ADEP || temp==IT_BODY_ADEP || 
					temp==IT_HELM_WIZR || temp==IT_BODY_WIZR ||
					((temp==IT_HELM_DAMA || temp==IT_BODY_DAMA) && RANDOM(2))
				)
				{
					if (legendary) suffix = "Maledictum"; else suffix = " of Cursing";
					it[in].skill[SK_CURSE][0] += 4 * mul;
				}
				else
				{
					if (legendary) suffix = "Magna"; else suffix = " of the Twohander";
					it[in].skill[SK_TWOHAND][0] += 3 * mul;
				}
				break;
			case 29:
			case 30:
				if (
					temp==IT_HELM_CAST || temp==IT_BODY_CAST || 
					temp==IT_HELM_ADEP || temp==IT_BODY_ADEP || 
					temp==IT_HELM_WIZR || temp==IT_BODY_WIZR ||
					((temp==IT_HELM_DAMA || temp==IT_BODY_DAMA) && RANDOM(2))
				)
				{
					if (legendary) suffix = "Tarda"; else suffix = " of Slowing";
					it[in].skill[SK_SLOW][0] += 4 * mul;
				}
				else
				{
					if (legendary) suffix = "Adductius"; else suffix = " of Cleaving";
					it[in].skill[SK_CLEAVE][0] += 4 * mul;
				}
				break;
			case 31:
			case 32:
				if (
					temp==IT_HELM_CAST || temp==IT_BODY_CAST || 
					temp==IT_HELM_ADEP || temp==IT_BODY_ADEP || 
					temp==IT_HELM_WIZR || temp==IT_BODY_WIZR ||
					((temp==IT_HELM_DAMA || temp==IT_BODY_DAMA) && RANDOM(2))
				)
				{
					if (legendary) suffix = "Praemium"; else suffix = " of Blasting";
					it[in].skill[SK_BLAST][0] += 4 * mul;
				}
				else
				{
					if (legendary) suffix = "Infirmi"; else suffix = " of Weakening";
					it[in].skill[SK_WEAKEN][0] += 4 * mul;
				}
				break;
			case 33:
			case 34:
				if (
					temp==IT_HELM_CAST || temp==IT_BODY_CAST || 
					temp==IT_HELM_ADEP || temp==IT_BODY_ADEP || 
					temp==IT_HELM_WIZR || temp==IT_BODY_WIZR ||
					((temp==IT_HELM_DAMA || temp==IT_BODY_DAMA) && RANDOM(2))
				)
				{
					if (legendary) suffix = "Familia"; else suffix = " of Company";
					it[in].skill[SK_GHOST][0] += 4 * mul;
				}
				else
				{
					if (legendary) suffix = "Regio"; else suffix = " of Surrounding";
					it[in].skill[SK_SURROUND][0] += 5 * mul;
				}
				break;
			case 35:
			case 36:
				if (
					temp==IT_HELM_CAST || temp==IT_BODY_CAST || 
					temp==IT_HELM_ADEP || temp==IT_BODY_ADEP || 
					temp==IT_HELM_WIZR || temp==IT_BODY_WIZR ||
					((temp==IT_HELM_DAMA || temp==IT_BODY_DAMA) && RANDOM(2))
				)
				{
					if (legendary) suffix = "Clypeus"; else suffix = " of Shielding";
					it[in].skill[SK_MSHIELD][0] += 4 * mul;
				}
				else
				{
					if (legendary) suffix = "Furorem"; else suffix = " of Taunting";
					it[in].skill[SK_TAUNT][0] += 4 * mul;
				}
				break;
			//
			case 37:
				if (legendary) suffix = "Caecus"; else suffix = " of Blinding";
				it[in].skill[SK_BLIND][0] += 4 * mul;
				break;
			case 39:
				if (legendary) suffix = "Sana"; else suffix = " of Healing";
				it[in].skill[SK_HEAL][0] += 4 * mul;
				break;
			case 40:
				if (legendary) suffix = "Renati"; else suffix = " of Regeneration";
				it[in].skill[SK_REGEN][0] += 5 * mul;
				break;
			case 41:
				if (legendary) suffix = "Requiem"; else suffix = " of Resting";
				it[in].skill[SK_REST][0] += 5 * mul;
				break;
			case 42:
				if (legendary) suffix = "Meditor"; else suffix = " of Meditation";
				it[in].skill[SK_MEDIT][0] += 5 * mul;
				break;
		}
	}

	switch(temp)
	{
		case IT_HELM_BRNZ: spr = 180; 	break;
		case IT_BODY_BRNZ: spr = 181; 	break;
		case IT_HELM_STEL: spr = 182; 	break;
		case IT_BODY_STEL: spr = 183; 	break;
		case IT_HELM_GOLD: spr = 184; 	break;
		case IT_BODY_GOLD: spr = 185; 	break;
		case IT_HELM_EMER: spr = 186; 	break;
		case IT_BODY_EMER: spr = 187; 	break;
		case IT_HELM_CRYS: spr = 188; 	break;
		case IT_BODY_CRYS: spr = 189; 	break;
		case IT_HELM_TITN: spr = 190;	break;
		case IT_BODY_TITN: spr = 191;	break;
		case IT_HELM_ADAM: spr = 192; 	break;
		case IT_BODY_ADAM: spr = 193; 	break;
		case IT_HELM_CAST: spr = 194; 	break;
		case IT_BODY_CAST: spr = 195; 	break;
		case IT_HELM_ADEP: spr = 196; 	break;
		case IT_BODY_ADEP: spr = 197; 	break;
		case IT_HELM_WIZR: spr = 198; 	break;
		case IT_BODY_WIZR: spr = 199; 	break;
		case IT_HELM_DAMA: spr =3733; 	break;
		case IT_BODY_DAMA: spr =3734; 	break;
		//
		default: spr = it[in].sprite[0]; break;
	}

	it[in].sprite[0]  = spr;
	it[in].max_damage = 0;
	it[in].flags |= IF_SINGLEAGE;
	
	strcpy(name, it[in].name);
	
	xlog("create_special_item: %s%s%s (%d)", pref, name, suffix, strlen(pref)+strlen(name)+strlen(suffix));
	
	if (legendary)
	{
		sprintf(newname, "%s%s, %s", pref, suffix, name);
	}
	else
	{
		sprintf(newname, "%s%s%s", pref, name, suffix);
	}
	
	for (n=39;n<60;n++)	newname[n]='\0'; // cull excess characters from string
	
	sprintf(it[in].name, "%s", newname);
	sprintf(it[in].reference, "%s", newname);
	sprintf(it[in].description, "A %s.", newname);
	it[in].name[0] = toupper(it[in].name[0]);

	return(in);
}


struct npc_class
{
	char *name;
};

struct npc_class npc_class[] = {
	{""							},	// 0  --  Stays blank (null)
	//
	{"Weak Thief"				},	// 1
	{"Crawler"					},	// 2
	{"Weak Skeleton"			},	// 3
	{"Weak Harakim"				},	// 4
	{"Weak Outlaw"				},	// 5
	{"Skeleton"					},	// 6
	{"Weak Ghost"				},	// 7
	{"Outlaw"					},	// 8
	{"Thief"					},	// 9
	{"Creeper"					},	// 10
	//
	{"Weak Spider"				},	// 11
	{"Spider"					},	// 12
	{"Strong Spider"			},	// 13
	{"Ghost"					},	// 14
	{"Bandit"					},	// 15
	//
	{"Arachnid"					},	// 16
	{"Templar"					},	// 17
	{"Strong Skeleton"          },	// 18
	{"Tarantula"                },	// 19
	{"Strong Templar"           },	// 20
	{"Strong Harakim"           },	// 21
	{"First Lieu. Grolm"		},	// 22
	{"First Lieu. Seagrel"      },	// 23
	{"Robber"					},	// 24
	{"Very Strong Skeleton"		},	// 25
	//
	{"Strong Bandit"			},	// 26
	{"Mud Golem"				},	// 27
	{"Specter"					},	// 28
	{"Grudge"					},	// 29
	{"Swamp Lizard"				},	// 30
	{"Viking"					},	// 31
	{"Wraith"					},	// 32
	{"Wight"					},	// 33
	{"Demilich"					},	// 34
	//
	{"Strong Undead"			},	// 35
	{"Living Flame"				},	// 36
	{"Colonel Ice Gargoyle"		},	// 37
	//
	{"Gargoyle Knight"			},	// 38
	{"Gargoyle Mage"			},	// 39
	//
	{""},	// 40
	{""},	// 41
	{""},	// 42
	{""},	// 43
	{""},	// 44
	{""},	// 45
	{""},	// 46
	{""},	// 47
	{""},	// 48
	{""},	// 49
	{""},	// 50
	{""},	// 51
	{""},	// 52
	{""},	// 53
	//
	{"Gargoyle Queen"			},	// 54
	{"Brig. Gen Ice Gargoyle"	},	// 55
	{"Major Gen Ice Gargoyle"	},	// 56
	{"Lieu. Gen Ice Gargoyle"	},	// 57
	{"General Ice Gargoyle"		},	// 58
	{"Ice Gargoyle Queen"		},	// 59
	//
	{"Second Lieu. Thrall"		},	// 60
	{"First Lieu. Thrall"		},	// 61
	{"Captain Thrall"			},	// 62
	{"Major Thrall"				},	// 63
	{"Lt.Colonel Thrall"		},	// 64
	{"Colonel Vampire"			},	// 65
	{"Brig. General Vampire"	},	// 66
	{"Major General Vampire"	},	// 67
	{"Lieu. General Vampire"	},	// 68
	{"General Vampire"			},	// 69
	{"F.Marshal Vampire"		},	// 70
	//
	{"Weak Skeleton Miner"		},	// 71
	{"Skeleton Miner"			},	// 72
	{"Silver Golem"				},	// 73
	{"Magma Gargoyle"			},	// 74
	{"Marble Golem"				},	// 75
	//
	{"Grolm Soldier"			},	// 76
	{"Grolm Mage"				},	// 77
	{"Grolm King"				},	// 78
	{"Lizard Youth"				},	// 79
	{"Lizard Worker"			},	// 80
	{"Lizard Seer"				},	// 81
	{"Undead"					},	// 82
	{"Reptite"					},	// 83 *
	{"Spellcaster"				},	// 84
	{"Puppeteer"				},	// 85
	{"Mad Knight"				},	// 86
	{"Old Bones"				},	// 87
	{"Sand Golem"				},	// 88
	{"Pharoh"					},	// 89
	{"Undead King"				},	// 90
	{"Stone Golem"				},	// 91
	{"Librarian"				},	// 92
	{"Golemancer"				},	// 93
	{"Grolm Trapper"			},	// 94
	{"Weaver"					},	// 95
	{"Iguana"					},	// 96
	{"Wood Golem"				},	// 97
	{"Sculpture"				},	// 98
	{"Barbarian"				},	// 99
	{"Magmaling"				},	// 100
	//
	{"Ratling"					},	// 101
	{"Ratling Fighter"			},	// 102
	{"Ratling Warrior"			},	// 103
	{"Ratling Knight"			},	// 104
	{"Ratling Baron"			},	// 105
	{"Ratling Count"			},	// 106
	{"Ratling Duke"				},	// 107
	{"Ratling Prince"			},	// 108
	{"Ratling King"				},	// 109
	{"Ratling Guard"			},	// 110
	//
	{"Greenling"				},	// 111
	{"Greenling Fighter"		},	// 112
	{"Greenling Warrior"		},	// 113
	{"Greenling Knight"			},	// 114
	{"Greenling Baron"			},	// 115
	{"Greenling Count"			},	// 116
	{"Greenling Duke"			},	// 117
	{"Greenling Prince"			},	// 118
	{"Greenling King"			},	// 119
	{"Greenling Veteran"		},	// 120
	//
	{"Sogling"					},	// 121
	{"Sogling Fighter"			},	// 122
	{"Sogling Warrior"			},	// 123
	{"Sogling Knight"			},	// 124
	{"Sogling Baron"			},	// 125
	{"Sogling Count"			},	// 126
	{"Sogling Duke"				},	// 127
	{"Sogling Prince"			},	// 128
	{"Sogling King"				},	// 129
	{"Sogling Archmage"			},	// 130
	//
	{"Private Grolm"			},	// 131
	{"PFC Grolm"				},	// 132
	{"Lance Corporal Grolm"		},	// 133
	{"Corporal Grolm"			},	// 134
	{"Sergeant Grolm"			},	// 135
	{"Staff Sergeant Grolm"		},	// 136
	{"Master Sergeant Grolm"	},	// 137
	{"First Sergeant Grolm"		},	// 138
	{"Sergeant Major Grolm"		},	// 139
	{"Second Lieu. Grolm"		},	// 140
	{"Second Lieu. Gargoyle"	},	// 141
	{"First Lieu. Gargoyle"		},	// 142
	{"Captain Gargoyle"			},	// 143
	{"Major Gargoyle"			},	// 144
	{"Lt.Colonel Gargoyle"		},	// 145
	{"Colonel Gargoyle"			},	// 146
	{"Brig. General Gargoyle"	},	// 147
	{"Brig. General Grulge"		},	// 148
	{"Major General Grulge"		},	// 149
	{"Lieu. General Grulge"		},	// 150
	{"General Grulge"			},	// 151
	{"F.Marshal Grulge"			},	// 152
	{"F.Marshal Ice Gargoyle"	},	// 153
	{"Knight Ice Gargoyle"		},	// 154
	{"Baron Ice Gargoyle"		},	// 155
	{"Baron Seagrel"			},	// 156
	{"Earl Seagrel"				},	// 157
	{"Earl Onyx Gargoyle"		},	// 158
	{"Warlord Onyx Gargoyle"	}	// 159
};

int killed_class(int cn, int val)
{
	int bit, tmp;

	if (val<32)
	{
		bit = 1 << (val);
		tmp = ch[cn].data[60] & bit;
		ch[cn].data[60] |= bit;
		return(tmp);
	}
	else if (val<64)
	{
		bit = 1 << (val - 32);
		tmp = ch[cn].data[61] & bit;
		ch[cn].data[61] |= bit;
		return(tmp);
	}
	else if (val<96)
	{
		bit = 1 << (val - 64);
		tmp = ch[cn].data[62] & bit;
		ch[cn].data[62] |= bit;
		return(tmp);
	}
	else if (val<128)
	{
		bit = 1 << (val - 96);
		tmp = ch[cn].data[63] & bit;
		ch[cn].data[63] |= bit;
		return(tmp);
	}
	else
	{
		bit = 1 << (val - 128);
		tmp = ch[cn].data[70] & bit;
		ch[cn].data[70] |= bit;
		return(tmp);
	}
}

char *get_class_name(int nr)
{
	/* CS, 991128: Check for out-of-bounds on class number */
	if (nr < 0)
	{
		xlog("error: get_class_name(%d)", nr);
		return("err... nothing");
	}
	else if (nr >= ARRAYSIZE(npc_class))
	{
		xlog("error: get_class_name(%d)", nr);
		return("umm... whatzit");
	}
	return(npc_class[nr].name);
}


/* Convert case in str as for a proper name: 1st is capital, others lower.
   BEWARE, this function changes its argument! */
void titlecase_str(char *str)
{
	if (*str)
	{
		*str = toupper(*str);
		for (str++; *str; str++)
		{
			*str = tolower(*str);
		}
	}
}

/* Returns 1 if the <abbr> is an abbreviation of <name> */
int prefix(char *abbr, char *name)
{
	if (!*abbr)
	{
		return( 0);
	}
	for (; *abbr; abbr++, name++)
	{
		if (*abbr != *name)
		{
			return( 0);
		}
	}
	return(1);
}


/* Convert a time difference in ticks to a number of months, days, hours, minutes */
/* NOTE: The time string returned is a shared static, so it will not keep fresh
   if someone else calls this function. */
char *ago_string(int dt)
{
	int minutes, hours, days, months;
	static char when_string[100];

	minutes = dt / (60 * TICKS);
	if (minutes <= 0)
	{
		strcpy(when_string, "just now");
	}
	else if (minutes < 60)
	{
		sprintf(when_string, "%d minutes ago", minutes);
	}
	else
	{
		hours = minutes / 60;
		if (hours <= 36)
		{
			sprintf(when_string, "%d hours ago", hours);
		}
		else
		{
			days = hours / 24;
			if (days <= 61)
			{
				sprintf(when_string, "%d days ago", days);
			}
			else
			{
				months = days / 30; // This is sloppy, I know.
				sprintf(when_string, "%d months ago", months);
			}
		}
	}
	return(when_string);
}

int use_labtransfer(int cn, int nr, int exp)
{
	int x, y, co;

	for (y = LAB_ARENA_TOP_Y; y<=LAB_ARENA_BOTTOM_Y; y++)
	{
		for (x = LAB_ARENA_TOP_X; x<=LAB_ARENA_BOTTOM_X; x++)
		{
			if ((co = map[x + y * MAPX].ch) && (ch[co].flags & (CF_PLAYER | CF_LABKEEPER)))
			{
				do_char_log(cn, 0, "Sorry, the area is still busy. %s is there.\n", ch[co].name);
				chlog(cn, "Sorry, the area is still busy. %s is there", ch[co].name);
				return(0);
			}
		}
	}

	switch(nr)
	{
		case 1:
			co = pop_create_char(CT_LAB_1_BOSS, 0);
			break;                                          // grolms
		case 2:
			co = pop_create_char(CT_LAB_2_BOSS, 0);
			break;                                          // lizard
		case 3:
			co = pop_create_char(CT_LAB_3_BOSS, 0);
			break;                                          // spellcaster
		case 4:
			co = pop_create_char(CT_LAB_4_BOSS, 0);
			break;                                          // knight
		case 5:
			co = pop_create_char(CT_LAB_5_BOSS, 0);
			break;                                          // undead
		case 6:
			co = pop_create_char(CT_LAB_6_BOSS, 0);
			break;                                          // light&dark
		case 7:
			co = pop_create_char(CT_LAB_7_BOSS, 0);
			break;                                          // underwater
		case 8:
			co = pop_create_char(CT_LAB_8_BOSS, 0);
			break;                                          // forest / golem
		case 9:
			co = pop_create_char(CT_LAB_9_BOSS, 0);
			break;                                          // riddle
		case 10:
			co = pop_create_char(CT_LAB10_BOSS, 0);
			break;                                          // seasons
		case 11:
			co = pop_create_char(CT_LAB11_BOSS, 0);
			break;                                          // seasons
		case 12:
			co = pop_create_char(CT_LAB12_BOSS, 0);
			break;                                          // seasons
		default:
			do_char_log(cn, 0, "Sorry, could not determine which enemy to send you.\n");
			chlog(cn, "Sorry, could not determine which enemy to send you");
			return( 0);
	}


	if (!co)
	{
		do_char_log(cn, 0, "Sorry, could not create your enemy.\n");
		chlog(cn, "Sorry, could not create your enemy");
		return(0);
	}

	if (!god_drop_char(co, LAB_ARENA_BOSS_X, LAB_ARENA_BOSS_Y))
	{
		do_char_log(cn, 0, "Sorry, could not place your enemy.\n");
		chlog(cn, "Sorry, could not place your enemy");
		god_destroy_items(co);
		ch[co].used = USE_EMPTY;
		return(0);
	}

	ch[co].data[64] = globs->ticker + 5 * 60 * TICKS; // die in two minutes if not otherwise
	ch[co].data[24] = 0;    // do not interfere in fights
	ch[co].data[36] = 0;    // no walking around
	ch[co].data[43] = 0;    // don't attack anyone
	ch[co].data[80] = 0;    // no enemies

	ch[co].data[0] = cn;      // person to make solve
	ch[co].data[1] = nr;      // labnr
	ch[co].data[2] = exp;     // exp plr is supposed to get
	ch[co].flags |= CF_LABKEEPER | CF_NOSLEEP;
	ch[co].flags &= ~CF_RESPAWN;

	npc_add_enemy(co, cn, 1); // make him attack the solver

	if (!god_transfer_char(cn, LAB_ARENA_SPAWN_X, LAB_ARENA_SPAWN_Y))
	{
		do_char_log(cn, 0, "Sorry, could not transfer you to your enemy.\n");
		chlog(cn, "Sorry, could not transfer you to your enemy");
		god_destroy_items(co);
		ch[co].used = USE_EMPTY;
		return(0);
	}
	chlog(cn, "Entered Labkeeper room");

	return(1);
}

void use_labtransfer2(int cn, int co)
{
	int cc;

	if (IS_COMPANION(cn) && (cc = ch[cn].data[63])==ch[co].data[0])
	{
		do_char_log(cc, 0, "Your Companion killed your enemy.\n");
		finish_laby_teleport(cc, ch[co].data[1], ch[co].data[2]);
		god_transfer_char(cn, HOME_TEMPLE_X, HOME_TEMPLE_Y);
		chlog(cc, "Labkeeper room solved by GC");
		return;
	}
	if (ch[co].data[0]!=cn)
	{
		do_char_log(cn, 0, "Sorry, this killing does not count, as you're not the designated killer.\n");
		chlog(cn, "Sorry, this killing does not count, as you're not the designated killer");
		return;
	}
	finish_laby_teleport(cn, ch[co].data[1], ch[co].data[2]);
	chlog(cn, "Solved Labkeeper Room");

	if ((cc = ch[cn].data[64]) && IS_SANENPC(cc) && IS_COMPANION(cc))         // transfer GC as well
	{
		god_transfer_char(cc, HOME_TEMPLE_X, HOME_TEMPLE_Y);
	}
}

//----------------------

static char **badword = NULL;
static int cursize = 0, maxsize = 0;

void init_badwords(void)
{
	char buf[80], *dst, *src;
	FILE *fp;

	if (badword)
	{
		free(badword);
		badword = NULL;
		cursize = maxsize = 0;
	}

	fp = fopen("badwords.txt", "r");
	if (!fp)
	{
		return;
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
			badword  = realloc(badword, maxsize * sizeof(char**));
		}
		badword[cursize++] = strdup(buf);
	}

	fclose(fp);
}

int is_badword(char *sentence)
{
	int n;
	char temp[255];

	if (strlen(sentence)>250)
	{
		return( 0);
	}
	strcpy(temp, sentence);
	strlower(temp);

	for (n = 0; n<cursize; n++)
	{
		if (strstr(temp, badword[n]))
		{
			return( 1);
		}
	}
	return(0);
}

// checks for "bad" words in player talking and mutes them
void player_analyser(int cn, char *text)
{
	if (ch[cn].flags & CF_SHUTUP)
	{
		return;
	}

	/* 
	// Old badword checker. This is kind of... moot after all these years, and we're all adults here. For now.
	// Going to retool this particular data slot ( 72 ) for internal quest flags. So avoid uncommetning this.
	if (is_badword(text))
	{
		ch[cn].data[x] += TICKS * 50;
		if (ch[cn].data[x]>TICKS * 120)
		{
			do_char_log(cn, 0, "Don't say I didn't warn you. Now I'll shut your mouth for you!\n");
			ch[cn].flags |= CF_SHUTUP;
			chlog(cn, "Auto-Shutup for \"%s\" (%d)", text, ch[cn].data[x] / TICKS);
		}
		else if (ch[cn].data[x]>TICKS * 80)
		{
			do_char_log(cn, 0, "My, what a filthy mouth you have. You'd better keep it closed for a while!\n");
			chlog(cn, "Bad-Mouth warning for \"%s\" (%d)", text, ch[cn].data[x] / TICKS);
		}
	}
	*/
}

void show_time(int cn)
{
	int hour, minute, day, month, year;

	hour = globs->mdtime / (60 * 60);
	minute = (globs->mdtime / 60) % 60;
	day = globs->mdday % 28 + 1;
	month = globs->mdday / 28 + 1;
	year  = globs->mdyear;

	do_char_log(cn, 1, "It's %d:%02d on the %d%s%s%s%s%s%s%s of the %d%s%s%s%s month of the year %d.\n",
	            hour, minute,
	            day,
	            day==1  ? "st" : "",
	            day==2  ? "nd" : "",
	            day==3  ? "rd" : "",
	            day==21 ? "st" : "",
	            day==22 ? "nd" : "",
	            day==23 ? "rd" : "",
	            (day>3 && day<21) || day>23 ? "th" : "",
	            month,
	            month==1 ? "st" : "",
	            month==2 ? "nd" : "",
	            month==3 ? "rd" : "",
	            month>3  ? "th" : "",
	            year);
}

void scprintf(char *dst, char *format, ...)
{
	va_list va;

	while (*dst)
	{
		dst++;
	}

	va_start(va, format);
	vsprintf(dst, format, va);
	va_end(va);
}

void effectlist(int cn)
{
	int n;
	char buf[256];

	for (n = 1; n<MAXEFFECT; n++)
	{
		if (!fx[n].used)
		{
			continue;
		}

		sprintf(buf, "%3d: ", n);

		switch(fx[n].type)
		{
		case 1:
			scprintf(buf, "REMINJ ");
			break;
		case 2:
			scprintf(buf, "RSPAWN (%d,%d t=%d)", fx[n].data[0], fx[n].data[1], fx[n].data[2]);
			break;
		case 3:
			scprintf(buf, "DTMIST ");
			break;
		case 4:
			scprintf(buf, "TSTONE ");
			break;
		case 5:
			scprintf(buf, "IMAGIC ");
			break;
		case 6:
			scprintf(buf, "GMAGIC ");
			break;
		case 7:
			scprintf(buf, "CMAGIC ");
			break;
		case 8:
			scprintf(buf, "RSMIST ");
			break;
		case 9:
			scprintf(buf, "MCREAT ");
			break;
		case 10:
			scprintf(buf, "RSOBJT ");
			break;
		case 11:
			scprintf(buf, "RQUEUE ");
			break;
		case 12:
			scprintf(buf, "TRMIST ");
			break;
		default:
			scprintf(buf, "?????? ");
			break;
		}
		scprintf(buf, "\n");
		do_char_log(cn, 0, buf);
	}
}

#define CAP (globs->cap)

void set_cap(int cn, int nr)
{
	if (!nr)
	{
		globs->flags &= ~GF_CAP;
		do_char_log(cn, 2, "Removed cap, old setting was at %d\n", CAP);
	}
	else
	{
		globs->flags |= GF_CAP;
		globs->cap = nr;
		do_char_log(cn, 2, "Enabled cap, setting to %d\n", CAP);
	}
}

int cap(int cn, int nr)
{
	int n, place, qsize = 0;

	if (!(globs->flags & GF_CAP))
	{
		return( 0);                                             // no cap if feature is turned off

	}
	if (cn && (ch[cn].flags & (CF_GOD)))
	{
		return( 0);                                             // always allow gods to come in all the time

	}
	if (cn)
	{
		for (n = 1; n<MAXCHARS; n++)                              // find body for possible grave retrieval
		{
			if (!ch[n].used)
			{
				continue;
			}
			if (!(ch[n].flags & CF_BODY))
			{
				continue;
			}

			if (ch[n].data[CHD_CORPSEOWNER]==cn)
			{
				return( 0);                             // always allow corpse retrieval
			}
		}
	}

	player[nr].prio += 10;

	if (cn && (ch[cn].flags & CF_IMP))
	{
		player[nr].prio += 25;
	}
	;                                                               // imps are preferred a lot

	if (cn && (ch[cn].flags & CF_STAFF))
	{
		player[nr].prio += 10;
	}
	;                                                               // staff is preferred

	if (cn && (ch[cn].flags & CF_GOLDEN))
	{
		player[nr].prio += 10;                                  // golden list players are preferred

	}
	if (cn && (ch[cn].flags & CF_BLACK))
	{
		player[nr].prio -= 5;                                   // not blacklisted player get disadvantage

	}
	for (n = place = 1; n<MAXPLAYER; n++)
	{
		if (!player[n].sock)
		{
			continue;
		}
		if (n==nr)
		{
			continue;
		}
		if (player[n].state==ST_NORMAL || player[n].state==ST_EXIT)
		{
			continue;
		}
		if (player[n].prio>=player[nr].prio)
		{
			place++;
		}
		qsize++;
	}

	globs->queuesize = qsize + globs->players_online - CAP;

	if (globs->players_online + place<=CAP)
	{
		return( 0);
	}

	return(place);
}

int soultransform(int cn, int in, int in2, int temp)
{
	god_take_from_char(in, cn);
	god_take_from_char(in2, cn);

	it[in].used  = USE_EMPTY;
	it[in2].used = USE_EMPTY;

	in = god_create_item(temp, 0);
	god_give_char(in, cn);

	return(in);
}

int soulrepair(int cn, int in, int in2)
{
	god_take_from_char(in, cn);
	it[in].used = USE_EMPTY;

	it[in2] = it_temp[it[in2].temp];
	it[in2].carried = cn;
	it[in2].flags |= IF_UPDATE;
	it[in2].temp   = 0;

	return(in2);
}

void souldestroy(int cn, int in)
{
	god_take_from_char(in, cn);
	it[in].used = USE_EMPTY;
}

// Soul catalyst multiplier chart. Yes it's ugly. I'm not sure of a more tact way of doing this.
// Each value in the table is out of 3. A 2/3 for example is 33% less likely to occur, while 4/3 is 33% more likely.
// I'd use just -1/0/+1 but it would take up more visual space and probably be harder to work with than just adjusting.
struct s_soulcat soulcat[N_SOULBONUS] = {
	{ 0,2,1,{4,2,3,3,3,3,3,3,3,3,4,2,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,4,2,3,3}}, // Braveness
	{ 1,2,1,{3,3,4,2,3,3,3,3,3,3,4,2,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,4,3,3,2,3}}, // Willpower
	{ 2,2,1,{3,3,3,3,4,2,3,3,3,3,4,2,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,2,3,3,4}}, // Intuition
	{ 3,2,1,{3,3,3,3,3,3,4,2,3,3,4,2,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,4,2}}, // Agility
	{ 4,2,1,{3,3,3,3,3,3,3,3,4,2,4,2,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,2,3,4,3,3}}, // Strength
	
	{ 5,1,5,{3,3,3,3,3,3,3,3,3,3,3,3,3,3,4,2,3,3,3,3,3,3,4,2,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3}}, // Hitpoints
	{ 6,2,5,{3,3,3,3,3,3,3,3,3,3,3,3,3,3,4,2,3,3,3,3,3,3,4,2,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3}}, // Endurance
	{ 7,1,5,{3,3,3,3,3,3,3,3,3,3,3,3,3,3,4,2,3,3,3,3,3,3,3,3,4,2,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3}}, // Mana
	
	{ 8,3,1,{3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,4,2,3,3,3,3,4,2,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3}}, // Weapon Value
	{ 9,3,1,{3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,4,2,3,3,4,2,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3}}, // Armor Value
	
	{10,4,1,{3,3,3,3,3,3,4,2,4,2,3,3,4,2,3,3,3,3,3,3,4,2,3,3,3,3,3,3,3,3,3,3,4,2,3,3,3,3,3,3,3,3,4,3,3,2}}, // Hand to Hand
	{11,2,1,{4,2,3,3,3,3,4,2,4,2,3,3,4,2,3,3,4,2,3,3,3,3,4,2,3,3,3,3,3,3,3,3,4,2,3,3,3,3,3,3,3,3,3,3,3,3}}, // Precision
	{12,4,1,{3,3,4,2,3,3,4,2,3,3,3,3,4,2,3,3,3,3,3,3,4,2,3,3,3,3,3,3,3,3,3,3,4,2,3,3,3,3,3,3,3,4,3,3,2,3}}, // Dagger
	{13,4,1,{4,2,3,3,3,3,4,2,4,2,3,3,4,2,3,3,3,3,3,3,4,2,3,3,3,3,3,3,3,3,3,3,4,2,3,3,3,3,3,3,4,3,3,2,3,3}}, // Sword
	{14,4,1,{3,3,3,3,3,3,4,2,4,2,3,3,4,2,3,3,3,3,3,3,4,2,3,3,3,3,3,3,3,3,3,3,4,2,3,3,3,3,3,3,2,3,3,4,3,3}}, // Axe
	{15,4,1,{3,3,3,3,4,2,3,3,4,2,3,3,4,2,3,3,3,3,3,3,4,2,3,3,3,3,3,3,3,3,3,3,4,2,3,3,3,3,3,3,3,3,2,3,3,4}}, // Staff
	{16,4,1,{3,3,3,3,3,3,4,2,4,2,3,3,4,2,3,3,3,3,3,3,4,2,3,3,3,3,3,3,3,3,3,3,4,2,3,3,3,3,3,3,3,2,3,3,4,3}}, // Twohander
	{17,2,1,{4,2,3,3,3,3,4,2,3,3,3,3,4,2,3,3,4,2,3,3,3,3,4,2,3,3,3,3,3,3,3,3,3,3,4,2,3,3,3,3,3,3,3,3,4,3}}, // Razor
	{18,1,2,{3,3,3,3,4,2,4,2,3,3,3,3,4,2,3,3,3,3,3,3,3,3,3,3,3,3,4,2,3,3,3,3,4,2,3,3,3,3,3,3,3,3,4,3,3,3}}, // Stealth
	{19,1,2,{3,3,3,3,4,2,4,2,3,3,3,3,4,2,3,3,3,3,3,3,3,3,3,3,3,3,4,2,3,3,3,3,4,2,3,3,3,3,3,3,4,3,3,3,3,3}}, // Perception
	
	{20,1,2,{3,3,4,2,3,3,4,2,4,2,3,3,4,2,3,3,3,3,3,3,3,3,3,3,3,3,4,2,3,3,3,3,4,2,3,3,3,3,3,3,2,3,3,3,3,3}}, // Swimming
	{21,1,1,{4,2,4,2,3,3,3,3,3,3,3,3,4,2,3,3,3,3,4,2,3,3,3,3,4,2,3,3,4,2,3,3,3,3,4,2,3,3,3,3,3,3,3,3,3,4}}, // Magic Shield
	{22,1,2,{4,2,3,3,4,2,3,3,3,3,3,3,4,2,3,3,3,3,3,3,3,3,3,3,3,3,4,2,3,3,3,3,4,2,3,3,3,3,3,3,3,3,3,3,3,3}}, // Bartering
	{23,1,2,{3,3,3,3,4,2,4,2,4,2,3,3,4,2,3,3,3,3,3,3,3,3,4,2,3,3,4,2,3,3,3,3,3,3,4,2,3,3,3,3,3,3,3,3,3,3}}, // Repair
	{24,1,1,{4,2,4,2,4,2,3,3,3,3,3,3,4,2,3,3,3,3,3,3,3,3,3,3,4,2,4,2,4,2,3,3,3,3,4,2,3,3,3,3,3,3,3,3,3,3}}, // Light
	{25,1,1,{4,2,4,2,4,2,3,3,3,3,3,3,4,2,3,3,3,3,4,2,3,3,3,3,4,2,4,2,3,3,3,3,3,3,4,2,3,3,3,3,3,3,3,3,3,3}}, // Recall
	{26,2,1,{4,2,4,2,3,3,3,3,4,2,3,3,4,2,3,3,3,3,3,3,4,2,3,3,3,3,3,3,3,3,3,3,4,2,3,3,3,3,3,3,2,3,3,3,3,3}}, // Shield
	{27,1,1,{4,2,4,2,3,3,3,3,3,3,3,3,4,2,3,3,3,3,4,2,3,3,3,3,4,2,3,3,4,2,3,3,3,3,4,2,3,3,3,3,3,2,3,3,3,3}}, // Protect
	{28,1,1,{4,2,4,2,3,3,3,3,3,3,3,3,4,2,3,3,4,2,3,3,3,3,3,3,4,2,3,3,4,2,3,3,3,3,4,2,3,3,3,3,3,3,3,3,3,2}}, // Enhance
	{29,2,1,{4,2,3,3,4,2,3,3,3,3,3,3,4,2,3,3,3,3,3,3,3,3,3,3,4,2,3,3,3,3,4,2,3,3,4,2,3,3,3,3,3,3,3,2,3,3}}, // Slow
	
	{30,2,1,{4,2,3,3,4,2,3,3,3,3,3,3,4,2,3,3,3,3,3,3,3,3,3,3,4,2,3,3,3,3,4,2,3,3,4,2,3,3,3,3,3,3,3,2,3,3}}, // Curse
	{31,2,1,{4,2,4,2,3,3,3,3,3,3,3,3,4,2,3,3,3,3,3,3,3,3,3,3,4,2,3,3,4,2,3,3,3,3,4,2,3,3,3,3,3,4,3,3,3,3}}, // Bless
	{32,2,1,{4,2,4,2,4,2,3,3,3,3,3,3,4,2,3,3,3,3,3,3,3,3,3,3,4,2,3,3,3,3,3,3,3,3,4,2,3,3,3,3,3,3,3,3,3,3}}, // Identify
	{33,3,1,{4,2,4,2,3,3,3,3,4,2,3,3,4,2,3,3,3,3,4,2,3,3,3,3,4,2,3,3,3,3,3,3,4,2,3,3,3,3,3,3,4,3,3,3,3,3}}, // Resistance
	{34,1,1,{4,2,3,3,4,2,3,3,3,3,3,3,4,2,3,3,4,2,3,3,3,3,3,3,4,2,3,3,3,3,4,2,3,3,4,2,3,3,3,3,3,3,3,3,3,4}}, // Blast
	{35,2,1,{4,2,4,2,4,2,3,3,3,3,3,3,4,2,3,3,3,3,3,3,3,3,3,3,4,2,3,3,3,3,3,3,3,3,4,2,3,3,3,3,3,2,3,3,3,3}}, // Dispel
	{36,1,1,{4,2,3,3,4,2,3,3,4,2,3,3,4,2,3,3,3,3,4,2,3,3,3,3,3,3,4,2,4,2,3,3,3,3,4,2,3,3,3,3,3,3,3,3,2,3}}, // Heal
	{37,2,1,{4,2,4,2,4,2,3,3,3,3,3,3,4,2,3,3,4,2,3,3,3,3,3,3,4,2,3,3,3,3,3,3,3,3,4,2,3,3,3,3,3,4,3,3,3,3}}, // Ghost Comp
	{38,1,2,{3,3,3,3,3,3,3,3,4,2,3,3,4,2,4,2,3,3,3,3,3,3,4,2,3,3,3,3,3,3,3,3,4,2,3,3,3,3,3,3,3,3,3,4,3,3}}, // Regenerate
	{39,1,2,{3,3,3,3,3,3,4,2,3,3,3,3,4,2,4,2,3,3,3,3,3,3,4,2,3,3,3,3,3,3,3,3,4,2,3,3,3,3,3,3,3,3,2,3,3,3}}, // Rest
	
	{40,1,2,{3,3,3,3,4,2,3,3,3,3,3,3,4,2,4,2,3,3,3,3,3,3,3,3,4,2,3,3,3,3,3,3,4,2,3,3,3,3,3,3,2,3,3,3,3,3}}, // Meditate
	{41,1,2,{3,3,4,2,3,3,3,3,4,2,3,3,4,2,3,3,3,3,3,3,3,3,3,3,3,3,4,2,3,3,3,3,4,2,3,3,3,3,3,3,3,3,3,3,3,3}}, // Sense Magic
	{42,3,1,{4,2,3,3,3,3,4,2,4,2,3,3,4,2,3,3,3,3,4,2,3,3,4,2,3,3,3,3,3,3,3,3,4,2,3,3,3,3,3,3,3,3,2,3,3,3}}, // Immunity
	{43,1,1,{4,2,3,3,3,3,4,2,4,2,3,3,4,2,3,3,3,3,3,3,3,3,4,2,3,3,3,3,3,3,3,3,4,2,3,3,3,3,3,3,3,3,3,4,3,3}}, // Surround Hit
	{44,4,1,{3,3,4,2,3,3,3,3,3,3,3,3,4,2,3,3,3,3,3,3,3,3,3,3,4,2,3,3,3,3,3,3,4,2,3,3,3,3,3,3,3,3,3,2,3,3}}, // Concentrate
	{45,2,1,{4,2,3,3,3,3,3,3,4,2,3,3,4,2,3,3,3,3,3,3,3,3,4,2,3,3,3,3,3,3,4,2,3,3,4,2,3,3,3,3,3,3,3,4,3,3}}, // Warcry
	{46,2,1,{4,2,3,3,3,3,4,2,4,2,3,3,4,2,3,3,3,3,3,3,4,2,3,3,3,3,3,3,3,3,3,3,4,2,3,3,3,3,3,3,4,3,3,3,3,3}}, // Dual Wield
	{47,2,1,{4,2,4,2,3,3,4,2,3,3,3,3,4,2,3,3,3,3,4,2,3,3,4,2,3,3,3,3,3,3,4,2,3,3,4,2,3,3,3,3,3,3,3,3,4,3}}, // Blind
	{48,1,1,{4,2,3,3,3,3,4,2,3,3,3,3,4,2,3,3,4,2,3,3,3,3,4,2,3,3,3,3,3,3,3,3,4,2,3,3,3,3,3,3,3,3,3,3,3,2}}, // Weapon Mastery
	{49,1,1,{4,2,3,3,3,3,3,3,4,2,3,3,4,2,3,3,3,3,4,2,3,3,4,2,3,3,3,3,3,3,3,3,4,2,3,3,3,3,3,3,3,3,3,3,2,3}}, // Armor Mastery
	
	{50,1,1,{4,2,3,3,3,3,3,3,4,2,3,3,4,2,3,3,4,2,3,3,3,3,4,2,3,3,3,3,3,3,3,3,3,3,4,2,3,3,3,3,3,2,3,3,3,3}}, // Cleave
	{51,2,1,{4,2,3,3,3,3,4,2,3,3,3,3,4,2,3,3,3,3,4,2,3,3,4,2,3,3,3,3,3,3,4,2,3,3,4,2,3,3,3,3,2,3,3,3,3,3}}, // Weaken
	{52,1,1,{4,2,3,3,4,2,3,3,3,3,3,3,4,2,3,3,4,2,3,3,3,3,3,3,4,2,3,3,3,3,4,2,3,3,4,2,3,3,3,3,3,3,4,3,3,3}}, // Poison
	{53,1,1,{4,2,3,3,4,2,3,3,3,3,3,3,4,2,3,3,4,2,3,3,3,3,3,3,4,2,3,3,4,2,3,3,3,3,4,2,3,3,3,3,3,3,3,3,3,4}}, // Pulse
	{54,1,1,{4,2,3,3,3,3,3,3,3,3,3,3,4,2,3,3,3,3,3,3,3,3,3,3,3,3,4,2,3,3,3,3,4,2,3,3,3,3,3,3,3,3,3,3,2,3}}, // Proximity
	{55,1,1,{4,2,4,2,4,2,3,3,3,3,3,3,4,2,3,3,3,3,3,3,3,3,3,3,4,2,3,3,3,3,3,3,4,2,3,3,3,3,3,3,3,4,3,3,3,3}}, // Comp Mastery
	{56,1,1,{4,2,4,2,3,3,3,3,3,3,3,3,4,2,3,3,3,3,3,3,3,3,3,3,4,2,3,3,4,2,3,3,3,3,4,2,3,3,3,3,3,3,2,3,3,3}}, // Shadow Copy
	{57,2,1,{4,2,4,2,3,3,4,2,3,3,3,3,4,2,3,3,3,3,3,3,3,3,3,3,4,2,3,3,4,2,3,3,3,3,4,2,3,3,3,3,3,3,4,3,3,3}}, // Haste
	{58,1,1,{4,2,3,3,3,3,3,3,4,2,3,3,4,2,3,3,3,3,4,2,3,3,4,2,3,3,3,3,4,2,4,2,3,3,4,2,3,3,3,3,3,3,3,3,3,2}}, // Taunt
	{59,1,1,{4,2,3,3,3,3,4,2,4,2,3,3,4,2,3,3,4,2,3,3,3,3,4,2,3,3,3,3,3,3,3,3,3,3,4,2,3,3,3,3,4,3,3,3,3,3}}, // Blink
	
	{60,6,1,{3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,4,2,3,3,3,3,3,3,4,2,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3}}, // * Move Speed
	{61,6,1,{3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,4,2,3,3,3,3,4,2,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3}}, // * Attack Speed
	{62,6,1,{3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,4,2,3,3,3,3,4,2,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3}}, // * Cast Speed
	{63,4,1,{3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,4,2,3,3,3,3,4,2,3,3,3,3,3,3,3,3,3,3,3,3,3,3}}, // * Spell Mod
	{64,1,2,{3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,4,2,3,3,4,2,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3}}, // * Spell Apt
	{65,1,2,{3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,4,2,3,3,3,3,4,2,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3}}, // * Cooldown Bonus
	{66,4,1,{3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,4,2,3,3,3,3,4,2,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3}}, // * Crit Chance
	{67,1,1,{3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,4,2,3,3,3,3,4,2,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3}}, // * Crit Multi
	{68,2,1,{3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,4,2,3,3,4,2,4,2,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3}}, // * Hit Rate
	{69,2,1,{3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,4,2,4,2,4,2,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3}}, // * Parry Rate
	{70,1,1,{3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,4,2,3,3,3,3,4,2,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3}}, // * Top Damage
	{71,4,1,{3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,4,2,3,3,3,3,3,3,4,2,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3}}  // * Damage Reflect
};

//

#define over_add(a, b) a=(((int)(a)+(int)(b)>120) ? (120) : ((a)+(b)))

static int soul_pen[9+MAXSKILL] = { // Penalties and requirements for each outcome for SS Gear
	1, //  0 Hitpoints
	2, //  1 Endurance
	1, //  2 Mana
	3, //  3 WV or AV
	
	2, //  4 Braveness
	2, //  5 Willpower
	2, //  6 Intuition
	2, //  7 Agility
	2, //  8 Strength
	
	4, //  9 Hand to Hand
	2, // 10 Precision
	4, // 11 Dagger
	4, // 12 Sword
	4, // 13 Axe
	4, // 14 Staff
	4, // 15 Two-Handed
	
	3, // 16 Focus
	1, // 17 Stealth
	1, // 18 Perception
	1, // 19 Swimming
	1, // 20 Magic Shield
	1, // 21 Bartering
	1, // 22 Repair
	1, // 23 Light
	1, // 24 Recall
	2, // 25 Shield
	1, // 26 Protect
	1, // 27 Enhance
	2, // 28 Slow
	2, // 29 Curse
	2, // 30 Bless
	1, // 31 Identify
	3, // 32 Resistance
	2, // 33 Blast
	2, // 34 Dispel
	1, // 35 Heal
	2, // 36 Ghost Companion
	1, // 37 Regenerate
	1, // 38 Rest
	1, // 39 Meditate
	1, // 40 Sense Magic
	3, // 41 Immunity
	1, // 42 Surround Hit
	4, // 43 Concentrate
	2, // 44 Warcry
	2, // 45 Dual Wield
	2, // 46 Combat Mastery
	1, // 47 Weapon Mastery
	1, // 48 Armor Mastery
	2, // 49 Cleave
	2, // 50 Weaken
	2, // 51 Poison
	1, // 52 Damage Proximity
	1, // 53 Hex Proximity
	2, // 54 Companion Mastery
	2, // 55 Shadow Copy
	2, // 56 Haste
	1, // 57 Surround Area
	1  // 58 Surround Rate
};

void soultrans_equipment(int cn, int in, int in2)
{
	int stren, rank, ran, i;
	int known[MAXSKILL] = {0};		// count of total skills in the game
	int cnt = 0, spen = 1;
	int orgt = 0;
	
	// Loop through skills the player knows and add them to a secondary list.
	for (i=0;i<MAXSKILL;i++)			// count of total skills in the game
	{
		if (ch[cn].skill[i][0])
		{
			known[cnt]=i;
			cnt++;
		}
	}
	
	// Set 'rank' to the power of the soul stone
	rank = it[in].data[0];

	while (rank)
	{
		ran = -1;
		stren = RANDOM(rank)+1;
		
		while (ran < 0)
		{
			ran = RANDOM(9+cnt); 		// Pick from known skills and attributes
			
			if (ran<=8)
				spen = soul_pen[ran];
			else
				spen = soul_pen[9+known[ran-8]];
			
			if (spen > stren) 			// If the stat has too high a penalty, try again
				ran = -1;
		}
		
		stren = max(1, min(9, stren/max(1, spen) ) );	// Reduce the strength that will be applied by its penalty cost
		rank -= (stren * spen);							// Reduce remaining rank by the now rounded strength value

		chlog(cn, "SOULSTONE: Rolled %d with stren of %d",ran,stren);

		if (ran==0) // For every 5 points of HP, END, MANA, it requires 20 base points to use.
		{
			it[in2].hp[2] += stren * 20;
			it[in2].hp[0] += stren * 5;
		}
		else if (ran==1)
		{
			//it[in2].end[2] += stren * 20;
			it[in2].end[0] += stren * 5;
		}
		else if (ran==2)
		{
			it[in2].mana[2] += stren * 20;
			it[in2].mana[0] += stren * 5;
		}
		else if (ran==3)
		{
			if (it[in2].weapon[0]>0)
			{
				it[in2].ss_weapon += stren;
			}
			else
			{
				it[in2].ss_armor += stren;
			}
		}
		else if (ran>=4 && ran<=8) // For every 1 attribute point, it requires 3 base points to use.
		{
			i = ran-4;
			over_add(it[in2].attrib[i][2], stren * 3);
			it[in2].attrib[i][0] += stren;
		}
		else if (ran > 8) // For every 1 skill point, it requires 5 base points to use.
		{
			i = known[ran-8];
			over_add(it[in2].skill[i][2], stren * 5);
			it[in2].skill[i][0] += stren;
		}
	}

	orgt = it[in2].temp;
	
	// Repair the item
		it[in2].orig_temp = orgt;

		it[in2].current_damage = 0;
		it[in2].current_age[0] = 0;
		it[in2].current_age[1] = 0;
		it[in2].damage_state = 0;
		
		it[in2].armor[0] = it_temp[orgt].armor[0] + it[in2].ss_armor;
		it[in2].armor[1] = it_temp[orgt].armor[1];
		
		it[in2].weapon[0] = it_temp[orgt].weapon[0] + it[in2].ss_weapon;
		it[in2].weapon[1] = it_temp[orgt].weapon[1];
		
		it[in2].sprite[0] = it_temp[orgt].sprite[0];
		it[in2].sprite[1] = it_temp[orgt].sprite[1];
	//
	
	if (it[in2].temp!=IT_CH_FOOL) 
	{
		it[in2].temp   = 0;
	}
	
	it[in2].flags |= IF_UPDATE | IF_IDENTIFIED | IF_SOULSTONE;
	it[in2].min_rank = max(it[in].data[0], it[in2].min_rank);
	it[in2].value -= 1;
	
	if (!it[in2].max_damage)
	{
		if (it[in2].weapon[0]>0)
		{
			it[in2].max_damage = 2500 * it[in2].weapon[0]/2;
		}
		else if (it[in2].power == 60)
		{
			it[in2].max_damage = 65000;
		}
		else if (it[in2].power == 75)
		{
			it[in2].max_damage = 85000;
		}
		else
		{
			it[in2].max_damage = 60000;
		}
	}
	
	it[in2].power += it[in].data[0] * 5;

	souldestroy(cn, in);

	sprintf(it[in2].description, "A %s enhanced by a rank %d soulstone.", it[in2].name, it[in].data[0]);
}

int can_be_soulstoned(int in)
{
	static int valid_ss[] = {
		// valid weapons
		32, 33, 34, 35, 36, 37, 38,						// Bronze
		284, 285, 286, 287, 288, 289, 290, 291, 292,	// Steel
		523, 524, 525, 526, 527, 528, 529, 530, 531,	// Gold
		532, 533, 534, 535, 536, 537, 538, 539, 540,	// Emerald
		541, 542, 543, 544, 545, 546, 547, 548, 549,	// Crystal
		572, 573, 574, 575, 576, 577, 578, 579, 580,	// Titanium
		693, 694, 695, 696, 697, 698, 699, 700, 701,	// Adamantine
		// valid armors
		27, 28, 29, 30, 31, 39, 40, 41, 42, 43, 51, 52, 53, 54, 55, 	// Cloth, Leather, Bronze
		56, 57, 58, 59, 60,	61, 62, 63, 64, 65, 66, 67, 68, 69, 70,		// Steel, Gold, Emerald
		71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 94, 95, 96, 97, 98, 	// Crystal, Titanium, Adamantine
		337, 338, 339, 340, 341, 342, 343, 344, 345, 346,				// Simple, Caster
		347, 348, 349, 350, 351, 352, 353, 354, 355, 356,				// Adept, Wizard
		// other
		IT_CH_FOOL
		};
	int n, temp;
	
	temp = it[in].temp;

	for (n = 0; n<ARRAYSIZE(valid_ss); n++)
	{
		if (temp == valid_ss[n])
		{
			if (temp==IT_CH_FOOL && (it[in].flags & IF_SOULSTONE))
			{
				return 0;
			}
			return 1;
		}
	}
	return 0;
}

int use_soulstone(int cn, int in)
{
	int in2, rank;

	if (!IS_SANECHAR(cn))
	{
		return( 0);
	}
	if (!IS_SANEITEM(in))
	{
		return( 0);
	}

	if (!(in2 = ch[cn].citem))
	{
		do_char_log(cn, 1, "Try using something with the soulstone. That is, click on the stone with an item under your cursor.\n");
		return(0);
	}

	if (!IS_SANEITEM(in2))
	{
		return( 0);
	}

	if (it[in2].driver==68)
	{
		it[in].data[1] += it[in2].data[1] - RANDOM(it[in2].data[1]/4 + 1);
		rank = points2rank(it[in].data[1]);
		
		if (rank > 18) 
			rank = 18;
		
		it[in].data[0] = rank;
		sprintf(it[in].description, "Level %d soulstone, holding %d exp.", rank, it[in].data[1]);

		if (rank==18)
		{
			do_char_log(cn, 1, "That's as high as they go.\n");
		}

		souldestroy(cn, in2);

		return(1);
	}

	switch(it[in2].temp)
	{
		case 18: // red flower
			in = soultransform(cn, in, in2, 101);
			it[in].hp[0] += 10;
			return( 1);
		case 46: // purple flower
			in = soultransform(cn, in, in2, 102);
			it[in].mana[0] += 10;
			return( 1);
		case 91: // torch
			in = soulrepair(cn, in, in2);
			it[in].max_age[1] *= 4;
			return( 1);
		case 100: // flask
			in = soultransform(cn, in, in2, 102);
			return( 1);
		case 101: // healing potion
			souldestroy(cn, in);
			it[in].hp[0] += 10;
			return( 1);
		case 102: // mana potion
			souldestroy(cn, in);
			it[in].mana[0] += 10;
			return( 1);
		default:
			if (can_be_soulstoned(in2))
			{
				soultrans_equipment(cn, in, in2);
				return( 1);
			}
			else
			{
				do_char_log(cn, 1, "Nothing happened.\n");
				return( 0);
			}
	}
}

int load_mod(void)
{
	int handle, len;
	extern char mod[];

	handle = open("mod.txt", O_RDONLY);
	if (handle!=-1)
	{
		len = read(handle, mod, 130);
		mod[len] = 0;
		close(handle);
	}
	else
	{
		strcpy(mod, "Live long and prosper!");
	}

	return(1);
}

/*
 #define SIZE 50

   void test_filesend(int nr,int size)
   {
        static char buf[16384];
        int n,trans;
        static int sent=0;

        for (n=0; n<sizeof(buf); n++) buf[n]=RANDOM(256);

        sent+=size/3;

        trans=(SIZE*player[nr].rtick)-sent;

        if (trans<5) return;

        trans=min(1024,trans);

        plog(nr,"trans=%d, allow=%d, sent=%d",trans,(SIZE*player[nr].rtick),sent);

        buf[0]=SV_IGNORE;
 *(unsigned int*)(buf+1)=trans;
        xsend(nr,buf,trans);
        sent+=trans;

   }*/

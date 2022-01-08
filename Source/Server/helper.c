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
		return 0;
	}

	m = x + y * MAPX;
	
	if (isbuilding)
	{
		return 1;
	}
	
	if (ismonster)
	{
		if (map[m].flags & (MF_SIGHTBLOCK | MF_NOMONST))
		{
			return 0;
		}
	}
	else
	{
		if (map[m].flags & MF_SIGHTBLOCK)
		{
			return 0;
		}
	}
	if (map[m].it && (it[map[m].it].flags & IF_SIGHTBLOCK))
	{
		return 0;
	}

	return 1;
}

static inline int check_map_go(int x, int y)
{
	int m;

	if (x<=0 || x>=MAPX || y<=0 || y>=MAPY)
	{
		return 0;
	}

	m = x + y * MAPX;

	if (map[m].flags & MF_MOVEBLOCK)
	{
		return 0;
	}

	if (map[m].it && (it[map[m].it].flags & IF_MOVEBLOCK))
	{
		return 0;
	}

	return 1;
}

static inline int close_vis_see(int x, int y, int v)
{
	// 02162020 - updated 20's and 40's for larger client render
	if (!check_map_see(x, y))
	{
		return 0;
	}

	x = x - ox + VISI_SIZE/2;
	y = y - oy + VISI_SIZE/2;

	if (visi[(x + 1) + (y) * VISI_SIZE]==v)
	{
		return 1;
	}
	if (visi[(x - 1) + (y) * VISI_SIZE]==v)
	{
		return 1;
	}
	if (visi[(x) + (y + 1) * VISI_SIZE]==v)
	{
		return 1;
	}
	if (visi[(x) + (y - 1) * VISI_SIZE]==v)
	{
		return 1;
	}
	if (visi[(x + 1) + (y + 1) * VISI_SIZE]==v)
	{
		return 1;
	}
	if (visi[(x + 1) + (y - 1) * VISI_SIZE]==v)
	{
		return 1;
	}
	if (visi[(x - 1) + (y + 1) * VISI_SIZE]==v)
	{
		return 1;
	}
	if (visi[(x - 1) + (y - 1) * VISI_SIZE]==v)
	{
		return 1;
	}

	return 0;
}

static inline int close_vis_go(int x, int y, int v)
{
	// 02162020 - updated 20's and 40's for larger client render
	if (!check_map_go(x, y))
	{
		return 0;
	}

	x = x - ox + VISI_SIZE/2;
	y = y - oy + VISI_SIZE/2;

	if (visi[(x + 1) + (y) * VISI_SIZE]==v)
	{
		return 1;
	}
	if (visi[(x - 1) + (y) * VISI_SIZE]==v)
	{
		return 1;
	}
	if (visi[(x) + (y + 1) * VISI_SIZE]==v)
	{
		return 1;
	}
	if (visi[(x) + (y - 1) * VISI_SIZE]==v)
	{
		return 1;
	}
	if (visi[(x + 1) + (y + 1) * VISI_SIZE]==v)
	{
		return 1;
	}
	if (visi[(x + 1) + (y - 1) * VISI_SIZE]==v)
	{
		return 1;
	}
	if (visi[(x - 1) + (y + 1) * VISI_SIZE]==v)
	{
		return 1;
	}
	if (visi[(x - 1) + (y - 1) * VISI_SIZE]==v)
	{
		return 1;
	}

	return 0;
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
		return 0;
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
			if (IS_MONSTER(cn) && !(ch[cn].flags & (CF_USURP | CF_THRALL)))
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
	if (v<      250)	return  0; // Private
	if (v<     1750)	return  1; // Private FIrst Class
	if (v<     7000)	return  2; // Lance Corporal
	if (v<    21000)	return  3; // Corporal
	if (v<    52500)	return  4; // Sergeant
	if (v<   115500)	return  5; // Staff Sergeant
	if (v<   231000)	return  6; // Master Sergeant
	if (v<   429000)	return  7; // First Sergeant
	if (v<   750750)	return  8; // Sergeant Major
	if (v<  1251250)	return  9; // Second Lieutenant
	if (v<  2002000)	return 10; // First Lieutenant
	if (v<  3094000)	return 11; // Captain
	if (v<  4641000)	return 12; // Major
	if (v<  6783000)	return 13; // Lieutenant Colonel
	if (v<  9690000)	return 14; // Colonel
	if (v< 13566000)	return 15; // Brigadier General
	if (v< 18653250)	return 16; // Major General
	if (v< 25236750)	return 17; // Lieutenant General
	if (v< 33649000)	return 18; // General
	if (v< 44275000)	return 19; // Field Marshal
	if (v< 57557500)	return 20; // Knight
	if (v< 74002500)	return 21; // Baron
	if (v< 94185000)	return 22; // Earl
	if (v<118755000)	return 23; // Marquess
						return 24; // Warlord
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
		return 0;
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
		return 0;                     // Can't do it
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

int get_best_worn(int cn, int v)
{
	int n = -1000, in;
	
	in = ch[cn].worn[v];
	
	if (in)
	{
		switch (v)
		{
			case WN_HEAD:
				if (it[in].attrib[AT_STR][2]>it[in].attrib[AT_INT][2]) 
					n = get_best_armor(cn, 1);
				else 
					n = get_best_armor(cn, 3);
				break;
			case WN_BODY:
				if (it[in].attrib[AT_STR][2]>it[in].attrib[AT_INT][2]) 
					n = get_best_armor(cn, 2);
				else 
					n = get_best_armor(cn, 4);
				break;
			case WN_RHAND:
				if ((it[in].flags & SK_DAGGER) && (it[in].flags & IF_WP_STAFF))
					n = get_best_weapon(cn, 7);
				else if ((it[in].flags & IF_WP_AXE) && (it[in].flags & IF_WP_TWOHAND))
					n = get_best_weapon(cn, 10);
				else if (it[in].flags & IF_WP_DAGGER)
					n = get_best_weapon(cn, SK_DAGGER);
				else if (it[in].flags & IF_WP_SWORD)
					n = get_best_weapon(cn, SK_SWORD);
				else if (it[in].flags & IF_WP_AXE)
					n = get_best_weapon(cn, SK_AXE);
				else if (it[in].flags & IF_WP_STAFF)
					n = get_best_weapon(cn, SK_STAFF);
				else if (it[in].flags & IF_WP_TWOHAND)
					n = get_best_weapon(cn, SK_TWOHAND);
				else 
					n = get_best_weapon(cn, SK_HAND);
				break;
			case WN_LHAND:
				if (it[in].flags & IF_OF_DUALSW)
					n = get_best_weapon(cn, SK_DUAL);
				else 
					n = get_best_weapon(cn, SK_SHIELD);
				break;
			default:
				break;
		}
	}
	
	return n;
}

int get_best_weapon(int cn, int v)
{
	int in = 0, n, m, z;
	static int w_dagger[6] = { 693, 572, 541, 532, 523, 284 };
	static int w_staff[6]  = { 694, 573, 542, 533, 524, 285 };
	static int w_spear[6]  = { 695, 574, 543, 534, 525, 286 };
	static int w_shield[6] = { 696, 575, 544, 535, 526, 287 };
	static int w_sword[6]  = { 697, 576, 545, 536, 527, 288 };
	static int w_dsword[6] = { 698, 577, 546, 537, 528, 289 };
	static int w_axe[6]    = { 699, 578, 547, 538, 529, 290 };
	static int w_twoh[6]   = { 700, 579, 548, 539, 530, 291 };
	static int w_great[6]  = { 701, 580, 549, 540, 531, 292 };
	static int w_claw[6]   = {1784,1783,1782,1781,1780,1779 };
	
	switch (v)
	{
		case SK_HAND:
			for (n=0;n<6;n++)
			{
				z = w_claw[n];
				if (it_temp[z].skill[SK_HAND][2]>B_SK(cn, SK_HAND)) continue;
				for (m=0;m<5;m++)
				{
					if (it_temp[z].attrib[m][2]>B_AT(cn, m)) break;
				}
				if (m==5) { in = z; break; }
			}
			break;
		case SK_DAGGER:
			for (n=0;n<6;n++)
			{
				z = w_dagger[n];
				if (it_temp[z].skill[SK_DAGGER][2]>B_SK(cn, SK_DAGGER)) continue;
				for (m=0;m<5;m++)
				{
					if (it_temp[z].attrib[m][2]>B_AT(cn, m)) break;
				}
				if (m==5) { in = z; break; }
			}
			break;
		case SK_SWORD:
			for (n=0;n<6;n++)
			{
				z = w_sword[n];
				if (it_temp[z].skill[SK_SWORD][2]>B_SK(cn, SK_SWORD)) continue;
				for (m=0;m<5;m++)
				{
					if (it_temp[z].attrib[m][2]>B_AT(cn, m)) break;
				}
				if (m==5) { in = z; break; }
			}
			break;
		case SK_AXE:
			for (n=0;n<6;n++)
			{
				z = w_axe[n];
				if (it_temp[z].skill[SK_AXE][2]>B_SK(cn, SK_AXE)) continue;
				for (m=0;m<5;m++)
				{
					if (it_temp[z].attrib[m][2]>B_AT(cn, m)) break;
				}
				if (m==5) { in = z; break; }
			}
			break;
		case SK_STAFF:
			for (n=0;n<6;n++)
			{
				z = w_staff[n];
				if (it_temp[z].skill[SK_STAFF][2]>B_SK(cn, SK_STAFF)) continue;
				for (m=0;m<5;m++)
				{
					if (it_temp[z].attrib[m][2]>B_AT(cn, m)) break;
				}
				if (m==5) { in = z; break; }
			}
			break;
		case SK_TWOHAND:
			for (n=0;n<6;n++)
			{
				z = w_twoh[n];
				if (it_temp[z].skill[SK_TWOHAND][2]>B_SK(cn, SK_TWOHAND)) continue;
				for (m=0;m<5;m++)
				{
					if (it_temp[z].attrib[m][2]>B_AT(cn, m)) break;
				}
				if (m==5) { in = z; break; }
			}
			break;
		case 7: // Spear
			for (n=0;n<6;n++)
			{
				z = w_spear[n];
				if (it_temp[z].skill[SK_DAGGER][2]>B_SK(cn, SK_DAGGER)) continue;
				if (it_temp[z].skill[SK_STAFF][2]>B_SK(cn, SK_STAFF)) continue;
				for (m=0;m<5;m++)
				{
					if (it_temp[z].attrib[m][2]>B_AT(cn, m)) break;
				}
				if (m==5) { in = z; break; }
			}
			break;
		case 10: // Greataxe
			for (n=0;n<6;n++)
			{
				z = w_great[n];
				if (it_temp[z].skill[SK_AXE][2]>B_SK(cn, SK_AXE)) continue;
				if (it_temp[z].skill[SK_TWOHAND][2]>B_SK(cn, SK_TWOHAND)) continue;
				for (m=0;m<5;m++)
				{
					if (it_temp[z].attrib[m][2]>B_AT(cn, m)) break;
				}
				if (m==5) { in = z; break; }
			}
			break;
		case SK_SHIELD:
			for (n=0;n<6;n++)
			{
				z = w_shield[n];
				if (it_temp[z].skill[SK_SHIELD][2]>B_SK(cn, SK_SHIELD)) continue;
				for (m=0;m<5;m++)
				{
					if (it_temp[z].attrib[m][2]>B_AT(cn, m)) break;
				}
				if (m==5) { in = z; break; }
			}
			break;
		case SK_DUAL:
			for (n=0;n<6;n++)
			{
				z = w_dsword[n];
				if (it_temp[z].skill[SK_DUAL][2]>B_SK(cn, SK_DUAL)) continue;
				for (m=0;m<5;m++)
				{
					if (it_temp[z].attrib[m][2]>B_AT(cn, m)) break;
				}
				if (m==5) { in = z; break; }
			}
			break;
		default: break;
	}
	
	return in;
}

int get_best_armor(int cn, int v)
{
	int in = 0, n, m, z;
	static int a_helm[7]   = { 94, 2028, 76, 71, 66, 61, 56, 51 };
	static int a_body[7]   = { 96, 2030, 78, 73, 68, 63, 58, 53 };
	static int c_helm[4]   = { 352, 347, 342, 337 };
	static int c_body[4]   = { 354, 349, 344, 339 };
	
	switch (v)
	{
		case 1: // Armor Helmet
			for (n=0;n<7;n++)
			{
				z = a_helm[n];
				for (m=0;m<5;m++)
				{
					if (it_temp[z].attrib[m][2]>B_AT(cn, m)) break;
				}
				if (m==5) { in = z; break; }
			}
			break;
		case 2: // Armor Body
			for (n=0;n<7;n++)
			{
				z = a_body[n];
				for (m=0;m<5;m++)
				{
					if (it_temp[z].attrib[m][2]>B_AT(cn, m)) break;
				}
				if (m==5) { in = z; break; }
			}
			break;
		case 3: // Caster Helmet
			for (n=0;n<4;n++)
			{
				z = c_helm[n];
				for (m=0;m<5;m++)
				{
					if (it_temp[z].attrib[m][2]>B_AT(cn, m)) break;
				}
				if (m==5) { in = z; break; }
			}
			break;
		case 4: // Caster Body
			for (n=0;n<4;n++)
			{
				z = c_body[n];
				for (m=0;m<5;m++)
				{
					if (it_temp[z].attrib[m][2]>B_AT(cn, m)) break;
				}
				if (m==5) { in = z; break; }
			}
			break;
		default: break;
	}
	
	return in;
}

int get_casino_potion()
{	
	static int item[] = {
		IT_POT_M_HP, IT_POT_N_HP, IT_POT_G_HP, IT_POT_H_HP, IT_POT_S_HP, IT_POT_C_HP, IT_POT_L_HP, IT_POT_D_HP,
		IT_POT_M_EN, IT_POT_N_EN, IT_POT_G_EN, IT_POT_H_EN, IT_POT_S_EN, IT_POT_C_EN, IT_POT_L_EN, IT_POT_D_EN, 
		IT_POT_M_MP, IT_POT_N_MP, IT_POT_G_MP, IT_POT_H_MP, IT_POT_S_MP, IT_POT_C_MP, IT_POT_L_MP, IT_POT_D_MP,
		IT_POT_VITA, IT_POT_CLAR, IT_POT_SAGE, IT_POT_RAIN, IT_POT_LIFE, IT_POT_DEAD, IT_POT_GOLEM, 
		IT_POT_BRV, IT_POT_WIL, IT_POT_INT, IT_POT_AGL, IT_POT_STR, IT_POT_EXHP, IT_POT_EXEN, IT_POT_EXMP, 
		IT_POT_PRE, IT_POT_EVA, IT_POT_MOB, IT_POT_FRE, IT_POT_MAR, IT_POT_IMM, IT_POT_CLA, IT_POT_THO, 
		IT_POT_BRU, IT_POT_RES, IT_POT_APT, IT_POT_OFF, IT_POT_DEF, IT_POT_PER, IT_POT_STE, 
		BUF_IT_PIGS, BUF_IT_AALE, BUF_IT_DRAG, BUF_IT_MANA, 
		IT_RPOT, IT_GPOT, IT_SPOT
	};
	return item[RANDOM(sizeof( item) / sizeof(int))];
}

int get_casino_scroll()
{
	static int item[] = {
		IT_POP_SPELL, IT_POP_SPELL+1, IT_POP_SPELL+2, IT_POP_SPELL+3, IT_POP_SPELL+4, IT_POP_SPELL+5, IT_POP_SPELL+6, 
		IT_POP_SPELL, IT_POP_SPELL+1, IT_POP_SPELL+2, IT_POP_SPELL+3, IT_POP_SPELL+4, IT_POP_SPELL+5, IT_POP_SPELL+6, 
		IT_POP_SPELL, IT_POP_SPELL+1, IT_POP_SPELL+2, IT_POP_SPELL+3, IT_POP_SPELL+4, IT_POP_SPELL+5, IT_POP_SPELL+6, 
		IT_POP_SPELL, IT_POP_SPELL+1, IT_POP_SPELL+2, IT_POP_SPELL+3, IT_POP_SPELL+4, IT_POP_SPELL+5, IT_POP_SPELL+6, 
		
		IT_POP_ASPEL, IT_POP_ASPEL+1, IT_POP_ASPEL+2, IT_POP_ASPEL+3, IT_POP_ASPEL+4, IT_POP_ASPEL+5, IT_POP_ASPEL+6, 
		IT_POP_ASPEL, IT_POP_ASPEL+1, IT_POP_ASPEL+2, IT_POP_ASPEL+3, IT_POP_ASPEL+4, IT_POP_ASPEL+5, IT_POP_ASPEL+6, 
		IT_POP_ASPEL, IT_POP_ASPEL+1, IT_POP_ASPEL+2, IT_POP_ASPEL+3, IT_POP_ASPEL+4, IT_POP_ASPEL+5, IT_POP_ASPEL+6, 
		
		IT_POP_ISPEL, IT_POP_ISPEL+1, IT_POP_ISPEL+2, IT_POP_ISPEL+3, IT_POP_ISPEL+4, IT_POP_ISPEL+5, IT_POP_ISPEL+6, 
		IT_POP_ISPEL, IT_POP_ISPEL+1, IT_POP_ISPEL+2, IT_POP_ISPEL+3, IT_POP_ISPEL+4, IT_POP_ISPEL+5, IT_POP_ISPEL+6, 
		
		IT_POP_SSPEL, IT_POP_SSPEL+1, IT_POP_SSPEL+2, IT_POP_SSPEL+3, IT_POP_SSPEL+4, IT_POP_SSPEL+5, IT_POP_SSPEL+6, 
		
		IT_EXPS, IT_EXPS, IT_EXPS, IT_EXPS, IT_EXPS, 
		IT_LUKS, IT_LUKS, IT_LUKS, IT_LUKS, IT_LUKS, 
		
		IT_RD_HP, IT_RD_HP, IT_RD_HP, 
		IT_RD_MP, IT_RD_MP, IT_RD_MP, 
		
		IT_RD_BRV, IT_RD_WIL, IT_RD_INT, IT_RD_AGL, IT_RD_STR
	};
	return item[RANDOM(sizeof( item) / sizeof(int))];
}

int get_casino_tarot()
{
	static int item[] = {
		IT_CH_FOOL, IT_CH_MAGI, IT_CH_PREIST, IT_CH_EMPRESS, IT_CH_EMPEROR, 
		IT_CH_HEIROPH, IT_CH_LOVERS, IT_CH_CHARIOT, IT_CH_STRENGTH, IT_CH_HERMIT, 
		IT_CH_WHEEL, IT_CH_JUSTICE, IT_CH_HANGED, IT_CH_DEATH, IT_CH_TEMPER, 
		IT_CH_DEVIL, IT_CH_TOWER, IT_CH_STAR, IT_CH_MOON, IT_CH_SUN, IT_CH_JUDGE, IT_CH_WORLD, 
		IT_CH_FOOL, IT_CH_MAGI, IT_CH_PREIST, IT_CH_EMPRESS, IT_CH_EMPEROR, 
		IT_CH_HEIROPH, IT_CH_LOVERS, IT_CH_CHARIOT, IT_CH_STRENGTH, IT_CH_HERMIT, 
		IT_CH_WHEEL, IT_CH_JUSTICE, IT_CH_HANGED, IT_CH_DEATH, IT_CH_TEMPER, 
		IT_CH_DEVIL, IT_CH_TOWER, IT_CH_STAR, IT_CH_MOON, IT_CH_SUN, IT_CH_JUDGE, IT_CH_WORLD, 
		IT_CH_FOOL, IT_CH_MAGI, IT_CH_PREIST, IT_CH_EMPRESS, IT_CH_EMPEROR, 
		IT_CH_HEIROPH, IT_CH_LOVERS, IT_CH_CHARIOT, IT_CH_STRENGTH, IT_CH_HERMIT, 
		IT_CH_WHEEL, IT_CH_JUSTICE, IT_CH_HANGED, IT_CH_DEATH, IT_CH_TEMPER, 
		IT_CH_DEVIL, IT_CH_TOWER, IT_CH_STAR, IT_CH_MOON, IT_CH_SUN, IT_CH_JUDGE, IT_CH_WORLD, 
		
		IT_CH_FOOL_R, IT_CH_MAGI_R, IT_CH_PREIST_R, IT_CH_EMPRES_R, IT_CH_EMPERO_R, 
		IT_CH_HEIROP_R, IT_CH_LOVERS_R, IT_CH_CHARIO_R, IT_CH_STRENG_R, IT_CH_HERMIT_R, 
		IT_CH_WHEEL_R, IT_CH_JUSTIC_R, IT_CH_HANGED_R, IT_CH_DEATH_R, IT_CH_TEMPER_R, 
		IT_CH_DEVIL_R, IT_CH_TOWER_R, IT_CH_STAR_R, IT_CH_MOON_R, IT_CH_SUN_R, IT_CH_JUDGE_R, IT_CH_WORLD_R
	};
	return item[RANDOM(sizeof( item) / sizeof(int))];
}

int get_casino_ring()
{
	static int item[] = {
		IT_SILV_RING, IT_SILV_RING, IT_SILV_RING, IT_SILV_RING, 
		IT_SILV_RING, IT_SILV_RING, IT_SILV_RING, IT_SILV_RING, 
		1378, 1384, 1390, 1379, 1385, 1391, 1380, 1386, 1392, 
		1381, 1387, 1393, 1382, 1388, 1394, 1383, 1389, 1395, 
		2073, 2074, 2075, 2076, 
		1378, 1384, 1390, 1379, 1385, 1391, 1380, 1386, 1392, 
		1381, 1387, 1393, 1382, 1388, 1394, 1383, 1389, 1395, 
		2073, 2074, 2075, 2076, 
		1378, 1384, 1390, 1379, 1385, 1391, 1380, 1386, 1392, 
		1381, 1387, 1393, 1382, 1388, 1394, 1383, 1389, 1395, 
		2073, 2074, 2075, 2076, 
		1378, 1384, 1390, 1379, 1385, 1391, 1380, 1386, 1392, 
		1381, 1387, 1393, 1382, 1388, 1394, 1383, 1389, 1395, 
		2073, 2074, 2075, 2076, 
		
		IT_GOLD_RING, IT_GOLD_RING, IT_GOLD_RING, 
		IT_GOLD_RING, IT_GOLD_RING, IT_GOLD_RING, 
		1396, 1402, 1408, 1397, 1403, 1409, 1398, 1404, 1410, 
		1399, 1405, 1411, 1400, 1406, 1412, 1401, 1407, 1413, 
		2077, 2081, 2078, 2082, 2079, 2083, 2080, 2084, 
		2085, 2086, 
		1396, 1402, 1408, 1397, 1403, 1409, 1398, 1404, 1410, 
		1399, 1405, 1411, 1400, 1406, 1412, 1401, 1407, 1413, 
		2077, 2081, 2078, 2082, 2079, 2083, 2080, 2084, 
		2085, 2086, 
		1396, 1402, 1408, 1397, 1403, 1409, 1398, 1404, 1410, 
		1399, 1405, 1411, 1400, 1406, 1412, 1401, 1407, 1413, 
		2077, 2081, 2078, 2082, 2079, 2083, 2080, 2084, 
		2085, 2086, 
		
		IT_PLAT_RING, IT_PLAT_RING, 
		IT_PLAT_RING, IT_PLAT_RING, 
		1414, 1420, 1426, 1415, 1421, 1427, 1416, 1422, 1428, 
		1417, 1423, 1429, 1418, 1424, 1430, 1419, 1425, 1431, 
		2087, 2091, 2097, 2088, 2092, 2098, 2089, 2093, 2099, 
		2090, 2094, 2100, 2095, 2101, 2102, 
		1414, 1420, 1426, 1415, 1421, 1427, 1416, 1422, 1428, 
		1417, 1423, 1429, 1418, 1424, 1430, 1419, 1425, 1431, 
		2087, 2091, 2097, 2088, 2092, 2098, 2089, 2093, 2099, 
		2090, 2094, 2100, 2095, 2101, 2102, 
		
		1272, 1272, 1272, 1272, 
		1273, 1273, 1273, 1273, 
		2399, 2399, 2399, 
		2381, 2381, 2381, 
		2103, 2103, 
		
		IT_TW_PROPHET, IT_TW_SINBIND
	};
	return item[RANDOM(sizeof( item) / sizeof(int))];
}

// Randomize certain prizes from the casino
int change_casino_shop_item(int in)
{
	if (!IS_SANEITEM(in))
		return 0;
	
	switch (in)
	{
		case IT_CASI_POTI:		// Random Potion
			in = get_casino_potion();
			break;
		
		case IT_CASI_SCRO:		// Random Scroll
			in = get_casino_scroll();
			break;
			
		case IT_CASI_TARO:		// Random Tarot
			in = get_casino_tarot();
			break;
			
		case IT_CASI_RING:		// Random Ring
			in = get_casino_ring();
			break;
		
		default: break;
	}
	
	return in;
}

int change_bs_shop_item(int cn, int in)
{
	int t;
	
	if (!IS_SANEITEM(in))
		return 0;
	
	switch (in)
	{
		// Steel Greataxe, change to appropriate 1st skill item
		case IT_GAXE_STEL:
			if (IS_ANY_TEMP(cn))
				in = get_best_weapon(cn, 10); // 1. Greataxe
			else if (IS_ANY_MERC(cn) || IS_BRAVER(cn))
				in = get_best_weapon(cn, SK_SWORD); // 1. Sword
			else if (IS_ANY_HARA(cn))
				in = get_best_weapon(cn, 7); // 1. Spear
			else
				in = 0;
			break;
		// Steel twohander, change to appropriate 2nd skill item
		case IT_THSW_STEL:
			if (IS_ANY_TEMP(cn))
				in = get_best_weapon(cn, SK_TWOHAND); // 2. Twohander
			else if (IS_ANY_MERC(cn))
				in = get_best_weapon(cn, SK_SHIELD); // 2. Shield
			else if (IS_ANY_HARA(cn))
				in = get_best_weapon(cn, SK_STAFF); // 2. Staff
			else if (IS_BRAVER(cn))
				in = get_best_weapon(cn, SK_DUAL); // 3. Dual Sword
			else
				in = 0;
			break;
		// Steel axe, change to appropriate 3rd skill item
		case IT_AXXE_STEL:
			if (IS_ANY_TEMP(cn) || IS_WARRIOR(cn))
				in = get_best_weapon(cn, SK_AXE); // 3. Axe
			else if (IS_ANY_HARA(cn) || IS_MERCENARY(cn) || IS_SORCERER(cn))
				in = get_best_weapon(cn, SK_DAGGER); // 3. Dagger
			else if (IS_BRAVER(cn))
				in = get_best_weapon(cn, SK_HAND); // 3. Claw
			else
				in = 0;
			break;
		// Steel claw, change to appropriate 4th skill item
		case IT_SHIE_STEL:
			if (IS_ANY_TEMP(cn))
				in = get_best_weapon(cn, SK_SHIELD); // 4. Shield
			else if (IS_WARRIOR(cn))
				in = get_best_weapon(cn, SK_DUAL); // 4. Dual Sword
			else if (IS_ANY_HARA(cn) || IS_MERCENARY(cn) || IS_SORCERER(cn))
				in = get_best_weapon(cn, SK_HAND); // 4. Claw
			else
				in = 0;
			break;
		// Steel sword, change to appropriate 5th skill item
		case IT_SWOR_STEL:
			if (IS_ANY_TEMP(cn))
				in = get_best_weapon(cn, SK_HAND); // 5. Claw
			else if (IS_WARRIOR(cn))
				in = get_best_weapon(cn, SK_DAGGER); // 5. Dagger
			else
				in = 0;
			break;
		// Steel sword, change to appropriate 6th skill item
		case IT_DUAL_STEL:
			if (IS_WARRIOR(cn))
				in = get_best_weapon(cn, SK_HAND); // 6. Claw
			else
				in = 0;
			break;
		// Bronze helm, change to best melee helm
		case IT_HELM_BRNZ:
			in = get_best_armor(cn, 1);
			break;
		// Bronze armor, change to best melee armor
		case IT_BODY_BRNZ:
			in = get_best_armor(cn, 2);
			break;
		// Caster cap, change to best spell cap
		case IT_HELM_CAST:
			in = get_best_armor(cn, 3);
			break;
		// Caster robe, change to best spell robe
		case IT_BODY_CAST:
			in = get_best_armor(cn, 4);
			break;
		
		// Osiris items...
		case IT_WP_LIFESPRIG:
			t = time(NULL) / (60*60*24);
			in += t % 20;
			break;
		case IT_WP_VOLCANF:
			t = time(NULL) / (60*60*24);
			in -= t % 20;
			break;
		
		default: break;
	}
	
	return in;
}

int get_special_item(int in, int gen_a, int gen_b, int gen_c)
{
	int in2;
	switch(in)
	{
		// Create an enchanted weapon
		case IT_GAXE_STEL:	case IT_THSW_STEL:	case IT_AXXE_STEL:	case IT_SHIE_STEL:	case IT_SWOR_STEL:
		case IT_DUAL_STEL:	case IT_DAGG_STEL:	case IT_STAF_STEL:	case IT_SPEA_STEL:	case IT_CLAW_STEL:
		case IT_GAXE_GOLD:	case IT_THSW_GOLD:	case IT_AXXE_GOLD:	case IT_SHIE_GOLD:	case IT_SWOR_GOLD:
		case IT_DUAL_GOLD:	case IT_DAGG_GOLD:	case IT_STAF_GOLD:	case IT_SPEA_GOLD:	case IT_CLAW_GOLD:
		case IT_GAXE_EMER:	case IT_THSW_EMER:	case IT_AXXE_EMER:	case IT_SHIE_EMER:	case IT_SWOR_EMER:
		case IT_DUAL_EMER:	case IT_DAGG_EMER:	case IT_STAF_EMER:	case IT_SPEA_EMER:	case IT_CLAW_EMER:
		case IT_GAXE_CRYS:	case IT_THSW_CRYS:	case IT_AXXE_CRYS:	case IT_SHIE_CRYS:	case IT_SWOR_CRYS:
		case IT_DUAL_CRYS:	case IT_DAGG_CRYS:	case IT_STAF_CRYS:	case IT_SPEA_CRYS:	case IT_CLAW_CRYS:
		case IT_GAXE_TITN:	case IT_THSW_TITN:	case IT_AXXE_TITN:	case IT_SHIE_TITN:	case IT_SWOR_TITN:
		case IT_DUAL_TITN:	case IT_DAGG_TITN:	case IT_STAF_TITN:	case IT_SPEA_TITN:	case IT_CLAW_TITN:
		case IT_GAXE_ADAM:	case IT_THSW_ADAM:	case IT_AXXE_ADAM:	case IT_SHIE_ADAM:	case IT_SWOR_ADAM:
		case IT_DUAL_ADAM:	case IT_DAGG_ADAM:	case IT_STAF_ADAM:	case IT_SPEA_ADAM:	case IT_CLAW_ADAM:
		case IT_GAXE_DAMA:	case IT_THSW_DAMA:	case IT_AXXE_DAMA:	case IT_SHIE_DAMA:	case IT_SWOR_DAMA:
		case IT_DUAL_DAMA:	case IT_DAGG_DAMA:	case IT_STAF_DAMA:	case IT_SPEA_DAMA:	case IT_CLAW_DAMA:
		// Create an enchanted armor piece
		case IT_HELM_BRNZ:	case IT_BODY_BRNZ:	case IT_HELM_STEL:	case IT_BODY_STEL:
		case IT_HELM_GOLD:	case IT_BODY_GOLD:	case IT_HELM_EMER:	case IT_BODY_EMER:
		case IT_HELM_CRYS:	case IT_BODY_CRYS:	case IT_HELM_TITN:	case IT_BODY_TITN:
		case IT_HELM_ADAM:	case IT_BODY_ADAM:	case IT_HELM_CAST:	case IT_BODY_CAST:
		case IT_HELM_ADEP:	case IT_BODY_ADEP:	case IT_HELM_WIZR:	case IT_BODY_WIZR:
		case IT_HELM_DAMA:	case IT_BODY_DAMA:
			in2 = create_special_item(in, gen_a, gen_b, gen_c);
			break;

		default:
			in2 = god_create_item(in);
			if (it[in2].temp == IT_EXPS)
			{
				it[in2].data[0] = RANDOM(25) * 2000;
			}
			else if (it[in2].temp == IT_LUKS)
			{
				it[in2].data[0] = RANDOM(25) * 200;
			}
			break;
	}
	return in2;
}

int get_special_spr(int temp, int spr)
{
	//xlog("get_special_spr IN:  %d  %d", temp, spr);
	
	// Return the sprite that we want for the magic/rare item
	switch(temp)
	{
		case IT_DAGG_STEL: spr = 4950; break;
		case IT_STAF_STEL: spr = 4951; break;
		case IT_SPEA_STEL: spr = 4952; break;
		case IT_SHIE_STEL: spr = 4953; break;
		case IT_SWOR_STEL: spr = 4954; break;
		case IT_DUAL_STEL: spr = 4955; break;
		case IT_AXXE_STEL: spr = 4956; break;
		case IT_THSW_STEL: spr = 4957; break;
		case IT_GAXE_STEL: spr = 4958; break;
		case IT_DAGG_GOLD: spr = 4959; break;
		case IT_STAF_GOLD: spr = 4960; break;
		case IT_SPEA_GOLD: spr = 4961; break;
		case IT_SHIE_GOLD: spr = 4962; break;
		case IT_SWOR_GOLD: spr = 4963; break;
		case IT_DUAL_GOLD: spr = 4964; break;
		case IT_AXXE_GOLD: spr = 4965; break;
		case IT_THSW_GOLD: spr = 4966; break;
		case IT_GAXE_GOLD: spr = 4967; break;
		case IT_DAGG_EMER: spr = 4968; break;
		case IT_STAF_EMER: spr = 4969; break;
		case IT_SPEA_EMER: spr = 4970; break;
		case IT_SHIE_EMER: spr = 4971; break;
		case IT_SWOR_EMER: spr = 4972; break;
		case IT_DUAL_EMER: spr = 4973; break;
		case IT_AXXE_EMER: spr = 4974; break;
		case IT_THSW_EMER: spr = 4975; break;
		case IT_GAXE_EMER: spr = 4976; break;
		case IT_DAGG_CRYS: spr = 4977; break;
		case IT_STAF_CRYS: spr = 4978; break;
		case IT_SPEA_CRYS: spr = 4979; break;
		case IT_SHIE_CRYS: spr = 4980; break;
		case IT_SWOR_CRYS: spr = 4981; break;
		case IT_DUAL_CRYS: spr = 4982; break;
		case IT_AXXE_CRYS: spr = 4983; break;
		case IT_THSW_CRYS: spr = 4984; break;
		case IT_GAXE_CRYS: spr = 4985; break;
		case IT_DAGG_TITN: spr = 4986; break;
		case IT_STAF_TITN: spr = 4987; break;
		case IT_SPEA_TITN: spr = 4988; break;
		case IT_SHIE_TITN: spr = 4989; break;
		case IT_SWOR_TITN: spr = 4990; break;
		case IT_DUAL_TITN: spr = 4991; break;
		case IT_AXXE_TITN: spr = 4992; break;
		case IT_THSW_TITN: spr = 4993; break;
		case IT_GAXE_TITN: spr = 4994; break;
		case IT_DAGG_ADAM: spr = 4995; break;
		case IT_STAF_ADAM: spr = 4996; break;
		case IT_SPEA_ADAM: spr = 4997; break;
		case IT_SHIE_ADAM: spr = 4998; break;
		case IT_SWOR_ADAM: spr = 4999; break;
		case IT_DUAL_ADAM: spr = 5000; break;
		case IT_AXXE_ADAM: spr = 5001; break;
		case IT_THSW_ADAM: spr = 5002; break;
		case IT_GAXE_ADAM: spr = 5003; break;
		case IT_CLAW_STEL: spr = 5004; break;
		case IT_CLAW_GOLD: spr = 5005; break;
		case IT_CLAW_EMER: spr = 5006; break;
		case IT_CLAW_CRYS: spr = 5007; break;
		case IT_CLAW_TITN: spr = 5008; break;
		case IT_CLAW_ADAM: spr = 5009; break;
		case IT_DAGG_DAMA: spr = 5010; break;
		case IT_STAF_DAMA: spr = 5011; break;
		case IT_SPEA_DAMA: spr = 5012; break;
		case IT_SHIE_DAMA: spr = 5013; break;
		case IT_SWOR_DAMA: spr = 5014; break;
		case IT_DUAL_DAMA: spr = 5015; break;
		case IT_AXXE_DAMA: spr = 5016; break;
		case IT_THSW_DAMA: spr = 5017; break;
		case IT_GAXE_DAMA: spr = 5018; break;
		case IT_CLAW_DAMA: spr = 5019; break;
		//
		case IT_HELM_BRNZ: spr =  180; break;
		case IT_BODY_BRNZ: spr =  181; break;
		case IT_HELM_STEL: spr =  182; break;
		case IT_BODY_STEL: spr =  183; break;
		case IT_HELM_GOLD: spr =  184; break;
		case IT_BODY_GOLD: spr =  185; break;
		case IT_HELM_EMER: spr =  186; break;
		case IT_BODY_EMER: spr =  187; break;
		case IT_HELM_CRYS: spr =  188; break;
		case IT_BODY_CRYS: spr =  189; break;
		case IT_HELM_TITN: spr =  190; break;
		case IT_BODY_TITN: spr =  191; break;
		case IT_HELM_ADAM: spr =  192; break;
		case IT_BODY_ADAM: spr =  193; break;
		case IT_HELM_CAST: spr =  194; break;
		case IT_BODY_CAST: spr =  195; break;
		case IT_HELM_ADEP: spr =  196; break;
		case IT_BODY_ADEP: spr =  197; break;
		case IT_HELM_WIZR: spr =  198; break;
		case IT_BODY_WIZR: spr =  199; break;
		case IT_HELM_DAMA: spr = 3733; break;
		case IT_BODY_DAMA: spr = 3734; break;
		//
		default: break;
	}
	
	//xlog("get_special_spr OUT: %d", spr);
	
	return spr;
}

int create_special_item(int temp, int gen_a, int gen_b, int gen_c)
{
	// roll is number of prefixes, *3/2
	int in, mul = 1, spr, roll=24, legendary = 0, n, m, prev=-1;
	char *pref, *suffix, name[60], newname[80];
	int is_armor=0, is_robe=0, is_dama=0, is_weap=0, is_mage=0;
	int is_gaxe=0, is_thsw=0, is_axxe=0, is_shie=0, is_swor=0;
	int is_dual=0, is_dagg=0, is_staf=0, is_spea=0, is_claw=0;

	in = god_create_item(temp);
	if (!in)
	{
		return 0;
	}
	
	it[in].temp = 0;
	
	// Go through each check for what kind of item this is
	if (temp==IT_HELM_BRNZ || temp==IT_BODY_BRNZ || temp==IT_HELM_STEL || temp==IT_BODY_STEL || 
		temp==IT_HELM_GOLD || temp==IT_BODY_GOLD || temp==IT_HELM_EMER || temp==IT_BODY_EMER || 
		temp==IT_HELM_CRYS || temp==IT_BODY_CRYS || temp==IT_HELM_TITN || temp==IT_BODY_TITN || 
		temp==IT_HELM_ADAM || temp==IT_BODY_ADAM)
		is_armor = 1;
	if (temp==IT_HELM_CAST || temp==IT_BODY_CAST || temp==IT_HELM_ADEP || temp==IT_BODY_ADEP || 
		temp==IT_HELM_WIZR || temp==IT_BODY_WIZR)
		is_robe = 1;
	if (temp==IT_HELM_DAMA || temp==IT_BODY_DAMA)
		is_dama = 1;
	//
	if (temp==IT_GAXE_STEL || temp==IT_GAXE_GOLD || temp==IT_GAXE_EMER || temp==IT_GAXE_CRYS || 
		temp==IT_GAXE_TITN || temp==IT_GAXE_ADAM || temp==IT_GAXE_DAMA)
		is_gaxe = 1;
	if (temp==IT_THSW_STEL || temp==IT_THSW_GOLD || temp==IT_THSW_EMER || temp==IT_THSW_CRYS || 
		temp==IT_THSW_TITN || temp==IT_THSW_ADAM || temp==IT_THSW_DAMA)
		is_thsw = 1;
	if (temp==IT_AXXE_STEL || temp==IT_AXXE_GOLD || temp==IT_AXXE_EMER || temp==IT_AXXE_CRYS || 
		temp==IT_AXXE_TITN || temp==IT_AXXE_ADAM || temp==IT_AXXE_DAMA)
		is_axxe = 1;
	if (temp==IT_SHIE_STEL || temp==IT_SHIE_GOLD || temp==IT_SHIE_EMER || temp==IT_SHIE_CRYS || 
		temp==IT_SHIE_TITN || temp==IT_SHIE_ADAM || temp==IT_SHIE_DAMA)
		is_shie = 1;
	if (temp==IT_SWOR_STEL || temp==IT_SWOR_GOLD || temp==IT_SWOR_EMER || temp==IT_SWOR_CRYS || 
		temp==IT_SWOR_TITN || temp==IT_SWOR_ADAM || temp==IT_SWOR_DAMA)
		is_swor = 1;
	if (temp==IT_DUAL_STEL || temp==IT_DUAL_GOLD || temp==IT_DUAL_EMER || temp==IT_DUAL_CRYS || 
		temp==IT_DUAL_TITN || temp==IT_DUAL_ADAM || temp==IT_DUAL_DAMA)
		is_dual = 1;
	if (temp==IT_DAGG_STEL || temp==IT_DAGG_GOLD || temp==IT_DAGG_EMER || temp==IT_DAGG_CRYS || 
		temp==IT_DAGG_TITN || temp==IT_DAGG_ADAM || temp==IT_DAGG_DAMA)
		is_dagg = 1;
	if (temp==IT_STAF_STEL || temp==IT_STAF_GOLD || temp==IT_STAF_EMER || temp==IT_STAF_CRYS || 
		temp==IT_STAF_TITN || temp==IT_STAF_ADAM || temp==IT_STAF_DAMA)
		is_staf = 1;
	if (temp==IT_SPEA_STEL || temp==IT_SPEA_GOLD || temp==IT_SPEA_EMER || temp==IT_SPEA_CRYS || 
		temp==IT_SPEA_TITN || temp==IT_SPEA_ADAM || temp==IT_SPEA_DAMA)
		is_spea = 1;
	if (temp==IT_CLAW_STEL || temp==IT_CLAW_GOLD || temp==IT_CLAW_EMER || temp==IT_CLAW_CRYS || 
		temp==IT_CLAW_TITN || temp==IT_CLAW_ADAM || temp==IT_CLAW_DAMA)
		is_claw = 1;
	//
	if (is_gaxe || is_thsw || is_axxe || is_swor || is_dual || is_claw)
		is_weap = 1;
	if (is_shie || is_dagg || is_staf || is_spea)
		is_mage = 1;
	//
	
	if (it[in].placement & PL_TWOHAND)
	{
		mul = 2;
	}
	
	// Special 'Legendary' affixes
	if (!RANDOM(12) || gen_c)
	{
		legendary = 1;
		roll = 16;
	}
	
	roll = RANDOM(roll);
	
	if (gen_a) roll = gen_a;
	
	// Pick a prefix
	switch(roll)
	{
		case  0: 
		case  1: 
			if (legendary) pref = "Leo "; else pref = "Lion's ";
			it[in].attrib[AT_BRV][0] += 4 * mul; 
			break;
		case  2: 
		case  3: 
			if (legendary) pref = "Anguis "; else pref = "Snake's "; 
			it[in].attrib[AT_WIL][0] += 4 * mul; 
			break;
		case  4: 
		case  5: 
			if (legendary) pref = "Ibis "; else pref = "Owl's "; 
			it[in].attrib[AT_INT][0] += 4 * mul; 
			break;
		case  6: 
		case  7: 
			if (legendary) pref = "Mus "; else pref = "Weasel's "; 
			it[in].attrib[AT_AGL][0] += 4 * mul; 
			break;
		case  8: 
		case  9: 
			if (legendary) pref = "Ursa "; else pref = "Bear's "; 
			it[in].attrib[AT_STR][0] += 4 * mul; 
			break;
		//
		case 10: 
		case 11: 
			if (legendary) pref = "Angelus "; else pref = "Angelic "; 
			it[in].attrib[AT_BRV][0] += 2 * mul; 
			it[in].attrib[AT_WIL][0] += 2 * mul; 
			it[in].attrib[AT_INT][0] += 2 * mul; 
			it[in].attrib[AT_AGL][0] += 2 * mul; 
			it[in].attrib[AT_STR][0] += 2 * mul; 
			break;
		case 12: 
		case 13: 
			if (legendary) pref = "Lux "; else pref = "Glowing "; 
			it[in].light[0] += 20 * mul;
			it[in].spell_apt[0] += 4 * mul;
			break;
		case 14: 
			if (legendary) pref = "Centurio "; else pref = "Centurion's "; 
			it[in].to_hit[0] += 2 * mul;
			it[in].to_parry[0] += 2 * mul;
			break;
		case 15: 
			if (legendary) pref = "Deus "; else pref = "Godly "; 
			mul *= 2; 
			break;
		default: 
			pref = ""; 
			break;
	}
	
	roll=RANDOM(32);

	// Pick a suffix, or two if legendary
	for (n=0;n<=legendary;n++)
	{
		while (roll==prev) // avoid picking the same thing twice if legendary
		{
			roll=RANDOM(32);
		}
		m=RANDOM(2);
		
		if (n==0 && gen_b) roll = gen_b;
		if (n==1 && gen_c) roll = gen_c;
		
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
			//
			case 11:
				if (legendary) suffix = "Vitale"; else suffix = " of Vitality";
				it[in].hp[0] += 40 * mul;
				it[in].skill[SK_REGEN][0] += 4 * mul;
				break;
			case 12:
				if (legendary) suffix = "Vigor"; else suffix = " of Vigor";
				it[in].end[0] += 20 * mul;
				it[in].skill[SK_REST][0] += 4 * mul;
				break;
			case 13:
				if (legendary) suffix = "Spiritus"; else suffix = " of Spirits";
				it[in].mana[0] += 40 * mul;
				it[in].skill[SK_MEDIT][0] += 4 * mul;
				break;
			case 14:
				if (legendary) suffix = "Impetus"; else suffix = " of Offense";
				it[in].weapon[0] += 4 * mul;
				break;
			case 15:
				if (legendary) suffix = "Defendere"; else suffix = " of Defense";
				it[in].armor[0] += 4 * mul;
				break;
			case 16:
				if (legendary) suffix = "Perfugium"; else suffix = " of Harbor";
				it[in].skill[SK_IMMUN][0] += 3 * mul;
				it[in].skill[SK_RESIST][0] += 3 * mul;
				break;
			//
			case 17:
				if (is_robe || is_spea || is_dagg || (is_dama && m))
				{
					if (legendary) suffix = "Occultus"; else suffix = " of Occult";
					it[in].skill[SK_DAGGER][0] += 2 * mul;
					it[in].skill[SK_SLOW][0] += 3 * mul;
				}
				else if (is_thsw)
				{
					if (legendary) suffix = "Concursus"; else suffix = " of Melee";
					it[in].skill[SK_TWOHAND][0] += 2 * mul;
					it[in].weapon[0] += 2 * mul;
				}
				else if (is_gaxe || is_axxe)
				{
					if (legendary) suffix = "Tumultus"; else suffix = " of Uproar";
					it[in].skill[SK_AXE][0] += 2 * mul;
					it[in].skill[SK_TAUNT][0] += 3 * mul;
				}
				else if (is_swor)
				{
					if (legendary) suffix = "Deceptio"; else suffix = " of Deception";
					it[in].skill[SK_SWORD][0] += 2 * mul;
					it[in].skill[SK_POISON][0] += 3 * mul;
				}
				else if (is_staf)
				{
					if (legendary) suffix = "Expello"; else suffix = " of Banish";
					it[in].skill[SK_STAFF][0] += 2 * mul;
					it[in].skill[SK_SLOW][0] += 3 * mul;
				}
				else if (is_shie)
				{
					if (legendary) suffix = "Talio"; else suffix = " of Retaliation";
					it[in].skill[SK_SHIELD][0] += 3 * mul;
					it[in].gethit_dam[0] += 2 * mul;
				}
				else if (is_dual)
				{
					if (legendary) suffix = "Talio"; else suffix = " of Retaliation";
					it[in].skill[SK_DUAL][0] += 3 * mul;
					it[in].gethit_dam[0] += 2 * mul;
				}
				else	// is_claw
				{
					if (legendary) suffix = "Rixa"; else suffix = " of Brawling";
					it[in].skill[SK_HAND][0] += 2 * mul;
					it[in].skill[SK_WEAKEN][0] += 3 * mul;
				}
				break;
			case 18:
				if (is_robe || is_spea || is_dagg || (is_dama && m))
				{
					if (legendary) suffix = "Cantatrix"; else suffix = " of Hexing";
					it[in].skill[SK_DAGGER][0] += 2 * mul;
					it[in].skill[SK_CURSE][0] += 3 * mul;
				}
				else if (is_thsw)
				{
					if (legendary) suffix = "Vindex"; else suffix = " of Champions";
					it[in].skill[SK_TWOHAND][0] += 2 * mul;
					it[in].base_crit += 1 * mul;
				}
				else if (is_gaxe || is_axxe)
				{
					if (legendary) suffix = "Feritas"; else suffix = " of Brutality";
					it[in].skill[SK_AXE][0] += 2 * mul;
					it[in].skill[SK_CLEAVE][0] += 3 * mul;
				}
				else if (is_swor)
				{
					if (legendary) suffix = "Callidus"; else suffix = " of Cunning";
					it[in].skill[SK_SWORD][0] += 2 * mul;
					it[in].skill[SK_BLIND][0] += 3 * mul;
				}
				else if (is_staf)
				{
					if (legendary) suffix = "Reconditus"; else suffix = " of Arcana";
					it[in].skill[SK_STAFF][0] += 2 * mul;
					it[in].skill[SK_CURSE][0] += 3 * mul;
				}
				else if (is_shie)
				{
					if (legendary) suffix = "Scutum"; else suffix = " of Shielding";
					it[in].skill[SK_SHIELD][0] += 3 * mul;
					it[in].armor[0] += 2 * mul;
				}
				else if (is_dual)
				{
					if (legendary) suffix = "Tracto"; else suffix = " of Wielding";
					it[in].skill[SK_DUAL][0] += 3 * mul;
					it[in].weapon[0] += 2 * mul;
				}
				else	// is_claw
				{
					if (legendary) suffix = "Inermis"; else suffix = " of Quarrel";
					it[in].skill[SK_HAND][0] += 2 * mul;
					it[in].skill[SK_TAUNT][0] += 3 * mul;
				}
				break;
			case 19:
				if (is_robe || is_spea || is_staf || (is_dama && m))
				{
					if (legendary) suffix = "Expello"; else suffix = " of Banish";
					it[in].skill[SK_STAFF][0] += 2 * mul;
					it[in].skill[SK_SLOW][0] += 3 * mul;
				}
				else if (is_claw)
				{
					if (legendary) suffix = "Rixa"; else suffix = " of Brawling";
					it[in].skill[SK_HAND][0] += 2 * mul;
					it[in].skill[SK_WEAKEN][0] += 3 * mul;
				}
				else if (is_axxe)
				{
					if (legendary) suffix = "Concursus"; else suffix = " of Melee";
					it[in].skill[SK_AXE][0] += 2 * mul;
					it[in].weapon[0] += 2 * mul;
				}
				else if (is_swor)
				{
					if (legendary) suffix = "Munitione"; else suffix = " of Fortification";
					it[in].skill[SK_SWORD][0] += 2 * mul;
					it[in].armor[0] += 2 * mul;
				}
				else if (is_dagg)
				{
					if (legendary) suffix = "Occultus"; else suffix = " of Occult";
					it[in].skill[SK_DAGGER][0] += 2 * mul;
					it[in].skill[SK_SLOW][0] += 3 * mul;
				}
				else if (is_shie)
				{
					if (legendary) suffix = "Orandi"; else suffix = " of Prayer";
					it[in].skill[SK_SHIELD][0] += 3 * mul;
					it[in].skill[SK_RESIST][0] += 3 * mul;
				}
				else if (is_dual)
				{
					if (legendary) suffix = "Armatus"; else suffix = " of Arming";
					it[in].skill[SK_DUAL][0] += 3 * mul;
					it[in].skill[SK_SURROUND][0] += 3 * mul;
				}
				else	// is_thsw
				{
					if (legendary) suffix = "Inhaere"; else suffix = " of Sparring";
					it[in].skill[SK_TWOHAND][0] += 2 * mul;
					it[in].skill[SK_CLEAVE][0] += 3 * mul;
				}
				break;
			case 20:
				if (is_robe || is_spea || is_staf || (is_dama && m))
				{
					if (legendary) suffix = "Reconditus"; else suffix = " of Arcana";
					it[in].skill[SK_STAFF][0] += 2 * mul;
					it[in].skill[SK_CURSE][0] += 3 * mul;
				}
				else if (is_claw)
				{
					if (legendary) suffix = "Inermis"; else suffix = " of Quarrel";
					it[in].skill[SK_HAND][0] += 2 * mul;
					it[in].skill[SK_TAUNT][0] += 3 * mul;
				}
				else if (is_axxe)
				{
					if (legendary) suffix = "Vindex"; else suffix = " of Champions";
					it[in].skill[SK_AXE][0] += 2 * mul;
					it[in].base_crit += 1 * mul;
				}
				else if (is_swor)
				{
					if (legendary) suffix = "Incantator"; else suffix = " of Wizardry";
					it[in].skill[SK_SWORD][0] += 2 * mul;
					it[in].spell_mod[0] += 1 * mul;
				}
				else if (is_dagg)
				{
					if (legendary) suffix = "Cantatrix"; else suffix = " of Hexing";
					it[in].skill[SK_DAGGER][0] += 2 * mul;
					it[in].skill[SK_CURSE][0] += 3 * mul;
				}
				else if (is_shie)
				{
					if (legendary) suffix = "Procella"; else suffix = " of Vim";
					it[in].skill[SK_SHIELD][0] += 3 * mul;
					it[in].skill[SK_IMMUN][0] += 3 * mul;
				}
				else if (is_dual)
				{
					if (legendary) suffix = "Festinare"; else suffix = " of Haste";
					it[in].skill[SK_DUAL][0] += 3 * mul;
					it[in].skill[SK_HASTE][0] += 3 * mul;
				}
				else	// is_thsw
				{
					if (legendary) suffix = "Hastiludium"; else suffix = " of Jousting";
					it[in].skill[SK_TWOHAND][0] += 2 * mul;
					it[in].skill[SK_WEAKEN][0] += 3 * mul;
				}
				break;
			case 21:
				if (is_robe || is_mage || (is_dama && m))
				{
					if (legendary) suffix = "Mandatum"; else suffix = " of Command";
					it[in].skill[SK_BLAST][0] += 3 * mul;
					it[in].skill[SK_GHOST][0] += 3 * mul;
				}
				else if (is_claw)
				{
					if (legendary) suffix = "Concursus"; else suffix = " of Melee";
					it[in].skill[SK_HAND][0] += 2 * mul;
					it[in].weapon[0] += 2 * mul;
				}
				else if (is_thsw)
				{
					if (legendary) suffix = "Inhaere"; else suffix = " of Sparring";
					it[in].skill[SK_TWOHAND][0] += 2 * mul;
					it[in].skill[SK_CLEAVE][0] += 3 * mul;
				}
				else if (is_swor)
				{
					if (legendary) suffix = "Vindex"; else suffix = " of Champions";
					it[in].skill[SK_SWORD][0] += 2 * mul;
					it[in].base_crit += 1 * mul;
				}
				else	// is_axxe
				{
					if (legendary) suffix = "Tumultus"; else suffix = " of Uproar";
					it[in].skill[SK_AXE][0] += 2 * mul;
					it[in].skill[SK_TAUNT][0] += 3 * mul;
				}
				break;
			case 22:
				if (is_robe || is_mage || (is_dama && m))
				{
					if (legendary) suffix = "Caeremonia"; else suffix = " of Ceremony";
					it[in].skill[SK_GHOST][0] += 3 * mul;
					it[in].skill[SK_MSHIELD][0] += 3 * mul;
				}
				else if (is_claw)
				{
					if (legendary) suffix = "Vindex"; else suffix = " of Champions";
					it[in].skill[SK_HAND][0] += 2 * mul;
					it[in].base_crit += 1 * mul;
				}
				else if (is_thsw)
				{
					if (legendary) suffix = "Hastiludium"; else suffix = " of Jousting";
					it[in].skill[SK_TWOHAND][0] += 2 * mul;
					it[in].skill[SK_WEAKEN][0] += 3 * mul;
				}
				else if (is_swor)
				{
					if (legendary) suffix = "Concursus"; else suffix = " of Melee";
					it[in].skill[SK_SWORD][0] += 2 * mul;
					it[in].weapon[0] += 2 * mul;
				}
				else	// is_axxe
				{
					if (legendary) suffix = "Feritas"; else suffix = " of Brutality";
					it[in].skill[SK_AXE][0] += 2 * mul;
					it[in].skill[SK_CLEAVE][0] += 3 * mul;
				}
				break;
			case 23:
				if (is_robe || is_mage || (is_dama && m))
				{
					if (legendary) suffix = "Vis"; else suffix = " of Energy";
					it[in].skill[SK_MSHIELD][0] += 3 * mul;
					it[in].skill[SK_BLAST][0] += 3 * mul;
				}
				else
				{
					if (legendary) suffix = "Contumacia"; else suffix = " of Defiance";
					it[in].skill[SK_SURROUND][0] += 3 * mul;
					it[in].skill[SK_TAUNT][0] += 3 * mul;
				}
				break;
			case 24:
				if (is_robe || is_mage || (is_dama && m))
				{
					if (legendary) suffix = "Maledictio"; else suffix = " of Unbrance";
					it[in].skill[SK_CURSE][0] += 3 * mul;
					it[in].skill[SK_SLOW][0] += 3 * mul;
				}
				else
				{
					if (legendary) suffix = "Debellare"; else suffix = " of Vanquish";
					it[in].skill[SK_CLEAVE][0] += 3 * mul;
					it[in].skill[SK_WEAKEN][0] += 3 * mul;
				}
				break;
			case 25:
				if (is_claw || is_gaxe || is_thsw || is_axxe)
				{
					if (legendary) suffix = "Impetus"; else suffix = " of Offense";
					it[in].weapon[0] += 4 * mul;
				}
				else if (is_spea || is_staf || is_dagg)
				{
					if (legendary) suffix = "Incantator"; else suffix = " of Wizardry";
					if (is_spea || is_dagg) it[in].skill[SK_DAGGER][0] += 2 * mul;
					if (is_spea || is_staf) it[in].skill[SK_STAFF][0] += 2 * mul;
					it[in].spell_mod[0] += 1 * mul;
				}
				else	// is_swor
				{
					if (legendary) suffix = "Deceptio"; else suffix = " of Deception";
					it[in].skill[SK_SWORD][0] += 2 * mul;
					it[in].skill[SK_POISON][0] += 3 * mul;
				}
				break;
			case 26:
				if (is_claw || is_gaxe || is_thsw || is_axxe)
				{
					if (legendary) suffix = "Defendere"; else suffix = " of Defense";
					it[in].armor[0] += 4 * mul;
				}
				else if (is_spea || is_staf || is_dagg)
				{
					if (legendary) suffix = "Munitione"; else suffix = " of Fortification";
					if (is_spea || is_dagg) it[in].skill[SK_DAGGER][0] += 2 * mul;
					if (is_spea || is_staf) it[in].skill[SK_STAFF][0] += 2 * mul;
					it[in].armor[0] += 2 * mul;
				}
				else	// is_swor
				{
					if (legendary) suffix = "Callidus"; else suffix = " of Cunning";
					it[in].skill[SK_SWORD][0] += 2 * mul;
					it[in].skill[SK_BLIND][0] += 3 * mul;
				}
				break;
			case 27:
				if (legendary) suffix = "Acerbita"; else suffix = " of Sharpness";
				it[in].skill[SK_GEARMAST][0] += 3 * mul;
				it[in].skill[SK_ENHANCE][0] += 3 * mul;
				break;
			case 28:
				if (legendary) suffix = "Fortitudo"; else suffix = " of Toughness";
				it[in].skill[SK_GEARMAST][0] += 3 * mul;
				it[in].skill[SK_PROTECT][0] += 3 * mul;
				break;
			case 29:
				if (legendary) suffix = "Salus"; else suffix = " of Safety";
				it[in].skill[SK_HEAL][0] += 4 * mul;
				it[in].skill[SK_SWIM][0] += 4 * mul;
				break;
			case 30:
				if (legendary) suffix = "Chirurgia"; else suffix = " of Surgery";
				it[in].crit_multi[0] += 10 * mul;
				break;
			case 31:
				if (legendary) suffix = "Acceleratio"; else suffix = " of Acceleration";
				it[in].speed[0] += 2 * mul;
				break;
			//
			default:
				if (legendary) suffix = "Null"; else suffix = " of Nothing";
				break;
		}
		prev = roll;
	}
	
	// Update the item's sprite
	spr = get_special_spr(temp, it[in].sprite[0]);
	
	it[in].orig_temp = temp;
	it[in].sprite[0]  = spr;
	it[in].max_damage = 0;
	it[in].flags |= IF_SINGLEAGE | IF_NOMARKET;
	
	//xlog("create_special_item: strcpy name");
	
	strcpy(name, it[in].name);
	
	//xlog("create_special_item: sprintf newname");
	
	if (legendary)
	{
		sprintf(newname, "%s%s, %s", pref, suffix, name);
	}
	else
	{
		sprintf(newname, "%s%s%s", pref, name, suffix);
	}
	
	//xlog("create_special_item: send log name");
	
	xlog("create_special_item: %s (%d)", newname, strlen(newname));
	
	//for (n=39;n<80;n++) newname[n]=0; // cull excess characters from string
	//sprintf(it[in].name, "%s", newname);
	//sprintf(it[in].reference, "%s", newname);
	strncpy(it[in].name, newname, 39);
	it[in].name[39] = 0;
	strncpy(it[in].reference, newname, 39);
	it[in].reference[39] = 0;
	sprintf(it[in].description, "A %s.", newname);
	//
	it[in].name[0] = toupper(it[in].name[0]);
	
	//xlog("create_special_item: complete");

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
	{"Xecko"                    },	// 40
	{"Thug"                     },	// 41
	{"Cultist"                  },	// 42
	{"Strider"                  },	// 43
	{"Lycanthrope"              },	// 44
	//
	{"Lizard Guard"             },	// 45
	{"Lizard Monk"              },	// 46
	{"Lizard Priest"            },	// 47
	//
	{"Lizard Bandit"            },	// 48
	{"Lizard Dweller"           },	// 49
	{"Lizard Retainer"          },	// 50
	{"Lizard Archmage"          },	// 51
	{"Lizard Archknight"        },	// 52
	{"Emerald Golem"            },	// 53
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
	{"Reptite"					},	// 83
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
	{"Marquess Onyx Gargoyle"	},	// 159
	//
	{"Draugr"               	},	// 160
	{"Banshee"              	},	// 161
	{"Revenant"             	},	// 162
	{"Hellfire"             	},	// 163
	//
	{"Knight Onyx Gargoyle" 	},	// 164
	{"Baron Onyx Gargoyle"  	},	// 165
	{"Onyx Gargoyle Queen"  	},	// 166
	//
	{"Fallen"               	},	// 167
	{"Undead Knight"         	},	// 168
	{"Coal Golem"             	},	// 169
	{"Gargoyle Archmage"       	},	// 170
	{"Gargoyle Archknight"  	},	// 171
	{"Ice Gargoyle Knight"    	},	// 172
	{"Ice Gargoyle Mage"     	},	// 173
	{"Drider"               	},	// 174
	{"Despair"              	},	// 175
	{"Ice Gargoyle Archknight"	},	// 176
	{"Ice Gargoyle Archmage"	},	// 177
	{"Minotaur"             	},	// 178
	{"Dullahan"             	},	// 179
	//
	{"Charred"              	},	// 180
	{"Basalt Golem"         	},	// 181
	{"Marauder"             	},	// 182
	{"Ash Golem"               	},	// 183
	{"Volcan"                  	},	// 184
	{"Ignis"                    },	// 185
	//
	{""                     	},	// 186
	{""                     	},	// 187
	{""                     	},	// 188
	{""                     	},	// 189
	{""                     	},	// 190
	{""                     	}	// 191
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
		tmp = ch[cn].data[CHD_MASTER] & bit;
		ch[cn].data[CHD_MASTER] |= bit;
		return(tmp);
	}
	else if (val<160)
	{
		bit = 1 << (val - 128);
		tmp = ch[cn].data[70] & bit;
		ch[cn].data[70] |= bit;
		return(tmp);
	}
	else
	{
		bit = 1 << (val - 160);
		tmp = ch[cn].data[93] & bit;
		ch[cn].data[93] |= bit;
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
		return 0;
	}
	for (; *abbr; abbr++, name++)
	{
		if (*abbr != *name)
		{
			return 0;
		}
	}
	return 1;
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
				return 0;
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
			return 0;
	}


	if (!co)
	{
		do_char_log(cn, 0, "Sorry, could not create your enemy.\n");
		chlog(cn, "Sorry, could not create your enemy");
		return 0;
	}

	if (!god_drop_char(co, LAB_ARENA_BOSS_X, LAB_ARENA_BOSS_Y))
	{
		do_char_log(cn, 0, "Sorry, could not place your enemy.\n");
		chlog(cn, "Sorry, could not place your enemy");
		god_destroy_items(co);
		ch[co].used = USE_EMPTY;
		return 0;
	}

	ch[co].data[64] = globs->ticker + 5 * 60 * TICKS; // die in two minutes if not otherwise
	ch[co].data[24] = 0;    // do not interfere in fights
	ch[co].data[36] = 0;    // no walking around
	ch[co].data[43] = 0;    // don't attack anyone
	ch[co].data[MCD_ENEMY1ST] = 0;    // no enemies

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
		return 0;
	}
	chlog(cn, "Entered Labkeeper room");

	return 1;
}

void use_labtransfer2(int cn, int co)
{
	int cc;

	if (IS_COMPANION(cn) && (cc = ch[cn].data[CHD_MASTER])==ch[co].data[0])
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
		return 0;
	}
	strcpy(temp, sentence);
	strlower(temp);

	for (n = 0; n<cursize; n++)
	{
		if (strstr(temp, badword[n]))
		{
			return 1;
		}
	}
	return 0;
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
		return 0;                                             // no cap if feature is turned off

	}
	if (cn && (ch[cn].flags & (CF_GOD)))
	{
		return 0;                                             // always allow gods to come in all the time

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
				return 0;                             // always allow corpse retrieval
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
		return 0;
	}

	return(place);
}

int soultransform(int cn, int in, int in2, int temp)
{
	god_take_from_char(in, cn);
	god_take_from_char(in2, cn);

	it[in].used  = USE_EMPTY;
	it[in2].used = USE_EMPTY;

	in = god_create_item(temp);
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

// Soulstones
// data[0] is the power of the stone
// data[1] is the exp held by the stone
// data[2] is the 'focus' of the stone; if 0 there is no focus
// data[3] is the rank consumed by catalyzed stats

// Soulstones are limited to only select from skills the character can learn via skill[n][2]
// The total the skill can go up to is 	3 + ( skill[n][2] / 15 )
// The req to equip is 					( score - 3 ) * 15

// Penalty and bonus chart - need X score for Y points, gives Z gear requirement
static int soul_bonus[N_SOULBONUS][3] = {
	{ 3, 1, 12 }, //  0 Hand to Hand
	{ 1, 1,  4 }, //  1 Precision
	{ 3, 1, 12 }, //  2 Dagger
	{ 3, 1, 12 }, //  3 Sword
	{ 3, 1, 12 }, //  4 Axe
	{ 3, 1, 12 }, //  5 Staff
	{ 3, 1, 12 }, //  6 Two-handed
	{ 2, 1,  8 }, //  7 Zephyr
	{ 1, 1,  4 }, //  8 Stealth
	{ 1, 1,  4 }, //  9 Perception
	{ 1, 1,  4 }, // 10 Swimming
	{ 2, 1,  8 }, // 11 Magic Shield
	{ 1, 1,  4 }, // 12 Bartering
	{ 1, 1,  4 }, // 13 Repair
	{ 1, 1,  4 }, // 14 Light
	{ 1, 1,  4 }, // 15 Recall
	{ 2, 1,  8 }, // 16 Shield
	{ 2, 1,  8 }, // 17 Protect
	{ 2, 1,  8 }, // 18 Enhance
	{ 2, 1,  8 }, // 19 Slow
	{ 2, 1,  8 }, // 20 Curse
	{ 2, 1,  8 }, // 21 Bless
	{ 1, 1,  4 }, // 22 Identify
	{ 2, 1,  8 }, // 23 Resistance
	{ 2, 1,  8 }, // 24 Blast
	{ 1, 1,  4 }, // 25 Dispel
	{ 1, 1,  4 }, // 26 Heal
	{ 2, 1,  8 }, // 27 Ghost Companion
	{ 1, 1,  4 }, // 28 Regenerate
	{ 1, 1,  4 }, // 29 Rest
	{ 1, 1,  4 }, // 30 Meditate
	{ 1, 1,  4 }, // 31 Sense Magic
	{ 2, 1,  8 }, // 32 Immunity
	{ 1, 1,  4 }, // 33 Surround Hit
	{ 1, 1,  4 }, // 34 Concentrate
	{ 2, 1,  8 }, // 35 Warcry
	{ 2, 1,  8 }, // 36 Dual Wield
	{ 2, 1,  8 }, // 37 Blind
	{ 1, 1,  4 }, // 38 Weapon Mastery
	{ 1, 1,  4 }, // 39 Armor Mastery
	{ 2, 1,  8 }, // 40 Cleave
	{ 2, 1,  8 }, // 41 Weaken
	{ 2, 1,  8 }, // 42 Poison
	{ 2, 1,  8 }, // 43 Pulse
	{ 1, 1,  4 }, // 44 Proximity
	{ 1, 1,  4 }, // 45 Companion Mastery
	{ 2, 1,  8 }, // 46 Shadow Copy
	{ 2, 1,  8 }, // 47 Haste
	{ 2, 1,  8 }, // 48 Taunt
	{ 2, 1,  8 }, // 49 Leap
	
	{ 2, 1,  8 }, // 50 Braveness
	{ 2, 1,  8 }, // 51 Willpower
	{ 2, 1,  8 }, // 52 Intuition
	{ 2, 1,  8 }, // 53 Agility
	{ 2, 1,  8 }, // 54 Strength
	
	{ 1,10, 10 }, // 55 Hitpoints
	{ 1, 5,  0 }, // 56 Endurance
	{ 1,10, 10 }, // 57 Mana
	
	{ 2, 1,  0 }, // 58 Weapon Value
	{ 2, 1,  0 }, // 59 Armor Value
	
	{ 1, 1,  0 }, // 60 Movement Speed
	{ 1, 1,  0 }, // 61 Attack Speed
	{ 1, 1,  0 }, // 62 Cast Speed
	{ 4, 1,  0 }, // 63 Spell Mod
	{ 1, 1,  0 }, // 64 Spell Apt
	{ 3, 1,  0 }, // 65 Cooldown
	{ 1, 1,  0 }, // 66 Crit Chance
	{ 1, 1,  0 }, // 67 Crit Multi
	{ 1, 1,  0 }, // 68 Top Damage
	{ 2, 1,  0 }  // 69 Thorns
};

int get_sb(int v, int i)
{
	v = max(0, min(N_SOULBONUS-1, v));
	return soul_bonus[v][i];
}

void soultrans_equipment(int cn, int in, int in2)
{
	int rank, focus, try, r, n, t, m, c=0, s=0, temp=0, bonus=1;
	char known[N_SOULBONUS] = {0};
	char selected[N_SOULMAX*8] = {0};
	int atrm[5]={0}, sklm[50]={0};
	
	// 0. Check if (in) is twohanded, grant a rank bonus if it is.
	if (it[in2].placement & PL_TWOHAND)
		bonus = 2;
	
	// 1. Set local variables to pull from the soulstone (in)
	rank   = max(1, min(N_SOULMAX, (it[in].data[0]-it[in].data[3])*bonus));
	focus  = (it[in].data[2] > 0 && it[in].data[2] < 8) ? it[in].data[2] : 8;
	
	// 2. Loop through known player skills and add to local array
	for (n=0;n<N_SOULBONUS;n++)
	{
		if ((n<MAXSKILL && ch[cn].skill[n][2]) || n>=MAXSKILL)
		{
			// If Seyan, check that we know a given arch skill before adding it to the list.
			if ((n==SK_ZEPHYR || n==SK_WARCRY || n==SK_LETHARGY || 
				n==SK_PULSE || n==SK_SHADOW || n==SK_LEAP || n==SK_RAGE) && 
				IS_SEYAN_DU(cn) && !B_SK(cn, n))
				continue;
			known[c] = n; 
			c++;
		}
	}
	if (c<1)
	{
		chlog(cn, "soultrans_equipment failed with zero entries known");
		do_char_log(cn, 2, "Nothing happens.");
		return;
	}
	
	// 3. Select 'focus' number of skills from the known list - duplicates are OK
	//    We add extras if a catalyst is giving a bonus to the roll odds.
	try = 0;
	for (n=0;n<focus;n++)
	{
		m = 1;
		r = known[RANDOM(c)];
		selected[s] = r; 
		s++;
	}
	if (s<1)
	{
		chlog(cn, "soultrans_equipment failed with zero entries selected");
		do_char_log(cn, 2, "Nothing happens.");
		return;
	}
	
	// 4.1  Add catalyzed stats
	if (it[in].data[3])
	{
		for (n=0;n<5;n++)
		{ it[in2].attrib[n][0] += it[in].attrib[n][0]*bonus;  atrm[n] += soul_bonus[n][2]*it[in].attrib[n][0]; }
		for (n=0;n<50;n++)
		{ it[in2].skill[n][0] += it[in].skill[n][0]*bonus;    sklm[n] += soul_bonus[n][2]*it[in].skill[n][0]; }
		it[in2].hp[0]          += it[in].hp[0]*bonus;   it[in2].hp[2] += soul_bonus[55][2]*it[in].hp[0]/10;
		it[in2].end[0]         += it[in].end[0]*bonus;
		it[in2].mana[0]        += it[in].mana[0]*bonus; it[in2].hp[2] += soul_bonus[57][2]*it[in].mana[0]/10;
		
		it[in2].weapon[0]      += it[in].weapon[0]*bonus;
		it[in2].armor[0]       += it[in].armor[0]*bonus;
		
		it[in2].move_speed[0]  += it[in].move_speed[0]*bonus;
		it[in2].atk_speed[0]   += it[in].atk_speed[0]*bonus;
		it[in2].cast_speed[0]  += it[in].cast_speed[0]*bonus;
		it[in2].spell_mod[0]   += it[in].spell_mod[0]*bonus;
		it[in2].spell_apt[0]   += it[in].spell_apt[0]*bonus;
		it[in2].cool_bonus[0]  += it[in].cool_bonus[0]*bonus;
		it[in2].crit_chance[0] += it[in].crit_chance[0]*bonus;
		it[in2].crit_multi[0]  += it[in].crit_multi[0]*bonus;
		it[in2].top_damage[0]  += it[in].top_damage[0]*bonus;
		it[in2].gethit_dam[0]  += it[in].gethit_dam[0]*bonus;
	}
	
	// 4.2  Add the random stats
	t = 0;
	if (rank) while (rank && t < 5)
	{
		// 4a. Loop through selected stats and try to pick one.
		m = 1; r = -1; try = 0;
		while (r < 0 && try < 9)
		{
			try++;
			r = selected[RANDOM(s)];
			if (soul_bonus[r][0] > rank) r = -1;
		}
		if (r<0) break;
		
		// 4b. Add the stat to the item
		switch (r)
		{
			case 50: case 51: case 52: case 53: case 54:	// Attributes
				n = r-50;
				if ((it[in2].attrib[n][0] + soul_bonus[r][1]) > ch[cn].attrib[n][2]/max(1,soul_bonus[r][0]*5)) 
				{
					t++; m=0;
				}
				else
				{	
					it[in2].attrib[n][0] += soul_bonus[r][1]; t=0;
					atrm[n] += soul_bonus[r][2];
				}
				break;
				
			case 55:										// Hitpoints
				it[in2].hp[0] += soul_bonus[r][1]; t=0;
				it[in2].hp[2] += soul_bonus[r][2];
				break;
				
			case 56:										// Endurance
				it[in2].end[0] += soul_bonus[r][1]; t=0;
				break;
				
			case 57:										// Mana
				it[in2].hp[0] += soul_bonus[r][1]; t=0;
				it[in2].hp[2] += soul_bonus[r][2];
				break;
				
			case 58:										// Weapon Value
				it[in2].ss_weapon += soul_bonus[r][1]; t=0;
				break;
				
			case 59:										// Armor Value
				it[in2].ss_armor += soul_bonus[r][1]; t=0;
				break;
				
			case 60:										// Movement Speed
				it[in2].move_speed[0] += soul_bonus[r][1]; t=0;
				break;
				
			case 61:										// Attack Speed
				it[in2].atk_speed[0] += soul_bonus[r][1]; t=0;
				break;
				
			case 62:										// Cast Speed
				it[in2].cast_speed[0] += soul_bonus[r][1]; t=0;
				break;
				
			case 63:										// Spell Mod
				it[in2].spell_mod[0] += soul_bonus[r][1]; t=0;
				break;
				
			case 64:										// Spell Apt
				it[in2].spell_apt[0] += soul_bonus[r][1]; t=0;
				break;
				
			case 65:										// Cooldown
				it[in2].cool_bonus[0] += soul_bonus[r][1]; t=0;
				break;
				
			case 66:										// Crit Chance
				it[in2].crit_chance[0] += soul_bonus[r][1]; t=0;
				break;
				
			case 67:										// Crit Multi
				it[in2].crit_multi[0] += soul_bonus[r][1]; t=0;
				break;
				
			case 68:										// Top Damage
				it[in2].top_damage[0] += soul_bonus[r][1]; t=0;
				break;
				
			case 69:										// Thorns
				it[in2].gethit_dam[0] += soul_bonus[r][1]; t=0;
				break;
			
			default:										// Skills
				n = r;
				if (n>=50 || 
					(it[in2].skill[n][0]+soul_bonus[r][1]) > ch[cn].skill[n][2]/max(1,soul_bonus[r][0]*5))
				{
					t++; m=0;
				}
				else
				{	
					it[in2].skill[n][0] += soul_bonus[r][1]; t=0;
					sklm[n] += soul_bonus[r][2];
				}
				break;
		}
		if (m) 
		{
			rank -= soul_bonus[r][0];
		}
	}
	for (n=0;n<5;n++) if (atrm[n] && it[in2].attrib[n][0])
		it[in2].attrib[n][2] = max(it[in2].attrib[n][2], atrm[n]);
	for (n=0;n<50;n++) if (sklm[n] && it[in2].skill[n][0])
		it[in2].skill[n][2] = max(it[in2].skill[n][2], sklm[n]);
	
	rank = max(1, min(N_SOULMAX, it[in].data[0]));
	
	// 5. Repair the transformed item
	temp = it[in2].temp;
	
	it[in2].orig_temp = temp;

	it[in2].current_damage = 0;
	it[in2].current_age[0] = 0;
	it[in2].current_age[1] = 0;
	it[in2].damage_state = 0;
	
	it[in2].armor[0] = it_temp[temp].armor[0] + it[in2].ss_armor;
	it[in2].armor[1] = it_temp[temp].armor[1];
	
	it[in2].weapon[0] = it_temp[temp].weapon[0] + it[in2].ss_weapon;
	it[in2].weapon[1] = it_temp[temp].weapon[1];
	
	it[in2].sprite[0] = it_temp[temp].sprite[0];
	it[in2].sprite[1] = it_temp[temp].sprite[1];
	
	if (it[in2].temp!=IT_CH_FOOL) it[in2].temp = 0;
	
	// 6. Finalize the item
	it[in2].flags &= ~IF_NOREPAIR;
	it[in2].flags |= IF_UPDATE | IF_IDENTIFIED | IF_SOULSTONE;
	it[in2].min_rank = max(rank, it[in2].min_rank);
	it[in2].value -= 1;
	
	if (!it[in2].max_damage)
	{
		if (it[in2].flags & IF_WEAPON)		it[in2].max_damage = 2500 * it[in2].weapon[0]/2;
		else if (it[in2].flags & IF_BOOK)	it[in2].max_damage = 90000;
		else if (it[in2].power == 60)		it[in2].max_damage = 65000;
		else if (it[in2].power == 75)		it[in2].max_damage = 85000;
		else								it[in2].max_damage = 60000;
	}
	
	it[in2].power += rank * 30/4;

	souldestroy(cn, in);

	sprintf(it[in2].description, "A %s enhanced by a rank %d soulstone.", it[in2].name, rank);
	
	if (t==3)
	{
		chlog(cn, "soultrans_equipment error - ran out of tries on rank loop");
		do_char_log(cn, 2, "Err... Well, it kind of worked...");
	}
	else
	{
		do_char_log(cn, 2, "You enhanced the %s with a rank %d soulstone.", it[in2].name, rank);
	}
}

int can_be_soulstoned(int in)
{
	static int valid_ss[] = {
		// valid weapons
		32, 33, 34, 35, 36, 37, 38,	1778,					// Bronze
		284, 285, 286, 287, 288, 289, 290, 291, 292, 1779,	// Steel
		523, 524, 525, 526, 527, 528, 529, 530, 531, 1780,	// Gold
		532, 533, 534, 535, 536, 537, 538, 539, 540, 1781,	// Emerald
		541, 542, 543, 544, 545, 546, 547, 548, 549, 1782,	// Crystal
		572, 573, 574, 575, 576, 577, 578, 579, 580, 1783,	// Titanium
		693, 694, 695, 696, 697, 698, 699, 700, 701, 1784,	// Adamant
		2044,2045,2046,2047,2048,2049,2050,2051,2052,2053,	// Damascus
		// Map Weapons
		2513, 2516, 2517, 2518, 2521, 2522, 2523, 2524, 2525, 
		2527, 2528, 2531, 2533, 2536, 2537, 2538, 2540, 2541, 
		2542, 2544, 2546, 2547, 2548, 2549, 2550,
		// valid armors
		27, 28, 29, 30, 31, 39, 40, 41, 42, 43, 51, 52, 53, 54, 55, 	// Cloth, Leather, Bronze
		56, 57, 58, 59, 60,	61, 62, 63, 64, 65, 66, 67, 68, 69, 70,		// Steel, Gold, Emerald
		71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 94, 95, 96, 97, 98, 	// Crystal, Titanium, Adamant
		2028,2029,2030,2031,2032,	// Damascus
		2033,2034,2035,2036,2037,	// 
		2038,2039,2040,2041,2042,	// 
		337, 338, 339, 340, 341, 342, 343, 344, 345, 346,				// Simple, Caster
		347, 348, 349, 350, 351, 352, 353, 354, 355, 356,				// Adept, Wizard
		//
		2033, 2034, 2035, 2036, 2037, // Lizard
		2038, 2039, 2040, 2041, 2042, // Midnight
		// books
		1648, 1649, 1650, 1651, 1652, 1653, 1654, 1655, 
		1656, 2045, 2055, 2056, 1657, 1658, 1659,
		// other
		2104, 2109, IT_CH_FOOL
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

int do_soulcatalyst(int ins, int inc)
{
	int n, m;
	
	// Determine the stone's current rank/used rank
	if (it[ins].data[0] < it[ins].data[3] + it[inc].data[3])
	{
		return 0;
	}
	
	it[ins].data[3] += it[inc].data[3];
	it[ins].flags |= IF_IDENTIFIED;
	
	for (n=0;n<5;n++)
	{
		it[ins].attrib[n][0] += it[inc].attrib[n][0];
		it[ins].attrib[n][1] += it[inc].attrib[n][1];
	}
	for (n=0;n<50;n++)
	{
		it[ins].skill[n][0] += it[inc].skill[n][0];
		it[ins].skill[n][1] += it[inc].skill[n][1];
	}
	it[ins].hp[0]   += it[inc].hp[0];   it[ins].hp[1]   += it[inc].hp[1];
	it[ins].end[0]  += it[inc].end[0];  it[ins].end[1]  += it[inc].end[1];
	it[ins].mana[0] += it[inc].mana[0]; it[ins].mana[1] += it[inc].mana[1];
	
	it[ins].weapon[0] += it[inc].weapon[0]; it[ins].weapon[1] += it[inc].weapon[1];
	it[ins].armor[0]   += it[inc].armor[0]; it[ins].armor[1]  += it[inc].armor[1];
	
	it[ins].move_speed[0]  += it[inc].move_speed[0];  it[ins].move_speed[1]  += it[inc].move_speed[1];
	it[ins].atk_speed[0]   += it[inc].atk_speed[0];   it[ins].atk_speed[1]   += it[inc].atk_speed[1];
	it[ins].cast_speed[0]  += it[inc].cast_speed[0];  it[ins].cast_speed[1]  += it[inc].cast_speed[1];
	it[ins].spell_mod[0]   += it[inc].spell_mod[0];   it[ins].spell_mod[1]   += it[inc].spell_mod[1];
	it[ins].spell_apt[0]   += it[inc].spell_apt[0];   it[ins].spell_apt[1]   += it[inc].spell_apt[1];
	it[ins].cool_bonus[0]  += it[inc].cool_bonus[0];  it[ins].cool_bonus[1]  += it[inc].cool_bonus[1];
	it[ins].crit_chance[0] += it[inc].crit_chance[0]; it[ins].crit_chance[1] += it[inc].crit_chance[1];
	it[ins].crit_multi[0]  += it[inc].crit_multi[0];  it[ins].crit_multi[1]  += it[inc].crit_multi[1];
	it[ins].top_damage[0]  += it[inc].top_damage[0];  it[ins].top_damage[1]  += it[inc].top_damage[1];
	it[ins].gethit_dam[0]  += it[inc].gethit_dam[0];  it[ins].gethit_dam[1]  += it[inc].gethit_dam[1];
	
	return 1;
}

int do_catalyst_focus(int cn, int inf, int inc)
{
	int v, m;
	
	v = it[inc].data[4];
	m = it[inc].data[3];
	
	if (!v || m <= get_sb(v-1, 0)*3/2) 
	{
		do_char_log(cn, 1, "Nothing happened. Seems this catalyst can't be focused further.\n");
		chlog(cn, "do_catalyst_focus: v=%d, m=%d", v-1, m);
		return 0;
	}
	if (it[inf].data[0] >= get_sb(v-1, 0)*3/2)
	{
		do_char_log(cn, 1, "Nothing happened. Seems this focus won't improve anything.\n");
		return 0;
	}
	
	v--;
	
	it[inc].data[3] = m = max(get_sb(v, 0)*3/2, min(it[inf].data[0], m));
	
	if (v>=50 && v<=54) { it[inc].attrib[v-50][1] = m; }
	else if (v==55)     { it[inc].hp[1] = m; }
	else if (v==56)     { it[inc].end[1] = m; }
	else if (v==57)     { it[inc].mana[1] = m; }
	else if (v==58)     { it[inc].weapon[1] = m; }
	else if (v==59)     { it[inc].armor[1] = m; }
	else if (v==60)     { it[inc].move_speed[1] = m; }
	else if (v==61)     { it[inc].atk_speed[1] = m; }
	else if (v==62)     { it[inc].cast_speed[1] = m; }
	else if (v==63)     { it[inc].spell_mod[1] = m; }
	else if (v==64)     { it[inc].spell_apt[1] = m; }
	else if (v==65)     { it[inc].cool_bonus[1] = m; }
	else if (v==66)     { it[inc].crit_chance[1] = m; }
	else if (v==67)     { it[inc].crit_multi[1] = m; }
	else if (v==68)     { it[inc].top_damage[1] = m; }
	else if (v==69)     { it[inc].gethit_dam[1] = m; }
	else                { it[inc].skill[v][1] = m; }
	
	return 1;
}

int use_soulstone(int cn, int in)
{
	int in2, rank, c, t, n, m;

	if (!IS_SANECHAR(cn))	return 0;
	if (!IS_SANEITEM(in))	return 0;
	if (!(in2 = ch[cn].citem))
	{
		do_char_log(cn, 1, "Try using something with the soulstone. Click on the stone with an item under your cursor.\n");
		return 0;
	}
	if (!IS_SANEITEM(in2))	return 0;
	if (it[in2].driver==68)
	{
		// Remember: in2 is the item on the cursor. It gets applied to the item in inventory.
		
		it[in].data[1] += it[in2].data[1] - RANDOM(it[in2].data[1]/10 + 1);
		rank = points2rank(it[in].data[1]);
		
		if (rank > N_SOULMAX) rank = N_SOULMAX;
		
		it[in].data[0] = rank;
		sprintf(it[in].description, "Level %d soulstone, holding %d exp.", rank, it[in].data[1]);

		if (rank==N_SOULMAX)
		{
			do_char_log(cn, 1, "That's as high as they go.\n");
		}
		
		// Copy Focus
		if (it[in2].data[2] && RANDOM(2))
		{
			it[in].data[2] += RANDOM(it[in2].data[2]);
			if (it[in].data[2]>8)
				it[in].data[2] = 0;
		}

		souldestroy(cn, in2);
		return 1;
	}
	
	// Soul Focus
	if (it[in2].driver==92)
	{
		// Remember: in2 is the item on the cursor. It gets applied to the item in inventory.
		if (it[in].data[2])
		{
			do_char_log(cn, 1, "Nothing happened. Seems this stone is already focused.\n");
			return 0;
		}
		
		it[in].data[2] = it[in2].data[0];
		
		souldestroy(cn, in2);
		return 1;
	}
	
	// Soul Catalyst
	if (it[in2].driver==93)
	{
		if (do_soulcatalyst(in, in2))
		{
			souldestroy(cn, in2);
			return 1;
		}
		do_char_log(cn, 0, "The soulstone isn't powerful enough for that.\n");
		return 0;
	}

	switch(it[in2].temp)
	{
		case 18: // red flower
			in = soultransform(cn, in, in2, 101);
			it[in].hp[0] += 10;
			return 1;
		case 46: // purple flower
			in = soultransform(cn, in, in2, 102);
			it[in].mana[0] += 10;
			return 1;
		case 91: // torch
			in = soulrepair(cn, in, in2);
			it[in].max_age[1] *= 4;
			return 1;
		case 100: // flask
			in = soultransform(cn, in, in2, 102);
			return 1;
		case 101: // healing potion
			souldestroy(cn, in);
			it[in].hp[0] += 10;
			return 1;
		case 102: // mana potion
			souldestroy(cn, in);
			it[in].mana[0] += 10;
			return 1;
		default:
			if (can_be_soulstoned(in2))
			{
				soultrans_equipment(cn, in, in2);
				return 1;
			}
			else
			{
				do_char_log(cn, 1, "Nothing happened.\n");
				return 0;
			}
	}
}

int use_soulfocus(int cn, int in) // driver 92
{
	int in2, c, v, m;

	if (!IS_SANECHAR(cn))	return 0;
	if (!IS_SANEITEM(in))	return 0;
	if (!(in2 = ch[cn].citem))
	{
		do_char_log(cn, 1, "Try using this item with a soulstone or catalyst.\n");
		return 0;
	}
	if (!IS_SANEITEM(in2))	return 0;

	if (it[in2].driver==68) // Soulstone
	{
		// Remember: in2 is the item on the cursor. It gets applied to the item in inventory.
		if (it[in2].data[2])
		{
			do_char_log(cn, 1, "Nothing happened. Seems this stone is already focused.\n");
			return 0;
		}
		
		it[in2].data[2] = it[in].data[0];
		
		chlog(cn, "used focus on soulstone");
		souldestroy(cn, in);
		return 1;
	}
	
	if (it[in2].driver==93) // Soul Catalyst
	{
		if (do_catalyst_focus(cn, in, in2))
		{
			chlog(cn, "used focus on catalyst");
			souldestroy(cn, in);
			return 1;
		}
		return 0;
	}

	do_char_log(cn, 1, "Nothing happened.\n");
	return 0;
}

int use_soulcatalyst(int cn, int in) // driver 93
{
	int in2, c, t, n, m;

	if (!IS_SANECHAR(cn))	return 0;
	if (!IS_SANEITEM(in))	return 0;
	if (!(in2 = ch[cn].citem))
	{
		do_char_log(cn, 1, "Try using this item with a soulstone or focus.\n");
		return 0;
	}
	if (!IS_SANEITEM(in2))	return 0;

	if (it[in2].driver==68)
	{
		if (do_soulcatalyst(in2, in))
		{
			chlog(cn, "used catalyst on soulstone");
			souldestroy(cn, in);
			return 1;
		}
		do_char_log(cn, 0, "The soulstone isn't powerful enough for that.\n");
		return 0;
	}
	
	if (it[in2].driver==92) // Soul Focus
	{
		if (do_catalyst_focus(cn, in2, in))
		{
			chlog(cn, "used focus on catalyst");
			souldestroy(cn, in2);
			return 1;
		}
		return 0;
	}

	do_char_log(cn, 1, "Nothing happened.\n");
	return 0;
}

void make_catalyst(int cn, int in, int base)
{
	int m, v;
	
	sprintf(it[in].name, "Soul Catalyst");
	sprintf(it[in].reference, "soul catalyst");
	sprintf(it[in].description, "A soul catalyst. Can be used on a soulstone to grant it static properties.");
	
	m = (5-RANDOM(base/33+1))+1;
	
	v = RANDOM(70);
	if (v>=50 && v<=54) { it[in].attrib[v-50][0] = get_sb(v, 1); m = it[in].attrib[v-50][1] = get_sb(v, 0)*3/2+RANDOM(m); }
	else if (v==55)     { it[in].hp[0]           = get_sb(v, 1); m = it[in].hp[1]           = get_sb(v, 0)*3/2+RANDOM(m); }
	else if (v==56)     { it[in].end[0]          = get_sb(v, 1); m = it[in].end[1]          = get_sb(v, 0)*3/2+RANDOM(m); }
	else if (v==57)     { it[in].mana[0]         = get_sb(v, 1); m = it[in].mana[1]         = get_sb(v, 0)*3/2+RANDOM(m); }
	else if (v==58)     { it[in].weapon[0]       = get_sb(v, 1); m = it[in].weapon[1]       = get_sb(v, 0)*3/2+RANDOM(m); }
	else if (v==59)     { it[in].armor[0]        = get_sb(v, 1); m = it[in].armor[1]        = get_sb(v, 0)*3/2+RANDOM(m); }
	else if (v==60)     { it[in].move_speed[0]   = get_sb(v, 1); m = it[in].move_speed[1]   = get_sb(v, 0)*3/2+RANDOM(m); }
	else if (v==61)     { it[in].atk_speed[0]    = get_sb(v, 1); m = it[in].atk_speed[1]    = get_sb(v, 0)*3/2+RANDOM(m); }
	else if (v==62)     { it[in].cast_speed[0]   = get_sb(v, 1); m = it[in].cast_speed[1]   = get_sb(v, 0)*3/2+RANDOM(m); }
	else if (v==63)     { it[in].spell_mod[0]    = get_sb(v, 1); m = it[in].spell_mod[1]    = get_sb(v, 0)*3/2+RANDOM(m); }
	else if (v==64)     { it[in].spell_apt[0]    = get_sb(v, 1); m = it[in].spell_apt[1]    = get_sb(v, 0)*3/2+RANDOM(m); }
	else if (v==65)     { it[in].cool_bonus[0]   = get_sb(v, 1); m = it[in].cool_bonus[1]   = get_sb(v, 0)*3/2+RANDOM(m); }
	else if (v==66)     { it[in].crit_chance[0]  = get_sb(v, 1); m = it[in].crit_chance[1]  = get_sb(v, 0)*3/2+RANDOM(m); }
	else if (v==67)     { it[in].crit_multi[0]   = get_sb(v, 1); m = it[in].crit_multi[1]   = get_sb(v, 0)*3/2+RANDOM(m); }
	else if (v==68)     { it[in].top_damage[0]   = get_sb(v, 1); m = it[in].top_damage[1]   = get_sb(v, 0)*3/2+RANDOM(m); }
	else if (v==69)     { it[in].gethit_dam[0]   = get_sb(v, 1); m = it[in].gethit_dam[1]   = get_sb(v, 0)*3/2+RANDOM(m); }
	else                { it[in].skill[v][0]     = get_sb(v, 1); m = it[in].skill[v][1]     = get_sb(v, 0)*3/2+RANDOM(m); }
	
	it[in].data[3] = m;
	it[in].data[4] = v+1;
	it[in].temp = 0;
	it[in].driver = 93;
	it[in].flags |= IF_IDENTIFIED;
	
	god_give_char(in, cn);
	chlog(cn, "got soul catalyst");
}

void make_focus(int cn, int in, int base)
{
	int v;
	
	v = 6 - RANDOM(base/66+3);
					
	sprintf(it[in].name, "Soul Focus");
	sprintf(it[in].reference, "soul focus");
	sprintf(it[in].description, "A soul focus of %d. Can be used on a soulstone to reduce the number of random results, or a soul catalyst to reduce its rank cost.", v);
	
	it[in].data[0] = v;
	it[in].temp = 0;
	it[in].driver = 92;
	
	god_give_char(in, cn);
	chlog(cn, "got soul focus");
}

int start_contract(int cn, int in)
{
	unsigned long long prof;
	int rank, tier, mission, flags=0;
	
	if (cn==0 || !(in && it[in].temp==MCT_CONTRACT))
	{
		return 0;
	}
	
	if (!it[in].data[0])
	{
		do_char_log(cn, 0, "This contract hasn't been signed yet. Find a coloured quill and use it on the contract to sign it.\n");
		return 0;
	}
	/*
	else if (cn != it[in].data[0])
	{
		do_char_log(cn, 0, "This contract isn't yours.\n");
		return 0;
	}
	*/
	
	mission = it[in].data[1];
	tier    = it[in].data[2];
	flags   = it[in].data[3];
	rank    = points2rank(ch[cn].points_tot) + tier;
	
	return build_new_map(cn, rank, flags, mission, tier, in);
}

int get_tier_font(int tier)
{
	switch (tier)
	{
		case  1: return 2; // Green
		case  2: return 3; // Blue
		case  3: return 0; // Red
		default: return 1; // Yellow
	}
}

void show_map_flags(int cn, int flags, int tier)
{
	int font;
	
	font = get_tier_font(tier);
	
	if (!flags) 
	{
		do_char_log(cn, font, "It has no other details.\n"); 
		do_char_log(cn, font, " \n"); 
		return;
	}
	
	do_char_log(cn, font, "Modifiers:\n");
	do_char_log(cn, font, " \n");
	
	// Sorted alphabetically
	if (flags & 0x08000000) do_char_log(cn, font, CFL_08000000);
	if (flags & 0x00000001) do_char_log(cn, font, CFL_00000001);
	if (flags & 0x00000020) do_char_log(cn, font, CFL_00000020);
	if (flags & 0x00000004) do_char_log(cn, font, CFL_00000004);
	if (flags & 0x00000001) do_char_log(cn, font, CFL_00000002);
	if (flags & 0x00000010) do_char_log(cn, font, CFL_00000010);
	if (flags & 0x00000008) do_char_log(cn, font, CFL_00000008);
	if (flags & 0x10000000) do_char_log(cn, font, CFL_10000000);
	if (flags & 0x20000000) do_char_log(cn, font, CFL_20000000);
	if (flags & 0x00000200) do_char_log(cn, font, CFL_00000200);
	if (flags & 0x00000400) do_char_log(cn, font, CFL_00000400);
	if (flags & 0x00000040) do_char_log(cn, font, CFL_00000040);
	if (flags & 0x04000000) do_char_log(cn, font, CFL_04000000);
	if (flags & 0x00002000) do_char_log(cn, font, CFL_00002000);
	if (flags & 0x00008000) do_char_log(cn, font, CFL_00008000);
	if (flags & 0x00000800) do_char_log(cn, font, CFL_00000800);
	if (flags & 0x00001000) do_char_log(cn, font, CFL_00001000);
	if (flags & 0x00004000) do_char_log(cn, font, CFL_00004000);
	if (flags & 0x00000100) do_char_log(cn, font, CFL_00000100);
	if (flags & 0x01000000) do_char_log(cn, font, CFL_01000000);
	if (flags & 0x02000000) do_char_log(cn, font, CFL_02000000);
	if (flags & 0x00000080) do_char_log(cn, font, CFL_00000080);
	if (flags & 0x00800000) do_char_log(cn, font, CFL_00800000);
	if (flags & 0x00100000) do_char_log(cn, font, CFL_00100000);
	if (flags & 0x00200000) do_char_log(cn, font, CFL_00200000);
	if (flags & 0x00400000) do_char_log(cn, font, CFL_00400000);
	if (flags & 0x00010000) do_char_log(cn, font, CFL_00010000);
	if (flags & 0x00020000) do_char_log(cn, font, CFL_00020000);
	if (flags & 0x00080000) do_char_log(cn, font, CFL_00080000);
	if (flags & 0x00040000) do_char_log(cn, font, CFL_00040000);

	do_char_log(cn, font, " \n");
}

void clear_map_buffs(int cn, int flag)
{
	int in, n;
	ch[cn].data[41] = 0;			// Reset active map number
	for (n = 0; n<MAXBUFFS; n++)	// Remove map buffs/debuffs
	{
		if ((in = ch[cn].spell[n])==0)	continue;
		if (bu[in].temp >= 104 && bu[in].temp <= 110 && (flag == 1 || bu[in].data[6] <= 0))
		{
			bu[in].used = USE_EMPTY;
			ch[cn].spell[n] = 0;
			do_char_log(cn, 0, "%s ran out.\n", bu[in].name);
		}
		else if (bu[in].temp >= 104 && bu[in].temp <= 110 && bu[in].data[6])
		{
			bu[in].data[6]--;
		}
	}
	do_update_char(cn);
}

// Add progress to everyone in a given map. Done here in case a group disbands.
// loc is location
void add_map_progress(int loc)
{
	int x, xx, y, yy, co;
	
	xx = MM_TARGETX;
	yy = MM_TARGETY + MM_TARG_OF*(loc-1);
	
	// 2) Clean the target location
	for (x = 0; x < MAP_CELL_SIZE*MAP_TILE_SIZE; x++)
	{
		for (y = 0; y < MAP_CELL_SIZE*MAP_TILE_SIZE; y++)
		{
			if (co = map[(xx+x)+(yy+y)*MAPX].ch)
			{
				if (ch[co].used==USE_EMPTY)		continue;
				if (!IS_SANEPLAYER(co))			continue;
				ch[co].data[41]++;
				chlog(co, "map progress increased, %d (%d)", CONT_PROG(co), ch[co].data[41]);
			}
		}
	}
}

char *get_map_enemy_name(int kin)
{
	switch (kin)
	{
		case  1: return "Skeleton";
		case  2: return "Ghost";
		case  3: return "Rodent";
		case  4: return "Undead";
		case  5: return "Grolm";
		case  6: return "Skink";
		case  7: return "Golem";
		case  8: return "Gargoyle";
		case  9: return "Ice Garg";
		case 10: return "Flame";
		//
		case 11: return "Soldier";
		case 12: return "Samurai";
		case 13: return "Magi";
		case 14: return "Hunter";
		case 15: return "Witch";
		case 16: return "Ranger";
		case 17: return "Cleric";
		case 18: return "Spirit";
		case 19: return "Bones";
		case 20: return "Wanderer";
		case 21: return "Imp";
		case 22: return "Gator";
		case 23: return "Swarmer";
		case 24: return "Phantom";
		case 25: return "Construct";
		case 26: return "Daemon";
		case 27: return "Mummy";
		case 28: return "Roach";
		case 29: return "Statue";
		case 30: return "Jinni";
		case 31: return "Dragoon";
		case 32: return "Gladiator";
		case 33: return "Alchemist";
		case 34: return "Lancer";
		case 35: return "Druid";
		case 36: return "Swordsman";
		case 37: return "Paladin";
		case 38: return "Amazon";
		case 39: return "Dancer";
		case 40: return "Warlock";
		case 41: return "Wizard";
		case 42: return "Mystic";
		case 43: return "Grosser";
		//
		case 11+NUM_MAP_ENEM+0: return "Kali";
		case 11+NUM_MAP_ENEM+1: return "Rundas";
		case 11+NUM_MAP_ENEM+2: return "Szelanya";
		case 11+NUM_MAP_ENEM+3: return "Azrael";
		case 11+NUM_MAP_ENEM+4: return "Brighid";
		case 11+NUM_MAP_ENEM+5: return "Nidhogg";
		case 11+NUM_MAP_ENEM+6: return "Metztli";
		//
		default: return "Weirdo";
	}
}

char *get_map_enemy_desc(int kin)
{
	switch (kin)
	{
		case  1: return "A skeleton. Its eyes glow a deep red.";
		case  2: return "A ghost. Its eyes glow a deep red.";
		case  3: return "A rodent. Its eyes glow a deep red.";
		case  4: return "An undead. Its eyes glow a deep red.";
		case  5: return "A grolm. Its eyes glow a deep red.";
		case  6: return "A skink. Its eyes glow a deep red.";
		case  7: return "A golem. Its eyes glow a deep red.";
		case  8: return "A gargoyle. Its eyes glow a deep red.";
		case  9: return "An ice gargoyle. Its eyes glow a deep red.";
		case 10: return "A living flame. Its eyes glow a deep red.";
		//
		case 11: return "A man clad in armor and ready to fight."; // Soldier
		case 12: return "A man wearing long robes with knotted hair."; // Samurai
		case 13: return "A man wearing dark robes and a pointed hat."; // Magi
		case 14: return "A man clad in leathers, eyeing you with a smile."; // Hunter
		case 15: return "A woman wearing dark robes with a pointed nose."; // Witch
		case 16: return "A woman clad in leathers, carefully watching her surroundings."; // Ranger
		case 17: return "A woman clad in armor, dressed with holy cloths and symbols."; // Cleric
		case 18: return "A spooky spirit flutters through the air."; // Spirit
		case 19: return "A skeleton. No flesh, just..."; // Bones
		case 20: return "A woman wearing tight wraps of various colours."; // Wanderer
		case 21: return "A nasty looking creature."; // Imp
		case 22: return "A huge lizardfolk with a bulky frame and powerful jaws."; // Gator
		case 23: return "An ugly humanoid rat, it's face long and twisted."; // Swarmer
		case 24: return "A walking set of living clothes."; // Phantom
		case 25: return "A hulking monster made of bits of scrap."; // Construct
		case 26: return "A large demon with flaming fur and menacing wings."; // Daemon
		case 27: return "A humanoid wrapped in dark bandages, shambling limply."; // Mummy
		case 28: return "A massive insect with a glossy shell."; // Roach
		case 29: return "A living statue adorned with glossy stone wings."; // Statue
		case 30: return "A spirit of living flames."; // Jinni
		case 31: return "A woman in a suit of slender armor holding a spear."; // Dragoon
		case 32: return "A man adorned in shabby bits of armor with a grilled helmet."; // Gladiator
		case 33: return "A woman wearing thick robes, belts adorned with potions."; // Alchemist
		case 34: return "A woman clad in armor, long hair flowing from the helmet."; // Lancer
		case 35: return "A burly man with thick branches growing from his hair."; // Druid
		case 36: return "A man armed and adorned with many swords."; // Swordsman
		case 37: return "A large man in a suit of heavy, regal armor."; // Paladin
		case 38: return "A massive woman in studded leather armor."; // Amazon
		case 39: return "A slender woman wearing lacey transparent garbs."; // Dancer
		case 40: return "A slender man with a wicked grin."; // Warlock
		case 41: return "A tall man adorn with colorful robes."; // Wizard
		case 42: return "A woman adorn with foreign clothing and charms."; // Mystic
		case 43: return "An ugly humanoid lizard, dripping in thick goo."; // Grosser
		//
		case 11+NUM_MAP_ENEM+0: return "A large woman wearing cerulean armor. She wears a belt of severed heads, a skirt of blood trailing from them."; // Kali
		case 11+NUM_MAP_ENEM+1: return "A man adorn in many thick furs, tribal paint smothering his face. He has a wild and disheveled manner in his movements."; // Rundas
		case 11+NUM_MAP_ENEM+2: return "An elderly woman, wise beyond her years. Despite her age she moves with grace, the wind swirling around her like knives."; // Szelanya
		case 11+NUM_MAP_ENEM+3: return "You are shocked to see Azrael, the old king, alive and in his prime. What manner of sorcery is this?"; // Azrael
		case 11+NUM_MAP_ENEM+4: return "A woman with flowing golden hair, wearing a long white gown. She seems out of place here."; // Brighid
		case 11+NUM_MAP_ENEM+5: return "A monsterous looking man in slick black robes. They wear a cloak of thick green and black scales."; // Nidhogg
		case 11+NUM_MAP_ENEM+6: return "A woman dressed in feathered garbs. She marches aglow with a strange pale light."; // Metztli
		//
		default: return "A weirdo. Looks like garbage.";
	}
}

int get_map_enemy_sprite(int kin)
{
	switch (kin)
	{
		case  1: return 10192; // BS Skeleton
		case  2: return  9168; // BS Ghost
		case  3: return 14288; // BS Rodent
		case  4: return 19408; // BS Undead
		case  5: return 12240; // BS Grolm
		case  6: return 13264; // BS Skink
		case  7: return 17360; // BS Golem
		case  8: return 18384; // BS Gargoyle
		case  9: return 21456; // BS Icey
		case 10: return 22480; // BS Flame
		//
		case 11: return  2000; // Soldier
		case 12: return  3024; // Samurai
		case 13: return  4048; // Magi
		case 14: return  5072; // Hunter
		case 15: return  6096; // Witch
		case 16: return  7120; // Ranger
		case 17: return  8144; // Cleric
		case 18: return  9168; // Spirit
		case 19: return 10192; // Bones
		case 20: return 11216; // Wanderer
		case 21: return 12240; // Imp
		case 22: return 13264; // Gator
		case 23: return 14288; // Swarmer
		case 24: return 15312; // Phantom
		case 25: return 17360; // Construct
		case 26: return 18384; // Daemon
		case 27: return 19408; // Mummy
		case 28: return 20432; // Roach
		case 29: return 21456; // Statue
		case 30: return 22480; // Jinni
		case 31: return 22948; // Dragoon
		case 32: return 23504; // Gladiator
		case 33: return 24048; // Alchemist
		case 34: return 24528; // Lancer
		case 35: return 25048; // Druid
		case 36: return 25552; // Swordsman
		case 37: return 26048; // Paladin
		case 38: return 26576; // Amazon
		case 39: return 27600; // Dancer
		case 40: return 28624; // Warlock
		case 41: return 29648; // Wizard
		case 42: return 30672; // Mystic
		case 43: return 31696; // Grosser
		// case 44: return 27088; // 
		// case 45: return 28112; // 
		//
		case 11+NUM_MAP_ENEM+0: return 24528; // Kali
		case 11+NUM_MAP_ENEM+1: return 26048; // Rundas
		case 11+NUM_MAP_ENEM+2: return 26576; // Szelanya
		case 11+NUM_MAP_ENEM+3: return 3024;  // Azrael
		case 11+NUM_MAP_ENEM+4: return 27600; // Brighid
		case 11+NUM_MAP_ENEM+5: return 25048; // Nidhogg
		case 11+NUM_MAP_ENEM+6: return 30672; // Metztli
		//
		default: return  2000;
	}
}

void set_map_enemy_tarot(int co, int kin, int tarot)
{
	if (!tarot) return;
	
	int in = 0, in2 = 0;
	int tmp;
	
	switch (kin)
	{
	case 11: 	in  = RANDOM(2)?IT_CH_JUSTICE:IT_CH_TEMPER; 
				in2 = RANDOM(2)?IT_CH_WHEEL_R:IT_CH_HANGED_R; 	break; // Soldier
	case 12: 	in  = RANDOM(2)?IT_CH_JUSTICE:IT_CH_WHEEL; 
				in2 = RANDOM(2)?IT_CH_EMPERO_R:IT_CH_PREIST_R; 	break; // Samurai
	case 13: 	in  = RANDOM(2)?IT_CH_JUDGE:IT_CH_PREIST; 
				in2 = RANDOM(2)?IT_CH_MAGI_R:IT_CH_JUDGE_R; 	break; // Magi
	case 14: 	in  = RANDOM(2)?IT_CH_EMPEROR:IT_CH_DEATH; 
				in2 = RANDOM(2)?IT_CH_STRENG_R:IT_CH_TOWER_R; 	break; // Hunter
	case 15: 	in  = RANDOM(2)?IT_CH_EMPEROR:IT_CH_TOWER; 
				in2 = RANDOM(2)?IT_CH_CHARIO_R:IT_CH_MAGI_R; 	break; // Witch
	case 16: 	in  = RANDOM(2)?IT_CH_CHARIOT:IT_CH_WHEEL; 
				in2 = RANDOM(2)?IT_CH_STAR_R:IT_CH_CHARIO_R; 	break; // Ranger
	case 17: 	in  = RANDOM(2)?IT_CH_HEIROPH:IT_CH_STAR; 
				in2 = RANDOM(2)?IT_CH_EMPRES_R:IT_CH_TEMPER_R; 	break; // Cleric
	case 18: 	in  = RANDOM(2)?IT_CH_CHARIOT:IT_CH_MAGI; 
				in2 = RANDOM(2)?IT_CH_DEATH_R:IT_CH_DEVIL_R; 	break; // Spirit
	case 19: 	in  = RANDOM(2)?IT_CH_JUDGE:IT_CH_LOVERS; 
				in2 = RANDOM(2)?IT_CH_STRENG_R:IT_CH_SUN_R; 	break; // Bones
	case 20: 	in  = RANDOM(2)?IT_CH_TOWER:IT_CH_PREIST; 
				in2 = RANDOM(2)?IT_CH_EMPRES_R:IT_CH_DEATH_R; 	break; // Wanderer
	case 21: 	in  = RANDOM(2)?IT_CH_TOWER:IT_CH_MAGI; 
				in2 = RANDOM(2)?IT_CH_JUSTIC_R:IT_CH_JUDGE_R; 	break; // Imp
	case 22: 	in  = RANDOM(2)?IT_CH_EMPRESS:IT_CH_STRENGTH; 
				in2 = RANDOM(2)?IT_CH_JUSTIC_R:IT_CH_TOWER_R; 	break; // Gator
	case 23: 	in  = RANDOM(2)?IT_CH_CHARIOT:IT_CH_DEATH; 
				in2 = RANDOM(2)?IT_CH_WORLD_R:IT_CH_DEVIL_R; 	break; // Swarmer
	case 24: 	in  = RANDOM(2)?IT_CH_EMPEROR:IT_CH_MAGI; 
				in2 = RANDOM(2)?IT_CH_EMPERO_R:IT_CH_HANGED_R; 	break; // Phantom
	case 25: 	in  = RANDOM(2)?IT_CH_STAR:IT_CH_HANGED; 
				in2 = RANDOM(2)?IT_CH_TEMPER_R:IT_CH_WORLD_R; 	break; // Construct
	case 26: 	in  = RANDOM(2)?IT_CH_EMPRESS:IT_CH_STAR; 
				in2 = RANDOM(2)?IT_CH_TEMPER_R:IT_CH_STAR_R; 	break; // Daemon
	case 27: 	in  = RANDOM(2)?IT_CH_EMPEROR:IT_CH_TOWER; 
				in2 = RANDOM(2)?IT_CH_CHARIO_R:IT_CH_SUN_R; 	break; // Mummy
	case 28: 	in  = RANDOM(2)?IT_CH_TEMPER:IT_CH_HANGED; 
				in2 = RANDOM(2)?IT_CH_STRENG_R:IT_CH_SUN_R; 	break; // Roach
	case 29: 	in  = RANDOM(2)?IT_CH_TEMPER:IT_CH_EMPRESS; 
				in2 = RANDOM(2)?IT_CH_WHEEL_R:IT_CH_STAR_R; 	break; // Statue
	case 30: 	in  = RANDOM(2)?IT_CH_HEIROPH:IT_CH_JUDGE; 
				in2 = RANDOM(2)?IT_CH_DEVIL_R:IT_CH_MAGI_R; 	break; // Jinni
	case 31: 	in  = RANDOM(2)?IT_CH_JUSTICE:IT_CH_WHEEL; 
				in2 = RANDOM(2)?IT_CH_EMPERO_R:IT_CH_JUSTIC_R; 	break; // Dragoon
	case 32: 	in  = RANDOM(2)?IT_CH_JUSTICE:IT_CH_STRENGTH; 
				in2 = RANDOM(2)?IT_CH_WHEEL_R:IT_CH_HANGED_R; 	break; // Gladiator
	case 33: 	in  = RANDOM(2)?IT_CH_CHARIOT:IT_CH_HEIROPH; 
				in2 = RANDOM(2)?IT_CH_EMPRES_R:IT_CH_TOWER_R; 	break; // Alchemist
	case 34: 	in  = RANDOM(2)?IT_CH_DEATH:IT_CH_LOVERS; 
				in2 = RANDOM(2)?IT_CH_JUSTIC_R:IT_CH_STRENG_R; 	break; // Lancer
	case 35: 	in  = RANDOM(2)?IT_CH_STAR:IT_CH_LOVERS; 
				in2 = RANDOM(2)?IT_CH_HEIROP_R:IT_CH_TEMPER_R; 	break; // Druid
	case 36: 	in  = RANDOM(2)?IT_CH_HANGED:IT_CH_LOVERS; 
				in2 = RANDOM(2)?IT_CH_DEATH_R:IT_CH_HANGED_R; 	break; // Swordsman
	case 37: 	in  = RANDOM(2)?IT_CH_EMPRESS:IT_CH_STRENGTH; 
				in2 = RANDOM(2)?IT_CH_EMPRES_R:IT_CH_EMPERO_R; 	break; // Paladin
	case 38: 	in  = RANDOM(2)?IT_CH_WHEEL:IT_CH_STRENGTH; 
				in2 = RANDOM(2)?IT_CH_PREIST_R:IT_CH_DEATH_R; 	break; // Amazon
	case 39: 	in  = RANDOM(2)?IT_CH_TEMPER:IT_CH_HANGED; 
				in2 = RANDOM(2)?IT_CH_DEVIL_R:IT_CH_STAR_R; 	break; // Dancer
	case 40: 	in  = RANDOM(2)?IT_CH_MAGI:IT_CH_PREIST; 
				in2 = RANDOM(2)?IT_CH_HEIROP_R:IT_CH_SUN_R; 	break; // Warlock
	case 41: 	in  = RANDOM(2)?IT_CH_JUDGE:IT_CH_PREIST; 
				in2 = RANDOM(2)?IT_CH_PREIST_R:IT_CH_JUDGE_R; 	break; // Wizard
	case 42: 	in  = RANDOM(2)?IT_CH_HEIROPH:IT_CH_PREIST; 
				in2 = RANDOM(2)?IT_CH_PREIST_R:IT_CH_HEIROP_R; 	break; // Mystic
	case 43: 	in  = RANDOM(2)?IT_CH_DEATH:IT_CH_STRENGTH; 
				in2 = RANDOM(2)?IT_CH_CHARIO_R:IT_CH_TOWER_R; 	break; // Grosser
	//
	case 11+NUM_MAP_ENEM+0: in = IT_CH_JUSTICE;  in2 = IT_CH_JUSTIC_R; break; // Kali
	case 11+NUM_MAP_ENEM+1: in = IT_CH_STRENGTH; in2 = IT_CH_STRENG_R; break; // Rundas
	case 11+NUM_MAP_ENEM+2: in = IT_CH_DEATH;    in2 = IT_CH_DEATH_R;  break; // Szelanya
	case 11+NUM_MAP_ENEM+3: in = IT_CH_HEIROPH;  in2 = IT_CH_HEIROP_R; break; // Azrael
	case 11+NUM_MAP_ENEM+4: in = IT_CH_STAR;     in2 = IT_CH_STAR_R;   break; // Brighid
	case 11+NUM_MAP_ENEM+5: in = IT_CH_CHARIOT;  in2 = IT_CH_CHARIO_R; break; // Nidhogg
	case 11+NUM_MAP_ENEM+6: in = IT_CH_PREIST;   in2 = IT_CH_PREIST_R; break; // Metztli
	//
	default: break;
	}
	
	if (in && in2)
	{
		if (tarot & 1) tmp = ch[co].worn[WN_CHARM]  = pop_create_item(in,  co); it[tmp].carried = co;
		if (tarot & 2) tmp = ch[co].worn[WN_CHARM2] = pop_create_item(in2, co); it[tmp].carried = co;
	}
}

char *get_map_enemy_affix(int affix)
{
	switch (affix)
	{
		case  1: return "tough";
		case  2: return "fierce";
		//
		case  3: return "divine";
		case  4: return "cruel";
		case  5: return "timid";
		case  6: return "";
		//
		default: return "odd";
	}
}

int get_map_enemy_affix_length(int affix)
{
	switch (affix)
	{
		case  1: return 5;
		case  2: return 6;
		//
		case  3: return 6;
		case  4: return 5;
		case  5: return 5;
		case  6: return 0;
		//
		default: return 3;
	}
}

int get_map_eme[11+NUM_MAP_ENEM+NUM_LEG_ENEM][60] = {
//    0              5             10             15             20             25            (30)
	{50, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,50,50,50,50,50,50,50,50,50,50}, //  0 : null
	{40, 0, 0, 0, 0, 0, 0, 0, 0,50, 0, 0, 0, 0, 0, 0, 0,50,50, 0, 0,50, 0,55,40, 0, 0, 0,75,80,
	 75, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,60, 0, 0,60,55,50,45,40,35,50,35,49,51}, //  1 : bs skele
	{40, 0, 0, 0, 0, 0, 0, 0, 0,50, 0,60, 0, 0, 0, 0, 0,50,50, 0,40,50, 0,55, 0, 0, 0, 0,75,80,
	 75, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,55,45,40,50,60,40,50,30,51,49}, //  2 : bs ghost
	{40, 0, 0, 0, 0, 0, 0, 0, 0,50, 0, 0, 0, 0, 0, 0, 0,50,50,60, 0,50, 0,55, 0, 0, 0, 0,75,80,
	 75, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,40, 0, 0, 0, 0, 0, 0, 0,60,50,45,40,55,35,50,35,50,50}, //  3 : bs rodent
	{40, 0, 0, 0, 0, 0, 0, 0, 0,50, 0, 0, 0, 0, 0, 0, 0,50,50, 0, 0,50, 0,55, 0, 0, 0, 0,75,80,
	 75, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,40, 0,60, 0, 0, 0, 0, 0, 0,50,40,55,60,45,30,50,40,52,48}, //  4 : bs undead
	{40, 0, 0, 0, 0, 0, 0, 0, 0,50, 0, 0, 0, 0, 0, 0, 0,50,50, 0, 0,50, 0,55, 0, 0, 0, 0,75,80,
	 75, 0, 0, 0, 0, 0, 0,60, 0, 0,40, 0, 0, 0, 0, 0, 0, 0, 0, 0,40,45,60,55,50,30,50,40,46,54}, //  5 : bs grolm
	{40, 0, 0, 0, 0, 0, 0, 0, 0,50, 0, 0, 0, 0, 0, 0, 0,50,50, 0, 0,50, 0,55, 0, 0, 0, 0,75,80,
	 75, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,60, 0, 0, 0, 0,40, 0, 0,55,60,45,40,50,40,50,30,54,46}, //  6 : bs skink
	{40, 0, 0, 0, 0, 0, 0, 0, 0,50, 0, 0, 0, 0, 0, 0, 0,50,50, 0, 0,50, 0,55, 0, 0, 0, 0,75,80,
	 75, 0, 0, 0, 0, 0, 0,40, 0, 0, 0,60, 0, 0, 0, 0, 0, 0, 0, 0,50,60,55,45,40,40,50,30,45,55}, //  7 : bs golem
	{40, 0, 0, 0, 0, 0, 0, 0, 0,50, 0,40, 0, 0, 0, 0, 0,50,50, 0, 0,50, 0,55, 0, 0, 0, 0,75,80,
	 75, 0, 0, 0, 0, 0, 0, 0, 0, 0,60, 0, 0, 0, 0, 0, 0, 0, 0, 0,40,55,50,60,45,35,50,35,47,53}, //  8 : bs garg
	{40, 0, 0, 0, 0, 0, 0, 0, 0,50, 0, 0, 0, 0, 0, 0, 0,50,50,40,60,50, 0,55, 0, 0, 0, 0,75,80,
	 75, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,45,50,40,55,60,35,50,35,53,47}, //  9 : bs icey
	{40, 0, 0, 0, 0, 0, 0, 0, 0,50, 0, 0, 0, 0, 0, 0, 0,50,50, 0, 0,50, 0,55,60, 0, 0, 0,75,80,
	 75, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,40, 0, 0, 0, 0, 0, 0,45,40,60,50,55,40,50,30,48,52}, // 10 : bs flame
//
	{40, 0, 0, 0, 0, 0, 0, 0, 0,50, 0, 0, 0, 0, 0, 0, 0,50,50, 0, 0,50, 0,60, 0, 0, 0, 0,75,75,
	 75, 0,55,60, 0, 0, 0, 0, 0, 0,40,45, 0, 0, 0, 0, 0, 0,50, 0,49,52,55,49,45,35,50,35,47,53}, // 11 : Soldier
	{40,40, 0, 0, 0, 0, 0, 0, 0,50, 0, 0, 0, 0, 0, 0, 0,50,50, 0, 0,50, 0,60, 0,45, 0, 0,75,75,
	 75, 0, 0, 0, 0,55, 0, 0, 0, 0,60, 0, 0, 0, 0, 0, 0,50, 0, 0,51,44,50,58,47,35,50,35,45,55}, // 12 : Samurai
	{40, 0, 0, 0, 0, 0, 0, 0, 0,50, 0,45, 0, 0, 0, 0, 0,50,50, 0, 0,50, 0,60,40, 0, 0,50,75,75,
	 75, 0, 0, 0,55, 0, 0, 0, 0, 0, 0, 0, 0,60, 0, 0, 0, 0, 0, 0,53,45,44,53,55,38,50,32,53,47}, // 13 : Magi
	{40, 0, 0, 0, 0, 0, 0, 0,50,50, 0, 0, 0, 0, 0, 0, 0,50,50,55, 0,50, 0,60, 0, 0, 0, 0,75,75,
	 75, 0, 0, 0, 0, 0, 0, 0, 0, 0,	0,40,60, 0, 0, 0, 0, 0,45, 0,46,55,51,48,50,36,50,34,50,50}, // 14 : Hunter
	{40, 0, 0, 0, 0, 0, 0, 0, 0,50, 0,45, 0, 0, 0, 0, 0,50,50,50,40,50, 0,60, 0, 0, 0, 0,75,75,
	 75, 0, 0, 0,60, 0, 0, 0, 0, 0,	0, 0,55, 0, 0, 0, 0, 0, 0, 0,45,49,43,53,60,39,50,31,50,50}, // 15 : Witch
	{40,45, 0, 0, 0, 0, 0, 0,40,50, 0, 0, 0, 0, 0, 0, 0,50,50,50, 0,50, 0,60,55, 0, 0, 0,75,75,
	 75, 0, 0, 0, 0, 0, 0,60, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,54,46,52,44,54,35,50,35,46,54}, // 16 : Ranger
	{40, 0, 0, 0, 0, 0, 0, 0, 0,50, 0,60, 0, 0, 0, 0, 0,50,50, 0, 0,50, 0,60, 0,45,40, 0,75,75,
	 75, 0,55, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0,50, 0, 0, 0, 0, 0, 0,51,48,49,52,50,32,50,38,52,48}, // 17 : Cleric
	{40, 0, 0, 0, 0, 0, 0,55, 0,50, 0, 0, 0, 0, 0, 0, 0,50,50,45, 0,50, 0,60, 0, 0, 0, 0,75,75,
	 75, 0, 0, 0,40, 0, 0,60, 0, 0,	0, 0, 0, 0, 0, 0,50, 0, 0, 0,52,54,45,45,54,38,50,32,51,49}, // 18 : Spirit
	{40, 0, 0, 0, 0, 0, 0, 0, 0,50, 0, 0, 0, 0, 0, 0, 0,50,50,40, 0,50, 0,60,55, 0, 0, 0,75,75,
	 75, 0, 0, 0, 0, 0, 0,50, 0, 0,	0, 0, 0, 0,60, 0, 0,45, 0, 0,45,51,52,51,51,34,50,36,49,51}, // 19 : Bones
	{40, 0, 0, 0, 0, 0, 0,40,50,50, 0, 0, 0, 0, 0, 0, 0,50,50, 0,60,50, 0,60, 0, 0, 0, 0,75,75,
	 75, 0, 0,55, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0,45, 0, 0,50,50,50,50,50,35,50,35,50,50}, // 20 : Wanderer
	{40, 0, 0, 0, 0, 0, 0, 0, 0,50, 0, 0, 0, 0, 0, 0, 0,50,50, 0,55,50, 0,60, 0, 0, 0,45,75,75,
	 75, 0, 0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0,40, 0, 0, 0,50, 0,60,59,50,48,40,53,37,50,33,49,51}, // 21 : Imp
	{40,55, 0, 0, 0, 0, 0, 0, 0,50, 0,45, 0, 0, 0, 0, 0,50,50, 0, 0,50, 0,60, 0, 0, 0, 0,75,75,
	 75, 0, 0, 0, 0, 0, 0, 0, 0, 0,	0, 0,40, 0, 0, 0, 0, 0,60,50,48,49,55,57,41,35,50,35,48,52}, // 22 : Gator
	{40, 0, 0, 0, 0, 0, 0, 0, 0,50, 0, 0, 0, 0, 0, 0, 0,50,50,60, 0,50, 0,60, 0, 0, 0, 0,75,75,
	 75, 0, 0, 0, 0, 0, 0,40, 0, 0,	0,55, 0, 0,50, 0,45, 0, 0, 0,60,51,47,43,49,34,50,36,53,47}, // 23 : Swarmling
	{40, 0, 0, 0, 0, 0, 0,50, 0,50, 0, 0, 0, 0, 0, 0, 0,50,50,45, 0,50, 0,60, 0, 0, 0, 0,75,75,
	 75, 0, 0, 0, 0,40, 0, 0, 0, 0,	0, 0, 0,55, 0, 0,60, 0, 0, 0,55,50,46,42,57,32,50,38,52,48}, // 24 : Phantom
	{40, 0, 0, 0, 0, 0, 0, 0, 0,50, 0,50, 0, 0, 0, 0, 0,50,50, 0, 0,50, 0,60, 0, 0,55, 0,75,75,
	 75, 0,40, 0, 0,60, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,45, 0,47,53,60,50,40,30,50,40,54,46}, // 25 : Construct
	{40, 0, 0, 0, 0, 0, 0, 0, 0,50, 0,60, 0, 0, 0, 0, 0,50,50, 0,55,50, 0,60,50, 0,45, 0,75,75,
	 75, 0, 0, 0, 0, 0, 0, 0, 0, 0,40, 0, 0, 0, 0, 0, 0, 0, 0, 0,54,58,50,46,42,33,50,37,49,51}, // 26 : Daemon
	{40, 0, 0, 0, 0, 0, 0, 0, 0,50, 0, 0, 0, 0, 0, 0, 0,50,50,55,40,50, 0,60, 0, 0, 0, 0,75,75,
	 75, 0,45, 0, 0, 0, 0,50, 0, 0,	0, 0,60, 0, 0, 0, 0, 0, 0, 0,44,53,51,56,46,33,50,37,50,50}, // 27 : Mummy
	{40, 0, 0, 0, 0, 0, 0, 0, 0,50, 0, 0, 0, 0, 0, 0, 0,50,50, 0, 0,50, 0,60, 0, 0, 0, 0,75,75,
	 75, 0,55,50, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0,40,60,45,58,41,54,41,56,31,50,39,50,50}, // 28 : Roach
	{40, 0, 0, 0, 0, 0, 0, 0, 0,50, 0,45, 0, 0, 0, 0, 0,50,50, 0, 0,50, 0,60, 0, 0, 0, 0,75,75,
	 75, 0,50, 0, 0, 0, 0, 0, 0, 0,55,50, 0, 0, 0, 0, 0, 0,50, 0,40,48,58,60,44,31,50,39,55,45}, // 29 : Statue
	{40, 0, 0, 0, 0, 0, 0, 0, 0,50, 0, 0, 0, 0, 0, 0, 0,50,50, 0, 0,50, 0,60,45,55, 0, 0,75,75,
	 75, 0, 0, 0,60, 0, 0, 0, 0, 0,	0, 0, 0, 0,40, 0,50, 0, 0, 0,50,60,42,47,51,39,50,31,47,53}, // 30 : Jinni
	{40,50, 0, 0, 0, 0, 0, 0, 0,50, 0, 0, 0, 0, 0, 0, 0,50,50, 0, 0,50, 0,60, 0, 0, 0, 0,75,75,
	 75, 0, 0,45, 0,55, 0, 0, 0, 0,60, 0, 0, 0, 0, 0, 0, 0, 0,40,46,59,54,46,45,34,50,36,47,53}, // 31 : Dragoon
	{40,60, 0, 0, 0, 0, 0, 0, 0,50, 0, 0, 0, 0, 0, 0, 0,50,50, 0, 0,50, 0,60, 0, 0, 0, 0,75,75,
	 75, 0, 0,40, 0, 0, 0,45, 0, 0,50, 0, 0, 0,55, 0, 0, 0, 0, 0,42,56,59,50,43,32,50,38,52,48}, // 32 : Gladiator
	{40, 0, 0, 0, 0, 0, 0, 0, 0,50, 0, 0, 0, 0, 0, 0, 0,50,50, 0, 0,50, 0,60, 0,60,50, 0,75,75,
	 75, 0, 0, 0, 0, 0, 0,55, 0, 0,	0, 0,45, 0,40, 0, 0, 0, 0, 0,55,47,45,51,52,37,50,33,51,49}, // 33 : Alchemist
	{40, 0, 0, 0, 0, 0, 0,60, 0,50, 0, 0, 0, 0, 0, 0, 0,50,50, 0, 0,50, 0,60, 0, 0, 0, 0,75,75,
	 75, 0, 0, 0, 0,40, 0, 0, 0, 0,	0,50, 0, 0, 0, 0, 0,55, 0,45,53,47,56,48,46,33,50,37,46,54}, // 34 : Lancer
	{40, 0, 0, 0, 0, 0, 0, 0,60,50, 0, 0, 0, 0, 0, 0, 0,50,50, 0, 0,50, 0,60, 0, 0,50,40,75,75,
	 75, 0, 0, 0,45, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0,55, 0, 0, 0, 0,57,40,46,55,52,36,50,34,54,46}, // 35 : Druid
	{40, 0, 0, 0, 0, 0, 0,40, 0,50, 0, 0, 0, 0, 0, 0, 0,50,50, 0, 0,50, 0,60, 0, 0, 0, 0,75,75,
	 75, 0,45,50, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0,55, 0,60,48,54,53,47,48,34,50,36,48,52}, // 36 : Swordsman
	{40, 0, 0, 0, 0, 0, 0, 0, 0,50, 0,50, 0, 0, 0, 0, 0,50,50, 0, 0,50, 0,60, 0,55,60, 0,75,75,
	 75, 0, 0, 0, 0,40, 0, 0, 0, 0,45, 0, 0, 0, 0, 0, 0, 0, 0, 0,41,55,47,59,48,33,50,37,55,45}, // 37 : Paladin
	{40,50, 0, 0, 0, 0, 0,55, 0,50, 0, 0, 0, 0, 0, 0, 0,50,50, 0,45,50, 0,60, 0, 0, 0, 0,75,75,
	 75, 0, 0, 0, 0,60, 0, 0, 0, 0,	0, 0, 0,40, 0, 0, 0, 0, 0, 0,49,52,53,49,47,36,50,34,45,55}, // 38 : Amazon
	{40, 0, 0, 0, 0, 0, 0, 0,45,50, 0, 0, 0, 0, 0, 0, 0,50,50, 0, 0,50, 0,60, 0, 0, 0, 0,75,75,
	 75, 0,55, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0,50,60,40, 0,47,57,48,45,53,37,50,33,49,51}, // 39 : Dancer
	{40, 0, 0, 0, 0, 0, 0, 0,55,50, 0, 0, 0, 0, 0, 0, 0,50,50, 0,50,50, 0,60, 0, 0, 0,60,75,75,
	 75, 0, 0, 0, 0, 0, 0,40, 0, 0,	0, 0, 0, 0, 0,45, 0, 0, 0, 0,50,42,49,54,55,38,50,32,51,49}, // 40 : Warlock
	{40, 0, 0, 0, 0, 0, 0, 0, 0,50, 0, 0, 0, 0, 0, 0, 0,50,50, 0, 0,50, 0,60,45, 0, 0,50,75,75,
	 75, 0, 0, 0,40, 0, 0, 0, 0, 0,	0, 0, 0,55,60, 0, 0, 0, 0, 0,56,43,40,52,59,40,50,30,51,49}, // 41 : Wizard
	{40, 0, 0, 0, 0, 0, 0, 0, 0,50, 0, 0, 0, 0, 0, 0, 0,50,50, 0, 0,50, 0,60,60,40, 0,55,75,75,
	 75, 0, 0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0,50,45, 0, 0, 0,52,45,41,54,58,37,50,33,48,52}, // 42 : Mystic
	{40, 0, 0, 0, 0, 0, 0, 0,50,50, 0, 0, 0, 0, 0, 0, 0,50,50, 0,55,50, 0,60, 0, 0, 0, 0,75,75,
	 75, 0, 0, 0, 0, 0, 0, 0, 0, 0,	0,60,40, 0,45, 0, 0, 0, 0, 0,43,46,57,55,49,36,50,34,53,47}, // 43 : Grossling
//   30             35             40             45             50             55            (60)
	{40,50, 0, 0, 0, 0, 0, 0, 0,44, 0, 0, 0, 0, 0, 0, 0,50,50, 0, 0,50, 0,55, 0, 0, 0, 0,75,75,
	 75, 0, 0,50, 0, 0, 0, 0, 0, 0,45, 0, 0, 0, 0, 0, 0, 0, 0,45,45,45,45,45,45,31,40,35,45,51}, // *1 : Kali
	{40, 0, 0, 0, 0, 0, 0, 0,50,44, 0, 0, 0, 0, 0, 0, 0,50,50,45, 0,50, 0,55, 0, 0, 0, 0,75,75,
	 75, 0, 0, 0, 0,45, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,50, 0,45,45,45,45,45,30,40,36,46,50}, // *2 : Rundas
	{40, 0, 0, 0, 0, 0, 0,45, 0,44, 0, 0, 0, 0, 0, 0, 0,50,50, 0, 0,50, 0,55, 0, 0, 0, 0,75,75,
	 75, 0,45, 0, 0, 0, 0, 0, 0, 0, 0,50, 0, 0, 0, 0, 0,50, 0, 0,45,45,45,45,45,35,40,31,49,47}, // *3 : Szelanya
	{40, 0, 0, 0, 0, 0, 0, 0, 0,44, 0,50, 0, 0, 0, 0, 0,50,50, 0, 0,50, 0,55, 0,45, 0, 0,75,75,
	 75, 0, 0,50, 0, 0, 0, 0, 0, 0,45, 0, 0, 0, 0, 0, 0, 0, 0, 0,45,45,45,45,45,33,40,33,48,48}, // *4 : Azrael
	{40, 0, 0, 0, 0, 0, 0, 0, 0,44, 0,50, 0, 0, 0, 0, 0,50,50, 0, 0,50, 0,55, 0,45,45, 0,75,75,
	 75, 0, 0, 0,50, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,45,45,45,45,45,34,40,32,51,45}, // *5 : Brighid
	{40, 0, 0, 0, 0, 0, 0, 0, 0,44, 0, 0, 0, 0, 0, 0, 0,50,50, 0, 0,50, 0,55, 0, 0, 0,45,75,75,
	 75, 0, 0, 0, 0, 0, 0,45, 0, 0, 0, 0,50, 0, 0,50, 0, 0, 0, 0,45,45,45,45,45,32,40,34,47,49}, // *6 : Nidhogg
	{40, 0, 0, 0, 0, 0, 0, 0, 0,44, 0, 0, 0, 0, 0, 0, 0,50,50, 0,45,50, 0,55, 0, 0, 0, 0,75,75,
	 75, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,45,50, 0,50, 0, 0, 0,45,45,45,45,45,36,40,30,50,46}  // *7 : Metztli
};

int generate_map_enemy(int temp, int kin, int xx, int yy, int base, int affix, int tarot)
{
	int co, tmp, pts, n, m, j, in, v, rank;
	char buf[40];
	
	co = pop_create_char(temp, 0);
	if (!co)
	{
		xlog("create char failed in generate_map_enemy()");
		return 0;
	}
	if (!god_drop_char_fuzzy(co, xx, yy))
	{
		xlog("drop char failed in generate_map_enemy()");
		god_destroy_items(co);
		ch[co].used = USE_EMPTY;
		return 0;
	}
	fx_add_effect(6, 0, ch[co].x, ch[co].y, 0);
	
	// Set name and sprite
	if (kin)
	{
		sprintf(ch[co].name, "%s", get_map_enemy_name(kin));
		sprintf(ch[co].reference, "the %s", get_map_enemy_name(kin));
		sprintf(ch[co].description, "%s", get_map_enemy_desc(kin));
		ch[co].sprite = get_map_enemy_sprite(kin);
		if (kin==10||kin==28||kin==30) // Set Simple Animation
			ch[co].flags |= CF_SIMPLE;
		set_map_enemy_tarot(co, kin, tarot);
	}
	
	// Mimic GC code
	for (n = 0; n<5; n++)
	{
		tmp = base * 50 / max(1, get_map_eme[kin][n+50]);
		B_AT(co, n) = max(10, min(135, tmp));
	}

	for (n = 0; n<MAXSKILL; n++)
	{
		if (get_map_eme[kin][n])
		{
			tmp = base * 50 / max(1, get_map_eme[kin][n]);
			B_SK(co, n) = max(1, min(135, tmp));
		}
	}
	
	tmp = base * 50 / max(1, get_map_eme[kin][55]) * 5;	ch[co].hp[0]   	= max(100, min(999, tmp));
	tmp = base * 50 / max(1, get_map_eme[kin][56]) * 2;	ch[co].end[0]  	= max(100, min(999, tmp));
	tmp = base * 50 / max(1, get_map_eme[kin][57]) * 5;	ch[co].mana[0] 	= max(100, min(999, tmp));
	tmp = base * 50 / max(1, get_map_eme[kin][58]) / 2; ch[co].weapon_bonus = max( 12, min(120, tmp));
	tmp = base * 50 / max(1, get_map_eme[kin][59]) / 2; ch[co].armor_bonus  = max(  8, min(120, tmp));
	
	chlog(co, "created map mob");
	
	// Apply flags and Prefix
	if (affix)
	{
		if (n = get_map_enemy_affix_length(affix))
		{
			sprintf(buf, "the %s %s", get_map_enemy_affix(affix), ch[co].name); buf[5+n] = tolower(buf[5+n]);
			strncpy(ch[co].reference, buf, 39); ch[co].reference[39] = 0;
			
			sprintf(buf, "%s %s", get_map_enemy_affix(affix), ch[co].name); buf[0] = toupper(buf[0]);
			strncpy(ch[co].name, buf, 39); ch[co].name[39] = 0;
		}
		
		if (!(ch[co].flags & CF_EXTRAEXP)) ch[co].flags |= CF_EXTRAEXP;
	
		switch (affix)
		{
			case 1:
				for (n = 0; n<5; n++)
					B_AT(co, n) += 3+RANDOM(3);
				for (n = 0; n<MAXSKILL; n++) if (B_SK(co, n))
					B_SK(co, n) += 3+RANDOM(3)*2;
				break;
			case 2:
				for (n = 0; n<5; n++)
					B_AT(co, n) += 6+RANDOM(5);
				for (n = 0; n<MAXSKILL; n++) if (B_SK(co, n))
					B_SK(co, n) += 6+RANDOM(5)*2;
				if (!(ch[co].flags & CF_CANCRIT))
					ch[co].flags |= CF_CANCRIT;
				break;
			//
			case 3:
				for (n = 0; n<5; n++)
					B_AT(co, n) += (B_AT(co, n)/20)+RANDOM(B_AT(co, n)/18);
				for (n = 0; n<MAXSKILL; n++) if (B_SK(co, n))
					B_SK(co, n) += (B_AT(co, n)/20)+RANDOM(B_AT(co, n)/18)*2;
				ch[co].data[72] = 3;
				//
				if (in = god_create_item(IT_SOULCATAL)) make_catalyst(co, in, base);
				// 
				break;
			case 4:
				for (n = 0; n<5; n++)
					B_AT(co, n) += (B_AT(co, n)/18)+RANDOM(B_AT(co, n)/16);
				for (n = 0; n<MAXSKILL; n++) if (B_SK(co, n))
					B_SK(co, n) += (B_AT(co, n)/18)+RANDOM(B_AT(co, n)/16)*2;
				ch[co].data[72] = 4;
				//
				if (in = god_create_item(IT_SOULCATAL)) make_catalyst(co, in, base);
				//
				break;
			case 5:
				for (n = 0; n<5; n++)
					B_AT(co, n) += (B_AT(co, n)/22)+RANDOM(B_AT(co, n)/20);
				for (n = 0; n<MAXSKILL; n++) if (B_SK(co, n))
					B_SK(co, n) += (B_AT(co, n)/22)+RANDOM(B_AT(co, n)/20)*2;
				ch[co].data[72] = 5;
				ch[co].data[25] = 1;
				if (in = god_create_item(IT_SOULFOCUS)) make_focus(co, in, base);
				break;
			case 6:
				for (n = 0; n<5; n++)
					B_AT(co, n) += (B_AT(co, n)/22)+RANDOM(B_AT(co, n)/22);
				for (n = 0; n<MAXSKILL; n++) if (B_SK(co, n))
					B_SK(co, n) += (B_AT(co, n)/22)+RANDOM(B_AT(co, n)/22)*2;
				ch[co].data[72] = 6;
				if (in = god_create_item(IT_SOULFOCUS)) make_focus(co, in, base);
				break;
			//
			default:
				break;
		}
	}
	
	// Calculate experience
	pts = 0;
	for (n = 0; n<5; n++) for (j = 10; j<B_AT(co, n); j++)
		pts += attrib_needed(j, 4);
	for (j = 50; j<ch[co].hp[0]; j++) 
		pts += hp_needed(j, 4);
	for (j = 50; j<ch[co].mana[0]; j++) 
		pts += mana_needed(j, 4);
	for (n = 0; n<MAXSKILL; n++) for (j = 1; j<B_SK(co, n); j++)
		pts += skill_needed(j, 3);
	ch[co].points_tot = pts;
	
	ch[co].gold   = 0;
	ch[co].a_hp   = 999999;
	ch[co].a_end  = 999999;
	ch[co].a_mana = 999999;
	
	if (ch[co].data[42] == 1100) // Map enemy
	{
		in = 0;
		rank = points2rank(ch[co].points_tot);
		
		if (rank>=18 && !RANDOM(45))
		{
			static int item[]  = {
				2515, 2519, 2523, 2527, 2531, 2536, 2540, 2544, 2548, 2552
			};
			in = RANDOM(sizeof(item) / sizeof(int)); in = item[in];
		}
		else if (rank>=15 && !RANDOM(50))
		{
			static int item[]  = {
				2514, 2518, 2522, 2526, 2530, 2535, 2539, 2543, 2547, 2551
			};
			in = RANDOM(sizeof(item) / sizeof(int)); in = item[in];
		}
		else if (rank>=12 && !RANDOM(55))
		{
			static int item[]  = {
				2513, 2517, 2521, 2525, 2529, 2534, 2538, 2542, 2546, 2550
			};
			in = RANDOM(sizeof(item) / sizeof(int)); in = item[in];
		}
		else if (!RANDOM(60))
		{
			static int item[]  = {
				2512, 2516, 2520, 2524, 2528, 2533, 2537, 2541, 2545, 2549
			};
			in = RANDOM(sizeof(item) / sizeof(int)); in = item[in];
		}
		
		if (in)
		{
			in = god_create_item(in);
			god_give_char(in, co);
			chlog(co, "got a %s", it[in].name);
		}
		
		ch[co].data[29] = xx + yy * MAPX;
		ch[co].data[30] = ch[co].dir = 1+RANDOM(8);
		if (!ch[co].data[72]) ch[co].data[72] = 1;
		if (!RANDOM(6)) ch[co].light_bonus = RANDOM(4)*25+25;
		switch (RANDOM(4))
		{
			case  1:	strcpy(ch[co].text[0], "An end to your misery, %s!"); break;
			case  2:	strcpy(ch[co].text[0], "Took you long enough, %s!"); break;
			case  3:	strcpy(ch[co].text[0], "It's about time! Goodbye, %s!"); break;
			default:	strcpy(ch[co].text[0], "Yes! Die, %s!"); break;
		}
		switch (RANDOM(4))
		{
			case  1:	strcpy(ch[co].text[1], "Something to do! Let's duel to the end, %s!"); break;
			case  2:	strcpy(ch[co].text[1], "I hope you know what you're doing, %s!"); break;
			case  3:	strcpy(ch[co].text[1], "A worthy foe! Let us shake the void, %s!"); break;
			default:	strcpy(ch[co].text[1], "Yahoo! An enemy! Prepare to die, %s!"); break;
		}
		switch (RANDOM(4))
		{
			case  1:	strcpy(ch[co].text[3], "Alas, I've lost. Good fight, %s!"); break;
			case  2:	strcpy(ch[co].text[3], "Seems this is the end for me. So long, %s."); break;
			case  3:	strcpy(ch[co].text[3], "Great fight, %s. I suppose this is goodbye."); break;
			default:	strcpy(ch[co].text[3], "Thank you %s! Everything is better than being here."); break;
		}
		
		
	}
	
	do_update_char(co);
	
	return co;
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

	return 1;
}


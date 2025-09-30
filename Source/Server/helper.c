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
static char  isinfrapot = 0;
static char  withoutcn = 0;

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
	
	if (ismonster && isinfrapot)
	{
		if ((map[m].flags & MF_SIGHTBLOCK) && (map[m].flags & MF_MOVEBLOCK) || (map[m].flags & MF_NOMONST))
		{
			return 0;
		}
	}
	else if (ismonster)
	{
		if (map[m].flags & (MF_SIGHTBLOCK | MF_NOMONST))
		{
			return 0;
		}
	}
	else if (isinfrapot || withoutcn)
	{
		if ((map[m].flags & MF_SIGHTBLOCK) && (map[m].flags & MF_MOVEBLOCK))
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
		withoutcn = 0;
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
			if ((((ch[(cn)].flags & CF_IGN_SB) != 0) || IS_MONSTER(cn)) && IS_IN_ZRAK(ch[cn].x, ch[cn].y))
			{
				isinfrapot = 1;
			}
			else
			{
				isinfrapot = 0;
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
			isinfrapot = 0;
			isbuilding = 0;
			withoutcn = 1;
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
				if (IS_IN_DW(M2X(m), M2Y(m)))
				{
					if  (map[m].flags & MF_TOUCHED) d = 200;
					else continue;
				}
				else
				{
					d = 256 / (v * (abs(xc - x) + abs(yc - y)));
				}
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
	
	// Pandium's arena center
	if (xc==PANDI_MIDX && yc==PANDI_MIDY)
	{
		map[xc + yc * MAPX].dlight = 255;
	}

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
					if ((v = it[in].light[I_A])!=0)
					{
						do_add_light(x, y, -v);
					}
				}
				else
				{
					if ((v = it[in].light[I_I])!=0)
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
					if ((v = it[in].light[I_A])!=0)
					{
						do_add_light(x, y, v);
					}
				}
				else
				{
					if ((v = it[in].light[I_I])!=0)
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
	if (v<     1750)	return  1; // Private First Class
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

// 1!3 =   168 x 8 =  1344/2 =  672
// 2!5 =   924 x 4 =  3696/2 = 1848
// 3!7 =  3432 x 2 =  6864/2 = 3432
// 4!9 = 10010 x 1 = 10010/2 = 5005

int getitemrank(int in, int v)
{
	return points2rank(it[in].cost*250/max(1,v));
}

int getrank(int cn)
{
	return points2rank(ch[cn].points_tot);
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
	return (getrank(co) - getrank(cn));
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

	diff = co_rank - getrank(cn);

	diff += RANKS;
	if (diff<0)
	{
		diff = 0;
	}
	if (diff>RANKS*2)
	{
		diff = RANKS*2;
	}

//      xlog("scale %d to %d: diff=%d, scale=%f",getrank(cn),co_rank,diff,scale_tab[diff]);

	return((int)(exp * scale_tab[diff]));

}

int scale_exps(int cn, int co, int exp)
{
	return(scale_exps2(cn, getrank(co), exp));
}

char *class_name[9] = {
	"Seyan'du",		"Arch-Templar", 
	"Skald",		"Warrior", 
	"Sorcerer",		"Summoner", 
	"Arch-Harakim",	"Braver", 
	"Lycanthrope"
};

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

int st_learned_skill(int st_val, int v)
{	// Returns the bitwise value of the parsed value v from the input value st_val
	return ( (st_val>>((12-v)+4))%2 );
}

int st_skill_pts_have(int st_val)
{	// Returns the available skill points
	return ( st_val%16 );
}

int st_skill_pts_all(int st_val)
{	// Returns the available skill points plus spent skill points
	int n, m = 0;
	for (n=0;n<12;n++)
	{
		m += st_learned_skill(st_val, n+1);
	}
	return ( m + st_skill_pts_have(st_val) );
}

int st_skillnum(int cn, int v, int n)
{
	int co;
	
	if (v <   1) return 0;
	if (n >= 12) return 0;
	
	if (IS_COMP_TEMP(cn) && IS_SANECHAR(co = ch[cn].data[CHD_MASTER]) && ch[cn].data[1]==4)
		cn = co;
	
	if (n <   0)
	{
		if (ch[cn].tree_node[v-1] == 0 || n == -1)
			return st_learned_skill(ch[cn].tree_points, v);
		return 0;
	}
	
	if (st_learned_skill(ch[cn].tree_points, n+1) && ch[cn].tree_node[n] == v)
		return 1;
	
	return 0;
}

int st_skillcount(int cn, int v)
{
	int co, n, in, count = 0;
	
	if (!IS_SANEPLAYER(cn)) return 0;
	
	if (IS_PLAYER_SC(cn) && IS_SANECHAR(co = ch[cn].data[CHD_MASTER]) && ch[cn].data[1]==4)
		cn = co;
	
	for (n=0; n<12; n++) count += st_skillnum(cn, v, n);
	
	if (IS_PLAYER_GC(cn) && IS_SANECHAR(co = ch[cn].data[CHD_MASTER]) && T_SUMM_SK(co, 10))
		cn = co;
	
	if ((in = ch[cn].worn[WN_CHARM])  && (it[in].flags & IF_CORRUPTED) && it[in].corruption == v) count++;
	if ((in = ch[cn].worn[WN_CHARM2]) && (it[in].flags & IF_CORRUPTED) && it[in].corruption == v) count++;
	if (IS_SINBINDER(in = ch[cn].worn[WN_LRING]) && (it[in].flags & IF_CORRUPTED) && it[in].corruption == v) count++;
	if (IS_SINBINDER(in = ch[cn].worn[WN_RRING]) && (it[in].flags & IF_CORRUPTED) && it[in].corruption == v) count++;
	if ((n = ch[cn].worn[WN_LRING]) && (it[n].temp==IT_SIGN_SYMM) && IS_SINBINDER(in = ch[cn].worn[WN_RRING]) && (it[in].flags & IF_CORRUPTED) && it[in].corruption == v) count++;
	if ((n = ch[cn].worn[WN_RRING]) && (it[n].temp==IT_SIGN_SYMM) && IS_SINBINDER(in = ch[cn].worn[WN_LRING]) && (it[in].flags & IF_CORRUPTED) && it[in].corruption == v) count++;
	
	return count;
}

int get_best_worn(int cn, int v)
{
	int n = -1000, in;
	
	in = ch[cn].worn[v];
	
	if (in)
	{
		switch (v)
		{
			case WN_HEAD:
				if (it[in].attrib[AT_STR][I_R]>=it[in].attrib[AT_INT][I_R]) 
					n = get_best_armor(cn, 1);
				else 
					n = get_best_armor(cn, 3);
				break;
			case WN_BODY:
				if (it[in].attrib[AT_STR][I_R]>=it[in].attrib[AT_INT][I_R]) 
					n = get_best_armor(cn, 2);
				else 
					n = get_best_armor(cn, 4);
				break;
			case WN_RHAND:
				if (IS_WPSPEAR(in))
					n = get_best_weapon(cn, 7);
				else if (IS_WPGAXE(in))
					n = get_best_weapon(cn, 10);
				else if (IS_WPDAGGER(in))
					n = get_best_weapon(cn, SK_DAGGER);
				else if (IS_WPSWORD(in))
					n = get_best_weapon(cn, SK_SWORD);
				else if (IS_WPAXE(in))
					n = get_best_weapon(cn, SK_AXE);
				else if (IS_WPSTAFF(in))
					n = get_best_weapon(cn, SK_STAFF);
				else if (IS_WPTWOHAND(in))
					n = get_best_weapon(cn, SK_TWOHAND);
				else if (IS_WPSHIELD(in))
					n = get_best_weapon(cn, SK_SHIELD);
				else 
					n = get_best_weapon(cn, SK_HAND);
				break;
			case WN_LHAND:
				if (IS_WPDUALSW(in))
					n = get_best_weapon(cn, SK_DUAL);
				else if (IS_WPDAGGER(in))
					n = get_best_weapon(cn, SK_DAGGER);
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
				if (it_temp[z].skill[SK_HAND][I_R]>B_SK(cn, SK_HAND)) continue;
				for (m=0;m<5;m++)
				{
					if (it_temp[z].attrib[m][I_R]>B_AT(cn, m)) break;
				}
				if (m==5) { in = z; break; }
			}
			break;
		case SK_DAGGER:
			for (n=0;n<6;n++)
			{
				z = w_dagger[n];
				if (it_temp[z].skill[SK_DAGGER][I_R]>B_SK(cn, SK_DAGGER)) continue;
				for (m=0;m<5;m++)
				{
					if (it_temp[z].attrib[m][I_R]>B_AT(cn, m)) break;
				}
				if (m==5) { in = z; break; }
			}
			break;
		case SK_SWORD:
			for (n=0;n<6;n++)
			{
				z = w_sword[n];
				if (it_temp[z].skill[SK_SWORD][I_R]>B_SK(cn, SK_SWORD)) continue;
				for (m=0;m<5;m++)
				{
					if (it_temp[z].attrib[m][I_R]>B_AT(cn, m)) break;
				}
				if (m==5) { in = z; break; }
			}
			break;
		case SK_AXE:
			for (n=0;n<6;n++)
			{
				z = w_axe[n];
				if (it_temp[z].skill[SK_AXE][I_R]>B_SK(cn, SK_AXE)) continue;
				for (m=0;m<5;m++)
				{
					if (it_temp[z].attrib[m][I_R]>B_AT(cn, m)) break;
				}
				if (m==5) { in = z; break; }
			}
			break;
		case SK_STAFF:
			for (n=0;n<6;n++)
			{
				z = w_staff[n];
				if (it_temp[z].skill[SK_STAFF][I_R]>B_SK(cn, SK_STAFF)) continue;
				for (m=0;m<5;m++)
				{
					if (it_temp[z].attrib[m][I_R]>B_AT(cn, m)) break;
				}
				if (m==5) { in = z; break; }
			}
			break;
		case SK_TWOHAND:
			for (n=0;n<6;n++)
			{
				z = w_twoh[n];
				if (it_temp[z].skill[SK_TWOHAND][I_R]>B_SK(cn, SK_TWOHAND)) continue;
				for (m=0;m<5;m++)
				{
					if (it_temp[z].attrib[m][I_R]>B_AT(cn, m)) break;
				}
				if (m==5) { in = z; break; }
			}
			break;
		case 7: // Spear
			for (n=0;n<6;n++)
			{
				z = w_spear[n];
				if (it_temp[z].skill[SK_DAGGER][I_R]>B_SK(cn, SK_DAGGER)) continue;
				if (it_temp[z].skill[SK_STAFF][I_R]>B_SK(cn, SK_STAFF)) continue;
				for (m=0;m<5;m++)
				{
					if (it_temp[z].attrib[m][I_R]>B_AT(cn, m)) break;
				}
				if (m==5) { in = z; break; }
			}
			break;
		case 10: // Greataxe
			for (n=0;n<6;n++)
			{
				z = w_great[n];
				if (it_temp[z].skill[SK_AXE][I_R]>B_SK(cn, SK_AXE)) continue;
				if (it_temp[z].skill[SK_TWOHAND][I_R]>B_SK(cn, SK_TWOHAND)) continue;
				for (m=0;m<5;m++)
				{
					if (it_temp[z].attrib[m][I_R]>B_AT(cn, m)) break;
				}
				if (m==5) { in = z; break; }
			}
			break;
		case SK_SHIELD:
			for (n=0;n<6;n++)
			{
				z = w_shield[n];
				if (it_temp[z].skill[SK_SHIELD][I_R]>B_SK(cn, SK_SHIELD)) continue;
				for (m=0;m<5;m++)
				{
					if (it_temp[z].attrib[m][I_R]>B_AT(cn, m)) break;
				}
				if (m==5) { in = z; break; }
			}
			break;
		case SK_DUAL:
			for (n=0;n<6;n++)
			{
				z = w_dsword[n];
				if (it_temp[z].skill[SK_DUAL][I_R]>B_SK(cn, SK_DUAL)) continue;
				for (m=0;m<5;m++)
				{
					if (it_temp[z].attrib[m][I_R]>B_AT(cn, m)) break;
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
					if (it_temp[z].attrib[m][I_R]>B_AT(cn, m)) break;
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
					if (it_temp[z].attrib[m][I_R]>B_AT(cn, m)) break;
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
					if (it_temp[z].attrib[m][I_R]>B_AT(cn, m)) break;
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
					if (it_temp[z].attrib[m][I_R]>B_AT(cn, m)) break;
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
		IT_POP_ASPEL, IT_POP_ASPEL+1, IT_POP_ASPEL+2, IT_POP_ASPEL+3, IT_POP_ASPEL+4, IT_POP_ASPEL+5, IT_POP_ASPEL+6, 
		IT_POP_ISPEL, IT_POP_ISPEL+1, IT_POP_ISPEL+2, IT_POP_ISPEL+3, IT_POP_ISPEL+4, IT_POP_ISPEL+5, IT_POP_ISPEL+6, 
		IT_POP_SSPEL, IT_POP_SSPEL+1, IT_POP_SSPEL+2, IT_POP_SSPEL+3, IT_POP_SSPEL+4, IT_POP_SSPEL+5, IT_POP_SSPEL+6, 
		
		IT_EXPS, IT_LUKS, IT_RD_HP, IT_RD_MP, 2475,
		
		2120, 2121, 2122, 2488, 2923, 2924, 2962, 
		1928, 1929, 1930, 1931, 1932, 1933, 1934, 
		
		IT_RD_BRV, IT_RD_WIL, IT_RD_INT, IT_RD_AGL, IT_RD_STR,
		IT_OS_BRV, IT_OS_WIL, IT_OS_INT, IT_OS_AGL, IT_OS_STR,
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
		IT_SILV_RING, 
		1378, 1384, 1390, 1379, 1385, 1391, 1380, 1386, 1392, 
		1381, 1387, 1393, 1382, 1388, 1394, 1383, 1389, 1395, 
		2073, 2074, 2075, 2076, 
		
		IT_GOLD_RING, 
		1396, 1402, 1408, 1397, 1403, 1409, 1398, 1404, 1410, 
		1399, 1405, 1411, 1400, 1406, 1412, 1401, 1407, 1413, 
		2077, 2081, 2078, 2082, 2079, 2083, 2080, 2084, 
		2085, 2086, 2689, 
		
		IT_PLAT_RING, 
		1414, 1420, 1426, 1415, 1421, 1427, 1416, 1422, 1428, 
		1417, 1423, 1429, 1418, 1424, 1430, 1419, 1425, 1431, 
		2087, 2091, 2097, 2088, 2092, 2098, 2089, 2093, 2099, 
		2090, 2094, 2100, 2095, 2101, 2102, 2690, 2691, 
		
		1272, 1273, 2399, 2381, 2103, 2482, 2483,
		
		3343, 3344, 3345, 
		3346, 3347, 3348, 3349, 3350, 3351, 3352, 3353, 3354, 
		
		3355, 
		3425, 3426, 3427, 3428, 3429, 3430, 3431, 3432, 3433,
		
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

int get_nullandvoid(int n)
{
	int t;
	
	t = time(NULL) / (60*60*2);
	
	if (t % 2)
	{
		if (n)
			return CT_NULLAN;
		else
			return CT_DVOID;
	}
	else
	{
		if (!n)
			return CT_NULLAN;
		else
			return CT_DVOID;
	}
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
			else if (IS_ANY_HARA(cn) || IS_SORCERER(cn))
				in = get_best_weapon(cn, 7); // 1. Spear
			else if (IS_ANY_MERC(cn) || IS_BRAVER(cn))
				in = get_best_weapon(cn, SK_SWORD); // 1. Sword
			else if (IS_LYCANTH(cn))
				in = get_best_weapon(cn, SK_HAND); // 1. Claw
			else
				in = 0;
			break;
		// Steel twohander, change to appropriate 2nd skill item
		case IT_THSW_STEL:
			if (IS_ANY_TEMP(cn))
				in = get_best_weapon(cn, SK_TWOHAND); // 2. Twohander
			else if (IS_ANY_HARA(cn) || IS_SORCERER(cn))
				in = get_best_weapon(cn, SK_STAFF); // 2. Staff
			else if (IS_ANY_MERC(cn))
				in = get_best_weapon(cn, SK_SHIELD); // 2. Shield
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
			else if (IS_SORCERER(cn))
				in = get_best_weapon(cn, SK_SWORD); // 4. Staff
			else if (IS_ANY_HARA(cn) || IS_MERCENARY(cn))
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
			else if (IS_SORCERER(cn))
				in = get_best_weapon(cn, SK_SHIELD); // 5. Shield
			else
				in = 0;
			break;
		// Steel sword, change to appropriate 6th skill item
		case IT_DUAL_STEL:
			if (IS_WARRIOR(cn) || IS_SORCERER(cn))
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
			in += t % 19;
			if (in>=IT_WP_LAMEDARG) in++;
			break;
		case IT_WP_VOLCANF: in--;
			t = time(NULL) / (60*60*24);
			in -= t % 19;
			break;
		case IT_WP_SUNSWORD:
			t = time(NULL) / (60*60*24);
			t = t % 12;
			switch (t)
			{
				case  1: in = 2005; break;
				case  2: in = 3202; break;
				case  3: in = 2006; break;
				case  4: in = 3203; break;
				case  5: in = 2017; break;
				case  6: in = 3204; break;
				case  7: in = 2000; break;
				case  8: in = 3205; break;
				case  9: in = 2011; break;
				case 10: in = 3206; break;
				case 11: in = 2012; break;
				default: break;
			}
			break;
		case 1985:
			t = time(NULL) / (60*60*24);
			in += t % 15;
			break;
		case 1793:
			t = time(NULL) / (60*60*24);
			in += t % 2;
			if (in == 1796) in = 2114;
			break;
		case 1928:
			t = time(NULL) / (60*60*24);
			in += t % 7;
			break;
		case  186:
		case 2511:
			t = time(NULL) / (60*60*24);
			in -= t % 5;
			break;
		case 3425:
			t = time(NULL) / (60*60*24);
			in += t % 9;
			break;
		
		default: break;
	}
	
	return in;
}

int change_xp_shop_item(int cn, int nr)
{
	int n, m = 0;
	
	for (n=0; n<50; n++) 
	{
		if (ch[cn].skill[n][0]) m++;
		if (m > nr) return n<50?n:-1;
	}
	
	return -1;
}

int get_special_item(int cn, int in, int gen_a, int gen_b, int gen_c)
{
	int in2;
	
	if (!in) return 0;
	
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
		case IT_HELM_LIZR:	case IT_BODY_LIZR:	case IT_HELM_AZUR:	case IT_BODY_AZUR:
		case IT_HELM_MIDN:	case IT_BODY_MIDN:	case IT_HELM_IVOR:	case IT_BODY_IVOR:
			in2 = create_special_item(in, gen_a, gen_b, gen_c);
			break;

		default:
			in2 = god_create_item(in);
			if (it[in2].temp == IT_EXPS)
			{
				it[in2].data[0] = 200*(getrank(cn)+1) + RANDOM(getrank(cn)+6) * 2000;
				it[in2].min_rank = max(0, getrank(cn)-3);
			}
			else if (it[in2].temp == IT_LUKS)
			{
				it[in2].data[0] = 20*(getrank(cn)+1) + RANDOM(getrank(cn)+6) * 100;
				it[in2].min_rank = max(0, getrank(cn)-3);
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
		case IT_DAGG_STEL: spr = IT_SPR_DAGG_STEL; break;
		case IT_STAF_STEL: spr = IT_SPR_STAF_STEL; break;
		case IT_SPEA_STEL: spr = IT_SPR_SPEA_STEL; break;
		case IT_SHIE_STEL: spr = IT_SPR_SHIE_STEL; break;
		case IT_SWOR_STEL: spr = IT_SPR_SWOR_STEL; break;
		case IT_DUAL_STEL: spr = IT_SPR_DUAL_STEL; break;
		case IT_AXXE_STEL: spr = IT_SPR_AXXE_STEL; break;
		case IT_THSW_STEL: spr = IT_SPR_THSW_STEL; break;
		case IT_GAXE_STEL: spr = IT_SPR_GAXE_STEL; break;
		//
		case IT_DAGG_GOLD: spr = IT_SPR_DAGG_GOLD; break;
		case IT_STAF_GOLD: spr = IT_SPR_STAF_GOLD; break;
		case IT_SPEA_GOLD: spr = IT_SPR_SPEA_GOLD; break;
		case IT_SHIE_GOLD: spr = IT_SPR_SHIE_GOLD; break;
		case IT_SWOR_GOLD: spr = IT_SPR_SWOR_GOLD; break;
		case IT_DUAL_GOLD: spr = IT_SPR_DUAL_GOLD; break;
		case IT_AXXE_GOLD: spr = IT_SPR_AXXE_GOLD; break;
		case IT_THSW_GOLD: spr = IT_SPR_THSW_GOLD; break;
		case IT_GAXE_GOLD: spr = IT_SPR_GAXE_GOLD; break;
		//
		case IT_DAGG_EMER: spr = IT_SPR_DAGG_EMER; break;
		case IT_STAF_EMER: spr = IT_SPR_STAF_EMER; break;
		case IT_SPEA_EMER: spr = IT_SPR_SPEA_EMER; break;
		case IT_SHIE_EMER: spr = IT_SPR_SHIE_EMER; break;
		case IT_SWOR_EMER: spr = IT_SPR_SWOR_EMER; break;
		case IT_DUAL_EMER: spr = IT_SPR_DUAL_EMER; break;
		case IT_AXXE_EMER: spr = IT_SPR_AXXE_EMER; break;
		case IT_THSW_EMER: spr = IT_SPR_THSW_EMER; break;
		case IT_GAXE_EMER: spr = IT_SPR_GAXE_EMER; break;
		//
		case IT_DAGG_CRYS: spr = IT_SPR_DAGG_CRYS; break;
		case IT_STAF_CRYS: spr = IT_SPR_STAF_CRYS; break;
		case IT_SPEA_CRYS: spr = IT_SPR_SPEA_CRYS; break;
		case IT_SHIE_CRYS: spr = IT_SPR_SHIE_CRYS; break;
		case IT_SWOR_CRYS: spr = IT_SPR_SWOR_CRYS; break;
		case IT_DUAL_CRYS: spr = IT_SPR_DUAL_CRYS; break;
		case IT_AXXE_CRYS: spr = IT_SPR_AXXE_CRYS; break;
		case IT_THSW_CRYS: spr = IT_SPR_THSW_CRYS; break;
		case IT_GAXE_CRYS: spr = IT_SPR_GAXE_CRYS; break;
		//
		case IT_DAGG_TITN: spr = IT_SPR_DAGG_TITN; break;
		case IT_STAF_TITN: spr = IT_SPR_STAF_TITN; break;
		case IT_SPEA_TITN: spr = IT_SPR_SPEA_TITN; break;
		case IT_SHIE_TITN: spr = IT_SPR_SHIE_TITN; break;
		case IT_SWOR_TITN: spr = IT_SPR_SWOR_TITN; break;
		case IT_DUAL_TITN: spr = IT_SPR_DUAL_TITN; break;
		case IT_AXXE_TITN: spr = IT_SPR_AXXE_TITN; break;
		case IT_THSW_TITN: spr = IT_SPR_THSW_TITN; break;
		case IT_GAXE_TITN: spr = IT_SPR_GAXE_TITN; break;
		//
		case IT_DAGG_ADAM: spr = IT_SPR_DAGG_ADAM; break;
		case IT_STAF_ADAM: spr = IT_SPR_STAF_ADAM; break;
		case IT_SPEA_ADAM: spr = IT_SPR_SPEA_ADAM; break;
		case IT_SHIE_ADAM: spr = IT_SPR_SHIE_ADAM; break;
		case IT_SWOR_ADAM: spr = IT_SPR_SWOR_ADAM; break;
		case IT_DUAL_ADAM: spr = IT_SPR_DUAL_ADAM; break;
		case IT_AXXE_ADAM: spr = IT_SPR_AXXE_ADAM; break;
		case IT_THSW_ADAM: spr = IT_SPR_THSW_ADAM; break;
		case IT_GAXE_ADAM: spr = IT_SPR_GAXE_ADAM; break;
		//
		case IT_CLAW_STEL: spr = IT_SPR_CLAW_STEL; break;
		case IT_CLAW_GOLD: spr = IT_SPR_CLAW_GOLD; break;
		case IT_CLAW_EMER: spr = IT_SPR_CLAW_EMER; break;
		case IT_CLAW_CRYS: spr = IT_SPR_CLAW_CRYS; break;
		case IT_CLAW_TITN: spr = IT_SPR_CLAW_TITN; break;
		case IT_CLAW_ADAM: spr = IT_SPR_CLAW_ADAM; break;
		//
		case IT_DAGG_DAMA: spr = IT_SPR_DAGG_DAMA; break;
		case IT_STAF_DAMA: spr = IT_SPR_STAF_DAMA; break;
		case IT_SPEA_DAMA: spr = IT_SPR_SPEA_DAMA; break;
		case IT_SHIE_DAMA: spr = IT_SPR_SHIE_DAMA; break;
		case IT_SWOR_DAMA: spr = IT_SPR_SWOR_DAMA; break;
		case IT_DUAL_DAMA: spr = IT_SPR_DUAL_DAMA; break;
		case IT_AXXE_DAMA: spr = IT_SPR_AXXE_DAMA; break;
		case IT_THSW_DAMA: spr = IT_SPR_THSW_DAMA; break;
		case IT_GAXE_DAMA: spr = IT_SPR_GAXE_DAMA; break;
		case IT_CLAW_DAMA: spr = IT_SPR_CLAW_DAMA; break;
		//
		case IT_HELM_BRNZ: spr = IT_SPR_HELM_BRNZ; break;
		case IT_BODY_BRNZ: spr = IT_SPR_BODY_BRNZ; break;
		case IT_HELM_STEL: spr = IT_SPR_HELM_STEL; break;
		case IT_BODY_STEL: spr = IT_SPR_BODY_STEL; break;
		case IT_HELM_GOLD: spr = IT_SPR_HELM_GOLD; break;
		case IT_BODY_GOLD: spr = IT_SPR_BODY_GOLD; break;
		case IT_HELM_EMER: spr = IT_SPR_HELM_EMER; break;
		case IT_BODY_EMER: spr = IT_SPR_BODY_EMER; break;
		case IT_HELM_CRYS: spr = IT_SPR_HELM_CRYS; break;
		case IT_BODY_CRYS: spr = IT_SPR_BODY_CRYS; break;
		case IT_HELM_TITN: spr = IT_SPR_HELM_TITN; break;
		case IT_BODY_TITN: spr = IT_SPR_BODY_TITN; break;
		case IT_HELM_ADAM: spr = IT_SPR_HELM_ADAM; break;
		case IT_BODY_ADAM: spr = IT_SPR_BODY_ADAM; break;
		case IT_HELM_CAST: spr = IT_SPR_HELM_CAST; break;
		case IT_BODY_CAST: spr = IT_SPR_BODY_CAST; break;
		case IT_HELM_ADEP: spr = IT_SPR_HELM_ADEP; break;
		case IT_BODY_ADEP: spr = IT_SPR_BODY_ADEP; break;
		case IT_HELM_WIZR: spr = IT_SPR_HELM_WIZR; break;
		case IT_BODY_WIZR: spr = IT_SPR_BODY_WIZR; break;
		case IT_HELM_DAMA: spr = IT_SPR_HELM_DAMA; break;
		case IT_BODY_DAMA: spr = IT_SPR_BODY_DAMA; break;
		//
		case IT_HELM_LIZR: spr = IT_SPR_HELM_LIZR; break;
		case IT_BODY_LIZR: spr = IT_SPR_BODY_LIZR; break;
		case IT_HELM_MIDN: spr = IT_SPR_HELM_MIDN; break;
		case IT_BODY_MIDN: spr = IT_SPR_BODY_MIDN; break;
		case IT_HELM_AZUR: spr = IT_SPR_HELM_AZUR; break;
		case IT_BODY_AZUR: spr = IT_SPR_BODY_AZUR; break;
		case IT_HELM_IVOR: spr = IT_SPR_HELM_IVOR; break;
		case IT_BODY_IVOR: spr = IT_SPR_BODY_IVOR; break;
		//
		default: break;
	}
	
	//xlog("get_special_spr OUT: %d", spr);
	
	return spr;
}

void remove_item_age(int in)
{
	it[in].current_damage = 0;
	it[in].max_damage = 0;
	it[in].max_age[I_I] = 0;
	it[in].max_age[I_A] = 0;
	it[in].flags |= IF_SINGLEAGE | IF_NOREPAIR;
	it[in].flags &= ~IF_CAN_SS;
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
	if (!in) return 0;
	
	it[in].temp = 0;
	
	// Go through each check for what kind of item this is
	if (temp==IT_HELM_BRNZ || temp==IT_BODY_BRNZ || temp==IT_HELM_STEL || temp==IT_BODY_STEL || 
		temp==IT_HELM_GOLD || temp==IT_BODY_GOLD || temp==IT_HELM_EMER || temp==IT_BODY_EMER || 
		temp==IT_HELM_CRYS || temp==IT_BODY_CRYS || temp==IT_HELM_TITN || temp==IT_BODY_TITN || 
		temp==IT_HELM_ADAM || temp==IT_BODY_ADAM || temp==IT_HELM_LIZR || temp==IT_BODY_LIZR || 
		temp==IT_HELM_AZUR || temp==IT_BODY_AZUR)
		is_armor = 1;
	if (temp==IT_HELM_CAST || temp==IT_BODY_CAST || temp==IT_HELM_ADEP || temp==IT_BODY_ADEP || 
		temp==IT_HELM_WIZR || temp==IT_BODY_WIZR || temp==IT_HELM_MIDN || temp==IT_BODY_MIDN || 
		temp==IT_HELM_IVOR || temp==IT_BODY_IVOR)
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
	
	if (IS_TWOHAND(in))
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
	
	if (gen_a) roll = max(0, gen_a-1);
	
	it[in].data[1] = legendary;
	it[in].data[2] = roll;
	// Pick a prefix
	switch(roll)
	{
		case  0: 
		case  1: 
			if (legendary) pref = "Leo "; else pref = "Lion's ";
			it[in].attrib[AT_BRV][I_P] += 4 * mul;
			break;
		case  2: 
		case  3: 
			if (legendary) pref = "Anguis "; else pref = "Snake's ";
			it[in].attrib[AT_WIL][I_P] += 4 * mul;
			break;
		case  4: 
		case  5: 
			if (legendary) pref = "Ibis "; else pref = "Owl's ";
			it[in].attrib[AT_INT][I_P] += 4 * mul;
			break;
		case  6: 
		case  7: 
			if (legendary) pref = "Mus "; else pref = "Weasel's ";
			it[in].attrib[AT_AGL][I_P] += 4 * mul;
			break;
		case  8: 
		case  9: 
			if (legendary) pref = "Ursa "; else pref = "Bear's ";
			it[in].attrib[AT_STR][I_P] += 4 * mul;
			break;
		//
		case 10: 
		case 11: 
			if (legendary) pref = "Angelus "; else pref = "Angelic ";
			it[in].attrib[AT_BRV][I_P] += 2 * mul;
			it[in].attrib[AT_WIL][I_P] += 2 * mul;
			it[in].attrib[AT_INT][I_P] += 2 * mul;
			it[in].attrib[AT_AGL][I_P] += 2 * mul;
			it[in].attrib[AT_STR][I_P] += 2 * mul;
			break;
		case 12: 
			if (legendary) pref = "Lux "; else pref = "Glowing "; 
			it[in].light[I_P]     += 20 * mul;
			it[in].spell_apt[I_P] +=  5 * mul;
			break;
		case 13: 
			if (legendary) pref = "Lepus "; else pref = "Rabbit's "; 
			it[in].speed[I_P] += 4 * mul;
			break;
		case 14: 
			if (legendary) pref = "Centurio "; else pref = "Centurion's "; 
			it[in].to_hit[I_P]   += 2 * mul;
			it[in].to_parry[I_P] += 2 * mul;
			break;
		case 15: 
			if (legendary) pref = "Deus "; else pref = "Godly "; 
			mul *= 2; 
			break;
		default: 
			pref = ""; 
			break;
	}
	
	roll=RANDOM(20);

	// Pick a suffix, or two if legendary
	for (n=0;n<=legendary;n++)
	{
		while (roll==prev) // avoid picking the same thing twice if legendary
		{
			roll=RANDOM(20);
		}
		m=RANDOM(2);
		
		if (n==0 && gen_b) roll = max(0, gen_b-1);
		if (n==1 && gen_c) roll = max(0, gen_c-1);
		
		it[in].data[3+n] = roll;
		switch(roll)
		{
			case  0:
			case  1:
				if (legendary) suffix = "Fortem"; else suffix = " of Braveness";
				it[in].attrib[AT_BRV][I_P] += (9 * mul)/2;
				break;
			case  2:
			case  3:
				if (legendary) suffix = "Autem"; else suffix = " of Willpower";
				it[in].attrib[AT_WIL][I_P] += (9 * mul)/2;
				break;
			case  4:
			case  5:
				if (legendary) suffix = "Intuitio"; else suffix = " of Intuition";
				it[in].attrib[AT_INT][I_P] += (9 * mul)/2;
				break;
			case  6:
			case  7:
				if (legendary) suffix = "Agilitas"; else suffix = " of Agility";
				it[in].attrib[AT_AGL][I_P] += (9 * mul)/2;
				break;
			case  8:
			case  9:
				if (legendary) suffix = "Viribus"; else suffix = " of Strength";
				it[in].attrib[AT_STR][I_P] += (9 * mul)/2;
				break;
			case 10:
			case 11:
				if (legendary) suffix = "Callidus"; else suffix = " of the Adept";
				it[in].attrib[AT_BRV][I_P] += (5 * mul)/2; 
				it[in].attrib[AT_WIL][I_P] += (5 * mul)/2; 
				it[in].attrib[AT_INT][I_P] += (5 * mul)/2; 
				it[in].attrib[AT_AGL][I_P] += (5 * mul)/2; 
				it[in].attrib[AT_STR][I_P] += (5 * mul)/2; 
				break;
			//
			case 12:
				if (legendary) suffix = "Vitale"; else suffix = " of Vitality";
				it[in].hp[I_P] += 40 * mul;
				it[in].skill[SK_REGEN][I_P] += (9 * mul)/2;
				break;
			case 13:
				if (legendary) suffix = "Vigor"; else suffix = " of Vigor";
				it[in].end[I_P] += 20 * mul;
				it[in].skill[SK_REST][I_P]  += (9 * mul)/2;
				break;
			case 14:
				if (legendary) suffix = "Spiritus"; else suffix = " of Spirits";
				it[in].mana[I_P] += 40 * mul;
				it[in].skill[SK_MEDIT][I_P] += (9 * mul)/2;
				break;
			case 15:
				if (legendary) suffix = "Impetus"; else suffix = " of Offense";
				it[in].weapon[I_P] += (7 * mul)/2;
				break;
			case 16:
				if (legendary) suffix = "Defendere"; else suffix = " of Defense";
				it[in].armor[I_P] += (7 * mul)/2;
				break;
			case 17:
				if (legendary) suffix = "Perfugium"; else suffix = " of Resisting";
				it[in].skill[SK_IMMUN][I_P]  += (7 * mul)/2;
				it[in].skill[SK_RESIST][I_P] += (7 * mul)/2;
				break;
			case 18:
				if (legendary) suffix = "Incantator"; else suffix = " of Wizardry";
				it[in].spell_mod[I_P] += (3 * mul)/2;
				break;
			case 19:
				if (legendary) suffix = "Bellum"; else suffix = " of Battle";
				it[in].to_hit[I_P]   += (5 * mul)/2;
				it[in].to_parry[I_P] += (5 * mul)/2;
				break;
			//
			default:
				if (legendary) suffix = "Null"; else suffix = " of Nothing";
				break;
		}
		prev = roll;
	}
	
	// Update the item's sprite
	spr = get_special_spr(temp, it[in].sprite[I_I]);
	
	it[in].orig_temp = temp;
	it[in].sprite[I_I]  = spr;
	remove_item_age(in);
	it[in].flags &= ~IF_CAN_EN;
	
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
	{"Weak Thief"				},	//   1
	{"Crawler"					},	//   2
	{"Weak Skeleton"			},	//   3
	{"Weak Harakim"				},	//   4
	{"Weak Outlaw"				},	//   5
	{"Skeleton"					},	//   6
	{"Weak Ghost"				},	//   7
	{"Outlaw"					},	//   8
	{"Thief"					},	//   9
	{"Creeper"					},	//  10
	//
	{"Weak Spider"				},	//  11
	{"Spider"					},	//  12
	{"Strong Spider"			},	//  13
	{"Ghost"					},	//  14
	{"Bandit"					},	//  15
	//
	{"Arachnid"					},	//  16
	{"Templar"					},	//  17
	{"Strong Skeleton"          },	//  18
	{"Tarantula"                },	//  19
	{"Strong Templar"           },	//  20
	{"Strong Harakim"           },	//  21
	{"First Lieu. Grolm"		},	//  22
	{"First Lieu. Seagrel"      },	//  23
	{"Robber"					},	//  24
	{"Very Strong Skeleton"		},	//  25
	//
	{"Strong Bandit"			},	//  26
	{"Mud Golem"				},	//  27
	{"Spectre"					},	//  28
	{"Grudge"					},	//  29
	{"Swamp Lizard"				},	//  30
	{"Viking"					},	//  31		-- 1/8 -- data[60]
	{"Wraith"					},	//  32
	{"Wight"					},	//  33
	//
	{"Lt.Colonel Ice Gargoyle"	},	//  34
	//
	{"Strong Undead"			},	//  35
	{"Living Flame"				},	//  36
	{"Colonel Ice Gargoyle"		},	//  37
	//
	{"Gargoyle Knight"			},	//  38
	{"Gargoyle Mage"			},	//  39
	//
	{"Xecko"                    },	//  40
	{"Thug"                     },	//  41
	{"Cultist"                  },	//  42
	{"Strider"                  },	//  43
	{"Lycanthrope"              },	//  44
	//
	{"Lizard Guard"             },	//  45
	{"Lizard Monk"              },	//  46
	{"Lizard Priest"            },	//  47
	//
	{"Lizard Bandit"            },	//  48
	{"Lizard Dweller"           },	//  49
	{"Lizard Retainer"          },	//  50
	{"Lizard Archmage"          },	//  51
	{"Lizard Archknight"        },	//  52
	{"Emerald Golem"            },	//  53
	//
	{"Gargoyle Queen"			},	//  54
	{"Brig. Gen Ice Gargoyle"	},	//  55
	{"Major Gen Ice Gargoyle"	},	//  56
	{"Lieu. Gen Ice Gargoyle"	},	//  57
	{"General Ice Gargoyle"		},	//  58
	{"Ice Gargoyle Queen"		},	//  59
	//
	{"Second Lieu. Thrall"		},	//  60
	{"First Lieu. Thrall"		},	//  61
	{"Captain Thrall"			},	//  62
	{"Major Thrall"				},	//  63		-- 2/8 -- data[61]
	{"Lt.Colonel Thrall"		},	//  64
	{"Colonel Vampire"			},	//  65
	{"Brig. General Vampire"	},	//  66
	{"Major General Vampire"	},	//  67
	{"Lieu. General Vampire"	},	//  68
	{"General Vampire"			},	//  69
	{"F.Marshal Vampire"		},	//  70
	//
	{"Weak Skeleton Miner"		},	//  71
	{"Skeleton Miner"			},	//  72
	{"Silver Golem"				},	//  73
	{"Magma Gargoyle"			},	//  74
	{"Marble Golem"				},	//  75
	//
	{"Grolm Soldier"			},	//  76
	{"Grolm Mage"				},	//  77
	{"Grolm King"				},	//  78
	{"Lizard Youth"				},	//  79
	{"Lizard Worker"			},	//  80
	{"Lizard Seer"				},	//  81
	{"Undead"					},	//  82
	{"Reptite"					},	//  83
	{"Spellcaster"				},	//  84
	{"Puppeteer"				},	//  85
	{"Mad Knight"				},	//  86
	{"Old Bones"				},	//  87
	{"Sand Golem"				},	//  88
	{"Pharoh"					},	//  89
	{"Undead King"				},	//  90
	{"Stone Golem"				},	//  91
	{"Librarian"				},	//  92
	{"Golemancer"				},	//  93
	{"Grolm Trapper"			},	//  94
	{"Weaver"					},	//  95		-- 3/8 -- data[62]
	{"Iguana"					},	//  96
	{"Wood Golem"				},	//  97
	{"Sculpture"				},	//  98
	{"Barbarian"				},	//  99
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
	{"Sogling Duke"				},	// 127		-- 4/8 -- data[63]
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
	{"Marquess Onyx Gargoyle"	},	// 159		-- 5/8 -- data[70]
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
	{"Lesser Scorpion"          },	// 186
	{"Scorpion"                 },	// 187
	{"Greater Scorpion"         },	// 188
	//
	{"Dire Rat"                 },	// 189
	//
	{"Ancient Gargoyle"         },	// 190
	{"Dweller"                  },	// 191		-- 6/8 -- data[93]
	{"Widow"                  	},	// 192
	{"Experiment"             	},	// 193
	{"Abomination"            	},	// 194
	//
	{"Forsaken"                 },	// 195
	{"Ninja"                    },	// 196
	{"Assassin"                 },	// 197
	{"Mahaha"                   },	// 198
	{"Ijiraq"                   },	// 199
	{"Smuggler"                 },	// 200
	//
	{"Ashling"              	},	// 201
	{"Devil" 					},	// 202
	{"Glass Gargoyle"          	},	// 203
	//
	{"Mummy"                    },	// 204
	{"Toxic Grulge"             },	// 205
	{"Kelpie"                   },	// 206
	//
	{"General Onyx Gargoyle" 	},	// 207
	{"F.Marshal Onyx Gargoyle" 	},	// 208
	//
	{"Cryskull"                 },	// 209
	{"Snow Gargoyle"            },	// 210
	//
	{"Dwarf"                    },	// 211
	{"Pirate"                   },	// 212
	{"F.Marshal Gargoyle"       },	// 213
	{"Enforcer"                 },	// 214
	{"Scaleling"                },	// 215
	{"Dragon"                   },	// 216
	//
	{"Dimling"                 	},	// 217
	{"Dimling Pawn"             },	// 218
	{"Dimling Rook"             },	// 219
	{"Dimling Jack"             },	// 220
	{"Dimling Queen"            },	// 221
	//
	{"Toxic Gargoyle"           },	// 222
	{"Vileling"               	},	// 223		-- 7/8 -- data[73]
	{"Living Armor"         	},	// 224
	{"Skyfire"              	},	// 225
	//
	{"Knight Antediluvian"      },	// 226
	{"Baron Antediluvian"       },	// 227
	{"Earl Antediluvian"        },	// 228
	{"Marquess Antediluvian"    },	// 229
	//
	{"Lizard Bones"             },	// 230
	{"Island Lizard"            },	// 231
	{"Island Shaman"            },	// 232
	//
	{"Frostling"                },	// 233
	{"Tundrake"                 },	// 234
	{"Deathlock"                },	// 235
	{"Shade"                    },	// 236
	{"Demon"                    },	// 237
	//
	{"Barnacle"                 },	// 238
	{"Sea Gargoyle"             },	// 239
	{"Sea Golem"                },	// 240
	//
	{"Buccaneer"                },	// 241
	{"Haunt"                    },	// 242
	//
	{""                     	},	// 243
	{""                     	},	// 244
	{""                     	},	// 245
	{""                     	},	// 246
	{""                     	},	// 247
	{""                     	},	// 248
	{""                     	},	// 249
	{""                     	},	// 250
	{""                     	},	// 251
	{""                     	},	// 252
	{""                     	},	// 253
	{""                     	},	// 254
	{""                     	}	// 255		-- 8/8 -- data[67]
};

int killed_class(int cn, int val)
{
	int bit, tmp;
	
	if (val > 1024) return 1;

	if (val<32)
	{
		bit = 1 << (val);
		tmp = ch[cn].data[60] & bit; ch[cn].data[60] |= bit;
		return tmp;
	}
	else if (val<64)
	{
		bit = 1 << (val - 32);
		tmp = ch[cn].data[61] & bit; ch[cn].data[61] |= bit;
		return tmp;
	}
	else if (val<96)
	{
		bit = 1 << (val - 64);
		tmp = ch[cn].data[62] & bit; ch[cn].data[62] |= bit;
		return tmp;
	}
	else if (val<128)
	{
		bit = 1 << (val - 96);
		tmp = ch[cn].data[63] & bit; ch[cn].data[63] |= bit;
		return tmp;
	}
	else if (val<160)
	{
		bit = 1 << (val - 128);
		tmp = ch[cn].data[70] & bit; ch[cn].data[70] |= bit;
		return tmp;
	}
	else if (val<192)
	{
		bit = 1 << (val - 160);
		tmp = ch[cn].data[93] & bit; ch[cn].data[93] |= bit;
		return tmp;
	}
	else if (val<224)
	{
		bit = 1 << (val - 192);
		tmp = ch[cn].data[73] & bit; ch[cn].data[73] |= bit;
		return tmp;
	}
	else
	{
		bit = 1 << (val - 224);
		tmp = ch[cn].data[67] & bit; ch[cn].data[67] |= bit;
		return tmp;
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
	
	if (ch[cn].data[20] >= nr)
	{
		finish_laby_teleport(cn, nr, 0);
		return 1;
	}

	switch (nr)
	{
		case  1: co = pop_create_char(CT_LAB_1_BOSS, 0); break; // grolms
		case  2: co = pop_create_char(CT_LAB_2_BOSS, 0); break; // lizard
		case  3: co = pop_create_char(CT_LAB_3_BOSS, 0); break; // spellcaster
		case  4: co = pop_create_char(CT_LAB_4_BOSS, 0); break; // knight
		case  5: co = pop_create_char(CT_LAB_5_BOSS, 0); break; // undead
		case  6: co = pop_create_char(CT_LAB_6_BOSS, 0); break; // desert
		case  7: co = pop_create_char(CT_LAB_7_BOSS, 0); break; // light&dark
		case  8: co = pop_create_char(CT_LAB_8_BOSS, 0); break; // underwater
		case  9: co = pop_create_char(CT_LAB_9_BOSS, 0); break; // riddle
		case 10: co = pop_create_char(CT_LAB10_BOSS, 0); break; // forest / golem
		case 11: co = pop_create_char(CT_LAB11_BOSS, 0); break; // seasons
		case 12: co = pop_create_char(CT_LAB12_BOSS, 0); break; // ascent
		case 13: co = pop_create_char(CT_LAB14_BOSS, 0); break; // miners
		case 14: co = pop_create_char(CT_LAB15_BOSS, 0); break; // vantablack
		case 15: co = pop_create_char(CT_LAB16_BOSS, 0); break; // pirates
		case 16: co = pop_create_char(CT_LAB17_BOSS, 0); break; // gargoyles
		case 17: co = pop_create_char(CT_LAB18_BOSS, 0); break; // commandment
		case 18: co = pop_create_char(CT_LAB19_BOSS, 0); break; // divinity
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

	ch[co].data[64] = globs->ticker + 5 * 60 * TICKS; // die in five minutes if not otherwise
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
		god_transfer_char(cn, ch[cc].x, ch[cc].y);
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

	hour = globs->mdtime / (MD_MIN * 60);
	minute = (globs->mdtime / MD_MIN) % 60;
	day = globs->mdday % 28 + 1;
	month = globs->mdday / 28 + 1;
	year  = globs->mdyear + GAMEYEAR;

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

// New Soulstones
// .stack is the current total power of the stone, for merging & display purposes
// .data[0] is the catalyzed stat, if applicable. Set to -1 if unused.
// IF_SOULSTONE is if this stone has been catalyzed.
// IF_ENCHANTED is if this stone has been focused, causing items to require repair.

#define SS_CAP        4
#define SS_FOCUS_CAP  6
#define SS_STAT_CAP   2

void add_ss_stat(int cn, int in, int c[MAXSKILL], int v, int n)
{
	int early_ss_list[37] = { 
		SK_HAND,     SK_DAGGER,  SK_SWORD,      SK_AXE,      SK_STAFF,   SK_TWOHAND,
		SK_STEALTH,  SK_PERCEPT, SK_METABOLISM, SK_MSHIELD,  SK_ECONOM,  SK_REPAIR,
		SK_SHIELD,   SK_PROTECT, SK_ENHANCE,    SK_SLOW,     SK_CURSE,   SK_BLESS,
		SK_RESIST,   SK_BLAST,   SK_DISPEL,     SK_HEAL,     SK_GHOST,   SK_REGEN, 
		SK_REST,     SK_MEDIT,   SK_IMMUN,      SK_SURROUND, SK_TACTICS, SK_BLIND,
		SK_GEARMAST, SK_CLEAVE,  SK_WEAKEN,     SK_POISON,   SK_HASTE,   SK_TAUNT };
	int m, sk = 0;
	
	for (m=0;m<99;m++)
	{
		if (cn)
		{
			if (!IS_PLAYER(cn))
			{
				if (getrank(cn)<12)
					sk = early_ss_list[RANDOM(37)];
				else
					sk = RANDOM(MAXSKILL);
			}
			else
				sk = c[RANDOM(v)];
		}
		else
			sk = RANDOM(MAXSKILL);
		
		if (it[in].skill[sk][n] < SS_STAT_CAP || n == I_P) // I_P is adding directly to an item, via the Fool card.
		{
			it[in].skill[sk][n]++;
			break;
		}
	}
	
}

void randomize_ss_stats(int cn, int in, int rank, int sk)
{
	int n, v = 0;
	int c[MAXSKILL] = {0};
	
	it[in].flags |= IF_IDENTIFIED;
	
	for (n=0;n<MAXSKILL;n++)
	{
		it[in].skill[n][I_I] = 0;
		if (IS_PLAYER(cn) && B_SK(cn, n))
		{
			c[v++] = n;
		}
	}
	
	if (sk>=0 && sk<MAXSKILL)
	{
		n = min(SS_STAT_CAP, rank);
		it[in].skill[sk][I_I] = n;
		rank -= n;
	}
	
	if (rank<1) return;
	
	for (n=0;n<rank;n++)
	{
		add_ss_stat(cn, in, c, v, I_I);
	}
}

int make_new_ss(int cn, int rank, int sk)
{
	int in;
	
	if (!rank) rank = getrank(cn);
	
	if (rank>=24)                  rank = 4;             // Warlord+
	else if (rank<=23 && rank>=20) rank = 3 + RANDOM(2); // Noble
	else if (rank==19)             rank = 3;             // FDM
	else if (rank<=18 && rank>=15) rank = 2 + RANDOM(2); // General
	else if (rank<=14 && rank>=13) rank = 2;             // Colonel
	else if (rank<=12 && rank>= 9) rank = 1 + RANDOM(2); // Officer
	else                           rank = 1;             // Low
	
	if (!(in = god_create_item(IT_SOULSTONE)))
	{
		chlog(cn, "ERROR in make_new_ss: god_create_item failure");
		return;
	}
	
	it[in].stack   = rank = max(rank, 1);
	
	sprintf(it[in].name, "Soulstone");
	sprintf(it[in].reference, "soulstone");
	sprintf(it[in].description, "A level %d soulstone.", rank);
	
	it[in].temp    = 0;
	it[in].driver  = 68;
	
	randomize_ss_stats(cn, in, rank, (it[in].data[0] = sk));
	
	return in;
}

void give_new_ss(int cn, int rank)
{
	int in;
	
	in = make_new_ss(cn, rank, -1);
	
	god_give_char(in, cn);
}

int merge_new_soulstones(int cn, int in, int in2)
{
	int rank;
	
	rank = min(max(it[in].stack, it[in2].stack), SS_CAP);
	
	it[in].stack  = rank = max(rank, 1);
	
	sprintf(it[in].description, "A level %d soulstone.", rank);
	do_char_log(cn, 7, "A new level %d stone was created.\n", rank);
	
	randomize_ss_stats(cn, in, rank, -1);
	
	use_consume_item(cn, in2, 0);
	
	return 1;
}

int apply_new_catalyst(int cn, int in, int in2) // in2 is the catalyst
{
	int rank, sk;
	
	rank = min(max(it[in].stack, 1), SS_FOCUS_CAP);
	
	sk = it[in2].data[4]-1;
	
	do_char_log(cn, 6, "The stone was rerolled with %s.\n", skilltab[sk].name);
	
	randomize_ss_stats(cn, in, rank, (it[in].data[0] = sk));
	it[in].flags |= IF_SOULSTONE | IF_UPDATE;
	
	use_consume_item(cn, in2, 1);
	
	return 1;
}

int make_new_catalyst(int cn, int n, int v)
{
	int early_ss_list[37] = { 
		SK_HAND,     SK_DAGGER,  SK_SWORD,      SK_AXE,      SK_STAFF,   SK_TWOHAND,
		SK_STEALTH,  SK_PERCEPT, SK_METABOLISM, SK_MSHIELD,  SK_ECONOM,  SK_REPAIR,
		SK_SHIELD,   SK_PROTECT, SK_ENHANCE,    SK_SLOW,     SK_CURSE,   SK_BLESS,
		SK_RESIST,   SK_BLAST,   SK_DISPEL,     SK_HEAL,     SK_GHOST,   SK_REGEN, 
		SK_REST,     SK_MEDIT,   SK_IMMUN,      SK_SURROUND, SK_TACTICS, SK_BLIND,
		SK_GEARMAST, SK_CLEAVE,  SK_WEAKEN,     SK_POISON,   SK_HASTE,   SK_TAUNT };
	int in;
	
	if (!(in = god_create_item(IT_SOULCATAL)))
	{
		chlog(cn, "ERROR in make_new_catalyst: god_create_item failure");
		return 0;
	}
	
	if (!v)
	{
		if (!IS_ANY_ARCH(cn) || (IS_MONSTER(cn) && getrank(cn)<12))
			v = early_ss_list[RANDOM(37)];
		else
			v = RANDOM(MAXSKILL);
	}
	
	sprintf(it[in].name, "Soul Catalyst (%s)", skilltab[v].name);
	sprintf(it[in].reference, "soul catalyst (%s)", skilltab[v].name);
	sprintf(it[in].description, "A soul catalyst. Can be used on a soulstone to grant it static properties.");
	
	it[in].temp          = 0;
	it[in].driver        = 93;
	it[in].data[4]       = v + 1;
	it[in].flags        |= IF_IDENTIFIED | IF_STACKABLE;
	it[in].skill[v][I_I] = SS_STAT_CAP;
	it[in].stack         = min(max(1, n), 10);
	
	if (!god_give_char(in, cn))
	{
		it[in].used = USE_EMPTY;
		return 0;
	}
	
	return in;
}

int apply_new_focus(int cn, int in, int in2) // in2 is the focus
{
	int n, m = 0, v = 0, rank;
	int c[MAXSKILL] = {0};
	
	m = it[in].stack;
	
	rank = min(it[in].stack+SS_STAT_CAP, SS_FOCUS_CAP);
	
	m = rank - m;
	
	if (m<=0)
	{
		do_char_log(cn, 1, "Nothing happened.\n");
		return 0;
	}
	
	it[in].stack  = rank = max(rank, 1);
	it[in].flags |= IF_ENCHANTED | IF_UPDATE;
	
	sprintf(it[in].description, "A volatile level %d soulstone.", rank);
	do_char_log(cn, 7, "The stone became volatile, with a new level of %d.\n", rank);
	
	for (n=0;n<MAXSKILL;n++)
	{
		if (IS_PLAYER(cn) && B_SK(cn, n))
		{
			c[v++] = n;
		}
	}
	
	for (n=0;n<m;n++)
		add_ss_stat(cn, in, c, v, I_I);
	
	use_consume_item(cn, in2, 1);
	
	return 1;
}

int make_new_focus(int cn)
{
	int in;
	
	if (!(in = god_create_item(IT_SOULFOCUS)))
	{
		chlog(cn, "ERROR in make_new_focus: god_create_item failure");
		return;
	}
	
	sprintf(it[in].name, "Soul Focus");
	sprintf(it[in].reference, "soul focus");
	sprintf(it[in].description, "A soul focus. Can be used on a soulstone to increase its level by %d, but may cause volatile side effects.", SS_STAT_CAP);
	
	it[in].temp = 0;
	it[in].driver = 92;
	
	god_give_char(in, cn);
}

int apply_new_ss(int cn, int in, int in2, int msg) // in2 is the soulstone
{
	int n, rank, fool, v = 0, temp, bonus = 1, vlt = 0;
	int c[MAXSKILL] = {0};
	
	rank = it[in2].stack;
	
	if (IS_TWOHAND(in)) bonus = 2;
	if (it[in2].flags & IF_ENCHANTED) vlt = 1;
	
	// loop through stats on the soulstone and apply them to the item.
	for (n=0;n<MAXSKILL;n++)
	{
		it[in].skill[n][I_P] += it[in2].skill[n][I_I]*bonus;
	}
	
	if (it[in].temp==IT_CH_FOOL)
	{
		fool = rank*2;
		
		for (n=0;n<MAXSKILL;n++)
		{
			if (IS_PLAYER(cn) && B_SK(cn, n))
			{
				c[v++] = n;
			}
		}
		
		for (n=0;n<fool;n++)
		{
			add_ss_stat(cn, in, c, v, I_P);
		}
	}
	
	// Repair the transformed item
	it[in].orig_temp = temp = it[in].temp;
	
	it[in].current_damage = it[in].current_age[I_I] = it[in].current_age[I_A] = it[in].damage_state = 0;
	
	if (temp)
	{
		it[in].armor[I_I]  = it_temp[temp].armor[I_I];  it[in].armor[I_A]  = it_temp[temp].armor[I_A];
		it[in].weapon[I_I] = it_temp[temp].weapon[I_I];	it[in].weapon[I_A] = it_temp[temp].weapon[I_A];
		
		if (NOT_SINBINDER(in))
			it[in].sprite[I_I] = it_temp[temp].sprite[I_I]; it[in].sprite[I_A] = it_temp[temp].sprite[I_A];
	}
	
	// Finalize the item
	sprintf(it[in].description, "A %s enhanced by a level %d soulstone.", it[in].name, rank);
	it[in].flags &= ~IF_CAN_SS;
	it[in].flags |= IF_UPDATE | IF_IDENTIFIED | IF_SOULSTONE;
	it[in].value -= 1;
	it[in].power += rank * 5 + max(0, (rank-1) * 5) + max(0, (rank-2) * 5) + max(0, (rank-3) * 5) + max(0, (rank-4) * 5) + max(0, (rank-5) * 5);
	
	if (vlt && !HAS_ENCHANT(in, 34))
	{
		if (msg && (it[in].flags & IF_NOREPAIR))
			do_char_log(cn, 7, "You enhanced the %s with a level %d soulstone. Due to the volatile nature of the stone, it lost its resistance to aging.\n", it[in].name, rank);
		it[in].flags &= ~IF_NOREPAIR;
		if (it[in].flags & IF_WEAPON) it[in].max_damage = it[in].power * 5000;
		else                          it[in].max_damage = it[in].power * 1250;
		
	}
	else if (msg)
		do_char_log(cn, 7, "You enhanced the %s with a level %d soulstone.\n", it[in].name, rank);
	
	use_consume_item(cn, in2, 0);
	
	return 1;
}

void create_new_ss_equipment(int cn, int in, int rank, int sk)
{
	int in2;
	
	if (!CAN_SOULSTONE(in)) return;
	
	in2 = make_new_ss(cn, rank, sk);
	
	apply_new_ss(cn, in, in2, 0);
}

int make_gskill(int cn)
{
	int in, v;
	
	if (!(in = god_create_item(IT_OS_SK)))
	{
		chlog(cn, "ERROR in make_gskill: god_create_item failure");
		return 0;
	}
	
	v = RANDOM(MAXSKILL);
	
	sprintf(it[in].name, "Greater Scroll of (%s)", skilltab[v].name);
	sprintf(it[in].reference, "greater scroll of (%s)", skilltab[v].name);
	
	it[in].data[1] = v;
	
	if (!god_give_char(in, cn))
	{
		it[in].used = USE_EMPTY;
		return 0;
	}
	
	return in;
}

void make_talisfrag(int cn, int n)
{
	int in;
	
	if (!(in = god_create_item(IT_TALISFRAG)))
	{
		chlog(cn, "ERROR in make_talisfrag: god_create_item failure");
		return;
	}
	
	it[in].stack = max(1, n);
	
	god_give_char(in, cn);	// chlog(cn, "got talisman fragment(s)");
}

void make_corruptor(int cn, int n)
{
	int in;
	
	if (!(in = god_create_item(IT_CORRUPTOR)))
	{
		chlog(cn, "ERROR in make_corruptor: god_create_item failure");
		return;
	}
	
	it[in].stack = max(1, n);
	
	god_give_char(in, cn);
	
	return in;
}

void create_random_talisman_equipment(int cn, int in2)
{
	int in;
	
	if (!CAN_ENCHANT(in2)) return;
	
	in = god_create_item(IT_TALISMAN);
	if (in)
	{
		it[in].data[0] = RANDOM(91)+1;
	}
	
	use_talisman(cn, in, in2);
}

int set_enchantment(int cn, int v)
{
	int in;
	in = ch[cn].citem;
	
	if (!in)
	{
		do_char_log(cn, 1, "You need an item for that.\n");
		return 0;
	}
	
	it[in].enchantment = v;
	
	if (v)
	{
		it[in].flags |= IF_UPDATE | IF_IDENTIFIED | IF_ENCHANTED | IF_LOOKSPECIAL;
		it[in].flags &= ~IF_CAN_EN;
	}
	else
	{
		it[in].flags |= IF_UPDATE | IF_IDENTIFIED | IF_CAN_EN;
		it[in].flags &= ~IF_ENCHANTED;
	}
	
	return 1;
}

int use_talisman(int cn, int in, int in2)
{	// [in] is talisman, [in2] is the target item
	int r, n, temp, inds = 0;
	int stk, val, mul = 1;
	
	if (!IS_SANECHAR(cn))	return 0;
	if (!IS_SANEITEM(in))	return 0;
	if (!in2)
	{
		do_char_log(cn, 1, "Try using something with the talisman. Click on it with an item under your cursor.\n");
		return 0;
	}
	if (!IS_SANEITEM(in2))	return 0;
	
	// No enchantment has been set yet; check for gemstones
	if (!it[in].data[0])
	{
		switch (it[in2].temp)
		{
			case IT_H_SA: r =  1; break;	case IT_H_RU: r =  2; break;
			case IT_H_AM: r =  3; break;	case IT_H_TO: r =  4; break;
			case IT_H_EM: r =  5; break;	case IT_H_DI: r =  6; break;
			case IT_H_SP: r =  7; break;	case IT_H_CI: r =  8; break;
			case IT_H_OP: r =  9; break;	case IT_H_AQ: r = 10; break;
			case IT_H_BE: r = 11; break;	case IT_H_ZI: r = 12; break;
			case IT_H_PH: r = 13; break;
			default: do_char_log(cn, 0, "That doesn't fit.\n"); return 0;
		}
		
		if (!it[in].data[1])
		{
			it[in].data[1] = r;
			if (r == 13) it[in].sprite[I_I] = 5871;
			else		 it[in].sprite[I_I] = 5643+r;
		}
		else
		{
			// Embeded switches is probably not the best way to go about this, but... it works.
			// This takes the current gemstone and checks it with the next gemstone to produce an answer #
			switch (it[in].data[1])
			{
			case  1: switch (r)
				{
				case  1: r =  1; break; case  2: r =  2; break; case  3: r =  3; break; case  4: r =  4; break;
				case  5: r =  5; break; case  6: r =  6; break; case  7: r =  7; break; case  8: r =  8; break;
				case  9: r =  9; break; case 10: r = 10; break; case 11: r = 11; break; case 12: r = 12; break;
				case 13: r = 79; break;
				}	break;
			case  2: switch (r)
				{
				case  1: r =  2; break; case  2: r = 13; break; case  3: r = 14; break; case  4: r = 15; break;
				case  5: r = 16; break; case  6: r = 17; break; case  7: r = 18; break; case  8: r = 19; break;
				case  9: r = 20; break; case 10: r = 21; break; case 11: r = 22; break; case 12: r = 23; break;
				case 13: r = 80; break;
				}	break;
			case  3: switch (r)
				{
				case  1: r =  3; break; case  2: r = 14; break; case  3: r = 24; break; case  4: r = 25; break;
				case  5: r = 26; break; case  6: r = 27; break; case  7: r = 28; break; case  8: r = 29; break;
				case  9: r = 30; break; case 10: r = 31; break; case 11: r = 32; break; case 12: r = 33; break;
				case 13: r = 81; break;
				}	break;
			case  4: switch (r)
				{
				case  1: r =  4; break; case  2: r = 15; break; case  3: r = 25; break; case  4: r = 34; break;
				case  5: r = 35; break; case  6: r = 36; break; case  7: r = 37; break; case  8: r = 38; break;
				case  9: r = 39; break; case 10: r = 40; break; case 11: r = 41; break; case 12: r = 42; break;
				case 13: r = 82; break;
				}	break;
			case  5: switch (r)
				{
				case  1: r =  5; break; case  2: r = 16; break; case  3: r = 26; break; case  4: r = 35; break;
				case  5: r = 43; break; case  6: r = 44; break; case  7: r = 45; break; case  8: r = 46; break;
				case  9: r = 47; break; case 10: r = 48; break; case 11: r = 49; break; case 12: r = 50; break;
				case 13: r = 83; break;
				}	break;
			case  6: switch (r)
				{
				case  1: r =  6; break; case  2: r = 17; break; case  3: r = 27; break; case  4: r = 36; break;
				case  5: r = 44; break; case  6: r = 51; break; case  7: r = 52; break; case  8: r = 53; break;
				case  9: r = 54; break; case 10: r = 55; break; case 11: r = 56; break; case 12: r = 57; break;
				case 13: r = 84; break;
				}	break;
			case  7: switch (r)
				{
				case  1: r =  7; break; case  2: r = 18; break; case  3: r = 28; break; case  4: r = 37; break;
				case  5: r = 45; break; case  6: r = 52; break; case  7: r = 58; break; case  8: r = 59; break;
				case  9: r = 60; break; case 10: r = 61; break; case 11: r = 62; break; case 12: r = 63; break;
				case 13: r = 85; break;
				}	break;
			case  8: switch (r)
				{
				case  1: r =  8; break; case  2: r = 19; break; case  3: r = 29; break; case  4: r = 38; break;
				case  5: r = 46; break; case  6: r = 53; break; case  7: r = 59; break; case  8: r = 64; break;
				case  9: r = 65; break; case 10: r = 66; break; case 11: r = 67; break; case 12: r = 68; break;
				case 13: r = 86; break;
				}	break;
			case  9: switch (r)
				{
				case  1: r =  9; break; case  2: r = 20; break; case  3: r = 30; break; case  4: r = 39; break;
				case  5: r = 47; break; case  6: r = 54; break; case  7: r = 60; break; case  8: r = 65; break;
				case  9: r = 69; break; case 10: r = 70; break; case 11: r = 71; break; case 12: r = 72; break;
				case 13: r = 87; break;
				}	break;
			case 10: switch (r)
				{
				case  1: r = 10; break; case  2: r = 21; break; case  3: r = 31; break; case  4: r = 40; break;
				case  5: r = 48; break; case  6: r = 55; break; case  7: r = 61; break; case  8: r = 66; break;
				case  9: r = 70; break; case 10: r = 73; break; case 11: r = 74; break; case 12: r = 75; break;
				case 13: r = 88; break;
				}	break;
			case 11: switch (r)
				{
				case  1: r = 11; break; case  2: r = 22; break; case  3: r = 32; break; case  4: r = 41; break;
				case  5: r = 49; break; case  6: r = 56; break; case  7: r = 62; break; case  8: r = 67; break;
				case  9: r = 71; break; case 10: r = 74; break; case 11: r = 76; break; case 12: r = 77; break;
				case 13: r = 89; break;
				}	break;
			case 12: switch (r)
				{
				case  1: r = 12; break; case  2: r = 23; break; case  3: r = 33; break; case  4: r = 42; break;
				case  5: r = 50; break; case  6: r = 57; break; case  7: r = 63; break; case  8: r = 68; break;
				case  9: r = 72; break; case 10: r = 75; break; case 11: r = 77; break; case 12: r = 78; break;
				case 13: r = 90; break;
				}	break;
			case 13: switch (r)
				{
				case  1: r = 79; break; case  2: r = 80; break; case  3: r = 81; break; case  4: r = 82; break;
				case  5: r = 83; break; case  6: r = 84; break; case  7: r = 85; break; case  8: r = 86; break;
				case  9: r = 87; break; case 10: r = 88; break; case 11: r = 89; break; case 12: r = 90; break;
				case 13: r = 91; break;
				}	break;
			default: break;
			}
			
			it[in].data[0] = r;
			it[in].data[1] = 0;
			if (r == 91)	 it[in].sprite[I_I] = 5872;
			else if (r > 78) it[in].sprite[I_I] = 5872+(r-78);
			else			 it[in].sprite[I_I] = 5655+r;
		}
		
		it[in].flags |= IF_UPDATE;
		
		use_consume_item(cn, in2, 1);
		return 1;
	}
	// Past here we can assume there is an active enchantment
	
	if (!CAN_ENCHANT(in2))
	{
		do_char_log(cn, 1, "Nothing happened.\n");
		return 0;
	}
	
	// Check if this enchant can be applied to this item
	switch (it[in].data[0])
	{
		// Only Helmets
		case  3: case 14: case 25: case 32: case 60: case 71: case 82:
			if (!(it[in2].placement & PL_HEAD))
			{
				do_char_log(cn, 1, "This can only be applied to Helmets.\n");
				return 0;
			}
			break;
		
		// Only Cloaks
		case  5: case 10: case 21: case 31: case 41: case 56: case 61:
			if (!(it[in2].placement & PL_CLOAK))
			{
				do_char_log(cn, 1, "This can only be applied to Cloaks.\n");
				return 0;
			}
			break;
		
		// Only Chests
		case  2: case 16: case 23: case 33: case 35: case 74: case 75:
			if (!(it[in2].placement & PL_BODY))
			{
				do_char_log(cn, 1, "This can only be applied to Body Armors.\n");
				return 0;
			}
			break;
		
		// Only Gloves
		case  8: case 19: case 29: case 38: case 46: case 53: case 86:
			if (!(it[in2].placement & PL_ARMS))
			{
				do_char_log(cn, 1, "This can only be applied to Gloves.\n");
				return 0;
			}
			break;
		
		// Only Boots
		case  4: case 15: case 30: case 37: case 65: case 70: case 72:
			if (!(it[in2].placement & PL_FEET))
			{
				do_char_log(cn, 1, "This can only be applied to Boots.\n");
				return 0;
			}
			break;
		
		// Only Jewellery
		case 22: case 54: case 79: case 80: case 83: case 85:
			if (!(it[in2].placement & PL_NECK) && !(it[in2].placement & PL_BELT) && !(it[in2].placement & PL_RING))
			{
				do_char_log(cn, 1, "This can only be applied to Jewellery.\n");
				return 0;
			}
			break;
		
		// Only Weapons
		case 49: case 62: case 66: case 67: case 77:
			if (!(it[in2].flags & IF_WEAPON))
			{
				do_char_log(cn, 1, "This can only be applied to Weapons.\n");
				return 0;
			}
			break;
		
		// Only Armors
		case  7: case 26: case 52: case 63: case 68:
			if (!(it[in2].flags & IF_ARMORS))
			{
				do_char_log(cn, 1, "This can only be applied to Armor Pieces.\n");
				return 0;
			}
			break;
		
		// Any
		default: break;
	}
	
	if (IS_TWOHAND(in2))
	{
		mul = 2;
	}
	
	// Apply the enchantment
	switch (it[in].data[0])
	{
		case  1: it[in2].attrib[AT_BRV][I_P] += 5*mul; break;
		case  2: it[in2].enchantment =  1; break;
		case  3: it[in2].enchantment =  2; break;
		case  4: it[in2].enchantment =  3; break;
		case  5: it[in2].enchantment =  4; break;
		case  6: it[in2].enchantment =  5; break;
		case  7: it[in2].aoe_bonus[I_P] += 1*mul; break;
		case  8: it[in2].enchantment =  7; break;
		case  9: it[in2].move_speed[I_P] += 6*mul; break;
		case 10: it[in2].enchantment =  8; break;
		case 11: it[in2].hp[I_P] += 35*mul; it[in2].mana[I_P] += 35*mul; break;
		case 12: it[in2].weapon[I_P] += 2*mul; it[in2].armor[I_P] += 2*mul; break;
		case 13: it[in2].attrib[AT_WIL][I_P] += 5*mul; break;
		case 14: it[in2].enchantment =  9; break;
		case 15: it[in2].enchantment = 10; break;
		case 16: it[in2].enchantment = 11; break;
		case 17: it[in2].enchantment = 12; break;
		case 18: it[in2].mana[I_P] += 50*mul; break;
		case 19: it[in2].enchantment = 13; break;
		case 20: it[in2].cast_speed[I_P] += 3*mul; break;
		case 21: it[in2].enchantment = 14; break;
		case 22: it[in2].enchantment = 15; break;
		case 23: it[in2].enchantment = 16; break;
		case 24: it[in2].attrib[AT_INT][I_P] += 5*mul; break;
		case 25: it[in2].enchantment = 17; break;
		case 26: it[in2].enchantment = 18; break;
		case 27: it[in2].enchantment = 19; break;
		case 28: it[in2].spell_mod[I_P] += 1*mul; break;
		case 29: it[in2].enchantment = 20; break;
		case 30: it[in2].enchantment =  6; break;
		case 31: it[in2].enchantment = 21; break;
		case 32: it[in2].enchantment = 22; break;
		case 33: it[in2].enchantment = 23; break;
		case 34: it[in2].attrib[AT_AGL][I_P] += 5*mul; break;
		case 35: it[in2].enchantment = 24; break;
		case 36: it[in2].enchantment = 25; break;
		case 37: it[in2].enchantment = 26; break;
		case 38: it[in2].enchantment = 27; break;
		case 39: it[in2].atk_speed[I_P] += 3*mul; break;
		case 40: it[in2].weapon[I_P] += 3*mul; break;
		case 41: it[in2].enchantment = 28; break;
		case 42: it[in2].to_parry[I_P] += 3*mul; break;
		case 43: it[in2].attrib[AT_STR][I_P] += 5*mul; break;
		case 44: it[in2].enchantment = 29; break;
		case 45: it[in2].hp[I_P] += 50*mul; break;
		case 46: it[in2].enchantment = 30; break;
		case 47: it[in2].end[I_P] += 25*mul; break;
		case 48: it[in2].to_hit[I_P] += 3*mul; break;
		case 49: it[in2].base_crit[I_P] += 1*mul; break;
		case 50: it[in2].armor[I_P] += 3*mul; break;
		case 51: for (n=0;n<5;n++) { it[in2].attrib[n][I_P] += 2*mul; } break;
		case 52: it[in2].light[I_P] += 20*mul; break;
		case 53: it[in2].enchantment = 31; break;
		case 54: it[in2].enchantment = 32; break;
		case 55: it[in2].to_hit[I_P] += 2*mul; it[in2].to_parry[I_P] += 2*mul; break;
		case 56: it[in2].enchantment = 33; break;
		case 57: it[in2].enchantment = 34; inds = 1; break;
		case 58: it[in2].spell_apt[I_P] += 6*mul; break;
		case 59: it[in2].enchantment = 35; break;
		case 60: it[in2].enchantment = 36; break;
		case 61: it[in2].enchantment = 37; break;
		case 62: it[in2].enchantment = 38; break;
		case 63: it[in2].enchantment = 39; break;
		case 64: it[in2].crit_multi[I_P] += 10*mul; break;
		case 65: it[in2].enchantment = 40; break;
		case 66: it[in2].enchantment = 41; break;
		case 67: it[in2].enchantment = 42; break;
		case 68: it[in2].enchantment = 43; break;
		case 69: it[in2].speed[I_P] += 2*mul; break;
		case 70: it[in2].enchantment = 44; break;
		case 71: it[in2].enchantment = 45; break;
		case 72: it[in2].enchantment = 46; break;
		case 73: it[in2].top_damage[I_P] += 12*mul; break;
		case 74: it[in2].enchantment = 47; break;
		case 75: it[in2].enchantment = 48; break;
		case 76: it[in2].crit_chance[I_P] += 16*mul; break;
		case 77: it[in2].enchantment = 49; break;
		case 78: it[in2].gethit_dam[I_P] += 2*mul; break;
		//
		case 79: it[in2].enchantment = 50; break;
		case 80: it[in2].enchantment = 51; break;
		case 81: it[in2].to_hit[I_P] -= 2*mul; it[in2].to_parry[I_P] += 4*mul; break;
		case 82: it[in2].enchantment = 52; break;
		case 83: it[in2].enchantment = 53; break;
		case 84: it[in2].enchantment = 54; break;
		case 85: it[in2].enchantment = 55; break;
		case 86: it[in2].enchantment = 56; break;
		case 87: it[in2].to_hit[I_P] += 4*mul; it[in2].to_parry[I_P] -= 2*mul; break;
		case 88: it[in2].weapon[I_P] += 2*mul; it[in2].top_damage[I_P] += 8*mul; break;
		case 89: it[in2].crit_chance[I_P] += 8*mul; it[in2].crit_multi[I_P] += 5*mul; break;
		case 90: it[in2].armor[I_P] += 2*mul; it[in2].gethit_dam[I_P] += 1*mul; break;
		case 91: it[in2].cool_bonus[I_P] += 2*mul; break;
		default: break;
	}
	
	// Finalize
	if (it[in2].temp)
	{
		temp = it[in2].temp;
		
		it[in2].orig_temp = temp;
	}
	else
	{
		temp = it[in2].orig_temp;
	}
	
	it[in2].current_damage = 0;
	it[in2].current_age[I_I] = 0;
	it[in2].current_age[I_A] = 0;
	it[in2].damage_state = 0;
	
	it[in2].armor[I_I] = it_temp[temp].armor[I_I];
	it[in2].armor[I_A] = it_temp[temp].armor[I_A];
	
	if (it[in2].driver!=40) // Prevent Seyan'du swords from resetting their WV
	{
		it[in2].weapon[I_I] = it_temp[temp].weapon[I_I];
		it[in2].weapon[I_A] = it_temp[temp].weapon[I_A];
	}
	
	if (temp && NOT_SINBINDER(in2))
	{
		it[in2].sprite[I_I] = it_temp[temp].sprite[I_I];
		it[in2].sprite[I_A] = it_temp[temp].sprite[I_A];
	}
	
	it[in2].flags |= IF_UPDATE | IF_IDENTIFIED | IF_ENCHANTED | IF_LOOKSPECIAL;
	it[in2].flags &= ~IF_CAN_EN;
	
	if (inds)
	{
		it[in2].max_damage = 0;
		it[in2].flags |= IF_NOREPAIR;
	}
	
	it[in2].value -= 1;
	it[in2].power += 15;
	
	if ((it[in2].flags & IF_SOULSTONE) && !HAS_ENCHANT(in2, 34))
	{
		it[in2].flags &= ~IF_NOREPAIR;
		if (it[in2].flags & IF_WEAPON)		it[in2].max_damage = it[in2].power * 4000;
		else								it[in2].max_damage = it[in2].power * 1000;
	}
	
	do_char_log(cn, 2, "You enchanted the %s with the talisman.\n", it[in2].name);
	use_consume_item(cn, in, 0);
	return 1;
}

int use_corruptor(int cn, int in)
{
	int in2;
	
	if (!IS_SANECHAR(cn))	return 0;
	if (!IS_SANEITEM(in))	return 0;
	
	if (it[in].data[0])
	{
		if (!IS_SANEITEM(in2 = ch[cn].citem))
		{
			do_char_log(cn, 1, "Try using a tarot card with the item. Or you can try using it on your skill tree...\n");
			return 0;
		}
		
		if ((it[in2].flags & IF_CORRUPTED) || (!(it[in2].placement)))
		{
			do_char_log(cn, 1, "Nothing happened.\n");
			return 0;
		}
		
		if (!IS_TAROT(in2))
		{
			do_char_log(cn, 1, "Hmm... Didn't work. Seems like this only works with tarot cards.\n");
			return 0;
		}
		
		if (it[in2].corruption = it[in].data[0]) {}
		else it[in2].corruption = RANDOM(NUM_CORR)+1;
		
		it[in2].flags |= IF_UPDATE | IF_CORRUPTED | IF_SHOPDESTROY;
		it[in2].power += 60;
		
		do_char_log(cn, 2, "You corrupted the card with %s.\n", sk_corrupt[it[in].data[0]-1]);
	}
	else
	{
		in2 = god_create_item(IT_CORRUPTOR);
		
		it[in2].data[0] = RANDOM(NUM_CORR)+1;
		it[in2].flags  |= (IF_LOOKSPECIAL | IF_SOULSTONE | IF_UPDATE);
		
		if (!god_give_char(in2, cn))
		{
			do_char_log(cn, 0, "Your backpack is full.\n");
			it[in2].used = USE_EMPTY;
			return 0;
		}
		do_char_log(cn, 1, "The %s was empowered with %s.\n", it[in2].reference, sk_corrupt[it[in2].data[0]-1]);
	}
	use_consume_item(cn, in, 1);
	
	return 1;
}

int use_soulstone(int cn, int in, int in2)
{
	int rank, c, t, n, m;

	if (!IS_SANECHAR(cn))	return 0;
	if (!IS_SANEITEM(in))	return 0;
	if (!in2)
	{
		do_char_log(cn, 1, "Try using something with the soulstone. Click on it with an item under your cursor.\n");
		return 0;
	}
	if (!IS_SANEITEM(in2))	return 0;
	
	switch (it[in2].driver)
	{
		case 68: // Soulstone
			return merge_new_soulstones(cn, in, in2);
		case 92: // Soul Focus
			return apply_new_focus(cn, in, in2);
		case 93: // Soul Catalyst
			return apply_new_catalyst(cn, in, in2);
		default:
			break;
	}
	
	if (CAN_SOULSTONE(in2))
		return apply_new_ss(cn, in2, in, 1);
	
	do_char_log(cn, 1, "Nothing happened.\n");
	return 0;
}

int use_soulfocus(int cn, int in) // driver 92
{
	int in2, c, v, m;

	if (!IS_SANECHAR(cn))	return 0;
	if (!IS_SANEITEM(in))	return 0;
	if (!(in2 = ch[cn].citem))
	{
		do_char_log(cn, 1, "Try using this item on a soulstone.\n");
		return 0;
	}
	if (!IS_SANEITEM(in2))	return 0;
	
	switch (it[in2].driver)
	{
		case 68: // Soulstone
			return apply_new_focus(cn, in2, in);
		case 92: // Soul Focus
			break;
		case 93: // Soul Catalyst
			break;
		default:
			break;
	}
	
	do_char_log(cn, 1, "Nothing happened.\n");
	return 0;
}

int use_soulcatalyst(int cn, int in) // driver 93
{
	int in2, in3, m, v, v1, v2;

	if (!IS_SANECHAR(cn))	return 0;
	if (!IS_SANEITEM(in))	return 0;
	if (!(in2 = ch[cn].citem))
	{
		do_char_log(cn, 1, "Try using this item on a soulstone.\n");
		return 0;
	}
	if (!IS_SANEITEM(in2))	return 0;
	
	switch (it[in2].driver)
	{
		case 68: // Soulstone
			return apply_new_catalyst(cn, in2, in);
		case 92: // Soul Focus
			break;
		case 93: // Soul Catalyst
			break;
		default:
			break;
	}
	do_char_log(cn, 1, "Nothing happened.\n");
	return 0;
}

void set_random_text(int cn)
{
	switch (RANDOM(4))
	{
		case  1:	strcpy(ch[cn].text[0], "An end to your misery, %s!"); break;
		case  2:	strcpy(ch[cn].text[0], "Took you long enough, %s!"); break;
		case  3:	strcpy(ch[cn].text[0], "It's about time! Goodbye, %s!"); break;
		default:	strcpy(ch[cn].text[0], "Yes! Die, %s!"); break;
	}
	switch (RANDOM(4))
	{
		case  1:	strcpy(ch[cn].text[1], "Something to do! Let's duel to the end, %s!"); break;
		case  2:	strcpy(ch[cn].text[1], "I hope you know what you're doing, %s!"); break;
		case  3:	strcpy(ch[cn].text[1], "A worthy foe! Let us shake the void, %s!"); break;
		default:	strcpy(ch[cn].text[1], "Yahoo! An enemy! Prepare to die, %s!"); break;
	}
	switch (RANDOM(4))
	{
		case  1:	strcpy(ch[cn].text[3], "Alas, I've lost. Good fight, %s!"); break;
		case  2:	strcpy(ch[cn].text[3], "Seems this is the end for me. So long, %s."); break;
		case  3:	strcpy(ch[cn].text[3], "Great fight, %s. I suppose this is goodbye."); break;
		default:	strcpy(ch[cn].text[3], "Thank you %s! Everything is better than being here."); break;
	}
}

int start_contract(int cn, int in)
{
	unsigned long long prof;
	int cc, rank, tier, mission, fl, bonus=0;
	int flags[NUM_MAP_POS+NUM_MAP_NEG] = {0};
	int r1=1, r2=2, r3=3;
	
	if (cn==0 || !(in && it[in].temp==MCT_CONTRACT))
	{
		return 0;
	}
	
	if (!it[in].data[0])
	{
		do_char_log(cn, 0, "This contract hasn't been signed yet. Find a coloured quill and use it on the contract to sign it.\n");
		return 0;
	}
	
	cc		= it[in].data[0];
	
	if (T_OS_TREE(cc, 2)) bonus++;
	if (T_OS_TREE(cc, 8)) bonus++;
	
	mission = it[in].data[1];
	tier    = it[in].data[2];
	rank    = getrank(cn) + tier + bonus;
	
	if (T_OS_TREE(cc, 9)) r1 = r2 = r3 = 3;
	
	if ((fl = it[in].data[3]) && fl > 0 && fl < NUM_MAP_POS+1) flags[fl-1] = r1;
	if ((fl = it[in].data[5]) && fl > 0 && fl < NUM_MAP_POS+1) flags[fl-1] = r2;
	if ((fl = it[in].data[7]) && fl > 0 && fl < NUM_MAP_POS+1) flags[fl-1] = r3;
	if ((fl = it[in].data[4]) && fl > 0 && fl < NUM_MAP_NEG+1) flags[fl-1+NUM_MAP_POS] = r1;
	if ((fl = it[in].data[6]) && fl > 0 && fl < NUM_MAP_NEG+1) flags[fl-1+NUM_MAP_POS] = r2;
	if ((fl = it[in].data[8]) && fl > 0 && fl < NUM_MAP_NEG+1) flags[fl-1+NUM_MAP_POS] = r3;
	
	return build_new_map(cn, cc, rank, flags, mission, tier, in);
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

int contract_has_flag(int in, int m)
{
	if (m < NUM_MAP_POS)
	{
		if (it[in].data[3] == m) return 1;
		if (it[in].data[5] == m) return 2;
		if (it[in].data[7] == m) return 3;
	}
	else
	{
		m -= NUM_MAP_POS;
		if (it[in].data[4] == m) return 1;
		if (it[in].data[6] == m) return 2;
		if (it[in].data[8] == m) return 3;
	}
	return 0;
}

void show_map_flags(int cn, int flags[NUM_MAP_POS+NUM_MAP_NEG], int tier)
{
	int font, n;
	
	font = get_tier_font(tier);
	
	for (n=0; n<NUM_MAP_POS+NUM_MAP_NEG; n++)
	{
		if (flags[n]) break;
	}
	if (n==NUM_MAP_POS+NUM_MAP_NEG) 
	{
		do_char_log(cn, font, "It has no other details.\n"); 
		do_char_log(cn, font, " \n"); 
		return;
	}
	
	do_char_log(cn, font, "Modifiers:\n");
	do_char_log(cn, font, " \n");
	
	// Sorted alphabetically
	if (flags[MM_P_DRGM]) do_char_log(cn, font, CFL_P_DRGM, flags[MM_P_DRGM]);
	if (flags[MM_N_ARUW]) do_char_log(cn, font, CFL_N_ARUW, flags[MM_N_ARUW]*25+25);
	if (flags[MM_P_AREQ]) do_char_log(cn, font, CFL_P_AREQ, flags[MM_P_AREQ]==3?"numerous":(flags[MM_P_AREQ]==2?"many":"some"));
	if (flags[MM_P_ARPT]) do_char_log(cn, font, CFL_P_ARPT, flags[MM_P_ARPT]==3?"numerous":(flags[MM_P_ARPT]==2?"many":"some"));
	if (flags[MM_N_ARDT]) do_char_log(cn, font, CFL_N_ARDT, flags[MM_N_ARDT]==3?"numerous":(flags[MM_N_ARDT]==2?"many":"some"));
	if (flags[MM_N_ARFL]) do_char_log(cn, font, CFL_N_ARFL, flags[MM_N_ARFL]==3?"numerous":(flags[MM_N_ARFL]==2?"many":"some"));
	if (flags[MM_P_ARGL]) do_char_log(cn, font, CFL_P_ARGL, flags[MM_P_ARGL]==3?"numerous":(flags[MM_P_ARGL]==2?"many":"some"));
	if (flags[MM_N_ARSP]) do_char_log(cn, font, CFL_N_ARSP, flags[MM_N_ARSP]==3?"numerous":(flags[MM_N_ARSP]==2?"many":"some"));
	if (flags[MM_P_CHST]) do_char_log(cn, font, CFL_P_CHST, flags[MM_P_CHST]);
	if (flags[MM_N_EXDV]) do_char_log(cn, font, CFL_N_EXDV, flags[MM_N_EXDV]);
	if (flags[MM_N_EXTY]) do_char_log(cn, font, CFL_N_EXTY, flags[MM_N_EXTY]);
	if (flags[MM_P_SHRN]) do_char_log(cn, font, CFL_P_SHRN, flags[MM_P_SHRN]);
	if (flags[MM_P_RANK]) do_char_log(cn, font, CFL_P_RANK, flags[MM_P_RANK]);
	if (flags[MM_N_ENFS]) do_char_log(cn, font, CFL_N_ENFS, flags[MM_N_ENFS]*10);
	if (flags[MM_N_ENRS]) do_char_log(cn, font, CFL_N_ENRS, flags[MM_N_ENRS]*10);
	if (flags[MM_N_ENSK]) do_char_log(cn, font, CFL_N_ENSK, flags[MM_N_ENSK]*10);
	if (flags[MM_N_ENUN]) do_char_log(cn, font, CFL_N_ENUN, flags[MM_N_ENUN]*50);
	if (flags[MM_P_ENGL]) do_char_log(cn, font, CFL_P_ENGL, flags[MM_P_ENGL]*RATE_P_ENGL);
	if (flags[MM_P_ENOS]) do_char_log(cn, font, CFL_P_ENOS, flags[MM_P_ENOS]*RATE_P_ENOS);
	if (flags[MM_P_ENBS]) do_char_log(cn, font, CFL_P_ENBS, flags[MM_P_ENBS]*RATE_P_ENBS);
	if (flags[MM_N_ENFO]) do_char_log(cn, font, CFL_N_ENFO, flags[MM_N_ENFO]*10);
	if (flags[MM_N_ENWI]) do_char_log(cn, font, CFL_N_ENWI, flags[MM_N_ENWI]*10);
	if (flags[MM_N_ENSH]) do_char_log(cn, font, CFL_N_ENSH, flags[MM_N_ENSH]*10);
	if (flags[MM_N_ENRO]) do_char_log(cn, font, CFL_N_ENRO, flags[MM_N_ENRO]*50);
	if (flags[MM_N_ENTR]) do_char_log(cn, font, CFL_N_ENTR, flags[MM_N_ENTR]==3?"multiple":(flags[MM_N_ENTR]==2?"reverse":"forward"));
	if (flags[MM_P_XEXP]) do_char_log(cn, font, CFL_P_XEXP, flags[MM_P_XEXP]*RATE_P_XEXP);
	if (flags[MM_P_XLUK]) do_char_log(cn, font, CFL_P_XLUK, flags[MM_P_XLUK]*RATE_P_XLUK);
	if (flags[MM_P_XOSP]) do_char_log(cn, font, CFL_P_XOSP, flags[MM_P_XOSP]*RATE_P_XOSP);
	if (flags[MM_P_XBSP]) do_char_log(cn, font, CFL_P_XBSP, flags[MM_P_XBSP]*RATE_P_XBSP);
	if (flags[MM_N_EXEN]) do_char_log(cn, font, CFL_N_EXEN, flags[MM_N_EXEN]);
	if (flags[MM_N_PLDB]) do_char_log(cn, font, CFL_N_PLDB, flags[MM_N_PLDB]*10);
	if (flags[MM_N_PLHY]) do_char_log(cn, font, CFL_N_PLHY, flags[MM_N_PLHY]*50);
	if (flags[MM_N_PLFR]) do_char_log(cn, font, CFL_N_PLFR, flags[MM_N_PLFR]*5);
	if (flags[MM_N_PLST]) do_char_log(cn, font, CFL_N_PLST, flags[MM_N_PLST]*20);
	if (flags[MM_P_PLXP]) do_char_log(cn, font, CFL_P_PLXP, flags[MM_P_PLXP]*RATE_P_PLXP);
	
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

void add_map_goal(int loc, int v)
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
				ch[co].data[41] += + v<<8;
				chlog(co, "map goal increased by %d (%d)", v, CONT_GOAL(co));
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
		case 27: return "Dwindle";
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
		case 44: return "Bard";
		case 45: return "Siren";
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
		case 27: return "A ghastly humanoid, warped skin on bone and little else."; // Dwindle
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
		case 44: return "A rugged man in beautiful clothes. He flashes a charming smile."; // Bard
		case 45: return "A beautiful woman, or so it seems. She calls for you."; // Siren
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
		case 27: return 19408; // Dwindle
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
		case 44: return 27088; // Bard
		case 45: return 28112; // Siren
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
				in2 = RANDOM(2)?IT_CH_EMPERO_R:IT_CH_MAGI_R; 	break; // Samurai
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
				in2 = RANDOM(2)?IT_CH_CHARIO_R:IT_CH_SUN_R; 	break; // Dwindle
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
				in2 = RANDOM(2)?IT_CH_MAGI_R:IT_CH_DEATH_R; 	break; // Amazon
	case 39: 	in  = RANDOM(2)?IT_CH_TEMPER:IT_CH_HANGED; 
				in2 = RANDOM(2)?IT_CH_DEVIL_R:IT_CH_STAR_R; 	break; // Dancer
	case 40: 	in  = RANDOM(2)?IT_CH_MAGI:IT_CH_PREIST; 
				in2 = RANDOM(2)?IT_CH_HEIROP_R:IT_CH_SUN_R; 	break; // Warlock
	case 41: 	in  = RANDOM(2)?IT_CH_JUDGE:IT_CH_PREIST; 
				in2 = RANDOM(2)?IT_CH_MAGI_R:IT_CH_JUDGE_R; 	break; // Wizard
	case 42: 	in  = RANDOM(2)?IT_CH_HEIROPH:IT_CH_PREIST; 
				in2 = RANDOM(2)?IT_CH_MAGI_R:IT_CH_HEIROP_R; 	break; // Mystic
	case 43: 	in  = RANDOM(2)?IT_CH_DEATH:IT_CH_STRENGTH; 
				in2 = RANDOM(2)?IT_CH_CHARIO_R:IT_CH_TOWER_R; 	break; // Grosser
	case 44: 	in  = RANDOM(2)?IT_CH_LOVERS:IT_CH_STAR; 
				in2 = RANDOM(2)?IT_CH_LOVERS_R:IT_CH_MAGI_R; 	break; // Bard
	case 45: 	in  = RANDOM(2)?IT_CH_TOWER:IT_CH_CHARIOT; 
				in2 = RANDOM(2)?IT_CH_EMPERO_R:IT_CH_CHARIO_R; 	break; // Siren
	//
	case 11+NUM_MAP_ENEM+0: in = IT_CH_JUSTICE;  in2 = IT_CH_JUSTIC_R; break; // Kali
	case 11+NUM_MAP_ENEM+1: in = IT_CH_STRENGTH; in2 = IT_CH_STRENG_R; break; // Rundas
	case 11+NUM_MAP_ENEM+2: in = IT_CH_DEATH;    in2 = IT_CH_DEATH_R;  break; // Szelanya
	case 11+NUM_MAP_ENEM+3: in = IT_CH_HEIROPH;  in2 = IT_CH_HEIROP_R; break; // Azrael
	case 11+NUM_MAP_ENEM+4: in = IT_CH_STAR;     in2 = IT_CH_STAR_R;   break; // Brighid
	case 11+NUM_MAP_ENEM+5: in = IT_CH_CHARIOT;  in2 = IT_CH_CHARIO_R; break; // Nidhogg
	case 11+NUM_MAP_ENEM+6: in = IT_CH_MAGI;     in2 = IT_CH_MAGI_R;   break; // Metztli
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
	 75, 0,45, 0, 0, 0, 0,50, 0, 0,	0, 0,60, 0, 0, 0, 0, 0, 0, 0,44,53,51,56,46,33,50,37,50,50}, // 27 : Dwindle
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
	 75, 0,45,50, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0,55, 0,60,48,54,53,47,48,35,50,36,48,52}, // 36 : Swordsman
	{40, 0, 0, 0, 0, 0, 0, 0, 0,50, 0,50, 0, 0, 0, 0, 0,50,50, 0, 0,50, 0,60, 0,55,60, 0,75,75,
	 75, 0, 0, 0, 0,40, 0, 0, 0, 0,45, 0, 0, 0, 0, 0, 0, 0, 0, 0,41,55,47,59,48,33,50,37,55,45}, // 37 : Paladin
	{40,50, 0, 0, 0, 0, 0,55, 0,50, 0, 0, 0, 0, 0, 0, 0,50,50, 0,45,50, 0,60, 0, 0, 0, 0,75,75,
	 75, 0, 0, 0, 0,60, 0, 0, 0, 0,	0, 0, 0,40, 0, 0, 0, 0, 0, 0,49,52,53,49,47,36,50,34,45,55}, // 38 : Amazon
	{40, 0, 0, 0, 0, 0, 0, 0,45,50, 0, 0, 0, 0, 0, 0, 0,50,50, 0, 0,50, 0,55, 0, 0, 0, 0,75,80,
	 75,50, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,55,60,40, 0,47,57,48,45,53,37,50,35,49,51}, // 39 : Dancer (v2)
	{40, 0, 0, 0, 0, 0, 0, 0,55,50, 0, 0, 0, 0, 0, 0, 0,50,50, 0,50,50, 0,60, 0, 0, 0,60,75,75,
	 75, 0, 0, 0, 0, 0, 0,40, 0, 0,	0, 0, 0, 0, 0,45, 0, 0, 0, 0,50,42,49,54,55,38,50,32,51,49}, // 40 : Warlock
	{40, 0, 0, 0, 0, 0, 0, 0, 0,50, 0, 0, 0, 0, 0, 0, 0,50,50, 0, 0,50, 0,60,45, 0, 0,50,75,75,
	 75, 0, 0, 0,40, 0, 0, 0, 0, 0,	0, 0, 0,55,60, 0, 0, 0, 0, 0,56,43,40,52,59,40,50,30,51,49}, // 41 : Wizard
	{40, 0, 0, 0, 0, 0, 0, 0, 0,50, 0, 0, 0, 0, 0, 0, 0,50,50, 0, 0,50, 0,60,60,40, 0,55,75,75,
	 75, 0, 0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0,50,45, 0, 0, 0,52,45,41,54,58,37,50,33,48,52}, // 42 : Mystic
	{40, 0, 0, 0, 0, 0, 0, 0,50,50, 0, 0, 0, 0, 0, 0, 0,50,50, 0,55,50, 0,60, 0, 0, 0, 0,75,75,
	 75, 0, 0, 0, 0, 0, 0, 0, 0, 0,	0,60,40, 0,45, 0, 0, 0, 0, 0,43,46,57,55,49,36,50,34,53,47}, // 43 : Grossling
	{40, 0, 0, 0, 0, 0, 0, 0, 0,50, 0,55, 0, 0, 0, 0, 0,50,50, 0, 0,50, 0,55, 0, 0,50, 0,75,80,
	 75,45,60,40, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,50,49,51,51,49,34,50,35,51,49}, // 44 : Bard
	{40, 0, 0, 0, 0, 0, 0, 0, 0,50, 0, 0, 0, 0, 0, 0, 0,50,50,45,60,50, 0,55, 0, 0, 0, 0,75,80,
	 75,55, 0, 0, 0,50, 0,40, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,50,51,49,49,51,35,50,33,48,52}, // 45 : Siren
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
	 75,50, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,45,45,45,45,45,34,40,32,51,45}, // *5 : Brighid
	{40, 0, 0, 0, 0, 0, 0, 0, 0,44, 0, 0, 0, 0, 0, 0, 0,50,50, 0, 0,50, 0,55, 0, 0, 0,45,75,75,
	 75, 0, 0, 0, 0, 0, 0,45, 0, 0, 0, 0,50, 0, 0,50, 0, 0, 0, 0,45,45,45,45,45,32,40,34,47,49}, // *6 : Nidhogg
	{40, 0, 0, 0, 0, 0, 0, 0, 0,44, 0, 0, 0, 0, 0, 0, 0,50,50, 0,45,50, 0,55, 0, 0, 0, 0,75,75,
	 75, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,45,50, 0,50, 0, 0, 0,45,45,45,45,45,36,40,30,50,46}  // *7 : Metztli
};

int generate_map_enemy(int cn, int temp, int kin, int xx, int yy, int base, int affix, int tarot)
{
	int co, tmp, pts, n, m, j, in, v, rank, tbelt=1, hasloot=0;
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
	if (affix!=7) fx_add_effect(6, 0, ch[co].x, ch[co].y, 0);
	
	// Set name and sprite
	if (kin)
	{
		if (affix==8)
		{
			for (n = 0; n < 40; n++) ch[co].name[n] = 0; m = 6 + RANDOM(4);
			for (n = 0; n < m; n++) ch[co].name[n] = 33 + RANDOM(94);
			sprintf(ch[co].reference, "the sanguine creature");
			switch (RANDOM(6))
			{
				case  1: sprintf(ch[co].description, "A strange creature covered in something red."); break;
				case  2: sprintf(ch[co].description, "a sTrange cReature cOvered iN sOmething rEd."); break;
				case  3: sprintf(ch[co].description, "A $tr@ng3 crrrrrrr c0vered in coVere_ ##NUL."); break;
				case  4: sprintf(ch[co].description, "it hurts."); break;
				case  5: sprintf(ch[co].description, "It's you. Your face, your eyes... bleeding."); break;
				default: sprintf(ch[co].description, "bEfore yOu sTands tHe sAnguine cReature."); break;
			}
		}
		else if (affix==7)
		{
			sprintf(ch[co].name, "Shade");
			sprintf(ch[co].reference, "the shadow creature");
			sprintf(ch[co].description, "A featureless void. It glides through the darkness with otherworldly speed.");
		}
		else
		{
			sprintf(ch[co].name, "%s", get_map_enemy_name(kin));
			sprintf(ch[co].reference, "the %s", get_map_enemy_name(kin));
			sprintf(ch[co].description, "%s", get_map_enemy_desc(kin));
		}
		ch[co].sprite = get_map_enemy_sprite(kin);
		if (kin==10||kin==28||kin==30) // Set Simple Animation
			ch[co].flags |= CF_SIMPLE;
		set_map_enemy_tarot(co, kin, tarot);
	}
	
	// Mimic GC code
	for (n = 0; n<5; n++)
	{
		tmp = base * 50 / max(1, get_map_eme[kin][n+50]);
		B_AT(co, n) = max(10, min(200, tmp));
	}

	for (n = 0; n<MAXSKILL; n++)
	{
		if (get_map_eme[kin][n])
		{
			tmp = base * 50 / max(1, get_map_eme[kin][n]);
			B_SK(co, n) = max(1, min(200, tmp));
		}
	}
	
	tmp = base * 50 / max(1, get_map_eme[kin][55]) * 5;	ch[co].hp[0]   	= max(100, min(999, tmp));
	tmp = base * 50 / max(1, get_map_eme[kin][56]) * 2;	ch[co].end[0]  	= max(100, min(999, tmp));
	tmp = base * 50 / max(1, get_map_eme[kin][57]) * 5;	ch[co].mana[0] 	= max(100, min(999, tmp));
	tmp = base * 50 / max(1, get_map_eme[kin][58]) / 2; ch[co].weapon_bonus = max( 12, min(200, tmp));
	tmp = base * 50 / max(1, get_map_eme[kin][59]) / 2; ch[co].armor_bonus  = max(  8, min(200, tmp));
	
	chlog(co, "created map mob");
	
	// Apply flags and Prefix
	if (affix)
	{
		if (affix != 8 && affix != 7 && (n = get_map_enemy_affix_length(affix)))
		{
			sprintf(buf, "the %s %s", get_map_enemy_affix(affix), ch[co].name); buf[5+n] = tolower(buf[5+n]);
			strncpy(ch[co].reference, buf, 39); ch[co].reference[39] = 0;
			
			sprintf(buf, "%s %s", get_map_enemy_affix(affix), ch[co].name); buf[0] = toupper(buf[0]);
			strncpy(ch[co].name, buf, 39); ch[co].name[39] = 0;
		}
		
		if (!(ch[co].flags & CF_EXTRAEXP)) ch[co].flags |= CF_EXTRAEXP;
	
		switch (affix)
		{
			case  1:												// "Tough" (BS)
				for (n = 0; n<5; n++)
					B_AT(co, n) += 3+RANDOM(3);
				for (n = 0; n<MAXSKILL; n++) if (B_SK(co, n))
					B_SK(co, n) += 3+RANDOM(3)*2;
				break;
			case  2:												// "Fierce" (BS)
				for (n = 0; n<5; n++)
					B_AT(co, n) += 6+RANDOM(5);
				for (n = 0; n<MAXSKILL; n++) if (B_SK(co, n))
					B_SK(co, n) += 6+RANDOM(5)*2;
				if (!(ch[co].flags & CF_EXTRACRIT)) ch[co].flags |= CF_EXTRACRIT;
				break;
			//
			case  3:												// "Divine" (CN)
				for (n = 0; n<5; n++)
					B_AT(co, n) += (B_AT(co, n)/20)+RANDOM(B_AT(co, n)/18);
				for (n = 0; n<MAXSKILL; n++) if (B_SK(co, n))
					B_SK(co, n) += (B_AT(co, n)/20)+RANDOM(B_AT(co, n)/18)*2;
				ch[co].data[72] = 3;
				//
				make_talisfrag(co, RANDOM(2)+1);
				// 
				break;
			case  4:												// "Cruel"
			case 14:
				for (n = 0; n<5; n++)
					B_AT(co, n) += (B_AT(co, n)/18)+RANDOM(B_AT(co, n)/16);
				for (n = 0; n<MAXSKILL; n++) if (B_SK(co, n))
					B_SK(co, n) += (B_AT(co, n)/18)+RANDOM(B_AT(co, n)/16)*2;
				ch[co].data[72] = 4;
				if (affix==14)
				{
					for (n = 0; n<5; n++)
						B_AT(co, n) += (B_AT(co, n)/18)+RANDOM(B_AT(co, n)/16);
					for (n = 0; n<MAXSKILL; n++) if (B_SK(co, n))
						B_SK(co, n) += (B_AT(co, n)/18)+RANDOM(B_AT(co, n)/16)*2;
					make_talisfrag(co, 4);
				}
				else
				{
					make_talisfrag(co, 2);
				}
				break;
			case  5:												// "Timid"
				for (n = 0; n<5; n++)
					B_AT(co, n) += (B_AT(co, n)/22)+RANDOM(B_AT(co, n)/20);
				for (n = 0; n<MAXSKILL; n++) if (B_SK(co, n))
					B_SK(co, n) += (B_AT(co, n)/22)+RANDOM(B_AT(co, n)/20)*2;
				ch[co].data[72] = 5;
				ch[co].data[25] = 1;
				//
				make_talisfrag(co, RANDOM(2)+2);
				//
				break;
			case  6:												// Legendary
				for (n = 0; n<5; n++)
					B_AT(co, n) += (B_AT(co, n)/22)+RANDOM(B_AT(co, n)/22);
				for (n = 0; n<MAXSKILL; n++) if (B_SK(co, n))
					B_SK(co, n) += (B_AT(co, n)/22)+RANDOM(B_AT(co, n)/22)*2;
				ch[co].data[72] = 6;
				if (!(ch[co].flags & CF_EXTRACRIT)) ch[co].flags |= CF_EXTRACRIT;
				//
				if (!RANDOM(6) && (in = god_create_item(3425+RANDOM(9))))
				{
					god_give_char(in, co);
					chlog(co, "got %s", itemvowel(in, 0));
					hasloot=1;
				}
				else
					make_talisfrag(co, 3);
				//
				break;
			case  7:												// Darkwood Shadow
				ch[co].kindred |= KIN_SHADOW;
				ch[co].flags |= CF_SILENCE | CF_INFRARED | CF_EXTRACRIT;
				ch[co].speed_mod = 120;
				ch[co].skill[SK_PERCEPT][1] = 30;
				switch (RANDOM(6))
				{
					case  1:	strcpy(ch[co].text[1], "%s..."); break;
					case  2:	strcpy(ch[co].text[1], "...%s..."); break;
					case  3:	strcpy(ch[co].text[1], "...%s."); break;
					default:	break;
				}
				break;
			case  8:												// Sanguine
				ch[co].kindred |= KIN_BLOODY;
				ch[co].flags |= CF_SILENCE | CF_INFRARED | CF_SIMPLE;
				switch (RANDOM(4))
				{
					case  1:	strcpy(ch[co].text[0], "dddddd%saaaaaaaaaaaaa"); break;
					case  2:	strcpy(ch[co].text[0], "#9#gOoodbye, goo0db, ggggg, %s!"); break;
					case  3:	strcpy(ch[co].text[0], ")AFA0 (((! !!! %s##%%!"); break;
					default:	strcpy(ch[co].text[0], "HA HA HA HA HA HA HA"); break;
				}
				switch (RANDOM(4))
				{
					case  1:	strcpy(ch[co].text[1], "%s is the one the one the one the one"); break;
					case  2:	strcpy(ch[co].text[1], "##### %s ##### !!! !!"); break;
					case  3:	strcpy(ch[co].text[1], "hhhhh eh lll pp mmm mmm mmm mmm mmm"); break;
					default:	strcpy(ch[co].text[1], "#9#ggggggk"); break;
				}
				switch (RANDOM(4))
				{
					case  1:	strcpy(ch[co].text[3], "%s thank you thAnk yUk thhh thh"); break;
					case  2:	strcpy(ch[co].text[3], "#9#AAAAAAAAHH!!!"); break;
					case  3:	strcpy(ch[co].text[3], "^&%%&^%%^&%%&^%%&%%&%%"); break;
					default:	strcpy(ch[co].text[3], "My $$S$S loves me very vEry veRY &&!!"); break;
				}
				make_corruptor(co, 2);
				break;
			//
			default:
				break;
		}
	}
	
	// Calculate experience
	pts = 0;
	for (n = 0; n<5; n++) for (j = 10; j<B_AT(co, n); j++)
		pts += attrib_needed(j, 3);
	for (j = 50; j<ch[co].hp[0]; j++) 
		pts += hp_needed(j, 3);
	for (j = 50; j<ch[co].mana[0]; j++) 
		pts += mana_needed(j, 3);
	for (n = 0; n<MAXSKILL; n++) for (j = 1; j<B_SK(co, n); j++)
		pts += skill_needed(j, 3);
	ch[co].points_tot = pts;
	
	ch[co].gold   = 0;
	ch[co].a_hp   = 9999999;
	ch[co].a_end  = 999999;
	ch[co].a_mana = 999999;
	
	if (temp == 350 && affix != 7 && affix != 8) // Map enemy
	{
		in = 0;
		rank = getrank(co);
		
		if (T_OS_TREE(cn, 1)) tbelt += 2;
		if (T_OS_TREE(cn, 5)) tbelt += 2;
		
		if (rank>=18 && try_boost(40))
		{
			static int item[]  = {
				2515, 2519, 2523, 2527, 2531, 
				2536, 2540, 2544, 2548, 2552,
				2609, 2609, 2609, 2609, 2609
			};
			in = RANDOM(sizeof(item) / sizeof(int)); in = item[in];
		}
		else if (rank>=15 && try_boost(40))
		{
			static int item[]  = {
				2514, 2518, 2522, 2526, 2530, 
				2535, 2539, 2543, 2547, 2551,
				2609, 2609, 2609, 2609, 2609
			};
			in = RANDOM(sizeof(item) / sizeof(int)); in = item[in];
		}
		else if (rank>=12 && try_boost(40))
		{
			static int item[]  = {
				2513, 2517, 2521, 2525, 2529, 
				2534, 2538, 2542, 2546, 2550,
				2609, 2609, 2609, 2609, 2609
			};
			in = RANDOM(sizeof(item) / sizeof(int)); in = item[in];
		}
		else if (try_boost(40))
		{
			static int item[]  = {
				2512, 2516, 2520, 2524, 2528,
				2533, 2537, 2541, 2545, 2549,
				2609, 2609, 2609, 2609, 2609
			};
			in = RANDOM(sizeof(item) / sizeof(int)); in = item[in];
		}
		
		if (in)
		{
			in = pop_create_item(in, co);
			god_give_char(in, co);
			chlog(co, "got %s", itemvowel(in, 0));
			hasloot=1;
		}
		
		in = 0;
		
		if (try_boost(40))
		{
			in = pop_create_bonus(co);
			god_give_char(in, co);
			hasloot=1;
		}
		if (try_boost(500))
		{
			in = pop_create_bonus(co);
			god_give_char(in, co);
			hasloot=1;
		}
		if (try_boost(6000/max(1,tbelt)))
		{
			in = pop_create_bonus_belt(co);
			god_give_char(in, co);
			hasloot=1;
		}
		
		if (!IS_LABY_MOB(co) && !ch[co].citem && !ch[co].item[0] && !hasloot && !(ch[co].flags & CF_EXTRAEXP) && !(ch[co].flags & CF_EXTRACRIT) && try_boost(DW_CHANCE))
		{
			if (in = god_create_item(IT_CORRUPTOR))
			{
				ch[co].citem = in;
				it[in].carried = co;
				it[in].corruption = 555;
			}
		}
		
		ch[co].data[29] = xx + yy * MAPX;
		ch[co].data[30] = ch[co].dir = 1+RANDOM(8);
		if (!ch[co].data[72]) ch[co].data[72] = 1;
		if (!RANDOM(6)) ch[co].light_bonus = RANDOM(4)*25+25;
		set_random_text(co);
	}
	
	if (affix==8) ch[co].light_bonus = 5;
	if (ch[co].sprite == 22480 && affix!=7) ch[co].light_bonus = 200;
	else if (ch[co].sprite == 14288 || ch[co].sprite == 31696)
	{
		ch[co].light_bonus = 0;
		ch[co].flags |= CF_INFRARED;
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


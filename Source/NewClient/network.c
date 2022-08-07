// **************************************************************** 
//
//  The Last Gate - SDL2 Client by Zachary "ZarroTsu" Baillie
//  Based on the Mercenaries of Astonia engine by Daniel Brockhaus
//
//  network.c
//  Network-related functions such as packet management.
//
// **************************************************************** 
#include "common.h"
#include "network.h"

static void load_unique(void);
static void save_unique(void);

int skill_cmp(const void *a,const void *b)
{
	const struct skilltab *c,*d;
	int m1,m2;

	c=a; d=b;

	m1=c->nr; m2=d->nr;
	
	if (m1==99 && m2!=99) return  1;
	if (m2==99 && m1!=99) return -1;
	
	if (pl.skill[m1][0]==0 && pl.skill[m2][0]!=0) return  1;
	if (pl.skill[m2][0]==0 && pl.skill[m1][0]!=0) return -1;
	
	if (pl.skill[m1][0]==0 && pl.skill[m2][0]==0 && 
		(m1==50||m1==51) && 
		(m2!=50&&m2!=51)) return -1;
	if (pl.skill[m2][0]==0 && pl.skill[m1][0]==0 && 
		(m2==50||m2==51) && 
		(m1!=50&&m1!=51)) return  1;

	// Stealth, Resistance, Immunity -- these are active even if you don't know them. m==8||m==23||m==28||m==29||m==30||m==32
	if (pl.skill[m1][0]==0 && pl.skill[m2][0]==0 && 
		(m1==8||m1==23||m1==28||m1==29||m1==30||m1==32||m1==44||m1==50||m1==51) && 
		(m2!=8&&m2!=23&&m2!=28&&m2!=29&&m2!=30&&m2!=32&&m2!=44&&m2!=50&&m2!=51)) return -1;
	if (pl.skill[m2][0]==0 && pl.skill[m1][0]==0 && 
		(m2==8||m2==23||m2==28||m2==29||m2==30||m2==32||m2==44||m2==50||m2==51) && 
		(m1!=8&&m1!=23&&m1!=28&&m1!=29&&m1!=30&&m1!=32&&m1!=44&&m1!=50&&m1!=51)) return  1;
	
	if (c->sortkey>d->sortkey) return  1;
	if (c->sortkey<d->sortkey) return -1;

	return strcmp(c->name,d->name);
}
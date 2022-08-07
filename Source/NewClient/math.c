// **************************************************************** 
//
//  The Last Gate - SDL2 Client by Zachary "ZarroTsu" Baillie
//  Based on the Mercenaries of Astonia engine by Daniel Brockhaus
//  
//  math.c
//  Handles math functions used elsewhere in code.
//
// **************************************************************** 
#include "common.h"
#include "math.h"

int MAT_Points2Rank(int v)
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

int MAT_Rank2Points(int v)
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
	if (v>=24)	return 118755000;
				return 0;
}

int MAT_Points2Level(int curr_exp)
{
	int curr_level, next_level, r, j;
	
	if (!curr_exp)
		return 250;
	
	curr_level = MAT_Points2Rank(curr_exp);
	
	if (curr_level == 24)
		return 118755000;
	
	next_level = curr_level + 1;
	
	r = MAT_Rank2Points(next_level);
	j = r-curr_exp;
	
	return j;
}

int MAT_AttribNeeded(int n,int v)
{
	if (v >= pl.attrib[n][2]) return HIGH_VAL;
	return v * v * v * pl.attrib[n][3] / 20;
}

int MAT_HPNeeded(int v)
{
	if (v >= pl.hp[2]) return HIGH_VAL;
	return v * pl.hp[3];
}

int MAT_EndNeeded(int v)
{
	if (v >= pl.end[2]) return HIGH_VAL;
	return v * pl.end[3] / 2;
}

int MAT_ManaNeeded(int v)
{
	if (v >= pl.mana[2]) return HIGH_VAL;
	return v * pl.mana[3];
}

int MAT_SkillNeeded(int n, int v)
{
	if (v >= pl.skill[n][2]) return HIGH_VAL;
	return max(v, v * v * v * pl.skill[n][3] / 40);
}

int MAT_ATScore(int n)
{
	return ( (pl.attrib[n][4] << 8) | pl.attrib[n][5] );
}

int MAT_SKScore(int n)
{
	return ( (pl.skill[n][4] << 8) | pl.skill[n][5] );
}

/* END OF FILE */
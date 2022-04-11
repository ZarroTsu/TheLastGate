/*************************************************************************

   This file is part of 'Mercenaries of Astonia v2'
   Copyright (c) 1997-2001 Daniel Brockhaus (joker@astonia.com)
   All rights reserved.

 **************************************************************************/

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

#include "server.h"
#include "driver.h"

void init_lights(void)
{
	int x, y, in, m, cnt1 = 0, cnt2 = 0;

	for (y = m = 0; y<MAPY; y++)
	{
		for (x = 0; x<MAPX; x++, m++)
		{
			map[m].light  = 0;
			map[m].dlight = 0;
		}
	}

	for (y = m = 0; y<MAPY; y++)
	{
		printf("%4d/%4d (%d,%d)\r", y, MAPY, cnt1, cnt2);
		fflush(stdout);
		for (x = 0; x<MAPX; x++, m++)
		{
			if (map[m].flags & MF_INDOORS)
			{
				cnt2++;
				compute_dlight(x, y);
			}
			if ((in = map[m].it)==0)
			{
				continue;
			}
			if (it[in].active)
			{
				if (it[in].light[1])
				{
					do_add_light(x, y, it[in].light[1]);
					cnt1++;
				}
			}
			else
			{
				if (it[in].light[0])
				{
					do_add_light(x, y, it[in].light[0]);
					cnt1++;
				}
			}
		}
	}
}

int pop_create_item(int temp, int cn)
{
	int n, m, in = 0, enbl = 0, godroll = 0, bonus = 1;
	char *gend, *godn, name[60], refer[60], descr[220];
	
	if (!(it_temp[temp].flags & IF_TAKE))
	{
		xlog("pop_create_item: bad template number %d", temp);
		return 0;
	}
	if (is_unique_able(temp))
	{
		if (!in && ch[cn].alignment<0 && !RANDOM(300)) 	// Reward a unique weapon
		{	godroll = RANDOM(4)+1; 						// Decide which god blessed the weapon.
			for (m = 1, n = 0; m<MAXITEM; m++)			// Check for up to three copies
			{
				if (it[m].used==USE_EMPTY || it[m].orig_temp!=temp) continue;
				if ((godroll == 1 && (it[m].flags & IF_KWAI_UNI) && (it[m].flags & IF_GORN_UNI)) ||
					(godroll == 2 && (it[m].flags & IF_KWAI_UNI)) ||
					(godroll == 3 && (it[m].flags & IF_GORN_UNI)) ||
					(godroll == 4 && (it[m].flags & IF_PURP_UNI)) ) n++;
				if (n>=3) break;
			}
			if (n<3)
				in = god_create_item(temp); 			// create the item
			if (in)										// assure the item is created
			{
				if (it[in].placement & PL_TWOHAND)
				{
					bonus = 2;
				}
				
				switch (godroll)
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
				
				if (is_unique_able(temp) > 54) // Claws
				{
					if (godroll==1)
						it[in].sprite[0] = 3715 + is_unique_able(temp)-55;
					else if (godroll==2)
						it[in].sprite[0] = 3721 + is_unique_able(temp)-55;
					else if (godroll==3)
						it[in].sprite[0] = 3727 + is_unique_able(temp)-55;
					else
						it[in].sprite[0] = 4944 + is_unique_able(temp)-55;
				}
				else if (is_unique_able(temp) > 45)
				{
					if (godroll==1)
						it[in].sprite[0] = 2602 + is_unique_able(temp)-46;
					else if (godroll==2)
						it[in].sprite[0] = 2611 + is_unique_able(temp)-46;
					else if (godroll==3)
						it[in].sprite[0] = 2620 + is_unique_able(temp)-46;
					else
						it[in].sprite[0] = 4935 + is_unique_able(temp)-46;
				}
				else
				{
					if (godroll==1)
						it[in].sprite[0] =  730 + is_unique_able(temp)-1;
					else if (godroll==2)
						it[in].sprite[0] = 2512 + is_unique_able(temp)-1;
					else if (godroll==3)
						it[in].sprite[0] = 2557 + is_unique_able(temp)-1;
					else
						it[in].sprite[0] = 4890 + is_unique_able(temp)-1;
				}
			}
			else
			{
				in = 0;
			}
		}
	}
	if (!in)
	{
		in = god_create_item(temp);
		if (in && it[in].max_damage>0)
		{
			if (RANDOM(2))
			{
				it[in].current_damage = it[in].max_damage + 1;
				item_age(in);
			}
			it[in].current_damage = RANDOM(it[in].max_damage);
		}
	}
	else
	{
		xlog("%s got unique item %s.", ch[cn].name, it[in].name);
	}

	return(in);
}

#define POP_GITEM	6

int pop_create_bonus(int cn, int chance)
{
	int n, rank;
	
	// Changing from raw exp to rank for simplicity sake
	rank = points2rank(ch[cn].points_tot);
	
	// Nobility ranks
	if (rank>=20)
	{
		static int gitem[] = {
			IT_POP_SSPEL, 
			IT_POP_SSPEL+1, IT_POP_SSPEL+2, IT_POP_SSPEL+3, 
			IT_POP_SSPEL+4, IT_POP_SSPEL+5, IT_POP_SSPEL+6,
			IT_POP_ISPEL, 
			IT_POP_ISPEL+1, IT_POP_ISPEL+2, IT_POP_ISPEL+3, 
			IT_POP_ISPEL+4, IT_POP_ISPEL+5, IT_POP_ISPEL+6,
			IT_F_SA, IT_F_RU, IT_F_AM, IT_F_TO, IT_F_EM, IT_F_DI,
			IT_F_SP, IT_F_CI, IT_F_OP, IT_F_AQ, IT_F_BE, IT_F_ZI,
			IT_H_SA, IT_H_RU, IT_H_AM, IT_H_TO, IT_H_EM, IT_H_DI,
			IT_H_SP, IT_H_CI, IT_H_OP, IT_H_AQ, IT_H_BE, IT_H_ZI,
			IT_PLAT, IT_GOLD, IT_GOLD,
			IT_POT_RAIN
		};
		static int item[]  = {
			IT_POT_G_HP, IT_POT_G_EN, IT_POT_G_MP,
			IT_POT_H_HP, IT_POT_H_EN, IT_POT_H_MP,
			IT_POT_S_HP, IT_POT_S_EN, IT_POT_S_MP,
			IT_POT_C_HP, IT_POT_C_EN, IT_POT_C_MP,
			IT_POT_L_HP, IT_POT_L_EN, IT_POT_L_MP,
			IT_FLO_W, IT_FLO_Y, IT_FLO_B, IT_FLO_T, IT_FLO_O, IT_FLO_M, IT_FLO_C, IT_FLO_V,
			IT_FLASK
		};
		if (RANDOM(POP_GITEM)) 	{ n = RANDOM(sizeof( item) / sizeof(int)); n =  item[n]; }
		else 					{ n = RANDOM(sizeof(gitem) / sizeof(int)); n = gitem[n]; }
	}
	// High officer
	else if (rank>=16)
	{
		static int gitem[] = {
			IT_POP_ISPEL, 
			IT_POP_ISPEL+1, IT_POP_ISPEL+2, IT_POP_ISPEL+3, 
			IT_POP_ISPEL+4, IT_POP_ISPEL+5, IT_POP_ISPEL+6,
			IT_H_SA, IT_H_RU, IT_H_AM, IT_H_TO, IT_H_EM, IT_H_DI,
			IT_H_SP, IT_H_CI, IT_H_OP, IT_H_AQ, IT_H_BE, IT_H_ZI,
			IT_B_SA, IT_B_RU, IT_B_AM, IT_B_TO, IT_B_EM, IT_B_DI,
			IT_B_SP, IT_B_CI, IT_B_OP, IT_B_AQ,
			IT_GOLD,
			IT_POT_RAIN
		};
		static int item[]  = {
			IT_POT_G_HP, IT_POT_G_EN, IT_POT_G_MP,
			IT_POT_G_HP, IT_POT_G_EN, IT_POT_G_MP,
			IT_FLO_W, IT_FLO_Y, IT_FLO_B, IT_FLO_T, IT_FLO_O,
			IT_FLASK
		};
		if (RANDOM(POP_GITEM)) 	{ n = RANDOM(sizeof( item) / sizeof(int)); n =  item[n]; }
		else 					{ n = RANDOM(sizeof(gitem) / sizeof(int)); n = gitem[n]; }
	}
	// Mid officer
	else if (rank>=12)
	{
		static int gitem[] = {
			IT_POP_ISPEL, 
			IT_POP_ISPEL+1, IT_POP_ISPEL+2, IT_POP_ISPEL+3, 
			IT_POP_ISPEL+4, IT_POP_ISPEL+5, IT_POP_ISPEL+6,
			IT_POP_ASPEL, 
			IT_POP_ASPEL+1, IT_POP_ASPEL+2, IT_POP_ASPEL+3, 
			IT_POP_ASPEL+4, IT_POP_ASPEL+5, IT_POP_ASPEL+6,
			IT_B_SA, IT_B_RU, IT_B_AM, IT_B_TO, IT_B_EM, IT_B_DI,
			IT_B_SP, IT_B_CI, IT_B_OP, IT_B_AQ,
			IT_M_SA, IT_M_RU, IT_M_AM, IT_M_TO, IT_M_EM, IT_M_DI,
			IT_GOLD, IT_GOLD, IT_SILV,
			IT_POT_RAIN
		};
		static int item[]  = {
			IT_POT_N_HP, IT_POT_N_EN, IT_POT_N_MP,
			IT_POT_G_HP, IT_POT_G_EN, IT_POT_G_MP,
			IT_FLO_R, IT_FLO_G, IT_FLO_P, IT_FLO_W, IT_FLO_Y, IT_FLO_B,
			IT_FLASK
		};
		if (RANDOM(POP_GITEM)) 	{ n = RANDOM(sizeof( item) / sizeof(int)); n =  item[n]; }
		else 					{ n = RANDOM(sizeof(gitem) / sizeof(int)); n = gitem[n]; }
	}
	// Low officer
	else if (rank>= 8)
	{
		static int gitem[] = {
			IT_POP_ASPEL, 
			IT_POP_ASPEL+1, IT_POP_ASPEL+2, IT_POP_ASPEL+3, 
			IT_POP_ASPEL+4, IT_POP_ASPEL+5, IT_POP_ASPEL+6,
			IT_M_SA, IT_M_RU, IT_M_AM, IT_M_TO, IT_M_EM, IT_M_DI,
			IT_S_SA, IT_S_RU, IT_S_AM, IT_S_TO, IT_S_EM, IT_S_DI,
			IT_GOLD, IT_SILV, IT_SILV,
			IT_POT_RAIN
		};
		static int item[]  = {
			IT_POT_N_HP, IT_POT_N_EN, IT_POT_N_MP,
			IT_POT_N_HP, IT_POT_N_EN, IT_POT_N_MP,
			IT_FLO_R, IT_FLO_G, IT_FLO_P, IT_FLO_W, IT_FLO_Y, IT_FLO_B,
			IT_FLASK
		};
		if (RANDOM(POP_GITEM)) 	{ n = RANDOM(sizeof( item) / sizeof(int)); n =  item[n]; }
		else 					{ n = RANDOM(sizeof(gitem) / sizeof(int)); n = gitem[n]; }
	}
	// Early ranks
	else if (rank>= 4)
	{
		static int gitem[] = {
			IT_POP_ASPEL, 
			IT_POP_ASPEL+1, IT_POP_ASPEL+2, IT_POP_ASPEL+3, 
			IT_POP_ASPEL+4, IT_POP_ASPEL+5, IT_POP_ASPEL+6,
			IT_POP_SPELL, 
			IT_POP_SPELL+1, IT_POP_SPELL+2, IT_POP_SPELL+3, 
			IT_POP_SPELL+4, IT_POP_SPELL+5, IT_POP_SPELL+6,
			IT_S_SA, IT_S_RU, IT_S_AM, IT_S_TO, IT_S_EM, IT_S_DI,
			IT_SILV,
			IT_POT_RAIN
		};
		static int item[]  = {
			IT_POT_M_HP, IT_POT_M_EN, IT_POT_M_MP,
			IT_POT_N_HP, IT_POT_N_EN, IT_POT_N_MP,
			IT_FLO_R, IT_FLO_G, IT_FLO_P, IT_FLO_W,
			IT_FLASK
		};
		if (RANDOM(POP_GITEM)) 	{ n = RANDOM(sizeof( item) / sizeof(int)); n =  item[n]; }
		else 					{ n = RANDOM(sizeof(gitem) / sizeof(int)); n = gitem[n]; }
	}
	// Newbie
	else
	{
		static int gitem[] = {
			IT_POP_SPELL, 
			IT_POP_SPELL+1, IT_POP_SPELL+2, IT_POP_SPELL+3, 
			IT_POP_SPELL+4, IT_POP_SPELL+5, IT_POP_SPELL+6,
			IT_POT_RAIN
		};
		static int item[]  = {
			IT_POT_M_HP, IT_POT_M_EN, IT_POT_M_MP,
			IT_POT_M_HP, IT_POT_M_EN, IT_POT_M_MP,
			IT_FLO_R, IT_FLO_G, IT_FLO_P, 
			IT_FLASK
		};
		if (RANDOM(POP_GITEM)) 	{ n = RANDOM(sizeof( item) / sizeof(int)); n =  item[n]; }
		else 					{ n = RANDOM(sizeof(gitem) / sizeof(int)); n = gitem[n]; }
	}
	
	if (n)
	{
		n = god_create_item(n);

		chlog(cn, "got %s (t=%d)", it[n].name, it[n].temp);
	}
	return(n);
}

/*	Added by SoulHunter  04.04.2000	*/
int pop_create_bonus_belt(int cn)
{
	int n = 1106;   // value to store id-number of created belt
	                // at start it contains template of 'rainbow_belt'
	int i, j;
	int rank, skill_value, skill_number, skm;


//	item will be created with chance 1 from 10000
//	if (RANDOM(10000)) return 0;
//      it is moved outside the function cause item is rarely created
//      but function is called each time when NPC respawning.
//	so it will be much faster if it will check for chances before
//	the function call

	// MAX_POWER = rank
	// MAX_SKILLS = rank
	rank = points2rank(ch[cn].points_tot);  // private wont get this belt
	if(!rank)
	{
		return 0;
	}
	if (n)
	{
		n = god_create_item(n);   // creating belt from template
		if(!n)
		{
			return 0;     // return if failed
		}
		// problem is if we keep template for newly created and
		// then changed item, it sometimes doesnt keep changes
		// we need template number for is_belt() function
		// but seems there is some checks by template so we reset it
		// and people wont notice belt :)
		it[n].temp = 0; // clearing template
		it[n].sprite[0] = 16964;
		strcpy(it[n].name, "Rainbow Belt");
		strcpy(it[n].reference, "rainbow belt");
		strcpy(it[n].description, "An ancient belt. It seems to be highly magical, and highly volatile.");
		// putting message about created belt into log-file
		chlog(cn, ", with rank %d, got %s (t=%d)", rank, it[n].name, it[n].temp);
	}

	j = RANDOM(rank+1)+1; // how many skills will be in belt?
	it[n].power += 5 * j;     // counting power of item, *remind* power = 10 in template
	it[n].value += 10000 * j; // counting price value, value = 100 in template
	// here we decide which skills will be in belt, not more than rank
	for(i = 0; i < j; i++)
	{
		skill_number = RANDOM(60);			// which skill it will be
		skill_value  = RANDOM(rank+2);		// with that value of skill
		skill_value  = skill_value >> 1;	// divide it by 2, cause it cant be more than 12 (max_rank/2)
		if(skill_value == 0)
		{
			skill_value = 1;              	// and cant be zero
		}
		// the following code put all these skills in belt
		// sometimes requirements are zeroed, cause if we have, in example,
		// dagger and sword skills in belt, this belt can be used only by
		// templar/seyan, but i dont want this
		switch(skill_number)
		{
			case 0:
				it[n].attrib[AT_BRV][0] += skill_value; 								// this line is how much it will raise attribute
				if(it[n].attrib[AT_BRV][0] > 12) it[n].attrib[AT_BRV][0] = 12; 			// this will check for max level = 12 and will down it back to 12
				it[n].attrib[AT_BRV][2] = 10 + (it[n].attrib[AT_BRV][0] * RANDOM(7)); 	// this line will set requirements
				break;
			case 1:
				it[n].attrib[AT_WIL][0] += skill_value;
				if(it[n].attrib[AT_WIL][0] > 12) it[n].attrib[AT_WIL][0] = 12;
				it[n].attrib[AT_WIL][2] = 10 + (it[n].attrib[AT_WIL][0] * RANDOM(7));
				break;
			case 2:
				it[n].attrib[AT_INT][0] += skill_value;
				if(it[n].attrib[AT_INT][0] > 12) it[n].attrib[AT_INT][0] = 12;
				it[n].attrib[AT_INT][2] = 10 + (it[n].attrib[AT_INT][0] * RANDOM(7));
				break;
			case 3:
				it[n].attrib[AT_AGL][0] += skill_value;
				if(it[n].attrib[AT_AGL][0] > 12) it[n].attrib[AT_AGL][0] = 12;
				it[n].attrib[AT_AGL][2] = 10 + (it[n].attrib[AT_AGL][0] * RANDOM(7));
				break;
			case 4:
				it[n].attrib[AT_STR][0] += skill_value;
				if(it[n].attrib[AT_STR][0] > 12) it[n].attrib[AT_STR][0] = 12;
				it[n].attrib[AT_STR][2] = 10 + (it[n].attrib[AT_STR][0] * RANDOM(7));
				break;
			case 5:
				it[n].hp[0] += (skill_value * 5);
				if(it[n].hp[0] > 60) it[n].hp[0] = 60;
				it[n].hp[2] = 50 + (it[n].hp[0] * RANDOM(7));
				break;
			case 6:
				it[n].end[0] += (skill_value * 5)/2;
				if(it[n].end[0] > 30) it[n].end[0] = 30;
				break;
			case 7:
				it[n].mana[0] += (skill_value * 5);
				if(it[n].mana[0] > 60) it[n].mana[0] = 60;
				it[n].mana[2] = 50 + (it[n].mana[0] * RANDOM(7));
				break;
			case 8:
				it[n].armor[0] += (skill_value+1)/2;
				if(it[n].armor[0] > 6) it[n].armor[0] = 6;
				break;
			case 9:
				it[n].weapon[0] += (skill_value+1)/2;
				if(it[n].weapon[0] > 6) it[n].armor[0] = 6;
				break;
			default:
				skm = skill_number-10;
				it[n].skill[skm][0] += skill_value;
				if(it[n].skill[skm][0] > 12) it[n].skill[skm][0] = 12;
				// Check for skills that *all* classes would know, to apply an equip requirement.
				if (skm == SK_HAND		|| skm == SK_REST    || skm == SK_MEDIT 
				 || skm == SK_HEAL		|| skm == SK_IDENT   || skm == SK_REPAIR 
				 || skm == SK_RESIST	|| skm == SK_PERCEPT || skm == SK_BARTER)
				{
					it[n].skill[skm][2] = (it[n].skill[skm][0] * RANDOM(7));
				}
				break;
		}
	}

	return(n);

}
/*	--end	*/

int pop_create_char(int n, int drop)
{
	int cn, tmp, m, flag = 0, hasitems = 0, chance;

	for (cn = 1; cn<MAXCHARS; cn++)
	{
		if (ch[cn].used==USE_EMPTY)
		{
			break;
		}
	}
	if (cn==MAXCHARS)
	{
		xlog("MAXCHARS reached!\n");
		return 0;
	}

	ch[cn] = ch_temp[n];
	ch[cn].pass1 = RANDOM(0x3fffffff);
	ch[cn].pass2 = RANDOM(0x3fffffff);
	ch[cn].temp  = n;

	for (m = 0; m<40; m++)
	{
		if ((tmp = ch[cn].item[m])!=0)
		{
			tmp = god_create_item(tmp);
			if (!tmp)
			{
				flag = 1;
				ch[cn].item[m] = 0;
			}
			else
			{
				it[tmp].carried = cn;
				ch[cn].item[m]  = tmp;
				hasitems = 1;
			}
		}
	}

	for (m = 0; m<20; m++)
	{
		if ((tmp = ch[cn].worn[m])!=0)
		{
			tmp = pop_create_item(tmp, cn);
			if (!tmp)
			{
				flag = 1;
				ch[cn].worn[m] = 0;
			}
			else
			{
				it[tmp].carried = cn;
				ch[cn].worn[m]  = tmp;
				hasitems = 1;
			}
		}
	}

	for (m = 0; m<MAXBUFFS; m++)
	{
		if (ch[cn].spell[m]!=0)
		{
			ch[cn].spell[m] = 0;
		}
	}

	if ((tmp = ch[cn].citem)!=0)
	{
		tmp = god_create_item(tmp);
		if (!tmp)
		{
			flag = 1;
			ch[cn].citem = 0;
		}
		else
		{
			it[tmp].carried = cn;
			ch[cn].citem = tmp;
		}
	}

	if (flag)
	{
		god_destroy_items(cn);
		ch[cn].used = USE_EMPTY;
		return 0;
	}

	ch[cn].a_end = 1000000;
	ch[cn].a_hp  = 1000000;
	if (B_SK(cn, SK_MEDIT))
	{
		ch[cn].a_mana = 1000000;
	}
	else
	{
		ch[cn].a_mana = RANDOM(8) * RANDOM(8) * RANDOM(8) * RANDOM(8) * 100;
	}
	ch[cn].dir = DX_DOWN;
	ch[cn].data[92] = TICKS * 60;

	chance = 25;
	if (!B_SK(cn, SK_MEDIT) && ch[cn].a_mana>15 * 100)
	{
		chance -= 6;
	}
	if (!B_SK(cn, SK_MEDIT) && ch[cn].a_mana>30 * 100)
	{
		chance -= 6;
	}
	if (!B_SK(cn, SK_MEDIT) && ch[cn].a_mana>65 * 100)
	{
		chance -= 6;
	}

	if (ch[cn].alignment<0)
	{
		for (m = 0; m<40; m++)
		{
			if (ch[cn].item[m]==0 && hasitems)
			{
				// this check placed here for speed reasons
				// they are the same as in pop_create_bonus_belt()
				if (RANDOM(chance) == 0)
				{
					tmp = pop_create_bonus(cn, chance);
					if (tmp)
					{
						it[tmp].carried = cn;
						ch[cn].item[m]  = tmp;
					}
				}
				break;
			}
		}

/*	Added by SoulHunter  04.04.2000	*/
		// creating rainbow belts
		for (m = 0; m<40; m++)
		{
			if (ch[cn].item[m]==0 && hasitems)
			{
				//	item will be created with chance 1 from 10000
				// this check placed here for speed reasons
				if (RANDOM(10000) == 0)
				{
					tmp = pop_create_bonus_belt(cn);
					if (tmp)
					{
						it[tmp].carried = cn;
						ch[cn].item[m]  = tmp;
					}
				}
				break;
			}
		}
/*	--end	*/

	}

	if (drop)
	{
		if (!god_drop_char(cn, ch[cn].x, ch[cn].y))
		{
			printf("Could not drop char %d\n", n);
			god_destroy_items(cn);
			ch[cn].used = USE_EMPTY;
			return 0;
		}
	}

	do_update_char(cn);
	globs->npcs_created++;
	return(cn);
}

void reset_char(int n)
{
	int cn, m, z, pts = 0, cnt = 0;

	if (n<1 || n>=MAXTCHARS)
	{
		return;
	}
	if (!ch_temp[n].used)
	{
		return;
	}
	if (!(ch_temp[n].flags & CF_RESPAWN))
	{
		return;
	}

	xlog("Resetting char %d (%s)", n, ch_temp[n].name);

	for (z = 0; z<5; z++)
	{
		for (m = 10; m<ch_temp[n].attrib[z][0]; m++)
		{
			pts += attrib_needed(m, 4);
		}
	}

	for (m = 50; m<ch_temp[n].hp[0]; m++)
	{
		pts += hp_needed(m, 4);
	}

	//for (m = 50; m<ch_temp[n].end[0]; m++)
	//{
	//	pts += end_needed(m, 2);
	//}

	for (m = 50; m<ch_temp[n].mana[0]; m++)
	{
		pts += mana_needed(m, 4);
	}

	for (z = 0; z<MAXSKILL; z++)
	{
		for (m = 1; m<ch_temp[n].skill[z][0]; m++)
		{
			pts += skill_needed(m, 3);
		}
	}

	ch_temp[n].points_tot = pts;

	for (cn = 1; cn<MAXCHARS; cn++)
	{
		if (ch[cn].used!=USE_ACTIVE)
		{
			continue;
		}
		if (ch[cn].temp==n)
		{
			xlog(" --> %s (%d) (%d,%d).", ch[cn].name, cn, ch[cn].x, ch[cn].y);
			god_destroy_items(cn);
			plr_map_remove(cn);
			ch[cn].used = USE_EMPTY;
			cnt++;
		}
	}

	for (m = 0; m<MAXEFFECT; m++)
	{
		if (fx[m].used!=USE_ACTIVE)
		{
			continue;
		}
		if (fx[m].type==2 && fx[m].data[2]==n)
		{
			xlog(" --> effect %d", m);
			fx[m].used = USE_EMPTY;
			cnt++;
		}
	}

	for (m = 0; m<MAXITEM; m++)
	{
		if (it[m].used!=USE_ACTIVE)
		{
			continue;
		}
		if (it[m].driver==7 && (cn = it[m].data[0])!=0)
		{
			if (ch[cn].temp==n)
			{
				xlog(" --> grave %d", m);
				god_destroy_items(cn);
				ch[cn].used = USE_EMPTY;
				it[m].data[0] = 0;
				cnt++;
			}
		}
	}

	if (cnt!=1)
	{
		xlog("AUTO-RESPAWN: Found %d instances of %s (%d)", cnt, ch_temp[n].name, n);
	}

	if (ch_temp[n].used==USE_ACTIVE)   // schedule respawn
	{
		fx_add_effect(2, TICKS * 10, ch_temp[n].x, ch_temp[n].y, n); // 10 seconds
	}

}

int skillcost(int val, int dif, int start)
{
	int n, p = 0;

	for (n = start; n<val; n++)
	{
		p += skill_needed(n, dif);
	}

	return(p);
}

void pop_skill(void)
{
	int cn, n, t, p;

	for (cn = 1; cn<MAXCHARS; cn++)
	{
		if (!(ch[cn].flags & (CF_PLAYER)))
		{
			continue;
		}
		t = ch[cn].temp;

		for (n = 0; n<MAXSKILL; n++)
		{
			if (B_SK(cn, n)==0 && ch_temp[t].skill[n][0])
			{
				B_SK(cn, n) = ch_temp[t].skill[n][0];
				xlog("added %s to %s", skilltab[n].name, ch[cn].name);
			}
			if (ch_temp[t].skill[n][2]<B_SK(cn, n))
			{
				p = skillcost(B_SK(cn, n), ch[cn].skill[n][3], ch_temp[t].skill[n][2]);
				xlog("reduced %s on %s from %d to %d, added %d exp",
				     skilltab[n].name,
				     ch[cn].name,
				     B_SK(cn, n),
				     ch_temp[t].skill[n][2],
				     p);
				B_SK(cn, n) = ch_temp[t].skill[n][2];
				ch[cn].points += p;
			}

			ch[cn].skill[n][1] = ch_temp[t].skill[n][1];
			ch[cn].skill[n][2] = ch_temp[t].skill[n][2];
			ch[cn].skill[n][3] = ch_temp[t].skill[n][3];
		}
	}
	xlog("Changed Skills.");
}

void reset_item(int n)
{
	int in;
	struct item tmp;

	if (n<2 || n>=MAXTITEM)
	{
		return;                 // never reset blank template (1) stuff

	}
	xlog("Resetting item %d (%s)", n, it_temp[n].name);

	for (in = 1; in<MAXITEM; in++)
	{
		if (it[in].used!=USE_ACTIVE)
		{
			continue;
		}
		if (it[in].flags & (IF_SPELL|IF_STACKABLE|IF_SOULSPLIT|IF_SOULSTONE))
		{
			continue;
		}
		if (it[in].temp==n)
		{
			xlog(" --> %s (%d) (%d, %d,%d).", it[in].name, in, it[in].carried, it[in].x, it[in].y);
			// make light calculations and update characters!!!
			
			if (it[in].flags & (IF_AUGMENTED|IF_DIMINISHED|IF_EASEUSE|IF_ENCHANTED))
			{
				if (it[in].flags & (IF_AUGMENTED|IF_DIMINISHED|IF_EASEUSE))
				{
					it[in].flags |= IF_UPDATE | IF_NOREPAIR | IF_LEGACY;
					it[in].max_damage = 100000;
				}
				if ((it[in].flags & IF_ENCHANTED) && !(it_temp[n].flags & IF_CAN_EN))
				{
					it[in].flags |= IF_UPDATE;
					it[in].flags &= ~IF_ENCHANTED;
					it[in].enchantment = 0;
				}
				continue;
			}
			
			if ((it_temp[n].flags & (IF_TAKE | IF_LOOK | IF_LOOKSPECIAL | IF_USE | IF_USESPECIAL)) || it[in].carried)
			{
				tmp = it_temp[n];
				tmp.x = it[in].x;
				tmp.y = it[in].y;
				tmp.carried = it[in].carried;
				tmp.temp = n;
				// do we need to copy more ? !!!
				it[in] = tmp;
			}
			else
			{
				map[it[in].x + it[in].y * MAPX].it = 0;
				it[in].used = USE_EMPTY;
				map[it[in].x + it[in].y * MAPX].fsprite = it_temp[n].sprite[0];
				if (it_temp[n].flags & IF_MOVEBLOCK)
				{
					map[it[in].x + it[in].y * MAPX].flags |= MF_MOVEBLOCK;
				}
				if (it_temp[n].flags & IF_SIGHTBLOCK)
				{
					map[it[in].x + it[in].y * MAPX].flags |= MF_SIGHTBLOCK;
				}
			}
		}
	}
}

void reset_changed_items(void)
{
	static int changelist[] = {};
	int n;

	for (n = 0; n<sizeof(changelist) / sizeof(int); n++)
	{
		reset_item(changelist[n]);
	}
}

#define RESETTICKER (TICKS*60)

void pop_tick(void)
{
	static int last_reset = 0;
	int nr;

	if (globs->ticker - last_reset>=RESETTICKER)      // reset one character per minute
	{
		nr = (globs->ticker / RESETTICKER) % MAXTCHARS;
		if (nr>0 && nr<MAXTCHARS)               // yes, we're paranoid :)
		{
			reset_char(nr);
		}
		last_reset = globs->ticker;
	}

	if (globs->reset_char)
	{
		reset_char(globs->reset_char);
		globs->reset_char = 0;
	}
	if (globs->reset_item)
	{
		reset_item(globs->reset_item);
		globs->reset_item = 0;
	}
}

void pop_reset_all(void)
{
	int n;

	for (n = 1; n<MAXTCHARS; n++)
	{
		if (ch_temp[n].used!=USE_EMPTY)
		{
			reset_char(n);
		}
	}
	for (n = 1; n<MAXTITEM; n++)
	{
		if (it_temp[n].used!=USE_EMPTY && it_temp[n].driver!=36 && it_temp[n].driver!=38)
		{
			reset_item(n);
		}
	}
}

void pop_wipe(void)
{
	int n;

	for (n = 1; n<MAXCHARS; n++)
	{
		if (ch[n].used==USE_EMPTY)
		{
			continue;
		}
		god_destroy_items(n);
		if (ch[n].used==USE_ACTIVE)
		{
			plr_map_remove(n);
		}

		ch[n].used = USE_EMPTY;
	}

	for (n = 1; n<MAXITEM; n++)
	{
		if (it[n].used==USE_EMPTY)
		{
			continue;
		}
		if (!(it[n].flags & IF_TAKE) && it[n].driver!=7)
		{
			continue;
		}
		if (it[n].used==USE_ACTIVE)
		{
			map[it[n].x + it[n].y * MAPX].it = 0;
		}
		it[n].used = USE_EMPTY;
	}

	for (n = 1; n<MAXEFFECT; n++)
	{
		if (fx[n].used==USE_EMPTY)
		{
			continue;
		}
		fx[n].used = USE_EMPTY;
	}

	globs->players_created = 0;
	globs->npcs_created = 0;
	globs->players_died = 0;
	globs->npcs_died  = 0;
	globs->expire_cnt = 0;
	globs->expire_run = 0;
	globs->gc_cnt = 0;
	globs->gc_run = 0;
	globs->lost_cnt = 0;
	globs->lost_run = 0;
	globs->reset_char = 0;
	globs->reset_item = 0;
	globs->total_online_time = 0;
	globs->uptime = 0;
	for (n = 0; n<24; n++)
	{
		globs->online_per_hour[n] = 0;
	}
}

void pop_remove(void)
{
	int h1, h2, h3, n, m, in, chc = 0, itc = 0;

	xlog("Saving players...");

	h1 = open(".tmp/char.dat", O_RDWR | O_CREAT | O_TRUNC, 0600);
	h2 = open(".tmp/item.dat", O_RDWR | O_CREAT | O_TRUNC, 0600);
	h3 = open(".tmp/global.dat", O_RDWR | O_CREAT | O_TRUNC, 0600);

	if (h1==-1 || h2==-1 || h3==-1)
	{
		xlog("pop remove failed: could not create temporary file.");
		return;
	}

	for (n = 1; n<MAXCHARS; n++)
	{
		if (ch[n].used==USE_EMPTY)
		{
			continue;
		}
		if (!(ch[n].flags & (CF_PLAYER)))
		{
			continue;
		}
		if (ch[n].flags & CF_BODY)
		{
			continue;
		}

		for (m = 0; m<40; m++)
		{
			if ((in = ch[n].item[m])!=0)
			{
				write(h2, &it[in], sizeof(struct item));
				itc++;
			}
		}

		for (m = 0; m<20; m++)
		{
			if ((in = ch[n].worn[m])!=0)
			{
				write(h2, &it[in], sizeof(struct item));
				itc++;
			}
		}
		
		for (m = 0; m<12; m++)
		{
			if ((in = ch[n].alt_worn[m])!=0)
			{
				write(h2, &it[in], sizeof(struct item));
				itc++;
			}
		}

		for (m = 0; m<MAXBUFFS; m++)
		{
			if ((in = ch[n].spell[m])!=0)
			{
				write(h2, &bu[in], sizeof(struct item));
				itc++;
			}
		}

		for (m = 0; m<62; m++)
		{
			if ((in = ch[n].depot[m])!=0)
			{
				write(h2, &it[in], sizeof(struct item));
				itc++;
			}
		}

		ch[n].data[99] = n;
		write(h1, &ch[n], sizeof(struct character));
		chc++;
	}

	write(h3, globs, sizeof(struct global));

/*	for (m=0; m<MAPX*MAPY; m++) {
                if ((in=map[m].it)!=0) {
                        if (reason to keep a takeable object) {

                                write(h2,&it[in],sizeof(struct item));
                                itc++;
                        ]
                }
        }*/

	close(h3);
	close(h2);
	close(h1);

	xlog("Saved %d chars, %d items.", chc, itc);
}

void pop_load(void)
{
	int h1, h2, h3, n, m, in, chc = 0, itc = 0;
	struct character ctmp;
	struct item itmp;

	xlog("Loading players...");

	h1 = open(".tmp/char.dat", O_RDONLY);
	h2 = open(".tmp/item.dat", O_RDONLY);
	h3 = open(".tmp/global.dat", O_RDONLY);

	if (h1==-1 || h2==-1 || h3==-1)
	{
		xlog("pop load failed: could not open temporary file.");
		return;
	}

	while (1)
	{
		if (read(h1, &ctmp, sizeof(struct character))<1)
		{
			break;
		}
		n = ctmp.data[99];

		if (ch[n].used!=USE_EMPTY)
		{
			xlog("Character slot %d to be loaded to not empty!", n);
			for (m = 0; m<40; m++)
			{
				if ((in = ch[n].item[m])!=0)
				{
					it[in].used = USE_EMPTY;
				}
			}
			for (m = 0; m<MAXBUFFS; m++)
			{
				if ((in = ch[n].spell[m])!=0)
				{
					bu[in].used = USE_EMPTY;
				}
			}
			for (m = 0; m<20; m++)
			{
				if ((in = ch[n].worn[m])!=0)
				{
					it[in].used = USE_EMPTY;
				}
			}
			for (m = 0; m<12; m++)
			{
				if ((in = ch[n].alt_worn[m])!=0)
				{
					it[in].used = USE_EMPTY;
				}
			}
			if ((in = ch[n].citem)!=0)
			{
				it[in].used = USE_EMPTY;
			}
		}
		ch[n] = ctmp;
		chc++;

		for (m = 0; m<40; m++)
		{
			if (ch[n].item[m])
			{
				for (in = 1; in<MAXITEM; in++)
				{
					if (it[in].used==USE_EMPTY)
					{
						break;
					}
				}
				if (in==MAXITEM)
				{
					xlog("MAXITEM reached.");
					break;
				}

				read(h2, &it[in], sizeof(struct item));
				itc++;

				ch[n].item[m] = in;
			}
		}

		for (m = 0; m<20; m++)
		{
			if (ch[n].worn[m])
			{
				for (in = 1; in<MAXITEM; in++)
				{
					if (it[in].used==USE_EMPTY)
					{
						break;
					}
				}
				if (in==MAXITEM)
				{
					xlog("MAXITEM reached.");
					break;
				}

				read(h2, &it[in], sizeof(struct item));
				itc++;

				ch[n].worn[m] = in;
			}
		}
		
		for (m = 0; m<12; m++)
		{
			if (ch[n].alt_worn[m])
			{
				for (in = 1; in<MAXITEM; in++)
				{
					if (it[in].used==USE_EMPTY)
					{
						break;
					}
				}
				if (in==MAXITEM)
				{
					xlog("MAXITEM reached.");
					break;
				}

				read(h2, &it[in], sizeof(struct item));
				itc++;

				ch[n].alt_worn[m] = in;
			}
		}

		for (m = 0; m<MAXBUFFS; m++)
		{
			if (ch[n].spell[m])
			{
				for (in = 1; in<MAXBUFF; in++)
				{
					if (bu[in].used==USE_EMPTY)
					{
						break;
					}
				}
				if (in==MAXBUFF)
				{
					xlog("MAXBUFF reached.");
					break;
				}

				read(h2, &bu[in], sizeof(struct item));
				itc++;

				ch[n].spell[m] = in;
			}
		}

		for (m = 0; m<62; m++)
		{
			if (ch[n].depot[m])
			{
				for (in = 1; in<MAXITEM; in++)
				{
					if (it[in].used==USE_EMPTY)
					{
						break;
					}
				}
				if (in==MAXITEM)
				{
					xlog("MAXITEM reached.");
					break;
				}

				read(h2, &it[in], sizeof(struct item));
				itc++;

				ch[n].depot[m] = in;
			}
		}
		if (ch[n].flags & (CF_PLAYER))
		{
			globs->players_created++;                      // this is useless now (?)
		}
		else
		{
			map[ch[n].x + ch[n].y * MAPX].ch = n;
		}
	}

	read(h3, globs, sizeof(struct global));

	while (1)
	{
		if (read(h2, &itmp, sizeof(struct item))<1)
		{
			break;
		}
		for (m = 1; m<MAXITEM; m++)
		{
			if (it[m].used==USE_EMPTY)
			{
				break;
			}
		}
		if (m==MAXITEM)
		{
			xlog("PANIC: Maxitem reached!");
			break;
		}
		it[m] = itmp;
		in = map[it[m].x + it[m].y * MAPX].it;
		if (in)
		{
			xlog("Destroyed object %s (%d) on %d,%d",
			     it[in].name, in,
			     it[in].x, it[in].y);
			it[in].used = USE_EMPTY;
		}
		map[it[m].x + it[m].y * MAPX].it = m;
		xlog("Dropped object %s (%d) on %d,%d",
		     it[m].name, in,
		     it[m].x, it[m].y);
		itc++;
	}

	close(h3);
	close(h2);
	close(h1);

	xlog("Loaded %d chars, %d items.", chc, itc);
}

void populate(void)
{
	int n, m;

	xlog("Started Populate");

	/*xlog("Load");

	   pop_load();*/

	xlog("Creating NPCs from Templates");

	for (n = 1; n<MAXTCHARS; n++)
	{
		if (ch_temp[n].used!=USE_EMPTY)
		{
			for (m = 1; m<MAXCHARS; m++)
			{
				if (ch[m].used!=USE_ACTIVE)
				{
					continue;
				}
				if (ch[m].flags & CF_BODY)
				{
					continue;
				}
				if (ch[m].temp==n)
				{
					break;
				}
			}
			if (m==MAXCHARS)
			{
				reset_char(n);
			}
		}
	}
}

void pop_save_char(int nr)
{
	int h1, m, in, chc = 0, itc = 0;
	char buf[80];
	unsigned long long prof;

	if (ch[nr].used==USE_EMPTY || !(ch[nr].flags & (CF_PLAYER)) || (ch[nr].flags & CF_BODY) || !(ch[nr].flags & CF_SAVEME))
	{
		return;
	}

	prof = prof_start();

	ch[nr].flags &= ~CF_SAVEME;

	//xlog("Saving player %d",nr);

	sprintf(buf, ".tmp/%d.chdat", nr);

	h1 = open(buf, O_RDWR | O_CREAT | O_TRUNC, 0600);

	if (h1==-1)
	{
		xlog("pop remove failed: could not create file %s.", buf);
		prof_stop(45, prof);
		return;
	}

	// save player
	ch[nr].data[99] = nr;
	write(h1, &ch[nr], sizeof(struct character));
	chc++;

	// save inventory
	for (m = 0; m<40; m++)
	{
		if ((in = ch[nr].item[m])!=0)
		{
			write(h1, &it[in], sizeof(struct item));
			itc++;
		}
	}

	// save worn items
	for (m = 0; m<20; m++)
	{
		if ((in = ch[nr].worn[m])!=0)
		{
			write(h1, &it[in], sizeof(struct item));
			itc++;
		}
	}
	
	// save alt_worn items
	for (m = 0; m<12; m++)
	{
		if ((in = ch[nr].alt_worn[m])!=0)
		{
			write(h1, &it[in], sizeof(struct item));
			itc++;
		}
	}

	// save depot contents
	for (m = 0; m<62; m++)
	{
		if ((in = ch[nr].depot[m])!=0)
		{
			write(h1, &it[in], sizeof(struct item));
			itc++;
		}
	}

	close(h1);

	//xlog("Saved %d chars, %d items.",chc,itc);
	prof_stop(45, prof);
}

void pop_load_char(int nr)
{
	int h1, m, in, chc = 0, itc = 0;
	struct character ctmp;
	char buf[80];

	sprintf(buf, ".tmp/%d.chdat", nr);

	h1 = open(buf, O_RDONLY);

	if (h1==-1)
	{
		return;
	}

	if (read(h1, &ctmp, sizeof(struct character))<1 || ctmp.data[99]!=nr)
	{
		xlog("data seems to be corrupt. Giving up.");
		return;
	}

	if (ch[nr].used!=USE_EMPTY)
	{
		xlog("Character slot %d to be loaded to not empty!", nr);
		for (m = 0; m<40; m++)
		{
			if ((in = ch[nr].item[m])!=0)
			{
				it[in].used = USE_EMPTY;
			}
		}
		for (m = 0; m<MAXBUFFS; m++)
		{
			if ((in = ch[nr].spell[m])!=0)
			{
				bu[in].used = USE_EMPTY;
			}
		}
		for (m = 0; m<20; m++)
		{
			if ((in = ch[nr].worn[m])!=0)
			{
				it[in].used = USE_EMPTY;
			}
		}
		for (m = 0; m<12; m++)
		{
			if ((in = ch[nr].alt_worn[m])!=0)
			{
				it[in].used = USE_EMPTY;
			}
		}
		for (m = 0; m<62; m++)
		{
			if ((in = ch[nr].depot[m])!=0)
			{
				it[in].used = USE_EMPTY;
			}
		}
		if ((in = ch[nr].citem)!=0)
		{
			it[in].used = USE_EMPTY;
		}
	}
	ch[nr] = ctmp;
	chc++;

	for (m = 0; m<40; m++)
	{
		if (ch[nr].item[m])
		{
			for (in = 1; in<MAXITEM; in++)
			{
				if (it[in].used==USE_EMPTY)
				{
					break;
				}
			}
			if (in==MAXITEM)
			{
				xlog("MAXITEM reached.");
				break;
			}

			read(h1, &it[in], sizeof(struct item));
			itc++;

			ch[nr].item[m] = in;
		}
	}

	for (m = 0; m<MAXBUFFS; m++)
	{
		ch[nr].spell[m] = 0;
	}

	for (m = 0; m<20; m++)
	{
		if (ch[nr].worn[m])
		{
			for (in = 1; in<MAXITEM; in++)
			{
				if (it[in].used==USE_EMPTY)
				{
					break;
				}
			}
			if (in==MAXITEM)
			{
				xlog("MAXITEM reached.");
				break;
			}

			read(h1, &it[in], sizeof(struct item));
			itc++;

			ch[nr].worn[m] = in;
		}
	}
	
	for (m = 0; m<12; m++)
	{
		if (ch[nr].alt_worn[m])
		{
			for (in = 1; in<MAXITEM; in++)
			{
				if (it[in].used==USE_EMPTY)
				{
					break;
				}
			}
			if (in==MAXITEM)
			{
				xlog("MAXITEM reached.");
				break;
			}

			read(h1, &it[in], sizeof(struct item));
			itc++;

			ch[nr].alt_worn[m] = in;
		}
	}

	for (m = 0; m<62; m++)
	{
		if (ch[nr].depot[m])
		{
			for (in = 1; in<MAXITEM; in++)
			{
				if (it[in].used==USE_EMPTY)
				{
					break;
				}
			}
			if (in==MAXITEM)
			{
				xlog("MAXITEM reached.");
				break;
			}

			read(h1, &it[in], sizeof(struct item));
			itc++;

			ch[nr].depot[m] = in;
		}
	}
	if (ch[nr].flags & (CF_PLAYER))
	{
		globs->players_created++;                       // this is useless now (?)
	}
	else
	{
		map[ch[nr].x + ch[nr].y * MAPX].ch = nr;
	}

	close(h1);
}

void pop_load_all_chars(void)
{
	int n;

	for (n = 1; n<MAXCHARS; n++)
	{
		pop_load_char(n);
	}
}

void pop_save_all_chars(void)
{
	int n;

	for (n = 1; n<MAXCHARS; n++)
	{
		ch[n].flags |= CF_SAVEME;
		pop_save_char(n);
	}
}

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
	int n = 0, m, in = 0, enbl = 0, godroll = 0, bonus = 1;
	char *gend, *godn, name[60], refer[60], descr[220];
	
	if (!(it_temp[temp].flags & IF_TAKE))
	{
		xlog("pop_create_item: bad template number %d", temp);
		return 0;
	}
	if (is_unique_able(temp))
	{
		if (!in && ch[cn].alignment<0 && !RANDOM(1000)) // Reward a unique weapon
		{	godroll = RANDOM(4)+1; 						// Decide which god blessed the weapon.
			/*
			for (m = 1, n = 0; m<MAXITEM; m++)			// Check for up to three copies
			{
				if (it[m].used==USE_EMPTY || it[m].orig_temp!=temp) continue;
				if ((godroll == 1 && IS_SKUAWEAP(m)) ||
					(godroll == 2 && IS_KWAIWEAP(m)) ||
					(godroll == 3 && IS_GORNWEAP(m)) ||
					(godroll == 4 && IS_PURPWEAP(m)) ) n++;
				if (n>=3) break;
			}
			if (n<3)*/
			in = god_create_item(temp); 			// create the item
			if (in)										// assure the item is created
			{
				if (IS_TWOHAND(in))
				{
					bonus = 2;
				}
				
				switch (godroll)
				{
					case 1:
						gend = " god ";
						godn = "Skua";
						it[in].flags |= IF_KWAI_UNI | IF_GORN_UNI;
						it[in].speed[0]      +=  9 * bonus/2;
						break;
					case 2:
						gend = " goddess ";
						godn = "Kwai";
						it[in].flags |= IF_KWAI_UNI;
						it[in].to_hit[0]     +=  2 * bonus;
						it[in].to_parry[0]   +=  2 * bonus;
						break;
					case 3:
						gend = " god ";
						godn = "Gorn";
						it[in].flags |= IF_GORN_UNI;
						it[in].spell_mod[0]  +=  3 * bonus/2;
						break;
					default:
						gend = " ";
						godn = "Purple One";
						it[in].flags |= IF_PURP_UNI;
						it[in].top_damage[0] += 15 * bonus;
						break;
				}
				if (it_temp[temp].armor[0] && it_temp[temp].weapon[0])
				{
					it[in].armor[0]  += 1 + 1 * bonus;
					it[in].weapon[0] += 1 + 1 * bonus;
				}
				else if (it_temp[temp].armor[0])
					it[in].armor[0]  += 2 + 2 * bonus;
				else if (it_temp[temp].weapon[0])
					it[in].weapon[0] += 2 + 2 * bonus;
				
				it[in].orig_temp = it[in].temp;
				it[in].temp = 0;
				it[in].max_damage = 0;
				it[in].flags |= IF_SINGLEAGE | IF_SHOPDESTROY | IF_NOMARKET | IF_UNIQUE | IF_NOREPAIR;
				it[in].flags &= ~(IF_CAN_SS | IF_CAN_EN);
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
		}
	}
	if ((m = is_osiris_weap(temp)) && (it_temp[temp].flags & IF_CAN_SS) && !RANDOM(4))
	{
		in = god_create_item(temp);		// create the item
		if (in)							// assure the item is created
		{
			it[in].orig_temp = it[in].temp;
			it[in].temp = 0;
			it[in].max_damage = 0;
			it[in].stack = 2 + m/2 + RANDOM((m*3+1)/2);
			switch (m)
			{
				case  4: it[in].data[0] = 1; // 4!9 = 5005 x 1 = 5005k
				case  3: it[in].data[0] = 2; // 3!7 = 1716 x 2 = 3432k
				case  2: it[in].data[0] = 4; // 2!5 =  462 x 4 = 1848k
				default: it[in].data[0] = 8; // 1!3 =   84 x 8 =  672k
			}
			it[in].data[1] = it[in].data[2] = it[in].data[3] = it[in].data[4] = 0;
			it[in].flags |= IF_IDENTIFIED | IF_SOULSTONE | IF_NOREPAIR | IF_PURP_UNI | IF_GORN_UNI;
			it[in].flags &= ~IF_CAN_SS;
			strcpy(descr, it[in].description);
			sprintf(it[in].description, "%s It has been blessed by the god Osiris.", descr);
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
		xlog("%s got blessed %s.", ch[cn].name, it[in].name);
	}

	return in;
}

#define POP_GITEM	8

int pop_create_bonus(int cn)
{
	int n, rank;
	
	// Changing from raw exp to rank for simplicity sake
	rank = getrank(cn);
	
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
			IT_PLAT, IT_PLAT,
			MCT_CONTRACT, MCT_QUILL_B, MCT_QUILL_R, 
			IT_POT_RAIN
		};
		static int item[]  = {
			IT_POT_G_HP, IT_POT_G_EN, IT_POT_G_MP,
			IT_POT_G_HP, IT_POT_G_EN, IT_POT_G_MP,
			IT_POT_G_HP, IT_POT_G_EN, IT_POT_G_MP,
			MCT_CONTRACT
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
			IT_F_SA, IT_F_RU, IT_F_AM, IT_F_TO, IT_F_EM, IT_F_DI,
			IT_F_SP, IT_F_CI, IT_F_OP, IT_F_AQ, IT_F_BE, IT_F_ZI, IT_F_PH,
			IT_H_SA, IT_H_RU, IT_H_AM, IT_H_TO, IT_H_EM, IT_H_DI,
			IT_H_SP, IT_H_CI, IT_H_OP, IT_H_AQ, IT_H_BE, IT_H_ZI, IT_H_PH,
			IT_PLAT, IT_GOLD, IT_GOLD,
			MCT_CONTRACT, MCT_QUILL_G, MCT_QUILL_B, 
			IT_POT_RAIN
		};
		static int item[]  = {
			IT_POT_G_HP, IT_POT_G_EN, IT_POT_G_MP,
			IT_POT_G_HP, IT_POT_G_EN, IT_POT_G_MP,
			IT_POT_N_HP, IT_POT_N_EN, IT_POT_N_MP,
			MCT_CONTRACT
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
			IT_H_SA, IT_H_RU, IT_H_AM, IT_H_TO, IT_H_EM, IT_H_DI,
			IT_H_SP, IT_H_CI, IT_H_OP, IT_H_AQ, IT_H_BE, IT_H_ZI, IT_H_PH,
			IT_B_SA, IT_B_RU, IT_B_AM, IT_B_TO, IT_B_EM, IT_B_DI,
			IT_B_SP, IT_B_CI, IT_B_OP, IT_B_AQ,
			IT_GOLD, IT_GOLD, 
			MCT_CONTRACT, MCT_QUILL_Y, MCT_QUILL_G, 
			IT_POT_RAIN
		};
		static int item[]  = {
			IT_POT_G_HP, IT_POT_G_EN, IT_POT_G_MP,
			IT_POT_N_HP, IT_POT_N_EN, IT_POT_N_MP,
			IT_POT_N_HP, IT_POT_N_EN, IT_POT_N_MP,
			MCT_CONTRACT
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
			IT_B_SA, IT_B_RU, IT_B_AM, IT_B_TO, IT_B_EM, IT_B_DI,
			IT_B_SP, IT_B_CI, IT_B_OP, IT_B_AQ,
			IT_M_SA, IT_M_RU, IT_M_AM, IT_M_TO, IT_M_EM, IT_M_DI,
			IT_GOLD, IT_GOLD, IT_SILV,
			MCT_CONTRACT, MCT_QUILL_Y,
			IT_POT_RAIN
		};
		static int item[]  = {
			IT_POT_N_HP, IT_POT_N_EN, IT_POT_N_MP,
			IT_POT_N_HP, IT_POT_N_EN, IT_POT_N_MP,
			IT_POT_M_HP, IT_POT_M_EN, IT_POT_M_MP,
			MCT_CONTRACT
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
			IT_M_SA, IT_M_RU, IT_M_AM, IT_M_TO, IT_M_EM, IT_M_DI,
			IT_S_SA, IT_S_RU, IT_S_AM, IT_S_TO, IT_S_EM, IT_S_DI,
			IT_GOLD, IT_SILV, IT_SILV,
			IT_POT_RAIN
		};
		static int item[]  = {
			IT_POT_N_HP, IT_POT_N_EN, IT_POT_N_MP,
			IT_POT_M_HP, IT_POT_M_EN, IT_POT_M_MP,
			IT_FLO_R, IT_FLO_G, IT_FLO_P,
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
			IT_S_SA, IT_S_RU, IT_S_AM, IT_S_TO, IT_S_EM, IT_S_DI,
			IT_SILV, IT_SILV,
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
	int n = 3340;   // value to store id-number of created belt
	                // at start it contains template of 'rainbow_belt'
	int i, j;
	int rank, skill_value, skill_number, skm, armor=0, weapon=0, thorn=0, aoe=0, spmod=0, tryspm=0, tryaoe=0;
	
	rank = getrank(cn);  // private wont get this belt
	if (!rank)
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

	j = rank/2+1 + RANDOM(rank/2+1); // how many skills will be in belt?
	it[n].power += 5 * j;     // counting power of item, *remind* power = 10 in template
	it[n].value += 10000 * j; // counting price value, value = 100 in template
	
	// Random attributes
	for(i = 0; i < j; i++)
	{
		skill_number = RANDOM(5);
		//if (!it[n].attrib[skill_number][0]) it[n].attrib[skill_number][0] += 1;
		it[n].attrib[skill_number][0] += 1;
	}
	for (i = 0; i < 5; i++)
	{
		if (it[n].attrib[i][0]) it[n].attrib[i][2] = max(0,(j-6)/3 * 5) + max(0, (it[n].attrib[i][0]/2) * 5);
	}
	
	// Random skills
	for(i = 0; i < j; i++)
	{
		skill_number = RANDOM(60);
		
		switch(skill_number)
		{
			case 50:
				if (!it[n].hp[0]) it[n].hp[0] += 10;
				it[n].hp[0] += 10;
				break;
			case 51:
				if (!it[n].end[0]) it[n].end[0] += 15;
				it[n].end[0] += 5;
				break;
			case 52:
				if (!it[n].mana[0]) it[n].mana[0] += 10;
				it[n].mana[0] += 10;
				break;
			case 53:
				if (!armor) armor += 1;
				armor += 1;
				break;
			case 54:
				if (!weapon) weapon += 1;
				weapon += 1;
				break;
			case 55:
				if (!spmod) spmod += 1;
				spmod += 1;
				tryspm++;
				if (!spmod%4) tryspm=0;
				break;
			case 56:
				if (!it[n].speed[0]) it[n].speed[0] += 1;
				it[n].speed[0] += 1;
				break;
			case 57:
				if (!thorn) thorn += 1;
				thorn += 1;
				break;
			case 58:
				if (!it[n].crit_chance[0]) it[n].crit_chance[0] += 5;
				it[n].crit_chance[0] += 5;
				break;
			case 59:
				if (!aoe) aoe += 1;
				aoe += 1;
				tryaoe++;
				if (!tryaoe%4) tryaoe=0;
				break;
			default:
				skm = 2;
				if (skill_number == 0 || skill_number == 2 || skill_number == 3 || 
					skill_number == 4 || skill_number == 5 || skill_number == 6 || 
					skill_number ==16 || skill_number ==36) skm = 1;
				if (!it[n].skill[skill_number][0] && skm==2) it[n].skill[skill_number][0] += skm;
				it[n].skill[skill_number][0] += 1;
				break;
		}
	}
	j = 0;
	if (armor%2)	j++;
	if (weapon%2)	j++;
	if (thorn%2)	j++;
	if (spmod%4)	j+=tryspm;
	if (aoe%4)		j+=tryaoe;
	
	if (it[n].hp[0])   it[n].hp[2]   = 50 + it[n].hp[0]/2;
	if (it[n].mana[0]) it[n].mana[2] = 50 + it[n].mana[0]/2;
	
	it[n].armor[0]      += armor/2;
	it[n].weapon[0]     += weapon/2;
	it[n].gethit_dam[0] += thorn/2;
	it[n].spell_mod[0]  += spmod/3;
	it[n].aoe_bonus[0]  += aoe/3;
	
	// cleanup remaining numbers
	if (j) for(i = 0; i < j; i++)
	{
		skill_number = RANDOM(50);
		skm = 2;
		if (skill_number == 0 || skill_number == 2 || skill_number == 3 || 
			skill_number == 4 || skill_number == 5 || skill_number == 6 || 
			skill_number ==16 || skill_number ==36) skm = 1;
		if (!it[n].skill[skill_number][0] && skm==2) it[n].skill[skill_number][0] += skm;
		it[n].skill[skill_number][0] += 1;
	}
	
	it[n].flags |= IF_CAN_EN;
	
	// Vanilla
	/*
	
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
			case  0:
				it[n].attrib[AT_BRV][0] += skill_value; 								// this line is how much it will raise attribute
				if (it[n].attrib[AT_BRV][0] > 12) it[n].attrib[AT_BRV][0] = 12; 			// this will check for max level = 12 and will down it back to 12
				it[n].attrib[AT_BRV][2] = 10 + (it[n].attrib[AT_BRV][0] * RANDOM(7)); 	// this line will set requirements
				break;
			case  1:
				it[n].attrib[AT_WIL][0] += skill_value;
				if (it[n].attrib[AT_WIL][0] > 12) it[n].attrib[AT_WIL][0] = 12;
				it[n].attrib[AT_WIL][2] = 10 + (it[n].attrib[AT_WIL][0] * RANDOM(7));
				break;
			case  2:
				it[n].attrib[AT_INT][0] += skill_value;
				if (it[n].attrib[AT_INT][0] > 12) it[n].attrib[AT_INT][0] = 12;
				it[n].attrib[AT_INT][2] = 10 + (it[n].attrib[AT_INT][0] * RANDOM(7));
				break;
			case  3:
				it[n].attrib[AT_AGL][0] += skill_value;
				if (it[n].attrib[AT_AGL][0] > 12) it[n].attrib[AT_AGL][0] = 12;
				it[n].attrib[AT_AGL][2] = 10 + (it[n].attrib[AT_AGL][0] * RANDOM(7));
				break;
			case  4:
				it[n].attrib[AT_STR][0] += skill_value;
				if (it[n].attrib[AT_STR][0] > 12) it[n].attrib[AT_STR][0] = 12;
				it[n].attrib[AT_STR][2] = 10 + (it[n].attrib[AT_STR][0] * RANDOM(7));
				break;
			case  5:
				it[n].hp[0] += (skill_value * 10);
				if (it[n].hp[0] > 120) it[n].hp[0] = 120;
				it[n].hp[2] = 50 + (it[n].hp[0] * RANDOM(9));
				break;
			case  6:
				it[n].end[0] += (skill_value * 10)/2;
				if (it[n].end[0] > 60) it[n].end[0] = 60;
				break;
			case  7:
				it[n].mana[0] += (skill_value * 10);
				if (it[n].mana[0] > 120) it[n].mana[0] = 120;
				it[n].mana[2] = 50 + (it[n].mana[0] * RANDOM(9));
				break;
			case  8:
				it[n].armor[0] += (skill_value+1)/2;
				if (it[n].armor[0] > 6) it[n].armor[0] = 6;
				break;
			case  9:
				it[n].weapon[0] += (skill_value+1)/2;
				if (it[n].weapon[0] > 6) it[n].armor[0] = 6;
				break;
			default:
				skm = skill_number-10;
				it[n].skill[skm][0] += skill_value;
				if (it[n].skill[skm][0] > 12) it[n].skill[skm][0] = 12;
				it[n].skill[skm][2] = (it[n].skill[skm][0] * RANDOM(5));
				break;
		}
	}
	*/
	return(n);
}
/*	--end	*/

int pop_create_char(int n, int drop)
{
	int cn, tmp, m, j=0, flag = 0, hasitems = 0, hasloot = 0;

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

	for (m = 0; m<MAXITEMS; m++)
	{
		if (tmp = ch[cn].item[m])
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
			if (m == WN_RHAND && n >= CT_VAMPIRE && n <= CT_LASTVAMPIRE && !RANDOM(2)) // Vampire equipment adjustment
			{
				int randm = RANDOM(9);
				if (tmp==IT_CLAW_STEL) tmp = IT_DAGG_STEL + randm;
				if (tmp==IT_CLAW_GOLD) tmp = IT_DAGG_GOLD + randm;
				if (tmp==IT_CLAW_EMER) tmp = IT_DAGG_EMER + randm;
				if (tmp==IT_CLAW_CRYS) tmp = IT_DAGG_CRYS + randm;
				if (tmp==IT_CLAW_TITN) tmp = IT_DAGG_TITN + randm;
				if (tmp==IT_CLAW_DAMA) tmp = IT_DAGG_DAMA + randm;
				if (tmp==IT_CLAW_ADAM) tmp = IT_DAGG_ADAM + randm;
				switch (randm)
				{
					case  0: // Dagger
						ch[cn].skill[SK_DAGGER][0]  = ch[cn].skill[SK_HAND][0];
						ch[cn].skill[SK_HAND][0]    = 0;
						break;
					case  1: // Staff
						ch[cn].skill[SK_STAFF][0]   = ch[cn].skill[SK_HAND][0]+3;
						ch[cn].skill[SK_HAND][0]    = 0;
						break;
					case  2: // Spear
						ch[cn].skill[SK_DAGGER][0]  = ch[cn].skill[SK_HAND][0]-3;
						ch[cn].skill[SK_STAFF][0]   = ch[cn].skill[SK_HAND][0]-3;
						ch[cn].skill[SK_HAND][0]    = 0;
						break;
					case  4: // Sword
						ch[cn].skill[SK_SWORD][0]   = ch[cn].skill[SK_HAND][0]+3;
						ch[cn].skill[SK_HAND][0]    = 0;
						break;
					case  6: // Axe
						ch[cn].skill[SK_AXE][0]     = ch[cn].skill[SK_HAND][0];
						ch[cn].skill[SK_HAND][0]    = 0;
						break;
					case  7: // Twohander
						ch[cn].skill[SK_TWOHAND][0] = ch[cn].skill[SK_HAND][0];
						ch[cn].skill[SK_HAND][0]    = 0;
						break;
					case  8: // Greataxe
						ch[cn].skill[SK_AXE][0]     = ch[cn].skill[SK_HAND][0];
						ch[cn].skill[SK_TWOHAND][0] = ch[cn].skill[SK_HAND][0];
						ch[cn].skill[SK_HAND][0]    = 0;
						break;
					default:
						tmp = ch[cn].worn[m];
						break;
				}
			}
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
				if (it[tmp].temp==0)
				{
					hasloot = 1;
				}
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
	ch[cn].a_hp  = 9999999;
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

	if (ch[cn].alignment<0)
	{
		for (m = 0; m<40; m++)
		{
			if (ch[cn].item[m]==0 && (hasitems || (RANDOM(2) && ch[cn].temp!=347 && ch[cn].temp!=350)))
			{
				j = m;
				if (try_boost(40))
				{
					tmp = pop_create_bonus(cn);
					if (tmp)
					{
						it[tmp].carried = cn;
						ch[cn].item[j]  = tmp;
						j++; if (j>=40) break;
						hasloot = 1;
					}
				}
				if (ch[cn].item[j]==0 && try_boost(500))
				{
					tmp = pop_create_bonus(cn);
					if (tmp)
					{
						it[tmp].carried = cn;
						ch[cn].item[j]  = tmp;
						j++; if (j>=40) break;
						hasloot = 1;
					}
				}
				if (ch[cn].item[j]==0 && try_boost(6000))
				{
					tmp = pop_create_bonus_belt(cn);
					if (tmp)
					{
						it[tmp].carried = cn;
						ch[cn].item[j]  = tmp;
						hasloot = 1;
					}
				}
				break;
			}
		}
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
	
	if (ch[cn].alignment>=0 || ch[cn].gold > 20000 || (ch[cn].flags & CF_EXTRACRIT) || getrank(cn)<9)
	{
		hasloot = 1;
	}
	
	if (!IS_LABY_MOB(cn) && !ch[cn].citem && !hasloot && !(ch[cn].flags & CF_EXTRAEXP) && !(ch[cn].flags & CF_EXTRACRIT) && try_boost(DW_CHANCE))
	{
		if (tmp = god_create_item(IT_CORRUPTOR))
		{
			ch[cn].citem = tmp;
			it[tmp].carried = cn;
			it[tmp].cost = 555;
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
			pts += attrib_needed(m, 3);
		}
	}

	for (m = 50; m<ch_temp[n].hp[0]; m++)
	{
		pts += hp_needed(m, 3);
	}

	//for (m = 50; m<ch_temp[n].end[0]; m++)
	//{
	//	pts += end_needed(m, 2);
	//}

	for (m = 50; m<ch_temp[n].mana[0]; m++)
	{
		pts += mana_needed(m, 3);
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
			
			if (it[in].flags & (IF_DIMINISHED|IF_EASEUSE|IF_ENCHANTED))
			{
				if (it[in].flags & (IF_DIMINISHED|IF_EASEUSE))
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
	int h1, h2, h3, n, m, m2, in, chc = 0, itc = 0;

	xlog("Saving players...");

	h1 = open(".tmp/char.dat",   O_RDWR | O_CREAT | O_TRUNC, 0600);
	h2 = open(".tmp/item.dat",   O_RDWR | O_CREAT | O_TRUNC, 0600);
	h3 = open(".tmp/global.dat", O_RDWR | O_CREAT | O_TRUNC, 0600);

	if (h1==-1 || h2==-1 || h3==-1)
	{
		xlog("pop remove failed: could not create temporary file.");
		return;
	}

	for (n = 1; n<MAXCHARS; n++)
	{
		if (ch[n].used==USE_EMPTY)			continue;
		if (!(ch[n].flags & (CF_PLAYER)))	continue;
		if (ch[n].flags & CF_BODY)			continue;

		for (m = 0; m<MAXITEMS; m++) if ((in = ch[n].item[m])!=0)
		{
			write(h2, &it[in], sizeof(struct item));
			itc++;
		}
		for (m = 0; m<20; m++) if ((in = ch[n].worn[m])!=0)
		{
			write(h2, &it[in], sizeof(struct item));
			itc++;
		}
		for (m = 0; m<12; m++) if ((in = ch[n].alt_worn[m])!=0)
		{
			write(h2, &it[in], sizeof(struct item));
			itc++;
		}
		for (m = 0; m<MAXBUFFS; m++) if ((in = ch[n].spell[m])!=0)
		{
			write(h2, &bu[in], sizeof(struct item));
			itc++;
		}
		/*
//	//	for (m = 0; m<62; m++) if ((in = ch[n].depot[m])!=0)
		{
			write(h2, &it[in], sizeof(struct item));
			itc++;
		}
		*/
		for (m = 0; m<ST_PAGES*ST_SLOTS; m++) if ((in = st[n].depot[m/ST_SLOTS][m%ST_SLOTS])!=0)
		{
			write(h2, &it[in], sizeof(struct item));
			itc++;
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
		if (read(h1, &ctmp, sizeof(struct character))<1) break;
		n = ctmp.data[99];
		if (ch[n].used!=USE_EMPTY)
		{
			xlog("Character slot %d to be loaded to not empty!", n);
			for (m = 0; m<MAXITEMS; m++)	if ((in = ch[n].item[m])!=0)		it[in].used = USE_EMPTY;
			for (m = 0; m<MAXBUFFS; m++)	if ((in = ch[n].spell[m])!=0)		bu[in].used = USE_EMPTY;
			for (m = 0; m<20; m++)			if ((in = ch[n].worn[m])!=0)		it[in].used = USE_EMPTY;
			for (m = 0; m<12; m++)			if ((in = ch[n].alt_worn[m])!=0)	it[in].used = USE_EMPTY;
											if ((in = ch[n].citem)!=0)			it[in].used = USE_EMPTY;
		}
		ch[n] = ctmp;
		chc++;
		for (m = 0; m<MAXITEMS; m++) if (ch[n].item[m])
		{
			for (in = 1; in<MAXITEM; in++) if (it[in].used==USE_EMPTY) break;
			if (in==MAXITEM)
			{
				xlog("MAXITEM reached.");
				break;
			}
			read(h2, &it[in], sizeof(struct item));
			itc++;
			ch[n].item[m] = in;
		}
		for (m = 0; m<20; m++) if (ch[n].worn[m])
		{
			for (in = 1; in<MAXITEM; in++) if (it[in].used==USE_EMPTY) break;
			if (in==MAXITEM)
			{
				xlog("MAXITEM reached.");
				break;
			}

			read(h2, &it[in], sizeof(struct item));
			itc++;

			ch[n].worn[m] = in;
		}
		for (m = 0; m<12; m++) if (ch[n].alt_worn[m])
		{
			for (in = 1; in<MAXITEM; in++) if (it[in].used==USE_EMPTY) break;
			if (in==MAXITEM)
			{
				xlog("MAXITEM reached.");
				break;
			}
			read(h2, &it[in], sizeof(struct item));
			itc++;
			ch[n].alt_worn[m] = in;
		}
		for (m = 0; m<MAXBUFFS; m++) if (ch[n].spell[m])
		{
			for (in = 1; in<MAXBUFF; in++) if (bu[in].used==USE_EMPTY) break;
			if (in==MAXBUFF)
			{
				xlog("MAXBUFF reached.");
				break;
			}
			read(h2, &bu[in], sizeof(struct item));
			itc++;
			ch[n].spell[m] = in;
		}
		/*
//	//	for (m = 0; m<62; m++) if (ch[n].depot[m])
		{
			for (in = 1; in<MAXITEM; in++) if (it[in].used==USE_EMPTY) break;
			if (in==MAXITEM)
			{
				xlog("MAXITEM reached.");
				break;
			}
			read(h2, &it[in], sizeof(struct item));
			itc++;
//	//		ch[n].depot[m] = in;
		}
		*/
		for (m = 0; m<ST_PAGES*ST_SLOTS; m++) if (st[n].depot[m/ST_SLOTS][m%ST_SLOTS])
		{
			for (in = 1; in<MAXITEM; in++) if (it[in].used==USE_EMPTY) break;
			if (in==MAXITEM)
			{
				xlog("MAXITEM reached.");
				break;
			}
			read(h2, &it[in], sizeof(struct item));
			itc++;
			st[n].depot[m/ST_SLOTS][m%ST_SLOTS] = in;
		}
		if (ch[n].flags & (CF_PLAYER)) 	globs->players_created++;
		else							map[ch[n].x + ch[n].y * MAPX].ch = n;
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
		return;

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
	for (m = 0; m<MAXITEMS; m++) if ((in = ch[nr].item[m])!=0)
	{
		write(h1, &it[in], sizeof(struct item));
		itc++;
	}
	// save worn items
	for (m = 0; m<20; m++) if ((in = ch[nr].worn[m])!=0)
	{
		write(h1, &it[in], sizeof(struct item));
		itc++;
	}
	// save alt_worn items
	for (m = 0; m<12; m++) if ((in = ch[nr].alt_worn[m])!=0)
	{
		write(h1, &it[in], sizeof(struct item));
		itc++;
	}
	/*
	// save depot contents
//	for (m = 0; m<62; m++) if ((in = ch[nr].depot[m])!=0)
	{
		write(h1, &it[in], sizeof(struct item));
		itc++;
	}
	*/
	// save new depot contents
	for (m = 0; m<ST_PAGES*ST_SLOTS; m++) if ((in = st[nr].depot[m/ST_SLOTS][m%ST_SLOTS])!=0)
	{
		write(h1, &it[in], sizeof(struct item));
		itc++;
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
		for (m = 0; m<MAXITEMS; m++) 			if ((in = ch[nr].item[m])!=0) 						it[in].used = USE_EMPTY;
		for (m = 0; m<MAXBUFFS; m++) 			if ((in = ch[nr].spell[m])!=0) 						bu[in].used = USE_EMPTY;
		for (m = 0; m<20; m++) 					if ((in = ch[nr].worn[m])!=0) 						it[in].used = USE_EMPTY;
		for (m = 0; m<12; m++) 					if ((in = ch[nr].alt_worn[m])!=0) 					it[in].used = USE_EMPTY;
//	//	for (m = 0; m<62; m++) 					if ((in = ch[nr].depot[m])!=0) 						it[in].used = USE_EMPTY;
		for (m = 0; m<ST_PAGES*ST_SLOTS; m++) 	if ((in = st[nr].depot[m/ST_SLOTS][m%ST_SLOTS])!=0) it[in].used = USE_EMPTY;
												if ((in = ch[nr].citem)!=0) 						it[in].used = USE_EMPTY;
	}
	ch[nr] = ctmp;
	chc++;

	for (m = 0; m<MAXITEMS; m++) if (ch[nr].item[m])
	{
		for (in = 1; in<MAXITEM; in++) if (it[in].used==USE_EMPTY) break;
		if (in==MAXITEM)
		{
			xlog("MAXITEM reached.");
			break;
		}
		read(h1, &it[in], sizeof(struct item));
		itc++;
		ch[nr].item[m] = in;
	}
	for (m = 0; m<MAXBUFFS; m++)
	{
		ch[nr].spell[m] = 0;
	}
	for (m = 0; m<20; m++) if (ch[nr].worn[m])
	{
		for (in = 1; in<MAXITEM; in++) if (it[in].used==USE_EMPTY) break;
		if (in==MAXITEM)
		{
			xlog("MAXITEM reached.");
			break;
		}
		read(h1, &it[in], sizeof(struct item));
		itc++;
		ch[nr].worn[m] = in;
	}
	for (m = 0; m<12; m++) if (ch[nr].alt_worn[m])
	{
		for (in = 1; in<MAXITEM; in++) if (it[in].used==USE_EMPTY) break;
		if (in==MAXITEM)
		{
			xlog("MAXITEM reached.");
			break;
		}
		read(h1, &it[in], sizeof(struct item));
		itc++;
		ch[nr].alt_worn[m] = in;
	}
	/*
//	for (m = 0; m<62; m++) if (ch[nr].depot[m])
	{
		for (in = 1; in<MAXITEM; in++) if (it[in].used==USE_EMPTY) break;
		if (in==MAXITEM)
		{
			xlog("MAXITEM reached.");
			break;
		}
		read(h1, &it[in], sizeof(struct item));
		itc++;
//	//	ch[nr].depot[m] = in;
	}
	*/
	for (m = 0; m<ST_PAGES*ST_SLOTS; m++) if (st[nr].depot[m/ST_SLOTS][m%ST_SLOTS])
	{
		for (in = 1; in<MAXITEM; in++) if (it[in].used==USE_EMPTY) break;
		if (in==MAXITEM)
		{
			xlog("MAXITEM reached.");
			break;
		}
		read(h1, &it[in], sizeof(struct item));
		itc++;
		st[nr].depot[m/ST_SLOTS][m%ST_SLOTS] = in;
	}
	if (ch[nr].flags & (CF_PLAYER)) globs->players_created++;
	else							map[ch[nr].x + ch[nr].y * MAPX].ch = nr;

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

/* // This needs to be updated if this does eventually happen... But it's not necessary right now.
void pop_copy_to_new_chars(void)
{
	int a, b, c;
	
	for (a = 1; a<MAXCHARS; a++)
	{
		ch_new[a].used = ch[a].used;
		for (b=0;b<40;b++) 
		{
			ch_new[a].name[b] = ch[a].name[b];
			ch_new[a].reference[b] = ch[a].reference[b];
		}
		for (b=0;b<LENDESC;b++) 
		{
			ch_new[a].description[b] = ch[a].description[b];
		}
		ch_new[a].kindred = ch[a].kindred;
		ch_new[a].player = ch[a].player;
		ch_new[a].pass1 = ch[a].pass1;
		ch_new[a].pass2 = ch[a].pass2;
		ch_new[a].sprite = ch[a].sprite;
		ch_new[a].sound = ch[a].sound;
		ch_new[a].flags = ch[a].flags;
		ch_new[a].alignment = ch[a].alignment;
		ch_new[a].temple_x = ch[a].temple_x;
		ch_new[a].temple_y = ch[a].temple_y;
		ch_new[a].tavern_x = ch[a].tavern_x;
		ch_new[a].tavern_y = ch[a].tavern_y;
		ch_new[a].temp = ch[a].temp;
		for (b=0;b<5;b++) for (c=0;c<6;c++) 
		{
			ch_new[a].attrib[b][c] = ch[a].attrib[b][c];
		}
		for (b=0;b<6;b++)
		{
			ch_new[a].hp[b] = ch[a].hp[b];
			ch_new[a].end[b] = ch[a].end[b];
			ch_new[a].mana[b] = ch[a].mana[b];
		}
		for (b=0;b<50;b++) for (c=0;c<6;c++) 
		{
			ch_new[a].skill[b][c] = ch[a].skill[b][c];
		}
		ch_new[a].weapon_bonus = ch[a].weapon_bonus;
		ch_new[a].armor_bonus = ch[a].armor_bonus;
		ch_new[a].a_hp = ch[a].a_hp;
		ch_new[a].a_end = ch[a].a_end;
		ch_new[a].a_mana = ch[a].a_mana;
		ch_new[a].light = ch[a].light;
		ch_new[a].mode = ch[a].mode;
		ch_new[a].speed = ch[a].speed;
		ch_new[a].points = ch[a].points;
		ch_new[a].points_tot = ch[a].points_tot;
		ch_new[a].armor = ch[a].armor;
		ch_new[a].weapon = ch[a].weapon;
		ch_new[a].x = ch[a].x;
		ch_new[a].y = ch[a].y;
		ch_new[a].tox = ch[a].tox;
		ch_new[a].toy = ch[a].toy;
		ch_new[a].frx = ch[a].frx;
		ch_new[a].fry = ch[a].fry;
		ch_new[a].status = ch[a].status;
		ch_new[a].status2 = ch[a].status2;
		ch_new[a].dir = ch[a].dir;
		ch_new[a].gold = ch[a].gold;
		for (b=0;b<40;b++)
		{
			ch_new[a].item[b] = ch[a].item[b];
		}
		for (b=0;b<20;b++)
		{
			ch_new[a].worn[b] = ch[a].worn[b];
		}
		for (b=0;b<MAXBUFFS;b++)
		{
			ch_new[a].spell[b] = ch[a].spell[b];
		}
		ch_new[a].citem = ch[a].citem;
		ch_new[a].creation_date = ch[a].creation_date;
		ch_new[a].login_date = ch[a].login_date;
		ch_new[a].addr = ch[a].addr;
		ch_new[a].current_online_time = ch[a].current_online_time;
		ch_new[a].total_online_time = ch[a].total_online_time;
		ch_new[a].comp_volume = ch[a].comp_volume;
		ch_new[a].raw_volume = ch[a].raw_volume;
		ch_new[a].idle = ch[a].idle;
		ch_new[a].attack_cn = ch[a].attack_cn;
		ch_new[a].skill_nr = ch[a].skill_nr;
		ch_new[a].skill_target1 = ch[a].skill_target1;
		ch_new[a].skill_target2 = ch[a].skill_target2;
		ch_new[a].goto_x = ch[a].goto_x;
		ch_new[a].goto_y = ch[a].goto_y;
		ch_new[a].use_nr = ch[a].use_nr;
		ch_new[a].misc_action = ch[a].misc_action;
		ch_new[a].misc_target1 = ch[a].misc_target1;
		ch_new[a].misc_target2 = ch[a].misc_target2;
		ch_new[a].cerrno = ch[a].cerrno;
		ch_new[a].escape_timer = ch[a].escape_timer;
		for (b=0;b<4;b++)
		{
			ch_new[a].enemy[b] = ch[a].enemy[b];
		}
		ch_new[a].current_enemy = ch[a].current_enemy;
		ch_new[a].retry = ch[a].retry;
		ch_new[a].stunned = ch[a].stunned;
		ch_new[a].speed_mod = ch[a].speed_mod;
		ch_new[a].last_action = ch[a].last_action;
		ch_new[a].gethit_dam = ch[a].gethit_dam;
		ch_new[a].gethit_bonus = ch[a].gethit_bonus;
		ch_new[a].light_bonus = ch[a].light_bonus;
		for (b=0;b<16;b++)
		{
			ch_new[a].passwd[b] = ch[a].passwd[b];
		}
		ch_new[a].lastattack = ch[a].lastattack;
		ch_new[a].move_speed = ch[a].move_speed;
		ch_new[a].atk_speed = ch[a].atk_speed;
		ch_new[a].cast_speed = ch[a].cast_speed;
		ch_new[a].spell_mod = ch[a].spell_mod;
		ch_new[a].spell_apt = ch[a].spell_apt;
		ch_new[a].cool_bonus = ch[a].cool_bonus;
		ch_new[a].crit_chance = ch[a].crit_chance;
		ch_new[a].crit_multi = ch[a].crit_multi;
		ch_new[a].to_hit = ch[a].to_hit;
		ch_new[a].to_parry = ch[a].to_parry;
		ch_new[a].top_damage = ch[a].top_damage;
		ch_new[a].taunted = ch[a].taunted;
		ch_new[a].bs_points = ch[a].bs_points;
		ch_new[a].sprite_override = ch[a].sprite_override;
		for (b=0;b<12;b++)
		{
			ch_new[a].alt_worn[b] = ch[a].alt_worn[b];
		}
		ch_new[a].os_points = ch[a].os_points;
		ch_new[a].gcm = ch[a].gcm;
		ch_new[a].aoe_bonus = ch[a].aoe_bonus;
		ch_new[a].colosseum = ch[a].colosseum;
		ch_new[a].spellfail = ch[a].spellfail;
		ch_new[a].dmg_bonus = ch[a].dmg_bonus;
		ch_new[a].dmg_reduction = ch[a].dmg_reduction;
		for (b=0;b<6;b++) for (c=0;c<2;c++)
		{
			ch_new[a].limit_break[b][c] = ch[a].limit_break[b][c];
		}
		for (b=0;b<6;b++)
		{
			ch_new[a].pandium_floor[b] = ch[a].pandium_floor[b];
		}
		ch_new[a].waypoints = ch[a].waypoints;
		ch_new[a].tokens = ch[a].tokens;
//	//	for (b=0;b<62;b++)
//	//	{
//	//	//	ch_new[a].depot[b] = ch[a].depot[b];
//	//	}
		ch_new[a].luck = ch[a].luck;
		ch_new[a].unreach = ch[a].unreach;
		ch_new[a].unreachx = ch[a].unreachx;
		ch_new[a].unreachy = ch[a].unreachy;
		ch_new[a].class = ch[a].class;
		ch_new[a].logout_date = ch[a].logout_date;
		for (b=0;b<100;b++)
		{
			ch_new[a].data[b] = ch[a].data[b];
		}
		for (b=0;b<10;b++) for (c=0;c<160;c++)
		{
			ch_new[a].text[b][c] = ch[a].text[b][c];
		}
	}
	
	for (a = 1; a<MAXTCHARS; a++)
	{
		ch_temp_new[a].used = ch_temp[a].used;
		for (b=0;b<40;b++) 
		{
			ch_temp_new[a].name[b] = ch_temp[a].name[b];
			ch_temp_new[a].reference[b] = ch_temp[a].reference[b];
		}
		for (b=0;b<LENDESC;b++) 
		{
			ch_temp_new[a].description[b] = ch_temp[a].description[b];
		}
		ch_temp_new[a].kindred = ch_temp[a].kindred;
		ch_temp_new[a].player = ch_temp[a].player;
		ch_temp_new[a].pass1 = ch_temp[a].pass1;
		ch_temp_new[a].pass2 = ch_temp[a].pass2;
		ch_temp_new[a].sprite = ch_temp[a].sprite;
		ch_temp_new[a].sound = ch_temp[a].sound;
		ch_temp_new[a].flags = ch_temp[a].flags;
		ch_temp_new[a].alignment = ch_temp[a].alignment;
		ch_temp_new[a].temple_x = ch_temp[a].temple_x;
		ch_temp_new[a].temple_y = ch_temp[a].temple_y;
		ch_temp_new[a].tavern_x = ch_temp[a].tavern_x;
		ch_temp_new[a].tavern_y = ch_temp[a].tavern_y;
		ch_temp_new[a].temp = ch_temp[a].temp;
		for (b=0;b<5;b++) for (c=0;c<6;c++) 
		{
			ch_temp_new[a].attrib[b][c] = ch_temp[a].attrib[b][c];
		}
		for (b=0;b<6;b++)
		{
			ch_temp_new[a].hp[b] = ch_temp[a].hp[b];
			ch_temp_new[a].end[b] = ch_temp[a].end[b];
			ch_temp_new[a].mana[b] = ch_temp[a].mana[b];
		}
		for (b=0;b<50;b++) for (c=0;c<6;c++) 
		{
			ch_temp_new[a].skill[b][c] = ch_temp[a].skill[b][c];
		}
		ch_temp_new[a].weapon_bonus = ch_temp[a].weapon_bonus;
		ch_temp_new[a].armor_bonus = ch_temp[a].armor_bonus;
		ch_temp_new[a].a_hp = ch_temp[a].a_hp;
		ch_temp_new[a].a_end = ch_temp[a].a_end;
		ch_temp_new[a].a_mana = ch_temp[a].a_mana;
		ch_temp_new[a].light = ch_temp[a].light;
		ch_temp_new[a].mode = ch_temp[a].mode;
		ch_temp_new[a].speed = ch_temp[a].speed;
		ch_temp_new[a].points = ch_temp[a].points;
		ch_temp_new[a].points_tot = ch_temp[a].points_tot;
		ch_temp_new[a].armor = ch_temp[a].armor;
		ch_temp_new[a].weapon = ch_temp[a].weapon;
		ch_temp_new[a].x = ch_temp[a].x;
		ch_temp_new[a].y = ch_temp[a].y;
		ch_temp_new[a].tox = ch_temp[a].tox;
		ch_temp_new[a].toy = ch_temp[a].toy;
		ch_temp_new[a].frx = ch_temp[a].frx;
		ch_temp_new[a].fry = ch_temp[a].fry;
		ch_temp_new[a].status = ch_temp[a].status;
		ch_temp_new[a].status2 = ch_temp[a].status2;
		ch_temp_new[a].dir = ch_temp[a].dir;
		ch_temp_new[a].gold = ch_temp[a].gold;
		for (b=0;b<40;b++)
		{
			ch_temp_new[a].item[b] = ch_temp[a].item[b];
		}
		for (b=0;b<20;b++)
		{
			ch_temp_new[a].worn[b] = ch_temp[a].worn[b];
		}
		for (b=0;b<MAXBUFFS;b++)
		{
			ch_temp_new[a].spell[b] = ch_temp[a].spell[b];
		}
		ch_temp_new[a].citem = ch_temp[a].citem;
		ch_temp_new[a].creation_date = ch_temp[a].creation_date;
		ch_temp_new[a].login_date = ch_temp[a].login_date;
		ch_temp_new[a].addr = ch_temp[a].addr;
		ch_temp_new[a].current_online_time = ch_temp[a].current_online_time;
		ch_temp_new[a].total_online_time = ch_temp[a].total_online_time;
		ch_temp_new[a].comp_volume = ch_temp[a].comp_volume;
		ch_temp_new[a].raw_volume = ch_temp[a].raw_volume;
		ch_temp_new[a].idle = ch_temp[a].idle;
		ch_temp_new[a].attack_cn = ch_temp[a].attack_cn;
		ch_temp_new[a].skill_nr = ch_temp[a].skill_nr;
		ch_temp_new[a].skill_target1 = ch_temp[a].skill_target1;
		ch_temp_new[a].skill_target2 = ch_temp[a].skill_target2;
		ch_temp_new[a].goto_x = ch_temp[a].goto_x;
		ch_temp_new[a].goto_y = ch_temp[a].goto_y;
		ch_temp_new[a].use_nr = ch_temp[a].use_nr;
		ch_temp_new[a].misc_action = ch_temp[a].misc_action;
		ch_temp_new[a].misc_target1 = ch_temp[a].misc_target1;
		ch_temp_new[a].misc_target2 = ch_temp[a].misc_target2;
		ch_temp_new[a].cerrno = ch_temp[a].cerrno;
		ch_temp_new[a].escape_timer = ch_temp[a].escape_timer;
		for (b=0;b<4;b++)
		{
			ch_temp_new[a].enemy[b] = ch_temp[a].enemy[b];
		}
		ch_temp_new[a].current_enemy = ch_temp[a].current_enemy;
		ch_temp_new[a].retry = ch_temp[a].retry;
		ch_temp_new[a].stunned = ch_temp[a].stunned;
		ch_temp_new[a].speed_mod = ch_temp[a].speed_mod;
		ch_temp_new[a].last_action = ch_temp[a].last_action;
		ch_temp_new[a].gethit_dam = ch_temp[a].gethit_dam;
		ch_temp_new[a].gethit_bonus = ch_temp[a].gethit_bonus;
		ch_temp_new[a].light_bonus = ch_temp[a].light_bonus;
		for (b=0;b<16;b++)
		{
			ch_temp_new[a].passwd[b] = ch_temp[a].passwd[b];
		}
		ch_temp_new[a].lastattack = ch_temp[a].lastattack;
		ch_temp_new[a].move_speed = ch_temp[a].move_speed;
		ch_temp_new[a].atk_speed = ch_temp[a].atk_speed;
		ch_temp_new[a].cast_speed = ch_temp[a].cast_speed;
		ch_temp_new[a].spell_mod = ch_temp[a].spell_mod;
		ch_temp_new[a].spell_apt = ch_temp[a].spell_apt;
		ch_temp_new[a].cool_bonus = ch_temp[a].cool_bonus;
		ch_temp_new[a].crit_chance = ch_temp[a].crit_chance;
		ch_temp_new[a].crit_multi = ch_temp[a].crit_multi;
		ch_temp_new[a].to_hit = ch_temp[a].to_hit;
		ch_temp_new[a].to_parry = ch_temp[a].to_parry;
		ch_temp_new[a].top_damage = ch_temp[a].top_damage;
		ch_temp_new[a].taunted = ch_temp[a].taunted;
		ch_temp_new[a].bs_points = ch_temp[a].bs_points;
		ch_temp_new[a].sprite_override = ch_temp[a].sprite_override;
		for (b=0;b<12;b++)
		{
			ch_temp_new[a].alt_worn[b] = ch_temp[a].alt_worn[b];
		}
		ch_temp_new[a].os_points = ch_temp[a].os_points;
		ch_temp_new[a].gcm = ch_temp[a].gcm;
		ch_temp_new[a].aoe_bonus = ch_temp[a].aoe_bonus;
		ch_temp_new[a].colosseum = ch_temp[a].colosseum;
		ch_temp_new[a].spellfail = ch_temp[a].spellfail;
		ch_temp_new[a].dmg_bonus = ch_temp[a].dmg_bonus;
		ch_temp_new[a].dmg_reduction = ch_temp[a].dmg_reduction;
		for (b=0;b<6;b++) for (c=0;c<2;c++)
		{
			ch_temp_new[a].limit_break[b][c] = ch_temp[a].limit_break[b][c];
		}
		for (b=0;b<6;b++)
		{
			ch_temp_new[a].pandium_floor[b] = ch_temp[a].pandium_floor[b];
		}
		ch_temp_new[a].waypoints = ch_temp[a].waypoints;
		ch_temp_new[a].tokens = ch_temp[a].tokens;
//	//	for (b=0;b<62;b++)
//	//	{
//	//	//	ch_temp_new[a].depot[b] = ch_temp[a].depot[b];
//	//	}
		ch_temp_new[a].luck = ch_temp[a].luck;
		ch_temp_new[a].unreach = ch_temp[a].unreach;
		ch_temp_new[a].unreachx = ch_temp[a].unreachx;
		ch_temp_new[a].unreachy = ch_temp[a].unreachy;
		ch_temp_new[a].class = ch_temp[a].class;
		ch_temp_new[a].logout_date = ch_temp[a].logout_date;
		for (b=0;b<100;b++)
		{
			ch_temp_new[a].data[b] = ch_temp[a].data[b];
		}
		for (b=0;b<10;b++) for (c=0;c<160;c++)
		{
			ch_temp_new[a].text[b][c] = ch_temp[a].text[b][c];
		}
	}
}
*/

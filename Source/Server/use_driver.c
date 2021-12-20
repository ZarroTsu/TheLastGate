/*************************************************************************

   This file is part of 'Mercenaries of Astonia v2'
   Copyright (c) 1997-2001 Daniel Brockhaus (joker@astonia.com)
   All rights reserved.

 **************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "server.h"
#include "funcs.h"

int sub_door_driver(int cn, int in)
{
	if (it[in].data[0]==65500)
	{
		return 0;
	}
	if (it[in].data[0]==65501 || it[in].data[0]==65502)   // star door in black stronghold
	{
		int empty = 0, star = 0, circle = 0, in2, n;
		static int loctab[4] = {216657, 215633, 216658, 215634};

		for (n = 0; n<4; n++)
		{
			in2 = map[loctab[n]].it;
			if (!in2)					continue;
			if (it[in2].data[1]!=n)		continue;

			if (it[in2].temp==IT_BSCOIN_STAR)	star++;
			if (it[in2].temp==IT_BSCOIN_MOON)	circle++;
			if (it[in2].temp==IT_BSCOIN_NULL)	empty++;
		}

		if (		it[in].data[0]==65501 && empty==3 && star==1)	return 1;
		else if (	it[in].data[0]==65502 && empty==3 && circle==1)	return 1;
		else														return 0;
	}

	return 0;
}

int use_door(int cn, int in)
{
	int in2, lock = 0, n, skill, power, temp, flags;

	if (map[it[in].x + it[in].y * MAPX].ch)
	{
		return 0;
	}
	
	if (it[in].data[5]>0)
	{
		do_char_log(cn, 0, "It's been jammed shut! Try again later?\n");
		return 0;
	}
	
	if (it[in].data[0])
	{
		if (cn==0)
		{
			lock = 1;
		}
		else if (it[in].data[0]>=65500)
		{
			lock = sub_door_driver(cn, in);
		}
		else if ((in2 = ch[cn].citem)!=0 && !(in2 & 0x80000000) && it[in2].temp==it[in].data[0])
		{
			lock = 1;
			if (it[in].data[3] && (!it[in].active) && IS_PLAYER(cn))
			{
				ch[cn].citem = 0;
				it[in2].used = USE_EMPTY;
				do_char_log(cn, 1, "The key vanished.\n");
			}
		}
		else
		{
			for (n = 0; n<40; n++)
			{
				if ((in2 = ch[cn].item[n])!=0)
				{
					if (it[in2].temp==it[in].data[0])
					{
						break;
					}
				}
			}
			if (n<40)
			{
				lock = 1;
				if (it[in].data[3] && (!it[in].active) && IS_PLAYER(cn))
				{
					ch[cn].item[n] = 0;
					ch[cn].item_lock[n] = 0;
					it[in2].used = USE_EMPTY;
					do_char_log(cn, 1, "The key vanished.\n");
				}
			}
		}
		if (!lock && (in2 = ch[cn].citem)!=0 && !(in2 & 0x80000000) && it[in2].driver==3) // try to pick the lock
		{
			skill = it[in2].data[0]; // DrData0 on the lockpick is the bonus power of the pick
			power = it[in].data[2];  // DrData2 on the door is the difficulty (power) of the door
			if (!(ch[cn].flags & CF_LOCKPICK))
			{
				do_char_log(cn, 0, "You don't know what you're doing, but succeed in bending the lock-pick a bit.\n");
				item_damage_citem(cn, 1);
			}
			if (!power) // Zero power means the door cannot be picked.
			{
				do_char_log(cn, 0, "You failed to pick the lock. In fact, you're certain this door can't be picked.\n");
			}
			else if (skill < power + RANDOM(20))
			{
				if (skill+19 < power)
					do_char_log(cn, 0, "You failed to pick the lock. You're not sure this tool is capable...\n");
				else
					do_char_log(cn, 0, "You failed to pick the lock.\n");
			}
			else
			{
				lock = 1;
			}
			item_damage_citem(cn, 1);
		}
		if (it[in].data[1] && !lock)
		{
			do_char_log(cn, 0, "It's locked and you don't have the right key.\n");
			return 0;
		}
	}
	reset_go(it[in].x, it[in].y);
	remove_lights(it[in].x, it[in].y);

	do_area_sound(0, 0, it[in].x, it[in].y, 10);

	if (!it[in].active)
	{
		it[in].flags &= ~(IF_MOVEBLOCK | IF_SIGHTBLOCK);
		it[in].data[1] = 0;
	}
	else
	{
		temp  = it[in].temp;
		flags = it_temp[temp].flags & IF_SIGHTBLOCK;

		it[in].flags |= IF_MOVEBLOCK | flags;
		//if (lock)
		//{
			it[in].data[1] = 1;
		//}
	}
	reset_go(it[in].x, it[in].y);
	add_lights(it[in].x, it[in].y);
	do_area_notify(cn, 0, ch[cn].x, ch[cn].y, NT_SEE, cn, 0, 0, 0);

	return 1;
}

int use_create_item(int cn, int in)
{
	int in2, n;

	if (cn==0)
	{
		return 0;
	}
	if (it[in].active)
	{
		return 0;
	}

	n = it[in].data[0];

	if (n<=0 || n>=MAXTITEM)
	{
		return 0;
	}

	in2 = god_create_item(n);

	if (!god_give_char(in2, cn))
	{
		do_char_log(cn, 0, "Your backpack is full, so you can't take the %s.\n", it[in2].reference);
		it[in2].used = USE_EMPTY;
		return 0;
	}
	do_char_log(cn, 1, "You got a %s.\n", it[in2].reference);
	chlog(cn, "Got %s from %s", it[in2].name, it[in].name);

	if (it[in].data[1] && (it[in].driver==53 || it[in].power==53))
	{
		char buf[300];

		do_char_log(cn, 0, "You feel yourself form a magical connection with the %s.\n", it[in2].reference);
		it[in2].data[0] = cn;

		sprintf(buf, "%s Engraved in it are the letters \"%s\".",
		        it[in2].description, ch[cn].name);
		if (strlen(buf)<200)
		{
			strcpy(it[in2].description, buf);
		}
	}

	if (it[in].driver==54)
	{
		do_area_notify(cn, 0, it[in].x, it[in].y, NT_HITME, cn, 0, 0, 0);
	}

	return 1;
}

int use_create_gold(int cn, int in)
{
	int n;

	if (cn==0)
	{
		return 0;
	}
	if (it[in].active)
	{
		return 0;
	}

	n = it[in].data[0] + RANDOM(it[in].data[1]+1);

	do_char_log(cn, 1, "You found %d Gold.\n", n);
	chlog(cn, "Got %dG from %s", n, it[in].name);
	ch[cn].gold += n * 100;

	return 1;
}

int use_create_item2(int cn, int in)
{
	int in2, n, in3;

	if (cn==0)
	{
		return 0;
	}
	if (it[in].active)
	{
		return 0;
	}

	if ((in3 = ch[cn].citem)==0 || (in3 & 0x80000000))
	{
		return 0;
	}
	if (it[in3].temp!=it[in].data[1])
	{
		return 0;
	}

	n = it[in].data[0];

	if (n<=0 || n>=MAXTITEM)
	{
		return 0;
	}

	in2 = god_create_item(n);

	if (!god_give_char(in2, cn))
	{
		do_char_log(cn, 0, "Your backpack is full, so you can't take the %s.\n", it[in2].reference);
		it[in2].used = USE_EMPTY;
		return 0;
	}
	do_char_log(cn, 1, "You got a %s.\n", it[in2].reference);
	chlog(cn, "Got %s from %s", it[in2].name, it[in].name);

	it[in3].used = USE_EMPTY;
	ch[cn].citem = 0;

	return 1;
}

int use_recall_chest(int cn, int in)
{
	int in2, n;
	
	if (cn==0)
	{
		return 0;
	}
	if (it[in].active)
	{
		return 0;
	}
	
	for (n=0;n<40;n++)
	{
		if (!(in2 = ch[cn].item[n]))
		{
			continue;
		}
		if (it[in2].temp==IT_LAGSCROLL)
		{
			it[in2].used = USE_EMPTY;
			ch[cn].item[n] = 0;
		}
	}
	
	in2 = god_create_buff();

	strcpy(bu[in2].name, "Recall");
	strcpy(bu[in2].reference, "recall");
	strcpy(bu[in2].description, "Recall.");

	bu[in2].flags    |= IF_SPELL;
	bu[in2].temp      = SK_RECALL;
	bu[in2].sprite[1] = BUF_SPR_RECALL;
	bu[in2].power 	  = 999;
	bu[in2].data[1]   = ch[cn].temple_x;
	bu[in2].data[2]   = ch[cn].temple_y;
	bu[in2].data[4]   = 1;
	bu[in2].duration  = bu[in2].active = 180;
	
	
	fx_add_effect(6, 0, ch[cn].x, ch[cn].y, 0);
	
	add_spell(cn, in2);
	
	if (it[in].power==53) return use_create_item(cn, in);
		
	return use_create_item3(cn, in);
}

int use_create_item3(int cn, int in)
{
	int in2, n;

	if (cn==0)
	{
		return 0;
	}
	if (it[in].active)
	{
		return 0;
	}

	for (n = 0; n<10; n++)
	{
		if (!it[in].data[n])
		{
			break;
		}
	}
	if (n==0)
	{
		return 0;
	}

	n = RANDOM(n);
	n = it[in].data[n];

	// Special hack to enable mixed gold rewards
	if (n > 100000 && n < 200000)
	{
		n -= 100000;
		do_char_log(cn, 1, "You found %d Gold.\n", n);
		chlog(cn, "Got %dG from %s", n, it[in].name);
		ch[cn].gold += n * 100;
		return 1;
	}
	
	// Special hack to pull all tarot cards from the pool
	if (n == IT_CH_FOOL || n == IT_CH_FOOL_R)
	{
		n += RANDOM(22);
	}
	
	if (n<=0 || n>=MAXTITEM)
	{
		return 0;
	}
	
	// Check for magic item variant
	in2 = get_special_item(n, 0, 0, 0);

	if (!in2)
	{
		do_char_log(cn, 1, "It's empty...\n");
		return 1;
	}

	if (!god_give_char(in2, cn))
	{
		do_char_log(cn, 0, "Your backpack is full, so you can't take anything.\n");
		it[in2].used = USE_EMPTY;
		return 0;
	}
	do_char_log(cn, 1, "You got a %s.\n", it[in2].reference);
	chlog(cn, "Got %s from %s", it[in2].name, it[in].name);

	return 1;
}

// Stack cursor item onto target item
void use_stack_items(int cn, int in)
{
	int in2, stack_a, stack_b;
	int value_a, value_b;
	
	// Abort if no character (how did we get here??)
	if (cn==0)
	{
		return;
	}
	
	// Abort if there's no cursor item (how did we get here??)
	if ((in2 = ch[cn].citem)==0)
	{
		return;
	}
	
	// Make sure item is held by a char
	if (it[in].carried==0)
	{
		do_char_log(cn, 0, "Too difficult to do on the ground.\n");
		return;
	}
	
	stack_a = it[in].stack;
	if (stack_a <  1) stack_a =  1;
	if (stack_a > 10) stack_a = 10;
	
	stack_b = it[in2].stack;
	if (stack_b <  1) stack_b =  1;
	if (stack_b > 10) stack_b = 10;
	
	value_a = it[in].value / stack_a;
	value_b = it[in2].value / stack_b;
	
	// Clean stacking
	if (stack_a + stack_b <= 10)
	{
		it[in].stack = stack_a + stack_b;
		it[in].value = value_a * it[in].stack;
		god_take_from_char(in2, cn);
	}
	else
	{
		it[in].stack = 10;
		it[in2].stack = stack_a + stack_b - 10;
		
		it[in].value  = value_a * it[in].stack;
		it[in2].value = value_b * it[in2].stack;
	}
}

int use_mix_potion(int cn, int in)
{
	int in2, in3 = 0, font = 0;

	if (cn==0)
	{
		return 0;
	}
	if ((in2 = ch[cn].citem)==0 || (in2 & 0x80000000))
	{
		do_char_log(cn, 0, "What do you want to do with it?\n");
		return 0;
	}

	if (it[in].carried==0)
	{
		do_char_log(cn, 0, "Too difficult to do on the ground.\n");
		return 0;
	}

	chlog(cn, "Trying to mix %s with %s", it[in].name, it[in2].name);

	if (it[in].temp==IT_FLASK)
	{
		switch(it[in2].temp)
		{
			case IT_FLO_R: 	in3 = IT_POT_N_HP; 	font=2;	 break;
			case IT_FLO_G: 	in3 = IT_POT_N_EN; 	font=2;	 break;
			case IT_FLO_P: 	in3 = IT_POT_N_MP; 	font=2;	 break;
			//
			case IT_FLO_Y: 	in3 = IT_MIX_Y_; 	font=1;	 break;
			case IT_FLO_B: 	in3 = IT_MIX_B_; 	font=1;	 break;
			case IT_FLO_W: 	in3 = IT_MIX_W_; 	font=1;	 break;
			case IT_FLO_M:	in3 = IT_MIX_M_; 	font=1;	 break;
			case IT_FLO_C:	in3 = IT_MIX_C_; 	font=1;	 break;
			case IT_FLO_V:	in3 = IT_MIX_V_; 	font=1;	 break;
			//
			case IT_FLO_T:	in3 = IT_POT_T; 	font=2;	 break;
			case IT_FLO_O:	in3 = IT_POT_O; 	font=2;	 break;
			//
			case IT_FLO_1:	in3 = IT_MIX_1_; 	font=1;	 break;
			case IT_FLO_2:	in3 = IT_MIX_2_; 	font=1;	 break;
			case IT_FLO_3:	in3 = IT_MIX_3_; 	font=1;	 break;
			//
			default: do_char_log(cn, 0, "Sorry?\n"); return 0;
		}
	}
	else if (it[in].temp==IT_MIX_W_)
	{
		switch(it[in2].temp)
		{
			case IT_FLO_R: 	in3 = IT_POT_G_HP; 	font=2;	 break;
			case IT_FLO_G: 	in3 = IT_POT_G_EN; 	font=2;	 break;
			case IT_FLO_P: 	in3 = IT_POT_G_MP; 	font=2;	 break;
			//
			case IT_FLO_Y: 	in3 = IT_MIX_WY; 	font=1;	 break;
			case IT_FLO_B: 	in3 = IT_MIX_BW; 	font=1;	 break;
			case IT_FLO_W: 	in3 = IT_POT_DEAD; 	break;
			case IT_FLO_M:	in3 = IT_MIX_MW; 	font=1;	 break;
			case IT_FLO_C:	in3 = IT_MIX_CW; 	font=1;	 break;
			case IT_FLO_V:	in3 = IT_MIX_VW; 	font=1;	 break;
			//
			case IT_FLO_T:	in3 = IT_POT_DEAD; 	break;
			case IT_FLO_O:	in3 = IT_POT_DEAD; 	break;
			//
			case IT_FLO_1:	in3 = IT_POT_DEAD; 	break;
			case IT_FLO_2:	in3 = IT_POT_DEAD; 	break;
			case IT_FLO_3:	in3 = IT_POT_DEAD; 	break;
			//
			default: do_char_log(cn, 0, "Sorry?\n"); return 0;
		}
	}
	else if (it[in].temp==IT_MIX_Y_)
	{
		switch(it[in2].temp)
		{
			case IT_FLO_R: 	in3 = IT_POT_DEAD; 	break;
			case IT_FLO_G: 	in3 = IT_POT_DEAD; 	break;
			case IT_FLO_P: 	in3 = IT_POT_DEAD; 	break;
			//
			case IT_FLO_Y: 	in3 = IT_POT_DEAD; 	break;
			case IT_FLO_B: 	in3 = IT_MIX_BY; 	font=1;	 break;
			case IT_FLO_W: 	in3 = IT_MIX_WY; 	font=1;	 break;
			case IT_FLO_M:	in3 = IT_MIX_MY; 	font=1;	 break;
			case IT_FLO_C:	in3 = IT_MIX_CY; 	font=1;	 break;
			case IT_FLO_V:	in3 = IT_MIX_VY; 	font=1;	 break;
			//
			case IT_FLO_T:	in3 = IT_POT_DEAD; 	break;
			case IT_FLO_O:	in3 = IT_POT_DEAD; 	break;
			//
			case IT_FLO_1:	in3 = IT_POT_DEAD; 	break;
			case IT_FLO_2:	in3 = IT_POT_DEAD; 	break;
			case IT_FLO_3:	in3 = IT_POT_DEAD; 	break;
			//
			default: do_char_log(cn, 0, "Sorry?\n"); return 0;
		}
	}
	else if (it[in].temp==IT_MIX_B_)
	{
		switch(it[in2].temp)
		{
			case IT_FLO_R: 	in3 = IT_POT_DEAD; 	break;
			case IT_FLO_G: 	in3 = IT_POT_DEAD; 	break;
			case IT_FLO_P: 	in3 = IT_POT_DEAD; 	break;
			//
			case IT_FLO_Y: 	in3 = IT_MIX_BY; 	font=1;	 break;
			case IT_FLO_B: 	in3 = IT_POT_DEAD; 	break;
			case IT_FLO_W: 	in3 = IT_MIX_BW; 	font=1;	 break;
			case IT_FLO_M:	in3 = IT_MIX_BM; 	font=1;	 break;
			case IT_FLO_C:	in3 = IT_MIX_BC; 	font=1;	 break;
			case IT_FLO_V:	in3 = IT_MIX_BV; 	font=1;	 break;
			//
			case IT_FLO_T:	in3 = IT_POT_DEAD; 	break;
			case IT_FLO_O:	in3 = IT_POT_DEAD; 	break;
			//
			case IT_FLO_1:	in3 = IT_POT_DEAD; 	break;
			case IT_FLO_2:	in3 = IT_POT_DEAD; 	break;
			case IT_FLO_3:	in3 = IT_POT_DEAD; 	break;
			//
			default: do_char_log(cn, 0, "Sorry?\n"); return 0;
		}
	}
	else if (it[in].temp==IT_MIX_M_)
	{
		switch(it[in2].temp)
		{
			case IT_FLO_R: 	in3 = IT_POT_DEAD; 	break;
			case IT_FLO_G: 	in3 = IT_POT_DEAD; 	break;
			case IT_FLO_P: 	in3 = IT_POT_DEAD; 	break;
			//
			case IT_FLO_Y: 	in3 = IT_MIX_MY; 	font=1;	 break;
			case IT_FLO_B: 	in3 = IT_MIX_BM; 	font=1;	 break;
			case IT_FLO_W: 	in3 = IT_MIX_MW; 	font=1;	 break;
			case IT_FLO_M:	in3 = IT_POT_DEAD; 	break;
			case IT_FLO_C:	in3 = IT_MIX_CM; 	font=1;	 break;
			case IT_FLO_V:	in3 = IT_MIX_CV; 	font=1;	 break;
			//
			case IT_FLO_T:	in3 = IT_POT_DEAD; 	break;
			case IT_FLO_O:	in3 = IT_POT_DEAD; 	break;
			//
			case IT_FLO_1:	in3 = IT_POT_DEAD; 	break;
			case IT_FLO_2:	in3 = IT_MIX_12; 	font=1;	 break;
			case IT_FLO_3:	in3 = IT_MIX_13; 	font=1;	 break;
			//
			default: do_char_log(cn, 0, "Sorry?\n"); return 0;
		}
	}
	else if (it[in].temp==IT_MIX_C_)
	{
		switch(it[in2].temp)
		{
			case IT_FLO_R: 	in3 = IT_POT_DEAD; 	break;
			case IT_FLO_G: 	in3 = IT_POT_DEAD; 	break;
			case IT_FLO_P: 	in3 = IT_POT_DEAD; 	break;
			//
			case IT_FLO_Y: 	in3 = IT_MIX_CY; 	font=1;	 break;
			case IT_FLO_B: 	in3 = IT_MIX_BC; 	font=1;	 break;
			case IT_FLO_W: 	in3 = IT_MIX_CW; 	font=1;	 break;
			case IT_FLO_M:	in3 = IT_MIX_CM; 	font=1;	 break;
			case IT_FLO_C:	in3 = IT_POT_DEAD; 	break;
			case IT_FLO_V:	in3 = IT_MIX_CV; 	font=1;	 break;
			//
			case IT_FLO_T:	in3 = IT_POT_DEAD; 	break;
			case IT_FLO_O:	in3 = IT_POT_DEAD; 	break;
			//
			case IT_FLO_1:	in3 = IT_MIX_13; 	font=1;	 break;
			case IT_FLO_2:	in3 = IT_MIX_23; 	font=1;	 break;
			case IT_FLO_3:	in3 = IT_POT_DEAD; 	break;
			//
			default: do_char_log(cn, 0, "Sorry?\n"); return 0;
		}
	}
	else if (it[in].temp==IT_MIX_V_)
	{
		switch(it[in2].temp)
		{
			case IT_FLO_R: 	in3 = IT_POT_DEAD; 	break;
			case IT_FLO_G: 	in3 = IT_POT_DEAD; 	break;
			case IT_FLO_P: 	in3 = IT_POT_DEAD; 	break;
			//
			case IT_FLO_Y: 	in3 = IT_MIX_VY; 	font=1;	 break;
			case IT_FLO_B: 	in3 = IT_MIX_BV; 	font=1;	 break;
			case IT_FLO_W: 	in3 = IT_MIX_VW; 	font=1;	 break;
			case IT_FLO_M:	in3 = IT_MIX_MV; 	font=1;	 break;
			case IT_FLO_C:	in3 = IT_MIX_CV; 	font=1;	 break;
			case IT_FLO_V:	in3 = IT_POT_DEAD; 	break;
			//
			case IT_FLO_T:	in3 = IT_POT_DEAD; 	break;
			case IT_FLO_O:	in3 = IT_POT_DEAD; 	break;
			//
			case IT_FLO_1:	in3 = IT_MIX_12; 	font=1;	 break;
			case IT_FLO_2:	in3 = IT_POT_DEAD; 	break;
			case IT_FLO_3:	in3 = IT_MIX_23; 	font=1;	 break;
			//
			default: do_char_log(cn, 0, "Sorry?\n"); return 0;
		}
	}
	else if (it[in].temp==IT_MIX_WY)
	{
		switch(it[in2].temp)
		{
			case IT_FLO_R: 	in3 = IT_POT_H_HP; 	font=2;	 break;
			case IT_FLO_G: 	in3 = IT_POT_H_EN; 	font=2;	 break;
			case IT_FLO_P: 	in3 = IT_POT_H_MP; 	font=2;	 break;
			//
			case IT_FLO_Y: 	in3 = IT_POT_DEAD; 	break;
			case IT_FLO_B: 	in3 = IT_POT_LIFE; 	font=2;	 break;
			case IT_FLO_W: 	in3 = IT_POT_DEAD; 	break;
			case IT_FLO_M:	in3 = IT_POT_MOB; 	font=2;	 break;
			case IT_FLO_C:	in3 = IT_POT_AGL; 	font=2;	 break;
			case IT_FLO_V:	in3 = IT_POT_FRE; 	font=2;	 break;
			//
			case IT_FLO_T:	in3 = IT_POT_MAR; 	font=2;	 break;
			case IT_FLO_O:	in3 = IT_POT_IMM; 	font=2;	 break;
			//
			case IT_FLO_1:	in3 = IT_POT_DEAD; 	break;
			case IT_FLO_2:	in3 = IT_POT_DEAD; 	break;
			case IT_FLO_3:	in3 = IT_POT_DEAD; 	break;
			//
			default: do_char_log(cn, 0, "Sorry?\n"); return 0;
		}
	}
	else if (it[in].temp==IT_MIX_BW)
	{
		switch(it[in2].temp)
		{
			case IT_FLO_R: 	in3 = IT_POT_S_HP; 	font=2;	 break;
			case IT_FLO_G: 	in3 = IT_POT_S_EN; 	font=2;	 break;
			case IT_FLO_P: 	in3 = IT_POT_S_MP; 	font=2;	 break;
			//
			case IT_FLO_Y: 	in3 = IT_POT_LIFE; 	font=2;	 break;
			case IT_FLO_B: 	in3 = IT_POT_DEAD; 	break;
			case IT_FLO_W: 	in3 = IT_POT_DEAD; 	break;
			case IT_FLO_M:	in3 = IT_POT_CLA; 	font=2;	 break;
			case IT_FLO_C:	in3 = IT_POT_INT; 	font=2;	 break;
			case IT_FLO_V:	in3 = IT_POT_THO; 	font=2;	 break;
			//
			case IT_FLO_T:	in3 = IT_POT_RES; 	font=2;	 break;
			case IT_FLO_O:	in3 = IT_POT_BRU; 	font=2;	 break;
			//
			case IT_FLO_1:	in3 = IT_POT_DEAD; 	break;
			case IT_FLO_2:	in3 = IT_POT_DEAD; 	break;
			case IT_FLO_3:	in3 = IT_POT_DEAD; 	break;
			//
			default: do_char_log(cn, 0, "Sorry?\n"); return 0;
		}
	}
	else if (it[in].temp==IT_MIX_MW)
	{
		switch(it[in2].temp)
		{
			case IT_FLO_R: 	in3 = IT_POT_C_HP; 	font=2;	 break;
			case IT_FLO_G: 	in3 = IT_POT_C_EN; 	font=2;	 break;
			case IT_FLO_P: 	in3 = IT_POT_C_MP; 	font=2;	 break;
			//
			case IT_FLO_Y: 	in3 = IT_POT_MOB; 	font=2;	 break;
			case IT_FLO_B: 	in3 = IT_POT_CLA; 	font=2;	 break;
			case IT_FLO_W: 	in3 = IT_POT_DEAD; 	break;
			case IT_FLO_M:	in3 = IT_POT_DEAD; 	break;
			case IT_FLO_C:	in3 = IT_POT_WIL; 	font=2;	 break;
			case IT_FLO_V:	in3 = IT_POT_APT; 	font=2;	 break;
			//
			case IT_FLO_T:	in3 = IT_POT_DEF; 	font=2;	 break;
			case IT_FLO_O:	in3 = IT_POT_OFF; 	font=2;	 break;
			//
			case IT_FLO_1:	in3 = IT_POT_DEAD; 	break;
			case IT_FLO_2:	in3 = IT_POT_DEAD; 	break;
			case IT_FLO_3:	in3 = IT_POT_DEAD; 	break;
			//
			default: do_char_log(cn, 0, "Sorry?\n"); return 0;
		}
	}
	else if (it[in].temp==IT_MIX_VW)
	{
		switch(it[in2].temp)
		{
			case IT_FLO_R: 	in3 = IT_POT_L_HP; 	font=2;	 break;
			case IT_FLO_G: 	in3 = IT_POT_L_EN; 	font=2;	 break;
			case IT_FLO_P: 	in3 = IT_POT_L_MP; 	font=2;	 break;
			//
			case IT_FLO_Y: 	in3 = IT_POT_FRE; 	font=2;	 break;
			case IT_FLO_B: 	in3 = IT_POT_THO; 	font=2;	 break;
			case IT_FLO_W: 	in3 = IT_POT_DEAD; 	break;
			case IT_FLO_M:	in3 = IT_POT_APT; 	font=2;	 break;
			case IT_FLO_C:	in3 = IT_POT_STR; 	font=2;	 break;
			case IT_FLO_V:	in3 = IT_POT_DEAD; 	break;
			//
			case IT_FLO_T:	in3 = IT_POT_STE; 	font=2;	 break;
			case IT_FLO_O:	in3 = IT_POT_PER; 	font=2;	 break;
			//
			case IT_FLO_1:	in3 = IT_POT_DEAD; 	break;
			case IT_FLO_2:	in3 = IT_POT_DEAD; 	break;
			case IT_FLO_3:	in3 = IT_POT_DEAD; 	break;
			//
			default: do_char_log(cn, 0, "Sorry?\n"); return 0;
		}
	}
	else if (it[in].temp==IT_MIX_CW)
	{
		switch(it[in2].temp)
		{
			case IT_FLO_R: 	in3 = IT_POT_EXHP; 	font=2;	 break;
			case IT_FLO_G: 	in3 = IT_POT_EXEN; 	font=2;	 break;
			case IT_FLO_P: 	in3 = IT_POT_EXMP; 	font=2;	 break;
			//
			case IT_FLO_Y: 	in3 = IT_POT_AGL; 	font=2;	 break;
			case IT_FLO_B: 	in3 = IT_POT_INT; 	font=2;	 break;
			case IT_FLO_W: 	in3 = IT_POT_DEAD; 	break;
			case IT_FLO_M:	in3 = IT_POT_WIL; 	font=2;	 break;
			case IT_FLO_C:	in3 = IT_POT_DEAD; 	break;
			case IT_FLO_V:	in3 = IT_POT_STR; 	font=2;	 break;
			//
			case IT_FLO_T:	in3 = IT_POT_EVA; 	font=2;	 break;
			case IT_FLO_O:	in3 = IT_POT_PRE; 	font=2;	 break;
			//
			case IT_FLO_1:	in3 = IT_POT_DEAD; 	break;
			case IT_FLO_2:	in3 = IT_POT_DEAD; 	break;
			case IT_FLO_3:	in3 = IT_POT_DEAD; 	break;
			//
			default: do_char_log(cn, 0, "Sorry?\n"); return 0;
		}
	}
	else if (it[in].temp==IT_MIX_BY)
	{
		switch(it[in2].temp)
		{
			case IT_FLO_R: 	in3 = IT_POT_RAIN; 	font=1;	 break;
			case IT_FLO_G: 	in3 = IT_POT_RAIN; 	font=1;	 break;
			case IT_FLO_P: 	in3 = IT_POT_RAIN; 	font=1;	 break;
			//
			case IT_FLO_Y: 	in3 = IT_POT_DEAD; 	break;
			case IT_FLO_B: 	in3 = IT_POT_DEAD; 	break;
			case IT_FLO_W: 	in3 = IT_POT_LIFE; 	font=2;	 break;
			case IT_FLO_M:	in3 = IT_POT_DEAD; 	break;
			case IT_FLO_C:	in3 = IT_POT_DEAD; 	break;
			case IT_FLO_V:	in3 = IT_POT_DEAD; 	break;
			//
			case IT_FLO_T:	in3 = IT_POT_DEAD; 	break;
			case IT_FLO_O:	in3 = IT_POT_DEAD; 	break;
			//
			case IT_FLO_1:	in3 = IT_POT_DEAD; 	break;
			case IT_FLO_2:	in3 = IT_POT_DEAD; 	break;
			case IT_FLO_3:	in3 = IT_POT_DEAD; 	break;
			//
			default: do_char_log(cn, 0, "Sorry?\n"); return 0;
		}
	}
	else if (it[in].temp==IT_MIX_MY)
	{
		switch(it[in2].temp)
		{
			case IT_FLO_R: 	in3 = IT_POT_RAIN; 	font=1;	 break;
			case IT_FLO_G: 	in3 = IT_POT_RAIN; 	font=1;	 break;
			case IT_FLO_P: 	in3 = IT_POT_RAIN; 	font=1;	 break;
			//
			case IT_FLO_Y: 	in3 = IT_POT_DEAD; 	break;
			case IT_FLO_B: 	in3 = IT_POT_DEAD; 	break;
			case IT_FLO_W: 	in3 = IT_POT_MOB; 	font=2;	 break;
			case IT_FLO_M:	in3 = IT_POT_DEAD; 	break;
			case IT_FLO_C:	in3 = IT_POT_DEAD; 	break;
			case IT_FLO_V:	in3 = IT_POT_DEAD; 	break;
			//
			case IT_FLO_T:	in3 = IT_POT_DEAD; 	break;
			case IT_FLO_O:	in3 = IT_POT_DEAD; 	break;
			//
			case IT_FLO_1:	in3 = IT_POT_DEAD; 	break;
			case IT_FLO_2:	in3 = IT_POT_DEAD; 	break;
			case IT_FLO_3:	in3 = IT_POT_DEAD; 	break;
			//
			default: do_char_log(cn, 0, "Sorry?\n"); return 0;
		}
	}
	else if (it[in].temp==IT_MIX_VY)
	{
		switch(it[in2].temp)
		{
			case IT_FLO_R: 	in3 = IT_POT_RAIN; 	font=1;	 break;
			case IT_FLO_G: 	in3 = IT_POT_RAIN; 	font=1;	 break;
			case IT_FLO_P: 	in3 = IT_POT_RAIN; 	font=1;	 break;
			//
			case IT_FLO_Y: 	in3 = IT_POT_DEAD; 	break;
			case IT_FLO_B: 	in3 = IT_POT_DEAD; 	break;
			case IT_FLO_W: 	in3 = IT_POT_FRE; 	font=2;	 break;
			case IT_FLO_M:	in3 = IT_POT_DEAD; 	break;
			case IT_FLO_C:	in3 = IT_POT_DEAD; 	break;
			case IT_FLO_V:	in3 = IT_POT_DEAD; 	break;
			//
			case IT_FLO_T:	in3 = IT_POT_DEAD; 	break;
			case IT_FLO_O:	in3 = IT_POT_DEAD; 	break;
			//
			case IT_FLO_1:	in3 = IT_POT_DEAD; 	break;
			case IT_FLO_2:	in3 = IT_POT_DEAD; 	break;
			case IT_FLO_3:	in3 = IT_POT_DEAD; 	break;
			//
			default: do_char_log(cn, 0, "Sorry?\n"); return 0;
		}
	}
	else if (it[in].temp==IT_MIX_CY)
	{
		switch(it[in2].temp)
		{
			case IT_FLO_R: 	in3 = IT_POT_RAIN; 	font=1;	 break;
			case IT_FLO_G: 	in3 = IT_POT_RAIN; 	font=1;	 break;
			case IT_FLO_P: 	in3 = IT_POT_RAIN; 	font=1;	 break;
			//
			case IT_FLO_Y: 	in3 = IT_POT_DEAD; 	break;
			case IT_FLO_B: 	in3 = IT_POT_DEAD; 	break;
			case IT_FLO_W: 	in3 = IT_POT_AGL; 	font=2;	 break;
			case IT_FLO_M:	in3 = IT_POT_DEAD; 	break;
			case IT_FLO_C:	in3 = IT_POT_DEAD; 	break;
			case IT_FLO_V:	in3 = IT_POT_DEAD; 	break;
			//
			case IT_FLO_T:	in3 = IT_POT_DEAD; 	break;
			case IT_FLO_O:	in3 = IT_POT_DEAD; 	break;
			//
			case IT_FLO_1:	in3 = IT_POT_DEAD; 	break;
			case IT_FLO_2:	in3 = IT_POT_DEAD; 	break;
			case IT_FLO_3:	in3 = IT_POT_DEAD; 	break;
			//
			default: do_char_log(cn, 0, "Sorry?\n"); return 0;
		}
	}
	else if (it[in].temp==IT_MIX_BM)
	{
		switch(it[in2].temp)
		{
			case IT_FLO_R: 	in3 = IT_POT_RAIN; 	font=1;	 break;
			case IT_FLO_G: 	in3 = IT_POT_RAIN; 	font=1;	 break;
			case IT_FLO_P: 	in3 = IT_POT_RAIN; 	font=1;	 break;
			//
			case IT_FLO_Y: 	in3 = IT_POT_DEAD; 	break;
			case IT_FLO_B: 	in3 = IT_POT_DEAD; 	break;
			case IT_FLO_W: 	in3 = IT_POT_CLA; 	font=2;	 break;
			case IT_FLO_M:	in3 = IT_POT_DEAD; 	break;
			case IT_FLO_C:	in3 = IT_POT_DEAD; 	break;
			case IT_FLO_V:	in3 = IT_POT_DEAD; 	break;
			//
			case IT_FLO_T:	in3 = IT_POT_DEAD; 	break;
			case IT_FLO_O:	in3 = IT_POT_DEAD; 	break;
			//
			case IT_FLO_1:	in3 = IT_POT_DEAD; 	break;
			case IT_FLO_2:	in3 = IT_POT_DEAD; 	break;
			case IT_FLO_3:	in3 = IT_POT_DEAD; 	break;
			//
			default: do_char_log(cn, 0, "Sorry?\n"); return 0;
		}
	}
	else if (it[in].temp==IT_MIX_BV)
	{
		switch(it[in2].temp)
		{
			case IT_FLO_R: 	in3 = IT_POT_RAIN; 	font=1;	 break;
			case IT_FLO_G: 	in3 = IT_POT_RAIN; 	font=1;	 break;
			case IT_FLO_P: 	in3 = IT_POT_RAIN; 	font=1;	 break;
			//
			case IT_FLO_Y: 	in3 = IT_POT_DEAD; 	break;
			case IT_FLO_B: 	in3 = IT_POT_DEAD; 	break;
			case IT_FLO_W: 	in3 = IT_POT_THO; 	font=2;	 break;
			case IT_FLO_M:	in3 = IT_POT_DEAD; 	break;
			case IT_FLO_C:	in3 = IT_POT_DEAD; 	break;
			case IT_FLO_V:	in3 = IT_POT_DEAD; 	break;
			//
			case IT_FLO_T:	in3 = IT_POT_DEAD; 	break;
			case IT_FLO_O:	in3 = IT_POT_DEAD; 	break;
			//
			case IT_FLO_1:	in3 = IT_POT_DEAD; 	break;
			case IT_FLO_2:	in3 = IT_POT_DEAD; 	break;
			case IT_FLO_3:	in3 = IT_POT_DEAD; 	break;
			//
			default: do_char_log(cn, 0, "Sorry?\n"); return 0;
		}
	}
	else if (it[in].temp==IT_MIX_BC)
	{
		switch(it[in2].temp)
		{
			case IT_FLO_R: 	in3 = IT_POT_RAIN; 	font=1;	 break;
			case IT_FLO_G: 	in3 = IT_POT_RAIN; 	font=1;	 break;
			case IT_FLO_P: 	in3 = IT_POT_RAIN; 	font=1;	 break;
			//
			case IT_FLO_Y: 	in3 = IT_POT_DEAD; 	break;
			case IT_FLO_B: 	in3 = IT_POT_DEAD; 	break;
			case IT_FLO_W: 	in3 = IT_POT_INT; 	font=2;	 break;
			case IT_FLO_M:	in3 = IT_POT_DEAD; 	break;
			case IT_FLO_C:	in3 = IT_POT_DEAD; 	break;
			case IT_FLO_V:	in3 = IT_POT_DEAD; 	break;
			//
			case IT_FLO_T:	in3 = IT_POT_DEAD; 	break;
			case IT_FLO_O:	in3 = IT_POT_DEAD; 	break;
			//
			case IT_FLO_1:	in3 = IT_POT_DEAD; 	break;
			case IT_FLO_2:	in3 = IT_POT_DEAD; 	break;
			case IT_FLO_3:	in3 = IT_POT_DEAD; 	break;
			//
			default: do_char_log(cn, 0, "Sorry?\n"); return 0;
		}
	}
	else if (it[in].temp==IT_MIX_CM)
	{
		switch(it[in2].temp)
		{
			case IT_FLO_R: 	in3 = IT_POT_DEAD; 	break;
			case IT_FLO_G: 	in3 = IT_POT_DEAD; 	break;
			case IT_FLO_P: 	in3 = IT_POT_DEAD; 	break;
			//
			case IT_FLO_Y: 	in3 = IT_POT_DEAD; 	break;
			case IT_FLO_B: 	in3 = IT_POT_DEAD; 	break;
			case IT_FLO_W: 	in3 = IT_POT_WIL; 	font=2;	 break;
			case IT_FLO_M:	in3 = IT_POT_DEAD; 	break;
			case IT_FLO_C:	in3 = IT_POT_DEAD; 	break;
			case IT_FLO_V:	in3 = IT_POT_BRV; 	font=2;	 break;
			//
			case IT_FLO_T:	in3 = IT_POT_DEAD; 	break;
			case IT_FLO_O:	in3 = IT_POT_DEAD; 	break;
			//
			case IT_FLO_1:	in3 = IT_POT_DEAD; 	break;
			case IT_FLO_2:	in3 = IT_POT_LAB2; 	font=2;	 break;
			case IT_FLO_3:	in3 = IT_POT_DEAD; 	break;
			//
			default: do_char_log(cn, 0, "Sorry?\n"); return 0;
		}
	}
	else if (it[in].temp==IT_MIX_CV)
	{
		switch(it[in2].temp)
		{
			case IT_FLO_R: 	in3 = IT_POT_DEAD; 	break;
			case IT_FLO_G: 	in3 = IT_POT_DEAD; 	break;
			case IT_FLO_P: 	in3 = IT_POT_DEAD; 	break;
			//
			case IT_FLO_Y: 	in3 = IT_POT_DEAD; 	break;
			case IT_FLO_B: 	in3 = IT_POT_DEAD; 	break;
			case IT_FLO_W: 	in3 = IT_POT_STR; 	font=2;	 break;
			case IT_FLO_M:	in3 = IT_POT_BRV; 	font=2;	 break;
			case IT_FLO_C:	in3 = IT_POT_DEAD; 	break;
			case IT_FLO_V:	in3 = IT_POT_DEAD; 	break;
			//
			case IT_FLO_T:	in3 = IT_POT_DEAD; 	break;
			case IT_FLO_O:	in3 = IT_POT_DEAD; 	break;
			//
			case IT_FLO_1:	in3 = IT_POT_LAB2; 	font=2;	 break;
			case IT_FLO_2:	in3 = IT_POT_DEAD; 	break;
			case IT_FLO_3:	in3 = IT_POT_DEAD; 	break;
			//
			default: do_char_log(cn, 0, "Sorry?\n"); return 0;
		}
	}
	else if (it[in].temp==IT_MIX_MV)
	{
		switch(it[in2].temp)
		{
			case IT_FLO_R: 	in3 = IT_POT_DEAD; 	break;
			case IT_FLO_G: 	in3 = IT_POT_DEAD; 	break;
			case IT_FLO_P: 	in3 = IT_POT_DEAD; 	break;
			//
			case IT_FLO_Y: 	in3 = IT_POT_DEAD; 	break;
			case IT_FLO_B: 	in3 = IT_POT_DEAD; 	break;
			case IT_FLO_W: 	in3 = IT_POT_APT; 	font=2;	 break;
			case IT_FLO_M:	in3 = IT_POT_DEAD; 	break;
			case IT_FLO_C:	in3 = IT_POT_BRV; 	font=2;	 break;
			case IT_FLO_V:	in3 = IT_POT_DEAD; 	break;
			//
			case IT_FLO_T:	in3 = IT_POT_DEAD; 	break;
			case IT_FLO_O:	in3 = IT_POT_DEAD; 	break;
			//
			case IT_FLO_1:	in3 = IT_POT_DEAD; 	break;
			case IT_FLO_2:	in3 = IT_POT_DEAD; 	break;
			case IT_FLO_3:	in3 = IT_POT_LAB2; 	font=2;	 break;
			//
			default: do_char_log(cn, 0, "Sorry?\n"); return 0;
		}
	}
	else if (it[in].temp==IT_MIX_1_)
	{
		switch(it[in2].temp)
		{
			case IT_FLO_R: 	in3 = IT_POT_DEAD; 	break;
			case IT_FLO_G: 	in3 = IT_POT_DEAD; 	break;
			case IT_FLO_P: 	in3 = IT_POT_DEAD; 	break;
			//
			case IT_FLO_Y: 	in3 = IT_POT_DEAD; 	break;
			case IT_FLO_B: 	in3 = IT_POT_DEAD; 	break;
			case IT_FLO_W: 	in3 = IT_POT_DEAD; 	break;
			case IT_FLO_M:	in3 = IT_POT_DEAD; 	break;
			case IT_FLO_C:	in3 = IT_MIX_13; 	font=1;	 break;
			case IT_FLO_V:	in3 = IT_MIX_12; 	font=1;	 break;
			//
			case IT_FLO_T:	in3 = IT_POT_DEAD; 	break;
			case IT_FLO_O:	in3 = IT_POT_DEAD; 	break;
			//
			case IT_FLO_1:	in3 = IT_POT_DEAD; 	break;
			case IT_FLO_2:	in3 = IT_MIX_12; 	font=1;	 break;
			case IT_FLO_3:	in3 = IT_MIX_13; 	font=1;	 break;
			//
			default: do_char_log(cn, 0, "Sorry?\n"); return 0;
		}
	}
	else if (it[in].temp==IT_MIX_2_)
	{
		switch(it[in2].temp)
		{
			case IT_FLO_R: 	in3 = IT_POT_DEAD; 	break;
			case IT_FLO_G: 	in3 = IT_POT_DEAD; 	break;
			case IT_FLO_P: 	in3 = IT_POT_DEAD; 	break;
			//
			case IT_FLO_Y: 	in3 = IT_POT_DEAD; 	break;
			case IT_FLO_B: 	in3 = IT_POT_DEAD; 	break;
			case IT_FLO_W: 	in3 = IT_POT_DEAD; 	break;
			case IT_FLO_M:	in3 = IT_MIX_12; 	font=1;	 break;
			case IT_FLO_C:	in3 = IT_MIX_23; 	font=1;	 break;
			case IT_FLO_V:	in3 = IT_POT_DEAD; 	break;
			//
			case IT_FLO_T:	in3 = IT_POT_DEAD; 	break;
			case IT_FLO_O:	in3 = IT_POT_DEAD; 	break;
			//
			case IT_FLO_1:	in3 = IT_MIX_12; 	font=1;	 break;
			case IT_FLO_2:	in3 = IT_POT_DEAD; 	break;
			case IT_FLO_3:	in3 = IT_MIX_23; 	font=1;	 break;
			//
			default: do_char_log(cn, 0, "Sorry?\n"); return 0;
		}
	}
	else if (it[in].temp==IT_MIX_3_)
	{
		switch(it[in2].temp)
		{
			case IT_FLO_R: 	in3 = IT_POT_DEAD; 	break;
			case IT_FLO_G: 	in3 = IT_POT_DEAD; 	break;
			case IT_FLO_P: 	in3 = IT_POT_DEAD; 	break;
			//
			case IT_FLO_Y: 	in3 = IT_POT_DEAD; 	break;
			case IT_FLO_B: 	in3 = IT_POT_DEAD; 	break;
			case IT_FLO_W: 	in3 = IT_POT_DEAD; 	break;
			case IT_FLO_M:	in3 = IT_MIX_13; 	font=1;	 break;
			case IT_FLO_C:	in3 = IT_POT_DEAD; 	break;
			case IT_FLO_V:	in3 = IT_MIX_23; 	font=1;	 break;
			//
			case IT_FLO_T:	in3 = IT_POT_DEAD; 	break;
			case IT_FLO_O:	in3 = IT_POT_DEAD; 	break;
			//
			case IT_FLO_1:	in3 = IT_MIX_13; 	font=1;	 break;
			case IT_FLO_2:	in3 = IT_MIX_23; 	font=1;	 break;
			case IT_FLO_3:	in3 = IT_POT_DEAD; 	break;
			//
			default: do_char_log(cn, 0, "Sorry?\n"); return 0;
		}
	}
	else if (it[in].temp==IT_MIX_12)
	{
		switch(it[in2].temp)
		{
			case IT_FLO_R: 	in3 = IT_POT_DEAD; 	break;
			case IT_FLO_G: 	in3 = IT_POT_DEAD; 	break;
			case IT_FLO_P: 	in3 = IT_POT_DEAD; 	break;
			//
			case IT_FLO_Y: 	in3 = IT_POT_DEAD; 	break;
			case IT_FLO_B: 	in3 = IT_POT_DEAD; 	break;
			case IT_FLO_W: 	in3 = IT_POT_DEAD; 	break;
			case IT_FLO_M:	in3 = IT_POT_DEAD; 	break;
			case IT_FLO_C:	in3 = IT_POT_LAB2; 	font=2;	 break;
			case IT_FLO_V:	in3 = IT_POT_DEAD; 	break;
			//
			case IT_FLO_T:	in3 = IT_POT_DEAD; 	break;
			case IT_FLO_O:	in3 = IT_POT_DEAD; 	break;
			//
			case IT_FLO_1:	in3 = IT_POT_DEAD; 	break;
			case IT_FLO_2:	in3 = IT_POT_DEAD; 	break;
			case IT_FLO_3:	in3 = IT_POT_LAB2; 	font=2;	 break;
			//
			default: do_char_log(cn, 0, "Sorry?\n"); return 0;
		}
	}
	else if (it[in].temp==IT_MIX_23)
	{
		switch(it[in2].temp)
		{
			case IT_FLO_R: 	in3 = IT_POT_DEAD; 	break;
			case IT_FLO_G: 	in3 = IT_POT_DEAD; 	break;
			case IT_FLO_P: 	in3 = IT_POT_DEAD; 	break;
			//
			case IT_FLO_Y: 	in3 = IT_POT_DEAD; 	break;
			case IT_FLO_B: 	in3 = IT_POT_DEAD; 	break;
			case IT_FLO_W: 	in3 = IT_POT_DEAD; 	break;
			case IT_FLO_M:	in3 = IT_POT_LAB2; 	font=2;	 break;
			case IT_FLO_C:	in3 = IT_POT_DEAD; 	break;
			case IT_FLO_V:	in3 = IT_POT_DEAD; 	break;
			//
			case IT_FLO_T:	in3 = IT_POT_DEAD; 	break;
			case IT_FLO_O:	in3 = IT_POT_DEAD; 	break;
			//
			case IT_FLO_1:	in3 = IT_POT_LAB2; 	font=2;	 break;
			case IT_FLO_2:	in3 = IT_POT_DEAD; 	break;
			case IT_FLO_3:	in3 = IT_POT_DEAD; 	break;
			//
			default: do_char_log(cn, 0, "Sorry?\n"); return 0;
		}
	}
	else if (it[in].temp==IT_MIX_13)
	{
		switch(it[in2].temp)
		{
			case IT_FLO_R: 	in3 = IT_POT_DEAD; 	break;
			case IT_FLO_G: 	in3 = IT_POT_DEAD; 	break;
			case IT_FLO_P: 	in3 = IT_POT_DEAD; 	break;
			//
			case IT_FLO_Y: 	in3 = IT_POT_DEAD; 	break;
			case IT_FLO_B: 	in3 = IT_POT_DEAD; 	break;
			case IT_FLO_W: 	in3 = IT_POT_DEAD; 	break;
			case IT_FLO_M:	in3 = IT_POT_DEAD; 	break;
			case IT_FLO_C:	in3 = IT_POT_DEAD; 	break;
			case IT_FLO_V:	in3 = IT_POT_LAB2; 	font=2;	 break;
			//
			case IT_FLO_T:	in3 = IT_POT_DEAD; 	break;
			case IT_FLO_O:	in3 = IT_POT_DEAD; 	break;
			//
			case IT_FLO_1:	in3 = IT_POT_DEAD; 	break;
			case IT_FLO_2:	in3 = IT_POT_LAB2; 	font=2;	 break;
			case IT_FLO_3:	in3 = IT_POT_DEAD; 	break;
			//
			default: do_char_log(cn, 0, "Sorry?\n"); return 0;
		}
	}
	else
	{
		do_char_log(cn, 0, "Sorry?\n");
		return 0;
	}

	in3 = god_create_item(in3);
	it[in3].flags |= IF_UPDATE;

	it[in2].used = USE_EMPTY;
	ch[cn].citem = 0;

	god_take_from_char(in, cn);
	it[in].used = USE_EMPTY;

	god_give_char(in3, cn);
	
	do_char_log(cn, font, "You produced a %s.\n", it[in3].name);

	return 1;
}

int use_chain(int cn, int in)
{
	int in2, in3 = 0;

	if (cn==0)
	{
		return 0;
	}
	if ((in2 = ch[cn].citem)==0 || (in2 & 0x80000000))
	{
		do_char_log(cn, 0, "What do you want to do with it?\n");
		return 0;
	}

	if (it[in].carried==0)
	{
		do_char_log(cn, 0, "Too difficult to do on the ground.\n");
		return 0;
	}
	if (it[in2].temp!=206)
	{
		do_char_log(cn, 0, "Sorry?\n");
		return 0;
	}

	if (it[in].temp>=it[in].data[0])
	{
		do_char_log(cn, 0, "It won't fit anymore.\n");
		return 0;
	}

	chlog(cn, "Added %s to %s", it[in2].name, it[in].name);

	in3 = it[in].temp + 1;

	in3 = god_create_item(in3);
	it[in3].flags |= IF_UPDATE;

	it[in2].used = USE_EMPTY;
	ch[cn].citem = 0;

	god_take_from_char(in, cn);
	it[in].used = USE_EMPTY;

	god_give_char(in3, cn);

	return 1;
}

int stone_sword(int cn, int in)
{
	int n, in2;

	if (cn==0)
	{
		return 0;
	}
	if (it[in].active)
	{
		return 0;
	}

	n = it[in].data[0];

	if (n<=0 || n>=MAXTITEM)
	{
		return 0;
	}

	if (M_AT(cn, AT_STR)<it[in].data[1]) // was 40
	{
		do_char_log(cn, 0, "You're not strong enough.\n");
		return 0;
	}

	in2 = god_create_item(n);

	chlog(cn, "Got %s from %s", it[in2].name, it[in].name);

	god_give_char(in2, cn);
	do_char_log(cn, 1, "You got a %s.\n", it[in2].reference);

	return 1;
}

void finish_laby_teleport(int cn, int nr, int exp)
{
	int n, in2;

	if (ch[cn].data[20]<nr)
	{
		ch[cn].data[20] = nr;
		do_char_log(cn, 0, "You have solved the %d%s%s%s%s part of the Labyrinth.\n", nr,
		            nr==1 ? "st" : "",
		            nr==2 ? "nd" : "",
		            nr==3 ? "rd" : "",
		            nr>=4 ? "th" : "");

		do_give_exp(cn, exp, 0, -1);
		chlog(cn, "Solved Labyrinth Part %d", nr);
	}
	if ((in2 = ch[cn].citem) && !(in2 & 0x80000000) && (it[in2].flags & IF_LABYDESTROY))
	{
		ch[cn].citem = 0;
		it[in2].used = USE_EMPTY;
		do_char_log(cn, 1, "Your %s vanished.\n", it[in2].reference);
	}
	for (n = 0; n<40; n++)
	{
		if ((in2 = ch[cn].item[n]) && (it[in2].flags & IF_LABYDESTROY))
		{
			ch[cn].item[n] = 0;
			ch[cn].item_lock[n] = 0;
			it[in2].used = USE_EMPTY;
			do_char_log(cn, 1, "Your %s vanished.\n", it[in2].reference);
		}
	}
	for (n = 0; n<20; n++)
	{
		if ((in2 = ch[cn].worn[n]) && (it[in2].flags & IF_LABYDESTROY))
		{
			ch[cn].worn[n] = 0;
			it[in2].used = USE_EMPTY;
			do_char_log(cn, 1, "Your %s vanished.\n", it[in2].reference);
		}
	}
	for (n = 0; n<12; n++)
	{
		if ((in2 = ch[cn].alt_worn[n]) && (it[in2].flags & IF_LABYDESTROY))
		{
			ch[cn].alt_worn[n] = 0;
			it[in2].used = USE_EMPTY;
			do_char_log(cn, 1, "Your %s vanished.\n", it[in2].reference);
		}
	}

	for (n = 0; n<MAXBUFFS; n++)
	{
		if ((in2 = ch[cn].spell[n]))
		{
			ch[cn].spell[n] = 0;
			bu[in2].used = USE_EMPTY;
			do_char_log(cn, 1, "Your %s vanished.\n", bu[in2].name);
		}
	}

	fx_add_effect(6, 0, ch[cn].x, ch[cn].y, 0);
	god_transfer_char(cn, HOME_TEMPLE_X, HOME_TEMPLE_Y);
	char_play_sound(cn, ch[cn].sound + 22, -150, 0);
	fx_add_effect(6, 0, ch[cn].x, ch[cn].y, 0);

	ch[cn].temple_x = ch[cn].tavern_x = ch[cn].x;
	ch[cn].temple_y = ch[cn].tavern_y = ch[cn].y;
}

int is_nolab_item(int in)
{
	if (!IS_SANEITEM(in))
	{
		return 0;
	}

	switch(it[in].temp)
	{
		case 331:               // tavern scroll
		case 500:               // lag scroll
		case 592:               // gorn scroll
		case 903:               // forest scroll
		case 1114:              // staffers corner scroll
		case 1118:              // inn scroll
		case 1144:              // arena scroll
			return 1;
		default:
			return 0;
	}
}

void quick_teleport(int cn, int x, int y)
{
	fx_add_effect(6, 0, ch[cn].x, ch[cn].y, 0);
	god_transfer_char(cn, x, y);
	char_play_sound(cn, ch[cn].sound + 22, -150, 0);
	fx_add_effect(6, 0, ch[cn].x, ch[cn].y, 0);
}

void quick_mob_respawn(int dr, int v)
{
	int in, m=0, n=MAPX, j=0, x=0, y=MAPX;
	
	if (dr==73) 
	{
		j = 4;
		y = 105;
		if (v>5)
		{
			m = XY2M(726, 109);
			n = XY2M(830, 143);
		}
		else
		{
			m = XY2M(442, 510);
			n = XY2M(490, 586);
		}
	}
	
	for (; m<n; m++)
	{
		//x++; if (x>y) { x=0; m+=(MAPX-y); }
		if ((in = map[m].it)==0) 	continue;
		if (it[in].driver!=dr) 		continue;
		if (it[in].data[0]!=v) 		continue;
		
		enemyspawner(in, j, 1);
		enemyspawner(in, j, 1);
	}
}

int reward_teleport(int cn, int in)
{
	int n=0, in2=0;
	
	if (!cn)
	{
		return 1;
	}
	
	if (it[in].data[9])
	{
		if (it[in].data[9]<12)
		{
			if (it[in].data[9]>10 && !RANDOM(20))
			{
				switch (RANDOM(5))
				{	case  0: n = IT_RD_BRV; break;
					case  1: n = IT_RD_WIL; break;
					case  2: n = IT_RD_INT; break;
					case  3: n = IT_RD_AGL; break;
					default: n = IT_RD_STR; break;
				}	in2 = god_create_item(n);
			}
			else if (it[in].data[9]>5 && !RANDOM(10))
			{
				switch (RANDOM(2))
				{	case  0: n = IT_RD_HP; break;
					default: n = IT_RD_MP; break;
				}	in2 = god_create_item(n);
			}
			else
			{
				switch (RANDOM(4))
				{	case  0: n = IT_LUKS; break;
					default: n = IT_EXPS; break;
				}	in2 = god_create_item(n);
				it[in2].data[0] = (it[in].data[9]*1000 + RANDOM(it[in].data[9]+6)*250) / ((n==IT_LUKS)?25:1);
			}
		}
		else
		{
			in2 = god_create_item(n);
		}

		if (!god_give_char(in2, cn))
		{
			do_char_log(cn, 0, "You should tidy up your backpack sometime!\n");
			it[in2].used = USE_EMPTY;
		}
		else
		{
			do_char_log(cn, 1, "You got a %s.\n", it[in2].reference);
			chlog(cn, "Got %s from %s", it[in2].name, it[in].name);
		}
		
		if (it[in].data[9]<12)
		{
			switch (it[in].data[9])
			{
				case  1: quick_mob_respawn(73, 0); quick_teleport(cn, 473, 541); return 1;
				case  2: quick_mob_respawn(73, 1); quick_teleport(cn, 459, 541); return 1;
				case  3: quick_mob_respawn(73, 2); quick_teleport(cn, 459, 555); return 1;
				case  4: quick_mob_respawn(73, 3); quick_teleport(cn, 459, 569); return 1;
				case  5: quick_mob_respawn(73, 4); quick_teleport(cn, 473, 569); return 1;
				case  6: quick_mob_respawn(73, 5); quick_teleport(cn, 792, 133); return 1;
				case  7: quick_mob_respawn(73, 6); quick_teleport(cn, 799, 126); return 1;
				case  8: quick_mob_respawn(73, 7); quick_teleport(cn, 792, 119); return 1;
				case  9: quick_mob_respawn(73, 8); quick_teleport(cn, 778, 112); return 1;
				case 10: quick_mob_respawn(73, 9); quick_teleport(cn, 764, 112); return 1;
				case 11: quick_mob_respawn(73,10); quick_teleport(cn, 757, 126); return 1;
				default: break;
			}
		}
	}
	
	return teleport(cn, in);
}

int teleport(int cn, int in)
{
	int n, in2;

	if (!cn)
	{
		return 1;
	}
	if (it[in].flags & IF_USEACTIVATE && !(it[in].active))
	{
		return 1;
	}

	if ((in2 = ch[cn].citem) && is_nolab_item(in2))
	{
		ch[cn].citem = 0;
		it[in2].used = USE_EMPTY;
		do_char_log(cn, 1, "Your %s vanished.\n", it[in2].reference);
	}

	for (n = 0; n<40; n++)
	{
		if ((in2 = ch[cn].item[n]) && is_nolab_item(in2))
		{
			ch[cn].item[n] = 0;
			ch[cn].item_lock[n] = 0;
			it[in2].used = USE_EMPTY;
			do_char_log(cn, 1, "Your %s vanished.\n", it[in2].reference);
		}
	}

	for (n = 0; n<MAXBUFFS; n++)
	{
		if ((in2 = ch[cn].spell[n]) && (bu[in2].temp==SK_RECALL))
		{
			ch[cn].spell[n] = 0;
			bu[in2].used = USE_EMPTY;
		}
	}

	if (it[in].data[2])     // lab-solved teleport
	{
		return(use_labtransfer(cn, it[in].data[2], it[in].data[3]));
	}
	else
	{
		quick_teleport(cn, it[in].data[0], it[in].data[1]);
	}

	return 1;
}

int teleport2(int cn, int in2)
{
	int in;

	if (!cn)
	{
		return 1;
	}

	chlog(cn, "Used teleport scroll to %d,%d (%s)",
	      it[in2].data[0],
	      it[in2].data[1],
	      get_area_m(it[in2].data[0], it[in2].data[1], 0));

	if (it[in2].data[2] && it[in2].data[2] + TICKS * 60 * 4<globs->ticker)
	{
		chlog(cn, "Lag Scroll Time Difference: %d ticks (%.2fs)",
		      globs->ticker - it[in2].data[2],
		      (globs->ticker - it[in2].data[2]) / (double)TICKS);

		do_char_log(cn, 0, "Sorry, this lag scroll was too old. You need to use it four minutes after lagging out or earlier!\n");

		return 1;
	}

	in = god_create_buff();
	if (!in)
	{
		xlog("god_create_buff failed in teleport2");
		return 0;
	}

	strcpy(bu[in].name, "Teleport");
	bu[in].flags |= IF_SPELL;
	bu[in].sprite[1] = BUF_SPR_RECALL;
	bu[in].duration  = bu[in].active = 180;
	bu[in].temp  = SK_RECALL;
	bu[in].power = it[in2].power;
	bu[in].data[1] = it[in2].data[0];
	bu[in].data[2] = it[in2].data[1];

	if (!add_spell(cn, in))
	{
		do_char_log(cn, 1, "Magical interference neutralized the %s's effect.\n", bu[in].name);
		return 0;
	}
	fx_add_effect(7, 0, ch[cn].x, ch[cn].y, 0);

	return 1;
}

int use_labyrinth(int cn, int in)
{
	int n, in2, flag = 0;

	if (ch[cn].temple_x!=HOME_START_X && ch[cn].data[20]<=10)
	{
		if ((in2 = ch[cn].citem) && is_nolab_item(in2))
		{
			ch[cn].citem = 0;
			it[in2].used = USE_EMPTY;
			do_char_log(cn, 1, "Your %s vanished.\n", it[in2].reference);
		}

		for (n = 0; n<40; n++)
		{
			if ((in2 = ch[cn].item[n]) && is_nolab_item(in2))
			{
				ch[cn].item[n] = 0;
				ch[cn].item_lock[n] = 0;
				it[in2].used = USE_EMPTY;
				do_char_log(cn, 1, "Your %s vanished.\n", it[in2].reference);
			}
		}

		for (n = 0; n<MAXBUFFS; n++)
		{
			if ((in2 = ch[cn].spell[n]) && (bu[in2].temp==SK_RECALL))
			{
				ch[cn].spell[n] = 0;
				bu[in2].used = USE_EMPTY;
			}
		}
	}
	
	if (ch[cn].data[20]>=13)
	{
		do_char_log(cn, 0, "You have already solved all existing parts of the labyrinth. Please come back later.\n");
	}
	else if (ch[cn].temple_x==HOME_START_X)
	{
		do_char_log(cn, 0, "It seems you haven't turned in the Guard Captain's request yet. Do that first and come back.\n");
	}
	else
	{
		fx_add_effect(6, 0, ch[cn].x, ch[cn].y, 0);
		switch(ch[cn].data[20])
		{
			case  0: flag = god_transfer_char(cn, LAB_I_X,    LAB_I_Y); 	break;
			case  1: flag = god_transfer_char(cn, LAB_II_X,   LAB_II_Y); 	break;
			case  2: flag = god_transfer_char(cn, LAB_III_X,  LAB_III_Y); 	break;
			case  3: flag = god_transfer_char(cn, LAB_IV_X,   LAB_IV_Y); 	break;
			case  4: flag = god_transfer_char(cn, LAB_V_X,    LAB_V_Y); 	break;
			case  5: flag = god_transfer_char(cn, LAB_VI_X,   LAB_VI_Y); 	break;
			//
			case  6: flag = god_transfer_char(cn, LAB_VII_X,  LAB_VII_Y); 	break;
			case  7: flag = god_transfer_char(cn, LAB_VIII_X, LAB_VIII_Y); 	break;
			case  8: flag = god_transfer_char(cn, LAB_IX_X,   LAB_IX_Y); 	break;
			case  9: flag = god_transfer_char(cn, LAB_X_X,    LAB_X_Y); 	break;
			case 10: flag = god_transfer_char(cn, LAB_XI_X,   LAB_XI_Y); 	break;
			case 11: flag = god_transfer_char(cn, LAB_XII_X,  LAB_XII_Y); 	break;
			//
			case 12: flag = god_transfer_char(cn, LAB_XIII_X, LAB_XIII_Y); 	break;
			//
		}
		char_play_sound(cn, ch[cn].sound + 22, -150, 0);
		fx_add_effect(6, 0, ch[cn].x, ch[cn].y, 0);
	}

	if (flag)
	{
		ch[cn].temple_x = ch[cn].tavern_x = ch[cn].x;
		ch[cn].temple_y = ch[cn].tavern_y = ch[cn].y;
	}

	return 1;
}

int use_ladder(int cn, int in)
{
	god_transfer_char(cn, it[in].x + it[in].data[0], it[in].y + it[in].data[1]);

	return 1;
}

int use_bag(int cn, int in)
{
	int co, owner;
	int in2, nr;

	co = it[in].data[0];
	owner = ch[co].data[CHD_CORPSEOWNER];

	// prevent grave-robbing by the unauthorized
	if (owner && owner!=cn && !may_attack_msg(cn, owner, 0) && ch[owner].data[PCD_ALLOW]!=cn && IS_SANEPLAYER(owner))
	{
		do_char_log(cn, 0, "This is %s's grave, not yours. "
		            "You may only search it with %s permission.\n",
		            ch[owner].name, HIS_HER(owner));
		if (IS_ACTIVECHAR(co) && ch[owner].x)
		{
			do_char_log(owner, 0, "%s just tried to search your grave. "
			            "You must #ALLOW %s if you want %s to.\n",
			            ch[cn].name, ch[cn].name, HIM_HER(cn));
		}
		chlog(cn, "Failed to access grave of %s (%d)", ch[owner].name, owner);
		return 0;
	}

	if (!(ch[cn].flags & CF_SYS_OFF))
		do_char_log(cn, 1, "You search the remains of %s.\n", ch[co].reference);
	if ((ch[co].flags & CF_BODY) && ch[co].gold)
	{
		ch[cn].gold += ch[co].gold;
		chlog(cn, "Took %dG %dS", ch[co].gold / 100, ch[co].gold % 100);
		do_char_log(cn, 1, "You took %dG %dS.\n", ch[co].gold / 100, ch[co].gold % 100);
		ch[co].gold = 0;
	}
	if ((ch[co].flags & CF_BODY) && (ch[cn].flags & CF_AUTOLOOT))
	{
		for (nr=0; nr<40; nr++)
		{
			if ((in2 = ch[co].item[nr])!=0)
			{
				god_take_from_char(in2, co);
				if (god_give_char(in2, cn))
				{
					chlog(cn, "Autoloot %s", it[in2].name);
					do_char_log(cn, 1, "You autoloot a %s.\n", it[in2].reference);
				}
				else
				{
					god_give_char(in2, co);
					do_char_log(cn, 0, "You couldn't autoloot the %s because your inventory is full.\n", it[in2].reference);
					break;
				}
			}
		}
		if (nr == 40 && (in2 = ch[co].worn[WN_LRING]) && is_soulstone(in2))
		{
			god_take_from_char(in2, co);
			if (god_give_char(in2, cn))
			{
				chlog(cn, "Autoloot %s", it[in2].name);
				do_char_log(cn, 1, "You autoloot a %s.\n", it[in2].reference);
			}
			else
			{
				god_give_char(in2, co);
				do_char_log(cn, 0, "You couldn't autoloot the %s because your inventory is full.\n", it[in2].reference);
				nr--;
			}
		}
		if (nr == 40 && (in2 = ch[co].worn[WN_RRING]) && is_soulstone(in2))
		{
			god_take_from_char(in2, co);
			if (god_give_char(in2, cn))
			{
				chlog(cn, "Autoloot %s", it[in2].name);
				do_char_log(cn, 1, "You autoloot a %s.\n", it[in2].reference);
			}
			else
			{
				god_give_char(in2, co);
				do_char_log(cn, 0, "You couldn't autoloot the %s because your inventory is full.\n", it[in2].reference);
				nr--;
			}
		}
	}
	do_look_char(cn, co, 0, 0, 1);
	return 1;
}

int use_scroll(int cn, int in)
{
	int nr, pts, v;

	nr = it[in].data[0];

	if (B_SK(cn, nr))
	{
		if (it[in].data[1])
		{
			do_char_log(cn, 0, "You already know %s.\n", skilltab[nr].name);
			return 0;
		}
		v = B_SK(cn, nr);
		if (v>=ch[cn].skill[nr][2])
		{
			do_char_log(cn, 0, "You cannot raise %s any higher.\n", skilltab[nr].name);
			return 0;
		}
		do_char_log(cn, 1, "Raised %s by one.\n", skilltab[nr].name);
		chlog(cn, "Used scroll to raise %s by one", skilltab[nr].name);
		pts = skill_needed(v, ch[cn].skill[nr][3]);
		ch[cn].points_tot += pts;
		B_SK(cn, nr)++;

		do_check_new_level(cn);
	}
	else if (!ch[cn].skill[nr][2])
	{
		do_char_log(cn, 0, "This scroll teaches %s, which you cannot learn.\n", skilltab[nr].name);
		return 0;
	}
	else
	{
		B_SK(cn, nr) = 1;
		do_char_log(cn, 1, "You learned %s!\n", skilltab[nr].name);
		chlog(cn, "Used scroll to learn %s", skilltab[nr].name);
	}

	it[in].used = USE_EMPTY;
	god_take_from_char(in, cn);

	do_update_char(cn);

	return 1;
}

int use_scroll2(int cn, int in)
{
	int nr, pts, v;

	nr = it[in].data[0];

	v = B_AT(cn, nr);
	if (v>=ch[cn].attrib[nr][2])
	{
		do_char_log(cn, 0, "You cannot raise %s any higher.\n", at_name[nr]);
		return 0;
	}
	do_char_log(cn, 1, "Raised %s by one.\n", at_name[nr]);
	chlog(cn, "Used scroll to raise %s by one", at_name[nr]);

	pts = attrib_needed(v, ch[cn].attrib[nr][3]);
	ch[cn].points_tot += pts;
	B_AT(cn, nr)++;

	do_check_new_level(cn);

	it[in].used = USE_EMPTY;
	god_take_from_char(in, cn);

	do_update_char(cn);

	return 1;
}

int use_scroll3(int cn, int in)
{
	int am, pts = 0, v, n;

	am = it[in].data[0];

	v = ch[cn].hp[0];
	if (v>=ch[cn].hp[2])
	{
		do_char_log(cn, 0, "You cannot raise Hitpoints any higher.\n");
		return 0;
	}
	do_char_log(cn, 1, "Raised Hitpoints by %d.\n", am);
	chlog(cn, "Used scroll to raise Hitpoints by %d", am);

	for (n = 0; n<am; n++)
	{
		pts += hp_needed(n + v, ch[cn].hp[3]);
	}
	ch[cn].points_tot += pts;
	ch[cn].hp[0] += am;

	do_check_new_level(cn);

	it[in].used = USE_EMPTY;
	god_take_from_char(in, cn);

	do_update_char(cn);

	return 1;
}

int use_scroll4(int cn, int in)
{
	int am, pts = 0, v, n;

	am = it[in].data[0];

	v = ch[cn].end[0];
	if (v>=ch[cn].end[2])
	{
		do_char_log(cn, 0, "You cannot raise Endurance any higher.\n");
		return 0;
	}
	do_char_log(cn, 1, "Raised Endurance by %d.\n", am);
	chlog(cn, "Used scroll to raise Endurance by %d", am);

	/*
	for (n = 0; n<am; n++)
	{
		pts += end_needed(n + v, ch[cn].end[3]);
	}
	ch[cn].points_tot += pts;
	*/
	ch[cn].end[0] += am;

	do_check_new_level(cn);

	it[in].used = USE_EMPTY;
	god_take_from_char(in, cn);

	do_update_char(cn);

	return 1;
}

int use_scroll5(int cn, int in)
{
	int am, pts = 0, v, n;

	am = it[in].data[0];

	v = ch[cn].mana[0];
	if (v>=ch[cn].mana[2])
	{
		do_char_log(cn, 0, "You cannot raise Mana any higher.\n");
		return 0;
	}
	do_char_log(cn, 1, "Raised Mana by %d.\n", am);
	chlog(cn, "Used scroll to raise Mana by %d", am);

	for (n = 0; n<am; n++)
	{
		pts += mana_needed(n + v, ch[cn].mana[3]);
	}
	ch[cn].points_tot += pts;
	ch[cn].mana[0] += am;

	do_check_new_level(cn);

	it[in].used = USE_EMPTY;
	god_take_from_char(in, cn);

	do_update_char(cn);

	return 1;
}

// Exp Scroll
int use_scroll6(int cn, int in)
{
	int exp;
	
	if (it[in].min_rank>points2rank(ch[cn].points_tot))
	{
		do_char_log(cn, 0, "You're not experienced enough to use this (ironically).\n");
		return;
	}

	exp = it[in].data[0];
	
	do_char_log(cn, 1, "Used the scroll of experience.\n");
	chlog(cn, "used exp scroll, %d exp", exp);
	do_give_exp(cn, exp, 0, -1);

	it[in].used = USE_EMPTY;
	god_take_from_char(in, cn);

	do_update_char(cn);

	return 1;
}

// Luck Scroll
int use_scroll7(int cn, int in)
{
	int luck;

	luck = it[in].data[0];
	
	do_char_log(cn, 1, "Used the scroll of luck.\n");
	chlog(cn, "used luck scroll, %d luck", luck);
	do_char_log(cn, 2, "You get %d luck.\n", luck);
	ch[cn].luck += luck;

	it[in].used = USE_EMPTY;
	god_take_from_char(in, cn);

	do_update_char(cn);

	return 1;
}

// Gold Scroll
int use_scroll8(int cn, int in)
{
	int gold;

	gold = it[in].data[0];
	
	do_char_log(cn, 1, "Used the scroll of gold.\n");
	chlog(cn, "used gold scroll, %d gold", gold);
	do_char_log(cn, 2, "You get %dG %dS.\n", gold / 100, gold % 100);
	ch[cn].gold += gold;

	it[in].used = USE_EMPTY;
	god_take_from_char(in, cn);

	do_update_char(cn);

	return 1;
}

// Gender Scroll
int use_scroll9(int cn, int in)
{
	if (IS_FEMALE(cn))
	{	// Change from Female to Male
		ch[cn].kindred |= KIN_MALE; ch[cn].kindred &= ~KIN_FEMALE;
		if (IS_SEYAN_DU(cn))			ch[cn].sprite =  3024;
		else if (IS_ARCHTEMPLAR(cn))	ch[cn].sprite = 23504;
		else if (IS_BRAWLER(cn))		ch[cn].sprite = 26048;
		else if (IS_WARRIOR(cn))		ch[cn].sprite = 25552;
		else if (IS_SORCERER(cn))		ch[cn].sprite = 28624;
		else if (IS_SUMMONER(cn))		ch[cn].sprite = 25048;
		else if (IS_ARCHHARAKIM(cn))	ch[cn].sprite = 29648;
		else if (IS_TEMPLAR(cn))		ch[cn].sprite =  2000;
		else if (IS_MERCENARY(cn))		ch[cn].sprite =  5072;
		else if (IS_HARAKIM(cn))		ch[cn].sprite =  4048;
	}
	else
	{	// Change from Male to Female
		ch[cn].kindred |= KIN_FEMALE; ch[cn].kindred &= ~KIN_MALE;
		if (IS_SEYAN_DU(cn))			ch[cn].sprite = 11216;
		else if (IS_ARCHTEMPLAR(cn))	ch[cn].sprite = 24528;
		else if (IS_BRAWLER(cn))		ch[cn].sprite = 22948;
		else if (IS_WARRIOR(cn))		ch[cn].sprite = 26576;
		else if (IS_SORCERER(cn))		ch[cn].sprite = 27600;
		else if (IS_SUMMONER(cn))		ch[cn].sprite = 24048;
		else if (IS_ARCHHARAKIM(cn))	ch[cn].sprite = 30672;
		else if (IS_TEMPLAR(cn))		ch[cn].sprite =  8144;
		else if (IS_MERCENARY(cn))		ch[cn].sprite =  5072;
		else if (IS_HARAKIM(cn))		ch[cn].sprite =  6096;
	}
	
	do_char_log(cn, 1, "Used the scroll of gender swap. It's magic takes hold, and...\n");
	chlog(cn, "used gender scroll");
	
	fx_add_effect(7, 0, ch[cn].x, ch[cn].y, 0);
	
	it[in].used = USE_EMPTY;
	god_take_from_char(in, cn);

	do_update_char(cn);

	return 1;
}

// Greater Stat Scroll
int use_scrollA(int cn, int in)
{
	int nr, n, m=0, mm=0;

	nr = it[in].data[0];
	
	if (nr<5)	// Attributes
	{
		for (n=0;n<5;n++) m += ch[cn].attrib[n][1];
		
		if (m>=10)
		{
			do_char_log(cn, 0, "You cannot use any more greater attribute scrolls.\n");
			return 0;
		}

		do_char_log(cn, 1, "Raised %s implicitly by one.\n", at_name[nr]);
		chlog(cn, "Used scroll to raise %s implicitly by one", at_name[nr]);
		
		ch[cn].attrib[nr][1]++;
		
		do_char_log(cn, 1, "You have used %d out of 10 greater attribute scrolls.\n", m+1);
	}
	else	// Skills
	{
		nr = it[in].data[1];
		
		for (n=0;n<50;n++) { m += ch[cn].skill[n][1]; mm = mm + (ch[cn].skill[n][0]?1:0); }
		
		if (m>=mm)
		{
			do_char_log(cn, 0, "You cannot use any more greater skill scrolls.\n");
			return 0;
		}
		
		if (ch[cn].skill[nr][1]>=10)
		{
			do_char_log(cn, 0, "You cannot raise that skill implicitly higher than 10.\n");
			return 0;
		}

		do_char_log(cn, 1, "Raised %s implicitly by one.\n", skilltab[nr].name);
		chlog(cn, "Used scroll to raise %s implicitly by one", skilltab[nr].name);
		
		ch[cn].skill[nr][1]++;
		
		do_char_log(cn, 1, "You have used %d out of %d greater skill scrolls.\n", m+1, mm);
	}
	
	fx_add_effect(6, 0, ch[cn].x, ch[cn].y, 0);
	it[in].used = USE_EMPTY;
	god_take_from_char(in, cn);

	do_update_char(cn);

	return 1;
}

// Greater Reset Scroll
int use_scroll_R(int cn, int in)
{
	int n;
	
	for (n=0;n<5;n++) ch[cn].attrib[n][1] = 0;
	for (n=0;n<50;n++) ch[cn].skill[n][1] = 0;
	
	do_char_log(cn, 1, "You used the reset scroll. Your implicit bonuses were reset.\n");
	
	fx_add_effect(6, 0, ch[cn].x, ch[cn].y, 0);
	it[in].used = USE_EMPTY;
	god_take_from_char(in, cn);

	do_update_char(cn);
	
	return 1;
}

// Sprite change scroll
int use_scroll_S(int cn, int in)
{
	int v;
	
	if (it[in].data[1])
	{
		v = ch[cn].temp;
		ch[cn].sprite = ch_temp[v].sprite;
		do_char_log(cn, 1, "You used the scroll. Your form returned to normal.\n");
	}
	else
	{
		v = it[in].data[0];
		ch[cn].sprite = v;
		do_char_log(cn, 1, "You used the scroll. Your form shifted and changed...\n");
	}
	
	fx_add_effect(6, 0, ch[cn].x, ch[cn].y, 0);
	it[in].used = USE_EMPTY;
	god_take_from_char(in, cn);

	do_update_char(cn);
}

int use_map_contract(int cn, int in)
{
	static int cntrct_1a[] = {
	//	Ex Enemy	Ex Divin	En Roam		Clr Exp		Clr Luck	Gold Pl
		0x00000001, 0x00000002, 0x00000100, 0x00100000, 0x00200000, 0x08000000
	};
	static int cntrct_1b[] = {
	//	Underwtr	En Perc		Pl Frag		En Tarot	En Tarot2
		0x00000008, 0x00000200, 0x00020000, 0x01000000, 0x02000000
	};
	static int cntrct_2a[] = {
	//	Boss En		Clr BSP		Kill Exp	+1 Rank
		0x00000080, 0x00400000, 0x04000000, 0x10000000
	};
	static int cntrct_2b[] = {
	//	Spikes		Undead		En Sharp	En Forti	En Wiser	Pl Debil
		0x00000020, 0x00000040, 0x00001000, 0x00002000, 0x00008000, 0x00010000
	};
	static int cntrct_3a[] = {
	//	Ex Chest	Ex Shrine	Ex OSP		+2 Rank
		0x00000004, 0x00000010, 0x00800000, 0x20000000
	};
	static int cntrct_3b[] = {
	//	En Resis	En Skill	En Haste	Pl Stigm	Pl Hyper
		0x00000400, 0x00000800, 0x00004000, 0x00040000, 0x00080000
	};
	int in2, spr, n, m, panic, font;
	int rank, tier;
	char buf[300];
	unsigned int flags=0;
	
	in2 = ch[cn].citem;
	
	if (!in2)
	{
		return 0;
	}
	
	if (it[in].data[0])
	{
		do_char_log(cn, 0, "This contract is already signed.\n");
		return 0;
	}
	
	switch (it[in2].temp)
	{
		case MCT_QUILL_Y: tier = 0; spr = MSP_Y; font = 1; break;
		case MCT_QUILL_G: tier = 1; spr = MSP_G; font = 2; break;
		case MCT_QUILL_B: tier = 2; spr = MSP_B; font = 3; break;
		case MCT_QUILL_R: tier = 3; spr = MSP_R; font = 0; break;
		default: return 0;
	}
	
	it[in].data[0] = cn;
	if (!it[in].data[4]) it[in].data[1] = RANDOM(MSN_COUNT);
	it[in].data[2] = tier;
	
	if (tier)
	{
		for (n=0;n<tier;n++)
		{
			for (panic=0,m=0;panic<99;panic++)
			{
				switch (RANDOM(tier)+1)
				{
					case  1: m = cntrct_1a[RANDOM(sizeof(cntrct_1a) / sizeof(cntrct_1a[0]))]; break;
					case  2: m = cntrct_2a[RANDOM(sizeof(cntrct_2a) / sizeof(cntrct_2a[0]))]; break;
					default: m = cntrct_3a[RANDOM(sizeof(cntrct_3a) / sizeof(cntrct_3a[0]))]; break;
				}
				if (!(flags & m))
				{
					chlog(cn, "Added %d to flag %d", m, flags);
					flags |= m;
					break;
				}
				if (panic>9)
				{
					chlog(cn, "Panic! Couldn't add flag A to contract");
					break;
				}
			}
			for (panic=0,m=0;panic<99;panic++)
			{
				switch (RANDOM(tier)+1)
				{
					case  1: m = cntrct_1b[RANDOM(sizeof(cntrct_1b) / sizeof(cntrct_1b[0]))]; break;
					case  2: m = cntrct_2b[RANDOM(sizeof(cntrct_2b) / sizeof(cntrct_2b[0]))]; break;
					default: m = cntrct_3b[RANDOM(sizeof(cntrct_3b) / sizeof(cntrct_3b[0]))]; break;
				}
				if (!(flags & m))
				{
					chlog(cn, "Added %d to flag %d", m, flags);
					flags |= m;
					break;
				}
				if (panic>9)
				{
					chlog(cn, "Panic! Couldn't add flag B to contract");
					break;
				}
			}
		}
	}
	
	it[in].data[3] = flags;
	it[in].data[4] = 1;
	it[in].sprite[0] = spr;
	it[in].flags |= IF_UPDATE;
	
	sprintf(buf, "Signed, %s.", ch[cn].name);
	if (strlen(buf)<200)
	{
		strcpy(it[in].description, buf);
	}
	
	if (in2)
	{
		ch[cn].citem = 0;
		it[in2].used = USE_EMPTY;
	}
	
	do_char_log(cn, font, "You signed the contract.\n");
	
	return 1;
}

int use_map_artifact(int cn, int in)
{
	if (cn==0)
	{
		return 0;
	}
	if (it[in].active)
	{
		return 0;
	}
	
	if (!use_create_item(cn, in)) return 0;
	
	// Contract progress
	if (CONT_SCEN(cn)==4) 
	{
		add_map_progress(CONT_NUM(cn));
		if (CONT_PROG(cn)>=CONT_GOAL(cn)) 
			do_char_log(cn, 2, "You got what you came for! You're good to go!\n");
	}
	
	return 1;
}

int use_map_pentigram(int cn, int in)
{
	int spawnT = 350, v;
	int co, bf, flags, base, affix=0, tarot, x, y, m, n;
	
	if (cn==0)
	{
		return 0;
	}
	if (it[in].active)
	{
		return 0;
	}
	
	x = it[in].x;
	y = it[in].y;
	m = x + y * MAPX;
	
	base  = max(15, (it[in].data[0]-5)*11/2+15) - 3;
	flags = it[in].data[1];
	tarot = it[in].data[2];
	
	v = RANDOM(2)+2;
	
	for (n=0;n<v;n++)
	{
		affix = 0;
		if (!RANDOM(27)) affix = 3;
		co = generate_map_enemy(spawnT, RANDOM(NUM_MAP_ENEM)+11, x, y, base - RANDOM(8), affix, tarot);
		if (co && (flags & 0x00000040)) ch[co].flags |= CF_UNDEAD;
		if (co && (flags & 0x00000100)) ch[co].data[73] += 6;
		if (co && (flags & 0x00000200)) ch[co].skill[SK_PERCEPT][1] = max(50, ch[co].skill[SK_PERCEPT][0]/8);
		if (co && (flags & 0x00000400)) ch[co].skill[SK_RESIST][1] = max(50, ch[co].skill[SK_RESIST][0]/8);
		if (co && (flags & 0x00000800)) ch[co].skill[SK_HAND][1] = max(50, ch[co].skill[SK_HAND][0]/8);
		if (co && (flags & 0x00001000)) ch[co].weapon_bonus = max(50, ch[co].weapon/8);
		if (co && (flags & 0x00002000)) ch[co].armor_bonus = max(50, ch[co].armor/8);
		if (co && (flags & 0x00004000)) ch[co].speed_mod = it[in].data[0]*2;
		if (co && (flags & 0x00008000) && (bf = make_new_buff(co, 104, BUF_SPR_STARL, 300, SP_DUR_MAPMOD, 0)))
		{
			strcpy(bu[bf].name, "* Wisdom");
			bu[bf].spell_mod[1] = it[in].data[0]; bu[bf].data[4] = 1; add_spell(co, bf);
		}
		if (co && (flags & 0x04000000) && (bf = make_new_buff(co, 105, BUF_SPR_POME, 300, SP_DUR_MAPMOD, 0)))
		{
			strcpy(bu[bf].name, "* Bonus EXP"); 
			bu[bf].data[4] = 1; add_spell(co, bf);
		}
		ch[co].data[29] = m;
		ch[co].data[60] = TICKS * 23;
		if (co) do_update_char(co);
	}
	
	// Contract progress
	if (CONT_SCEN(cn)==7) 
	{
		add_map_progress(CONT_NUM(cn));
		do_char_log(cn, 1, "You touched the pentagram. That's %d of %d.\n", CONT_PROG(cn), CONT_GOAL(cn));
		if (CONT_PROG(cn)>=CONT_GOAL(cn)) 
			do_char_log(cn, 2, "That's all of them! You're good to go!\n");
	}
	
	return 1;
}

int use_map_chest(int cn, int in)
{
	// data 0 = rank
	int in2, n;
	
	if (cn==0)
	{
		return 0;
	}
	if (it[in].active)
	{
		return 0;
	}
	
	switch (RANDOM(it[in].data[0]+1))
	{
		case  0: 	n = IT_SILVAMMY+RANDOM(2)*5; 				break;
		case  1: 	n = get_best_worn(cn, WN_HEAD); 			break;
		case  2: 	n = get_best_worn(cn, WN_BODY); 			break;
		case  3: 	n = get_best_worn(cn, WN_RHAND); 			break;
		case  4: 	n = get_best_worn(cn, WN_LHAND); 			break;
		case  5: 	n = IT_EXPS+RANDOM(2); 						break; 
		case  6: 	n = IT_POP_SPELL+RANDOM(7); 				break; // Staff Sergeant
		case  7: 	n = -1000; 									break;
		case  8: 	n = !RANDOM(3)?(RANDOM(2)?102:125):101; 	break;
		case  9: 	n = IT_CH_FOOL; 							break;
		case 10: 	n = IT_POP_ASPEL+RANDOM(7); 				break; // Second Lieutenant
		case 11: 	n = IT_POT_EXHP+RANDOM(18); 				break;
		case 12: 	n = IT_RPOT; 								break;
		case 13: 	n = !RANDOM(3)?(RANDOM(2)?131:273):127; 	break;
		case 14: 	n = IT_POT_VITA+RANDOM(3); 					break;
		case 15: 	n = IT_RD_HP+RANDOM(2)*2; 					break;
		case 16: 	n = IT_POP_ISPEL+RANDOM(7); 				break; // Brigadier General
		case 17: 	n = IT_BL_SOLAR+RANDOM(2); 					break;
		case 18: 	n = IT_GPOT; 								break;
		case 19: 	n = IT_CH_FOOL_R; 							break;
		case 20: 	n = IT_RD_BRV+RANDOM(5); 					break; 
		case 21: 	n = IT_POP_SSPEL+RANDOM(7); 				break; // Knight
		case 22: 	n = IT_POT_D_HP+RANDOM(3)*6; 				break;
		case 23: 	n = 1243+RANDOM(2)*2; 						break;
		case 24: 	n = IT_SPOT; 								break;
		case 25: 	n = IT_OS_BRV+RANDOM(5); 					break; // Warlord
		case 26: 	n = IT_OS_BRV+RANDOM(5); 					break; 
		default: 	n = IT_OS_SK; 								break; // Warlord + 2
	}
	
	if (n == IT_CH_FOOL || n == IT_CH_FOOL_R)
	{
		n += RANDOM(22);
	}
	
	if (n==-1000)
	{
		n = (RANDOM(it[in].data[0])+1)*(RANDOM(it[in].data[0])+1)*(RANDOM(6)+5);
		do_char_log(cn, 1, "You found %d Gold.\n", n);
		chlog(cn, "Got %dG from %s", n, it[in].name);
		ch[cn].gold += n * 100;
	}
	else
	{
		if (n<=0 || n>=MAXTITEM)
		{
			return 0;
		}
		
		in2 = get_special_item(n, 0, 0, 0);
		
		if (!in2)
		{
			do_char_log(cn, 1, "It's empty...\n");
		}
		else
		{
			if (n==IT_OS_SK) it[in2].data[1] = RANDOM(50);
			
			if (!god_give_char(in2, cn))
			{
				do_char_log(cn, 0, "Your backpack is full, so you can't take anything.\n");
				it[in2].used = USE_EMPTY;
				return 0;
			}
			do_char_log(cn, 1, "You got a %s.\n", it[in2].reference);
			chlog(cn, "Got %s from %s", it[in2].name, it[in].name);
		}
	}

	// Contract progress
	if (CONT_SCEN(cn)==6)
	{
		add_map_progress(CONT_NUM(cn));
		if (CONT_PROG(cn)>=CONT_GOAL(cn)) 
			do_char_log(cn, 2, "That's all of them! You're good to go!\n");
		else
			do_char_log(cn, 1, "%d down, %d to go.\n", CONT_PROG(cn), (CONT_GOAL(cn)-CONT_PROG(cn)));
	}
	
	return 1;
}

int get_random_backpack_item(int cn)
{
	int catalog[40] = {0};
	int c, n;
	
	for (n = 0, c = 0; n<40; n++)
	{
		if (catalog[c] = ch[cn].item[n]) c++;
	}
	
	if (!c) return 0;
	
	return (catalog[RANDOM(c)]);
}

int get_random_gear_item(int cn)
{
	int catalog[12] = {0};
	int c, n;
	
	for (n = 0, c = 0; n<12; n++)
	{
		if (catalog[c] = ch[cn].worn[n]) c++;
	}
	
	if (!c) return 0;
	
	return (catalog[RANDOM(c)]);
}

int use_map_shrine(int cn, int in)
{
	// data 0 = type		1=Red	2=Blue	3=Green
	// data 1 = effect
	int inc, inr, buff=0, p=0, temp=0, rank, tier, n, m=0, v, c=0, panic=0;
	int catalog[70] = {0};
	
	if (cn==0)
	{
		return 0;
	}
	if (it[in].active)
	{
		return 0;
	}
	
	rank = it[in].data[2];
	tier = it[in].data[3]*4/3;
	
	switch (it[in].data[0])
	{
		case  1:		// Red - Item effects
			inc = ch[cn].citem;
			switch (it[in].data[1])
			{
				case  1:
					do_char_log(cn, 0, "Duplicity.\n");
					if (inc)   temp = it[inc].temp;
					if (!temp) temp = it[inc].orig_temp;
					if (!temp) temp = it[get_random_backpack_item(cn)].temp;
					inr = god_create_item(temp);
					if (!inr)
					{
						do_char_log(cn, 1, "Nothing happened...\n");
						break;
					}
					if (!god_give_char(inr, cn))
					{
						do_char_log(cn, 1, "Your backpack is full...\n");
						it[inr].used = USE_EMPTY;
						break;
					}
					do_char_log(cn, 1, "You got a %s.\n", it[inr].reference);
					break;
				case  2:
					do_char_log(cn, 0, "Augmentation.\n");
					if (!(it[inc].placement)) inc = get_random_gear_item(cn);
					if (!inc)
					{
						do_char_log(cn, 1, "Nothing happened...\n");
						break;
					}
					if (it[inc].flags & IF_AUGMENTED)
					{
						do_char_log(cn, 1, "But it had been done to the %s before.\n", it[inc].reference);
						break;
					}
					if (RANDOM(2))
					{
						it[inc].attrib[RANDOM(5)][0] += 1+rank/10;
					}
					else
					{
						it[inc].skill[RANDOM(50)][0] += 1+rank/10;
					}
					it[inc].flags |= IF_AUGMENTED;
					do_char_log(cn, 1, "Your %s was altered.\n", it[inc].reference);
					break;
				case  3:
					do_char_log(cn, 0, "Power diminished.\n");
					if (!inc) inc = RANDOM(2)?get_random_backpack_item(cn):get_random_gear_item(cn);
					if (!inc)
					{
						do_char_log(cn, 1, "Nothing happened...\n");
						break;
					}
					if (it[inc].flags & IF_DIMINISHED)
					{
						do_char_log(cn, 1, "But it had been done to the %s before.\n", it[inc].reference);
						break;
					}
					it[inc].power = it[inc].power/2;
					it[inc].flags |= IF_DIMINISHED;
					do_char_log(cn, 1, "Your %s was altered.\n", it[inc].reference);
					break;
				case  4:
					do_char_log(cn, 0, "An imprint of the soul.\n");
					if (!inc) inc = RANDOM(2)?get_random_backpack_item(cn):get_random_gear_item(cn);
					if (!inc)
					{
						do_char_log(cn, 1, "Nothing happened...\n");
						break;
					}
					if (it[inc].flags & IF_SOULSPLIT)
					{
						do_char_log(cn, 1, "But it had been done to the %s before.\n", it[inc].reference);
						break;
					}
					inr = god_create_item(IT_SOULSTONE);
					if (!inr)
					{
						do_char_log(cn, 1, "Nothing happened...\n");
						break;
					}
					else
					{
						if (rank > 24)
							rank = 24;
						
						sprintf(it[inr].name, "Soulstone");
						sprintf(it[inr].reference, "soulstone");
						sprintf(it[inr].description, "Level %d soulstone, holding %d exp.", rank, 0);
						
						it[inr].data[0] = rank;
						it[inr].data[1] = 0;
						it[inr].temp = 0;
						it[inr].driver = 68;
						
						for (n = 0; n < 4; n++)
						{
							if (it[inc].attrib[n][0])
							{
								it[inr].attrib[n][0] = get_sb(n+50, 1);
								it[inr].attrib[n][1] = get_sb(n+50, 0)*2;
								m += get_sb(n+50, 0)*2;
								catalog[c] = n+50; c++;
							}
						}
						for (n = 0; n < 50; n++)
						{
							if (it[inc].skill[n][0])
							{
								it[inr].skill[n][0] = get_sb(n, 1);
								it[inr].skill[n][1] = get_sb(n, 0)*2;
								m += get_sb(n, 0)*2;
								catalog[c] = n; c++;
							}
						}
						if (it[inc].hp[0])
						{
							n = 55; it[inr].hp[0] = get_sb(n, 1); it[inr].hp[1] = get_sb(n, 0)*2; 
							m += get_sb(n, 0)*2; catalog[c] = n; c++;
						}
						if (it[inc].end[0])
						{
							n = 56; it[inr].end[0] = get_sb(n, 1); it[inr].end[1] = get_sb(n, 0)*2; 
							m += get_sb(n, 0)*2; catalog[c] = n; c++;
						}
						if (it[inc].mana[0])
						{
							n = 57; it[inr].mana[0] = get_sb(n, 1); it[inr].mana[1] = get_sb(n, 0)*2; 
							m += get_sb(n, 0)*2; catalog[c] = n; c++;
						}
						if (it[inc].weapon[0])
						{
							n = 58; it[inr].weapon[0] = get_sb(n, 1); it[inr].weapon[1] = get_sb(n, 0)*2; 
							m += get_sb(n, 0)*2; catalog[c] = n; c++;
						}
						if (it[inc].armor[0])
						{
							n = 59; it[inr].armor[0] = get_sb(n, 1); it[inr].armor[1] = get_sb(n, 0)*2; 
							m += get_sb(n, 0)*2; catalog[c] = n; c++;
						}
						if (it[inc].move_speed[0])
						{
							n = 60; it[inr].move_speed[0] = get_sb(n, 1); it[inr].move_speed[1] = get_sb(n, 0)*2; 
							m += get_sb(n, 0)*2; catalog[c] = n; c++;
						}
						if (it[inc].atk_speed[0])
						{
							n = 61; it[inr].atk_speed[0] = get_sb(n, 1); it[inr].atk_speed[1] = get_sb(n, 0)*2; 
							m += get_sb(n, 0)*2; catalog[c] = n; c++;
						}
						if (it[inc].cast_speed[0])
						{
							n = 62; it[inr].cast_speed[0] = get_sb(n, 1); it[inr].cast_speed[1] = get_sb(n, 0)*2; 
							m += get_sb(n, 0)*2; catalog[c] = n; c++;
						}
						if (it[inc].spell_mod[0])
						{
							n = 63; it[inr].spell_mod[0] = get_sb(n, 1); it[inr].spell_mod[1] = get_sb(n, 0)*2; 
							m += get_sb(n, 0)*2; catalog[c] = n; c++;
						}
						if (it[inc].spell_apt[0])
						{
							n = 64; it[inr].spell_apt[0] = get_sb(n, 1); it[inr].spell_apt[1] = get_sb(n, 0)*2; 
							m += get_sb(n, 0)*2; catalog[c] = n; c++;
						}
						if (it[inc].cool_bonus[0])
						{
							n = 65; it[inr].cool_bonus[0] = get_sb(n, 1); it[inr].cool_bonus[1] = get_sb(n, 0)*2; 
							m += get_sb(n, 0)*2; catalog[c] = n; c++;
						}
						if (it[inc].crit_chance[0])
						{
							n = 66; it[inr].crit_chance[0] = get_sb(n, 1); it[inr].crit_chance[1] = get_sb(n, 0)*2; 
							m += get_sb(n, 0)*2; catalog[c] = n; c++;
						}
						if (it[inc].crit_multi[0])
						{
							n = 67; it[inr].crit_multi[0] = get_sb(n, 1); it[inr].crit_multi[1] = get_sb(n, 0)*2; 
							m += get_sb(n, 0)*2; catalog[c] = n; c++;
						}
						if (it[inc].top_damage[0])
						{
							n = 68; it[inr].top_damage[0] = get_sb(n, 1); it[inr].top_damage[1] = get_sb(n, 0)*2; 
							m += get_sb(n, 0)*2; catalog[c] = n; c++;
						}
						if (it[inc].gethit_dam[0])
						{
							n = 69; it[inr].gethit_dam[0] = get_sb(n, 1); it[inr].gethit_dam[1] = get_sb(n, 0)*2; 
							m += get_sb(n, 0)*2; catalog[c] = n; c++;
						}
						
						panic = v = n = 0;
						
						if (m) while (m > rank && panic < 9)
						{
							v = RANDOM(c);
							n = v==n?RANDOM(c):v;
							v = catalog[n];
							if (v>=50 && v<=54) { m -= it[inr].attrib[v-50][1]; it[inr].attrib[v-50][0] = it[inr].attrib[v-50][1] = 0; }
							else if (v==55)     { m -= it[inc].hp[1];           it[inc].hp[0]           = it[inc].hp[1]           = 0; }
							else if (v==56)     { m -= it[inc].end[1];          it[inc].end[0]          = it[inc].end[1]          = 0; }
							else if (v==57)     { m -= it[inc].mana[1];         it[inc].mana[0]         = it[inc].mana[1]         = 0; }
							else if (v==58)     { m -= it[inc].weapon[1];       it[inc].weapon[0]       = it[inc].weapon[1]       = 0; }
							else if (v==59)     { m -= it[inc].armor[1];        it[inc].armor[0]        = it[inc].armor[1]        = 0; }
							else if (v==60)     { m -= it[inc].move_speed[1];   it[inc].move_speed[0]   = it[inc].move_speed[1]   = 0; }
							else if (v==61)     { m -= it[inc].atk_speed[1];    it[inc].atk_speed[0]    = it[inc].atk_speed[1]    = 0; }
							else if (v==62)     { m -= it[inc].cast_speed[1];   it[inc].cast_speed[0]   = it[inc].cast_speed[1]   = 0; }
							else if (v==63)     { m -= it[inc].spell_mod[1];    it[inc].spell_mod[0]    = it[inc].spell_mod[1]    = 0; }
							else if (v==64)     { m -= it[inc].spell_apt[1];    it[inc].spell_apt[0]    = it[inc].spell_apt[1]    = 0; }
							else if (v==65)     { m -= it[inc].cool_bonus[1];   it[inc].cool_bonus[0]   = it[inc].cool_bonus[1]   = 0; }
							else if (v==66)     { m -= it[inc].crit_chance[1];  it[inc].crit_chance[0]  = it[inc].crit_chance[1]  = 0; }
							else if (v==67)     { m -= it[inc].crit_multi[1];   it[inc].crit_multi[0]   = it[inc].crit_multi[1]   = 0; }
							else if (v==68)     { m -= it[inc].top_damage[1];   it[inc].top_damage[0]   = it[inc].top_damage[1]   = 0; }
							else if (v==69)     { m -= it[inc].gethit_dam[1];   it[inc].gethit_dam[0]   = it[inc].gethit_dam[1]   = 0; }
							else if (v<50)      { m -= it[inc].skill[v][1];     it[inc].skill[v][0]     = it[inc].skill[v][1]     = 0; }
							else				{ panic++; }
							catalog[n] = 99;
						}
						
						it[inr].data[3] = m;
						it[inr].flags |= IF_IDENTIFIED;
					}
					if (!god_give_char(inr, cn))
					{
						do_char_log(cn, 1, "Your backpack is full...\n");
						it[inr].used = USE_EMPTY;
						break;
					}
					it[inc].flags |= IF_SOULSPLIT;
					do_char_log(cn, 1, "You got a %s.\n", it[inr].reference);
					break;
				default:
					do_char_log(cn, 0, "Ease of use.\n");
					if (!inc) inc = RANDOM(2)?get_random_backpack_item(cn):get_random_gear_item(cn);
					if (it[inc].flags & IF_EASEUSE)
					{
						do_char_log(cn, 1, "But it had been done to the %s before.\n", it[inc].reference);
						break;
					}
					for (p=0;p<5;p++)
					{
						if (it[inc].attrib[p][2]) 
							it[inc].attrib[p][2] = it[inc].attrib[p][2] * 3/4;
					}
					for (p=0;p<50;p++)
					{
						if (it[inc].skill[p][2]) 
							it[inc].skill[p][2] = it[inc].skill[p][2] * 3/4;
					}
					if (it[inc].min_rank)
						it[inc].min_rank = it[inc].min_rank * 3/4;
					it[inc].flags |= IF_EASEUSE;
					do_char_log(cn, 1, "Your %s was altered.\n", it[inc].reference);
					break;
			}
			break;
		case  2:		// Blue - Beneficial buffs
			v = 104;
			if (has_buff(cn, v)) v = 105;
			switch (it[in].data[1])
			{
				case  1:
					if (buff = make_new_buff(cn, v, BUF_SPR_ACCEL, 300, SP_DUR_SHRINE, 0))
					{
						strcpy(bu[buff].name, "Shrine of Acceleration");
						bu[buff].speed[1] = 20; 
						bu[buff].data[4] = 1; bu[buff].data[6] = 3; add_spell(cn, buff);
						do_char_log(cn, 3, "Be accelerated.\n");
					}
					break;
				case  2:
					if (buff = make_new_buff(cn, v, BUF_SPR_SHARP, 300, SP_DUR_SHRINE, 0))
					{
						strcpy(bu[buff].name, "Shrine of Sharpness");
						bu[buff].weapon[1] = 20; 
						bu[buff].data[4] = 1; bu[buff].data[6] = 3; add_spell(cn, buff);
						do_char_log(cn, 3, "Be sharpened.\n");
					}
					break;
				case  3:
					if (buff = make_new_buff(cn, v, BUF_SPR_FORTI, 300, SP_DUR_SHRINE, 0))
					{
						strcpy(bu[buff].name, "Shrine of Fortify");
						bu[buff].armor[1] = 20; 
						bu[buff].data[4] = 1; bu[buff].data[6] = 3; add_spell(cn, buff);
						do_char_log(cn, 3, "Be fortified.\n");
					}
					break;
				case  4:
					if (buff = make_new_buff(cn, v, BUF_SPR_BLSSN, 300, SP_DUR_SHRINE, 0))
					{
						strcpy(bu[buff].name, "Shrine of Blessing");
						for (n = 0; n<5; n++) bu[buff].attrib[n][1] = 20;
						bu[buff].data[4] = 1; bu[buff].data[6] = 3; add_spell(cn, buff);
						do_char_log(cn, 3, "Be blessed.\n");
					}
					break;
				default:
					if (buff = make_new_buff(cn, v, BUF_SPR_WISDM, 300, SP_DUR_SHRINE, 0))
					{
						strcpy(bu[buff].name, "Shrine of Wisdom");
						bu[buff].spell_mod[1] = 20;
						bu[buff].data[4] = 1; bu[buff].data[6] = 3; add_spell(cn, buff);
						do_char_log(cn, 3, "Be wisened.\n");
					}
					break;
			}
			char_play_sound(cn, ch[cn].sound + 1, -150, 0);
			do_char_log(cn, 1, "You were strengthened.\n");
			break;
		default:		// Green - Character bonus
			p = sqrt(rank2points(rank)) * 7 + 7;
			p -= RANDOM(p/25+1); p += RANDOM(p/25*(tier+1)+1);
			switch (it[in].data[1])
			{
				case  1:
					p = p/53; ch[cn].luck += p;
					do_char_log(cn, 2, "Your lucky day.\n");
					chlog(cn, "Gets %d Luck", p);
					do_char_log(cn, 1, "You got %d luck.\n", p);
					break;
				case  2:
					p = p*2; ch[cn].gold += p;
					do_char_log(cn, 2, "Your pockets fill.\n");
					chlog(cn, "Gets %dG %dS", p / 100, p % 100);
					do_char_log(cn, 1, "You got %dG %dS.\n", p / 100, p % 100);
					break;
				case  3:
					ch[cn].points += p; ch[cn].points_tot += p;
					do_char_log(cn, 2, "Inspiration.\n");
					chlog(cn, "Gets %d EXP", p);
					do_char_log(cn, 1, "You got %d experience points.\n", p);
					break;
				case  4:
					p = p/47; ch[cn].os_points += p;
					do_char_log(cn, 2, "Osiris smiles.\n");
					chlog(cn, "Gets %d OSP", p);
					do_char_log(cn, 1, "You got %d contract points.\n", p);
					break;
				default:
					p = p/41; ch[cn].bs_points += p;
					do_char_log(cn, 2, "The stronghold falls.\n");
					chlog(cn, "Gets %d BSP", p);
					do_char_log(cn, 1, "You got %d stronghold points.\n", p);
					break;
			}
			break;
	}
	
	// Contract progress
	if (CONT_SCEN(cn)==5) 
	{
		add_map_progress(CONT_NUM(cn));
		if (CONT_PROG(cn)>=CONT_GOAL(cn)) 
			do_char_log(cn, 2, "That's all of them! You're good to go!\n");
		else
			do_char_log(cn, 1, "%d down, %d to go.\n", CONT_PROG(cn), (CONT_GOAL(cn)-CONT_PROG(cn)));
	}
	
	return 1;
}

int use_map_portal(int cn, int in)
{
	int rank, tier, exp, n, luck, bsp, osp;
	
	// Check for mission completion
	if (CONT_PROG(cn) < CONT_GOAL(cn))
	{
		do_char_log(cn, 0, "Your contract isn't finished yet!\n");
		return 0;
	}
	
	//do_char_log(cn, 1, "%d > %d\n", CONT_PROG(cn), CONT_GOAL(cn));
	
	rank = it[in].data[0];
	tier = it[in].data[5]*4/3;
	
	do_char_log(cn, 1, "You completed the area.\n");

	exp = luck = bsp = osp = sqrt(rank2points(rank)) * 7 + 7;
	exp -= RANDOM(exp/20+1);
	exp += RANDOM(exp/20*(tier+1)+1);
	
	if (it[in].data[1]==1) exp += RANDOM(exp/8) + RANDOM(exp/8);
	
	chlog(cn, "Finished map of rank %d tier %d", rank, tier);
	char_play_sound(cn, ch[cn].sound + 19, 0, 0);
	
	chlog(cn, "Gets %d EXP", exp);
	ch[cn].points += exp;
	ch[cn].points_tot += exp;
	
	if (tier || it[in].data[4]==1)
	{
		osp /= 47; 
		osp -= RANDOM(osp/20+1); 
		osp += RANDOM(osp/20*(tier+1)+1);
		
		if (it[in].data[4]==1 && tier) osp += RANDOM(osp/8) + RANDOM(osp/8);
		else if (it[in].data[4]==1) osp = RANDOM(osp/8) + RANDOM(osp/8);
		
		ch[cn].os_points += osp;
		if (!(ch[cn].flags & CF_SYS_OFF))
			do_char_log(cn, 2, "You get %d exp and %d contract pts.\n", exp, osp);
	}
	else
	{
		if (!(ch[cn].flags & CF_SYS_OFF))
			do_char_log(cn, 2, "You get %d experience points.\n", exp);
	}
	
	if (it[in].data[2]==1) // Exit portal grants ex luck
	{
		luck /= 53;	
		luck -= RANDOM(luck/20+1); 
		luck += RANDOM(luck/20*(tier+1)+1);
		ch[cn].luck += luck;
		if (!(ch[cn].flags & CF_SYS_OFF))
			do_char_log(cn, 2, "You get %d additional luck.\n", luck);
	}
	if (it[in].data[3]==1) // Exit portal grants ex BSP
	{
		bsp /= 41; 
		bsp -= RANDOM(bsp/20+1); 
		bsp += RANDOM(bsp/20*(tier+1)+1);
		ch[cn].bs_points += bsp;
		if (!(ch[cn].flags & CF_SYS_OFF))
			do_char_log(cn, 2, "You get %d additional stronghold points.\n", bsp);
	}
	//
	
	fx_add_effect(6, 0, ch[cn].x, ch[cn].y, 0);
	god_transfer_char(cn, OS_WARP_X, OS_WARP_Y);
	char_play_sound(cn, ch[cn].sound + 22, -150, 0);
	fx_add_effect(6, 0, ch[cn].x, ch[cn].y, 0);
	
	clear_map_buffs(cn, 0);
	
	return 1;
}

int use_special_spell(int cn, int in)
{
	int spell, power, ret, co=0;

	spell = it[in].data[0];
	
	if ((co = ch[cn].skill_target1)) ;
	else co = cn;

	if (!do_char_can_see(cn, co))
	{
		do_char_log(cn, 0, "You cannot see your target.\n");
		return 0;
	}

	if (spell==SK_BLOODLET)
	{
		if (!is_facing(cn, co))
		{
			do_char_log(cn, 0, "You must be facing your enemy!\n");
			return 0;
		}
		if (!may_attack_msg(cn, co, 1))
		{
			chlog(cn, "Prevented from attacking %s (%d)", ch[co].name, co);
			return 0;
		}
	}
	else if (!player_or_ghost(cn, co))
	{
		do_char_log(cn, 0, "Changed target of spell from %s to %s.\n", ch[co].name, ch[cn].name);
		co = cn;
	}

	switch(spell)
	{
		case    SK_BLOODLET:
			if (!get_gear(cn, IT_WP_BLOODLET)) 
			{
				do_char_log(cn, 0, "You must equip it first.\n");
				return 0;
			}
			power = ch[cn].hp[5]/4;
			if (ch[cn].a_hp + 500 < power)
			{
				do_char_log(cn, 0, "You dont have enough life.\n");
				return 0;
			}
			ch[cn].a_hp -= power;
			ret = spell_bloodletting(cn, co, power);
			break;
		case    SK_STARLIGHT:
			if (!get_gear(cn, IT_WP_STARLIGHT)) 
			{
				do_char_log(cn, 0, "You must equip it first.\n");
				return 0;
			}
			power = ch[cn].mana[5]/4;
			if (ch[cn].a_mana + 500 < power)
			{
				do_char_log(cn, 0, "You dont have enough mana.\n");
				return 0;
			}
			ch[cn].a_mana -= power;
			item_damage_worn(cn, WN_RHAND, 500);
			if (get_gear(cn, IT_WP_LIFESPRIG) && power) spell_pomesol(cn, cn, power, 1);
			ret = spell_starlight(cn, co, power, 1);
			break;
		case    SK_PHALANX:
			if (!get_gear(cn, IT_WP_PHALANX)) 
			{
				do_char_log(cn, 0, "You must equip it first.\n");
				return 0;
			}
			power = ch[cn].end[5]/2;
			if (ch[cn].a_end + 500 < power)
			{
				do_char_log(cn, 0, "You dont have enough endurance.\n");
				return 0;
			}
			ch[cn].a_end -= power;
			item_damage_worn(cn, WN_LHAND, 500);
			if (get_gear(cn, IT_WP_CRESSUN) && power) spell_pomesol(cn, cn, power, 0);
			ret = spell_phalanx(cn, co, power, 1);
			break;
		default:
			ret = 0;
			break;
	}

	return ret;
}

int use_lame(int cn, int in)
{
	int in2, worn=0;
	
	if (it[in].data[0]<REQ_LAME) 
	{
		do_char_log(cn, 3, "Thou art not worthy.\n");
		return 0;
	}
	
	in2 = god_create_item(IT_WP_EXCALIBUR);
	
	if (in2)
	{
		char buf[300];

		it[in2].data[0] = cn;

		sprintf(buf, "%s Engraved in it are the letters \"%s\".",
		        it[in2].description, ch[cn].name);
		if (strlen(buf)<200)
		{
			strcpy(it[in2].description, buf);
		}
	}
	
	if (ch[cn].worn[WN_RHAND]==in)
	{
		god_take_from_char(in, cn);
		
		ch[cn].worn[WN_RHAND] = in2;

		it[in2].x = 0;
		it[in2].y = 0;
		it[in2].carried = cn;

		do_update_char(cn);
	}
	else
	{
		god_take_from_char(in, cn);
		it[in].used = USE_EMPTY;
		god_give_char(in2, cn);
	}
	
	do_char_log(cn, 1, "My power is yours, worthy one.\n");
}

//

int use_crystal_sub(int cn, int in)
{
	int group, n, cnt, cc, base, pts = 0, m, z, tmp, sbase, miss = 0;
	int chk_one, chk_two, chk_skl, gear, rand = 1, tier, can_offhand = 0;
	int warsorc = 0, my_weapon = 0, has_lring=0, has_rring=0, sss=0;
	int my_armor[5] = { 0, 0, 0, 0, 0 };
	char buf[80];
	static int a_helm[9] = { 94, 76, 71, 66, 61, 56, 51, 39, 27 };
	static int a_cloa[9] = { 95, 77, 72, 67, 62, 57, 52, 40, 28 };
	static int a_body[9] = { 96, 78, 73, 68, 63, 58, 53, 41, 29 };
	static int a_glov[9] = { 97, 79, 74, 69, 64, 59, 54, 42, 30 };
	static int a_boot[9] = { 98, 80, 75, 70, 65, 60, 55, 43, 31 };
	static int c_helm[5] = { 352, 347, 342, 337, 27 };
	static int c_cloa[5] = { 353, 348, 343, 338, 28 };
	static int c_body[5] = { 354, 349, 344, 339, 29 };
	static int c_glov[5] = { 355, 350, 345, 340, 30 };
	static int c_boot[5] = { 356, 351, 346, 341, 31 };
	static int w_dagg[6] = { 693, 572, 541, 532, 523, 284 };
	static int w_staf[6] = { 694, 573, 542, 533, 524, 285 };
	static int w_spea[6] = { 695, 574, 543, 534, 525, 286 };
	static int w_shie[6] = { 696, 575, 544, 535, 526, 287 };
	static int w_swor[6] = { 697, 576, 545, 536, 527, 288 };
	static int w_dual[6] = { 698, 577, 546, 537, 528, 289 };
	static int w_axes[6] = { 699, 578, 547, 538, 529, 290 };
	static int w_twoh[6] = { 700, 579, 548, 539, 530, 291 };
	static int w_grea[6] = { 701, 580, 549, 540, 531, 292 };
	static int w_claw[6] = { 1784, 1783, 1782, 1781, 1780, 1779 };
	static int temps[23] = {
		// Tier 0 & 1
		CT_MERC_M, 		CT_MERC_F, 
		CT_RD_SKELE, 	CT_RD_GHOST,
		CT_TEMP_M, 		CT_TEMP_F, 
		CT_HARA_M, 		CT_HARA_F,
		// Tier 1 & 2
		CT_RD_GROLM,
		// Tier 2 & 3
		CT_WARRIOR_M, 	CT_WARRIOR_F, 
		CT_SORCERER_M, 	CT_SORCERER_F, 
		CT_ARCHTEMP_M, 	CT_ARCHTEMP_F, 
		CT_PUGILIST_M, 	CT_PUGILIST_F, 
		CT_SUMMONER_M, 	CT_SUMMONER_F,
		CT_ARCHHARA_M, 	CT_ARCHHARA_F,
		CT_RD_GARG, 	CT_RD_ICEY
	}; 
	// ^^ These are the templates used by random dungeon ^^
	int baseg[10] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

	group = it[in].data[0]; // 1001, 1002, 1003
	tier = group-1001;
	if (tier < 0) tier = 0;	// Tier 0: lv   0 -  49
							// Tier 1: lv  40 -  89
	if (tier > 2) tier = 2;	// Tier 2: lv  80 - 129

	for (n = 1, cnt = 0; n<MAXCHARS; n++) // check all characters in the game
	{
		if (ch[n].used==USE_ACTIVE && !(ch[n].flags & CF_BODY) && ch[n].data[CHD_GROUP]==group) // for the group on the RD crystal (10)
		{
			base = ch[n].data[0]; // get generic 1 (the base saved on the mob)
			if (base>129)
			{
				base = 129;
			}
			baseg[(base-tier*40)/5]++; // add a count to the number of this mob
			cnt++;
		}
	}

	miss = it[in].data[1] - cnt; // There can only be a total of 50 RD mobs in the game at once
	if (miss<=0)
	{
		return 0;
	}

	for (n = 0, sbase = 0, tmp = 999; n<10; n++)
	{
		if (baseg[n]<tmp)
		{
			tmp = baseg[n];
			sbase = n;
		}
	}
	
	xlog("Randoms: smallest base is %d with %d (miss=%d)", sbase, tmp, miss);
	
	switch (tier)
	{
		case 2: 	tmp = temps[RANDOM(14)+9];  break;
		case 1: 	tmp = temps[RANDOM(8)+2]; 	break;
		default:	tmp = temps[RANDOM(8)]; 	break;
	}

	if (tmp==CT_WARRIOR_M||tmp==CT_WARRIOR_F||tmp==CT_SORCERER_M||tmp==CT_SORCERER_F)
		warsorc=1;
	else
		warsorc=0;
	
	if (!RANDOM(20)) sss = 10;

	cc = god_create_char(tmp, 0);
	if (!god_drop_char_fuzzy(cc, it[in].x, it[in].y))
	{
		xlog("use_crystal (%d,%d): drop_char failed", it[in].x, it[in].y);
		ch[cc].used = USE_EMPTY;
		return 0;
	}

	ch[cc].data[CHD_GROUP] = group;
	
	do
	{
		m = RANDOM(64) + 128 + (RANDOM(64) + 64) * MAPX;
	}
	while (!plr_check_target(m));
	
	ch[cc].goto_x = m % MAPX;
	ch[cc].goto_y = m / MAPX;
	ch[cc].data[60] = 18 * 20;
	ch[cc].data[62] = 1;
	
	if (sss)
	{
		switch (RANDOM(10))
		{
			case  1:	sprintf(buf, "Wise %s",  ch[cc].name); break;
			case  2:	sprintf(buf, "Bold %s",  ch[cc].name); break;
			case  3:	sprintf(buf, "Brave %s", ch[cc].name); break;
			case  4:	sprintf(buf, "Hardy %s", ch[cc].name); break;
			case  5:	sprintf(buf, "Able %s",  ch[cc].name); break;
			case  6:	sprintf(buf, "Stout %s", ch[cc].name); break;
			case  7:	sprintf(buf, "Meek %s",  ch[cc].name); break;
			case  8:	sprintf(buf, "Petty %s", ch[cc].name); break;
			case  9:	sprintf(buf, "Keen %s",  ch[cc].name); break;
			default:	sprintf(buf, "Kind %s",  ch[cc].name); break;
		}
		strncpy(ch[cc].name, buf, 39);
		ch[cc].name[39] = 0;
		
		if (!(ch[cc].flags & CF_EXTRAEXP)) ch[cc].flags |= CF_EXTRAEXP;
	}
	
	switch (RANDOM(4))
	{
		case  1:	strcpy(ch[cc].text[0], "An end to your misery, %s!"); break;
		case  2:	strcpy(ch[cc].text[0], "Took you long enough, %s!"); break;
		case  3:	strcpy(ch[cc].text[0], "It's about time! Goodbye, %s!"); break;
		default:	strcpy(ch[cc].text[0], "Yes! Die, %s!"); break;
	}
	switch (RANDOM(4))
	{
		case  1:	strcpy(ch[cc].text[1], "Something to do! Let's duel to the end, %s!"); break;
		case  2:	strcpy(ch[cc].text[1], "I hope you know what you're doing, %s!"); break;
		case  3:	strcpy(ch[cc].text[1], "A worthy foe! Let us shake the void, %s!"); break;
		default:	strcpy(ch[cc].text[1], "Yahoo! An enemy! Prepare to die, %s!"); break;
	}
	switch (RANDOM(4))
	{
		case  1:	strcpy(ch[cc].text[3], "Alas, I've lost. Good fight, %s!"); break;
		case  2:	strcpy(ch[cc].text[3], "Seems this is the end for me. So long, %s."); break;
		case  3:	strcpy(ch[cc].text[3], "Great fight, %s. I suppose this is goodbye."); break;
		default:	strcpy(ch[cc].text[3], "Thank you %s! Everything is better than being here."); break;
	}
	
	ch[cc].data[48] = 33;

	base = sbase * 5 + RANDOM(5) + tier*40;
	ch[cc].data[0] = base;
	
	for (n = 0; n<5; n++)
	{
		tmp = 5*tier + base + RANDOM(15) + sss;
		if (warsorc && ch[cc].attrib[n][2]==105)	// Special case for warrs and sorcs
		{
			tmp = tmp * 3 / max(1, ch[cc].attrib[n][3]-tier);
		}
		else
		{
			tmp = tmp * 3 / max(1, ch[cc].attrib[n][3]);
		}
		B_AT(cc, n) = max(10, min(ch[cc].attrib[n][2]+sss*tier, tmp));
	}

	for (n = 0; n<MAXSKILL; n++)
	{
		tmp = 5*tier + base + RANDOM(15) + sss;
		if (warsorc && ch[cc].skill[n][2]==105)	// Special case for warrs and sorcs
		{
			tmp = tmp * 3 / max(1, ch[cc].skill[n][3]-tier);
		}
		else
		{
			tmp = tmp * 3 / max(1, ch[cc].skill[n][3]);
		}
		if (ch[cc].skill[n][2])
		{
			B_SK(cc, n) = min(ch[cc].skill[n][2]+5*tier+sss*tier, tmp);
		}
	}

	ch[cc].hp[0]   = max(50, min(ch[cc].hp[2], base * 5 + tier * 125 + RANDOM(50) + RANDOM(50)*tier)) + sss*5*(tier+1);
	ch[cc].end[0]  = 100 + RANDOM(8+base/3)+RANDOM(50)*tier + sss*5*(tier+1);
	ch[cc].mana[0] = max(50, min(ch[cc].mana[2], base * 5 + tier * 125 + RANDOM(50) + RANDOM(50)*tier)) + sss*5*(tier+1);

	// calculate experience
	for (z = 0; z<5; z++) { for (m = 10; m<B_AT(cc, z); m++) { pts += attrib_needed(m, 4); } }
	for (m = 50; m<ch[cc].hp[0]; m++) { pts += hp_needed(m, 4); }
	for (m = 50; m<ch[cc].mana[0]; m++) { pts += mana_needed(m, 4); }
	for (z = 0; z<MAXSKILL; z++) { for (m = 1; m<B_SK(cc, z); m++) { pts += skill_needed(m, 3); } }

	ch[cc].points_tot = pts;
	ch[cc].gold   = (base/2 * (RANDOM(base)+1) + RANDOM(100) + 1);
	ch[cc].gold  += RANDOM(1+ch[cc].gold*sss);
	ch[cc].a_hp   = 999999;
	ch[cc].a_end  = 999999;
	ch[cc].a_mana = 999999;

	ch[cc].alignment = -RANDOM(7500);

	xlog("Created %s (%d) with base %d in Random Dungeon Tier %d", ch[cc].name, cc, base, tier);

	//  -------- Set up RD mob armor -------- //

	chk_skl = B_SK(cc, SK_MSHIELD);
	chk_one = B_AT(cc, AT_WIL);
	chk_two = B_AT(cc, AT_INT);
	
	// Check for magic shield so the mage armor actually benefits the wearer
	if (chk_skl && (chk_one+chk_two)>(B_AT(cc, AT_AGL)+B_AT(cc, AT_STR)))
	{
		gear = 4; // Cloth
		rand = 1;
		
		if			(chk_one >= 90 && chk_two >= 90)	// Wizard
		{	// 90 + 90 = 180 out of 240 ;; 60 threshold
			gear = 0;
			if 			((chk_one+chk_two)<(180+60)) rand = 3;
			else if 	((chk_one+chk_two)<(180+30)) rand = 4;
		}
		else if	(chk_one >= 60 && chk_two >= 60)	// Adept
		{	// 60 + 60 = 120 out of 180 ;; 60 threshold
			gear = 1;
			if 			((chk_one+chk_two)<(120+60)) rand = 2;
			else if 	((chk_one+chk_two)<(120+30)) rand = 3;
		}
		else if	(chk_one >= 35 && chk_two >= 35)	// Caster
		{	// 35 + 35 = 70 out of 120 ;; 50 threshold
			gear = 2;
			if 			((chk_one+chk_two)<(70+50)) rand = 2;
			else if 	((chk_one+chk_two)<(70+25)) rand = 3;
		}
		else if	(chk_one >= 15 && chk_two >= 15)	// Simple
		{	// 15 + 15 = 30 out of 70 ;; 40 threshold
			gear = 3;
			if 			((chk_one+chk_two)<(30+40)) rand = 2;
		}
		
		if (sss) rand--;
		
		for (n = 0; n < 5; n++)
		{
			my_armor[n] = gear+RANDOM(rand);
		}
		
		tmp = ch[cc].worn[WN_HEAD]  = pop_create_item(c_helm[my_armor[0]], cc); it[tmp].carried = cc;
		tmp = ch[cc].worn[WN_CLOAK] = pop_create_item(c_cloa[my_armor[1]], cc); it[tmp].carried = cc;
		tmp = ch[cc].worn[WN_BODY]  = pop_create_item(c_body[my_armor[2]], cc); it[tmp].carried = cc;
		tmp = ch[cc].worn[WN_ARMS]  = pop_create_item(c_glov[my_armor[3]], cc); it[tmp].carried = cc;
		tmp = ch[cc].worn[WN_FEET]  = pop_create_item(c_boot[my_armor[4]], cc); it[tmp].carried = cc;
	}
	else
	{
		chk_one = B_AT(cc, AT_AGL);
		chk_two = B_AT(cc, AT_STR);
		
		gear = 8; // Cloth
		rand = 1;
		
		if			(chk_one >= 105 && chk_two >= 105 && RANDOM(2))	// Adamantine
		{	// 105+105 = 210 out of 240 ;; 30 threshold
			gear = 0;
			rand = 4;
			if 			((chk_one+chk_two)<(210+30)) rand = 5;
			else if 	((chk_one+chk_two)<(210+15)) rand = 6;
		}
		else if	(chk_one >= 84 && chk_two >= 84)	// Titanium
		{	// 84 + 84 = 168 out of 180 ;; 42 threshold
			gear = 1;
			if 			((chk_one+chk_two)<(168+42)) rand = 4;
			else if 	((chk_one+chk_two)<(168+21)) rand = 5;
		}
		else if	(chk_one >= 64 && chk_two >= 64)	// Crystal
		{	// 64 + 64 = 128 out of 168 ;; 40 threshold
			gear = 2;
			if 			((chk_one+chk_two)<(128+40)) rand = 4;
			else if 	((chk_one+chk_two)<(128+20)) rand = 5;
		}
		else if	(chk_one >= 48 && chk_two >= 48)	// Emerald
		{	// 48 + 48 =  96 out of 128 ;; 32 threshold
			gear = 3;
			if 			((chk_one+chk_two)<(96+32)) rand = 3;
			else if 	((chk_one+chk_two)<(96+16)) rand = 4;
		}
		else if	(chk_one >= 34 && chk_two >= 34)	// Gold
		{	// 34 + 34 =  68 out of  96 ;; 28 threshold
			gear = 4;
			if 			((chk_one+chk_two)<(68+28)) rand = 3;
			else if 	((chk_one+chk_two)<(68+14)) rand = 4;
		}
		else if	(chk_one >= 24 && chk_two >= 24)	// Steel
		{	// 24 + 24 =  48 out of  68 ;; 20 threshold
			gear = 5;
			if 			((chk_one+chk_two)<(48+20)) rand = 2;
			else if 	((chk_one+chk_two)<(48+10)) rand = 3;
		}
		else if	(chk_one >= 16 && chk_two >= 16)	// Bronze
		{	// 16 + 16 =  32 out of  48 ;; 16 threshold
			gear = 6;
			if 			((chk_one+chk_two)<(32+16)) rand = 2;
			else if 	((chk_one+chk_two)<(32+ 8)) rand = 3;
		}
		else if	(chk_one >= 12 && chk_two >= 12)	// Leather
		{	// 12 + 12 =  24 out of  32 ;;  8 threshold
			gear = 7;
			if 			((chk_one+chk_two)<(24+ 8)) rand = 2;
		}
		
		if (sss) rand--;
		
		for (n = 0; n < 5; n++)
		{
			my_armor[n] = gear+RANDOM(rand);
		}
		
		tmp = ch[cc].worn[WN_HEAD]  = pop_create_item(a_helm[my_armor[0]], cc); it[tmp].carried = cc;
		tmp = ch[cc].worn[WN_CLOAK] = pop_create_item(a_cloa[my_armor[1]], cc); it[tmp].carried = cc;
		tmp = ch[cc].worn[WN_BODY]  = pop_create_item(a_body[my_armor[2]], cc); it[tmp].carried = cc;
		tmp = ch[cc].worn[WN_ARMS]  = pop_create_item(a_glov[my_armor[3]], cc); it[tmp].carried = cc;
		tmp = ch[cc].worn[WN_FEET]  = pop_create_item(a_boot[my_armor[4]], cc); it[tmp].carried = cc;
	}
	
	//  -------- Set up RD mob weapon -------- //

	gear = 5; // Steel
	rand = 1;
	
	if (!RANDOM(3) && B_SK(cc, SK_HAND) >= B_SK(cc, SK_AXE) &&
		B_SK(cc, SK_HAND) >= B_SK(cc, SK_TWOHAND) && B_SK(cc, SK_HAND) > B_SK(cc, SK_DAGGER) &&
		B_SK(cc, SK_HAND) > B_SK(cc, SK_STAFF) && B_SK(cc, SK_HAND) > B_SK(cc, SK_SWORD))
	{
		//w_claw
		chk_skl = B_SK(cc, SK_HAND);
		chk_one = B_AT(cc, AT_AGL);
		chk_two = B_AT(cc, AT_STR);
		
		if 			(chk_skl >= 96 && chk_one >=115 && chk_two >= 82 && RANDOM(2))	// Adamantine
		{ gear = 0; rand = 3; }
		else if 	(chk_skl >= 80 && chk_one >= 85 && chk_two >= 60)	// Titanium
		{ gear = 1; rand = 2; }
		else if 	(chk_skl >= 64 && chk_one >= 60 && chk_two >= 42)	// Crystal
		{ gear = 2; }
		else if 	(chk_skl >= 48 && chk_one >= 40 && chk_two >= 28)	// Emerald
		{ gear = 3; }
		else if 	(chk_skl >= 32 && chk_one >= 25 && chk_two >= 18)	// Gold
		{ gear = 4; }
		
		if (sss && rand>1) rand--;
		
		my_weapon = gear+RANDOM(rand);
		
		tmp = ch[cc].worn[WN_RHAND] = pop_create_item(w_claw[my_weapon], cc); 
		it[tmp].carried = cc;
	}
	else if (B_SK(cc, SK_AXE) || B_SK(cc, SK_TWOHAND))
	{
		chk_one = B_AT(cc, AT_AGL);
		chk_two = B_AT(cc, AT_STR);
		
		if (B_SK(cc, SK_AXE)+15 >= B_SK(cc, SK_TWOHAND) && B_SK(cc, SK_TWOHAND)+15 >= B_SK(cc, SK_AXE) 
			&& RANDOM(3)==0) // Greataxe
		{
			if (B_SK(cc, SK_AXE)>B_SK(cc, SK_TWOHAND))
				chk_skl = B_SK(cc, SK_TWOHAND);
			else
				chk_skl = B_SK(cc, SK_AXE);
			
			if 			(chk_skl >= 90 && chk_one >= 92 && chk_two >=124 && RANDOM(2))	// Adamantine
			{ gear = 0; rand = 3; }
			else if 	(chk_skl >= 75 && chk_one >= 68 && chk_two >= 90)	// Titanium
			{ gear = 1; rand = 2; }
			else if 	(chk_skl >= 60 && chk_one >= 48 && chk_two >= 62)	// Crystal
			{ gear = 2; }
			else if 	(chk_skl >= 45 && chk_one >= 32 && chk_two >= 40)	// Emerald
			{ gear = 3; }
			else if 	(chk_skl >= 30 && chk_one >= 20 && chk_two >= 24)	// Gold
			{ gear = 4; }
			
			if (sss && rand>1) rand--;
			
			my_weapon = gear+RANDOM(rand);
			
			tmp = ch[cc].worn[WN_RHAND] = pop_create_item(w_grea[my_weapon], cc); 
			it[tmp].carried = cc;
		}
		else if (B_SK(cc, SK_AXE)>B_SK(cc, SK_TWOHAND))		// Axe
		{
			chk_skl = B_SK(cc, SK_AXE);
			
			if 			(chk_skl >=108 && chk_one >= 82 && chk_two >=115 && RANDOM(2))	// Adamantine
			{ gear = 0; rand = 3; }
			else if 	(chk_skl >= 90 && chk_one >= 60 && chk_two >= 85)	// Titanium
			{ gear = 1; rand = 2; }
			else if 	(chk_skl >= 72 && chk_one >= 42 && chk_two >= 60)	// Crystal
			{ gear = 2; }
			else if 	(chk_skl >= 54 && chk_one >= 28 && chk_two >= 40)	// Emerald
			{ gear = 3; }
			else if 	(chk_skl >= 36 && chk_one >= 18 && chk_two >= 25)	// Gold
			{ gear = 4; }
			
			if (sss && rand>1) rand--;
			
			my_weapon = gear+RANDOM(rand);
			
			tmp = ch[cc].worn[WN_RHAND] = pop_create_item(w_axes[my_weapon], cc); 
			it[tmp].carried = cc;
			
			can_offhand = 1;
		}
		else																// Twohander
		{
			chk_skl = B_SK(cc, SK_TWOHAND);
			
			if 			(chk_skl >=108 && chk_one >=105 && chk_two >= 92 && RANDOM(2))	// Adamantine
			{ gear = 0; rand = 3; }
			else if 	(chk_skl >= 90 && chk_one >= 80 && chk_two >= 60)	// Titanium
			{ gear = 1; rand = 2; }
			else if 	(chk_skl >= 72 && chk_one >= 58 && chk_two >= 48)	// Crystal
			{ gear = 2; }
			else if 	(chk_skl >= 54 && chk_one >= 40 && chk_two >= 32)	// Emerald
			{ gear = 3; }
			else if 	(chk_skl >= 36 && chk_one >= 26 && chk_two >= 20)	// Gold
			{ gear = 4; }
			
			if (sss && rand>1) rand--;
			
			my_weapon = gear+RANDOM(rand);
			
			tmp = ch[cc].worn[WN_RHAND] = pop_create_item(w_twoh[my_weapon], cc); 
			it[tmp].carried = cc;
		}
	}
	else if (B_SK(cc, SK_SWORD))									// Sword
	{
		chk_skl = B_SK(cc, SK_SWORD);
		chk_one = B_AT(cc, AT_AGL);
		chk_two = B_AT(cc, AT_STR);
		
		if 			(chk_skl >= 72 && chk_one >= 66 && chk_two >= 52 && RANDOM(2))	// Adamantine
		{ gear = 0; rand = 3; }
		else if 	(chk_skl >= 60 && chk_one >= 52 && chk_two >= 40)	// Titanium
		{ gear = 1; rand = 2; }
		else if 	(chk_skl >= 48 && chk_one >= 40 && chk_two >= 30)	// Crystal
		{ gear = 2; }
		else if 	(chk_skl >= 36 && chk_one >= 30 && chk_two >= 22)	// Emerald
		{ gear = 3; }
		else if 	(chk_skl >= 24 && chk_one >= 22 && chk_two >= 16)	// Gold
		{ gear = 4; }
		
		if (sss && rand>1) rand--;
		
		my_weapon = gear+RANDOM(rand);
		
		tmp = ch[cc].worn[WN_RHAND] = pop_create_item(w_swor[my_weapon], cc); 
		it[tmp].carried = cc;
		
		can_offhand = 1;
	}
	else if (B_SK(cc, SK_DAGGER) || B_SK(cc, SK_STAFF))
	{
		if (B_SK(cc, SK_DAGGER)+8 >= B_SK(cc, SK_STAFF) 
			&& B_SK(cc, SK_STAFF)+8 >= B_SK(cc, SK_DAGGER)
			&& RANDOM(3)==0)		// Spear
		{
			if (B_SK(cc, SK_DAGGER)>B_SK(cc, SK_STAFF))
				chk_skl = B_SK(cc, SK_STAFF);
			else
				chk_skl = B_SK(cc, SK_DAGGER);
			
			chk_one = B_AT(cc, AT_WIL);
			chk_two = B_AT(cc, AT_STR);
			
			if 			(chk_skl >= 48 && chk_one >=105 && chk_two >= 30 && RANDOM(2))	// Adamantine
			{ gear = 0; rand = 3; }
			else if 	(chk_skl >= 40 && chk_one >= 76 && chk_two >= 24)	// Titanium
			{ gear = 1; rand = 2; }
			else if 	(chk_skl >= 32 && chk_one >= 52 && chk_two >= 20)	// Crystal
			{ gear = 2; }
			else if 	(chk_skl >= 24 && chk_one >= 34 && chk_two >= 16)	// Emerald
			{ gear = 3; }
			else if 	(chk_skl >= 16 && chk_one >= 22 && chk_two >= 14)	// Gold
			{ gear = 4; }
			
			if (sss && rand>1) rand--;
			
			my_weapon = gear+RANDOM(rand);
			
			tmp = ch[cc].worn[WN_RHAND] = pop_create_item(w_spea[my_weapon], cc); 
			it[tmp].carried = cc;
		}
		else if (B_SK(cc, SK_DAGGER)>B_SK(cc, SK_STAFF))		// Dagger
		{
			chk_skl = B_SK(cc, SK_DAGGER);
			chk_one = B_AT(cc, AT_WIL);
			chk_two = B_AT(cc, AT_AGL);
			
			if 			(chk_skl >= 60 && chk_one >=102 && chk_two >= 30 && RANDOM(2))	// Adamantine
			{ gear = 0; rand = 3; }
			else if 	(chk_skl >= 50 && chk_one >= 72 && chk_two >= 24)	// Titanium
			{ gear = 1; rand = 2; }
			else if 	(chk_skl >= 40 && chk_one >= 48 && chk_two >= 20)	// Crystal
			{ gear = 2; }
			else if 	(chk_skl >= 30 && chk_one >= 30 && chk_two >= 16)	// Emerald
			{ gear = 3; }
			else if 	(chk_skl >= 20 && chk_one >= 18 && chk_two >= 14)	// Gold
			{ gear = 4; }
			
			if (sss && rand>1) rand--;
			
			my_weapon = gear+RANDOM(rand);
			
			tmp = ch[cc].worn[WN_RHAND] = pop_create_item(w_dagg[my_weapon], cc); 
			it[tmp].carried = cc;
		}
		else																// Staff
		{
			chk_skl = B_SK(cc, SK_STAFF);
			chk_one = B_AT(cc, AT_INT);
			chk_two = B_AT(cc, AT_STR);
			
			if 			(chk_skl >= 60 && chk_one >=120 && chk_two >= 40 && RANDOM(2))	// Adamantine
			{ gear = 0; rand = 3; }
			else if 	(chk_skl >= 50 && chk_one >= 84 && chk_two >= 30)	// Titanium
			{ gear = 1; rand = 2; }
			else if 	(chk_skl >= 40 && chk_one >= 56 && chk_two >= 24)	// Crystal
			{ gear = 2; }
			else if 	(chk_skl >= 30 && chk_one >= 35 && chk_two >= 18)	// Emerald
			{ gear = 3; }
			else if 	(chk_skl >= 20 && chk_one >= 21 && chk_two >= 14)	// Gold
			{ gear = 4; }
			
			if (sss && rand>1) rand--;
			
			my_weapon = gear+RANDOM(rand);
			
			tmp = ch[cc].worn[WN_RHAND] = pop_create_item(w_staf[my_weapon], cc); 
			it[tmp].carried = cc;
		}
	}
	
	if (RANDOM(tier+2) && can_offhand)
	{
		gear = 5; // Steel
		rand = 1;
		
		chk_one = B_AT(cc, AT_AGL);
		chk_two = B_AT(cc, AT_STR);
		
		if (chk_one >= 60 && chk_two >= 60 &&
			(B_SK(cc, SK_DUAL)+30)>B_SK(cc, SK_SHIELD))		// Dual Sword
		{
			chk_skl = B_SK(cc, SK_DUAL);
			
			if 			(chk_skl >= 90 && chk_one >= 96 && chk_two >= 96)	// Adamantine
			{ gear = 0; rand = 3; }
			else if 	(chk_skl >= 75 && chk_one >= 88 && chk_two >= 88)	// Titanium
			{ gear = 1; rand = 2; }
			else if 	(chk_skl >= 60 && chk_one >= 80 && chk_two >= 80)	// Crystal
			{ gear = 2; }
			else if 	(chk_skl >= 45 && chk_one >= 74 && chk_two >= 74)	// Emerald
			{ gear = 3; }
			else if 	(chk_skl >= 30 && chk_one >= 68 && chk_two >= 68)	// Gold
			{ gear = 4; }
			
			if (sss && rand>1) rand--;
			
			my_weapon = gear+RANDOM(rand);
			
			tmp = ch[cc].worn[WN_LHAND] = pop_create_item(w_dual[my_weapon], cc); 
			it[tmp].carried = cc;
		}
		else if (B_SK(cc, SK_SHIELD))							// Shield
		{
			chk_skl = B_SK(cc, SK_SHIELD);
			chk_one = B_AT(cc, AT_BRV);
			
			if 			(chk_skl >= 72 && chk_one >= 82)	// Adamantine
			{ gear = 0; rand = 3; }
			else if 	(chk_skl >= 60 && chk_one >= 60)	// Titanium
			{ gear = 1; rand = 2; }
			else if 	(chk_skl >= 48 && chk_one >= 42)	// Crystal
			{ gear = 2; }
			else if 	(chk_skl >= 36 && chk_one >= 28)	// Emerald
			{ gear = 3; }
			else if 	(chk_skl >= 24 && chk_one >= 18)	// Gold
			{ gear = 4; }
			
			if (sss && rand>1) rand--;
			
			my_weapon = gear+RANDOM(rand);
			
			tmp = ch[cc].worn[WN_LHAND] = pop_create_item(w_shie[my_weapon], cc); 
			it[tmp].carried = cc;
		}
	}
	
	if (sss)
	{
		tmp = RANDOM(43); if (tmp>=21) tmp+=IT_CH_FOOL_R-21; else tmp+=IT_CH_MAGI;
		tmp = pop_create_item(tmp, cc); ch[cc].worn[WN_CHARM] = tmp; it[tmp].carried = cc;
		xlog("  got %s", it[tmp].name);
	}
	
	// New: Rings with 'big' gems.
	if (base >= 105)
	{	if (RANDOM(20-sss)==0 && !ch[cc].worn[WN_LRING])
		{	switch(RANDOM(6))
			{	case  0: tmp = pop_create_item(IT_RD_SILV1+RANDOM(6), cc); break;
				case  1: tmp = pop_create_item(IT_RD_SILV2+RANDOM(4), cc); break;
				case  2: tmp = pop_create_item(IT_RD_GOLD1+RANDOM(6), cc); break;
				case  3: tmp = pop_create_item(IT_RD_GOLD2+RANDOM(4), cc); break;
				case  4: tmp = pop_create_item(IT_RD_PLAT1+RANDOM(6), cc); break;
				default: tmp = pop_create_item(IT_RD_PLAT2+RANDOM(4), cc); break;
			}	if (tmp) { ch[cc].worn[WN_LRING] = tmp; it[tmp].carried = cc; xlog("  got %s", it[tmp].name); }
		}
		if (RANDOM(20-sss)==0 && !ch[cc].worn[WN_RRING])
		{	switch(RANDOM(6))
			{	case  0: tmp = pop_create_item(IT_RD_SILV1+RANDOM(6), cc); break;
				case  1: tmp = pop_create_item(IT_RD_SILV2+RANDOM(4), cc); break;
				case  2: tmp = pop_create_item(IT_RD_GOLD1+RANDOM(6), cc); break;
				case  3: tmp = pop_create_item(IT_RD_GOLD2+RANDOM(4), cc); break;
				case  4: tmp = pop_create_item(IT_RD_PLAT1+RANDOM(6), cc); break;
				default: tmp = pop_create_item(IT_RD_PLAT2+RANDOM(4), cc); break;
			}	if (tmp) { ch[cc].worn[WN_RRING] = tmp; it[tmp].carried = cc; xlog("  got %s", it[tmp].name); }
		}
	}
	if (base >= 65)
	{	if (RANDOM(20-sss)==0 && !ch[cc].worn[WN_LRING])
		{	switch(RANDOM(4))
			{	case  0: tmp = pop_create_item(IT_RD_SILV1+RANDOM(6), cc); break;
				case  1: tmp = pop_create_item(IT_RD_SILV2+RANDOM(4), cc); break;
				case  2: tmp = pop_create_item(IT_RD_GOLD1+RANDOM(6), cc); break;
				default: tmp = pop_create_item(IT_RD_GOLD2+RANDOM(4), cc); break;
			}	if (tmp) { ch[cc].worn[WN_LRING] = tmp; it[tmp].carried = cc; xlog("  got %s", it[tmp].name); }
		}
		if (RANDOM(20-sss)==0 && !ch[cc].worn[WN_RRING])
		{	switch(RANDOM(4))
			{	case  0: tmp = pop_create_item(IT_RD_SILV1+RANDOM(6), cc); break;
				case  1: tmp = pop_create_item(IT_RD_SILV2+RANDOM(4), cc); break;
				case  2: tmp = pop_create_item(IT_RD_GOLD1+RANDOM(6), cc); break;
				default: tmp = pop_create_item(IT_RD_GOLD2+RANDOM(4), cc); break;
			}	if (tmp) { ch[cc].worn[WN_RRING] = tmp; it[tmp].carried = cc; xlog("  got %s", it[tmp].name); }
		}
	}
	if (base >= 25)
	{	if (RANDOM(20-sss)==0 && !ch[cc].worn[WN_LRING])
		{	switch(RANDOM(2))
			{	case  0: tmp = pop_create_item(IT_RD_SILV1+RANDOM(6), cc); break;
				default: tmp = pop_create_item(IT_RD_SILV2+RANDOM(4), cc); break;
			}	if (tmp) { ch[cc].worn[WN_LRING] = tmp; it[tmp].carried = cc; xlog("  got %s", it[tmp].name); }
		}
		if (RANDOM(20-sss)==0 && !ch[cc].worn[WN_RRING])
		{	switch(RANDOM(2))
			{	case  0: tmp = pop_create_item(IT_RD_SILV1+RANDOM(6), cc); break;
				default: tmp = pop_create_item(IT_RD_SILV2+RANDOM(4), cc); break;
			}	if (tmp) { ch[cc].worn[WN_RRING] = tmp; it[tmp].carried = cc; xlog("  got %s", it[tmp].name); }
		}
	}

	if (tier==0) // Tier 0: "base" ranges from   0 -  49
	{	if (RANDOM(20-sss)==0)
		{	switch(RANDOM(3))
			{	case  0: god_give_char(tmp = god_create_item(IT_RD_HEAL), cc); break;
				case  1: god_give_char(tmp = god_create_item(IT_RD_MANA), cc); break;
				default: god_give_char(tmp = god_create_item(IT_RD_END), cc); break;
			}	xlog("  got %s", it[tmp].name);
		}
		else if (RANDOM(40-sss)==0 && base>(0+8))
		{	switch(RANDOM(3))
			{	case  0: god_give_char(tmp = god_create_item(IT_RD_GHEAL), cc); break;
				case  1: god_give_char(tmp = god_create_item(IT_RD_GMANA), cc); break;
				default: god_give_char(tmp = god_create_item(IT_RD_GEND), cc); break;
			}	xlog("  got %s", it[tmp].name);
		}
		if (RANDOM(60-sss)==0 && base>(0+16))
		{	switch(RANDOM(2))
			{	case  0: tmp = ch[cc].worn[WN_NECK] = pop_create_item(IT_RD_AMMYONE, cc); it[tmp].carried = cc; break;
				default: tmp = ch[cc].worn[WN_BELT] = pop_create_item(IT_RD_BELTONE, cc); it[tmp].carried = cc; break;
			}	xlog("  got %s", it[tmp].name);
		}
		if (RANDOM(120-sss)==0 && base>(0+24))
		{	switch(RANDOM(2))
			{	case  0: god_give_char(tmp = god_create_item(IT_RD_HP), cc); break;
				default: god_give_char(tmp = god_create_item(IT_RD_MP), cc); break;
			}	xlog("  got %s", it[tmp].name);
		}
	}
	else if (tier==1) // Tier 1: "base" ranges from  40 -  89
	{	if (RANDOM(25-sss)==0)
		{	switch(RANDOM(6))
			{	case  0: god_give_char(tmp = god_create_item(IT_RD_HEAL), cc); break;
				case  1: god_give_char(tmp = god_create_item(IT_RD_MANA), cc); break;
				case  2: god_give_char(tmp = god_create_item(IT_RD_END), cc); break;
				case  3: god_give_char(tmp = god_create_item(IT_RD_GHEAL), cc); break;
				case  4: god_give_char(tmp = god_create_item(IT_RD_GMANA), cc); break;
				default: god_give_char(tmp = god_create_item(IT_RD_GEND), cc); break;
			}	xlog("  got %s", it[tmp].name);
		}
		else if (RANDOM(50-sss)==0 && base>(40+6))
		{	switch(RANDOM(12))
			{	case  0: god_give_char(tmp = god_create_item(IT_POT_H_HP), cc); break;
				case  1: god_give_char(tmp = god_create_item(IT_POT_S_HP), cc); break;
				case  2: god_give_char(tmp = god_create_item(IT_POT_C_HP), cc); break;
				case  3: god_give_char(tmp = god_create_item(IT_POT_L_HP), cc); break;
				case  4: god_give_char(tmp = god_create_item(IT_POT_H_EN), cc); break;
				case  5: god_give_char(tmp = god_create_item(IT_POT_S_EN), cc); break;
				case  6: god_give_char(tmp = god_create_item(IT_POT_C_EN), cc); break;
				case  7: god_give_char(tmp = god_create_item(IT_POT_L_EN), cc); break;
				case  8: god_give_char(tmp = god_create_item(IT_POT_H_MP), cc); break;
				case  9: god_give_char(tmp = god_create_item(IT_POT_S_MP), cc); break;
				case 10: god_give_char(tmp = god_create_item(IT_POT_C_MP), cc); break;
				default: god_give_char(tmp = god_create_item(IT_POT_L_MP), cc); break;
			}	xlog("  got %s", it[tmp].name);
		}
		if (RANDOM(50-sss)==0 && base>(40+12))
		{	switch(RANDOM(2))
			{	case  0: tmp = ch[cc].worn[WN_NECK] = pop_create_item(IT_RD_AMMYONE, cc); it[tmp].carried = cc; break;
				default: tmp = ch[cc].worn[WN_BELT] = pop_create_item(IT_RD_BELTONE, cc); it[tmp].carried = cc; break;
			}	xlog("  got %s", it[tmp].name);
		}
		else if (RANDOM(75-sss)==0 && base>(40+18))
		{	switch(RANDOM(2))
			{	case  0: tmp = ch[cc].worn[WN_NECK] = pop_create_item(IT_RD_AMMYTWO, cc); it[tmp].carried = cc; break;
				default: tmp = ch[cc].worn[WN_BELT] = pop_create_item(IT_RD_BELTTWO, cc); it[tmp].carried = cc; break;
			}	xlog("  got %s", it[tmp].name);
		}
		if (RANDOM(100-sss)==0 && base>(40+24))
		{	switch(RANDOM(2))
			{	case  0: god_give_char(tmp = god_create_item(IT_RD_HP), cc); break;
				default: god_give_char(tmp = god_create_item(IT_RD_MP), cc); break;
			}	xlog("  got %s", it[tmp].name);
		}
		else if (RANDOM(150-sss)==0 && base>(40+30))
		{	switch(RANDOM(5))
			{	case  0: god_give_char(tmp = god_create_item(IT_RD_BRV), cc); break;
				case  1: god_give_char(tmp = god_create_item(IT_RD_WIL), cc); break;
				case  2: god_give_char(tmp = god_create_item(IT_RD_INT), cc); break;
				case  3: god_give_char(tmp = god_create_item(IT_RD_AGL), cc); break;
				default: god_give_char(tmp = god_create_item(IT_RD_STR), cc); break;
			}	xlog("  got %s", it[tmp].name);
		}
	}
	else if (tier==2)	// Tier 2: "base" ranges from  80 - 129
	{	if (RANDOM(20-sss)==0)
		{	switch(RANDOM(15))
			{	case  0: god_give_char(tmp = god_create_item(IT_RD_GHEAL), cc); break;
				case  1: god_give_char(tmp = god_create_item(IT_RD_GMANA), cc); break;
				case  2: god_give_char(tmp = god_create_item(IT_RD_GEND), cc); break;
				case  3: god_give_char(tmp = god_create_item(IT_POT_H_HP), cc); break;
				case  4: god_give_char(tmp = god_create_item(IT_POT_S_HP), cc); break;
				case  5: god_give_char(tmp = god_create_item(IT_POT_C_HP), cc); break;
				case  6: god_give_char(tmp = god_create_item(IT_POT_L_HP), cc); break;
				case  7: god_give_char(tmp = god_create_item(IT_POT_H_EN), cc); break;
				case  8: god_give_char(tmp = god_create_item(IT_POT_S_EN), cc); break;
				case  9: god_give_char(tmp = god_create_item(IT_POT_C_EN), cc); break;
				case 10: god_give_char(tmp = god_create_item(IT_POT_L_EN), cc); break;
				case 11: god_give_char(tmp = god_create_item(IT_POT_H_MP), cc); break;
				case 12: god_give_char(tmp = god_create_item(IT_POT_S_MP), cc); break;
				case 13: god_give_char(tmp = god_create_item(IT_POT_C_MP), cc); break;
				default: god_give_char(tmp = god_create_item(IT_POT_L_MP), cc); break;
			}	xlog("  got %s", it[tmp].name);
		}
		else if (RANDOM(100-sss)==0 && base>(80+4))
		{	god_give_char(tmp = god_create_item(1985+RANDOM(15)), cc);
			xlog("  got %s", it[tmp].name);
		}
		if (RANDOM(40-sss)==0 && base>(80+8))
		{	switch(RANDOM(2))
			{	case  0: tmp = ch[cc].worn[WN_NECK] = pop_create_item(IT_RD_AMMYONE, cc); it[tmp].carried = cc; break;
				default: tmp = ch[cc].worn[WN_BELT] = pop_create_item(IT_RD_BELTONE, cc); it[tmp].carried = cc; break;
			}	xlog("  got %s", it[tmp].name);
		}
		else if (RANDOM(60-sss)==0 && base>(80+12))
		{	switch(RANDOM(2))
			{	case  0: tmp = ch[cc].worn[WN_NECK] = pop_create_item(IT_RD_AMMYTWO, cc); it[tmp].carried = cc; break;
				default: tmp = ch[cc].worn[WN_BELT] = pop_create_item(IT_RD_BELTTWO, cc); it[tmp].carried = cc; break;
			}	xlog("  got %s", it[tmp].name);
		}
		else if (RANDOM(80-sss)==0 && base>(80+16))
		{	switch(RANDOM(2))
			{	case  0: tmp = ch[cc].worn[WN_NECK] = pop_create_item(IT_RD_AMMYTHR, cc); it[tmp].carried = cc; break;
				default: tmp = ch[cc].worn[WN_BELT] = pop_create_item(IT_RD_BELTTHR, cc); it[tmp].carried = cc; break;
			}	xlog("  got %s", it[tmp].name);
		}
		else if (RANDOM(100-sss)==0 && base>(80+24))
		{	switch(RANDOM(2))
			{	case  0: tmp = ch[cc].worn[WN_NECK] = pop_create_item(IT_RD_AMMYFOU, cc); it[tmp].carried = cc; break;
				default: tmp = ch[cc].worn[WN_BELT] = pop_create_item(IT_RD_BELTFOU, cc); it[tmp].carried = cc; break;
			}	xlog("  got %s", it[tmp].name);
		}
		if (RANDOM(120-sss)==0 && base>(80+20))
		{	switch(RANDOM(5))
			{	case  0: god_give_char(tmp = god_create_item(IT_RD_BRV), cc); break;
				case  1: god_give_char(tmp = god_create_item(IT_RD_WIL), cc); break;
				case  2: god_give_char(tmp = god_create_item(IT_RD_INT), cc); break;
				case  3: god_give_char(tmp = god_create_item(IT_RD_AGL), cc); break;
				default: god_give_char(tmp = god_create_item(IT_RD_STR), cc); break;
			}	xlog("  got %s", it[tmp].name);
		}
	}

	do_update_char(cc);

	return(miss);
}

int use_crystal(int cn, int in)
{
	int cnt = 0;

	while (use_crystal_sub(cn, in)>4 && cnt<4)
	{
		cnt++;
	}

	if (!cnt)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

int use_mine_respawn(int cn, int in)
{
	int group, n, cnt, cc, m, tmp, in2, maxcnt;

	group = it[in].data[0];
	tmp = it[in].data[1];
	maxcnt = it[in].data[2];

	for (n = 3; n<10; n++)
	{
		if ((m = it[in].data[n])==0)
		{
			break;
		}
		if ((in2 = map[m].it)==0)
		{
			return 0;
		}
		if (it[in2].driver!=26)
		{
			return 0;
		}
	}

	for (n = 1, cnt = 0; n<MAXCHARS; n++)
	{
		if (ch[n].used==USE_ACTIVE && !(ch[n].flags & CF_BODY) && ch[n].data[CHD_GROUP]==group)
		{
			cnt++;
		}
	}

	if (cnt>maxcnt)
	{
		return 0;
	}

	cc = pop_create_char(tmp, 0);
	if (!god_drop_char_fuzzy(cc, it[in].x, it[in].y))
	{
		xlog("mine_respawn (%d,%d): drop_char failed", it[in].x, it[in].y);
		ch[cc].used = USE_EMPTY;
		return 0;
	}

	do_update_char(cc);

	return 1;
}

int rat_eye(int cn, int in)
{
	int in2, n;

	if (cn==0)
	{
		return 0;
	}

	if ((in2 = ch[cn].citem)==0 || (in2 & 0x80000000))
	{
		do_char_log(cn, 0, "What do you want to do with it?\n");
		return 0;
	}

	if (it[in].carried==0)
	{
		do_char_log(cn, 0, "Too difficult to do on the ground.\n");
		return 0;
	}

	for (n = 0; n<9; n++)
	{
		if (it[in].data[n] && it[in].data[n]==it[in2].temp)
		{
			break;
		}
	}
	if (n==9)
	{
		do_char_log(cn, 0, "This doesn't fit.\n");
		return 0;
	}

	chlog(cn, "Added %s to %s", it[in2].name, it[in].name);

	it[in].data[n] = 0;
	it[in].sprite[0]++;
	it[in].flags |= IF_UPDATE;
	it[in].temp   = 0;

	it[in2].used = USE_EMPTY;
	ch[cn].citem = 0;

	for (n = 0; n<9; n++)
	{
		if (it[in].data[n])
		{
			break;
		}
	}
	if (n==9)
	{
		int in3;

		in3 = it[in].data[9];

		in3 = god_create_item(in3);
		it[in3].flags |= IF_UPDATE;

		god_take_from_char(in, cn);
		it[in].used = USE_EMPTY;

		god_give_char(in3, cn);
	}

	return 1;
}

int skua_protect(int cn, int in)
{
	if (ch[cn].worn[WN_RHAND]!=in)
	{
		do_char_log(cn, 0, "You cannot use Skua's weapon if you're not wielding it.\n");
		return 0;
	}

	if (IS_PURPLE(cn))
	{
		do_char_log(cn, 0, "How dare you to call on Skua to help you? Slave of the Purple One!\n");
		do_char_log(cn, 0, "Your weapon vanished.\n");
		ch[cn].worn[WN_RHAND] = 0;
		it[in].used = USE_EMPTY;
	}
	else
	{
		do_char_log(cn, 0, "You feel Skua's presence protect you.\n");
		do_char_log(cn, 0, "He takes away His weapon and replaces it by a common one.\n");
		spell_from_item(cn, in);
		it[in].used = USE_EMPTY;
		in = god_create_item(it[in].data[2]);
		it[in].carried = cn;
		ch[cn].worn[WN_RHAND] = in;
		it[in].flags |= IF_UPDATE;
	}

	return 1;
}

int purple_protect(int cn, int in)
{
	if (ch[cn].worn[WN_RHAND]!=in)
	{
		do_char_log(cn, 0, "You cannot use the Purple One's weapon if you're not wielding it.\n");
		return 0;
	}

	if (!IS_PURPLE(cn))
	{
		do_char_log(cn, 0, "How dare you to call on the Purple One to help you? Slave of Skua!\n");
		do_char_log(cn, 0, "Your weapon vanished.\n");
		ch[cn].worn[WN_RHAND] = 0;
		it[in].used = USE_EMPTY;
	}
	else
	{
		do_char_log(cn, 0, "You feel the Purple One's presence protect you.\n");
		do_char_log(cn, 0, "He takes away His weapon and replaces it by a common one.\n");
		spell_from_item(cn, in);
		it[in].used = USE_EMPTY;
		in = god_create_item(it[in].data[2]);
		it[in].carried = cn;
		ch[cn].worn[WN_RHAND] = in;
		it[in].flags |= IF_UPDATE;
	}

	return 1;
}

int use_lever(int cn, int in)
{
	int in2, m;

	m = it[in].data[0];
	in2 = map[m].it;

	if (!in2)
	{
		return 0;
	}
	if (it[in2].active)
	{
		return 0;
	}

	do_area_sound(0, 0, it[in].x, it[in].y, 10);
	use_driver(0, in2, 0);
	it[in2].active = it[in2].duration;
	if (it[in2].light[0]!=it[in2].light[1])
	{
		do_add_light(it[in2].x, it[in2].y, it[in2].light[1] - it[in2].light[0]);
	}

	return 1;
}

int use_spawn(int cn, int in)
{
	int in2, temp;

	if (it[in].active)
	{
		return 0;
	}

	if (cn && it[in].data[1])
	{
		if (!(in2 = ch[cn].citem) || (in2 & 0x80000000))
		{
			return 0;
		}
		if (it[in2].temp!=it[in].data[1])
		{
			return 0;
		}
		it[in2].used = USE_EMPTY;
		ch[cn].citem = 0;
	}

	if ((temp = it[in].data[2])!=0)
	{
		fx_add_effect(2, TICKS * 10, ch_temp[temp].x, ch_temp[temp].y, temp); // 10 seconds
	}

	return 1;
}

int use_pile(int cn, int in)
{
	int in2, x, y, m, level, tmp, chance;
	static int find_S[] = { 
		// 25% - Silver
		IT_SILV, IT_SILV, IT_SILV, IT_SILV, IT_SILV, 
		// 10% ea - Small Gem
		IT_S_SA, IT_S_SA,    IT_S_RU, IT_S_RU,    IT_S_AM, IT_S_AM,
		IT_S_TO, IT_S_TO,    IT_S_EM, IT_S_EM,    IT_S_DI, IT_S_DI,
		// 15% - Skeleton
		IT_E_WK, IT_E_WK, IT_E_WK, 
		0
	};
	static int find_M[] = { 
		// 14% - Silver
		IT_SILV, IT_SILV, IT_SILV, IT_SILV, IT_SILV, IT_SILV, IT_SILV, IT_SILV, IT_SILV, IT_SILV, 
		IT_SILV, IT_SILV, IT_SILV, IT_SILV,
		//  7% ea - Small Gem
		IT_S_SA, IT_S_SA, IT_S_SA, IT_S_SA, IT_S_SA, IT_S_SA, IT_S_SA, 
		IT_S_RU, IT_S_RU, IT_S_RU, IT_S_RU, IT_S_RU, IT_S_RU, IT_S_RU, 
		IT_S_AM, IT_S_AM, IT_S_AM, IT_S_AM, IT_S_AM, IT_S_AM, IT_S_AM, 
		IT_S_TO, IT_S_TO, IT_S_TO, IT_S_TO, IT_S_TO, IT_S_TO, IT_S_TO, 
		IT_S_EM, IT_S_EM, IT_S_EM, IT_S_EM, IT_S_EM, IT_S_EM, IT_S_EM, 
		IT_S_DI, IT_S_DI, IT_S_DI, IT_S_DI, IT_S_DI, IT_S_DI, IT_S_DI, 
		//  4% ea - Medium Gem
		IT_M_SA, IT_M_SA, IT_M_SA, IT_M_SA,    IT_M_RU, IT_M_RU, IT_M_RU, IT_M_RU, 
		IT_M_AM, IT_M_AM, IT_M_AM, IT_M_AM,    IT_M_TO, IT_M_TO, IT_M_TO, IT_M_TO, 
		IT_M_EM, IT_M_EM, IT_M_EM, IT_M_EM,    IT_M_DI, IT_M_DI, IT_M_DI, IT_M_DI, 
		// 20% - Skeleton
		IT_E_SK, IT_E_SK, IT_E_SK, IT_E_SK, IT_E_SK, IT_E_SK, IT_E_SK, IT_E_SK, IT_E_SK, IT_E_SK, 
		IT_E_SK, IT_E_SK, IT_E_SK, IT_E_SK, IT_E_SK, IT_E_SK, IT_E_SK, IT_E_SK, IT_E_SK, IT_E_SK, 
		0
	};
	static int find_B[] = { 
		// 14% - Silver
		IT_SILV, IT_SILV, IT_SILV, IT_SILV, IT_SILV, IT_SILV, IT_SILV, IT_SILV, IT_SILV, IT_SILV, 
		IT_SILV, IT_SILV, IT_SILV, IT_SILV, 
		//  6% - Gold
		IT_GOLD, IT_GOLD, IT_GOLD, IT_GOLD, IT_GOLD, IT_GOLD, 
		//  5% ea - Medium Gem
		IT_M_SA, IT_M_SA, IT_M_SA, IT_M_SA, IT_M_SA, 
		IT_M_RU, IT_M_RU, IT_M_RU, IT_M_RU, IT_M_RU, 
		IT_M_AM, IT_M_AM, IT_M_AM, IT_M_AM, IT_M_AM, 
		IT_M_TO, IT_M_TO, IT_M_TO, IT_M_TO, IT_M_TO, 
		IT_M_EM, IT_M_EM, IT_M_EM, IT_M_EM, IT_M_EM, 
		IT_M_DI, IT_M_DI, IT_M_DI, IT_M_DI, IT_M_DI, 
		//  3% ea - Big Gem
		IT_B_SA, IT_B_SA, IT_B_SA,    IT_B_RU, IT_B_RU, IT_B_RU, 
		IT_B_AM, IT_B_AM, IT_B_AM,    IT_B_TO, IT_B_TO, IT_B_TO, 
		IT_B_EM, IT_B_EM, IT_B_EM,    IT_B_DI, IT_B_DI, IT_B_DI, 
		IT_B_SP, IT_B_SP, IT_B_SP,    IT_B_CI, IT_B_CI, IT_B_CI, 
		IT_B_OP, IT_B_OP, IT_B_OP,    IT_B_AQ, IT_B_AQ, IT_B_AQ, 
		// 20% - Golem
		IT_E_GO, IT_E_GO, IT_E_GO, IT_E_GO, IT_E_GO, IT_E_GO, IT_E_GO, IT_E_GO, IT_E_GO, IT_E_GO, 
		IT_E_GO, IT_E_GO, IT_E_GO, IT_E_GO, IT_E_GO, IT_E_GO, IT_E_GO, IT_E_GO, IT_E_GO, IT_E_GO, 
		0
	};
	static int find_H[] = { 
		// 11% - Gold
		IT_GOLD, IT_GOLD, IT_GOLD, IT_GOLD, IT_GOLD, IT_GOLD, IT_GOLD, IT_GOLD, IT_GOLD, IT_GOLD, 
		IT_GOLD, 
		//  4% ea - Big Gem
		IT_B_SA, IT_B_SA, IT_B_SA, IT_B_SA,    IT_B_RU, IT_B_RU, IT_B_RU, IT_B_RU, 
		IT_B_AM, IT_B_AM, IT_B_AM, IT_B_AM,    IT_B_TO, IT_B_TO, IT_B_TO, IT_B_TO, 
		IT_B_EM, IT_B_EM, IT_B_EM, IT_B_EM,    IT_B_DI, IT_B_DI, IT_B_DI, IT_B_DI, 
		IT_B_SP, IT_B_SP, IT_B_SP, IT_B_SP,    IT_B_CI, IT_B_CI, IT_B_CI, IT_B_CI, 
		IT_B_OP, IT_B_OP, IT_B_OP, IT_B_OP,    IT_B_AQ, IT_B_AQ, IT_B_AQ, IT_B_AQ, 
		//  2% ea - Huge Gem
		IT_H_SA, IT_H_SA,    IT_H_RU, IT_H_RU, 
		IT_H_AM, IT_H_AM,    IT_H_TO, IT_H_TO, 
		IT_H_EM, IT_H_EM,    IT_H_DI, IT_H_DI, 
		IT_H_SP, IT_H_SP,    IT_H_CI, IT_H_CI, 
		IT_H_OP, IT_H_OP,    IT_H_AQ, IT_H_AQ, 
		IT_H_BE, IT_H_BE,    IT_H_ZI, IT_H_ZI, 
		// 25% - Gargoyle
		IT_E_GA, IT_E_GA, IT_E_GA, IT_E_GA, IT_E_GA, IT_E_GA, IT_E_GA, IT_E_GA, IT_E_GA, IT_E_GA, 
		IT_E_GA, IT_E_GA, IT_E_GA, IT_E_GA, IT_E_GA, IT_E_GA, IT_E_GA, IT_E_GA, IT_E_GA, IT_E_GA, 
		IT_E_GA, IT_E_GA, IT_E_GA, IT_E_GA, IT_E_GA, 
		0
	};
	static int find_F[] = { 
		//  9% - Gold
		IT_GOLD, IT_GOLD, IT_GOLD, IT_GOLD, IT_GOLD, IT_GOLD, IT_GOLD, IT_GOLD, IT_GOLD, 
		//  6% - Platinum
		IT_PLAT, IT_PLAT, IT_PLAT, IT_PLAT, IT_PLAT, IT_PLAT, 
		//  3% ea - Huge Gem
		IT_H_SA, IT_H_SA, IT_H_SA,    IT_H_RU, IT_H_RU, IT_H_RU, 
		IT_H_AM, IT_H_AM, IT_H_AM,    IT_H_TO, IT_H_TO, IT_H_TO, 
		IT_H_EM, IT_H_EM, IT_H_EM,    IT_H_DI, IT_H_DI, IT_H_DI, 
		IT_H_SP, IT_H_SP, IT_H_SP,    IT_H_CI, IT_H_CI, IT_H_CI, 
		IT_H_OP, IT_H_OP, IT_H_OP,    IT_H_AQ, IT_H_AQ, IT_H_AQ, 
		IT_H_BE, IT_H_BE, IT_H_BE,    IT_H_ZI, IT_H_ZI, IT_H_ZI, 
		//  2% ea - Flawless Gem
		IT_F_SA, IT_F_SA,    IT_F_RU, IT_F_RU, 
		IT_F_AM, IT_F_AM,    IT_F_TO, IT_F_TO, 
		IT_F_EM, IT_F_EM,    IT_F_DI, IT_F_DI, 
		IT_F_SP, IT_F_SP,    IT_F_CI, IT_F_CI, 
		IT_F_OP, IT_F_OP,    IT_F_AQ, IT_F_AQ, 
		IT_F_BE, IT_F_BE,    IT_F_ZI, IT_F_ZI, 
		// 25% - Golem
		IT_E_MA, IT_E_MA, IT_E_MA, IT_E_MA, IT_E_MA, IT_E_MA, IT_E_MA, IT_E_MA, IT_E_MA, IT_E_MA, 
		IT_E_MA, IT_E_MA, IT_E_MA, IT_E_MA, IT_E_MA, IT_E_MA, IT_E_MA, IT_E_MA, IT_E_MA, IT_E_MA, 
		IT_E_MA, IT_E_MA, IT_E_MA, IT_E_MA, IT_E_MA, 
		0
	};

	if (it[in].active)
	{
		return 0;
	}

	// destroy this object
	it[in].used = USE_EMPTY;
	x = it[in].x;
	y = it[in].y;
	m = x + y * MAPX;
	level = it[in].data[0];
	map[m].it = 0;

	// decide what the player's gonna find
	chance = 6;
	if (ch[cn].luck<0)			chance++;
	if (ch[cn].luck<=-500)		chance++;
	if (ch[cn].luck<=-1000)		chance++;
	if (ch[cn].luck<=-2000)		chance++;
	if (ch[cn].luck<=-4000)		chance++;
	if (ch[cn].luck<=-8000)		chance++;
	
	if (it[in].data[4]>0) chance-=it[in].data[4];
	if (chance<2) chance=2;

	if (!RANDOM(chance))   // something there?
	{
		switch (level)
		{
			case 4: 	tmp = RANDOM(ARRAYSIZE(find_F)-1); tmp = find_F[tmp]; break;
			case 3: 	tmp = RANDOM(ARRAYSIZE(find_H)-1); tmp = find_H[tmp]; break;
			case 2: 	tmp = RANDOM(ARRAYSIZE(find_B)-1); tmp = find_B[tmp]; break;
			case 1: 	tmp = RANDOM(ARRAYSIZE(find_M)-1); tmp = find_M[tmp]; break;
			default: 	tmp = RANDOM(ARRAYSIZE(find_S)-1); tmp = find_S[tmp]; break;
		}

		// create it and give it to player (unless it's the monster, then have it attack)
		in2 = god_create_item(tmp);
		if (it[in2].flags & IF_TAKE)      // takeable object?
		{
			god_give_char(in2, cn);
			do_char_log(cn, 0, "You've found a %s!\n", it[in2].reference);
		}
		else     // no? then it's an object which creates a monster
		{
			god_drop_item(in2, x, y);
			fx_add_effect(9, 16, in2, it[in2].data[0], 0);
		}
	}
	return 1;
}

int use_grave(int cn, int in)
{
	int cc;

	// get previously spawned character
	cc = it[in].data[0];
	// still alive? then don't spawn new one
	if (ch[cc].data[0]==in && !(ch[cc].flags & CF_BODY) && ch[cc].used)
	{
		return 1;
	}

	cc = pop_create_char(CT_UNDEAD, 0);
	if (!god_drop_char_fuzzy(cc, it[in].x, it[in].y))
	{
		god_destroy_items(cc);
		ch[cc].used = USE_EMPTY;
		return 1;
	}

	// create link between item and character
	ch[cc].data[0] = in;
	it[in].data[0] = cc;

	return 1;
}

int mine_wall(int in, int x, int y, int dmg)
{
	int temp, carried, tmp;

	if (!in)
	{
		in = map[x + y * MAPX].it;
	}
	if (!in)
	{
		return 0;
	}

	if (it[in].data[3]) // add rebuild wall effect
	{
		fx_add_effect(10, TICKS * 60 * 15, it[in].x, it[in].y, it[in].temp);
	}

	temp = it[in].data[0];
	x = it[in].x;
	y = it[in].y;
	carried = it[in].carried;

	it[in] = it_temp[temp];
	it[in].x = x;
	it[in].y = y;
	it[in].carried = carried;
	it[in].temp = temp;
	if (carried)
	{
		it[in].flags |= IF_UPDATE;
	}
	
	// Add leftover damage from previous state
	if (it[in].driver==25)
	{
		tmp = it[in].data[1] - dmg;
		if (tmp<=0)
		{
			tmp = mine_wall(in, 0, 0, abs(tmp)) + 1;
		}
		else
		{
			it[in].data[1] = tmp;
		}
	}

	return(it[in].data[2]);
}

int mine_state(int x, int y)
{
	int in;

	in = map[x + y * MAPX].it;
	if (!in)
	{
		return 0;
	}
	if (it[in].driver!=25)
	{
		return 0;
	}

	return(it[in].data[2]);
}

int use_mine(int cn, int in)
{
	int tmp, in2, str;

	str = M_AT(cn, AT_STR);

	// substract endurance
	if (ch[cn].a_end<1500)
	{
		do_char_log(cn, 0, "You're too exhausted to continue digging.\n");
		ch[cn].misc_action = DR_IDLE;
		return 0;
	}

	ch[cn].a_end -= 1000;

	// check for proper tools!
	if ((in2 = ch[cn].worn[WN_RHAND])!=0)
	{
		if (it[in2].temp==IT_PICKAXE)
		{
			item_damage_weapon(cn, str / 10);
			str *= 2;
		}
		else
		{
			item_damage_weapon(cn, str * 10);
			str /= 4;
		}
		char_play_sound(cn, 11, -150, 0);
		do_area_sound(cn, 0, ch[cn].x, ch[cn].y, 11);
	}
	else
	{
		str /= 8;
		if (ch[cn].a_hp<10000)
		{
			do_char_log(cn, 0, "You don't want to kill yourself beating at this wall with your bare hands, so you stop.\n");
			ch[cn].misc_action = DR_IDLE;
			return 0;
		}
		ch[cn].a_hp -= 1000;
	}
	
	// Damage wall
	tmp = it[in].data[1] - str;
	if (tmp<=0)
	{
		reset_go(it[in].x, it[in].y); remove_lights(it[in].x, it[in].y);
		tmp = mine_wall(in, 0, 0, abs(tmp)) + 1;
		reset_go(it[in].x, it[in].y); add_lights(it[in].x, it[in].y);
	}
	else
	{
		it[in].data[1] = tmp;
	}

	return 0;
}

int use_mine_fast(int cn, int in)
{
	if (!cn)
	{
		return 0;
	}
	if (it[in].carried)
	{
		return 0;
	}

	fx_add_effect(10, TICKS * 60 * 15, it[in].x, it[in].y, it[in].temp);

	reset_go(it[in].x, it[in].y);
	remove_lights(it[in].x, it[in].y);

	map[it[in].x + it[in].y * MAPX].it = 0;
	it[in].used = USE_EMPTY;

	reset_go(it[in].x, it[in].y);
	add_lights(it[in].x, it[in].y);

	return 1;
}

int build_ring(int cn, int in)
{
	int t1, t2, in2, r, in3;

	t1 = it[in].temp;

	in2 = ch[cn].citem;
	if (!in2)
	{
		t2 = 0;
	}
	else if (it[in2].temp==0) // prevents altered gear from being consumed
	{
		return 0;
	}
	else
	{
		t2 = it[in2].temp;
	}

	// Turn clumps of metal into rings
	if 			(t1==IT_SILV && t2==0) r = IT_SILV_RING;
	else if 	(t1==IT_GOLD && t2==0) r = IT_GOLD_RING;
	else if 	(t1==IT_PLAT && t2==0) r = IT_PLAT_RING;
	//
	else if (t1==IT_PLAT_RING)
	{
		switch(t2)
		{
			case IT_S_SA: 
			case IT_S_RU: 
			case IT_S_AM: 
			case IT_S_TO: 
			case IT_S_EM: 
			case IT_S_DI: 
			//
			case IT_M_SA: 
			case IT_M_RU: 
			case IT_M_AM: 
			case IT_M_TO: 
			case IT_M_EM: 
			case IT_M_DI: 
				do_char_log(cn, 0, "This stone is too weak for a platinum ring.\n");
				return (0);
			//
			case IT_B_SA: r = 1414; break;
			case IT_B_RU: r = 1415; break;
			case IT_B_AM: r = 1416; break;
			case IT_B_TO: r = 1417; break;
			case IT_B_EM: r = 1418; break;
			case IT_B_DI: r = 1419; break;
			//
			case IT_H_SA: r = 1420; break;
			case IT_H_RU: r = 1421; break;
			case IT_H_AM: r = 1422; break;
			case IT_H_TO: r = 1423; break;
			case IT_H_EM: r = 1424; break;
			case IT_H_DI: r = 1425; break;
			//
			case IT_F_SA: r = 1426; break;
			case IT_F_RU: r = 1427; break;
			case IT_F_AM: r = 1428; break;
			case IT_F_TO: r = 1429; break;
			case IT_F_EM: r = 1430; break;
			case IT_F_DI: r = 1431; break;
			//
			case IT_B_SP: r = 2087; break;
			case IT_B_CI: r = 2088; break;
			case IT_B_OP: r = 2089; break;
			case IT_B_AQ: r = 2090; break;
			//
			case IT_H_SP: r = 2091; break;
			case IT_H_CI: r = 2092; break;
			case IT_H_OP: r = 2093; break;
			case IT_H_AQ: r = 2094; break;
			case IT_H_BE: r = 2095; break;
			case IT_H_ZI: r = 2096; break;
			//
			case IT_F_SP: r = 2097; break;
			case IT_F_CI: r = 2098; break;
			case IT_F_OP: r = 2099; break;
			case IT_F_AQ: r = 2100; break;
			case IT_F_BE: r = 2101; break;
			case IT_F_ZI: r = 2102; break;
			//
			default: 
				return (0);
		}
	}
	else if (t1==IT_GOLD_RING)
	{
		switch(t2)
		{
			case IT_S_SA: 
			case IT_S_RU: 
			case IT_S_AM: 
			case IT_S_TO: 
			case IT_S_EM: 
			case IT_S_DI: 
				do_char_log(cn, 0, "This stone is too weak for a gold ring.\n");
				return (0);
			//
			case IT_M_SA: r = 1396; break;
			case IT_M_RU: r = 1397; break;
			case IT_M_AM: r = 1398; break;
			case IT_M_TO: r = 1399; break;
			case IT_M_EM: r = 1400; break;
			case IT_M_DI: r = 1401; break;
			//
			case IT_B_SA: r = 1402; break;
			case IT_B_RU: r = 1403; break;
			case IT_B_AM: r = 1404; break;
			case IT_B_TO: r = 1405; break;
			case IT_B_EM: r = 1406; break;
			case IT_B_DI: r = 1407; break;
			//
			case IT_H_SA: r = 1408; break;
			case IT_H_RU: r = 1409; break;
			case IT_H_AM: r = 1410; break;
			case IT_H_TO: r = 1411; break;
			case IT_H_EM: r = 1412; break;
			case IT_H_DI: r = 1413; break;
			//
			case IT_B_SP: r = 2077; break;
			case IT_B_CI: r = 2078; break;
			case IT_B_OP: r = 2079; break;
			case IT_B_AQ: r = 2080; break;
			//
			case IT_H_SP: r = 2081; break;
			case IT_H_CI: r = 2082; break;
			case IT_H_OP: r = 2083; break;
			case IT_H_AQ: r = 2084; break;
			case IT_H_BE: r = 2085; break;
			case IT_H_ZI: r = 2086; break;
			//
			case IT_F_SP:
			case IT_F_CI:
			case IT_F_OP:
			case IT_F_AQ:
			case IT_F_BE:
			case IT_F_ZI:
			//
			case IT_F_SA: 
			case IT_F_RU: 
			case IT_F_AM: 
			case IT_F_TO: 
			case IT_F_EM: 
			case IT_F_DI: 
				do_char_log(cn, 0, "This stone is too powerful for a gold ring.\n");
			//
			default: 
				return (0);
		}
	}
	else if (t1==IT_SILV_RING)
	{
		switch(t2)
		{
			case IT_S_SA: r = 1378; break;
			case IT_S_RU: r = 1379; break;
			case IT_S_AM: r = 1380; break;
			case IT_S_TO: r = 1381; break;
			case IT_S_EM: r = 1382; break;
			case IT_S_DI: r = 1383; break;
			//
			case IT_M_SA: r = 1384; break;
			case IT_M_RU: r = 1385; break;
			case IT_M_AM: r = 1386; break;
			case IT_M_TO: r = 1387; break;
			case IT_M_EM: r = 1388; break;
			case IT_M_DI: r = 1389; break;
			//
			case IT_B_SA: r = 1390; break;
			case IT_B_RU: r = 1391; break;
			case IT_B_AM: r = 1392; break;
			case IT_B_TO: r = 1393; break;
			case IT_B_EM: r = 1394; break;
			case IT_B_DI: r = 1395; break;
			//
			case IT_B_SP: r = 2073; break;
			case IT_B_CI: r = 2074; break;
			case IT_B_OP: r = 2075; break;
			case IT_B_AQ: r = 2076; break;
			//
			case IT_H_SP:
			case IT_H_CI:
			case IT_H_OP:
			case IT_H_AQ:
			case IT_H_BE:
			case IT_H_ZI:
			//
			case IT_F_SP:
			case IT_F_CI:
			case IT_F_OP:
			case IT_F_AQ:
			case IT_F_BE:
			case IT_F_ZI:
			//
			case IT_H_SA: 
			case IT_H_RU: 
			case IT_H_AM: 
			case IT_H_TO: 
			case IT_H_EM: 
			case IT_H_DI: 
			//
			case IT_F_SA: 
			case IT_F_RU: 
			case IT_F_AM: 
			case IT_F_TO: 
			case IT_F_EM: 
			case IT_F_DI: 
				do_char_log(cn, 0, "This stone is too powerful for a silver ring.\n");
			//
			default: 
				return (0);
		}
	}
	else
	{
		return 0;
	}

	in3 = god_create_item(r);
	it[in3].flags |= IF_UPDATE;

	if (in2)
	{
		ch[cn].citem = 0;
		it[in2].used = USE_EMPTY;
	}

	god_take_from_char(in, cn);
	it[in].used = USE_EMPTY;

	god_give_char(in3, cn);

	return 1;
}

int build_ankh(int cn, int in)
{
	int in2, in3, r = 0;
	
	in2 = ch[cn].citem;
	if (!in2 || (in2 & 0x80000000))
		return 0;
	
	if (it[in].data[0] != cn)
	{
		do_char_log(cn, 1, "Nothing happened.\n");
		return 0;
	}
	
	if 			(it[in2].temp==IT_UNS_AMBER)	r = IT_AMBERANKH;
	else if 	(it[in2].temp==IT_UNS_TURQU)	r = IT_TURQUANKH;
	else if 	(it[in2].temp==IT_UNS_GARNE)	r = IT_GARNEANKH;
	else
		return 0;
	
	in3 = god_create_item(r);
	
	if (in3)
	{
		char buf[300];

		it[in3].data[0] = cn;

		sprintf(buf, "%s Engraved in it are the letters \"%s\".",
		        it[in3].description, ch[cn].name);
		if (strlen(buf)<200)
		{
			strcpy(it[in3].description, buf);
		}
	}
	
	if (in2)
	{
		ch[cn].citem = 0;
		it[in2].used = USE_EMPTY;
	}

	god_take_from_char(in, cn);
	it[in].used = USE_EMPTY;

	god_give_char(in3, cn);
	
	do_char_log(cn, 1, "You enhanced the amulet.\n");
}

int place_ankh(int cn, int in)
{
	int in2, ta, tb, tc;
	
	in2 = ch[cn].citem;
	
	if (!in2)
	{
		if (it[in].active)
			do_char_log(cn, 1, "You can't seem to take it back.\n");
		else
			do_char_log(cn, 1, "You feel something could be put here...\n");
		return 0;
	}
	if (it[in].active)
	{
		do_char_log(cn, 1, "This spot is occupied.\n");
		return 0;
	}
	if (it[in2].temp != it[in].data[0])
	{
		do_char_log(cn, 1, "Seems like that doesn't fit here.\n");
		return 0;
	}
	if (it[in2].data[0] != cn)
	{
		do_char_log(cn, 1, "It was shoved back into your hand.\n");
		return 0;
	}
	
	do_char_log(cn, 1, "You placed your %s on the table.\n", it[in2].name);
	
	ch[cn].citem = 0;
	it[in2].used = USE_EMPTY;
	
	it[in].active = it[in].duration;
	
	if ((ta = map[120369].it) && it[ta].active && 
		(tb = map[120371].it) && it[tb].active && 
		(tc = map[120373].it) && it[tc].active)
	{
		// All ankhs are placed. A winner is you.
		in2 = map[130611].it;
		it[in2].active = it[ta].active = it[tb].active = it[tc].active = 0;
		
		do_char_log(cn, 2, "The ankhs danced around the room in a flash and merged together on the table behind you.\n");
		
		fx_add_effect(6, 0, 561, 117, 0);
		fx_add_effect(6, 0, 563, 117, 0);
		fx_add_effect(6, 0, 565, 117, 0);
		fx_add_effect(7, 0, 563, 127, 0);
		
		return 0;
	}
}

int build_sinbinder(int cn, int in)
{
	int n, t1, t2, in2;
	char buf[300];

	t1 = it[in].temp;
	
	in2 = ch[cn].citem;
	if (t1!=IT_TW_SINBIND || !in2 || (in2 & 0x80000000) || it[in].data[1])
	{
		do_char_log(cn, 1, "Nothing happens.\n");
		return 0;
	}

	t2 = it[in2].temp;
	
	if (t2==IT_CH_FOOL)
	{
		for (n=0; n<5; n++) { it[in].attrib[n][0] = it[in2].attrib[n][0]; it[in].attrib[n][2] = it[in2].attrib[n][2]; }
		it[in].hp[0]          = it[in2].hp[0];
		it[in].hp[2]          = it[in2].hp[2];
		it[in].end[0]         = it[in2].end[0];
		it[in].mana[0]        = it[in2].mana[0];
		it[in].mana[2]        = it[in2].mana[2];
		it[in].armor[0]       = it[in2].armor[0];
		it[in].weapon[0]      = it[in2].weapon[0];
		it[in].move_speed[0]  = it[in2].move_speed[0];
		it[in].atk_speed[0]   = it[in2].atk_speed[0];
		it[in].cast_speed[0]  = it[in2].cast_speed[0];
		it[in].spell_mod[0]   = it[in2].spell_mod[0];
		it[in].spell_apt[0]   = it[in2].spell_apt[0];
		it[in].cool_bonus[0]  = it[in2].cool_bonus[0];
		it[in].crit_chance[0] = it[in2].crit_chance[0];
		it[in].crit_multi[0]  = it[in2].crit_multi[0];
		it[in].top_damage[0]  = it[in2].top_damage[0];
		it[in].gethit_dam[0]  = it[in2].gethit_dam[0];
		for (n=0; n<50; n++) { it[in].skill[n][0] = it[in2].skill[n][0]; it[in].skill[n][2] = it[in2].skill[n][2]; }
	}
	else if (t2>=IT_CH_MAGI   && t2<=IT_CH_WORLD  ) ;
	else if (t2>=IT_CH_FOOL_R && t2<=IT_CH_WORLD_R) ;
	else
	{
		do_char_log(cn, 1, "That doesn't fit.\n");
		return 0;
	}
	
	if (in2)
	{
		sprintf(buf, "A hollow ring made of an odd brass-like metal. Attached is the card %s. Engraved in it are the letters \"%s\".", 
			it[in2].name, ch[cn].name);
		if (strlen(buf)<200)
		{
			strcpy(it[in].description, buf);
		}
		
		ch[cn].citem = 0;
		it[in2].used = USE_EMPTY;
	}
	
	it[in].data[1] = 1;
	it[in].data[2] = t2;
	it[in].sprite[0] = IT_TW_SINSPR;
	it[in].flags &= ~IF_USESPECIAL;
	it[in].flags |= IF_UPDATE;
}

int build_object(int cn, int in) // Used for Sun Amulet and Hourglass Pieces and Deus Amulet
{
	int t1, t2, in2, r, in3;

	t1 = it[in].temp;

	in2 = ch[cn].citem;
	if (!in2 || (in2 & 0x80000000))
	{
		do_char_log(cn, 1, "Nothing happens.\n");
		return 0;
	}

	t2 = it[in2].temp;

	// Sun amulet pieces
	if 			(t1==471 && t2==472)	r = 476;
	else if 	(t1==472 && t2==471)	r = 476;
	else if 	(t1==471 && t2==473)	r = 474;
	else if 	(t1==473 && t2==471)	r = 474;
	else if 	(t1==472 && t2==473)	r = 475;
	else if 	(t1==473 && t2==472)	r = 475;
	else if 	(t1==471 && t2==475)	r = 466;
	else if 	(t1==475 && t2==471)	r = 466;
	else if 	(t1==472 && t2==474)	r = 466;
	else if 	(t1==474 && t2==472)	r = 466;
	else if 	(t1==473 && t2==476)	r = 466;
	else if 	(t1==476 && t2==473)	r = 466;
	// Hourglass pieces
	else if 	(t1==1821 && t2==1822)	r = 1823;
	else if 	(t1==1821 && t2==1825)	r = 1828;
	else if 	(t1==1821 && t2==1826)	r = 1831;
	else if 	(t1==1821 && t2==1827)	r = 1829;
	else if 	(t1==1821 && t2==1832)	r = 1820;
	else if 	(t1==1824 && t2==1825)	r = 1826;
	else if 	(t1==1824 && t2==1822)	r = 1827;
	else if 	(t1==1824 && t2==1823)	r = 1829;
	else if 	(t1==1824 && t2==1828)	r = 1831;
	else if 	(t1==1824 && t2==1830)	r = 1820;
	else if 	(t1==1822 && t2==1821)	r = 1823;
	else if 	(t1==1822 && t2==1824)	r = 1827;
	else if 	(t1==1822 && t2==1826)	r = 1832;
	else if 	(t1==1822 && t2==1828)	r = 1830;
	else if 	(t1==1822 && t2==1831)	r = 1820;
	else if 	(t1==1825 && t2==1824)	r = 1826;
	else if 	(t1==1825 && t2==1821)	r = 1828;
	else if 	(t1==1825 && t2==1823)	r = 1830;
	else if 	(t1==1825 && t2==1827)	r = 1832;
	else if 	(t1==1825 && t2==1829)	r = 1820;
	else if 	(t1==1823 && t2==1825)	r = 1830;
	else if 	(t1==1823 && t2==1824)	r = 1829;
	else if 	(t1==1823 && t2==1826)	r = 1820;
	else if 	(t1==1826 && t2==1821)	r = 1831;
	else if 	(t1==1826 && t2==1822)	r = 1832;
	else if 	(t1==1826 && t2==1823)	r = 1820;
	else if 	(t1==1827 && t2==1821)	r = 1829;
	else if 	(t1==1827 && t2==1825)	r = 1832;
	else if 	(t1==1827 && t2==1828)	r = 1820;
	else if 	(t1==1828 && t2==1824)	r = 1831;
	else if 	(t1==1828 && t2==1822)	r = 1830;
	else if 	(t1==1828 && t2==1827)	r = 1820;
	else if 	(t1==1830 && t2==1824)	r = 1820;
	else if 	(t1==1829 && t2==1825)	r = 1820;
	else if 	(t1==1831 && t2==1822)	r = 1820;
	else if 	(t1==1832 && t2==1821)	r = 1820;
	// Deus
	else if 	(t1==2295 && t2==2296)	r = 2297;
	else if 	(t1==2296 && t2==2295)	r = 2297;
	// Eclipse belt
	else if 	(t1==IT_BL_SOLAR && t2==IT_BL_LUNAR)	r = IT_BL_ECLIPSE;
	else if 	(t1==IT_BL_LUNAR && t2==IT_BL_SOLAR)	r = IT_BL_ECLIPSE;
	//
	else
	{
		do_char_log(cn, 1, "That doesn't fit.\n");
		return 0;
	}

	in3 = god_create_item(r);
	it[in3].flags |= IF_UPDATE;

	if (in2)
	{
		ch[cn].citem = 0;
		it[in2].used = USE_EMPTY;
	}

	god_take_from_char(in, cn);
	it[in].used = USE_EMPTY;

	god_give_char(in3, cn);

	return 1;
}

int use_gargoyle(int cn, int in)
{
	int cc;

	if (!cn)
	{
		return 0;
	}
	if (!it[in].carried)
	{
		return 0;
	}

	cc = god_create_char(CT_GARGSTAT, 1);
	if (!god_drop_char_fuzzy(cc, ch[cn].x, ch[cn].y))
	{
		ch[cc].used = USE_EMPTY;
		do_char_log(cn, 0, "The Gargoyle could not materialize.\n");
		return 0;
	}

	god_take_from_char(in, cn);
	it[in].used = USE_EMPTY;

	ch[cc].data[CHD_GROUP] = 65536 + cn;                       // set group
	ch[cc].data[59] = 65536 + cn;                       // protect all other members of this group

	ch[cc].data[CHD_MASTER] = cn;                             // obey and protect char
	ch[cc].data[69] = cn;                             // follow char
	ch[cc].data[64] = globs->ticker + (TICKS * 60 * 15);    // self destruction

	return 1;
}

int use_grolm(int cn, int in)
{
	int cc;

	if (!cn)
	{
		return 0;
	}
	if (!it[in].carried)
	{
		return 0;
	}

	cc = god_create_char(CT_GROLMSTA, 1);
	if (!god_drop_char_fuzzy(cc, ch[cn].x, ch[cn].y))
	{
		ch[cc].used = USE_EMPTY;
		do_char_log(cn, 0, "The Grolm could not materialize.\n");
		return 0;
	}

	god_take_from_char(in, cn);
	it[in].used = USE_EMPTY;

	ch[cc].data[CHD_GROUP] = 65536 + cn;                       // set group
	ch[cc].data[59] = 65536 + cn;                       // protect all other members of this group

	ch[cc].data[CHD_MASTER] = cn;                             // obey and protect char
	ch[cc].data[69] = cn;                             // follow char
	ch[cc].data[64] = globs->ticker + (TICKS * 60 * 15);    // self destruction

	return 1;
}

void boost_char(int cn, int divi)
{
	int n, in, rank, exp, v, m;
	char buf[80];
	
	// Moved up here to prevent Rank 0 soul stones from generating.
	exp  = ch[cn].points_tot / 8 + RANDOM(ch[cn].points_tot / 16 + 1);
	rank = points2rank(exp);
	
	if (rank > N_SOULMAX)
		rank = N_SOULMAX;

	if (rank)
	{
		for (n = 0; n<5; n++)
		{
			if (B_AT(cn, n)>divi)
			{
				B_AT(cn, n) += RANDOM(B_AT(cn, n) / divi);
			}
		}
		for (n = 0; n<MAXSKILL; n++)
		{
			if (B_SK(cn, n)>divi)
			{
				B_SK(cn, n) += RANDOM(B_SK(cn, n) / divi);
			}
		}

		sprintf(buf, "Strong %s", ch[cn].name);
		strncpy(ch[cn].name, buf, 39);
		ch[cn].name[39] = 0;
		sprintf(buf, "the %s", ch[cn].name);
		strncpy(ch[cn].reference, buf, 39);
		ch[cn].reference[39] = 0;
		
		// New
		if (!(ch[cn].flags & CF_EXTRAEXP)) ch[cn].flags |= CF_EXTRAEXP;

		in = god_create_item(IT_SOULSTONE);
		if (in)
		{
			sprintf(it[in].name, "Soulstone");
			sprintf(it[in].reference, "soulstone");
			sprintf(it[in].description, "Level %d soulstone, holding %d exp.", rank, exp);
			
			it[in].data[0] = rank;
			it[in].data[1] = exp;
			it[in].temp = 0;
			it[in].driver = 68;
			
			if (rank>7 && !RANDOM(4))
			{
				v = RANDOM(70);
				if (v>=50 && v<=54)	{ it[in].attrib[v-50][0] = get_sb(v, 1); m = it[in].attrib[v-50][1] = get_sb(v, 0)*2; }
				else if (v==55)     { it[in].hp[0]           = get_sb(v, 1); m = it[in].hp[1]           = get_sb(v, 0)*2; }
				else if (v==56)     { it[in].end[0]          = get_sb(v, 1); m = it[in].end[1]          = get_sb(v, 0)*2; }
				else if (v==57)     { it[in].mana[0]         = get_sb(v, 1); m = it[in].mana[1]         = get_sb(v, 0)*2; }
				else if (v==58)     { it[in].weapon[0]       = get_sb(v, 1); m = it[in].weapon[1]       = get_sb(v, 0)*2; }
				else if (v==59)     { it[in].armor[0]        = get_sb(v, 1); m = it[in].armor[1]        = get_sb(v, 0)*2; }
				else if (v==60)     { it[in].move_speed[0]   = get_sb(v, 1); m = it[in].move_speed[1]   = get_sb(v, 0)*2; }
				else if (v==61)     { it[in].atk_speed[0]    = get_sb(v, 1); m = it[in].atk_speed[1]    = get_sb(v, 0)*2; }
				else if (v==62)     { it[in].cast_speed[0]   = get_sb(v, 1); m = it[in].cast_speed[1]   = get_sb(v, 0)*2; }
				else if (v==63)     { it[in].spell_mod[0]    = get_sb(v, 1); m = it[in].spell_mod[1]    = get_sb(v, 0)*2; }
				else if (v==64)     { it[in].spell_apt[0]    = get_sb(v, 1); m = it[in].spell_apt[1]    = get_sb(v, 0)*2; }
				else if (v==65)     { it[in].cool_bonus[0]   = get_sb(v, 1); m = it[in].cool_bonus[1]   = get_sb(v, 0)*2; }
				else if (v==66)     { it[in].crit_chance[0]  = get_sb(v, 1); m = it[in].crit_chance[1]  = get_sb(v, 0)*2; }
				else if (v==67)     { it[in].crit_multi[0]   = get_sb(v, 1); m = it[in].crit_multi[1]   = get_sb(v, 0)*2; }
				else if (v==68)     { it[in].top_damage[0]   = get_sb(v, 1); m = it[in].top_damage[1]   = get_sb(v, 0)*2; }
				else if (v==69)     { it[in].gethit_dam[0]   = get_sb(v, 1); m = it[in].gethit_dam[1]   = get_sb(v, 0)*2; }
				else                { it[in].skill[v][0]     = get_sb(v, 1); m = it[in].skill[v][1]     = get_sb(v, 0)*2; }
				
				it[in].data[3] = m;
				it[in].flags |= IF_IDENTIFIED;
			}
			
			if (exp>= 6500000 && !RANDOM(8)) it[in].data[2] = 4+RANDOM(4);
			if (exp>=14000000 && !RANDOM(6)) it[in].data[2] = 3+RANDOM(3);
			
			god_give_char(in, cn);
		}
	}
}

int spawn_penta_enemy(int in)
{
	int cn, tmp;

	tmp = it[in].data[9] + (RANDOM(3) - 1); // -1, 0, +1
	
	if (it[in].data[9]==9)			// diffi  9 -- 2nd Leiu Grolm
	{
		if (tmp >  9) tmp =  9;
	}
	else if (it[in].data[9]==10)	// diffi 10 -- 2nd Leiu Garg
	{
		if (tmp < 10) tmp = 10;
	}
	else if (it[in].data[9]==16)	// diffi 16 -- Brig Gen Garg
	{
		if (tmp > 16) tmp = 16;
	}
	else if (it[in].data[9]==17)	// diffi 17 -- Brig Gen Grulge
	{
		if (tmp < 17) tmp = 17;
	}
	else if (it[in].data[9]==21)	// diffi 21 -- Field Marshal Grulge
	{
		if (tmp > 21) tmp = 21;
	}
	else if (it[in].data[9]>=22)	// diffi 22 -- Field Marshal Ice Garg
	{
		tmp = it[in].data[9]; 		// No more variance after ice gargs
	}

	if (tmp< 0) tmp =  0;
	if (tmp>28) tmp = 28;
	
	cn = pop_create_char(CT_PENTMOB + tmp, 0);
	if (!cn) return 0;
	
	ch[cn].flags &= ~CF_RESPAWN;
	ch[cn].data[0]  = in;
	ch[cn].data[29] = it[in].x + it[in].y * MAPX;
	ch[cn].data[60] = TICKS * 60 * 2;
	ch[cn].data[73] = 8;
	ch[cn].dir = 1;

	if (!RANDOM(30))
	{
		boost_char(cn, 5);
	}

	if (!god_drop_char_fuzzy(cn, it[in].x, it[in].y))
	{
		god_destroy_items(cn);
		ch[cn].used = USE_EMPTY;
		return 0;
	}
	else
	{
		return cn;
	}
}

static int penta_needed = 5;

int is_inpents(int cn)
{
	int x, y;
	
	if(!IS_PLAYER(cn)) return 0;
	if(!IS_ACTIVECHAR(cn)) return 0;
	
	x = ch[cn].x;
	y = ch[cn].y;
	
	if (x >= 658 && y >= 344 && x <= 730 && y <= 461) return 1; // Pents 1
	if (x >= 217 && y >= 232 && x <= 441 && y <= 586) return 1; // Pents 2
	
	return 0;
}

int get_pent_value(int pnum)
{
	// Values are multiplied by 10000 here for decimal accuracy.
	// Apparently each bracket step gets truncated as its own int, who knew?
	return ( ( pnum*pnum*10000 ) / ( 7*10000+pnum*10000/99 ) +1 );
}

int get_kill_streak_exp(int k, int p)
{
	return ( k*2 + k*(p*p/16)/8 );
}

void show_pent_count(int cn)
{
	do_char_log(cn, 2, "Next solve is at %d pentagrammas.\n", penta_needed);
}

void solved_pentagram(int cn, int in)
{
	int n, m, bonus, inpents = 0, gbonus, co;
	int xf, yf, xt, yt, x, y, dontkill;

	bonus = get_pent_value(it[in].data[0]) * 3; //(it[in].data[0] * it[in].data[0] * 3) / 9 + 1;
	
	ch[cn].data[18] += bonus;

	do_char_log(cn, 0, "You solved the pentagram quest. Congratulations! You will get %d bonus experience points.\n", bonus);
	chlog(cn, "Solved pentagram quest");
	
	//
	for (n = 1; n<MAXCHARS; n++) if (is_inpents(n)) inpents++;
	if (inpents<1) inpents = 1;
	
	penta_needed = 2 + (inpents * (4 + RANDOM(3)));
	if (penta_needed > 66) penta_needed = 66;
	xlog("New solve will be at %d (%d player(s) in pents)", penta_needed, inpents);
	//
	
	for (n = 1; n<MAXCHARS; n++)
	{
		if (ch[n].used==USE_EMPTY)
		{
			continue;
		}
		if (!(ch[n].flags & (CF_PLAYER | CF_USURP)))
		{
			continue;
		}

		if (ch[n].used==USE_ACTIVE)
		{
			if (n!=cn)
			{
				do_char_log(n, 0, "%s solved the pentagram quest!\n", ch[cn].name);
				if (isgroup(n, cn) && isgroup(cn, n) && ch[n].data[18]>0)
				{
					gbonus = bonus/2;
					
					if (gbonus > ch[n].data[18]/3)
						gbonus = ch[n].data[18]/3;
					
					ch[n].data[18]+=gbonus;
					
					do_char_log(n, 0, "Since you were a member of their group, you earned %d bonus experience points.\n", gbonus);
				}
			}
			/*
			else
			{
				do_char_log(n, 1, "Next solve will be at %d pentagrammas.\n", penta_needed);
			}
			*/
		}
		
		if (ch[n].data[18] || (ch[n].data[77] && ch[n].data[78]))
		{
			do_give_exp(n, ch[n].data[18]+min(ch[n].data[18], get_kill_streak_exp(ch[n].data[77],ch[n].data[78])), 0, -1);
			ch[n].data[18] = 0;	ch[n].data[77] = 0;	ch[n].data[78] = 0;
			do_char_log(n, 1, "Next solve will be at %d pentagrammas.\n", penta_needed);
		}
		else if (ch[n].data[77])
		{
			ch[n].data[77] = 0;
		}
	}

	for (n = 1; n<MAXITEM; n++)
	{
		if (it[n].used==USE_EMPTY)
		{
			continue;
		}
		if (it[n].driver!=33)
		{
			continue;
		}
		if (it[n].active==0)
		{
			if (it[n].light[0]!=it[n].light[1] && it[n].x>0)
			{
				do_add_light(it[n].x, it[n].y, it[n].light[1] - it[n].light[0]);
			}
		}
		it[n].duration = it[n].active = TICKS * 10 + RANDOM(TICKS*10);
		
		for (m = 1; m<4; m++)
		{
			dontkill = 0;
			if ((co = it[n].data[m])!=0 && ch[co].data[0]==n && ch[co].used!=USE_EMPTY && !(ch[co].flags & CF_BODY))
			{
				xf = max(1, it[n].x - 11);
				yf = max(1, it[n].y - 11);
				xt = min(MAPX - 1, it[n].x + 12);
				yt = min(MAPY - 1, it[n].y + 12);
				for (x = xf; x<xt; x++) for (y = yf; y<yt; y++) if ((cn = map[x+y*MAPX].ch)) if (cn==co) 
				{
					dontkill = 1;
				}
				if (!dontkill) do_char_killed(0, co, 1);
			}
		}
	}
}

int use_pentagram(int cn, int in)
{
	int n, v, tot = 0, act = 0, b[5] = {0, 0, 0, 0, 0}, bv[5] = {0, 0, 0, 0, 0}, exp = 0, co, m;
	int r1, r2;

	if (it[in].active)
	{
		if (cn)
		{
			do_char_log(cn, 0, "This pentagram is already active.\n");
		}
		else
		{
			for (m = 1; m<4; m++)
			{
				if ((co = it[in].data[m])==0 || ch[co].data[0]!=in || ch[co].used==USE_EMPTY || (ch[co].flags & CF_BODY))
				{
					it[in].data[m] = spawn_penta_enemy(in);
				}
			}
		}
		return 0;
	}

	r1 = points2rank(ch[cn].points_tot);
	r2 = it[in].data[9]+5;

	if (r1>r2)
	{
		do_char_log(cn, 0, "You cannot use this pentagram. It is reserved for rank %s and below.\n",
		            rank_name[min(23, max(0, r2))]);
		return 0;
	}

	v = it[in].data[0];
	it[in].data[8]  = cn;
	it[in].duration = -1;
	
	do_char_log(cn, 1, "You activated the pentagram with the value %d. It is worth %d experience point%s.\n", v, get_pent_value(v), v==1 ? "" : "s");

	for (n = 1; n<MAXITEM; n++)
	{
		if (it[n].used==USE_EMPTY)
		{
			continue;
		}
		if (it[n].driver!=33)
		{
			continue;
		}
		tot++;
		if (n!=in && it[n].active!=-1)
		{
			continue;
		}
		act++;
		if (it[n].data[8]!=cn)
		{
			continue;
		}

		v = it[n].data[0];
		if (v>bv[0])
		{
			bv[4] = bv[3];
			bv[3] = bv[2];
			bv[2] = bv[1];
			bv[1] = bv[0];
			bv[0] = v;
			b[4]  = b[3];
			b[3]  = b[2];
			b[2]  = b[1];
			b[1]  = b[0];
			b[0]  = n;
		}
		else if (v>bv[1])
		{
			bv[4] = bv[3];
			bv[3] = bv[2];
			bv[2] = bv[1];
			bv[1] = v;
			b[4]  = b[3];
			b[3]  = b[2];
			b[2]  = b[1];
			b[1]  = n;
		}
		else if (v>bv[2])
		{
			bv[4] = bv[3];
			bv[3] = bv[2];
			bv[2] = v;
			b[4]  = b[3];
			b[3]  = b[2];
			b[2]  = n;
		}
		else if (v>bv[3])
		{
			bv[4] = bv[3];
			bv[3] = v;
			b[4]  = b[3];
			b[3]  = n;
		}
		else if (v>bv[4])
		{
			bv[4] = v;
			b[4]  = n;
		}
	}

	if (b[0] && !(ch[cn].flags & CF_SYS_OFF))
	{
		do_char_log(cn, 1, "You're holding:\n");
	}

	for (n = 0; n<5; n++)
	{
		if (b[n])
		{
			if (!(ch[cn].flags & CF_SYS_OFF))
				do_char_log(cn, 1, "Pentagram %3d, worth %5d point%s.\n",
									bv[n], get_pent_value(bv[n]), bv[n]==1 ? "" : "s");
			exp += get_pent_value(bv[n]); //(bv[n] * bv[n]) / 9 + 1;
		}
	}
	
	ch[cn].data[78] = bv[0];
	ch[cn].data[18] = exp;
	//                 "!        .         .   |     .         .        !"
	do_char_log(cn, 2, "Your pentagrammas have a total of %d exp.\n", exp);
	if (r1<9) 
		do_char_log(cn, 1, "Note that only the highest 5 pentagrammas count towards your experience bonus.\n");
	do_char_log(cn, 1, "%d of the %d pentagrammas are now active.\n", act, tot);
	if (ch[cn].data[77]) 
		do_char_log(cn, 0, "Your kill streak is %d, worth %d exp.\n", 
			ch[cn].data[77], min(exp, get_kill_streak_exp(ch[cn].data[77],ch[cn].data[78])));

	chlog(cn, "Activated pentagram %d (%d of %d)", it[in].data[0], act, penta_needed);

	if (act>=penta_needed)
	{
		solved_pentagram(cn, in);
		return 0;
	}

	for (m = 1; m<4; m++)
	{
		if ((co = it[in].data[m])==0 || ch[co].data[0]!=in || ch[co].used==USE_EMPTY || (ch[co].flags & CF_BODY))
		{
			it[in].data[m] = spawn_penta_enemy(in);
		}
	}

	return 1;
}

int use_shrine(int cn, int in)
{
	int in2, val, rank, m;

	if (!cn)
	{
		return 0;
	}

	if ((in2 = ch[cn].citem)==0)
	{
		do_char_log(cn, 1, "You get the feeling that it would be appropriate to give the gods a present.\n");
		return 0;
	}
	
	if (in2 & 0x80000000)
	{
		val = in2 & 0x7fffffff;
		ch[cn].citem = 0;
	}
	else
	{
		val = it[in2].value;
		if (it[in2].flags & IF_UNIQUE)
		{
			val *= 4;
		}
		it[in2].used = USE_EMPTY;
		ch[cn].citem = 0;
	}

	rank = points2rank(ch[cn].points_tot) + 1;
	rank = rank * rank * rank * 7;
	val += RANDOM(val + 1);

	if (val>=rank)
	{
		if (ch[cn].a_mana<ch[cn].mana[5] * 1000)
		{
			ch[cn].a_mana = ch[cn].mana[5] * 1000;
			do_char_log(cn, 0, "The Goddess of Magic touches your mind.\n");
			fx_add_effect(6, 0, ch[cn].x, ch[cn].y, 0);
		}

		if (val>=rank * 64)
		{
			do_char_log(cn, 1, "The gods are madly in love with your offer.\n");
		}
		else if (val>=rank * 32)
		{
			do_char_log(cn, 1, "The gods love your offer very much.\n");
		}
		else if (val>=rank * 16)
		{
			do_char_log(cn, 1, "The gods love your offer.\n");
		}
		else if (val>=rank * 8)
		{
			do_char_log(cn, 1, "The gods are very pleased with your offer.\n");
		}
		else if (val>=rank * 4)
		{
			do_char_log(cn, 1, "The gods are pleased with your offer.\n");
		}
		else if (val>=rank * 2)
		{
			do_char_log(cn, 1, "The gods deemed your offer appropriate.\n");
		}
		else
		{
			do_char_log(cn, 1, "The gods accepted your offer.\n");
		}
		if (val && rank)
		{
			m = val / max(1, sqrt(rank)*2);
			ch[cn].luck += m;
			if (in2 & 0x80000000)
			{
				chlog(cn, "Increased luck by %d to %d for %dG %dS", m, ch[cn].luck, val / 100, val % 100);
			}
			else
			{
				chlog(cn, "Increased luck by %d to %d for %dG %dS (%s)", m, ch[cn].luck, val / 100, val % 100, it[in2].name);
			}
		}
	}
	else
	{
		m = 0;
		
		if (val<rank / 8)
		{
			do_char_log(cn, 1, "You have angered the gods with your unworthy gift.\n");
			m -= 2+ch[cn].luck/100;
			ch[cn].luck += m;
		}
		else if (val<rank / 4)
		{
			do_char_log(cn, 1, "The gods sneer at your gift.\n");
			m -= 1+ch[cn].luck/1000;
			ch[cn].luck += m;
		}
		else if (val<rank / 2)
		{
			do_char_log(cn, 1, "The gods think you're cheap.\n");
			m -= 1;
			ch[cn].luck--;
		}
		else
		{
			do_char_log(cn, 1, "You feel that it takes more than this to please the gods.\n");
		}
	}

	do_char_log(cn, 1, " \n");
	
	//                                             "!        .         .         .         .        !"
	if (ch[cn].luck<-5000)		do_char_log(cn, 1, "You feel the gods simply hate you.\n");
	else if (ch[cn].luck<-2000)	do_char_log(cn, 1, "You feel the gods are mad at you.\n");
	else if (ch[cn].luck<0)		do_char_log(cn, 1, "You feel the gods are angry at you.\n");
	else if (ch[cn].luck<100)	do_char_log(cn, 1, "You feel the gods stance towards you is neutral.\n");
	else if (ch[cn].luck<2000)	do_char_log(cn, 1, "You feel the gods are pleased with you.\n");
	else if (ch[cn].luck<5000)	do_char_log(cn, 1, "You feel the gods are very fond of you.\n");
	else						do_char_log(cn, 1, "You feel the gods will save your life one day.\n");
	do_char_log(cn, 2, "You have %d luck (%+d).\n", ch[cn].luck, m);
	
	return 1;
}

int use_kill_undead(int cn, int in)
{
	int x, y, co;

	if (!cn)
	{
		return 0;
	}

	if (ch[cn].worn[WN_RHAND]!=in)
	{
		return 0;
	}

	fx_add_effect(7, 0, ch[cn].x, ch[cn].y, 0);

	for (y = ch[cn].y - 8; y<ch[cn].y + 8; y++)
	{
		if (y<1 || y>=MAPY)
		{
			continue;
		}
		for (x = ch[cn].x - 8; x<ch[cn].x + 8; x++)
		{
			if (x<1 || x>=MAPX)
			{
				continue;
			}
			if ((co = map[x + y * MAPX].ch)!=0)
			{
				if ((ch[co].flags & CF_UNDEAD) || 
					(ch[co].worn[WN_BODY] && it[ch[co].worn[WN_BODY]].temp == IT_BONEARMOR))
				{
					do_hurt(cn, co, 500, 2);
					fx_add_effect(5, 0, ch[co].x, ch[co].y, 0);
				}
			}
		}
	}

	item_damage_worn(cn, WN_RHAND, 500);

	return 1;
}

int teleport3(int cn, int in)    // out of labyrinth
{
	int n, in2;

	if (!cn)
	{
		return 1;
	}
	if (it[in].flags & IF_USEACTIVATE && !(it[in].active))
	{
		return 1;
	}

	if ((in2 = ch[cn].citem) && is_nolab_item(in2))
	{
		ch[cn].citem = 0;
		it[in2].used = USE_EMPTY;
		do_char_log(cn, 1, "Your %s vanished.\n", it[in2].reference);
	}

	for (n = 0; n<40; n++)
	{
		if ((in2 = ch[cn].item[n]) && is_nolab_item(in2))
		{
			ch[cn].item[n] = 0;
			ch[cn].item_lock[n] = 0;
			it[in2].used = USE_EMPTY;
			do_char_log(cn, 1, "Your %s vanished.\n", it[in2].reference);
		}
	}

	for (n = 0; n<MAXBUFFS; n++)
	{
		if ((in2 = ch[cn].spell[n]) && (bu[in2].temp==SK_RECALL))
		{
			ch[cn].spell[n] = 0;
			bu[in2].used = USE_EMPTY;
		}
	}

	fx_add_effect(6, 0, ch[cn].x, ch[cn].y, 0);
	god_transfer_char(cn, it[in].data[0], it[in].data[1]);
	char_play_sound(cn, ch[cn].sound + 22, -150, 0);
	fx_add_effect(6, 0, ch[cn].x, ch[cn].y, 0);

	if ((in2 = ch[cn].citem) && !(in2 & 0x80000000) && (it[in2].flags & IF_LABYDESTROY))
	{
		ch[cn].citem = 0;
		it[in2].used = USE_EMPTY;
		do_char_log(cn, 1, "Your %s vanished.\n", it[in2].reference);
	}
	for (n = 0; n<40; n++)
	{
		if ((in2 = ch[cn].item[n]) && (it[in2].flags & IF_LABYDESTROY))
		{
			ch[cn].item[n] = 0;
			ch[cn].item_lock[n] = 0;
			it[in2].used = USE_EMPTY;
			do_char_log(cn, 1, "Your %s vanished.\n", it[in2].reference);
		}
	}
	for (n = 0; n<20; n++)
	{
		if ((in2 = ch[cn].worn[n]) && (it[in2].flags & IF_LABYDESTROY))
		{
			ch[cn].worn[n] = 0;
			it[in2].used = USE_EMPTY;
			do_char_log(cn, 1, "Your %s vanished.\n", it[in2].reference);
		}
	}
	for (n = 0; n<12; n++)
	{
		if ((in2 = ch[cn].alt_worn[n]) && (it[in2].flags & IF_LABYDESTROY))
		{
			ch[cn].alt_worn[n] = 0;
			it[in2].used = USE_EMPTY;
			do_char_log(cn, 1, "Your %s vanished.\n", it[in2].reference);
		}
	}

	if (IS_PURPLE(cn))
	{
		ch[cn].temple_x = ch[cn].tavern_x = HOME_PURPLE_X;
		ch[cn].temple_y = ch[cn].tavern_y = HOME_PURPLE_Y;
	}
	else if (ch[cn].flags & CF_STAFF)
	{
		ch[cn].temple_x = ch[cn].tavern_x = HOME_STAFF_X;
		ch[cn].temple_y = ch[cn].tavern_y = HOME_STAFF_Y;
	}
	else
	{
		ch[cn].temple_x = ch[cn].tavern_x = HOME_TEMPLE_X;
		ch[cn].temple_y = ch[cn].tavern_y = HOME_TEMPLE_Y;
	}

	return 1;
}

int use_seyan_shrine(int cn, int in)
{
	struct item tmp;
	int in2, in3, n, bits;
	unsigned int bit;

	if (!cn)
	{
		return 0;
	}

	if (!IS_SEYAN_DU(cn))
	{
		do_char_log(cn, 0, "You have the feeling you're in the wrong place here.\n");
		return 0;
	}
	
	in3 = ch[cn].alt_worn[WN_RHAND];
	
	// remove old sword if necessary
	if (!(in2 = ch[cn].worn[WN_RHAND]) || it[in2].driver!=40 || it[in2].data[0]!=cn)
	{
		if (!in3 || it[in3].driver!=40 || it[in3].data[0]!=cn) for (n = 1; n<MAXITEM; n++)
		{
			if (it[n].used!=USE_ACTIVE)
			{
				continue;
			}
			if (it[n].driver!=40)
			{
				continue;
			}
			if (it[n].data[0]!=cn)
			{
				continue;
			}

			tmp = it_temp[IT_DEADSEYSWORD];
			tmp.x = it[n].x;
			tmp.y = it[n].y;
			tmp.carried = it[n].carried;
			tmp.temp = IT_DEADSEYSWORD;
			it[n] = tmp;
			it[n].flags |= IF_UPDATE;
		}

		if (ch[cn].luck<50)
		{
			do_char_log(cn, 0, "Kwai, the great goddess of war, deemed you unworthy to receive a new blade.\n");
			return 0;
		}

		in2 = god_create_item(IT_SEYANSWORD);
		god_give_char(in2, cn);
		it[in2].data[0] = cn;
		do_char_log(cn, 0, "Kwai, the great goddess of war, deemed you worthy to receive a new blade.\n");
		ch[cn].luck -= 50;
	}

	if (!(ch[cn].data[21] & it[in].data[0]))
	{
		ch[cn].data[21] |= it[in].data[0];

		do_char_log(cn, 0, "You found a new shrine of Kwai!\n");
		ch[cn].luck += 10;
	}

	for (bits = 0, bit = 1; bit; bit <<= 1)
	{
		if (ch[cn].data[21] & bit)
		{
			bits++;
		}
	}
	
	if (bits<32) 
	{
		do_char_log(cn, 1, "You have visited %d of the 32 shrines of Kwai.\n", bits);
		do_char_log(cn, 1, "You may use #kwai to see unvisited shrines.\n");
	}
	else
	{
		do_char_log(cn, 1, "You have visited all of the shrines of Kwai!\n", bits);
	}

		it[in2].weapon[0] = 15 + bits * 3;
		
		it[in2].to_hit[0]   = max(0, min(5, (bits-10)/4));
		it[in2].to_parry[0] = max(0, min(5, (bits- 8)/4));
		
		for (n = 0; n<5; n++) 
		{
			it[in2].attrib[n][0] = max(0, min(5, (bits-(5-n))/5));
		}
		
		it[in2].crit_multi[0] = max(0, min(25, bits - 5));
		
		if (bits>=31) it[in2].base_crit    = 3;
		if (bits==32) it[in2].spell_mod[0] = 5;
		
		it[in2].flags |= IF_UPDATE;
		it[in2].temp   = 0;
		
		sprintf(it[in2].description, "A huge two-handed sword, engraved with runes and magic symbols. It bears the name %s.",
	        ch[cn].name);
	
	if (in3 && it[in3].driver==40 && it[in3].data[0]==cn)
	{
		it[in3].weapon[0] = 15 + bits * 3;
		
		it[in3].to_hit[0]   = max(0, min(5, (bits-10)/4));
		it[in3].to_parry[0] = max(0, min(5, (bits- 8)/4));
		
		for (n = 0; n<5; n++) 
		{
			it[in3].attrib[n][0] = max(0, min(5, (bits-(5-n))/5));
		}
		
		it[in3].crit_multi[0] = max(0, min(25, bits - 5));
		
		if (bits>=31) it[in3].base_crit    = 3;
		if (bits==32) it[in3].spell_mod[0] = 5;
		
		it[in3].flags |= IF_UPDATE;
		it[in3].temp   = 0;
		
		sprintf(it[in3].description, "A huge two-handed sword, engraved with runes and magic symbols. It bears the name %s.",
	        ch[cn].name);
	}

	do_update_char(cn);

	return 0;
}

int use_seyan_door(int cn, int in)
{
	if (cn && !IS_SEYAN_DU(cn))
	{
		return 0;
	}
	else
	{
		return( use_door(cn, in));
	}
}

int use_seyan_portal(int cn, int in)
{
	int in2, n, co;

	if (!cn)
	{
		return 0;
	}

	if (IS_SEYAN_DU(cn))
	{
		do_char_log(cn, 0, "You're already Seyan'Du, aren't you?\n");
	}
	else
	{
		do_char_log(cn, 0, "The Seyan'Du welcome you among their ranks, %s!\n", ch[cn].name);
		if (IS_MALE(cn))	god_racechange(cn, CT_SEYANDU_M, 0);
		else				god_racechange(cn, CT_SEYANDU_F, 0);
		
		ch[cn].temple_x = ch[cn].tavern_x = HOME_START_X;
		ch[cn].temple_y = ch[cn].tavern_y = HOME_START_Y;
		
		in2 = god_create_item(IT_SEYANSWORD);
		god_give_char(in2, cn);
		it[in2].data[0] = cn;
		
		/* Announce the player's new race */
		if (co<MAXCHARS)
		{
			char message[100];
			sprintf(message, "Hear ye, hear ye! %s has joined the Seyan'du!", ch[cn].name);
			do_shout(co, message);
		}
	}

	if ((in2 = ch[cn].citem) && !(in2 & 0x80000000) && (it[in2].flags & IF_LABYDESTROY))
	{
		ch[cn].citem = 0;
		it[in2].used = USE_EMPTY;
		do_char_log(cn, 1, "Your %s vanished.\n", it[in2].reference);
	}
	for (n = 0; n<40; n++)
	{
		if ((in2 = ch[cn].item[n]) && (it[in2].flags & IF_LABYDESTROY))
		{
			ch[cn].item[n] = 0;
			ch[cn].item_lock[n] = 0;
			it[in2].used = USE_EMPTY;
			do_char_log(cn, 1, "Your %s vanished.\n", it[in2].reference);
		}
	}
	for (n = 0; n<20; n++)
	{
		if ((in2 = ch[cn].worn[n]) && (it[in2].flags & IF_LABYDESTROY))
		{
			ch[cn].worn[n] = 0;
			it[in2].used = USE_EMPTY;
			do_char_log(cn, 1, "Your %s vanished.\n", it[in2].reference);
		}
	}
	for (n = 0; n<12; n++)
	{
		if ((in2 = ch[cn].alt_worn[n]) && (it[in2].flags & IF_LABYDESTROY))
		{
			ch[cn].alt_worn[n] = 0;
			it[in2].used = USE_EMPTY;
			do_char_log(cn, 1, "Your %s vanished.\n", it[in2].reference);
		}
	}

	fx_add_effect(6, 0, ch[cn].x, ch[cn].y, 0);
	god_transfer_char(cn, it[in].data[0], it[in].data[1]);
	char_play_sound(cn, ch[cn].sound + 22, -150, 0);
	fx_add_effect(6, 0, ch[cn].x, ch[cn].y, 0);
	
	return 1;
}

int spell_scroll(int cn, int in)
{
	int spell, power, charges, ret, co=0, value;
	int in2, stack_a, stack_b, tmpv;

	spell = it[in].data[0];
	power = it[in].data[1];
	charges = it[in].data[2];
	value = it[in].value / charges;
	
	// Stack scrolls
	if ((in2 = ch[cn].citem)!=0 && it[in2].temp==it[in].temp)
	{
		stack_a = it[in].data[2];
		if (stack_a <  1) stack_a =  1;
		if (stack_a > 10) stack_a = 10;
		
		stack_b = it[in2].data[2];
		if (stack_b <  1) stack_b =  1;
		if (stack_b > 10) stack_b = 10;
		
		// Clean stacking
		if (stack_a + stack_b <= 10)
		{
			charges = stack_a + stack_b;
			god_take_from_char(in2, cn);
		}
		else
		{
			charges = 10;
			tmpv = it[in2].value / it[in2].data[2];
			it[in2].stack = it[in2].data[2] = stack_a + stack_b - 10;
			it[in2].value = tmpv * it[in2].data[2];
		}
		
		it[in].stack = it[in].data[2] = charges;
		it[in].value = value * charges;
		
		return 0;
	}
	
	if (!charges)
	{
		do_char_log(cn, 0, "Nothing happened!\n");
		return 0;
	}

	if ((co = ch[cn].skill_target1)) ;
	else co = cn;

	if (!do_char_can_see(cn, co))
	{
		do_char_log(cn, 0, "You cannot see your target.\n");
		return 0;
	}

	if (spell==SK_CURSE || spell==SK_SLOW)
	{
		if (!may_attack_msg(cn, co, 1))
		{
			chlog(cn, "Prevented from attacking %s (%d)", ch[co].name, co);
			return 0;
		}
	}
	else if (!player_or_ghost(cn, co))
	{
		do_char_log(cn, 0, "Changed target of spell from %s to %s.\n", ch[co].name, ch[cn].name);
		co = cn;
	}

	switch(spell)
	{
		case    SK_LIGHT:
			ret = spell_light(cn, co, power);
			break;
		case    SK_ENHANCE:
			ret = spell_enhance(cn, co, power, 1);
			break;
		case    SK_PROTECT:
			ret = spell_protect(cn, co, power, 1);
			break;
		case    SK_BLESS:
			ret = spell_bless(cn, co, power, 1);
			break;
		case    SK_MSHIELD:
			ret = spell_mshield(cn, co, power, 1);
			break;
		case    SK_HASTE:
			ret = spell_haste(cn, co, power, 1);
			break;
		case    SK_IDENT:
			if (!IS_SANEITEM(in2 = ch[cn].citem)) in2 = 0;
			if ((in2 && chance_base(cn, SK_IDENT, SP_MULT_IDENTIFY, it[in2].power, power+100))
				|| (!in2 && cn!=co && chance_base(cn, SK_IDENT, SP_MULT_IDENTIFY, get_target_resistance(cn, co), power+100)))
			{
				ret = 1;
			}
			else
			{
				ret = spell_identify(cn, co, in2);
			}
			break;
		// Blast?
		case    SK_CURSE:
			if (chance_base(cn, SK_CURSE, SP_MULT_CURSE, get_target_resistance(cn, co), power+100))
			{
				ret = 1;
			}
			else
			{
				ret = spell_curse(cn, co, power, 0);
			}
			break;
		case    SK_SLOW:
			if (chance_base(cn, SK_SLOW, SP_MULT_SLOW, get_target_resistance(cn, co), power+100))
			{
				ret = 1;
			}
			else
			{
				ret = spell_slow(cn, co, power, 0);
			}
			break;
		
		default:
			ret = 0;
	}

	if (ret)
	{
		charges--;
		it[in].stack = it[in].data[2] = charges;
		it[in].value = value * charges;
		if (charges<1)
		{
			return 1;
		}
	}

	return 0;
}

int use_blook_penta(int cn, int in)
{
	if (!cn)
	{
		return 0;
	}

	do_char_log(cn, 1, "You try to wipe off the blood, but it seems to be coming back slowly.\n");
	it[in].data[0] = 1;
	it[in].sprite[0] = it[in].data[1] + it[in].data[0];

	return 1;
}

int use_create_npc(int cn, int in)
{
	int co;

	if (it[in].active)
	{
		return 0;
	}
	if (!cn)
	{
		return 0;
	}

	co = pop_create_char(it[in].data[0], 0);
	if (!co)
	{
		return 0;
	}

	if (!god_drop_char_fuzzy(co, it[in].x, it[in].y))
	{
		god_destroy_items(co);
		ch[co].used = USE_EMPTY;
		return 0;
	}

	ch[co].data[0] = cn;

	return 1;
}

int use_rotate(int cn, int in)
{
	if (!cn)
	{
		return 0;
	}

	it[in].data[1]++;
	if (it[in].data[1]>3)
	{
		it[in].data[1] = 0;
	}
	it[in].sprite[0] = it[in].data[0] + it[in].data[1];
	it[in].flags |= IF_UPDATE;

	return 1;
}

/* CS, 991127: assemble lab8 key */
int use_lab8_key(int cn, int in)
{
	// data[0] = matching key part
	// data[1] = resulting key part
	// data[2] = (optional) other matching key part
	// data[3] = (optional) other resulting key part

	int in2, in3 = 0;

	if (cn == 0)
	{
		return 0;
	}

	in2 = ch[cn].citem;
	if (!in2 || (in2 & 0x80000000))
	{
		do_char_log(cn, 1, "Nothing happens.\n");
		return 0;
	}

	if (it[in].carried==0)
	{
		do_char_log(cn, 0, "Too difficult to do on the ground.\n");
		return 0;
	}

	// Check for one of the 1 or 2 matching parts
	if (it[in].data[0] == it[in2].temp)
	{
		in3 = it[in].data[1];
	}
	if (it[in].data[2] == it[in2].temp)
	{
		in3 = it[in].data[3];
	}
	if (in3 == 0)
	{
		do_char_log(cn, 0, "Those don't fit together.\n");
		return 0;
	}

	chlog(cn, "Added %s to %s", it[in2].name, it[in].name);

	god_take_from_char(in, cn);
	it[in].used = USE_EMPTY;

	ch[cn].citem = 0;
	it[in2].used = USE_EMPTY;

	in3 = god_create_item(in3);
	it[in3].flags |= IF_UPDATE;
	god_give_char(in3, cn);

	return 1;
}

/* CS, 991127: Shrines in Lab 8 */
int use_lab8_shrine(int cn, int in)
{
	// data[0] = item accepted as offering
	// data[1] = item returned as gift

	int offer, gift;

	if (cn == 0)
	{
		return 0;
	}

	if ((offer = ch[cn].citem) == 0)
	{
		do_char_log(cn, 1, "You get the feeling that it would be appropriate to give the Goddess a present.\n");
		return 0;
	}

	/* CS, 991130: Forgot to check for money here. <Slap Elrac> */
	if ((offer & 0x80000000) ||
	    (it[offer].temp != it[in].data[0]))
	{
		do_char_log(cn, 1, "The Goddess only wants her property back, and rejects your offer.\n");
		return 0;
	}

	ch[cn].citem = 0;
	it[offer].used = USE_EMPTY;

	chlog(cn, "Offered %s at %s", it[offer].reference, it[in].reference);

	gift = god_create_item(it[in].data[1]);
	if (!god_give_char(gift, cn))
	{
		ch[cn].citem = gift;
		it[gift].carried = cn;
	}
	do_char_log(cn, 1, "The Goddess has given you a %s in return!\n",
	            it[gift].reference);

	return 1;
}

/* CS, 991127: Shrines in Lab 8 */
int use_lab8_moneyshrine(int cn, int in)
{
	// data[0] = minimum offering accepted
	// data[1] = teleport coordinate x
	// data[2] = teleport coordinate y

	int offer;

	if (cn == 0)
	{
		return 0;
	}

	if ((offer = ch[cn].citem) == 0)
	{
		do_char_log(cn, 1, "You get the feeling that it would be appropriate to give the Goddess a present.\n");
		return 0;
	}

	if ((offer & 0x80000000) == 0)
	{
		do_char_log(cn, 1, "Only money is accepted at this shrine.\n");
		return 0;
	}

	offer &= 0x7fffffff;
	if (offer < it[in].data[0])
	{
		do_char_log(cn, 1, "Your offering is not sufficient, and was rejected.\n");
		return 0;
	}

	chlog(cn, "offered %dG at %s", offer / 100, it[in].reference);

	ch[cn].citem = 0;
	
	fx_add_effect(6, 0, ch[cn].x, ch[cn].y, 0);
	god_transfer_char(cn, it[in].data[1], it[in].data[2]);
	char_play_sound(cn, ch[cn].sound + 22, -150, 0);
	fx_add_effect(6, 0, ch[cn].x, ch[cn].y, 0);
	if (ch[cn].a_mana<ch[cn].mana[5] * 1000)
	{
		ch[cn].a_mana = ch[cn].mana[5] * 1000;
		do_char_log(cn, 0, "You feel the hand of the Goddess of Magic touch your mind.\n");
	}
	return 1;
}

void change_to_archtemplar(int cn)
{
	if (B_AT(cn, AT_AGL)<60)
	{
		do_char_log(cn, 1, "Your agility is too low. There is still room for improvement.\n");
		return;
	}
	if (B_AT(cn, AT_STR)<75)
	{
		do_char_log(cn, 1, "Your strength is too low. There is still room for improvement.\n");
		return;
	}

	if (IS_MALE(cn))	god_minor_racechange(cn, CT_ARCHTEMP_M);
	else				god_minor_racechange(cn, CT_ARCHTEMP_F);

	do_char_log(cn, 1, "You are truly worthy to become an Arch-Templar. Congratulations, %s.\n", ch[cn].name);
}

void change_to_pugilist(int cn)
{
	if (B_AT(cn, AT_AGL)<75)
	{
		do_char_log(cn, 1, "Your agility is too low. There is still room for improvement.\n");
		return;
	}
	if (B_AT(cn, AT_STR)<60)
	{
		do_char_log(cn, 1, "Your strength is too low. There is still room for improvement.\n");
		return;
	}

	if (IS_MALE(cn))	god_minor_racechange(cn, CT_PUGILIST_M);
	else				god_minor_racechange(cn, CT_PUGILIST_F);

	do_char_log(cn, 1, "You are truly worthy to become a Brawler. Congratulations, %s.\n", ch[cn].name);
}

void change_to_summoner(int cn)
{
	if (B_AT(cn, AT_WIL)<75)
	{
		do_char_log(cn, 1, "Your willpower is too low. There is still room for improvement.\n");
		return;
	}
	if (B_AT(cn, AT_INT)<60)
	{
		do_char_log(cn, 1, "Your intuition is too low. There is still room for improvement.\n");
		return;
	}

	if (IS_MALE(cn))	god_minor_racechange(cn, CT_SUMMONER_M);
	else				god_minor_racechange(cn, CT_SUMMONER_F);

	do_char_log(cn, 1, "You are truly worthy to become a Summoner. Congratulations, %s.\n", ch[cn].name);
}

void change_to_archharakim(int cn)
{
	if (B_AT(cn, AT_WIL)<60)
	{
		do_char_log(cn, 1, "Your willpower is too low. There is still room for improvement.\n");
		return;
	}
	if (B_AT(cn, AT_INT)<75)
	{
		do_char_log(cn, 1, "Your intuition is too low. There is still room for improvement.\n");
		return;
	}

	if (IS_MALE(cn))	god_minor_racechange(cn, CT_ARCHHARA_M);
	else				god_minor_racechange(cn, CT_ARCHHARA_F);

	do_char_log(cn, 1, "You are truly worthy to become an Arch-Harakim. Congratulations, %s.\n", ch[cn].name);
}

void change_to_warrior(int cn)
{
	if (B_AT(cn, AT_AGL)<60)
	{
		do_char_log(cn, 1, "Your agility is too low. There is still room for improvement.\n");
		return;
	}
	if (B_AT(cn, AT_STR)<60)
	{
		do_char_log(cn, 1, "Your strength is too low. There is still room for improvement.\n");
		return;
	}

	if (IS_MALE(cn))	god_minor_racechange(cn, CT_WARRIOR_M);
	else				god_minor_racechange(cn, CT_WARRIOR_F);

	do_char_log(cn, 1, "You are truly worthy to become a Warrior. Congratulations, %s.\n", ch[cn].name);
}

void change_to_sorcerer(int cn)
{
	if (B_AT(cn, AT_WIL)<60)
	{
		do_char_log(cn, 1, "Your willpower is too low. There is still room for improvement.\n");
		return;
	}
	if (B_AT(cn, AT_INT)<60)
	{
		do_char_log(cn, 1, "Your intuition is too low. There is still room for improvement.\n");
		return;
	}

	if (IS_MALE(cn))	god_minor_racechange(cn, CT_SORCERER_M);
	else				god_minor_racechange(cn, CT_SORCERER_F);

	do_char_log(cn, 1, "You are truly worthy to become a Sorcerer. Congratulations, %s.\n", ch[cn].name);
}

int shrine_of_change(int cn, int in)
{
	int n, in2, in3 = 0;

	if (!cn)
	{
		return 0;
	}

	if (!(in2 = ch[cn].citem) || in2 & 0x80000000)
	{
		do_char_log(cn, 1, "Read the notes, my friend.\n");
		return 0;
	}

	if (IS_ANY_ARCH(cn) && ((it[in2].temp>=IT_CH_FOOL && it[in2].temp<=IT_CH_WORLD) || 
		(it[in2].temp>=IT_CH_FOOL_R && it[in2].temp<=IT_CH_WORLD_R)))	// Tarot cards for slot 2
	{
		if (in3 = ch[cn].worn[WN_CHARM])
		{
			if (it[in3].temp == it[in2].temp)
			{
				do_char_log(cn, 0, "You can only have one of this tarot card equipped at a time.\n");
				return 0;
			}
		}
		if (it[in2].temp==IT_CH_FOOL) // special case - need to check equip requirements
		{
			if (do_check_fool(cn, in2)<1)
				return 0;
		}
		god_take_from_char(in2, cn);
		do_char_log(cn, 1, "You now have the effects of your %s equipped.\n", it[in2].name);
		fx_add_effect(6, 0, ch[cn].x, ch[cn].y, 0);
		if (in3 = ch[cn].worn[WN_CHARM2])
		{
			do_char_log(cn, 1, "Your %s was returned to you.\n", it[in3].name);
		}
		it[in2].x = 0;
		it[in2].y = 0;
		it[in2].carried = cn;
		ch[cn].citem = in3;
		ch[cn].worn[WN_CHARM2] = in2;
		if (n = ch[cn].data[PCD_COMPANION])  answer_transfer(cn, n, 0);
		if (n = ch[cn].data[PCD_SHADOWCOPY]) answer_transfer(cn, n, 0);
		remove_all_spells(cn);
		ch[cn].misc_action = DR_IDLE;
		return 0;
	}
	
	if (it[in2].temp==127 || it[in2].temp==274)     // greater healing potion
	{
		if (IS_TEMPLAR(cn))			change_to_archtemplar(cn);
		else if (IS_MERCENARY(cn))	change_to_warrior(cn);
		else if (IS_HARAKIM(cn))	change_to_summoner(cn);
		else do_char_log(cn, 1, "You've already arched!\n");
		return 0;
	}

	if (it[in2].temp==131 || it[in2].temp==273)     // greater mana potion
	{
		if (IS_TEMPLAR(cn))			change_to_pugilist(cn);
		else if (IS_MERCENARY(cn))	change_to_sorcerer(cn);
		else if (IS_HARAKIM(cn))	change_to_archharakim(cn);
		else do_char_log(cn, 1, "You've already arched!\n");
		return 0;
	}
	do_char_log(cn, 1, "Read the notes, my friend.\n");
	return 0;
}

int explorer_point(int cn, int in)
{
	int exp;

	if (!(ch[cn].data[46] & it[in].data[0]) && !(ch[cn].data[47] & it[in].data[1]) &&
	    !(ch[cn].data[48] & it[in].data[2]) && !(ch[cn].data[49] & it[in].data[3]) && 
		!(ch[cn].data[91] & it[in].data[5]))
	{

		ch[cn].data[46] |= it[in].data[0];
		ch[cn].data[47] |= it[in].data[1];
		ch[cn].data[48] |= it[in].data[2];
		ch[cn].data[49] |= it[in].data[3];
		ch[cn].data[91] |= it[in].data[5];

		do_char_log(cn, 0, "You found a new exploration point!\n");
		ch[cn].luck += 10;

		exp  = it[in].data[4];
		exp -= RANDOM(exp/10+1);
		exp  = min(ch[cn].points_tot / 10, exp);        // not more than 10% of total experience
		exp += RANDOM(exp/10+1);
		
		chlog(cn, "Found exp point, granting %d (of %d) exp", exp, it[in].data[4]);
		char_play_sound(cn, ch[cn].sound + 19, 0, 0);
		do_give_exp(cn, exp, 0, -1);
	}
	else
	{
		do_char_log(cn, 1, "Hmm. Seems somewhat familiar. You've been here before...\n");
	}

	return 1;
}

int way_point(int cn, int in)
{
	unsigned char buf[2];
	int nr;

	if (!(ch[cn].waypoints & it[in].data[0]))
	{
		ch[cn].waypoints |= it[in].data[0];

		do_char_log(cn, 0, "You found a new waypoint!\n");
		ch[cn].luck += 10;
		
		chlog(cn, "Found waypoint %d", it[in].data[0]);
		char_play_sound(cn, ch[cn].sound + 19, 0, 0);
	}
	
	nr = ch[cn].player;
	
	buf[0] = SV_WAYPOINTS;
	xsend(nr, buf, 1);

	return 1;
}

int use_garbage(int cn, int in)
{
	int in2, val;

	if (!cn)
	{
		return 0;
	}
	/* if no character, do nothing */

	if ((in2 = ch[cn].citem)==0)
	{
		do_char_log(cn, 1, "You feel that you could dispose of unwanted items in this disgusting garbage can.\n");
		return 0;
	}
	if (in2 & 0x80000000)
	{
		val = in2 & 0x7fffffff;
		ch[cn].citem = 0;

		do_char_log(cn, 1, "You disposed of %u gold and %u silver.\n", val / 100, val % 100);
	}
	else
	{
		ch[cn].citem = 0;
		it[in2].used = USE_EMPTY;
		/* JC, 000615: set item in hand to nothing, effectively destroying it */

		do_char_log(cn, 1, "You disposed of the %s.\n", it[in2].reference);
	}

	return 1;
}

void use_driver(int cn, int in, int carried)
{
	int ret, m;
	int thousand, tmp;

	if (in<=0 || in>=MAXITEM)
	{
		return;
	}
	if (cn<0 || cn>=MAXCHARS)
	{
		return;
	}
	if (cn && (ch[cn].flags & CF_BUILDMODE))
	{
		return;
	}

	if (cn && !carried)
	{
		ch[cn].cerrno = ERR_FAILED;             // will be overriden later if another result is desired

	}
	if ((it[in].flags & IF_USE) || !cn)
	{
		// check prerequisites (cost, minimum stats etc) !!!
		// remove cost !!!

		if (!carried)
		{
			m = it[in].x + it[in].y * MAPX;
			if (map[m].ch || map[m].to_ch)
			{
				return;
			}
		}

		if ((it[in].flags & IF_USESPECIAL) || it[in].temp==1705 || it[in].temp==1706)
		{
			switch(it[in].driver)
			{
			case 1:
				ret = use_create_item(cn, in);
				break;                                             // uses may-deactivate
			case 2:
				ret = use_door(cn, in);
				break;
			case 3:
				do_char_log(cn, 0, "You cannot use the lock-pick directly. Hold it under your mouse cursor and click on the door...\n");
				ret = 0;
				break;
			case 4:
				ret = use_mix_potion(cn, in);
				break;
			case 5:
				ret = stone_sword(cn, in);
				break;
			case 6:
				ret = teleport(cn, in);
				break;
			case 7:
				ret = use_bag(cn, in);
				break;
			case 8:
				ret = use_scroll(cn, in);
				break;                                        // skills
			case 9:
				ret = use_crystal(cn, in);
				break;
			case 10:
				ret = use_scroll2(cn, in);
				break;                                         // attribs
			case 11:
				ret = use_scroll3(cn, in);
				break;                                         // hp
			case 12:
				ret = use_scroll4(cn, in);
				break;                                         // endurance
			case 13:
				ret = use_scroll5(cn, in);
				break;                                         // mana
			case 14:
				ret = use_chain(cn, in);
				break;                                       // leather necklace + lizards teeth
			case 15:
				ret = use_labyrinth(cn, in);
				break;                                           // teleport into labyrinth special
			case 16:
				ret = use_ladder(cn, in);
				break;
			case 17:
				ret = rat_eye(cn, in);
				break;
			case 18:
				ret = skua_protect(cn, in);
				break;
			case 19:
				ret = use_lever(cn, in);
				break;
			case 20:
				ret = use_door(cn, in);
				break;
			case 21:
				ret = use_spawn(cn, in);
				break;
			case 22:
				ret = use_pile(cn, in);
				break;
			case 23:
				ret = teleport2(cn, in);
				break;
			case 24:
				ret = build_ring(cn, in);
				break;
			case 25:
				ret = use_mine(cn, in);
				break;
			case 26:
				ret = use_mine_fast(cn, in);
				break;
			case 27:
				ret = use_mine_respawn(cn, in);
				break;
			case 28:
				ret = use_gargoyle(cn, in);
				break;
			case 29:
				ret = use_grave(cn, in);
				break;
			case 30:
				ret = use_create_item2(cn, in);
				break;
			case 31:
				ret = 0;
				break;                        // empty, hole water
			case 32:
				ret = build_object(cn, in);
				break;
			case 33:
				ret = use_pentagram(cn, in);
				break;
			case 34:
				ret = use_seyan_shrine(cn, in);
				break;
			case 35:
				ret = use_seyan_door(cn, in);
				break;
			case 36:
				ret = 0;
				break;                        // magic portal 1 in lab13
			case 37:
				ret = 0;
				break;                        // traps
			case 38:
				ret = 0;
				break;                        // magic portal 2 in lab13
			case 39:
				ret = purple_protect(cn, in);
				break;
			case 40:
				ret = 0;
				break;                        // seyan'du sword
			case 41:
				ret = use_shrine(cn, in);
				break;
			case 42:
				ret = use_create_item3(cn, in);
				break;
			case 43:
				ret = 0;
				break;                        // spiderweb
			case 44:
				ret = use_kill_undead(cn, in);
				break;
			case 45:
				ret = use_seyan_portal(cn, in);
				break;
			case 46:
				ret = teleport3(cn, in);
				break;
			case 47:
				ret = 0;
				break;                        // arena portal
			case 48:
				ret = spell_scroll(cn, in);
				break;
			case 49:
				ret = use_blook_penta(cn, in);
				break;                                             // blood-penta
			case 50:
				ret = use_create_npc(cn, in);
				break;
			case 51:
				ret = use_rotate(cn, in);
				break;
			case 52:
				if (it[in].temp==IT_ANKHAMULET)
					ret = build_ankh(cn, in);
				else if (it[in].temp==IT_TW_SINBIND)
					ret = build_sinbinder(cn, in);
				else
					ret = 0;
				break;                        // personal item
			case 53:
				ret = use_create_item(cn, in);
				break;                                             // for personal items
			case 54:
				ret = use_create_item(cn, in);
				break;                                             // attack trigger
			case 55:
				ret = shrine_of_change(cn, in);
				break;
			case 56:
				ret = 0;
				break;                        // greenling green ball
			case 57:
				ret = explorer_point(cn, in);
				break;
			case 58:
				ret = use_grolm(cn, in);
				break;
			case 59:
				ret = use_create_gold(cn, in);
				break;
			/* CS, 991127: assembling Lab 8 key */
			case 61:
				ret = use_lab8_key(cn, in);
				break;
			/* CS, 991127: offering to shrines in Lab 8 */
			case 63:
				ret = use_lab8_shrine(cn, in);
				break;
			/* CS, 991127: offering money to shrine in Lab 8 */
			case 64:
				ret = use_lab8_moneyshrine(cn, in);
				break;
			/* CS, 991219: switches in lab9 */
			case 65:
				ret = use_lab9_switch(cn, in);
				break;
			/* CS, 991219: lab9 one way door */
			case 66:
				ret = use_lab9_door(cn, in);
				break;
			/* JC, 000615: garbage disposal */
			case 67:
				ret = use_garbage(cn, in);
				break;
			case 68:
				ret = use_soulstone(cn, in);
				break;
			case 69:
			case 70:
			case 71:
			case 72:
			case 73:
			case 74:
			case 75:
			case 76:
				ret = 0;
				break;
			case 77:						// walk-through like a door, but behaves like a ladder
				ret = use_ladder(cn, in);
				break;
			case 78:						// walk-through like a door, but behaves like a portal
				ret = teleport(cn, in);
				break;
			case 79:
			case 80:
			case 81:
			case 82:
			case 83:
			case 84:
			case 85:
			case 86:
			case 87:
				ret = 0;
				break;
			case 88:						// Waypoint object - use to turn on the WP bit
				ret = way_point(cn, in);
				break;
			case 89:						// Exp scroll
				ret = use_scroll6(cn, in);
				break;
			case 90:						// Luck scroll
				ret = use_scroll7(cn, in);
				break;
			case 91:						// Gold scroll
				ret = use_scroll8(cn, in);
				break;
			case 92:
				ret = use_soulfocus(cn, in);
				break;
			case 93:
				ret = use_soulcatalyst(cn, in);
				break;
			case 94:						// rewards on use while teleporting. /diffi sets reward tier in drdata[9]
				ret = reward_teleport(cn, in); 
				break;
			case 95:
			case 96:
			case 97:
			case 98:
			case 99:
			case 100:
				ret = 0;
				break;
			case 101:
				ret = use_toxic(cn, in);
				break;
			case 102:
				ret = place_ankh(cn, in);
				break;
			case 103:
				ret = use_recall_chest(cn, in);
				break;
			case 104:
			case 105:
				ret = 0;
				break;
			case 106:						// Gender scroll
				ret = use_scroll9(cn, in);
				break;
			case 107:
				ret = use_map_chest(cn, in);
				break;
			case 108:
				ret = use_map_shrine(cn, in);
				break;
			case 109:
				ret = use_map_portal(cn, in);
				break;
			case 110:
				ret = use_scrollA(cn, in);
				break;
			case 111:
				ret = use_map_artifact(cn, in);
				break;
			case 112:
				ret = use_map_pentigram(cn, in);
				break;
			case 113:
				ret = use_map_contract(cn, in);
				break;
			case 114:
				ret = use_special_spell(cn, in);
				break;
			case 115:
				ret = use_lame(cn, in);
				break;
			case 116:
				ret = use_scroll_R(cn, in);
				break;
			case 117:
				ret = use_scroll_S(cn, in);
				break;
			default:
				xlog("use_driver (use_driver.c): Unknown use_driver %d for item %d", it[in].driver, in);
				ret = 0;
				break;
			}
			if (cn)
			{
				if (!ret)
				{
					if (!carried)
					{
						ch[cn].cerrno = ERR_FAILED;
					}
					return;
				}
				else if (!carried)
				{
					ch[cn].cerrno = ERR_SUCCESS;
				}
			}
			do_update_char(cn);
		}
		if (!cn)
		{
			return;         // item_tick does activate and deactive as well

		}
		if (it[in].active)
		{
			if ((it[in].flags & IF_USEDEACTIVATE) || it[in].temp==1705 || it[in].temp==1706)
			{
				it[in].active = 0;
				if (it[in].light[0]!=it[in].light[1] && it[in].x>0)
				{
					do_add_light(it[in].x, it[in].y, it[in].light[0] - it[in].light[1]);
				}
				if (carried)
				{
					it[in].flags |= IF_UPDATE;
					do_update_char(cn);
				}
				if (cn && !carried)
				{
					ch[cn].cerrno = ERR_SUCCESS;
				}
			}
		}
		else
		{
			if ((it[in].flags & IF_USEACTIVATE) || it[in].temp==1705 || it[in].temp==1706)
			{
				it[in].active = it[in].duration;
				if (it[in].light[0]!=it[in].light[1] && it[in].x>0)
				{
					do_add_light(it[in].x, it[in].y, it[in].light[1] - it[in].light[0]);
				}
				if (carried)
				{
					it[in].flags |= IF_UPDATE;
					do_update_char(cn);
				}
				if (cn && !carried)
				{
					ch[cn].cerrno = ERR_SUCCESS;
				}
			}
			// Item stacking - held items are stackable and the same template
			if (carried && (it[in].flags & IF_STACKABLE) && ch[cn].citem && it[in].temp == it[ch[cn].citem].temp)
			{
				use_stack_items(cn, in);
			}
			else if (carried && (it[in].flags & IF_USEDESTROY))
			{
				thousand = 1000;

				if (it[in].min_rank>points2rank(ch[cn].points_tot))
				{
					do_char_log(cn, 0, "You're not experienced enough to use this.\n");
					return;
				}

				chlog(cn, "Used %s", it[in].name);
				
				// Active item effect
				if (it[in].duration)
				{
					if (!spell_from_item(cn, in)) return;
					do_update_char(cn);
				}
				
				if (get_book(cn, IT_BOOK_ALCH)) // Book: Alchemy 101
				{
					thousand = 1500;
				}
				
				// Special case for the Rainbow Potion
				// CGI Rainbow Potion heals 20, here we give a random ++(0-40)
				if (it[in].temp==IT_POT_RAIN)
				{
					ch[cn].a_hp += RANDOM(41) * thousand;
					ch[cn].a_end += RANDOM(41) * thousand;
					ch[cn].a_mana += RANDOM(41) * thousand;
				}
				
				// Immediate healing from item
				ch[cn].a_hp += it[in].hp[0] * thousand;
				if (ch[cn].a_hp<0)	 ch[cn].a_hp = 0;
				ch[cn].a_end += it[in].end[0] * thousand;
				if (ch[cn].a_end<0)	 ch[cn].a_end = 0;
				ch[cn].a_mana += it[in].mana[0] * thousand;
				if (ch[cn].a_mana<0) ch[cn].a_mana = 0;
				
				// Reduce stack or destroy item
				if ((it[in].flags & IF_STACKABLE) && it[in].stack > 1)
				{
					tmp = it[in].value / it[in].stack;
					it[in].stack--;
					it[in].value = tmp * it[in].stack;
				}
				else
				{
					it[in].used = USE_EMPTY;
					god_take_from_char(in, cn);
				}
				
				if (ch[cn].a_hp<500)
				{
					do_area_log(cn, 0, ch[cn].x, ch[cn].y, 0, "%s was killed by %s.\n",
					            ch[cn].name, it[in].reference);

					do_char_log(cn, 0, "You were killed by %s.\n",
					            it[in].reference);

					do_char_killed(0, cn, 0);
				}

				do_update_char(cn);
			}
		}
	}
}

int item_age(int in)
{
	int act, st;

	if (it[in].active)
	{
		act = 1;
	}
	else
	{
		act = 0;
	}

	if ((it[in].max_age[act] && it[in].current_age[act]>it[in].max_age[act]) ||
	    (it[in].max_damage && it[in].current_damage>it[in].max_damage))
	{
		it[in].flags |= IF_UPDATE;

		it[in].current_damage = 0;

		it[in].current_age[0] = 0;
		it[in].current_age[1] = 0;

		it[in].damage_state++;

		it[in].value /= 2;

		if (it[in].damage_state>1)
		{
			st = max(0, 4 - it[in].damage_state);

			if (it[in].armor[0]>st)
			{
				it[in].armor[0]--;
			}
			if (it[in].armor[1]>st)
			{
				it[in].armor[1]--;
			}

			if (it[in].weapon[0]>st * 2)
			{
				it[in].weapon[0]--;
				if (it[in].weapon[0]>0)
				{
					it[in].weapon[0]--;
				}
			}
			if (it[in].weapon[1]>st * 2)
			{
				it[in].weapon[1]--;
				if (it[in].weapon[1]>0)
				{
					it[in].weapon[1]--;
				}
			}
		}

		if (!(it[in].flags & IF_SINGLEAGE))
		{
			if (it[in].max_age[0])
			{
				it[in].sprite[0]++;
			}
			if (it[in].max_age[1])
			{
				it[in].sprite[1]++;
			}
		}

		return 1;
	}

	// expire no-age items in one jump after thirty minutes
	// lag scrolls vanish after 60 seconds
	if (!it[in].max_age[act] &&
	    (it[in].current_age[act]>TICKS * 60 * 30 ||
	     (it[in].temp==IT_LAGSCROLL && it[in].current_age[act]>TICKS * 60 * 2)))
	{
		it[in].damage_state = 5;
		return 1;
	}

	return 0;
}

void item_damage_worn(int cn, int n, int dam)
{
	int in;

	if ((in = ch[cn].worn[n])!=0 && it[in].max_damage)
	{
		it[in].current_damage += dam;
		if (item_age(in))
		{
			if (it[in].damage_state==1)
			{
				do_char_log(cn, 1, "The %s you are using is showing signs of use.\n",
				            it[in].reference);
			}
			else if (it[in].damage_state==2)
			{
				do_char_log(cn, 1, "The %s you are using was slightly damaged.\n",
				            it[in].reference);
			}
			else if (it[in].damage_state==3)
			{
				do_char_log(cn, 1, "The %s you are using was damaged.\n",
				            it[in].reference);
			}
			else if (it[in].damage_state==4)
			{
				do_char_log(cn, 0, "The %s you are using was badly damaged.\n",
				            it[in].reference);
			}
			else if (it[in].damage_state==5)
			{
				ch[cn].worn[n] = 0;
				it[in].used = USE_EMPTY;
				do_char_log(cn, 0, "The %s you were using was destroyed.\n",
				            it[in].reference);
			}
			do_update_char(cn);
		}
	}
}

void item_damage_citem(int cn, int dam)
{
	int in;

	if ((in = ch[cn].citem)!=0 && !(in & 0x80000000) && it[in].max_damage)
	{
		it[in].current_damage += dam;
		if (item_age(in))
		{
			if (it[in].damage_state==1)
			{
				do_char_log(cn, 1, "The %s you are using is showing signs of use.\n",
				            it[in].reference);
			}
			else if (it[in].damage_state==2)
			{
				do_char_log(cn, 1, "The %s you are using was slightly damaged.\n",
				            it[in].reference);
			}
			else if (it[in].damage_state==3)
			{
				do_char_log(cn, 1, "The %s you are using was damaged.\n",
				            it[in].reference);
			}
			else if (it[in].damage_state==4)
			{
				do_char_log(cn, 0, "The %s you are using was badly damaged.\n",
				            it[in].reference);
			}
			else if (it[in].damage_state==5)
			{
				ch[cn].citem = 0;
				it[in].used  = USE_EMPTY;
				do_char_log(cn, 0, "The %s you were using was destroyed.\n",
				            it[in].reference);
			}
		}
	}
}

void item_damage_armor(int cn, int dam)
{
	int n, in;
	
	for (n = 0; n<MAXBUFFS; n++)
	{
		if ((in = ch[cn].spell[n])!=0 && bu[in].temp==SK_MSHIELD)
		{
			dam -= bu[in].armor[1];
		}
	}
	
	dam = dam / 4 + 1;

	for (n = 0; n<20; n++)
	{
		if (n!=WN_RHAND && n!=WN_LHAND && n!=WN_CHARM && n!=WN_CHARM2)
		{
			if (RANDOM(3))
			{
				item_damage_worn(cn, n, dam);
			}
		}
		else if ((n==WN_LHAND && (it[ch[cn].worn[WN_LHAND]].flags & IF_OF_SHIELD)) || (n==WN_RHAND && (it[ch[cn].worn[WN_RHAND]].flags & IF_OF_SHIELD)))
		{
			if (RANDOM(3))
			{
				item_damage_worn(cn, n, dam);
			}
		}
	}
}

void item_damage_weapon(int cn, int dam)
{
	dam = dam / 4 + 1;
	if (it[ch[cn].worn[WN_LHAND]].flags & IF_OF_DUALSW)
	{
		if (RANDOM(2))
		{
			item_damage_worn(cn, WN_LHAND, dam);
			return;
		}
	}
	if (it[ch[cn].worn[WN_RHAND]].flags & IF_OF_SHIELD)
	{
		return;
	}
	item_damage_worn(cn, WN_RHAND, dam);
}

void lightage(int in, int multi)
{
	int m, cn, light, act;

	if ((cn = it[in].carried)!=0)
	{
		m = ch[cn].x + ch[cn].y * MAPX;
	}
	else
	{
		m = it[in].x + it[in].y * MAPX;
	}

	if (m<1 || m>=MAPX * MAPY)
	{
		return;
	}

	light = map[m].light;
	if (light<1)
	{
		return;
	}
	if (light>250)
	{
		light = 250;
	}

	light *= multi;

	if (it[in].active)
	{
		act = 1;
	}
	else
	{
		act = 0;
	}

	it[in].current_age[act] += light * 2;
}

/* CS, 991114: age messages for ice stuff and other stuff */
void age_message(int cn, int in, char *where)
{
	char *msg  = "whatzit";
	int   font = 1;

	if (it[in].driver == 60)
	{
		if (it[in].temp==986||it[in].temp==2442)
		{
			switch (it[in].damage_state) // onyx egg or cloak
			{
				case 1: msg = "The %s %s is beginning to crumble.\n"; break;
				case 2: msg = "The %s %s is crumbling fairly rapidly.\n"; break;
				case 3: msg = "The %s %s is crumbling down and pouring dust everywhere.\n"; break;
				case 4: font = 0; msg  = "The %s %s has crumbled down to a small sooty lump and large piles of dust.\n"; break;
				case 5: font = 0; msg  = "The %s %s has completely crumbled away, leaving you covered in dust.\n";
			}
		}
		else if (it[in].temp==585||it[in].temp==586)
		{
			switch (it[in].damage_state) // fire egg or cloak
			{
				case 1: msg = "The %s %s is beginning to smolder.\n"; break;
				case 2: msg = "The %s %s is smoldering fairly rapidly.\n"; break;
				case 3: msg = "The %s %s is smoldering down and pouring ash everywhere.\n"; break;
				case 4: font = 0; msg  = "The %s %s has smoldered down to a small glowing lump and large piles of ash.\n"; break;
				case 5: font = 0; msg  = "The %s %s has completely smoldered away, leaving you covered in soot.\n";
			}
		}
		else
		{
			switch (it[in].damage_state) // ice egg or cloak
			{
				case 1: msg = "The %s %s is beginning to melt.\n"; break;
				case 2: msg = "The %s %s is melting fairly rapidly.\n"; break;
				case 3: msg = "The %s %s is melting down and dripping water everywhere.\n"; break;
				case 4: font = 0; msg  = "The %s %s has melted down to a small icy lump and large puddles of water.\n"; break;
				case 5: font = 0; msg  = "The %s %s has completely melted away, leaving you all wet.\n";
			}
		}
	}
	else     // anything else
	{
		switch (it[in].damage_state)
		{
		case 1: msg = "The %s %s is showing signs of age.\n"; break;
		case 2: msg = "The %s %s is getting fairly old.\n"; break;
		case 3: msg = "The %s %s is getting old.\n"; break;
		case 4: font = 0; msg  = "The %s %s is getting very old and battered.\n"; break;
		case 5: font = 0; msg  = "The %s %s was so old and battered that it finally vanished.\n";
		}
	}
	do_char_log(cn, font, msg, it[in].reference, where);
}

void char_item_expire(int cn)
{
	int n, in, act;
	/* CS, 991114: Age ice cloak more slowly if not worn */
	static int clock4 = 0;
	int must_update = 0;

	if (IS_BUILDING(cn))
	{
		return;
	}

	clock4++;
	/* age items in backpack */
	for (n = 0; n<40; n++)
	{
		in = ch[cn].item[n];
		if (in)
		{
			if (it[in].active)
			{
				act = 1;
			}
			else
			{
				act = 0;
			}

			if ((act==0 && (it[in].flags & IF_ALWAYSEXP1)) || (act==1 && (it[in].flags & IF_ALWAYSEXP2)))
			{
				// fire/ice cloak ages more slowly when not worn or held
				if (it[in].driver == 60 && (clock4 % 4 != 0))
				{
					continue;
				}
				it[in].current_age[act]++;
				if (it[in].flags & IF_LIGHTAGE)
				{
					lightage(in, 1);
				}
				if (item_age(in))
				{
					must_update = 1;
					age_message(cn, in, "in your backpack");
					if (it[in].damage_state==5)
					{
						ch[cn].item[n] = 0;
						ch[cn].item_lock[n] = 0;
						it[in].used = USE_EMPTY;
					}
				}
			}
		}
	}

	/* age items in gear slots */
	for (n = 0; n<20; n++)
	{
		in = ch[cn].worn[n];
		if (in)
		{
			if (it[in].active)
			{
				act = 1;
			}
			else
			{
				act = 0;
			}

			if ((act==0 && (it[in].flags & IF_ALWAYSEXP1)) || (act==1 && (it[in].flags & IF_ALWAYSEXP2)))
			{
				it[in].current_age[act]++;
				if (it[in].flags & IF_LIGHTAGE)
				{
					lightage(in, 1);
				}
				if (item_age(in))
				{
					must_update = 1;
					if (it[in].damage_state==5)
					{
						age_message(cn, in, "you were using");
						ch[cn].worn[n] = 0;
						it[in].used = USE_EMPTY;
					}
					else
					{
						age_message(cn, in, "you are using");
					}
				}
			}
		}
	}
	
	/* age items in alt_set */
	for (n = 0; n<12; n++)
	{
		in = ch[cn].alt_worn[n];
		if (in)
		{
			if (it[in].active)
			{
				act = 1;
			}
			else
			{
				act = 0;
			}

			if ((act==0 && (it[in].flags & IF_ALWAYSEXP1)) || (act==1 && (it[in].flags & IF_ALWAYSEXP2)))
			{
				it[in].current_age[act]++;
				if (it[in].flags & IF_LIGHTAGE)
				{
					lightage(in, 1);
				}
				if (item_age(in))
				{
					must_update = 1;
					if (it[in].damage_state==5)
					{
						age_message(cn, in, "you had in your alt gear");
						ch[cn].worn[n] = 0;
						it[in].used = USE_EMPTY;
					}
					else
					{
						age_message(cn, in, "in your alt gear");
					}
				}
			}
		}
	}

	/* age item under mouse cursor (held) */
	in = ch[cn].citem;
	if (in && !(in & 0x80000000))
	{
		if (it[in].active)
		{
			act = 1;
		}
		else
		{
			act = 0;
		}

		if ((act==0 && (it[in].flags & IF_ALWAYSEXP1)) || (act==1 && (it[in].flags & IF_ALWAYSEXP2)))
		{
			it[in].current_age[act]++;
			if (it[in].flags & IF_LIGHTAGE)
			{
				lightage(in, 1);
			}
			if (item_age(in))
			{
				must_update = 1;
				if (it[in].damage_state==5)
				{
					age_message(cn, in, "you were using");
					ch[cn].citem = 0;
					it[in].used  = USE_EMPTY;
				}
				else
				{
					age_message(cn, in, "you are using");
				}
			}
		}
	}
	if (must_update)
	{
		do_update_char(cn);
	}
}

int may_deactivate(int in)
{
	int n, m, in2;

	if (it[in].driver!=1)
	{
		return 1;
	}

	for (n = 1; n<10; n++)
	{
		if ((m = it[in].data[n])==0)
		{
			return 1;
		}
		if ((in2 = map[m].it)==0)
		{
			return 0;
		}
		if (it[in2].driver!=26)
		{
			return 0;
		}
	}
	return 1;
}



void pentagram(int in)
{
	int n, cn;

	if (it[in].active)
	{
		return;
	}
	if (RANDOM(18))
	{
		return;
	}

	for (n = 1; n<4; n++)
	{
		if ((cn = it[in].data[n])==0 || ch[cn].data[0]!=in || ch[cn].used==USE_EMPTY || (ch[cn].flags & CF_BODY))
		{
			it[in].data[n] = spawn_penta_enemy(in);
			break;
		}
	}
}

/*
void spiderweb(int in)
{
	int n, cn;

	if (it[in].active)
	{
		return;
	}
	if (RANDOM(60))
	{
		return;
	}

	for (n = 1; n<4; n++)
	{
		if ((cn = it[in].data[n])==0 || ch[cn].data[0]!=in || ch[cn].used==USE_EMPTY || (ch[cn].flags & CF_BODY))
		{
			cn = pop_create_char(CT_SPIDER + RANDOM(3), 0);
			if (!cn)
			{
				continue;
			}
			ch[cn].flags &= ~CF_RESPAWN;
			ch[cn].data[0]  = in;
			ch[cn].data[29] = it[in].x + it[in].y * MAPX;
			ch[cn].data[60] = TICKS * 60 * 2;
			ch[cn].data[73] = 8;
			ch[cn].dir = 1;
			if (!god_drop_char_fuzzy(cn, it[in].x, it[in].y))
			{
				god_destroy_items(cn);
				ch[cn].used = USE_EMPTY;
			}
			else
			{
				it[in].data[n] = cn;
			}
			break;
		}
	}
}
*/

void enemyspawner(int in, int type, int flag)
{
	int n, cn;
	int t_ran = 20, count = 3, t_temp = 83, t_dist = 8, m_mult = 0;
	int wait = 0;

	switch (type)
	{
		case 0:			// Spider
			t_temp = CT_SPIDER + RANDOM(3);
			t_ran = 60;	count = 3;	t_dist = 8;	wait = 0;
			break;
		case 1:			// Ratling
			t_temp = CT_RATLING + it[in].data[0];
			t_ran = 40;	count = 1;	t_dist = 3;	wait = 0;
			break;
		case 2:			// Greenling
			t_temp = CT_GREENLING + it[in].data[0];
			t_ran = 30;	count = 2;	t_dist = 8;	wait = 0;
			break;
		case 3:			// Sogling
			t_temp = CT_DREADLING + it[in].data[0];
			t_ran = 20;	count = 3;	t_dist = 8;	wait = 0;
			break;
		case 4:			// Thrall / Vampire
			t_temp = CT_VAMPIRE + it[in].data[0];
			t_ran =  7;	count = 2;	t_dist = 8; wait = 0;
			break;
		case 5:			// Arachnid
			t_temp = 418;
			t_ran = 60;	count = 1;	t_dist = 8; wait = 0;
			break;
		case 6:			// Tarantula
			t_temp = 441;
			t_ran = 45;	count = 3;	t_dist = 8; wait = 0;
			break;
		case 7:			// Robber
			t_temp = 652;
			t_ran = 50;	count = 2;	t_dist = 8; wait = 0;
			break;
		case 8:			// Stone Golems (Forest Gorge)
			t_temp = CT_STONEGOLEM;
			t_ran = 50;	count = 3;	t_dist = 8;	wait = 1;
			break;
		case 9:			// Season Gorge Mobs
			t_temp = CT_SEASONSMOB + it[in].data[0];
			t_ran = 45;	count = 1;	t_dist = 8;	wait = 1;
			break;
		case 10:		// Ascent Gorge Mobs
			t_temp = CT_ASCENTMOB + it[in].data[0];
			t_ran = 40;	count = 1;	t_dist = 8;	wait = 1;
			break;
		case 11:		// Canyon Bandits
			t_temp = CT_CANYONBNDT;
			t_ran = 50;	count = 2;	t_dist = 8; wait = 0;
			break;
		case 12:		// Mud Golems
			t_temp = CT_MUDGOLEM;
			t_ran = 45;	count = 1;	t_dist = 8; wait = 0;
			break;
		case 13:		// Swamp Lizards
			t_temp = CT_SWAMPLIZ;
			t_ran = 35;	count = 2;	t_dist = 8;	wait = 1;
			break;
		case 14:		// Mountain Vikings
			t_temp = CT_MOUNTVIK;
			t_ran = 40;	count = 2;	t_dist = 8;	wait = 1;
			break;
		case 15:		// Gargoyle Nest
			t_temp = CT_GARGNEST + it[in].data[0];
			t_ran = 30;	count = 7;	t_dist =12; wait = 0;
			break;
		case 16:		// Gargoyle Nest Basement
			t_temp = CT_GARGNEST + it[in].data[0];
			t_ran = 30;	count = 2;	t_dist = 8; wait = 0;
			break;
		case 17:		// Autumn Xecko enemies
			t_temp = m_mult = CT_XECKO;
			t_ran = 25;	count = 2;	t_dist = 8; wait = 0;
			break;
		case 18:		// Ice Gargoyle Nest (either floor)			- 96
			t_temp = CT_ICENEST + it[in].data[0];
			t_ran = 30;	count = 2;	t_dist = 2; wait = 0;
			break;
		case 19:		// Ice Gargoyle Nest (either floor) (mixed)	- 97
			t_temp = m_mult = CT_ICENEST + it[in].data[0];
			t_ran = 30;	count = 2;	t_dist = 2; wait = 0;
			break;
		case 20:		// Cultists & Lycanthropes
			t_temp = CT_CULTIST + it[in].data[0];
			t_ran = 40;	count = 1;	t_dist = 8; wait = 0;
			break;
		case 21:		// Thugs
			t_temp = CT_THUGS;
			t_ran = 35;	count = 2;	t_dist =12; wait = 1;
			break;
		case 22:		// Lizards
			t_temp = CT_LIZARDKIN + it[in].data[0];
			t_ran = 28;	count = 1;	t_dist =10; wait = 0;
			break;
		case 23:		// Basalt mobs
			t_temp = CT_BASALT + it[in].data[0];
			t_ran = 24;	count = 2;	t_dist =10; wait = 1;
			break;
		case 24:		// Tower mobs
			t_temp = CT_TOWER + it[in].data[0];
			t_ran = 12;	count = 1;	t_dist = 0; wait = 1;
			break;
		default:
			return;
	}

	if (!flag && it[in].active)		return;
	if (!flag && RANDOM(t_ran))		return;
	
	for (n = 1; n<(count+1); n++)
	{
		if (m_mult && n>1) // force one of each of this mob type
		{
			t_temp = m_mult + (n-1);
		}
		cn = it[in].data[n];
		if (cn == 0 || ch[cn].data[0]!=in || ch[cn].used==USE_EMPTY || (ch[cn].flags & CF_BODY))
		{
			if (wait && (ch[cn].flags & CF_BODY) && ch[cn].used!=USE_EMPTY)
			{
				xlog("enemyspawner: waiting for body to decay (%d)", cn);
				continue;
			}
			cn = pop_create_char(t_temp, 0);
			if (!cn)
			{
				xlog("enemyspawner: pop_create_char failed (%d)", cn);
				continue;
			}
			ch[cn].flags &= ~CF_RESPAWN;
			ch[cn].flags &= ~CF_NOSLEEP;
			ch[cn].data[0]  = in;
			ch[cn].data[29] = it[in].x + it[in].y * MAPX;
			ch[cn].data[73] = t_dist;
			if (it[in].power)
			{
				ch[cn].data[30] = ch[cn].dir = it[in].power;
				ch[cn].data[60] = 0;
			}
			else
			{
				ch[cn].dir = 1;
				ch[cn].data[60] = TICKS * 60 * 2;
			}
			if (!god_drop_char_fuzzy(cn, it[in].x, it[in].y))
			{
				god_destroy_items(cn);
				ch[cn].used = USE_EMPTY;
				it[in].data[n] = 0;
				xlog("enemyspawner: failed to spawn at %d %d",it[in].x,it[in].y);
			}
			else
			{
				it[in].data[n] = cn;
				xlog("enemyspawner: spawned %s at %d %d",ch[cn].name,it[in].x,it[in].y);
			}
			break;
		}
		/*
		else if ((cn = it[in].data[n]) && ch[cn].used==USE_ACTIVE && ch[cn].temp != t_temp 
		&& ch[cn].temp != t_temp+1 && ch[cn].temp != t_temp+2 && ch[cn].temp != t_temp-1 && ch[cn].temp != t_temp-2)
		{
			it[in].data[n] = 0;
			xlog("enemyspawner: wiped bad template slot");
		}
		*/
	}
	for (n=count+1; n<9; n++)
	{
		it[in].data[n] = 0;
	}
}

void expire_blood_penta(int in)
{
	if (it[in].data[0])
	{
		it[in].data[0]++;
		if (it[in].data[0]>7)
		{
			it[in].data[0] = 0;
		}
		it[in].sprite[0] = it[in].data[1] + it[in].data[0];
	}
}

void expire_driver(int in)
{
	switch(it[in].driver)
	{
		case 49: expire_blood_penta(in); break;
		default: xlog("unknown expire driver %d for item %s (%d)\n", it[in].driver, it[in].name, in); break;
	}
}

#define EXP_TIME (MAPY/4)

void item_tick_expire(void)
{
	static int y = 0;
	int x, in, m, act, cn;

	for (x = 0, m = y * MAPX; x<MAPX; x++, m++)
	{
		if ((in = map[m].it)!=0)
		{
			if ((it[in].flags & IF_REACTIVATE) && !it[in].active)
			{
				if (!map[m].ch && !map[m].to_ch)
				{
					it[in].active = it[in].duration;
					if (it[in].light[0]!=it[in].light[1])
					{
						do_add_light(x, y, it[in].light[1] - it[in].light[0]);
					}
				}
			}
			// active and expire
			if (it[in].active && it[in].active!=0xffffffff)
			{
				if (it[in].active<=EXP_TIME)
				{
					if (may_deactivate(in) && !map[m].ch && !map[m].to_ch)
					{
						use_driver(0, in, 0);
						it[in].active = 0;
						if (it[in].light[0]!=it[in].light[1])
						{
							do_add_light(x, y, it[in].light[0] - it[in].light[1]);
						}
					}
				}
				else
				{
					it[in].active -= EXP_TIME;
				}
			}
			
			// Tick down door-jam
			if (it[in].driver==2 && it[in].data[5]>0)
			{
				it[in].data[5] -= EXP_TIME;
				if (it[in].data[5]<0)
					it[in].data[5] = 0;
			}

			// legacy drivers, replace by IF_EXPIREPROC!
			if (it[in].driver==33)
			{
				pentagram(in);
			}
			if (it[in].driver==43)
			{
				enemyspawner(in, 0, 0); //spiderweb(in);
			}
			if (it[in].driver==56)	enemyspawner(in, 2, 0);
			if (it[in].driver==71)	enemyspawner(in, 1, 0);
			if (it[in].driver==72)	enemyspawner(in, 3, 0);
			if (it[in].driver==73)	enemyspawner(in, 4, 0);
			if (it[in].driver==74)	enemyspawner(in, 5, 0);
			if (it[in].driver==75)	enemyspawner(in, 6, 0);
			if (it[in].driver==76)	enemyspawner(in, 7, 0);
			if (it[in].driver==79)	enemyspawner(in, 8, 0);
			if (it[in].driver==80)	enemyspawner(in, 9, 0);
			if (it[in].driver==81)	enemyspawner(in, 10, 0);
			if (it[in].driver==82)	enemyspawner(in, 11, 0);
			if (it[in].driver==83)	enemyspawner(in, 12, 0);
			if (it[in].driver==84)	enemyspawner(in, 13, 0);
			if (it[in].driver==85)	enemyspawner(in, 14, 0);
			if (it[in].driver==86)	enemyspawner(in, 15, 0);
			if (it[in].driver==87)	enemyspawner(in, 16, 0);
			if (it[in].driver==95)	enemyspawner(in, 17, 0);
			if (it[in].driver==96)	enemyspawner(in, 18, 0);
			if (it[in].driver==97)	enemyspawner(in, 19, 0);
			if (it[in].driver==98)	enemyspawner(in, 20, 0);
			if (it[in].driver==99)	enemyspawner(in, 21, 0);
			if (it[in].driver==100)	enemyspawner(in, 22, 0);
			if (it[in].driver==104)	enemyspawner(in, 23, 0);
			if (it[in].driver==105)	enemyspawner(in, 24, 0);

			if (it[in].flags & IF_EXPIREPROC)
			{
				expire_driver(in);
			}

			if (!(it[in].flags & IF_TAKE) && it[in].driver!=7)
			{
				goto noexpire;
			}
			if ((map[m].flags & MF_NOEXPIRE) && it[in].driver!=7 && 
				it[in].driver!=51 && !(it[in].flags & IF_IS_KEY)) // 51 is BS coin
			{
				goto noexpire;                                                  // yuck!
			}
			if (it[in].driver==37)
			{
				goto noexpire;
			}

			if (it[in].flags & IF_NOEXPIRE)
			{
				goto noexpire;
			}

			if (it[in].active)
			{
				act = 1;
			}
			else
			{
				act = 0;
			}

			it[in].current_age[act] += EXP_TIME;      // each place is only checked every MAPY ticks
			                                          // so we add MAPY instead of one

			if (it[in].flags & IF_LIGHTAGE)
			{
				lightage(in, EXP_TIME);
			}


			if (item_age(in) && it[in].damage_state==5)
			{
				if (it[in].light[act])
				{
					do_add_light(x, y, -it[in].light[act]);
				}
				map[m].it = 0;
				it[in].used = USE_EMPTY;
				globs->expire_cnt++;

				if (it[in].driver==7)   // tomb
				{
					int co, temp;

					co = it[in].data[0];
					temp = ch[co].temp;

					god_destroy_items(co);
					ch[co].used = USE_EMPTY;

					if (temp && (ch[co].flags & CF_RESPAWN))
					{
						// Eyeball kings take extra time to respawn
						if (temp==CT_RATKING || temp==CT_GREENKING || temp==CT_DREADKING || temp==CT_LIZEMPEROR ) 
						{
							fx_add_effect(2, TICKS * 60 * 12 + RANDOM(TICKS * 60 * 5), ch_temp[temp].x, ch_temp[temp].y, temp); // 12 - 17 minutes
						}
						else
						{
							fx_add_effect(2, TICKS * 60 * 1 + RANDOM(TICKS * 60 * 1), ch_temp[temp].x, ch_temp[temp].y, temp); // 1 - 2 minutes
						}
						xlog("respawn %d (%s): YES", co, ch[co].name);
					}
					else
					{
						xlog("respawn %d (%s): NO", co, ch[co].name);
					}
				}
			}
		}

noexpire:
		// checker
		if ((cn = map[m].ch)!=0)
		{
			if (ch[cn].x!=x || ch[cn].y!=y || ch[cn].used!=USE_ACTIVE)
			{
				xlog("map[%d,%d].ch reset from %d (%s) to 0", x, y, cn, ch[cn].reference);
				map[m].ch = 0;
				globs->lost_cnt++;
			}
		}
		if ((cn = map[m].to_ch)!=0)
		{
			if (ch[cn].tox!=x || ch[cn].toy!=y || ch[cn].used!=USE_ACTIVE)
			{
				xlog("map[%d,%d].to_ch reset from %d (%s) to 0", x, y, cn, ch[cn].reference);
				map[m].to_ch = 0;
				globs->lost_cnt++;
			}
		}
		if ((in = map[m].it)!=0)
		{
			if (it[in].x!=x || it[in].y!=y || it[in].used!=USE_ACTIVE)
			{
				xlog("map[%d,%d].it reset from %d (%s) to 0", x, y, in, it[in].reference);
				map[m].it = 0;
				globs->lost_cnt++;
			}
		}
	}

	y++;
	if (y>=MAPY)
	{
		globs->expire_run++;
		globs->lost_run++;
		y = 0;
	}
}

void item_tick_gc(void)
{
	static int off = 0, cnt = 0;
	int n, m, cn, z, in2;

	m = min(off + 256, MAXITEM);

	for (n = off; n<m; n++)
	{
		if (it[n].used==USE_EMPTY)
		{
			continue;
		}
		cnt++;

		// hack: make reset seyan swords unuable
		if (it[n].driver==40 && !it[n].data[0])
		{
			struct item tmp;

			tmp = it_temp[IT_DEADSEYSWORD];
			tmp.x = it[n].x;
			tmp.y = it[n].y;
			tmp.carried = it[n].carried;
			tmp.temp = IT_DEADSEYSWORD;
			it[n] = tmp;
			it[n].flags |= IF_UPDATE;

			cn = it[n].carried;

			if (cn)
			{
				do_update_char(cn);
			}

			xlog("reset sword from %s (%d)", ch[cn].name, cn);
		}

		if ((cn = it[n].carried)!=0)
		{
			if (IS_SANECHAR(cn) && ch[cn].used)
			{
				for (z = 0; z<40; z++)
				{
					if (ch[cn].item[z]==n)
					{
						break;
					}
				}
				if (z<40)
				{
					continue;
				}
				for (z = 0; z<20; z++)
				{
					if (ch[cn].worn[z]==n)
					{
						break;
					}
				}
				if (z<20)
				{
					continue;
				}
				for (z = 0; z<12; z++)
				{
					if (ch[cn].alt_worn[z]==n)
					{
						break;
					}
				}
				if (z<12)
				{
					continue;
				}
				for (z = 0; z<MAXBUFFS; z++)
				{
					if (ch[cn].spell[z]==n)
					{
						break;
					}
				}
				if (z<MAXBUFFS)
				{
					continue;
				}
				if (ch[cn].citem==n)
				{
					continue;
				}
				if (ch[cn].flags & CF_PLAYER)
				{
					for (z = 0; z<62; z++)
					{
						if (ch[cn].depot[z]==n)
						{
							break;
						}
					}
					if (z<62)
					{
						continue;
					}
				}
			}
		}
		else
		{
			in2 = map[it[n].x + it[n].y * MAPX].it;
			if (in2==n)
			{
				continue;
			}
		}
		xlog("Garbage: Item %d (%s) (%d, %d,%d)", n, it[n].reference, it[n].carried, it[n].x, it[n].y);
		it[n].used = USE_EMPTY;
		globs->gc_cnt++;
	}
	off += 256;
	if (off>=MAXITEM)
	{
		off = 0;
		globs->item_cnt = cnt;
		globs->gc_run++;
		cnt = 0;
	}
}

void item_tick(void)
{
	item_tick_expire();
	item_tick_expire();
	item_tick_expire();
	item_tick_expire();
	//item_tick_expire();
	//item_tick_expire();
	//item_tick_expire();
	//item_tick_expire();
	item_tick_gc();
}

void trap1(int cn, int in)
{
	int n, m=0, act=0, must_update=0;

	if ((n = it[in].data[1]))
	{
		in = map[n].it;
		if (!in || it[in].active || it[in].data[0])
		{
			do_char_log(cn, 0, "You stepped on a trap, but nothing happened!\n");
			return;
		}
	}
	
	do_char_log(cn, 0, "You triggered an acid trap.\n");
	chlog(cn, "Stepped on Acid Trap");
	
	/* age items in gear slots */
	for (n = 0; n<WN_CHARM2; n++)
	{
		if (n==WN_CHARM || n==WN_CHARM2) continue;
		
		in = ch[cn].worn[n];
		if (in)
		{
			m++;
			if (it[in].flags & IF_ALWAYSEXP2)
			{
				act = 1;
			}
			else
			{
				act = 0;
			}
			
			it[in].current_age[act]=it[in].max_age[act];
			it[in].current_age[act]++;
			if (item_age(in))
			{
				must_update=1;
				if (it[in].damage_state==5)
				{
					age_message(cn, in, "you were using");
					ch[cn].worn[n] = 0;
					it[in].used = USE_EMPTY;
				}
				else
				{
					age_message(cn, in, "you are using");
				}
			}
		}
	}
	
	if (m<11)
	{
		do_char_log(cn, 0, "It hit your skin, burning painfully.\n");
		do_hurt(0, cn, 350*(11-m), 0);
	}
	if (must_update)
	{
		do_update_char(cn);
	}
	/*
	n  = RANDOM(11);
	if (n==WN_CHARM) n=WN_RRING; // Can't go disintigrating tarot cards, that's just mean
	in = ch[cn].worn[n];

	if (in)
	{
		do_char_log(cn, 0, "You triggered an acid attack. Your %s desintegrated.\n", it[in].name);
		chlog(cn, "Stepped on Acid Trap, %s (t=%d) vanished", it[in].name, it[in].temp);
		it[in].used = USE_EMPTY;
		ch[cn].worn[n] = 0;
		do_update_char(cn);
	}
	else
	{
		do_char_log(cn, 0, "You triggered an acid attack, but it hit only your skin.\n");
		chlog(cn, "Stepped on Acid Trap");
		do_hurt(0, cn, 350, 0);
	}
	*/
}

void trap2(int cn, int tmp)
{
	int cc;

	cc = pop_create_char(tmp, 0);
	if (!god_drop_char_fuzzy(cc, ch[cn].x, ch[cn].y))
	{
		xlog("drop failed");
		ch[cc].used = USE_EMPTY;
		return;
	}
	do_update_char(cc);
	ch[cc].attack_cn = cn;
}

/* traps:
   data[0] = type
 */

void start_trap(int cn, int in)
{
	if (it[in].duration)
	{
		it[in].active = it[in].duration;
		if (it[in].light[0]!=it[in].light[1] && it[in].x>0)
		{
			do_add_light(it[in].x, it[in].y, it[in].light[1] - it[in].light[0]);
		}
	}

	switch(it[in].data[0])
	{
	case    0:
		chlog(cn, "Stepped on Arrow Trap");
		do_char_log(cn, 0, "You feel a sudden pain!\n");
		do_hurt(0, cn, 250, 0);
		break;

	case    1:
		chlog(cn, "Stepped on Attack Trigger Trap");
		do_char_log(cn, 0, "You hear a loud croaking noise!\n");
		do_area_notify(cn, 0, it[in].x, it[in].y, NT_HITME, cn, 0, 0, 0);
		break;

	case    2:
		trap1(cn, in);
		break;
	case    3:
		trap2(cn, CT_GOLEMTRAP); // Golem
		break;
	case    4:
		trap2(cn, CT_GARGTRAP); // Gargoyle
		break;
	case    5:
		chlog(cn, "Stepped on big Arrow Trap");
		do_char_log(cn, 0, "You feel a sudden pain!\n");
		do_hurt(0, cn, 666, 0);
		break;
	default:
		do_char_log(cn, 0, "Phew. Must be your lucky day today.\n");
		break;
	}
}

int step_trap(int cn, int in)
{
	if (ch[cn].flags & (CF_PLAYER))
	{
		start_trap(cn, in);
	}
	else
	{
		do_char_log(cn, 0, "You stepped on a trap. Fortunately, nothing happened.\n");
	}

	return 0;
}

void step_trap_remove(int cn, int in)
{
	if (it[in].active)
	{
		it[in].active = 0;
		if (it[in].light[0]!=it[in].light[1] && it[in].x>0)
		{
			do_add_light(it[in].x, it[in].y, it[in].light[0] - it[in].light[1]);
		}
	}
}

int step_portal1_lab13(int cn, int in)
{
	int flag = 0;
	int n;

	if (!IS_HARAKIM(cn) && !IS_TEMPLAR(cn) && !IS_MERCENARY(cn))
	{
		do_char_log(cn, 0, "This portal opens only for Templars, Mercenaries and Harakim.\n");
		return -1;
	}

	if (ch[cn].citem)
	{
		flag = 1;
	}

	for (n = 0; n<40 && !flag; n++)
	{
		if (ch[cn].item[n])
		{
			flag = 1;
		}
	}

	for (n = 0; n<12 && !flag; n++)
	{
		if (n==WN_CHARM||n==WN_CHARM2) continue;
		if (ch[cn].worn[n])
		{
			flag = 1;
		}
	}

	if (flag)
	{
		do_char_log(cn, 0, "You may not pass unless you leave all your items behind.\n");
		return -1;
	}

	for (n = 0; n<MAXBUFFS; n++)
	{
		if ((in = ch[cn].spell[n])!=0)
		{
			bu[in].used = USE_EMPTY;
			ch[cn].spell[n] = 0;
		}
	}
	do_update_char(cn);

	return 1;
}

int step_portal2_lab13(int cn, int in)
{
	int x, y, co, n, flag = 0, in2;

	if (!IS_PLAYER(cn))
	{
		return -1;
	}

	for (x = 137; x<=147 && !flag; x++) // Within the boundry of the victory walk
	{
		for (y = 194; y<=245 && !flag; y++)
		{
			if ((co = map[x + y * MAPX].ch)!=0 && co!=cn && (ch[co].flags & (CF_PLAYER)))
			{
				flag = 1;
			}
			// 664 is the key for XIII to turn seyan
			if ((in2 = map[x + y * MAPX].it)!=0 && (it[in2].temp==664 || it[in2].temp==IT_TOMBSTONE))
			{
				flag = 2;
			}
		}
	}

	for (x = 136; x<=148 && !flag; x++) // Within the boundry of the keeper room
	{
		for (y = 182; y<=193 && !flag; y++)
		{
			if ((co = map[x + y * MAPX].ch)!=0 && co!=cn && (ch[co].flags & (CF_PLAYER)))
			{
				flag = 1;
			}
			// 664 is the key for XIII to turn seyan
			if ((in2 = map[x + y * MAPX].it)!=0 && (it[in2].temp==664 || it[in2].temp==IT_TOMBSTONE))
			{
				flag = 2;
			}
		}
	}

	if (flag==2)
	{
		do_char_log(cn, 0, "The Final Test is waiting for a certain item to expire, please try again later.\n");
		return -1;
	}

	if (flag)
	{
		do_char_log(cn, 0, "You may not pass while another player is inside.\n");
		return -1;
	}

	for (n = flag = 0; n<MAXCHARS; n++)
	{
		if (ch[n].used!=USE_ACTIVE || (ch[n].flags & CF_BODY))
		{
			continue;
		}
		if (ch[n].temp!=CT_LAB13_KEEP) // Gatekeeper template
		{
			continue;
		}
		if (ch[n].a_hp>ch[n].hp[5] * 900 && ch[n].a_mana>ch[n].mana[5] * 900)
		{
			flag = 1;
		}
		break;
	}
	if (!flag)
	{
		do_char_log(cn, 0, "The Gatekeeper is currently busy. Please try again in a few minutes.\n");
		return -1;
	}

	if (!it[12042].data[1]) // coord of exit door
	{
		do_char_log(cn, 0, "The doors aren't closed again yet. Please try again in a few minutes.\n");
		return -1;
	}

	for (n = 0; n<MAXBUFFS; n++)
	{
		if ((in = ch[cn].spell[n])!=0)
		{
			bu[in].used = USE_EMPTY;
			ch[cn].spell[n] = 0;
		}
	}

	for (n = 0; n<40; n++)
	{
		if ((in2 = ch[cn].item[n])!=0 && it[in2].temp==664)
		{
			ch[cn].item[n] = 0;
			ch[cn].item_lock[n] = 0;
			it[in2].used = USE_EMPTY;
		}
	}
	if ((in2 = ch[cn].citem)!=0 && !(in2 & 0x80000000) && it[in2].temp==664)
	{
		ch[cn].citem = 0;
		it[in2].used = USE_EMPTY;
	}
	do_update_char(cn);

	return 1;
}

int step_portal_arena(int cn, int in)
{
	int nr, co, in2, n, flag = 0;
	int xs, ys, xe, ye, x, y;
	int money;
	
	if ((in2 = ch[cn].citem) && !(in2 & 0x80000000) && it[in2].temp==687)
	{
		ch[cn].citem = 0;
		it[in2].used = USE_EMPTY;
		flag = 1;
	}

	for (n = 0; n<40; n++)
	{
		if ((in2 = ch[cn].item[n])!=0 && it[in2].temp==687)
		{
			flag = 1;
			ch[cn].item[n] = 0;
			ch[cn].item_lock[n] = 0;
			it[in2].used = USE_EMPTY;
		}
	}

	if (flag)
	{
		do_char_log(cn, 1, "A winner! You gain one arena-rank!\n");
		ch[cn].data[22]++;
		ch[cn].data[23] = 1;
		money = ch[cn].data[22]*(ch[cn].data[22]+1)*67;
		ch[cn].gold += money;
		do_char_log(cn, 2, "You received %dG %dS.\n", money / 100, money % 100);
		return 1;
	}

	nr  = ch[cn].data[22];
	nr += CT_ARENA_FIRST;
	if (nr>CT_ARENA_LAST)
	{
		do_char_log(cn, 1, "Please tell the gods to add more potent monsters to the arena.\n");
		return -1;
	}

	xs = it[in].data[1] % MAPX;
	ys = it[in].data[1] / MAPX;
	xe = it[in].data[2] % MAPX;
	ye = it[in].data[2] / MAPX;

	if (ch[cn].frx>=xs && ch[cn].frx<=xe && ch[cn].fry>=ys && ch[cn].fry<=ye)
	{
		do_char_log(cn, 1, "You forfeit this fight.\n");
		return 1;
	}

	for (x = xs; x<=xe; x++)
	{
		for (y = ys; y<=ye; y++)
		{
			if (map[x + y * MAPX].ch)
			{
				do_char_log(cn, 1, "The arena is busy. Please come back later.\n");
				return -1;
			}
		}
	}

	co = pop_create_char(nr, 0);
	if (!god_drop_char_fuzzy(co, it[in].data[0] % MAPX, it[in].data[0] / MAPX))
	{
		do_char_log(cn, 1, "Please tell the gods that the arena isn't working.\n");
		return -1;
	}
	if (!(ch[co].flags & CF_EXTRAEXP)) ch[co].flags |= CF_EXTRAEXP;

	ch[co].data[64] = globs->ticker + TICKS * 60 * 5;

	in = god_create_item(IT_ARENANOTE);
	god_give_char(in, co);
	spell_light(co, co, 30);

	ch[cn].data[23] = 0;

	return 1;
}

/* CS, 991127: Teleport tile */
int step_teleport(int cn, int in)
{
	int m, x, y, j, m2, m3;
	int gc, sc;
	static int loc_off[] = { 0, -MAPX, MAPX, 1, -1 };

	if (cn <= 0)
	{
		xlog("step_teleport(): cn = %d", cn);
		return -1;
	}
	x = it[in].data[0];
	y = it[in].data[1];
	if (!SANEXY(x, y))
	{
		xlog("step_teleport(): bad coordinates in item %d", in);
		return -1;
	}
	gc = ch[cn].data[PCD_COMPANION];
	sc = ch[cn].data[PCD_SHADOWCOPY];
	m  = XY2M(x, y);
	m3 = 0;
	/* check for unoccupied landing spot */
	for (j = 0; j<ARRAYSIZE(loc_off); j++)
	{
		m2 = m + loc_off[j];
		if (map[m2].flags & MF_MOVEBLOCK)
		{
			continue;
		}
		if (map[m2].ch)
		{
			continue;
		}
		if (map[m2].to_ch)
		{
			continue;
		}
		if ((in = map[m2].it)!=0 && (it[in].flags & IF_MOVEBLOCK))
		{
			continue;
		}
		if (map[m2].flags & (MF_TAVERN | MF_DEATHTRAP))
		{
			continue;
		}
		m3 = m2;
		break;
	}
	if (m3 == 0)
	{
		// target occupied: fail silently
		return -1;
	}
	fx_add_effect(6, 0, ch[cn].x, ch[cn].y, 0);
	plr_map_remove(cn);

	/* mostly copied from plr_map_set() */
	ch[cn].status = 0;
	ch[cn].attack_cn = 0;
	ch[cn].skill_nr  = 0;
	ch[cn].goto_x = 0;


	// instead of plr_map_set(cn);
	map[m3].ch = cn;
	map[m3].to_ch = 0;
	ch[cn].x = m3 % MAPX;
	ch[cn].y = m3 / MAPX;
	do_area_notify(cn, 0, ch[cn].x, ch[cn].y, NT_SEE, cn, 0, 0, 0);

	fx_add_effect(6, 0, ch[cn].x, ch[cn].y, 0);
	
	if (ch[cn].flags & CF_GCTOME)
	{
		if (IS_SANECHAR(gc)) god_transfer_char(gc, m3 % MAPX, m3 / MAPX);
		if (IS_SANECHAR(sc)) god_transfer_char(sc, m3 % MAPX, m3 / MAPX);
	}
	
	return(2); // TELEPORT_SUCCESS
}

int use_toxic(int cn, int in)
{
	int in2;
	
	do_char_log(cn, 0, "As you touch the object, you suddenly feel dizzy and overwhelmed with nausia.\n");

	in2 = god_create_buff();

	strcpy(bu[in2].name, "Plague");
	strcpy(bu[in2].reference, "plague");
	strcpy(bu[in2].description, "Plague.");

	bu[in2].hp[0]  = -2500;
	bu[in2].end[0]  = -25;
	bu[in2].mana[0]  = -250;
	bu[in2].active = bu[in2].duration = 1;
	bu[in2].flags  = IF_SPELL | IF_PERMSPELL;
	bu[in2].temp = it[in].temp;
	bu[in2].sprite[1] = BUF_SPR_VENOM;

	add_spell(cn, in2);

	return 0;
}

int step_firefloor(int cn, int in)
{
	int in2;

	do_char_log(cn, 0, "Ouch!!\n");

	in2 = god_create_buff();

	strcpy(bu[in2].name, "Fire");
	strcpy(bu[in2].reference, "fire");
	strcpy(bu[in2].description, "Fire.");

	bu[in2].hp[0]  = -5000;
	bu[in2].active = bu[in2].duration = 1;
	bu[in2].flags  = IF_SPELL | IF_PERMSPELL;
	bu[in2].temp = it[in].temp;
	bu[in2].sprite[1] = BUF_SPR_FIRE;

	add_spell(cn, in2);

	return 0;
}

void step_firefloor_remove(int cn, int in)
{
	int n, in2;

	for (n = 0; n<MAXBUFFS; n++)
	{
		if ((in2 = ch[cn].spell[n]) && bu[in2].temp==it[in].temp)
		{
			break;
		}
	}
	if (n==MAXBUFFS)
	{
		return;
	}

	bu[in2].used = USE_EMPTY;
	ch[cn].spell[n] = 0;
}

int step_driver(int cn, int in)
{
	int ret = 0;
	
	// Only let players trigger floor objects
	if (!IS_PLAYER(cn))
	{
		return 0;
	}
	
	switch(it[in].driver)
	{
		case 36:
			ret = step_portal1_lab13(cn, in);
			break;
		case 37:
			ret = step_trap(cn, in);
			break;
		case 38:
			ret = step_portal2_lab13(cn, in);
			break;
		case 47:
			ret = step_portal_arena(cn, in);
			break;
		case 62:
			ret = step_teleport(cn, in);
			break;
		case 69:
			ret = step_firefloor(cn, in);
			break;
		default:
			xlog("unknown step driver %d for item %s (%d)", it[in].driver, it[in].name, in);
			break;
	}

	return(ret);
}

void step_driver_remove(int cn, int in)
{
	switch(it[in].driver)
	{
	case    36:
		break;
	case    37:
		step_trap_remove(cn, in);
		break;
	case    38:
		break;
	case    47:
		break;
	case    62:
		break;
	case    69:
		step_firefloor_remove(cn, in);
		break;
	default:
		xlog("unknown step driver %d for item %s (%d)", it[in].driver, it[in].name, in);
		break;
	}
}

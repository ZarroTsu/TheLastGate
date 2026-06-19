/*************************************************************************

   This file is part of 'Mercenaries of Astonia v2'
   Copyright (c) 1997-2001 Daniel Brockhaus (joker@astonia.com)
   All rights reserved.

 **************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "server.h"
#include "driver.h"

void plr_map_remove(int cn)     // remove character from map
{
	int m, in;

	m = ch[cn].x + ch[cn].y * MAPX;

	map[m].ch = 0;
	map[ch[cn].tox + ch[cn].toy * MAPX].to_ch = 0;

	if (ch[cn].light)
	{
		do_add_light(ch[cn].x, ch[cn].y, -ch[cn].light);
	}

	if (!(ch[cn].flags & CF_BODY))
	{
		if (IS_SANEITEM(in = map[m].it) && (it[in].flags & IF_STEPACTION))
		{
			step_driver_remove(cn, in);
		}
	}
}

int char_wears_item(int cn, int tmp)
{
	int in, n;

	for (n = 0; n<20; n++)
	{
		if ((in = ch[cn].worn[n])!=0 && it[in].temp==tmp)
		{
			return 1;
		}
	}

	return 0;
}

void step_desertfloor(int cn)
{
	int n, in2, in3;
	int alreadyhave = 0;

	for (n = 0; n<MAXBUFFS; n++)	if ((in3 = ch[cn].spell[n])!=0) // Check for coconut
	{
		if (bu[in3].temp==205) break;
		if (bu[in3].temp==206) alreadyhave = 1;
	}
	if (n==MAXBUFFS)
	{
		in2 = god_create_buff(206);
		strcpy(bu[in2].name, "Heatstroke");
		bu[in2].sprite = BUF_SPR_HEATSTR;
		bu[in2].r_hp   = -50;
		bu[in2].r_end  = -50;
		bu[in2].r_mana = -25;
		bu[in2].active = bu[in2].duration = TICKS*15;
		
		if (add_spell(cn, in2) && !alreadyhave) 
			do_char_log(cn, 0, "The heat is making you dizzy...\n");
	}
	return;
}

void step_desertfloor_remove(int cn)
{
	int n, in2;

	for (n = 0; n<MAXBUFFS; n++) if ((in2 = ch[cn].spell[n]) && bu[in2].temp==206) break;
	if (n==MAXBUFFS) return;

	bu[in2].active = bu[in2].duration = TICKS*15;
	bu[in2].flags &= ~(BF_PERMASPELL);
}

void step_vantablack(int cn)
{
	if (!has_buff(cn, 215))
	{
		remove_buff(cn, SK_LIGHT);
		debuff_vantablack(cn);
		do_char_log(cn, 0, "You feel a terrible curse wash over you.\n");
	}
}

void step_oppressed(int cn, int fl)
{
	int in, m;
	
	if ((in = has_buff(cn, SK_OPPRESSED2)))
	{
		if (bu[in].power != fl)
		{
			bu[in].power = fl;
			for (m = 0; m<5; m++) bu[in].attrib[m] = -(min(127, fl/2));
			for (m = 0; m<50; m++) bu[in].skill[m] = -(min(127, (fl+1)/2));
			do_char_log(cn, 0, "You feel the pressure on you shift.\n");
			do_update_char(cn);
		}
	}
	else
	{
		in = god_create_buff(SK_OPPRESSED2);
		
		strcpy(bu[in].name, "Oppressed");
		
		bu[in].power = fl;
		for (m = 0; m<5; m++) bu[in].attrib[m] = -(min(127, fl/2));
		for (m = 0; m<50; m++) bu[in].skill[m] = -(min(127, (fl+1)/2));
		
		bu[in].active = bu[in].duration = 1;
		bu[in].flags  = BF_PERMASPELL;
		bu[in].sprite = min(6780, 6761+fl-1);
		
		add_spell(cn, in);
		
		do_char_log(cn, 0, "You feel something press down on you.\n");
	}
}

void step_dw(int cn)
{
	int x, y, m, in=0, in2=0;
	
	if (!IS_SANEPLAYER(cn)) return;
	if (ch[cn].flags & CF_INVISIBLE) return;
	
	if ((in = has_buff(cn, SK_DWLIGHT))) ;
	else
	{
		if (has_buff(cn, SK_LIGHT)) remove_buff(cn, SK_LIGHT);
		debuff_dwlight(cn);
		return;
	}
	
	x = ch[cn].x;
	y = ch[cn].y;
	m = XY2M(x, y);
	
	if (IS_IN_INDW(x, y) && !(map[m].flags & MF_TOUCHED) && !(map[m].flags & MF_INDOORS))
	{
		map[m].flags   |= MF_TOUCHED;
		if (in && bu[in].power>1)
		{
			bu[in].power--;
			
			if (!(bu[in].power%21)) use_spawn_dw_mobs(cn, in);	// Spawn a mob to give chase
		}
		else if (in && bu[in].power==1)
		{
			bu[in].power = 0;
			bu[in].light = 0;
			do_update_char(cn);
			
			reset_go(ch[cn].x, ch[cn].y);
			remove_lights(ch[cn].x, ch[cn].y);
			
			if (can_drop(m))						;
			else if (can_drop(m + 1))				m += 1;
			else if (can_drop(m - 1))				m += -1;
			else if (can_drop(m + MAPX))			m += MAPX;
			else if (can_drop(m - MAPX))			m += -MAPX;
			else if (can_drop(m + 1 + MAPX))		m += 1 + MAPX;
			else if (can_drop(m + 1 - MAPX))		m += 1 - MAPX;
			else if (can_drop(m - 1 + MAPX))		m += -1 + MAPX;
			else if (can_drop(m - 1 - MAPX))		m += -1 - MAPX;
			else if (can_drop(m + 2))				m += 2;
			else if (can_drop(m - 2))				m += -2;
			else if (can_drop(m + 2 * MAPX))		m += 2 * MAPX;
			else if (can_drop(m - 2 * MAPX))		m += -2 * MAPX;
			else if (can_drop(m + 2 + MAPX))		m += 2 + MAPX;
			else if (can_drop(m + 2 - MAPX))		m += 2 - MAPX;
			else if (can_drop(m - 2 + MAPX))		m += -2 + MAPX;
			else if (can_drop(m - 2 - MAPX))		m += -2 - MAPX;
			else if (can_drop(m + 1 + 2 * MAPX))	m += 1 + 2 * MAPX;
			else if (can_drop(m + 1 - 2 * MAPX))	m += 1 - 2 * MAPX;
			else if (can_drop(m - 1 + 2 * MAPX))	m += -1 + 2 * MAPX;
			else if (can_drop(m - 1 - 2 * MAPX))	m += -1 - 2 * MAPX;
			else if (can_drop(m + 2 + 2 * MAPX))	m += 2 + 2 * MAPX;
			else if (can_drop(m + 2 - 2 * MAPX))	m += 2 - 2 * MAPX;
			else if (can_drop(m - 2 + 2 * MAPX))	m += -2 + 2 * MAPX;
			else if (can_drop(m - 2 - 2 * MAPX))	m += -2 - 2 * MAPX;
			
			fx_add_effect(12, 0, M2X(m), M2Y(m), 0);
			in2 = build_item(190, M2X(m), M2Y(m));
			it[in2].light[I_I] = 60;
			
			reset_go(ch[cn].x, ch[cn].y);
			add_lights(ch[cn].x, ch[cn].y);
			
			char_play_sound(cn, ch[cn].sound + 22, -150, 0);
			ch[cn].goto_x = 0;
			ch[cn].goto_y = 0;
			ch[cn].misc_action = 0;
		}
	}
	check_build_dw(cn);
}

void plr_map_set(int cn)        // set character to map and remove target character
{
	int m, n, in, ret, x, y;
	int to_x, to_y;
	int x_offset=0, y_offset=0;

	m = ch[cn].x + ch[cn].y * MAPX;

	if (!(ch[cn].flags & CF_BODY))
	{
		if (IS_SANEITEM(in = map[m].it) && (it[in].flags & IF_STEPACTION))
		{
			ret = step_driver(cn, in);
			if (ret==1)
			{
				map[m].to_ch = 0;
				
				x_offset = to_x = (ch[cn].x - ch[cn].frx);
				y_offset = to_y = (ch[cn].y - ch[cn].fry);
				
				if (to_x> 1) to_x= 1;
				if (to_x<-1) to_x=-1;
				if (to_y> 1) to_y= 1;
				if (to_y<-1) to_y=-1;
				
				x = ch[cn].x + to_x;
				y = ch[cn].y + to_y;

				if (!map[x + y * MAPX].ch)
				{
					ch[cn].x = x;
					ch[cn].y = y;

					map[ch[cn].x + ch[cn].y * MAPX].ch = cn;

					ch[cn].use_nr = 0;
					ch[cn].skill_nr  = 0;
					ch[cn].attack_cn = 0;
					ch[cn].goto_x = 0;
					ch[cn].goto_y = 0;
					ch[cn].misc_action = 0;
					
					if (ch[cn].light)
					{
						do_add_light(ch[cn].x, ch[cn].y, ch[cn].light);
					}
					
					/*
					//reset_go(ch[cn].x, ch[cn].y);
					remove_lights(ch[cn].x, ch[cn].y);
					//reset_go(ch[cn].x, ch[cn].y);
					add_lights(ch[cn].x, ch[cn].y);
					*/
					return;
				}
				else
				{
					ret = -1;
				}
			}
			if (ret==-1)
			{
				map[m].to_ch = 0;
				
				x_offset = (ch[cn].x - ch[cn].frx);
				y_offset = (ch[cn].y - ch[cn].fry);
				
				// Prevent disastrous accidents
				if (x_offset>1 || x_offset<-1 || y_offset>1 || y_offset<1)
				{
					ch[cn].x = ch[cn].temple_x;
					ch[cn].y = ch[cn].temple_y;
					do_char_log(cn,1,"Seems like you would have been flung into a wall. A god kindly returns you home.\n");
				}
				else
				{
					ch[cn].x = ch[cn].frx;
					ch[cn].y = ch[cn].fry;
				}
				
				map[ch[cn].x + ch[cn].y * MAPX].ch = cn;

				ch[cn].use_nr = 0;
				ch[cn].skill_nr  = 0;
				ch[cn].attack_cn = 0;
				ch[cn].goto_x = 0;
				ch[cn].goto_y = 0;
				ch[cn].misc_action = 0;
				
				if (ch[cn].light)
				{
					do_add_light(ch[cn].x, ch[cn].y, ch[cn].light);
				}
				return;
			}
			/* CS, 991127: Support for step_teleport() */
			if (ret==2)   // TELEPORT_SUCCESS
			{
				if (ch[cn].light)
				{
					do_add_light(ch[cn].x, ch[cn].y, ch[cn].light);
				}
				return;
			}
		}
		else if (IS_SANEPLAYER(cn) || IS_PLAYER_COMP(cn))
		{
			x = ch[cn].x;
			y = ch[cn].y;
			if (IS_IN_SUN(x, y))                step_desertfloor(cn); // Heatstroke for lab 6 & Volcano
			else if (IS_IN_VANTA(x, y))         step_vantablack(cn);
			else if (IS_IN_DW(x, y))            step_dw(cn);
			else if ((n = IS_IN_AQUE(x, y)))    step_oppressed(cn, n);
			//else if ((n = IS_IN_ABYSS(x, y))) step_oppressed(cn, n);
		}
		
		if ((map[m].flags & MF_TAVERN) && (ch[cn].flags & (CF_PLAYER)))
		{
			if (IS_BUILDING(cn))
			{
				god_build(cn, 0);
			}
			ch[cn].tavern_x = ch[cn].x;
			ch[cn].tavern_y = ch[cn].y;
			chlog(cn, "Entered tavern");
			plr_logout(cn, ch[cn].player, LO_TAVERN);
			return;
		}

		if (((map[m].flags & MF_NOMAGIC) && !do_get_iflag(cn, SF_AM_SUN)) || do_get_iflag(cn, SF_AM_MOON) ||
			(IS_MONSTER(cn) && IS_IN_XIX(ch[cn].x, ch[cn].y)))
		{
			if (!IS_NOMAGIC(cn))
			{
				ch[cn].flags |= CF_NOMAGIC;
				remove_spells(cn);
				do_char_log(cn, 0, "You feel your magic fail.\n");
			}
		}
		else
		{
			if (IS_NOMAGIC(cn))
			{
				ch[cn].flags &= ~CF_NOMAGIC;
				do_update_char(cn);
				do_char_log(cn, 0, "You feel your magic return.\n");
			}
		}
	}

	map[m].ch = cn;
	map[m].to_ch = 0;

	if (!(ch[cn].flags & CF_BODY))
	{
		if (ch[cn].light)
		{
			do_add_light(ch[cn].x, ch[cn].y, ch[cn].light);
		}
		if (map[m].flags & MF_DEATHTRAP)
		{
			if (try_lucksave(cn))
			{
				do_lucksave(cn, "deathtrap");
			}
			else
			{
				do_char_log(cn, 0, "You entered a Deathtrap. You are dead!\n");
				chlog(cn, "entered a Deathtrap");
				do_char_killed(0, cn, 0);
			}
			return;
		}
	}
	do_area_notify(cn, 0, ch[cn].x, ch[cn].y, NT_SEE, cn, 0, 0, 0);
}

void plr_move_up(int cn)
{
	plr_map_remove(cn);
	ch[cn].frx = ch[cn].x;
	ch[cn].fry = ch[cn].y;
	ch[cn].y--;
	ch[cn].tox = ch[cn].x;
	ch[cn].toy = ch[cn].y;
	plr_map_set(cn);
	ch[cn].cerrno = ERR_SUCCESS;
}

void plr_move_down(int cn)
{
	plr_map_remove(cn);
	ch[cn].frx = ch[cn].x;
	ch[cn].fry = ch[cn].y;
	ch[cn].y++;
	ch[cn].tox = ch[cn].x;
	ch[cn].toy = ch[cn].y;
	plr_map_set(cn);
	ch[cn].cerrno = ERR_SUCCESS;
}

void plr_move_left(int cn)
{
	plr_map_remove(cn);
	ch[cn].frx = ch[cn].x;
	ch[cn].fry = ch[cn].y;
	ch[cn].x--;
	ch[cn].tox = ch[cn].x;
	ch[cn].toy = ch[cn].y;
	plr_map_set(cn);
	ch[cn].cerrno = ERR_SUCCESS;
}

void plr_move_right(int cn)
{
	plr_map_remove(cn);
	ch[cn].frx = ch[cn].x;
	ch[cn].fry = ch[cn].y;
	ch[cn].x++;
	ch[cn].tox = ch[cn].x;
	ch[cn].toy = ch[cn].y;
	plr_map_set(cn);
	ch[cn].cerrno = ERR_SUCCESS;
}

void plr_move_leftup(int cn)
{
	plr_map_remove(cn);
	ch[cn].frx = ch[cn].x;
	ch[cn].fry = ch[cn].y;
	ch[cn].x--;
	ch[cn].y--;
	ch[cn].tox = ch[cn].x;
	ch[cn].toy = ch[cn].y;
	plr_map_set(cn);
	ch[cn].cerrno = ERR_SUCCESS;
}

void plr_move_leftdown(int cn)
{
	plr_map_remove(cn);
	ch[cn].frx = ch[cn].x;
	ch[cn].fry = ch[cn].y;
	ch[cn].x--;
	ch[cn].y++;
	ch[cn].tox = ch[cn].x;
	ch[cn].toy = ch[cn].y;
	plr_map_set(cn);
	ch[cn].cerrno = ERR_SUCCESS;
}

void plr_move_rightup(int cn)
{
	plr_map_remove(cn);
	ch[cn].frx = ch[cn].x;
	ch[cn].fry = ch[cn].y;
	ch[cn].x++;
	ch[cn].y--;
	ch[cn].tox = ch[cn].x;
	ch[cn].toy = ch[cn].y;
	plr_map_set(cn);
	ch[cn].cerrno = ERR_SUCCESS;
}

void plr_move_rightdown(int cn)
{
	plr_map_remove(cn);
	ch[cn].frx = ch[cn].x;
	ch[cn].fry = ch[cn].y;
	ch[cn].x++;
	ch[cn].y++;
	ch[cn].tox = ch[cn].x;
	ch[cn].toy = ch[cn].y;
	plr_map_set(cn);
	ch[cn].cerrno = ERR_SUCCESS;
}

void plr_turn_up(int cn)
{
	do_area_notify(cn, 0, ch[cn].x, ch[cn].y, NT_SEE, cn, 0, 0, 0);
	ch[cn].dir = DX_UP;
	ch[cn].cerrno = ERR_SUCCESS;
}

void plr_turn_leftup(int cn)
{
	do_area_notify(cn, 0, ch[cn].x, ch[cn].y, NT_SEE, cn, 0, 0, 0);
	ch[cn].dir = DX_LEFTUP;
	ch[cn].cerrno = ERR_SUCCESS;
}

void plr_turn_leftdown(int cn)
{
	do_area_notify(cn, 0, ch[cn].x, ch[cn].y, NT_SEE, cn, 0, 0, 0);
	ch[cn].dir = DX_LEFTDOWN;
	ch[cn].cerrno = ERR_SUCCESS;
}

void plr_turn_down(int cn)
{
	do_area_notify(cn, 0, ch[cn].x, ch[cn].y, NT_SEE, cn, 0, 0, 0);
	ch[cn].dir = DX_DOWN;
	ch[cn].cerrno = ERR_SUCCESS;
}

void plr_turn_rightdown(int cn)
{
	do_area_notify(cn, 0, ch[cn].x, ch[cn].y, NT_SEE, cn, 0, 0, 0);
	ch[cn].dir = DX_RIGHTDOWN;
	ch[cn].cerrno = ERR_SUCCESS;
}

void plr_turn_rightup(int cn)
{
	do_area_notify(cn, 0, ch[cn].x, ch[cn].y, NT_SEE, cn, 0, 0, 0);
	ch[cn].dir = DX_RIGHTUP;
	ch[cn].cerrno = ERR_SUCCESS;
}

void plr_turn_left(int cn)
{
	do_area_notify(cn, 0, ch[cn].x, ch[cn].y, NT_SEE, cn, 0, 0, 0);
	ch[cn].dir = DX_LEFT;
	ch[cn].cerrno = ERR_SUCCESS;
}

void plr_turn_right(int cn)
{
	do_area_notify(cn, 0, ch[cn].x, ch[cn].y, NT_SEE, cn, 0, 0, 0);
	ch[cn].dir = DX_RIGHT;
	ch[cn].cerrno = ERR_SUCCESS;
}

void plr_attack(int cn, int surround)
{
	int co, x, y;

	do_area_notify(cn, 0, ch[cn].x, ch[cn].y, NT_SEE, cn, 0, 0, 0);

	x = ch[cn].x;
	y = ch[cn].y;

	switch(ch[cn].dir)
	{
	case DX_UP:
		y--;
		break;
	case DX_DOWN:
		y++;
		break;
	case DX_LEFT:
		x--;
		break;
	case DX_RIGHT:
		x++;
		break;
	default:
		xlog("plr_attack (svr_act.c): unknown dir %d for char %d", ch[cn].dir, cn);
		ch[cn].cerrno = ERR_FAILED;
		return;
	}

	co = map[x + y * MAPX].ch;
	if (!co)
	{
		co = map[x + y * MAPX].to_ch;
	}
	if (!co)
	{
		co = ch[cn].attack_cn;
		if (ch[co].frx!=x || ch[co].fry!=y)
		{
			co = 0;
		}
	}
	if (!co)
	{
		do_char_log(cn, 2, "Your target moved away!\n");
		return;
	}

	if (ch[cn].attack_cn==co)
	{
		do_attack(cn, co, surround);
	}
}

void plr_give(int cn)
{
	int co, x, y;

	do_area_notify(cn, 0, ch[cn].x, ch[cn].y, NT_SEE, cn, 0, 0, 0);

	x = ch[cn].x;
	y = ch[cn].y;

	switch(ch[cn].dir)
	{
		case DX_UP:
			y--;
			break;
		case DX_DOWN:
			y++;
			break;
		case DX_LEFT:
			x--;
			break;
		case DX_RIGHT:
			x++;
			break;
		default:
			xlog("plr_give (svr_act.c): Unknown dir %d for char %d", ch[cn].dir, cn);
			ch[cn].cerrno = ERR_FAILED;
			return;
	}
	
	if (!(co = map[x + y * MAPX].ch))
		co = map[x + y * MAPX].to_ch;
	
	if (!co)
	{
		do_char_log(cn, 2, "Your target moved away!\n");
		return;
	}
	
	do_give(cn, co);
}

void plr_pickup(int cn)
{
	int m, in, in2, x, y, n;
	
	do_area_notify(cn, 0, ch[cn].x, ch[cn].y, NT_SEE, cn, 0, 0, 0);
	
	if (ch[cn].citem)
	{
		ch[cn].cerrno = ERR_FAILED;
		return;
	}
	
	if (ch[cn].dir==DX_UP && ch[cn].y>0)
	{
		m = ch[cn].x + ch[cn].y * MAPX - MAPX;
		x = ch[cn].x;
		y = ch[cn].y - 1;
	}
	else if (ch[cn].dir==DX_DOWN && ch[cn].y<MAPY - 1)
	{
		m = ch[cn].x + ch[cn].y * MAPX + MAPX;
		x = ch[cn].x;
		y = ch[cn].y + 1;
	}
	else if (ch[cn].dir==DX_LEFT && ch[cn].x>0)
	{
		m = ch[cn].x + ch[cn].y * MAPX - 1;
		x = ch[cn].x - 1;
		y = ch[cn].y;
	}
	else if (ch[cn].dir==DX_RIGHT && ch[cn].x<MAPX - 1)
	{
		m = ch[cn].x + ch[cn].y * MAPX + 1;
		x = ch[cn].x + 1;
		y = ch[cn].y;
	}
	else
	{
		ch[cn].cerrno = ERR_FAILED;
		return;
	}
	
	in = map[m].it;
	if (!in)
	{
		ch[cn].cerrno = ERR_FAILED;
		return;
	}
	if (!(it[in].flags & IF_TAKE))
	{
		ch[cn].cerrno = ERR_FAILED;
		return;
	}
	if (SEASON_CHECK(cn, in))
	{
		ch[cn].cerrno = ERR_FAILED;
		do_char_log(cn, 0, "This item is out of your season.\n");
		return;
	}
	
	ch[cn].cerrno = ERR_SUCCESS;
	do_update_char(cn);
	
	// support for money:
	if (it[in].flags & IF_MONEY)
	{
		ch[cn].gold += it[in].value;
		
		do_char_log(cn, 2, "You got %dG %dS\n", it[in].value / 100, it[in].value % 100);
		chlog(cn, "Took %dG %dS", it[in].value / 100, it[in].value % 100);
		
		map[m].it = 0;
		
		it[in].used = USE_EMPTY;
		it[in].x = 0;
		it[in].y = 0;
		
		if (it[in].active && it[in].light[I_A]) do_add_light(x, y, -it[in].light[I_A]);
		else if (it[in].light[I_I])             do_add_light(x, y, -it[in].light[I_I]);
		
		return;
	}
	
	map[m].it = 0;
	
	if (ch[cn].flags & (CF_PLAYER))
	{
		// Loop through and check if the item can be stacked with any existing item on the current page
		for (n = 0; n<MAXITEMS; n++) 
		{
			if ((in2 = ch[cn].item[n]))
			{
				if (god_stack_items(in, in2)==1) // All the picked-up items got stacked to an existing slot
				{
					chlog(cn, "Took %s", it[in].name);
					if (it[in].active && it[in].light[I_A]) do_add_light(x, y, -it[in].light[I_A]);
					else if (it[in].light[I_I])             do_add_light(x, y, -it[in].light[I_I]);
					return;
				}
			}
		}
		// Find an empty inventory slot for any remaining item
		for (n = 0; n<MAXITEMS; n++) if (!ch[cn].item[n]) break;
		if (n<MAXITEMS) ch[cn].item[n] = in;
		else            ch[cn].citem = in;
		chlog(cn, "Took %s", it[in].name);
	}
	else
	{
		ch[cn].citem = in;
	}
	
	it[in].x = 0;
	it[in].y = 0;
	it[in].carried = cn;
	
	if (it[in].active && it[in].light[I_A]) do_add_light(x, y, -it[in].light[I_A]);
	else if (it[in].light[I_I])             do_add_light(x, y, -it[in].light[I_I]);
}

void plr_bow(int cn)
{
	do_area_notify(cn, 0, ch[cn].x, ch[cn].y, NT_SEE, cn, 0, 0, 0);

	do_char_log(cn, 2, "You bow deeply.\n");
	do_area_log(cn, 0, ch[cn].x, ch[cn].y, 1, "%s bows deeply.\n", ch[cn].reference);
	chlog(cn, "Bows");
	ch[cn].cerrno = ERR_SUCCESS;
}

void plr_wave(int cn)
{
	do_area_notify(cn, 0, ch[cn].x, ch[cn].y, NT_SEE, cn, 0, 0, 0);

	do_char_log(cn, 2, "You wave happily.\n");
	do_area_log(cn, 0, ch[cn].x, ch[cn].y, 1, "%s waves happily.\n", ch[cn].reference);
	chlog(cn, "Waves");
	ch[cn].cerrno = ERR_SUCCESS;
}

void plr_use(int cn)
{
	int m, in; //, x, y;

	do_area_notify(cn, 0, ch[cn].x, ch[cn].y, NT_SEE, cn, 0, 0, 0);

	if (ch[cn].dir==DX_UP && ch[cn].y>0)
	{
		m = ch[cn].x + ch[cn].y * MAPX - MAPX;
//		x = ch[cn].x;
//		y = ch[cn].y - 1;
	}
	else if (ch[cn].dir==DX_DOWN && ch[cn].y<MAPY - 1)
	{
		m = ch[cn].x + ch[cn].y * MAPX + MAPX;
//		x = ch[cn].x;
//		y = ch[cn].y + 1;
	}
	else if (ch[cn].dir==DX_LEFT && ch[cn].x>0)
	{
		m = ch[cn].x + ch[cn].y * MAPX - 1;
//		x = ch[cn].x - 1;
//		y = ch[cn].y;
	}
	else if (ch[cn].dir==DX_RIGHT && ch[cn].x<MAPX - 1)
	{
		m = ch[cn].x + ch[cn].y * MAPX + 1;
//		x = ch[cn].x + 1;
//		y = ch[cn].y;
	}
	else
	{
		ch[cn].cerrno = ERR_FAILED;
		return;
	}

	in = map[m].it;
	if (!in)
	{
		ch[cn].cerrno = ERR_FAILED;
		return;
	}
	if (!(it[in].flags & (IF_USE | IF_USESPECIAL)))
	{
		ch[cn].cerrno = ERR_FAILED;
		return;
	}

	use_driver(cn, in, 0);    // use_driver sets errno
}

void plr_skill(int cn)
{
	do_area_notify(cn, 0, ch[cn].x, ch[cn].y, NT_SEE, cn, 0, 0, 0);

	skill_driver(cn, ch[cn].skill_target2);  // skill_driver sets errno
}

void plr_drop(int cn)
{
	int m, in, x, y, tmp, /*money = 0,*/ in2;

	do_area_notify(cn, 0, ch[cn].x, ch[cn].y, NT_SEE, cn, 0, 0, 0);

	in = ch[cn].citem;
	if (!in)
	{
		return;
	}

	if (ch[cn].dir==DX_UP && ch[cn].y>0)
	{
		m = ch[cn].x + ch[cn].y * MAPX - MAPX;
		x = ch[cn].x;
		y = ch[cn].y - 1;
	}
	else if (ch[cn].dir==DX_DOWN && ch[cn].y<MAPY - 1)
	{
		m = ch[cn].x + ch[cn].y * MAPX + MAPX;
		x = ch[cn].x;
		y = ch[cn].y + 1;
	}
	else if (ch[cn].dir==DX_LEFT && ch[cn].x>0)
	{
		m = ch[cn].x + ch[cn].y * MAPX - 1;
		x = ch[cn].x - 1;
		y = ch[cn].y;
	}
	else if (ch[cn].dir==DX_RIGHT && ch[cn].x<MAPX - 1)
	{
		m = ch[cn].x + ch[cn].y * MAPX + 1;
		x = ch[cn].x + 1;
		y = ch[cn].y;
	}
	else
	{
		ch[cn].cerrno = ERR_FAILED;
		return;
	}

	if (IS_SANEITEM(in2 = map[m].it) && (it[in2].flags & IF_STEPACTION))
	{
		step_driver(cn, in2);
		ch[cn].cerrno = ERR_FAILED;
		return;
	}

	if (!can_drop(m))
	{
		ch[cn].cerrno = ERR_FAILED;
		return;
	}
	
	if (is_atpandium(cn))
	{
		ch[cn].cerrno = ERR_FAILED;
		return;
	}

	ch[cn].citem = 0;

	ch[cn].cerrno = ERR_SUCCESS;
	do_update_char(cn);

	if (in & 0x80000000)
	{
		tmp = in & 0x7FFFFFFF;
		in  = god_create_item(1); // blank template
		if (!in)
		{
			ch[cn].cerrno = ERR_FAILED;
			return;
		}
		it[in].flags |= IF_TAKE | IF_LOOK | IF_MONEY;
		it[in].value  = tmp;
		strcpy(it[in].reference, "some money");
		if (tmp>999999)
		{
			strcpy(it[in].description, "A huge pile of gold coins");
			it[in].sprite[I_I] = 121;
		}
		else if (tmp>99999)
		{
			strcpy(it[in].description, "A very large pile of gold coins");
			it[in].sprite[I_I] = 120;
		}
		else if (tmp>9999)
		{
			strcpy(it[in].description, "A large pile of gold coins");
			it[in].sprite[I_I] = 41;
		}
		else if (tmp>999)
		{
			strcpy(it[in].description, "A small pile of gold coins");
			it[in].sprite[I_I] = 40;
		}
		else if (tmp>99)
		{
			strcpy(it[in].description, "Some gold coins");
			it[in].sprite[I_I] = 39;
		}
		else if (tmp>9)
		{
			strcpy(it[in].description, "A pile of silver coins");
			it[in].sprite[I_I] = 38;
		}
		else if (tmp>2)
		{
			strcpy(it[in].description, "A few silver coins");
			it[in].sprite[I_I] = 37;
		}
		else if (tmp==2)
		{
			strcpy(it[in].description, "A couple of silver coins");
			it[in].sprite[I_I] = 37;
		}
		else if (tmp==1)
		{
			strcpy(it[in].description, "A lonely silver coin");
			it[in].sprite[I_I] = 37;
		}
//		money = 1;
		
		chlog(cn, "Dropped %dG %dS", tmp / 100, tmp % 100);
	}
	else
	{
		if (!do_maygive(cn, 0, in))
		{
			do_char_log(cn, 0, "You are not allowed to do that!\n");
			ch[cn].citem  = in;
			ch[cn].cerrno = ERR_FAILED;
			return;
		}
		chlog(cn, "Dropped %s", it[in].name);
	}

	map[m].it = in;

	it[in].x = (short)x;
	it[in].y = (short)y;
	it[in].carried = 0;

	if (it[in].active)
	{
		if (it[in].light[I_A])
		{
			do_add_light(x, y, it[in].light[I_A]);
		}
	}
	else
	{
		if (it[in].light[I_I])
		{
			do_add_light(x, y, it[in].light[I_I]);
		}
	}
}

void plr_misc(int cn)
{
	int surround = 0;
	
	switch(ch[cn].status2)
	{
		case  0:
			if (IS_SEYA_OR_BRAV(cn)) surround = 3;
			plr_attack(cn, surround);
			break;
		case  1:
			plr_pickup(cn);
			break;
		case  2:
			plr_drop(cn);
			break;
		case  3:
			plr_give(cn);
			break;
		case  4:
			plr_use(cn);
			break;
		case  5:
			surround = 1;
			plr_attack(cn, surround);
			break;
		case  6:
			if (IS_BRAVER(cn))  surround = 3;
			if (IS_WARRIOR(cn)) surround = 2;
			if (IS_LYCANTH(cn)) surround = 3;
			plr_attack(cn, surround);
			break;
		case  7:
			plr_bow(cn);
			break;
		case  8:
			plr_wave(cn);
			break;
		case  9:
			plr_skill(cn);
			break;
		default:
			xlog("plr_misc (svr_act.c): unknown status2 %d for char %d", ch[cn].status2, cn);
			ch[cn].cerrno = ERR_FAILED;
			break;
	}
}

int plr_check_target(int m)
{
	int mt;
	if (map[m].ch || map[m].to_ch)
	{
		return 0;
	}

	if (map[m].flags & MF_MOVEBLOCK)
	{
		return 0;
	}

	if (IS_SANEITEM(mt = map[m].it) && (it[mt].flags & IF_MOVEBLOCK))
	{
		return 0;
	}

	return 1;
}

int plr_set_target(int m, int cn)
{
	if (!plr_check_target(m))
	{
		return 0;
	}

	map[m].to_ch = cn;

	return 1;
}


// -------------------
// interface functions
// -------------------

// Only these may be called from the outside!

void act_move_up(int cn)
{
	ch[cn].cerrno = ERR_NONE;

	if (ch[cn].y<1)
	{
		ch[cn].cerrno = ERR_FAILED;
		return;
	}
	if (ch[cn].dir!=DX_UP)
	{
		ch[cn].cerrno = ERR_FAILED;
		return;
	}

	if (IS_IN_XVIII(ch[cn].x, ch[cn].y) && has_item(cn, IT_COMMAND1)) // Lab 18 - Cannot move West
	{
		ch[cn].cerrno = ERR_FAILED;
		return;
	}

	if (!do_char_can_flee(cn))
	{
		ch[cn].cerrno = ERR_FAILED;
		return;
	}

	if (!plr_set_target(ch[cn].x + ch[cn].y * MAPX - MAPX, cn))
	{
		ch[cn].cerrno = ERR_FAILED;
		return;
	}

	ch[cn].status = 16;

	ch[cn].tox = ch[cn].x;
	ch[cn].toy = ch[cn].y - 1;
}

void act_move_down(int cn)
{
	ch[cn].cerrno = ERR_NONE;

	if (ch[cn].y>=MAPY - 2)
	{
		ch[cn].cerrno = ERR_FAILED;
		return;
	}
	if (ch[cn].dir!=DX_DOWN)
	{
		ch[cn].cerrno = ERR_FAILED;
		return;
	}

	if (!do_char_can_flee(cn))
	{
		ch[cn].cerrno = ERR_FAILED;
		return;
	}

	if (!plr_set_target(ch[cn].x + ch[cn].y * MAPX + MAPX, cn))
	{
		ch[cn].cerrno = ERR_FAILED;
		return;
	}

	ch[cn].status = 24;

	ch[cn].tox = ch[cn].x;
	ch[cn].toy = ch[cn].y + 1;
}

void act_move_left(int cn)
{
	ch[cn].cerrno = ERR_NONE;

	if (ch[cn].x<1)
	{
		ch[cn].cerrno = ERR_FAILED;
		return;
	}
	if (ch[cn].dir!=DX_LEFT)
	{
		ch[cn].cerrno = ERR_FAILED;
		return;
	}

	if (!do_char_can_flee(cn))
	{
		ch[cn].cerrno = ERR_FAILED;
		return;
	}

	if (!plr_set_target(ch[cn].x + ch[cn].y * MAPX - 1, cn))
	{
		ch[cn].cerrno = ERR_FAILED;
		return;
	}

	ch[cn].status = 32;

	ch[cn].tox = ch[cn].x - 1;
	ch[cn].toy = ch[cn].y;
}

void act_move_right(int cn)
{
	ch[cn].cerrno = ERR_NONE;

	if (ch[cn].x>=MAPX - 2)
	{
		ch[cn].cerrno = ERR_FAILED;
		return;
	}
	if (ch[cn].dir!=DX_RIGHT)
	{
		ch[cn].cerrno = ERR_FAILED;
		return;
	}

	if (!do_char_can_flee(cn))
	{
		ch[cn].cerrno = ERR_FAILED;
		return;
	}

	if (!plr_set_target(ch[cn].x + ch[cn].y * MAPX + 1, cn))
	{
		ch[cn].cerrno = ERR_FAILED;
		return;
	}

	ch[cn].status = 40;

	ch[cn].tox = ch[cn].x + 1;
	ch[cn].toy = ch[cn].y;
}

void act_move_leftup(int cn)
{
	ch[cn].cerrno = ERR_NONE;

	if (ch[cn].x<1)
	{
		ch[cn].cerrno = ERR_FAILED;
		return;
	}
	if (ch[cn].y<1)
	{
		ch[cn].cerrno = ERR_FAILED;
		return;
	}
	if (ch[cn].dir!=DX_LEFTUP)
	{
		ch[cn].cerrno = ERR_FAILED;
		return;
	}
	
	if (IS_IN_XVIII(ch[cn].x, ch[cn].y) && has_item(cn, IT_COMMAND1)) // Lab 18 - Cannot move West
	{
		ch[cn].cerrno = ERR_FAILED;
		return;
	}

	if (!do_char_can_flee(cn))
	{
		ch[cn].cerrno = ERR_FAILED;
		return;
	}

	if (!plr_check_target(ch[cn].x + ch[cn].y * MAPX - MAPX))
	{
		ch[cn].cerrno = ERR_FAILED;
		return;
	}
	if (!plr_check_target(ch[cn].x + ch[cn].y * MAPX - 1))
	{
		ch[cn].cerrno = ERR_FAILED;
		return;
	}
	if (!plr_set_target(ch[cn].x + ch[cn].y * MAPX - MAPX - 1, cn))
	{
		ch[cn].cerrno = ERR_FAILED;
		return;
	}

	ch[cn].status = 48;

	ch[cn].tox = ch[cn].x - 1;
	ch[cn].toy = ch[cn].y - 1;
}

void act_move_leftdown(int cn)
{
	ch[cn].cerrno = ERR_NONE;

	if (ch[cn].x<1)
	{
		ch[cn].cerrno = ERR_FAILED;
		return;
	}
	if (ch[cn].y>=MAPY - 2)
	{
		ch[cn].cerrno = ERR_FAILED;
		return;
	}
	if (ch[cn].dir!=DX_LEFTDOWN)
	{
		ch[cn].cerrno = ERR_FAILED;
		return;
	}

	if (!do_char_can_flee(cn))
	{
		ch[cn].cerrno = ERR_FAILED;
		return;
	}

	if (!plr_check_target(ch[cn].x + ch[cn].y * MAPX + MAPX))
	{
		ch[cn].cerrno = ERR_FAILED;
		return;
	}
	if (!plr_check_target(ch[cn].x + ch[cn].y * MAPX - 1))
	{
		ch[cn].cerrno = ERR_FAILED;
		return;
	}
	if (!plr_set_target(ch[cn].x + ch[cn].y * MAPX + MAPX - 1, cn))
	{
		ch[cn].cerrno = ERR_FAILED;
		return;
	}

	ch[cn].status = 60;

	ch[cn].tox = ch[cn].x - 1;
	ch[cn].toy = ch[cn].y + 1;
}

void act_move_rightup(int cn)
{
	ch[cn].cerrno = ERR_NONE;

	if (ch[cn].x>=MAPX - 2)
	{
		ch[cn].cerrno = ERR_FAILED;
		return;
	}
	if (ch[cn].y<1)
	{
		ch[cn].cerrno = ERR_FAILED;
		return;
	}
	if (ch[cn].dir!=DX_RIGHTUP)
	{
		ch[cn].cerrno = ERR_FAILED;
		return;
	}
	
	if (IS_IN_XVIII(ch[cn].x, ch[cn].y) && has_item(cn, IT_COMMAND1)) // Lab 18 - Cannot move West
	{
		ch[cn].cerrno = ERR_FAILED;
		return;
	}

	if (!do_char_can_flee(cn))
	{
		ch[cn].cerrno = ERR_FAILED;
		return;
	}

	if (!plr_check_target(ch[cn].x + ch[cn].y * MAPX - MAPX))
	{
		ch[cn].cerrno = ERR_FAILED;
		return;
	}
	if (!plr_check_target(ch[cn].x + ch[cn].y * MAPX + 1))
	{
		ch[cn].cerrno = ERR_FAILED;
		return;
	}
	if (!plr_set_target(ch[cn].x + ch[cn].y * MAPX - MAPX + 1, cn))
	{
		ch[cn].cerrno = ERR_FAILED;
		return;
	}

	ch[cn].status = 72;

	ch[cn].tox = ch[cn].x + 1;
	ch[cn].toy = ch[cn].y - 1;
}

void act_move_rightdown(int cn)
{
	ch[cn].cerrno = ERR_NONE;

	if (ch[cn].x>=MAPX - 2)
	{
		ch[cn].cerrno = ERR_FAILED;
		return;
	}
	if (ch[cn].y>=MAPY - 2)
	{
		ch[cn].cerrno = ERR_FAILED;
		return;
	}
	if (ch[cn].dir!=DX_RIGHTDOWN)
	{
		ch[cn].cerrno = ERR_FAILED;
		return;
	}

	if (!do_char_can_flee(cn))
	{
		ch[cn].cerrno = ERR_FAILED;
		return;
	}

	if (!plr_check_target(ch[cn].x + ch[cn].y * MAPX + MAPX))
	{
		ch[cn].cerrno = ERR_FAILED;
		return;
	}
	if (!plr_check_target(ch[cn].x + ch[cn].y * MAPX + 1))
	{
		ch[cn].cerrno = ERR_FAILED;
		return;
	}
	if (!plr_set_target(ch[cn].x + ch[cn].y * MAPX + MAPX + 1, cn))
	{
		ch[cn].cerrno = ERR_FAILED;
		return;
	}

	ch[cn].status = 84;

	ch[cn].tox = ch[cn].x + 1;
	ch[cn].toy = ch[cn].y + 1;
}

void act_turn_up(int cn)
{
	ch[cn].cerrno = ERR_NONE;

	if (ch[cn].dir==DX_DOWN)
	{
		act_turn_rightdown(cn);
	}
	else if (ch[cn].dir==DX_LEFTDOWN)
	{
		act_turn_left(cn);
	}
	else if (ch[cn].dir==DX_RIGHTDOWN)
	{
		act_turn_right(cn);
	}
	else if (ch[cn].dir==DX_LEFT)
	{
		act_turn_leftup(cn);
	}
	else if (ch[cn].dir==DX_RIGHT)
	{
		act_turn_rightup(cn);
	}
	else if (ch[cn].dir==DX_LEFTUP)
	{
		ch[cn].status = 132;
	}
	else
	{
		ch[cn].status = 148; // dir=DX_RIGHT and any possible insane value...
	}
}

void act_turn_down(int cn)
{
	ch[cn].cerrno = ERR_NONE;

	if (ch[cn].dir==DX_UP)
	{
		act_turn_leftup(cn);
	}
	else if (ch[cn].dir==DX_LEFTUP)
	{
		act_turn_left(cn);
	}
	else if (ch[cn].dir==DX_RIGHTUP)
	{
		act_turn_right(cn);
	}
	else if (ch[cn].dir==DX_LEFT)
	{
		act_turn_leftdown(cn);
	}
	else if (ch[cn].dir==DX_RIGHT)
	{
		act_turn_rightdown(cn);
	}
	else if (ch[cn].dir==DX_LEFTDOWN)
	{
		ch[cn].status = 140;
	}
	else
	{
		ch[cn].status = 156; // dir=DX_RIGHT and any possible insane value...
	}
}

void act_turn_left(int cn)
{
	ch[cn].cerrno = ERR_NONE;

	if (ch[cn].dir==DX_RIGHT)
	{
		act_turn_rightup(cn);
	}
	else if (ch[cn].dir==DX_RIGHTUP)
	{
		act_turn_up(cn);
	}
	else if (ch[cn].dir==DX_RIGHTDOWN)
	{
		act_turn_down(cn);
	}
	else if (ch[cn].dir==DX_UP)
	{
		act_turn_leftup(cn);
	}
	else if (ch[cn].dir==DX_DOWN)
	{
		act_turn_leftdown(cn);
	}
	else if (ch[cn].dir==DX_LEFTUP)
	{
		ch[cn].status = 100;
	}
	else
	{
		ch[cn].status = 116; // dir=DX_DOWN and any possible insane value...
	}
}

void act_turn_right(int cn)
{
	ch[cn].cerrno = ERR_NONE;

	if (ch[cn].dir==DX_LEFT)
	{
		act_turn_leftdown(cn);
	}
	else if (ch[cn].dir==DX_LEFTUP)
	{
		act_turn_up(cn);
	}
	else if (ch[cn].dir==DX_LEFTDOWN)
	{
		act_turn_down(cn);
	}
	else if (ch[cn].dir==DX_UP)
	{
		act_turn_rightup(cn);
	}
	else if (ch[cn].dir==DX_DOWN)
	{
		act_turn_rightdown(cn);
	}
	else if (ch[cn].dir==DX_RIGHTUP)
	{
		ch[cn].status = 108;
	}
	else
	{
		ch[cn].status = 124; // dir=DX_DOWN and any possible insane value...
	}
}

void act_turn_leftup(int cn)
{
	ch[cn].cerrno = ERR_NONE;

	if (ch[cn].dir==DX_RIGHTDOWN)
	{
		act_turn_down(cn);
	}
	else if (ch[cn].dir==DX_DOWN)
	{
		act_turn_leftdown(cn);
	}
	else if (ch[cn].dir==DX_RIGHT)
	{
		act_turn_rightup(cn);
	}
	else if (ch[cn].dir==DX_RIGHTUP)
	{
		act_turn_up(cn);
	}
	else if (ch[cn].dir==DX_LEFTDOWN)
	{
		act_turn_left(cn);
	}
	else if (ch[cn].dir==DX_UP)
	{
		ch[cn].status = 96;
	}
	else
	{
		ch[cn].status = 128; // dir=DX_LEFT and any possible insane value...
	}
}

void act_turn_leftdown(int cn)
{
	ch[cn].cerrno = ERR_NONE;

	if (ch[cn].dir==DX_RIGHTUP)
	{
		act_turn_up(cn);
	}
	else if (ch[cn].dir==DX_UP)
	{
		act_turn_leftup(cn);
	}
	else if (ch[cn].dir==DX_RIGHT)
	{
		act_turn_rightdown(cn);
	}
	else if (ch[cn].dir==DX_RIGHTDOWN)
	{
		act_turn_down(cn);
	}
	else if (ch[cn].dir==DX_LEFTUP)
	{
		act_turn_left(cn);
	}
	else if (ch[cn].dir==DX_DOWN)
	{
		ch[cn].status = 112;
	}
	else
	{
		ch[cn].status = 136; // dir=DX_LEFT and any possible insane value...
	}
}

void act_turn_rightup(int cn)
{
	ch[cn].cerrno = ERR_NONE;

	if (ch[cn].dir==DX_LEFTDOWN)
	{
		act_turn_down(cn);
	}
	else if (ch[cn].dir==DX_DOWN)
	{
		act_turn_rightdown(cn);
	}
	else if (ch[cn].dir==DX_LEFT)
	{
		act_turn_leftup(cn);
	}
	else if (ch[cn].dir==DX_LEFTUP)
	{
		act_turn_up(cn);
	}
	else if (ch[cn].dir==DX_RIGHTDOWN)
	{
		act_turn_right(cn);
	}
	else if (ch[cn].dir==DX_UP)
	{
		ch[cn].status = 104;
	}
	else
	{
		ch[cn].status = 144; // dir=DX_RIGHT and any possible insane value...
	}
}

void act_turn_rightdown(int cn)
{
	ch[cn].cerrno = ERR_NONE;

	if (ch[cn].dir==DX_LEFTUP)
	{
		act_turn_up(cn);
	}
	else if (ch[cn].dir==DX_UP)
	{
		act_turn_rightup(cn);
	}
	else if (ch[cn].dir==DX_LEFT)
	{
		act_turn_leftdown(cn);
	}
	else if (ch[cn].dir==DX_LEFTDOWN)
	{
		act_turn_down(cn);
	}
	else if (ch[cn].dir==DX_RIGHTUP)
	{
		act_turn_right(cn);
	}
	else if (ch[cn].dir==DX_DOWN)
	{
		ch[cn].status = 120;
	}
	else
	{
		ch[cn].status = 152; // dir=DX_LEFT and any possible insane value...
	}
}

void act_turn(int cn, int dir)
{
	ch[cn].cerrno = ERR_NONE;

	if (ch[cn].dir==dir)
	{
		ch[cn].cerrno = ERR_SUCCESS;
		return;
	}

	switch(dir)
	{
	case DX_UP:
		act_turn_up(cn);
		break;
	case DX_DOWN:
		act_turn_down(cn);
		break;
	case DX_RIGHT:
		act_turn_right(cn);
		break;
	case DX_LEFT:
		act_turn_left(cn);
		break;
	case DX_LEFTUP:
		act_turn_leftup(cn);
		break;
	case DX_LEFTDOWN:
		act_turn_leftdown(cn);
		break;
	case DX_RIGHTUP:
		act_turn_rightup(cn);
		break;
	case DX_RIGHTDOWN:
		act_turn_rightdown(cn);
		break;
	default:
		xlog("act_turn (svr_act.c): unknown dir %d for char %d", dir, cn);
		ch[cn].cerrno = ERR_FAILED;
		break;
	}
}

void act_attack(int cn)         // attack character in front of the character
{
	int v;

	ch[cn].cerrno = ERR_NONE;

	if (!(ch[cn].flags & CF_SIMPLE))
	{
		do
		{
			v = RANDOM(3);
		}
		while (ch[cn].lastattack==v);
		ch[cn].lastattack = v;
		if (v)
		{
			v += 4;
		}
	}
	else
	{
		v = 0;
	}

	switch(ch[cn].dir)
	{
	case    DX_UP:
		if (ch[cn].x>0)
		{
			ch[cn].status  = 160;
			ch[cn].status2 = v;
		}
		else
		{
			ch[cn].cerrno = ERR_FAILED;
		} break;
	case    DX_DOWN:
		if (ch[cn].x<MAPX - 1)
		{
			ch[cn].status  = 168;
			ch[cn].status2 = v;
		}
		else
		{
			ch[cn].cerrno = ERR_FAILED;
		} break;
	case    DX_LEFT:
		if (ch[cn].y>0)
		{
			ch[cn].status  = 176;
			ch[cn].status2 = v;
		}
		else
		{
			ch[cn].cerrno = ERR_FAILED;
		} break;
	case    DX_RIGHT:
		if (ch[cn].y<MAPY - 1)
		{
			ch[cn].status  = 184;
			ch[cn].status2 = v;
		}
		else
		{
			ch[cn].cerrno = ERR_FAILED;
		} break;
	default:
		ch[cn].cerrno = ERR_FAILED;
		break;
	}
}

void act_give(int cn)   // give current object to character in front of the character
{
	if ((ch[cn].flags & CF_SIMPLE))
	{
		ch[cn].cerrno = ERR_FAILED;
		return;
	}

	ch[cn].cerrno = ERR_NONE;

	switch(ch[cn].dir)
	{
	case    DX_UP:
		if (ch[cn].x>0)
		{
			ch[cn].status  = 160;
			ch[cn].status2 = 3;
		}
		else
		{
			ch[cn].cerrno = ERR_FAILED;
		} break;
	case    DX_DOWN:
		if (ch[cn].x<MAPX - 1)
		{
			ch[cn].status  = 168;
			ch[cn].status2 = 3;
		}
		else
		{
			ch[cn].cerrno = ERR_FAILED;
		} break;
	case    DX_LEFT:
		if (ch[cn].y>0)
		{
			ch[cn].status  = 176;
			ch[cn].status2 = 3;
		}
		else
		{
			ch[cn].cerrno = ERR_FAILED;
		} break;
	case    DX_RIGHT:
		if (ch[cn].y<MAPY - 1)
		{
			ch[cn].status  = 184;
			ch[cn].status2 = 3;
		}
		else
		{
			ch[cn].cerrno = ERR_FAILED;
		} break;
	default:
		ch[cn].cerrno = ERR_FAILED;
		break;
	}
}

void act_bow(int cn)
{
	if ((ch[cn].flags & CF_SIMPLE))
	{
		ch[cn].cerrno = ERR_FAILED;
		return;
	}

	ch[cn].cerrno = ERR_NONE;

	switch(ch[cn].dir)
	{
	case    DX_UP:
		if (ch[cn].x>0)
		{
			ch[cn].status  = 160;
			ch[cn].status2 = 7;
		}
		else
		{
			ch[cn].cerrno = ERR_FAILED;
		} break;
	case    DX_DOWN:
		if (ch[cn].x<MAPX - 1)
		{
			ch[cn].status  = 168;
			ch[cn].status2 = 7;
		}
		else
		{
			ch[cn].cerrno = ERR_FAILED;
		} break;
	case    DX_LEFT:
		if (ch[cn].y>0)
		{
			ch[cn].status  = 176;
			ch[cn].status2 = 7;
		}
		else
		{
			ch[cn].cerrno = ERR_FAILED;
		} break;
	case    DX_RIGHT:
		if (ch[cn].y<MAPY - 1)
		{
			ch[cn].status  = 184;
			ch[cn].status2 = 7;
		}
		else
		{
			ch[cn].cerrno = ERR_FAILED;
		} break;
	default:
		ch[cn].cerrno = ERR_FAILED;
		break;
	}
}

void act_wave(int cn)
{
	if ((ch[cn].flags & CF_SIMPLE))
	{
		ch[cn].cerrno = ERR_FAILED;
		return;
	}

	ch[cn].cerrno = ERR_NONE;

	switch(ch[cn].dir)
	{
	case    DX_UP:
		if (ch[cn].x>0)
		{
			ch[cn].status  = 160;
			ch[cn].status2 = 8;
		}
		else
		{
			ch[cn].cerrno = ERR_FAILED;
		} break;
	case    DX_DOWN:
		if (ch[cn].x<MAPX - 1)
		{
			ch[cn].status  = 168;
			ch[cn].status2 = 8;
		}
		else
		{
			ch[cn].cerrno = ERR_FAILED;
		} break;
	case    DX_LEFT:
		if (ch[cn].y>0)
		{
			ch[cn].status  = 176;
			ch[cn].status2 = 8;
		}
		else
		{
			ch[cn].cerrno = ERR_FAILED;
		} break;
	case    DX_RIGHT:
		if (ch[cn].y<MAPY - 1)
		{
			ch[cn].status  = 184;
			ch[cn].status2 = 8;
		}
		else
		{
			ch[cn].cerrno = ERR_FAILED;
		} break;
	default:
		ch[cn].cerrno = ERR_FAILED;
		break;
	}
}

void act_skill(int cn)
{
	if ((ch[cn].flags & CF_SIMPLE))
	{
		ch[cn].cerrno = ERR_FAILED;
		skill_driver(cn, ch[cn].skill_nr);
		return;
	}

	ch[cn].cerrno = ERR_NONE;

	switch(ch[cn].dir)
	{
	case    DX_UP:
		ch[cn].status  = 160;
		ch[cn].status2 = 9;
		return;
	case    DX_DOWN:
		ch[cn].status  = 168;
		ch[cn].status2 = 9;
		return;
	case    DX_LEFT:
		ch[cn].status  = 176;
		ch[cn].status2 = 9;
		return;
	case    DX_RIGHT:
		ch[cn].status  = 184;
		ch[cn].status2 = 9;
		return;
	}
	ch[cn].cerrno = ERR_FAILED;
}

void act_pickup(int cn)         // get the object in front of the character
{
	if ((ch[cn].flags & CF_SIMPLE))
	{
		ch[cn].cerrno = ERR_FAILED;
		return;
	}

	ch[cn].cerrno = ERR_NONE;

	if (!do_char_can_flee(cn)  || (ch[cn].flags & CF_SIMPLE))
	{
		ch[cn].cerrno = ERR_FAILED;
		return;
	}

	switch(ch[cn].dir)
	{
	case    DX_UP:
		if (ch[cn].y>0)
		{
			ch[cn].status  = 160;
			ch[cn].status2 = 1;
		}
		else
		{
			ch[cn].cerrno = ERR_FAILED;
		} break;
	case    DX_DOWN:
		if (ch[cn].y<MAPY - 1)
		{
			ch[cn].status  = 168;
			ch[cn].status2 = 1;
		}
		else
		{
			ch[cn].cerrno = ERR_FAILED;
		} break;
	case    DX_LEFT:
		if (ch[cn].x>0)
		{
			ch[cn].status  = 176;
			ch[cn].status2 = 1;
		}
		else
		{
			ch[cn].cerrno = ERR_FAILED;
		} break;
	case    DX_RIGHT:
		if (ch[cn].x<MAPX - 1)
		{
			ch[cn].status  = 184;
			ch[cn].status2 = 1;
		}
		else
		{
			ch[cn].cerrno = ERR_FAILED;
		} break;
	default:
		ch[cn].cerrno = ERR_FAILED;
		break;
	}
}

void act_use(int cn)            // get the object in front of the character
{
	ch[cn].cerrno = ERR_NONE;

	if (!do_char_can_flee(cn) || (ch[cn].flags & CF_SIMPLE))
	{
		ch[cn].cerrno = ERR_FAILED;
		return;
	}

	switch(ch[cn].dir)
	{
	case    DX_UP:
		if (ch[cn].y>0)
		{
			ch[cn].status  = 160;
			ch[cn].status2 = 4;
		}
		else
		{
			ch[cn].cerrno = ERR_FAILED;
		} break;
	case    DX_DOWN:
		if (ch[cn].y<MAPY - 1)
		{
			ch[cn].status  = 168;
			ch[cn].status2 = 4;
		}
		else
		{
			ch[cn].cerrno = ERR_FAILED;
		} break;
	case    DX_LEFT:
		if (ch[cn].x>0)
		{
			ch[cn].status  = 176;
			ch[cn].status2 = 4;
		}
		else
		{
			ch[cn].cerrno = ERR_FAILED;
		} break;
	case    DX_RIGHT:
		if (ch[cn].x<MAPX - 1)
		{
			ch[cn].status  = 184;
			ch[cn].status2 = 4;
		}
		else
		{
			ch[cn].cerrno = ERR_FAILED;
		} break;
	default:
		ch[cn].cerrno = ERR_FAILED;
		break;
	}
}

void act_drop(int cn)  // drops the current object in front of the character
{
	ch[cn].cerrno = ERR_NONE;

	if (!do_char_can_flee(cn) || (ch[cn].flags & CF_SIMPLE))
	{
		ch[cn].cerrno = ERR_FAILED;
		return;
	}
	
	if (IS_IN_DW(ch[cn].x, ch[cn].y))
	{
		ch[cn].cerrno = ERR_FAILED;
		return;
	}

	switch(ch[cn].dir)
	{
	case    DX_UP:
		if (ch[cn].y>0)
		{
			ch[cn].status  = 160;
			ch[cn].status2 = 2;
		}
		else
		{
			ch[cn].cerrno = ERR_FAILED;
		} break;
	case    DX_DOWN:
		if (ch[cn].y<MAPY - 1)
		{
			ch[cn].status  = 168;
			ch[cn].status2 = 2;
		}
		else
		{
			ch[cn].cerrno = ERR_FAILED;
		} break;
	case    DX_LEFT:
		if (ch[cn].x>0)
		{
			ch[cn].status  = 176;
			ch[cn].status2 = 2;
		}
		else
		{
			ch[cn].cerrno = ERR_FAILED;
		} break;
	case    DX_RIGHT:
		if (ch[cn].x<MAPX - 1)
		{
			ch[cn].status  = 184;
			ch[cn].status2 = 2;
		}
		else
		{
			ch[cn].cerrno = ERR_FAILED;
		} break;
	default:
		ch[cn].cerrno = ERR_FAILED;
		break;
	}
}

void act_idle(int cn)
{
	if ((globs->ticker & 15)==(cn & 15))
	{
		do_area_notify(cn, 0, ch[cn].x, ch[cn].y, NT_SEE, cn, 0, 0, 0);
	}
}

// ------------------
// end of act library
// ------------------

void plr_reset_status(int cn)
{
	switch(ch[cn].dir)
	{
	case    DX_UP:
		ch[cn].status = 0;
		break;
	case    DX_DOWN:
		ch[cn].status = 1;
		break;
	case    DX_LEFT:
		ch[cn].status = 2;
		break;
	case    DX_RIGHT:
		ch[cn].status = 3;
		break;
	case    DX_LEFTUP:
		ch[cn].status = 4;
		break;
	case    DX_LEFTDOWN:
		ch[cn].status = 5;
		break;
	case    DX_RIGHTUP:
		ch[cn].status = 6;
		break;
	case    DX_RIGHTDOWN:
		ch[cn].status = 7;
		break;
	default:
		xlog("plr_doact (svr_act.c): illegal value for dir: %d for char %d", ch[cn].dir, cn);
		ch[cn].dir = DX_UP;
		ch[cn].status = 0;
		break;
	}
}


void plr_doact(int cn)
{
	unsigned long long prof;

	// put idle value in status in case driver does nothing...
	plr_reset_status(cn);

	if (group_active(cn))
	{
		prof = prof_start();
		driver(cn);
		prof_stop(24, prof);
	}
}

// Gets the effective "speed table" value, the same way as a hypothetical table of 1's and 0's would.
static inline int get_speedValue(int speedV, int tickV, int cast)
{
	int bonus = 0;
	
	if (cast)
	{
		if (speedV <=   0)                  bonus = 8;
		if (speedV <=  33 && speedV >   0)  bonus = 4;
		if (speedV <=  66 && speedV >  33)  bonus = 2;
		if (speedV <=  99 && speedV >  66)  bonus = 1;
	}
	
	return ((SPEED_CAP-speedV)*(tickV+1)/CTICK_MAX - (SPEED_CAP-speedV)*tickV/CTICK_MAX) + bonus;
}

static inline int speedo(int n)
{
	int moveSpd;
	
	moveSpd = ch[n].speed - ch[n].move_speed;
	moveSpd = clamp(moveSpd, 0, (SPEED_CAP-1));
	
	return get_speedValue(moveSpd, ctick, 0);
}
static inline int speedoMisc(int n)
{
	int miscSpd;
	
	miscSpd = ch[n].speed;
	
	switch(ch[n].status2)
	{
		// 0, 5, 6 == Attack
		case  0:
		case  5:
		case  6:
			miscSpd -= ch[n].atk_speed;
			miscSpd  = clamp(miscSpd, 0, (SPEED_CAP-1));
			break;
			
		// 9 == Use skill, mostly casting
		case  9:
			miscSpd -= ch[n].cast_speed;
			miscSpd  = clamp(miscSpd, 0, (SPEED_CAP-1));
			if (IS_PLAYER(n)) // Instant cast limited to Players only
				return get_speedValue(miscSpd, ctick, 1);
			break;
			
		// Default - Shouldn't happen but here as a redundancy
		default:
			miscSpd  = clamp(miscSpd, 0, (SPEED_CAP-1));
			break;
	}
	
	return get_speedValue(miscSpd, ctick, 0);
}

void plr_act(int cn)
{
	unsigned long long prof;
	int m;
	
	if (ch[cn].stunned==1)
	{
		act_idle(cn);
		if (ch[cn].temp==CT_PANDIUM && (!(ch[cn].flags & (CF_PLAYER | CF_USURP))))
		{
			prof = prof_start();
			npc_driver_low(cn);
			prof_stop(27, prof);
		}
		return;
	}
	if (ch[cn].flags & CF_STONED)
	{
		act_idle(cn);
		return;
	}
	
	switch(ch[cn].status)
	{
		// idle up
		case   0: act_idle(cn); plr_doact(cn); return;
		// idle down
		case   1: act_idle(cn); plr_doact(cn); return;
		// idle left
		case   2: act_idle(cn); plr_doact(cn); return;
		// idle right
		case   3: act_idle(cn); plr_doact(cn); return;
		// idle left-up
		case   4: act_idle(cn); plr_doact(cn); return;
		// idle left-down
		case   5: act_idle(cn); plr_doact(cn); return;
		// idle right-up
		case   6: act_idle(cn); plr_doact(cn); return;
		// idle right-down
		case   7: act_idle(cn); plr_doact(cn); return;
		
		// walk up
		case  16: case  17: case  18: case  19: case  20: case  21: case  22: case  23:
			if ((m = speedo(cn)))
			{
				if ((ch[cn].status + m) > 23)
				{
					ch[cn].status = 16 + ((ch[cn].status + m) - 23 - 1); 
					plr_move_up(cn); plr_doact(cn);
				}
				else
					ch[cn].status += m;
			}
			return;
		
		// walk down
		case  24: case  25: case  26: case  27: case  28: case  29: case  30: case  31:
			if ((m = speedo(cn)))
			{
				if ((ch[cn].status + m) > 31)
				{
					ch[cn].status = 24 + ((ch[cn].status + m) - 31 - 1); 
					plr_move_down(cn); plr_doact(cn);
				}
				else
					ch[cn].status += m;
			}
			return;
		
		// walk left
		case  32: case  33: case  34: case  35: case  36: case  37: case  38: case  39:
			if ((m = speedo(cn)))
			{
				if ((ch[cn].status + m) > 39)
				{
					ch[cn].status = 32 + ((ch[cn].status + m) - 39 - 1); 
					plr_move_left(cn); plr_doact(cn);
				}
				else
					ch[cn].status += m;
			}
			return;
		
		// walk right
		case  40: case  41: case  42: case  43: case  44: case  45: case  46: case  47:
			if ((m = speedo(cn)))
			{
				if ((ch[cn].status + m) > 47)
				{
					ch[cn].status = 40 + ((ch[cn].status + m) - 47 - 1); 
					plr_move_right(cn); plr_doact(cn);
				}
				else
					ch[cn].status += m;
			}
			return;
		
		// left+up:
		case  48: case  49: case  50: case  51: case  52: case  53:
		case  54: case  55: case  56: case  57: case  58: case  59:
			if ((m = speedo(cn)))
			{
				if ((ch[cn].status + m) > 59)
				{
					ch[cn].status = 48 + ((ch[cn].status + m) - 59 - 1); 
					plr_move_leftup(cn); plr_doact(cn);
				}
				else
					ch[cn].status += m;
			}
			return;
		
		// left+down
		case  60: case  61: case  62: case  63: case  64: case  65:
		case  66: case  67: case  68: case  69: case  70: case  71:
			if ((m = speedo(cn)))
			{
				if ((ch[cn].status + m) > 71)
				{
					ch[cn].status = 60 + ((ch[cn].status + m) - 71 - 1); 
					plr_move_leftdown(cn); plr_doact(cn);
				}
				else
					ch[cn].status += m;
			}
			return;
		
		// right+up
		case  72: case  73: case  74: case  75: case  76: case  77:
		case  78: case  79: case  80: case  81: case  82: case  83:
			if ((m = speedo(cn)))
			{
				if ((ch[cn].status + m) > 83)
				{
					ch[cn].status = 72 + ((ch[cn].status + m) - 83 - 1); 
					plr_move_rightup(cn); plr_doact(cn);
				}
				else
					ch[cn].status += m;
			}
			return;
		
		// right+down
		case  84: case  85: case  86: case  87: case  88: case  89:
		case  90: case  91: case  92: case  93: case  94: case  95:
			if ((m = speedo(cn)))
			{
				if ((ch[cn].status + m) > 95)
				{
					ch[cn].status = 84 + ((ch[cn].status + m) - 95 - 1); 
					plr_move_rightdown(cn); plr_doact(cn);
				}
				else
					ch[cn].status += m;
			}
			return;
		
		// turn up to left-up
		case  96: case  97: case  98: case  99:
			if ((m = speedo(cn)))
			{
				if ((ch[cn].status + m) > 99)
				{
					ch[cn].status = 96 + ((ch[cn].status + m) - 99 - 1); 
					plr_turn_leftup(cn); plr_doact(cn);
				}
				else
					ch[cn].status += m;
			}
			return;
		
		// turn left-up to left
		case 100: case 101: case 102: case 103:
			if ((m = speedo(cn)))
			{
				if ((ch[cn].status + m) > 103)
				{
					ch[cn].status = 100 + ((ch[cn].status + m) - 103 - 1); 
					plr_turn_left(cn); plr_doact(cn);
				}
				else
					ch[cn].status += m;
			}
			return;
		
		// turn up to right-up
		case 104: case 105: case 106: case 107:
			if ((m = speedo(cn)))
			{
				if ((ch[cn].status + m) > 107)
				{
					ch[cn].status = 104 + ((ch[cn].status + m) - 107 - 1); 
					plr_turn_rightup(cn); plr_doact(cn);
				}
				else
					ch[cn].status += m;
			}
			return;
			

		// turn right-up to up
		case 108: case 109: case 110: case 111:
			if ((m = speedo(cn)))
			{
				if ((ch[cn].status + m) > 111)
				{
					ch[cn].status = 108 + ((ch[cn].status + m) - 111 - 1); 
					plr_turn_right(cn); plr_doact(cn);
				}
				else
					ch[cn].status += m;
			}
			return;
		
		// turn down to left-down
		case 112: case 113: case 114: case 115:
			if ((m = speedo(cn)))
			{
				if ((ch[cn].status + m) > 115)
				{
					ch[cn].status = 112 + ((ch[cn].status + m) - 115 - 1); 
					plr_turn_leftdown(cn); plr_doact(cn);
				}
				else
					ch[cn].status += m;
			}
			return;
		
		// turn left-down to down
		case 116: case 117: case 118: case 119:
			if ((m = speedo(cn)))
			{
				if ((ch[cn].status + m) > 119)
				{
					ch[cn].status = 116 + ((ch[cn].status + m) - 119 - 1); 
					plr_turn_left(cn); plr_doact(cn);
				}
				else
					ch[cn].status += m;
			}
			return;
		
		// turn down to right-down
		case 120: case 121: case 122: case 123:
			if ((m = speedo(cn)))
			{
				if ((ch[cn].status + m) > 123)
				{
					ch[cn].status = 120 + ((ch[cn].status + m) - 123 - 1); 
					plr_turn_rightdown(cn); plr_doact(cn);
				}
				else
					ch[cn].status += m;
			}
			return;
		
		// turn right-down to right
		case 124: case 125: case 126: case 127:
			if ((m = speedo(cn)))
			{
				if ((ch[cn].status + m) > 127)
				{
					ch[cn].status = 124 + ((ch[cn].status + m) - 127 - 1); 
					plr_turn_right(cn); plr_doact(cn);
				}
				else
					ch[cn].status += m;
			}
			return;
		
		// turn left to left-up
		case 128: case 129: case 130: case 131:
			if ((m = speedo(cn)))
			{
				if ((ch[cn].status + m) > 131)
				{
					ch[cn].status = 128 + ((ch[cn].status + m) - 131 - 1); 
					plr_turn_leftup(cn); plr_doact(cn);
				}
				else
					ch[cn].status += m;
			}
			return;
		
		// turn left-up to up
		case 132: case 133: case 134: case 135:
			if ((m = speedo(cn)))
			{
				if ((ch[cn].status + m) > 135)
				{
					ch[cn].status = 132 + ((ch[cn].status + m) - 135 - 1); 
					plr_turn_up(cn); plr_doact(cn);
				}
				else
					ch[cn].status += m;
			}
			return;
		
		// turn left to left-down
		case 136: case 137: case 138: case 139:
			if ((m = speedo(cn)))
			{
				if ((ch[cn].status + m) > 139)
				{
					ch[cn].status = 136 + ((ch[cn].status + m) - 139 - 1); 
					plr_turn_leftdown(cn); plr_doact(cn);
				}
				else
					ch[cn].status += m;
			}
			return;
		
		// turn left-down to down
		case 140: case 141: case 142: case 143:
			if ((m = speedo(cn)))
			{
				if ((ch[cn].status + m) > 143)
				{
					ch[cn].status = 140 + ((ch[cn].status + m) - 143 - 1); 
					plr_turn_down(cn); plr_doact(cn);
				}
				else
					ch[cn].status += m;
			}
			return;
		
		// turn right to right-up
		case 144: case 145: case 146: case 147:
			if ((m = speedo(cn)))
			{
				if ((ch[cn].status + m) > 147)
				{
					ch[cn].status = 144 + ((ch[cn].status + m) - 147 - 1); 
					plr_turn_rightup(cn); plr_doact(cn);
				}
				else
					ch[cn].status += m;
			}
			return;
		
		// turn right-up to right
		case 148: case 149: case 150: case 151:
			if ((m = speedo(cn)))
			{
				if ((ch[cn].status + m) > 151)
				{
					ch[cn].status = 148 + ((ch[cn].status + m) - 151 - 1); 
					plr_turn_up(cn); plr_doact(cn);
				}
				else
					ch[cn].status += m;
			}
			return;
		
		// turn right to right-down
		case 152: case 153: case 154: case 155:
			if ((m = speedo(cn)))
			{
				if ((ch[cn].status + m) > 155)
				{
					ch[cn].status = 152 + ((ch[cn].status + m) - 155 - 1); 
					plr_turn_rightdown(cn); plr_doact(cn);
				}
				else
					ch[cn].status += m;
			}
			return;
		
		// turn right-down to down
		case 156: case 157: case 158: case 159:
			if ((m = speedo(cn)))
			{
				if ((ch[cn].status + m) > 159)
				{
					ch[cn].status = 156 + ((ch[cn].status + m) - 159 - 1); 
					plr_turn_down(cn); plr_doact(cn);
				}
				else
					ch[cn].status += m;
			}
			return;
		
		// misc up
		case 160: case 161: case 162: case 163: case 164: case 165: case 166: case 167:
			if ((m = speedoMisc(cn)))
			{
				if ((ch[cn].status + m) > 167)
				{
					ch[cn].status = 160 + ((ch[cn].status + m) - 167 - 1); 
					plr_misc(cn); plr_doact(cn);
				}
				else
					ch[cn].status += m;
			}
			return;
		
		// misc down
		case 168: case 169: case 170: case 171: case 172: case 173: case 174: case 175:
			if ((m = speedoMisc(cn)))
			{
				if ((ch[cn].status + m) > 175)
				{
					ch[cn].status = 168 + ((ch[cn].status + m) - 175 - 1); 
					plr_misc(cn); plr_doact(cn);
				}
				else
					ch[cn].status += m;
			}
			return;
		
		// misc left
		case 176: case 177: case 178: case 179: case 180: case 181: case 182: case 183:
			if ((m = speedoMisc(cn)))
			{
				if ((ch[cn].status + m) > 183)
				{
					ch[cn].status = 176 + ((ch[cn].status + m) - 183 - 1); 
					plr_misc(cn); plr_doact(cn);
				}
				else
					ch[cn].status += m;
			}
			return;
		
		// misc right
		case 184: case 185: case 186: case 187: case 188: case 189: case 190: case 191:
			if ((m = speedoMisc(cn)))
			{
				if ((ch[cn].status + m) > 191)
				{
					ch[cn].status = 184 + ((ch[cn].status + m) - 191 - 1); 
					plr_misc(cn); plr_doact(cn);
				}
				else
					ch[cn].status += m;
			}
			return;
		
		default:
			xlog("plr_act (svr_act.c): Unknown character status %d for char %d", ch[cn].status, cn);
			ch[cn].status = 0;
			return;
	}
	/* not reached */
}

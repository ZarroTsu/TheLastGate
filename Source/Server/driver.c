/*************************************************************************

   This file is part of 'Mercenaries of Astonia v2'
   Copyright (c) 1997-2001 Daniel Brockhaus (joker@astonia.com)
   All rights reserved.

 **************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#include "server.h"
#include "driver.h"
#include "npc.h"

/*
   DATA 0-9 is for exclusive drivers as MERCHANT

   DATA usage by all NPC drivers

   10-18:  patrol-stops, using m=x+y*MAPX
   19:     next stop
   20-23:  doors to close
   24:     prevent fight mode, -1=defend evil, 0=no interference, 1=defend good
   25:     special driver
			1: Grolmy, SeaGrolmy or Timid enemy
			2: City-attack BS raid npc
			3: Malte
			4: Shiva
   
   26:     special sub-driver
			Most are not reached if the main driver is different, but best to use other values
			  1 : Cityguard A  -- Outpost
			  2 : Shiva
			  3 : Cityguard B  -- City
			  4 : Triggered by croaking floor trap
			  5 : Temple Sitter -- Notices unique weapon
			 12 : Lab 12 lord -- spawns helpers when they gain 1 healing sickness stack
			 13 : Lizard Emperor - Does some fancy shenanigans
			 14 : Shiva II - Much simplier than Shiva I, just summons monsters occasionally.
			 15 : SeaGrolmy - used to check between this and normal grolmy
			 20 : Tower XX - Lock doors
			 30 : Abyss  X - Lock doors
			10X : BS Tier 1
			20X : BS Tier 2
			30X : BS Tier 3
   
   27:     last time we got stop command (password logic)
   28:     exp earned since creation (GHOST COMPANION et al)
   29:     resting position (x+y*MAPX)
   30:     resting dir (what a waste of space)
   31:     protect character created from template X
   32-35:  lights to keep burning
   36:     frust timer. if above 100, char will use NPC-magic (teleport etc.) to reach objective
   37-40:  last character we talked to. set 37 to !=0 to activate talking
   41:     turn on/off lights from this template if daylight on/off
   42:     group
   43-46:  if 43 is set attack everyone NOT member of one of the groups 43-46
   47:     get and destroy/donate garbage
   48:     say text on death with this probability (X%)
   49:     wants item X
   50:     teaches skill X in exchange for item (raises it to 1)
   51:     raises EXP by X in exchange for item
   52:     shout for help if attacked, using code X
   53:     come to help if called by code X
   54:     place of shout
   55:     timeout for shout, if 54 is zero and 55 is set, WE shouted for help
   56:     timeout2 for greet
   57:     rest time between patrol (used by driver)
   58:     importance of current job (for better endurance management) (0=low, 1=medium, 2=high)
   59:     help all members of group X
   60:     random walk, time between walks
   61:     time elapsed
   62:     create light: 0=never, 1=when dark, 2=when dark and not resting, 3=when dark and fighting
   63:     obey and protect character X (CHD_MASTER)
   64:     self destruct in X ticks (PCD_COMPANION??)
   65:     friend is under attack - help by magic etc.
   66:     gives item X in exchange for item from 49
   67:     timeout for greeting
   68:     value of knowledge (PCD_ATTACKTIME??)
   69:     follow character X (PCD_ATTACKVICT??)
   70:     last time we called our god for help
   71:     talkativity (MCD_TALKATIVE)
   72:     area of knowledge
   73:     random walk: max distance to origin
   74:     last time we created a ghost
   75:     last time we stunned someone
   76:     last known position of an enemy
   77:     timeout for 76
   78:     attacked by invisible
   79:     rest time between patrol (value from admin interface)
   80-91:  characters on kill list
   92:     no-sleep bonus
   93:     attack distance (gets warning first)
   94:     last warning time
   95:     keyword action, 1=wait for password and attack if not given, 2=dont attack anyone further away then [93] from resting position
   96:     queued spells
   97:     being usurped by
   98:     GC no-see-master timeout
   99:     used by populate

   TEXT usage by all NPC drivers
   0:      killed enemy %1
   1:      attacking new enemy %1
   2:      greeting %1
   3:      killed by %1
   4:      shouting for help (against %1)
   5:      coming to help %1
   6:      keyword                 -|
   7:      reaction to keyword      |
   8:      warning message         -|
   9:      abused as memory of already searched graves. i know, i know, bad style and all...
 */

// ***************************************
// *         Helper Routines             *
// ***************************************

int get_frust_x_off(int f)
{
	switch(f % 5)
	{
	case    0:
		return 0;
	case    1:
		return 1;
	case    2:
		return -1;
	case    3:
		return( 2);
	case    4:
		return( -2);
	default:
		return 0;
	}
}

int get_frust_y_off(int f)
{
	switch((f / 5) % 5)
	{
	case    0:
		return 0;
	case    1:
		return 1;
	case    2:
		return -1;
	case    3:
		return( 2);
	case    4:
		return( -2);
	default:
		return 0;
	}
}

int npc_dist(int cn, int co)
{
	return(max(abs(ch[cn].x - ch[co].x), abs(ch[cn].y - ch[co].y)));
}

// Enemy [cn] adds [co] to their kill list
int npc_add_enemy(int cn, int co, int always)
{
	int n, idx, d1, d2, cc;

	// don't attack anyone of the same group. Never, never, never.
	if (ch[cn].data[CHD_GROUP]==ch[co].data[CHD_GROUP])
	{
		return 0;
	}
	
	// Thralled enemies ignore other enemies and vice versa
	if (!IS_PLAYER(co) && !IS_PLAYER_COMP(co) && (ch[cn].data[CHD_GROUP]==65500 || ch[co].data[CHD_GROUP]==65500))
	{
		return 0;
	}
	
	// Thralled enemies always ignore their enthraller
	if (ch[cn].data[CHD_GROUP]==65500 && co == ch[cn].data[CHD_MASTER])
	{
		return 0;
	}
	
	// Group check for GCs
	if (!IS_THRALL(cn) &&
		(!IS_PLAYER(cn) && IS_SANECHAR(cc = ch[cn].data[CHD_MASTER]) && ch[cc].data[CHD_GROUP]==ch[co].data[CHD_GROUP]) || 
		(!IS_PLAYER(co) && IS_SANECHAR(cc = ch[co].data[CHD_MASTER]) && ch[cn].data[CHD_GROUP]==ch[cc].data[CHD_GROUP]))
	{
		return 0;
	}
	
	// Group 1 mobs shall not attack ghost companions.
	// Ishtar said 65536 to 65536+4096, I hope this is OK.
	if (!always && (ch[cn].data[CHD_GROUP] == 1) && (ch[co].data[CHD_GROUP] & 0x10000))
	{
		return 0;
	}
	
	if (getrank(cn)<20 && !IS_THRALL(cn))
	{
		if (!always && (ch[cn].points_tot + 500) * 20<ch[co].points_tot)
		{
			return 0;
		}
	}

	ch[cn].data[76] = ch[co].x + ch[co].y * MAPX;
	ch[cn].data[77] = globs->ticker;

	cc = ch[cn].attack_cn;

	d1 = npc_dist(cn, cc);
	d2 = npc_dist(cn, co);

	if (!ch[cn].attack_cn ||
	    (d1>d2 && (globs->flags & GF_CLOSEENEMY)) ||
	    (d1==d2 && (!cc || ch[cc].attack_cn!=cn) && ch[co].attack_cn==cn) )
	{
		ch[cn].attack_cn = co;
		ch[cn].goto_x = 0;        // cancel goto (patrol) as well
		ch[cn].data[58] = 2;
	}

	idx = co | (char_id(co) << 16);

	for (n = MCD_ENEMY1ST; n<=MCD_ENEMYZZZ; n++)
	{
		if (ch[cn].data[n]==idx)
		{
			return 0;
		}
	}

	for (n = MCD_ENEMYZZZ; n>MCD_ENEMY1ST; n--)
	{
		ch[cn].data[n] = ch[cn].data[n - 1];
	}

	ch[cn].data[MCD_ENEMY1ST] = idx;

	return 1;
}

int npc_is_enemy(int cn, int co)
{
	int n, idx;

	idx = co | (char_id(co) << 16);

	for (n = MCD_ENEMY1ST; n<=MCD_ENEMYZZZ; n++)
	{
		if (ch[cn].data[n]==idx)
		{
			return 1;
		}
	}

	return 0;
}

int npc_list_enemies(int npc, int cn)
{
	int none = 1;
	int n;
	int cv;

	do_char_log(cn, 2, "Enemies of %s:\n", ch[npc].name);
	for (n = MCD_ENEMY1ST; n<=MCD_ENEMYZZZ; n++)
	{
		if ((cv = ch[npc].data[n] & 0xFFFF))
		{
			do_char_log(cn, 2, "  %s\n", ch[cv].name);
			none = 0;
		}
	}
	if (none)
	{
		do_char_log(cn, 2, "-none-\n");
		return 0;
	}
	return 1;
}

int npc_remove_enemy(int npc, int enemy)
{
	int found = 0;
	int n;

	for (n = MCD_ENEMY1ST; n<=MCD_ENEMYZZZ; n++)
	{
		if ((ch[npc].data[n] & 0xFFFF) == enemy)
		{
			found = 1;
		}
		if (found)
		{
			if (n < MCD_ENEMYZZZ)
			{
				ch[npc].data[n] = ch[npc].data[n + 1];
			}
			else
			{
				ch[npc].data[n] = 0;
			}
		}
	}
	return(found);
}

void npc_remove_all_enemies(int npc)
{
	int n;
	
	for (n = MCD_ENEMY1ST; n<=MCD_ENEMYZZZ; n++) // remove enemy
	{
		ch[npc].data[n] = 0;
	}
	ch[npc].attack_cn = 0;
	ch[npc].goto_x = 0;
	ch[npc].goto_y = 0;
	ch[npc].data[76] = 0;
	ch[npc].data[77] = 0;
	ch[npc].data[78] = 0;
}

/* say one of the NPC's canned response messages, with optional name. */
void npc_saytext_n(int npc, int n, char *name)
{
	if (ch[npc].flags & CF_SHUTUP)
	{
		return;
	}

	if (ch[npc].text[n][0])
	{
		if (IS_COMP_TEMP(npc))
		{
			if (ch[npc].data[MCD_TALKATIVE] == -10)
			{
				do_sayx(npc, ch[npc].text[n], name);
			}
		}
		else
		{
			do_sayx(npc, ch[npc].text[n], name);
		}
	}
}

// ***************************************
// *         Message Handlers            *
// ***************************************

int npc_see(int cn, int co);

int npc_gotattack(int cn, int co, int dam)
{
	int cc;

	ch[cn].data[92] = TICKS * 60 * 10;

	if (co && (ch[co].flags & CF_PLAYER) && ch[cn].alignment==10000 && (strcmp(ch[cn].name, "Peacekeeper") || ch[cn].a_hp<ch[cn].hp[5] * 500) && ch[cn].data[70]<globs->ticker)
	{
		do_sayx(cn, "Skua! Protect the innocent! Send me a Peacekeeper!");
		fx_add_effect(6, 0, ch[cn].x, ch[cn].y, 0);
		ch[cn].data[70] = globs->ticker + TICKS * 60 * 1;

		cc = god_create_char(CT_PEACEKEEPER, 1);
		ch[cc].temp = CT_COMPANION;
		ch[cc].data[CHD_GROUP] = 65536 + cn;                       // set group
		ch[cc].data[59] = 65536 + cn;                       // protect all other members of this group

		/* make thrall harmless */
		ch[cc].data[24] = 0;    // do not interfere in fights
		ch[cc].data[36] = 0;    // no walking around
		ch[cc].data[43] = 0;    // don't attack anyone
		ch[cc].data[MCD_ENEMY1ST] = co | (char_id(co) << 16);   // attacker is enemy
		ch[cc].data[CHD_MASTER] = cn;   // obey and protect enthraller
		ch[cc].data[64] = globs->ticker + 120 * TICKS;        // make her vanish after 2 minutes
		ch[cc].data[70] = globs->ticker + TICKS * 60 * 1;       // and make sure thralled peacies don't summon more than one more

		strcpy(ch[cc].name, "Shadow of Peace");
		strcpy(ch[cc].reference, "Shadow of Peace");
		strcpy(ch[cc].description, "You see a Shadow of Peace.");

		if (!god_drop_char_fuzzy(cc, ch[co].x, ch[co].y))
		{
			god_destroy_items(cc);
			ch[cc].used = 0;
		}
	}

	if (ch[cn].alignment>1000 && ch[cn].data[70]<globs->ticker && ch[cn].a_mana<ch[cn].mana[5] * 333 && !IS_COMP_TEMP(cn))
	{
		do_sayx(cn, "Skua! Help me!");
		ch[cn].data[70] = globs->ticker + TICKS * 60 * 2;
		ch[cn].a_mana = ch[cn].mana[5] * 1000;
		fx_add_effect(6, 0, ch[cn].x, ch[cn].y, 0);
	}

	if (ch[cn].data[52] && ch[cn].a_hp<ch[cn].hp[5] * 666)    // we're supposed to shout for help if attacked and HP below 2/3rd
	{
		if (ch[cn].data[55] + TICKS * 180<globs->ticker) // shout every 180 seconds
		{
			ch[cn].data[54] = 0;
			ch[cn].data[55] = globs->ticker;
			npc_saytext_n(cn, 4, ch[co].name);
			do_npc_shout(cn, NT_SHOUT, cn, ch[cn].data[52], ch[cn].x, ch[cn].y);
		}
	}

	if (!do_char_can_see(cn, co, 0))    // we have been attacked but cannot see the attacker
	{
		ch[cn].data[78] = globs->ticker + TICKS * 30;
		return 1;
	}

	// fight back when attacked - all NPCs do this:
	if (!IS_PLAYER_COMP(cn) || (IS_PLAYER_COMP(cn) && (ch[cn].data[1]!=1 || ch[cn].a_hp<ch[cn].hp[5] * 600))) // passive gc check - only fight back if at risk of dying
	{
		if (npc_add_enemy(cn, co, 1))
		{
			if (!(ch[co].flags & CF_SILENCE) && !(ch[cn].flags & CF_SILENCE))
			{
				if (IS_PLAYER_COMP(cn))
				{
					if (ch[cn].data[55]+TICKS*2<globs->ticker)
					{
						ch[cn].data[55] = globs->ticker;
						npc_saytext_n(cn, 1, ch[co].name);
					}
				}
				else
					npc_saytext_n(cn, 1, ch[co].name);
			}
			chlog(cn, "Added %s to kill list for attacking me", ch[co].name);
		}
		/*
		else
		{
			chlog(cn, "Ignoring %s's attack", ch[co].name);
		}
		*/
	}

	return 1;
}

int npc_gothit(int cn, int co, int dam)
{
	if (npc_gotattack(cn, co, dam))
	{
		return 1;
	}

	return 0;
}

int npc_gotmiss(int cn, int co)
{
	if (npc_gotattack(cn, co, 0))
	{
		return 1;
	}
	return 0;
}

int npc_didhit(int cn, int co, int dam)
{
	return 0;
}

int npc_didmiss(int cn, int co)
{
	return 0;
}

int npc_killed(int cn, int cc, int co)
{
	int n, idx;

	if (ch[cn].attack_cn==co)
	{
		ch[cn].attack_cn = 0;
	}
	ch[cn].data[76] = ch[cn].data[77] = ch[cn].data[78] = 0;
	npc_activate_rings(cn, 0);
	npc_wedge_doors(ch[cn].data[26], 0);

	idx = co | (char_id(co) << 16);

	for (n = MCD_ENEMY1ST; n<=MCD_ENEMYZZZ; n++) // remove enemy
	{
		if (ch[cn].data[n]==idx)
		{
			// add expansion checks and a handler for texts !!!
			if (cn==cc)
			{
				npc_saytext_n(cn, 0, ch[co].name);
			}
			// add killed mob to data so we don't re-add it immediately 
			if (IS_COMPANION(cn)) ch[cn].data[91] = idx; 
			ch[cn].data[n] = 0;
			return 1;
		}
	}

	return 0;
}

int npc_didkill(int cn, int co)
{
	if (npc_killed(cn, cn, co))
	{
		return 1;
	}

	return 0;
}

int npc_gotexp(int cn, int amount)
{
	return 0;
}

int npc_seekill(int cn, int cc, int co)
{
	if (npc_killed(cn, cc, co))
	{
		return 1;
	}

	return 0;
}

int npc_seeattack(int cn, int cc, int co)
{
	int diff, ret, c2, c3;
	int idx;

	ch[cn].data[92] = TICKS * 60 * 10;

	if (!do_char_can_see(cn, co, 0))
	{
		return 1;                     // processed it: we cannot see the defender, so ignore it
	}
	if (!do_char_can_see(cn, cc, 0))
	{
		return 1;                     // processed it: we cannot see the attacker, so ignore it
	}
	if (ch[cn].data[24])   // prevent fight mode
	{
		diff = (ch[cc].alignment - 50) - ch[co].alignment;
		if (diff<=0)
		{
			if (ch[cn].data[24]>0)
			{
				ret = npc_add_enemy(cn, cc, 1);
				c2  = cc;
				c3  = co;
			}
			else
			{
				ret = npc_add_enemy(cn, co, 1);
				c2  = co;
				c3  = cc;
			}
		}
		else
		{
			if (ch[cn].data[24]>0)
			{
				ret = npc_add_enemy(cn, co, 1);
				c2  = co;
				c3  = cc;
			}
			else
			{
				ret = npc_add_enemy(cn, cc, 1);
				c2  = cc;
				c3  = co;
			}
		}
		if (ret)
		{
			if (!(ch[c2].flags & CF_SILENCE) && !(ch[cn].flags & CF_SILENCE))
			{
				npc_saytext_n(cn, 1, ch[c2].name);
			}
			chlog(cn, "Added %s to kill list for attacking %s (prevent fight)", ch[c2].name, ch[c3].name);
		}
		return 1;
	}

	if (ch[cn].data[31])   // protect char (by temp)
	{
		ret = 0;
		if (ch[co].temp==ch[cn].data[31])
		{
			ret = npc_add_enemy(cn, cc, 1);
			if (ret)
			{
				if (!(ch[cc].flags & CF_SILENCE) && !(ch[cn].flags & CF_SILENCE))
				{
					npc_saytext_n(cn, 1, ch[cc].name);
				}
				chlog(cn, "Added %s to kill list for attacking %s (protect char)", ch[cc].name, ch[co].name);
			}
			if (!ch[cn].data[65])
			{
				ch[cn].data[65] = co;
			}
		}
		if (ch[cc].temp==ch[cn].data[31])
		{
			ret = npc_add_enemy(cn, co, 1);
			if (ret)
			{
				if (!(ch[co].flags & CF_SILENCE) && !(ch[cn].flags & CF_SILENCE))
				{
					npc_saytext_n(cn, 1, ch[co].name);
				}
				chlog(cn, "Added %s to kill list for being attacked by %s (protect char)", ch[co].name, ch[cc].name);
			}
			if (!ch[cn].data[65])
			{
				ch[cn].data[65] = cc;
			}
		}
	}

	if (ch[cn].data[CHD_MASTER] || ch[ch[cn].data[CHD_MASTER]].data[PCD_COMPANION] || ch[ch[cn].data[CHD_MASTER]].data[PCD_SHADOWCOPY])   // protect char (by nr)
	{
		ret = 0;
		if ((co==ch[cn].data[CHD_MASTER] || co==ch[ch[cn].data[CHD_MASTER]].data[PCD_COMPANION] || co==ch[ch[cn].data[CHD_MASTER]].data[PCD_SHADOWCOPY]) && ch[cn].data[1]!=1)
		{
			idx = cc | (char_id(cc) << 16);
			if (IS_COMPANION(cn) && ch[cn].data[91]!=idx)
			{
				ret = npc_add_enemy(cn, cc, 1);
			}
			if (ret)
			{
				if (!(ch[cc].flags & CF_SILENCE) && !(ch[cn].flags & CF_SILENCE))
				{
					npc_saytext_n(cn, 1, ch[cc].name);
				}
				chlog(cn, "Added %s to kill list for attacking %s (protect char)", ch[cc].name, ch[co].name);
			}
			if (!ch[cn].data[65])
			{
				ch[cn].data[65] = co;
			}
		}
		if ((cc==ch[cn].data[CHD_MASTER] || cc==ch[ch[cn].data[CHD_MASTER]].data[PCD_COMPANION] || cc==ch[ch[cn].data[CHD_MASTER]].data[PCD_SHADOWCOPY]) && ch[cn].data[1]!=1)
		{
			idx = co | (char_id(co) << 16);
			if (IS_COMPANION(cn) && ch[cn].data[91]!=idx)
			{
				ret = npc_add_enemy(cn, co, 1);
			}
			if (ret)
			{
				if (!(ch[co].flags & CF_SILENCE) && !(ch[cn].flags & CF_SILENCE))
				{
					npc_saytext_n(cn, 1, ch[co].name);
				}
				chlog(cn, "Added %s to kill list for being attacked by %s (protect char)", ch[co].name, ch[cc].name);
			}
			if (!ch[cn].data[65])
			{
				ch[cn].data[65] = cc;
			}
		}
	}

	if (ch[cn].data[59])   // protect by group
	{
		ret = 0;
		if (ch[cn].data[59]==ch[co].data[CHD_GROUP])
		{
			if (ch[cn].temp!=CT_SHIVA_I) ret = npc_add_enemy(cn, cc, 1);
			if (ret)
			{
				if (!(ch[cc].flags & CF_SILENCE) && !(ch[cn].flags & CF_SILENCE))
				{
					npc_saytext_n(cn, 1, ch[cc].name);
				}
				chlog(cn, "Added %s to kill list for attacking %s (protect group)", ch[cc].name, ch[co].name);
			}
			if (!ch[cn].data[65])
			{
				ch[cn].data[65] = co;
			}
		}
		if (ch[cn].data[59]==ch[cc].data[CHD_GROUP])
		{
			if (ch[cn].temp!=CT_SHIVA_I) ret = npc_add_enemy(cn, co, 1);
			if (ret)
			{
				if (!(ch[co].flags & CF_SILENCE) && !(ch[cn].flags & CF_SILENCE))
				{
					npc_saytext_n(cn, 1, ch[co].name);
				}
				chlog(cn, "Added %s to kill list for being attacked by %s (protect group)", ch[co].name, ch[cc].name);
			}
			if (!ch[cn].data[65])
			{
				ch[cn].data[65] = cc;
			}
		}
	}

	if (IS_COMP_TEMP(co) && ch[co].data[CHD_MASTER]==cn)   // MY ghost companion
	{
		if (!ch[cn].data[65])
		{
			ch[cn].data[65] = co;
		}
	}

	if (IS_COMP_TEMP(cc) && ch[cc].data[CHD_MASTER]==cn)   // MY ghost companion
	{
		if (!ch[cn].data[65])
		{
			ch[cn].data[65] = cc;
		}
	}

	return 0;
}

int npc_seehit(int cn, int cc, int co)
{
	if (npc_seeattack(cn, cc, co))
	{
		return 1;
	}
	if (npc_see(cn, cc))
	{
		return 1;
	}
	if (npc_see(cn, co))
	{
		return 1;
	}

	return 0;
}

int npc_seemiss(int cn, int cc, int co)
{
	if (npc_seeattack(cn, cc, co))
	{
		return 1;
	}
	if (npc_see(cn, cc))
	{
		return 1;
	}
	if (npc_see(cn, co))
	{
		return 1;
	}

	return 0;
}

int npc_quest_check(int cn, int val)
{
	int bit, tmp;

	if (val<32)
	{
		bit = 1 << (val);
		tmp = ch[cn].data[72] & bit;
		ch[cn].data[72] |= bit;
		return(tmp);
	}
	else
	{
		bit = 1 << (val - 32);
		tmp = ch[cn].data[94] & bit;
		ch[cn].data[94] |= bit;
		return(tmp);
	}
}

int npc_quest_cleared(int cn, int val)
{
	int bit, tmp;

	if (val<32)
	{
		bit = 1 << (val);
		tmp = ch[cn].data[72] & bit;
		return(tmp);
	}
	else
	{
		bit = 1 << (val - 32);
		tmp = ch[cn].data[94] & bit;
		return(tmp);
	}
}

int convert_skill_for_group(int co, int nr)
{
	// Revert to original values if changed
	if (nr == SK_POISON)   nr = SK_TAUNT; 	// Poison   -> Taunt
	if (nr == SK_STAFF)    nr = SK_TAUNT; 	// Staff    -> Taunt
	if (nr == SK_MSHIELD)  nr = SK_ENHANCE; // MShield  -> Enhance
	if (nr == SK_WEAKEN)   nr = SK_SLOW;    // Weaken   -> Slow
	if (nr == SK_SURROUND) nr = SK_CURSE;   // Surround -> Curse
	if (nr == SK_TACTICS)  nr = SK_SHIELD;  // Tactics  -> Shield
	if (nr == SK_DISPEL)   nr = SK_IMMUN;   // Dispel   -> Immun
	
	// Hacky flip-flopping
	if (IS_ANY_TEMP(co))
	{
		if (nr == SK_ENHANCE) nr = SK_MSHIELD;  // Enhance -> MShield
		if (nr == SK_SLOW)    nr = SK_WEAKEN;   // Slow    -> Weaken
		if (nr == SK_CURSE)   nr = SK_SURROUND; // Curse   -> Surround
	}
	if (IS_ANY_MERC(co))
	{
		if (nr == SK_TAUNT)   nr = SK_POISON; // Taunt -> Poison
	}
	if (IS_ANY_HARA(co))
	{
		if (nr == SK_TAUNT)  nr = SK_STAFF;   // Taunt -> Staff
		if (nr == SK_SHIELD) nr = SK_TACTICS; // Shiel -> Tactics
		if (nr == SK_IMMUN)  nr = SK_DISPEL;  // Immun -> Dispel
	}
	if (IS_BRAVER(co))
	{
		if (nr == SK_ENHANCE) nr = SK_MSHIELD;  // Enhance -> MShield
		if (nr == SK_SLOW)    nr = SK_WEAKEN;   // Slow    -> Weaken
		if (nr == SK_IMMUN)   nr = SK_DISPEL; 	// Immun   -> Dispel
		if (nr == SK_CURSE)   nr = SK_SURROUND; // Curse   -> Surround
	}
	if (IS_SEYAN_DU(co))
	{
		if (nr == SK_TAUNT   && B_SK(co, SK_TAUNT))   nr = SK_POISON;
		if (nr == SK_POISON  && B_SK(co, SK_POISON))  nr = SK_STAFF;
		if (nr == SK_SLOW    && B_SK(co, SK_SLOW))    nr = SK_WEAKEN;
		if (nr == SK_CURSE   && B_SK(co, SK_CURSE))   nr = SK_SURROUND;
		if (nr == SK_SHIELD  && B_SK(co, SK_SHIELD))  nr = SK_TACTICS;
		if (nr == SK_IMMUN   && B_SK(co, SK_IMMUN))   nr = SK_DISPEL;
	}
	if (IS_LYCANTH(co))
	{
		if (nr == SK_SHIELD) nr = SK_TACTICS; // Shiel -> Tactics
	}
	 
	return nr;
}

int npc_give(int cn, int co, int in, int money)
{
	int nr, nr2, ar, canlearn = 1, stsz = 1;
	int tmp = 0;
	int qnum = 0;
	int n, in2 = 0;
	unsigned char buf[3];

	if (ch[co].flags & (CF_PLAYER | CF_USURP))
		ch[cn].data[92] = TICKS * 60;
	else if (!group_active(cn))
		return 0;

	// Special hack for arch skills.
	if ((nr = ch[cn].data[50])!=0)
	{
		// Check each arch skill against the race that is intended to learn it.
		if (( nr == SK_WARCRY && !IS_SEYA_OR_ARTM(co) )
		 || ( nr == SK_PULSE  && !IS_SEYA_OR_ARHR(co) )
		 || ( nr == SK_LEAP   && !IS_SEYA_OR_SKAL(co) )
		 || ( nr == SK_LETHARGY && !IS_SEYA_OR_SORC(co) )
		 || ( nr == SK_GCMASTERY && !IS_SEYA_OR_SUMM(co) )
		 || ( nr == SK_ZEPHYR && !IS_SEYA_OR_WARR(co) )
		 || ( nr == SK_FINESSE && !IS_SEYA_OR_BRAV(co) )
		 || ( nr == SK_RAGE && !IS_SEYA_OR_LYCA(co) ))
			canlearn = 0;
		
		// Seyan'du can learn any arch skill, but only two!
		if ((nr == SK_WARCRY || nr == SK_PULSE || nr == SK_LEAP || nr == SK_LETHARGY || 
			nr == SK_GCMASTERY || nr == SK_ZEPHYR || nr == SK_FINESSE || nr == SK_RAGE) && IS_SEYAN_DU(co))
		{
			canlearn = 2;
			if (B_SK(co, SK_WARCRY))   canlearn--;
			if (B_SK(co, SK_LEAP))     canlearn--;
			if (B_SK(co, SK_GCMASTERY))canlearn--;
			if (B_SK(co, SK_LETHARGY)) canlearn--;
			if (B_SK(co, SK_PULSE))    canlearn--;
			if (B_SK(co, SK_ZEPHYR))   canlearn--;
			if (B_SK(co, SK_FINESSE))  canlearn--;
			if (B_SK(co, SK_RAGE))     canlearn--;
			
			if (canlearn>=1) 
				canlearn = 1;
			else 
				canlearn = 0;
		}
	}

	if (in && (
		ch[cn].data[49]==it[in].temp 
		|| 
		(ch[cn].temp==CT_OSIRIS && it[in].temp==MCT_CONTRACT)
		|| 
		(ch[cn].temp==CT_TACTICIAN && 
		(it[in].temp==IT_BS_CAN1 || it[in].temp==IT_BS_CAN2 || it[in].temp==IT_BS_CAN3)) 
		||
		((ch[cn].temp==CT_BISHOP || ch[cn].temp==CT_KWAIVICAR || ch[cn].temp==CT_GORNPASTOR) && (
		(it[in].temp>=IT_CH_FOOL && it[in].temp<=IT_CH_WORLD) || 
		(it[in].temp>=IT_CH_FOOL_R && it[in].temp<=IT_CH_WORLD_R)))
		||
		(ch[cn].temp==CT_PRIEST && IS_SINBINDER(in) && it[in].data[1]==1)
		||
		(ch[cn].temp==CT_HERBCOLL && 
		(it[in].temp==IT_HERBA || it[in].temp==IT_HERBB || it[in].temp==IT_HERBC || it[in].temp==IT_HERBD))
		||
		(ch[cn].temp==CT_HERBCOLL2 && it[in].temp==IT_HERBE)
		||
		(ch[cn].temp==CT_SCORPCOLL && 
		(it[in].temp==IT_SCORL || it[in].temp==IT_SCORP || it[in].temp==IT_SCORG || it[in].temp==IT_SCORQ))
		||
		(ch[cn].temp==CT_ANTIQUECL && it[in].temp==IT_ANTIQ)
		) && canlearn)
	{
		// Assure the player has inventory space before we do anything
		if (ch[cn].data[66] || ch[cn].data[50]==50)
		{
			for (n = 0; n<MAXITEMS; n++) if (!ch[co].item[n]) break;
			if (n==MAXITEMS)
			{
				do_char_log(co, 0, "You get the feeling you should clear some space in your backpack first.\n");
				god_take_from_char(in, cn);
				god_give_char(in, co);
				return 0;
			}
		}
		
		if (it[in].stack>1) 
			stsz = it[in].stack;
		
		if (ch[cn].temp==CT_OSIRIS && it[in].temp==MCT_CONTRACT)
		{
			if (!it[in].data[0])
				do_sayx(cn, "Hmm...");
			/*
			else if (co != it[in].data[0])
				do_sayx(cn, "Don't sour my mood, %s.", ch[co].name);
			*/
			else
				do_sayx(cn, "Very well %s. Show me what you can do!", ch[co].name);
			god_take_from_char(in, cn);
			if (start_contract(co, in))
				do_area_log(cn, 0, ch[cn].x, ch[cn].y, 1, "%s was whisked away somewhere.\n", ch[co].name);
			else
				god_give_char(in, co);
		}
		else if (ch[cn].temp==CT_TACTICIAN && (it[in].temp==IT_BS_CAN1 || it[in].temp==IT_BS_CAN2 || it[in].temp==IT_BS_CAN3))
		{	// hack for black candle
			ar = it[in].data[0];
			do_give_bspoints(co, ar, 1);
			use_consume_item(cn, in, 1);
			do_sayx(cn, "Ah, a black candle! Great work, %s! Now we may see peace for a while...", ch[co].name);
			do_area_log(cn, 0, ch[cn].x, ch[cn].y, 1, "The Tactician was impressed by %s's deed.\n", ch[co].name);
			ch[co].misc_action = DR_IDLE;
			return 0;
		}
		else if ((ch[cn].temp==CT_BISHOP || ch[cn].temp==CT_KWAIVICAR || ch[cn].temp==CT_GORNPASTOR) && (
			(it[in].temp>=IT_CH_FOOL && it[in].temp<=IT_CH_WORLD) || 
			(it[in].temp>=IT_CH_FOOL_R && it[in].temp<=IT_CH_WORLD_R) ))
		{
			if (in2 = ch[co].worn[WN_CHARM2])
			{
				if (it[in2].temp == it[in].temp)
				{
					do_sayx(cn, "I'm sorry %s, but you already have this tarot card's effects on you!", ch[co].name);
					god_take_from_char(in, cn);
					god_give_char(in, co);
					do_char_log(co, 1, "%s returned the %s to you.\n", ch[cn].reference, it[in].name);
					return 0;
				}
			}
			if (it[in].temp==IT_CH_FOOL) // special case - need to check equip requirements
			{
				if (do_check_fool(co, in)==-1)
				{
					god_take_from_char(in, cn);
					god_give_char(in, co);
					return 0;
				}
			}
			do_sayx(cn, "A tarot card, I see. Allow me to apply its magic to you, %s.", ch[co].name);
			god_take_from_char(in, cn);
			do_char_log(co, 1, "You now have the effects of your %s equipped.\n", it[in].name);
			fx_add_effect(6, 0, ch[co].x, ch[co].y, 0);
			if (in2 = ch[co].worn[WN_CHARM])
			{
				do_sayx(cn, "I have removed your %s for you as well. Please take it.", it[in2].name);
				do_char_log(co, 1, "%s returned the %s to you.\n", ch[cn].reference, it[in2].name);
			}
			it[in].x = 0;
			it[in].y = 0;
			it[in].carried = co;
			ch[co].citem = in2;
			ch[co].worn[WN_CHARM] = in;
			if (n = ch[co].data[PCD_COMPANION])  answer_transfer(co, n, 0);
			if (n = ch[co].data[PCD_SHADOWCOPY]) answer_transfer(co, n, 0);
			remove_all_spells(co, 0);
			ch[co].misc_action = DR_IDLE;
			return 0;
		}
		else if (ch[cn].temp==CT_PRIEST && IS_SINBINDER(in) && it[in].data[1]==1)
		{
			if (((in2 = ch[co].worn[WN_RRING]) && NOT_SINBINDER(in2)) || ch[co].worn[WN_LRING])
			{
				do_sayx(cn, "So sorry %s, but could you take the rings you're wearing off first? Thanks.", ch[co].name);
				god_take_from_char(in, cn);
				god_give_char(in, co);
				do_char_log(co, 1, "%s returned the ring to you.\n", ch[cn].reference);
				return 0;
			}
			if (((in2 = ch[co].worn[WN_CHARM]) && it[in].data[2]==it[in2].temp) || ((in2 = ch[co].worn[WN_CHARM2]) && it[in].data[2]==it[in2].temp))
			{
				do_sayx(cn, "So sorry %s, but you already have this tarot card's effects on you!", ch[co].name);
				god_take_from_char(in, cn);
				god_give_char(in, co);
				do_char_log(co, 1, "%s returned the ring to you.\n", ch[cn].reference);
				return 0;
			}
			if (it[in].data[2]==IT_CH_FOOL) // special case - need to check equip requirements
			{
				if (do_check_fool(co, in)==-1)
				{
					god_take_from_char(in, cn);
					god_give_char(in, co);
					return 0;
				}
			}
			do_sayx(cn, "A Sinbinder, I see. Allow me to apply its magic to you, %s.", ch[co].name);
			god_take_from_char(in, cn);
			do_char_log(co, 1, "The Priest chanted something and the %s slipped on your finger.\n", it[in].name);
			fx_add_effect(6, 0, ch[co].x, ch[co].y, 0);
			if ((in2 = ch[co].worn[WN_RRING]) && IS_SINBINDER(in2))
			{
				do_sayx(cn, "I have removed your old Sinbinder for you. It's all yours.");
				do_char_log(co, 1, "%s returned the previous Sinbinder Ring to you.\n", ch[cn].reference);
			}
			it[in].x = 0;
			it[in].y = 0;
			it[in].carried = co;
			ch[co].citem = in2;
			ch[co].worn[WN_RRING] = in;
			if (n = ch[co].data[PCD_COMPANION])  answer_transfer(co, n, 0);
			if (n = ch[co].data[PCD_SHADOWCOPY]) answer_transfer(co, n, 0);
			remove_all_spells(co, 0);
			ch[co].misc_action = DR_IDLE;
			return 0;
		}
		else if ((ch[cn].temp==CT_HERBCOLL && 
			(it[in].temp==IT_HERBA || it[in].temp==IT_HERBB || it[in].temp==IT_HERBC || it[in].temp==IT_HERBD)) || 
			(ch[cn].temp==CT_HERBCOLL2 && it[in].temp==IT_HERBE) ||
			(ch[cn].temp==CT_SCORPCOLL && (it[in].temp==IT_SCORL || it[in].temp==IT_SCORP || it[in].temp==IT_SCORG || it[in].temp==IT_SCORQ)) ||
			(ch[cn].temp==CT_ANTIQUECL && it[in].temp==IT_ANTIQ) )
		{
			if ((ch[cn].temp==CT_HERBCOLL || ch[cn].temp==CT_SCORPCOLL) && getrank(co)<8)
			{
				if (ch[cn].temp==CT_HERBCOLL)
					do_sayx(cn, "I'm sorry %s, I don't have a use for weeds!", ch[co].name);
				else
					do_sayx(cn, "I'm sorry %s, come back a little stronger, eh?", ch[co].name);
				god_take_from_char(in, cn);
				god_give_char(in, co);
				do_char_log(co, 1, "%s did not accept the %s.\n", ch[cn].reference, it[in].name);
				return 0;
			}
			
			switch (it[in].temp)
			{
				case IT_HERBA: money =  10000*stsz; nr = money; break;
				case IT_HERBB: money =  15000*stsz; nr = money; break;
				case IT_HERBC: money =  25000*stsz; nr = money; break;
				case IT_HERBD: money =  60000*stsz; nr = money; break;
				case IT_HERBE: money = 120000*stsz; nr = money; break;
				
				case IT_SCORL: money =    500*stsz; nr = money*3/2; break;
				case IT_SCORP: money =   1000*stsz; nr = money; break;
				case IT_SCORG: money =   2500*stsz; nr = money*3/4; break;
				case IT_SCORQ: money =  20000*stsz; nr = money*3; break;
				
				case IT_ANTIQ: money =   5000*stsz; nr = money; break;
				
				default: money = 1000*stsz; nr = money; break;
			}
			
			if (ch[cn].temp==CT_HERBCOLL2)
				do_sayx(cn, "Here'sss your payment, and a bit of knowledge.");
			else
				do_sayx(cn, "Here's your payment, and a bit of knowledge.");
			
			use_consume_item(cn, in, 1);
			do_give_exp(co, nr, 0, -1, money);
			// <group rewards>
			for (n = 1; n<MAXCHARS; n++)
			{
				if (ch[n].used==USE_EMPTY || !(ch[n].flags & (CF_PLAYER | CF_USURP))) continue;
				if (isgroup(n, co) && isgroup(co, n) && isnearby(co, n))
				{
					do_give_exp(n, nr, 0, -1, 0);
				}
			}
			// </group rewards>
			ch[co].misc_action = DR_IDLE;
			return 0;
		}
		else if (ch[cn].temp==CT_ISHTAR || ch[cn].temp==CT_ANKH || ch[cn].temp==CT_KWAI || ch[cn].temp==CT_GORN)
		{
			do_sayx(cn, "Thank you %s. That's the %s I wanted.", ch[co].name, it[in].reference);
			use_consume_item(cn, in, 1);
			// Check for player Chalice item to upgrade, or otherwise award the worst one.
			if (n = has_item(co, IT_CHALICE3)) nr = IT_CHALICE4;
			else if (n = has_item(co, IT_CHALICE2)) nr = IT_CHALICE3;
			else if (n = has_item(co, IT_CHALICE1)) nr = IT_CHALICE2;
			else nr = IT_CHALICE1;
			//
			if (!(ch[cn].flags&(CF_BODY)))
			{
				if (nr == IT_CHALICE1) 
				{
					do_sayx(cn, "Here is your %s in exchange.", it_temp[nr].reference);
				}
				else 
				{
					do_sayx(cn, "I shall improve your %s in exchange.", it[n].reference);
					use_consume_item(co, n, 1);
				}
				in = god_create_item(nr);
				god_give_char(in, co);
			}
			ch[co].misc_action = DR_IDLE;
			return 0;
		}
		else
		{
			// Tutorial 6
			if (ch[cn].temp==118&&ch[co].data[76]<(1<<6))
			{
				chlog(co, "SV_SHOWMOTD tutorial 6");
				buf[0] = SV_SHOWMOTD;
				*(unsigned char*)(buf + 1) = 106;
				xsend(ch[co].player, buf, 2);
			}
			do_sayx(cn, "Thank you %s. That's the %s I wanted.", ch[co].name, it[in].reference);
		}
		
		ch[co].misc_action = DR_IDLE;

		/* quest-requested items */
		if ((nr = ch[cn].data[50])!=0)
		{
			// Mercs and Seyans learn Haste from Regal now.
			if ((IS_ANY_MERC(co) || IS_SEYAN_DU(co)) && nr == 117) nr = SK_HASTE;
			
			nr = convert_skill_for_group(co, nr);

			if (nr==SK_TAUNT && IS_SEYAN_DU(co))
				do_sayx(cn, "Bring me the item again to learn Poison, %s!", ch[co].name);
			if (nr==SK_POISON && IS_SEYAN_DU(co))
				do_sayx(cn, "Bring me the item once more to learn Staff, %s!", ch[co].name);
			if (nr==SK_SLOW && IS_SEYAN_DU(co))
				do_sayx(cn, "Bring me the item again to learn Weaken, %s!", ch[co].name);
			if (nr==SK_CURSE && IS_SEYAN_DU(co))
				do_sayx(cn, "Bring me the item again to learn Surround Hit, %s!", ch[co].name);
			if (nr==SK_SHIELD && IS_SEYAN_DU(co))
				do_sayx(cn, "Bring me the item again to learn Tactics, %s!", ch[co].name);
			if (nr==SK_IMMUN && IS_SEYAN_DU(co))
				do_sayx(cn, "Bring me the item again to learn Dispel, %s!", ch[co].name);
			// end hack
			
			if (nr == 50) // Special value for Lockpicking
			{
				do_sayx(cn, "Now I'll teach you how to pick locks.");
				
				if (ch[co].flags & CF_LOCKPICK)
				{
					do_sayx(cn, "But you already know how to pick locks, %s!", ch[co].name);
					if ((nr = ch[cn].data[51])!=0)
					{
						do_sayx(cn, "Well, let me teach you a couple of small life lessons instead...");
						do_give_exp(co, (nr/4), 0, -1, 0);
					}
					use_consume_item(cn, in, 1);
				}
				else
				{
					do_sayx(cn, "You will need a lockpick, and I happen to have a spare. Take it, please.");
					use_consume_item(cn, in, 1);
					in = god_create_item(93);
					god_give_char(in, co);
					do_sayx(cn, "You can LOOK at a door and you will be able to tell you how hard it is to pick it.");
					do_sayx(cn, "If you're able to pick the lock, HOLD the lockpick in your hand and then USE the door.");
					ch[co].flags |= CF_LOCKPICK;
					do_char_log(co, 0, "You learned how to pick locks!\n");
					do_update_char(co);
					if ((nr = ch[cn].data[51])!=0)
					{
						do_sayx(cn, "Now I'll teach you a bit about life, the world and everything, %s.", ch[co].name);
						do_give_exp(co, nr, 0, -1, 0);
					}
				}
				// <group rewards>
				for (n = 1; n<MAXCHARS; n++)
				{
					if (ch[n].used==USE_EMPTY || !(ch[n].flags & (CF_PLAYER | CF_USURP))) continue;
					if (isgroup(n, co) && isgroup(co, n) && isnearby(co, n))
					{
						int div = 4;
						if (!(ch[n].flags & CF_LOCKPICK))
						{
							in = god_create_item(93);
							god_give_char(in, n);
							ch[n].flags |= CF_LOCKPICK;
							do_char_log(n, 0, "You learned how to pick locks!\n");
							div = 1;
						}
						if ((nr = ch[cn].data[51])!=0) do_give_exp(n, nr/max(1,div), 0, -1, 0);
					}
				}
				// </group rewards>
			}
			else if (nr == 51) // Special value to set Temple of Skua as your spawn point
			{
				if (ch[co].temple_x==HOME_START_X)
				{
					do_sayx(cn, "I see, so he's dead now. A shame, but it had to be done. Here is your reward.");
					money = 10000;
					ch[co].gold += money;
					do_char_log(co, 2, "You received %dG %dS.\n", money / 100, money % 100);
					for (n = 0; n<MAXBUFFS; n++)	// Remove any lingering Recall spells since it would warp back to the previous spawn
					{
						if ((in2 = ch[co].spell[n]) && (bu[in2].temp==SK_RECALL))
						{
							ch[co].spell[n] = 0;
							bu[in2].used = USE_EMPTY;
						}
					}
					if (IS_PURPLE(co))
					{
						do_sayx(cn, "For your effort, I will now set your spawn point to Aston, in the Temple of the Purple One.");
						do_char_log(n, 0, "Your spawn point was set to the Staffer's Lounge.\n");
						ch[co].temple_x = ch[co].tavern_x = HOME_PURPLE_X;
						ch[co].temple_y = ch[co].tavern_y = HOME_PURPLE_Y;
					}
					else if (ch[co].flags & CF_STAFF)
					{
						do_sayx(cn, "For your effort, I will now set your spawn point to the Staffer's Lounge.");
						do_char_log(n, 0, "Your spawn point was set to the Staffer's Lounge.\n");
						ch[co].temple_x = ch[co].tavern_x = HOME_STAFF_X;
						ch[co].temple_y = ch[co].tavern_y = HOME_STAFF_Y;
					}
					else
					{
						do_sayx(cn, "For your effort, I will now set your spawn point to Aston, in the Temple of Skua.");
						do_char_log(n, 0, "Your spawn point was set to the Temple of Skua.\n");
						ch[co].temple_x = ch[co].tavern_x = HOME_TEMPLE_X;
						ch[co].temple_y = ch[co].tavern_y = HOME_TEMPLE_Y;
					}
					fx_add_effect(6, 0, ch[co].x, ch[co].y, 0);
					if ((nr = ch[cn].data[51])!=0)
					{
						do_sayx(cn, "Now I'll teach you a bit about life, the world and everything, %s.", ch[co].name);
						do_give_exp(co, nr, 0, -1, 0);
					}
					use_consume_item(cn, in, 1);
					
					// <group rewards>
					for (n = 1; n<MAXCHARS; n++)
					{
						if (ch[n].used==USE_EMPTY || !(ch[n].flags & (CF_PLAYER | CF_USURP))) continue;
						if (ch[n].temple_x!=HOME_START_X) continue;
						if (isgroup(n, co) && isgroup(co, n) && isnearby(co, n))
						{
							ch[n].gold += money;
							do_char_log(n, 2, "You received %dG %dS as part of %s's reward.\n", money/100, money%100, ch[co].name);
							if ((nr = ch[cn].data[51])!=0) do_give_exp(n, nr, 0, -1, 0);
							if (IS_PURPLE(n))
							{
								do_char_log(n, 0, "Your spawn point was set to the Temple of the Purple One as part of %s's reward.\n", ch[co].name);
								ch[n].temple_x = ch[n].tavern_x = HOME_PURPLE_X; 
								ch[n].temple_y = ch[n].tavern_y = HOME_PURPLE_Y;
							}
							else if (ch[n].flags & CF_STAFF)
							{
								do_char_log(n, 0, "Your spawn point was set to the Staffer's Lounge as part of %s's reward.\n", ch[co].name);
								ch[n].temple_x = ch[n].tavern_x = HOME_STAFF_X; 
								ch[n].temple_y = ch[n].tavern_y = HOME_STAFF_Y;
							}
							else
							{
								do_char_log(n, 0, "Your spawn point was set to the Temple of Skua as part of %s's reward.\n", ch[co].name);
								ch[n].temple_x = ch[n].tavern_x = HOME_TEMPLE_X; 
								ch[n].temple_y = ch[n].tavern_y = HOME_TEMPLE_Y;
							}
							do_char_log(n, 0, "Your #quest list has been updated and will now show quests in Aston.\n", ch[co].name);
						}
					}
					// </group rewards>
				}
				else
				{
					do_sayx(cn, "But you already claimed your reward, %s!", ch[co].name);
					god_take_from_char(in, cn);
					god_give_char(in, co);
					do_char_log(co, 1, "%s did not accept the %s.\n", ch[cn].reference, it[in].name);
					return 0;
				}
			}
			else if (nr == 52) // Special value to learn 'Value Appraisal'
			{
				do_sayx(cn, "Now I'll teach you how to appraise items.");
				if (ch[co].flags & CF_APPRAISE)
				{
					do_sayx(cn, "But you already know how to appraise items, %s!", ch[co].name);
					if ((nr = ch[cn].data[51])!=0)
					{
						do_sayx(cn, "Well, let me teach you a couple of small life lessons instead...");
						do_give_exp(co, nr/4, 0, -1, 0);
					}
					use_consume_item(cn, in, 1);
				}
				else
				{
					do_sayx(cn, "Simply look at any item in your possession, and you will now know its true value.");
					use_consume_item(cn, in, 1);
					ch[co].flags |= CF_APPRAISE;
					do_char_log(co, 0, "You learned how to appraise items!\n");
					do_update_char(co);
					if ((nr = ch[cn].data[51])!=0)
					{
						do_sayx(cn, "Now I'll teach you a bit about life, the world and everything, %s.", ch[co].name);
						do_give_exp(co, nr, 0, -1, 0);
					}
				}
				// <group rewards>
				for (n = 1; n<MAXCHARS; n++)
				{
					if (ch[n].used==USE_EMPTY || !(ch[n].flags & (CF_PLAYER | CF_USURP))) continue;
					if (isgroup(n, co) && isgroup(co, n) && isnearby(co, n))
					{
						int div = 4;
						if (!(ch[n].flags & CF_APPRAISE))
						{
							ch[n].flags |= CF_APPRAISE;
							do_char_log(n, 0, "You learned how to appraise items!\n");
							do_update_char(n);
							div = 1;
						}
						if ((nr = ch[cn].data[51])!=0) do_give_exp(n, nr/max(1,div), 0, -1, 0);
					}
				}
				// </group rewards>
			}
			else if (nr == 53) // Sense Magic flag
			{
				do_sayx(cn, "Now I'll teach you how to sense magic.");
				if (ch[co].flags & CF_SENSE)
				{
					do_sayx(cn, "But you already know how to sense magic, %s!", ch[co].name);
					god_take_from_char(in, cn);
					god_give_char(in, co);
					do_char_log(co, 1, "%s did not accept the %s.\n", ch[cn].reference, it[in].name);
					return 0;
				}
				else
				{
					do_sayx(cn, "You will now notice a magic item when it's dropped if you can see it. Additionally, you may now notice when enemies try to cast spells on you.");
					use_consume_item(cn, in, 1);
					ch[co].flags |= CF_SENSE;
					do_char_log(co, 0, "You learned how to sense magic!\n");
					do_update_char(co);
					if ((nr = ch[cn].data[51])!=0)
					{
						do_sayx(cn, "Now I'll teach you a bit about life, the world and everything, %s.", ch[co].name);
						do_give_exp(co, nr, 0, -1, 0);
					}
				}
				// <group rewards>
				for (n = 1; n<MAXCHARS; n++)
				{
					if (ch[n].used==USE_EMPTY || !(ch[n].flags & (CF_PLAYER | CF_USURP))) continue;
					if (isgroup(n, co) && isgroup(co, n) && isnearby(co, n))
					{
						int div = 4;
						if (!(ch[n].flags & CF_SENSE))
						{
							ch[n].flags |= CF_SENSE;
							do_char_log(n, 0, "You learned how to sense magic!\n");
							do_update_char(n);
							div = 1;
						}
						if ((nr = ch[cn].data[51])!=0) do_give_exp(n, nr/max(1,div), 0, -1, 0);
					}
				}
				// </group rewards>
			}
			else if (nr == 54) // Identify flag
			{
				do_sayx(cn, "Now I'll teach you Identify.");
				if (ch[co].kindred & KIN_IDENTIFY)
				{
					do_sayx(cn, "But you already know how Identify, %s!", ch[co].name);
					if ((nr = ch[cn].data[51])!=0)
					{
						do_sayx(cn, "Well, let me teach you a couple of small life lessons instead...");
						do_give_exp(co, nr/4, 0, -1, 0);
					}
					use_consume_item(cn, in, 1);
				}
				else
				{
					use_consume_item(cn, in, 1);
					ch[co].kindred |= KIN_IDENTIFY;
					do_char_log(co, 0, "You learned Identify!\n");
					do_update_char(co);
					if ((nr = ch[cn].data[51])!=0)
					{
						do_sayx(cn, "Now I'll teach you a bit about life, the world and everything, %s.", ch[co].name);
						do_give_exp(co, nr, 0, -1, 0);
					}
				}
				// <group rewards>
				for (n = 1; n<MAXCHARS; n++)
				{
					if (ch[n].used==USE_EMPTY || !(ch[n].flags & (CF_PLAYER | CF_USURP))) continue;
					if (isgroup(n, co) && isgroup(co, n) && isnearby(co, n))
					{
						int div = 4;
						if (!(ch[n].kindred & KIN_IDENTIFY))
						{
							ch[n].kindred |= KIN_IDENTIFY;
							do_char_log(n, 0, "You learned Identify!\n");
							do_update_char(n);
							div = 1;
						}
						if ((nr = ch[cn].data[51])!=0) do_give_exp(n, nr/max(1,div), 0, -1, 0);
					}
				}
				// </group rewards>
			}
			else if (nr == 55) // Special value to learn 'Spell Knowledge'
			{
				do_sayx(cn, "Now I'll teach you how to understand spell effects.");
				if (ch[co].flags & CF_KNOWSPELL)
				{
					do_sayx(cn, "But you already know how to understand spell effects, %s!", ch[co].name);
					if ((nr = ch[cn].data[51])!=0)
					{
						do_sayx(cn, "Well, let me teach you a couple of small life lessons instead...");
						do_give_exp(co, nr/4, 0, -1, 0);
					}
					use_consume_item(cn, in, 1);
				}
				else
				{
					do_sayx(cn, "Simply cast identify or use the #buffs command, and you will now know what each spell is doing to you!");
					use_consume_item(cn, in, 1);
					ch[co].flags |= CF_KNOWSPELL;
					do_char_log(co, 0, "You learned how to understand spell effects!\n");
					do_update_char(co);
					if ((nr = ch[cn].data[51])!=0)
					{
						do_sayx(cn, "Now I'll teach you a bit about life, the world and everything, %s.", ch[co].name);
						do_give_exp(co, nr, 0, -1, 0);
					}
				}
				// <group rewards>
				for (n = 1; n<MAXCHARS; n++)
				{
					if (ch[n].used==USE_EMPTY || !(ch[n].flags & (CF_PLAYER | CF_USURP))) continue;
					if (isgroup(n, co) && isgroup(co, n) && isnearby(co, n))
					{
						int div = 4;
						if (!(ch[n].flags & CF_KNOWSPELL))
						{
							ch[n].flags |= CF_KNOWSPELL;
							do_char_log(n, 0, "You learned how to understand spell effects!\n");
							do_update_char(n);
							div = 1;
						}
						if ((nr = ch[cn].data[51])!=0) do_give_exp(n, nr/max(1,div), 0, -1, 0);
					}
				}
				// </group rewards>
			}
			else if ((qnum = nr) >= 101) // Flags for quests which don't teach skills
			{
				tmp = npc_quest_check(co, qnum-101); // Returns 0 if not clear, 1 if already cleared
			}
			else if (IS_LYCANTH(co) && (nr==SK_TAUNT||nr==SK_TACTICS||nr==SK_SLOW||nr==SK_CURSE))
			{
				int div = 4, nr3;
				nr2 = nr;
				
				switch (nr)
				{
					case SK_TAUNT:   nr3 = SK_POISON;   break; // learn Taunt AND Poison
					case SK_TACTICS: nr3 = SK_BLIND;    break; // learn Tactics AND Blind
					case SK_SLOW:    nr3 = SK_WEAKEN;   break; // learn Slow AND Weaken
					case SK_CURSE:   nr3 = SK_SURROUND; break; // learn Curse AND Surround Hit
					default: 
						god_take_from_char(in, cn);
						god_give_char(in, co);
						do_char_log(co, 1, "%s did not accept the %s.\n", ch[cn].reference, it[in].name);
						return 0;
				}
				
				do_sayx(cn, "Now I'll teach you %s and %s.", skilltab[nr].name, skilltab[nr3].name);
				if (B_SK(co, nr) && B_SK(co, nr3))
				{
					do_sayx(cn, "But you already know those skills, %s!", ch[co].name);
					if ((nr = ch[cn].data[51])!=0)
					{
						do_sayx(cn, "Well, let me teach you a couple of small life lessons instead...");
						do_give_exp(co, nr/4 + (nr*(stsz-1))/4, 0, -1, 0);
					}
					use_consume_item(cn, in, 1);
				}
				else
				{
					B_SK(co, nr)  = 1;
					B_SK(co, nr3) = 1;
					do_char_log(co, 0, "You learned %s and %s!\n", skilltab[nr].name, skilltab[nr3].name);
					do_update_char(co);
					
					if ((nr = ch[cn].data[51])!=0)
					{
						do_sayx(cn, "Now I'll teach you a bit about life, the world and everything, %s.", ch[co].name);
						do_give_exp(co, nr + (nr*(stsz-1))/4, 0, -1, 0);
					}
					use_consume_item(cn, in, 1);
				}
				// <group rewards>
				for (n = 1; n<MAXCHARS; n++)
				{
					if (ch[n].used==USE_EMPTY || !(ch[n].flags & (CF_PLAYER | CF_USURP))) continue;
					if (isgroup(n, co) && isgroup(co, n) && isnearby(co, n))
					{
						nr2 = convert_skill_for_group(n, nr2);
						if (!B_SK(n, nr2) && ch[n].skill[nr2][2])
						{
							B_SK(n, nr2) = 1;
							do_char_log(n, 0, "You learned %s!\n", skilltab[nr2].name);
							do_update_char(n);
							div = 1;
						}
						if ((nr2 = ch[cn].data[51])!=0) do_give_exp(n, nr2/max(1,div) + (nr*(stsz-1))/4, 0, -1, 0);
					}
				}
				// </group rewards>
			}
			else
			{
				int div = 4;
				nr2 = nr;
				if (IS_LYCANTH(co) && IS_SHIFTED(co) && nr==SK_RAGE)
					do_sayx(cn, "Now I'll teach you Calm.");
				else
					do_sayx(cn, "Now I'll teach you %s.", skilltab[nr].name);
				if (!ch[co].skill[nr2][2])
				{
					do_sayx(cn, "But you can't learn %s, %s!", skilltab[nr].name, ch[co].name);
					god_take_from_char(in, cn);
					god_give_char(in, co);
					do_char_log(co, 1, "%s did not accept the %s.\n", ch[cn].reference, it[in].name);
					return 0;
				}
				else if (B_SK(co, nr))
				{
					if (IS_LYCANTH(co) && IS_SHIFTED(co) && nr==SK_RAGE)
						do_sayx(cn, "But you already know Calm, %s!", ch[co].name);
					else
						do_sayx(cn, "But you already know %s, %s!", skilltab[nr].name, ch[co].name);
					if ((nr = ch[cn].data[51])!=0)
					{
						do_sayx(cn, "Well, let me teach you a couple of small life lessons instead...");
						do_give_exp(co, nr/4 + (nr*(stsz-1))/4, 0, -1, 0);
					}
					use_consume_item(cn, in, 1);
				}
				else
				{
					B_SK(co, nr) = 1;
					if (IS_LYCANTH(co) && IS_SHIFTED(co) && nr==SK_RAGE)
						do_char_log(co, 0, "You learned Calm!\n");
					else
						do_char_log(co, 0, "You learned %s!\n", skilltab[nr].name);
					do_update_char(co);
					
					if ((nr = ch[cn].data[51])!=0)
					{
						do_sayx(cn, "Now I'll teach you a bit about life, the world and everything, %s.", ch[co].name);
						do_give_exp(co, nr + (nr*(stsz-1))/4, 0, -1, 0);
					}
					use_consume_item(cn, in, 1);
				}
				// <group rewards>
				for (n = 1; n<MAXCHARS; n++)
				{
					if (ch[n].used==USE_EMPTY || !(ch[n].flags & (CF_PLAYER | CF_USURP))) continue;
					if (isgroup(n, co) && isgroup(co, n) && isnearby(co, n))
					{
						nr2 = convert_skill_for_group(n, nr2);
						if (!B_SK(n, nr2) && ch[n].skill[nr2][2])
						{
							B_SK(n, nr2) = 1;
							if (IS_LYCANTH(n) && IS_SHIFTED(n) && nr2==SK_RAGE)
								do_char_log(n, 0, "You learned Calm!\n");
							else
								do_char_log(n, 0, "You learned %s!\n", skilltab[nr2].name);
							do_update_char(n);
							div = 1;
						}
						if ((nr2 = ch[cn].data[51])!=0) do_give_exp(n, nr2/max(1,div) + (nr*(stsz-1))/4, 0, -1, 0);
					}
				}
				// </group rewards>
			}
		}
		
		if ((nr2 = ch[cn].data[50]) && (IS_ANY_MERC(co) || IS_SEYAN_DU(co)) && nr2 == 117) nr2 = SK_HASTE;
		
		/* items with a return gift - SK_HASTE is special since we don't want Regal giving a book with the skill */
		if (nr2 != SK_HASTE && (nr = ch[cn].data[66])!=0)
		{
			use_consume_item(cn, in, 1);
			if (nr == 1354) nr = 1354 + RANDOM(6); // Medium Gems
			if (nr == 1360) // Big Gems
			{
				switch (RANDOM(10))
				{
					case  0: nr = 1360; break; case  1: nr = 1361; break; case  2: nr = 1362; break;
					case  3: nr = 1363; break; case  4: nr = 1364; break; case  5: nr = 1365; break;
					case  6: nr = 2057; break; case  7: nr = 2058; break; case  8: nr = 2059; break;
					default: nr = 2060; break;
				}
			}
			if (nr == 1366) // Huge Gems
			{
				switch (RANDOM(13))
				{
					case  0: nr = 1366; break; case  1: nr = 1367; break; case  2: nr = 1368; break;
					case  3: nr = 1369; break; case  4: nr = 1370; break; case  5: nr = 1371; break;
					case  6: nr = 2061; break; case  7: nr = 2062; break; case  8: nr = 2063; break;
					case  9: nr = 2064; break; case 10: nr = 2065; break; case 11: nr = 2066; break;
					default: nr = 2687; break;
				}
			}
			if (nr == 3343) // Signet Rings (Tactician)
			{
				if (IS_ANY_MERC(co)||IS_SEYAN_DU(co)) nr = 3344;
				if (IS_ANY_HARA(co)|| IS_LYCANTH(co)) nr = 3345;
			}
			if (nr == 3346) // Arch Signet Rings (Damor)
			{
				if (IS_SKALD(co)) 		nr = 3347;
				if (IS_WARRIOR(co)) 	nr = 3348;
				if (IS_SORCERER(co)) 	nr = 3349;
				if (IS_SUMMONER(co)) 	nr = 3350;
				if (IS_ARCHHARAKIM(co)) nr = 3351;
				if (IS_SEYAN_DU(co)) 	nr = 3352;
				if (IS_BRAVER(co)) 		nr = 3353;
				if (IS_LYCANTH(co)) 	nr = 3354;
			}
			if (!(ch[cn].flags&(CF_BODY))) 
			{
				if ((nr>=182 && nr<=186) && tmp) ;
				else
				{
					do_sayx(cn, "Here is your %s in exchange.", it_temp[nr].reference);
					in = god_create_item(nr);
					god_give_char(in, co);
					if (it[in].driver==52)
					{
						char buf[300];
						it[in].data[0] = co;
						sprintf(buf, "%s Engraved in it are the letters \"%s\".", it[in].description, ch[co].name);
						if (strlen(buf)<200) strcpy(it[in].description, buf);
					}
				}
				if ((nr = ch[cn].data[51])!=0)
				{
					int div = 4;
					do_sayx(cn, "For your effort, allow me to teach you some mysteries of the world.");
					do_give_exp(co, (tmp ? nr/max(1,div) : nr) + (qnum ? (nr*(stsz-1))/4 : nr*(stsz-1)), 0, -1, 0);
					// <group rewards>
					for (n = 1; n<MAXCHARS; n++)
					{
						if (ch[n].used==USE_EMPTY || !(ch[n].flags & (CF_PLAYER | CF_USURP))) continue;
						if (isgroup(n, co) && isgroup(co, n) && isnearby(co, n))
						{
							div = 4;
							if (qnum >= 101) tmp = npc_quest_check(n, qnum-101);
							do_give_exp(n, (tmp ? nr/max(1,div) : nr) + (qnum ? (nr*(stsz-1))/4 : nr*(stsz-1)), 0, -1, 0);
						}
					}
					// </group rewards>
				}
			}
		}
		else if ((money = ch[cn].data[69]*stsz)!=0)
		{
			if (money < 10000)
				do_sayx(cn, "Here is a small token of gratitude, %s.", ch[co].name);
			else
				do_sayx(cn, "Here is your reward in exchange, %s!", ch[co].name);
			use_consume_item(cn, in, 1);
			ch[co].gold += money;
			do_char_log(co, 2, "You received %dG %dS.\n", money / 100, money % 100);
			if ((nr = ch[cn].data[51])!=0)
			{
				do_sayx(cn, "As an extra thanks, let me teach you a little something I know.");
				do_give_exp(co, (tmp ? nr/4 : nr) + (qnum ? (nr*(stsz-1))/4 : nr*(stsz-1)), 0, -1, 0);
				// <group rewards>
				for (n = 1; n<MAXCHARS; n++)
				{
					if (ch[n].used==USE_EMPTY || !(ch[n].flags & (CF_PLAYER | CF_USURP))) continue;
					if (isgroup(n, co) && isgroup(co, n) && isnearby(co, n))
					{
						do_give_exp(n, (tmp ? nr/4 : nr) + (qnum ? (nr*(stsz-1))/4 : nr*(stsz-1)), 0, -1, 0);
					}
				}
				// </group rewards>
			}
		}

		/* special for riddle givers */
		ar = ch[cn].data[72];
		if (IS_PLAYER(co) && (ar >= RIDDLE_MIN_AREA) && (ar <= RIDDLE_MAX_AREA))
		{
			int idx;

			/* determine which of the 5 riddlers are active */
			idx = ar - RIDDLE_MIN_AREA;

			if ((guesser[idx] != 0) && (guesser[idx] != co))
			{
				do_sayx(cn, "I'm still riddling %s; please come back later!\n", ch[guesser[idx]].name);
				god_take_from_char(in, cn);
				god_give_char(in, co);
				return 0;
			}

			/* ok to destroy the gift now */
			use_consume_item(co, in, 1);
			/* Select and ask a riddle */
			lab9_pose_riddle(cn, co);
		}

	}
	else if (!in && money)
	{
		do_sayx(cn, "I'm sure your coin is better in your pocket than mine, %s!", ch[co].name);
		ch[co].gold += money;
		ch[cn].gold -= money;
		do_char_log(co, 1, "%s did not accept the gold.\n", ch[cn].reference);
	}
	else if (IS_PLAYER_GC(cn) && co == CN_OWNER(cn) && it[in].driver==117) // Companion sprite set
	{
		fx_add_effect(7, 0, ch[cn].x, ch[cn].y, 0);
		if (it[in].data[1])
		{
			ch[cn].sprite = 15312; ch[co].class  = 0;
			do_char_log(co, 1, "Your ghost companion's form returned to normal.\n");
		}
		else if (ch[cn].sprite == it[in].data[0])
		{
			int v;
			v = it[in].data[0];
			switch (v)
			{
				case  3024: v = 11216; break; // Seyan'du
				case 23504: v = 24528; break; // Arch Temp
				case 26048: v = 22948; break; // Skald
				case 25552: v = 26576; break; // Warrior
				case 28624: v = 27600; break; // Sorcerer
				case 25048: v = 24048; break; // Summoner
				case 29648: v = 30672; break; // Arch Hara
				case 27088: v = 28112; break; // Braver
				default: break;
			}
			ch[cn].sprite = ch[co].class = v;
			do_char_log(co, 1, "Your ghost companion's form shifts and changes...\n");
		}
		else
		{
			ch[cn].sprite = ch[co].class = it[in].data[0];
			do_char_log(co, 1, "Your ghost companion's form shifts and changes...\n");
		}
		god_take_from_char(in, cn);
	}
	else
	{
		god_take_from_char(in, cn);
		god_give_char(in, co);
		do_char_log(co, 1, "%s did not accept the %s.\n", ch[cn].reference, it[in].name);
	}

	return 0;
}

int npc_cityguard_see(int cn, int co, int flag)
{
	int n;

	if (ch[co].data[CHD_GROUP]==601 || ch[co].data[CHD_GROUP]==602 || ch[co].data[CHD_GROUP]==603)   // shout if enemy in sight (!)
	{
		if (ch[cn].data[55] + TICKS * 180<globs->ticker) // shout every 180 seconds
		{
			ch[cn].data[54] = 0;
			ch[cn].data[55] = globs->ticker;
			npc_saytext_n(cn, 4, ch[co].name);
			do_npc_shout(cn, NT_SHOUT, cn, ch[cn].data[52], ch[cn].x, ch[cn].y);

			// shout for players too
			for (n = 1; n<MAXCHARS; n++)
			{
				if ((ch[n].flags & (CF_PLAYER | CF_USURP)) && ch[n].used==USE_ACTIVE && !(ch[n].flags & CF_NOSHOUT))
				{
					if (flag)
					{
						if (ch[co].data[CHD_GROUP]==601 
							  && (is_inline(n, 1) || (
							     getrank(n)>= 5  // Staff Sergeant
							  && getrank(n)<= 8))) // Sergeant Major
							do_char_log(n, 3, "Gate Guard: \"The monsters are approaching the north gate! Alert!\"\n");
						else if (ch[co].data[CHD_GROUP]==602
							  && (is_inline(n, 2) || (
							     getrank(n)>= 9  // Second Lieutenant
							  && getrank(n)<=13))) // Lieutenant Colonel
							do_char_log(n, 3, "Gate Guard: \"The monsters are approaching the center gate! Alert!\"\n");
						else if (ch[co].data[CHD_GROUP]==603
							  && (is_inline(n, 3) || (
							     getrank(n)>=14))) // Colonel
							do_char_log(n, 3, "Gate Guard: \"The monsters are approaching the south gate! Alert!\"\n");
					}
					else
					{
						if (ch[co].data[CHD_GROUP]==601
							  && (is_inline(n, 1) || (
							     getrank(n)>= 5  // Staff Sergeant
							  && getrank(n)<= 8))) // Sergeant Major
							do_char_log(n, 3, "Outpost Guard: \"The monsters are approaching the north outpost! Alert!\"\n");
						else if (ch[co].data[CHD_GROUP]==602
							  && (is_inline(n, 2) || (
							     getrank(n)>= 9  // Second Lieutenant
							  && getrank(n)<=13))) // Lieutenant Colonel
							do_char_log(n, 3, "Outpost Guard: \"The monsters are approaching the center outpost! Alert!\"\n");
						else if (ch[co].data[CHD_GROUP]==603
							  && (is_inline(n, 3) || (
							     getrank(n)>=14))) // Colonel
							do_char_log(n, 3, "Outpost Guard: \"The monsters are approaching the south outpost! Alert!\"\n");
					}
				}
			}
		}
	}

	return 0;
}

int is_osiris_weap(int temp)
{
	int n, m = 0;
	static int validitem[40] = {
		2515, 2519, 2523, 2527, 2531, 2536, 2540, 2544, 2548, 2552,
		2514, 2518, 2522, 2526, 2530, 2535, 2539, 2543, 2547, 2551,
		2513, 2517, 2521, 2525, 2529, 2534, 2538, 2542, 2546, 2550,
		2512, 2516, 2520, 2524, 2528, 2533, 2537, 2541, 2545, 2549
	};
	
	for (n = 0; n<40; n++)
	{
		if (temp == validitem[n]) { m = 1+n/10; break; }
	}
	
	return m;
}

int is_unique_able(int temp)
{
	int n, m = 0;
	static int validitem[60] = {
	//	Dag  Sta  Spe  Shi  Swo  Dua  Axe  Two  Gre
		284, 285, 286, 287, 288, 289, 290, 291, 292, // Steel
		523, 524, 525, 526, 527, 528, 529, 530, 531, // Gold
		532, 533, 534, 535, 536, 537, 538, 539, 540, // Emerald
		541, 542, 543, 544, 545, 546, 547, 548, 549, // Crystal
		572, 573, 574, 575, 576, 577, 578, 579, 580, // Titanium		// 45
		693, 694, 695, 696, 697, 698, 699, 700, 701, // Adamantium		// 54
		1779, 1780, 1781, 1782, 1783, 1784			 // Claws			// 60
	};
	
	for (n = 0; n<60; n++)
	{
		if (temp == validitem[n]) { m = n+1; break; }
	}
	
	return m;
}

int count_uniques(int cn)
{
	int n, in, cnt = 0;
	
	if (IS_BUILDING(cn)) return 0;
	
	if (IS_SANEITEM(in = ch[cn].citem) && !(in & 0x80000000) && IS_UNIQUE(in))									cnt++;
	for (n = 0; n<MAXITEMS; n++)			if ((in = ch[cn].item[n]) && IS_UNIQUE(in)) 						cnt++;
	for (n = 0; n<20; n++)					if ((in = ch[cn].worn[n]) && IS_UNIQUE(in))							cnt++;
	for (n = 0; n<12; n++)					if ((in = ch[cn].alt_worn[n]) && IS_UNIQUE(in))						cnt++;
//	for (n = 0; n<62; n++)					if ((in = ch[cn].depot[n]) && IS_UNIQUE(in))						cnt++;
	for (n = 0; n<ST_PAGES*ST_SLOTS; n++) 	if ((in = st[cn].depot[n/ST_SLOTS][n%ST_SLOTS]) && IS_UNIQUE(in)) 	cnt++;
	
	return(cnt);
}

int npc_see(int cn, int co)
{
	int n, n2, m, idx, indoor1, indoor2;
	int x1, x2, y1, y2, dist, ret, cnt, cc;
	unsigned char buf[3];

	if (ch[co].flags & (CF_PLAYER | CF_USURP))
	{
		ch[cn].data[92] = TICKS * 60;
	}
	else if (!group_active(cn))
	{
		return 0;
	}

	if (!do_char_can_see(cn, co, 0))
	{
		return 1;                     // processed it: we cannot see him, so ignore him
	}
	
	
	/* CS, 000209: Check for Ghost Companion seeing his master */
	if (IS_COMP_TEMP(cn) && co == ch[cn].data[CHD_MASTER])
	{
		// happy to see master, timeout reset
		ch[cn].data[98] = globs->ticker + COMPANION_TIMEOUT;
	}

	// special sub driver
	if (ch[cn].data[26])
	{
		switch(ch[cn].data[26])
		{
		case  1:
			ret = npc_cityguard_see(cn, co, 0);
			break;
		case  3:
			ret = npc_cityguard_see(cn, co, 1);
			break;
		default:
			ret = 0;
			break;
		}
		if (ret)
		{
			return 1;
		}
	}

	if (map[ch[cn].x + ch[cn].y * MAPX].flags & MF_INDOORS)
	{
		indoor1 = 1;
	}
	else
	{
		indoor1 = 0;
	}
	if (map[ch[co].x + ch[co].y * MAPX].flags & MF_INDOORS)
	{
		indoor2 = 1;
	}
	else
	{
		indoor2 = 0;
	}
	
	// if we're taunted, try to attack the taunter
	if ((cc = ch[cn].taunted) && IS_SANECHAR(cc) && (do_char_can_see(cn, cc, 0) || ch[cn].data[78]))
	{
		// If our last attempt to attack failed, wander near the taunter
		if (!ch[cn].attack_cn && !ch[cn].goto_x && ch[cn].data[78] && ch[cn].data[27]!=1)
		{
			ch[cn].goto_x = ch[cc].x + 5 - RANDOM(10);
			ch[cn].goto_y = ch[cc].y + 5 - RANDOM(10);
		}
		// Otherwise, try to attack the taunter
		else if (do_char_can_see(cn, cc, 0) && ch[cn].attack_cn!=cc)
		{
			ch[cn].attack_cn = cc;
			if (!ch[cn].data[78]) ch[cn].goto_x = 0;
			ch[cn].data[78] = globs->ticker + TICKS * 5;
		}
		ch[cn].data[58] = 2;
		return 1;
	}
	
	// check if this is an enemy we added to our list earlier
	if (!ch[cn].attack_cn)   // only attack him if we aren't fighting already
	{
		idx = co | (char_id(co) << 16);

		for (n = MCD_ENEMY1ST; n<=MCD_ENEMYZZZ; n++)
		{
			if (ch[cn].data[n]==idx)
			{
				ch[cn].attack_cn = co;
				ch[cn].goto_x = 0; // cancel goto (patrol)
				ch[cn].data[58] = 2;
				return 1;
			}
		}
	}
	
	// check if we need to attack him (by group)
	if (ch[cn].data[43])
	{
		for (n = 43; n<47; n++)
		{
			if (ch[cn].data[n] && ch[co].data[CHD_GROUP]==ch[cn].data[n])
			{
				break;
			}
			if (ch[cn].data[n]==65536 && (IS_PLAYER(co) || IS_COMP_TEMP(co)))
			{
				break;
			}
			if ((IS_COMP_TEMP(co) || IS_SHADOW(co)) && !ch[co].data[CHD_GROUP]) // Give benefit of the doubt to new GC's and SC's
			{
				break;
			}
		}
		if (n==47)
		{
			int ccc;
			if (ch[cn].data[95]==2 && ch[cn].data[93])   // attack distance
			{
				if (ch[cn].temp==1498||ch[cn].temp==1499) // Enforcers only check the direction they face
				{
					switch (ch[cn].dir)
					{
						case  1: dist = max(0, ch[co].x - (ch[cn].data[29] % MAPX)); if (abs((ch[cn].data[29] / MAPX) - ch[co].y)!=0) dist=9; break; // south (x++)
						case  2: dist = max(0, (ch[cn].data[29] % MAPX) - ch[co].x); if (abs((ch[cn].data[29] / MAPX) - ch[co].y)!=0) dist=9; break; // north (x--)
						case  3: dist = max(0, (ch[cn].data[29] / MAPX) - ch[co].y); if (abs((ch[cn].data[29] % MAPX) - ch[co].x)!=0) dist=9; break; // west (y--)
						case  4: dist = max(0, ch[co].y - (ch[cn].data[29] / MAPX)); if (abs((ch[cn].data[29] % MAPX) - ch[co].x)!=0) dist=9; break; // east (y++)
						default: dist = 9; break;
					}
					if (dist==0) dist = 9;
				}
				else
				{
					dist = max(abs((ch[cn].data[29] % MAPX) - ch[co].x), abs((ch[cn].data[29] / MAPX) - ch[co].y));
				}
				if (dist>ch[cn].data[93])
				{
					ccc = co;
					co = 0;
				}
			}
			if (co && npc_add_enemy(cn, co, 0))
			{
				if (!(ch[co].flags & CF_SILENCE) && !(ch[cn].flags & CF_SILENCE))
				{
					npc_saytext_n(cn, 1, ch[co].name);
				}
				chlog(cn, "Added %s to kill list because he's not in my group", ch[co].name);
				return 1;
			}
			co = ccc;
		}
	}
	
	if (IS_COMP_TEMP(cn) && ch[cn].data[1]>=2 && ((ch[cn].alignment>0 && ch[co].alignment<0) || (ch[cn].alignment<0 && ch[co].alignment>0)))
	{
		int mdist, coma, idx2;

		if (ch[cn].data[1]==2 && (coma=ch[cn].data[CHD_MASTER])) // Offense Mode
		{
			idx = coma | (char_id(coma) << 16);
			idx2 = co | (char_id(co) << 16);
			for (n = MCD_ENEMY1ST; n<=MCD_ENEMYZZZ; n++)
			{
				if (ch[co].data[n]==idx && ch[cn].data[91]!=idx2) // check enemy's kill list. If the master is on it, we fight them.
				{
					break;
				}
			}
			if (n==MCD_ENEMYZZZ+1)
			{
				co = 0;
			}
		}
		
		if (co && npc_add_enemy(cn, co, 1))
		{
			if (!(ch[co].flags & CF_SILENCE) && !(ch[cn].flags & CF_SILENCE) && ch[cn].data[55]+TICKS*2<globs->ticker)
			{
				ch[cn].data[55] = globs->ticker;
				npc_saytext_n(cn, 1, ch[co].name);
			}
			chlog(cn, "Added %s to kill list (gc mode %d)", ch[co].name, ch[cn].data[1]);
			return 1;
		}
	}

	// attack with warning
	if (ch[cn].data[95]==1 && (ch[co].flags & (CF_PLAYER)) && globs->ticker>ch[cn].data[27] + TICKS * 120)
	{
		x1 = ch[co].x;
		x2 = ch[cn].data[29] % MAPX;
		y1 = ch[co].y;
		y2 = ch[cn].data[29] / MAPX;
		dist = abs(x1 - x2) + abs(y1 - y2);
		if (dist<=ch[cn].data[93])
		{
			if (npc_add_enemy(cn, co, 0))
			{
				if (!(ch[co].flags & CF_SILENCE) && !(ch[cn].flags & CF_SILENCE))
				{
					npc_saytext_n(cn, 1, ch[co].name);
				}
				chlog(cn, "Added %s to kill list because he didn't say the password.", ch[co].name);
				return 1;
			}
		}
		else if (dist<=ch[cn].data[93] * 2 && ch[cn].data[94] + TICKS * 15<globs->ticker)
		{
			npc_saytext_n(cn, 8, NULL);
			ch[cn].data[94] = globs->ticker;
			return 1;
		}
	}
	
	// check if we need to talk to him
	if (!ch[cn].attack_cn && (ch[co].flags & (CF_PLAYER)) && ch[cn].data[37] && indoor1==indoor2 && ch[cn].data[56]<globs->ticker)
	{
		for (n = 37; n<41; n++)
		{
			if (ch[cn].data[n]==co)
			{
				break;
			}
		}
		if (n==41)
		{
			// Mar 2020 - storing all quest text here instead of in CGI.
			if (strcmp(ch[cn].text[2], "#greetnew")==0) // New player greeting from Innkeeper.
			{
				// Process a short greeting delay if the player has 0 exp (just enough that their name changes first)
				if (ch[co].points_tot==0 && ch[co].total_online_time < TICKS*6)
				{
					return 0;
				}
				
				if (!B_SK(co, SK_ECONOM))
					do_sayx(cn, "Welcome to Bluebird Tavern, %s. If you're looking for adventure, perhaps approach Jamil. He seems a bit down on his luck.", ch[co].name);
				else if (!B_SK(co, SK_REPAIR))
					do_sayx(cn, "Well done, %s. I see Jamil is in a better mood! There may be others around town who need help. If you'd like, say QUEST to me and I shall tell you.", ch[co].name);
				else if (!B_SK(co, SK_REST))
					do_sayx(cn, "Welcome, %s. I've heard great things about you! There may be others around town who need help. If you'd like, say QUEST to me and I shall tell you.", ch[co].name);
				else
					do_sayx(cn, "Welcome back, %s. Enjoy your stay!", ch[co].name);
			}
			else if (strcmp(ch[cn].text[2], "#woodsguard")==0) // One of the guards to Weeping Woods
			{
				if (!B_SK(co, SK_REST)) // Rest - if the player has Rest they're probably ok. Probably.
					do_sayx(cn, "Hello %s. For the time being, please stay inside Lynbore's walls. You're far too weak to go outside for now!", ch[co].name);
				else
					do_sayx(cn, "Hello %s. Are you on your way to Aston? If you find any bandits on the way, please report to Aston's Guard Captain.", ch[co].name);
			}
			else if (strcmp(ch[cn].text[2], "#guardcaptain")==0) // Guard Captain in Aston
			{
				if (ch[co].temple_x==HOME_START_X)
					do_sayx(cn, "Greetings, %s. I am trying to track down Argha, in the Weeping Woods south of here. Have you found anything of his in your travels?", ch[co].name);
				else
					do_sayx(cn, "Greetings, %s. Thank you for your hard work!", ch[co].name);
			}
			else if (strcmp(ch[cn].text[2], "#skill01")==0) //    12		Economize			( Jamil )
			{
				// Tutorial 2
				if (ch[co].data[76]<(1<<2))
				{
					chlog(co, "SV_SHOWMOTD tutorial 2");
					buf[0] = SV_SHOWMOTD;
					*(unsigned char*)(buf + 1) = 102;
					xsend(ch[co].player, buf, 2);
				}
				
				if (!B_SK(co, SK_ECONOM))
					do_sayx(cn, "Hello, %s. Some thieves across the north road stole my gold amulet from me. I'd teach you ECONOMIZE if you could get it back for me.", ch[co].name);
				else
					do_sayx(cn, "Hello, %s! Thanks again for helping me!", ch[co].name);
			}
			else if (strcmp(ch[cn].text[2], "#skill02")==0) //    15		Recall				( Inga )
			{
				if (IS_LYCANTH(co))
				{
					if (!B_SK(co, SK_TAUNT))
						do_sayx(cn, "Greetings, %s. I was attacked and lost my Stone Dagger in the park across the street. If you could return it, I'd teach you TAUNT and POISON.", ch[co].name);
					else
						do_sayx(cn, "Greetings, %s.", ch[co].name);
				}
				else if (IS_ANY_TEMP(co) || IS_BRAVER(co) || (IS_SEYAN_DU(co) && !B_SK(co, SK_TAUNT)))
				{
					if (!B_SK(co, SK_TAUNT))
						do_sayx(cn, "Greetings, %s. I was attacked and lost my Stone Dagger in the park across the street. If you could return it, I'd teach you TAUNT.", ch[co].name);
					else
						do_sayx(cn, "Greetings, %s.", ch[co].name);
				}
				else if (IS_ANY_MERC(co) || (IS_SEYAN_DU(co) && !B_SK(co, SK_POISON)))
				{
					if (!B_SK(co, SK_POISON))
						do_sayx(cn, "Greetings, %s. I was attacked and lost my Stone Dagger in the park across the street. If you could return it, I'd teach you POISON.", ch[co].name);
					else
						do_sayx(cn, "Greetings, %s.", ch[co].name);
				}
				else
				{
					if (!B_SK(co, SK_STAFF))
						do_sayx(cn, "Greetings, %s. I was attacked and lost my Stone Dagger in the park across the street. If you could return it, I'd teach you STAFF.", ch[co].name);
					else
						do_sayx(cn, "Greetings, %s.", ch[co].name);
				}
			}
			else if (strcmp(ch[cn].text[2], "#skill03")==0) //    18/38	* EW or WM			( Sirjan )
			{
				if (IS_ANY_TEMP(co) || IS_BRAVER(co))
				{
					if (!B_SK(co, SK_MSHIELD))
						do_sayx(cn, "Hello, %s. There is an ancient weapon hiding in the cursed tomb past the crossroad. Bring it to me and I shall teach you MAGIC SHIELD.", ch[co].name);
					else
						do_sayx(cn, "Hello, %s!", ch[co].name);
				}
				else
				{
					if (!B_SK(co, SK_ENHANCE))
						do_sayx(cn, "Hello, %s. There is an ancient weapon hiding in the cursed tomb past the crossroad. Bring it to me and I shall teach you ENHANCE.", ch[co].name);
					else
						do_sayx(cn, "Hello, %s!", ch[co].name);
				}
			}
			
			else if (strcmp(ch[cn].text[2], "#skill04")==0) // 41/19   	* Weaken or Slow		( Amity )
			{
				if (IS_LYCANTH(co))
				{
					if (!B_SK(co, SK_WEAKEN))
						do_sayx(cn, "Hello, %s. Crazed Harakim have taken over the library, and stole a precious belt from me. Return it, and I shall teach you WEAKEN and SLOW.", ch[co].name);
					else
						do_sayx(cn, "Hello, %s! Thanks again!", ch[co].name);
				}
				else if (IS_ANY_TEMP(co) || IS_BRAVER(co) 
					|| (IS_SEYAN_DU(co) && B_SK(co, SK_SLOW)))
				{
					if (!B_SK(co, SK_WEAKEN))
						do_sayx(cn, "Hello, %s. Crazed Harakim have taken over the library, and stole a precious belt from me. Return it, and I shall teach you WEAKEN.", ch[co].name);
					else
						do_sayx(cn, "Hello, %s! Thanks again!", ch[co].name);
				}
				else
				{
					if (!B_SK(co, SK_SLOW))
						do_sayx(cn, "Hello, %s. Crazed Harakim have taken over the library, and stole a precious belt from me. Return it, and I shall teach you SLOW.", ch[co].name);
					else
						do_sayx(cn, "Hello, %s! Thanks again!", ch[co].name);
				}
			}
			else if (strcmp(ch[cn].text[2], "#skill05")==0) //    13		Repair				( Jefferson )
			{
				if (!B_SK(co, SK_REPAIR))
					do_sayx(cn, "Bring me the Bronze Ruby Armor Joe stole from me, %s, and I'll teach you REPAIR.", ch[co].name);
				else
					do_sayx(cn, "Welcome, %s. Please, make yourself at home.", ch[co].name);
			}
			else if (strcmp(ch[cn].text[2], "#skill06")==0) //     * 		Lockpick			( Steven )
			{
				if (!(ch[co].flags & CF_LOCKPICK))
					do_sayx(cn, "Greetings, %s. Give me the Barbarian Sword from the Templar Outlaws and I would teach you how to pick locks.", ch[co].name);
				else
					do_sayx(cn, "Greetings, %s. Fine weather we're having.", ch[co].name);
			}
			else if (strcmp(ch[cn].text[2], "#skill07")==0) //    32/25	* Immu or Dispel	( Ingrid )
			{
				if (IS_ANY_HARA(co) || IS_BRAVER(co) 
					|| (IS_SEYAN_DU(co) && B_SK(co, SK_IMMUN)))
				{
					if (!B_SK(co, SK_DISPEL))
						do_sayx(cn, "Welcome, %s. Bring me the Decorative Sword from the Skeleton Lord and I'll teach you DISPEL.", ch[co].name);
					else
						do_sayx(cn, "Welcome, %s.", ch[co].name);
				}
				else
				{
					if (!B_SK(co, SK_IMMUN))
						do_sayx(cn, "Welcome, %s. Bring me the Decorative Sword from the Skeleton Lord and I'll teach you IMMUNITY.", ch[co].name);
					else
						do_sayx(cn, "Welcome, %s.", ch[co].name);
				}
			}
			else if (strcmp(ch[cn].text[2], "#skill08")==0) // 33/20   	* Surr or Curse		( Leopold )
			{
				if (IS_LYCANTH(co))
				{
					if (!B_SK(co, SK_SURROUND))
						do_sayx(cn, "Hi, %s. Bring me a pair of Rusted Spikes from the Haunted Castle and I'll teach you SURROUND HIT and CURSE.", ch[co].name);
					else
						do_sayx(cn, "Hi, %s.", ch[co].name);
				}
				else if (IS_ANY_TEMP(co) || IS_BRAVER(co) 
					|| (IS_SEYAN_DU(co) && B_SK(co, SK_CURSE)))
				{
					if (!B_SK(co, SK_SURROUND))
						do_sayx(cn, "Hi, %s. Bring me a pair of Rusted Spikes from the Haunted Castle and I'll teach you SURROUND HIT.", ch[co].name);
					else
						do_sayx(cn, "Hi, %s.", ch[co].name);
				}
				else
				{
					if (!B_SK(co, SK_CURSE))
						do_sayx(cn, "Hi, %s. Bring me a pair of Rusted Spikes from the Haunted Castle and I'll teach you CURSE.", ch[co].name);
					else
						do_sayx(cn, "Hi, %s.", ch[co].name);
				}
			}
			else if (strcmp(ch[cn].text[2], "#skill09")==0) //    26		Heal				( Gunther )
			{
				if (!B_SK(co, SK_HEAL))
					do_sayx(cn, "Greetings, %s. I would teach you HEAL if you bring me the Amulet of Immunity from the Dungeon of Doors.", ch[co].name);
				else
					do_sayx(cn, "Greetings, %s! How are you?", ch[co].name);
			}
			else if (strcmp(ch[cn].text[2], "#skill10")==0) //    31		Sense				( Manfred )
			{
				if (!(ch[co].flags & CF_SENSE)) 
					do_sayx(cn, "Hello, %s. If you bring me a silver ring adorned with a small ruby, I'll teach you Sense Magic. You can find these in the mines!", ch[co].name);
				else
					do_sayx(cn, "Hello, %s. Find anything in the mines lately?", ch[co].name);
			}
			else if (strcmp(ch[cn].text[2], "#skill11")==0) //    23		Resist				( Serena )
			{
				if (!B_SK(co, SK_RESIST))
					do_sayx(cn, "Give me the sword from that stone, and I'll teach you RESISTANCE, %s.", ch[co].name);
				else
					do_sayx(cn, "%s, welcome. Make yourself at home.", ch[co].name);
			}
			else if (strcmp(ch[cn].text[2], "#skill12")==0) //    21   		Bless				( Cirrus )
			{
				if (ch[co].flags & CF_LOCKPICK)
				{
					if (!B_SK(co, SK_BLESS))
						do_sayx(cn, "Greetings, %s. Bring me the Agate Amulet from the Thieves House and I'll teach you BLESS.", ch[co].name);
					else
						do_sayx(cn, "Leave me be, %s. I've no more to teach you.", ch[co].name);
				}
				else
					do_sayx(cn, "Please leave me be, %s. I've need of someone who knows how to pick locks, and you quite clearly don't.", ch[co].name);
			}
			else if (strcmp(ch[cn].text[2], "#skill13")==0) //    29		Rest				( Gordon )
			{
				if (!B_SK(co, SK_REST))
					do_sayx(cn, "Hello, %s. I am very ill. If you would bring me a Potion of Life, I would teach you REST.", ch[co].name);
				else
					do_sayx(cn, "Hello, %s. I am feeling better today.", ch[co].name);
			}
			else if (strcmp(ch[cn].text[2], "#skill14")==0) //    16/ 5	* Shield or Tactics	( Edna )
			{
				if (IS_LYCANTH(co))
				{
					if (!B_SK(co, SK_TACTICS))
						do_sayx(cn, "Hello, %s. Bring me the Oak Buckler from the hedge maze next door, and I shall teach you TACTICS and BLIND!", ch[co].name);
					else
						do_sayx(cn, "Hello, %s. How goes your training?", ch[co].name);
				}
				else if (IS_ANY_HARA(co) || (IS_SEYAN_DU(co) && B_SK(co, SK_SHIELD)))
				{
					if (!B_SK(co, SK_TACTICS))
						do_sayx(cn, "Hello, %s. Bring me the Oak Buckler from the hedge maze next door, and I shall teach you TACTICS!", ch[co].name);
					else
						do_sayx(cn, "Hello, %s. How goes your training?", ch[co].name);
				}
				else
				{
					if (!B_SK(co, SK_SHIELD))
						do_sayx(cn, "Hello, %s. Bring me the Oak Buckler from the hedge maze next door, and I shall teach you how to use a SHIELD!", ch[co].name);
					else
						do_sayx(cn, "Hello, %s. How goes your training?", ch[co].name);
				}
			}
			else if (strcmp(ch[cn].text[2], "#skill15")==0) //    22		Identify			( Nasir )
			{
				if (!(ch[co].kindred & KIN_IDENTIFY))
					do_sayx(cn, "Hello, %s. In the Magic Maze, across from here, Jane has created a cruel weapon. Bring it to me and I shall teach you IDENTIFY.", ch[co].name);
				else
					do_sayx(cn, "Hello, %s. Have you identified anything interesting lately?", ch[co].name);
			}
			//
			else if (strcmp(ch[cn].text[2], "#quest055")==0) // 	55 - Iggy / Mine 3 / Devil's Doorway
			{
				if (getrank(co)<13) // Lt Col
					do_sayx(cn, "Greetings and salutations, %s! Come back later, would you? I'm very busy!", ch[co].name);
				else if (!(ch[co].flags & CF_KNOWSPELL))
					do_sayx(cn, "Greetings and salutations, %s! I'm in need of a very odd book which I believe you may find in the deepest part of the Mines. Bring it to me, and I would teach you how to better understand the effects of spells!", ch[co].name);
				else
					do_sayx(cn, "Greetings and salutations, %s! No, my hair isn't on fire this time, thanks for asking!", ch[co].name);
			}
			else if (strcmp(ch[cn].text[2], "#quest100")==0) // 	0 - Mansion / Shield / Appraisal
			{
				if (!(ch[co].flags & CF_APPRAISE))
					do_sayx(cn, "Greetings, %s! I have a request. If you could bring me the Ruby Kite Shield from the mansion behind this house, I would teach you how to APPRAISE items!", ch[co].name);
				else
					do_sayx(cn, "Greetings, %s! Find anything valuable in your travels?", ch[co].name);
			}
			else if (strcmp(ch[cn].text[2], "#quest101")==0) // 	1 - Park / Tulip / Tulip Potion
			{
				do_sayx(cn, "Welcome, %s. If you would bring me a Yellow Tulip from the park behind my house, I would make you a Pure Tulip Potion in return.", ch[co].name);
			}
			else if (strcmp(ch[cn].text[2], "#quest102")==0) // 	2 - Barry / Butcher / Glow Ring
			{
				do_sayx(cn, "Oh, hi %s. There are rumors of a ghastly butcher living in the bell house behind me. If you could bring me this butcher's weapon, I would reward you with a Glow Ring.", ch[co].name);
			}
			else if (strcmp(ch[cn].text[2], "#quest103")==0) // 	3 - Bush / Fang / Amulet
			{
				if (getrank(co)<6) // Master Sergeant
					do_sayx(cn, "Hello... %s. Please come back when you're stronger... I have a request...", ch[co].name);
				else
					do_sayx(cn, "Hello... %s. Please bring me a Spider's Fang... from the Webbed Bush south of the Strange Forest... I would give you an Amulet of Resistance in exchange...", ch[co].name);
			}
			else if (strcmp(ch[cn].text[2], "#quest104")==0) // 	4 - Mine 2 / Golem Pot / Moodstone
			{
				if (getrank(co)<7) // First Sergeant
					do_sayx(cn, "Welcome, %s. When you are stronger, I have a request of you.", ch[co].name);
				else
					do_sayx(cn, "Welcome, %s. Deep in the second level of the mines, golems hide a secret spring of mud. If you would bring me a flask of this, I would reward you with a Moodstone Ring.", ch[co].name);
			}
			else if (strcmp(ch[cn].text[2], "#quest105")==0) // 	5 - Penitentiary / Headsman / Belt
			{
				if (getrank(co)<7) // First Sergeant
					do_sayx(cn, "Greetings, %s. Please be careful here, the prisoners and guards alike are a rowdy bunch.", ch[co].name);
				else
					do_sayx(cn, "Greetings, %s. The warden of this prison is up to no good and has holed himself away in his office. Please bring me evidence of his crimes, and I would reward you.", ch[co].name);
			}
			else if (strcmp(ch[cn].text[2], "#quest106")==0) // 	6 - Grolm Lab / Ruby Axe / Amulet
			{
				if (getrank(co)<9) // 2nd Leiu
					do_sayx(cn, "Hello, %s. Have you been to the Pentagram Quest?", ch[co].name);
				else
					do_sayx(cn, "Hello, %s. Deep in the Pentagram quest there is said to be a laboratory guarded by a cowardly grolm. Bring me a Ruby Axe from this lab, and I would reward you.", ch[co].name);
			}
			else if (strcmp(ch[cn].text[2], "#quest107")==0) // 	7 - Hermit / Rose / Potion
			{
				if (getrank(co)<6) // Master Sergeant
					do_sayx(cn, "Hi, %s... I'm very busy at the moment, please come back later.", ch[co].name);
				else
					do_sayx(cn, "Hi, %s. There is a mad hermit living in the Strange Forest. He possesses a rare Pink Orchid in his garden. If you could bring me this rose, I would make you a Pure Orchid Potion.", ch[co].name);
			}
			else if (strcmp(ch[cn].text[2], "#quest108")==0) // 	8 - Swamp / Marline / Rattan Bo
			{
				if (getrank(co)<11) // Captain
					do_sayx(cn, "Welcome, %s. Please make yourself at home.", ch[co].name);
				else
					do_sayx(cn, "Welcome, %s. In the Southern Swamp some lizards fashioned themselves a staff called Rattan Bo. Fetch it for me and I'll give you a special tarot card.", ch[co].name);
			}
			else if (strcmp(ch[cn].text[2], "#quest109")==0) // 	9 - Garg Nest / Rufus / Royal Targe
			{
				if (getrank(co)<11) // Captain
					do_sayx(cn, "Ahoy, %s.", ch[co].name);
				else
					do_sayx(cn, "Ahoy, %s. Ay've a request. Thar's un ol' shield holed away un de Gargoyle Nest. Bring ut to me an I'll reward ye wit a tarot card o' Strength.", ch[co].name);
			}
			else if (strcmp(ch[cn].text[2], "#quest110")==0) //   10 - Mine 3 / Gomez / Gold Huge Diamond Ring
			{
				if (getrank(co)<9) // 2nd Lieu
					do_sayx(cn, "Ah, welcome %s!", ch[co].name);
				else
					do_sayx(cn, "Welcome, %s! Would you mind sparing the time to fetch me a Golden Ring adorned with a Huge Diamond? I'd reward you with this lovely tarot card.", ch[co].name);
			}
			else if (strcmp(ch[cn].text[2], "#quest111")==0) //   11 - Mine 3 / Donna / Garg Statuette
			{
				if (getrank(co)<10) // 1st Lieu
					do_sayx(cn, "Hello, %s. Please don't touch anything.", ch[co].name);
				else
					do_sayx(cn, "Hello, %s. In the lowest floor of the mines there's said to be a striking Gargoyle Statuette. I'd reward you with this devilish tarot card.", ch[co].name);
			}
			else if (strcmp(ch[cn].text[2], "#quest112")==0) //   12 - UG2 / Rose / Green Herbs
			{
				if (getrank(co)<11) // Captain
					do_sayx(cn, "Leave me be, %s.", ch[co].name);
				else
					do_sayx(cn, "Hello, %s. I'm researching the monsters in the underground. If you could, bring me the strange green plants that grow there... I'd pay you well.", ch[co].name);
			}
			else if (strcmp(ch[cn].text[2], "#quest113")==0) //   13 - UG2 / Monica / Greenl Collector
			{
				if (getrank(co)<12) // Major
					do_sayx(cn, "Greetings, %s. Are you lost?", ch[co].name);
				else
					do_sayx(cn, "Greetings, %s. If you could bring me a completed Greenling Eye Collection, I would present you a Greenling Eye Essense in return.", ch[co].name);
			}
			else if (strcmp(ch[cn].text[2], "#quest114")==0) //   14 - Canyon / Oscar / Templar Heater
			{
				if (getrank(co)<6) // Master Serg
					do_sayx(cn, "Hello, %s.", ch[co].name);
				else
					do_sayx(cn, "Hello, %s... Do you have a moment? Could you retrieve my Templar Heater from the bandits in the forgotten canyon? I'd give you my Serpentine Amulet in return.", ch[co].name);
			}
			else if (strcmp(ch[cn].text[2], "#quest115")==0) //   15 - Archive / Castor / Traveller's Guide
			{
				if (getrank(co)<7) // First Serg
					do_sayx(cn, "Salutations, %s.", ch[co].name);
				else
					do_sayx(cn, "Salutations! %s, if you could, please find me an old book called The Traveller's Guide. If you would, I'd give you a signed copy of my own book!", ch[co].name);
			}
			else if (strcmp(ch[cn].text[2], "#quest116")==0) //   16 - Pass / Grover / Gold/Sapph Helmet
			{
				if (getrank(co)<8) // Serg Major
					do_sayx(cn, "Please leave.", ch[co].name);
				else
					do_sayx(cn, "Ah, %s. Do come in. I've a request - could you retrive my Sapphire Golden Helmet from the bandits in the Jagged pass? I'd reward you handsomely.", ch[co].name);
			}
			else if (strcmp(ch[cn].text[2], "#quest117")==0) //   17 - Valley / Regal / Spellblade [1142]
			{
				if (IS_ANY_MERC(co) || IS_SEYAN_DU(co))
				{
					if (!B_SK(co, SK_HASTE))
						do_sayx(cn, "A visitor? Far too inexperienced... Fetch me the blade from those golems, would you? I would teach you HASTE in return.");
					else
						do_sayx(cn, "Back again? Are you ready to challenge me to a duel?");
				}
				else
				{
					do_sayx(cn, "A visitor? Are you here to challenge me to a duel? If not, fetch me the blade from those golems, would you? You can have a book on swords in return.");
				}
				
			}
			else if (strcmp(ch[cn].text[2], "#quest118")==0) //   18 - Garg Nest / Shera / Fire Egg
			{
				if (getrank(co)<12) // Major
					do_sayx(cn, "Hmm... %s, could you come back a little stronger? I may have use of you.", ch[co].name);
				else
					do_sayx(cn, "Welcome, %s! If you'd bring me the Fire Egg from the Gargoyle Nest, I'd reward you with a Cloak of Fire of my own creation.", ch[co].name);
			}
			else if (strcmp(ch[cn].text[2], "#quest119")==0) //   19 - Black Stronghold / Tactician / Shiva's Sceptre
			{
				if (getrank(co)<12) // Major
					do_sayx(cn, "Greetings, %s. We need HELP against the forces of the Black Stronghold. Will you HELP us?", ch[co].name);
				else
					do_sayx(cn, "Greetings, %s. We need HELP against the forces of the Black Stronghold. Please kill its master, Shiva, and bring me his staff. We'd all be forever in your debt.", ch[co].name);
			}
			else if (strcmp(ch[cn].text[2], "#quest120")==0) //   20 - UG2 / Danica / Sogl Collector
			{
				if (getrank(co)<16) // Major General
					do_sayx(cn, "Hi, %s... Um...", ch[co].name);
				else
					do_sayx(cn, "Hi, %s... Bring me a completed Sogling Eye Collection, and... would a Sogling Eye Essense be okay...?", ch[co].name);
			}
			else if (strcmp(ch[cn].text[2], "#quest121")==0) //   21 - Ice Nest / Blanche / Ice Egg
			{
				if (getrank(co)<17) // Lieu General
					do_sayx(cn, "Come back a bit stronger, %s. I may need you to do something for me.", ch[co].name);
				else
					do_sayx(cn, "Hello, %s! If you'd bring me the Ice Egg from the Ice Gargoyle Nest, I'd reward you with a Cloak of Ice in return.", ch[co].name);
			}
			else if (strcmp(ch[cn].text[2], "#quest122")==0) //   22 - Ice Nest / Aster / Lion's Paws
			{
				if (getrank(co)<13) // Lt Col
					do_sayx(cn, "Good day, %s. Cold out.", ch[co].name);
				else
					do_sayx(cn, "Good day, %s. Please find me the Lion's Paws in the Ice Gargoyle Nest, and I would reward you with this magical tarot card.", ch[co].name);
			}
			else if (strcmp(ch[cn].text[2], "#quest123")==0) //   23 - Ludolf - Cathedral
			{
				if (getrank(co)< 9) // 2nd Leiu
					do_sayx(cn, "I'm sorry, %s, but I don't have time for kids right now.", ch[co].name);
				else
					do_sayx(cn, "Those dang cultists ruined my farm, %s. Bring me their leader's dagger and I would reward you with a tarot card of good fortune.", ch[co].name);
			}
			else if (strcmp(ch[cn].text[2], "#quest124")==0) //   24 - Flanders - Striders
			{
				if (getrank(co)<10) // 1st Leiu
					do_sayx(cn, "Howdy do, %s.", ch[co].name);
				else
					do_sayx(cn, "Howdy do, %s. A while back I lost a family heirloom to the striders in the east. If you could return it I'd give you my lonely tarot card.", ch[co].name);
			}
			else if (strcmp(ch[cn].text[2], "#quest125")==0) //   25 - Topham - Thugs
			{
				if (getrank(co)<11) // Captain
					do_sayx(cn, "Good day and goodbye, %s.", ch[co].name);
				else
					do_sayx(cn, "Good day to you, %s. Those pesky thugs to the far east stole a very sturdy shield from my collection. Return it and I would reward you handsomely.", ch[co].name);
			}
			else if (strcmp(ch[cn].text[2], "#quest126")==0) //   26 - Navarre - Shadow Talons
			{
				do_sayx(cn, "Welcome, human. The foul lizard Venominousss hasss blinded and killed many of our children. Bring me the source of his poisonsss, and I will give you thisss blue amulet.", ch[co].name);
			}
			else if (strcmp(ch[cn].text[2], "#quest127")==0) //   27 - Tsulu - Regal/Defender
			{
				do_sayx(cn, "Isssh. Human, if you sssee a man named Regal, please defeat him. Bring me hisss sssword, and I would reward you thisss red amulet.", ch[co].name);
			}
			else if (strcmp(ch[cn].text[2], "#quest128")==0) //   28 - Shafira - Venom Compendium
			{
				do_sayx(cn, "Hello, human. There isss a ssscary beassst of a lizard in the Emerald Cave we call Ssshadefang. Bring me itsss clawsss, and you may have thisss green amulet.", ch[co].name);
			}
			else if (strcmp(ch[cn].text[2], "#quest129")==0) //   29 - Dracus - Emperor's Crown
			{
				do_sayx(cn, "Come in and ssstay quiet. The Lizard Emperor is a fool of a man. Bring me hisss helmet, and you may have thisss yellow amulet, and the honor if our kin.", ch[co].name);
			}
			else if (strcmp(ch[cn].text[2], "#quest130")==0) //   30 - Rassa - Onyx Egg
			{
				do_sayx(cn, "Human... Bring me an Onyx Egg from the Onyx Gargoyle Nessst, and I would give you a Cloak of Onyx... If you sssurvive, that isss.", ch[co].name);
			}
			else if (strcmp(ch[cn].text[2], "#quest131")==0) //   31 - Makira - Save the Queen
			{
				do_sayx(cn, "Welcome, Human. I would give you thisss belt if you could climb the Volcano to the Eassst and bring me the sssword sssleeping at the sssummit.", ch[co].name);
			}
			else if (strcmp(ch[cn].text[2], "#quest132")==0) //   32 -  Vora - Coral Axe
			{
				do_sayx(cn, "Isssh... Human, bring me... the Necronomicon from the Demilich under your human Abandoned Archivesss. I may give you thisss belt in exchange.", ch[co].name);
			}
			else if (strcmp(ch[cn].text[2], "#quest133")==0) //   33 - Oswald - Old Well
			{
				if (getrank(co)<9) // 2nd Lieu
					do_sayx(cn, "'Ello, mate.");
				else
					do_sayx(cn, "'Ello, mate. Bring me some of the water from the ol' well cistern and I'd reward you with this here leather necklace.");
			}
			else if (strcmp(ch[cn].text[2], "#quest134")==0) //   34 - Maude - Dwellers
			{
				if (getrank(co)<12) // Major
					do_sayx(cn, "'Ello there, %s.", ch[co].name);
				else
					do_sayx(cn, "'Ello there, %s. If you'd bring me the emerald chalice from the Buried Brush, I'd pay you mighty finely.", ch[co].name);
			}
			else if (strcmp(ch[cn].text[2], "#quest135")==0) //   35 - Brenna - Marauders
			{
				if (getrank(co)<14) // Colonel
					do_sayx(cn, "Well howdy, %s.", ch[co].name);
				else
					do_sayx(cn, "Well howdy, %s. The marauders in the Empty Outset stole a fancy-lookin' Khopesh. If you'd bring it here, I can give you this hardy tarot card for the trouble.", ch[co].name);
			}
			else if (strcmp(ch[cn].text[2], "#quest136")==0) //   36 - Wicker - Merlin
			{
				if (getrank(co)<16) // Major Gen
					do_sayx(cn, "Hmm... Welcome, I suppose.");
				else
					do_sayx(cn, "Hmm... %s, I've been tortured by awful noises coming from Merlin's shop, but I need proof. Find me some, and I would reward you with a Scroll of Mana.", ch[co].name);
			}
			else if (strcmp(ch[cn].text[2], "#quest137")==0) //   37 - Jasper - Smugglers
			{
				if (getrank(co)<16) // Major Gen
					do_sayx(cn, "Howdy, %s. What brings you here?", ch[co].name);
				else
					do_sayx(cn, "Howdy, %s. Some smugglers stole mah antique amulet. Bring 'er back and you cun 'ave this here howlin' ol' tarot card.", ch[co].name);
			}
			else if (strcmp(ch[cn].text[2], "#quest138")==0) //   38 - Soyala - Cold Cavern
			{
				if (getrank(co)<18) // General
					do_sayx(cn, "Welcome, %s...?", ch[co].name);
				else
					do_sayx(cn, "Welcome, %s... The howling cold of the cavern to the north-east hides a shining shield. Bring it and you may have this fiery tarot card.", ch[co].name);
			}
			else if (strcmp(ch[cn].text[2], "#quest139")==0) //   39 - Runa - Ninjas
			{
				if (getrank(co)<18) // General
					do_sayx(cn, "Oh, hello %s.", ch[co].name);
				else
					do_sayx(cn, "Oh, hello %s. Rumor has it there are ninjas to the north-east. Bring me one of their books and you may have my flaunty tarot card.", ch[co].name);
			}
			else if (strcmp(ch[cn].text[2], "#quest140")==0) //   40 - Zephan - Ninjas
			{
				if (getrank(co)<18) // General
					do_sayx(cn, "Look what the cat dragged in...");
				else
					do_sayx(cn, "Well lookie here. %s, bring me the ninja lord's fancy lil' dagger and I'd reward you with this here devilish tarot card!", ch[co].name);
			}
			else if (strcmp(ch[cn].text[2], "#quest141")==0) //   41 - Rikus - Ziggurat
			{
				if (getrank(co)<14) // Colonel
					do_sayx(cn, "Greetings, %s.", ch[co].name);
				else
					do_sayx(cn, "Greetings, %s. A foul necromancer is raising an army in the Ziggurat in the Basalt Desert. Bring me their dagger, and I would reward you with this dubious tarot card.", ch[co].name);
			}
			else if (strcmp(ch[cn].text[2], "#quest142")==0) //   42 - Charlotte - Widower
			{
				if (getrank(co)<18) // General
					do_sayx(cn, "Greetings...");
				else
					do_sayx(cn, "Greetings, %s... There is a witch hiding to the south whose foul magics have twisted adventurers. Bring me her memoire, and I will give you this venomous tarot card.", ch[co].name);
			}
			else if (strcmp(ch[cn].text[2], "#quest143")==0) //   43 - Marco - Bluebeard
			{
				if (getrank(co)<17) // Lt Gen
					do_sayx(cn, "Ahoy, %s.", ch[co].name);
				else
					do_sayx(cn, "Ahoy there, %s. There be a pirate named ol' Blue Beard to the cavern south o' here. Bring me his sword, and I would give you this angry ol' tarot card in return.", ch[co].name);
			}
			// #quest144 - done in CGI 						 //   44 - Ratling Outcast
			else if (strcmp(ch[cn].text[2], "#quest145")==0) //   45 - Western Watch - Rescue Malte
			{
				if (getrank(co)<10) // 1st Lieu
					do_sayx(cn, "Hello, %s. Enter the room to my right to take a break.", ch[co].name);
				else
					do_sayx(cn, "Hello, %s. Malte, one of our spies deployed to the Black Stronghold, has been missing for some time. If you could bring proof of his rescue, I would reward you with this tarot card.", ch[co].name);
			}
			else if (strcmp(ch[cn].text[2], "#quest146")==0) //   46 - Eastern Watch - Sadem Ridge
			{
				if (getrank(co)<11) // Captain
					do_sayx(cn, "Greetings, %s. Enter the room to my left to take a break.", ch[co].name);
				else
					do_sayx(cn, "Greetings, %s. There have been sightings of a living gargoyle made of snow to the east, carrying a Sapphire Chalice. Bring this chalice back to me, and I will reward you well.", ch[co].name);
			}
			else if (strcmp(ch[cn].text[2], "#quest147")==0) //   47 - Superintendant - Thugs 2
			{
				if (getrank(co)<11) // Captain
					do_sayx(cn, "Welcome, %s. Please don't try anything funny.", ch[co].name);
				else
					do_sayx(cn, "Welcome, %s. The thugs to the far east stole an important Ruby Chalice, and have hidden it away deep in their camp. Return it, and I would reward you well.", ch[co].name);
			}
			else if (strcmp(ch[cn].text[2], "#quest148")==0) //   48 - Diplomat - Lizard Temple
			{
				if (getrank(co)<13) // Lt Col
					do_sayx(cn, "Salutations, %s. Take care travelling north-east.", ch[co].name);
				else
					do_sayx(cn, "Salutations, %s. Lizards to the north east have stolen a valuable ornament, The Sign of Skua, and stashed it away in their temple. Bring it back to me, and I will reward you with a special belt.", ch[co].name);
			}
			else if (strcmp(ch[cn].text[2], "#quest149")==0) //   49 - Vincent - Dimling's Den
			{
				if (getrank(co)<13) // Lt Col
					do_sayx(cn, "Velcome, %s. Make yourzelv at home.", ch[co].name);
				else
					do_sayx(cn, "Velcome, %s. Zhere are evil creaturez in ze park north of my houze. I plunged a zilver dagger into the heart of their queen, but... Vell, return it to me, and I vould revard you with zis zimple tarot card.", ch[co].name);
			}
			else if (strcmp(ch[cn].text[2], "#quest150")==0) //   50 - Jabilo - Thaumaturge's Hut
			{
				if (getrank(co)<15) // Brig Gen
					do_sayx(cn, "Ello, %s. My home is your home.", ch[co].name);
				else
					do_sayx(cn, "Ello, %s. There is a nasty thaumaturge living in the Violet Bog in his own hut. Bring me his staff, and I would reward you with this blessed tarot card.", ch[co].name);
			}
			else if (strcmp(ch[cn].text[2], "#quest151")==0) //   51 - Caine - Violet Thicket
			{
				if (getrank(co)<16) // Major Gen
					do_sayx(cn, "Hello there, %s. Stay a while and chat.", ch[co].name);
				else
					do_sayx(cn, "Hello there, %s. Deep in the Violet Bog lies the Violet Thicket, teaming with vilelings. Kill their queen, and bring me its scythe, and I would reward you with a Scroll of Hitpoints.", ch[co].name);
			}
			//
			//
			//
			else if (strcmp(ch[cn].text[2], "#quest156")==0) //   56 - Brye - Temple in the Sky
			{
				if (npc_quest_cleared(co, 156))
					do_sayx(cn, "Welcome, %s. Thanks again!", ch[co].name);
				else if (getrank(co)<19) // FDM
					do_sayx(cn, "Welcome, %s.", ch[co].name);
				else
					do_sayx(cn, "Welcome, %s. Legends tell of a temple floating on the clouds, and a legendary shield housed deep inside. If you found it for me, I would give you a Scroll of Strength for your efforts. Alas...", ch[co].name);
			}
			else if (strcmp(ch[cn].text[2], "#quest157")==0) //   57 - Hamako - Coral Axe
			{
				if (npc_quest_cleared(co, 157))
					do_sayx(cn, "Greetings, %s. Do you need anything of me?", ch[co].name);
				else if (getrank(co)<19) // FDM
					do_sayx(cn, "Greetings, %s. Please don't touch anything.", ch[co].name);
				else
					do_sayx(cn, "Greetings, %s. Long ago I fought a nasty Seagrel King deep in the Pentagram Quest. If you could defeat it, and bring me its axe, I would give you a Scroll of Braveness in return.", ch[co].name);
			}
			// 
			else if (strcmp(ch[cn].text[2], "#blackherbs")==0) //   xx - Zorani - Black Plants
			{
				do_sayx(cn, "Greetingsss, human! Pleassse bring me rare, black herbsss from treacherousss placesss. I would pay you very well.", ch[co].name);
			}
			else if (strcmp(ch[cn].text[2], "#scorpions")==0) //   xx - Faiza - Scorpions
			{
				if (getrank(co)<8) // SGM
					do_sayx(cn, "Salutations, %s.", ch[co].name);
				else
					do_sayx(cn, "Salutations, %s. There are many scorpions in the canyon and desert south of here. I pay good prices for their heads!", ch[co].name);
			}
			else if (strcmp(ch[cn].text[2], "#bandits")==0) //   xx - Dwyn - Bandits
			{
				if (getrank(co)<16) // Major Gen
					do_sayx(cn, "Greetings, %s. Be wary of the north.", ch[co].name);
				else
					do_sayx(cn, "Greetings, %s. I'd like you to halt a smuggler operation north of here. I would reward you for each antique you can retrieve.", ch[co].name);
			}
			else if (strcmp(ch[cn].text[2], "#skill16")==0) //    10   	Metabolism			( Lucci )
			{
				if (getrank(co)<8) // SGM
					do_sayx(cn, "Hello, %s! When you're a little stronger, come see me and I can teach you how to swim.", ch[co].name);
				else if (!B_SK(co, SK_METABOLISM))
					do_sayx(cn, "Hello, %s! Under the lake in the southern Strange Forest, there's said to be a valuable glittering cleaver. Bring it to me and I will teach you METABOLISM!", ch[co].name);
				else
					do_sayx(cn, "Hello, %s!", ch[co].name);
			}
			//
			else if (strcmp(ch[cn].text[2], "#clara")==0)
			{
				if (getrank(co)<18) // General
					do_sayx(cn, "Get out, %s! Damor doesn't wish to see you!", ch[co].name);
			}
			else if (strcmp(ch[cn].text[2], "#damor")==0)	//  155 - Damor - Shiva II
			{
				if (getrank(co)>=18)
					do_sayx(cn, "%s. It's about time. Bring me a candle from the deepest part of the Black Stronghold, and I would reward you with this ring.", ch[co].name);
			}
			//
			else 
			{
				int knowarch=0;
				if (B_SK(co, SK_WARCRY))   knowarch++;
				if (B_SK(co, SK_LEAP))     knowarch++;
				if (B_SK(co, SK_GCMASTERY))knowarch++;
				if (B_SK(co, SK_LETHARGY)) knowarch++;
				if (B_SK(co, SK_PULSE))    knowarch++;
				if (B_SK(co, SK_ZEPHYR))   knowarch++;
				if (B_SK(co, SK_FINESSE))  knowarch++;
				if (B_SK(co, SK_RAGE))     knowarch++;
				if (strcmp(ch[cn].text[2], "#skill21")==0) // ArTm - 35 - Warcry
				{
					if (IS_SEYA_OR_ARTM(co))
					{
						if ((IS_SEYAN_DU(co) && knowarch==2) || (!IS_SEYAN_DU(co) && knowarch))
							do_sayx(cn, "Greetings, %s!", ch[co].name);
						else
							do_sayx(cn, "Greetings, %s! Bring me the Star Amulet from the Northern Mountains and I would teach you WARCRY.", ch[co].name);
					}
					else
						do_sayx(cn, "Greetings, %s! Did you know Arch-Templar can strike fear into their enemies with a single shout?", ch[co].name);
				}
				else if (strcmp(ch[cn].text[2], "#skill22")==0) // Warr - 7 - Zephyr
				{
					if (IS_SEYA_OR_WARR(co))
					{
						if ((IS_SEYAN_DU(co) && knowarch==2) || (!IS_SEYAN_DU(co) && knowarch))
							do_sayx(cn, "Greetings, %s!", ch[co].name);
						else
							do_sayx(cn, "Greetings, %s! Bring me the Star Amulet from the Northern Mountains and I would teach you ZEPHYR.", ch[co].name);
					}
					else
						do_sayx(cn, "Greetings, %s! Did you know Warriors can make the very wind strike foes alongside them?", ch[co].name);
				}
				else if (strcmp(ch[cn].text[2], "#skill23")==0) // Summ - 46 - GC Mastery
				{
					if (IS_SEYA_OR_SUMM(co))
					{
						if ((IS_SEYAN_DU(co) && knowarch==2) || (!IS_SEYAN_DU(co) && knowarch))
							do_sayx(cn, "Greetings, %s!", ch[co].name);
						else
							do_sayx(cn, "Greetings, %s! Bring me the Star Amulet from the Northern Mountains and I would teach you COMPANION MASTERY.", ch[co].name);
					}
					else
						do_sayx(cn, "Greetings, %s! Did you know Summoners can command powerful allies, stronger than their peers?", ch[co].name);
				}
				else if (strcmp(ch[cn].text[2], "#skill24")==0) // Sorc - 42 - Lethargy
				{
					if (IS_SEYA_OR_SORC(co))
					{
						if ((IS_SEYAN_DU(co) && knowarch==2) || (!IS_SEYAN_DU(co) && knowarch))
							do_sayx(cn, "Greetings, %s!", ch[co].name);
						else
							do_sayx(cn, "Greetings, %s! Bring me the Star Amulet from the Northern Mountains and I would teach you LETHARGY.", ch[co].name);
					}
					else
						do_sayx(cn, "Greetings, %s! Did you know Sorcerers can enter a trance to bypass enemy resistances?", ch[co].name);
				}
				else if (strcmp(ch[cn].text[2], "#skill25")==0) // ArHr - 43 - Pulse
				{
					if (IS_SEYA_OR_ARHR(co))
					{
						if ((IS_SEYAN_DU(co) && knowarch==2) || (!IS_SEYAN_DU(co) && knowarch))
							do_sayx(cn, "Greetings, %s!", ch[co].name);
						else
							do_sayx(cn, "Greetings, %s! Bring me the Star Amulet from the Northern Mountains and I would teach you PULSE.", ch[co].name);
					}
					else
						do_sayx(cn, "Greetings, %s! Did you know Arch-Harakim can cause damaging bursts without thinking?", ch[co].name);
				}
				else if (strcmp(ch[cn].text[2], "#skill26")==0) // Skal - 49 - Leap
				{
					if (IS_SEYA_OR_SKAL(co))
					{
						if ((IS_SEYAN_DU(co) && knowarch==2) || (!IS_SEYAN_DU(co) && knowarch))
							do_sayx(cn, "Greetings, %s!", ch[co].name);
						else
							do_sayx(cn, "Greetings, %s! Bring me the Star Amulet from the Northern Mountains and I would teach you LEAP.", ch[co].name);
					}
					else
						do_sayx(cn, "Greetings, %s! Did you know Skalds can leap great distances to strike distant foes?", ch[co].name);
				}
				else if (strcmp(ch[cn].text[2], "#skill27")==0) // Brav - 14 - Finesse
				{
					if (IS_SEYA_OR_BRAV(co))
					{
						if ((IS_SEYAN_DU(co) && knowarch==2) || (!IS_SEYAN_DU(co) && knowarch))
							do_sayx(cn, "Greetings, %s!", ch[co].name);
						else
							do_sayx(cn, "Greetings, %s! Bring me the Star Amulet from the Northern Mountains and I would teach you FINESSE.", ch[co].name);
					}
					else
						do_sayx(cn, "Greetings, %s! Did you know Bravers can capitalize on having full health?", ch[co].name);
				}
				else if (strcmp(ch[cn].text[2], "#skill28")==0) // Lyca - 22 - Rage
				{
					if (IS_SEYA_OR_LYCA(co))
					{
						if ((IS_SEYAN_DU(co) && knowarch==2) || (!IS_SEYAN_DU(co) && knowarch))
							do_sayx(cn, "Greetings, %s!", ch[co].name);
						else
							do_sayx(cn, "Greetings, %s! Bring me the Star Amulet from the Northern Mountains and I would teach you RAGE and CALM.", ch[co].name);
					}
					else
						do_sayx(cn, "Greetings, %s! Did you know Lycanthropes can force themselves into a furious rage, or hypnotizing calm?", ch[co].name);
				}
				else if (strcmp(ch[cn].text[2], "#gatekeeper13")==0)
				{
					if (knowarch && IS_SEYAN_DU(co))
					{
						if (knowarch==2)
							do_sayx(cn, "Hello, %s. Are you happy with the skills you've learned? If not, I can remove them for an experience cost of 250,000. Say UNLEARN and I can make it so.", ch[co].name);
						else
							do_sayx(cn, "Hello, %s. Are you happy with the skill you've learned? If not, I can remove it for an experience cost of 250,000. Say UNLEARN and I can make it so.", ch[co].name);
					}
					else if (IS_ANY_ARCH(co))
						do_sayx(cn, "Congratulations, %s! Now that you have arched, you can donate a secondary tarot card to the shrine and it will apply it too! I can also REMOVE it for you.", ch[co].name);
					else
						do_sayx(cn, "Welcome, %s! I'm sure you have many questions. Read the notes in front of me, and you may ask me 'about X' for anything capitalized. I also act as a bank!", ch[co].name);
				}
				else 
				{
					if (ch[cn].temp == CT_PRIEST)
					{
						for (m=0;m<MAXITEMS;m++) if (IS_SINBINDER(ch[cn].item[n])) break;
					}
					// here is message filter, if talking NPC is priest and player char is PURPLE we greet him
					if ((ch[cn].temp == CT_PRIEST) && (IS_PURPLE(co) || m<MAXITEMS))
					{
						if (m==MAXITEMS)
							do_sayx(cn, "Greetings, %s!", ch[co].name);
						else
							do_sayx(cn, "Hello there, %s. If you happen to have a working Sinbinder Ring, I could... 'assist' you in putting it on.", ch[co].name);
					}
					else if ((ch[cn].temp == CT_KWAIVICAR) && !IS_CLANKWAI(co))
					{
						do_sayx(cn, "Welcome, %s. Would you like to join clan Kwai? If so, use #kwai to join. But beware! Members of the Gorn clan may attack you!", ch[co].name);
					}
					else if ((ch[cn].temp == CT_GORNPASTOR) && !IS_CLANGORN(co))
					{
						do_sayx(cn, "Welcome, %s. Would you like to join clan Gorn? If so, use #gorn to join. But beware! Members of the Kwai clan may attack you!", ch[co].name);
					}
					else	// NPC will talk as usual
					{
						npc_saytext_n(cn, 2, ch[co].name);
					}
				}
			}

			ch[cn].data[40] = ch[cn].data[39];
			ch[cn].data[39] = ch[cn].data[38];
			ch[cn].data[38] = ch[cn].data[37];
			ch[cn].data[37] = co;
			ch[cn].data[56] = globs->ticker + TICKS * 30;

			if (ch[cn].data[26]==5)   // special proc for unique warning
			{
				cnt = count_uniques(co);

				if (cnt==1)
				{
					do_sayx(cn, "I see you have a weapon dedicated to the gods. Make good use of it, %s.\n", ch[co].name);
				}
				else if (cnt==2)
				{
					do_sayx(cn, "I see you have a few weapons dedicated to the gods. Make good use of them, %s.\n", ch[co].name);
				}
				else if (cnt>2)
				{
					do_sayx(cn, "I see you have several weapons dedicated to the gods. They will get angry if you keep more than two, %s.\n", ch[co].name);
				}
			}
		}
	}
	return 0;
}

int npc_died(int cn, int co)
{
	if (ch[co].flags & CF_SILENCE)
	{
		return 0;
	}
	if (ch[cn].data[48] && co)
	{
		if (RANDOM(100)<ch[cn].data[48])
		{
			npc_saytext_n(cn, 3, ch[co].name);
		}
		return 1;
	}
	return 0;
}

int npc_shout(int cn, int co, int code, int x, int y)
{
	if (ch[cn].data[53] && ch[cn].data[53]==code)   // someone called help. If it's our code, we come to the rescue
	{
		ch[cn].data[92] = TICKS * 60;
		ch[cn].data[54] = x + y * MAPX;
		ch[cn].data[55] = globs->ticker;
		npc_saytext_n(cn, 5, ch[co].name);

		// cancel current long-term actions:
		ch[cn].goto_x = 0;
		ch[cn].misc_action = 0;

		return 1;
	}

	return 0;
}

int npc_hitme(int cn, int co)
{
	if (ch[cn].data[26]==4 || (ch[cn].temp==CT_BSMAGE1 || ch[cn].temp==CT_BSMAGE2 || ch[cn].temp==CT_BSMAGE3))   // generic or candle-keeper
	{
		if (!do_char_can_see(cn, co, 0))
		{
			return 1;
		}
		if (npc_add_enemy(cn, co, 1))
		{
			if (!(ch[co].flags & CF_SILENCE) && !(ch[cn].flags & CF_SILENCE))
			{
				npc_saytext_n(cn, 1, ch[co].name);
			}
			chlog(cn, "Added %s to kill list for stepping on trap", ch[co].name);
			return 1;
		}
	}
	else if (ch[cn].data[26]==8) // investigate without adding to kill list
	{
		for (int try = 0; try < 3; try++)
			if(npc_moveto(cn, ch[co].frx, ch[co].fry)) break;
	}
	return 0;
}

int npc_msg(int cn, int type, int dat1, int dat2, int dat3, int dat4)
{
	if (ch[cn].data[25])
	{
		switch(ch[cn].data[25])
		{
		case  1:
			return( npc_stunrun_msg(cn, type, dat1, dat2, dat3, dat4) );
		case  2:
			return( npc_cityattack_msg(cn, type, dat1, dat2, dat3, dat4) );
		case  3:
			return( npc_malte_msg(cn, type, dat1, dat2, dat3, dat4) );
		case  4:
			return( npc_shiva_msg(cn, type, dat1, dat2, dat3, dat4) );
		default:
			chlog(cn, "unknown special driver %d", ch[cn].data[25]);
			break;
		}
		return 0;
	}
	switch(type)
	{
	case    NT_GOTHIT:
		return( npc_gothit(cn, dat1, dat2));
	case    NT_GOTMISS:
		return( npc_gotmiss(cn, dat1));
	case    NT_DIDHIT:
		return( npc_didhit(cn, dat1, dat2));
	case    NT_DIDMISS:
		return( npc_didmiss(cn, dat1));
	case    NT_DIDKILL:
		return( npc_didkill(cn, dat1));
	case    NT_GOTEXP:
		return( npc_gotexp(cn, dat1));
	case    NT_SEEKILL:
		return( npc_seekill(cn, dat1, dat2));
	case    NT_SEEHIT:
		return( npc_seehit(cn, dat1, dat2));
	case    NT_SEEMISS:
		return( npc_seemiss(cn, dat1, dat2));
	case    NT_GIVE:
		return( npc_give(cn, dat1, dat2, dat3));
	case    NT_SEE:
		return( npc_see(cn, dat1));
	case    NT_DIED:
		return( npc_died(cn, dat1));
	case    NT_SHOUT:
		return( npc_shout(cn, dat1, dat2, dat3, dat4));
	case    NT_HITME:
		return( npc_hitme(cn, dat1));
	default:
		xlog("Unknown NPC message for %d (%s): %d", cn, ch[cn].name, type);
		return 0;
	}
}

// ********************************
// *       High Prio Driver       *
// ********************************

int get_spellcost(int cn, int spell)
{
	switch(spell)
	{
		case SK_BLIND:		return SP_COST_BLIND;
		case SK_CLEAVE:		return 20;
		case SK_SHIELD:		return 20;
		case SK_LEAP:		return 20;
		case SK_RAGE:		return SP_COST_RAGE;
		case SK_TAUNT:		return SP_COST_TAUNT;
		case SK_WEAKEN:		return SP_COST_WEAKEN;
		case SK_WARCRY:		return SP_COST_WARCRY;
		
		case SK_BLAST:		return 20;
		case SK_BLESS:		return SP_COST_BLESS;
		case SK_CURSE:		return SP_COST_CURSE;
		case SK_DISPEL:		return SP_COST_DISPEL;
		case SK_ENHANCE:	return SP_COST_ENHANCE;
		case SK_GHOST:		return SP_COST_GHOST;
		case SK_HASTE:		return SP_COST_HASTE;
		case SK_HEAL:		return SP_COST_HEAL;
		case SK_IDENT:		return SP_COST_IDENT;
		case SK_LETHARGY:	return SP_COST_LETHARGY;
		case SK_LIGHT:		return SP_COST_LIGHT;
		case SK_MSHIELD:	return SP_COST_MSHIELD;
		case SK_POISON:		return SP_COST_POISON;
		case SK_PROTECT:	return SP_COST_PROTECT;
		case SK_PULSE:		return SP_COST_PULSE;
		case SK_RECALL:		return SP_COST_RECALL;
		case SK_SHADOW:		return SP_COST_SHADOW;
		case SK_SLOW:		return SP_COST_SLOW;
		
		default: return 9999;
	}
}

int spellflag(int spell)
{
	switch(spell)
	{
		case SK_BLIND:		return SP_BLIND;
		case SK_RAGE:		return SP_RAGE;
		case SK_TAUNT:		return SP_TAUNT;
		case SK_WARCRY:		return SP_WARCRY;
		case SK_WEAKEN:		return SP_REND;
		
		case SK_BLESS:		return SP_BLESS;
		case SK_CURSE:		return SP_CURSE;
		case SK_DISPEL:		return SP_DISPEL;
		case SK_ENHANCE:	return SP_ENHANCE;
		case SK_HASTE:		return SP_HASTE;
		case SK_HEAL:		return SP_HEAL;
		case SK_LETHARGY:	return SP_LETHARGY;
		case SK_LIGHT:		return SP_LIGHT;
		case SK_MSHIELD:	return SP_MSHIELD;
		case SK_POISON:		return SP_POISON;
		case SK_PROTECT:	return SP_PROTECT;
		case SK_PULSE:		return SP_PULSE;
		case SK_SLOW:		return SP_SLOW;
		
		default:				return 0;
	}
}

int npc_try_spell(int cn, int co, int spell)
{
	int mana, end, n, in, tmp, cc, truespell, usemana = 0;
	int offn, defn, tpow, tdef, timm;

	if (spell!=SK_CLEAVE && spell!=SK_SHIELD && spell!=SK_LEAP && spell!=SK_WEAKEN && spell!=SK_TAUNT && 
		spell!=SK_WARCRY && spell!=SK_BLIND && spell!=SK_RAGE && spell!=SK_CALM)
	{
		usemana = 1;
		if ((ch[cn].flags & CF_NOMAGIC) || (ch[co].flags & CF_NOMAGIC))
		{
			return 0;
		}
	}
	
	if (ch[co].used!=USE_ACTIVE)
	{
		return 0;
	}
	if (ch[co].flags & CF_BODY)
	{
		return 0;
	}

	if (!B_SK(cn, spell) && spell!=SK_LIGHT && spell!=SK_RECALL) // we don't know this spell
	{
		return 0;
	}
	
	if (ch[co].flags & CF_STONED) // he's stoned, dont spell him
	{
		return 0;                     
	}
	
	if (ch[co].flags & CF_IMMORTAL) // Ignore immortals, or we'd be flailing like morons.
	{
		return 0;
	}
	
	offn = M_SK(cn, spell);
	
	if (usemana)	offn = spell_multiplier(offn, cn);
	else			offn = skill_multiplier(offn, cn);
	
	defn = get_target_resistance(cn, co)*10;
	
	truespell = spell;
	
	// ** Translate spell to truespell based on caster tarot card     
	
	if (spell==SK_CURSE  && get_tarot(cn, IT_CH_TOWER))   	truespell = SK_CURSE2;
	if (spell==SK_BLIND  && get_tarot(cn, IT_CH_CHARIOT)) 	truespell = SK_DOUSE;
	if (spell==SK_POISON && get_tarot(cn, IT_CH_TOWER_R)) 	truespell = SK_VENOM;
	if (spell==SK_PULSE  && get_tarot(cn, IT_CH_JUDGE_R)) 	truespell = SK_IMMOLATE;
	if (spell==SK_WEAKEN && get_tarot(cn, IT_CH_DEATH)) 	truespell = SK_WEAKEN2;
	if (spell==SK_SLOW   && get_tarot(cn, IT_CH_EMPEROR)) 	truespell = SK_SLOW2;
	if (spell==SK_WARCRY && get_tarot(cn, IT_CH_EMPERO_R)) 	truespell = SK_WARCRY3;
	if (spell==SK_HEAL   && get_tarot(cn, IT_CH_STAR)) 		truespell = SK_REGEN;
	
	/*
	if (spell==SK_WEAKEN && get_enchantment(co,  8)) return 0;
	if (spell==SK_SLOW   && get_enchantment(co, 14)) return 0;
	if (spell==SK_CURSE  && get_enchantment(co, 21)) return 0;
	if (spell==SK_BLIND  && get_enchantment(co, 33)) return 0;
	*/
	
	// dont blast if enemy armor is too strong
	// Updated 02/11/2020 - changed the formula to include immunity and allow blasts that would do at least 5 damage.
	if (truespell==SK_BLAST && ( (offn - max(0, get_target_immunity(cn, co)/2)) * 2 ) - ch[co].armor < 20/3)
	{
		return 0;
	}
	
	// dont cleave if enemy armor is too strong
	if (truespell==SK_CLEAVE && ( ((offn + ch[cn].weapon/4 + ch[cn].top_damage/4 - max(0, (ch[co].to_parry/2))) * 2 ) - ch[co].armor < 20/3) )
	{
		return 0;
	}
	
	// dont bash if enemy armor is too strong
	if (truespell==SK_SHIELD && ( ((offn + ch[cn].armor/2 - max(0, (ch[co].to_parry/2))) * 2 ) - ch[co].armor < 20/3) )
	{
		return 0;
	}
	
	// dont leap if enemy armor is too strong
	if (truespell==SK_LEAP && ( ((offn + ch[cn].weapon/4 + ch[cn].top_damage/4 - max(0, (ch[co].to_parry/2))) * 2 ) - ch[co].armor < 20/3) )
	{
		return 0;
	}
	
	// dont debuff if chances of success are bad
	if (truespell==SK_CURSE   && SP_MULT_CURSE   * offn / max(1, defn)< 7) return 0;
	if (truespell==SK_CURSE2  && SP_MULT_CURSE2  * offn / max(1, defn)< 7) return 0;
	if (truespell==SK_BLIND   && SP_MULT_BLIND   * offn / max(1, defn)< 7) return 0;
	if (truespell==SK_DOUSE   && SP_MULT_BLIND   * offn / max(1, defn)< 7) return 0;
	if (truespell==SK_POISON  && SP_MULT_POISON  * offn / max(1, defn)< 8) return 0;
	if (truespell==SK_VENOM   && SP_MULT_POISON2 * offn / max(1, defn)< 8) return 0;
	if (truespell==SK_WEAKEN  && SP_MULT_WEAKEN  * offn / max(1, defn)< 8) return 0;
	if (truespell==SK_WEAKEN2 && SP_MULT_WEAKEN2 * offn / max(1, defn)< 8) return 0;
	if (truespell==SK_SLOW    && SP_MULT_SLOW    * offn / max(1, defn)< 9) return 0;
	if (truespell==SK_SLOW2   && SP_MULT_SLOW2   * offn / max(1, defn)< 9) return 0;
	if (truespell==SK_DISPEL  && SP_MULT_DISPEL2 * offn / max(1, defn)< 9) return 0;
	if (truespell==SK_TAUNT   && SP_MULT_TAUNT   * offn / max(1, defn)<10) return 0;
	if (truespell==SK_WARCRY  && SP_MULT_WARCRY  * offn / max(1, defn)<10) return 0;
	
	// prevent thralling multiple ghosts
	if (spell==SK_GHOST)
	{
		for (n = 1; n<MAXCHARS; n++)
		{
			if (ch[n].used==USE_EMPTY || (ch[n].flags & (CF_PLAYER | CF_USURP))) 
				continue;
			if (ch[n].data[CHD_MASTER]==cn)
				return 0;
		}
	}
	
	for (n = 0; n<MAXBUFFS; n++)
	{
		in = ch[cn].spell[n];
		// Cancel if exhausted                // prevent thralling multiple shadows
		if (in && (bu[in].temp==SK_EXHAUST))
		{
			return 0;
		}
	}

	mana = (ch[cn].a_mana-500) / 1000;
	end = (ch[cn].a_end-500) / 1000;
	tdef = (usemana && get_target_resistance(cn, co) >= offn) ? 1 : 0;
	
	if (!IS_PLAYER_COMP(cn) && truespell==SK_BLAST && mana<((offn * 2) / 8 + 5)) return 0; // can't afford Blast
	
	for (n = 0; n<MAXBUFFS; n++)
	{
		if ((in = ch[co].spell[n])!=0)
		{
			tpow = bu[in].power+15;
			timm = spell_immunity(offn, get_target_immunity(cn, co));
			timm = tdef ? (timm/2) : timm;
			// Cancel if target is already buffed or debuffed (except for heal)
			if (bu[in].temp==spell && spell!=SK_HEAL /*&& tpow>=timm*/ && bu[in].active>bu[in].duration/4)
			{
				break;
			}
			// Immunize/Inoculate prevents up to three ailments
			if ((bu[in].temp==SK_DISPEL || bu[in].temp==SK_DISPEL2) &&
				(truespell==bu[in].data[1] || truespell==bu[in].data[2] || truespell==bu[in].data[3]))
			{
				break;
			}
			// Poison layer adjustments
			if (bu[in].temp==truespell && truespell==SK_VENOM && bu[in].active>bu[in].duration/4)
			{
				break;
			}
			// Don't cast heal if target can't be healed.
			if (bu[in].temp==spell && spell==SK_HEAL && bu[in].data[1] >= 4)
			{
				break;
			}
		}
	}
	
	if (get_tarot(cn, IT_CH_WORLD_R)) usemana = 1;

	if (n==MAXBUFFS)
	{
		tmp = spellflag(spell);
		if (!usemana)
		{
			if (end>=get_spellcost(cn, spell) && !(ch[co].data[96] & tmp))
			{
				//chlog(cn,"Trying endu skill %d on %s (%d)",spell,ch[co].name,co);
				ch[cn].skill_nr = spell;
				ch[cn].skill_target1 = co;
				ch[co].data[96] |= tmp;
				fx_add_effect(11, 8, co, tmp, 0);
				return 1;
			}
		}
		else
		{
			if (mana>=get_spellcost(cn, spell) && !(ch[co].data[96] & tmp))
			{
				//chlog(cn,"Trying mana spell %d on %s (%d)",spell,ch[co].name,co);
				ch[cn].skill_nr = spell;
				ch[cn].skill_target1 = co;
				ch[co].data[96] |= tmp;
				fx_add_effect(11, 8, co, tmp, 0);
				return 1;
			}
		}
	}
	return 0;
}

int npc_can_spell(int cn, int co, int spell)
{
	if (!get_tarot(cn, IT_CH_WORLD_R) && 
       (spell==SK_CLEAVE || spell==SK_SHIELD || spell==SK_WEAKEN || spell==SK_WARCRY || spell==SK_BLIND || 
		spell==SK_TAUNT || spell==SK_LEAP || spell==SK_RAGE || spell==SK_CALM))
	{
		if ((ch[cn].a_end-500) / 1000 < get_spellcost(cn, spell)) return 0;
	}
	else
	{
		if ((ch[cn].a_mana-500) / 1000 < get_spellcost(cn, spell) * (get_tarot(cn, IT_CH_MAGI_R)?3:2)/2) return 0;
	}
	if (!B_SK(cn, spell))
	{
		return 0;
	}
	if (M_SK(co, spell)>M_SK(cn, spell))
	{
		return 0;
	}

	return 1;
}

int npc_quaff_potion(int cn, int itemp, int stemp)
{
	int n, in;

	for (n = 0; n<MAXBUFFS; n++)
	{
		if ((in = ch[cn].spell[n]) && bu[in].temp==stemp)
		{
			return 0;                                         // potion already active
		}
	}
	for (n = 0; n<40; n++)
	{
		if ((in = ch[cn].item[n]) && it[in].temp==itemp)
		{
			break;                                                           // find potion
		}
	}
	if (n==40)
	{
		return 0;                                                             // no potion :(
	}
	do_area_log(cn, 0, ch[cn].x, ch[cn].y, 1, "%s drinks deep from a magical brew.\n", ch[cn].reference);

	use_driver(cn, in, 1);

	return 1;
}

void npc_wedge_doors(int n, int flag) // flag = wedge the door!
{
	int cn, in, x, y;
	
	switch (n)
	{
		case 13: // Liz Emperor
			if (flag)
			{
				x = 928; y = 423; cn = map[XY2M(x, y)].ch; if (cn) god_transfer_char(cn, x, y-1);
				in = map[XY2M(x, y)].it; it[in].active = 0; it[in].data[5] = TICKS * 60 * 15;
				reset_go(it[in].x, it[in].y); it[in].flags |= IF_MOVEBLOCK | IF_SIGHTBLOCK; reset_go(it[in].x, it[in].y);
				x = 934; y = 421; cn = map[XY2M(x, y)].ch; if (cn) god_transfer_char(cn, x-1, y);
				in = map[XY2M(x, y)].it; it[in].active = 0; it[in].data[5] = TICKS * 60 * 15;
				reset_go(it[in].x, it[in].y); it[in].flags |= IF_MOVEBLOCK | IF_SIGHTBLOCK; reset_go(it[in].x, it[in].y);
			}
			else
			{
				in = map[XY2M(928, 423)].it; it[in].data[5] = 0;
				in = map[XY2M(934, 421)].it; it[in].data[5] = 0;
			}
			break;
		case 14: // Shiva II
			if (flag)
			{
				x = 576; y = 130; cn = map[XY2M(x, y)].ch; if (cn) god_transfer_char(cn, x, y-1);
				in = map[XY2M(x, y)].it; it[in].active = 0; it[in].data[5] = TICKS * 60 * 15;
				reset_go(it[in].x, it[in].y); it[in].flags |= IF_MOVEBLOCK | IF_SIGHTBLOCK; reset_go(it[in].x, it[in].y);
				x = 568; y = 122; cn = map[XY2M(x, y)].ch; if (cn) god_transfer_char(cn, x+1, y);
				in = map[XY2M(x, y)].it; it[in].active = 0; it[in].data[5] = TICKS * 60 * 15;
				reset_go(it[in].x, it[in].y); it[in].flags |= IF_MOVEBLOCK | IF_SIGHTBLOCK; reset_go(it[in].x, it[in].y);	
			}
			else
			{
				in = map[XY2M(576, 130)].it; it[in].data[5] = 0;
				in = map[XY2M(568, 122)].it; it[in].data[5] = 0;
			}
			break;
		case 20: // Tower XX
			if (flag)
			{
				x = 188; y = 202; cn = map[XY2M(x, y)].ch; if (cn) god_transfer_char(cn, x, y+1);
				in = map[XY2M(x, y)].it; it[in].active = 0; it[in].data[5] = TICKS * 60 * 15;
				reset_go(it[in].x, it[in].y); it[in].flags |= IF_MOVEBLOCK | IF_SIGHTBLOCK; reset_go(it[in].x, it[in].y);	
				x = 188; y = 218; cn = map[XY2M(x, y)].ch; if (cn) god_transfer_char(cn, x, y-1);
				in = map[XY2M(x, y)].it; it[in].active = 0; it[in].data[5] = TICKS * 60 * 15;
				reset_go(it[in].x, it[in].y); it[in].flags |= IF_MOVEBLOCK | IF_SIGHTBLOCK; reset_go(it[in].x, it[in].y);	
			}
			else
			{
				in = map[XY2M(188, 202)].it; it[in].data[5] = 0;
				in = map[XY2M(188, 218)].it; it[in].data[5] = 0;
			}
			break;
		case 30: // Abyss X
			if (flag)
			{
				x = 530; y = 160; cn = map[XY2M(x, y)].ch; if (cn) god_transfer_char(cn, x, y+1);
				in = map[XY2M(x, y)].it; it[in].active = 0; it[in].data[5] = TICKS * 60 * 15;
				reset_go(it[in].x, it[in].y); it[in].flags |= IF_MOVEBLOCK | IF_SIGHTBLOCK; reset_go(it[in].x, it[in].y);
				x = 530; y = 176; cn = map[XY2M(x, y)].ch; if (cn) god_transfer_char(cn, x, y-1);
				in = map[XY2M(x, y)].it; it[in].active = 0; it[in].data[5] = TICKS * 60 * 15;
				reset_go(it[in].x, it[in].y); it[in].flags |= IF_MOVEBLOCK | IF_SIGHTBLOCK; reset_go(it[in].x, it[in].y);	
			}
			else
			{
				in = map[XY2M(530, 160)].it; it[in].data[5] = 0;
				in = map[XY2M(530, 176)].it; it[in].data[5] = 0;
			}
			break;
		default:
			break;
	}
}

// spark rings - flag=1 for on, flag=0 for off
void npc_activate_rings(int cn, int flag)
{
	int in;
	
	if (in = ch[cn].worn[WN_LRING])
	{
		if (flag && !it[in].active)
			use_driver(cn, in, 1);
		else if (!flag && it[in].active)
			use_driver(cn, in, 1);
	}
	if (in = ch[cn].worn[WN_RRING])
	{
		if (flag && !it[in].active)
			use_driver(cn, in, 1);
		else if (!flag && it[in].active)
			use_driver(cn, in, 1);
	}
}

void die_companion(int cn)
{
	fx_add_effect(7, 0, ch[cn].x, ch[cn].y, 0);
	god_destroy_items(cn);
	ch[cn].gold = 0;
	do_char_killed(0, cn, 0);
}

int is_inline(int cn, int ln)
{
	int x, y;
	
	if(!IS_PLAYER(cn)) return 0;
	if(!IS_ACTIVECHAR(cn)) return 0;
	
	x = ch[cn].x;
	y = ch[cn].y;
	
	if (ln == 1 && x >= 539 && y >= 282 && x <= 562 && y <= 372) return 1; // Line 1
	if (ln == 2 && x >= 563 && y >= 282 && x <= 589 && y <= 372) return 1; // Line 2
	if (ln == 3 && x >= 590 && y >= 282 && x <= 613 && y <= 372) return 1; // Line 3
	
	return 0;
}

int is_incolosseum(int cn, int ln)
{
	int x, y;
	
	if (!IS_SANECHAR(cn)) return 0;
	if (!IS_ACTIVECHAR(cn)) return 0;
	
	x = ch[cn].x;
	y = ch[cn].y;
	
	if (ln == 0 && ((x >= 738 && y >= 946 && x <= 746 && y <= 950) ||
					(x >= 705 && y >= 921 && x <= 713 && y <= 925) ||
					(x >= 771 && y >= 921 && x <= 779 && y <= 925) ||
					(x >= 705 && y >= 971 && x <= 713 && y <= 975) ||
					(x >= 771 && y >= 971 && x <= 779 && y <= 975) )) return 0; // Reward rooms (spectate prevention)
	
	if ((ln == 0 || ln == 1) && x >= COLOS1_X1 && y >= COLOS1_Y1 && x <= COLOS1_X2 && y <= COLOS1_Y2) return 1; // Area 1
	if ((ln == 0 || ln == 2) && x >= COLOS2_X1 && y >= COLOS2_Y1 && x <= COLOS2_X2 && y <= COLOS2_Y2) return 1; // Area 2
	if ((ln == 0 || ln == 3) && x >= COLOS3_X1 && y >= COLOS3_Y1 && x <= COLOS3_X2 && y <= COLOS3_Y2) return 1; // Area 3
	if ((ln == 0 || ln == 4) && x >= COLOS4_X1 && y >= COLOS4_Y1 && x <= COLOS4_X2 && y <= COLOS4_Y2) return 1; // Area 4
	if ((ln == 0 || ln == 5) && x >= COLOS5_X1 && y >= COLOS5_Y1 && x <= COLOS5_X2 && y <= COLOS5_Y2) return 1; // Area 5
	
	return 0;
}

// The inner colosseum area for monster stuck checks
int is_reallyincolosseum(int cn, int ln)
{
	int x, y;
	
	if (!IS_SANECHAR(cn)) return 0;
	if (!IS_ACTIVECHAR(cn)) return 0;
	
	x = ch[cn].x;
	y = ch[cn].y;
	
	if (ln == 1 && x >= COLOS1_X1+8 && y >= COLOS1_Y1+8 && x <= COLOS1_X2-8 && y <= COLOS1_Y2-8) return 1; // Area 1
	if (ln == 2 && x >= COLOS2_X1+8 && y >= COLOS2_Y1+8 && x <= COLOS2_X2-8 && y <= COLOS2_Y2-8) return 1; // Area 2
	if (ln == 3 && x >= COLOS3_X1+8 && y >= COLOS3_Y1+8 && x <= COLOS3_X2-8 && y <= COLOS3_Y2-8) return 1; // Area 3
	if (ln == 4 && x >= COLOS4_X1+8 && y >= COLOS4_Y1+8 && x <= COLOS4_X2-8 && y <= COLOS4_Y2-8) return 1; // Area 4
	if (ln == 5 && x >= COLOS5_X1+8 && y >= COLOS5_Y1+8 && x <= COLOS5_X2-8 && y <= COLOS5_Y2-8) return 1; // Area 5
	
	return 0;
}

int is_atpandium(int cn)
{
	int x, y;
	
	if (!IS_SANECHAR(cn)) return 0;
	if (!IS_ACTIVECHAR(cn)) return 0;
	
	x = ch[cn].x;
	y = ch[cn].y;
	
	if (x >= PANDI_X1 && y >= PANDI_Y1 && x <= PANDI_X2 && y <= PANDI_Y2) return 1;
	
	return 0;
}

// special sub-proc for black stronghold mages
// This is where monsters are spawned and candles are considered
// Triggers if mana is above 95%, if HP is below 60% and mana is above 30%, or at the times when new candles would generate
void stronghold_mage_driver(int cn)
{
	int bscanA, bscanB, bscanC, bscanD, bscanE;
	int bs_x, bs_y, bs_group, bs_temp, spawn_temp[60], attr_bonus[6] = {0}, wv_bonus[6] = {0}, hp_bonus[6] = {0}, ws_bonus[6] = {0};
	int n, m, j, in, co, magenum=0, pts=0, modified=0;
	int pinline = 0;
	char buf[80];
	
	if (!(globs->flags & GF_STRONGHOLD)) return;
	
	if (ch[cn].temp==CT_BSMAGE1) magenum = 1;
	if (ch[cn].temp==CT_BSMAGE2) magenum = 2;
	if (ch[cn].temp==CT_BSMAGE3) magenum = 3;
	
	// Check for players in the area
	for (n = 1; n<MAXCHARS; n++) 
	{
		if (ch[n].used==USE_EMPTY) continue;
		if (!(ch[n].flags & (CF_PLAYER | CF_USURP))) continue;
		if (is_inline(n, magenum)) { pinline++; ch[cn].data[5] = n; }
	}
	
	// Set up candle mobs
	if (magenum==3)
	{
		bscanA = BS_CAN_3_1;	bscanB = BS_CAN_3_2;	bscanC = BS_CAN_3_3;	bscanD = BS_CAN_3_4;	bscanE = BS_CAN_3_5;
		bs_x = 598;				bs_y = 288;				bs_group = 603;			bs_temp = 359;
		ch[cn].data[0] = pinline;
	}
	else if (magenum==2)
	{
		bscanA = BS_CAN_2_1;	bscanB = BS_CAN_2_2;	bscanC = BS_CAN_2_3;	bscanD = BS_CAN_2_4;	bscanE = BS_CAN_2_5;
		bs_x = 576;				bs_y = 288;				bs_group = 602;			bs_temp = 353;
		ch[cn].data[0] = pinline;
	}
	else
	{
		bscanA = BS_CAN_1_1; 	bscanB = BS_CAN_1_2; 	bscanC = BS_CAN_1_3; 	bscanD = BS_CAN_1_4; 	bscanE = BS_CAN_1_5;
		bs_x = 554; 			bs_y = 288; 			bs_group = 601; 		bs_temp = 347;
		ch[cn].data[0] = pinline;
	}
	
	if (pinline<1)
	{
		// Clean up candles once every 6 hours if nobody is there
		if (globs->mdtime%(MD_HOUR*6)>1 && globs->mdtime%(MD_HOUR*6)<2160 && ch[cn].data[3]!=0 && ch[cn].data[3]!=1)
		{
			if (shiva_activate_candle(cn, in, 0))
			{
				if (in = map[bscanB].it) it[in].active = it[in].duration;
				if (in = map[bscanC].it) it[in].active = it[in].duration;
				if (in = map[bscanD].it) it[in].active = it[in].duration;
				if (in = map[bscanE].it) it[in].active = it[in].duration;
				for (n = 1; n<MAXCHARS; n++)
				{
					if (ch[n].used==USE_EMPTY) continue;
					if (ch[n].flags & (CF_PLAYER | CF_USURP)) continue;
					if (ch[n].data[25]==2 && (ch[n].data[26]>=magenum*100+1 && ch[n].data[26]<=magenum*100+99))
					{
						do_char_killed(0, n, 1);
					}
				}
			}
		}
		return; // No process when nobody is in the area
	}
	
	// Spawn some stuff
	// a) Under 60% hp and over 30% mana (Fighting someone!)
	// b) There is someone here - once per in-game minute
	// c) Spawn a new candle - once per in-game hour, faster with more people
	if ((ch[cn].a_hp<ch[cn].hp[5]*600 && ch[cn].a_mana>ch[cn].mana[5]*300)
		|| (pinline && ch[cn].a_hp>=ch[cn].hp[5]*999 && globs->mdtime%MD_MIN==0) 
		|| (ch[cn].a_mana>ch[cn].mana[5]*666 && (pinline<1 
		|| (ch[cn].data[3]<2 && ch[cn].data[2]>=BS_RC*1) || (ch[cn].data[3]< 4 && ch[cn].data[2]>=BS_RC*2)
		|| (ch[cn].data[3]<7 && ch[cn].data[2]>=BS_RC*3) || (ch[cn].data[3]<11 && ch[cn].data[2]>=BS_RC*4))))
	{
		// Count currently active BS raid minions.
		for (n = 1, m = 0, j = 0; n<MAXCHARS; n++)
		{
			if (ch[n].used!=USE_ACTIVE) continue;
			if (ch[n].flags & (CF_BODY | CF_RESPAWN)) continue;
			if (ch[n].data[CHD_GROUP]==bs_group) 
			{
				m++;
				if (ch[n].data[0]<2) j++;
			}
		}
		
		// Store the count so it can be pulled by the raid
		ch[cn].data[1] = j;
		
		// Heal if we're panic-summoning
		if (ch[cn].a_hp<ch[cn].hp[5]*600) ch[cn].a_mana -= ch[cn].mana[5]*333;	ch[cn].a_hp += ch[cn].hp[5]*500;
		
		// If there are less than BS_COUNTER monsters in the given group...
		if (m<BS_COUNTER+3)
		{
			n = 0;
			
			if ((in = map[bscanA].it) && !it[in].active) n++;
			if ((in = map[bscanB].it) && !it[in].active) n++;
			if ((in = map[bscanC].it) && !it[in].active) n++;
			if ((in = map[bscanD].it) && !it[in].active) n++;
			if ((in = map[bscanE].it) && !it[in].active) n++;
			
			switch (n)
			{
				case 1:
													ch[cn].data[3] =  1;
					break;
				case 2:
													ch[cn].data[3] =  2;
					if (ch[cn].data[2]>=BS_RC*1)	ch[cn].data[3] =  3;
					break;
				case 3:
													ch[cn].data[3] =  4;
					if (ch[cn].data[2]>=BS_RC*1)	ch[cn].data[3] =  5;
					if (ch[cn].data[2]>=BS_RC*2)	ch[cn].data[3] =  6;
					break;
				case 4:
													ch[cn].data[3] =  7;
					if (ch[cn].data[2]>=BS_RC*1)	ch[cn].data[3] =  8;
					if (ch[cn].data[2]>=BS_RC*2)	ch[cn].data[3] =  9;
					if (ch[cn].data[2]>=BS_RC*3)	ch[cn].data[3] = 10;
					break;
				case 5:
					for (j=0;j<256;j++)
					{
						if (ch[cn].data[2]>=BS_RC*j && ch[cn].data[2]<BS_RC*(j+1))
						{
							ch[cn].data[3] = 11+j;
							break;
						}
					}
					break;
				default: 
													ch[cn].data[3] =  0;
					break;
			}
			
			if (n==0
			|| (n==1 && ch[cn].data[2]>=BS_RC*1)
			|| (n==2 && ch[cn].data[2]>=BS_RC*2)
			|| (n==3 && ch[cn].data[2]>=BS_RC*3)
			|| (n==4 && ch[cn].data[2]>=BS_RC*4))
			{
				if ((in = map[bscanA].it) && it[in].active)
				{
					if (shiva_activate_candle(cn, in, 1)) return;
				}
				if ((in = map[bscanB].it) && it[in].active)
				{
					if (shiva_activate_candle(cn, in, 2)) return;
				}
				if ((in = map[bscanC].it) && it[in].active)
				{
					if (shiva_activate_candle(cn, in, 3)) return;
				}
				if ((in = map[bscanD].it) && it[in].active)
				{
					if (shiva_activate_candle(cn, in, 4)) return;
				}
				if ((in = map[bscanE].it) && it[in].active)
				{
					if (shiva_activate_candle(cn, in, 5)) return;
				}
			}
			
			if (ch[cn].a_hp<ch[cn].hp[5]*600) ch[cn].data[3] = -1;

			if (ch[cn].data[3]>ch[cn].data[4])
			{
				for (n = 1; n<MAXCHARS; n++) 
				{
					if (ch[n].used==USE_EMPTY) continue;
					if (!(ch[n].flags & (CF_PLAYER | CF_USURP))) continue;
					if (is_inline(n, magenum)) do_char_log(n, 3, " --- WAVE %d START --- \n", ch[cn].data[3]);
				}
				ch[cn].data[4]=ch[cn].data[3];
			}
			
			// If we have candles... 
			if (ch[cn].data[3]!=0)
			{
				switch(ch[cn].data[3])
				{
					case -1:
						for (m= 0; m<12; m++) spawn_temp[m] = bs_temp;
						for (m=12; m<24; m++) spawn_temp[m] = bs_temp+1;
						for (m=24; m<36; m++) spawn_temp[m] = bs_temp+2;
						for (m=36; m<48; m++) spawn_temp[m] = bs_temp+3;
						for (m=48; m<60; m++) spawn_temp[m] = bs_temp+4;
						break;
					// 1 Candle
					case  1:
						for (m= 0; m<60; m++) spawn_temp[m] = bs_temp;
						break;
					// 2 Candles
					case  2:
						for (m= 0; m<30; m++) spawn_temp[m] = bs_temp;
						for (m=30; m<60; m++) spawn_temp[m] = bs_temp+1;
						break;
					case  3:
						for (m= 0; m<20; m++) spawn_temp[m] = bs_temp;
						for (m=20; m<60; m++) spawn_temp[m] = bs_temp+1;
						break;
					// 3 Candles
					case  4:
						for (m= 0; m<20; m++) spawn_temp[m] = bs_temp;
						for (m=20; m<40; m++) spawn_temp[m] = bs_temp+1;
						for (m=40; m<60; m++) spawn_temp[m] = bs_temp+2;
						break;
					case  5:
						for (m= 0; m<15; m++) spawn_temp[m] = bs_temp;
						for (m=15; m<36; m++) spawn_temp[m] = bs_temp+1;
						for (m=36; m<60; m++) spawn_temp[m] = bs_temp+2;
						break;
					case  6:
						for (m= 0; m<12; m++) spawn_temp[m] = bs_temp;
						for (m=12; m<32; m++) spawn_temp[m] = bs_temp+1;
						for (m=32; m<60; m++) spawn_temp[m] = bs_temp+2;
						break;
					// 4 Candles
					case  7:
						for (m= 0; m< 9; m++) spawn_temp[m] = bs_temp;
						for (m= 9; m<26; m++) spawn_temp[m] = bs_temp+1;
						for (m=26; m<43; m++) spawn_temp[m] = bs_temp+2;
						for (m=43; m<60; m++) spawn_temp[m] = bs_temp+3;
						break;
					case  8:
						for (m= 0; m< 6; m++) spawn_temp[m] = bs_temp;
						for (m= 6; m<21; m++) spawn_temp[m] = bs_temp+1;
						for (m=21; m<38; m++) spawn_temp[m] = bs_temp+2;
						for (m=38; m<60; m++) spawn_temp[m] = bs_temp+3;
						break;
					case  9:
						for (m= 0; m< 3; m++) spawn_temp[m] = bs_temp;
						for (m= 3; m<17; m++) spawn_temp[m] = bs_temp+1;
						for (m=17; m<33; m++) spawn_temp[m] = bs_temp+2;
						for (m=33; m<60; m++) spawn_temp[m] = bs_temp+3;
						break;
					case 10:
						for (m= 0; m<12; m++) spawn_temp[m] = bs_temp+1;
						for (m=12; m<28; m++) spawn_temp[m] = bs_temp+2;
						for (m=28; m<60; m++) spawn_temp[m] = bs_temp+3;
						break;
					// 5 Candles
					case 11:
						for (m= 0; m< 9; m++) spawn_temp[m] = bs_temp+1;
						for (m= 9; m<26; m++) spawn_temp[m] = bs_temp+2;
						for (m=26; m<43; m++) spawn_temp[m] = bs_temp+3;
						for (m=43; m<60; m++) spawn_temp[m] = bs_temp+4;
						break;
					case 12:
						for (m= 0; m< 6; m++) spawn_temp[m] = bs_temp+1;
						for (m= 6; m<21; m++) spawn_temp[m] = bs_temp+2;
						for (m=21; m<38; m++) spawn_temp[m] = bs_temp+3;
						for (m=38; m<60; m++) spawn_temp[m] = bs_temp+4;
						break;
					case 13:
						for (m= 0; m< 3; m++) spawn_temp[m] = bs_temp+1;
						for (m= 3; m<17; m++) spawn_temp[m] = bs_temp+2;
						for (m=17; m<33; m++) spawn_temp[m] = bs_temp+3;
						for (m=33; m<60; m++) spawn_temp[m] = bs_temp+4;
						break;
					case 14:
						for (m= 0; m<12; m++) spawn_temp[m] = bs_temp+2;
						for (m=12; m<28; m++) spawn_temp[m] = bs_temp+3;
						for (m=28; m<60; m++) spawn_temp[m] = bs_temp+4;
						break;
					case 15:
						for (m= 0; m< 9; m++) spawn_temp[m] = bs_temp+2;
						for (m= 9; m<25; m++) spawn_temp[m] = bs_temp+3;
						for (m=25; m<60; m++) spawn_temp[m] = bs_temp+4;
						break;
					// Wave convergence and endlessness
					case 16:
						for (m= 0; m< 2; m++) spawn_temp[m] = bs_temp+1;
						for (m= 2; m< 8; m++) spawn_temp[m] = bs_temp+2;
						for (m= 8; m<23; m++) spawn_temp[m] = bs_temp+3;
						for (m=23; m<58; m++) spawn_temp[m] = bs_temp+4;
						for (m=58; m<60; m++) spawn_temp[m] = bs_temp+5;
						for (j=0; j<4; j++)
						{
							  hp_bonus[j] = (magenum*25)*(5-j)-(magenum*25)*2;
							attr_bonus[j] = (3+magenum)*(5-j)-(3+magenum)*2;
							  ws_bonus[j] = (10-magenum*2)*(5-j)-(10-magenum)*2;
							  wv_bonus[j] = 6*(5-j)-12;
						}
						break;
					case 17:
						for (m= 0; m< 2; m++) spawn_temp[m] = bs_temp;
						for (m= 2; m< 6; m++) spawn_temp[m] = bs_temp+1;
						for (m= 6; m<13; m++) spawn_temp[m] = bs_temp+2;
						for (m=13; m<27; m++) spawn_temp[m] = bs_temp+3;
						for (m=27; m<56; m++) spawn_temp[m] = bs_temp+4;
						for (m=56; m<60; m++) spawn_temp[m] = bs_temp+5;
						for (j=0; j<4; j++)
						{
							  hp_bonus[j] = (magenum*25)*(5-j)-(magenum*25)*3/2;
							attr_bonus[j] = (3+magenum)*(5-j)-(3+magenum)*3/2;
							  ws_bonus[j] = (10-magenum*2)*(5-j)-(10-magenum)*3/2;
							  wv_bonus[j] = 6*(5-j)-9;
						}
						break;
					case 18:
						for (m= 0; m< 4; m++) spawn_temp[m] = bs_temp;
						for (m= 4; m<10; m++) spawn_temp[m] = bs_temp+1;
						for (m=10; m<18; m++) spawn_temp[m] = bs_temp+2;
						for (m=18; m<31; m++) spawn_temp[m] = bs_temp+3;
						for (m=31; m<56; m++) spawn_temp[m] = bs_temp+4;
						for (m=56; m<60; m++) spawn_temp[m] = bs_temp+5;
						for (j=0; j<4; j++)
						{
							  hp_bonus[j] = (magenum*25)*(5-j)-(magenum*25);
							attr_bonus[j] = (3+magenum)*(5-j)-(3+magenum);
							  ws_bonus[j] = (10-magenum*2)*(5-j)-(10-magenum);
							  wv_bonus[j] = 6*(5-j)-6;
						}
						break;
					case 19:
						for (m= 0; m< 6; m++) spawn_temp[m] = bs_temp;
						for (m= 6; m<14; m++) spawn_temp[m] = bs_temp+1;
						for (m=14; m<23; m++) spawn_temp[m] = bs_temp+2;
						for (m=23; m<35; m++) spawn_temp[m] = bs_temp+3;
						for (m=35; m<54; m++) spawn_temp[m] = bs_temp+4;
						for (m=54; m<60; m++) spawn_temp[m] = bs_temp+5;
						for (j=0; j<5; j++)
						{
							  hp_bonus[j] = (magenum*25)*(5-j)-(magenum*25)/2;
							attr_bonus[j] = (3+magenum)*(5-j)-(3+magenum)/2;
							  ws_bonus[j] = (10-magenum*2)*(5-j)-(10-magenum)/2;
							  wv_bonus[j] = 6*(5-j)-3;
						}
						break;
					case 20:
						for (m= 0; m< 8; m++) spawn_temp[m] = bs_temp;
						for (m= 8; m<17; m++) spawn_temp[m] = bs_temp+1;
						for (m=17; m<26; m++) spawn_temp[m] = bs_temp+2;
						for (m=26; m<37; m++) spawn_temp[m] = bs_temp+3;
						for (m=37; m<52; m++) spawn_temp[m] = bs_temp+4;
						for (m=52; m<60; m++) spawn_temp[m] = bs_temp+5;
						for (j=0; j<5; j++)
						{
							  hp_bonus[j] = (magenum*25)*(5-j);
							attr_bonus[j] = (3+magenum)*(5-j);
							  ws_bonus[j] = (10-magenum*2)*(5-j);
							  wv_bonus[j] = 6*(5-j);
						}
						break;
					default:
						for (m= 0; m<10; m++) spawn_temp[m] = bs_temp;
						for (m=10; m<20; m++) spawn_temp[m] = bs_temp+1;
						for (m=20; m<30; m++) spawn_temp[m] = bs_temp+2;
						for (m=30; m<40; m++) spawn_temp[m] = bs_temp+3;
						for (m=40; m<50; m++) spawn_temp[m] = bs_temp+4;
						for (m=50; m<60; m++) spawn_temp[m] = bs_temp+5;
						for (j=0; j<6; j++)
						{
							  hp_bonus[j] = (magenum*25)*(5-j)+((magenum*5)*(ch[cn].data[3]-20));
							attr_bonus[j] = (3+magenum)*(5-j)+((3+magenum)*(ch[cn].data[3]-20));
							  ws_bonus[j] = (10-magenum*2)*(5-j)+((10-magenum)*(ch[cn].data[3]-20))/4;
							  wv_bonus[j] = 6*(5-j)+(ch[cn].data[3]-20)/3;
						}
						break;
				}
				
				// Spawn things!
				for (m = 0; m<min(6,2+pinline); m++)
				{
					modified=0;
					j = spawn_temp[RANDOM(60)];
					// ** generate_map_enemy starts here **
					co = pop_create_char(j, 0);
					if (!co)
					{
						do_sayx(cn, "create char failed (%d)", ch[cn].data[3]);
						break;
					}
					if (!god_drop_char_fuzzy(co, bs_x, bs_y))
					{
						do_sayx(cn, "drop char failed (%d)", ch[cn].data[3]);
						god_destroy_items(co);
						ch[co].used = USE_EMPTY;
						break;
					}
					fx_add_effect(6, 0, ch[co].x, ch[co].y, 0);
					//
					j = j-bs_temp;
					if (j>=0 && j<=5)
					{
						if (attr_bonus[j]>0)
						{
							for (n = 0; n<5; n++)
							{
								B_AT(co, n) = B_AT(co, n)+attr_bonus[j]<248?B_AT(co, n)+attr_bonus[j]:248;
							}
							ch[co].hp[0]	= ch[co].hp[0]+hp_bonus[j]<999?ch[co].hp[0]+hp_bonus[j]:999;
							ch[co].mana[0]	= ch[co].mana[0]+hp_bonus[j]<999?ch[co].mana[0]+hp_bonus[j]:999;
							B_SK(co, n) 	= B_SK(co, n)+ws_bonus[j]<248?B_SK(co, n)+ws_bonus[j]:248;
							for (n = 1; n<MAXSKILL; n++)
							{
								if (B_SK(co, n))
								{
									B_SK(co, n) = B_SK(co, n)+attr_bonus[j]/2<243?B_SK(co, n)+attr_bonus[j]/2:243;
								}
							}
							ch[co].weapon = ch[co].weapon+wv_bonus[j]<255?ch[co].weapon+wv_bonus[j]:255;
							ch[co].armor  = ch[co].armor+wv_bonus[j]<255?ch[co].armor+wv_bonus[j]:255;
							modified=1;
						}
					}
					//
					if (ch[cn].data[3]>=7 && !RANDOM(max(1,30-(pinline*2))))
					{
						for (n = 0; n<5; n++)
						{
							B_AT(co, n) += 2+RANDOM(3+magenum);
						}
						for (n = 0; n<MAXSKILL; n++)
						{
							if (B_SK(co, n))
							{
								B_SK(co, n) += 2+RANDOM(3+magenum)*2;
							}
						}
						sprintf(buf, "the tough %s", ch[co].name);
						buf[10] = tolower(buf[10]);
						strncpy(ch[co].reference, buf, 39);
						ch[co].reference[39] = 0;
						
						sprintf(buf, "Tough %s", ch[co].name);
						strncpy(ch[co].name, buf, 39);
						ch[co].name[39] = 0;
						
						if (!(ch[co].flags & CF_EXTRAEXP)) 
							ch[co].flags |= CF_EXTRAEXP;
						modified=1;
					}
					else if (ch[cn].data[3]>=16 && !RANDOM(max(1,40-(pinline*2))))
					{
						for (n = 0; n<5; n++)
						{
							B_AT(co, n) += 4+RANDOM(5+magenum);
						}
						for (n = 0; n<MAXSKILL; n++)
						{
							if (B_SK(co, n))
							{
								B_SK(co, n) += 4+RANDOM(5+magenum)*3;
							}
						}
						sprintf(buf, "the fierce %s", ch[co].name);
						buf[11] = tolower(buf[11]);
						strncpy(ch[co].reference, buf, 39);
						ch[co].reference[39] = 0;
						
						sprintf(buf, "Fierce %s", ch[co].name);
						strncpy(ch[co].name, buf, 39);
						ch[co].name[39] = 0;
						
						if (!(ch[co].flags & CF_EXTRAEXP)) 
							ch[co].flags |= CF_EXTRAEXP;
						if (!(ch[co].flags & CF_EXTRACRIT)) 
							ch[co].flags |= CF_EXTRACRIT;
						modified=1;
					}
					if (modified)
					{
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
					}
					// ** generate_map_enemy ends here **
				}
				
				fx_add_effect(7, 0, ch[cn].x, ch[cn].y, 0);
				do_sayx(cn, "Khuzak gurawin duskar!");
				chlog(cn, "created %d new monsters", n);
				if (ch[cn].data[3]<BS_RC*256) 
					ch[cn].data[2]+=1+pinline;
			}
		}
	}
}

/*
	Stronghold mage version 2
	Each mage has their own distinct series of waves, pulling from monsters in helper.c
		via generate_map_enemy(cn, temp, kin, xx, yy, base, affix, tarot)
	
	Each mage has 15 waves, 10 subwaves, and up to 6 monsters per subwave. 
		Each subwave has a breif timer between spawns.
	
	Waves last until all subwaves have passed and all monsters are killed either by players or NPCs.
	
	Mage 1 starts at base 15 adding 1 per wave,
	Mage 2 starts at base 30 adding 2 per wave,
	Mage 3 starts at base 60 adding 3 per wave.
	
	Valid Monsters:
	 1: Skeleton	 2: Ghost		 3: Rodent		 4: Undead		 5: Grolm
	 6: Skink	 	 7: Golem		 8: Gargoyle	 9: Ice Garg	10: Flame
*/

// stronghold wave sets - [mage][wave][sub][6monster&time]
static int bs_waves[3][15][9][7] = {
{		// Mage 1
	{			// Wave  1
		{ 1, 0, 0, 0, 0, 0, TICKS * 5 }, { 1, 0, 0, 0, 0, 0, TICKS * 5 }, { 1, 0, 0, 0, 0, 0, TICKS * 5 }, 
		{ 1, 0, 0, 0, 0, 0, TICKS * 5 }, { 1, 0, 0, 0, 0, 0, TICKS * 5 }, { 1, 0, 0, 0, 0, 0, TICKS * 5 },
		{ 1, 0, 0, 0, 0, 0, TICKS * 5 }, { 1, 0, 0, 0, 0, 0, TICKS * 5 }, { 1, 0, 0, 0, 0, 0, TICKS *15 }
	}, {		// Wave  2
		{ 1, 0, 0, 0, 0, 0, TICKS * 5 }, { 1, 0, 0, 0, 0, 0, TICKS * 5 }, { 1, 1, 0, 0, 0, 0, TICKS *10 }, 
		{ 1, 0, 0, 0, 0, 0, TICKS * 5 }, { 1, 0, 0, 0, 0, 0, TICKS * 5 }, { 1, 1, 0, 0, 0, 0, TICKS *10 },
		{ 1, 0, 0, 0, 0, 0, TICKS * 5 }, { 1, 1, 0, 0, 0, 0, TICKS *10 }, { 1, 1, 0, 0, 0, 0, TICKS *30 }
	}, {		// Wave  3
		{ 1, 0, 0, 0, 0, 0, TICKS * 5 }, { 1, 0, 0, 0, 0, 0, TICKS * 5 }, { 2, 0, 0, 0, 0, 0, TICKS * 5 }, 
		{ 1, 0, 0, 0, 0, 0, TICKS * 5 }, { 1, 0, 0, 0, 0, 0, TICKS * 5 }, { 2, 1, 0, 0, 0, 0, TICKS *10 },
		{ 1, 0, 0, 0, 0, 0, TICKS * 5 }, { 2, 1, 0, 0, 0, 0, TICKS *10 }, { 2, 2, 0, 0, 0, 0, TICKS *30 }
	}, {		// Wave  4
		{ 2, 0, 0, 0, 0, 0, TICKS * 5 }, { 2, 0, 0, 0, 0, 0, TICKS * 5 }, { 2, 1, 0, 0, 0, 0, TICKS *10 }, 
		{ 2, 0, 0, 0, 0, 0, TICKS * 5 }, { 2, 1, 0, 0, 0, 0, TICKS *10 }, { 2, 2, 0, 0, 0, 0, TICKS *10 },
		{ 2, 0, 0, 0, 0, 0, TICKS * 5 }, { 2, 2, 0, 0, 0, 0, TICKS *10 }, { 2, 2, 1, 0, 0, 0, TICKS *45 }
	}, {		// Wave  5
		{ 3, 0, 0, 0, 0, 0, TICKS * 5 }, { 3, 0, 0, 0, 0, 0, TICKS * 5 }, { 3, 0, 0, 0, 0, 0, TICKS * 5 }, 
		{ 3, 0, 0, 0, 0, 0, TICKS * 5 }, { 3, 0, 0, 0, 0, 0, TICKS * 5 }, { 3, 0, 0, 0, 0, 0, TICKS * 5 },
		{ 3, 0, 0, 0, 0, 0, TICKS * 5 }, { 3, 0, 0, 0, 0, 0, TICKS * 5 }, { 3, 3, 3, 0, 0, 0, TICKS *45 }
	}, {		// Wave  6
		{ 1, 0, 0, 0, 0, 0, TICKS * 5 }, { 2, 0, 0, 0, 0, 0, TICKS * 5 }, { 3, 2, 0, 0, 0, 0, TICKS *10 }, 
		{ 2, 0, 0, 0, 0, 0, TICKS * 5 }, { 2, 0, 0, 0, 0, 0, TICKS * 5 }, { 3, 1, 0, 0, 0, 0, TICKS *10 },
		{ 3, 0, 0, 0, 0, 0, TICKS * 5 }, { 3, 3, 0, 0, 0, 0, TICKS *10 }, { 1, 1, 1, 0, 0, 0, TICKS *45 }
	}, {		// Wave  7
		{ 1, 1, 0, 0, 0, 0, TICKS * 9 }, { 1, 1, 0, 0, 0, 0, TICKS * 9 }, { 2, 2, 0, 0, 0, 0, TICKS * 9 }, 
		{ 2, 2, 0, 0, 0, 0, TICKS * 9 }, { 3, 3, 0, 0, 0, 0, TICKS * 9 }, { 3, 3, 0, 0, 0, 0, TICKS * 9 },
		{ 1, 2, 0, 0, 0, 0, TICKS * 9 }, { 4, 0, 0, 0, 0, 0, TICKS * 5 }, { 4, 4, 0, 0, 0, 0, TICKS *27 }
	}, {		// Wave  8
		{ 4, 1, 0, 0, 0, 0, TICKS * 9 }, { 4, 2, 0, 0, 0, 0, TICKS * 9 }, { 4, 4, 0, 0, 0, 0, TICKS * 9 }, 
		{ 4, 2, 0, 0, 0, 0, TICKS * 9 }, { 4, 1, 0, 0, 0, 0, TICKS * 9 }, { 4, 4, 0, 0, 0, 0, TICKS * 9 },
		{ 4, 4, 0, 0, 0, 0, TICKS * 9 }, { 4, 4, 0, 0, 0, 0, TICKS * 9 }, { 5, 1, 1, 0, 0, 0, TICKS *41 }
	}, {		// Wave  9
		{ 5, 0, 0, 0, 0, 0, TICKS * 5 }, { 5, 0, 0, 0, 0, 0, TICKS * 4 }, { 5, 1, 0, 0, 0, 0, TICKS * 9 }, 
		{ 5, 0, 0, 0, 0, 0, TICKS * 5 }, { 5, 0, 0, 0, 0, 0, TICKS * 4 }, { 5, 2, 0, 0, 0, 0, TICKS * 9 },
		{ 5, 0, 0, 0, 0, 0, TICKS * 5 }, { 5, 3, 0, 0, 0, 0, TICKS * 9 }, { 5, 5, 4, 4, 0, 0, TICKS *54 }
	}
}, {	// Mage 2
	{			// Wave  1
		{ 1, 0, 0, 0, 0, 0, TICKS * 5 }, { 1, 0, 0, 0, 0, 0, TICKS * 4 }, { 1, 1, 0, 0, 0, 0, TICKS * 9 }, 
		{ 1, 0, 0, 0, 0, 0, TICKS * 5 }, { 1, 0, 0, 0, 0, 0, TICKS * 4 }, { 1, 1, 0, 0, 0, 0, TICKS * 9 },
		{ 1, 0, 0, 0, 0, 0, TICKS * 5 }, { 1, 0, 0, 0, 0, 0, TICKS * 4 }, { 1, 1, 0, 0, 0, 0, TICKS *27 }
	}, {		// Wave  2
		{ 1, 0, 0, 0, 0, 0, TICKS * 5 }, { 2, 0, 0, 0, 0, 0, TICKS * 4 }, { 1, 0, 0, 0, 0, 0, TICKS * 5 }, 
		{ 2, 0, 0, 0, 0, 0, TICKS * 4 }, { 1, 2, 0, 0, 0, 0, TICKS * 9 }, { 2, 1, 0, 0, 0, 0, TICKS * 9 },
		{ 1, 2, 0, 0, 0, 0, TICKS * 9 }, { 2, 1, 0, 0, 0, 0, TICKS * 9 }, { 4, 4, 0, 0, 0, 0, TICKS *27 }
	}, {		// Wave  3
		{ 3, 3, 0, 0, 0, 0, TICKS * 9 }, { 3, 3, 0, 0, 0, 0, TICKS * 9 }, { 3, 3, 0, 0, 0, 0, TICKS * 9 }, 
		{ 3, 3, 0, 0, 0, 0, TICKS * 9 }, { 3, 3, 0, 0, 0, 0, TICKS * 9 }, { 3, 3, 0, 0, 0, 0, TICKS * 9 },
		{ 3, 3, 0, 0, 0, 0, TICKS * 9 }, { 3, 3, 0, 0, 0, 0, TICKS * 9 }, { 3, 3, 0, 0, 0, 0, TICKS *27 }
	}, {		// Wave  4
		{ 1, 1, 0, 0, 0, 0, TICKS * 8 }, { 1, 2, 0, 0, 0, 0, TICKS * 8 }, { 2, 2, 0, 0, 0, 0, TICKS * 8 }, 
		{ 2, 1, 0, 0, 0, 0, TICKS * 8 }, { 1, 1, 0, 0, 0, 0, TICKS * 8 }, { 1, 2, 4, 0, 0, 0, TICKS *12 },
		{ 1, 1, 4, 0, 0, 0, TICKS *12 }, { 2, 1, 4, 0, 0, 0, TICKS *12 }, { 2, 2, 4, 0, 0, 0, TICKS *36 }
	}, {		// Wave  5
		{ 3, 1, 1, 0, 0, 0, TICKS *12 }, { 3, 1, 1, 0, 0, 0, TICKS *12 }, { 3, 2, 2, 0, 0, 0, TICKS *12 }, 
		{ 3, 2, 2, 0, 0, 0, TICKS *12 }, { 3, 4, 4, 0, 0, 0, TICKS *12 }, { 3, 4, 4, 0, 0, 0, TICKS *12 },
		{ 3, 3, 3, 0, 0, 0, TICKS *12 }, { 3, 3, 3, 0, 0, 0, TICKS *12 }, { 3, 5, 5, 0, 0, 0, TICKS *36 }
	}, {		// Wave  6
		{ 5, 5, 0, 0, 0, 0, TICKS * 8 }, { 5, 5, 0, 0, 0, 0, TICKS * 8 }, { 5, 5, 1, 0, 0, 0, TICKS *12 }, 
		{ 5, 5, 0, 0, 0, 0, TICKS * 8 }, { 5, 5, 0, 0, 0, 0, TICKS * 8 }, { 5, 5, 2, 0, 0, 0, TICKS *12 },
		{ 5, 5, 3, 0, 0, 0, TICKS *12 }, { 5, 5, 4, 0, 0, 0, TICKS *12 }, { 5, 5, 5, 0, 0, 0, TICKS *36 }
	}, {		// Wave  7
		{ 5, 5, 2, 0, 0, 0, TICKS *12 }, { 5, 5, 2, 0, 0, 0, TICKS *12 }, { 5, 5, 1, 1, 0, 0, TICKS *16 }, 
		{ 5, 5, 3, 0, 0, 0, TICKS *12 }, { 5, 5, 3, 0, 0, 0, TICKS *12 }, { 5, 5, 4, 4, 0, 0, TICKS *16 },
		{ 4, 2, 1, 0, 0, 0, TICKS *12 }, { 4, 1, 2, 0, 0, 0, TICKS *12 }, { 5, 5, 6, 6, 0, 0, TICKS *48 }
	}, {		// Wave  8
		{ 7, 0, 0, 0, 0, 0, TICKS * 4 }, { 7, 0, 0, 0, 0, 0, TICKS * 4 }, { 7, 7, 3, 3, 0, 0, TICKS *16 }, 
		{ 7, 0, 0, 0, 0, 0, TICKS * 4 }, { 7, 0, 0, 0, 0, 0, TICKS * 4 }, { 7, 7, 6, 6, 0, 0, TICKS *16 },
		{ 8, 0, 0, 0, 0, 0, TICKS * 4 }, { 8, 0, 0, 0, 0, 0, TICKS * 4 }, { 8, 8, 7, 7, 0, 0, TICKS *48 }
	}, {		// Wave  9
		{ 8, 8, 0, 0, 0, 0, TICKS * 8 }, { 7, 7, 0, 0, 0, 0, TICKS * 8 }, { 8, 8, 0, 0, 0, 0, TICKS * 8 }, 
		{ 7, 7, 0, 0, 0, 0, TICKS * 8 }, { 6, 6, 6, 6, 0, 0, TICKS *16 }, { 7, 7, 0, 0, 0, 0, TICKS * 8 },
		{ 8, 8, 0, 0, 0, 0, TICKS * 8 }, { 7, 7, 0, 0, 0, 0, TICKS * 8 }, { 8, 8, 0, 0, 0, 0, TICKS *24 }
	}, {		// Wave 10
		{ 1, 1, 1, 1, 0, 0, TICKS *14 }, { 1, 1, 1, 1, 0, 0, TICKS *14 }, { 1, 1, 1, 1, 0, 0, TICKS *14 }, 
		{ 1, 1, 1, 1, 0, 0, TICKS *14 }, { 1, 1, 1, 0, 0, 0, TICKS *11 }, { 1, 1, 1, 0, 0, 0, TICKS *11 },
		{ 1, 1, 1, 0, 0, 0, TICKS *11 }, { 1, 1, 1, 0, 0, 0, TICKS *11 }, { 7, 8, 7, 8, 0, 0, TICKS *42 }
	}, {		// Wave 11
		{ 1, 2, 4, 4, 0, 0, TICKS *14 }, { 2, 2, 4, 4, 0, 0, TICKS *14 }, { 3, 3, 4, 4, 0, 0, TICKS *14 }, 
		{ 4, 5, 6, 6, 0, 0, TICKS *14 }, { 5, 5, 6, 6, 0, 0, TICKS *14 }, { 7, 7, 6, 6, 0, 0, TICKS *14 },
		{ 1, 2, 3, 4, 0, 0, TICKS *14 }, { 5, 6, 7, 8, 0, 0, TICKS *14 }, { 2, 4, 6, 8, 0, 0, TICKS *42 }
	}, {		// Wave 12
		{ 8, 8, 0, 0, 0, 0, TICKS * 7 }, { 8, 8, 0, 0, 0, 0, TICKS * 7 }, { 8, 8, 8, 0, 0, 0, TICKS *11 }, 
		{ 7, 8, 0, 0, 0, 0, TICKS * 7 }, { 6, 8, 0, 0, 0, 0, TICKS * 7 }, { 5, 6, 7, 0, 0, 0, TICKS *11 },
		{ 1, 2, 0, 0, 0, 0, TICKS * 7 }, { 3, 4, 0, 0, 0, 0, TICKS * 7 }, { 9, 9, 9, 9, 0, 0, TICKS *42 }
	}
}, {	// Mage 3
	{			// Wave  1
		{10, 0, 0, 0, 0, 0, TICKS * 4 }, {10, 0, 0, 0, 0, 0, TICKS * 4 }, {10,10, 0, 0, 0, 0, TICKS * 7 }, 
		{10,10, 0, 0, 0, 0, TICKS * 7 }, {10,10,10, 0, 0, 0, TICKS *11 }, {10,10,10, 0, 0, 0, TICKS *11 },
		{10,10,10,10, 0, 0, TICKS *14 }, {10,10,10,10, 0, 0, TICKS *14 }, {10,10,10,10,10, 0, TICKS *53 }
	}, {		// Wave  2
		{ 1, 1, 0, 0, 0, 0, TICKS * 7 }, { 2, 2, 0, 0, 0, 0, TICKS * 7 }, { 1, 1, 0, 0, 0, 0, TICKS * 7 }, 
		{ 4, 4, 0, 0, 0, 0, TICKS * 7 }, { 1, 1, 1, 0, 0, 0, TICKS *11 }, { 2, 2, 2, 0, 0, 0, TICKS *11 },
		{ 1, 1, 1, 0, 0, 0, TICKS *11 }, { 4, 4, 4, 0, 0, 0, TICKS *11 }, { 1, 1, 1, 1, 0, 0, TICKS *42 }
	}, {		// Wave  3
		{ 1, 2, 0, 0, 0, 0, TICKS * 7 }, { 2, 1, 0, 0, 0, 0, TICKS * 7 }, { 1, 1, 2, 0, 0, 0, TICKS *11 }, 
		{ 1, 2, 2, 0, 0, 0, TICKS *11 }, { 2, 2, 2, 0, 0, 0, TICKS *11 }, { 2, 2, 1, 1, 0, 0, TICKS *14 },
		{ 1, 1, 2, 2, 0, 0, TICKS *14 }, { 1, 2, 2, 4, 0, 0, TICKS *14 }, { 1, 2, 4, 4, 0, 0, TICKS *42 }
	}, {		// Wave  4
		{ 3, 3, 3, 3, 0, 0, TICKS *12 }, { 3, 3, 3, 3, 0, 0, TICKS *12 }, { 3, 3, 3, 3, 0, 0, TICKS *12 }, 
		{ 3, 3, 3, 3, 0, 0, TICKS *12 }, { 3, 3, 3, 3, 0, 0, TICKS *12 }, { 3, 3, 3, 3, 0, 0, TICKS *12 },
		{ 3, 3, 3, 3, 3, 0, TICKS *15 }, { 3, 3, 3, 3, 3, 0, TICKS *15 }, { 3, 3, 3, 3, 3, 0, TICKS *45 }
	}, {		// Wave  5
		{ 5, 5, 5, 0, 0, 0, TICKS * 9 }, { 5, 5, 8, 0, 0, 0, TICKS * 9 }, { 5, 8, 8, 0, 0, 0, TICKS * 9 }, 
		{ 5, 8, 9, 0, 0, 0, TICKS * 9 }, { 8, 8, 9, 0, 0, 0, TICKS * 9 }, { 8, 9, 9, 0, 0, 0, TICKS * 9 },
		{ 9, 9, 9, 0, 0, 0, TICKS * 9 }, { 5, 5, 8, 9, 0, 0, TICKS *12 }, { 5, 8, 8, 9, 0, 0, TICKS *36 }
	}, {		// Wave  6
		{ 7, 6, 7, 8, 0, 0, TICKS *12 }, { 7, 4, 7, 9, 0, 0, TICKS *12 }, { 7, 6, 7, 8, 0, 0, TICKS *12 }, 
		{ 7, 1, 7, 2, 0, 0, TICKS *12 }, { 7, 3, 7,10, 0, 0, TICKS *12 }, { 7, 7, 7, 7, 0, 0, TICKS *12 },
		{ 7, 5, 7, 5, 0, 0, TICKS *12 }, { 5, 8, 5, 8, 0, 0, TICKS *12 }, { 8, 9, 8, 9, 7, 0, TICKS *45 }
	}, {		// Wave  7
		{ 1, 2, 3, 4, 0, 0, TICKS *12 }, { 1, 2, 3, 4, 0, 0, TICKS *12 }, { 2, 3, 4, 5, 0, 0, TICKS *12 }, 
		{ 2, 3, 4, 5, 0, 0, TICKS *12 }, { 3, 4, 5, 6, 0, 0, TICKS *12 }, { 3, 4, 5, 6, 0, 0, TICKS *12 },
		{ 4, 5, 6, 7, 0, 0, TICKS *12 }, { 4, 5, 6, 7, 0, 0, TICKS *12 }, { 5, 6, 7, 8, 0, 0, TICKS *36 }
	}, {		// Wave  8
		{10,10, 8, 8, 0, 0, TICKS *12 }, {10,10, 8, 8, 0, 0, TICKS *12 }, {10,10, 8, 8, 8, 0, TICKS *15 }, 
		{ 8, 8, 9, 9, 0, 0, TICKS *12 }, { 8, 8, 9, 9, 0, 0, TICKS *12 }, { 8, 8, 9, 9, 9, 0, TICKS *15 },
		{ 9, 9,10,10, 0, 0, TICKS *12 }, { 9, 9,10,10, 0, 0, TICKS *12 }, { 9, 9,10,10,10, 0, TICKS *45 }
	}, {		// Wave  9
		{ 6, 6, 6, 7, 0, 0, TICKS *12 }, { 6, 6, 7, 7, 0, 0, TICKS *12 }, { 6, 7, 7, 7, 0, 0, TICKS *12 }, 
		{ 6, 6, 7, 7, 0, 0, TICKS *12 }, { 6, 6, 6, 7, 0, 0, TICKS *12 }, { 5, 6, 6, 6, 7, 0, TICKS *15 },
		{ 5, 6, 6, 7, 7, 0, TICKS *15 }, { 5, 6, 7, 7, 7, 0, TICKS *15 }, { 5, 5, 5, 6, 7, 0, TICKS *45 }
	}, {		// Wave 10
		{ 3, 3, 3, 0, 0, 0, TICKS * 8 }, { 3, 3, 3, 0, 0, 0, TICKS * 7 }, { 3, 3, 3, 3, 0, 0, TICKS *10 }, 
		{ 1, 1, 1, 0, 0, 0, TICKS * 8 }, { 1, 1, 1, 0, 0, 0, TICKS * 7 }, { 1, 1, 1, 1, 0, 0, TICKS *10 },
		{ 3, 3, 3, 3, 0, 0, TICKS *10 }, { 3, 3, 3, 3, 0, 0, TICKS *10 }, { 3, 3, 3, 3, 1, 1, TICKS *45 }
	}, {		// Wave 11
		{ 2, 2, 6, 6, 0, 0, TICKS *10 }, { 2, 2, 6, 6, 0, 0, TICKS *10 }, { 2, 2, 7, 7, 0, 0, TICKS *10 }, 
		{ 2, 2, 2, 4, 4, 0, TICKS *13 }, { 2, 2, 5, 5, 0, 0, TICKS *10 }, { 2, 2, 5, 5, 0, 0, TICKS *10 },
		{ 2, 2, 8, 8, 0, 0, TICKS *10 }, { 2, 2, 2, 2, 2, 0, TICKS *12 }, { 2, 4, 4, 4, 2, 0, TICKS *39 }
	}, {		// Wave 12
		{ 8, 8, 8, 8, 0, 0, TICKS *10 }, { 8, 8, 8, 9, 0, 0, TICKS *10 }, { 8, 8, 8,10, 0, 0, TICKS *10 }, 
		{ 8, 7, 8, 9, 0, 0, TICKS *10 }, { 8, 7, 8,10, 0, 0, TICKS *10 }, { 8, 7, 7, 9, 0, 0, TICKS *10 },
		{ 8, 7, 9,10, 0, 0, TICKS *10 }, { 1, 8, 1, 9,10, 0, TICKS *13 }, { 2, 2, 1, 1,10,10, TICKS *45 }
	}, {		// Wave 13
		{ 4, 4, 2, 3, 1, 0, TICKS *13 }, { 4, 4, 3, 2, 1, 0, TICKS *12 }, { 4, 4, 1, 2, 3, 0, TICKS *13 }, 
		{ 5, 6, 5, 6, 7, 0, TICKS *12 }, { 5, 6, 5, 6, 4, 0, TICKS *13 }, { 5, 6, 5, 6, 8, 0, TICKS *12 },
		{10, 9, 8, 7, 6, 0, TICKS *13 }, { 8, 7, 6, 5, 4, 0, TICKS *12 }, { 6, 5, 4, 3, 2, 1, TICKS *45 }
	}, {		// Wave 14
		{ 1, 1, 1, 1, 1, 0, TICKS *13 }, { 2, 2, 2, 2, 2, 0, TICKS *12 }, { 4, 4, 4, 4, 4, 4, TICKS *15 }, 
		{ 3, 3, 3, 3, 3, 0, TICKS *13 }, { 7, 7, 7, 7, 7, 0, TICKS *12 }, { 6, 6, 6, 6, 6, 6, TICKS *15 },
		{ 5, 5, 5, 5, 5, 0, TICKS *13 }, { 8, 8, 8, 8, 8, 0, TICKS *12 }, { 9, 9, 9, 9, 9, 9, TICKS *45 }
	}, {		// Wave 15
		{ 1, 2, 3, 4, 5, 6, TICKS *15 }, { 2, 4, 6, 8,10, 2, TICKS *15 }, { 3, 6, 9, 2, 5, 8, TICKS *15 }, 
		{ 4, 8, 2, 6,10, 4, TICKS *15 }, { 5,10, 5,10, 5,10, TICKS *15 }, { 6, 2, 8, 4,10, 6, TICKS *15 },
		{ 7, 4, 1, 8, 5, 2, TICKS *15 }, { 8, 6, 4, 2,10, 8, TICKS *15 }, { 9, 8, 7, 6, 5, 4, TICKS *45 }
	}
}
};

// Remaking the BS mage stuff to be more functionally interesting and consistent
void stronghold_mage_driver_ver2(int cn)
{
	int candleA, candleB, candleC, candleD, candleE;
	int spawnX, spawnY, magenum=0, groupnum=601;
	int p=0, n, m, j, in=0, co=0;
	int cw[4] = {0};
	
	if (!(globs->flags & GF_STRONGHOLD)) return;
	
	/*
		Mage driver data will carry references to pass along to cityattack.c
		
		data[0] = whether or not we're active / number of players
		data[1] = main wave number
		data[2] = sub wave number
		data[3] = global time for next event process
		data[4] = number of enemies spawned so far
		data[5] = number of enemies killed so far
		
		generate_map_enemy(cn, 347, bs_waves[magenum-1][ch[cn].data[1]][ch[cn].data[2]][m], spawnX, spawnY, (magenum*15+(magenum==3?15:0))+ch[cn].data[1]*magenum, 0, 0);
	*/
	
	// Assign mage number
	if (ch[cn].temp==CT_BSMAGE1) magenum = 1;
	if (ch[cn].temp==CT_BSMAGE2) magenum = 2;
	if (ch[cn].temp==CT_BSMAGE3) magenum = 3;
	
	// Check for players in the area
	for (n = 1; n<MAXCHARS; n++) 
	{
		if (ch[n].used==USE_EMPTY) continue;
		if (!(ch[n].flags & (CF_PLAYER | CF_USURP))) continue;
		if (is_inline(n, magenum)) p++;
	}
	ch[cn].data[0] = p;
	
	// Set up candles & spawn coords
	if (magenum==3)
	{
		candleA = BS_CAN_3_1;	candleB = BS_CAN_3_2;	candleC = BS_CAN_3_3;
		candleD = BS_CAN_3_4;	candleE = BS_CAN_3_5;
		spawnX = 598;	spawnY = 288;	groupnum = 603;
		cw[0] = 5; cw[1] = 8; cw[2] = 12; cw[3] = 15;
	}
	else if (magenum==2)
	{
		candleA = BS_CAN_2_1;	candleB = BS_CAN_2_2;	candleC = BS_CAN_2_3;
		candleD = BS_CAN_2_4;	candleE = BS_CAN_2_5;
		spawnX = 576;	spawnY = 288;	groupnum = 602;
		cw[0] = 4; cw[1] = 7; cw[2] = 10; cw[3] = 12;
	}
	else
	{
		candleA = BS_CAN_1_1;	candleB = BS_CAN_1_2;	candleC = BS_CAN_1_3;
		candleD = BS_CAN_1_4;	candleE = BS_CAN_1_5;
		spawnX = 554;	spawnY = 288;	groupnum = 601;
		cw[0] = 3; cw[1] = 5; cw[2] = 7; cw[3] =  9;
	}
	
	// If no players...
	if (p<1)
	{
		// Clean up candles once every 6 hours if nobody is there
		if (globs->mdtime%(MD_HOUR*6)>1 && globs->mdtime%(MD_HOUR*6)<2160 && 
			ch[cn].data[1]!=0 && ch[cn].data[1]!=1)
		{
			if (shiva_activate_candle(cn, in, 0))
			{
				if (in = map[candleB].it) it[in].active = it[in].duration;
				if (in = map[candleC].it) it[in].active = it[in].duration;
				if (in = map[candleD].it) it[in].active = it[in].duration;
				if (in = map[candleE].it) it[in].active = it[in].duration;
				for (n = 1; n<MAXCHARS; n++)
				{
					if (ch[n].used==USE_EMPTY) continue;
					if (ch[n].flags & (CF_PLAYER | CF_USURP)) continue;
					if (ch[n].data[25]==2 && (ch[n].data[26]>=magenum*100+1 && ch[n].data[26]<=magenum*100+99))
					{
						do_char_killed(0, n, 1);
					}
				}
				ch[cn].data[1] = 0;
				ch[cn].data[2] = 0;
				ch[cn].data[3] = 0;
				ch[cn].data[4] = 0;
				ch[cn].data[5] = 0;
				ch[cn].data[43] = 0;
			}
		}
		return; // No further process when nobody is in the area
	}
	
	// Pre-process waves
	if (ch[cn].data[1]==0)
	{
		ch[cn].data[1] = 1;
		ch[cn].data[2] = 1;
		ch[cn].data[3] = globs->ticker + TICKS * 3;
		ch[cn].data[4] = 0;
		ch[cn].data[5] = 0;
	}
	
	// Return if time is less than next action
	if (globs->ticker<ch[cn].data[3])
		return;
	
	// End the series
	if ((magenum==1 && ch[cn].data[1]==10) ||
		(magenum==2 && ch[cn].data[1]==13) ||
		(magenum==3 && ch[cn].data[1]==16))
	{
		ch[cn].data[43] = groupnum;
		j = 0;
		switch (ch[cn].data[2])
		{
			case 1:
				if (magenum==3)			j = npc_moveto(cn, 598, 305);
				else if (magenum==2)	j = npc_moveto(cn, 576, 305);
				else					j = npc_moveto(cn, 554, 305);
				break;
			case 2:
				if (magenum==3)			j = npc_moveto(cn, 600, 336);
				else if (magenum==2)	j = npc_moveto(cn, 576, 336);
				else					j = npc_moveto(cn, 554, 336);
				break;
			case 3:
				if (magenum==3)			npc_moveto(cn, 598, 368);
				else if (magenum==2)	npc_moveto(cn, 576, 368);
				else					npc_moveto(cn, 554, 368);
				break;
			default: break;
		}
		
		if (j) 
		{
			ch[cn].data[2]++;
			ch[cn].data[3] = globs->ticker + TICKS * 10;
		}
		return;
	}
	
	// Send the wave number to everyone in line & activate candles
	if (ch[cn].data[2]==1)
	{
		if (ch[cn].data[1]==1     && (in = map[candleA].it) && it[in].active) shiva_activate_candle(cn, in, 1);
		if (ch[cn].data[1]==cw[0] && (in = map[candleB].it) && it[in].active) shiva_activate_candle(cn, in, 2);
		if (ch[cn].data[1]==cw[1] && (in = map[candleC].it) && it[in].active) shiva_activate_candle(cn, in, 3);
		if (ch[cn].data[1]==cw[2] && (in = map[candleD].it) && it[in].active) shiva_activate_candle(cn, in, 4);
		if (ch[cn].data[1]==cw[3] && (in = map[candleE].it) && it[in].active) shiva_activate_candle(cn, in, 5);
		for (n = 1; n<MAXCHARS; n++) 
		{
			if (ch[n].used==USE_EMPTY) continue;
			if (!IS_PLAYER(n)) continue;
			if (is_inline(n, magenum)) 
			{
				do_char_log(n, 3, " --- WAVE %d BEGINS --- \n", ch[cn].data[1]);
			}
		}
	}
	
	n=0;
	// Spawn mobs
	for (m=0; m<6; m++)
	{
		if (bs_waves[magenum-1][ch[cn].data[1]-1][ch[cn].data[2]-1][m]>0 && 
			bs_waves[magenum-1][ch[cn].data[1]-1][ch[cn].data[2]-1][m]<11)
		{
			if (co = generate_map_enemy(cn, 347, bs_waves[magenum-1][ch[cn].data[1]-1][ch[cn].data[2]-1][m], 
				spawnX, spawnY, (magenum*15+(magenum==3?15:0))+ch[cn].data[1]*magenum, 
				(ch[cn].data[1]>10&&try_boost(20))?2:((ch[cn].data[1]>5&&try_boost(20))?1:0), 0))
			{
				ch[co].flags |= CF_NOSLEEP;
				ch[co].data[25] = 2;
				ch[co].data[26] = 101*magenum;
				ch[co].data[31] = ch[cn].temp;
				ch[co].data[CHD_GROUP] = ch[co].data[43] = ch[co].data[59] = groupnum;
				ch[co].data[44] = 60;
				do_update_char(co);
				ch[cn].data[4]++;
				n++;
			}
		}
	}
	
	fx_add_effect(7, 0, ch[cn].x, ch[cn].y, 0);
	if (!((ch[cn].data[2]-1)%2)) do_sayx(cn, "Khuzak gurawin duskar!");
	m = bs_waves[magenum-1][ch[cn].data[1]-1][ch[cn].data[2]-1][6];
	if (IS_GLOB_MAYHEM) m = m - m / 4;
	chlog(cn, "created %d new monsters, waiting %d ticks", n, m);
	
	// Increment subwave and/or wave number
	if (ch[cn].data[2]<8)
	{
		ch[cn].data[2]++;
	}
	else
	{
		for (n = 1; n<MAXCHARS; n++) 
		{
			if (ch[n].used==USE_EMPTY) continue;
			if (!IS_PLAYER(n)) continue;
			if (is_inline(n, magenum))
			{
				do_char_log(n, 3, " --- WAVE %d ENDS --- \n", ch[cn].data[1]);
				if (ch[n].data[26])
				{
					j = ch[cn].data[1]*ch[cn].data[1]*(magenum==3?4:magenum)*3/2+ch[n].data[26]*(magenum==3?4:magenum);
					ch[n].data[26] = 0;
					chlog(n, "Gets %d BSP", j);
					ch[n].bs_points += j;
					if (!(ch[n].flags & CF_SYS_OFF))
						do_char_log(n, 2, "You get %d stronghold points.\n", j);
				}
			}
		}
		if ((in = map[candleA].it) && !it[in].active)
		{
			it[in].data[1] = ch[cn].data[1]*ch[cn].data[1]*(magenum==3?4:magenum)*2+ch[n].data[26]*(magenum==3?4:magenum);
		}
		if ((in = map[candleB].it) && !it[in].active)
		{
			it[in].data[1] = ch[cn].data[1]*ch[cn].data[1]*(magenum==3?4:magenum)*2+ch[n].data[26]*(magenum==3?4:magenum);
		}
		if ((in = map[candleC].it) && !it[in].active)
		{
			it[in].data[1] = ch[cn].data[1]*ch[cn].data[1]*(magenum==3?4:magenum)*2+ch[n].data[26]*(magenum==3?4:magenum);
		}
		if ((in = map[candleD].it) && !it[in].active)
		{
			it[in].data[1] = ch[cn].data[1]*ch[cn].data[1]*(magenum==3?4:magenum)*2+ch[n].data[26]*(magenum==3?4:magenum);
		}
		if ((in = map[candleE].it) && !it[in].active)
		{
			it[in].data[1] = ch[cn].data[1]*ch[cn].data[1]*(magenum==3?4:magenum)*2+ch[n].data[26]*(magenum==3?4:magenum);
		}
		
		ch[cn].data[1]++;
		ch[cn].data[2] = 1;
	}
	
	// Increment ticker storage
	ch[cn].data[3] = globs->ticker + m;
}

void tell_incolosseum(int anum, char *msg)
{
	int n;
	
	for (n = 1; n<MAXCHARS; n++)
	{
		if (ch[n].used==USE_EMPTY) continue;
		if (anum<9 && is_incolosseum(n, anum)) // || (IS_SANEPLAYER(player[n].spectating) && is_incolosseum(player[n].spectating, anum)))
		{
			do_char_log(n, 3, "Announcer: \"%s\"\n", msg);
		}
		else if (anum==9 && is_atpandium(n)) // || (IS_SANEPLAYER(player[n].spectating) && is_incolosseum(player[n].spectating, anum)))
		{
			do_char_log(n, 3, "Pandium: \"%s\"\n", msg);
		}
	}
}

void show_colosseum_rewards(int anum, int diffi, int wave)
{
	int n;
	
	for (n = 1; n<MAXCHARS; n++) 
	{
		if (ch[n].used==USE_EMPTY) continue;
		if (IS_PLAYER(n) && is_incolosseum(n, anum))
		{
			if (wave<7) do_char_log(n, 1, "* Your rewards so far:\n");
			else 		do_char_log(n, 1, "* Final rewards:\n");
				
			switch (diffi)
			{
				case  1:	// Standard
					if (wave==1 || wave==2) 
						do_char_log(n, 1, "    125 Gold\n");
					if (wave==3 || wave==4) 
						do_char_log(n, 1, "    375 Gold\n");
					if (wave>=5) 
						do_char_log(n, 1, "    750 Gold\n");
					if (wave>=2) 
						do_char_log(n, 1, "    1x Ratling Eye Essence\n");
					if (wave>=4) 
						do_char_log(n, 1, "    1x Greenling Eye Essence\n");
					if (wave>=6) 
						do_char_log(n, 6, "    1x Attribute scroll (random)\n");
					if (wave==7) 
					{
						do_char_log(n, 1, "    1x RARE Titanium weapon\n");
						do_char_log(n, 9, "    1x +1 Augmentation to any equipable item\n");
					}
					break;
				case  2:	// Cruel
					if (wave==1 || wave==2) 
						do_char_log(n, 1, "    250 Gold\n");
					if (wave==3 || wave==4) 
						do_char_log(n, 1, "    750 Gold\n");
					if (wave>=5) 
						do_char_log(n, 1, "    1500 Gold\n");
					if (wave==2 || wave==3) 
						do_char_log(n, 1, "    1x Greenling Eye Essence\n");
					if (wave>=4) 
						do_char_log(n, 1, "    2x Greenling Eye Essences\n");
					if (wave>=6) 
						do_char_log(n, 6, "    1x Greater skill scroll (random)\n");
					if (wave==7) 
					{
						do_char_log(n, 1, "    1x RARE Damascus weapon\n");
						do_char_log(n, 9, "    2x +1 Augmentation to any equipable item\n");
					}
					break;
				case  3:	// Merciless
					if (wave==1 || wave==2) 
						do_char_log(n, 1, "    500 Gold\n");
					if (wave==3 || wave==4) 
						do_char_log(n, 1, "    1500 Gold\n");
					if (wave>=5) 
						do_char_log(n, 1, "    3000 Gold\n");
					if (wave>=2) 
						do_char_log(n, 1, "    1x Greenling Eye Essence\n");
					if (wave>=4) 
						do_char_log(n, 1, "    1x Sogling Eye Essences\n");
					if (wave>=6) 
						do_char_log(n, 6, "    1x Greater attribute scroll (random)\n");
					if (wave==7) 
					{
						do_char_log(n, 1, "    1x RARE Adamant weapon\n");
						do_char_log(n, 9, "    1x +2 Augmentation to any equipable item\n");
					}
					break;
				default:	// Absurd
					if (wave==1 || wave==2) 
						do_char_log(n, 1, "    1000 Gold\n");
					if (wave==3 || wave==4) 
						do_char_log(n, 1, "    3000 Gold\n");
					if (wave>=5) 
						do_char_log(n, 1, "    6000 Gold\n");
					if (wave==2 || wave==3) 
						do_char_log(n, 1, "    1x Sogling Eye Essence\n");
					if (wave>=4) 
						do_char_log(n, 1, "    2x Sogling Eye Essences\n");
					if (wave==6) 
						do_char_log(n, 6, "    1x Greater skill scroll (random)\n");
					if (wave==7) 
					{
						do_char_log(n, 6, "    2x Greater skill scrolls (random)\n");
						do_char_log(n, 9, "    2x +2 Augmentations to any equipable item\n");
					}
					break;
			}
			do_char_log(n, 1, " \n");
		}
	}
}

// [tier][wave][4 monster templates & power value]
static int col_waves[4][7][5] = {
	{		// Standard
		{		// Wave 1
			11, 11, 11, 11, 70		// Brig
		},{		// Wave 2
			18, 19, 18, 19, 72
		},{		// Wave 3
			15, 40, 15, 40, 74
		},{		// Wave 4
			22, 22, 22, 22, 76
		},{		// Wave 5
			23, 43, 23, 43, 78
		},{		// Wave 6
			35, 35, 42, 35, 80
		},{		// Wave 7
			17, 32, 41, 45, 84		// General
		}
	},{		// Cruel
		{		// Wave 1
			11, 11, 13, 32, 84		// General
		},{		// Wave 2
			14, 15, 16, 17, 87
		},{		// Wave 3
			12, 22, 22, 36, 90
		},{		// Wave 4
			30, 21, 30, 21, 93
		},{		// Wave 5
			27, 34, 31, 27, 96
		},{		// Wave 6
			39, 38, 39, 38, 99
		},{		// Wave 7
			41, 40, 33, 45, 105		// Baron
		}
	},{		// Merciless
		{		// Wave 1
			11, 17, 11, 17, 105		// Baron
		},{		// Wave 2
			28, 28, 28, 28, 108
		},{		// Wave 3
			37, 40, 37, 40, 111
		},{		// Wave 4
			44, 45, 44, 45, 114
		},{		// Wave 5
			20, 39, 19, 18, 117
		},{		// Wave 6
			16, 31, 22, 24, 120
		},{		// Wave 7
			25, 26, 25, 29, 126		// Marquess
		}
	},{		// Absurd
		{		// Wave 1
			41, 40, 15, 20, 124		// Marquess
		},{		// Wave 2
			21, 29, 26, 30, 128
		},{		// Wave 3
			31, 32, 33, 34, 132
		},{		// Wave 4
			35, 36, 37, 38, 136
		},{		// Wave 5
			39, 42, 12, 52, 140
		},{		// Wave 6
			11, 47, 17, 51, 144
		},{		// Wave 7
			49, 46, 48, 50, 152		// Warlord+++
		}
	}
};

void spawn_colosseum_rewards(int x, int y, int diffi, int wave)
{
	int in;
	
	switch (diffi)
	{
		case  1: 	// Standard
			if (wave>0) { if (in = build_item(CR_GOLD, x,   y+3)) it[in].data[0] =  125; }
			if (wave>1) { if (in = build_item(CR_ITEM, x+1, y+3)) it[in].data[0] = IT_RPOT; }
			if (wave>2) { if (in = build_item(CR_GOLD, x-1, y+3)) it[in].data[0] =  250; }
			if (wave>3) { if (in = build_item(CR_ITEM, x+2, y+3)) it[in].data[0] = IT_GPOT; }
			if (wave>4) { if (in = build_item(CR_GOLD, x-2, y+3)) it[in].data[0] =  375; }
			if (wave>5) { if (in = build_item(CR_ITEM, x+3, y+3)) it[in].data[0] = IT_RD_BRV+RANDOM(5); }
			if (wave>6) { if (in = build_item(CR_ITEM, x-3, y+3)) it[in].data[0] = IT_DAGG_TITN; }
			if (wave>6) { if (in = build_item(CR_AUGM, x-4, y+1)) it[in].data[0] = 1; it[in].data[1] = 1; }
			break;
		case  2: 	// Cruel
			if (wave>0) { if (in = build_item(CR_GOLD, x,   y+3)) it[in].data[0] =  250; }
			if (wave>1) { if (in = build_item(CR_ITEM, x+1, y+3)) it[in].data[0] = IT_GPOT; }
			if (wave>2) { if (in = build_item(CR_GOLD, x-1, y+3)) it[in].data[0] =  500; }
			if (wave>3) { if (in = build_item(CR_ITEM, x+2, y+3)) it[in].data[0] = IT_GPOT; }
			if (wave>4) { if (in = build_item(CR_GOLD, x-2, y+3)) it[in].data[0] =  750; }
			if (wave>5) { if (in = build_item(CR_ITEM, x+3, y+3)) it[in].data[0] = IT_OS_SK; }
			if (wave>6) { if (in = build_item(CR_ITEM, x-3, y+3)) it[in].data[0] = IT_DAGG_DAMA; }
			if (wave>6) { if (in = build_item(CR_AUGM, x-4, y+1)) it[in].data[0] = 1; it[in].data[1] = 2; }
			break;
		case  3: 	// Merciless
			if (wave>0) { if (in = build_item(CR_GOLD, x,   y+3)) it[in].data[0] =  500; }
			if (wave>1) { if (in = build_item(CR_ITEM, x+1, y+3)) it[in].data[0] = IT_GPOT; }
			if (wave>2) { if (in = build_item(CR_GOLD, x-1, y+3)) it[in].data[0] = 1000; }
			if (wave>3) { if (in = build_item(CR_ITEM, x+2, y+3)) it[in].data[0] = IT_SPOT; }
			if (wave>4) { if (in = build_item(CR_GOLD, x-2, y+3)) it[in].data[0] = 1500; }
			if (wave>5) { if (in = build_item(CR_ITEM, x+3, y+3)) it[in].data[0] = IT_OS_BRV+RANDOM(5); }
			if (wave>6) { if (in = build_item(CR_ITEM, x-3, y+3)) it[in].data[0] = IT_DAGG_ADAM; }
			if (wave>6) { if (in = build_item(CR_AUGM, x-4, y+1)) it[in].data[0] = 2; it[in].data[1] = 1; }
			break;
		default: 	// Absurd
			if (wave>0) { if (in = build_item(CR_GOLD, x,   y+3)) it[in].data[0] = 1000; }
			if (wave>1) { if (in = build_item(CR_ITEM, x+1, y+3)) it[in].data[0] = IT_SPOT; }
			if (wave>2) { if (in = build_item(CR_GOLD, x-1, y+3)) it[in].data[0] = 2000; }
			if (wave>3) { if (in = build_item(CR_ITEM, x+2, y+3)) it[in].data[0] = IT_SPOT; }
			if (wave>4) { if (in = build_item(CR_GOLD, x-2, y+3)) it[in].data[0] = 3000; }
			if (wave>5) { if (in = build_item(CR_ITEM, x+3, y+3)) it[in].data[0] = IT_OS_SK; }
			if (wave>6) { if (in = build_item(CR_ITEM, x-3, y+3)) it[in].data[0] = IT_OS_SK; }
			if (wave>6) { if (in = build_item(CR_AUGM, x-4, y+1)) it[in].data[0] = 2; it[in].data[1] = 2; }
			break;
	}
}

void spawn_colosseum_enemy(int cn, int x, int y, int tox, int toy, int parent, int diffi, int wave, int m)
{
	int co, try;
	
	if (co = generate_map_enemy(cn, 347, col_waves[diffi][wave][m], x, y, col_waves[diffi][wave][4], 0, 3))
	{
		ch[co].flags |= CF_NOSLEEP;
		ch[co].kindred |= KIN_MONSTER;
		ch[co].data[31] = ch[parent].temp;    // parent template
		ch[co].data[CHD_GROUP] = ch[co].data[43] = ch[co].data[59] = 60;
		ch[co].data[25] = 0;
//		ch[co].data[27] = 1;
		ch[co].data[29] = tox+toy*MAPX;
		ch[co].skill[SK_PERCEPT][1] = 30;
		npc_moveto(co, tox, toy);
		do_update_char(co);
		ch[parent].data[5+m] = co;
	}
}

// For the Colosseum NPC(s) to track player progress
void colosseum_driver(int cn)
{
	int n, m=0, i=0, p=0, x, y, anum=0, co=0, try;
	static int from_xy[5][5][2] = {
		{ {735,933},{751,949},{767,933},{751,917} },
		{ {702,908},{718,924},{734,908},{718,892} },
		{ {768,908},{784,924},{800,908},{784,892} },
		{ {702,958},{718,974},{734,958},{718,942} },
		{ {768,958},{784,974},{800,958},{784,942} } };
		//    N         E         S         W         X
	static int to_xy[5][5][2] = {
		{ {747,933},{751,937},{755,933},{751,929},{742,947} },
		{ {714,908},{718,912},{722,908},{718,904},{709,922} },
		{ {780,908},{784,912},{788,908},{784,904},{775,922} },
		{ {714,958},{718,962},{722,958},{718,954},{709,972} },
		{ {780,958},{784,962},{788,958},{784,954},{775,972} } };
	
	/*
		Announcer driver
		
		data[0] = whether or not we're active / number of players
		data[1] = Wave process number
		data[2] = Chat process number
		data[3] = global time for next event process
		data[4] = remaining monsters
		
		generate_map_enemy(cn, 347, bs_waves[magenum-1][ch[cn].data[1]][ch[cn].data[2]][m], spawnX, spawnY, (magenum*15+(magenum==3?15:0))+ch[cn].data[1]*magenum, 0, 0);
	*/
	
	// Get announcer number for the local arena
	if (ch[cn].temp==CT_ANNOU1) anum = 1;
	if (ch[cn].temp==CT_ANNOU2) anum = 2;
	if (ch[cn].temp==CT_ANNOU3) anum = 3;
	if (ch[cn].temp==CT_ANNOU4) anum = 4;
	if (ch[cn].temp==CT_ANNOU5) anum = 5;
	
	// Check for players in the relevant arena
	for (n = 1; n<MAXCHARS; n++) 
	{
		if (!IS_SANECHAR(n) || ch[n].used==USE_EMPTY) continue;
		if (IS_PLAYER(n) && is_incolosseum(n, anum) && p == 0) p = ch[n].colosseum;
		if (!IS_PLAYER(n) && !IS_PLAYER_COMP(n) && 
			is_incolosseum(n, anum) && ch[n].data[CHD_GROUP]==60) m++;
	}
	ch[cn].data[0] = p;
	ch[cn].data[4] = m;
	
	// Initialize the announcer
	if (p<1 || ch[cn].data[1]==9)
	{
		if (p<1) 
		{
			m = ch[cn].data[29];
			x = m % MAPX;
			y = m / MAPX;
			if (ch[cn].x != x) god_transfer_char(cn, x, y); // Teleport back home if necessary
			ch[cn].data[1] = 9;
		}
		else
		{
			ch[cn].data[1] = 0;
		}
		ch[cn].data[2] = 0;
		ch[cn].data[3] = globs->ticker + TICKS * 3;
		ch[cn].data[4] = 0;
		ch[cn].data[5] = 0;
		ch[cn].data[6] = 0;
		ch[cn].data[7] = 0;
		ch[cn].data[8] = 0;
		ch[cn].data[9] = 0;
		return; // No further process when nobody is in the area
	}
	
	// Return if time is less than next action
	if (globs->ticker < ch[cn].data[3])
		return;
	
	// Process by wave number
	if (ch[cn].data[1]==0) // Fresh start
	{
		switch (ch[cn].data[2])
		{
			case  0: ch[cn].data[2]++; ch[cn].data[3] = globs->ticker + TICKS * 5; chlog(cn, "beginning fresh colosseum (%d)", anum);
				tell_incolosseum(anum, "Ladies and gentlemen! It's the moment you've all been waiting for! Today, our glorious colosseum has a new challenger!"); break;
			case  1: ch[cn].data[2]++; ch[cn].data[3] = globs->ticker + TICKS * 5;
				tell_incolosseum(anum, "Can they make it through all seven rounds and claim the grand prize? Or will they resign part-way? Only time will tell!"); break;
			case  2: ch[cn].data[1]=1; ch[cn].data[2]=0; ch[cn].data[3] = globs->ticker + TICKS / 4;
				tell_incolosseum(anum, "Let's make some noise! Round 1 starts NOW!!"); break;
			default: break;
		}
	}
	else if (ch[cn].data[1]>=1 && ch[cn].data[1]<=7) // Waves
	{
		switch (ch[cn].data[2])
		{
			case  0: 	// Spawn mobs
				n = anum-1;
				i = ch[cn].data[0]-1;
				p = ch[cn].data[1]-1;
				m = 0; spawn_colosseum_enemy(cn, from_xy[n][m][0], from_xy[n][m][1], to_xy[n][m][0], to_xy[n][m][1], cn, i, p, m);
				m = 1; spawn_colosseum_enemy(cn, from_xy[n][m][0], from_xy[n][m][1], to_xy[n][m][0], to_xy[n][m][1], cn, i, p, m);
				m = 2; spawn_colosseum_enemy(cn, from_xy[n][m][0], from_xy[n][m][1], to_xy[n][m][0], to_xy[n][m][1], cn, i, p, m);
				m = 3; spawn_colosseum_enemy(cn, from_xy[n][m][0], from_xy[n][m][1], to_xy[n][m][0], to_xy[n][m][1], cn, i, p, m);
				chlog(cn, "created 4 monsters in colosseum %d", anum);
				ch[cn].data[9] = globs->ticker + TICKS*20;
				ch[cn].data[2]++;
				break;
			case  1: 	// Wait for mobs to die
				if (ch[cn].data[4]) 
				{
					if (globs->ticker < ch[cn].data[9])
						return;
					n = anum-1;
					for (m=0;m<4;m++)
					{
						// Try to force NPCs out of spawns if they're not doing anything at the moment.
						if (IS_SANECHAR(co = ch[cn].data[5+m]))
						{
							if (ch[co].used != USE_EMPTY && ch[co].temp == 347 && !is_reallyincolosseum(co, anum) && ch[co].data[9]>=TICKS*5)
							{
								quick_teleport(co, to_xy[n][m][0]-2+RANDOM(5), to_xy[n][m][1]-2+RANDOM(5));
								ch[co].data[9] = 0;
								ch[cn].data[9] = globs->ticker + TICKS*5;
							}
							else
							{
								ch[co].data[9]++;
							}
						}
					}
					return;
				}
				ch[cn].data[2]++; ch[cn].data[3] = globs->ticker + TICKS / 2; break;
			case  2: 	// Warp after clear
				x = to_xy[anum-1][0][0];
				y = to_xy[anum-1][0][1];
				if (ch[cn].data[1] < 7) quick_teleport(cn, x, y); 
				ch[cn].data[2]++; ch[cn].data[3] = globs->ticker + TICKS * 1; break;
			case  3:   // Continue check
				ch[cn].data[3] = globs->ticker + TICKS * 1;
				if (ch[cn].data[1] == 7)
				{
					ch[cn].data[2] = 7;
					tell_incolosseum(anum, "Round 7 clear! Congratulations!");
					show_colosseum_rewards(anum, ch[cn].data[0], ch[cn].data[1]);
				}
				else
				{
					ch[cn].data[2]++;
					tell_incolosseum(anum, "Round clear! Well done!");
					show_colosseum_rewards(anum, ch[cn].data[0], ch[cn].data[1]);
					tell_incolosseum(anum, "Shall we CONTINUE? Or would you like to RESIGN?");
				}
				break;
			case  4: // Wait for reply
				break;
			case  5: // Reply: Continue
				m = ch[cn].data[29];
				x = m % MAPX;
				y = m / MAPX;
				quick_teleport(cn, x, y); 
				ch[cn].data[2]++; ch[cn].data[3] = globs->ticker + TICKS * 2; break;
			case  6: // Continue
				ch[cn].data[1]++; ch[cn].data[2]=0; ch[cn].data[3] = globs->ticker + TICKS * 1;
				switch (ch[cn].data[1])
				{
					case  2: tell_incolosseum(anum, "We haven't seen anything yet! Round 2 starts NOW!!"); break;
					case  3: tell_incolosseum(anum, "They haven't even broken a sweat! Round 3 starts NOW!!"); break;
					case  4: tell_incolosseum(anum, "Can they keep it up? Round 4 starts NOW!!"); break;
					case  5: tell_incolosseum(anum, "The crowd cheers loud and wild! Round 5 starts NOW!!"); break;
					case  6: tell_incolosseum(anum, "Our challenger feels luck on their side! Round 6 starts NOW!!"); break;
					default: tell_incolosseum(anum, "It's finally time for the main event! The final round starts NOW!!"); break;
				}
				break;
			case  7: // Reply: Quit
				m = ch[cn].data[29];
				x = m % MAPX;
				y = m / MAPX;
				quick_teleport(cn, x, y); 
				spawn_colosseum_rewards(to_xy[anum-1][4][0], to_xy[anum-1][4][1], ch[cn].data[0], ch[cn].data[1]);
				ch[cn].data[2]++; ch[cn].data[3] = globs->ticker + TICKS * 2; break;
			case  8: // Quit / Victory
				ch[cn].data[2]++;
				tell_incolosseum(anum, "One last round of applause for our challenger!");
				x = to_xy[anum-1][4][0];
				y = to_xy[anum-1][4][1];
				for (n = 1; n<MAXCHARS; n++) 
				{
					if (ch[n].used==USE_EMPTY) continue;
					if (IS_PLAYER(n) && is_incolosseum(n, anum)) quick_teleport(n, x, y);
					/*
					if (IS_PLAYER(n) && IS_SANEPLAYER(player[n].spectating) && is_incolosseum(player[n].spectating, anum) && 
						!(ch[player[n].spectating].flags & CF_ALW_SPECT))
					{
						player[n].spectating = 0;
						do_char_log(n, 0, "No longer spectating.\n");
					}
					*/
				}
				break;
			default: break;
		}
	}
}

#define PANDI_LIMIT 	3

void spawn_pandium_rewards(int cn, int fl, int x, int y)
{
	int in, m;
	
	m = IT_POT_BRV+RANDOM(5);
	if (m==1982) m = 135;
	
	// Clear any existing chests
	if ((in = map[x + y * MAPX].it)!=0) { it[in].used = 0; map[x + y * MAPX].it = 0; }
	//
	if 		((fl >= 50 && fl%5==0) && (in = build_item(CR_ITEM, x, y))) { it[in].power=53; it[in].data[0] = 2921; } // Obsidian Crown
	//
	else if (fl >=  1 && ch[cn].pandium_floor[2] < 1 && (in = build_item(CR_BUFF, x, y))) it[in].data[0] = 1; // .
	else if (fl >= 10 && ch[cn].pandium_floor[2] < 2 && (in = build_item(CR_BUFF, x, y))) it[in].data[0] = 2; //  .
	else if (fl >= 20 && ch[cn].pandium_floor[2] < 3 && (in = build_item(CR_BUFF, x, y))) it[in].data[0] = 3; // Permanent buff shrines
	else if (fl >= 30 && ch[cn].pandium_floor[2] < 4 && (in = build_item(CR_BUFF, x, y))) it[in].data[0] = 4; //  .
	else if (fl >= 40 && ch[cn].pandium_floor[2] < 5 && (in = build_item(CR_BUFF, x, y))) it[in].data[0] = 5; // .
	//
	else if (fl == 45 && (in = build_item(CR_ITEM, x, y))) it[in].data[0] = 2924; // Exp Scroll 750,000
	else if (fl == 15 && (in = build_item(CR_ITEM, x, y))) it[in].data[0] = 2923; // Exp Scroll 250,000
	else if (fl ==  0 && (in = build_item(CR_ITEM, x, y))) it[in].data[0] = 0;    // You get nothing. You lose. Good day, sir.
	//
	else if (fl%19==0 && (in = build_item(CR_ITEM, x, y))) it[in].data[0] = IT_SPOT;                 // Spot
	else if (fl%17==0 && (in = build_item(CR_ITEM, x, y))) it[in].data[0] = 2307;                    // Heavensplitter
	else if (fl%13==0 && (in = build_item(CR_ITEM, x, y))) it[in].data[0] = IT_POP_SSPEL+RANDOM(7);  // Skua scroll
	else if (fl%11==0 && (in = build_item(CR_ITEM, x, y))) it[in].data[0] = 2962;                    // Luck scroll
	else if (fl% 7==0 && (in = build_item(CR_ITEM, x, y))) it[in].data[0] = IT_GPOT;                 // Gpot
	else if (fl% 5==0 && (in = build_item(CR_ITEM, x, y))) it[in].data[0] = IT_OS_BRV+RANDOM(5);     // Greater attribute scroll
	else if (fl% 3==0 && (in = build_item(CR_ITEM, x, y))) it[in].data[0] = m;                       // Greater attribute potion
	else if (fl% 2==0 && (in = build_item(CR_ITEM, x, y))) it[in].data[0] = IT_POT_D_HP+RANDOM(3)*6; // Divine potion
	else if (in = build_item(CR_ITEM, x, y))               it[in].data[0] = IT_OS_SK;                // Greater skill scroll
	
	xlog("Spawned Pandium Rewards for %s with floor value of %d", ch[cn].name, fl);
}

void make_explode(int x, int y, int dmg)
{
	int in=0;
	
	// it==0 assures there's no existing bomb first.
	if (map[x + y * MAPX].it==0 && (in = build_item(IT_EXPLOSION, x, y)))
	{ 
		it[in].data[0] = dmg; 
		do_add_light(x, y, it[in].light[1]);
	}
}

void pandium_clean(void)
{
	int x, y, in=0;
	
	for (x = PANDI_X1-1; x<=PANDI_X2+1; x++) for (y = PANDI_Y1-1; y<=PANDI_Y2+1; y++)
	{
		if (in = map[x + y * MAPX].it)
		{
			do_add_light(x, y, -it[in].light[1]);
			it[in].used = 0;
			map[x + y * MAPX].it = 0;
		}
	}
}

void pandium_pattern(int fl)
{
	int x, y, frx, fry, tox, toy, mdx, mdy;
	int m, n, off, dmg, rng=0;
	
	frx = PANDI_X1; 
	tox = PANDI_X2; 
	fry = PANDI_Y1; 
	toy = PANDI_Y2;
	mdx = PANDI_MIDX;
	mdy = PANDI_MIDY;
	
	dmg = 400+40*fl;
	
	// Do a funny pattern on a prime numbered depth
	switch (fl)
	{
		/*
			+ - - - - - - - - - - - - - - - - - +	1
			|                 X                 |
			|                 X                 |
			|                 X                 |
			|                 X                 |
			|                 X                 |
			|                 X                 |
			|                 X                 |
			|                 X                 |
			| X X X X X X X X # X X X X X X X X |
			|                 X                 |
			|                 X                 |
			|                 X                 |
			|                 X                 |
			|                 X                 |
			|                 X                 |
			|                 X                 |
			|                 X                 |
			+ - - - - - - - - - - - - - - - - - +	score = 33
		*/
		case  7:
			for (x = frx; x<=tox; x++) for (y = fry; y<=toy; y++)
			{
				m = x-frx; n = y-fry;
				if (m!=8 && n!=8) continue;
				make_explode(x, y, dmg);
			}
			break;
		/*
			+ - - - - - - - - - - - - - - - - - +	2
			| X                               X |
			|   X                           X   |
			|     X                       X     |
			|       X                   X       |
			|         X               X         |
			|           X           X           |
			|             X       X             |
			|               X   X               |
			|                 #                 |
			|               X   X               |
			|             X       X             |
			|           X           X           |
			|         X               X         |
			|       X                   X       |
			|     X                       X     |
			|   X                           X   |
			| X                               X |
			+ - - - - - - - - - - - - - - - - - +	score = 33
		*/
		case  9:
			for (x = frx; x<=tox; x++) for (y = fry; y<=toy; y++)
			{
				m = x-frx; n = y-fry;
				if (m!=n && 16-m != n) continue;
				make_explode(x, y, dmg);
			}
			break;
		/*
			+ - - - - - - - - - - - - - - - - - +	3
			|                                   |
			|                                   |
			|                                   |
			|       X X X X X X X X X X X       |
			|       X                   X       |
			|       X                   X       |
			|       X     X X X X X     X       |
			|       X     X       X     X       |
			|       X     X   #   X     X       |
			|       X     X       X     X       |
			|       X     X X X X X     X       |
			|       X                   X       |
			|       X                   X       |
			|       X X X X X X X X X X X       |
			|                                   |
			|                                   |
			|                                   |
			+ - - - - - - - - - - - - - - - - - +	score = 57
		*/
		case 11:
			for (x = frx; x<=tox; x++) for (y = fry; y<=toy; y++)
			{
				m = x-frx; n = y-fry;
				if ((!((m)>=3 && (n)>=3 && (m)<=13 && (n)<=13)) || (((m)>=4 && (n)>=4 && (m)<=12 && (n)<=12) &&
					 !((m)>=6 && (n)>=6 && (m)<=10 && (n)<=10)) || (((m)>=7 && (n)>=7 && (m)<= 9 && (n)<= 9) &&
					 !((m)>=5 && (n)>=5 && (m)<= 5 && (n)<= 5)) && !(m==8 && n==8)) continue;
				make_explode(x, y, dmg);
			}
			break;
		/*
			+ - - - - - - - - - - - - - - - - - +	4
			| X               X               X |
			|   X           X   X           X   |
			|     X       X       X       X     |
			|       X   X           X   X       |
			|         X               X         |
			|       X   X           X   X       |
			|     X       X       X       X     |
			|   X           X   X           X   |
			| X               #               X |
			|   X           X   X           X   |
			|     X       X       X       X     |
			|       X   X           X   X       |
			|         X               X         |
			|       X   X           X   X       |
			|     X       X       X       X     |
			|   X           X   X           X   |
			| X               X               X |
			+ - - - - - - - - - - - - - - - - - +	score = 61
		*/
		case 13:
			for (x = frx; x<=tox; x++) for (y = fry; y<=toy; y++)
			{
				m = x-frx; n = y-fry;
				if (m%8!=n && m!=n%8 && m%8!=n%8 && 16-m%8!=n && 16-m!=n%8 && 8-m!=n%8) continue;
				make_explode(x, y, dmg);
			}
			break;
		/*
			+ - - - - - - - - - - - - - - - - - +	5
			|                                   |
			|                                   |
			|     X X X     X X X     X X X     |
			|     X X X     X X X     X X X     |
			|     X X X     X X X     X X X     |
			|                                   |
			|                                   |
			|     X X X     X X X     X X X     |
			|     X X X     X # X     X X X     |
			|     X X X     X X X     X X X     |
			|                                   |
			|                                   |
			|     X X X     X X X     X X X     |
			|     X X X     X X X     X X X     |
			|     X X X     X X X     X X X     |
			|                                   |
			|                                   |
			+ - - - - - - - - - - - - - - - - - +	score = 81
		*/
		case 15:
			for (x = frx; x<=tox; x++) for (y = fry; y<=toy; y++)
			{
				m = x-frx; n = y-fry;
				if ((m-1)%5==0 || (n-1)%5==0 || (m)%5==0 || (n)%5==0) continue;
				make_explode(x, y, dmg);
			}
			break;
		/*
			+ - - - - - - - - - - - - - - - - - +	6
			|               X X X               |
			|               X X X               |
			|               X X X               |
			|               X X X               |
			|               X X X               |
			|               X X X               |
			|               X X X               |
			| X X X X X X X X X X X X X X X X X |
			| X X X X X X X X # X X X X X X X X |
			| X X X X X X X X X X X X X X X X X |
			|               X X X               |
			|               X X X               |
			|               X X X               |
			|               X X X               |
			|               X X X               |
			|               X X X               |
			|               X X X               |
			+ - - - - - - - - - - - - - - - - - +	score = 93
		*/
		case 17:
			for (x = frx; x<=tox; x++) for (y = fry; y<=toy; y++)
			{
				m = x-frx; n = y-fry;
				if (m!=7 && n!=7 && m!=8 && n!=8 && m!=9 && n!=9) continue;
				make_explode(x, y, dmg);
			}
			break;
		/*
			+ - - - - - - - - - - - - - - - - - +	7
			|     X       X       X       X     |
			|     X       X       X       X     |
			| X X X X X X X X X X X X X X X X X |
			|     X       X       X       X     |
			|     X       X       X       X     |
			|     X       X       X       X     |
			| X X X X X X X X X X X X X X X X X |
			|     X       X       X       X     |
			|     X       X   @   X       X     |
			|     X       X       X       X     |
			| X X X X X X X X X X X X X X X X X |
			|     X       X       X       X     |
			|     X       X       X       X     |
			|     X       X       X       X     |
			| X X X X X X X X X X X X X X X X X |
			|     X       X       X       X     |
			|     X       X       X       X     |
			+ - - - - - - - - - - - - - - - - - +	score = 120
		*/
		case 19:
			for (x = frx; x<=tox; x++) for (y = fry; y<=toy; y++)
			{
				m = x-frx; n = y-fry;
				if ((m+2)%4!=0 && (n+2)%4!=0) continue;
				make_explode(x, y, dmg);
			}
			break;
		/*
			+ - - - - - - - - - - - - - - - - - +	8
			|                                   |
			|                                   |
			|                                   |
			|       X X X   X X X   X X X       |
			|       X X X   X X X   X X X       |
			|       X X X   X X X   X X X       |
			|                                   |
			|       X X X   X X X   X X X       |
			|       X X X   X # X   X X X       |
			|       X X X   X X X   X X X       |
			|                                   |
			|       X X X   X X X   X X X       |
			|       X X X   X X X   X X X       |
			|       X X X   X X X   X X X       |
			|                                   |
			|                                   |
			|                                   |
			+ - - - - - - - - - - - - - - - - - +	score = 81
		*/
		case 21:
			for (x = frx; x<=tox; x++) for (y = fry; y<=toy; y++)
			{
				m = x-frx; n = y-fry;
				if (m%4==2 || m<=1 || m>=15 || n%4==2 || n<=1 || n>=15) continue;
				make_explode(x, y, dmg);
			}
			break;
		/*
			+ - - - - - - - - - - - - - - - - - +	9
			| X X                           X X |
			| X X X                       X X X |
			|   X X X                   X X X   |
			|     X X X               X X X     |
			|       X X X           X X X       |
			|         X X X       X X X         |
			|           X X X   X X X           |
			|             X X X X X             |
			|               X # X               |
			|             X X X X X             |
			|           X X X   X X X           |
			|         X X X       X X X         |
			|       X X X           X X X       |
			|     X X X               X X X     |
			|   X X X                   X X X   |
			| X X X                       X X X |
			| X X                           X X |
			+ - - - - - - - - - - - - - - - - - +	score = 93
		*/
		case 23:
			for (x = frx; x<=tox; x++) for (y = fry; y<=toy; y++)
			{
				m = x-frx; n = y-fry;
				if (m  !=n && 16-m   != n && 
					m+1!=n && 16-m+1 != n && 
					m-1!=n && 16-m-1 != n ) continue;
				make_explode(x, y, dmg);
			}
			break;
		/*
			+ - - - - - - - - - - - - - - - - - +	10
			| X X X X X X X X X X X X X X X X X |
			| X       X       X       X       X |
			| X       X       X       X       X |
			| X       X       X       X       X |
			| X X X X X X X X X X X X X X X X X |
			| X       X       X       X       X |
			| X       X       X       X       X |
			| X       X       X       X       X |
			| X X X X X X X X # X X X X X X X X |
			| X       X       X       X       X |
			| X       X       X       X       X |
			| X       X       X       X       X |
			| X X X X X X X X X X X X X X X X X |
			| X       X       X       X       X |
			| X       X       X       X       X |
			| X       X       X       X       X |
			| X X X X X X X X X X X X X X X X X |
			+ - - - - - - - - - - - - - - - - - +	score = 145
		*/
		case 25:
			for (x = frx; x<=tox; x++) for (y = fry; y<=toy; y++)
			{
				m = x-frx; n = y-fry;
				if ((m)%4!=0 && (n)%4!=0) continue;
				make_explode(x, y, dmg);
			}
			break;
		/*
			+ - - - - - - - - - - - - - - - - - +	11
			| X   X   X   X   X   X   X   X   X |
			|                                   |
			| X   X   X   X   X   X   X   X   X |
			|                                   |
			| X   X   X   X   X   X   X   X   X |
			|                                   |
			| X   X   X   X   X   X   X   X   X |
			|                                   |
			| X   X   X   X   #   X   X   X   X |
			|                                   |
			| X   X   X   X   X   X   X   X   X |
			|                                   |
			| X   X   X   X   X   X   X   X   X |
			|                                   |
			| X   X   X   X   X   X   X   X   X |
			|                                   |
			| X   X   X   X   X   X   X   X   X |
			+ - - - - - - - - - - - - - - - - - +	score = 81
		*/
		case 27:
			for (x = frx; x<=tox; x++) for (y = fry; y<=toy; y++)
			{
				m = x-frx; n = y-fry;
				if (m%2==1 || n%2==1) continue;
				make_explode(x, y, dmg);
			}
			break;
		/*
			+ - - - - - - - - - - - - - - - - - +	12
			|     X X X X X       X X X X X     |
			|       X X X           X X X       |
			| X       X       X       X       X |
			| X X           X X X           X X |
			| X X X       X X X X X       X X X |
			| X X           X X X           X X |
			| X       X       X       X       X |
			|       X X X           X X X       |
			|     X X X X X   @   X X X X X     |
			|       X X X           X X X       |
			| X       X       X       X       X |
			| X X           X X X           X X |
			| X X X       X X X X X       X X X |
			| X X           X X X           X X |
			| X       X       X       X       X |
			|       X X X           X X X       |
			|     X X X X X       X X X X X     |
			+ - - - - - - - - - - - - - - - - - +	score = 124
		*/
		case 29:
			for (x = frx; x<=tox; x++) for (y = fry; y<=toy; y++)
			{
				m = x-frx; n = y-fry;
				if (!((m  )!=   n && (m  )!=   n+1 && (m  )!=   n-1 &&
					  (m+8)!=   n && (m+8)!=   n+1 && (m+8)!=   n-1 &&
					  (m-8)!=   n && (m-8)!=   n+1 && (m-8)!=   n-1 &&
					  (m  )!=16-n && (m  )!=16-n+1 && (m  )!=16-n-1 &&
					  (m+8)!=16-n && (m+8)!=16-n+1 && (m+8)!=16-n-1 &&
					  (m-8)!=16-n && (m-8)!=16-n+1 && (m-8)!=16-n-1 )) continue;
				make_explode(x, y, dmg);
			}
			break;
		/*
			+ - - - - - - - - - - - - - - - - - +	13
			|                                   |
			|   X   X   X   X   X   X   X   X   |
			|     X       X       X       X     |
			|   X   X   X   X   X   X   X   X   |
			|         X               X         |
			|   X   X   X   X   X   X   X   X   |
			|     X       X       X       X     |
			|   X   X   X   X   X   X   X   X   |
			|                 @                 |
			|   X   X   X   X   X   X   X   X   |
			|     X       X       X       X     |
			|   X   X   X   X   X   X   X   X   |
			|         X               X         |
			|   X   X   X   X   X   X   X   X   |
			|     X       X       X       X     |
			|   X   X   X   X   X   X   X   X   |
			|                                   |
			+ - - - - - - - - - - - - - - - - - +	score = 84
		*/
		case 31:
			for (x = frx; x<=tox; x++) for (y = fry; y<=toy; y++)
			{
				m = x-frx; n = y-fry;
				if (!(((m+2)%4==0 && (n+2)%4==0) || 
					  ((m+1)%2==0 && (n+1)%2==0) || 
					  ((m+4)%8==0 && (n+4)%8==0))) continue;
				make_explode(x, y, dmg);
			}
			break;
		/*
			+ - - - - - - - - - - - - - - - - - +	14
			| X X   X X   X X   X X   X X   X X |
			| X X   X X   X X   X X   X X   X X |
			|                                   |
			| X X   X X   X X   X X   X X   X X |
			| X X   X X   X X   X X   X X   X X |
			|                                   |
			| X X   X X   X X   X X   X X   X X |
			| X X   X X   X X   X X   X X   X X |
			|                 @                 |
			| X X   X X   X X   X X   X X   X X |
			| X X   X X   X X   X X   X X   X X |
			|                                   |
			| X X   X X   X X   X X   X X   X X |
			| X X   X X   X X   X X   X X   X X |
			|                                   |
			| X X   X X   X X   X X   X X   X X |
			| X X   X X   X X   X X   X X   X X |
			+ - - - - - - - - - - - - - - - - - +	score = 144
		*/
		case 33:
			for (x = frx; x<=tox; x++) for (y = fry; y<=toy; y++)
			{
				m = x-frx; n = y-fry;
				if (m%3==2 || n%3==2) continue;
				make_explode(x, y, dmg);
			}
			break;
		/*
			+ - - - - - - - - - - - - - - - - - +	15
			| X X X       X X X X X       X X X |
			| X X X X       X X X       X X X X |
			| X X X X X       X       X X X X X |
			|   X X X X X           X X X X X   |
			|     X X X X X       X X X X X     |
			|       X X X           X X X       |
			| X       X       X       X       X |
			| X X           X X X           X X |
			| X X X       X X # X X       X X X |
			| X X           X X X           X X |
			| X       X       X       X       X |
			|       X X X           X X X       |
			|     X X X X X       X X X X X     |
			|   X X X X X           X X X X X   |
			| X X X X X       X       X X X X X |
			| X X X X       X X X       X X X X |
			| X X X       X X X X X       X X X |
			+ - - - - - - - - - - - - - - - - - +	score = 153
		*/
		case 35:
			for (x = frx; x<=tox; x++) for (y = fry; y<=toy; y++)
			{
				m = x-frx; n = y-fry;
				if (!((m-4)!=   n && (m-4)!=   n+1 && (m-4)!=   n-1 &&
					  (m+4)!=   n && (m+4)!=   n+1 && (m+4)!=   n-1 &&
					  (m-4)!=16-n && (m-4)!=16-n+1 && (m-4)!=16-n-1 &&
					  (m+4)!=16-n && (m+4)!=16-n+1 && (m+4)!=16-n-1 )) continue;
				make_explode(x, y, dmg);
			}
			break;
			
		/*
			+ - - - - - - - - - - - - - - - - - +	16
			| X X X X X X X X X X X X X X X X X |
			|   X X X X X X X X X X X X X X X   |
			|     X X X X X X X X X X X X X     |
			|       X X X X X X X X X X X       |
			|         X X X X X X X X X         |
			|           X X X X X X X           |
			|             X X X X X             |
			|               X X X               |
			|                 #                 |
			|               X X X               |
			|             X X X X X             |
			|           X X X X X X X           |
			|         X X X X X X X X X         |
			|       X X X X X X X X X X X       |
			|     X X X X X X X X X X X X X     |
			|   X X X X X X X X X X X X X X X   |
			| X X X X X X X X X X X X X X X X X |
			+ - - - - - - - - - - - - - - - - - +	score = 161
		*/
		case 37:
			rng = RANDOM(2);
			for (x = frx; x<=tox; x++) for (y = fry; y<=toy; y++)
			{
				m = x-frx; n = y-fry;
				if (rng==0 && (((m)>(n) && (16-n)>(m)) || ((m)<(n) && (16-n)<(m)))) continue;
				if (rng==1 && (((m)<(n) && (16-m)>(n)) || ((m)>(n) && (16-m)<(n)))) continue;
				make_explode(x, y, dmg);
			}
			break;
		/*
			+ - - - - - - - - - - - - - - - - - +	17
			|       X X X           X X X       |
			|         X X X       X X X         |
			|           X X X   X X X           |
			| X           X X X X X           X |
			| X X           X X X           X X |
			| X X X       X X X X X       X X X |
			|   X X X   X X X   X X X   X X X   |
			|     X X X X X       X X X X X     |
			|       X X X     @     X X X       |
			|     X X X X X       X X X X X     |
			|   X X X   X X X   X X X   X X X   |
			| X X X       X X X X X       X X X |
			| X X           X X X           X X |
			| X           X X X X X           X |
			|           X X X   X X X           |
			|         X X X       X X X         |
			|       X X X           X X X       |
			+ - - - - - - - - - - - - - - - - - +	score = 136
		*/
		case 39:
			for (x = frx; x<=tox; x++) for (y = fry; y<=toy; y++)
			{
				m = x-frx; n = y-fry;
				if ((m-4)!=   n && (m-4)!=   n+1 && (m-4)!=   n-1 &&
					(m+4)!=   n && (m+4)!=   n+1 && (m+4)!=   n-1 &&
					(m-4)!=16-n && (m-4)!=16-n+1 && (m-4)!=16-n-1 &&
					(m+4)!=16-n && (m+4)!=16-n+1 && (m+4)!=16-n-1 ) continue;
				make_explode(x, y, dmg);
			}
			break;
		/*
			+ - - - - - - - - - - - - - - - - - +	18
			| X X           X X X           X X |
			| X X X       X X X X X       X X X |
			|   X X X   X X X   X X X   X X X   |
			|     X X X X X       X X X X X     |
			|       X X X           X X X       |
			|     X X X X X       X X X X X     |
			|   X X X   X X X   X X X   X X X   |
			| X X X       X X X X X       X X X |
			| X X           X # X           X X |
			| X X X       X X X X X       X X X |
			|   X X X   X X X   X X X   X X X   |
			|     X X X X X       X X X X X     |
			|       X X X           X X X       |
			|     X X X X X       X X X X X     |
			|   X X X   X X X   X X X   X X X   |
			| X X X       X X X X X       X X X |
			| X X           X X X           X X |
			+ - - - - - - - - - - - - - - - - - +	score = 165
		*/
		case 41:
			for (x = frx; x<=tox; x++) for (y = fry; y<=toy; y++)
			{
				m = x-frx; n = y-fry;
				if ((m  )!=(   n) && (m  )!=   n+1 && (m  )!=   n-1 &&
					(m+8)!=(   n) && (m+8)!=   n+1 && (m+8)!=   n-1 &&
					(m-8)!=(   n) && (m-8)!=   n+1 && (m-8)!=   n-1 &&
					(m  )!=(16-n) && (m  )!=16-n+1 && (m  )!=16-n-1 &&
					(m+8)!=(16-n) && (m+8)!=16-n+1 && (m+8)!=16-n-1 &&
					(m-8)!=(16-n) && (m-8)!=16-n+1 && (m-8)!=16-n-1 ) continue;
				make_explode(x, y, dmg);
			}
			break;
		/*
			+ - - - - - - - - - - - - - - - - - +	19
			| X X X X X X X       X X X X X     |
			| X X X X X X       X X X X X       |
			| X X X X X       X X X X X       X |
			| X X X X       X X X X X       X X |
			| X X X       X X X X X       X X X |
			| X X       X X X X X       X X X X |
			| X       X X X X X       X X X X X |
			|       X X X X X       X X X X X   |
			|     X X X X X   @   X X X X X     |
			|   X X X X X       X X X X X       |
			| X X X X X       X X X X X       X |
			| X X X X       X X X X X       X X |
			| X X X       X X X X X       X X X |
			| X X       X X X X X       X X X X |
			| X       X X X X X       X X X X X |
			|       X X X X X       X X X X X X |
			|     X X X X X       X X X X X X X |
			+ - - - - - - - - - - - - - - - - - +	score = 186
		*/
		case 43:
			rng = RANDOM(2);
			for (x = frx; x<=tox; x++) for (y = fry; y<=toy; y++)
			{
				m = x-frx; n = y-fry;
				if (rng==0 && !((m  )!=16-n && (m  )!=16-n+1 && (m  )!=16-n-1 &&
								(m+8)!=16-n && (m+8)!=16-n+1 && (m+8)!=16-n-1 &&
								(m-8)!=16-n && (m-8)!=16-n+1 && (m-8)!=16-n-1 )) continue;
				if (rng==1 && !((m  )!=   n && (m  )!=   n+1 && (m  )!=   n-1 &&
								(m+8)!=   n && (m+8)!=   n+1 && (m+8)!=   n-1 &&
								(m-8)!=   n && (m-8)!=   n+1 && (m-8)!=   n-1 )) continue;
				make_explode(x, y, dmg);
			}
			break;
		/*
			+ - - - - - - - - - - - - - - - - - +	20
			|                                   |
			|   X X X X X X X X X X X X X X X   |
			|   X X X X X X X X X X X X X X X   |
			|   X X                       X X   |
			|   X X   X X X X X X X X X   X X   |
			|   X X   X X X X X X X X X   X X   |
			|   X X   X X           X X   X X   |
			|   X X   X X   X X X   X X   X X   |
			|   X X   X X   X # X   X X   X X   |
			|   X X   X X   X X X   X X   X X   |
			|   X X   X X           X X   X X   |
			|   X X   X X X X X X X X X   X X   |
			|   X X   X X X X X X X X X   X X   |
			|   X X                       X X   |
			|   X X X X X X X X X X X X X X X   |
			|   X X X X X X X X X X X X X X X   |
			|                                   |
			+ - - - - - - - - - - - - - - - - - +	score = 169
		*/
		case 45:
			for (x = frx; x<=tox; x++) for (y = fry; y<=toy; y++)
			{
				m = x-frx; n = y-fry;
				if (!((((m)>=1 && (n)>=1 && (m)<=15 && (n)<=15) && !((m)>=3 && (n)>=3 && (m)<=13 && (n)<=13)) ||
					  (((m)>=4 && (n)>=4 && (m)<=12 && (n)<=12) && !((m)>=6 && (n)>=6 && (m)<=10 && (n)<=10)) ||
					  (((m)>=7 && (n)>=7 && (m)<= 9 && (n)<= 9) && !((m)>=5 && (n)>=5 && (m)<= 5 && (n)<= 5)) )) continue;
				make_explode(x, y, dmg);
			}
			break;
		/*
			+ - - - - - - - - - - - - - - - - - +	21
			| X X X X X   X X X X X   X X X X X |
			| X X X X X   X X X X X   X X X X X |
			| X X X X X   X X X X X   X X X X X |
			| X X X X X   X X X X X   X X X X X |
			| X X X X X   X X X X X   X X X X X |
			|                                   |
			| X X X X X   X X X X X   X X X X X |
			| X X X X X   X X X X X   X X X X X |
			| X X X X X   X X # X X   X X X X X |
			| X X X X X   X X X X X   X X X X X |
			| X X X X X   X X X X X   X X X X X |
			|                                   |
			| X X X X X   X X X X X   X X X X X |
			| X X X X X   X X X X X   X X X X X |
			| X X X X X   X X X X X   X X X X X |
			| X X X X X   X X X X X   X X X X X |
			| X X X X X   X X X X X   X X X X X |
			+ - - - - - - - - - - - - - - - - - +	score = 225
		*/
		case 47:
			for (x = frx; x<=tox; x++) for (y = fry; y<=toy; y++)
			{
				m = x-frx; n = y-fry;
				if ((m+1)%6==0 || (n+1)%6==0) continue;
				make_explode(x, y, dmg);
			}
			break;
		/*
			+ - - - - - - - - - - - - - - - - - +	22
			| X X X X X X X X X X X X X X X X X |
			| X   X   X   X   X   X   X   X   X |
			| X X   X X X   X X X   X X X   X X |
			| X   X   X   X   X   X   X   X   X |
			| X X X X   X X X X X X X   X X X X |
			| X   X   X   X   X   X   X   X   X |
			| X X   X X X   X X X   X X X   X X |
			| X   X   X   X   X   X   X   X   X |
			| X X X X X X X X # X X X X X X X X |
			| X   X   X   X   X   X   X   X   X |
			| X X   X X X   X X X   X X X   X X |
			| X   X   X   X   X   X   X   X   X |
			| X X X X   X X X X X X X   X X X X |
			| X   X   X   X   X   X   X   X   X |
			| X X   X X X   X X X   X X X   X X |
			| X   X   X   X   X   X   X   X   X |
			| X X X X X X X X X X X X X X X X X |
			+ - - - - - - - - - - - - - - - - - +	score = 205
		*/
		case 49:
			for (x = frx; x<=tox; x++) for (y = fry; y<=toy; y++)
			{
				m = x-frx; n = y-fry;
				if (((m+2)%4==0 && (n+2)%4==0) || 
					((m+1)%2==0 && (n+1)%2==0) || 
					((m+4)%8==0 && (n+4)%8==0) ) continue;
				make_explode(x, y, dmg);
			}
			break;
			
		// Non-special cases :: Just make a square in the middle
		default:
			// Variable setup for inner explosion box
			off = 1;
			if (fl>=10) off = 2; // 5x5
			if (fl>=20) off = 3; // 7x7
			if (fl>=30) off = 4; // 9x9
			if (fl>=40) off = 1; // 3x3*
			if (fl>=50) off = 2; // 5x5*
			// Variable setup for x/y offset variance
			m = 1;
			if (fl>=15) m = 3;
			if (fl==20) m = 1;
			if (fl>=25) m = 5;
			if (fl==30) m = 1;
			if (fl>=35) m = 3;
			if (fl==40) m = 1;
			if (fl>=45) m = 5;
			if (fl==50) m = 1;
			if (fl>=55) m = 3;
			n = RANDOM(m) - (m-1)/2;
			m = RANDOM(m) - (m-1)/2;
			// Create inner explosion box
			for (x = mdx+n-off; x<mdx+n+1+off; x++) for (y = mdy+m-off; y<mdy+m+1+off; y++)
			{
				make_explode(x, y, dmg);
			}
			// Variable setup for outer explosion box
			off = 0;
			if (fl>=40) off = 4; // 9x9
			if (fl>=50) off = 3; // 7x7
			// Create outer explosion box
			if (off) for (x = frx; x<=tox; x++) for (y = fry; y<=toy; y++)
			{
				if (x>=mdx-off && x<=mdx+off && y>=mdy-off && y<=mdy+off) continue; // Excludes middle
				make_explode(x, y, dmg);
			}
			break;
	}
}

void clean_and_go(int cn, int x, int y, int z, int hpm)
{
	ch[cn].data[29] = x + y * MAPX;
	ch[cn].dir      = z;
	ch[cn].data[30] = z;
	ch[cn].a_hp = ch[cn].hp[5]*hpm;
	remove_buff(cn, SK_ZEPHYR2);
	remove_buff(cn, SK_TAUNT);
	ch[cn].taunted = 0;
	npc_remove_all_enemies(cn);
	quick_teleport(cn, x, y);
	chlog(cn, "clean_and_go %d, %d", x, y);
}

void pandium_driver(int cn) // CT_PANDIUM
{
	int n, m, j, p=0, fl=999, x, y, frx, fry, tox, toy, in=0, co=0, dir=0, try=0;
	int old_p, old_fl, old_try;
	//                   Players     Pandium     Shadow 1    Shadow 2    Shadow 3
	int go_xy[5][3] = { {283,989,4},{283,995,3},{283,983,4},{277,989,1},{289,989,2} };
	//					             Pandium     Shadow 1    Shadow 2    Shadow 3
	static int to_xy[4][3] = {      {283,995,3},{283,983,4},{277,989,1},{289,989,2} };
	int temp[5] = {0};
	int ch_in[PANDI_LIMIT] = {0};
	
	/*
		Pandium Fight Driver
		
		data[0] = Player/counter
		data[1] = Floor ID - set by lowest member if grouped
		data[2] = Phase
		data[3] = Phase timer
		data[4] = auto-blast timer
		data[5] = targeted auto-blast timer
	*/
	
	old_p   = ch[cn].data[0];
	old_fl  = ch[cn].data[1];
	old_try = ch[cn].data[32];
	
	// Check for players in the relevant arena
	for (n = 1; n<MAXCHARS; n++) 
	{
		if (!IS_SANECHAR(n) || ch[n].used==USE_EMPTY) continue;
		if (IS_PLAYER(n) && is_atpandium(n)) try++;
	}
	for (n = 1; n<MAXCHARS; n++) 
	{
		if (!IS_SANECHAR(n) || ch[n].used==USE_EMPTY) continue;
		if (IS_PLAYER(n) && is_atpandium(n))
		{
			if (p+1>PANDI_LIMIT) // Too many people! Get outta here!
			{
				quick_teleport(n, 512, 512);
				continue;
			}
			ch_in[p] = n;
			p++;
			if (try==1) fl = ch[n].pandium_floor[0];
			else        fl = min(fl, ch[n].pandium_floor[1]); // whoever's floor is smallest
		}
	}
	if (old_p   ==   0) ch[cn].data[0] = p;
	else if (p  !=   0) p  = old_p;
	else 				ch[cn].data[0] = 0;
	
	if (old_fl  ==   0 || old_fl  == 999) 	ch[cn].data[1] = fl;
	else if (fl !=   0 && fl != 999) 		fl = old_fl;
	else 									ch[cn].data[1] = 0;
	
	if (old_try  ==  0) ch[cn].data[32] = try;
	else if (try !=  0) try = old_try;
	else 				ch[cn].data[32] = 0;
	
	// Pandium's endurance and mana never runs out
	ch[cn].a_end  = 999999;
	ch[cn].a_mana = 888888;
	
	frx = PANDI_X1; 
	tox = PANDI_X2; 
	fry = PANDI_Y1; 
	toy = PANDI_Y2;
	
	// Initialize
	if (p<1 || ch[cn].data[2]==99)
	{
		if (p<1) 
		{
			for (x = frx; x<tox; x++) for (y = fry; y<toy; y++)
			{
				// Clear any lingering shadows
				if ((co = map[x + y * MAPX].ch) && !IS_PLAYER(co) && ch[co].temp != CT_PANDIUM)
				{
					god_destroy_items(co);
					if (ch[co].used==USE_ACTIVE) plr_map_remove(co);
					ch[co].flags = 0;
					ch[co].used = USE_EMPTY;
				}
				// Clear any lingering graves/etc
				if ((in = map[x + y * MAPX].it)!=0 && (it[in].flags & IF_USE))
				{
					it[in].used = 0;
					map[x + y * MAPX].it = 0;
				}
			}
			ch[cn].a_hp   = 9999999;
			if (ch[cn].data[2] != 99)
			{
				remove_buff(cn, SK_ZEPHYR2);
				remove_buff(cn, SK_TAUNT);
				remove_buff(cn, SK_POISON);
				remove_buff(cn, SK_BLEED);
				ch[cn].taunted = 0;
				npc_remove_all_enemies(cn);
				for (n=0;n<4;n++)
				{
					to_xy[n][0] = go_xy[n+1][0];
					to_xy[n][1] = go_xy[n+1][1];
					to_xy[n][2] = go_xy[n+1][2];
				}
				x = to_xy[0][0];
				y = to_xy[0][1];
				ch[cn].dir = to_xy[0][2];
				ch[cn].data[29] = x + y * MAPX;
				ch[cn].data[30] = 3;
				god_transfer_char(cn, x, y);
			}
			ch[cn].data[2] = 99;
		}
		else
		{
			ch[cn].data[2] = 0;
			chlog(cn, "Initialized");
			for (n = 1; n<MAXCHARS; n++) 
			{
				if (!IS_SANECHAR(n) || ch[n].used==USE_EMPTY) continue;
				if (IS_PLAYER(n) && is_atpandium(n))
				{
					do_char_log(n, 8, "  == The Archon's Trial, Depth %2d ==\n", fl);
				}
			}
		}
		ch[cn].data[3] = globs->ticker + TICKS / 2;
		ch[cn].data[4] = 0;
		ch[cn].data[5] = 0;
		ch[cn].data[6] = 0;
		ch[cn].data[7] = 0;
		ch[cn].data[8] = 0;
		ch[cn].data[9] = 0;
		return; // No further process when nobody is in the area
	}
	
	if (p && ch[cn].data[2] == 0) // Phase 0
	{
		// Assure Pandium is buffed (SK_OPPRESSION) based on floor #
		// +0.5% damage reduction and +2 to all attributes each 1 power.
		ch[cn].weapon_bonus          = min(127, fl/5 +  1);
		ch[cn].armor_bonus           = min(127, fl/5 + 12);
		ch[cn].skill[SK_PERCEPT][1]  = 90;
		ch[cn].skill[SK_SAFEGRD][1]  =  0;
		ch[cn].skill[SK_IMMUN][1]    = min(125, fl/2);
		ch[cn].skill[SK_AXE][0]      = ch[cn].skill[SK_TWOHAND][0]  = 120 - min(100, fl);
		ch[cn].hp[0]                 = 1000+1000*p;
		ch[cn].a_hp                  = 9999999;
		if (has_buff(cn, SK_OPPRESSION)) remove_buff(cn, SK_OPPRESSION);
		in = god_create_buff();
		strcpy(bu[in].name, "Oppression");
		strcpy(bu[in].reference, "oppression");
		strcpy(bu[in].description, "Oppression.");
		for (m=0;m<5;m++) 
		{
			ch[cn].attrib[m][1] = min(127, fl*5/4);
			bu[in].attrib[m][1] = min(127, fl*5/4);
		}
		bu[in].power = fl;
		bu[in].dmg_reduction[1] = min(127, fl);
		bu[in].active = bu[in].duration = 1;
		bu[in].flags  = IF_SPELL | IF_PERMSPELL;
		bu[in].temp = SK_OPPRESSION;
		bu[in].sprite[1] = min(6780, 6761+(fl/5)-1);
		add_spell(cn, in);
		// Grant Pandium new skills based on floor #
		ch[cn].skill[SK_CURSE][0]    = 0;
		ch[cn].skill[SK_WEAKEN][0]   = 0;
		ch[cn].skill[SK_SLOW][0]     = 0;
		ch[cn].skill[SK_BLIND][0]    = 0;
		ch[cn].skill[SK_LETHARGY][0] = 0;
		ch[cn].skill[SK_LEAP][0]     = 0;
		if (fl>= 4) { ch[cn].skill[SK_CURSE][0]    = min(150, 90+fl/3); }
		if (fl>= 8) { ch[cn].skill[SK_WEAKEN][0]   = min(150, 80+fl/3); }
		if (fl>=12) { ch[cn].skill[SK_CLEAVE][0]   = min(150, 90+fl/5); }
		if (fl>=16) { ch[cn].skill[SK_SLOW][0]     = min(150, 85+fl/3); }
		if (fl>=20) { ch[cn].skill[SK_BLIND][0]    = min(150, 75+fl/5); }
		if (fl>=24) { ch[cn].skill[SK_LETHARGY][0] = min(150, 30+fl/4); }
		if (fl>=28) { ch[cn].skill[SK_LEAP][0]     = min(150, 90+fl/4); }
		if (fl>=32)		{ temp[0]=2916; temp[1]=2917; temp[2]=2918; temp[3]=2919; temp[4]=2920; } // Obsidian
		else if (fl>=16){ temp[0]=  94; temp[1]=  95; temp[2]=  96; temp[3]=  97; temp[4]=  98; } // Adamant
		else 			{ temp[0]=  76; temp[1]=  77; temp[2]=  78; temp[3]=  79; temp[4]=  80; } // Titanium
		if (fl>=50) { temp[0]=2921; } // Crown
		in = ch[cn].worn[WN_HEAD]  = pop_create_item(temp[0], cn); it[in].carried = cn;
		in = ch[cn].worn[WN_CLOAK] = pop_create_item(temp[1], cn); it[in].carried = cn;
		in = ch[cn].worn[WN_BODY]  = pop_create_item(temp[2], cn); it[in].carried = cn;
		in = ch[cn].worn[WN_ARMS]  = pop_create_item(temp[3], cn); it[in].carried = cn;
		in = ch[cn].worn[WN_FEET]  = pop_create_item(temp[4], cn); it[in].carried = cn;
		do_update_char(cn);
		// Assure players in the area are debuffed (SK_OPPRESSED) based on floor #
		// -2 to all skills and attributes, and -1 to all caps per 1 power (1 pow per 5 floors, starting floor 5).
		if (fl/5>0) for (n = 1; n<MAXCHARS; n++) 
		{
			if (!IS_SANECHAR(n) || ch[n].used==USE_EMPTY) continue;
			if (IS_PLAYER(n) && is_atpandium(n))
			{
				do_char_log(n, 0, "You feel something press down on you.\n", fl);
				if (has_buff(n, SK_OPPRESSED)) remove_buff(n, SK_OPPRESSED);
				in = god_create_buff();
				strcpy(bu[in].name, "Oppressed");
				strcpy(bu[in].reference, "oppressed");
				strcpy(bu[in].description, "oppressed.");
				bu[in].power = fl/5;
				for (m = 0; m<5; m++) bu[in].attrib[m][1] = -(min(127, fl/5)*2);
				for (m = 0; m<50; m++) bu[in].skill[m][1] = -(min(127, fl/5)*2);
				bu[in].active = bu[in].duration = 1;
				bu[in].flags  = IF_SPELL | IF_PERMSPELL;
				bu[in].temp = SK_OPPRESSED;
				bu[in].sprite[1] = min(6780, 6761+(fl/5)-1);
				add_spell(n, in);
				do_update_char(n);
			}
			// If we're not in there, wipe off any SK_OPPRESSED
			// Placed here so it's not checked all the time
			if (!is_atpandium(n) && has_buff(n, SK_OPPRESSED)) 
			{
				remove_buff(n, SK_OPPRESSED);
				do_update_char(n);
			}
		}
		ch[cn].data[2] = 1;
		ch[cn].data[3] = globs->ticker + TICKS;
		ch[cn].data[4] = 0;
		ch[cn].data[5] = 0;
		ch[cn].data[6] = 0;
		ch[cn].data[7] = 0;
		ch[cn].data[8] = 0;
		ch[cn].data[9] = 0;
		chlog(cn, "beginning depth %d fight vs %d challengers", fl, p);
	}
	
	// Auto-blast timer loop - Starts on depth 10
	if (ch[cn].data[4] && globs->ticker >= ch[cn].data[4])
	{
		for (try=0;try<3;try++)
		{
			x = frx + RANDOM(tox-frx+1);
			y = fry + RANDOM(toy-fry+1);
			if (map[x + y * MAPX].it==0 && (in = build_item(IT_EXPLOSION, x, y)))
			{
				do_area_sound(0, 0, x, y, 22);
				it[in].data[0] = 200+20*fl;
				do_add_light(x, y, it[in].light[1]);
				break;
			}
		}
		ch[cn].data[4] = globs->ticker + max(1, 1+TICKS - (TICKS*fl/50));
	}
	
	// targeted auto-blast timer loop - Starts on depth 15
	if (ch[cn].data[5] && globs->ticker >= ch[cn].data[5])
	{
		for (n=0;n<PANDI_LIMIT;n++)
		{
			if (ch_base_status(ch[ch_in[n]].status) >= 8) continue;
			try = 0; if (fl>=25) try = 1; // 3x3
			m   = 1; if (fl>=40) m   = 3; // offset
			j = RANDOM(m) - (m-1)/2;
			m = RANDOM(m) - (m-1)/2;
			for (x = ch[ch_in[n]].x+j-try; x<ch[ch_in[n]].x+j+1+try; x++) for (y = ch[ch_in[n]].y+m-try; y<ch[ch_in[n]].y+m+1+try; y++)
			{	// Check for clean tile & spawn explosion
				if (map[x + y * MAPX].it==0 && (in = build_item(IT_EXPLOSION, x, y))) 
				{	
					if (x==ch[ch_in[n]].x && y==ch[ch_in[n]].y) do_area_sound(0, 0, ch[ch_in[n]].x, ch[ch_in[n]].y, 22);
					it[in].data[0] = 200+40*fl;
					do_add_light(x, y, it[in].light[1]);
				}
			}
		}
		ch[cn].data[5] = globs->ticker + max(TICKS, TICKS * 5 - (TICKS*3*fl/50));
	}
	
	// Return if time is less than next action
	if (globs->ticker < ch[cn].data[3])
		return;
	
	switch (ch[cn].data[2])
	{
		case 1: 
			if (fl>=52) 		tell_incolosseum(9, "Fight.");
			else if (fl==51) 	tell_incolosseum(9, "I lied.");
			else if (fl==50) 	tell_incolosseum(9, "Time and again you've stood before me. A king among mortal men.");
			else if (fl>=34) 	tell_incolosseum(9, "You bore me waiting once again.");
			else if (fl==33) 	tell_incolosseum(9, "Aemon, my brother... Will you one day find him? Face him? Fight him too?");
			else if (fl>=10) 	tell_incolosseum(9, "You return once more. And clash we shall.");
			else if (fl>= 5) 	tell_incolosseum(9, "Together, we grow strong. We pillar above men.");
			else if (fl>= 2) 	tell_incolosseum(9, "Welcome back. Here to test your limits again?");
			else  				tell_incolosseum(9, "Fwahaha, a new face in my domain. Welcome, challenger!");
			ch[cn].data[2]++; 
			ch[cn].data[3] = globs->ticker + max(TICKS, TICKS * 3 - (TICKS*3*fl/50));
			break;
		case 2: 
			if (fl>=52) 		;
			else if (fl==51) 	tell_incolosseum(9, "May we clash and drive eachother's strength, forevermore.");
			else if (fl==50) 	tell_incolosseum(9, "This shall be my greatest test of you. Come! Claim your crown.");
			else if (fl>=34) 	tell_incolosseum(9, "Let us dance together, ever more.");
			else if (fl==33) 	tell_incolosseum(9, "You've stood my tests alone. But some day, together, we will clash for the final time. And on that day, Skua shall weep.");
			else if (fl>=10) 	tell_incolosseum(9, "Know this... Only one of us may stand.");
			else if (fl>= 5) 	tell_incolosseum(9, "Shall we climb even higher? Come.");
			else if (fl>= 2) 	tell_incolosseum(9, "Come, then. Let us shake the mountain.");
			else  				tell_incolosseum(9, "Break yourself upon me so we may grow strong!");
			for (n=0;n<PANDI_LIMIT;n++) npc_add_enemy(cn, ch_in[n], 1);
			if (fl<10) 	ch[cn].data[2]=6; // Skip ahead a phase
			else 		ch[cn].data[2]++; 
			ch[cn].data[3] = globs->ticker + TICKS / 2;
			break;
		// 75% HP Breakpoint
		case 3: 
			if (ch[cn].a_hp>ch[cn].hp[5]*750) return; // Wait until 75%
			if (fl>=20)
			{
				m = RANDOM(4)+1;
				to_xy[0][0] = go_xy[m][0];
				to_xy[0][1] = go_xy[m][1];
				to_xy[0][2] = go_xy[m][2];
			}
			clean_and_go(cn, to_xy[0][0], to_xy[0][1], to_xy[0][2], 750);
			x = go_xy[0][0];
			y = go_xy[0][1];
			for (n=0;n<PANDI_LIMIT;n++) 
			{
				remove_buff(ch_in[n], SK_ZEPHYR2);
				remove_buff(ch_in[n], SK_TAUNT);
				ch[ch_in[n]].taunted = 0;
				remove_enemy(ch_in[n]);
				quick_teleport(ch_in[n], x, y);
				if (IS_SANECHAR(co = ch[ch_in[n]].data[PCD_COMPANION]) && IS_ALIVEMASTER(co, ch_in[n]))
				{
					remove_buff(co, SK_ZEPHYR2);
					remove_buff(co, SK_TAUNT);
					ch[co].taunted = 0;
					npc_remove_all_enemies(co);
					quick_teleport(co, x, y);
				}
				if (IS_SANECHAR(co = ch[ch_in[n]].data[PCD_SHADOWCOPY]) && IS_ALIVEMASTER(co, ch_in[n]))
				{
					remove_buff(co, SK_ZEPHYR2);
					remove_buff(co, SK_TAUNT);
					ch[co].taunted = 0;
					npc_remove_all_enemies(co);
					quick_teleport(co, x, y);
				}
			}
			ch[cn].data[2]++;
			ch[cn].data[3] = globs->ticker + TICKS / 2;
			break;
		case 4: 
			if (fl>=40) 		tell_incolosseum(9, "Fire everlasting.");
			else if (fl>=15) 	tell_incolosseum(9, "I shall tear down the stars and throw them at you.");
			else  				tell_incolosseum(9, "May the heavens weep, and the sky smolder and fall around us!");
			ch[cn].skill[SK_SAFEGRD][1] = 30;
			fx_add_effect(7, 0, ch[cn].x, ch[cn].y, 0);
			do_area_sound(0, 0, go_xy[0][0], go_xy[0][1]+1, 21);
			ch[cn].data[4] = globs->ticker + TICKS * 2;
			if (fl>=15) ch[cn].data[5] = globs->ticker + TICKS * 2-1;
			ch[cn].data[2]++;
			ch[cn].data[3] = globs->ticker + max(TICKS, TICKS * 3 - (TICKS*3*fl/50));
			break;
		case 5: 
			if (fl>=40) 		;
			else if (fl>=15) 	tell_incolosseum(9, "Choke and burn!");
			else  				tell_incolosseum(9, "Smolder and dance!");
			for (n=0;n<PANDI_LIMIT;n++) npc_add_enemy(cn, ch_in[n], 1);
			ch[cn].data[2]++;
			ch[cn].data[3] = globs->ticker + TICKS / 2;
			break;
		// 50% HP Breakpoint
		case 6: 
			if (ch[cn].a_hp>ch[cn].hp[5]*500) return; // Wait until 50%
			if (fl>=20)
			{
				m = RANDOM(4)+1;
				for (n=0;n<4;n++)
				{
					to_xy[n][0] = go_xy[m][0];
					to_xy[n][1] = go_xy[m][1];
					to_xy[n][2] = go_xy[m][2];
					m++;
					if (m>4) m = 1;
				}
			}
			clean_and_go(cn, to_xy[0][0], to_xy[0][1], to_xy[0][2], 500);
			x = go_xy[0][0];
			y = go_xy[0][1];
			for (n=0;n<PANDI_LIMIT;n++)
			{
				remove_buff(ch_in[n], SK_ZEPHYR2);
				remove_buff(ch_in[n], SK_TAUNT);
				ch[ch_in[n]].taunted = 0;
				remove_enemy(ch_in[n]);
				quick_teleport(ch_in[n], x, y);
				if (IS_SANECHAR(co = ch[ch_in[n]].data[PCD_COMPANION]) && IS_ALIVEMASTER(co, ch_in[n]))
				{
					remove_buff(co, SK_ZEPHYR2);
					remove_buff(co, SK_TAUNT);
					ch[co].taunted = 0;
					npc_remove_all_enemies(co);
					quick_teleport(co, x, y);
				}
				if (IS_SANECHAR(co = ch[ch_in[n]].data[PCD_SHADOWCOPY]) && IS_ALIVEMASTER(co, ch_in[n]))
				{
					remove_buff(co, SK_ZEPHYR2);
					remove_buff(co, SK_TAUNT);
					ch[co].taunted = 0;
					npc_remove_all_enemies(co);
					quick_teleport(co, x, y);
				}
			}
			ch[cn].data[2]++;
			ch[cn].data[3] = globs->ticker + TICKS / 2;
			break;
		case 7:
			if (fl>= 5) tell_incolosseum(9, "May venom and shadows claim you!");
			else  		tell_incolosseum(9, "Hahaha! Good! I shall hold nothing back!");
			ch[cn].skill[SK_SAFEGRD][1] = 60;
			ch[cn].data[2]++;
			ch[cn].data[3] = globs->ticker + TICKS * 1;
			break;
		case 8:
			// Chug potion at half-way point
			if (fl>=50) n = IT_GPOT;
			else		n = IT_RPOT;
			in = god_create_item(n);
			god_give_char(in, cn);
			npc_quaff_potion(cn, n, 254);
			// Learn skills
			try = 0;
			if (fl>= 5) { ch[cn].skill[SK_POISON][0] =  90; ch[cn].skill[SK_HASTE][0] = 60; try=1; }
			if (fl>=10) { ch[cn].skill[SK_POISON][0] =  96; ch[cn].skill[SK_HASTE][0] = 68; try=2; }
			if (fl>=15) { ch[cn].skill[SK_POISON][0] = 105; ch[cn].skill[SK_HASTE][0] = 75; try=3; }
			if (fl>=20) { ch[cn].skill[SK_POISON][0] = 112; ch[cn].skill[SK_HASTE][0] = 82; try=3; }
			if (fl>=30) { ch[cn].skill[SK_POISON][0] = 120; ch[cn].skill[SK_HASTE][0] = 90; try=3; }
			do_update_char(cn);
			// Spawn shadows
			for (n=0;n<try;n++)
			{
				co = 1432;
				     if (try==1) { x = to_xy[1  ][0]; y = to_xy[1  ][1]; dir = to_xy[1  ][2]; }
				else if (try==2) { x = to_xy[2+n][0]; y = to_xy[2+n][1]; dir = to_xy[2+n][2]; }
				else 			 { x = to_xy[1+n][0]; y = to_xy[1+n][1]; dir = to_xy[1+n][2]; }
				co = pop_create_char(co, 0);
				if (co) 
				{
					ch[co].data[PCD_COMPANION] = globs->ticker + TICKS * 60 * 20;
					ch[co].kindred |= KIN_SHADOW;
					ch[co].flags |= CF_NOSLEEP;
					ch[co].hp[0] = 500 + fl*10;
					ch[co].a_hp  = 9999999;
					ch[co].skill[SK_PERCEPT][1]  = 90;
					ch[co].skill[SK_SAFEGRD][1]  = min(127, fl);
					ch[co].skill[SK_IMMUN][1]    = min(127, fl/2);
				}
				else 
				{ 
					do_sayx(cn, "driver_pandium create char failed (%d)", ch[cn].data[2]); 
					continue; 
				}
				if (!god_drop_char_fuzzy(co, x, y)) 
				{ 
					god_destroy_items(co); 
					ch[co].used = USE_EMPTY; 
					continue; 
				}
				fx_add_effect(12, 0, ch[co].x, ch[co].y, 0);
				ch[co].dir = dir;
				for (m=0;m<PANDI_LIMIT;m++) ch[co].data[m] = ch_in[n];
				ch[cn].data[7+n] = co;
			}
			ch[cn].data[2]++;
			ch[cn].data[3] = globs->ticker + max(TICKS, TICKS * 2 - (TICKS*2*fl/50));
			break;
		case 9:
			if (fl>= 5) tell_incolosseum(9, "Prepare thyself!");
			else  		tell_incolosseum(9, "Embrace power eternal!");
			for (n=0;n<PANDI_LIMIT;n++) 
			{
				if (IS_SANECHAR(co = ch[cn].data[7+n]) && ch[co].used != USE_EMPTY)
				{
					for (m=0;m<PANDI_LIMIT;m++) npc_add_enemy(co, ch_in[m], 1);
				}
				npc_add_enemy(cn, ch_in[n], 1);
				ch[cn].data[7+n] = ch_in[n]; // Set here for when we die later
			}
			if (fl<5) 	ch[cn].data[2]=14; // Skip final phase and wait for end
			else 		ch[cn].data[2]++; 
			ch[cn].data[3] = globs->ticker + TICKS / 2;
			break;
		// 25% HP Breakpoint
		case 10:
			if (ch[cn].a_hp>ch[cn].hp[5]*250) return; // Wait until 25%
			if (fl>=20)
			{
				m = RANDOM(4)+1;
				to_xy[0][0] = go_xy[m][0];
				to_xy[0][1] = go_xy[m][1];
				to_xy[0][2] = go_xy[m][2];
			}
			clean_and_go(cn, to_xy[0][0], to_xy[0][1], to_xy[0][2], 250);
			x = go_xy[0][0];
			y = go_xy[0][1];
			for (n=0;n<PANDI_LIMIT;n++) 
			{
				remove_buff(ch_in[n], SK_ZEPHYR2);
				remove_buff(ch_in[n], SK_TAUNT);
				ch[ch_in[n]].taunted = 0;
				ch[ch_in[n]].data[74] = globs->ticker + TICKS*6; // prevent warcries
				remove_enemy(ch_in[n]);
				quick_teleport(ch_in[n], x, y);
				if (IS_SANECHAR(co = ch[ch_in[n]].data[PCD_COMPANION]) && IS_ALIVEMASTER(co, ch_in[n]))
				{
					remove_buff(co, SK_ZEPHYR2);
					remove_buff(co, SK_TAUNT);
					ch[co].taunted = 0;
					npc_remove_all_enemies(co);
					quick_teleport(co, x, y);
				}
				if (IS_SANECHAR(co = ch[ch_in[n]].data[PCD_SHADOWCOPY]) && IS_ALIVEMASTER(co, ch_in[n]))
				{
					remove_buff(co, SK_ZEPHYR2);
					remove_buff(co, SK_TAUNT);
					ch[co].taunted = 0;
					npc_remove_all_enemies(co);
					quick_teleport(co, x, y);
				}
			}
			ch[cn].data[2]++;
			ch[cn].data[3] = globs->ticker + TICKS / 2;
			break;
		case 11:
			if (fl== 5) tell_incolosseum(9, "Dodge!");
			else  		tell_incolosseum(9, "Obliteration.");
			if (ch[cn].data[4]) ch[cn].data[4] = globs->ticker + TICKS*5+TICKS/2;
			if (ch[cn].data[5]) ch[cn].data[5] = globs->ticker + TICKS*5+TICKS/2;
			ch[cn].data[74] = globs->ticker + TICKS*5+TICKS/2; // prevent warcries
			pandium_clean();
			ch[cn].skill[SK_SAFEGRD][1] = 90;
			ch[cn].data[2]++;
			ch[cn].data[3] = globs->ticker + TICKS * 1;
			break;
		case 12:
			pandium_pattern(fl);
			do_area_sound(0, 0, go_xy[0][0], go_xy[0][1], 14);
			ch[cn].data[2]++;
			ch[cn].data[3] = globs->ticker + max(TICKS, TICKS * 2 - (TICKS*2*fl/50));
			break;
		case 13:
			if (fl== 5) tell_incolosseum(9, "Let's end this.");
			else  		tell_incolosseum(9, "Ha ha ha!");
			for (n=0;n<PANDI_LIMIT;n++) npc_add_enemy(cn, ch_in[n], 1);
			ch[cn].data[2]++;
			ch[cn].data[3] = globs->ticker + TICKS * 5;
			break;
		// Wait until death
		default: break;
	}
}

void change_the_arena(int v, int frx, int fry, int tox, int toy)
{
	int x, y, a, b, c, d, in;
	
	a = MWL_MARBLE;
	b = MTR_WHITE;
	c = MFL_MARBLE;
	d = 520; // red carpet
	
	switch (v)
	{
		case 1:
			a = MWL_EARTH;
			b = MTR_GROLM;
			c = MFL_JUNGLE;
			d = 531; // purple carpet
			break;
		case 2:
			a = MWL_RED;
			b = MTR_GOLD;
			c = MFL_BRICK;
			d = 5034; // green carpet
			break;
		case 3:
			a = MWL_ICE;
			b = MTR_ICE;
			c = MFL_SNOW;
			d = 531; // purple carpet
			break;
		case 4:
			a = MWL_BLACK;
			b = MTR_BOWL;
			c = MFL_BSSMOOTH;
			break;
		default:
			break;
	}
	
	for (x=frx; x<=tox; x++) for (y=fry; y<=toy; y++)
	{
		// outer wall change
		if (x==frx&&y>=fry&&y<=toy) build_drop(x, y, a);
		if (x>=frx&&x<=tox&&y==fry) build_drop(x, y, a);
		if (x==tox&&y>=fry&&y<=toy) build_drop(x, y, a);
		if (x>=frx&&x<=tox&&y==toy) build_drop(x, y, a);
		// candle change
		if ((in=map[x+y*MAPX].it)!=0 && it[in].driver!=124 && (it[in].flags & IF_LOOK)) { build_remove(x, y); build_drop(x, y, b); }
		// floor change
		build_drop(x, y, 0x20000000 | c);
		if (x> 199&&x<209&&y>1549&&y<1559)	build_drop(x, y, 0x20000000 | d + 0); // carpet
		if (x==199&&y==1549) 				build_drop(x, y, 0x20000000 | d + 3); // carpet corner
		if (x==199&&y>1549&&y<1559) 		build_drop(x, y, 0x20000000 | d + 4); // carpet edge
		if (x==199&&y==1559) 				build_drop(x, y, 0x20000000 | d + 5); // carpet corner
		if (x> 199&&x<209&&y==1559) 		build_drop(x, y, 0x20000000 | d + 6); // carpet edge
		if (x==209&&y==1559) 				build_drop(x, y, 0x20000000 | d + 7); // carpet corner
		if (x==209&&y>1549&&y<1559) 		build_drop(x, y, 0x20000000 | d + 8); // carpet edge
		if (x==209&&y==1549) 				build_drop(x, y, 0x20000000 | d + 9); // carpet corner
		if (x> 199&&x<209&&y==1549) 		build_drop(x, y, 0x20000000 | d +10); // carpet edge
	}
}

void gatekeeper_driver(int cn) // CT_LAB20_KEEP
{
	int n, m, x, y, frx, fry, tox, toy, try, co, in, p=0;
	static int go_xy[8][3] = { {211,1554,2},{208,1554,2},{204,1547,4},{204,1550,4},
							   {197,1554,1},{200,1554,1},{204,1561,3},{204,1558,3} };
	/*
		Uber GK Fight Driver
		
		data[0] = Player
		data[1] = Phase
		data[2] = Phase timer
		data[3] = Cascade timer
		data[4] = Cascade type
		data[5] = Cascade tile offset & ender
		data[6] = data[7] = Shadow trace for deletion
		data[8] = M data for Cascade alignment
	*/
	
	// Check for players
	for (n = 1; n<MAXCHARS; n++)
	{
		if (!IS_SANECHAR(n) || ch[n].used==USE_EMPTY) continue;
		if (IS_PLAYER(n) && IS_IN_TLG(ch[n].x, ch[n].y)) { p = n; break; }
	}
	ch[cn].data[0] = p;
	
	frx = TLG_X1; 
	tox = TLG_X2; 
	fry = TLG_Y1; 
	toy = TLG_Y2;
	
	// Initialize
	if (p<1 || ch[cn].data[1]==99)
	{
		if (p<1) 
		{
			for (x = frx; x<tox; x++) for (y = fry; y<toy; y++)
			{
				// Clear any lingering shadows
				if ((co = map[x + y * MAPX].ch) && !IS_PLAYER(co) && ch[co].temp != CT_LAB20_KEEP)
				{
					god_destroy_items(co);
					if (ch[co].used==USE_ACTIVE) plr_map_remove(co);
					ch[co].flags = 0;
					ch[co].used = USE_EMPTY;
				}
				// Clear any lingering graves/etc
				if ((in = map[x + y * MAPX].it)!=0 && (it[in].flags & IF_USE))
				{
					it[in].used = 0;
					map[x + y * MAPX].it = 0;
				}
			}
			ch[cn].a_hp   = 9999999;
			if (ch[cn].data[1] != 99)
			{
				change_the_arena(0, frx, fry, tox, toy);
				remove_buff(cn, SK_ZEPHYR2);
				remove_buff(cn, SK_TAUNT);
				remove_buff(cn, SK_POISON);
				remove_buff(cn, SK_BLEED);
				ch[cn].taunted = 0;
				npc_remove_all_enemies(cn);
				x = go_xy[7][0];
				y = go_xy[7][1]-1;
				ch[cn].dir = go_xy[7][2];
				ch[cn].data[30] = go_xy[7][2];
				ch[cn].data[29] = x + y * MAPX;
				god_transfer_char(cn, x, y);
			}
			ch[cn].data[1] = 99;
		}
		else
		{
			chlog(cn, "Initialized");
			ch[cn].data[1] = 0;
		}
		ch[cn].data[2] = globs->ticker + TICKS / 2;
		ch[cn].data[3] = 0;
		ch[cn].data[4] = 0;
		ch[cn].data[5] = 0;
		ch[cn].data[6] = 0;
		ch[cn].data[7] = 0;
		ch[cn].data[8] = 0;
		return; // No further process when nobody is in the area
	}
	
	if (p && ch[cn].data[1] == 0) // Phase 0
	{
		ch[cn].skill[SK_ZEPHYR][0]   = 0;
		ch[cn].skill[SK_LETHARGY][0] = 0;
		ch[cn].skill[SK_WARCRY][0]   = 0;
		ch[cn].skill[SK_PULSE][0]    = 0;
		ch[cn].skill[SK_SHADOW][0]   = 0;
		ch[cn].skill[SK_LEAP][0]     = 0;
		//
		in = ch[cn].worn[WN_CHARM]; it[in].x = 0; it[in].y = 0; it[in].carried = 0; ch[cn].worn[WN_CHARM] = 0;
		do_update_char(cn);
		//
		ch[cn].data[1] = 1;
		ch[cn].data[2] = globs->ticker + TICKS;
		ch[cn].data[3] = 0;
		ch[cn].data[4] = 0;
		ch[cn].data[5] = 0;
		ch[cn].data[6] = 0;
		ch[cn].data[7] = 0;
		ch[cn].data[8] = 0;
		chlog(cn, "beginning gk fight");
	}
	
	// Cascading blast timer loop
	if (ch[cn].data[4] && ch[cn].data[3] && globs->ticker >= ch[cn].data[3])
	{
		for (try=0;try<3;try++)
		{
			switch (ch[cn].data[4])
			{
				case  1: x = (TLG_X2-1) - ch[cn].data[5]; y = M2Y(ch[cn].data[8]); break;
				case  2: x = M2X(ch[cn].data[8]); y = (TLG_Y1+1) + ch[cn].data[5]; break;
				case  3: x = (TLG_X1+1) + ch[cn].data[5]; y = M2Y(ch[cn].data[8]); break;
				case  4: x = M2X(ch[cn].data[8]); y = (TLG_Y2-1) - ch[cn].data[5]; break;
				default: break;
			}
			// Single line sweep
			if (IS_IN_TLG(x, y) && map[x + y * MAPX].it==0 && (in = build_item(IT_EXPLOSION, x, y)))
			{
				do_area_sound(0, 0, x, y, 22);
				it[in].duration = 40; it[in].data[0] = 450+100*ch[cn].data[4];
				do_add_light(x, y, it[in].light[1]);
				try = 9;
			}
			// Three line sweep
			if (ch[cn].data[4]>=2)
			{
				switch (ch[cn].data[4])	{ case 2: x-=1; y-=2; break; case 3: x-=2; y-=1; break; case 4: x-=1; y+=2; break; default: break; }
				if (IS_IN_TLG(x, y) && map[x + y * MAPX].it==0 && (in = build_item(IT_EXPLOSION, x, y)))
				{	it[in].duration = 40; it[in].data[0] = 440+90*ch[cn].data[4]; do_add_light(x, y, it[in].light[1]);	}
				switch (ch[cn].data[4]) { case 2: x+=2; break; case 3: y+=2; break; case 4: x+=2; break; default: break; }
				if (IS_IN_TLG(x, y) && map[x + y * MAPX].it==0 && (in = build_item(IT_EXPLOSION, x, y)))
				{	it[in].duration = 40; it[in].data[0] = 440+90*ch[cn].data[4]; do_add_light(x, y, it[in].light[1]);	}
			}
			// Five line sweep
			if (ch[cn].data[4]==3)
			{
				x-=2; y-=3;
				if (IS_IN_TLG(x, y) && map[x + y * MAPX].it==0 && (in = build_item(IT_EXPLOSION, x, y)))
				{	it[in].duration = 40; it[in].data[0] = 430+80*ch[cn].data[4]; do_add_light(x, y, it[in].light[1]);	}
				y+=4;
				if (IS_IN_TLG(x, y) && map[x + y * MAPX].it==0 && (in = build_item(IT_EXPLOSION, x, y)))
				{	it[in].duration = 40; it[in].data[0] = 430+80*ch[cn].data[4]; do_add_light(x, y, it[in].light[1]);	}
			}
			// Three three line sweeps
			if (ch[cn].data[4]==4)
			{
				x-=5; y-=2;
				if (IS_IN_TLG(x, y) && map[x + y * MAPX].it==0 && (in = build_item(IT_EXPLOSION, x, y)))
				{	it[in].duration = 40; it[in].data[0] = 450+100*ch[cn].data[4]; do_add_light(x, y, it[in].light[1]);	}
				x-=1; y+=2;
				if (IS_IN_TLG(x, y) && map[x + y * MAPX].it==0 && (in = build_item(IT_EXPLOSION, x, y)))
				{	it[in].duration = 40; it[in].data[0] = 440+90*ch[cn].data[4]; do_add_light(x, y, it[in].light[1]);	}
				x+=2;
				if (IS_IN_TLG(x, y) && map[x + y * MAPX].it==0 && (in = build_item(IT_EXPLOSION, x, y)))
				{	it[in].duration = 40; it[in].data[0] = 440+90*ch[cn].data[4]; do_add_light(x, y, it[in].light[1]);	}
				x+=7; y-=2;
				if (IS_IN_TLG(x, y) && map[x + y * MAPX].it==0 && (in = build_item(IT_EXPLOSION, x, y)))
				{	it[in].duration = 40; it[in].data[0] = 450+100*ch[cn].data[4]; do_add_light(x, y, it[in].light[1]);	}
				x-=1; y+=2;
				if (IS_IN_TLG(x, y) && map[x + y * MAPX].it==0 && (in = build_item(IT_EXPLOSION, x, y)))
				{	it[in].duration = 40; it[in].data[0] = 440+90*ch[cn].data[4]; do_add_light(x, y, it[in].light[1]);	}
				x+=2;
				if (IS_IN_TLG(x, y) && map[x + y * MAPX].it==0 && (in = build_item(IT_EXPLOSION, x, y)))
				{	it[in].duration = 40; it[in].data[0] = 440+90*ch[cn].data[4]; do_add_light(x, y, it[in].light[1]);	}
			}
		}
		ch[cn].data[3] = globs->ticker + 1;
		ch[cn].data[5]++;
		if (ch[cn].data[5]>24) // End of the line
		{
			ch[cn].data[3] = 0;
			ch[cn].data[4] = 0;
			ch[cn].data[5] = 0;
		}
	}
	
	// Return if time is less than next action
	if (globs->ticker < ch[cn].data[2])
		return;
	
	switch (ch[cn].data[1])
	{
		case 1: 
			do_char_log(p, 3, "Gatekeeper: \"%s\"\n", "Welcome, champion. To beyond the coveted Last Gate. The last challenge -- my last challenge to you. You've journeyed far... Now, step forward and take the first strike.");
			ch[cn].data[1]++; 
			ch[cn].data[2] = globs->ticker + 1;
			break;
		case 2: 
			if (!ch[cn].attack_cn) return;
			do_char_log(p, 3, "Gatekeeper: \"%s\"\n", "Let's begin!");
			ch[cn].data[1]++; 
			ch[cn].data[2] = globs->ticker + TICKS;
			break;
		// Breakpoint 1: 80% Health
		case 3: 
			if (ch[cn].a_hp>ch[cn].hp[5]*800) return; // Wait until 80%
			clean_and_go(cn, go_xy[0][0], go_xy[0][1], go_xy[0][2], 800);
			do_char_log(p, 3, "Gatekeeper: \"%s\"\n", "Stalwart. Strong. The labyrinth has tested your might and muscle well, my friend.");
			ch[cn].data[1]++; 
			ch[cn].data[2] = globs->ticker + TICKS * 2;
			break;
		case 4:
			clean_and_go(cn, go_xy[0][0], min(max(ch[p].y,1549),1559), go_xy[0][2], 800);
			do_char_log(p, 3, "Gatekeeper: \"%s\"\n", "Let's try something new!");
			ch[cn].skill_nr = SK_MSHIELD; ch[cn].skill_target1 = cn; ch[cn].data[96] |= SP_MSHIELD; fx_add_effect(11, 8, cn, SP_MSHIELD, 0);
			//
			in = ch[cn].worn[WN_CHARM]; it[in].x = 0; it[in].y = 0; it[in].carried = 0;
			in = ch[cn].worn[WN_CHARM]  = pop_create_item(IT_CH_DEATH, cn); it[in].carried = cn;
			ch[cn].skill[SK_WARCRY][0]   = 90;
			do_update_char(cn);
			//
			ch[cn].data[1]++; 
			ch[cn].data[2] = globs->ticker + TICKS / 4;
			break;
		case 5:
			change_the_arena(1, frx, fry, tox, toy);
			ch[cn].data[1]++; 
			ch[cn].data[2] = globs->ticker + TICKS * 3;
			ch[cn].data[3] = 1;
			ch[cn].data[4] = 1;
			ch[cn].data[8] = XY2M(ch[cn].x, ch[cn].y);
			break;
		case 6:
			do_char_log(p, 3, "Gatekeeper: \"%s\"\n", "Ha!");
			clean_and_go(cn, go_xy[1][0], go_xy[1][1], go_xy[1][2], 800);
			npc_add_enemy(cn, p, 1);
			ch[cn].data[1]++; 
			ch[cn].data[2] = globs->ticker + TICKS;
			break;
		// Breakpoint 2: 60% Health
		case 7: 
			if (ch[cn].a_hp>ch[cn].hp[5]*600) return; // Wait until 60%
			clean_and_go(cn, go_xy[2][0], go_xy[2][1], go_xy[2][2], 600);
			do_char_log(p, 3, "Gatekeeper: \"%s\"\n", "Intuitive. Wise. The labyrinth has tested your reason and understanding, truly.");
			ch[cn].data[1]++; 
			ch[cn].data[2] = globs->ticker + TICKS * 2;
			break;
		case 8:
			clean_and_go(cn, min(max(ch[p].x,200),208), go_xy[2][1], go_xy[2][2], 600);
			do_char_log(p, 3, "Gatekeeper: \"%s\"\n", "Show me what you've got!");
			ch[cn].skill_nr = SK_MSHIELD; ch[cn].skill_target1 = cn; ch[cn].data[96] |= SP_MSHIELD; fx_add_effect(11, 8, cn, SP_MSHIELD, 0);
			//
			in = ch[cn].worn[WN_CHARM]; it[in].x = 0; it[in].y = 0; it[in].carried = 0;
			in = ch[cn].worn[WN_CHARM]  = pop_create_item(IT_CH_TOWER, cn); it[in].carried = cn;
			ch[cn].skill[SK_PULSE][0]    = 90;
			do_update_char(cn);
			//
			ch[cn].data[1]++; 
			ch[cn].data[2] = globs->ticker + TICKS / 4;
			break;
		case 9:
			change_the_arena(2, frx, fry, tox, toy);
			ch[cn].data[1]++; 
			ch[cn].data[2] = globs->ticker + TICKS * 3;
			ch[cn].data[3] = 1;
			ch[cn].data[4] = 2;
			ch[cn].data[8] = XY2M(ch[cn].x, ch[cn].y);
			break;
		case 10:
			do_char_log(p, 3, "Gatekeeper: \"%s\"\n", "Ha!");
			clean_and_go(cn, go_xy[3][0], go_xy[3][1], go_xy[3][2], 600);
			npc_add_enemy(cn, p, 1);
			ch[cn].data[1]++; 
			ch[cn].data[2] = globs->ticker + TICKS;
			break;
		// Breakpoint 3: 40% Health
		case 11: 
			if (ch[cn].a_hp>ch[cn].hp[5]*400) return; // Wait until 40%
			clean_and_go(cn, go_xy[4][0], go_xy[4][1], go_xy[4][2], 400);
			do_char_log(p, 3, "Gatekeeper: \"%s\"\n", "Dexterous. Agile. The labyrinth has tested your ability to navigate its tests.");
			ch[cn].data[1]++; 
			ch[cn].data[2] = globs->ticker + TICKS * 2;
			break;
		case 12:
			clean_and_go(cn, go_xy[4][0], min(max(ch[p].y,1551),1557), go_xy[4][2], 400);
			do_char_log(p, 3, "Gatekeeper: \"%s\"\n", "How much further shall you travel?");
			ch[cn].skill_nr = SK_MSHIELD; ch[cn].skill_target1 = cn; ch[cn].data[96] |= SP_MSHIELD; fx_add_effect(11, 8, cn, SP_MSHIELD, 0);
			//
			in = ch[cn].worn[WN_CHARM]; it[in].x = 0; it[in].y = 0; it[in].carried = 0;
			in = ch[cn].worn[WN_CHARM]  = pop_create_item(IT_CH_EMPEROR, cn); it[in].carried = cn;
			ch[cn].skill[SK_ZEPHYR][0]   = 90;
			ch[cn].skill[SK_LEAP][0]     = 90;
			do_update_char(cn);
			//
			ch[cn].data[1]++; 
			ch[cn].data[2] = globs->ticker + TICKS / 4;
			break;
		case 13:
			change_the_arena(3, frx, fry, tox, toy);
			ch[cn].data[1]++; 
			ch[cn].data[2] = globs->ticker + TICKS * 3;
			ch[cn].data[3] = 1;
			ch[cn].data[4] = 3;
			ch[cn].data[8] = XY2M(ch[cn].x, ch[cn].y);
			break;
		case 14:
			do_char_log(p, 3, "Gatekeeper: \"%s\"\n", "Ha!");
			clean_and_go(cn, go_xy[5][0], go_xy[5][1], go_xy[5][2], 400);
			npc_add_enemy(cn, p, 1);
			ch[cn].data[1]++; 
			ch[cn].data[2] = globs->ticker + TICKS;
			break;
		// Breakpoint 4: 20% Health - Add shadows for the sweep
		case 15: 
			if (ch[cn].a_hp>ch[cn].hp[5]*200) return; // Wait until 20%
			clean_and_go(cn, go_xy[6][0], go_xy[6][1], go_xy[6][2], 200);
			//
			co = pop_create_char(1432, 0); x = go_xy[6][0]-4; y = go_xy[6][1];
			if (co) { ch[cn].data[6] = co; ch[co].kindred |= KIN_SHADOW; ch[co].flags |= CF_NOSLEEP; ch[co].a_hp = 525; ch[co].sprite = ch[cn].sprite; }
			if (!god_drop_char_fuzzy(co, x, y)) { god_destroy_items(co); ch[co].used = USE_EMPTY; }
			fx_add_effect(12, 0, x, y, 0); ch[co].dir = go_xy[6][2];
			//
			co = pop_create_char(1432, 0); x = go_xy[6][0]+4; y = go_xy[6][1];
			if (co) { ch[cn].data[7] = co; ch[co].kindred |= KIN_SHADOW; ch[co].flags |= CF_NOSLEEP; ch[co].a_hp = 525; ch[co].sprite = ch[cn].sprite; }
			if (!god_drop_char_fuzzy(co, x, y)) { god_destroy_items(co); ch[co].used = USE_EMPTY; }
			fx_add_effect(12, 0, x, y, 0); ch[co].dir = go_xy[6][2];
			//
			do_char_log(p, 3, "Gatekeeper: \"%s\"\n", "Brave. Willing. The labyrinth has challenged your guile and mettle time and again!");
			ch[cn].data[1]++; 
			ch[cn].data[2] = globs->ticker + TICKS * 2;
			break;
		case 16:
			do_char_log(p, 3, "Gatekeeper: \"%s\"\n", "Let's bring this fight to a close, shall we?");
			ch[cn].skill_nr = SK_MSHIELD; ch[cn].skill_target1 = cn; ch[cn].data[96] |= SP_MSHIELD; fx_add_effect(11, 8, cn, SP_MSHIELD, 0);
			ch[ch[cn].data[6]].skill_nr = SK_MSHIELD; ch[ch[cn].data[6]].skill_target1 = ch[cn].data[6]; 
				ch[ch[cn].data[6]].data[96] |= SP_MSHIELD; fx_add_effect(11, 8, ch[cn].data[6], SP_MSHIELD, 0);
			ch[ch[cn].data[7]].skill_nr = SK_MSHIELD; ch[ch[cn].data[7]].skill_target1 = ch[cn].data[7]; 
				ch[ch[cn].data[7]].data[96] |= SP_MSHIELD; fx_add_effect(11, 8, ch[cn].data[7], SP_MSHIELD, 0);
			//
			in = ch[cn].worn[WN_CHARM]; it[in].x = 0; it[in].y = 0; it[in].carried = 0;
			in = ch[cn].worn[WN_CHARM]  = pop_create_item(IT_CH_JUDGE, cn); it[in].carried = cn;
			ch[cn].skill[SK_LETHARGY][0] = 90;
			do_update_char(cn);
			//
			ch[cn].data[1]++; 
			ch[cn].data[2] = globs->ticker + TICKS / 4;
			break;
		case 17:
			change_the_arena(4, frx, fry, tox, toy);
			ch[cn].data[1]++; 
			ch[cn].data[2] = globs->ticker + TICKS * 3;
			ch[cn].data[3] = 1;
			ch[cn].data[4] = 4;
			ch[cn].data[8] = XY2M(ch[cn].x, ch[cn].y);
			break;
		case 18:
			do_char_log(p, 3, "Gatekeeper: \"%s\"\n", "Ha!");
			//
			if (IS_SANECHAR(co = ch[cn].data[6]))
			{
				god_destroy_items(co);
				if (ch[co].used==USE_ACTIVE) plr_map_remove(co);
				ch[co].flags = 0;
				ch[co].used = USE_EMPTY;
				ch[cn].data[6] = 0;
			}
			fx_add_effect(12, 0, go_xy[6][0]-4, go_xy[6][1], 0);
			if (IS_SANECHAR(co = ch[cn].data[7]))
			{
				god_destroy_items(co);
				if (ch[co].used==USE_ACTIVE) plr_map_remove(co);
				ch[co].flags = 0;
				ch[co].used = USE_EMPTY;
				ch[cn].data[7] = 0;
			}
			fx_add_effect(12, 0, go_xy[6][0]+4, go_xy[6][1], 0);
			//
			clean_and_go(cn, go_xy[7][0], go_xy[7][1], go_xy[7][2], 200);
			npc_add_enemy(cn, p, 1);
			ch[cn].data[1]++; 
			ch[cn].data[2] = globs->ticker + TICKS;
			break;
		// Wait until death
		default: break;
	}
}

void npc_lab_lord_driver(int cn)
{
	int co, in;
	
	if (ch[cn].data[2]==0 && ch[cn].data[0])
	{
		ch[cn].data[2] = 1;
		co = pop_create_char(ch[cn].data[0], 0);
		if (!co)
		{
			do_sayx(cn, "create char (%d)", co);
			return;
		}
		if (!god_drop_char_fuzzy(co, ch[cn].x, ch[cn].y))
		{
			do_sayx(cn, "drop char (%d)", co);
			god_destroy_items(co);
			ch[co].used = USE_EMPTY;
			return;
		}
		fx_add_effect(6, 0, ch[co].x, ch[co].y, 0);
		fx_add_effect(7, 0, ch[cn].x, ch[cn].y, 0);
		do_sayx(cn, "Underling, come to my aid!");
		chlog(cn, "created a new monster (lab_lord)");
	}
}

void npc_emperor_driver(int cn)
{
	int co, in, n;
	int x = 928, y = 408;
	
	if (ch[cn].data[2]==0 && ch[cn].data[0])
	{
		ch[cn].data[3] = globs->ticker + TICKS*10;
		ch[cn].data[4] = ch[cn].attack_cn;
		ch[cn].data[2] = 1;
		
		if ((in = map[EP_GOLEM1X+EP_GOLEM1Y*MAPX].it) && !it[in].active)
		{
			it[in].active = it[in].duration;
			co = pop_create_char(ch[cn].data[0], 0);
			if (!co) 
			{
				chlog(cn, "npc_emperor_driver pop failed");
				return;
			}
			ch[co].dir = 1;
			if (!god_drop_char(co, EP_GOLEM1X, EP_GOLEM1Y))
			{
				chlog(cn, "npc_emperor_driver drop failed");
				god_destroy_items(co); ch[co].used = USE_EMPTY; return;
			}
			ch[co].data[64] = globs->ticker + TICKS * 60 * 15;
			ch[co].goto_x = x;
			ch[co].goto_y = y;
			ch[co].attack_cn = ch[cn].attack_cn;
		}
		if ((in = map[EP_GOLEM2X+EP_GOLEM2Y*MAPX].it) && !it[in].active)
		{
			it[in].active = it[in].duration;
			co = pop_create_char(ch[cn].data[0], 0);
			if (!co) 
			{
				chlog(cn, "npc_emperor_driver pop failed");
				return;
			}
			ch[co].dir = 2;
			if (!god_drop_char(co, EP_GOLEM2X, EP_GOLEM2Y))
			{
				chlog(cn, "npc_emperor_driver drop failed");
				god_destroy_items(co); ch[co].used = USE_EMPTY; return;
			}
			ch[co].data[64] = globs->ticker + TICKS * 60 * 15;
			ch[co].goto_x = x;
			ch[co].goto_y = y;
			ch[co].attack_cn = ch[cn].attack_cn;
		}
		if ((in = map[EP_GOLEM3X+EP_GOLEM3Y*MAPX].it) && !it[in].active)
		{
			it[in].active = it[in].duration;
			co = pop_create_char(ch[cn].data[0], 0);
			if (!co) 
			{
				chlog(cn, "npc_emperor_driver pop failed");
				return;
			}
			ch[co].dir = 4;
			if (!god_drop_char(co, EP_GOLEM3X, EP_GOLEM3Y))
			{
				chlog(cn, "npc_emperor_driver drop failed");
				god_destroy_items(co); ch[co].used = USE_EMPTY; return;
			}
			ch[co].data[64] = globs->ticker + TICKS * 60 * 15;
			ch[co].goto_x = x;
			ch[co].goto_y = y;
			ch[co].attack_cn = ch[cn].attack_cn;
		}
		if ((in = map[EP_GOLEM4X+EP_GOLEM4Y*MAPX].it) && !it[in].active)
		{
			it[in].active = it[in].duration;
			co = pop_create_char(ch[cn].data[0], 0);
			if (!co) 
			{
				chlog(cn, "npc_emperor_driver pop failed");
				return;
			}
			ch[co].dir = 3;
			if (!god_drop_char(co, EP_GOLEM4X, EP_GOLEM4Y))
			{
				chlog(cn, "npc_emperor_driver drop failed");
				god_destroy_items(co); ch[co].used = USE_EMPTY; return;
			}
			ch[co].data[64] = globs->ticker + TICKS * 60 * 15;
			ch[co].goto_x = x;
			ch[co].goto_y = y;
			ch[co].attack_cn = ch[cn].attack_cn;
		}
		
		do_sayx(cn, "Rissse my conssstructs! Purge thisss filthy outsssider!");
		chlog(cn, "spawned emerald golems");
		
		fx_add_effect(6, 0, ch[cn].x, ch[cn].y, 0);
		god_transfer_char(cn, 928, 416);
		ch[cn].dir = 3;
		ch[ch[cn].attack_cn].attack_cn = 0;
		ch[cn].attack_cn = 0;
		npc_remove_all_enemies(cn);
		fx_add_effect(6, 0, ch[cn].x, ch[cn].y, 0);
		
		do_area_log(cn, 0, ch[cn].x, ch[cn].y, 1, "The %s leapt across the room.\n", ch[cn].name);
		
		B_SK(cn, SK_SLOW) = 90;
		B_SK(cn, SK_CURSE) = 90;
		
		ch[cn].a_hp   = 9999999;
		ch[cn].a_end  = 999999;
		ch[cn].a_mana = 999999;
	}
	else if (ch[cn].data[2]==1 && ch[cn].data[4]==0)
	{
		ch[cn].data[2] = 2;
		
		in = god_create_item(IT_RPOT);
		god_give_char(in, cn);
		
		do_sayx(cn, "Enough of thisss charade! Die!!");
		
		npc_quaff_potion(cn, IT_RPOT, 254);
		
		B_SK(cn, SK_ZEPHYR) = 90;
		
		ch[cn].a_hp   = 9999999;
		ch[cn].a_end  = 999999;
		ch[cn].a_mana = 999999;
	}
}

void npc_shivab_driver(int cn)
{
	int co, in, n;
	
	if (ch[cn].data[2]==0)
	{
		ch[cn].data[2] = 1;
		//
		co = pop_create_char(1244, 0);
		if (!co) { chlog(cn, "npc_shivab_driver pop failed"); return; }
		if (!god_drop_char(co, 580, 118))
		{	chlog(cn, "npc_shivab_driver drop failed");
			god_destroy_items(co); ch[co].used = USE_EMPTY; return; }
		ch[co].data[64] = globs->ticker + TICKS * 60 * 15;
		ch[co].attack_cn = ch[cn].attack_cn;
		fx_add_effect(5, 0, ch[co].x, ch[co].y, 0);
		//
		co = pop_create_char(1244, 0);
		if (!co) { chlog(cn, "npc_shivab_driver pop failed"); return; }
		if (!god_drop_char(co, 573, 118))
		{	chlog(cn, "npc_shivab_driver drop failed");
			god_destroy_items(co); ch[co].used = USE_EMPTY; return; }
		ch[co].data[64] = globs->ticker + TICKS * 60 * 15;
		ch[co].attack_cn = ch[cn].attack_cn;
		fx_add_effect(5, 0, ch[co].x, ch[co].y, 0);
		//
		co = pop_create_char(1244, 0);
		if (!co) { chlog(cn, "npc_shivab_driver pop failed"); return; }
		if (!god_drop_char(co, 571, 124))
		{	chlog(cn, "npc_shivab_driver drop failed");
			god_destroy_items(co); ch[co].used = USE_EMPTY; return; }
		ch[co].data[64] = globs->ticker + TICKS * 60 * 15;
		ch[co].attack_cn = ch[cn].attack_cn;
		fx_add_effect(5, 0, ch[co].x, ch[co].y, 0);
		//
		
		do_sayx(cn, "How about a little plague?");
		chlog(cn, "spawned plagues");
		
		ch[ch[cn].attack_cn].attack_cn = 0;
		ch[cn].attack_cn = 0;
		fx_add_effect(6, 0, ch[cn].x, ch[cn].y, 0);
		god_transfer_char(cn, 577, 126);
		fx_add_effect(6, 0, ch[cn].x, ch[cn].y, 0);
		ch[cn].dir = 3;
		
		ch[cn].a_hp   = 9999999;
		ch[cn].a_end  = 999999;
		ch[cn].a_mana = 999999;
	}
	else if (ch[cn].data[2]==1)
	{
		ch[cn].data[2] = 2;
		//
		co = pop_create_char(1245, 0);
		if (!co) { chlog(cn, "npc_shivab_driver pop failed"); return; }
		if (!god_drop_char(co, 580, 118))
		{	chlog(cn, "npc_shivab_driver drop failed");
			god_destroy_items(co); ch[co].used = USE_EMPTY; return; }
		ch[co].data[64] = globs->ticker + TICKS * 60 * 15;
		ch[co].attack_cn = ch[cn].attack_cn;
		fx_add_effect(5, 0, ch[co].x, ch[co].y, 0);
		//
		co = pop_create_char(1245, 0);
		if (!co) { chlog(cn, "npc_shivab_driver pop failed"); return; }
		if (!god_drop_char(co, 580, 125))
		{	chlog(cn, "npc_shivab_driver drop failed");
			god_destroy_items(co); ch[co].used = USE_EMPTY; return; }
		ch[co].data[64] = globs->ticker + TICKS * 60 * 15;
		ch[co].attack_cn = ch[cn].attack_cn;
		fx_add_effect(5, 0, ch[co].x, ch[co].y, 0);
		//
		co = pop_create_char(1245, 0);
		if (!co) { chlog(cn, "npc_shivab_driver pop failed"); return; }
		if (!god_drop_char(co, 574, 127))
		{	chlog(cn, "npc_shivab_driver drop failed");
			god_destroy_items(co); ch[co].used = USE_EMPTY; return; }
		ch[co].data[64] = globs->ticker + TICKS * 60 * 15;
		ch[co].attack_cn = ch[cn].attack_cn;
		fx_add_effect(5, 0, ch[co].x, ch[co].y, 0);
		//
		
		do_sayx(cn, "The pits of hell are quite welcoming this time of year.");
		chlog(cn, "spawned devils");
		
		ch[ch[cn].attack_cn].attack_cn = 0;
		ch[cn].attack_cn = 0;
		fx_add_effect(6, 0, ch[cn].x, ch[cn].y, 0);
		god_transfer_char(cn, 572, 121);
		fx_add_effect(6, 0, ch[cn].x, ch[cn].y, 0);
		ch[cn].dir = 1;
		
		ch[cn].a_hp   = 9999999;
		ch[cn].a_end  = 999999;
		ch[cn].a_mana = 999999;
	}
	else if (ch[cn].data[2]==2)
	{
		ch[cn].data[2] = 3;
		//
		co = pop_create_char(1246, 0);
		if (!co) { chlog(cn, "npc_shivab_driver pop failed"); return; }
		if (!god_drop_char(co, 573, 118))
		{	chlog(cn, "npc_shivab_driver drop failed");
			god_destroy_items(co); ch[co].used = USE_EMPTY; return; }
		ch[co].data[64] = globs->ticker + TICKS * 60 * 15;
		ch[co].attack_cn = ch[cn].attack_cn;
		fx_add_effect(5, 0, ch[co].x, ch[co].y, 0);
		//
		co = pop_create_char(1246, 0);
		if (!co) { chlog(cn, "npc_shivab_driver pop failed"); return; }
		if (!god_drop_char(co, 571, 124))
		{	chlog(cn, "npc_shivab_driver drop failed");
			god_destroy_items(co); ch[co].used = USE_EMPTY; return; }
		ch[co].data[64] = globs->ticker + TICKS * 60 * 15;
		ch[co].attack_cn = ch[cn].attack_cn;
		fx_add_effect(5, 0, ch[co].x, ch[co].y, 0);
		//
		co = pop_create_char(1246, 0);
		if (!co) { chlog(cn, "npc_shivab_driver pop failed"); return; }
		if (!god_drop_char(co, 574, 127))
		{	chlog(cn, "npc_shivab_driver drop failed");
			god_destroy_items(co); ch[co].used = USE_EMPTY; return; }
		ch[co].data[64] = globs->ticker + TICKS * 60 * 15;
		ch[co].attack_cn = ch[cn].attack_cn;
		fx_add_effect(5, 0, ch[co].x, ch[co].y, 0);
		//
		
		do_sayx(cn, "Out with the flame, and in with the cold!");
		chlog(cn, "spawned hypotherms");
		
		ch[ch[cn].attack_cn].attack_cn = 0;
		ch[cn].attack_cn = 0;
		fx_add_effect(6, 0, ch[cn].x, ch[cn].y, 0);
		god_transfer_char(cn, 580, 121);
		fx_add_effect(6, 0, ch[cn].x, ch[cn].y, 0);
		ch[cn].dir = 2;
		
		ch[cn].a_hp   = 9999999;
		ch[cn].a_end  = 999999;
		ch[cn].a_mana = 999999;
	}
	else if (ch[cn].data[2]==3)
	{
		ch[cn].data[2] = 4;
		//
		co = pop_create_char(1244, 0);
		if (!co) { chlog(cn, "npc_shivab_driver pop failed"); return; }
		if (!god_drop_char(co, 573, 118))
		{	chlog(cn, "npc_shivab_driver drop failed");
			god_destroy_items(co); ch[co].used = USE_EMPTY; return; }
		ch[co].data[64] = globs->ticker + TICKS * 60 * 15;
		ch[co].attack_cn = ch[cn].attack_cn;
		fx_add_effect(5, 0, ch[co].x, ch[co].y, 0);
		//
		co = pop_create_char(1245, 0);
		if (!co) { chlog(cn, "npc_shivab_driver pop failed"); return; }
		if (!god_drop_char(co, 580, 118))
		{	chlog(cn, "npc_shivab_driver drop failed");
			god_destroy_items(co); ch[co].used = USE_EMPTY; return; }
		ch[co].data[64] = globs->ticker + TICKS * 60 * 15;
		ch[co].attack_cn = ch[cn].attack_cn;
		fx_add_effect(5, 0, ch[co].x, ch[co].y, 0);
		//
		co = pop_create_char(1246, 0);
		if (!co) { chlog(cn, "npc_shivab_driver pop failed"); return; }
		if (!god_drop_char(co, 580, 125))
		{	chlog(cn, "npc_shivab_driver drop failed");
			god_destroy_items(co); ch[co].used = USE_EMPTY; return; }
		ch[co].data[64] = globs->ticker + TICKS * 60 * 15;
		ch[co].attack_cn = ch[cn].attack_cn;
		fx_add_effect(5, 0, ch[co].x, ch[co].y, 0);
		//
		
		do_sayx(cn, "Let's end this!");
		chlog(cn, "spawned all three");
		
		in = god_create_item(IT_RPOT);
		god_give_char(in, cn);
		npc_quaff_potion(cn, IT_RPOT, 254);
		
		ch[ch[cn].attack_cn].attack_cn = 0;
		ch[cn].attack_cn = 0;
		fx_add_effect(6, 0, ch[cn].x, ch[cn].y, 0);
		god_transfer_char(cn, 572, 126);
		fx_add_effect(6, 0, ch[cn].x, ch[cn].y, 0);
		ch[cn].dir = 7;
		
		ch[cn].a_hp   = 9999999;
		ch[cn].a_end  = 999999;
		ch[cn].a_mana = 999999;
	}
}

int npc_driver_high(int cn)
{
	int x, y, in, co, indoor1, indoor2, cc, in2, n;
	int mc, m, priestess=0;
	
	if (get_tarot(cn, IT_CH_PREIST_R)) priestess = 1;

	if (ch[cn].data[25])
	{
		switch(ch[cn].data[25])
		{
		case  1:
			return( npc_stunrun_high(cn) );
		case  2:
			return( npc_cityattack_high(cn) );
		case  3:
			return( npc_malte_high(cn) );
		case  4:
			return( npc_shiva_high(cn) );
		default:
			chlog(cn, "unknown special driver %d", ch[cn].data[25]);
			break;
		}
		return 0;
	}

	// reset panic mode if expired
	if (ch[cn].data[78]<globs->ticker)
	{
		ch[cn].data[78] = 0;
	}

	// self destruct
	if (ch[cn].data[64])
	{
		if (ch[cn].data[64]<TICKS * 60 * 15)  // values less than 15 minutes mean it's a value set from the administration
		{                                     // interface and shall wait that long...
			ch[cn].data[64] += globs->ticker;
		}

		if (ch[cn].data[64]<globs->ticker && IS_SHADOW(cn) && IS_PLAYER(ch[cn].data[CHD_MASTER]))
		{
			ch[ch[cn].data[CHD_MASTER]].data[PCD_SHADOWCOPY] = 0;
			if (IS_SANECHAR(ch[cn].data[CHD_MASTER])) 
				do_sayx(cn, "I'm done, %s...", ch[ch[cn].data[CHD_MASTER]].name);
			do_give_exp(ch[cn].data[CHD_MASTER], ch[cn].data[28], 1, -1, 0);
			fx_add_effect(6, 0, ch[ch[cn].data[CHD_MASTER]].x, ch[ch[cn].data[CHD_MASTER]].y, 0);
			fx_add_effect(7, 0, ch[cn].x, ch[cn].y, 0);
			die_companion(cn);
			return 1;
		}
		else if (ch[cn].data[64]<globs->ticker && IS_SHADOW(cn) && IS_PLAYER_SC(ch[cn].data[CHD_MASTER]) && IS_PLAYER(ch[ch[cn].data[CHD_MASTER]].data[CHD_MASTER]))
		{
			if (ch[ch[cn].data[CHD_MASTER]].data[9] == 2) ch[ch[cn].data[CHD_MASTER]].data[9] = 1;
			ch[ch[cn].data[CHD_MASTER]].data[PCD_SHADOWCOPY] = 0;
			if (IS_SANECHAR(ch[cn].data[CHD_MASTER])) 
				do_sayx(cn, "I'm done, %s...", ch[ch[cn].data[CHD_MASTER]].name);
			do_give_exp(ch[ch[cn].data[CHD_MASTER]].data[CHD_MASTER], ch[cn].data[28], 1, -1, 0);
			fx_add_effect(6, 0, ch[ch[ch[cn].data[CHD_MASTER]].data[CHD_MASTER]].x, ch[ch[ch[cn].data[CHD_MASTER]].data[CHD_MASTER]].y, 0);
			fx_add_effect(7, 0, ch[cn].x, ch[cn].y, 0);
			die_companion(cn);
			return 1;
		}
		else if (ch[cn].data[64]<globs->ticker && IS_SHADOW(cn))
		{
			ch[ch[cn].data[CHD_MASTER]].data[PCD_SHADOWCOPY] = 0;
			if (IS_SANECHAR(ch[cn].data[CHD_MASTER])) 
				do_sayx(cn, "I'm done, %s...", ch[ch[cn].data[CHD_MASTER]].name);
			fx_add_effect(7, 0, ch[cn].x, ch[cn].y, 0);
			god_destroy_items(cn);
			plr_map_remove(cn);
			ch[cn].used = USE_EMPTY;
			remove_enemy(cn);
			return 1;
		}

		if (ch[cn].data[64]<globs->ticker)
		{
			if (IS_BLOODY(cn))
				do_sayx(cn, "fRee333333!!");
			else
				do_sayx(cn, "Free!");
			fx_add_effect(7, 0, ch[cn].x, ch[cn].y, 0);
			god_destroy_items(cn);
			plr_map_remove(cn);
			ch[cn].used = USE_EMPTY;
			remove_enemy(cn);
			return 1;
		}
	}

	// Count down master-no-see timer for player ghost companions
	if (IS_COMP_TEMP(cn) && ch[cn].data[64]==0)
	{
		co = ch[cn].data[CHD_MASTER]; // master
		if (!IS_SANECHAR(co) || ch[co].data[64] != cn)   // mismatch
		{
			chlog(cn, "killed for bad master(%d)", co);
			die_companion(cn);
			return 1;
		}
		if (globs->ticker > ch[cn].data[98])
		{
			ch[co].luck--;
			chlog(cn, "Self-destructed because of neglect by %s", ch[co].name);
			die_companion(cn);
			return 1;
		}
	}

	// Count down riddle timeout for riddle givers
	if ((ch[cn].data[72] >= RIDDLE_MIN_AREA) && (ch[cn].data[72] <= RIDDLE_MAX_AREA))
	{
		int idx;

		idx = ch[cn].data[72] - RIDDLE_MIN_AREA;
		if (riddletimeout[idx] > 0)
		{
			if (--riddletimeout[idx] <= 0)
			{
				// Guesser forgets riddler
				if (IS_SANECHAR(guesser[idx]) && IS_PLAYER(guesser[idx]) && IS_IN_IX(ch[guesser[idx]].x, ch[guesser[idx]].y))
				{
					ch[guesser[idx]].data[PCD_RIDDLER] = 0;
				}
				guesser[idx] = 0;
				do_char_log(cn, 1, "%s tells you: Too late! Too late! Try again some time.\n", ch[cn].name);
			}
		}
	}
	
	// Guess I'll put this here?
	if (ch[cn].temp==CT_PANDIUM)
	{
		pandium_driver(cn);
	}
	if (ch[cn].temp==CT_LAB20_KEEP)
	{
		gatekeeper_driver(cn);
	}
	
	// driver check if low health
	if (ch[cn].a_hp<ch[cn].hp[5] * 400)
	{
		if (ch[cn].data[26]==12)
		{
			npc_lab_lord_driver(cn);
		}
		else if (ch[cn].data[26]==13)
		{
			npc_emperor_driver(cn);
		}
		else if (ch[cn].data[26]==14)
		{
			npc_shivab_driver(cn);
		}
	}
	
	// heal us if we're hurt
	if ((ch[cn].a_hp<ch[cn].hp[5] * 600 && (!IS_PLAYER_COMP(cn) || get_tarot(cn, IT_CH_STAR))) || ch[cn].a_hp<ch[cn].hp[5] * 400)
	{
		if (npc_try_spell(cn, cn, SK_HEAL))
		{
			return 1;
		}
		
		if (ch[cn].temp==CT_BSMAGE1 || ch[cn].temp==CT_BSMAGE2 || ch[cn].temp==CT_BSMAGE3) 
		{
			if (globs->flags & GF_NEWBS)
				stronghold_mage_driver_ver2(cn);
			else
				stronghold_mage_driver(cn);
		}
	}
	
	// Dispel - for self
	if (B_SK(cn, SK_DISPEL) && !get_tarot(cn, IT_CH_HEIROPH) && !has_buff(cn, SK_DISPEL))
	{
		for (n = 0, m = 0; n<MAXBUFFS; n++)
		{
			if ((in = ch[cn].spell[n])==0) continue;
			if (IS_DISPELABLE1(bu[in].temp)) m++;
		}
		if (m>1)
		{
			if (npc_try_spell(cn, cn, SK_DISPEL))
			{
				return 1;
			}
		}
	}

	// donate/destroy citem if that's our job
	if ((in = ch[cn].citem)!=0 && ch[cn].data[47]>0 && ch[cn].data[47]<10)
	{
		if (it[in].damage_state || (it[in].flags & IF_SHOPDESTROY) || !(it[in].flags & IF_DONATE))
		{
			it[in].used  = USE_EMPTY;
			ch[cn].citem = 0;
		}
		else
		{
			it[in].current_age[0] = 0;
			it[in].current_age[1] = 0;
			it[in].current_damage = 0;
			god_donate_item(in, ch[cn].data[47]);
			ch[cn].citem = 0;
		}
	}

	if ((in = ch[cn].item[39])!=0 && ch[cn].data[47]>0 && ch[cn].data[47]<10)
	{
		if (it[in].damage_state || (it[in].flags & IF_SHOPDESTROY) || !(it[in].flags & IF_DONATE))
		{
			it[in].used  = USE_EMPTY;
			ch[cn].citem = 0;
		}
		else
		{
			it[in].current_age[0] = 0;
			it[in].current_age[1] = 0;
			it[in].current_damage = 0;
			god_donate_item(in, ch[cn].data[47]);
			ch[cn].item[39] = 0;
		}
	}

	// generic spell management
	if (ch[cn].a_mana>ch[cn].mana[5] * 850 && !(ch[cn].flags & CF_MERCHANT))
	{
		if (ch[cn].a_mana>75000 && npc_try_spell(cn, cn, SK_BLESS))
		{
			return 1;
		}
		if (npc_try_spell(cn, cn, SK_PROTECT))
		{
			return 1;
		}
		if (M_SK(cn, SK_MSHIELD) >= 84) // to avoid persistent awake timers
		{
			if (!priestess && npc_try_spell(cn, cn, SK_MSHIELD))
			{
				return 1;
			}
		}
		if (npc_try_spell(cn, cn, SK_HASTE))
		{
			return 1;
		}
		if (npc_try_spell(cn, cn, SK_ENHANCE))
		{
			return 1;
		}
		if (npc_try_spell(cn, cn, SK_BLESS))
		{
			return 1;
		}
	}
	
	if (IS_PLAYER_COMP(cn) && ch[cn].data[1]<2 && IS_SANECHAR(co = ch[cn].data[CHD_MASTER]))
	{
		// companion endurance management
		if (ch[cn].a_end>10000)
		{
			if (ch[co].mode==2 && ch[cn].mode!=2)
			{
				ch[cn].mode = 2;
				do_update_char(cn);
			}
			if (ch[co].mode==1 && ch[cn].mode!=1)
			{
				ch[cn].mode = 1;
				do_update_char(cn);
			}
			if (ch[co].mode==0 && ch[cn].mode!=0)
			{
				ch[cn].mode = 0;
				do_update_char(cn);
			}
		}
		else if (ch[cn].mode!=0)
		{
			ch[cn].mode = 0;
			do_update_char(cn);
		}
	}
	else
	{
		// generic endurance management
		if (ch[cn].data[58]>1 && ch[cn].a_end>10000)
		{
			if (ch[cn].mode!=2)
			{
				ch[cn].mode = 2;
				do_update_char(cn);
			}
		}
		else if (ch[cn].data[58]==1 && ch[cn].a_end>10000)
		{
			if (ch[cn].mode!=1)
			{
				ch[cn].mode = 1;
				do_update_char(cn);
			}
		}
		else if (ch[cn].mode!=0)
		{
			ch[cn].mode = 0;
			do_update_char(cn);
		}
	}

	// create light
	if ((ch[cn].data[62]<11 && ch[cn].data[62]>ch[cn].data[58] && check_dlight(ch[cn].x, ch[cn].y)<20 && map[ch[cn].x + ch[cn].y * MAPX].light<20)
		|| ch[cn].data[62]-10>ch[cn].data[58])
	{
		if (npc_try_spell(cn, cn, SK_LIGHT))
		{
			return 1;
		}
	}

	// make sure protected character survives
	if ((co = ch[cn].data[CHD_MASTER])!=0)
	{
		if ((ch[co].a_hp<ch[co].hp[5] * 600 && (!IS_PLAYER_COMP(cn) || get_tarot(cn, IT_CH_STAR))) || ch[co].a_hp<ch[co].hp[5] * 400) // he's hurt
		{
			if (npc_try_spell(cn, co, SK_HEAL))
			{
				return 1;
			}
		}
		
		// Dispel - for friend
		if (B_SK(cn, SK_DISPEL) && !get_tarot(cn, IT_CH_HEIROPH) && !has_buff(co, SK_DISPEL))
		{
			for (n = 0, m = 0; n<MAXBUFFS; n++)
			{
				if ((in = ch[co].spell[n])==0) continue;
				if (IS_DISPELABLE1(bu[in].temp)) m++;
			}
			if (m>1)
			{
				if (npc_try_spell(cn, co, SK_DISPEL))
				{
					return 1;
				}
			}
		}
	}

	// help friend
	if ((co = ch[cn].data[65])!=0 && ch[cn].a_mana>ch[cn].mana[5]*250)
	{
		cc = ch[co].attack_cn;

		// bless us first if we have enough mana - makes spells more powerful
		if (ch[cn].a_mana>get_spellcost(cn, SK_BLESS) * 2 + get_spellcost(cn, SK_PROTECT) + get_spellcost(cn, SK_ENHANCE))
		{
			if (npc_try_spell(cn, cn, SK_BLESS))
			{
				return 1;
			}
		}

		if ((ch[co].a_hp<ch[co].hp[5] * 600 && (!IS_PLAYER_COMP(cn) || get_tarot(cn, IT_CH_STAR))) || ch[co].a_hp<ch[co].hp[5] * 400) // he's hurt
		{
			if (npc_try_spell(cn, co, SK_HEAL))
			{
				return 1;
			}
		}
		
		// Dispel - for friend
		if (B_SK(cn, SK_DISPEL) && !get_tarot(cn, IT_CH_HEIROPH) && !has_buff(co, SK_DISPEL))
		{
			for (n = 0, m = 0; n<MAXBUFFS; n++)
			{
				if ((in = ch[co].spell[n])==0) continue;
				if (IS_DISPELABLE1(bu[in].temp)) m++;
			}
			if (m>1)
			{
				if (npc_try_spell(cn, co, SK_DISPEL))
				{
					return 1;
				}
			}
		}

		if (!npc_can_spell(co, cn, SK_PROTECT) && npc_try_spell(cn, co, SK_PROTECT))
		{
			return 1;
		}
		if (!npc_can_spell(co, cn, SK_ENHANCE) && npc_try_spell(cn, co, SK_ENHANCE))
		{
			return 1;
		}
		if (!npc_can_spell(co, cn, SK_BLESS) && npc_try_spell(cn, co, SK_BLESS))
		{
			return 1;
		}
		
		// taunt this enemy to pull them off our friend
		if (cc && globs->ticker>ch[cn].data[74] && npc_try_spell(cn, cc, SK_TAUNT))
		{
			ch[cn].data[74] = globs->ticker + TICKS * 40;
			return 1;
		}
		
		// blast this enemy if our friend is losing and his enemy is our enemy as well
		if (cc && ch[co].a_hp<ch[co].hp[5] * 650 && npc_is_enemy(cn, cc))
		{
			if (globs->ticker>ch[co].data[75] && npc_try_spell(cn, cc, SK_BLAST))
			{
				ch[co].data[75] = globs->ticker + (TICKS * 6)/2;
				return 1;
			}
		}
		
		m = ch[co].x + ch[co].y * MAPX;
		
		// Check for enemies around our ally and see if they're hitting too
		//  this is helpful if our ally has fightback turned off
		for (n=0; n<4; n++)
		{
			switch (n)
			{
				case 0: mc = m + 1; break;
				case 1: mc = m - 1; break;
				case 2: mc = m + MAPX; break;
				case 3: mc = m - MAPX; break;
			}
			if ((cc = map[mc].ch)!=0 && ch[cc].attack_cn==co)
			{
				// taunt this enemy to pull them off our friend
				if (cc && globs->ticker>ch[cn].data[74] && npc_try_spell(cn, cc, SK_TAUNT))
				{
					ch[cn].data[74] = globs->ticker + TICKS * 40;
					return 1;
				}
				
				// blast this enemy if our friend is losing and his enemy is our enemy as well
				if (cc && ch[co].a_hp<ch[co].hp[5] * 650 && npc_is_enemy(cn, cc))
				{
					if (globs->ticker>ch[co].data[75] && npc_try_spell(cn, cc, SK_BLAST))
					{
						ch[co].data[75] = globs->ticker + (TICKS * 6)/2;
						return 1;
					}
				}
			}
		}
		//
		ch[cn].data[65] = 0;
	}
	
	// Liz Emperor
	if (ch[cn].data[26]==13 && ch[cn].data[4] && globs->ticker>ch[cn].data[3])
	{
		ch[cn].attack_cn = ch[cn].data[4];
		ch[cn].data[3] = 0;
		ch[cn].data[4] = 0;
	}
	
	// generic fight-magic management
	if ((co = ch[cn].attack_cn)!=0 || ch[cn].data[78]) // we're fighting
	{
		if (ch[cn].data[26]) npc_wedge_doors(ch[cn].data[26], 1);
		npc_activate_rings(cn, 1);
		/*
		if (npc_quaff_potion(cn, 1479, 254)) // use spot if available
			return 1;
		if (npc_quaff_potion(cn, 833, 254)) // use gpot if available
			return 1;
		if (npc_quaff_potion(cn, 267, 254)) // use rpot if available
			return 1;
		*/
		if (co && (ch[cn].a_hp<ch[cn].hp[5] * 600 || !RANDOM(10))) // we're losing
		{
			if (globs->ticker>ch[co].data[75] && npc_try_spell(cn, co, SK_BLAST))
			{
				ch[co].data[75] = globs->ticker + TICKS;
				return 1;
			}
		}

		if (ch[cn].a_mana>75000 && npc_try_spell(cn, cn, SK_BLESS))
		{
			return 1;
		}
		if (npc_try_spell(cn, cn, SK_PROTECT))
		{
			return 1;
		}
		if (!priestess && npc_try_spell(cn, cn, SK_MSHIELD))
		{
			return 1;
		}
		if (npc_try_spell(cn, cn, SK_HASTE))
		{
			return 1;
		}
		if (npc_try_spell(cn, cn, SK_ENHANCE))
		{
			return 1;
		}
		if (npc_try_spell(cn, cn, SK_BLESS))
		{
			return 1;
		}
		if (get_tarot(cn, IT_CH_EMPERO_R) && ch[cn].a_end>ch[cn].end[5]*500 && npc_try_spell(cn, cn, SK_WARCRY))
		{
			return 1;
		}
		
		if (co && ch[cn].a_mana>ch[cn].mana[5]*600 && !has_buff(cn, SK_LETHARGY) && !get_tarot(cn, IT_CH_EMPRES_R) && npc_try_spell(cn, cn, SK_LETHARGY))
		{
			return 1;
		}
		if (co && ch[cn].a_hp>ch[cn].hp[5]*800 && !has_buff(cn, SK_LETHARGY) && get_tarot(cn, IT_CH_EMPRES_R) && npc_try_spell(cn, cn, SK_LETHARGY))
		{
			return 1;
		}
		if (co && ch[cn].a_hp<ch[cn].hp[5]*300 && has_buff(cn, SK_LETHARGY) && get_tarot(cn, IT_CH_EMPRES_R) && npc_try_spell(cn, cn, SK_LETHARGY))
		{
			return 1;
		}
		//if (co && ch[cn].a_end>ch[cn].end[5]*600 && !has_buff(cn, SK_RAGE) && npc_try_spell(cn, cn, SK_RAGE))
		//{
		//	return 1;
		//}
		if (co && ch[cn].a_hp>ch[cn].hp[5]*800 && !has_buff(cn, SK_IMMOLATE) && get_tarot(cn, IT_CH_JUDGE_R) && npc_try_spell(cn, cn, SK_PULSE))
		{
			return 1;
		}
		if (ch[cn].a_hp<ch[cn].hp[5]*300 && has_buff(cn, SK_IMMOLATE) && get_tarot(cn, IT_CH_JUDGE_R) && npc_try_spell(cn, cn, SK_PULSE))
		{
			return 1;
		}
		
		// Dispel - for enemy
		if (co && B_SK(cn, SK_DISPEL) && get_tarot(cn, IT_CH_HEIROPH) && !has_buff(co, SK_DISPEL2))
		{
			for (n = 0, m = 0; n<MAXBUFFS; n++)
			{
				if ((in = ch[co].spell[n])==0) continue;
				if (IS_DISPELABLE2(bu[in].temp)) m++;
			}
			if (m>1)
			{
				if (npc_try_spell(cn, co, SK_DISPEL))
				{
					return 1;
				}
			}
		}
		//
		if (co && npc_try_spell(cn, co, SK_SLOW))
		{
			return 1;
		}
		if (co && is_facing(cn,co) && npc_try_spell(cn, co, SK_WEAKEN))
		{
			return 1;
		}
		
		if (co && npc_try_spell(cn, co, SK_CURSE))
		{
			return 1;
		}
		if (co && globs->ticker>ch[cn].data[74] && npc_try_spell(cn, co, SK_GHOST))
		{
			ch[cn].data[74] = globs->ticker + TICKS * 10;
			return 1;
		}
		if ((IS_COMPANION(cn) && ch[cn].data[9] == 1))
		{
			if (co && globs->ticker>ch[cn].data[74] && npc_try_spell(cn, co, SK_SHADOW))
			{
				ch[cn].data[9] = 2;
				ch[cn].data[74] = globs->ticker + TICKS * 10;
				return 1;
			}
		}
		else
		{
			if (co && globs->ticker>ch[cn].data[74] && npc_try_spell(cn, co, SK_SHADOW))
			{
				ch[cn].data[74] = globs->ticker + TICKS * 10;
				return 1;
			}
		}
		if (co && !get_tarot(cn, IT_CH_JUDGE_R) && npc_try_spell(cn, cn, SK_PULSE))
		{
			return 1;
		}
		if (co && npc_try_spell(cn, co, SK_POISON))
		{
			return 1;
		}
		
		if (co && !is_facing(cn,co) && globs->ticker>ch[co].data[75] && npc_try_spell(cn, co, SK_LEAP))
		{
			ch[co].data[75] = globs->ticker + TICKS;
			return 1;
		}
		if (co && is_near(cn, co, PRXB_RAD-1 + ch[cn].aoe_bonus) && globs->ticker>ch[cn].data[74] && npc_try_spell(cn, co, SK_BLIND))
		{
			ch[cn].data[74] = globs->ticker + TICKS * 15;
			return 1;
		}
		if (co && is_near(cn, co, PRXW_RAD-1 + ch[cn].aoe_bonus) &&
			((!IS_PLAYER(co) && globs->ticker>ch[cn].data[74]) || (IS_PLAYER(co) && globs->ticker>ch[co].data[74])) && 
			!get_tarot(cn, IT_CH_EMPERO_R) && ch[cn].a_end>ch[cn].end[5]*200 && npc_try_spell(cn, co, SK_WARCRY))
		{
			if (IS_PLAYER(co))
				ch[co].data[74] = globs->ticker + SP_DUR_WARCRY2(M_SK(cn, SK_WARCRY))*2;
			else
				ch[cn].data[74] = globs->ticker + SP_DUR_WARCRY2(M_SK(cn, SK_WARCRY))*2;
			return 1;
		}
		if (co && is_facing(cn,co) && globs->ticker>ch[co].data[75] && npc_try_spell(cn, co, SK_CLEAVE))
		{
			ch[co].data[75] = globs->ticker + TICKS;
			return 1;
		}
		if (co && is_facing(cn,co) && globs->ticker>ch[co].data[75] && get_gear(cn, IT_WP_THEWALL) && npc_try_spell(cn, co, SK_SHIELD))
		{
			ch[co].data[75] = globs->ticker + TICKS;
			return 1;
		}
		if (co && is_facing(cn,co) && globs->ticker>ch[co].data[75] && npc_try_spell(cn, co, SK_LEAP))
		{
			ch[co].data[75] = globs->ticker + TICKS;
			return 1;
		}
		
		if ((ch[cn].a_end>=ch[cn].end[5]*950) && co && !is_facing(cn,co) && globs->ticker>ch[cn].data[74] && npc_try_spell(cn, co, SK_TAUNT))
		{
			ch[cn].data[74] = globs->ticker + TICKS * 30;
			return 1;
		}

		if (co && ch[co].armor + 5>ch[cn].weapon) // blast always if we cannot hurt him otherwise
		{
			if (globs->ticker>ch[co].data[75] && npc_try_spell(cn, co, SK_BLAST))
			{
				ch[co].data[75] = globs->ticker + TICKS;
				return 1;
			}
		}
	}

	// did we panic?
	if (ch[cn].data[78] && !ch[cn].attack_cn && !ch[cn].goto_x)
	{
		if (ch[cn].data[27]!=1)
		{
			ch[cn].goto_x = ch[cn].x + 5 - RANDOM(10);
			ch[cn].goto_y = ch[cn].y + 5 - RANDOM(10);
			return 1;
		}
		else
		{
			ch[cn].skill[SK_PERCEPT][1] = 120;
		}
	}

	// are we on protect and want to follow our master?
	if (!ch[cn].attack_cn && (co = ch[cn].data[69])!=0 && follow_driver(cn, ch[cn].data[69]))
	{
		if (abs(ch[cn].x - ch[co].y) + abs(ch[cn].y - ch[co].y)>6)
		{
			ch[cn].data[58] = 2;
		}
		else
		{
			ch[cn].data[58] = 1;
		}
		return 1;
	}

	if (!ch[cn].data[41] && !(ch[cn].data[47]>0 && ch[cn].data[47]<10))
	{
		return 0;                                     // don't scan if we don't use the information anyway
	}
	// save some work. you need to check here if no other work needs to be done!
	if (ch[cn].data[41] && ch[cn].misc_action==DR_USE && ch[cn].data[58]==1)
	{
		return 0;
	}
	if (ch[cn].data[47]>0 && ch[cn].data[47]<10 && ch[cn].misc_action==DR_PICKUP)
	{
		return 0;
	}
	if (ch[cn].data[47]>0 && ch[cn].data[47]<10 && ch[cn].misc_action==DR_USE)
	{
		return 0;
	}

	if (map[ch[cn].x + ch[cn].y * MAPX].flags & MF_INDOORS)
	{
		indoor1 = 1;
	}
	else
	{
		indoor1 = 0;
	}

	for (y = max(ch[cn].y - 7, 1); y<min(ch[cn].y + 7, MAPY - 1); y++)
	{
		for (x = max(ch[cn].x - 7, 1); x<min(ch[cn].x + 7, MAPX - 1); x++)
		{
			if ((in = map[x + y * MAPX].it)!=0)
			{
				if (map[x + y * MAPX].flags & MF_INDOORS)
				{
					indoor2 = 1;
				}
				else
				{
					indoor2 = 0;
				}

				if (it[in].temp==ch[cn].data[41])
				{
					if (it[in].temp==3079 && it[in].active) // Vantablack candles
					{
						ch[cn].misc_action  = DR_USE;
						ch[cn].misc_target1 = x;
						ch[cn].misc_target2 = y;
						ch[cn].goto_x = 0;        // cancel goto, which stems probably from patrol
						ch[cn].data[58] = 1;
						return 1;
					}
					if (it[in].temp==3311 && !it[in].active) // Greywood candles
					{
						ch[cn].misc_action  = DR_USE;
						ch[cn].misc_target1 = x;
						ch[cn].misc_target2 = y;
						ch[cn].goto_x = 0;        // cancel goto, which stems probably from patrol
						ch[cn].data[58] = 1;
						return 1;
					}
					if (!it[in].active && (globs->dlight<200 || indoor2) && it[in].temp!=3079 && it[in].temp!=3311)
					{
						ch[cn].misc_action  = DR_USE;
						ch[cn].misc_target1 = x;
						ch[cn].misc_target2 = y;
						ch[cn].goto_x = 0;        // cancel goto, which stems probably from patrol
						ch[cn].data[58] = 1;
						return 1;
					}
					if (it[in].active && globs->dlight>200 && !indoor2 && it[in].temp!=3079 && it[in].temp!=3311)
					{
						ch[cn].misc_action  = DR_USE;
						ch[cn].misc_target1 = x;
						ch[cn].misc_target2 = y;
						ch[cn].goto_x = 0;        // cancel goto, which stems probably from patrol
						ch[cn].data[58] = 1;
						return 1;
					}
				}
				if (ch[cn].data[47]>0 && ch[cn].data[47]<10 && indoor1==indoor2 && (it[in].flags & IF_TAKE) &&
				    can_go(ch[cn].x, ch[cn].y, it[in].x, it[in].y) &&
				    do_char_can_see_item(cn, in) && it[in].temp!=IT_FLO_R)  // Janitor pickup - ignore red flowers
				{
					ch[cn].misc_action  = DR_PICKUP;
					ch[cn].misc_target1 = x;
					ch[cn].misc_target2 = y;
					ch[cn].goto_x = 0;
					ch[cn].data[58] = 1;
					return 1;
				}
				if (ch[cn].data[47]>0 && ch[cn].data[47]<10 && indoor1==indoor2 && (it[in].driver==7) &&
				    can_go(ch[cn].x, ch[cn].y, it[in].x, it[in].y) &&
				    do_char_can_see_item(cn, in) && ch[cn].temp != 1454 && ch[cn].temp != 1455)
				{
					if (plr_check_target(x + y * MAPX + 1) && !map[x + y * MAPX + 1].it)
					{
						in2 = god_create_item(18);
						it[in2].carried = cn;
						ch[cn].citem = in2;

						ch[cn].misc_action  = DR_DROP;
						ch[cn].misc_target1 = x + 1;
						ch[cn].misc_target2 = y;
						ch[cn].goto_x = 0;
						ch[cn].data[58] = 1;
						return 1;
					}
				}
			}
		}
	}

	return 0;
}


// ********************************
// *        Low Prio Driver       *
// ********************************

int shiva_activate_candle(int cn, int in, int candlenum)
{
	int n;

	if (!(globs->flags & GF_NEWBS))
	{
		if (candlenum>1 && ch[cn].data[2]<BS_RC)
		{
			return 0;
		}
		ch[cn].data[2] = 0;
		if (!candlenum) { ch[cn].data[3] = 0; ch[cn].data[4] = 0; }
		chlog(cn, "Created new candle (%d), time=%d, day=%d, last day=%d", 
			candlenum, globs->mdtime, globs->mdday, ch[cn].data[0]);
	}
	else
	{
		chlog(cn, "Created new candle (%d)", candlenum);
	}

	it[in].active = 0;
	if (it[in].light[0]!=it[in].light[1] && it[in].x>0)
	{
		do_add_light(it[in].x, it[in].y, it[in].light[0] - it[in].light[1]);
	}

	fx_add_effect(6, 0, it[in].x, it[in].y, 0);
	fx_add_effect(7, 0, ch[cn].x, ch[cn].y, 0);

	do_sayx(cn, "Shirak ishagur gorweran dulak!");

	ch[cn].a_mana -= ch[cn].mana[5] * 333; // subtract 33% of mana

	// Alert players about new candle
	for (n = 1; n<MAXCHARS; n++)
	{
		if ((ch[n].flags & (CF_PLAYER | CF_USURP)) && ch[n].used==USE_ACTIVE && !(ch[n].flags & CF_NOSHOUT))
		{
			if (ch[cn].temp==CT_BSMAGE1 && (is_inline(n, 1) || (getrank(n)>= 5 && getrank(n)<=8)))
			{
				switch (candlenum)
				{
					case 0: 
						do_char_log(n, 3, "Tactician shouts: \"The northern stronghold is calming down!\"\n"); break;
					case 1: 
						do_char_log(n, 3, "Tactician shouts: \"The northern stronghold is starting to stir!\"\n"); break;
					case 2: 
						do_char_log(n, 3, "Tactician shouts: \"The northern stronghold is growing in numbers!\"\n"); break;
					case 3: 
						do_char_log(n, 3, "Tactician shouts: \"The northern stronghold is building its forces! Alert!\"\n"); break;
					case 4: 
						do_char_log(n, 3, "Tactician shouts: \"The northern stronghold is nearing full strength! Alert!\"\n"); break;
					case 5: 
						do_char_log(n, 3, "Tactician shouts: \"The northern stronghold is at full strength! Alert!\"\n"); break;
				}
			}
			else if (ch[cn].temp==CT_BSMAGE2 && (is_inline(n, 2) || (getrank(n)>=9 && getrank(n)<=13)))
			{
				switch (candlenum)
				{
					case 0: 
						do_char_log(n, 3, "Tactician shouts: \"The central stronghold is calming down!\"\n"); break;
					case 1: 
						do_char_log(n, 3, "Tactician shouts: \"The central stronghold is starting to stir!\"\n"); break;
					case 2: 
						do_char_log(n, 3, "Tactician shouts: \"The central stronghold is growing in numbers!\"\n"); break;
					case 3: 
						do_char_log(n, 3, "Tactician shouts: \"The central stronghold is building its forces! Alert!\"\n"); break;
					case 4: 
						do_char_log(n, 3, "Tactician shouts: \"The central stronghold is nearing full strength! Alert!\"\n"); break;
					case 5: 
						do_char_log(n, 3, "Tactician shouts: \"The central stronghold is at full strength! Alert!\"\n"); break;
				}
			}
			else if (ch[cn].temp==CT_BSMAGE3 && (is_inline(n, 3) || getrank(n)>=14))
			{
				switch (candlenum)
				{
					case 0: 
						do_char_log(n, 3, "Tactician shouts: \"The southern stronghold has calmed down!\"\n"); break;
					case 1: 
						do_char_log(n, 3, "Tactician shouts: \"The southern stronghold has started to stir!\"\n"); break;
					case 2: 
						do_char_log(n, 3, "Tactician shouts: \"The southern stronghold is growing in numbers!\"\n"); break;
					case 3: 
						do_char_log(n, 3, "Tactician shouts: \"The southern stronghold is building its forces! Alert!\"\n"); break;
					case 4: 
						do_char_log(n, 3, "Tactician shouts: \"The southern stronghold is nearing full strength! Alert!\"\n"); break;
					case 5: 
						do_char_log(n, 3, "Tactician shouts: \"The southern stronghold is at full strength! Alert!\"\n"); break;
				}
			}
		}
	}
	
	return 1;
}

// grave looting and friends

// does not check correctly for two-handed weapons
int npc_check_placement(int cn, int in, int n)
{
	switch(n)
	{
	case    WN_HEAD:
		if (!(it[in].placement & PL_HEAD))
		{
			return 0;
		}
		else
		{
			break;
		}
	case    WN_NECK:
		if (!(it[in].placement & PL_NECK))
		{
			return 0;
		}
		else
		{
			break;
		}
	case    WN_BODY:
		if (!(it[in].placement & PL_BODY))
		{
			return 0;
		}
		else
		{
			break;
		}
	case    WN_ARMS:
		if (!(it[in].placement & PL_ARMS))
		{
			return 0;
		}
		else
		{
			break;
		}
	case    WN_BELT:
		if (!(it[in].placement & PL_BELT))
		{
			return 0;
		}
		else
		{
			break;
		}
	case    WN_FEET:
		if (!(it[in].placement & PL_FEET))
		{
			return 0;
		}
		else
		{
			break;
		}
	case    WN_LHAND:
		if (!(it[in].placement & PL_SHIELD))
		{
			return 0;
		}
		else if ((in = ch[cn].worn[WN_RHAND])!=0 && IS_TWOHAND(in))
		{
			return 0;
		}
		else
		{
			break;
		}
	case    WN_RHAND:
		if (!(it[in].placement & PL_WEAPON) && !((it[in].flags & IF_OF_SHIELD) && IS_ARCHTEMPLAR(cn)))
		{
			return 0;
		}
		else if (IS_TWOHAND(in) && ch[cn].worn[WN_LHAND])
		{
			return 0;
		}
		else
		{
			break;
		}
	case    WN_CLOAK:
		if (!(it[in].placement & PL_CLOAK))
		{
			return 0;
		}
		else
		{
			break;
		}
	case    WN_RRING:
	case    WN_LRING:
		if (!(it[in].placement & PL_RING))
		{
			return 0;
		}
		else if (n==WN_RRING && (in = ch[cn].worn[WN_LRING])!=0 && IS_TWOHAND(in))
		{
			return 0;
		}
		else if (n==WN_LRING && (in = ch[cn].worn[WN_RRING])!=0 && IS_TWOHAND(in))
		{
			return 0;
		}
		else
		{
			break;
		}
	default:
		return 0;
	}

	return 1;
}

int npc_can_wear_item(int cn, int in)
{
	int m;

	if (in & 0x80000000)
	{
		return 0;
	}

	for (m = 0; m<5; m++)
	{
		if ((unsigned char)it[in].attrib[m][2] > B_AT(cn, m))
		{
			return 0;
		}
	}

	for (m = 0; m<MAXSKILL; m++)
	{
		if ((unsigned char)it[in].skill[m][2] > B_SK(cn, m))
		{
			return 0;
		}
	}

	if (it[in].hp[2]>ch[cn].hp[0])
	{
		return 0;
	}

	if (it[in].end[2]>ch[cn].end[0])
	{
		return 0;
	}

	if (it[in].mana[2]>ch[cn].mana[0])
	{
		return 0;
	}

	return 1;
}

int npc_item_value(int cn, int in)
{
	int score = 0, n;

	if (ch[cn].worn[WN_RHAND]==0 && in==0) // Should prevent unarmed npcs from using a bad weapon
	{
		score += M_SK(cn, SK_HAND) * 5;
	}

	for (n = 0; n<5; n++)
	{
		score += it[in].attrib[n][0] * 10;
	}
	for (n = 0; n<MAXSKILL; n++)
	{
		score += it[in].skill[n][0] * 5;
	}

	score += it[in].value / 10;

	score += it[in].weapon[0] * 50;
	score += it[in].armor[0] * 50;

	score -= it[in].damage_state;

	return(score);
}

int npc_want_item(int cn, int in)
{
	if (ch[cn].item[38])
	{
		return 0;             // hack: dont take more stuff if inventory is almost full

	}
	if (ch[cn].citem)
	{
		chlog(cn, "have %s in citem", it[in].name);
		if (do_store_item(cn)==-1)
		{
			it[ch[cn].citem].used = USE_EMPTY;
			ch[cn].citem = 0;
		}
	}

	if (it[in].temp==IT_SPOT || it[in].temp==IT_GPOT || it[in].temp==IT_RPOT) // Gpot or Rpot
	{
		ch[cn].citem = in;
		it[in].carried = cn;
		do_store_item(cn);
		return 1;
	}

	return 0;
}

int npc_equip_item(int cn, int in)
{
	int n;

	if (ch[cn].citem)
	{
		chlog(cn, "have %s in citem", it[in].name);
		if (do_store_item(cn)==-1)
		{
			it[ch[cn].citem].used = USE_EMPTY;
			ch[cn].citem = 0;
		}
	}

	for (n = 0; n<20; n++)
	{
		if ((!ch[cn].worn[n] && (n==1||n==4||n==10||n==11)) || npc_item_value(cn, in)>npc_item_value(cn, ch[cn].worn[n]))
		{
			if (npc_check_placement(cn, in, n))
			{
				if (npc_can_wear_item(cn, in))
				{
					chlog(cn, "now wearing %s", it[in].name);

					// remove old item if any
					if (ch[cn].worn[n])
					{
						chlog(cn, "storing item");
						ch[cn].citem = ch[cn].worn[n];
						if (do_store_item(cn)==-1)
						{
							return 0;                     // stop looting if our backpack is full
						}
					}

					ch[cn].worn[n] = in;
					it[in].carried = cn;

					do_update_char(cn);
					return 1;
				}
			}
		}
	}

	return 0;
}

int npc_loot_grave(int cn, int in)
{
	int co, n;

	if (abs(ch[cn].x - it[in].x) + abs(ch[cn].y - it[in].y)>1 || drv_dcoor2dir(it[in].x - ch[cn].x, it[in].y - ch[cn].y)!=ch[cn].dir)
	{
		ch[cn].misc_action  = DR_USE;
		ch[cn].misc_target1 = it[in].x;
		ch[cn].misc_target2 = it[in].y;
		return 1;
	}

	co = it[in].data[0];

	for (n = 0; n<20; n++)
	{
		if ((in = ch[co].worn[n]))
		{
			if (npc_equip_item(cn, in))
			{
				chlog(cn, "got %s from %s's grave", it[in].name, ch[co].name);
				ch[co].worn[n] = 0;
				return 1;
			}
		}
	}
	for (n = 0; n<MAXITEMS; n++)
	{
		if (in = ch[co].item[n])
		{
			if (npc_equip_item(cn, in))
			{
				chlog(cn, "got %s from %s's grave", it[in].name, ch[co].name);
				ch[co].item[n] = 0;
				return 1;
			}
			if (npc_want_item(cn, in))
			{
				chlog(cn, "got %s from %s's grave", it[in].name, ch[co].name);
				ch[co].item[n] = 0;
				return 1;
			}
		}
	}

	if (ch[co].gold)
	{
		chlog(cn, "got %.2fG from %s's grave", ch[co].gold / 100.0, ch[co].name);
		ch[cn].gold += ch[co].gold;
		ch[co].gold  = 0;
		return 1;
	}

	return 0;
}

int npc_already_searched_grave(int cn, int in)
{
	int n;

	for (n = 0; n<160; n += sizeof(int))
	{
		if (*(int*)(ch[cn].text[9] + n)==in)
		{
			return 1;
		}
	}

	return 0;
}

void npc_add_searched_grave(int cn, int in)
{
	memmove(ch[cn].text[9] + sizeof(int), ch[cn].text[9], sizeof(ch[cn].text[9]) - sizeof(int));

	*(int*)(ch[cn].text[9]) = in;
}

int npc_grave_logic(int cn)
{
	int x, y, in;

	for (y = max(ch[cn].y - 8, 1); y<min(ch[cn].y + 8, MAPY - 1); y++)
	{
		for (x = max(ch[cn].x - 8, 1); x<min(ch[cn].x + 8, MAPX - 1); x++)
		{
			if ((in = map[x + y * MAPX].it)!=0)
			{
				if (it[in].temp==IT_TOMBSTONE &&
				    can_go(ch[cn].x, ch[cn].y, it[in].x, it[in].y) &&
				    do_char_can_see_item(cn, in) &&
				    !npc_already_searched_grave(cn, in))
				{
					if (!npc_loot_grave(cn, in))
					{
						npc_add_searched_grave(cn, in);
						ch[cn].flags &= ~CF_ISLOOTING;
					}
					else
					{
						ch[cn].flags |= CF_ISLOOTING;
					}
					return 1;
				}
			}
		}
	}
	return 0;
}

void npc_driver_low(int cn)
{
	int n, x, y, j, m, in, panic, co;
	int alone = 1;

	if (ch[cn].data[25])    // check for special driver routine
	{
		switch(ch[cn].data[25])
		{
		case  1:
			npc_stunrun_low(cn);
			return;
		case  2:
			npc_cityattack_low(cn);
			return;
		case  3:
			if (ch[cn].data[26]==6) npc_archeologist_low(cn);
			else npc_malte_low(cn);
			return;
		case  4:
			npc_shiva_low(cn);
			return;
		default:
			chlog(cn, "unknown special driver %d", ch[cn].data[25]);
			break;
		}
		return;
	}

	if (ch[cn].last_action==ERR_SUCCESS)
	{
		ch[cn].data[36] = 0;                                    // reset frust with successful action
	}
	else if (ch[cn].last_action==ERR_FAILED)
	{
		ch[cn].data[36]++;                                      // increase frust with failed action
	}
	
	if (ch[cn].temp==CT_ANNOU1 || ch[cn].temp==CT_ANNOU2 || ch[cn].temp==CT_ANNOU3 || 
		ch[cn].temp==CT_ANNOU4 || ch[cn].temp==CT_ANNOU5)
	{
		colosseum_driver(cn);
	}
	
	if (ch[cn].temp==CT_PANDIUM)
	{
		pandium_driver(cn);
	}
	
	if (ch[cn].temp==CT_LAB20_KEEP)
	{
		gatekeeper_driver(cn);
	}
	
	// are we supposed to loot graves?
	if (ch[cn].alignment<0 && (globs->flags & GF_LOOTING) &&
	    ((cn & 15)==(globs->ticker & 15) || (ch[cn].flags & CF_ISLOOTING)) &&
	    !IS_COMP_TEMP(cn) && !(ch[cn].temp>=176 && ch[cn].temp<=179) && !(ch[cn].citem))
	{
		if (npc_grave_logic(cn))
		{
			return;
		}
	}

	// did someone call help? - high prio
	if (ch[cn].data[55] && ch[cn].data[55] + TICKS * 120>globs->ticker && ch[cn].data[54] && ch[cn].data[27]!=1)
	{
		m = ch[cn].data[54];
		ch[cn].goto_x = m % MAPX + get_frust_x_off(globs->ticker);
		ch[cn].goto_y = m / MAPX + get_frust_y_off(globs->ticker);
		ch[cn].data[58] = 2;
		return;
	}

	// go to last known enemy position and stay there for up to 30 seconds
	if (ch[cn].data[77] && ch[cn].data[77] + TICKS * 30>globs->ticker && ch[cn].data[27]!=1)
	{
		m = ch[cn].data[76];
		ch[cn].goto_x = m % MAPX + get_frust_x_off(ch[cn].data[36]);
		ch[cn].goto_y = m / MAPX + get_frust_y_off(ch[cn].data[36]);
		return;
	}

	if (ch[cn].a_hp<ch[cn].hp[5] * 750) // we're hurt: rest
	{
		return;
	}
	
	j = 9; // check within 10 spaces for a player
	if (ch[cn].data[26]==12 || ch[cn].data[26]==13 || ch[cn].data[26]==14) j = 24; // liz emp checks farther

	for (y = max(0, ch[cn].y - j); y<min(MAPY, ch[cn].y + j+1); y++)
	{
		m = y * MAPX;
		for (x = max(0, ch[cn].x - j); x<min(MAPX, ch[cn].x + j+1); x++)
		if ((co = map[x + m].ch)!=0) if (co!=cn && ch[co].flags&CF_PLAYER)
		if ((map[ch[cn].x + ch[cn].y * MAPX].flags & MF_INDOORS) && (map[ch[co].x + ch[co].y * MAPX].flags & MF_INDOORS))
		{
			alone = 0;
		}
	}
	
	if (alone && (ch[cn].data[26]==12 || ch[cn].data[26]==14)) // npc_lab_lord_driver / npc_shivab_driver
		ch[cn].data[2] = 0;
	
	if (alone && ch[cn].data[26]==13) // npc_emperor_driver
	{
		ch[cn].data[2] = 0;
		ch[cn].data[3] = 0;
		ch[cn].data[4] = 0;
		
		B_SK(cn, SK_SLOW) = 0;
		B_SK(cn, SK_CURSE) = 0;
		B_SK(cn, SK_ZEPHYR) = 0;
	}
	
	if (alone) for (n = 20; n<24; n++) // close door, medium prio -- only if no players nearby
	{
		if ((m = ch[cn].data[n])!=0)
		{
			// check if the door is free:
			if (!map[m].ch && !map[m].to_ch &&
			    !map[m + 1].ch && !map[m + 1].to_ch &&
			    !map[m - 1].ch && !map[m - 1].to_ch &&
			    !map[m + MAPX].ch && !map[m + MAPX].to_ch &&
			    !map[m - MAPX].ch && !map[m - MAPX].to_ch)
			{
				in = map[m].it;
				if (in && it[in].active)
				{
					ch[cn].misc_action  = DR_USE;
					ch[cn].misc_target1 = m % MAPX;
					ch[cn].misc_target2 = m / MAPX;
					ch[cn].data[58] = 1;
					return;
				}
			}
		}
	}

	for (n = 32; n<36; n++) // activate light, medium prio
	{
		if ((m = ch[cn].data[n])!=0 && m<MAPX * MAPY)
		{
			in = map[m].it;
			if (in && !it[in].active)
			{
				ch[cn].misc_action  = DR_USE;
				ch[cn].misc_target1 = m % MAPX;
				ch[cn].misc_target2 = m / MAPX;
				ch[cn].data[58] = 1;
				return;
			}
		}
	}

	if (ch[cn].data[10])    // patrol, low
	{
		n = ch[cn].data[19];
		if (n<10 || n>18)
		{
			ch[cn].data[19] = n = 10;
		}

		if (ch[cn].data[57]>globs->ticker)
		{
			return;
		}

		m = ch[cn].data[n];
		x = m % MAPX + get_frust_x_off(ch[cn].data[36]);
		y = m / MAPX + get_frust_y_off(ch[cn].data[36]);
		if (ch[cn].data[36]>20 || abs(ch[cn].x - x) + abs(ch[cn].y - y)<4)
		{
			if (ch[cn].data[36]<=20)
			{
				if (ch[cn].data[79])
				{
					ch[cn].data[57] = globs->ticker + ch[cn].data[79];
				}
			}
			if (ch[cn].data[36]>20 && (map[ch[cn].x + ch[cn].y * MAPX].flags & MF_MOVEBLOCK)) // unstick the stuck
			{
				fx_add_effect(12, 0, ch[cn].x, ch[cn].y, 0);
				reset_char(ch[cn].temp);
			}

			n++;
			if (n>18)
			{
				n = 10;
			}
			if (ch[cn].data[n]==0)
			{
				n = 10;
			}

			ch[cn].data[19] = n;
			ch[cn].data[36] = 0;

			return;
		}
		ch[cn].goto_x = x;
		ch[cn].goto_y = y;
		ch[cn].data[58] = 0;
		return;
	}
	if (ch[cn].data[60] && ch[cn].data[27]!=1)   //random walk, low
	{
		ch[cn].data[58] = 0;
		if (ch[cn].data[61]<1)
		{
			ch[cn].data[61] = ch[cn].data[60];

			for (panic = 0; panic<5; panic++)
			{
				x = ch[cn].x - 5 + RANDOM(11);
				y = ch[cn].y - 5 + RANDOM(11);

				if (x<1 || x>=MAPX || y<1 || y>MAPY)
				{
					continue;
				}

				if (ch[cn].data[73])   // too far away from origin?
				{
					int xo, yo;

					xo = ch[cn].data[29] % MAPX;
					yo = ch[cn].data[29] / MAPX;

					if (abs(x - xo) + abs(y - yo)>ch[cn].data[73])
					{
						if (plr_check_target(xo + yo * MAPX))
						{
							ch[cn].goto_x = xo;
							ch[cn].goto_y = yo;
							return;
						}
						else if (plr_check_target(xo + 1 + yo * MAPX))
						{
							ch[cn].goto_x = xo + 1;
							ch[cn].goto_y = yo;
							return;
						}
						else if (plr_check_target(xo - 1 + yo * MAPX))
						{
							ch[cn].goto_x = xo - 1;
							ch[cn].goto_y = yo;
							return;
						}
						else if (plr_check_target(xo + yo * MAPX + MAPX))
						{
							ch[cn].goto_x = xo;
							ch[cn].goto_y = yo + 1;
							return;
						}
						else if (plr_check_target(xo + yo * MAPX - MAPX))
						{
							ch[cn].goto_x = xo;
							ch[cn].goto_y = yo - 1;
							return;
						}
						else
						{
							continue;
						}
					}
				}

				if (!plr_check_target(x + y * MAPX))
				{
					continue;
				}
				if (!can_go(ch[cn].x, ch[cn].y, x, y))
				{
					continue;
				}
				break;
			}

			if (panic==5)
			{
				return;
			}
			
			ch[cn].goto_x = x;
			ch[cn].goto_y = y;
			return;

		}
		else
		{
			ch[cn].data[61]--;
		}

		return;
	}

	if (ch[cn].data[29] && ch[cn].data[27]!=1)    // resting position, lowest prio
	{
		// Turn off toggles when resting
		if (ch[cn].a_mana<ch[cn].mana[5]*400 && has_buff(cn, SK_LETHARGY) && !get_tarot(cn, IT_CH_EMPRES_R) && npc_try_spell(cn, cn, SK_LETHARGY))
		{
			return 1;
		}
		if (co && ch[cn].a_hp<ch[cn].hp[5]*600 && has_buff(cn, SK_LETHARGY) && get_tarot(cn, IT_CH_EMPRES_R) && npc_try_spell(cn, cn, SK_LETHARGY))
		{
			return 1;
		}
		//if (ch[cn].a_end<ch[cn].end[5]*400 && has_buff(cn, SK_RAGE) && npc_try_spell(cn, cn, SK_RAGE))
		//{
		//	return 1;
		//}
		if (ch[cn].a_hp<ch[cn].hp[5]*600 && has_buff(cn, SK_IMMOLATE) && get_tarot(cn, IT_CH_JUDGE_R) && npc_try_spell(cn, cn, SK_PULSE))
		{
			return 1;
		}
		
		npc_activate_rings(cn, 0);
		npc_wedge_doors(ch[cn].data[26], 0);
		ch[cn].attack_cn = 0;
		
		m = ch[cn].data[29];
		x = m % MAPX + get_frust_x_off(ch[cn].data[36]);
		y = m / MAPX + get_frust_y_off(ch[cn].data[36]);

		ch[cn].data[58] = 0;
		if (ch[cn].x!=x || ch[cn].y!=y)
		{
			ch[cn].goto_x = x;
			ch[cn].goto_y = y;
			if (!(ch[cn].flags & CF_MERCHANT) && ch[cn].temp!=CT_ANNOU1 && ch[cn].temp!=CT_ANNOU2 && 
				       ch[cn].temp!=CT_ANNOU3 && ch[cn].temp!=CT_ANNOU4 && ch[cn].temp!=CT_ANNOU5)
			{
				ch[cn].data[9]++;
				if (ch[cn].data[9]>TICKS*60*5) // We haven't been able to return home for the past 5 minutes.
				{
					int xo, yo;
					
					xo = ch[cn].x;
					yo = ch[cn].y;

					if (god_transfer_char(cn, x, y))
					{
						if (!(ch[cn].flags & CF_INVISIBLE))
						{
							fx_add_effect(12, 0, xo, yo, 0);
							fx_add_effect(12, 0, ch[cn].x, ch[cn].y, 0);
						}
					}
					ch[cn].status = 0;
					ch[cn].attack_cn = 0;
					ch[cn].skill_nr  = 0;
					ch[cn].goto_x = 0;
					ch[cn].use_nr = 0;
					ch[cn].misc_action = 0;
					ch[cn].dir = DX_DOWN;
					
					chlog(cn, "Couldn't get home, forced Recall");
					ch[cn].data[9]=0;
				}
			}
			return;
		}
		
		if (!(ch[cn].flags & CF_MERCHANT) && ch[cn].temp!=CT_ANNOU1 && ch[cn].temp!=CT_ANNOU2 && 
				   ch[cn].temp!=CT_ANNOU3 && ch[cn].temp!=CT_ANNOU4 && ch[cn].temp!=CT_ANNOU5) 
			ch[cn].data[9]=0;

		if (ch[cn].dir!=ch[cn].data[30])
		{
			ch[cn].misc_action = DR_TURN;

			switch(ch[cn].data[30])
			{
			case DX_UP:
				ch[cn].misc_target1 = x;
				ch[cn].misc_target2 = y - 1;
				break;
			case DX_DOWN:
				ch[cn].misc_target1 = x;
				ch[cn].misc_target2 = y + 1;
				break;
			case DX_LEFT:
				ch[cn].misc_target1 = x - 1;
				ch[cn].misc_target2 = y;
				break;
			case DX_RIGHT:
				ch[cn].misc_target1 = x + 1;
				ch[cn].misc_target2 = y;
				break;
			case DX_LEFTUP:
				ch[cn].misc_target1 = x - 1;
				ch[cn].misc_target2 = y - 1;
				break;
			case DX_LEFTDOWN:
				ch[cn].misc_target1 = x - 1;
				ch[cn].misc_target2 = y + 1;
				break;
			case DX_RIGHTUP:
				ch[cn].misc_target1 = x + 1;
				ch[cn].misc_target2 = y - 1;
				break;
			case DX_RIGHTDOWN:
				ch[cn].misc_target1 = x + 1;
				ch[cn].misc_target2 = y + 1;
				break;
			default:
				ch[cn].misc_action = DR_IDLE;
				break;
			}
			return;
		}
		if (ch[cn].temp==1498||ch[cn].temp==1499) // Enforcer reset
			npc_remove_all_enemies(cn);
	}

	// reset talked-to list
	if (ch[cn].data[67] + (TICKS * 60 * 5)<globs->ticker)
	{
		if (ch[cn].data[37])
		{
			for (n = 37; n<41; n++)
			{
				ch[cn].data[n] = -1;                            // hope we never have a character nr 1!
			}
		}
		ch[cn].data[67] = globs->ticker;
	}
	
	if (ch[cn].temp==CT_BSMAGE1 || ch[cn].temp==CT_BSMAGE2 || ch[cn].temp==CT_BSMAGE3)
	{
		if (globs->flags & GF_NEWBS)
			stronghold_mage_driver_ver2(cn);
		else
			stronghold_mage_driver(cn);
	}
}

void update_shop(int cn)
{
	int n, m, x, z, in, temp;
	int sale[10];

	for (n = 0; n<10; n++) // data[9] is zero fsr
	{
		sale[n] = ch[cn].data[n];
	}

	// check if we have free space (at least 10)
	do_sort(cn, "v");
	for (n = m = x = 0; n<40; n++)
	{
		if (!(in = ch[cn].item[n]))
		{
			m++;
		}
		else
		{
			for (z = 0; z<10; z++)
			{
				if (it[in].temp==sale[z])
				{
					sale[z] = 0;
					break;
				}
			}
			if (z==10)
			{
				x = n;
			}
		}
	}
	if (m<2 && ch[cn].item[x])
	{
		in = ch[cn].item[x];

		if (RANDOM(2) && (it[in].flags & IF_DONATE))
		{
			god_donate_item(in, 0);
		}
		else
		{
			it[in].used = USE_EMPTY;
		}
		ch[cn].item[x] = 0;
	}

	// check if our store is complete
	for (n = 0; n<10; n++)
	{
		temp = sale[n];
		if (!temp)
		{
			continue;
		}
		in = god_create_item(temp);
		if (in)
		{
			if (!god_give_char(in, cn))
			{
				it[in].used = USE_EMPTY;
			}
		}
	}

	// small-repair all items (set current max_damage and age to zero)
	// junk all items needing serious repair
	for (n = 0; n<40; n++)
	{
		if ((in = ch[cn].item[n])!=0)
		{
			if ((it[in].damage_state || (it[in].flags & IF_SHOPDESTROY)) && ch[cn].temp != CT_NULLAN)
			{
				it[in].used = USE_EMPTY;
				ch[cn].item[n] = 0;
			//	ch[cn].item_lock[n] = 0;
			}
			else
			{
				it[in].current_damage = 0;
				it[in].current_age[0] = 0;
				it[in].current_age[1] = 0;
				if (it[in].stack > 1)
				{
					it[in].value = it[in].value / it[in].stack;
					it[in].stack = 1;
				}
			}
		}
	}
	do_sort(cn, "v");
}

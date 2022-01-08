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
			1: Grolmy
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
			 15 : Sea Grolmy - used to check between this and normal grolmy
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

int npc_add_enemy(int cn, int co, int always)
{
	int n, idx, d1, d2, cc;

	// don't attack anyone of the same group. Never, never, never.
	if (ch[cn].data[CHD_GROUP]==ch[co].data[CHD_GROUP])
	{
		return 0;
	}
	
	// Group check for GCs
	if ((!IS_PLAYER(cn) && IS_SANECHAR(cc = ch[cn].data[CHD_MASTER]) && ch[cc].data[CHD_GROUP]==ch[co].data[CHD_GROUP]) || 
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
	
	if (points2rank(ch[cn].points_tot)<20)
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

	if (!do_char_can_see(cn, co))    // we have been attacked but cannot see the attacker
	{
		ch[cn].data[78] = globs->ticker + TICKS * 30;
		return 1;
	}

	// fight back when attacked - all NPCs do this:
	if (!IS_PLAYER_COMP(cn) || (IS_PLAYER_COMP(cn) && (ch[cn].data[1]!=1 || ch[cn].a_hp<ch[cn].hp[5] * 600))) // passive gc check - only fight back if at risk of dying
	{
		if (npc_add_enemy(cn, co, 1))
		{
			if (!(ch[co].flags & CF_SILENCE))
			{
				npc_saytext_n(cn, 1, ch[co].name);
			}
			chlog(cn, "Added %s to kill list for attacking me", ch[co].name);
		}
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

	if (!do_char_can_see(cn, co))
	{
		return 1;                     // processed it: we cannot see the defender, so ignore it
	}
	if (!do_char_can_see(cn, cc))
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
			if (!(ch[c2].flags & CF_SILENCE))
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
				if (!(ch[cc].flags & CF_SILENCE))
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
				if (!(ch[co].flags & CF_SILENCE))
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
				if (!(ch[cc].flags & CF_SILENCE))
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
				if ((IS_COMPANION(co) && !(ch[ch[co].data[CHD_MASTER]].flags & CF_SILENCE)) || !(ch[co].flags & CF_SILENCE))
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
				if (!(ch[cc].flags & CF_SILENCE))
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
				if (!(ch[co].flags & CF_SILENCE))
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

int convert_skill_for_group(int co, int nr)
{
	// Revert to original values if changed
	if (nr == SK_POISON)   nr = SK_TAUNT; 	// Poison   -> Taunt
	if (nr == SK_CONCEN)   nr = SK_TAUNT; 	// Concentr -> Taunt
	if (nr == SK_MSHIELD)  nr = SK_ENHANCE; // MShield  -> Enhance
	if (nr == SK_WEAKEN)   nr = SK_SLOW;    // Weaken   -> Slow
	if (nr == SK_SURROUND) nr = SK_CURSE;   // Surround -> Curse
	if (nr == SK_STAFF)    nr = SK_SHIELD;  // Staff    -> Shield
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
		if (nr == SK_TAUNT)  nr = SK_CONCEN; // Taunt -> Concentr
		if (nr == SK_SHIELD) nr = SK_STAFF;  // Shiel -> Staff
		if (nr == SK_IMMUN)  nr = SK_DISPEL; // Immun -> Dispel
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
		if (nr == SK_POISON  && B_SK(co, SK_POISON))  nr = SK_CONCEN;
		if (nr == SK_SLOW    && B_SK(co, SK_SLOW))    nr = SK_WEAKEN;
		if (nr == SK_CURSE   && B_SK(co, SK_CURSE))   nr = SK_SURROUND;
		if (nr == SK_SHIELD  && B_SK(co, SK_SHIELD))  nr = SK_STAFF;
		if (nr == SK_IMMUN   && B_SK(co, SK_IMMUN))   nr = SK_DISPEL;
	}
	 
	return nr;
}

int npc_give(int cn, int co, int in, int money)
{
	int nr, ar, canlearn = 1;
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
		 || ( nr == SK_SHADOW && !IS_SEYA_OR_SUMM(co) )
		 || ( nr == SK_ZEPHYR && !IS_SEYA_OR_WARR(co) )
		 || ( nr == SK_RAGE && !IS_SEYA_OR_BRAV(co) ))
			canlearn = 0;
		
		// Seyan'du can learn any arch skill, but only one!
		if ((nr == SK_WARCRY || nr == SK_PULSE || nr == SK_LEAP || 
			nr == SK_LETHARGY || nr == SK_SHADOW || nr == SK_ZEPHYR || 
			nr == SK_RAGE) && IS_SEYAN_DU(co) && 
			(B_SK(co, SK_WARCRY) || B_SK(co, SK_LEAP) || B_SK(co, SK_SHADOW) ||
			 B_SK(co, SK_LETHARGY) || B_SK(co, SK_PULSE) || B_SK(co, SK_ZEPHYR ||
			 B_SK(co, SK_RAGE))))
			canlearn = 0;
	}

	if (in && (
		ch[cn].data[49]==it[in].temp 
		|| 
		(ch[cn].temp==CT_OSIRIS && it[in].temp==MCT_CONTRACT)
		|| 
		(ch[cn].temp==CT_TACTICIAN && 
		(it[in].temp==IT_BS_CAN1 || it[in].temp==IT_BS_CAN2 || it[in].temp==IT_BS_CAN3)) 
		||
		(ch[cn].temp==CT_BISHOP && (
		(it[in].temp>=IT_CH_FOOL && it[in].temp<=IT_CH_WORLD) || 
		(it[in].temp>=IT_CH_FOOL_R && it[in].temp<=IT_CH_WORLD_R)))
		||
		(ch[cn].temp==CT_PRIEST && it[in].temp==IT_TW_SINBIND && it[in].data[1]==1)
		||
		(ch[cn].temp==CT_HERBCOLL && 
		(it[in].temp==IT_HERBA || it[in].temp==IT_HERBB || it[in].temp==IT_HERBC || it[in].temp==IT_HERBD))
		||
		(ch[cn].temp==CT_HERBCOLL2 && 
		(it[in].temp==IT_HERBE))
		) && canlearn)
	{
		// Assure the player has inventory space before we do anything
		if (ch[cn].data[66] || ch[cn].data[50]==50)
		{
			for (n = 0; n<40; n++)
			{
				// Find an empty inventory slot
				if (!ch[co].item[n])
				{
					break;
				}
			}
			if (n==40)
			{
				do_char_log(co, 0, "You get the feeling you should clear some space in your backpack first.\n");
				return 0;
			}
		}
		
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
			god_take_from_char(in, cn);
			it[in].used = USE_EMPTY;  // silently destroy the item.
			do_sayx(cn, "Ah, a black candle! Great work, %s! Now we may see peace for a while...", ch[co].name);
			do_area_log(cn, 0, ch[cn].x, ch[cn].y, 1, "The Tactician was impressed by %s's deed.\n", ch[co].name);
			ch[co].misc_action = DR_IDLE;
			return 0;
		}
		else if (ch[cn].temp==CT_BISHOP && (
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
			remove_all_spells(co);
			ch[co].misc_action = DR_IDLE;
			return 0;
		}
		else if (ch[cn].temp==CT_PRIEST && it[in].temp==IT_TW_SINBIND && it[in].data[1]==1)
		{
			if (((in2 = ch[co].worn[WN_RRING]) && it[in2].temp!=IT_TW_SINBIND) || ch[co].worn[WN_LRING])
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
			if ((in2 = ch[co].worn[WN_RRING]) && it[in2].temp==IT_TW_SINBIND)
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
			remove_all_spells(co);
			ch[co].misc_action = DR_IDLE;
			return 0;
		}
		else if ((ch[cn].temp==CT_HERBCOLL && 
			(it[in].temp==IT_HERBA || it[in].temp==IT_HERBB || it[in].temp==IT_HERBC || it[in].temp==IT_HERBD)) || 
			(ch[cn].temp==CT_HERBCOLL2 && it[in].temp==IT_HERBE))
		{
			if (ch[cn].temp==CT_HERBCOLL && points2rank(ch[co].points_tot)<8)
			{
				do_sayx(cn, "I'm sorry %s, I don't have a use for weeds!", ch[co].name);
				god_take_from_char(in, cn);
				god_give_char(in, co);
				do_char_log(co, 1, "%s did not accept the %s.\n", ch[cn].reference, it[in].name);
				return 0;
			}
			if (it[in].temp==IT_HERBA)			money =  10000;
			else if (it[in].temp==IT_HERBB)	money =  15000;
			else if (it[in].temp==IT_HERBC)	money =  25000;
			else if (it[in].temp==IT_HERBD)	money =  60000;
			else								money = 120000;
			nr = money;
			if (ch[cn].temp==CT_HERBCOLL2)
				do_sayx(cn, "Here'sss your payment, and a bit of knowledge.");
			else
				do_sayx(cn, "Here's your payment, and a bit of knowledge.");
			god_take_from_char(in, cn);
			it[in].used = USE_EMPTY;
			ch[co].gold += money;
			do_char_log(co, 2, "You received %dG %dS.\n", money / 100, money % 100);
			do_give_exp(co, nr, 0, -1);
			// <group rewards>
			for (n = 1; n<MAXCHARS; n++)
			{
				if (ch[n].used==USE_EMPTY || !(ch[n].flags & (CF_PLAYER | CF_USURP))) continue;
				if (isgroup(n, co) && isgroup(co, n) && isnearby(co, n))
				{
					do_give_exp(n, nr, 0, -1);
				}
			}
			// </group rewards>
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
			nr = convert_skill_for_group(co, nr);

			if (nr==SK_TAUNT && IS_SEYAN_DU(co))
				do_sayx(cn, "Bring me the item again to learn Poison, %s!", ch[co].name);
			if (nr==SK_POISON && IS_SEYAN_DU(co))
				do_sayx(cn, "Bring me the item once more to learn Concentrate, %s!", ch[co].name);
			if (nr==SK_SLOW && IS_SEYAN_DU(co))
				do_sayx(cn, "Bring me the item again to learn Weaken, %s!", ch[co].name);
			if (nr==SK_CURSE && IS_SEYAN_DU(co))
				do_sayx(cn, "Bring me the item again to learn Surround Hit, %s!", ch[co].name);
			if (nr==SK_SHIELD && IS_SEYAN_DU(co))
				do_sayx(cn, "Bring me the item again to learn Staff, %s!", ch[co].name);
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
						do_give_exp(co, (nr/4), 0, -1);
					}
					god_take_from_char(in, cn);
					it[in].used = USE_EMPTY;  // silently destroy the item.
				}
				else
				{
					do_sayx(cn, "You will need a lockpick, and I happen to have a spare. Take it, please.");
					god_take_from_char(in, cn);
					it[in].used = USE_EMPTY;
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
						do_give_exp(co, nr, 0, -1);
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
						if ((nr = ch[cn].data[51])!=0) do_give_exp(n, nr/max(1,div), 0, -1);
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
						do_give_exp(co, nr, 0, -1);
					}
					god_take_from_char(in, cn);
					it[in].used = USE_EMPTY;
					
					// <group rewards>
					for (n = 1; n<MAXCHARS; n++)
					{
						if (ch[n].used==USE_EMPTY || !(ch[n].flags & (CF_PLAYER | CF_USURP))) continue;
						if (ch[n].temple_x!=HOME_START_X) continue;
						if (isgroup(n, co) && isgroup(co, n) && isnearby(co, n))
						{
							ch[n].gold += money;
							do_char_log(n, 2, "You received %dG %dS as part of %s's reward.\n", money/100, money%100, ch[co].name);
							if ((nr = ch[cn].data[51])!=0) do_give_exp(n, nr, 0, -1);
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
						do_give_exp(co, nr/4, 0, -1);
					}
					god_take_from_char(in, cn);
					it[in].used = USE_EMPTY;  // silently destroy the item.
				}
				else
				{
					do_sayx(cn, "Simply look at any item in your possession, and you will now know its true value.");
					god_take_from_char(in, cn);
					it[in].used = USE_EMPTY;
					ch[co].flags |= CF_APPRAISE;
					do_char_log(co, 0, "You learned how to appraise items!\n");
					do_update_char(co);
					if ((nr = ch[cn].data[51])!=0)
					{
						do_sayx(cn, "Now I'll teach you a bit about life, the world and everything, %s.", ch[co].name);
						do_give_exp(co, nr, 0, -1);
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
							div = 1;
						}
						if ((nr = ch[cn].data[51])!=0) do_give_exp(n, nr/max(1,div), 0, -1);
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
					if ((nr = ch[cn].data[51])!=0)
					{
						do_sayx(cn, "Well, let me teach you a couple of small life lessons instead...");
						do_give_exp(co, nr/4, 0, -1);
					}
					god_take_from_char(in, cn);
					it[in].used = USE_EMPTY;  // silently destroy the item.
				}
				else
				{
					do_sayx(cn, "You will now notice a magic item when it's dropped if you can see it. Additionally, you may now notice when enemies try to cast spells on you.");
					god_take_from_char(in, cn);
					it[in].used = USE_EMPTY;
					ch[co].flags |= CF_SENSE;
					do_char_log(co, 0, "You learned how to sense magic!\n");
					do_update_char(co);
					if ((nr = ch[cn].data[51])!=0)
					{
						do_sayx(cn, "Now I'll teach you a bit about life, the world and everything, %s.", ch[co].name);
						do_give_exp(co, nr, 0, -1);
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
							div = 1;
						}
						if ((nr = ch[cn].data[51])!=0) do_give_exp(n, nr/max(1,div), 0, -1);
					}
				}
				// </group rewards>
			}
			else if ((qnum = nr) >= 101) // Flags for quests which don't teach skills
			{
				tmp = npc_quest_check(co, qnum-101); // Returns 0 if not clear, 1 if already cleared
			}
			else
			{
				int nr2 = nr, div = 4;
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
					do_sayx(cn, "But you already know %s, %s!", skilltab[nr].name, ch[co].name);
					if ((nr = ch[cn].data[51])!=0)
					{
						do_sayx(cn, "Well, let me teach you a couple of small life lessons instead...");
						do_give_exp(co, nr/4, 0, -1);
					}
					god_take_from_char(in, cn);
					it[in].used = USE_EMPTY;  // silently destroy the item.
				}
				else
				{
					B_SK(co, nr) = 1;
					do_char_log(co, 0, "You learned %s!\n", skilltab[nr].name);
					do_update_char(co);
					
					if ((nr = ch[cn].data[51])!=0)
					{
						do_sayx(cn, "Now I'll teach you a bit about life, the world and everything, %s.", ch[co].name);
						do_give_exp(co, nr, 0, -1);
					}
					god_take_from_char(in, cn);
					it[in].used = USE_EMPTY;  // silently destroy the item.
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
						if ((nr2 = ch[cn].data[51])!=0) do_give_exp(n, nr2/max(1,div), 0, -1);
					}
				}
				// </group rewards>
			}
		}

		/* items with a return gift */
		if ((nr = ch[cn].data[66])!=0)
		{
			god_take_from_char(in, cn);
			it[in].used = USE_EMPTY;
			if (!(ch[cn].flags&(CF_BODY))) 
			{
				do_sayx(cn, "Here is your %s in exchange.", it_temp[nr].reference);
				in = god_create_item(nr);
				god_give_char(in, co);
				if ((nr = ch[cn].data[51])!=0)
				{
					int div = 4;
					do_sayx(cn, "For your effort, allow me to teach you some mysteries of the world.");
					do_give_exp(co, tmp ? nr/max(1,div) : nr, 0, -1);
					// <group rewards>
					for (n = 1; n<MAXCHARS; n++)
					{
						if (ch[n].used==USE_EMPTY || !(ch[n].flags & (CF_PLAYER | CF_USURP))) continue;
						if (isgroup(n, co) && isgroup(co, n) && isnearby(co, n))
						{
							div = 4;
							if (qnum >= 101) tmp = npc_quest_check(n, qnum-101);
							do_give_exp(n, tmp ? nr/max(1,div) : nr, 0, -1);
						}
					}
					// </group rewards>
				}
			}
		}
		else if ((money = ch[cn].data[69])!=0)
		{
			if (money < 10000)
				do_sayx(cn, "Here is a small token of gratitude, %s.", ch[co].name);
			else
				do_sayx(cn, "Here is your reward in exchange, %s!", ch[co].name);
			god_take_from_char(in, cn);
			it[in].used = USE_EMPTY;
			ch[co].gold += money;
			do_char_log(co, 2, "You received %dG %dS.\n", money / 100, money % 100);
			if ((nr = ch[cn].data[51])!=0)
			{
				do_sayx(cn, "As an extra thanks, let me teach you a little something I know.");
				do_give_exp(co, tmp ? nr/4 : nr, 0, -1);
				// <group rewards>
				for (n = 1; n<MAXCHARS; n++)
				{
					if (ch[n].used==USE_EMPTY || !(ch[n].flags & (CF_PLAYER | CF_USURP))) continue;
					if (isgroup(n, co) && isgroup(co, n) && isnearby(co, n))
					{
						do_give_exp(n, tmp ? nr/4 : nr, 0, -1);
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
			god_take_from_char(in, co);
			it[in].used = USE_EMPTY;
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
							     points2rank(ch[n].points_tot)>= 5  // Staff Sergeant
							  && points2rank(ch[n].points_tot)<= 8))) // Sergeant Major
							do_char_log(n, 3, "Gate Guard: \"The monsters are approaching the north gate! Alert!\"\n");
						else if (ch[co].data[CHD_GROUP]==602
							  && (is_inline(n, 2) || (
							     points2rank(ch[n].points_tot)>= 9  // Second Lieutenant
							  && points2rank(ch[n].points_tot)<=13))) // Lieutenant Colonel
							do_char_log(n, 3, "Gate Guard: \"The monsters are approaching the center gate! Alert!\"\n");
						else if (ch[co].data[CHD_GROUP]==603
							  && (is_inline(n, 3) || (
							     points2rank(ch[n].points_tot)>=14))) // Colonel
							do_char_log(n, 3, "Gate Guard: \"The monsters are approaching the south gate! Alert!\"\n");
					}
					else
					{
						if (ch[co].data[CHD_GROUP]==601
							  && (is_inline(n, 1) || (
							     points2rank(ch[n].points_tot)>= 5  // Staff Sergeant
							  && points2rank(ch[n].points_tot)<= 8))) // Sergeant Major
							do_char_log(n, 3, "Outpost Guard: \"The monsters are approaching the north outpost! Alert!\"\n");
						else if (ch[co].data[CHD_GROUP]==602
							  && (is_inline(n, 2) || (
							     points2rank(ch[n].points_tot)>= 9  // Second Lieutenant
							  && points2rank(ch[n].points_tot)<=13))) // Lieutenant Colonel
							do_char_log(n, 3, "Outpost Guard: \"The monsters are approaching the center outpost! Alert!\"\n");
						else if (ch[co].data[CHD_GROUP]==603
							  && (is_inline(n, 3) || (
							     points2rank(ch[n].points_tot)>=14))) // Colonel
							do_char_log(n, 3, "Outpost Guard: \"The monsters are approaching the south outpost! Alert!\"\n");
					}
				}
			}
		}
	}

	return 0;
}

int is_potion(int in)
{
	static int potions[] = {
		IT_POT_M_HP, IT_POT_N_HP, IT_POT_G_HP, IT_POT_H_HP, 
		IT_POT_S_HP, IT_POT_C_HP, IT_POT_L_HP, IT_POT_D_HP, 
		IT_POT_M_EN, IT_POT_N_EN, IT_POT_G_EN, IT_POT_H_EN, 
		IT_POT_S_EN, IT_POT_C_EN, IT_POT_L_EN, IT_POT_D_EN, 
		IT_POT_M_MP, IT_POT_N_MP, IT_POT_G_MP, IT_POT_H_MP, 
		IT_POT_S_MP, IT_POT_C_MP, IT_POT_L_MP, IT_POT_D_MP, 
		IT_POT_VITA, IT_POT_CLAR, IT_POT_SAGE, IT_POT_LIFE, 
		IT_POT_T, IT_POT_O, IT_POT_PT, IT_POT_PO, 
		IT_POT_LAB2, IT_POT_GOLEM, 
		IT_POT_BRV, IT_POT_WIL, IT_POT_INT, IT_POT_AGL, IT_POT_STR, 
		IT_POT_EXHP, IT_POT_EXEN, IT_POT_EXMP, 
		IT_POT_PRE, IT_POT_EVA, IT_POT_MOB, IT_POT_FRE, IT_POT_MAR, 
		IT_POT_IMM, IT_POT_CLA, IT_POT_THO, IT_POT_BRU, IT_POT_RES, 
		IT_POT_APT, IT_POT_OFF, IT_POT_DEF, IT_POT_PER, IT_POT_STE, 
		IT_POT_RAIN };
	int tn, n;

	tn = it[in].temp;
	for (n = 0; n<ARRAYSIZE(potions); n++)
	{
		if (tn == potions[n])
		{
			return 1;
		}
	}
	return 0;
}

int is_scroll(int in)
{
	int tn;

	tn = it[in].temp;
	return (((tn >= 1314) && (tn <= 1341)) || ((tn >= 182) && (tn <= 189)));
}

int is_soulstone(int in)
{
	return (it[in].driver == 68);
}

int is_gemstone(int in)
{
	return (it[in].flags & IF_GEMSTONE);
}

int is_unique_able(int temp)
{
	static int uniqueable[60] = {
	//	Dag  Sta  Spe  Shi  Swo  Dua  Axe  Two  Gre
		284, 285, 286, 287, 288, 289, 290, 291, 292, // Steel
		523, 524, 525, 526, 527, 528, 529, 530, 531, // Gold
		532, 533, 534, 535, 536, 537, 538, 539, 540, // Emerald
		541, 542, 543, 544, 545, 546, 547, 548, 549, // Crystal
		572, 573, 574, 575, 576, 577, 578, 579, 580, // Titanium		// 45
		693, 694, 695, 696, 697, 698, 699, 700, 701, // Adamantium		// 54
		1779, 1780, 1781, 1782, 1783, 1784			 // Claws			// 60
	};
	int n, m=0;
	
	for (n = 0; n<60; n++)
	{
		if (temp==uniqueable[n])
		{
			m = n+1;
			break;
		}
	}
	return m;
}

int is_unique(int in)
{
	//static int unique[] = {};
	//int n;
	if (it[in].flags & IF_UNIQUE) return 1;
	/*
	for (n = 0; n<ARRAYSIZE(unique); n++)
	{
		if (it[in].temp==unique[n] && (it[in].flags & IF_UNIQUE))
		{
			return 1;
		}
	}
	*/
	return 0;
}

int count_uniques(int cn)
{
	int n, in, cnt = 0;

	if (IS_BUILDING(cn))
	{
		do_char_log(cn, 0, "Not in build mode.\n");
		return 0;
	}

	if ((in = ch[cn].citem) && !(in & 0x80000000) && is_unique(in))
	{
		cnt++;
	}
	for (n = 0; n<40; n++)
	{
		if ((in = ch[cn].item[n]) && is_unique(in))
		{
			cnt++;
		}
	}
	for (n = 0; n<20; n++)
	{
		if ((in = ch[cn].worn[n]) && is_unique(in))
		{
			cnt++;
		}
	}
	for (n = 0; n<12; n++)
	{
		if ((in = ch[cn].alt_worn[n]) && is_unique(in))
		{
			cnt++;
		}
	}
	for (n = 0; n<62; n++)
	{
		if ((in = ch[cn].depot[n]) && is_unique(in))
		{
			cnt++;
		}
	}
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

	if (!do_char_can_see(cn, co))
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
	if ((cc = ch[cn].taunted) && IS_SANECHAR(cc) && (do_char_can_see(cn, cc) || ch[cn].data[78]))
	{
		// If our last attempt to attack failed, wander near the taunter
		if (!ch[cn].attack_cn && !ch[cn].goto_x && ch[cn].data[78])
		{
			ch[cn].goto_x = ch[cc].x + 5 - RANDOM(10);
			ch[cn].goto_y = ch[cc].y + 5 - RANDOM(10);
		}
		// Otherwise, try to attack the taunter
		else if (do_char_can_see(cn, cc) && ch[cn].attack_cn!=cc)
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
			if ((IS_COMP_TEMP(co) || (ch[co].flags & CF_SHADOWCOPY)) && !ch[co].data[CHD_GROUP]) // Give benefit of the doubt to new GC's and SC's
			{
				break;
			}
		}
		if (n==47)
		{
			int ccc;
			if (ch[cn].data[95]==2 && ch[cn].data[93])   // attack distance
			{
				dist = max(abs((ch[cn].data[29] % MAPX) - ch[co].x), abs((ch[cn].data[29] / MAPX) - ch[co].y));
				if (dist>ch[cn].data[93])
				{
					ccc = co;
					co = 0;
				}
			}
			if (co && npc_add_enemy(cn, co, 0))
			{
				if (!(ch[co].flags & CF_SILENCE))
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
			if (!(ch[co].flags & CF_SILENCE))
			{
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
				if (!(ch[co].flags & CF_SILENCE))
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
				
				if (!B_SK(co, SK_BARTER))
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
			else if (strcmp(ch[cn].text[2], "#skill01")==0) //    12		Bartering			( Jamil )
			{
				// Tutorial 2
				if (ch[co].data[76]<(1<<2))
				{
					chlog(co, "SV_SHOWMOTD tutorial 2");
					buf[0] = SV_SHOWMOTD;
					*(unsigned char*)(buf + 1) = 102;
					xsend(ch[co].player, buf, 2);
				}
				
				if (!B_SK(co, SK_BARTER))
					do_sayx(cn, "Hello, %s. Some thieves across the north road stole my gold amulet from me. I'd teach you BARTERING if you could get it back for me.", ch[co].name);
				else
					do_sayx(cn, "Hello, %s! Thanks again for helping me!", ch[co].name);
			}
			else if (strcmp(ch[cn].text[2], "#skill02")==0) //    15		Recall				( Inga )
			{
				if (IS_ANY_TEMP(co) || IS_BRAVER(co) || (IS_SEYAN_DU(co) && !B_SK(co, SK_TAUNT)))
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
					if (!B_SK(co, SK_CONCEN))
						do_sayx(cn, "Greetings, %s. I was attacked and lost my Stone Dagger in the park across the street. If you could return it, I'd teach you CONCENTRATE.", ch[co].name);
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
				if (IS_ANY_TEMP(co) || IS_BRAVER(co) 
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
				if (IS_ANY_TEMP(co) || IS_BRAVER(co) 
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
						do_sayx(cn, "Greetings, %s. Bring me the Ruby Amulet from the Thieves House and I'll teach you BLESS.", ch[co].name);
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
			else if (strcmp(ch[cn].text[2], "#skill14")==0) //    16/ 5	* Shield or Staff	( Edna )
			{
				if (IS_ANY_HARA(co)	|| (IS_SEYAN_DU(co) && B_SK(co, SK_SHIELD)))
				{
					if (!B_SK(co, SK_STAFF))
						do_sayx(cn, "Hello, %s. Bring me the Oak Buckler from the hedge maze next door, and I shall teach you how to use a STAFF!", ch[co].name);
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
				if (!B_SK(co, SK_IDENT))
					do_sayx(cn, "Hello, %s. In the Magic Maze, across from here, Jane has created a cruel weapon. Bring it to me and I shall teach you IDENTIFY.", ch[co].name);
				else
					do_sayx(cn, "Hello, %s. Have you identified anything interesting lately?", ch[co].name);
			}
			//
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
				if (points2rank(ch[co].points_tot)<7) // First Sergeant
					do_sayx(cn, "Hello... %s. Please come back when you're stronger... I have a request...", ch[co].name);
				else
					do_sayx(cn, "Hello... %s. Please bring me a Spider's Fang... from the Webbed Bush south of the Strange Forest... I would give you an Amulet of Resistance in exchange...", ch[co].name);
			}
			else if (strcmp(ch[cn].text[2], "#quest104")==0) // 	4 - Mine 2 / Golem Pot / Moodstone
			{
				if (points2rank(ch[co].points_tot)<7) // First Sergeant
					do_sayx(cn, "Welcome, %s. When you are stronger, I have a request of you.", ch[co].name);
				else
					do_sayx(cn, "Welcome, %s. Deep in the second level of the mines, golems hide a secret spring of mud. If you would bring me a flask of this, I would reward you with a Moodstone Ring.", ch[co].name);
			}
			else if (strcmp(ch[cn].text[2], "#quest105")==0) // 	5 - Penitentiary / Headsman / Belt
			{
				if (points2rank(ch[co].points_tot)<8) // Sergeant Major
					do_sayx(cn, "Greetings, %s. Please be careful here, the prisoners and guards alike are a rowdy bunch.", ch[co].name);
				else
					do_sayx(cn, "Greetings, %s. The warden of this prison is up to no good and has holed himself away in his office. Please bring me evidence of his crimes, and I would reward you.", ch[co].name);
			}
			else if (strcmp(ch[cn].text[2], "#quest106")==0) // 	6 - Grolm Lab / Ruby Axe / Amulet
			{
				if (points2rank(ch[co].points_tot)<9) // 2nd Leiu
					do_sayx(cn, "Hello, %s. Have you been to the Pentagram Quest?", ch[co].name);
				else
					do_sayx(cn, "Hello, %s. Deep in the Pentagram quest there is said to be a laboratory guarded by a cowardly grolm. Bring me a Ruby Axe from this lab, and I would reward you.", ch[co].name);
			}
			else if (strcmp(ch[cn].text[2], "#quest107")==0) // 	7 - Hermit / Rose / Potion
			{
				if (points2rank(ch[co].points_tot)<8) // Sergeant Major
					do_sayx(cn, "Hi, %s... I'm very busy at the moment, please come back later.", ch[co].name);
				else
					do_sayx(cn, "Hi, %s. There is a mad hermit living in the Strange Forest. He possesses a rare Pink Orchid in his garden. If you could bring me this rose, I would make you a Pure Orchid Potion.", ch[co].name);
			}
			else if (strcmp(ch[cn].text[2], "#quest108")==0) // 	8 - Swamp / Marline / Rattan Bo
			{
				if (points2rank(ch[co].points_tot)<9) // 2nd Lieu
					do_sayx(cn, "Welcome, %s. Please make yourself at home.", ch[co].name);
				else
					do_sayx(cn, "Welcome, %s. In the Southern Swamp some lizards fashioned themselves a staff called Rattan Bo. Fetch it for me and I'll give you a special tarot card.", ch[co].name);
			}
			else if (strcmp(ch[cn].text[2], "#quest109")==0) // 	9 - Garg Nest / Rufus / Royal Targe
			{
				if (points2rank(ch[co].points_tot)<10) // 1st Lieu
					do_sayx(cn, "Ahoy, %s.", ch[co].name);
				else
					do_sayx(cn, "Ahoy, %s. Ay've a request. Thar's un ol' shield holed away un de Gargoyle Nest. Bring ut to me an I'll reward ye wit a tarot card o' Strength.", ch[co].name);
			}
			else if (strcmp(ch[cn].text[2], "#quest110")==0) //   10 - Mine 3 / Gomez / Gold Huge Diamond Ring
			{
				if (points2rank(ch[co].points_tot)<9) // 2nd Lieu
					do_sayx(cn, "Ah, welcome %s!", ch[co].name);
				else
					do_sayx(cn, "Welcome, %s! Would you mind sparing the time to fetch me a Golden Ring adorned with a Huge Diamond? I'd reward you with this lovely tarot card.", ch[co].name);
			}
			else if (strcmp(ch[cn].text[2], "#quest111")==0) //   11 - Mine 3 / Donna / Garg Statuette
			{
				if (points2rank(ch[co].points_tot)<9) // 2nd Lieu
					do_sayx(cn, "Hello, %s. Please don't touch anything.", ch[co].name);
				else
					do_sayx(cn, "Hello, %s. In the lowest floor of the mines there's said to be a striking Gargoyle Statuette. I'd reward you with this devilish tarot card.", ch[co].name);
			}
			else if (strcmp(ch[cn].text[2], "#quest112")==0) //   12 - UG2 / Rose / Green Herbs
			{
				if (points2rank(ch[co].points_tot)<8) // Sergeant Major
					do_sayx(cn, "Leave me be, %s.", ch[co].name);
				else
					do_sayx(cn, "Hello, %s. I'm researching the monsters in the underground. If you could, bring me the strange green plants that grow there... I'd pay you well.", ch[co].name);
			}
			else if (strcmp(ch[cn].text[2], "#quest113")==0) //   13 - UG2 / Monica / Greenl Collector
			{
				if (points2rank(ch[co].points_tot)<9) // 2nd Lieu
					do_sayx(cn, "Greetings, %s. Are you lost?", ch[co].name);
				else
					do_sayx(cn, "Greetings, %s. If you could bring me a completed Greenling Eye Collection, I would present you a Greenling Eye Essense in return.", ch[co].name);
			}
			else if (strcmp(ch[cn].text[2], "#quest114")==0) //   14 - Canyon / Oscar / Templar Heater
			{
				if (points2rank(ch[co].points_tot)<6) // Master Serg
					do_sayx(cn, "Hello, %s.", ch[co].name);
				else
					do_sayx(cn, "Hello, %s... Do you have a moment? Could you retreive my Templar Heater from the bandits in the forgotten canyon? I'd give you my Serpentine Amulet in return.", ch[co].name);
			}
			else if (strcmp(ch[cn].text[2], "#quest115")==0) //   15 - Archive / Castor / Traveller's Guide
			{
				if (points2rank(ch[co].points_tot)<6) // Master Serg
					do_sayx(cn, "Salutations, %s.", ch[co].name);
				else
					do_sayx(cn, "Salutations! %s, if you could, please find me an old book called The Traveller's Guide. If you would, I'd give you a signed copy of my own book!", ch[co].name);
			}
			else if (strcmp(ch[cn].text[2], "#quest116")==0) //   16 - Pass / Grover / Gold/Sapph Helmet
			{
				if (points2rank(ch[co].points_tot)<7) // First Serg
					do_sayx(cn, "Please leave.", ch[co].name);
				else
					do_sayx(cn, "Ah, %s. Do come in. I've a request - could you retrive my Sapphire Golden Helmet from the bandits in the Jagged pass? I'd reward you handsomely.", ch[co].name);
			}
			else if (strcmp(ch[cn].text[2], "#quest117")==0) //   17 - Valley / Regal / Spellblade [1142]
			{
				do_sayx(cn, "A visitor? Are you here to challenge me to a duel? If not, fetch me the blade from those golems, would you? You can have a book on swords in return.", ch[co].name);
			}
			else if (strcmp(ch[cn].text[2], "#quest118")==0) //   18 - Garg Nest / Shera / Fire Egg
			{
				if (points2rank(ch[co].points_tot)<13) // Lieu Colonel
					do_sayx(cn, "Hmm... %s, could you come back a little stronger? I may have use of you.", ch[co].name);
				else
					do_sayx(cn, "Welcome, %s! If you'd bring me the Fire Egg from the Gargoyle Nest, I'd reward you with a Cloak of Fire of my own creation.", ch[co].name);
			}
			else if (strcmp(ch[cn].text[2], "#quest119")==0) //   19 - Black Stronghold / Tactician / Shiva's Sceptre
			{
				if (points2rank(ch[co].points_tot)<12) // Major
					do_sayx(cn, "Greetings, %s. We need HELP against the forces of the Black Stronghold. Will you HELP us?", ch[co].name);
				else
					do_sayx(cn, "Greetings, %s. We need HELP against the forces of the Black Stronghold. Please kill its master, Shiva, and bring me his staff. We'd all be forever in your debt.", ch[co].name);
			}
			else if (strcmp(ch[cn].text[2], "#quest120")==0) //   13 - UG2 / Danica / Sogl Collector
			{
				if (points2rank(ch[co].points_tot)<15) // Brig Gen
					do_sayx(cn, "Hi, %s... Um...", ch[co].name);
				else
					do_sayx(cn, "Hi, %s... Bring me a completed Sogling Eye Collection, and... would a Sogling Eye Essense be okay...?", ch[co].name);
			}
			else if (strcmp(ch[cn].text[2], "#quest121")==0) //   20 - Ice Nest / Blanche / Ice Egg
			{
				if (points2rank(ch[co].points_tot)<17) // Lieu General
					do_sayx(cn, "Come back a bit stronger, %s. I may need you to do something for me.", ch[co].name);
				else
					do_sayx(cn, "Hello, %s! If you'd bring me the Ice Egg from the Ice Gargoyle Nest, I'd reward you with a Cloak of Ice in return.", ch[co].name);
			}
			else if (strcmp(ch[cn].text[2], "#quest122")==0) //   21 - Ice Nest / Aster / Lion's Paws
			{
				if (points2rank(ch[co].points_tot)<14) // Colonel
					do_sayx(cn, "Good day, %s. Cold out.", ch[co].name);
				else
					do_sayx(cn, "Good day, %s. Please find me the Lion's Paws in the Ice Gargoyle Nest, and I would reward you with this magical tarot card.", ch[co].name);
			}
			else if (strcmp(ch[cn].text[2], "#quest123")==0) //   22 - Ludolf - Cathedral
			{
				if (points2rank(ch[co].points_tot)<11) // Captain
					do_sayx(cn, "I'm sorry, %s, but I don't have time for kids right now.", ch[co].name);
				else
					do_sayx(cn, "Those dang cultists ruined my farm, %s. Bring me their leader's dagger and I would reward you with a tarot card of good fortune.", ch[co].name);
			}
			else if (strcmp(ch[cn].text[2], "#quest124")==0) //   23 - Flanders - Striders
			{
				if (points2rank(ch[co].points_tot)<11) // Captain
					do_sayx(cn, "Howdy do, %s.", ch[co].name);
				else
					do_sayx(cn, "Howdy do, %s. A while back I lost a family heirloom to the striders in the east. If you could return it I'd give you my lonely tarot card.", ch[co].name);
			}
			else if (strcmp(ch[cn].text[2], "#quest125")==0) //   24 - Topham - Thugs
			{
				if (points2rank(ch[co].points_tot)<11) // Captain
					do_sayx(cn, "Good day and goodbye, %s.", ch[co].name);
				else
					do_sayx(cn, "Good day to you, %s. Those pesky thugs to the far east stole a very sturdy shield from my collection. Return it and I would reward you handsomely.", ch[co].name);
			}
			else if (strcmp(ch[cn].text[2], "#quest126")==0) //   25 - Navarre - Regal/Defender
			{
				do_sayx(cn, "Isssh. Human, if you sssee a man named Regal, please defeat him. Bring me hisss sssword, and I would reward you thisss red amulet.", ch[co].name);
			}
			else if (strcmp(ch[cn].text[2], "#quest127")==0) //   26 - Tsulu - Shadow Talons
			{
				do_sayx(cn, "Welcome, human. The foul lizard Venominousss hasss blinded and killed many of our children. Bring me the source of his poisonsss, and I will give you thisss blue amulet.", ch[co].name);
			}
			else if (strcmp(ch[cn].text[2], "#quest128")==0) //   27 - Shafira - Venom Compendium
			{
				do_sayx(cn, "Hello, human. There isss a ssscary beassst of a lizard in the Emerald Cave we call Ssshadefang. Bring me itsss clawsss, and you may have thisss green amulet.", ch[co].name);
			}
			else if (strcmp(ch[cn].text[2], "#quest129")==0) //   28 - Dracus - Emperor's Crown
			{
				do_sayx(cn, "Come in and ssstay quiet. The Lizard Emperor is a fool of a man. Bring me hisss helmet, and you may have thisss yellow amulet, and the honor if our kin.", ch[co].name);
			}
			else if (strcmp(ch[cn].text[2], "#quest130")==0) //   29 - Rassa - Onyx Egg
			{
				do_sayx(cn, "Human... Bring me an Onyx Egg from the Onyx Gargoyle Nessst, and I would give you a Cloak of Onyx... If you sssurvive, that isss.", ch[co].name);
			}
			else if (strcmp(ch[cn].text[2], "#quest131")==0) //   30 - Makira - Save the Queen
			{
				do_sayx(cn, "Welcome, Human. I would give you thisss belt if you could climb the Volcano to the Eassst and bring me the sssword sssleeping at the sssummit.", ch[co].name);
			}
			else if (strcmp(ch[cn].text[2], "#quest132")==0) //   31 -  Vora - Coral Axe
			{
				do_sayx(cn, "Isssh... Human, bring me... the Coral Axe from the King of the Sssea Pentagram Quessst. I may give you thisss belt in exchange.", ch[co].name);
			}
			else if (strcmp(ch[cn].text[2], "#quest133")==0) //   xx - Zorani - Black Plants
			{
				do_sayx(cn, "Greetingsss, human! Pleassse bring me rare, black herbsss from treacherousss placesss. I would pay you very well.", ch[co].name);
			}
			else if (strcmp(ch[cn].text[2], "#skill16")==0) //    10   	Swimming			( Lucci )
			{
				if (points2rank(ch[co].points_tot)<9) // 2nd Lieu
					do_sayx(cn, "Hello, %s! When you're a little stronger, come see me and I can teach you how to swim.", ch[co].name);
				else if (!B_SK(co, SK_SWIM))
					do_sayx(cn, "Hello, %s! Under the lake in the southern Strange Forest, there's said to be a valuable glittering cleaver. Bring it to me and I will teach you SWIMMING!", ch[co].name);
				else
					do_sayx(cn, "Hello, %s!", ch[co].name);
			}
			//
			else if (strcmp(ch[cn].text[2], "#clara")==0)
			{
				if (points2rank(ch[co].points_tot)<18) // General
					do_sayx(cn, "Get out, %s! Damor doesn't wish to see you!", ch[co].name);
			}
			else if (strcmp(ch[cn].text[2], "#damor")==0)
			{
				if (points2rank(ch[co].points_tot)>=18)
					do_sayx(cn, "%s. It's about time. Bring me a candle from the deepest part of the Black Stronghold, and I would reward you with this amulet.", ch[co].name);
			}
			//
			else 
			{
				int knowarch=0;
				if (B_SK(co, SK_WARCRY)   || B_SK(co, SK_LEAP)  || B_SK(co, SK_SHADOW) ||
					B_SK(co, SK_LETHARGY) || B_SK(co, SK_PULSE) || B_SK(co, SK_ZEPHYR) || 
					B_SK(co, SK_RAGE))
					knowarch=1;
				if (strcmp(ch[cn].text[2], "#skill21")==0) // ArTm - 35 - Warcry
				{
					if (IS_SEYA_OR_ARTM(co))
					{
						if (knowarch)
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
						if (knowarch)
							do_sayx(cn, "Greetings, %s!", ch[co].name);
						else
							do_sayx(cn, "Greetings, %s! Bring me the Star Amulet from the Northern Mountains and I would teach you ZEPHYR.", ch[co].name);
					}
					else
						do_sayx(cn, "Greetings, %s! Did you know Warriors can make the very wind strike foes alongside them?", ch[co].name);
				}
				else if (strcmp(ch[cn].text[2], "#skill23")==0) // Summ - 46 - Shadow Copy
				{
					if (IS_SEYA_OR_SUMM(co))
					{
						if (knowarch)
							do_sayx(cn, "Greetings, %s!", ch[co].name);
						else
							do_sayx(cn, "Greetings, %s! Bring me the Star Amulet from the Northern Mountains and I would teach you SHADOW COPY.", ch[co].name);
					}
					else
						do_sayx(cn, "Greetings, %s! Did you know Summoners can create copies from their very shadows?", ch[co].name);
				}
				else if (strcmp(ch[cn].text[2], "#skill24")==0) // Sorc - 42 - Lethargy
				{
					if (IS_SEYA_OR_SORC(co))
					{
						if (knowarch)
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
						if (knowarch)
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
						if (knowarch)
							do_sayx(cn, "Greetings, %s!", ch[co].name);
						else
							do_sayx(cn, "Greetings, %s! Bring me the Star Amulet from the Northern Mountains and I would teach you LEAP.", ch[co].name);
					}
					else
						do_sayx(cn, "Greetings, %s! Did you know Skalds can leap great distances to strike distant foes?", ch[co].name);
				}
				else if (strcmp(ch[cn].text[2], "#skill27")==0) // Brav - 14 - Rage
				{
					if (IS_SEYA_OR_BRAV(co))
					{
						if (knowarch)
							do_sayx(cn, "Greetings, %s!", ch[co].name);
						else
							do_sayx(cn, "Greetings, %s! Bring me the Star Amulet from the Northern Mountains and I would teach you RAGE.", ch[co].name);
					}
					else
						do_sayx(cn, "Greetings, %s! Did you know Bravers can force themselves into a furious rage?", ch[co].name);
				}
				else if (strcmp(ch[cn].text[2], "#gatekeeper13")==0)
				{
					if (knowarch && IS_SEYAN_DU(co))
					{
						do_sayx(cn, "Hello, %s. Are you happy with the skill you learned? If not, I can remove it for an experience cost of 250,000. Say UNLEARN and I can make it so.", ch[co].name);
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
						for (m=0;m<40;m++)
						{
							if (it[ch[cn].item[n]].temp==IT_TW_SINBIND)
								break;
						}
					}
					// here is message filter, if talking NPC is priest and player char is PURPLE we greet him
					if((ch[cn].temp == CT_PRIEST) && (IS_PURPLE(co) || m<40)) // priest template is 180
					{
						if (m==40)
							do_sayx(cn, "Greetings, %s!", ch[co].name);
						else
							do_sayx(cn, "Hello there, %s. If you happen to have a working Sinbinder Ring, I could... 'assist' you in putting it on.", ch[co].name);
					}
					else     // if it is not priest or player is not purple we do nothing, NPC will talk as usual
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
		if (!do_char_can_see(cn, co))
		{
			return 1;
		}
		if (npc_add_enemy(cn, co, 1))
		{
			if (!(ch[co].flags & CF_SILENCE))
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
	int prox, kin_hara, kin_sorc;
	
	prox = (PROX_MULTI+M_SK(cn, SK_PROX))/PROX_MULTI;
	kin_hara = (IS_ARCHHARAKIM(cn) && B_SK(cn, SK_PROX));
	kin_sorc = (IS_SORCERER(cn) && B_SK(cn, SK_PROX));
	
	switch(spell)
	{
		case SK_BLIND:		return SP_COST_BLIND;
		case SK_CLEAVE:		return 20;
		case SK_LEAP:		return 20;
		case SK_TAUNT:		return SP_COST_TAUNT;
		case SK_WEAKEN:		return SP_COST_WEAKEN;
		case SK_WARCRY:		return SP_COST_WARCRY;
		
		case SK_BLAST:		return 20 * (kin_hara ? prox : 1);
		case SK_BLESS:		return SP_COST_BLESS;
		case SK_CURSE:		return SP_COST_CURSE * (kin_sorc ? prox : 1);
		case SK_DISPEL:		return SP_COST_DISPEL;
		case SK_ENHANCE:	return SP_COST_ENHANCE;
		case SK_GHOST:		return SP_COST_GHOST;
		case SK_HASTE:		return SP_COST_HASTE;
		case SK_HEAL:		return SP_COST_HEAL;
		case SK_IDENT:		return SP_COST_IDENT;
		case SK_LIGHT:		return SP_COST_LIGHT;
		case SK_MSHIELD:	return SP_COST_MSHIELD;
		case SK_POISON:		return SP_COST_POISON * (kin_sorc ? prox : 1);
		case SK_PROTECT:	return SP_COST_PROTECT;
		case SK_PULSE:		return SP_COST_PULSE;
		case SK_ZEPHYR:		return SP_COST_ZEPHYR;
		case SK_RECALL:		return SP_COST_RECALL;
		case SK_SHADOW:		return SP_COST_SHADOW;
		case SK_SLOW:		return SP_COST_SLOW * (kin_sorc ? prox : 1);
		
		default: return 9999;
	}
}

int spellflag(int spell)
{
	switch(spell)
	{
		case    SK_BLIND:		return SP_BLIND;
		case    SK_TAUNT:		return SP_TAUNT;
		case    SK_WARCRY:		return SP_WARCRY;
		case    SK_WEAKEN:		return SP_REND;
		
		case    SK_BLESS:		return SP_BLESS;
		case    SK_CURSE:		return SP_CURSE;
		case    SK_DISPEL:		return SP_DISPEL;
		case    SK_ENHANCE:		return SP_ENHANCE;
		case    SK_HASTE:		return SP_HASTE;
		case    SK_HEAL:		return SP_HEAL;
		case    SK_LIGHT:		return SP_LIGHT;
		case    SK_MSHIELD:		return SP_MSHIELD;
		case    SK_POISON:		return SP_POISON;
		case    SK_PROTECT:		return SP_PROTECT;
		case    SK_PULSE:		return SP_PULSE;
		case    SK_ZEPHYR:		return SP_ZEPHYR;
		case    SK_SLOW:		return SP_SLOW;
		
		default:				return 0;
	}
}

int npc_try_spell(int cn, int co, int spell)
{
	int mana, end, n, in, tmp, cc;
	int offn, defn;

	if (spell!=SK_CLEAVE && spell!=SK_LEAP && spell!=SK_WEAKEN && 
		spell!=SK_TAUNT && spell!=SK_WARCRY && spell!=SK_BLIND)
	{
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

	if (!B_SK(cn, spell)) // we don't know this spell
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
	defn = get_target_resistance(cn, co)*10;
	
	// Invidia
	/*
	if (spell==SK_HEAL && cn==co && IS_COMP_TEMP(cn) && (cc = ch[cn].data[CHD_MASTER]) && IS_SANEPLAYER(cc) && get_gear(cc, IT_TW_INVIDIA))
	{
		return 0;
	}
	*/
	
	// dont blast if enemy armor is too strong
	// Updated 02/11/2020 - changed the formula to include immunity and allow blasts that would do at least 5 damage.
	if (spell==SK_BLAST && ( (offn - max(0, get_target_immunity(cn, co)/2)) * 2 ) - ch[co].armor < 20/3)
	{
		return 0;
	}
	
	// dont cleave if enemy armor is too strong
	if (spell==SK_CLEAVE && ( ((offn + ch[cn].weapon/4 - max(0, (ch[co].to_parry/2))) * 2 ) - ch[co].armor < 20/3) )
	{
		return 0;
	}
	
	// dont leap if enemy armor is too strong
	if (spell==SK_LEAP && ( ((offn + max(0,((SPEED_CAP-ch[cn].speed)+ch[cn].atk_speed-120))/4
		- max(0, (ch[co].to_parry/2))) * 2 ) - ch[co].armor < 20/3) )
	{
		return 0;
	}

	// dont debuff if chances of success are bad
	if (spell==SK_CURSE  && SP_MULT_CURSE   * offn / max(1, defn)< 7) return 0;
	if (spell==SK_BLIND  && SP_MULT_BLIND   * offn / max(1, defn)< 7) return 0;
	if (spell==SK_POISON && SP_MULT_POISON  * offn / max(1, defn)< 8) return 0;
	if (spell==SK_WEAKEN && SP_MULT_WEAKEN  * offn / max(1, defn)< 8) return 0;
	if (spell==SK_SLOW   && SP_MULT_SLOW    * offn / max(1, defn)< 9) return 0;
	if (spell==SK_DISPEL && SP_MULT_DISPEL2 * offn / max(1, defn)< 9) return 0;
	if (spell==SK_TAUNT  && SP_MULT_TAUNT   * offn / max(1, defn)<10) return 0;
	if (spell==SK_WARCRY && SP_MULT_WARCRY  * offn / max(1, defn)<10) return 0;
	
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
		if (in && (bu[in].temp==SK_EXHAUST || (spell==SK_SHADOW && bu[in].temp==SK_SHADOW)))
		{
			return 0;
		}
	}

	mana = ch[cn].a_mana / 1000;
	end = ch[cn].a_end / 1000;

	for (n = 0; n<MAXBUFFS; n++)
	{
		if ((in = ch[co].spell[n])!=0)
		{
			// Cancel if target is already buffed or debuffed (except for heal)
			if (bu[in].temp==spell && spell!=SK_HEAL && 
				(bu[in].power + 10)>=spell_immunity(M_SK(cn, spell), get_target_immunity(cn, co)) && 
				bu[in].active>bu[in].duration / 4 * 3)
			{
				break;
			}
			// Immunize/Inoculate prevents up to three ailments
			if ((bu[in].temp==SK_DISPEL || bu[in].temp==SK_DISPEL2) &&
				(spell==bu[in].data[1] || spell==bu[in].data[2] || spell==bu[in].data[3]))
			{
				chlog(cn,"Immunize true (%d)",bu[in].temp);
				break;
			}
			// Poison layer adjustments
			if (bu[in].temp==spell && spell==SK_POISON && bu[in].active>bu[in].duration/3)
			{
				break;
			}
		}
	}

	if (n==MAXBUFFS)
	{
		tmp = spellflag(spell);
		if (spell==SK_WEAKEN || spell==SK_CLEAVE || spell==SK_WARCRY || 
			spell==SK_BLIND || spell==SK_TAUNT || spell==SK_LEAP)
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
	if (spell==SK_CLEAVE || spell==SK_WEAKEN || spell==SK_WARCRY || 
		spell==SK_BLIND || spell==SK_TAUNT || spell==SK_LEAP)
	{
		if (ch[cn].a_end / 1000 < get_spellcost(cn, spell))
		{
			return 0;
		}
	}
	else
	{
		if (ch[cn].a_mana / 1000 < get_spellcost(cn, spell) * (get_tarot(cn, IT_CH_MAGI_R)?3:2)/2)
		{
			return 0;
		}
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
	do_area_log(cn, 0, ch[cn].x, ch[cn].y, 1, "The %s uses a %s.\n", ch[cn].name, it[in].name);

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
				in = map[XY2M(x, y)].it; it[in].active = 0; it[in].data[5] = 16200;
				reset_go(it[in].x, it[in].y); it[in].flags |= IF_MOVEBLOCK | IF_SIGHTBLOCK; reset_go(it[in].x, it[in].y);
				x = 934; y = 421; cn = map[XY2M(x, y)].ch; if (cn) god_transfer_char(cn, x-1, y);
				in = map[XY2M(x, y)].it; it[in].active = 0; it[in].data[5] = 16200;
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
				in = map[XY2M(x, y)].it; it[in].active = 0; it[in].data[5] = 16200;
				reset_go(it[in].x, it[in].y); it[in].flags |= IF_MOVEBLOCK | IF_SIGHTBLOCK; reset_go(it[in].x, it[in].y);
				x = 568; y = 122; cn = map[XY2M(x, y)].ch; if (cn) god_transfer_char(cn, x+1, y);
				in = map[XY2M(x, y)].it; it[in].active = 0; it[in].data[5] = 16200;
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
				in = map[XY2M(x, y)].it; it[in].active = 0; it[in].data[5] = 16200;
				reset_go(it[in].x, it[in].y); it[in].flags |= IF_MOVEBLOCK | IF_SIGHTBLOCK; reset_go(it[in].x, it[in].y);	
				x = 188; y = 218; cn = map[XY2M(x, y)].ch; if (cn) god_transfer_char(cn, x, y-1);
				in = map[XY2M(x, y)].it; it[in].active = 0; it[in].data[5] = 16200;
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
				in = map[XY2M(x, y)].it; it[in].active = 0; it[in].data[5] = 16200;
				reset_go(it[in].x, it[in].y); it[in].flags |= IF_MOVEBLOCK | IF_SIGHTBLOCK; reset_go(it[in].x, it[in].y);
				x = 530; y = 176; cn = map[XY2M(x, y)].ch; if (cn) god_transfer_char(cn, x, y-1);
				in = map[XY2M(x, y)].it; it[in].active = 0; it[in].data[5] = 16200;
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

int is_inline(int cn, int linenum)
{
	int x, y;
	
	if(!IS_PLAYER(cn)) return 0;
	if(!IS_ACTIVECHAR(cn)) return 0;
	
	x = ch[cn].x;
	y = ch[cn].y;
	
	if (linenum == 1 && x >= 539 && y >= 282 && x <= 562 && y <= 372) return 1; // Line 1
	if (linenum == 2 && x >= 563 && y >= 282 && x <= 589 && y <= 372) return 1; // Line 2
	if (linenum == 3 && x >= 590 && y >= 282 && x <= 613 && y <= 372) return 1; // Line 3
	
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
		if (globs->mdtime%(BS_HOUR*6)>1 && globs->mdtime%(BS_HOUR*6)<2160 && ch[cn].data[3]!=0 && ch[cn].data[3]!=1)
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
		|| (pinline && ch[cn].a_hp>=ch[cn].hp[5]*999 && globs->mdtime%(BS_MINS)==0) 
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
						if (!(ch[co].flags & CF_CANCRIT)) 
							ch[co].flags |= CF_CANCRIT;
						modified=1;
					}
					if (modified)
					{
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
		via generate_map_enemy(temp, kin, xx, yy, base, affix, tarot)
	
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
	int spawnX, spawnY, magenum=0, groupnum=601, spawnT = 347;
	int p=0, n, m, j, in=0, co=0;
	int cw[4] = {0};
	
	/*
		Mage driver data will carry references to pass along to cityattack.c
		
		data[0] = whether or not we're active / number of players
		data[1] = main wave number
		data[2] = sub wave number
		data[3] = global time for next event process
		data[4] = number of enemies spawned so far
		data[5] = number of enemies killed so far
		
		generate_map_enemy(spawnT, bs_waves[magenum-1][ch[cn].data[1]][ch[cn].data[2]][m], spawnX, spawnY, (magenum*15+(magenum==3?15:0))+ch[cn].data[1]*magenum, 0, 0);
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
		if (globs->mdtime%(BS_HOUR*6)>1 && globs->mdtime%(BS_HOUR*6)<2160 && 
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
			if (co = generate_map_enemy(spawnT, bs_waves[magenum-1][ch[cn].data[1]-1][ch[cn].data[2]-1][m], 
				spawnX, spawnY, (magenum*15+(magenum==3?15:0))+ch[cn].data[1]*magenum, 
				(ch[cn].data[1]>10&&!RANDOM(20))?2:((ch[cn].data[1]>5&&!RANDOM(20))?1:0), 0))
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
		for (n = MCD_ENEMY1ST; n<=MCD_ENEMYZZZ; n++) // remove enemy
		{
			ch[cn].data[n] = 0;
		}
		fx_add_effect(6, 0, ch[cn].x, ch[cn].y, 0);
		
		do_area_log(cn, 0, ch[cn].x, ch[cn].y, 1, "The %s leapt across the room.\n", ch[cn].name);
		
		B_SK(cn, SK_SLOW) = 90;
		B_SK(cn, SK_CURSE) = 90;
		
		ch[cn].a_hp   = 999999;
		ch[cn].a_end  = 999999;
		ch[cn].a_mana = 999999;
	}
	else if (ch[cn].data[2]==1 && ch[cn].data[4]==0)
	{
		ch[cn].data[2] = 2;
		
		in = god_create_item(833);
		god_give_char(in, cn);
		
		do_sayx(cn, "Enough of thisss charade! Die!!");
		
		npc_quaff_potion(cn, 833, 254);
		
		B_SK(cn, SK_ZEPHYR) = 90;
		
		ch[cn].a_hp   = 999999;
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
		
		ch[cn].a_hp   = 999999;
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
		
		ch[cn].a_hp   = 999999;
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
		
		ch[cn].a_hp   = 999999;
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
		
		in = god_create_item(267);
		god_give_char(in, cn);
		npc_quaff_potion(cn, 267, 254);
		
		ch[ch[cn].attack_cn].attack_cn = 0;
		ch[cn].attack_cn = 0;
		fx_add_effect(6, 0, ch[cn].x, ch[cn].y, 0);
		god_transfer_char(cn, 572, 126);
		fx_add_effect(6, 0, ch[cn].x, ch[cn].y, 0);
		ch[cn].dir = 7;
		
		ch[cn].a_hp   = 999999;
		ch[cn].a_end  = 999999;
		ch[cn].a_mana = 999999;
	}
}

int npc_driver_high(int cn)
{
	int x, y, in, co, indoor1, indoor2, cc, in2, n;
	int mc, m;

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

		if (ch[cn].data[64]<globs->ticker && (ch[cn].flags&CF_SHADOWCOPY) && IS_PLAYER(ch[cn].data[CHD_MASTER]))
		{
			ch[ch[cn].data[CHD_MASTER]].data[PCD_SHADOWCOPY] = 0;
			do_sayx(cn, "I'm done, %s...", ch[ch[cn].data[CHD_MASTER]].name);
			do_give_exp(ch[cn].data[CHD_MASTER], ch[cn].data[28], 1, -1);
			fx_add_effect(6, 0, ch[ch[cn].data[CHD_MASTER]].x, ch[ch[cn].data[CHD_MASTER]].y, 0);
			fx_add_effect(7, 0, ch[cn].x, ch[cn].y, 0);
			die_companion(cn);
			return 1;
		}
		else if (ch[cn].data[64]<globs->ticker && (ch[cn].flags&CF_SHADOWCOPY))
		{
			ch[ch[cn].data[CHD_MASTER]].data[PCD_SHADOWCOPY] = 0;
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
				if (IS_SANECHAR(guesser[idx]) && IS_PLAYER(guesser[idx]))
				{
					ch[guesser[idx]].data[PCD_RIDDLER] = 0;
				}
				guesser[idx] = 0;
				do_char_log(cn, 1, "%s tells you: Too late! Too late! Try again some time.\n", ch[cn].name);
			}
		}
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
	if (ch[cn].a_hp<ch[cn].hp[5] * 600 && !get_tarot(cn, IT_CH_STAR))
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
	else if (ch[cn].a_hp<ch[cn].hp[5] * 600 && get_tarot(cn, IT_CH_STAR))
	{
		for (n = 0; n<MAXBUFFS; n++) if ((in = ch[cn].spell[n])!=0) if (bu[in].temp==SK_REGEN) break;
		if (n==MAXBUFFS)
		{
			if (npc_try_spell(cn, cn, SK_HEAL))
			{
				return 1;
			}
		}
	}
	
	// Dispel - for self
	if (B_SK(cn, SK_DISPEL) && !get_tarot(cn, IT_CH_HEIROPH))
	{
		for (n = 0, m = 0; n<MAXBUFFS; n++)
		{
			if ((in = ch[cn].spell[n])==0) continue;
			if (bu[in].temp==SK_DISPEL) { m=0; break; }
			if (bu[in].temp==SK_POISON	|| bu[in].temp==SK_BLEED   || bu[in].temp==SK_BLIND  || 
				bu[in].temp==SK_WARCRY2 || bu[in].temp==SK_CURSE2  || bu[in].temp==SK_CURSE  || 
				bu[in].temp==SK_WARCRY  || bu[in].temp==SK_WEAKEN2 || bu[in].temp==SK_WEAKEN || 
				bu[in].temp==SK_SLOW2   || bu[in].temp==SK_SLOW    || bu[in].temp==SK_DOUSE  || 
				bu[in].temp==SK_SCORCH  || bu[in].temp==SK_DISPEL2) m++;
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
	if ((in = ch[cn].citem)!=0 && ch[cn].data[47])
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

	if ((in = ch[cn].item[39])!=0 && ch[cn].data[47])
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
			if (npc_try_spell(cn, cn, SK_MSHIELD))
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
		if (ch[co].a_hp<ch[co].hp[5] * 600 && !get_tarot(cn, IT_CH_STAR)) // he's hurt
		{
			if (npc_try_spell(cn, co, SK_HEAL))
			{
				return 1;
			}
		}
		else if (ch[co].a_hp<ch[co].hp[5] * 600 && get_tarot(cn, IT_CH_STAR))
		{
			for (n = 0; n<MAXBUFFS; n++) if ((in = ch[co].spell[n])!=0)	if (bu[in].temp==SK_REGEN) break;
			if (n==MAXBUFFS)
			{
				if (npc_try_spell(cn, co, SK_HEAL))
				{
					return 1;
				}
			}
		}
		
		// Dispel - for friend
		if (B_SK(cn, SK_DISPEL) && !get_tarot(cn, IT_CH_HEIROPH))
		{
			for (n = 0, m = 0; n<MAXBUFFS; n++)
			{
				if ((in = ch[co].spell[n])==0) continue;
				if (bu[in].temp==SK_DISPEL) { m=0; break; }
				if (bu[in].temp==SK_POISON	|| bu[in].temp==SK_BLEED   || bu[in].temp==SK_BLIND  || 
					bu[in].temp==SK_WARCRY2 || bu[in].temp==SK_CURSE2  || bu[in].temp==SK_CURSE  || 
					bu[in].temp==SK_WARCRY  || bu[in].temp==SK_WEAKEN2 || bu[in].temp==SK_WEAKEN || 
					bu[in].temp==SK_SLOW2   || bu[in].temp==SK_SLOW    || bu[in].temp==SK_DOUSE  || 
					bu[in].temp==SK_SCORCH  || bu[in].temp==SK_DISPEL2) m++;
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

		if (ch[co].a_hp<ch[co].hp[5] * 600 && !get_tarot(cn, IT_CH_STAR)) // he's hurt
		{
			if (npc_try_spell(cn, co, SK_HEAL))
			{
				return 1;
			}
		}
		else if (ch[co].a_hp<ch[co].hp[5] * 600 && get_tarot(cn, IT_CH_STAR))
		{
			for (n = 0; n<MAXBUFFS; n++) if ((in = ch[co].spell[n])!=0)	if (bu[in].temp==SK_REGEN) break;
			if (n==MAXBUFFS)
			{
				if (npc_try_spell(cn, co, SK_HEAL))
				{
					return 1;
				}
			}
		}
		
		// Dispel - for friend
		if (B_SK(cn, SK_DISPEL) && !get_tarot(cn, IT_CH_HEIROPH))
		{
			for (n = 0, m = 0; n<MAXBUFFS; n++)
			{
				if ((in = ch[co].spell[n])==0) continue;
				if (bu[in].temp==SK_DISPEL) { m=0; break; }
				if (bu[in].temp==SK_POISON	|| bu[in].temp==SK_BLEED   || bu[in].temp==SK_BLIND  || 
					bu[in].temp==SK_WARCRY2 || bu[in].temp==SK_CURSE2  || bu[in].temp==SK_CURSE  || 
					bu[in].temp==SK_WARCRY  || bu[in].temp==SK_WEAKEN2 || bu[in].temp==SK_WEAKEN || 
					bu[in].temp==SK_SLOW2   || bu[in].temp==SK_SLOW    || bu[in].temp==SK_DOUSE  || 
					bu[in].temp==SK_SCORCH  || bu[in].temp==SK_DISPEL2) m++;
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
		if (npc_try_spell(cn, cn, SK_MSHIELD))
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
		// Dispel - for enemy
		if (co && B_SK(cn, SK_DISPEL) && get_tarot(cn, IT_CH_HEIROPH) && !has_buff(co, SK_DISPEL2))
		{
			for (n = 0, m = 0; n<MAXBUFFS; n++)
			{
				if ((in = ch[co].spell[n])==0) continue;
				if (bu[in].temp==SK_DISPEL2) { m=0; break; }
				if (bu[in].temp==SK_HASTE 	|| bu[in].temp==SK_BLESS   || bu[in].temp==SK_MSHIELD || 
					bu[in].temp==SK_MSHELL  || bu[in].temp==SK_PULSE   || bu[in].temp==SK_ZEPHYR  || 
					bu[in].temp==SK_GUARD   || bu[in].temp==SK_DISPEL  || bu[in].temp==SK_REGEN   || 
					bu[in].temp==SK_PROTECT || bu[in].temp==SK_ENHANCE || bu[in].temp==SK_LIGHT) m++;
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
		if (co && globs->ticker>ch[cn].data[74] && npc_try_spell(cn, co, SK_SHADOW))
		{
			ch[cn].data[74] = globs->ticker + TICKS * 10;
			return 1;
		}
		if (co && !get_tarot(cn, IT_CH_JUDGE_R) && npc_try_spell(cn, cn, SK_PULSE))
		{
			return 1;
		}
		if (co && npc_try_spell(cn, cn, SK_ZEPHYR))
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
		if (co && globs->ticker>ch[cn].data[74] && npc_try_spell(cn, co, SK_BLIND))
		{
			ch[cn].data[74] = globs->ticker + TICKS * 15;
			return 1;
		}
		if (co && ((!IS_PLAYER(co) && globs->ticker>ch[cn].data[74]) || (IS_PLAYER(co) && globs->ticker>ch[co].data[74])) && 
			npc_try_spell(cn, co, SK_WARCRY))
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
		ch[cn].goto_x = ch[cn].x + 5 - RANDOM(10);
		ch[cn].goto_y = ch[cn].y + 5 - RANDOM(10);
		return 1;
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

	if (!ch[cn].data[41] && !ch[cn].data[47])
	{
		return 0;                                     // don't scan if we don't use the information anyway

	}
	// save some work. you need to check here if no other work needs to be done!
	if (ch[cn].data[41] && ch[cn].misc_action==DR_USE)
	{
		return 0;
	}
	if (ch[cn].data[47] && ch[cn].misc_action==DR_PICKUP)
	{
		return 0;
	}
	if (ch[cn].data[47] && ch[cn].misc_action==DR_USE)
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
					if (!it[in].active && (globs->dlight<200 || indoor2))
					{
						ch[cn].misc_action  = DR_USE;
						ch[cn].misc_target1 = x;
						ch[cn].misc_target2 = y;
						ch[cn].goto_x = 0;        // cancel goto, which stems probably from patrol
						ch[cn].data[58] = 1;
						return 1;
					}

					if (it[in].active && globs->dlight>200 && !indoor2)
					{
						ch[cn].misc_action  = DR_USE;
						ch[cn].misc_target1 = x;
						ch[cn].misc_target2 = y;
						ch[cn].goto_x = 0;        // cancel goto, which stems probably from patrol
						ch[cn].data[58] = 1;
						return 1;
					}
				}
				if (ch[cn].data[47] && indoor1==indoor2 && (it[in].flags & IF_TAKE) &&
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
				if (ch[cn].data[47] && indoor1==indoor2 && (it[in].driver==7) &&
				    can_go(ch[cn].x, ch[cn].y, it[in].x, it[in].y) &&
				    do_char_can_see_item(cn, in))
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
			if (ch[cn].temp==CT_BSMAGE1 && (is_inline(n, 1) || (points2rank(ch[n].points_tot)>= 5 && points2rank(ch[n].points_tot)<=8)))
			{
				switch (candlenum)
				{
					case 0: 
						do_char_log(n, 3, "Tactician: \"The northern stronghold is calming down!\"\n"); break;
					case 1: 
						do_char_log(n, 3, "Tactician: \"The northern stronghold is starting to stir!\"\n"); break;
					case 2: 
						do_char_log(n, 3, "Tactician: \"The northern stronghold is growing in numbers!\"\n"); break;
					case 3: 
						do_char_log(n, 3, "Tactician: \"The northern stronghold is building its forces! Alert!\"\n"); break;
					case 4: 
						do_char_log(n, 3, "Tactician: \"The northern stronghold is nearing full strength! Alert!\"\n"); break;
					case 5: 
						do_char_log(n, 3, "Tactician: \"The northern stronghold is at full strength! Alert!\"\n"); break;
				}
			}
			else if (ch[cn].temp==CT_BSMAGE2 && (is_inline(n, 2) || (points2rank(ch[n].points_tot)>=9 && points2rank(ch[n].points_tot)<=13)))
			{
				switch (candlenum)
				{
					case 0: 
						do_char_log(n, 3, "Tactician: \"The central stronghold is calming down!\"\n"); break;
					case 1: 
						do_char_log(n, 3, "Tactician: \"The central stronghold is starting to stir!\"\n"); break;
					case 2: 
						do_char_log(n, 3, "Tactician: \"The central stronghold is growing in numbers!\"\n"); break;
					case 3: 
						do_char_log(n, 3, "Tactician: \"The central stronghold is building its forces! Alert!\"\n"); break;
					case 4: 
						do_char_log(n, 3, "Tactician: \"The central stronghold is nearing full strength! Alert!\"\n"); break;
					case 5: 
						do_char_log(n, 3, "Tactician: \"The central stronghold is at full strength! Alert!\"\n"); break;
				}
			}
			else if (ch[cn].temp==CT_BSMAGE3 && (is_inline(n, 3) || points2rank(ch[n].points_tot)>=14))
			{
				switch (candlenum)
				{
					case 0: 
						do_char_log(n, 3, "Tactician: \"The southern stronghold has calmed down!\"\n"); break;
					case 1: 
						do_char_log(n, 3, "Tactician: \"The southern stronghold has started to stir!\"\n"); break;
					case 2: 
						do_char_log(n, 3, "Tactician: \"The southern stronghold is growing in numbers!\"\n"); break;
					case 3: 
						do_char_log(n, 3, "Tactician: \"The southern stronghold is building its forces! Alert!\"\n"); break;
					case 4: 
						do_char_log(n, 3, "Tactician: \"The southern stronghold is nearing full strength! Alert!\"\n"); break;
					case 5: 
						do_char_log(n, 3, "Tactician: \"The southern stronghold is at full strength! Alert!\"\n"); break;
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
		else if ((in = ch[cn].worn[WN_RHAND])!=0 && (it[in].placement & PL_TWOHAND))
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
		else if ((it[in].placement & PL_TWOHAND) && ch[cn].worn[WN_LHAND])
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
		else if (n==WN_RRING && (in = ch[cn].worn[WN_LRING])!=0 && (it[in].placement & PL_TWOHAND))
		{
			return 0;
		}
		else if (n==WN_LRING && (in = ch[cn].worn[WN_RRING])!=0 && (it[in].placement & PL_TWOHAND))
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
		if (!ch[cn].worn[n] || npc_item_value(cn, in)>npc_item_value(cn, ch[cn].worn[n]))
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
	for (n = 0; n<40; n++)
	{
		if ((in = ch[co].item[n]))
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
	int n, x, y, m, in, panic, co;
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
	
	// are we supposed to loot graves?
	if (ch[cn].alignment<0 &&
	    (globs->flags & GF_LOOTING) &&
	    ((cn & 15)==(globs->ticker & 15) || (ch[cn].flags & CF_ISLOOTING)) &&
	    !IS_COMP_TEMP(cn) && !(ch[cn].temp>=176 && ch[cn].temp<=179))
	{
		if (npc_grave_logic(cn))
		{
			return;
		}
	}

	// did someone call help? - high prio
	if (ch[cn].data[55] && ch[cn].data[55] + TICKS * 120>globs->ticker && ch[cn].data[54])
	{
		m = ch[cn].data[54];
		ch[cn].goto_x = m % MAPX + get_frust_x_off(globs->ticker);
		ch[cn].goto_y = m / MAPX + get_frust_y_off(globs->ticker);
		ch[cn].data[58] = 2;
		return;
	}

	// go to last known enemy position and stay there for up to 30 seconds
	if (ch[cn].data[77] && ch[cn].data[77] + TICKS * 30>globs->ticker)
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

	for (y = max(0, ch[cn].y - 9); y<min(MAPY, ch[cn].y + 10); y++) // check within 10 spaces for a player
	{
		m = y * MAPX;
		for (x = max(0, ch[cn].x - 9); x<min(MAPX, ch[cn].x + 10); x++)
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
	if (ch[cn].data[60])   //random walk, low
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

	if (ch[cn].data[29])    // resting position, lowest prio
	{
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
			if (!(ch[cn].flags & CF_MERCHANT))
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
		
		if (!(ch[cn].flags & CF_MERCHANT)) 
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
			if (it[in].damage_state || (it[in].flags & IF_SHOPDESTROY))
			{
				it[in].used = USE_EMPTY;
				ch[cn].item[n] = 0;
				ch[cn].item_lock[n] = 0;
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

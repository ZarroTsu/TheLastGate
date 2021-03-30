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

// 1 in-game hour
#define T_HOUR 3600
// The rate at which raids happen
#define T_RAID (T_HOUR*4)

// 3600*4 = 21600 / 18 / 60 = 13 minutes IRL
// 3600*5 = 21600 / 18 / 60 = 16 minutes IRL
// 3600*6 = 21600 / 18 / 60 = 20 minutes IRL
// 3600*8 = 28800 / 18 / 60 = 26 minutes irl
// 3600*9 = 32400 / 18 / 60 = 30 minutes IRL


// NOTE: Even though we don't use the data[] array, we may only use it as it is used in driver.c, since a lot
//	 of other functions rely on the data in there being either absent or correct.

// data[0] to data[24] are safe to use:

// data[0] = state
// data[1] = failed movetos

// data[29] to data[41] are safe to use:

// data[50] to data[70] are safe to use:


int npc_cityattack_gotattack(int cn, int co)
{
	int n;
	// fight back when attacked - all NPCs do this:
	if (npc_add_enemy(cn, co, 1))
	{
		chlog(cn, "Added %s to kill list for attacking me", ch[co].name);
		/*
		if (IS_PLAYER(co) || IS_COMP_TEMP(co))
		{
			for (n = 1; n<MAXCHARS; n++)
			{
				if (ch[n].used==USE_EMPTY) continue;
				if (ch[n].data[25]==2 && ch[n].data[0]<2 && (
					(ch[cn].data[26]>=101 && ch[n].data[26]>=101 && ch[cn].data[26]<=199 && ch[n].data[26]<=110) ||
					(ch[cn].data[26]>=201 && ch[n].data[26]>=201 && ch[cn].data[26]<=299 && ch[n].data[26]<=210) ||
					(ch[cn].data[26]>=301 && ch[n].data[26]>=301 && ch[cn].data[26]<=399 && ch[n].data[26]<=310)
				))
				{
					ch[n].data[0]=2; // Force the raid since someone attacked a member
				}
			}
		}
		*/
	}
	return(1);
}

int npc_cityattack_seeattack(int cn, int cc, int co)
{
	if (!do_char_can_see(cn, co))
	{
		return( 1);                     // processed it: we cannot see the defender, so ignore it
	}
	if (!do_char_can_see(cn, cc))
	{
		return( 1);                     // processed it: we cannot see the attacker, so ignore it
	}
	
	if (ch[cn].data[31])   // protect char (by temp)
	{
		if (ch[co].temp==ch[cn].data[31])
		{
			if (npc_add_enemy(cn, cc, 1))
			{
				chlog(cn, "Added %s to kill list for attacking %s (protect char)", ch[cc].name, ch[co].name);
			}
		}
		if (ch[cc].temp==ch[cn].data[31])
		{
			if (npc_add_enemy(cn, co, 1))
			{
				chlog(cn, "Added %s to kill list for being attacked by %s (protect char)", ch[co].name, ch[cc].name);
			}
		}
		if (ch[cn].data[0]<2) ch[cn].data[0]=2; // Force the raid state
	}

	return(0);
}

int npc_cityattack_see(int cn, int co)
{
	int cc, idx;

	if (!do_char_can_see(cn, co))
	{
		return( 1);                     // processed it: we cannot see him, so ignore him
	}
	if (ch[cn].data[42]!=ch[co].data[42] && ch[co].data[42]!=60 && ch[cn].data[0]>=2) // only fight if we're raiding
	{
		if (!(cc = ch[cn].attack_cn) || npc_dist(cn, co)<npc_dist(cn, cc))
		{
			ch[cn].attack_cn = co;
			ch[cn].goto_x = 0;
			idx = co | (char_id(co) << 16); // so GCs can attack
			ch[cn].data[80] = idx;			// so GCs can attack
		}
	}

	return(1);
}

int npc_cityattack_msg(int cn, int type, int dat1, int dat2, int dat3, int dat4)
{
	switch(type)
	{
	case    NT_GOTHIT:
		return( npc_cityattack_gotattack(cn, dat1));
	case    NT_GOTMISS:
		return( npc_cityattack_gotattack(cn, dat1));
	case    NT_DIDHIT:
		return( 0);
	case    NT_DIDMISS:
		return( 0);
	case    NT_DIDKILL:
		return( 0);
	case    NT_GOTEXP:
		return( 0);
	case    NT_SEEKILL:
		return( 0);
	case    NT_SEEHIT:
		return( npc_cityattack_seeattack(cn, dat1, dat2));
	case    NT_SEEMISS:
		return( npc_cityattack_seeattack(cn, dat1, dat2));
	case    NT_GIVE:
		return( 0);
	case    NT_SEE:
		return( npc_cityattack_see(cn, dat1));
	case    NT_DIED:
		return( 0);
	case    NT_SHOUT:
		return( 0);
	case    NT_HITME:
		return( 0);

	default:
		xlog("Unknown NPC message for %d (%s): %d",
		     cn, ch[cn].name, type);
		return( 0);
	}
}

int npc_cityattack_high(int cn)
{
	int co;

	// heal if hurt
	if (ch[cn].a_hp<ch[cn].hp[5] * 600)
	{
		if (npc_try_spell(cn, cn, SK_HEAL))
		{
			return( 1);
		}
	}

	// generic spell management
	if (ch[cn].a_mana>ch[cn].mana[5] * 850 && ch[cn].skill[SK_MEDIT][0])
	{
		if (ch[cn].a_mana>75000 && npc_try_spell(cn, cn, SK_BLESS))
		{
			return( 1);
		}
		if (npc_try_spell(cn, cn, SK_PROTECT))
		{
			return( 1);
		}
		if (npc_try_spell(cn, cn, SK_MSHIELD))
		{
			return( 1);
		}
		if (npc_try_spell(cn, cn, SK_HASTE))
		{
			return( 1);
		}
		if (npc_try_spell(cn, cn, SK_ENHANCE))
		{
			return( 1);
		}
		if (npc_try_spell(cn, cn, SK_BLESS))
		{
			return( 1);
		}
	}

	// generic endurance management
	if (ch[cn].attack_cn && ch[cn].a_end>10000)
	{
		if (ch[cn].mode!=2)
		{
			ch[cn].mode = 2;
			do_update_char(cn);
		}
	}
	else if (ch[cn].a_end>10000)
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

	// fight management
	if ((co = ch[cn].attack_cn)) // we're fighting
	{
		if (co && (IS_PLAYER(co) || is_facing(cn,co)) && (ch[cn].a_hp<ch[cn].hp[5] * 600 || !RANDOM(10))) // we're losing
		{
			if (globs->ticker>ch[co].data[75] && npc_try_spell(cn, co, SK_BLAST))
			{
				ch[co].data[75] = globs->ticker + (TICKS * 6)/2;
				return( 1);
			}
		}

		if (ch[cn].a_mana>75000 && npc_try_spell(cn, cn, SK_BLESS))
		{
			return( 1);
		}
		if (npc_try_spell(cn, cn, SK_PROTECT))
		{
			return( 1);
		}
		if (npc_try_spell(cn, cn, SK_MSHIELD))
		{
			return( 1);
		}
		if (npc_try_spell(cn, cn, SK_HASTE))
		{
			return( 1);
		}
		if (npc_try_spell(cn, cn, SK_ENHANCE))
		{
			return( 1);
		}
		if (npc_try_spell(cn, cn, SK_BLESS))
		{
			return( 1);
		}
		if (co && (IS_PLAYER(co) || is_facing(cn,co)) && npc_try_spell(cn, co, SK_SLOW))
		{
			return(1);
		}
		if (co && is_facing(cn,co) && npc_try_spell(cn, co, SK_WEAKEN))
		{
			return( 1);
		}
		if (co && (IS_PLAYER(co) || is_facing(cn,co)) && npc_try_spell(cn, co, SK_CURSE))
		{
			return( 1);
		}
		if (co && globs->ticker>ch[cn].data[74] + TICKS * 10 && npc_try_spell(cn, co, SK_GHOST))
		{
			ch[cn].data[74] = globs->ticker;
			return(1);
		}
		if (co && (IS_PLAYER(co) || is_facing(cn,co)) && npc_try_spell(cn, co, SK_POISON))
		{
			return( 1);
		}
		if (co && (IS_PLAYER(co) || is_facing(cn,co)) && globs->ticker>ch[co].data[75] && npc_try_spell(cn, co, SK_WARCRY))
		{
			ch[co].data[75] = globs->ticker + (TICKS*2 + TICKS * get_skill_score(cn, SK_WARCRY)/80)*3/2;
			return( 1);
		}
		if (co && is_facing(cn,co) && globs->ticker>ch[co].data[75] && npc_try_spell(cn, co, SK_CLEAVE))
		{
			ch[co].data[75] = globs->ticker + (TICKS * 5)*3/4;
			return( 1);
		}

		if (co && (IS_PLAYER(co) || is_facing(cn,co)) && ch[co].armor + 5>ch[cn].weapon) // blast always if we cannot hurt him otherwise
		{
			if (globs->ticker>ch[co].data[75] && npc_try_spell(cn, co, SK_BLAST))
			{
				ch[co].data[75] = globs->ticker + (TICKS * 6)/2;
				return( 1);
			}
		}
	}

	return(0);
}

int npc_moveto(int cn, int x, int y)
{
	int dx, dy, try;

	if (abs(ch[cn].x - x)<3 && abs(ch[cn].y - y)<3)
	{
		ch[cn].data[1] = 0;
		return(1);
	}

	if (ch[cn].data[1]==0 && npc_check_target(x, y))
	{
		ch[cn].data[1]++;
		ch[cn].goto_x = x;
		ch[cn].goto_y = y;
		return(0);
	}

	for (dx = 0, try = 1; dx<3; dx++)
	{
		for (dy = 0; dy<3; dy++, try++)
		{
			if (ch[cn].data[1]<try && npc_check_target(x + dx, y + dy))
			{
				ch[cn].data[1]++;
				ch[cn].goto_x = x + dx;
				ch[cn].goto_y = y + dy;
				return(0);
			}
			if (ch[cn].data[1]<try && npc_check_target(x - dx, y + dy))
			{
				ch[cn].data[1]++;
				ch[cn].goto_x = x - dx;
				ch[cn].goto_y = y + dy;
				return(0);
			}
			if (ch[cn].data[1]<try && npc_check_target(x + dx, y - dy))
			{
				ch[cn].data[1]++;
				ch[cn].goto_x = x + dx;
				ch[cn].goto_y = y - dy;
				return(0);
			}
			if (ch[cn].data[1]<try && npc_check_target(x - dx, y - dy))
			{
				ch[cn].data[1]++;
				ch[cn].goto_x = x - dx;
				ch[cn].goto_y = y - dy;
				return(0);
			}

		}
	}

	ch[cn].data[1] = 0;

	return(0);
}

int npc_cityattack_wait(cn)
{
	int n;
	int bsm1=0, bsm2=0, bsm3=0;
	/*
	int t_rone = 0;
	int t_rtwo = T_RAID/2;
	int t_rthr = 0;
	int t_secs = TICKS*2;
	
		 if (ch[cn].data[26]>=301 && ch[cn].data[26]<=399 && (globs->mdtime % T_RAID)>=t_rthr && (globs->mdtime % T_RAID)<(t_rthr+t_secs))
		return 1;
	else if (ch[cn].data[26]>=201 && ch[cn].data[26]<=299 && (globs->mdtime % T_RAID)>=t_rtwo && (globs->mdtime % T_RAID)<(t_rtwo+t_secs))
		return 1;
	else if (ch[cn].data[26]>=101 && ch[cn].data[26]<=199 && (globs->mdtime % T_RAID)>=t_rone && (globs->mdtime % T_RAID)<(t_rone+t_secs))
		return 1;
	*/
	
	for (n = 1; n<MAXCHARS; n++)
	{
		if (ch[n].used==USE_EMPTY) continue;
		if (ch[n].temp==CT_BSMAGE3) bsm3 = ch[n].data[1];
		if (ch[n].temp==CT_BSMAGE2) bsm2 = ch[n].data[1];
		if (ch[n].temp==CT_BSMAGE1) bsm1 = ch[n].data[1];
	}
	
	if (ch[cn].data[26]>=301 && ch[cn].data[26]<=399 && bsm3>=BS_COUNTER) return 1;
	if (ch[cn].data[26]>=201 && ch[cn].data[26]<=299 && bsm2>=BS_COUNTER) return 1;
	if (ch[cn].data[26]>=101 && ch[cn].data[26]<=199 && bsm1>=BS_COUNTER) return 1;
	
	return 0;
}

void npc_cityattack_low(int cn)
{
	int ret = 0;

	if (ch[cn].data[26]>=301 && ch[cn].data[26]<=399)
	{
		switch(ch[cn].data[0])
		{
			case 0:	ret = npc_moveto(cn, 598, 295);	break;		// Navigate to waiting spot
			case 1:	ret = npc_cityattack_wait(cn);	break;		// Wait for signal time
			case 2:	ret = npc_moveto(cn, 598, 305);	break;		// Charge east - 1
			case 3:	ret = npc_moveto(cn, 600, 336);	break;		// Charge east - 2
			case 4:	ret = npc_moveto(cn, 598, 368);	break;		// Charge east - 3
		}
	}
	else if (ch[cn].data[26]>=201 && ch[cn].data[26]<=299)
	{
		switch(ch[cn].data[0])
		{
			case 0:	ret = npc_moveto(cn, 576, 295);	break;		// Navigate to waiting spot
			case 1:	ret = npc_cityattack_wait(cn);	break;		// Wait for signal time
			case 2:	ret = npc_moveto(cn, 576, 305);	break;		// Charge east - 1
			case 3:	ret = npc_moveto(cn, 576, 336);	break;		// Charge east - 2
			case 4:	ret = npc_moveto(cn, 576, 368);	break;		// Charge east - 3
		}
	}
	else // 101 - 199
	{
		switch(ch[cn].data[0])
		{
			case 0:	ret = npc_moveto(cn, 554, 295);	break;		// Navigate to waiting spot
			case 1:	ret = npc_cityattack_wait(cn);	break;		// Wait for signal time
			case 2:	ret = npc_moveto(cn, 554, 305);	break;		// Charge east - 1
			case 3:	ret = npc_moveto(cn, 552, 336);	break;		// Charge east - 2
			case 4:	ret = npc_moveto(cn, 554, 368);	break;		// Charge east - 3
		}
	}

	if (ret)
	{
		ch[cn].data[0]++;
	}
}

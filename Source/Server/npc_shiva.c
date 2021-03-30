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

// data[0] to data[24] are safe to use:

// data[0] = Phase counter 	- starts at 0 and adds 1 per phase.
// data[1] = Phase timer    - pause a little bit before continuing to fight
// data[2] = aggro timer	- Set when fight starts and only goes down if Shiva is heading home. When 0, resets phase to 0.
// data[3] = subPhase		- For timer to progress properly
// data[4] = door lock		- Store the key needed for the door, so we can force it to lock
// data[5] = store player	- Store primary antagonist to resume fighting after thralls
// data[6] = last enemy

// data[29] to data[41] are safe to use:
// data[50] to data[70] are safe to use:

int npc_shiva_see(int cn, int co)
{
	int n, n2, idx, indoor1, indoor2;
	int x1, x2, y1, y2, dist, ret, cnt;
	
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
	
	// check if this is an enemy we added to our list earlier
	if (!ch[cn].attack_cn && ch[cn].data[1]<=0)   // only attack him if we aren't fighting already
	{
		idx = co | (char_id(co) << 16);

		for (n = 80; n<92; n++)
		{
			if (ch[cn].data[n]==idx)
			{
				ch[cn].attack_cn = co;
				ch[cn].goto_x = 0; // cancel goto (patrol)
				ch[cn].data[58] = 2;
				return(1);
			}
		}
	}
	
	// attack
	if (ch[cn].data[43] && ch[cn].data[0]==0)
	{
		for (n = 43; n<47; n++)
		{
			if (ch[cn].data[n] && ch[co].data[42]==ch[cn].data[n])
			{
				break;
			}
			if (ch[cn].data[n]==65536 && (IS_PLAYER(co) || IS_COMP_TEMP(co)))
			{
				break;
			}
			if (IS_COMP_TEMP(co) && !ch[co].data[42]) // Give benefit of the doubt to new GC's
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
				npc_saytext_n(cn, 1, ch[co].name);
				chlog(cn, "Added %s to kill list because he's not in my group", ch[co].name);
				return(1);
			}
			co = ccc;
		}
	}
	
	if (!ch[cn].data[0] && (ch[co].flags & (CF_PLAYER)) && ch[cn].data[37] && ch[cn].data[56]<globs->ticker)
	{
		
		npc_saytext_n(cn, 2, ch[co].name);
		ch[cn].data[40] = ch[cn].data[39];
		ch[cn].data[39] = ch[cn].data[38];
		ch[cn].data[38] = ch[cn].data[37];
		ch[cn].data[37] = co;
		ch[cn].data[56] = globs->ticker + TICKS * 30;
	}
	
	return 1;
}

int npc_shiva_gotattack(int cn, int co)
{
	int cc;
	
	ch[cn].data[92] = TICKS * 60 * 10;

	if (!do_char_can_see(cn, co))    // we have been attacked but cannot see the attacker
	{
		ch[cn].data[78] = globs->ticker + TICKS * 30;
		return(1);
	}
	
	// fight back when attacked - all NPCs do this:
	if (npc_add_enemy(cn, co, 1))
	{
		if (ch[cn].data[0]==0) npc_saytext_n(cn, 1, ch[co].name);
		chlog(cn, "Added %s to kill list for attacking me", ch[co].name);
	}

	return(1);
}

int npc_shiva_msg(int cn, int type, int dat1, int dat2, int dat3, int dat4)
{
	switch(type)
	{
		case    NT_GOTHIT:	return npc_shiva_gotattack(cn, dat1);
		case    NT_GOTMISS:	return npc_shiva_gotattack(cn, dat1);
		case    NT_DIDHIT:	return 0;
		case    NT_DIDMISS:	return 0;
		case    NT_DIDKILL:	return 0;
		case    NT_GOTEXP:	return 0;
		case    NT_SEEKILL:	return 0;
		case    NT_SEEHIT:	return npc_seeattack(cn, dat1, dat2);
		case    NT_SEEMISS:	return npc_seeattack(cn, dat1, dat2);
		case    NT_GIVE:	return 0;
		case    NT_SEE:		return npc_shiva_see(cn, dat1);
		case    NT_DIED:	return 0;
		case    NT_SHOUT:	return 0;
		case    NT_HITME:	return 0;
		
		default: 
			xlog("Unknown NPC message for %d (%s): %d", cn, ch[cn].name, type); 
			return 0;
	}
}

// Shiva coordinates
static int shiva_x[5] 		= { 607, 602, 602, 607, 610 };
static int shiva_y[5] 		= { 221, 224, 229, 229, 224 };
static int shiva_center[2] 	= { 606, 225 };

void npc_shiva_warp_away(int cn, int m) // Warp Shiva to the furthest point in the room from his current location
{
	int n, gohere;
	int x1, x2, y1, y2, dist1, dist2;
	int xo, yo;
	int in, co;
	int x, y, xf, yf, xt, yt;
	
	gohere  = m;
	
	// 3 first
	// 4 2nd
	// 2 is last
	
	if (m<2)
	{
		x1 = ch[cn].x;
		y1 = ch[cn].y;
		
		x2 = shiva_x[m];
		y2 = shiva_y[m];
		
		dist1 = abs(x1 - x2) + abs(y1 - y2);
		
		x2 = shiva_x[m+3];
		y2 = shiva_y[m+3];
		
		dist2 = abs(x1 - x2) + abs(y1 - y2);

		if (dist1>dist2) 
		{
			gohere = m;
		}
		else
		{
			gohere = m+3;
		}
	}

	xf = max(1, shiva_center[0] - 8);
	yf = max(1, shiva_center[1] - 8);
	xt = min(MAPX - 1, shiva_center[0] + 8+1);
	yt = min(MAPY - 1, shiva_center[1] + 8+1);
	
	// Stun everyone in the room except the final thrall.
	for (x = xf; x<xt; x++) for (y = yf; y<yt; y++) if ((co = map[x + y * MAPX].ch)) if (cn!=co) 
		if (ch[cn].data[42]!=ch[co].data[42])
	{
		in = god_create_buff();
		if (!in)
		{
			xlog("god_create_buff failed in npc_shiva_warp_away");
			return 0;
		}

		strcpy(bu[in].name, "Greater Slow");
		bu[in].flags |= IF_SPELL;
		bu[in].sprite[1] = BUF_SPR_SLOW2;
		bu[in].duration  = bu[in].active = TICKS * 6;
		bu[in].temp  = SK_SLOW2;
		bu[in].power = 200;
		
		do_char_log(co, 0, "You have been badly slowed.\n");
		
		char_play_sound(co, ch[cn].sound + 7, -150, 0);
		
		add_spell(co, in);
		
		fx_add_effect(5, 0, ch[co].x, ch[co].y, 0);
	}
	//
	
	/*
	for (n=0; n<5; n++) // Loop through each of the 8 points and see which is furthest away
	{
		x1 = ch[cn].x;		x2 = shiva_x[n];
		y1 = ch[cn].y;		y2 = shiva_y[n];
		
		dist = abs(x1 - x2) + abs(y1 - y2);
		
		if (dist>olddist) 
		{
			gohere  = n;
			olddist = dist;
		}
	}
	*/
	
	ch[cn].a_hp 	= 999999;
	ch[cn].a_mana 	= 999999;
	
	ch[cn].data[5] = ch[cn].attack_cn;
	
	for (n = 80; n<92; n++) // remove enemy
	{
		ch[cn].data[n] = 0;
	}
	
	xo = ch[cn].x;
	yo = ch[cn].y;
	
	ch[cn].data[76] = ch[cn].data[77] = ch[cn].data[78] = 0;
	
	if (god_transfer_char(cn, shiva_x[gohere], shiva_y[gohere]))
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
	
	ch[ch[cn].data[5]].attack_cn = 0;
	//ch[cn].misc_action = 0;
	//ch[cn].dir = DX_DOWN;
	
	ch[cn].misc_action = DR_TURN;
	ch[cn].misc_target1 = xo;
	ch[cn].misc_target2 = yo;
}

int npc_shiva_phasing(int cn)
{
	int phase, sub, timer, temp, n, co, x, y, count;
	int in, in2;
	int cx = 0;
	
	if (globs->ticker<ch[cn].data[1])
	{
		return;
	}
	
	phase = ch[cn].data[0];
	timer = ch[cn].data[1];
	sub   = ch[cn].data[3];
	cx    = ch[cn].data[6];
	
	// Gloat
	if (cx==0 && timer<=globs->ticker && sub==0)
	{
		switch (phase)
		{
			case 2: // Phase 2
				do_sayx(cn, "Now, behold... The fruit of my labor!");
				break;
			case 4: // Phase 4
				do_sayx(cn, "I'm not done! Not by a long shot! I WILL make you bow before me like the rest!");
				break;
			case 6: // Phase 6
				do_sayx(cn, "By The Purple One's deep void and endless reach, you WILL die this very minute!!");
				break;
			default:
				return;
		}
		ch[cn].data[3] = 1;
		ch[cn].data[1] = globs->ticker + TICKS * 3;
	}
	
	// Summon
	if (cx==0 && timer<=globs->ticker && sub==1)
	{
		if (phase==6 || !ch[cn].data[5])
		{
			x = shiva_center[0];
			y = shiva_center[1];
		}
		else
		{
			x = ch[ch[cn].data[5]].x;
			y = ch[ch[cn].data[5]].y;
		}
		
		switch (phase)
		{
			case 2: // Phase 2  - Grolms
				temp  = 665;
				count = 5;
				fx_add_effect(7, 0, ch[cn].x, ch[cn].y, 0);
				do_sayx(cn, "Khuzak gurawin duskar!");
				//ch[cn].a_mana -= ch[cn].mana[5] * 300;
				break;
			case 4: // Phase 4  - Gargs
				temp  = 666;
				count = 3;
				fx_add_effect(7, 0, ch[cn].x, ch[cn].y, 0);
				do_sayx(cn, "Khuzak gurawin duskar giharr!");
				//ch[cn].a_mana -= ch[cn].mana[5] * 300;
				break;
			case 6: // Phase 6  - Uh oh
				temp  = 667;
				count = 1;
				fx_add_effect(7, 0, ch[cn].x, ch[cn].y, 0);
				do_sayx(cn, "AEMON KHUZAK GURAWIN DUSKAR GIHARR!!");
				ch[cn].a_mana = 0;
				break;
			default:
				return;
		}
		for (n = 0; n<count; n++)
		{
			co = pop_create_char(temp, 0);
			if (!co)
			{
				do_sayx(cn, "create char (%d)", n);
				break;
			}
			if (!god_drop_char_fuzzy(co, x, y))
			{
				do_sayx(cn, "drop char (%d)", n);
				god_destroy_items(co);
				ch[co].used = USE_EMPTY;
				break;
			}
			fx_add_effect(6, 0, ch[co].x, ch[co].y, 0);
			if (phase!=6) ch[co].attack_cn = ch[cn].data[5];
		}
		if (phase==6) 
		{
			ch[cn].data[6] = co;
			ch[cn].data[3] = 0;
			ch[cn].data[1] = globs->ticker + TICKS * 1;
			ch[cn].data[0] = 7;
			return;
		}
		else
		{
			ch[cn].data[1] = globs->ticker + TICKS * 20;
			ch[cn].data[3] = 2;
		}
	}
	
	// Resume fighting
	if (cx==0 && timer<=globs->ticker && sub==2) 
	{
		switch (ch[cn].data[0])
		{
			case 2: // Phase 2
				do_sayx(cn, "Useless...!");
				break;
			case 4: // Phase 4
				do_sayx(cn, "Why won't you yield!?");
				break;
		}
		ch[cn].attack_cn = ch[cn].data[5];
		ch[cn].data[3] = 3;
		return;
	}
	
	if (cx==0) return;
	
	// Uh oh.
	if (timer<=globs->ticker && sub==0)
	{
		int xf, yf, xt, yt;
		
		xf = max(1, shiva_center[0] - 8);
		yf = max(1, shiva_center[1] - 8);
		xt = min(MAPX - 1, shiva_center[0] + 8+1);
		yt = min(MAPY - 1, shiva_center[1] + 8+1);
		
		// Stun everyone in the room except the final thrall.
		for (x = xf; x<xt; x++) for (y = yf; y<yt; y++) if ((co = map[x + y * MAPX].ch)) if (cx!=co && cn!=co) 
		{
			if (!(IS_PLAYER(co) || IS_COMP_TEMP(co))) 
			{
				remove_spells(co);
				//do_hurt(0, co, 999, 0);
			}
			
			in = god_create_buff();
			if (!in)
			{
				xlog("god_create_buff failed in npc_shiva_high");
				return 0;
			}

			strcpy(bu[in].name, "Stun");
			bu[in].flags |= IF_SPELL;
			bu[in].sprite[1] = BUF_SPR_WARCRY2;
			bu[in].duration  = bu[in].active = TICKS*45;
			bu[in].temp  = 666;
			bu[in].power = 666;
			
			do_char_log(co, 0, "Suddenly, you can't move...!\n");
			
			char_play_sound(co, ch[cn].sound + 7, -150, 0);
			
			add_spell(co, in);
			
			fx_add_effect(5, 0, ch[co].x, ch[co].y, 0);
		}
		remove_spells(cn);
		fx_add_effect(5, 0, ch[cn].x, ch[cn].y, 0);
		ch[cn].attack_cn = 0;
		for (n = 80; n<92; n++) // remove enemy
		{
			ch[cn].data[n] = 0;
		}
		
		if (in2 = map[606+233*MAPX].it) // Let's lock that door real quick
		{
			ch[cn].data[4] = it[in2].data[0];
			it[in2].data[0] = 1;
		}
		
		ch[cn].data[1] = globs->ticker + TICKS * 6;
		ch[cn].data[3]++;
	}
	if (timer<=globs->ticker && sub==1)
	{
		do_sayx(cx, "YOU, WHO DARES SPEAK ILL OF OUR MASTER.");
		ch[cn].data[1] = globs->ticker + TICKS * 5;
		ch[cn].data[3]++;
	}
	if (timer<=globs->ticker && sub==2)
	{
		do_sayx(cx, "YOU, WHO DARES CALL UPON OUR POWER.");
		ch[cn].data[1] = globs->ticker + TICKS * 3;
		ch[cn].data[3]++;
	}
	if (timer<=globs->ticker && sub==3)
	{
		do_sayx(cn, "What--");
		ch[cx].misc_action = DR_TURN;
		ch[cx].misc_target1 = ch[cn].x;
		ch[cx].misc_target2 = ch[cn].y;
		do_sayx(cx, "BE SILENT.");
		ch[cn].data[1] = globs->ticker + TICKS * 4;
		ch[cn].data[3]++;
	}
	if (timer<=globs->ticker && sub==4)
	{
		do_sayx(cn, "I absolutely will not! I enthralled YOU! I am your master! I command you to release me at once!!");
		ch[cn].data[1] = globs->ticker + TICKS * 5;
		ch[cn].data[3]++;
	}
	if (timer<=globs->ticker && sub==5)
	{
		do_sayx(cx, "YOU ARE BUT A WHELPLING IN THE FACE OF HIS MAJESTY.");
		ch[cx].goto_x = 603;
		ch[cx].goto_y = 228;
		ch[cn].data[1] = globs->ticker + TICKS * 3;
		ch[cn].data[3]++;
	}
	if (timer<=globs->ticker && sub==6)
	{
		do_sayx(cn, "What the devil are you talking about, 'His majesty'??");
		ch[cn].data[1] = globs->ticker + TICKS * 5;
		ch[cn].data[3]++;
	}
	if (timer<=globs->ticker && sub==7)
	{
		do_sayx(cx, "...");
		ch[cn].data[1] = globs->ticker + TICKS * 5;
		ch[cn].data[3]++;
	}
	if (timer<=globs->ticker && sub==8)
	{
		do_sayx(cx, "IN THE NAME OF MY LORD, KING AEMON OF THE BURNING PLAINS, I SENTENCE YOU MORTALS TO DEATH.");
		ch[cn].data[1] = globs->ticker + TICKS * 5;
		ch[cn].data[3]++;
	}
	if (timer<=globs->ticker && sub==9)
	{
		npc_add_enemy(cx, cn, 0);
		ch[cx].attack_cn = cn;
		if (in2 = map[606+233*MAPX].it) // Let's unlock that door now
		{
			it[in2].data[0] = ch[cn].data[4];
		}
		ch[cn].data[1] = globs->ticker + TICKS * 2;
		ch[cn].data[3]++;
	}
	if (timer<=globs->ticker && sub==10)
	{
		ch[cx].data[43] = ch[cx].data[42];
		ch[cn].hp[0] = 10000;
		do_sayx(cn, "Unhand me at once! ...Stay away from me! No! NO!!");
		ch[cn].data[3]++;
	}
}

int npc_shiva_high(int cn)
{
	int in, co, cc;
	int n, temp;
	
	// reset panic mode if expired
	if (ch[cn].data[78]<globs->ticker)
	{
		ch[cn].data[78] = 0;
	}
	
	npc_shiva_phasing(cn);
	
	if (ch[cn].a_hp<ch[cn].hp[5] * 600)
	{
		// Phase transition
		switch (ch[cn].data[0])
		{
			case 0: // End phase 0
				if (ch[cn].a_mana<30000)
				{
					ch[cn].a_mana+=30000;
				}
				if (npc_try_spell(cn, cn, SK_HEAL)) // Heal once normally, advance to phase 1
				{
					ch[cn].data[0] = 1;
					return 1;
				}
				break;
			case 1: // End phase 1
				ch[cn].data[0] = 2;
				ch[cn].data[1] = globs->ticker + TICKS * 3;
				ch[cn].data[3] = 0;
				do_sayx(cn, "Enough of this charade!");
				npc_shiva_warp_away(cn, 0);
				break;
			case 2: // End phase 2
				if (ch[cn].a_mana<30000)
				{
					ch[cn].a_mana+=30000;
				}
				if (npc_try_spell(cn, cn, SK_HEAL)) // Heal once normally, advance to phase 1
				{
					ch[cn].data[0] = 3;
					return 1;
				}
				break;
			case 3: // End phase 3
				ch[cn].data[0] = 4;
				ch[cn].data[1] = globs->ticker + TICKS * 3;
				ch[cn].data[3] = 0;
				do_sayx(cn, "Enough I say!");
				npc_shiva_warp_away(cn, 1);
				break;
			case 4: // End phase 4
				if (ch[cn].a_mana<30000)
				{
					ch[cn].a_mana+=30000;
				}
				if (npc_try_spell(cn, cn, SK_HEAL)) // Heal once normally, advance to phase 1
				{
					ch[cn].data[0] = 5;
					return 1;
				}
				break;
			case 5: // End phase 5
				ch[cn].data[0] = 6;
				ch[cn].data[1] = globs->ticker + TICKS * 3;
				ch[cn].data[3] = 0;
				do_sayx(cn, "Grrrr...!");
				npc_shiva_warp_away(cn, 2);
				break;
		}
	}
	
	// generic spell management
	if (ch[cn].a_mana>ch[cn].mana[5] * 850 && ch[cn].skill[SK_MEDIT][0] && ch[cn].data[6]==0)
	{
		if (ch[cn].a_mana>75000 && npc_try_spell(cn, cn, SK_BLESS)) return 1;
		if (npc_try_spell(cn, cn, SK_PROTECT)) return 1;
		if (npc_try_spell(cn, cn, SK_MSHIELD)) return 1;
		if (npc_try_spell(cn, cn, SK_HASTE)) return 1;
		if (npc_try_spell(cn, cn, SK_ENHANCE)) return 1;
		if (npc_try_spell(cn, cn, SK_BLESS)) return 1;
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
	if (check_dlight(ch[cn].x, ch[cn].y)<20 && map[ch[cn].x + ch[cn].y * MAPX].light<20)
	{
		if (npc_try_spell(cn, cn, SK_LIGHT)) return 1;
	}
	
	// help friend
	if ((co = ch[cn].data[65])!=0 && ch[cn].data[6]==0 && ch[cn].a_mana>ch[cn].mana[5]*250)
	{
		// bless us first if we have enough mana - makes spells more powerful
		if (ch[cn].a_mana>get_spellcost(cn, SK_BLESS) * 2 + get_spellcost(cn, SK_PROTECT) + get_spellcost(cn, SK_ENHANCE))
		{
			if (npc_try_spell(cn, cn, SK_BLESS))	return 1;
		}

		if (!npc_can_spell(co, cn, SK_PROTECT) && npc_try_spell(cn, co, SK_PROTECT))	return 1;
		if (!npc_can_spell(co, cn, SK_ENHANCE) && npc_try_spell(cn, co, SK_ENHANCE))	return 1;
		if (!npc_can_spell(co, cn, SK_BLESS) && npc_try_spell(cn, co, SK_BLESS))		return 1;

		ch[cn].data[65] = 0;
	}
	
	// generic fight-magic management
	if (ch[cn].data[6]==0 && ch[cn].a_mana>30000)
	{
		if ((co = ch[cn].attack_cn)!=0 || ch[cn].data[78]) // we're fighting
		{
			if (co && (ch[cn].a_hp<ch[cn].hp[5] * 600 || !RANDOM(10))) // we're losing
			{
				if (npc_try_spell(cn, co, SK_BLAST)) return 1;
			}

			if (ch[cn].a_mana>75000 && npc_try_spell(cn, cn, SK_BLESS)) return 1;
			if (npc_try_spell(cn, cn, SK_PROTECT)) return 1;
			if (npc_try_spell(cn, cn, SK_MSHIELD)) return 1;
			if (npc_try_spell(cn, cn, SK_HASTE)) return 1;
			if (npc_try_spell(cn, cn, SK_ENHANCE)) return 1;
			if (npc_try_spell(cn, cn, SK_BLESS)) return 1;
			if (co && npc_try_spell(cn, co, SK_SLOW)) return 1;
			if (co && npc_try_spell(cn, co, SK_CURSE)) return 1;
			if (co && npc_try_spell(cn, co, SK_POISON)) return 1;
			if (co && ch[co].armor + 5>ch[cn].weapon) // blast always if we cannot hurt him otherwise
			{
				if (npc_try_spell(cn, co, SK_BLAST)) return 1;
			}
		}
	}
	
	return(0);
}

void npc_shiva_low(int cn)
{
	int n, x, y, m;
	
	if (ch[cn].last_action==ERR_SUCCESS)
	{
		ch[cn].data[36] = 0;                                    // reset frust with successful action
	}
	else if (ch[cn].last_action==ERR_FAILED)
	{
		ch[cn].data[36]++;                                      // increase frust with failed action
	}
	
	// go to last known enemy position and stay there for up to 30 seconds
	/*
	if (ch[cn].data[77] && ch[cn].data[77] + TICKS * 30>globs->ticker)
	{
		m = ch[cn].data[76];
		ch[cn].goto_x = m % MAPX + get_frust_x_off(ch[cn].data[36]);
		ch[cn].goto_y = m / MAPX + get_frust_y_off(ch[cn].data[36]);
		return;
	}
	*/
	
	if (ch[cn].a_hp<ch[cn].hp[5] * 750) // we're hurt: rest
		return;
	
	if (ch[cn].data[2]>=globs->ticker) 	// Reset the fight
	{
		ch[cn].data[0]=0;
		ch[cn].data[1]=0;
		ch[cn].data[3]=0;
		for (n = 80; n<92; n++) // remove enemy
		{
			ch[cn].data[n] = 0;
		}
	}
	
	if (ch[cn].data[29] && ch[cn].data[0]==0)    // resting position, lowest prio
	{
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
}

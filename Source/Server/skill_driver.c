/*************************************************************************

   This file is part of 'Mercenaries of Astonia v2'
   Copyright (c) 1997-2001 Daniel Brockhaus (joker@astonia.com)
   All rights reserved.

 **************************************************************************/

#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include "server.h"

char *at_name[5] = {
	"Braveness",
	"Willpower",
	"Intuition",
	"Agility",
	"Strength"
};

char *sp_name[60] = {
	"00", 					// SK_HAND
	"01", 					// SK_PRECISION
	"02", 					// SK_DAGGER
	"03", 					// SK_SWORD
	"04", 					// SK_AXE
	"05", 					// SK_STAFF
	"06", 					// SK_TWOHAND
	"07", 					// SK_FOCUS
	"08", 					// SK_STEALTH
	"09", 					// SK_PERCEPT
	"10",					// SK_SWIM
	"Magic Shield",			// SK_MSHIELD
	"12",					// SK_BARTER
	"Repair",				// SK_REPAIR
	"Light",				// SK_LIGHT
	"Recall",				// SK_RECALL
	"16", 					// SK_SHIELD
	"Protect",				// SK_PROTECT
	"Enhance",				// SK_ENHANCE
	"Slow",					// SK_SLOW
	"Curse",				// SK_CURSE
	"Bless",				// SK_BLESS
	"Identify",				// SK_IDENT
	"23",					// SK_RESIST
	"Exhaust",				// SK_BLAST
	"Dispel",				// SK_DISPEL
	"Healing Sickness",		// SK_HEAL
	"27",					// SK_GHOST
	"Regen", 				// SK_REGEN
	"29",					// SK_REST
	"30",					// SK_MEDIT
	"31",					// SK_SENSE
	"32",					// SK_IMMUN
	"33",					// SK_SURROUND
	"34",					// SK_CONCEN
	"Fear",					// SK_WARCRY
	"36", 					// SK_DUAL
	"37", 					// SK_COMBATM
	"38", 					// SK_WEAPONM
	"39", 					// SK_ARMORM
	"40",					// SK_CLEAVE
	"Weakness",				// SK_WEAKEN
	"Poison",				// SK_POISON
	"43",					// SK_DAMAREA
	"44",					// SK_HEXAREA
	"45",					// SK_GCMASTERY
	"Shadow Copy",			// SK_SHADOW
	"Haste",				// SK_HASTE
	"48",					// SK_SURRAREA
	"49",					// SK_SURRSPEED
	"Stun",					// SK_WARCRY2
	"Bleed",				// SK_BLEED
	"Greater Weakness",		// SK_WEAKEN2
	"Scorch",				// SK_SCORCH
	"Greater Curse",		// SK_CURSE2
	"Greater Slow",			// SK_SLOW2
	"Magic Shell",			// SK_MSHELL
	"57",					// 
	"58",					// 
	"59"					// 
};

int friend_is_enemy(int cn, int cc)
{
	int co;

	if (!(co = ch[cn].attack_cn))	{ return(0); }
	if (may_attack_msg(cc, co, 0))	{ return(0); }
	return(1);
}

int player_or_ghost(int cn, int co)
{
	if (ch[co].flags&CF_SHADOWCOPY)	{ return(0); }
	if (!(ch[cn].flags&CF_PLAYER))	{ return(1); }
	if (ch[co].flags&CF_PLAYER) 	{ return(1); }
	if (ch[co].data[63]==cn)		{ return(1); }
	
	return(0);
}

int is_exhausted(int cn)
{
	int n, in;

	for (n = 0; n<MAXBUFFS; n++)
	{
		if ((in = ch[cn].spell[n])!=0 && bu[in].temp==SK_BLAST) { break; }
	}
	if (n<MAXBUFFS)
	{
		do_char_log(cn, 0, "You are still exhausted from your last skill!\n");
		return(1);
	}
	return(0);
}

int get_target(int cn, int cnts, int buff, int redir, int cost, int in, int usemana)
{
	int m, co;
	
	if (cnts && (ch[cn].flags & CF_PLAYER))
	{
		ch[cn].data[71] += CNTSAY;
		if (ch[cn].data[71]>MAXSAY)
		{
			do_char_log(cn, 0, "Oops, you're a bit too fast for me!\n");
			return 0;
		}
	}
	
	m = ch[cn].x + ch[cn].y * MAPX;
	
	if ((co = ch[cn].skill_target1)!=0) ;
	else if (!buff && ch[cn].dir==DX_DOWN  && (co = map[m + 1].ch)!=0) ;
	else if (!buff && ch[cn].dir==DX_UP    && (co = map[m - 1].ch)!=0) ;
	else if (!buff && ch[cn].dir==DX_RIGHT && (co = map[m + MAPX].ch)!=0) ;
	else if (!buff && ch[cn].dir==DX_LEFT  && (co = map[m - MAPX].ch)!=0) ;
	else if (!buff && (co = ch[cn].attack_cn)!=0) ;
	else co = cn;
	
	if (!do_char_can_see(cn, co))
	{
		do_char_log(cn, 0, "You cannot see your target.\n");
		return 0;
	}
	
	if (is_exhausted(cn))
	{ 
		return 0; 
	}
	
	if (spellcost(cn, cost, in, 1))
	{
		return 0; 
	}
	
	if (redir && !player_or_ghost(cn, co))
	{
		do_char_log(cn, 0, "Changed target of spell from %s to %s.\n", 
			ch[co].name, ch[cn].name);
		
		co = cn;
	}
	
	if (chance(cn, FIVE_PERC_FAIL))
	{
		if (cn!=co && 
			(get_skill_score(co, SK_SENSE) > get_skill_score(cn, in) + 5) && 
			!(ch[co].flags & CF_SENSE))
		{
			do_char_log(co, 1, "%s tried to cast %s on you but failed.\n", 
				ch[cn].reference, sp_name[in]);
		}
		return 0;
	}
	
	return co;
}

int make_new_buff(int cn, int intemp, int sptemp, int power, int dur, int ext)
{
	int in;
	
	in = god_create_buff();
	if (!in)
	{
		xlog("god_create_buff failed for spell_%s", sp_name[intemp]);
		return 0;
	}
	
	strcpy(bu[in].name, sp_name[intemp]);
	bu[in].flags 	|= IF_SPELL;
	bu[in].temp  	 = intemp;
	bu[in].sprite[1] = sptemp;
	bu[in].power 	 = power;
	
	if (ext && !(ch[cn].flags & CF_PLAYER))
	{
		bu[in].duration = bu[in].active = SP_DUR_MONSTERS;
	}
	else
	{
		bu[in].duration = bu[in].active = dur;
	}
	
	return in;
}

int has_buff(int cn, int bu_temp)
{
	int in, n;
	
	for (n = 0; n<MAXBUFFS; n++)
	{
		if ((in = ch[cn].spell[n])!=0)
		{
			if (bu[in].temp==bu_temp)
			{
				return in;
			}
		}
	}
	
	return 0;
}

int is_facing(int cn, int co)
{
	int ok = 0;

	switch(ch[cn].dir)
	{
		case    DX_RIGHT:
			if (ch[cn].x + 1==ch[co].x && ch[cn].y==ch[co].y)
			{
				ok = 1;
			}
			break;
		case    DX_LEFT:
			if (ch[cn].x - 1==ch[co].x && ch[cn].y==ch[co].y)
			{
				ok = 1;
			}
			break;
		case    DX_UP:
			if (ch[cn].x==ch[co].x && ch[cn].y - 1==ch[co].y)
			{
				ok = 1;
			}
			break;
		case    DX_DOWN:
			if (ch[cn].x==ch[co].x && ch[cn].y + 1==ch[co].y)
			{
				ok = 1;
			}
			break;
		default:
			break;
	}

	return(ok);
}

int is_back(int cn, int co)
{
	int ok = 0;

	switch(ch[cn].dir)
	{
		case    DX_LEFT:
			if (ch[cn].x + 1==ch[co].x && ch[cn].y==ch[co].y)
			{
				ok = 1;
			}
			break;
		case    DX_RIGHT:
			if (ch[cn].x - 1==ch[co].x && ch[cn].y==ch[co].y)
			{
				ok = 1;
			}
			break;
		case    DX_DOWN:
			if (ch[cn].x==ch[co].x && ch[cn].y - 1==ch[co].y)
			{
				ok = 1;
			}
			break;
		case    DX_UP:
			if (ch[cn].x==ch[co].x && ch[cn].y + 1==ch[co].y)
			{
				ok = 1;
			}
			break;
		default:
			break;
	}

	return(ok);
}

int spellcost(int cn, int cost, int in, int usemana)
{
	int cotfk_cost = 0;
	int devil_cost = 0;
	
	if (in != SK_BLAST && in != SK_CLEAVE && in != SK_WEAKEN && in != SK_WARCRY)
		cost = max(SP_COST_BASE, min(cost, cost*get_skill_score(cn, in)/100));
	
	// Devil Tarot Card
	if (get_tarot(cn, IT_CH_DEVIL))
	{
		devil_cost=cost*50/100;
	}
	
	if (usemana)
	{
		// Crown of the First King
		if (it[ch[cn].worn[WN_HEAD]].temp==IT_TW_CROWN)
		{
			cotfk_cost=cost*25/100;
		}
		
		cost-=cotfk_cost;
		cost-=devil_cost;
		
		if (has_spell_from_item(cn, BUF_IT_MANA))
		{
			cost = cost * 85 / 100;
		}
		if (ch[cn].skill[SK_CONCEN][0])
		{
			int t;
			
			if (it[ch[cn].worn[WN_LHAND]].temp==IT_BOOK_PROD) // Book: Great Prodigy
			{
				t = cost * get_skill_score(cn, SK_CONCEN) / 300;
			}
			else
			{
				t = cost * get_skill_score(cn, SK_CONCEN) / 400;
			}
			
			if (t>cost) { cost = 1; }
			else { cost -= t; }
		}
		
		if (devil_cost*1000 > ch[cn].a_hp)
		{
			do_char_log(cn, 0, "You don't have enough life.\n");
			return -1;
		}
		if (cotfk_cost*1000 > ch[cn].a_end)
		{
			do_char_log(cn, 0, "You don't have enough endurance.\n");
			return -1;
		}
		if (cost*1000 > ch[cn].a_mana)
		{
			do_char_log(cn, 0, "You don't have enough mana.\n");
			return -1;
		}
		
		ch[cn].a_mana -= cost*1000;
		ch[cn].a_end -= cotfk_cost*1000;
		ch[cn].a_hp -= devil_cost*1000;
	}
	else
	{
		cost-=devil_cost;
		
		if (has_spell_from_item(cn, BUF_IT_AALE))
		{
			cost = cost * 85 / 100;
		}
		
		if (devil_cost*1000 > ch[cn].a_hp)
		{
			do_char_log(cn, 0, "You don't have enough life.\n");
			return -1;
		}
		if (cost*1000 > ch[cn].a_end)
		{
			do_char_log(cn, 0, "You're too exhausted for that right now!\n");
			return -1;
		}
		
		ch[cn].a_end -= cost*1000;
		ch[cn].a_hp -= devil_cost*1000;
	}
	return 0;
}

int chance_base(int cn, int skill, int d20, int power)
{
	int chance, roll, tmp;

	chance = d20 * skill / max(1, power);

	if (ch[cn].flags & (CF_PLAYER))
	{
		if (ch[cn].luck<0)
		{
			chance += ch[cn].luck / 500 - 1;
		}
	}
	
	roll = RANDOM(20); // Lower result is better.
	
	if (ch[cn].flags & (CF_PLAYER)) // Players have small advantage
	{
		tmp = RANDOM(20);
		if (tmp < roll) roll = (tmp+roll)/2;
	}
	if (it[ch[cn].worn[WN_LHAND]].temp==IT_BOOK_ADVA) // Book: Castor's Advantage
	{
		tmp = RANDOM(20);
		if (tmp < roll) roll = tmp;
	}
	if (it[ch[cn].worn[WN_LHAND]].temp==IT_BOOK_SHIV && (d20==10||d20==12)) // Book: Shiva's Malice
	{
		tmp = RANDOM(20);
		if (tmp > roll) roll = tmp;
	}

	if (chance< 0) { chance =  0; }
	if (chance>18) { chance = 18; }

	if (roll>chance || power>skill + (skill / 2))
	{
		if (!(ch[cn].flags & (CF_PLAYER)))
		{
			add_exhaust(cn, TICKS * 4); // 4 second exhaust for NPCS to keep them from spam-failing
		}
		do_char_log(cn, 0, "Your spell fizzled!\n");
		return(-1);
	}
	return(0);
}

int chance(int cn, int d20)
{
	if (ch[cn].skill[SK_FOCUS][0]) return(0);
	
	if (ch[cn].flags & (CF_PLAYER))
	{
		// Bad luck gives worse dice
		if (ch[cn].luck<0) { d20 += ch[cn].luck / 500 - 1; }
	}

	if (d20<0)  { d20 =  0; }
	if (d20>18) { d20 = 18; }

	if (RANDOM(20)>d20)
	{
		do_char_log(cn, 0, "You lost your focus!\n");
		return(-1);
	}
	return(0);
}

void damage_mshell(int co)
{
	int tmp = 0, n, in;
	
	for (n = 0; n<MAXBUFFS; n++)
	{
		if ((in = ch[co].spell[n])!=0)
		{
			if (bu[in].temp==SK_MSHELL)
			{
				if (ch[co].kindred & KIN_SEYAN_DU) 
					tmp = (bu[in].active / 768 + 1)*5;
				else
					tmp = (bu[in].active / 512 + 1)*5;

				if (tmp>0)
				{
					if (tmp>=bu[in].active)
					{
						ch[co].spell[n] = 0;
						bu[in].used = 0;
						do_update_char(co);
					}
					else
					{
						bu[in].active -= tmp;
						if (ch[co].kindred & KIN_SEYAN_DU) 
						{
							bu[in].skill[SK_RESIST][1] = bu[in].active / 768 + 1;
							bu[in].skill[SK_IMMUN][1]  = bu[in].active / 768 + 1;
						}
						else
						{
							bu[in].skill[SK_RESIST][1] = bu[in].active / 512 + 1;
							bu[in].skill[SK_IMMUN][1]  = bu[in].active / 512 + 1;
						}
						bu[in].power = bu[in].active / 128;
						do_update_char(co);
					}
				}
			}
		}
	}
}

int get_target_resistance(int co)
{
	int target_resist = 1;
	
	// Tarot Card - Hanged Man :: Resistance behaves as extra Immunity instead
	if (get_tarot(co, IT_CH_HANGED))
		target_resist = (get_skill_score(co, SK_RESIST)*30/100);
	else
		target_resist = get_skill_score(co, SK_RESIST);
	
	return target_resist;
}

int get_target_immunity(int co)
{
	int target_immune = 1;
	
	// Tarot Card - Hanged Man :: Resistance behaves as extra Immunity instead
	if (get_tarot(co, IT_CH_HANGED))
		target_immune = get_skill_score(co, SK_IMMUN) + (get_skill_score(co, SK_RESIST)*30/100);
	else
		target_immune = get_skill_score(co, SK_IMMUN);
	
	return target_immune;
}

int spell_immunity(int power, int immun)
{
	immun /= 2;
	
	if (power<=immun)
		return(1);
	
	else
		return(power - immun);
}

int spell_race_mod(int power, int cn)
{
	int kindred;
	double mod;
	
	kindred = ch[cn].kindred;

		 if 	(kindred & KIN_TEMPLAR)		{ mod = 0.60; }
	else if 	(kindred & KIN_MERCENARY)	{ mod = 1.00; }
	else if 	(kindred & KIN_HARAKIM)		{ mod = 1.00; }

	else if 	(kindred & KIN_SEYAN_DU)	{ mod = 0.90; }
	else if 	(kindred & KIN_ARCHTEMPLAR)	{ mod = 0.80; }
	else if 	(kindred & KIN_PUGILIST)	{ mod = 0.80; }
	else if 	(kindred & KIN_WARRIOR)		{ mod = 1.05; }
	else if 	(kindred & KIN_SORCERER)	{ mod = 1.05; }
	else if 	(kindred & KIN_SUMMONER)	{ mod = 1.10; }
	else if 	(kindred & KIN_ARCHHARAKIM)	{ mod = 1.10; }
	else									{ mod = 1.00; }

	if (ch[cn].skill[SK_FOCUS][0])
	{
		mod = (mod*1500+get_skill_score(cn, SK_FOCUS))/1500;
	}
	
	if (globs->newmoon)		{ mod += 0.10; }
	if (globs->fullmoon)	{ mod += 0.15; }
	
	// Amulet of Adepts
	if (it[ch[cn].worn[WN_NECK]].temp==IT_RD_AMMYTHR)
	{
		mod += 0.05;
	}
	// Ring of Prophets
	if (it[ch[cn].worn[WN_LRING]].temp==IT_TW_PROPHET)
	{
		if (it[ch[cn].worn[WN_LRING]].active) 	mod += 0.10;
		else 									mod += 0.05;
	}
	// Ring of Prophets
	if (it[ch[cn].worn[WN_RRING]].temp==IT_TW_PROPHET)
	{
		if (it[ch[cn].worn[WN_RRING]].active) 	mod += 0.10;
		else 									mod += 0.05;
	}
	
	return((int)(power * mod));
}

int add_spell(int cn, int in)
{
	int n, in2, weak = 999, weakest = 99;
	int m;

	m = ch[cn].x + ch[cn].y * MAPX;
	if (map[m].flags & CF_NOMAGIC) { return(0); }

	// overwrite spells if same spell is cast twice and the new spell is more powerful
	for (n = 0; n<MAXBUFFS; n++)
	{
		if ((in2 = ch[cn].spell[n])!=0)
		{
			if (bu[in2].used==USE_EMPTY) continue;
			// Feb 2020 - special case for healing sickness
			// If you already have the first stack, add a new stack.
			if (bu[in].temp==SK_HEAL&&bu[in2].temp==SK_HEAL)
			{
				int sick=3;

				if (it[ch[cn].worn[WN_LHAND]].temp==IT_BOOK_HOLY) // Book: Holy Etiquette
				{
					sick=2;
				}
				
				bu[in].cost = bu[in2].cost + 1;
				if (bu[in].cost > 3) bu[in].cost = 3;
				else if (bu[in].cost > sick) bu[in].cost = sick;
				bu[in2].used = USE_EMPTY;
				break;
			}
			else if (bu[in2].temp==bu[in].temp)
			{
				int tickminimum = TICKS*60;
				
				if (bu[in2].temp==SK_POISON || bu[in2].temp==SK_SLOW || bu[in2].temp==SK_SLOW2 || bu[in2].temp==SK_CURSE2)
					tickminimum = TICKS*5;
				
				if (bu[in].power<bu[in2].power && bu[in2].active>tickminimum)
				{
					bu[in].used = USE_EMPTY;
					return 0;
				}
				bu[in2].used = USE_EMPTY;
				ch[cn].spell[n] = 0;
				break;
			}
		}
	}

	if (n==MAXBUFFS)
	{
		for (n = 0; n<MAXBUFFS; n++)
		{
			if (!(in2 = ch[cn].spell[n]))
			{
				break;
			}
			if (bu[in2].power<weak)
			{
				weak = bu[in2].power;
				weakest = n;
			}
		}
		if (n==MAXBUFFS)      // overwrite weakest spell if it is weaker than the new spell
		{
			if (weak<999 && weak<bu[in].power)
			{
				n = weakest;
				if ((in2 = ch[cn].spell[n])!=0)
				{
					bu[in2].used = USE_EMPTY;
					ch[cn].spell[n] = 0;
				}
			}
			else
			{
				bu[in].used = USE_EMPTY;
				return 0;
			}
		}
	}

	ch[cn].spell[n] = in;
	bu[in].carried  = cn;

	do_update_char(cn);

	return 1;
}

void add_exhaust(int cn, int len)
{
	int in=0, baselen=100;
	
	// Book: Damor's Grudge
	if (it[ch[cn].worn[WN_LHAND]].temp==IT_BOOK_DAMO)
	{
		baselen=90;
	}
	
	// Tarot - Magician - Effects of Strength and Intuition are swapped
	if (get_tarot(cn, IT_CH_MAGI))
	{
		len = len*(baselen-max(0,get_attrib_score(cn, AT_STR)-10)/4)/100;
	}
	else
	{
		len = len*(baselen-max(0,get_attrib_score(cn, AT_INT)-10)/4)/100;
	}
	
	in = god_create_buff();
	if (!in)
	{
		xlog("god_create_buff failed in add_exhaust");
		return;
	}
	
	// Trying to assure Exhaustion isn't 'permanent'
	if (len > SK_EXH_MAXIMUM) len = SK_EXH_MAXIMUM;
	
	strcpy(bu[in].name, "Skill Exhaustion");
	bu[in].flags 	|= IF_SPELL;
	bu[in].sprite[1] = BUF_SPR_EXHAUST;
	bu[in].duration  = bu[in].active = len;
	bu[in].temp  	 = SK_BLAST;
	bu[in].power 	 = 255;
	
	add_spell(cn, in);
}

void spell_from_item(int cn, int in2)
{
	int in, n;

	if (ch[cn].flags & CF_NOMAGIC)
	{
		do_char_log(cn, 0, "The magic didn't work! Must be external influences.\n");
		return;
	}

	in = god_create_buff();
	if (!in)
	{
		xlog("god_create_buff failed in skill_from_item");
		return;
	}

	strcpy(bu[in].name, it[in2].name);
	bu[in].flags |= IF_SPELL;

	bu[in].armor[1]			= it[in2].armor[1];
	bu[in].weapon[1]		= it[in2].weapon[1];

	bu[in].hp[1]			= it[in2].hp[1];
	bu[in].end[1]			= it[in2].end[1];
	bu[in].mana[1]			= it[in2].mana[1];
	bu[in].sprite_override	= it[in2].sprite_override;

	for (n = 0; n<5; n++) 			{ bu[in].attrib[n][1] = it[in2].attrib[n][1]; }
	for (n = 0; n<MAXSKILL; n++) 	{ bu[in].skill[n][1]  = it[in2].skill[n][1];  }

	if (it[in2].data[0]) 	{ bu[in].sprite[1] = it[in2].data[0]; }
	else 					{ bu[in].sprite[1] = BUF_SPR_GENERIC; }
	
	bu[in].duration 		= bu[in].active = it[in2].duration;
	
	if (it[in2].data[1]) 	{ bu[in].temp = it[in2].data[1]; }
	else 					{ bu[in].temp = 101; }
	
	bu[in].power 			= it[in2].power;
	
	bu[in].data[0]			= it[in2].temp;

	if (!add_spell(cn, in))
	{
		do_char_log(cn, 1, "Magical interference neutralized the %s's effect.\n", bu[in].name);
		return;
	}
	do_char_log(cn, 1, "You feel changed.\n");

	char_play_sound(cn, ch[cn].sound + 1, -150, 0);
}

int has_spell_from_item(int cn, int temp)
{
	int n, in;
	for (n = 0; n<MAXBUFFS; n++)
	{
		if (!ch[cn].spell[n])
		{
			continue;
		}
		in = ch[cn].spell[n];
		if (bu[in].data[0]==temp)
		{
			break;
		}
	}
	if (n<MAXBUFFS)
	{
		return 1;
	}
	return 0;
}

// debuff: 1 for normal debuff, 2 for AoE debuff (to remove interference spam)
int cast_a_spell(int cn, int co, int in, int debuff)
{
	char *log_name;
	char *log_sense, *log_self, *log_other;
	int arealog = 0;
	
	log_name = bu[in].name;
	
	switch (bu[in].temp)
	{
		case SK_LIGHT:
			log_sense 	= " cast light on you.";
			log_self 	= "You start to emit light.";
			log_other 	= " starts to emit light.";
			arealog = 1;
			break;
		case SK_PROTECT:
			log_sense 	= " cast protect on you.";
			log_self 	= "You feel protected.";
			log_other 	= " is now protected.";
			break;
		case SK_ENHANCE:
			log_sense 	= " cast enhance weapon on you.";
			log_self 	= "Your weapon feels stronger.";
			log_other 	= "'s weapon is now stronger.";
			break;
		case SK_BLESS:
			log_sense 	= " cast bless on you.";
			log_self 	= "You have been blessed.";
			log_other 	= " was blessed.";
			break;
		case SK_MSHIELD:
			log_sense 	= " cast magic shield on you.";
			log_self 	= "Magic Shield active!";
			log_other 	= "'s Magic Shield activated.";
			break;
		case SK_MSHELL:
			log_sense 	= " cast magic shell on you.";
			log_self 	= "Magic Shell active!";
			log_other 	= "'s Magic Shell activated.";
			break;
		case SK_HASTE:
			log_sense 	= " cast haste on you.";
			log_self 	= "You suddenly feel faster!";
			log_other 	= " has been hasted.";
			break;
		case SK_HEAL:
			log_name	= "Heal";
			log_sense 	= " cast heal on you.";
			log_self 	= "You have been healed.";
			log_other 	= " was healed.";
			break;
		case SK_REGEN:
			log_name	= "Regen";
			log_sense 	= " cast regen on you.";
			log_self 	= "You begin regenerating hitpoints!";
			log_other 	= " starts regenerating.";
			break;
			
		case SK_CURSE2:
			log_sense 	= " cast greater curse on you.";
			log_self 	= "You have been badly cursed.";
			log_other 	= " was badly cursed.";
			break;
		case SK_CURSE:
			log_sense 	= " cast curse on you.";
			log_self 	= "You have been cursed.";
			log_other 	= " was cursed.";
			break;
		
		default:
			log_sense 	= " cast something... strange on you.";
			log_self 	= "You feel... strange.";
			log_other 	= " has been spelled by... whatever that was.";
			arealog = 1;
			break;
	}
	
	if (cn!=co)
	{
		if (!add_spell(co, in))
		{
			if (debuff<2)
			{
				do_char_log(cn, 1, 
				"Magical interference neutralized the %s's effect.\n", log_name);
			}
			return 0;
		}
		if (get_skill_score(co, SK_SENSE) + 10 > bu[in].power)
		{
			do_char_log(co, 1, "%s%s\n", ch[cn].reference, log_sense);
		}
		else
		{
			do_char_log(co, debuff>0?0:1, "%s\n", log_self);
		}
		if (arealog) 
		{
			do_area_log(co, 0, ch[co].x, ch[co].y, 2, "%s%s\n", ch[co].name, log_other);
		}
		else if (debuff<2)
		{
			do_char_log(cn, 1, "%s%s\n", ch[co].name, log_other);
		}
		if (debuff)
		{
			if (!IS_IGNORING_SPELLS(co))
			{
				do_notify_char(co, NT_GOTHIT, cn, 0, 0, 0);
			}
			do_notify_char(cn, NT_DIDHIT, co, 0, 0, 0);
		}
		chlog(cn, "Cast %s on %s", log_name, ch[co].name);
		char_play_sound(cn, ch[cn].sound + 1, -150, 0);
		if (debuff)
		{
			char_play_sound(co, ch[cn].sound + 7, -150, 0);
			fx_add_effect(5, 0, ch[co].x, ch[co].y, 0);
		}
		else
		{
			char_play_sound(co, ch[cn].sound + 1, -150, 0);
			fx_add_effect(6, 0, ch[co].x, ch[co].y, 0);
		}
	}
	else
	{
		if (!add_spell(cn, in))
		{
			do_char_log(cn, 1, 
			"Magical interference neutralized the %s's effect.\n", bu[in].name);
			return 0;
		}
		do_char_log(cn, 1, "%s\n", log_self);
		if (ch[cn].flags & (CF_PLAYER))
		{
			chlog(cn, "Cast %s", bu[in].name);
		}
		char_play_sound(cn, ch[cn].sound + 1, -150, 0);
		fx_add_effect(6, 0, ch[cn].x, ch[cn].y, 0);
	}
	fx_add_effect(7, 0, ch[cn].x, ch[cn].y, 0);
	
	return 1;
}

int spellpower(int cn, int power, int in)
{
	int pow_wil, pow_int, n, m;
	int in2 = 0, tarot = 0;
	
	if (get_tarot(cn, IT_CH_MAGI))
		tarot = 1;
	
	pow_wil = ch[cn].attrib[AT_WIL][0];
	pow_int = tarot ? ch[cn].attrib[AT_STR][0] : ch[cn].attrib[AT_INT][0];
	
	return (spell_race_mod(pow_wil+pow_int, cn));
}

// Feb 2020 -- Added to clean up other blocks...
int spellpower_check(int cn, int co, int power, int in)
{
	int tmp;
	
	if (IS_PLAYER(co) || IS_COMP_TEMP(co))
	{
		if (cn!=co) 
		{
			if (IS_COMP_TEMP(co)) 
				tmp = spellpower(co, power, in)*2+get_attrib_score(cn, AT_WIL)/4;
			else 
				tmp = spellpower(co, power, in)+get_attrib_score(cn, AT_WIL)/4;
			
			if (power>tmp)
			{
				power = tmp;
				do_char_log(cn, 1, "Seeing that %s's mind cannot support the power of your spell, you reduced its strength.\n", ch[co].reference);
			}
		}
		else if ((ch[cn].kindred & (KIN_TEMPLAR | KIN_ARCHTEMPLAR | KIN_PUGILIST)) && !(ch[cn].kindred & KIN_SEYAN_DU))
		{
			tmp = spellpower(co, power, in);
			if (power>tmp)
			{
				power = tmp;
				do_char_log(cn, 1, "Your mind is not powerful enough to use the full strength of this spell.\n");
			}
		}
		else
		{
			tmp = spellpower(co, power, in)*2;
			if (power>tmp)
			{
				power = tmp;
				do_char_log(cn, 1, "Your mind is not powerful enough to use the full strength of this spell.\n");
			}
		}
	}
	
	return power;
}

int spell_light(int cn, int co, int power)
{
	int in;
	
	power = spell_race_mod(power, cn);
	
	if (!(in = make_new_buff(cn, SK_LIGHT, BUF_SPR_LIGHT, power, SP_DUR_LIGHT, 0))) 
		return 0;
	
	bu[in].light[1]  = min(250, power * 4);
	
	return cast_a_spell(cn, co, in, 0);
}
void skill_light(int cn)
{
	int co;
	
	if (!(co = get_target(cn, 1, 1, 0, SP_COST_LIGHT, SK_LIGHT, 1))) 
		return;
	
	spell_light(cn, co, get_skill_score(cn, SK_LIGHT));

	add_exhaust(cn, SK_EXH_LIGHT);
}

int spell_protect(int cn, int co, int power)
{
	int in;
	
	power = spellpower_check(cn, co, spell_race_mod(power, cn), 0);

	if (!(in = make_new_buff(cn, SK_PROTECT, BUF_SPR_PROTECT, power, SP_DUR_PROTECT, 1))) 
		return 0;

	if (ch[co].kindred & KIN_SEYAN_DU)
	{
		bu[in].armor[1]  = power / 6 + 3;
	}
	else
	{
		bu[in].armor[1]  = power / 4 + 4;
	}

	return cast_a_spell(cn, co, in, 0);
}
void skill_protect(int cn)
{
	int co;
	
	if (!(co = get_target(cn, 0, 1, 1, SP_COST_PROTECT, SK_PROTECT, 1))) 
		return;

	spell_protect(cn, co, get_skill_score(cn, SK_PROTECT));

	add_exhaust(cn, SK_EXH_PROTECT);
}

int spell_enhance(int cn, int co, int power)
{
	int in;
	
	power = spellpower_check(cn, co, spell_race_mod(power, cn), 0);
	
	if (!(in = make_new_buff(cn, SK_ENHANCE, BUF_SPR_ENHANCE, power, SP_DUR_ENHANCE, 1))) 
		return 0;
	
	if (ch[co].kindred & KIN_SEYAN_DU)
	{
		bu[in].weapon[1] = power / 6 + 3;
	}
	else
	{
		bu[in].weapon[1] = power / 4 + 4;
	}
	
	return cast_a_spell(cn, co, in, 0);
}
void skill_enhance(int cn)
{
	int co;
	
	if (!(co = get_target(cn, 0, 1, 1, SP_COST_ENHANCE, SK_ENHANCE, 1))) 
		return;
	
	spell_enhance(cn, co, get_skill_score(cn, SK_ENHANCE));

	add_exhaust(cn, SK_EXH_ENHANCE); // Half-second
}

int spell_bless(int cn, int co, int power)
{
	int in, n;
	
	power = spellpower_check(cn, co, spell_race_mod(power, cn), 1);
	
	if (!(in = make_new_buff(cn, SK_BLESS, BUF_SPR_BLESS, power, SP_DUR_BLESS, 1))) 
		return 0;
	
	for (n = 0; n<5; n++) 
	{
		bu[in].attrib[n][1] = ((power*2/3)-n) / 5 + 3;
	}
	
	return cast_a_spell(cn, co, in, 0);
}
void skill_bless(int cn)
{
	int co;
	
	if (!(co = get_target(cn, 0, 1, 1, SP_COST_BLESS, SK_BLESS, 1))) 
		return;
	
	spell_bless(cn, co, get_skill_score(cn, SK_BLESS));

	add_exhaust(cn, SK_EXH_BLESS);
}

int spell_mshield(int cn, int co, int power)
{
	int in, ta_cn_cha, ta_co_emp, cc;
	
	power = spellpower_check(cn, co, spell_race_mod(power, cn), 0);
	
	ta_cn_cha = get_tarot(cn, IT_CH_CHARIOT);
	ta_co_emp = get_tarot(co, IT_CH_EMPRESS);
	
	if (ta_cn_cha)
	{
		in = make_new_buff(cn, SK_MSHELL, BUF_SPR_MSHELL, power, SP_DUR_MSHELL(power), 1);
	}
	else
	{
		in = make_new_buff(cn, SK_MSHIELD, BUF_SPR_MSHIELD, power, SP_DUR_MSHIELD(power), 1);
	}
	
	if (!in) 
		return 0;
	
	// Tarot Card - Empress :: Change MS target to the target's Ghost Companion instead
	if (ta_co_emp)
	{
		if ((ch[co].flags & CF_PLAYER) && (cc = ch[co].data[CHD_COMPANION]))
		{
			if (!IS_SANECHAR(cc) || ch[cc].data[63]!=co || (ch[cc].flags & CF_BODY) || 
				ch[cc].used==USE_EMPTY)
			{
				cc = 0;
			}
			if (cc)
			{
				co = cc;
			}
			else
			{
				if (ta_cn_cha)
				{
					do_char_log(co, 0, 
					"Since you lack a Ghost Companion, the Magic Shell's power was halved.\n");
				}
				else
				{
					do_char_log(co, 0, 
					"Since you lack a Ghost Companion, the Magic Shield's power was halved.\n");
				}
				bu[in].duration /= 2;
				bu[in].active   /= 2;
			}
		}
		else
		{
			if (ta_cn_cha)
			{
				do_char_log(co, 0, 
				"Since you lack a Ghost Companion, the Magic Shell's power was halved.\n");
			}
			else
			{
				do_char_log(co, 0, 
				"Since you lack a Ghost Companion, the Magic Shield's power was halved.\n");
			}
			bu[in].duration /= 2;
			bu[in].active   /= 2;
		}
	}
	
	// Tarot Card - Chariot :: Change Magic Shield into Magic Shell
	if (ta_cn_cha)
	{
		bu[in].power = bu[in].active / 128;
		if (ch[co].kindred & KIN_SEYAN_DU) 
		{
			bu[in].skill[SK_RESIST][1] = bu[in].active / 768 + 1;
			bu[in].skill[SK_IMMUN][1]  = bu[in].active / 768 + 1;
		}
		else
		{
			bu[in].skill[SK_RESIST][1] = bu[in].active / 512 + 1;
			bu[in].skill[SK_IMMUN][1]  = bu[in].active / 512 + 1;
		}
	}
	else
	{
		bu[in].power = bu[in].active / 256;
		if (ch[co].kindred & KIN_SEYAN_DU) 
		{
			bu[in].armor[1]  = bu[in].active / 1536 + 1;
		}
		else
		{
			bu[in].armor[1]  = bu[in].active / 1024 + 1;
		}
	}
	
	return cast_a_spell(cn, co, in, 0);
}
void skill_mshield(int cn)
{
	if (is_exhausted(cn)) 								{ return; }
	if (spellcost(cn, SP_COST_MSHIELD, SK_MSHIELD, 1))	{ return; }
	if (chance(cn, FIVE_PERC_FAIL)) 					{ return; }

	spell_mshield(cn, cn, get_skill_score(cn, SK_MSHIELD));

	add_exhaust(cn, SK_EXH_MSHIELD);
}

// Feb 2020 -- Haste
int spell_haste(int cn, int co, int power)
{
	int in;
	
	power = spellpower_check(cn,co,spell_race_mod(power, cn),0);
	
	if (!(in = make_new_buff(cn, SK_HASTE, BUF_SPR_HASTE, power, SP_DUR_HASTE(power), 1))) 
		return 0;
	
	return cast_a_spell(cn, co, in, 0);
}
void skill_haste(int cn)
{
	if (is_exhausted(cn)) 							{ return; }
	if (spellcost(cn, SP_COST_HASTE, SK_HASTE, 1))	{ return; }
	if (chance(cn, FIVE_PERC_FAIL)) 				{ return; }

	spell_haste(cn, cn, get_skill_score(cn, SK_HASTE));

	add_exhaust(cn, SK_EXH_HASTE);
}

// Oct 2020 - Regen via Tarot Card
int spell_regen(int cn, int co, int power)
{
	int in;
	
	power = spell_race_mod(power, cn);
	
	if (!(in = make_new_buff(cn, SK_REGEN, BUF_SPR_REGEN, power, SP_DUR_REGEN, 0))) 
		return 0;
	
	bu[in].hp[0] = (power * 1875) / SP_DUR_REGEN;
	
	return cast_a_spell(cn, co, in, 0);
}
int spell_heal(int cn, int co, int power)
{
	int in, in2, n, tmp;
	
	if (!(in = make_new_buff(cn, SK_HEAL, BUF_SPR_HEALSICK, power, SP_DUR_HEAL, 0))) 
		return 0;
	
	bu[in].cost = 0;
	
	// Every time heal is cast it updates itself and adds 1 to 'cost'
	if (cast_a_spell(cn, co, in, 0))
	{
		if ((in2=has_buff(cn, SK_HEAL))!=0)
		{
			// Each stack of heal sickness reduces the spell power by 1/4th
			tmp = 4 - bu[in2].cost;
			power = power * tmp / 4;
			if (4-tmp) 
				do_char_log(cn, 1, "Heal's power was reduced by %d%%\n", 100-tmp*25);
			break;
		}
	}
	
	if (cn!=co)
	{
		ch[co].a_hp += spell_race_mod(power * 2500, cn);
	}
	else
	{
		ch[co].a_hp += power * 2500;
	}
	
	if (ch[co].a_hp > ch[co].hp[5] * 1000)
	{
		ch[co].a_hp = ch[co].hp[5] * 1000;
	}
	
	return 1;
}
void skill_heal(int cn)
{
	int co;
	
	if (!(co = get_target(cn, 0, 1, 1, SP_COST_HEAL, SK_HEAL, 1))) 
		return;
	
	// Tarot Card - Star :: Change Heal into Regen
	if (get_tarot(cn, IT_CH_STAR))
	{
		spell_regen(cn, co, get_skill_score(cn, SK_HEAL));
	}
	else
	{
		spell_heal(cn, co, get_skill_score(cn, SK_HEAL));
	}

	add_exhaust(cn, SK_EXH_HEAL);
}

int spell_curse(int cn, int co, int power, int flag)
{
	int in, n;

	if (ch[cn].attack_cn!=co && ch[co].alignment==10000) { return 0; }
	if (ch[co].flags & CF_IMMORTAL) { return 0; }

	power = spell_immunity(power, get_target_immunity(co));
	power = spell_race_mod(power, cn);
	
	// Tarot Card - Tower :: Change Curse into Curse II
	if (get_tarot(cn, IT_CH_TOWER))
	{
		if (!(in = make_new_buff(cn, SK_CURSE2, BUF_SPR_CURSE2, power, SP_DUR_CURSE2, 0)))
			return 0;
		
		for (n = 0; n<5; n++) 
		{
			bu[in].attrib[n][1] = -(2 + CURSE2FORM(power, (4 - n)));
		}
		bu[in].cost  = power*100;
	}
	else
	{
		if (!(in = make_new_buff(cn, SK_CURSE, BUF_SPR_CURSE, power, SP_DUR_CURSE, 0)))
			return 0;
		
		for (n = 0; n<5; n++)
		{
			bu[in].attrib[n][1] = -(2+(power-(4-n)) / 5);
		}
	}

	return cast_a_spell(cn, co, in, 1+flag);
}
void skill_curse(int cn)
{
	int d20 = 10;
	int n, cost, power, spellaoe, xf, yf, xt, yt, x, y, count = 0, hit = 0, aoefocus = 0; // Added for AoE Stuff
	int co, co_orig = -1, m;

	if ((co = ch[cn].skill_target1)) { ; }
	else if (ch[cn].attack_cn!=0) { co = ch[cn].attack_cn; }
	else { co = cn; }
	
	cost = SP_COST_CURSE;
	if (get_tarot(cn, IT_CH_TOWER)) { cost *= (4/3); d20-=1; }
	if (ch[cn].skill[SK_HEXAREA][0] && !(ch[cn].flags & CF_AREA_OFF)) cost = cost * (PROXIMITY_MULTI+get_skill_score(cn, SK_HEXAREA)) / PROXIMITY_MULTI;
	power = get_skill_score(cn, SK_CURSE);
	
	if (cn==co && (!ch[cn].skill[SK_HEXAREA][0] || (ch[cn].flags & CF_AREA_OFF)))
	{
		do_char_log(cn, 0, "You cannot curse yourself!\n");
		return;
	}
	else if ((co==ch[cn].data[CHD_SHADOWCOPY] || co==ch[cn].data[CHD_COMPANION]) && (!ch[cn].skill[SK_HEXAREA][0] || (ch[cn].flags & CF_AREA_OFF)))
	{ 
		do_char_log(cn, 0, "You stop yourself from cursing your companion. That would be silly.\n");
		return;
	}
	else if (cn!=co && co!=ch[cn].data[CHD_SHADOWCOPY] && co!=ch[cn].data[CHD_COMPANION])
	{
		if (!do_char_can_see(cn, co))
		{
			do_char_log(cn, 0, "You cannot see your target.\n");
			return;
		}
		
		/* CS, 000209: Remember PvP attacks */
		remember_pvp(cn, co);
		
		if (is_exhausted(cn)) 					{ return; }
		if (spellcost(cn, cost, SK_CURSE, 1)) 	{ return; }
		
		if (!may_attack_msg(cn, co, 1))
		{
			chlog(cn, "Prevented from attacking %s (%d)", ch[co].name, co);
			return;
		}
		
		damage_mshell(co);
		
		if (chance_base(cn, power, d20, get_target_resistance(co)))
		{
			if (cn!=co && get_skill_score(co, SK_SENSE)>power + 5)
			{
				if (!(ch[co].flags & CF_SENSE))
					do_char_log(co, 0, "%s tried to cast curse on you but failed.\n", ch[cn].reference);
				if (!IS_IGNORING_SPELLS(co))
				{
					do_notify_char(co, NT_GOTMISS, cn, 0, 0, 0);
				}
			}
			if (ch[cn].kindred&KIN_MONSTER) add_exhaust(cn,TICKS*2);
			if (it[ch[cn].worn[WN_LHAND]].temp==IT_BOOK_SHIV) // Book: Shiva's Malice
			{
				skill_slow(cn, 0);
			}
			return;
		}
		
		spell_curse(cn, co, power, 0);
		
		co_orig = co;
		count++;
		hit++;
		
		if (ch[co].flags & CF_IMMORTAL)
		{
			do_char_log(cn, 0, "You lost your focus.\n");
			return;
		}
	}
	// Feb 2020 - AoE Curses
	if (ch[cn].skill[SK_HEXAREA][0] && !(ch[cn].flags & CF_AREA_OFF))
	{
		if (co) co_orig = co;
		
		spellaoe = get_skill_score(cn, SK_HEXAREA)/PROXIMITY_CAP; //get_spell_aoe(get_skill_score(cn, SK_HEXAREA));
		
		xf = max(1, ch[cn].x - spellaoe);
		yf = max(1, ch[cn].y - spellaoe);
		xt = min(MAPX - 1, ch[cn].x + spellaoe+1);
		yt = min(MAPY - 1, ch[cn].y + spellaoe+1);
		
		// Loop through and count the number of targets first
		for (x = xf; x<xt; x++) for (y = yf; y<yt; y++) if ((co = map[x + y * MAPX].ch)) if (cn!=co && co_orig!=co) 
		{ 
			if (!do_surround_check(cn, co, 0)) continue;
			count++; 
		}
		if (!count)
		{ 
			if (co_orig==ch[cn].data[CHD_SHADOWCOPY] || co_orig==ch[cn].data[CHD_COMPANION])
			{ do_char_log(cn, 0, "You stop yourself from cursing your companion. That would be silly.\n"); return; }
			else
			{ do_char_log(cn, 0, "You cannot curse yourself!\n"); return; }
		}
		if (!hit)
		{
			if (is_exhausted(cn)) 					return;
			if (spellcost(cn, cost, SK_CURSE, 1)) 	return;
			if (chance(cn, FIVE_PERC_FAIL))			return;
		}
		
		// Then loop through and apply the effect based off the number of targets
		for (x = xf; x<xt; x++)	for (y = yf; y<yt; y++)	if ((co = map[x + y * MAPX].ch))
		if (cn!=co && co_orig!=co)
		{
			remember_pvp(cn, co);
			if (!do_surround_check(cn, co, 1)) continue;
			damage_mshell(co);
			if (power+RANDOM(max(1,24-max(4,count/2))) > get_target_resistance(co)+RANDOM(max(1,12+max(4,count/2))))
			{
				spell_curse(cn, co, power, 1);
				hit++;
			}
			else
			{
				if (cn!=co && get_skill_score(co, SK_SENSE)>power + 5)
				{
					if (!(ch[co].flags & CF_SENSE))
						do_char_log(co, 0, "%s tried to include you in a mass-curse but failed.\n", ch[cn].reference);
					if (!IS_IGNORING_SPELLS(co))
						do_notify_char(co, NT_GOTMISS, cn, 0, 0, 0);
				}
				else
					do_char_log(co, 0, "You feel a wiked power emanate from somewhere.\n");
			}
		}
		do_char_log(cn, 1, "You unleash a powerful mass-curse.\n");
		do_char_log(cn, 1, "You affected %d of %d creatures in range.\n", hit, count);
	}
	else
	{
		m = ch[cn].x + ch[cn].y * MAPX;
		if ((co = map[m + 1].ch)!=0 && ch[co].attack_cn==cn && co_orig!=co)
		{
			damage_mshell(co);
			if (power+RANDOM(20) > get_target_resistance(co)+RANDOM(16)) spell_curse(cn, co, power, 0);
		}
		if ((co = map[m - 1].ch)!=0 && ch[co].attack_cn==cn && co_orig!=co)
		{
			damage_mshell(co);
			if (power+RANDOM(20) > get_target_resistance(co)+RANDOM(16)) spell_curse(cn, co, power, 0);
		}
		if ((co = map[m + MAPX].ch)!=0 && ch[co].attack_cn==cn && co_orig!=co)
		{
			damage_mshell(co);
			if (power+RANDOM(20) > get_target_resistance(co)+RANDOM(16)) spell_curse(cn, co, power, 0);
		}
		if ((co = map[m - MAPX].ch)!=0 && ch[co].attack_cn==cn && co_orig!=co)
		{
			damage_mshell(co);
			if (power+RANDOM(20) > get_target_resistance(co)+RANDOM(16)) spell_curse(cn, co, power, 0);
		}
	}

	fx_add_effect(7, 0, ch[cn].x, ch[cn].y, 0);

	if (it[ch[cn].worn[WN_LHAND]].temp==IT_BOOK_SHIV) // Book: Shiva's Malice
		skill_slow(cn, 1);
	else
		add_exhaust(cn, SK_EXH_CURSE);
}
// Feb 2020 - Slow
int spell_slow(int cn, int co, int power, int flag)
{
	int in, n;

	if (ch[cn].attack_cn!=co && ch[co].alignment==10000) { return 0; }
	if (ch[co].flags & CF_IMMORTAL) { return 0; }

	in = god_create_buff();
	if (!in)
	{
		xlog("god_create_buff failed in spell_slow");
		return( 0);
	}

	power = spell_immunity(power, get_target_immunity(co));
	power = spell_race_mod(power, cn);
	
	// Tarot Card - Emperor :: Change Slow into Slow II
	if (get_tarot(cn, IT_CH_EMPEROR))
	{
		strcpy(bu[in].name, "Greater Slow");
		bu[in].flags |= IF_SPELL;
		bu[in].sprite[1] = BUF_SPR_SLOW2;
		bu[in].duration  = bu[in].active = SP_DUR_SLOW2(power);
		bu[in].temp  = SK_SLOW2;
		bu[in].power = bu[in].cost = power;
		//bu[in].cost  = SLOW2FORM(power); // power of the slowness, gradually reduced over time

		if (!add_spell(co, in))
		{
			if (!flag) 
				do_char_log(cn, 1, "Magical interference neutralized the %s's effect.\n", bu[in].name);
			return 0;
		}
		if (get_skill_score(co, SK_SENSE) + 10>power)
			do_char_log(co, 1, "%s cast greater slow on you.\n", ch[cn].reference);
		else
			do_char_log(co, 0, "You have been badly slowed.\n");
		if (!flag) 
			do_char_log(cn, 1, "%s was badly slowed.\n", ch[co].name);
		if (!IS_IGNORING_SPELLS(co))
			do_notify_char(co, NT_GOTHIT, cn, 0, 0, 0);
		do_notify_char(cn, NT_DIDHIT, co, 0, 0, 0);
		char_play_sound(co, ch[cn].sound + 7, -150, 0);
		char_play_sound(cn, ch[cn].sound + 1, -150, 0);
		chlog(cn, "Cast Greater Slow on %s", ch[co].name);
		fx_add_effect(5, 0, ch[co].x, ch[co].y, 0);
	}
	else
	{
		strcpy(bu[in].name, "Slow");
		bu[in].flags |= IF_SPELL;
		bu[in].sprite[1] = BUF_SPR_SLOW;
		bu[in].duration  = bu[in].active = SP_DUR_SLOW(power);
		bu[in].temp  = SK_SLOW;
		bu[in].power = bu[in].cost = power;
		//bu[in].cost = SLOWFORM(power); // power of the slowness, gradually reduced over time

		if (!add_spell(co, in))
		{
			if (!flag) 
				do_char_log(cn, 1, "Magical interference neutralized the %s's effect.\n", bu[in].name);
			return 0;
		}
		if (get_skill_score(co, SK_SENSE) + 10>power)
			do_char_log(co, 1, "%s cast slow on you.\n", ch[cn].reference);
		else
			do_char_log(co, 0, "You have been slowed.\n");
		if (!flag) 
			do_char_log(cn, 1, "%s was slowed.\n", ch[co].name);
		if (!IS_IGNORING_SPELLS(co))
			do_notify_char(co, NT_GOTHIT, cn, 0, 0, 0);
		do_notify_char(cn, NT_DIDHIT, co, 0, 0, 0);
		char_play_sound(co, ch[cn].sound + 7, -150, 0);
		char_play_sound(cn, ch[cn].sound + 1, -150, 0);
		chlog(cn, "Cast Slow on %s", ch[co].name);
		fx_add_effect(5, 0, ch[co].x, ch[co].y, 0);
	}

	return 1;
}
void skill_slow(int cn, int flag)
{
	int d20 = 12;
	int n, cost, power, spellaoe, xf, yf, xt, yt, x, y, count = 0, hit = 0, aoefocus = 0; // Added for AoE Stuff
	int co, co_orig = -1, m;

	if ((co = ch[cn].skill_target1)) { ; }
	else if (ch[cn].attack_cn!=0) { co = ch[cn].attack_cn; }
	else { co = cn; }

	cost = SP_COST_SLOW;
	if (get_tarot(cn, IT_CH_EMPEROR)) { cost *= (4/3); d20 -=1; }
	if (ch[cn].skill[SK_HEXAREA][0] && !(ch[cn].flags & CF_AREA_OFF)) cost = cost * (PROXIMITY_MULTI+get_skill_score(cn, SK_HEXAREA)) / PROXIMITY_MULTI;
	power = get_skill_score(cn, SK_SLOW);
	
	if (cn==co && (!ch[cn].skill[SK_HEXAREA][0] || (ch[cn].flags & CF_AREA_OFF)))
	{
		do_char_log(cn, 0, "You cannot slow yourself!\n");
		return;
	}
	else if ((co==ch[cn].data[CHD_SHADOWCOPY] || co==ch[cn].data[CHD_COMPANION]) && (!ch[cn].skill[SK_HEXAREA][0] || (ch[cn].flags & CF_AREA_OFF)))
	{ 
		do_char_log(cn, 0, "You stop yourself from slowing your companion. That would be silly.\n");
		return;
	}
	else if (cn!=co && co!=ch[cn].data[CHD_SHADOWCOPY] && co!=ch[cn].data[CHD_COMPANION])
	{
		if (!do_char_can_see(cn, co))
		{
			do_char_log(cn, 0, "You cannot see your target.\n");
			return;
		}
		
		/* CS, 000209: Remember PvP attacks */
		remember_pvp(cn, co);
		
		if (is_exhausted(cn)) 					{ return; }
		if (spellcost(cn, cost, SK_SLOW, 1)) 	{ if (flag) add_exhaust(cn,TICKS*4); return; }
		
		if (!may_attack_msg(cn, co, 1))
		{
			chlog(cn, "Prevented from attacking %s (%d)", ch[co].name, co);
			return;
		}
		
		damage_mshell(co);
		if (chance_base(cn, power, d20, get_target_resistance(co)))
		{
			if (cn!=co && get_skill_score(co, SK_SENSE)>power + 5)
			{
				if (!(ch[co].flags & CF_SENSE))
					do_char_log(co, 0, "%s tried to cast slow on you but failed.\n", ch[cn].reference);
				if (!IS_IGNORING_SPELLS(co))
				{
					do_notify_char(co, NT_GOTMISS, cn, 0, 0, 0);
				}
			}
			if (ch[cn].kindred&KIN_MONSTER) add_exhaust(cn,TICKS*2);
			if (flag) add_exhaust(cn,TICKS*4);
			return;
		}
		
		spell_slow(cn, co, power, 0);
		
		co_orig = co;
		count++;
		hit++;
		
		if (ch[co].flags & CF_IMMORTAL)
		{
			do_char_log(cn, 0, "You lost your focus.\n");
			if (flag) add_exhaust(cn,TICKS*4);
			return;
		}
	}

	// Feb 2020 - AoE Slows
	if (ch[cn].skill[SK_HEXAREA][0] && !(ch[cn].flags & CF_AREA_OFF))
	{
		if (co) co_orig = co;
		
		spellaoe = get_skill_score(cn, SK_HEXAREA)/PROXIMITY_CAP; //get_spell_aoe(get_skill_score(cn, SK_HEXAREA));
		
		xf = max(1, ch[cn].x - spellaoe);
		yf = max(1, ch[cn].y - spellaoe);
		xt = min(MAPX - 1, ch[cn].x + spellaoe+1);
		yt = min(MAPY - 1, ch[cn].y + spellaoe+1);
		
		// Loop through and count the number of targets first
		for (x = xf; x<xt; x++) for (y = yf; y<yt; y++) if ((co = map[x + y * MAPX].ch)) if (cn!=co && co_orig!=co) 
		{ 
			if (!do_surround_check(cn, co, 0)) continue;
			count++; 
		}
		if (!count)
		{ 
			if (co_orig==ch[cn].data[CHD_SHADOWCOPY] || co_orig==ch[cn].data[CHD_COMPANION])
			{ do_char_log(cn, 0, "You stop yourself from slowing your companion. That would be silly.\n"); return; }
			else
			{ do_char_log(cn, 0, "You cannot slow yourself!\n"); return; }
		}
		if (!hit)
		{
			if (is_exhausted(cn)) 					return;
			if (spellcost(cn, cost, SK_SLOW, 1))	{ if (flag) add_exhaust(cn,TICKS*4); return; }
			if (chance(cn, FIVE_PERC_FAIL))			{ if (flag) add_exhaust(cn,TICKS*4); return; }
		}
		// Then loop through and apply the effect based off the number of targets
		for (x = xf; x<xt; x++)	for (y = yf; y<yt; y++)	if ((co = map[x + y * MAPX].ch))
		if (cn!=co && co_orig!=co)
		{
			remember_pvp(cn, co);
			if (!do_surround_check(cn, co, 1)) continue;
			damage_mshell(co);
			if (power+2+RANDOM(max(1,24-max(4,count/2))) > get_target_resistance(co)+RANDOM(max(1,12+max(4,count/2))))
			{
				spell_slow(cn, co, power, 1);
				hit++;
			}
			else
			{
				if (cn!=co && get_skill_score(co, SK_SENSE)>power + 5)
				{
					if (!(ch[co].flags & CF_SENSE))
						do_char_log(co, 0, "%s tried to include you in a mass-slow but failed.\n", ch[cn].reference);
					if (!IS_IGNORING_SPELLS(co))
						do_notify_char(co, NT_GOTMISS, cn, 0, 0, 0);
				}
				else
					do_char_log(co, 0, "You feel a chilling power emanate from somewhere.\n");
			}
		}
		do_char_log(cn, 1, "You unleash a powerful mass-slow.\n");
		do_char_log(cn, 1, "You affected %d of %d creatures in range.\n", hit, count);
	}
	else
	{
		m = ch[cn].x + ch[cn].y * MAPX;
		if ((co = map[m + 1].ch)!=0 && ch[co].attack_cn==cn && co_orig!=co)
		{
			damage_mshell(co);
			if (power+RANDOM(20) > get_target_resistance(co)+RANDOM(16)) spell_slow(cn, co, power, 0);
		}
		if ((co = map[m - 1].ch)!=0 && ch[co].attack_cn==cn && co_orig!=co)
		{
			damage_mshell(co);
			if (power+RANDOM(20) > get_target_resistance(co)+RANDOM(16)) spell_slow(cn, co, power, 0);
		}
		if ((co = map[m + MAPX].ch)!=0 && ch[co].attack_cn==cn && co_orig!=co)
		{
			damage_mshell(co);
			if (power+RANDOM(20) > get_target_resistance(co)+RANDOM(16)) spell_slow(cn, co, power, 0);
		}
		if ((co = map[m - MAPX].ch)!=0 && ch[co].attack_cn==cn && co_orig!=co)
		{
			damage_mshell(co);
			if (power+RANDOM(20) > get_target_resistance(co)+RANDOM(16)) spell_slow(cn, co, power, 0);
		}
	}

	fx_add_effect(7, 0, ch[cn].x, ch[cn].y, 0);
	if (flag)
		add_exhaust(cn, SK_EXH_CURSE + SK_EXH_SLOW);
	else
		add_exhaust(cn, SK_EXH_SLOW);
}

// Feb 2020 - Poison
int spell_poison(int cn, int co, int power, int flag)
{
	int in, n, dur;

	if (ch[cn].attack_cn!=co && ch[co].alignment==10000) { return 0; }
	if (ch[co].flags & CF_IMMORTAL) { return 0; }

	in = god_create_buff();
	if (!in)
	{
		xlog("god_create_buff failed in spell_poison");
		return( 0);
	}

	power = spell_immunity(power, get_target_immunity(co));
	power = spell_race_mod(power, cn);
	
	dur = SP_DUR_POISON(power);
	
	if (it[ch[cn].worn[WN_LHAND]].temp==IT_BOOK_VENO) // Book: Venom Compendium
	{
		dur = dur*70/100; // 70% duration = between 14 and 42 seconds
	}
	
	strcpy(bu[in].name, "Poison");
	bu[in].flags |= IF_SPELL;
	bu[in].sprite[1] = BUF_SPR_POISON;
	bu[in].duration  = bu[in].active = dur; 
	bu[in].temp  = SK_POISON;
	bu[in].power = power;
	bu[in].cost = cn; // Special case -- the user of the poison should earn the kill for the poison.

	if (!add_spell(co, in))
	{
		if (!flag) 
			do_char_log(cn, 1, "Magical interference neutralized the %s's effect.\n", bu[in].name);
		return 0;
	}
	if (get_skill_score(co, SK_SENSE) + 10>power)
		do_char_log(co, 1, "%s cast poison on you.\n", ch[cn].reference);
	else
		do_char_log(co, 0, "You have been poisoned!\n");
	if (!flag) 
		do_char_log(cn, 1, "%s was poisoned.\n", ch[co].name);
	if (!IS_IGNORING_SPELLS(co))
		do_notify_char(co, NT_GOTHIT, cn, 0, 0, 0);
	do_notify_char(cn, NT_DIDHIT, co, 0, 0, 0);
	char_play_sound(co, ch[cn].sound + 7, -150, 0);
	char_play_sound(cn, ch[cn].sound + 1, -150, 0);
	chlog(cn, "Cast Poison on %s", ch[co].name);
	fx_add_effect(5, 0, ch[co].x, ch[co].y, 0);

	return 1;
}
void skill_poison(int cn)
{
	int d20 = 13;
	int n, cost, power, dampower, spellaoe, xf, yf, xt, yt, x, y, count = 0, hit = 0, aoefocus = 0; // Added for AoE Stuff
	int co, co_orig = -1, m;

	if ((co = ch[cn].skill_target1)) { ; }
	else if (ch[cn].attack_cn!=0) { co = ch[cn].attack_cn; }
	else { co = cn; }
	
	cost = SP_COST_POISON;
	if ((ch[cn].flags & CF_PLAYER) && (ch[cn].kindred & KIN_SORCERER)) cost /= 2;
	if (ch[cn].skill[SK_HEXAREA][0] && !(ch[cn].flags & CF_AREA_OFF)) cost = cost * (PROXIMITY_MULTI+get_skill_score(cn, SK_HEXAREA)) / PROXIMITY_MULTI;
	else if (ch[cn].skill[SK_DAMAREA][0] && !(ch[cn].flags & CF_AREA_OFF)) cost = cost * (PROXIMITY_MULTI+get_skill_score(cn, SK_DAMAREA)) / PROXIMITY_MULTI;
	dampower = power = get_skill_score(cn, SK_POISON);
	
	if (cn==co && (!(ch[cn].skill[SK_HEXAREA][0] || ch[cn].skill[SK_DAMAREA][0]) || (ch[cn].flags & CF_AREA_OFF)))
	{
		do_char_log(cn, 0, "You cannot poison yourself!\n");
		return;
	}
	else if ((co==ch[cn].data[CHD_SHADOWCOPY] || co==ch[cn].data[CHD_COMPANION]) 
		&& (!(ch[cn].skill[SK_HEXAREA][0] || ch[cn].skill[SK_DAMAREA][0]) || (ch[cn].flags & CF_AREA_OFF)))
	{ 
		do_char_log(cn, 0, "You stop yourself from poisoning your companion. That would be silly.\n");
		return;
	}
	else if (cn!=co && co!=ch[cn].data[CHD_SHADOWCOPY] && co!=ch[cn].data[CHD_COMPANION])
	{
		if (!do_char_can_see(cn, co))
		{
			do_char_log(cn, 0, "You cannot see your target.\n");
			return;
		}
		
		/* CS, 000209: Remember PvP attacks */
		remember_pvp(cn, co);
		
		if (is_exhausted(cn)) 					{ return; }
		if (spellcost(cn, cost, SK_POISON, 1)) 	{ return; }
		
		if (!may_attack_msg(cn, co, 1))
		{
			chlog(cn, "Prevented from attacking %s (%d)", ch[co].name, co);
			return;
		}
		
		damage_mshell(co);
		if (chance_base(cn, power, d20, get_target_resistance(co)))
		{
			if (cn!=co && get_skill_score(co, SK_SENSE)>power + 5)
			{
				if (!(ch[co].flags & CF_SENSE))
					do_char_log(co, 0, "%s tried to cast poison on you but failed.\n", ch[cn].reference);
				if (!IS_IGNORING_SPELLS(co))
				{
					do_notify_char(co, NT_GOTMISS, cn, 0, 0, 0);
				}
			}
			if (ch[cn].kindred&KIN_MONSTER) add_exhaust(cn,TICKS*2);
			return;
		}
		
		spell_poison(cn, co, power, 0);
		
		co_orig = co;
		count++;
		hit++;
		
		if (ch[co].flags & CF_IMMORTAL)
		{
			do_char_log(cn, 0, "You lost your focus.\n");
			return;
		}
	}

	// Feb 2020 - AoE Poisons
	if ((ch[cn].skill[SK_HEXAREA][0] || ch[cn].skill[SK_DAMAREA][0]) && !(ch[cn].flags & CF_AREA_OFF))
	{
		if (co) co_orig = co;
		
		spellaoe = 1;
		
		
		if (ch[cn].skill[SK_HEXAREA][0]) 
		{
			spellaoe = get_skill_score(cn, SK_HEXAREA)/PROXIMITY_CAP; //get_spell_aoe(get_skill_score(cn, SK_HEXAREA));
			dampower = power/2 + power/max(4, count/2-(get_skill_score(cn, SK_HEXAREA)/40));
		}
		else 
		{
			spellaoe = get_skill_score(cn, SK_DAMAREA)/PROXIMITY_CAP; //get_spell_aoe(get_skill_score(cn, SK_DAMAREA));
			dampower = power/2 + power/max(4, count/2-(get_skill_score(cn, SK_DAMAREA)/40));
		}
		
		xf = max(1, ch[cn].x - spellaoe);
		yf = max(1, ch[cn].y - spellaoe);
		xt = min(MAPX - 1, ch[cn].x + spellaoe+1);
		yt = min(MAPY - 1, ch[cn].y + spellaoe+1);
		
		// Loop through and count the number of targets first
		for (x = xf; x<xt; x++) for (y = yf; y<yt; y++) if ((co = map[x + y * MAPX].ch)) if (cn!=co && co_orig!=co) 
		{ 
			if (!do_surround_check(cn, co, 0)) continue;
			count++; 
		}
		if (!count)
		{ 
			if (co_orig==ch[cn].data[CHD_SHADOWCOPY] || co_orig==ch[cn].data[CHD_COMPANION])
			{ do_char_log(cn, 0, "You stop yourself from poisoning your companion. That would be silly.\n"); return; }
			else
			{ do_char_log(cn, 0, "You cannot poison yourself!\n"); return; }
		}
		if (!hit)
		{
			if (is_exhausted(cn)) 					return;
			if (spellcost(cn, cost, SK_POISON, 1)) 	return;
			if (chance(cn, FIVE_PERC_FAIL))			return;
		}
		// Then loop through and apply the effect based off the number of targets
		for (x = xf; x<xt; x++)	for (y = yf; y<yt; y++)	if ((co = map[x + y * MAPX].ch)) if (cn!=co && co_orig!=co)
		{
			remember_pvp(cn, co);
			if (!do_surround_check(cn, co, 1)) continue;
			damage_mshell(co);
			if (power+1+RANDOM(max(1,24-max(4,count/2))) > get_target_resistance(co)+RANDOM(max(1,12+max(4,count/2))))
			{
				spell_poison(cn, co, dampower, 1);
				hit++;
			}
			else
			{
				if (cn!=co && get_skill_score(co, SK_SENSE)>power + 5)
				{
					if (!(ch[co].flags & CF_SENSE))
						do_char_log(co, 0, "%s tried to include you in a mass-poison but failed.\n", ch[cn].reference);
					if (!IS_IGNORING_SPELLS(co))
						do_notify_char(co, NT_GOTMISS, cn, 0, 0, 0);
				}
				else
					do_char_log(co, 0, "You feel a toxic aura emanate from somewhere.\n");
			}
		}
		do_char_log(cn, 1, "You unleash a powerful mass-poison.\n");
		do_char_log(cn, 1, "You affected %d of %d creatures in range.\n", hit, count);
	}
	else
	{
		dampower = power/2 + power/4;
		m = ch[cn].x + ch[cn].y * MAPX;
		if ((co = map[m + 1].ch)!=0 && ch[co].attack_cn==cn && co_orig!=co)
		{
			damage_mshell(co);
			if (power+RANDOM(20) > get_target_resistance(co)+RANDOM(16)) spell_poison(cn, co, dampower, 0);
		}
		if ((co = map[m - 1].ch)!=0 && ch[co].attack_cn==cn && co_orig!=co)
		{
			damage_mshell(co);
			if (power+RANDOM(20) > get_target_resistance(co)+RANDOM(16)) spell_poison(cn, co, dampower, 0);
		}
		if ((co = map[m + MAPX].ch)!=0 && ch[co].attack_cn==cn && co_orig!=co)
		{
			damage_mshell(co);
			if (power+RANDOM(20) > get_target_resistance(co)+RANDOM(16)) spell_poison(cn, co, dampower, 0);
		}
		if ((co = map[m - MAPX].ch)!=0 && ch[co].attack_cn==cn && co_orig!=co)
		{
			damage_mshell(co);
			if (power+RANDOM(20) > get_target_resistance(co)+RANDOM(16)) spell_poison(cn, co, dampower, 0);
		}
	}

	fx_add_effect(7, 0, ch[cn].x, ch[cn].y, 0);

	add_exhaust(cn, SK_EXH_POISON);
}

int warcry(int cn, int co, int power)
{
	int n, in;

	// dont affect good NPCs unless they're a direct target
	if (ch[cn].attack_cn!=co && ch[co].alignment==10000) return 0;
	// dont attack those we may not attack anyway
	if (!may_attack_msg(cn, co, 0)) return 0;
	// ignore those whose resistance is greater than our skill
	damage_mshell(co);
	if (power<get_target_resistance(co)) return 0;
	// ignore those we grouped with
	for (n = 1; n<10; n++) if (ch[cn].data[n]==co) return 0;
	// ignore the immortals
	if (ch[co].flags & CF_IMMORTAL) { return 0; }

	if (!IS_IGNORING_SPELLS(co)) { do_notify_char(co, NT_GOTHIT, cn, 0, 0, 0); }

	power = spell_immunity(power, get_target_immunity(co));

	in = god_create_buff();
	if (!in)
	{
		xlog("god_create_buff failed in skill_warcry (1)");
		return( 0);
	}

	strcpy(bu[in].name, "Stun");
	bu[in].flags |= IF_SPELL;
	bu[in].sprite[1] = BUF_SPR_WARCRY2;
	bu[in].duration  = bu[in].active = SP_DUR_WARCRY2(power);
	bu[in].temp  = SK_WARCRY2;
	bu[in].power = power;

	add_spell(co, in);

	in = god_create_buff();
	if (!in)
	{
		xlog("god_create_buff failed in skill_warcry (2)");
		return( 0);
	}

	strcpy(bu[in].name, "Fear");
	bu[in].flags |= IF_SPELL;
	for (n = 0; n<5; n++) bu[in].attrib[n][1] = -(2+(power/(10/3)-n) / 5);
	bu[in].sprite[1] = BUF_SPR_WARCRY;
	bu[in].duration  = bu[in].active = SP_DUR_WARCRY;
	bu[in].temp  = SK_WARCRY;
	bu[in].power = power / 2;

	add_spell(co, in);

	chlog(cn, "Used Warcry on %s", ch[co].name);

	fx_add_effect(5, 0, ch[co].x, ch[co].y, 0);

	return(1);
}
void skill_warcry(cn)
{
	int power, n, xf, yf, xt, yt, x, y, co, hit = 0, count = 0, sk_aoe;
	
	if (is_exhausted(cn)) { return; }
	if (spellcost(cn, SP_COST_WARCRY, SK_WARCRY, 0)) { return; }
	
	power = get_skill_score(cn, SK_WARCRY);
	sk_aoe = 6 + get_skill_score(cn, SK_WARCRY)/40;

	xf = max(1, ch[cn].x - sk_aoe);
	yf = max(1, ch[cn].y - sk_aoe);
	xt = min(MAPX - 1, ch[cn].x + sk_aoe+1);
	yt = min(MAPY - 1, ch[cn].y + sk_aoe+1);

	for (x = xf; x<xt; x++) for (y = yf; y<yt; y++) if ((co = map[x + y * MAPX].ch)) if (cn!=co) 
	{ 
		if (!do_surround_check(cn, co, 0)) continue;
		count++; 
	}

	for (x = xf; x<xt; x++) for (y = yf; y<yt; y++) if ((co = map[x + y * MAPX].ch)) if (cn!=co) 
	{
		if (!do_surround_check(cn, co, 1)) continue;
		if (warcry(cn, co, power))
		{
			/* CS, 000209: Remember PvP attacks */
			remember_pvp(cn, co);
			do_char_log(co, 0, "You hear %s's warcry. You feel frightened and immobilized.\n", ch[cn].reference);
			hit++;
		}
		else
		{
			do_char_log(co, 0, "You hear %s's warcry.\n", ch[cn].reference);
		}
	}
	do_char_log(cn, 1, "You cry out loud and clear. You affected %d of %d creatures in range.\n", hit, count);
	
	add_exhaust(cn, SK_EXH_WARCRY + TICKS * power/80);
}

void item_info(int cn, int in, int look)
{
	int n;

	// if (!look) {
	do_char_log(cn, 1, "%s:\n", it[in].name);
	// }
	do_char_log(cn, 1, "Stat         Mod0 Mod1 Min\n");
	for (n = 0; n<5; n++)
	{
		if (!it[in].attrib[n][0] && !it[in].attrib[n][1] && !it[in].attrib[n][2])
		{
			continue;
		}
		do_char_log(cn, 1, "%-12.12s %+4d %+4d %3d\n",
		            at_name[n], it[in].attrib[n][0], it[in].attrib[n][1], it[in].attrib[n][2]);
	}

	if (it[in].hp[0] || it[in].hp[1] || it[in].hp[2])
	{
		do_char_log(cn, 1, "%-12.12s %+4d %+4d %3d\n",
		            "Hitpoints", it[in].hp[0], it[in].hp[1], it[in].hp[2]);
	}

	if (it[in].end[0] || it[in].end[1] || it[in].end[2])
	{
		do_char_log(cn, 1, "%-12.12s %+4d %+4d %3d\n",
		            "Endurance", it[in].end[0], it[in].end[1], it[in].end[2]);
	}

	if (it[in].mana[0] || it[in].mana[1] || it[in].mana[2])
	{
		do_char_log(cn, 1, "%-12.12s %+4d %+4d %3d\n",
		            "Mana", it[in].mana[0], it[in].mana[1], it[in].mana[2]);
	}

	for (n = 0; n<MAXSKILL; n++)
	{
		if (!it[in].skill[n][0] && !it[in].skill[n][1] && !it[in].skill[n][2])
		{
			continue;
		}
		do_char_log(cn, 1, "%-12.12s %+4d %+4d %3d\n",
		            skilltab[n].name, it[in].skill[n][0], it[in].skill[n][1], it[in].skill[n][2]);
	}

	if (it[in].weapon[0] || it[in].weapon[1])
	{
		do_char_log(cn, 1, "%-12.12s %+4d %+4d\n",
		            "Weapon", it[in].weapon[0], it[in].weapon[1]);
	}
	if (it[in].armor[0] || it[in].armor[1])
	{
		do_char_log(cn, 1, "%-12.12s %+4d %+4d\n",
		            "Armor", it[in].armor[0], it[in].armor[1]);
	}
	if (it[in].light[0] || it[in].light[1])
	{
		do_char_log(cn, 1, "%-12.12s %+4d %+4d\n",
		            "Light", it[in].light[0], it[in].light[1]);
	}

	if (it[in].power)
	{
		do_char_log(cn, 1, "%-12.12s %+4d",
		            "Power", it[in].power);
	}

	if (it[in].min_rank)
	{
		do_char_log(cn, 1, "%-12.12s %+4d",
		            "Min. Rank", it[in].min_rank);
	}
}

void char_info(int cn, int co)
{
	int n, in, flag = 0, n1 = -1, n2 = -1;

	do_char_log(cn, 1, "%s:\n", ch[co].name);
	do_char_log(cn, 1, " \n");
	for (n = 0; n<MAXBUFFS; n++)
	{
		if ((in = ch[co].spell[n])!=0)
		{
			if (bu[in].cost && (ch[cn].flags & CF_GOD))
				do_char_log(cn, 1, "%s for %dm %ds power of %d (%d)\n",
							bu[in].name, bu[in].active / (18 * 60), (bu[in].active / 18) % 60, bu[in].power, bu[in].cost);
			else
				do_char_log(cn, 1, "%s for %dm %ds power of %d\n",
							bu[in].name, bu[in].active / (18 * 60), (bu[in].active / 18) % 60, bu[in].power);
			flag = 1;
		}
	}
	if (!flag)
	{
		do_char_log(cn, 1, "No spells active.\n");
	}
	do_char_log(cn, 1, " \n");

	for (n = 0; n<MAXSKILL; n++)
	{
		if (ch[co].skill[n][0] && n1==-1)
		{
			n1 = n;
		}
		else if (ch[co].skill[n][0] && n2==-1)
		{
			n2 = n;
		}

		if (n1!=-1 && n2!=-1)
		{
			do_char_log(cn, 1, "%-12.12s %3d/%3d  !  %-12.12s %3d/%3d\n",
			            skilltab[n1].name, ch[co].skill[n1][0], get_skill_score(co, n1),
			            skilltab[n2].name, ch[co].skill[n2][0], get_skill_score(co, n2));
			n1 = -1;
			n2 = -1;
		}
	}

	if (n1!=-1)
	{
		do_char_log(cn, 1, "%-12.12s %3d/%3d\n",
		            skilltab[n1].name, ch[co].skill[n1][0], get_skill_score(co, n1));
	}

	do_char_log(cn, 1, "%-12.12s %3d/%3d  !  %-12.12s %3d/%3d\n",
	            at_name[AT_BRV], ch[co].attrib[AT_BRV][0], get_attrib_score(co, AT_BRV),
	            at_name[AT_WIL], ch[co].attrib[AT_WIL][0], get_attrib_score(co, AT_WIL));
	do_char_log(cn, 1, "%-12.12s %3d/%3d  !  %-12.12s %3d/%3d\n",
	            at_name[AT_INT], ch[co].attrib[AT_INT][0], get_attrib_score(co, AT_INT),
	            at_name[AT_AGL], ch[co].attrib[AT_AGL][0], get_attrib_score(co, AT_AGL));
	do_char_log(cn, 1, "%-12.12s %3d/%3d\n",
	            at_name[AT_STR], ch[co].attrib[AT_STR][0], get_attrib_score(co, AT_STR));

	do_char_log(cn, 1, " \n");
}

void skill_identify(int cn)
{
	int d20 = 18;
	int co = 0, in = 0, power;

	if (is_exhausted(cn))
	{
		return;
	}

	if (spellcost(cn, SP_COST_IDENT, SK_IDENT, 1))
	{
		return;
	}

	if ((in = ch[cn].citem)!=0 && IS_SANEITEM(in))
	{
		power = it[in].power;
	}
	else
	{
		if ((co = ch[cn].skill_target1)!=0)
		{
			power = get_target_resistance(co);
		}
		else
		{
			co = cn;
			power = 10;
		}
		in = 0;
	}

	if (chance_base(cn, get_skill_score(cn, SK_IDENT), d20, power))
	{
		return;
	}

	char_play_sound(cn, ch[cn].sound + 1, -150, 0);
	chlog(cn, "Cast Identify");

	if (in)
	{
		item_info(cn, in, 0);
		it[in].flags ^= IF_IDENTIFIED;
		if (!(it[in].flags & IF_IDENTIFIED))
		{
			do_char_log(cn, 1, "Identify data removed from item.\n");
		}
	}
	else
	{
		char_info(cn, co);
		fx_add_effect(6, 0, ch[co].x, ch[co].y, 0);
	}
	
	fx_add_effect(7, 0, ch[cn].x, ch[cn].y, 0);

	add_exhaust(cn, SK_EXH_IDENT);
}

int spell_scorch(int cn, int co, int power, int flag)
{
	int in, n, tmp;

	if (ch[cn].attack_cn!=co && ch[co].alignment==10000) { return 0; }
	if (ch[co].flags & CF_IMMORTAL) { return 0; }

	in = god_create_buff();
	if (!in)
	{
		xlog("god_create_buff failed in spell_scorch");
		return( 0);
	}

	power = spell_race_mod(spell_immunity(power, get_target_immunity(co)), cn);
	
	if (flag)	tmp = 10;
	else		tmp = 30;
	
	strcpy(bu[in].name, "Scorch");
	bu[in].flags |= IF_SPELL;
	bu[in].sprite[1] = BUF_SPR_SCORCH;
	bu[in].duration  = bu[in].active = SP_DUR_SCORCH(tmp);
	bu[in].temp  = SK_SCORCH;
	bu[in].power = power;

	add_spell(co, in);
	
	chlog(cn, "Inflicted Scorch on %s", ch[co].name);
	if (flag)	do_char_log(co, 0, "You have been scorched.\n");
	else		do_char_log(co, 0, "You were scorched by the blast!\n");
	
	if (flag)
	{
		if (!IS_IGNORING_SPELLS(co))
			do_notify_char(co, NT_GOTHIT, cn, 0, 0, 0);
		do_notify_char(cn, NT_DIDHIT, co, 0, 0, 0);
		char_play_sound(co, ch[cn].sound + 7, -150, 0);
		char_play_sound(cn, ch[cn].sound + 1, -150, 0);
		chlog(cn, "Cast Scorch on %s", ch[co].name);
		fx_add_effect(5, 0, ch[co].x, ch[co].y, 0);
	}

	return 1;
}
void skill_blast(int cn)
{
	int n, spellaoe, xf, yf, xt, yt, x, y, count = 0, hit = 0, avgdmg = 0, aoefocus = 0, aoeimm = 0; // Added for AoE Stuff
	int co, dam, m, co_orig = -1, tmp, base_power, power, cost;
	int scorch = 0;

	if ((co = ch[cn].skill_target1)) { ; }
	else if (ch[cn].attack_cn!=0) { co = ch[cn].attack_cn; }
	else { co = cn; }

	base_power = get_skill_score(cn, SK_BLAST);
	
	if (cn==co && (!ch[cn].skill[SK_DAMAREA][0] || (ch[cn].flags & CF_AREA_OFF)))
	{ 
		do_char_log(cn, 0, "You cannot blast yourself!\n"); 
		return; 
	}
	else if ((co==ch[cn].data[CHD_SHADOWCOPY] || co==ch[cn].data[CHD_COMPANION]) && (!ch[cn].skill[SK_DAMAREA][0] || (ch[cn].flags & CF_AREA_OFF)))
	{ 
		do_char_log(cn, 0, "You stop yourself from blasting your companion. That would be silly.\n");
		return;
	}
	else if (cn!=co && co!=ch[cn].data[CHD_SHADOWCOPY] && co!=ch[cn].data[CHD_COMPANION])
	{
		if (!do_char_can_see(cn, co))
			{ do_char_log(cn, 0, "You cannot see your target.\n"); return; }
		if (ch[co].flags & CF_STONED)
			{ do_char_log(cn, 0, "Your target is lagging. Try again later.\n"); return; }
		
		/* CS, 000209: Remember PvP attacks */
		remember_pvp(cn, co);
		
		power = spell_race_mod(spell_immunity(base_power, get_target_immunity(co)), cn);
		dam = power * 2;
		cost = dam / 8 + 5;
		if (ch[cn].skill[SK_DAMAREA][0] && !(ch[cn].flags & CF_AREA_OFF)) cost = cost * (PROXIMITY_MULTI+get_skill_score(cn, SK_DAMAREA)) / PROXIMITY_MULTI;
		
		// Tarot Card - Judgement :: Weaken Blast's damage & inflict Scorched
		if (get_tarot(cn, IT_CH_JUDGE))
		{
			dam = dam * 85/100;
			scorch = 1;
		}
		
		// Harakim costs less, Sorc costs slightly less, monster cost more mana
		if ((ch[cn].flags & CF_PLAYER) && (ch[cn].kindred & (KIN_HARAKIM | KIN_ARCHHARAKIM | KIN_SUMMONER)))
			cost = cost/3;
		if ((ch[cn].flags & CF_PLAYER) && (ch[cn].kindred & KIN_SORCERER))
			cost = (cost/3)*2;
		if (!(ch[cn].flags & CF_PLAYER)) 
			cost = cost*2;
		//
		
		if (is_exhausted(cn)) 					{ return; }
		if (spellcost(cn, cost, SK_BLAST, 1)) 	{ return; }

		if (!may_attack_msg(cn, co, 1))
			{ chlog(cn, "Prevented from attacking %s (%d)", ch[co].name, co); return; }
		
		damage_mshell(co);
		
		if (chance(cn, FIVE_PERC_FAIL))
		{
			if (cn!=co && get_skill_score(co, SK_SENSE)>get_skill_score(cn, SK_BLAST) + 5)
			{
				if (!(ch[co].flags & CF_SENSE))
					do_char_log(co, 0, "%s tried to cast blast on you but failed.\n", ch[cn].reference);
				if (!IS_IGNORING_SPELLS(co))
				{
					do_notify_char(co, NT_GOTMISS, cn, 0, 0, 0);
				}
			}
			if (ch[cn].kindred&KIN_MONSTER) add_exhaust(cn,TICKS*3);
			return;
		}
		//
		do_area_sound(co, 0, ch[co].x, ch[co].y, ch[cn].sound + 6);
		char_play_sound(co, ch[cn].sound + 6, -150, 0);
		chlog(cn, "Cast Blast on %s", ch[co].name);
		//
		tmp = do_hurt(cn, co, dam, 1);
		if (tmp<1)	do_char_log(cn, 0, "You cannot penetrate %s's armor.\n", ch[co].reference);
		else
		{
			do_char_log(cn, 1, "You blast %s for %d HP.\n", ch[co].reference, tmp);
			do_char_log(co, 1, "%s blasted you for %d HP.\n", ch[cn].name, tmp);
		}
		//
		fx_add_effect(5, 0, ch[co].x, ch[co].y, 0);

		if (scorch) spell_scorch(cn, co, power, 0);

		co_orig = co;
		avgdmg+=tmp;
		count++;
		hit++;
	}

	if (ch[cn].skill[SK_DAMAREA][0] && !(ch[cn].flags & CF_AREA_OFF))
	{
		if (co) co_orig = co;
		
		spellaoe = get_skill_score(cn, SK_DAMAREA)/PROXIMITY_CAP; //get_spell_aoe(get_skill_score(cn, SK_DAMAREA));
		
		xf = max(1, ch[cn].x - spellaoe);
		yf = max(1, ch[cn].y - spellaoe);
		xt = min(MAPX - 1, ch[cn].x + spellaoe+1);
		yt = min(MAPY - 1, ch[cn].y + spellaoe+1);
		
		// Loop through and count the number of targets first
		for (x = xf; x<xt; x++) for (y = yf; y<yt; y++) if ((co = map[x + y * MAPX].ch)) if (cn!=co && co_orig!=co) 
		{ 
			if (!do_surround_check(cn, co, 0)) continue;
			damage_mshell(co);
			count++; aoeimm += get_target_immunity(co); 
		}
		if (!count)
		{ 
			if (co_orig==ch[cn].data[CHD_SHADOWCOPY] || co_orig==ch[cn].data[CHD_COMPANION])
			{ do_char_log(cn, 0, "You stop yourself from blasting your companion. That would be silly.\n"); return; }
			else
			{ do_char_log(cn, 0, "You cannot blast yourself!\n"); return; }
		}
		
		aoeimm /= count;
		power = spell_race_mod(spell_immunity(get_skill_score(cn, SK_BLAST), aoeimm), cn);
		dam = power * 2;
		cost = (dam / 8 + 5) * (PROXIMITY_MULTI+get_skill_score(cn, SK_DAMAREA)) / PROXIMITY_MULTI;
		//dam = dam/(min(1,count)+1)+dam/(min(1,count)/(min(1,count)/2));
		dam = dam/2 + dam/max(4, count/2-(get_skill_score(cn, SK_DAMAREA)/40));
		
		// Tarot Card - Judgement :: Weaken Blast's damage & inflict Scorched
		if (get_tarot(cn, IT_CH_JUDGE))
		{
			dam = dam * 85/100;
			scorch = 1;
		}
		
		// Harakim costs less mana, monster cost more mana
		if ((ch[cn].flags & CF_PLAYER) && (ch[cn].kindred & (KIN_HARAKIM | KIN_ARCHHARAKIM)))
			cost /= 3;
		if (!(ch[cn].flags & CF_PLAYER)) 
			cost *= 2;
		//
		if (!hit)
		{
			if (is_exhausted(cn)) 					{ return; }
			if (spellcost(cn, cost, SK_BLAST, 1)) 	{ return; }
			aoefocus = chance(cn, FIVE_PERC_FAIL);
		}
		
		// Loop through and apply the effect based off the number of targets
		for (x = xf; x<xt; x++)	for (y = yf; y<yt; y++)	if ((co = map[x + y * MAPX].ch)) if (cn!=co && co_orig!=co)
		{
			remember_pvp(cn, co);
			if (!do_surround_check(cn, co, 1)) continue;
			if (aoefocus)
			{
				if (cn!=co && get_skill_score(co, SK_SENSE)>power + 5)
				{
					if (!(ch[co].flags & CF_SENSE))
						do_char_log(co, 0, "%s tried to include you in a mass-blast but failed.\n", ch[cn].reference);
					if (!IS_IGNORING_SPELLS(co))
						do_notify_char(co, NT_GOTMISS, cn, 0, 0, 0);
				}
				else
					do_char_log(co, 0, "You feel a tingling shockwave from somewhere.\n");
			}
			else
			{
				do_area_sound(co, 0, ch[co].x, ch[co].y, ch[cn].sound + 6);
				char_play_sound(co, ch[cn].sound + 6, -150, 0);
				chlog(cn, "Cast Blast on %s", ch[co].name);
				fx_add_effect(5, 0, ch[co].x, ch[co].y, 0);
				tmp = do_hurt(cn, co, dam, 1);
				if (tmp>0) do_char_log(co, 1, "%s blasted you for %d HP.\n", ch[cn].name, tmp);
				avgdmg+=tmp;
				hit++;
				if (scorch) spell_scorch(cn, co, power, 0);
			}
		}
		if (aoefocus) return;
		do_char_log(cn, 1, "You unleash a destructive shockwave.\n");
		do_char_log(cn, 1, "You hit %d of %d creatures in range.\n", hit, count);
		do_char_log(cn, 1, "You delt an average of %d damage.\n", avgdmg/max(1,hit));
	}
	else
	{
		dam = dam/2 + dam/4;
		m = ch[cn].x + ch[cn].y * MAPX;
		if ((co = map[m + 1].ch)!=0 && ch[co].attack_cn==cn && co_orig!=co)
		{
			power = spell_race_mod(spell_immunity(base_power, get_target_immunity(co)), cn);
			dam = (power * 2)*3/4; if (scorch) dam = dam * 85/100;
			
			damage_mshell(co);
			chlog(cn, "Cast Blast on %s", ch[co].name);
			tmp = do_hurt(cn, co, dam, 1);
			if (tmp<1)	do_char_log(cn, 0, "You cannot penetrate %s's armor.\n", ch[co].reference);
			else
			{
				do_char_log(cn, 1, "You blast %s for %d HP.\n", ch[co].reference, tmp);
				do_char_log(co, 1, "%s blasted you for %d HP.\n", ch[cn].name, tmp);
			}
			fx_add_effect(5, 0, ch[co].x, ch[co].y, 0);
			if (scorch) spell_scorch(cn, co, power, 0);
		}
		if ((co = map[m - 1].ch)!=0 && ch[co].attack_cn==cn && co_orig!=co)
		{
			power = spell_race_mod(spell_immunity(base_power, get_target_immunity(co)), cn);
			dam = (power * 2)*3/4; if (scorch) dam = dam * 85/100;
			
			damage_mshell(co);
			chlog(cn, "Cast Blast on %s", ch[co].name);
			tmp = do_hurt(cn, co, dam, 1);
			if (tmp<1)	do_char_log(cn, 0, "You cannot penetrate %s's armor.\n", ch[co].reference);
			else
			{
				do_char_log(cn, 1, "You blast %s for %d HP.\n", ch[co].reference, tmp);
				do_char_log(co, 1, "%s blasted you for %d HP.\n", ch[cn].name, tmp);
			}
			fx_add_effect(5, 0, ch[co].x, ch[co].y, 0);
			if (scorch) spell_scorch(cn, co, power, 0);
		}
		if ((co = map[m + MAPX].ch)!=0 && ch[co].attack_cn==cn && co_orig!=co)
		{
			power = spell_race_mod(spell_immunity(base_power, get_target_immunity(co)), cn);
			dam = (power * 2)*3/4; if (scorch) dam = dam * 85/100;
			
			damage_mshell(co);
			chlog(cn, "Cast Blast on %s", ch[co].name);
			tmp = do_hurt(cn, co, dam, 1);
			if (tmp<1)	do_char_log(cn, 0, "You cannot penetrate %s's armor.\n", ch[co].reference);
			else
			{
				do_char_log(cn, 1, "You blast %s for %d HP.\n", ch[co].reference, tmp);
				do_char_log(co, 1, "%s blasted you for %d HP.\n", ch[cn].name, tmp);
			}
			fx_add_effect(5, 0, ch[co].x, ch[co].y, 0);
			if (scorch) spell_scorch(cn, co, power, 0);
		}
		if ((co = map[m - MAPX].ch)!=0 && ch[co].attack_cn==cn && co_orig!=co)
		{
			power = spell_race_mod(spell_immunity(base_power, get_target_immunity(co)), cn);
			dam = (power * 2)*3/4; if (scorch) dam = dam * 85/100;
			
			damage_mshell(co);
			chlog(cn, "Cast Blast on %s", ch[co].name);
			tmp = do_hurt(cn, co, dam, 1);
			if (tmp<1)	do_char_log(cn, 0, "You cannot penetrate %s's armor.\n", ch[co].reference);
			else
			{
				do_char_log(cn, 1, "You blast %s for %d HP.\n", ch[co].reference, tmp);
				do_char_log(co, 1, "%s blasted you for %d HP.\n", ch[cn].name, tmp);
			}
			fx_add_effect(5, 0, ch[co].x, ch[co].y, 0);
			if (scorch) spell_scorch(cn, co, power, 0);
		}
	}
	if (scorch) 
	{
		do_char_log(cn, 1, "Your foes were scorched by your blast!\n");
	}
	
	fx_add_effect(7, 0, ch[cn].x, ch[cn].y, 0);
	
	add_exhaust(cn, SK_EXH_BLAST);
}

void skill_repair(int cn)
{
	int in, chan, die, in2, power;

	if ((in = ch[cn].citem)==0)
	{
		do_char_log(cn, 0, "Repair. Repair what?\n");
		return;
	}
	if (it[in].damage_state==0)
	{
		do_char_log(cn, 0, "That isn't damaged.\n");
		return;
	}
	if (it[in].flags & IF_NOREPAIR)
	{
		do_char_log(cn, 0, "This item cannot be repaired.\n");
		return;
	}
	if (it[in].flags & IF_SOULSTONE)
	{
		power = it[in].power*3;
	}
	else
	{
		power = it[in].power;
	}
	if (power>get_skill_score(cn, SK_REPAIR))
	{
		do_char_log(cn, 0, "That's too difficult for you (requires %d Repair).\n", power);
		return;
	}
	if (ch[cn].a_end<it[in].power * 500)
	{
		do_char_log(cn, 0, "You're too exhausted to repair that.\n");
		return;
	}
	ch[cn].a_end -= it[in].power * 500;

	if (it[in].power)	chan = get_skill_score(cn, SK_REPAIR) * 15 / it[in].power;
	else				chan = 18;
	if (chan<0)			chan = 0;
	if (chan>18)		chan = 18;

	die = RANDOM(20);

	if (die<=chan)
	{
		in2 = god_create_item(it[in].temp, 0); // Repair by making new item
		if (!in2)
		{
			do_char_log(cn, 0, "You failed.\n");
			return;
		}
		it[in].used  = USE_EMPTY;
		ch[cn].citem = in2;
		it[in2].carried = cn;
		do_char_log(cn, 1, "Success!\n");
	}
	else
	{
		do_char_log(cn, 0, "You failed.\n");
		item_damage_citem(cn, 1000000);  // make sure it reaches the next state
		if (die - chan>3)
		{
			item_damage_citem(cn, 1000000);
		}
		if (die - chan>6)
		{
			item_damage_citem(cn, 1000000);
		}
	}
	chlog(cn, "Cast Repair");

}

void skill_recall(int cn)
{
	int in;

	if (is_exhausted(cn))
	{
		return;
	}

	if (spellcost(cn, SP_COST_RECALL, SK_RECALL, 1))
	{
		return;
	}
	if (chance(cn, 18))
	{
		return;
	}

	in = god_create_buff();
	if (!in)
	{
		xlog("god_create_buff failed in skill_recall");
		return;
	}

	strcpy(bu[in].name, "Recall");
	bu[in].flags |= IF_SPELL;
	bu[in].sprite[1] = BUF_SPR_RECALL;
	bu[in].duration  = bu[in].active = SP_DUR_RECALL(get_skill_score(cn, SK_RECALL));
	bu[in].temp  = SK_RECALL;
	bu[in].power = get_skill_score(cn, SK_RECALL);
	bu[in].data[0] = ch[cn].temple_x;
	bu[in].data[1] = ch[cn].temple_y;

	if (!add_spell(cn, in))
	{
		do_char_log(cn, 1, "Magical interference neutralized the %s's effect.\n", bu[in].name);
		return;
	}
	chlog(cn, "Cast Recall");

	fx_add_effect(7, 0, ch[cn].x, ch[cn].y, 0);

	add_exhaust(cn, SK_EXH_RECALL);
}

void remove_spells(int cn) // Handles No-Magic-Zones, not Dispel
{
	int in, n;

	for (n = 0; n<MAXBUFFS; n++)
	{
		if ((in = ch[cn].spell[n])==0)	continue;
		if (bu[in].temp == SK_BLAST) 	continue;
		if (bu[in].temp == SK_HEAL) 	continue;
		if (bu[in].temp == SK_SHADOW) 	continue;
		if (bu[in].temp == SK_BLEED) 	continue;
		if (bu[in].temp == SK_WEAKEN) 	continue;
		if (bu[in].temp == SK_WEAKEN2) 	continue;
		if (bu[in].temp == SK_WARCRY) 	continue;
		if (bu[in].temp == SK_WARCRY2) 	continue;
		bu[in].used = USE_EMPTY;
		ch[cn].spell[n] = 0;
	}
	do_update_char(cn);
}

void skill_dispel(int cn)
{
	int in, co, n, m, power, ail_pow = 0, success = 0, chanc;
	int ail[12] = { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 };
	int multi = 12;

	if ((co = ch[cn].skill_target1)) { ; }
	else { co = cn; }
	
	if (!do_char_can_see(cn, co))
	{
		do_char_log(cn, 0, "You cannot see your target.\n");
		return;
	}

	if (is_exhausted(cn)) { return; }
	if (spellcost(cn, SP_COST_DISPEL, SK_DISPEL, 1)) { return; }
	
	if (!player_or_ghost(cn, co) && get_tarot(cn, IT_CH_HEIROPH))
	{
		do_char_log(cn, 0, "Changed target of spell from %s to %s.\n", ch[co].name, ch[cn].name);
		co = cn;
	}

	power = spell_race_mod(get_skill_score(cn, SK_DISPEL), cn);

	// Tarot Card - Hierophant :: Dispel removes positive spells instead of negative spells
	if (get_tarot(cn, IT_CH_HEIROPH))
	{
		multi = 9;
		// Remove each positive spell in sequence, from most to least expensive
		for (n = 0; n<MAXBUFFS; n++)
		{
			if ((in = ch[co].spell[n])==0) { continue; }
			if (bu[in].temp==SK_HASTE)		ail[0] = n;
			if (bu[in].temp==SK_BLESS) 		ail[1] = n;
			if (bu[in].temp==SK_MSHIELD) 	ail[2] = n;
			if (bu[in].temp==SK_MSHELL) 	ail[3] = n;
			if (bu[in].temp==SK_REGEN) 		ail[4] = n;
			if (bu[in].temp==SK_PROTECT) 	ail[5] = n;
			if (bu[in].temp==SK_ENHANCE) 	ail[6] = n;
			if (bu[in].temp==SK_LIGHT) 		ail[7] = n;
		}
	}
	else
	{
		// Remove each negative spell in sequence, from most to least imperative
		for (n = 0; n<MAXBUFFS; n++)
		{
			if ((in = ch[co].spell[n])==0) { continue; }
			if (bu[in].temp==SK_POISON)		ail[0] = n;
			if (bu[in].temp==SK_BLEED) 		ail[1] = n;
			if (bu[in].temp==SK_WARCRY2) 	ail[2] = n;
			if (bu[in].temp==SK_CURSE2) 	ail[3] = n;
			if (bu[in].temp==SK_CURSE) 		ail[4] = n;
			if (bu[in].temp==SK_WARCRY) 	ail[5] = n;
			if (bu[in].temp==SK_WEAKEN2) 	ail[6] = n;
			if (bu[in].temp==SK_WEAKEN) 	ail[7] = n;
			if (bu[in].temp==SK_SLOW2) 		ail[8] = n;
			if (bu[in].temp==SK_SLOW) 		ail[9] = n;
			if (bu[in].temp==SK_SCORCH) 	ail[10] = n;
			if (bu[in].temp==SK_HEAL) 		ail[11] = n;
		}
	}
	for (m = 0; m<12; m++) if (ail[m]>-1)
	{
		in = ch[co].spell[ail[m]];
		ail_pow = bu[in].power;
		chanc = multi * power / max(1, ail_pow); if (chanc > 18) chanc = 18;
		if (chanc<RANDOM(20))
		{
			if (!success)
			{
				do_char_log(cn, 0, "Your spell fizzled.\n");
				if (cn!=co && get_skill_score(co, SK_SENSE)>get_skill_score(cn, SK_DISPEL) + 5)
				{
					if (!(ch[co].flags & CF_SENSE))
						do_char_log(co, 0, "%s tried to cast dispel magic on you but failed.\n", ch[cn].reference);
				}
				return;
			}
			else break;
		}
		if (co!=cn)
		{
			bu[in].used = USE_EMPTY; 
			ch[co].spell[ail[m]] = 0;
			do_update_char(co);
			if (get_tarot(cn, IT_CH_HEIROPH)) remember_pvp(cn,co);
			if (get_skill_score(co, SK_SENSE) + 10>get_skill_score(cn, SK_DISPEL))
				do_char_log(co, 0, "%s has been dispelled by %s.\n", bu[in].name, ch[cn].reference);
			else
				do_char_log(co, 0, "%s has been dispelled.\n", bu[in].name);
			do_char_log(cn, 1, "Dispelled %s from %s.\n", bu[in].name, ch[co].name);
			if (get_tarot(cn, IT_CH_HEIROPH) && !(ch[co].flags&(CF_PLAYER))) 
			{
				if (!IS_IGNORING_SPELLS(co)) 
					do_notify_char(co,NT_GOTHIT,cn,0,0,0);
				do_notify_char(cn,NT_DIDHIT,co,0,0,0);
            }
		}
		else
		{
			bu[in].used = USE_EMPTY; 
			ch[cn].spell[ail[m]] = 0;
			do_update_char(co);
			do_char_log(cn, 1, "%s has been removed.\n", bu[in].name);
		}
		
		success = 1;
		power -= ail_pow;
		if (power < 1) break;
	}
	if (!success && n==20)
	{
		if (get_tarot(cn, IT_CH_HEIROPH))
		{
			if (co==cn)	{ do_char_log(cn, 0, "But you aren't positively spelled!\n"); return; }
			else 		{ do_char_log(cn, 0, "%s isn't positively spelled!\n", ch[co].name); return; }
		}
		else
		{
			if (co==cn)	{ do_char_log(cn, 0, "But you aren't negatively spelled!\n"); return; }
			else 		{ do_char_log(cn, 0, "%s isn't negatively spelled!\n", ch[co].name); return; }
		}
	}
	
	do_update_char(co);
	
	if (co!=cn)
	{
		chlog(cn, "Cast Dispel on %s", ch[co].name);
		char_play_sound(co, ch[cn].sound + 1, -150, 0);
		char_play_sound(cn, ch[cn].sound + 1, -150, 0);
		fx_add_effect(6, 0, ch[co].x, ch[co].y, 0);
	}
	else
	{
		chlog(cn, "Cast Dispel");
		char_play_sound(cn, ch[cn].sound + 1, -150, 0);
		fx_add_effect(6, 0, ch[cn].x, ch[cn].y, 0);
	}
	
	fx_add_effect(7, 0, ch[cn].x, ch[cn].y, 0);
	
	add_exhaust(cn, SK_EXH_DISPEL);
}

void skill_ghost(int cn)
{
	int co, cc, cz, n, base = 0, tmp, pts = 0, z, m, idx;
	int archgc = 0, archbonus = 0, dreadplate = 0, necronomicon = 0;

	if (IS_BUILDING(cn))
	{
		do_char_log(cn, 0, "Not in build mode.\n");
		return;
	}

	if ((ch[cn].flags & CF_PLAYER) && (co = ch[cn].data[CHD_COMPANION]))
	{
		if (!IS_SANECHAR(co) || ch[co].data[63]!=cn || (ch[co].flags & CF_BODY) || ch[co].used==USE_EMPTY)
		{
			co = 0;
		}
		if (co && !(ch[co].flags & CF_SHADOWCOPY))
		{
			do_char_log(cn, 0, "You may not have more than one Ghost Companion (%d).\n", co);
			return;
		}
	}

	if ((co = ch[cn].skill_target1)) { ; }
	else { co = 0; }
	if (cn==co) { co = 0; }
	if (co==ch[cn].data[CHD_SHADOWCOPY] || co==ch[cn].data[CHD_COMPANION]) { co = 0; }

	if (co && !do_char_can_see(cn, co))
	{
		do_char_log(cn, 0, "You cannot see your target.\n");
		return;
	}

	if (is_exhausted(cn)) { return; }

	if (co && !may_attack_msg(cn, co, 1))
	{
		chlog(cn, "Prevented from attacking %s (%d)", ch[co].name, co);
		return;
	}

	if (spellcost(cn, SP_COST_GHOST, SK_GHOST, 1)) { return; }

	/* CS, 000109: No GC in Gatekeeper's room */
	if (ch[cn].x >= 137 && ch[cn].x <= 147 && ch[cn].y >= 182 && ch[cn].y <= 192)
	{
		do_char_log(cn, 0, "You must fight this battle alone.\n");
		return;
	}

	if (chance(cn, TEN_PERC_FAIL))
	{
		if (co && cn!=co && get_skill_score(co, SK_SENSE)>get_skill_score(cn, SK_GHOST) + 5)
		{
			if (!(ch[co].flags & CF_SENSE))
				do_char_log(co, 0, "%s tried to cast ghost companion on you but failed.\n", ch[cn].reference);
			if (!IS_IGNORING_SPELLS(co))
			{
				do_notify_char(co, NT_GOTMISS, cn, 0, 0, 0);
			}
		}
		return;
	}
	
	if (it[ch[cn].worn[WN_BODY]].temp==IT_TW_DREAD) dreadplate = 1;
	if (it[ch[cn].worn[WN_LHAND]].temp==IT_BOOK_NECR) necronomicon = 1;
	
	archgc 		= ch[cn].skill[SK_GCMASTERY][0];
	if (archgc) 
		archbonus 	= get_skill_score(cn, SK_GCMASTERY)/6;
	
	if (dreadplate)
	{
		// Special GC for Tower item
		if (archgc)	cc = god_create_char(CT_ARCHCASTER, 1); 
		else 		cc = god_create_char(CT_CASTERCOMP, 1); 
	}
	else
	{
		// Better GC for Summoner
		if (archgc)	cc = god_create_char(CT_ARCHCOMP, 1); 
		// Create the normal GC
		else 		cc = god_create_char(CT_COMPANION, 1); 
	}
	
	if (!god_drop_char_fuzzy(cc, ch[cn].x, ch[cn].y))
	{
		ch[cc].used = USE_EMPTY;
		do_char_log(cn, 0, "The ghost companion could not materialize.\n");
		return;
	}

	if (co)
	{
		if (!IS_IGNORING_SPELLS(co)) { do_notify_char(co, NT_GOTHIT, cn, 0, 0, 0); }
		do_notify_char(cn, NT_DIDHIT, co, 0, 0, 0);
	}

	if (ch[cn].flags & CF_PLAYER) { ch[cn].data[CHD_COMPANION] = cc; }
	
	// Feb 2020 - better base values  ( 4 -> 5 ; 11 -> 11 )
	base = (get_skill_score(cn, SK_GHOST) * 5) / 11;
	base = spell_race_mod(base, cn);

	ch[cc].data[29] = 0;				// reset experience earned
	ch[cc].data[42] = 65536 + cn;		// set group
	ch[cc].kindred &= ~(KIN_MONSTER);	// Add 'monster' flag
	
	if (co)
	{
		ch[cc].attack_cn = co;
		idx = co | (char_id(co) << 16);
		ch[cc].data[80] = idx;            // add enemy to kill list
	}
	ch[cc].data[63] = cn;
	ch[cc].data[69] = cn;
	
	// Set duration of GC -- Forever for players, 5 minutes for NPCs
	if (ch[cn].flags & CF_PLAYER) { ch[cc].data[CHD_COMPANION] = 0; }
	else 
	{ 
		ch[cc].data[CHD_COMPANION] = globs->ticker + TICKS * 60 * 5;
		ch[cc].data[42] = ch[cn].data[42];
		ch[cc].data[59] = ch[cn].data[42];
	}
	
	ch[cc].data[98] = globs->ticker + COMPANION_TIMEOUT;

	strcpy(ch[cc].text[0], "#14#Yes! %s buys the farm!");
	strcpy(ch[cc].text[1], "#13#Yahoo! An enemy! Prepare to die, %s!");
	strcpy(ch[cc].text[3], "My successor will avenge me, %s!");
	if (dreadplate)
	{
		if (archgc)
			ch[cc].data[CHD_TALKATIVE] = ch_temp[CT_ARCHCASTER].data[CHD_TALKATIVE];
		else 		
			ch[cc].data[CHD_TALKATIVE] = ch_temp[CT_CASTERCOMP].data[CHD_TALKATIVE];
	}
	else
	{
		if (archgc)
			ch[cc].data[CHD_TALKATIVE] = ch_temp[CT_ARCHCOMP].data[CHD_TALKATIVE];
		else 		
			ch[cc].data[CHD_TALKATIVE] = ch_temp[CT_COMPANION].data[CHD_TALKATIVE];
	}
	
	if (ch[cn].flags & CF_SILENCE) 
	{
		ch[cc].data[CHD_TALKATIVE] = 0;
		ch[cc].flags |= CF_SILENCE;
	}

	ch[cc].data[48] = 33;
	
	if (ch[cn].flags & CF_PLAYER) 
	{
		ch[cc].flags |= CF_CANCRIT;
		ch[cc].data[2] = 1; // Base crit chance
	}

	for (n = 0; n<5; n++)
	{
		tmp = base;
		tmp = tmp * 5 / max(1, ch[cc].attrib[n][3]);
		ch[cc].attrib[n][0] = max(10, min(ch[cc].attrib[n][2]+archbonus, tmp));
	}

	for (n = 0; n<MAXSKILL; n++)
	{
		tmp = base;
		tmp = tmp * 5 / max(1, ch[cc].skill[n][3]);
		if (ch[cc].skill[n][2])
		{
			ch[cc].skill[n][0] = min(ch[cc].skill[n][2]+archbonus, tmp);
		}
	}
	
	if (necronomicon)
	{
		if (dreadplate) // caster ~ becomes ghost
		{
			ch[cc].skill[SK_CONCEN][0] = ch[cc].skill[SK_RESIST][0];
			ch[cc].skill[SK_MEDIT][0] += 60;
			ch[cc].sprite = 9168;
		}
		else			// fighter ~ becomes skeleton
		{
			ch[cc].skill[SK_CLEAVE][0] = ch[cc].skill[SK_RESIST][0];
			ch[cc].skill[SK_REST][0] += 60;
			ch[cc].sprite = 10192;
		}
		ch[cc].data[1] = 3; // BERSERK!!
	}

	ch[cc].hp[0]   		= max(50, min(ch[cc].hp[2],   base * 5));
	ch[cc].end[0]  		= max(100,min(ch[cc].end[2],  base * 3));
	
	// Feb 2020 -- Gonna need mana to cast heal!
	if (archgc||dreadplate)
		ch[cc].mana[0] 	= max(50, min(ch[cc].mana[2], base * 4));
	else
		ch[cc].mana[0] 	= 0;

	// calculate experience
	for (z = 0; z<5; z++)
	{
		for (m = 10; m<ch[cc].attrib[z][0]; m++)
		{
			pts += attrib_needed(m, ch[cc].attrib[z][3]-2);
		}
	}

	for (m = 50; m<ch[cc].hp[0]; m++)
	{
		pts += hp_needed(m, ch[cc].hp[3]-1);
	}

	//for (m = 50; m<ch[cc].end[0]; m++)
	//{
	//	pts += end_needed(m, ch[cc].end[3]-1);
	//}

	for (m = 50; m<ch[cc].mana[0]; m++)
	{
		pts += mana_needed(m, ch[cc].mana[3]-1);
	}

	for (z = 0; z<MAXSKILL; z++)
	{
		for (m = 1; m<ch[cc].skill[z][0]; m++)
		{
			pts += skill_needed(m, ch[cc].skill[z][3]-1);
		}
	}

	ch[cc].points_tot = pts;
	ch[cc].gold   = 0;
	ch[cc].a_hp   = 999999;
	ch[cc].a_end  = 999999;
	ch[cc].a_mana = 999999;

	ch[cc].alignment = ch[cn].alignment / 2;
	
	// Feb 2020 -- better WV/AV values; no more tiers.
	if (dreadplate)
	{
		ch[cc].weapon_bonus = max(8, min(48+archbonus/3, base*5/9+8));
		ch[cc].armor_bonus  = max(8, min(48+archbonus/3, base*5/9+8));
	}
	else
	{
		ch[cc].weapon_bonus = max(12, min(72+archbonus/3, base*5/7+12));
		ch[cc].armor_bonus  = max(10, min(60+archbonus/3, base*5/6+10));
	}
	
	xlog("Created %s (%d) with base %d as Ghost Companion for %s (group %d)", ch[cc].name, cc, base, ch[cn].reference, ch[cc].data[42]);

	/* CS, 000109: Less chatty GC */
	if (co)
	{
		do_sayx(cc, ch[cc].text[1], ch[co].name);
	}
	else if (points2rank(ch[cc].points_tot) < 6)     // GC not yet Master Sergeant
	{
		do_sayx(cc, "I shall defend you and obey your COMMAND, %s. "
		        "I will WAIT, FOLLOW, or ATTACK for you and can tell you the TIME. "
				"You can COMMAND me in other ways too, if you ask. "
		        "I can also TRANSFER my experience to you, though it would cost my life.",
		        ch[cn].name);
	}
	else
	{
		tmp = 100;
		
		if (necronomicon) tmp = RANDOM(40);
		switch (tmp)
		{
			case 0:		do_sayx(cc, "I shall rend their flesh!"); break;
			case 1:		do_sayx(cc, "Hahahahah..."); break;
			case 2:		do_sayx(cc, "Who... what am I...?"); break;
			case 3:		do_sayx(cc, "They watch you with great interest, %s.", ch[cn].name); break;
			case 4:		do_sayx(cc, "#9#Thank you for creating me, %s!", ch[cn].name); break;
			default:	do_sayx(cc, "Thank you for creating me, %s!", ch[cn].name); break;
		}
	}
	
	// GC Management notes:
	// Now you can set the aggro level of your GC using talk commands. This uses data[1].
	// 0 -- Defense - Normal behavior
	// 1 -- Passive - Don't attack or fight back unless dying
	// 2 -- Offense - Add enemies to kill list if they share a group with an enemy on your kill list
	// 3 -- Berserk - Attack all enemies under 0 alignment no matter what

	do_update_char(cc);

	fx_add_effect(6, 0, ch[cc].x, ch[cc].y, 0);
	fx_add_effect(7, 0, ch[cn].x, ch[cn].y, 0);

	add_exhaust(cn, SK_EXH_GHOST);
}

void skill_shadow(int cn)
{
	int co, cc, cz, n, base = 0, power = 0, m, idx, in, cost, tmp;
	int wpnskill = 0, powlimit = 250, necronomicon = 0;

	if (IS_BUILDING(cn))
	{
		do_char_log(cn, 0, "Not in build mode.\n");
		return;
	}
	
	if ((ch[cn].flags & CF_PLAYER) && (co = ch[cn].data[CHD_SHADOWCOPY]))
	{
		if (!IS_SANECHAR(co) || ch[co].data[63]!=cn || (ch[co].flags & CF_BODY) || ch[co].used==USE_EMPTY)
		{
			co = 0;
		}
		if (co && (ch[co].flags & CF_SHADOWCOPY))
		{
			do_char_log(cn, 0, "You may not have more than one Shadow Copy (%d).\n", co);
			return;
		}
	}

	if ((co = ch[cn].skill_target1)) { ; }
	else { co = 0; }
	if (cn==co) { co = 0; }
	if (co==ch[cn].data[CHD_SHADOWCOPY] || co==ch[cn].data[CHD_COMPANION]) { co = 0; }

	if (co && !do_char_can_see(cn, co))
	{
		do_char_log(cn, 0, "You cannot see your target.\n");
		return;
	}

	if (is_exhausted(cn)) { return; }

	if (co && !may_attack_msg(cn, co, 1))
	{
		chlog(cn, "Prevented from attacking %s (%d)", ch[co].name, co);
		return;
	}
	
	cost = SP_COST_SHADOW;
	
	if (ch[cn].kindred & KIN_PUGILIST) cost /= 3;
	
	if (spellcost(cn, cost, SK_SHADOW, 1)) { return; }
	
	if (chance(cn, FIVE_PERC_FAIL))
	{
		if (co && cn!=co && get_skill_score(co, SK_SENSE)>get_skill_score(cn, SK_GHOST) + 5)
		{
			if (!(ch[co].flags & CF_SENSE))
				do_char_log(co, 0, "%s tried to cast shadow copy on you but failed.\n", ch[cn].reference);
			if (!IS_IGNORING_SPELLS(co))
			{
				do_notify_char(co, NT_GOTMISS, cn, 0, 0, 0);
			}
		}
		return;
	}
	
	cc = god_create_char(CT_COMPANION, 1); 
	
	if (!god_drop_char_fuzzy(cc, ch[cn].x, ch[cn].y))
	{
		ch[cc].used = USE_EMPTY;
		do_char_log(cn, 0, "The shadow copy could not materialize.\n");
		return;
	}

	if (co)
	{
		if (!IS_IGNORING_SPELLS(co)) { do_notify_char(co, NT_GOTHIT, cn, 0, 0, 0); }
		do_notify_char(cn, NT_DIDHIT, co, 0, 0, 0);
	}
	
	if (ch[cn].flags & CF_PLAYER) { ch[cn].data[CHD_SHADOWCOPY] = cc; }
	
	if (it[ch[cn].worn[WN_LHAND]].temp==IT_BOOK_NECR) necronomicon = 1;
	
	// Mod determines how strong the copy is compared to you.
	// The first 120 points reaches a perfect copy. Beyond 180 points the shadow becomes *stronger* than you.
	base = spell_race_mod(get_skill_score(cn, SK_SHADOW), cn);
	power = base/8 - 10;
	
	if (necronomicon)
	{
		sprintf(ch[cc].name, "%c%c%c%c%c's Zombie", ch[cn].name[0], ch[cn].name[1], ch[cn].name[2], ch[cn].name[3], ch[cn].name[4]);
		ch[cc].flags |= CF_UNDEAD;
		ch[cc].sprite = 19408;
	}
	else
	{
		sprintf(ch[cc].name, "%c%c%c%c%c's Shadow", ch[cn].name[0], ch[cn].name[1], ch[cn].name[2], ch[cn].name[3], ch[cn].name[4]);
	}
	ch[cc].data[29] = 0;				// reset experience earned
	ch[cc].data[42] = 65536 + cn;		// set group
	ch[cc].kindred &= ~(KIN_MONSTER);	// Add 'monster' flag

	if (co)
	{
		ch[cc].attack_cn = co;
		idx = co | (char_id(co) << 16);
		ch[cc].data[80] = idx;            // add enemy to kill list
	}
	ch[cc].data[63] = cn;
	ch[cc].data[69] = cn;
	
	// Set duration of SC based on skill mod. Always lasts at least 15 seconds, up to 40ish
	ch[cc].data[CHD_COMPANION] = globs->ticker + TICKS*15 + TICKS*base/10;
	
	ch[cc].data[98] = globs->ticker + COMPANION_TIMEOUT;

	strcpy(ch[cc].text[0], "#14#%s has been eliminated.");
	strcpy(ch[cc].text[1], "#13#Your death will be swift, %s!");
	strcpy(ch[cc].text[3], "I am but one of many, %s!");
	ch[cc].data[CHD_TALKATIVE] = ch_temp[CT_COMPANION].data[CHD_TALKATIVE];
	
	if (ch[cn].flags & CF_SILENCE) 
	{
		ch[cc].data[CHD_TALKATIVE] = 0;
		ch[cc].flags |= CF_SILENCE;
	}

	ch[cc].data[48] = 33;
	
	ch[cc].data[1] = 2; // Offense
	
	ch[cc].flags |= CF_SHADOWCOPY;
	
	if ((ch[cn].flags & CF_PLAYER) || ch[cn].skill[SK_PRECISION][0]) 
	{
		ch[cc].flags |= CF_CANCRIT;
	}
	
	wpnskill = get_skill_score(cn, SK_HAND);
	ch[cc].data[2] = 1; // Base crit chance
	
	for (n = 0; n<5; n++)
	{
		ch[cc].attrib[n][0] = 1;
	}
	for (n = 0; n<MAXSKILL; n++)
	{
		ch[cc].skill[n][0] = 0;
	}
	
	in = ch[cn].worn[WN_RHAND];
	
	if (it[in].flags & IF_WP_CLAW)
	{
		ch[cc].data[2] = 3;
	}
	if (it[in].flags & IF_WP_SWORD)
	{
		wpnskill = get_skill_score(cn, SK_SWORD);
		ch[cc].data[2] = 1;
	}
	if ((it[in].flags & IF_WP_DAGGER) && (it[in].flags & IF_WP_STAFF)) // Spear
	{
		if (get_skill_score(cn, SK_DAGGER) > get_skill_score(cn, SK_STAFF))
		{
			wpnskill = get_skill_score(cn, SK_DAGGER);
			ch[cc].data[2] = 2;
		}
		else
		{
			wpnskill = get_skill_score(cn, SK_STAFF);
			ch[cc].data[2] = 1;
		}
	}
	if (it[in].flags & IF_WP_DAGGER)
	{
		wpnskill = get_skill_score(cn, SK_DAGGER);
		ch[cc].data[2] = 2;
	}
	if (it[in].flags & IF_WP_STAFF)
	{
		wpnskill = get_skill_score(cn, SK_STAFF);
		ch[cc].data[2] = 1;
	}
	if ((it[in].flags & IF_WP_AXE) && (it[in].flags & IF_WP_TWOHAND)) // Greataxe
	{
		if (get_skill_score(cn, SK_AXE) > get_skill_score(cn, SK_TWOHAND))
		{
			wpnskill = get_skill_score(cn, SK_AXE);
			ch[cc].data[2] = 1;
		}
		else
		{
			wpnskill = get_skill_score(cn, SK_TWOHAND);
			ch[cc].data[2] = 2;
		}
	}
	if (it[in].flags & IF_WP_AXE)
	{
		wpnskill = get_skill_score(cn, SK_AXE);
		ch[cc].data[2] = 1;
	}
	if (it[in].flags & IF_WP_TWOHAND)
	{
		wpnskill = get_skill_score(cn, SK_TWOHAND);
		ch[cc].data[2] = 2;
	}
	
	ch[cc].skill[SK_HAND][0] 	  = max(1,  min(wpnskill+power, powlimit));
	
	if (ch[cn].skill[SK_COMBATM][0]) 	
		ch[cc].skill[SK_COMBATM][0]   = max(0,  min(get_skill_score(cn, SK_COMBATM)+power, powlimit));
	if (ch[cn].skill[SK_PRECISION][0]) 	
		ch[cc].skill[SK_PRECISION][0] = max(0,  min(get_skill_score(cn, SK_PRECISION)+power, powlimit));
	ch[cc].skill[SK_RESIST][0] 	  = max(0,  min(get_skill_score(cn, SK_RESIST)+power, powlimit));
	ch[cc].skill[SK_PERCEPT][0]   = max(50, min(get_skill_score(cn, SK_PERCEPT)+power, powlimit));
	
	ch[cc].skill[SK_IMMUN][0] 	  = max(0,  min(get_skill_score(cn, SK_IMMUN)+power, powlimit));
	if (ch[cn].skill[SK_SURROUND][0]) 
		ch[cc].skill[SK_SURROUND][0]  = max(0,  min(get_skill_score(cn, SK_SURROUND)+power, powlimit));
	
	ch[cc].hp[0]   				  = max(50, min(ch[cn].hp[5]+power*5, 999));
	ch[cc].end[0]  				  = max(100,min(ch[cn].end[5]+power*5, 999));
	ch[cc].mana[0] 				  = max(50, min(ch[cn].mana[5]+power*5, 999));
	
	// Special case skills - never stronger than their original
	power -= 10;
	
	ch[cc].armor_bonus  		  = max(0,  min(ch[cn].armor+power, powlimit));
	
	if (ch[cn].skill[SK_CLEAVE][0]) 
		ch[cc].skill[SK_CLEAVE][0] 	  = max(0,  min(get_skill_score(cn, SK_CLEAVE)+power, powlimit));
	if (ch[cn].skill[SK_BLAST][0]) 
		ch[cc].skill[SK_BLAST][0] 	  = max(0,  min(get_skill_score(cn, SK_BLAST)+power, powlimit));
	
	power -= 20;
	
	ch[cc].weapon_bonus 		  = max(0,  min(ch[cn].weapon+power, powlimit));
	
	if (ch[cn].skill[SK_WEAKEN][0]) 
		ch[cc].skill[SK_WEAKEN][0] 	  = max(0,  min(get_skill_score(cn, SK_WEAKEN)+power, powlimit));
	if (ch[cn].skill[SK_CURSE][0]) 
		ch[cc].skill[SK_CURSE][0] 	  = max(0,  min(get_skill_score(cn, SK_CURSE)+power, powlimit));
	if (ch[cn].skill[SK_SLOW][0]) 
		ch[cc].skill[SK_SLOW][0] 	  = max(0,  min(get_skill_score(cn, SK_SLOW)+power, powlimit));

	
	ch[cc].points_tot = ch[cn].points_tot + (ch[cn].points_tot/100 * (power+20));
	ch[cc].gold   = 0;
	ch[cc].a_hp   = 999999;
	ch[cc].a_end  = 999999;
	ch[cc].a_mana = 999999;

	ch[cc].alignment = ch[cn].alignment / 2;

	xlog("Created %s (%d) with base %d as Shadow Copy for %s", ch[cc].name, cc, base, ch[cn].reference);

	/* CS, 000109: Less chatty GC */
	if (co)
	{
		do_sayx(cc, ch[cc].text[1], ch[co].name);
	}
	else
	{
		tmp = 100;
		
		if (necronomicon) tmp = RANDOM(40);
		switch (tmp)
		{
			case 0:		do_sayx(cc, "I shall rend their flesh!"); break;
			case 1:		do_sayx(cc, "Hahahahah..."); break;
			case 2:		do_sayx(cc, "Who... what am I...?"); break;
			case 3:		do_sayx(cc, "They watch you with great interest, %s.", ch[cn].name); break;
			case 4:		do_sayx(cc, "#9#My time is short, %s. Point me to the enemy.", ch[cn].name); break;
			default:	do_sayx(cc, "My time is short, %s. Point me to the enemy.", ch[cn].name); break;
		}
	}
	
	do_update_char(cc);
	
	// Add a little buff icon to the player to show them roughly how much time the copy has left
	in = god_create_buff();
	if (!in)
	{
		xlog("god_create_buff failed in skill_shadow");
		return;
	}
	
	strcpy(bu[in].name, "Shadow Copy");
	bu[in].flags 	|= IF_SPELL;
	bu[in].sprite[1] = BUF_SPR_SHADOW;
	bu[in].duration  = bu[in].active = SP_DUR_SHADOW(base);
	bu[in].temp  	 = SK_SHADOW;
	bu[in].power 	 = base;
	
	add_spell(cn, in);
	//
	
	fx_add_effect(6, 0, ch[cc].x, ch[cc].y, 0);
	fx_add_effect(7, 0, ch[cn].x, ch[cn].y, 0);
	
	add_exhaust(cn, SK_EXH_SHADOW);
}

int spell_bleed(int cn, int co, int power)
{
	int in, n;

	if (ch[cn].attack_cn!=co && ch[co].alignment==10000) { return 0; }
	if (ch[co].flags & CF_IMMORTAL) { return 0; }

	in = god_create_buff();
	if (!in)
	{
		xlog("god_create_buff failed in spell_bleed");
		return( 0);
	}

	power = spell_immunity(power, get_target_immunity(co));
	
	strcpy(bu[in].name, "Bleed");
	bu[in].flags |= IF_SPELL;
	bu[in].sprite[1] = BUF_SPR_BLEED;
	bu[in].duration  = bu[in].active = SP_DUR_BLEED;
	bu[in].temp  = SK_BLEED;
	bu[in].power = power;
	bu[in].cost = cn; // the user of the bleed should earn the kill for the bleed.

	add_spell(co, in);
	
	chlog(cn, "Inflicted Bleed on %s", ch[co].name);
	do_char_log(co, 0, "You began bleeding from the cleave!\n");

	return 1;
}
void skill_cleave(int cn)
{
	int co,dam,m,co_orig,tmp,base_power,power,cost;
	int bleeding = 0;
	int in=0;
	
	if ((co = ch[cn].skill_target1)) { ; }
	if (ch[cn].attack_cn!=0) { co = ch[cn].attack_cn; }
	else {
		do_char_log(cn,0,"But you're not fighting anybody!\n");
		return;
	}

	if (!is_facing(cn,co)) {
		do_char_log(cn,0,"You must be facing your enemy!\n");
		return;
	}
	
	if (!do_char_can_see(cn, co))
	{
		do_char_log(cn, 0, "You cannot see your target.\n");
		return;
	}
	
	if (ch[co].flags&CF_STONED) {
		do_char_log(cn,0,"Your target is lagging. Try again later.\n");
		return;
	}

	if (!may_attack_msg(cn,co,1)) {
		chlog(cn,"Prevented from attacking %s (%d)",ch[co].name,co);
		return;
	}
	
	/* CS, 000209: Remember PvP attacks */
	remember_pvp(cn,co);
	
	if (is_exhausted(cn)) return;
	
	base_power = get_skill_score(cn, SK_CLEAVE)+ch[cn].weapon/4;
	power = spell_immunity(base_power, get_fight_skill(co)+get_offhand_skill(co,2)+get_combat_skill(co,0));
	dam = power * 2;
	
	cost = dam / 24 + 5;
	
	if ((ch[cn].flags & CF_PLAYER) && (ch[cn].kindred & (KIN_TEMPLAR | KIN_ARCHTEMPLAR | KIN_PUGILIST)))
		cost = (cost/3)*2;
	if ((ch[cn].flags & CF_PLAYER) && (ch[cn].kindred & KIN_WARRIOR))
		cost = (cost/6)*5;
	if (!(ch[cn].flags & CF_PLAYER)) 
		cost = cost*3/2;
	
	if (spellcost(cn, cost, SK_CLEAVE, 0)) return;
	
	// Tarot Card - Justice :: Weaken Cleave's damage & inflict Bleed
	if (get_tarot(cn, IT_CH_JUSTICE))
	{
		dam = dam * 70/100;
		bleeding = 1;
	}

	do_area_sound(co,0,ch[co].x,ch[co].y,ch[cn].sound+6);
	char_play_sound(co,ch[cn].sound+6,-300,0);

	chlog(cn,"Used Cleave on %s",ch[co].name);

	tmp=do_hurt(cn,co,dam,5);

	if (tmp<1) do_char_log(cn,0,"You cannot penetrate %s's armor.\n",ch[co].reference);
	else 
	{
		do_char_log(cn,1,"You cleaved %s for %d HP.\n",ch[co].reference,tmp);
		do_char_log(co,1,"%s cleaved you for %d HP.\n",ch[cn].name,tmp);
	}
	
	fx_add_effect(5,0,ch[co].x,ch[co].y,0);
	
	if (bleeding) spell_bleed(cn, co, dam);

	co_orig=co;

	dam=dam/2+dam/4;

	m=ch[cn].x+ch[cn].y*MAPX;
	if ((co=map[m+1].ch)!=0 && ch[co].attack_cn==cn && co_orig!=co) {
		chlog(cn,"Used Cleave on %s",ch[co].name);
		
		power = spell_immunity(base_power, get_fight_skill(co)+get_offhand_skill(co,2)+get_combat_skill(co,0));
		dam = (power * 2)/4*3; if (bleeding) dam = dam * 70/100;
		
		tmp=do_hurt(cn,co,dam,5);

		if (tmp<1) do_char_log(cn,0,"You cannot penetrate %s's armor.\n",ch[co].reference);
		else 
		{
			do_char_log(cn,1,"You cleaved %s for %d HP.\n",ch[co].reference,tmp);
			do_char_log(co,1,"%s cleaved you for %d HP.\n",ch[cn].name,tmp);
		}
		fx_add_effect(5,0,ch[co].x,ch[co].y,0);
		if (bleeding) spell_bleed(cn, co, dam);
	}
	if ((co=map[m-1].ch)!=0 && ch[co].attack_cn==cn && co_orig!=co) {
		chlog(cn,"Used Cleave on %s",ch[co].name);
		
		power = spell_immunity(base_power, get_fight_skill(co)+get_offhand_skill(co,2)+get_combat_skill(co,0));
		dam = (power * 2)/4*3; if (bleeding) dam = dam * 70/100;
		
		tmp=do_hurt(cn,co,dam,5);

		if (tmp<1) do_char_log(cn,0,"You cannot penetrate %s's armor.\n",ch[co].reference);
		else 
		{
			do_char_log(cn,1,"You cleaved %s for %d HP.\n",ch[co].reference,tmp);
			do_char_log(co,1,"%s cleaved you for %d HP.\n",ch[cn].name,tmp);
		}
		fx_add_effect(5,0,ch[co].x,ch[co].y,0);
		if (bleeding) spell_bleed(cn, co, dam);
	}
	if ((co=map[m+MAPX].ch)!=0 && ch[co].attack_cn==cn && co_orig!=co) {
		chlog(cn,"Used Cleave on %s",ch[co].name);

		power = spell_immunity(base_power, get_fight_skill(co)+get_offhand_skill(co,2)+get_combat_skill(co,0));
		dam = (power * 2)/4*3; if (bleeding) dam = dam * 70/100;

		tmp=do_hurt(cn,co,dam,5);

		if (tmp<1) do_char_log(cn,0,"You cannot penetrate %s's armor.\n",ch[co].reference);
		else 
		{
			do_char_log(cn,1,"You cleaved %s for %d HP.\n",ch[co].reference,tmp);
			do_char_log(co,1,"%s cleaved you for %d HP.\n",ch[cn].name,tmp);
		}
		fx_add_effect(5,0,ch[co].x,ch[co].y,0);
		if (bleeding) spell_bleed(cn, co, dam);
	}
	if ((co=map[m-MAPX].ch)!=0 && ch[co].attack_cn==cn && co_orig!=co) {
		chlog(cn,"Used Cleave on %s",ch[co].name);

		power = spell_immunity(base_power, get_fight_skill(co)+get_offhand_skill(co,2)+get_combat_skill(co,0));
		dam = (power * 2)/4*3; if (bleeding) dam = dam * 70/100;

		tmp=do_hurt(cn,co,dam,5);

		if (tmp<1) do_char_log(cn,0,"You cannot penetrate %s's armor.\n",ch[co].reference);
		else 
		{
			do_char_log(cn,1,"You cleaved %s for %d HP.\n",ch[co].reference,tmp);
			do_char_log(co,1,"%s cleaved you for %d HP.\n",ch[cn].name,tmp);
		}
		fx_add_effect(5,0,ch[co].x,ch[co].y,0);
		if (bleeding) spell_bleed(cn, co, dam);
	}
	if (bleeding) 
	{
		do_char_log(cn, 1, "Your foes began bleeding from your cleave!\n");
	}
	
	add_exhaust(cn, SK_EXH_CLEAVE);
}

int spell_weaken(int cn, int co, int power, int flag)
{
	int in, n;

	if (ch[cn].attack_cn!=co && ch[co].alignment==10000) { return 0; }
	if (ch[co].flags & CF_IMMORTAL) { return 0; }

	in = god_create_buff();
	if (!in)
	{
		xlog("god_create_buff failed in spell_weaken");
		return( 0);
	}
	
	power = spell_immunity(power, get_target_immunity(co));
	
	// Tarot Card - Death :: Change Weaken into Weaken II
	if (get_tarot(cn, IT_CH_DEATH))
	{
		strcpy(bu[in].name, "Greater Weakness");
		bu[in].flags |= IF_SPELL;
		bu[in].armor[1]  = -(power / 6 + 2);
		bu[in].sprite[1] = BUF_SPR_REND2;
		bu[in].duration  = bu[in].active = SP_DUR_WEAKEN;
		bu[in].temp  = SK_WEAKEN2;
		bu[in].power = power;

		if (!add_spell(co, in))
		{
			if (!flag)
				do_char_log(cn, 1, "Your opponent is already badly weakened!\n");
			return 0;
		}
		if (get_skill_score(co, SK_SENSE) + 10>power)
			do_char_log(co, 1, "%s badly weakened you.\n", ch[cn].reference);
		else
			do_char_log(co, 0, "Your equipment feels very heavy.\n");
		if (!flag)
			do_char_log(cn, 1, "%s was badly weakened.\n", ch[co].name);
		if (!IS_IGNORING_SPELLS(co))
			do_notify_char(co, NT_GOTHIT, cn, 0, 0, 0);
		do_notify_char(cn, NT_DIDHIT, co, 0, 0, 0);
		char_play_sound(co, ch[cn].sound + 7, -300, 0);
		char_play_sound(cn, ch[cn].sound + 1, -300, 0);
		chlog(cn, "Used Greater Weaken on %s", ch[co].name);
		fx_add_effect(5, 0, ch[co].x, ch[co].y, 0);
	}
	else
	{
		strcpy(bu[in].name, "Weakness");
		bu[in].flags |= IF_SPELL;
		bu[in].weapon[1]  = -(power / 8 + 2);
		bu[in].armor[1]  = -(power / 8 + 2);
		bu[in].sprite[1] = BUF_SPR_REND;
		bu[in].duration  = bu[in].active = SP_DUR_WEAKEN;
		bu[in].temp  = SK_WEAKEN;
		bu[in].power = power;

		if (!add_spell(co, in))
		{
			if (!flag)
				do_char_log(cn, 1, "Your opponent is already weakened!\n");
			return 0;
		}
		if (get_skill_score(co, SK_SENSE) + 10>power)
			do_char_log(co, 1, "%s weakened you.\n", ch[cn].reference);
		else
			do_char_log(co, 0, "Your equipment feels heavy.\n");
		if (!flag)
			do_char_log(cn, 1, "%s was weakened.\n", ch[co].name);
		if (!IS_IGNORING_SPELLS(co))
			do_notify_char(co, NT_GOTHIT, cn, 0, 0, 0);
		do_notify_char(cn, NT_DIDHIT, co, 0, 0, 0);
		char_play_sound(co, ch[cn].sound + 7, -300, 0);
		char_play_sound(cn, ch[cn].sound + 1, -300, 0);
		chlog(cn, "Used Weaken on %s", ch[co].name);
		fx_add_effect(5, 0, ch[co].x, ch[co].y, 0);
	}

	return 1;
}
void skill_weaken(int cn)
{
	int d20 = 11;
	int co, co_orig, m, power, defence, chance, roll, tmp;
	int cost = SP_COST_WEAKEN;

	if ((co = ch[cn].skill_target1)) { ; }
	if (ch[cn].attack_cn!=0) { co = ch[cn].attack_cn; }
	else {
		do_char_log(cn,0,"But you're not fighting anybody!\n");
		return;
	}

	if (!is_facing(cn,co)) {
		do_char_log(cn,0,"You must be facing your enemy!\n");
		return;
	}
	
	if (!do_char_can_see(cn, co))
	{
		do_char_log(cn, 0, "You cannot see your target.\n");
		return;
	}
	
	if (ch[co].flags&CF_STONED) {
		do_char_log(cn,0,"Your target is lagging. Try again later.\n");
		return;
	}

	if (!may_attack_msg(cn,co,1)) {
		chlog(cn,"Prevented from attacking %s (%d)",ch[co].name,co);
		return;
	}

	/* CS, 000209: Remember PvP attacks */
	remember_pvp(cn,co);
	if (is_exhausted(cn)) return;
	if (get_tarot(cn, IT_CH_DEATH)) cost *= (3/2);
	if (spellcost(cn, cost, SK_WEAKEN, 0)) return;
	
	power 	= get_skill_score(cn, SK_WEAKEN);
	defence = get_target_resistance(co);
	chance 	= d20 * power / max(1, defence);
	if (ch[cn].flags & (CF_PLAYER))
	{
		if (ch[cn].luck<0)
		{
			chance += ch[cn].luck / 500 - 1;
		}
	}
	
	if (chance< 0) chance= 0;
	if (chance>18) chance=18;
	
	roll = RANDOM(20);
	
	if (ch[cn].flags & (CF_PLAYER)) // Players have small advantage
	{
		tmp = RANDOM(20);
		if (tmp < roll) roll = (tmp+roll)/2;
	}
	if (it[ch[cn].worn[WN_LHAND]].temp==IT_BOOK_ADVA) // Book: Castor's Advantage
	{
		tmp = RANDOM(20);
		if (tmp < roll) roll = tmp;
	}
	
	damage_mshell(co);
	
	if (roll>chance || defence>power + (power / 2))
	{
		do_char_log(co, 0, "%s tried to weaken you but failed.\n", ch[cn].reference);
		do_char_log(cn, 0, "%s resisted your attempt to weaken them.\n", ch[co].reference);
		if (!(ch[cn].flags & (CF_PLAYER)))
		{
			add_exhaust(cn, TICKS * 4); // 4 second exhaust for NPCS to keep them from spam-failing
		}
		return;
	}

	if (ch[co].flags&CF_IMMORTAL) {
			do_char_log(cn,0,"Your attempt to weaken an immortal has failed.\n");
			return;
	}

	spell_weaken(cn, co, get_skill_score(cn, SK_WEAKEN), 0);

	co_orig=co;
	m=ch[cn].x+ch[cn].y*MAPX;
	if ((co=map[m+1].ch)!=0 && ch[co].attack_cn==cn && co_orig!=co) 
	{
		damage_mshell(co);
		if (power+RANDOM(20)>get_target_resistance(co)+RANDOM(16)) 
			spell_weaken(cn,co,power, 0);
	}
	if ((co=map[m-1].ch)!=0 && ch[co].attack_cn==cn && co_orig!=co) 
	{
		damage_mshell(co);
		if (power+RANDOM(20)>get_target_resistance(co)+RANDOM(16)) 
			spell_weaken(cn,co,power, 0);
	}
	if ((co=map[m+MAPX].ch)!=0 && ch[co].attack_cn==cn && co_orig!=co) 
	{
		damage_mshell(co);
		if (power+RANDOM(20)>get_target_resistance(co)+RANDOM(16)) 
			spell_weaken(cn,co,power, 0);
	}
	if ((co=map[m-MAPX].ch)!=0 && ch[co].attack_cn==cn && co_orig!=co) 
	{
		damage_mshell(co);
		if (power+RANDOM(20)>get_target_resistance(co)+RANDOM(16)) 
			spell_weaken(cn,co,power, 0);
	}
	
	add_exhaust(cn, SK_EXH_WEAKEN);
}

void nomagic(int cn)
{
	do_char_log(cn, 0, "Your magic fails. You seem to be unable to cast spells.\n");
}

/* Look up skills by name. Return the index of the skill in skilltab or -1 if not found.
   The matching algorithm tries to be very tolerant: It succeeds when either of the words
   ends or hits a blank with no discrepancy found so far.
   For compatibility reasons, it also decodes numeric skill values. */
int skill_lookup(char *name)
{
	int n, j;
	char *p, *q;

	if (*name == '\0')
	{
		return( -1);                            // empty string does not match
	}
	if (name[0]=='0' && name[1]=='\0')
	{
		return( 0);                             // special case: "0".
	}
	n = atoi(name);                                 // try numeric
	if (!SANESKILL(n))
	{
		return( -1);                            // bad numeric
	}
	if (n > 0)
	{
		return( n);                             // good numeric
	}
	// try alpha
	for (j = 0; j<MAXSKILL; j++)
	{
		for (p = name, q = skilltab[j].name; 1; p++, q++)
		{
			if (*p=='\0' || *q=='\0' || *q==' ')
			{
				return( j);
			}
			if (tolower(*p) != tolower(*q))
			{
				break;
			}
		}
	}
	// fallen out of loop: not found
	return(-1);
}

void skill_driver(int cn, int nr)
{
	int tmp = 0;
	
//      ch[cn].errno=ERR_FAILED;        // will be overriden later if another result is desired

	if (!ch[cn].skill[nr][0])
	{
		do_char_log(cn, 0, "You cannot use this skill/spell.\n");
		return;
	}

	switch(nr)
	{
		case    SK_LIGHT:
			if (ch[cn].flags & CF_NOMAGIC)
				nomagic(cn);
			else
				skill_light(cn);
			break;
			
		case    SK_PROTECT:
			if (ch[cn].flags & CF_NOMAGIC)
				nomagic(cn);
			else
				skill_protect(cn);
			break;
			
		case    SK_ENHANCE:
			if (ch[cn].flags & CF_NOMAGIC)
				nomagic(cn);
			else
				skill_enhance(cn);
			break;
			
		case    SK_BLESS:
			if (ch[cn].flags & CF_NOMAGIC)
				nomagic(cn);
			else
				skill_bless(cn);
			break;
			
		case    SK_CURSE:
			if (ch[cn].flags & CF_NOMAGIC)
				nomagic(cn);
			else
				skill_curse(cn);
			break;
			
		case    SK_SLOW:
			if (ch[cn].flags & CF_NOMAGIC)
				nomagic(cn);
			else
				skill_slow(cn, 0);
			break;
			
		case    SK_IDENT:
			if (ch[cn].flags & CF_NOMAGIC)
				nomagic(cn);
			else
				skill_identify(cn);
			break;
			
		case    SK_BLAST:
			if (ch[cn].flags & CF_NOMAGIC)
				nomagic(cn);
			else
				skill_blast(cn);
			break;
			
		case    SK_POISON:
			if (ch[cn].flags & CF_NOMAGIC)
				nomagic(cn);
			else
				skill_poison(cn);
			break;
			
		case    SK_REPAIR:
			skill_repair(cn);
			break;

		case    SK_RECALL:
			if (ch[cn].flags & CF_NOMAGIC)
				nomagic(cn);
			else
				skill_recall(cn);
			break;

		case    SK_DISPEL:
			if (ch[cn].flags & CF_NOMAGIC)
				nomagic(cn);
			else
				skill_dispel(cn);
			break;
		
		case    SK_HASTE:
			if (ch[cn].flags & CF_NOMAGIC)
				nomagic(cn);
			else
				skill_haste(cn);
			break;
		
		case    SK_HEAL:
			if (ch[cn].flags & CF_NOMAGIC)
				nomagic(cn);
			else
				skill_heal(cn);
			break;

		case    SK_GHOST:
			if (ch[cn].flags & CF_NOMAGIC)
				nomagic(cn);
			else
				skill_ghost(cn);
			break;
			
		case    SK_SHADOW:
			if (ch[cn].flags & CF_NOMAGIC)
				nomagic(cn);
			else
				skill_shadow(cn);
			break;
			
		case    SK_MSHIELD:
			if (ch[cn].flags & CF_NOMAGIC)
				nomagic(cn);
			else
				skill_mshield(cn);
			break;

		case    SK_IMMUN:
		case	SK_RESIST:
			do_char_log(cn, 0, "You use this skill automatically when someone casts negative spells on you.\n");
			break;

		case    SK_REGEN:
		case	SK_REST:
		case    SK_MEDIT:
			do_char_log(cn, 0, "You use this skill automatically.\n");
			break;

		case    SK_DAGGER:
		case    SK_SWORD:
		case    SK_AXE:
		case    SK_STAFF:
		case    SK_TWOHAND:
		case    SK_SURROUND:
			do_char_log(cn, 0, "You use this skill automatically when you fight.\n");
			break;

		case    SK_CONCEN:
			do_char_log(cn, 0, "You use this skill automatically when you cast spells.\n");
			break;

		case    SK_CLEAVE:
			skill_cleave(cn);
			break;
			
		case    SK_WEAKEN:
			skill_weaken(cn);
			break;

		case    SK_WARCRY:
			skill_warcry(cn);
			break;

		default:
			do_char_log(cn, 0, "You cannot use this skill/spell.\n");
			break;
	}
}

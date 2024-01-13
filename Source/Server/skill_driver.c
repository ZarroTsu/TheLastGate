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

struct s_splog splog[66] = {
	{
		0
	},{
		SK_EXHAUST, 	"Exhaust",		"exhaust",		"exhausting"
	},{ 
		SK_BLEED, 		"Bleed",		"bleed",		"bleeding"
	},{ 
		SK_WEAKEN2, 	"Crushed",		"crushed",	"crushing",
		"Your armor feels heavy.",
		" was crushed.",
		" crushed your defenses."
	},{ 
		SK_SCORCH, 		"Scorch",		"scorch",		"scorching",
		"You have been scorched.",
		" was scorched.",
		" scorched you."
	},{ 
		SK_CURSE2, 		"Greater Curse",	"curse",	"cursing",
		"You have been badly cursed.",
		" was badly cursed.",
		" cast greater curse on you."
	},{ 
		SK_SLOW2, 		"Greater Slow",		"slow",		"slowing",
		"You have been badly slowed.",
		" was badly slowed.",
		" cast greater slow on you."
	},{
		SK_ZEPHYR,		"Zephyr",		"zephyr",		"zephyring",
		"Zephyr active!",
		"'s Zephyr activated.",
		" cast zephyr on you."
	},{
		SK_ZEPHYR2,		"Zephyr",		"zephyr",		"zephyring"
	},{
		SK_POME,		"Pome",			"pome",			"poming"
	},{
		SK_DOUSE,		"Douse",		"douse",		"dousing",
		"You have been doused.",
		" was doused.",
		" cast douse on you."
	},{ 
		SK_MSHIELD,		"Magic Shield",	"magic shield",	"magic shielding",
		"Magic Shield active!",
		"'s Magic Shield activated.",
		" cast magic shield on you."
	},{
		SK_MSHELL, 		"Magic Shell",	"magic shell",	"magic shelling",
		"Magic Shell active!",
		"'s Magic Shell activated.",
		" cast magic shell on you."
	},{
		SK_REPAIR, 		"Repair",		"repair",		"repairing"
	},{ 
		SK_PLAGUE,		"Plague",		"plague",		"plaguing"
	},{ 
		SK_LETHARGY, 	"Lethargy",		"lethargy",		"letharging",
		"You enter a lethargic state.",
		" becomes lethargic.", ""
	},{
		SK_GUARD,		"Guard",		"guard",		"guarding",
		"You raise your guard.",
		" rose their guard.",
		" cast guard on you."
	},{
		SK_PROTECT,		"Protect",		"protect",		"protecting",
		"You feel protected.",
		" is now protected.",
		" cast protect on you."
	},{ 
		SK_ENHANCE,		"Enhance",		"enhance",		"enhancing",
		"Your weapon feels stronger.",
		"'s weapon is now stronger.",
		" cast enhance weapon on you."
	},{ 
		SK_SLOW, 		"Slow",			"slow",			"slowing",
		"You have been slowed.",
		" was slowed.",
		" cast slow on you.",
		"You unleash a powerful mass-slow.",
		"You feel a chilling power emanate from somewhere.",
		" tried to include you in a mass-slow but failed."
	},{ 
		SK_CURSE,		"Curse",		"curse",		"cursing",
		"You have been cursed.",
		" was cursed.",
		" cast curse on you.",
		"You unleash a powerful mass-curse.",
		"You feel a wiked power emanate from somewhere.",
		" tried to include you in a mass-curse but failed."
	},{ 
		SK_BLESS, 		"Bless",		"bless",		"blessing",
		"You have been blessed.",
		" was blessed.",
		" cast bless on you."
	},{ 
		SK_RAGE, 		"Rage",			"rage",			"raging",
		"You enter a bloodthirsty rage!",
		" starts to rage.", ""
	},{
		SK_STARLIGHT,	"Starlight",	"starlight",	"starlighting",
		"You feel wisened.",
		" was granted starlight.",
		" cast starlight on you."
	},{
		SK_BLAST, 		"Blast",		"blast",		"blasting",
		"","","",
		"You unleash a destructive shockwave.",
		"You feel a tingling shockwave from somewhere.",
		" tried to include you in a mass-blast but failed."
	},{ 
		SK_DISPEL, 		"Immunize",		"dispel",		"immunizing",
		"You became immunized from debuffs!",
		" was immunized.",
		" cast dispel on you."
	},{ 
		SK_HEAL, 		"Healing Sickness",	"heal",		"healing",
		"You have been healed.",
		" was healed.",
		" cast heal on you."
	},{
		SK_GHOST
	},{
		SK_REGEN, 		"Regen",		"regen",		"regenerating",
		"You begin regenerating hitpoints!",
		" starts regenerating.",
		" cast regen on you."
	},{
		SK_VENOM,		"Venom",		"venom",		"envenoming",
		"You have been envenomed!",
		" was envenomed.",
		" cast venom on you.",
		"You unleash a powerful mass-venom.",
		"You feel a putrid aura emanate from somewhere.",
		" tried to include you in a mass-venom but failed."
	},{
		SK_WARCRY3, 	"Rally",		"rally",		"rallying",
		"You feel rallied to a cause!",
		" was rallied.",
		" rallied you."
	},{
		SK_ARIA,		"Aria",			"aria",			"ariaing"
	},{
		SK_DISPEL2, 	"Inoculate",	"dispel",		"inoculating",
		"You became inoculated from buffs!",
		" was inoculated.",
		" cast dispel on you."
	},{
		SK_SOL,			"Sol",			"sol",			"soling"
	},{
		SK_PHALANX,		"Phalanx",		"phalanx",		"phalanxing",
		"You feel well defended.",
		" was granted a phalanx.",
		" cast phalanx on you."
	},{
		SK_WARCRY, 		"Fear",			"fear",			"scaring",
		"","","",
		"You cry out loud and clear."
	},{
		SK_WARCRY2, 	"Stun",			"stun",			"stunning"
	},{
		SK_BLIND,		"Blind",		"blind",		"blinding",
		"","","",
		"You kick up a cloud of dust."
	},{
		SK_AGGRAVATE, 	"Aggravate",	"aggravate",	"aggravating",
		"You feel sore.",
		" was aggravated.",
		" aggravated your wounds."
	},{
		SK_ARIA2,		"Aria",			"aria",			"ariaing"
	},{
		SK_CLEAVE,		"Cleave",		"cleave",		"cleaving"
	},{
		SK_WEAKEN, 		"Weakness",		"weaken",		"weakening",
		"Your weapon feels heavy.",
		" was weakened.",
		" weakened your offenses."
	},{ 
		SK_POISON, 		"Poison",		"poison",		"poisoning",
		"You have been poisoned!",
		" was poisoned.",
		" cast poison on you.",
		"You unleash a powerful mass-poison.",
		"You feel a toxic aura emanate from somewhere.",
		" tried to include you in a mass-poison but failed."
	},{
		SK_PULSE,		"Pulse",		"pulse",		"pulsing",
		"You begin pulsing with energy!",
		" began pulsing with energy.",
		" cast pulse on you."
	},{
		SK_IMMOLATE,	"Immolate",		"immolate",		"immolating"
	},{
		SK_IMMOLATE2,	"Immolate",		"immolate",		"immolating"	
	},{
		SK_SHADOW, 		"Shadow Copy",	"shadow copy",	"shadow copying"
	},{ 
		SK_HASTE, 		"Haste",		"haste",		"hasting",
		"You suddenly feel faster!",
		" has been hasted.",
		" cast haste on you."
	},{
		SK_TAUNT,		"Taunt",		"taunt",		"taunting",
		"You're thrown into a blind rage!",
		" was taunted.",
		" taunted you with insults.",
		"You taunt everyone nearby.",
		"You hear someone yell broad insults.",
		" tried to include you in a mass-taunt but failed."
	},{
		SK_LEAP,		"Leap",			"leap",			"leaping"
	},{
		SK_LIGHT, 		"Light",		"light",		"lighting",
		"You start to emit light.",
		" starts to emit light.",
		" cast light on you."
	},{
		SK_RECALL, 		"Recall",		"recall",		"recalling"
	},{
		SK_IDENT, 		"Identify",		"identify",		"identifying"
	},{
		SK_FEROC, 		"",		"",		""
	},{
		SK_SHIFT, 		"Shift Exhaust",		"shift",		"shifting"
	},{
		SK_CALM, 		"Calm",			"calm",			"calming"
	},{
		SK_FROSTB, 		"Frostburn",	"frostburn",	"frostburning"
	},{
		SK_SHOCK, 		"Shock",		"shock",		"shocked"
	},{
		SK_CHARGE, 		"Charge",		"charge",		"charged"
	},{
		SK_SLOW3, 		"Greater Slow"
	},{
		SK_DIVINITY, 	"Divinity"
	},{
		SK_OPPRESSION, 	"Oppression"
	},{
		SK_OPPRESSED, 	"Oppression"
	},{
		SK_OPPRESSED2, 	"Oppression"
	},{
		SK_PULSE2, 		"Pulse"
	},{
		SK_MJOLNIR, 	"Lightning"
	}
};

void damage_mshell(int co, int dam);
int chance_base(int cn, int co, int skill, int d20, int defense, int usemana, int msg);
int chance(int cn, int d20);
int add_exhaust(int cn, int len);
void skill_slow(int cn, int flag);
int add_spell(int cn, int in);
int spell_plague(int cn, int co, int flag);
int spell_shock(int cn, int co, int power);

int check_gloves(int cn, int co, int orig_co, int dr1, int dr2)
{
	int in, glv, glv_base = 120;
	int d20 = 5;
	
	if (ch[co].escape_timer > TICKS*3) { return 0; }
	if (ch[co].flags & CF_BODY) { return 0; }
	if (ch[cn].attack_cn!=co && ch[co].alignment==10000) { return 0; }
	if (ch[co].flags & CF_IMMORTAL) { return 0; }
	if (co==orig_co) { return 0; }
	
	// Special gloves
	if (!dr1)
	{
		glv = glv_base;
		if (chance_compare(co, glv_base+glv_base/2+RANDOM(20), get_target_resistance(cn, co)+RANDOM(10), 0))
		{
			in = it[ch[cn].worn[WN_ARMS]].temp;
			if (in==IT_GL_SERPENT) 
			{ 
				if (spell_poison(cn, co, glv, 1))
				{
					if (!(ch[cn].flags & CF_SYS_OFF) && !orig_co) 
						do_char_log(cn, 0, "You poisoned your enemies!\n"); 
				}
				else in=0; 
			}
			if (in==IT_GL_BURNING) 
			{ 
				if (spell_scorch(cn, co, glv, 1))
				{	
					if (!(ch[cn].flags & CF_SYS_OFF) && !orig_co) 
						do_char_log(cn, 0, "You scorched your enemies!\n"); 
				}
				else in=0; 
			}
			if (in==IT_GL_SHADOW)  
			{ 
				if (spell_blind(cn, co, glv, 0))
				{	
					if (!(ch[cn].flags & CF_SYS_OFF) && !orig_co) 
						do_char_log(cn, 0, "You blinded your enemies!\n"); 
				}
				else in=0; 
			}
			if (in==IT_GL_CHILLED) 
			{ 
				if (spell_slow(cn, co, glv, 1))
				{ 
					if (!(ch[cn].flags & CF_SYS_OFF) && !orig_co) 
						do_char_log(cn, 0, "You slowed your enemies!\n");   
				}
				else in=0; 
			}
			if (in==IT_GL_CURSED)  
			{ 
				if (spell_curse(cn, co, glv, 1))
				{
					if (!(ch[cn].flags & CF_SYS_OFF) && !orig_co) 
						do_char_log(cn, 0, "You cursed your enemies!\n");   
				}
				else in=0; 
			}
			if (in==IT_GL_TITANS)  
			{ 
				if (spell_weaken(cn, co, glv, 1))
				{
					if (!(ch[cn].flags & CF_SYS_OFF) && !orig_co) 
						do_char_log(cn, 0, "You weakened your enemies!\n"); 
				}
				else in=0; 
			}
			if (in==IT_GL_BLVIPER) 
			{ 
				if (spell_frostburn(cn, co, glv))
				{
					if (!(ch[cn].flags & CF_SYS_OFF) && !orig_co) 
						do_char_log(cn, 0, "You glaciated your enemies!\n"); 
				}
				else in=0; 
			}
			if (in==IT_TW_DOUSER) 
			{ 
				if (spell_blind(cn, co, glv, 1))
				{
					if (!(ch[cn].flags & CF_SYS_OFF) && !orig_co) 
						do_char_log(cn, 0, "You doused your enemies!\n"); 
				}
				else in=0; 
			}
		}
	}
	if (!dr2)
	{
		glv_base += 120;
		glv = glv_base;
		if (chance_compare(co, glv_base+glv_base/2+RANDOM(20), get_target_resistance(cn, co)+RANDOM(10), 0))
		{
			in = it[ch[cn].worn[WN_RHAND]].temp;
			if (in==0) in = it[ch[cn].worn[WN_RHAND]].orig_temp;
			if (in==IT_TW_LUXURIA)
			{
				if (spell_warcry(cn, co, glv, 1)) 
				{
					if (!(ch[cn].flags & CF_SYS_OFF) && !orig_co) 
						do_char_log(cn, 0, "You stunned your enemies!\n"); 
				}
				else in=0; 
			}
		}
	}
	
	return in;
}

int friend_is_enemy(int cn, int cc)
{
	int co;

	if (!(co = ch[cn].attack_cn))	{ return 0; }
	if (may_attack_msg(cc, co, 0))	{ return 0; }
	return 1;
}

int player_or_ghost(int cn, int co)
{
	if (IS_SHADOW(co))					{ return 0; }
	if (!IS_PLAYER(cn))					{ return 1; }
	if (IS_PLAYER(co)) 					{ return 1; }
	if (ch[co].data[CHD_MASTER]==cn)	{ return 1; }
	
	return 0;
}

int is_exhausted(int cn)
{
	int n, in;

	for (n = 0; n<MAXBUFFS; n++)
	{
		if ((in = ch[cn].spell[n])!=0 && bu[in].temp==SK_EXHAUST) { break; }
	}
	if (n<MAXBUFFS)
	{
		do_char_log(cn, 0, "You are still exhausted from your last skill!\n");
		return 1;
	}
	return 0;
}

void nomagic(int cn, int flag)
{
	if (flag)
		do_char_log(cn, 0, "Your magic fizzles as it reaches your target. Seems something stopped it.\n");
	else
		do_char_log(cn, 0, "Your magic fails. You seem to be unable to cast spells.\n");
}

int get_target(int cn, int cnts, int buff, int redir, int cost, int in, int usemana, int power, int d20)
{
	int m, co = 0, aoe_spell = 0, need_combat = 0, cc=0;
	char buf[50];
	strcpy(buf, ch[cn].reference); buf[0] = toupper(buf[0]);
	
	if (cnts && IS_PLAYER(cn))
	{
		ch[cn].data[71] += CNTSAY;
		if (ch[cn].data[71]>MAXSAY)
		{
			do_char_log(cn, 0, "Oops, you're a bit too fast for me!\n");
			return 0;
		}
	}
	
	m = ch[cn].x + ch[cn].y * MAPX;
	
	if (IS_LIVINGCHAR(co = ch[cn].skill_target1)) ;
	else if (!buff && ch[cn].dir==DX_DOWN  && IS_LIVINGCHAR(co = map[m + MAPX].ch) && may_attack_msg(cn, co, 0)>0) ;
	else if (!buff && ch[cn].dir==DX_UP    && IS_LIVINGCHAR(co = map[m - MAPX].ch) && may_attack_msg(cn, co, 0)>0) ;
	else if (!buff && ch[cn].dir==DX_RIGHT && IS_LIVINGCHAR(co = map[m + 1].ch) && may_attack_msg(cn, co, 0)>0) ;
	else if (!buff && ch[cn].dir==DX_LEFT  && IS_LIVINGCHAR(co = map[m - 1].ch) && may_attack_msg(cn, co, 0)>0) ;
	else if (!buff && IS_LIVINGCHAR(co = ch[cn].attack_cn)) ;
	else if (!buff && (ch[cn].dir==DX_RIGHT || ch[cn].dir==DX_LEFT) && IS_LIVINGCHAR(co = map[m + MAPX].ch) && may_attack_msg(cn, co, 0)>0) ;
	else if (!buff && (ch[cn].dir==DX_RIGHT || ch[cn].dir==DX_LEFT) && IS_LIVINGCHAR(co = map[m - MAPX].ch) && may_attack_msg(cn, co, 0)>0) ;
	else if (!buff && (ch[cn].dir==DX_DOWN || ch[cn].dir==DX_UP) && IS_LIVINGCHAR(co = map[m + 1].ch) && may_attack_msg(cn, co, 0)>0) ;
	else if (!buff && (ch[cn].dir==DX_DOWN || ch[cn].dir==DX_UP) && IS_LIVINGCHAR(co = map[m - 1].ch) && may_attack_msg(cn, co, 0)>0) ;
	else if (!buff && ch[cn].dir==DX_UP && IS_LIVINGCHAR(co = map[m + MAPX].ch) && may_attack_msg(cn, co, 0)>0) ;
	else if (!buff && ch[cn].dir==DX_DOWN && IS_LIVINGCHAR(co = map[m - MAPX].ch) && may_attack_msg(cn, co, 0)>0) ;
	else if (!buff && ch[cn].dir==DX_LEFT && IS_LIVINGCHAR(co = map[m + 1].ch) && may_attack_msg(cn, co, 0)>0) ;
	else if (!buff && ch[cn].dir==DX_RIGHT && IS_LIVINGCHAR(co = map[m - 1].ch) && may_attack_msg(cn, co, 0)>0) ;
	else co = cn;
	
	/* Event logging for debug
	if (co!=cn)
	{
		chlog(cn, "  Trying to cast %s on %s", splog[in].name, ch[co].name);
	}
	else
	{
		chlog(cn, "  Trying to cast %s on self", splog[in].name);
	}
	*/
	
	if (usemana && (ch[co].flags & CF_NOMAGIC) && !(IS_IN_CAROV(ch[co].x, ch[co].y)))
	{
		nomagic(cn, 1);
		return 0;
	}
	
	if (in==SK_CLEAVE || in==SK_SHIELD || (in==SK_WEAKEN && !CAN_BRAV_PROX(cn)))
	{
		need_combat = 1;
	}
	
	if (!buff)
	{
		// AoE spells forgo failing since there may be targets handled by cast_aoe_spell
		// The spell cost is set to zero and will be spent in cast_aoe_spell
		if (cn==co && !(ch[cn].flags & CF_AREA_OFF) && ((B_SK(cn, SK_PROX) && ((IS_BRAVER(cn) && in==SK_WEAKEN) || 
			(IS_ARCHHARAKIM(cn) && (in==SK_BLAST||in==SK_POISON)) || (IS_SORCERER(cn) && (in==SK_CURSE||in==SK_SLOW||in==SK_POISON)) )) || 
			(IS_ARCHTEMPLAR(cn) && in==SK_TAUNT) || (IS_WARRIOR(cn) && in==SK_BLIND) ||
			(IS_SEYAN_DU(cn) && (in==SK_CURSE||in==SK_SLOW||in==SK_POISON||in==SK_TAUNT||in==SK_BLAST||in==SK_WEAKEN)) ))
		{
			cost = 0;
		}
		else if (cn==co)
		{
			if (need_combat)
			{
				do_char_log(cn, 0, 
				"But you're not fighting anybody!\n");
			}
			else
			{
				do_char_log(cn, 0, 
				"You cannot %s yourself!\n", splog[in].ref);
			}
			return 0;
		}
		else if (co==ch[cn].data[PCD_SHADOWCOPY] || co==ch[cn].data[PCD_COMPANION])
		{ 
			do_char_log(cn, 0, 
			"You stop yourself from %s your companion. That would be silly.\n", splog[in].act);
			return 0;
		}
	}
	
	if (cn!=co)
	{
		if (need_combat && !is_facing(cn,co))
		{
			do_char_log(cn, 0, "You must be facing your enemy!\n");
			return 0;
		}
		
		if (!do_char_can_see(cn, co, 0))
		{
			do_char_log(cn, 0, "You cannot see your target.\n");
			return 0;
		}
		
		if (!buff && (ch[co].flags & CF_STONED))
		{
			do_char_log(cn, 0, "Your target is lagging. Try again later.\n");
			return 0;
		}
		
		if (!buff)
		{
			remember_pvp(cn, co);
		}
	}
	
	if (in!=SK_LEAP) if (is_exhausted(cn))
	{ 
		return 0; 
	}
	
	if (spellcost(cn, cost, in, usemana))
	{
		return 0; 
	}
	
	if (cn!=co)
	{
		if (!buff)
		{
			if (!may_attack_msg(cn, co, 1))
			{
				chlog(cn, "Prevented from attacking %s (%d)", ch[co].name, co);
				return 0;
			}
			damage_mshell(co, power);
		}
		
		if (redir && !player_or_ghost(cn, co))
		{
			do_char_log(cn, 0, "Changed target of spell from %s to %s.\n", 
				ch[co].name, ch[cn].name);
			
			co = cn;
		}
	}
	
	if (d20!=-1 && (in!=SK_CLEAVE && in!=SK_SHIELD && in!=SK_LEAP && in!=SK_TAUNT && in!=SK_BLIND && in!=SK_DOUSE &&
		((!d20 && chance(cn, FIVE_PERC_FAIL)) || 
		(d20 && cn!=co && chance_base(cn, co, in, d20, get_target_resistance(cn, co), usemana, 1)))))
	{
		if (!buff && cn!=co) do_area_notify(cn, co, ch[cn].x, ch[cn].y, NT_SEEMISS, cn, co, power, 0);
		if (usemana && cn!=co && (CAN_SENSE(co) && M_SK(co, SK_PERCEPT) > power + 5))
		{
			if (!(ch[co].flags & CF_SENSEOFF))
			{
				do_char_log(co, 1, "%s tried to cast %s on you but failed.\n", 
					buf, splog[in].ref);
			}
			if (!buff && !IS_IGNORING_SPELLS(co))
			{
				do_notify_char(co, NT_GOTMISS, cn, 0, 0, 0);
			}
		}
		else if (!usemana && !buff && cn!=co)
		{
			do_char_log(co, 0, "%s tried to %s you but failed.\n", 
				buf, splog[in].ref);
		}
		if (!buff && !IS_PLAYER(cn))
		{
			add_exhaust(cn, TICKS * 4);
		}
		return 0;
	}
	
	if (cn!=co && !need_combat && !buff && (ch[co].flags & CF_IMMORTAL) && in!=SK_LEAP)
	{
		do_char_log(cn, 0, "You lost your focus.\n");
		return 0;
	}
	
	//chlog(cn, "    End of get_target");
	
	if (!buff && cn!=co) do_area_notify(cn, co, ch[cn].x, ch[cn].y, NT_SEEHIT, cn, co, power, 0);
	
	return co;
}

int get_use_mana(int spell)
{
	switch (spell)
	{
		case SK_BLIND:
		case SK_DOUSE:
		case SK_TAUNT:
		case SK_CLEAVE:
		case SK_SHIELD:
		case SK_LEAP:
		case SK_WARCRY:
			return 0;
		default:
			return 1;
	}
}

int cast_aoe_spell(int cn, int co, int intemp, int power, int aoe_power, int cost, int count, int hit, int avgdmg, int dr1, int dr2)
{
	int co_orig, spellaoe, spellrad, hitpower, aoeimm = 0, tmp = 0, crit_dam=0;
	double _cap, _hit, _pow, tmpa, tmph, tmpha, tmpp, tmppa, tmppl=0;
	int xf, yf, xt, yt, xc, yc, x, y, n, in, in2;
	int no_target = 0, usemana = 1;
	char buf[50];
	
	if (co)
	{
		co_orig = co;
	}
	
	hitpower = power;
	aoe_power += 15; // a small extra 'oomph' so the circle is a bit thicker
	
	xc = ch[cn].x;
	yc = ch[cn].y;
	
	switch (intemp)
	{
		case SK_PLAGUE:
			if (IS_SHIFTED(cn))
			{
				spellrad = PRXW_RAD + ch[cn].aoe_bonus;
				_cap = (double)(PRXW_CAP);
				_hit = (double)(PRXW_HIT);
				_pow = (double)(PRXW_POW);
			}
			else
			{
				spellrad = PRXB_RAD + ch[cn].aoe_bonus;
				_cap = (double)(PRXB_CAP);
				_hit = (double)(PRXB_HIT);
				_pow = (double)(PRXB_POW);
			}
			xc = ch[co_orig].x; // plague origin is the infected
			yc = ch[co_orig].y;
			no_target = 1;
			tmppl = power;
			break;
		case SK_WARCRY:
			spellrad = PRXW_RAD + ch[cn].aoe_bonus;
			_cap = (double)(PRXW_CAP);
			_hit = (double)(PRXW_HIT);
			_pow = (double)(PRXW_POW);
			no_target = 1;
			break;
		case SK_TAUNT: if (aoe_power==15) break;
		case SK_BLIND:
		case SK_DOUSE:
			spellrad = PRXB_RAD + ch[cn].aoe_bonus;
			_cap = (double)(PRXB_CAP);
			_hit = (double)(PRXB_HIT);
			_pow = (double)(PRXB_POW);
			if (intemp!=SK_TAUNT) no_target = 1;
			break;
		case SK_BLAST: 
			hitpower = power/2 + power/4;
		default:
			spellrad = ch[cn].aoe_bonus;
			_cap = (double)(PROX_CAP);
			_hit = (double)(PROX_HIT);
			_pow = (double)(PROX_POW);
			break;
	}
	
	spellaoe = (aoe_power/_cap + spellrad) * (T_SORC_SK(cn, 5)?12:10)/10;
	tmpa     = (double)((aoe_power*100/_cap + spellrad*100) * (T_SORC_SK(cn, 5)?12:10)/10);
	tmpha    = (double)((sqr(aoe_power*100/_hit-tmpa)/500+spellrad*300) * (T_SORC_SK(cn, 5)?12:10)/10);
	tmppa    = (double)((sqr(aoe_power*100/_pow-tmpa)/500+spellrad*300) * (T_SORC_SK(cn, 5)?12:10)/10);
	
	xf = max(1, xc - spellaoe);
	yf = max(1, yc - spellaoe);
	xt = min(MAPX - 1, xc + 1 + spellaoe);
	yt = min(MAPY - 1, yc + 1 + spellaoe);
	
	// Loop through and count the number of targets first
	for (x = xf; x<xt; x++) for (y = yf; y<yt; y++) 
	{
		// This makes the radius circular instead of square
		if (sqr(xc - x) + sqr(yc - y) > (sqr(tmpa/100) + 1))
		{
			continue;
		}
		if ((co = map[x + y * MAPX].ch) && cn!=co && co_orig!=co)
		{ 
			if (!do_surround_check(cn, co, 0)) 
			{
				continue;
			}
			damage_mshell(co, power);
			aoeimm += get_target_immunity(cn, co);
			count++;
		}
	}
	if (!count && !no_target)
	{ 
		if (co_orig==ch[cn].data[PCD_SHADOWCOPY] || co_orig==ch[cn].data[PCD_COMPANION])
		{ 
			do_char_log(cn, 0, 
			"You stop yourself from %s your companion. That would be silly.\n", splog[intemp].act);
			return -1;
		}
		else
		{ 
			do_char_log(cn, 0, 
			"You cannot %s yourself!\n", splog[intemp].ref); 
			return -1;
		}
	}
	
	aoeimm /= max(1,count);
	
	if (intemp==SK_BLAST)
	{
		hitpower = spell_immunity(power, aoeimm) * 2;
		hitpower = hitpower/2 + hitpower/4;
		cost = ((power * 2) / 8 + 5) * (PROX_MULTI + aoe_power) / PROX_MULTI;
		
		// Harakim costs less, monster cost more mana
		if (IS_PLAYER(cn) && IS_ANY_HARA(cn))
			cost = cost/3;
		else if (IS_PLAYER_GC(cn))
			cost = 20;
		else if (!IS_PLAYER(cn))
			cost = cost*2;
	}
	
	usemana = get_use_mana(intemp);
	if (!hit && !no_target && spellcost(cn, cost, intemp, usemana))
		return -1;
	
	tmph = (double)(hitpower);
	tmpp = (double)(power);
	
	// Then loop through and apply the effect based off the number of targets
	for (x = xf; x<xt; x++)	for (y = yf; y<yt; y++)	
	{
		// This makes the radius circular instead of square
		if (sqr(xc - x) + sqr(yc - y) > (sqr(tmpa/100) + 1))
		{
			continue;
		}
		if ((co = map[x + y * MAPX].ch) && cn!=co && co_orig!=co)
		{
			// Adjust power to the radius - distant targets take a weaker hit
			hitpower = (int)(double)(min(tmph, tmph / max(1, (
				sqr(abs(xc - x)) + sqr(abs(yc - y))) / (tmpha / 100))));
			power    = (int)(double)(min(tmpp, tmpp / max(1, (
				sqr(abs(xc - x)) + sqr(abs(yc - y))) / (tmppa / 100))));
			if (no_target)
			{
				strcpy(buf, ch[cn].reference); buf[0] = toupper(buf[0]);
				switch (intemp)
				{
					case SK_PLAGUE:
						if (do_surround_check(cn, co, 0) && do_char_can_see(co_orig, co, 1) && spell_plague(cn, co, tmppl))
						{
							for (n = 0; n<MAXBUFFS; n++)
							{
								if ((in = ch[co_orig].spell[n])==0)	continue;
								if (bu[in].data[5]==1 && bu[in].temp!=SK_SHADOW && !has_buff(co, bu[in].temp))
								{
									in2 = copy_buff(in);
									if (bu[in2].data[7] > -1) // 20% more damage per spread
									{
										if (bu[in2].data[7] < 5) bu[in2].data[7]++;
										tmp = 100 + bu[in2].data[7]*20;
										bu[in2].data[1] = bu[in2].data[6] * tmp / 100;
									}
									add_spell(co, in2);
								}
							}
							hit++;
						}
						else
						{
							continue;
						}
						break;
					case SK_WARCRY:
						if (spell_warcry(cn, co, power, 0))
						{
							do_char_log(co, 0, 
								"You hear %s's warcry. You feel frightened and immobilized.\n", ch[cn].reference);
							hit++;
						}
						else
						{
							if (!(ch[co].flags & CF_SYS_OFF))
								do_char_log(co, 0, "You hear %s's warcry.\n", ch[cn].reference);
							continue;
						}
						break;
					case SK_BLIND:
						if (spell_blind(cn, co, power, 0))
						{
							do_char_log(co, 0, 
								"%s kicks up a cloud of dust. You feel your eyes fail you.\n", buf);
							hit++;
						}
						else
						{
							if (!(ch[co].flags & CF_SYS_OFF))
								do_char_log(co, 0, "%s kicks up a cloud of dust.\n", buf);
							continue;
						}
						break;
					case SK_DOUSE:
						if (spell_blind(cn, co, power, 1))
						{
							do_char_log(co, 0, 
								"%s kicks up a splash of mud. You're sopping wet.\n", buf);
							hit++;
						}
						else
						{
							if (!(ch[co].flags & CF_SYS_OFF))
								do_char_log(co, 0, "%s kicks up a splash of mud.\n", buf);
							continue;
						}
						break;
					default:
						break;
				}
			}
			else if (!do_surround_check(cn, co, 1)) 
			{
				continue;
			}
			else if (intemp==SK_BLAST)
			{
				tmp = spell_blast(cn, co, power, 0, 1);
				
				avgdmg += max(0, tmp-1);
				hit++;
				
				check_gloves(cn, co, co_orig, dr1, dr2);
			}
			else if (chance_compare(co, power+RANDOM(20), get_target_resistance(cn, co)+RANDOM(20), usemana))
			{
				if (cn!=co) do_area_notify(cn, co, ch[cn].x, ch[cn].y, NT_SEEHIT, cn, co, power, 0);
				switch (intemp)
				{
					case SK_CURSE:
						spell_curse(cn, co, hitpower, 1);
						break;
					case SK_SLOW:
						spell_slow(cn, co, hitpower, 1);
						break;
					case SK_POISON:
						spell_poison(cn, co, hitpower, 1);
						break;
					case SK_WEAKEN:
						spell_weaken(cn, co, hitpower, 1);
						break;
					case SK_TAUNT:
						spell_taunt(cn, co, hitpower, 1);
						break;
					default:
						break;
				}
				hit++;
			}
			else
			{
				if (cn!=co) do_area_notify(cn, co, ch[cn].x, ch[cn].y, NT_SEEMISS, cn, co, power, 0);
				if (cn!=co && CAN_SENSE(co) && M_SK(co, SK_PERCEPT) > power + 5)
				{
					if (!(ch[co].flags & CF_SENSEOFF))
					{
						do_char_log(co, 0, 
						"%s%s\n", ch[cn].reference, splog[intemp].senseaoe);
					}
					if (!IS_IGNORING_SPELLS(co))
					{
						do_notify_char(co, NT_GOTMISS, cn, 0, 0, 0);
					}
				}
				else
				{
					if (!(ch[co].flags & CF_SYS_OFF))
						do_char_log(co, 0, "%s\n", splog[intemp].otheraoe);
				}
			}
			remember_pvp(cn, co);
		}
	}
	do_char_log(cn, 1, "%s\n", splog[intemp].selfaoe);
	if (intemp==SK_BLAST)
	{
		if (!(ch[cn].flags & CF_SYS_OFF))
		{
			do_char_log(cn, 1, 
				"You hit %d of %d creatures in range.\n", hit, count);
			do_char_log(cn, 1, 
				"You dealt an average of %d damage.\n", max(0, (avgdmg-1)/max(1,hit)) );
		}
	}
	else if (intemp!=SK_PLAGUE)
	{
		if (!(ch[cn].flags & CF_SYS_OFF))
			do_char_log(cn, 1, 
				"You affected %d of %d creatures in range.\n", hit, count);
	}
	
	return hit;
}

int spell_scorch(int cn, int co, int power, int flag);
int spell_aggravate(int cn, int co, int power);
int spell_bleed(int cn, int co, int power);

int surround_cast(int cn, int co_orig, int cc_orig, int intemp, int power, int dr1, int dr2)
{
	int m, n, mc, co, hitpower, tmp, tmpmp, hit=0, crit_dam=0, usemana = 1;
	int aggravate = 0, mjolnir = 0;
	
	m = ch[cn].x + ch[cn].y * MAPX;
	
	if (co_orig && co_orig!=cn)
	{
		if (intemp!=SK_LEAP) if (is_exhausted(cn)) return 0;
	}
	
	if (intemp==SK_MJOLNIR)
	{
		mjolnir = 1;
		intemp = SK_BLAST;
	}
	
	usemana = get_use_mana(intemp);
	
	for (n=0; n<4; n++)
	{
		switch (n)
		{
			case 0: mc = m + 1; break;
			case 1: mc = m - 1; break;
			case 2: mc = m + MAPX; break;
			case 3: mc = m - MAPX; break;
		}
		if ((co = map[mc].ch)!=0 && ch[co].attack_cn==cn && co_orig!=co && cc_orig!=co)
		{
			if (intemp==SK_BLAST)
			{
				spell_blast(cn, co, power, co_orig, 0);
				check_gloves(cn, co, co_orig, dr1, dr2);
				if (mjolnir) spell_shock(cn, co, power);
			}
			else if (intemp==SK_CLEAVE)
			{
				spell_cleave(cn, co, power, co_orig, dr1, dr2);
				continue; // skip damage_mshell
			}
			else if (intemp==SK_SHIELD)
			{
				spell_bash(cn, co, power, co_orig, dr1, dr2);
				continue; // skip damage_mshell
			}
			else if (intemp==SK_LEAP)
			{
				chlog(cn, "Used Leap on %s", ch[co].name);
				
				hitpower = spell_immunity(power, ch[co].to_parry) * 2;
				hitpower = hitpower/2 + hitpower/4;
				
				if (ch[co].a_hp >= ch[co].hp[5]*950) 
				{
					hitpower = hitpower * ch[cn].crit_multi / 100;
					if (!get_tarot(cn, IT_CH_JUSTIC_R)) spell_warcry(cn, co, hitpower/2, 1);
				}
				
				tmp = do_hurt(cn, co, hitpower, 8);
				
				if (tmp<1)
					do_char_log(cn, 0, "You cannot penetrate %s's armor.\n", ch[co].reference);
				else
				{
					if (!(ch[cn].flags & CF_SYS_OFF))
						do_char_log(cn, 1, "You sliced %s for %d HP.\n", ch[co].reference, tmp);
					if (!(ch[co].flags & CF_SYS_OFF))
						do_char_log(co, 1, "%s sliced you for %d HP.\n", ch[cn].name, tmp);
				}
				
				fx_add_effect(5, 0, ch[co].x, ch[co].y, 0);
				
				check_gloves(cn, co, co_orig, dr1, dr2);
				
				continue; // skip damage_mshell
			}
			else if (chance_compare(co, power+RANDOM(20), get_target_resistance(cn, co)+RANDOM(20), usemana)) 
			{
				switch (intemp)
				{
					case SK_CURSE: 
						spell_curse(cn, co, power, 0);
						break;
					case SK_SLOW: 
						spell_slow(cn, co, power, 0);
						break;
					case SK_POISON:
						spell_poison(cn, co, power, 0);
						break;
					case SK_WEAKEN:
						spell_weaken(cn, co, power, 0);
						break;
					case SK_TAUNT:
						spell_taunt(cn, co, power, 0);
						break;
					case SK_BLOODLET:
						spell_bleed(cn, co, power);
						break;
					default:
						break;
				}
			}
			damage_mshell(co, power);
			hit++;
		}
	}
	
	return hit;
}

int copy_buff(int bf)
{
	int in;
	
	in = god_copy_buff(bf);
	if (!in)
	{
		xlog("god_copy_buff failed");
		return 0;
	}
	
	bu[in].duration = max(bu[in].active, bu[in].duration * 90/100);
	bu[in].active   = bu[in].duration;
	
	return in;
}

int make_new_buff(int cn, int intemp, int sptemp, int power, int dur, int ext)
{
	int in;
	
	in = god_create_buff();
	if (!in)
	{
		xlog("god_create_buff failed for spell_%s", splog[intemp].ref);
		return 0;
	}
	
	strcpy(bu[in].name, splog[intemp].name);
	bu[in].flags 	|= IF_SPELL;
	bu[in].temp  	 = intemp;
	bu[in].sprite[1] = sptemp;
	bu[in].power 	 = power;
	bu[in].data[0]   = cn;		// data[0] = original caster
	bu[in].data[1]   = power;	// data[1] = power for values that decay, x on recall
	bu[in].data[2]   = 0;		// data[2] = y on recall
	bu[in].data[3]   = 0;		// data[3] = template from item
	bu[in].data[4]   = 0;		// data[4] = 0 is removed by NMZ, 1 is not
	bu[in].data[5]   = 0;		// data[5] = 1 is a debuff
	
	if (dur==-1)
	{
		bu[in].flags |= IF_PERMSPELL;
	}
	
	if (ext && !IS_PLAYER(cn))
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

void remove_buff(int cn, int bu_temp)
{
	int in, n;
	
	for (n = 0; n<MAXBUFFS; n++)
	{
		if ((in = ch[cn].spell[n])!=0)
		{
			if (bu[in].temp==bu_temp)
			{
				bu[in].used = USE_EMPTY;
				ch[cn].spell[n] = 0;
			}
		}
	}
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

int is_near(int cn, int co, int v)
{
	int nx, ny, ox, oy, dist;
	
	nx = ch[cn].x;
	ny = ch[cn].y;
	ox = ch[co].x;
	oy = ch[co].y;
	
	dist = sqrt(abs(nx - ox)*abs(nx - ox) + abs(ny - oy)*abs(ny - oy));
	
	if (dist <= v) return 1;
	
	return 0;
}

int spellcost(int cn, int cost, int in, int usemana)
{
	int cotfk_cost = 0, devil_cost = 0, hp_cost = 0;
	int mana_cost, end_cost, t, in2;
	
	if (IS_PLAYER(cn) && in != SK_BLAST && 
		in != SK_CLEAVE && in != SK_SHIELD && in != SK_WEAKEN && in != SK_WARCRY && 
		in != SK_BLIND && in != SK_DOUSE && in != SK_TAUNT && 
		in != SK_LEAP && in != SK_RAGE)
	{
		cost = max(SP_COST_BASE, min(cost, cost*M_SK(cn, in)/100));
	}
	if (in == SK_LEAP && (in2 = has_buff(cn, SK_EXHAUST)))
	{
		cost += bu[in2].active;
	}
	if (get_gear(cn, IT_TW_CROWN))
	{
		cost -= cost*25/100;
	}
	if (get_enchantment(cn, 22))
	{
		cost -= cost*20/100;
	}
	
	mana_cost = end_cost = cost;
	
	// Devil Tarot Card
	if (get_tarot(cn, IT_CH_DEVIL))
	{
		devil_cost=cost/3;
	}
	
	if (usemana>0)
	{
		// Crown of the First King
		if (get_gear(cn, IT_TW_CROWN))
		{
			cotfk_cost = cost*25/100;
		}
		
		mana_cost-=cotfk_cost;
		mana_cost-=devil_cost;
		hp_cost+=devil_cost;
		
		mana_cost = mana_cost * ch[cn].mana_cost / 10000;
		
		if (hp_cost*1000 > ch[cn].a_hp)
		{
			do_char_log(cn, 0, "You don't have enough life.\n");
			if (!IS_PLAYER(cn)) add_exhaust(cn, TICKS * 4); // to keep NPCs from spam-failing
			return -1;
		}
		if (cotfk_cost*1000 > ch[cn].a_end)
		{
			do_char_log(cn, 0, "You don't have enough endurance.\n");
			if (!IS_PLAYER(cn)) add_exhaust(cn, TICKS * 4); // to keep NPCs from spam-failing
			return -1;
		}
		if (mana_cost*1000 > ch[cn].a_mana)
		{
			do_char_log(cn, 0, "You don't have enough mana.\n");
			if (!IS_PLAYER(cn)) add_exhaust(cn, TICKS * 4); // to keep NPCs from spam-failing
			return -1;
		}
	}
	if (usemana==0 || usemana==2)
	{
		// Crown of the First King
		if (get_gear(cn, IT_TW_CROWN))
		{
			cotfk_cost = cost*25/100;
		}
		
		end_cost-=cotfk_cost;
		end_cost-=devil_cost;
		
		if (has_spell_from_item(cn, BUF_IT_AALE))
		{
			end_cost = end_cost * 85 / 100;
		}
		
		if ((hp_cost+devil_cost*2)*1000 > ch[cn].a_hp)
		{
			do_char_log(cn, 0, "You don't have enough life.\n");
			if (!IS_PLAYER(cn)) add_exhaust(cn, TICKS * 4); // to keep NPCs from spam-failing
			return -1;
		}
		if ((end_cost+cotfk_cost)*1000 > ch[cn].a_end)
		{
			do_char_log(cn, 0, "You're too exhausted for that right now!\n");
			if (!IS_PLAYER(cn)) add_exhaust(cn, TICKS * 4); // to keep NPCs from spam-failing
			return -1;
		}
		if (cotfk_cost*1000 > ch[cn].a_mana)
		{
			do_char_log(cn, 0, "You don't have enough mana.\n");
			if (!IS_PLAYER(cn)) add_exhaust(cn, TICKS * 4); // to keep NPCs from spam-failing
			return -1;
		}
	}
	if (usemana>0)
	{
		ch[cn].a_mana -= mana_cost*1000;
		ch[cn].a_end  -= cotfk_cost*1000;
		ch[cn].a_hp   -= hp_cost*1000;
		
		if (get_gear(cn, IT_WP_LIFESPRIG) && mana_cost)
			spell_pomesol(cn, cn, mana_cost, 1);
		if (get_gear(cn, IT_WP_CRESSUN) && cotfk_cost)
			spell_pomesol(cn, cn, cotfk_cost, 0);
	}
	if (usemana==0 || usemana==2)
	{
		ch[cn].a_end  -= end_cost*1000;
		ch[cn].a_mana -= cotfk_cost*1000;
		ch[cn].a_hp   -= (hp_cost+devil_cost*2)*1000;
		
		if (get_gear(cn, IT_WP_LIFESPRIG) && cotfk_cost)
			spell_pomesol(cn, cn, cotfk_cost, 1);
		if (get_gear(cn, IT_WP_CRESSUN) && end_cost)
			spell_pomesol(cn, cn, end_cost, 0);
	}
	return 0;
}

int chance_compare(int co, int offense, int defense, int usemana)
{
	if (offense > defense) return 1;
	return 0;
}

int chance_base(int cn, int co, int skill, int d20, int defense, int usemana, int msg)
{
	int chance, roll, tmp, power;
	
	if (usemana>100)
		power = usemana-100;
	else
		power = M_SK(cn, skill);
	
	if (!IS_PLAYER(co) && !usemana)
	{
		d20 += 25;
	}
	
	chance = d20 * power / max(1, defense*10);

	if (IS_PLAYER(cn))
	{
		chance += 1;
		if (ch[cn].luck<0)
		{
			chance += ch[cn].luck / 500 - 1;
		}
	}
	
	// Lower result is better.
	roll = RANDOM(20);
	
	if (usemana)
	{
		// Book - Castor's Advantage
		if (get_book(cn, IT_BOOK_ADVA))
		{
			tmp = RANDOM(20);
			if (tmp < roll) roll = tmp;
		}
		
		// Book - Shiva's Malice
		if (get_book(cn, IT_BOOK_SHIV))
		{
			tmp = RANDOM(20);
			if (tmp > roll) roll = tmp;
		}
	}

	if (chance< 0) { chance =  0; }
	if (chance>18) { chance = 18; }
	
	ch[cn].spellfail = 0;
	ch[co].spellfail = 0;
	
	if (roll > chance || defense >= power + (power / 2))
	{
		if (!IS_PLAYER(cn))
		{
			// 4 second exhaust for NPCS to keep them from spam-failing
			add_exhaust(cn, TICKS * 4);
		}
		if (usemana)
		{
			if (IS_PLAYER(cn) && !IS_PLAYER(co) && skill!=SK_IDENT) // && !(ch[cn].flags & CF_OVERRIDE)
			{
				ch[cn].spellfail = 2;
				ch[co].spellfail = 1;
				if (msg) 
					do_char_log(cn, 5, "Your spell was suppressed by your target.\n");
				return 0;
			}
			else if (msg) 
			{
				do_char_log(cn, 0, "Your spell fizzled!\n");
			}
		}
		else if (msg)
		{
			do_char_log(cn, 0, "Your target resisted your attempt to %s them.\n", splog[skill].ref);
		}
		return -1;
	}
	if (IS_PLAYER(co) && skill!=SK_IDENT && usemana && defense >= power)
	{
		ch[cn].spellfail = 2;
		ch[co].spellfail = 1;
		if (msg) 
			do_char_log(cn, 5, "Your spell was suppressed by your target.\n");
	}
	return 0;
}

int chance(int cn, int d20)
{
	if (get_gear(cn, IT_TW_CROWN) || get_book(cn, IT_BOOK_MALT) || get_enchantment(cn, 15))
		return 0;
	
	if (IS_PLAYER(cn))
	{
		// Bad luck gives worse dice
		if (ch[cn].luck<0) { d20 += ch[cn].luck / 500 - 1; }
	}

	if (d20<0)  { d20 =  0; }
	if (d20>18) { d20 = 18; }
	
	if (RANDOM(20)>d20)
	{
		do_char_log(cn, 0, "You lost your focus!\n");
		return -1;
	}
	return 0;
}

void damage_mshell(int co, int dam)
{
	int tmp = 0, n, in;
	
	//chlog(co,"Magic Shell damaged.");
	
	for (n = 0; n<MAXBUFFS; n++)
	{
		if ((in = ch[co].spell[n])!=0)
		{
			if (bu[in].temp==SK_MSHELL)
			{
				if (IS_SEYA_OR_BRAV(co))
					tmp = bu[in].active / 768 + 1;
				else
					tmp = bu[in].active / 512 + 1;
				tmp = (dam + tmp - get_target_resistance(0, co)) * 5;

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
						if (IS_SEYA_OR_BRAV(co))
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
	
	if (IS_SANEITEM(in = ch[co].worn[WN_CLOAK]) && it[in].driver == 60 && RANDOM(3))
		item_damage_worn(co, WN_CLOAK, dam);
}

int get_target_resistance(int cn, int co)
{
	int target_resist = 1, in;
	
	// Tarot Card - Hanged Man :: Resistance behaves as extra Immunity instead
	if (get_tarot(co, IT_CH_HANGED))
		target_resist = (M_SK(co, SK_RESIST)*2/3);
	else
		target_resist = M_SK(co, SK_RESIST);
	
	if (IS_SANECHAR(cn))
	{
		// Tarot - Chariot.R : 25% res/imm penetration
		if (get_tarot(cn, IT_CH_CHARIO_R))
			target_resist = target_resist * 4/5;
		
		// Lethargy - penetrate
		if (in = has_buff(cn, SK_LETHARGY))
		{
			if (bu[in].data[2])
				target_resist -= bu[in].power/4;
			else
				target_resist -= bu[in].power/3;
		}
		
		// Tree - seya
		if (T_SEYA_SK(cn, 6)) target_resist = target_resist*4/5;
	}
	
	return max(1, target_resist);
}

int get_target_immunity(int cn, int co)
{
	int target_immune = 1, in;
	
	// Tarot Card - Hanged Man :: Resistance behaves as extra Immunity instead
	if (get_tarot(co, IT_CH_HANGED))
		target_immune = M_SK(co, SK_IMMUN) + (M_SK(co, SK_RESIST)/3);
	else
		target_immune = M_SK(co, SK_IMMUN);
	
	if (IS_SANECHAR(cn))
	{
		// Tarot - Chariot.R : 25% res/imm penetration
		if (get_tarot(cn, IT_CH_CHARIO_R))
			target_immune = target_immune * 4/5;
		
		// Lethargy - penetrate
		if (in = has_buff(cn, SK_LETHARGY))
		{
			if (bu[in].data[2])
				target_immune -= bu[in].power/4;
			else
				target_immune -= bu[in].power/3;
		}
		
		// Tree - lycan
		if (T_LYCA_SK(cn, 10)) target_immune = target_immune*4/5;
	}
	
	return target_immune;
}

int get_target_metabolism(int co)
{
	int target_metabolism = 0;
	
	if (B_SK(co, SK_METABOLISM))
		target_metabolism = M_SK(co, SK_METABOLISM);
	
	return target_metabolism;
}

int spell_metabolism(int power, int metabolism)
{
	metabolism = metabolism/2;
	
	return max(1, (power - metabolism) * (200 - metabolism) / 200);
}

int spell_immunity(int power, int immun)
{
	immun = immun/2;
	
	if (power<=immun) 		return 1;
	else					return (power - immun);
}

int spell_race_mod(int power, int cn)
{
	int kindred, mod;
	
	kindred = ch[cn].kindred;
	
	// Tarot - Star.R : Spell modifier is at least 1.00
	     if 	(get_tarot(cn, IT_CH_STAR_R))	{ mod =  90; }
	else if 	(kindred & KIN_TEMPLAR)			{ mod =  75; }
	else if 	(kindred & KIN_MERCENARY)		{ mod = 100; }
	else if 	(kindred & KIN_HARAKIM)			{ mod = 105; }

	else if 	(kindred & KIN_SEYAN_DU)		{ mod =  95; }
	else if 	(kindred & KIN_ARCHTEMPLAR)		{ mod =  80; }
	else if 	(kindred & KIN_SKALD)			{ mod =  80; }
	else if 	(kindred & KIN_WARRIOR)			{ mod = 105; }
	else if 	(kindred & KIN_SORCERER)		{ mod = 105; }
	else if 	(kindred & KIN_SUMMONER)		{ mod = 110; }
	else if 	(kindred & KIN_ARCHHARAKIM)		{ mod = 110; }
	else if 	(kindred & KIN_BRAVER)			{ mod = 115; }
	else if 	(kindred & KIN_LYCANTH)			{ mod = 100; }
	else if	(ch[cn].temp == CT_CASTERCOMP 
		|| ch[cn].temp == CT_ARCHCASTER)		{ mod = 120; }
	else										{ mod = 100; }
	
	if (kindred & KIN_MONSTER)
	{
		if (IS_GLOB_MAYHEM)
			mod += (getrank(cn)-4);
		else
			mod += (getrank(cn)-4)/2;
	}
	
	if (globs->newmoon)		{ mod += 10; }
	if (globs->fullmoon)	{ mod += 15; }
	
	return (power * mod) / 100;
}

int spell_multiplier(int power, int cn)
{
	// Tarot - Star.R : No spellmod effect on spells.
	if (get_tarot(cn, IT_CH_STAR_R))
		return power;
	
	return (power * ch[cn].spell_mod / 100);
}

int skill_multiplier(int power, int cn)
{
	// Tarot - Star.R : Spellmod now effects skills.
	if (get_tarot(cn, IT_CH_STAR_R))
		return (power * ch[cn].spell_mod / 100);
	
	return power;
}

int add_spell(int cn, int in)
{
	int n, in2, weak = 999, weakest = 99, tmp;
	int m, stack, tickminimum = TICKS*60;

	m = ch[cn].x + ch[cn].y * MAPX;
	if (map[m].flags & CF_NOMAGIC) { return 0; }
	
	tmp = bu[in].temp;
	
	for (n = 0; n<MAXBUFFS; n++)
	{
		if ((in2 = ch[cn].spell[n])!=0)
		{
			// Immunize/Inoculate prevents up to three ailments
			if ((bu[in2].temp==SK_DISPEL || bu[in2].temp==SK_DISPEL2) &&
				(tmp==bu[in2].data[1] || tmp==bu[in2].data[2] || tmp==bu[in2].data[3]))
			{
				chlog(cn,"Immunize true (%d)",bu[in2].temp);
				if (bu[in2].temp==SK_DISPEL2) do_char_log(cn, 0, "The magic didn't work!\n");
				return 0;
			}
		}
	}
	
	// Acedia
	if (IS_SANECHAR(bu[in].data[0]))
	{
		if (it[ch[bu[in].data[0]].worn[WN_RHAND]].temp==IT_TW_ACEDIA) // less
			bu[in].duration = bu[in].active = bu[in].duration * 3/4;
		if (it[ch[bu[in].data[0]].worn[WN_LHAND]].temp==IT_TW_ACEDIA) // more
			bu[in].duration = bu[in].active = bu[in].duration * 6/4;
	}
	
	// overwrite spells if same spell is cast twice and the new spell is more powerful
	for (n = 0; n<MAXBUFFS; n++)
	{
		if ((in2 = ch[cn].spell[n])!=0)
		{
			if (bu[in2].used==USE_EMPTY) continue;
			if (bu[in].temp==SK_HEAL && bu[in2].temp==SK_HEAL)
			{
				// Multiple heals stack 'healing sickness' (SK_HEAL), reducing heal power
				stack=3;

				if (get_book(cn, IT_BOOK_HOLY)) // Book: Holy Etiquette
					stack--;
				if (has_spell_from_item(cn, BUF_IT_DRAG)) // Dragon's Breath
					stack--;
				if (get_tarot(cn, IT_CH_TEMPER_R)) // Tarot - Temperance.R : +1 stack max
					stack++;
					
				if (!(IS_PLAYER(cn) || IS_PLAYER_COMP(cn))) stack = 4;
				
				bu[in].data[1] = bu[in2].data[1] + 1;
				
				if (bu[in].data[1] > stack)
					bu[in].data[1] = max(stack, bu[in2].data[1]);
				
				bu[in].sprite[1] = min(6720+stack, 6721+bu[in].data[1]);
				
				bu[in2].used = USE_EMPTY;
				break;
			}
			else if (bu[in].temp==SK_VENOM && bu[in2].temp==SK_VENOM)
			{
				bu[in].data[2] = max(bu[in].data[2], bu[in2].data[2]);
				bu[in].data[1] = min(bu[in].data[2], bu[in].data[1] + bu[in2].data[1]);
				bu[in].data[7] = max(bu[in].data[7], bu[in2].data[7]);
				bu[in].data[6] = min(bu[in].data[7], bu[in].data[6] + bu[in2].data[6]);
				bu[in].skill[SK_IMMUN][1] = -(bu[in].data[6]);
				bu[in].value = min(3, bu[in2].value + 1);
				bu[in].sprite[1] = min(6733, 6731+bu[in].value-1);
				
				bu[in2].used = USE_EMPTY;
				break;
			}
			else if (bu[in].temp==SK_SHOCK && bu[in2].temp==SK_SHOCK)
			{
				bu[in].value = min(5, bu[in2].value + 1);
				bu[in].dmg_reduction[1] = -(bu[in].value*4);
				bu[in].dmg_bonus[1]     = -(bu[in].value*4);
				bu[in].sprite[1] = min(6745, 6741+bu[in].value-1);
				
				bu[in2].used = USE_EMPTY;
				break;
			}
			else if (bu[in].temp==SK_CHARGE && bu[in2].temp==SK_CHARGE)
			{
				bu[in].value = min(5, bu[in2].value + 1);
				bu[in].dmg_reduction[1] = bu[in].value*4;
				bu[in].dmg_bonus[1]     = bu[in].value*4;
				bu[in].sprite[1] = min(6750, 6746+bu[in].value-1);
				
				bu[in2].used = USE_EMPTY;
				break;
			}
			else if (bu[in].temp==SK_ZEPHYR2 && bu[in2].temp==SK_ZEPHYR2)
			{
				// Multiple zephyrs store their expiry time so each can occur
				bu[in].data[2] = bu[in2].data[1];
				bu[in].data[1] = bu[in2].active;
				bu[in].value = min(3, bu[in2].value + 1);
				bu[in].sprite[1] = max(6726, min(6728, 6726+bu[in].value-1));
				
				bu[in2].used = USE_EMPTY;
				break;
			}
			else if (bu[in].temp==SK_IMMOLATE2 && bu[in2].temp==SK_IMMOLATE2)
			{
				bu[in2].data[1] = max(bu[in].data[1], bu[in2].data[1]);
				bu[in2].duration = bu[in2].active = bu[in].duration;
				
				bu[in].used = USE_EMPTY;
				return 0;
			}
			else if (bu[in].temp==SK_ARIA && bu[in2].temp==SK_ARIA)
			{
				bu[in2].cool_bonus[1] = max(bu[in].cool_bonus[1], bu[in2].cool_bonus[1]);
				if (IS_SKALD(cn)) bu[in2].dmg_bonus[1] = max(bu[in].dmg_bonus[1], bu[in2].dmg_bonus[1]);
				bu[in2].duration = bu[in2].active = bu[in].duration;
				
				bu[in].used = USE_EMPTY;
				return 0;
			}
			else if (bu[in].temp==SK_ARIA2 && bu[in2].temp==SK_ARIA2)
			{
				bu[in2].cool_bonus[1] = min(bu[in].cool_bonus[1], bu[in2].cool_bonus[1]);
				bu[in2].duration = bu[in2].active = bu[in].duration;
				
				bu[in].used = USE_EMPTY;
				return 0;
			}
			else if (bu[in2].temp==bu[in].temp)
			{
				if (bu[in2].temp==SK_SLOW || bu[in2].temp==SK_SLOW2 || bu[in2].temp==SK_CURSE2)
					tickminimum = TICKS*5;
				
				if ((ch[cn].flags & CF_OVERRIDE) && bu[in].data[0]==cn &&
					(bu[in].temp==SK_BLESS   || bu[in].temp==SK_PROTECT || bu[in].temp==SK_ENHANCE ||
					 bu[in].temp==SK_MSHIELD || bu[in].temp==SK_HASTE   || bu[in].temp==SK_ZEPHYR  ||
					 bu[in].temp==SK_PULSE   || bu[in].temp==SK_WARCRY3))
				{
					bu[in2].used = USE_EMPTY;
					ch[cn].spell[n] = 0;
					break;
				}
				
				if (IS_PLAYER_GC(cn) && (ch[CN_OWNER(cn)].flags & CF_OVERRIDE) && bu[in].data[0]==cn &&
					(bu[in].temp==SK_BLESS   || bu[in].temp==SK_PROTECT || bu[in].temp==SK_ENHANCE ||
					 bu[in].temp==SK_MSHIELD || bu[in].temp==SK_HASTE   || bu[in].temp==SK_ZEPHYR  ||
					 bu[in].temp==SK_PULSE   || bu[in].temp==SK_WARCRY3))
				{
					bu[in2].used = USE_EMPTY;
					ch[cn].spell[n] = 0;
					break;
				}
				
				if (bu[in2].temp==SK_LIGHT && bu[in2].active>(bu[in2].duration-TICKS*5) && bu[in].data[0]==cn)
				{
					bu[in].used = USE_EMPTY;
					bu[in2].used = USE_EMPTY;
					ch[cn].spell[n] = 0;
					return -1;
				}
				
				if (bu[in].power<bu[in2].power && bu[in2].active>tickminimum && bu[in2].temp!=SK_LIGHT)
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
	
	if (bu[in].data[5] == 1 && bu[in].temp != SK_SHADOW && bu[in].temp != SK_HEAL && !has_buff(cn, SK_PLAGUE) &&
		IS_SANECHAR(n = bu[in].data[0]) && IS_LYCANTH(n)) spell_plague(n, cn, 2);

	return 1;
}

int add_exhaust(int cn, int len)
{
	int n, in2, in = 0, baselen = 100;
	
	// Book: Damor's Grudge
	if (get_book(cn, IT_BOOK_DAMO))
	{
		baselen = 90;
	}
	
	len = len * baselen / max(25, ch[cn].cool_bonus);
	
	// Acedia
	if (it[ch[cn].worn[WN_RHAND]].temp==IT_TW_ACEDIA) len = len * 3/4; // less
	if (it[ch[cn].worn[WN_LHAND]].temp==IT_TW_ACEDIA) len = len * 6/4; // more
	
	if (ch[cn].spellfail==2)
	{
		len = len/2;
		ch[cn].spellfail = 0;
		if (T_ARHR_SK(cn, 6)) return 0;
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
	bu[in].temp  	 = SK_EXHAUST;
	bu[in].power 	 = 300;
	
	// Remove healing potion effects
	for (n = 0; n<MAXBUFFS; n++)
	{
		if ((in2 = ch[cn].spell[n])!=0)
		{
			if (bu[in2].temp==102)
			{
				bu[in2].used = USE_EMPTY;
				ch[cn].spell[n] = 0;
			}
		}
	}
	//
	
	add_spell(cn, in);
	
	return in;
}

int spell_from_item(int cn, int in2)
{
	int in, n, thousand = 1000;
	
	// 102 are healing potions, 103 are drinks/food
	if ((ch[cn].flags & CF_NOMAGIC) && !(it[in2].data[1]>=102 && it[in2].data[1]<=110))
	{
		do_char_log(cn, 0, "The magic didn't work! There must be external influences.\n");
		return 0;
	}

	in = god_create_buff();
	if (!in)
	{
		xlog("god_create_buff failed in skill_from_item");
		return 0;
	}
	
	if (get_book(cn, IT_BOOK_ALCH)) // Book: Alchemy 101
	{
		thousand = thousand * 3/2;
	}
	
	if (get_enchantment(cn, 2))
	{
		thousand = thousand * 5/4;
	}
	
	strcpy(bu[in].name, it[in2].name);
	bu[in].flags |= IF_SPELL;
	bu[in].data[0] = cn; // Set the owner
	
	bu[in].armor[1]			= it[in2].armor[1];
	bu[in].weapon[1]		= it[in2].weapon[1];

	bu[in].hp[1]			= it[in2].hp[1];
	bu[in].end[1]			= it[in2].end[1];
	bu[in].mana[1]			= it[in2].mana[1];
	bu[in].sprite_override	= it[in2].sprite_override;

	for (n = 0; n<5; n++) 			{ bu[in].attrib[n][1] = it[in2].attrib[n][1]; }
	for (n = 0; n<MAXSKILL; n++) 	{ bu[in].skill[n][1]  = it[in2].skill[n][1];  }
	
	bu[in].gethit_dam[1] 	= it[in2].gethit_dam[1];
	bu[in].speed[1] 		= it[in2].speed[1];
	bu[in].move_speed[1] 	= it[in2].move_speed[1];
	bu[in].atk_speed[1] 	= it[in2].atk_speed[1];
	bu[in].cast_speed[1] 	= it[in2].cast_speed[1];
	bu[in].spell_mod[1] 	= it[in2].spell_mod[1];
	bu[in].spell_apt[1] 	= it[in2].spell_apt[1];
	bu[in].cool_bonus[1] 	= it[in2].cool_bonus[1];
	bu[in].crit_chance[1] 	= it[in2].crit_chance[1];
	bu[in].crit_multi[1] 	= it[in2].crit_multi[1];
	bu[in].to_hit[1] 		= it[in2].to_hit[1];
	bu[in].to_parry[1] 		= it[in2].to_parry[1];
	bu[in].top_damage[1] 	= it[in2].top_damage[1];
	bu[in].dmg_bonus[1] 	= it[in2].dmg_bonus[1];
	bu[in].dmg_reduction[1] = it[in2].dmg_reduction[1];
	
	// it[in2].data[0] = display sprite number
	if (it[in2].data[0]) 	{ bu[in].sprite[1] = it[in2].data[0]; }
	else 					{ bu[in].sprite[1] = BUF_SPR_GENERIC; }
	
	bu[in].duration 		= bu[in].active = it[in2].duration;
	
	// Special case for the Rainbow Potion
	// CGI Rainbow Potion has a duration of 72, here we give a random ++(0-108)
	if (it[in2].temp==IT_POT_RAIN)
	{
		bu[in].duration 	+= RANDOM(109);
	}
	
	// it[in2].data[1] = template number for overwriting
	if (it[in2].data[1]) 	{ bu[in].temp = it[in2].data[1]; }
	else 					{ bu[in].temp = 101; }
	
	// it[in2].data[2] = HP regen over the given duration
	if (it[in2].data[2])
	{
		bu[in].hp[0]		= it[in2].data[2] * thousand / it[in2].duration;
	}
	// it[in2].data[3] = EN regen over the given duration
	if (it[in2].data[3])
	{
		bu[in].end[0]		= it[in2].data[3] * thousand / it[in2].duration;
	}
	// it[in2].data[4] = MP regen over the given duration
	if (it[in2].data[4])
	{
		bu[in].mana[0]		= it[in2].data[4] * thousand / it[in2].duration;
	}
	
	// Special case for the Rainbow Potion
	// CGI Rainbow Potion heals 40, here we give a random ++(0-80)
	if (it[in2].temp==IT_POT_RAIN)
	{
		bu[in].hp[0]		+= RANDOM(81) * thousand / it[in2].duration;
		bu[in].end[0]		+= RANDOM(81) * thousand / it[in2].duration;
		bu[in].mana[0]		+= RANDOM(81) * thousand / it[in2].duration;
	}
	
	bu[in].power 			= it[in2].power;
	
	bu[in].data[3]			= it[in2].temp;
	
	if (it[in2].data[1]>=102 && it[in2].data[1]<=110)
	{
		bu[in].data[4] = 1; // Effects not removed by NMZ (102 - 110)
	}
	
	if (!add_spell(cn, in))
	{
		do_char_log(cn, 1, "Magical interference neutralized the %s's effect.\n", bu[in].name);
		return 1;
	}
	
	// Special case for Antidote to dispel Poison and Venom
	if (it[in2].temp==IT_POT_ANTI)
	{
		remove_buff(cn, SK_POISON);
		remove_buff(cn, SK_VENOM);
		bu[in].temp      = SK_DISPEL;
		bu[in].sprite[1] = BUF_SPR_IMMUNI;
		bu[in].data[1]   = SK_POISON;
		bu[in].data[2]   = SK_VENOM;
	}
	
	do_char_log(cn, 1, "You feel changed.\n");

	char_play_sound(cn, ch[cn].sound + 1, -150, 0);
	
	return 1;
}

int has_spell(int cn, int temp)
{
	int n, in;
	for (n = 0; n<MAXBUFFS; n++)
	{
		if ((in = ch[cn].spell[n]) && bu[in].temp==temp)
		{
			return in;
		}
	}
	return 0;
}

int has_spell_from_item(int cn, int temp)
{
	int n, in;
	for (n = 0; n<MAXBUFFS; n++)
	{
		if ((in = ch[cn].spell[n]) && bu[in].temp > 100 && bu[in].data[3]==temp)
		{
			return in;
		}
	}
	return 0;
}

// debuff: 1 for normal debuff, 2 to remove interference messages
int cast_a_spell(int cn, int co, int in, int debuff, int msg)
{
	int temp, arealog = 0, n, in2, li, font=1;
	char buf[50];
	
	strcpy(buf, ch[cn].reference); buf[0] = toupper(buf[0]);
	temp = bu[in].temp;
	
	if (temp==SK_LIGHT)
	{
		arealog = 1;
	}
	
	if (debuff>0)
	{
		if (ch[co].spellfail==1)
		{
			font = 4;
			ch[co].spellfail = 0;
		}
		else
		{
			font = 0;
		}
	}
	
	// Check for immunize and inoculate from dispel to see if we can grant the spell
	for (n = 0; n<MAXBUFFS; n++)
	{
		if ((in2 = ch[co].spell[n])!=0)
		{
			// Immunize/Inoculate prevents up to three ailments
			if ((bu[in2].temp==SK_DISPEL || bu[in2].temp==SK_DISPEL2) &&
				(temp==bu[in2].data[1] || temp==bu[in2].data[2] || temp==bu[in2].data[3]))
			{
				if (msg>0) // don't log AoE spells or 'secondary' debuffs
					do_char_log(cn, 1, "%s neutralized the %s's effect.\n", bu[in2].name, splog[temp].ref);
				return 0;
			}
		}
	}
	
	if (cn!=co)
	{
		if (!add_spell(co, in))
		{
			if (debuff<2 && (in==SK_WEAKEN || in==SK_WEAKEN2))
			{
				do_char_log(cn, 1, 
				"Your opponent is already weakened!\n");
			}
			else if (debuff<2)
			{
				do_char_log(cn, 1, 
				"Magical interference neutralized the %s's effect.\n", splog[temp].ref);
			}
			return 0;
		}
		if (CAN_SENSE(co) && M_SK(co, SK_PERCEPT) + 10 > bu[in].power)
		{
			do_char_log(co, font, "%s%s\n", buf, splog[temp].sense);
		}
		else
		{
			do_char_log(co, font, "%s\n", splog[temp].self);
		}
		if (arealog) 
		{
			do_area_log(co, 0, ch[co].x, ch[co].y, 2, "%s%s\n", ch[co].name, splog[temp].other);
		}
		else if (debuff<2)
		{
			do_char_log(cn, 1, "%s%s\n", ch[co].name, splog[temp].other);
		}
		if (debuff)
		{
			if (temp==SK_TAUNT)
			{
				do_area_notify(cn, co, ch[cn].x, ch[cn].y, NT_SEEMISS, cn, co, 0, 0);
				do_notify_char(co, NT_GOTMISS, cn, 0, 0, 0);
			}
			else
			{
				do_area_notify(cn, co, ch[cn].x, ch[cn].y, NT_SEEHIT, cn, co, 0, 0);
				if (!IS_IGNORING_SPELLS(co))
				{
					do_notify_char(co, NT_GOTHIT, cn, 0, 0, 0);
				}
				do_notify_char(cn, NT_DIDHIT, co, 0, 0, 0);
			}
		}
		chlog(cn, "Cast %s on %s", splog[temp].ref, ch[co].name);
		if (debuff<3)
		{
			char_play_sound(cn, ch[cn].sound + 1, -150, 0);
		}
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
		if (!(li = add_spell(cn, in)))
		{
			do_char_log(cn, 1, 
			"Magical interference neutralized the %s's effect.\n", bu[in].name);
			return 0;
		}
		if (temp!=SK_RECALL && temp!=SK_IDENT)
		{
			if (li<0)
			{
				if (temp==SK_LIGHT) do_char_log(cn, 1, "You stop emitting light.\n");
				if (temp==SK_RAGE)  do_char_log(cn, 1, "Rage no longer active.\n");
				if (temp==SK_CALM)  do_char_log(cn, 1, "Calm no longer active.\n");
			}
			else
				do_char_log(cn, 1, "%s\n", splog[temp].self);
			char_play_sound(cn, ch[cn].sound + 1, -150, 0);
			fx_add_effect(6, 0, ch[cn].x, ch[cn].y, 0);
		}
		if (IS_PLAYER(cn))
		{
			chlog(cn, "Cast %s", bu[in].name);
		}
	}
	if (debuff<2)
	{
		fx_add_effect(7, 0, ch[cn].x, ch[cn].y, 0);
	}
	
	return 1;
}

int spellpower_check(int cn, int co, int power, int fromscroll)
{
	int tmp;
	
	if (IS_PLAYER(co) || IS_COMP_TEMP(co))
	{
		if (cn!=co) 
		{
			if (IS_COMP_TEMP(co)) 
				tmp = ch[co].spell_apt * 2 + M_AT(cn, AT_WIL)/4;
			else 
				tmp = ch[co].spell_apt + M_AT(cn, AT_WIL)/4;
			
			tmp = max(tmp, 1);
			
			if (power>tmp)
			{
				power = tmp;
				do_char_log(cn, 1, "Seeing that %s's mind cannot support the power of your spell, you reduced its strength.\n", ch[co].reference);
			}
		}
		else if (fromscroll)
		{
			tmp = ch[co].spell_apt;
			if (power>tmp)
			{
				power = tmp;
				do_char_log(cn, 1, "Your mind is not powerful enough to use the full strength of this spell.\n");
			}
		}
		else
		{
			tmp = ch[co].spell_apt * 2;
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
	int in, dur = SP_DUR_LIGHT;
	
	power = spell_multiplier(power, cn);
	
	if (has_buff(cn, 215)) dur = TICKS*5;
	
	if (!(in = make_new_buff(cn, SK_LIGHT, BUF_SPR_LIGHT, power, dur, 0))) 
		return 0;
	
	power = min(250, 25 + power / 12 * 9);
	bu[in].light[1]  = power;
	
	return cast_a_spell(cn, co, in, 0, 1); // SK_LIGHT
}
void skill_light(int cn)
{
	int co, power;
	
	power = M_SK(cn, SK_LIGHT);
	
	if (!(co = get_target(cn, 1, 1, 0, SP_COST_LIGHT, SK_LIGHT, 1, power, 0))) 
		return;
	
	spell_light(cn, co, power);

	add_exhaust(cn, SK_EXH_LIGHT);
}

int spell_enhance(int cn, int co, int power, int fromscroll)
{
	int in;
	
	power = spellpower_check(cn, co, spell_multiplier(power, cn), fromscroll);
	
	if (!(in = make_new_buff(cn, SK_ENHANCE, BUF_SPR_ENHANCE, power, SP_DUR_ENHANCE, 1))) 
		return 0;
	
	if (IS_SEYA_OR_BRAV(co))
		bu[in].weapon[1] = min(127, power / 6 + 3);
	else
		bu[in].weapon[1] = min(127, power / 4 + 4);
	
	return cast_a_spell(cn, co, in, 0, 1); // SK_ENHANCE
}
void skill_enhance(int cn, int flag)
{
	int co, power;
	
	power = M_SK(cn, SK_ENHANCE);
	
	if (!(co = get_target(cn, 0, 1, 1, flag?0:SP_COST_ENHANCE, SK_ENHANCE, 1, power, flag?-1:0))) 
		return;
	
	spell_enhance(cn, co, power, 0);
	
	// Book - Bishop :: add exhaust together
	if (flag)
	{
		add_exhaust(cn, SK_EXH_BLESS+SK_EXH_PROTECT+SK_EXH_ENHANCE);
	}
	else
	{
		add_exhaust(cn, SK_EXH_ENHANCE); // Half-second
	}
}

int spell_protect(int cn, int co, int power, int fromscroll)
{
	int in;
	
	power = spellpower_check(cn, co, spell_multiplier(power, cn), fromscroll);

	if (!(in = make_new_buff(cn, SK_PROTECT, BUF_SPR_PROTECT, power, SP_DUR_PROTECT, 1))) 
		return 0;

	if (IS_SEYA_OR_BRAV(co))
		bu[in].armor[1] = min(127, power / 6 + 3);
	else
		bu[in].armor[1] = min(127, power / 4 + 4);

	return cast_a_spell(cn, co, in, 0, 1); // SK_PROTECT
}
void skill_protect(int cn, int flag)
{
	int co, power;
	
	power = M_SK(cn, SK_PROTECT);
	
	if (!(co = get_target(cn, 0, 1, 1, flag?0:SP_COST_PROTECT, SK_PROTECT, 1, power, flag?-1:0))) 
		return;

	spell_protect(cn, co, power, 0);
	
	// Book - Bishop :: cast enhance after if flag says so
	if (flag && B_SK(cn, SK_ENHANCE))
	{
		skill_enhance(cn, 1);
	}
	else if (flag)
	{
		add_exhaust(cn, SK_EXH_BLESS+SK_EXH_PROTECT);
	}
	else
	{
		add_exhaust(cn, SK_EXH_PROTECT);
	}
}

int spell_bless(int cn, int co, int power, int fromscroll)
{
	int in, n;
	
	power = spellpower_check(cn, co, spell_multiplier(power, cn), fromscroll);
	
	if (!(in = make_new_buff(cn, SK_BLESS, BUF_SPR_BLESS, power, SP_DUR_BLESS, 1))) 
		return 0;
	
	for (n = 0; n<5; n++) 
		bu[in].attrib[n][1] = min(127, ((power*2/3)-n) / 5 + 3);
	
	return cast_a_spell(cn, co, in, 0, 1); // SK_BLESS
}
void skill_bless(int cn)
{
	int co, power, flag = 0;
	
	if (get_book(cn, IT_BOOK_BISH)) flag = 1;
	
	power = M_SK(cn, SK_BLESS);
	
	if (!(co = get_target(cn, 0, 1, 1, flag?(SP_COST_BLESS+SP_COST_PROTECT+SP_COST_ENHANCE):SP_COST_BLESS, SK_BLESS, 1, power, 0))) 
		return;
	
	spell_bless(cn, co, power, 0);
	
	// Book - Bishop :: cast protect next
	if (flag && B_SK(cn, SK_PROTECT))
	{
		skill_protect(cn, 1);
	}
	else if (flag && B_SK(cn, SK_ENHANCE))
	{
		skill_enhance(cn, 1);
	}
	else
	{
		add_exhaust(cn, SK_EXH_BLESS);
	}
}

int spell_mshield(int cn, int co, int power, int fromscroll)
{
	int in, ta_co_emp, n, cc;
	
	power = spellpower_check(cn, co, spell_multiplier(power, cn), fromscroll);
	
	ta_co_emp = get_tarot(cn, IT_CH_EMPRESS);
	
	if (ta_co_emp && !fromscroll)
	{
		in = make_new_buff(cn, SK_MSHELL, BUF_SPR_MSHELL, power, SP_DUR_MSHELL(power), 0);
		n = SK_MSHELL;
	}
	else
	{
		in = make_new_buff(cn, SK_MSHIELD, BUF_SPR_MSHIELD, power, SP_DUR_MSHIELD(power), 0);
		n = SK_MSHIELD;
	}
	
	if (!in) 
		return 0;
	
	// Tarot Card - Empress :: Change Magic Shield into Magic Shell
	if (ta_co_emp)
	{
		bu[in].power = bu[in].active / 128;
		if (IS_SEYA_OR_BRAV(co)) 
		{
			bu[in].skill[SK_RESIST][1] = min(127, bu[in].active / 768 + 1);
			bu[in].skill[SK_IMMUN][1]  = min(127, bu[in].active / 768 + 1);
		}
		else
		{
			bu[in].skill[SK_RESIST][1] = min(127, bu[in].active / 512 + 1);
			bu[in].skill[SK_IMMUN][1]  = min(127, bu[in].active / 512 + 1);
		}
	}
	else
	{
		bu[in].power = bu[in].active / 256;
		if (IS_SEYA_OR_BRAV(co)) 
		{
			bu[in].armor[1]  = min(127, bu[in].active / 1536 + 1);
		}
		else
		{
			bu[in].armor[1]  = min(127, bu[in].active / 1024 + 1);
		}
	}
	
	return cast_a_spell(cn, co, in, 0, 1); // SK_MSHIELD / SK_MSHELL
}
void skill_mshield(int cn)
{
	int co, power;
	
	if (is_exhausted(cn)) 								{ return; }
	if (spellcost(cn, SP_COST_MSHIELD, SK_MSHIELD, 1))	{ return; }
	if (chance(cn, FIVE_PERC_FAIL)) 					{ return; }
	
	power = M_SK(cn, SK_MSHIELD);
	
	spell_mshield(cn, cn, power, 0);
	
	if (T_SUMM_SK(cn, 12))
	{
		if (IS_PLAYER(cn) && IS_PLAYER_COMP(co = ch[cn].data[PCD_COMPANION] )) spell_mshield(cn, co, power, 0);
		if (IS_PLAYER(cn) && IS_PLAYER_COMP(co = ch[cn].data[PCD_SHADOWCOPY])) spell_mshield(cn, co, power, 0);
	}

	add_exhaust(cn, SK_EXH_MSHIELD);
}

// Feb 2020 -- Haste
int spell_haste(int cn, int co, int power, int fromscroll)
{
	int in;
	
	power = spellpower_check(cn, co, spell_multiplier(power, cn), fromscroll);
	
	if (!(in = make_new_buff(cn, SK_HASTE, BUF_SPR_HASTE, power, SP_DUR_HASTE, 1))) 
		return 0;
	
	bu[in].speed[1]				= min(300, 10 + (power  )/ 6);
	bu[in].atk_speed[1]			= min(127,  5 + (power+6)/12);
	bu[in].cast_speed[1]		= min(127,  5 + (power+6)/12);
	
	if (T_SORC_SK(co, 12))
		bu[in].move_speed[1]	= min(127,  2 + (power  )/30);
	
	return cast_a_spell(cn, co, in, 0, 1); // SK_HASTE
}
void skill_haste(int cn)
{
	int co, power;
	
	power = M_SK(cn, SK_HASTE);
	
	if (T_SORC_SK(cn, 12))
	{
		if (!(co = get_target(cn, 0, 1, 1, SP_COST_HASTE, SK_HASTE, 1, power, 0)))
			return;
		
		spell_haste(cn, co, power, 0);
	}
	else
	{
		if (is_exhausted(cn)) 							{ return; }
		if (spellcost(cn, SP_COST_HASTE, SK_HASTE, 1))	{ return; }
		if (chance(cn, FIVE_PERC_FAIL)) 				{ return; }
		
		spell_haste(cn, cn, power, 0);
	}
	
	add_exhaust(cn, SK_EXH_HASTE);
}

// Oct 2020 - Regen via Tarot Card
int spell_regen(int cn, int co, int power)
{
	int in, healing = 1875;
	
	power = spell_multiplier(power, cn);
	
	if (!(in = make_new_buff(cn, SK_REGEN, BUF_SPR_REGEN, power, SP_DUR_REGEN, 0))) 
		return 0;
	
	bu[in].hp[0] = (power * healing) / SP_DUR_REGEN;
	
	return cast_a_spell(cn, co, in, 0, 1); // SK_REGEN
}
int spell_heal(int cn, int co, int power)
{
	int in, in2, n, tmp, healing = 2500, dur = SP_DUR_HEAL;
	
	if (get_enchantment(co, 9)) dur = dur * 2/3;
	
	if (!(in = make_new_buff(cn, SK_HEAL, BUF_SPR_HEALSICK, power, dur, 0))) 
		return 0;
	
	bu[in].data[1] = 0;
	bu[in].data[4] = 1; // Effects not removed by NMZ (SK_HEAL)
	bu[in].data[5] = 1;
	
	// Every time heal is cast it updates itself and adds 1 to data[1]
	if (cast_a_spell(cn, co, in, 0, 0)) // SK_HEAL
	{
		if ((in2=has_buff(co, SK_HEAL))!=0)
		{
			// Each stack of heal sickness reduces the spell power by 1/4th
			tmp = 4 - bu[in2].data[1];
			power = power * tmp * (T_BRAV_SK(co, 10)?60:100) / 400;
			if (4-tmp) 
			{
				do_char_log(cn, 1, "Heal's power was reduced by %d%%\n", (T_BRAV_SK(co, 10)?(100-tmp*25*6/10):(100-tmp*25)));
			}
		}
	}
	
	if (cn!=co)
	{
		ch[co].a_hp += spell_multiplier(power * healing, cn);
	}
	else
	{
		ch[co].a_hp += power * healing;
	}
	
	if (ch[co].a_hp > ch[co].hp[5] * 1000)
	{
		ch[co].a_hp = ch[co].hp[5] * 1000;
	}
	
	return 1;
}
void skill_heal(int cn)
{
	int co, power;
	
	power = M_SK(cn, SK_HEAL);
	
	if (!IS_PLAYER(cn)) power = power * 2/3;
	if (get_enchantment(cn, 56)) power = power*6/5;
	if (get_gear(cn, IT_TW_SUPERBIA)) power /= 2;
	
	if (!(co = get_target(cn, 0, 1, 1, SP_COST_HEAL, SK_HEAL, 1, power, 0))) 
		return;
	
	// Tarot Card - Star :: Change Heal into Regen
	if (get_tarot(cn, IT_CH_STAR))
	{
		spell_regen(cn, co, power);
	}
	else
	{
		spell_heal(cn, co, power);
	}

	add_exhaust(cn, SK_EXH_HEAL);
}

void speed_plague(int co, int in)
{
	if (bu[in].data[5] != 3)
	{
		bu[in].data[1] *= 4;
		bu[in].data[6] *= 4;
		bu[in].active  /= 2;
		bu[in].data[5]  = 3;
	}
}
int spell_plague(int cn, int co, int flag)
{
	int in, n, dur, power, ppow;
	
	if (ch[co].escape_timer > TICKS*3) { return 0; }
	if (ch[co].flags & CF_BODY) { return 0; }
	if (ch[cn].attack_cn!=co && ch[co].alignment==10000) { return 0; }
	if (ch[co].flags & CF_IMMORTAL) { return 0; }
	
	power = M_SK(cn, SK_FEROC);
	power = spell_multiplier(power, cn);
	power = spell_immunity(power, get_target_immunity(cn, co));
	
	if (T_SEYA_SK(co, 10))				power = power*4/5;
	if (get_enchantment(cn, 30)) 		power = power*6/5;
	if (get_tarot(cn, IT_CH_CHARIO_R)) 	power = power*3/4;
	if (get_enchantment(co, 4)) 		power = power*4/5;
	
	dur = SP_DUR_PLAGUE;
	ppow = PLAGUEFORM(power, dur);
	
	if (flag == 3)
	{
		dur  /= 2;
		ppow *= 4;
	}
	
	if (!(in = make_new_buff(cn, SK_PLAGUE, BUF_SPR_PLAGUE, power, dur, 0)))
		return 0;
	
	bu[in].data[1] = ppow;				// Set the decay rate
	bu[in].data[5] = flag;
	bu[in].data[6] = ppow;				// Backup decay rate
	bu[in].data[7] = -1;
	
	do_char_log(co, 0, "The plague spreads through you.\n");
	
	return cast_a_spell(cn, co, in, 3, 0); // SK_PLAGUE
}
int skill_plague(int cn, int co, int flag)
{
	cast_aoe_spell(cn, co, SK_PLAGUE, flag, 0, 0, 0, 0, 0, -1, -1);
}

int spell_curse(int cn, int co, int power, int flag)
{
	int in, n;
	
	if (ch[co].escape_timer > TICKS*3) { return 0; }
	if (ch[co].flags & CF_BODY) { return 0; }
	if (ch[cn].attack_cn!=co && ch[co].alignment==10000) { return 0; }
	if (ch[co].flags & CF_IMMORTAL) { return 0; }
	
	power = spell_multiplier(power, cn);
	power = spell_immunity(power, get_target_immunity(cn, co));
	
	// Spell suppression
	if (ch[co].spellfail==1) 
	{
		chlog(co, "Suppressed the spell");
		power = (power - (power * (M_AT(co, AT_WIL) - M_AT(cn, AT_WIL)) / 500))/2;
	}
	
	if (T_SEYA_SK(co, 10))				power = power*4/5;
	if (T_SORC_SK(cn,  9)) 				power = power + (power * M_AT(cn, AT_INT)/2500);
	if (get_enchantment(cn, 20)) 		power = power*6/5;
	if (get_tarot(cn, IT_CH_CHARIO_R)) 	power = power*3/4;
	if (get_enchantment(co, 4)) 		power = power*4/5;
	if (get_enchantment(co, 21)) 		power = power/5;
	
	// Tarot Card - Tower :: Change Curse into Greater Curse
	if (get_tarot(cn, IT_CH_TOWER))
	{
		if (!(in = make_new_buff(cn, SK_CURSE2, BUF_SPR_CURSE2, power, SP_DUR_CURSE2, 0)))
			return 0;
		
		for (n = 0; n<5; n++) 
		{
			bu[in].attrib[n][1] = -(5 + CURSE2FORM(power, (4 - n)));
		}
	}
	else
	{
		if (!(in = make_new_buff(cn, SK_CURSE, BUF_SPR_CURSE, power, SP_DUR_CURSE, 0)))
			return 0;
		
		for (n = 0; n<5; n++)
		{
			bu[in].attrib[n][1] = -(3 + (power - (4 - n)) / 5);
		}
	}
	bu[in].data[5] = 1;
	
	return cast_a_spell(cn, co, in, 1+flag, 1-flag); // SK_CURSE / SK_CURSE2
}
void skill_curse(int cn)
{
	int d20 = SP_MULT_CURSE;
	int power, aoe_power, cost, cost2, flag = 0;
	int count = 0, hit = 0;
	int co, co_orig = -1;
	int can_aoe = CAN_SORC_PROX(cn);
	
	if (get_book(cn, IT_BOOK_SHIV)) flag = 1;
	
	power = M_SK(cn, SK_CURSE);
	aoe_power = M_SK(cn, SK_PROX);
	cost = SP_COST_CURSE;
	
	// Tarot Card - Tower :: Change Curse into Greater Curse
	if (get_tarot(cn, IT_CH_TOWER)) 
	{ 
		cost = cost * 4 / 3;
		d20 = SP_MULT_CURSE2;
	}
	
	if (flag)
	{
		cost2 = SP_COST_SLOW;
	
		// Tarot Card - Emperor :: Change Slow into Greater Slow
		if (get_tarot(cn, IT_CH_EMPEROR)) 
		{ 
			cost2 = cost2 * 3 / 4;
		}
		
		cost += cost2;
	}
	
	// Get spell target - return on failure
	if (!(co = get_target(cn, 0, 0, 0, cost, SK_CURSE, 1, power, d20)))
		return;
	
	// If we have a valid target, cast Curse on them
	if (cn!=co && co!=ch[cn].data[PCD_SHADOWCOPY] && co!=ch[cn].data[PCD_COMPANION])
	{
		spell_curse(cn, co, power, 0);
		
		co_orig = co;
		count++;
		hit++;
	}
	
	// Cast AoE or general surround-hit
	if (can_aoe)
	{
		if (cast_aoe_spell(cn, co, SK_CURSE, power, aoe_power, cost, count, hit, 0, -1, -1) < 0)
			return;
		
		fx_add_effect(7, 0, ch[cn].x, ch[cn].y, 0);
	}
	else
	{
		surround_cast(cn, co_orig, 0, SK_CURSE, power, -1, -1);
	}
	
	// Book - Shiva's Malice :: Cast Slow after casting Curse on success
	if (flag)
	{
		skill_slow(cn, 1);
	}
	else
	{
		add_exhaust(cn, SK_EXH_CURSE);
	}
}
// Feb 2020 - Slow
int spell_slow(int cn, int co, int power, int flag)
{
	int in;
	
	if (ch[co].escape_timer > TICKS*3) { return 0; }
	if (ch[co].flags & CF_BODY) { return 0; }
	if (ch[cn].attack_cn!=co && ch[co].alignment==10000) { return 0; }
	if (ch[co].flags & CF_IMMORTAL) { return 0; }
	
	power = spell_multiplier(power, cn);
	power = spell_immunity(power, get_target_immunity(cn, co));
	
	// Spell suppression
	if (ch[co].spellfail==1) 
	{
		chlog(co, "Suppressed the spell");
		power = (power - (power * (M_AT(co, AT_WIL) - M_AT(cn, AT_WIL)) / 500))/2;
	}
	
	if (T_SEYA_SK(co, 10))				power = power*4/5;
	if (T_SORC_SK(cn,  9)) 				power = power + (power * M_AT(cn, AT_INT)/2500);
	if (get_enchantment(cn, 13)) 		power = power*6/5;
	if (get_tarot(cn, IT_CH_CHARIO_R)) 	power = power*3/4;
	if (get_enchantment(co, 4)) 		power = power*4/5;
	if (get_enchantment(co, 14)) 		power = power/5;
	
	// Tarot Card - Emperor :: Change Slow into Greater Slow
	if (get_tarot(cn, IT_CH_EMPEROR))
	{
		if (!(in = make_new_buff(cn, SK_SLOW2, BUF_SPR_SLOW2, power, SP_DUR_SLOW2(power), 0)))
			return 0;
		
		bu[in].speed[1] = -(min(300, 30 + SLOW2FORM(power)));
	}
	else
	{
		if (!(in = make_new_buff(cn, SK_SLOW, BUF_SPR_SLOW, power, SP_DUR_SLOW(power), 0)))
			return 0;
		
		bu[in].speed[1] 		= -(min(300, 20 + SLOWFORM(power)*2/3));
		bu[in].atk_speed[1] 	= -(min(127, 10 + SLOWFORM(power)/3));
		bu[in].cast_speed[1] 	= -(min(127, 10 + SLOWFORM(power)/3));
	}
	bu[in].data[5] = 1;
	
	return cast_a_spell(cn, co, in, 1+flag, 1-flag); // SK_SLOW / SK_SLOW2
}
void skill_slow(int cn, int flag)
{
	int d20 = SP_MULT_SLOW;
	int power, aoe_power, cost;
	int count = 0, hit = 0;
	int co, co_orig = -1;
	int can_aoe = CAN_SORC_PROX(cn);
	
	power = M_SK(cn, SK_SLOW);
	aoe_power = M_SK(cn, SK_PROX);
	cost = SP_COST_SLOW;
	
	// Tarot Card - Emperor :: Change Slow into Greater Slow
	if (get_tarot(cn, IT_CH_EMPEROR)) 
	{ 
		cost = cost * 3 / 4;
		d20 = SP_MULT_SLOW2;
	}
	
	// Get spell target - return on failure
	if (!(co = get_target(cn, 0, 0, 0, flag?0:cost, SK_SLOW, 1, power, flag?-1:d20)))
		return;
	
	// If we have a valid target, cast Slow on them
	if (cn!=co && co!=ch[cn].data[PCD_SHADOWCOPY] && co!=ch[cn].data[PCD_COMPANION])
	{
		spell_slow(cn, co, power, 0);
		
		co_orig = co;
		count++;
		hit++;
	}
	
	// Cast AoE or general surround-hit
	if (can_aoe)
	{
		if (cast_aoe_spell(cn, co, SK_SLOW, power, aoe_power, cost, count, hit, 0, -1, -1) < 0)
			return;
		
		fx_add_effect(7, 0, ch[cn].x, ch[cn].y, 0);
	}
	else
	{
		surround_cast(cn, co_orig, 0, SK_SLOW, power, -1, -1);
	}
	
	// Book - Shiva's Malice :: Extend exhaust after casting both Curse and Slow
	if (flag)
	{
		add_exhaust(cn, SK_EXH_CURSE + SK_EXH_SLOW);
	}
	else
	{
		add_exhaust(cn, SK_EXH_SLOW);
	}
}

int spell_frostburn(int cn, int co, int power)
{
	int in, dur, ppow;
	
	if (ch[co].escape_timer > TICKS*3) { return 0; }
	if (ch[co].flags & CF_BODY) { return 0; }
	if (ch[cn].attack_cn!=co && ch[co].alignment==10000) { return 0; }
	if (ch[co].flags & CF_IMMORTAL) { return 0; }
	
	power = spell_multiplier(power, cn);
	power = spell_immunity(power, get_target_immunity(cn, co));
	
	if (T_SEYA_SK(co, 10))				power = power*4/5;
	if (get_tarot(cn, IT_CH_CHARIO_R)) 	power = power*3/4;
	if (get_enchantment(co, 4)) 		power = power*4/5;
	
	dur = SP_DUR_FROSTB; 	// 20 seconds
	
	ppow = -FROSTBFORM(power, dur);
	
	if (!(in = make_new_buff(cn, SK_FROSTB, BUF_SPR_FROSTB, power, dur, 0)))
		return 0;
	
	// Set the decay rate of the frostburn
	bu[in].end[0]  = ppow;
	bu[in].mana[0] = ppow;
	bu[in].data[5] = 1;
	
	return cast_a_spell(cn, co, in, 2, 0); // SK_FROSTB
}

// Feb 2020 - Poison
int spell_poison(int cn, int co, int power, int flag)
{
	int in, dur, ppow, venommod;
	
	if (ch[co].escape_timer > TICKS*3) { return 0; }
	if (ch[co].flags & CF_BODY) { return 0; }
	if (ch[cn].attack_cn!=co && ch[co].alignment==10000) { return 0; }
	if (ch[co].flags & CF_IMMORTAL) { return 0; }
	
	power = spell_multiplier(power, cn);
	power = spell_immunity(power, get_target_immunity(cn, co));
	
	// Spell suppression
	if (ch[co].spellfail==1) 
	{
		chlog(co, "Suppressed the spell");
		power = (power - (power * (M_AT(co, AT_WIL) - M_AT(cn, AT_WIL)) / 500))/2;
	}
	
	if (T_SEYA_SK(co, 10))				power = power*4/5;
	if (get_enchantment(cn, 27)) 		power = power*6/5;
	if (get_tarot(cn, IT_CH_CHARIO_R)) 	power = power*3/4;
	if (get_enchantment(co, 4)) 		power = power*4/5;
	
	dur = SP_DUR_POISON; 			// 30 seconds
	
	// Book - Venom Compendium
	if (get_book(cn, IT_BOOK_VENO)) 	dur = dur *  4/ 5;	// 80% duration = 24 seconds = 25% more dps
	if (T_SORC_SK(cn, 4)) 				dur = dur *  9/10;	// 90% duration = 27 seconds = 11% more dps
	
	if (IS_PLAYER(cn))
	{
		ppow = PL_POISFORM(power, dur);
		venommod = 3;
	}
	else
	{
		ppow = MN_POISFORM(power, dur);
		venommod = 2;
	}
	
	// Tarot - Tower.R : Poison becomes Venom
	if (get_tarot(cn, IT_CH_TOWER_R))
	{
		if (!(in = make_new_buff(cn, SK_VENOM, BUF_SPR_VENOM, power, dur, 0)))
			return 0;
		
		bu[in].data[1] = ppow/2;			// Set the decay rate
		bu[in].data[2] = ppow/2*3;			// Set the decay maximum
		bu[in].data[6] = (power/12 + 3)*venommod/3;   // Set immunity reduction
		bu[in].data[7] = (power/12 + 3)*venommod/3*3; // Set immunity maximum
		bu[in].skill[SK_IMMUN][1] = -(bu[in].data[6]);
		bu[in].value = 1;
	}
	else
	{
		if (!(in = make_new_buff(cn, SK_POISON, BUF_SPR_POISON, power, dur, 0)))
			return 0;
		
		bu[in].data[1] = ppow;				// Set the decay rate
	}
	bu[in].data[5] = 1;
	
	return cast_a_spell(cn, co, in, 1+flag, 1-flag); // SK_POISON
}
void skill_poison(int cn)
{
	int d20 = SP_MULT_POISON;
	int power, aoe_power, cost;
	int count = 0, hit = 0;
	int co, co_orig = -1;
	int can_aoe = (CAN_SORC_PROX(cn)||CAN_ARHR_PROX(cn));
	
	power = M_SK(cn, SK_POISON);
	aoe_power = M_SK(cn, SK_PROX);
	cost = SP_COST_POISON;
	
	if (get_tarot(cn, IT_CH_TOWER_R)) d20 = SP_MULT_POISON2;
	
	if (IS_PLAYER(cn) && IS_ANY_MERC(cn)) 
	{
		cost /= 2;
	}
	
	// Get spell target - return on failure
	if (!(co = get_target(cn, 0, 0, 0, cost, SK_POISON, 1, power, d20)))
		return;
	
	// If we have a valid target, cast Poison on them
	if (cn!=co && co!=ch[cn].data[PCD_SHADOWCOPY] && co!=ch[cn].data[PCD_COMPANION])
	{
		spell_poison(cn, co, power, 0);
		
		co_orig = co;
		count++;
		hit++;
	}
	
	// Cast AoE or general surround-hit
	if (can_aoe)
	{
		if (cast_aoe_spell(cn, co, SK_POISON, power, aoe_power, cost, count, hit, 0, -1, -1) < 0)
			return;
		
		fx_add_effect(7, 0, ch[cn].x, ch[cn].y, 0);
	}
	else
	{
		surround_cast(cn, co_orig, 0, SK_POISON, power, -1, -1);
	}
	
	add_exhaust(cn, SK_EXH_POISON);
}

int spell_stun(int cn, int co, int power)
{
	int n, in, dur;
	
	if (ch[co].escape_timer > TICKS*3) { return 0; }
	if (ch[co].flags & CF_BODY) { return 0; }
	if (!do_surround_check(cn, co, 1) || 
		chance_base(cn, co, SK_WARCRY, SP_MULT_WARCRY, get_target_resistance(cn, co), 0, 0)) { return 0; }
	if (cn!=co) do_area_notify(cn, co, ch[cn].x, ch[cn].y, NT_SEEHIT, cn, co, 0, 0);
	if (!IS_IGNORING_SPELLS(co)) { do_notify_char(co, NT_GOTHIT, cn, 0, 0, 0); }
	
	power = spell_immunity(power, get_target_immunity(cn, co));
	
	if (T_SEYA_SK(co, 10))				power = power*4/5;
	if (T_ARTM_SK(cn,  7)) 				power = power + (power * M_AT(cn, AT_STR)/2000);
	if (get_tarot(cn, IT_CH_CHARIO_R)) 	power = power*3/4;
	if (get_enchantment(co, 4)) 		power = power*4/5;
	
	dur = SP_DUR_WARCRY2(power);
	
	// Add War-Stun
	if (!(in = make_new_buff(cn, SK_WARCRY2, BUF_SPR_WARCRY2, power, dur, 0)))
		return 0;
	
	bu[in].data[4] = 1; // Effects not removed by NMZ (SK_WARCRY2)
	bu[in].data[5] = 1;
	
	add_spell(co, in);
}

int spell_rally(int cn, int co, int power)
{
	int in;
	
	if (T_ARTM_SK(cn, 7)) power = power + (power * M_AT(cn, AT_STR)/2000);
	
	if (!(in = make_new_buff(cn, SK_WARCRY3, BUF_SPR_WARCRY3, power, SP_DUR_RALLY, 1))) 
		return 0;
	
	bu[in].to_hit[1]   = power/10;
	bu[in].to_parry[1] = power/10;
	bu[in].data[4]     = 1;

	return cast_a_spell(cn, co, in, 0, 1); // SK_WARCRY3
}
int skill_rally(int cn, int power)
{
	int aoe_power;
	int xf, yf,	xt, yt, xc, yc, x, y, co, spellaoe, spellrad;
	int m1, m2;
	double _cap, tmpa;
	
	aoe_power = power + 15;
		
	spellrad = PRXW_RAD + ch[cn].aoe_bonus;
	_cap = (double)(PRXW_CAP);
	spellaoe = aoe_power/_cap + spellrad;
	tmpa = (double)(aoe_power*100/_cap + spellrad*100);
	
	xc = ch[cn].x;
	yc = ch[cn].y;
	xf = max(1, ch[cn].x - spellaoe);
	yf = max(1, ch[cn].y - spellaoe);
	xt = min(MAPX - 1, ch[cn].x + spellaoe+1);
	yt = min(MAPY - 1, ch[cn].y + spellaoe+1);
	
	m1 = XY2M(ch[cn].x, ch[cn].y);
	
	for (x = xf; x<xt; x++) for (y = yf; y<yt; y++) if ((co = map[x + y * MAPX].ch) /*&& co!=cn*/)
	{
		if (co != cn)
		{
			// This makes the radius circular instead of square
			if (sqr(xc - x) + sqr(yc - y) > (sqr(tmpa/100) + 1))
			{
				continue;
			}
			if ((IS_PLAYER(cn) && !(IS_PLAYER(co) || IS_PLAYER_COMP(co))) || 
				!IS_PLAYER(cn) && (IS_PLAYER(co) || IS_PLAYER_COMP(co)))
			{
				if (!(ch[co].flags & CF_SYS_OFF))
					do_char_log(co, 1, "You hear %s rally their allies.\n", ch[cn].reference);
				continue;
			}
			//if (IS_PURPLE(cn) && IS_PURPLE(co)) continue;
			if (IS_OPP_CLAN(cn, co)) 
			{
				if (!(ch[co].flags & CF_SYS_OFF))
					do_char_log(co, 1, "You hear %s rally their allies.\n", ch[cn].reference);
				continue;
			}
			if (!IS_PLAYER(cn) && ch[cn].data[CHD_GROUP] != ch[co].data[CHD_GROUP]) 
			{
				if (!(ch[co].flags & CF_SYS_OFF))
					do_char_log(co, 1, "You hear %s rally their allies.\n", ch[cn].reference);
				continue;
			}
			m2 = XY2M(ch[co].x, ch[co].y);
			if ((map[m1].flags & MF_ARENA) && (map[m2].flags & MF_ARENA)) 
			{
				if (!(ch[co].flags & CF_SYS_OFF))
					do_char_log(co, 1, "You hear %s rally their allies.\n", ch[cn].reference);
				continue;
			}
		}
		if (spell_rally(cn, co, cn==co?power*3/4:power))
		{
			if (cn != co)
				do_char_log(co, 1, "You hear %s's rally and feel inspired!\n", ch[cn].reference);
		}
		else
		{
			if (!(ch[co].flags & CF_SYS_OFF))
				do_char_log(co, 1, "You hear %s's rally, but are somehow uninspired.\n", ch[cn].reference);
		}
	}
	do_char_log(cn, 1, "You call for your allies, rallying them to your cause!\n");
	
	return 1;
}
int spell_warcry(int cn, int co, int power, int flag)
{
	int n, in, dur;
	
	if (ch[co].escape_timer > TICKS*3) { return 0; }
	if (ch[co].flags & CF_BODY) { return 0; }
	if (flag != 2 && (!do_surround_check(cn, co, 1) || 
		chance_base(cn, co, SK_WARCRY, SP_MULT_WARCRY, get_target_resistance(cn, co), 0, 0))) { return 0; }
	if (cn!=co) do_area_notify(cn, co, ch[cn].x, ch[cn].y, NT_SEEHIT, cn, co, 0, 0);
	if (!IS_IGNORING_SPELLS(co)) { do_notify_char(co, NT_GOTHIT, cn, 0, 0, 0); }
	
	power = spell_immunity(power, get_target_immunity(cn, co));
	
	if (T_SEYA_SK(co, 10))				power = power*4/5;
	if (!flag && T_ARTM_SK(cn,  7)) 	power = power + (power * M_AT(cn, AT_STR)/2000);
	if (get_tarot(cn, IT_CH_CHARIO_R)) 	power = power*3/4;
	if (get_enchantment(co, 4)) 		power = power*4/5;
	
	dur = SP_DUR_WARCRY2(power);
	
	if (ch[cn].temp == CT_PANDIUM) dur = dur/5;
	
	// Add War-Stun
	if (!(in = make_new_buff(cn, SK_WARCRY2, BUF_SPR_WARCRY2, power, dur, 0)))
		return 0;
	
	bu[in].data[4] = 1; // Effects not removed by NMZ (SK_WARCRY2)
	bu[in].data[5] = 1;
	
	add_spell(co, in);
	//
	if (flag) return 1;
	//
	
	// Add War-Fear
	if (!(in = make_new_buff(cn, SK_WARCRY, BUF_SPR_WARCRY, power, SP_DUR_WARCRY, 0)))
		return 0;
	
	if (IS_PLAYER(cn) && IS_ARCHTEMPLAR(cn))
	{
		for (n = 0; n<5; n++) bu[in].attrib[n][1] = -(4+((power*5/8)-n) / 5);
	}
	else
	{
		for (n = 0; n<5; n++) bu[in].attrib[n][1] = -(3+(power/2-n) / 5);
	}
	
	bu[in].data[4] = 1; // Effects not removed by NMZ (SK_WARCRY)
	bu[in].data[5] = 1;
	
	add_spell(co, in);
	//
	
	chlog(cn, "Used Warcry on %s", ch[co].name);
	fx_add_effect(5, 0, ch[co].x, ch[co].y, 0);
	return 1;
}
void skill_warcry(cn)
{
	int power, aoepower, cost;
	
	power = M_SK(cn, SK_WARCRY);
	power = skill_multiplier(power, cn);
	aoepower = M_SK(cn, SK_WARCRY) + (B_SK(cn, SK_PROX)?(M_SK(cn, SK_PROX)/2):0);
	cost = SP_COST_WARCRY;
	
	if (is_exhausted(cn)) { return; }
	if (spellcost(cn, cost, SK_WARCRY, 0)) { return; }
	
	if (get_tarot(cn, IT_CH_EMPERO_R))
	{
		skill_rally(cn, power);
	}
	else
	{
		if (cast_aoe_spell(cn, 0, SK_WARCRY, power, aoepower, 0, 0, 0, 0, -1, -1) < 0)
			return;
	}
	
	add_exhaust(cn, SK_EXH_WARCRY + TICKS * power/80);
}

void item_info(int cn, int in, int look)
{
	int n, soulstone=0;
	
	if (it[in].driver==68 || it[in].driver==92 || it[in].driver==93) soulstone=1;
	
	if (look<2) {
		do_char_log(cn, 1, "%s:\n", it[in].name);
	}
	if (soulstone)
	{
		do_char_log(cn, 1, "Stat          Mod+   R.Cost   Equip Req\n");
		
		for (n = 0; n<MAXSKILL; n++)
		{
			if (!it[in].skill[n][0] && !it[in].skill[n][1])
			{
				continue;
			}
			do_char_log(cn, 1, "%-12.12s  %+4d   %6d   %9d\n",
			skilltab[n].name, it[in].skill[n][0], it[in].skill[n][1], it[in].skill[n][0]*5);
		}
	}
	else
	{
		do_char_log(cn, 1, "Stat          Mod0   Mod1   Min\n");
	
		for (n = 0; n<5; n++)
		{
			if (!it[in].attrib[n][0] && !it[in].attrib[n][1] && !it[in].attrib[n][2])
			{
				continue;
			}
			do_char_log(cn, 1, "%-12.12s  %+4d   %+4d   %3d\n",
			at_name[n], it[in].attrib[n][0], it[in].attrib[n][1], (unsigned char)it[in].attrib[n][2]);
		}
		
		if (it[in].hp[0] || it[in].hp[1] || it[in].hp[2])
		{
			do_char_log(cn, 1, "%-12.12s  %+4d   %+4d   %3d\n",
			"Hitpoints", it[in].hp[0], it[in].hp[1], it[in].hp[2]);
		}
		if (it[in].end[0] || it[in].end[1] || it[in].end[2])
		{
			do_char_log(cn, 1, "%-12.12s  %+4d   %+4d   %3d\n",
			"Endurance", it[in].end[0], it[in].end[1], it[in].end[2]);
		}
		if (it[in].mana[0] || it[in].mana[1] || it[in].mana[2])
		{
			do_char_log(cn, 1, "%-12.12s  %+4d   %+4d   %3d\n",
			"Mana", it[in].mana[0], it[in].mana[1], it[in].mana[2]);
		}

		for (n = 0; n<MAXSKILL; n++)
		{
			if (!it[in].skill[n][0] && !it[in].skill[n][1] && !it[in].skill[n][2])
			{
				continue;
			}
			do_char_log(cn, 1, "%-12.12s  %+4d   %+4d   %3d\n",
			skilltab[n].name, it[in].skill[n][0], it[in].skill[n][1], (unsigned char)it[in].skill[n][2]);
		}
	}
	
	if (it[in].armor[0] || it[in].armor[1])
	{
		do_char_log(cn, 1, "%-12.12s  %+4d   %+4d\n",
		"Armor Value", it[in].armor[0], it[in].armor[1]);
	}
	if (it[in].weapon[0] || it[in].weapon[1])
	{
		do_char_log(cn, 1, "%-12.12s  %+4d   %+4d\n",
		"Weapon Value", it[in].weapon[0], it[in].weapon[1]);
	}
	if (it[in].base_crit && !soulstone)
	{
		do_char_log(cn, 1, "%-12.12s  %4d%%\n",
		"Base Crit", it[in].base_crit);
	}
	
	if (it[in].to_hit[0] || it[in].to_hit[1])
	{
		do_char_log(cn, 1, "%-12.12s  %+4d   %+4d\n",
		"Hit Bonus", it[in].to_hit[0], it[in].to_hit[1]);
	}
	if (it[in].to_parry[0] || it[in].to_parry[1])
	{
		do_char_log(cn, 1, "%-12.12s  %+4d   %+4d\n",
		"Parry Bonus", it[in].to_parry[0], it[in].to_parry[1]);
	}
	
	if (it[in].dmg_bonus[0] || it[in].dmg_bonus[1])
	{
		do_char_log(cn, 1, "%-12.12s  %+2d.%1d   %+2d.%1d\n",
		"Dmg Bonus", it[in].dmg_bonus[0]/20, it[in].dmg_bonus[0]/2%10, it[in].dmg_bonus[1]/20, it[in].dmg_bonus[1]/2%10);
	}
	if (it[in].dmg_reduction[0] || it[in].dmg_reduction[1])
	{
		do_char_log(cn, 1, "%-12.12s  %+2d.%1d   %+2d.%1d\n",
		"Dmg Reduce", it[in].dmg_reduction[0]/20, it[in].dmg_reduction[0]/2%10, it[in].dmg_reduction[1]/20, it[in].dmg_reduction[1]/2%10);
	}
	
	if (soulstone)
	{
		if (it[in].crit_chance[0] || it[in].crit_chance[1])
		{
			do_char_log(cn, 1, "%-12.12s  %+4d%%  %+4d\n",
			"Crit Bonus", it[in].crit_chance[0], it[in].crit_chance[1]);
		}
		if (it[in].crit_multi[0] || it[in].crit_multi[1])
		{
			do_char_log(cn, 1, "%-12.12s  %+4d%%  %+4d\n",
			"Crit Multi", it[in].crit_multi[0], it[in].crit_multi[1]);
		}
	}
	else
	{
		if (it[in].crit_chance[0] || it[in].crit_chance[1])
		{
			do_char_log(cn, 1, "%-12.12s  %+4d%%  %+4d%%\n",
			"Crit Bonus", it[in].crit_chance[0], it[in].crit_chance[1]);
		}
		if (it[in].crit_multi[0] || it[in].crit_multi[1])
		{
			do_char_log(cn, 1, "%-12.12s  %+4d%%  %+4d%%\n",
			"Crit Multi", it[in].crit_multi[0], it[in].crit_multi[1]);
		}
	}
	if (it[in].top_damage[0] || it[in].top_damage[1])
	{
		do_char_log(cn, 1, "%-12.12s  %+4d   %+4d\n",
		"Top Damage", it[in].top_damage[0], it[in].top_damage[1]);
	}
	if (it[in].gethit_dam[0] || it[in].gethit_dam[1])
	{
		do_char_log(cn, 1, "%-12.12s  %+4d   %+4d\n",
		"Thorns", it[in].gethit_dam[0], it[in].gethit_dam[1]);
	}
	
	if (it[in].aoe_bonus[0] || it[in].aoe_bonus[1])
	{
		do_char_log(cn, 1, "%-12.12s  %+4d   %+4d\n",
		"AoE Bonus", it[in].aoe_bonus[0], it[in].aoe_bonus[1]);
	}
	
	if (it[in].speed[0] || it[in].speed[1])
	{
		do_char_log(cn, 1, "%-12.12s  %+4d   %+4d\n",
		"All Speed", it[in].speed[0], it[in].speed[1]);
	}
	if (it[in].move_speed[0] || it[in].move_speed[1])
	{
		do_char_log(cn, 1, "%-12.12s  %+4d   %+4d\n",
		"Move Speed", it[in].move_speed[0], it[in].move_speed[1]);
	}
	if (it[in].atk_speed[0] || it[in].atk_speed[1])
	{
		do_char_log(cn, 1, "%-12.12s  %+4d   %+4d\n",
		"Atk Speed", it[in].atk_speed[0], it[in].atk_speed[1]);
	}
	if (it[in].cast_speed[0] || it[in].cast_speed[1])
	{
		do_char_log(cn, 1, "%-12.12s  %+4d   %+4d\n",
		"Cast Speed", it[in].cast_speed[0], it[in].cast_speed[1]);
	}
	
	if (it[in].spell_mod[0] || it[in].spell_mod[1])
	{
		do_char_log(cn, 1, "%-12.12s  %+4d   %+4d\n",
		"Spell Mod", it[in].spell_mod[0], it[in].spell_mod[1]);
	}
	if (it[in].spell_apt[0] || it[in].spell_apt[1])
	{
		do_char_log(cn, 1, "%-12.12s  %+4d   %+4d\n",
		"Spell Apt", it[in].spell_apt[0], it[in].spell_apt[1]);
	}
	if (it[in].cool_bonus[0] || it[in].cool_bonus[1])
	{
		do_char_log(cn, 1, "%-12.12s  %+4d   %+4d\n",
		"Cooldown", it[in].cool_bonus[0], it[in].cool_bonus[1]);
	}
	
	if (it[in].light[0] || it[in].light[1])
	{
		do_char_log(cn, 1, "%-12.12s  %+4d   %+4d\n",
		"Glow", it[in].light[0], it[in].light[1]);
	}
	
	if (it[in].data[2] && !soulstone && !it[in].placement)
	{
		do_char_log(cn, 1, "%-12.12s  %+4d (%+4d/s)\n",
		"HP Regen", it[in].data[2], it[in].data[2]/max(1, it[in].duration/TICKS));
	}
	if (it[in].data[3] && !soulstone && !it[in].placement)
	{
		do_char_log(cn, 1, "%-12.12s  %+4d (%+4d/s)\n",
		"End Regen", it[in].data[3], it[in].data[3]/max(1, it[in].duration/TICKS));
	}
	if (it[in].data[4] && !soulstone && !it[in].placement)
	{
		do_char_log(cn, 1, "%-12.12s  %+4d (%+4d/s)\n",
		"Mana Regen", it[in].data[4], it[in].data[4]/max(1, it[in].duration/TICKS));
	}
	
	if (it[in].duration>0 && it[in].duration<3888000 && !soulstone && !it[in].placement)
	{
		do_char_log(cn, 1, "%-12.12s  %4d seconds\n",
		"Duration", it[in].duration/TICKS);
	}
	
	if (it[in].power && !soulstone)
	{
		do_char_log(cn, 1, "%-12.12s  %4d\n",
		"Complexity", it[in].power);
	}

	if (it[in].min_rank && !soulstone)
	{
		do_char_log(cn, 1, "%-12.12s  %4d\n",
		"Min. Rank", it[in].min_rank);
	}
}

void char_info(int cn, int co)
{
	int n, in, flag = 0, n1 = -1, n2 = -1;
	
	if (IS_PLAYER_COMP(co))
		do_char_log(cn, 1, "%s (power of %d (%d) +%d):\n", ch[co].name, ch[co].data[6], ch[co].data[7], ch[co].data[8]);
	else
		do_char_log(cn, 1, "%s:\n", ch[co].name);
	do_char_log(cn, 1, " \n");
	for (n = 0; n<MAXBUFFS; n++)
	{
		if ((in = ch[co].spell[n])!=0)
		{
			if (bu[in].data[1] && (ch[cn].flags & CF_GOD))
			{
				do_char_log(cn, 1, "%s for %dm %ds power of %d (%d)\n", 
					bu[in].name, bu[in].active / (TICKS * 60), (bu[in].active / TICKS) % 60, bu[in].power, bu[in].data[1]);
			}
			else
			{
				do_char_log(cn, 1, "%s for %dm %ds power of %d\n", bu[in].name, bu[in].active / (TICKS * 60), (bu[in].active / TICKS) % 60, bu[in].power);
				if ((ch[cn].flags & CF_KNOWSPELL) && !(ch[cn].flags & CF_KNOW_OFF))
				{
					switch (bu[in].temp)
					{
						case SK_LIGHT:
							do_char_log(cn, 6, " : %+d Glow\n", bu[in].light[1]); 
							break;
						case SK_MSHIELD: 
						case SK_PROTECT:
						case SK_WEAKEN2:
							do_char_log(cn, 6, " : %+d Armor Value\n", bu[in].armor[1]);
							break;
						case SK_ENHANCE: 
						case SK_WEAKEN:
							do_char_log(cn, 6, " : %+d Weapon Value\n", bu[in].weapon[1]);
							break;
						case SK_SLOW:
						case SK_SLOW2:
						case SK_HASTE:
							if (bu[in].speed[1])      do_char_log(cn, 6, " : %+d Speed\n",        bu[in].speed[1]);
							if (bu[in].atk_speed[1])  do_char_log(cn, 6, " : %+d Attack Speed\n", bu[in].atk_speed[1]);
							if (bu[in].cast_speed[1]) do_char_log(cn, 6, " : %+d Cast Speed\n",   bu[in].cast_speed[1]);
							break;
						case SK_CURSE:
						case SK_CURSE2:
						case SK_BLESS:
						case SK_WARCRY:
							do_char_log(cn, 6, " : %+d Braveness\n",  bu[in].attrib[0][1]); 
							do_char_log(cn, 6, " : %+d Willpower\n",  bu[in].attrib[1][1]); 
							do_char_log(cn, 6, " : %+d Intuition\n",  bu[in].attrib[2][1]); 
							do_char_log(cn, 6, " : %+d Agility\n",    bu[in].attrib[3][1]); 
							do_char_log(cn, 6, " : %+d Strength\n",   bu[in].attrib[4][1]);
							break;
						case SK_REGEN: // bu[in].hp[0]
							do_char_log(cn, 6, " : %+d.%02d HP/sec\n", bu[in].hp[0]/50, bu[in].hp[0]%50); break;
						case SK_ARIA:
						case SK_ARIA2:
							do_char_log(cn, 6, " : %+d Cooldown Rate\n", bu[in].cool_bonus[1]);
							if (bu[in].dmg_bonus[1])
								do_char_log(cn, 6, " : %+d.%02d%% Damage Dealt\n", bu[in].dmg_bonus[1]/20, bu[in].dmg_bonus[1]/2%10);
							break;
						case SK_WARCRY3:
						case SK_BLIND:
							do_char_log(cn, 6, " : %+d Hit Score\n",   bu[in].to_hit[1]); 
							do_char_log(cn, 6, " : %+d Parry Score\n", bu[in].to_parry[1]); 
							if (bu[in].skill[SK_PERCEPT][1])
								do_char_log(cn, 6, " : %+d Perception\n",  bu[in].skill[SK_PERCEPT][1]);
							break;
						case SK_DOUSE:
							do_char_log(cn, 6, " : %+d Spell Modifier\n", bu[in].spell_mod[1]);
							do_char_log(cn, 6, " : %+d Stealth\n",        bu[in].skill[SK_STEALTH][1]);
							break;
						case SK_MSHELL:
							do_char_log(cn, 6, " : %+d Resistance\n", bu[in].skill[SK_RESIST][1]);
							do_char_log(cn, 6, " : %+d Immunity\n",   bu[in].skill[SK_IMMUN][1]);
							break;
						case SK_GUARD:
						case SK_AGGRAVATE:
						case SK_SCORCH:
						case SK_SHOCK:
						case SK_CHARGE:
							if (bu[in].dmg_reduction[1]) 
								do_char_log(cn, 6, " : %+d.%02d%% Damage Taken\n", -1*bu[in].dmg_reduction[1]/2, (-100*bu[in].dmg_reduction[1]/2)%100);
							if (bu[in].dmg_bonus[1]) 
								do_char_log(cn, 6, " : %+d.%02d%% Damage Dealt\n", bu[in].dmg_bonus[1]/2, (100*bu[in].dmg_bonus[1]/2)%100);
							break;
						case SK_LETHARGY:
							if (bu[in].data[2])
								do_char_log(cn, 6, " : %+d Res Piercing\n", bu[in].power/4);
							else
								do_char_log(cn, 6, " : %+d Res Piercing\n", bu[in].power/3);
							break;
						case SK_RAGE:
							do_char_log(cn, 6, " : %+d Top Damage\n", bu[in].top_damage[1]);
							do_char_log(cn, 6, " : +%d.%02d%% DoT Dealt\n", 100*(2000+bu[in].data[4])/2000, (10000*(2000+bu[in].data[4])/2000)%100); break;
						case SK_CALM:
							do_char_log(cn, 6, " : %+d Top Damage Taken\n", bu[in].data[3]*-1);
							do_char_log(cn, 6, " : %d.%02d%% DoT Taken\n", 100*(2000-bu[in].data[4])/2000, (10000*(2000-bu[in].data[4])/2000)%100); break;
						case 254: // R/G/S Essence
							do_char_log(cn, 6, " : %+d to each attribute\n", bu[in].attrib[0][1]);
							if (bu[in].skill[SK_GEARMAST][1])
								do_char_log(cn, 6, " : %+d to some skills\n",    bu[in].skill[SK_GEARMAST][1]);
							break;
						case SK_WARCRY2:
							do_char_log(cn, 6, " : Stunned!\n");
							break;
						case SK_TAUNT:
							do_char_log(cn, 6, " : Taunted!\n");
							break;
						case SK_POISON:
						case SK_BLEED:
						case SK_PLAGUE:
						case SK_VENOM:
							break;
						default: break;
					}
				}
			}
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
		if ((B_SK(co, n) || IS_P_SKILL(n)) && n1==-1)
		{
			n1 = n;
		}
		else if ((B_SK(co, n) || IS_P_SKILL(n)) && n2==-1)
		{
			n2 = n;
		}

		if (n1!=-1 && n2!=-1)
		{
			do_char_log(cn, 1, 
			"%-12.12s %3d/%3d  !  %-12.12s %3d/%3d\n",
			skilltab[n1].name, B_SK(co, n1), M_SK(co, n1),
			skilltab[n2].name, B_SK(co, n2), M_SK(co, n2));
			n1 = -1;
			n2 = -1;
		}
	}

	if (n1!=-1)
	{
		do_char_log(cn, 1, 
		"%-12.12s %3d/%3d\n",
		skilltab[n1].name, B_SK(co, n1), M_SK(co, n1));
	}
	
	do_char_log(cn, 1, " \n");

	do_char_log(cn, 1, 
	"%-12.12s %3d/%3d  !  %-12.12s %3d/%3d\n",
	at_name[AT_BRV], B_AT(co, AT_BRV), M_AT(co, AT_BRV),
	at_name[AT_WIL], B_AT(co, AT_WIL), M_AT(co, AT_WIL));
	do_char_log(cn, 1, 
	"%-12.12s %3d/%3d  !  %-12.12s %3d/%3d\n",
	at_name[AT_INT], B_AT(co, AT_INT), M_AT(co, AT_INT),
	at_name[AT_AGL], B_AT(co, AT_AGL), M_AT(co, AT_AGL));
	do_char_log(cn, 1, 
	"%-12.12s %3d/%3d\n",
	at_name[AT_STR], B_AT(co, AT_STR), M_AT(co, AT_STR));
	
	do_char_log(cn, 1, " \n");
	
	do_char_log(cn, 1, 
	"%-12.12s     %3d  !  %-12.12s     %3d\n",
	"Hit Rate", ch[co].to_hit, "Parry Rate", ch[co].to_parry);
	do_char_log(cn, 1, 
	"%-12.12s     %3d  !  %-12.12s     %3d\n",
	"Weapon Value", ch[co].weapon, "Armor Value", ch[co].armor);

	do_char_log(cn, 1, " \n");
}

int spell_identify(int cn, int co, int in)
{
	char_play_sound(cn, ch[cn].sound + 1, -150, 0);
	chlog(cn, "Cast Identify");

	if (in)
	{
		item_info(cn, in, 0);
		it[in].flags ^= IF_IDENTIFIED;
		if (it[in].flags & IF_CORRUPTED)
		{
			do_char_log(cn, 0, "Corrupted.\n");
		}
		else
		{
			if (CAN_SOULSTONE(in))
			{
				do_char_log(cn, 3, "This item can be Soulstoned.\n");
			}
			else if (it[in].flags & IF_SOULSTONE)
			{
				if (it[in].driver==68)
					do_char_log(cn, 6, "Has been Catalyzed.\n");
				else
					do_char_log(cn, 7, "Has been Soulstoned.\n");
			}
			else if (it[in].placement && !CAN_SOULSTONE(in))
			{
				do_char_log(cn, 0, "Cannot be Soulstoned.\n");
			}
			if (CAN_ENCHANT(in))
			{
				do_char_log(cn, 3, "This item can be Enchanted.\n");
			}
			else if (it[in].flags & IF_ENCHANTED)
			{
				if (it[in].driver==68)
					do_char_log(cn, 7, "Has been Focused.\n");
				else
					do_char_log(cn, 8, "Has been Enchanted.\n");
			}
			else if (it[in].placement && !CAN_ENCHANT(in))
			{
				do_char_log(cn, 0, "Cannot be Enchanted.\n");
			}
			if (it[in].flags & IF_AUGMENTED)
			{
				do_char_log(cn, 7, "Has been augmented.\n");
			}
			if (it[in].flags & IF_EASEUSE)
			{
				do_char_log(cn, 5, "Was made easier to use.\n");
			}
			if (it[in].flags & IF_DIMINISHED)
			{
				do_char_log(cn, 5, "Has had its complexity reduced.\n");
			}
			if (it[in].flags & IF_SOULSPLIT)
			{
				do_char_log(cn, 5, "Has been soulsplit.\n");
			}
			if (it[in].flags & IF_DUPLICATED)
			{
				do_char_log(cn, 5, "Has been duplicated.\n");
			}
			if (it[in].flags & IF_LEGACY)
			{
				do_char_log(cn, 9, "Legacy item.\n");
			}
			if (!(it[in].flags & IF_IDENTIFIED))
			{
				do_char_log(cn, 1, "Identify data removed from item.\n");
			}
		}
	}
	else
	{
		char_info(cn, co);
		fx_add_effect(6, 0, ch[co].x, ch[co].y, 0);
	}
	
	fx_add_effect(7, 0, ch[cn].x, ch[cn].y, 0);
	
	return 1;
}
void skill_identify(int cn)
{
	int d20 = SP_MULT_IDENTIFY;
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
			power = get_target_resistance(cn, co);
		}
		else
		{
			co = cn;
			power = 10;
		}
		in = 0;
	}

	if (chance_base(cn, co, SK_IDENT, d20, power, 1, 1))
	{
		return;
	}
	
	spell_identify(cn, co, in);
	
	add_exhaust(cn, SK_EXH_IDENT);
}

int spell_scorch(int cn, int co, int power, int flag)
{
	int in;
	
	if (ch[co].escape_timer > TICKS*3) 						{ return 0; }
	if (ch[co].flags & CF_BODY) 							{ return 0; }
	if (ch[cn].attack_cn!=co && ch[co].alignment==10000) 	{ return 0; }
	if (ch[co].flags & CF_IMMORTAL) 						{ return 0; }
	
	power = spell_multiplier(power, cn);
	power = spell_immunity(power, get_target_immunity(cn, co));
	
	if (T_SEYA_SK(co, 10))				power = power*4/5;
	if (get_tarot(cn, IT_CH_CHARIO_R)) 	power = power*3/4;
	if (get_enchantment(co, 4)) 		power = power*4/5;
	
	if (!(in = make_new_buff(cn, SK_SCORCH, BUF_SPR_SCORCH, power, SP_DUR_SCORCH, 0)))
		return 0;
	
	bu[in].dmg_reduction[1] = -(20*2);	// 20% more damage taken
	bu[in].data[5] = 1;
	
	return cast_a_spell(cn, co, in, 1+flag, 1-flag); // SK_SCORCH
}
int spell_blast(int cn, int co, int power, int co_orig, int aoe)
{
	int hitpower, crit_dam=0, tmp, in;
	
	chlog(cn, "Cast Blast on %s", ch[co].name);
	
	hitpower = spell_immunity(power, get_target_immunity(cn, co)) * 2;
	if (co_orig) hitpower = hitpower/2 + hitpower/4;
	
	// Ira
	if (get_gear(cn, IT_TW_IRA))
		crit_dam = max(0, do_crit(cn, co, hitpower, 1)/2);
	
	// Tarot Card - Judgment :: Less damage
	if (get_tarot(cn, IT_CH_JUDGE))
		tmp = do_hurt(cn, co, (hitpower+crit_dam)*85/100, 1);
	else
		tmp = do_hurt(cn, co, hitpower+crit_dam, 1);
	
	if (aoe)
	{
		if (tmp>0)
		{
			if (!(ch[co].flags & CF_SYS_OFF))
				do_char_log(co, 1, "%s blasted you for %d HP.\n", ch[cn].name, tmp);
		}
	}
	else
	{
		if (tmp<1)
		{
			do_char_log(cn, 0, "You cannot penetrate %s's armor.\n", ch[co].reference);
		}
		else
		{
			if (!(ch[cn].flags & CF_SYS_OFF))
				do_char_log(cn, 1, "You blast %s for %d HP.\n", ch[co].reference, tmp);
			if (!(ch[co].flags & CF_SYS_OFF))
				do_char_log(co, 1, "%s blasted you for %d HP.\n", ch[cn].name, tmp);
		}
	}
	
	if (in = has_buff(co, SK_PLAGUE)) speed_plague(co, in);
	
	if (!co_orig)
	{
		char_play_sound(co, ch[cn].sound + 6, -150, 0);
		do_area_sound(co, 0, ch[co].x, ch[co].y, ch[cn].sound + 6);
	}
	fx_add_effect(5, 0, ch[co].x, ch[co].y, 0);
	
	// Tarot Card - Judgment :: Inflict Scorch
	if (get_tarot(cn, IT_CH_JUDGE)) 
		spell_scorch(cn, co, hitpower, 0);
	
	return 1+tmp;
}
void skill_blast(int cn)
{
	int dr1 = RANDOM(GLVDICE), dr2 = RANDOM(GLVDICE);
	int power, aoe_power, cost, exhst = SK_EXH_BLAST;
	int count = 0, hit = 0;
	int co = 0, co_orig = 0;
	int avgdmg = 0;
	int can_aoe = CAN_ARHR_PROX(cn);
	
	power = M_SK(cn, SK_BLAST);
	power = spell_multiplier(power, cn);
	aoe_power = M_SK(cn, SK_PROX);
	cost = (power * 2) / 8 + 5;
	
	// Harakim & Sorc costs less, monster cost more mana
	if (IS_PLAYER(cn) && (IS_ANY_HARA(cn) || IS_SORCERER(cn) || IS_BRAVER(cn) || IS_LYCANTH(cn)))
		cost = cost*2/5;
	if (IS_PLAYER_COMP(cn))
		cost = 20;
	
	// Get spell target - return on failure
	if (!(co = get_target(cn, 0, 0, 0, cost, SK_BLAST, 1, power, 0)))
		return;
	
	// If we have a valid target, cast Blast on them
	if (cn!=co && co!=ch[cn].data[PCD_SHADOWCOPY] && co!=ch[cn].data[PCD_COMPANION])
	{
		avgdmg = spell_blast(cn, co, power, 0, 0);
		
		co_orig = co;
		count++;
		hit++;
		
		check_gloves(cn, co, 0, dr1, dr2);
	}
	
	// Cast AoE or general surround-hit
	if (can_aoe)
	{
		if ((hit = cast_aoe_spell(cn, co, SK_BLAST, power, aoe_power, cost, count, hit, avgdmg, dr1, dr2)) < 0)
			return;
		
		fx_add_effect(7, 0, ch[cn].x, ch[cn].y, 0);
	}
	else
	{
		hit += surround_cast(cn, co_orig, 0, SK_BLAST, power, dr1, dr2);
	}
	
	// Tree - cooldown reduced by # hit
	if (T_ARHR_SK(cn, 4))
		exhst = max(SK_EXH_BLAST/4, exhst - hit*5);
	
	add_exhaust(cn, exhst);
}

int item_repair(int cn, int in, int power, int n, int flag)
{
	int die, chance, orgt, in2;
	
	power = skill_multiplier(power, cn);
	
	if (power>M_SK(cn, SK_REPAIR))
	{
		if (!flag) do_char_log(cn, 0, "That's too difficult for you (requires %d Repair).\n", power);
		return 0;
	}
	if (ch[cn].a_end<power * 200)
	{
		if (!flag) do_char_log(cn, 0, "You're too exhausted to repair that.\n");
		return 0;
	}
	ch[cn].a_end -= power * 200;

	if (power)		chance = M_SK(cn, SK_REPAIR) * 15 / power;
	else			chance = 19;
	if (chance< 0)	chance =  0;
	if (chance>19)	chance = 19;

	die = RANDOM(20);

	if (die<=chance)
	{
		// Repair - option 1: reset values of the item to their originals.
		// This option is for soulstone items, to allow repairing them without a template.
		if (IS_SANEITEMPLATE(it[in].orig_temp))
		{
			orgt = it[in].orig_temp;
			
			it[in].flags |= IF_UPDATE;
			
			it[in].current_damage = 0;
			it[in].current_age[0] = 0;
			it[in].current_age[1] = 0;
			it[in].damage_state = 0;
			
			it[in].armor[0] = it_temp[orgt].armor[0] + it[in].ss_armor;
			it[in].armor[1] = it_temp[orgt].armor[1];
			
			it[in].weapon[0] = it_temp[orgt].weapon[0] + it[in].ss_weapon;
			it[in].weapon[1] = it_temp[orgt].weapon[1];
			
			it[in].sprite[0] = it_temp[orgt].sprite[0];
			it[in].sprite[1] = it_temp[orgt].sprite[1];
		}
		// Repair - option 2: just make a new item
		else
		{
			in2 = god_create_item(it[in].temp);
			if (!in2)
			{
				if (!flag) do_char_log(cn, 0, "You failed.\n");
				return 0;
			}
			if (it[in].flags & IF_IDENTIFIED) it[in2].flags |= IF_IDENTIFIED;
			it[in].used  = USE_EMPTY;
			if (n<0) 			ch[cn].citem 		= in2;
			else if (flag==3)	ch[cn].alt_worn[n] 	= in2;
			else if (flag==2)	ch[cn].worn[n] 		= in2;
			else				ch[cn].item[n] 		= in2;
			it[in2].carried = cn;
		}
		if (!flag) do_char_log(cn, 1, "Success!\n");
		return 1;
	}
	else
	{
		if (!flag) 
		{
			do_char_log(cn, 0, "You failed.\n");
			item_damage_citem(cn, 1000000);
		}
		else if (n>=0)
		{
			if (flag==3)
			{
				item_damage_alt_worn(cn, n, 1000000);
			}
			else if (flag==2)
			{
				item_damage_worn(cn, n, 1000000);
			}
			else
			{
				item_damage_held(cn, n, 1000000);
			}
		}
		return -1;
	}
}
void skill_repair(int cn)
{
	int in, n, c = 0, j = 0, k = 0, r = 0;

	if ((in = ch[cn].citem)==0)
	{
		//do_char_log(cn, 0, "Repair. Repair what?\n");
		for (n=0;n<12;n++)
		{
			if ((in = ch[cn].worn[n])==0) continue;
			if (it[in].flags & IF_NOREPAIR) continue;
			if (it[in].damage_state)
			{
				r = item_repair(cn, in, it[in].power, n, 2);
				if (r ==  1) j++;
				if (r == -1) k++;
				c++;
			}
		}
		for (n=0;n<MAXITEMS;n++)
		{
			if ((in = ch[cn].item[n])==0) continue;
			if (it[in].flags & IF_NOREPAIR) continue;
			if (it[in].damage_state)
			{
				r = item_repair(cn, in, it[in].power, n, 1);
				if (r ==  1) j++;
				if (r == -1) k++;
				c++;
			}
		}
		for (n=0;n<12;n++)
		{
			if ((in = ch[cn].alt_worn[n])==0) continue;
			if (it[in].flags & IF_NOREPAIR) continue;
			if (it[in].damage_state)
			{
				r = item_repair(cn, in, it[in].power, n, 3);
				if (r ==  1) j++;
				if (r == -1) k++;
				c++;
			}
		}
		
		if (j)			do_char_log(cn, 1, "You repaired %d of %d items.\n", j, c);
		else if (c)	do_char_log(cn, 0, "You couldn't repair anything.\n");
		else			do_char_log(cn, 1, "Nothing repairable is damaged.\n");
		if (k)			do_char_log(cn, 0, "You damaged %d of your items.\n", k);
		
		chlog(cn, "Cast Repair-all");
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
	
	item_repair(cn, in, it[in].power, -1, 0);
	
	chlog(cn, "Cast Repair");
}

void skill_recall(int cn)
{
	int in, power;

	if (is_exhausted(cn)) { return; }
	if (spellcost(cn, SP_COST_RECALL, SK_RECALL, 1)) { return; }
	if (chance(cn, FIVE_PERC_FAIL)) { return; }
	
	power = M_SK(cn, SK_RECALL);
	
	if (!(in = make_new_buff(cn, SK_RECALL, BUF_SPR_RECALL, power, SP_DUR_RECALL(power), 0)))
		return;
	
	bu[in].data[1] = ch[cn].temple_x;
	bu[in].data[2] = ch[cn].temple_y;
	
	if (!cast_a_spell(cn, cn, in, 0, 0)) // SK_RECALL
		return;
	
	add_exhaust(cn, SK_EXH_RECALL);
}

void remove_shadow(int cn)
{
	int in, n;

	for (n = 0; n<MAXBUFFS; n++)
	{
		if ((in = ch[cn].spell[n])==0 || bu[in].temp!=SK_SHADOW)	continue;
		bu[in].used = USE_EMPTY;
		ch[cn].spell[n] = 0;
	}
	do_update_char(cn);
}

void remove_spells(int cn) // Handles No-Magic-Zones, not Dispel
{
	int in, n;

	for (n = 0; n<MAXBUFFS; n++)
	{
		if ((in = ch[cn].spell[n])==0)	continue;
		if (bu[in].temp == 1)			continue;
		if (bu[in].temp == SK_DIVINITY) continue;
		if (bu[in].data[4] == 1)		continue; // Effects not removed by NMZ
		bu[in].used = USE_EMPTY;
		ch[cn].spell[n] = 0;
	}
	do_update_char(cn);
}

void remove_all_spells(int cn, int flag) // Card turn-ins & Lycan Shifting
{
	int in, n;

	for (n = 0; n<MAXBUFFS; n++)
	{
		if ((in = ch[cn].spell[n])==0)	continue;
		if (bu[in].temp == SK_SHIFT)	continue;
		if (flag)
		{
			if (bu[in].temp == SK_LIGHT)    continue;
			if (bu[in].temp == SK_BLESS)    continue;
			if (bu[in].temp == SK_ENHANCE)  continue;
			if (bu[in].temp == SK_PROTECT)  continue;
			if (bu[in].temp == SK_HASTE)    continue;
			if (bu[in].temp == SK_HEAL)     continue;
			if (bu[in].temp == SK_REGEN)    continue;
			if (bu[in].temp == SK_MSHIELD)  continue;
			if (bu[in].temp == SK_CHARGE)   continue;
			if (bu[in].temp == SK_WARCRY3)  continue;
			if (bu[in].temp == SK_DIVINITY) continue;
			if (bu[in].temp == SK_RAGE) 	continue;
			if (bu[in].temp == SK_CALM) 	continue;
			if (bu[in].temp == 635) continue; // Infrared 1
			if (bu[in].temp == 637) continue; // Infrared 2
			if (bu[in].temp == 639) continue; // Infrared 3
			if (bu[in].temp == 641) continue; // Infrared 4
			if (bu[in].temp == 205) continue; // Coconut
			if (bu[in].temp == 206) continue; // Heatstroke
			if (bu[in].temp == 215) continue; // Vantablack
			if (bu[in].temp == SK_SANGUINE) continue; // Sanguine
			if (bu[in].temp >= 106 && bu[in].temp <= 109) continue; // contract debuffs
		}
		if (bu[in].temp == 103)			continue; // Don't remove food items
		if (bu[in].temp == 254)			continue; // Don't remove rpot/gpot/spot
		bu[in].used = USE_EMPTY;
		ch[cn].spell[n] = 0;
	}
	do_update_char(cn);
}

#define DISPEL_MAX		18
#define DISPEL_STORE	 3

int spell_dispel(int cn, int co, int power, int sto[DISPEL_STORE], int flag)
{
	int in;
	
	// spell_multiplier is already done in skill_dispel
	
	if (flag)	// Debuff version
	{
		if (ch[cn].attack_cn!=co && ch[co].alignment==10000) { return 0; }
		if (ch[co].flags & CF_IMMORTAL) { return 0; }
		
		power = spell_immunity(power, get_target_immunity(cn, co));
		
		if (T_SEYA_SK(co, 10))				power = power*4/5;
		if (get_tarot(cn, IT_CH_CHARIO_R)) 	power = power*3/4;
		if (get_enchantment(co, 4)) 		power = power*4/5;
		
		if (!(in = make_new_buff(cn, SK_DISPEL2, BUF_SPR_INNOCU, power, SP_DUR_DISPEL(power/4), 0))) 
			return 0;
		
		bu[in].data[5] = 1;
	}
	else		// Buff version
	{
		power = spellpower_check(cn, co, power, 0);
		
		if (!(in = make_new_buff(cn, SK_DISPEL, BUF_SPR_IMMUNI, power, SP_DUR_DISPEL(power), 0))) 
			return 0;
	}
	
	//chlog(cn, "Immunizing against %d, %d, %d", sto1, sto2, sto3);
	
	bu[in].data[1] = sto[0];
	bu[in].data[2] = sto[1];
	bu[in].data[3] = sto[2];
	
	return cast_a_spell(cn, co, in, flag, 1-flag); // SK_DISPEL / SK_DISPEL2
}
void skill_dispel(int cn)
{
	int in, co, n, m, base_pow, power, ail_pow = 0, success = 0, chanc, flag, verd, ismon=1;
	int ail[DISPEL_MAX] = {0};
	int d20 = SP_MULT_DISPEL;
	int sto[DISPEL_STORE] = {0};

	if ((co = ch[cn].skill_target1)) { ; }
	else { co = cn; }
	
	if (!do_char_can_see(cn, co, 0))
	{
		do_char_log(cn, 0, "You cannot see your target.\n");
		return;
	}

	if (is_exhausted(cn)) { return; }
	if (spellcost(cn, SP_COST_DISPEL, SK_DISPEL, 1)) { return; }
	
	// Tarot Card - Hierophant :: Dispel removes positive spells instead of negative spells
	flag = get_tarot(cn, IT_CH_HEIROPH);
	verd = get_book(cn, IT_BOOK_VERD);
	
	if (cn!=co)
	{
		if (flag)
		{
			remember_pvp(cn, co);
			if (!may_attack_msg(cn, co, 1))
			{
				chlog(cn, "Prevented from attacking %s (%d)", ch[co].name, co);
				return;
			}
		}
		else if (!player_or_ghost(cn, co))
		{
			do_char_log(cn, 0, "Changed target of spell from %s to %s.\n", ch[co].name, ch[cn].name);
			co = cn;
		}
	}
	
	if (IS_PLAYER(co) || IS_PLAYER_COMP(co)) ismon = 0;
	
	// initialize these arrays
	for (n=0;n<DISPEL_MAX;n++)   ail[n] = -1;
	for (n=0;n<DISPEL_STORE;n++) sto[n] =  0;
	
	base_pow = power = spell_multiplier(M_SK(cn, SK_DISPEL), cn);
	
	if (flag)
	{
		d20 = SP_MULT_DISPEL2;
		// Remove each positive spell in sequence, from most to least expensive
		for (n = 0; n<MAXBUFFS; n++)
		{
			if ((in = ch[co].spell[n])==0) { continue; }
			if (bu[in].active<=1)          { continue; }
			if (bu[in].temp==SK_DIVINITY)  { continue; }
			
			if (bu[in].temp==SK_HASTE)		ail[0] = n;
			if (bu[in].temp==SK_BLESS) 		ail[1] = n;
			if (bu[in].temp==SK_MSHIELD) 	ail[2] = n;
			if (bu[in].temp==SK_MSHELL) 	ail[3] = n;
			if (bu[in].temp==SK_PULSE) 		ail[4] = n;
			if (bu[in].temp==SK_ZEPHYR) 	ail[5] = n;
			if (bu[in].temp==SK_GUARD) 		ail[6] = n;
			if (bu[in].temp==SK_WARCRY3) 	ail[7] = n;
			if (bu[in].temp==SK_CHARGE) 	ail[8] = n;
			if (bu[in].temp==SK_DISPEL) 	ail[9] = n;
			if (bu[in].temp==SK_REGEN) 		ail[10] = n;
			if (bu[in].temp==SK_PROTECT) 	ail[11] = n;
			if (bu[in].temp==SK_ENHANCE) 	ail[12] = n;
			if (bu[in].temp==SK_LIGHT) 		ail[13] = n;
		}
	}
	else
	{
		// Remove each negative spell in sequence, from most to least imperative
		for (n = 0; n<MAXBUFFS; n++)
		{
			if ((in = ch[co].spell[n])==0) { continue; }
			if (bu[in].active<=1)          { continue; }
			if (bu[in].temp==SK_DIVINITY)  { continue; }
			
			if (!ismon && bu[in].temp==SK_VENOM)	ail[0] = n;
			if (!ismon && bu[in].temp==SK_POISON)	ail[1] = n;
			if (!ismon && bu[in].temp==SK_BLEED) 	ail[2] = n;
			if (!ismon && bu[in].temp==SK_FROSTB) 	ail[3] = n;
			if (bu[in].temp==SK_BLIND) 				ail[4] = n;
			if (bu[in].temp==SK_WARCRY2) 			ail[5] = n;
			if (bu[in].temp==SK_CURSE2) 			ail[6] = n;
			if (bu[in].temp==SK_CURSE) 				ail[7] = n;
			if (bu[in].temp==SK_WARCRY) 			ail[8] = n;
			if (bu[in].temp==SK_WEAKEN2) 			ail[9] = n;
			if (bu[in].temp==SK_WEAKEN) 			ail[10] = n;
			if (bu[in].temp==SK_SLOW2) 				ail[11] = n;
			if (bu[in].temp==SK_SLOW) 				ail[12] = n;
			if (bu[in].temp==SK_DOUSE) 				ail[13] = n;
			if (bu[in].temp==SK_AGGRAVATE) 			ail[14] = n;
			if (bu[in].temp==SK_SCORCH) 			ail[15] = n;
			if (bu[in].temp==SK_SHOCK) 				ail[16] = n;
			if (bu[in].temp==SK_DISPEL2) 			ail[17] = n;
		}
	}
	for (m = 0; m<DISPEL_MAX; m++) if (ail[m]>-1)
	{
		in = ch[co].spell[ail[m]];
		ail_pow = bu[in].power;
		if (flag)
		{
			ail_pow = max(ail_pow, get_target_resistance(cn, co));
		}
		chanc = d20 * power / max(1, ail_pow*10); if (chanc > 18) chanc = 18;
		if (chanc<RANDOM(20))
		{
			if (!success)
			{
				do_char_log(cn, 0, "Your spell fizzled.\n");
				if (cn!=co && CAN_SENSE(co) && M_SK(co, SK_PERCEPT)>M_SK(cn, SK_DISPEL) + 5)
				{
					if (!(ch[co].flags & CF_SENSEOFF))
						do_char_log(co, 0, "%s tried to cast dispel magic on you but failed.\n", ch[cn].reference);
				}
				return;
			}
			else break;
		}
		
		if (!verd) sto[success] = bu[in].temp;
		
		if (co!=cn)
		{
			bu[in].used = USE_EMPTY; 
			ch[co].spell[ail[m]] = 0;
			if (flag) remember_pvp(cn,co);
			if (CAN_SENSE(co) && M_SK(co, SK_PERCEPT) + 10>M_SK(cn, SK_DISPEL))
				do_char_log(co, 0, "%s has been dispelled by %s.\n", bu[in].name, ch[cn].reference);
			else
				do_char_log(co, 0, "%s has been dispelled.\n", bu[in].name);
			do_char_log(cn, 1, "Dispelled %s from %s.\n", bu[in].name, ch[co].name);
			if (flag && !IS_PLAYER(co)) 
			{
				do_area_notify(cn, co, ch[cn].x, ch[cn].y, NT_SEEHIT, cn, co, 0, 0);
				if (!IS_IGNORING_SPELLS(co)) 
					do_notify_char(co,NT_GOTHIT,cn,0,0,0);
				do_notify_char(cn,NT_DIDHIT,co,0,0,0);
            }
		}
		else
		{
			bu[in].used = USE_EMPTY; 
			ch[cn].spell[ail[m]] = 0;
			do_char_log(cn, 1, "%s has been removed.\n", bu[in].name);
		}
		
		success++;
		
		if (verd) 
		{
			power -= ail_pow/4;
			continue;
		}
		
		power -= ail_pow/2;
		
		if (power < 1) break;
		if (success >= DISPEL_STORE || flag) break;
	}
	
	do_update_char(co);
	
	if (!success && n==MAXBUFFS)
	{
		if (flag)
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
	else
	{
		spell_dispel(cn, co, base_pow, sto, flag);
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

int skill_gc_atk(int cn, int co, int cz)
{
	int idx;
	
	if (cz == cn)
	{
		do_sayx(co, "But %s, I would never attack you!", ch[cn].name);
		return 0;
	}
	if (cz == co)
	{
		do_sayx(co, "You want me to attack myself? That's silly, %s!", ch[cn].name);
		return 0;
	}
	if (!may_attack_msg(cn, cz, 0))
	{
		do_sayx(co, "The Gods would be angry if we did that, you didn't want to anger the Gods, %s did you?", ch[cn].name);
		return 0;
	}
	ch[co].attack_cn = cz;
	idx = cz | (char_id(cz) << 16);
	ch[co].data[MCD_ENEMY1ST] = idx;
	do_sayx(co, "Yes %s, I will kill %s!", ch[cn].name, ch[cz].reference);
	do_notify_char(cz, NT_GOTMISS, cn, 0, 0, 0);
	return 1;
}

void skill_ghost(int cn)
{
	int co, cc, cz, n, m, z, base = 0, tmp, pts = 0, idx;
	int archgc = 0, archbonus = 0, archtmp = 0, dreadplate = 0, necronomicon = 0, invidia = 0;

	if (IS_BUILDING(cn))
	{
		do_char_log(cn, 0, "Not in build mode.\n");
		return;
	}

	if (IS_PLAYER(cn) && (co = ch[cn].data[PCD_COMPANION]))
	{
		if (!IS_SANECHAR(co) || ch[co].data[CHD_MASTER]!=cn || 
			(ch[co].flags & CF_BODY) || ch[co].used==USE_EMPTY)
		{
			co = 0;
		}
		if (co && !IS_SHADOW(co))
		{
			if ((cz = ch[cn].skill_target1) && do_char_can_see(cn, cz, 0))
			{
				if (cz == co)
				{
					do_char_log(cn, 1, "You dismissed your Ghost Companion.\n", co);
					answer_transfer(co, cn, 0);
				}
				else
				{
					chlog(cn, "Commands attack against %s (%d)", ch[cz].name, cz);
					skill_gc_atk(cn, co, cz);
				}
			}
			else
			{
				do_char_log(cn, 5, "You recalled your Ghost Companion.\n");
				quick_teleport(co, ch[cn].x, ch[cn].y);
				add_exhaust(cn, TICKS*2);
			}
			return;
		}
	}
	
	co = ch[cn].skill_target1;
	
	if (cn==co) co = 0;
	if (co==ch[cn].data[PCD_SHADOWCOPY] || 
		co==ch[cn].data[PCD_COMPANION] ) co = 0;

	if (co && !do_char_can_see(cn, co, 0))
	{
		do_char_log(cn, 0, "You cannot see your target.\n");
		return;
	}

	if (is_exhausted(cn)) return;

	if (co && !may_attack_msg(cn, co, 1))
	{
		chlog(cn, "Prevented from attacking %s (%d)", ch[co].name, co);
		return;
	}
	
	/* No GC in Gatekeeper's room */
	if (ch[cn].x >= 137 && ch[cn].x <= 147 && 
		ch[cn].y >= 182 && ch[cn].y <= 192 )
	{
		do_char_log(cn, 0, "You must fight this battle alone.\n");
		return;
	}

	if (spellcost(cn, SP_COST_GHOST, SK_GHOST, 1)) return;

	if (chance(cn, TEN_PERC_FAIL))
	{
		if (co && cn!=co) 
			do_area_notify(cn, co, ch[cn].x, ch[cn].y, 
				NT_SEEMISS, cn, co, M_SK(cn, SK_GHOST), 0);
		if (co && cn!=co && CAN_SENSE(co) && 
			M_SK(co, SK_PERCEPT)>M_SK(cn, SK_GHOST) + 5)
		{
			if (!(ch[co].flags & CF_SENSEOFF))
				do_char_log(co, 0, "%s tried to cast ghost companion on you but failed.\n", 
					ch[cn].reference);
			if (!IS_IGNORING_SPELLS(co))
				do_notify_char(co, NT_GOTMISS, cn, 0, 0, 0);
		}
		return;
	}
	
	if (get_gear(cn, IT_TW_DREAD)) dreadplate = 1;
	if (get_book(cn, IT_BOOK_NECR)) necronomicon = 1;
	if (get_gear(cn, IT_TW_INVIDIA)) invidia = 1;
	
	archgc 					= B_SK(cn, SK_GCMASTERY);
	if (archgc) archbonus 	= M_SK(cn, SK_GCMASTERY);
	if (IS_PLAYER(cn) && !IS_SUMMONER(cn)) archbonus /= 2;
	
	if (dreadplate) // Special GC for Tower item
	{	
		if (archgc) cz = CT_ARCHCASTER; else cz = CT_CASTERCOMP; 
	}
	else // Better GC for Summoner, or normal GC otherwise
	{	
		if (archgc) cz = CT_ARCHCOMP; else cz = CT_COMPANION; 
	}
	
	cc = god_create_char(cz, 1); 
	
	if (!god_drop_char_fuzzy(cc, ch[cn].x, ch[cn].y))
	{
		ch[cc].used = USE_EMPTY;
		do_char_log(cn, 0, "The ghost companion could not materialize.\n");
		return;
	}

	if (co)
	{
		if (cn!=co) 
			do_area_notify(cn, co, ch[cn].x, ch[cn].y, 
				NT_SEEHIT, cn, co, 0, 0);
		if (!IS_IGNORING_SPELLS(co)) 
			do_notify_char(co, NT_GOTHIT, cn, 0, 0, 0);
		do_notify_char(cn, NT_DIDHIT, co, 0, 0, 0);
	}
	
	if (IS_PLAYER(cn)) ch[cn].data[PCD_COMPANION] = cc;
	
	// base is the power of the summon
	base = spell_multiplier(M_SK(cn, SK_GHOST), cn);
	ch[cc].data[6] = base;
	if (IS_MONSTER(cn)) 
		base = base * 4 / 11;
	else 
		base = base * 5 / 11;
	ch[cc].data[7] = base;
	ch[cc].data[8] = archbonus;

	ch[cc].data[29] = 0;					// reset experience earned
	ch[cc].data[CHD_GROUP] = 65536 + cn;	// set group
	ch[cc].kindred &= ~(KIN_MONSTER);		// Remove 'Monster' flag
	
	if (IS_MONSTER(cn)) ch[cc].kindred |= KIN_MONSTER;
	
	strcpy(ch[cc].text[0], "#14#Yes! %s buys the farm!");
	strcpy(ch[cc].text[1], "#13#Yahoo! An enemy! Prepare to die, %s!");
	strcpy(ch[cc].text[3], "My successor will avenge me, %s!");
	
	if (invidia)
		ch[cc].kindred |= KIN_ARCHTEMPLAR;
	
	if (co)
	{
		ch[cc].attack_cn = co;
		idx = co | (char_id(co) << 16);
		ch[cc].data[MCD_ENEMY1ST] = idx;            // add enemy to kill list
	}
	ch[cc].data[CHD_MASTER] = ch[cc].data[69] = cn;
	
	// Set duration of GC -- Forever for players, 5 minutes for NPCs
	if (IS_PLAYER(cn)) ch[cc].data[PCD_COMPANION] = 0;
	else 
	{ 
		ch[cc].data[PCD_COMPANION] = globs->ticker + TICKS * 60 * 5;
		ch[cc].data[CHD_GROUP] = ch[cn].data[CHD_GROUP];
		ch[cc].data[59] = ch[cn].data[CHD_GROUP];
	}
	ch[cc].data[MCD_TALKATIVE] = ch_temp[cz].data[MCD_TALKATIVE];
	ch[cc].data[98]            = globs->ticker + COMPANION_TIMEOUT;
	
	if (ch[cn].flags & CF_SILENCE) 
	{
		ch[cc].data[MCD_TALKATIVE] = 0;
		ch[cc].flags |= CF_SILENCE;
	}
	if (ch[cn].flags & CF_INFRARED) ch[cc].flags |= CF_INFRARED;
	if (ch[cn].flags & CF_IGN_SB)   ch[cc].flags |= CF_IGN_SB;
	
	ch[cc].data[48] = 33;
	ch[cc].data[1] = ch[cn].gcm;	// set GC mode
	
	//{
	
	for (n = 0; n<5; n++)
	{
		tmp = base;
		tmp = tmp * 5 / max(1, ch[cc].attrib[n][3]);
		B_AT(cc, n) = max(10, min(ch[cc].attrib[n][2]+(archbonus-n*2)/10, tmp));
	}
	
	for (n = 0; n<MAXSKILL; n++)
	{
		tmp = base;
		tmp = tmp * 5 / max(1, ch[cc].skill[n][3]);
		if (ch[cc].skill[n][2])
		{
			if (n==0)
			{
				B_SK(cc, n) = min(ch[cc].skill[n][2]+archbonus*3/20, tmp);
			}
			else
			{
				B_SK(cc, n) = min(ch[cc].skill[n][2]+(archbonus-archtmp)/10, tmp);
				archtmp++;
			}
		}
	}
	
	// Tree - Seyan GC/SC learns Regen
	if (T_SEYA_SK(cn, 9))
	{
		tmp = base;
		tmp = tmp * 5 / 8;
		B_SK(cc, SK_HEAL) = min(60, tmp);
		tmp = god_create_item(IT_CH_STAR);
		ch[cc].worn[WN_CHARM] = tmp;
		it[tmp].carried = cc;
	}
	
	if (necronomicon)
	{
		if (dreadplate) // caster ~ becomes ghost
		{
			B_SK(cc, SK_POISON) = B_SK(cc, SK_RESIST);
			ch[cc].sprite = 9168;
		}
		else			// fighter ~ becomes skeleton
		{
			B_SK(cc, SK_CLEAVE) = B_SK(cc, SK_RESIST);
			ch[cc].sprite = 10192;
		}
		ch[cc].data[1] = 3; // BERSERK!!
	}
	
	// sprite setup
	if (IS_PLAYER(cn) && ch[cn].class > 0) 
		ch[cc].sprite = ch[cn].class;

	ch[cc].hp[0]       = max(50, min(ch[cc].hp[2],   base * 5));
	ch[cc].end[0]      = max(100,min(ch[cc].end[2],  base * 3));
	if (archgc||dreadplate)
		ch[cc].mana[0] = max(50, min(ch[cc].mana[2], base * 4));
	else
		ch[cc].mana[0] = 0;

	// calculate experience
	for (z = 0; z<5; z++) for (m = 10; m<B_AT(cc, z); m++)
		pts += attrib_needed(m, 3);
	
	for (m = 50; m<ch[cc].hp[0]; m++)
		pts += hp_needed(m, 3);

	for (m = 50; m<ch[cc].mana[0]; m++)
		pts += mana_needed(m, 3);

	for (z = 0; z<MAXSKILL; z++) for (m = 1; m<B_SK(cc, z); m++)
		pts += skill_needed(m, 3);
	//
	
	if (dreadplate)
	{
		ch[cc].weapon_bonus = max( 8, 
			                  min(54+(archbonus)/20, base*5/7+8));
		ch[cc].armor_bonus  = max(12, 
			                  min(60+(archbonus+10)/20, base*5/6+6));
	}
	else
	{
		ch[cc].weapon_bonus = max(12, 
			                  min(80+(archbonus)/20, base*5/5+6));
		ch[cc].armor_bonus  = max(8, 
			                  min(72+(archbonus+10)/20, base*5/6+8));
	}

	ch[cc].points_tot = pts;
	ch[cc].gold       = 0;
	ch[cc].a_hp       = 9999999;
	ch[cc].a_end      = 999999;
	ch[cc].a_mana     = 999999;
	ch[cc].alignment  = ch[cn].alignment / 2;
	//}
	
	xlog("Created %s (%d) with base %d as Ghost Companion for %s (group %d)", ch[cc].name, cc, base, ch[cn].reference, ch[cc].data[CHD_GROUP]);

	/* CS, 000109: Less chatty GC */
	if (co)
	{
		do_sayx(cc, ch[cc].text[1], ch[co].name);
	}
	else if (getrank(cc) < 6)     // GC not yet Master Sergeant
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
		
		if (necronomicon) tmp = RANDOM(20);
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

int spell_shadow(int cn, int co, int cz, int archbonus, int dont_atk)
{
	int cc, n, m, z, power, base = 1, basedur = 1, idx, in, tmp, pts = 0, sprite = 2000;
	int archtmp = 0, w = 0, necronomicon = 0, devilR = 0;
	
	cc = god_create_char(CT_COMPANION, 1); 
	
	if (!god_drop_char_fuzzy(cc, ch[cz].x, ch[cz].y))
	{
		if (!god_drop_char_fuzzy(cc, ch[cn].x, ch[cn].y))
		{
			ch[cc].used = USE_EMPTY;
			do_char_log(cn, 0, "The shadow copy could not materialize.\n");
			return 0;
		}
	}

	if (co && !dont_atk)
	{
		if (cn!=co) 
			do_area_notify(cn, co, ch[cn].x, ch[cn].y, 
				NT_SEEHIT, cn, co, 0, 0);
		if (!IS_IGNORING_SPELLS(co)) 
			do_notify_char(co, NT_GOTHIT, cn, 0, 0, 0);
		do_notify_char(cn, NT_DIDHIT, co, 0, 0, 0);
	}
	
	if (get_book(cn, IT_BOOK_NECR)) necronomicon = 1;
	if (get_tarot(cn, IT_CH_DEVIL_R)) devilR = 1;
	
	if (IS_PLAYER(cn)) ch[cn].data[PCD_SHADOWCOPY] = cc;
	
	power = M_SK(cn, SK_SHADOW);
	if (T_SUMM_SK(cn, 9)) power = power + (power * M_AT(cn, AT_WIL)/1000);
	// basedur is how long the copy lasts; base is the power of the summon
	basedur = base = spell_multiplier(power, cn);
	ch[cc].data[6] = base;
	base = base * 5 / 11;
	ch[cc].data[7] = base;
	ch[cc].data[8] = archbonus;
	
	ch[cc].data[29] = 0;					// reset experience earned
	ch[cc].data[CHD_GROUP] = 65536 + cn;	// set group
	ch[cc].kindred &= ~(KIN_MONSTER);		// Remove 'monster' flag
	
	if (IS_MONSTER(cn)) ch[cc].kindred |= KIN_MONSTER;
	
	// Adjust to match the caster
	if (necronomicon)
	{
		ch[cc].flags |= CF_UNDEAD;
		sprite = ch[cc].sprite = 19408;
	}
	else
	{
		sprite = ch[cc].sprite = ch[cz].sprite;  // ch[cc].sprite = 15312;
		if (ch[cz].flags & CF_SIMPLE) 	// Set 'simple' animation flag for Flames and Spiders.
			ch[cc].flags |= CF_SIMPLE;
	}
	//sprintf(ch[cc].reference, "%s", ch[cc].name);
	strcpy(ch[cc].text[0], "#14#%s has been eliminated.");
	strcpy(ch[cc].text[1], "#13#Your death will be swift, %s!");
	strcpy(ch[cc].text[3], "I am but one of many, %s!");
	
	if (ch[cn].data[CHD_GROUP]==1100)   // hacky workaround for map NPCs using SC.
		ch[cc].data[CHD_GROUP] = ch[cc].data[43] = ch[cc].data[59] = 1100;
	
	if (co && !dont_atk)
	{
		ch[cc].attack_cn = co;
		idx = co | (char_id(co) << 16);
		ch[cc].data[MCD_ENEMY1ST] = idx;            // add enemy to kill list
	}
	ch[cc].data[CHD_MASTER] = ch[cc].data[69] = cn;
	
	// Set duration of SC based on skill mod.
	ch[cc].data[PCD_COMPANION] = globs->ticker + SP_DUR_SHADOW(basedur);
	ch[cc].data[MCD_TALKATIVE] = ch_temp[CT_COMPANION].data[MCD_TALKATIVE];
	ch[cc].data[98]            = globs->ticker + COMPANION_TIMEOUT;
	
	if (ch[cn].flags & CF_SILENCE) 
	{
		ch[cc].data[MCD_TALKATIVE] = 0;
		ch[cc].flags |= CF_SILENCE;
	}
	if (ch[cn].flags & CF_INFRARED) ch[cc].flags |= CF_INFRARED;
	if (ch[cn].flags & CF_IGN_SB)   ch[cc].flags |= CF_IGN_SB;

	ch[cc].data[48] = 33;
	ch[cc].data[1]  =  2;	// Force offense mode
	
	ch[cc].kindred |= KIN_SHADOW;
	
	//{
	
	// 1. Determine scores we want based off target sprite
	ch[cc].skill[SK_MEDIT][2] = 90;   ch[cc].skill[SK_MEDIT][3]   = 6;
	ch[cc].skill[SK_SAFEGRD][2] = 75; ch[cc].skill[SK_SAFEGRD][3] = 7;
	ch[cc].mana[2] = ch[cc].hp[2];
	switch (sprite)
	{
		case  2000: // Templar Group
		case  8144: //  Cleave & Weaken
		case 22948: 
		case 23504: 
		case 24528: 
		case 26048: 
			ch[cc].skill[SK_CLEAVE][2] = 120; ch[cc].skill[SK_CLEAVE][3] = 4;
			ch[cc].skill[SK_WEAKEN][2] = 105; ch[cc].skill[SK_WEAKEN][3] = 5;
			break;
		
		case  5072: // Mercenary Group
		case  7120: //  Poison & Curse
		case 25552: 
		case 26576: 
		case 27600: 
		case 28624: 
			ch[cc].skill[SK_POISON][2] = 120; ch[cc].skill[SK_POISON][3] = 4;
			ch[cc].skill[SK_CURSE][2]  = 105; ch[cc].skill[SK_CURSE][3]  = 5;
			break;
		
		case  4048: // Harakim Group (Default Summoner)
		case  6096: //  Blast & Curse
		case 24048: 
		case 25048: 
		case 29648: 
		case 30672: 
			ch[cc].skill[SK_BLAST][2]  = 120; ch[cc].skill[SK_BLAST][3]  = 4;
			ch[cc].skill[SK_SLOW][2]   = 105; ch[cc].skill[SK_SLOW][3]   = 5;
			break;
		
		case  3024: // Seyan'du Group (Default Seyan'du)
		case 11216: //  Cleave & Curse
		case 27088: 
		case 28112: 
		case 17360: 
		case 15312: 
			ch[cc].skill[SK_CLEAVE][2] = 120; ch[cc].skill[SK_CLEAVE][3] = 4;
			ch[cc].skill[SK_CURSE][2]  = 105; ch[cc].skill[SK_CURSE][3]  = 5;
			break;
		
		case  9168: // Dead Group (& Necronomicon)
		case 10192: //  Blast & Weaken
		case 19408: 
			ch[cc].skill[SK_BLAST][2]  = 120; ch[cc].skill[SK_BLAST][3]  = 4;
			ch[cc].skill[SK_WEAKEN][2] = 105; ch[cc].skill[SK_WEAKEN][3] = 5;
			break;
		
		case 14288: // Vermin Group
		case 31696: //  Poison & Weaken
		case 20432: 
			ch[cc].skill[SK_POISON][2] = 120; ch[cc].skill[SK_POISON][3] = 4;
			ch[cc].skill[SK_WEAKEN][2] = 105; ch[cc].skill[SK_WEAKEN][3] = 5;
			break;
		
		case 13264: // Lizard Group
					//  Poison & Slow
			ch[cc].skill[SK_POISON][2] = 120; ch[cc].skill[SK_POISON][3] = 4;
			ch[cc].skill[SK_SLOW][2]   = 105; ch[cc].skill[SK_SLOW][3]   = 5;
			break;
		
		case 12240: // Gargoyle Group
		case 18384: //  Cleave & Slow
		case 21456: 
			ch[cc].skill[SK_CLEAVE][2] = 120; ch[cc].skill[SK_CLEAVE][3] = 4;
			ch[cc].skill[SK_SLOW][2]   = 105; ch[cc].skill[SK_SLOW][3]   = 5;
			break;
		
		case 22480: // Misc. Group (Flame)
		default:	//  Blast & Slow
			ch[cc].skill[SK_BLAST][2]  = 120; ch[cc].skill[SK_BLAST][3]  = 4;
			ch[cc].skill[SK_SLOW][2]   = 105; ch[cc].skill[SK_SLOW][3]   = 5;
			break;
	}
	
	// 2. Identical to GC code.
	for (n = 0; n<5; n++)
	{
		tmp = base;
		tmp = tmp * 5 / max(1, ch[cc].attrib[n][3]);
		B_AT(cc, n) = max(10, min(ch[cc].attrib[n][2]+(archbonus-n*2)/10, tmp));
	}
	
	for (n = 0; n<MAXSKILL; n++)
	{
		tmp = base;
		tmp = tmp * 5 / max(1, ch[cc].skill[n][3]);
		if (ch[cc].skill[n][2])
		{
			if (n==0)
			{
				B_SK(cc, n) = min(ch[cc].skill[n][2]+archbonus*3/20, tmp);
			}
			else
			{
				B_SK(cc, n) = min(ch[cc].skill[n][2]+(archbonus-archtmp)/10, tmp);
				archtmp++;
			}
		}
	}
	
	// Tree - Seyan GC/SC learns Regen
	if (T_SEYA_SK(cn, 9))
	{
		tmp = base;
		tmp = tmp * 5 / 8;
		B_SK(cc, SK_HEAL) = min(60, tmp);
		tmp = god_create_item(IT_CH_STAR);
		ch[cc].worn[WN_CHARM] = tmp;
		it[tmp].carried = cc;
	}
	
	if (necronomicon)
	{
		ch[cc].data[1] = 3; // BERSERK!!
	}

	ch[cc].hp[0]   = max( 50, min(ch[cc].hp[2],   base * 5));
	ch[cc].end[0]  = max(100, min(ch[cc].end[2],  base * 3));
	ch[cc].mana[0] = max( 50, min(ch[cc].mana[2], base * 4));

	// calculate experience
	for (z = 0; z<5; z++) for (m = 10; m<B_AT(cc, z); m++)
		pts += attrib_needed(m, 3);
	
	for (m = 50; m<ch[cc].hp[0]; m++)
		pts += hp_needed(m, 3);

	for (m = 50; m<ch[cc].mana[0]; m++)
		pts += mana_needed(m, 3);

	for (z = 0; z<MAXSKILL; z++) for (m = 1; m<B_SK(cc, z); m++)
		pts += skill_needed(m, 3);
	//
	
	ch[cc].weapon_bonus = max(12, 
						  min(80+(archbonus)/20, base*5/5+6));
	ch[cc].armor_bonus  = max(8, 
						  min(72+(archbonus+10)/20, base*5/6+8));

	ch[cc].points_tot = pts/2;
	ch[cc].gold       = 0;
	ch[cc].a_hp       = 9999999;
	ch[cc].a_end      = 999999;
	ch[cc].a_mana     = 999999;
	ch[cc].alignment  = ch[cn].alignment / 2;
	//}
	
	// Add a buff to the copy to carry over values that cannot otherwise be copied over
		in = god_create_buff();
		if (!in)
		{
			xlog("god_create_buff failed on shadow in skill_shadow");
			return 0;
		}
		
		strcpy(bu[in].name, "Shadow Copy");
		bu[in].flags 	|= IF_SPELL;
		bu[in].sprite[1] = BUF_SPR_SHADOW;
		bu[in].duration  = bu[in].active = SP_DUR_SHADOW(basedur)*10; // x10 to avoid run-out msg
		bu[in].temp  	 = SK_SHADOW;
		bu[in].power 	 = base;
		
		// additional specs from caster's spells
		if (IS_SEYA_OR_BRAV(co))
		{
			bu[in].armor[1]     = spell_multiplier(M_SK(cn, SK_PROTECT), cn) / 6 + 3;
			bu[in].weapon[1]    = spell_multiplier(M_SK(cn, SK_ENHANCE), cn) / 6 + 3;
		}
		else
		{
			bu[in].armor[1]     = spell_multiplier(M_SK(cn, SK_PROTECT), cn) / 4 + 4;
			bu[in].weapon[1]    = spell_multiplier(M_SK(cn, SK_ENHANCE), cn) / 4 + 4;
		}
		for (n = 0; n<5; n++) 
			bu[in].attrib[n][1] = ((spell_multiplier(M_SK(cn, SK_BLESS), cn)*2/3)-n) / 5 + 3;
		
		bu[in].data[4]   = 1; // Effects not removed by NMZ (SK_SHADOW on Shadow)
		
		if (devilR) // Shadow Copy deals 25%% more damage and takes 25%% less damage
		{
			bu[in].dmg_bonus[1]     = (25*2);
			bu[in].dmg_reduction[1] = (25*2);
			bu[in].data[5] = 1;
		}
		
		add_spell(cc, in);
	//
	
	xlog("Created %s (%d) with base %d as Shadow Copy for %s", 
		ch[cc].name, cc, base, ch[cn].reference);
	
	if (co && !dont_atk)
	{
		do_sayx(cc, ch[cc].text[1], ch[co].name);
	}
	else
	{
		tmp = 100;
		if (necronomicon) tmp = RANDOM(40);
		switch (tmp)
		{
			case  0: do_sayx(cc, "I shall rend their flesh!"); break;
			case  1: do_sayx(cc, "Hahahahah..."); break;
			case  2: do_sayx(cc, "Who... what am I...?"); break;
			case  3: do_sayx(cc, "They watch you with great interest, %s.", ch[cn].name); break;
			case  4: do_sayx(cc, "#9#My time is short, %s.", ch[cn].name); break;
			default: do_sayx(cc, "My time is short, %s.", ch[cn].name); break;
		}
	}
	
	do_update_char(cc);
	
	// Add a little buff icon to the player to show them roughly how much time the copy has left
		in = god_create_buff();
		if (!in)
		{
			xlog("god_create_buff failed on player in skill_shadow");
			return 0;
		}
		
		strcpy(bu[in].name, "Shadow Copy");
		bu[in].flags 	|= IF_SPELL;
		bu[in].sprite[1] = BUF_SPR_SHADOW;
		bu[in].duration  = bu[in].active = SP_DUR_SHADOW(basedur);
		bu[in].temp  	 = SK_SHADOW;
		bu[in].power 	 = base;
		bu[in].data[4]   = 1; // Effects not removed by NMZ (SK_SHADOW on owner)
		
		if (devilR) // Player deals 20%% less damage and takes 20%% more damage
		{
			bu[in].dmg_bonus[1]     = -(20*2);
			bu[in].dmg_reduction[1] = -(20*2);
			bu[in].data[5] = 2;
		}
		
		add_spell(cn, in);
	//
	
	fx_add_effect(6, 0, ch[cc].x, ch[cc].y, 0);
	fx_add_effect(7, 0, ch[cn].x, ch[cn].y, 0);
	
	return 1;
}
void skill_shadow(int cn)
{
	int co, cz;
	int archgc = 0, archbonus = 0, archtmp = 0, w = 0, necronomicon = 0, devilR = 0;
	int dont_atk = 0;

	if (IS_BUILDING(cn))
	{
		do_char_log(cn, 0, "Not in build mode.\n");
		return;
	}
	
	if (IS_PLAYER(cn) && (co = ch[cn].data[PCD_SHADOWCOPY]))
	{
		if (!IS_SANECHAR(co) || ch[co].data[CHD_MASTER]!=cn || 
			(ch[co].flags & CF_BODY) || ch[co].used==USE_EMPTY)
		{
			co = 0;
		}
		if (co && IS_SHADOW(co))
		{
			if ((cz = ch[cn].skill_target1) && do_char_can_see(cn, cz, 0))
			{
				if (cz == co)
				{
					do_char_log(cn, 1, "You dismissed your Shadow Copy.\n", co);
					answer_transfer(co, cn, 0);
				}
				else
				{
					chlog(cn, "Commands attack against %s (%d)", ch[cz].name, cz);
					if (skill_gc_atk(cn, co, cz) && get_gear(cn, IT_TW_INVIDIA))
					{
						quick_teleport(co, ch[cz].x, ch[cz].y);
					}
				}
			}
			else
			{
				do_char_log(cn, 5, "You recalled your Shadow Copy.\n");
				quick_teleport(co, ch[cn].x, ch[cn].y);
				add_exhaust(cn, TICKS*2);
			}
			return;
		}
	}
	
	co = ch[cn].skill_target1;
	
	// select who's sprite/array we're copying
	cz = cn;
	if (co && (IS_PLAYER(cn) || !RANDOM(2))) cz = co;
	
	if (cn==co) co = 0;
	if (co==ch[cn].data[PCD_SHADOWCOPY] || 
		co==ch[cn].data[PCD_COMPANION] ) co = 0;
	
	if (co && !do_char_can_see(cn, co, 0))
	{
		do_char_log(cn, 0, "You cannot see your target.\n");
		return;
	}

	if (is_exhausted(cn)) return;

	if (co && !may_attack_msg(cn, co, 0))
	{
		dont_atk = 1;
	}
	
	if (spellcost(cn, SP_COST_SHADOW, SK_SHADOW, 1)) return;
	
	if (chance(cn, FIVE_PERC_FAIL))
	{
		if (co && cn!=co) 
			do_area_notify(cn, co, ch[cn].x, ch[cn].y, 
				NT_SEEMISS, cn, co, M_SK(cn, SK_SHADOW), 0);
		if (co && cn!=co && CAN_SENSE(co) && 
			M_SK(co, SK_PERCEPT)>M_SK(cn, SK_SHADOW) + 5 && !dont_atk)
		{
			if (!(ch[co].flags & CF_SENSEOFF))
				do_char_log(co, 0, "%s tried to cast shadow copy on you but failed.\n", 
					ch[cn].reference);
			if (!IS_IGNORING_SPELLS(co))
				do_notify_char(co, NT_GOTMISS, cn, 0, 0, 0);
		}
		return;
	}
	
	archgc 					= B_SK(cn, SK_GCMASTERY);
	if (archgc) archbonus 	= M_SK(cn, SK_GCMASTERY);
	if (IS_PLAYER(cn) && !IS_SUMMONER(cn)) archbonus /= 2;
	
	if (spell_shadow(cn, co, cz, archbonus, dont_atk))
		add_exhaust(cn, SK_EXH_SHADOW);
}

int spell_aggravate(int cn, int co, int power)
{
	int in;
	
	if (ch[co].escape_timer > TICKS*3) 						{ return 0; }
	if (ch[co].flags & CF_BODY) 							{ return 0; }
	if (ch[cn].attack_cn!=co && ch[co].alignment==10000) 	{ return 0; }
	if (ch[co].flags & CF_IMMORTAL) 						{ return 0; }
	
	power = spell_immunity(power, get_target_immunity(cn, co));
	
	if (T_SEYA_SK(co, 10))				power = power*4/5;
	if (get_tarot(cn, IT_CH_CHARIO_R)) 	power = power*3/4;
	if (get_enchantment(co, 4)) 		power = power*4/5;
	
	if (!(in = make_new_buff(cn, SK_AGGRAVATE, BUF_SPR_AGGRAVATE, power, SP_DUR_AGGRAVATE, 0)))
		return 0;
	
	bu[in].dmg_reduction[1] = -(10*2);	// 10% more damage taken
	bu[in].data[4] = 1; 	// Effects not removed by NMZ (SK_AGGRAVATE)
	bu[in].data[5] = 1;
	
	if (add_spell(co, in))
	{
		chlog(cn, "Inflicted Aggravate on %s", ch[co].name);
		do_char_log(co, 0, "Your wound was aggravated from the blow!\n");
	}
	
	return 1;
}
int spell_bleed(int cn, int co, int power)
{
	int in, dur, bpow;
	
	if (ch[co].escape_timer > TICKS*3) 						{ return 0; }
	if (ch[co].flags & CF_BODY) 							{ return 0; }
	if (ch[cn].attack_cn!=co && ch[co].alignment==10000) 	{ return 0; }
	if (ch[co].flags & CF_IMMORTAL) 						{ return 0; }
	
	power = spell_immunity(power, get_target_immunity(cn, co));
	
	if (T_SEYA_SK(co, 10))				power = power*4/5;
	if (get_enchantment(cn, 30)) 		power = power*6/5;
	if (get_tarot(cn, IT_CH_CHARIO_R)) 	power = power*3/4;
	if (get_enchantment(co, 4)) 		power = power*4/5;
	
	dur = SP_DUR_BLEED; 			// 15 seconds
	
	bpow = BLEEDFORM(power, dur);
	
	if (!(in = make_new_buff(cn, SK_BLEED, BUF_SPR_BLEED, power, dur, 0)))
		return 0;
	
	bu[in].data[1] = bpow; 	// Set the decay rate of the bleed
	bu[in].data[4] = 1; 	// Effects not removed by NMZ (SK_BLEED)
	bu[in].data[5] = 1;
	
	if (add_spell(co, in))
	{
		chlog(cn, "Inflicted Bleed on %s", ch[co].name);
		do_char_log(co, 0, "You began bleeding from the blow!\n");
	}
	
	return 1;
}
int spell_cleave(int cn, int co, int power, int co_orig, int dr1, int dr2)
{
	int hitpower = power, aggravate=0, tmp, tmpmp=0, in;
	
	chlog(cn, "Used Cleave on %s", ch[co].name);
	
	if (T_ARTM_SK(cn, 4)) hitpower = hitpower + ch[cn].gethit_dam;
	if (T_WARR_SK(cn, 9)) hitpower = hitpower + (hitpower * M_AT(cn, AT_STR)/500);
	hitpower = spell_immunity(hitpower, ch[co].to_parry) * 2;
	if (co_orig) hitpower = hitpower/2 + hitpower/4;
	
	// Tarot Card - Justice :: Inflict Aggravate instead of Bleed
	if (get_tarot(cn, IT_CH_JUSTICE))
	{
		aggravate = 1;
	}
	
	tmp = do_hurt(cn, co, hitpower, 5);
	
	if (get_gear(cn, IT_WP_BRONCHIT))
	{
		tmpmp = tmp/5;
	}
	
	if (tmp<1)
	{
		do_char_log(cn, 0, "You cannot penetrate %s's armor.\n", ch[co].reference);
	}
	else if (tmpmp<1)
	{
		if (!(ch[cn].flags & CF_SYS_OFF))
			do_char_log(cn, 1, "You cleaved %s for %d HP.\n", ch[co].reference, tmp);
		if (!(ch[co].flags & CF_SYS_OFF))
			do_char_log(co, 1, "%s cleaved you for %d HP.\n", ch[cn].name, tmp);
	}
	else
	{
		if (!(ch[cn].flags & CF_SYS_OFF))
			do_char_log(cn, 1, "You cleaved %s for %d HP and %d mana.\n", ch[co].reference, tmp, tmpmp);
		if (!(ch[co].flags & CF_SYS_OFF))
			do_char_log(co, 1, "%s cleaved you for %d HP and %d mana.\n", ch[cn].name, tmp, tmpmp);
	}
	
	//if (in = has_buff(co, SK_PLAGUE)) speed_plague(co, in); 
	
	// Plague spreading
	if ((in = has_buff(co, SK_PLAGUE)) && IS_SANECHAR(bu[in].data[0]))
	{
		skill_plague(bu[in].data[0], co, bu[in].data[5]);
	}
	
	if (!co_orig)
	{
		char_play_sound(co, ch[cn].sound + 24, -50, 0);
		do_area_sound(co, 0, ch[co].x, ch[co].y, ch[cn].sound + 24);
	}
	fx_add_effect(5, 0, ch[co].x, ch[co].y, 0);
	
	if (aggravate)
		spell_aggravate(cn, co, hitpower);
	else if (tmp)
		spell_bleed(cn, co, tmp);
	
	check_gloves(cn, co, co_orig, dr1, dr2);
	
	if (!co_orig)
		surround_cast(cn, co, 0, SK_CLEAVE, power, dr1, dr2);
	
	return 1;
}
void skill_cleave(int cn)
{
	int power, cost, co;
	
	chlog(cn, "Uses CLEAVE on %s", ch[co].name);
	
	power = M_SK(cn, SK_CLEAVE) + ch[cn].weapon / 4 + ch[cn].top_damage / 4;
	power = skill_multiplier(power, cn);
	cost = power/12 + 5;
	
	if (IS_PLAYER(cn) && (IS_ANY_TEMP(cn) || IS_WARRIOR(cn) || IS_LYCANTH(cn)))
		cost = cost/3*2;
	else if (IS_PLAYER_GC(cn))
		cost = 5;
	else if (!IS_PLAYER(cn))
		cost = 20;
	
	// Get hit target - return on failure
	if (!(co = get_target(cn, 0, 0, 0, cost, SK_CLEAVE, 0, power, 0)))
		return;
	
	spell_cleave(cn, co, power, 0, RANDOM(GLVDICE), RANDOM(GLVDICE));
	
	add_exhaust(cn, SK_EXH_CLEAVE);
}

int spell_bash(int cn, int co, int power, int co_orig, int dr1, int dr2)
{
	int hitpower = power, aggravate=0, tmp;
	
	chlog(cn, "Used Shield Bash on %s", ch[co].name);
	
	hitpower = spell_immunity(hitpower, ch[co].to_parry) * 2;
	if (co_orig) hitpower = hitpower/2 + hitpower/4;
	tmp = do_hurt(cn, co, hitpower, 5);
	
	if (tmp<1)
	{
		do_char_log(cn, 0, "You cannot penetrate %s's armor.\n", ch[co].reference);
	}
	else
	{
		if (!(ch[cn].flags & CF_SYS_OFF)) do_char_log(cn, 1, "You bashed %s for %d HP.\n", ch[co].reference, tmp);
		if (!(ch[co].flags & CF_SYS_OFF)) do_char_log(co, 1, "%s bashed you for %d HP.\n", ch[cn].name, tmp);
		spell_warcry(cn, co, tmp, 2);
	}
	
	if (!co_orig)
	{
		char_play_sound(co, ch[cn].sound + 24, -50, 0);
		do_area_sound(co, 0, ch[co].x, ch[co].y, ch[cn].sound + 24);
	}
	fx_add_effect(5, 0, ch[co].x, ch[co].y, 0);
	
	check_gloves(cn, co, co_orig, dr1, dr2);
	
	if (!co_orig) surround_cast(cn, co, 0, SK_SHIELD, power, dr1, dr2);
	
	return 1;
}
void skill_bash(int cn)
{
	int power, cost, co;
	
	chlog(cn, "Uses SHIELD BASH on %s", ch[co].name);
	
	power = M_SK(cn, SK_SHIELD) + ch[cn].armor / 2;
	power = skill_multiplier(power, cn);
	cost = power/10 + 5;
	
	if (!IS_PLAYER(cn)) cost = 20;
	
	// Get hit target - return on failure
	if (!(co = get_target(cn, 0, 0, 0, cost, SK_SHIELD, 0, power, 0)))
		return;
	
	spell_bash(cn, co, power, 0, RANDOM(GLVDICE), RANDOM(GLVDICE));
	
	add_exhaust(cn, SK_EXH_CLEAVE);
}

int spell_weaken(int cn, int co, int power, int flag)
{
	int in;
	
	if (ch[co].escape_timer > TICKS*3) 						{ return 0; }
	if (ch[co].flags & CF_BODY) 							{ return 0; }
	if (ch[cn].attack_cn!=co && ch[co].alignment==10000) 	{ return 0; }
	if (ch[co].flags & CF_IMMORTAL) 						{ return 0; }
	
	power = spell_immunity(power, get_target_immunity(cn, co));
	
	if (T_SEYA_SK(co, 10))				power = power*4/5;
	if (get_enchantment(cn, 7)) 		power = power*6/5;
	if (get_tarot(cn, IT_CH_CHARIO_R)) 	power = power*3/4;
	if (get_enchantment(co, 4)) 		power = power*4/5;
	if (get_enchantment(co, 8)) 		power = power/5;
	
	// Tarot Card - Death :: Change Weaken into Crush
	if (get_tarot(cn, IT_CH_DEATH))
	{
		if (!(in = make_new_buff(cn, SK_WEAKEN2, BUF_SPR_REND2, power, SP_DUR_WEAKEN, 0)))
			return 0;
		
		if (!IS_PLAYER(cn))
			bu[in].armor[1]  = max(-127, -(power / 6 + 2));
		else
			bu[in].armor[1]  = max(-127, -(power / 4 + 4));
	}
	else
	{
		if (!(in = make_new_buff(cn, SK_WEAKEN, BUF_SPR_REND, power, SP_DUR_WEAKEN, 0)))
			return 0;
		
		if (!IS_PLAYER(cn))
			bu[in].weapon[1]  = max(-127, -(power / 6 + 2));
		else
			bu[in].weapon[1]  = max(-127, -(power / 4 + 4));
	}
	
	// Tree
	if (T_SKAL_SK(cn, 10))
		bu[in].crit_chance[1] = max(-127, -(power / 4 + 4));
	
	bu[in].data[4] = 1; // Effects not removed by NMZ (SK_WEAKEN & SK_WEAKEN2)
	bu[in].data[5] = 1;
	
	return cast_a_spell(cn, co, in, 1+flag, 1-flag); // SK_WEAKEN / SK_WEAKEN2
}
void skill_weaken(int cn)
{
	int d20 = SP_MULT_WEAKEN;
	int power, aoe_power, cost;
	int count = 0, hit = 0;
	int co = 0, co_orig = 0;
	int can_aoe = CAN_BRAV_PROX(cn);
	
	power = M_SK(cn, SK_WEAKEN);
	power = skill_multiplier(power, cn);
	aoe_power = M_SK(cn, SK_PROX);
	cost = SP_COST_WEAKEN;
	
	// Tarot Card - Death :: Change Weaken into Crush
	if (get_tarot(cn, IT_CH_DEATH)) 
	{ 
		cost = cost * 5 / 4;
		d20 = SP_MULT_WEAKEN2;
	}
	
	// Get spell target - return on failure
	if (!(co = get_target(cn, 0, 0, 0, cost, SK_WEAKEN, 0, power, d20)))
		return;
	
	// If we have a valid target, cast Slow on them
	if (cn!=co && co!=ch[cn].data[PCD_SHADOWCOPY] && co!=ch[cn].data[PCD_COMPANION])
	{
		spell_weaken(cn, co, power, 0);
		
		co_orig = co;
		count++;
		hit++;
	}
	
	// Cast AoE or general surround-hit
	if (can_aoe)
	{
		if (cast_aoe_spell(cn, co, SK_WEAKEN, power, aoe_power, cost, count, hit, 0, -1, -1) < 0)
			return;
		
		fx_add_effect(7, 0, ch[cn].x, ch[cn].y, 0);
	}
	else
	{
		surround_cast(cn, co_orig, 0, SK_WEAKEN, power, -1, -1);
	}
	
	add_exhaust(cn, SK_EXH_WEAKEN);
}

// Blind reduces enemy perception, hit, and parry scores. Uses Endurance.
// Gets AoE with Proximity skill
int spell_blind(int cn, int co, int power, int flag)
{
	int n, in, d20 = SP_MULT_BLIND;
	
	if (!IS_PLAYER(co)) d20 += 25;
	if (ch[co].escape_timer > TICKS*3) 	{ return 0; }
	if (ch[co].flags & CF_BODY) 		{ return 0; }
	if (!do_surround_check(cn, co, 1) || 
		chance_base(cn, co, SK_BLIND, SP_MULT_BLIND, get_target_resistance(cn, co), 0, 0)) { return 0; }
	if (cn!=co) do_area_notify(cn, co, ch[cn].x, ch[cn].y, NT_SEEHIT, cn, co, 0, 0);
	if (!IS_IGNORING_SPELLS(co)) { do_notify_char(co, NT_GOTHIT, cn, 0, 0, 0); }
	
	power = spell_immunity(power, get_target_immunity(cn, co));
	
	if (T_SEYA_SK(co, 10))				power = power*4/5;
	if (T_WARR_SK(cn, 7))				power = power + (power * M_AT(cn, AT_AGL)/2000);
	if (get_enchantment(cn, 31)) 		power = power*6/5;
	if (get_tarot(cn, IT_CH_CHARIO_R)) 	power = power*3/4;
	if (get_enchantment(co, 4)) 		power = power*4/5;
	if (get_enchantment(co, 33)) 		power = power/5;
	
	// Tarot Card - Chariot :: Change Blind into Douse
	if (flag)
	{ 
		// Add Douse
		if (!(in = make_new_buff(cn, SK_DOUSE, BUF_SPR_DOUSE, power, SP_DUR_DOUSE, 0)))
			return 0;
		
		if (IS_ANY_MERC(cn) && IS_PLAYER(cn))
		{
			bu[in].skill[SK_STEALTH][1] = max(-127, -(power/2 + 4));
			bu[in].spell_mod[1]         = max(-127, -(power/6 + 2));
		}
		else
		{
			bu[in].skill[SK_STEALTH][1] = max(-127, -(power/3 + 3));
			bu[in].spell_mod[1]         = max(-127, -(power/8 + 1));
		}
		chlog(cn, "Used Douse on %s", ch[co].name);
	}
	else
	{
		// Add Blind
		if (!(in = make_new_buff(cn, SK_BLIND, BUF_SPR_BLIND, power, SP_DUR_BLIND, 0)))
			return 0;
		
		if (IS_ANY_MERC(cn) && IS_PLAYER(cn))
		{
			bu[in].skill[SK_PERCEPT][1] = max(-127, -(power/2 + 4));
			bu[in].to_hit[1]            = max(-127, -(power/6 + 2));
			bu[in].to_parry[1]          = max(-127, -(power/6 + 2));
		}
		else
		{
			bu[in].skill[SK_PERCEPT][1] = max(-127, -(power/3 + 3));
			bu[in].to_hit[1]            = max(-127, -(power/8 + 1));
			bu[in].to_parry[1]          = max(-127, -(power/8 + 1));
		}
		chlog(cn, "Used Blind on %s", ch[co].name);
	}
	
	bu[in].data[4] = 1; // Effects not removed by NMZ (SK_BLIND)
	bu[in].data[5] = 1;
	
	add_spell(co, in);
	//
	fx_add_effect(5, 0, ch[co].x, ch[co].y, 0);
	return 1;
}
void skill_blind(cn)
{
	int power, aoe_power, cost;
	int can_aoe = CAN_WARR_PROX(cn);
	
	power = M_SK(cn, SK_BLIND);
	power = skill_multiplier(power, cn);
	aoe_power = M_SK(cn, SK_BLIND) + (B_SK(cn, SK_PROX)?(M_SK(cn, SK_PROX)/2):0);
	cost = SP_COST_BLIND;
	
	if (IS_ANY_MERC(cn))
		cost /= 2;
	
	if (is_exhausted(cn)) { return; }
	if (spellcost(cn, cost, SK_BLIND, 0)) { return; }
	
	// Tarot Card - Chariot :: Change Blind into Douse
	if (get_tarot(cn, IT_CH_CHARIOT)) 
	{
		if (cast_aoe_spell(cn, 0, SK_DOUSE, power, can_aoe?aoe_power:0, 0, 0, 0, 0, -1, -1) < 0)
			return;
	}
	else
	{
		if (cast_aoe_spell(cn, 0, SK_BLIND, power, can_aoe?aoe_power:0, 0, 0, 0, 0, -1, -1) < 0)
			return;
	}
	
	add_exhaust(cn, SK_EXH_BLIND);
}

// Pulse causes small bursts of AoE damage over its duration
// Gets AoE with Proximity skill
int spell_shock(int cn, int co, int power)
{
	int in;
	
	if (ch[co].escape_timer > TICKS*3) 						{ return 0; }
	if (ch[co].flags & CF_BODY) 							{ return 0; }
	if (ch[cn].attack_cn!=co && ch[co].alignment==10000) 	{ return 0; }
	if (ch[co].flags & CF_IMMORTAL) 						{ return 0; }
	
	power = spell_immunity(power, get_target_immunity(cn, co));
	
	if (T_SEYA_SK(co, 10))				power = power*4/5;
	if (get_tarot(cn, IT_CH_CHARIO_R)) 	power = power*3/4;
	if (get_enchantment(co, 4)) 		power = power*4/5;
	
	if (!(in = make_new_buff(cn, SK_SHOCK, BUF_SPR_SHOCK, power, SP_DUR_SHOCK, 0))) 
		return 0;
	
	bu[in].dmg_reduction[1] = -(2*2);	// x% more damage taken
	bu[in].dmg_bonus[1]     = -(2*2);	// x% less damage dealt
	bu[in].value = 1;
	bu[in].data[5] = 1;
	
	return add_spell(co, in); // SK_SHOCK
}
int spell_charge(int cn, int co, int power)
{
	int in;
	
	if (!(in = make_new_buff(cn, SK_CHARGE, BUF_SPR_CHARGE, power, SP_DUR_SHOCK, 0))) 
		return 0;
	
	bu[in].dmg_reduction[1] = -(2*2);	// x% more damage taken
	bu[in].dmg_bonus[1]     = -(2*2);	// x% less damage dealt
	bu[in].value = 1;
	
	return add_spell(co, in); // SK_CHARGE
}
int spell_pulse(int cn, int co, int power, int tarot)
{
	int in, len, baselen = 100;

	// Book: Damor's Grudge
	if (get_book(cn, IT_BOOK_DAMO))
	{
		baselen = 90;
	}
	
	len = (SK_EXH_PULSE/2) * baselen / max(100, ch[cn].cool_bonus);
	
	power = spell_multiplier(power, cn);
	
	// Tarot - Judgement.R : Pulse changed to buff form
	if (tarot)
	{
		if (!(in = make_new_buff(cn, SK_PULSE2, BUF_SPR_PULSE, power, SP_DUR_PULSE, 0))) 
			return 0;
	}
	else
	{
		if (!(in = make_new_buff(cn, SK_PULSE, BUF_SPR_PULSE, power, SP_DUR_PULSE, 0))) 
			return 0;
	}
	
	bu[in].data[1] = len; 								// tick rate
	bu[in].data[2] = globs->ticker + bu[in].data[1]; 	// next tick
	bu[in].data[3] = PRXP_RAD + ch[cn].aoe_bonus;
	
	return cast_a_spell(cn, co, in, 0, 1); // SK_PULSE
}
int spell_immolate(int cn, int co, int power)
{
	int in;
	
	if (get_book(cn, IT_BOOK_BURN)) power = power + ch[cn].hp[4]/20;	
	if (!(in = make_new_buff(cn, SK_IMMOLATE, BUF_SPR_IMMOLATE, power, SP_DUR_PULSE, 0))) 
		return 0;
	
	bu[in].hp[0]   = -300;
	bu[in].data[3] 	= PRXP_RAD + ch[cn].aoe_bonus;
	
	bu[in].flags |= IF_PERMSPELL;
	
	return cast_a_spell(cn, co, in, 0, 1); // SK_IMMOLATE
}
void skill_pulse(int cn)
{
	int co, power, tarot = 0;
	
	tarot = get_tarot(cn, IT_CH_JUDGE_R);
	
	if (is_exhausted(cn)) 								{ return; }
	if (tarot && has_buff(cn, SK_IMMOLATE))
	{
		do_char_log(cn, 1, "Immolate no longer active.\n");
		remove_buff(cn, SK_IMMOLATE);
		do_update_char(cn);
		return;
	}
	if (spellcost(cn, SP_COST_PULSE, SK_PULSE, 1))		{ return; }
	if (chance(cn, FIVE_PERC_FAIL)) 					{ return; }
	
	power = M_SK(cn, SK_PULSE);
	if (T_ARHR_SK(cn, 7)) power = power + (power * M_AT(cn, AT_INT)/500);
	
	spell_pulse(cn, cn, power, tarot);

	add_exhaust(cn, SK_EXH_PULSE);
}

// Taunt pulls targets to the user
// Grants a defense bonus to Arch Templars and an offense bonus to Skalds
// Gets AoE with Proximity skill
int spell_taunt(int cn, int co, int power, int flag)
{
	int in;
	
	if (ch[co].escape_timer > TICKS*3) { return 0; }
	if (ch[co].flags & CF_BODY) { return 0; }
	if (ch[cn].attack_cn!=co && ch[co].alignment==10000) { return 0; }
	if (ch[co].flags & CF_IMMORTAL) { return 0; }
	
	power = spell_immunity(power, get_target_immunity(cn, co));
	
	if (T_SEYA_SK(co, 10))				power = power*4/5;
	if (get_tarot(cn, IT_CH_CHARIO_R)) 	power = power*3/4;
	if (get_enchantment(co, 4)) 		power = power*4/5;
	
	if (!(in = make_new_buff(cn, SK_TAUNT, BUF_SPR_TAUNT, power, max(TICKS/4, power), 0)))
		return 0;
	
	bu[in].data[4] = 1; // Effects not removed by NMZ (SK_TAUNT)
	bu[in].data[5] = 1;
	
	return cast_a_spell(cn, co, in, 1+flag, 1-flag); // SK_TAUNT
}
int spell_guard(int cn, int co, int power, int hit)
{
	int in, dur;
	
	dur = SP_DUR_GUARD;
	
	// Tarot - Temperance - 100% more power, 50% less duration.
	if (get_tarot(cn, IT_CH_TEMPER))
	{
		power = power * 2;
		dur   = dur / 2;
	}
	
	if (!(in = make_new_buff(cn, SK_GUARD, BUF_SPR_GUARD, power, dur, 0))) 
		return 0;
	
	bu[in].dmg_reduction[1] = min(100, power/5);	// 0.5% less damage taken per 5 power
	bu[in].data[4] = 1; 							// Effects not removed by NMZ (SK_GUARD)
	
	return cast_a_spell(cn, co, in, 0, 0); // SK_GUARD
}
void skill_taunt(int cn)
{
	int d20 = SP_MULT_TAUNT;
	int power, aoe_power, cost;
	int count = 0, hit = 0;
	int co, co_orig = -1;
	int can_aoe = CAN_ARTM_PROX(cn);
	
	power = M_SK(cn, SK_TAUNT);
	power = skill_multiplier(power, cn);
	aoe_power = M_SK(cn, SK_TAUNT)*2;
	cost = SP_COST_TAUNT;
	
	if (IS_SANEPLAYER(ch[cn].data[CHD_MASTER])) // Ghost Comp
	{
		cost = 5;
	}
	
	// Get skill target - return on failure
	if (!(co = get_target(cn, 0, 0, 0, cost, SK_TAUNT, 0, power, d20)))
		return;
	
	// If we have a valid target, cast Slow on them
	if (cn!=co && co!=ch[cn].data[PCD_SHADOWCOPY] && co!=ch[cn].data[PCD_COMPANION])
	{
		spell_taunt(cn, co, power, 0);
		
		co_orig = co;
		count++;
		hit++;
	}
	
	// Cast AoE or general surround-hit
	if (can_aoe)
	{
		hit = cast_aoe_spell(cn, co, SK_TAUNT, power, aoe_power, cost, count, hit, 0, -1, -1);
		
		fx_add_effect(7, 0, ch[cn].x, ch[cn].y, 0);
	}
	else
	{
		hit += surround_cast(cn, co_orig, 0, SK_TAUNT, power, -1, -1);
	}
	
	if (hit) spell_guard(cn, cn, power, max(1,hit));
	
	add_exhaust(cn, SK_EXH_TAUNT);
}

int invalid_leap(int cn, int m, int md)
{
	int mt;
	
	return ((IS_LIVINGCHAR(mt = map[m + md].ch) && mt!=cn) || 
		(IS_LIVINGCHAR(mt = map[m + md].to_ch) && mt!=cn) || 
		(map[m + md].flags & MF_MOVEBLOCK) || 
		(IS_MONSTER(cn) && (map[m + md].flags & MF_NOMONST)) ||
		(!IS_MONSTER(cn) && (map[m + md].flags & MF_NOPLAYER) && 
			(!IS_IN_XVIII(M2X(m + md), M2Y(m + md)) || has_item(cn, IT_COMMAND4))) ||
		(IS_IN_XVIII(M2X(m + md), M2Y(m + md)) && has_item(cn, IT_COMMAND2) && (map[m + md].flags & MF_BANK)) ||
		(IS_SANEITEM(mt = map[m + md].it) && (it[mt].flags & IF_MOVEBLOCK)));
}
// Leap teleports behind your target, damaging them and the enemy you're fighting
// Escapes combat in the process
// Gets bonus damage from attack speed score
void skill_leap(int cn, int flag)
{
	int dr1 = RANDOM(GLVDICE), dr2 = RANDOM(GLVDICE);
	int power, aoepower, cost, dist, cost_dist, cost_pow, tmp, critical;
	int co, cc=0, dam;
	int x, y, n, m, md, mt, obstructed = 0, newdir = 0, randomtarg = 0, cooldown = SK_EXH_LEAP;
	int dist_target=0, same_target=0, dostun=1;
	int xc, yc, xf, yf, xt, yt, c;
	int catalog[64] = { 0 };
	
	power = M_SK(cn, SK_LEAP) + ch[cn].weapon / 4 + ch[cn].top_damage / 4;
	power = skill_multiplier(power, cn);
	aoepower = 10 + ch[cn].aoe_bonus;
	critical = ch[cn].crit_multi;
	
	// Tarot Card - Justice.R :: make target random
	if (get_tarot(cn, IT_CH_JUSTIC_R) || flag)
	{
		randomtarg = 1;
		dostun = 0;
	}
	if (IS_MONSTER(cn) || has_buff(cn, SK_EXHAUST)) dostun = 0;
	
	if (randomtarg)
	{
		c = 0;
		
		xc = ch[cn].x;
		yc = ch[cn].y;
		xf = max(1, xc - aoepower);
		yf = max(1, yc - aoepower);
		xt = min(MAPX - 1, xc + 1 + aoepower);
		yt = min(MAPY - 1, yc + 1 + aoepower);
		
		for (x = xf; x<xt; x++) for (y = yf; y<yt; y++) 
		{
			if (sqr(xc - x) + sqr(yc - y) > sqr(aoepower))
			{
				continue;
			}
			if (IS_SANECHAR(co = map[x + y * MAPX].ch) && cn!=co)
			{
				if (!do_char_can_see(cn, co, 0)) continue;
				if (do_surround_check(cn, co, 1))
				{
					catalog[c] = co;
					c++;
				}
			}
		}
		if (c)
		{
			co = catalog[RANDOM(c)];
		}
		else
		{
			if (!flag) do_char_log(cn, 0, "You cannot leap on yourself!\n");
			return;
		}
	}
	else
	{
		// we leap to 'co' and damage both 'cc' and 'co' in the process
		if (!(co = get_target(cn, 0, 0, 0, 0, SK_LEAP, 0, power, 0))) return;
	}
	
	if (!is_facing(cn, co))
		dist_target = 1;

	if (dist_target && is_facing(cn, (cc = ch[cn].attack_cn))) ;
	else { cc = co; same_target = 1; }
	
	if (cc==cn)
	{
		if (!randomtarg && ch[cn].goto_x) // walking teleport
		{
			x = ch[cn].goto_x;
			y = ch[cn].goto_y;
			fx_add_effect(12, 0, ch[cn].x, ch[cn].y, 0);
			god_transfer_char(cn, x, y);
			fx_add_effect(12, 0, ch[cn].x, ch[cn].y, 0);
			ch[cn].escape_timer = TICKS*3;
			for (m = 0; m<4; m++) ch[cn].enemy[m] = 0;
			remove_enemy(cn);
			ch[cn].goto_x = 0;
			add_exhaust(cn, cooldown);
			return;
		}
		else
		{
			if (!flag) do_char_log(cn, 0, "You cannot leap on yourself!\n");
			return;
		}
	}
	
	if (dist_target && !do_char_can_see(cn, co, 0))
	{
		if (!flag) do_char_log(cn, 0, "You cannot see your target.\n");
		return;
	}
	
	if ((dist_target && ch[co].flags & CF_STONED) || (ch[cc].flags & CF_STONED))
	{
		if (!flag) do_char_log(cn, 0, "Your target is lagging. Try again later.\n");
		return;
	}
	
	// Set up map specific variables to scope surroundings
	x = ch[co].x;  y = ch[co].y;  m = XY2M(x, y);
	
	if (ch[co].dir==DX_DOWN  || ch[co].dir==DX_RIGHTDOWN)	{	md =  MAPX;		newdir = DX_UP;		}
	if (ch[co].dir==DX_UP    || ch[co].dir==DX_LEFTUP)		{	md = -MAPX;		newdir = DX_DOWN;	}
	if (ch[co].dir==DX_RIGHT || ch[co].dir==DX_RIGHTUP)		{	md =  1;		newdir = DX_LEFT;	}
	if (ch[co].dir==DX_LEFT  || ch[co].dir==DX_LEFTDOWN)	{	md = -1;		newdir = DX_RIGHT;	}
		
	// Check for obstructions
	if (invalid_leap(cn, m, md))
	{
		chlog(cn, "Leap Obstructed 1/4");
		// try again with target back
		if (ch[co].dir==DX_DOWN  || ch[co].dir==DX_RIGHTDOWN)	{	md = -MAPX;		newdir = DX_DOWN;	}
		if (ch[co].dir==DX_UP    || ch[co].dir==DX_LEFTUP)		{	md =  MAPX;		newdir = DX_UP;		}
		if (ch[co].dir==DX_RIGHT || ch[co].dir==DX_RIGHTUP)		{	md = -1;		newdir = DX_RIGHT;	}
		if (ch[co].dir==DX_LEFT  || ch[co].dir==DX_LEFTDOWN)	{	md =  1;		newdir = DX_LEFT;	}
		
		if (invalid_leap(cn, m, md))
		{
			chlog(cn, "Leap Obstructed 2/4");
			// try with side
			if (ch[co].dir==DX_DOWN  || ch[co].dir==DX_RIGHTDOWN)	{	md =  1;		newdir = DX_LEFT;	}
			if (ch[co].dir==DX_UP    || ch[co].dir==DX_LEFTUP)		{	md = -1;		newdir = DX_RIGHT;	}
			if (ch[co].dir==DX_RIGHT || ch[co].dir==DX_RIGHTUP)		{	md =  MAPX;		newdir = DX_UP;		}
			if (ch[co].dir==DX_LEFT  || ch[co].dir==DX_LEFTDOWN)	{	md = -MAPX;		newdir = DX_DOWN;	}
			
			if (invalid_leap(cn, m, md))
			{
				chlog(cn, "Leap Obstructed 3/4");
				// try with other side
				if (ch[co].dir==DX_DOWN  || ch[co].dir==DX_RIGHTDOWN)	{	md = -1;		newdir = DX_RIGHT;	}
				if (ch[co].dir==DX_UP    || ch[co].dir==DX_LEFTUP)		{	md =  1;		newdir = DX_LEFT;	}
				if (ch[co].dir==DX_RIGHT || ch[co].dir==DX_RIGHTUP)		{	md = -MAPX;		newdir = DX_DOWN;	}
				if (ch[co].dir==DX_LEFT  || ch[co].dir==DX_LEFTDOWN)	{	md =  MAPX;		newdir = DX_UP;		}
				
				if (invalid_leap(cn, m, md))
				{
					chlog(cn, "Leap Obstructed 4/4");
					obstructed = 1;
				}
			}
		}
	}
	if (!obstructed)
	{
		x = (m + md) % MAPX;
		y = (m + md) / MAPX;
	}
	
	dist = sqrt(abs(ch[cn].x - x)*abs(ch[cn].x - x) + abs(ch[cn].y - y)*abs(ch[cn].y - y));
	
	if (dist > aoepower)
	{
		if (!flag) do_char_log(cn, 0, "Your target is too far away!\n");
		return;
	}
	
	if (co!=ch[cn].attack_cn && obstructed)
	{
		if (!flag) do_char_log(cn, 0, "You cannot reach your target.\n");
		return;
	}
	
	cost_dist = dist*2 + power / 60 + 2;
	cost_pow  = power / 30 + 2;
	
	cost = randomtarg?cost_pow:((cost_dist>cost_pow&&!obstructed)?cost_dist:cost_pow);
	
	if (!IS_PLAYER(cn)) cost = 20;
	if (!flag && spellcost(cn, cost, SK_LEAP, 0)) return;
	
	char_play_sound(cn, ch[cn].sound + 24, -50, 0);
	
	if (!randomtarg) surround_cast(cn, co, cc, SK_LEAP, power, dr1, dr2);
	
	if (!obstructed)
	{
		fx_add_effect(12, 0, ch[cn].x, ch[cn].y, 0);
		god_transfer_char(cn, x, y);
		fx_add_effect(12, 0, ch[cn].x, ch[cn].y, 0);
		ch[cn].escape_timer = TICKS*3;
		for (m = 0; m<4; m++) ch[cn].enemy[m] = 0;
		remove_enemy(cn);
		ch[cn].dir = newdir;
		ch[cn].attack_cn = co;
		if (!randomtarg) surround_cast(cn, co, cc, SK_LEAP, power, dr1, dr2);
	}
	
	if (!same_target && !obstructed && !flag)
	{
		chlog(cn, "Used Leap on %s and %s", ch[co].name, ch[cc].name);
		// Damage Fight target (cc)
		dam = spell_immunity(power, ch[cc].to_parry) * 2;
		if (ch[cc].a_hp >= ch[cc].hp[5]*950) 
		{
			dam = dam * critical / 100;
			if (dostun) spell_warcry(cn, cc, dam/2, 2);
		}
		else if (dostun) spell_warcry(cn, cc, dam/2, 1);
		tmp = do_hurt(cn, cc, dam, randomtarg?0:8);
		if (tmp<1) do_char_log(cn, 0, "You cannot penetrate %s's armor.\n", ch[cc].reference);
		else
		{
			if (!(ch[cn].flags & CF_SYS_OFF))
				do_char_log(cn, 1, "You sliced %s for %d HP.\n", ch[cc].reference, tmp);
			if (!(ch[cc].flags & CF_SYS_OFF))
				do_char_log(cc, 1, "%s sliced you for %d HP.\n", ch[cn].name, tmp);
		}
		do_area_sound(cc, 0, ch[cc].x, ch[cc].y, ch[cn].sound + 24);
		fx_add_effect(5, 0, ch[cc].x, ch[cc].y, 0);
		check_gloves(cn, cc, 0, dr1, dr2);
		// Damage leap target (co)
		dam = spell_immunity(power, ch[co].to_parry) * 2;
		if (ch[co].a_hp >= ch[co].hp[5]*950) 
		{
			dam = dam * critical / 100;
			if (dostun) spell_warcry(cn, co, dam/2, 2);
		}
		else if (dostun) spell_warcry(cn, co, dam/2, 1);
		tmp = do_hurt(cn, co, dam, randomtarg?0:8);
		if (tmp<1) do_char_log(cn, 0, "You cannot penetrate %s's armor.\n", ch[co].reference);
		else
		{
			if (!(ch[cn].flags & CF_SYS_OFF))
				do_char_log(cn, 1, "You sliced %s for %d HP.\n", ch[co].reference, tmp);
			if (!(ch[co].flags & CF_SYS_OFF))
				do_char_log(co, 1, "%s sliced you for %d HP.\n", ch[cn].name, tmp);
		}
		do_area_sound(co, 0, ch[co].x, ch[co].y, ch[cn].sound + 24);
		fx_add_effect(5, 0, ch[co].x, ch[co].y, 0);
		check_gloves(cn, co, cc, dr1, dr2);
	}
	else
	{
		chlog(cn, "Used Leap on %s", ch[co].name);
		// Damage
		dam = spell_immunity(power, ch[co].to_parry) * 2;
		if (ch[co].a_hp >= ch[co].hp[5]*950) 
		{
			dam = dam * critical / 100;
			if (dostun) spell_warcry(cn, co, dam/2, 2);
		}
		else if (dostun) spell_warcry(cn, co, dam/2, 1);
		tmp = do_hurt(cn, co, dam, randomtarg?0:8);
		if (tmp<1) do_char_log(cn, 0, "You cannot penetrate %s's armor.\n", ch[co].reference);
		else
		{
			if (!(ch[cn].flags & CF_SYS_OFF))
				do_char_log(cn, 1, "You sliced %s for %d HP.\n", ch[co].reference, tmp);
			if (!(ch[co].flags & CF_SYS_OFF))
				do_char_log(co, 1, "%s sliced you for %d HP.\n", ch[cn].name, tmp);
		}
		do_area_sound(co, 0, ch[co].x, ch[co].y, ch[cn].sound + 24);
		fx_add_effect(5, 0, ch[co].x, ch[co].y, 0);
		check_gloves(cn, co, 0, dr1, dr2);
	}
	
	if (!flag)
	{
		if (tmp = has_buff(cn, SK_EXHAUST))
		{
			bu[tmp].duration = bu[tmp].active = cooldown;
		}
		else
		{
			tmp = add_exhaust(cn, cooldown);
		}
		if (randomtarg && tmp)
		{
			bu[tmp].data[0]  = SK_LEAP;
			bu[tmp].data[1] += 2;
		}
	}
}

// Zephyr grants a stacking debuff to hits, dealing additional damage when it expires
// This damage is considered melee
int spell_zephyr(int cn, int co, int power, int flag)
{
	int in, deathR = 0;
	
	// Tarot - Death.R : Trigger, imm/res, and damage flip-flop
	if (get_tarot(cn, IT_CH_DEATH_R))
	{
		deathR = 1;
	}
	
	if (flag)	// Debuff version
	{
		if (ch[cn].attack_cn!=co && ch[co].alignment==10000) { return 0; }
		if (ch[co].flags & CF_IMMORTAL) { return 0; }
		
		power = power + max(0, ((SPEED_CAP - ch[cn].speed) + ch[cn].atk_speed - 120)) / 3;
		
		if (IS_PLAYER(co))	power = spell_immunity(power, get_target_immunity(cn, co));
		else				power = spell_immunity(power, get_target_immunity(cn, co)/2);
		
		if (T_SEYA_SK(co, 10))				power = power*4/5;
		if (get_tarot(cn, IT_CH_CHARIO_R)) 	power = power*3/4;
		if (get_enchantment(co, 4)) 		power = power*4/5;
		
		if (!IS_PLAYER(cn)) power = power/2;
		
		if (!(in = make_new_buff(cn, SK_ZEPHYR2, BUF_SPR_ZEPHYR2, power, TICKS, 0))) 
			return 0;
		
		bu[in].data[2] = bu[in].data[1] = 0;
		bu[in].data[4] = 1; // Effects not removed by NMZ (SK_ZEPHYR2)
		bu[in].data[5] = 1;
		
		bu[in].value = 1;
		
		return add_spell(co, in); // SK_ZEPHYR2
	}
	else		// Buff version
	{
		power = spell_multiplier(power, cn);
		
		if (!(in = make_new_buff(cn, SK_ZEPHYR, BUF_SPR_ZEPHYR, power, SP_DUR_ZEPHYR, 0))) 
			return 0;
		
		return cast_a_spell(cn, co, in, 0, 1); // SK_ZEPHYR
	}
}
void skill_zephyr(int cn)
{
	if (is_exhausted(cn)) 								{ return; }
	if (spellcost(cn, SP_COST_ZEPHYR, SK_ZEPHYR, 1))	{ return; }
	if (chance(cn, FIVE_PERC_FAIL)) 					{ return; }

	spell_zephyr(cn, cn, M_SK(cn, SK_ZEPHYR), 0);

	add_exhaust(cn, SK_EXH_ZEPHYR);
}

int spell_lethargy(int cn, int co, int power)
{
	int in;
	
	if (T_SORC_SK(cn, 7)) power = power + (power * M_AT(cn, AT_WIL)/1000);
	power = spellpower_check(cn, co, spell_multiplier(power, cn), 0);
	
	if (!(in = make_new_buff(cn, SK_LETHARGY, BUF_SPR_LETHARGY, power, SP_DUR_LETHARGY, 1))) 
		return 0;
	
	// Tarot - Empress.R : Lethargy does life over time instead
	if (get_tarot(co, IT_CH_EMPRES_R)) 
	{
		if (IS_SEYAN_DU(co))
			bu[in].hp[0]   = -150;
		else
			bu[in].hp[0]   = -200;
	}
	else
	{
		if (IS_SEYAN_DU(co))
			bu[in].mana[0] = -75;
		else
			bu[in].mana[0] = -100;
	}
	
	// Seyan'du get a weaker variant
	if (IS_SEYAN_DU(co)) bu[in].data[2] = 1;
	else				 bu[in].data[2] = 0;
	
	bu[in].flags |= IF_PERMSPELL;
	
	return cast_a_spell(cn, co, in, 0, 1); // SK_LETHARGY
}
void skill_lethargy(int cn)
{
	if (is_exhausted(cn)) 									{ return; }
	if (has_buff(cn, SK_LETHARGY))
	{
		do_char_log(cn, 1, "Lethargy no longer active.\n");
		remove_buff(cn, SK_LETHARGY);
		do_update_char(cn);
		return;
	}
	if (spellcost(cn, SP_COST_LETHARGY, SK_LETHARGY, 1))	{ return; }

	spell_lethargy(cn, cn, M_SK(cn, SK_LETHARGY));

	add_exhaust(cn, SK_EXH_LETHARGY);
}

// Shifting between forms for Lycanthropes
int spell_shift(int cn, int co, int power)
{
	int in, dur = SK_EXH_SHIFT, bspmod = 95;
	
	dur = dur - power/4;
	
	// Need a unique sprite for this.
	if (!(in = make_new_buff(cn, SK_SHIFT, BUF_SPR_SHIFT, power, dur, 1))) 
		return 0;
	
	if (IS_SHIFTED(cn)) bspmod = 105;
	
	bu[in].data[2] = ch[cn].spell_mod - bspmod;
	
	return add_spell(co, in); // SK_SHIFT
}
void skill_shift(int cn, int force)
{
	int n, tmp;
	short sp1=0, sp2=0;
	
	if (!force)
	{
		if (is_exhausted(cn)) { return; }
		if (!IS_LYCANTH(cn))  { return; }
		if (!IS_PLAYER(cn))   { return; }
		if (has_buff(cn, SK_SHIFT)) 
		{
			do_char_log(cn, 0, "You cannot do that yet!\n");
			return; 
		}
		if (!spell_shift(cn, cn, M_SK(cn, SK_SHIFT)))
		{
			do_char_log(cn, 0, "Hmm. That didn't work. Try again?\n");
			return;
		}
	}
	
	sp1 = (short)(ch[cn].class % 1<<16);
	sp2 = (short)(ch[cn].class / 1<<16);
	
	if (IS_SHIFTED(cn))		// Change from Greenling to Ratling
	{
		ch[cn].kindred &= ~KIN_SHIFTED;
		sp2 = ch[cn].sprite;
		if (sp1>0)	ch[cn].sprite = sp1;
		else		ch[cn].sprite = 14288;
	}
	else					// Change from Ratling to Greenling
	{
		ch[cn].kindred |=  KIN_SHIFTED;
		sp1 = ch[cn].sprite;
		if (sp2>0)	ch[cn].sprite = sp2;
		else		ch[cn].sprite = 31696;
	}
	
	ch[cn].class = (sp2 * 1<<16) + sp1;
	
	remove_all_spells(cn, 1); // Dispel self
	
	for (n = 0; n < 6; n++)
	{
		// Willpower <-> Agility
		tmp = ch[cn].attrib[AT_WIL][n];
		ch[cn].attrib[AT_WIL][n] = ch[cn].attrib[AT_AGL][n];
		ch[cn].attrib[AT_AGL][n] = tmp;
		
		// Intuition <-> Strength
		tmp = ch[cn].attrib[AT_INT][n];
		ch[cn].attrib[AT_INT][n] = ch[cn].attrib[AT_STR][n];
		ch[cn].attrib[AT_STR][n] = tmp;
		
		// Hitpoints <-> Mana
		tmp = ch[cn].hp[n];
		ch[cn].hp[n] = ch[cn].mana[n];
		ch[cn].mana[n] = tmp;
		
		// Regen <-> Meditate
		tmp = ch[cn].skill[SK_REGEN][n];
		ch[cn].skill[SK_REGEN][n] = ch[cn].skill[SK_MEDIT][n];
		ch[cn].skill[SK_MEDIT][n] = tmp;
		
		// Tactics <-> Blind
		tmp = ch[cn].skill[SK_TACTICS][n];
		ch[cn].skill[SK_TACTICS][n] = ch[cn].skill[SK_BLIND][n];
		ch[cn].skill[SK_BLIND][n] = tmp;
		
		// Cleave <-> Blast
		tmp = ch[cn].skill[SK_CLEAVE][n];
		ch[cn].skill[SK_CLEAVE][n] = ch[cn].skill[SK_BLAST][n];
		ch[cn].skill[SK_BLAST][n] = tmp;
		
		// Taunt <-> Poison
		tmp = ch[cn].skill[SK_TAUNT][n];
		ch[cn].skill[SK_TAUNT][n] = ch[cn].skill[SK_POISON][n];
		ch[cn].skill[SK_POISON][n] = tmp;
		
		// Weaken <-> Slow
		tmp = ch[cn].skill[SK_WEAKEN][n];
		ch[cn].skill[SK_WEAKEN][n] = ch[cn].skill[SK_SLOW][n];
		ch[cn].skill[SK_SLOW][n] = tmp;
		
		// Curse <-> Surround Hit
		tmp = ch[cn].skill[SK_CURSE][n];
		ch[cn].skill[SK_CURSE][n] = ch[cn].skill[SK_SURROUND][n];
		ch[cn].skill[SK_SURROUND][n] = tmp;
	}
	
	tmp = ch[cn].a_hp;
	ch[cn].a_hp = ch[cn].a_mana;
	ch[cn].a_mana = tmp;
	
	do_update_char(cn);
	
	if (!force)
	{
		do_char_log(cn, 1, "Your body transforms...\n");
		add_exhaust(cn, TICKS/2);
	}
}

int spell_calm(int cn, int co, int power)
{
	int in, tmp = 0;
	
	// Need custom sprite
	if (!(in = make_new_buff(cn, SK_CALM, BUF_SPR_CALM, power, SP_DUR_RAGE, 1))) 
		return 0;
	
	// Tarot - Hermit R
	if (get_tarot(co, IT_CH_HERMIT_R)) 
	{
		bu[in].data[2] = 2;
		bu[in].end[0]  = -(ch[co].a_end /2000);
	}
	else
	{
		bu[in].data[2] = 3;
		bu[in].mana[0] = -(ch[co].a_mana/2000);
	}
	
	if (T_LYCA_SK(co, 7))
	{
		tmp = ((ch[co].hp[5]*1000 - ch[co].a_hp)/1000) + ((ch[co].end[5]*1000 - ch[co].a_end)/1000) + ((ch[co].mana[5]*1000 - ch[co].a_mana)/1000);
		power = power + (power * tmp / 4000);
	}
	
	if (IS_SEYAN_DU(co))
	{
		bu[in].data[3] 			= min(127, power/ 6 + 5);
		bu[in].data[4]			= power*2/3;
	}
	else
	{
		bu[in].data[3] 			= min(127, power/ 4 + 5);
		bu[in].data[4]			= power*2/2;
	}
	
	bu[in].flags |= IF_PERMSPELL;
	
	return cast_a_spell(cn, co, in, 0, 1); // SK_CALM
}
int spell_rage(int cn, int co, int power)
{
	int in, tmp = 0;
	
	// Need custom sprite?
	if (!(in = make_new_buff(cn, SK_RAGE, BUF_SPR_RAGE, power, SP_DUR_RAGE, 1))) 
		return 0;
	
	// Tarot - Hermit R
	if (get_tarot(co, IT_CH_HERMIT_R)) 
	{
		bu[in].data[2] = 2;
		bu[in].end[0]  = -(ch[co].a_end /2000);
	}
	else
	{
		bu[in].data[2] = 1;
		bu[in].hp[0]   = -(ch[co].a_hp  /2000);
	}
	
	if (T_LYCA_SK(co, 7))
	{
		tmp = ((ch[co].hp[5]*1000 - ch[co].a_hp)/1000) + ((ch[co].end[5]*1000 - ch[co].a_end)/1000) + ((ch[co].mana[5]*1000 - ch[co].a_mana)/1000);
		power = power + (power * tmp / 4000);
	}
	
	if (IS_SEYAN_DU(co))
	{
		bu[in].top_damage[1] 	= min(127, power/ 6 + 5);
		bu[in].data[4]			= power*2/3;
	}
	else
	{
		bu[in].top_damage[1] 	= min(127, power/ 4 + 5);
		bu[in].data[4]			= power*2/2;
	}
	
	bu[in].flags |= IF_PERMSPELL;
	
	return cast_a_spell(cn, co, in, 0, 1); // SK_RAGE
}
void skill_rage(int cn)
{
	int in, power;
	
	power = M_SK(cn, SK_RAGE);
	power = skill_multiplier(power, cn);
	
	if (is_exhausted(cn)) 							{ return; }
	if (!IS_SHIFTED(cn) && has_buff(cn, SK_RAGE))
	{
		do_char_log(cn, 1, "Rage no longer active.\n");
		remove_buff(cn, SK_RAGE);
		if ((in = has_buff(cn, SK_CALM)) && (bu[in].active>(bu[in].duration-TICKS*5)))
		{
			do_char_log(cn, 1, "Calm no longer active.\n");
			remove_buff(cn, SK_CALM);
		}
		do_update_char(cn);
		return;
	}
	if (IS_SHIFTED(cn) && has_buff(cn, SK_CALM))
	{
		do_char_log(cn, 1, "Calm no longer active.\n");
		remove_buff(cn, SK_CALM);
		if ((in = has_buff(cn, SK_RAGE)) && (bu[in].active>(bu[in].duration-TICKS*5)))
		{
			do_char_log(cn, 1, "Rage no longer active.\n");
			remove_buff(cn, SK_RAGE);
		}
		do_update_char(cn);
		return;
	}
	if (spellcost(cn, SP_COST_RAGE, SK_RAGE, 0))	{ return; }
	
	if (IS_SHIFTED(cn))
		spell_calm(cn, cn, power);
	else
		spell_rage(cn, cn, power);

	add_exhaust(cn, SK_EXH_RAGE);
}

int spell_pomesol(int cn, int co, int power, int flag)
{
	int in, temp, dur;
	
	if (flag)
	{
		temp = SK_POME;
		dur = SP_DUR_POME;
	}
	else
	{
		temp = SK_SOL;
		dur = SP_DUR_SOL;
	}
	
	if (!(in = make_new_buff(cn, temp, BUF_SPR_POME, power, dur, 1))) 
		return 0;
	
	bu[in].hp[0] = power*1000/dur;
	
	return cast_a_spell(cn, co, in, 0, 1); // SK_POME & SK_SOL
}

int spell_bloodletting(int cn, int co, int power)
{
	if (surround_cast(cn, 0, 0, SK_BLOODLET, power*2, -1, -1))
	{
		do_char_log(cn, 1, "Your foes began bleeding!\n");
		return 1;
	}
	else
	{
		return 0;
	}
}

int spell_starlight(int cn, int co, int power)
{
	int in;
	
	if (!(in = make_new_buff(cn, SK_STARLIGHT, BUF_SPR_STARL, power, SP_DUR_STARL, 1))) 
		return 0;
	
	bu[in].spell_mod[1] = power/50+1;
	
	return cast_a_spell(cn, co, in, 0, 1); // SK_STARLIGHT
}

int spell_phalanx(int cn, int co, int power)
{
	int in;
	
	if (!(in = make_new_buff(cn, SK_PHALANX, BUF_SPR_PHALANX, power, SP_DUR_PHALANX, 1))) 
		return 0;
	
	bu[in].dmg_reduction[1] = min(100, power*2/5);	// 1% less damage taken per 5 power
	bu[in].data[4] = 1; 							// Effects not removed by NMZ (SK_PHALANX)
	
	return cast_a_spell(cn, co, in, 0, 1); // SK_PHALANX
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
		return -1;                            // empty string does not match
	}
	if (name[0]=='0' && name[1]=='\0')
	{
		return 0;                             // special case: "0".
	}
	n = atoi(name);                                 // try numeric
	if (!IS_SANESKILL(n))
	{
		return -1;                            // bad numeric
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
	return -1;
}

void skill_driver(int cn, int nr)
{
	int tmp = 0, nmz = 0;
	
	if (nr!=50 && nr!=51 && nr!=52 && nr!=53 && nr!=54 && !B_SK(cn, nr))
	{
		do_char_log(cn, 0, "You cannot use this skill/spell.\n");
		return;
	}
	if (nr==SK_IDENT && !(ch[cn].kindred & KIN_IDENTIFY)) return;
	if ((nr==SK_FEROC||nr==SK_SHIFT) && !IS_LYCANTH(cn)) return;
	
	nmz = (ch[cn].flags & CF_NOMAGIC);
	
	switch(nr)
	{
		case SK_BLIND:		skill_blind(cn);	break;
		case SK_CLEAVE:		skill_cleave(cn);	break;
		case SK_LEAP:		skill_leap(cn,0);	break;
		case SK_RAGE:		skill_rage(cn);		break;	
		case SK_REPAIR:		skill_repair(cn);	break;
		case SK_SHIFT:		skill_shift(cn,0);	break;
		case SK_TAUNT:		skill_taunt(cn);	break;
		case SK_WARCRY:		skill_warcry(cn);	break;
		case SK_WEAKEN:		skill_weaken(cn);	break;	
		
		case SK_BLAST:		if (nmz) nomagic(cn,0); else skill_blast(cn);		break;
		case SK_BLESS:		if (nmz) nomagic(cn,0); else skill_bless(cn);		break;
		case SK_CURSE:		if (nmz) nomagic(cn,0); else skill_curse(cn);		break;
		case SK_DISPEL:		if (nmz) nomagic(cn,0); else skill_dispel(cn);		break;
		case SK_ENHANCE:	if (nmz) nomagic(cn,0); else skill_enhance(cn, 0);	break;
		case SK_GHOST:		if (nmz) nomagic(cn,0); else skill_ghost(cn);		break;
		case SK_HASTE:		if (nmz) nomagic(cn,0); else skill_haste(cn);		break;
		case SK_HEAL:		if (nmz) nomagic(cn,0); else skill_heal(cn);		break;
		case SK_IDENT:		if (nmz) nomagic(cn,0); else skill_identify(cn);	break;
		case SK_LETHARGY:	if (nmz) nomagic(cn,0); else skill_lethargy(cn);	break;
		case SK_LIGHT:		if (nmz) nomagic(cn,0); else skill_light(cn);		break;
		case SK_MSHIELD:	if (nmz) nomagic(cn,0); else skill_mshield(cn);		break;
		case SK_POISON:		if (nmz) nomagic(cn,0); else skill_poison(cn);		break;
		case SK_PROTECT:	if (nmz) nomagic(cn,0); else skill_protect(cn, 0);	break;
		case SK_PULSE:		if (nmz) nomagic(cn,0); else skill_pulse(cn);		break;
		case SK_RECALL:		if (nmz) nomagic(cn,0); else skill_recall(cn);		break;
		case SK_SHADOW:		if (nmz) nomagic(cn,0); else skill_shadow(cn);		break;
		case SK_SLOW:		if (nmz) nomagic(cn,0); else skill_slow(cn, 0);		break;
		
		case SK_REGEN:
		case SK_REST:
		case SK_MEDIT:
		case SK_ARIA:
		case SK_FEROC:
		case SK_GEARMAST:
		case SK_GCMASTERY:
		case SK_PERCEPT:
		case SK_PROX:
		case SK_SAFEGRD:
		case SK_STEALTH:
			do_char_log(cn, 0, "You use this skill automatically.\n");
			break;
			
		case SK_ECONOM:
			do_char_log(cn, 0, "You use this skill automatically while shopping or casting spells.\n");
			break;
		
		case SK_HAND:
		case SK_AXE:
		case SK_DAGGER:
		case SK_DUAL:
		case SK_FINESSE:
		case SK_STAFF:
		case SK_SWORD:
		case SK_TWOHAND:
		case SK_PRECISION:
		case SK_SURROUND:
		case SK_TACTICS:
		case SK_ZEPHYR:
			do_char_log(cn, 0, "You use this skill automatically when you fight.\n");
			break;
		
		case SK_SHIELD:
			if (get_gear(cn, IT_WP_THEWALL)) 
				skill_bash(cn);
			else 
				do_char_log(cn, 0, "You use this skill automatically when you fight.\n");
			break;
		
		case SK_IMMUN:
		case SK_RESIST:
			do_char_log(cn, 0, "You use this skill automatically when someone casts negative spells on you.\n");
			break;
			
		case SK_METABOLISM:
			do_char_log(cn, 0, "You use this skill automatically while underwater or taking damage over time.\n");
			break;
		
		default:
			do_char_log(cn, 0, "You cannot use this skill/spell.\n");
			break;
	}
}

/*************************************************************************

   This file is part of 'Mercenaries of Astonia v2'
   Copyright (c) 1997-2001 Daniel Brockhaus (joker@astonia.com)
   All rights reserved.

 **************************************************************************/

#include "server.h"

void look_item_details(int cn, int in)
{
	int act, in2 = 0;
	
	if (it[in].active)
	{
		act = 1;
	}
	else
	{
		act = 0;
	}
	
	if (it[in].max_age[act] || it[in].max_damage)
	{
		if (it[in].damage_state==0)
		{
			do_char_log(cn, 1, "It's in perfect condition.\n");
		}
		else if (it[in].damage_state==1)
		{
			do_char_log(cn, 1, "It's showing signs of age.\n");
		}
		else if (it[in].damage_state==2)
		{
			do_char_log(cn, 1, "It's fairly old.\n");
		}
		else if (it[in].damage_state==3)
		{
			do_char_log(cn, 1, "It is old.\n");
		}
		else if (it[in].damage_state==4)
		{
			do_char_log(cn, 0, "It is very old and battered.\n");
		}
	}
	if (IS_BUILDING(cn))
	{
		do_char_log(cn, 1, "Temp: %d, Sprite: %d,%d.\n", it[in].temp, it[in].sprite[0], it[in].sprite[1]);
		do_char_log(cn, 1, "In-Active Age %d of %d.\n", it[in].current_age[0], it[in].max_age[0]);
		do_char_log(cn, 1, "Active Age %d of %d.\n", it[in].current_age[1], it[in].max_age[1]);
		do_char_log(cn, 1, "Damage %d of %d.\n", it[in].current_damage, it[in].max_damage);
		do_char_log(cn, 1, "Active %d of %d.\n", it[in].active, it[in].duration);
		do_char_log(cn, 1, "Driver=%d [%d,%d,%d,%d,%d,%d,%d,%d,%d,%d].\n",
					it[in].driver, it[in].data[0], it[in].data[1], it[in].data[2], it[in].data[3], it[in].data[4],
					it[in].data[5], it[in].data[6], it[in].data[7], it[in].data[8], it[in].data[9]);
	}
	if (ch[cn].flags & CF_GOD)
	{
		do_char_log(cn, 2, "ID=%d, Temp=%d, Value: %dG %dS.\n", in, it[in].temp, it[in].value / 100, it[in].value % 100);
		do_char_log(cn, 2, "active=%d, sprite=%d/%d\n", it[in].active, it[in].sprite[0], it[in].sprite[1]);
		do_char_log(cn, 2, "max_age=%d/%d, current_age=%d/%d\n", it[in].max_age[0], it[in].max_age[1], it[in].current_age[0], it[in].current_age[1]);
		do_char_log(cn, 2, "max_damage=%d, current_damage=%d\n", it[in].max_damage, it[in].current_damage);
	}
	in2 = ch[cn].citem;
	/* CS, 000208: Check for sane item */
	if (IS_SANEITEM(in2))
	{
		do_char_log(cn, 1, " \n");
		do_char_log(cn, 1, "You compare it with a %s:\n", it[in2].name);
		if (it[in].weapon[0]>it[in2].weapon[0])
		{
			do_char_log(cn, 1, "A %s is the better weapon.\n", it[in].name);
		}
		else if (it[in].weapon[0]<it[in2].weapon[0])
		{
			do_char_log(cn, 1, "A %s is the better weapon.\n", it[in2].name);
		}
		else
		{
			do_char_log(cn, 1, "No difference as a weapon.\n");
		}

		if (it[in].armor[0]>it[in2].armor[0])
		{
			do_char_log(cn, 1, "A %s is the better armor.\n", it[in].name);
		}
		else if (it[in].armor[0]<it[in2].armor[0])
		{
			do_char_log(cn, 1, "A %s is the better armor.\n", it[in2].name);
		}
		else
		{
			do_char_log(cn, 1, "No difference as armor.\n");
		}
	}
	else
	{
		if (it[in].flags & IF_IDENTIFIED)
		{
			item_info(cn, in, 1);
		}
		do_appraisal(cn, in);
	}
}

void look_extra(int cn, int in)
{
	do_char_log(cn, 1, "%s\n", it[in].description);
	
	// -------- TOWER ITEMS --------
	
	if (it[in].temp==IT_TW_CROWN) // Crown of the First King
		do_char_log(cn, 3, "When equipped, 25%% of mana costs are taken from Endurance instead.\n");
	else if (it[in].temp==IT_TW_CLOAK) // Cloak of Shadows
		do_char_log(cn, 3, "When equipped, 20%% of damage taken is delt to Endurance instead.\n");
	else if (it[in].temp==IT_TW_DREAD) // Dreadplate
		do_char_log(cn, 3, "When equipped, your ghost companion can cast Blast, but becomes worse in melee combat.\n");
	else if (it[in].temp==IT_TW_ROOTS) // Commander's Roots
		do_char_log(cn, 3, "When equipped, the effect of Slow and Stun when cast on you are reduced by half.\n");
	else if (it[in].temp==IT_TW_BBELT) // The Black Belt
		do_char_log(cn, 3, "When equipped, your shield parry bonus is added while your off-hand is empty.\n");
	else if (it[in].temp==IT_TW_OUTSIDE) // Outsider's Eye
		do_char_log(cn, 3, "When equipped, enemies beside and behind you no longer gain a bonus to hitting you.\n");
	else if (it[in].temp==IT_TW_PROPHET) // Ring of Prophets
		do_char_log(cn, 3, "When equipped, grants 5%% additional spell power. Grants 10%% spell power while active.\n");
		
	else if (it[in].temp==IT_TW_HEAVENS) // Fists of the Heavens
		do_char_log(cn, 3, "When equipped, the damage bonus granted by your Strength is doubled.\n");
	
	// -------- GLOVE ITEMS --------
	
	else if (it[in].temp==IT_GL_SERPENT)
		do_char_log(cn, 3, "When equipped, grants a 5%% chance to inflict poison on hit.\n");
	else if (it[in].temp==IT_GL_SPIDER)
		do_char_log(cn, 3, "When equipped, grants a 5%% chance to inflict slow on hit.\n");
	else if (it[in].temp==IT_GL_CURSED)
		do_char_log(cn, 3, "When equipped, grants a 5%% chance to inflict curse on hit.\n");
	else if (it[in].temp==IT_GL_TITANS)
		do_char_log(cn, 3, "When equipped, grants a 5%% chance to inflict weaken on hit.\n");
	else if (it[in].temp==IT_GL_BURNING)
		do_char_log(cn, 3, "When equipped, grants a 5%% chance to inflict scorch on hit, causing enemies to take additional damage.\n");
	
	// -------- DRINK ITEMS --------
	
	else if (it[in].temp==BUF_IT_PIGS)
		do_char_log(cn, 3, "Grants 15%% additional maximum hitpoints for its duration.\n");
	else if (it[in].temp==BUF_IT_AALE)
		do_char_log(cn, 3, "Grants 15%% reduced endurance costs for its duration.\n");
	else if (it[in].temp==BUF_IT_DRAG)
		do_char_log(cn, 3, "Grants additional critical strike multiplier for its duration.\n");
	else if (it[in].temp==BUF_IT_MANA)
		do_char_log(cn, 3, "Grants 15%% reduced mana costs for its duration.\n");
	
	// -------- BOOK  ITEMS --------
	
	else if (it[in].temp==IT_BOOK_ALCH)
		do_char_log(cn, 3, "When equipped, healing items are 50%% more effective.\n");
	else if (it[in].temp==IT_BOOK_HOLY)
		do_char_log(cn, 3, "When equipped, maximum healing sickness you can inflict is reduced by 1 stack.\n");
	else if (it[in].temp==IT_BOOK_ADVA)
		do_char_log(cn, 3, "When equipped, debuffs you cast are more likely to pass resistance checks.\n");
	else if (it[in].temp==IT_BOOK_TRAV)
		do_char_log(cn, 3, "When equipped, secondary effects of Braveness and Agility are swapped.\n");
	else if (it[in].temp==IT_BOOK_SWOR)
		do_char_log(cn, 3, "When equipped, each successful parry grants an increased chance to deal critical hits, resetting upon success.\n");
	else if (it[in].temp==IT_BOOK_DAMO)
		do_char_log(cn, 3, "When equipped, increases cooldown recovery speed by 11%%.\n");
	else if (it[in].temp==IT_BOOK_SHIV)
		do_char_log(cn, 3, "When equipped, casting Curse will immediately cast Slow as well. These are less likely to pass resistance checks.\n");
	else if (it[in].temp==IT_BOOK_PROD)
		do_char_log(cn, 3, "When equipped, improves the effectiveness of the Concentrate skill.\n");
	else if (it[in].temp==IT_BOOK_VENO)
		do_char_log(cn, 3, "When equipped, the Poison spell deals 15%% more damage, but has 30%% less duration.\n");
	else if (it[in].temp==IT_BOOK_NECR)
		do_char_log(cn, 3, "When equipped, newly casted Ghost Companions and Shadow Copies become undead monsters.\n");
	
	// -------- OTHER ITEMS --------
	
	else if (it[in].temp==IT_RD_AMMYTHR)
		do_char_log(cn, 3, "When equipped, grants 5%% additional spell power.\n");
	
	// -------- TAROT CARDS --------
	
	else if (it[in].temp==IT_CH_FOOL && !(it[in].flags & IF_SOULSTONE))
		do_char_log(cn, 3, "You can apply a Soulstone to this card. Once applied, it will contribute its bonuses while equipped.\n");
	else if (it[in].temp==IT_CH_MAGI)
		do_char_log(cn, 3, "When equipped, secondary effects of Intuition and Strength are swapped.\n");
	else if (it[in].temp==IT_CH_PREIST)
		do_char_log(cn, 3, "When equipped, your maximum life is reduced by 20%%. 20%% of damage taken from hits is instead dealt to your Mana.\n");
	else if (it[in].temp==IT_CH_EMPRESS)
		do_char_log(cn, 3, "When equipped, Magic Shield will instead be cast on your Ghost Companion when cast by or on you.\n");
	else if (it[in].temp==IT_CH_EMPEROR)
		do_char_log(cn, 3, "When equipped, replaces your Slow spell with an increased cost and effectiveness, but a much more rapid decay.\n");
	else if (it[in].temp==IT_CH_HEIROPH)
		do_char_log(cn, 3, "When equipped, your Dispel spell will no longer affect you or your allies, and instead removes enemy buffs.\n");
	else if (it[in].temp==IT_CH_LOVERS)
		do_char_log(cn, 3, "When equipped, your Weapon Value and Armor Value become the average of your Weapon Value and Armor Value.\n");
	else if (it[in].temp==IT_CH_CHARIOT)
		do_char_log(cn, 3, "When equipped, replaces your Magic Shield with a version which instead grants a temporary Resistance and Immunity effect.\n");
	else if (it[in].temp==IT_CH_STRENGTH)
		do_char_log(cn, 3, "When equipped, reduces your action speed by 20%%, but grants 25%% more damage with melee attacks.\n");
	else if (it[in].temp==IT_CH_HERMIT)
		do_char_log(cn, 3, "When equipped, 10%% more Armor Value, 10%% less Resistance and Immunity.\n");
	else if (it[in].temp==IT_CH_WHEEL)
		do_char_log(cn, 3, "When equipped, your critical hit chance is halved, but your critical hit damage is doubled.\n");
	else if (it[in].temp==IT_CH_JUSTICE)
		do_char_log(cn, 3, "When equipped, your Cleave skill deals 30%% less damage, but inflicts a Bleeding effect for 10 seconds.\n");
	else if (it[in].temp==IT_CH_HANGED)
		do_char_log(cn, 3, "When equipped, 30%% of Resistance is instead used to reduce the strength of incoming enemy spells.\n");
	else if (it[in].temp==IT_CH_DEATH)
		do_char_log(cn, 3, "When equipped, replaces your Weaken skill with an increased cost and improved armor reduction, but does not reduce enemy Weapon Value.\n");
	else if (it[in].temp==IT_CH_TEMPER)
		do_char_log(cn, 3, "When equipped, 20%% chance to avoid damage when hit, 20%% less Armor Value.\n");
	else if (it[in].temp==IT_CH_DEVIL)
		do_char_log(cn, 3, "When equipped, half of all skill and spell costs are instead taken from your Hitpoints.\n");
	else if (it[in].temp==IT_CH_TOWER)
		do_char_log(cn, 3, "When equipped, replaces your Curse spell with an increased cost and effectiveness, but it decays and has reduced duration.\n");
	else if (it[in].temp==IT_CH_STAR)
		do_char_log(cn, 3, "When equipped, replaces your Heal spell with a buff which regenerates the target's Hitpoints over 15 seconds.\n");
	else if (it[in].temp==IT_CH_MOON)
		do_char_log(cn, 3, "When equipped, life regeneration is instead applied as mana regeneration while not at full mana.\n");
	else if (it[in].temp==IT_CH_SUN)
		do_char_log(cn, 3, "When equipped, endurance regeneration is instead applied as life regeneration while not at full life.\n");
	else if (it[in].temp==IT_CH_JUDGE)
		do_char_log(cn, 3, "When equipped, your Blast spell deals 15%% less damage, but inflicts a debuff which increases damage dealt to the target.\n");
	else if (it[in].temp==IT_CH_WORLD)
		do_char_log(cn, 3, "When equipped, mana regeneration is instead applied as endurance regeneration while not at full endurance.\n");
	
	look_item_details(cn, in);
}

void look_door(int cn, int in)
{
	int power;
	
	do_char_log(cn, 1, "%s\n", it[in].description);
	
	// If the door needs a key, it is locked.
	// If you know how to pick locks, you can summarize the door
	// This is done in increments of 20.
	// Ideally each available lockpick (if any) will also increment by 20.
	if (it[in].data[0] && (ch[cn].flags & CF_LOCKPICK))
	{
		// The difficulty of the door to pick
		power = it[in].data[2];
		
		if 	(!power)
			do_char_log(cn, 0, "You're not sure this door can be picked...\n");
		else if (power >=100) 	
			do_char_log(cn, 1, "The lock seems extremely difficult to pick.\n");
		else if (power >= 80) 	
			do_char_log(cn, 1, "The lock seems very difficult to pick.\n");
		else if (power >= 60) 	
			do_char_log(cn, 1, "The lock seems difficult to pick.\n");
		else if (power >= 40) 	
			do_char_log(cn, 1, "The lock seems tricky to pick.\n");
		else if (power >= 20) 	
			do_char_log(cn, 1, "The lock seems simple to pick.\n");
		else
			do_char_log(cn, 1, "The lock seems very simple to pick.\n");
	}
	else if (ch[cn].flags & CF_LOCKPICK)
	{
		do_char_log(cn, 1, "This door appears to be unlocked.\n");
	}
	look_item_details(cn, in);
}

void look_rat_eye(int cn, int in)
{
	int n;

	do_char_log(cn, 1, "%s\n", it[in].description);

	for (n = 0; n<9; n++)
	{
		if (it[in].data[n])
		{
			do_char_log(cn, 1, "The slot for a %s is free.\n", it_temp[it[in].data[n]].name);
		}
	}

}

void look_spell_scroll(int cn, int in)
{
	int n;

	do_char_log(cn, 1, "%s\n", it[in].description);

	n = it[in].data[2];

	do_char_log(cn, 1, "There are %d charge%s left.\n", n, (n==1 ? "s" : ""));
}

void look_driver(int cn, int in)
{
	switch(it[in].driver)
	{
	case     0:
	case    52:
		look_extra(cn, in);
		break;
	case     2:
		look_door(cn, in);
		break;
	case    17:
		look_rat_eye(cn, in);
		break;
	case    48:
		look_spell_scroll(cn, in);
		break;
	default:
		xlog("Unknown look_driver %d", it[in].driver);
		break;
	}
}

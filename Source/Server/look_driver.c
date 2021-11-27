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
	int temp;
	
	do_char_log(cn, 1, "%s\n", it[in].description);
	
	temp = it[in].temp;
	
	switch (it[in].temp)
	{
	// -------- TOWER ITEMS --------
	
	case IT_TW_CROWN: // Crown of the First King
		do_char_log(cn, 3, "When equipped, 25%% of mana costs from spells are instead taken from endurance, and 25%% of endurance costs from skills are instead taken from mana. You no longer lose focus.\n"); 
		break;
	case IT_TW_CLOAK: // Cloak of Shadows
		do_char_log(cn, 3, "When equipped, 20%% of damage taken is negated, and half of the negated damage is dealt to Endurance instead.\n");
		break;
	case IT_TW_DREAD: // The Dreadplate
		do_char_log(cn, 3, "When equipped, your ghost companion is replaced with a spellcaster companion.\n");
		break;
	case IT_TW_DOUSER: // Douser Gauntlets
		do_char_log(cn, 3, "When equipped, grants a 20%% chance on skill hit and a 4%% chance on melee hit to inflict douse on, reducing enemy spell modifier.\n");
		break;
	case IT_TW_MARCH: // Commander's March
		do_char_log(cn, 3, "When equipped, Stun is turned into a speed reduction of 1.50, and all other speed reductions are reduced by half.\n");
		break;
	case IT_TW_BBELT: // The Black Belt
		do_char_log(cn, 3, "When equipped, get WV from your Hand to Hand skill and 2% base crit if your main hand is empty, half of shield parry bonus if your off-hand is empty, and 4%% total base crit if both hands are empty.\n");
		break;
	case IT_TW_OUTSIDE: // Outsider's Eye
		do_char_log(cn, 3, "When equipped, enemies beside and behind you no longer gain a bonus to hitting you.\n");
		break;
	case IT_TW_HEAVENS: // Fists of the Heavens
		do_char_log(cn, 3, "Uses your highest weapon skill for hit and parry scores. When equipped, your highest attribute score is increased by 20%%.\n");
		break;
		
	case IT_TW_ACEDIA: // Sin Dagger
		do_char_log(cn, 0, "When equipped in your main-hand, you get 25%% less skill cooldown, but 25%% less buff and debuff duration. In your off-hand, you get 25%% more buff and debuff duration, but 25%% more skill cooldown.\n");
		break;
	case IT_TW_IRA: // Sin Staff
		do_char_log(cn, 0, "When equipped, your critical hit chance and multiplier effect your Blast spell, but you lose mana over time and cannot deal melee critical hits.\n");
		break;
	case IT_TW_INVIDIA: // Sin Spear
		do_char_log(cn, 0, "When equipped, your ghost companion takes damage instead from hits you take, but it forgets heal and cannot regenerate health.\n");
		break;
	case IT_TW_GULA: // Sin Sword
		do_char_log(cn, 0, "When equipped, you restore health upon hitting, cleaving or blasting an enemy, but you lose hitpoints over time.\n");
		break;
	case IT_TW_LUXURIA: // Sin Axe
		do_char_log(cn, 0, "When equipped, grants a 20%% chance on skill hit and a 4%% chance on melee hit to inflict stun.\n");
		break;
	case IT_TW_AVARITIA: // Sin Twohander
		do_char_log(cn, 0, "When equipped, your lowest attribute increases critical hit multiplier and your highest attribute increases critical hit chance, but you lose endurance over time.\n");
		break;
	case IT_TW_SUPERBIA: // Sin Greataxe
		break;
	case IT_TW_SINBIND: // Sinbinder ring
		do_char_log(cn, 0, "Locks your ring slot.\n");
		temp = it[in].data[2];
		break;
	
	// -------- GLOVE ITEMS --------
	
	case IT_GL_SERPENT: 
		do_char_log(cn, 3, "When equipped, grants a 20%% chance on skill hit and a 4%% chance on melee hit to inflict poison, dealing damage over time.\n");
		break;
	case IT_GL_BURNING: 
		do_char_log(cn, 3, "When equipped, grants a 20%% chance on skill hit and a 4%% chance on melee hit to inflict scorch, causing enemies to take additional damage.\n");
		break;
	case IT_GL_SHADOW: 
		do_char_log(cn, 3, "When equipped, grants a 20%% chance on skill hit and a 4%% chance on melee hit to inflict blind, reducing enemy perception, hit and parry.\n");
		break;
	case IT_GL_CHILLED: 
		do_char_log(cn, 3, "When equipped, grants a 20%% chance on skill hit and a 4%% chance on melee hit to inflict slow, reducing enemy action speed.\n");
		break;
	case IT_GL_CURSED: 
		do_char_log(cn, 3, "When equipped, grants a 20%% chance on skill hit and a 4%% chance on melee hit to inflict curse, reducing enemy attributes.\n");
		break;
	case IT_GL_TITANS: 
		do_char_log(cn, 3, "When equipped, grants a 20%% chance on skill hit and a 4%% chance on melee hit to inflict weaken, reducing enemy weapon and armor values.\n");
		break;
	case IT_GL_BLVIPER: 
		do_char_log(cn, 3, "When equipped, grants a 20%% chance on skill hit and a 4%% chance on melee hit to inflict frostburn, draining enemy endurance and mana.\n");
		break;
		
	// -------- BOOT  ITEMS --------
	
	case IT_BT_NATURES: 
		do_char_log(cn, 3, "When equipped, grants unreduced regeneration rates while walking.\n");
		break;
		
	// -------- AMMY  ITEMS --------
	
	case IT_ANKHAMULET: 
		do_char_log(cn, 3, "When equipped, grants additional passive regeneration bonuses.\n");
		break;
	case IT_AMBERANKH: 
		do_char_log(cn, 3, "When equipped, grants additional passive regeneration bonuses. Grants improved life regeneration.\n");
		break;
	case IT_TURQUANKH: 
		do_char_log(cn, 3, "When equipped, grants additional passive regeneration bonuses. Grants improved endurance regeneration.\n");
		break;
	case IT_GARNEANKH: 
		do_char_log(cn, 3, "When equipped, grants additional passive regeneration bonuses. Grants improved mana regeneration.\n");
		break;
	case IT_TRUEANKH: 
		do_char_log(cn, 3, "When equipped, grants very strong passive regeneration bonuses.\n");
		break;
	case IT_AM_BLOODS: 
		do_char_log(cn, 3, "When equipped, penalizes meditate, but greatly improves the effect of regenerate.\n");
		break;
	case IT_AM_VERDANT: 
		do_char_log(cn, 3, "When equipped, penalizes regenerate, but greatly improves the effect of rest.\n");
		break;
	case IT_AM_SEABREZ: 
		do_char_log(cn, 3, "When equipped, penalizes rest, but greatly improves the effect of meditate.\n");
		break;
	case IT_AM_OBSIDI: 
		do_char_log(cn, 3, "When equipped, increases the radius of area-of-effect skills by 2.\n");
		break;
	case IT_AM_TRUESUN: 
		do_char_log(cn, 3, "When equipped, you are immune to areas that would disable your magic.\n");
		break;
	case IT_AM_FALMOON: 
		do_char_log(cn, 3, "When equipped, you cannot cast or receive spells of any kind.\n");
		break;
	case IT_GAMBLERFAL: 
		do_char_log(cn, 3, "When equipped, this item activates for 4 seconds and grants 10 hitpoints upon dealing a critical hit.\n");
		break;
		
	// -------- BELT  ITEMS --------
	
	case IT_BL_SOLAR: 
	case IT_BL_LUNAR: 
	case IT_BL_ECLIPSE: 
		do_char_log(cn, 3, "Grants passive bonuses during the day (6:00 to 18:00), and active bonuses during the night (18:00 to 6:00).\n");
		break;
	case IT_WHITEBELT: // The White Belt
		do_char_log(cn, 3, "When equipped, get WV from your Hand to Hand skill and 1% base crit if your main hand and off hand are empty.\n");
		break;
		
	// -------- CASINO  ITEMS --------
		
	case IT_SILVAMMY: 
	case IT_SILVBELT: 
		if (!(it[in].flags & IF_SOULSTONE))
			do_char_log(cn, 3, "Can be soulstoned.\n");
		break;
	case IT_MISERRING: // Miser Ring
		do_char_log(cn, 3, "When equipped and activated, grants 50%% more gold from enemies you kill.\n");
		break;
	case IT_FORTERING: // Forte Ring
		do_char_log(cn, 3, "When equipped and activated, grants 50%% more EXP from enemies you kill.\n");
		break;

	// -------- DRINK ITEMS --------
	
	case BUF_IT_PIGS: 
		do_char_log(cn, 3, "Grants 15%% additional maximum hitpoints for its duration.\n");
		break;
	case BUF_IT_AALE: 
		do_char_log(cn, 3, "Grants 15%% reduced endurance costs for its duration.\n");
		break;
	case BUF_IT_DRAG: 
		do_char_log(cn, 3, "Grants additional critical strike multiplier for its duration.\n");
		break;
	case BUF_IT_MANA: 
		do_char_log(cn, 3, "Grants 15%% reduced mana costs for its duration.\n");
		break;
	case IT_POT_OFF: 
		do_char_log(cn, 3, "Grants 10%% more damage dealt for its duration.\n");
		break;
	case IT_POT_DEF: 
		do_char_log(cn, 3, "Grants 10%% less damage taken for its duration.\n");
		break;
	
	// -------- BOOK  ITEMS --------
	
	case IT_BOOK_ALCH: 
		do_char_log(cn, 3, "When equipped, healing items are 50%% more effective.\n");
		break;
	case IT_BOOK_HOLY: 
		do_char_log(cn, 3, "When equipped, maximum healing sickness you can inflict is reduced by 1 stack.\n");
		break;
	case IT_BOOK_ADVA: 
		do_char_log(cn, 3, "When equipped, debuffs you cast are more likely to pass resistance checks.\n");
		break;
	case IT_BOOK_TRAV: 
		do_char_log(cn, 3, "When equipped, secondary effects of Braveness and Agility are swapped.\n");
		break;
	case IT_BOOK_SWOR: 
		do_char_log(cn, 3, "When equipped, each successful parry grants an increased chance to deal critical hits, resetting upon success.\n");
		break;
	case IT_BOOK_DAMO: 
		do_char_log(cn, 3, "When equipped, increases cooldown recovery speed by 11%%.\n");
		break;
	case IT_BOOK_SHIV: 
		do_char_log(cn, 3, "When equipped, casting Curse will immediately cast Slow as well. These are less likely to pass resistance checks.\n");
		break;
	case IT_BOOK_PROD: 
		do_char_log(cn, 3, "When equipped, improves the effectiveness of the Concentrate skill.\n");
		break;
	case IT_BOOK_VENO: 
		do_char_log(cn, 3, "When equipped, the Poison spell deals 25%% more damage, but has 33%% less duration.\n");
		break;
	case IT_BOOK_NECR: 
		do_char_log(cn, 3, "When equipped, newly casted Ghost Companions and Shadow Copies become undead monsters.\n");
		break;
	case IT_BOOK_BISH: 
		do_char_log(cn, 3, "When equipped, casting Bless will immediately cast Protect and Enhance as well.\n");
		break;
	case IT_BOOK_GREA: 
		do_char_log(cn, 3, "When equipped, damage dealt to the duration of your Magic Shield and Magic Shell is halved.\n");
		break;
	
	// -------- UNIQUE ITEMS --------
	
	case IT_WP_LIFESPRIG: 
		do_char_log(cn, 3, "When equipped, mana spent is restored as life over 10 seconds. This effect is overwritten by stronger sources.\n");
		break;
	case IT_WP_BLOODLET: 
		do_char_log(cn, 3, "When equipped, this weapon can be used to cast 'Bloodletting', costing 25%% of total hitpoints. Bloodletting inflicts bleeding on surrounding enemies.\n");
		break;
	case IT_WP_GEMCUTTER: 
		do_char_log(cn, 3, "When equipped, stats granted by your ring slots are improved by 17%%.\n");
		break;
	case IT_WP_STARLIGHT: 
		do_char_log(cn, 3, "When equipped, this weapon can be used to cast 'Starlight', costing 25%% of total mana. Starlight grants a self-buff, increasing spell modifier for 60 seconds.\n");
		break;
	case IT_WP_KELPTRID: 
		do_char_log(cn, 3, "When equipped, grants +10 to action speed while underwater.\n");
		break;
	case IT_WP_PHALLENX: 
		do_char_log(cn, 3, "When equipped, this shield can be used to cast 'Phalanx', costing 50%% of total endurance. Phalanx grants a self-buff, reducing damage taken for 60 seconds.\n");
		break;
	case IT_WP_LAMEDARG: 
		if (it[in].data[0]<10000)
			do_char_log(cn, 3, "Thine worth shall be proven. %d remain.\n", max(0, 10000-it[in].data[0]));
		else
			do_char_log(cn, 3, "Thou art worthy. Use me when ready.\n");
		break;
	case IT_WP_WHITEODA: 
		do_char_log(cn, 3, "When equipped, grants additional armor value based on total spell modifier.\n");
		break;
	case IT_WP_EXCALIBUR: 
		do_char_log(cn, 3, "When equipped, 10%% of attack speed is granted as additional weapon value.\n");
		break;
	case IT_WP_FELLNIGHT: 
		do_char_log(cn, 3, "When equipped, grants full bonus to weapon value when used by a Templar, Arch-Templar, or Brawler.\n");
		break;
	case IT_WP_BLACKTAC: 
		do_char_log(cn, 3, "When equipped, grants additional weapon value based on total spell modifier.\n");
		break;
	case IT_WP_CRIMRIP: 
		do_char_log(cn, 3, "When equipped, this weapon can be used to cast 'Bloodletting', costing 25%% of total hitpoints. Bloodletting inflicts bleeding on surrounding enemies.\n");
		break;
	case IT_WP_CRESSUN: 
		do_char_log(cn, 3, "When equipped, endurance spent is restored as life over 5 seconds. This effect is overwritten by stronger sources.\n");
		break;
	case IT_WP_GILDSHINE: 
		do_char_log(cn, 3, "When equipped, grants additional critical hit multiplier based off total Bartering score.\n");
		break;
	case IT_WP_BRONCHIT: 
		do_char_log(cn, 3, "When equipped, 10%% of Cleave's damage is also dealt to the target's mana.\n");
		break;
	case IT_WP_VOLCANF: 
		do_char_log(cn, 3, "When equipped, you cannot naturally deal critical hits. If your enemy is scorched, the scorch is removed to guarantee a critical hit.\n");
		break;
		
	case IT_BONEARMOR: 
		do_char_log(cn, 0, "Become undead.\n");
		break;
	
	default:
		break;
	}
	
	switch (temp)
	{
		// -------- TAROT CARDS --------
	
	case IT_CH_FOOL: 
		if (!(it[in].flags & IF_SOULSTONE))
			do_char_log(cn, 3, "You can apply a soulstone to this card. Once applied, it will contribute its bonuses while equipped.\n");
		break;
	case IT_CH_MAGI: 
		do_char_log(cn, 3, "When equipped, secondary effects of Intuition and Strength are swapped.\n");
		break;
	case IT_CH_PREIST: 
		do_char_log(cn, 3, "When equipped, your maximum life is reduced by 20%%. 20%% of damage taken from hits is negated, and half of the negated damage is dealt to Mana instead.\n");
		break;
	case IT_CH_EMPRESS: 
		do_char_log(cn, 3, "When equipped, your Magic Shield spell is replaced with Magic Shell. Magic Shell grants a temporary Resistance and Immunity bonus.\n");
		break;
	case IT_CH_EMPEROR: 
		do_char_log(cn, 3, "When equipped, your Slow spell is replaced with Greater Slow. Greater Slow no longer decays and has an increased duration.\n");
		break;
	case IT_CH_HEIROPH: 
		do_char_log(cn, 3, "When equipped, your Dispel spell will no longer affect you or your allies, and instead removes enemy buffs.\n");
		break;
	case IT_CH_LOVERS: 
		do_char_log(cn, 3, "When equipped, your Weapon Value and Armor Value become the average of your Weapon Value and Armor Value.\n");
		break;
	case IT_CH_CHARIOT: 
		do_char_log(cn, 3, "When equipped, your Blind skill is replaced with Douse. Douse reduces your target's stealth and spell modifier.\n");
		break;
	case IT_CH_STRENGTH: 
		do_char_log(cn, 3, "When equipped, reduces your attack and cast speed by 20%%, but grants 20%% more damage with melee attacks.\n");
		break;
	case IT_CH_HERMIT: 
		do_char_log(cn, 3, "When equipped, you have 20%% more Armor Value, but 10%% less Resistance and Immunity.\n");
		break;
	case IT_CH_WHEEL: 
		do_char_log(cn, 3, "When equipped, your critical hit chance is reduced by 33%%, but you have 1.5x critical hit multiplier.\n");
		break;
	case IT_CH_JUSTICE: 
		do_char_log(cn, 3, "When equipped, your Cleave skill deals 30%% less damage, but inflicts a Bleeding effect for 10 seconds.\n");
		break;
	case IT_CH_HANGED: 
		do_char_log(cn, 3, "When equipped, 25%% of your Resistance is instead used to reduce the strength of incoming enemy spells.\n");
		break;
	case IT_CH_DEATH: 
		do_char_log(cn, 3, "When equipped, your Weaken skill is replaced with Greater Weaken. Greater Weaken improves armor reduction, but no longer reduces enemy Weapon Value.\n");
		break;
	case IT_CH_TEMPER: 
		do_char_log(cn, 3, "When equipped, your Taunt skill grants 30%% less Guard power, but Guard duration is tripled.\n");
		break;
	case IT_CH_DEVIL: 
		do_char_log(cn, 3, "When equipped, 30%% of all skill and spell costs are instead twice taken from your Hitpoints.\n");
		break;
	case IT_CH_TOWER: 
		do_char_log(cn, 3, "When equipped, your Curse spell is replaced with Greater Curse. Greater Curse has increased effect, but decays and has reduced duration.\n");
		break;
	case IT_CH_STAR: 
		do_char_log(cn, 3, "When equipped, your Heal spell is replaced with Regen. Regen grants a buff which regenerates the target's Hitpoints over 20 seconds.\n");
		break;
	case IT_CH_MOON: 
		do_char_log(cn, 3, "When equipped, life regeneration is instead applied as mana regeneration while not at full mana.\n");
		break;
	case IT_CH_SUN: 
		do_char_log(cn, 3, "When equipped, endurance regeneration is instead applied as life regeneration while not at full life.\n");
		break;
	case IT_CH_JUDGE: 
		do_char_log(cn, 3, "When equipped, your Blast spell deals 15%% less damage but inflicts Scorch, causing the target to take additional damage for 20 seconds.\n");
		break;
	case IT_CH_WORLD: 
		do_char_log(cn, 3, "When equipped, mana regeneration is instead applied as endurance regeneration while not at full endurance.\n");
		break;
		
	case IT_CH_FOOL_R: 
		do_char_log(cn, 3, "When equipped, your attributes become the average of all attributes.\n");
		break;
	case IT_CH_MAGI_R: 
		do_char_log(cn, 3, "When equipped, your concentrate skill instead increases the cost of spells, but it also increases your spell modifier.\n");
		break;
	case IT_CH_PREIST_R: 
		do_char_log(cn, 3, "When equipped, your maximum mana is reduced by 20%% to increase your cooldown bonus by 10%% of the subtracted mana.\n");
		break;
	case IT_CH_EMPRES_R:
		do_char_log(cn, 3, "When equipped, your buffs are applied at 125%% of the target's spell aptitude, but your spell aptitude is halved.\n");
		break;
	case IT_CH_EMPERO_R: 
		do_char_log(cn, 3, "When equipped, your Warcry skill is replaced with Rally. Rally grants nearby allies a buff which improves Hit Score and Parry Score.\n");
		break;
	case IT_CH_HEIROP_R: 
		do_char_log(cn, 3, "When equipped, your Ghost Companion shares the bonus granted to you by your other tarot card slot.\n");
		break;
	case IT_CH_LOVERS_R: 
		do_char_log(cn, 3, "When equipped, your Hit Score and Parry Score become the average of your Hit Score and Parry Score.\n");
		break;
	case IT_CH_CHARIO_R: 
		do_char_log(cn, 3, "When equipped, your debuffs ignore 20%% of target resistance and immunity, but are 25%% weaker once applied.\n");
		break;
	case IT_CH_STRENG_R: 
		do_char_log(cn, 3, "When equipped, you have 20%% more Weapon Value, but 20%% less hit score.\n");
		break;
	case IT_CH_HERMIT_R: 
		do_char_log(cn, 3, "When equipped, you may no longer inflict melee critical hits. Instead, your Thorns value has a chance of inflicting a critical hit.\n");
		break;
	case IT_CH_WHEEL_R: 
		do_char_log(cn, 3, "When equipped, you have 20%% more Armor Value, but a 25%% chance to be hit when you would have parried.\n");
		break;
	case IT_CH_JUSTIC_R: 
		do_char_log(cn, 3, "When equipped, your Leap skill deals 30%% less damage, but inflicts a Bleeding effect for 10 seconds\n");
		break;
	case IT_CH_HANGED_R: 
		do_char_log(cn, 3, "When equipped, you have 20%% more Top Damage, but 10%% less Weapon Value.\n");
		break;
	case IT_CH_DEATH_R: 
		do_char_log(cn, 3, "When equipped, your Zephyr skill grants a bonus to Resistance instead of Immunity. Zephyr triggers on parry instead of on hit, and earns a damage bonus from Thorns instead of Attack Speed.\n");
		break;
	case IT_CH_TEMPER_R: 
		do_char_log(cn, 3, "When equipped, you gain 5%% more Weapon Value per stack of Healing Sickness on you. The maximum healing sickness you can receive is increased by 1 stack.\n");
		break;
	case IT_CH_DEVIL_R: 
		do_char_log(cn, 3, "When equipped, your Shadow Copy deals 20%% more damage and takes 20%% less damage, but while your Shadow Copy is active you deal 20%% less damage and take 20%% more damage.\n");
		break;
	case IT_CH_TOWER_R: 
		do_char_log(cn, 3, "When equipped, your Poison spell is replaced with Venom. Venom deals twice as much damage and reduces enemy Resistance instead of Immunity, but it cannot stack.\n");
		break;
	case IT_CH_STAR_R: 
		do_char_log(cn, 3, "When equipped, your Spell Modifier no longer effects spell power and instead effects your Critical Hit Chance.\n");
		break;
	case IT_CH_MOON_R: 
		do_char_log(cn, 3, "When equipped, the effectiveness of your Meditate skill is tripled while fighting, but zero while stationary.\n");
		break;
	case IT_CH_SUN_R: 
		do_char_log(cn, 3, "When equipped, the effectiveness of your Regenerate skill is tripled while fighting, but zero while stationary.\n");
		break;
	case IT_CH_JUDGE_R: 
		do_char_log(cn, 3, "When equipped, you cannot cast your Pulse spell yourself. Pulse is cast on your Ghost Companion upon creation and is permanent, but 20%% of damage is taken by your Ghost Companion each pulse.\n");
		break;
	case IT_CH_WORLD_R: 
		do_char_log(cn, 3, "When equipped, the effectiveness of your Rest skill is tripled while fighting, but zero while stationary.\n");
		break;
		
	default:
		break;
	}
	
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

void look_soulstone(int cn, int in)
{
	int t, nt=0;
	char *tag_name[N_SOULTAGS];
	
	do_char_log(cn, 1, "%s\n", it[in].description);
	
	if (it[in].data[2])
	{
		do_char_log(cn, 2, "Has a focus of %d.\n", it[in].data[2]);
	}
	
	for (t=0;t<N_SOULTAGS;t++)
	{
		if (it[in].data[t+3]>0 && it[in].data[t+3]<N_SOULCAT)
		{
			chlog(cn, "look_soulstone : strcpy tag name (%d)", it[in].data[t+3]);
			//strcpy(tag_name[nt], get_soulname(it[in].data[t+3]));
			tag_name[nt] = get_soulname(it[in].data[t+3]);
			nt++;
		}
	}
	
	if (nt)
	{
		chlog(cn, "look_soulstone : display tag names");
		do_char_log(cn, 1, "This stone has the following tags:\n");
		for (t=0;t<nt;t++)
		{
			do_char_log(cn, 2, "%d. %s:\n", t+1, tag_name[t]);
		}
	}
	
	look_item_details(cn, in);
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
	case    68:
		look_soulstone(cn, in);
		break;
	default:
		xlog("Unknown look_driver %d", it[in].driver);
		break;
	}
}

/*************************************************************************

   This file is part of 'Mercenaries of Astonia v2'
   Copyright (c) 1997-2001 Daniel Brockhaus (joker@astonia.com)
   All rights reserved.

 **************************************************************************/

#include "server.h"

// Font Colors:
// 0 = Red
// 1 = Yellow
// 2 = Green
// 3 = Blue
// 4 = Pink
// 5 = Orange
// 6 = Lime
// 7 = Skyblue
// 8 = Violet
// 9 = White

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
		do_char_log(cn, 2, "driver=%d, active=%d, sprite=%d/%d\n", it[in].driver, it[in].active, it[in].sprite[0], it[in].sprite[1]);
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
			}
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
		do_appraisal(cn, in);
	}
}

void look_extra(int cn, int in)
{
	int temp, n, m=0, mm=0;
	
	do_char_log(cn, 1, "%s\n", it[in].description);
	
	temp = it[in].temp;
	
	if (!temp && it[in].orig_temp) temp = it[in].orig_temp;
	
	switch (temp)
	{
	// -------- TOWER ITEMS --------
	
	case IT_TW_CROWN: // Crown of the First King
		do_char_log(cn, 7, "When equipped, 25%% of skill costs are nullified, 25%% of mana costs from spells are taken from endurance, and 25%% of endurance costs from skills are taken from mana. You no longer lose focus.\n"); 
		break;
	case IT_TW_CLOAK: // Cloak of Shadows
		do_char_log(cn, 7, "When equipped, 20%% of damage taken is negated, and half of the negated damage is dealt to Endurance instead.\n");
		break;
	case IT_TW_DREAD: // The Dreadplate
		do_char_log(cn, 7, "When equipped, your ghost companion is replaced with a spellcaster companion.\n");
		break;
	case IT_TW_DOUSER: // Douser Gauntlets
		do_char_log(cn, 7, "When equipped, grants a 20%% chance on skill hit and a 4%% chance on melee hit to inflict douse on, reducing enemy spell modifier.\n");
		break;
	case IT_TW_MARCH: // Commander's March
		do_char_log(cn, 7, "When equipped, Stun is turned into a speed reduction of 1.50, and all other speed reductions are reduced by one third.\n");
		break;
	case IT_TW_BBELT: // The Black Belt
		do_char_log(cn, 7, "When equipped, get WV from your Hand to Hand skill and +2%% base crit if your main hand is empty, half of shield parry bonus if your off-hand is empty, and 5%% total base crit if both hands are empty.\n");
		break;
	case IT_TW_OUTSIDE: // Outsider's Eye
		do_char_log(cn, 7, "When equipped, enemies beside and behind you no longer gain a bonus to hitting you.\n");
		break;
	case IT_TW_HEAVENS: // Fists of the Heavens
		do_char_log(cn, 7, "Uses your highest weapon skill for hit and parry scores. When equipped, your highest attribute score is increased by 20%%.\n");
		break;
		
	case IT_TW_ACEDIA: // Sin Dagger
		do_char_log(cn, 4, "When equipped in your main-hand, you get 25%% less skill cooldown, but 25%% less buff and debuff duration. In your off-hand, you get 50%% more buff and debuff duration, but 50%% more skill cooldown.\n");
		break;
	case IT_TW_IRA: // Sin Staff
		do_char_log(cn, 4, "When equipped, your critical hit chance and half of critical multiplier effect your Blast spell, but you lose mana over time and cannot deal melee critical hits.\n");
		break;
	case IT_TW_INVIDIA: // Sin Spear
		do_char_log(cn, 4, "When equipped, your ghost companion has surround hit AoE, and your shadow copy teleports to new targets, but each cannot regenerate health.\n");
		break;
	case IT_TW_GULA: // Sin Sword
		do_char_log(cn, 4, "When equipped, you restore health upon hitting, cleaving or blasting an enemy, but you lose hitpoints over time.\n");
		break;
	case IT_TW_LUXURIA: // Sin Axe
		do_char_log(cn, 4, "When equipped, grants a 25%% chance on skill hit and a 5%% chance on melee hit to inflict stun.\n");
		break;
	case IT_TW_AVARITIA: // Sin Twohander
		do_char_log(cn, 4, "When equipped, twice your lowest attribute increases critical hit chance and half your highest attribute increases critical hit multiplier, but you lose endurance over time.\n");
		break;
	case IT_TW_SUPERBIA: // Sin Greataxe
		do_char_log(cn, 4, "When equipped, the effects of your Heal or Regen spell is halved.\n");
		break;
	case IT_TW_SINBIND: // Sinbinder ring
		do_char_log(cn, 4, "You may only equip one Sinbinder at a time.\n");
		temp = it[in].data[2];
		break;
	
	// -------- GLOVE ITEMS --------
	
	case IT_GL_SERPENT: 
		do_char_log(cn, 5, "When equipped, grants a 25%% chance on skill hit and a 5%% chance on melee hit to inflict poison, dealing damage over time.\n");
		break;
	case IT_GL_BURNING: 
		do_char_log(cn, 5, "When equipped, grants a 25%% chance on skill hit and a 5%% chance on melee hit to inflict scorch, causing enemies to take additional damage.\n");
		break;
	case IT_GL_SHADOW: 
		do_char_log(cn, 5, "When equipped, grants a 25%% chance on skill hit and a 5%% chance on melee hit to inflict blind, reducing enemy perception, hit and parry.\n");
		break;
	case IT_GL_CHILLED: 
		do_char_log(cn, 5, "When equipped, grants a 25%% chance on skill hit and a 5%% chance on melee hit to inflict slow, reducing enemy action speed.\n");
		break;
	case IT_GL_CURSED: 
		do_char_log(cn, 5, "When equipped, grants a 25%% chance on skill hit and a 5%% chance on melee hit to inflict curse, reducing enemy attributes.\n");
		break;
	case IT_GL_TITANS: 
		do_char_log(cn, 5, "When equipped, grants a 25%% chance on skill hit and a 5%% chance on melee hit to inflict weaken, reducing enemy weapon value.\n");
		break;
	case IT_GL_BLVIPER: 
		do_char_log(cn, 5, "When equipped, grants a 25%% chance on skill hit and a 5%% chance on melee hit to inflict frostburn, draining enemy endurance and mana.\n");
		break;
		
	// -------- BOOT  ITEMS --------
	
	case IT_BT_NATURES: 
		do_char_log(cn, 5, "When equipped, grants 33%% more regeneration rates while idle.\n");
		break;
		
	// -------- AMMY  ITEMS --------
	
	case IT_ANKHAMULET: 
		do_char_log(cn, 5, "When equipped, grants additional passive regeneration bonuses.\n");
		break;
	case IT_AMBERANKH: 
		do_char_log(cn, 5, "When equipped, grants additional passive regeneration bonuses. Grants improved life regeneration.\n");
		break;
	case IT_TURQUANKH: 
		do_char_log(cn, 5, "When equipped, grants additional passive regeneration bonuses. Grants improved endurance regeneration.\n");
		break;
	case IT_GARNEANKH: 
		do_char_log(cn, 5, "When equipped, grants additional passive regeneration bonuses. Grants improved mana regeneration.\n");
		break;
	case IT_TRUEANKH: 
		do_char_log(cn, 5, "When equipped, grants very strong passive regeneration bonuses.\n");
		break;
	case IT_AM_BLOODS: 
		do_char_log(cn, 5, "When equipped, penalizes meditate, but greatly improves the effect of regenerate.\n");
		break;
	case IT_AM_VERDANT: 
		do_char_log(cn, 5, "When equipped, penalizes regenerate, but greatly improves the effect of rest.\n");
		break;
	case IT_AM_SEABREZ: 
		do_char_log(cn, 5, "When equipped, penalizes rest, but greatly improves the effect of meditate.\n");
		break;
	case IT_AM_TRUESUN: 
		do_char_log(cn, 5, "When equipped, you are immune to areas that would disable your magic.\n");
		break;
	case IT_AM_FALMOON: 
		do_char_log(cn, 5, "When equipped, you cannot cast or receive spells of any kind.\n");
		break;
	case IT_GAMBLERFAL: 
		do_char_log(cn, 5, "When equipped, grants 20 hitpoints upon dealing a critical hit, but your chance to deal critical hits is reduced for 4 seconds after dealing one.\n");
		break;
		
	// -------- BELT  ITEMS --------
	
	case IT_BL_SOLAR: 
	case IT_BL_LUNAR: 
	case IT_BL_ECLIPSE: 
		do_char_log(cn, 5, "Grants passive bonuses during the day (6:00 to 18:00), and active bonuses during the night (18:00 to 6:00).\n");
		break;
	case IT_WHITEBELT: // The White Belt
		do_char_log(cn, 5, "When equipped, get WV from your Hand to Hand skill and +2%% base crit if your main hand and off hand are empty.\n");
		break;
		
	// -------- CASINO  ITEMS --------
		
	case IT_SILVAMMY: 
	case IT_SILVBELT: 
		if (!(it[in].flags & IF_SOULSTONE))
			do_char_log(cn, 5, "Can be soulstoned.\n");
		break;
	case IT_MISERRING: // Miser Ring
		do_char_log(cn, 5, "When equipped and activated, grants 50%% more gold from enemies you kill.\n");
		break;
	case IT_FORTERING: // Forte Ring
		do_char_log(cn, 5, "When equipped and activated, grants 25%% more EXP from enemies you kill.\n");
		break;

	// -------- DRINK ITEMS --------
	
	case BUF_IT_PIGS: 
		do_char_log(cn, 5, "Grants 15%% additional maximum hitpoints for its duration.\n");
		break;
	case BUF_IT_AALE: 
		do_char_log(cn, 5, "Grants 15%% reduced endurance costs for its duration.\n");
		break;
	case BUF_IT_DRAG: 
		do_char_log(cn, 5, "Grants 1 reduced maximum healing sickness stack for its duration.\n");
		break;
	case BUF_IT_MANA: 
		do_char_log(cn, 5, "Grants 15%% reduced mana costs for its duration.\n");
		break;
	
	// -------- BOOK  ITEMS --------
	
	case IT_BOOK_ALCH: 
		do_char_log(cn, 5, "When equipped, healing items are 50%% more effective.\n");
		break;
	case IT_BOOK_HOLY: 
		do_char_log(cn, 5, "When equipped, maximum healing sickness you can inflict is reduced by 1 stack.\n");
		break;
	case IT_BOOK_ADVA: 
		do_char_log(cn, 5, "When equipped, debuffs you cast are more likely to pass resistance checks.\n");
		break;
	case IT_BOOK_TRAV: 
		do_char_log(cn, 5, "When equipped, secondary effects of Braveness and Agility are equal to the higher of the two.\n");
		break;
	case IT_BOOK_DAMO: 
		do_char_log(cn, 5, "When equipped, increases cooldown recovery speed by 11%%.\n");
		break;
	case IT_BOOK_SHIV: 
		do_char_log(cn, 5, "When equipped, casting Curse will immediately cast Slow as well. These are less likely to pass resistance checks.\n");
		break;
	case IT_BOOK_PROD: 
		do_char_log(cn, 5, "When equipped, improves the mana effectiveness of the Economize skill.\n");
		break;
	case IT_BOOK_VENO: 
		do_char_log(cn, 5, "When equipped, Poisons and Venoms you inflict deal damage 25%% faster.\n");
		break;
	case IT_BOOK_NECR: 
		do_char_log(cn, 5, "When equipped, newly casted Ghost Companions and Shadow Copies become undead monsters.\n");
		break;
	case IT_BOOK_BISH: 
		do_char_log(cn, 5, "When equipped, casting Bless will immediately cast Protect and Enhance as well.\n");
		break;
	case IT_BOOK_GREA: 
		do_char_log(cn, 5, "When equipped, damage dealt to the duration of your Magic Shield and Magic Shell is halved.\n");
		break;
	case IT_BOOK_DEVI: 
		do_char_log(cn, 5, "When equipped, this book can be used to freely cast Shadow Copy.\n");
		break;
	case IT_BOOK_BURN: 
		do_char_log(cn, 5, "When equipped, your Immolate spell gains 1 additional base power per 20 uncapped hitpoints.\n");
		break;
	case IT_BOOK_VERD: 
		do_char_log(cn, 5, "When equipped, your Dispel spell loses power less quickly and has no removal limit, but no longer immunizes or inoculates.\n");
		break;
	case IT_BOOK_MALT: 
		do_char_log(cn, 5, "When equipped, you no longer lose focus.\n");
		break;
	case IT_BOOK_GRAN: 
		do_char_log(cn, 5, "When equipped, your highest attribute score is increased by 10%%.\n");
		break;
	
	// -------- UNIQUE ITEMS --------
	
	case IT_WP_LIFESPRIG: 
		do_char_log(cn, 5, "When equipped, mana spent is restored as life for 5 seconds. This effect is overwritten by stronger sources.\n");
		break;
	case IT_WP_BLOODLET: 
		do_char_log(cn, 5, "When equipped, this weapon can be used to cast 'Bloodletting', costing 1/3 of uncapped hitpoints. Bloodletting inflicts bleeding on surrounding enemies.\n");
		break;
	case IT_WP_GEMCUTTER: 
		do_char_log(cn, 5, "When equipped, stats granted by your ring slots are improved by 40%%.\n");
		break;
	case IT_WP_STARLIGHT: 
		do_char_log(cn, 5, "When equipped, this weapon can be used to cast 'Starlight', costing 1/3 of uncapped mana. Starlight grants 1 additional spell modifier per 50 mana spent.\n");
		break;
	case IT_WP_KELPTRID: 
		do_char_log(cn, 5, "When equipped, grants +30 to action speed while underwater.\n");
		break;
	case IT_WP_PHALANX: 
		do_char_log(cn, 5, "When equipped, this shield can be used to cast 'Phalanx', costing 1/3 of uncapped endurance. Phalanx grants 1%% damage reduction per 5 endurance spent.\n");
		break;
	case IT_WP_LAMEDARG: 
		if (it[in].data[0]<REQ_LAME)
			do_char_log(cn, 5, "Thine worth shall be proven. %d remain.\n", max(0, REQ_LAME-it[in].data[0]));
		else
			do_char_log(cn, 9, "Thou art worthy. Use me when ready.\n");
		break;
	case IT_WP_WHITEODA: 
		do_char_log(cn, 5, "When equipped, grants additional armor value based on total spell modifier.\n");
		break;
	case IT_WP_EXCALIBUR: 
		do_char_log(cn, 5, "When equipped, 20%% of uncapped attack speed is granted as additional weapon value.\n");
		break;
	case IT_WP_BLACKTAC: 
		do_char_log(cn, 5, "When equipped, grants additional weapon value based on total spell modifier.\n");
		break;
	case IT_WP_CRIMRIP: 
		do_char_log(cn, 5, "When equipped, this weapon can be used to cast 'Bloodletting', costing 1/3 of uncapped hitpoints. Bloodletting inflicts bleeding on surrounding enemies.\n");
		break;
	case IT_WP_CRESSUN: 
		do_char_log(cn, 5, "When equipped, endurance spent is restored as life for 5 seconds. This effect is overwritten by stronger sources.\n");
		break;
	case IT_WP_GILDSHINE: 
		do_char_log(cn, 5, "When equipped, grants additional critical hit multiplier based off total Economize score.\n");
		break;
	case IT_WP_BRONCHIT: 
		do_char_log(cn, 5, "When equipped, 20%% of Cleave's damage is also dealt to the target's mana.\n");
		break;
	case IT_WP_VOLCANF: 
		do_char_log(cn, 5, "When equipped, you cannot naturally deal critical hits. If your enemy is scorched, the scorch is removed to guarantee a critical hit.\n");
		break;
	case IT_WP_QUICKSILV: 
		do_char_log(cn, 5, "When equipped, this weapon grants full damage and shield parry while in the offhand.\n");
		break;
	case IT_WP_PUTRIDIRE: 
		do_char_log(cn, 5, "When equipped, this weapon can be used to freely cast Poison, with power equal to 1/3 of your uncapped mana.\n");
		break;
	case IT_WP_BLOODYSCY: 
		do_char_log(cn, 5, "When equipped, this weapon can be used to freely cast Cleave, with power equal to 1/3 of your uncapped hitpoints.\n");
		break;
	case IT_WP_FLAGBEAR: 
		do_char_log(cn, 5, "When equipped, this weapon can be used to freely cast Rally, with power equal to your uncapped endurance.\n");
		break;
	case IT_WP_RISINGPHO: 
		do_char_log(cn, 5, "When equipped, this shield can be used to freely cast Immolate, with power equal to 1/3 of your uncapped hitpoints, ignoring spell modifier.\n");
		break;
	case IT_WP_THEWALL: 
		do_char_log(cn, 5, "When equipped, your Shield skill becomes Shield Bash. Shield Bash inflicts Stun and deals damage based on your Armor Value.\n");
		break;
	case IT_WP_SOVERIGNS: 
		do_char_log(cn, 5, "When equipped, %s and %s become the average of the two, plus 10.\n", skilltab[it[in].data[1]].name, skilltab[it[in].data[2]].name);
		break;
	case IT_WP_MJOLNIR: 
		do_char_log(cn, 5, "When equipped, this weapon can be used to cast Blast, with power based on your WV and top damage, ignoring spell modifier. Blasts cast this way will inflict a stack of Shock, and grant you a stack of Charge.\n");
		break;
	case IT_WP_CROSSBLAD: 
		do_char_log(cn, 5, "When equipped, Surround Hit has a base radius of 4.\n");
		break;
	case IT_WP_GUNGNIR: 
		do_char_log(cn, 5, "When equipped, bleeding you inflict deals twice as much damage.\n");
		break;
	
	case IT_BONEARMOR: 
		do_char_log(cn, 5, "When equipped, all armor is converted to health regeneration.\n");
		break;
	
	case IT_SIGNET_TE:
	case IT_SIGNET_MR:
	case IT_SIGNET_HA:
	case IT_SIGNET_AT:
	case IT_SIGNET_SK:
	case IT_SIGNET_WA:
	case IT_SIGNET_SO:
	case IT_SIGNET_SU:
	case IT_SIGNET_AH:
	case IT_SIGNET_SE:
	case IT_SIGNET_BR:
	case IT_SIGNET_LY:
	case IT_SIGNET_XX:
	case IT_ICELOTUS: 
		do_char_log(cn, 4, "You may only equip one %s at a time.\n", it[in].name);
		break;
		
	case IT_OS_BRV:
	case IT_OS_WIL:
	case IT_OS_INT:
	case IT_OS_AGL:
	case IT_OS_STR:
		if (temp==IT_OS_BRV) do_char_log(cn, 8, "Grants an implicit +1 to Braveness.\n");
		if (temp==IT_OS_WIL) do_char_log(cn, 8, "Grants an implicit +1 to Willpower.\n");
		if (temp==IT_OS_INT) do_char_log(cn, 8, "Grants an implicit +1 to Intuition.\n");
		if (temp==IT_OS_AGL) do_char_log(cn, 8, "Grants an implicit +1 to Agility.\n");
		if (temp==IT_OS_STR) do_char_log(cn, 8, "Grants an implicit +1 to Strength.\n");
		do_char_log(cn, 1, " \n");
		for (n=0;n<5;n++) m += ch[cn].attrib[n][1];
		do_char_log(cn, 6, "You have used %d out of 10 greater attribute scrolls.\n", m);
		break;
		
	case IT_OS_SK:
		do_char_log(cn, 8, "Grants an implicit +1 to %s.\n", skilltab[it[in].data[1]].name);
		do_char_log(cn, 1, " \n");
		for (n=0;n<50;n++) { m += ch[cn].skill[n][1]; mm = mm + (ch[cn].skill[n][0]?1:0); }
		do_char_log(cn, 6, "You have used %d out of %d greater skill scrolls.\n", m, mm);
		break;
	
	default:
		break;
	}
	
	switch (temp)
	{
		// -------- TAROT CARDS --------
	
	case IT_CH_FOOL: if (!(it[in].flags & IF_SOULSTONE)) do_char_log(cn, 8, DESC_FOOL); break;
	case IT_CH_MAGI: 		do_char_log(cn, 8, DESC_MAGI); 		break;
	case IT_CH_PREIST: 		do_char_log(cn, 8, DESC_PREIST); 	break;
	case IT_CH_EMPRESS: 	do_char_log(cn, 8, DESC_EMPRESS); 	break;
	case IT_CH_EMPEROR: 	do_char_log(cn, 8, DESC_EMPEROR); 	break;
	case IT_CH_HEIROPH: 	do_char_log(cn, 8, DESC_HEIROPH); 	break;
	case IT_CH_LOVERS: 		do_char_log(cn, 8, DESC_LOVERS); 	break;
	case IT_CH_CHARIOT: 	do_char_log(cn, 8, DESC_CHARIOT); 	break;
	case IT_CH_STRENGTH: 	do_char_log(cn, 8, DESC_STRENGTH); 	break;
	case IT_CH_HERMIT: 		do_char_log(cn, 8, DESC_HERMIT); 	break;
	case IT_CH_WHEEL: 		do_char_log(cn, 8, DESC_WHEEL); 	break;
	case IT_CH_JUSTICE: 	do_char_log(cn, 8, DESC_JUSTICE); 	break;
	case IT_CH_HANGED: 		do_char_log(cn, 8, DESC_HANGED); 	break;
	case IT_CH_DEATH: 		do_char_log(cn, 8, DESC_DEATH); 	break;
	case IT_CH_TEMPER: 		do_char_log(cn, 8, DESC_TEMPER); 	break;
	case IT_CH_DEVIL: 		do_char_log(cn, 8, DESC_DEVIL); 	break;
	case IT_CH_TOWER: 		do_char_log(cn, 8, DESC_TOWER); 	break;
	case IT_CH_STAR: 		do_char_log(cn, 8, DESC_STAR); 		break;
	case IT_CH_MOON: 		do_char_log(cn, 8, DESC_MOON); 		break;
	case IT_CH_SUN: 		do_char_log(cn, 8, DESC_SUN); 		break;
	case IT_CH_JUDGE: 		do_char_log(cn, 8, DESC_JUDGE); 	break;
	case IT_CH_WORLD: 		do_char_log(cn, 8, DESC_WORLD); 	break;
	
	case IT_CH_FOOL_R: 		do_char_log(cn, 8, DESC_FOOL_R);	break;
	case IT_CH_MAGI_R: 		do_char_log(cn, 8, DESC_MAGI_R);	break;
	case IT_CH_PREIST_R: 	do_char_log(cn, 8, DESC_PREIST_R);	break;
	case IT_CH_EMPRES_R: 	do_char_log(cn, 8, DESC_EMPRES_R);	break;
	case IT_CH_EMPERO_R: 	do_char_log(cn, 8, DESC_EMPERO_R);	break;
	case IT_CH_HEIROP_R: 	do_char_log(cn, 8, DESC_HEIROP_R);	break;
	case IT_CH_LOVERS_R: 	do_char_log(cn, 8, DESC_LOVERS_R);	break;
	case IT_CH_CHARIO_R: 	do_char_log(cn, 8, DESC_CHARIO_R);	break;
	case IT_CH_STRENG_R: 	do_char_log(cn, 8, DESC_STRENG_R);	break;
	case IT_CH_HERMIT_R: 	do_char_log(cn, 8, DESC_HERMIT_R);	break;
	case IT_CH_WHEEL_R: 	do_char_log(cn, 8, DESC_WHEEL_R);	break;
	case IT_CH_JUSTIC_R: 	do_char_log(cn, 8, DESC_JUSTIC_R);	break;
	case IT_CH_HANGED_R: 	do_char_log(cn, 8, DESC_HANGED_R);	break;
	case IT_CH_DEATH_R: 	do_char_log(cn, 8, DESC_DEATH_R);	break;
	case IT_CH_TEMPER_R: 	do_char_log(cn, 8, DESC_TEMPER_R);	break;
	case IT_CH_DEVIL_R: 	do_char_log(cn, 8, DESC_DEVIL_R);	break;
	case IT_CH_TOWER_R: 	do_char_log(cn, 8, DESC_TOWER_R);	break;
	case IT_CH_STAR_R: 		do_char_log(cn, 8, DESC_STAR_R);	break;
	case IT_CH_MOON_R: 		do_char_log(cn, 8, DESC_MOON_R);	break;
	case IT_CH_SUN_R: 		do_char_log(cn, 8, DESC_SUN_R);		break;
	case IT_CH_JUDGE_R: 	do_char_log(cn, 8, DESC_JUDGE_R);	break;
	case IT_CH_WORLD_R: 	do_char_log(cn, 8, DESC_WORLD_R);	break;
		
	default:
		break;
	}
	
	if ((it[in].flags & IF_ENCHANTED) || (it[in].flags & IF_CORRUPTED))
	{
		switch (it[in].enchantment)
		{
		case  1: do_char_log(cn, 8, "8%% more total Armor Value.\n"); break;
		case  2: do_char_log(cn, 8, "Healing items are 25%% more effective.\n"); break;
		case  3: do_char_log(cn, 8, "Cannot trigger floor traps.\n"); break;
		case  4: do_char_log(cn, 8, "20%% less effect of debuffs on you.\n"); break;
		case  5: do_char_log(cn, 8, "%d%% more total Braveness.\n", 3 * (IS_TWOHAND(in)?2:1)); break;
		case  6: do_char_log(cn, 8, "Stealth is 25%% stronger while moving.\n"); break;
		case  7: do_char_log(cn, 8, "20%% more effect of Weakens you inflict.\n"); break;
		case  8: do_char_log(cn, 8, "Weakens inflicted on you are 80%% weaker.\n"); break;
		case  9: do_char_log(cn, 8, "Reduces Healing Sickness duration on you by 25%%.\n"); break;
		case 10: do_char_log(cn, 8, "Cannot step on death traps.\n"); break;
		case 11: do_char_log(cn, 8, "10%% of total Armor Value is granted as additional Resistance.\n"); break;
		case 12: do_char_log(cn, 8, "%d%% more total Willpower.\n", 3 * (IS_TWOHAND(in)?2:1)); break;
		case 13: do_char_log(cn, 8, "20%% more effect of Slows you inflict.\n"); break;
		case 14: do_char_log(cn, 8, "Slows inflicted on you are 80%% weaker.\n"); break;
		case 15: do_char_log(cn, 8, "Cannot lose focus.\n"); break;
		case 16: do_char_log(cn, 8, "20%% of damage taken is dealt to your Endurance instead.\n"); break;
		case 17: do_char_log(cn, 8, "Grants infravision.\n"); break;
		case 18: do_char_log(cn, 8, "20%% more maximum Endurance.\n"); break;
		case 19: do_char_log(cn, 8, "%d%% more total Intuition.\n", 3 * (IS_TWOHAND(in)?2:1)); break;
		case 20: do_char_log(cn, 8, "20%% more effect of Curses you inflict.\n"); break;
		case 21: do_char_log(cn, 8, "Curses inflicted on you are 80%% weaker.\n"); break;
		case 22: do_char_log(cn, 8, "20%% less cost of skills and spells.\n"); break;
		case 23: do_char_log(cn, 8, "20%% of damage taken is dealt to your Mana instead.\n"); break;
		case 24: do_char_log(cn, 8, "10%% of total Armor Value is granted as additional Immunity.\n"); break;
		case 25: do_char_log(cn, 8, "%d%% more total Agility.\n", 3 * (IS_TWOHAND(in)?2:1)); break;
		case 26: do_char_log(cn, 8, "Movement speed cannot fall below 1.50.\n"); break;
		case 27: do_char_log(cn, 8, "20%% more effect of Poisons you inflict.\n"); break;
		case 28: do_char_log(cn, 8, "Stealth is 25%% stronger while idle.\n"); break;
		case 29: do_char_log(cn, 8, "%d%% more total Strength.\n", 3 * (IS_TWOHAND(in)?2:1)); break;
		case 30: do_char_log(cn, 8, "20%% more effect of Bleeds you inflict.\n"); break;
		case 31: do_char_log(cn, 8, "20%% more effect of Blinds you inflict.\n"); break;
		case 32: do_char_log(cn, 8, "Grants unreduced regeneration rates while walking.\n"); break;
		case 33: do_char_log(cn, 8, "Blinds inflicted on you are 80%% weaker.\n"); break;
		case 34: do_char_log(cn, 8, "Item is indestructible.\n"); break;
		case 35: do_char_log(cn, 8, "8%% chance to half damage taken.\n"); break;
		case 36: do_char_log(cn, 8, "Half of your Meditate score is also applied to your Hitpoints.\n"); break;
		case 37: do_char_log(cn, 8, "5%% additional chance to avoid being hit.\n"); break;
		case 38: do_char_log(cn, 8, "Restore %d Mana upon hitting an enemy.\n", (IS_TWOHAND(in)?2:1)); break;
		case 39: do_char_log(cn, 8, "Restore 2 Mana upon being hit by an enemy.\n"); break;
		case 40: do_char_log(cn, 8, "20%% more total Movement Speed.\n"); break;
		case 41: do_char_log(cn, 8, "%d%% additional chance to hit.\n", 5 * (IS_TWOHAND(in)?2:1)); break;
		case 42: do_char_log(cn, 8, "Restore %d Endurance upon hitting an enemy.\n", (IS_TWOHAND(in)?2:1)); break;
		case 43: do_char_log(cn, 8, "Restore 2 Endurance upon being hit by an enemy.\n"); break;
		case 44: do_char_log(cn, 8, "Movement Speed is doubled while underwater.\n"); break;
		case 45: do_char_log(cn, 8, "Half of your Rest score is also applied to your Mana.\n"); break;
		case 46: do_char_log(cn, 8, "You can always escape from combat.\n"); break;
		case 47: do_char_log(cn, 8, "Reduces extra damage taken from enemy critical hits by 50%%.\n"); break;
		case 48: do_char_log(cn, 8, "30%% more total Thorns score.\n"); break;
		case 49: do_char_log(cn, 8, "Restore %d hitpoints upon hitting an enemy.\n", (IS_TWOHAND(in)?2:1)); break;
		case 50: do_char_log(cn, 8, "2%% more damage dealt.\n"); break;
		case 51: do_char_log(cn, 8, "2%% less damage taken.\n"); break;
		case 52: do_char_log(cn, 8, "Perception is 25%% stronger.\n"); break;
		case 53: do_char_log(cn, 8, "15%% reduced damage taken from damage over time.\n"); break;
		case 54: do_char_log(cn, 8, "%d%% more total attributes.\n", 2 * (IS_TWOHAND(in)?2:1)); break;
		case 55: do_char_log(cn, 8, "Total Glow score is reduced to 0.\n"); break;
		case 56: do_char_log(cn, 8, "20%% more effect of heals and regens you apply.\n"); break;
		//
		case 111: 
		switch (it[in].cost)
		{
			case IT_CH_MAGI: 		do_char_log(cn, 0, DESC_MAGI); 		break;
			case IT_CH_PREIST: 		do_char_log(cn, 0, DESC_PREIST); 	break;
			case IT_CH_EMPRESS: 	do_char_log(cn, 0, DESC_EMPRESS); 	break;
			case IT_CH_EMPEROR: 	do_char_log(cn, 0, DESC_EMPEROR); 	break;
			case IT_CH_HEIROPH: 	do_char_log(cn, 0, DESC_HEIROPH); 	break;
			case IT_CH_LOVERS: 		do_char_log(cn, 0, DESC_LOVERS); 	break;
			case IT_CH_CHARIOT: 	do_char_log(cn, 0, DESC_CHARIOT); 	break;
			case IT_CH_STRENGTH: 	do_char_log(cn, 0, DESC_STRENGTH); 	break;
			case IT_CH_HERMIT: 		do_char_log(cn, 0, DESC_HERMIT); 	break;
			case IT_CH_WHEEL: 		do_char_log(cn, 0, DESC_WHEEL); 	break;
			case IT_CH_JUSTICE: 	do_char_log(cn, 0, DESC_JUSTICE); 	break;
			case IT_CH_HANGED: 		do_char_log(cn, 0, DESC_HANGED); 	break;
			case IT_CH_DEATH: 		do_char_log(cn, 0, DESC_DEATH); 	break;
			case IT_CH_TEMPER: 		do_char_log(cn, 0, DESC_TEMPER); 	break;
			case IT_CH_DEVIL: 		do_char_log(cn, 0, DESC_DEVIL); 	break;
			case IT_CH_TOWER: 		do_char_log(cn, 0, DESC_TOWER); 	break;
			case IT_CH_STAR: 		do_char_log(cn, 0, DESC_STAR); 		break;
			case IT_CH_MOON: 		do_char_log(cn, 0, DESC_MOON); 		break;
			case IT_CH_SUN: 		do_char_log(cn, 0, DESC_SUN); 		break;
			case IT_CH_JUDGE: 		do_char_log(cn, 0, DESC_JUDGE); 	break;
			case IT_CH_WORLD: 		do_char_log(cn, 0, DESC_WORLD); 	break;
			
			case IT_CH_FOOL_R: 		do_char_log(cn, 0, DESC_FOOL_R);	break;
			case IT_CH_MAGI_R: 		do_char_log(cn, 0, DESC_MAGI_R);	break;
			case IT_CH_PREIST_R: 	do_char_log(cn, 0, DESC_PREIST_R);	break;
			case IT_CH_EMPRES_R: 	do_char_log(cn, 0, DESC_EMPRES_R);	break;
			case IT_CH_EMPERO_R: 	do_char_log(cn, 0, DESC_EMPERO_R);	break;
			case IT_CH_HEIROP_R: 	do_char_log(cn, 0, DESC_HEIROP_R);	break;
			case IT_CH_LOVERS_R: 	do_char_log(cn, 0, DESC_LOVERS_R);	break;
			case IT_CH_CHARIO_R: 	do_char_log(cn, 0, DESC_CHARIO_R);	break;
			case IT_CH_STRENG_R: 	do_char_log(cn, 0, DESC_STRENG_R);	break;
			case IT_CH_HERMIT_R: 	do_char_log(cn, 0, DESC_HERMIT_R);	break;
			case IT_CH_WHEEL_R: 	do_char_log(cn, 0, DESC_WHEEL_R);	break;
			case IT_CH_JUSTIC_R: 	do_char_log(cn, 0, DESC_JUSTIC_R);	break;
			case IT_CH_HANGED_R: 	do_char_log(cn, 0, DESC_HANGED_R);	break;
			case IT_CH_DEATH_R: 	do_char_log(cn, 0, DESC_DEATH_R);	break;
			case IT_CH_TEMPER_R: 	do_char_log(cn, 0, DESC_TEMPER_R);	break;
			case IT_CH_DEVIL_R: 	do_char_log(cn, 0, DESC_DEVIL_R);	break;
			case IT_CH_TOWER_R: 	do_char_log(cn, 0, DESC_TOWER_R);	break;
			case IT_CH_STAR_R: 		do_char_log(cn, 0, DESC_STAR_R);	break;
			case IT_CH_MOON_R: 		do_char_log(cn, 0, DESC_MOON_R);	break;
			case IT_CH_SUN_R: 		do_char_log(cn, 0, DESC_SUN_R);		break;
			case IT_CH_JUDGE_R: 	do_char_log(cn, 0, DESC_JUDGE_R);	break;
			case IT_CH_WORLD_R: 	do_char_log(cn, 0, DESC_WORLD_R);	break;
			default:break;
		}
		break;
		default:break;
		}
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
	do_char_log(cn, 1, "%s\n", it[in].description);
	
	if (it[in].data[2])
	{
		do_char_log(cn, 4, "Has been focused.\n");
	}
	
	if (it[in].data[3])
	{
		do_char_log(cn, 6, "Augments use %d of %d rank.\n", it[in].data[3], it[in].data[0]);
	}
	
	look_item_details(cn, in);
}

void look_contract(int cn, int in, int desc)
{
	int rank, tier, mission, fl, n, font;
	int flags[NUM_MAP_POS+NUM_MAP_NEG] = {0};
	
	if (!it[in].data[1]) it[in].data[1] = RANDOM(MSN_COUNT)+1;
	
	mission = it[in].data[1];
	tier    = it[in].data[2];
	
	if ((fl = it[in].data[3]) && fl > 0 && fl < NUM_MAP_POS+1) flags[fl-1] = 1;
	if ((fl = it[in].data[5]) && fl > 0 && fl < NUM_MAP_POS+1) flags[fl-1] = 2;
	if ((fl = it[in].data[7]) && fl > 0 && fl < NUM_MAP_POS+1) flags[fl-1] = 3;
	if ((fl = it[in].data[4]) && fl > 0 && fl < NUM_MAP_NEG+1) flags[fl-1+NUM_MAP_POS] = 1;
	if ((fl = it[in].data[6]) && fl > 0 && fl < NUM_MAP_NEG+1) flags[fl-1+NUM_MAP_POS] = 2;
	if ((fl = it[in].data[8]) && fl > 0 && fl < NUM_MAP_NEG+1) flags[fl-1+NUM_MAP_POS] = 3;
	
	font = get_tier_font(tier);
	
	if (desc)
	{
		do_char_log(cn, 1, "A contract. It reads:\n");
		do_char_log(cn, 1, " \n");
	}
	switch (mission)
	{
		case  1: do_char_log(cn, font, MSN_CN MSN_01 "\n"); break;
		case  2: do_char_log(cn, font, MSN_CN MSN_02 "\n"); break;
		case  3: do_char_log(cn, font, MSN_CN MSN_03 "\n"); break;
		case  4: do_char_log(cn, font, MSN_CN MSN_04 "\n"); break;
		case  5: do_char_log(cn, font, MSN_CN MSN_05 "\n"); break;
		case  6: do_char_log(cn, font, MSN_CN MSN_06 "\n"); break;
		case  7: do_char_log(cn, font, MSN_CN MSN_07 "\n"); break;
		case  8: do_char_log(cn, font, MSN_CN MSN_08 "\n"); break;
		case  9: do_char_log(cn, font, MSN_CN MSN_09 "\n"); break;
		default: do_char_log(cn, font, MSN_CN MSN_00 "\n"); break;
	}
	do_char_log(cn, 1, " \n");
	if (desc || it[in].data[3])	show_map_flags(cn, flags, tier);
	if (desc) do_char_log(cn, font, "%s\n", it[in].description);
	if (ch[cn].flags & CF_GOD)
	{
		look_item_details(cn, in);
	}
}

void look_talisman(int cn, int in)
{
	// No enchantment
	if (!it[in].data[0])
	{
		switch (it[in].data[1])
		{
		case  1: do_char_log(cn, 1, "A talisman socketed with a huge sapphire. A second socket remains vacant.\n"); break;
		case  2: do_char_log(cn, 1, "A talisman socketed with a huge ruby. A second socket remains vacant.\n"); break;
		case  3: do_char_log(cn, 1, "A talisman socketed with a huge amethyst. A second socket remains vacant.\n"); break;
		case  4: do_char_log(cn, 1, "A talisman socketed with a huge topaz. A second socket remains vacant.\n"); break;
		case  5: do_char_log(cn, 1, "A talisman socketed with a huge emerald. A second socket remains vacant.\n"); break;
		case  6: do_char_log(cn, 1, "A talisman socketed with a huge diamond. A second socket remains vacant.\n"); break;
		case  7: do_char_log(cn, 1, "A talisman socketed with a huge spinel. A second socket remains vacant.\n"); break;
		case  8: do_char_log(cn, 1, "A talisman socketed with a huge citrine. A second socket remains vacant.\n"); break;
		case  9: do_char_log(cn, 1, "A talisman socketed with a huge opal. A second socket remains vacant.\n"); break;
		case 10: do_char_log(cn, 1, "A talisman socketed with a huge aquamarine. A second socket remains vacant.\n"); break;
		case 11: do_char_log(cn, 1, "A talisman socketed with a huge beryl. A second socket remains vacant.\n"); break;
		case 12: do_char_log(cn, 1, "A talisman socketed with a huge zircon. A second socket remains vacant.\n"); break;
		default: do_char_log(cn, 1, "%s\n", it[in].description); break;
		}
	}
	else
	{
		do_char_log(cn, 1, "A Talisman socketed with two gemstones. It can be used on a piece of gear to grant the following effect:\n");
		switch (it[in].data[0])
		{
		case  1: do_char_log(cn, 1, "%-12.12s  %+4d\n", "Braveness", 4); break;
		case  2: do_char_log(cn, 8, "[Chest Only] 8%% more total Armor Value.\n"); break; //  1
		case  3: do_char_log(cn, 8, "[Helmet Only] Healing items are 50%% more effective.\n"); break; //  2
		case  4: do_char_log(cn, 8, "[Boots Only] Cannot trigger floor traps.\n"); break; //  3
		case  5: do_char_log(cn, 8, "[Cloak Only] 20%% less effect of debuffs on you.\n"); break; //  4
		case  6: do_char_log(cn, 8, "3%% more total Braveness.\n"); break; //  5
		case  7: do_char_log(cn, 8, "[Armor Pieces Only] %-12.12s  %+4d\n", "AoE Bonus", 1); break;
		case  8: do_char_log(cn, 8, "[Gloves Only] 20%% more effect of Weakens you inflict.\n"); break; //  7
		case  9: do_char_log(cn, 1, "%-12.12s  %+4d\n", "Move Speed", 6); break;
		case 10: do_char_log(cn, 8, "[Cloak Only] Weakens inflicted on you are 80%% weaker.\n"); break; //  8
		case 11: do_char_log(cn, 1, "%-12.12s  %+4d\n", "HP & Mana", 35); break;
		case 12: do_char_log(cn, 1, "%-12.12s  %+4d\n", "WV & AV", 2); break;
		case 13: do_char_log(cn, 1, "%-12.12s  %+4d\n", "Willpower", 4); break;
		case 14: do_char_log(cn, 8, "[Helmet Only] Reduces Healing Sickness duration on you by 33%%.\n"); break; //  9
		case 15: do_char_log(cn, 8, "[Boots Only] Cannot step on death traps.\n"); break; // 10
		case 16: do_char_log(cn, 8, "[Chest Only] 10%% of total Armor Value is granted as additional Resistance.\n"); break; // 11
		case 17: do_char_log(cn, 8, "3%% more total Willpower.\n"); break; // 12
		case 18: do_char_log(cn, 1, "%-12.12s  %+4d\n", "Mana", 50); break;
		case 19: do_char_log(cn, 8, "[Gloves Only] 20%% more effect of Slows you inflict.\n"); break; // 13
		case 20: do_char_log(cn, 1, "%-12.12s  %+4d\n", "Cast Speed", 3); break;
		case 21: do_char_log(cn, 8, "[Cloak Only] Slows inflicted on you are 80%% weaker.\n"); break; // 14
		case 22: do_char_log(cn, 8, "[Jewellery Only] Cannot lose focus.\n"); break; // 15
		case 23: do_char_log(cn, 8, "[Chest Only] 20%% of damage taken is dealt to your Endurance instead.\n"); break; // 16
		case 24: do_char_log(cn, 1, "%-12.12s  %+4d\n", "Intuition", 4); break;
		case 25: do_char_log(cn, 8, "[Helmet Only] Grants infravision.\n"); break; // 17
		case 26: do_char_log(cn, 8, "[Armor Pieces Only] 20%% more maximum Endurance.\n"); break; // 18
		case 27: do_char_log(cn, 8, "3%% more total Intuition.\n"); break; // 19
		case 28: do_char_log(cn, 1, "%-12.12s  %+4d\n", "Spell Mod", 1); break;
		case 29: do_char_log(cn, 8, "[Gloves Only] 20%% more effect of Curses you inflict.\n"); break; // 20
		case 30: do_char_log(cn, 8, "[Boots Only] Stealth is 25%% stronger while moving.\n"); break; //  6
		case 31: do_char_log(cn, 8, "[Cloak Only] Curses inflicted on you are 80%% weaker.\n"); break; // 21
		case 32: do_char_log(cn, 8, "[Helmet Only] 20%% less cost of skills and spells.\n"); break; // 22
		case 33: do_char_log(cn, 8, "[Chest Only] 20%% of damage taken is dealt to your Mana instead.\n"); break; // 23
		case 34: do_char_log(cn, 1, "%-12.12s  %+4d\n", "Agility", 4); break;
		case 35: do_char_log(cn, 8, "[Chest Only] 10%% of total Armor Value is granted as additional Immunity.\n"); break; // 24
		case 36: do_char_log(cn, 8, "3%% more total Agility.\n"); break; // 25
		case 37: do_char_log(cn, 8, "[Boots Only] Movement speed cannot fall below 1.50.\n"); break; // 26
		case 38: do_char_log(cn, 8, "[Gloves Only] 20%% more effect of Poisons you inflict.\n"); break; // 27
		case 39: do_char_log(cn, 1, "%-12.12s  %+4d\n", "Atk Speed", 3); break;
		case 40: do_char_log(cn, 1, "%-12.12s  %+4d\n", "Weapon Value", 3); break;
		case 41: do_char_log(cn, 8, "[Cloak Only] Stealth is 25%% stronger while idle.\n"); break; // 28
		case 42: do_char_log(cn, 1, "%-12.12s  %+4d\n", "Parry Bonus", 3); break;
		case 43: do_char_log(cn, 1, "%-12.12s  %+4d\n", "Strength", 4); break;
		case 44: do_char_log(cn, 8, "3%% more total Strength.\n"); break; // 29
		case 45: do_char_log(cn, 1, "%-12.12s  %+4d\n", "Hitpoints", 50); break;
		case 46: do_char_log(cn, 8, "[Gloves Only] 20%% more effect of Bleeds you inflict.\n"); break; // 30
		case 47: do_char_log(cn, 1, "%-12.12s  %+4d\n", "Endurance", 25); break;
		case 48: do_char_log(cn, 1, "%-12.12s  %+4d\n", "Hit Bonus", 3); break;
		case 49: do_char_log(cn, 8, "[Weapons Only] %-12.12s  %+4d\n", "Base Crit", 1); break;
		case 50: do_char_log(cn, 1, "%-12.12s  %+4d\n", "Armor Value", 3); break;
		case 51: do_char_log(cn, 1, "%-12.12s  %+4d\n", "All Attribs", 2); break;
		case 52: do_char_log(cn, 8, "[Armor Pieces Only] %-12.12s  %+4d\n", "Glow", 20); break;
		case 53: do_char_log(cn, 8, "[Gloves Only] 20%% more effect of Blinds you inflict.\n"); break; // 31
		case 54: do_char_log(cn, 8, "[Jewellery Only] Grants unreduced regeneration rates while walking.\n"); break; // 32
		case 55: do_char_log(cn, 1, "%-12.12s  %+4d\n", "Hit & Parry", 2); break;
		case 56: do_char_log(cn, 8, "[Cloak Only] Blinds inflicted on you are 80%% weaker.\n"); break; // 33
		case 57: do_char_log(cn, 8, "Item is indestructible.\n"); break; // 34
		case 58: do_char_log(cn, 1, "%-12.12s  %+4d\n", "Spell Apt", 6); break;
		case 59: do_char_log(cn, 8, "8%% chance to half damage taken.\n"); break; // 35
		case 60: do_char_log(cn, 8, "[Helmet Only] Half of your Meditate score is also applied to your Hitpoints.\n"); break; // 36
		case 61: do_char_log(cn, 8, "[Cloak Only] 5%% additional chance to avoid being hit.\n"); break; // 37
		case 62: do_char_log(cn, 8, "[Weapons Only] Restore 1 Mana upon hitting an enemy.\n"); break; // 38
		case 63: do_char_log(cn, 8, "[Armor Pieces Only] Restore 2 Mana upon being hit by an enemy.\n"); break; // 39
		case 64: do_char_log(cn, 1, "%-12.12s  %+4d\n", "Crit Multi", 10); break;
		case 65: do_char_log(cn, 8, "[Boots Only] 20%% more total Movement Speed.\n"); break; // 40
		case 66: do_char_log(cn, 8, "[Weapons Only] 5%% additional chance to hit.\n"); break; // 41
		case 67: do_char_log(cn, 8, "[Weapons Only] Restore 1 Endurance upon hitting an enemy.\n"); break; // 42
		case 68: do_char_log(cn, 8, "[Armor Pieces Only] Restore 2 Endurance upon being hit by an enemy.\n"); break; // 43
		case 69: do_char_log(cn, 1, "%-12.12s  %+4d\n", "All Speed", 2); break;
		case 70: do_char_log(cn, 8, "[Boots Only] Movement Speed is doubled while underwater.\n"); break; // 44
		case 71: do_char_log(cn, 8, "[Helmet Only] Half of your Rest score is also applied to your Mana.\n"); break; // 45
		case 72: do_char_log(cn, 8, "[Boots Only] You can always escape from combat.\n"); break; // 46
		case 73: do_char_log(cn, 1, "%-12.12s  %+4d\n", "Top Damage", 12); break;
		case 74: do_char_log(cn, 8, "[Chest Only] Reduces extra damage taken from enemy critical hits by 50%%.\n"); break; // 47
		case 75: do_char_log(cn, 8, "[Chest Only] 30%% more total Thorns score.\n"); break; // 48
		case 76: do_char_log(cn, 1, "%-12.12s  %+4d\n", "Crit Bonus", 16); break;
		case 77: do_char_log(cn, 8, "[Weapons Only] Restore 1 hitpoints upon hitting an enemy.\n"); break; // 49
		case 78: do_char_log(cn, 1, "%-12.12s  %+4d\n", "Thorns", 2); break;
		case 79: do_char_log(cn, 8, "[Jewellery Only] 2%% more damage dealt.\n"); break; // 50
		case 80: do_char_log(cn, 8, "[Jewellery Only] 2%% less damage taken.\n"); break; // 51
		case 81: do_char_log(cn, 1, "%-12.12s  %+4d\n", "Parry", 4);
				 do_char_log(cn, 1, "%-12.12s  %+4d\n", "Hit", -2); break;
		case 82: do_char_log(cn, 8, "[Helmet Only] Perception is 33%% stronger.\n"); break; // 52
		case 83: do_char_log(cn, 8, "[Jewellery Only] 15%% reduced damage taken from damage over time.\n"); break; // 53
		case 84: do_char_log(cn, 8, "2%% more total attributes.\n"); break; // 54
		case 85: do_char_log(cn, 8, "[Jewellery Only] Total Glow score is reduced to 0.\n"); break; // 55
		case 86: do_char_log(cn, 8, "[Gloves Only] 20%% more effect of heals and regens you apply.\n"); break; // 56
		case 87: do_char_log(cn, 1, "%-12.12s  %+4d\n", "Hit", 4);
				 do_char_log(cn, 1, "%-12.12s  %+4d\n", "Parry", -2); break;
		case 88: do_char_log(cn, 1, "%-12.12s  %+4d\n", "Weapon Value", 2);
				 do_char_log(cn, 1, "%-12.12s  %+4d\n", "Top Damage", 8); break;
		case 89: do_char_log(cn, 1, "%-12.12s  %+4d\n", "Crit Bonus", 8);
				 do_char_log(cn, 1, "%-12.12s  %+4d\n", "Crit Multi", 5); break;
		case 90: do_char_log(cn, 1, "%-12.12s  %+4d\n", "Armor Value", 2);
				 do_char_log(cn, 1, "%-12.12s  %+4d\n", "Thorns", 1); break;
		case 91: do_char_log(cn, 1, "%-12.12s  %+4d\n", "Cooldown", 2); break;
		default: break;
		}
	}
	
	if (ch[cn].flags & CF_GOD)
	{
		do_char_log(cn, 2, "ID=%d, Temp=%d, Value: %dG %dS.\n", in, it[in].temp, it[in].value / 100, it[in].value % 100);
		do_char_log(cn, 2, "driver=%d, active=%d, sprite=%d/%d\n", it[in].driver, it[in].active, it[in].sprite[0], it[in].sprite[1]);
		do_char_log(cn, 2, "max_age=%d/%d, current_age=%d/%d\n", it[in].max_age[0], it[in].max_age[1], it[in].current_age[0], it[in].current_age[1]);
		do_char_log(cn, 2, "max_damage=%d, current_damage=%d\n", it[in].max_damage, it[in].current_damage);
	}
	
	do_appraisal(cn, in);
}

void look_driver(int cn, int in)
{
	switch(it[in].driver)
	{
	case     0:
	case    32:
	case	40:
	case    52:
	case	60:
	case    92:
	case    93:
	case   110:
	case   114:
	case   115:
	case   116:
	case   133:
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
	case   113:
		look_contract(cn, in, 1);
		break;
	case   119:
		look_talisman(cn, in);
		break;
	default:
		xlog("Unknown look_driver %d", it[in].driver);
		break;
	}
}

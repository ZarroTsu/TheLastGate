/*************************************************************************

   This file is part of 'Mercenaries of Astonia v2'
   Copyright (c) 1997-2001 Daniel Brockhaus (joker@astonia.com)
   All rights reserved.

 **************************************************************************/

#include "server.h"

//		AT_BRV	AT_WIL	AT_INT	AT_AGL	AT_STR

struct s_skilltab skilltab[MAXSKILL] = {
//	{ //, '/', 	"////////////////",		"////////////////////////////////////////////////////////////////////////////////",
	{  0, 'C', 	"Hand to Hand", 		"Passive ability to hit and parry while unarmed.", 
				"", "", "", 
				{ AT_AGL, AT_AGL, AT_STR }},
				
	{  1, 'F', 	"Precision", 			"Passively improves your ability to inflict critical hits.", 
				"", "", "", 
				{ AT_BRV, AT_AGL, AT_AGL }},
				
	{  2, 'D', 	"Dagger", 				"Passive ability to hit and parry with a dagger in your main hand.", 
				"", "", "", 
				{ AT_BRV, AT_WIL, AT_AGL }},
				
	{  3, 'D', 	"Sword", 				"Passive ability to hit and parry with a one-handed sword in your main hand.", 
				"", "", "", 
				{ AT_BRV, AT_AGL, AT_STR }},
				
	{  4, 'D', 	"Axe", 					"Passive ability to hit and parry with an axe in your main hand.", 
				"", "", "", 
				{ AT_AGL, AT_STR, AT_STR }},
				
	{  5, 'D', 	"Staff", 				"Passive ability to hit and parry with a staff in your main hand.", 
				"", "", "", 
				{ AT_BRV, AT_INT, AT_STR }},
				
	{  6, 'D', 	"Two-Handed", 			"Passive ability to hit and parry with a two-handed weapon in your main hand.", 
				"", "", "", 
				{ AT_AGL, AT_AGL, AT_STR }},
				
	{  7, 'H', 	"Razor", 				"Use: Applies a self-buff, granting extra melee hits after a 1 second delay.", 
				"", "", "", 
				{ AT_BRV, AT_BRV, AT_BRV }},
				
	{  8, 'R', 	"Stealth", 				"Passive ability to stay hidden from others' sight.", 
				"", "", "", 
				{ AT_INT, AT_AGL, AT_AGL }},
				
	{  9, 'R', 	"Perception", 			"Passive ability to see and hear your surroundings.", 
				"", "", "", 
				{ AT_WIL, AT_INT, AT_AGL }},
//	{ //, '/', 	"////////////////",		"////////////////////////////////////////////////////////////////////////////////",
	{ 10, 'U', 	"Swimming", 			"Passive ability to prevent the loss of hitpoints while moving underwater.", 
				"", "", "", 
				{ AT_WIL, AT_AGL, AT_STR }},
				
	{ 11, 'O', 	"Magic Shield", 		"Use: Applies a self-buff, granting temporary armor.", 
				"Magic Shell", 			"Use: Applies a self-buff, granting temporary resistance and immunity.", "", 
				{ AT_BRV, AT_WIL, AT_WIL }},
				
	{ 12, 'U', 	"Bartering", 			"Passive ability to get good prices while buying or selling.", 
				"", "", "", 
				{ AT_BRV, AT_INT, AT_INT }},
				
	{ 13, 'P', 	"Repair", 				"Use: You will try to repair the item under your cursor.", 
				"", "", "", 
				{ AT_INT, AT_AGL, AT_STR }},
				
	{ 14, 'O', 	"Light", 				"Use: Make yourself or your target glow in the dark.", 
				"", "", "", 
				{ AT_BRV, AT_WIL, AT_INT }},
				
	{ 15, 'O', 	"Recall", 				"Use: Warp yourself to a safe place.", 
				"", "", "", 
				{ AT_BRV, AT_WIL, AT_INT }},
				
	{ 16, 'D', 	"Shield", 				"Passive ability to parry while using a shield.", 
				"", "", "", 
				{ AT_BRV, AT_WIL, AT_STR }},
				
	{ 17, 'O', 	"Protect", 				"Use: Applies a buff, raising your target's armor value.", 
				"", "", "", 
				{ AT_BRV, AT_WIL, AT_WIL }},
				
	{ 18, 'O', 	"Enhance", 				"Use: Applies a buff, raising your target's weapon value.", 
				"", "", "", 
				{ AT_BRV, AT_WIL, AT_WIL }},
				
	{ 19, 'K', 	"Slow", 				"Use: Applies a decaying debuff, greatly reducing your target's action speed.", 
				"Greater Slow", 		"Use: Applies a debuff, reducing your target's action speed.", "", 
				{ AT_BRV, AT_INT, AT_INT }},
//	{ //, '/', 	"////////////////",		"////////////////////////////////////////////////////////////////////////////////",
	{ 20, 'K', 	"Curse", 				"Use: Applies a debuff, reducing your target's attributes.", 
				"Greater Curse", 		"Use: Applies a decaying debuff, greatly reducing your target's attributes.", "", 
				{ AT_BRV, AT_INT, AT_INT }},
				
	{ 21, 'O', 	"Bless", 				"Use: Applies a buff, raising your target's attributes.", 
				"", "", "", 
				{ AT_BRV, AT_WIL, AT_WIL }},
				
	{ 22, 'O', 	"Identify", 			"Use: Identify the properties of a target or an item.", 
				"", "", "", 
				{ AT_BRV, AT_WIL, AT_INT }},
				
	{ 23, 'Q', 	"Resistance", 			"Passive ability to avoid enemy negative spells.", 
				"", "", "", 
				{ AT_BRV, AT_WIL, AT_STR }},
				
	{ 24, 'I', 	"Blast", 				"Use: Damages the selected target and any around you. This also applies a debuff, increasing the damage dealt to the target.", 
				"Blast", 				"Use: Damages the selected target and any around you. This also applies a debuff, reducing your target's spell modifier.", "", 
				{ AT_BRV, AT_INT, AT_INT }},
				
	{ 25, 'O', 	"Dispel", 				"Use: Removes debuffs from your target.", 
				"Dispel", 				"Use: Removes buffs from your target.", "", 
				{ AT_BRV, AT_WIL, AT_INT }},
				
	{ 26, 'O', 	"Heal", 				"Use: Heals your target. Repeat uses lose their full effect.", 
				"Regen", 				"Use: Applies a buff, granting your target health regeneration.", 
										"Use: Consumes a portion of your hitpoints to restore your target's mana.", 
				{ AT_BRV, AT_INT, AT_STR }},
				
	{ 27, 'M', 	"Ghost Companion", 		"Use: Summons a companion to follow you and your commands.", 
				"", "", "", 
				{ AT_BRV, AT_WIL, AT_INT }},
				
	{ 28, 'A', 	"Regenerate", 			"Passive ability to regenerate hitpoints over time.", 
				"", "", "", 
				{ AT_STR, AT_STR, AT_STR }},
				
	{ 29, 'A', 	"Rest", 				"Passive ability to regenerate endurance over time.", 
				"", "", "", 
				{ AT_AGL, AT_AGL, AT_AGL }},
//	{ //, '/', 	"////////////////",		"////////////////////////////////////////////////////////////////////////////////",
	{ 30, 'B', 	"Meditate", 			"Passive ability to regenerate mana over time.", 
				"", "", "", 
				{ AT_INT, AT_INT, AT_INT }},
				
	{ 31, 'U', 	"Sense Magic", 			"Passive ability to sense who or what did something to you, and sense magical items.", 
				"", "", "", 
				{ AT_BRV, AT_WIL, AT_STR }},
				
	{ 32, 'Q', 	"Immunity", 			"Passive ability to reduce the strength of enemy negative spells.", 
				"", "", "", 
				{ AT_BRV, AT_AGL, AT_STR }},
				
	{ 33, 'S', 	"Surround Hit", 		"Passive ability to hit surrounding enemies. Grants additional damage in front of you.", 
				"", "", "", 
				{ AT_BRV, AT_AGL, AT_STR }},
				
	{ 34, 'H', 	"Concentrate", 			"Passive ability to reduce the mana cost of spells and abilities.", 
				"", "", "", 
				{ AT_WIL, AT_WIL, AT_WIL }},
				
	{ 35, 'G', 	"Warcry", 				"Use: Shout to stun and strike fear into all nearby enemies.", 
				"", "", "", 
				{ AT_BRV, AT_STR, AT_STR }},
				
	{ 36, 'D', 	"Dual Wield", 			"Passive ability to hit while using a dual-sword.", 
				"", "", "", 
				{ AT_BRV, AT_AGL, AT_STR }},
				
	{ 37, 'F', 	"Blind", 				"Use: Applies a debuff to nearby foes, reducing their hit and parry rates.", 
				"", "", "", 
				{ AT_BRV, AT_AGL, AT_STR }},
				
	{ 38, 'D', 	"Weapon Mastery", 		"Passive ability to improve weapon value granted by your equipped weapon.", 
				"", "", "", 
				{ AT_BRV, AT_AGL, AT_STR }},
				
	{ 39, 'E', 	"Armor Mastery", 		"Passive ability to improve armor value granted by your equipped armor.", 
				"", "", "", 
				{ AT_BRV, AT_AGL, AT_STR }},
//	{ //, '/', 	"////////////////",		"////////////////////////////////////////////////////////////////////////////////",
	{ 40, 'G', 	"Cleave", 				"Use: Strike your foe and deal damage to surrounding enemies.", 
				"", "", "", 
				{ AT_BRV, AT_STR, AT_STR }},
				
	{ 41, 'G', 	"Weaken", 				"Use: Reduce your foe's weapon and armor values.", 
				"Greater Weaken", 		"Use: Greatly reduce your foe's armor value.", "", 
				{ AT_BRV, AT_AGL, AT_AGL }},
				
	{ 42, 'I', 	"Poison", 				"Use: Applies a stacking debuff, causing your target to take damage over time.", 
				"", "", "", 
				{ AT_BRV, AT_INT, AT_INT }},
				
	{ 43, 'J', 	"Pulse", 				"Use: Applies a self-buff, causing a repeating burst of energy to damage nearby foes.", 
				"", "", "", 
				{ AT_WIL, AT_INT, AT_INT }},
				
	{ 44, 'L', 	"Proximity", 			"Passively improves the area-of-effect of your Warcry, Taunt, and Surround Hit skills.", // Arch-Templar
										"Passively improves your Surround Hit skill, and improves the area-of-effect to your Blind skill.", // Warrior
										"Passively improves the area-of-effect of your Poison, Curse, and Slow spells.", // Sorcerer
										"Passively improves the area-of-effect of your Blast and Pulse spells.", // Arch-Harakim
				{ AT_WIL, AT_INT, AT_INT }},
				
	{ 45, 'N', 	"Companion Mastery", 	"Improves the power and abilities of your ghost companion.", 
				"", "", "", 
				{ AT_BRV, AT_WIL, AT_INT }},
				
	{ 46, 'N', 	"Shadow Copy", 			"Use: Summons a temporary doppelganger to attack your enemies.", 
				"", "", "", 
				{ AT_WIL, AT_AGL, AT_AGL }},
				
	{ 47, 'O', 	"Haste", 				"Use: Applies a self-buff, increasing your action speed.", 
				"", "", "", 
				{ AT_BRV, AT_WIL, AT_WIL }},
				
	{ 48, 'T', 	"Taunt",				"Use: Applies a debuff, forcing your target to attack you. This also applies a self-buff, granting damage resistance.", 
				"", "", "", 
				{ AT_BRV, AT_STR, AT_STR }},
				
	{ 49, 'T', 	"Trice", 				"Use: Strike your foe and leap behind them, striking up to three consecutive foes.", 
				"", "", "", 
				{ AT_BRV, AT_AGL, AT_STR }}
//	{ //, '/', 	"////////////////",		"////////////////////////////////////////////////////////////////////////////////",
};

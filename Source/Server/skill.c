/*************************************************************************

   This file is part of 'Mercenaries of Astonia v2'
   Copyright (c) 1997-2001 Daniel Brockhaus (joker@astonia.com)
   All rights reserved.

 **************************************************************************/

#include "server.h"

//		AT_BRV	AT_WIL	AT_INT	AT_AGL	AT_STR

struct s_skilltab skilltab[MAXSKILL] = {
//	{ //, '/', "////////////////",	"//////////////////////////////////////////////////",			{ //////, //////, ////// }}, //
	{  0, 'C', "Hand to Hand", 		"Your ability to hit and parry while unarmed.", 				{ AT_AGL, AT_AGL, AT_STR }},
	{  1, 'F', "Precision", 		"Improves your ability to inflict critical hits.", 				{ AT_BRV, AT_AGL, AT_AGL }},
	{  2, 'D', "Dagger", 			"Your ability to hit and parry with daggers.", 					{ AT_BRV, AT_WIL, AT_AGL }},
	{  3, 'D', "Sword", 			"Your ability to hit and parry with one-handed swords.",		{ AT_BRV, AT_AGL, AT_STR }},
	{  4, 'D', "Axe", 				"Your ability to hit and parry with axes.", 					{ AT_AGL, AT_STR, AT_STR }},
	{  5, 'D', "Staff", 			"Your ability to hit and parry with staffs.", 					{ AT_BRV, AT_INT, AT_STR }},
	{  6, 'D', "Two-Handed", 		"Your ability to hit and parry with two-handed weapons.", 		{ AT_AGL, AT_AGL, AT_STR }},
	{  7, 'H', "Focus", 			"You cannot lose focus. Grants a bonus to total spellpower.",	{ AT_BRV, AT_BRV, AT_BRV }},
	{  8, 'R', "Stealth", 			"Your ability to stay hidden from others' sight.", 				{ AT_INT, AT_AGL, AT_AGL }},
	{  9, 'R', "Perception", 		"Your ability to see and hear your surroundings.", 				{ AT_WIL, AT_INT, AT_AGL }},
//	{ //, '/', "////////////////",	"//////////////////////////////////////////////////",			{ //////, //////, ////// }}, //
	{ 10, 'U', "Swimming", 			"Prevents the loss of hitpoints while moving underwater.", 		{ AT_WIL, AT_AGL, AT_STR }},
	{ 11, 'O', "Magic Shield", 		"Use: Applies a buff to yourself, granting temporary armor.", 	{ AT_BRV, AT_WIL, AT_WIL }},
	{ 12, 'U', "Bartering", 		"Getting good prices while buying or selling.", 				{ AT_BRV, AT_INT, AT_INT }},
	{ 13, 'P', "Repair", 			"Use: You will try to repair the item under your cursor.", 		{ AT_INT, AT_AGL, AT_STR }},
	{ 14, 'O', "Light", 			"Use: Make yourself or your target glow in the dark.", 			{ AT_BRV, AT_WIL, AT_INT }},
	{ 15, 'O', "Recall", 			"Use: Warp yourself to a safe place.", 							{ AT_BRV, AT_WIL, AT_INT }},
	{ 16, 'D', "Shield", 			"Your ability to score bonus parries while using a shield.", 	{ AT_BRV, AT_WIL, AT_STR }},
	{ 17, 'O', "Protect", 			"Use: Applies a buff, raising your target's armor value.", 		{ AT_BRV, AT_WIL, AT_WIL }},
	{ 18, 'O', "Enhance", 			"Use: Applies a buff, raising your target's weapon value.", 	{ AT_BRV, AT_WIL, AT_WIL }},
	{ 19, 'K', "Slow", 				"Use: Applies a debuff, reducing your target's action speed.", 	{ AT_BRV, AT_INT, AT_INT }},
//	{ //, '/', "////////////////",	"//////////////////////////////////////////////////",			{ //////, //////, ////// }}, //
	{ 20, 'K', "Curse", 			"Use: Applies a debuff, reducing your target's attributes.", 	{ AT_BRV, AT_INT, AT_INT }},
	{ 21, 'O', "Bless", 			"Use: Applies a buff, raising your target's attributes.", 		{ AT_BRV, AT_WIL, AT_WIL }},
	{ 22, 'O', "Identify", 			"Use: Identify the properties of a target or an item.", 		{ AT_BRV, AT_WIL, AT_INT }},
	{ 23, 'Q', "Resistance", 		"Resists against enemy negative spells.", 						{ AT_BRV, AT_WIL, AT_STR }},
	{ 24, 'I', "Blast", 			"Use: Damages the selected target and any around you.", 		{ AT_BRV, AT_INT, AT_INT }},
	{ 25, 'O', "Dispel", 			"Use: Removes debuffs from your target.", 						{ AT_BRV, AT_WIL, AT_INT }},
	{ 26, 'O', "Heal", 				"Use: Heals your target. Repeat uses lose their full effect.", 	{ AT_BRV, AT_INT, AT_STR }},
	{ 27, 'M', "Ghost Companion", 	"Use: Summons a companion to follow you and your commands.", 	{ AT_BRV, AT_WIL, AT_INT }},
	{ 28, 'A', "Regenerate", 		"Regenerating hitpoints, usually while out-of-combat.", 		{ AT_STR, AT_STR, AT_STR }},
	{ 29, 'A', "Rest", 				"Regenerating endurance, usually while out-of-combat.", 		{ AT_AGL, AT_AGL, AT_AGL }},
//	{ //, '/', "////////////////",	"//////////////////////////////////////////////////",			{ //////, //////, ////// }}, //
	{ 30, 'B', "Meditate", 			"Regenerating mana, usually while out-of-combat.", 				{ AT_INT, AT_INT, AT_INT }},
	{ 31, 'U', "Sense Magic", 		"Sensing who or what did something to you.", 					{ AT_BRV, AT_WIL, AT_STR }},
	{ 32, 'Q', "Immunity", 			"Reduces the strength of enemy negative spells.", 				{ AT_BRV, AT_AGL, AT_STR }},
	{ 33, 'S', "Surround Hit", 		"Hitting all surrounding enemies, including your front.", 		{ AT_BRV, AT_AGL, AT_STR }},
	{ 34, 'H', "Concentrate", 		"Reduces the mana cost of spells and abilities.", 				{ AT_WIL, AT_WIL, AT_WIL }},
	{ 35, 'G', "Warcry", 			"Use: Shout to stun and strike fear into all nearby enemies.", 	{ AT_BRV, AT_STR, AT_STR }},
	{ 36, 'D', "Dual Wield", 		"Your ability to score bonus hits while dual-wielding.", 		{ AT_BRV, AT_AGL, AT_STR }},
	{ 37, 'F', "Combat Mastery", 	"Improves your ability to hit and parry with any weapon.", 		{ AT_BRV, AT_AGL, AT_STR }},
	{ 38, 'D', "Weapon Mastery", 	"Improves the weapon value for your equipped weapon.", 			{ AT_BRV, AT_AGL, AT_STR }},
	{ 39, 'E', "Armor Mastery", 	"Improves the armor value for your equipped armor.", 			{ AT_BRV, AT_AGL, AT_STR }},
//	{ //, '/', "////////////////",	"//////////////////////////////////////////////////",			{ //////, //////, ////// }}, //
	{ 40, 'G', "Cleave", 			"Use: Strike your foe and deal damage to surrounding enemies.", { AT_BRV, AT_STR, AT_STR }},
	{ 41, 'G', "Weaken", 			"Use: Reduce your foe's weapon and armor values.", 				{ AT_BRV, AT_AGL, AT_AGL }},
	{ 42, 'I', "Poison", 			"Use: Poison the selected target and any around you.", 			{ AT_BRV, AT_INT, AT_INT }},
	{ 43, 'J', "Damage Proximity", 	"Increases the area-of-effect of both Blast and Poison.", 		{ AT_WIL, AT_INT, AT_INT }},
	{ 44, 'L', "Hex Proximity", 	"Increases the area-of-effect of Poison, Curse and Slow.", 		{ AT_WIL, AT_INT, AT_INT }},
	{ 45, 'N', "Companion Mastery", "Improves the power and abilities of your ghost companion.", 	{ AT_BRV, AT_WIL, AT_INT }},
	{ 46, 'N', "Shadow Copy", 		"Use: Summon a doppelganger to attack your enemies with you.", 	{ AT_WIL, AT_AGL, AT_AGL }},
	{ 47, 'O', "Haste", 			"Use: Applies a buff to yourself, increasing action speed.",	{ AT_BRV, AT_WIL, AT_WIL }},
	{ 48, 'T', "Surround Area",		"Improves the area-of-effect for Surround Hit.", 				{ AT_BRV, AT_STR, AT_STR }},
	{ 49, 'T', "Surround Rate", 	"Improves how often Surround Hit is triggered.", 				{ AT_BRV, AT_AGL, AT_STR }}
//	{ //, '/', "////////////////",	"//////////////////////////////////////////////////",			{ //////, //////, ////// }}, //
};

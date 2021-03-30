/*************************************************************************

   This file is part of 'Mercenaries of Astonia v2'
   Copyright (c) 1997-2001 Daniel Brockhaus (joker@astonia.com)
   All rights reserved.

 **************************************************************************/

#include "server.h"

/*
   Definition for s_skilltab
   struct s_skilltab
   {
        int nr;
        char sortkey;
        char name[40];
        char desc[200];

        int attrib[3];
   };
 */

/*
	AT_BRV	AT_WIL	AT_INT	AT_AGL	AT_STR
*/

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


/*
	OLD skill list just in case...
//
struct s_skilltab skilltab[MAXSKILL] = {
//	{ //, '/', "////////////////",	"//////////////////////////////////////////////////",	{ //////, //////, ////// }}, //
	{  0, 'C', "Hand to Hand", 		"Fighting without weapons.", 							{ AT_BRV, AT_AGL, AT_STR }},
	{  1, 'C', "Karate", 			"Fighting without weapons and doing damage.", 			{ AT_BRV, AT_AGL, AT_STR }},
	{  2, 'C', "Dagger", 			"Fighting with daggers or similar weapons.", 			{ AT_BRV, AT_AGL, AT_INT }},
	{  3, 'C', "Sword", 			"Fighting with swords or similar weapons.", 			{ AT_BRV, AT_AGL, AT_STR }},
	{  4, 'C', "Axe", 				"Fighting with axes or similar weapons.",				{ AT_BRV, AT_STR, AT_STR }},
	{  5, 'C', "Staff", 			"Fighting with staffs or similar weapons.",				{ AT_AGL, AT_STR, AT_STR }},
	{  6, 'C', "Two-Handed", 		"Fighting with two-handed weapons.",					{ AT_AGL, AT_STR, AT_STR }},
//	{ //, '/', "////////////////",	"//////////////////////////////////////////////////",	{ //////, //////, ////// }}, //
	{  7, 'G', "Lock-Picking", 		"Opening doors without keys.",							{ AT_INT, AT_WIL, AT_AGL }},
	{  8, 'G', "Stealth", 			"Moving without being seen or heard.",					{ AT_INT, AT_WIL, AT_AGL }},
	{  9, 'G', "Perception", 		"Seeing and hearing.",									{ AT_INT, AT_WIL, AT_AGL }},
//	{ //, '/', "////////////////",	"//////////////////////////////////////////////////",	{ //////, //////, ////// }}, //
	{ 10, 'M', "Swimming", 			"Moving through water without drowning.",				{ AT_INT, AT_WIL, AT_AGL }},
	{ 11, 'R', "Magic Shield", 		"Spell: Create a magic shield.",						{ AT_BRV, AT_INT, AT_WIL }},
//	{ //, '/', "////////////////",	"//////////////////////////////////////////////////",	{ //////, //////, ////// }}, //
	{ 12, 'G', "Bartering", 		"Getting good prices from merchants.",					{ AT_BRV, AT_INT, AT_WIL }},
	{ 13, 'G', "Repair", 			"Repairing items.",										{ AT_INT, AT_WIL, AT_AGL }},
//	{ //, '/', "////////////////",	"//////////////////////////////////////////////////",	{ //////, //////, ////// }}, //
	{ 14, 'R', "Light", 			"Spell: Create light.",									{ AT_BRV, AT_INT, AT_WIL }},
	{ 15, 'R', "Recall", 			"Spell: Teleport to temple.",							{ AT_BRV, AT_INT, AT_WIL }},
	{ 16, 'R', "Guardian Angel", 	"Spell: Teleport to temple if hp<15%%.",				{ AT_BRV, AT_INT, AT_WIL }},
	{ 17, 'R', "Protection", 		"Spell: Enhance Armor of target.",						{ AT_BRV, AT_INT, AT_WIL }},
	{ 18, 'R', "Enhance Weapon", 	"Spell: Enhance Weapon of target.",						{ AT_BRV, AT_INT, AT_WIL }},
	{ 19, 'R', "Stun", 				"Spell: Make target motionless.",						{ AT_BRV, AT_INT, AT_WIL }},
	{ 20, 'R', "Curse", 			"Spell: Decrease attributes of target.",				{ AT_BRV, AT_INT, AT_WIL }},
	{ 21, 'R', "Bless", 			"Spell: Increase attributes of target.",				{ AT_BRV, AT_INT, AT_WIL }},
	{ 22, 'R', "Identify", 			"Spell: Read stats of item/character.",					{ AT_BRV, AT_INT, AT_WIL }},
//	{ //, '/', "////////////////",	"//////////////////////////////////////////////////",	{ //////, //////, ////// }}, //
	{ 23, 'G', "Resistance", 		"Resist against magic.",								{ AT_BRV, AT_WIL, AT_STR }},
//	{ //, '/', "////////////////",	"//////////////////////////////////////////////////",	{ //////, //////, ////// }}, //
	{ 24, 'R', "Blast", 			"Spell: Inflict injuries to target.",					{ AT_BRV, AT_INT, AT_WIL }},
	{ 25, 'R', "Dispel Magic", 		"Spell: Removes all magic from target.",				{ AT_BRV, AT_INT, AT_WIL }},
	{ 26, 'R', "Heal", 				"Spell: Heal injuries.",								{ AT_BRV, AT_INT, AT_WIL }},
	{ 27, 'R', "Ghost Companion", 	"Spell: Create ghostly slave to assist you.",			{ AT_BRV, AT_INT, AT_WIL }},
//	{ //, '/', "////////////////",	"//////////////////////////////////////////////////",	{ //////, //////, ////// }}, //
	{ 28, 'B', "Regenerate", 		"Regenerate Hitpoints faster.",							{ AT_STR, AT_STR, AT_STR }},
	{ 29, 'B', "Rest", 				"Regenerate Endurance faster.",							{ AT_AGL, AT_AGL, AT_AGL }},
	{ 30, 'B', "Meditate", 			"Regenerate Mana faster.",								{ AT_INT, AT_WIL, AT_WIL }},
//	{ //, '/', "////////////////",	"//////////////////////////////////////////////////",	{ //////, //////, ////// }}, //
	{ 31, 'G', "Sense Magic", 		"Find out who casts what at you.",						{ AT_BRV, AT_INT, AT_WIL }},
	{ 32, 'G', "Immunity", 			"Partial immunity against negative magic.",				{ AT_BRV, AT_AGL, AT_STR }},
	{ 33, 'G', "Surround Hit", 		"Hit all your enemies at once.",						{ AT_BRV, AT_AGL, AT_STR }},
	{ 34, 'G', "Concentrate", 		"Reduces mana cost for all spells.",					{ AT_WIL, AT_WIL, AT_WIL }},
	{ 35, 'G', "Warcry", 			"Frighten all enemies in hearing distance.",			{ AT_BRV, AT_BRV, AT_STR }}
//	{ //, '/', "////////////////",	"//////////////////////////////////////////////////",	{ //////, //////, ////// }}, //
};
*/

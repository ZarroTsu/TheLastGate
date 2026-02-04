/*************************************************************************

   This file is part of 'Mercenaries of Astonia v2'
   Copyright (c) 1997-2001 Daniel Brockhaus (joker@astonia.com)
   All rights reserved.

 **************************************************************************/

#include "server.h"

//		AT_BRV	AT_WIL	AT_INT	AT_AGL	AT_STR

struct s_skilltab skilltab[MAXSKILL+5] = {
//	{ //, '/', 	"////////////////",		"         '         '         '         '         '         '         '         '         '         '         '         ",
	{  0, 'C', 	"Hand to Hand", 		"Passive ability to hit and parry while unarmed.", 
				"", "",
				{ AT_BRV, AT_AGL, AT_STR }},
				
	{  1, 'G', 	"Precision", 			"Passively improves your ability to inflict critical hits.", 
				"", "",
				{ AT_BRV, AT_AGL, AT_AGL }},
				
	{  2, 'D', 	"Dagger", 				"Passive ability to hit and parry with a dagger in your main hand.", 
				"", "",
				{ AT_WIL, AT_WIL, AT_AGL }},
				
	{  3, 'D', 	"Sword", 				"Passive ability to hit and parry with a one-handed sword in your main hand.", 
				"", "",
				{ AT_BRV, AT_AGL, AT_STR }},
				
	{  4, 'D', 	"Axe", 					"Passive ability to hit and parry with an axe in your main hand.", 
				"", "",
				{ AT_AGL, AT_STR, AT_STR }},
				
	{  5, 'D', 	"Staff", 				"Passive ability to hit and parry with a staff in your main hand.", 
				"", "",
				{ AT_INT, AT_INT, AT_STR }},
				
	{  6, 'D', 	"Two-Handed", 			"Passive ability to hit and parry with a two-handed weapon in your main hand.", 
				"", "",
				{ AT_AGL, AT_AGL, AT_STR }},
				
	{  7, 'G', 	"Zephyr", 				"Passive ability granting your hits an additional hit after a brief delay.", 
				"Zephyr (Thorns)", 		"Passive ability granting retaliation hits after parrying.",
				{ AT_BRV, AT_BRV, AT_INT }},
				
	{  8, 'G', 	"Stealth", 				"Passive ability to stay hidden from others' sight. More effective while in SLOW mode.", 
				"", "",
				{ AT_INT, AT_AGL, AT_AGL }},
				
	{  9, 'G', 	"Perception", 			"Passive ability to see and hear your surroundings.", 
				"", "",
				{ AT_INT, AT_INT, AT_AGL }},
				
	{ 10, 'G', 	"Metabolism", 			"Passive ability to prevent the loss of hitpoints while you are underwater and against damage-over-time.", 
				"", "",
				{ AT_BRV, AT_WIL, AT_INT }},
				
	{ 11, 'F', 	"Magic Shield", 		"Use (Spell): Applies a buff to yourself, granting temporary armor.", 
				"Magic Shell", 			"Use (Spell): Applies a buff to yourself, granting temporary resistance and immunity.",
				{ AT_BRV, AT_WIL, AT_WIL }},
				
	{ 12, 'C', 	"Tactics", 				"Passive ability to hit and parry with any weapon. Grants summoned companions a similar bonus.", 
				"Tactics (Inverse)", 	"Passive ability to hit and parry with any weapon. Grants summoned companions a similar bonus.",
				{ AT_BRV, AT_WIL, AT_INT }},
				
	{ 13, 'E', 	"Repair", 				"Use (Skill): You will try to repair the item under your cursor.", 
				"", "",
				{ AT_INT, AT_AGL, AT_STR }},
				
	{ 14, 'G', 	"Finesse", 				"Passive ability which grants more global damage the healthier you are.", 
				"Finesse (Inverse)", 	"Passive ability which grants more global damage while near death.",
				{ AT_BRV, AT_BRV, AT_AGL }},
//	{ //, '/', 	"////////////////",		"         '         '         '         '         '         '         '         '         '         '         '         ",
	{ 15, 'F', 	"Lethargy", 			"Use (Spell): Applies a buff to yourself which reserves mana to let you pierce enemy Resistance and Immunity.", 
				"", "",
				{ AT_BRV, AT_WIL, AT_INT }},
				
	{ 16, 'D', 	"Shield", 				"Passive ability to parry while using a shield.", 
				"Shield Bash",          "Use (Skill): Strike your foe with your shield, stunning them and dealing damage proportional to your Armor Value.",
				{ AT_BRV, AT_WIL, AT_STR }},
				
	{ 17, 'F', 	"Protect", 				"Use (Spell): Applies a buff to you or your target, raising their armor value.", 
				"", "",
				{ AT_BRV, AT_WIL, AT_WIL }},
				
	{ 18, 'F', 	"Enhance", 				"Use (Spell): Applies a buff to you or your target, raising their weapon value.", 
				"", "",
				{ AT_BRV, AT_WIL, AT_WIL }},
				
	{ 19, 'F', 	"Slow", 				"Use (Spell): Applies a decaying debuff to your target and surrounding enemies, greatly reducing their action speed.", 
				"Slow (Greater)", 		"Use (Spell): Applies a debuff to your target and surrounding enemies, reducing their action speed.",
				{ AT_BRV, AT_INT, AT_INT }},
				
	{ 20, 'F', 	"Curse", 				"Use (Spell): Applies a debuff to your target and surrounding enemies, reducing their attributes.", 
				"Curse (Greater)", 		"Use (Spell): Applies a decaying debuff to your target and surrounding enemies, greatly reducing their attributes.",
				{ AT_BRV, AT_INT, AT_INT }},
				
	{ 21, 'F', 	"Bless", 				"Use (Spell): Applies a buff to you or your target, raising their attributes.", 
				"", "",
				{ AT_BRV, AT_WIL, AT_WIL }},
//	{ //, '/', 	"////////////////",		"         '         '         '         '         '         '         '         '         '         '         '         ",
	{ 22, 'E', 	"Pact", 				"Use (Skill): Applies a buff to yourself which reserves health to grant additional Damage and Damage Reduction.",
				"Pact (Mana)",          "Use (Skill): Applies a buff to yourself which reserves mana to grant additional Damage and Damage Reduction.",
				{ AT_BRV, AT_INT, AT_STR }},
				
	{ 23, 'G', 	"Resistance", 			"Passive ability to avoid enemy negative spells.", 
				"", "",
				{ AT_BRV, AT_WIL, AT_STR }},
//	{ //, '/', 	"////////////////",		"         '         '         '         '         '         '         '         '         '         '         '         ",
	{ 24, 'F', 	"Blast", 				"Use (Spell): Damages your target and surrounding enemies.", 
				"Blast (Scorch)", 		"Use (Spell): Damages your target and surrounding enemies. This also applies a debuff which increases damage taken.", 
				{ AT_BRV, AT_INT, AT_INT }},
				
	{ 25, 'F', 	"Dispel", 				"Use (Spell): Removes debuffs from your target.", 
				"Dispel (Enemy)", 		"Use (Spell): Removes buffs from your target.",
				{ AT_BRV, AT_WIL, AT_INT }},
				
	{ 26, 'F', 	"Heal", 				"Use (Spell): Heals you or your target. This also applies Healing Sickness, reducing the power of consecutive heals.", 
				"Heal (Regen)", 		"Use (Spell): Applies a buff to you or your target, granting them health regeneration.",
				{ AT_BRV, AT_WIL, AT_STR }},
				
	{ 27, 'F', 	"Ghost Companion", 		"Use (Spell): Summons a companion to follow you and your commands. Say COMMAND to it for a list of commands.", 
				"", "",
				{ AT_BRV, AT_WIL, AT_WIL }},
				
	{ 28, 'A', 	"Regenerate", 			"Passive ability to recover hitpoints over time.", 
				"", "",
				{ AT_STR, AT_STR, AT_STR }},
				
	{ 29, 'A', 	"Rest", 				"Passive ability to recover endurance over time.", 
				"", "",
				{ AT_AGL, AT_AGL, AT_AGL }},
				
	{ 30, 'B', 	"Meditate", 			"Passive ability to recover mana over time.", 
				"", "",
				{ AT_INT, AT_INT, AT_INT }},
//	{ //, '/', 	"////////////////",		"         '         '         '         '         '         '         '         '         '         '         '         ",
	{ 31, 'G', 	"Aria", 				"Passively grants you and nearby allies a buff based off equipped rings, and a debuff to enemies.", 
				"", "",
				{ AT_BRV, AT_AGL, AT_AGL }},
				
	{ 32, 'G', 	"Immunity", 			"Passive ability to reduce the strength of enemy negative spells.", 
				"", "",
				{ AT_BRV, AT_AGL, AT_STR }},
				
	{ 33, 'G', 	"Surround Hit", 		"Passive ability to deal a portion of melee hit damage to all foes around you.", 
				"", "",
				{ AT_AGL, AT_STR, AT_STR }},
//	{ //, '/', 	"////////////////",		"         '         '         '         '         '         '         '         '         '         '         '         ",
	{ 34, 'G', 	"Economize", 			"Passive ability to reduce the mana cost of spells and abilities. Grants better prices while buying or selling.", 
				"", "",
				{ AT_WIL, AT_WIL, AT_WIL }},
				
	{ 35, 'E', 	"Warcry", 				"Use (Skill): Shout to stun and strike fear into all nearby enemies. Has a base radius of 6 tiles.", 
				"Warcry (Rally)", 		"Use (Skill): Shout to rally yourself and your allies. Has a base radius of 6 tiles.",
				{ AT_BRV, AT_STR, AT_STR }},
				
	{ 36, 'D', 	"Dual Wield", 			"Passive ability to hit while using a dual-sword.", 
				"", "",
				{ AT_BRV, AT_AGL, AT_STR }},
//	{ //, '/', 	"////////////////",		"         '         '         '         '         '         '         '         '         '         '         '         ",
	{ 37, 'E', 	"Blind", 				"Use (Skill): Applies a debuff which reduces hit and parry scores. Has a base radius of 4 tiles.", 
				"Blind (Douse)", 		"Use (Skill): Applies a debuff which reduces stealth and spell modifier. Has a base radius of 4 tiles.",
				{ AT_BRV, AT_INT, AT_AGL }},
				
	{ 38, 'G', 	"Gear Mastery", 		"Passive ability to improve weapon and armor values granted by your equipment.", 
				"", "",
				{ AT_BRV, AT_AGL, AT_STR }},
				
	{ 39, 'G', 	"Safeguard", 			"Passive ability to reduce damage taken.", 
				"", "",
				{ AT_BRV, AT_STR, AT_STR }},
//	{ //, '/', 	"////////////////",		"         '         '         '         '         '         '         '         '         '         '         '         ",
	{ 40, 'E', 	"Cleave", 				"Use (Skill): Strike your foe and surrounding enemies. This also applies a debuff which deals damage over time.", 
				"Cleave (Aggravate)",	"Use (Skill): Strike your foe and surrounding enemies. This also applies a debuff which causes additional damage.",
				{ AT_AGL, AT_STR, AT_STR }},
				
	{ 41, 'E', 	"Weaken", 				"Use (Skill): Applies a debuff which reduces weapon value.", 
				"Weaken (Crush)", 		"Use (Skill): Applies a debuff which reduces armor value.",
				{ AT_BRV, AT_AGL, AT_AGL }},
//	{ //, '/', 	"////////////////",		"         '         '         '         '         '         '         '         '         '         '         '         ",
	{ 42, 'F', 	"Poison", 				"Use (Spell): Applies a debuff which causes them to take damage over time.", 
				"Poison (Venom)", 		"Use (Spell): Applies a stacking debuff which reduces immunity and causes damage over time. Stacks up to 3 times.",
				{ AT_BRV, AT_INT, AT_INT }},
//	{ //, '/', 	"////////////////",		"         '         '         '         '         '         '         '         '         '         '         '         ",
	{ 43, 'F', 	"Pulse", 				"Use (Spell): Applies a buff to yourself, bursting with energy to shock nearby foes. Has a base radius of 3 tiles.", 
				"Pulse (Charge)", 		"Use (Spell): Applies a buff to yourself, bursting with energy to charge nearby allies. Has a base radius of 3 tiles.",
				{ AT_BRV, AT_INT, AT_INT }},
				
	{ 44, 'G', 	"Proximity", 			"Passively improves the area-of-effect of several skills.",
				"", "",
				{ AT_BRV, AT_WIL, AT_INT }},
				
	{ 45, 'G', 	"Companion Mastery", 	"Passively increases the limit and number of abilities known by your ghost companion.", 
				"", "",
				{ AT_BRV, AT_WIL, AT_WIL }},
				
	{ 46, 'F', 	"Shadow Copy", 			"Use (Spell): Summons a temporary doppelganger to attack your enemies.", 
				"", "",
				{ AT_BRV, AT_WIL, AT_WIL }},
				
	{ 47, 'F', 	"Haste", 				"Use (Spell): Applies a buff to yourself, increasing your action speed.", 
				"", "",
				{ AT_BRV, AT_WIL, AT_AGL }},
//	{ //, '/', 	"////////////////",		"         '         '         '         '         '         '         '         '         '         '         '         ",
	{ 48, 'E', 	"Taunt",				"Use (Skill): Forces the target and surrounding enemies to attack you. Grants you damage resistance when it hits.", 
				"", "",
				{ AT_BRV, AT_STR, AT_STR }},
//	{ //, '/', 	"////////////////",		"         '         '         '         '         '         '         '         '         '         '         '         ",
	{ 49, 'E', 	"Leap", 				"Use (Skill): Strike your foe and leap to a number of random nearby enemies.",
				"Leap (Critical)", 		"Use (Skill): Strike your foe and leap to your target, dealing critical damage and stunning each enemy hit.",
				{ AT_BRV, AT_AGL, AT_STR }},
				
	{ 50, 'H', 	"Light", 				"Use (Spell): Applies a buff to you or your target, making them glow in the dark.", 
				"", "",
				{ 0, 0, 0 }},
	{ 51, 'H', 	"Recall", 				"Use (Spell): Teleport yourself to a safe location after a brief delay.", 
				"", "",
				{ 0, 0, 0 }},
	{ 52, 'H', 	"Identify", 			"Use (Spell): Identify the properties of a target or an item. Can be used on an already identified item to clear it.", 
				"", "",
				{ 0, 0, 0 }},
	{ 53, 'H', 	"Ferocity", 			"Passively grants a bonus to WV and AV. The bonus increases for each empty gear slot.", 
				"", "",
				{ 0, 0, 0 }},
//	{ //, '/', 	"////////////////",		"         '         '         '         '         '         '         '         '         '         '         '         ",
	{ 54, 'H', 	"Shift", 				"Use (Skill): Change form from that of a Ratling to that of a Greenling, and vice versa. Has a unique cooldown timer.", 
				"", "",
				{ 0, 0, 0 }}
};

struct sk_tree sk_tree[10][12]={
	{	// Seyan'du
		{ "Accuracy",                      "+4 to Hit Score.",
		  6685,                            "" },
		{ "Expertise",                     "+2 to all Attributes.",
		  6602,                            "" },
		{ "Avoidance",                     "+4 to Parry Score.",
		  6624,                            "" },
		//"         '         '         ", "         '         '         '         '      "
		{ "Absolution",                    "2%% more damage dealt for each buff or debuff ",
		  6604,                            "on you." },
		{ "Rigor",                         "4%% increased total Hit Score.",
		  6689,                            "" },
		{ "Scorn",                         "Your skills and spells ignore 20%% of enemy ",
		  6606,                            "immunity." },
		//"         '         '         ", "         '         '         '         '      "
		{ "Determination",                 "Gain +1 to Weapon and Armor Value for every ",
		  6615,                            "75 total attributes." },
		{ "Jack of All Trades",            "5%% increased total Attributes.",
		  6608,                            "" },
		{ "Brilliance",                    "Gain +1 to Spellpower Bonus for every 75 ",
		  6623,                            "total attributes." },
		//"         '         '         ", "         '         '         '         '      "
		{ "Enigmatic",                     "20%% less effect of debuffs on you.",
		  6610,                            "" },
		{ "Flexibility",                   "4%% increased total Parry Score.",
		  6695,                            "" },
		{ "Penance",                       "2%% less damage taken for each buff or debuff ",
		  6612,                            "on you." }
	}, // "         '         '         ", "         '         '         '         '      "
	{	// Arch Templar
		{ "Ravager",                       "+6 to Top Damage.",
		  6697,                            "" },
		{ "Might",                         "+6 to Strength.",
		  6614,                            "" },
		{ "Toughness",                     "+4 to Armor Value.",
		  6603,                            "" },
		//"         '         '         ", "         '         '         '         '      "
		{ "Bulwark",                       "30%% less Top Damage taken from enemies ",
		  6612,                            "attacking you." },
		{ "Vanquisher",                    "10%% increased total Top Damage.",
		  6701,                            "" },
		{ "Impact",                        "Your Top Damage is rolled an additional time ",
		  6702,                            "on hit, using the higher roll." },
		//"         '         '         ", "         '         '         '         '      "
		{ "Barbarism",                     "Your active melee skills gain an additional ",
		  6625,                            "20%% Strength bonus." },
		{ "Overwhelming Strength",         "6%% increased total Strength. ",
		  6620,                            "+10 to Strength Limit." },
		{ "Overlord",                      "1%% increased effect of Warcry for every 10 ",
		  6619,                            "total Strength." },
		//"         '         '         ", "         '         '         '         '      "
		{ "Rampart",                       "50%% more parry granted by your Shield skill.",
		  6624,                            "" },
		{ "Unbreakable",                   "10%% increased total Armor Value.",
		  6611,                            "" },
		{ "Fortitude",                     "1%% increased effect of Safeguard for every 10 ",
		  6622,                            "total Armor Value." }
	}, // "         '         '         ", "         '         '         '         '      "
	{	// Skald
		{ "Decisiveness",                  "+2%% to Base Critical Hit Chance.",
		  6688,                            "" },
		{ "Dexterity",                     "+6 to Agility.",
		  6626,                            "" },
		{ "Sanctity",                      "+4 to Immunity.",
		  6667,                            "" },
		//"         '         '         ", "         '         '         '         '      "
		{ "Litheness",                     "50%% reduced extra damage taken from enemy ",
		  6634,                            "critical strikes." },
		{ "Brutality",                     "+20%% to Critial Hit Multiplier.",
		  6631,                            "" },
		{ "Crushing Blows",                "Your Cleave skill can inflict critical hits.",
		  6616,                            "" },
		//"         '         '         ", "         '         '         '         '      "
		{ "Nocturne",                      "1%% increased effect of Precision for every 10 ",
		  6687,                            "total Agility." },
		{ "Overwhelming Agility",          "6%% increased total Agility. ",
		  6632,                            "+10 to Agility Limit." },
		{ "Celerity",                      "Your active melee skills gain an additional ",
		  6625,                            "20%% Agility bonus." },
		//"         '         '         ", "         '         '         '         '      "
		{ "Guardian",                      "Your Aria grants affected allies 20%% of your ",
		  6672,                            "total Immunity." },
		{ "Indifference",                  "10%% increased total Immunity.",
		  6681,                            "" },
		{ "Bastion",                       "20%% reduced reservation of hitpoints, ",
		  6628,                            "endurance, or mana." }
	}, // "         '         '         ", "         '         '         '         '      "
	{	// Warrior
		{ "Rapidity",                      "+6 to Attack Speed.",
		  6637,                            "" },
		{ "Ruffian",                       "+4 to Strength & +4 to Agility.",
		  6638,                            "" },
		{ "Stamina",                       "+30 to Maximum Endurance.",
		  6627,                            "" },
		//"         '         '         ", "         '         '         '         '      "
		{ "Dismissal",                     "1%% increased effect of Immunity for every 25 ",
		  6648,                            "uncapped Attack Speed." },
		{ "Swiftness",                     "20%% increased total Attack Speed.",
		  6641,                            "" },
		{ "Flash Step",                    "Your Leap skill repeats an additional time.",
		  6640,                            "" },
		//"         '         '         ", "         '         '         '         '      "
		{ "Slayer",                        "1%% increased power of Leap for every 10 total ",
		  6696,                            "Strength." },
		{ "Harrier",                       "4%% increased total Agility and Strength.",
		  6644,                            "" },
		{ "Antagonizer",                   "1%% increased power of Blind for every 10 ",
		  6643,                            "total Agility." },
		//"         '         '         ", "         '         '         '         '      "
		{ "Champion",                      "Enemies beside and behind you no longer gain ",
		  6646,                            "a bonus to hitting you." },
		{ "Perseverance",                  "20%% increased Maximum Endurance. ",
		  6635,                            "2%% of damage dealt is leeched as Endurance." },
		{ "Tenacity",                      "30%% of damage taken is dealt to your ",
		  6636,                            "Endurance before Hitpoints." }
	}, // "         '         '         ", "         '         '         '         '      "
	{	// Sorcerer
		{ "Passion",                       "+4 to Spell Aptitude.",
		  6639,                            "" },
		{ "Potency",                       "+4 to Willpower & +4 to Intuition.",
		  6650,                            "" },
		{ "Quickstep",                     "+6 to Movement Speed.",
		  6651,                            "" },
		//"         '         '         ", "         '         '         '         '      "
		{ "Introspection",                 "Gain +1 to Spellpower Bonus for every 25 ",
		  6623,                            "total Spell Aptitude." },
		{ "Zealotry",                      "10%% increased total Spell Aptitude.",
		  6647,                            "" },
		{ "Rewind",                        "Your Slow spell is 25%% weaker, but is now a ",
		  6658,                            "passive aura with a base radius of 4." },
		//"         '         '         ", "         '         '         '         '      "
		{ "Toxins",                        "1%% increased power of Poison for every 10 ",
		  6652,                            "total Intuition." },
		{ "Pragmatism",                    "4%% increased total Willpower and Intuition.",
		  6656,                            "" },
		{ "Hex Master",                    "1%% increased power of Lethargy for every 10 ",
		  6655,                            "total Willpower." },
		//"         '         '         ", "         '         '         '         '      "
		{ "Fast Forward",                  "Your Haste spell is 25%% weaker, but is now a ",
		  6660,                            "passive aura with a base radius of 4." },
		{ "Fleet-Foot",                    "20%% increased total Movement Speed.",
		  6659,                            "" },
		{ "Dodging",                       "1%% increased effect of Resistance for every ",
		  6648,                            "25 uncapped Movement Speed." }
	}, // "         '         '         ", "         '         '         '         '      "
	{	// Summoner
		{ "Nimbleness",                    "+6 to Cast Speed.",
		  6661,                            "" },
		{ "Wisdom",                        "+6 to Willpower.",
		  6662,                            "" },
		{ "Barrier",                       "+4 to Resistance.",
		  6667,                            "" },
		//"         '         '         ", "         '         '         '         '      "
		{ "Strategist",                    "Increases and multipliers to Cast Speed also ",
		  6664,                            "affect Attack Speed." },
		{ "Spellslinger",                  "20%% increased total Cast Speed.",
		  6665,                            "" },
		{ "Tactician",                     "1%% increased effect of Tactics for every 25 ",
		  6666,                            "uncapped Cast Speed." },
		//"         '         '         ", "         '         '         '         '      "
		{ "Mysticism",                     "Your active magic spells gain an additional ",
		  6629,                            "20%% Willpower bonus." },
		{ "Overwhelming Willpower",        "6%% increased total Willpower. ",
		  6668,                            "+10 to Willpower Limit." },
		{ "Shaper",                        "1%% increased effect of Companion Mastery for ",
		  6669,                            "every 10 total Willpower." },
		//"         '         '         ", "         '         '         '         '      "
		{ "Diviner",                       "Casting a companion skill while holding a ",
		  6670,                            "tarot card grants the companion its effects." },
		{ "Constitution",                  "10%% increased total Resistance.",
		  6681,                            "" },
		{ "Necromancy",                    "2%% of damage dealt by your companions is ",
		  6703,                            "leeched to you as Hitpoints." }
	}, // "         '         '         ", "         '         '         '         '      "
	{	// Arch Harakim
		{ "Composure",                     "+4 to Cooldown Rate.",
		  6673,                            "" },
		{ "Intellect",                     "+6 to Intuition.",
		  6674,                            "" },
		{ "Wellspring",                    "+30 to Maximum Mana.",
		  6675,                            "" },
		//"         '         '         ", "         '         '         '         '      "
		{ "Malice",                        "Your spells cannot be suppressed.",
		  6657,                            "" },
		{ "Serenity",                      "10%% increased total Cooldown Rate.",
		  6677,                            "" },
		{ "Destroyer",                     "Your Blast spell repeats an additional time ",
		  6676,                            "at the end of its cooldown." },
		//"         '         '         ", "         '         '         '         '      "
		{ "Psychosis",                     "1%% increased power of Pulse for every 10 ",
		  6679,                            "total Intuition." },
		{ "Overwhelming Intuition",        "6%% increased total Intuition. ",
		  6680,                            "+10 to Intuition Limit." },
		{ "Concentration",                 "Your active magic spells gain an additional ",
		  6629,                            "20%% Intuition bonus." },
		//"         '         '         ", "         '         '         '         '      "
		{ "Flow",                          "Gain +1 to Spellpower bonus for every 100 ",
		  6623,                            "uncapped Maximum Mana." },
		{ "Perpetuity",                    "20%% increased Maximum Mana. ",
		  6683,                            "2%% of damage dealt is leeched as Mana." },
		{ "Resourcefulness",               "30%% of damage taken is dealt to your Mana ",
		  6684,                            "before Hitpoints." }
	}, // "         '         '         ", "         '         '         '         '      "
	{	// Braver
		{ "Muscle",                        "+4 to Weapon Value.",
		  6601,                            "" },
		{ "Boldness",                      "+6 to Braveness.",
		  6686,                            "" },
		{ "Mindfulness",                   "+4 to Spellpower Bonus.",
		  6642,                            "" },
		//"         '         '         ", "         '         '         '         '      "
		{ "Perfectionism",                 "1%% increased effect of Finesse for every 10 ",
		  6691,                            "total Weapon Value." },
		{ "Valor",                         "10%% increased total Weapon Value.",
		  6605,                            "" },
		{ "Presence",                      "Your Curse spell is 25%% weaker, but is now a ",
		  6658,                            "passive aura with a base radius of 4." },
		//"         '         '         ", "         '         '         '         '      "
		{ "Virtuosity",                    "Your passive skills gain an additional 20%% ",
		  6645,                            "Braveness bonus." },
		{ "Overwhelming Braveness",        "6%% increased total Braveness. ",
		  6692,                            "+10 to Braveness Limit." },
		{ "Alacrity",                      "1%% increased power of Zephyr for every 10 ",
		  6640,                            "total Braveness." },
		//"         '         '         ", "         '         '         '         '      "
		{ "Spellblade",                    "Your Spellpower Bonus is granted as ",
		  6654,                            "additional Top Damage." },
		{ "Wizardry",                      "20%% increased total Spellpower Bonus.",
		  6707,                            "" },
		{ "Resilience",                    "40%% reduced effect of healing sickness on ",
		  6694,                            "you." }
	}, // "         '         '         ", "         '         '         '         '      "
	{	// Lycanthrope
		{ "Expanse",                       "+2 to Area of Effect.",
		  6649,                            "" },
		{ "Feast",                         "+30 to Maximum Hitpoints.",
		  6663,                            "" },
		{ "Sharkskin",                     "+6 to Thorns.",
		  6613,                            "" },
		//"         '         '         ", "         '         '         '         '      "
		{ "Sickness",                      "Your Weaken skill is 25%% weaker, but is now ",
		  6658,                            "a passive aura with a radius of 2." },
		{ "Pride",                         "50%% increased total Area of Effect.",
		  6653,                            "" },
		{ "Greed",                         "Enemies affected by your debuffs grant 2%% of ",
		  6700,                            "damage taken as Hitpoints to the attacker." },
		//"         '         '         ", "         '         '         '         '      "
		{ "Lust",                          "50%% increased effect of recovery from all ",
		  6706,                            "sources." },
		{ "Gluttony",                      "20%% increased Maximum Hitpoints. ",
		  6671,                            "2%% of damage dealt is leeched as Hitpoints." },
		{ "Wrath",                         "1%% increased effect of bonus from Pact for ",
		  6705,                            "every 50 uncapped Maximum Hitpoints." },
		//"         '         '         ", "         '         '         '         '      "
		{ "Sloth",                         "20%% of Thorns is triggered upon parrying.",
		  6618,                            "" },
		{ "Envy",                          "20%% increased total Thorns score.",
		  6617,                            "" },
		{ "Serration",                     "Bleeding inflicted by you deals increased ",
		  6702,                            "damage equal to your Thorns score." }
	}, // "         '         '         ", "         '         '         '         '      "
	{	// Contract
		{ "Reward",                        "+1%% chance of finding Rainbow Belts and ",
		  6709,                            "Unusual Portals in contracts signed by you." },
		{ "Challenge",                     "+1 to rank of contracts signed by you.",
		  6710,                            "" },
		{ "Army",                          "+1 enemy per spawn in contracts signed by you.",
		  6711,                            "" },
		//"         '         '         ", "         '         '         '         '      "
		{ "Hope",                          "20%% increased effect of rewards from green ",
		  6712,                            "shrines in contracts signed by you." },
		{ "Opalescence",                   "+1%% chance of finding Rainbow Belts and ",
		  6713,                            "Unusual Portals in contracts signed by you." },
		{ "Scholar",                       "20%% more clear experience from contracts ",
		  6714,                            "signed by you." },
		//"         '         '         ", "         '         '         '         '      "
		{ "Fate",                          "20%% more effect of blue shrines in contracts ",
		  6715,                            "signed by you." },
		{ "Hubris",                        "+1 to rank of contracts signed by you.",
		  6716,                            "" },
		{ "Binding",                       "Contracts signed by you always grant tier 3 ",
		  6717,                            "effects from quills." },
		//"         '         '         ", "         '         '         '         '      "
		{ "Destiny",                       "Red shrines produce harder enemies with more ",
		  6718,                            "rewards in contracts signed by you." },
		{ "Swarm",                         "+1 enemy per spawn in contracts signed by you.",
		  6719,                            "" },
		{ "Incentive",                     "Enemies grant an additional 5%% of exp as ",
		  6720,                            "Contract Pts in contracts signed by you." }
	}  // "         '         '         ", "         '         '         '         '      "
};

struct sk_tree sk_corrupt[NUM_CORR]={
   // "         '         '         ", "         '         '         '         '      "
	{ "False Accuracy",                "(+2) to Hit Score.",
	  6685,                            "" },
	{ "False Expertise",               "(+1) to all Attributes.",
	  6602,                            "" },
	{ "False Avoidance",               "(+2) to Parry Score.",
	  6624,                            "" },
	//"         '         '         ", "         '         '         '         '      "
	{ "False Absolution",              "(+1%%) more damage dealt for each buff or ",
	  6604,                            "debuff on you." },
	{ "False Rigor",                   "(+2%%) increased total Hit Score.",
	  6689,                            "" },
	{ "False Scorn",                   "Your skills and spells ignore (+10%%) of enemy ",
	  6606,                            "immunity." },
	//"         '         '         ", "         '         '         '         '      "
	{ "False Determination",           "Gain (+1) to Weapon and Armor Value for every ",
	  6615,                            "150 total attributes." },
	{ "Master of None",                "(+5) to all Skill limits.",
	  6704,                            "" },
	{ "False Brilliance",              "Gain (+1) to Spellpower Bonus for every 150 ",
	  6623,                            "total attributes." },
	//"         '         '         ", "         '         '         '         '      "
	{ "False Enigmatic",               "(+10%%) less effect of debuffs on you.",
	  6610,                            "" },
	{ "False Flexibility",             "(+2%%) increased total Parry Score.",
	  6695,                            "" },
	{ "False Penance",                 "(+1%%) less damage taken for each buff or ",
	  6612,                            "debuff on you." },
	//"         '         '         ", "         '         '         '         '      " ////////
	{ "False Ravager",                 "(+3) to Top Damage.",
	  6697,                            "" },
	{ "False Might",                   "(+3) to Strength.",
	  6614,                            "" },
	{ "False Toughness",               "(+2) to Armor Value.",
	  6603,                            "" },
	//"         '         '         ", "         '         '         '         '      "
	{ "False Bulwark",                 "(+15%%) less Top Damage taken from enemies ",
	  6612,                            "attacking you." },
	{ "False Vanquisher",              "(+5%%) increased total Top Damage.",
	  6701,                            "" },
	{ "Goliath",                       "(+15%%) increased effect of stats granted by ",
	  6630,                            "Greataxes." },
	//"         '         '         ", "         '         '         '         '      "
	{ "False Barbarism",               "Your active melee skills gain an additional ",
	  6625,                            "(+10%%) Strength bonus." },
	{ "False Strength",                "(+3%%) increased total Strength. ",
	  6620,                            "(+5) to Strength Limit." },
	{ "False Overlord",                "(+1%%) increased effect of Warcry for every 20 ",
	  6619,                            "total Strength." },
	//"         '         '         ", "         '         '         '         '      "
	{ "Full Cover",                    "(+15%%) incresed effect of stats granted by ",
	  6621,                            "Shields." },
	{ "False Unbreakable",             "(+5%%) increased total Armor Value.",
	  6611,                            "" },
	{ "False Fortitude",               "(+1%%) increased effect of Safeguard for every ",
	  6622,                            "20 total Armor Value." },
	//"         '         '         ", "         '         '         '         '      " ////////
	{ "False Decisiveness",            "(+1%%) to Base Critical Hit Chance.",
	  6688,                            "" },
	{ "False Dexterity",               "(+3) to Agility.",
	  6626,                            "" },
	{ "False Sanctity",                "(+2) to Immunity.",
	  6667,                            "" },
	//"         '         '         ", "         '         '         '         '      "
	{ "False Litheness",               "(+25%%) reduced extra damage taken from enemy ",
	  6634,                            "critical strikes." },
	{ "False Brutality",               "(+10%%) to Critical Hit Multiplier.",
	  6631,                            "" },
	{ "Monkey Grip",                   "(+15%%) increased effect of stats granted by ",
	  6630,                            "Twohanders." },
	//"         '         '         ", "         '         '         '         '      "
	{ "False Nocturne",                "(+1%%) increased effect of Precision for every ",
	  6687,                            "20 total Agility." },
	{ "False Agility",                 "(+3%%) increased total Agility. ",
	  6632,                            "(+5) to Agility Limit." },
	{ "False Celerity",                "Your active melee skills gain an additional ",
	  6625,                            "(+10%%) Agility bonus." },
	//"         '         '         ", "         '         '         '         '      "
	{ "Towering Presence",             "(+15%%) increased effect of passive aura ",
	  6672,                            "abilities." },
	{ "False Indifference",            "(+5%%) increased total Immunity.",
	  6681,                            "" },
	{ "False Bastion",                 "(+10%%) reduced reservation of hitpoints, ",
	  6628,                            "endurance, or mana." },
	//"         '         '         ", "         '         '         '         '      " ////////
	{ "False Rapidity",                "(+3) to Attack speed.",
	  6637,                            "" },
	{ "False Ruffian",                 "(+2) to Strength & (+2) to Agility.",
	  6638,                            "" },
	{ "False Stamina",                 "(+15) to Maximum Endurance.",
	  6627,                            "" },
	//"         '         '         ", "         '         '         '         '      "
	{ "False Dismissal",               "(+1%%) increased effect of Immunity for every ",
	  6648,                            "50 uncapped Attack Speed." },
	{ "False Swiftness",               "(+10%%) incressed total Attack Speed.",
	  6641,                            "" },
	{ "Axeman",                        "(+15%%) increased effect of stats granted by ",
	  6630,                            "Axes." },
	//"         '         '         ", "         '         '         '         '      "
	{ "False Slayer",                  "(+1%%) increased power of Leap for every 20 ",
	  6696,                            "total Strength." },
	{ "False Harrier",                 "(+2%%) increased total Agility and Strength.",
	  6644,                            "" },
	{ "False Antagonizer",             "(+1%%) increased power of Blind for every 20 ",
	  6643,                            "total Agility." },
	//"         '         '         ", "         '         '         '         '      "
	{ "Severance",                     "(+15%%) increased effect of stats granted by ",
	  6685,                            "Dual Swords." },
	{ "False Perseverance",            "(+10%%) increased Maximum Endurance. (+1%%) ",
	  6635,                            "of damage dealt is leeched as Endurance." },
	{ "False Tenacity",                "(+15%%) of damage taken is dealt to your ",
	  6636,                            "Endurance before Hitpoints." },
	//"         '         '         ", "         '         '         '         '      " ////////
	{ "False Passion",                 "(+2) to Spell Aptitude.",
	  6639,                            "" },
	{ "False Potency",                 "(+2) to Willpower & (+2) to Intuition.",
	  6650,                            "" },
	{ "False Quickstep",               "(+3) to Movement Speed.",
	  6651,                            "" },
	//"         '         '         ", "         '         '         '         '      "
	{ "False Introspection",           "Gain (+1) to Spellpower Bonus for every 50 ",
	  6623,                            "total Spell Aptitude." },
	{ "False Zealotry",                "(+5%%) increased total Spell Aptitude.",
	  6647,                            "" },
	{ "Assassin",                      "(+15%%) increased effect of stats granted by ",
	  6630,                            "Daggers." },
	//"         '         '         ", "         '         '         '         '      "
	{ "False Toxins",                  "(+1%%) increased power of Poison for every 20 ",
	  6652,                            "total Intuition." },
	{ "False Pragmatism",              "(+2%%) increased total Willpower and ",
	  6656,                            "Intuition." },
	{ "False Hex Master",              "(+1%%) increased power of Lethargy for every ",
	  6655,                            "20 total Willpower." },
	//"         '         '         ", "         '         '         '         '      "
	{ "Skip Ahead",                    "Up to (+15%%) of missing Health, Endurance & ",
	  6660,                            "Mana is recovered when skill exhaustion ends." },
	{ "False Fleet-Foot",              "(+10%%) increased total Movement Speed.",
	  6659,                            "" },
	{ "False Dodging",                 "(+1%%) increased effect of Resistance for ",
	  6648,                            "every 50 uncapped Movement Speed." },
	//"         '         '         ", "         '         '         '         '      " ////////
	{ "False Nimbleness",              "(+3) to Cast Speed.",
	  6661,                            "" },
	{ "False Wisdom",                  "(+3) to Willpower.",
	  6662,                            "" },
	{ "False Barrier",                 "(+2) to Resistance.",
	  6667,                            "" },
	//"         '         '         ", "         '         '         '         '      "
	{ "Harpooner",                     "(+15%%) increased effect of stats granted by ",
	  6630,                            "Spears." },
	{ "False Spellslinger",            "(+10%%) increased total Cast Speed.",
	  6665,                            "" },
	{ "False Tactician",               "(+1%%) increased effect of Tactics for every ",
	  6666,                            "50 uncapped Cast Speed." },
	//"         '         '         ", "         '         '         '         '      "
	{ "False Mysticism",               "Your active magic spells gain an additional ",
	  6629,                            "(+10%%) Willpower bonus." },
	{ "False Willpower",               "(+3%%) increased total Willpower. ",
	  6668,                            "(+5) to Willpower Limit." },
	{ "False Shaper",                  "(+1%%) increased effect of Companion Mastery ",
	  6669,                            "for every 20 total Willpower." },
	//"         '         '         ", "         '         '         '         '      "
	{ "Wraithlord",                    "(+2%%) of damage dealt by you is leeched to ",
	  6708,                            "your companions as Hitpoints." },
	{ "False Constitution",            "(+5%%) increased total Resistance.",
	  6681,                            "" },
	{ "False Necromancy",              "(+1%%) of damage dealt by your companions is ",
	  6703,                            "leeched to you as Hitpoints." },
	//"         '         '         ", "         '         '         '         '      " ////////
	{ "False Composure",               "(+2) to Cooldown Rate.",
	  6673,                            "" },
	{ "False Intellect",               "(+3) to Intuition.",
	  6674,                            "" },
	{ "False Wellspring",              "(+15) to Maximum Mana.",
	  6675,                            "" },
	//"         '         '         ", "         '         '         '         '      "
	{ "Repurpose",                     "(+15%%) of uncapped Maximum Mana is converted ",
	  6682,                            "into Maximum Hitpoints." },
	{ "False Serenity",                "(+5%%) increased total Cooldown Rate.",
	  6677,                            "" },
	{ "Warlock",                       "(+15%%) increased effect of stats granted by ",
	  6630,                            "Staffs." },
	//"         '         '         ", "         '         '         '         '      "
	{ "False Psychosis",               "(+1%%) increased power of Pulse for every 20 ",
	  6679,                            "total Intuition." },
	{ "False Intuition",               "(+3%%) increased total Intuition. ",
	  6680,                            "(+5) to Intuition Limit." },
	{ "False Concentration",           "Your active magic spells gain an additional ",
	  6629,                            "(+10%%) Intuition bonus." },
	//"         '         '         ", "         '         '         '         '      "
	{ "False Flow",                    "Gain (+1) Spellpower bonus for every 200 ",
	  6623,                            "uncapped Maximum Mana." },
	{ "False Perpetuity",              "(+10%%) increased Maximum Mana. ",
	  6683,                            "(+1%%) of damage dealt is leeched as Mana." },
	{ "False Resourcefulness",         "(+15%%) of damage taken is dealt to your Mana ",
	  6684,                            "before Hitpoints." },
	//"         '         '         ", "         '         '         '         '      " ////////
	{ "False Muscle",                  "(+2) to Weapon Value.",
	  6601,                            "" },
	{ "False Boldness",                "(+3) to Braveness.",
	  6686,                            "" },
	{ "False Mindfulness",             "(+2) to Spellpower Bonus.",
	  6642,                            "" },
	//"         '         '         ", "         '         '         '         '      "
	{ "False Perfectionism",           "(+1%%) increased effect of Finesse for every ",
	  6691,                            "20 total Weapon Value." },
	{ "False Valor",                   "(+5%%) increased total Weapon Value.",
	  6605,                            "" },
	{ "Madness",                       "(+2) to Spellpower Multiplier.",
	  6699,                            "" },
	//"         '         '         ", "         '         '         '         '      "
	{ "False Virtuosity",              "Your passive skills gain an additional (+10%%) ",
	  6645,                            "Braveness bonus." },
	{ "False Braveness",               "(+3%%) increased total Braveness. ",
	  6692,                            "(+5) to Braveness Limit." },
	{ "False Alacrity",                "(+1%%) increased power of Zephyr for every 20 ",
	  6640,                            "total Braveness." },
	//"         '         '         ", "         '         '         '         '      "
	{ "Swordsman",                     "(+15%%) increased effect of stats granted by ",
	  6630,                            "Swords." },
	{ "False Wizardry",                "(10%%) increased total Spellpower Bonus.",
	  6707,                            "" },
	{ "Guardian Angel",                "(+1%%) increased power of Bless for every 20 ",
	  6694,                            "total Braveness." },
	//"         '         '         ", "         '         '         '         '      " ////////
	{ "False Expanse",                 "(+1) to Area of Effect.",
	  6649,                            "" },
	{ "False Feast",                   "(+15) to Maximum Hitpoints.",
	  6663,                            "" },
	{ "False Sharkskin",               "(+3) to Thorns.",
	  6613,                            "" },
	//"         '         '         ", "         '         '         '         '      "
	{ "Martial Artist",                "(+5%%) to Base Critical Hit Chance while ",
	  6698,                            "unarmed." },
	{ "False Pride",                   "(+25%%) increased total Area of Effect.",
	  6653,                            "" },
	{ "False Greed",                   "Enemies affected by your debuffs grant (+1%%) ",
	  6700,                            "of damage taken as Hitpoints to the attacker." },
	//"         '         '         ", "         '         '         '         '      "
	{ "False Lust",                    "(+25%%) increased effect of recovery from all ",
	  6706,                            "sources." },
	{ "False Gluttony",                "(+10%%) increased Maximum Hitpoints. (+1%%) ",
	  6671,                            "of damage dealt is leeched as Hitpoints." },
	{ "False Wrath",                   "(+1%%) increased effect of bonus from Pact ",
	  6705,                            "for every 100 uncapped Maximum Hitpoints." },
	//"         '         '         ", "         '         '         '         '      "
	{ "False Sloth",                   "(+10%%) of Thorns is triggered upon parrying.",
	  6618,                            "" },
	{ "False Envy",                    "(+10%%) increased total Thorns score.",
	  6617,                            "" },
	{ "Wild Child",                    "(+15%%) increased effect of stats granted by ",
	  6630,                            "Claws." },
	//"         '         '         ", "         '         '         '         '      " ////////
	{ "Shackle",                       "(+1%%) increased power of Slow for every 20 ",
	  6646,                            "total Willpower." },
	{ "Famine",                        "(+1%%) increased power of Curse for every 20 ",
	  6657,                            "total Intuition." },
	{ "Burden",                        "(+1%%) increased power of Weaken for every 20 ",
	  6669,                            "total Agility." },
	//"         '         '         ", "         '         '         '         '      "
	{ "Conquest",                      "(+1%%) increased power of Cleave for every 20 ",
	  6616,                            "total Strength." },
	{ "Torment",                       "Damage dealt by Damage Over Time is dealt ",
	  6652,                            "(+15%%) faster." },
	{ "Courage",                       "(+25%%) reduced damage taken from enemy ",
	  6690,                            "Thorns." },
	//"         '         '         ", "         '         '         '         '      "
	{ "Ambidexterity",                 "(+10%%) of total Weapon Value is granted as ",
	  6664,                            "additional Cooldown Rate." },
	{ "Razor Shell",                   "(+10%%) of total Armor Value is granted as ",
	  6618,                            "additional Thorns." },
	{ "Redemption",                    "(+15%%) of total Weapon and Armor Value is ",
	  6609,                            "granted to your companions." },
	//"         '         '         ", "         '         '         '         '      "
	{ "Steadfast",                     "Magic Shields and Shells affecting you take ",
	  6621,                            "(+15%%) reduced damage from enemies." },
	{ "Censure",                       "(+5%%) of damage taken from hits is staggered ",
	  6622,                            "over 5 seconds." },
	{ "Pestilence",                    "Your skills and spells ignore (+10%%) of enemy ",
	  6606,                            "resistance." }
};

struct metaStat metaStats[103] = {
	//
	//   Topmost standard stats
	//                                             "         '         '         '         '         '         '         '         '         '         '         '         "
	{ -1, 1, 1, "Cooldown Duration",    "x",       "The multiplier that cooldown from skills is applied with, the lower the better. Affected by Cooldown Rate." },
	{ -1, 0, 4, "Spell Aptitude",       "",        "How powerful a spell you can receive from any source. Determined by Willpower, Intuition, and class Spell Modifier." },
	{ -1, 0, 4, "Spell Bonus",          "",        "Flat added bonus to the power of spells you cast." },
	{ -1, 1, 4, "Spell Multiplier",     "x",       "A multiplier which effects the strength of spells you cast. Determined by your character class." },
	{ -1, 1, 6, "Base Action Speed",    "",        "The base speed at which ALL actions are performed. Determined by Agility and Strength." },
	{ -1, 0, 7, "Hit Score",            "",        "The value used to determine the rate of hitting enemies in melee combat. Granted by your weapon skill." },
	{ -1, 0, 7, "Parry Score",          "",        "The value used to determine the rate of avoiding damage from enemies. Granted by your weapon skill." },
	//
	//   Offense Stats
	//                                             "         '         '         '         '         '         '         '         '         '         '         '         "
	{ -1, 0, 9, "  Passive Stats:",     "",        "" }, //  7
	{ -1, 1, 1, "Damage Multiplier",    "%",       "The final multiplier for all damage you deal." },
	{ -1, 1, 7, "Est. Melee DPS",       "",        "The average of your damage per hit, times your attack speed. Does not account for bonus damage from your Hit Score." },
	{ -1, 0, 5, "Est. Melee Hit Dmg",   "",        "Ranges from the minimum damage a melee hit could do, to the maximum damage a melee hit could do." },
	{ -1, 0, 5, "Critical Multiplier",  "%",       "The damage multiplier upon dealing a successful critical hit." },
	{ -1, 1, 5, "Critical Chance",      "%",       "The chance, out of 100, that you will inflict a melee critical hit." },
	{ -1, 0, 5, "Melee Ceiling Damage", "",        "The highest possible damage a melee hit may deal. Affected by increases to Top Damage and your critical hit scores." },
	{ -1, 0, 5, "Melee  Floor  Damage", "",        "Melee Floor Damage is the lowest possible damage a melee hit may deal. Determined by 1/4 of your Weapon Value." },
	{ -1, 1, 6, "Attack Speed",         "",        "The speed at which melee attacks are performed. Increased by Agility." },
	{ -1, 1, 6, "Cast Speed",           "",        "The speed at which casting and action animations occur per second. Increased by Willpower." },
	{ -1, 0, 1, "Thorns Score",         "",        "Damage dealt to attackers when you are hit (even if you take no damage). Does not damage attackers if they miss." },
	{ -1, 1, 4, "Mana Cost Multiplier", "%",       "The multiplier of mana for spells, determined by your Concentrate skill." },
	{ -1, 0, 1, "Total AoE Bonus",      "Tiles",   "Flat increase to area-of-effect skills." },
	{ -2, 0, 0, "", "", "" }, // blank
	{ -2, 0, 0, "", "", "" }, // blank
	{ -2, 0, 0, "", "", "" }, // blank
	{ -1, 0, 9, "  Active Stats:",      "",        "" }, // 23
	{ 40, 0, 5, "Cleave Hit Damage",    "",        "Damage dealt by your Cleave skill, before enemy defenses." },
	{ 40, 1, 5, "Cleave Bleed Degen",   "/s",      "Effective damage over time dealt by Bleeding caused by Cleave, before enemy defenses." },
	{ 40, 1, 5, "Cleave Cooldown",      "Seconds", "Skill exhaustion duration expected upon using your Cleave skill." },
	{ 49, 0, 1, "Leap Hit Damage",      "",        "Damage dealt by your Leap skill if your target is at or near maximum hitpoints." },
	{ 49, 0, 1, "Leap # of Repeats",    "Repeats", "Damage dealt by your Leap skill, before enemy defenses." },
	{ 49, 1, 1, "Leap Cooldown",        "Seconds", "Skill exhaustion duration expected upon using your Leap skill." },
	{ 22, 1, 5, "Pact Dmg Bonus",       "%",       "Effective multiplier to damage granted while under the effect of your Pact skill." },
	{ 22, 0, 5, "Pact Health Reserve",  "%",       "The amount of health, as a percent, reserved while Pact is active." },
	{ 24, 0, 4, "Blast Hit Damage",     "",        "Damage dealt by your Blast spell, before enemy defenses." },
	{ 24, 1, 4, "Blast Cooldown",       "Seconds", "Skill exhaustion duration expected upon using your Blast spell." },
	{ 15, 0, 1, "Lethargy Effect",      "I/R Pen", "Effective penetration of target Immunity and Resistance when casting debuffs." },
	{ 42, 1, 4, "Poison Degen",         "/s",      "Effective damage over time dealt by your Poison spell, before reduction from target Immunity." },
	{ 42, 1, 4, "Poison Cooldown",      "Seconds", "Skill exhaustion duration expected upon using your Poison spell." },
	{ 43, 0, 1, "Pulse Hit Damage",     "/tick",   "Damage dealt by the Pulse spell to surrounding enemies when pulsing, before enemy defenses." },
	{ 43, 0, 1, "Pulse Count",          "",        "Number of pulses expected during the duration of your Pulse spell, determined by Cooldown Rate." },
	{ 43, 1, 1, "Pulse Cooldown",       "Seconds", "Skill exhaustion duration expected upon using your Pulse spell." },
	{  7, 1, 6, "Zephyr Hit Damage",    "",        "Damage granted by your Zephyr spell, before enemy defenses. This occurs one second after a successful hit." },
	{ 44, 1, 4, "Immolate Degen",       "/s",      "Effective damage over time dealt to nearby enemies while affected by Immolate, before enemy defenses." },
	{ -2, 0, 0, "", "", "" }, // blank
	{ 27, 0, 1, "Ghost Comp Potency",   "",        "Effective power of your Ghost Companion, granted by your Ghost Companion spell." },
	{ 27, 1, 1, "Ghost Comp Cooldown",  "Seconds", "Skill exhaustion duration expected upon using your Ghost Companion spell." },
	{ 46, 0, 4, "Shadow Copy Potency",  "",        "Effective power of your Shadow Copy, granted by your Shadow Copy spell." },
	{ 46, 1, 4, "Shadow Copy Duration", "Seconds", "Effective duration of your Shadow Copy, granted by your Shadow Copy spell." },
	{ 46, 1, 4, "Shadow Copy Cooldown", "Seconds", "Skill exhaustion duration expected upon using your Shadow Copy spell." },
	//
	//   Defense Stats
	//                                             "         '         '         '         '         '         '         '         '         '         '         '         "
	{ -1, 0, 9, "  Passive Stats:",     "",        "" }, // 48
	{ -1, 1, 1, "Damage Reduction",     "%",       "Damage Reduction is the final multiplier for all damage you take." },
	{ -1, 0, 7, "Effective Hitpoints",  "",        "Your effective total hitpoints. Determined by maximum health and damage reduction and conversions." },
	{ -1, 1, 5, "Health Regen Rate",    "/s",      "Rate at which health is regenerated per second. This is improved by the Regenerate skill." },
	{ -1, 1, 6, "Endurance Regen Rate", "/s",      "Rate at which endurance is regenerated per second. This is improved by the Rest skill." },
	{ -1, 1, 4, "Mana Regen Rate",      "/s",      "Rate at which mana is regenerated per second. This is improved by the Meditate skill." },
	{ -1, 0, 1, "Effective Immunity",   "",        "Estimated Immunity score. This displays your 'true' Immunity value after special effects." },
	{ -1, 0, 1, "Effective Resistance", "",        "Estimated Resistance score. This displays your 'true' Resistance value after special effects." },
	{ -1, 1, 6, "Movement Speed",       "",        "The speed at which your character runs around Astonia." },
	{ -1, 1, 6, "Cast Speed",           "",        "The speed at which casting and action animations occur per second. Increased by Willpower." },
	{ -1, 0, 1, "Thorns Score",         "",        "Damage dealt to attackers when you are hit (even if you take no damage). Does not damage attackers if they miss." },
	{ -1, 1, 4, "Mana Cost Multiplier", "%",       "The multiplier of mana for spells, determined by your Economize skill." },
	{ -1, 0, 4, "Total AoE Bonus",      "Tiles",   "Flat increase to area-of-effect skills." },
	{ -1, 0, 4, "Buffing Apt Bonus",    "",        "Bonus granted to target allies when casting friendly spells. Granted by Willpower." },
	{ -1, 1, 1, "Underwater Degen",     "/s",      "Rate at which health is lost while underwater. This can be reduced by the Metabolism skill." },
	{ -2, 0, 0, "", "", "" }, // blank
	{ -1, 0, 9, "  Active Stats:",      "",        "" }, // 64
	{ 21, 0, 1, "Bless Effect",         "Attribs", "Estimated increase to attributes granted by your Bless spell." },
	{ 18, 0, 1, "Enhance Effect",       "WV",      "Effective increase to Weapon Value granted by your Enhance spell." },
	{ 17, 0, 1, "Protect Effect",       "AV",      "Effective increase to Armor Value granted by your Protect spell." },
	{ 11, 0, 4, "M.Shield Effect",      "AV",      "Effective increase to Armor Value granted by your Magic Shield spell. Decreases as you take damage." },
	{ 11, 1, 4, "M.Shield Duration",    "Seconds", "Estimated duration of your Magic Shell, not including reductions from taking damage." },
	{ 47, 0, 6, "Haste Effect",         "Speed",   "Estimated increase to Speed granted by your Haste spell." },
	{ 22, 1, 5, "Pact Dmg Taken",       "%",       "Effective multiplier to incoming damage granted while under the effect of your Pact skill." },
	{ 22, 0, 5, "Pact Health Reserve",  "%",       "The amount of health, as a percent, reserved while Pact is active." },
	{ 26, 0, 1, "Heal Effect",          "",        "Effective flat healing expected when casting your heal spell." },
	{ 37, 0, 5, "Blind Effect",         "",        "Effective reduction of target Hit and Parry Scores when using your Blind skill, before enemy defenses." },
	{ 37, 1, 5, "Blind Cooldown",       "Seconds", "Skill exhaustion duration expected upon using your Blind skill." },
	{ 35, 0, 1, "Warcry Effect",        "Attribs", "Effective reduction of target attributes when using your Warcry skill, before enemy defenses." },
	{ 35, 1, 1, "Warcry Cooldown",      "Seconds", "Skill exhaustion duration expected upon using your Warcry skill." },
	{ 41, 0, 5, "Weaken Effect",        "WV",      "Effective reduction of target Weapon Value when using your Weaken skill, before enemy defenses." },
	{ 41, 1, 5, "Weaken Cooldown",      "Seconds", "Skill exhaustion duration expected upon using your Weaken skill." },
	{ 20, 0, 2, "Curse Effect",         "Attribs", "Effective reduction of target attributes when casting your Curse spell, before enemy defenses." },
	{ 20, 1, 2, "Curse Cooldown",       "Seconds", "Skill exhaustion duration expected upon using your Curse spell." },
	{ 19, 0, 4, "Slow Effect",          "Speed",   "Effective reduction of target action speed when casting your Slow spell, before enemy defenses." },
	{ 19, 1, 4, "Slow Cooldown",        "Seconds", "Skill exhaustion duration expected upon using your Slow spell." },
	{ 27, 0, 1, "Ghost Comp Potency",   "",        "Effective power of your Ghost Companion, granted by your Ghost Companion spell." },
	{ 27, 1, 1, "Ghost Comp Cooldown",  "Seconds", "Skill exhaustion duration expected upon using your Ghost Companion spell." },
	{ 46, 0, 4, "Shadow Copy Potency",  "",        "Effective power of your Shadow Copy, granted by your Shadow Copy spell." },
	{ 46, 1, 4, "Shadow Copy Duration", "Seconds", "Effective duration of your Shadow Copy, granted by your Shadow Copy spell." },
	{ 46, 1, 4, "Shadow Copy Cooldown", "Seconds", "Skill exhaustion duration expected upon using your Shadow Copy spell." },
	//
	//   Alternative values
	//                                             "         '         '         '         '         '         '         '         '         '         '         '         "
	{ -1, 0, 5, "Skill Bonus",          "",        "Flat added bonus to the power of skills you use." },
	{ 42, 1, 4, "Venom Degen",          "/s",      "Effective damage over time dealt by your Venom spell, before enemy defenses. This can stack up to three times." },
	{ 42, 1, 4, "Venom Cooldown",       "Seconds", "Skill exhaustion duration expected upon using your Venom spell." },
	{ 43, 1, 1, "Pulse Hit Heal",       "/tick",   "Healing caused by the Pulse spell to surrounding allies when pulsing." },
	{ 11, 0, 4, "M.Shell Effect",       "Res&Imm", "Effective increase to Resistance and Immunity Scores granted by your Magic Shell spell." },
	{ 11, 1, 4, "M.Shell Duration",     "Seconds", "Estimated duration of your Magic Shell, not including reductions from taking or avoid debuffs." },
	{ 26, 1, 1, "Regen Effect",         "/s",      "Effective healing over time expected when casting your Regen spell." },
	{ 37, 1, 5, "Douse Effect",         "%",       "Effective reduction of target Spell Modifier when using your Douse skill, before enemy defenses." },
	{ 37, 1, 5, "Douse Cooldown",       "Seconds", "Skill exhaustion duration expected upon using your Douse skill." },
	{ 35, 0, 1, "Rally Effect",         "Attribs", "Effective bonus to hit and parry score granted to allies when using your Rally skill." },
	{ 35, 1, 1, "Rally Cooldown",       "Seconds", "Skill exhaustion duration expected upon using your Rally skill." },
	{ 41, 0, 5, "Crush Effect",         "AV",      "Effective reduction of target Armor Value when using your Crush skill, before enemy defenses." },
	{ 41, 1, 5, "Crush Cooldown",       "Seconds", "Skill exhaustion duration expected upon using your Crush skill." },
	{ 22, 0, 5, "Pact Mana Reserve",    "%",       "The amount of mana, as a percent, reserved while Pact is active." }
};


/*************************************************************************

   This file is part of 'Mercenaries of Astonia v2'
   Copyright (c) 1997-2001 Daniel Brockhaus (joker@astonia.com)
   All rights reserved.

 **************************************************************************/

#include "server.h"

//		AT_BRV	AT_WIL	AT_INT	AT_AGL	AT_STR

struct s_skilltab skilltab[MAXSKILL+5] = {
//	{ //, '/', 	"////////////////",		"////////////////////////////////////////////////////////////////////////////////",
	{  0, 'C', 	"Hand to Hand", 		"Passive ability to hit and parry while unarmed.", 
				"", "",
				{ AT_BRV, AT_AGL, AT_STR }},
				
	{  1, 'G', 	"Precision", 			"Passively improves your ability to inflict critical hits.", 
				"", "",
				{ AT_BRV, AT_BRV, AT_INT }},
				
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
				{ AT_BRV, AT_AGL, AT_STR }},
				
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
				
	{ 12, 'C', 	"Tactics", 				"Passive ability to hit and parry with any weapon. Loses effectiveness while not at full mana.", 
				"Tactics (Inverse)", 	"Passive ability to hit and parry with any weapon. Only effective while low on mana.",
				{ AT_BRV, AT_WIL, AT_INT }},
				
	{ 13, 'E', 	"Repair", 				"Use (Skill): You will try to repair the item under your cursor.", 
				"", "",
				{ AT_INT, AT_AGL, AT_STR }},
				
	{ 14, 'G', 	"Finesse", 				"Passive ability which grants more global damage the healthier you are.", 
				"Finesse (Inverse)", 	"Passive ability which grants more global damage while near death.",
				{ AT_BRV, AT_BRV, AT_AGL }},
				
	{ 15, 'F', 	"Lethargy", 			"Use (Spell): Applies a buff to yourself, letting you pierce enemy Resistance and Immunity at the cost of mana over time.", 
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
				
	{ 22, 'E', 	"Rage", 				"Use (Skill): Applies a buff to yourself, granting additional Top Damage and damage-over-time at the cost of health per second.", 
				"Calm", 				"Use (Skill): Applies a buff to yourself, granting resistance to enemy Top Damage and damage-over-time at the cost of mana per second.",
				{ AT_BRV, AT_INT, AT_STR }},
				
	{ 23, 'G', 	"Resistance", 			"Passive ability to avoid enemy negative spells.", 
				"", "",
				{ AT_BRV, AT_WIL, AT_STR }},
				
	{ 24, 'F', 	"Blast", 				"Use (Spell): Damages your target and surrounding enemies.", 
				"Blast (Scorch)", 		"Use (Spell): Damages your target and surrounding enemies. This also applies a debuff, increasing the damage dealt to the target.", 
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
				
	{ 31, 'G', 	"Aria", 				"Passively grants you and nearby allies a buff to cooldown rate, and debuffs nearby enemy cooldown rate. Has a base radius of 5 tiles.", 
				"", "",
				{ AT_BRV, AT_AGL, AT_AGL }},
				
	{ 32, 'G', 	"Immunity", 			"Passive ability to reduce the strength of enemy negative spells.", 
				"", "",
				{ AT_BRV, AT_AGL, AT_STR }},
				
	{ 33, 'G', 	"Surround Hit", 		"Passive ability to deal a portion of melee hit damage to all foes around you.", 
				"", "",
				{ AT_AGL, AT_STR, AT_STR }},
				
	{ 34, 'G', 	"Economize", 			"Passive ability to reduce the mana cost of spells and abilities. Additionally grants better prices while buying or selling.", 
				"", "",
				{ AT_WIL, AT_WIL, AT_WIL }},
				
	{ 35, 'E', 	"Warcry", 				"Use (Skill): Shout to stun and strike fear into all nearby enemies. Has a base radius of 6 tiles.", 
				"Warcry (Rally)", 		"Use (Skill): Shout to rally your allies and improve hit and parry score. Has a base radius of 6 tiles.",
				{ AT_BRV, AT_STR, AT_STR }},
				
	{ 36, 'D', 	"Dual Wield", 			"Passive ability to hit while using a dual-sword.", 
				"", "",
				{ AT_BRV, AT_AGL, AT_STR }},
				
	{ 37, 'E', 	"Blind", 				"Use (Skill): Applies a debuff to nearby enemies, reducing their hit and parry rates. Has a base radius of 4 tiles.", 
				"Blind (Douse)", 		"Use (Skill): Applies a debuff to nearby enemies, reducing their stealth and spell modifier. Has a base radius of 4 tiles.",
				{ AT_BRV, AT_INT, AT_AGL }},
				
	{ 38, 'G', 	"Gear Mastery", 		"Passive ability to improve weapon and armor values granted by your equipment.", 
				"", "",
				{ AT_BRV, AT_AGL, AT_STR }},
				
	{ 39, 'G', 	"Safeguard", 			"Passive ability to reduce damage taken.", 
				"", "",
				{ AT_BRV, AT_STR, AT_STR }},
				
	{ 40, 'E', 	"Cleave", 				"Use (Skill): Strike your foe and deal damage to surrounding enemies. This also applies a debuff, causing them to take damage over time.", 
				"Cleave (Aggravate)",	"Use (Skill): Strike your foe and deal damage to surrounding enemies. This also applies a debuff, causing them to take additional damage.",
				{ AT_AGL, AT_STR, AT_STR }},
				
	{ 41, 'E', 	"Weaken", 				"Use (Skill): Applies a debuff to your foe and surrounding enemies, reducing their weapon value.", 
				"Weaken (Crush)", 		"Use (Skill): Applies a debuff to your foe and surrounding enemies, reducing their armor value.",
				{ AT_BRV, AT_AGL, AT_AGL }},
				
	{ 42, 'F', 	"Poison", 				"Use (Spell): Applies a debuff to your target and surrounding enemies, causing them to take damage over time.", 
				"Poison (Venom)", 		"Use (Spell): Applies a stacking debuff to your target and surrounding enemies, reducing immunity and causing damage over time. Stacks up to 3 times.",
				{ AT_BRV, AT_INT, AT_INT }},
				
	{ 43, 'F', 	"Pulse", 				"Use (Spell): Applies a buff to yourself, causing a repeating burst of energy to damage nearby foes and inflict shock, reducing their damage dealt and increasing their damage taken. Has a base radius of 3 tiles.", 
				"Pulse (Charge)", 		"Use (Spell): Applies a buff to yourself, causing a repeating burst of energy to heal nearby allies and inflict charge, reducing their damage taken and increasing their damage dealt. Has a base radius of 3 tiles.",
				{ AT_BRV, AT_INT, AT_INT }},
				
	{ 44, 'G', 	"Proximity", 			"Passively improves the area-of-effect of skills which already have an area-of-effect.",
				"Proximity",			"Passively improves the area-of-effect of your Poison, Curse, and Slow spells.", // Sorcerer
				{ AT_BRV, AT_WIL, AT_INT }},
				
	{ 45, 'G', 	"Companion Mastery", 	"Passively increases the limit and number of abilities known by your ghost companion.", 
				"Proximity",			"Passively improves the area-of-effect of your Blast and Pulse spells.", // Arch-Harakim
				{ AT_BRV, AT_WIL, AT_WIL }},
				
	{ 46, 'F', 	"Shadow Copy", 			"Use (Spell): Summons a temporary doppelganger to attack your enemies.", 
				"Proximity",			"Passively improves the area-of-effect of your Aria and Weaken skills.", // Braver
				{ AT_BRV, AT_WIL, AT_WIL }},
				
	{ 47, 'F', 	"Haste", 				"Use (Spell): Applies a buff to yourself, increasing your action speed.", 
				"", "",
				{ AT_BRV, AT_WIL, AT_AGL }},
				
	{ 48, 'E', 	"Taunt",				"Use (Skill): Applies a debuff to your target and surrounding enemies, forcing them to attack you. This also applies a buff to yourself, granting damage resistance.", 
				"", "",
				{ AT_BRV, AT_STR, AT_STR }},
				
	{ 49, 'E', 	"Leap", 				"Use (Skill): Strike your foe and leap to a random nearby enemy, dealing critical damage to enemies at full life. Higher cooldown rate lets this skill repeat additional times.",
				"Leap (Critical)", 		"Use (Skill): Strike your foe and leap to your target, dealing critical damage and stunning enemies it hits.",
				{ AT_BRV, AT_AGL, AT_AGL }},
				
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
	{ 54, 'H', 	"Shift", 				"Use (Skill): Change form from that of a Ratling to that of a Greenling, and vice versa. Has its own unique cooldown timer.", 
				"", "",
				{ 0, 0, 0 }}
};

struct sk_tree sk_tree[10][12]={
	{	// Seyan'du
		{ "Sharpness",                     "+2 to Weapon Value.",
		  6601,                            "" },
		{ "Expertise",                     "+2 to All Attributes.",
		  6602,                            "" },
		{ "Toughness",                     "+2 to Armor Value.",
		  6603,                            "" },
		{ "Absolution",                    "0.5%% more damage dealt for each buff ",
		  6604,                            "or debuff on you." },
		{ "Vanquisher",                    "6%% more total Weapon Value.",
		  6605,                            "" },
		{ "Scorn",                         "Your debuffs ignore 20%% of enemy ",
		  6606,                            "Resistance." },
		{ "Determination",                 "Gain 1 additional Hit and Parry for ",
		  6607,                            "every 100 total Attributes." },
		{ "Jack of All Trades",            "4%% increased total Attributes.",
		  6608,                            "" },
		{ "Redemption",                    "Companions have Hit and Parry scores ",
		  6609,                            "equal to yours, and learn Regen." },
		{ "Enigmatic",                     "20%% reduced effect of debuffs on you.",
		  6610,                            "" },
		{ "Steelskin",                     "6%% more total Armor Value.",
		  6611,                            "" },
		{ "Penance",                       "0.5%% less damage taken for each buff ",
		  6612,                            "or debuff on you." }
	}, // "         '         '         ", "         '         '         '         '         "
	{	// Arch Templar
		{ "Spiked",                        "+5 to Thorns.",
		  6613,                            "" },
		{ "Might",                         "+4 to Strength.",
		  6614,                            "" },
		{ "Bulwark",                       "+3 to Armor Value.",
		  6615,                            "" },
		{ "Serrated Blades",               "Cleave deals additional damage based on ",
		  6616,                            "your total Thorns." },
		{ "Sharkskin",                     "20%% more total Thorns.",
		  6617,                            "" },
		{ "Retaliation",                   "Your Thorns can now trigger on a ",
		  6618,                            "parried hit with 10%% power." },
		{ "Overlord",                      "0.5%% more effect of Warcry and Rally ",
		  6619,                            "for every 10 total Strength." },
		{ "Overwhelming Strength",         "3%% increased total Strength. ",
		  6620,                            "+10 to Strength Limit." },
		{ "Censure",                       "Taunt reduces enemy Hit score by 5%% ",
		  6621,                            "for its duration." },
		{ "Bastion",                       "20%% of total Resistance is granted as ",
		  6622,                            "extra Immunity." },
		{ "Unbreakable",                   "9%% more total Armor Value.",
		  6623,                            "" },
		{ "Rampart",                       "50%% more Parry granted by your Shield ",
		  6624,                            "skill." }
	}, // "         '         '         ", "         '         '         '         '         "
	{	// Skald
		{ "Muscle",                        "+3 to Weapon Value.",
		  6625,                            "" },
		{ "Dexterity",                     "+4 to Agility.",
		  6626,                            "" },
		{ "Persistance",                   "+20 Endurance.",
		  6627,                            "" },
		{ "Nocturne",                      "20%% increased effect of Aria.",
		  6628,                            "" },
		{ "Valor",                         "9%% more total Weapon Value.",
		  6629,                            "" },
		{ "Enthusiasm",                    "Your Aria additionally grants nearby ",
		  6630,                            "allies 10%% of your Weapon Value." },
		{ "Slaying",                       "+3%% Critical Multiplier for every 10 ",
		  6631,                            "total Agility." },
		{ "Overwhelming Agility",          "3%% increased total Agility. ",
		  6632,                            "+10 to Agility Limit." },
		{ "Acumen",                        "All melee skills use the attributes ",
		  6633,                            "(STR+BRV/2) + Agility + Agility." },
		{ "Impact",                        "Weaken and Crush also reduce enemy damage ",
		  6634,                            "multiplier and damage reduction." },
		{ "Perseverance",                  "20%% more total Endurance.",
		  6635,                            "" },
		{ "Tenacity",                      "20%% of damage taken is dealt to your ",
		  6636,                            "Endurance instead." }
	}, // "         '         '         ", "         '         '         '         '         "
	{	// Warrior
		{ "Rapidity",                      "+5 to Attack Speed.",
		  6637,                            "" },
		{ "Ruffian",                       "+3 to Strength & +3 to Agility.",
		  6638,                            "" },
		{ "Passion",                       "+5 to Spell Aptitude.",
		  6639,                            "" },
		{ "Alacrity",                      "Zephyr deals 20%% more damage.",
		  6640,                            "" },
		{ "Swiftness",                     "10%% more total Attack Speed.",
		  6641,                            "" },
		{ "Intensity",                     "+2 to Spell Modifier.",
		  6642,                            "" },
		{ "Antagonizer",                   "0.5%% more effect of Blind and Douse ",
		  6643,                            "for every 10 total Agility." },
		{ "Harrier",                       "3%% increased total Agility and ",
		  6644,                            "Strength." },
		{ "Butchery",                      "0.5%% more effect of Cleave for every ",
		  6645,                            "10 total Strength." },
		{ "Champion",                      "Enemies beside and behind you no longer ",
		  6646,                            "gain a bonus to hitting you." },
		{ "Zealotry",                      "20%% more total Spell Aptitude.",
		  6647,                            "" },
		{ "Fervor",                        "20%% of Spell Aptitude is used to reduce ",
		  6648,                            "the strength of incoming enemy spells." }
	}, // "         '         '         ", "         '         '         '         '         "
	{	// Sorcerer
		{ "Expansiveness",                 "+1 to Area of Effect.",
		  6649,                            "" },
		{ "Potency",                       "+3 to Willpower & +3 to Intuition.",
		  6650,                            "" },
		{ "Quickstep",                     "+5 to Movement Speed.",
		  6651,                            "" },
		{ "Tormenter",                     "Poison deals damage 20%% faster.",
		  6652,                            "" },
		{ "Grandiosity",                   "20%% more total Area of Effect.",
		  6653,                            "" },
		{ "Brilliance",                    "+1 to Spell Modifier.",
		  6654,                            "" },
		{ "Coordination",                  "0.5%% more effect of Lethargy for every ",
		  6655,                            "10 total Willpower." },
		{ "Pragmatic",                     "3%% increased total Willpower and ",
		  6656,                            "Intuition." },
		{ "Hex Master",                    "0.5%% more effect of Curse and Slow ",
		  6657,                            "for every 10 total Intuition." },
		{ "Nimble",                        "You no longer have a parry penalty if ",
		  6658,                            "hit while not fighting." },
		{ "Fleet-footed",                  "20%% more total Movement Speed.",
		  6659,                            "" },
		{ "Acceleration",                  "Haste grants 20%% more move speed to ",
		  6660,                            "you, and it can now be cast on allies." }
	}, // "         '         '         ", "         '         '         '         '         "
	{	// Summoner
		{ "Nimbleness",                    "+5 to Cast Speed.",
		  6661,                            "" },
		{ "Wisdom",                        "+4 to Willpower.",
		  6662,                            "" },
		{ "Vitality",                      "+20 Hitpoints.",
		  6663,                            "" },
		{ "Tactician",                     "Increases and multipliers to Cast Speed ",
		  6664,                            "also affect Attack Speed." },
		{ "Spellslinger",                  "10%% more total Cast Speed.",
		  6665,                            "" },
		{ "Harpooner",                     "20%% more Hit and Parry score while ",
		  6666,                            "using a Spear." },
		{ "Mysticism",                     "All spell skills use the attributes ",
		  6667,                            "(BRV+INT)/2 + Willpower + Willpower." },
		{ "Overwhelming Willpower",        "3%% increased total Willpower. ",
		  6668,                            "+10 to Willpower Limit." },
		{ "Shaper",                        "0.5%% more effect of Shadow Copy for ",
		  6669,                            "every 10 total Willpower." },
		{ "Diviner",                       "Ghost Companions inherit your Dispel ",
		  6670,                            "and the effects of your tarot cards." },
		{ "Constitution",                  "20%% more total Hitpoints.",
		  6671,                            "" },
		{ "Protector",                     "Magic Shield and Magic Shell are also ",
		  6672,                            "cast on your active companions." }
	}, // "         '         '         ", "         '         '         '         '         "
	{	// Arch Harakim
		{ "Composure",                     "+5 to Cooldown Rate.",
		  6673,                            "" },
		{ "Intellect",                     "+4 to Intuition.",
		  6674,                            "" },
		{ "Wellspring",                    "+20 Mana.",
		  6675,                            "" },
		{ "Destroyer",                     "Blast has its base cooldown reduced ",
		  6676,                            "for each enemy hit by it." },
		{ "Serenity",                      "5%% more total Cooldown Rate.",
		  6677,                            "" },
		{ "Strategist",                    "You suffer no cooldown if a spell is ",
		  6678,                            "suppressed." },
		{ "Psychosis",                     "0.5%% more effect of Pulse for every 10 ",
		  6679,                            "total Intuition." },
		{ "Overwhelming Intuition",        "3%% increased total Intuition.",
		  6680,                            "+10 to Intuition Limit." },
		{ "Wizardry",                      "All spell skills use the attributes ",
		  6681,                            "(BRV+WIL)/2 + Intuition + Intuition." },
		{ "Flow",                          "25%% of overcapped Mana is granted as ",
		  6682,                            "additional Hitpoints." },
		{ "Perpetuity",                    "20%% more total Mana.",
		  6683,                            "" },
		{ "Resourcefulness",               "20%% of damage taken is dealt to your ",
		  6684,                            "Mana instead." }
	}, // "         '         '         ", "         '         '         '         '         "
	{	// Braver
		{ "Accuracy",                      "+3 to Hit Score.",
		  6685,                            "" },
		{ "Boldness",                      "+4 to Braveness.",
		  6686,                            "" },
		{ "Avoidance",                     "+3 to Parry Score.",
		  6687,                            "" },
		{ "Assassination",                 "20%% increased effect of Precision.",
		  6688,                            "" },
		{ "Rigor",                         "4%% more total Hit Score.",
		  6689,                            "" },
		{ "Deftness",                      "50%% reduced damage taken from ",
		  6690,                            "triggering enemy Thorns." },
		{ "Perfectionism",                 "0.5%% more effect of Finesse for every 10 ",
		  6691,                            "total Braveness." },
		{ "Overwhelming Braveness",        "3%% increased total Braveness.",
		  6692,                            "+10 to Braveness Limit." },
		{ "Virtuosity",                    "All weapon skills use the attributes ",
		  6693,                            "(AGL+STR)/2 + Braveness + Braveness." },
		{ "Resilience",                    "40%% less effect of Healing Sickness ",
		  6694,                            "on you." },
		{ "Flexibility",                   "4%% more total Parry Score.",
		  6695,                            "" },
		{ "Litheness",                     "50%% reduced extra damage taken from ",
		  6696,                            "enemy Critical Hits." }
	}, // "         '         '         ", "         '         '         '         '         "
	{	// Lycanthrope
		{ "Maiming",                       "+5 to Top Damage.",
		  6697,                            "" },
		{ "Feast",                         "+10 Hitpoints, Endurance, and Mana.",
		  6698,                            "" },
		{ "Insight",                       "+2 to Spell Modifier.",
		  6699,                            "" },
		{ "Lust",                          "Ferocity grants +1%% base crit chance ",
		  6700,                            "per empty gear slot." },
		{ "Ravager",                       "20%% more total Top Damage.",
		  6701,                            "" },
		{ "Greed",                         "Your Top Damage is rolled an additional ",
		  6702,                            "time, using the higher result." },
		{ "Wrath",                         "0.5%% more effect of Rage & Calm per 50 ",
		  6703,                            "missing Hitpoints, Endurance, and Mana." },
		{ "Sloth",                         "10%% more Hitpoints, Endurance, and Mana.",
		  6704,                            "" },
		{ "Gluttony",                      "8%% of damage dealt is restored as ",
		  6705,                            "Hitpoints, Endurance, and Mana." },
		{ "Pride",                         "Your debuffs ignore 20%% of enemy ",
		  6706,                            "Immunity." },
		{ "Madness",                       "+3 to Spell Modifier.",
		  6707,                            "" },
		{ "Envy",                          "Ferocity grants +1 Spell Modifier per ",
		  6708,                            "empty gear slot." }
	}, // "         '         '         ", "         '         '         '         '         "
	{	// Contract
		{ "Reward",                        "200%% increased chance of finding Rainbow ",
		  6709,                            "Belts in contracts signed by you." },
		{ "Challenge",                     "+1 to rank of contracts signed by you.",
		  6710,                            "" },
		{ "Army",                          "+1 enemy per spawn in contracts signed by ",
		  6711,                            "you." },
		{ "Hope",                          "20%% increased effect of rewards from ",
		  6712,                            "green shrines in contracts signed by you." },
		{ "Opalescence",                   "200%% increased chance of finding Rainbow ",
		  6713,                            "Belts in contracts signed by you." },
		{ "Scholar",                       "20%% more clear experience from ",
		  6714,                            "contracts signed by you." },
		{ "Fate",                          "20%% more effect of blue shrines in ",
		  6715,                            "contracts signed by you." },
		{ "Hubris",                        "+1 to rank of contracts signed by you.",
		  6716,                            "" },
		{ "Binding",                       "Contracts signed by you always grant ",
		  6717,                            "tier 3 effects from quills." },
		{ "Destiny",                       "Red shrines produce harder enemies with ",
		  6718,                            "more rewards in contracts signed by you." },
		{ "Swarm",                         "+1 enemy per spawn in contracts signed by ",
		  6719,                            "you." },
		{ "Incentive",                     "Enemies grant an additional 5%% of exp as ",
		  6720,                            "Contract Pts in contracts signed by you." }
	}  // "         '         '         ", "         '         '         '         '         "
};

struct sk_tree sk_corrupt[NUM_CORR]={
   // "         '         '         ", "         '         '         '         '         "
	{ "* Sharpness *",                 "+1 to Weapon Value.",
	  6601,                            "" },
	{ "* Expertise *",                 "+1 to All Attributes.",
	  6602,                            "" },
	{ "* Toughness *",                 "+1 to Armor Value.",
	  6603,                            "" },
	{ "* Absolution *",                "0.2%% more damage dealt for each buff ",
	  6604,                            "or debuff on you." },
	{ "* Vindication *",               "2%% of Total Armor Value granted as ",			// *
	  6605,                            "extra Weapon Value." },
	{ "* Scorn *",                     "Your debuffs ignore 5%% of enemy ",
	  6606,                            "Resistance." },
	{ "* Courage *",                   "Gain 1 additional Hit and Parry for ",			// *
	  6607,                            "every 100 missing hitpoints." },
	{ "* Master of None *",            "+2 to all skill limits.",
	  6608,                            "" },
	{ "* Necromancy *",                "2%% more Companion Hit and Parry scores.",		// *
	  6609,                            "" },
	{ "* Enigmatic *",                 "5%% reduced effect of debuffs on you.",
	  6610,                            "" },
	{ "* Barkskin *",                  "2%% of Total Weapon Value granted as ",			// *
	  6611,                            "extra Armor Value." },
	{ "* Penance *",                   "0.2%% less damage taken for each buff ",
	  6612,                            "or debuff on you." },
   // "         '         '         ", "         '         '         '         '         "
	{ "* Spiked *",                    "+2 to Thorns.",
	  6613,                            "" },
	{ "* Might *",                     "+2 to Strength.",
	  6614,                            "" },
	{ "* Ironskin *",                  "Gain 1 additional Armor Value for ",			// *
	  6615,                            "every 200 total Attributes." },
	{ "* Decapitation *",              "Cleave kills enemies left below ",				// *
	  6616,                            "2%% remaining health." },
	{ "* Sharkskin *",                 "5%% more total Thorns.",
	  6617,                            "" },
	{ "* Razor Shell *",               "10%% of Shield Armor Value is granted ",		// *
	  6618,                            "as extra Thorns." },
	{ "* Overlord *",                  "0.2%% more effect of Warcry and Rally ",
	  6619,                            "for every 10 total Strength." },
	{ "* Overwhelming Strength *",     "2%% increased total Strength. ",
	  6620,                            "+1 to Strength Limit." },
	{ "* Towering *",                  "8%% more Armor Value from Shields.",			// *
	  6621,                            "" },
	{ "* Bastion *",                   "4%% of total Resistance is granted as ",
	  6622,                            "extra Immunity." },
	{ "* Unbreakable *",               "3%% more total Armor Value.",
	  6623,                            "" },
	{ "* Deflecting *",                "5%% of Shield Armor Value is granted as ",		// *
	  6624,                            "extra Parry Score." },
   // "         '         '         ", "         '         '         '         '         "
	{ "* Force *",                     "Gain 1 additional Weapon Value for ",			// *
	  6625,                            "every 200 total Attributes." },
	{ "* Dexterity *",                 "+2 to Agility.",
	  6626,                            "" },
	{ "* Persistance *",               "+10 Endurance.",
	  6627,                            "" },
	{ "* Nocturne *",                  "5%% increased effect of Aria.",
	  6628,                            "" },
	{ "* Valor *",                     "3%% more total Weapon Value.",
	  6629,                            "" },
	{ "* Blade Dancer *",              "8%% more Weapon Value from Dual Swords.",		// *
	  6630,                            "" },
	{ "* Slaying *",                   "+1%% Critical Multiplier for every 10 ",
	  6631,                            "total Agility." },
	{ "* Overwhelming Agility *",      "2%% increased total Agility. ",
	  6632,                            "+1 to Agility Limit." },
	{ "* Axeman *",                    "2%% more damage dealt while using an ",			// *
	  6633,                            "Axe or Greataxe." },
	{ "* Overwhelm *",                 "5%% increased effect of Weaken and Crush.",	// *
	  6634,                            "" },
	{ "* Perseverance *",              "5%% more total Endurance.",
	  6635,                            "" },
	{ "* Recycle *",                   "10%% of Endurance spent is granted as ",		// *
	  6636,                            "additional Mana." },
   // "         '         '         ", "         '         '         '         '         "
	{ "* Rapidity *",                  "+2 to Attack Speed.",
	  6637,                            "" },
	{ "* Ruffian *",                   "+1 to Strength & +1 to Agility.",
	  6638,                            "" },
	{ "* Passion *",                   "+2 to Spell Aptitude.",
	  6639,                            "" },
	{ "* Alacrity *",                  "Zephyr deals 5%% more damage.",
	  6640,                            "" },
	{ "* Swiftness *",                 "3%% more total Attack Speed.",
	  6641,                            "" },
	{ "* Full Moon *",                 "10%% increased effect of bonuses granted ",		// *
	  6642,                            "during Full Moons." },
	{ "* Antagonizer *",               "0.2%% more effect of Blind and Douse ",
	  6643,                            "for every 10 total Agility." },
	{ "* Harrier *",                   "1%% increased total Agility and ",
	  6644,                            "Strength." },
	{ "* Butchery *",                  "0.2%% more effect of Cleave for every ",
	  6645,                            "10 total Strength." },
	{ "* Conqueror *",                 "5%% more damage dealt to enemies beside ",		// *
	  6646,                            "or behind you." },
	{ "* Zealotry *",                  "5%% more total Spell Aptitude.",
	  6647,                            "" },
	{ "* Crusade *",                   "+1 Skill Modifier per 50 Spell Aptitude.",
	  6648,                            "" },
   // "         '         '         ", "         '         '         '         '         "
	{ "* Expansiveness *",             "+1 to Area of Effect.",
	  6649,                            "" },
	{ "* Potency *",                   "+1 to Willpower & +1 to Intuition.",
	  6650,                            "" },
	{ "* Quickstep *",                 "+2 to Movement Speed.",
	  6651,                            "" },
	{ "* Tormenter *",                 "Poison deals damage 5%% faster.",
	  6652,                            "" },
	{ "* Grandiosity *",               "10%% more total Area of Effect.",
	  6653,                            "" },
	{ "* New Moon *",                  "10%% increased effect of bonuses granted ",		// *
	  6654,                            "during New Moons." },
	{ "* Coordination *",              "0.2%% more effect of Lethargy for every ",
	  6655,                            "10 total Willpower." },
	{ "* Pragmatic *",                 "1%% increased total Willpower and ",
	  6656,                            "Intuition." },
	{ "* Hex Master *",                "0.2%% more effect of Curse and Slow ",
	  6657,                            "for every 10 total Intuition." },
	{ "* Adroitness *",                "5%% less damage taken from enemies beside ",	// *
	  6658,                            "or behind you." },
	{ "* Fleet-footed *",              "3%% more total Movement Speed.",
	  6659,                            "" },
	{ "* Acceleration *",              "Haste grants 10%% more move speed to you.",
	  6660,                            "" },
   // "         '         '         ", "         '         '         '         '         "
	{ "* Nimbleness *",                "+2 to Cast Speed.",
	  6661,                            "" },
	{ "* Wisdom *",                    "+2 to Willpower.",
	  6662,                            "" },
	{ "* Vitality *",                  "+10 Hitpoints.",
	  6663,                            "" },
	{ "* Denial *",                    "5%% chance to not be hit when you should ",		// *
	  6664,                            "have been." },
	{ "* Spellslinger *",              "3%% more total Cast Speed.",
	  6665,                            "" },
	{ "* Training *",                  "5%% more effective surround hit modifier ",
	  6666,                            "while using a Spear." },
	{ "* Waning *",                    "Spells gain an additional 10%% of ",			// *
	  6667,                            "Willpower towards attribute bonuses." },
	{ "* Overwhelming Willpower *",    "2%% increased total Willpower. ",
	  6668,                            "+1 to Willpower Limit." },
	{ "* Shaper *",                    "0.2%% more effect of Shadow Copy for ",
	  6669,                            "every 10 total Willpower." },
	{ "* Wraithlord *",                "2%% of damage dealt by you is granted ",		// *
	  6670,                            "to your Companions as Hitpoints." },
	{ "* Constitution *",              "5%% more total Hitpoints.",
	  6671,                            "" },
	{ "* Barrier *",                   "Magic Shields and Shells affecting you ",		// *
	  6672,                            "take 10%% reduced damage from enemies." },
   // "         '         '         ", "         '         '         '         '         "
	{ "* Composure *",                 "+2 to Cooldown Rate.",
	  6673,                            "" },
	{ "* Intellect *",                 "+2 to Intuition.",
	  6674,                            "" },
	{ "* Wellspring *",                "+10 Mana.",
	  6675,                            "" },
	{ "* Detonation *",                "Blast kills enemies left below ",				// *
	  6676,                            "2%% remaining health." },
	{ "* Serenity *",                  "2%% more total Cooldown Rate.",
	  6677,                            "" },
	{ "* Refrigerate *",               "3%% chance for skills to have no ",				// *
	  6678,                            "cooldown." },
	{ "* Psychosis *",                 "0.2%% more effect of Pulse for every 10 ",
	  6679,                            "total Intuition." },
	{ "* Overwhelming Intuition *",    "2%% increased total Intuition. ",
	  6680,                            "+1 to Intuition Limit." },
	{ "* Waxing *",                    "Spells gain an additional 10%% of ",			// *
	  6681,                            "Intuition towards attribute bonuses." },
	{ "* Flow *",                      "10%% of overcapped Mana is granted as ",
	  6682,                            "additional Hitpoints." },
	{ "* Perpetuity *",                "5%% more total Mana.",
	  6683,                            "" },
	{ "* Repurpose *",                 "10%% of Mana spent is granted as ",				// *
	  6684,                            "additional Endurance." },
   // "         '         '         ", "         '         '         '         '         "
	{ "* Accuracy *",                  "+1 to Hit Score.",
	  6685,                            "" },
	{ "* Boldness *",                  "+2 to Braveness.",
	  6686,                            "" },
	{ "* Avoidance *",                 "+1 to Parry Score.",
	  6687,                            "" },
	{ "* Assassination *",             "5%% increased effect of Precision.",
	  6688,                            "" },
	{ "* Rigor *",                     "1%% more total Hit Score.",
	  6689,                            "" },
	{ "* Rebuke *",                    "5%% of damage taken from enemy thorns ",		// *
	  6690,                            "is reflected." },
	{ "* Perfectionism *",             "0.2%% more effect of Finesse for every 10 ",
	  6691,                            "total Braveness." },
	{ "* Overwhelming Braveness *",    "2%% increased total Braveness. ",
	  6692,                            "+1 to Braveness Limit." },
	{ "* Swordsman *",                 "2%% less damage taken while using a ",			// *
	  6693,                            "Sword or Twohander." },
	{ "* Mending *",                   "10%% increased effect of Heal and Regen.",		// *
	  6694,                            "" },
	{ "* Flexibility *",               "1%% more total Parry Score.",
	  6695,                            "" },
	{ "* Revoke *",                    "5%% of damage taken from enemy critical ",		// *
	  6696,                            "hits is reflected." },
   // "         '         '         ", "         '         '         '         '         "
	{ "* Maiming *",                   "+2 to Top Damage.",
	  6697,                            "" },
	{ "* Feast *",                     "+5 Hitpoints, Endurance, and Mana.",
	  6698,                            "" },
	{ "* Half Moon *",                 "5%% increased effect of bonuses granted ",		// *
	  6699,                            "during Moons." },
	{ "* Lustful *",                   "+1%% base crit chance per empty ring ",			// *
	  6700,                            "slot." },
	{ "* Ravager *",                   "5%% more total Top Damage.",
	  6701,                            "" },
	{ "* Culling *",                   "Critical Hits kill enemies left below ",		// *
	  6702,                            "2%% remaining health." },
	{ "* Wrath *",                     "0.2%% more effect of Rage & Calm per 50 ",
	  6703,                            "missing Hitpoints, Endurance, and Mana." },
	{ "* Sloth *",                     "3%% more Hitpoints, Endurance, and Mana.",
	  6704,                            "" },
	{ "* Hunger *",                    "2%% of damage dealt is restored as ",
	  6705,                            "Hitpoints, Endurance, or Mana." },
	{ "* Pride *",                     "Your debuffs ignore 5%% of enemy ",
	  6706,                            "Immunity." },
	{ "* Madness *",                   "+1 to Spell Modifier.",
	  6707,                            "" },
	{ "* Envious *",                   "+1 Spell Modifier per empty ring slot.",		// *
	  6708,                            "" }
};


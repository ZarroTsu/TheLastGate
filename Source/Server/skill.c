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
//	{ //, '/', 	"////////////////",		"////////////////////////////////////////////////////////////////////////////////",
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
//	{ //, '/', 	"////////////////",		"////////////////////////////////////////////////////////////////////////////////",
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
//	{ //, '/', 	"////////////////",		"////////////////////////////////////////////////////////////////////////////////",
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
//	{ //, '/', 	"////////////////",		"////////////////////////////////////////////////////////////////////////////////",
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
				
	{ 44, 'G', 	"Proximity", 			"Passively improves the area-of-effect of your Aria and Weaken skills.", // Braver
										"Passively improves the area-of-effect of your Poison, Curse, and Slow spells.", // Sorcerer
										"Passively improves the area-of-effect of your Blast and Pulse spells.", // Arch-Harakim
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
				
	{ 48, 'E', 	"Taunt",				"Use (Skill): Applies a debuff to your target and surrounding enemies, forcing them to attack you. This also applies a buff to yourself, granting damage resistance.", 
				"", "",
				{ AT_BRV, AT_STR, AT_STR }},
				
	{ 49, 'E', 	"Leap", 				"Use (Skill): Strike your foe and leap to a random nearby enemy, dealing critical damage to enemies at full life. Higher cooldown rate lets this skill repeat additional times.",
				"Leap (Critical)", 		"Use (Skill): Strike your foe and leap to your target, dealing critical damage and stunning enemies it hits.",
				{ AT_BRV, AT_AGL, AT_AGL }},
//	{ //, '/', 	"////////////////",		"////////////////////////////////////////////////////////////////////////////////",
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
		{ "Sharpness", 				"+2 to Weapon Value." },
		{ "Expertise", 				"+2 to All Attributes."},
		{ "Toughness", 				"+2 to Armor Value."},
		{ "Absolution", 			"0.5% more damage dealt for each buff or debuff on you."},
		{ "Vanquisher", 			"6% more total Weapon Value."},
		{ "Scorn", 					"Your debuffs ignore 20% of enemy Resistance."},
		{ "Determination", 			"Gain 1 additional Hit and Parry for every 100 total Attributes."},
		{ "Jack of All Trades", 	"4% increased total Attributes."},
		{ "Redemption", 			"Companions have Hit and Parry scores equal to yours, and learn Regen."},
		{ "Enigmatic", 				"20% reduced effect of debuffs on you."},
		{ "Steelskin", 				"6% more total Armor Value."},
		{ "Penance", 				"0.5% less damage taken for each buff or debuff on you."}
	},{	// Arch Templar
		{ "Spiked", 				"+5 to Thorns."},
		{ "Might", 					"+4 to Strength."},
		{ "Bulwark", 				"+3 to Armor Value."},
		{ "Serrated Blades", 		"Cleave deals additional damage based on your total Thorns."},
		{ "Sharkskin", 				"20% more total Thorns."},
		{ "Retaliation", 			"Your Thorns can now trigger on a parried hit with 10% power."},
		{ "Overlord", 				"0.5% more effect of Warcry and Rally for every 10 total Strength."},
		{ "Overwhelming Strength", 	"3% increased total Strength. +10 to Strength Limit."},
		{ "Censure", 				"Taunt reduces enemy Hit score by 5% for its duration."},
		{ "Bastion", 				"20% of total Resistance is granted as extra Immunity."},
		{ "Unbreakable", 			"9% more total Armor Value."},
		{ "Rampart", 				"50% more Parry granted by your Shield skill."}
	},{	// Skald
		{ "Muscle", 				"+3 to Weapon Value."},
		{ "Dexterity", 				"+4 to Agility."},
		{ "Persistance", 			"+20 Endurance."},
		{ "Nocturne", 				"20% increased effect of Aria."},
		{ "Valor", 					"9% more total Weapon Value."},
		{ "Enthusiasm", 			"Your Aria additionally grants nearby allies 10% of your Weapon Value."},
		{ "Slaying", 				"+3% Critical Multiplier for every 10 total Agility."},
		{ "Overwhelming Agility", 	"3% increased total Agility. +10 to Agility Limit."},
		{ "Acumen", 				"All melee skills use the attributes (STR+BRV/2) + Agility + Agility."},
		{ "Impact", 				"Weaken and Crush also reduce enemy damage multiplier and damage reduction."},
		{ "Perseverance", 			"20% more total Endurance."},
		{ "Tenacity", 				"20% of damage taken is dealt to your Endurance instead."}
	},{	// Warrior
		{ "Rapidity", 				"+5 to Attack Speed."},
		{ "Ruffian", 				"+3 to Strength & +3 to Agility."},
		{ "Passion", 				"+5 to Spell Aptitude."},
		{ "Alacrity", 				"Zephyr deals 20% more damage."},
		{ "Swiftness", 				"10% more total Attack Speed."},
		{ "Intensity", 				"+2 to Spell Modifier."},
		{ "Antagonizer", 			"0.5% more effect of Blind and Douse for every 10 total Agility."},
		{ "Harrier", 				"3% increased total Agility and Strength."},
		{ "Butchery", 				"0.5% more effect of Cleave for every 10 total Strength."},
		{ "Champion", 				"Enemies beside and behind you no longer gain a bonus to hitting you."},
		{ "Zealotry", 				"20% more total Spell Aptitude."},
		{ "Fervor", 				"20% of Spell Aptitude is used to reduce the strength of incoming enemy spells."}
	},{	// Sorcerer
		{ "Expansiveness", 			"+1 to Area of Effect."},
		{ "Potency", 				"+3 to Willpower & +3 to Intuition."},
		{ "Quickstep", 				"+5 to Movement Speed."},
		{ "Tormenter", 				"Poison deals damage 20% faster."},
		{ "Grandiosity", 			"20% more total Area of Effect."},
		{ "Brilliance", 			"+2 to Spell Modifier."},
		{ "Coordination", 			"0.5% more effect of Lethargy for every 10 total Willpower."},
		{ "Pragmatic", 				"3% increased total Willpower and Intuition."},
		{ "Hex Master", 			"0.5% more effect of Curse and Slow for every 10 total Intuition."},
		{ "Nimble", 				"You no longer have a parry penalty if hit while not fighting."},
		{ "Fleet-footed", 			"20% more total Movement Speed."},
		{ "Acceleration", 			"Haste grants 20% more move speed to you, and it can now be cast on allies."}
	},{	// Summoner
		{ "Nimbleness", 			"+5 to Cast Speed."},
		{ "Wisdom", 				"+4 to Willpower."},
		{ "Vitality", 				"+20 Hitpoints."},
		{ "Tactician", 				"Increases and multipliers to Cast Speed also affect Attack Speed."},
		{ "Spellslinger", 			"10% more total Cast Speed."},
		{ "Harpooner", 				"20% more Hit and Parry score while using a Spear."},
		{ "Mysticism", 				"All spell skills use the attributes (BRV+INT)/2 + Willpower + Willpower."},
		{ "Overwhelming Willpower", "3% increased total Willpower. +10 to Willpower Limit."},
		{ "Shaper", 				"0.5% more effect of Shadow Copy for every 10 total Willpower."},
		{ "Diviner", 				"Ghost Companions inherit your Dispel and the effects of your tarot cards."},
		{ "Constitution", 			"20% more total Hitpoints."},
		{ "Protector", 				"Magic Shield and Magic Shell are also cast on your active companions."}
	},{	// Arch Harakim
		{ "Composure", 				"+5 to Cooldown Rate."},
		{ "Intellect", 				"+4 to Intuition."},
		{ "Wellspring", 			"+20 Mana."},
		{ "Destroyer", 				"Blast has its base cooldown reduced for each enemy hit by it."},
		{ "Serenity", 				"5% more total Cooldown Rate."},
		{ "Strategist", 			"You suffer no cooldown if a spell is suppressed."},
		{ "Psychosis", 				"0.5% more effect of Pulse for every 10 total Intuition."},
		{ "Overwhelming Intuition", "3% increased total Intuition. +10 to Intuition Limit."},
		{ "Wizardry", 				"All spell skills use the attributes (BRV+WIL)/2 + Intuition + Intuition."},
		{ "Flow", 					"25% of overcapped Mana is granted as additional Hitpoints."},
		{ "Perpetuity", 			"20% more total Mana."},
		{ "Resourcefulness", 		"20% of damage taken is dealt to your Mana instead."}
	},{	// Braver
		{ "Accuracy", 				"+3 to Hit Score."},
		{ "Boldness", 				"+4 to Braveness."},
		{ "Avoidance", 				"+3 to Parry Score."},
		{ "Assassination", 			"20% increased effect of Precision."},
		{ "Rigor", 					"4% more total Hit Score."},
		{ "Deftness", 				"50% reduced damage taken from triggering enemy Thorns."},
		{ "Perfectionism", 			"0.5% more effect of Finesse for every 10 total Braveness."},
		{ "Overwhelming Braveness", "3% increased total Braveness. +10 to Braveness Limit."},
		{ "Virtuosity", 			"All weapon skills use the attributes (AGL+STR)/2 + Braveness + Braveness."},
		{ "Resilience", 			"40% less effect of Healing Sickness on you."},
		{ "Flexibility", 			"4% more total Parry Score."},
		{ "Litheness", 				"50% reduced extra damage taken from enemy Critical Hits."}
	},{	// Lycanthrope
		{ "Maiming", 				"+5 to Top Damage."},
		{ "Feast", 					"+10 Hitpoints, Endurance, and Mana."},
		{ "Insight", 				"+2 to Spell Modifier."},
		{ "Lust", 					"Ferocity grants +1% base crit chance per empty gear slot."},
		{ "Ravager", 				"20% more total Top Damage"},
		{ "Greed", 					"Your Top Damage is rolled an additional time, using the higher result."},
		{ "Wrath", 					"0.5% more effect of Rage & Calm per 50 missing Hitpoints, Endurance, and Mana."},
		{ "Sloth", 					"10% more Hitpoints, Endurance, and Mana."},
		{ "Gluttony", 				"8% of damage dealt is restored as Hitpoints, Endurance, and Mana."},
		{ "Pride", 					"Your debuffs ignore 20% of enemy Immunity."},
		{ "Madness", 				"+3 to Spell Modifier."},
		{ "Envy", 					"Ferocity grants +1 Spell Modifier per empty gear slot."}
	},{	// Contract
		{ "Reward", 				"200% increased chance of finding Rainbow Belts in contracts signed by you."},
		{ "Challenge", 				"+1 to rank of contracts signed by you."},
		{ "Army", 					"+1 enemy per spawn in contracts signed by you."},
		{ "Hope", 					"20% increased effect of rewards from green shrines in contracts signed by you."},
		{ "Opalescence", 			"200% increased chance of finding Rainbow Belts in contracts signed by you."},
		{ "Scholar", 				"20% more clear experience from contracts signed by you."},
		{ "Fate", 					"20% more effect of blue shrines in contracts signed by you."},
		{ "Hubris", 				"+1 to rank of contracts signed by you."},
		{ "Binding", 				"Contracts signed by you always grant tier 3 effects from quills."},
		{ "Destiny", 				"Red shrines produce harder enemies with more rewards in contracts signed by you."},
		{ "Swarm", 					"+1 enemy per spawn in contracts signed by you."},
		{ "Incentive", 				"Enemies grant an additional 5% of exp as Contract Pts in contracts signed by you."}
	}
};

struct sk_tree sk_corrupt[NUM_CORR]={
	{ "Sharpness", 				"(+1) to Weapon Value." },
	{ "Expertise", 				"(+1) to All Attributes."},
	{ "Toughness", 				"(+1) to Armor Value."},
	{ "Absolution", 			"(0.2%) more damage dealt for each buff or debuff on you."},
	{ "Vindication", 			"(2%) of Total Armor Value granted as extra Weapon Value."}, // *
	{ "Scorn", 					"Your debuffs ignore (5%) of enemy Resistance."},
	{ "Courage", 				"Gain (1) additional Hit and Parry for every 100 missing hitpoints."}, // *
	{ "Master of None", 		"(+2) to all skill limits."}, // *
	{ "Necromancy",				"(2%) more Companion Hit and Parry scores."}, // *
	{ "Enigmatic", 				"(5%) reduced effect of debuffs on you."},
	{ "Barkskin", 				"(2%) of Total Weapon Value granted as extra Armor Value."}, // *
	{ "Penance", 				"(0.2%) less damage taken for each buff or debuff on you."},
	{ "Spiked", 				"(+2) to Thorns."},
	{ "Might", 					"(+2) to Strength."},
	{ "Ironskin", 				"Gain (1) additional Armor Value for every 200 total Attributes."}, // *
	{ "Decapitation",			"Cleave kills enemies left below (2%) remaining health."}, // *
	{ "Sharkskin", 				"(5%) more total Thorns."},
	{ "Razor Shell",			"(10%) of Shield Armor Value is granted as extra Thorns."}, // *
	{ "Overlord", 				"(0.2%) more effect of Warcry and Rally for every 10 total Strength."},
	{ "Overwhelming Strength", 	"(2%) increased total Strength. (+1) to Strength Limit."},
	{ "Towering", 				"(8%) more Armor Value from Shields."}, // *
	{ "Bastion", 				"(4%) of total Resistance is granted as extra Immunity."},
	{ "Unbreakable", 			"(3%) more total Armor Value."},
	{ "Deflecting",				"(5%) of Shield Armor Value is granted as extra Parry Score."}, // *
	{ "Force", 					"Gain (1) additional Weapon Value for every 200 total Attributes."}, // *
	{ "Dexterity", 				"(+2) to Agility."},
	{ "Persistance", 			"(+10) Endurance."},
	{ "Nocturne", 				"(5%) increased effect of Aria."},
	{ "Valor", 					"(3%) more total Weapon Value."},
	{ "Blade Dancer", 			"(8%) more Weapon Value from Dual Swords."}, // *
	{ "Slaying", 				"(+1%) Critical Multiplier for every 10 total Agility."},
	{ "Overwhelming Agility", 	"(2%) increased total Agility. (+1) to Agility Limit."},
	{ "Axeman", 				"(2%) more damage dealt while using an Axe or Greataxe."}, // *
	{ "Overwhelm", 				"(5%) increased effect of Weaken and Crush."}, // *
	{ "Perseverance", 			"(5%) more total Endurance."},
	{ "Recycle", 				"(10%) of Endurance spent is granted as additional Mana."}, // *
	{ "Rapidity", 				"(+2) to Attack Speed."},
	{ "Ruffian", 				"(+1) to Strength & (+1) to Agility."},
	{ "Passion", 				"(+2) to Spell Aptitude."},
	{ "Alacrity", 				"Zephyr deals (5%) more damage."},
	{ "Swiftness", 				"(3%) more total Attack Speed."},
	{ "Full Moon", 				"(10%) increased effect of bonuses granted during Full Moons."}, // *
	{ "Antagonizer", 			"(0.2%) more effect of Blind and Douse for every 10 total Agility."},
	{ "Harrier", 				"(1%) increased total Agility and Strength."},
	{ "Butchery", 				"(0.2%) more effect of Cleave for every 10 total Strength."},
	{ "Conqueror", 				"(5%) more damage dealt to enemies beside or behind you."}, // *
	{ "Zealotry", 				"(5%) more total Spell Aptitude."},
	{ "Crusade", 				"(+1) Skill Modifier per 50 Spell Aptitude."},
	{ "Expansiveness", 			"(+1) to Area of Effect."},
	{ "Potency", 				"(+1) to Willpower & (+1) to Intuition."},
	{ "Quickstep", 				"(+2) to Movement Speed."},
	{ "Tormenter", 				"Poison deals damage (5%) faster."},
	{ "Grandiosity", 			"(10%) more total Area of Effect."},
	{ "New Moon", 				"(10%) increased effect of bonuses granted during New Moons."}, // *
	{ "Coordination", 			"(0.2%) more effect of Lethargy for every 10 total Willpower."},
	{ "Pragmatic", 				"(1%) increased total Willpower and Intuition."},
	{ "Hex Master", 			"(0.2%) more effect of Curse and Slow for every 10 total Intuition."},
	{ "Adroitness", 			"(5%) less damage taken from enemies beside or behind you."}, // *
	{ "Fleet-footed", 			"(3%) more total Movement Speed."},
	{ "Acceleration", 			"Haste grants (10%) more move speed to you."},
	{ "Nimbleness", 			"(+2) to Cast Speed."},
	{ "Wisdom", 				"(+2) to Willpower."},
	{ "Vitality", 				"(+10) Hitpoints."},
	{ "Denial", 				"(5%) chance to not be hit when you should have been."}, // *
	{ "Spellslinger", 			"(3%) more total Cast Speed."},
	{ "Training", 				"(5%) more effective surround hit modifier while using a Spear."},
	{ "Waning", 				"Spells gain an additional (10%) of Willpower towards attribute bonuses."}, // *
	{ "Overwhelming Willpower", "(2%) increased total Willpower. (+1) to Willpower Limit."},
	{ "Shaper", 				"(0.2%) more effect of Shadow Copy for every 10 total Willpower."},
	{ "Wraithlord", 			"(2%) of damage dealt by you is granted to your Companions as Hitpoints."}, // *
	{ "Constitution", 			"(5%) more total Hitpoints."},
	{ "Barrier", 				"Magic Shields and Shells affecting you take 10% reduced damage from enemies."}, // *
	{ "Composure", 				"(+2) to Cooldown Rate."},
	{ "Intellect", 				"(+2) to Intuition."},
	{ "Wellspring", 			"(+10) Mana."},
	{ "Detonation", 			"Blast kills enemies left below (2%) remaining health."}, // *
	{ "Serenity", 				"(2%) more total Cooldown Rate."},
	{ "Refrigerate", 			"(3%) chance for skills to have no cooldown."}, // *
	{ "Psychosis", 				"(0.2%) more effect of Pulse for every 10 total Intuition."},
	{ "Overwhelming Intuition", "(2%) increased total Intuition. (+1) to Intuition Limit."},
	{ "Waxing", 				"Spells gain an additional (10%) of Intuition towards attribute bonuses."}, // *
	{ "Flow", 					"(10%) of overcapped Mana is granted as additional Hitpoints."},
	{ "Perpetuity", 			"(5%) more total Mana."},
	{ "Repurpose", 				"(10%) of Mana spent is granted as additional Endurance."}, // *
	{ "Accuracy", 				"(+1) to Hit Score."},
	{ "Boldness", 				"(+2) to Braveness."},
	{ "Avoidance", 				"(+1) to Parry Score."},
	{ "Assassination", 			"(5%) increased effect of Precision."},
	{ "Rigor", 					"(1%) more total Hit Score."},
	{ "Rebuke", 				"(5%) of damage taken from enemy thorns is reflected."}, // *
	{ "Perfectionism", 			"(0.2%) more effect of Finesse for every 10 total Braveness."},
	{ "Overwhelming Braveness", "(2%) increased total Braveness. (+1) to Braveness Limit."},
	{ "Swordsman", 				"(2%) less damage taken while using a Sword or Twohander."}, // *
	{ "Mending", 				"(10%) increased effect of Heal and Regen."}, // *
	{ "Flexibility", 			"(1%) more total Parry Score."},
	{ "Revoke", 				"(5%) of damage taken from enemy critical hits is reflected."}, // *
	{ "Maiming", 				"(+2) to Top Damage."},
	{ "Feast", 					"(+5) Hitpoints, Endurance, and Mana."},
	{ "Half Moon", 				"(5%) increased effect of bonuses granted during Moons."}, // *
	{ "Lustful", 				"(+1%) base crit chance per empty ring slot."},
	{ "Ravager", 				"(5%) more total Top Damage"},
	{ "Culling", 				"Critical Hits kill enemies left below (2%) remaining health."}, // *
	{ "Wrath", 					"(0.2%) more effect of Rage & Calm per 50 missing Hitpoints, Endurance, and Mana."},
	{ "Sloth", 					"(3%) more Hitpoints, Endurance, and Mana."},
	{ "Hunger", 				"(2%) of damage dealt is restored as Hitpoints, Endurance, or Mana."},
	{ "Pride", 					"Your debuffs ignore (5%) of enemy Immunity."},
	{ "Madness", 				"(+1) to Spell Modifier"},
	{ "Envious", 				"(+1) Spell Modifier per empty ring slot."}
};
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
				
	{ 49, 'E', 	"Leap", 				"Use (Skill): Strike your foe and leap to your target, dealing critical damage and stunning them if they are at full health. Cooldown can be bypassed at an additional cost.", 
				"Leap (Random)", 		"Use (Skill): Strike your foe and leap through three random targets, dealing critical damage if they are at full health. Cooldown can be bypassed at an additional cost.",
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

struct sk_tree sk_tree[9][12]={
	{	// Seyan'du
		{ "Sharpness", 				"+2 to Weapon Value." },
		{ "Expertise", 				"+2 to All Attributes."},
		{ "Toughness", 				"+2 to Armor Value."},
		{ "Absolution", 			"0.5% more damage dealt for each buff or debuff on you."},
		{ "Vanquisher", 			"6% more total Weapon Value."},
		{ "Scorn", 					"Your debuffs ignore 20% of enemy Resistance."},
		{ "Determination", 			"Gain 1 additional Hit and Parry for every 100 total Attributes."},
		{ "Jack of All Trades", 	"4% more total Attributes."},
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
		{ "Overwhelming Strength", 	"+10 to Strength Limit."},
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
		{ "Slaying", 				"+2.5% Critical Multiplier for every 10 total Agility."},
		{ "Overwhelming Agility", 	"+10 to Agility Limit."},
		{ "Acumen", 				"All melee skills use the attributes (STR+BRV/2) + Agility + Agility."},
		{ "Impact", 				"Weaken and Crush also reduce enemy critical hit chance."},
		{ "Perseverance", 			"20% more total Endurance."},
		{ "Tenacity", 				"10% of damage taken is dealt to your Endurance instead."}
	},{	// Warrior
		{ "Rapidity", 				"+5 to Attack Speed."},
		{ "Ruffian", 				"+3 to Strength & +3 to Agility."},
		{ "Passion", 				"+5 to Spell Aptitude."},
		{ "Alacrity", 				"Zephyr deals 20% more damage."},
		{ "Swiftness", 				"10% more total Attack Speed."},
		{ "Intensity", 				"+2 to Spell Modifier."},
		{ "Antagonizer", 			"0.5% more effect of Blind and Douse for every 10 total Agility."},
		{ "Harrier", 				"3% more total Agility and Strength."},
		{ "Butchery", 				"2% more effect of Cleave for every 10 total Strength."},
		{ "Champion", 				"Enemies beside and behind you no longer gain a bonus to hitting you."},
		{ "Zealotry", 				"20% more total Spell Aptitude."},
		{ "Fervor", 				"0.5% less damage taken per 10 Spell Aptitude."}
	},{	// Sorcerer
		{ "Expansiveness", 			"+1 to Area of Effect."},
		{ "Potency", 				"+3 to Willpower & +3 to Intuition."},
		{ "Quickstep", 				"+5 to Movement Speed."},
		{ "Tormenter", 				"Poison deals damage 10% faster."},
		{ "Grandiosity", 			"20% more total Area of Effect."},
		{ "Brilliance", 			"+2 to Spell Modifier."},
		{ "Coordination", 			"1% more effect of Lethargy for every 10 total Willpower."},
		{ "Pragmatic", 				"3% more total Willpower and Intuition."},
		{ "Hex Master", 			"0.4% more effect of Curse and Slow for every 10 total Intuition."},
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
		{ "Overwhelming Willpower", "+10 to Willpower Limit."},
		{ "Shaper", 				"1% more effect of Shadow Copy for every 10 total Willpower."},
		{ "Diviner", 				"Ghost Companions inherit the effects of your tarot cards."},
		{ "Constitution", 			"20% more total Hitpoints."},
		{ "Protector", 				"Magic Shield and Magic Shell are also cast on your active companions."}
	},{	// Arch Harakim
		{ "Composure", 				"+5 to Cooldown Rate."},
		{ "Intellect", 				"+4 to Intuition."},
		{ "Wellspring", 			"+20 Mana."},
		{ "Destroyer", 				"Blast has its base cooldown reduced for each enemy hit by it."},
		{ "Serenity", 				"10% more total Cooldown Rate."},
		{ "Strategist", 			"You suffer no cooldown if a spell is suppressed."},
		{ "Psychosis", 				"2% more effect of Pulse for every 10 total Intuition."},
		{ "Overwhelming Intuition", "+10 to Intuition Limit."},
		{ "Wizardry", 				"All spell skills use the attributes (BRV+WIL)/2 + Intuition + Intuition."},
		{ "Flow", 					"50% of overcapped Mana is granted as additional Hitpoints."},
		{ "Perpetuity", 			"20% more total Mana."},
		{ "Resourcefulness", 		"10% of damage taken is dealt to your Mana instead."}
	},{	// Braver
		{ "Accuracy", 				"+3 to Hit Score."},
		{ "Boldness", 				"+4 to Braveness."},
		{ "Avoidance", 				"+3 to Parry Score."},
		{ "Assassination", 			"15% increased effect of Precision."},
		{ "Rigor", 					"4% more total Hit Score."},
		{ "Deftness", 				"50% reduced damage taken from triggering enemy Thorns."},
		{ "Perfectionism", 			"1% more effect of Finesse for every 10 total Braveness."},
		{ "Overwhelming Braveness", "+10 to Braveness Limit."},
		{ "Virtuosity", 			"All weapon skills use the attributes (AGL+STR)/2 + Braveness + Braveness."},
		{ "Resilience", 			"25% less effect of Healing Sickness on you."},
		{ "Flexibility", 			"4% more total Parry Score."},
		{ "Litheness", 				"50% reduced extra damage taken from enemy Critical Hits."}
	},{	// Lycanthrope
		{ "Maiming", 				"+5 to Top Damage."},
		{ "Feast", 					"+10 Hitpoints, Endurance, and Mana."},
		{ "Insight", 				"+2 to Spell Modifier."},
		{ "Lust", 					"While in Ratling form, 5% of damage dealt is restored as hitpoints."},
		{ "Ravager", 				"20% more total Top Damage"},
		{ "Greed", 					"Your Top Damage is rolled an additional time, using the higher result."},
		{ "Wrath", 					"1% more effect of Rage for every 50 missing Hitpoints, Endurance, and Mana."},
		{ "Gluttony", 				"10% more Hitpoints, Endurance, and Mana."},
		{ "Sloth", 					"1% more effect of Calm for every 50 missing Hitpoints, Endurance, and Mana."},
		{ "Pride", 					"Your debuffs ignore 20% of enemy Immunity."},
		{ "Madness", 				"+3 to Spell Modifier"},
		{ "Envy", 					"While in Greenling form, 5% of damage dealt is restored as mana."}
	}
};
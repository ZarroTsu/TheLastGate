/*************************************************************************

   This file is part of 'Mercenaries of Astonia v2'
   Copyright (c) 1997-2001 Daniel Brockhaus (joker@astonia.com)
   All rights reserved.

 **************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include "server.h"

//{ top
// You HAVE to use DO_SAYX! not DO_SAY!!

void strlower(char *a)
{
	while (*a)
	{
		*a = tolower(*a);
		a++;
	}
}

// these words are ignore
char *fillword[] = {"the", "a", "an", "do", "'", "of", "is", "that", "those", "these", "they", "-", "does", "can", "oh", "me", "about", "to", "if", "for"};

int is_fillword(char *a)
{
	int n;

	for (n = 0; n<sizeof(fillword) / sizeof(fillword[0]); n++)
	{
		if (!strcmp(a, fillword[n]))
		{
			return( 1);
		}
	}

	return(0);
}

// left word is changed into right word before parsing the text
char *syn[] = {
	"1",           "one",
	"2",           "two",
	"3",           "three",
	"4",           "four",
	"5",           "five",
	"6",           "six",
	"7",           "seven",
	"8",           "eight",
	"9",           "nine",
	"n",           "north",
	"northern",    "north",
	"e",           "east",
	"eastern",     "east",
	"s",           "south",
	"southern",    "south",
	"w",           "west",
	"western",     "west",
	"whats",       "what",
	"which",       "what",
	"wheres",      "where",
	"dangers",     "danger",
	"enemies",     "danger",
	"enemy",       "danger",
	"foe",         "danger",
	"foes",        "danger",
	"thieves",     "thief",
	"trouble",     "danger",
	"laby",        "labyrinth",
	"laboratory",  "lab",
	"rubies",      "ruby",
	"joes",        "joe",
	"armour",      "armor",
	"tulip",       "flower",
	"rose",        "flower",
	"skeletons",   "skeleton",
	"templars",    "templar",
	"outlaws",     "outlaw",
	"merchants",   "merchant",
	"lakebed",     "lake",
	"targe",       "shield",
	"heater",      "shield",
	"hi",          "hello",
	"hail",        "hello",
	"greetings",   "hello",
	"goodbye",     "bye",
	"whos",        "who",
	"thanks",      "thank",
	"mission",     "quest",
	"starting",    "start",
	"damors",      "damor",
	"mines",       "mine",
	"statuette",   "statue",
	"bandits",     "bandit",
	"point",       "place",
	"1st",         "first",
	"2nd",         "second",
	"3rd",         "third",
	"limitations", "limitation",
	"limits",      "limitation",
	"limit",       "limitation",
	"quit",        "exit",
	"leave",       "exit",
	"ratlings",    "ratling",
	"greenlings",  "greenling",
	"soglings",    "sogling",
	"eyes",        "eye",
	"helmet",      "helm",
	"shadows",     "shadow",
	"prison",      "jail",
	"penitentiary","jail",
	"poems",       "poem",
	"doors",       "door",
	"keys",        "key",
	"skills",      "skill",
	"riddles",     "riddle"
};

void replace_synonym(char *a)
{
	int n;

	for (n = 0; n<sizeof(syn) / sizeof(syn[0]); n += 2)
	{
		if (!strcmp(a, syn[n]))
		{
			strcpy(a, syn[n + 1]);
			return;
		}
	}
}

struct know
{
	char *word[20];         // up to 20 keywords which trigger the response
	                        // !word means the word is essential, ?word means the word may or may not be in the input
	                        // ? alone shows a question.
	                        // example: "!where", "?haunted", "!castle", "?"
	                        // "where" and "castle" have to be in the input, while "haunted" may be there or not.

	int value;              // difficulty of the question. used to keep some NPCs from telling everything about an area
							// ** the number ranges used is arbitrary, so it doesn't matter if it ranges from [1 - 5] or [1 - 100] **
	int area;               // area of the game the question belongs to
	int temp;               // ID of the NPC who know the question (example follows)
	char *answer;           // actual text of the answer. Note: Never more than 400 chars!
	int special;            // needs a special function (piece of code) to answer the question
};

/* example:

   assume value=50, area=AR_ASTON and temp=42

   the question will only get answered if the NPC has a knowledge of at least 50, he knows about AR_ASTON and has the ID 42 */

// Lynbore 		- starting town		// llyn bore    - meaning 'morning lake'
// Adernal  	- middle town		// aderyn haul  - meaning 'Sun bird' (ie. Skua)
// Neiseer 		- last town			// neithiwr - meaning 'last night'

// "Area" Numbers
#define AR_GENERAL		  0
//}
//{ Lynbore
#define AR_LYNBOR	 	100		// locations in Lynbore
#define AR_THIEF1		101
#define AR_PARK			102
#define AR_TOMB			103
#define AR_LIBRY		104
#define AR_JOE			105
#define AR_OUTLAW		106
#define AR_SKLORD		107
#define AR_CASTLE		108
#define AR_DOORS		109
#define AR_MINE1		110
#define AR_STONE		111
#define AR_THIEF2		112
#define AR_LIPOT		113
#define AR_HEDGE		114
#define AR_MMAZE		115
#define AR_WOODS		116
#define AR_ARGHA		117
//}
//{ Aston
#define AR_ASTON		200		// locations in Astonia
#define AR_SKUA			201
#define AR_PURPLE		202
#define AR_ARACH		203
#define AR_MANSION		204
#define AR_BELL			205
#define AR_WEBBED		206
#define AR_MINE2		207
#define AR_PENITN		208
#define AR_GROLMLB		209
#define AR_HERMIT		210
#define AR_LAKEBED		211
#define AR_SEWER1		212
#define AR_SEWER2		213
#define AR_GARG			214
#define AR_FOREST		215
#define AR_STHOLD		216
//
#define AR_CANYON		217
#define AR_PASS			218
#define AR_ARCHIVE		219
#define AR_MINE3		220
#define AR_SWAMP		221
#define AR_MOUNT		222
#define AR_DIAMOND		223
#define AR_HERBS		224
#define AR_TARGE		225
#define AR_SEWER3		226
//}
//{ Neiseer
#define AR_NEISEE		300		// locations in Neiseer
//}
//{ Labyrinth
#define AR_LABYRINTH	1000	// locations in the Labyrinth
#define AR_LAB_GROLM	1010
#define AR_LAB_LIZARD	1020
#define AR_LAB_UNDEAD	1030
#define AR_LAB_SPELL	1040
#define AR_LAB_KNIGHT	1050
#define AR_LAB_DESERT	1060
#define AR_LAB_DARK		1070
#define AR_LAB_WATER	1080
#define AR_LAB_RIDDLE	1090
#define AR_LAB_RIDDLE1  1091	// Keep in tune with RIDDLE_MIN_AREA/RIDDLE_MAX_AREA in lab9.h !!
#define AR_LAB_RIDDLE2  1092	// Keep in tune with RIDDLE_MIN_AREA/RIDDLE_MAX_AREA in lab9.h !!
#define AR_LAB_RIDDLE3  1093	// Keep in tune with RIDDLE_MIN_AREA/RIDDLE_MAX_AREA in lab9.h !!
#define AR_LAB_RIDDLE4  1094	// Keep in tune with RIDDLE_MIN_AREA/RIDDLE_MAX_AREA in lab9.h !!
#define AR_LAB_RIDDLE5  1095	// Keep in tune with RIDDLE_MIN_AREA/RIDDLE_MAX_AREA in lab9.h !!
#define AR_LAB_FOREST	1100
#define AR_LAB_SEASON	1110
#define AR_LAB_MOUNT	1120
#define AR_LAB_GATE		1130
//}

#define AR_ALL 			12345	// NPC with this area value knows /everything/

// "Answer" values
//{ Answers for each skill question ("What is ...?")
#define AN_SK_HAND		"The Hand to Hand skill assists with one's ability to hit and parry while unarmed."
#define AN_SK_PUGILISM	"Precision is an advanced skill learned by Brawlers. It lets them score critical hits more often."
#define AN_SK_DAGGER	"The Dagger skill will help with one's ability to hit and parry while using a dagger."
#define AN_SK_SWORD		"The Sword skill helps with one's ability to hit and parry attacks with a sword."
#define AN_SK_AXE		"The Axe skill will help with one's ability to hit and parry when using an axe."
#define AN_SK_STAFF		"The Staff skill will assist with hitting and parrying while using a staff."
#define AN_SK_TWOHAND	"The Two-Handed skill helps with hitting and parrying when using a two-handed weapon."
#define AN_SK_FOCUS		"Focus is a rare skill learned by veteran Seyan'du. It improves their ability to cast any spell."
#define AN_SK_STEALTH	"Stealth is a skill that assists with avoiding fights. With enough stealth, one can appear invisible!"
#define AN_SK_PERCEPT	"Perception is a skill for seeing and hearing. Some say that with enough investment, one can see in the dark!"
#define AN_SK_SWIM		"The Swimming skill allows one to survive underwater without drowning."
#define AN_SK_MSHIELD	"Magic Shield is a spell learned by Harakims. It provides an added layer of defense that degrades over time."
#define AN_SK_BARTER	"Bartering will help you haggle with shopkeepers, letting you sell for more and buy for less."
#define AN_SK_REPAIR	"Repair, when used while holding an item, allows one to try to repair it."
#define AN_SK_LIGHT		"Light is the most simple spell. It allows one, or their target, to begin glowing."
#define AN_SK_RECALL	"Recall is a spell that allows one to return to a place they call safe, be it a tavern or temple."
#define AN_SK_SHIELD	"The Shield skill assists with parrying blows when equipped with a shield."
#define AN_SK_PROTECT	"Protect is a spell that provides additional armor value to its caster or their target."
#define AN_SK_ENHANCE	"Enhance is a spell that provides additional weapon value to its caster or their target."
#define AN_SK_SLOW		"Slow is a spell that, well, slows a caster's target. The target may slowly speed back up over time."
#define AN_SK_CURSE		"Curse is a powerful spell that reduces the stats of a caster's target."
#define AN_SK_BLESS		"Bless is a powerful spell that improves all stats of a caster or their target."
#define AN_SK_IDENTIFY	"Identify is a spell that allows a caster to look at the stats of an item or an individual."
#define AN_SK_RESIST	"The Resistance skill allows you to avoid being affected by negative ailments like Curse or Slow."
#define AN_SK_BLAST		"Blast is a powerful spell used by Harakims. It inflicts damage on their targets at long range."
#define AN_SK_DISPEL	"Dispel is a defensive spell that allows a caster to remove negative ailments from themself or their target."
#define AN_SK_HEAL		"Heal is a spell that can heal a caster or their target's wounds. Repeated casts have a worse effect, however."
#define AN_SK_COMPAN	"Ghost Companion is a powerful spell used by Harakims. It allows them to summon a minion to assist with fighting."
#define AN_SK_REGEN		"Regenerate is a skill that allows one to slowly recover their health over time."
#define AN_SK_REST		"Rest is a skill that allows one to recover their endurance over time."
#define AN_SK_MEDIT		"Meditate is a skill that allows one to recover their mana over time."
#define AN_SK_SENSE		"The Sense Magic skill allows one to be more perceptive of items or effects that are magical in nature."
#define AN_SK_IMMUNITY	"Immunity is a skill learned by Templars. It reduces the effectiveness of negative ailments on them."
#define AN_SK_SURROUND	"Surround Hit is a skill learned by Templars. It allows them to hit everything around them at once!"
#define AN_SK_CONCENT	"Concentrate is an advanced skill learned by powerful mages. It reduces the mana cost of all spells."
#define AN_SK_WARCRY	"Warcry is an advanced skill learned by powerful fighters. It frightens and stuns all nearby enemies."
#define AN_SK_DUALW		"The Dual Wield skill provides additional chance to hit enemies when using an off-handed weapons."
#define AN_SK_COMBATM	"Combat Mastery is a skill used by Templars. It gives them additional chances to hit and parry with any weapon."
#define AN_SK_WEAPONM	"Weapon Mastery is a skill used by Templars. It gives them up to double the weapon value of their weapons."
#define AN_SK_ARMORM	"Armor Mastery is a skill used by Templars. It gives them up to double the armor value of their armor."
#define AN_SK_CLEAVE	"Cleave is a powerful skill used by Templars. It allows them to hit their enemies with a powerful blow."
#define AN_SK_WEAKEN	"Weaken is a powerful skill used by Templars. It allows them to reduce their enemy's weapon and armor values."
#define AN_SK_POISON	"Poison is an advanced spell learned by powerful mages. It inflicts an ailment that eats away at their target's health."
#define AN_SK_DAMPROX	"Damage Proximity is an advanced skill learned by powerful Harakim. It grants an area of effect to spells that deal damage."
#define AN_SK_HEXPROX	"Hex Proximity is an advanced skill learned by Sorcerers. It grants an area of effect to spells that inflict ailments."
#define AN_SK_COMPM		"Companion Mastery is an advanced skill learned by Summoners. Their Ghost Companion becomes stronger and learns the Heal spell."
#define AN_SK_SHADOW	"Shadow Copy is an advanced spell learned by Summoners and Brawlers. It lets them summon a copy of themself to fight for them."
#define AN_SK_HASTE		"Haste is an advanced spell learned by Warriors and Sorcerers. It allows them to dramatically speed up their actions."
#define AN_SK_SURRAREA	"Surround Area is an advanced skill learned by powerful Templar. It grants an area of effect to their Surround Hit skill."
#define AN_SK_SURRRATE	"Surround Rate is an advanced skill learned by Warriors. It grants an additional chance to trigger Surround Hit."
//}
//{ "What is ...?" for each race
#define AN_RA_TEMP		"Templars are powerful fighters. They are not very good with spells, but they have learned to adapt without them."
#define AN_RA_MERC		"Mercenaries are adept at both fighting and spellcasting, but do not excel in either."
#define AN_RA_HARA		"Harakim are powerful spellcasters. They are not very good at combat, and prefer to act from long range."
#define AN_RA_SEYANDU	"Seyan'du are very powerful and can excel in a large number of things at once."
#define AN_RA_ARCHTEMP	"Arch-Templar are Templar who have become weapon masters. They can cleave the air so hard that it can fell an entire army."
#define AN_RA_PUGILIST	"Brawlers are Templars who attack with agility and precision. Their attacks are freightening and their very shadows can assist them."
#define AN_RA_WARRIOR	"Warriors are Merceneries who have preferred melee over spells. They can become very agile and use multiple weapons at once."
#define AN_RA_SORCERER	"Sorcerers are Merceneries who have preferred spells over melee. They can cast powerful ailments in a wide area instantaniously."
#define AN_RA_SUMMONER	"Summoners are Harakims who have dedicated their arts to their companion. They can summon their shadows to assist with spellcasting."
#define AN_RA_ARCHHARA	"Arch-Harakim are Harakim who have become masterful casters. They know powerful spells and can detonate enemy swarms in an instant."
//}
//{ Etc Common knowledge
// Common Labyrinth knowledge ("What gate?" "What labyrinth?")
#define AN_LABY			"The labyrinth is a huge maze, full of dangerous trials. If you can reach the end, you can advance to more powerful classes."
#define AN_LABY_GATE	"The last gate of the labyrinth, where you are tested by the great Gatekeeper."
#define AN_LABY_KEEPER	"The Gatekeeper is a decendant of the first Seyan'du. He is old, but wise, and very strong."
//}
//{ Dietys
// Common deity knowledge ("Who is ...?")
#define AN_GOD_SKUA		"Skua is the god of peace and order. He is said to have created the sun."
#define AN_GOD_ANKH		"Ankh is the goddess of magic. She is said to have created the moon."
#define AN_GOD_ISHTAR	"Ishtar is the diety of dreams. They are said to have created the stars."
#define AN_GOD_GORN		"Gorn is the god of prosperity. He is said to have created the clouds."
#define AN_GOD_KWAI		"Kwai is the goddess of war. She is said to have created the sky."
#define AN_GOD_PURPLE	"The Purple One is the god of chaos and disorder. He is said to be the inky black void of the night."

// More complex deity knowledge ("Who is ...?" from bishop)
#define AN_GOD_SKUA2	"Skua is the god of peace and order. With his divine power he created the sun so that we may live in warmth and light."
#define AN_GOD_ANKH2	"Ankh is the goddess of magic. With her divine power she created the moon, so that we may tell time and receive blessings each month."
#define AN_GOD_ISHTAR2	"Ishtar is the diety of dreams. With their divine power they created the stars so that the night sky might be filled with wonder."
#define AN_GOD_GORN2	"Gorn is the god of prosperity. With his divine power he created the clouds, so that rain might pour to feed our crops."
#define AN_GOD_KWAI2	"Kwai is the goddess of war. With her divine weapon she tore open the sky, so that we might see the gods in all their splendor."
#define AN_GOD_PURPLE2	"The Purple One is the god of chaos and disorder. We dare not speak nor know his true name, lest his divine hatred destroy us all."

// Twisted deity knowledge ("Who is ...?" from priest)
#define AN_GOD_SKUA3	"Skua is the false god of peace and order. He took the sun from our lord Purple One, and hangs it high away from his grasp."
#define AN_GOD_ANKH3	"Ankh is the false goddess of magic. She took the moon from our lord Purple One, and taunts him with each passing day."
#define AN_GOD_ISHTAR3	"Ishtar is the false diety of dreams. They took the stars from our lord Purple One, and tempts him with their splendor."
#define AN_GOD_GORN3	"Gorn is the false god of prosperity. He took the clouds from our lord Purple one, and rains rot and dispair upon us all."
#define AN_GOD_KWAI3	"Kwai is the false goddess of war. Her weapon cleaved darkness away from our lord Purple one, but he shall strike back through us."
#define AN_GOD_PURPLE3	"The Purple One is the god of all things. With his divine blessing we fight against false gods, so that he may rise once more."

// Purple One jibberish
#define AN_PURPLE1 "Our order, the Cult of the Purple One, does not believes in rules. Join us, and you can do whatever you want."
#define AN_PURPLE2 "If you join us, you will be able to kill your fellow players. But beware! Others can kill you as well!"
#define AN_PURPLE3 "So be it. But you have to pass a test first: Kill me!"
#define AN_PURPLE4 "Yes. To join, you have to kill me. Go ahead, you coward!"
//}
//{ Lynbore
// Answers to common questions (in Lynbore)
// "Where is...?"
#define AN_LYN_TAVERN	"The Bluebird Tavern is located on the west end, nested between First Street, Second Street, and Shore Crescent."
#define AN_LYN_THIEF	"The Thieves House is located on the west end of First Street."
#define AN_LYN_PARK		"The park is located west of the corner of First Street and Silver Avenue."
#define AN_LYN_TOMB		"The Cursed Tomb is located east of the corner of First Street and Silver Avenue."
#define AN_LYN_LIBRARY	"The library is located between First Street and Second Street, on the east end of town."
#define AN_LYN_JOE		"Joe's house is on Second Street, between Silver Avenue and Rose Lane."
#define AN_LYN_OUTLAW	"The Templar Outlaws live in a fortified building on the east end of town, off Rose Lane."
#define AN_LYN_SKLORD	"The Skeleton Lord may be found on Rose Lane, between Second Street and Castle Way."
#define AN_LYN_CASTLE	"The Haunted Castle is located in the south-east corner of Lynbore, at the end of Castle Way."
#define AN_LYN_DOORS	"The Dungeon of Doors is located to the south on Castle Way."
#define AN_LYN_MMAZE	"The Magic Maze is located on the east end of Shore Crescent where it meets Silver Avenue."
#define AN_LYN_HEDGE	"The Hedge Maze is located between Second Street and Shore Crescent."
#define AN_LYN_MINE1	"You can find the Lynbore Mines on the north end of Silver Avenue."
#define AN_LYN_WOODS	"The Weeping Woods are to the east of Lynbore, off the east end of First Street."
#define AN_LYN_SPIDERS	"You can find a den full of Spiders on the south side of Weeping Woods, a short walk from Lynbore."
#define AN_LYN_MANOR	"The Abandoned Manor is on the south side of Weeping Woods, hiding amongst the trees."
#define AN_LYN_BANK		"There is a bank off the crossroad of Second Street and Silver Avenue."
#define AN_LYN_SHOP		"You can find many shops at the middle of Silver Avenue. There is also a small magic shop in the Lynbore Library."
#define AN_LYN_SHOCRE	"Shore Crescent is on the east end of Lynbore. It travels along the coastline, curving inland."
#define AN_LYN_FIRSTR	"First Street is the northern street in Lynbore. It travels from the coastline to the Weeping Woods."
#define AN_LYN_SECSTR	"Second Street is in the middle of Lynbore. It travels from the coastline to Rose Lane."
#define AN_LYN_SILAVE	"Silver Avenue is in the middle of Lynbore, travelling from the northern Mines all the way south to Castle Way."
#define AN_LYN_CASWAY	"Castle Way is on the southern end of Lynbore. It travels from the Haunted Castle, toward Silver Avenue and Shore Crescent"
#define AN_LYN_ROSLAN	"Rose Lane is on the eastern side of Lynbore. It travels from the Guard House down toward Castle Way."
#define AN_LYN_GUAHOU	"You can find the Guard House next to the exit from Lynbore into the Weeping Woods. Many Guards can be found there."
#define AN_LYN_LYNBORE	"This town is named Lynbore, the lakeside town of Astonia."

// Answers to quest questions (in Lynbore)
// WHO took it, WHAT was the item, WHERE is the quest, WHY do you want the item, and any other clues
#define AN_THIEF1_WHO	"The thieves are a crafty bunch, but not terribly strong."
#define AN_THIEF1_WHAT	"It's a small golden amulet. It was once my mother's."
#define AN_THIEF1_WHER	"The Thieves House is located on the west end of First Street, right across the road outside."
#define AN_THIEF1_WHY	"The thieves stole my amulet. I would quite like to have it back, thank you."
#define AN_THIEF1_LOK1	"There may be a few locked doors in the thieves house. The first key should be easy to find."
#define AN_THIEF1_LOK2	"The second door leads further inside, where stronger thieves hide. You might want to avoid going there..."

#define AN_PARK_WHO		"You may find Crawlers in the park; they snuck up on me last I visited. They are disgusting creatures."
#define AN_PARK_WHAT	"It is a dagger made of stone. It is very old and worn."
#define AN_PARK_WHER	"Lynbore Park can be found across the road, at the corner of First Street and Silver Avenue."
#define AN_PARK_WHY		"I need the stone dagger as a part of my research into the occult."

#define AN_TOMB_WHO		"The Cursed Tomb is filled with living skeletons. They're poor fighters but make up for it with numbers."
#define AN_TOMB_WHAT	"It is a large, serated axe, but primitive in design. A Sereated Slab, to put it a better way."
#define AN_TOMB_WHER	"You can find the Cursed Tomb across the crossroad from here, at the opposite corner of First Street and Silver Avenue."
#define AN_TOMB_WHY		"I like to collect strange weapons, and this one piqued my interests."
#define AN_TOMB_WALL	"There may be illusionary walls in the Cursed Tomb. Ones that aren't really there, that you can walk through like air."

#define AN_LIBRY_WHO	"The Harakim are weak mages, but crafty. Be wary of their Blast spell."
#define AN_LIBRY_WHAT	"It is a golden belt with a magical enchantment."
#define AN_LIBRY_WHER	"We're in the library right now, silly. Try one of the two doors side-by-side."
#define AN_LIBRY_WHY	"The belt was a part of my studies. I'm learning about magical items and enchanting."
#define AN_LIBRY_LOCK	"The harakims may have hidden the key somewhere in the library. Or maybe it's hiding in plain sight?"

#define AN_JOE_WHO		"Joe is a coward and often calls his brothers to fight his battles. It may be wise to prevent that."
#define AN_JOE_WHAT		"It is a bronze breastplate adorned with a small ruby. It was a gift from my father."
#define AN_JOE_WHER		"Joe's House is next door, here on Second Street."
#define AN_JOE_WHY		"It was a very valuable gift from my father. Joe stole it from me."

#define AN_OUTLAW_WHO	"The Templar Outlaws are skilled fighters and may all attack at once. Be careful!"
#define AN_OUTLAW_WHAT	"The barbarian sword is an old weapon. It was made by an now extinct race of barbarians."
#define AN_OUTLAW_WHER	"The Templar Outlaws live in a fortified building on the east side of Lynbore, just off of Rose Lane."
#define AN_OUTLAW_WHY	"It's a good weapon. I like it. And the templars are outlaws, taking from them is not stealing..."

#define AN_SKLORD_WHO	"The Skeleton Lord was one the lord of Lynbore. After the fire, he and his guards returned to walk his halls."
#define AN_SKLORD_WHAT	"The Decorative Sword is a two-handed longsword fitted with... well, decorations."
#define AN_SKLORD_WHER	"The Skeleton Lord hides in a burned building at the middle of Rose Lane. It is covered in a thick grey ash."
#define AN_SKLORD_WHY	"I gifted it to the lord when he was still alive. He has no more use for it now."

#define AN_CASTLE_WHO	"The Haunted Castle was once home to Azrael, an old king of Aston. It is filled with ghosts now."
#define AN_CASTLE_WHAT	"The Spellblade is a powerful weapon made for the off-hand. It enhances magic spells."
#define AN_CASTLE_WHER	"The Haunted Castle can be found at the end of Castle Way. It's where the name comes from, after all!"
#define AN_CASTLE_WHY	"It is a powerful weapon, why wouldn't someone want it?"
#define AN_CASTLE_LOCK	"There are a few locked doors in the Haunted Castle. Perhaps some ghosts hide behind false walls?"
#define AN_CASTLE_AZR1	"Azrael, the shadow king he was called. He once wore a striking helmet made of black material."
#define AN_CASTLE_AZR2	"Azrael's corpse was left in his throne. There may be a way inside, but I'd leave him be if I were you!"
#define AN_CASTLE_AZR3	"The Shadow Helmet was said to give Azrael terrifying skill with any weapon. Its magic may have faded by now, however."

#define AN_DOORS_WHO	"The Dungeon of Doors is full of Mercenary Outlaws. They are quite strong, but their numbers are thin."
#define AN_DOORS_WHAT	"It is a small amulet made of a magical blue metal."
#define AN_DOORS_WHER	"You can find the Dungeon of Doors on the south end of Lynbore, on Castle Way. It is a large, grey building."
#define AN_DOORS_WHY	"The amulet is quite powerful and useful for preventing curses and hexes. The outlaws have no use for such a thing!"

#define AN_MINE1_WHO	"The mines collapsed many years ago. Those who go looking for riches flee from the walking skeletons they find!"
#define AN_MINE1_WHAT	"The silver ring adorned with a small ruby? It is a symbol of love for some people."
#define AN_MINE1_WHER	"You can find the Lynbore Mines on the north end of town, at the tip of Silver Avenue."
#define AN_MINE1_WHY	"I am not able to get one myself, but I wanted a ring so I could propose to someone."
#define AN_MINE1_HOW	"You will need a pick to mine walls. When you find the materials, you can use the ruby on the silver ring to put them together."

#define AN_STONE_WHAT	"The sword in that stone on the floor."
#define AN_STONE_WHER	"Hmm, let me think. Where could it be? Oh, right there on the floor!"
#define AN_STONE_WHY	"I've been staring at this sword for years. I'd have thrown the stone away and been done with it otherwise!"
#define AN_STONE_HOW	"If you're not strong enough, perhaps some form of enchantment will help. Why not check the magic shop in the library?"

#define AN_THIEF2_WHO	"The thieves to the back of the thieves house are stronger than the others."
#define AN_THIEF2_WHAT	"It's a small golden amulet adorned with a piecs of red agate."
#define AN_THIEF2_WHER	"The Thieves House is located on the west end of First Street, across the road to the north."
#define AN_THIEF2_WHY	"The thieves stole my amulet from me and are holding for ransom. But I'd much rather take it back."
#define AN_THIEF2_LOCK	"You will need to use a lock-pick to open the second door in the thieves house. They're very careful with the key."

#define AN_LIPOT_WHAT	"The Potion of Life is a powerful restorative potion. It is said to be quite powerful."
#define AN_LIPOT_WHER	"I don't think you can find the potion anywhere. You may need to mix one yourself."
#define AN_LIPOT_WHY	"I am very ill, %s. The Potion of Life may be the only cure."
#define AN_LIPOT_HOW	"You'll need a flask and five different flowers. There's an order you need to mix them, but I'm not sure myself."

#define AN_HEDGE_WHO	"The hedge maze is home to a few nasty creepers. They're big insects with pincers."
#define AN_HEDGE_WHAT	"It's a small buckler made of oak wood. It's light weight and easy to use."
#define AN_HEDGE_WHER	"The hedge maze is just next door, between Second Street and Shore Crescent. It's that big wall of bushes."
#define AN_HEDGE_WHY	"I made it myself a while back, but I lost it in the maze. It was clumsy of me."

#define AN_MMAZE_WHO	"The sorceress Jane is a powerful witch. She seeks to be more powerful, and lures unsuspecting adventurers to their doom."
#define AN_MMAZE_WHAT	"The weapon Jane is making, her 'Obliterator' she calls it, is a powerful sceptre. It enhances the Blast spell."
#define AN_MMAZE_WHER	"The Magic Maze is across the road, at the east end of Shore Crescent. It's up against the lake shore."
#define AN_MMAZE_WHY	"I want to stop Jane from hurting any more people. Take that weapon away from her by force!"
#define AN_MMAZE_HOW	"Jane hires other witches to help from time to time. They may have a way to navigate the maze on them."

#define AN_WOODS_WHO	"There have been bandit sightings in these woods, lead by their leader Argha. Be careful not to walk into an ambush."
#define AN_WOODS_WHO2	"The bandit leader Argha has evaded capture for a long time. The Guard Captain in Aston has a reward for proof of his defeat."
#define AN_WOODS_WHAT	"If not the body, Argha has a personally crafted knife like no other. Giving that to the Guard Captain would do."
#define AN_WOODS_WHER	"You can find Aston to the north east. It is home to the great Temple of Skua, and is the center of the world."
#define AN_WOODS_WHY	"Argha's inflience is the reason why there are so many thieves and outlaws lately. They emulate him and want fame and fortune."
#define AN_WOODS_HOW	"If you wish to visit Aston, travel straight east. The woods curve northward, after which you'll see Aston on the horizon."

#define AN_ARGHA_WHO	"The bandits in the Weeping Woods have been terrorizing and looting travellers. I'm surprised you hadn't encountered them!"
#define AN_ARGHA_WHO2	"The bandit leader Argha has evaded capture for a long time. I want him dead or alive."
#define AN_ARGHA_WHAT	"The reward for Argha's capture is 500 Gold. Proof of his death will yield 100 Gold."
#define AN_ARGHA_WHER	"Argha and his bandits can be found south, in the Weeping Woods. Somewhere between here and Lynbore, surely."
#define AN_ARGHA_WHY	"Argha has stolen from and killed many travellers and merchants. There is a lot of blood on his hands."

//}
//{ Aston
// Answers to common questions (in Aston)
// "Where is...?"

#define AN_AST_SKUA		"The Temple of Skua is located on Temple Street, to the east of Merchant's Way."
#define AN_AST_PURPLE 	"The Temple of The Purple One is located on Temple Street, to the west of Merchant's Way."
#define AN_AST_BANK 	"There are two banks in Aston. Both can be found on Merchant's Way."
#define AN_AST_SHOP 	"The Merchant's Square and other shops can be found between Merchant's Way and Marble Lane."
#define AN_AST_TEMSTR 	"Temple Street is in the northern part of Aston. It reaches from the far east to the far west."
#define AN_AST_SOUEND 	"The South End can be found at the southmost part of Aston."
#define AN_AST_ROSSTR 	"Rose Street can be found on the east side of Aston."
#define AN_AST_MARLAN 	"Marble Lane can be found in the middle of Aston."
#define AN_AST_MERWAY 	"Merchant's Way can be found in the middle of Aston. It extends between the north and southmost ends."
#define AN_AST_BULAVE 	"Bulwark Avenue can be found on the westmost end of Aston. It follows along the entire eastern wall."
#define AN_AST_GUAHOU	"The Guard House in Aston can be found at the southern entrance to Aston."
#define AN_AST_ASTON	"This city is named Aston, the capital of Astonia."

// Answers to quest questions (in Aston)
// WHO took it, WHAT was the item, WHERE is the quest, WHY do you want the item, and any other clues
#define AN_ARACH_WHO	"There have been sightings of large spiders in the park. Please be cautious!"
#define AN_ARACH_WHAT	"It is a small yellow tulip. It is different from other flowers."
#define AN_ARACH_WHER	"Aston Park is just behind my house, south of here. You may find the tulip underground there."
#define AN_ARACH_WHY	"I'm hoping to further my research into making potions."

#define AN_MANSION_WHO	"Be careful of the templar guards, and especially careful of the Butler!"
#define AN_MANSION_WHAT	"It is a kite shield made of ruby. It is a lovely shade of red."
#define AN_MANSION_WHER	"The mansion can be found behind my house, on Rose Street."
#define AN_MANSION_WHY	"The challenge and excitement, my friend!"

#define AN_BELL_WHO		"The Butcher and his victims haunt the Bell House. Be very careful!"
#define AN_BELL_WHAT	"The Butcher's weapon is a foul axe, originally for carving meat."
#define AN_BELL_WHER	"You can find the Bell House behind my house. There is a path outside which leads there."
#define AN_BELL_WHY		"The Butcher was once a serial killer and has harmed many people. I want to put a stop to it."

#define AN_WEBBED_WHO	"The bush is filled with Tarantulas... Their poison is quite lethal..."
#define AN_WEBBED_WHAT	"There's a big spider... It's fang is big enough to use as a weapon..."
#define AN_WEBBED_WHER	"The webbed bush... It's in the southeast corner of the Strange Forest... Straight south from the entrance..."
#define AN_WEBBED_WHY	"I want to... Make an antidote for the poison... I want to help heal people..."

#define AN_MINE2_WHO	"There are strong golems in the mines below the ground. They fight bare handed and hit quite hard."
#define AN_MINE2_WHAT	"A Potion of Golem, some call it. Drinking it is said to turn you into one temporarily."
#define AN_MINE2_WHER	"The Aston Mines are north of here, at the end of Merchant's Way. A ladder goes down to the lower floors."
#define AN_MINE2_WHY	"Why indeed... It is a very powerful potion, no doubt. I wish to see how strong, ha-ha!"

#define AN_PENITN_WHO	"Prisoners, Templars and Harakim... Together they may be formidible foes. The Warden himself? The strongest."
#define AN_PENITN_WHAT	"I'm looking for evidence, perhaps a note of some sort. A reason for his behavior."
#define AN_PENITN_WHER	"Here in the prison. His office is to the south-west, but it is locked. You may need to head east first."
#define AN_PENITN_WHY	"Well, clearly it's unwise to let inmates and traitors take over a prison."

#define AN_GROLMLB_WHO	"The grolms in the pentagram quest can get quite strong, but the grolms in the laboratory are even stronger."
#define AN_GROLMLB_WHAT	"An axe made of Ruby. Grolm aren't very smart, but they've been experimenting with different weapon materials."
#define AN_GROLMLB_WHER	"Deep in the Aston Mines, there is an entrance to the Pentagram Quest. Deeper still, you'll find the Laboratory."
#define AN_GROLMLB_WHY	"I would like to test this research myself and see just how intelligent the Grolms have or have not become."

#define AN_HERMIT_WHO	"The mad hermit has been alone with his thoughts for many years, but he is a powerful magician."
#define AN_HERMIT_WHAT	"A red rose. It is quite striking and different than other flowers."
#define AN_HERMIT_WHER	"In the southern part of the Strange Forest, you may come across a fenced house. It is there."
#define AN_HERMIT_WHY	"I am an alchemist, you see. I want to progress my studies."

#define AN_LAKEBED_WHO	"Powerful aquatic grolms called Seagrels can be found in the lake. Be careful not to drown!"
#define AN_LAKEBED_WHAT	"A glittering cleaver... It is a unique axe that is more comfortable when used in one's off-hand."
#define AN_LAKEBED_WHER	"In the southern part of the Strange Forest, there is a large lake. There should be a way underneath it."
#define AN_LAKEBED_WHY	"It is quite an odd weapon, and I'd like to add it to my collection."

#define AN_SEWER1_WHO	"Humans with rat heads... Ratlings live below the city in the underground. They see quite well in the dark!"
#define AN_SEWER1_WHAT	"I need ratling eyes to create a powerful stimulant. To better organize them, check Damor's shop for a red collector."
#define AN_SEWER1_WHER	"There are many holes around town which may lead to the underground. There is one behind my house, in fact!"
#define AN_SEWER1_WHY	"The potion I create will be very powerful. I will be able to create two potions, and give you one as payment."

#define AN_SEWER2_WHO	"Scaley humanoids with rat-like heads... Greenlings live in massive swarms deep below the city, deeper than ratlings."
#define AN_SEWER2_WHAT	"I need greenling eyes to create a powerful drug. To keep them organized, check Damor's magic shop for a green collector."
#define AN_SEWER2_WHER	"There is one entrance I know of, below the Temple of the Purple One. It casts an eerie green glow."
#define AN_SEWER2_WHY	"The potion I create will be very powerful. I will be able to create two potions, and give you one as payment."

#define AN_GARG_WHO		"Gargoyles - striking red figures with glowing wings. They wield longswords made of hardened magma."
#define AN_GARG_WHAT	"I heard the gargoyles are quite protective of eggs made of living fire. Perhaps it is kept in the deepest reach of their nest."
#define AN_GARG_WHER	"The Gargoyle Nest can be found in the Strange Forest, north of the river which splits the northern half."
#define AN_GARG_WHY		"I would use it to make a powerful Fire Cloak, providing strong defenses. It is prone to burning itself away, however."

#define AN_CANYON_WHO	"There are many bandits who have taken residence in the canyon. They're tougher than the ones in the woods."
#define AN_CANYON_WHAT	"The Templar Heater Shield is a strong shield, well, made for Templars. It is hardier than most other shields."
#define AN_CANYON_WHER	"The canyon is tough to reach on foot, but some say there's a tunnel leading there underground. It's to the South."
#define AN_CANYON_WHY	"The shield is very rare and only a few were made years ago. I want to add it to my collection."

#define AN_PASS_WHO		"More bandits, they say. Some stronger than others."
#define AN_PASS_WHAT	"A golden helmet adorned with a large sapphire. Must I repeat myself?"
#define AN_PASS_WHER	"It is found to the south-east of the forgotten canyon. The canyon can be found through a tunnel in the underground."
#define AN_PASS_WHY		"It was a gift from my late father many years ago. The bandits stole it from me, and I'd like it back if you must know."

#define AN_ARCHIVE_WHO	"Terrible spectres took refuge in that old archive. Some say they're the reason it was abandoned..."
#define AN_ARCHIVE_WHAT	"It's a rare book, the traveller's guide. It contains the records of the many wonders of the world."
#define AN_ARCHIVE_WHER	"The archives are south-west of the forgotten canyon. The canyon can be found through a tunnel in the underground."
#define AN_ARCHIVE_WHY	"Well, I'm a writer you see. I'd like some new ideas for writing, and the book would help me greatly."

#define AN_MINE3_WHO	"In the lowest floor of the mines, you may find gargoyles made of living magma. They can be quite the handful."
#define AN_MINE3_WHAT	"It is a small statuette of a miniature gargoyle. Some say you can use it to enthrall a life-size gargoyle of your own."
#define AN_MINE3_WHER	"The mines can be found north of here, at the end of Merchant's Way. Twice down the ladder you'll find the lowest floor."
#define AN_MINE3_WHY	"I'm something of a... collector of sorts. No need for further details than that."

#define AN_SWAMP_WHO	"Dreadful Swamp Lizards live in the sothern swamp. Some say they are quite intelligent."
#define AN_SWAMP_WHAT	"The Rattan Bo, is-- well, it's a polished staff made of a special wood. Quite good for hitting things."
#define AN_SWAMP_WHER	"The Southern Swamp can be found in the south-west corner of the Strange Forest. It's a long journey."
#define AN_SWAMP_WHY	"Why shouldn't I want something a different species crafted? It's quite facinating."

#define AN_MOUNT_WHO	"Some say that a tribe of Vikings have taken refuge in the mountains. They must be very strong to live there."
#define AN_MOUNT_WHAT	"An amulet in the shape of a star. It is quite powerful."
#define AN_MOUNT_WHER	"The Northern Mountains can be found in the north-west corner of the Strange Forest."
#define AN_MOUNT_WHY	"To some, the Star Amulet is a symbol of passion and victory. A fitting request for a fitting reward."

#define AN_DIAMOND_WHO	"I'd be careful of Gargoyles in the mines, if I were you. They're a scary lot."
#define AN_DIAMOND_WHAT	"Exactly as it sounds, a Golden Ring adorned with a Huge Diamond. Both materials are quite hard to come by."
#define AN_DIAMOND_WHER	"The mines are north of Merchant's Way. A ladder goes down to the lower floors."
#define AN_DIAMOND_WHY	"It is a gift for my fiance, my future bride to be."

#define AN_HERBS_WHO	"Greenlings, mutated ratlings of sorts, live in the Underground. They're quite grotesque."
#define AN_HERBS_WHAT	"The green plants amidst the greenlings, I suspect are the cause of their mutation."
#define AN_HERBS_WHER	"The plants can be found deep underground amongst the greenlings. I suspect there may be various types of them."
#define AN_HERBS_WHY	"I am researching precicely what greenlings are, and if there's a way to weaken their brood."

#define AN_TARGE_WHO	"Aye, gargoyles, a deadly sort. Dey get stronger ta deeper ye go."
#define AN_TARGE_WHAT	"It's un ancient shield passed down through ta royal family. How it ended up thar is anyone's guess."
#define AN_TARGE_WHER	"Te Gargoyle's Nest is to te north of te Strange Forest. Ye might see a wall o' reddened dirt. It's thar."
#define AN_TARGE_WHY	"I be a craftsman by trade, an' ayed like to learn a ting or two from te crafters o' old."

#define AN_SEWER3_WHO	"Ugly aquatic rats... Soglings live deep underground... Very, very deep..."
#define AN_SEWER3_WHAT	"Sogling eyes can make an ultra strong drug... Damor's magic shop has a collector..."
#define AN_SEWER3_WHER	"Somewhere... um... in the first part of the greenling's den..."
#define AN_SEWER3_WHY	"The potion will be very powerful... I can make two, and give you one..."

// 
#define AN_BS_HELP		"Every few hours a horde of monsters from the stronghold attacks our outposts and gates in the Strange Forest. If you could help PROTECT these places - or enter the STRONGHOLD to stop the monsters - we would REWARD you."
#define AN_BS_PROT		"Wait near one of the three gates, or the outposts beyond, until the guards shout alert. Then, help them in the fight. They will report your success as POINTS, and you can collect a REWARD here afterward."
#define AN_BS_REWA		"Depending on your success in fending off the monsters, you can purchase money, potions, experience, or other items. Ask about your POINTS for more information."
#define AN_BS_POIN		"You have %d points. Say TABLE and I will tell you everything you can purchase."
#define AN_BS_STRO		"The monsters come from various parts of the Black Stronghold from the NORTH, CENTER, and SOUTH. As far as we know, this magic uses black candles. If you could bring us these black candles, we will REWARD you further."
#define AN_BS_NORT		"The north gate is the closest to Aston, and fortunately our reports indicate these monsters are weakest. If you are unsure of yourself, this would be a good place to start assisting us."
#define AN_BS_CENT		"The center gate leads into the Black Stronghold proper, and our reports indicate these monsters are of middling power. If you wish to enter the Black Stronghold, this is where you can approach."
#define AN_BS_SOUT		"The south gate is the farthest from Aston, but our repors indicate these monsters are extremely strong. You should avoid approach, but if you assist us here you will be greatly REWARDed."
//
//}
//{ Labyrinth Answers
#define AN_LAB_HELP		"I can help with DOORS, KEYS, and RIDDLES. Let me know!"
#define AN_LAB_WHO		"Who am I...? Ah, just a tired old man. Don't you worry about me just yet..."

#define AN_LAB1_DOORS	"There are three doors in this part of the labyrinth. They are in the south-east corner."
#define AN_LAB1_KEYS	"There are three keys in this part of the labyrinth. One is a weapon, and one is a crown."
#define AN_LAB1_RIDDLES	"Hmm, let me think... 'Behind the waiting soldier sleeps a key wreathed by flame'."

#define AN_LAB2_DOORS	"There are three doors in this gorge. As you can see, they're right beside me!"
#define AN_LAB2_KEYS	"There are three keys in this gorge. Speak with the merchants, they have some quests for you!"
#define AN_LAB2_RIDDLES	"There are no riddles here, but the merchants each seek help!"
#define AN_LAB2_COCO	"The coconut? Well, coconuts grow in trees, you know?"
#define AN_LAB2_POTION	"You have to mix the Potion of Superior Agility from some flower which grow here."
#define AN_LAB2_TEETH	"Lizard's Teeth? I'd assume they have them in their mouths."
#define AN_LAB2_NECK	"To create the Lizard's Teeth Necklace, you need a leather string and nine teeth."

#define AN_LAB3_DOORS	"There is only one door in this part of the labyrinth. It is down the hall to the south."
#define AN_LAB3_KEYS	"There is only one key in this part of the labyrinth. It is guarded by an evil presense."
#define AN_LAB3_RIDDLES	"Some scribbles of a poor grave digger lie here. Some are directions, some are... mis-directions."

#define AN_LAB4_DOORS	"There are three doors in this gorge. They are right beside me!"
#define AN_LAB4_KEYS	"There are three keys is in this gorge. Each is held by a different master, each puppeteering their court."
#define AN_LAB4_RIDDLES	"Hmm, let me think... 'Six fool's tool's tools, yet far more than you need'."

#define AN_LAB5_DOORS	"There are two doors in this gorge. They are right beside me!"
#define AN_LAB5_KEYS	"There are two keys in this gorge. One is held by a mad knight, the other is held by a mad knight."
#define AN_LAB5_RIDDLES	"A mage wanders these halls, and perhaps he has the riddles you seek. But beware, he is prone to anger!"
#define AN_LAB5_POEM1	"Where the sun rises in a clouded sky     A mere touch will reveal where hidden lie       A skull which, when enlighted, will give thee    A brave enemy and a precious key."
#define AN_LAB5_POEM2	"A dark sky, a large tree                 Weavers work in crimson                         The corner of which guides thee                  To key's holder, the lord's son."

#define AN_LAB6_DOORS	"There are two doors in this gorge. They can be found to the west of here."
#define AN_LAB6_KEYS	"There are two keys in this gorge. Each is held by a person in need of help."
#define AN_LAB6_RIDDLES	"Hmm, let me think... 'The pharoh is guarded by two hollow eyes and an oasis'."

#define AN_LAB7_DOORS	"There is a single door in this gorge. It can be found at the end of the hall to the north."
#define AN_LAB7_KEYS	"There is a single key in this gorge. It is held by an absurdly powerful Undead Lord!"
#define AN_LAB7_RIDDLES	"Hmm, let me think... 'Light makes it weep, dark makes it strong; The skeletal staff sings a death-sealing song'."

#define AN_LAB8_DOORS	"There is a single door in this gorge. It can be found at the end of the hall to the south."
#define AN_LAB8_KEYS	"There is a single key in this gorge. It is guarded by a pair of cautious gargoyles."
#define AN_LAB8_RIDDLES	"Hmm, let me think... 'Heart made of stone, brain made of kelp; though as deadly as steel, these seafarers are great help'."

#define AN_LAB9_DOORS	"There are several locked doors, too many to count. Some keys are hidden, others are in plain sight."
#define AN_LAB9_KEYS	"There are several keys hiding in ridiculous places. The librarians are quite absent-minded!"
#define AN_LAB9_RIDDLES	"Yes, this is the Riddle Gorge after all!"

#define AN_LAB0_DOORS	"There are four locked doors, each a striking color with a matching colored key."
#define AN_LAB0_KEYS	"There are four keys to match the doors. But you'll find they're scattered in more places than four!"
#define AN_LAB0_RIDDLES	"Hmm, let me think... 'A fool falls for a trap. A wise man looks for them'."

#define AN_LABX_DOORS	"There are five locked doors, four of which are simple and one more complex."
#define AN_LABX_KEYS	"There are four keys, yet one final object when united will act as the last one."
#define AN_LABX_RIDDLES	"Hmm, let me think... 'Four fiends filled and felled with folly, seasonal sorcerers seek senseless strength'."

#define AN_LABY_DOORS	"There are five locked doors, each splitting the ascent into three tribes."
#define AN_LABY_KEYS	"There are five keys, held separate and shared by the lords of this mountain."
#define AN_LABY_RIDDLES	"Hmm, let me think... 'The brute, the fiend, the wisened monster. Apart they are strong, together they are stronger'."

#define AN_LABZ_DOORS	"There is a single door here, but you may never wish to see it!"
#define AN_LABZ_KEYS	"There is a single key. It's in my back pocket... But alas, you can't have it here."
#define AN_LABZ_RIDDLES	"There are no more riddles, I'm afraid. Simply a choice laid bare and simple."
#define AN_LABZ_SEYAN	"The Seyan'du are powerful people who learn every basic skill. You need to start from the beginning if you become one."
#define AN_LABZ_GATE	"Yes, the gate to the east. That blue-ish square with the gold trim. You'll need to leave your posessions behind!"
#define AN_LABZ_SWORD	"The Sword of a Seyan'du starts rather plain, but can outclass any other weapon through trial and experience."
#define AN_LABZ_KWAI	"Kwai, the goddess of war. She birthed the tribe of Seyan'du long, long ago in her quest to vanquish The Purple One."
#define AN_LABZ_SKILLS	"Each advanced class can learn unique skills that a Seyan'du will never know. Some are quite powerful!"
#define AN_LABZ_SHRINE	"Yes, the gold shrine against the north wall over there. If you wish, present a given potion as the notes say."
#define AN_LABZ_TEMPL	"Arch Templars will learn how to warcry and dual wield weapons, and may eventually learn to hit armies in a single slash."
#define AN_LABZ_BRAWL	"Brawlers will learn how to deal a volley of critical hits, and may eventually learn to copy their very shadow to assist them."
#define AN_LABZ_WARRI	"Warriors learn the Haste spell, dual wield, and several templar skills. They can learn to hit surrounding foes more quickly."
#define AN_LABZ_SORCE	"Sorcerers learn the Haste spell, poison, and several harakim skills. They can learn to curse armies in a single word."
#define AN_LABZ_SUMMO	"Summoners will gain greater mastery over their companions, and may eventually learn to copy their very shadow to assist them."
#define AN_LABZ_HARAK	"Arch Harakim will learn how to concentrate and poison foes, and may eventually learn to blast armies in a single word."

//}
//{ Special answers
// Lab 9 generic riddle response (from vanilla)
#define AN_RIDDLE		"If you bring me the right volume of the Book of Wisdom, I will tell you a riddle. To answer the riddle, just say the word the riddle asks for. If you answer correctly, I will bring you to the next stage of your quest."

// "Special Answer" Numbers
#define SP_HEALTH    	 1
#define SP_SHOP      	 2
#define SP_GREET     	 3
#define SP_WHOAMI    	 4
#define SP_WHERE     	 5
#define SP_STOP      	 6
#define SP_MOVE      	 7
#define SP_ATTACK    	 8
#define SP_WAIT      	 9
#define SP_FOLLOW    	10
#define SP_TIME      	11
#define SP_POINTS    	12
#define SP_TABLE	   	13
#define SP_TRANSFER  	14
#define SP_SPELLINFO 	15
#define SP_QUIET     	16
#define SP_QUEST		17
#define SP_DEFENSE		18
#define SP_PASSIVE		19
#define SP_OFFENSE		20
#define SP_BERSERK		21
#define SP_COMMAND		22
#define SP_MOVE_N      	23
#define SP_MOVE_E      	24
#define SP_MOVE_S      	25
#define SP_MOVE_W      	26
#define SP_TAROT		27
#define SP_TAROT2		28

// Stronghold
#define SP_BUY_GOLD    		601
#define SP_BUY_EXP    		602
#define SP_BUY_HEAL 		603
#define SP_BUY_MANA   		604
#define SP_BUY_END   		605
#define SP_BUY_GHEAL 		606
#define SP_BUY_GMANA   		607
#define SP_BUY_GEND   		608
#define SP_BUY_RATLING 		609
#define SP_BUY_GREENLING   	610
#define SP_BUY_DREADLING   	611
#define SP_BUY_WEAPON1   	612
#define SP_BUY_WEAPON2   	613
#define SP_BUY_WEAPON3   	614
#define SP_BUY_HELMET   	615
#define SP_BUY_ARMOR	   	616

//}

// Knowledge table - the big one that handles everything
struct know know[] = {
	
	//{ Lynbore
	{{"!quest", NULL}, 100, AR_LYNBOR, 0, NULL, SP_QUEST},
	// Key words ................................... , Dif,      Area, Tmp,         Answer, Spc		----- Lynbore stuff -----
	{{"!where",  "?bluebird", "!tavern",   "?", NULL},   0, AR_LYNBOR,   0, AN_LYN_TAVERN ,   0},
	{{"!where",  "!thief",    "?house",    "?", NULL},   0, AR_LYNBOR,   0, AN_LYN_THIEF  ,   0},
	{{"!where",  "?lynbore",  "!park",     "?", NULL},   0, AR_LYNBOR,   0, AN_LYN_PARK   ,   0},
	{{"!where",  "?cursed",   "!tomb",     "?", NULL},   0, AR_LYNBOR,   0, AN_LYN_TOMB   ,   0},
	{{"!where",  "?lynbore",  "!library",  "?", NULL},   0, AR_LYNBOR,   0, AN_LYN_LIBRARY,   0},
	{{"!where",  "!joe",      "?house",    "?", NULL},   0, AR_LYNBOR,   0, AN_LYN_JOE    ,   0},
	{{"!where",  "?templar",  "!outlaw",   "?", NULL},   0, AR_LYNBOR,   0, AN_LYN_OUTLAW ,   0},
	{{"!where",  "?skeleton", "!lord",     "?", NULL},   0, AR_LYNBOR,   0, AN_LYN_SKLORD ,   0},
	{{"!where",  "?haunted",  "!castle",   "?", NULL},   0, AR_LYNBOR,   0, AN_LYN_CASTLE ,   0},
	{{"!where",  "!dungeon",  "!door",     "?", NULL},   0, AR_LYNBOR,   0, AN_LYN_DOORS  ,   0},
	{{"!where",  "!magic",    "!maze",     "?", NULL},   0, AR_LYNBOR,   0, AN_LYN_MMAZE  ,   0},
	{{"!where",  "!hedge",    "?maze",     "?", NULL},   0, AR_LYNBOR,   0, AN_LYN_HEDGE  ,   0},
	{{"!where",  "!mine",     "?",              NULL},   0, AR_LYNBOR,   0, AN_LYN_MINE1  ,   0},
	{{"!where",  "?weeping",  "!woods",    "?", NULL},   0, AR_LYNBOR,   0, AN_LYN_WOODS  ,   0},
	{{"!where",  "!spider",   "!den",      "?", NULL},  55, AR_LYNBOR,   0, AN_LYN_SPIDERS,   0},
	{{"!where",  "?abandoned","!manor",    "?", NULL},  60, AR_LYNBOR,   0, AN_LYN_MANOR  ,   0},
	{{"!where",  "!bank",     "?",              NULL},   0, AR_LYNBOR,   0, AN_LYN_BANK   ,   0},
	{{"!where",  "!shop",     "?",              NULL},   0, AR_LYNBOR,   0, AN_LYN_SHOP   ,   0},
	{{"!where",  "!buy",      "?",              NULL},   0, AR_LYNBOR,   0, AN_LYN_SHOP   ,   0},
	{{"!where",  "!sell",     "?",              NULL},   0, AR_LYNBOR,   0, AN_LYN_SHOP   ,   0},
	{{"!where",  "!shore",    "!crescent", "?", NULL},   0, AR_LYNBOR,   0, AN_LYN_SHOCRE ,   0},
	{{"!where",  "!first",    "!street",   "?", NULL},   0, AR_LYNBOR,   0, AN_LYN_FIRSTR ,   0},
	{{"!where",  "!second",   "!street",   "?", NULL},   0, AR_LYNBOR,   0, AN_LYN_SECSTR ,   0},
	{{"!where",  "!silver",   "!avenue",   "?", NULL},   0, AR_LYNBOR,   0, AN_LYN_SILAVE ,   0},
	{{"!where",  "!castle",   "!way",      "?", NULL},   0, AR_LYNBOR,   0, AN_LYN_CASWAY ,   0},
	{{"!where",  "!rose",     "!lane",     "?", NULL},   0, AR_LYNBOR,   0, AN_LYN_ROSLAN ,   0},
	{{"!where",  "!guard",    "?house",    "?", NULL},   0, AR_LYNBOR,   0, AN_LYN_GUAHOU ,   0},
	{{"?where",  "?what",     "!lynbore",  "?", NULL},   0, AR_LYNBOR,   0, AN_LYN_LYNBORE,   0},
	// Key words ................................... , Dif,      Area, Tmp,         Answer, Spc		AR_THIEF1
	{{"!danger", "!thief",    "?house",    "?", NULL},   0, AR_THIEF1,   0, AN_THIEF1_WHO ,   0},
	{{"!tell",   "!thief",    "?house",    "?", NULL},   0, AR_THIEF1,   0, AN_THIEF1_WHO ,   0},
	{{"!who",    "!thief",    "?house",    "?", NULL},   0, AR_THIEF1,   0, AN_THIEF1_WHO ,   0},
	{{"!what",   "!amulet",   "?",              NULL},   0, AR_THIEF1,   0, AN_THIEF1_WHAT,   0},
	{{"!tell",   "?about",    "!amulet",   "?", NULL},   0, AR_THIEF1,   0, AN_THIEF1_WHAT,   0},
	{{"!where",  "!thief",    "?house",    "?", NULL},   0, AR_THIEF1,   0, AN_THIEF1_WHER,   0},
	{{"!why",    "?want",     "!amulet",   "?", NULL},   0, AR_THIEF1,   0, AN_THIEF1_WHY ,   0},
	{{"!locked", "!door",     "?",              NULL},   0, AR_THIEF1,   0, AN_THIEF1_LOK1,   0},
	{{"!second", "!door",     "?",              NULL},   0, AR_THIEF1,   0, AN_THIEF1_LOK2,   0},
	// Key words ................................... , Dif,      Area, Tmp,         Answer, Spc		AR_PARK
	{{"!danger", "?lynbore",  "!park",     "?", NULL},   0, AR_PARK,     0, AN_PARK_WHO ,     0},
	{{"!tell",   "?lynbore",  "!park",     "?", NULL},   0, AR_PARK,     0, AN_PARK_WHO ,     0},
	{{"!who",    "?lynbore",  "!park",     "?", NULL},   0, AR_PARK,     0, AN_PARK_WHO ,     0},
	{{"!what",   "!dagger",   "?",              NULL},   0, AR_PARK,     0, AN_PARK_WHAT,     0},
	{{"!tell",   "?about",    "!dagger",   "?", NULL},   0, AR_PARK,     0, AN_PARK_WHAT,     0},
	{{"!where",  "?lynbore",  "!park",     "?", NULL},   0, AR_PARK,     0, AN_PARK_WHER,     0},
	{{"!why",    "?want",     "!dagger",   "?", NULL},   0, AR_PARK,     0, AN_PARK_WHY ,     0},
	// Key words ................................... , Dif,      Area, Tmp,         Answer, Spc		AR_TOMB
	{{"!danger", "?cursed",   "!tomb",     "?", NULL},   0, AR_TOMB,     0, AN_TOMB_WHO ,     0},
	{{"!tell",   "?cursed",   "!tomb",     "?", NULL},   0, AR_TOMB,     0, AN_TOMB_WHO ,     0},
	{{"!who",    "?cursed",   "!tomb",     "?", NULL},   0, AR_TOMB,     0, AN_TOMB_WHO ,     0},
	{{"!what",   "?ancient",  "!weapon",   "?", NULL},   0, AR_TOMB,     0, AN_TOMB_WHAT,     0},
	{{"!tell",   "?about",    "!weapon",   "?", NULL},   0, AR_TOMB,     0, AN_TOMB_WHAT,     0},
	{{"!where",  "?cursed",   "!tomb",     "?", NULL},   0, AR_TOMB,     0, AN_TOMB_WHER,     0},
	{{"!why",    "?want",     "!weapon",   "?", NULL},   0, AR_TOMB,     0, AN_TOMB_WHY ,     0},
	{{"!help",   "?with",     "!tomb",     "?", NULL},   0, AR_TOMB,     0, AN_TOMB_WALL,     0},
	{{"!where",  "?ancient",  "!weapon",   "?", NULL},   0, AR_TOMB,     0, AN_TOMB_WALL,     0},
	// Key words ................................... , Dif,      Area, Tmp,         Answer, Spc		AR_LIBRY
	{{"!danger", "?lynbore",  "!library",  "?", NULL},   0, AR_LIBRY,    0, AN_LIBRY_WHO ,    0},
	{{"!tell",   "?lynbore",  "!library",  "?", NULL},   0, AR_LIBRY,    0, AN_LIBRY_WHO ,    0},
	{{"!who",    "?lynbore",  "!library",  "?", NULL},   0, AR_LIBRY,    0, AN_LIBRY_WHO ,    0},
	{{"!what",   "!belt",                  "?", NULL},   0, AR_LIBRY,    0, AN_LIBRY_WHAT,    0},
	{{"!tell",   "?about",    "!belt",     "?", NULL},   0, AR_LIBRY,    0, AN_LIBRY_WHAT,    0},
	{{"!where",  "?lynbore",  "!library",  "?", NULL},   0, AR_LIBRY,    0, AN_LIBRY_WHER,    0},
	{{"!why",    "?want",     "!belt",     "?", NULL},   0, AR_LIBRY,    0, AN_LIBRY_WHY ,    0},
	{{"!locked", "!door",     "?",              NULL},   0, AR_LIBRY,    0, AN_LIBRY_LOCK,    0},
	// Key words ................................... , Dif,      Area, Tmp,         Answer, Spc		AR_JOE
	{{"!danger", "?joe",      "?house",    "?", NULL},   0, AR_JOE,      0, AN_JOE_WHO ,      0},
	{{"!tell",   "?joe",      "?house",    "?", NULL},   0, AR_JOE,      0, AN_JOE_WHO ,      0},
	{{"!who",    "?joe",      "?house",    "?", NULL},   0, AR_JOE,      0, AN_JOE_WHO ,      0},
	{{"!what",   "!armor",                 "?", NULL},   0, AR_JOE,      0, AN_JOE_WHAT,      0},
	{{"!tell",   "?about",    "!armor",    "?", NULL},   0, AR_JOE,      0, AN_JOE_WHAT,      0},
	{{"!where",  "?joe",      "?house",    "?", NULL},   0, AR_JOE,      0, AN_JOE_WHER,      0},
	{{"!why",    "?want",     "!armor",    "?", NULL},   0, AR_JOE,      0, AN_JOE_WHY ,      0},
	// Key words ................................... , Dif,      Area, Tmp,         Answer, Spc		AR_OUTLAW
	{{"!danger", "?templar",  "!outlaws",  "?", NULL},   0, AR_OUTLAW,   0, AN_OUTLAW_WHO ,   0},
	{{"!tell",   "?templar",  "!outlaws",  "?", NULL},   0, AR_OUTLAW,   0, AN_OUTLAW_WHO ,   0},
	{{"!who",    "?templar",  "!outlaws",  "?", NULL},   0, AR_OUTLAW,   0, AN_OUTLAW_WHO ,   0},
	{{"!what",   "!sword",                 "?", NULL},   0, AR_OUTLAW,   0, AN_OUTLAW_WHAT,   0},
	{{"!tell",   "?about",    "!sword",    "?", NULL},   0, AR_OUTLAW,   0, AN_OUTLAW_WHAT,   0},
	{{"!where",  "?templar",  "!outlaws",  "?", NULL},   0, AR_OUTLAW,   0, AN_OUTLAW_WHER,   0},
	{{"!why",    "?want",     "!sword",    "?", NULL},   0, AR_OUTLAW,   0, AN_OUTLAW_WHY ,   0},
	// Key words ................................... , Dif,      Area, Tmp,         Answer, Spc		AR_SKLORD
	{{"!danger", "?skeleton", "!lord",     "?", NULL},   0, AR_SKLORD,   0, AN_SKLORD_WHO ,   0},
	{{"!tell",   "?skeleton", "!lord",     "?", NULL},   0, AR_SKLORD,   0, AN_SKLORD_WHO ,   0},
	{{"!who",    "?skeleton", "!lord",     "?", NULL},   0, AR_SKLORD,   0, AN_SKLORD_WHO ,   0},
	{{"!what",   "!sword",                 "?", NULL},   0, AR_SKLORD,   0, AN_SKLORD_WHAT,   0},
	{{"!tell",   "?about",    "!sword",    "?", NULL},   0, AR_SKLORD,   0, AN_SKLORD_WHAT,   0},
	{{"!where",  "?skeleton", "!lord",     "?", NULL},   0, AR_SKLORD,   0, AN_SKLORD_WHER,   0},
	{{"!why",    "?want",     "!sword",    "?", NULL},   0, AR_SKLORD,   0, AN_SKLORD_WHY ,   0},
	// Key words ................................... , Dif,      Area, Tmp,         Answer, Spc		AR_CASTLE
	{{"!danger", "?haunted",  "!castle",   "?", NULL},   0, AR_CASTLE,   0, AN_CASTLE_WHO ,   0},
	{{"!tell",   "?haunted",  "!castle",   "?", NULL},   0, AR_CASTLE,   0, AN_CASTLE_WHO ,   0},
	{{"!who",    "?haunted",  "!castle",   "?", NULL},   0, AR_CASTLE,   0, AN_CASTLE_WHO ,   0},
	{{"!what",   "!sword",                 "?", NULL},   0, AR_CASTLE,   0, AN_CASTLE_WHAT,   0},
	{{"!what",   "!spellblade",            "?", NULL},   0, AR_CASTLE,   0, AN_CASTLE_WHAT,   0},
	{{"!tell",   "?about",    "!sword",    "?", NULL},   0, AR_CASTLE,   0, AN_CASTLE_WHAT,   0},
	{{"!tell",   "?about",   "!spellblade","?", NULL},   0, AR_CASTLE,   0, AN_CASTLE_WHAT,   0},
	{{"!where",  "?haunted",  "!castle",   "?", NULL},   0, AR_CASTLE,   0, AN_CASTLE_WHER,   0},
	{{"!why",    "?want",     "!sword",    "?", NULL},   0, AR_CASTLE,   0, AN_CASTLE_WHY ,   0},
	{{"!why",    "?want",   "!spellblade", "?", NULL},   0, AR_CASTLE,   0, AN_CASTLE_WHY ,   0},
	{{"!locked", "!door",     "?",              NULL},   0, AR_CASTLE,   0, AN_CASTLE_LOCK,   0},
	{{"!who",    "!azrael",   "?", 				NULL},   0, AR_CASTLE,   0, AN_CASTLE_AZR1,   0},
	{{"!tell",   "?about",    "!azrael",   "?", NULL},   0, AR_CASTLE,   0, AN_CASTLE_AZR1,   0},
	{{"!where",  "!azrael",   "?", 				NULL},   0, AR_CASTLE,   0, AN_CASTLE_AZR2,   0},
	{{"!what",   "!helmet",   "?", 				NULL},   0, AR_CASTLE,   0, AN_CASTLE_AZR3,   0},
	{{"!tell",   "?about",    "!helmet",   "?", NULL},   0, AR_CASTLE,   0, AN_CASTLE_AZR3,   0},
	// Key words ................................... , Dif,      Area, Tmp,         Answer, Spc		AR_DOORS
	{{"!danger", "!dungeon",  "?door",     "?", NULL},   0, AR_DOORS,    0, AN_DOORS_WHO ,    0},
	{{"!tell",   "!dungeon",  "?door",     "?", NULL},   0, AR_DOORS,    0, AN_DOORS_WHO ,    0},
	{{"!who",    "!dungeon",  "?door",     "?", NULL},   0, AR_DOORS,    0, AN_DOORS_WHO ,    0},
	{{"!what",   "!amulet",                "?", NULL},   0, AR_DOORS,    0, AN_DOORS_WHAT,    0},
	{{"!tell",   "?about",    "!amulet",   "?", NULL},   0, AR_DOORS,    0, AN_DOORS_WHAT,    0},
	{{"!where",  "!dungeon",  "?door",     "?", NULL},   0, AR_DOORS,    0, AN_DOORS_WHER,    0},
	{{"!why",    "?want",     "!amulet",   "?", NULL},   0, AR_DOORS,    0, AN_DOORS_WHY ,    0},
	// Key words ................................... , Dif,      Area, Tmp,         Answer, Spc		AR_MINE1
	{{"!danger", "?lynbore",  "!mine",     "?", NULL},   0, AR_MINE1,    0, AN_MINE1_WHO ,    0},
	{{"!tell",   "?lynbore",  "!mine",     "?", NULL},   0, AR_MINE1,    0, AN_MINE1_WHO ,    0},
	{{"!who",    "?lynbore",  "!mine",     "?", NULL},   0, AR_MINE1,    0, AN_MINE1_WHO ,    0},
	{{"!what",   "!ring",                  "?", NULL},   0, AR_MINE1,    0, AN_MINE1_WHAT,    0},
	{{"!tell",   "?about",    "!ring",     "?", NULL},   0, AR_MINE1,    0, AN_MINE1_WHAT,    0},
	{{"!where",  "?lynbore",  "!mine",     "?", NULL},   0, AR_MINE1,    0, AN_MINE1_WHER,    0},
	{{"!why",    "?want",     "!ring",     "?", NULL},   0, AR_MINE1,    0, AN_MINE1_WHY ,    0},
	{{"!how",    "!mine",     "?walls",    "?", NULL},   0, AR_MINE1,    0, AN_MINE1_HOW ,    0},
	{{"!how",    "!make",     "?ring",     "?", NULL},   0, AR_MINE1,    0, AN_MINE1_HOW ,    0},
	// Key words ................................... , Dif,      Area, Tmp,         Answer, Spc		AR_STONE
	{{"!what",   "!sword",                 "?", NULL},   0, AR_STONE,    0, AN_STONE_WHAT,    0},
	{{"!tell",   "?about",    "!sword",    "?", NULL},   0, AR_STONE,    0, AN_STONE_WHAT,    0},
	{{"!where",  "?stone",    "?sword",    "?", NULL},   0, AR_STONE,    0, AN_STONE_WHER,    0},
	{{"!why",    "?want",     "!amulet",   "?", NULL},   0, AR_STONE,    0, AN_STONE_WHY ,    0},
	{{"!how",    "?get",      "?sword",    "?", NULL},   0, AR_STONE,    0, AN_STONE_HOW ,    0},
	{{"!how",    "?take",     "?sword",    "?", NULL},   0, AR_STONE,    0, AN_STONE_HOW ,    0},
	{{"?not",    "?strong",   "?enough",   "?", NULL},   0, AR_STONE,    0, AN_STONE_HOW ,    0},
	{{"?too",    "?weak",                  "?", NULL},   0, AR_STONE,    0, AN_STONE_HOW ,    0},
	// Key words ................................... , Dif,      Area, Tmp,         Answer, Spc		AR_THIEF2
	{{"!danger", "!thief",    "?house",    "?", NULL},   0, AR_THIEF2,   0, AN_THIEF2_WHO ,   0},
	{{"!tell",   "!thief",    "?house",    "?", NULL},   0, AR_THIEF2,   0, AN_THIEF2_WHO ,   0},
	{{"!who",    "!thief",    "?house",    "?", NULL},   0, AR_THIEF2,   0, AN_THIEF2_WHO ,   0},
	{{"!what",   "!amulet",   "?",              NULL},   0, AR_THIEF2,   0, AN_THIEF2_WHAT,   0},
	{{"!tell",   "?about",    "!amulet",   "?", NULL},   0, AR_THIEF2,   0, AN_THIEF2_WHAT,   0},
	{{"!where",  "!thief",    "?house",    "?", NULL},   0, AR_THIEF2,   0, AN_THIEF2_WHER,   0},
	{{"!why",    "?want",     "!amulet",   "?", NULL},   0, AR_THIEF2,   0, AN_THIEF2_WHY ,   0},
	{{"!locked", "!door",     "?",              NULL},   0, AR_THIEF2,   0, AN_THIEF2_LOCK,   0},
	// Key words ................................... , Dif,      Area, Tmp,         Answer, Spc		AR_LIPOT
	{{"!what",   "!potion",   "?",              NULL},   0, AR_LIPOT,    0, AN_LIPOT_WHAT,    0},
	{{"!tell", "?about","?life", "!potion","?", NULL},   0, AR_LIPOT,    0, AN_LIPOT_WHAT,    0},
	{{"!tell", "?about","!potion", "?life","?", NULL},   0, AR_LIPOT,    0, AN_LIPOT_WHAT,    0},
	{{"!where",  "?life",     "!potion",   "?", NULL},   0, AR_LIPOT,    0, AN_LIPOT_WHER,    0},
	{{"!where",  "!potion",   "?life",     "?", NULL},   0, AR_LIPOT,    0, AN_LIPOT_WHER,    0},
	{{"!why",    "?want",     "!dagger",   "?", NULL},   0, AR_LIPOT,    0, AN_LIPOT_WHY ,    0},
	{{"!how",    "!make",     "?potion",   "?", NULL},   0, AR_LIPOT,    0, AN_LIPOT_HOW ,    0},
	{{"!how",    "!mix",      "?potion",   "?", NULL},   0, AR_LIPOT,    0, AN_LIPOT_HOW ,    0},
	{{"!how",    "!get",      "?potion",   "?", NULL},   0, AR_LIPOT,    0, AN_LIPOT_HOW ,    0},
	// Key words ................................... , Dif,      Area, Tmp,         Answer, Spc		AR_HEDGE
	{{"!danger", "!hedge",    "?maze",     "?", NULL},   0, AR_HEDGE,    0, AN_HEDGE_WHO ,    0},
	{{"!tell",   "!hedge",    "?maze",     "?", NULL},   0, AR_HEDGE,    0, AN_HEDGE_WHO ,    0},
	{{"!who",    "!hedge",    "?maze",     "?", NULL},   0, AR_HEDGE,    0, AN_HEDGE_WHO ,    0},
	{{"!what",   "!shield",   "?",              NULL},   0, AR_HEDGE,    0, AN_HEDGE_WHAT,    0},
	{{"!what",   "!buckler",  "?",              NULL},   0, AR_HEDGE,    0, AN_HEDGE_WHAT,    0},
	{{"!tell",   "?about",    "!shield",   "?", NULL},   0, AR_HEDGE,    0, AN_HEDGE_WHAT,    0},
	{{"!tell",   "?about",    "!buckler",  "?", NULL},   0, AR_HEDGE,    0, AN_HEDGE_WHAT,    0},
	{{"!where",  "!hedge",    "?maze",     "?", NULL},   0, AR_HEDGE,    0, AN_HEDGE_WHER,    0},
	{{"!why",    "?want",     "!shield",   "?", NULL},   0, AR_HEDGE,    0, AN_HEDGE_WHY ,    0},
	{{"!why",    "?want",     "!buckler",  "?", NULL},   0, AR_HEDGE,    0, AN_HEDGE_WHY ,    0},
	// Key words ................................... , Dif,      Area, Tmp,         Answer, Spc		AR_MMAZE
	{{"!danger", "?magic",    "!maze",     "?", NULL},   0, AR_MMAZE,    0, AN_MMAZE_WHO ,    0},
	{{"!tell",   "?magic",    "!maze",     "?", NULL},   0, AR_MMAZE,    0, AN_MMAZE_WHO ,    0},
	{{"!tell",   "?about",    "!jane",     "?", NULL},   0, AR_MMAZE,    0, AN_MMAZE_WHO ,    0},
	{{"!who",    "?magic",    "!maze",     "?", NULL},   0, AR_MMAZE,    0, AN_MMAZE_WHO ,    0},
	{{"!who",    "?jane",                  "?", NULL},   0, AR_MMAZE,    0, AN_MMAZE_WHO ,    0},
	{{"!what",   "!staff",                 "?", NULL},   0, AR_MMAZE,    0, AN_MMAZE_WHAT,    0},
	{{"!what",   "!weapon",                "?", NULL},   0, AR_MMAZE,    0, AN_MMAZE_WHAT,    0},
	{{"!tell",   "?about",    "!staff",    "?", NULL},   0, AR_MMAZE,    0, AN_MMAZE_WHAT,    0},
	{{"!tell",   "?about",    "!weapon",   "?", NULL},   0, AR_MMAZE,    0, AN_MMAZE_WHAT,    0},
	{{"!where",  "?magic",    "!maze",     "?", NULL},   0, AR_MMAZE,    0, AN_MMAZE_WHER,    0},
	{{"!why",    "?want",     "!staff",    "?", NULL},   0, AR_MMAZE,    0, AN_MMAZE_WHY ,    0},
	{{"!why",    "?want",     "!weapon",   "?", NULL},   0, AR_MMAZE,    0, AN_MMAZE_WHY ,    0},
	{{"!how",    "?navigate", "!maze",     "?", NULL},   0, AR_MMAZE,    0, AN_MMAZE_HOW ,    0},
	{{"!how",    "?finish",   "!maze",     "?", NULL},   0, AR_MMAZE,    0, AN_MMAZE_HOW ,    0},
	// Key words ................................... , Dif,      Area, Tmp,         Answer, Spc		AR_WOODS
	{{"!danger", "?wailing",  "!woods",    "?", NULL},   0, AR_WOODS,    0, AN_WOODS_WHO ,    0},
	{{"!tell",   "?wailing",  "!woods",    "?", NULL},   0, AR_WOODS,    0, AN_WOODS_WHO ,    0},
	{{"!who",    "?wailing",  "!woods",    "?", NULL},   0, AR_WOODS,    0, AN_WOODS_WHO ,    0},
	{{"!what",   "!bandit",   "?",              NULL},   0, AR_WOODS,    0, AN_WOODS_WHO ,    0},
	{{"!tell",   "!argha",    "?",              NULL},   0, AR_WOODS,    0, AN_WOODS_WHO2,    0},
	{{"!who",    "!argha",    "?",              NULL},   0, AR_WOODS,    0, AN_WOODS_WHO2,    0},
	{{"!what",   "!proof",    "?",              NULL},   0, AR_WOODS,    0, AN_WOODS_WHAT,    0},
	{{"!how",    "!prove",    "?",              NULL},   0, AR_WOODS,    0, AN_WOODS_WHAT,    0},
	{{"!where",  "!aston",    "?",              NULL},   0, AR_WOODS,    0, AN_WOODS_WHER,    0},
	{{"!why",    "?want",     "!argha",   "?",  NULL},   0, AR_WOODS,    0, AN_WOODS_WHY ,    0},
	{{"!how",    "?get",      "!aston",   "?",  NULL},   0, AR_WOODS,    0, AN_WOODS_HOW ,    0},
	// Key words ................................... , Dif,      Area, Tmp,         Answer, Spc		AR_ARGHA
	{{"!danger", "!bandit",   "?",              NULL},   0, AR_ARGHA,    0, AN_ARGHA_WHO ,    0},
	{{"!tell",   "!bandit",   "?",              NULL},   0, AR_ARGHA,    0, AN_ARGHA_WHO ,    0},
	{{"!who",    "!argha",    "?",              NULL},   0, AR_ARGHA,    0, AN_ARGHA_WHO2,    0},
	{{"!what",   "!reward",                "?", NULL},   0, AR_ARGHA,    0, AN_ARGHA_WHAT,    0},
	{{"!tell",   "?about",    "!reward",   "?", NULL},   0, AR_ARGHA,    0, AN_ARGHA_WHAT,    0},
	{{"!where",  "!argha",    "?",              NULL},   0, AR_ARGHA,    0, AN_ARGHA_WHER,    0},
	{{"!where",  "!bandit",   "?",              NULL},   0, AR_ARGHA,    0, AN_ARGHA_WHER,    0},
	{{"!why",    "?want",     "!argha",    "?", NULL},   0, AR_ARGHA,    0, AN_ARGHA_WHY ,    0},
	{{"!why",    "?kill",     "!argha",    "?", NULL},   0, AR_ARGHA,    0, AN_ARGHA_WHY ,    0},
	//}
	//{ Aston
	// Key words ................................... , Dif,      Area, Tmp,         Answer, Spc		AR_ASTON
	{{"!where",  "?temple",   "!skua",     "?", NULL},   0,  AR_ASTON,   0, AN_AST_SKUA   ,   0},
	{{"!where",  "?temple",   "!purple",   "?", NULL},   0,  AR_ASTON,   0, AN_AST_PURPLE ,   0},
	{{"!where",  "!bank",     "?",              NULL},   0,  AR_ASTON,   0, AN_AST_BANK   ,   0},
	{{"!where",  "!shop",     "?",              NULL},   0,  AR_ASTON,   0, AN_AST_SHOP   ,   0},
	{{"!where",  "!buy",      "?",              NULL},   0,  AR_ASTON,   0, AN_AST_SHOP   ,   0},
	{{"!where",  "!sell",     "?",              NULL},   0,  AR_ASTON,   0, AN_AST_SHOP   ,   0},
	{{"!where",  "!temple",   "!street",   "?", NULL},   0,  AR_ASTON,   0, AN_AST_TEMSTR ,   0},
	{{"!where",  "!south",    "!end",      "?", NULL},   0,  AR_ASTON,   0, AN_AST_SOUEND ,   0},
	{{"!where",  "!rose",     "!street",   "?", NULL},   0,  AR_ASTON,   0, AN_AST_ROSSTR ,   0},
	{{"!where",  "!marble",   "!lane",     "?", NULL},   0,  AR_ASTON,   0, AN_AST_MARLAN ,   0},
	{{"!where",  "!merchant", "!way",      "?", NULL},   0,  AR_ASTON,   0, AN_AST_MERWAY ,   0},
	{{"!where",  "!bulwark",  "!avenue",   "?", NULL},   0,  AR_ASTON,   0, AN_AST_BULAVE ,   0},
	{{"!where",  "!guard",    "?house",    "?", NULL},   0,  AR_ASTON,   0, AN_AST_GUAHOU ,   0},
	{{"?where",  "?what",     "!lynbore",  "?", NULL},   0,  AR_ASTON,   0, AN_AST_ASTON  ,   0},
	// Key words ................................... , Dif,      Area, Tmp,         Answer, Spc		AR_ARACH
	{{"!danger", "?aston",  "!park",     "?",   NULL},   0,  AR_ARACH,   0,  AN_ARACH_WHO ,   0},
	{{"!tell",   "?aston",  "!park",     "?",   NULL},   0,  AR_ARACH,   0,  AN_ARACH_WHO ,   0},
	{{"!who",    "?aston",  "!park",     "?",   NULL},   0,  AR_ARACH,   0,  AN_ARACH_WHO ,   0},
	{{"!what",   "!flower", "?",                NULL},   0,  AR_ARACH,   0,  AN_ARACH_WHAT,   0},
	{{"!tell",   "?about",  "!flower",     "?", NULL},   0,  AR_ARACH,   0,  AN_ARACH_WHAT,   0},
	{{"!where",  "?aston",  "!park",       "?", NULL},   0,  AR_ARACH,   0,  AN_ARACH_WHER,   0},
	{{"!why",    "?want",   "!flower",     "?", NULL},   0,  AR_ARACH,   0,  AN_ARACH_WHY ,   0},
	// Key words ................................... , Dif,      Area, Tmp,         Answer, Spc		AR_MANSION
	{{"!danger", "?butler", "!mansion",    "?", NULL},   0,AR_MANSION,   0,AN_MANSION_WHO ,   0},
	{{"!tell",   "?butler", "!mansion",    "?", NULL},   0,AR_MANSION,   0,AN_MANSION_WHO ,   0},
	{{"!who",    "?butler", "!mansion",    "?", NULL},   0,AR_MANSION,   0,AN_MANSION_WHO ,   0},
	{{"!what",   "?kite", "!shield",   "?",     NULL},   0,AR_MANSION,   0,AN_MANSION_WHAT,   0},
	{{"!tell","?about","?kite","!shield", "?",  NULL},   0,AR_MANSION,   0,AN_MANSION_WHAT,   0},
	{{"!where",  "?butler",  "!mansion",   "?", NULL},   0,AR_MANSION,   0,AN_MANSION_WHER,   0},
	{{"!why", "?want", "?kite", "!shield", "?", NULL},   0,AR_MANSION,   0,AN_MANSION_WHY ,   0},
	// Key words ................................... , Dif,      Area, Tmp,         Answer, Spc		AR_BELL
	{{"!danger", "?bell",   "!house",      "?", NULL},   0,   AR_BELL,   0,   AN_BELL_WHO ,   0},
	{{"!tell",   "?bell",   "!house",      "?", NULL},   0,   AR_BELL,   0,   AN_BELL_WHO ,   0},
	{{"!who",    "?bell",   "!house",      "?", NULL},   0,   AR_BELL,   0,   AN_BELL_WHO ,   0},
	{{"!what",   "!weapon",                "?", NULL},   0,   AR_BELL,   0,   AN_BELL_WHAT,   0},
	{{"!tell",   "?about",  "!weapon",     "?", NULL},   0,   AR_BELL,   0,   AN_BELL_WHAT,   0},
	{{"!where",  "?bell",   "!house",      "?", NULL},   0,   AR_BELL,   0,   AN_BELL_WHER,   0},
	{{"!why",    "?want",   "!weapon",     "?", NULL},   0,   AR_BELL,   0,   AN_BELL_WHY ,   0},
	// Key words ................................... , Dif,      Area, Tmp,         Answer, Spc		AR_WEBBED
	{{"!danger", "?webbed", "!bush",       "?", NULL},   0, AR_WEBBED,   0, AN_WEBBED_WHO ,   0},
	{{"!tell",   "?webbed", "!bush",       "?", NULL},   0, AR_WEBBED,   0, AN_WEBBED_WHO ,   0},
	{{"!who",    "?webbed", "!bush",       "?", NULL},   0, AR_WEBBED,   0, AN_WEBBED_WHO ,   0},
	{{"!what",   "?spider", "!fang",       "?", NULL},   0, AR_WEBBED,   0, AN_WEBBED_WHAT,   0},
	{{"!tell","?about","?spider","!fang",  "?", NULL},   0, AR_WEBBED,   0, AN_WEBBED_WHAT,   0},
	{{"!where",  "?webbed", "!bush",       "?", NULL},   0, AR_WEBBED,   0, AN_WEBBED_WHER,   0},
	{{"!why","?want","?spider","!fang",    "?", NULL},   0, AR_WEBBED,   0, AN_WEBBED_WHY ,   0},
	// Key words ................................... , Dif,      Area, Tmp,         Answer, Spc		AR_MINE2
	{{"!danger",            "!mine",       "?", NULL},   0,  AR_MINE2,   0,  AN_MINE2_WHO ,   0},
	{{"!tell",              "!mine",       "?", NULL},   0,  AR_MINE2,   0,  AN_MINE2_WHO ,   0},
	{{"!who",               "!mine",       "?", NULL},   0,  AR_MINE2,   0,  AN_MINE2_WHO ,   0},
	{{"!what",   "!potion",                "?", NULL},   0,  AR_MINE2,   0,  AN_MINE2_WHAT,   0},
	{{"!tell",   "?about",  "!potion",     "?", NULL},   0,  AR_MINE2,   0,  AN_MINE2_WHAT,   0},
	{{"!where",             "!mine",       "?", NULL},   0,  AR_MINE2,   0,  AN_MINE2_WHER,   0},
	{{"!why",    "?want",   "!potion",     "?", NULL},   0,  AR_MINE2,   0,  AN_MINE2_WHY ,   0},
	// Key words ................................... , Dif,      Area, Tmp,         Answer, Spc		AR_PENITN
	{{"!danger", "?aston",  "!jail",       "?", NULL},   0, AR_PENITN,   0, AN_PENITN_WHO ,   0},
	{{"!tell",   "?aston",  "!jail",       "?", NULL},   0, AR_PENITN,   0, AN_PENITN_WHO ,   0},
	{{"!who",    "?aston",  "!jail",       "?", NULL},   0, AR_PENITN,   0, AN_PENITN_WHO ,   0},
	{{"!what",   "!evidence",              "?", NULL},   0, AR_PENITN,   0, AN_PENITN_WHAT,   0},
	{{"!tell",   "?about",  "!evidence",   "?", NULL},   0, AR_PENITN,   0, AN_PENITN_WHAT,   0},
	{{"!where",  "?aston",  "!jail",       "?", NULL},   0, AR_PENITN,   0, AN_PENITN_WHER,   0},
	{{"!why",    "?want",   "!evidence",   "?", NULL},   0, AR_PENITN,   0, AN_PENITN_WHY ,   0},
	// Key words ................................... , Dif,      Area, Tmp,         Answer, Spc		AR_GROLMLB
	{{"!danger", "?grolm",  "!lab",        "?", NULL},   0,AR_GROLMLB,   0,AN_GROLMLB_WHO ,   0},
	{{"!tell",   "?grolm",  "!lab",        "?", NULL},   0,AR_GROLMLB,   0,AN_GROLMLB_WHO ,   0},
	{{"!who",    "?grolm",  "!lab",        "?", NULL},   0,AR_GROLMLB,   0,AN_GROLMLB_WHO ,   0},
	{{"!what",   "?ruby",   "!axe",        "?", NULL},   0,AR_GROLMLB,   0,AN_GROLMLB_WHAT,   0},
	{{"!what",   "!weapon",                "?", NULL},   0,AR_GROLMLB,   0,AN_GROLMLB_WHAT,   0},
	{{"!tell", "?about", "?ruby", "!axe",  "?", NULL},   0,AR_GROLMLB,   0,AN_GROLMLB_WHAT,   0},
	{{"!tell",   "?about",  "!weapon",     "?", NULL},   0,AR_GROLMLB,   0,AN_GROLMLB_WHAT,   0},
	{{"!where",  "?grolm",  "!lab",        "?", NULL},   0,AR_GROLMLB,   0,AN_GROLMLB_WHER,   0},
	{{"!why", "?want", "?ruby", "!axe",    "?", NULL},   0,AR_GROLMLB,   0,AN_GROLMLB_WHY ,   0},
	{{"!why",    "?want",   "!weapon",     "?", NULL},   0,AR_GROLMLB,   0,AN_GROLMLB_WHY ,   0},
	// Key words ................................... , Dif,      Area, Tmp,         Answer, Spc		AR_HERMIT
	{{"!danger", "?mad",    "!hermit",     "?", NULL},   0, AR_HERMIT,   0, AN_HERMIT_WHO ,   0},
	{{"!tell",   "?mad",    "!hermit",     "?", NULL},   0, AR_HERMIT,   0, AN_HERMIT_WHO ,   0},
	{{"!who",    "?mad",    "!hermit",     "?", NULL},   0, AR_HERMIT,   0, AN_HERMIT_WHO ,   0},
	{{"!what",   "!flower",                "?", NULL},   0, AR_HERMIT,   0, AN_HERMIT_WHAT,   0},
	{{"!tell",   "?about",  "!flower",     "?", NULL},   0, AR_HERMIT,   0, AN_HERMIT_WHAT,   0},
	{{"!where",  "?mad",    "!hermit",     "?", NULL},   0, AR_HERMIT,   0, AN_HERMIT_WHER,   0},
	{{"!why",    "?want",   "!flower",     "?", NULL},   0, AR_HERMIT,   0, AN_HERMIT_WHY ,   0},
	// Key words ................................... , Dif,      Area, Tmp,         Answer, Spc		AR_LAKEBED
	{{"!danger", "?violet", "!lake",       "?", NULL},   0,AR_LAKEBED,   0,AN_LAKEBED_WHO ,   0},
	{{"!tell",   "?violet", "!lake",       "?", NULL},   0,AR_LAKEBED,   0,AN_LAKEBED_WHO ,   0},
	{{"!who",    "?violet", "!lake",       "?", NULL},   0,AR_LAKEBED,   0,AN_LAKEBED_WHO ,   0},
	{{"!what",   "!weapon",                "?", NULL},   0,AR_LAKEBED,   0,AN_LAKEBED_WHAT,   0},
	{{"!tell",   "?about",  "!weapon",     "?", NULL},   0,AR_LAKEBED,   0,AN_LAKEBED_WHAT,   0},
	{{"!where",  "?violet", "!lake",       "?", NULL},   0,AR_LAKEBED,   0,AN_LAKEBED_WHER,   0},
	{{"!why",    "?want",   "!weapon",     "?", NULL},   0,AR_LAKEBED,   0,AN_LAKEBED_WHY ,   0},
	// Key words ................................... , Dif,      Area, Tmp,         Answer, Spc		AR_SEWER1
	{{"!danger", "!underground",           "?", NULL},   0, AR_SEWER1,   0, AN_SEWER1_WHO ,   0},
	{{"!tell",   "!underground",           "?", NULL},   0, AR_SEWER1,   0, AN_SEWER1_WHO ,   0},
	{{"!who",    "!underground",           "?", NULL},   0, AR_SEWER1,   0, AN_SEWER1_WHO ,   0},
	{{"!what",   "?ratling", "!eye",       "?", NULL},   0, AR_SEWER1,   0, AN_SEWER1_WHAT,   0},
	{{"!tell","?about","?ratling", "!eye", "?", NULL},   0, AR_SEWER1,   0, AN_SEWER1_WHAT,   0},
	{{"!where",  "!underground",           "?", NULL},   0, AR_SEWER1,   0, AN_SEWER1_WHER,   0},
	{{"!why", "?want", "?ratling", "!eye", "?", NULL},   0, AR_SEWER1,   0, AN_SEWER1_WHY ,   0},
	//}
	//{ Aston pt 2
	// Key words ................................... , Dif,      Area, Tmp,         Answer, Spc		AR_SEWER2
	{{"!danger", "!underground",           "?", NULL},   0, AR_SEWER2,   0, AN_SEWER2_WHO ,   0},
	{{"!tell",   "!underground",           "?", NULL},   0, AR_SEWER2,   0, AN_SEWER2_WHO ,   0},
	{{"!who",    "!underground",           "?", NULL},   0, AR_SEWER2,   0, AN_SEWER2_WHO ,   0},
	{{"!what",   "?greenling", "!eye",     "?", NULL},   0, AR_SEWER2,   0, AN_SEWER2_WHAT,   0},
	{{"!tell","?about","?greenling","!eye","?", NULL},   0, AR_SEWER2,   0, AN_SEWER2_WHAT,   0},
	{{"!where",  "!underground",           "?", NULL},   0, AR_SEWER2,   0, AN_SEWER2_WHER,   0},
	{{"!why", "?want", "?greenling","!eye","?", NULL},   0, AR_SEWER2,   0, AN_SEWER2_WHY ,   0},
	// Key words ................................... , Dif,      Area, Tmp,         Answer, Spc		AR_GARG
	{{"!danger", "?gargoyle", "!nest",     "?", NULL},   0, AR_GARG  ,   0, AN_GARG_WHO   ,   0},
	{{"!tell",   "?gargoyle", "!nest",     "?", NULL},   0, AR_GARG  ,   0, AN_GARG_WHO   ,   0},
	{{"!who",    "?gargoyle", "!nest",     "?", NULL},   0, AR_GARG  ,   0, AN_GARG_WHO   ,   0},
	{{"!what",   "?fire", "!egg",          "?", NULL},   0, AR_GARG  ,   0, AN_GARG_WHAT  ,   0},
	{{"!tell","?about","?fire", "!egg",    "?", NULL},   0, AR_GARG  ,   0, AN_GARG_WHAT  ,   0},
	{{"!where",  "?gargoyle", "!nest",     "?", NULL},   0, AR_GARG  ,   0, AN_GARG_WHER  ,   0},
	{{"!why", "?want", "?fire", "!egg",    "?", NULL},   0, AR_GARG  ,   0, AN_GARG_WHY   ,   0},
	// Key words ................................... , Dif,      Area, Tmp,         Answer, Spc		AR_CANYON
	{{"!danger", "!canyon",                "?", NULL},   0, AR_CANYON,   0, AN_CANYON_WHO ,   0},
	{{"!tell",   "!canyon",                "?", NULL},   0, AR_CANYON,   0, AN_CANYON_WHO ,   0},
	{{"!who",    "!canyon",                "?", NULL},   0, AR_CANYON,   0, AN_CANYON_WHO ,   0},
	{{"!what",   "!shield",                "?", NULL},   0, AR_CANYON,   0, AN_CANYON_WHAT,   0},
	{{"!tell","?about", "!shield",         "?", NULL},   0, AR_CANYON,   0, AN_CANYON_WHAT,   0},
	{{"!where",  "!canyon",                "?", NULL},   0, AR_CANYON,   0, AN_CANYON_WHER,   0},
	{{"!why", "?want", "!shield",          "?", NULL},   0, AR_CANYON,   0, AN_CANYON_WHY ,   0},
	// Key words ................................... , Dif,      Area, Tmp,         Answer, Spc		AR_PASS  
	{{"!danger", "?jagged", "!pass",       "?", NULL},   0, AR_PASS  ,   0, AN_PASS_WHO   ,   0},
	{{"!tell",   "?jagged", "!pass",       "?", NULL},   0, AR_PASS  ,   0, AN_PASS_WHO   ,   0},
	{{"!who",    "?jagged", "!pass",       "?", NULL},   0, AR_PASS  ,   0, AN_PASS_WHO   ,   0},
	{{"!what",   "!helmet",                "?", NULL},   0, AR_PASS  ,   0, AN_PASS_WHAT  ,   0},
	{{"!tell","?about", "!helmet",         "?", NULL},   0, AR_PASS  ,   0, AN_PASS_WHAT  ,   0},
	{{"!where",  "?jagged", "!pass",       "?", NULL},   0, AR_PASS  ,   0, AN_PASS_WHER  ,   0},
	{{"!why", "?want", "!helmet",          "?", NULL},   0, AR_PASS  ,   0, AN_PASS_WHY   ,   0},
	// Key words ................................... , Dif,      Area, Tmp,         Answer, Spc		AR_ARCHIVE
	{{"!danger", "?abandoned", "!archive", "?", NULL},   0, AR_ARCHIVE,  0, AN_ARCHIVE_WHO ,  0},
	{{"!tell",   "?abandoned", "!archive", "?", NULL},   0, AR_ARCHIVE,  0, AN_ARCHIVE_WHO ,  0},
	{{"!who",    "?abandoned", "!archive", "?", NULL},   0, AR_ARCHIVE,  0, AN_ARCHIVE_WHO ,  0},
	{{"!what",   "!book",                  "?", NULL},   0, AR_ARCHIVE,  0, AN_ARCHIVE_WHAT,  0},
	{{"!tell","?about","!book",            "?", NULL},   0, AR_ARCHIVE,  0, AN_ARCHIVE_WHAT,  0},
	{{"!where",  "?abandoned", "!archive", "?", NULL},   0, AR_ARCHIVE,  0, AN_ARCHIVE_WHER,  0},
	{{"!why", "?want", "!book",            "?", NULL},   0, AR_ARCHIVE,  0, AN_ARCHIVE_WHY ,  0},
	// Key words ................................... , Dif,      Area, Tmp,         Answer, Spc		AR_MINE3 
	{{"!danger", "!mine",                  "?", NULL},   0, AR_MINE3 ,   0, AN_MINE3_WHO  ,   0},
	{{"!tell",   "!mine",                  "?", NULL},   0, AR_MINE3 ,   0, AN_MINE3_WHO  ,   0},
	{{"!who",    "!mine",                  "?", NULL},   0, AR_MINE3 ,   0, AN_MINE3_WHO  ,   0},
	{{"!what",   "!statue",                "?", NULL},   0, AR_MINE3 ,   0, AN_MINE3_WHAT ,   0},
	{{"!tell","?about","!statue",          "?", NULL},   0, AR_MINE3 ,   0, AN_MINE3_WHAT ,   0},
	{{"!where",  "!mine",                  "?", NULL},   0, AR_MINE3 ,   0, AN_MINE3_WHER ,   0},
	{{"!why", "?want", "!statue",          "?", NULL},   0, AR_MINE3 ,   0, AN_MINE3_WHY  ,   0},
	// Key words ................................... , Dif,      Area, Tmp,         Answer, Spc		AR_SWAMP 
	{{"!danger", "?south", "!swamp",       "?", NULL},   0, AR_SWAMP ,   0, AN_SWAMP_WHO  ,   0},
	{{"!tell",   "?south", "!swamp",       "?", NULL},   0, AR_SWAMP ,   0, AN_SWAMP_WHO  ,   0},
	{{"!who",    "?south", "!swamp",       "?", NULL},   0, AR_SWAMP ,   0, AN_SWAMP_WHO  ,   0},
	{{"!what",   "?ratling", "!eye",       "?", NULL},   0, AR_SWAMP ,   0, AN_SWAMP_WHAT ,   0},
	{{"!tell","?about","?ratling", "!eye", "?", NULL},   0, AR_SWAMP ,   0, AN_SWAMP_WHAT ,   0},
	{{"!where",  "?south", "!swamp",       "?", NULL},   0, AR_SWAMP ,   0, AN_SWAMP_WHER ,   0},
	{{"!why", "?want", "?ratling", "!eye", "?", NULL},   0, AR_SWAMP ,   0, AN_SWAMP_WHY  ,   0},
	// Key words ................................... , Dif,      Area, Tmp,         Answer, Spc		AR_MOUNT 
	{{"!danger", "?north", "!mountain",    "?", NULL},   0, AR_MOUNT ,   0, AN_MOUNT_WHO  ,   0},
	{{"!tell",   "?north", "!mountain",    "?", NULL},   0, AR_MOUNT ,   0, AN_MOUNT_WHO  ,   0},
	{{"!who",    "?north", "!mountain",    "?", NULL},   0, AR_MOUNT ,   0, AN_MOUNT_WHO  ,   0},
	{{"!what",   "!amulet",                "?", NULL},   0, AR_MOUNT ,   0, AN_MOUNT_WHAT ,   0},
	{{"!tell","?about","!amulet",          "?", NULL},   0, AR_MOUNT ,   0, AN_MOUNT_WHAT ,   0},
	{{"!where",  "?north", "!mountain",    "?", NULL},   0, AR_MOUNT ,   0, AN_MOUNT_WHER ,   0},
	{{"!why", "?want", "!amulet",          "?", NULL},   0, AR_MOUNT ,   0, AN_MOUNT_WHY  ,   0},
	// Key words ................................... , Dif,      Area, Tmp,         Answer, Spc		AR_DIAMOND
	{{"!danger", "!mine",                  "?", NULL},   0, AR_DIAMOND,  0, AN_DIAMOND_WHO ,  0},
	{{"!tell",   "!mine",                  "?", NULL},   0, AR_DIAMOND,  0, AN_DIAMOND_WHO ,  0},
	{{"!who",    "!mine",                  "?", NULL},   0, AR_DIAMOND,  0, AN_DIAMOND_WHO ,  0},
	{{"!what",   "?diamond", "!ring",      "?", NULL},   0, AR_DIAMOND,  0, AN_DIAMOND_WHAT,  0},
	{{"!tell","?about","?diamond", "!ring","?", NULL},   0, AR_DIAMOND,  0, AN_DIAMOND_WHAT,  0},
	{{"!where",  "!mine",                  "?", NULL},   0, AR_DIAMOND,  0, AN_DIAMOND_WHER,  0},
	{{"!why", "?want", "?diamond", "!ring","?", NULL},   0, AR_DIAMOND,  0, AN_DIAMOND_WHY ,  0},
	// Key words ................................... , Dif,      Area, Tmp,         Answer, Spc		AR_HERBS
	{{"!danger", "!underground",           "?", NULL},   0, AR_HERBS ,   0, AN_HERBS_WHO  ,   0},
	{{"!tell",   "!underground",           "?", NULL},   0, AR_HERBS ,   0, AN_HERBS_WHO  ,   0},
	{{"!who",    "!underground",           "?", NULL},   0, AR_HERBS ,   0, AN_HERBS_WHO  ,   0},
	{{"!what",   "?green", "!plant",       "?", NULL},   0, AR_HERBS ,   0, AN_HERBS_WHAT ,   0},
	{{"!tell","?about","?green", "!plant", "?", NULL},   0, AR_HERBS ,   0, AN_HERBS_WHAT ,   0},
	{{"!where",  "!underground",           "?", NULL},   0, AR_HERBS ,   0, AN_HERBS_WHER ,   0},
	{{"!why", "?want", "?green", "!plant", "?", NULL},   0, AR_HERBS ,   0, AN_HERBS_WHY  ,   0},
	// Key words ................................... , Dif,      Area, Tmp,         Answer, Spc		AR_TARGE
	{{"!danger", "?gargoyle", "!nest",     "?", NULL},   0, AR_TARGE ,   0, AN_TARGE_WHO  ,   0},
	{{"!tell",   "?gargoyle", "!nest",     "?", NULL},   0, AR_TARGE ,   0, AN_TARGE_WHO  ,   0},
	{{"!who",    "?gargoyle", "!nest",     "?", NULL},   0, AR_TARGE ,   0, AN_TARGE_WHO  ,   0},
	{{"!what",   "!shield",                "?", NULL},   0, AR_TARGE ,   0, AN_TARGE_WHAT ,   0},
	{{"!tell","?about","!shield",          "?", NULL},   0, AR_TARGE ,   0, AN_TARGE_WHAT ,   0},
	{{"!where",  "?gargoyle", "!nest",     "?", NULL},   0, AR_TARGE ,   0, AN_TARGE_WHER ,   0},
	{{"!why", "?want", "!shield",          "?", NULL},   0, AR_TARGE ,   0, AN_TARGE_WHY  ,   0},
	// Key words ................................... , Dif,      Area, Tmp,         Answer, Spc		AR_SEWER3
	{{"!danger", "!underground",           "?", NULL},   0, AR_SEWER3,   0, AN_SEWER3_WHO ,   0},
	{{"!tell",   "!underground",           "?", NULL},   0, AR_SEWER3,   0, AN_SEWER3_WHO ,   0},
	{{"!who",    "!underground",           "?", NULL},   0, AR_SEWER3,   0, AN_SEWER3_WHO ,   0},
	{{"!what",   "?sogling", "!eye",       "?", NULL},   0, AR_SEWER3,   0, AN_SEWER3_WHAT,   0},
	{{"!tell","?about","?sogling","!eye",  "?", NULL},   0, AR_SEWER3,   0, AN_SEWER3_WHAT,   0},
	{{"!where",  "!underground",           "?", NULL},   0, AR_SEWER3,   0, AN_SEWER3_WHER,   0},
	{{"!why", "?want", "?sogling","!eye",  "?", NULL},   0, AR_SEWER3,   0, AN_SEWER3_WHY ,   0},
	//}
	//{ Forest, Stronghold
	// Key words ................................... , Dif,      Area, Tmp,         Answer, Spc		AR_FOREST
	// Key words ................................... , Dif,      Area, Tmp,         Answer, Spc		AR_STHOLD
	{{"?need",   "!help",   "?stronghold", "?", NULL}, 0,   AR_STHOLD, 372,     AN_BS_HELP,   0},
	{{"!help",                             "!", NULL}, 0,   AR_STHOLD, 372,     AN_BS_HELP,   0},
	{{"!protect","?outpost","?gate","?city","?",NULL}, 0,   AR_STHOLD, 372,     AN_BS_PROT,   0},
	{{"!reward","?protect",        "?",         NULL}, 0,   AR_STHOLD, 372,     AN_BS_REWA,   0},
	{{"!points","?",                            NULL}, 0,   AR_STHOLD, 372,     AN_BS_POIN,   SP_POINTS},
	{{"?enter", "?black","!stronghold","?",     NULL}, 0,   AR_STHOLD, 372,     AN_BS_STRO,   0},
	{{"!stronghold",                       "!", NULL}, 0,   AR_STHOLD, 372,     AN_BS_STRO,   0},
	{{"!north",  "?outpost", "?gate",      "?", NULL}, 0,   AR_STHOLD, 372,     AN_BS_NORT,   0},
	{{"!center", "?outpost", "?gate",      "?", NULL}, 0,   AR_STHOLD, 372,     AN_BS_CENT,   0},
	{{"!south",  "?outpost", "?gate",      "?", NULL}, 0,   AR_STHOLD, 372,     AN_BS_SOUT,   0},
	// Black Stronghold Special Answers
	{{"!table",                            "?", NULL}, 0,   AR_STHOLD, 372, NULL, SP_TABLE},
	{{"!buy", "!money",                         NULL}, 0,   AR_STHOLD, 372, NULL, SP_BUY_GOLD},
	{{"!buy", "!gold",                          NULL}, 0,   AR_STHOLD, 372, NULL, SP_BUY_GOLD},
	{{"!buy", "!experience",                    NULL}, 0,   AR_STHOLD, 372, NULL, SP_BUY_EXP},
	{{"!buy", "!exp",                           NULL}, 0,   AR_STHOLD, 372, NULL, SP_BUY_EXP},
	{{"!buy", "!health",                        NULL}, 0,   AR_STHOLD, 372, NULL, SP_BUY_HEAL},
	{{"!buy", "!healing",                       NULL}, 0,   AR_STHOLD, 372, NULL, SP_BUY_HEAL},
	{{"!buy", "!heal",                          NULL}, 0,   AR_STHOLD, 372, NULL, SP_BUY_HEAL},
	{{"!buy", "!mana",                          NULL}, 0,   AR_STHOLD, 372, NULL, SP_BUY_MANA},
	{{"!buy", "!endurance",                     NULL}, 0,   AR_STHOLD, 372, NULL, SP_BUY_END},
	{{"!buy", "!end",                           NULL}, 0,   AR_STHOLD, 372, NULL, SP_BUY_END},
	{{"!buy", "!ghealth",                       NULL}, 0,   AR_STHOLD, 372, NULL, SP_BUY_GHEAL},
	{{"!buy", "!ghealing",                      NULL}, 0,   AR_STHOLD, 372, NULL, SP_BUY_GHEAL},
	{{"!buy", "!gheal",                         NULL}, 0,   AR_STHOLD, 372, NULL, SP_BUY_GHEAL},
	{{"!buy", "!gmana",                         NULL}, 0,   AR_STHOLD, 372, NULL, SP_BUY_GMANA},
	{{"!buy", "!gendurance",                    NULL}, 0,   AR_STHOLD, 372, NULL, SP_BUY_GEND},
	{{"!buy", "!gend",                          NULL}, 0,   AR_STHOLD, 372, NULL, SP_BUY_GEND},
	{{"!buy", "!ratling",                       NULL}, 0,   AR_STHOLD, 372, NULL, SP_BUY_RATLING},
	{{"!buy", "!rat",                           NULL}, 0,   AR_STHOLD, 372, NULL, SP_BUY_RATLING},
	{{"!buy", "!greenling",                     NULL}, 0,   AR_STHOLD, 372, NULL, SP_BUY_GREENLING},
	{{"!buy", "!green",                         NULL}, 0,   AR_STHOLD, 372, NULL, SP_BUY_GREENLING},
  // {{"!buy", "!sogling",                     NULL}, 0,   AR_STHOLD, 372, NULL, SP_BUY_DREADLING},
  // {{"!buy", "!sog",                         NULL}, 0,   AR_STHOLD, 372, NULL, SP_BUY_DREADLING},
	{{"!buy", "!weapon1",                       NULL}, 0,   AR_STHOLD, 372, NULL, SP_BUY_WEAPON1},
	{{"!buy", "!weap1",                         NULL}, 0,   AR_STHOLD, 372, NULL, SP_BUY_WEAPON1},
	{{"!buy", "!weapon2",                       NULL}, 0,   AR_STHOLD, 372, NULL, SP_BUY_WEAPON2},
	{{"!buy", "!weap2",                         NULL}, 0,   AR_STHOLD, 372, NULL, SP_BUY_WEAPON2},
	{{"!buy", "!weapon3",                       NULL}, 0,   AR_STHOLD, 372, NULL, SP_BUY_WEAPON3},
	{{"!buy", "!weap3",                         NULL}, 0,   AR_STHOLD, 372, NULL, SP_BUY_WEAPON3},
	{{"!buy", "!helmet",                        NULL}, 0,   AR_STHOLD, 372, NULL, SP_BUY_HELMET},
	{{"!buy", "!helm",                          NULL}, 0,   AR_STHOLD, 372, NULL, SP_BUY_HELMET},
	{{"!buy", "!hat",                           NULL}, 0,   AR_STHOLD, 372, NULL, SP_BUY_HELMET},
	{{"!buy", "!armour",                        NULL}, 0,   AR_STHOLD, 372, NULL, SP_BUY_ARMOR},
	{{"!buy", "!armor",                         NULL}, 0,   AR_STHOLD, 372, NULL, SP_BUY_ARMOR},
	//}
	//{ Dieties
	// Key words ................................... , Dif,      Area, Tmp,         Answer, Spc		CT_BISHOP
	{{"!remove", "?",                           NULL}, 0,  AR_GENERAL, CT_BISHOP,    NULL, SP_TAROT},
	{{"!remove", "?tarot", "?card", "?",        NULL}, 0,  AR_GENERAL, CT_BISHOP,    NULL, SP_TAROT},
	{{"!who",   "!skua",                   "?", NULL}, 0,  AR_GENERAL, CT_BISHOP, AN_GOD_SKUA2,   0},
	{{"!who",   "!ankh",                   "?", NULL}, 0,  AR_GENERAL, CT_BISHOP, AN_GOD_ANKH2,   0},
	{{"!who",   "!ishtar",                 "?", NULL}, 0,  AR_GENERAL, CT_BISHOP, AN_GOD_ISHTAR2, 0},
	{{"!who",   "!gorn",                   "?", NULL}, 0,  AR_GENERAL, CT_BISHOP, AN_GOD_GORN2,   0},
	{{"!who",   "!kwai",                   "?", NULL}, 0,  AR_GENERAL, CT_BISHOP, AN_GOD_KWAI2,   0},
	{{"!who",   "!purple", "?one",         "?", NULL}, 0,  AR_GENERAL, CT_BISHOP, AN_GOD_PURPLE2, 0},
	// Key words ................................... , Dif,      Area, Tmp,         Answer, Spc		
	{{"!who",   "!skua",                   "?", NULL}, 0,   AR_PURPLE,   0, AN_GOD_SKUA3,   0},
	{{"!who",   "!ankh",                   "?", NULL}, 0,   AR_PURPLE,   0, AN_GOD_ANKH3,   0},
	{{"!who",   "!ishtar",                 "?", NULL}, 0,   AR_PURPLE,   0, AN_GOD_ISHTAR3, 0},
	{{"!who",   "!gorn",                   "?", NULL}, 0,   AR_PURPLE,   0, AN_GOD_GORN3,   0},
	{{"!who",   "!kwai",                   "?", NULL}, 0,   AR_PURPLE,   0, AN_GOD_KWAI3,   0},
	{{"!who",   "!purple", "?one",         "?", NULL}, 0,   AR_PURPLE,   0, AN_GOD_PURPLE3, 0},
	{{"?what","!order","?purple","?one",   "?", NULL}, 0,   AR_PURPLE,   0, AN_PURPLE1,     0},
	{{"?tell","!order","?purple","?one",   "?", NULL}, 0,   AR_PURPLE,   0, AN_PURPLE1,     0},
	{{"?how","!join","?order","?purple","?one","?",NULL},0, AR_PURPLE,   0, AN_PURPLE2,     0},
	{{"?what","!happens","!join","?order","?purple","?one","?",NULL},0,AR_PURPLE,0,AN_PURPLE2,0},
	{{"!yes", "!join", "?want",            "!", NULL}, 0,   AR_PURPLE,   0, AN_PURPLE3,     0},
	{{"!yes",                              "!", NULL}, 0,   AR_PURPLE,   0, AN_PURPLE3,     0},
	{{"!kill", "?you",                     "?", NULL}, 0,   AR_PURPLE,   0, AN_PURPLE4,     0},
	// Key words ................................... , Dif,      Area, Tmp,         Answer, Spc		(general dieties)
	{{"!who",   "!skua",                   "?", NULL}, 0,  AR_GENERAL,   0, AN_GOD_SKUA,   0},
	{{"!who",   "!ankh",                   "?", NULL}, 0,  AR_GENERAL,   0, AN_GOD_ANKH,   0},
	{{"!who",   "!ishtar",                 "?", NULL}, 0,  AR_GENERAL,   0, AN_GOD_ISHTAR, 0},
	{{"!who",   "!gorn",                   "?", NULL}, 0,  AR_GENERAL,   0, AN_GOD_GORN,   0},
	{{"!who",   "!kwai",                   "?", NULL}, 0,  AR_GENERAL,   0, AN_GOD_KWAI,   0},
	{{"!who",   "!purple", "?one",         "?", NULL}, 0,  AR_GENERAL,   0, AN_GOD_PURPLE, 0},
	//}
	//{ Labyrinth - These are ordered backwards so that higher difficulties come before lower ones.
	// Key words ................................... , Dif,      Area, Tmp,         Answer, Spc		AR_LAB_GATE
	{{"!remove", "?",                           NULL}, 13, AR_LABYRINTH, 0,    NULL, SP_TAROT2},
	{{"!remove", "?tarot", "?card", "?",        NULL}, 13, AR_LABYRINTH, 0,    NULL, SP_TAROT2},
	{{"!door",                             "?", NULL}, 13, AR_LABYRINTH, 0, AN_LABZ_DOORS   , 0},
	{{"!key",                              "?", NULL}, 13, AR_LABYRINTH, 0, AN_LABZ_KEYS    , 0},
	{{"!riddle",                           "?", NULL}, 13, AR_LABYRINTH, 0, AN_LABZ_RIDDLES , 0},
	{{"?about", "!gate",                   "?", NULL}, 13, AR_LABYRINTH, 0, AN_LABZ_GATE    , 0},
	{{"?about", "!seyan",                  "?", NULL}, 13, AR_LABYRINTH, 0, AN_LABZ_SEYAN   , 0},
	{{"?about", "!seyans",                 "?", NULL}, 13, AR_LABYRINTH, 0, AN_LABZ_SEYAN   , 0},
	{{"?about", "!seyan", "!du",           "?", NULL}, 13, AR_LABYRINTH, 0, AN_LABZ_SEYAN   , 0},
	{{"?about", "!seyan", "!dus",          "?", NULL}, 13, AR_LABYRINTH, 0, AN_LABZ_SEYAN   , 0},
	{{"?about", "!seyan'du",               "?", NULL}, 13, AR_LABYRINTH, 0, AN_LABZ_SEYAN   , 0},
	{{"?about", "!seyan'dus",              "?", NULL}, 13, AR_LABYRINTH, 0, AN_LABZ_SEYAN   , 0},
	{{"?about", "!sword",                  "?", NULL}, 13, AR_LABYRINTH, 0, AN_LABZ_SWORD   , 0},
	{{"?about", "!kwai",                   "?", NULL}, 13, AR_LABYRINTH, 0, AN_LABZ_KWAI    , 0},
	{{"?about", "!skill",                  "?", NULL}, 13, AR_LABYRINTH, 0, AN_LABZ_SKILLS  , 0},
	{{"?about", "!shrine",                 "?", NULL}, 13, AR_LABYRINTH, 0, AN_LABZ_SHRINE  , 0},
	{{"?about", "!arch", "!templar",       "?", NULL}, 13, AR_LABYRINTH, 0, AN_LABZ_TEMPL   , 0},
	{{"?about", "!arch", "!templars",      "?", NULL}, 13, AR_LABYRINTH, 0, AN_LABZ_TEMPL   , 0},
	{{"?about", "!archtemplar",            "?", NULL}, 13, AR_LABYRINTH, 0, AN_LABZ_TEMPL   , 0},
	{{"?about", "!archtemplars",           "?", NULL}, 13, AR_LABYRINTH, 0, AN_LABZ_TEMPL   , 0},
	{{"?about", "!brawler",                "?", NULL}, 13, AR_LABYRINTH, 0, AN_LABZ_BRAWL   , 0},
	{{"?about", "!brawlers",               "?", NULL}, 13, AR_LABYRINTH, 0, AN_LABZ_BRAWL   , 0},
	{{"?about", "!warrior",                "?", NULL}, 13, AR_LABYRINTH, 0, AN_LABZ_WARRI   , 0},
	{{"?about", "!warriors",               "?", NULL}, 13, AR_LABYRINTH, 0, AN_LABZ_WARRI   , 0},
	{{"?about", "!sorcerer",               "?", NULL}, 13, AR_LABYRINTH, 0, AN_LABZ_SORCE   , 0},
	{{"?about", "!sorcerers",              "?", NULL}, 13, AR_LABYRINTH, 0, AN_LABZ_SORCE   , 0},
	{{"?about", "!summoner",               "?", NULL}, 13, AR_LABYRINTH, 0, AN_LABZ_SUMMO   , 0},
	{{"?about", "!summoners",              "?", NULL}, 13, AR_LABYRINTH, 0, AN_LABZ_SUMMO   , 0},
	{{"?about", "!arch", "!harakim",       "?", NULL}, 13, AR_LABYRINTH, 0, AN_LABZ_HARAK   , 0},
	{{"?about", "!arch", "!harakims",      "?", NULL}, 13, AR_LABYRINTH, 0, AN_LABZ_HARAK   , 0},
	{{"?about", "!archharakim",            "?", NULL}, 13, AR_LABYRINTH, 0, AN_LABZ_HARAK   , 0},
	{{"?about", "!archharakims",           "?", NULL}, 13, AR_LABYRINTH, 0, AN_LABZ_HARAK   , 0},
	// Key words ................................... , Dif,      Area, Tmp,         Answer, Spc		AR_LAB_MOUNT
	{{"!door",                             "?", NULL}, 12, AR_LABYRINTH, 0, AN_LABY_DOORS   , 0},
	{{"!key",                              "?", NULL}, 12, AR_LABYRINTH, 0, AN_LABY_KEYS    , 0},
	{{"!riddle",                           "?", NULL}, 12, AR_LABYRINTH, 0, AN_LABY_RIDDLES , 0},
	// Key words ................................... , Dif,      Area, Tmp,         Answer, Spc		AR_LAB_SEASON
	{{"!door",                             "?", NULL}, 11, AR_LABYRINTH, 0, AN_LABX_DOORS   , 0},
	{{"!key",                              "?", NULL}, 11, AR_LABYRINTH, 0, AN_LABX_KEYS    , 0},
	{{"!riddle",                           "?", NULL}, 11, AR_LABYRINTH, 0, AN_LABX_RIDDLES , 0},
	// Key words ................................... , Dif,      Area, Tmp,         Answer, Spc		AR_LAB_FOREST
	{{"!door",                             "?", NULL}, 10, AR_LABYRINTH, 0, AN_LAB0_DOORS   , 0},
	{{"!key",                              "?", NULL}, 10, AR_LABYRINTH, 0, AN_LAB0_KEYS    , 0},
	{{"!riddle",                           "?", NULL}, 10, AR_LABYRINTH, 0, AN_LAB0_RIDDLES , 0},
	// Key words ................................... , Dif,      Area, Tmp,         Answer, Spc		AR_LAB_RIDDLE
	{{"!door",                             "?", NULL},  9, AR_LABYRINTH, 0, AN_LAB9_DOORS   , 0},
	{{"!key",                              "?", NULL},  9, AR_LABYRINTH, 0, AN_LAB9_KEYS    , 0},
	{{"!riddle",                           "?", NULL},  9, AR_LABYRINTH, 0, AN_LAB9_RIDDLES , 0},
	// Key words ................................... , Dif,      Area, Tmp,         Answer, Spc		AR_LAB_WATER
	{{"!door",                             "?", NULL},  8, AR_LABYRINTH, 0, AN_LAB8_DOORS   , 0},
	{{"!key",                              "?", NULL},  8, AR_LABYRINTH, 0, AN_LAB8_KEYS    , 0},
	{{"!riddle",                           "?", NULL},  8, AR_LABYRINTH, 0, AN_LAB8_RIDDLES , 0},
	// Key words ................................... , Dif,      Area, Tmp,         Answer, Spc		AR_LAB_DARK
	{{"!door",                             "?", NULL},  7, AR_LABYRINTH, 0, AN_LAB7_DOORS   , 0},
	{{"!key",                              "?", NULL},  7, AR_LABYRINTH, 0, AN_LAB7_KEYS    , 0},
	{{"!riddle",                           "?", NULL},  7, AR_LABYRINTH, 0, AN_LAB7_RIDDLES , 0},
	// Key words ................................... , Dif,      Area, Tmp,         Answer, Spc		AR_LAB_DESERT
	{{"!door",                             "?", NULL},  6, AR_LABYRINTH, 0, AN_LAB6_DOORS   , 0},
	{{"!key",                              "?", NULL},  6, AR_LABYRINTH, 0, AN_LAB6_KEYS    , 0},
	{{"!riddle",                           "?", NULL},  6, AR_LABYRINTH, 0, AN_LAB6_RIDDLES , 0},
	// Key words ................................... , Dif,      Area, Tmp,         Answer, Spc		AR_LAB_KNIGHT
	{{"!door",                             "?", NULL},  5, AR_LABYRINTH, 0, AN_LAB5_DOORS   , 0},
	{{"!key",                              "?", NULL},  5, AR_LABYRINTH, 0, AN_LAB5_KEYS    , 0},
	{{"!riddle",                           "?", NULL},  5, AR_LABYRINTH, 0, AN_LAB5_RIDDLES , 0},
	{{"!poem", "?first",                   "?", NULL},  0, AR_LAB_KNIGHT, 0, AN_LAB5_POEM1  , 0},
	{{"!poem", "?second",                  "?", NULL},  0, AR_LAB_KNIGHT, 0, AN_LAB5_POEM2  , 0},
	{{"!second",                           "?", NULL},  0, AR_LAB_KNIGHT, 0, AN_LAB5_POEM2  , 0},
	{{"!next",                             "?", NULL},  0, AR_LAB_KNIGHT, 0, AN_LAB5_POEM2  , 0},
	{{"!other",                            "?", NULL},  0, AR_LAB_KNIGHT, 0, AN_LAB5_POEM2  , 0},
	// Key words ................................... , Dif,      Area, Tmp,         Answer, Spc		AR_LAB_SPELL
	{{"!door",                             "?", NULL},  4, AR_LABYRINTH, 0, AN_LAB4_DOORS   , 0},
	{{"!key",                              "?", NULL},  4, AR_LABYRINTH, 0, AN_LAB4_KEYS    , 0},
	{{"!riddle",                           "?", NULL},  4, AR_LABYRINTH, 0, AN_LAB4_RIDDLES , 0},
	// Key words ................................... , Dif,      Area, Tmp,         Answer, Spc		AR_LAB_UNDEAD
	{{"!door",                             "?", NULL},  3, AR_LABYRINTH, 0, AN_LAB3_DOORS   , 0},
	{{"!key",                              "?", NULL},  3, AR_LABYRINTH, 0, AN_LAB3_KEYS    , 0},
	{{"!riddle",                           "?", NULL},  3, AR_LABYRINTH, 0, AN_LAB3_RIDDLES , 0},
	// Key words ................................... , Dif,      Area, Tmp,         Answer, Spc		AR_LAB_LIZARD
	{{"!door",                             "?", NULL},  2, AR_LABYRINTH, 0, AN_LAB2_DOORS   , 0},
	{{"!key",                              "?", NULL},  2, AR_LABYRINTH, 0, AN_LAB2_KEYS    , 0},
	{{"!riddle",                           "?", NULL},  2, AR_LABYRINTH, 0, AN_LAB2_RIDDLES , 0},
	{{"!where", "!coconut", "?lizard",     "?", NULL},  0,AR_LAB_LIZARD, 400, AN_LAB2_COCO  , 0},
	{{"!where", "!potion", "?agility",     "?", NULL},  0,AR_LAB_LIZARD, 399, AN_LAB2_POTION, 0},
	{{"!where", "!teeth", "?lizard",       "?", NULL},  0,AR_LAB_LIZARD, 398, AN_LAB2_TEETH , 0},
	{{"!how", "?teeth", "!necklace",       "?", NULL},  0,AR_LAB_LIZARD, 398, AN_LAB2_NECK  , 0},
	// Key words ................................... , Dif,      Area, Tmp,         Answer, Spc		AR_LAB_GROLM
	{{"!door",                             "?", NULL},  1, AR_LABYRINTH, 0, AN_LAB1_DOORS   , 0},
	{{"!key",                              "?", NULL},  1, AR_LABYRINTH, 0, AN_LAB1_KEYS    , 0},
	{{"!riddle",                           "?", NULL},  1, AR_LABYRINTH, 0, AN_LAB1_RIDDLES , 0},
	// Key words ................................... , Dif,      Area, Tmp,         Answer, Spc		(generic labyrinth)
	{{"!assist", "?me",                    "?", NULL},  0, AR_LABYRINTH, 0, AN_LAB_HELP     , 0},
	{{"!who", "!are", "!you",              "?", NULL},  0, AR_LABYRINTH, 0, AN_LAB_WHO      , 0},
	//}
	//{ Generic Answers
	// Key words ................................... , Dif,      Area, Tmp,         Answer, Spc		
	{{"?what", "!hand",       "!skill",    "?", NULL}, 0, AR_GENERAL, 0, AN_SK_HAND, 0},
	{{"?what", "!precision",  "!skill",    "?", NULL}, 0, AR_GENERAL, 0, AN_SK_PUGILISM, 0},
	{{"?what", "!dagger",     "!skill",    "?", NULL}, 0, AR_GENERAL, 0, AN_SK_DAGGER, 0},
	{{"?what", "!sword",      "!skill",    "?", NULL}, 0, AR_GENERAL, 0, AN_SK_SWORD, 0},
	{{"?what", "!axe",        "!skill",    "?", NULL}, 0, AR_GENERAL, 0, AN_SK_AXE, 0},
	{{"?what", "!staff",      "!skill",    "?", NULL}, 0, AR_GENERAL, 0, AN_SK_STAFF, 0},
	{{"?what", "!twohand",    "!skill",    "?", NULL}, 0, AR_GENERAL, 0, AN_SK_TWOHAND, 0},
	{{"?what", "!focus",      "!skill",    "?", NULL}, 0, AR_GENERAL, 0, AN_SK_FOCUS, 0},
	{{"?what", "!stealth",    "!skill",    "?", NULL}, 0, AR_GENERAL, 0, AN_SK_STEALTH, 0},
	{{"?what", "!perception", "!skill",    "?", NULL}, 0, AR_GENERAL, 0, AN_SK_PERCEPT, 0},
	{{"?what", "!swimming",   "!skill",    "?", NULL}, 0, AR_GENERAL, 0, AN_SK_SWIM, 0},
	{{"?what", "!magic", "!shield",        "?", NULL}, 0, AR_GENERAL, 0, AN_SK_MSHIELD, 0},
	{{"?what", "!bartering",               "?", NULL}, 0, AR_GENERAL, 0, AN_SK_BARTER, 0},
	{{"?what", "!repair",                  "?", NULL}, 0, AR_GENERAL, 0, AN_SK_REPAIR, 0},
	{{"?what", "!light",                   "?", NULL}, 0, AR_GENERAL, 0, AN_SK_LIGHT, 0},
	{{"?what", "!recall",                  "?", NULL}, 0, AR_GENERAL, 0, AN_SK_RECALL, 0},
	{{"?what", "!shield",     "!skill",    "?", NULL}, 0, AR_GENERAL, 0, AN_SK_SHIELD, 0},
	{{"?what", "!protect",                 "?", NULL}, 0, AR_GENERAL, 0, AN_SK_PROTECT, 0},
	{{"?what", "!enhance",                 "?", NULL}, 0, AR_GENERAL, 0, AN_SK_ENHANCE, 0},
	{{"?what", "!slow",                    "?", NULL}, 0, AR_GENERAL, 0, AN_SK_SLOW, 0},
	{{"?what", "!curse",                   "?", NULL}, 0, AR_GENERAL, 0, AN_SK_CURSE, 0},
	{{"?what", "!bless",                   "?", NULL}, 0, AR_GENERAL, 0, AN_SK_BLESS, 0},
	{{"?what", "!identify",                "?", NULL}, 0, AR_GENERAL, 0, AN_SK_IDENTIFY, 0},
	{{"?what", "!resistance",              "?", NULL}, 0, AR_GENERAL, 0, AN_SK_RESIST, 0},
	{{"?what", "!blast",                   "?", NULL}, 0, AR_GENERAL, 0, AN_SK_BLAST, 0},
	{{"?what", "!dispel",                  "?", NULL}, 0, AR_GENERAL, 0, AN_SK_DISPEL, 0},
	{{"?what", "!heal",                    "?", NULL}, 0, AR_GENERAL, 0, AN_SK_HEAL, 0},
	{{"?what", "?ghost", "!companion",     "?", NULL}, 0, AR_GENERAL, 0, AN_SK_COMPAN, 0},
	{{"?what", "!regenerate",              "?", NULL}, 0, AR_GENERAL, 0, AN_SK_REGEN, 0},
	{{"?what", "!rest",                    "?", NULL}, 0, AR_GENERAL, 0, AN_SK_REST, 0},
	{{"?what", "!meditate",                "?", NULL}, 0, AR_GENERAL, 0, AN_SK_MEDIT, 0},
	{{"?what", "!sense", "!magic",         "?", NULL}, 0, AR_GENERAL, 0, AN_SK_SENSE, 0},
	{{"?what", "!immunity",                "?", NULL}, 0, AR_GENERAL, 0, AN_SK_IMMUNITY, 0},
	{{"?what", "!surround", "!hit",        "?", NULL}, 0, AR_GENERAL, 0, AN_SK_SURROUND, 0},
	{{"?what", "!concentrate",             "?", NULL}, 0, AR_GENERAL, 0, AN_SK_CONCENT, 0},
	{{"?what", "!warcry",                  "?", NULL}, 0, AR_GENERAL, 0, AN_SK_WARCRY, 0},
	{{"?what", "!dual", "!wield", "!skill","?", NULL}, 0, AR_GENERAL, 0, AN_SK_DUALW, 0},
	{{"?what", "!combat", "!mastery",      "?", NULL}, 0, AR_GENERAL, 0, AN_SK_COMBATM, 0},
	{{"?what", "!weapon", "!mastery",      "?", NULL}, 0, AR_GENERAL, 0, AN_SK_WEAPONM, 0},
	{{"?what", "!armor", "!mastery",       "?", NULL}, 0, AR_GENERAL, 0, AN_SK_ARMORM, 0},
	{{"?what", "!cleave",                  "?", NULL}, 0, AR_GENERAL, 0, AN_SK_CLEAVE, 0},
	{{"?what", "!weaken",                  "?", NULL}, 0, AR_GENERAL, 0, AN_SK_WEAKEN, 0},
	{{"?what", "!poison",                  "?", NULL}, 0, AR_GENERAL, 0, AN_SK_POISON, 0},
	{{"?what", "!damage", "!proximity",    "?", NULL}, 0, AR_GENERAL, 0, AN_SK_DAMPROX, 0},
	{{"?what", "!hex", "!proximity",       "?", NULL}, 0, AR_GENERAL, 0, AN_SK_HEXPROX, 0},
	{{"?what", "!companion", "!mastery",   "?", NULL}, 0, AR_GENERAL, 0, AN_SK_COMPM, 0},
	{{"?what", "!shadow", "!copy",         "?", NULL}, 0, AR_GENERAL, 0, AN_SK_SHADOW, 0},
	{{"?what", "!haste",                   "?", NULL}, 0, AR_GENERAL, 0, AN_SK_HASTE, 0},
	{{"?what", "!surround", "!area",       "?", NULL}, 0, AR_GENERAL, 0, AN_SK_SURRAREA, 0},
	{{"?what", "!surround", "!rate",       "?", NULL}, 0, AR_GENERAL, 0, AN_SK_SURRRATE, 0},
	// Key words ................................... , Dif,      Area, Tmp,         Answer, Spc		
	{{"?what", "!templar",                 "?", NULL}, 0, AR_GENERAL, 0, AN_RA_TEMP, 0},
	{{"?what", "!mercenary",               "?", NULL}, 0, AR_GENERAL, 0, AN_RA_MERC, 0},
	{{"?what", "!harakim",                 "?", NULL}, 0, AR_GENERAL, 0, AN_RA_HARA, 0},
	{{"?what", "!seyandu",                 "?", NULL}, 0, AR_GENERAL, 0, AN_RA_SEYANDU, 0},
	{{"?what", "!arch", "!templar",        "?", NULL}, 0, AR_GENERAL, 0, AN_RA_ARCHTEMP, 0},
	{{"?what", "!brawler",                 "?", NULL}, 0, AR_GENERAL, 0, AN_RA_PUGILIST, 0},
	{{"?what", "!warrior",                 "?", NULL}, 0, AR_GENERAL, 0, AN_RA_WARRIOR, 0},
	{{"?what", "!sorcerer",                "?", NULL}, 0, AR_GENERAL, 0, AN_RA_SORCERER, 0},
	{{"?what", "!summoner",                "?", NULL}, 0, AR_GENERAL, 0, AN_RA_SUMMONER, 0},
	{{"?what", "!arch", "!harakim",        "?", NULL}, 0, AR_GENERAL, 0, AN_RA_ARCHHARA, 0},
	//}

/*
{{"?black", "!stronghold", "!coin", "?", NULL}, 25, AR_STRONGHOLD, 72,  
"Bring me one of the black candles from the Stronghold, and I'll give you the star part of the coin.", 0},
{{"?what", "!gate", "?", NULL}, 0, AR_GENERAL, 0,   
"The last gate of the Labyrinth.", 0},
{{"?what", "!labyrinth", "?", NULL}, 0, AR_GENERAL, 0,   
"The Labyrinth. It's a huge maze full of dangers. If you survive it, you'll become a Seyan'Du.", 0},
{{"?ice", "!egg", NULL}, 0, AR_NEST, 615, 
"I heard that the ice gargoyles guard an ice egg in their nest. I'd be most grateful if you could bring it to me. But hurry, before it melts!", 0},
{{"?ice", "!cloak", NULL}, 0, AR_NEST, 615, 
"The Ice Cloak is a fine piece of armor, but it melts in due time. I can give you one if you obtain an ice egg for me.", 0},
*/

	// Generic Special Answers
	{{"!how",	"!are",	"!you",	"?",	NULL},	0,	AR_GENERAL,	0,	NULL,	SP_HEALTH},
	{{"!who",	"!are",	"!you",	"?",	NULL},	0,	AR_GENERAL,	0,	NULL,	SP_WHOAMI},
	{{"!where",	"!are",	"!you",	"?",	NULL},	0,	AR_GENERAL,	0,	NULL,	SP_WHERE},
	{{"!where",	"!am",	"!i",	"?",	NULL},	0,	AR_GENERAL,	0,	NULL,	SP_WHERE},
	{{"!buy",							NULL},	0,	AR_GENERAL, 0,	NULL,	SP_SHOP},
	{{"!sell",							NULL},	0,	AR_GENERAL, 0,	NULL,	SP_SHOP},
	{{"!shop",							NULL},	0,	AR_GENERAL, 0,	NULL,	SP_SHOP},
	{{"!exit",							NULL},	0,	AR_GENERAL, 0,  "Enter the side-room of a tavern to leave the game, %s.", 0},
	{{"!hello",	"!",	"$",			NULL},	0,	AR_GENERAL,	0,	NULL,	SP_GREET},
	{{"!bye",	"!",	"$",			NULL},	0,	AR_GENERAL,	0,	"Goodbye, %s.", 0},
	{{"!thank",	"?you",	"!",	"$",	NULL},	0,	AR_GENERAL,	0,	"You're welcome, %s.", 0},
	{{"!thank",							NULL},	0,	AR_GENERAL,	0,	"You're welcome, %s.", 0},
	{{"!time",	"!what",	"?",		NULL},	0,	AR_GENERAL,	0,	NULL,	SP_TIME},
	
	// Ghost Companion Stuff	CT_COMPANION		
	{{"!command",		NULL},	0,	AR_GENERAL,	CT_COMPANION,	NULL, 	SP_COMMAND},
	{{"!stop",			NULL},	0,	AR_GENERAL,	CT_COMPANION,	NULL, 	SP_STOP},
	{{"!move","!north",	NULL},	0,	AR_GENERAL,	CT_COMPANION,	NULL, 	SP_MOVE_N},
	{{"!move","!east",	NULL},	0,	AR_GENERAL,	CT_COMPANION,	NULL, 	SP_MOVE_E},
	{{"!move","!south",	NULL},	0,	AR_GENERAL,	CT_COMPANION,	NULL, 	SP_MOVE_S},
	{{"!move","!west",	NULL},	0,	AR_GENERAL,	CT_COMPANION,	NULL, 	SP_MOVE_W},
	{{"!move",			NULL},	0,	AR_GENERAL,	CT_COMPANION,	NULL, 	SP_MOVE},
	{{"!attack",		NULL},	0,	AR_GENERAL, CT_COMPANION,	NULL, 	SP_ATTACK},
	{{"!wait",			NULL},	0,	AR_GENERAL, CT_COMPANION, 	NULL, 	SP_WAIT},
	{{"!follow",		NULL},	0,	AR_GENERAL, CT_COMPANION, 	NULL, 	SP_FOLLOW},
	{{"!transfer",		NULL},	0,	AR_GENERAL, CT_COMPANION, 	NULL, 	SP_TRANSFER},
	{{"!geronimo",		NULL},	0,	AR_GENERAL, CT_COMPANION, 	NULL, 	SP_SPELLINFO},
	{{"!quiet",			NULL},	0,	AR_GENERAL, CT_COMPANION, 	NULL, 	SP_QUIET},
	{{"!defense",		NULL},	0,	AR_GENERAL, CT_COMPANION, 	NULL, 	SP_DEFENSE},
	{{"!passive",		NULL},	0,	AR_GENERAL, CT_COMPANION, 	NULL, 	SP_PASSIVE},
	{{"!offense",		NULL},	0,	AR_GENERAL, CT_COMPANION, 	NULL, 	SP_OFFENSE},
	
	// Arch GC					CT_ARCHCOMP
	{{"!command",		NULL},	0,	AR_GENERAL,	CT_ARCHCOMP,	NULL, 	SP_COMMAND},
	{{"!stop",			NULL},	0,	AR_GENERAL, CT_ARCHCOMP, 	NULL, 	SP_STOP},
	{{"!move","!north",	NULL},	0,	AR_GENERAL,	CT_ARCHCOMP,	NULL, 	SP_MOVE_N},
	{{"!move","!east",	NULL},	0,	AR_GENERAL,	CT_ARCHCOMP,	NULL, 	SP_MOVE_E},
	{{"!move","!south",	NULL},	0,	AR_GENERAL,	CT_ARCHCOMP,	NULL, 	SP_MOVE_S},
	{{"!move","!west",	NULL},	0,	AR_GENERAL,	CT_ARCHCOMP,	NULL, 	SP_MOVE_W},
	{{"!move",			NULL},	0,	AR_GENERAL, CT_ARCHCOMP, 	NULL, 	SP_MOVE},
	{{"!attack",		NULL},	0,	AR_GENERAL, CT_ARCHCOMP, 	NULL, 	SP_ATTACK},
	{{"!wait",			NULL},	0,	AR_GENERAL, CT_ARCHCOMP, 	NULL, 	SP_WAIT},
	{{"!follow",		NULL},	0,	AR_GENERAL, CT_ARCHCOMP, 	NULL, 	SP_FOLLOW},
	{{"!transfer",		NULL},	0,	AR_GENERAL, CT_ARCHCOMP, 	NULL, 	SP_TRANSFER},
	{{"!geronimo",		NULL},	0,	AR_GENERAL, CT_ARCHCOMP, 	NULL, 	SP_SPELLINFO},
	{{"!quiet",			NULL},	0,	AR_GENERAL, CT_ARCHCOMP, 	NULL, 	SP_QUIET},
	{{"!defense",		NULL},	0,	AR_GENERAL, CT_ARCHCOMP, 	NULL, 	SP_DEFENSE},
	{{"!passive",		NULL},	0,	AR_GENERAL, CT_ARCHCOMP, 	NULL, 	SP_PASSIVE},
	{{"!offense",		NULL},	0,	AR_GENERAL, CT_ARCHCOMP, 	NULL, 	SP_OFFENSE},
	
	// Caster GC				CT_CASTERCOMP		
	{{"!command",		NULL},	0,	AR_GENERAL,	CT_CASTERCOMP,	NULL, 	SP_COMMAND},
	{{"!stop",			NULL},	0,	AR_GENERAL,	CT_CASTERCOMP,	NULL, 	SP_STOP},
	{{"!move","!north",	NULL},	0,	AR_GENERAL,	CT_CASTERCOMP,	NULL, 	SP_MOVE_N},
	{{"!move","!east",	NULL},	0,	AR_GENERAL,	CT_CASTERCOMP,	NULL, 	SP_MOVE_E},
	{{"!move","!south",	NULL},	0,	AR_GENERAL,	CT_CASTERCOMP,	NULL, 	SP_MOVE_S},
	{{"!move","!west",	NULL},	0,	AR_GENERAL,	CT_CASTERCOMP,	NULL, 	SP_MOVE_W},
	{{"!move",			NULL},	0,	AR_GENERAL,	CT_CASTERCOMP,	NULL, 	SP_MOVE},
	{{"!attack",		NULL},	0,	AR_GENERAL, CT_CASTERCOMP,	NULL, 	SP_ATTACK},
	{{"!wait",			NULL},	0,	AR_GENERAL, CT_CASTERCOMP, 	NULL, 	SP_WAIT},
	{{"!follow",		NULL},	0,	AR_GENERAL, CT_CASTERCOMP, 	NULL, 	SP_FOLLOW},
	{{"!transfer",		NULL},	0,	AR_GENERAL, CT_CASTERCOMP, 	NULL, 	SP_TRANSFER},
	{{"!geronimo",		NULL},	0,	AR_GENERAL, CT_CASTERCOMP, 	NULL, 	SP_SPELLINFO},
	{{"!quiet",			NULL},	0,	AR_GENERAL, CT_CASTERCOMP, 	NULL, 	SP_QUIET},
	{{"!defense",		NULL},	0,	AR_GENERAL, CT_CASTERCOMP, 	NULL, 	SP_DEFENSE},
	{{"!passive",		NULL},	0,	AR_GENERAL, CT_CASTERCOMP, 	NULL, 	SP_PASSIVE},
	{{"!offense",		NULL},	0,	AR_GENERAL, CT_CASTERCOMP, 	NULL, 	SP_OFFENSE},
	
	// Arch Caster GC			CT_ARCHCASTER
	{{"!command",		NULL},	0,	AR_GENERAL,	CT_ARCHCASTER,	NULL, 	SP_COMMAND},
	{{"!stop",			NULL},	0,	AR_GENERAL, CT_ARCHCASTER, 	NULL, 	SP_STOP},
	{{"!move","!north",	NULL},	0,	AR_GENERAL,	CT_ARCHCASTER,	NULL, 	SP_MOVE_N},
	{{"!move","!east",	NULL},	0,	AR_GENERAL,	CT_ARCHCASTER,	NULL, 	SP_MOVE_E},
	{{"!move","!south",	NULL},	0,	AR_GENERAL,	CT_ARCHCASTER,	NULL, 	SP_MOVE_S},
	{{"!move","!west",	NULL},	0,	AR_GENERAL,	CT_ARCHCASTER,	NULL, 	SP_MOVE_W},
	{{"!move",			NULL},	0,	AR_GENERAL, CT_ARCHCASTER, 	NULL, 	SP_MOVE},
	{{"!attack",		NULL},	0,	AR_GENERAL, CT_ARCHCASTER, 	NULL, 	SP_ATTACK},
	{{"!wait",			NULL},	0,	AR_GENERAL, CT_ARCHCASTER, 	NULL, 	SP_WAIT},
	{{"!follow",		NULL},	0,	AR_GENERAL, CT_ARCHCASTER, 	NULL, 	SP_FOLLOW},
	{{"!transfer",		NULL},	0,	AR_GENERAL, CT_ARCHCASTER, 	NULL, 	SP_TRANSFER},
	{{"!geronimo",		NULL},	0,	AR_GENERAL, CT_ARCHCASTER, 	NULL, 	SP_SPELLINFO},
	{{"!quiet",			NULL},	0,	AR_GENERAL, CT_ARCHCASTER, 	NULL, 	SP_QUIET},
	{{"!defense",		NULL},	0,	AR_GENERAL, CT_ARCHCASTER, 	NULL, 	SP_DEFENSE},
	{{"!passive",		NULL},	0,	AR_GENERAL, CT_ARCHCASTER, 	NULL, 	SP_PASSIVE},
	{{"!offense",		NULL},	0,	AR_GENERAL, CT_ARCHCASTER, 	NULL, 	SP_OFFENSE},
	
	// Riddle Stuff
	{{"!riddle",	NULL},	10,	AR_LAB_RIDDLE1, 	899, 	AN_RIDDLE, 	0},
	{{"!riddle",	NULL},	10,	AR_LAB_RIDDLE2, 	905, 	AN_RIDDLE, 	0},
	{{"!riddle",	NULL},	10,	AR_LAB_RIDDLE3, 	911, 	AN_RIDDLE, 	0},
	{{"!riddle",	NULL},	10,	AR_LAB_RIDDLE4, 	912, 	AN_RIDDLE, 	0},
	{{"!riddle",	NULL},	10,	AR_LAB_RIDDLE5, 	913, 	AN_RIDDLE, 	0}
};

int obey(int cn, int co)
{
	if (ch[cn].data[63]==co)
	{
		return( 1);
	}
	if ((ch[cn].data[26] & ch[co].kindred) && (ch[cn].data[28] & 1))
	{
		return( 2);
	}
	return(0);
}

void answer_spellinfo(int cn, int co)
{
	int n, in, found = 0;

	if (obey(cn, co)==1)
	{
		for (n = 0; n<MAXBUFFS; n++)
		{
			if ((in = ch[cn].spell[n]))
			{
				do_sayx(cn, "%s, for %dm %ds.",
				        bu[in].name, bu[in].active / (18 * 60), (bu[in].active / 18) % 60);
				found = 1;
			}
		}
		if (!found)
		{
			do_sayx(cn, "I have no spells on me at the moment.");
		}
	}
}

void answer_transfer(int cn, int co)
{
	int removeshadow = 0, n, in;
	if (obey(cn, co)==1)
	{
		if (ch[cn].flags & CF_SHADOWCOPY) 
		{
			ch[co].data[CHD_SHADOWCOPY] = 0;
			removeshadow = 1;
		}
		else
		{
			ch[co].data[CHD_COMPANION] = 0;
		}
		do_sayx(cn, "I'd prefer to die in battle, %s. But I shall obey my master.", ch[co].name);
		do_give_exp(co, ch[cn].data[28], 1, -1);
		fx_add_effect(6, 0, ch[co].x, ch[co].y, 0);
		fx_add_effect(7, 0, ch[cn].x, ch[cn].y, 0);
		die_companion(cn);
		ch[co].luck--;
		
		// Remove the shadow indicator from the buff list
		if (removeshadow) for (n = 0; n<MAXBUFFS; n++)
		{
			if ((in = ch[co].spell[n])==0)	continue;
			if (bu[in].temp == SK_SHADOW)
			{
				bu[in].used = USE_EMPTY;
				ch[co].spell[n] = 0;
				do_update_char(co);
				return;
			}
		}
	}
}

void answer_follow(int cn, int co)
{
	int n;

	if (obey(cn, co)==1)
	{
		for (n = 80; n<92; n++)
		{
			ch[cn].data[n] = 0;
		}

		ch[cn].attack_cn = 0;
		ch[cn].goto_x = ch[cn].goto_y = 0;
		ch[cn].misc_action = 0;

		ch[cn].data[69] = co;
		ch[cn].data[29] = 0;

		do_sayx(cn, "Yes, %s!", ch[co].name);
	}
}

void answer_wait(int cn, int co)
{
	int n;

	if (obey(cn, co)==1)
	{
		for (n = 80; n<92; n++)
		{
			ch[cn].data[n] = 0;
		}

		ch[cn].attack_cn = 0;
		ch[cn].goto_x = ch[cn].goto_y = 0;
		ch[cn].misc_action = 0;

		ch[cn].data[29] = ch[cn].x + ch[cn].y * MAPX;
		ch[cn].data[30] = ch[cn].dir;

		ch[cn].data[69] = 0;

		do_sayx(cn, "Yes, %s!", ch[co].name);
	}
}

void answer_gccommand(int cn)
{
	do_char_log(cn, 1, "Now listing valid Ghost Companion Commands:\n");
	do_char_log(cn, 1, " \n");
	//                 "!        .         |         .         .        !"
	do_char_log(cn, 1, "ATTACK <name>  to attack <name>.\n");
	do_char_log(cn, 1, "FOLLOW         to start following you.\n");
	do_char_log(cn, 1, "GERONIMO       to tell you how I'm feeling.\n");
	do_char_log(cn, 1, "MOVE           to try to step away.\n");
	do_char_log(cn, 1, "QUIET          to stop talking.\n");
	do_char_log(cn, 1, "STOP           to stop what I'm doing.\n");
	do_char_log(cn, 1, "TRANSFER       to transfer my exp to you.\n");
	do_char_log(cn, 1, "WAIT           to stop following you.\n");
	do_char_log(cn, 1, " \n");
	do_char_log(cn, 1, "DEFENSE        I will defend you. (DEFAULT)\n");
	do_char_log(cn, 1, "PASSIVE        I will not fight.\n");
	do_char_log(cn, 1, "OFFENSE        I will focus on enemies.\n");
	do_char_log(cn, 1, " \n");
}

void answer_stop(int cn, int co)
{
	int n;

	if (obey(cn, co))
	{
		for (n = 80; n<92; n++)
		{
			ch[cn].data[n] = 0;
		}

		ch[cn].attack_cn = 0;
		ch[cn].goto_x = ch[cn].goto_y = 0;
		ch[cn].misc_action = 0;

		ch[cn].data[78] = 0;
		ch[cn].data[27] = globs->ticker;

		do_sayx(cn, "Yes master %s!", ch[co].name);
	}
}

void answer_move(int cn, int co, int dir)
{
	if (obey(cn, co))
	{
		ch[cn].attack_cn = 0;
		ch[cn].misc_action = 0;
		
		if (dir)
		{
			switch (dir)
			{
				case 1: // North
					ch[cn].goto_x = ch[cn].x-2;
					ch[cn].goto_y = ch[cn].y;
					break;
				case 2: // East
					ch[cn].goto_x = ch[cn].x;
					ch[cn].goto_y = ch[cn].y+2;
					break;
				case 3: // South
					ch[cn].goto_x = ch[cn].x+2;
					ch[cn].goto_y = ch[cn].y;
					break;
				case 4: // West
					ch[cn].goto_x = ch[cn].x;
					ch[cn].goto_y = ch[cn].y-2;
					break;
			}
		}
		else
		{
			ch[cn].goto_x = ch[cn].x + 4 - RANDOM(9);
			ch[cn].goto_y = ch[cn].y + 4 - RANDOM(9);
		}
		
		do_sayx(cn, "Yes master %s!", ch[co].name);
	}
}

void answer_gcmode(int cn, int co, int mode)
{
	if (ch[cn].data[1]!=3) ch[cn].data[1] = mode;
	switch (mode)
	{
		case 1: 	do_sayx(cn, "Yes %s, I will now remain passive.", ch[co].name); break;
		case 2: 	do_sayx(cn, "Yes %s, I will now attack aggressive enemies.", ch[co].name); break;
		case 3: 	do_sayx(cn, "Yes %s, I will now attack all enemies.", ch[co].name); break;
		default: 	do_sayx(cn, "Yes %s, I will now defend you.", ch[co].name); break;
	}
}

void answer_attack(int cn, int co, char *text)
{
	int n, best = 9999, bestn = 0, dist, idx;
	char name[50];

	if (obey(cn, co))
	{
		while (isalpha(*text))
		{
			text++;
		}
		while (isspace(*text))
		{
			text++;
		}

		for (n = 0; n<45 && *text; name[n++] = *text++)
		{
			;
		}
		name[n] = 0;
		if (n<1)
		{
			return;
		}

		for (n = 1; n<MAXCHARS; n++)
		{
			if (ch[n].used!=USE_ACTIVE)
			{
				continue;
			}
			if (ch[n].flags & CF_BODY)
			{
				continue;
			}
			if (!strcasecmp(ch[n].name, name))
			{
				dist = abs(ch[cn].x - ch[n].x) + abs(ch[cn].y - ch[n].y);
				if (dist<best)
				{
					best  = dist;
					bestn = n;
				}
			}
		}

		if (bestn && best<40)
		{
			/* CS, 000209: Prevent attacks on self */
			if (bestn == co)
			{
				do_sayx(cn, "But %s, I would never attack you!", ch[co].name);
				return;
			}
			if (bestn == cn)
			{
				do_sayx(cn, "You want me to attack myself? That's silly, %s!", ch[co].name);
				return;
			}
			if (!may_attack_msg(co, bestn, 0))
			{
				do_sayx(cn, "The Gods would be angry if we did that, you didn't want to anger the Gods, %s did you?.", ch[co].name);
				return;
			}
			ch[cn].attack_cn = bestn;
			idx = bestn | (char_id(bestn) << 16);
			ch[cn].data[80] = idx;
			do_sayx(cn, "Yes %s, I will kill %s!", ch[co].name, ch[bestn].reference);
//                      do_sayx(cn,ch[cn].text[1],ch[bestn].name);
			do_notify_char(bestn, NT_GOTMISS, co, 0, 0, 0);
		}
	}
}

void answer_quiet(int cn, int co)
{
	if (!ch[cn].data[CHD_TALKATIVE])
	{
		ch[cn].data[CHD_TALKATIVE] = ch_temp[ch[cn].temp].data[CHD_TALKATIVE];
		do_sayx(cn, "Thank you, %s, for letting me talk again!", ch[co].name);
	}
	else
	{
		do_sayx(cn, "Yes %s, I will shut up now.", ch[co].name);
		ch[cn].data[CHD_TALKATIVE] = 0;
	}
}

void answer_quest(int cn, int co)
{
	// Innkeeper will let you know what quest(s) are next to do, based on what you have learned already.
	if (!ch[co].skill[12][0])
	{
		do_sayx(cn, "Jamil seems down on his luck. You can find him here in the Tavern, at the table over there.");
		return;
	}
	if (!ch[co].skill[15][0])
	{
		do_sayx(cn, "I hear Inga is in need of help. She's a harakim and she lives on First Street.");
		return;
	}
	if (((ch[co].kindred & (KIN_TEMPLAR | KIN_ARCHTEMPLAR | KIN_PUGILIST)) && !ch[co].skill[38][0])
	 || ((ch[co].kindred & (KIN_HARAKIM | KIN_ARCHHARAKIM | KIN_SUMMONER | KIN_MERCENARY | KIN_WARRIOR | KIN_SORCERER)) && !ch[co].skill[18][0])
	 || ((ch[co].kindred & KIN_SEYAN_DU) && !ch[co].skill[38][0] && !ch[co].skill[18][0]))
	{
		do_sayx(cn, "I hear Sirjan is in need of help. He's mercenary and he lives on First Street.");
		return;
	}
	if (((ch[co].kindred & (KIN_TEMPLAR | KIN_ARCHTEMPLAR | KIN_PUGILIST)) && !ch[co].skill[41][0])
	 || ((ch[co].kindred & (KIN_HARAKIM | KIN_ARCHHARAKIM | KIN_SUMMONER | KIN_MERCENARY | KIN_WARRIOR | KIN_SORCERER)) && !ch[co].skill[19][0])
	 || ((ch[co].kindred & KIN_SEYAN_DU) && !ch[co].skill[41][0] && !ch[co].skill[19][0])
	)
	{
		do_sayx(cn, "I hear Amity is in need of help. She's a harakim and you can find her in Lynbore Library.");
		return;
	}
	if (!ch[co].skill[13][0])
	{
		do_sayx(cn, "I hear Jefferson is in need of help. He's a templar and he lives on Second Street.");
		return;
	}
	if (!(ch[co].flags & CF_LOCKPICK))
	{
		do_sayx(cn, "I hear Steven is in need of help. He's a mercenary and he lives on Second Street.");
		return;
	}
	if (((ch[co].kindred & (KIN_TEMPLAR | KIN_ARCHTEMPLAR | KIN_PUGILIST | KIN_MERCENARY | KIN_WARRIOR | KIN_SORCERER)) && !ch[co].skill[32][0])
	 || ((ch[co].kindred & (KIN_HARAKIM | KIN_ARCHHARAKIM | KIN_SUMMONER)) && !ch[co].skill[25][0])
	 || ((ch[co].kindred & KIN_SEYAN_DU) && !ch[co].skill[32][0] && !ch[co].skill[25][0]))
	{
		do_sayx(cn, "I hear Ingrid is in need of help. She's a mercenary and she lives on Castle Way.");
		return;
	}
	if (((ch[co].kindred & (KIN_TEMPLAR | KIN_ARCHTEMPLAR | KIN_PUGILIST)) && !ch[co].skill[33][0])
	 || ((ch[co].kindred & (KIN_HARAKIM | KIN_ARCHHARAKIM | KIN_SUMMONER | KIN_MERCENARY | KIN_WARRIOR | KIN_SORCERER)) && !ch[co].skill[20][0])
	 || ((ch[co].kindred & KIN_SEYAN_DU) && !ch[co].skill[33][0] && !ch[co].skill[20][0])
	)
	{
		do_sayx(cn, "I hear Leopold is in need of help. He's a harakim and he lives on Castle Way.");
		return;
	}
	if (!ch[co].skill[26][0])
	{
		do_sayx(cn, "I hear Gunther is in need of help. He's a mercenary and he lives on Castle Way.");
		return;
	}
	if (!ch[co].skill[31][0])
	{
		do_sayx(cn, "I hear Manfred is in need of help. He's a templar and he lives on Silver Avenue.");
		return;
	}
	if (!ch[co].skill[23][0])
	{
		do_sayx(cn, "I hear Serena is in need of help. She's a templar and she lives on Second Street.");
		return;
	}
	if (((ch[co].kindred & (KIN_TEMPLAR | KIN_ARCHTEMPLAR | KIN_PUGILIST)) && !ch[co].skill[37][0])
	 || ((ch[co].kindred & (KIN_HARAKIM | KIN_ARCHHARAKIM | KIN_SUMMONER | KIN_MERCENARY | KIN_WARRIOR | KIN_SORCERER)) && !ch[co].skill[21][0])
	 || ((ch[co].kindred & KIN_SEYAN_DU) && !ch[co].skill[37][0] && !ch[co].skill[21][0])
	)
	{
		do_sayx(cn, "I hear Cirrus is in need of help. You can find him here in the tavern, down the hall in that room over there.");
		return;
	}
	if (!ch[co].skill[29][0])
	{
		do_sayx(cn, "I hear Gordon is in need of help. He's a harakim and he lives on Shore Crescent.");
		return;
	}
	if (((ch[co].kindred & (KIN_TEMPLAR | KIN_ARCHTEMPLAR | KIN_PUGILIST | KIN_MERCENARY | KIN_WARRIOR | KIN_SORCERER)) && !ch[co].skill[16][0])
	 || ((ch[co].kindred & (KIN_HARAKIM | KIN_ARCHHARAKIM | KIN_SUMMONER)) && !ch[co].skill[5][0])
	 || ((ch[co].kindred & KIN_SEYAN_DU) && !ch[co].skill[16][0] && !ch[co].skill[5][0])
	)
	{
		do_sayx(cn, "I hear Edna is in need of help. She's a templar and she lives on Shore Crescent.");
		return;
	}
	if (!ch[co].skill[22][0])
	{
		do_sayx(cn, "I hear Nasir is in need of help. He's a harakim and he lives on Shore Crescent.");
		return;
	}
	do_sayx(cn, "%s, you've done an excellent job in Lynbore! Everyone I've spoken to sings praise of your fine work!", ch[co].name);
	do_sayx(cn, "If you're looking for work, perhaps you should travel to Aston, the capital of Astonia.");
}

void answer_tarot(int cn, int co)
{
	int in = 0;
	
	if (in = ch[co].worn[WN_CHARM])
	{
		do_sayx(cn, "Very well, %s, I have removed your %s for you.", ch[co].name, it[in].name);
		if (god_give_char(in, co))
		{
			ch[co].worn[WN_CHARM] = 0;
			do_char_log(co, 1, "%s returned the %s to your inventory.\n", ch[cn].reference, it[in].name);
			fx_add_effect(6, 0, ch[co].x, ch[co].y, 0);
			remove_spells(co);
		}
		else
		{
			do_sayx(cn, "Or, erm... I would, but your inventory seems a bit full. Please come back less encumbered!");
		}
	}
	else
	{
		do_sayx(cn, "But you do not have a card equipped, %s!", ch[co].name);
	}
}

void answer_tarot2(int cn, int co)
{
	int in = 0;
	
	if (in = ch[co].worn[WN_CHARM2])
	{
		do_sayx(cn, "Very well, %s, I have removed your %s for you.", ch[co].name, it[in].name);
		if (god_give_char(in, co))
		{
			ch[co].worn[WN_CHARM2] = 0;
			do_char_log(co, 1, "%s returned the %s to your inventory.\n", ch[cn].reference, it[in].name);
			fx_add_effect(6, 0, ch[co].x, ch[co].y, 0);
			remove_spells(co);
		}
		else
		{
			do_sayx(cn, "Or, erm... I would, but your inventory seems a bit full. Please come back less encumbered!");
		}
	}
	else
	{
		do_sayx(cn, "But you do not have a secondary card equipped, %s!", ch[co].name);
	}
}

void answer_health(int cn, int co)
{
	if (ch[cn].a_hp>ch[cn].hp[5] * 550)
	{
		do_sayx(cn, "I'm fine, %s.", ch[co].name);
	}
	else if (ch[cn].a_hp>ch[cn].hp[5] * 250)
	{
		do_sayx(cn, "I don't feel so good, %s.", ch[co].name);
	}
	else
	{
		do_sayx(cn, "I'm dying!!");
	}
}

void answer_shop(int cn, int co)
{
	if (ch[cn].flags & CF_MERCHANT)
	{
		do_sayx(cn, "Hold down CTRL or ALT and right click on me to buy or sell, %s.", ch[co].name);
	}
	else
	{
		do_sayx(cn, "I'm not a merchant, %s.", ch[co].name);
	}
}

void answer_greeting(int cn, int co)
{
/*	changed by SoulHunter 25.04.2000	*/
/*	simplified by DB 1.5.2000 */
	if (ch[cn].text[2][0] && ch[cn].text[2][0]!='#')
	{
		if((ch[cn].temp == 180) && (ch[co].kindred & KIN_PURPLE))
		{
			do_sayx(cn, "Greetings, %s!", ch[co].name);
			return;
		}

		do_sayx(cn, ch[cn].text[2], ch[co].name);
		return;
	}
/*	--end	*/
}

void answer_whoami(int cn, int co)
{
	do_sayx(cn, "I am %s.", ch[cn].name);
}

void answer_where(int cn, int co)
{
	do_sayx(cn, get_area(cn, 1));
}

void answer_time(int cn, int co)
{
	do_sayx(cn, "Today is the %d%s%s%s%s day of the Year %d. It is %d:%02d Astonian Standard Time.\n",
	        globs->mdday,
	        (globs->mdday==1 ? "st" : ""),
	        (globs->mdday==2 ? "nd" : ""),
	        (globs->mdday==3 ? "rd" : ""),
	        (globs->mdday>3 ? "th" : ""),
	        globs->mdyear, globs->mdtime / 3600, (globs->mdtime / 60) % 60);
}

// Cost table for Black Stronghold Rewards...

#define BS_COST_GOLD		   2
#define BS_COST_EXP			  56
#define BS_COST_HEAL		   5
#define BS_COST_MANA		  10
#define BS_COST_END			  15
#define BS_COST_GHEAL		  20
#define BS_COST_GMANA		  40
#define BS_COST_GEND		  60
#define BS_COST_RATLING		 400
#define BS_COST_GREENLING	1600
#define BS_COST_DREADLING	6400
#define BS_COST_WEAPON1		  20
#define BS_COST_WEAPON2		  50
#define BS_COST_WEAPON3		 125
#define BS_COST_HELMET		  15
#define BS_COST_ARMOR		  15

// </>

int stronghold_points(int cn)
{
	return(ch[cn].data[26] / 5 	// tier 1
			+
			ch[cn].data[27] 		// tier 2
			+
			ch[cn].data[28] * 5 	// tier 3
			//+
			//ch[cn].data[43] * 25 	// candles
			-
			ch[cn].data[41]);		// used points
}

void answer_table(int cn)
{
	int cnrank = points2rank(ch[cn].points_tot);
	
	do_char_log(cn, 1, "Now listing Black Stronghold rewards:\n");
	do_char_log(cn, 1, " \n");
	//                 "!        .         .   |     .         .        !"
	do_char_log(cn, 1, "Say Phrase... | Cost  | Reward... \n");
	do_char_log(cn, 1, "--------------+-------+------------------------\n");
	do_char_log(cn, 1, "BUY GOLD        *ALL*   %d gold per point\n", BS_COST_GOLD);
	do_char_log(cn, 1, "BUY EXP         *ALL*   %d exp per point\n", BS_COST_EXP+2*cnrank);
	do_char_log(cn, 1, "BUY HEAL         %4d   1 Healing Potion\n", BS_COST_HEAL);
	do_char_log(cn, 1, "BUY MANA         %4d   1 Mana Potion\n", BS_COST_MANA);
	do_char_log(cn, 1, "BUY END          %4d   1 Endurance Potion\n", BS_COST_END);
	do_char_log(cn, 1, "BUY GHEAL        %4d   1 Greater Healing\n", BS_COST_GHEAL);
	do_char_log(cn, 1, "BUY GMANA        %4d   1 Greater Mana\n", BS_COST_GMANA);
	do_char_log(cn, 1, "BUY GEND         %4d   1 Greater Endurance\n", BS_COST_GEND);
	do_char_log(cn, 1, "BUY RATLING      %4d   1 Ratling Eye Essence\n", BS_COST_RATLING);
	do_char_log(cn, 1, "BUY GREENLING    %4d   1 Greenling Eye Essence\n", BS_COST_GREENLING);
  //do_char_log(cn, 1, "BUY SOGLING      %4d   1 Sogling Eye Essence\n", BS_COST_DREADLING);
	do_char_log(cn, 1, "BUY WEAPON1      %4d   Magic weapon, +1 skill\n", BS_COST_WEAPON1+BS_COST_WEAPON1*cnrank);
	do_char_log(cn, 1, "BUY WEAPON2      %4d   Magic weapon, +2 skill\n", BS_COST_WEAPON2+BS_COST_WEAPON2*cnrank);
	do_char_log(cn, 1, "BUY WEAPON3      %4d   Magic weapon, +3 skill\n", BS_COST_WEAPON3+BS_COST_WEAPON3*cnrank);
	do_char_log(cn, 1, "BUY HELMET       %4d   Magic helmet\n", BS_COST_HELMET+BS_COST_HELMET*cnrank*2);
	do_char_log(cn, 1, "BUY ARMOR        %4d   Magic armor\n", BS_COST_ARMOR+BS_COST_ARMOR*cnrank*2);
	do_char_log(cn, 1, " \n");
}

void answer_points(int cn, int co, int nr)
{
	int exp, pts;
	pts = stronghold_points(co);
	do_sayx(cn, know[nr].answer, pts);
}

// -------- <black stronghold points> -------- //

void answer_buy_gold(int cn, int co)
{
	int pts; pts = stronghold_points(co);
	if (pts<1)	{ do_sayx(cn, "But you don't have any points to spend, %s!", ch[co].name); return; }
	ch[co].data[41] += pts; ch[co].gold += pts * BS_COST_GOLD*100;
	do_sayx(cn, "There you are, %s. %d gold coins. Thank you for your help!", ch[co].name, pts*BS_COST_GOLD);
	chlog(co, "bought %d gold from cityguard (%d pts)", pts*BS_COST_GOLD, pts);
}

void answer_buy_exp(int cn, int co)
{
	int corank = points2rank(ch[co].points_tot);
	int pts; pts = stronghold_points(co);
	if (pts<1) { do_sayx(cn, "But you don't have any points to spend, %s!", ch[co].name); return; }
	ch[co].data[41] += pts;	ch[co].points += pts * (BS_COST_EXP+2*corank);	
	ch[co].points_tot += pts * (BS_COST_EXP+2*corank);	do_check_new_level(co);
	do_sayx(cn, "Now I'll teach you a bit about life, the world and everything, %s. Thank you for your help!", ch[co].name);
	do_char_log(co, 2, "You get %d experience points.\n", pts*(BS_COST_EXP+2*corank));
	chlog(co, "bought %d exps from cityguard (%d pts)", pts*(BS_COST_EXP+2*corank), pts);
}

void answer_buy_potion(int cn, int co, int in, int cost)
{
	int in2, pts; pts = stronghold_points(co);
	if (pts<cost) { do_sayx(cn, "But you don't have enough points to spend, %s!", ch[co].name); return; }
	in2 = god_create_item(in, 0); 
	if (!god_give_char(in2, co))
	{
		do_sayx(cn, "I'm sorry, %s. Your backpack is full. Please clear some room first!", ch[co].name);
		it[in2].used = USE_EMPTY;
		return;
	}
	ch[co].data[41] += cost; 
	do_sayx(cn, "There you are, %s. One %s. Thank you for your help!", ch[co].name, it[in2].reference);
	chlog(co, "bought %s from cityguard", it[in].reference);
}

int answer_best_weapon_reward(int co, int in)
{
	int chk_wil, chk_int, chk_agl, chk_str, gear;
	static int w_dagger[6] = { 693, 572, 541, 532, 523, 284 };
	static int w_staff[6]  = { 694, 573, 542, 533, 524, 285 };
	static int w_spear[6]  = { 695, 574, 543, 534, 525, 286 };
	static int w_sword[6]  = { 697, 576, 545, 536, 527, 288 };
	static int w_axe[6]    = { 699, 578, 547, 538, 529, 290 };
	static int w_twoh[6]   = { 700, 579, 548, 539, 530, 291 };
	static int w_great[6]  = { 701, 580, 549, 540, 531, 292 };
	
	chk_wil = ch[co].attrib[AT_WIL][0];
	chk_int = ch[co].attrib[AT_INT][0];
	chk_agl = ch[co].attrib[AT_AGL][0];
	chk_str = ch[co].attrib[AT_STR][0];
	
	gear = 5;
	
	if (it[in].flags & IF_WP_SWORD)
	{
		if			(chk_agl >= 96 && chk_str >= 82) gear = 0;	// Adamant
		else if	(chk_agl >= 72 && chk_str >= 60) gear = 1;	// Titanium
		else if	(chk_agl >= 52 && chk_str >= 42) gear = 2;	// Crystal
		else if	(chk_agl >= 36 && chk_str >= 28) gear = 3;	// Emerald
		else if	(chk_agl >= 24 && chk_str >= 18) gear = 4;	// Gold
		return 	(w_sword[gear]);
	}
	else if ((it[in].flags & IF_WP_DAGGER) && (it[in].flags & IF_WP_STAFF)) // Spear
	{
		if			(chk_wil >=105 && chk_str >= 30) gear = 0;	// Adamant
		else if	(chk_wil >= 90 && chk_str >= 30) gear = 1;	// Titanium
		else if	(chk_wil >= 64 && chk_str >= 22) gear = 2;	// Crystal
		else if	(chk_wil >= 42 && chk_str >= 16) gear = 3;	// Emerald
		else if	(chk_wil >= 25 && chk_str >= 12) gear = 4;	// Gold
		return 	(w_spear[gear]);
	}
	else if (it[in].flags & IF_WP_DAGGER)
	{
		if			(chk_wil >=110 && chk_agl >= 40) gear = 0;	// Adamant
		else if	(chk_wil >= 82 && chk_agl >= 30) gear = 1;	// Titanium
		else if	(chk_wil >= 58 && chk_agl >= 22) gear = 2;	// Crystal
		else if	(chk_wil >= 38 && chk_agl >= 16) gear = 3;	// Emerald
		else if	(chk_wil >= 22 && chk_agl >= 12) gear = 4;	// Gold
		return 	(w_dagger[gear]);
	}
	else if (it[in].flags & IF_WP_STAFF)
	{
		if			(chk_int >=110 && chk_str >= 40) gear = 0;	// Adamant
		else if	(chk_int >= 82 && chk_str >= 30) gear = 1;	// Titanium
		else if	(chk_int >= 58 && chk_str >= 22) gear = 2;	// Crystal
		else if	(chk_int >= 38 && chk_str >= 16) gear = 3;	// Emerald
		else if	(chk_int >= 22 && chk_str >= 12) gear = 4;	// Gold
		return 	(w_staff[gear]);
	}
	else if ((it[in].flags & IF_WP_AXE) && (it[in].flags & IF_WP_TWOHAND)) // Greataxe
	{
		if			(chk_agl >= 92 && chk_str >=124) gear = 0;	// Adamant
		else if	(chk_agl >= 68 && chk_str >= 90) gear = 1;	// Titanium
		else if	(chk_agl >= 48 && chk_str >= 62) gear = 2;	// Crystal
		else if	(chk_agl >= 32 && chk_str >= 40) gear = 3;	// Emerald
		else if	(chk_agl >= 20 && chk_str >= 24) gear = 4;	// Gold
		return 	(w_great[gear]);
	}
	else if (it[in].flags & IF_WP_AXE)
	{
		if			(chk_agl >= 82 && chk_str >=115) gear = 0;	// Adamant
		else if	(chk_agl >= 60 && chk_str >= 85) gear = 1;	// Titanium
		else if	(chk_agl >= 42 && chk_str >= 60) gear = 2;	// Crystal
		else if	(chk_agl >= 28 && chk_str >= 40) gear = 3;	// Emerald
		else if	(chk_agl >= 18 && chk_str >= 25) gear = 4;	// Gold
		return 	(w_axe[gear]);
	}
	else if (it[in].flags & IF_WP_TWOHAND)
	{
		if			(chk_agl >= 92 && chk_str >=105) gear = 0;	// Adamant
		else if	(chk_agl >= 68 && chk_str >= 80) gear = 1;	// Titanium
		else if	(chk_agl >= 48 && chk_str >= 58) gear = 2;	// Crystal
		else if	(chk_agl >= 32 && chk_str >= 40) gear = 3;	// Emerald
		else if	(chk_agl >= 20 && chk_str >= 26) gear = 4;	// Gold
		return 	(w_twoh[gear]);
	}
	
	return (697);
}

void answer_buy_weapon(int cn, int co, int stren, int cost)
{
	char buf[300];
	int in, in2, pts; pts = stronghold_points(co);
	if (pts<cost) { do_sayx(cn, "But you don't have enough points to spend, %s!", ch[co].name); return; }
	
	if (in = ch[co].worn[WN_RHAND])
	{
		in2 = god_create_item(answer_best_weapon_reward(co, in), 0);
		if (it[in2].flags & IF_WP_SWORD)
			it[in2].skill[SK_SWORD][0] += stren;
		else if ((it[in2].flags & IF_WP_DAGGER) && (it[in].flags & IF_WP_STAFF)) // Spear
		{
			it[in2].skill[SK_DAGGER][0] += stren;
			it[in2].skill[SK_STAFF][0] += stren;
		}
		else if (it[in2].flags & IF_WP_DAGGER)
			it[in2].skill[SK_DAGGER][0] += stren;
		else if (it[in2].flags & IF_WP_STAFF)
			it[in2].skill[SK_STAFF][0] += stren;
		else if ((it[in2].flags & IF_WP_AXE) && (it[in].flags & IF_WP_TWOHAND)) // Greataxe
		{
			it[in2].skill[SK_AXE][0] += stren;
			it[in2].skill[SK_TWOHAND][0] += stren;
		}
		else if (it[in2].flags & IF_WP_AXE)
			it[in2].skill[SK_AXE][0] += stren;
		else if (it[in2].flags & IF_WP_TWOHAND)
			it[in2].skill[SK_TWOHAND][0] += stren;
		
		it[in2].temp   = 0;
		it[in2].flags |= IF_UPDATE | IF_IDENTIFIED | IF_NOREPAIR | IF_SOULSTONE;
		
		if (!it[in2].max_damage)
			it[in2].max_damage = 3500*it[in2].weapon[0]*2;
		else
			it[in2].max_damage*=2;
		
		if (!god_give_char(in2, co))
		{
			do_sayx(cn, "I'm sorry, %s. Your backpack is full. Please clear some room first!", ch[co].name);
			it[in2].used = USE_EMPTY;
			return;
		}
		ch[co].data[41] += cost;
	}
	else
	{
		do_sayx(cn, "I'm sorry, %s, but I have no weapons for unarmed fighters!", ch[co].name); return;
	}
	do_sayx(cn, "There you are, %s. A +%d %s. Thank you for your help!", ch[co].name, stren, it[in2].reference);
	do_char_log(co, 0, "You feel yourself form a magical connection with the %s.\n", it[in2].reference);
	it[in2].data[0] = co;
	sprintf(buf, "%s Engraved in it are the letters \"%s\".", it[in2].description, ch[co].name);
	if (strlen(buf)<200)
	{
		strcpy(it[in2].description, buf);
	}
	chlog(co, "bought %s from cityguard", it[in].reference);
}

int answer_best_armor_reward(int co, int flag)
{
	int chk_wil, chk_int, chk_agl, chk_str, gear;
	static int a_helm[7] = { 94, 76, 71, 66, 61, 56, 51 };
	static int a_body[7] = { 96, 78, 73, 68, 63, 58, 53 };
	static int c_helm[4] = { 352, 347, 342, 337 };
	static int c_body[4] = { 354, 349, 344, 339 };
	
	chk_wil = ch[co].attrib[AT_WIL][0];
	chk_int = ch[co].attrib[AT_INT][0];
	chk_agl = ch[co].attrib[AT_AGL][0];
	chk_str = ch[co].attrib[AT_STR][0];
	
	gear = 6; 	// Bronze
	
	if ((chk_wil+chk_int)>(chk_agl+chk_str))
	{
		gear = 3; 	// Simple
		
		if			(chk_wil >= 90 && chk_int >= 90) gear = 0;	// Wizard
		else if	(chk_wil >= 60 && chk_int >= 60) gear = 1;	// Adept
		else if	(chk_wil >= 35 && chk_int >= 35) gear = 2;	// Caster
		
		if (flag)	return (c_body[gear]);
		else		return (c_helm[gear]);	
	}
	else if	(chk_agl >=105 && chk_str >=105) gear = 0;	// Adamantine
	else if	(chk_agl >= 84 && chk_str >= 84) gear = 1;	// Titanium
	else if	(chk_agl >= 64 && chk_str >= 64) gear = 2;	// Crystal
	else if	(chk_agl >= 48 && chk_str >= 48) gear = 3;	// Emerald
	else if	(chk_agl >= 34 && chk_str >= 34) gear = 4;	// Gold
	else if	(chk_agl >= 24 && chk_str >= 24) gear = 5;	// Steel
	
	if (flag)	return (a_body[gear]);
	return (a_helm[gear]);
}

void answer_buy_armor(int cn, int co, int flag, int cost)
{
	int in, pts; pts = stronghold_points(co);
	if (pts<cost) { do_sayx(cn, "But you don't have enough points to spend, %s!", ch[co].name); return; }
	in = create_special_item(answer_best_armor_reward(co, flag));
	if (!god_give_char(in, co))
	{
		do_sayx(cn, "I'm sorry, %s. Your backpack is full. Please clear some room first!", ch[co].name);
		it[in].used = USE_EMPTY;
		return;
	}
	ch[co].data[41] += cost; 
	do_sayx(cn, "There you are, %s. One magic %s. Thank you for your help!", ch[co].name, it[in].reference);
	chlog(co, "bought %s from cityguard", it[in].reference);
}

// -------- </black stronghold points> -------- //

void special_answer(int cn, int co, int spec, char *word, int nr)
{
	int corank = points2rank(ch[co].points_tot);
	switch(spec)
	{
		case SP_HEALTH:		answer_health(cn, co); break;
		case SP_SHOP: 		answer_shop(cn, co); break;
		case SP_GREET:		answer_greeting(cn, co); break;
		case SP_WHOAMI:		answer_whoami(cn, co); break;
		case SP_WHERE:		answer_where(cn, co); break;
		case SP_STOP:		answer_stop(cn, co); break;
		case SP_MOVE:		answer_move(cn, co, 0); break;
		case SP_MOVE_N:		answer_move(cn, co, 1); break;
		case SP_MOVE_E:		answer_move(cn, co, 2); break;
		case SP_MOVE_S:		answer_move(cn, co, 3); break;
		case SP_MOVE_W:		answer_move(cn, co, 4); break;
		case SP_ATTACK:		answer_attack(cn, co, word); break;
		case SP_WAIT:		answer_wait(cn, co); break;
		case SP_FOLLOW:		answer_follow(cn, co); break;
		case SP_TIME:		answer_time(cn, co); break;
		case SP_POINTS:		answer_points(cn, co, nr); break;
		case SP_TABLE:		answer_table(co); break;
		case SP_TRANSFER:	answer_transfer(cn, co); break;
		case SP_SPELLINFO:	answer_spellinfo(cn, co); break;
		case SP_QUIET:		answer_quiet(cn, co); break;
		case SP_QUEST:		answer_quest(cn, co); break;
		case SP_DEFENSE:	answer_gcmode(cn, co, 0); break;
		case SP_PASSIVE:	answer_gcmode(cn, co, 1); break;
		case SP_OFFENSE:	answer_gcmode(cn, co, 2); break;
		case SP_BERSERK:	answer_gcmode(cn, co, 3); break;
		case SP_COMMAND:	answer_gccommand(co); break;
		case SP_TAROT:		answer_tarot(cn, co); break;
		case SP_TAROT2:		answer_tarot2(cn, co); break;
		//
		case SP_BUY_GOLD:		answer_buy_gold(cn, co); break;
		case SP_BUY_EXP:		answer_buy_exp(cn, co); break;
		case SP_BUY_HEAL:		answer_buy_potion(cn, co, 101, BS_COST_HEAL); break;
		case SP_BUY_MANA:		answer_buy_potion(cn, co, 102, BS_COST_MANA); break;
		case SP_BUY_END:		answer_buy_potion(cn, co, 125, BS_COST_END); break;
		case SP_BUY_GHEAL:		answer_buy_potion(cn, co, 127, BS_COST_GHEAL); break;
		case SP_BUY_GMANA:		answer_buy_potion(cn, co, 131, BS_COST_GMANA); break;
		case SP_BUY_GEND:		answer_buy_potion(cn, co, 273, BS_COST_GEND); break;
		case SP_BUY_RATLING:	answer_buy_potion(cn, co, 267, BS_COST_RATLING); break;
		case SP_BUY_GREENLING:	answer_buy_potion(cn, co, 833, BS_COST_GREENLING); break;
	  //case SP_BUY_DREADLING:	answer_buy_potion(cn, co,1479, BS_COST_DREADLING); break;
		case SP_BUY_WEAPON1:	answer_buy_weapon(cn, co,   1, BS_COST_WEAPON1+BS_COST_WEAPON1*corank); break;
		case SP_BUY_WEAPON2:	answer_buy_weapon(cn, co,   2, BS_COST_WEAPON2+BS_COST_WEAPON2*corank); break;
		case SP_BUY_WEAPON3:	answer_buy_weapon(cn, co,   3, BS_COST_WEAPON3+BS_COST_WEAPON3*corank); break;
		case SP_BUY_HELMET:		answer_buy_armor(cn, co,    0, BS_COST_HELMET+BS_COST_HELMET*corank*2); break;
		case SP_BUY_ARMOR:		answer_buy_armor(cn, co,    1, BS_COST_ARMOR+BS_COST_ARMOR*corank*2); break;
		//
		default:break;
	}
}

// no user servicable parts below this line ;-)

void npc_hear(int cn, int co, char *text)
{
	char buf[512];
	char word[20][40];
	int  n, cnt, z, flag = 1, m;
	int  exclam = 0, question = 0, name = 0, misscost, found, hitcost, gotword[20];
	int  hit, miss, bestconf = 0, bestnr = -1, conf, talk;

	// got keyword meaning stop
	if (!strcasecmp(text, ch[cn].text[6]))
	{
		for (n = 80; n<92; n++)
		{
			ch[cn].data[n] = 0;
		}

		ch[cn].attack_cn = 0;
		ch[cn].goto_x = ch[cn].goto_y = 0;
		ch[cn].misc_action = 0;

		ch[cn].data[78] = 0;
		ch[cn].data[27] = globs->ticker;
		if (ch[cn].text[7][0])
		{
			do_sayx(cn, ch[cn].text[7]);
		}
		return;
	}

	// dont talk to enemies
	if (!obey(cn, co))
	{
		for (n = 80; n<92; n++)
		{
			if ((ch[cn].data[n] & 0xffff)==co)
			{
				return;
			}
		}
	}

	for (n = 0; n<20; n++)
	{
		word[n][0] = 0;
	}

	strcpy(buf, text);
	strlower(buf);

	for (z = n = cnt = 0; buf[z]; z++)
	{
		if (buf[z]=='!')
		{
			exclam++;
			continue;
		}
		if (buf[z]=='?')
		{
			question++;
			continue;
		}
		if (!isspace(buf[z]) && !isalnum(buf[z]))
		{
			continue;
		}

		if (!isspace(buf[z]) && n<39)
		{
			word[cnt][n++] = buf[z];
			flag = 0;
		}
		else
		{
			if (!flag)
			{
				word[cnt][n] = 0;
				if (is_fillword(word[cnt]))
				{
					;
				}
				else if (!strcasecmp(word[cnt], ch[cn].name))
				{
					name = 1;
				}
				else
				{
					cnt++;
				}
				n = 0;
				if (cnt==20)
				{
					break;
				}
				flag = 1;
			}
		}
	}
	if (n)
	{
		word[cnt][n] = 0;
		if (is_fillword(word[cnt]))
		{
			;
		}
		else if (!strcasecmp(word[cnt], ch[cn].name))
		{
			name = 1;
		}
		else
		{
			word[cnt][n] = 0;
			cnt++;
		}
	}

	for (n = 0; n<cnt; n++)
	{
		replace_synonym(word[n]);
	}

/*      for (n=0; n<cnt; n++) {
                do_sayx(cn,"%d: \"%s\".",cn,word[n]);
        }

        do_sayx(cn,"Question=%d, Exlam=%d, name=%d",question,exclam,name); */

	for (n = 0; n<sizeof(know) / sizeof(know[0]); n++)
	{
		if (ch[cn].data[68]>=know[n].value // NPC level of knowledge is >= "value" of the response
			&& 
			(ch[cn].data[72]==know[n].area || ch[cn].data[72]==AR_ALL || know[n].area==AR_GENERAL) // If the NPC knows the area, or if it is general knowledge
			&& 
			(!know[n].temp || know[n].temp==ch[cn].temp)) // If the template is 0, or the template matches the NPC template number
		{

			hit = miss = 0;

			for (z = 0; z<cnt; z++)
			{
				gotword[z] = 0;
			}

			for (m = 0; know[n].word[m]; m++)
			{
				if (know[n].word[m][1]==0)
				{
					found = 0;
					if (know[n].word[m][0]=='?')
					{
						if (question)
						{
							found = 1;
						}
					}
					else if (know[n].word[m][0]=='!')
					{
						if (exclam)
						{
							found = 1;
						}
					}
					else if (know[n].word[m][0]=='$')
					{
						if (name)
						{
							found = 1;
						}
					}
					if (found)
					{
						hit++;
					}
					else
					{
						miss++;
					}
				}
				else
				{
					if (know[n].word[m][0]=='?')
					{
						misscost = 1;
						hitcost  = 1;
					}
					else if (know[n].word[m][0]=='!')
					{
						misscost = 5;
						hitcost  = 2;
					}
					else
					{
						hitcost = misscost = 0;
					}

					for (z = 0; z<cnt; z++)
					{
						if (!strcmp(word[z], know[n].word[m] + 1))
						{
							gotword[z] = 1;
							break;
						}
					}
					if (z==cnt)
					{
						miss += misscost;
					}
					else
					{
						hit += hitcost;
					}
				}
			}
			for (z = 0; z<cnt; z++)
			{
				if (gotword[z])
				{
					hit++;
				}
				else
				{
					miss++;
				}
			}
			conf = hit - miss;
			if (conf>bestconf)
			{
				bestconf = conf;
				bestnr = n;
			}
//                      do_sayx(cn,"%d: %s",conf,know[n].answer);
		}
	}

	talk = ch[cn].data[CHD_TALKATIVE] + name;
	if (obey(cn, co))
	{
		talk += 20;
	}

	if (talk>0)     // only talk if we're talkative or addressed directly
	{
		if (bestconf>0)
		{
			if (!know[bestnr].special)
			{
				do_sayx(cn, know[bestnr].answer, ch[co].name);
				chlog(cn, "answered \"%s\" with \"%s\".", text, know[bestnr].answer);
			}
			else
			{
				special_answer(cn, co, know[bestnr].special, text, bestnr);
				chlog(cn, "answered \"%s\" with special %d", text, know[bestnr].special);
			}
		}
		else
		{
			if (name)
			{
				do_sayx(cn, "I don't know about that.");
			}
			if (bestconf<=0)
			{
				chlog(cn, "Could not answer \"%s\".", text);
			}
		}
	}

//      do_sayx(cn,"talk=%d, bestconf=%d",talk,bestconf);
}

// **************************************************************** 
//
//  The Last Gate - SDL2 Client by Zachary "ZarroTsu" Baillie
//  Based on the Mercenaries of Astonia engine by Daniel Brockhaus
//  
//  gamescreen.h
//  Header information specific to gamescreen.c
//
// **************************************************************** 
#ifndef GAMESCREEN_H
#define GAMESCREEN_H

#define AT_BRV		0
#define AT_WIL		1
#define AT_INT		2
#define AT_AGL		3
#define AT_STR		4

#define RENDERDIST			54

#define SPR_EMPTY			999

#define XS	49
#define LL	22
#define XLL (22*10)
#define XMS	43
#define MLL 26

int logstart = 0;

static char logtext[XLL][60];
static char logfont[XLL];
static char motdtext[MLL][60];
static char motdfont[MLL];

struct cplayer pl;
struct cmap *map = NULL;
struct look look = {0,{0,0,0,0,0,0,0,0,0,0},0,0,"",0,0,0,0};

static char *at_name[5]={
	"Braveness",
	"Willpower",
	"Intuition",
	"Agility",
	"Strength"
};

static char *rank[25] = {
	"Private",
	"Private First Class",
	"Lance Corporal",
	"Corporal",
	"Sergeant",
	"Staff Sergeant",
	"Master Sergeant",
	"First Sergeant",
	"Sergeant Major",
	"Second Lieutenant",
	"First Lieutenant",
	"Captain",
	"Major",
	"Lieutenant Colonel",
	"Colonel",
	"Brigadier General",
	"Major General",
	"Lieutenant General",
	"General",
	"Field Marshal",
	"Knight",
	"Baron",
	"Earl",
	"Marquess",
	"Warlord"
};

static int rank_sprite[25] = {
	   10,    11,    12,    13,    14, 
	   15,    16,    17,    18,    19, 
	   20,    21,    22,    23,    24, 
	   25,    26,    27,    28,    29,
	   30,     6,     7,     8,     9
};

int stat_raised[108] = {0,0,0,0,0,0,0,0,0,0,0,0,0,};

int stat_points_used = 0;

struct skilltab *skilltab;
struct skilltab _skilltab[52] = {
//	{ //, '/', 	"////////////////",		"////////////////////////////////////////////////////////////////////////////////",
	{  0, 'C', 	"Hand to Hand", 		"Passive ability to hit and parry while unarmed.", 
				"", "", "", 
				{ AT_AGL, AT_AGL, AT_STR }},
				
	{  1, 'G', 	"Precision", 			"Passively improves your ability to inflict critical hits.", 
				"", "", "", 
				{ AT_BRV, AT_BRV, AT_WIL }},
				
	{  2, 'D', 	"Dagger", 				"Passive ability to hit and parry with a dagger in your main hand.", 
				"", "", "", 
				{ AT_WIL, AT_WIL, AT_AGL }},
				
	{  3, 'D', 	"Sword", 				"Passive ability to hit and parry with a one-handed sword in your main hand.", 
				"", "", "", 
				{ AT_BRV, AT_AGL, AT_STR }},
				
	{  4, 'D', 	"Axe", 					"Passive ability to hit and parry with an axe in your main hand.", 
				"", "", "", 
				{ AT_AGL, AT_STR, AT_STR }},
				
	{  5, 'D', 	"Staff", 				"Passive ability to hit and parry with a staff in your main hand.", 
				"", "", "", 
				{ AT_INT, AT_INT, AT_STR }},
				
	{  6, 'D', 	"Two-Handed", 			"Passive ability to hit and parry with a two-handed weapon in your main hand.", 
				"", "", "", 
				{ AT_AGL, AT_AGL, AT_STR }},
				
	{  7, 'F', 	"Zephyr", 				"Use (Spell): Applies a self-buff, granting additional melee hits after a brief delay. This buff also grants a small bonus to immunity.", 
				"Zephyr (Thorns)", 		"Use (Spell): Applies a self-buff, granting retaliation hits after parrying. This buff also grants a small bonus to resistance.", "", 
				{ AT_BRV, AT_AGL, AT_STR }},
				
	{  8, 'G', 	"Stealth", 				"Passive ability to stay hidden from others' sight. More effective while in SLOW mode.", 
				"", "", "", 
				{ AT_INT, AT_INT, AT_AGL }},
				
	{  9, 'G', 	"Perception", 			"Passive ability to see and hear your surroundings.", 
				"", "", "", 
				{ AT_INT, AT_INT, AT_AGL }},
//	{ //, '/', 	"////////////////",		"////////////////////////////////////////////////////////////////////////////////",
	{ 10, 'G', 	"Swimming", 			"Passive ability to prevent the loss of hitpoints while you are underwater.", 
				"", "", "", 
				{ AT_WIL, AT_AGL, AT_STR }},
				
	{ 11, 'F', 	"Magic Shield", 		"Use (Spell): Applies a buff to yourself, granting temporary armor.", 
				"Magic Shell", 			"Use (Spell): Applies a buff to yourself, granting temporary resistance and immunity.", "", 
				{ AT_BRV, AT_WIL, AT_WIL }},
				
	{ 12, 'G', 	"Bartering", 			"Passive ability to get better prices while buying or selling.", 
				"", "", "", 
				{ AT_BRV, AT_INT, AT_AGL }},
				
	{ 13, 'E', 	"Repair", 				"Use (Skill): You will try to repair the item under your cursor.", 
				"", "", "", 
				{ AT_INT, AT_STR, AT_STR }},
				
	{ 14, 'E', 	"Rage", 				"Use (Skill): Applies a buff to yourself, improving your Weapon Value at the cost of endurance over time.", 
				"Rage (Furious)", 		"Use (Skill): Applies a buff to yourself, improving your Weapon Value at the cost of life over time.", "", 
				{ AT_BRV, AT_BRV, AT_AGL }},
				
	{ 15, 'F', 	"Lethargy", 			"Use (Spell): Applies a buff to yourself, letting you pierce enemy Resistance and Immunity at the cost of mana over time.", 
				"", "", "", 
				{ AT_BRV, AT_WIL, AT_INT }},
				
	{ 16, 'D', 	"Shield", 				"Passive ability to parry while using a shield.", 
				"", "", "", 
				{ AT_BRV, AT_WIL, AT_STR }},
				
	{ 17, 'F', 	"Protect", 				"Use (Spell): Applies a buff to you or your target, raising their armor value.", 
				"", "", "", 
				{ AT_BRV, AT_WIL, AT_WIL }},
				
	{ 18, 'F', 	"Enhance", 				"Use (Spell): Applies a buff to you or your target, raising their weapon value.", 
				"", "", "", 
				{ AT_BRV, AT_WIL, AT_WIL }},
				
	{ 19, 'F', 	"Slow", 				"Use (Spell): Applies a decaying debuff to your target and surrounding enemies, greatly reducing their action speed.", 
				"Slow (Greater)", 		"Use (Spell): Applies a debuff to your target and surrounding enemies, reducing their action speed.", "", 
				{ AT_BRV, AT_INT, AT_INT }},
//	{ //, '/', 	"////////////////",		"////////////////////////////////////////////////////////////////////////////////",
	{ 20, 'F', 	"Curse", 				"Use (Spell): Applies a debuff to your target and surrounding enemies, reducing their attributes.", 
				"Curse (Greater)", 		"Use (Spell): Applies a decaying debuff to your target and surrounding enemies, greatly reducing their attributes.", "", 
				{ AT_BRV, AT_INT, AT_INT }},
				
	{ 21, 'F', 	"Bless", 				"Use (Spell): Applies a buff to you or your target, raising their attributes.", 
				"", "", "", 
				{ AT_BRV, AT_WIL, AT_WIL }},
				
	{ 22, 'F', 	"Identify", 			"Use (Spell): Identify the properties of a target or an item. Can be used on an already identified item to clear it.", 
				"", "", "", 
				{ AT_WIL, AT_WIL, AT_INT }},
				
	{ 23, 'G', 	"Resistance", 			"Passive ability to avoid enemy negative spells.", 
				"", "", "", 
				{ AT_BRV, AT_WIL, AT_STR }},
				
	{ 24, 'F', 	"Blast", 				"Use (Spell): Damages your target and surrounding enemies.", 
				"Blast (Scorch)", 		"Use (Spell): Damages your target and surrounding enemies. This also applies a debuff, increasing the damage dealt to the target.", "",  
				{ AT_BRV, AT_INT, AT_INT }},
				
	{ 25, 'F', 	"Dispel", 				"Use (Spell): Removes debuffs from your target.", 
				"Dispel (Enemy)", 		"Use (Spell): Removes buffs from your target.", "", 
				{ AT_BRV, AT_WIL, AT_INT }},
				
	{ 26, 'F', 	"Heal", 				"Use (Spell): Heals you or your target. This also applies Healing Sickness, reducing the power of consecutive heals.", 
				"Heal (Regen)", 		"Use (Spell): Applies a buff to you or your target, granting them health regeneration.", 
										"Use (Spell): Consumes a portion of your hitpoints to restore your target's mana.", 
				{ AT_BRV, AT_INT, AT_STR }},
				
	{ 27, 'F', 	"Ghost Companion", 		"Use (Spell): Summons a companion to follow you and your commands. Say COMMAND to it for a list of commands.", 
				"", "", "", 
				{ AT_BRV, AT_WIL, AT_INT }},
				
	{ 28, 'A', 	"Regenerate", 			"Passive ability to recover hitpoints over time.", 
				"", "", "", 
				{ AT_STR, AT_STR, AT_STR }},
				
	{ 29, 'A', 	"Rest", 				"Passive ability to recover endurance over time.", 
				"", "", "", 
				{ AT_AGL, AT_AGL, AT_AGL }},
//	{ //, '/', 	"////////////////",		"////////////////////////////////////////////////////////////////////////////////",
	{ 30, 'B', 	"Meditate", 			"Passive ability to recover mana over time.", 
				"", "", "", 
				{ AT_INT, AT_INT, AT_INT }},
				
	{ 31, 'G', 	"Aria", 				"Passively grants you and nearby allies a buff to cooldown rate, and debuffs nearby enemy cooldown rate. Has a base radius of 5 tiles.", 
				"", "", "", 
				{ AT_BRV, AT_AGL, AT_AGL }},
				
	{ 32, 'G', 	"Immunity", 			"Passive ability to reduce the strength of enemy negative spells.", 
				"", "", "", 
				{ AT_BRV, AT_AGL, AT_STR }},
				
	{ 33, 'G', 	"Surround Hit", 		"Passive ability to deal a portion of melee hit damage to all foes around you.", 
				"", "", "", 
				{ AT_BRV, AT_AGL, AT_STR }},
				
	{ 34, 'G', 	"Concentrate", 			"Passive ability to reduce the mana cost of spells and abilities.", 
				"", "", "", 
				{ AT_WIL, AT_WIL, AT_WIL }},
				
	{ 35, 'E', 	"Warcry", 				"Use (Skill): Shout to stun and strike fear into all nearby enemies. Has a base radius of 6 tiles.", 
				"Warcry (Rally)", 		"Use (Skill): Shout to rally your allies and improve hit and parry score. Has a base radius of 6 tiles.", "", 
				{ AT_BRV, AT_STR, AT_STR }},
				
	{ 36, 'D', 	"Dual Wield", 			"Passive ability to hit while using a dual-sword.", 
				"", "", "", 
				{ AT_BRV, AT_AGL, AT_STR }},
				
	{ 37, 'E', 	"Blind", 				"Use (Skill): Applies a debuff to nearby enemies, reducing their hit and parry rates. Has a base radius of 4 tiles.", 
				"Blind (Douse)", 		"Use (Skill): Applies a debuff to nearby enemies, reducing their stealth and spell modifier. Has a base radius of 4 tiles.", "", 
				{ AT_BRV, AT_INT, AT_AGL }},
				
	{ 38, 'G', 	"Gear Mastery", 		"Passive ability to improve weapon and armor values granted by your equipment.", 
				"", "", "", 
				{ AT_BRV, AT_AGL, AT_STR }},
				
	{ 39, 'G', 	"Safeguard", 			"Passive ability to reduce damage taken.", 
				"", "", "", 
				{ AT_BRV, AT_STR, AT_STR }},
//	{ //, '/', 	"////////////////",		"////////////////////////////////////////////////////////////////////////////////",
	{ 40, 'E', 	"Cleave", 				"Use (Skill): Strike your foe and deal damage to surrounding enemies. This also applies a debuff, causing them to take damage over time.", 
				"Cleave (Aggravate)",	"Use (Skill): Strike your foe and deal damage to surrounding enemies. This also applies a debuff, causing them to take additional damage.", "", 
				{ AT_BRV, AT_STR, AT_STR }},
				
	{ 41, 'E', 	"Weaken", 				"Use (Skill): Applies a debuff to your foe and surrounding enemies, reducing their weapon value.", 
				"Weaken (Crush)", 		"Use (Skill): Applies a debuff to your foe and surrounding enemies, reducing their armor value.", "", 
				{ AT_BRV, AT_AGL, AT_AGL }},
				
	{ 42, 'F', 	"Poison", 				"Use (Spell): Applies a stacking debuff to your target and surrounding enemies, causing them to take damage over time. Stacks up to 3 times.", 
				"Poison (Venom)", 		"Use (Spell): Applies a debuff to your target and surrounding enemies, reducing their immunity and causing them to take damage over time. Does not stack.", "", 
				{ AT_BRV, AT_INT, AT_INT }},
				
	{ 43, 'F', 	"Pulse", 				"Use (Spell): Applies a buff to yourself, causing a repeating burst of energy to damage nearby foes. Has a base radius of 3 tiles.", 
				"", "", "", 
				{ AT_BRV, AT_INT, AT_INT }},
				
	{ 44, 'G', 	"Proximity", 			"", // Arch-Templar
										"Passively improves the area-of-effect of your Aria and Weaken skills.", // Braver
										"Passively improves the area-of-effect of your Poison, Curse, and Slow spells.", // Sorcerer
										"Passively improves the area-of-effect of your Blast and Pulse spells.", // Arch-Harakim
				{ AT_BRV, AT_WIL, AT_INT }},
				
	{ 45, 'G', 	"Companion Mastery", 	"Passively increases the limit and number of abilities known by your ghost companion.", 
				"", "", "", 
				{ AT_BRV, AT_WIL, AT_WIL }},
				
	{ 46, 'F', 	"Shadow Copy", 			"Use (Spell): Summons a temporary doppelganger to attack your enemies.", 
				"", "", "", 
				{ AT_BRV, AT_WIL, AT_WIL }},
				
	{ 47, 'F', 	"Haste", 				"Use (Spell): Applies a buff to yourself, increasing your action speed.", 
				"", "", "", 
				{ AT_BRV, AT_WIL, AT_AGL }},
				
	{ 48, 'E', 	"Taunt",				"Use (Skill): Applies a debuff to your target and surrounding enemies, forcing them to attack you. This also applies a buff to yourself, granting damage resistance.", 
				"", "", "", 
				{ AT_BRV, AT_STR, AT_STR }},
				
	{ 49, 'E', 	"Leap", 				"Use (Skill): Strike your foe and leap to your target, dealing critical damage if they are the same enemy. Cooldown can be bypassed by sacrificing life.", 
				"Leap (Random)", 		"Use (Skill): Strike your foe and leap to a random target, dealing critical damage if they are the same enemy. Cooldown can be bypassed by sacrificing life.", "", 
				{ AT_BRV, AT_AGL, AT_AGL }},
//	{ //, '/', 	"////////////////",		"////////////////////////////////////////////////////////////////////////////////",
	{ 50, 'H', 	"Light", 				"Use (Spell): Applies a buff to you or your target, making them glow in the dark.", 
				"", "", "", 
				{ 0, 0, 0 }},
	{ 51, 'H', 	"Recall", 				"Use (Spell): Teleport yourself to a safe location after a brief delay.", 
				"", "", "", 
				{ 0, 0, 0 }}
};

struct wpslist wpslist[MAXWPS]={
//    nr, "123456789012345678901234567890",	"\"2345678901234567890123456789\""
	{  0, "Lynbore, Tavern", 				"\"Humble beginnings.\"" },
	{  1, "Lynbore, East", 					"\"Ghosts and spiders duel.\"" },
	{  2, "Aston, South", 					"\"The Weeping Woods.\"" },
	{  3, "Aston, Crossroads", 				"\"Between ivory and violet.\"" },
	{  4, "Aston, West", 					"\"The Strange Forest.\"" },
	{  5, "Aston, East", 					"\"The Autumn Meadow.\"" },
	{  6, "Forgotten Canyon", 				"\"Old secrets echo.\"" },
	{ 20, "Lizard Temple, South", 			"\"Where reptiles implore.\"" },
	{  7, "Lizard Temple, North", 			"\"The Beryl Jungle.\"" },
	{ 21, "Lizard Settlement, East", 		"\"Betwixt emerald and magma.\"" },
	{ 22, "The Emerald Cavern, East", 		"\"A feudal audience.\"" },
	{  8, "Temple of Osiris", 				"\"Gods toy with greed.\"" },
	{ 23, "The Tower", 						"\"Sky-piercing obelisk.\"" },
	{  9, "Neiseer, West", 					"\"The Basalt Desert.\"" },
	{ 10, "Neiseer", 						"\"Twilit stars sing sweetly.\"" },
	{ 11, "Neiseer, North", 				"\"The Burning Plains.\"" },
	{ 12, "Mausoleum, Basement", 			"\"An accursed tomb.\"" },
	{ 13, "Pentagram Quest, Novice", 		"\"Endless adventure begins.\"" },
	{ 14, "Pentagram Quest, Earth", 		"\"Smells of dirt and soot.\"" },
	{ 15, "Pentagram Quest, Fire", 			"\"Boiling and burning.\"" },
	{ 16, "Pentagram Quest, Jungle", 		"\"A verdant expanse.\"" },
	{ 17, "Pentagram Quest, Ice", 			"\"Frozen still for eternity.\"" },
	{ 18, "Pentagram Quest, Underwater",	"\"Embrace foul waters.\"" },
	{ 19, "Pentagram Quest, Onyx", 			"\"Depths known only by fear.\"" }
//    nr, "123456789012345678901234567890",	"123456789012345678901234567890"
};

char tutorial_text[6][3][12][45] = { // overall
	{ // tutorial 1
		{ // page 1
			{ "The diamond-shaped space in the middle of" },
			{ "your screen is the game world." },
			{ " " },
			{ "As you move your mouse in this space, it" },
			{ "will highlight tiles in the world." },
			{ " " },
			{ "Right-clicking a tile will rotate your" },
			{ "character to face that direction." },
			{ " " },
			{ "Left-clicking a tile will tell your" },
			{ "character to travel to that tile." },
			{ " " }
		},
		{ // page 2
			{ "In the upper right corner of the screen is" },
			{ "the dialog window." },
			{ " " },
			{ "This displays chat messages, system" },
			{ "messages, and game world events as text." },
			{ " " },
			{ "You can type at any time to make text, and" },
			{ "can press the Enter/Return key to send it." },
			{ " " },
			{ "There are also various chat commands that" },
			{ "can be sent starting with the # or / key," },
			{ "such as /help or /shout." }
		},
		{ // page 3
			{ "To get started, try approaching Jamil. He" },
			{ "can be found in the room NORTH of here (to" },
			{ "the upper left of your screen)." },
			{ " " },
			{ "You can click the X in the upper right of" },
			{ "this dialog window or press the ESCAPE key" },
			{ "to continue." },
			{ " " },
			{ " " },
			{ " " },
			{ " " },
			{ " " }
		}
	},
	{ // tutorial 2
		{ // page 1
			{ "Jamil has given you your first quest!" },
			{ "The Thieves House is located farther to" },
			{ "the NORTH, outside of this tavern." },
			{ " " },
			{ "To leave this tavern, you can click on the" },
			{ "same tile as the door to open it and stand" },
			{ "in its frame, revealing tiles beyond it." },
			{ " " },
			{ "You can also click inside the black void" },
			{ "beyond the door, and your character will" },
			{ "attempt to navigate to that tile if they" },
			{ "are able to." }
		},
		{ // page 2
			{ "While out in the game world, the CTRL, ALT," },
			{ "and SHIFT keys change the way clicking" },
			{ "interacts with the world." },
			{ " " },
			{ "The SHIFT key allows interactions with" },
			{ "objects and items." },
			{ " " },
			{ "The CTRL key allows interactions with NPCS" },
			{ "and monsters." },
			{ " " },
			{ "If you are a spellcaster, the ALT key lets" },
			{ "you highlight a target for your spells." }
		},
		{ // page 3
			{ "For now, feel free to explore, try out the" },
			{ "controls, maybe test the /help command?" },
			{ " " },
			{ "To assist your character, try casting the" },
			{ "LIGHT spell on the hotkey list in the lower" },
			{ "right. These spells can also be used by" },
			{ "holding the CTRL or ALT key, and pressing" },
			{ "the number or letter shown next to them." },
			{ " " },
			{ "If you happen to stumble into combat, your" },
			{ "character will fight back automatically." },
			{ " " }
		}
	},
	{ // tutorial 3
		{ // page 1
			{ "You got your first kill! You have been" },
			{ "awarded a bunch of experience points." },
			{ " " },
			{ "In The Last Gate, experience points are" },
			{ "put towards an assortment of skills and" },
			{ "passives which effect your character." },
			{ " " },
			{ "The skill list in the upper left should now" },
			{ "be filled with +'s. Clicking on these will" },
			{ "allocate some experience toward that skill." },
			{ " " },
			{ " " }
		},
		{ // page 2
			{ "You can RIGHT-CLICK on skills on this list" },
			{ "to see what they do. This will be displayed" },
			{ "in the dialog window on the right." },
			{ " " },
			{ "With your experience allocated, it will NOT" },
			{ "affect your character until it is applied." },
			{ " " },
			{ "To apply this experience, always remember" },
			{ "to LEFT-CLICK on the 'Update' button at the" },
			{ "bottom of the list." },
			{ " " },
			{ " " }
		},
		{ // page 3
			{ "Here are some helpful tips to raising your" },
			{ "first character:" },
			{ " " },
			{ "Raising the weapon skill that matches your" },
			{ "weapon, such as AXE, SWORD or DAGGER, can" },
			{ "improve survivability by making enemies" },
			{ "miss more often when they strike you." },
			{ " " },
			{ "Harakim may also benefit from raising" },
			{ "spells such as BLAST or GHOST COMPANION," },
			{ "which get stronger as they increase." },
			{ " " }
		}
	},
	{ // tutorial 4
		{ // page 1
			{ "This particular enemy has dropped a key," },
			{ "which you'll need to get further inside." },
			{ " " },
			{ "To loot their grave and grab the key, you" },
			{ "can hold down your SHIFT key and LEFT-" },
			{ "CLICK while highlighting their gravestone." },
			{ " " },
			{ "This will open a new window, displaying the" },
			{ "content of their grave such as what they" },
			{ "were carrying, their gear, and any money" },
			{ "they may have had." },
			{ " " }
		},
		{ // page 2
			{ "If you RIGHT-CLICK on any item in this" },
			{ "grave window, it will display the item's" },
			{ "name in the dialog window to the right." },
			{ " " },
			{ "You can LEFT-CLICK on any item in this" },
			{ "grave window to take it, adding it to your" },
			{ "inventory." },
			{ " " },
			{ "Money will be taken automatically and" },
			{ "added to your carried money." },
			{ " " },
			{ " " }
		},
		{ // page 3
			{ "Similarly, RIGHT-CLICKING items in your" },
			{ "inventory will let you look at them, and" },
			{ "LEFT-CLICKING items in your inventory will" },
			{ "use them." },
			{ " " },
			{ "Some items, such as keys, will be used" },
			{ "automatically while they are in your" },
			{ "inventory. Keys let you walk through locked" },
			{ "doors as if they were open." },
			{ " " },
			{ "Consumable items such as potions will be" },
			{ "reduced or removed when you use them." }
		}
	},
	{ // tutorial 5
		{ // page 1
			{ "You killed the boss of the Thieves Guild!" },
			{ " " },
			{ "Jamil's amulet aside, he is also carrying" },
			{ "additional pieces of armor you can wear." },
			{ " " },
			{ "If you hold down the SHIFT key and LEFT-" },
			{ "CLICK on items in your inventory, you will" },
			{ "pick that item up and hold it under your" },
			{ "cursor." },
			{ " " },
			{ "You can drop the item on your cursor by" },
			{ "holding SHIFT and LEFT-CLICKING elsewhere." }
		},
		{ // page 2
			{ "Equipping the armor the thief was carrying" },
			{ "will grant you additional Armor Value," },
			{ "which reduces damage taken when being hit." },
			{ " " },
			{ "You can also try to equip Jamil's amulet if" },
			{ "you like, but you might not be able to..." },
			{ " " },
			{ "As you explore and your character improves," },
			{ "you will find new weapons and armor that" },
			{ "can be equipped. Some can give bonuses to" },
			{ "your skills and attributes as well." },
			{ " " }
		},
		{ // page 3
			{ "Your final task is to return to Jamil and" },
			{ "give back his amulet." },
			{ " " },
			{ "While holding an item under your cursor," },
			{ "holding down the CTRL key and LEFT-CLICKING" },
			{ "on a character will attempt to give the" },
			{ "held item to that character." },
			{ " " },
			{ "Holding CTRL and LEFT-CLICKING without an" },
			{ "item on your cursor will instead tell your" },
			{ "character to begin combat with that target." },
			{ "Be careful with your clicks!" }
		}
	},
	{ // tutorial 6
		{ // page 1
			{ "You have completed your first quest!" },
			{ " " },
			{ "Jamil has given you a new skill, as well as" },
			{ "some more experience points to play around" },
			{ "with. Spend this however you like!" },
			{ " " },
			{ "Of course, Jamil is only the first quest." },
			{ "There are many NPCs dotted around town who" },
			{ "may ask for your help when you meet them." },
			{ " " },
			{ "The Innkeeper may help you find additional" },
			{ "quests to try out if you approach him." }
		},
		{ // page 2
			{ "If, on the other hand, you feel like this" },
			{ "is a good stopping point to take a break" },
			{ "from the game, you can do so by entering" },
			{ "the vacant room in the lower right." },
			{ " " },
			{ "Although it looks dark and uninviting, this" },
			{ "will allow your character to safely exit" },
			{ "the game." },
			{ " " },
			{ "Clicking on the 'EXIT' button in the lower" },
			{ "right will quit the game faster, but this" },
			{ "may punish you if you try to cheat combat." }
		},
		{ // page 3
			{ "The other buttons in the lower right can" },
			{ "adjust how the game world is displayed," },
			{ "each behaving as a toggle for the named" },
			{ "element in the game world." },
			{ " " },
			{ "The 'FAST', 'NORMAL', and 'SLOW' buttons" },
			{ "adjust movement speed, each carrying their" },
			{ "own strengths and weaknesses." },
			{ " " },
			{ "So we reach the end of this tutorial. We" },
			{ "hope you have fun in this little world of" },
			{ "ours, and mayest thou pass The Last Gate!" }
		}
	},
};

int tutorial_image[6][3] = { 
	{ 18010, 18011, 18012 },
	{ 18013, 18014, 18015 },
	{ 18016, 18017, 18018 },
	{ 18019, 18020, 18021 },
	{ 18022, 18023, 18024 },
	{ 18025, 18026, 18027 }
};

enum {
	GAM_GUI_ATT_NAMES,
	GAM_GUI_ATT_NUMS,
	GAM_GUI_ATT_PM,
	GAM_GUI_SKL_NAMES,
	GAM_GUI_SKL_NUMS,
	GAM_GUI_SKL_PM,
	GAM_GUI_SKL_UP,
	GAM_GUI_SKL_DOWN,
	GAM_GUI_UPDATE,
	GAM_GUI_INV,
	GAM_GUI_INV_UP,
	GAM_GUI_INV_DOWN,
	GAM_GUI_EQ_SWITCH,
	GAM_GUI_EQ_HEAD,
	GAM_GUI_EQ_NECK,
	GAM_GUI_EQ_BODY,
	GAM_GUI_EQ_ARMS,
	GAM_GUI_EQ_BELT,
	GAM_GUI_EQ_CHRM,
	GAM_GUI_EQ_FEET,
	GAM_GUI_EQ_LHND,
	GAM_GUI_EQ_RHND,
	GAM_GUI_EQ_CLOK,
	GAM_GUI_EQ_LRNG,
	GAM_GUI_EQ_RRNG,
	GAM_GUI_EQ_CHR2,
	GAM_GUI_COIN,
	GAM_GUI_TRASH,
	GAM_GUI_LOG,
	GAM_GUI_S1,
	GAM_GUI_F1,
	GAM_GUI_ENUM_SIZE
} GAM_GUI_ENUM;

// Zarro 2020 - Define gui rectangles as arrays - easier to find and change them here (sort of)
int gui_inv_up[] 	= { 600,   5, 612,  35 };
int gui_inv_down[]	= { 600,  76, 612, 106 };

int gui_skl_up[]	= { 233, 117, 245, 152 };
int gui_skl_down[]	= { 233, 222, 245, 256 };
int gui_update[]	= { 136, 257, 185, 270 };
int gui_skl_pm[]	= { 160,   6, 184, 255 };
int gui_skl_names[]	= {   6, 118, 135, 255 };

int gui_f_col[]		= {1080 };
int gui_f_row[]		= { 662, 678, 694, 587, 602, 617, 632 };

int gui_trash[]		= { 535, 111 };
int gui_coin[]		= { 437, 143 };

//int gui_coin_x[]	= { 393, 425, 457, 489, 521, 553, 585 };
//int gui_coin_y[]	= {  77 };

int gui_inv_x[]		= { 260, 599 };
int gui_inv_y[]		= {   5, 106 };

//					   HEAD,NECK,BODY,ARMS,BELT,CHRM,FEET,LHND,RHND,CLOK,LRNG,RRNG,CHRM2
int gui_equ_x[]		= { 738, 700, 738, 704, 738, 777, 738, 806, 670, 772, 776, 700, 801 };
int gui_equ_y[]		= {   5,  18,  39,  56,  73,  17, 107,  56,  56,  56,  94,  94,  17 };

int gui_equ_s[]		= { 670,   5 };

static SDL_Rect gam_gui[GAM_GUI_ENUM_SIZE] = {
	{ 0, 0, 0, 0 }, // GAM_GUI_ATT_NAMES
	{ 0, 0, 0, 0 }, // GAM_GUI_ATT_NUMS
	{ 0, 0, 0, 0 }, // GAM_GUI_ATT_PM
	{ 0, 0, 0, 0 }, // GAM_GUI_SKL_NAMES
	{ 0, 0, 0, 0 }, // GAM_GUI_SKL_NUMS
	{ 0, 0, 0, 0 }, // GAM_GUI_SKL_PM
	{ 0, 0, 0, 0 }, // GAM_GUI_SKL_UP
	{ 0, 0, 0, 0 }, // GAM_GUI_SKL_DOWN
	{ 0, 0, 0, 0 }, // GAM_GUI_UPDATE
	{ 0, 0, 0, 0 }, // GAM_GUI_INV
	{ 0, 0, 0, 0 }, // GAM_GUI_INV_UP
	{ 0, 0, 0, 0 }, // GAM_GUI_INV_DOWN
	{ 0, 0, 0, 0 }, // GAM_GUI_EQ_SWITCH
	{ 0, 0, 0, 0 }, // GAM_GUI_EQ_HEAD
	{ 0, 0, 0, 0 }, // GAM_GUI_EQ_NECK
	{ 0, 0, 0, 0 }, // GAM_GUI_EQ_BODY
	{ 0, 0, 0, 0 }, // GAM_GUI_EQ_ARMS
	{ 0, 0, 0, 0 }, // GAM_GUI_EQ_BELT
	{ 0, 0, 0, 0 }, // GAM_GUI_EQ_CHRM
	{ 0, 0, 0, 0 }, // GAM_GUI_EQ_FEET
	{ 0, 0, 0, 0 }, // GAM_GUI_EQ_LHND
	{ 0, 0, 0, 0 }, // GAM_GUI_EQ_RHND
	{ 0, 0, 0, 0 }, // GAM_GUI_EQ_CLOK
	{ 0, 0, 0, 0 }, // GAM_GUI_EQ_LRNG
	{ 0, 0, 0, 0 }, // GAM_GUI_EQ_RRNG
	{ 0, 0, 0, 0 }, // GAM_GUI_EQ_CHR2
	{ 0, 0, 0, 0 }, // GAM_GUI_COIN
	{ 0, 0, 0, 0 }, // GAM_GUI_TRASH
	{ 0, 0, 0, 0 }, // GAM_GUI_LOG
	{ 0, 0, 0, 0 }, // GAM_GUI_S1
	{ 0, 0, 0, 0 }, // GAM_GUI_F1
};

#endif // GAMESCREEN_H
/* END OF FILE */
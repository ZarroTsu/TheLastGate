#include <alloc.h>
#include <windows.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#pragma hdrstop
#include "dd.h"
#include "common.h"
#include "inter.h"

int init_done=0;
int frame=0;
extern int mx,my;

extern int ticker;

int pskip=0,pidle=0;
extern int t_size;

extern int cursor_type;
extern HCURSOR cursor[10];

extern int screen_width, screen_height, screen_tilexoff, screen_tileyoff;
//extern int screen_overlay_sprite;
extern short screen_windowed;
extern short screen_renderdist;

// from dd.c
int copysprite(int nr,int effect,int x,int y,int xoff,int yoff);
void dd_flip(void);
void dd_flip_windowed(void);
void dd_showbar(int xf,int yf,int xs,int ys,unsigned short col);
void copyspritex(int nr,int xpos,int ypos,int effect);
void dd_showbox(int xf,int yf,int xs,int ys,unsigned short col);
void dd_alphaeffect_magic(int nr,int str,int xpos,int ypos,int xoff,int yoff);
int get_avgcol(int nr);

char *lookup(int nr,unsigned short id);

extern char input[];
extern int in_len;
extern int cur_pos;
extern int view_pos;
extern int logstart;
extern int logtimer;
extern int RED,GREEN,BLUE;
int tput=0;

extern int do_shadow;
extern int do_darkmode;

void do_msg(void);

// Going to write up the tutorial right here. Unfortunately it takes up a lot of space.

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
//

char *at_name[5]={
	"Braveness",
	"Willpower",
	"Intuition",
	"Agility",
	"Strength"
};

#define AT_BRV		0
#define AT_WIL		1
#define AT_INT		2
#define AT_AGL		3
#define AT_STR		4

struct skilltab *skilltab;
struct skilltab _skilltab[55] = {
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
				
	{  7, 'G', 	"Zephyr", 				"Passive ability granting an additional hit after a brief delay.", 
				"", "",
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
				
	{ 43, 'F', 	"Pulse", 				"Use (Spell): Applies a buff to yourself, causing a repeating burst of energy to damage nearby foes and inflict shock. Has a base radius of 3 tiles.", 
				"Pulse (Charge)", 		"Use (Spell): Applies a buff to yourself, causing a repeating burst of energy to heal nearby allies and inflict charge. Has a base radius of 3 tiles.",
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
	{ 53, 'I', 	"Ferocity", 			"Passively grants a bonus to WV and AV. The bonus increases for each empty gear slot.", 
				"", "",
				{ 0, 0, 0 }},
	{ 54, 'I', 	"Shift", 				"Use (Skill): Change form from that of a Ratling to that of a Greenling, and vice versa. Has its own unique cooldown timer.", 
				"", "",
				{ 0, 0, 0 }}
};

struct sk_tree sk_tree[10][12]={
	{	// Seyan'du
		{ "Sharpness", 				"+2 to Weapon Value.", 	// W
		  6601,						"" },
		{ "Expertise", 				"+2 to All Attributes.", 	// N
		  6602,						"" },
		{ "Toughness", 				"+2 to Armor Value.", 	// E
		  6603,						"" },
		{ "Absolution", 			"0.5%% more damage dealt for each buff", 	// WS
		  6604,						"or debuff on you." },
		{ "Vanquisher", 			"6%% more total Weapon Value.", 	// WW
		  6605,						"" },
		{ "Scorn", 					"Your debuffs ignore 20%% of enemy", 	// WN
		  6606,						"Resistance." },
		{ "Determination", 			"Gain 1 additional Hit and Parry for", 	// NW
		  6607,						"every 100 total Attributes." },
		{ "Jack of All Trades", 	"4%% increased total Attributes.", 	// NN
		  6608,						"" },
		{ "Redemption", 			"Companions have Hit and Parry scores", 	// NE
		  6609,						"equal to yours, and learn Regen." },
		{ "Enigmatic", 				"20%% reduced effect of debuffs on you.", 	// EN
		  6610,						"" },
		{ "Steelskin", 				"6%% more total Armor Value.", 	// EE
		  6611,						"" },
		{ "Penance", 				"0.5%% less damage taken for each buff", 	// ES
		  6612,						"or debuff on you." }
	}, // "         '         '  ",	"         '         '         '         "
	{	// Arch Templar
		{ "Spiked", 				"+5 to Thorns.", 	// W
		  6613,						"" },
		{ "Might", 					"+4 to Strength.", 	// N
		  6614,						"" },
		{ "Bulwark", 				"+3 to Armor Value.", 	// E
		  6615,						"" },
		{ "Serrated Blades", 		"Cleave deals additional damage based on", 	// WS
		  6616,						"your total Thorns." },
		{ "Sharkskin", 				"20%% more total Thorns.", 	// WW
		  6617,						"" },
		{ "Retaliation", 			"Your Thorns can now trigger on a", 	// WN
		  6618,						"parried hit with 10%% power." },
		{ "Overlord", 				"0.5%% more effect of Warcry and Rally", 	// NW
		  6619,						"for every 10 total Strength." },
		{ "Overwhelming Strength", 	"3%% increased total Strength.", 	// NN
		  6620,						"+10 to Strength Limit." },
		{ "Censure", 				"Taunt reduces enemy Hit score by 5%%", 	// NE
		  6621,						"for its duration." },
		{ "Bastion", 				"20%% of total Resistance is granted as", 	// EN
		  6622,						"extra Immunity." },
		{ "Unbreakable", 			"9%% more total Armor Value.", 	// EE
		  6623,						"" },
		{ "Rampart", 				"50%% more Parry granted by your Shield", 	// ES
		  6624,						"skill." }
	}, // "         '         '  ", "         '         '         '         "
	{	// Skald
		{ "Muscle", 				"+3 to Weapon Value.", 	// W
		  6625,						"" },
		{ "Dexterity", 				"+4 to Agility.", 	// N
		  6626,						"" },
		{ "Persistance", 			"+20 Endurance.", 	// E
		  6627,						"" },
		{ "Nocturne", 				"20%% increased effect of Aria.", 	// WS
		  6628,						"" },
		{ "Valor", 					"9%% more total Weapon Value.", 	// WW
		  6629,						"" },
		{ "Enthusiasm", 			"Your Aria additionally grants nearby", 	// WN
		  6630,						"allies 10%% of your Weapon Value." },
		{ "Slaying", 				"+3%% Critical Multiplier for every 10", 	// NW
		  6631,						"total Agility." },
		{ "Overwhelming Agility", 	"3%% increased total Agility.", 	// NN
		  6632,						"+10 to Agility Limit." },
		{ "Acumen", 				"All melee skills use the attributes", 	// NE
		  6633,						"(STR+BRV/2) + Agility + Agility." },
		{ "Impact", 				"Weaken and Crush also reduce enemy", 	// EN
		  6634,						"critical hit chance." },
		{ "Perseverance", 			"20%% more total Endurance.", 	// EE
		  6635,						"" },
		{ "Tenacity", 				"20%% of damage taken is dealt to your", 	// ES
		  6636,						"Endurance instead." }
	}, // "         '         '  ", "         '         '         '         "
	{	// Warrior
		{ "Rapidity", 				"+5 to Attack Speed.", 	// W
		  6637,						"" },
		{ "Ruffian", 				"+3 to Strength & +3 to Agility.", 	// N
		  6638,						"" },
		{ "Passion", 				"+5 to Spell Aptitude.", 	// E
		  6639,						"" },
		{ "Alacrity", 				"Zephyr deals 20%% more damage.", 	// WS
		  6640,						"" },
		{ "Swiftness", 				"10%% more total Attack Speed.", 	// WW
		  6641,						"" },
		{ "Intensity", 				"+2 to Spell Modifier.", 	// WN
		  6642,						"" },
		{ "Antagonizer", 			"0.5%% more effect of Blind and Douse", 	// NW
		  6643,						"for every 10 total Agility." },
		{ "Harrier", 				"3%% increased total Agility and", 	// NN
		  6644,						"Strength." },
		{ "Butchery", 				"0.5%% more effect of Cleave for every", 	// NE
		  6645,						"10 total Strength." },
		{ "Champion", 				"Enemies beside and behind you no longer", 	// EN
		  6646,						"gain a bonus to hitting you." },
		{ "Zealotry", 				"20%% more total Spell Aptitude.", 	// EE
		  6647,						"" },
		{ "Fervor", 				"2%% less damage taken per 10 Spell", 	// ES
		  6648,						"Aptitude." }
	}, // "         '         '  ", "         '         '         '         "
	{	// Sorcerer
		{ "Expansiveness", 			"+1 to Area of Effect.", 	// W
		  6649,						"" },
		{ "Potency", 				"+3 to Willpower & +3 to Intuition.", 	// N
		  6650,						"" },
		{ "Quickstep", 				"+5 to Movement Speed.", 	// E
		  6651,						"" },
		{ "Tormenter", 				"Poison deals damage 20%% faster.", 	// WS
		  6652,						"" },
		{ "Grandiosity", 			"20%% more total Area of Effect.", 	// WW
		  6653,						"" },
		{ "Brilliance", 			"+1 to Spell Modifier.", 	// WN
		  6654,						"" },
		{ "Coordination", 			"0.5%% more effect of Lethargy for every", 	// NW
		  6655,						"10 total Willpower." },
		{ "Pragmatic", 				"3%% increased total Willpower and", 	// NN
		  6656,						"Intuition." },
		{ "Hex Master", 			"0.5%% more effect of Curse and Slow", 	// NE
		  6657,						"for every 10 total Intuition." },
		{ "Nimble", 				"You no longer have a parry penalty if", 	// EN
		  6658,						"hit while not fighting." },
		{ "Fleet-footed", 			"20%% more total Movement Speed.", 	// EE
		  6659,						"" },
		{ "Acceleration", 			"Haste grants 20%% more move speed to", 	// ES
		  6660,						"you, and it can now be cast on allies." }
	}, // "         '         '  ", "         '         '         '         "
	{	// Summoner
		{ "Nimbleness", 			"+5 to Cast Speed.", 	// W
		  6661,						"" },
		{ "Wisdom", 				"+4 to Willpower.", 	// N
		  6662,						"" },
		{ "Vitality", 				"+20 Hitpoints.", 	// E
		  6663,						"" },
		{ "Tactician", 				"Increases and multipliers to Cast Speed", 	// WS
		  6664,						"also affect Attack Speed." },
		{ "Spellslinger", 			"10%% more total Cast Speed.", 	// WW
		  6665,						"" },
		{ "Harpooner", 				"20%% more Hit and Parry score while", 	// WN
		  6666,						"using a Spear." },
		{ "Mysticism", 				"All spell skills use the attributes", 	// NW
		  6667,						"(BRV+INT)/2 + Willpower + Willpower." },
		{ "Overwhelming Willpower", "3%% increased total Willpower.", 	// NN
		  6668,						"+10 to Willpower Limit." },
		{ "Shaper", 				"0.5%% more effect of Shadow Copy for", 	// NE
		  6669,						"every 10 total Willpower." },
		{ "Diviner", 				"Ghost Companions inherit your Dispel", 	// EN
		  6670,						"and the effects of your tarot cards." },
		{ "Constitution", 			"20%% more total Hitpoints.", 	// EE
		  6671,						"" },
		{ "Protector", 				"Magic Shield and Magic Shell are also", 	// ES
		  6672,						"cast on your active companions." }
	}, // "         '         '  ", "         '         '         '         "
	{	// Arch Harakim
		{ "Composure", 				"+5 to Cooldown Rate.", 	// W
		  6673,						"" },
		{ "Intellect", 				"+4 to Intuition.", 	// N
		  6674,						"" },
		{ "Wellspring", 			"+20 Mana.", 	// E
		  6675,						"" },
		{ "Destroyer", 				"Blast has its base cooldown reduced", 	// WS
		  6676,						"for each enemy hit by it." },
		{ "Serenity", 				"5%% more total Cooldown Rate.", 	// WW
		  6677,						"" },
		{ "Strategist", 			"You suffer no cooldown if a spell is", 	// WN
		  6678,						"suppressed." },
		{ "Psychosis", 				"0.5%% more effect of Pulse for every 10", 	// NW
		  6679,						"total Intuition." },
		{ "Overwhelming Intuition", "3%% increased total Intuition.", 	// NN
		  6680,						"+10 to Intuition Limit." },
		{ "Wizardry", 				"All spell skills use the attributes", 	// NE
		  6681,						"(BRV+WIL)/2 + Intuition + Intuition." },
		{ "Flow", 					"25%% of overcapped Mana is granted as", 	// EN
		  6682,						"additional Hitpoints." },
		{ "Perpetuity", 			"20%% more total Mana.", 	// EE
		  6683,						"" },
		{ "Resourcefulness", 		"20%% of damage taken is dealt to your", 	// ES
		  6684,						"Mana instead." }
	}, // "         '         '  ", "         '         '         '         "
	{	// Braver
		{ "Accuracy", 				"+3 to Hit Score.", 	// W
		  6685,						"" },
		{ "Boldness", 				"+4 to Braveness.", 	// N
		  6686,						"" },
		{ "Avoidance", 				"+3 to Parry Score.", 	// E
		  6687,						"" },
		{ "Assassination", 			"20%% increased effect of Precision.", 	// WS
		  6688,						"" },
		{ "Rigor", 					"4%% more total Hit Score.", 	// WW
		  6689,						"" },
		{ "Deftness", 				"50%% reduced damage taken from", 	// WN
		  6690,						"triggering enemy Thorns." },
		{ "Perfectionism", 			"0.5%% more effect of Finesse for every 10", 	// NW
		  6691,						"total Braveness." },
		{ "Overwhelming Braveness", "3%% increased total Braveness.", 	// NN
		  6692,						"+10 to Braveness Limit." },
		{ "Virtuosity", 			"All weapon skills use the attributes", 	// NE
		  6693,						"(AGL+STR)/2 + Braveness + Braveness." },
		{ "Resilience", 			"40%% less effect of Healing Sickness", 	// EN
		  6694,						"on you." },
		{ "Flexibility", 			"4%% more total Parry Score.", 	// EE
		  6695,						"" },
		{ "Litheness", 				"50%% reduced extra damage taken from", 	// ES
		  6696,						"enemy Critical Hits." }
	}, // "         '         '  ", "         '         '         '         "
	{	// Lycanthrope
		{ "Maiming", 				"+5 to Top Damage.", 	// W
		  6697,						"" },
		{ "Feast", 					"+10 Hitpoints, Endurance, and Mana.", 	// N
		  6698,						"" },
		{ "Insight", 				"+2 to Spell Modifier.", 	// E
		  6699,						"" },
		{ "Lust", 					"Ferocity grants +1%% base crit chance", 	// WS
		  6700,						"per empty gear slot." },
		{ "Ravager", 				"20%% more total Top Damage.", 	// WW
		  6701,						"" },
		{ "Greed", 					"Your Top Damage is rolled an additional", 	// WN
		  6702,						"time, using the higher result." },
		{ "Wrath", 					"0.5%% more effect of Rage & Calm per 50", 	// NW
		  6703,						"missing Hitpoints, Endurance, and Mana." },
		{ "Gluttony", 				"10%% more Hitpoints, Endurance, and Mana.", 	// NN
		  6704,						"" },
		{ "Sloth", 					"10%% of damage dealt is restored as", 	// NE
		  6705,						"Hitpoints, Endurance, and Mana." },
		{ "Pride", 					"Your debuffs ignore 20%% of enemy", 	// EN
		  6706,						"Immunity." },
		{ "Madness", 				"+3 to Spell Modifier.", 	// EE
		  6707,						"" },
		{ "Envy", 					"Ferocity grants +1 Spell Modifier per", 	// ES
		  6708,						"empty gear slot." }
	}, // "         '         '  ", "         '         '         '         ' "
	{	// Contract
		{ "Reward", 				"200%% increased chance of finding Rainbow", 	// W
		  6709,						"Belts in contracts signed by you." },
		{ "Challenge", 				"+1 to rank of contracts signed by you.", 	// N
		  6710,						"" },
		{ "Army", 					"+1 enemy per spawn in contracts signed by", 	// E
		  6711,						"you." },
		{ "Hope", 					"20%% increased effect of rewards from", 	// WS
		  6712,						"green shrines in contracts signed by you." },
		{ "Opalescence", 			"200%% increased chance of finding Rainbow", 	// WW
		  6713,						"Belts in contracts signed by you." },
		{ "Scholar", 				"20%% more clear experience from", 	// WN
		  6714,						"contracts signed by you." },
		{ "Fate", 					"20%% more effect of blue shrines in", 	// NW
		  6715,						"contracts signed by you." },
		{ "Hubris", 				"+1 to rank of contracts signed by you.", 	// NN
		  6716,						"" },
		{ "Binding", 				"Contracts signed by you always grant", 	// NE
		  6717,						"tier 3 effects from quills." },
		{ "Destiny", 				"Red shrines produce harder enemies with", 	// EN
		  6718,						"more rewards in contracts signed by you." },
		{ "Swarm", 					"+1 enemy per spawn in contracts signed by", 	// EE
		  6719,						"you." },
		{ "Incentive", 				"Enemies grant an additional 5%% of exp as", 	// ES
		  6720,						"Contract Pts in contracts signed by you." }
	}  // "         '         '  ", "         '         '         '         ' "
};

struct sk_tree sk_corrupt[108]={
//    "         '         '      ", "         '         '         '         ' "
	{ "* Sharpness *", 				"+1 to Weapon Value.",
	  6601,							"" },
	{ "* Expertise *", 				"+1 to All Attributes.",
	  6602,							"" },
	{ "* Toughness *", 				"+1 to Armor Value.",
	  6603,							"" },
	{ "* Absolution *", 			"0.2%% more damage dealt for each buff",
	  6604,							"or debuff on you." },
	{ "* Vindication *", 			"2%% of Total Armor Value granted as",			// *
	  6605,							"extra Weapon Value." },
	{ "* Scorn *", 					"Your debuffs ignore 5%% of enemy",
	  6606,							"Resistance." },
	{ "* Courage *", 				"Gain 1 additional Hit and Parry for",			// *
	  6607,							"every 100 missing hitpoints." },
	{ "* Master of None *", 		"+2 to all skill limits.",
	  6608,							"" },
	{ "* Necromancy *", 			"2%% more Companion Hit and Parry scores.",		// *
	  6609,							"" },
	{ "* Enigmatic *", 				"5%% reduced effect of debuffs on you.",
	  6610,							"" },
	{ "* Barkskin *", 				"2%% of Total Weapon Value granted as",			// *
	  6611,							"extra Armor Value." },
	{ "* Penance *", 				"0.2%% less damage taken for each buff",
	  6612,							"or debuff on you." },
//    "         '         '      ", "         '         '         '         ' "
	{ "* Spiked *", 				"+2 to Thorns.",
	  6613,							"" },
	{ "* Might *", 					"+2 to Strength.",
	  6614,							"" },
	{ "* Ironskin *", 				"Gain 1 additional Armor Value for",			// *
	  6615,							"every 200 total Attributes." },
	{ "* Decapitation *", 			"Cleave kills enemies left below",				// *
	  6616,							"2%% remaining health." },
	{ "* Sharkskin *", 				"5%% more total Thorns.",
	  6617,							"" },
	{ "* Razor Shell *", 			"10%% of Shield Armor Value is granted",		// *
	  6618,							"as extra Thorns." },
	{ "* Overlord *", 				"0.2%% more effect of Warcry and Rally",
	  6619,							"for every 10 total Strength." },
	{ "* Overwhelming Strength *", 	"2%% increased total Strength.",
	  6620,							"+1 to Strength Limit." },
	{ "* Towering *", 				"8%% more Armor Value from Shields.",			// *
	  6621,							"" },
	{ "* Bastion *", 				"4%% of total Resistance is granted as",
	  6622,							"extra Immunity." },
	{ "* Unbreakable *", 			"3%% more total Armor Value.",
	  6623,							"" },
	{ "* Deflecting *", 			"5%% of Shield Armor Value is granted as",		// *
	  6624,							"extra Parry Score." },
//    "         '         '      ", "         '         '         '         ' "
	{ "* Force *", 					"Gain 1 additional Weapon Value for",			// *
	  6625,							"every 200 total Attributes." },
	{ "* Dexterity *", 				"+2 to Agility.",
	  6626,							"" },
	{ "* Persistance *", 			"+10 Endurance.",
	  6627,							"" },
	{ "* Nocturne *", 				"5%% increased effect of Aria.",
	  6628,							"" },
	{ "* Valor *", 					"3%% more total Weapon Value.",
	  6629,							"" },
	{ "* Blade Dancer *", 			"8%% more Weapon Value from Dual Swords.",		// *
	  6630,							"" },
	{ "* Slaying *", 				"+1%% Critical Multiplier for every 10",
	  6631,							"total Agility." },
	{ "* Overwhelming Agility *", 	"2%% increased total Agility.",
	  6632,							"+1 to Agility Limit." },
	{ "* Axeman *", 				"2%% more damage dealt while using an",			// *
	  6633,							"Axe or Greataxe." },
	{ "* Overwhelm *", 				"5%% increased effect of Weaken and Crush.",	// *
	  6634,							"" },
	{ "* Perseverance *", 			"5%% more total Endurance.",
	  6635,							"" },
	{ "* Recycle *", 				"10%% of Endurance spent is granted as",		// *
	  6636,							"additional Mana." },
//    "         '         '      ", "         '         '         '         ' "
	{ "* Rapidity *", 				"+2 to Attack Speed.",
	  6637,							"" },
	{ "* Ruffian *", 				"+1 to Strength & +1 to Agility.",
	  6638,							"" },
	{ "* Passion *", 				"+2 to Spell Aptitude.",
	  6639,							"" },
	{ "* Alacrity *", 				"Zephyr deals 5%% more damage.",
	  6640,							"" },
	{ "* Swiftness *", 				"3%% more total Attack Speed.",
	  6641,							"" },
	{ "* Full Moon *", 				"10%% increased effect of bonuses granted",		// *
	  6642,							"during Full Moons." },
	{ "* Antagonizer *", 			"0.2%% more effect of Blind and Douse",
	  6643,							"for every 10 total Agility." },
	{ "* Harrier *", 				"1%% increased total Agility and",
	  6644,							"Strength." },
	{ "* Butchery *", 				"0.2%% more effect of Cleave for every",
	  6645,							"10 total Strength." },
	{ "* Conqueror *", 				"5%% more damage dealt to enemies beside",		// *
	  6646,							"or behind you." },
	{ "* Zealotry *", 				"5%% more total Spell Aptitude.",
	  6647,							"" },
	{ "* Crusade *", 				"+1 Skill Modifier per 50 Spell Aptitude.",
	  6648,							"" },
//    "         '         '      ", "         '         '         '         ' "
	{ "* Expansiveness *", 			"+1 to Area of Effect.",
	  6649,							"" },
	{ "* Potency *", 				"+1 to Willpower & +1 to Intuition.",
	  6650,							"" },
	{ "* Quickstep *", 				"+2 to Movement Speed.",
	  6651,							"" },
	{ "* Tormenter *", 				"Poison deals damage 5%% faster.",
	  6652,							"" },
	{ "* Grandiosity *", 			"10%% more total Area of Effect.",
	  6653,							"" },
	{ "* New Moon *", 				"10%% increased effect of bonuses granted",		// *
	  6654,							"during New Moons." },
	{ "* Coordination *", 			"0.2%% more effect of Lethargy for every",
	  6655,							"10 total Willpower." },
	{ "* Pragmatic *", 				"1%% increased total Willpower and",
	  6656,							"Intuition." },
	{ "* Hex Master *", 			"0.2%% more effect of Curse and Slow",
	  6657,							"for every 10 total Intuition." },
	{ "* Adroitness *", 			"5%% less damage taken from enemies beside",	// *
	  6658,							"or behind you." },
	{ "* Fleet-footed *", 			"3%% more total Movement Speed.",
	  6659,							"" },
	{ "* Acceleration *", 			"Haste grants 10%% more move speed to you.",
	  6660,							"" },
//    "         '         '      ", "         '         '         '         ' "
	{ "* Nimbleness *", 			"+2 to Cast Speed.",
	  6661,							"" },
	{ "* Wisdom *", 				"+2 to Willpower.",
	  6662,							"" },
	{ "* Vitality *", 				"+10 Hitpoints.",
	  6663,							"" },
	{ "* Denial *", 				"5%% chance to not be hit when you should",		// *
	  6664,							"have been." },
	{ "* Spellslinger *", 			"3%% more total Cast Speed.",
	  6665,							"" },
	{ "* Training *", 				"5%% more effective surround hit modifier",
	  6666,							"while using a Spear." },
	{ "* Waning *", 				"Spells gain an additional 10%% of",			// *
	  6667,							"Willpower towards attribute bonuses." },
	{ "* Overwhelming Willpower *",	"2%% increased total Willpower.",
	  6668,							"+1 to Willpower Limit." },
	{ "* Shaper *", 				"0.2%% more effect of Shadow Copy for",
	  6669,							"every 10 total Willpower." },
	{ "* Wraithlord *", 			"2%% of damage dealt by you is granted",		// *
	  6670,							"to your Companions as Hitpoints." },
	{ "* Constitution *", 			"5%% more total Hitpoints.",
	  6671,							"" },
	{ "* Barrier *", 				"Magic Shields and Shells affecting you",		// *
	  6672,							"take 10%% reduced damage from enemies." },
//    "         '         '      ", "         '         '         '         ' "
	{ "* Composure *", 				"+2 to Cooldown Rate.",
	  6673,							"" },
	{ "* Intellect *", 				"+2 to Intuition.",
	  6674,							"" },
	{ "* Wellspring *", 			"+10 Mana.",
	  6675,							"" },
	{ "* Detonation *", 			"Blast kills enemies left below",				// *
	  6676,							"2%% remaining health." },
	{ "* Serenity *", 				"2%% more total Cooldown Rate.",
	  6677,							"" },
	{ "* Refrigerate *", 			"3%% chance for skills to have no",				// *
	  6678,							"cooldown." },
	{ "* Psychosis *", 				"0.2%% more effect of Pulse for every 10",
	  6679,							"total Intuition." },
	{ "* Overwhelming Intuition *",	"2%% increased total Intuition.",
	  6680,							"+1 to Intuition Limit." },
	{ "* Waxing *", 				"Spells gain an additional 10%% of",			// *
	  6681,							"Intuition towards attribute bonuses." },
	{ "* Flow *", 					"10%% of overcapped Mana is granted as",
	  6682,							"additional Hitpoints." },
	{ "* Perpetuity *", 			"5%% more total Mana.",
	  6683,							"" },
	{ "* Repurpose *", 				"10%% of Mana spent is granted as",				// *
	  6684,							"additional Endurance." },
//    "         '         '      ", "         '         '         '         ' "
	{ "* Accuracy *", 				"+1 to Hit Score.",
	  6685,							"" },
	{ "* Boldness *", 				"+2 to Braveness.",
	  6686,							"" },
	{ "* Avoidance *", 				"+1 to Parry Score.",
	  6687,							"" },
	{ "* Assassination *", 			"5%% increased effect of Precision.",
	  6688,							"" },
	{ "* Rigor *", 					"1%% more total Hit Score.",
	  6689,							"" },
	{ "* Rebuke *", 				"5%% of damage taken from enemy thorns",		// *
	  6690,							"is reflected." },
	{ "* Perfectionism *", 			"0.2%% more effect of Finesse for every 10",
	  6691,							"total Braveness." },
	{ "* Overwhelming Braveness *",	"2%% increased total Braveness.",
	  6692,							"+1 to Braveness Limit." },
	{ "* Swordsman *", 				"2%% less damage taken while using a",			// *
	  6693,							"Sword or Twohander." },
	{ "* Mending *", 				"10%% increased effect of Heal and Regen.",		// *
	  6694,							"" },
	{ "* Flexibility *", 			"1%% more total Parry Score.",
	  6695,							"" },
	{ "* Revoke *", 				"5%% of damage taken from enemy critical",		// *
	  6696,							"hits is reflected." },
//    "         '         '      ", "         '         '         '         ' "
	{ "* Maiming *", 				"+2 to Top Damage.",
	  6697,							"" },
	{ "* Feast *", 					"+5 Hitpoints, Endurance, and Mana.",
	  6698,							"" },
	{ "* Half Moon *", 				"5%% increased effect of bonuses granted",		// *
	  6699,							"during Moons." },
	{ "* Lustful *", 				"+1%% base crit chance per empty ring",			// *
	  6700,							"slot." },
	{ "* Ravager *", 				"5%% more total Top Damage.",
	  6701,							"" },
	{ "* Culling *", 				"Critical Hits kill enemies left below",		// *
	  6702,							"2%% remaining health." },
	{ "* Wrath *", 					"0.2%% more effect of Rage & Calm per 50",
	  6703,							"missing Hitpoints, Endurance, and Mana." },
	{ "* Gluttony *", 				"3%% more Hitpoints, Endurance, and Mana.",
	  6704,							"" },
	{ "* Sloth *", 					"2%% of damage dealt is restored as",
	  6705,							"Hitpoints, Endurance, and Mana." },
	{ "* Pride *", 					"Your debuffs ignore 5%% of enemy",
	  6706,							"Immunity." },
	{ "* Madness *", 				"+1 to Spell Modifier.",
	  6707,							"" },
	{ "* Envious *", 				"+1 Spell Modifier per empty ring slot.",		// *
	  6708,							"" }
};

#define ST_OFFSET_IC		-12
#define ST_OFFSET_HL		-31

struct sk_icon sk_icon[12]={
	{  96, 165 }, // W
	{ 140,  89 }, // N
	{ 184, 165 }, // E
	{  96, 216 }, // WS
	{  52, 190 }, // WW
	{  52, 140 }, // WN
	{  96,  64 }, // NW
	{ 140,  38 }, // NN
	{ 184,  64 }, // NE
	{ 228, 140 }, // EN
	{ 228, 190 }, // EE
	{ 184, 216 }  // ES
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
	{ 25, "Neiseer, South", 				"\"The Raging Rivers.\"" },
	{ 24, "Neiseer, East", 					"\"The Violet Bog.\"" },
	{ 11, "Neiseer, North", 				"\"The Ravaged Prairie.\"" },
	{ 12, "Mausoleum, Basement", 			"\"An accursed tomb.\"" },
	{ 13, "Pentagram Quest, Novice", 		"\"Endless adventure begins.\"" },
	{ 14, "Pentagram Quest, Earth", 		"\"Smells of dirt and soot.\"" },
	{ 15, "Pentagram Quest, Fire", 			"\"Boiling and burning.\"" },
	{ 16, "Pentagram Quest, Jungle", 		"\"A verdant expanse.\"" },
	{ 17, "Pentagram Quest, Ice", 			"\"Frozen still for eternity.\"" },
	{ 18, "Pentagram Quest, Underwater",	"\"Embrace foul waters.\"" },
	{ 19, "Pentagram Quest, Onyx", 			"\"Depths known only by fear.\"" },
	{ 26, "The Burning Plains", 			"\"The archon waits.\"" }
//    nr, "123456789012345678901234567890",	"123456789012345678901234567890"
};

int skill_cmp(const void *a,const void *b)
{
	const struct skilltab *c,*d;
	int m1,m2;

	c=a; d=b;

	m1=c->nr; m2=d->nr;
	
	if (m1==99 && m2!=99) return  1;
	if (m2==99 && m1!=99) return -1;
	
	if (pl.skill[m1][0]==0 && pl.skill[m2][0]!=0) return  1;
	if (pl.skill[m2][0]==0 && pl.skill[m1][0]!=0) return -1;
	
	if (m1==52 && m2!=52 && !KNOW_IDENTIFY) return  1;
	if (m2==52 && m1!=52 && !KNOW_IDENTIFY) return -1;
	
	if ((m1==53||m1==54) && (m2!=53&&m2!=54) && !IS_LYCANTH) return  1;
	if ((m2==53||m2==54) && (m1!=53&&m1!=54) && !IS_LYCANTH) return -1;
	
	if ((m1==50||m1==51||m1==52||m1==53||m1==54) && 
		(m2!=50&&m2!=51&&m2!=52&&m2!=53&&m2!=54)) return -1;
	if ((m2==50||m2==51||m2==52||m2==53||m2==54) && 
		(m1!=50&&m1!=51&&m1!=52&&m1!=53&&m1!=54)) return  1;
	
	// Stealth, Resistance, Immunity -- these are active even if you don't know them. m==8||m==23||m==28||m==29||m==30||m==32
	if (pl.skill[m1][0]==0 && pl.skill[m2][0]==0 && 
		(m1==8||m1==23||m1==28||m1==29||m1==30||m1==32||m1==44||m1==50||m1==51||m1==52||m1==53||m1==54) && 
		(m2!=8&&m2!=23&&m2!=28&&m2!=29&&m2!=30&&m2!=32&&m2!=44&&m2!=50&&m2!=51&&m2!=52&&m2!=53&&m2!=54)) return -1;
	if (pl.skill[m2][0]==0 && pl.skill[m1][0]==0 && 
		(m2==8||m2==23||m2==28||m2==29||m2==30||m2==32||m2==44||m2==50||m2==51||m2==52||m2==53||m2==54) && 
		(m1!=8&&m1!=23&&m1!=28&&m1!=29&&m1!=30&&m1!=32&&m1!=44&&m1!=50&&m1!=51&&m1!=52&&m1!=53&&m1!=54)) return  1;
	
	if (m1==44 && !IS_SEYAN_DU) return  1;
	if (m2==44 && !IS_SEYAN_DU) return -1;
	
	if (c->sortkey>d->sortkey) return  1;
	if (c->sortkey<d->sortkey) return -1;

	return strcmp(c->name,d->name);
}

// from main.c
extern int quit;

int idle=0;
int ttime=0,xtime=0;
int ctick=0;

int do_exit=0;

int xoff=0,yoff=0;

extern int selected_char;

struct look look={0,{0},0,0,"",0,0,0,0};

// ************* CHARACTER ****************

struct cplayer pl;

static char *rank[25]={
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
static int rank_sprite[25]={
	   10,    11,    12,    13,    14, 
	   15,    16,    17,    18,    19, 
	   20,    21,    22,    23,    24, 
	   25,    26,    27,    28,    29,
	   30,     6,     7,     8,     9
};

int stat_raised[108]={0,0,0,0,0,0,0,0,0,0,0,0,0,};
int stat_points_used=0;

int points2rank(int v)
{
	if (v<      250)	return( 0); // Private
	if (v<     1750)	return( 1); // Private FIrst Class
	if (v<     7000)	return( 2); // Lance Corporal
	if (v<    21000)	return( 3); // Corporal
	if (v<    52500)	return( 4); // Sergeant
	if (v<   115500)	return( 5); // Staff Sergeant
	if (v<   231000)	return( 6); // Master Sergeant
	if (v<   429000)	return( 7); // First Sergeant
	if (v<   750750)	return( 8); // Sergeant Major
	if (v<  1251250)	return( 9); // Second Lieutenant
	if (v<  2002000)	return(10); // First Lieutenant
	if (v<  3094000)	return(11); // Captain
	if (v<  4641000)	return(12); // Major
	if (v<  6783000)	return(13); // Lieutenant Colonel
	if (v<  9690000)	return(14); // Colonel
	if (v< 13566000)	return(15); // Brigadier General
	if (v< 18653250)	return(16); // Major General
	if (v< 25236750)	return(17); // Lieutenant General
	if (v< 33649000)	return(18); // General
	if (v< 44275000)	return(19); // Field Marshal
	if (v< 57557500)	return(20); // Knight
	if (v< 74002500)	return(21); // Baron
	if (v< 94185000)	return(22); // Earl
	if (v<118755000)	return(23); // Marquess
						return(24); // Warlord
}

int rank2points(int v)
{
	if (v==1)	return 250;
	if (v==2)	return 1750;
	if (v==3)	return 7000;
	if (v==4)	return 21000;
	if (v==5)	return 52500;
	if (v==6)	return 115500;
	if (v==7)	return 231000;
	if (v==8)	return 429000;
	if (v==9)	return 750750;
	if (v==10)	return 1251250;
	if (v==11)	return 2002000;
	if (v==12)	return 3094000;
	if (v==13)	return 4641000;
	if (v==14)	return 6783000;
	if (v==15)	return 9690000;
	if (v==16)	return 13566000;
	if (v==17)	return 18653250;
	if (v==18)	return 25236750;
	if (v==19)	return 33649000;
	if (v==20)	return 44275000;
	if (v==21)	return 57557500;
	if (v==22)	return 74002500;
	if (v==23)	return 94185000;
	if (v>=24)	return 118755000;
				return 0;
}

/* Calculates experience to next level from current experience and the
   points2rank() function. As no inverse function is supplied we use a
   binary search to determine the experience for the next level.
   If the given number of points corresponds to the highest level,
   return 0. */
int points_tolevel(int curr_exp)
{
        int curr_level, next_level, r, j;  //, p0, p5, p9;

		if (!curr_exp) return 250;	//0 exp
        curr_level = points2rank(curr_exp);
        if (curr_level == 24) return 118755000;
        next_level = curr_level + 1;

		r = rank2points(next_level);
		j = r-curr_exp;
		
		return j;
}

int attrib_needed(int n,int v)
{
	if (v>=pl.attrib[n][2])	return HIGH_VAL;

	return v*v*v*pl.attrib[n][3]/20;
}

int hp_needed(int v)
{
	if (v>=pl.hp[2]) return HIGH_VAL;

	return v*pl.hp[3];
}

int mana_needed(int v)
{
	if (v>=pl.mana[2]) return HIGH_VAL;

	return v*pl.mana[3];
}

int skill_needed(int n,int v)
{
	if (v>=pl.skill[n][2]) return HIGH_VAL;

	return max(v,v*v*v*pl.skill[n][3]/40);
}



// ************* MAP **********************

struct cmap *map=NULL;

void eng_init_map(void)
{
	int n;

	map=calloc(screen_renderdist*screen_renderdist*sizeof(struct cmap),1);

	for (n=0; n<screen_renderdist*screen_renderdist; n++)	map[n].ba_sprite=SPR_EMPTY;
}

void eng_init_player(void)
{
	memset(&pl,0,sizeof(struct cplayer));
}

// ************* DISPLAY ******************

unsigned int	show_shop=0,show_wps=0,show_tree=0,dept_page=0;

unsigned int	show_book=0,show_motd=0,show_newp=0,show_tuto=0,tuto_page=0,tuto_max=0;

int				inv_pos=0,skill_pos=0,wps_pos=0,hudmode=0,mm_magnify=2;

unsigned int   show_look=0,
look_nr=0,			// look at char/item nr
look_type=0,		// 1=char, 2=item
look_timer=0;		// look_timer

unsigned char   inv_block[20]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

extern int inv_object;			// object carried with the mouse cursor
extern int mouse_x,mouse_y;	// current mouse coordinates

#define XS	49
#define LL	22
#define XLL (22*10)
static char logtext[XLL][60];
static char logfont[XLL];

#define XMS	43
#define MLL 27
static char motdtext[MLL][60];
static char motdfont[MLL];

#define MAXTS            20

// 2020 - Zarrotsu
#define GUI_HP_COUNT_X	  9
#define GUI_HP_COUNT_Y	288
#define GUI_EN_COUNT_X	  9
#define GUI_EN_COUNT_Y	302
#define GUI_MP_COUNT_X	  9
#define GUI_MP_COUNT_Y	316

#define GUI_MONEY_X		267 // 442
#define GUI_MONEY_Y		114

#define GUI_UPDATE_X	144
#define GUI_UPDATE_Y	260

#define GUI_UPOINTS_X	189
#define GUI_UPOINTS_Y	260

//					   HEAD,NECK,BODY,ARMS,BELT,CHRM,FEET,LHND,RHND,CLOK,LRNG,RRNG,CHRM2
int gui_equ_x[]		= { 738, 700, 738, 704, 738, 777, 738, 806, 670, 772, 776, 700, 801 };
int gui_equ_y[]		= {   5,  18,  39,  56,  73,  17, 107,  56,  56,  56,  94,  94,  17 };

#define GUI_BAR_X		845
#define GUI_BAR_HP		131
#define GUI_BAR_EN		138
#define GUI_BAR_MP		145

#define GUI_F_COL1		1084
#define GUI_F_COL2		1132
#define GUI_F_COL3		1180
#define GUI_F_COL4		1228
#define GUI_F_ROW1		 667
#define GUI_F_ROW2		 682
#define GUI_F_ROW3		 697

#define GUI_WV_X		1112
#define GUI_WV_Y		 261

#define GUI_XPBAR_X		1109
#define GUI_XPBAR_Y		 300
#define GUI_XPBAR_W		 165

#define GUI_LOG_X		977

#define GUI_DPS_X		 265
#define GUI_DPS_Y		 120


//#define GUI_SHOP_X	220
//#define GUI_SHOP_Y	260
#ifndef GUI_SHOP_X
	#define GUI_SHOP_X		((1280/2)-(320/2))
	#define GUI_SHOP_Y		((736/2)-(320/2)+72)
#endif

#define GUI_BAR_BLU		0x00B0
#define GUI_BAR_GRE		0x0B00
#define GUI_BAR_RED		0xB000

#define GUI_BAR_EXP		0xBB00

#define CHAR_BAR_HP		0x0B00
#define CHAR_BAR_RD		0xA000
#define CHAR_BAR_BL		0x0020

//

int load=0;

int st_learned_skill(int st_val, int v)
{	// Returns the bitwise value of the parsed value v from the input value st_val
	return ( (st_val>>((12-v)+4))%2 );
}

int st_skill_pts_have(int st_val)
{	// Returns the available skill points
	return ( st_val%16 );
}

int st_skill_pts_all(int st_val)
{	// Returns the available skill points plus spent skill points
	int n, m = 0;
	for (n=0;n<12;n++)
	{
		m += st_learned_skill(st_val, n+1);
	}
	return ( m + st_skill_pts_have(st_val) );
}

int at_score(int n)
{
	return ( (pl.attrib[n][4] << 8) | pl.attrib[n][5] );
}

int sk_score(int n)
{
	return ( (pl.skill[n][4] << 8) | pl.skill[n][5] );
}

int pl_speed=0, pl_atksp=0, pl_spmod=0, pl_skmod=0, pl_spapt=0, pl_movsp=0;
int pl_critc=0, pl_critm=0, pl_topdm=0, pl_topd2=0, pl_reflc=0, pl_aoebn=0;
int pl_hitsc=0, pl_parry=0, pl_coold=0, pl_casts=0, pl_dmgbn=0;
int pl_flags=0, pl_flagb=0, pl_flagc=0, pl_basel=0, pl_dmgml=0, pl_dmgrd=0;
int pl_dlow=0,  pl_dhigh=0, pl_dps=0,   pl_hitdm=0, pl_cdrate=0, pl_armor=0;
int sk_proxi=0, sk_immun=0, sk_resis=0, sk_conce=0, sk_ghost=0, sk_poiso=0, sk_slowv=0, sk_curse=0;
int sk_blast=0, sk_scorc=0, sk_pulse=0, sk_pucnt=0, sk_razor=0, sk_leapv=0, sk_blind=0;
int sk_water=0, sk_cleav=0, sk_weake=0, sk_warcr=0, sk_regen=0, sk_restv=0, sk_medit=0;
int sk_shado=0, sk_rally=0, sk_immol=0, sk_shadd=0, sk_metab=0;
int pl_ehp=0,   cri_leap=0, sk_bleed=0, sk_rage=0,  sk_calm=0,  sk_letha=0;
int sk_hem=0,   sk_rage2=0, sk_calm2=0;
int sk_bless=0, sk_enhan=0, sk_prote=0, sk_mshie=0, sk_mdura=0, sk_haste=0, sk_healr=0;
int coo_clea=0, coo_leap=0, coo_blas=0, coo_pois=0, coo_puls=0, coo_ghos=0, coo_shad=0;
int coo_blin=0, coo_warc=0, coo_weak=0, coo_curs=0, coo_slow=0;

void init_meta_stats(void)
{
	int moonmult = 20;
	int hpmult=0, endmult=0, manamult=0;
	int race_reg = 0, race_res = 0, race_med = 0;
	int len = 100;
	
	if (pl.worn[WN_SPMOD]==NULL) return;
	
	// Player Speed and Attack Speed - WN_SPEED
	pl_speed = SPEED_CAP - pl.worn[WN_SPEED]; 
	pl_atksp = pl_speed + pl.worn_p[WN_SPEED]; 
	pl_movsp = pl_speed + pl.end[0];
	
	if (pl_speed > SPEED_CAP) pl_speed = SPEED_CAP; if (pl_speed < 0) pl_speed = 0;
	if (pl_atksp > SPEED_CAP) pl_atksp = SPEED_CAP; if (pl_atksp < 0) pl_atksp = 0;
	if (pl_movsp > SPEED_CAP) pl_movsp = SPEED_CAP; if (pl_movsp < 0) pl_movsp = 0;

	// Player Spell Mod and Spell Apt - WN_SPMOD
	pl_spmod = pl.worn[WN_SPMOD];
	pl_skmod = 100;
	pl_spapt = pl.worn_p[WN_SPMOD];
	
	// Player Crit chance and Crit Multiplier - WN_CRIT
	pl_critc = pl.worn[WN_CRIT];
	pl_critm = pl.worn_p[WN_CRIT];
	
	// Player Top damage and Reflection - WN_TOP
	pl_topdm = pl.worn[WN_TOP]+6+8;
	pl_reflc = pl.worn_p[WN_TOP];
	
	// Player Hit and Parry - WN_HITPAR
	pl_hitsc = pl.worn[WN_HITPAR];
	pl_parry = pl.worn_p[WN_HITPAR];
	
	// Player Cooldown and Cast Speed - WN_CLDWN
	pl_coold = pl.worn[WN_CLDWN];
	pl_casts = pl_speed/2 + pl.worn_p[WN_CLDWN]*2; 
	
	if (pl_casts > SPEED_CAP) pl_casts = SPEED_CAP; if (pl_casts < 0) pl_casts = 0;
	
	pl_aoebn = pl.end[1];
	pl_dmgbn = pl.end[2];
	pl_dmgrd = pl.end[3];
	pl_flagc = pl.end[4];
	
	// Player Flags from special items
	pl_flags = pl.worn[WN_FLAGS];
	pl_flagb = pl.worn_p[WN_FLAGS];
	pl_basel = 100;
	pl_dmgml = 100;
	
	// Book - Damor's Grudge (cooldown bonus)
	if (pl_flags & (1 <<  0))
		pl_basel =  90;
	
	// Tarot - Strength (damage multiplier)  // *pl_dmgbn/10000*pl_dmgml/100
	if (pl_flags & (1 <<  1))
		pl_dmgml = 120;
	
	// Player DPS - pl_dlow, pl_dhigh, pl_dps
	pl_dlow  = (pl.weapon*pl_dmgml/100)/4*pl_dmgbn/10000;
	pl_dhigh = pl.weapon+pl_topdm;
	pl_topd2 = ((pl_topdm+pl_topdm*pl_critc*pl_critm/1000000)*pl_dmgml/100)/4*pl_dmgbn/10000;
	pl_dhigh = ((pl_dhigh+pl_dhigh*pl_critc*pl_critm/1000000)*pl_dmgml/100)/4*pl_dmgbn/10000;
	pl_hitdm = (pl_dlow+pl_dhigh+(T_LYCA_SK(6)?pl_topd2/2:0))/2;
	pl_dps   = pl_hitdm*pl_atksp;
	
	// Player cooldown rate - pl_cdrate
	pl_cdrate = 100 * pl_basel / max(25, pl_coold);
	
	/*
		Moon multiplier adjustments
	*/
	// FULL MOON
	if (pl_flagb & (1 <<  8))
	{
		moonmult = 30;
	}
	// NEW MOON
	if (pl_flagb & (1 <<  9))
	{
		moonmult = 40;
	}
	hpmult = endmult = manamult = moonmult;
	
	// Reverse Star
	if (pl_flagc&(1<<10))
	{
		pl_skmod = pl_spmod;
		pl_spmod = 100;
	}
	
	/*
		Additional skill bonuses for GUI
	*/
	sk_proxi = sk_score(44) / (300/12);
	sk_ghost = sk_score(27)*pl_spmod/100 * 5 / 11;
	sk_poiso = (sk_score(42)*pl_spmod/100 + 5) * DAM_MULT_POISON / 300;
	sk_blast = sk_score(24)*pl_spmod/100 * 2 * DAM_MULT_BLAST/1000;
	sk_scorc = 1000 + sk_score(24)*pl_spmod/100;
	sk_pulse = (sk_score(43)+(sk_score(43)*(T_ARHR_SK(7)?at_score(AT_INT)/1000:0)))*pl_spmod/100 * 2 * DAM_MULT_PULSE/1000;
	sk_pucnt = (60*2*100 / (3*pl_cdrate));
	sk_water = 25 * TICKS;
	sk_cleav = ((sk_score(40)+(sk_score(40)*(T_WARR_SK(9)?at_score(AT_STR)/1000:0)))*pl_skmod/100+pl.weapon/4+pl_topdm/4+(T_ARTM_SK(4)?pl_reflc:0)) * 2 * DAM_MULT_CLEAVE/1000;
	sk_warcr = -(2+((sk_score(35)+(sk_score(35)*(T_ARTM_SK(7)?at_score(AT_STR)/2000:0)))*pl_skmod/100/(10/3)) / 5);
	sk_rally = sk_score(35)*pl_skmod/100/10;
	sk_shado = (sk_score(46)+(sk_score(46)*(T_SUMM_SK(9)?at_score(AT_WIL)/1000:0)))*pl_spmod/100 * 5 / 11;
	sk_shadd = 15 + (sk_score(46)+(sk_score(46)*(T_SUMM_SK(9)?at_score(AT_WIL)/1000:0)))*pl_spmod/500;
	sk_weake = -(sk_score(41)*pl_skmod/100 / 4 + 2);
	
	sk_bleed = (sk_cleav + 5) * DAM_MULT_BLEED / 150;
	
	sk_hem   = ((pl.hp[5] - pl.a_hp)/10) + ((pl.end[5] - pl.a_end)/10) + ((pl.mana[5] - pl.a_mana)/10);
	
	sk_rage  = sk_rage2 = sk_score(22);
	if (T_LYCA_SK(9)) sk_rage = sk_rage + (sk_rage * sk_hem / 500);
	sk_rage2 = 10000 * (1000 + (IS_SEYAN_DU?(sk_rage*2/3):(sk_rage))) / 1000;
	sk_rage  = min(127, IS_SEYAN_DU?(sk_rage/6 + 5):(sk_rage/4 + 5));
	
	sk_calm  = sk_calm2 = sk_score(22);
	if (T_LYCA_SK(7)) sk_calm = sk_calm + (sk_calm * sk_hem / 500);
	sk_calm2 = 10000 * (1000 - (IS_SEYAN_DU?(sk_calm*2/3):(sk_calm))) / 1000;
	sk_calm  = min(127, IS_SEYAN_DU?(sk_calm/6 + 5):(sk_calm/4 + 5)) * -1;
	
	sk_letha = (sk_score(15)+(sk_score(15)*(T_SORC_SK(7)?at_score(AT_WIL)/1000:0)))*pl_spmod/100/(IS_SEYAN_DU?4:3);
	sk_bless = min(127, (sk_score(21)*pl_spmod/100*2/3) / 5 + 3);
	sk_enhan = min(127, (IS_SEYAN_DU?(sk_score(18)*pl_spmod/100/6+3):(sk_score(18)*pl_spmod/100/4+4)));
	sk_prote = min(127, (IS_SEYAN_DU?(sk_score(17)*pl_spmod/100/6+3):(sk_score(17)*pl_spmod/100/4+4)));
	sk_mdura = sk_score(11)*pl_spmod/100 * ((pl_flagb&(1<<10))?128:256);
	sk_mshie = min(127, (IS_SEYAN_DU?(sk_mdura/((pl_flagb&(1<<10))?768:1536)+1):(sk_mdura/((pl_flagb&(1<<10))?512:1024)+1)));
	sk_mdura = sk_mdura/20;
	sk_haste = min(300, 10 + (sk_score(47)*pl_spmod/100)/ 6) + min(127, 5 + (sk_score(47)+6)/12);
	sk_healr = ((pl_flags&(1<<14))?sk_score(26)*pl_spmod/100*1875/20:sk_score(26)*pl_spmod/100*2500)/1000;
	
	if (pl_flagb & (1<<7))	// Tarot - Rev.Justice (Leap dmg reduced)
	{
		sk_leapv = (sk_score(49)*pl_skmod/100+pl.weapon/4) * 2 * DAM_MULT_HIT/1000;
	}
	else
	{
		sk_leapv = (sk_score(49)*pl_skmod/100+pl.weapon/4) * 2 * DAM_MULT_LEAP/1000;
	}
	
	
	if (IS_MERCENARY || IS_WARRIOR || IS_SORCERER)
	{
		sk_blind = -((sk_score(37)+(sk_score(37)*(T_WARR_SK(7)?at_score(AT_AGL)/2000:0)))*pl_skmod/100 / 6 + 2);
	}
	else
	{
		sk_blind = -(sk_score(37)*pl_skmod/100 / 8 + 1);
	}
	
	// Tarot - Hanged Man (immunity&resistance)
	if (pl_flags & (1 <<  2))
	{
		sk_immun = sk_score(32) + sk_score(23)/3;
		sk_resis = sk_score(23) * 2/3;
	}
	else
	{
		sk_immun = sk_score(32);
		sk_resis = sk_score(23);
	}
	
	// Book - Great Prodigy (concentrate bonus)
	if (pl_flags & (1 <<  3))
	{
		sk_conce = max(1, 100 - 100 * sk_score(34) / 300);
	}
	else
	{
		sk_conce = max(1, 100 - 100 * sk_score(34) / 400);
	}
	
	// Book - Venom Compendium (poison bonus)
	if (pl_flags & (1 <<  4))
		sk_poiso = sk_poiso * 5 / 4;
	// Tree - 10% faster = 11.1% more damage
	if (T_SORC_SK(4))
		sk_poiso = sk_poiso * 10 / 9;
	// Tarot - Rev.Tower (Venom)
	if (pl_flagb & (1 << 14))
		sk_poiso = sk_poiso / 2;
	
	sk_razor = (sk_score(7)*pl_spmod/100 + max(0,(pl_atksp-120))/3) * 2 * DAM_MULT_ZEPHYR/1000;
	
	if (IS_SEYAN_DU || IS_BRAVER)
	{
		sk_weake = -(sk_score(41) / 6 + 2);
	}
	
	// Tarot - Emperor (slow bonus)
	if (pl_flags & (1 <<  5))
	{
		sk_slowv = -(30 + ((sk_score(19)+(sk_score(19)*(T_SORC_SK(9)?at_score(AT_INT)/2500:0)))*pl_spmod/100)/4);
	}
	else
	{
		sk_slowv = -(30 + ((sk_score(19)+(sk_score(19)*(T_SORC_SK(9)?at_score(AT_INT)/2500:0)))*pl_spmod/100)/3);
	}
	
	// Tarot - Tower (curse bonus)
	if (pl_flags & (1 <<  6))
	{
		sk_curse = -(2 + ((((sk_score(20)+(sk_score(20)*(T_SORC_SK(9)?at_score(AT_INT)/2500:0)))*pl_spmod/100)*5/4)-4)/5);
	}
	else
	{
		sk_curse = -(2 + (((sk_score(20)+(sk_score(20)*(T_SORC_SK(9)?at_score(AT_INT)/2500:0)))*pl_spmod/100)-4)/5);
	}
	
	sk_metab = 0;
	
	// Metabolism skill
	if (pl.skill[10][0]) sk_metab = sk_score(10)/2;
	sk_water = max(1, (250 - sk_metab) * (200 - sk_metab) / 200) * 20/10;
	
	// Amulet - Water breathing (degen/2)		0 1 1
	if (!(pl_flagb & (1 <<  0)) && (pl_flagb & (1 <<  1)) && (pl_flagb & (1 <<  2)))
	{
		sk_water /= 4;
	}
	
	// Immolate
	sk_immol = pl.hp[4] * 30 / 100;
	
	// Book - Burning
	if (pl_flagb & (1 << 13)) sk_immol = sk_immol + pl.hp[4]/25;
	
	// Immolate damage calc
	sk_immol = sk_immol*3/2;
	sk_immol = 10 + sk_immol*4;
	
	if (pl_flagc & (1<<2)) // 20% more weaken effect
		sk_weake = sk_weake * 6/5;
	if (pl_flagc & (1<<3)) // 20% more slow effect
		sk_slowv = sk_slowv * 6/5;
	if (pl_flagc & (1<<4)) // 20% more curse effect
		sk_curse = sk_curse * 6/5;
	if (pl_flagc & (1<<5)) // 20% more poison effect
		sk_poiso = sk_poiso * 6/5;
	if (pl_flagc & (1<<6)) // 20% more bleed effect
		sk_bleed = sk_bleed * 6/5;
	if (pl_flagc & (1<<7)) // 20% more blind effect
		sk_blind = sk_blind * 6/5;
	if (pl_flagc & (1<<8)) // 20% more heal effect
		sk_healr = sk_healr * 6/5;
	
	/*
		Regeneration stats
	*/
	race_reg = sk_score(28) * moonmult / 20 + sk_score(28) * pl.hp[5]/2000;
	race_res = sk_score(29) * moonmult / 20 + sk_score(29) * pl.end[5]/2000;
	race_med = sk_score(30) * moonmult / 20 + sk_score(30) * pl.mana[5]/2000;
	
	// Tarot - Moon :: While not full mana, life regen is mana regen
	if ((pl_flags & (1 << 11)) && (pl.a_mana<pl.mana[5]))
	{
		race_med += race_reg;	race_reg -= race_reg;
		manamult += hpmult;		hpmult   -= hpmult;
	}
	// Tarot - Sun :: While not full life, end regen is life regen
	if ((pl_flags & (1 << 12)) && (pl.a_hp<pl.hp[5]))
	{
		race_reg += race_res;	race_res -= race_res;
		hpmult   += endmult;	endmult  -= endmult;
	}
	// Tarot - World :: While not full end, mana regen is end regen
	if ((pl_flags & (1 << 13)) && (pl.a_end<pl.end[5]))
	{
		race_res += race_med;	race_med -= race_med;
		endmult  += manamult;	manamult -= manamult;
	}
	
	// Meditate added to Hitpoints
	if (pl_flagc & (1<<0))
	{
		race_reg += race_med/2;
		hpmult   += manamult/2;
	}
	// Rest added to mana
	if (pl_flagc & (1<<1))
	{
		race_med += race_res/2;
		manamult += endmult/2;
	}
	
	sk_regen = (pl.skill[28][0]?race_reg:0) + hpmult   * 2;
	sk_restv = (pl.skill[29][0]?race_res:0) + endmult  * 3;
	sk_medit = (pl.skill[30][0]?race_med:0) + manamult * 1;
	
	pl_armor = pl.armor;
	
	if (pl_flagb&(1<<4)) // Bone Armor
	{
		sk_regen += pl_armor * 25 / 2;
		pl_armor = 0;
	}
	
	// Amulet - Standard Ankh			1 0 0
	if ((pl_flagb & (1 <<  0)) && !(pl_flagb & (1 <<  1)) && !(pl_flagb & (1 <<  2)))
	{
		sk_regen += pl.skill[28][0]?race_reg/12:0;
		sk_restv += pl.skill[29][0]?race_res/12:0;
		sk_medit += pl.skill[30][0]?race_med/12:0;
	}
	// Amulet - Amber Ankh (Life) 		0 1 0
	if (!(pl_flagb & (1 <<  0)) && (pl_flagb & (1 <<  1)) && !(pl_flagb & (1 <<  2)))
	{
		sk_regen += pl.skill[28][0]?race_reg/ 6:0;
		sk_restv += pl.skill[29][0]?race_res/24:0;
		sk_medit += pl.skill[30][0]?race_med/24:0;
	}
	// Amulet - Turquoise Ankh (End)	1 1 0
	if ((pl_flagb & (1 <<  0)) && (pl_flagb & (1 <<  1)) && !(pl_flagb & (1 <<  2)))
	{
		sk_regen += pl.skill[28][0]?race_reg/24:0;
		sk_restv += pl.skill[29][0]?race_res/ 6:0;
		sk_medit += pl.skill[30][0]?race_med/24:0;
	}
	// Amulet - Garnet Ankh (Mana)		0 0 1
	if (!(pl_flagb & (1 <<  0)) && !(pl_flagb & (1 <<  1)) && (pl_flagb & (1 <<  2)))
	{
		sk_regen += pl.skill[28][0]?race_reg/24:0;
		sk_restv += pl.skill[29][0]?race_res/24:0;
		sk_medit += pl.skill[30][0]?race_med/ 6:0;
	}
	// Amulet - True Ankh				1 0 1
	if ((pl_flagb & (1 <<  0)) && !(pl_flagb & (1 <<  1)) && (pl_flagb & (1 <<  2)))
	{
		sk_regen += pl.skill[28][0]?race_reg/ 6:0;
		sk_restv += pl.skill[29][0]?race_res/ 6:0;
		sk_medit += pl.skill[30][0]?race_med/ 6:0;
	}
	
	sk_regen = sk_regen * 20/10;
	sk_restv = sk_restv * 20/10;
	sk_medit = sk_medit * 20/10;
	
	// add *pl_dmgbn/10000 to damage values for the HUD
	sk_cleav = sk_cleav*pl_dmgbn/10000*pl_dmgml/100;
	sk_leapv = sk_leapv*pl_dmgbn/10000*pl_dmgml/100;
	sk_blast = sk_blast*pl_dmgbn/10000*pl_dmgml/100;
	sk_poiso = sk_poiso*pl_dmgbn/10000*pl_dmgml/100;
	if (!(pl_flagb&(1<<6))) sk_pulse = sk_pulse*pl_dmgbn/10000*pl_dmgml/100;
	sk_razor = sk_razor*pl_dmgbn/10000*pl_dmgml/100;
	sk_immol = sk_immol*pl_dmgbn/10000*pl_dmgml/100;
	pl_reflc = pl_reflc*pl_dmgbn/10000*pl_dmgml/100;
	
	cri_leap = sk_leapv*pl_critm/100;
	
	// Acedia - Sprite 5556
	if (pl.worn[WN_RHAND] == 5556) len = len * 3/4; // less
	if (pl.worn[WN_LHAND] == 5556) len = len * 6/4; // more
	
	coo_clea = 500 * pl_cdrate / 100 * len / 100;
	coo_leap = 500 * pl_cdrate / 100 * len / 100;
	coo_blas = (600-T_ARHR_SK(4)*25) * pl_cdrate / 100 * len / 100;
	coo_pois = 500 * pl_cdrate / 100 * len / 100;
	coo_puls = 600 * pl_cdrate / 100 * len / 100;
	coo_ghos = 800 * pl_cdrate / 100 * len / 100;
	coo_shad = 400 * pl_cdrate / 100 * len / 100;
	coo_blin = 300 * pl_cdrate / 100 * len / 100;
	coo_warc = 300 * pl_cdrate / 100 * len / 100;
	coo_weak = 300 * pl_cdrate / 100 * len / 100;
	coo_curs = 400 * pl_cdrate / 100 * len / 100;
	coo_slow = 400 * pl_cdrate / 100 * len / 100;
	
	pl_ehp = pl.hp[5]*10000/pl_dmgrd;
	
	if (pl_flagc & (1<<9)) // 20% damage shifted to end/mana
		pl_ehp = pl_ehp * 100 /  80;
	if (pl_flagc & (1<<11)) // 5% chance to not be hit by melee
		pl_ehp = pl_ehp * 100 /  95;
	if (pl_flagc & (1<<12)) // 20% damage shifted to end/mana
		pl_ehp = pl_ehp * 100 /  80;
	if (pl_flagc & (1<<14)) // 10% damage null/shifted to endurance
		pl_ehp = pl_ehp * 100 /  90;
	if (pl_flags & (1<<9)) // 20% damage null/shifted to mana
		pl_ehp = pl_ehp * 100 /  80;
}

void meta_stat(int flag, int n, int font, char* va, int vb, int vc, char* ve)
{
	int m = 8;
	
	if (flag)		m += 8*14;
	
					  dd_xputtext(9,  m+n*14,font,"%-20.20s", va    );
	if (vc>=0)		  dd_xputtext(140,m+n*14,font,"%4d.%02d", vb, vc);
	else if (vb>=0)	  dd_xputtext(140,m+n*14,font,"%7d",      vb    );
	else if (flag==2) dd_xputtext(140,m+n*14,font,"%7d",      vb    );
					  dd_xputtext(189,m+n*14,font,"%-7.7s",   ve    );
}

void show_meta_stats(int n)
{
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
	
	if (pl.worn[WN_SPMOD]==NULL) return;
	
	if (n<7)					// Topmost standard stats
	{
		switch (n)
		{
			case  0: meta_stat(0,n,1,"Cooldown Duration", pl_cdrate/100, pl_cdrate%100, "x"); break;
			case  1: meta_stat(0,n,4,"Spell Aptitude",    pl_spapt,      -1,            "" ); break;
			case  2: if (pl_flagc&(1<<10)) meta_stat(0,n,5,"Skill Modifier", pl_skmod /100, pl_skmod %100, "x");
					 else meta_stat(0,n,4,"Spell Modifier", pl_spmod /100, pl_spmod %100, "x"); break;
			case  3: meta_stat(0,n,6,"Base Action Speed", pl_speed/100,  pl_speed%100,  "" ); break;
			case  4: meta_stat(0,n,6,"Movement Speed",    pl_movsp/100,  pl_movsp%100,  "" ); break;
			case  5: meta_stat(0,n,7,"Hit Score",         pl_hitsc,      -1,            "" ); break;
			case  6: meta_stat(0,n,7,"Parry Score",       pl_parry,      -1,            "" ); break;
			default: break;
		}
	}
	else if (hudmode==1)		// Offense Stats
	{
		n-=7;
		switch (n+skill_pos)
		{
			case  0: meta_stat(1,n,9,"  Passive Stats:",     -1,           -1,           ""       ); break;
			case  1: if (pl_dmgbn!=10000)
					 meta_stat(1,n,1,"Damage Multiplier",    pl_dmgbn/100, pl_dmgbn%100, "%"      ); break;
			case  2: meta_stat(1,n,7,"Est. Melee DPS",       pl_dps/100,   pl_dps%100,   ""       ); break;
			case  3: meta_stat(1,n,5,"Est. Melee Hit Dmg",   pl_hitdm,     -1,           ""       ); break;
			case  4: meta_stat(1,n,5,"Critical Multiplier",  pl_critm,     -1,           "%"      ); break;
			case  5: meta_stat(1,n,5,"Critical Chance",      pl_critc/100, pl_critc%100, "%"      ); break;
			case  6: meta_stat(1,n,5,"Melee Ceiling Damage", pl_dhigh,     -1,           ""       ); break;
			case  7: meta_stat(1,n,5,"Melee  Floor  Damage", pl_dlow,      -1,           ""       ); break;
			case  8: meta_stat(1,n,6,"Attack Speed",         pl_atksp/100, pl_atksp%100, ""       ); break;
			case  9: meta_stat(1,n,6,"  Cast Speed",         pl_casts/100, pl_casts%100, ""       ); break;
			case 10: if (pl_reflc>0)
					 meta_stat(1,n,1,"Thorns Score",         pl_reflc,     -1,           ""       ); break;
			case 11: if (pl.skill[34][0])
					 meta_stat(1,n,4,"Mana Cost Multiplier", sk_conce/100, sk_conce%100, "%"      ); break;
			case 12: if (pl_aoebn)
					 meta_stat(1,n,1,"Total AoE Bonus",      pl_aoebn,     -1,           "Tiles"  ); break;
			//
			case 16: meta_stat(1,n,9,"  Active Stats:",      -1,           -1,           ""       ); break;
			case 17: if (pl.skill[40][0])
					 meta_stat(2,n,5,"Cleave Hit Damage",    sk_cleav,     -1,           ""       ); break;
			case 18: if (pl.skill[40][0] && !(pl_flags&(1<<8)))
					 meta_stat(2,n,5,"Cleave Bleed Degen",   sk_bleed/100, sk_bleed%100, "/s"     ); break;
			case 19: if (pl.skill[40][0])
					 meta_stat(2,n,5,"Cleave Cooldown",      coo_clea/100, coo_clea%100, "Seconds"); break;
			case 20: if (pl.skill[49][0])
					 meta_stat(2,n,1,"Leap Crit Damage",     cri_leap,     -1,           ""       ); break;
			case 21: if (pl.skill[49][0])
					 meta_stat(2,n,1,"Leap Hit Damage",      sk_leapv,     -1,           ""       ); break;
			case 22: if (pl.skill[49][0])
					 meta_stat(2,n,1,"Leap Cooldown",        coo_leap/100, coo_leap%100, "Seconds"); break;
			case 23: if (pl.skill[22][0]) {
					 meta_stat(2,n,5,"Rage TD Bonus",  sk_rage, -1,          "Top Dmg"); } break;
			case 24: if (pl.skill[22][0]) {
					 meta_stat(2,n,5,"Rage DoT Bonus", sk_rage2/100, sk_rage2%100, "%"); } break;
			case 25: if (pl.skill[24][0]) 
					 meta_stat(2,n,4,"Blast Hit Damage",     sk_blast,     -1,           ""       ); break;
			case 26: if (pl.skill[24][0])
					 meta_stat(2,n,4,"Blast Cooldown",       coo_blas/100, coo_blas%100, "Seconds"); break;
			case 27: if (pl.skill[15][0]) 
					 meta_stat(2,n,1,"Lethargy Effect",      sk_letha,     -1,           "I/R Pen"); break;
			case 28: if (pl.skill[42][0])
					 meta_stat(2,n,4,(pl_flagb&(1<<14))?"Venom Degen":"Poison Degen",sk_poiso/100,sk_poiso%100,"/s"); break;
			case 29: if (pl.skill[42][0])
					 meta_stat(2,n,4,(pl_flagb&(1<<14))?"Venom Cooldown":"Poison Cooldown",coo_pois/100,coo_pois%100,"Seconds"); break;
			case 30: if (pl.skill[43][0]) {
					 if (pl_flagb&(1<<6)) { meta_stat(2,n,1,"Pulse Hit Heal",sk_pulse,-1,""); }
					 else                 { meta_stat(2,n,1,"Pulse Hit Damage",sk_pulse,-1,""); } } break;
			case 31: if (pl.skill[43][0])
					 meta_stat(2,n,1,"Pulse Count",          sk_pucnt,     -1,           ""       ); break;
			case 32: if (pl.skill[43][0])
					 meta_stat(2,n,1,"Pulse Cooldown",       coo_puls/100,coo_puls%100,  "Seconds"); break;
			case 33: if (pl.skill[ 7][0])
					 meta_stat(2,n,6,"Zephyr Hit Damage",    sk_razor,     -1,           ""       ); break;
			case 34: if (pl_flagc&(1<<13))
					 meta_stat(2,n,1,"Immolate Degen",       sk_immol/100,sk_immol%100,  "/s"     ); break;
			case 36: if (pl.skill[27][0])
					 meta_stat(2,n,1,"Ghost Comp Potency", 	 sk_ghost,     -1,           ""       ); break;
			case 37: if (pl.skill[27][0])
					 meta_stat(2,n,1,"Ghost Comp Cooldown",  coo_ghos/100, coo_ghos%100, "Seconds"); break;
			case 38: if (pl.skill[46][0])
					 meta_stat(2,n,4,"Shadow Copy Potency",  sk_shado,     -1,           ""       ); break;
			case 39: if (pl.skill[46][0])
					 meta_stat(2,n,4,"Shadow Copy Duration", sk_shadd,     -1,           "Seconds"); break;
			case 40: if (pl.skill[46][0])
					 meta_stat(2,n,4,"Shadow Copy Cooldown", coo_shad/100, coo_shad%100, "Seconds"); break;
			//
			default: break;
		}
	}
	else						// Defense Stats
	{
		n-=7;
		switch (n+skill_pos)
		{
			case  0: meta_stat(1,n,9,"  Passive Stats:",     -1,           -1,           ""       ); break;
			case  1: if (pl_dmgrd!=10000)
					 meta_stat(1,n,1,"Damage Reduction",     pl_dmgrd/100, pl_dmgrd%100, "%"      ); break;
			case  2: if (pl_dmgrd!=10000||(pl_flagc&(1<<9|1<<11|1<<12|1<<14))||(pl_flags&(1<<9)))
					 meta_stat(1,n,7,"Effective Hitpoints",  pl_ehp,       -1,           ""       ); break;
			case  3: meta_stat(1,n,5,"Health Regen Rate",    sk_regen/100, sk_regen%100, "/s"     ); break;
			case  4: meta_stat(1,n,6,"Endurance Regen Rate", sk_restv/100, sk_restv%100, "/s"     ); break;
			case  5: meta_stat(1,n,4,"Mana Regen Rate",      sk_medit/100, sk_medit%100, "/s"     ); break;
			case  6: meta_stat(1,n,1,"Effective Immunity",   sk_immun,     -1,           ""       ); break;
			case  7: meta_stat(1,n,1,"Effective Resistance", sk_resis,     -1,           ""       ); break;
			case  8: meta_stat(1,n,6,"Attack Speed",         pl_atksp/100, pl_atksp%100, ""       ); break;
			case  9: meta_stat(1,n,6,"  Cast Speed",         pl_casts/100, pl_casts%100, ""       ); break;
			case 10: if (pl_reflc>0)
					 meta_stat(1,n,1,"Thorns Score",         pl_reflc,     -1,           ""       ); break;
			case 11: if (pl.skill[34][0])
					 meta_stat(1,n,4,"Mana Cost Multiplier", sk_conce/100, sk_conce%100, "%"      ); break;
			case 12: if (pl_aoebn)
					 meta_stat(1,n,4,"Total AoE Bonus",      pl_aoebn,     -1,           "Tiles"  ); break;
			case 13: meta_stat(1,n,4,"Buffing Apt Bonus",    at_score(AT_WIL)/4, -1,     ""       ); break;
			case 14: meta_stat(2,n,1,"Underwater Degen",     sk_water/100, sk_water%100, "/s"     ); break;
			//
			case 16: meta_stat(1,n,9,"  Active Stats:",      -1,           -1,           ""       ); break;
			case 17: if (pl.skill[21][0])
					 meta_stat(2,n,1,"Bless Effect",         sk_bless,     -1,           "Attribs"); break;
			case 18: if (pl.skill[18][0])
					 meta_stat(2,n,1,"Enhance Effect",       sk_enhan,     -1,           "WV"     ); break;
			case 19: if (pl.skill[17][0])
					 meta_stat(2,n,1,"Protect Effect",       sk_prote,     -1,           "AV"     ); break;
			case 20: if (pl.skill[11][0])
					 meta_stat(2,n,4,(pl_flagb&(1<<10))?"M.Shell Effect":"M.Shield Effect", sk_mshie, -1, (pl_flagb&(1<<10))?"Res&Imm":"AV"); break;
			case 21: if (pl.skill[11][0])
					 meta_stat(2,n,4,(pl_flagb&(1<<10))?"M.Shell Duration":"M.Shield Duration", sk_mdura, -1, "Seconds"); break;
			case 22: if (pl.skill[47][0])
					 meta_stat(2,n,6,"Haste Effect",         sk_haste,     -1,           "Speed"  ); break;
			case 23: if (pl.skill[22][0]) {
					 meta_stat(2,n,5,"Calm TD Taken", sk_calm,    -1,         "Top Dmg"); } break;
			case 24: if (pl.skill[22][0]) {
					 meta_stat(2,n,5,"Calm DoT Taken", sk_calm2/100, sk_calm2%100, "%" ); } break;
			case 25: if (pl.skill[26][0])
					 meta_stat(2,n,1,(pl_flags&(1<<14))?"Regen Effect":"Heal Effect",sk_healr,-1,(pl_flags&(1<<14))?"/s":""); break;
			case 26: if (pl.skill[37][0])  {
					 if (pl_flagb&(5<<11)) { meta_stat(2,n,5,"Douse Effect", sk_blind/100, sk_blind%100, ""); }
					 else                  { meta_stat(2,n,5,"Blind Effect", sk_blind,     -1,           ""); } } break;
			case 27: if (pl.skill[37][0])
					 meta_stat(2,n,5,(pl_flagb&(1<<11))?"Douse Cooldown":"Blind Cooldown", coo_blin/100, coo_blin%100, "Seconds"); break;
			case 28: if (pl.skill[35][0])  {
					 if (pl_flagb&(1<<12)) { meta_stat(2,n,5,"Rally Effect",  sk_rally, -1, "Hit/Par"); }
					 else                  { meta_stat(2,n,5,"Warcry Effect", sk_warcr, -1, "Attribs"); } } break;
			case 29: if (pl.skill[35][0])
					 meta_stat(2,n,1,(pl_flagb&(1<<12))?"Rally Cooldown":"Warcry Cooldown", coo_warc/100, coo_warc%100, "Seconds"); break;
			case 30: if (pl.skill[41][0])
					 meta_stat(2,n,5,(pl_flags&(1<<10))?"Crush Effect":"Weaken Effect", sk_weake, -1, (pl_flags&(1<<10))?"AV":"WV"); break;
			case 31: if (pl.skill[41][0])
					 meta_stat(2,n,5,(pl_flags&(1<<10))?"Crush Cooldown":"Weaken Cooldown", coo_weak/100, coo_weak%100, "Seconds"); break;
			case 32: if (pl.skill[20][0])
					 meta_stat(2,n,1,"Curse Effect",         sk_curse,     -1,           "Attribs"); break;
			case 33: if (pl.skill[20][0])
					 meta_stat(2,n,1,"Curse Cooldown",       coo_curs/100, coo_curs%100, "Seconds"); break;
			case 34: if (pl.skill[19][0])
					 meta_stat(2,n,4,"Slow Effect",          sk_slowv,     -1,           "Speed"  ); break;
			case 35: if (pl.skill[19][0])
					 meta_stat(2,n,4,"Slow Cooldown",        coo_slow/100, coo_slow%100, "Seconds"); break;
			case 36: if (pl.skill[27][0])
					 meta_stat(2,n,1,"Ghost Comp Potency", 	 sk_ghost,     -1,           ""       ); break;
			case 37: if (pl.skill[27][0])
					 meta_stat(2,n,1,"Ghost Comp Cooldown",  coo_ghos/100, coo_ghos%100, "Seconds"); break;
			case 38: if (pl.skill[46][0])
					 meta_stat(2,n,4,"Shadow Copy Potency",  sk_shado,     -1,           ""       ); break;
			case 39: if (pl.skill[46][0])
					 meta_stat(2,n,4,"Shadow Copy Duration", sk_shadd,     -1,           "Seconds"); break;
			case 40: if (pl.skill[46][0])
					 meta_stat(2,n,4,"Shadow Copy Cooldown", coo_shad/100, coo_shad%100, "Seconds"); break;
			//
			default: break;
		}
	}
}

void eng_display_win(int plr_sprite,int init)
{
	int y,n,m,v,pr,hh,xx,yy;
	char *tmp,buf[50];
	int pl_flags, pl_flagb;
	int buffs[MAXBUFFS][2], debuffs[MAXBUFFS][2], bf, df;

	//if (load) dd_xputtext(670,300+MAXTS,1,"%3d%%",load);

	if (init) {
		reset_block();

		// Draw red rectangles around lower right gui component toggles
		if (pl.mode==2)			dd_showbox(GUI_F_COL1,GUI_F_ROW1,45,12,(unsigned short)(RED));
		else if (pl.mode==1)	dd_showbox(GUI_F_COL2,GUI_F_ROW1,45,12,(unsigned short)(RED));
		else if (pl.mode==0)	dd_showbox(GUI_F_COL3,GUI_F_ROW1,45,12,(unsigned short)(RED));
		if (pdata.show_proz)	dd_showbox(GUI_F_COL4,GUI_F_ROW1,45,12,(unsigned short)(RED));
		//
		if (pdata.show_stats)	dd_showbox(GUI_F_COL1,GUI_F_ROW2,45,12,(unsigned short)(RED));
		if (pdata.hide)			dd_showbox(GUI_F_COL2,GUI_F_ROW2,45,12,(unsigned short)(RED));
		if (pdata.show_names)	dd_showbox(GUI_F_COL3,GUI_F_ROW2,45,12,(unsigned short)(RED));
		if (pdata.show_bars)	dd_showbox(GUI_F_COL4,GUI_F_ROW2,45,12,(unsigned short)(RED));
		//
		if (hudmode==0)			dd_showbox(261,182,64,12,(unsigned short)(RED));
		else if (hudmode==1)	dd_showbox(261,197,64,12,(unsigned short)(RED));
		else if (hudmode==2)	dd_showbox(261,212,64,12,(unsigned short)(RED));
		else if (hudmode==3)	dd_showbox(261,182,64,12,(unsigned short)(GREEN));

		// inventory    251  6
		for (n=0; n<30; n++) {
			// Draw inventory items
			if (pl.item[n+inv_pos]) {
				if (hightlight==HL_BACKPACK && hightlight_sub==n+(signed)inv_pos)
				{
					// Draw item sprite
					copyspritex(pl.item[n+inv_pos],261+(n%10)*34,6+(n/10)*34,16);
					// Draw lock icon for locked items
					/*
					if (pl.item_l[n+inv_pos]&1)
						copyspritex(4000,261+(n%10)*34,6+(n/10)*34,16);
					*/
					// Draw soulstone icon 
					if (pl.item_l[n+inv_pos]&2)
						copyspritex(4496,261+(n%10)*34,6+(n/10)*34,16);
					// Draw talisman icon 
					if (pl.item_l[n+inv_pos]&4)
						copyspritex(4497,261+(n%10)*34,6+(n/10)*34,16);
					// Draw corruption icon 
					if (pl.item_l[n+inv_pos]&8)
						copyspritex(6881,261+(n%10)*34,6+(n/10)*34,16);
					// Draw catalyst name
					if (pl.item_p[n+inv_pos])
						copyspritex(6999+pl.item_p[n+inv_pos],261+(n%10)*34,6+(n/10)*34,16);
					// Draw stack count 
					if (pl.item_s[n+inv_pos]>0&&pl.item_s[n+inv_pos]<=10)
						copyspritex(4000+pl.item_s[n+inv_pos],261+(n%10)*34,6+(n/10)*34,16);
				}
				else
				{
					// Draw item sprite
					copyspritex(pl.item[n+inv_pos],261+(n%10)*34,6+(n/10)*34,0);
					// Draw lock icon for locked items
					/*
					if (pl.item_l[n+inv_pos]&1)
						copyspritex(4000,261+(n%10)*34,6+(n/10)*34,0);
					*/
					// Draw soulstone icon 
					if (pl.item_l[n+inv_pos]&2)
						copyspritex(4496,261+(n%10)*34,6+(n/10)*34,0);
					// Draw talisman icon 
					if (pl.item_l[n+inv_pos]&4)
						copyspritex(4497,261+(n%10)*34,6+(n/10)*34,0);
					// Draw corruption icon 
					if (pl.item_l[n+inv_pos]&8)
						copyspritex(6881,261+(n%10)*34,6+(n/10)*34,0);
					// Draw catalyst name
					if (pl.item_p[n+inv_pos])
						copyspritex(6999+pl.item_p[n+inv_pos],261+(n%10)*34,6+(n/10)*34,0);
					// Draw stack count 
					if (pl.item_s[n+inv_pos]>0&&pl.item_s[n+inv_pos]<=10)
						copyspritex(4000+pl.item_s[n+inv_pos],261+(n%10)*34,6+(n/10)*34,0);
				}
			}
			// Draw shortcut key IDs
			for (m=0; m<20; m++) if (pdata.xbutton[m].skill_nr==100+n+(signed)inv_pos)
				copyspritex(4011+m,261+(n%10)*34,6+(n/10)*34,0);
		}

		// Prepare spell icons // pl.spell[n] is the SPRITE of the debuff being received.
		// This can be used to cheat and split buffs and debuffs into two groups.
		// buffs[MAXBUFFS][2], debuffs[MAXBUFFS][2];
		bf = 0;
		df = 0;
		for (n=0; n<MAXBUFFS; n++)
		{
			buffs[n][0] = 0; debuffs[n][0] = 0;
			buffs[n][1] = 0; debuffs[n][1] = 0;
		}
		// Split buffs and debuffs
		for (n=0; n<MAXBUFFS; n++)
		{
			if (m = pl.spell[n]) 
			{
				if (m==   89 || m==   91 || m==   97 || m==  119 || m==  149 ||
					m==  178 || m==  224 || m==  225 || m==  319 || m==  325 ||
					m==  329 || m==  338 || m==  365 || m==  411 || m==  419 ||
					m==  471 || m==  487 || m==  489 || m==  498 || m==  617 ||
					m==  618 || m==  702 || m== 1009 || m== 1015 || m== 3493 ||
					m== 3494 ||(m>= 6721 && m<= 6745)||(m>= 6761 && m<= 6780)||
				   (m>=16860 && m<=16863) )
				{
					debuffs[df][0] = pl.spell[n];
					debuffs[df][1] = pl.active[n];
					df++;
				}
				else
				{
					buffs[bf][0] = pl.spell[n];
					buffs[bf][1] = pl.active[n];
					bf++;
				}
			}
		}
		
		// Scroll Bars for Skills and Inventory
		dd_showbar(234,152+(skill_pos*58)/(MAXSKILL-10)+(skill_pos>25?1:0), 11,11,(unsigned short)GUI_BAR_GRE);
		dd_showbar(601, 36+(inv_pos *  9)/10, 11,13,(unsigned short)GUI_BAR_GRE);

		// display info-texts
		// HP, EN, MA below the skill list
		dd_xputtext(GUI_HP_COUNT_X,	GUI_HP_COUNT_Y,	1,"Hitpoints         %3d %3d",pl.a_hp,pl.hp[5]);
		dd_xputtext(GUI_EN_COUNT_X,	GUI_EN_COUNT_Y,	1,"Endurance         %3d %3d",pl.a_end,pl.end[5]);
		dd_xputtext(GUI_MP_COUNT_X,	GUI_MP_COUNT_Y,	1,"Mana              %3d %3d",pl.a_mana,pl.mana[5]);
		
		// Money, BSP, CST, OSP
			dd_xputtext(GUI_MONEY_X,	GUI_MONEY_Y,	1,"      Money");
		dd_xputtext(GUI_MONEY_X+74,	GUI_MONEY_Y,	1,"%9dG %2dS",pl.gold/100,pl.gold%100);
		if (pl.bs_points==0)
			dd_xputtext(GUI_MONEY_X,	GUI_MONEY_Y+14,	1,"        ???");
		else
			dd_xputtext(GUI_MONEY_X,	GUI_MONEY_Y+14,	1," Stronghold");
		dd_xputtext(GUI_MONEY_X+74,	GUI_MONEY_Y+14,	1,"  %12d",pl.bs_points);
		if (pl.tokens==0)
			dd_xputtext(GUI_MONEY_X,	GUI_MONEY_Y+28,	1,"        ???");
		else
			dd_xputtext(GUI_MONEY_X,	GUI_MONEY_Y+28,	1,"     Casino");
		dd_xputtext(GUI_MONEY_X+74,	GUI_MONEY_Y+28,	1,"  %12d",pl.tokens);
		if (pl.os_points==0)
			dd_xputtext(GUI_MONEY_X,	GUI_MONEY_Y+42,	1,"        ???");
		else
			dd_xputtext(GUI_MONEY_X,	GUI_MONEY_Y+42,	1,"   Contract");
		dd_xputtext(GUI_MONEY_X+74,	GUI_MONEY_Y+42,	1,"  %12d",pl.os_points);
		
		// Update and Update exp
		if (hudmode==0||hudmode==3)
		{
			copyspritex(do_darkmode?18099:99, 134, 3,  0);
			dd_xputtext(GUI_UPDATE_X,	GUI_UPDATE_Y,	1,"Update");
			dd_xputtext(GUI_UPOINTS_X,	GUI_UPOINTS_Y,	1,"%7d",pl.points-stat_points_used);
		}
		if (hudmode==3)
			copyspritex(do_darkmode?18098:18097, 134, 3,  0);
		
		init_meta_stats();
		
		// Display Skill Tree button if applicable - a similar check is required in inter.c
		if (st_skill_pts_all(pl.tree_points)>0)
		{
			if (st_skill_pts_have(pl.tree_points)>0 && show_tree!=1)
				copyspritex(18008, 337, 177, min(15, max(0, abs(8-(GetTickCount()%16))*2)));
			else if (st_skill_pts_have(pl.os_tree)>0 && show_tree!=2)
				copyspritex(18008, 337, 177, min(15, max(0, abs(8-(GetTickCount()%16))*2)));
			copyspritex(do_darkmode?18004:18002, 339, 179,  0);
		}

		// WV, AV, EXP
		dd_xputtext(GUI_WV_X,   GUI_WV_Y,1,   "Weapon Value");
		dd_xputtext(GUI_WV_X+92,GUI_WV_Y,1,   "%11d",pl.weapon);
		dd_xputtext(GUI_WV_X,   GUI_WV_Y+14,1,"Armor Value");
		dd_xputtext(GUI_WV_X+92,GUI_WV_Y+14,1,"%11d",pl_armor);
		dd_xputtext(GUI_WV_X,   GUI_WV_Y+28,1,"Experience");
		dd_xputtext(GUI_WV_X+92,GUI_WV_Y+28,1,"%11d",pl.points_tot);

		// display spell shortcut buttons
		for (n=0; n<20; n++) {
			dd_xputtext(1038+(n%5)*48,600+(n/5)*15,1,pdata.xbutton[n].name);
		}
		
		if (hudmode==0)
		{
			for (n=0; n<5; n++) {
				dd_xputtext(9,8+n*14,1,"%-20.20s",at_name[n]);
				//
				if (pdata.show_stats) dd_xputtext(117,(8+n*14),3,"%3d",pl.attrib[n][0]+stat_raised[n]);
				dd_xputtext(140,(8+n*14),1,"%3d",at_score(n)+stat_raised[n]);
				//
				if (attrib_needed(n,pl.attrib[n][0]+stat_raised[n])<=pl.points-stat_points_used) 
					dd_putc(163,8+n*14,1,'+');
				if (stat_raised[n]>0) 
					dd_putc(177,8+n*14,1,'-');
				if (attrib_needed(n,pl.attrib[n][0]+stat_raised[n])!=HIGH_VAL) 
					dd_xputtext(189,8+n*14,1,"%7d",attrib_needed(n,pl.attrib[n][0]+stat_raised[n]));
			}
			
			dd_xputtext(9,8+5*14,1,"Hitpoints");
			//
			if (pdata.show_stats) dd_xputtext(117,(8+5*14),3,"%3d",pl.hp[0]+stat_raised[5]);
			dd_xputtext(140,(8+5*14),1,"%3d",pl.hp[5]+stat_raised[5]);
			//
			if (hp_needed(pl.hp[0]+stat_raised[5])<=pl.points-stat_points_used)	
				dd_putc(163,8+5*14,1,'+');
			if (stat_raised[5]>0) 
				dd_putc(177,8+5*14,1,'-');
			if (hp_needed(pl.hp[0]+stat_raised[5])!=HIGH_VAL) 
				dd_xputtext(189,8+5*14,1,"%7d",hp_needed(pl.hp[0]+stat_raised[5]));
			
			dd_xputtext(9,8+6*14,1,"Mana");
			//
			if (pdata.show_stats) dd_xputtext(117,(8+6*14),3,"%3d",pl.mana[0]+stat_raised[7]);
			dd_xputtext(140,(8+6*14),1,"%3d",pl.mana[5]+stat_raised[7]);
			//
			if (mana_needed(pl.mana[0]+stat_raised[7])<=pl.points-stat_points_used)	
				dd_putc(163,8+6*14,1,'+');
			if (stat_raised[7]>0) 
				dd_putc(177,8+6*14,1,'-');
			if (mana_needed(pl.mana[0]+stat_raised[7])!=HIGH_VAL) 
				dd_xputtext(189,8+6*14,1,"%7d",mana_needed(pl.mana[0]+stat_raised[7]));
		}
		else
		{
			for (n=0; n<7; n++) 
			{
				show_meta_stats(n);
			}
		}
		
		// Player Flags from special items
		pl_flags = pl.worn[WN_FLAGS];
		pl_flagb = pl.worn_p[WN_FLAGS];
		
		for (n=0; n<10; n++) 
		{
			// regular skill tab functionality
			if (hudmode==0||hudmode==3)
			{
				m=skilltab[n+skill_pos].nr;
				if (m>=55)
				{
					dd_xputtext(9,(8+8*14)+n*14,1,"-");
				}
				else if (m>=50)
				{
					if ((m==52 && !KNOW_IDENTIFY) || ((m==53||m==54) && !IS_LYCANTH)) 
					{
						dd_xputtext(9,(8+8*14)+n*14,1,"-");
					}
					else
					{
						dd_xputtext(9,(8+8*14)+n*14,5,"%-20.20s",skilltab[n+skill_pos].name);
						dd_xputtext(140,(8+8*14)+n*14,5,"%3d",min(300, max(1,(points2rank(pl.points_tot)+1)*8)));
					}
					continue;
				}
				else if (!pl.skill[m][0]) 
				{
					// Stealth, Resist, Regen, Rest, Medit, Immun -- these are active even if you don't know them.
					if (m==8||m==23||m==28||m==29||m==30||m==32||(m==44&&IS_SEYAN_DU)) 
					{
						dd_xputtext(9,(8+8*14)+n*14,0,"%-20.20s",skilltab[n+skill_pos].name);
						dd_xputtext(140,(8+8*14)+n*14,0,"%3d",sk_score(m));
					}
					else
						dd_xputtext(9,(8+8*14)+n*14,1,"-");
					continue;
				}
				if (	(m==11&&(pl_flagb & (1 << 10))) ||	// Magic Shield -> Magic Shell
						(m==19&&(pl_flags & (1 <<  5))) ||	// Slow -> Greater Slow
						(m==20&&(pl_flags & (1 <<  6))) ||	// Curse -> Greater Curse
						(m==24&&(pl_flags & (1 <<  7))) ||	// Blast -> +Scorch
						(m==26&&(pl_flags & (1 << 14))) ||	// Heal -> Regen
						(m==37&&(pl_flagb & (1 << 11))) ||	// Blind -> Douse
						(m==40&&(pl_flags & (1 <<  8))) ||	// Cleave -> +Aggravate
						(m==41&&(pl_flags & (1 << 10))) ||  // Weaken -> Greater Weaken
						(m==16&&(pl_flagb & (1 <<  5))) ||  // Shield -> Shield Bash
						(m==43&&(pl_flagb & (1 <<  6))) ||  // Pulse -> Healing Pulses
						(m==49&&(pl_flagb & (1 <<  7))) ||  // Leap
						(m==35&&(pl_flagb & (1 << 12))) ||  // Warcry -> Rally
						(m==42&&(pl_flagb & (1 << 14))) ||  // Poison -> Venom
						(m==12&&(pl_flagb & (1 <<  3))) ||  // Tactics invert
						(m==22&&IS_SHIFTED) // Rage -> Calm
					)
					dd_xputtext(9,(8+8*14)+n*14,1,"%-20.20s",skilltab[n+skill_pos].alt_a);
				else
					dd_xputtext(9,(8+8*14)+n*14,1,"%-20.20s",skilltab[n+skill_pos].name);
				
				if (pdata.show_stats) dd_xputtext(117,(8+8*14)+n*14,3,"%3d",pl.skill[m][0]+stat_raised[n+8+skill_pos]);
				dd_xputtext(140,(8+8*14)+n*14,1,"%3d",sk_score(m)+stat_raised[n+8+skill_pos]);
				if (skill_needed(m,pl.skill[m][0]+stat_raised[n+8+skill_pos])<=pl.points-stat_points_used) 
					dd_putc(163,(8+8*14)+n*14,1,'+');
				if (stat_raised[n+8+skill_pos]>0) 
					dd_putc(177,(8+8*14)+n*14,1,'-');
				if (skill_needed(m,pl.skill[m][0]+stat_raised[n+8+skill_pos])!=HIGH_VAL)
					dd_xputtext(189,(8+8*14)+n*14,1,"%7d",skill_needed(m,pl.skill[m][0]+stat_raised[n+8+skill_pos]));
			}
			else
			{
				show_meta_stats(7+n);
			}
		}
	}

	// experience bar
	if (pl.points_tot>0 && points2rank(pl.points_tot)<24) 
	{
		n=min(GUI_XPBAR_W,((unsigned int)(pl.points_tot-rank2points(points2rank(pl.points_tot)))
			)*GUI_XPBAR_W/((unsigned int)(pl.points_tot+points_tolevel(pl.points_tot)-rank2points(points2rank(pl.points_tot)))
			));
	}
	else 
	{
		n=0;
	}
	if (points2rank(pl.points_tot)==24)
		dd_showbar(GUI_XPBAR_X,GUI_XPBAR_Y,GUI_XPBAR_W,6,(unsigned short)GUI_BAR_EXP);
	else
	{
		dd_showbar(GUI_XPBAR_X,GUI_XPBAR_Y,GUI_XPBAR_W,6,(unsigned short)GUI_BAR_BLU);
		dd_showbar(GUI_XPBAR_X,GUI_XPBAR_Y,n,          6,(unsigned short)GUI_BAR_EXP);
	}
	//

	// logtext
	if (logtimer) logtimer--;
	else logstart=0;

	for (y=0; y<LL; y++) {
		dd_puttext(GUI_LOG_X,8+y*10,logfont[LL-y-1+logstart],logtext[LL-y-1+logstart]);
	}

	input[in_len]=0;
	if (cur_pos-view_pos>45) view_pos=cur_pos-45;
	if (cur_pos-5<view_pos)	view_pos=max(0,cur_pos-5);
	memcpy(buf,input+view_pos,48);
	buf[48]=0;

	dd_puttext(GUI_LOG_X,13+10*LL,1,buf);
	dd_putc(GUI_LOG_X+6*(cur_pos-view_pos),13+10*LL,1,127);

	if (init) {
		if (show_shop || show_wps || show_tree || show_book || show_motd || show_newp || show_tuto) show_look=0;
		if (!show_look) {
			/*
			for (n=0; n<12; n++) 
			{
				if (pl.worn[wntab[n]]) 
				{
					if (hightlight==HL_EQUIPMENT && hightlight_sub==wntab[n])
						copyspritex(pl.worn[wntab[n]],303+(n%2)*35,2+(n/2)*35,16);
					else
						copyspritex(pl.worn[wntab[n]],303+(n%2)*35,2+(n/2)*35,0);
				}
				if (inv_block[wntab[n]]) copyspritex(4,303+(n%2)*35,2+(n/2)*35,0);
			}
			*/
			
			if (pl.worn[WN_CHARM2])
			{
				copyspritex(do_darkmode?18003:3, 775, 15,  0);
			}
			
			// Show your own gear
			for (n = 0; n < 13; n++)
			{
				if (pl.worn[n])
				{
					if (hightlight==HL_EQUIPMENT && hightlight_sub==n)
					{
						copyspritex(pl.worn[n], gui_equ_x[n]+1, gui_equ_y[n]+1, 16);
						// Draw soulstone icon 
						if (pl.worn_p[n]&PL_SOULSTONED)
							copyspritex(4496, gui_equ_x[n]+1, gui_equ_y[n]+1, 16);
						// Draw talisman icon 
						if (pl.worn_p[n]&PL_ENCHANTED)
							copyspritex(4497, gui_equ_x[n]+1, gui_equ_y[n]+1, 16);
						// Draw corrupt icon 
						if (pl.worn_p[n]&PL_CORRUPTED)
							copyspritex(6881, gui_equ_x[n]+1, gui_equ_y[n]+1, 16);
						// Draw stack count
						if (pl.worn_s[n]>0&&pl.worn_s[n]<=10)
							copyspritex(4000+pl.worn_s[n], gui_equ_x[n]+1, gui_equ_y[n]+1, 16);
					}
					else
					{
						copyspritex(pl.worn[n], gui_equ_x[n]+1, gui_equ_y[n]+1, 0);
						// Draw soulstone icon 
						if (pl.worn_p[n]&PL_SOULSTONED)
							copyspritex(4496, gui_equ_x[n]+1, gui_equ_y[n]+1, 0);
						// Draw talisman icon 
						if (pl.worn_p[n]&PL_ENCHANTED)
							copyspritex(4497, gui_equ_x[n]+1, gui_equ_y[n]+1, 0);
						// Draw corrupt icon 
						if (pl.worn_p[n]&PL_CORRUPTED)
							copyspritex(6881, gui_equ_x[n]+1, gui_equ_y[n]+1, 0);
						// Draw stack count
						if (pl.worn_s[n]>0&&pl.worn_s[n]<=10)
							copyspritex(4000+pl.worn_s[n], gui_equ_x[n]+1, gui_equ_y[n]+1, 0);
					}
					// Draw shortcut key IDs
					for (m=0; m<20; m++) if (pdata.xbutton[m].skill_nr==200+n)
						copyspritex(4011+m,gui_equ_x[n]+1,gui_equ_y[n]+1, 0);
					
					if (inv_block[n] && n==WN_CHARM2)
						copyspritex(4,          gui_equ_x[n]+1, gui_equ_y[n]+1,  0); 
				}
				// Get [X] sprite if we can't equip the slot
				if (inv_block[n] && n!=WN_CHARM2)
						copyspritex(4,          gui_equ_x[n]+1, gui_equ_y[n]+1,  0); 
			}
			//

			if (selected_char) tmp=lookup(selected_char,0);
			else tmp=pl.name;
			dd_xputtext(846+(125-strlen(tmp)*6)/2,32,1,tmp);


			// Bar for HP
			if (pl.hp[5]>0)	n=min(124,pl.hp[5]*62/pl.hp[5]);
			else n=0;
			dd_showbar(GUI_BAR_X,GUI_BAR_HP,n,6,(unsigned short)GUI_BAR_BLU);
			if (pl.hp[5]>0)	n=min(124,pl.a_hp*62/pl.hp[5]);
			else n=0;
			dd_showbar(GUI_BAR_X,GUI_BAR_HP,n,6,(unsigned short)GUI_BAR_GRE);

			// Bar for EN
			if (pl.end[5]>0) n=min(124,pl.end[5]*62/pl.end[5]);
			else n=0;
			dd_showbar(GUI_BAR_X,GUI_BAR_EN,n,6,(unsigned short)GUI_BAR_BLU);
			if (pl.end[5]>0) n=min(124,pl.a_end*62/pl.end[5]);
			else n=0;
			dd_showbar(GUI_BAR_X,GUI_BAR_EN,n,6,(unsigned short)GUI_BAR_GRE);

			// Bar for MP
			if (pl.mana[5]>0) n=min(124,pl.mana[5]*62/pl.mana[5]);
			else n=0;
			dd_showbar(GUI_BAR_X,GUI_BAR_MP,n,6,(unsigned short)GUI_BAR_BLU);
			if (pl.mana[5]>0) n=min(124,pl.a_mana*62/pl.mana[5]);
			else n=0;
			dd_showbar(GUI_BAR_X,GUI_BAR_MP,n,6,(unsigned short)GUI_BAR_GRE);
			
			if (!show_shop) {
				copyspritex(rank_sprite[points2rank(pl.points_tot)],935,42,0);
				copyspritex(plr_sprite,935-61,36,0);
				dd_xputtext(846+(125-strlen(pl.name)*6)/2,157,1,pl.name);
				dd_xputtext(846+(125-strlen(rank[points2rank(pl.points_tot)])*6)/2,176,1,rank[points2rank(pl.points_tot)]);
			}

		} else {
			if (look.worn[WN_CHARM2])
			{
				copyspritex(do_darkmode?18003:3, 775, 15,  0);
			}
			// Look at target gear
			for (n = 0; n < 13; n++)
			{
				if (look.worn[n]) 
				{
					copyspritex(look.worn[n], gui_equ_x[n]+1, gui_equ_y[n]+1, 0);
				}
			}
			
			if (look.sprite) 
			{
				n = 0;
				if (look.extended & 2) n|=128;
				if (look.extended & 4) n|=256;
				copyspritex(look.sprite,935-61,36,n);
			}

			dd_xputtext(846+(125-strlen(rank[points2rank(look.points)])*6)/2,176,1,rank[points2rank(look.points)]);
			dd_xputtext(846+(125-strlen(look.name)*6)/2,157,1,look.name);
			
			// Bar for HP
			if (pl.hp[5]) n=min(124,look.hp*62/pl.hp[5]);
			else n=0;
			dd_showbar(GUI_BAR_X,GUI_BAR_HP,n,6,(unsigned short)GUI_BAR_BLU);
			if (pl.hp[5]) n=min(124,look.a_hp*62/pl.hp[5]);
			else n=0;
			dd_showbar(GUI_BAR_X,GUI_BAR_HP,n,6,(unsigned short)GUI_BAR_RED);
			
			// Bar for EN
			if (pl.end[5]) n=min(124,look.end*62/pl.end[5]);
			else n=0;
			dd_showbar(GUI_BAR_X,GUI_BAR_EN,n,6,(unsigned short)GUI_BAR_BLU);
			if (pl.end[5]) n=min(124,look.a_end*62/pl.end[5]);
			else n=0;
			dd_showbar(GUI_BAR_X,GUI_BAR_EN,n,6,(unsigned short)GUI_BAR_RED);

			// Bar for MP
			if (pl.mana[5])	n=min(124,look.mana*62/pl.mana[5]);
			else n=0;
			dd_showbar(GUI_BAR_X,GUI_BAR_MP,n,6,(unsigned short)GUI_BAR_BLU);
			if (pl.mana[5])	n=min(124,look.a_mana*62/pl.mana[5]);
			else n=0;
			dd_showbar(GUI_BAR_X,GUI_BAR_MP,n,6,(unsigned short)GUI_BAR_RED);

			copyspritex(rank_sprite[points2rank(look.points)],935,42,0);
		}
		
		// Draw buffs and debuffs
		for (n=0; n<MAXBUFFS; n++)
		{
			if (buffs[n][0])
				copyspritex(  buffs[n][0], 848+(n/5)*20,      8+(n%5)*23, 15-min(15,  buffs[n][1]));
			if (debuffs[n][0])
				copyspritex(debuffs[n][0], 848+5*20-(n/5)*20, 8+(n%5)*23, 15-min(15,debuffs[n][1]));
		}
		
		if (show_shop==111) // New Depot
		{
			copyspritex(do_darkmode?18094:18093,GUI_SHOP_X,GUI_SHOP_Y,0); // GUI element
			for (n=0; n<64; n++)
			{
				if (!shop.depot[dept_page][n]) continue;
				
				hh = 0;
				xx = GUI_SHOP_X+6+(n%8)*34;
				yy = GUI_SHOP_Y+6+(n/8)*34;
				
				if (!pl.citem && hightlight==HL_SHOP && (hightlight_sub%64)==n) hh = 16;
				
				copyspritex(shop.depot[dept_page][n],xx,yy,hh);												// Draw Item
				if (shop.depot_f[dept_page][n] & 1) copyspritex(4496,xx,yy,hh); 							// Draw SS
				if (shop.depot_f[dept_page][n] & 2) copyspritex(4497,xx,yy,hh); 							// Draw EN
				if (shop.depot_f[dept_page][n] & 4) copyspritex(6881,xx,yy,hh); 							// Draw CR
				if (shop.depot_c[dept_page][n])		copyspritex(6999+shop.depot_c[dept_page][n],xx,yy,hh);	// Draw CA
				if (shop.depot_s[dept_page][n]>0&&shop.depot_s[dept_page][n]<=10)							// Draw Stack
					copyspritex(4000+shop.depot_s[dept_page][n],xx,yy,hh);
			}
			if (shop.sprite) copyspritex(shop.sprite,935-61,36,0);
			copyspritex(rank_sprite[points2rank(shop.points)],935,42,0);
			dd_xputtext(846+(125-strlen(rank[points2rank(shop.points)])*6)/2,176,1,rank[points2rank(shop.points)]);
			dd_xputtext(846+(125-strlen(shop.name)*6)/2,157,1,shop.name);
			
			// Depot Page Buttons
			xx = GUI_SHOP_X+7 + (dept_page%4)*68;
			yy = GUI_SHOP_Y+281 + (dept_page/4)*17;
			dd_showbox(xx,yy,63,12,(unsigned short)(GREEN));
		}
		else if (show_shop) 
		{
			copyspritex(do_darkmode?18092:92,GUI_SHOP_X,GUI_SHOP_Y,0); // GUI element
			y=0;
			for (n=0; n<62; n++) 
			{
				if (!shop.item[n]) continue;
				
				hh = 0;
				xx = GUI_SHOP_X+2+(n%8)*35;
				yy = GUI_SHOP_Y+2+(n/8)*35;
				
				if ((shop.price[n] - (shop.price[n] & (1<<30)) - (shop.price[n] & (1<<31))) > 0) y=1;
				if (!pl.citem && hightlight==HL_SHOP && hightlight_sub==n) hh = 16;
				
				copyspritex(shop.item[n],xx,yy,hh);
				if (shop.price[n] & (1<<30))       copyspritex(4496,xx,yy,hh); 
				if (shop.price[n] & (1<<31))       copyspritex(4497,xx,yy,hh);
				if (shop.item_p[n]) copyspritex(6999+shop.item_p[n],xx,yy,hh);
				
				if (hh && (pr = (shop.price[n] - (shop.price[n]&(1<<30)) - (shop.price[n]&(1<<31)))))
				{
					if (show_shop>=1 && show_shop<=101) // Normal shop
					{
						if ((GetAsyncKeyState(VK_CONTROL)&0x8000)||(GetAsyncKeyState(VK_MENU)&0x8000)) 
						{
							pr*=10;
							dd_xputtext(GUI_SHOP_X+7,GUI_SHOP_Y+300,1,"Buy 10 for: %9dG %2dS",pr/100,pr%100);
						}
						else
						{
							dd_xputtext(GUI_SHOP_X+7,GUI_SHOP_Y+300,1,"   Buy for: %9dG %2dS",pr/100,pr%100);
						}
					}
					if (show_shop==102) // Black Stronghold
						dd_xputtext(GUI_SHOP_X+7,GUI_SHOP_Y+300,1,"    Take reward for: %9d Stronghold Pts",pr);
					if (show_shop==103) // Casino
						dd_xputtext(GUI_SHOP_X+7,GUI_SHOP_Y+300,1,"Take reward for: %9d Tokens",pr);
					if (show_shop==104) // Contract
						dd_xputtext(GUI_SHOP_X+7,GUI_SHOP_Y+300,1,"  Take reward for: %9d Contract Pts",pr);
					if (show_shop==105) // Exp
						dd_xputtext(GUI_SHOP_X+7,GUI_SHOP_Y+300,1,"         Buy for: %9d Exp",pr);
				}
			}
			if (pl.citem && shop.pl_price)
			{
				if (show_shop>=1 && show_shop<=101)
					dd_xputtext(GUI_SHOP_X+7,GUI_SHOP_Y+300,1,"  Sell for: %9dG %2dS",shop.pl_price/100,shop.pl_price%100);
			}
			/*  Sadly this is harder to do than it looks
			else if ((hightlight==HL_BACKPACK && hightlight_sub==n+(signed)inv_pos) && shop.pl_price)
			{
				if (show_shop>=1 && show_shop<=101)
					dd_xputtext(GUI_SHOP_X+7,GUI_SHOP_Y+300,1,"  Sell for: %9dG %2dS",shop.pl_price/100,shop.pl_price%100);
			}
			*/
			if (y)
			{
				if (show_shop>=1 && show_shop<=101)
					dd_xputtext(GUI_SHOP_X+7,GUI_SHOP_Y+287,1,"Your Money: %9dG %2dS",pl.gold/100,pl.gold%100);
				if (show_shop==102) // Black Stronghold
					dd_xputtext(GUI_SHOP_X+7,GUI_SHOP_Y+287,1,"Your Stronghold Pts: %9d",pl.bs_points);
				if (show_shop==103) // Casino
					dd_xputtext(GUI_SHOP_X+7,GUI_SHOP_Y+287,1,"    Your Tokens: %9d",pl.tokens);
				if (show_shop==104) // Contract
					dd_xputtext(GUI_SHOP_X+7,GUI_SHOP_Y+287,1,"Your Contract Pts: %9d",pl.os_points);
				if (show_shop==105) // Exp
					dd_xputtext(GUI_SHOP_X+7,GUI_SHOP_Y+287,1,"Your Unspent Exp: %9d",pl.points);
			}

			if (shop.sprite) copyspritex(shop.sprite,935-61,36,0);
			copyspritex(rank_sprite[points2rank(shop.points)],935,42,0);
			dd_xputtext(846+(125-strlen(rank[points2rank(shop.points)])*6)/2,176,1,rank[points2rank(shop.points)]);
			dd_xputtext(846+(125-strlen(shop.name)*6)/2,157,1,shop.name);
		}
		
		if (show_wps)
		{
			copyspritex(do_darkmode?18005:5,GUI_SHOP_X,GUI_SHOP_Y,0); // GUI element
			//wps_pos
			for (n=0; n<8; n++) {
				m=n+wps_pos;
				pr = wpslist[m].nr;
				if (hightlight==HL_WAYPOINT && hightlight_sub==n)
					copyspritex((pl.waypoints&(1<<pr))?4500+pr:4533+pr,GUI_SHOP_X+2,GUI_SHOP_Y+2+n*35,16);
				else
					copyspritex((pl.waypoints&(1<<pr))?4500+pr:4533+pr,GUI_SHOP_X+2,GUI_SHOP_Y+2+n*35, 0);
				if (pl.waypoints&(1<<pr))
				{
					dd_xputtext(GUI_SHOP_X+74,GUI_SHOP_Y+ 7+n*35,1,wpslist[m].name);
					dd_xputtext(GUI_SHOP_X+74,GUI_SHOP_Y+18+n*35,1,wpslist[m].desc);
				}
				else
				{
					dd_xputtext(GUI_SHOP_X+74,GUI_SHOP_Y+ 7+n*35,1,"Unknown Location");
					dd_xputtext(GUI_SHOP_X+74,GUI_SHOP_Y+18+n*35,1,"\"???\"");
				}
			}
			// Scroll bar
			dd_showbar(GUI_SHOP_X+269, GUI_SHOP_Y+36+(wps_pos*176)/(MAXWPS-8),11,33,(unsigned short)GUI_BAR_GRE);
		}
		
		if (show_tree)
		{
			copyspritex(do_darkmode?18007:18006,GUI_SHOP_X,GUI_SHOP_Y,0); // GUI element HL_SKTREE
			if      (show_tree==2)   m = 9;
			else if (IS_SEYAN_DU)    m = 0;
			else if (IS_ARCHTEMPLAR) m = 1;
			else if (IS_SKALD)       m = 2;
			else if (IS_WARRIOR)     m = 3;
			else if (IS_SORCERER)    m = 4;
			else if (IS_SUMMONER)    m = 5;
			else if (IS_ARCHHARAKIM) m = 6;
			else if (IS_BRAVER)      m = 7;
			else                     m = 8;
			for (n=0; n<12; n++) {
				if ((m<9 && st_learned_skill(pl.tree_points, n+1)) || (m==9 && st_learned_skill(pl.os_tree, n+1)))
				{
					switch (n)
					{
						case  3: case 11: 
							copyspritex(18111,GUI_SHOP_X+sk_icon[n].x+ST_OFFSET_HL,GUI_SHOP_Y+sk_icon[n].y+ST_OFFSET_HL, 0); break;
						case  0: case  4: 
							copyspritex(18112,GUI_SHOP_X+sk_icon[n].x+ST_OFFSET_HL,GUI_SHOP_Y+sk_icon[n].y+ST_OFFSET_HL, 0); break;
						case  5: case  6: 
							copyspritex(18113,GUI_SHOP_X+sk_icon[n].x+ST_OFFSET_HL,GUI_SHOP_Y+sk_icon[n].y+ST_OFFSET_HL, 0); break;
						case  1: case  7: 
							copyspritex(18114,GUI_SHOP_X+sk_icon[n].x+ST_OFFSET_HL,GUI_SHOP_Y+sk_icon[n].y+ST_OFFSET_HL, 0); break;
						case  8: case  9: 
							copyspritex(18115,GUI_SHOP_X+sk_icon[n].x+ST_OFFSET_HL,GUI_SHOP_Y+sk_icon[n].y+ST_OFFSET_HL, 0); break;
						default: 
							copyspritex(18116,GUI_SHOP_X+sk_icon[n].x+ST_OFFSET_HL,GUI_SHOP_Y+sk_icon[n].y+ST_OFFSET_HL, 0); break;
					}
				}
				if (m<9 && (v = pl.tree_node[n]))
				{
					if (hightlight==HL_SKTREE && hightlight_sub==n)
					{
						copyspritex(sk_corrupt[v-1].icon,GUI_SHOP_X+sk_icon[n].x+ST_OFFSET_IC,GUI_SHOP_Y+sk_icon[n].y+ST_OFFSET_IC,16);
						copyspritex(7067,GUI_SHOP_X+sk_icon[n].x+ST_OFFSET_IC,GUI_SHOP_Y+sk_icon[n].y+ST_OFFSET_IC, 16);
					}
					else
					{
						copyspritex(sk_corrupt[v-1].icon,GUI_SHOP_X+sk_icon[n].x+ST_OFFSET_IC,GUI_SHOP_Y+sk_icon[n].y+ST_OFFSET_IC, 0);
						copyspritex(7067,GUI_SHOP_X+sk_icon[n].x+ST_OFFSET_IC,GUI_SHOP_Y+sk_icon[n].y+ST_OFFSET_IC, 0);
					}
				}
				else
				{
					if (hightlight==HL_SKTREE && hightlight_sub==n)
						copyspritex(sk_tree[m][n].icon,GUI_SHOP_X+sk_icon[n].x+ST_OFFSET_IC,GUI_SHOP_Y+sk_icon[n].y+ST_OFFSET_IC,16);
					else
						copyspritex(sk_tree[m][n].icon,GUI_SHOP_X+sk_icon[n].x+ST_OFFSET_IC,GUI_SHOP_Y+sk_icon[n].y+ST_OFFSET_IC, 0);
				}
			}
			if (m<9)
			{
				copyspritex(6600,GUI_SHOP_X+140+ST_OFFSET_IC,GUI_SHOP_Y+140+ST_OFFSET_IC, 0);
				dd_xputtext(GUI_SHOP_X+260,GUI_SHOP_Y+270,1,"%d/%d",st_skill_pts_have(pl.tree_points),st_skill_pts_all(pl.tree_points));
			}
			else
			{
				copyspritex(7066,GUI_SHOP_X+140+ST_OFFSET_IC,GUI_SHOP_Y+140+ST_OFFSET_IC, 0);
				dd_xputtext(GUI_SHOP_X+260,GUI_SHOP_Y+270,1,"%d/%d",st_skill_pts_have(pl.os_tree),st_skill_pts_all(pl.os_tree));
			}
			if (hightlight==HL_SKTREE)
			{
				if (m<9 && (v = pl.tree_node[hightlight_sub]))
				{
					dd_xputtext(GUI_SHOP_X+7,GUI_SHOP_Y+272,1,sk_corrupt[v-1].name);
					dd_xputtext(GUI_SHOP_X+7,GUI_SHOP_Y+287,1,sk_corrupt[v-1].dsc1);
					dd_xputtext(GUI_SHOP_X+7,GUI_SHOP_Y+300,1,sk_corrupt[v-1].dsc2);
				}
				else
				{
					dd_xputtext(GUI_SHOP_X+7,GUI_SHOP_Y+272,1,sk_tree[m][hightlight_sub].name);
					dd_xputtext(GUI_SHOP_X+7,GUI_SHOP_Y+287,1,sk_tree[m][hightlight_sub].dsc1);
					dd_xputtext(GUI_SHOP_X+7,GUI_SHOP_Y+300,1,sk_tree[m][hightlight_sub].dsc2);
				}
			}
		}
		
		if (show_book)
		{
			copyspritex(do_darkmode?18046:18045,GUI_SHOP_X,GUI_SHOP_Y,0); // GUI element
			for (y=0+MLL/3*(tuto_page-1); y<MLL/3*tuto_page; y++) {
				dd_puttext(GUI_SHOP_X+10,GUI_SHOP_Y+10+y*15,
					motdfont[y],motdtext[y]);
			}
		}
		
		if (show_motd)
		{
			copyspritex(do_darkmode?18042:42,GUI_SHOP_X,GUI_SHOP_Y,0); // GUI element
			for (y=0; y<MLL; y++) {
				dd_puttext(GUI_SHOP_X+10,GUI_SHOP_Y+10+y*10,
					motdfont[y],motdtext[y]);
			}
		}
		
		if (show_newp)
		{
			copyspritex(do_darkmode?18043:43,GUI_SHOP_X,GUI_SHOP_Y,0); // GUI element
			for (y=0; y<MLL; y++) {
				dd_puttext(GUI_SHOP_X+10,GUI_SHOP_Y+10+y*10,
					motdfont[y],motdtext[y]);
			}
		}
		
		if (show_tuto)
		{
			copyspritex(do_darkmode?18044:44,GUI_SHOP_X,GUI_SHOP_Y,0); // GUI element
			copyspritex(tutorial_image[show_tuto-1][tuto_page-1],GUI_SHOP_X+6,GUI_SHOP_Y+145,0);
			for (y=0;y<12;y++)
			{
				dd_puttext(GUI_SHOP_X+10,GUI_SHOP_Y+10+y*10,
					1,tutorial_text[show_tuto-1][tuto_page-1][y]);
			}
		}
	}
}

struct looks {
	char known;
	char name[21];
	char proz;
	unsigned short id;
};

struct looks *looks=NULL;
int lookmax=0;
int lookat=0;

char *lookup(int nr,unsigned short id)
{
	static char buf[40];
	int n;

	if (nr>=lookmax) {
		looks=realloc(looks,sizeof(struct looks)*(nr+10));
		for (n=lookmax; n<nr+10; n++) {
			strcpy(looks[n].name,"");
			looks[n].known=0;
			looks[n].proz=0;
		}
		lookmax=nr+10;
	}

	if (id && id!=looks[nr].id) {
		looks[nr].known=0;
		looks[nr].name[0]=0;
		looks[nr].proz=0;
		looks[nr].id=id;
	}

	if (!looks[nr].known) lookat=nr;

	if (!id) return looks[nr].name;

	if (pdata.show_names && pdata.show_proz) 
	{
		if (looks[nr].proz) 
		{
			sprintf(buf,"%s %d%%",looks[nr].name,looks[nr].proz);
			return buf;
		} 
		else return looks[nr].name;
	} 
	else if (pdata.show_names) return looks[nr].name;
	else if (pdata.show_proz) 
	{
		if (looks[nr].proz) 
		{
			sprintf(buf,"%d%%",looks[nr].proz);
			return buf;
		} 
		else return "";
	} 
	else return "";
}

void add_look(unsigned short nr,char *name,unsigned short id)
{
	int n;

	if (nr>=lookmax) {
		looks=realloc(looks,sizeof(struct looks)*(nr+10));
		for (n=lookmax; n<nr+10; n++) {
			strcpy(looks[n].name,"");
			looks[n].known=0;
			looks[n].proz=0;
		}
		lookmax=nr+10;
	}

	if (id!=looks[nr].id) {
		looks[nr].known=0;
		looks[nr].name[0]=0;
		looks[nr].proz=0;
	}

	strncpy(looks[nr].name,name,16);
	looks[nr].name[16]=0;
	looks[nr].known=1;
	looks[nr].proz=0;
	looks[nr].id=id;
}

void set_look_proz(unsigned short nr,unsigned short id,int proz)
{
	int n;

	if (nr>=lookmax) {
		looks=realloc(looks,sizeof(struct looks)*(nr+10));
		for (n=lookmax; n<nr+10; n++) {
			strcpy(looks[n].name,"");
			looks[n].known=0;
		}
		lookmax=nr+10;
	}
	if (id!=looks[nr].id) {
		looks[nr].known=0;
		looks[nr].name[0]=0;
		looks[nr].proz=0;
		looks[nr].id=id;
	}
	looks[nr].proz=(unsigned char)proz;
}

int tile_x=-1,tile_y=-1,tile_type=-1;

void dd_show_map(unsigned short *src,int xo,int yo,int magnify);

int autohide(int x,int y)
{
	if (x>=(screen_renderdist/2) || (y<=screen_renderdist/2)) return 0;
	return 1;
}

int facing(int x,int y,int dir)
{
	if (dir==1 && x==screen_renderdist/2+1 && y==screen_renderdist/2) return 1;
	if (dir==2 && x==screen_renderdist/2-1 && y==screen_renderdist/2) return 1;
	if (dir==4 && x==screen_renderdist/2 && y==screen_renderdist/2+1) return 1;
	if (dir==3 && x==screen_renderdist/2 && y==screen_renderdist/2-1) return 1;

	return 0;
}

int mapxy_rand(int x,int y,int dur)
{
    int val,tim;

    val=(x*43+y*77+x*y+x*24+y*39)%666;
    tim=(ticker/dur)%666;

    if (tim==val) return ticker%dur;
    else return 0;
}

void display_floortile(int tile,int light,int x,int y,int xoff,int yoff,int mx,int my)
{
    switch(tile) {
	case 16980:	tile+=mapxy_rand(mx,my,10)/2; break;
    }
    copysprite(tile,light,x,y,xoff,yoff);
}

unsigned short ymap[MAPX_MAX*MAPY_MAX];
unsigned short xmap[MAPX_MAX*MAPY_MAX];

void eng_display(int init)	// optimize me!!!!!
{
	int x,y,rx,ry,m,plr_sprite,tmp,mapx,mapy,selected_visible=0,alpha,alphastr,txtclr;
	extern int dd_cache_hit,dd_cache_miss,swap,MAXCACHE;
	static xm_flag=1;
	int inj;

	if (xm_flag) {
		for (m=0; m<MAPX_MAX*MAPY_MAX; m++)	{ xmap[m]=0; ymap[m]=0; }
		xm_flag=0;
	}

	// check if we're visible. If not, just leave.
	if (!dd_isvisible()) return;

	mouse(mx,my,0);
	SetCursor(cursor[cursor_type]);

	// *******
	// * map *
	// *******

	if (init) {
		if (do_shadow) dd_shadow_clear();
		xoff=-map[(screen_renderdist/2)+(screen_renderdist/2)*screen_renderdist].obj_xoff-176; //-176;
		yoff=-map[(screen_renderdist/2)+(screen_renderdist/2)*screen_renderdist].obj_yoff; //-176;
		plr_sprite=map[(screen_renderdist/2)+(screen_renderdist/2)*screen_renderdist].obj2;

		mapx=map[(screen_renderdist/2)+(screen_renderdist/2)*screen_renderdist].x;
		mapy=map[(screen_renderdist/2)+(screen_renderdist/2)*screen_renderdist].y;

		for (y=screen_renderdist-1; y>=0; y--) {
			for (x=0; x<screen_renderdist; x++) {

				// background
				m=x+y*screen_renderdist;

				if (hightlight==HL_MAP && tile_type==0 && tile_x==x && tile_y==y) tmp=16;
				else tmp=0;
				if (map[m].flags&INVIS)	tmp|=64;
				if (map[m].flags&INFRARED) tmp|=256;
				if (map[m].flags&UWATER) tmp|=512;

				display_floortile(map[m].back,map[m].light|tmp,x*32,y*32,xoff,yoff,map[x+y*screen_renderdist].x,map[x+y*screen_renderdist].y);

				if (map[m].x<MAPX_MAX && map[m].y<MAPY_MAX && !(map[m].flags&INVIS)) 
				{
					if (!xmap[map[m].y+map[m].x*MAPX_MAX] || xmap[map[m].y+map[m].x*MAPX_MAX]==0xffff 
						|| ymap[map[m].y+map[m].x*MAPX_MAX]==1)
					{
						xmap[map[m].y+map[m].x*MAPX_MAX]=(unsigned short)get_avgcol(map[m].back);
						ymap[map[m].y+map[m].x*MAPX_MAX]=0;
					}
				}

				if (pl.goto_x==map[m].x && pl.goto_y==map[m].y)
					copysprite(31,0,x*32,y*32,xoff,yoff);
			}
		}

		for (y=screen_renderdist-1; y>=0; y--) {
			for (x=0; x<screen_renderdist; x++) {

				if (	(y <= 5 && x >= screen_renderdist-9) 
					|| 	(y <= 6 && x >= screen_renderdist-8)
					|| 	(y <= 7 && x >= screen_renderdist-7)
					|| 	(y <= 8 && x >= screen_renderdist-6)
					|| 	(y <= 9 && x >= screen_renderdist-5)) 
				continue; // x+, y-

				m=x+y*screen_renderdist;

				if (map[x+y*screen_renderdist].flags&INVIS) continue; //tmp=128;
				else tmp=0;

				if (map[m].flags&INFRARED) tmp|=256;
				if (map[m].flags&UWATER) tmp|=512;

				// object
                if (pdata.hide==0 || (map[m].flags&ISITEM) || autohide(x,y)) 
				{
					int tmp2;

					if (map[m].obj1>=16350 && map[m].obj1< 16422 && map[m].obj1!=16357 &&
						map[m].obj1!=16365 && map[m].obj1!=16373 && map[m].obj1!=16381 &&
						map[m].obj1!=16357 && map[m].obj1!=16389 && map[m].obj1!=16397 &&
						map[m].obj1!=16405 && map[m].obj1!=16413 && map[m].obj1!=16421 &&
						!facing(x,y,pl.dir) && !autohide(x,y) && pdata.hide) { // mine hack

						if (map[m].obj1<16358) tmp2=457;
						else if (map[m].obj1<16366)	tmp2=456;
						else if (map[m].obj1<16374)	tmp2=455;
						else if (map[m].obj1<16382)	tmp2=466;
						else if (map[m].obj1<16390)	tmp2=459;
						else if (map[m].obj1<16398)	tmp2=458;
						else if (map[m].obj1<16398)	tmp2=449;
						else if (map[m].obj1<16406)	tmp2=468;
						else tmp2=467;

						if (hightlight==HL_MAP && tile_type==1 && tile_x==x && tile_y==y) 
							copysprite(tmp2,map[m].light|16|tmp,x*32,y*32,xoff,yoff);
						else 
							copysprite(tmp2,map[m].light|tmp,x*32,y*32,xoff,yoff);
					} 
					
					else if (map[m].obj1==598 && (	// waypoint floor hacks - set by server build.c
						(pl.waypoints&(1<< 0) && map[m].back==1002) || (pl.waypoints&(1<< 1) && map[m].back==1008) ||
						(pl.waypoints&(1<< 2) && map[m].back==1013) || (pl.waypoints&(1<< 3) && map[m].back==1034) ||
						(pl.waypoints&(1<< 4) && map[m].back==1010) || (pl.waypoints&(1<< 5) && map[m].back==1052) ||
						(pl.waypoints&(1<< 6) && map[m].back==1100) || (pl.waypoints&(1<< 7) && map[m].back==1099) ||
						(pl.waypoints&(1<< 8) && map[m].back==1012) || (pl.waypoints&(1<< 9) && map[m].back==1109) ||
						(pl.waypoints&(1<<10) && map[m].back==1118) || (pl.waypoints&(1<<11) && map[m].back==1141) ||
						(pl.waypoints&(1<<12) && map[m].back==1158) || (pl.waypoints&(1<<13) && map[m].back==1145) ||
						(pl.waypoints&(1<<14) && map[m].back==1014) || (pl.waypoints&(1<<15) && map[m].back==1005) ||
						(pl.waypoints&(1<<16) && map[m].back==1006) || (pl.waypoints&(1<<17) && map[m].back==1007) ||
						(pl.waypoints&(1<<18) && map[m].back== 402) || (pl.waypoints&(1<<19) && map[m].back== 500) ||
						(pl.waypoints&(1<<20) && map[m].back== 520) || (pl.waypoints&(1<<21) && map[m].back== 531) ||
						(pl.waypoints&(1<<22) && map[m].back== 542) || (pl.waypoints&(1<<23) && map[m].back== 551) ||
						(pl.waypoints&(1<<24) && map[m].back== 558) || (pl.waypoints&(1<<25) && map[m].back== 543) ||
						(pl.waypoints&(1<<26) && map[m].back== 544) || (pl.waypoints&(1<<27) && map[m].back== 545) ||
						(pl.waypoints&(1<<28) && map[m].back== 546) || (pl.waypoints&(1<<29) && map[m].back== 547) ||
						(pl.waypoints&(1<<30) && map[m].back== 548) || (pl.waypoints&(1<<31) && map[m].back== 549) ))
					{
						// display waypoints as "lit" if you have that flag.
						if (hightlight==HL_MAP && tile_type==1 && tile_x==x && tile_y==y) 
							copysprite(599,map[m].light|16|tmp,x*32,y*32,xoff,yoff);
						else 
							copysprite(599,map[m].light|tmp,x*32,y*32,xoff,yoff);
					}
					else
					{
						if (hightlight==HL_MAP && tile_type==1 && tile_x==x && tile_y==y) 
							copysprite(map[m].obj1,map[m].light|16|tmp,x*32,y*32,xoff,yoff);
						else 
							copysprite(map[m].obj1,map[m].light|tmp,x*32,y*32,xoff,yoff);
					}					

				} else if (map[m].obj1) {					
					copysprite(map[m].obj1+1,map[m].light|tmp,x*32,y*32,xoff,yoff);					
				}

				if (map[m].obj1 && map[m].x<MAPX_MAX && map[m].y<MAPY_MAX) {
					xmap[map[m].y+map[m].x*MAPX_MAX]=(unsigned short)get_avgcol(map[m].obj1);
				}

				// character
				if (tile_type==2 && tile_x==x && tile_y==y)	tmp=16;
				else tmp=0;
				if (map[m].ch_nr==selected_char) {
					tmp|=32; selected_visible=1;
				}
				if (map[m].flags&INVIS)	tmp|=64;
				if (map[m].flags&STONED) tmp|=128;
				if (map[m].flags&INFRARED) tmp|=256;
				if (map[m].flags&UWATER) tmp|=512;
				if (map[m].flags&BLOODY) tmp|=256;

				if (do_shadow) dd_shadow(map[m].obj2,x*32,y*32,xoff+map[m].obj_xoff,yoff+map[m].obj_yoff+4);
				copysprite(map[m].obj2,map[m].light|tmp,x*32,y*32,xoff+map[m].obj_xoff,yoff+map[m].obj_yoff);				

				if (pl.attack_cn && pl.attack_cn==map[m].ch_nr)
					copysprite(34,0,x*32,y*32,xoff+map[m].obj_xoff,yoff+map[m].obj_yoff);

				if (pl.misc_action==DR_GIVE && pl.misc_target1==map[m].ch_id)
					copysprite(45,0,x*32,y*32,xoff+map[m].obj_xoff,yoff+map[m].obj_yoff);
				
				if ((pdata.show_names|pdata.show_proz) && map[m].ch_nr) 
				{
					set_look_proz(map[m].ch_nr,map[m].ch_id,map[m].ch_proz);
					// Colors 1 and 0 are flip-flopped in case the color can't be pulled from the server.
					if (map[m].ch_fontcolor==0)
						txtclr = 1;
					else if (map[m].ch_fontcolor==1)
						txtclr = 0;
					else
						txtclr = map[m].ch_fontcolor;
					dd_gputtext(x*32,y*32,txtclr,lookup(map[m].ch_nr,map[m].ch_id),xoff+map[m].obj_xoff,yoff+map[m].obj_yoff-8);
				}
				
				// Healthbar over characters
				if (pdata.show_bars && map[m].ch_nr) 
				{
					set_look_proz(map[m].ch_nr,map[m].ch_id,map[m].ch_proz);
					rx=((x*32)/2)+((y*32)/2)+32-HPBAR_WIDTH/2+screen_tilexoff-((screen_renderdist-34)/2*32)+xoff+map[m].obj_xoff;
					ry=((x*32)/4)-((y*32)/4)+screen_tileyoff-60+yoff+map[m].obj_yoff;
					
					if (looks[map[m].ch_nr].proz) 
					{
						dd_showbar(rx-1,ry-1,HPBAR_WIDTH+2,4,(unsigned short)CHAR_BAR_BL);
						dd_showbar(rx,ry,HPBAR_WIDTH,2,(unsigned short)CHAR_BAR_RD);
					}
					dd_showbar(rx,ry,(int)(HPBAR_WIDTH*((float)looks[map[m].ch_nr].proz/100.0)),2,(unsigned short)CHAR_BAR_HP);
				}
				
				if (pl.misc_action==DR_DROP && pl.misc_target1==map[m].x && pl.misc_target2==map[m].y)
					copysprite(32,0,x*32,y*32,xoff,yoff);
				if (pl.misc_action==DR_PICKUP && pl.misc_target1==map[m].x && pl.misc_target2==map[m].y)
					copysprite(33,0,x*32,y*32,xoff,yoff);
				if (pl.misc_action==DR_USE && pl.misc_target1==map[m].x && pl.misc_target2==map[m].y)
					copysprite(45,0,x*32,y*32,xoff,yoff);

				// effects
				if (map[m].flags2&MF_MOVEBLOCK)	copysprite(55,0,x*32,y*32,xoff,yoff);
				if (map[m].flags2&MF_SIGHTBLOCK) copysprite(84,0,x*32,y*32,xoff,yoff);
				if (map[m].flags2&MF_INDOORS) copysprite(56,0,x*32,y*32,xoff,yoff);
				if (map[m].flags2&MF_UWATER) copysprite(75,0,x*32,y*32,xoff,yoff);
				if (map[m].flags2&MF_NOFIGHT) copysprite(58,0,x*32,y*32,xoff,yoff);
				if (map[m].flags2&MF_NOMONST) copysprite(59,0,x*32,y*32,xoff,yoff);
				if (map[m].flags2&MF_BANK) copysprite(60,0,x*32,y*32,xoff,yoff);
				if (map[m].flags2&MF_TAVERN) copysprite(61,0,x*32,y*32,xoff,yoff);
				if (map[m].flags2&MF_NOMAGIC) copysprite(62,0,x*32,y*32,xoff,yoff);
				if (map[m].flags2&MF_DEATHTRAP)	copysprite(73,0,x*32,y*32,xoff,yoff);
				if (map[m].flags2&MF_NOLAG)	copysprite(57,0,x*32,y*32,xoff,yoff);
				if (map[m].flags2&MF_ARENA)	copysprite(76,0,x*32,y*32,xoff,yoff);
				if (map[m].flags2&MF_NOPLAYER) copysprite(77,0,x*32,y*32,xoff,yoff);
				if (map[m].flags2&MF_NOEXPIRE) copysprite(82,0,x*32,y*32,xoff,yoff);
				if (map[m].flags2&0x80000000) copysprite(72,0,x*32,y*32,xoff,yoff);
				
				inj = 1079;
				
				if (map[m].flags&CRITTED)
					inj = 1206;
				if ((map[m].flags&(INJURED|INJURED1|INJURED2))==INJURED)
					copysprite(inj,0,x*32,y*32,xoff+map[m].obj_xoff,yoff+map[m].obj_yoff);
				if ((map[m].flags&(INJURED|INJURED1|INJURED2))==(INJURED|INJURED1))
					copysprite(inj+1,0,x*32,y*32,xoff+map[m].obj_xoff,yoff+map[m].obj_yoff);
				if ((map[m].flags&(INJURED|INJURED1|INJURED2))==(INJURED|INJURED2))
					copysprite(inj+2,0,x*32,y*32,xoff+map[m].obj_xoff,yoff+map[m].obj_yoff);
				if ((map[m].flags&(INJURED|INJURED1|INJURED2))==(INJURED|INJURED1|INJURED2))
					copysprite(inj+3,0,x*32,y*32,xoff+map[m].obj_xoff,yoff+map[m].obj_yoff);

				if (map[m].flags&DEATH) {
					if (map[m].obj2) copysprite(280+((map[m].flags&DEATH)>>17)-1,0,x*32,y*32,xoff+map[m].obj_xoff,yoff+map[m].obj_yoff);
					else copysprite(280+((map[m].flags&DEATH)>>17)-1,0,x*32,y*32,xoff,yoff);
				}
				if (map[m].flags&TOMB) {
					copysprite(240+((map[m].flags&TOMB)>>12)-1,map[m].light,x*32,y*32,xoff,yoff);
				}

				alpha=0; alphastr=0;

				if (map[m].flags&EMAGIC) {
					alpha|=1;
					alphastr=max((unsigned)alphastr,((map[m].flags&EMAGIC)>>22));
				}

				if (map[m].flags&GMAGIC) {
					alpha|=2;
					alphastr=max((unsigned)alphastr,((map[m].flags&GMAGIC)>>25));
				}

				if (map[m].flags&CMAGIC) {
					alpha|=4;
					alphastr=max((unsigned)alphastr,((map[m].flags&CMAGIC)>>28));
				}
				if (alpha) dd_alphaeffect_magic(alpha,alphastr,x*32,y*32,xoff+map[m].obj_xoff,yoff+map[m].obj_yoff);
			}
		}
	} else {
		for (y=screen_renderdist-1; y>=0; y--) {
			for (x=0; x<screen_renderdist; x++) {
				// background
				copysprite(SPR_EMPTY,0,x*32,y*32,-176,0);
			}
		}
	}

	if (!selected_visible) selected_char=0;

	copyspritex(do_darkmode?18001:1,0,0,0);

	if (init) {
		// Store your position on the map as a white pixel
		xmap[mapy+mapx*MAPX_MAX]=0xffff;
		
		// offsets the minimap draw, so that it begins drawing from 64 tiles to the upper-left of your position,
		//   drawing 128 x 128 tiles as pixels in dd_show_map.
		// Added to this, mm_magnify carries a magnification value, which shrinks the tile offsets
		//   and increases the size of the drawn 'pixels' to fill the 128x128 space.
		mapx=mapx-(64/max(1,mm_magnify));
		if (mapx<0)	mapx=0;
		if (mapx>1024-((128/max(1,mm_magnify))+1)) mapx=1024-((128/max(1,mm_magnify))+1);

		mapy=mapy-(64/max(1,mm_magnify));
		if (mapy<0)	mapy=0;
		if (mapy>MAPY_MAX-((128/max(1,mm_magnify))+1)) mapy=MAPY_MAX-((128/max(1,mm_magnify))+1);

		dd_show_map(xmap,mapx,mapy,max(1,mm_magnify));
	}

	eng_display_win(plr_sprite,init);

	// ********************
	// display cursors etc.
	// ********************

	if (init && pl.citem) {
		if (cursor_type==CT_DROP || cursor_type==CT_SWAP || cursor_type==CT_USE)
		{
			copyspritex(pl.citem,mouse_x-16,mouse_y-16,16);
			// Draw soulstone icon 
			if (pl.citem_p&PL_SOULSTONED)
				copyspritex(4496,mouse_x-16,mouse_y-16,16);
			// Draw talisman icon 
			if (pl.citem_p&PL_ENCHANTED)
				copyspritex(4497,mouse_x-16,mouse_y-16,16);
			// Draw corrupt icon 
			if (pl.citem_p&PL_CORRUPTED)
				copyspritex(6881,mouse_x-16,mouse_y-16,16);
			
			if (pl.citem_s>0&&pl.citem_s<=10)
				copyspritex(4000+pl.citem_s,mouse_x-16,mouse_y-16,16);
		}
		else
		{
			copyspritex(pl.citem,mouse_x-16,mouse_y-16,0);
			// Draw soulstone icon 
			if (pl.citem_p&PL_SOULSTONED)
				copyspritex(4496,mouse_x-16,mouse_y-16,0);
			// Draw talisman icon 
			if (pl.citem_p&PL_ENCHANTED)
				copyspritex(4497,mouse_x-16,mouse_y-16,0);
			// Draw corrupt icon 
			if (pl.citem_p&PL_CORRUPTED)
				copyspritex(6881,mouse_x-16,mouse_y-16,0);
			
			if (pl.citem_s>0&&pl.citem_s<=10)
				copyspritex(4000+pl.citem_s,mouse_x-16,mouse_y-16,0);
		}
	}
}

// DISPLAY: TEXT OUTPUT

void tlog(char *text,char font)
{
	int n,panic=0;
	static int flag=0;

	if (!flag) {
		for (n=0; n<XLL*60; n++) {
			logtext[0][n]=0;
		}
		for (n=0; n<XLL; n++) {
			logfont[n]=0;
		}
		flag=1;
	}

	if (strlen(text)<1)	return;
	
	// This loop moves previous line of text into the next line of text
	// with line breaks in mind, this arranges the text lines backwards.
	while (panic++<XLL) {
		do_msg();
		memmove(logtext[1],logtext[0],XLL*60-60);
		memmove(&logfont[1],&logfont[0],XLL-1);
		memcpy(logtext[0],text,min(60-1,strlen(text)+1));
		logfont[0]=font;
		logtext[0][60-1]=0;
		if (strlen(text)<XS-1) return;
		for (n=XS-1; n>0; n--) if (logtext[0][n]==' ') break;
		if (n!=0) {
			logtext[0][n]=0; text+=n+1;
		} else text+=XS-1;
	}
}

void motdlog(char *text,char font)
{
	int n,panic=0;
	static int flag=0;
	static int currline=0;
	
	if (!flag) {
		for (n=0; n<MLL*60; n++) {
			motdtext[0][n]=0;
		}
		for (n=0; n<MLL; n++) {
			motdfont[n]=0;
		}
		flag=1;
	}

	if (strlen(text)<1)	return;
	
	// Tracking the text array with currline, this should pass to the next line
	// this will go until currline exceeds 60. As a failsafe, it then resets to 0.
	while (panic++<MLL) {
		do_msg();
		
		if (currline>=59) currline=0;
		
		memcpy(motdtext[currline],text,min(60-1,strlen(text)+1));
		motdfont[currline]=font;
		motdtext[currline][60-1]=0;
		
		if (strlen(text)<XMS-1) break;
		for (n=XMS-1; n>0; n--) if (motdtext[currline][n]==' ') break;
		if (n!=0) {
			motdtext[currline][n]=0; text+=n+1;
		} else text+=XMS-1;
		
		currline++;
	}
	currline++;
}

void xlog(char font,char *format,...)
{
	va_list args;
	char buf[1024];

	va_start(args,format);
	vsprintf(buf,format,args);
	tlog(buf,font);
	va_end(args);
}

void mxlog(char font,char *format,...)
{
	va_list args;
	char buf[1024];

	va_start(args,format);
	vsprintf(buf,format,args);
	motdlog(buf,font);
	va_end(args);
}


// ************* MAIN *********************

void init_engine(void)
{
	eng_init_map();
	eng_init_player();
}

void do_msg(void)
{
	MSG msg;

	if (PeekMessage(&msg,NULL,0,0,PM_REMOVE)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}

void eng_flip(unsigned int t)
{
	int diff;
	MSG msg;

	diff=t-GetTickCount();
	if (diff>0)	idle+=diff;

	do {
		if (PeekMessage(&msg,NULL,0,0,PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		} else Sleep(1);
	} while (t>GetTickCount());

	if (screen_windowed == 1) {
		dd_flip_windowed();
	} else {
		dd_flip();
	}

	frame++;
}

// Ultra big speed table. I recommend just collapsing this bracket (if you can stand the lag when scrolling), it is massive.
unsigned char speedtab[300][200]=
{
//   1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0,1,2,3,4... etc
	{1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2},
	{1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,1,2},
	{1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,1,2},
	{1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,1,2},
	{1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,1,2},
	{1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,1,2},
	{1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,1,2},
	{1,2,1,2,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,2,1,2,1,1,2},
	{1,2,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,2,1,1,2},
	{1,2,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,2,1,1,2},
	{1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,1,2},
	{1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,1,2},
	{1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,1,2},
	{1,2,1,2,1,2,1,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,1,2,1,2,1,2,1,1,2},
	{1,2,1,2,1,2,1,1,2,1,2,1,2,1,1,2,1,2,1,2,1,1,2,1,2,1,2,1,1,2,1,2,1,2,1,1,2,1,2,1,2,1,1,2,1,2,1,2,1,1,2,1,2,1,2,1,1,2,1,2,1,2,1,1,2,1,2,1,2,1,1,2,1,2,1,2,1,1,2,1,2,1,2,1,1,2,1,2,1,2,1,1,2,1,2,1,2,1,1,2,1,2,1,2,1,2,1,1,2,1,2,1,2,1,1,2,1,2,1,2,1,1,2,1,2,1,2,1,1,2,1,2,1,2,1,1,2,1,2,1,2,1,1,2,1,2,1,2,1,1,2,1,2,1,2,1,1,2,1,2,1,2,1,1,2,1,2,1,2,1,1,2,1,2,1,2,1,1,2,1,2,1,2,1,1,2,1,2,1,2,1,1,2,1,2,1,2,1,1,2},
	{1,2,1,2,1,1,2,1,2,1,2,1,1,2,1,2,1,2,1,1,2,1,2,1,2,1,1,2,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,2,1,1,2,1,2,1,2,1,1,2,1,2,1,2,1,1,2,1,2,1,2,1,1,2,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,2,1,1,2,1,2,1,2,1,1,2,1,2,1,2,1,1,2,1,2,1,2,1,1,2,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,2,1,1,2,1,2,1,2,1,1,2,1,2,1,2,1,1,2,1,2,1,2,1,1,2,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,2,1,1,2,1,2,1,2,1,1,2,1,2,1,2,1,1,2,1,2,1,2,1,1,2,1,2,1,2,1,1,2,1,2,1,1,2},
	{1,2,1,2,1,1,2,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,2,1,1,2,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,2,1,1,2,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,2,1,1,2,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,2,1,1,2,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,2,1,1,2,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,2,1,1,2,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,2,1,1,2,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,2,1,1,2,1,2,1,1,2},
	{1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2},
	{1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2},
	{1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2},
	{1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2},
	{1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,1,2},
	{1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,1,2},
	{1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,1,2},
	{1,2,1,1,2,1,2,1,1,2,1,1,2,1,2,1,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,1,2,1,2,1,1,2,1,1,2,1,2,1,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,1,2,1,2,1,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,1,2,1,2,1,1,2,1,1,2,1,2,1,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,1,2,1,2,1,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,1,2,1,2,1,1,2,1,1,2,1,2,1,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,1,2,1,2,1,1,2,1,1,2,1,2,1,1,2,1,2,1,1,2,1,1,2,1,2,1,1,2,1,1,2,1,2,1,1,2,1,1,2},
	{1,2,1,1,2,1,1,2,1,2,1,1,2,1,1,2,1,2,1,1,2,1,1,2,1,2,1,1,2,1,1,2,1,2,1,1,2,1,1,2,1,2,1,1,2,1,1,2,1,2,1,1,2,1,1,2,1,2,1,1,2,1,1,2,1,2,1,1,2,1,1,2,1,2,1,1,2,1,1,2,1,2,1,1,2,1,1,2,1,2,1,1,2,1,1,2,1,2,1,1,2,1,1,2,1,2,1,1,2,1,1,2,1,2,1,1,2,1,1,2,1,2,1,1,2,1,1,2,1,2,1,1,2,1,1,2,1,2,1,1,2,1,1,2,1,2,1,1,2,1,1,2,1,2,1,1,2,1,1,2,1,2,1,1,2,1,1,2,1,2,1,1,2,1,1,2,1,2,1,1,2,1,1,2,1,2,1,1,2,1,1,2,1,2,1,1,2,1,1,2},
	{1,2,1,1,2,1,1,2,1,2,1,1,2,1,1,2,1,2,1,1,2,1,1,2,1,1,2,1,2,1,1,2,1,1,2,1,2,1,1,2,1,1,2,1,2,1,1,2,1,1,2,1,1,2,1,2,1,1,2,1,1,2,1,2,1,1,2,1,1,2,1,2,1,1,2,1,1,2,1,1,2,1,2,1,1,2,1,1,2,1,2,1,1,2,1,1,2,1,1,2,1,2,1,1,2,1,1,2,1,2,1,1,2,1,1,2,1,2,1,1,2,1,1,2,1,1,2,1,2,1,1,2,1,1,2,1,2,1,1,2,1,1,2,1,2,1,1,2,1,1,2,1,1,2,1,2,1,1,2,1,1,2,1,2,1,1,2,1,1,2,1,2,1,1,2,1,1,2,1,1,2,1,2,1,1,2,1,1,2,1,2,1,1,2,1,1,2,1,1,2},
	{1,2,1,1,2,1,1,2,1,2,1,1,2,1,1,2,1,1,2,1,2,1,1,2,1,1,2,1,1,2,1,2,1,1,2,1,1,2,1,1,2,1,2,1,1,2,1,1,2,1,1,2,1,2,1,1,2,1,1,2,1,1,2,1,2,1,1,2,1,1,2,1,2,1,1,2,1,1,2,1,1,2,1,2,1,1,2,1,1,2,1,1,2,1,2,1,1,2,1,1,2,1,1,2,1,2,1,1,2,1,1,2,1,1,2,1,2,1,1,2,1,1,2,1,1,2,1,2,1,1,2,1,1,2,1,2,1,1,2,1,1,2,1,1,2,1,2,1,1,2,1,1,2,1,1,2,1,2,1,1,2,1,1,2,1,1,2,1,2,1,1,2,1,1,2,1,1,2,1,2,1,1,2,1,1,2,1,1,2,1,2,1,1,2,1,1,2,1,1,2},
	{1,2,1,1,2,1,1,2,1,1,2,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,2,1,1,2,1,1,2,1,1,2,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,2,1,1,2,1,1,2,1,1,2,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,2,1,1,2,1,1,2,1,1,2,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,2,1,1,2,1,1,2,1,1,2,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,2,1,1,2,1,1,2,1,1,2,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,2,1,1,2,1,1,2,1,1,2,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,2,1,1,2,1,1,2,1,1,2,1,2,1,1,2,1,1,2,1,1,2,1,1,2},
	{1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2},
	{1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2},
	{1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2},
	{1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2},
	{1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2},
	{1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,1,2},
	{1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,1,2},
	{1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,1,2},
	{1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,1,2},
	{1,1,2,1,1,2,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,2,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,2,1,1,2,1,1,1,2},
	{1,1,2,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,2,1,1,1,2},
	{1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,2,1,1,1,2},
	{1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,1,2},
	{1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,1,2},
	{1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,1,2},
	{1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,1,2},
	{1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,1,2},
	{1,1,2,1,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2},
	{1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2},
	{1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2},
	{1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2},
	{1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2},
	{1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,1,2},
	{1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,1,2},
	{1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,1,2},
	{1,1,1,2,1,1,1,2,1,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,1,2,1,1,1,2,1,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,1,2,1,1,1,2,1,1,1,1,2},
	{1,1,1,2,1,1,1,2,1,1,1,1,2,1,1,1,2,1,1,1,1,2,1,1,1,2,1,1,1,1,2,1,1,1,2,1,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,1,2,1,1,1,2,1,1,1,1,2,1,1,1,2,1,1,1,1,2,1,1,1,2,1,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,1,2,1,1,1,2,1,1,1,1,2,1,1,1,2,1,1,1,1,2,1,1,1,2,1,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,1,2,1,1,1,2,1,1,1,1,2,1,1,1,2,1,1,1,1,2,1,1,1,2,1,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,1,2,1,1,1,2,1,1,1,1,2,1,1,1,2,1,1,1,1,2,1,1,1,2,1,1,1,1,2},
	{1,1,1,2,1,1,1,1,2,1,1,1,2,1,1,1,1,2,1,1,1,2,1,1,1,1,2,1,1,1,2,1,1,1,1,2,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,2,1,1,1,1,2,1,1,1,2,1,1,1,1,2,1,1,1,2,1,1,1,1,2,1,1,1,2,1,1,1,1,2,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,2,1,1,1,1,2,1,1,1,2,1,1,1,1,2,1,1,1,2,1,1,1,1,2,1,1,1,2,1,1,1,1,2,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,2,1,1,1,1,2,1,1,1,2,1,1,1,1,2,1,1,1,2,1,1,1,1,2,1,1,1,2,1,1,1,1,2,1,1,1,2,1,1,1,1,2,1,1,1,1,2},
	{1,1,1,2,1,1,1,1,2,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,2,1,1,1,1,2,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,2,1,1,1,1,2,1,1,1,1,2},
	{1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,1,2},
	{1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,1,2},
	{1,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,1,2},
	{1,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,1,1,2},
	{1,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,1,1,2},
	{1,1,1,1,2,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,2,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,2,1,1,1,1,1,2},
	{1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,2},
	{1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,2},
	{1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,2},
	{1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,1,2},
	{1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,1,2},
	{1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,1,2},
	{1,1,1,1,1,2,1,1,1,1,1,1,2,1,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,1,2,1,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,1,2,1,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,1,2,1,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,1,2,1,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,1,2,1,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,1,2,1,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,1,2,1,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,1,2,1,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,1,2,1,1,1,1,1,1,2},
	{1,1,1,1,1,2,1,1,1,1,1,1,2,1,1,1,1,1,1,2,1,1,1,1,1,1,2,1,1,1,1,1,1,2,1,1,1,1,1,1,2,1,1,1,1,1,1,2,1,1,1,1,1,1,2,1,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,1,2,1,1,1,1,1,1,2,1,1,1,1,1,1,2,1,1,1,1,1,1,2,1,1,1,1,1,1,2,1,1,1,1,1,1,2,1,1,1,1,1,1,2,1,1,1,1,1,1,2,1,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,1,2,1,1,1,1,1,1,2,1,1,1,1,1,1,2,1,1,1,1,1,1,2,1,1,1,1,1,1,2,1,1,1,1,1,1,2,1,1,1,1,1,1,2,1,1,1,1,1,1,2,1,1,1,1,1,1,2},
	{1,1,1,1,1,1,2,1,1,1,1,1,1,2,1,1,1,1,1,1,2,1,1,1,1,1,1,2,1,1,1,1,1,1,2,1,1,1,1,1,1,2,1,1,1,1,1,1,1,2,1,1,1,1,1,1,2,1,1,1,1,1,1,2,1,1,1,1,1,1,2,1,1,1,1,1,1,2,1,1,1,1,1,1,2,1,1,1,1,1,1,2,1,1,1,1,1,1,1,2,1,1,1,1,1,1,2,1,1,1,1,1,1,2,1,1,1,1,1,1,2,1,1,1,1,1,1,2,1,1,1,1,1,1,2,1,1,1,1,1,1,2,1,1,1,1,1,1,1,2,1,1,1,1,1,1,2,1,1,1,1,1,1,2,1,1,1,1,1,1,2,1,1,1,1,1,1,2,1,1,1,1,1,1,2,1,1,1,1,1,1,2,1,1,1,1,1,1,1,2},
	{1,1,1,1,1,1,2,1,1,1,1,1,1,2,1,1,1,1,1,1,1,2,1,1,1,1,1,1,2,1,1,1,1,1,1,1,2,1,1,1,1,1,1,2,1,1,1,1,1,1,2,1,1,1,1,1,1,1,2,1,1,1,1,1,1,2,1,1,1,1,1,1,1,2,1,1,1,1,1,1,2,1,1,1,1,1,1,2,1,1,1,1,1,1,1,2,1,1,1,1,1,1,2,1,1,1,1,1,1,1,2,1,1,1,1,1,1,2,1,1,1,1,1,1,2,1,1,1,1,1,1,1,2,1,1,1,1,1,1,2,1,1,1,1,1,1,1,2,1,1,1,1,1,1,2,1,1,1,1,1,1,2,1,1,1,1,1,1,1,2,1,1,1,1,1,1,2,1,1,1,1,1,1,1,2,1,1,1,1,1,1,2,1,1,1,1,1,1,1,2},
	{1,1,1,1,1,1,2,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,2,1,1,1,1,1,1,2,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,2,1,1,1,1,1,1,2,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,2,1,1,1,1,1,1,2,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,2,1,1,1,1,1,1,2,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,2,1,1,1,1,1,1,2,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,2,1,1,1,1,1,1,2,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,2,1,1,1,1,1,1,2,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,2},
	{1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,2},
	{1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,2},
	{1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,2},
	{1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,2},
	{1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,2},
	{1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,2},
	{1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,2},
	{1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,2},
	{1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,2},
	{1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,2},
	{1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,2},
	{1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2},
	{1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2},
	{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2},
	{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2},
	{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2},
	{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2},
	{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2},
	{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2},
	{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2},
	{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2},
	{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2},
	{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2},
	{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2},
	{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2},
	{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
	{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1},
	{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1},
	{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1},
	{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1},
	{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1},
	{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1},
	{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1},
	{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1},
	{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1},
	{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1},
	{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1},
	{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1},
	{1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1},
	{1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1},
	{1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,0,1},
	{1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,0,1},
	{1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,0,1},
	{1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,0,1},
	{1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,0,1},
	{1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,0,1},
	{1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,0,1},
	{1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,0,1},
	{1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,0,1},
	{1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,0,1},
	{1,1,1,1,1,1,0,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,0,1},
	{1,1,1,1,1,1,0,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,0,1,1,1,1,1,1,0,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,0,1,1,1,1,1,1,0,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,0,1,1,1,1,1,1,0,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,0,1,1,1,1,1,1,0,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,0,1,1,1,1,1,1,0,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,0,1,1,1,1,1,1,0,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,0,1,1,1,1,1,1,0,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,0,1,1,1,1,1,1,0,1},
	{1,1,1,1,1,1,0,1,1,1,1,1,1,0,1,1,1,1,1,1,1,0,1,1,1,1,1,1,0,1,1,1,1,1,1,1,0,1,1,1,1,1,1,0,1,1,1,1,1,1,0,1,1,1,1,1,1,1,0,1,1,1,1,1,1,0,1,1,1,1,1,1,1,0,1,1,1,1,1,1,0,1,1,1,1,1,1,0,1,1,1,1,1,1,1,0,1,1,1,1,1,1,0,1,1,1,1,1,1,1,0,1,1,1,1,1,1,0,1,1,1,1,1,1,0,1,1,1,1,1,1,1,0,1,1,1,1,1,1,0,1,1,1,1,1,1,1,0,1,1,1,1,1,1,0,1,1,1,1,1,1,0,1,1,1,1,1,1,1,0,1,1,1,1,1,1,0,1,1,1,1,1,1,1,0,1,1,1,1,1,1,0,1,1,1,1,1,1,0,1},
	{1,1,1,1,1,1,0,1,1,1,1,1,1,0,1,1,1,1,1,1,0,1,1,1,1,1,1,0,1,1,1,1,1,1,0,1,1,1,1,1,1,0,1,1,1,1,1,1,0,1,1,1,1,1,1,1,0,1,1,1,1,1,1,0,1,1,1,1,1,1,0,1,1,1,1,1,1,0,1,1,1,1,1,1,0,1,1,1,1,1,1,0,1,1,1,1,1,1,0,1,1,1,1,1,1,1,0,1,1,1,1,1,1,0,1,1,1,1,1,1,0,1,1,1,1,1,1,0,1,1,1,1,1,1,0,1,1,1,1,1,1,0,1,1,1,1,1,1,0,1,1,1,1,1,1,1,0,1,1,1,1,1,1,0,1,1,1,1,1,1,0,1,1,1,1,1,1,0,1,1,1,1,1,1,0,1,1,1,1,1,1,0,1,1,1,1,1,1,0,1},
	{1,1,1,1,1,0,1,1,1,1,1,1,0,1,1,1,1,1,1,0,1,1,1,1,1,1,0,1,1,1,1,1,1,0,1,1,1,1,1,1,0,1,1,1,1,1,1,0,1,1,1,1,1,1,0,1,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,1,0,1,1,1,1,1,1,0,1,1,1,1,1,1,0,1,1,1,1,1,1,0,1,1,1,1,1,1,0,1,1,1,1,1,1,0,1,1,1,1,1,1,0,1,1,1,1,1,1,0,1,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,1,0,1,1,1,1,1,1,0,1,1,1,1,1,1,0,1,1,1,1,1,1,0,1,1,1,1,1,1,0,1,1,1,1,1,1,0,1,1,1,1,1,1,0,1,1,1,1,1,1,0,1,1,1,1,1,0,1},
	{1,1,1,1,1,0,1,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,1,0,1,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,1,0,1,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,1,0,1,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,1,0,1,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,1,0,1,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,1,0,1,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,1,0,1,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,1,0,1,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,1,0,1,1,1,1,1,1,0,1,1,1,1,1,0,1},
	{1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1},
	{1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1},
	{1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1},
	{1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,0,1},
	{1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,0,1},
	{1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,0,1},
	{1,1,1,1,0,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1},
	{1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1},
	{1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1},
	{1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1},
	{1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,0,1},
	{1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,0,1},
	{1,1,1,0,1,1,1,1,0,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,0,1,1,1,1,0,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,0,1,1,1,1,0,1,1,1,0,1},
	{1,1,1,0,1,1,1,1,0,1,1,1,0,1,1,1,1,0,1,1,1,0,1,1,1,1,0,1,1,1,0,1,1,1,1,0,1,1,1,0,1,1,1,1,0,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,0,1,1,1,1,0,1,1,1,0,1,1,1,1,0,1,1,1,0,1,1,1,1,0,1,1,1,0,1,1,1,1,0,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,0,1,1,1,1,0,1,1,1,0,1,1,1,1,0,1,1,1,0,1,1,1,1,0,1,1,1,0,1,1,1,1,0,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,0,1,1,1,1,0,1,1,1,0,1,1,1,1,0,1,1,1,0,1,1,1,1,0,1,1,1,0,1,1,1,1,0,1,1,1,0,1},
	{1,1,1,0,1,1,1,0,1,1,1,1,0,1,1,1,0,1,1,1,1,0,1,1,1,0,1,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,1,0,1,1,1,0,1,1,1,1,0,1,1,1,0,1,1,1,1,0,1,1,1,0,1,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,1,0,1,1,1,0,1,1,1,1,0,1,1,1,0,1,1,1,1,0,1,1,1,0,1,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,1,0,1,1,1,0,1,1,1,1,0,1,1,1,0,1,1,1,1,0,1,1,1,0,1,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,1,0,1,1,1,0,1,1,1,1,0,1,1,1,0,1,1,1,1,0,1,1,1,0,1,1,1,1,0,1,1,1,0,1,1,1,0,1},
	{1,1,1,0,1,1,1,0,1,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,1,0,1,1,1,0,1,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,1,0,1,1,1,0,1,1,1,0,1},
	{1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1},
	{1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1},
	{1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1},
	{1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1},
	{1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,0,1},
	{1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,0,1},
	{1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,0,1},
	{1,1,0,1,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,1,0,1,1,0,1},
	{1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,0,1},
	{1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,0,1},
	{1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,0,1},
	{1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,0,1},
	{1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,0,1},
	{1,1,0,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,0,1},
	{1,1,0,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,0,1,1,0,1},
	{1,1,0,1,1,0,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1},
	{1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1},
	{1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1},
	{1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1},
	{1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1},
	{1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,0,1},
	{1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,0,1},
	{1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,0,1},
	{1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,0,1},
	{1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,0,1},
	{1,0,1,1,0,1,1,0,1,1,0,1,0,1,1,0,1,1,0,1,1,0,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,0,1,1,0,1,1,0,1,1,0,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,0,1,1,0,1,1,0,1,1,0,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,0,1,1,0,1,1,0,1,1,0,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,0,1,1,0,1,1,0,1,1,0,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,0,1,1,0,1,1,0,1,1,0,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,0,1,1,0,1,1,0,1,1,0,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,0,1,1,0,1,1,0,1,1,0,1,0,1},
	{1,0,1,1,0,1,1,0,1,0,1,1,0,1,1,0,1,1,0,1,0,1,1,0,1,1,0,1,1,0,1,0,1,1,0,1,1,0,1,1,0,1,0,1,1,0,1,1,0,1,1,0,1,0,1,1,0,1,1,0,1,1,0,1,0,1,1,0,1,1,0,1,0,1,1,0,1,1,0,1,1,0,1,0,1,1,0,1,1,0,1,1,0,1,0,1,1,0,1,1,0,1,1,0,1,0,1,1,0,1,1,0,1,1,0,1,0,1,1,0,1,1,0,1,1,0,1,0,1,1,0,1,1,0,1,0,1,1,0,1,1,0,1,1,0,1,0,1,1,0,1,1,0,1,1,0,1,0,1,1,0,1,1,0,1,1,0,1,0,1,1,0,1,1,0,1,1,0,1,0,1,1,0,1,1,0,1,1,0,1,0,1,1,0,1,1,0,1,0,1},
	{1,0,1,1,0,1,1,0,1,0,1,1,0,1,1,0,1,0,1,1,0,1,1,0,1,1,0,1,0,1,1,0,1,1,0,1,0,1,1,0,1,1,0,1,0,1,1,0,1,1,0,1,1,0,1,0,1,1,0,1,1,0,1,0,1,1,0,1,1,0,1,0,1,1,0,1,1,0,1,1,0,1,0,1,1,0,1,1,0,1,0,1,1,0,1,1,0,1,0,1,1,0,1,1,0,1,1,0,1,0,1,1,0,1,1,0,1,0,1,1,0,1,1,0,1,1,0,1,0,1,1,0,1,1,0,1,0,1,1,0,1,1,0,1,0,1,1,0,1,1,0,1,1,0,1,0,1,1,0,1,1,0,1,0,1,1,0,1,1,0,1,0,1,1,0,1,1,0,1,1,0,1,0,1,1,0,1,1,0,1,0,1,1,0,1,1,0,1,0,1},
	{1,0,1,1,0,1,0,1,1,0,1,1,0,1,0,1,1,0,1,1,0,1,0,1,1,0,1,1,0,1,0,1,1,0,1,1,0,1,0,1,1,0,1,1,0,1,0,1,1,0,1,1,0,1,0,1,1,0,1,1,0,1,0,1,1,0,1,1,0,1,0,1,1,0,1,1,0,1,0,1,1,0,1,1,0,1,0,1,1,0,1,1,0,1,0,1,1,0,1,1,0,1,0,1,1,0,1,1,0,1,0,1,1,0,1,1,0,1,0,1,1,0,1,1,0,1,0,1,1,0,1,1,0,1,0,1,1,0,1,1,0,1,0,1,1,0,1,1,0,1,0,1,1,0,1,1,0,1,0,1,1,0,1,1,0,1,0,1,1,0,1,1,0,1,0,1,1,0,1,1,0,1,0,1,1,0,1,1,0,1,0,1,1,0,1,1,0,1,0,1},
	{1,0,1,1,0,1,0,1,1,0,1,1,0,1,0,1,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,1,0,1,0,1,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,1,0,1,0,1,1,0,1,1,0,1,0,1,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,1,0,1,0,1,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,1,0,1,0,1,1,0,1,1,0,1,0,1,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,1,0,1,0,1,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,1,0,1,0,1,1,0,1,1,0,1,0,1,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,1,0,1,0,1,1,0,1,1,0,1,0,1,1,0,1,0,1},
	{1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1},
	{1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1},
	{1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1},
	{1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1},
	{1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,0,1},
	{1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,0,1},
	{1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,0,1},
	{1,0,1,0,1,1,0,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,0,1,1,0,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,0,1,1,0,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,0,1,1,0,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,0,1,1,0,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,0,1,1,0,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,0,1,1,0,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,0,1,1,0,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,0,1,1,0,1,0,1,0,1},
	{1,0,1,0,1,1,0,1,0,1,0,1,1,0,1,0,1,0,1,1,0,1,0,1,0,1,1,0,1,0,1,0,1,1,0,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,0,1,1,0,1,0,1,0,1,1,0,1,0,1,0,1,1,0,1,0,1,0,1,1,0,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,0,1,1,0,1,0,1,0,1,1,0,1,0,1,0,1,1,0,1,0,1,0,1,1,0,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,0,1,1,0,1,0,1,0,1,1,0,1,0,1,0,1,1,0,1,0,1,0,1,1,0,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,0,1,1,0,1,0,1,0,1,1,0,1,0,1,0,1,1,0,1,0,1,0,1,1,0,1,0,1,0,1},
	{1,0,1,0,1,0,1,1,0,1,0,1,0,1,1,0,1,0,1,0,1,1,0,1,0,1,0,1,1,0,1,0,1,0,1,1,0,1,0,1,0,1,1,0,1,0,1,0,1,1,0,1,0,1,0,1,1,0,1,0,1,0,1,1,0,1,0,1,0,1,1,0,1,0,1,0,1,1,0,1,0,1,0,1,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,1,0,1,0,1,0,1,1,0,1,0,1,0,1,1,0,1,0,1,0,1,1,0,1,0,1,0,1,1,0,1,0,1,0,1,1,0,1,0,1,0,1,1,0,1,0,1,0,1,1,0,1,0,1,0,1,1,0,1,0,1,0,1,1,0,1,0,1,0,1,1,0,1,0,1,0,1,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1},
	{1,0,1,0,1,0,1,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1},
	{1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1},
	{1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,0,1},
	{1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,0,1},
	{1,0,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,0,1,0,1},
	{1,0,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,0,1,0,1},
	{1,0,1,0,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1},
	{1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1},
	{1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1},
	{1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1},
	{1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1},
	{1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1},
	{1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1},
	{0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1},
	{0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,0,1},
	{0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,0,1},
	{0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,0,1},
	{0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,0,1},
	{0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,0,1},
	{0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,0,1},
	{0,1,0,1,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,1,0,1,0,0,1},
	{0,1,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,1,0,0,1},
	{0,1,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,1,0,0,1},
	{0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,0,1},
	{0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,0,1},
	{0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,0,1},
	{0,1,0,1,0,1,0,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,0,1,0,1,0,1,0,0,1},
	{0,1,0,1,0,1,0,0,1,0,1,0,1,0,0,1,0,1,0,1,0,0,1,0,1,0,1,0,0,1,0,1,0,1,0,0,1,0,1,0,1,0,0,1,0,1,0,1,0,0,1,0,1,0,1,0,0,1,0,1,0,1,0,0,1,0,1,0,1,0,0,1,0,1,0,1,0,0,1,0,1,0,1,0,0,1,0,1,0,1,0,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,0,1,0,1,0,1,0,0,1,0,1,0,1,0,0,1,0,1,0,1,0,0,1,0,1,0,1,0,0,1,0,1,0,1,0,0,1,0,1,0,1,0,0,1,0,1,0,1,0,0,1,0,1,0,1,0,0,1,0,1,0,1,0,0,1,0,1,0,1,0,0,1,0,1,0,1,0,0,1,0,1,0,1,0,0,1},
	{0,1,0,1,0,0,1,0,1,0,1,0,0,1,0,1,0,1,0,0,1,0,1,0,1,0,0,1,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,1,0,0,1,0,1,0,1,0,0,1,0,1,0,1,0,0,1,0,1,0,1,0,0,1,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,1,0,0,1,0,1,0,1,0,0,1,0,1,0,1,0,0,1,0,1,0,1,0,0,1,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,1,0,0,1,0,1,0,1,0,0,1,0,1,0,1,0,0,1,0,1,0,1,0,0,1,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,1,0,0,1,0,1,0,1,0,0,1,0,1,0,1,0,0,1,0,1,0,1,0,0,1,0,1,0,1,0,0,1,0,1,0,0,1},
	{0,1,0,1,0,0,1,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,1,0,0,1,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,1,0,0,1,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,1,0,0,1,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,1,0,0,1,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,1,0,0,1,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,1,0,0,1,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,1,0,0,1,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,1,0,0,1,0,1,0,0,1},
	{0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1},
	{0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1},
	{0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1},
	{0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1},
	{0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,0,1},
	{0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,0,1},
	{0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,0,1},
	{0,1,0,0,1,0,1,0,0,1,0,0,1,0,1,0,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,0,1,0,1,0,0,1,0,0,1,0,1,0,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,0,1,0,1,0,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,0,1,0,1,0,0,1,0,0,1,0,1,0,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,0,1,0,1,0,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,0,1,0,1,0,0,1,0,0,1,0,1,0,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,0,1,0,1,0,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,0,1,0,1,0,0,1,0,0,1,0,1,0,0,1,0,0,1},
	{0,1,0,0,1,0,0,1,0,1,0,0,1,0,0,1,0,1,0,0,1,0,0,1,0,1,0,0,1,0,0,1,0,1,0,0,1,0,0,1,0,1,0,0,1,0,0,1,0,1,0,0,1,0,0,1,0,1,0,0,1,0,0,1,0,1,0,0,1,0,0,1,0,1,0,0,1,0,0,1,0,1,0,0,1,0,0,1,0,1,0,0,1,0,0,1,0,1,0,0,1,0,0,1,0,1,0,0,1,0,0,1,0,1,0,0,1,0,0,1,0,1,0,0,1,0,0,1,0,1,0,0,1,0,0,1,0,1,0,0,1,0,0,1,0,1,0,0,1,0,0,1,0,1,0,0,1,0,0,1,0,1,0,0,1,0,0,1,0,1,0,0,1,0,0,1,0,1,0,0,1,0,0,1,0,1,0,0,1,0,0,1,0,1,0,0,1,0,0,1},
	{0,1,0,0,1,0,0,1,0,1,0,0,1,0,0,1,0,1,0,0,1,0,0,1,0,0,1,0,1,0,0,1,0,0,1,0,1,0,0,1,0,0,1,0,1,0,0,1,0,0,1,0,0,1,0,1,0,0,1,0,0,1,0,1,0,0,1,0,0,1,0,1,0,0,1,0,0,1,0,0,1,0,1,0,0,1,0,0,1,0,1,0,0,1,0,0,1,0,0,1,0,1,0,0,1,0,0,1,0,1,0,0,1,0,0,1,0,1,0,0,1,0,0,1,0,0,1,0,1,0,0,1,0,0,1,0,1,0,0,1,0,0,1,0,1,0,0,1,0,0,1,0,0,1,0,1,0,0,1,0,0,1,0,1,0,0,1,0,0,1,0,1,0,0,1,0,0,1,0,0,1,0,1,0,0,1,0,0,1,0,1,0,0,1,0,0,1,0,0,1},
	{0,1,0,0,1,0,0,1,0,1,0,0,1,0,0,1,0,0,1,0,1,0,0,1,0,0,1,0,0,1,0,1,0,0,1,0,0,1,0,0,1,0,1,0,0,1,0,0,1,0,0,1,0,1,0,0,1,0,0,1,0,0,1,0,1,0,0,1,0,0,1,0,1,0,0,1,0,0,1,0,0,1,0,1,0,0,1,0,0,1,0,0,1,0,1,0,0,1,0,0,1,0,0,1,0,1,0,0,1,0,0,1,0,0,1,0,1,0,0,1,0,0,1,0,0,1,0,1,0,0,1,0,0,1,0,1,0,0,1,0,0,1,0,0,1,0,1,0,0,1,0,0,1,0,0,1,0,1,0,0,1,0,0,1,0,0,1,0,1,0,0,1,0,0,1,0,0,1,0,1,0,0,1,0,0,1,0,0,1,0,1,0,0,1,0,0,1,0,0,1},
	{0,1,0,0,1,0,0,1,0,0,1,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,1,0,0,1,0,0,1,0,0,1,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,1,0,0,1,0,0,1,0,0,1,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,1,0,0,1,0,0,1,0,0,1,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,1,0,0,1,0,0,1,0,0,1,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,1,0,0,1,0,0,1,0,0,1,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,1,0,0,1,0,0,1,0,0,1,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,1,0,0,1,0,0,1,0,0,1,0,1,0,0,1,0,0,1,0,0,1,0,0,1},
	{0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1},
	{0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1},
	{0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1},
	{0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1},
	{0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1},
	{0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,0,1},
	{0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,0,1},
	{0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,0,1},
	{0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,0,1},
	{0,0,1,0,0,1,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,1,0,0,1,0,0,0,1},
	{0,0,1,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,1,0,0,0,1},
	{0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,1,0,0,0,1},
	{0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,0,1},
	{0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,0,1},
	{0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,0,1},
	{0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,0,1},
	{0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,0,1},
	{0,0,1,0,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1},
	{0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1},
	{0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1},
	{0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1},
	{0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1},
	{0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,0,1},
	{0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,0,1},
	{0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,0,1},
	{0,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0,1,0,0,0,0,1},
	{0,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0,1,0,0,0,0,1},
	{0,0,0,1,0,0,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0,0,1},
	{0,0,0,1,0,0,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0,0,1},
	{0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1},
	{0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1},
	{0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1},
	{0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,0,1},
	{0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,0,1},
	{0,0,0,0,1,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,1,0,0,0,0,0,1},
	{0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,1},
	{0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,1},
	{0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,1},
	{0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,0,1},
	{0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,0,1},
	{0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,0,1},
	{0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,1},
	{0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,1},
	{0,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1},
	{0,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1},
	{0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1},
	{0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1},
	{0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,1},
	{0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,1},
	{0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,1},
	{0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,1},
	{0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,1},
	{0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,1},
	{0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,1},
	{0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,1},
	{0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,1},
	{0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
	{0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1}
};

int speedo(int n)
{
	int moveSpeedValue;
	
	moveSpeedValue = map[n].ch_speed - map[n].ch_movespd;
	if (moveSpeedValue < 0)
	{
		moveSpeedValue = 0;
	}
	if (moveSpeedValue > 299)
	{
		moveSpeedValue = 299;
	}
	return speedtab[moveSpeedValue][ctick];
}

int speedoMisc(int n)
{
	int miscSpeedValue;
	
	miscSpeedValue = map[n].ch_speed;
	
	switch(map[n].ch_stat_off)
	{
		// 0, 5, 6 == Attack
		case    0:
		case    5:
		case    6:
			miscSpeedValue -= map[n].ch_atkspd;
			if (miscSpeedValue < 0) 
			{
				miscSpeedValue = 0;
			}
			if (miscSpeedValue > 299)
			{
				miscSpeedValue = 299;
			}
			return(speedtab[miscSpeedValue][ctick]);
			
		// 9 == Use skill, mostly casting
		case    9:
			miscSpeedValue  = map[n].ch_speed*5/4;
			miscSpeedValue -= map[n].ch_castspd*3/2;
			if (miscSpeedValue < 0) 
			{
				miscSpeedValue = 0;
			}
			if (miscSpeedValue > 299)
			{
				miscSpeedValue = 299;
			}
			return(speedtab[miscSpeedValue][ctick]);
			
		// Default - Shouldn't happen but here as a redundancy
		default:
			if (miscSpeedValue < 0) 
			{
				miscSpeedValue = 0;
			}
			if (miscSpeedValue > 299)
			{
				miscSpeedValue = 299;
			}
			return(speedtab[miscSpeedValue][ctick]);
	}
}

int speedstep(int n,int d,int s,int update)
{
	int hard_step;
	int soft_step;
	int total_step;
	int speed;
	int dist;
	int z,m;

	speed=map[n].ch_speed - map[n].ch_movespd;
	if (speed < 0) speed = 0;
	if (speed > 299) speed = 299;
	
	hard_step=map[n].ch_status-d;

	if (!update) return 32*hard_step/s;

	z=ctick;
	soft_step=0;
	m=hard_step;

	while (m) {
		z--;
		if (z<0) z=199;	// ctick extended from 20 to 24 to 200
		soft_step++;
		if (speedtab[speed][z])	m--;
	}
	while (1) {
		z--;
		if (z<0) z=199;	// ctick extended from 20 to 24 to 200
		if (speedtab[speed][z])	break;
		soft_step++;
	}

	z=ctick;
	total_step=soft_step;
	m=s-hard_step;

	while (1) {
		if (speedtab[speed][z])	m--;
		if (m<1) break;
		z++;
		if (z>199) z=0;	// ctick extended from 20 to 24 to 200
		total_step++;
	}
	dist=32*(soft_step)/(total_step+1);

	return dist;
}
//  0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10
static int stattab[]={ 0, 1, 1, 6, 6, 2, 3, 4, 5, 7, 4};

#define do_idle(ani,sprite)  (sprite==22480 ? ani : 0)

int eng_char(int n)
{
	int tmp,update=1;

	if (map[n].flags&STUNNED) update=0;

	switch (map[n].ch_status) 
	{
		case   0:	// idle up
			map[n].obj_xoff=0; map[n].obj_yoff=0;
			map[n].idle_ani++; if (map[n].idle_ani>7) map[n].idle_ani=0;
			tmp=SPF_IDLE_UP+do_idle(map[n].idle_ani,map[n].ch_sprite);
			break;
		case   1:	// idle down
			map[n].obj_xoff=0; map[n].obj_yoff=0;
			if (speedo(n) && update) { map[n].idle_ani++; if (map[n].idle_ani>7) map[n].idle_ani=0; }
			tmp=SPF_IDLE_DOWN+do_idle(map[n].idle_ani,map[n].ch_sprite);
			break;
		case   2:	// idle left
			map[n].obj_xoff=0; map[n].obj_yoff=0;
			if (speedo(n) && update) { map[n].idle_ani++; if (map[n].idle_ani>7) map[n].idle_ani=0; }
			tmp=SPF_IDLE_LEFT+do_idle(map[n].idle_ani,map[n].ch_sprite);
			break;
		case   3:	// idle right
			map[n].obj_xoff=0; map[n].obj_yoff=0;
			if (speedo(n) && update) { map[n].idle_ani++; if (map[n].idle_ani>7) map[n].idle_ani=0; }
			tmp=SPF_IDLE_RIGHT+do_idle(map[n].idle_ani,map[n].ch_sprite);
			break;
		case   4:	// idle left-up
			map[n].obj_xoff=0; map[n].obj_yoff=0;
			if (speedo(n) && update) { map[n].idle_ani++; if (map[n].idle_ani>7) map[n].idle_ani=0; }
			tmp=SPF_IDLE_LEFTUP+do_idle(map[n].idle_ani,map[n].ch_sprite);
			break;
		case   5:	// idle left-down
			map[n].obj_xoff=0; map[n].obj_yoff=0;
			if (speedo(n) && update) { map[n].idle_ani++; if (map[n].idle_ani>7) map[n].idle_ani=0; }
			tmp=SPF_IDLE_LEFTDOWN+do_idle(map[n].idle_ani,map[n].ch_sprite);
			break;
		case   6:	// idle right-up
			map[n].obj_xoff=0; map[n].obj_yoff=0;
			if (speedo(n) && update) { map[n].idle_ani++; if (map[n].idle_ani>7) map[n].idle_ani=0; }
			tmp=SPF_IDLE_RIGHTUP+do_idle(map[n].idle_ani,map[n].ch_sprite);
			break;
		case   7:	// idle right-down
			map[n].obj_xoff=0; map[n].obj_yoff=0;
			if (speedo(n) && update) { map[n].idle_ani++; if (map[n].idle_ani>7) map[n].idle_ani=0; }
			tmp=SPF_IDLE_RIGHTDOWN+do_idle(map[n].idle_ani,map[n].ch_sprite);
			break;
		
		case  16:	// walk up
		case  17:
		case  18:
		case  19:
		case  20:
		case  21: 
			map[n].obj_xoff=-speedstep(n,16,8,update)/2;
			map[n].obj_yoff=speedstep(n,16,8,update)/4;
			tmp=(map[n].ch_status-16)+SPF_WALK_UP;
			if (speedo(n) && update) map[n].ch_status+=speedo(n);
			break;
		case  22: 
			map[n].obj_xoff=-speedstep(n,16,8,update)/2;
			map[n].obj_yoff=speedstep(n,16,8,update)/4;
			tmp=(map[n].ch_status-16)+SPF_WALK_UP;
			if (speedo(n)==1 && update) map[n].ch_status++;
			else if (speedo(n)>1 && update) map[n].ch_status=16;
			break;
		case  23:   
			map[n].obj_xoff=-speedstep(n,16,8,update)/2;
			map[n].obj_yoff=speedstep(n,16,8,update)/4;
			tmp=(map[n].ch_status-16)+SPF_WALK_UP;
			if (speedo(n) && update) map[n].ch_status=16+((speedo(n)>1)?1:0);
			break;
		
		case  24:	// walk down
		case  25:
		case  26:
		case  27:
		case  28:
		case  29: 
			map[n].obj_xoff=speedstep(n,24,8,update)/2;
			map[n].obj_yoff=-speedstep(n,24,8,update)/4;
			tmp=(map[n].ch_status-24)+SPF_WALK_DOWN;
			if (speedo(n) && update) map[n].ch_status+=speedo(n);
			break;
		case  30: 
			map[n].obj_xoff=speedstep(n,24,8,update)/2;
			map[n].obj_yoff=-speedstep(n,24,8,update)/4;
			tmp=(map[n].ch_status-24)+SPF_WALK_DOWN;
			if (speedo(n)==1 && update) map[n].ch_status++;
			else if (speedo(n)>1 && update) map[n].ch_status=24;
			break;
		case  31:   
			map[n].obj_xoff=speedstep(n,24,8,update)/2;
			map[n].obj_yoff=-speedstep(n,24,8,update)/4;
			tmp=(map[n].ch_status-24)+SPF_WALK_DOWN;
			if (speedo(n) && update) map[n].ch_status=24+((speedo(n)>1)?1:0);
			break;
		
		case  32:	// walk left
		case  33:
		case  34:
		case  35:
		case  36:
		case  37: 
			map[n].obj_xoff=-speedstep(n,32,8,update)/2;
			map[n].obj_yoff=-speedstep(n,32,8,update)/4;
			tmp=(map[n].ch_status-32)+SPF_WALK_LEFT;
			if (speedo(n) && update) map[n].ch_status+=speedo(n);
			break;
		case  38: 
			map[n].obj_xoff=-speedstep(n,32,8,update)/2;
			map[n].obj_yoff=-speedstep(n,32,8,update)/4;
			tmp=(map[n].ch_status-32)+SPF_WALK_LEFT;
			if (speedo(n)==1 && update) map[n].ch_status++;
			else if (speedo(n)>1 && update) map[n].ch_status=32;
			break;
		case  39:   
			map[n].obj_xoff=-speedstep(n,32,8,update)/2;
			map[n].obj_yoff=-speedstep(n,32,8,update)/4;
			tmp=(map[n].ch_status-32)+SPF_WALK_LEFT;
			if (speedo(n) && update) map[n].ch_status=32+((speedo(n)>1)?1:0);
			break;
		
		case  40:	// walk right
		case  41:
		case  42:
		case  43:
		case  44:
		case  45: 
			map[n].obj_xoff=speedstep(n,40,8,update)/2;
			map[n].obj_yoff=speedstep(n,40,8,update)/4;
			tmp=(map[n].ch_status-40)+SPF_WALK_RIGHT;
			if (speedo(n) && update) map[n].ch_status+=speedo(n);
			break;
		case  46: 
			map[n].obj_xoff=speedstep(n,40,8,update)/2;
			map[n].obj_yoff=speedstep(n,40,8,update)/4;
			tmp=(map[n].ch_status-40)+SPF_WALK_RIGHT;
			if (speedo(n)==1 && update) map[n].ch_status++;
			else if (speedo(n)>1 && update) map[n].ch_status=40;
			break;
		case  47:   
			map[n].obj_xoff=speedstep(n,40,8,update)/2;
			map[n].obj_yoff=speedstep(n,40,8,update)/4;
			tmp=(map[n].ch_status-40)+SPF_WALK_RIGHT;
			if (speedo(n) && update) map[n].ch_status=40+((speedo(n)>1)?1:0);
			break;
		
		case  48:	// left+up
		case  49:
		case  50:
		case  51:
		case  52:
		case  53:
		case  54:
		case  55:
		case  56:
		case  57:   
			map[n].obj_xoff=-speedstep(n,48,12,update);
			map[n].obj_yoff=0;
			tmp=(map[n].ch_status-48)*8/12+SPF_WALK_LEFTUP;
			if (speedo(n) && update) map[n].ch_status+=speedo(n);
			break;
		case  58:  
			map[n].obj_xoff=-speedstep(n,48,12,update);
			map[n].obj_yoff=0;
			tmp=(map[n].ch_status-48)*8/12+SPF_WALK_LEFTUP;
			if (speedo(n)==1 && update) map[n].ch_status++;
			else if (speedo(n)>1 && update) map[n].ch_status=48;
			break;
		case  59:   
			map[n].obj_xoff=-speedstep(n,48,12,update);
			map[n].obj_yoff=0;
			tmp=(map[n].ch_status-48)*8/12+SPF_WALK_LEFTUP;
			if (speedo(n) && update) map[n].ch_status=48+((speedo(n)>1)?1:0);
			break;
		
		case  60:	// left+down
		case  61:
		case  62:
		case  63:
		case  64:
		case  65:
		case  66:
		case  67:
		case  68:
		case  69:   
			map[n].obj_xoff=0;
			map[n].obj_yoff=-speedstep(n,60,12,update)/2;
			tmp=(map[n].ch_status-60)*8/12+SPF_WALK_LEFTDOWN;
			if (speedo(n) && update) map[n].ch_status+=speedo(n);
			break;
		case  70:   
			map[n].obj_xoff=0;
			map[n].obj_yoff=-speedstep(n,60,12,update)/2;
			tmp=(map[n].ch_status-60)*8/12+SPF_WALK_LEFTDOWN;
			if (speedo(n)==1 && update) map[n].ch_status++;
			else if (speedo(n)>1 && update) map[n].ch_status=60;
			break;
		case  71:   
			map[n].obj_xoff=0;
			map[n].obj_yoff=-speedstep(n,60,12,update)/2;
			tmp=(map[n].ch_status-60)*8/12+SPF_WALK_LEFTDOWN;
			if (speedo(n) && update) map[n].ch_status=60+((speedo(n)>1)?1:0);
			break;
		
		case  72:	// right+up
		case  73:
		case  74:
		case  75:
		case  76:
		case  77:
		case  78:
		case  79:
		case  80:
		case  81:   
			map[n].obj_xoff=0;
			map[n].obj_yoff=speedstep(n,72,12,update)/2;
			tmp=(map[n].ch_status-72)*8/12+SPF_WALK_RIGHTUP;
			if (speedo(n) && update) map[n].ch_status+=speedo(n);
			break;
		case  82:   
			map[n].obj_xoff=0;
			map[n].obj_yoff=speedstep(n,72,12,update)/2;
			tmp=(map[n].ch_status-72)*8/12+SPF_WALK_RIGHTUP;
			if (speedo(n)==1 && update) map[n].ch_status++;
			else if (speedo(n)>1 && update) map[n].ch_status=72;
			break;
		case  83:   
			map[n].obj_xoff=0;
			map[n].obj_yoff=speedstep(n,72,12,update)/2;
			tmp=(map[n].ch_status-72)*8/12+SPF_WALK_RIGHTUP;
			if (speedo(n) && update) map[n].ch_status=72+((speedo(n)>1)?1:0);
			break;
		
		case  84:	// right+down
		case  85:
		case  86:
		case  87:
		case  88:
		case  89:
		case  90:
		case  91:
		case  92:
		case  93:   
			map[n].obj_xoff=speedstep(n,84,12,update);
			map[n].obj_yoff=0;
			tmp=(map[n].ch_status-84)*8/12+SPF_WALK_RIGHTDOWN;
			if (speedo(n) && update) map[n].ch_status+=speedo(n);
			break;
		case  94:   
			map[n].obj_xoff=speedstep(n,84,12,update);
			map[n].obj_yoff=0;
			tmp=(map[n].ch_status-84)*8/12+SPF_WALK_RIGHTDOWN;
			if (speedo(n)==1 && update) map[n].ch_status++;
			else if (speedo(n)>1 && update) map[n].ch_status=84;
			break;
		case  95:   
			map[n].obj_xoff=speedstep(n,84,12,update);
			map[n].obj_yoff=0;
			tmp=(map[n].ch_status-84)*8/12+SPF_WALK_RIGHTDOWN;
			if (speedo(n) && update) map[n].ch_status=84+((speedo(n)>1)?1:0);
			break;
		
		case  96:	// turn up to left-up
		case  97: 
			map[n].obj_xoff=0; map[n].obj_yoff=0;
			tmp=(map[n].ch_status-96)+SPF_TURN_UPLEFTUP;
			if (speedo(n) && update) map[n].ch_status+=speedo(n);
			break;
		case  98: 
			map[n].obj_xoff=0; map[n].obj_yoff=0;
			tmp=(map[n].ch_status-96)+SPF_TURN_UPLEFTUP;
			if (speedo(n)==1 && update) map[n].ch_status++;
			else if (speedo(n)>1 && update) map[n].ch_status=96;
			break;
		case  99: 
			map[n].obj_xoff=0; map[n].obj_yoff=0;
			tmp=(map[n].ch_status-96)+SPF_TURN_UPLEFTUP;
			if (speedo(n) && update) map[n].ch_status=96+((speedo(n)>1)?1:0);
			break;
		
		case 100:	// turn left-up to up
		case 101:  
			map[n].obj_xoff=0; map[n].obj_yoff=0;
			tmp=(map[n].ch_status-100)+SPF_TURN_LEFTUPUP;
			if (speedo(n) && update) map[n].ch_status+=speedo(n);
			break;
		case 102:  
			map[n].obj_xoff=0; map[n].obj_yoff=0;
			tmp=(map[n].ch_status-100)+SPF_TURN_LEFTUPUP;
			if (speedo(n)==1 && update) map[n].ch_status++;
			else if (speedo(n)>1 && update) map[n].ch_status=100;
			break;
		case 103:   
			map[n].obj_xoff=0; map[n].obj_yoff=0;
			tmp=(map[n].ch_status-100)+SPF_TURN_LEFTUPUP;
			if (speedo(n) && update) map[n].ch_status=100+((speedo(n)>1)?1:0);
			break;
		
		case 104:	// turn up to right-up
		case 105:   
			map[n].obj_xoff=0; map[n].obj_yoff=0;
			tmp=(map[n].ch_status-104)+SPF_TURN_UPRIGHTUP;
			if (speedo(n) && update) map[n].ch_status+=speedo(n);
			break;
		case 106:   
			map[n].obj_xoff=0; map[n].obj_yoff=0;
			tmp=(map[n].ch_status-104)+SPF_TURN_UPRIGHTUP;
			if (speedo(n)==1 && update) map[n].ch_status++;
			else if (speedo(n)>1 && update) map[n].ch_status=104;
			break;
		case 107:   
			tmp=(map[n].ch_status-104)+SPF_TURN_UPRIGHTUP;
			if (speedo(n) && update) map[n].ch_status=104+((speedo(n)>1)?1:0);
			break;
		
		case 108:	// turn right-up to right
		case 109:   
			map[n].obj_xoff=0; map[n].obj_yoff=0;
			tmp=(map[n].ch_status-108)+SPF_TURN_RIGHTUPRIGHT;
			if (speedo(n) && update) map[n].ch_status+=speedo(n);
			break;
		case 110:   
			map[n].obj_xoff=0; map[n].obj_yoff=0;
			tmp=(map[n].ch_status-108)+SPF_TURN_RIGHTUPRIGHT;
			if (speedo(n)==1 && update) map[n].ch_status++;
			else if (speedo(n)>1 && update) map[n].ch_status=108;
			break;
		case 111:   
			tmp=(map[n].ch_status-108)+SPF_TURN_RIGHTUPRIGHT;
			if (speedo(n) && update) map[n].ch_status=108+((speedo(n)>1)?1:0);
			break;
		
		case 112:	// turn down to left-down
		case 113: 
			map[n].obj_xoff=0; map[n].obj_yoff=0;
			tmp=(map[n].ch_status-112)+SPF_TURN_DOWNLEFTDOWN;
			if (speedo(n) && update) map[n].ch_status+=speedo(n);
			break;
		case 114: 
			map[n].obj_xoff=0; map[n].obj_yoff=0;
			tmp=(map[n].ch_status-112)+SPF_TURN_DOWNLEFTDOWN;
			if (speedo(n)==1 && update) map[n].ch_status++;
			else if (speedo(n)>1 && update) map[n].ch_status=112;
			break;
		case 115: 
			tmp=(map[n].ch_status-112)+SPF_TURN_DOWNLEFTDOWN;
			if (speedo(n) && update) map[n].ch_status=112+((speedo(n)>1)?1:0);
			break;
		
		case 116:	// turn left-down to left
		case 117:  
			map[n].obj_xoff=0; map[n].obj_yoff=0;
			tmp=(map[n].ch_status-116)+SPF_TURN_LEFTDOWNLEFT;
			if (speedo(n) && update) map[n].ch_status+=speedo(n);
			break;
		case 118:   
			map[n].obj_xoff=0; map[n].obj_yoff=0;
			tmp=(map[n].ch_status-116)+SPF_TURN_LEFTDOWNLEFT;
			if (speedo(n)==1 && update) map[n].ch_status++;
			else if (speedo(n)>1 && update) map[n].ch_status=116;
			break;
		case 119:   
			map[n].obj_xoff=0; map[n].obj_yoff=0;
			tmp=(map[n].ch_status-116)+SPF_TURN_LEFTDOWNLEFT;
			if (speedo(n) && update) map[n].ch_status=116+((speedo(n)>1)?1:0);
			break;
		
		case 120:	// turn down to right-down
		case 121:   
			map[n].obj_xoff=0; map[n].obj_yoff=0;
			tmp=(map[n].ch_status-120)+SPF_TURN_DOWNRIGHTDOWN;
			if (speedo(n) && update) map[n].ch_status+=speedo(n);
			break;
		case 122:   
			map[n].obj_xoff=0; map[n].obj_yoff=0;
			tmp=(map[n].ch_status-120)+SPF_TURN_DOWNRIGHTDOWN;
			if (speedo(n)==1 && update) map[n].ch_status++;
			else if (speedo(n)>1 && update) map[n].ch_status=120;
			break;
		case 123:   
			tmp=(map[n].ch_status-120)+SPF_TURN_DOWNRIGHTDOWN;
			if (speedo(n) && update) map[n].ch_status=120+((speedo(n)>1)?1:0);
			break;
		
		case 124:	// turn right-down to down
		case 125:   
			map[n].obj_xoff=0; map[n].obj_yoff=0;
			tmp=(map[n].ch_status-124)+SPF_TURN_RIGHTDOWNDOWN;
			if (speedo(n) && update) map[n].ch_status+=speedo(n);
			break;
		case 126:   
			map[n].obj_xoff=0; map[n].obj_yoff=0;
			tmp=(map[n].ch_status-124)+SPF_TURN_RIGHTDOWNDOWN;
			if (speedo(n)==1 && update) map[n].ch_status++;
			else if (speedo(n)>1 && update) map[n].ch_status=124;
			break;
		case 127:   
			map[n].obj_xoff=0; map[n].obj_yoff=0;
			tmp=(map[n].ch_status-124)+SPF_TURN_RIGHTDOWNDOWN;
			if (speedo(n) && update) map[n].ch_status=124+((speedo(n)>1)?1:0);
			break;
		
		case 128:	// turn left to left-up
		case 129:   
			map[n].obj_xoff=0; map[n].obj_yoff=0;
			tmp=(map[n].ch_status-128)+SPF_TURN_LEFTLEFTUP;
			if (speedo(n) && update) map[n].ch_status+=speedo(n);
			break;
		case 130:   
			map[n].obj_xoff=0; map[n].obj_yoff=0;
			tmp=(map[n].ch_status-128)+SPF_TURN_LEFTLEFTUP;
			if (speedo(n)==1 && update) map[n].ch_status++;
			else if (speedo(n)>1 && update) map[n].ch_status=128;
			break;
		case 131:   
			tmp=(map[n].ch_status-128)+SPF_TURN_LEFTLEFTUP;
			if (speedo(n) && update) map[n].ch_status=128+((speedo(n)>1)?1:0);
			break;
		
		case 132:	// turn left-up to left
		case 133:   
			map[n].obj_xoff=0; map[n].obj_yoff=0;
			tmp=(map[n].ch_status-132)+SPF_TURN_LEFTUPLEFT;
			if (speedo(n) && update) map[n].ch_status+=speedo(n);
			break;
		case 134:   
			map[n].obj_xoff=0; map[n].obj_yoff=0;
			tmp=(map[n].ch_status-132)+SPF_TURN_LEFTUPLEFT;
			if (speedo(n)==1 && update) map[n].ch_status++;
			else if (speedo(n)>1 && update) map[n].ch_status=132;
			break;
		case 135:   
			map[n].obj_xoff=0; map[n].obj_yoff=0;
			tmp=(map[n].ch_status-132)+SPF_TURN_LEFTUPLEFT;
			if (speedo(n) && update) map[n].ch_status=132+((speedo(n)>1)?1:0);
			break;
		
		case 136:	// turn left to left-down
		case 137:   
			map[n].obj_xoff=0; map[n].obj_yoff=0;
			tmp=(map[n].ch_status-136)+SPF_TURN_LEFTLEFTDOWN;
			if (speedo(n) && update) map[n].ch_status+=speedo(n);
			break;
		case 138:   
			map[n].obj_xoff=0; map[n].obj_yoff=0;
			tmp=(map[n].ch_status-136)+SPF_TURN_LEFTLEFTDOWN;
			if (speedo(n)==1 && update) map[n].ch_status++;
			else if (speedo(n)>1 && update) map[n].ch_status=136;
			break;
		case 139:   
			tmp=(map[n].ch_status-136)+SPF_TURN_LEFTLEFTDOWN;
			if (speedo(n) && update) map[n].ch_status=136+((speedo(n)>1)?1:0);
			break;
		
		case 140:	// turn left-down to down
		case 141:   
			map[n].obj_xoff=0; map[n].obj_yoff=0;
			tmp=(map[n].ch_status-140)+SPF_TURN_LEFTDOWNDOWN;
			if (speedo(n) && update) map[n].ch_status+=speedo(n);
			break;
		case 142:   
			map[n].obj_xoff=0; map[n].obj_yoff=0;
			tmp=(map[n].ch_status-140)+SPF_TURN_LEFTDOWNDOWN;
			if (speedo(n)==1 && update) map[n].ch_status++;
			else if (speedo(n)>1 && update) map[n].ch_status=140;
			break;
		case 143:   
			map[n].obj_xoff=0; map[n].obj_yoff=0;
			tmp=(map[n].ch_status-140)+SPF_TURN_LEFTDOWNDOWN;
			if (speedo(n) && update) map[n].ch_status=140+((speedo(n)>1)?1:0);
			break;
		
		case 144:	// turn right to right-up
		case 145:   
			map[n].obj_xoff=0; map[n].obj_yoff=0;
			tmp=(map[n].ch_status-144)+SPF_TURN_RIGHTRIGHTUP;
			if (speedo(n) && update) map[n].ch_status+=speedo(n);
			break;
		case 146:   
			map[n].obj_xoff=0; map[n].obj_yoff=0;
			tmp=(map[n].ch_status-144)+SPF_TURN_RIGHTRIGHTUP;
			if (speedo(n)==1 && update) map[n].ch_status++;
			else if (speedo(n)>1 && update) map[n].ch_status=144;
			break;
		case 147:   
			tmp=(map[n].ch_status-144)+SPF_TURN_RIGHTRIGHTUP;
			if (speedo(n) && update) map[n].ch_status=144+((speedo(n)>1)?1:0);
			break;
		
		case 148:	// turn right-up to up
		case 149:   
			map[n].obj_xoff=0; map[n].obj_yoff=0;
			tmp=(map[n].ch_status-148)+SPF_TURN_RIGHTUPUP;
			if (speedo(n) && update) map[n].ch_status+=speedo(n);
			break;
		case 150:   
			map[n].obj_xoff=0; map[n].obj_yoff=0;
			tmp=(map[n].ch_status-148)+SPF_TURN_RIGHTUPUP;
			if (speedo(n)==1 && update) map[n].ch_status++;
			else if (speedo(n)>1 && update) map[n].ch_status=148;
			break;
		case 151:   
			map[n].obj_xoff=0; map[n].obj_yoff=0;
			tmp=(map[n].ch_status-148)+SPF_TURN_RIGHTUPUP;
			if (speedo(n) && update) map[n].ch_status=148+((speedo(n)>1)?1:0);
			break;
		
		case 152:	// turn right to right-down
		case 153:   
			map[n].obj_xoff=0; map[n].obj_yoff=0;
			tmp=(map[n].ch_status-152)+SPF_TURN_RIGHTRIGHTDOWN;
			if (speedo(n) && update) map[n].ch_status+=speedo(n);
			break;
		case 154:   
			map[n].obj_xoff=0; map[n].obj_yoff=0;
			tmp=(map[n].ch_status-152)+SPF_TURN_RIGHTRIGHTDOWN;
			if (speedo(n)==1 && update) map[n].ch_status++;
			else if (speedo(n)>1 && update) map[n].ch_status=152;
			break;
		case 155:   
			tmp=(map[n].ch_status-152)+SPF_TURN_RIGHTRIGHTDOWN;
			if (speedo(n) && update) map[n].ch_status=152+((speedo(n)>1)?1:0);
			break;
		
		case 156:	// turn right-down to down
		case 157:   
			map[n].obj_xoff=0; map[n].obj_yoff=0;
			tmp=(map[n].ch_status-156)+SPF_TURN_RIGHTDOWNRIGHT;
			if (speedo(n) && update) map[n].ch_status+=speedo(n);
			break;
		case 158:   
			map[n].obj_xoff=0; map[n].obj_yoff=0;
			tmp=(map[n].ch_status-156)+SPF_TURN_RIGHTDOWNRIGHT;
			if (speedo(n)==1 && update) map[n].ch_status++;
			else if (speedo(n)>1 && update) map[n].ch_status=156;
			break;
		case 159:   
			map[n].obj_xoff=0; map[n].obj_yoff=0;
			tmp=(map[n].ch_status-156)+SPF_TURN_RIGHTDOWNRIGHT;
			if (speedo(n) && update) map[n].ch_status=156+((speedo(n)>1)?1:0);
			break;
		
		case 160:	// misc up
		case 161:
		case 162:
		case 163:
		case 164:
		case 165:   
			map[n].obj_xoff=0; map[n].obj_yoff=0;
			tmp=(map[n].ch_status-160)+SPF_MISC_UP+((int)(stattab[map[n].ch_stat_off])<<5);
			if (speedoMisc(n) && update) map[n].ch_status+=speedoMisc(n);
			break;
		case 166:   
			map[n].obj_xoff=0; map[n].obj_yoff=0;
			tmp=(map[n].ch_status-160)+SPF_MISC_UP+((int)(stattab[map[n].ch_stat_off])<<5);
			if (speedoMisc(n)==1 && update) map[n].ch_status++;
			else if (speedoMisc(n)>1 && update) map[n].ch_status=160;
			break;
		case 167:   
			map[n].obj_xoff=0; map[n].obj_yoff=0;
			tmp=(map[n].ch_status-160)+SPF_MISC_UP+((int)(stattab[map[n].ch_stat_off])<<5);
			if (speedoMisc(n) && update) map[n].ch_status=160+((speedoMisc(n)>1)?1:0);
			break;
		
		case 168:	// misc down
		case 169:
		case 170:
		case 171:
		case 172:
		case 173:   
			map[n].obj_xoff=0; map[n].obj_yoff=0;
			tmp=(map[n].ch_status-168)+SPF_MISC_DOWN+((int)(stattab[map[n].ch_stat_off])<<5);
			if (speedoMisc(n) && update) map[n].ch_status+=speedoMisc(n);
			break;
		case 174:   
			map[n].obj_xoff=0; map[n].obj_yoff=0;
			tmp=(map[n].ch_status-168)+SPF_MISC_DOWN+((int)(stattab[map[n].ch_stat_off])<<5);
			if (speedoMisc(n)==1 && update) map[n].ch_status++;
			else if (speedoMisc(n)>1 && update) map[n].ch_status=168;
			break;
		case 175:   
			map[n].obj_xoff=0; map[n].obj_yoff=0;
			tmp=(map[n].ch_status-168)+SPF_MISC_DOWN+((int)(stattab[map[n].ch_stat_off])<<5);
			if (speedoMisc(n) && update) map[n].ch_status=168+((speedoMisc(n)>1)?1:0);
			break;
		
		case 176:	// misc left
		case 177:
		case 178:
		case 179:
		case 180:
		case 181:   
			map[n].obj_xoff=0; map[n].obj_yoff=0;
			tmp=(map[n].ch_status-176)+SPF_MISC_LEFT+((int)(stattab[map[n].ch_stat_off])<<5);
			if (speedoMisc(n) && update) map[n].ch_status+=speedoMisc(n);
			break;
		case 182:   
			map[n].obj_xoff=0; map[n].obj_yoff=0;
			tmp=(map[n].ch_status-176)+SPF_MISC_LEFT+((int)(stattab[map[n].ch_stat_off])<<5);
			if (speedoMisc(n)==1 && update) map[n].ch_status++;
			else if (speedoMisc(n)>1 && update) map[n].ch_status=176;
			break;
		case 183:   
			map[n].obj_xoff=0; map[n].obj_yoff=0;
			tmp=(map[n].ch_status-176)+SPF_MISC_LEFT+((int)(stattab[map[n].ch_stat_off])<<5);
			if (speedoMisc(n) && update) map[n].ch_status=176+((speedoMisc(n)>1)?1:0);
			break;
		
		case 184:	// misc right
		case 185:
		case 186:
		case 187:
		case 188:
		case 189:   
			map[n].obj_xoff=0; map[n].obj_yoff=0;
			tmp=(map[n].ch_status-184)+SPF_MISC_RIGHT+((int)(stattab[map[n].ch_stat_off])<<5);
			if (speedoMisc(n) && update) map[n].ch_status+=speedoMisc(n);
			break;
		case 190:   
			map[n].obj_xoff=0; map[n].obj_yoff=0;
			tmp=(map[n].ch_status-184)+SPF_MISC_RIGHT+((int)(stattab[map[n].ch_stat_off])<<5);
			if (speedoMisc(n)==1 && update) map[n].ch_status++;
			else if (speedoMisc(n)>1 && update) map[n].ch_status=184;
			break;
		case 191:   
			map[n].obj_xoff=0; map[n].obj_yoff=0;
			tmp=(map[n].ch_status-184)+SPF_MISC_RIGHT+((int)(stattab[map[n].ch_stat_off])<<5);
			if (speedoMisc(n) && update) map[n].ch_status=184+((speedoMisc(n)>1)?1:0);
			break;

		default:        
			xlog(0,"Unknown ch_status %d",map[n].ch_status);
			return 0;
	}
	
	return tmp;
}

int eng_item(int n)
{
	switch (map[n].it_status) 
	{
		case 0: return map[n].it_sprite;
		case 1: return map[n].it_sprite;

			// four sprite animation, 2-step
		case 2: if (speedtab[200][ctick]) map[n].it_status++;	// adjusted these speedtab values from 10 to 24 to 200
			return map[n].it_sprite;

		case 3: if (speedtab[200][ctick]) map[n].it_status++;
			return map[n].it_sprite+2;

		case 4: if (speedtab[200][ctick]) map[n].it_status++;
			return map[n].it_sprite+4;

		case 5: if (speedtab[200][ctick]) map[n].it_status=2;
			return map[n].it_sprite+6;

			// two sprite animation, 1-step
		case 6: if (speedtab[200][ctick]) map[n].it_status++;
			return map[n].it_sprite;

		case 7: if (speedtab[200][ctick]) map[n].it_status=6;
			return map[n].it_sprite+1;

			// eight sprite animation, 1-step
		case 8: if (speedtab[200][ctick]) map[n].it_status++;
			return map[n].it_sprite;

		case 9: if (speedtab[200][ctick]) map[n].it_status++;
			return map[n].it_sprite+1;

		case 10: if (speedtab[200][ctick]) map[n].it_status++;
			return map[n].it_sprite+2;

		case 11: if (speedtab[200][ctick]) map[n].it_status++;
			return map[n].it_sprite+3;

		case 12: if (speedtab[200][ctick]) map[n].it_status++;
			return map[n].it_sprite+4;

		case 13: if (speedtab[200][ctick]) map[n].it_status++;
			return map[n].it_sprite+5;

		case 14: if (speedtab[200][ctick]) map[n].it_status++;
			return map[n].it_sprite+6;

		case 15: if (speedtab[200][ctick]) map[n].it_status=8;
			return map[n].it_sprite+7;

			// five sprite animation, 1-step, random
		case 16: if (speedtab[200][ctick]) map[n].it_status++;
			return map[n].it_sprite;

		case 17: if (speedtab[200][ctick]) map[n].it_status++;
			return map[n].it_sprite+1;

		case 18: if (speedtab[200][ctick]) map[n].it_status++;
			return map[n].it_sprite+2;

		case 19: if (speedtab[200][ctick]) map[n].it_status++;
			return map[n].it_sprite+3;

		case 20: if (speedtab[200][ctick]) map[n].it_status=16;
			return map[n].it_sprite+4;

		case 21: 
			return map[n].it_sprite+(ticker&63);

		default: xlog(0,"Unknown it_status");
			return map[n].it_sprite;
	}
}

void engine_tick(void)
{
	int n,tmp;

	ticker++;

	for (n=0; n<screen_renderdist*screen_renderdist; n++) {
		map[n].back=0;
		map[n].obj1=0;
		map[n].obj2=0;
		map[n].ovl_xoff=0;
		map[n].ovl_yoff=0;
	}

	for (n=0; n<screen_renderdist*screen_renderdist; n++) {

		map[n].back=map[n].ba_sprite;

		// item
		if (map[n].it_sprite) {
			tmp=eng_item(n);
			map[n].obj1=tmp;
		}

		// character
		if (map[n].ch_sprite) {
			// eng_char has been modified to only pull the animation frame value.  -- Zarro 12/29/2021
			tmp=eng_char(n);
			map[n].obj2=map[n].ch_sprite+tmp;
			//map[n].obj2f=tmp;
		}
	}
}

void send_opt(void)
{
	static int state=0;
	unsigned char buf[16];
	int n;

	buf[0]=CL_CMD_SETUSER;

	switch (state) 
	{
		case  0: buf[1]=0; buf[2]= 0; for (n=0; n<13; n++) buf[n+3]=pdata.cname[n];    xlog(1,"Transfering user data..."); break;
		case  1: buf[1]=0; buf[2]=13; for (n=0; n<13; n++) buf[n+3]=pdata.cname[n+13]; break;
		case  2: buf[1]=0; buf[2]=26; for (n=0; n<13; n++) buf[n+3]=pdata.cname[n+26]; break;
		case  3: buf[1]=0; buf[2]=39; for (n=0; n<13; n++) buf[n+3]=pdata.cname[n+39]; break;
		case  4: buf[1]=0; buf[2]=52; for (n=0; n<13; n++) buf[n+3]=pdata.cname[n+52]; break;
		case  5: buf[1]=0; buf[2]=65; for (n=0; n<13; n++) buf[n+3]=pdata.cname[n+65]; break;

		case  6: buf[1]=1; buf[2]= 0; for (n=0; n<13; n++) buf[n+3]=pdata.desc[n];     break;
		case  7: buf[1]=1; buf[2]=13; for (n=0; n<13; n++) buf[n+3]=pdata.desc[n+13];  break;
		case  8: buf[1]=1; buf[2]=26; for (n=0; n<13; n++) buf[n+3]=pdata.desc[n+26];  break;
		case  9: buf[1]=1; buf[2]=39; for (n=0; n<13; n++) buf[n+3]=pdata.desc[n+39];  break;
		case 10: buf[1]=1; buf[2]=52; for (n=0; n<13; n++) buf[n+3]=pdata.desc[n+52];  break;
		case 11: buf[1]=1; buf[2]=65; for (n=0; n<13; n++) buf[n+3]=pdata.desc[n+65];  break;

		case 12: buf[1]=2; buf[2]= 0; for (n=0; n<13; n++) buf[n+3]=pdata.desc[n+78];  break;
		case 13: buf[1]=2; buf[2]=13; for (n=0; n<13; n++) buf[n+3]=pdata.desc[n+91];  break;
		case 14: buf[1]=2; buf[2]=26; for (n=0; n<13; n++) buf[n+3]=pdata.desc[n+104]; break;
		case 15: buf[1]=2; buf[2]=39; for (n=0; n<13; n++) buf[n+3]=pdata.desc[n+117]; break;
		case 16: buf[1]=2; buf[2]=52; for (n=0; n<13; n++) buf[n+3]=pdata.desc[n+130]; break;
		case 17: buf[1]=2; buf[2]=65; for (n=0; n<13; n++) buf[n+3]=pdata.desc[n+143];
			pdata.changed=0; save_options();
			xlog(1,"Transfer done."); break;
	}
	xsend(buf);
	state++;
}

int firstquit=0;
int wantquit=0;
int maynotquit=TICKS*5/TICKMULTI;

void cmd_exit(void)
{
	if (do_exit || !maynotquit) 
	{
		quit=1;
		return;
	}
	if (!firstquit) 
	{
		xlog(0," ");
		xlog(0,"(F12) Leaving the game without entering a tavern will cause you to take damage, and possibly lose your life. Click again if you need to leave immediately.");
		xlog(0,"A tavern will be located near where you entered the game.");
		firstquit=1;
		return;
	}

	if (!wantquit) 
	{
		cmd1(CL_CMD_EXIT,0);
		wantquit=1;
		xlog(0," ");
		xlog(0,"(F12) Exit request acknowledged. Please wait a moment...");
	}
}

int noshop=0;
extern int xmove,xxtimer;
extern int do_ticker;

void engine(void)
{
	int tmp,tick,init=0;
	int step=0,skip=0,lookstep=0,optstep=0,skipinrow=0,n,panic,xtimer=0,autosort=0;
	extern int cmd_count,tick_count;
	unsigned int t;

	skilltab=malloc(sizeof(struct skilltab)*MAXSKILL);
	for (n=0; n<MAXSKILL; n++) 
	{
		skilltab[n]=_skilltab[n];
		skilltab[n].attrib[0]=1;
	}

	init_done=1;

	t=GetTickCount();

	while (!quit) 
	{
		do_msg();
		if (wantquit && maynotquit)	maynotquit--;

		if (do_ticker && (ticker&15)==0) cmd1s(CL_CMD_CTICK,ticker);

		if (step++>16) 
		{
			pskip=100.0*(float)skip/(float)frame;
			pidle=100.0*(float)idle/(float)xtime;
            skip=frame=0;
			idle=xtime=0;
			step=0;
		}

		frame++;

		lookstep++;
		if (lookat && lookstep>QSIZE*3) 
		{
			if (lookat>=lookmax || looks[lookat].known==0)
				cmd1s(CL_CMD_AUTOLOOK,lookat);
			lookat=0;
			lookstep=0;
		}

		if (look_timer)	look_timer--;
		else show_look=0;

		if ((show_shop) && lookstep>QSIZE) 
		{
			cmd1s(CL_CMD_LOOK,shop.nr);
			lookstep=0;
		}

		optstep++;
		if (optstep>4 && pdata.changed) 
		{
			send_opt();
			optstep=0;
		}
		
		if (autosort<5) autosort++;
		if (autosort==5) 
		{
			qsort(skilltab,55,sizeof(struct skilltab),skill_cmp);
			autosort++;
		}

		if (xtime>0) xtimer--;

		if (xmove && xtimer<1) 
		{
			switch (xmove) 
			{
				case 1: cmds(CL_CMD_MOVE,map[(RENDERDIST/2-7)+screen_renderdist*RENDERDIST/2].x,map[(RENDERDIST/2-7)+screen_renderdist*RENDERDIST/2].y); break;
				case 3: cmds(CL_CMD_MOVE,map[(RENDERDIST/2+7)+screen_renderdist*RENDERDIST/2].x,map[(RENDERDIST/2+7)+screen_renderdist*RENDERDIST/2].y); break;
				case 2: cmds(CL_CMD_MOVE,map[RENDERDIST/2+screen_renderdist*(RENDERDIST/2-7)].x,map[RENDERDIST/2+screen_renderdist*(RENDERDIST/2-7)].y); break;
				case 4: cmds(CL_CMD_MOVE,map[RENDERDIST/2+screen_renderdist*(RENDERDIST/2+7)].x,map[RENDERDIST/2+screen_renderdist*(RENDERDIST/2+7)].y); break;
				default: break;
			}
			xtimer=4;
			if (xxtimer++>1000) xmove=xxtimer=0;
		}

		panic=0;
		do {
			do_msg();
			tmp=game_loop();
			panic++;
		} while (tmp && panic<8192);

		tmp=tick_do();
        if (tmp) init=1;
		if (do_exit) init=0;

		do_msg();

		if (noshop>0) 
		{
			noshop--;
			show_shop=0;
			show_wps=0;
			show_tree=0;
			show_book=0;
			show_motd=0;
			show_newp=0;
			show_tuto=0;
		}
		if (t>GetTickCount() || skipinrow>100)	// display frame only if we've got enough time
		{
			eng_display(init);
			eng_flip(t);
			skipinrow=0;
		} 
		else 
		{
			skip++; skipinrow++;
		}

		do_msg();

		if (t_size) tick=TICK*QSIZE/t_size;
		else tick=TICK;

		t+=tick; ttime+=tick; xtime+=tick;
	}
}

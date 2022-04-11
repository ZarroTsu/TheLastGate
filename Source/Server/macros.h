/*************************************************************************

   This file is part of 'Mercenaries of Astonia v2'
   Copyright (c) 1997-2001 Daniel Brockhaus (joker@astonia.com)
   All rights reserved.

 **************************************************************************/

#define max(a, b)		((a)>(b) ? (a) : (b))
#define min(a, b)		((a)<(b) ? (a) : (b))
#define RANDOM(a)		(a>0?random()%(a):0)
#define ARRAYSIZE(a)	(sizeof(a)/sizeof(a[0]))
#define sqr(a)			((a) * (a))

// Sanity checks on map locations x and y
#define SANEX(x)     	((x) >= 0 && (x) < MAPX)
#define SANEY(y)     	((y) >= 0 && (y) < MAPY)
#define SANEXY(x, y) 	(SANEX(x) && SANEY(y))

// Convert (x,y) coordinates to absolute position
#define XY2M(x, y) 		((x) + (y) * MAPX)
#define M2X(m) 			((m) % MAPX)
#define M2Y(m) 			((m) / MAPX)

// Character 'cn's absolute position
#define CHPOSM(cn) (XY2M(ch[(cn)].x,ch[(cn)].y))



/* *** ITEMS *** */

// Use flag check on a STRUCTURE ELEMENT (works for characters too)
// DB: i dont know how it might fail... but i've added some parents anyway. here and in other places.
// DB: looked fairly safe before, so no need to worry
#define IS_USED(x) ((x).used != USE_EMPTY)

// Sanity checks on item numbers
#define IS_SANEITEM(in)     ((in) > 0 && (in) < MAXITEM)
#define IS_USEDITEM(in)     (IS_USED(it[(in)]))
#define IS_SANEUSEDITEM(in) (IS_SANEITEM(in) && IS_USEDITEM(in))

#define CAN_SOULSTONE(in)	(it[(in)].flags & IF_CAN_SS)
#define CAN_ENCHANT(in)		(it[(in)].flags & IF_CAN_EN)
#define HAS_ENCHANT(in, n)	(it[(in)].enchantment == (n))

#define IS_SINBINDER(in)		(it[(in)].temp==IT_TW_SINBIND || it[(in)].orig_temp==IT_TW_SINBIND)
#define NOT_SINBINDER(in)		(it[(in)].temp!=IT_TW_SINBIND && it[(in)].orig_temp!=IT_TW_SINBIND)


/* *** TEMPLATES *** */

// Sanity checks on item templates
#define IS_SANEITEMPLATE(tn) ((tn) > 0 && (tn) < MAXTITEM)
#define IS_SANECTEMPLATE(tn) ((tn) > 0 && (tn) < MAXTCHARS)

#define IS_LONG_RESPAWN(temp) (temp==CT_RATKING || temp==CT_GREENKING || temp==CT_DREADKING || temp==CT_LIZEMPEROR || temp==CT_BSMAGE1 || temp==CT_BSMAGE2 || temp==CT_BSMAGE3)

/* *** CHARACTERS *** */

// Sanity checks on character numbers
#define IS_SANECHAR(cn)     ((cn) > 0 && (cn) < MAXCHARS)
#define IS_LIVINGCHAR(cn)   (IS_SANECHAR(cn) && ch[(cn)].used != USE_EMPTY)
#define IS_ACTIVECHAR(cn)   (IS_SANECHAR(cn) && ch[(cn)].used == USE_ACTIVE)
#define IS_USEDCHAR(cn)     (IS_USED(ch[(cn)]))
#define IS_SANEUSEDCHAR(cn) (IS_SANECHAR(cn) && IS_USEDCHAR(cn))

// flag checks
#define IS_PLAYER(cn)          ((ch[(cn)].flags & CF_PLAYER) != 0)
#define IS_STAFF(cn)           ((ch[(cn)].flags & CF_STAFF) != 0)
#define IS_GOD(cn)             ((ch[(cn)].flags & CF_GOD) != 0)
#define IS_USURP(cn)           ((ch[(cn)].flags & CF_USURP) != 0)
#define IS_IMP(cn)             ((ch[(cn)].flags & CF_IMP) != 0)
#define IS_QM(cn)              ((ch[(cn)].flags & (CF_IMP|CF_USURP)) != 0)
#define IS_IGNORING_SPELLS(cn) ((ch[(cn)].flags & CF_SPELLIGNORE) != 0)
#define IS_CCP(cn)             ((ch[(cn)].flags & CF_CCP) != 0)
#define IS_BUILDING(cn)        ((ch[(cn)].flags & CF_BUILDMODE) != 0)

// special character group checks
#define IS_COMPANION(cn) 	(IS_SANECHAR(cn) && (ch[(cn)].temp == CT_COMPANION || ch[(cn)].temp == CT_ARCHCOMP || ch[(cn)].temp == CT_CASTERCOMP || ch[(cn)].temp == CT_ARCHCASTER))
#define IS_COMP_TEMP(cn) 	(ch[(cn)].temp == CT_COMPANION || ch[(cn)].temp == CT_ARCHCOMP || ch[(cn)].temp == CT_CASTERCOMP || ch[(cn)].temp == CT_ARCHCASTER)
#define IS_PLAYER_GC(cn)  	(IS_SANEPLAYER(ch[(cn)].data[CHD_MASTER]) && ch[ch[(cn)].data[CHD_MASTER]].data[PCD_COMPANION]==(cn))
#define IS_PLAYER_SC(cn)  	(IS_SANEPLAYER(ch[(cn)].data[CHD_MASTER]) && ch[ch[(cn)].data[CHD_MASTER]].data[PCD_SHADOWCOPY]==(cn))
#define IS_PLAYER_COMP(cn) 	(IS_PLAYER_GC(cn) || IS_PLAYER_SC(cn))
#define CN_OWNER(cn) 		(ch[(cn)].data[CHD_MASTER] ? ch[(cn)].data[CHD_MASTER] : 3577)

// Visibility, etc.
#define IS_INVISIBLE(cn)	((ch[(cn)].flags & CF_INVISIBLE) != 0)
#define IS_PURPLE(cn)		((ch[(cn)].kindred & KIN_PURPLE) != 0)
#define IS_MONSTER(cn)		((ch[(cn)].kindred & KIN_MONSTER) != 0)
#define IS_MALE(cn)			((ch[(cn)].kindred & KIN_MALE) != 0)
#define IS_FEMALE(cn)		((ch[(cn)].kindred & KIN_FEMALE) != 0)
#define HE_SHE(cn)			(IS_FEMALE(cn) ? "she" : "he")
#define HE_SHE_CAPITAL(cn)	(IS_FEMALE(cn) ? "She" : "He")
#define HIS_HER(cn)			(IS_FEMALE(cn) ? "her" : "his")
#define HIM_HER(cn)			(IS_FEMALE(cn) ? "her" : "him")

// Ditto, with sanity check
#define IS_SANEPLAYER(cn)	(IS_SANECHAR(cn) && IS_PLAYER(cn))
#define IS_SANESTAFF(cn)	(IS_SANECHAR(cn) && IS_STAFF(cn))
#define IS_SANEGOD(cn)		(IS_SANECHAR(cn) && IS_GOD(cn))
#define IS_SANEUSURP(cn)	(IS_SANECHAR(cn) && IS_USURP(cn))
// IS_SANENPC is derived. No IS_NPC because of... logic.
#define IS_SANENPC(cn)		(IS_SANECHAR(cn) && !IS_PLAYER(cn))
#define IS_SANECCP(cn)		(IS_SANECHAR(cn) && IS_CCP(cn))

/* RACE CHECKS */
#define IS_TEMPLAR(cn)		(ch[(cn)].kindred & KIN_TEMPLAR)
#define IS_MERCENARY(cn)	(ch[(cn)].kindred & KIN_MERCENARY)
#define IS_HARAKIM(cn)		(ch[(cn)].kindred & KIN_HARAKIM)

#define IS_SEYAN_DU(cn)		(ch[(cn)].kindred & KIN_SEYAN_DU)
#define IS_ARCHTEMPLAR(cn)	(ch[(cn)].kindred & KIN_ARCHTEMPLAR)
#define IS_SKALD(cn)		(ch[(cn)].kindred & KIN_SKALD)
#define IS_WARRIOR(cn)		(ch[(cn)].kindred & KIN_WARRIOR)
#define IS_SORCERER(cn)		(ch[(cn)].kindred & KIN_SORCERER)
#define IS_SUMMONER(cn)		(ch[(cn)].kindred & KIN_SUMMONER)
#define IS_ARCHHARAKIM(cn)	(ch[(cn)].kindred & KIN_ARCHHARAKIM)
#define IS_BRAVER(cn)		(ch[(cn)].kindred & KIN_BRAVER)

#define IS_ANY_TEMP(cn)		(IS_TEMPLAR(cn) || IS_ARCHTEMPLAR(cn) || IS_SKALD(cn))
#define IS_ANY_MERC(cn)		(IS_MERCENARY(cn) || IS_WARRIOR(cn) || IS_SORCERER(cn))
#define IS_ANY_HARA(cn)		(IS_HARAKIM(cn) || IS_SUMMONER(cn) || IS_ARCHHARAKIM(cn))

#define IS_ANY_ARCH(cn)		(IS_SEYAN_DU(cn) || IS_ARCHTEMPLAR(cn) || IS_SKALD(cn) || IS_WARRIOR(cn) || IS_SORCERER(cn) || IS_SUMMONER(cn) || IS_ARCHHARAKIM(cn) || IS_BRAVER(cn))

#define IS_SEYA_OR_ARTM(cn)	(IS_SEYAN_DU(cn) || IS_ARCHTEMPLAR(cn))
#define IS_SEYA_OR_SKAL(cn)	(IS_SEYAN_DU(cn) || IS_SKALD(cn))
#define IS_SEYA_OR_WARR(cn)	(IS_SEYAN_DU(cn) || IS_WARRIOR(cn))
#define IS_SEYA_OR_SORC(cn)	(IS_SEYAN_DU(cn) || IS_SORCERER(cn))
#define IS_SEYA_OR_SUMM(cn)	(IS_SEYAN_DU(cn) || IS_SUMMONER(cn))
#define IS_SEYA_OR_ARHR(cn)	(IS_SEYAN_DU(cn) || IS_ARCHHARAKIM(cn))
#define IS_SEYA_OR_BRAV(cn)	(IS_SEYAN_DU(cn) || IS_BRAVER(cn))

#define IS_PROX_CLASS(cn)	(B_SK(cn, SK_PROX) || IS_SEYAN_DU(cn) || IS_ARCHTEMPLAR(cn) || IS_WARRIOR(cn))
#define CAN_ARTM_PROX(cn)	(IS_SEYA_OR_ARTM(cn) && !(ch[(cn)].flags & CF_AREA_OFF))
#define CAN_WARR_PROX(cn)	(IS_SEYA_OR_WARR(cn) && !(ch[(cn)].flags & CF_AREA_OFF))
#define CAN_SORC_PROX(cn)	(IS_SEYA_OR_SORC(cn) && !(ch[(cn)].flags & CF_AREA_OFF))
#define CAN_ARHR_PROX(cn)	(IS_SEYA_OR_ARHR(cn) && !(ch[(cn)].flags & CF_AREA_OFF))


/* *** SKILLS *** */

// Sanity check on skill number
#define IS_SANESKILL(s) ((s)>=0 && (s)<MAXSKILL)

// Fancy get/setters
#define B_AT(cn, a)		(ch[(cn)].attrib[(a)][0])
#define M_AT(cn, a)		(get_attrib_score((cn), (a)))
#define B_SK(cn, s)		(ch[(cn)].skill[(s)][0])
#define M_SK(cn, s)		(get_skill_score((cn), (s)))

#define IS_P_SKILL(a)	(a==8||a==9||a==23||a==32)

#define CAN_SENSE(cn)	((ch[(cn)].flags & CF_SENSE) || !IS_PLAYER(cn))

// Haste's formula
#define HASTEFORM(n)		(n/4)

// Slow's formula (used to degrade)
#define SLOWFORM(n)			(n/2*9/10)

// Slow2's formula (used to degrade) 
#define SLOW2FORM(n)		(n/3*9/10)

// Curse2's formula (used to degrade)
#define CURSE2FORM(p, n)	(((p*4/3)-n)/5)

// Poison's formula (damage per tick)
#define S_POISONFORM(p, d)	((p * 1750) / d)
#define POISONFORM(p, d)	((p * 1250) / d)

// Bleed's formula (damage per tick)
#define BLEEDFORM(p, d)		((p *  750) / d)

// Frostburn's formula (degen per tick)
#define FROSTBFORM(p, d)	((p * 1000) / d)

/* Tarot Card Descriptiors (for both r-click and /tarot command) */

#define DESC_FOOL		"You can apply a soulstone to this card. Once applied, it will contribute its bonuses while equipped.\n"
#define DESC_MAGI		"When equipped, secondary effects of Intuition and Strength are equal to the average of the two.\n"
#define DESC_PREIST		"When equipped, your maximum life is reduced by 10%%. 20%% of damage taken from hits is negated, and half of the negated damage is dealt to Mana instead.\n"
#define DESC_EMPRESS	"When equipped, your Magic Shield spell is replaced with Magic Shell. Magic Shell grants a temporary Resistance and Immunity bonus.\n"
#define DESC_EMPEROR	"When equipped, your Slow spell is replaced with Greater Slow. Greater Slow no longer decays and has an increased duration.\n"
#define DESC_HEIROPH	"When equipped, your Dispel spell will no longer affect you or your allies, and instead removes enemy buffs.\n"
#define DESC_LOVERS		"When equipped, your Weapon Value and Armor Value become the average of your Weapon Value and Armor Value.\n"
#define DESC_CHARIOT	"When equipped, your Blind skill is replaced with Douse. Douse reduces your target's stealth and spell modifier.\n"
#define DESC_STRENGTH	"When equipped, reduces your attack and cast speed by 20%%, but grants 20%% more damage with melee attacks.\n"
#define DESC_HERMIT		"When equipped, you have 16%% more Armor Value, but 12%% less Resistance and Immunity.\n"
#define DESC_WHEEL		"When equipped, your critical hit chance is reduced by 33%%, but you have 1.5x critical hit multiplier.\n"
#define DESC_JUSTICE	"When equipped, your Cleave skill no longer inflicts a Bleeding, and instead inflicts Aggravate, causing the target to take additional damage for 20 seconds.\n"
#define DESC_HANGED		"When equipped, 25%% of your Resistance is instead used to reduce the strength of incoming enemy spells.\n"
#define DESC_DEATH		"When equipped, your Weaken skill is replaced with Crush. Crush reduces a target's Armor Value, but no longer reduces enemy Weapon Value.\n"
#define DESC_TEMPER		"When equipped, your Taunt skill grants 30%% less Guard power, but Guard duration is tripled.\n"
#define DESC_DEVIL		"When equipped, 30%% of all skill and spell costs are instead twice taken from your Hitpoints.\n"
#define DESC_TOWER		"When equipped, your Curse spell is replaced with Greater Curse. Greater Curse has increased effect, but decays and has reduced duration.\n"
#define DESC_STAR		"When equipped, your Heal spell is replaced with Regen. Regen grants a buff which regenerates the target's Hitpoints over 20 seconds.\n"
#define DESC_MOON		"When equipped, life regeneration is instead applied as mana regeneration while not at full mana.\n"
#define DESC_SUN		"When equipped, endurance regeneration is instead applied as life regeneration while not at full life.\n"
#define DESC_JUDGE		"When equipped, your Blast spell deals 15%% less damage but inflicts Scorch, causing the target to take additional damage for 20 seconds.\n"
#define DESC_WORLD		"When equipped, mana regeneration is instead applied as endurance regeneration while not at full endurance.\n"

#define DESC_FOOL_R		"When equipped, your attributes become the average of all attributes, plus 10%%.\n"
#define DESC_MAGI_R		"When equipped, your concentrate skill instead increases the cost of spells, but it also increases your spell modifier.\n"
#define DESC_PREIST_R	"When equipped, your maximum mana is reduced by 20%% to increase your cooldown bonus by 10%% of the subtracted mana.\n"
#define DESC_EMPRES_R	"When equipped, your buffs are applied at 125%% of the target's spell aptitude, but your spell aptitude is halved.\n"
#define DESC_EMPERO_R	"When equipped, your Warcry skill is replaced with Rally. Rally grants nearby allies a buff which improves Hit Score and Parry Score.\n"
#define DESC_HEIROP_R	"When equipped, your Ghost Companion shares the bonus granted to you by your other tarot card slot.\n"
#define DESC_LOVERS_R	"When equipped, your Hit Score and Parry Score become the average of your Hit Score and Parry Score.\n"
#define DESC_CHARIO_R	"When equipped, your debuffs ignore 20%% of target resistance and immunity, but are 25%% weaker once applied.\n"
#define DESC_STRENG_R	"When equipped, you have 20%% more Weapon Value, but 20%% less hit score.\n"
#define DESC_HERMIT_R	"When equipped, your Rage skill grants more Weapon Value, but costs life over time instead of endurance over time.\n"
#define DESC_WHEEL_R	"When equipped, you have 16%% more Armor Value, but a 25%% chance to be hit when you would have parried.\n"
#define DESC_JUSTIC_R	"When equipped, your Leap skill deals 30%% less damage, but has reduced base cooldown and instead chooses a random nearby target.\n"
#define DESC_HANGED_R	"When equipped, you have 24%% more Top Damage, but 12%% less Weapon Value.\n"
#define DESC_DEATH_R	"When equipped, your Zephyr skill grants a bonus to Resistance instead of Immunity. Zephyr triggers on parry instead of on hit, and earns a damage bonus from Thorns instead of Attack Speed.\n"
#define DESC_TEMPER_R	"When equipped, you gain 6.25%% more Weapon Value per stack of Healing Sickness on you. The maximum healing sickness you can receive is increased by 1 stack.\n"
#define DESC_DEVIL_R	"When equipped, your Shadow Copy deals 20%% more damage and takes 20%% less damage, but while your Shadow Copy is active you deal 20%% less damage and take 20%% more damage.\n"
#define DESC_TOWER_R	"When equipped, your Poison spell is replaced with Venom. Venom deals twice as much damage and reduces enemy Immunity, but it cannot stack.\n"
#define DESC_STAR_R		"When equipped, your Spell Modifier no longer effects spell power and instead effects your Critical Hit Chance.\n"
#define DESC_MOON_R		"When equipped, the effectiveness of your Meditate skill is tripled while fighting, but zero while stationary.\n"
#define DESC_SUN_R		"When equipped, the effectiveness of your Regenerate skill is tripled while fighting, but zero while stationary.\n"
#define DESC_JUDGE_R	"When equipped, you cannot cast your Pulse spell yourself. Pulse is cast on your Ghost Companion upon creation and is permanent, but 20%% of damage is taken by your Ghost Companion each pulse.\n"
#define DESC_WORLD_R	"When equipped, the effectiveness of your Rest skill is tripled while fighting, but zero while stationary.\n"


/* *** CASINO *** */

#define TOKEN_RATE			1000

#define C_CUR_GAME(a)		(ch[(a)].data[26]>>26)
#define C_CUR_WAGER(a)		(ch[(a)].data[27]>>26)

#define C_GAME_HR			1
#define C_GAME_SE			2
#define C_GAME_BJ			3

#define C_SET_GAME_HR		(1<<26)
#define C_SET_GAME_SE		(2<<26)
#define C_SET_GAME_BJ		(3<<26)

#define BJ_NUM_CARDS		26

/* *** CONTRACTS *** */

#define MSN_COUNT			10

#define CONT_NUM(a)			(ch[(a)].data[41]>>24)
#define CONT_SCEN(a)		((ch[(a)].data[41]%(1<<24))>>16)
#define CONT_GOAL(a)		(((ch[(a)].data[41]%(1<<24))%(1<<16))>>8)
#define CONT_PROG(a)		(((ch[(a)].data[41]%(1<<24))%(1<<16))%(1<<8))

#define IS_KILL_CON(a)		(CONT_SCEN(a)==1 || CONT_SCEN(a)==2 || CONT_SCEN(a)==3 || CONT_SCEN(a)==8 || CONT_SCEN(a)==9)

#define IS_CON_NME(a)		(ch[(a)].data[42]==1100 && ch[(a)].data[72]>=1)
#define IS_CON_DIV(a)		(ch[(a)].data[42]==1100 && ch[(a)].data[72]==3)
#define IS_CON_CRU(a)		(ch[(a)].data[42]==1100 && ch[(a)].data[72]==4)
#define IS_CON_COW(a)		(ch[(a)].data[42]==1100 && ch[(a)].data[72]==5)
#define IS_CON_UNI(a)		(ch[(a)].data[42]==1100 && ch[(a)].data[72]==6)

#define MSN_CN			"CONTRACT: "
#define MSN_00			"Find the exit!"
#define MSN_01			"Defeat all enemies!"
#define MSN_02			"Defeat all divine enemies!"
#define MSN_03			"Defeat all cruel enemies!"
#define MSN_04			"Find the artifact!"
#define MSN_05			"Find all the shrines!"
#define MSN_06			"Find all the chests!"
#define MSN_07			"Touch all the pentagrams!"
#define MSN_08			"Defeat the timid enemy!"
#define MSN_09			"Defeat the unique enemy!"

// 						"!        .         .   |     .         .        !"
#define CFL_08000000	"  Area contains piles of gold\n"
#define CFL_00000001	"  Area has additional enemies\n"
#define CFL_00000020	"  Area has additional spike traps\n"
#define CFL_00000004	"  Area has an additional chest\n"
#define CFL_00000002	"  Area has an additional Divine enemy\n"
#define CFL_00000010	"  Area has an additional shrine\n"
#define CFL_00000008	"  Area is underwater\n"
#define CFL_10000000	"  Contract rank is increased by 1\n"
#define CFL_20000000	"  Contract rank is increased by 2\n"
#define CFL_00000200	"  Enemies are perceptive\n"
#define CFL_00000400	"  Enemies are resistant\n"
#define CFL_00000040	"  Enemies are undead\n"
#define CFL_04000000	"  Enemies grant additional exp on kill\n"
#define CFL_00002000	"  Enemies have additional armor value\n"
#define CFL_00008000	"  Enemies have additional spellmod\n"
#define CFL_00000800	"  Enemies have additional weapon skill\n"
#define CFL_00001000	"  Enemies have additional weapon value\n"
#define CFL_00004000	"  Enemies move faster\n"
#define CFL_00000100	"  Enemies roam farther\n"
#define CFL_01000000	"  Enemies use forward tarot cards\n"
#define CFL_02000000	"  Enemies use reverse tarot cards\n"
#define CFL_00000080	"  Exit is guarded by an additional Cruel enemy\n"
#define CFL_00800000	"  Exit portal grants additional contract pts\n"
#define CFL_00100000	"  Exit portal grants additional exp\n"
#define CFL_00200000	"  Exit portal grants additional luck\n"
#define CFL_00400000	"  Exit portal grants additional stronghold pts\n"
#define CFL_00010000	"  Players are debilitated, reducing attributes\n"
#define CFL_00020000	"  Players are fragile, reducing WV and AV\n"
#define CFL_00080000	"  Players are hyperthermic, draining EN and MP\n"
#define CFL_00040000	"  Players are stigmatic, reducing perception\n"


#define IS_IN_SUN(x, y) ((x>=32 && y>=407 && x<=57 && y<= 413) || (x>=32 && y>=414 && x<=64 && y<= 428) || (x>=22 && y>=429 && x<=64 && y<=450) || (x>=22 && y>=451 && x<=27 && y<=459) || (x>=59 && y>=451 && x<=64 && y<=465) || (x>=173 && y>=921 && x<=255 && y<=1003))
/*************************************************************************

   This file is part of 'Mercenaries of Astonia v2'
   Copyright (c) 1997-2001 Daniel Brockhaus (joker@astonia.com)
   All rights reserved.

 **************************************************************************/

#define max(a, b)				((a)>(b) ? (a) : (b))
#define min(a, b)				((a)<(b) ? (a) : (b))
#define RANDOM(a)				(a>0?random()%(a):0)
#define ARRAYSIZE(a)			(sizeof(a)/sizeof(a[0]))
#define sqr(a)					((a) * (a))

// Sanity checks on map locations x and y
#define SANEX(x)     			((x) >= 0 && (x) < MAPX)
#define SANEY(y)     			((y) >= 0 && (y) < MAPY)
#define SANEXY(x, y) 			(SANEX(x) && SANEY(y))

// Convert (x,y) coordinates to absolute position
#define XY2M(x, y) 				((x) + (y) * MAPX)
#define M2X(m) 					((m) % MAPX)
#define M2Y(m) 					((m) / MAPX)

// Character 'cn's absolute position
#define CHPOSM(cn) 				(XY2M(ch[(cn)].x,ch[(cn)].y))

/* *** GLOBALS *** */
#define IS_GLOB_MAYHEM			(globs->flags & GF_MAYHEM)

/* *** ITEMS *** */

// Use flag check on a STRUCTURE ELEMENT (works for characters too)
// DB: i dont know how it might fail... but i've added some parents anyway. here and in other places.
// DB: looked fairly safe before, so no need to worry
#define IS_USED(x) 				((x).used != USE_EMPTY)

// Sanity checks on item numbers
#define IS_SANEITEM(in)			((in) > 0 && (in) < MAXITEM)
#define IS_USEDITEM(in)			(IS_USED(it[(in)]))
#define IS_SANEUSEDITEM(in)		(IS_SANEITEM(in) && IS_USEDITEM(in))

#define CAN_SOULSTONE(in)		(it[(in)].flags & IF_CAN_SS)
#define CAN_ENCHANT(in)			(it[(in)].flags & IF_CAN_EN)
#define HAS_ENCHANT(in, n)		(it[(in)].enchantment == (n))

#define IS_SINBINDER(in)		(it[(in)].temp==IT_TW_SINBIND || it[(in)].orig_temp==IT_TW_SINBIND)
#define NOT_SINBINDER(in)		(it[(in)].temp!=IT_TW_SINBIND && it[(in)].orig_temp!=IT_TW_SINBIND)

#define IS_GEMSTONE(in)			(it[(in)].flags & IF_GEMSTONE)
#define IS_SOULSTONE(in)		(it[(in)].driver==68)
#define IS_SOULFOCUS(in)		(it[(in)].driver==92)
#define IS_SOULCAT(in)			(it[(in)].driver==93)
#define IS_TAROT(in)			((it[(in)].temp>=IT_CH_FOOL && it[(in)].temp<=IT_CH_WORLD) || (it[(in)].temp>=IT_CH_FOOL_R && it[(in)].temp<=IT_CH_WORLD_R))

#define IS_CONTRACT(in)			(it[(in)].temp==MCT_CONTRACT)
#define IS_QUILL(in)			(it[(in)].temp==MCT_QUILL_Y||it[(in)].temp==MCT_QUILL_G||it[(in)].temp==MCT_QUILL_B||it[(in)].temp==MCT_QUILL_R)

#define IS_TWOHAND(in)			(it[(in)].placement & PL_TWOHAND)
#define IS_USETWOHAND(cn)		(it[ch[(cn)].worn[WN_RHAND]].placement & PL_TWOHAND)

#define IS_SOULSTONED(in)		(it[(in)].flags & IF_SOULSTONE)
#define IS_ENCHANTED(in)		(it[(in)].flags & IF_ENCHANTED)


/* *** TEMPLATES *** */

// Sanity checks on item templates
#define IS_SANEITEMPLATE(tn) 	((tn) > 0 && (tn) < MAXTITEM)
#define IS_SANECTEMPLATE(tn) 	((tn) > 0 && (tn) < MAXTCHARS)

#define IS_LONG_RESPAWN(temp) 	(temp==CT_RATKING || temp==CT_GREENKING || temp==CT_DREADKING || temp==CT_LIZEMPEROR || temp==CT_BSMAGE1 || temp==CT_BSMAGE2 || temp==CT_BSMAGE3 || temp==CT_SCORP_Q)

/* *** CHARACTERS *** */

// Sanity checks on character numbers
#define IS_SANECHAR(cn)     	((cn) > 0 && (cn) < MAXCHARS)
#define IS_LIVINGCHAR(cn)   	(IS_SANECHAR(cn) && ch[(cn)].used != USE_EMPTY)
#define IS_ACTIVECHAR(cn)   	(IS_SANECHAR(cn) && ch[(cn)].used == USE_ACTIVE)
#define IS_USEDCHAR(cn)     	(IS_USED(ch[(cn)]))
#define IS_SANEUSEDCHAR(cn) 	(IS_SANECHAR(cn) && IS_USEDCHAR(cn))
#define IS_ALIVEMASTER(cn, co)	(ch[(cn)].data[CHD_MASTER]==(co) && !(ch[(cn)].flags & CF_BODY) && ch[(cn)].used!=USE_EMPTY)

// flag checks
#define IS_PLAYER(cn)			((ch[(cn)].flags & CF_PLAYER) != 0)
#define IS_STAFF(cn)			((ch[(cn)].flags & CF_STAFF) != 0)
#define IS_GOD(cn)				((ch[(cn)].flags & CF_GOD) != 0)
#define IS_USURP(cn)			((ch[(cn)].flags & CF_USURP) != 0)
#define IS_IMP(cn)				((ch[(cn)].flags & CF_IMP) != 0)
#define IS_QM(cn)				((ch[(cn)].flags & (CF_IMP|CF_USURP)) != 0)
#define IS_IGNORING_SPELLS(cn)	((ch[(cn)].flags & CF_SPELLIGNORE) != 0)
#define IS_CCP(cn)				((ch[(cn)].flags & CF_CCP) != 0)
#define IS_BUILDING(cn)			((ch[(cn)].flags & CF_BUILDMODE) != 0)
#define IS_THRALL(cn)			((ch[(cn)].flags & CF_THRALL) || ch[(cn)].data[CHD_GROUP] == 65500)

// special character group checks
#define IS_COMPANION(cn) 		(IS_SANECHAR(cn) && (ch[(cn)].temp == CT_COMPANION || ch[(cn)].temp == CT_ARCHCOMP || ch[(cn)].temp == CT_CASTERCOMP || ch[(cn)].temp == CT_ARCHCASTER))
#define IS_COMP_TEMP(cn) 		(ch[(cn)].temp == CT_COMPANION || ch[(cn)].temp == CT_ARCHCOMP || ch[(cn)].temp == CT_CASTERCOMP || ch[(cn)].temp == CT_ARCHCASTER)
#define IS_PLAYER_GC(cn)  		(IS_SANEPLAYER(ch[(cn)].data[CHD_MASTER]) && ch[ch[(cn)].data[CHD_MASTER]].data[PCD_COMPANION]==(cn))
#define IS_PLAYER_SC(cn)  		(IS_SANEPLAYER(ch[(cn)].data[CHD_MASTER]) && ch[ch[(cn)].data[CHD_MASTER]].data[PCD_SHADOWCOPY]==(cn))
#define IS_PLAYER_COMP(cn) 		((IS_PLAYER_GC(cn) || IS_PLAYER_SC(cn)) && !IS_THRALL(cn))
#define CN_OWNER(cn) 			(ch[(cn)].data[CHD_MASTER] ? ch[(cn)].data[CHD_MASTER] : 3577)

// Visibility, etc.
#define IS_INVISIBLE(cn)		((ch[(cn)].flags & CF_INVISIBLE) != 0)
#define IS_PURPLE(cn)			((ch[(cn)].kindred & KIN_PURPLE) != 0)
#define IS_MONSTER(cn)			((ch[(cn)].kindred & KIN_MONSTER) != 0)
#define IS_FEMALE(cn)			((ch[(cn)].kindred & KIN_FEMALE) != 0)
#define IS_CLANKWAI(cn)			((ch[(cn)].kindred & KIN_CLANKWAI) != 0)
#define IS_CLANGORN(cn)			((ch[(cn)].kindred & KIN_CLANGORN) != 0)
#define HE_SHE(cn)				(IS_FEMALE(cn) ? "she" : "he")
#define HE_SHE_CAPITAL(cn)		(IS_FEMALE(cn) ? "She" : "He")
#define HIS_HER(cn)				(IS_FEMALE(cn) ? "her" : "his")
#define HIM_HER(cn)				(IS_FEMALE(cn) ? "her" : "him")

#define IS_OPP_CLAN(cn, co)		((IS_CLANKWAI(cn) && IS_CLANGORN(co)) || (IS_CLANKWAI(co) && IS_CLANGORN(cn)))

// Ditto, with sanity check
#define IS_SANEPLAYER(cn)		(IS_SANECHAR(cn) && IS_PLAYER(cn))
#define IS_SANESTAFF(cn)		(IS_SANECHAR(cn) && IS_STAFF(cn))
#define IS_SANEGOD(cn)			(IS_SANECHAR(cn) && IS_GOD(cn))
#define IS_SANEUSURP(cn)		(IS_SANECHAR(cn) && IS_USURP(cn))
// IS_SANENPC is derived. No IS_NPC because of... logic.
#define IS_SANENPC(cn)			(IS_SANECHAR(cn) && !IS_PLAYER(cn))
#define IS_SANECCP(cn)			(IS_SANECHAR(cn) && IS_CCP(cn))

/* RACE CHECKS */
#define IS_TEMPLAR(cn)			(ch[(cn)].kindred & KIN_TEMPLAR)
#define IS_MERCENARY(cn)		(ch[(cn)].kindred & KIN_MERCENARY)
#define IS_HARAKIM(cn)			(ch[(cn)].kindred & KIN_HARAKIM)

#define IS_SEYAN_DU(cn)			(ch[(cn)].kindred & KIN_SEYAN_DU)
#define IS_ARCHTEMPLAR(cn)		(ch[(cn)].kindred & KIN_ARCHTEMPLAR)
#define IS_SKALD(cn)			(ch[(cn)].kindred & KIN_SKALD)
#define IS_WARRIOR(cn)			(ch[(cn)].kindred & KIN_WARRIOR)
#define IS_SORCERER(cn)			(ch[(cn)].kindred & KIN_SORCERER)
#define IS_SUMMONER(cn)			(ch[(cn)].kindred & KIN_SUMMONER)
#define IS_ARCHHARAKIM(cn)		(ch[(cn)].kindred & KIN_ARCHHARAKIM)
#define IS_BRAVER(cn)			(ch[(cn)].kindred & KIN_BRAVER)

#define IS_ANY_TEMP(cn)			(IS_TEMPLAR(cn) || IS_ARCHTEMPLAR(cn) || IS_SKALD(cn))
#define IS_ANY_MERC(cn)			(IS_MERCENARY(cn) || IS_WARRIOR(cn) || IS_SORCERER(cn))
#define IS_ANY_HARA(cn)			(IS_HARAKIM(cn) || IS_SUMMONER(cn) || IS_ARCHHARAKIM(cn))

#define IS_ANY_ARCH(cn)			(IS_SEYAN_DU(cn) || IS_ARCHTEMPLAR(cn) || IS_SKALD(cn) || IS_WARRIOR(cn) || IS_SORCERER(cn) || IS_SUMMONER(cn) || IS_ARCHHARAKIM(cn) || IS_BRAVER(cn))

#define IS_SEYA_OR_ARTM(cn)		(IS_SEYAN_DU(cn) || IS_ARCHTEMPLAR(cn))
#define IS_SEYA_OR_SKAL(cn)		(IS_SEYAN_DU(cn) || IS_SKALD(cn))
#define IS_SEYA_OR_WARR(cn)		(IS_SEYAN_DU(cn) || IS_WARRIOR(cn))
#define IS_SEYA_OR_SORC(cn)		(IS_SEYAN_DU(cn) || IS_SORCERER(cn))
#define IS_SEYA_OR_SUMM(cn)		(IS_SEYAN_DU(cn) || IS_SUMMONER(cn))
#define IS_SEYA_OR_ARHR(cn)		(IS_SEYAN_DU(cn) || IS_ARCHHARAKIM(cn))
#define IS_SEYA_OR_BRAV(cn)		(IS_SEYAN_DU(cn) || IS_BRAVER(cn))

#define IS_PROX_CLASS(cn)		(B_SK(cn, SK_PROX) || IS_SEYAN_DU(cn) || IS_ARCHTEMPLAR(cn) || IS_WARRIOR(cn))
#define CAN_ARTM_PROX(cn)		(IS_SEYA_OR_ARTM(cn) && !(ch[(cn)].flags & CF_AREA_OFF))
#define CAN_WARR_PROX(cn)		(IS_SEYA_OR_WARR(cn) && !(ch[(cn)].flags & CF_AREA_OFF))
#define CAN_SORC_PROX(cn)		(IS_SEYA_OR_SORC(cn) && !(ch[(cn)].flags & CF_AREA_OFF))
#define CAN_ARHR_PROX(cn)		(IS_SEYA_OR_ARHR(cn) && !(ch[(cn)].flags & CF_AREA_OFF))
#define CAN_BRAV_PROX(cn)		(IS_SEYA_OR_BRAV(cn) && !(ch[(cn)].flags & CF_AREA_OFF))


/* *** SKILLS *** */

// Sanity check on skill number
#define IS_SANESKILL(s) 		((s)>=0 && (s)<MAXSKILL)

// Fancy get/setters
#define B_AT(cn, a)				(ch[(cn)].attrib[(a)][0])
#define M_AT(cn, a)				(get_attrib_score((cn), (a)))
#define B_SK(cn, s)				(ch[(cn)].skill[(s)][0])
#define M_SK(cn, s)				((s)==SK_PERCEPT?(get_skill_score((cn), (s))*(HAS_ENCHANT(ch[(cn)].worn[WN_HEAD], 52)?4:3)/3):get_skill_score((cn), (s)))

#define T_SK(cn, a)				(IS_SANEPLAYER(cn)  && st_learned_skill(ch[(cn)].tree_points, (a)))
#define T_SEYA_SK(cn, a)		(IS_SEYAN_DU(cn)    && T_SK((cn), (a)))
#define T_ARTM_SK(cn, a)		(IS_ARCHTEMPLAR(cn) && T_SK((cn), (a)))
#define T_SKAL_SK(cn, a)		(IS_SKALD(cn)       && T_SK((cn), (a)))
#define T_WARR_SK(cn, a)		(IS_WARRIOR(cn)     && T_SK((cn), (a)))
#define T_SORC_SK(cn, a)		(IS_SORCERER(cn)    && T_SK((cn), (a)))
#define T_SUMM_SK(cn, a)		(IS_SUMMONER(cn)    && T_SK((cn), (a)))
#define T_ARHR_SK(cn, a)		(IS_ARCHHARAKIM(cn) && T_SK((cn), (a)))
#define T_BRAV_SK(cn, a)		(IS_BRAVER(cn)      && T_SK((cn), (a)))

#define IS_P_SKILL(a)			(a==8||a==9||a==23||a==32)

#define CAN_SENSE(cn)			((ch[(cn)].flags & CF_SENSE) || !IS_PLAYER(cn))

// Slow's formula (used to degrade)
#define SLOWFORM(n)				(n/2*9/10)

// Slow2's formula (used to degrade) 
#define SLOW2FORM(n)			(n/3*9/10)

// Curse2's formula (used to degrade)
#define CURSE2FORM(p, n)		(((p*5/3)-n)/5)

// Poison's formula (damage per tick)
#define PL_POISFORM(p, d)		(((p+ 5) * 4500) / d)
#define MN_POISFORM(p, d)		(((p   ) * 3000) / d)

// Bleed's formula (damage per tick)
#define BLEEDFORM(p, d)			(((p+ 5) *  750) / d)

// Frostburn's formula (degen per tick)
#define FROSTBFORM(p, d)		(((p+10) * 1000) / d)

/* Tarot Card Descriptiors (for both r-click and /tarot command) */

#define DESC_FOOL		"You can apply a soulstone to this card. Once applied, it will contribute its bonuses while equipped.\n"
#define DESC_MAGI		"When equipped, secondary effects of Intuition and Strength are equal to the higher of the two.\n"
#define DESC_PREIST		"When equipped, your maximum life is reduced by 10%%. 20%% of damage taken from hits is negated, and half of the negated damage is dealt to Mana instead.\n"
#define DESC_EMPRESS	"When equipped, your Magic Shield spell is replaced with Magic Shell. Magic Shell grants a temporary Resistance and Immunity bonus.\n"
#define DESC_EMPEROR	"When equipped, your Slow spell is replaced with Greater Slow. Greater Slow no longer decays and has an increased duration.\n"
#define DESC_HEIROPH	"When equipped, your Dispel spell will no longer affect you or your allies, and instead removes enemy buffs.\n"
#define DESC_LOVERS		"When equipped, your Weapon Value and Armor Value become the average of your Weapon Value and Armor Value.\n"
#define DESC_CHARIOT	"When equipped, your Blind skill is replaced with Douse. Douse reduces your target's stealth and spell modifier.\n"
#define DESC_STRENGTH	"When equipped, reduces your attack and cast speed by 15%%, but grants 20%% more damage with melee attacks.\n"
#define DESC_HERMIT		"When equipped, you have 15%% more Armor Value, but 20%% less Resistance and Immunity.\n"
#define DESC_WHEEL		"When equipped, your critical hit chance is reduced by 33%%, but you have 33%% more critical hit multiplier.\n"
#define DESC_JUSTICE	"When equipped, your Cleave skill no longer inflicts a Bleeding, and instead inflicts Aggravate, causing the target to take additional damage for 20 seconds.\n"
#define DESC_HANGED		"When equipped, 33%% of your Resistance is instead used to reduce the strength of incoming enemy spells.\n"
#define DESC_DEATH		"When equipped, your Weaken skill is replaced with Crush. Crush reduces a target's Armor Value, but no longer reduces enemy Weapon Value.\n"
#define DESC_TEMPER		"When equipped, your Taunt skill grants 100%% more Guard power, but Guard duration is halved.\n"
#define DESC_DEVIL		"When equipped, 30%% of all skill and spell costs are instead twice taken from your Hitpoints.\n"
#define DESC_TOWER		"When equipped, your Curse spell is replaced with Greater Curse. Greater Curse has increased effect, but decays and has reduced duration.\n"
#define DESC_STAR		"When equipped, your Heal spell is replaced with Regen. Regen grants a buff which regenerates the target's Hitpoints over 20 seconds.\n"
#define DESC_MOON		"When equipped, life regeneration is instead applied as mana regeneration while not at full mana.\n"
#define DESC_SUN		"When equipped, endurance regeneration is instead applied as life regeneration while not at full life.\n"
#define DESC_JUDGE		"When equipped, your Blast spell deals 15%% less damage but inflicts Scorch, causing the target to take additional damage for 20 seconds.\n"
#define DESC_WORLD		"When equipped, mana regeneration is instead applied as endurance regeneration while not at full endurance.\n"

#define DESC_FOOL_R		"When equipped, your attributes become the average of all attributes, plus 8%%.\n"
#define DESC_MAGI_R		"When equipped, your Concentrate skill no longer reduces the Mana cost of skills, and instead increases your Cooldown rate.\n"
#define DESC_PREIST_R	"When equipped, your HP and MP limits are now 800, and natural overcap effects are disabled. Overcapped HP now increases damage taken by 1%% per 50. Overcapped MP now increases mana costs by 1%% per 20. Overcapping either resource now increases damage dealt by 1%% per 100.\n"
#define DESC_EMPRES_R	"When equipped, your Lethargy skill costs life over time instead of mana over time.\n"
#define DESC_EMPERO_R	"When equipped, your Warcry skill is replaced with Rally. Rally grants nearby allies a buff which improves Hit Score and Parry Score.\n"
#define DESC_HEIROP_R	"When equipped, your Ghost Companion has 12%% more Weapon Value and Armor Value, but has a 20%% chance to miss when it should have hit.\n"
#define DESC_LOVERS_R	"When equipped, your Hit Score and Parry Score become the average of your Hit Score and Parry Score.\n"
#define DESC_CHARIO_R	"When equipped, your debuffs ignore 20%% of target resistance and immunity, but are 25%% weaker once applied.\n"
#define DESC_STRENG_R	"When equipped, you have 20%% more Weapon Value, but 20%% less hit score.\n"
#define DESC_HERMIT_R	"When equipped, your Rage skill grants a large bonus to Top Damage instead of Weapon Value.\n"
#define DESC_WHEEL_R	"When equipped, you take 20%% less damage from melee attacks, but have a 25%% chance to be hit when you would have parried.\n"
#define DESC_JUSTIC_R	"When equipped, your Leap skill deals halved critical damage, has halved base cooldown, and instead chooses a random nearby target.\n"
#define DESC_HANGED_R	"When equipped, you have 24%% more Top Damage, but 12%% less Weapon Value.\n"
#define DESC_DEATH_R	"When equipped, your Zephyr skill grants a bonus to Resistance instead of Immunity. Zephyr triggers on parry instead of on hit, and earns a damage bonus from Thorns instead of Attack Speed.\n"
#define DESC_TEMPER_R	"When equipped, you gain 6.25%% more Weapon Value per stack of Healing Sickness on you. The maximum healing sickness you can receive is increased by 1 stack.\n"
#define DESC_DEVIL_R	"When equipped, your Shadow Copy deals 25%% more damage and takes 25%% less damage, but while your Shadow Copy is active you deal 20%% less damage and take 20%% more damage.\n"
#define DESC_TOWER_R	"When equipped, your Poison spell is replaced with Venom. Venom deals half as much damage, but it reduces enemy Immunity and can stack up to three times.\n"
#define DESC_STAR_R		"When equipped, your base Spell Modifier is 0.90. Your Spell Modifier no longer effects spell power and instead effects skill power.\n"
#define DESC_MOON_R		"When equipped, the effectiveness of your Meditate skill is normalized while fighting, but zero while stationary.\n"
#define DESC_SUN_R		"When equipped, the effectiveness of your Regenerate skill is normalized while fighting, but zero while stationary.\n"
#define DESC_JUDGE_R	"When equipped, your Pulse spell is replaced with Immolate. Immolate is a toggle that causes you and surrounding enemies to take damage over time.\n"
#define DESC_WORLD_R	"When equipped, the effectiveness of your Rest skill is normalized while fighting, but zero while stationary.\n"


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
#define CFL_P_CHST		"  Area has %d additional chests\n"
#define CFL_P_SHRN		"  Area has %d additional shrines\n"
#define CFL_P_XEXP		"  Exit grants %d%% more exp\n"
#define RATE_P_XEXP		25
#define CFL_P_XLUK		"  Exit grants %d%% of exp as luck\n"
#define RATE_P_XLUK		5
#define CFL_P_XBSP		"  Exit grants %d%% of exp as stronghold pts\n"
#define RATE_P_XBSP		10
#define CFL_P_XOSP		"  Exit grants %d%% of exp as Osiris pts\n"
#define RATE_P_XOSP		10
#define CFL_P_PLXP		"  Players earn %d%% more exp from enemies\n"
#define RATE_P_PLXP		20
#define CFL_P_ENBS		"  Enemies grant %d%% of exp as stronghold pts\n"
#define RATE_P_ENBS		10
#define CFL_P_ENOS		"  Enemies grant %d%% of exp as Osiris pts\n"
#define RATE_P_ENOS		10
#define CFL_P_ENGL		"  Enemies drop %d%% of exp as gold\n"
#define RATE_P_ENGL		5
#define CFL_P_ARGL		"  Area contains %s piles of gold\n"
#define CFL_P_AREQ		"  Area contains %s discarded equipment\n"
#define CFL_P_ARPT		"  Area contains %s discarded potions\n"
#define CFL_P_DRGM		"  %d enemies drop an additional huge gem\n"
#define CFL_P_RANK		"  Contract rank is increased by %d\n"
// 						"!        .         .   |     .         .        !"
#define CFL_N_EXTY		"  Area has %d additional enemy spawns\n"
#define CFL_N_EXDV		"  Area has %d additional divine enemy spawns\n"
#define CFL_N_ARUW		"  %d%% of area is underwater\n"
#define CFL_N_ENUN		"  Enemies are %d%% undead\n"
#define CFL_N_EXEN		"  Exit is guarded by %d cruel enemies\n"
#define CFL_N_ENRO		"  Enemies roam %d%% farther\n"
#define CFL_N_ENRS		"  Enemies are %d%% more resistant\n"
#define CFL_N_ENSK		"  Enemies are %d%% more skillful\n"
#define CFL_N_ENSH		"  Enemies have %d%% more weapon value\n"
#define CFL_N_ENFO		"  Enemies have %d%% more armor value\n"
#define CFL_N_ENFS		"  Enemies are %d%% faster\n"
#define CFL_N_ENWI		"  Enemies have %d%% more spellmod\n"
#define CFL_N_PLDB		"  Players are %d%% debilitated\n"
#define CFL_N_PLFR		"  Players are %d%% more fragile\n"
#define CFL_N_PLST		"  Players are %d%% stigmatic\n"
#define CFL_N_PLHY		"  Players are %d%% hyperthermic\n"
#define CFL_N_ENTR		"  Enemies use %s tarot cards.\n"
#define CFL_N_ARSP		"  Area contains %s spike traps\n"
#define CFL_N_ARDT		"  Area contains %s hidden dart traps\n"
#define CFL_N_ARFL		"  Area contains %s open flames\n"
// 						"!        .         .   |     .         .        !"

#define IS_IN_SUN(x, y) 	((x>=  32&&y>= 407&&x<=  57&&y<= 413)||(x>=  32&&y>= 414&&x<=  64&&y<= 428)||(x>=  22&&y>= 429&&x<=  64&&y<= 450)||(x>=  22&&y>= 451&&x<=  27&&y<= 459)||(x>=  59&&y>= 451&&x<=  64&&y<= 465)||(x>= 173&&y>= 921&&x<= 255&&y<=1003))
#define IS_IN_ABYSS(x, y)	((x>=438&&y>=110&&x<=470&&y<=142)?1:((x>=438&&y>=148&&x<=470&&y<=180)?2:((x>=476&&y>=148&&x<=508&&y<=180)?3:((x>=476&&y>=110&&x<=508&&y<=142)?4:((x>=476&&y>= 72&&x<=508&&y<=104)?5:((x>=476&&y>= 34&&x<=508&&y<= 66)?6:((x>=514&&y>= 34&&x<=546&&y<= 66)?7:((x>=514&&y>= 72&&x<=546&&y<=104)?8:((x>=514&&y>=110&&x<=546&&y<=142)?9:((x>=523&&y>=148&&x<=537&&y<=180)?10:0))))))))))
#define IS_IN_XIII(x, y) 	((x>= 132&&y>= 153&&x<= 152&&y<= 240))
#define IS_IN_VANTA(x, y) 	((x>=  94&&y>=1135&&x<= 182&&y<=1223))
#define IS_IN_XVIII(x, y) 	((x>=  94&&y>=1474&&x<= 171&&y<=1564))
#define IS_IN_XIX(x, y) 	((x>=  94&&y>=1587&&x<= 260&&y<=1753))
#define IS_IN_ZRAK(x, y) 	((x>=  94&&y>=1587&&x<= 185&&y<=1670))
#define IS_IN_BOJ(x, y) 	((x>= 203&&y>=1587&&x<= 260&&y<=1670))
#define IS_IN_CAROV(x, y) 	((x>= 178&&y>=1687&&x<= 260&&y<=1753))
#define IS_IN_XX(x, y) 		((x>= 194&&y>=1474&&x<= 214&&y<=1533))
#define IS_IN_TLG(x, y) 	((x>= 195&&y>=1545&&x<= 213&&y<=1563))

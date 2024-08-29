/*************************************************************************

   This file is part of 'Mercenaries of Astonia v2'
   Copyright (c) 1997-2001 Daniel Brockhaus (joker@astonia.com)
   All rights reserved.

 **************************************************************************/

#define max(a, b)				((a)>(b) ? (a) : (b))
#define min(a, b)				((a)<(b) ? (a) : (b))
#define RANDOM(a)				(a>1?random()%(a):0)
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

/* *** ANIMATION *** */
#define A_ST(cn)				(ch_base_status(ch[(cn)].status))
#define IS_CNSTANDING(cn)		(A_ST(cn)>=  0&&A_ST(cn)<=  7)
#define IS_CNWALKING(cn)		(A_ST(cn)== 16||A_ST(cn)== 24||A_ST(cn)== 32||A_ST(cn)== 40||A_ST(cn)== 48||A_ST(cn)== 60||A_ST(cn)== 72||A_ST(cn)== 84||A_ST(cn)== 96||A_ST(cn)==100||A_ST(cn)==104||A_ST(cn)==108||A_ST(cn)==112||A_ST(cn)==116||A_ST(cn)==120||A_ST(cn)==124||A_ST(cn)==128||A_ST(cn)==132||A_ST(cn)==136||A_ST(cn)==140||A_ST(cn)==144||A_ST(cn)==148||A_ST(cn)==152)
#define IS_CNFIGHTING(cn)		(A_ST(cn)==160||A_ST(cn)==168||A_ST(cn)==176||A_ST(cn)==184)

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

#define IS_MAGICITEM(in)		(it[(in)].flags & IF_MAGIC)
#define IS_UNIQUE(in)			(it[(in)].flags & IF_UNIQUE)
#define IS_QUESTITEM(in)		((it[(in)].flags & IF_SHOPDESTROY) || (it[(in)].flags & IF_LABYDESTROY) || (it[(in)].flags & IF_NODEPOT))
#define IS_GEMSTONE(in)			(it[(in)].flags & IF_GEMSTONE)
#define IS_SOULSTONE(in)		(it[(in)].driver== 68)
#define IS_SOULFOCUS(in)		(it[(in)].driver== 92)
#define IS_SOULCAT(in)			(it[(in)].driver== 93)
#define IS_GSCROLL(in)			(it[(in)].driver==110)
#define IS_CORRUPTOR(in)		(it[(in)].driver==133)
#define IS_TAROT(in)			((it[(in)].temp>=IT_CH_FOOL && it[(in)].temp<=IT_CH_WORLD) || (it[(in)].temp>=IT_CH_FOOL_R && it[(in)].temp<=IT_CH_WORLD_R))
#define IS_CONTRACT(in)			(it[(in)].temp==MCT_CONTRACT)
#define IS_QUILL(in)			(it[(in)].temp==MCT_QUILL_Y||it[(in)].temp==MCT_QUILL_G||it[(in)].temp==MCT_QUILL_B||it[(in)].temp==MCT_QUILL_R)

#define IS_MATCH_CAT(in, in2)	(IS_SOULCAT(in)   && it[(in)].data[4] != it[(in2)].data[4])
#define IS_MATCH_GSC(in, in2)	(IS_GSCROLL(in)   && it[(in)].data[1] != it[(in2)].data[1] && it[(in)].data[0] == 5 && it[(in2)].data[0] == 5)
#define IS_MATCH_COR(in, in2)	(IS_CORRUPTOR(in) && it[(in)].data[0] != it[(in2)].data[0] && it[(in)].data[0] != 0 && it[(in2)].data[0] != 0)

int is_apotion(int in);
int is_ascroll(int in);

#define IS_POTION(in)			(is_apotion(in))
#define IS_SCROLL(in)			(is_ascroll(in))

#define IS_WPDAGGER(in)			((it[(in)].flags & IF_WP_DAGGER) && !(it[(in)].flags & IF_WP_STAFF))
#define IS_WPSTAFF(in)			((it[(in)].flags & IF_WP_STAFF) && !(it[(in)].flags & IF_WP_DAGGER))
#define IS_WPSPEAR(in)			((it[(in)].flags & IF_WP_DAGGER) && (it[(in)].flags & IF_WP_STAFF))
#define IS_WPSHIELD(in)			(it[(in)].flags & IF_OF_SHIELD)
#define IS_WPSWORD(in)			(it[(in)].flags & IF_WP_SWORD)
#define IS_WPDUALSW(in)			(it[(in)].flags & IF_OF_DUALSW)
#define IS_WPCLAW(in)			(it[(in)].flags & IF_WP_CLAW)
#define IS_WPAXE(in)			((it[(in)].flags & IF_WP_AXE) && !(it[(in)].flags & IF_WP_TWOHAND))
#define IS_WPTWOHAND(in)		((it[(in)].flags & IF_WP_TWOHAND) && !(it[(in)].flags & IF_WP_AXE))
#define IS_WPGAXE(in)			((it[(in)].flags & IF_WP_AXE) && (it[(in)].flags & IF_WP_TWOHAND))

#define IS_EQHEAD(in)			(it[(in)].placement & PL_HEAD)
#define IS_EQNECK(in)			(it[(in)].placement & PL_NECK)
#define IS_EQBODY(in)			(it[(in)].placement & PL_BODY)
#define IS_EQARMS(in)			(it[(in)].placement & PL_ARMS)
#define IS_EQBELT(in)			(it[(in)].placement & PL_BELT)
#define IS_EQCHARM(in)			(it[(in)].placement & PL_CHARM)
#define IS_EQFEET(in)			(it[(in)].placement & PL_FEET)
#define IS_EQWEAPON(in)			(it[(in)].placement & PL_WEAPON)
#define IS_EQDUALSW(in)			((it[(in)].placement & PL_SHIELD) && (it[(in)].flags & IF_OF_DUALSW))
#define IS_EQSHIELD(in)			((it[(in)].placement & PL_SHIELD) && (it[(in)].flags & IF_OF_SHIELD))
#define IS_EQCLOAK(in)			(it[(in)].placement & PL_CLOAK)
#define IS_EQRING(in)			(it[(in)].placement & PL_RING)


#define IS_TWOHAND(in)			(it[(in)].placement & PL_TWOHAND)
#define IS_OFFHAND(in)			(it[(in)].placement & PL_SHIELD)
#define IS_USETWOHAND(cn)		(it[ch[(cn)].worn[WN_RHAND]].placement & PL_TWOHAND)

#define IS_SOULSTONED(in)		(it[(in)].flags & IF_SOULSTONE)
#define IS_ENCHANTED(in)		(it[(in)].flags & IF_ENCHANTED)
#define IS_SOULCHANTED(in)		(IS_SOULSTONED(in) && IS_ENCHANTED(in))

#define IS_ONLYONERING(in)		((it[(in)].temp>=IT_SIGNET_TE&&it[(in)].temp<=IT_SIGN_SKUA)||(it[(in)].temp>=IT_SIGN_SHOU&&it[(in)].temp<=IT_SIGN_SCRE)||it[(in)].temp==IT_ICELOTUS)

#define IS_SKUAWEAP(in)			((it[(in)].flags & IF_KWAI_UNI) &&  (it[(in)].flags & IF_GORN_UNI) && !(it[(in)].flags & IF_PURP_UNI))
#define IS_GORNWEAP(in)			((it[(in)].flags & IF_GORN_UNI) && !(it[(in)].flags & IF_KWAI_UNI) && !(it[(in)].flags & IF_PURP_UNI))
#define IS_KWAIWEAP(in)			((it[(in)].flags & IF_KWAI_UNI) && !(it[(in)].flags & IF_GORN_UNI) && !(it[(in)].flags & IF_PURP_UNI))
#define IS_PURPWEAP(in)			((it[(in)].flags & IF_PURP_UNI) && !(it[(in)].flags & IF_GORN_UNI) && !(it[(in)].flags & IF_KWAI_UNI))
#define IS_OSIRWEAP(in)			((it[(in)].flags & IF_PURP_UNI) &&  (it[(in)].flags & IF_GORN_UNI) && !(it[(in)].flags & IF_KWAI_UNI))
#define IS_GODWEAPON(in)		(IS_SKUAWEAP(in) || IS_GORNWEAP(in) || IS_KWAIWEAP(in) || IS_PURPWEAP(in))

#define IS_RANSACKGEAR(in)		(IS_EQNECK(in) || IS_EQBELT(in) || IS_EQRING(in) || IS_SOULSTONED(in) || IS_ENCHANTED(in) || IS_UNIQUE(in))
#define IS_MAGICDROP(in)		(IS_MAGICITEM(in) && IS_RANSACKGEAR(in))


/* *** TEMPLATES *** */

// Sanity checks on item templates
#define IS_SANEITEMPLATE(tn) 	((tn) > 0 && (tn) < MAXTITEM)
#define IS_SANECTEMPLATE(tn) 	((tn) > 0 && (tn) < MAXTCHARS)

#define IS_LONG_RESPAWN(temp) 	(temp==CT_RATKING || temp==CT_GREENKING || temp==CT_DREADKING || temp==CT_LIZEMPEROR || temp==CT_VILEQUEEN || temp==CT_BSMAGE1 || temp==CT_BSMAGE2 || temp==CT_BSMAGE3 || temp==CT_SCORP_Q)

/* *** CHARACTERS *** */

#define WILL_FIGHTBACK(cn)		(!(ch[(cn)].flags & CF_FIGHT_OFF) && ch[(cn)].misc_action != DR_GIVE)

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
#define IS_RB(cn)				((ch[(cn)].rebirth & 1) != 0)

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
#define IS_LYCANTH(cn)			(ch[(cn)].kindred & KIN_LYCANTH)
#define IS_SHIFTED(cn)			(ch[(cn)].kindred & KIN_SHIFTED)

#define IS_SHADOW(cn)			(ch[(cn)].kindred & KIN_SHADOW)
#define IS_BLOODY(cn)			(ch[(cn)].kindred & KIN_BLOODY)

#define IS_ANY_TEMP(cn)			(IS_TEMPLAR(cn) || IS_ARCHTEMPLAR(cn) || IS_SKALD(cn))
#define IS_ANY_MERC(cn)			(IS_MERCENARY(cn) || IS_WARRIOR(cn) || IS_SORCERER(cn))
#define IS_ANY_HARA(cn)			(IS_HARAKIM(cn) || IS_SUMMONER(cn) || IS_ARCHHARAKIM(cn))

#define IS_ANY_ARCH(cn)			(IS_SEYAN_DU(cn) || IS_ARCHTEMPLAR(cn) || IS_SKALD(cn) || IS_WARRIOR(cn) || IS_SORCERER(cn) || IS_SUMMONER(cn) || IS_ARCHHARAKIM(cn) || IS_BRAVER(cn) || IS_LYCANTH(cn))

#define IS_SEYA_OR_ARTM(cn)		(IS_SEYAN_DU(cn) || IS_ARCHTEMPLAR(cn))
#define IS_SEYA_OR_SKAL(cn)		(IS_SEYAN_DU(cn) || IS_SKALD(cn))
#define IS_SEYA_OR_WARR(cn)		(IS_SEYAN_DU(cn) || IS_WARRIOR(cn))
#define IS_SEYA_OR_SORC(cn)		(IS_SEYAN_DU(cn) || IS_SORCERER(cn))
#define IS_SEYA_OR_SUMM(cn)		(IS_SEYAN_DU(cn) || IS_SUMMONER(cn))
#define IS_SEYA_OR_ARHR(cn)		(IS_SEYAN_DU(cn) || IS_ARCHHARAKIM(cn))
#define IS_SEYA_OR_BRAV(cn)		(IS_SEYAN_DU(cn) || IS_BRAVER(cn))
#define IS_SEYA_OR_LYCA(cn)		(IS_SEYAN_DU(cn) || IS_LYCANTH(cn))

#define IS_PROX_CLASS(cn)		(B_SK(cn, SK_PROX) || IS_SEYAN_DU(cn) || IS_ARCHTEMPLAR(cn) || IS_WARRIOR(cn))
#define CAN_ARTM_PROX(cn)		(IS_SEYA_OR_ARTM(cn) && !(ch[(cn)].flags & CF_AREA_OFF))
#define CAN_WARR_PROX(cn)		(IS_SEYA_OR_WARR(cn) && !(ch[(cn)].flags & CF_AREA_OFF))
#define CAN_SORC_PROX(cn)		(IS_SEYA_OR_SORC(cn) && !(ch[(cn)].flags & CF_AREA_OFF) && M_SK(cn, SK_PROX) > 45)
#define CAN_ARHR_PROX(cn)		(IS_SEYA_OR_ARHR(cn) && !(ch[(cn)].flags & CF_AREA_OFF) && M_SK(cn, SK_PROX) > 45)
#define CAN_BRAV_PROX(cn)		(IS_SEYA_OR_BRAV(cn) && !(ch[(cn)].flags & CF_AREA_OFF) && M_SK(cn, SK_PROX) > 45)

#define IS_LABY_MOB(cn)			(ch[(cn)].data[CHD_GROUP]==13)

/* *** SKILLS *** */

// Sanity check on skill number
#define IS_SANESKILL(s) 		((s)>=0 && (s)<MAXSKILL)

// Fancy get/setters
#define B_AT(cn, a)				(ch[(cn)].attrib[(a)][0])
#define M_AT(cn, a)				(get_attrib_score((cn), (a)))
#define B_SK(cn, s)				(ch[(cn)].skill[(s)][0])
#define M_SK(cn, s)				((s)==SK_PERCEPT?(get_skill_score((cn), (s))*(HAS_ENCHANT(ch[(cn)].worn[WN_HEAD], 52)?4:3)/3):get_skill_score((cn), (s)))

#define T_SK(cn, a)				(IS_SANECHAR(cn)    && st_skillnum((cn), (a), (-1)))
#define T_SKT(cn, a)			(IS_SANECHAR(cn)    && st_skillnum((cn), (a), (-2)))
#define T_SEYA_SK(cn, a)		(IS_SEYAN_DU(cn)    && T_SKT((cn), (a)))
#define T_ARTM_SK(cn, a)		(IS_ARCHTEMPLAR(cn) && T_SKT((cn), (a)))
#define T_SKAL_SK(cn, a)		(IS_SKALD(cn)       && T_SKT((cn), (a)))
#define T_WARR_SK(cn, a)		(IS_WARRIOR(cn)     && T_SKT((cn), (a)))
#define T_SORC_SK(cn, a)		(IS_SORCERER(cn)    && T_SKT((cn), (a)))
#define T_SUMM_SK(cn, a)		(IS_SUMMONER(cn)    && T_SKT((cn), (a)))
#define T_ARHR_SK(cn, a)		(IS_ARCHHARAKIM(cn) && T_SKT((cn), (a)))
#define T_BRAV_SK(cn, a)		(IS_BRAVER(cn)      && T_SKT((cn), (a)))
#define T_LYCA_SK(cn, a)		(IS_LYCANTH(cn)     && T_SKT((cn), (a)))
#define T_OS_TREE(cn, a)		(IS_SANEPLAYER(cn)  && st_learned_skill(ch[(cn)].os_tree, (a)))

#define IS_P_SKILL(a)			(a==8||a==9||a==23||a==32)

#define CAN_SENSE(cn)			((ch[(cn)].flags & CF_SENSE) || !IS_PLAYER(cn))

#define SP_SUPPRESS(p, cn, co)	(max(0,(p-(p*(M_AT(co,AT_WIL)-M_AT(cn, AT_WIL))/500+M_SK(co,SK_RESIST)/20))/2))

// Slow's formula (used to degrade)
#define SLOWFORM(n)				(n/2*9/10)

// Slow2's formula (used to degrade) 
#define SLOW2FORM(n)			(n/3*9/10)

// Curse2's formula (used to degrade)
#define CURSE2FORM(p, n)		(((p*5/3)-n)/5)

// Poison's formula (damage per tick)
#define PL_POISFORM(p, d)		(((p+ 5) * 4500) / max(1, d))
#define MN_POISFORM(p, d)		(((p   ) * 3000) / max(1, d))

// Bleed's formula (damage per tick)
#define BLEEDFORM(p, d)			(((p+ 5) *  750) / max(1, d))

// Plague's formula
#define PLAGUEFORM(p, d)		(((p+ 5) * 2000) / max(1, d))

// Frostburn's formula (degen per tick)
#define FROSTBFORM(p, d)		(((p+10) * 1000) / max(1, d))

#define IS_DISPELABLE1(tmp)		((tmp)==SK_BLIND || (tmp)==SK_WARCRY2 || (tmp)==SK_CURSE2 || (tmp)==SK_CURSE || (tmp)==SK_WARCRY || (tmp)==SK_WEAKEN2 || (tmp)==SK_WEAKEN || (tmp)==SK_SLOW2 || (tmp)==SK_SLOW || (tmp)==SK_DOUSE || (tmp)==SK_AGGRAVATE || (tmp)==SK_SCORCH || (tmp)==SK_DISPEL2)
#define IS_DISPELABLE2(tmp)		((tmp)==SK_HASTE || (tmp)==SK_BLESS || (tmp)==SK_MSHIELD || (tmp)==SK_MSHELL || (tmp)==SK_PULSE || (tmp)==SK_ZEPHYR || (tmp)==SK_GUARD || (tmp)==SK_DISPEL || (tmp)==SK_REGEN || (tmp)==SK_PROTECT || (tmp)==SK_ENHANCE || (tmp)==SK_LIGHT)

/* Tarot Card Descriptiors (for both r-click and /tarot command) */

#define DESC_FOOL		"You can apply a soulstone to this card. Once applied, it will contribute its bonuses while equipped.\n"
#define DESC_MAGI		"When equipped, secondary effects of Intuition and Strength are equal to the higher of the two.\n"
#define DESC_PREIST		"When equipped, 20%% of damage taken from hits is dealt to Mana instead.\n"
#define DESC_EMPRESS	"When equipped, your Magic Shield spell is replaced with Magic Shell. Magic Shell grants a temporary Resistance and Immunity bonus.\n"
#define DESC_EMPEROR	"When equipped, your Slow spell is replaced with Greater Slow. Greater Slow no longer decays and has an increased duration.\n"
#define DESC_HEIROPH	"When equipped, Immunize and Inoculate from your Dispel spell lasts four times as long, but your Dispel spell can only remove a single buff or debuff at a time.\n"
#define DESC_LOVERS		"When equipped, your Weapon Value and Armor Value become the average of your Weapon Value and Armor Value.\n"
#define DESC_CHARIOT	"When equipped, your Blind skill is replaced with Douse. Douse reduces your target's stealth and spell modifier.\n"
#define DESC_STRENGTH	"When equipped, reduces your attack speed, cast speed, and cooldown recovery speed by 15%%, but grants 20%% more damage with hits.\n"
#define DESC_HERMIT		"When equipped, you have 15%% more Armor Value, but 20%% less Resistance and Immunity.\n"
#define DESC_WHEEL		"When equipped, your critical hit chance is reduced by 33%%, but you have 33%% more critical hit multiplier.\n"
#define DESC_JUSTICE	"When equipped, your Cleave skill no longer inflicts a Bleeding, and instead inflicts Aggravate, causing the target to take additional damage for 20 seconds.\n"
#define DESC_HANGED		"When equipped, 33%% of your Resistance is instead used to reduce the strength of incoming enemy spells.\n"
#define DESC_DEATH		"When equipped, your Weaken skill is replaced with Crush. Crush reduces a target's Armor Value, but no longer reduces enemy Weapon Value.\n"
#define DESC_TEMPER		"When equipped, your Taunt skill grants 100%% more Guard power, but Guard duration is halved.\n"
#define DESC_DEVIL		"When equipped, 33%% of all skill and spell costs are instead taken from your Hitpoints.\n"
#define DESC_TOWER		"When equipped, your Curse spell is replaced with Greater Curse. Greater Curse has increased effect, but decays over time and has a reduced duration.\n"
#define DESC_STAR		"When equipped, your Heal spell is replaced with Regen. Regen grants a buff which regenerates the target's Hitpoints over 20 seconds.\n"
#define DESC_MOON		"When equipped, life regeneration is instead applied as mana regeneration while not at full mana.\n"
#define DESC_SUN		"When equipped, endurance regeneration is instead applied as life regeneration while not at full life.\n"
#define DESC_JUDGE		"When equipped, your Blast spell deals 15%% less damage but inflicts Scorch, causing the target to take additional damage for 20 seconds.\n"
#define DESC_WORLD		"When equipped, mana regeneration is instead applied as endurance regeneration while not at full endurance.\n"

#define DESC_FOOL_R		"When equipped, your attributes become the average of all attributes, plus 8%%.\n"
#define DESC_MAGI_R		"When equipped, your Economize skill no longer reduces the Mana cost of skills, and instead increases your Cooldown rate.\n"
#define DESC_PREIST_R	"When equipped, your Magic Shield and Magic Shell are now passive and regenerate quickly, but are more fragile. You cannot gain Magic Shield or Magic Shell from other sources.\n"
#define DESC_EMPRES_R	"When equipped, your Lethargy skill costs life over time instead of mana over time.\n"
#define DESC_EMPERO_R	"When equipped, your Warcry skill is replaced with Rally. Rally grants nearby allies a buff which improves Hit Score and Parry Score.\n"
#define DESC_HEIROP_R	"When equipped, your Ghost Companion has 12%% more Weapon Value and Armor Value, but has a 20%% chance to miss when it should have hit.\n"
#define DESC_LOVERS_R	"When equipped, your Hit Score and Parry Score become the average of your Hit Score and Parry Score.\n"
#define DESC_CHARIO_R	"When equipped, your debuffs ignore 25%% of target resistance and immunity, but are 20%% weaker once applied.\n"
#define DESC_STRENG_R	"When equipped, you have 20%% more Weapon Value, but 20%% less hit score.\n"
#define DESC_HERMIT_R	"When equipped, your Rage and Calm skills instead cost endurance over time.\n"
#define DESC_WHEEL_R	"When equipped, you take 20%% less damage from melee attacks, but have a 25%% chance to be hit when you would have parried.\n"
#define DESC_JUSTIC_R	"When equipped, your Leap skill no longer repeats. Leap now always deals a critical hit and stuns everything it hits.\n"
#define DESC_HANGED_R	"When equipped, you have 24%% more Top Damage, but 12%% less Weapon Value.\n"
#define DESC_DEATH_R	"When equipped, your Zephyr skill no longer triggers on hit, and instead triggers when an enemy attacks you.\n"
#define DESC_TEMPER_R	"When equipped, you gain 6.25%% more Weapon Value per stack of Healing Sickness on you. The maximum healing sickness you can receive is increased by 1 stack.\n"
#define DESC_DEVIL_R	"When equipped, your Shadow Copy deals 25%% more damage and takes 25%% less damage, but while your Shadow Copy is active you deal 20%% less damage and take 20%% more damage.\n"
#define DESC_TOWER_R	"When equipped, your Poison spell is replaced with Venom. Venom deals half as much damage, but it reduces enemy Immunity and can stack up to three times.\n"
#define DESC_STAR_R		"When equipped, your base Spell Modifier is 0.90. Your Spell Modifier no longer effects spell power and instead effects skill power.\n"
#define DESC_MOON_R		"When equipped, your Tactics skill has 1%% increased effect per 50 uncapped mana, but no longer grants its bonus at full mana and instead grants it at low mana. You lose 0.2%% of current mana per second per 50 uncapped mana.\n"
#define DESC_SUN_R		"When equipped, the effectiveness of your Regenerate, Rest, and Meditate skills behave as if stationary while fighting, but as if fighting while stationary.\n"
#define DESC_JUDGE_R	"When equipped, your Pulse spell no longer deals damage to enemies and instead heals allies with each pulse. It inflicts Charge instead of Shock to allies, granting them additional damage and damage reduction.\n"
#define DESC_WORLD_R	"When equipped, 50%% of damage taken is dealt to Endurance instead. All Endurance costs instead use Mana, and all skills grant Endurance on use. You lose 40%% of current endurance per second, mitigated by your Rest skill.\n"

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

#define IS_IN_SKUA(x, y)	((x>= 499&&x<= 531&&y>= 504&&y<= 520)||(x>= 505&&x<= 519&&y>= 492&&y<= 535))
#define IS_IN_GORN(x, y)	((x>= 773&&x<= 817&&y>= 780&&y<= 796)||(x>= 787&&x<= 803&&y>= 775&&y<= 812))
#define IS_IN_KWAI(x, y)	((x>= 685&&x<= 729&&y>= 848&&y<= 864)||(x>= 699&&x<= 715&&y>= 832&&y<= 868))
#define IS_IN_PURP(x, y)	((x>= 549&&x<= 585&&y>= 448&&y<= 462)||(x>= 564&&x<= 575&&y>= 463&&y<= 474))
#define IS_IN_TEMPLE(x, y)	(IS_IN_SKUA(x, y) || IS_IN_GORN(x, y) || IS_IN_KWAI(x, y) || IS_IN_PURP(x, y))

#define IS_IN_BRAV(x, y)	((x>= 884&&y>= 504&&x<= 964&&y<= 534))
#define IS_IN_SUN(x, y) 	((x>=  32&&y>= 407&&x<=  57&&y<= 413)||(x>=  32&&y>= 414&&x<=  64&&y<= 428)||(x>=  22&&y>= 429&&x<=  64&&y<= 450)||(x>=  22&&y>= 451&&x<=  27&&y<= 459)||(x>=  59&&y>= 451&&x<=  64&&y<= 465)||(x>= 173&&y>= 921&&x<= 255&&y<=1003))
#define IS_IN_ABYSS(x, y)	((x>=438&&y>=110&&x<=470&&y<=142)?1:((x>=438&&y>=148&&x<=470&&y<=180)?2:((x>=476&&y>=148&&x<=508&&y<=180)?3:((x>=476&&y>=110&&x<=508&&y<=142)?4:((x>=476&&y>= 72&&x<=508&&y<=104)?5:((x>=476&&y>= 34&&x<=508&&y<= 66)?6:((x>=514&&y>= 34&&x<=546&&y<= 66)?7:((x>=514&&y>= 72&&x<=546&&y<=104)?8:((x>=514&&y>=110&&x<=546&&y<=142)?9:((x>=523&&y>=148&&x<=537&&y<=180)?10:0))))))))))
#define IS_IN_IX(x, y) 		((x>=  21&&y>= 657&&x<= 125&&y<= 706))
#define IS_IN_XIII(x, y) 	((x>= 132&&y>= 153&&x<= 152&&y<= 240))
#define IS_IN_VANTA(x, y) 	((x>=  94&&y>=1135&&x<= 182&&y<=1223))
#define IS_IN_XVIII(x, y) 	((x>=  94&&y>=1474&&x<= 171&&y<=1564))
#define IS_IN_XIX(x, y) 	((x>=  94&&y>=1587&&x<= 260&&y<=1753))
#define IS_IN_ZRAK(x, y) 	((x>=  94&&y>=1587&&x<= 185&&y<=1670))
#define IS_IN_BOJ(x, y) 	((x>= 203&&y>=1587&&x<= 260&&y<=1670))
#define IS_IN_CAROV(x, y) 	((x>= 178&&y>=1687&&x<= 260&&y<=1753))
#define IS_IN_XX(x, y) 		((x>= 194&&y>=1474&&x<= 214&&y<=1533))
#define IS_IN_TLG(x, y) 	((x>= 195&&y>=1545&&x<= 213&&y<=1563))
#define IS_IN_SANG(x, y)	((x>= 888&&y>=1027&&x<= 989&&y<=2013))
#define IS_IN_DW(x, y)		((x>=  21&&y>=1776&&x<= 273&&y<=2028))
#define IS_IN_INDW(x, y)	((x>=  24&&y>=1779&&x<= 270&&y<=2025))
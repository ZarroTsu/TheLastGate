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



/* *** TEMPLATES *** */

// Sanity checks on item templates
#define IS_SANEITEMPLATE(tn) ((tn) > 0 && (tn) < MAXTITEM)
#define IS_SANECTEMPLATE(tn) ((tn) > 0 && (tn) < MAXTCHARS)



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
#define IS_BRAWLER(cn)		(ch[(cn)].kindred & KIN_BRAWLER)
#define IS_WARRIOR(cn)		(ch[(cn)].kindred & KIN_WARRIOR)
#define IS_SORCERER(cn)		(ch[(cn)].kindred & KIN_SORCERER)
#define IS_SUMMONER(cn)		(ch[(cn)].kindred & KIN_SUMMONER)
#define IS_ARCHHARAKIM(cn)	(ch[(cn)].kindred & KIN_ARCHHARAKIM)

#define IS_ANY_TEMP(cn)		(IS_TEMPLAR(cn) || IS_ARCHTEMPLAR(cn) || IS_BRAWLER(cn))
#define IS_ANY_MERC(cn)		(IS_MERCENARY(cn) || IS_WARRIOR(cn) || IS_SORCERER(cn))
#define IS_ANY_HARA(cn)		(IS_HARAKIM(cn) || IS_SUMMONER(cn) || IS_ARCHHARAKIM(cn))

#define IS_ANY_ARCH(cn)		(IS_SEYAN_DU(cn) || IS_ARCHTEMPLAR(cn) || IS_BRAWLER(cn) || IS_WARRIOR(cn) || IS_SORCERER(cn) || IS_SUMMONER(cn) || IS_ARCHHARAKIM(cn))

#define IS_SEYA_OR_ARTM(cn)	(IS_SEYAN_DU(cn) || IS_ARCHTEMPLAR(cn))
#define IS_SEYA_OR_BRWL(cn)	(IS_SEYAN_DU(cn) || IS_BRAWLER(cn))
#define IS_SEYA_OR_WARR(cn)	(IS_SEYAN_DU(cn) || IS_WARRIOR(cn))
#define IS_SEYA_OR_SORC(cn)	(IS_SEYAN_DU(cn) || IS_SORCERER(cn))
#define IS_SEYA_OR_SUMM(cn)	(IS_SEYAN_DU(cn) || IS_SUMMONER(cn))
#define IS_SEYA_OR_ARHR(cn)	(IS_SEYAN_DU(cn) || IS_ARCHHARAKIM(cn))

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
#define POISONFORM(p, d)	((p * 1500) / d)

// Bleed's formula (damage per tick)
#define BLEEDFORM(p, d)		((p * 1250) / d)

// Frostburn's formula (degen per tick)
#define FROSTBFORM(p, d)	((p * 1000) / d)


/* *** CASINO *** */

#define TOKEN_RATE			10

#define C_CUR_GAME(a)		(ch[(a)].data[26]>>26)
#define C_CUR_WAGER(a)		(ch[(a)].data[27]>>26)

#define C_GAME_HR			1
#define C_GAME_SE			2
#define C_GAME_BJ			3

#define C_SET_GAME_HR		(1<<26)
#define C_SET_GAME_SE		(2<<26)
#define C_SET_GAME_BJ		(3<<26)

#define BJ_NUM_CARDS		26

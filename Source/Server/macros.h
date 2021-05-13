/*************************************************************************

   This file is part of 'Mercenaries of Astonia v2'
   Copyright (c) 1997-2001 Daniel Brockhaus (joker@astonia.com)
   All rights reserved.

 **************************************************************************/

#define ARRAYSIZE(a) (sizeof(a)/sizeof(a[0]))

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
#define IS_COMPANION(cn) (IS_SANECHAR(cn) && (ch[(cn)].temp == CT_COMPANION || ch[(cn)].temp == CT_ARCHCOMP || ch[(cn)].temp == CT_CASTERCOMP || ch[(cn)].temp == CT_ARCHCASTER))
#define IS_COMP_TEMP(cn) (ch[(cn)].temp == CT_COMPANION || ch[(cn)].temp == CT_ARCHCOMP || ch[(cn)].temp == CT_CASTERCOMP || ch[(cn)].temp == CT_ARCHCASTER)

// Visibility, etc.
#define IS_INVISIBLE(cn)   ((ch[(cn)].flags & CF_INVISIBLE) != 0)
#define IS_PURPLE(cn)      ((ch[(cn)].kindred & KIN_PURPLE) != 0)
#define IS_FEMALE(cn)      ((ch[cn].kindred & KIN_FEMALE) != 0)
#define HE_SHE(cn)         (IS_FEMALE(cn) ? "she" : "he")
#define HE_SHE_CAPITAL(cn) (IS_FEMALE(cn) ? "She" : "He")
#define HIS_HER(cn)        (IS_FEMALE(cn) ? "her" : "his")
#define HIM_HER(cn)        (IS_FEMALE(cn) ? "her" : "him")

// Ditto, with sanity check
#define IS_SANEPLAYER(cn) (IS_SANECHAR(cn) && IS_PLAYER(cn))
#define IS_SANESTAFF(cn)  (IS_SANECHAR(cn) && IS_STAFF(cn))
#define IS_SANEGOD(cn)    (IS_SANECHAR(cn) && IS_GOD(cn))
#define IS_SANEUSURP(cn)  (IS_SANECHAR(cn) && IS_USURP(cn))
// IS_SANENPC is derived. No IS_NPC because of... logic.
#define IS_SANENPC(cn) (IS_SANECHAR(cn) && !IS_PLAYER(cn))
#define IS_SANECCP(cn) (IS_SANECHAR(cn) && IS_CCP(cn))


/* *** SKILLS *** */

// Sanity check on skill number
#define SANESKILL(s) ((s)>=0 && (s)<MAXSKILL)

// Haste's formula
#define HASTEFORM(n)		(n/4)
// Slow's formula (used to degrade)
#define SLOWFORM(n)			(n/2)
// Slow2's formula (used to degrade) 
#define SLOW2FORM(n)		(n/(8/3))
// Curse2's formula (used to degrade)
#define CURSE2FORM(p, n)	(((p*4/3)-n)/5)
// Poison's formula (damage per tick)
#define POISONFORM(p, d)	((p * 1500) / d)
// Bleed's formula (damage per tick)
#define BLEEDFORM(p, d)		((p * 1000) / d)

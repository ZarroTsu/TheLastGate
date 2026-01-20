#pragma once

#include "common.h"
EXTERN_C_BEGIN

extern int noshop;
extern int pskip, pidle;

extern int st_learned_skill(int st_val, int v);

extern int stat_points_used;
extern struct cplayer pl;
extern char *at_name[5];
extern struct skilltab *skilltab;
extern struct skilltab *raw_skilltab;
extern int skill_needed(int n,int v);
extern int attrib_needed(int n,int v);
extern int stat_raised[108];
extern void cmd_options(void);
extern int points2rank(int v);
extern int rank2points(int v);
extern int points_tolevel(int curr_exp);
void update_skill_tab_from_raw(void);
void xlog(char font,char *format,...);
int at_score(int n);
int sk_score(int n);
int points2rank(int v);

#define AT_BRV		0
#define AT_WIL		1
#define AT_INT		2
#define AT_AGL		3
#define AT_STR		4

#ifndef GUI_SHOP_X
    #define GUI_SHOP_X		((1280/2)-(320/2))
    #define GUI_SHOP_Y		((736/2)-(320/2)+72)
#endif

EXTERN_C_END
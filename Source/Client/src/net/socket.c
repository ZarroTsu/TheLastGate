#include <stdio.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>
#include <zlib.h>
#include <SDL2/SDL_timer.h>
#include <SDL2/SDL_net.h>

#include "engine.h"
#include "game/game_input.h"
#include "game/game_ui.h"
#include "graphics/render.h"
#include "input/input.h"
#include "log/log.h"
#include "mods/stubborn_actions.h"
#include "mods/give_more.h"
#include "mods/use_queue.h"
#include "security/security.h"
#include "util/math_util.h"

struct z_stream_s zs;

#include "../../common.h"
#include "../../inter.h"

char passwd[15]={0};

#define DEBUG(a)
//printf("%d: %s\n",__cnt++,a); fflush(stdout)
//static int __cnt=0;
#define DEBUG2(a)
//xlog(1,a)

static struct look tmplook;
struct look shop;

extern int show_look,look_timer;

TCPsocket sock = NULL;
SDLNet_SocketSet socket_set = NULL;

int t_size=0;	// ticks in queue

int ser_ver=0;

int ticker=0;

extern short screen_renderdist;

extern char host_addr[];
extern char host_proxy[];
extern int host_port;

extern struct key okey;
extern int do_exit;

int sv_cmd(unsigned char *buf);
void sv_newplayer(unsigned char *buf);
unsigned int xcrypt(unsigned int val);

int so_status=0;

char *logout_reason[]={
"unknown",                                          //0
"Client failed challenge.",                         //1
"Client was idle too long.",                        //2
"No room to drop character.",                       //3
"Invalid parameters.",                              //4
"Character already active or no player character.", //5
"Invalid password.",                                //6
"Client too slow.",                                 //7
"Receive failure.",                                 //8
"Server is being shutdown.",                        //9
"You entered a Tavern.",                            //10
"Client version too old. Update needed.",           //11
"Aborting on user request.",                        //12
"this should never show up",                        //13
"You have been banned for an hour. Enhance your social behavior before you come back."                 //14
};

int xrecv(TCPsocket sock, char *buf, int len, int flags) {
	int ret, size = 0;

	while (size < len) {
		ret = SDLNet_TCP_Recv(sock, buf, len);
		if (ret < 1) return size;
		size += ret;
	}

	return size;
}

void so_error(const char *err)
{
	deinit();

	save_options();

	if (!do_exit) {
		log_error("Networking Error: %s", err);
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR,"Irregular Exit",err, NULL);
	}

	exit(0);
}

// ---------------------------------

static char secret[256]={"\
Ifhjf64hH8sa,-#39ddj843tvxcv0434dvsdc40G#34Trefc349534Y5#34trecerr943\
5#erZt#eA534#5erFtw#Trwec,9345mwrxm gerte-534lMIZDN(/dn8sfn8&DBDB/D&s\
8efnsd897)DDzD'D'D''Dofs,t0943-rg-gdfg-gdf.t,e95.34u.5retfrh.wretv.56\
9v4#asf.59m(D)/ND/DDLD;gd+dsa,fw9r,x  OD(98snfsfa"};

unsigned int xcrypt(unsigned int val)
{
	unsigned int res=0;

	res+=(unsigned int)(secret[ val     &255]);
	res+=(unsigned int)(secret[(val>>8 )&255])<<8;
	res+=(unsigned int)(secret[(val>>16)&255])<<16;
	res+=(unsigned int)(secret[(val>>24)&255])<<24;

	res^=0x5a7ce52e;

	return res;
}

int sv_terminology(unsigned char *buf) {
	int tn = -1, n = 0;

	DEBUG("SV TERMINOLOGY");

	if (buf[0] == SV_TERM_STREE) tn = 0;
	if (buf[0] == SV_TERM_CTREE) tn = 1;

	if (tn >= 0) {
		n = buf[2];

		switch (buf[1]) {
			case ST_TREE_ICON:   sk_tree[tn][n].icon = *(unsigned short*)(buf+3); return  5;
			case ST_TREE_NAME1:  memcpy(sk_tree[tn][n].name,    buf+3, 10);       return 13;
			case ST_TREE_NAME2:  memcpy(sk_tree[tn][n].name+10, buf+3, 10);       return 13;
			case ST_TREE_NAME3:  memcpy(sk_tree[tn][n].name+20, buf+3, 10);       return 13;
			case ST_TREE_DESC1A: memcpy(sk_tree[tn][n].dsc1,    buf+3, 10);       return 13;
			case ST_TREE_DESC1B: memcpy(sk_tree[tn][n].dsc1+10, buf+3, 10);       return 13;
			case ST_TREE_DESC1C: memcpy(sk_tree[tn][n].dsc1+20, buf+3, 10);       return 13;
			case ST_TREE_DESC1D: memcpy(sk_tree[tn][n].dsc1+30, buf+3, 10);       return 13;
			case ST_TREE_DESC1E: memcpy(sk_tree[tn][n].dsc1+40, buf+3, 10);       return 13;
			case ST_TREE_DESC2A: memcpy(sk_tree[tn][n].dsc2,    buf+3, 10);       return 13;
			case ST_TREE_DESC2B: memcpy(sk_tree[tn][n].dsc2+10, buf+3, 10);       return 13;
			case ST_TREE_DESC2C: memcpy(sk_tree[tn][n].dsc2+20, buf+3, 10);       return 13;
			case ST_TREE_DESC2D: memcpy(sk_tree[tn][n].dsc2+30, buf+3, 10);       return 13;
			case ST_TREE_DESC2E: memcpy(sk_tree[tn][n].dsc2+40, buf+3, 10);       return 13;
			default: break;
		}


	}

	if (buf[0]==SV_TERM_SKILLS) {
		n = buf[2];
		switch (buf[1]) {
			case ST_SKILLS_SORT: skilltab[n].sortkey = *(unsigned char *) (buf + 3);
				skilltab[n].attrib[0] = *(unsigned char *) (buf + 4);
				skilltab[n].attrib[1] = *(unsigned char *) (buf + 5);
				skilltab[n].attrib[2] = *(unsigned char *) (buf + 6);
				skilltab[n].show = *(unsigned char *) (buf + 7);
				return 8;
			case ST_SKILLS_NAME1: memcpy(skilltab[n].name,     buf+3, 10); return 13;
			case ST_SKILLS_NAME2: memcpy(skilltab[n].name+ 10, buf+3, 10); return 13;
			case ST_SKILLS_NAME3: memcpy(skilltab[n].name+ 20, buf+3, 10); return 13;
			case ST_SKILLS_DESC01: memcpy(skilltab[n].desc,     buf+3, 10); return 13;
			case ST_SKILLS_DESC02: memcpy(skilltab[n].desc+ 10, buf+3, 10); return 13;
			case ST_SKILLS_DESC03: memcpy(skilltab[n].desc+ 20, buf+3, 10); return 13;
			case ST_SKILLS_DESC04: memcpy(skilltab[n].desc+ 30, buf+3, 10); return 13;
			case ST_SKILLS_DESC05: memcpy(skilltab[n].desc+ 40, buf+3, 10); return 13;
			case ST_SKILLS_DESC06: memcpy(skilltab[n].desc+ 50, buf+3, 10); return 13;
			case ST_SKILLS_DESC07: memcpy(skilltab[n].desc+ 60, buf+3, 10); return 13;
			case ST_SKILLS_DESC08: memcpy(skilltab[n].desc+ 70, buf+3, 10); return 13;
			case ST_SKILLS_DESC09: memcpy(skilltab[n].desc+ 80, buf+3, 10); return 13;
			case ST_SKILLS_DESC10: memcpy(skilltab[n].desc+ 90, buf+3, 10); return 13;
			case ST_SKILLS_DESC11: memcpy(skilltab[n].desc+100, buf+3, 10); return 13;
			case ST_SKILLS_DESC12: memcpy(skilltab[n].desc+110, buf+3, 10); return 13;
			case ST_SKILLS_DESC13: memcpy(skilltab[n].desc+120, buf+3, 10); return 13;
			case ST_SKILLS_DESC14: memcpy(skilltab[n].desc+130, buf+3, 10); return 13;
			case ST_SKILLS_DESC15: memcpy(skilltab[n].desc+140, buf+3, 10); return 13;
			case ST_SKILLS_DESC16: memcpy(skilltab[n].desc+150, buf+3, 10); return 13;
			case ST_SKILLS_DESC17: memcpy(skilltab[n].desc+160, buf+3, 10); return 13;
			case ST_SKILLS_DESC18: memcpy(skilltab[n].desc+170, buf+3, 10); return 13;
			case ST_SKILLS_DESC19: memcpy(skilltab[n].desc+180, buf+3, 10); return 13;
			case ST_SKILLS_DESC20: memcpy(skilltab[n].desc+190, buf+3, 10); return 13;
			default: break;
		}
	}

	if (buf[0]==SV_TERM_META) {
		n = buf[2];

		switch (buf[1]) {
			case ST_META_NAME1:  memcpy(meta_stats[n].name,     buf+3, 10); return 13;
			case ST_META_NAME2:  memcpy(meta_stats[n].name+ 10, buf+3, 10); return 13;
			case ST_META_NAME3:  memcpy(meta_stats[n].name+ 20, buf+3, 10); return 13;
			case ST_META_DESC01: memcpy(meta_stats[n].desc,     buf+3, 10); return 13;
			case ST_META_DESC02: memcpy(meta_stats[n].desc+ 10, buf+3, 10); return 13;
			case ST_META_DESC03: memcpy(meta_stats[n].desc+ 20, buf+3, 10); return 13;
			case ST_META_DESC04: memcpy(meta_stats[n].desc+ 30, buf+3, 10); return 13;
			case ST_META_DESC05: memcpy(meta_stats[n].desc+ 40, buf+3, 10); return 13;
			case ST_META_DESC06: memcpy(meta_stats[n].desc+ 50, buf+3, 10); return 13;
			case ST_META_DESC07: memcpy(meta_stats[n].desc+ 60, buf+3, 10); return 13;
			case ST_META_DESC08: memcpy(meta_stats[n].desc+ 70, buf+3, 10); return 13;
			case ST_META_DESC09: memcpy(meta_stats[n].desc+ 80, buf+3, 10); return 13;
			case ST_META_DESC10: memcpy(meta_stats[n].desc+ 90, buf+3, 10); return 13;
			case ST_META_DESC11: memcpy(meta_stats[n].desc+100, buf+3, 10); return 13;
			case ST_META_DESC12: memcpy(meta_stats[n].desc+110, buf+3, 10); return 13;
			case ST_META_DESC13: memcpy(meta_stats[n].desc+120, buf+3, 10); return 13;
			case ST_META_DESC14: memcpy(meta_stats[n].desc+130, buf+3, 10); return 13;
			case ST_META_DESC15: memcpy(meta_stats[n].desc+140, buf+3, 10); return 13;
			case ST_META_DESC16: memcpy(meta_stats[n].desc+150, buf+3, 10); return 13;
			case ST_META_DESC17: memcpy(meta_stats[n].desc+160, buf+3, 10); return 13;
			case ST_META_DESC18: memcpy(meta_stats[n].desc+170, buf+3, 10); return 13;
			case ST_META_DESC19: memcpy(meta_stats[n].desc+180, buf+3, 10); return 13;
			case ST_META_DESC20: memcpy(meta_stats[n].desc+190, buf+3, 10); return 13;
			case ST_META_VALUES:
				meta_stats[n].value = *(short int *) (buf + 3);
				meta_stats[n].flag = *(unsigned char *) (buf + 5);
				memcpy(meta_stats[n].affix, buf + 6, 8);
				meta_stats[n].font = *(unsigned char *) (buf + 14);
				meta_stats[n].show = *(unsigned char *) (buf + 15);
				return 16;
			default: break;
		}
	}

	return 16; // Should not be reached
}

void sv_newplayer(unsigned char *buf)
{
	DEBUG("SV NEWPLAYER");

	okey.usnr=*(unsigned long*)(buf+1);
	okey.pass1=*(unsigned long*)(buf+5);
	okey.pass2=*(unsigned long*)(buf+9);

	save_options();
}

void sv_setchar_name1(unsigned char *buf)
{
	DEBUG("SV SETCHAR NAME1");
	memcpy(pl.name,buf+1,15);
}

void sv_setchar_name2(unsigned char *buf)
{
	DEBUG("SV SETCHAR NAME2");
	memcpy(pl.name+15,buf+1,15);
}

void sv_setchar_name3(unsigned char *buf)
{
	DEBUG("SV SETCHAR NAME3");
	memcpy(pl.name+30,buf+1,10);
	strcpy(okey.name,pl.name);
	okey.race=*(unsigned long *)(buf+11);
	save_options();
}

void sv_setchar_mode(unsigned char *buf)
{
	DEBUG("SV SETCHAR MODE");
	pl.mode=buf[1];
}

void sv_setchar_hp(unsigned char *buf)
{
	DEBUG("SV SETCHAR HP");
	pl.hp[0]=*(unsigned short *)(buf+1);
	pl.hp[1]=*(unsigned short *)(buf+3);
	pl.hp[2]=*(unsigned short *)(buf+5);
	pl.hp[3]=*(unsigned short *)(buf+7);
	pl.hp[4]=*(unsigned short *)(buf+9);
	pl.hp[5]=*(unsigned short *)(buf+11);
}

void sv_setchar_endur(unsigned char *buf)
{
	DEBUG("SV SETCHAR ENDUR");
	pl.end[0]=*(short int*)(buf+1);
	pl.end[1]=*(short int*)(buf+3);
	pl.end[2]=*(short int*)(buf+5);
	pl.end[3]=*(short int*)(buf+7);
	pl.end[4]=*(short int*)(buf+9);
	pl.end[5]=*(short int*)(buf+11);
}

void sv_setchar_mana(unsigned char *buf)
{
	DEBUG("SV SETCHAR MANA");
	pl.mana[0]=*(short int*)(buf+1);
	pl.mana[1]=*(short int*)(buf+3);
	pl.mana[2]=*(short int*)(buf+5);
	pl.mana[3]=*(short int*)(buf+7);
	pl.mana[4]=*(short int*)(buf+9);
	pl.mana[5]=*(short int*)(buf+11);
}

void sv_setchar_attrib(unsigned char *buf)
{
	int n;
	DEBUG("SV SETCHAR ATTRIB");

	n=buf[1];
	if (n<0 || n>4) return;

	pl.attrib[n][0]=buf[2];
	pl.attrib[n][1]=buf[3];
	pl.attrib[n][2]=buf[4];
	pl.attrib[n][3]=buf[5];
	pl.attrib[n][4]=buf[6];
	pl.attrib[n][5]=buf[7];
}

void sv_setchar_skill(unsigned char *buf)
{
	int n;
	extern int skill_cmp(const void *a,const void *b);

	DEBUG("SV SETCHAR SKILL");

	n=buf[1];
	if (n<0 || n>49) return;

	pl.skill[n][0]=buf[2];
	pl.skill[n][1]=buf[3];
	pl.skill[n][2]=buf[4];
	pl.skill[n][3]=buf[5];
	pl.skill[n][4]=buf[6];
	pl.skill[n][5]=buf[7];

	qsort(skilltab,55,sizeof(struct skilltab),skill_cmp);
}

void sv_setchar_ahp(unsigned char *buf)
{
	DEBUG("SV SETCHAR AHP");
	pl.a_hp=*(unsigned short*)(buf+1);
}

void sv_setchar_aend(unsigned char *buf)
{
	DEBUG("SV SETCHAR AEND");
	pl.a_end=*(unsigned short*)(buf+1);
}

void sv_setchar_amana(unsigned char *buf)
{
	DEBUG("SV SETCHAR AMAANA");
	pl.a_mana=*(unsigned short*)(buf+1);
}

void sv_setchar_dir(unsigned char *buf)
{
	DEBUG("SV SETCHAR DIR");
	pl.dir=*(unsigned char*)(buf+1);
}

extern int stat_raised[];
extern int stat_points_used;

void sv_setchar_pts(unsigned char *buf)
{
	int n;

	DEBUG("SV SETCHAR PTS");
	pl.points=*(unsigned long*)(buf+1);
	pl.points_tot=*(unsigned long*)(buf+5);

	if (pl.kindred != *(unsigned long*)(buf+9))
	{
		stat_points_used=0;
		for (n=0; n<108; n++)
		{
			stat_raised[n]=0;
		}
	}

	pl.kindred=*(unsigned long*)(buf+9);
}

void sv_setchar_wps(unsigned char *buf)
{
	DEBUG("SV SETCHAR WPS");
	pl.waypoints=*(unsigned long*)(buf+1);
	pl.bs_points=*(unsigned long*)(buf+5);
	pl.os_points=*(unsigned long*)(buf+9);
}

void sv_setchar_tok(unsigned char *buf)
{
	DEBUG("SV SETCHAR TOK");
	pl.tokens=*(unsigned long*)(buf+1);
	pl.tree_points=*(unsigned short*)(buf+5);
	pl.os_tree=*(unsigned short*)(buf+7);
}

void sv_setchar_tre(unsigned char *buf)
{
	int n;
	DEBUG("SV SETCHAR TRE");

	n = *(unsigned char*)(buf+1);
	if (n<0 || n>11) xlog(0,"Invalid setchar tre");

	pl.tree_node[n]=*(unsigned char*)(buf+2);
}

void sv_setchar_gold(unsigned char *buf)
{
	DEBUG("SV SETCHAR GOLD");
	pl.gold=*(unsigned long*)(buf+1);
	pl.armor=*(unsigned long*)(buf+5);
	pl.weapon=*(unsigned long*)(buf+9);
}

void sv_setchar_item(unsigned char *buf)
{
	int n;
	DEBUG("SV SETCHAR ITEM");

	n=*(unsigned long*)(buf+1);
	if (n<0 || n>(MAXITEMS-1)) xlog(0,"Invalid setchar item");

	pl.item[n]=*(short int*)(buf+5);
	pl.item_p[n]=*(short int*)(buf+7);
	pl.item_s[n]=*(unsigned char*)(buf+9); // stack size
	pl.item_l[n]=*(unsigned char*)(buf+10); // item lock

	pl.item_info[n] = (ItemDisplayInfo){pl.item[n], pl.item_p[n], pl.item_s[n], pl.item_l[n]};

//	xlog("SV SETCHAR ITEM (%d,%d,%d)",*(unsigned long*)(buf+1),*(short int*)(buf+5),*(short int*)(buf+7));
}

void sv_setchar_worn(unsigned char *buf)
{
	int n;
	DEBUG("SV SETCHAR WORN");

	n=*(unsigned long*)(buf+1);
	if (n<0 || n>19) xlog(0,"Invalid setchar worn");
	pl.worn[n]=*(short int*)(buf+5);
	pl.worn_p[n]=*(short int*)(buf+7);
	pl.worn_s[n]=*(unsigned char*)(buf+9);

	pl.worn_info[n] = (ItemDisplayInfo){pl.worn[n], pl.worn_p[n], pl.worn_s[n], 0};
}

void sv_setchar_spell(unsigned char *buf)
{
	int n;
	DEBUG("SV SETCHAR SPELL");

	n=*(unsigned long*)(buf+1);
	if (n<0 || n>(MAXBUFFS-1)) xlog(0,"Invalid setchar spell");
	pl.spell[n]=*(short int*)(buf+5);
	pl.active[n]=*(short int*)(buf+7);
}

void sv_setchar_location1(unsigned char *buf)
{
	DEBUG("SV SETCHAR LOCA1");
	memcpy(pl.location,buf+1,10);
}

void sv_setchar_location2(unsigned char *buf)
{
	DEBUG("SV SETCHAR LOCA2");
	memcpy(pl.location+10,buf+1,10);
}

void sv_setchar_obj(unsigned char *buf)
{
	DEBUG("SV SETCHAR OBJ");

	pl.citem=*(short int*)(buf+1);
	pl.citem_p=*(short int*)(buf+3);
	pl.citem_s=*(unsigned char*)(buf+5); // stack size

	if (pl.citem == 0) {
		cursor_emptied();
	}

//	xlog("SV SETCHAR OBJ (%d,%d)",*(short int*)(buf+1),*(short int*)(buf+3));
}

static int lastn=0;

int sv_setmap(unsigned char *buf,int off)
{
	int n,p;
	static int cnt[8]={0,0,0,0,0,0,0,0};

	DEBUG("SV SETMAP");

	if (off) {
		n=lastn+off;
		p=2;
	} else {
		n=*(unsigned short*)(buf+2);
		p=4;
	}

	if (n<0 || n>=screen_renderdist*screen_renderdist) { xlog(0,"corrupt setmap!"); return -1; }

	lastn=n;
	if (!buf[1]) { DEBUG("Warning: no flags in setmap!"); return -1; }

	if (buf[1]&1) {
		map[n].ba_sprite=*(unsigned short*)(buf+p); p+=2;
		cnt[0]++;
	}
	if (buf[1]&2) {
		map[n].flags=*(unsigned int*)(buf+p); p+=4;
		cnt[1]++;
	}
	if (buf[1]&4) {
		map[n].flags2=*(unsigned int*)(buf+p); p+=4;
		cnt[2]++;
	}
	if (buf[1]&8) {
		map[n].it_sprite=*(unsigned short*)(buf+p); p+=2;
		cnt[3]++;
	}
	if (buf[1]&16) {
		map[n].it_status=*(unsigned char*)(buf+p); p+=1;
		cnt[4]++;
	}
	if (buf[1]&32) {
		map[n].ch_sprite=*(unsigned short*)(buf+p); p+=2;
		map[n].ch_status=*(unsigned char*)(buf+p); p+=1;
		map[n].ch_stat_off=*(unsigned char*)(buf+p); p+=1;
		cnt[5]++;
	}
	if (buf[1]&64) {
		map[n].ch_nr=*(unsigned short*)(buf+p); p+=2;
		map[n].ch_id=*(unsigned short*)(buf+p); p+=2;
		map[n].ch_speed=*(short int*)(buf+p); p+=2;
		cnt[6]++;
	}
	if (buf[1]&128) {
		map[n].ch_proz=*(unsigned char*)(buf+p); p+=1;
		// Additional data received - cast speed, attack speed, move speed,
		//   and special font colorization of a given npc
		map[n].ch_castspd=*(short int*)(buf+p); p+=2;
		map[n].ch_atkspd=*(short int*)(buf+p); p+=2;
		map[n].ch_movespd=*(short int*)(buf+p); p+=2;
		map[n].ch_fontcolor=*(unsigned char*)(buf+p); p+=1;
		cnt[7]++;
	}
	return p;
}

int sv_setmap3(unsigned char *buf,int cnt)
{
	int n,m,p;
	unsigned char tmp;

	//printf("cnt=%d, ",cnt);
	DEBUG("SV SETMAP3");

	// Old system
	//n=(*(unsigned short*)(buf+1))&4095;
	//tmp=(*(unsigned short*)(buf+1))>>12;

	n=*(unsigned int*)(buf+1);
	tmp=*(unsigned char*)(buf+5);
	p=6;
	if (n<0 || n>=screen_renderdist*screen_renderdist) { xlog(0,"corrupt setmap3!"); return -1; }

	map[n].light=tmp;

	if (cnt>0) {
		for (m=n+2; m<n+cnt+2; m+=2,p++) {
			if (m<screen_renderdist*screen_renderdist) {
				tmp=*(unsigned char*)(buf+p);

				map[m].light=(unsigned char)(tmp&15);
				map[m-1].light=(unsigned char)(tmp>>4);
			}
		}
	}

	return p;
}

void sv_setorigin(unsigned char *buf)
{
	int x,y,xp,yp,n;

	DEBUG("SV SETORIGIN");

	xp=*(short*)(buf+1);
	yp=*(short*)(buf+3);

	for (y=n=0; y<screen_renderdist; y++) {
		for (x=0; x<screen_renderdist; x++,n++) {
			map[n].x=(unsigned short)(x+xp);
			map[n].y=(unsigned short)(y+yp);
		}
	}
}

void sv_tick(unsigned char *buf)
{
	DEBUG2("SV TICK");
	ctick=*(unsigned char*)(buf+1);
}

void sv_log(unsigned char *buf,int font)
{
	static char text[512];
	static int cnt=0;
	int n;

	DEBUG("SV LOG");

	memcpy(text+cnt,buf+1,15);

	for (n=cnt; n<cnt+15; n++)
		if (text[n]==10) {
			text[n]=0;
			tlog(text,font);
			cnt=0;
			return;
		}
	cnt+=15;

	if (cnt>500) {
		xlog(0,"sv_log(): cnt too big!");

		text[cnt]=0;
		xlog(1,text);
		cnt=0;
	}
}

void sv_motd(unsigned char *buf,int font)
{
	static char text[512];
	static int cnt=0;
	int n;

	DEBUG("SV MOTD");

	memcpy(text+cnt,buf+1,15);

	for (n=cnt; n<cnt+15; n++)
		if (text[n]==10) {
			text[n]=0;
			motdlog(text,font);
			cnt=0;
			return;
		}
	cnt+=15;

	if (cnt>500) {
		xlog(0,"sv_motd(): cnt too big!");

		text[cnt]=0;
		mxlog(1,text);
		cnt=0;
	}
}

#pragma argsused
void sv_scroll_right(unsigned char *buf)
{
	DEBUG("SV SCROLL_RIGHT");

	memmove(map,map+1,sizeof(struct cmap)*(screen_renderdist*screen_renderdist-1));
}

#pragma argsused
void sv_scroll_left(unsigned char *buf)
{
	DEBUG("SV SCROLL_LEFT");

	memmove(map+1,map,sizeof(struct cmap)*(screen_renderdist*screen_renderdist-1));
}

#pragma argsused
void sv_scroll_down(unsigned char *buf)
{
	DEBUG("SV SCROLL_DOWN");

	memmove(map,map+screen_renderdist,sizeof(struct cmap)*(screen_renderdist*screen_renderdist-screen_renderdist));
}

#pragma argsused
void sv_scroll_up(unsigned char *buf)
{
	DEBUG("SV SCROLL_UP");

	memmove(map+screen_renderdist,map,sizeof(struct cmap)*(screen_renderdist*screen_renderdist-screen_renderdist));
}

#pragma argsused
void sv_scroll_leftup(unsigned char *buf)
{
	DEBUG("SV SCROLL_LEFTUP");

	memmove(map+screen_renderdist+1,map,sizeof(struct cmap)*(screen_renderdist*screen_renderdist-screen_renderdist-1));
}

#pragma argsused
void sv_scroll_leftdown(unsigned char *buf)
{
	DEBUG("SV SCROLL_LEFTDOWN");

	memmove(map,map+screen_renderdist-1,sizeof(struct cmap)*(screen_renderdist*screen_renderdist-screen_renderdist+1));
}

#pragma argsused
void sv_scroll_rightup(unsigned char *buf)
{
	DEBUG("SV SCROLL_RIGHTUP");

	memmove(map+screen_renderdist-1,map,sizeof(struct cmap)*(screen_renderdist*screen_renderdist-screen_renderdist+1));
}

#pragma argsused
void sv_scroll_rightdown(unsigned char *buf)
{
	DEBUG("SV SCROLL_RIGHTDOWN");

	memmove(map,map+screen_renderdist+1,sizeof(struct cmap)*(screen_renderdist*screen_renderdist-screen_renderdist-1));
}

void sv_look1(unsigned char *buf)
{
	DEBUG("SV LOOK1");

	tmplook.worn[0]=*(unsigned short*)(buf+1);
	tmplook.worn[2]=*(unsigned short*)(buf+3);
	tmplook.worn[3]=*(unsigned short*)(buf+5);
	tmplook.worn[5]=*(unsigned short*)(buf+7);
	tmplook.worn[6]=*(unsigned short*)(buf+9);
	tmplook.worn[7]=*(unsigned short*)(buf+11);
	tmplook.worn[8]=*(unsigned short*)(buf+13);
	tmplook.autoflag=*(unsigned char*)(buf+15);
}

void sv_look2(unsigned char *buf)
{
	DEBUG("SV LOOK2");

	tmplook.worn[9]=*(unsigned short*)(buf+1); // 1 2
	tmplook.sprite=*(unsigned short*)(buf+3); // 3 4
	tmplook.points=*(unsigned int*)(buf+5); // 5 6 7 8
	tmplook.hp=*(unsigned int*)(buf+9); //9 10 11 12
	tmplook.worn[10]=*(unsigned short*)(buf+13); // 13 14
}

void sv_look3(unsigned char *buf)
{
	DEBUG("SV LOOK3");

	tmplook.end=*(unsigned short*)(buf+1);
	tmplook.a_hp=*(unsigned short*)(buf+3);
	tmplook.a_end=*(unsigned short*)(buf+5);
	tmplook.nr=*(unsigned short*)(buf+7);
	tmplook.id=*(unsigned short*)(buf+9);
	tmplook.mana=*(unsigned short*)(buf+11);
	tmplook.a_mana=*(unsigned short*)(buf+13);
}

void sv_look4(unsigned char *buf)
{
	DEBUG("SV LOOK4");

	tmplook.worn[1]=*(unsigned short*)(buf+1);
	tmplook.worn[4]=*(unsigned short*)(buf+3);
	tmplook.extended=buf[5];
	tmplook.pl_price=*(unsigned int*)(buf+6);
	tmplook.worn[11]=*(unsigned short*)(buf+10);
	tmplook.worn[12]=*(unsigned short*)(buf+12);
	tmplook.worn[13]=*(unsigned short*)(buf+14);
}

void sv_look5(unsigned char *buf)
{
	int n;

	DEBUG("SV LOOK5");

	for (n=0; n<15; n++) tmplook.name[n]=buf[n+1];
	tmplook.name[15]=0;

	if (!(tmplook.extended & 1)) {
		if (!tmplook.autoflag) {
			show_look=1;
			look=tmplook;
			look_timer=10*TICKS/TICKMULTI;
		}
		add_look(tmplook.nr,tmplook.name,tmplook.id);
	}
}

void sv_look6(unsigned char *buf)
{
	int n,s;

	DEBUG("SV LOOK6");

	s=buf[1];

	for (n=s; n<min(62,s+2); n++) {
		tmplook.item[n]   =*(unsigned short*)(buf+2+(n-s)*6);
		tmplook.price[n]  =*(unsigned int  *)(buf+4+(n-s)*6);
		tmplook.item_p[n] =*(unsigned char *)(buf+14+(n-s));
		tmplook.item_info[n] = (ItemDisplayInfo){tmplook.item[n], tmplook.price[n], 0, tmplook.item_p[n]};
	}
	if (n==62) {
		game_ui_state.open_shop=1+*(unsigned char*)(buf+14); // gold slot bit
		shop=tmplook;
	}
	if (game_ui_state.open_shop)
	{
		game_ui_state.show_waypoints = false;
		game_ui_state.open_book=0;
		game_ui_state.show_motd = false;
		game_ui_state.show_new_player = false;
		game_ui_state.tutorial.open=0;
		game_ui_state.open_skill_tree = 0;
	}
}

void sv_look7(unsigned char *buf)
{
	int n,s;

	DEBUG("SV LOOK7");

	n=buf[1];
	s=buf[2];

	  tmplook.depot[n][s] =*(unsigned short*)(buf+3);
	tmplook.depot_s[n][s] =*(unsigned char *)(buf+5);
	tmplook.depot_f[n][s] =*(unsigned char *)(buf+6);
	tmplook.depot_c[n][s] =*(unsigned char *)(buf+7);

	tmplook.depot_info[n][s] = (ItemDisplayInfo){
		tmplook.depot[n][s], tmplook.depot_c[n][s], tmplook.depot_s[n][s], tmplook.depot_f[n][s]
	};

	if (n==7 && s==63)
	{
		game_ui_state.open_shop=112;
		shop=tmplook;
	}
	if (game_ui_state.open_shop)
	{
		game_ui_state.show_waypoints = false;
		game_ui_state.open_book=0;
		game_ui_state.show_motd = false;
		game_ui_state.show_new_player = false;
		game_ui_state.tutorial.open=0;
		game_ui_state.open_skill_tree = 0;
	}
}

void sv_look8(unsigned char *buf)	// Blacksmith
{
	int n;

	DEBUG("SV LOOK8");

	n =*(unsigned char*)(buf+1);
	shop.nr=*(unsigned short*)(buf+2);

	if (n>5||n<0)	return;
	else if (n==5)	game_ui_state.open_shop=111;	// Is armour
	else if (n==4)	game_ui_state.open_shop=110;	// Is weapon
	else
	{
		  pl.sitem[n]=*(unsigned short*)(buf+4); // Item sprite
		pl.sitem_s[n]=*(unsigned char *)(buf+6); // Item stack
		pl.sitem_f[n]=*(unsigned char *)(buf+7); // Item flags

		  pl.smith_info[n] = (ItemDisplayInfo){pl.sitem[n], 0, pl.sitem_s[n], pl.sitem_f[n]};
	}

	if (game_ui_state.open_shop)
	{
		game_ui_state.show_waypoints = false;
		game_ui_state.open_book=0;
		game_ui_state.show_motd = false;
		game_ui_state.show_new_player = false;
		game_ui_state.tutorial.open=0;
		game_ui_state.open_skill_tree = 0;
	}
}

extern int noshop;

void sv_closeshop()
{
	DEBUG("SV CLOSESHOP");
	game_ui_state.open_shop=0; noshop=QSIZE*3;
}

void sv_showmotd(unsigned char *buf)
{
	int n;

	DEBUG("SV SHOWMOTD");

	n =*(unsigned char *)(buf+1);

	if (n>=100)
	{	// Display tutorial
		game_ui_state.tutorial.page=1;
		game_ui_state.tutorial.open=(n%100);
		game_ui_state.tutorial.count=3;
	}
	else if (n==99)
	{	// Display NEW PLAYER MotD from server and offer tutorial
		game_ui_state.show_new_player = true;
	}
	else if (n)
	{
		game_ui_state.open_book=n;
		game_ui_state.tutorial.page=1;
		game_ui_state.tutorial.count=3;
	}
	else
	{	// Display normal MotD from server
		game_ui_state.show_motd = true;
	}
}

void sv_waypoints()
{
	DEBUG("SV WAYPOINTS");

	game_ui_state.show_waypoints = true;
}

extern unsigned short ymap[MAPX_MAX*MAPY_MAX];
extern unsigned short xmap[MAPX_MAX*MAPY_MAX];

void sv_clearbox(unsigned char *buf)
{
	int xx, yy, x, y, w, h;

	DEBUG("SV CLEARBOX");

	x =*(unsigned short*)(buf+1);
	y =*(unsigned short*)(buf+3);
	w =*(unsigned short*)(buf+5);
	h =*(unsigned short*)(buf+7);

	for (xx=x; xx<x+w; xx++)
	{
		for (yy=y; yy<y+h; yy++)
		{
			xmap[yy+xx*MAPX_MAX]=0;
			ymap[yy+xx*MAPX_MAX]=1;
		}
	}
}

void sv_settarget(unsigned char *buf)
{
	DEBUG("SV SETTARGET");
	bool was_using = false;
	if (pl.misc_action == 4) was_using = true;

	pl.attack_cn    =*(unsigned short*)(buf+1);
	pl.goto_x       =*(unsigned short*)(buf+3);
	pl.goto_y       =*(unsigned short*)(buf+5);
	pl.misc_action  =*(unsigned short*)(buf+7);
	pl.misc_target1 =*(unsigned short*)(buf+9);
	pl.misc_target2 =*(unsigned short*)(buf+11);

	if (was_using && pl.misc_action != 4) {
		if (!mod_stubborn_actions_pending_use()) {
			pop_cmd_from_queue();
		}
	} else if (!was_using) {
		if (pl.misc_action == DR_USE) {
			cmd_queue_on_target(pl.misc_target1, pl.misc_target2);
		}
	}
}

void sv_playsound(unsigned char *buf)
{
	int nr,vol,pan;
	char name[80];

	DEBUG("SV PLAYSOUND");

	nr=*(unsigned int*)(buf+1);
	vol=*(int*)(buf+5);
	pan=*(int*)(buf+9);

//  xlog(1,"sample=%d, pan=%d, vol=%d",nr,pan,vol);

	sprintf(name,"sfx/%d.wav",nr);
	play_sound(name,vol,-pan);		// add flag to reverse channels!!
}

void sv_exit(unsigned char *buf)
{
	int reason;

	DEBUG("SV EXIT");

	reason=*(unsigned int*)(buf+1);

	xlog(1," ");
	if (reason<1 || reason>12) xlog(1,"EXIT: Reason unknown.");
	else xlog(1,"EXIT: %s",logout_reason[reason]);

	do_exit=1;
}

void sv_load(unsigned char *buf)
{
	extern int load;

	DEBUG("SV LOAD");

	load=*(unsigned int*)(buf+1);
}

void sv_unique(unsigned char *buf)
{
	DEBUG("SV UNIQUE");

	unique1=*(unsigned int*)(buf+1);
	unique2=*(unsigned int*)(buf+5);
	security_id_save();
}

int sv_ignore(unsigned char *buf)
{
	int size,d;
	static int cnt=0,got=0,start=0;

	size=*(unsigned int*)(buf+1);
	got+=size;

	if (!start) start=time(NULL);

	if (cnt++>16) {
		cnt=0;
		d=time(NULL)-start;
		if (d==0) d=1;

                xlog(3,"ignore=%d, got=%d, tps=%.2fK/s",size,got,(double)got/d/1024.0);
	}

	return size;
}

int sv_cmd(unsigned char *buf)
{

	if (buf[0]&SV_SETMAP) return sv_setmap(buf,buf[0]&~SV_SETMAP);

	switch(buf[0]) {
		case	SV_SETCHAR_NAME1:	sv_setchar_name1(buf); break;
		case	SV_SETCHAR_NAME2:	sv_setchar_name2(buf); break;
		case	SV_SETCHAR_NAME3:	sv_setchar_name3(buf); break;
		case	SV_SETCHAR_MODE:	sv_setchar_mode(buf); return 2;
		case	SV_SETCHAR_ATTRIB:	sv_setchar_attrib(buf); return 8;
		case	SV_SETCHAR_SKILL:	sv_setchar_skill(buf); return 8;
		case	SV_SETCHAR_HP:		sv_setchar_hp(buf); return 13;
		case	SV_SETCHAR_ENDUR:	sv_setchar_endur(buf); return 13;
		case	SV_SETCHAR_MANA:	sv_setchar_mana(buf); return 13;
		case	SV_SETCHAR_AHP:		sv_setchar_ahp(buf); return 3;
		case	SV_SETCHAR_AEND:    sv_setchar_aend(buf); return 3;
		case	SV_SETCHAR_AMANA:	sv_setchar_amana(buf); return 3;
		case	SV_SETCHAR_DIR:		sv_setchar_dir(buf); return 2;

		case	SV_SETCHAR_PTS:		sv_setchar_pts(buf); return 13;
		case	SV_SETCHAR_WPS:		sv_setchar_wps(buf); return 13;
		case	SV_SETCHAR_TOK:		sv_setchar_tok(buf); return 9;
		case	SV_SETCHAR_TRE:		sv_setchar_tre(buf); return 3;
		case	SV_SETCHAR_GOLD:	sv_setchar_gold(buf); return 13;
		case	SV_SETCHAR_ITEM:	sv_setchar_item(buf); return 11;
		case	SV_SETCHAR_WORN:	sv_setchar_worn(buf); return 10;
		case	SV_SETCHAR_SPELL:	sv_setchar_spell(buf); return 9;
		case	SV_SETCHAR_OBJ:		sv_setchar_obj(buf); return 6;
		case	SV_SETCHAR_LOCA1:	sv_setchar_location1(buf); return 11;
		case	SV_SETCHAR_LOCA2:	sv_setchar_location2(buf); return 11;

		case	SV_SETMAP3:		return sv_setmap3(buf,20);
		case	SV_SETMAP4:		return sv_setmap3(buf,0);
		case	SV_SETMAP5:		return sv_setmap3(buf,2);
		case	SV_SETMAP6:		return sv_setmap3(buf,6);
		case	SV_SETORIGIN:		sv_setorigin(buf); return 5;

		case	SV_TICK:		sv_tick(buf); return 2;

		case	SV_LOG0:		sv_log(buf,0); break;
		case	SV_LOG1:		sv_log(buf,1); break;
		case	SV_LOG2:		sv_log(buf,2); break;
		case	SV_LOG3:		sv_log(buf,3); break;
		case	SV_LOG4:		sv_log(buf,4); break;
		case	SV_LOG5:		sv_log(buf,5); break;
		case	SV_LOG6:		sv_log(buf,6); break;
		case	SV_LOG7:		sv_log(buf,7); break;
		case	SV_LOG8:		sv_log(buf,8); break;
		case	SV_LOG9:		sv_log(buf,9); break;

		case SV_TERM_STREE:
		case SV_TERM_CTREE:
		case SV_TERM_SKILLS:
		case SV_TERM_META:
			return sv_terminology(buf);

		case	SV_MOTD0:		sv_motd(buf,0); break;
		case	SV_MOTD1:		sv_motd(buf,1); break;
		case	SV_MOTD2:		sv_motd(buf,2); break;
		case	SV_MOTD3:		sv_motd(buf,3); break;

		case	SV_SCROLL_RIGHT:	sv_scroll_right(buf); return 1;
		case	SV_SCROLL_LEFT:		sv_scroll_left(buf); return 1;
		case	SV_SCROLL_DOWN:		sv_scroll_down(buf); return 1;
		case	SV_SCROLL_UP:		sv_scroll_up(buf); return 1;

		case	SV_SCROLL_RIGHTDOWN:	sv_scroll_rightdown(buf); return 1;
		case	SV_SCROLL_RIGHTUP:		sv_scroll_rightup(buf); return 1;
		case	SV_SCROLL_LEFTDOWN:		sv_scroll_leftdown(buf); return 1;
		case	SV_SCROLL_LEFTUP:		sv_scroll_leftup(buf); return 1;

		case	SV_LOOK1:				sv_look1(buf); break;
		case	SV_LOOK2:				sv_look2(buf); break;
		case	SV_LOOK3:				sv_look3(buf); break;
		case	SV_LOOK4:				sv_look4(buf); break;
		case	SV_LOOK5:				sv_look5(buf); break;
		case	SV_LOOK6:				sv_look6(buf); break;
		case	SV_LOOK7:				sv_look7(buf); return 8;
		case	SV_LOOK8:				sv_look8(buf); return 8;

		case	SV_CLOSESHOP:			sv_closeshop(); return 1;

		case	SV_SETTARGET:			sv_settarget(buf); return 13;

		case	SV_PLAYSOUND:			sv_playsound(buf); return 13;

		case	SV_EXIT:				sv_exit(buf); break;

		case  	SV_LOAD:             	sv_load(buf); return 5;

		case  	SV_UNIQUE:             	sv_unique(buf); return 9;
		case 	SV_IGNORE:		return sv_ignore(buf);

		case	SV_WAYPOINTS:			sv_waypoints(); return 1;
		case	SV_SHOWMOTD:			sv_showmotd(buf); return 2;

		case	SV_CLEARBOX:			sv_clearbox(buf); return 9;

		default: 			xlog(0,"Unknown SV: %d",buf[0]); return -1;
	}

	return 16;
}

#pragma argused
void so_perf_report()
{
	unsigned char buf[16];

	buf[0]=CL_PERF_REPORT;
	*(unsigned short*)(buf+1)=(unsigned short)0;
	*(unsigned short*)(buf+3)=(unsigned short)0;
	*(unsigned short*)(buf+5)=(unsigned short)0;
	*(float*)(buf+7)=0;
	xsend(buf);
}

void xsend(unsigned char *buf) {
	int len = 0, ret;

	while (len < 16) {
		ret = SDLNet_TCP_Send(sock, buf + len, 16 - len);
		if (ret < 0) {
			so_error(SDLNet_GetError());
		}
		len += ret;
	}
}

#define TSIZE	(8192*16)

unsigned char tickbuf[TSIZE];
int ticksize=0;		// amount of data in tickbuf
int tickstart=0;	// start index to scan buffer for next tick

int game_loop(void) {
	int ret, tmp;


	while (1) {
		if (SDLNet_CheckSockets(socket_set, 0) > 0 && SDLNet_SocketReady(sock)) {
			ret = SDLNet_TCP_Recv(sock, tickbuf+ticksize, TSIZE-ticksize);
			if (ret <= 0) {
				so_error("receive error");
			}
		} else {
			ret = 0;  // No data available
		}

		ticksize += ret;

		if (ticksize >= tickstart + 2) {
			tmp = *(unsigned short *) (tickbuf + tickstart);
			tmp &= 0x7fff;
			if (tmp < 2) so_error("transmission corrupt");
			tickstart += tmp;
			t_size++;
		} else break;
		handle_input();
	}

	return 0; // no more work
}

int tick_do(void)
{
	int len,idx=0,ret,csize,comp;
	static char buf[65536];
	static int ctot=1,utot=1,t=0,td;

	if (!t) t=time(NULL);

        len=*(unsigned short*)(tickbuf);
	comp=len&0x8000;
	len&=0x7fff;
	ctot+=len;
        if (len>ticksize) return 0;

        if (comp) {
		zs.next_in=tickbuf+2;
		zs.avail_in=len-2;

		zs.next_out=buf;
		zs.avail_out=65536;

                ret=inflate(&zs,Z_SYNC_FLUSH);
		if (ret!=Z_OK) { xlog(0,"uncompress error %d!",ret); }

		if (zs.avail_in) { xlog(0,"uncompress: avail is %d!!\n",zs.avail_in); }

                csize=65536-zs.avail_out;
	} else {
		csize=len-2;
		if (csize) memcpy(buf,tickbuf+2,csize);
	}

	utot+=csize;

	td=time(NULL)-t;
	if (!td) td=1;

	lastn=-1;	// reset sv_setmap
	ctick++; if (ctick>199) ctick=0;		// Feb 2020 - extended ctick array from 20 to 24 to 200

        while (idx<csize) {
		ret=sv_cmd(buf+idx);
		if (ret==-1) { xlog(1,"Warning: syntax error in server data"); log_critical("Warning: syntax error in server data"); exit(1); }
		idx+=ret;
	}

	ticksize-=len;
	tickstart-=len;
	t_size--;
        if (ticksize) memmove(tickbuf,tickbuf+len,ticksize);

        engine_tick();

	return 1;
}

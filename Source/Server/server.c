/*************************************************************************

   This file is part of 'Mercenaries of Astonia v2'
   Copyright (c) 1997-2001 Daniel Brockhaus (joker@astonia.com)
   All rights reserved.

 **************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define __USE_BSD_SIGNAL
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdarg.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <sys/wait.h>
#include <errno.h>

#define TBUFSIZE (4096*16)
#define OBUFSIZE (TBUFSIZE)

#include "server.h"

volatile int quit = 0;

struct player player[MAXPLAYER];
struct see_map *see;

FILE *logfp;

FILE *discordWho;
FILE *discordShoutIn;
FILE *discordShoutOut;
FILE *discordRanked;
FILE *discordTopA;
FILE *discordTopB;
FILE *discordPandium;

char  mod[256];

#define DISC_R    8
#define DISC_T    5
#define DISC_P   15

void discord_top_five(void)
{
	int j, n, m, font;
	int r1, r2, r3, r4;
	int b[DISC_R][DISC_T], nr[DISC_R][DISC_T];
	
	for (j = 0; j<DISC_R; j++) for (m = 0; m<DISC_T; m++)
	{
		b[j][m]  = -1;
		nr[j][m] = -1;
	}
	for (j = 0; j<DISC_R; j++) for (n = 1; n<MAXCHARS; n++)
	{
		if (ch[n].used==USE_EMPTY)				continue;
		if (!(ch[n].flags & (CF_PLAYER)))		continue;
		if (ch[n].flags & (CF_GOD | CF_NOLIST))	continue;
		
		if (j==0 && !IS_SEYAN_DU(n))	continue;
		if (j==1 && !IS_ARCHTEMPLAR(n))	continue;
		if (j==2 && !IS_SKALD(n))		continue;
		if (j==3 && !IS_WARRIOR(n))		continue;
		if (j==4 && !IS_SORCERER(n))	continue;
		if (j==5 && !IS_SUMMONER(n))	continue;
		if (j==6 && !IS_ARCHHARAKIM(n))	continue;
		if (j==7 && !IS_BRAVER(n))		continue;
		
		for (m = 0; m<DISC_T; m++)
		{
			if (ch[n].points_tot>b[j][m])
			{
				if (m<(DISC_T-1))
				{
					memmove(&b[j][m + 1], &b[j][m], ((DISC_T-1) - m) * sizeof(int));
					memmove(&nr[j][m + 1], &nr[j][m], ((DISC_T-1) - m) * sizeof(int));
				}
				b[j][m]  = ch[n].points_tot;
				nr[j][m] = n;
				break;
			}
		}
	}
	
	discordTopA = fopen("topa.txt", "w");
	fprintf(discordTopA, "```diff\n");
	for (j = 0; j<DISC_R/2; j++) 
	{
		if (j!=0) fprintf(discordTopA, " \n");
		switch (j)
		{
			case  0: fprintf(discordTopA, "- Top Seyan'du:\n"); break;
			case  1: fprintf(discordTopA, "- Top Arch-Templar:\n"); break;
			case  2: fprintf(discordTopA, "- Top Skalds:\n"); break;
			default: fprintf(discordTopA, "- Top Warriors:\n"); break;
		}
		for (m = 0; m<DISC_T; m++)
		{
			if (nr[j][m]==-1) continue;
			font = (IS_STAFF(nr[j][m]) || IS_GOD(nr[j][m])) ? 1 : 0;
			
			r1 = ch[nr[j][m]].points_tot / 1000000000;
			r2 = ch[nr[j][m]].points_tot / 1000000 % 1000;
			r3 = ch[nr[j][m]].points_tot / 1000 % 1000;
			r4 = ch[nr[j][m]].points_tot % 1000;
			
			if (r1)
				fprintf(discordTopA, "%c %10.10s   %20.20s %3d,%03d,%03d,%03d\n", font ? '+' : ' ', 
					ch[nr[j][m]].name, rank_name[getrank(nr[j][m])], 
					r1, r2, r3, r4);
			else if (r2)
				fprintf(discordTopA, "%c %10.10s   %20.20s     %3d,%03d,%03d\n", font ? '+' : ' ', 
					ch[nr[j][m]].name, rank_name[getrank(nr[j][m])], 
					r2, r3, r4);
			else if (r3)
				fprintf(discordTopA, "%c %10.10s   %20.20s         %3d,%03d\n", font ? '+' : ' ', 
					ch[nr[j][m]].name, rank_name[getrank(nr[j][m])], 
					r3, r4);
			else
				fprintf(discordTopA, "%c %10.10s   %20.20s             %3d\n", font ? '+' : ' ', 
					ch[nr[j][m]].name, rank_name[getrank(nr[j][m])], 
					r4);
		}
	}
	fprintf(discordTopA, "```\n");
	fflush(discordTopA);
	fclose(discordTopA);
	
	discordTopB = fopen("topb.txt", "w");
	fprintf(discordTopB, "```diff\n");
	for (j = DISC_R/2; j<DISC_R; j++) 
	{
		if (j!=DISC_R/2) fprintf(discordTopB, " \n");
		switch (j)
		{
			case  4: fprintf(discordTopB, "- Top Sorcerers:\n"); break;
			case  5: fprintf(discordTopB, "- Top Summoners:\n"); break;
			case  6: fprintf(discordTopB, "- Top Arch-Harakim:\n"); break;
			default: fprintf(discordTopB, "- Top Bravers:\n"); break;
		}
		for (m = 0; m<DISC_T; m++)
		{
			if (nr[j][m]==-1) continue;
			font = (IS_STAFF(nr[j][m]) || IS_GOD(nr[j][m])) ? 1 : 0;
			
			r1 = ch[nr[j][m]].points_tot / 1000000000;
			r2 = ch[nr[j][m]].points_tot / 1000000 % 1000;
			r3 = ch[nr[j][m]].points_tot / 1000 % 1000;
			r4 = ch[nr[j][m]].points_tot % 1000;
			
			if (r1)
				fprintf(discordTopB, "%c %10.10s   %20.20s %3d,%03d,%03d,%03d\n", font ? '+' : ' ', 
					ch[nr[j][m]].name, rank_name[getrank(nr[j][m])], 
					r1, r2, r3, r4);
			else if (r2)
				fprintf(discordTopB, "%c %10.10s   %20.20s     %3d,%03d,%03d\n", font ? '+' : ' ', 
					ch[nr[j][m]].name, rank_name[getrank(nr[j][m])], 
					r2, r3, r4);
			else if (r3)
				fprintf(discordTopB, "%c %10.10s   %20.20s         %3d,%03d\n", font ? '+' : ' ', 
					ch[nr[j][m]].name, rank_name[getrank(nr[j][m])], 
					r3, r4);
			else
				fprintf(discordTopB, "%c %10.10s   %20.20s             %3d\n", font ? '+' : ' ', 
					ch[nr[j][m]].name, rank_name[getrank(nr[j][m])], 
					r4);
		}
	}
	fprintf(discordTopB, "```\n");
	fflush(discordTopB);
	fclose(discordTopB);
}

void discord_gmoon(void)
{
	int hour, minute, day, month, year;

	hour = globs->mdtime / (MD_MIN * 60);
	minute = (globs->mdtime / MD_MIN) % 60;
	day = globs->mdday % 28 + 1;
	month = globs->mdday / 28 + 1;
	year  = globs->mdyear;
	
	fprintf(discordWho, "```diff\n");
	fprintf(discordWho, "It's %d:%02d on the %d%s%s%s%s%s%s%s of the %d%s%s%s%s month of the year %d.\n",
	            hour, minute,
	            day,
	            day==1  ? "st" : "",
	            day==2  ? "nd" : "",
	            day==3  ? "rd" : "",
	            day==21 ? "st" : "",
	            day==22 ? "nd" : "",
	            day==23 ? "rd" : "",
	            (day>3 && day<21) || day>23 ? "th" : "",
	            month,
	            month==1 ? "st" : "",
	            month==2 ? "nd" : "",
	            month==3 ? "rd" : "",
	            month>3  ? "th" : "",
	            year);
	if 		(IS_GLOB_MAYHEM)				fprintf(discordWho, "- Holiday Mayhem in effect! (Monsters bolster in darkness)\n");
	else if ((globs->mdday % 28) + 1==1)  	fprintf(discordWho, "+ New Moon in effect! (+0.10 Spellmod & 100%% faster regen)\n");
	else if ((globs->mdday % 28) + 1<15)  	fprintf(discordWho, "  The Moon is growing...\n");
	else if ((globs->mdday % 28) + 1==15) 	fprintf(discordWho, "+ Full Moon in effect! (+0.15 Spellmod & 50%% faster regen)\n");
	else 								  	fprintf(discordWho, "  The moon is dwindling...\n");
	fprintf(discordWho, "```\n");
}

void discord_pandium(void)
{
	// pandium.txt
	int j, n, m, font;
	int cl;
	int nr[2][DISC_P];	// Char # solo/group
	int fl[2][DISC_P];	// Floor # solo/group    ch[n].pandium_floor[0]    ch[n].pandium_floor[1]
	
	for (j = 0; j<2; j++) for (m = 0; m<DISC_P; m++)
	{
		fl[j][m]  = -1;
		nr[j][m] = -1;
	}
	for (j = 0; j<2; j++) for (n = 1; n<MAXCHARS; n++)
	{
		if (ch[n].used==USE_EMPTY)				continue;
		if (!(ch[n].flags & (CF_PLAYER)))		continue;
		if (ch[n].flags & (CF_GOD | CF_NOLIST))	continue;
		
		if (ch[n].pandium_floor[j]<=1)	continue;
		
		for (m = 0; m<DISC_P; m++)
		{
			if (ch[n].pandium_floor[j]>fl[j][m])
			{
				if (m<(DISC_P-1))
				{
					memmove(&fl[j][m + 1], &fl[j][m], ((DISC_P-1) - m) * sizeof(int));
					memmove(&nr[j][m + 1], &nr[j][m], ((DISC_P-1) - m) * sizeof(int));
				}
				fl[j][m] = ch[n].pandium_floor[j]-1;
				nr[j][m] = n;
				break;
			}
		}
	}
	
	for (m = 0; m<3; m++)
	{
		if (IS_SANEPLAYER(nr[0][m])) globs->top_ps[m] = nr[0][m];
		if (IS_SANEPLAYER(nr[1][m])) globs->top_pg[m] = nr[1][m];
	}
	
	discordPandium = fopen("pandium.txt", "w");
	fprintf(discordPandium, "```diff\n");
	for (j = 0; j<2; j++)
	{
		if (j!=0) fprintf(discordPandium, " \n");
		switch (j)
		{
			case  0: fprintf(discordPandium, "- Top Solo Archon Clears:\n"); break;
			default: fprintf(discordPandium, "- Top Group Archon Clears:\n"); break;
		}
		for (m = 0; m<(j?DISC_P:DISC_T); m++)
		{
			if (nr[j][m]==-1) continue;
			font = (IS_STAFF(nr[j][m]) || IS_GOD(nr[j][m])) ? 1 : 0;
			
			cl = -1;
			if (IS_SEYAN_DU(nr[j][m]))    cl = 0;
			if (IS_ARCHTEMPLAR(nr[j][m])) cl = 1;
			if (IS_SKALD(nr[j][m]))       cl = 2;
			if (IS_WARRIOR(nr[j][m]))     cl = 3;
			if (IS_SORCERER(nr[j][m]))    cl = 4;
			if (IS_SUMMONER(nr[j][m]))    cl = 5;
			if (IS_ARCHHARAKIM(nr[j][m])) cl = 6;
			if (IS_BRAVER(nr[j][m]))      cl = 7;

			fprintf(discordPandium, "%c %10.10s   %20.20s             %3d\n", font ? '+' : ' ', 
				ch[nr[j][m]].name, (cl == -1) ? "Archless" : class_name[cl], fl[j][m]);
		}
	}
	fprintf(discordPandium, "```\n");
	fflush(discordPandium);
	fclose(discordPandium);
}

void discord_who(void)
{
	int n, players, gc, font, showarea;
	
	// open Discord files
	discordWho = fopen("who.txt", "w");
	
	fprintf(discordWho, "```diff\n");
	fprintf(discordWho, "Players Online:\n");
	fprintf(discordWho, "-------------------------------------------------\n");
	players = 0;
	for (n = 1; n<MAXCHARS; n++)
	{
		if (!(ch[n].flags & (CF_PLAYER)))
		{
			continue;
		}
		if (ch[n].used!=USE_ACTIVE || (ch[n].flags & (CF_INVISIBLE | CF_NOWHO)))
		{
			continue;
		}
		players++;
		/* color staff and gods green */
		font = (IS_STAFF(n) || IS_GOD(n)) ? 1 : 0;

		showarea = 1;
		if (ch[n].flags & CF_GOD)
		{
			showarea = 0;
		}
		if (IS_CLANKWAI(n) || IS_CLANGORN(n))
		{
			showarea = 0;
		}

		fprintf(discordWho, "%c %.5s %-10.10s%c%c %.24s\n",
					font ? '+' : ' ',
		            who_rank_name[getrank(n)],
		            ch[n].name,
					IS_CLANKWAI(n) ? 'K' : (IS_CLANGORN(n) ? 'G' : ' '),
		            IS_PURPLE(n) ? '*' : ' ',
		            !showarea ? "--------" : get_area(n, 0));
	}
	if (players) fprintf(discordWho, "-------------------------------------------------\n");
	fprintf(discordWho, "%3d player%s online.\n", players, (players != 1) ? "s" : "");
	fprintf(discordWho, "```\n");
	
	discord_gmoon();
	
	fflush(discordWho);
	fclose(discordWho);
	
	discord_top_five();
	discord_pandium();
}

void discord_shout_in(void)
{
	char *text;
	long length;
	int  n;
	
	discordShoutIn = fopen("shoutin.txt", "r");
	
	fseek(discordShoutIn, 0, SEEK_END);
	length = ftell(discordShoutIn);
	fseek(discordShoutIn, 0, SEEK_SET);
	
	if (length > 200 || length < 2) 
	{
		fclose(discordShoutIn);
		return;
	}
	
	text = malloc(length);
	if (text)
	{
		fread(text, 1, length, discordShoutIn);
	}

	if (!text || strcmp(text, "$")==0) 
	{
		fclose(discordShoutIn);
		return;
	}
	
	for (n = 1; n<MAXCHARS; n++)
	{
		if ((ch[n].flags & (CF_PLAYER | CF_USURP)) && ch[n].used==USE_ACTIVE && !(ch[n].flags & CF_NOSHOUT))
		{
			do_char_log(n, 8, "%s\n", text);
		}
	}
	xlog("Received /sh from Discord");
	fclose(discordShoutIn);
	
	// Clean file after use
	discordShoutIn = fopen("shoutin.txt", "w");
	fprintf(discordShoutIn, "$");
	fflush(discordShoutIn);
	fclose(discordShoutIn);
	xlog("Received /sh from Discord");
}

void discord_ranked(char *text)
{
	discordRanked = fopen("ranked.txt", "w");
	fprintf(discordRanked, "```diff\n");
	fprintf(discordRanked, text);
	fprintf(discordRanked, "```\n");
	fflush(discordRanked);
	fclose(discordRanked);
}



/* disembodied server log */
void xlog(char *format, ...)
{
	va_list args;
	char buf[1024];
	struct tm *tm;
	time_t  t;

	va_start(args, format);
	vsprintf(buf, format, args);
	va_end(args);

	time(&t);
	tm = localtime(&t);
	fprintf(logfp, "%02d.%02d.%02d %02d:%02d:%02d: %s\n",
	        tm->tm_mday, tm->tm_mon + 1, tm->tm_year - 100,
	        tm->tm_hour, tm->tm_min, tm->tm_sec,
	        buf);
	fflush(logfp);
}

/* server log message about a character */
void chlog(int cn, char *format, ...)
{
	va_list args;
	char buf[1024];
	struct tm *tm;
	time_t  t;
	int nr, co;
	unsigned int addr;
	char *  name;
	char *  usurp = NULL;
	unsigned long long unique;

	nr = ch[cn].player;
	if (nr<1 || nr>MAXPLAYER)
	{
		nr = 0;
	}
	if (player[nr].usnr!=cn)
	{
		nr = 0;
	}
	if (nr)
	{
		addr = player[nr].addr;
		unique = player[nr].unique;
	}
	else
	{
		addr = 0;
		unique = 0;
	}

	name = ch[cn].name;

	if ((co = ch[cn].data[97]) && IS_SANECHAR(co) && IS_PLAYER(co))
	{
		usurp = ch[co].name;
	}

	va_start(args, format);
	vsprintf(buf, format, args);
	va_end(args);

	time(&t);
	tm = localtime(&t);
	fprintf(logfp, "%02d.%02d.%02d %02d:%02d:%02d: %s %s%s%s(%d at %d,%d from %d.%d.%d.%d, ID=%lld): %s\n",
	        tm->tm_mday, tm->tm_mon + 1, tm->tm_year - 100,
	        tm->tm_hour, tm->tm_min, tm->tm_sec,
	        name, usurp ? "[" : "", usurp ? usurp : "", usurp ? "] " : "",
	        cn, ch[cn].x, ch[cn].y,
	        addr & 255, (addr >> 8) & 255, (addr >> 16) & 255, addr >> 24,
	        unique, buf);
	fflush(logfp);
}

/* server log about a player */
void plog(int nr, char *format, ...)
{
	va_list args;
	char buf[1024];
	struct tm *tm;
	time_t  t;
	int cn, x, y;
	unsigned int addr;
	char *  name;
	unsigned long long unique;

	cn = player[nr].usnr;
	if (cn<=0 || cn>=MAXCHARS)
	{
		cn = 0;
	}
	if (ch[cn].player!=nr)
	{
		cn = 0;
	}
	if (cn)
	{
		name = ch[cn].name;
		x = ch[cn].x;
		y = ch[cn].y;
	}
	else
	{
		x = 0;
		y = 0;
		name = "*unknown*";
	}

	addr = player[nr].addr;
	unique = player[nr].unique;

	va_start(args, format);
	vsprintf(buf, format, args);
	va_end(args);

	time(&t);
	tm = localtime(&t);
	fprintf(logfp, "%02d.%02d.%02d %02d:%02d:%02d: %s (%d at %d,%d from %d.%d.%d.%d, ID=%lld): %s\n",
	        tm->tm_mday, tm->tm_mon + 1, tm->tm_year - 100,
	        tm->tm_hour, tm->tm_min, tm->tm_sec,
	        name, cn, x, y,
	        addr & 255, (addr >> 8) & 255, (addr >> 16) & 255, addr >> 24,
	        unique, buf);
	fflush(logfp);
}

/* Process a new connection by finding, initializing and connecting a player entry to a new socket */
void new_player(int sock)
{
	int n, m, nsock, len = sizeof(struct sockaddr_in), one = 1, onek = 65536, zero = 0;
	struct sockaddr_in addr;

	nsock = accept(sock, (struct sockaddr *)&addr, &len);
	if (nsock==-1)
	{
		xlog("new_player (server.c): accept() failed");
		return;
	}
	ioctl(nsock, FIONBIO, (u_long*)&one);     // non-blocking mode

	//setsockopt(nsock,IPPROTO_TCP,TCP_NODELAY,(const char *)&one,sizeof(int));
	setsockopt(nsock, SOL_SOCKET, SO_SNDBUF, (const char *)&onek, sizeof(int));
	setsockopt(nsock, SOL_SOCKET, SO_RCVBUF, (const char *)&onek, sizeof(int));
	setsockopt(nsock, SOL_SOCKET, SO_LINGER, (const char *)&zero, sizeof(int));
	setsockopt(nsock, SOL_SOCKET, SO_KEEPALIVE, (const char *)&one, sizeof(int));

	for (n = 1; n<MAXPLAYER; n++)
	{
		if (!player[n].sock)
		{
			break;
		}
	}
	if (n==MAXPLAYER)
	{
		close(nsock);
		xlog("new_player (server.c): MAXPLAYER reached");
		return;
	}

	player[n].zs.zalloc = Z_NULL;
	player[n].zs.zfree  = Z_NULL;
	player[n].zs.opaque = Z_NULL;
	if (deflateInit(&player[n].zs, 9))
	{
		close(nsock);
		xlog("new_player (server.c): could not init compressor");
		return;
	}

	player[n].sock = nsock;
	player[n].addr = addr.sin_addr.s_addr;

	player[n].inbuf[0] = 0;
	player[n].in_len = 0;
	player[n].iptr = 0;
	player[n].optr = 0;
	player[n].tptr = 0;
	player[n].challenge = 0;
	player[n].state = 0;
	player[n].usnr  = 0;
	player[n].pass1 = 0;
	player[n].pass2 = 0;
	player[n].state = ST_CONNECT;
	player[n].lasttick  = globs->ticker;
	player[n].lasttick2 = globs->ticker;
	player[n].prio = 0;
	player[n].ticker_started = 0;
	player[n].spectating = 0;

	memset(&player[n].cpl, 0, sizeof(struct cplayer));
	memset(player[n].cmap, 0, sizeof(struct cmap) * TILEX * TILEY);
	memset(player[n].xmap, 0, sizeof(player[n].xmap));
	memset(player[n].passwd, 0, sizeof(player[n].passwd));

	for (m = 0; m<TILEX * TILEY; m++)
	{
		player[n].cmap[m].ba_sprite = SPR_EMPTY;
		player[n].smap[m].ba_sprite = SPR_EMPTY;
	}

	plog(n, "New connection");
}

void send_player(int nr)
{
	int ret, len;
	char *ptr;

	if (player[nr].iptr<player[nr].optr)
	{
		len = OBUFSIZE - player[nr].optr;
		ptr = player[nr].obuf + player[nr].optr;
	}
	else
	{
		len = player[nr].iptr - player[nr].optr;
		ptr = player[nr].obuf + player[nr].optr;
	}

	ret = send(player[nr].sock, ptr, len, 0);
	if (ret==-1)    // send failure
	{
		plog(nr, "Connection closed (send, %s)", strerror(errno));
		plr_logout(player[nr].usnr, nr, 0);
		close(player[nr].sock);
		player[nr].sock  = 0;
		player[nr].ltick = 0;
		player[nr].rtick = 0;
		deflateEnd(&player[nr].zs);
		return;
	}
	globs->send += ret;

	player[nr].optr += ret;

	if (player[nr].optr==OBUFSIZE)
	{
		player[nr].optr = 0;
	}
}

int csend(int nr, unsigned char *buf, int len)
{
	int tmp;

	if (nr<1 || nr>=MAXPLAYER)
	{
		return -1;
	}

	if (player[nr].sock==0)
	{
		return -1;
	}

	while (len)
	{
		tmp = player[nr].iptr + 1;
		if (tmp==OBUFSIZE)
		{
			tmp = 0;
		}

		if (tmp==player[nr].optr)
		{
			plog(nr, "Connection to too slow, terminated");
			plr_logout(player[nr].usnr, nr, 0);
			close(player[nr].sock);
			player[nr].sock  = 0;
			player[nr].ltick = 0;
			player[nr].rtick = 0;
			deflateEnd(&player[nr].zs);
			return -1;
		}

		player[nr].obuf[player[nr].iptr] = *buf++;
		player[nr].iptr = tmp;
		len--;
	}
	return 0;
}

int pkt_cnt[256];
int pkt_mapshort = 0;
int pkt_light = 0;

void pkt_list(void)
{
	int n, m = 0, tot = 0;

	for (n = 0; n<256; n++)
	{
		tot += pkt_cnt[n];
		if (pkt_cnt[n]>m)
		{
			m = pkt_cnt[n];
		}
	}

	for (n = 0; n<256; n++)
	{
		if (pkt_cnt[n]>m / 16)
		{
			xlog("pkt type %2d: %5d (%.2f%%)", n, pkt_cnt[n], 100.0 / tot * pkt_cnt[n]);
		}
	}
	xlog("pkt type %2d: %5d (%.2f%%)", n, pkt_mapshort, 100.0 / tot * pkt_mapshort);
	n++;
	xlog("pkt type %2d: %5d (%.2f%%)", n, pkt_light, 100.0 / tot * pkt_light);
	n++;
}

void xsend(int nr, unsigned char *buf, int len)
{
	int pnr;

	if (nr<1 || nr>=MAXPLAYER)
	{
		return;
	}

	if (player[nr].sock==0)
	{
		return;
	}

	if (player[nr].tptr + len>=TBUFSIZE)
	{
		plog(nr, "#INTERNAL ERROR# ticksize too large, terminated");
		plr_logout(player[nr].usnr, nr, 0);
		close(player[nr].sock);
		player[nr].sock  = 0;
		player[nr].ltick = 0;
		player[nr].rtick = 0;
		deflateEnd(&player[nr].zs);
		return;
	}

	memcpy(player[nr].tbuf + player[nr].tptr, buf, len);
	player[nr].tptr += len;

	pnr = buf[0];
	pkt_cnt[pnr] += len;
	if (pnr>128)
	{
		pkt_mapshort += len;
	}
	else if (pnr==SV_SETMAP3 || pnr==SV_SETMAP4 || pnr==SV_SETMAP5 || pnr==SV_SETMAP6)
	{
		pkt_light += len;
	}
}

void rec_player(int nr)
{
	int len;

	len = recv(player[nr].sock, (char*)player[nr].inbuf + player[nr].in_len, 256 - player[nr].in_len, 0);

	if (len<1)      // receive failure
	{
		if (errno!=EWOULDBLOCK)
		{
			plog(nr, "Connection closed (recv, %s)", strerror(errno));
			plr_logout(player[nr].usnr, nr, 0);
			close(player[nr].sock);
			player[nr].sock  = 0;
			player[nr].ltick = 0;
			player[nr].rtick = 0;
			deflateEnd(&player[nr].zs);
		}
		return;
	}
	player[nr].in_len += len;
	globs->recv += len;
}

long long timel(void)
{
	struct timeval tv;

	gettimeofday(&tv, NULL);

	return((long long)tv.tv_sec * (long long)1000000 +
	       (long long)tv.tv_usec);
}

void compress_ticks(void)
{
	int n, ilen, olen, csize, ret;
	static unsigned char obuf[OBUFSIZE];

	for (n = 1; n<MAXPLAYER; n++)
	{
		if (!player[n].sock)
		{
			continue;
		}
		if (!player[n].ticker_started)
		{
			continue;
		}
		if (player[n].usnr<0 || player[n].usnr>=MAXCHARS)
		{
			player[n].usnr = 0;
		}

		ilen = player[n].tptr;
		olen = player[n].tptr + 2;

		if (olen>16)
		{
			player[n].zs.next_in  = player[n].tbuf;
			player[n].zs.avail_in = ilen;

			player[n].zs.next_out  = obuf;
			player[n].zs.avail_out = OBUFSIZE;

			ret = deflate(&player[n].zs, Z_SYNC_FLUSH);
			if (ret!=Z_OK)
			{
				xlog("ARGH %d", ret);
			}

			if (player[n].zs.avail_in)
			{
				printf("HELP (%d)\n", player[n].zs.avail_in);
			}

			csize = 65536 - player[n].zs.avail_out;

			olen = (csize + 2) | 0x8000;
			csend(n, (void*)(&olen), 2);
			csend(n, obuf, csize);
		}
		else
		{
			csend(n, (void*)(&olen), 2);
			if (ilen)
			{
				csend(n, player[n].tbuf, ilen);
			}
		}

		ch[player[n].usnr].comp_volume += olen;
		ch[player[n].usnr].raw_volume  += ilen;
		player[n].tptr = 0;

		//xlog("uncompressed tick size=%d byte",ilen+4);
		//xlog("compressed tick size=%d byte",olen+4);
	}
}

void game_loop(int sock)
{
	int n, fmax = 0, tmp;
	fd_set in_fd, out_fd;
	struct timeval tv;
	long long ttime;
	static long long ltime = 0;
	int tdiff, panic = 0;
	unsigned long long prof;

	if (ltime==0)
	{
		ltime = timel();
	}

	ttime = timel();
	if (ttime>ltime)
	{
		ltime += TICK;

		prof = prof_start();
		tick();
		prof_stop(25, prof);
		prof = prof_start();
		compress_ticks();
		prof_stop(44, prof);

		ttime = timel();
		if (ttime>ltime + TICK * TICKS * 10)// serious slowness, do something about that
		{
			xlog("Server too slow");
			ltime = ttime;
		}
	}
	tdiff = ltime - ttime;
	if (tdiff<1)
	{
		tdiff = 1;
	}

	if (globs->ticker)
	{
		while (panic<500)
		{

			prof = prof_start();

			panic++;

			FD_ZERO(&in_fd);
			FD_ZERO(&out_fd);

			FD_SET(sock, &in_fd);
			if (sock>fmax)
			{
				fmax = sock;
			}

			for (n = 1; n<MAXPLAYER; n++)
			{
				if (player[n].sock)
				{
					if (player[n].in_len<256)
					{
						FD_SET(player[n].sock, &in_fd);
						if (player[n].sock>fmax)
						{
							fmax = player[n].sock;
						}
					}
					if (player[n].iptr!=player[n].optr)
					{
						FD_SET(player[n].sock, &out_fd);
						if (player[n].sock>fmax)
						{
							fmax = player[n].sock;
						}
					}
				}
			}

			tv.tv_sec  = 0;
			tv.tv_usec = 0;

			prof_stop(42, prof);

			prof = prof_start();
			tmp  = select(fmax + 1, &in_fd, &out_fd, NULL, &tv);
			prof_stop(43, prof);

			if (tmp<1)
			{
				break;
			}

			prof = prof_start();

			if (FD_ISSET(sock, &in_fd))
			{
				new_player(sock);
			}

			for (n = 1; n<MAXPLAYER; n++)
			{
				if (!player[n].sock)
				{
					continue;
				}
				if (FD_ISSET(player[n].sock, &in_fd))
				{
					rec_player(n);
				}
				if (FD_ISSET(player[n].sock, &out_fd))
				{
					send_player(n);
				}
			}
			prof_stop(42, prof);
		}
	}

	ttime = timel();
	tdiff = ltime - ttime;
	if (tdiff<1)
	{
		return;
	}

	tv.tv_sec  = 0;
	tv.tv_usec = tdiff;
	prof = prof_start();
	select(0, NULL, NULL, NULL, &tv);
	prof_stop(43, prof);

}

void logrotate(int dummy)
{
	xlog("SIGHUP received, closing log...");
	if (logfp!=stdout)
	{
		fclose(logfp);
	}
	if (logfp!=stdout)
	{
		logfp = fopen("server.log", "a");
	}
	if (!logfp)
	{
		exit(1);
	}
	xlog("Re-opened log...");
}


void leave(int dummy)
{
	if (!quit)
	{
		xlog("Got signal to terminate. Shutdown initiated...");
	}
	else
	{
		xlog("Alright, alright, I'm already terminating!");
	}

	quit = 1;
}

unsigned long long proftab[100];

char *profname[100] = {
	"misc",         //0
	"  pathfinder", //1
	"  area_log",   //2
	"  area_say",   //3
	"  area_sound", //4
	"  area_notify", //5
	"  npc_shout",  //6
	"  update_char", //7
	" regenerate",  //8
	"  add_light",  //9
	" getmap",      //10
	" change",      //11
	" act",         //12
	" pop_tick",    //13
	" effect_tick", //14
	" item_tick",   //15
	"   can_see",   //16
	"   can_go",    //17
	"   compute_dlight", //18
	"   remove_lights", //19
	"   add_lights", //20
	"   char_can_see", //21
	"   char_can_see_item", //22
	"   god_create_item", //23
	" driver",      //24
	"tick",         //25
	"global_tick",  //26
	" npc_driver",  //27
	"  drv_dropto", //28
	"  drv_pickup", //29
	"  drv_give",   //30
	"  drv_use",    //31
	"  drv_bow",    //32
	"  drv_wave",   //33
	"  drv_turn",   //34
	"  drv_attack", //35
	"  drv_moveto", //36
	"  drv_skill",  //37
	"  drv_use",    //38
	"  npc_high",   //39
	"  plr_driver_med", //40
	" ccp_driver",  //41
	"net IO", //42
	"IDLE", //43
	"compress", //44
	" plr_save_char", //45
};

double cycles_per_sek = 0;

#define CYCLES_PER_SEK (cycles_per_sek)         //750000000.0
#define MAX_BEST       10
#define PROF_FREQ      2// in seconds

void god_prof(void)
{
	int bestn[MAX_BEST];
	unsigned long long bestv[MAX_BEST];
	int m, n, cn;

	for (n = 0; n<MAX_BEST; n++)
	{
		bestn[n] = -1;
		bestv[n] = 1;
	}

	for (n = 0; n<100; n++)
	{
		for (m = 0; m<MAX_BEST; m++)
		{
			if (proftab[n]>bestv[m])
			{
				if (m<(MAX_BEST - 1))
				{
					memmove(&bestv[m + 1], &bestv[m], sizeof(long long) * ((MAX_BEST - 1) - m));
					memmove(&bestn[m + 1], &bestn[m], sizeof(int) * ((MAX_BEST - 1) - m));
				}
				bestv[m] = proftab[n];
				bestn[m] = n;
				break;
			}
		}
	}

	for (cn = 1; cn<MAXCHARS; cn++)
	{
		int font;

		if (ch[cn].used==USE_EMPTY)
		{
			continue;
		}
		if (!(ch[cn].flags & CF_PROF))
		{
			continue;
		}

		for (n = 0; n<MAX_BEST && bestn[n]!=-1; n++)
		{
			if (strcmp(profname[bestn[n]], "IDLE")==0)
			{
				font = 3;
			}
			else
			{
				font = 2;
			}

			do_char_log(cn, font, "%-20.20s %-3.2f%%\n",
			            profname[bestn[n]],
			            (double)(100.0) / (double)(CYCLES_PER_SEK * PROF_FREQ) * (double)(proftab[bestn[n]]));
		}
		do_char_log(cn, 2, " \n");
	}
}

void prof_tick(void)
{
	int n;
	static int t = 0;

	t++;
	if (t<TICKS * PROF_FREQ)
	{
		return;
	}
	t = 0;

	god_prof();
	for (n = 0; n<100; n++)
	{
		proftab[n] = 0;
	}
}

unsigned long long prof_start(void)
{
	return(rdtsc());
}

void prof_stop(int task, unsigned long long cycle)
{
	unsigned long long td;

	td = rdtsc() - cycle;

	if (task>=0 && task<100)
	{
		proftab[task] += td;
	}
}

void tmplabcheck(int in)
{
	int cn;

	// carried by a player?
	if (!(cn = it[in].carried) | !IS_SANEPLAYER(cn))
	{
		return;
	}

	// player is inside a lab?
	if (ch[cn].temple_x!=512 && ch[cn].temple_x!=558 && ch[cn].temple_x!=813)
	{
		return;
	}

	god_take_from_char(in, cn);
	it[in].used = USE_EMPTY;

	chlog(cn, "Removed Lab Item %s", it[in].name);
}

int see_hit = 0, see_miss = 0;

int main(int argc, char *args[])
{
	int sock, n, one = 1, doleave = 0, ltimer = 0;
	struct sockaddr_in addr;
	int pidfile;
	char pid_str[10];
	unsigned long long t1, t2;

	nice(5);

	if (argc==1)
	{
		if (fork())
		{
			exit(0);
		}
		for (n = 0; n<256; n++)
		{
			close(n);
		}
		setsid();

		// open logfile
		logfp = fopen("server.log", "a");
		if (!logfp)
		{
			exit(1);
		}
	}
	else
	{
		logfp = stdout;
	}

	xlog("Mercenaries of Astonia Server v%d.%02d.%02d", VERSION >> 16, (VERSION >> 8) & 255, VERSION & 255);
	xlog("Copyright (C) 1997-2001 Daniel Brockhaus");

	t1 = rdtsc();
	sleep(1);
	t2 = rdtsc();
	t1 = t2 - t1;
	cycles_per_sek = ((t1 + 25000000) / 50000000) * 50000000;
	xlog("Speed test: %.0f Mhz", cycles_per_sek / 1000000);

	// ignore the silly pipe errors:
	signal(SIGPIPE, SIG_IGN);

	signal(SIGHUP, logrotate);

	// shutdown gracefully if possible:
	signal(SIGQUIT, leave);
	signal(SIGINT, leave);
	signal(SIGTERM, leave);
	signal(SIGCHLD, SIG_IGN);

	for (n = 0; n<100; n++)
	{
		proftab[n] = 0;
	}

	if (load())
	{
		xlog("load() failed.\n");
		return 1;
	}

	if (globs->flags & GF_DIRTY)
	{
		xlog("Data files were not cleanly unmounted.");
		if (argc!=2)
		{
			exit(1);
		}
	}

	see = malloc(sizeof(struct see_map) * MAXCHARS);
	if (see==NULL)
	{
		xlog("Memory low!");
		exit(1);
	}

	for (n = 1; n<MAXCHARS; n++)
	{
		see[n].x = see[n].y = 0;
	}

	if (argc==2)
	{
		if (strcasecmp("pop", args[1])==0)
		{
			populate();
			unload();
			exit(0);
		}
		else if (strcasecmp("rem", args[1])==0)
		{
			pop_remove();
			unload();
			exit(0);
		}
		else if (strcasecmp("wipe", args[1])==0)
		{
			pop_wipe();
			unload();
			exit(0);
		}
		else if (strcasecmp("light", args[1])==0)
		{
			init_lights();
			unload();
			exit(0);
		}
		else if (strcasecmp("skill", args[1])==0)
		{
			pop_skill();
			unload();
			exit(0);
		}
		else if (strcasecmp("load", args[1])==0)
		{
			pop_load_all_chars();
			unload();
			exit(0);
		}
		else if (strcasecmp("save", args[1])==0)
		{
			pop_save_all_chars();
			unload();
			exit(0);
		}
		else if (strcasecmp("copy", args[1])==0)
		{
			pop_copy_to_new_chars();
			unload();
			exit(0);
		}
	}

	sock = socket(PF_INET, SOCK_STREAM, 0);
	if (sock==-1)
	{
		xlog("socket() failed.\n");
		return 1;
	}

	ioctl(sock, FIONBIO, (u_long*)&one);      // non-blocking mode
	setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (const char *)&one, sizeof(int));

	addr.sin_family = AF_INET;
	addr.sin_port = htons(5555);
	addr.sin_addr.s_addr = 0;

	if (bind(sock, (struct sockaddr *)&addr, sizeof(addr)))
	{
		xlog("bind() failed.\n");
		return 1;
	}

	if (listen(sock, 5))
	{
		xlog("listen() failed.\n");
		return 1;
	}

	for (n = 1; n<MAXPLAYER; n++)
	{
		player[n].sock  = 0;
		player[n].tbuf  = malloc(16 * TBUFSIZE);
		player[n].obuf  = malloc(OBUFSIZE);
		player[n].ltick = 0;
		player[n].rtick = 0;
	}

	for (n = 1; n<MAXCHARS; n++)
	{
		if (ch[n].used==USE_ACTIVE)
		{
			plr_logout(n, 0, LO_SHUTDOWN);
		}
		/* // (vvv REMOVE AFTER UPDATE!!!)
		ch[n].data[23] = ch[n].data[23] + ch[n].data[24] + ch[n].data[25];
		ch[n].data[24] = 0;
		ch[n].data[25] = 0;
		// */ // (^^^ REMOVE AFTER UPDATE!!!)
		ch[n].data[75] = 0;
		clear_map_buffs(n, 1);
	}
	
	/* // (vvv REMOVE AFTER UPDATE!!!)
	for (n = 1; n<MAXITEM; n++)
	{
		if (it[n].used==USE_EMPTY) continue;
		if (it[n].orig_temp && it[n].crit_multi[0] != it_temp[it[n].orig_temp].crit_multi[0])
		{
			it[n].flags |= IF_UPDATE | IF_NOREPAIR | IF_LEGACY;
			it[n].max_damage = 100000;
		}
	}
	// */ // (^^^ REMOVE AFTER UPDATE!!!)

	srand(time(NULL));

	/* set up pid file */
	pidfile = open("server.pid", O_WRONLY | O_CREAT | O_TRUNC, 0664);
	if (pidfile!=-1)
	{
		sprintf(pid_str, "%d\n", getpid());
		write(pidfile, pid_str, strlen(pid_str));
		close(pidfile);
		chmod("server.pid", 0664);
	}

	init_node();
	init_lab9();
	god_init_freelist();
	god_init_badnames();
	init_badwords();
	god_read_banlist();
	reset_changed_items();
	
	/*
	for (n = 1; n<MAXITEM; n++)
	{
		if (!it[n].used)
		{
			continue;
		}
		if ((it[n].flags & IF_STACKABLE) && it[n].stack == 0)
		{
			it[n].stack = 1;
			it[n].flags |= IF_UPDATE;
		}
		if (it[n].flags & IF_LABYDESTROY)
		{
			tmplabcheck(n);
		}
		if ((it[n].flags & IF_SOULSTONE) && it[n].max_damage==0)
		{
			if 		(it[n].power == 60) it[n].max_damage = 65000;
			else if	(it[n].power == 75)	it[n].max_damage = 85000;
			else 						it[n].max_damage = 60000;
			xlog("Set %s (%d) max_damage to %d", it[n].name, n, it[n].max_damage);
		}
		
	}
	*/
	
	for (n = 1; n<MAXTCHARS; n++)
	{
		int x, y;

		if (!ch_temp[n].used)
		{
			continue;
		}

		x = ch_temp[n].data[29] % MAPX;
		y = ch_temp[n].data[29] / MAPX;

		if (!x && !y)
		{
			continue;
		}

		if (abs(x - ch_temp[n].x) + abs(y - ch_temp[n].y)>200)
		{
			xlog("RESET %d (%s): %d %d -> %d %d", n, ch_temp[n].name, ch_temp[n].x, ch_temp[n].y, x, y);
			ch_temp[n].data[29] = ch_temp[n].x + ch_temp[n].y * MAPX;
		}
	}
	
	/*
	god_reset_npcs(0);
	init_lights();
	*/

	globs->flags |= GF_DIRTY;

	load_mod();

	xlog("Entering game loop...");

	while (!doleave)
	{
		if ((globs->ticker & 4095)==0)
		{
			load_mod();
			update();
		}
		game_loop(sock);
		if (quit)
		{
			if  (ltimer==0)
			{
				// kick all players
				for (n = 1; n<MAXPLAYER; n++)
				{
					if (player[n].sock)
					{
						plr_logout(player[n].usnr, n, LO_SHUTDOWN);
					}
				}
				xlog("Sending shutdown message");
				ltimer++;
			}
			else
			{
				ltimer++;
			}
			if (ltimer>25)   // reset this to 250 !!!
			{
				xlog("Leaving main loop");
				// savety measure only. Players should be out already
				for (n = 1; n<MAXPLAYER; n++)
				{
					if (player[n].sock)
					{
						close(player[n].sock);
					}
				}
				doleave = 1;
			}
		}
	}

	globs->flags &= ~GF_DIRTY;
	unload();

	xlog("Server down (%d,%d)", see_hit, see_miss);
	unlink("server.pid");
	if (logfp!=stdout)
	{
		fclose(logfp);
	}
	fclose(discordWho);
	fclose(discordShoutIn);
	fclose(discordShoutOut);
	fclose(discordRanked);
	fclose(discordTopA);
	fclose(discordPandium);

	return 0;
}

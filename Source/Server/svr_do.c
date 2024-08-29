/*************************************************************************

   This file is part of 'Mercenaries of Astonia v2'
   Copyright (c) 1997-2001 Daniel Brockhaus (joker@astonia.com)
   All rights reserved.

 **************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#define _XOPEN_SOURCE
#define __USE_XOPEN
#include <unistd.h>
#include <math.h>

#include "server.h"

#define KILLERONLY

#define AREASIZE   12
#define BASESPIRALSIZE	20
#define SPIRALSIZE ((2*BASESPIRALSIZE+1)*(2*BASESPIRALSIZE+1))

int do_is_ignore(int cn, int co, int flag);
void do_look_player_depot(int cn, char *cv);
void do_look_player_inventory(int cn, char *cv);
void do_look_player_equipment(int cn, char *cv);
void do_steal_player(int cn, char *cv, char *ci);

/* CS, 991113: Support for outwardly spiralling area with a single loop */
int areaspiral[SPIRALSIZE] = {0, 0};

/* This routine initializes areaspiral[] with a set of offsets from a given location
   that form a spiral starting from a central point, where the offset is 0. */
void initspiral()
{
	int j, dist;
	int point = 0; // offset in array

	areaspiral[point] = 0;
	for (dist = 1; dist<=BASESPIRALSIZE; dist++)
	{
		areaspiral[++point] = -MAPX;                          // N
		for (j = 2 * dist - 1; j; j--)
		{
			areaspiral[++point] = -1;                     // W
		}
		for (j = 2 * dist; j; j--)
		{
			areaspiral[++point] = MAPX;                   // S
		}
		for (j = 2 * dist; j; j--)
		{
			areaspiral[++point] = 1;                      // E
		}
		for (j = 2 * dist; j; j--)
		{
			areaspiral[++point] = -MAPX;                  // N
		}
	}
}


void do_area_log(int cn, int co, int xs, int ys, int font, char *format, ...) // cn,co are the only ones NOT to get the message
{
	int x, y, cc, m, n, nr;
	va_list args;
	char buf[1024];
	unsigned long long prof;

	prof = prof_start();

	va_start(args, format);
	vsprintf(buf, format, args);
	va_end(args);

	for (y = max(0, ys - 12); y<min(MAPY, ys + 13); y++) // was 12, 13
	{
		m = y * MAPX;
		for (x = max(0, xs - 12); x<min(MAPX, xs + 13); x++)
		{
			if ((cc = map[x + m].ch)!=0)
			{
				for (n=1; n<MAXCHARS; n++)
				{
					if (ch[n].used==USE_EMPTY)
						continue;
					if (!IS_SANEPLAYER(n) || !IS_ACTIVECHAR(n))
						continue;
					if (n==cc) 
						continue;
					nr = ch[n].player;
					if (player[nr].spectating && player[nr].spectating == cc)
					{
						do_log(n, font, buf);
					}
				}
				if (cc!=cn && cc!=co)
				{
					if ((!ch[cc].player && ch[cc].temp!=15) || ((ch[cc].flags & CF_SYS_OFF) && font==0))
					{
						continue;
					}
					do_log(cc, font, buf);
				}
			}
		}
	}
	prof_stop(2, prof);
}

/* CS, 991113: Respect invisibility in 'say'. */
void do_area_say1(int cn, int xs, int ys, char *msg)
{
	char msg_named[500], msg_invis[500];
	int  invis = 0;
	int  npcs[20], cnt = 0;
	int  j, x, m, cc, nr;
	unsigned long long prof;

	prof = prof_start();

	sprintf(msg_named, "%.30s: \"%.300s\"\n", ch[cn].name, msg);
	if (IS_INVISIBLE(cn))
	{
		invis = 1;
		sprintf(msg_invis, "Somebody says: \"%.300s\"\n", msg);
	}
	if (areaspiral[1] == 0)
	{
		initspiral();
	}
	m = XY2M(xs, ys); // starting point
	for (j = 0; j<SPIRALSIZE; j++)
	{
		m += areaspiral[j];
		if (m < 0 || m >= (MAPX * MAPY))
		{
			continue;
		}
		cc = map[m].ch;
		if (!IS_SANECHAR(cc))
		{
			continue;
		}
		if ((ch[cc].flags & (CF_PLAYER | CF_USURP)))      // listener is a player
		{
			if (!invis || invis_level(cn) <= invis_level(cc))
			{
				do_log(cc, 3, msg_named); // talker visible to listener
			}
			else
			{
				do_log(cc, 3, msg_invis); // talker invis
			}
			for (x=1; x<MAXCHARS; x++)
			{
				if (ch[x].used==USE_EMPTY)
					continue;
				if (!IS_SANEPLAYER(x) || !IS_ACTIVECHAR(x))
					continue;
				if (/*x==cn || */x==cc) 
					continue;
				nr = ch[x].player;
				if (/*player[nr].spectating == cn || */player[nr].spectating == cc)
				{
					if (!invis || invis_level(cn) <= invis_level(x))
					{
						do_log(x, 3, msg_named); // talker visible to listener
					}
					else
					{
						do_log(x, 3, msg_invis); // talker invis
					}
				}
			}
		}
		else     // listener is NPC: Store in list for second pass
		{        // DB: note: this should be changed for staff/god NPCs
			if (!invis && cnt<ARRAYSIZE(npcs))   // NPCs pretend not to hear invis people
			{
				if (j < 169)   // don't address mobs outside radius 6
				{
					npcs[cnt++] = cc;
				}
			}
		}
	}
	for (j = 0; j<cnt; j++)
	{
		if (do_char_can_see(npcs[j], cn, 0))
		{
			npc_hear(npcs[j], cn, msg);
		}
	}
	prof_stop(3, prof);
}

void do_area_sound(int cn, int co, int xs, int ys, int nr)
{
	int x, y, cc, s, m;
	int xvol, xpan;
	unsigned long long prof;

	prof = prof_start();
	for (y = max(0, ys - 12); y<min(MAPY, ys + 13); y++) // 8, 9
	{
		m = y * MAPX;
		for (x = max(0, xs - 12); x<min(MAPX, xs + 13); x++)
		{
			if ((cc = map[x + m].ch)!=0)
			{
				if (cc!=cn && cc!=co)
				{
					if (!ch[cc].player)
					{
						continue;
					}
					s = ys - y + xs - x;
					if (s<0)
					{
						xpan = -500;
					}
					else if (s>0)
					{
						xpan = 500;
					}
					else
					{
						xpan = 0;
					}

					s = ((ys - y) * (ys - y) + (xs - x) * (xs - x)) * 30;

					xvol = -150 - s;
					if (xvol<-5000)
					{
						xvol = -5000;
					}
					char_play_sound(cc, nr, xvol, xpan);
				}
			}
		}
	}
	prof_stop(4, prof);
}

void do_notify_char(int cn, int type, int dat1, int dat2, int dat3, int dat4)
{
	driver_msg(cn, type, dat1, dat2, dat3, dat4);
}

void do_area_notify(int cn, int co, int xs, int ys, int type, int dat1, int dat2, int dat3, int dat4)
{
	int x, y, cc, m;
	unsigned long long prof;

	prof = prof_start();

	for (y = max(0, ys - AREASIZE); y<min(MAPY, ys + AREASIZE + 1); y++)
	{
		m = y * MAPX;
		for (x = max(0, xs - AREASIZE); x<min(MAPX, xs + AREASIZE + 1); x++)
		{
			if ((cc = map[x + m].ch)!=0)
			{
				if (cc!=cn && cc!=co)
				{
					do_notify_char(cc, type, dat1, dat2, dat3, dat4);
				}
			}
		}
	}
	prof_stop(5, prof);
}

// use this one sparingly! It uses quite a bit of computation time!
/* This routine finds the 3 closest NPCs to the one doing the shouting,
   so that they can come to the shouter's rescue or something. */
void do_npc_shout(int cn, int type, int dat1, int dat2, int dat3, int dat4)
{
	int co, dist;
	int best[3] = {99, 99, 99}, bestn[3] = {0, 0, 0};
	unsigned long long prof;

	prof = prof_start();
	if (ch[cn].data[52]==3)
	{

		for (co = 1; co<MAXCHARS; co++)
		{
			if (co!=cn && ch[co].used==USE_ACTIVE && !(ch[co].flags & CF_BODY))
			{
				if (ch[co].flags & (CF_PLAYER | CF_USURP))
				{
					continue;
				}
				if (ch[co].data[53]!=ch[cn].data[52])
				{
					continue;
				}
				dist = abs(ch[cn].x - ch[co].x) + abs(ch[cn].y - ch[co].y);
				if (dist<best[0])
				{
					best[2]  = best[1];
					best[1]  = best[0];
					bestn[2] = bestn[1];
					bestn[1] = bestn[0];
					best[0]  = dist;
					bestn[0] = co;
				}
				else if (dist<best[1])
				{
					best[2]  = best[1];
					bestn[2] = bestn[1];
					best[1]  = dist;
					bestn[1] = co;
				}
				else if (dist<best[3])
				{
					best[3]  = dist;
					bestn[3] = co;
				}
			}
		}

		if (bestn[0])
		{
			do_notify_char(bestn[0], type, dat1, dat2, dat3, dat4);
		}
		if (bestn[1])
		{
			do_notify_char(bestn[1], type, dat1, dat2, dat3, dat4);
		}
		if (bestn[2])
		{
			do_notify_char(bestn[2], type, dat1, dat2, dat3, dat4);
		}
	}
	else
	{
		for (co = 1; co<MAXCHARS; co++)
		{
			if (co!=cn && ch[co].used==USE_ACTIVE && !(ch[co].flags & CF_BODY))
			{
				if (ch[co].flags & (CF_PLAYER | CF_USURP))
				{
					continue;
				}
				if (ch[co].data[53]!=ch[cn].data[52])
				{
					continue;
				}
				do_notify_char(co, type, dat1, dat2, dat3, dat4);
			}
		}
	}
	prof_stop(6, prof);
}

void do_motd(int cn, int font, char *text)
{
	int n = 0, len, nr;
	unsigned char buf[16];

	nr = ch[cn].player;
	if (nr<1 || nr>=MAXPLAYER)
	{
		return;
	}

	if (player[nr].usnr!=cn)
	{
		ch[cn].player = 0;
		return;
	}

	len = strlen(text) - 1;

	while (n<=len)
	{
		buf[0] = SV_MOTD + font;
		memcpy(buf + 1, text + n, 15); // possible bug: n+15>textend !!!
		xsend(ch[cn].player, buf, 16);

		n += 15;
	}
}

void do_char_motd(int cn, int font, char *format, ...)
{
	va_list args;
	char buf[1024];

	if (!ch[cn].player && ch[cn].temp!=15)
	{
		return;
	}

	va_start(args, format);
	vsprintf(buf, format, args);
	do_motd(cn, font, buf);
	va_end(args);
}


void do_log(int cn, int font, char *text)
{
	int n = 0, len, nr, m=0;
	unsigned char buf[16];

	nr = ch[cn].player;
	
	if (nr<1 || nr>=MAXPLAYER)
	{
		return;
	}

	if (player[nr].usnr!=cn && !player[nr].spectating)
	{
		ch[cn].player = 0;
		return;
	}

	len = strlen(text) - 1;

	while (n<=len)
	{
		buf[0] = SV_LOG + font;
		memcpy(buf + 1, text + n, 15); // possible bug: n+15>textend !!!
		xsend(ch[cn].player, buf, 16);

		n += 15;
	}
}

void do_char_log(int cn, int font, char *format, ...)
{
	va_list args;
	char buf[1024];

	if (!ch[cn].player && ch[cn].temp!=15)
	{
		return;
	}

	va_start(args, format);
	vsprintf(buf, format, args);
	do_log(cn, font, buf);
	va_end(args);
}

void do_staff_log(int font, char *format, ...)
{
	va_list args;
	char buf[1024];
	int  n;

	va_start(args, format);
	vsprintf(buf, format, args);

	for (n = 1; n<MAXCHARS; n++)
	{
		if (ch[n].player && (ch[n].flags & (CF_STAFF | CF_IMP | CF_USURP)) && !(ch[n].flags & CF_NOSTAFF))
		{
			do_log(n, font, buf);
		}
	}
	va_end(args);
}

/* CS, 991113: #NOWHO and visibility levels */
/* A level conscious variant of do_staff_log() */
void do_admin_log(int source, char *format, ...)
{
	va_list args;
	char buf[1024];
	int  n;

	va_start(args, format);
	vsprintf(buf, format, args);

	for (n = 1; n<MAXCHARS; n++)
	{
		/* various tests to exclude listeners (n) */
		if (!ch[n].player)
		{
			continue;            // not a player
		}
		if (!(ch[n].flags & (CF_STAFF | CF_IMP | CF_USURP)))
		{
			continue;                                          // only to staffers and QMs
		}
		if ((ch[source].flags & (CF_INVISIBLE | CF_NOWHO)) && // privacy wanted
		    invis_level(source) > invis_level(n))
		{
			continue;                                   // and source outranks listener

		}
		do_log(n, 2, buf);
	}
	va_end(args);
}

/* CS, 991205: Players see new arrivals */
/* Announcement to all players */
void do_announce(int source, int author, char *format, ...)
{
	va_list args;
	char buf_anon[1024];
	char buf_named[1024];
	int  n;

	if (!*format)
	{
		return;
	}

	va_start(args, format);
	vsprintf(buf_anon, format, args);
	if (author)
	{
		sprintf(buf_named, "[%s] %s", ch[author].name, buf_anon);
	}
	else
	{
		strcpy(buf_named, buf_anon);
	}

	for (n = 1; n<MAXCHARS; n++)
	{
		/* various tests to exclude listeners (n) */
		if (!ch[n].player && ch[n].temp!=15)
		{
			continue;                              // not a player
		}
		if ((ch[source].flags & (CF_INVISIBLE | CF_NOWHO)) && // privacy wanted
		    invis_level(source) > invis_level(n))
		{
			continue;                                   // and source outranks listener
		}
		if ((source != 0) && (invis_level(source) <= invis_level(n)))
		{
			do_log(n, 2, buf_named);
		}
		else
		{
			do_log(n, 2, buf_anon);
		}
	}
	va_end(args);
}

void do_server_announce(int source, int author, char *format, ...)
{
	va_list args;
	char buf_anon[1024];
	char buf_named[1024];
	int  n;

	if (!*format)
	{
		return;
	}

	va_start(args, format);
	vsprintf(buf_anon, format, args);
	if (author)
	{
		sprintf(buf_named, "[%s] %s", ch[author].name, buf_anon);
	}
	else
	{
		strcpy(buf_named, buf_anon);
	}

	for (n = 1; n<MAXCHARS; n++)
	{
		/* various tests to exclude listeners (n) */
		if (!ch[n].player && ch[n].temp!=15)
		{
			continue;                              // not a player
		}
		if ((source != 0) && (invis_level(source) <= invis_level(n)))
		{
			do_log(n, 9, buf_named);
		}
		else
		{
			do_log(n, 9, buf_anon);
		}
	}
	va_end(args);
}

void do_caution(int source, int author, char *format, ...)
{
	va_list args;
	char buf_anon[1024];
	char buf_named[1024];
	int  n;

	if (!*format)
	{
		return;
	}

	va_start(args, format);
	vsprintf(buf_anon, format, args);
	if (author)
	{
		sprintf(buf_named, "[%s] %s", ch[author].name, buf_anon);
	}
	else
	{
		strcpy(buf_named, buf_anon);
	}

	for (n = 1; n<MAXCHARS; n++)
	{
		/* various tests to exclude listeners (n) */
		if (!ch[n].player && ch[n].temp!=15)
		{
			continue;                              // not a player
		}
		if ((ch[source].flags & (CF_INVISIBLE | CF_NOWHO)) && // privacy wanted
		    invis_level(source) > invis_level(n))
		{
			continue;                                   // and source outranks listener
		}
		if ((source != 0) && (invis_level(source) <= invis_level(n)))
		{
			do_log(n, 0, buf_named);
		}
		else
		{
			do_log(n, 0, buf_anon);
		}
	}
	va_end(args);
}

void do_imp_log(int font, char *format, ...)
{
	va_list args;
	char buf[1024];
	int  n;

	va_start(args, format);
	vsprintf(buf, format, args);

	for (n = 1; n<MAXCHARS; n++)
	{
		if (ch[n].player && (ch[n].flags & (CF_IMP | CF_USURP)))
		{
			do_log(n, font, buf);
		}
	}
	va_end(args);
}

/* CS, 991204: Match on partial names */
int do_lookup_char(char *name)
{
	int n;
	char matchname[100];
	int len;
	int bestmatch = 0;
	int quality = 0; // 1 = npc 2 = inactive plr 3 = active plr

	len = strlen(name);
	if (len < 2)
	{
		return 0;
	}
	sprintf(matchname, "%-.90s", name);

	for (n = 1; n<MAXCHARS; n++)
	{
		if (ch[n].used!=USE_ACTIVE && ch[n].used!=USE_NONACTIVE)
		{
			continue;
		}
		if (ch[n].flags & CF_BODY)
		{
			continue;
		}
		if (strncasecmp(ch[n].name, matchname, len))
		{
			continue;
		}
		if (strlen(ch[n].name) == len)   // perfect match
		{
			bestmatch = n;
			break;
		}
		if (ch[n].flags & (CF_PLAYER | CF_USURP))
		{
			if (ch[n].x != 0)   // active plr
			{
				if (quality < 3)
				{
					bestmatch = n;
					quality = 3;
				}
			}
			else     // inactive plr
			{
				if (quality < 2)
				{
					bestmatch = n;
					quality = 2;
				}
			}
		}
		else     // NPC
		{
			if (quality < 1)
			{
				bestmatch = n;
				quality = 1;
			}
		}
	}
	return(bestmatch);
}

/* look up a character by name.
   special case "self" returns the looker. */
int do_lookup_char_self(char *name, int cn)
{
	if (!strcasecmp(name, "self"))
	{
		return( cn);
	}
	return(do_lookup_char(name));
}

int do_is_ignore(int cn, int co, int flag)
{
	int n;

	if (!flag)
	{
		for (n = 30; n<39; n++)
		{
			if (ch[co].data[n]==cn)
			{
				return 1;
			}
		}
	}


	for (n = 50; n<59; n++)
	{
		if (ch[co].data[n]==cn)
		{
			return 1;
		}
	}

	return 0;
}

void do_tell(int cn, char *con, char *text)
{
	int co;
	char buf[256];

	if (ch[cn].flags & CF_SHUTUP)
	{
		do_char_log(cn, 0, "You try to speak, but you only produce a croaking sound.\n");
		return;
	}

	co = do_lookup_char(con);
	if (!co)
	{
		do_char_log(cn, 0, "Unknown name: %s\n", con);
		return;
	}
	if (!(ch[co].flags & (CF_PLAYER)) || ch[co].used!=USE_ACTIVE || ((ch[co].flags & CF_INVISIBLE) && invis_level(cn)<invis_level(co)) ||
	    (!(ch[cn].flags & CF_GOD) && ((ch[co].flags & CF_NOTELL) || do_is_ignore(cn, co, 0))))
	{
		do_char_log(cn, 0, "%s is not listening\n", ch[co].name);
		return;
	}

	/* CS, 991127: Support for AFK <message> */
	if (ch[co].data[0])
	{
		if (ch[co].text[0][0])
		{
			do_char_log(cn, 0, "%s is away from keyboard; Message:\n", ch[co].name);
			do_char_log(cn, 7, "  \"%s\"\n", ch[co].text[0]);
		}
		else
		{
			do_char_log(cn, 0, "%s is away from keyboard.\n", ch[co].name);
		}
	}

	if (!text)
	{
		do_char_log(cn, 0, "I understand that you want to tell %s something. But what?\n", ch[co].name);
		return;
	}

	if ((ch[cn].flags & CF_INVISIBLE) && invis_level(cn)>invis_level(co))
	{
		sprintf(buf, "Somebody tells you: \"%.200s\"\n", text);
	}
	else
	{
		sprintf(buf, "%s tells you: \"%.200s\"\n", ch[cn].name, text);
	}
	do_char_log(co, 7, "%s", buf);

	if (ch[co].flags & CF_CCP)
	{
		ccp_tell(co, cn, text);
	}

	do_char_log(cn, 1, "Told %s: \"%.200s\"\n", ch[co].name, text);

	if (cn==co)
	{
		do_char_log(cn, 1, "Do you like talking to yourself?\n");
	}

	if (ch[cn].flags & (CF_PLAYER))
	{
		chlog(cn, "Told %s: \"%s\"", ch[co].name, text);
	}
}

void do_notell(int cn)
{
	ch[cn].flags ^= CF_NOTELL;

	if (ch[cn].flags & CF_NOTELL)
	{
		do_char_log(cn, 1, "You will no longer hear people #tell you something.\n");
	}
	else
	{
		do_char_log(cn, 1, "You will hear if people #tell you something.\n");
	}

	if (ch[cn].flags & (CF_PLAYER))
	{
		chlog(cn, "Set notell to %s", (ch[cn].flags & CF_NOTELL) ? "on" : "off");
	}
}

void do_noshout(int cn)
{
	ch[cn].flags ^= CF_NOSHOUT;

	if (ch[cn].flags & CF_NOSHOUT)
	{
		do_char_log(cn, 1, "You will no longer hear people #shout.\n");
	}
	else
	{
		do_char_log(cn, 1, "You will hear people #shout.\n");
	}

	if (ch[cn].flags & (CF_PLAYER))
	{
		chlog(cn, "Set noshout to %s", (ch[cn].flags & CF_NOSHOUT) ? "on" : "off");
	}
}

void do_toggle_aoe(int cn)
{
	ch[cn].flags ^= CF_AREA_OFF;

	if (ch[cn].flags & CF_AREA_OFF)
	{
		do_char_log(cn, 1, "You will no longer deal area-of-effect.\n");
	}
	else
	{
		do_char_log(cn, 1, "You will deal area-of-effect again.\n");
	}

	if (ch[cn].flags & (CF_PLAYER))
	{
		chlog(cn, "Set area_off to %s", (ch[cn].flags & CF_AREA_OFF) ? "on" : "off");
	}
}

void do_toggle_appraisal(int cn)
{
	ch[cn].flags ^= CF_APPR_OFF;

	if (ch[cn].flags & CF_APPR_OFF)
	{
		do_char_log(cn, 1, "You will no longer see appraisal messages.\n");
	}
	else
	{
		do_char_log(cn, 1, "You will see appraisal messages again.\n");
	}

	if (ch[cn].flags & (CF_PLAYER))
	{
		chlog(cn, "Set appr_off to %s", (ch[cn].flags & CF_APPR_OFF) ? "on" : "off");
	}
}

void do_toggle_spellknowledge(int cn)
{
	ch[cn].flags ^= CF_KNOW_OFF;

	if (ch[cn].flags & CF_KNOW_OFF)
	{
		do_char_log(cn, 1, "You will no longer see buff or debuff details.\n");
	}
	else
	{
		do_char_log(cn, 1, "You will see buff and debuff details again.\n");
	}

	if (ch[cn].flags & (CF_PLAYER))
	{
		chlog(cn, "Set know_off to %s", (ch[cn].flags & CF_KNOW_OFF) ? "on" : "off");
	}
}

void do_autoloot(int cn)
{
	int cc;
	ch[cn].flags ^= CF_AUTOLOOT;
	
	if (ch[cn].flags & CF_AUTOLOOT)
	{
		do_char_log(cn, 1, "You will now automatically loot graves.\n");
	}
	else
	{
		do_char_log(cn, 1, "You will no longer automatically loot graves.\n");
	}
	
	if (ch[cn].flags & (CF_PLAYER))
	{
		chlog(cn, "Set autoloot to %s", (ch[cn].flags & CF_AUTOLOOT) ? "on" : "off");
	}
}

void do_sense(int cn)
{
	int cc;
	ch[cn].flags ^= CF_SENSEOFF;
	
	if (ch[cn].flags & CF_SENSEOFF)
	{
		do_char_log(cn, 1, "You will no longer see sense-magic spell messages.\n");
	}
	else
	{
		do_char_log(cn, 1, "You will see sense-magic spell messages again.\n");
	}
	
	if (ch[cn].flags & (CF_PLAYER))
	{
		chlog(cn, "Set sense to %s", (ch[cn].flags & CF_SENSEOFF) ? "on" : "off");
	}
}

void do_silence(int cn)
{
	int cc;
	ch[cn].flags ^= CF_SILENCE;
	
	if ((cc = ch[cn].data[PCD_COMPANION]) && IS_SANECHAR(cc))
	{
		ch[cc].flags ^= CF_SILENCE;
	}
	if ((cc = ch[cn].data[PCD_SHADOWCOPY]) && IS_SANECHAR(cc))
	{
		ch[cc].flags ^= CF_SILENCE;
	}

	if (ch[cn].flags & CF_SILENCE)
	{
		do_char_log(cn, 1, "You will no longer hear enemy messages.\n");
	}
	else
	{
		do_char_log(cn, 1, "You will hear enemy messages again.\n");
	}

	if (ch[cn].flags & (CF_PLAYER))
	{
		chlog(cn, "Set silence to %s", (ch[cn].flags & CF_SILENCE) ? "on" : "off");
	}
}

void do_override(int cn)
{
	int cc;
	ch[cn].flags ^= CF_OVERRIDE;

	if (ch[cn].flags & CF_OVERRIDE)
	{
		do_char_log(cn, 1, "You can now override your own buffs.\n");
	}
	else
	{
		do_char_log(cn, 1, "You will no longer override your own buffs.\n");
	}

	if (ch[cn].flags & (CF_PLAYER))
	{
		chlog(cn, "Set override to %s", (ch[cn].flags & CF_OVERRIDE) ? "on" : "off");
	}
}

void do_sysoff(int cn)
{
	int cc;
	ch[cn].flags ^= CF_SYS_OFF;

	if (ch[cn].flags & CF_SYS_OFF)
	{
		do_char_log(cn, 1, "You will no longer see system messages.\n");
	}
	else
	{
		do_char_log(cn, 1, "You will now see system messages again.\n");
	}

	if (ch[cn].flags & (CF_PLAYER))
	{
		chlog(cn, "Set override to %s", (ch[cn].flags & CF_SYS_OFF) ? "on" : "off");
	}
}

void do_gctome(int cn)
{
	ch[cn].flags ^= CF_GCTOME;

	if (ch[cn].flags & CF_GCTOME)
	{
		do_char_log(cn, 1, "Your companions will now follow you through passages.\n");
	}
	else
	{
		do_char_log(cn, 1, "Your companions will no longer follow you through passages.\n");
	}

	if (ch[cn].flags & (CF_PLAYER))
	{
		chlog(cn, "Set notell to %s", (ch[cn].flags & CF_GCTOME) ? "on" : "off");
	}
}

void do_trash(int cn)
{
	int in2, val;
	
	in2 = ch[cn].citem;
	
	if (!in2)
	{
		do_char_log(cn, 1, "Hold the item you'd like to dispose under your cursor first.\n");
		return;
	}
	if (in2 & 0x80000000)
	{
		val = in2 & 0x7fffffff;
		ch[cn].citem = 0;

		do_char_log(cn, 1, "You disposed of %d gold and %d silver.\n", val / 100, val % 100);
	}
	else
	{
		ch[cn].citem = 0;
		it[in2].used = USE_EMPTY;

		do_char_log(cn, 1, "You disposed of the %s.\n", it[in2].reference);
	}
	do_update_char(cn);
}

void do_swap_gear(int cn)
{
	int n, in, in2, flag=0;
	
	if (it[ch[cn].worn[WN_RHAND]].temp==IT_WP_RISINGPHO || it[ch[cn].worn[WN_RHAND]].orig_temp==IT_WP_RISINGPHO ||
		it[ch[cn].worn[WN_LHAND]].temp==IT_WP_RISINGPHO || it[ch[cn].worn[WN_LHAND]].orig_temp==IT_WP_RISINGPHO || 
		it[ch[cn].worn[WN_RHAND]].temp==IT_WB_RISINGPHO || it[ch[cn].worn[WN_RHAND]].orig_temp==IT_WB_RISINGPHO ||
		it[ch[cn].worn[WN_LHAND]].temp==IT_WB_RISINGPHO || it[ch[cn].worn[WN_LHAND]].orig_temp==IT_WB_RISINGPHO)
		flag = 1;
	
	for (n=0; n<12; n++)
	{
		if (n==WN_CHARM||n==WN_CHARM2) continue; // don't swap cards!
		//if (IS_SINBINDER(ch[cn].worn[n])) continue; // don't swap sinbinders!
		in = ch[cn].alt_worn[n];
		it[in].carried = cn;
		
		in2 = ch[cn].worn[n];
		it[in2].carried = cn;
		
		ch[cn].worn[n] = in;
		ch[cn].alt_worn[n] = in2;
	}
	
	if (it[ch[cn].worn[WN_RHAND]].temp!=IT_WP_RISINGPHO && it[ch[cn].worn[WN_RHAND]].orig_temp!=IT_WP_RISINGPHO && 
		it[ch[cn].worn[WN_LHAND]].temp!=IT_WP_RISINGPHO && it[ch[cn].worn[WN_LHAND]].orig_temp!=IT_WP_RISINGPHO && 
		it[ch[cn].worn[WN_RHAND]].temp!=IT_WB_RISINGPHO && it[ch[cn].worn[WN_RHAND]].orig_temp!=IT_WB_RISINGPHO && 
		it[ch[cn].worn[WN_LHAND]].temp!=IT_WB_RISINGPHO && it[ch[cn].worn[WN_LHAND]].orig_temp!=IT_WB_RISINGPHO &&
		flag && has_buff(cn, SK_IMMOLATE))
	{
		do_char_log(cn, 1, "Immolate no longer active.\n");
		remove_buff(cn, SK_IMMOLATE);
	}
	
	do_char_log(cn, 1, "You swapped your worn gear set.\n");
	do_update_char(cn);
}

void do_shout(int cn, char *text)
{
	char buf[256];
	int  n;

	if (!text)
	{
		do_char_log(cn, 0, "Shout. Yes. Shout it will be. But what do you want to shout?\n");
		return;
	}

	if (ch[cn].flags & CF_SHUTUP)
	{
		do_char_log(cn, 0, "You try to shout, but you only produce a croaking sound.\n");
		return;
	}

	if (ch[cn].flags & CF_INVISIBLE)
	{
		sprintf(buf, "Somebody shouts: \"%.200s\"\n", text);
	}
	else
	{
		sprintf(buf, "%.30s shouts: \"%.200s\"\n", ch[cn].name, text);
	}

	for (n = 1; n<MAXCHARS; n++)
	{
		if (((ch[n].flags & (CF_PLAYER | CF_USURP)) || ch[n].temp==15) && ch[n].used==USE_ACTIVE && ((!(ch[n].flags & CF_NOSHOUT) && !do_is_ignore(cn, n, 0)) || (ch[cn].flags & CF_GOD)))
		{
			do_char_log(n, 3, "%s", buf);
			if (ch[n].flags & CF_CCP)
			{
				ccp_shout(n, cn, text);
			}
		}
	}

	if (ch[cn].flags & (CF_PLAYER))
	{
		chlog(cn, "Shouts \"%s\"", text);
	}

}

void do_itell(int cn, char *text)
{
	int co;

	if (!text)
	{
		do_char_log(cn, 0, "Imp-Tell. Yes. imp-tell it will be. But what do you want to tell the other imps?\n");
		return;
	}

	if (ch[cn].flags & CF_SHUTUP)
	{
		do_char_log(cn, 0, "You try to imp-tell, but you only produce a croaking sound.\n");
		return;
	}

	if ((ch[cn].flags & CF_USURP) && IS_SANECHAR(co = ch[cn].data[97]))
	{
		do_imp_log(2, "%.30s (%.30s) imp-tells: \"%.170s\"\n", ch[cn].name, ch[co].name, text);
	}
	else
	{
		do_imp_log(2, "%.30s imp-tells: \"%.200s\"\n", ch[cn].name, text);
	}

	if (ch[cn].flags & (CF_PLAYER))
	{
		chlog(cn, "imp-tells \"%s\"", text);
	}

}

void do_stell(int cn, char *text)
{
	if (!text)
	{
		do_char_log(cn, 0, "Staff-Tell. Yes. staff-tell it will be. But what do you want to tell the other staff members?\n");
		return;
	}

	if (ch[cn].flags & CF_SHUTUP)
	{
		do_char_log(cn, 0, "You try to staff-tell, but you only produce a croaking sound.\n");
		return;
	}

	do_staff_log(2, "%.30s staff-tells: \"%.200s\"\n", ch[cn].name, text);

	if (ch[cn].flags & (CF_PLAYER))
	{
		chlog(cn, "staff-tells \"%s\"", text);
	}

}

void do_nostaff(int cn)
{
	ch[cn].flags ^= CF_NOSTAFF;

	if (ch[cn].flags & CF_NOSTAFF)
	{
		do_char_log(cn, 1, "You will no longer hear people using #stell.\n");
	}
	else
	{
		do_char_log(cn, 1, "You will hear people using #stell.\n");
	}

	if (ch[cn].flags & (CF_PLAYER))
	{
		chlog(cn, "Set nostaff to %s", (ch[cn].flags & CF_NOSTAFF) ? "on" : "off");
	}
}

/* Group tell */
void do_gtell(int cn, char *text)
{
	int n, co, found = 0;

	if (!text)
	{
		do_char_log(cn, 0, "Group-Tell. Yes. group-tell it will be. But what do you want to tell the other group members?\n");
		return;
	}

	if (ch[cn].flags & CF_SHUTUP)
	{
		do_char_log(cn, 0, "You try to group-tell, but you only produce a croaking sound.\n");
		return;
	}

	for (n = PCD_MINGROUP; n<=PCD_MAXGROUP; n++)
	{
		if ((co = ch[cn].data[n]))
		{
			if (!isgroup(co, cn))
			{
				ch[cn].data[n] = 0;     // throw out defunct group member
			}
			else
			{
				do_char_log(co, 6, "%s group-tells: \"%s\"\n", ch[cn].name, text);
				found = 1;
			}
		}
	}
	if (found)
	{
		do_char_log(cn, 6, "Told the group: \"%s\"\n", text);
		if (ch[cn].flags & (CF_PLAYER))
		{
			chlog(cn, "group-tells \"%s\"", text);
		}
	}
	else
	{
		do_char_log(cn, 0, "You don't have a group to talk to!\n");
	}
}

void do_help(int cn, char *topic)
{
	int pagenum = 1;
	if (strcmp(topic, "8")==0 && (ch[cn].flags & (CF_STAFF | CF_IMP | CF_USURP | CF_GOD | CF_GREATERGOD)))
	{
		if (ch[cn].flags & (CF_STAFF | CF_IMP | CF_USURP))
		{
			do_char_log(cn, 2, "Staff Commands:\n");
			do_char_log(cn, 2, " \n");
			//                 "!        .         .   |     .         .        !"
			do_char_log(cn, 2, "#announce <message>    broadcast IMPORTANT msg.\n");
			do_char_log(cn, 2, "#caution <text>        warn the population.\n");
			do_char_log(cn, 2, "#info <player>         identify player.\n");
			do_char_log(cn, 2, "#look <player>         look at player.\n");
			do_char_log(cn, 2, "#stell <text>          tell all staff members.\n");
			do_char_log(cn, 2, " \n");
		}

		if (ch[cn].flags & (CF_IMP | CF_USURP))
		{
			do_char_log(cn, 3, "Imp Commands:\n");
			do_char_log(cn, 3, " \n");
			//                 "!        .         .   |     .         .        !"
			do_char_log(cn, 3, "#addban <player>       add plr to ban list.\n");
			do_char_log(cn, 3, "#delban <lineno>       del plr from ban list.\n");
			do_char_log(cn, 3, "#enemy <NPC><char>     make NPC fight char.\n");
			do_char_log(cn, 3, "#enter                 fake enter the game.\n");
			do_char_log(cn, 3, "#exit                  return from #USURP.\n");
			do_char_log(cn, 3, "#force <char><text>    make him act.\n");
			//do_char_log(cn,3,"#gargoyle              turn self into a garg.\n");
			do_char_log(cn, 3, "#goto <char>           go to char.\n");
			do_char_log(cn, 3, "#goto <x> <y>          goto x,y.\n");
			do_char_log(cn, 3, "#goto n|e|s|w <nnn>    goto <nnn> in dir.\n");
			//do_char_log(cn,3,"#grolm                 turn self into a grolm.\n");
			do_char_log(cn, 3, "#itell <text>          tell all imps.\n");
			do_char_log(cn, 3, "#kick <player>         kick player out.\n");
			do_char_log(cn, 3, "#leave                 fake leave the game.\n");
			do_char_log(cn, 3, "#listban               show ban list.\n");
			do_char_log(cn, 3, "#look <player>         look at player.\n");
			do_char_log(cn, 3, "#luck <player> <val>   set players luck.\n");
			do_char_log(cn, 3, "#mark <player> <text>  mark a player with notes.\n");
			do_char_log(cn, 3, "#name <name> <N.Name>  change chars(npcs) names.\n");
			do_char_log(cn, 3, "#nodesc <player>       remove description.\n");
			do_char_log(cn, 3, "#nolist <player>       exempt from top 10.\n");
			do_char_log(cn, 3, "#nostaff               you won't hear #stell.\n");
			do_char_log(cn, 3, "#nowho <player>        not listed in who.\n");
			do_char_log(cn, 3, "#npclist <search>      display list of NPCs.\n");
			do_char_log(cn, 3, "#raise <player> <exp>  give player exps.\n");
			do_char_log(cn, 3, "#raisebs <player> <#>  give player bs points.\n");
			do_char_log(cn, 3, "#respawn <temp-id>     make npcs id respawn.\n");
			do_char_log(cn, 3, "#shutup <player>       make unable to talk.\n");
			do_char_log(cn, 3, "#slap <player>         slap in face.\n");
			do_char_log(cn, 3, "#sprite <player>       change a player's sprite.\n");
			//do_char_log(cn,3,"#summon <name> [<rank> [<which>]]\n");
			do_char_log(cn, 3, "#thrall <name> [<rank>] clone slave.\n");
			do_char_log(cn, 3, "#usurp <ID>            turn self into ID.\n");
			do_char_log(cn, 3, "#write <text>          make scrolls with text.\n");
			do_char_log(cn, 3, " \n");
		}
		if (ch[cn].flags & (CF_GOD))
		{
			do_char_log(cn, 3, "God Commands:\n");
			do_char_log(cn, 3, " \n");
			//                 "!        .         .   |     .         .        !"
			//do_char_log(cn,3,"#build <template>      build mode.\n");
			do_char_log(cn, 3, "#create <item templ>   creating items.\n");
			//do_char_log(cn,3,"#creator <player>      make player a Creator.\n");
			do_char_log(cn, 3, "#ggold <amount>        give money to a player.\n");
			do_char_log(cn, 3, "#god <player>          make player a God.\n");
			do_char_log(cn, 3, "#imp <player> <amnt>   make player an Imp.\n");
			do_char_log(cn, 3, "#mailpass <player>     send passwd to admin.\n");
			do_char_log(cn, 3, "#password <name>       change a plr's passwd.\n");
			do_char_log(cn, 3, "#perase <player>       total player erase.\n");
			do_char_log(cn, 3, "#pol <player>          make player POH leader.\n");
			//do_char_log(cn,3,"#race <player> <temp>  new race for a player(avoid).\n");
			do_char_log(cn, 3, "#send <playr> <targt>  teleport player to target.\n");
			do_char_log(cn, 3, "#staffer <player>      make a player staffer.\n");
			do_char_log(cn, 3, "#summon <name> [<rank> [<which>]]\n");
			do_char_log(cn, 3, "#tavern                log off quickly.\n");
			do_char_log(cn, 3, " \n");
		}
		if (ch[cn].flags & (CF_GREATERGOD))
		{
			do_char_log(cn, 3, "Greater God Commands:\n");
			do_char_log(cn, 3, " \n");
			//                 "!        .         .   |     .         .        !"
			do_char_log(cn, 3, "#build <template>      build mode.\n");
			do_char_log(cn, 3, "#creator <player>      make player a Creator.\n");
			do_char_log(cn, 3, "#greatergod <player>   make player a G-God.\n");
			do_char_log(cn, 3, "#lookinv <player>      look for items.\n");
			do_char_log(cn, 3, "#lookdepot <player>    look for items.\n");
			do_char_log(cn, 3, "#lookequip <player>    look for items.\n");
			do_char_log(cn, 3, "#steal <playr> <item>  Steal item from player.\n");

			do_char_log(cn, 3, " \n");
			do_char_log(cn, 3, "Current ticker is: %d\n", globs->ticker);
			do_char_log(cn, 3, " \n");
		}
	}
	else
	{
		if (strcmp(topic, "6")==0)
		{
			pagenum = 6;
			do_char_log(cn, 1, "The following commands are available (PAGE 6):\n");
			do_char_log(cn, 1, " \n");
			//                 "!        .         .   |     .         .        !"
			do_char_log(cn, 1, "#wave                  you'll wave.\n");
			do_char_log(cn, 1, "#weapon <type>         list weapon stats.\n");
			do_char_log(cn, 1, "#who                   see who's online.\n");
			do_char_log(cn, 1, "#zircon                list zircon rings.\n");
			if (ch[cn].kindred & (KIN_POH_LEADER))
			{
				do_char_log(cn, 1, " \n");
				//                 "!        .         .   |     .         .        !"
				do_char_log(cn, 1, "#poh <player>          add player to POH.\n");
				do_char_log(cn, 1, "#pol <player>          make plr POH leader.\n");
			}
		}
		if (strcmp(topic, "5")==0)
		{
			pagenum = 5;
			do_char_log(cn, 1, "The following commands are available (PAGE 5):\n");
			do_char_log(cn, 1, " \n");
			//                 "!        .         .   |     .         .        !"
			do_char_log(cn, 1, "#sort <order>          sort inventory.\n");
			do_char_log(cn, 1, "#sortdepot <ord/ch>    sort depot.\n");
			do_char_log(cn, 1, "#spear                 list spear stats.\n");
			do_char_log(cn, 1, "#spellignore           don't attack if spelled.\n");
			do_char_log(cn, 1, "#sphalerite            list sphalerite rings.\n"); 
			do_char_log(cn, 1, "#spinel                list spinel rings.\n");
			do_char_log(cn, 1, "#staff                 list staff stats.\n");
			do_char_log(cn, 1, "#swap                  swap with facing player.\n");
			do_char_log(cn, 1, "#sword                 list sword stats.\n");
			do_char_log(cn, 1, "#sysoff                disable all system msgs.\n");
			do_char_log(cn, 1, "#tarot                 list tarot cards.\n");
			do_char_log(cn, 1, "#tell <player> <text>  tells player text.\n");
			do_char_log(cn, 1, "#topaz                 list topaz rings.\n");
			do_char_log(cn, 1, "#trash                 delete item from cursor.\n");
			do_char_log(cn, 1, "#twohander             list twohander stats.\n");
		}
		else if (strcmp(topic, "4")==0)
		{
			pagenum = 4;
			do_char_log(cn, 1, "The following commands are available (PAGE 4):\n");
			do_char_log(cn, 1, " \n");
			//                 "!        .         .   |     .         .        !"
			do_char_log(cn, 1, "#quest <page>          list available quests.\n");
			do_char_log(cn, 1, "#rank                  show exp for next rank.\n");
			do_char_log(cn, 1, "#ranks                 show exp for all ranks.\n");
			do_char_log(cn, 1, "#ring <type>           list ring stats.\n");
			do_char_log(cn, 1, "#ruby                  list ruby rings.\n");
			do_char_log(cn, 1, "#sapphire              list sapphire rings.\n");
			if (IS_ANY_HARA(cn) || IS_SEYAN_DU(cn))
			{
				do_char_log(cn, 1, "#scbuff                display sc buff timers.\n");
				do_char_log(cn, 1, "#scmax                 list shadow maximums.\n");
			}
			if (ch[cn].flags & CF_SENSE)
				do_char_log(cn, 1, "#sense                 disable enemy spell msgs.\n");
			do_char_log(cn, 1, "#silence               you won't hear enemies.\n");
			do_char_log(cn, 1, "#seen <player>         when last seen here?.\n");
			do_char_log(cn, 1, "#shield                list shield stats.\n");
			do_char_log(cn, 1, "#shout <text>          to all players.\n");
			if (IS_SEYAN_DU(cn))
				do_char_log(cn, 1, "#shrine <page>         list unattained shrines.\n");
			do_char_log(cn, 1, "#skua                  leave purple/gorn/kwai.\n");
		}
		else if (strcmp(topic, "3")==0)
		{
			pagenum = 3;
			do_char_log(cn, 1, "The following commands are available (PAGE 3):\n");
			do_char_log(cn, 1, " \n");
			//                 "!        .         .   |     .         .        !"
			do_char_log(cn, 1, "#gold <amount>         get X gold coins.\n");
			do_char_log(cn, 1, "#greataxe              list greataxe stats.\n");
			do_char_log(cn, 1, "#group <player>        group with player.\n");
			do_char_log(cn, 1, "#gtell <message>       tell to your group.\n");
			do_char_log(cn, 1, "#ignore <player>       ignore that player.\n");
			do_char_log(cn, 1, "#iignore <player>      ignore normal talk too.\n");
			if (ch[cn].flags & CF_KNOWSPELL)
				do_char_log(cn, 1, "#knowspells        toggle knowledge skill.\n");
			do_char_log(cn, 1, "#lag <seconds>         lag control.\n");
			do_char_log(cn, 1, "#listskills <page>     list skill attributes.\n");
			do_char_log(cn, 1, "#max                   list character maximums.\n");
			do_char_log(cn, 1, "#noshout               you won't hear shouts.\n");
			do_char_log(cn, 1, "#notell                you won't hear tells.\n");
			do_char_log(cn, 1, "#opal                  list opal rings.\n");
			do_char_log(cn, 1, "#override              you always spell self.\n");
			do_char_log(cn, 1, "#poles <page>          lists unattained poles.\n");
		}
		else if (strcmp(topic, "2")==0)
		{
			pagenum = 2;
			do_char_log(cn, 1, "The following commands are available (PAGE 2):\n");
			do_char_log(cn, 1, " \n");
			//                 "!        .         .   |     .         .        !"
			do_char_log(cn, 1, "#buffs                 display buff timers.\n");
			do_char_log(cn, 1, "#chars                 list your chars and xp.\n");
			do_char_log(cn, 1, "#citrine               list citrine rings.\n");
			do_char_log(cn, 1, "#claw                  list claw stats.\n");
			do_char_log(cn, 1, "#contract              list current contract.\n");
			do_char_log(cn, 1, "#dagger                list dagger stats.\n");
			do_char_log(cn, 1, "#diamond               list diamond rings.\n");
			do_char_log(cn, 1, "#dualsword             list dualsword stats.\n");
			do_char_log(cn, 1, "#emerald               list emerald rings.\n");
			do_char_log(cn, 1, "#fightback             toggle auto-fightback.\n");
			do_char_log(cn, 1, "#follow <player|self>  you'll follow player.\n");
			do_char_log(cn, 1, "#garbage               delete item from cursor.\n");
			if (IS_ANY_HARA(cn) || IS_SEYAN_DU(cn))
			{
				do_char_log(cn, 1, "#gcbuff                display gc buff timers.\n");
				do_char_log(cn, 1, "#gcmax                 list ghostcomp maximums.\n");
			}
			do_char_log(cn, 1, "#gctome                gc travels with you.\n");
		}
		else
		{
			pagenum = 1;
			do_char_log(cn, 1, "The following commands are available (PAGE 1):\n");
			do_char_log(cn, 1, " \n");
			//                 "!        .         .   |     .         .        !"
			do_char_log(cn, 1, "#afk <message>         away from keyboard.\n");
			do_char_log(cn, 1, "#allow <player>        to access your grave.\n");
			do_char_log(cn, 1, "#allpoles <page>       lists all poles.\n");
			do_char_log(cn, 1, "#allquests <page>      lists all quests.\n");
			do_char_log(cn, 1, "#amethyst              list amethyst rings.\n");
			if (ch[cn].flags & CF_APPRAISE)
				do_char_log(cn, 1, "#appraise              toggle appraise skill.\n");
			do_char_log(cn, 1, "#aquamarine            list aquamarine rings.\n");
			if (B_SK(cn, SK_PROX) || IS_SEYAN_DU(cn))
				do_char_log(cn, 1, "#area                  toggle area skills.\n");
			do_char_log(cn, 1, "#armor                 list armor stats.\n");
			do_char_log(cn, 1, "#autoloot              automatic grave looting.\n");
			do_char_log(cn, 1, "#axe                   list axe stats.\n");
			do_char_log(cn, 1, "#belt                  list belt stats.\n");
			do_char_log(cn, 1, "#beryl                 list beryl rings.\n");
			do_char_log(cn, 1, "#bow                   you'll bow.\n");
			do_char_log(cn, 1, "#bs                    display BS points.\n");
		}
		do_char_log(cn, 1, " \n");
	}
	
	// Seems that the game supports 22 lines of text at once... a bit yuck but...
	// 2 header
	// 15 body
	// 5 footer

	if (strcmp(topic, "8") || !(ch[cn].flags & (CF_STAFF | CF_IMP | CF_USURP | CF_GOD | CF_GREATERGOD)))	// Eats 4 lines total, 5 if staff
	{
		//                 "!        .         .         .         .        !"
		do_char_log(cn, 1, "You can replace the '#' with '/'. Some commands are toggles, use again to turn off the effect.\n"); // eats 2 lines
		do_char_log(cn, 2, "Showing page %d of 6. #help <x> to swap page.\n", pagenum);
		if (ch[cn].flags & (CF_STAFF | CF_IMP | CF_USURP | CF_GOD | CF_GREATERGOD))
			do_char_log(cn, 3, "Use /help 8 to display staff-specific commands.\n");
		do_char_log(cn, 1, " \n");
	}
}

// -------- THE LAST GATE - Adding new commands here! -------- //

void do_listmax(int cn)
{
	int n;
	do_char_log(cn, 1, "Now listing skill maximums for your character:\n");
	do_char_log(cn, 1, " \n");
	//
	for (n=0;n<5;n++)
	{
		if (ch[cn].attrib[n][1])
		{
			do_char_log(cn, (B_AT(cn, n)==ch[cn].attrib[n][2])?7:5, 
			"%20s  %3d  %3d (+%2d)\n", 
			at_name[n], B_AT(cn, n), ch[cn].attrib[n][2], ch[cn].attrib[n][1]);
		}
		else
		{
			do_char_log(cn, (B_AT(cn, n)==ch[cn].attrib[n][2])?2:1, 
			"%20s  %3d  %3d\n", 
			at_name[n], B_AT(cn, n), ch[cn].attrib[n][2]);
		}
	}
	//
	do_char_log(cn, (ch[cn].hp[0]==ch[cn].hp[2])?2:1, 
		"%20s  %3d  %3d\n", 
		"Hitpoints", ch[cn].hp[0], ch[cn].hp[2]);
	do_char_log(cn, (ch[cn].mana[0]==ch[cn].mana[2])?2:1, 
		"%20s  %3d  %3d\n", 
		"Mana",      ch[cn].mana[0], ch[cn].mana[2]);
	//
	for (n=0;n<50;n++)
	{
		if (ch[cn].skill[n][2]) 
		{
			if (ch[cn].skill[n][1]) 
			{
				do_char_log(cn, (B_SK(cn, n)==ch[cn].skill[n][2])?7:5, 
				"%20s  %3d  %3d (+%2d)\n", 
				skilltab[n].name, B_SK(cn, n), ch[cn].skill[n][2], ch[cn].skill[n][1]);
			}
			else
			{
				do_char_log(cn, (B_SK(cn, n)==ch[cn].skill[n][2])?2:1, 
				"%20s  %3d  %3d\n", 
				skilltab[n].name, B_SK(cn, n), ch[cn].skill[n][2]);
			}
		}
	}
	do_char_log(cn, 1, " \n");
}

void do_listgcmax(int cn, int shadow)
{
	int n, co=0, archbonus=0, archtmp=0, m = PCD_COMPANION;
	
	if (shadow) m = PCD_SHADOWCOPY;
	
	if (co = ch[cn].data[m])
	{
		if (!IS_SANECHAR(co) || ch[co].data[CHD_MASTER]!=cn || (ch[co].flags & CF_BODY) || ch[co].used==USE_EMPTY)
		{
			co = 0;
		}
	}
	if (!co)
	{
		do_char_log(cn, 0, "You must summon a new companion first.\n");
		return;
	}
	if (B_SK(cn, SK_GCMASTERY)) 
		archbonus 	= M_SK(cn, SK_GCMASTERY);
	
	//                 "!        .         .   |     .         .        !"
	do_char_log(cn, 1, "Now listing skill maximums for your ghost:\n");
	do_char_log(cn, 1, " \n");
	//
	for (n=0;n<5;n++)
	{
		do_char_log(cn, (B_AT(co, n)>=ch[co].attrib[n][2]+max(0,archbonus-n)/10)?2:1,
		"%20s  %3d  %3d\n", 
		at_name[n], B_AT(co, n), ch[co].attrib[n][2]+max(0,archbonus-n)/10);
	}
	//
	do_char_log(cn, (ch[co].hp[0]>=ch[co].hp[2])?2:1, 
		"%20s  %3d  %3d\n", 
		"Hitpoints", ch[co].hp[0], ch[co].hp[2]);
	do_char_log(cn, (ch[co].mana[0]>=ch[co].mana[2])?2:1, 
		"%20s  %3d  %3d\n", 
		"Mana",      ch[co].mana[0], ch[co].mana[2]);
	//
	for (n=0;n<50;n++)
	{
		if (ch[co].skill[n][2]) 
		{
			if (n==0)
			{
				do_char_log(cn, (B_SK(co, n)>=ch[co].skill[n][2]+archbonus/5)?2:1, 
					"%20s  %3d  %3d\n", 
					skilltab[n].name, B_SK(co, n), ch[co].skill[n][2]+archbonus/5);
			}
			else
			{
				do_char_log(cn, (B_SK(co, n)>=ch[co].skill[n][2]+(archbonus-archtmp)/10)?2:1, 
					"%20s  %3d  %3d\n", 
					skilltab[n].name, B_SK(co, n), ch[co].skill[n][2]+(archbonus-archtmp)/10);
				archtmp++;
			}
		}
	}
	do_char_log(cn, 1, " \n");
}

void do_listskills(int cn, char *topic)
{
	int pagenum = 0;
	if (strcmp(topic, "4")==0)
	{
							pagenum = 4;
							do_char_log(cn, 1, "Now listing skill attributes (PAGE 4):\n");
							do_char_log(cn, 1, " \n");
							//                 "!        .         .   |     .         .        !"
							do_char_log(cn, 1, "Aria                   BRV + AGL + AGL\n");
							do_char_log(cn, 5, "Companion Mastery      BRV + WIL + WIL\n");
							do_char_log(cn, 1, "Economize              WIL + WIL + WIL\n");
							do_char_log(cn, 5, "Finesse                BRV + BRV + AGL\n");
if (T_SKAL_SK(cn, 9))		do_char_log(cn, 7, "Gear Mastery         (BRV+STR)/2 + AGL + AGL\n");
else						do_char_log(cn, 1, "Gear Mastery           BRV + AGL + STR\n");
							do_char_log(cn, 5, "Immunity               WIL + AGL + STR\n");
							do_char_log(cn, 1, "Metabolism             BRV + WIL + INT\n");
							do_char_log(cn, 5, "Perception             INT + INT + AGL\n");
							do_char_log(cn, 1, "Precision              BRV + BRV + INT\n");
							do_char_log(cn, 5, "Proximity              BRV + INT + INT\n");
							do_char_log(cn, 1, "Resistance             BRV + WIL + STR\n");
							do_char_log(cn, 5, "Safeguard              BRV + STR + STR\n");
							do_char_log(cn, 1, "Stealth                INT + AGL + AGL\n");
if (T_SKAL_SK(cn, 9))		do_char_log(cn, 3, "Surround Hit         (BRV+STR)/2 + AGL + AGL\n");
else						do_char_log(cn, 5, "Surround Hit           AGL + STR + STR\n");
							do_char_log(cn, 1, "Zephyr                 BRV + AGL + STR\n");
	}
	else if (strcmp(topic, "3")==0)
	{
							pagenum = 3;
							do_char_log(cn, 1, "Now listing skill attributes (PAGE 3):\n");
							do_char_log(cn, 1, " \n");
							//                 "!        .         .   |     .         .        !"
if (T_SUMM_SK(cn, 7))		do_char_log(cn, 7, "Blast                (BRV+INT)/2 + WIL + WIL\n");
else if (T_ARHR_SK(cn, 9))	do_char_log(cn, 7, "Blast                (BRV+WIL)/2 + INT + INT\n");
else						do_char_log(cn, 1, "Blast                  BRV + INT + INT\n");
if (T_SUMM_SK(cn, 7))		do_char_log(cn, 3, "Bless                (BRV+INT)/2 + WIL + WIL\n");
else if (T_ARHR_SK(cn, 9))	do_char_log(cn, 3, "Bless                (BRV+WIL)/2 + INT + INT\n");
else if (IS_LYCANTH(cn))	do_char_log(cn, 3, "Bless                  BRV + WIL + AGL\n");
else						do_char_log(cn, 5, "Bless                  BRV + WIL + WIL\n");
if (T_SUMM_SK(cn, 7))		do_char_log(cn, 7, "Curse                (BRV+INT)/2 + WIL + WIL\n");
else if (T_ARHR_SK(cn, 9))	do_char_log(cn, 7, "Curse                (BRV+WIL)/2 + INT + INT\n");
else						do_char_log(cn, 1, "Curse                  BRV + INT + INT\n");
if (T_SUMM_SK(cn, 7))		do_char_log(cn, 3, "Dispel               (BRV+INT)/2 + WIL + WIL\n");
else if (T_ARHR_SK(cn, 9))	do_char_log(cn, 3, "Dispel               (BRV+WIL)/2 + INT + INT\n");
else						do_char_log(cn, 5, "Dispel                 BRV + WIL + INT\n");
if (T_SUMM_SK(cn, 7))		do_char_log(cn, 7, "Enhance              (BRV+INT)/2 + WIL + WIL\n");
else if (T_ARHR_SK(cn, 9))	do_char_log(cn, 7, "Enhance              (BRV+WIL)/2 + INT + INT\n");
else if (IS_LYCANTH(cn))	do_char_log(cn, 7, "Enhance                BRV + WIL + AGL\n");
else						do_char_log(cn, 1, "Enhance                BRV + WIL + WIL\n");
if (T_SUMM_SK(cn, 7))		do_char_log(cn, 3, "Ghost Companion      (BRV+INT)/2 + WIL + WIL\n");
else if (T_ARHR_SK(cn, 9))	do_char_log(cn, 3, "Ghost Companion      (BRV+WIL)/2 + INT + INT\n");
else						do_char_log(cn, 5, "Ghost Companion        BRV + WIL + WIL\n");
if (T_SUMM_SK(cn, 7))		do_char_log(cn, 7, "Haste                (BRV+INT)/2 + WIL + WIL\n");
else if (T_ARHR_SK(cn, 9))	do_char_log(cn, 7, "Haste                (BRV+WIL)/2 + INT + INT\n");
else if (IS_LYCANTH(cn))	do_char_log(cn, 7, "Haste                  BRV + WIL + AGL\n");
else						do_char_log(cn, 1, "Haste                  BRV + WIL + AGL\n");
if (T_SUMM_SK(cn, 7))		do_char_log(cn, 3, "Heal                 (BRV+INT)/2 + WIL + WIL\n");
else if (T_ARHR_SK(cn, 9))	do_char_log(cn, 3, "Heal                 (BRV+WIL)/2 + INT + INT\n");
else						do_char_log(cn, 5, "Heal                   BRV + WIL + STR\n");
							do_char_log(cn, 1, "Lethargy               BRV + WIL + INT\n");
if (T_SUMM_SK(cn, 7))		do_char_log(cn, 3, "Magic Shield         (BRV+INT)/2 + WIL + WIL\n");
else if (T_ARHR_SK(cn, 9))	do_char_log(cn, 3, "Magic Shield         (BRV+WIL)/2 + INT + INT\n");
else if (IS_LYCANTH(cn))	do_char_log(cn, 3, "Magic Shield           BRV + WIL + AGL\n");
else						do_char_log(cn, 5, "Magic Shield           BRV + WIL + WIL\n");
if (T_SUMM_SK(cn, 7))		do_char_log(cn, 7, "Poison               (BRV+INT)/2 + WIL + WIL\n");
else if (T_ARHR_SK(cn, 9))	do_char_log(cn, 7, "Poison               (BRV+WIL)/2 + INT + INT\n");
else						do_char_log(cn, 1, "Poison                 BRV + INT + INT\n");
if (T_SUMM_SK(cn, 7))		do_char_log(cn, 3, "Protect              (BRV+INT)/2 + WIL + WIL\n");
else if (T_ARHR_SK(cn, 9))	do_char_log(cn, 3, "Protect              (BRV+WIL)/2 + INT + INT\n");
else if (IS_LYCANTH(cn))	do_char_log(cn, 3, "Protect                BRV + WIL + AGL\n");
else						do_char_log(cn, 5, "Protect                BRV + WIL + WIL\n");
if (T_SUMM_SK(cn, 7))		do_char_log(cn, 7, "Pulse                (BRV+INT)/2 + WIL + WIL\n");
else if (T_ARHR_SK(cn, 9))	do_char_log(cn, 7, "Pulse                (BRV+WIL)/2 + INT + INT\n");
else						do_char_log(cn, 1, "Pulse                  BRV + INT + INT\n");
if (T_SUMM_SK(cn, 7))		do_char_log(cn, 3, "Shadow Copy          (BRV+INT)/2 + WIL + WIL\n");
else if (T_ARHR_SK(cn, 9))	do_char_log(cn, 3, "Shadow Copy          (BRV+WIL)/2 + INT + INT\n");
else						do_char_log(cn, 5, "Shadow Copy            BRV + WIL + WIL\n");
if (T_SUMM_SK(cn, 7))		do_char_log(cn, 7, "Slow                 (BRV+INT)/2 + WIL + WIL\n");
else if (T_ARHR_SK(cn, 9))	do_char_log(cn, 7, "Slow                 (BRV+WIL)/2 + INT + INT\n");
else						do_char_log(cn, 1, "Slow                   BRV + INT + INT\n");
	}
	else if (strcmp(topic, "2")==0)
	{
							pagenum = 2;
							do_char_log(cn, 1, "Now listing skill attributes (PAGE 2):\n");
							do_char_log(cn, 1, " \n");
							//                 "!        .         .   |     .         .        !"
if (T_SKAL_SK(cn, 9))		do_char_log(cn, 7, "Blind                (BRV+STR)/2 + AGL + AGL\n");
else						do_char_log(cn, 1, "Blind                  BRV + INT + AGL\n");
if (T_SKAL_SK(cn, 9))		do_char_log(cn, 3, "Cleave               (BRV+STR)/2 + AGL + AGL\n");
else						do_char_log(cn, 5, "Cleave                 AGL + STR + STR\n");
if (T_SKAL_SK(cn, 9))		do_char_log(cn, 7, "Leap                 (BRV+STR)/2 + AGL + AGL\n");
else						do_char_log(cn, 1, "Leap                   BRV + AGL + AGL\n");
							do_char_log(cn, 5, "Rage / Calm            BRV + INT + STR\n");
							do_char_log(cn, 1, "Repair                 INT + AGL + STR\n");
if (T_SKAL_SK(cn, 9))		do_char_log(cn, 3, "Taunt                (BRV+STR)/2 + AGL + AGL\n");
else						do_char_log(cn, 5, "Taunt                  BRV + STR + STR\n");
							do_char_log(cn, 1, "Warcry                 BRV + STR + STR\n");
if (T_SKAL_SK(cn, 9))		do_char_log(cn, 3, "Weaken               (BRV+STR)/2 + AGL + AGL\n");
else						do_char_log(cn, 5, "Weaken                 BRV + AGL + AGL\n");
	}
	else
	{
							pagenum = 1;
							do_char_log(cn, 1, "Now listing skill attributes (PAGE 1):\n");
							do_char_log(cn, 1, " \n");
							//                 "!        .         .   |     .         .        !"
							do_char_log(cn, 1, "Regenerate             STR + STR + STR\n");
							do_char_log(cn, 5, "Rest                   AGL + AGL + AGL\n");
							do_char_log(cn, 1, "Meditate               INT + INT + INT\n");
if (get_gear(cn, 3494))		do_char_log(cn, 3, "Hand to Hand           BRV + BRV + BRV\n"); 		// IT_WB_LIONSPAWS
else if (T_SKAL_SK(cn, 9))	do_char_log(cn, 3, "Hand to Hand         (BRV+STR)/2 + AGL + AGL\n");
else if (T_BRAV_SK(cn, 9))	do_char_log(cn, 3, "Hand to Hand         (AGL+STR)/2 + BRV + BRV\n");
else						do_char_log(cn, 5, "Hand to Hand           BRV + AGL + STR\n");
							do_char_log(cn, 1, "Tactics                BRV + WIL + INT\n");
if (get_gear(cn, 3501))		do_char_log(cn, 3, "Axe                    BRV + AGL + AGL\n"); 		// IT_WB_GULLOXI
else if (T_SKAL_SK(cn, 9))	do_char_log(cn, 3, "Axe                  (BRV+STR)/2 + AGL + AGL\n");
else if (T_BRAV_SK(cn, 9))	do_char_log(cn, 3, "Axe                  (AGL+STR)/2 + BRV + BRV\n");
else						do_char_log(cn, 5, "Axe                    AGL + STR + STR\n");
if (T_SKAL_SK(cn, 9))		do_char_log(cn, 7, "Dagger               (BRV+STR)/2 + AGL + AGL\n");
else if (T_BRAV_SK(cn, 9))	do_char_log(cn, 7, "Dagger               (AGL+STR)/2 + BRV + BRV\n");
else						do_char_log(cn, 1, "Dagger                 WIL + WIL + AGL\n");
							do_char_log(cn, 5, "Dual Wield             BRV + AGL + STR\n");
if (T_SKAL_SK(cn, 9))		do_char_log(cn, 7, "Shield               (BRV+STR)/2 + AGL + AGL\n");
else						do_char_log(cn, 1, "Shield                 BRV + WIL + STR\n");
if (T_SKAL_SK(cn, 9))		do_char_log(cn, 3, "Staff                (BRV+STR)/2 + AGL + AGL\n");
else if (T_BRAV_SK(cn, 9))	do_char_log(cn, 3, "Staff                (AGL+STR)/2 + BRV + BRV\n");
else						do_char_log(cn, 5, "Staff                  INT + INT + STR\n");
if (get_gear(cn, 3477))		do_char_log(cn, 7, "Sword                  BRV + STR + STR\n"); 		// IT_WB_BARBSWORD
else if (T_SKAL_SK(cn, 9))	do_char_log(cn, 7, "Sword                (BRV+STR)/2 + AGL + AGL\n");
else if (T_BRAV_SK(cn, 9))	do_char_log(cn, 7, "Sword                (AGL+STR)/2 + BRV + BRV\n");
else						do_char_log(cn, 1, "Sword                  BRV + AGL + STR\n");
if (T_SKAL_SK(cn, 9))		do_char_log(cn, 3, "Two-Handed           (BRV+STR)/2 + AGL + AGL\n");
else if (T_BRAV_SK(cn, 9))	do_char_log(cn, 3, "Two-Handed           (AGL+STR)/2 + BRV + BRV\n");
else						do_char_log(cn, 5, "Two-Handed             AGL + AGL + STR\n");
	}
	do_char_log(cn, 1, " \n");
	do_char_log(cn, 2, "Showing page %d of 4. #listskills <x> to swap.\n", pagenum);
	do_char_log(cn, 1, " \n");
}

void do_listweapons(int cn, char *topic)
{
	if (strcmp(topic, "0")==0 
		|| strcmp(topic, "CLAW")==0 || strcmp(topic, "CLAWS")==0
		|| strcmp(topic, "claw")==0 || strcmp(topic, "claws")==0)
	{
		do_char_log(cn, 1, "Now listing CLAW weapons:\n");
		do_char_log(cn, 1, " \n");
		//                 "!        .         .         .         .        !"
		do_char_log(cn, 1, "   Weapon   |   Requirements: |   Gives:    \n");
		do_char_log(cn, 1, "    Tier    | AGL | STR | Skl | WV | TopDmg \n");
		do_char_log(cn, 1, "------------+-----+-----+-----+----+--------\n");
		do_char_log(cn, 1, "Bronze      |   1 |   1 |   1 | 10 |     6  \n");
		do_char_log(cn, 1, "Steel       |  15 |  12 |  16 | 20 |    12  \n");
		do_char_log(cn, 1, "Gold        |  25 |  18 |  32 | 30 |    18  \n");
		do_char_log(cn, 1, "Emerald     |  40 |  28 |  48 | 40 |    24  \n");
		do_char_log(cn, 1, "Crystal     |  60 |  42 |  64 | 50 |    30  \n");
		do_char_log(cn, 1, "Titanium    |  85 |  60 |  80 | 60 |    36  \n");
		do_char_log(cn, 1, " \n");
		do_char_log(cn, 2, "* Claws will use both hand slots.\n");
		do_char_log(cn, 2, "* Claws have a critical hit chance of 2%%.\n");
		do_char_log(cn, 1, " \n");
	}
	else if (strcmp(topic, "1")==0 
		|| strcmp(topic, "DAGGER")==0 || strcmp(topic, "DAGGERS")==0
		|| strcmp(topic, "dagger")==0 || strcmp(topic, "daggers")==0)
	{
		do_char_log(cn, 1, "Now listing DAGGER weapons:\n");
		do_char_log(cn, 1, " \n");
		//                 "!        .         .         .         .        !"
		do_char_log(cn, 1, "   Weapon   |   Requirements:   |    Gives:   \n");
		do_char_log(cn, 1, "    Tier    | WIL | AGL | Skill |  WV | Parry \n");
		do_char_log(cn, 1, "------------+-----+-----+-------+-----+-------\n");
		do_char_log(cn, 1, "Bronze      |   1 |   1 |     1 |   6 |    2  \n");
		do_char_log(cn, 1, "Steel       |  12 |  12 |    10 |  12 |    4  \n");
		do_char_log(cn, 1, "Gold        |  18 |  14 |    20 |  18 |    6  \n");
		do_char_log(cn, 1, "Emerald     |  30 |  16 |    30 |  24 |    8  \n");
		do_char_log(cn, 1, "Crystal     |  48 |  20 |    40 |  30 |   10  \n");
		do_char_log(cn, 1, "Titanium    |  72 |  24 |    50 |  36 |   12  \n");
		do_char_log(cn, 1, " \n");
		do_char_log(cn, 2, "* Daggers can be used in the off-hand slot.\n");
		do_char_log(cn, 0, "* WV is reduced by 50%% in the off-hand slot.\n");
		do_char_log(cn, 2, "* Daggers have a critical hit chance of 3%%.\n");
		do_char_log(cn, 1, " \n");
	}
	else if (strcmp(topic, "2")==0 
		|| strcmp(topic, "STAFF")==0 || strcmp(topic, "STAFFS")==0
		|| strcmp(topic, "staff")==0 || strcmp(topic, "staffs")==0)
	{
		do_char_log(cn, 1, "Now listing STAFF weapons:\n");
		do_char_log(cn, 1, " \n");
		//                 "!        .         .         .         .        !"
		do_char_log(cn, 1, "   Weapon   |   Requirements:   |   Gives:  \n");
		do_char_log(cn, 1, "    Tier    | INT | STR | Skill |  WV | INT \n");
		do_char_log(cn, 1, "------------+-----+-----+-------+-----+-----\n");
		do_char_log(cn, 1, "Bronze      |   1 |   1 |     1 |   4 |   1 \n");
		do_char_log(cn, 1, "Steel       |  14 |  12 |    10 |   8 |   2 \n");
		do_char_log(cn, 1, "Gold        |  21 |  14 |    20 |  12 |   3 \n");
		do_char_log(cn, 1, "Emerald     |  35 |  16 |    30 |  16 |   4 \n");
		do_char_log(cn, 1, "Crystal     |  56 |  20 |    40 |  20 |   5 \n");
		do_char_log(cn, 1, "Titanium    |  84 |  24 |    50 |  24 |   6 \n");
		do_char_log(cn, 1, " \n");
	}
	else if (strcmp(topic, "3")==0 
		|| strcmp(topic, "SPEAR")==0 || strcmp(topic, "SPEARS")==0
		|| strcmp(topic, "spear")==0 || strcmp(topic, "spears")==0)
	{
		do_char_log(cn, 1, "Now listing SPEAR weapons:\n");
		do_char_log(cn, 1, " \n");
		//                 "!        .         .         .         .        !"
		do_char_log(cn, 1, "  Weapon  |  Requirements:  |      Gives:      \n");
		do_char_log(cn, 1, "   Tier   | WIL | STR | Skl | WV | AV | Ht&WIL \n");
		do_char_log(cn, 1, "----------+-----+-----+-----+----+----+--------\n");
		do_char_log(cn, 1, "Steel     |  16 |  12 |   8 | 20 |  2 |      4 \n");
		do_char_log(cn, 1, "Gold      |  22 |  14 |  16 | 30 |  3 |      6 \n");
		do_char_log(cn, 1, "Emerald   |  34 |  16 |  24 | 40 |  4 |      8 \n");
		do_char_log(cn, 1, "Crystal   |  52 |  20 |  32 | 50 |  5 |     10 \n");
		do_char_log(cn, 1, "Titanium  |  76 |  24 |  40 | 60 |  6 |     12 \n");
		do_char_log(cn, 1, " \n");
		do_char_log(cn, 2, "* Spears will use both hand slots.\n");
		do_char_log(cn, 0, "* Requires both Dagger and Staff skills.\n");
		do_char_log(cn, 1, "* Fighting uses the higher of either skill.\n");
		do_char_log(cn, 1, "* Grants Surround Hit with the lower skill.\n");
		do_char_log(cn, 2, "* Spears have a critical hit chance of 2%%.\n");
		do_char_log(cn, 1, " \n");
	}
	else if (strcmp(topic, "4")==0 
		|| strcmp(topic, "SHIELD")==0 || strcmp(topic, "SHIELDS")==0
		|| strcmp(topic, "shield")==0 || strcmp(topic, "shields")==0)
	{
		do_char_log(cn, 1, "Now listing SHIELD items:\n");
		do_char_log(cn, 1, " \n");
		//                 "!        .         .         .         .        !"
		do_char_log(cn, 1, "   Weapon   |  Requires:  | Gives: \n");
		do_char_log(cn, 1, "    Tier    | BRV | Skill |   AV   \n");
		do_char_log(cn, 1, "------------+-----+-------+--------\n");
		do_char_log(cn, 1, "Bronze      |   1 |     1 |    4   \n");
		do_char_log(cn, 1, "Steel       |  12 |    12 |    8   \n");
		do_char_log(cn, 1, "Gold        |  18 |    24 |   12   \n");
		do_char_log(cn, 1, "Emerald     |  28 |    36 |   16   \n");
		do_char_log(cn, 1, "Crystal     |  42 |    48 |   20   \n");
		do_char_log(cn, 1, "Titanium    |  60 |    60 |   24   \n");
		do_char_log(cn, 1, " \n");
		do_char_log(cn, 2, "* Shields use the off-hand slot.\n");
		do_char_log(cn, 1, " \n");
	}
	else if (strcmp(topic, "5")==0 
		|| strcmp(topic, "SWORD")==0 || strcmp(topic, "SWORDS")==0
		|| strcmp(topic, "sword")==0 || strcmp(topic, "swords")==0)
	{
		do_char_log(cn, 1, "Now listing SWORD weapons:\n");
		do_char_log(cn, 1, " \n");
		//                 "!        .         .         .         .        !"
		do_char_log(cn, 1, "   Weapon   |   Requirements:   | Gives: \n");
		do_char_log(cn, 1, "    Tier    | AGL | STR | Skill |   WV   \n");
		do_char_log(cn, 1, "------------+-----+-----+-------+--------\n");
		do_char_log(cn, 1, "Bronze      |   1 |   1 |     1 |    8   \n");
		do_char_log(cn, 1, "Steel       |  16 |  12 |    12 |   16   \n");
		do_char_log(cn, 1, "Gold        |  22 |  16 |    24 |   24   \n");
		do_char_log(cn, 1, "Emerald     |  30 |  22 |    36 |   32   \n");
		do_char_log(cn, 1, "Crystal     |  40 |  30 |    48 |   40   \n");
		do_char_log(cn, 1, "Titanium    |  52 |  40 |    60 |   48   \n");
		do_char_log(cn, 1, " \n");
		do_char_log(cn, 2, "* Swords have a critical hit chance of 2%%.\n");
		do_char_log(cn, 1, " \n");
	}
	else if (strcmp(topic, "6")==0 
		|| strcmp(topic, "DUALSWORD")==0 || strcmp(topic, "DUALSWORDS")==0
		|| strcmp(topic, "dualsword")==0 || strcmp(topic, "dualswords")==0)
	{
		do_char_log(cn, 1, "Now listing DUALSWORD weapons:\n");
		do_char_log(cn, 1, " \n");
		//                 "!        .         .         .         .        !"
		do_char_log(cn, 1, "   Weapon   |   Requirements:   | Gives: \n");
		do_char_log(cn, 1, "    Tier    | AGL | STR | Skill |   WV   \n");
		do_char_log(cn, 1, "------------+-----+-----+-------+--------\n");
		do_char_log(cn, 1, "Bronze      |   1 |   1 |     1 |    6   \n");
		do_char_log(cn, 1, "Steel       |  12 |  16 |    15 |   12   \n");
		do_char_log(cn, 1, "Gold        |  16 |  22 |    30 |   18   \n");
		do_char_log(cn, 1, "Emerald     |  22 |  30 |    45 |   24   \n");
		do_char_log(cn, 1, "Crystal     |  30 |  40 |    60 |   30   \n");
		do_char_log(cn, 1, "Titanium    |  40 |  52 |    75 |   36   \n");
		do_char_log(cn, 1, " \n");
		do_char_log(cn, 2, "* Dual-swords use the off-hand slot.\n");
		do_char_log(cn, 1, " \n");
	}
	else if (strcmp(topic, "7")==0 
		|| strcmp(topic, "AXE")==0 || strcmp(topic, "AXES")==0
		|| strcmp(topic, "axe")==0 || strcmp(topic, "axes")==0)
	{
		do_char_log(cn, 1, "Now listing AXE weapons:\n");
		do_char_log(cn, 1, " \n");
		//                 "!        .         .         .         .        !"
		do_char_log(cn, 1, "   Weapon   |   Requirements:   | Gives: \n");
		do_char_log(cn, 1, "    Tier    | AGL | STR | Skill |   WV   \n");
		do_char_log(cn, 1, "------------+-----+-----+-------+--------\n");
		do_char_log(cn, 1, "Bronze      |   1 |   1 |     1 |   10   \n");
		do_char_log(cn, 1, "Steel       |  12 |  14 |    16 |   20   \n");
		do_char_log(cn, 1, "Gold        |  18 |  22 |    32 |   30   \n");
		do_char_log(cn, 1, "Emerald     |  28 |  34 |    48 |   40   \n");
		do_char_log(cn, 1, "Crystal     |  42 |  50 |    64 |   50   \n");
		do_char_log(cn, 1, "Titanium    |  60 |  74 |    80 |   60   \n");
		do_char_log(cn, 1, " \n");
	}
	else if (strcmp(topic, "8")==0 
		|| strcmp(topic, "TWOHANDER")==0 || strcmp(topic, "TWOHANDERS")==0
		|| strcmp(topic, "twohander")==0 || strcmp(topic, "twohanders")==0)
	{
		do_char_log(cn, 1, "Now listing TWOHANDER weapons:\n");
		do_char_log(cn, 1, " \n");
		//                 "!        .         .         .         .        !"
		do_char_log(cn, 1, "   Weapon   |   Requirements:   |    Gives:   \n");
		do_char_log(cn, 1, "    Tier    | AGL | STR | Skill |  WV | Multi \n");
		do_char_log(cn, 1, "------------+-----+-----+-------+-----+-------\n");
		do_char_log(cn, 1, "Bronze      |   1 |   1 |     1 |  12 |    5  \n");
		do_char_log(cn, 1, "Steel       |  16 |  12 |    18 |  24 |   10  \n");
		do_char_log(cn, 1, "Gold        |  26 |  20 |    36 |  36 |   15  \n");
		do_char_log(cn, 1, "Emerald     |  40 |  32 |    54 |  48 |   20  \n");
		do_char_log(cn, 1, "Crystal     |  58 |  48 |    72 |  60 |   25  \n");
		do_char_log(cn, 1, "Titanium    |  80 |  68 |    90 |  72 |   30  \n");
		do_char_log(cn, 1, " \n");
		do_char_log(cn, 2, "* Twohanders will use both hand slots.\n");
		do_char_log(cn, 2, "* Twohanders have a critical hit chance of 2%%.\n");
		do_char_log(cn, 1, " \n");
	}
	else if (strcmp(topic, "9")==0 
		|| strcmp(topic, "GREATAXE")==0 || strcmp(topic, "GREATAXES")==0
		|| strcmp(topic, "greataxe")==0 || strcmp(topic, "greataxes")==0)
	{
		do_char_log(cn, 1, "Now listing GREATAXE weapons:\n");
		do_char_log(cn, 1, " \n");
		//                 "!        .         .         .         .        !"
		do_char_log(cn, 1, "   Weapon   |   Requirements:   |   Gives:  \n");
		do_char_log(cn, 1, "    Tier    | AGL | STR | Skill |  WV |  AV \n");
		do_char_log(cn, 1, "------------+-----+-----+-------+-----+-----\n");
		do_char_log(cn, 1, "Steel       |  12 |  14 |    15 |  28 |   2 \n");
		do_char_log(cn, 1, "Gold        |  20 |  24 |    30 |  42 |   3 \n");
		do_char_log(cn, 1, "Emerald     |  32 |  40 |    45 |  56 |   4 \n");
		do_char_log(cn, 1, "Crystal     |  48 |  62 |    60 |  70 |   5 \n");
		do_char_log(cn, 1, "Titanium    |  68 |  90 |    75 |  84 |   6 \n");
		do_char_log(cn, 1, " \n");
		do_char_log(cn, 2, "* Greataxes will use both hand slots.\n");
		do_char_log(cn, 0, "* Requires both Axe and Two-Handed skills.\n");
		do_char_log(cn, 1, "* Fighting uses the higher of either skill.\n");
		do_char_log(cn, 1, " \n");
	}
	else
	{
		do_char_log(cn, 1, "Use one of the following after #weapon:\n");
		do_char_log(cn, 1, " \n");
		//                 "!        .         .   |     .         .        !"
		do_char_log(cn, 1, "0 or CLAW              lists claws\n");
		do_char_log(cn, 1, "1 or DAGGER            lists daggers\n");
		do_char_log(cn, 1, "2 or STAFF             lists staffs\n");
		do_char_log(cn, 1, "3 or SPEAR             lists spears\n");
		do_char_log(cn, 1, "4 or SHIELD            lists shields\n");
		do_char_log(cn, 1, "5 or SWORD             lists swords\n");
		do_char_log(cn, 1, "6 or DUALSWORD         lists dual-swords\n");
		do_char_log(cn, 1, "7 or AXE               lists axes\n");
		do_char_log(cn, 1, "8 or TWOHANDER         lists twohanders\n");
		do_char_log(cn, 1, "9 or GREATAXE          lists greataxes\n");
		do_char_log(cn, 1, " \n");
		//do_char_log(cn, 1, "Use SPECIAL beforehand to list extra weapons.\n");
		//do_char_log(cn, 1, "ie. #weapon SPECIAL DAGGER\n");
	}
}

void do_listarmors(int cn, char *topic)
{
	do_char_log(cn, 1, "Now listing armors:\n");
	do_char_log(cn, 1, " \n");
	//                 "!        .         .         .         .        !"
	do_char_log(cn, 1, "   Armour   |   Requirements:   |    Gives:    \n");
	do_char_log(cn, 1, "    Tier    | AGL/STR | WIL/INT | AV | MS/Prot \n");
	do_char_log(cn, 1, "------------+---------+---------+----+---------\n");
	do_char_log(cn, 1, "Cloth       |   1   1 |         |  1 |         \n");
	do_char_log(cn, 1, "Leather     |  12  12 |         |  2 |         \n");
	do_char_log(cn, 1, "Bronze      |  16  16 |         |  3 |         \n");
	do_char_log(cn, 1, "Steel       |  24  24 |         |  4 |         \n");
	do_char_log(cn, 1, "Gold        |  34  34 |         |  5 |         \n");
	do_char_log(cn, 1, "Emerald     |  48  48 |         |  6 |         \n");
	do_char_log(cn, 1, "Crystal     |  64  64 |         |  7 |         \n");
	do_char_log(cn, 1, "Titanium    |  84  84 |         |  8 |         \n");
	do_char_log(cn, 1, "------------+---------+---------+----+---------\n");
	do_char_log(cn, 1, "Simple      |         |  15  15 |  2 |       2 \n");
	do_char_log(cn, 1, "Caster      |         |  35  35 |  3 |       2 \n");
	do_char_log(cn, 1, "Adept       |         |  60  60 |  4 |       2 \n");
	do_char_log(cn, 1, "Wizard      |         |  90  90 |  5 |       2 \n");
	do_char_log(cn, 1, " \n");
	do_char_log(cn, 2, "* All chest slots offer doubled values.\n");
	do_char_log(cn, 2, "* Multiply the given value by 6 for the total.\n");
	do_char_log(cn, 1, " \n");
}

void do_listbelts(int cn, char *topic)
{
	do_char_log(cn, 1, "Now listing belts:\n");
	do_char_log(cn, 1, " \n");
	//                 "!        .         .         .         .        !"
	do_char_log(cn, 1, "  Belt  | Requirements:  |    Gives:           \n");
	do_char_log(cn, 1, "  Tier  | BR/WI/IN/AG/ST | EN | BR/WI/IN/AG/ST \n");
	do_char_log(cn, 1, "--------+----------------+----+----------------\n");
	do_char_log(cn, 1, "Green   |                |  5 |  2             \n");
	do_char_log(cn, 1, "Gold    |          15    | 10 | -1        3 -1 \n");
	do_char_log(cn, 1, "Lth/Sil | 12 16 16 12 12 | 10 |  3     1 -1    \n");
	do_char_log(cn, 1, "Lth/Gld | 12 12 12 16 16 | 20 |     1 -2  1  2 \n");
	do_char_log(cn, 1, "Sil/Gld | 18 24 24 18 18 | 15 |  2  3  2 -1 -1 \n");
	do_char_log(cn, 1, "Sil/Red | 18 18 18 24 24 | 25 | -1 -1     3  3 \n");
	do_char_log(cn, 1, "Gld/Sil | 30 40 40 30 30 | 20 |  2  5  4 -2 -1 \n");
	do_char_log(cn, 1, "Gld/Red | 30 30 30 40 40 | 30 | -2 -1 -1  5  6 \n");
//	do_char_log(cn, 1, "Silv *  | 45 60 60 45 45 | 25 |  3  5  5 -3 -2 \n");
//	do_char_log(cn, 1, "Gold *  | 45 45 45 60 60 | 35 |  3 -3 -3  5  5 \n");
	do_char_log(cn, 1, " \n");
}

void do_listrings(int cn, char *topic)
{
	if (strcmp(topic, "0")==0 || strcmp(topic, "DIAMOND")==0 || strcmp(topic, "diamond")==0)
	{
		do_char_log(cn, 1, "Now listing DIAMOND rings:\n");
		do_char_log(cn, 1, " \n");
		//                 "!        .         .         .         .        !"
		do_char_log(cn, 1, "     Ring Tier     | Need  |  *BWIAS*  \n");
		do_char_log(cn, 1, "   Metal, Gem Size | Rank  | OFF |  ON \n");
		do_char_log(cn, 1, "-------------------+-------+-----+-----\n");
		do_char_log(cn, 1, "  Silver, Small    |  Sgt  |   1 |   3 \n");
		do_char_log(cn, 1, "  Silver, Medium   |  SSg  |   2 |   4 \n");
		do_char_log(cn, 1, "  Silver, Big      |  1Sg  |   3 |   5 \n");
		do_char_log(cn, 1, "    Gold, Medium   |  MSg  |   3 |   7 \n");
		do_char_log(cn, 1, "    Gold, Big      |  SgM  |   4 |   8 \n");
		do_char_log(cn, 1, "    Gold, Huge     | 1Lieu |   5 |   9 \n");
		do_char_log(cn, 1, "Platinum, Big      | 2Lieu |   5 |  11 \n");
		do_char_log(cn, 1, "Platinum, Huge     | Captn |   6 |  12 \n");
		do_char_log(cn, 1, "Platinum, Flawless | LtCol |   7 |  13 \n");
		do_char_log(cn, 1, " \n");
	}
	else if (strcmp(topic, "1")==0 || strcmp(topic, "SAPPHIRE")==0 || strcmp(topic, "sapphire")==0)
	{
		do_char_log(cn, 1, "Now listing SAPPHIRE rings:\n");
		do_char_log(cn, 1, " \n");
		//                 "!        .         .         .         .        !"
		do_char_log(cn, 1, "     Ring Tier     | Need  | Braveness \n");
		do_char_log(cn, 1, "   Metal, Gem Size | Rank  | OFF |  ON \n");
		do_char_log(cn, 1, "-------------------+-------+-----+-----\n");
		do_char_log(cn, 1, "  Silver, Small    |  LCp  |   3 |   6 \n");
		do_char_log(cn, 1, "  Silver, Medium   |  Sgt  |   4 |   7 \n");
		do_char_log(cn, 1, "  Silver, Big      |  MSg  |   5 |   8 \n");
		do_char_log(cn, 1, "    Gold, Medium   |  SSg  |   5 |  11 \n");
		do_char_log(cn, 1, "    Gold, Big      |  1Sg  |   7 |  13 \n");
		do_char_log(cn, 1, "    Gold, Huge     | 2Lieu |   9 |  15 \n");
		do_char_log(cn, 1, "Platinum, Big      |  SgM  |   9 |  18 \n");
		do_char_log(cn, 1, "Platinum, Huge     | 1Lieu |  12 |  21 \n");
		do_char_log(cn, 1, "Platinum, Flawless | Major |  15 |  24 \n");
		do_char_log(cn, 1, " \n");
	}
	else if (strcmp(topic, "2")==0 || strcmp(topic, "RUBY")==0 || strcmp(topic, "ruby")==0)
	{
		do_char_log(cn, 1, "Now listing RUBY rings:\n");
		do_char_log(cn, 1, " \n");
		//                 "!        .         .         .         .        !"
		do_char_log(cn, 1, "     Ring Tier     | Need  | Willpower \n");
		do_char_log(cn, 1, "   Metal, Gem Size | Rank  | OFF |  ON \n");
		do_char_log(cn, 1, "-------------------+-------+-----+-----\n");
		do_char_log(cn, 1, "  Silver, Small    |  LCp  |   3 |   6 \n");
		do_char_log(cn, 1, "  Silver, Medium   |  Sgt  |   4 |   7 \n");
		do_char_log(cn, 1, "  Silver, Big      |  MSg  |   5 |   8 \n");
		do_char_log(cn, 1, "    Gold, Medium   |  SSg  |   5 |  11 \n");
		do_char_log(cn, 1, "    Gold, Big      |  1Sg  |   7 |  13 \n");
		do_char_log(cn, 1, "    Gold, Huge     | 2Lieu |   9 |  15 \n");
		do_char_log(cn, 1, "Platinum, Big      |  SgM  |   9 |  18 \n");
		do_char_log(cn, 1, "Platinum, Huge     | 1Lieu |  12 |  21 \n");
		do_char_log(cn, 1, "Platinum, Flawless | Major |  15 |  24 \n");
		do_char_log(cn, 1, " \n");
	}
	else if (strcmp(topic, "3")==0 || strcmp(topic, "AMETHYST")==0 || strcmp(topic, "amethyst")==0)
	{
		do_char_log(cn, 1, "Now listing AMETHYST rings:\n");
		do_char_log(cn, 1, " \n");
		//                 "!        .         .         .         .        !"
		do_char_log(cn, 1, "     Ring Tier     | Need  | Intuition \n");
		do_char_log(cn, 1, "   Metal, Gem Size | Rank  | OFF |  ON \n");
		do_char_log(cn, 1, "-------------------+-------+-----+-----\n");
		do_char_log(cn, 1, "  Silver, Small    |  LCp  |   3 |   6 \n");
		do_char_log(cn, 1, "  Silver, Medium   |  Sgt  |   4 |   7 \n");
		do_char_log(cn, 1, "  Silver, Big      |  MSg  |   5 |   8 \n");
		do_char_log(cn, 1, "    Gold, Medium   |  SSg  |   5 |  11 \n");
		do_char_log(cn, 1, "    Gold, Big      |  1Sg  |   7 |  13 \n");
		do_char_log(cn, 1, "    Gold, Huge     | 2Lieu |   9 |  15 \n");
		do_char_log(cn, 1, "Platinum, Big      |  SgM  |   9 |  18 \n");
		do_char_log(cn, 1, "Platinum, Huge     | 1Lieu |  12 |  21 \n");
		do_char_log(cn, 1, "Platinum, Flawless | Major |  15 |  24 \n");
		do_char_log(cn, 1, " \n");
	}
	else if (strcmp(topic, "4")==0 || strcmp(topic, "TOPAZ")==0 || strcmp(topic, "topaz")==0)
	{
		do_char_log(cn, 1, "Now listing TOPAZ rings:\n");
		do_char_log(cn, 1, " \n");
		//                 "!        .         .         .         .        !"
		do_char_log(cn, 1, "     Ring Tier     | Need  | Agility   \n");
		do_char_log(cn, 1, "   Metal, Gem Size | Rank  | OFF |  ON \n");
		do_char_log(cn, 1, "-------------------+-------+-----+-----\n");
		do_char_log(cn, 1, "  Silver, Small    |  LCp  |   3 |   6 \n");
		do_char_log(cn, 1, "  Silver, Medium   |  Sgt  |   4 |   7 \n");
		do_char_log(cn, 1, "  Silver, Big      |  MSg  |   5 |   8 \n");
		do_char_log(cn, 1, "    Gold, Medium   |  SSg  |   5 |  11 \n");
		do_char_log(cn, 1, "    Gold, Big      |  1Sg  |   7 |  13 \n");
		do_char_log(cn, 1, "    Gold, Huge     | 2Lieu |   9 |  15 \n");
		do_char_log(cn, 1, "Platinum, Big      |  SgM  |   9 |  18 \n");
		do_char_log(cn, 1, "Platinum, Huge     | 1Lieu |  12 |  21 \n");
		do_char_log(cn, 1, "Platinum, Flawless | Major |  15 |  24 \n");
		do_char_log(cn, 1, " \n");
	}
	else if (strcmp(topic, "5")==0 || strcmp(topic, "EMERALD")==0 || strcmp(topic, "emerald")==0)
	{
		do_char_log(cn, 1, "Now listing EMERALD rings:\n");
		do_char_log(cn, 1, " \n");
		//                 "!        .         .         .         .        !"
		do_char_log(cn, 1, "     Ring Tier     | Need  | Strength  \n");
		do_char_log(cn, 1, "   Metal, Gem Size | Rank  | OFF |  ON \n");
		do_char_log(cn, 1, "-------------------+-------+-----+-----\n");
		do_char_log(cn, 1, "  Silver, Small    |  LCp  |   3 |   6 \n");
		do_char_log(cn, 1, "  Silver, Medium   |  Sgt  |   4 |   7 \n");
		do_char_log(cn, 1, "  Silver, Big      |  MSg  |   5 |   8 \n");
		do_char_log(cn, 1, "    Gold, Medium   |  SSg  |   5 |  11 \n");
		do_char_log(cn, 1, "    Gold, Big      |  1Sg  |   7 |  13 \n");
		do_char_log(cn, 1, "    Gold, Huge     | 2Lieu |   9 |  15 \n");
		do_char_log(cn, 1, "Platinum, Big      |  SgM  |   9 |  18 \n");
		do_char_log(cn, 1, "Platinum, Huge     | 1Lieu |  12 |  21 \n");
		do_char_log(cn, 1, "Platinum, Flawless | Major |  15 |  24 \n");
		do_char_log(cn, 1, " \n");
	}
	else if (strcmp(topic, "6")==0 || strcmp(topic, "SPINEL")==0 || strcmp(topic, "spinel")==0)
	{
		do_char_log(cn, 1, "Now listing SPINEL rings:\n");
		do_char_log(cn, 1, " \n");
		//                 "!        .         .         .         .        !"
		do_char_log(cn, 1, "     Ring Tier     | Need  | Spell Apt \n");
		do_char_log(cn, 1, "   Metal, Gem Size | Rank  | OFF |  ON \n");
		do_char_log(cn, 1, "-------------------+-------+-----+-----\n");
		do_char_log(cn, 1, "  Silver, Big      |  1Sg  |   5 |   8 \n");
		do_char_log(cn, 1, "    Gold, Big      |  SgM  |   7 |  13 \n");
		do_char_log(cn, 1, "    Gold, Huge     | 1Lieu |   9 |  15 \n");
		do_char_log(cn, 1, "Platinum, Big      | 2Lieu |   9 |  18 \n");
		do_char_log(cn, 1, "Platinum, Huge     | Captn |  12 |  21 \n");
		do_char_log(cn, 1, "Platinum, Flawless | LtCol |  15 |  24 \n");
		do_char_log(cn, 1, " \n");
	}
	else if (strcmp(topic, "7")==0 || strcmp(topic, "CITRINE")==0 || strcmp(topic, "citrine")==0)
	{
		do_char_log(cn, 1, "Now listing CITRINE rings:\n");
		do_char_log(cn, 1, " \n");
		//                 "!        .         .         .         .        !"
		do_char_log(cn, 1, "     Ring Tier     | Need  | Crit Mult \n");
		do_char_log(cn, 1, "   Metal, Gem Size | Rank  | OFF |  ON \n");
		do_char_log(cn, 1, "-------------------+-------+-----+-----\n");
		do_char_log(cn, 1, "  Silver, Big      |  1Sg  |   5 |   8 \n");
		do_char_log(cn, 1, "    Gold, Big      |  SgM  |  10 |  16 \n");
		do_char_log(cn, 1, "    Gold, Huge     | 1Lieu |  15 |  21 \n");
		do_char_log(cn, 1, "Platinum, Big      | 2Lieu |  15 |  24 \n");
		do_char_log(cn, 1, "Platinum, Huge     | Captn |  20 |  29 \n");
		do_char_log(cn, 1, "Platinum, Flawless | LtCol |  25 |  34 \n");
		do_char_log(cn, 1, " \n");
	}
	else if (strcmp(topic, "8")==0 || strcmp(topic, "OPAL")==0 || strcmp(topic, "opal")==0)
	{
		do_char_log(cn, 1, "Now listing OPAL rings:\n");
		do_char_log(cn, 1, " \n");
		//                 "!        .         .         .         .        !"
		do_char_log(cn, 1, "     Ring Tier     | Need  | All Speed \n");
		do_char_log(cn, 1, "   Metal, Gem Size | Rank  | OFF |  ON \n");
		do_char_log(cn, 1, "-------------------+-------+-----+-----\n");
		do_char_log(cn, 1, "  Silver, Big      |  1Sg  |   5 |   8 \n");
		do_char_log(cn, 1, "    Gold, Big      |  SgM  |   7 |  13 \n");
		do_char_log(cn, 1, "    Gold, Huge     | 1Lieu |   9 |  15 \n");
		do_char_log(cn, 1, "Platinum, Big      | 2Lieu |   9 |  18 \n");
		do_char_log(cn, 1, "Platinum, Huge     | Captn |  12 |  21 \n");
		do_char_log(cn, 1, "Platinum, Flawless | LtCol |  15 |  24 \n");
		do_char_log(cn, 1, " \n");
	}
	else if (strcmp(topic, "9")==0 || strcmp(topic, "AQUAMARINE")==0 || strcmp(topic, "aquamarine")==0)
	{
		do_char_log(cn, 1, "Now listing AQUAMARINE rings:\n");
		do_char_log(cn, 1, " \n");
		//                 "!        .         .         .         .        !"
		do_char_log(cn, 1, "     Ring Tier     | Need  |  Top Dmg  \n");
		do_char_log(cn, 1, "   Metal, Gem Size | Rank  | OFF |  ON \n");
		do_char_log(cn, 1, "-------------------+-------+-----+-----\n");
		do_char_log(cn, 1, "  Silver, Big      |  1Sg  |   5 |   8 \n");
		do_char_log(cn, 1, "    Gold, Big      |  SgM  |   7 |  13 \n");
		do_char_log(cn, 1, "    Gold, Huge     | 1Lieu |   9 |  15 \n");
		do_char_log(cn, 1, "Platinum, Big      | 2Lieu |   9 |  18 \n");
		do_char_log(cn, 1, "Platinum, Huge     | Captn |  12 |  21 \n");
		do_char_log(cn, 1, "Platinum, Flawless | LtCol |  15 |  24 \n");
		do_char_log(cn, 1, " \n");
	}
	else if (strcmp(topic, "10")==0 || strcmp(topic, "BERYL")==0 || strcmp(topic, "beryl")==0)
	{
		do_char_log(cn, 1, "Now listing BERYL rings:\n");
		do_char_log(cn, 1, " \n");
		//                 "!        .         .         .         .        !"
		do_char_log(cn, 1, "     Ring Tier     | Need  | Crit Bonus\n");
		do_char_log(cn, 1, "  Metal, Gem Size  | Rank  | OFF |  ON \n");
		do_char_log(cn, 1, "-------------------+-------+-----+-----\n");
		do_char_log(cn, 1, "    Gold, Huge     | Captn |  24 |  30 \n");
		do_char_log(cn, 1, "Platinum, Huge     | Major |  32 |  41 \n");
		do_char_log(cn, 1, "Platinum, Flawless | Colnl |  40 |  49 \n");
		do_char_log(cn, 1, " \n");
	}
	else if (strcmp(topic, "11")==0 || strcmp(topic, "ZIRCON")==0 || strcmp(topic, "zircon")==0)
	{
		do_char_log(cn, 1, "Now listing ZIRCON rings:\n");
		do_char_log(cn, 1, " \n");
		//                 "!        .         .         .         .        !"
		do_char_log(cn, 1, "     Ring Tier     | Need  |  Thorns   \n");
		do_char_log(cn, 1, "  Metal, Gem Size  | Rank  | OFF |  ON \n");
		do_char_log(cn, 1, "-------------------+-------+-----+-----\n");
		do_char_log(cn, 1, "    Gold, Huge     | Captn |   4 |   6 \n");
		do_char_log(cn, 1, "Platinum, Huge     | Major |   5 |   8 \n");
		do_char_log(cn, 1, "Platinum, Flawless | Colnl |   7 |  10 \n");
		do_char_log(cn, 1, " \n");
	}
	else if (strcmp(topic, "12")==0 || strcmp(topic, "SPHALERITE")==0 || strcmp(topic, "sphalerite")==0)
	{
		do_char_log(cn, 1, "Now listing SPHALERITE rings:\n");
		do_char_log(cn, 1, " \n");
		//                 "!        .         .         .         .        !"
		do_char_log(cn, 1, "     Ring Tier     | Need  | Cooldown  \n");
		do_char_log(cn, 1, "  Metal, Gem Size  | Rank  | OFF |  ON \n");
		do_char_log(cn, 1, "-------------------+-------+-----+-----\n");
		do_char_log(cn, 1, "    Gold, Huge     | Captn |   4 |   6 \n");
		do_char_log(cn, 1, "Platinum, Huge     | Major |   5 |   8 \n");
		do_char_log(cn, 1, "Platinum, Flawless | Colnl |   7 |  10 \n");
		do_char_log(cn, 1, " \n");
	}
	else
	{
		do_char_log(cn, 1, "Use one of the following after #ring:\n");
		do_char_log(cn, 1, " \n");
		//                 "!        .         .   |     .         .        !"
		do_char_log(cn, 1, " 0 or DIAMOND           lists diamond rings\n");
		do_char_log(cn, 1, " 1 or SAPPHIRE          lists sapphire rings\n");
		do_char_log(cn, 1, " 2 or RUBY              lists ruby rings\n");
		do_char_log(cn, 1, " 3 or AMETHYST          lists amethyst rings\n");
		do_char_log(cn, 1, " 4 or TOPAZ             lists topaz rings\n");
		do_char_log(cn, 1, " 5 or EMERALD           lists emerald rings\n");
		do_char_log(cn, 1, " 6 or SPINEL            lists spinel rings\n");
		do_char_log(cn, 1, " 7 or CITRINE           lists citrine rings\n");
		do_char_log(cn, 1, " 8 or OPAL              lists opal rings\n");
		do_char_log(cn, 1, " 9 or AQUAMARINE        lists aquamarine rings\n");
		do_char_log(cn, 1, "10 or BERYL             lists beryl rings\n");
		do_char_log(cn, 1, "11 or ZIRCON            lists zircon rings\n");
		do_char_log(cn, 1, "12 or SPHALERITE        lists sphalerite rings\n");
		do_char_log(cn, 1, " \n");
	}
}

void do_listtarots(int cn, char *topic)
{
	if (strcmp(topic, "0")==0 || strcmp(topic, "1")==0)
	{	//                 "!        .         .         .         .        !"
		do_char_log(cn, 1, "0 The Fool:\n");
		do_char_log(cn, 8, DESC_FOOL);
		do_char_log(cn, 1, " \n");
		do_char_log(cn, 1, "0 The Fool, Reversed:\n");
		do_char_log(cn, 8, DESC_FOOL_R);
		do_char_log(cn, 1, " \n");
		do_char_log(cn, 1, "I The Magician:\n");
		do_char_log(cn, 8, DESC_MAGI);
		do_char_log(cn, 1, " \n");
		do_char_log(cn, 1, "I The Magician, Reversed:\n");
		do_char_log(cn, 8, DESC_MAGI_R);
		do_char_log(cn, 1, " \n");
	}
	else if (strcmp(topic, "2")==0 || strcmp(topic, "3")==0)
	{	//                 "!        .         .         .         .        !"
		do_char_log(cn, 1, "II The High Priestess:\n");
		do_char_log(cn, 8, DESC_PREIST);
		do_char_log(cn, 1, " \n");
		do_char_log(cn, 1, "II The High Priestess, Reversed:\n");
		do_char_log(cn, 8, DESC_PREIST_R);
		do_char_log(cn, 1, " \n");
		do_char_log(cn, 1, "III The Empress:\n");
		do_char_log(cn, 8, DESC_EMPRESS);
		do_char_log(cn, 1, " \n");
		do_char_log(cn, 1, "III The Empress, Reversed:\n");
		do_char_log(cn, 8, DESC_EMPRES_R);
		do_char_log(cn, 1, " \n");
	}
	else if (strcmp(topic, "4")==0 || strcmp(topic, "5")==0)
	{	//                 "!        .         .         .         .        !"
		do_char_log(cn, 1, "IV The Emperor:\n");
		do_char_log(cn, 8, DESC_EMPEROR);
		do_char_log(cn, 1, " \n");
		do_char_log(cn, 1, "IV The Emperor, Reversed:\n");
		do_char_log(cn, 8, DESC_EMPERO_R);
		do_char_log(cn, 1, " \n");
		do_char_log(cn, 1, "V The Hierophant:\n");
		do_char_log(cn, 8, DESC_HEIROPH);
		do_char_log(cn, 1, " \n");
		do_char_log(cn, 1, "V The Hierophant, Reversed:\n");
		do_char_log(cn, 8, DESC_HEIROP_R);
		do_char_log(cn, 1, " \n");
	}
	else if (strcmp(topic, "6")==0 || strcmp(topic, "7")==0)
	{	//                 "!        .         .         .         .        !"
		do_char_log(cn, 1, "VI The Lovers:\n");
		do_char_log(cn, 8, DESC_LOVERS);
		do_char_log(cn, 1, " \n");
		do_char_log(cn, 1, "VI The Lovers, Reversed:\n");
		do_char_log(cn, 8, DESC_LOVERS_R);
		do_char_log(cn, 1, " \n");
		do_char_log(cn, 1, "VII The Chariot:\n");
		do_char_log(cn, 8, DESC_CHARIOT);
		do_char_log(cn, 1, " \n");
		do_char_log(cn, 1, "VII The Chariot, Reversed:\n");
		do_char_log(cn, 8, DESC_CHARIO_R);
		do_char_log(cn, 1, " \n");
	}
	else if (strcmp(topic, "8")==0 || strcmp(topic, "9")==0)
	{	//                 "!        .         .         .         .        !"
		do_char_log(cn, 1, "VIII Strength:\n");
		do_char_log(cn, 8, DESC_STRENGTH);
		do_char_log(cn, 1, " \n");
		do_char_log(cn, 1, "VIII Strength, Reversed:\n");
		do_char_log(cn, 8, DESC_STRENG_R);
		do_char_log(cn, 1, " \n");
		do_char_log(cn, 1, "IX The Hermit:\n");
		do_char_log(cn, 8, DESC_HERMIT);
		do_char_log(cn, 1, " \n");
		do_char_log(cn, 1, "IX The Hermit, Reversed:\n");
		do_char_log(cn, 8, DESC_HERMIT_R);
		do_char_log(cn, 1, " \n");
	}
	else if (strcmp(topic, "10")==0 || strcmp(topic, "11")==0)
	{	//                 "!        .         .         .         .        !"
		do_char_log(cn, 1, "X Wheel of Fortune:\n");
		do_char_log(cn, 8, DESC_WHEEL);
		do_char_log(cn, 1, " \n");
		do_char_log(cn, 1, "X Wheel of Fortune, Reversed:\n");
		do_char_log(cn, 8, DESC_WHEEL_R);
		do_char_log(cn, 1, " \n");
		do_char_log(cn, 1, "XI Justice:\n");
		do_char_log(cn, 8, DESC_JUSTICE);
		do_char_log(cn, 1, " \n");
		do_char_log(cn, 1, "XI Justice, Reversed:\n");
		do_char_log(cn, 8, DESC_JUSTIC_R);
		do_char_log(cn, 1, " \n");
	}
	else if (strcmp(topic, "12")==0 || strcmp(topic, "13")==0)
	{	//                 "!        .         .         .         .        !"
		do_char_log(cn, 1, "XII The Hanged Man:\n");
		do_char_log(cn, 8, DESC_HANGED);
		do_char_log(cn, 1, " \n");
		do_char_log(cn, 1, "XII The Hanged Man, Reversed:\n");
		do_char_log(cn, 8, DESC_HANGED_R);
		do_char_log(cn, 1, " \n");
		do_char_log(cn, 1, "XIII Death:\n");
		do_char_log(cn, 8, DESC_DEATH);
		do_char_log(cn, 1, " \n");
		do_char_log(cn, 1, "XIII Death, Reversed:\n");
		do_char_log(cn, 8, DESC_DEATH_R);
		do_char_log(cn, 1, " \n");
	}
	else if (strcmp(topic, "14")==0 || strcmp(topic, "15")==0)
	{	//                 "!        .         .         .         .        !"
		do_char_log(cn, 1, "XIV Temperance:\n");
		do_char_log(cn, 8, DESC_TEMPER);
		do_char_log(cn, 1, " \n");
		do_char_log(cn, 1, "XIV Temperance, Reversed:\n");
		do_char_log(cn, 8, DESC_TEMPER_R);
		do_char_log(cn, 1, " \n");
		do_char_log(cn, 1, "XV The Devil:\n");
		do_char_log(cn, 8, DESC_DEVIL);
		do_char_log(cn, 1, " \n");
		do_char_log(cn, 1, "XV The Devil, Reversed:\n");
		do_char_log(cn, 8, DESC_DEVIL_R);
		do_char_log(cn, 1, " \n");
	}
	else if (strcmp(topic, "16")==0 || strcmp(topic, "17")==0)
	{	//                 "!        .         .         .         .        !"
		do_char_log(cn, 1, "XVI The Tower:\n");
		do_char_log(cn, 8, DESC_TOWER);
		do_char_log(cn, 1, " \n");
		do_char_log(cn, 1, "XVI The Tower, Reversed:\n");
		do_char_log(cn, 8, DESC_TOWER_R);
		do_char_log(cn, 1, " \n");
		do_char_log(cn, 1, "XVII The Star:\n");
		do_char_log(cn, 8, DESC_STAR);
		do_char_log(cn, 1, " \n");
		do_char_log(cn, 1, "XVII The Star, Reversed:\n");
		do_char_log(cn, 8, DESC_STAR_R);
		do_char_log(cn, 1, " \n");
	}
	else if (strcmp(topic, "18")==0 || strcmp(topic, "19")==0)
	{	//                 "!        .         .         .         .        !"
		do_char_log(cn, 1, "XVIII The Moon:\n");
		do_char_log(cn, 8, DESC_MOON);
		do_char_log(cn, 1, " \n");
		do_char_log(cn, 1, "XVIII The Moon, Reversed:\n");
		do_char_log(cn, 8, DESC_MOON_R);
		do_char_log(cn, 1, " \n");
		do_char_log(cn, 1, "XIX The Sun:\n");
		do_char_log(cn, 8, DESC_SUN);
		do_char_log(cn, 1, " \n");
		do_char_log(cn, 1, "XIX The Sun, Reversed:\n");
		do_char_log(cn, 8, DESC_SUN_R);
		do_char_log(cn, 1, " \n");
	}
	else if (strcmp(topic, "20")==0 || strcmp(topic, "21")==0)
	{	//                 "!        .         .         .         .        !"
		do_char_log(cn, 1, "XX Judgement:\n");
		do_char_log(cn, 8, DESC_JUDGE);
		do_char_log(cn, 1, " \n");
		do_char_log(cn, 1, "XX Judgement, Reversed:\n");
		do_char_log(cn, 8, DESC_JUDGE_R);
		do_char_log(cn, 1, " \n");
		do_char_log(cn, 1, "XXI The World:\n");
		do_char_log(cn, 8, DESC_WORLD);
		do_char_log(cn, 1, " \n");
		do_char_log(cn, 1, "XXI The World, Reversed:\n");
		do_char_log(cn, 8, DESC_WORLD_R);
		do_char_log(cn, 1, " \n");
	}
	else
	{
		do_char_log(cn, 1, "Use one of the following after #tarot:\n");
		do_char_log(cn, 1, " \n");
		//                 "!        .         .   |     .         .        !"
		do_char_log(cn, 1, " 0 or  1      lists FOOL     and MAGICIAN cards\n");
		do_char_log(cn, 1, " 2 or  3      lists PRIEST   and EMPRESS  cards\n");
		do_char_log(cn, 1, " 4 or  5      lists EMPEROR  and HEIROPH  cards\n");
		do_char_log(cn, 1, " 6 or  7      lists LOVERS   and CHARIOT  cards\n");
		do_char_log(cn, 1, " 8 or  9      lists STRENGTH and HERMIT   cards\n");
		do_char_log(cn, 1, "10 or 11      lists WHEEL    and JUSTICE  cards\n");
		do_char_log(cn, 1, "12 or 13      lists HANGED   and DEATH    cards\n");
		do_char_log(cn, 1, "14 or 15      lists TEMPER   and DEVIL    cards\n");
		do_char_log(cn, 1, "16 or 17      lists TOWER    and STAR     cards\n");
		do_char_log(cn, 1, "18 or 19      lists MOON     and SUN      cards\n");
		do_char_log(cn, 1, "20 or 21      lists JUDGE    and WORLD    cards\n");
		do_char_log(cn, 1, " \n");
	}
}

void do_strongholdpoints(int cn)
{
	int n, wavenum=0, waveprog=0;
	do_char_log(cn, 2, "You currently have %d Black Stronghold Points.\n",ch[cn].bs_points);
	
	if (globs->flags & GF_NEWBS)
	{
		for (n = 1; n<MAXCHARS; n++) 
		{
			if (ch[n].used==USE_EMPTY) continue;
			if (ch[n].temp==CT_BSMAGE1 && is_inline(cn, 1)) { wavenum = ch[n].data[1]; waveprog = ch[n].data[2]-1; break; }
			if (ch[n].temp==CT_BSMAGE2 && is_inline(cn, 2)) { wavenum = ch[n].data[1]; waveprog = ch[n].data[2]-1; break; }
			if (ch[n].temp==CT_BSMAGE3 && is_inline(cn, 3)) { wavenum = ch[n].data[1]; waveprog = ch[n].data[2]-1; break; }
		}
		
		if (wavenum>0)
		{
			do_char_log(cn, 3, "Wave %d progress: %3d%%\n", wavenum, waveprog*10);
		}
	}
	else
	{
		for (n = 1; n<MAXCHARS; n++) 
		{
			if (ch[n].used==USE_EMPTY) continue;
			if (ch[n].temp==CT_BSMAGE1 && is_inline(cn, 1)) { wavenum = ch[n].data[3]; waveprog = ch[n].data[2]; break; }
			if (ch[n].temp==CT_BSMAGE2 && is_inline(cn, 2)) { wavenum = ch[n].data[3]; waveprog = ch[n].data[2]; break; }
			if (ch[n].temp==CT_BSMAGE3 && is_inline(cn, 3)) { wavenum = ch[n].data[3]; waveprog = ch[n].data[2]; break; }
		}
		
		if (wavenum>0)
		{
			switch (wavenum)
			{
				case  1: 	waveprog = 10000*(waveprog-(BS_RC*0))/BS_RC; break;
				case  2: 	waveprog = 10000*(waveprog-(BS_RC*0))/BS_RC; break;
				case  3: 	waveprog = 10000*(waveprog-(BS_RC*1))/BS_RC; break;
				case  4: 	waveprog = 10000*(waveprog-(BS_RC*0))/BS_RC; break;
				case  5: 	waveprog = 10000*(waveprog-(BS_RC*1))/BS_RC; break;
				case  6: 	waveprog = 10000*(waveprog-(BS_RC*2))/BS_RC; break;
				case  7: 	waveprog = 10000*(waveprog-(BS_RC*0))/BS_RC; break;
				case  8: 	waveprog = 10000*(waveprog-(BS_RC*1))/BS_RC; break;
				case  9: 	waveprog = 10000*(waveprog-(BS_RC*2))/BS_RC; break;
				case 10: 	waveprog = 10000*(waveprog-(BS_RC*3))/BS_RC; break;
				default:	waveprog = 10000*(waveprog-(BS_RC*(wavenum-11)))/BS_RC; break;
			}
			do_char_log(cn, 3, "Wave %d progress: %3d%%\n", wavenum, waveprog/100);
		}
	}
}

void do_showrank(int cn)
{
	if (getrank(cn)>=RANKS-1)
	{
		do_char_log(cn, 2, "You are at the maximum rank. Good job.\n");
	}
	else
	{
		do_char_log(cn, 2, "You need %d exp to rank to %s.\n",
			rank2points(getrank(cn)) - ch[cn].points_tot,
			rank_name[getrank(cn)+1]);
	}
}

void do_showranklist(int cn)
{
	do_char_log(cn, 1, "Now listing total exp for each rank:\n");
	do_char_log(cn, 1, " \n");
	//                 "!        .         .         .         .        !"
	do_char_log(cn, 1, "Private             0    Lieu Col    4,641,000\n");
	do_char_log(cn, 1, "Private FC        250    Colonel     6,783,000\n");
	do_char_log(cn, 1, "Lance Corp      1,750    Brig Gen    9,690,000\n");
	do_char_log(cn, 1, "Corporal        7,000    Major Gen  13,566,000\n");
	do_char_log(cn, 1, "Sergeant       21,000    Lieu Gen   18,653,250\n");
	do_char_log(cn, 1, "Staff Serg     52,500    General    25,236,750\n");
	do_char_log(cn, 1, "Mast Serg     115,500    Field Mar  33,649,000\n");
	do_char_log(cn, 1, "1st Serg      231,000    Knight     44,275,000\n");
	do_char_log(cn, 1, "Serg Major    429,000    Baron      57,557,500\n");
	do_char_log(cn, 1, "2nd Lieu      750,750    Earl       74,002,500\n");
	do_char_log(cn, 1, "1st Lieu    1,251,250    Marquess   94,185,000\n");
	do_char_log(cn, 1, "Captain     2,002,000    Warlord   118,755,000\n");
	do_char_log(cn, 1, "Major       3,094,000    \n");
	do_char_log(cn, 1, " \n");
}

void do_showkwai(int cn, char *topic)
{
	int n, m, t, page = 1;
	int kwai[32]={0};

	if (strcmp(topic, "2")==0) page = 2;

	for (n=0;n<32;n++)
	{
		if (ch[cn].data[21] & (1 << n)) kwai[n] = 1;
	}

	do_char_log(cn, 1, "Now listing Shrines of Kwai (PAGE %d):\n", page);
	do_char_log(cn, 1, " \n");
	
	n = (page-1)*16;
	t = page*16;
	m = 0;
	//
	if (!kwai[ 0]) { m++; if (m<=t && m>n) do_char_log(cn, 1, "Cursed Tomb\n"); }
	if (!kwai[ 1]) { m++; if (m<=t && m>n) do_char_log(cn, 1, "Magic Maze\n"); }
	if (!kwai[14]) { m++; if (m<=t && m>n) do_char_log(cn, 1, "Novice Pentagram Quest\n"); }
	if (!kwai[17]) { m++; if (m<=t && m>n) do_char_log(cn, 1, "Lab I, Grolm Gorge\n"); }
	if (!kwai[15]) { m++; if (m<=t && m>n) do_char_log(cn, 1, "Earth Pentagram Quest\n"); }
	if (!kwai[18]) { m++; if (m<=t && m>n) do_char_log(cn, 1, "Lab II, Lizard Gorge\n"); }
	if (!kwai[19]) { m++; if (m<=t && m>n) do_char_log(cn, 1, "Lab III, Undead Gorge\n"); }
	if (!kwai[ 7]) { m++; if (m<=t && m>n) do_char_log(cn, 1, "Underground I\n"); }
	if (!kwai[20]) { m++; if (m<=t && m>n) do_char_log(cn, 1, "Lab IV, Caster Gorge\n"); }
	if (!kwai[21]) { m++; if (m<=t && m>n) do_char_log(cn, 1, "Lab V, Knight Gorge\n"); }
	if (!kwai[ 3]) { m++; if (m<=t && m>n) do_char_log(cn, 1, "Stronghold, North\n"); }
	if (!kwai[22]) { m++; if (m<=t && m>n) do_char_log(cn, 1, "Lab VI, Desert Gorge\n"); }
	if (!kwai[23]) { m++; if (m<=t && m>n) do_char_log(cn, 1, "Lab VII, Light/Dark Gorge\n"); }
	if (!kwai[ 2]) { m++; if (m<=t && m>n) do_char_log(cn, 1, "Damor's Magic Shop\n"); }
	if (!kwai[10]) { m++; if (m<=t && m>n) do_char_log(cn, 1, "Winding Valley\n"); }
	if (!kwai[24]) { m++; if (m<=t && m>n) do_char_log(cn, 1, "Lab VIII, Underwater Gorge\n"); }
	if (!kwai[25]) { m++; if (m<=t && m>n) do_char_log(cn, 1, "Lab IX, Riddle Gorge\n"); }
	if (!kwai[ 8]) { m++; if (m<=t && m>n) do_char_log(cn, 1, "Underground II\n"); }
	if (!kwai[26]) { m++; if (m<=t && m>n) do_char_log(cn, 1, "Lab X, Forest Gorge\n"); }
	if (!kwai[27]) { m++; if (m<=t && m>n) do_char_log(cn, 1, "Lab XI, Seasons Gorge\n"); }
	if (!kwai[12]) { m++; if (m<=t && m>n) do_char_log(cn, 1, "Gargoyle Nest\n"); }
	if (!kwai[28]) { m++; if (m<=t && m>n) do_char_log(cn, 1, "Lab XII, Ascent Gorge\n"); }
	if (!kwai[ 4]) { m++; if (m<=t && m>n) do_char_log(cn, 1, "Black Stronghold\n"); }
	if (!kwai[11]) { m++; if (m<=t && m>n) do_char_log(cn, 1, "Northern Mountains\n"); }
	if (!kwai[30]) { m++; if (m<=t && m>n) do_char_log(cn, 1, "Empty Outset\n"); }
	if (!kwai[ 5]) { m++; if (m<=t && m>n) do_char_log(cn, 1, "Stronghold, South\n"); }
	if (!kwai[ 9]) { m++; if (m<=t && m>n) do_char_log(cn, 1, "Underground III\n"); }
	if (!kwai[13]) { m++; if (m<=t && m>n) do_char_log(cn, 1, "The Emerald Cavern\n"); }
	if (!kwai[29]) { m++; if (m<=t && m>n) do_char_log(cn, 1, "Ice Gargoyle Nest\n"); }
	if (!kwai[16]) { m++; if (m<=t && m>n) do_char_log(cn, 1, "Jungle Pentagram Quest\n"); }
	if (!kwai[ 6]) { m++; if (m<=t && m>n) do_char_log(cn, 1, "Cold Cavern\n"); }
	if (!kwai[31]) { m++; if (m<=t && m>n) do_char_log(cn, 1, "Onyx Gargoyle Nest\n"); }
	//
	do_char_log(cn, 1, " \n");
	do_char_log(cn, 2, "Showing page %d of %d. #shrine <x> to swap.\n", page, max(1,min(2, (m-1)/16+1)));
	do_char_log(cn, 1, " \n");
}

int display_pole(int cn, int flag, int page, int m, int pole, char *qtxt, int exp)
{
	if (IS_RB(cn)) exp = exp/2;
	if (flag || !pole) 
	{ 
		m++; 
		if (m<=page*16 && m>(page-1)*16) 
		{
			do_char_log(cn, pole?3:1, "%-23.23s  %6d\n", qtxt, pole?0:exp);
		}
	}
	return m;
}

int disp_rb_pole(int cn, int flag, int page, int m, int pole, char *qtxt)
{
	if (IS_RB(cn) && (flag || !pole))
	{ 
		m++; 
		if (m<=page*16 && m>(page-1)*16) 
		{
			do_char_log(cn, pole?6:9, "%-23.23s\n", qtxt);
		}
	}
	return m;
}

void do_showpoles(int cn, int flag, char *topic) // flag 1 = all, 0 = unattained
{
	int n, m, page = 1;
	int pole1[32]={0};
	int pole2[32]={0};
	int pole3[32]={0};
	int pole4[32]={0};
	int pole5[32]={0};
	int pole6[32]={0};
	int poleR[32]={0};

	if (strcmp(topic,  "2")==0) page = 2;
	if (strcmp(topic,  "3")==0) page = 3;
	if (strcmp(topic,  "4")==0) page = 4;
	if (strcmp(topic,  "5")==0) page = 5;
	if (strcmp(topic,  "6")==0) page = 6;
	if (strcmp(topic,  "7")==0) page = 7;
	if (strcmp(topic,  "8")==0) page = 8;
	if (strcmp(topic,  "9")==0) page = 9;
	if (strcmp(topic, "10")==0) page = 10;
	if (strcmp(topic, "11")==0) page = 11;
	if (strcmp(topic, "12")==0) page = 12;
	if (strcmp(topic, "13")==0) page = 13;
	if (strcmp(topic, "14")==0) page = 14;
	if (strcmp(topic, "15")==0) page = 15;
	if (strcmp(topic, "16")==0) page = 16;

	for (n=0;n<32;n++)
	{
		if (ch[cn].data[46] & (1 << n)) pole1[n] = 1;
		if (ch[cn].data[47] & (1 << n)) pole2[n] = 1;
		if (ch[cn].data[48] & (1 << n)) pole3[n] = 1;
		if (ch[cn].data[49] & (1 << n)) pole4[n] = 1;
		if (ch[cn].data[91] & (1 << n)) pole5[n] = 1;
		if (ch[cn].data[24] & (1 << n)) pole6[n] = 1;
		if (ch[cn].rebirth  & (1 << n)) poleR[n] = 1;
	}

	do_char_log(cn, 1, "Now listing poles (PAGE %d):\n", page);
	do_char_log(cn, 1, " \n");
	
	m = 0;
	//
	m = display_pole(cn, flag, page, m, pole1[ 0], "Thieves House Cellar",       200);
	m = display_pole(cn, flag, page, m, pole4[ 4], "Novice Pentagram Quest",    1000);
	m = display_pole(cn, flag, page, m, pole1[ 2], "Weeping Woods",             2000);
	m = display_pole(cn, flag, page, m, pole1[ 3], "Weeping Woods",             2000);
	m = display_pole(cn, flag, page, m, pole4[ 5], "Novice Pentagram Quest",    2000);
	m = display_pole(cn, flag, page, m, pole4[ 6], "Novice Pentagram Quest",    3000);
	m = display_pole(cn, flag, page, m, pole1[ 4], "Spider's Den",              4000);
	m = display_pole(cn, flag, page, m, pole1[ 5], "Old Manor",                 4000);
	m = display_pole(cn, flag, page, m, pole4[ 7], "Earth Pentagram Quest",     4000);
	m = display_pole(cn, flag, page, m, pole2[14], "Underground I",             6000);
	m = display_pole(cn, flag, page, m, pole4[ 8], "Earth Pentagram Quest",     6000);
	m = display_pole(cn, flag, page, m, pole3[ 0], "Lab I, Grolm Gorge",        6125);
	m = display_pole(cn, flag, page, m, pole3[ 1], "Lab I, Grolm Gorge",        6125);
	m = display_pole(cn, flag, page, m, pole3[ 2], "Lab I, Grolm Gorge",        6125);
	m = disp_rb_pole(cn, flag, page, m, poleR[ 1], "Lab I, Grolm Gorge");
	m = display_pole(cn, flag, page, m, pole2[15], "Underground I",             6500);
	m = display_pole(cn, flag, page, m, pole2[16], "Underground I",             7000);
	m = display_pole(cn, flag, page, m, pole1[ 6], "Arachnid Den",              7000);
	m = display_pole(cn, flag, page, m, pole2[17], "Underground I",             7500);
	m = display_pole(cn, flag, page, m, pole1[ 7], "Butler's Mansion",          7750);
	m = display_pole(cn, flag, page, m, pole1[ 1], "Azrael's Throne Room",      8000);
	m = display_pole(cn, flag, page, m, pole2[18], "Underground I",             8000);
	m = display_pole(cn, flag, page, m, pole4[ 9], "Earth Pentagram Quest",     8000);
	m = display_pole(cn, flag, page, m, pole1[ 8], "Bell House Basement",       8750);
	m = display_pole(cn, flag, page, m, pole3[ 3], "Lab II, Lizard Gorge",      8750);
	m = display_pole(cn, flag, page, m, pole3[ 4], "Lab II, Lizard Gorge",      8750);
	m = display_pole(cn, flag, page, m, pole3[ 5], "Lab II, Lizard Gorge",      8750);
	m = disp_rb_pole(cn, flag, page, m, poleR[ 2], "Lab II, Lizard Gorge");
	m = display_pole(cn, flag, page, m, pole2[19], "Underground I",            10000);
	m = display_pole(cn, flag, page, m, pole2[ 7], "Stronghold, North",        10000);
	m = display_pole(cn, flag, page, m, pole4[10], "Earth Pentagram Quest",    10000);
	m = display_pole(cn, flag, page, m, pole3[ 6], "Lab III, Undead Gorge",    12000);
	m = display_pole(cn, flag, page, m, pole3[ 7], "Lab III, Undead Gorge",    12000);
	m = display_pole(cn, flag, page, m, pole3[ 8], "Lab III, Undead Gorge",    12000);
	m = disp_rb_pole(cn, flag, page, m, poleR[ 3], "Lab III, Undead Gorge");
	m = display_pole(cn, flag, page, m, pole4[11], "Earth Pentagram Quest",    12000);
	m = display_pole(cn, flag, page, m, pole5[ 2], "Strange Forest",           13250);
	m = display_pole(cn, flag, page, m, pole5[ 3], "Strange Forest",           13250);
	m = display_pole(cn, flag, page, m, pole4[12], "Earth Pentagram Quest",    15000);
	m = display_pole(cn, flag, page, m, pole3[ 9], "Lab IV, Caster Gorge",     16750);
	m = display_pole(cn, flag, page, m, pole3[10], "Lab IV, Caster Gorge",     16750);
	m = display_pole(cn, flag, page, m, pole3[11], "Lab IV, Caster Gorge",     16750);
	m = disp_rb_pole(cn, flag, page, m, poleR[ 4], "Lab IV, Caster Gorge");
	m = display_pole(cn, flag, page, m, pole4[13], "Fire Pentagram Quest",     18000);
	m = display_pole(cn, flag, page, m, pole5[ 4], "Autumn Meadow",            18250);
	m = display_pole(cn, flag, page, m, pole5[ 5], "Autumn Meadow",            18250);
	m = display_pole(cn, flag, page, m, pole1[ 9], "Webbed Bush",              18500);
	m = display_pole(cn, flag, page, m, pole1[16], "Forgotten Canyon",         20000);
	m = display_pole(cn, flag, page, m, pole2[ 0], "Gargoyle Nest",            20000);
	m = display_pole(cn, flag, page, m, pole3[12], "Lab V, Knight Gorge",      20625);
	m = display_pole(cn, flag, page, m, pole3[13], "Lab V, Knight Gorge",      20625);
	m = display_pole(cn, flag, page, m, pole3[14], "Lab V, Knight Gorge",      20625);
	m = disp_rb_pole(cn, flag, page, m, poleR[ 5], "Lab V, Knight Gorge");
	m = display_pole(cn, flag, page, m, pole4[14], "Fire Pentagram Quest",     21000);
	m = display_pole(cn, flag, page, m, pole1[13], "The Mad Hermit's House",   21500);
	m = display_pole(cn, flag, page, m, pole2[20], "Underground II",           24000);
	m = display_pole(cn, flag, page, m, pole4[15], "Fire Pentagram Quest",     24000);
	m = display_pole(cn, flag, page, m, pole1[11], "Astonia Penitentiary",     26000);
	m = display_pole(cn, flag, page, m, pole2[21], "Underground II",           26000);
	m = display_pole(cn, flag, page, m, pole1[10], "Aston Mines, Level II",    27000);
	m = display_pole(cn, flag, page, m, pole3[15], "Lab VI, Desert Gorge",     27250);
	m = display_pole(cn, flag, page, m, pole3[16], "Lab VI, Desert Gorge",     27250);
	m = display_pole(cn, flag, page, m, pole3[17], "Lab VI, Desert Gorge",     27250);
	m = disp_rb_pole(cn, flag, page, m, poleR[ 6], "Lab VI, Desert Gorge");
	m = display_pole(cn, flag, page, m, pole2[22], "Underground II",           28000);
	m = display_pole(cn, flag, page, m, pole4[16], "Fire Pentagram Quest",     28000);
	m = display_pole(cn, flag, page, m, pole2[23], "Underground II",           30000);
	m = display_pole(cn, flag, page, m, pole2[ 1], "Gargoyle Nest",            30500);
	m = display_pole(cn, flag, page, m, pole1[19], "Abandoned Archives",       31000);
	m = display_pole(cn, flag, page, m, pole2[24], "Underground II",           32000);
	m = display_pole(cn, flag, page, m, pole4[17], "Fire Pentagram Quest",     32000);
	m = display_pole(cn, flag, page, m, pole3[18], "Lab VII, Light/Dark Gor",  33250);
	m = display_pole(cn, flag, page, m, pole3[19], "Lab VII, Light/Dark Gor",  33250);
	m = display_pole(cn, flag, page, m, pole3[20], "Lab VII, Light/Dark Gor",  33250);
	m = disp_rb_pole(cn, flag, page, m, poleR[ 7], "Lab VII, Light/Dark Gor");
	m = display_pole(cn, flag, page, m, pole1[17], "Jagged Pass Cellar",       34500);
	m = display_pole(cn, flag, page, m, pole6[20], "Dimling's Den",            36000);
	m = display_pole(cn, flag, page, m, pole4[18], "Fire Pentagram Quest",     36000);
	m = display_pole(cn, flag, page, m, pole1[14], "Lavender Lakebed",         38500);
	m = display_pole(cn, flag, page, m, pole2[25], "Underground II",           40000);
	m = display_pole(cn, flag, page, m, pole4[19], "Fire Pentagram Quest",     40000);
	m = display_pole(cn, flag, page, m, pole2[ 2], "Gargoyle Nest",            40500);
	m = display_pole(cn, flag, page, m, pole3[21], "Lab VIII, Underwater Go",  42000);
	m = display_pole(cn, flag, page, m, pole3[22], "Lab VIII, Underwater Go",  42000);
	m = display_pole(cn, flag, page, m, pole3[23], "Lab VIII, Underwater Go",  42000);
	m = disp_rb_pole(cn, flag, page, m, poleR[ 8], "Lab VIII, Underwater Go");
	m = display_pole(cn, flag, page, m, pole6[21], "Dimling's Den",            44000);
	m = display_pole(cn, flag, page, m, pole1[12], "Grolm Laboratory",         45000);
	m = display_pole(cn, flag, page, m, pole5[21], "Basalt Desert",            45000);
	m = display_pole(cn, flag, page, m, pole4[20], "Jungle Pentagram Quest",   45000);
	m = display_pole(cn, flag, page, m, pole1[23], "Crumbling Cathedral",      46000);
	m = display_pole(cn, flag, page, m, pole5[ 9], "Old Well",                 48000);
	m = display_pole(cn, flag, page, m, pole2[26], "Underground III",          48000);
	m = display_pole(cn, flag, page, m, pole1[18], "Winding Valley",           49000);
	m = display_pole(cn, flag, page, m, pole2[ 3], "Gargoyle Nest",            50000);
	m = display_pole(cn, flag, page, m, pole2[ 8], "Black Stronghold",         50000);
	m = display_pole(cn, flag, page, m, pole4[21], "Jungle Pentagram Quest",   50000);
	m = display_pole(cn, flag, page, m, pole2[27], "Underground III",          52000);
	m = display_pole(cn, flag, page, m, pole5[ 6], "Scorpion Burrow",          52500);
	m = display_pole(cn, flag, page, m, pole3[24], "Lab IX, Riddle Gorge",     54000);
	m = display_pole(cn, flag, page, m, pole3[25], "Lab IX, Riddle Gorge",     54000);
	m = display_pole(cn, flag, page, m, pole3[26], "Lab IX, Riddle Gorge",     54000);
	m = disp_rb_pole(cn, flag, page, m, poleR[ 9], "Lab IX, Riddle Gorge");
	m = display_pole(cn, flag, page, m, pole4[22], "Jungle Pentagram Quest",   55000);
	m = display_pole(cn, flag, page, m, pole2[28], "Underground III",          56000);
	m = display_pole(cn, flag, page, m, pole1[24], "Skitter Hatchery",         57000);
	m = display_pole(cn, flag, page, m, pole6[22], "Dimling's Den",            60000);
	m = disp_rb_pole(cn, flag, page, m, poleR[25], "Dimling's Den");
	m = display_pole(cn, flag, page, m, pole2[29], "Underground III",          60000);
	m = display_pole(cn, flag, page, m, pole4[23], "Jungle Pentagram Quest",   60000);
	m = display_pole(cn, flag, page, m, pole3[27], "Lab X, Forest Gorge",      63250);
	m = display_pole(cn, flag, page, m, pole3[28], "Lab X, Forest Gorge",      63250);
	m = display_pole(cn, flag, page, m, pole3[29], "Lab X, Forest Gorge",      63250);
	m = disp_rb_pole(cn, flag, page, m, poleR[10], "Lab X, Forest Gorge");
	m = display_pole(cn, flag, page, m, pole5[22], "Violet Bog",               63750);
	m = display_pole(cn, flag, page, m, pole6[18], "Sadem Ridge",              64000);
	m = display_pole(cn, flag, page, m, pole2[30], "Underground III",          64000);
	m = display_pole(cn, flag, page, m, pole1[21], "Southern Swamp",           65000);
	m = display_pole(cn, flag, page, m, pole4[24], "Jungle Pentagram Quest",   66000);
	m = display_pole(cn, flag, page, m, pole1[25], "Thug's Camp",              67000);
	m = display_pole(cn, flag, page, m, pole6[19], "Thug's Hideaway",          69000);
	m = display_pole(cn, flag, page, m, pole4[25], "Ice Pentagram Quest",      72000);
	m = display_pole(cn, flag, page, m, pole4[26], "Ice Pentagram Quest",      78000);
	m = display_pole(cn, flag, page, m, pole3[30], "Lab XI, Seasons Gorge",    78250);
	m = display_pole(cn, flag, page, m, pole3[31], "Lab XI, Seasons Gorge",    78250);
	m = display_pole(cn, flag, page, m, pole4[ 0], "Lab XI, Seasons Gorge",    78250);
	m = disp_rb_pole(cn, flag, page, m, poleR[11], "Lab XI, Seasons Gorge");
	m = display_pole(cn, flag, page, m, pole5[10], "Buried Brush",             79000);
	m = display_pole(cn, flag, page, m, pole2[31], "Underground III",          80000);
	m = display_pole(cn, flag, page, m, pole2[ 4], "Ice Gargoyle Nest",        82000);
	m = display_pole(cn, flag, page, m, pole2[11], "Tower X",                  82500);
	m = display_pole(cn, flag, page, m, pole5[14], "Hollow Trench",            83750);
	m = display_pole(cn, flag, page, m, pole4[27], "Ice Pentagram Quest",      84000);
	m = display_pole(cn, flag, page, m, pole2[ 5], "Ice Gargoyle Nest",        86000);
	m = display_pole(cn, flag, page, m, pole5[ 7], "Scorpion Hive",            87500);
	m = display_pole(cn, flag, page, m, pole1[26], "Lizard Temple",            89000);
	m = display_pole(cn, flag, page, m, pole1[15], "Aston Mines, Level III",   90000);
	m = display_pole(cn, flag, page, m, pole2[ 6], "Ice Gargoyle Nest",        90000);
	m = display_pole(cn, flag, page, m, pole2[ 9], "Stronghold, South",        90000);
	m = display_pole(cn, flag, page, m, pole1[22], "Northern Mountains",       91000);
	m = display_pole(cn, flag, page, m, pole4[ 1], "Lab XII, Ascent Gorge",    91500);
	m = display_pole(cn, flag, page, m, pole4[ 2], "Lab XII, Ascent Gorge",    91500);
	m = display_pole(cn, flag, page, m, pole4[ 3], "Lab XII, Ascent Gorge",    91500);
	m = disp_rb_pole(cn, flag, page, m, poleR[12], "Lab XII, Ascent Gorge");
	m = display_pole(cn, flag, page, m, pole4[28], "Sea Pentagram Quest",      92000);
	m = display_pole(cn, flag, page, m, pole4[29], "Sea Pentagram Quest",     100000);
	m = display_pole(cn, flag, page, m, pole5[ 8], "Outset Den",              105000);
	m = display_pole(cn, flag, page, m, pole1[30], "Onyx Gargoyle Nest",      105000);
//	m = display_pole(cn, flag, page, m, pole5[23], "Northern Tundra",         106250);
	m = display_pole(cn, flag, page, m, pole6[ 0], "Lab XIV, Miner's Gorge",  106875);
	m = display_pole(cn, flag, page, m, pole6[ 1], "Lab XIV, Miner's Gorge",  106875);
	m = display_pole(cn, flag, page, m, pole6[ 2], "Lab XIV, Miner's Gorge",  106875);
	m = disp_rb_pole(cn, flag, page, m, poleR[13], "Lab XIV, Miner's Gorge");
	m = display_pole(cn, flag, page, m, pole4[30], "Onyx Pentagram Quest",    110000);
	m = display_pole(cn, flag, page, m, pole5[18], "The Basalt Ziggurat",     112500);
	m = display_pole(cn, flag, page, m, pole1[31], "Onyx Gargoyle Nest",      120000);
	m = display_pole(cn, flag, page, m, pole4[31], "Onyx Pentagram Quest",    120000);
	m = disp_rb_pole(cn, flag, page, m, poleR[24], "Onyx Pentagram Quest");
	m = display_pole(cn, flag, page, m, pole1[27], "The Emerald Cavern",      121250);
	m = display_pole(cn, flag, page, m, pole1[28], "The Emerald Cavern",      121250);
	m = display_pole(cn, flag, page, m, pole1[20], "Forbidden Tomes",         127500);
	m = display_pole(cn, flag, page, m, pole5[ 1], "The Volcano",             127500);
	m = display_pole(cn, flag, page, m, pole6[ 3], "Lab XV, Vantablack Gorg", 127500);
	m = display_pole(cn, flag, page, m, pole6[ 4], "Lab XV, Vantablack Gorg", 127500);
	m = display_pole(cn, flag, page, m, pole6[ 5], "Lab XV, Vantablack Gorg", 127500);
	m = disp_rb_pole(cn, flag, page, m, poleR[14], "Lab XV, Vantablack Gorg");
	m = display_pole(cn, flag, page, m, pole5[24], "Thaumaterge's Hut",       130000);
	m = display_pole(cn, flag, page, m, pole5[13], "Smuggler's Hovel",        140000);
	m = display_pole(cn, flag, page, m, pole5[25], "Violet Thicket",          142500);
	m = display_pole(cn, flag, page, m, pole6[ 6], "Lab XVI, Pirate's Gorge", 148750);
	m = display_pole(cn, flag, page, m, pole6[ 7], "Lab XVI, Pirate's Gorge", 148750);
	m = display_pole(cn, flag, page, m, pole6[ 8], "Lab XVI, Pirate's Gorge", 148750);
	m = disp_rb_pole(cn, flag, page, m, poleR[15], "Lab XVI, Pirate's Gorge");
	m = display_pole(cn, flag, page, m, pole5[12], "Merlin's Laboratory",     150000);
	m = display_pole(cn, flag, page, m, pole2[12], "Tower XX",                162500);
	m = disp_rb_pole(cn, flag, page, m, poleR[19], "Tower XX");
	m = display_pole(cn, flag, page, m, pole5[20], "Wellspring Chasm",        167500);
	m = display_pole(cn, flag, page, m, pole6[ 9], "Lab XVII, Gargoyle's Go", 168750);
	m = display_pole(cn, flag, page, m, pole6[10], "Lab XVII, Gargoyle's Go", 168750);
	m = display_pole(cn, flag, page, m, pole6[11], "Lab XVII, Gargoyle's Go", 168750);
	m = disp_rb_pole(cn, flag, page, m, poleR[16], "Lab XVII, Gargoyle's Go");
	m = display_pole(cn, flag, page, m, pole1[29], "The Emerald Palace",      170000);
	m = disp_rb_pole(cn, flag, page, m, poleR[22], "The Emerald Palace");
	m = display_pole(cn, flag, page, m, pole5[11], "Platinum Mines II",       175000);
//	m = display_pole(cn, flag, page, m, pole5[26], "Southern Shore",          177500);
	m = display_pole(cn, flag, page, m, pole6[12], "Lab XVIII, Commandment ", 185000);
	m = display_pole(cn, flag, page, m, pole6[13], "Lab XVIII, Commandment ", 185000);
	m = display_pole(cn, flag, page, m, pole6[14], "Lab XVIII, Commandment ", 185000);
	m = disp_rb_pole(cn, flag, page, m, poleR[17], "Lab XVIII, Commandment ");
	m = display_pole(cn, flag, page, m, pole5[19], "The Widow's Nest",        187500);
	m = display_pole(cn, flag, page, m, pole5[15], "Cold Cavern",             192500);
//	m = display_pole(cn, flag, page, m, pole5[27], "Untainted Isle",          195000);
//	m = disp_rb_pole(cn, flag, page, m, poleR[28], "Untainted Isle");
	m = display_pole(cn, flag, page, m, pole6[15], "Lab XIX, Divinity Gorge", 195625);
	m = display_pole(cn, flag, page, m, pole6[16], "Lab XIX, Divinity Gorge", 195625);
	m = display_pole(cn, flag, page, m, pole6[17], "Lab XIX, Divinity Gorge", 195625);
	m = disp_rb_pole(cn, flag, page, m, poleR[18], "Lab XIX, Divinity Gorge");
//	m = display_pole(cn, flag, page, m, pole5[28], "The Aqueduct",            200000);
//	m = disp_rb_pole(cn, flag, page, m, poleR[27], "The Aqueduct");
	m = display_pole(cn, flag, page, m, pole5[16], "Seppuku House",           210000);
	m = display_pole(cn, flag, page, m, pole5[29], "Temple in the Sky",       215000);
	m = disp_rb_pole(cn, flag, page, m, poleR[26], "Temple in the Sky");
	m = display_pole(cn, flag, page, m, pole5[ 0], "Seagrel King's Quarters", 222500);
//	m = display_pole(cn, flag, page, m, pole5[30], "Edge of the World",       230000);
//	m = disp_rb_pole(cn, flag, page, m, poleR[29], "Edge of the World");
	m = display_pole(cn, flag, page, m, pole5[17], "The Obsidian Fortress",   235000);
	m = disp_rb_pole(cn, flag, page, m, poleR[23], "The Obsidian Fortress");
	m = display_pole(cn, flag, page, m, pole2[10], "Black Stronghold Baseme", 240000);
	m = disp_rb_pole(cn, flag, page, m, poleR[21], "Black Stronghold Baseme");
	m = display_pole(cn, flag, page, m, pole2[13], "Abyss X",                 247500);
	m = disp_rb_pole(cn, flag, page, m, poleR[20], "Abyss X");
//	m = display_pole(cn, flag, page, m, pole5[31], "Aemon's Palace",          250000);
//	m = disp_rb_pole(cn, flag, page, m, poleR[30], "Aemon's Palace");
	//
//	m = display_pole(cn, flag, page, m, pole6[23], "", 0);
//	m = display_pole(cn, flag, page, m, pole6[24], "", 0);
//	m = display_pole(cn, flag, page, m, pole6[25], "", 0);
//	m = display_pole(cn, flag, page, m, pole6[26], "", 0);
//	m = display_pole(cn, flag, page, m, pole6[27], "", 0);
//	m = display_pole(cn, flag, page, m, pole6[28], "", 0);
//	m = display_pole(cn, flag, page, m, pole6[29], "", 0);
//	m = display_pole(cn, flag, page, m, pole6[30], "", 0);
//	m = display_pole(cn, flag, page, m, pole6[31], "", 0);
	//
	do_char_log(cn, 1, " \n");
	do_char_log(cn, 2, "Showing page %d of %d. #poles <x> to swap.\n", page, max(1,min(12,(m-1)/16+1)));
	do_char_log(cn, 1, " \n");
}

int display_quest(int cn, int flag, int page, int m, int req, int quest, int rank, char *npcn, char *nloc, char *nrew, int exp)
{
	if (req && (flag || !quest))
	{ 
		m++; 
		if (m<=page*16 && m>(page-1)*16)
		{
			do_char_log(cn, (req==2)?6:(quest?3:1), "%-5.5s  %-8.8s  %-12.12s  %-8.8s  %6d\n", 
				who_rank_name[rank], npcn, nloc, nrew, quest?exp/4:exp);
		}
	}
	return m;
}
void do_questlist(int cn, int flag, char *topic) // flag 1 = all, 0 = unattained
{
	int n, m, t, page = 1, ex = 0;
	int ast = 1, liz = 0, nei = 0, ars = 0, arc = 0, nlb = 0, nrb = 0;
	int spi = 0, sp2 = 0, sco = 0, sc2 = 0, sc3 = 0, gre = 0, bla = 0, hou = 0, dmr = 0;
	int quest1[28]={0};
	int quest2[32]={0}; // ch[cn].data[72]
	int quest3[32]={0}; // ch[cn].data[94]
	int questP[32]={0}; // ch[cn].data[20]
	int questZ[ 6]={0}; // ch[cn].pandium_floor[2]
	
	if (strcmp(topic, "2")==0) page = 2;
	if (strcmp(topic, "3")==0) page = 3;
	if (strcmp(topic, "4")==0) page = 4;
	if (strcmp(topic, "5")==0) page = 5;
	if (strcmp(topic, "6")==0) page = 6;
	if (strcmp(topic, "7")==0) page = 7;
	if (strcmp(topic, "8")==0) page = 8;
	if (strcmp(topic, "9")==0) page = 9;
	
	if (B_SK(cn, SK_ECONOM))															quest1[ 0] = 1;	// ( Jamil )
	if (IS_ANY_MERC(cn) || IS_ANY_HARA(cn) || B_SK(cn, SK_TAUNT))						quest1[ 1] = 1;	// ( Inga )
	if (IS_BRAVER(cn)   || IS_ANY_HARA(cn) || IS_ANY_TEMP(cn) || B_SK(cn, SK_POISON))	quest1[24] = 1;	// ( Inga )
	if (IS_BRAVER(cn)||IS_LYCANTH(cn)||IS_ANY_TEMP(cn)||IS_ANY_MERC(cn)||B_SK(cn, SK_STAFF)) quest1[25] = 1;	// ( Inga )
	if (IS_BRAVER(cn)   || IS_ANY_TEMP(cn) || B_SK(cn, SK_ENHANCE))						quest1[ 2] = 1;	// ( Sirjan )
	if (IS_ANY_MERC(cn)||IS_LYCANTH(cn)|| IS_ANY_HARA(cn)||B_SK(cn, SK_MSHIELD))		quest1[ 3] = 1;	// ( Sirjan )
	if (IS_ANY_MERC(cn) || IS_ANY_HARA(cn) || B_SK(cn, SK_WEAKEN))						quest1[ 4] = 1;	// ( Amity )
	if (IS_BRAVER(cn)   || IS_ANY_TEMP(cn) || B_SK(cn, SK_SLOW))						quest1[ 5] = 1;	// ( Amity )
	if (B_SK(cn, SK_REPAIR))															quest1[ 6] = 1;	// ( Jefferson )
	if (ch[cn].flags & CF_LOCKPICK)														quest1[ 7] = 1;	// ( Steven )
	if (IS_BRAVER(cn)   || IS_ANY_HARA(cn)	|| B_SK(cn, SK_IMMUN))						quest1[ 8] = 1;	// ( Ingrid )
	if (IS_ANY_TEMP(cn)||IS_LYCANTH(cn)|| IS_ANY_MERC(cn)||B_SK(cn, SK_DISPEL))			quest1[ 9] = 1;	// ( Ingrid )
	if (IS_ANY_MERC(cn) || IS_ANY_HARA(cn) || B_SK(cn, SK_SURROUND))					quest1[10] = 1;	// ( Leopold )
	if (IS_BRAVER(cn)   || IS_ANY_TEMP(cn) || B_SK(cn, SK_CURSE))						quest1[11] = 1;	// ( Leopold )
	if (B_SK(cn, SK_HEAL))																quest1[12] = 1;	// ( Gunther )
	if (ch[cn].flags & CF_SENSE)														quest1[13] = 1;	// ( Manfred )
	if (B_SK(cn, SK_RESIST))															quest1[14] = 1;	// ( Serena )
	if (B_SK(cn, SK_BLESS))																quest1[15] = 1;	// ( Cirrus )
	if (B_SK(cn, SK_REST))																quest1[17] = 1;	// ( Gordon )
	if (IS_ANY_HARA(cn)||IS_LYCANTH(cn)||B_SK(cn, SK_SHIELD))							quest1[18] = 1;	// ( Edna )
	if (IS_BRAVER(cn)   || IS_ANY_TEMP(cn) || IS_ANY_MERC(cn) || B_SK(cn, SK_TACTICS))	quest1[19] = 1;	// ( Edna )
	if (!IS_LYCANTH(cn) || B_SK(cn, SK_BLIND))											quest1[26] = 1; // ( Edna )
	if (ch[cn].kindred & KIN_IDENTIFY)													quest1[20] = 1;	// ( Nasir )
	if (ch[cn].temple_x!=HOME_START_X)													quest1[21] = 1;	// Get to Aston
	if (ch[cn].flags & CF_APPRAISE)														quest1[22] = 1; // ( Richie )
	if (B_SK(cn, SK_METABOLISM))														quest1[23] = 1;	// ( Lucci )
	if ((IS_ANY_MERC(cn) || IS_SEYAN_DU(cn)) && B_SK(cn, SK_HASTE))						quest2[16] = 1; // ( Regal )
	if (ch[cn].flags & CF_KNOWSPELL)													quest1[27] = 1; // ( Iggy )
	
	//
	if ((B_SK(cn, SK_WARCRY) 	&& IS_ARCHTEMPLAR(cn)))		quest1[16] = 1;
	if ((B_SK(cn, SK_LEAP) 		&& IS_SKALD(cn)))			quest1[16] = 1;
	if ((B_SK(cn, SK_ZEPHYR) 	&& IS_WARRIOR(cn)))			quest1[16] = 1;
	if ((B_SK(cn, SK_LETHARGY) 	&& IS_SORCERER(cn)))		quest1[16] = 1;
	if ((B_SK(cn, SK_GCMASTERY) && IS_SUMMONER(cn)))		quest1[16] = 1;
	if ((B_SK(cn, SK_PULSE) 	&& IS_ARCHHARAKIM(cn)))		quest1[16] = 1;
	if ((B_SK(cn, SK_FINESSE) 	&& IS_BRAVER(cn)))			quest1[16] = 1;
	if ((B_SK(cn, SK_RAGE) 		&& IS_LYCANTH(cn)))			quest1[16] = 1;
	//
	if (IS_SEYAN_DU(cn))
	{
		n = 0;
		if (B_SK(cn, SK_WARCRY))   n++;		if (B_SK(cn, SK_LEAP))     n++;
		if (B_SK(cn, SK_SHADOW))   n++;		if (B_SK(cn, SK_LETHARGY)) n++;
		if (B_SK(cn, SK_PULSE))    n++;		if (B_SK(cn, SK_ZEPHYR))   n++;
		if (B_SK(cn, SK_FINESSE))  n++;		if (B_SK(cn, SK_RAGE))     n++;		
		if (n>=2)  		quest1[16] = 1;
	}
	//
	for (n=0;n<32;n++)
	{
		if (ch[cn].data[72] & (1 << n)) 			   quest2[n]   = 1;
		if (ch[cn].data[94] & (1 << n)) 			   quest3[n]   = 1;
		if (ch[cn].data[20] >= n && n >=13 && n <= 18) questP[n+1] = 1;
		else if (ch[cn].data[20] >= n) 				   questP[n]   = 1;
	}
	if (IS_ANY_ARCH(cn)) 						 questP[13] = 1;
	if (st_skill_pts_all(ch[cn].tree_points)>=7) questP[20] = 1;
	for (n=0;n<5;n++)
	{
		if (ch[cn].pandium_floor[2]>n) questZ[n] = 1;
	}
	if (ch[cn].pandium_floor[0]>=50 || ch[cn].pandium_floor[1]>=50) questZ[6] = 1;
	
	if (ch[cn].temple_x==HOME_START_X && !quest2[ 0] && !(ch[cn].waypoints&(1<<2))) ast = 0;
	if ((ch[cn].waypoints&(1<<7))     ||  quest2[25]) liz = 1;
	if ((ch[cn].waypoints&(1<<10))    ||  quest3[ 2]) nei = 1;
	if ((ch[cn].waypoints&(1<<26))    ||  questZ[ 0]) arc = 1;
	if (nei && IS_RB(cn)) 							  nrb = 1;
	else if (nei)									  nlb = 1;
	if (getrank(cn)>17) dmr = 1;
	
	if (getrank(cn)< 7 && !ast) spi = 2;
	if (getrank(cn)< 7 &&  ast) sp2 = 2;
	if (getrank(cn)> 6 && getrank(cn)<11) sco = 2;
	if (getrank(cn)>11 && getrank(cn)<17) gre = 2;
	if (getrank(cn)>10 && getrank(cn)<14) sc2 = 2;
	if (getrank(cn)>13 && getrank(cn)<17) sc3 = 2;
	if (getrank(cn)>16) hou = 2;
	if (getrank(cn)>16) bla = 2;
	
	do_char_log(cn, 1, "Now listing available quests (PAGE %d):\n", page);
	do_char_log(cn, 1, " \n");
	
	n = (page-1)*16;
	t = page*16;
	m = 0;
	//
	m = display_quest(cn, flag, page, m,   1, quest1[ 0],  0, "Jamil",    "Bluebird Tav", "*Econimi",    100);
	m = display_quest(cn, flag, page, m,   1, quest1[ 1],  1, "Inga",     "First Street", "*Taunt",      200);
	m = display_quest(cn, flag, page, m,   1, quest1[24],  1, "Inga",     "First Street", "*Poison",     200);
	m = display_quest(cn, flag, page, m,   1, quest1[25],  1, "Inga",     "First Street", "*Staff",      200);
	m = display_quest(cn, flag, page, m,   1, quest1[ 2],  1, "Sirjan",   "First Street", "*Enhance",    300);
	m = display_quest(cn, flag, page, m,   1, quest1[ 3],  1, "Sirjan",   "First Street", "*Magic S",    300);
	m = display_quest(cn, flag, page, m,   1, quest1[ 4],  2, "Amity",    "Lynbore Libr", "*Weaken",     450);
	m = display_quest(cn, flag, page, m,   1, quest1[ 5],  2, "Amity",    "Lynbore Libr", "*Slow",       450);
	m = display_quest(cn, flag, page, m,   1, quest1[ 6],  2, "Jefferso", "Second Stree", "*Repair",     600);
	m = display_quest(cn, flag, page, m,   1, quest1[ 7],  2, "Steven",   "Second Stree", "*Lockpic",    800);
	m = display_quest(cn, flag, page, m,   1, quest1[ 8],  3, "Ingrid",   "Castle Way",   "*Immunit",   1000);
	m = display_quest(cn, flag, page, m,   1, quest1[ 9],  3, "Ingrid",   "Castle Way",   "*Dispel",    1000);
	m = display_quest(cn, flag, page, m,   1, quest1[10],  3, "Leopold",  "Castle Way",   "*Surroun",   1250);
	m = display_quest(cn, flag, page, m,   1, quest1[11],  3, "Leopold",  "Castle Way",   "*Curse",     1250);
	m = display_quest(cn, flag, page, m,   1, quest1[12],  3, "Gunther",  "Castle Way",   "*Heal",      1500);
	m = display_quest(cn, flag, page, m,   1, quest1[13],  3, "Manfred",  "Silver Avenu", "*Sense M",   1800);
	m = display_quest(cn, flag, page, m,   1, quest1[14],  4, "Serena",   "Second Stree", "*Resista",   2100);
	m = display_quest(cn, flag, page, m,   1, quest1[15],  4, "Cirrus",   "Bluebird Tav", "*Bless",     2450);
	m = display_quest(cn, flag, page, m,   1, quest1[17],  4, "Gordon",   "Shore Cresce", "*Rest",      2800);
	m = display_quest(cn, flag, page, m,   1, quest1[18],  4, "Edna",     "Shore Cresce", "*Shield",    3300);
	m = display_quest(cn, flag, page, m,   1, quest1[19],  4, "Edna",     "Shore Cresce", "*Tactics",   3300);
	m = display_quest(cn, flag, page, m,   1, quest1[26],  4, "Edna",     "Shore Cresce", "*Blind",     3300);
	m = display_quest(cn, flag, page, m,   1, quest1[20],  4, "Nasir",    "Shore Cresce", "*Identif",   3800);
	m = display_quest(cn, flag, page, m, spi,          0,  4, "Garna",    "First Street", "@Gold",       125);
	m = display_quest(cn, flag, page, m,   1, quest1[21],  5, "Guard Ca", "South Aston",  "Gold",       8000);
	//
	m = display_quest(cn, flag, page, m,!ast, quest2[ 0],  5, "???",      "Aston",        "???",        5750);
	//
	m = display_quest(cn, flag, page, m, sp2,          0,  4, "Gene",     "South End",    "@Gold",       125);
	
	m = display_quest(cn, flag, page, m, ast, quest2[ 0],  5, "April",    "Marble Lane",  "Potion",     5750);
	m = display_quest(cn, flag, page, m, ast, quest1[22],  5, "Richie",   "Marble Lane",  "*Apprais",   6250);
	m = display_quest(cn, flag, page, m, ast, quest2[ 1],  5, "Alphonse", "Rose Street",  "Ring",       7000);
	
	m = display_quest(cn, flag, page, m, ast, questP[ 1],  6, "Lab I",    "Grolm Gorge",  "-",         19500);
	m = display_quest(cn, flag, page, m, ast, quest3[11],  6, "Ratling",  "Underground",  "Gem",       10000); // #144
	m = display_quest(cn, flag, page, m, ast, questP[ 2],  6, "Lab II",   "Lizard Gorge", "-",         28000);
	
	m = display_quest(cn, flag, page, m, ast, quest2[11],  7, "Robin",    "South End",    "Potion",    10000);
	m = display_quest(cn, flag, page, m, ast, quest2[ 2],  7, "Cherri",   "Merchant's W", "Amulet",    14500);
	m = display_quest(cn, flag, page, m, ast, questP[ 3],  7, "Lab III",  "Undead Gorge", "-",         38500);
	m = display_quest(cn, flag, page, m, ast, quest2[13],  7, "Oscar",    "Temple Stree", "Amulet",    16000);
	m = display_quest(cn, flag, page, m, ast, quest2[ 6],  7, "Elric",    "Merchant's W", "Potion",    17000);
	
	m = display_quest(cn, flag, page, m, ast, questP[ 4],  8, "Lab IV",   "Caster Gorge", "-",         53500);
	m = display_quest(cn, flag, page, m, ast, quest2[ 4],  8, "Bradley",  "Astonia Peni", "Belt",      20500);
	m = display_quest(cn, flag, page, m, ast, quest2[ 3],  8, "Rocky",    "Merchant's W", "Ring",      21500);
	m = display_quest(cn, flag, page, m, ast, questP[ 5],  8, "Lab V",    "Knight Gorge", "-",         66000);
	m = display_quest(cn, flag, page, m, ast, quest2[14],  8, "Castor",   "Marble Lane",  "Book",      25000);
	
	m = display_quest(cn, flag, page, m, ast, quest2[15],  9, "Grover",   "Marble Lane",  "Amulet",    27500);
	m = display_quest(cn, flag, page, m, ast, questP[ 6],  9, "Lab VI",   "Desert Gorge", "-",         87000);
	m = display_quest(cn, flag, page, m, sco,          0,  9, "Faiza",    "Temple Stree", "@Gold",       750);
	m = display_quest(cn, flag, page, m, ast, quest1[23],  9, "Lucci",    "Temple Stree", "*Metabol",  30500);
	m = display_quest(cn, flag, page, m, ast, questP[ 7],  9, "Lab VII",  "Light/Dark G", "-",        106500);
	
	m = display_quest(cn, flag, page, m, ast, quest2[ 5], 10, "Roxie",    "Temple Stree", "Amulet",    36000);
	m = display_quest(cn, flag, page, m, ast, quest2[22], 10, "Ludolf",   "Aston Farms",  "Tarot",     37000);
	m = display_quest(cn, flag, page, m, ast, quest2[ 9], 10, "Gomez",    "Temple Stree", "Tarot",     37500);
	m = display_quest(cn, flag, page, m, ast, questP[ 8], 10, "Lab VIII", "Underwater G", "-",        135000);
	m = display_quest(cn, flag, page, m, ast, quest3[ 0], 10, "Oswald",   "Aston Farms",  "Amulet",    38000);
	if (IS_ANY_MERC(cn) || IS_SEYAN_DU(cn))
		m = display_quest(cn, flag, page, m, ast, quest2[16], 10, "Regal","Winding Vall", "*Haste",    40000);
	else
		m = display_quest(cn, flag, page, m, ast, quest2[16], 10, "Regal","Winding Vall", "Book",      40000);
	
	m = display_quest(cn, flag, page, m, ast, questP[ 9], 11, "Lab IX",   "Riddle Gorge", "-",        173000);
	m = display_quest(cn, flag, page, m, sc2,          0, 11, "Faiza",    "Temple Stree", "@Gold",      1000);
	m = display_quest(cn, flag, page, m, ast, quest2[10], 11, "Donna",    "Merchant's W", "Tarot",     42500);
	m = display_quest(cn, flag, page, m, ast, quest2[23], 11, "Flanders", "Aston Farms",  "Tarot",     45000);
	m = display_quest(cn, flag, page, m, ast, questP[10], 11, "Lab X",    "Forest Gorge", "-",        202000);
	m = display_quest(cn, flag, page, m, ast, quest3[12], 11, "WesternW", "West Barrack", "Tarot",     47000); // #145 Rescue Malte

	m = display_quest(cn, flag, page, m, ast, quest2[ 8], 12, "Rufus",    "Temple Stree", "Tarot",     50000);
	m = display_quest(cn, flag, page, m, ast, questP[11], 12, "Lab XI",   "Seasons Gorg", "-",        251000);
	m = display_quest(cn, flag, page, m, ast, quest3[13], 12, "EasternW", "East Barrack", "Gold",      51000); // #146 Sadem Ridge
	m = display_quest(cn, flag, page, m, ast, quest2[ 7], 12, "Marline",  "Marble Lane",  "Tarot",     52000);
	m = display_quest(cn, flag, page, m, ast, quest2[24], 12, "Topham",   "Temple Stree", "Gem",       62500);
	m = display_quest(cn, flag, page, m, ast, quest3[14], 12, "Superint", "South Aston",  "Gold",      55000); // #147 Thug Hideaway
	m = display_quest(cn, flag, page, m, gre,          0, 12, "Rose",     "Blue Ogre Ta", "@Gold",     10000);
	
	m = display_quest(cn, flag, page, m, ast, quest2[12], 13, "Monica",   "South End",    "Potion",    48000);
	m = display_quest(cn, flag, page, m, ast, quest2[17], 13, "Shera",    "Bulwark Aven", "Cloak",     62000);
	m = display_quest(cn, flag, page, m, ast, quest3[ 1], 13, "Maude",    "Aston Farms",  "Gold",      63000);
	m = display_quest(cn, flag, page, m, ast, questP[12], 13, "Lab XII",  "Ascent Gorge", "-",        293000);
	m = display_quest(cn, flag, page, m, ast, questP[13], 13, "Lab XIII", "Gatekeeper's", "+Arch",         0);
	m = display_quest(cn, flag, page, m, ast, quest2[18], 13, "Tacticia", "West Gate",    "Ring",      66000);
	
	m = display_quest(cn, flag, page, m, ast, quest3[15], 14, "Diplomat", "East Gate",    "Belt",      71000); // #148 Lizard Temple
	//
	ars = (ast && (IS_ARCHTEMPLAR(cn) || (IS_SEYAN_DU(cn) && !quest1[16] && !B_SK(cn, SK_WARCRY))));
	m = display_quest(cn, flag, page, m, ars, quest1[16], 14, "Jamie",    "Bulwark Aven", "*Warcry",   73000);
	ars = (ast && (IS_SKALD(cn) || (IS_SEYAN_DU(cn) && !quest1[16] && !B_SK(cn, SK_LEAP))));
	m = display_quest(cn, flag, page, m, ars, quest1[16], 14, "Ellis",    "Bulwark Aven", "*Leap",     73000);
	ars = (ast && (IS_SUMMONER(cn) || (IS_SEYAN_DU(cn) && !quest1[16] && !B_SK(cn, SK_GCMASTERY))));
	m = display_quest(cn, flag, page, m, ars, quest1[16], 14, "Roger",    "Bulwark Aven", "*Compani",  73000);
	ars = (ast && (IS_SORCERER(cn) || (IS_SEYAN_DU(cn) && !quest1[16] && !B_SK(cn, SK_LETHARGY))));
	m = display_quest(cn, flag, page, m, ars, quest1[16], 14, "Boris",    "Bulwark Aven", "*Letharg",  73000);
	ars = (ast && (IS_ARCHHARAKIM(cn) || (IS_SEYAN_DU(cn) && !quest1[16] && !B_SK(cn, SK_PULSE))));
	m = display_quest(cn, flag, page, m, ars, quest1[16], 14, "Kaleigh",  "Bulwark Aven", "*Pulse",    73000);
	ars = (ast && (IS_WARRIOR(cn) || (IS_SEYAN_DU(cn) && !quest1[16] && !B_SK(cn, SK_ZEPHYR))));
	m = display_quest(cn, flag, page, m, ars, quest1[16], 14, "Terri",    "Bulwark Aven", "*Zephyr",   73000);
	ars = (ast && (IS_BRAVER(cn) || (IS_SEYAN_DU(cn) && !quest1[16] && !B_SK(cn, SK_FINESSE))));
	m = display_quest(cn, flag, page, m, ars, quest1[16], 14, "Sierra",   "Bulwark Aven", "*Finesse",  73000);
	ars = (ast && (IS_LYCANTH(cn) || (IS_SEYAN_DU(cn) && !quest1[16] && !B_SK(cn, SK_RAGE))));
	m = display_quest(cn, flag, page, m, ars, quest1[16], 14, "Mystery",  "Bulwark Aven", "*Rage",     73000);
	m = display_quest(cn, flag, page, m, ast, quest1[27], 14, "Iggy",     "Bulwark Aven", "*Spellkn",  74000);
	m = display_quest(cn, flag, page, m,!nei, quest3[16], 14, "???",      "Neiseer",      "???",       75000); // #149 Neiseer Park / Dimling Den
	m = display_quest(cn, flag, page, m, nei, quest3[16], 14, "Vincent",  "Turtle Road",  "Tarot",     75000); // #149 Neiseer Park / Dimling Den
	m = display_quest(cn, flag, page, m, ast, quest2[21], 14, "Aster",    "Aston Farms",  "Tarot",     78000);
	//
	m = display_quest(cn, flag, page, m,!liz, quest2[25], 15, "???",      "Beryl Jungle", "???",       80000);
	m = display_quest(cn, flag, page, m, liz, quest2[26], 15, "Tsulu",    "Settlement",   "Amulet",    80000);
	m = display_quest(cn, flag, page, m, nei, quest3[ 2], 15, "Brenna",   "Victory Road", "Tarot",     85000); // #135
	m = display_quest(cn, flag, page, m, sc3,          0, 15, "Faiza",    "Temple Stree", "@Gold",      1875);
	m = display_quest(cn, flag, page, m, nei, quest3[ 8], 15, "Rikus",    "Victory Road", "Tarot",     90000); // #141
	
	m = display_quest(cn, flag, page, m, liz, quest2[27], 16, "Shafira",  "Settlement",   "Amulet",    96250);
	m = display_quest(cn, flag, page, m, liz, quest2[25], 16, "Navarre",  "Settlement",   "Amulet",    96250);
	m = display_quest(cn, flag, page, m, liz, quest2[31], 16, "Vora",     "Settlement",   "Belt",     102500);
	m = display_quest(cn, flag, page, m, liz, quest2[30], 16, "Makira",   "Settlement",   "Belt",     102500);
	m = display_quest(cn, flag, page, m, nlb, questP[14], 16, "Lab XIV",  "Miner's Gorg", "+1 SP",    340000);
	m = display_quest(cn, flag, page, m, nrb, questP[14], 16, "Lab XIV",  "Miner's Gorg", "-",        340000);
	m = display_quest(cn, flag, page, m, nei, quest3[17], 16, "Jabilo",   "Warlock Way",  "Tarot",    105000); // #150 Thaumaterge

	m = display_quest(cn, flag, page, m, hou,          0, 17, "Dwyn",     "Titan Street", "@Gold",      5000);
	m = display_quest(cn, flag, page, m, bla,          0, 17, "Zorani",   "Settlement",   "@Gold",    120000);
	m = display_quest(cn, flag, page, m, nei, quest3[ 4], 17, "Jasper",   "Ravaged Prai", "Tarot",    112500); // #137
	m = display_quest(cn, flag, page, m, nei, quest3[18], 17, "Caine",    "Warlock Way",  "Scroll",   115000); // #151 Violet Thicket
	m = display_quest(cn, flag, page, m, ast, quest2[19], 17, "Danica",   "Warlock Way",  "Potion",    92000);
	m = display_quest(cn, flag, page, m, nlb, questP[15], 17, "Lab XV",   "Vantablack G", "+1 SP",    407500);
	m = display_quest(cn, flag, page, m, nrb, questP[15], 17, "Lab XV",   "Vantablack G", "-",        407500);
	m = display_quest(cn, flag, page, m, nei, quest3[ 3], 17, "Wicker",   "Warlock Way",  "Scroll",   120000); // #136

	m = display_quest(cn, flag, page, m, nei, quest2[20], 18, "Blanche",  "Turtle Road",  "Cloak",    127500);
	m = display_quest(cn, flag, page, m, nei, quest3[10], 18, "Marco",    "Warlock Way",  "Tarot",    132500); // #143
	m = display_quest(cn, flag, page, m, liz, quest2[28], 18, "Dracus",   "Settlement",   "Amulet",   135000);
	m = display_quest(cn, flag, page, m, nlb, questP[16], 18, "Lab XVI",  "Pirate's Gor", "+1 SP",    475000);
	m = display_quest(cn, flag, page, m, nrb, questP[16], 18, "Lab XVI",  "Pirate's Gor", "-",        475000);
//	m = display_quest(cn, flag, page, m, nei, quest3[19], 18, "", "", "", 142500); // #152 Southern Shore

	m = display_quest(cn, flag, page, m, nei, quest3[ 9], 19, "Charlotte","Warlock Way",  "Tarot",    150000); // #142
	m = display_quest(cn, flag, page, m, nei, quest3[ 5], 19, "Soyala",   "Last Avenue",  "Tarot",    155000); // #138
//	m = display_quest(cn, flag, page, m, nei, quest3[20], 19, "", "", "", 155000); // #153 Untainted Island
	m = display_quest(cn, flag, page, m, nlb, questP[17], 19, "Lab XVII", "Gargoyle's G", "+1 SP",    540000);
	m = display_quest(cn, flag, page, m, nrb, questP[17], 19, "Lab XVII", "Gargoyle's G", "-",        540000);
//	m = display_quest(cn, flag, page, m, nei, quest3[21], 19, "", "", "", 160000); // #154 The Aqueduct

	m = display_quest(cn, flag, page, m, nei, quest3[ 6], 20, "Runa",     "Last Avenue",  "Tarot",    168750); // #139
	m = display_quest(cn, flag, page, m, nei, quest3[ 7], 20, "Zephan",   "Last Avenue",  "Tarot",    168750); // #140
	m = display_quest(cn, flag, page, m, nei, quest3[23], 20, "Brye",     "Warlock Way",  "Scroll",   172500); // #156 Temple in the Sky
	m = display_quest(cn, flag, page, m, nlb, questP[18], 20, "Lab XVIII","Commandment ", "+1 SP",    592500);
	m = display_quest(cn, flag, page, m, nrb, questP[18], 20, "Lab XVIII","Commandment ", "-",        592500);
	m = display_quest(cn, flag, page, m, nei, quest3[24], 20, "Hamako",   "Titan Street", "Scroll",   177500); // #157 Seagrel King

	m = display_quest(cn, flag, page, m, liz, quest2[29], 21, "Rassa",    "Settlement",   "Cloak",    180000);
//	m = display_quest(cn, flag, page, m, nei, quest3[25], 21, "", "", "", 185000); // #158 Edge of the World
	m = display_quest(cn, flag, page, m, nlb, questP[19], 21, "Lab XIX",  "Divinity Gor", "+1 SP",    625000);
	m = display_quest(cn, flag, page, m, nrb, questP[19], 21, "Lab XIX",  "Divinity Gor", "-",        625000);
	m = display_quest(cn, flag, page, m, dmr, quest3[22], 21, "Damor",    "Damor's Magi", "Ring",     190000); // #155 Damor / Shiva 2

	m = display_quest(cn, flag, page, m, nlb, questP[20], 22, "Lab XX",   "Final Gorge",  "+1 SP",         0);
//	m = display_quest(cn, flag, page, m, nei, quest3[26], 22, "", "", "", 200000); // #159 Aemon's Palace
	//
	m = display_quest(cn, flag, page, m,!arc,        questZ[ 0], 23, "???",      "Burning Plai", "???",    0);
	//
	m = display_quest(cn, flag, page, m, arc,        questZ[ 0], 23, "Pandium",  "The Archon", "Depth  1", 0);
	m = display_quest(cn, flag, page, m, questZ[ 0], questZ[ 1], 23, "Pandium",  "The Archon", "Depth 10", 0);
	m = display_quest(cn, flag, page, m, questZ[ 1], questZ[ 2], 23, "Pandium",  "The Archon", "Depth 20", 0);
	m = display_quest(cn, flag, page, m, questZ[ 2], questZ[ 3], 24, "Pandium",  "The Archon", "Depth 30", 0);
	m = display_quest(cn, flag, page, m, questZ[ 3], questZ[ 4], 24, "Pandium",  "The Archon", "Depth 40", 0);
	m = display_quest(cn, flag, page, m, questZ[ 4], questZ[ 5], 24, "Pandium",  "The Archon", "Depth 50", 0);
	//
//	m = display_quest(cn, flag, page, m, , quest3[27], 0, "", "", "", 0); // #160 
//	m = display_quest(cn, flag, page, m, , quest3[28], 0, "", "", "", 0); // #161 
//	m = display_quest(cn, flag, page, m, , quest3[29], 0, "", "", "", 0); // #162 
//	m = display_quest(cn, flag, page, m, , quest3[30], 0, "", "", "", 0); // #163 
//	m = display_quest(cn, flag, page, m, , quest3[31], 0, "", "", "", 0); // #164 
	//
	do_char_log(cn, 1, " \n");
	do_char_log(cn, 2, "Showing page %d of %d. #quest <x> to swap.\n", page, max(1,min(8,(m-1)/16+1)));
	do_char_log(cn, 1, " \n");
}

void do_showchars(int cn)
{
	int n, m, co, j = 0;
	
	do_char_log(cn, 1, "Now listing characters from this PC:\n");
	do_char_log(cn, 1, " \n");
	//                 "!        .         .         .         .        !"
	do_char_log(cn, 1, "Character      Rank                  Total EXP \n");
	
	for (co = 1; co<MAXCHARS; co++)
	{
		if (ch[co].used==USE_EMPTY || !IS_SANEPLAYER(co)) continue;
		for (n = 80; n<89; n++)
		{
			if (ch[cn].data[n]==0) continue;
			for (m = 80; m<89; m++)
			{
				if (ch[co].data[m]==0) continue;
				if (ch[cn].data[n]==ch[co].data[m])
				{
					do_char_log(cn, 1, "%12s   %19s   %10d\n", ch[co].name, rank_name[getrank(co)], ch[co].points_tot);
					j = 1;
					break;
				}
			}
			if (j==1)
			{
				j = 0;
				break;
			}
		}
	}
	
	do_char_log(cn, 1, " \n");
}

int do_showbuffs(int cn, int co)
{
	int n, in, flag = 0;
	for (n = 0; n<MAXBUFFS; n++)
	{
		if ((in = ch[co].spell[n])!=0)
		{
			flag = 1;
			if (bu[in].temp == SK_MSHIELD || bu[in].temp == SK_MSHELL)
				do_char_log(cn, 1, "%s power of %d / %d\n", bu[in].name, bu[in].power, bu[in].duration / 256);
			else if (bu[in].temp == SK_HEAL)
				do_char_log(cn, 1, "%s power of %d (x%d) for %dm %ds\n", bu[in].name, bu[in].power, bu[in].data[1], bu[in].active / (TICKS * 60), (bu[in].active / TICKS) % 60);
			else if (bu[in].temp == SK_VENOM || bu[in].temp == SK_SHOCK || bu[in].temp == SK_CHARGE || bu[in].temp == SK_ZEPHYR2)
				do_char_log(cn, 1, "%s power of %d (x%d) for %dm %ds\n", bu[in].name, bu[in].power, bu[in].value, bu[in].active / (TICKS * 60), (bu[in].active / TICKS) % 60);
			else if (bu[in].flags & IF_PERMSPELL)
				do_char_log(cn, 1, "%s power of %d\n", bu[in].name, bu[in].power);
			else
				do_char_log(cn, 1, "%s power of %d for %dm %ds\n", bu[in].name, bu[in].power, bu[in].active / (TICKS * 60), (bu[in].active / TICKS) % 60);
			if ((ch[cn].flags & CF_KNOWSPELL) && !(ch[cn].flags & CF_KNOW_OFF))
			{
				switch (bu[in].temp)
				{
					case SK_LIGHT:
						do_char_log(cn, 6, " : %+d Glow\n", bu[in].light[1]); 
						break;
					case SK_MSHIELD: 
					case SK_PROTECT:
					case SK_WEAKEN2:
						do_char_log(cn, 6, " : %+d Armor Value\n", bu[in].armor[1]);
						break;
					case SK_ENHANCE: 
					case SK_WEAKEN:
						do_char_log(cn, 6, " : %+d Weapon Value\n", bu[in].weapon[1]);
						break;
					case SK_SLOW:
					case SK_SLOW2:
					case SK_HASTE:
						if (bu[in].speed[1])      do_char_log(cn, 6, " : %+d Speed\n",        bu[in].speed[1]);
						if (bu[in].atk_speed[1])  do_char_log(cn, 6, " : %+d Attack Speed\n", bu[in].atk_speed[1]);
						if (bu[in].cast_speed[1]) do_char_log(cn, 6, " : %+d Cast Speed\n",   bu[in].cast_speed[1]);
						break;
					case SK_CURSE:
					case SK_CURSE2:
					case SK_BLESS:
					case SK_WARCRY:
						do_char_log(cn, 6, " : %+d Braveness\n",  bu[in].attrib[0][1]); 
						do_char_log(cn, 6, " : %+d Willpower\n",  bu[in].attrib[1][1]); 
						do_char_log(cn, 6, " : %+d Intuition\n",  bu[in].attrib[2][1]); 
						do_char_log(cn, 6, " : %+d Agility\n",    bu[in].attrib[3][1]); 
						do_char_log(cn, 6, " : %+d Strength\n",   bu[in].attrib[4][1]);
						break;
					case SK_REGEN: // bu[in].hp[0]
						do_char_log(cn, 6, " : %+d.%02d HP/sec\n", bu[in].hp[0]/50, bu[in].hp[0]%50); break;
					case SK_ARIA:
					case SK_ARIA2:
						do_char_log(cn, 6, " : %+d Cooldown Rate\n", bu[in].cool_bonus[1] * (bu[in].temp==SK_ARIA?2:1));
						if (bu[in].dmg_bonus[1])
							do_char_log(cn, 6, " : %+d.%02d%% More Damage Dealt\n", abs(bu[in].dmg_bonus[1]/2), abs(100*bu[in].dmg_bonus[1]/2)%100);
						if (bu[in].weapon[1])
							do_char_log(cn, 6, " : %+d Weapon Value\n", bu[in].weapon[1]);
						if (bu[in].armor[1])
							do_char_log(cn, 6, " : %+d Armor Value\n", bu[in].armor[1]);
						break;
					case SK_WARCRY3:
					case SK_BLIND:
						do_char_log(cn, 6, " : %+d Hit Score\n",   bu[in].to_hit[1]); 
						do_char_log(cn, 6, " : %+d Parry Score\n", bu[in].to_parry[1]); 
						if (bu[in].skill[SK_PERCEPT][1])
							do_char_log(cn, 6, " : %+d Perception\n",  bu[in].skill[SK_PERCEPT][1]);
						break;
					case SK_DOUSE:
						do_char_log(cn, 6, " : %+d Spell Modifier\n", bu[in].spell_mod[1]);
						do_char_log(cn, 6, " : %+d Stealth\n",        bu[in].skill[SK_STEALTH][1]);
						break;
					case SK_MSHELL:
						do_char_log(cn, 6, " : %+d Resistance\n", bu[in].skill[SK_RESIST][1]);
						do_char_log(cn, 6, " : %+d Immunity\n",   bu[in].skill[SK_IMMUN][1]);
						break;
					case SK_GUARD:
					case SK_AGGRAVATE:
					case SK_SCORCH:
					case SK_SHOCK:
					case SK_CHARGE:
						if (bu[in].dmg_reduction[1]>0)
							do_char_log(cn, 6, " : %+d.%02d%% Less Damage Taken\n", abs(bu[in].dmg_reduction[1]/2), abs(100*bu[in].dmg_reduction[1]/2)%100);
						if (bu[in].dmg_reduction[1]<0)
							do_char_log(cn, 6, " : %+d.%02d%% More Damage Taken\n", abs(bu[in].dmg_reduction[1]/2), abs(100*bu[in].dmg_reduction[1]/2)%100);
						if (bu[in].dmg_bonus[1]>0) 
							do_char_log(cn, 6, " : %+d.%02d%% More Damage Dealt\n", abs(bu[in].dmg_bonus[1]/2), abs(100*bu[in].dmg_bonus[1]/2)%100);
						if (bu[in].dmg_bonus[1]<0) 
							do_char_log(cn, 6, " : %+d.%02d%% Less Damage Dealt\n", abs(bu[in].dmg_bonus[1]/2), abs(100*bu[in].dmg_bonus[1]/2)%100);
						break;
					case SK_LETHARGY:
						if (bu[in].data[2])
							do_char_log(cn, 6, " : %+d Res&Imm Piercing\n", bu[in].power/4);
						else
							do_char_log(cn, 6, " : %+d Res&Imm Piercing\n", bu[in].power/3);
						break;
					case SK_RAGE:
						do_char_log(cn, 6, " : %+d Top Damage\n", bu[in].top_damage[1]);
						do_char_log(cn, 6, " : +%d.%02d%% DoT Dealt\n", 100*(2000+bu[in].data[4])/2000, abs(10000*(2000+bu[in].data[4])/2000)%100); break;
					case SK_CALM:
						do_char_log(cn, 6, " : %+d Top Damage Taken\n", bu[in].data[3]*-1);
						do_char_log(cn, 6, " : %d.%02d%% DoT Taken\n", 100*(2000-bu[in].data[4])/2000, abs(10000*(2000-bu[in].data[4])/2000)%100); break;
					case 254: // R/G/S Essence
						do_char_log(cn, 6, " : %+d to each attribute\n",  bu[in].attrib[0][1]);
						if (bu[in].skill[SK_GEARMAST][1])
							do_char_log(cn, 6, " : %+d to some skills\n", bu[in].skill[SK_GEARMAST][1]);
						break;
					case SK_WARCRY2:
						do_char_log(cn, 6, " : Stunned!\n");
						break;
					case SK_TAUNT:
						do_char_log(cn, 6, " : Taunted!\n");
						break;
					case SK_POISON:
					case SK_BLEED:
					case SK_PLAGUE:
					case SK_VENOM:
						break;
					default: break;
				}
			}
		}
	}
	return flag;
}

void do_listbuffs(int cn, int co)
{
	if (cn!=co) do_char_log(cn, 1, "Your companion's active buffs and debuffs:\n");
	else        do_char_log(cn, 1, "Your active buffs and debuffs:\n");
	do_char_log(cn, 1, " \n");
	if (!do_showbuffs(cn, co))
	{
		do_char_log(cn, 1, "None.\n");
	}
	do_char_log(cn, 1, " \n");
}

void do_listgcbuffs(int cn, int shadow)
{
	int co=0, n, in, flag = 0, m = PCD_COMPANION;
	
	if (shadow) m = PCD_SHADOWCOPY;
	if (co = ch[cn].data[m])
	{
		if (!IS_SANECHAR(co) || ch[co].data[CHD_MASTER]!=cn || (ch[co].flags & CF_BODY) || ch[co].used==USE_EMPTY)
		{
			co = 0;
		}
	}
	if (!co)
	{
		do_char_log(cn, 0, "You must summon a new companion first.\n");
		return;
	}
	do_listbuffs(cn, co);
}

void do_seeskills(int cn, int co)
{
	int n, m;
	
	if (co<=0 || co>=MAXCHARS || !IS_SANECHAR(co))
	{
		do_char_log(cn, 0, "Error: Bad ID.\n");
		return;
	}
	
	do_char_log(cn, 1, "Allocated Skills : %s\n", ch[co].name);
	do_char_log(cn, 1, "-----------------------------------\n");
	
	if (IS_SEYAN_DU(co))         m = 0;
	else if (IS_ARCHTEMPLAR(co)) m = 1;
	else if (IS_SKALD(co))       m = 2;
	else if (IS_WARRIOR(co))     m = 3;
	else if (IS_SORCERER(co))    m = 4;
	else if (IS_SUMMONER(co))    m = 5;
	else if (IS_ARCHHARAKIM(co)) m = 6;
	else                         m = 7;
	
	for (n=0; n<12; n++)
	{
		if (T_SK(co, n+1)) do_char_log(cn, 1, "%d : %s\n", n+1, sk_tree[m][n].name);
	}
}

void do_showcontract(int cn)
{
	int x, y, in, maploc;
	
	maploc = CONT_NUM(cn);
	
	if (!maploc) return;
	
	x = MM_TARGETX - 10;
	y = MM_TARGETY + MM_TARG_OF*(maploc-1);
	
	in = map[x + y * MAPX].it;
	
	if (in && it[in].driver == 113)
	{
		look_contract(cn, in, 0);
		do_char_log(cn, 1, "Progress: %d out of %d.\n", CONT_PROG(cn), max(1, CONT_GOAL(cn)));
	}
}

void do_swap_chars(int cn)
{
	int m, co, cn_x, cn_y, co_x, co_y;
	
	switch(ch[cn].dir)
	{
		case DX_RIGHT: 	m = (ch[cn].x + 1) + (ch[cn].y    ) * MAPX;	break;
		case DX_LEFT: 	m = (ch[cn].x - 1) + (ch[cn].y    ) * MAPX;	break;
		case DX_UP: 	m = (ch[cn].x    ) + (ch[cn].y - 1) * MAPX;	break;
		case DX_DOWN: 	m = (ch[cn].x    ) + (ch[cn].y + 1) * MAPX;	break;
		default: return;
	}
	
	co = map[m].ch;
	
	if (!IS_SANECHAR(co))
	{
		do_char_log(cn, 1, "You must be facing someone to swap places with them.\n");
		return;
	}
	if (!IS_PLAYER(co) && !IS_PLAYER_COMP(co))
	{
		do_char_log(cn, 1, "You must be facing a player or companion.\n");
		return;
	}
	if (IS_GOD(co))
	{
		do_char_log(cn, 1, "You try, and fail, to swap places with a god.\n");
		return;
	}
	
	/*
	if (ch[cn].goto_x || ch[cn].goto_y)
	{
		do_char_log(cn, 1, "You can't do that while moving!\n");
		return;
	}
	if (ch[co].goto_x || ch[co].goto_y)
	{
		do_char_log(cn, 1, "You can't do that while your target is moving!\n");
		return;
	}
	*/
	
	if (ch[cn].attack_cn)
	{
		do_char_log(cn, 1, "You can't do that while fighting!\n");
		return;
	}
	if (ch[co].attack_cn)
	{
		do_char_log(cn, 1, "You can't do that while your target is fighting!\n");
		return;
	}
	
	do_char_log(co, 0, "%s swapped places with you.\n", ch[cn].name);
	
	cn_x = ch[cn].x;
	cn_y = ch[cn].y;
	co_x = ch[co].x;
	co_y = ch[co].y;
	
	god_transfer_char(cn, 13, 13);
	god_transfer_char(co, cn_x, cn_y);
	god_transfer_char(cn, co_x, co_y);
	
	fx_add_effect(12, 0, ch[cn].x, ch[cn].y, 0);
	fx_add_effect(12, 0, ch[co].x, ch[co].y, 0);
	
	chlog(cn, "DO_SWAP: swapped %s with %s.", ch[cn].name, ch[co].name);
}

void do_force_recall(int cn)
{
	int in;
	
	in = god_create_buff();

	strcpy(bu[in].name, "Recall");
	strcpy(bu[in].reference, "recall");
	strcpy(bu[in].description, "Recall.");

	bu[in].flags    |= IF_SPELL;
	bu[in].temp      = SK_RECALL;
	bu[in].sprite[1] = BUF_SPR_RECALL;
	bu[in].power 	  = 999;
	bu[in].data[1]   = ch[cn].temple_x;
	bu[in].data[2]   = ch[cn].temple_y;
	bu[in].data[4]   = 1;
	bu[in].duration  = bu[in].active = TICKS * 5;
	
	fx_add_effect(6, 0, ch[cn].x, ch[cn].y, 0);
	
	add_spell(cn, in);
	
	return 1;
}

// -------- //

void do_afk(int cn, char *msg)
{
	if (ch[cn].data[PCD_AFK])
	{
		do_char_log(cn, 1, "Back.\n");
		ch[cn].data[PCD_AFK] = 0;
	}
	else
	{
		ch[cn].data[PCD_AFK] = 1;
		if (msg != NULL)
		{
			do_char_log(cn, 1, "Away. Use #afk again to show you're back. Message:\n");
			sprintf(ch[cn].text[0], "%-.48s", msg);
			do_char_log(cn, 3, "  \"%s\"\n", ch[cn].text[0]);
		}
		else
		{
			do_char_log(cn, 1, "Away. Use #afk again to show you're back.\n");
			ch[cn].text[0][0] = '\0';
		}
	}
}

void do_mark(int cn, int co, char *msg)
{
	if (!IS_SANEPLAYER(co))
	{
		do_char_log(cn, 0, "That's not a player\n");
		return;
	}

	if (!msg)
	{
		do_char_log(cn, 1, "Removed mark \"%s\" from %s\n",
		            ch[co].text[3], ch[co].name);
		ch[co].text[3][0] = 0;
		return;
	}
	else
	{
		strncpy(ch[co].text[3], msg, 159);
		ch[co].text[3][159] = 0;
		do_char_log(cn, 1, "Marked %s with \"%s\"\n", ch[co].name, ch[co].text[3]);
		return;
	}
}

void do_allow(int cn, int co)
{
	if (!IS_SANEPLAYER(co))
	{
		do_char_log(cn, 0, "That's not a player\n");
		return;
	}
	
	ch[cn].data[PCD_ALLOW] = co;
	if (co)
	{
		do_char_log(cn, 0, "%s is now allowed to access your grave.\n", ch[co].name);
	}
	else
	{
		do_char_log(cn, 0, "Nobody may now access your grave.\n");
	}
}

int isgroup(int cn, int co)
{
	int n;

	for (n = 1; n<10; n++)
	{
		if (ch[cn].data[n]==co)
		{
			return 1;
		}
	}

	return 0;
}

int isnearby(int cn, int co)
{
	int xf, yf, xt, yt, x, y, cc;
	int area = 7;

	xf = max(1, ch[cn].x - area);
	yf = max(1, ch[cn].y - area);
	xt = min(MAPX - 1, ch[cn].x + area+1);
	yt = min(MAPY - 1, ch[cn].y + area+1);

	for (x = xf; x<xt; x++) for (y = yf; y<yt; y++) if ((cc = map[x + y * MAPX].ch)) 
		if (cc==co) 
			return 1;

	return 0;
}

void do_group(int cn, char *name)
{
	int n, co, tmp, allow;

	if (name[0]==0)
	{
		do_char_log(cn, 1, "Your group consists of:\n");
		do_char_log(cn, 1, "%-15.15s %d/%dH, %d/%dE, %d/%dM\n",
		            ch[cn].name,
		            (ch[cn].a_hp + 500) / 1000,
		            ch[cn].hp[5],
		            (ch[cn].a_end + 500) / 1000,
		            ch[cn].end[5],
		            (ch[cn].a_mana + 500) / 1000,
		            ch[cn].mana[5]);
		for (n = 1; n<10; n++)
		{
			if ((co = ch[cn].data[n])==0)
			{
				continue;
			}
			if (isgroup(co, cn))
			{
				do_char_log(cn, 1, "%-15.15s %d/%dH, %d/%dE, %d/%dM\n",
				            ch[co].name,
				            (ch[co].a_hp + 500) / 1000,
				            ch[co].hp[5],
				            (ch[co].a_end + 500) / 1000,
				            ch[co].end[5],
				            (ch[co].a_mana + 500) / 1000,
				            ch[co].mana[5]);
			}
			else
			{
				do_char_log(cn, 1, "%-15.15s (not acknowledged)\n",
				            ch[co].name);
			}
		}
	}
	else
	{
		co = do_lookup_char(name);
		if (co==0)
		{
			do_char_log(cn, 0, "Sorry, I cannot find \"%s\".\n", name);
			return;
		}
		if (co==cn)
		{
			do_char_log(cn, 0, "You're automatically part of your own group.\n");
			return;
		}
		if (!(ch[co].flags & (CF_PLAYER)))
		{
			do_char_log(cn, 0, "Sorry, %s is not a player.\n", name);
			return;
		}
		if (ch[co].used!=USE_ACTIVE || (IS_INVISIBLE(co) && (invis_level(cn) < invis_level(co))))
		{
			do_char_log(cn, 0, "Sorry, %s seems not to be online.\n", name);
			for (n = 1; n<10; n++)
			{
				if (ch[cn].data[n]==co)
				{
					do_char_log(cn, 0, "Inactive player removed from your group.\n");
					ch[cn].data[n] = 0;
				}
			}
			return;
		}
		for (n = 1; n<10; n++)
		{
			if (ch[cn].data[n]==co)
			{
				ch[cn].data[n] = 0;
				do_char_log(cn, 1, "%s removed from your group.\n", ch[co].name);
				do_char_log(co, 0, "You are no longer part of %s's group.\n", ch[cn].name);
				return;
			}
		}

		switch(max(getrank(cn), getrank(co)))
		{
		case 19:
			allow = GROUP_RANGE+1;
			break;
		case 20:
			allow = GROUP_RANGE+2;
			break;
		case 21:
			allow = GROUP_RANGE+3;
			break;
		case 22:
			allow = GROUP_RANGE+4;
			break;
		case 23:
			allow = GROUP_RANGE+5;
			break;
		case 24:
			allow = GROUP_RANGE+6;
			break;
		default:
			allow = GROUP_RANGE;
			break;
		}

		if (abs(tmp = rankdiff(cn, co))>allow)
		{
			do_char_log(cn, 0, "Sorry, you cannot group with %s; he is %d ranks %s you. %s maximum distance is %d.\n",
			            ch[co].name, abs(tmp), tmp>0 ? "above" : "below", tmp>0 ? "Their" : "Your", allow);
			return;
		}


		for (n = 1; n<10; n++)
		{
			if (ch[cn].data[n]==0)
			{
				ch[cn].data[n] = co;
				do_char_log(cn, 1, "%s added to your group.\n", ch[co].name);
				do_char_log(co, 0, "You are now part of %s's group.\n", ch[cn].name);

				if (isgroup(co, cn))
				{
					do_char_log(cn, 1, "Two way group established.\n");
					do_char_log(co, 1, "Two way group established.\n");
				}
				else
				{
					do_char_log(co, 0, "Use \"#group %s\" to add her/him to your group.\n", ch[cn].name);
				}
				return;
			}
		}
		do_char_log(cn, 0, "Sorry, I can only handle ten group members.\n");
	}
}

void do_ignore(int cn, char *name, int flag)
{
	int n, co, tmp;

	if (!flag)
	{
		tmp = 30;
	}
	else
	{
		tmp = 50;
	}

	if (name[0]==0)
	{
		do_char_log(cn, 1, "Your ignore group consists of:\n");
		for (n = tmp; n<tmp + 10; n++)
		{
			if ((co = ch[cn].data[n])==0)
			{
				continue;
			}
			do_char_log(cn, 1, "%15.15s\n",
			            ch[co].name);
		}
	}
	else
	{
		co = do_lookup_char(name);
		if (co==0)
		{
			do_char_log(cn, 0, "Sorry, I cannot find \"%s\".\n", name);
			return;
		}
		if (co==cn)
		{
			do_char_log(cn, 0, "Ignoring yourself won't do you much good.\n");
			return;
		}
		for (n = tmp; n<tmp + 10; n++)
		{
			if (ch[cn].data[n]==co)
			{
				ch[cn].data[n] = 0;
				do_char_log(cn, 1, "%s removed from your ignore group.\n", ch[co].name);
				return;
			}
		}
		if (!(ch[co].flags & (CF_PLAYER)))
		{
			do_char_log(cn, 0, "Sorry, %s is not a player.\n", name);
			return;
		}
		for (n = tmp; n<tmp + 10; n++)
		{
			if (ch[cn].data[n]==0)
			{
				ch[cn].data[n] = co;
				do_char_log(cn, 1, "%s added to your ignore group.\n", ch[co].name);
				return;
			}
		}
		do_char_log(cn, 0, "Sorry, I can only handle ten ignore group members.\n");
	}
}

void do_follow(int cn, char *name)
{
	int co;

	if (name[0]==0)
	{
		if ((co = ch[cn].data[10])!=0)
		{
			do_char_log(cn, 1, "You're following %s; type '#follow self' to stop.\n", ch[co].name);
		}
		else
		{
			do_char_log(cn, 1, "You're not following anyone.\n");
		}
		return;
	}
	co = do_lookup_char_self(name, cn);
	if (!co)
	{
		do_char_log(cn, 0, "Sorry, I cannot find %s.\n", name);
		return;
	}
	if (co==cn)
	{
		do_char_log(cn, 1, "Now following no one.\n");
		ch[cn].data[10] = 0;
		ch[cn].goto_x = 0;
		return;
	}
	/* CS, 991127: No #FOLLOW of invisible Imps */
	if (ch[co].flags & (CF_INVISIBLE | CF_NOWHO) &&
	    invis_level(co) > invis_level(cn))
	{
		do_char_log(cn, 0, "Sorry, I cannot find %s.\n", name);
		return;
	}
	ch[cn].data[10] = co;
	do_char_log(cn, 1, "Now following %s.\n", ch[co].name);
}

void do_fightback(int cn)
{
	ch[cn].flags ^= CF_FIGHT_OFF;

	if (ch[cn].flags & CF_FIGHT_OFF)
		do_char_log(cn, 1, "Auto-Fightback disabled.\n");
	else
		do_char_log(cn, 1, "Auto-Fightback enabled.\n");

	if (ch[cn].flags & (CF_PLAYER))
		chlog(cn, "Set fight_off to %s", (ch[cn].flags & CF_FIGHT_OFF) ? "on" : "off");
}

void do_deposit(int cn, int g, int s, char *topic)
{
	int m, v, co=0;
	
	if (strcmp(topic, "")!=0)
	{
		for (m = 1; m<MAXCHARS; m++)
		{
			if (ch[m].used!=USE_EMPTY && strcmp(toupper(topic), toupper(ch[m].name))==0)	// Character with this name exists
			{
				co = m;
				break;
			}
		}
		if (!co)
		{
			do_char_log(cn, 0, "That character doesn't exist.\n");
			return;
		}
	}

	m = ch[cn].x + ch[cn].y * MAPX;
	if (!(map[m].flags & MF_BANK))
	{
		do_char_log(cn, 0, "Sorry, deposit works only in banks.\n");
		return;
	}
	v = 100 * g + s;
	// DB: very large numbers map to negative signed integers - so this might be confusing as well
	if (v < 0)
	{
		do_char_log(cn, 0, "If you want to withdraw money, then say so!\n");
		return;
	}
	if (v>ch[cn].gold)
	{
		do_char_log(cn, 0, "Sorry, you don't have that much money.\n");
		return;
	}
	ch[cn].gold -= v;
	if (co)
	{
		ch[co].data[13] += v;

		do_update_char(cn);
		do_update_char(co);

		do_char_log(cn, 1, "You deposited %dG %dS into %s's account; their new balance is %dG %dS.\n",
					v / 100, v % 100, ch[co].name, ch[co].data[13] / 100, ch[co].data[13] % 100);
	}
	else
	{
		ch[cn].data[13] += v;

		do_update_char(cn);

		do_char_log(cn, 1, "You deposited %dG %dS; your new balance is %dG %dS.\n",
					v / 100, v % 100, ch[cn].data[13] / 100, ch[cn].data[13] % 100);
	}
}

void do_withdraw(int cn, int g, int s, char *topic)
{
	int m, v, co=0;
	
	if (strcmp(topic, "")!=0)
	{
		for (m = 1; m<MAXCHARS; m++)
		{
			if (ch[m].used!=USE_EMPTY && strcmp(toupper(topic), toupper(ch[m].name))==0)	// Character with this name exists
			{
				co = m;
				break;
			}
		}
		if (!co)
		{
			do_char_log(cn, 0, "That character doesn't exist.\n");
			return;
		}
	}

	m = ch[cn].x + ch[cn].y * MAPX;
	if (!(map[m].flags & MF_BANK))
	{
		do_char_log(cn, 0, "Sorry, withdraw works only in banks.\n");
		return;
	}
	v = 100 * g + s;

	if (v < 0)
	{
		do_char_log(cn, 0, "If you want to deposit money, then say so!\n");
		return;
	}
	if (co)
	{
		if (v>ch[co].data[13] || v<0)
		{
			do_char_log(cn, 0, "Sorry, %s doesn't have that much money in the bank.\n", ch[co].name);
			return;
		}
		ch[cn].gold += v;
		ch[co].data[13] -= v;

		do_update_char(cn);
		do_update_char(co);

		do_char_log(cn, 1, "You withdraw %dG %dS from %s's account; their new balance is %dG %dS.\n",
					v / 100, v % 100, ch[co].name, ch[co].data[13] / 100, ch[co].data[13] % 100);
	}
	else
	{
		if (v>ch[cn].data[13] || v<0)
		{
			do_char_log(cn, 0, "Sorry, you don't have that much money in the bank.\n");
			return;
		}
		ch[cn].gold += v;
		ch[cn].data[13] -= v;

		do_update_char(cn);

		do_char_log(cn, 1, "You withdraw %dG %dS; your new balance is %dG %dS.\n",
					v / 100, v % 100, ch[cn].data[13] / 100, ch[cn].data[13] % 100);
	}
}

void do_balance(int cn)
{
	int n, in, m, tmp = 0;

	m = ch[cn].x + ch[cn].y * MAPX;
	if (!(map[m].flags & MF_BANK))
	{
		do_char_log(cn, 0, "Sorry, balance works only in banks.\n");
		return;
	}
	do_char_log(cn, 1, "Your balance is %dG %dS.\n", ch[cn].data[13] / 100, ch[cn].data[13] % 100);

//	for (n = 0; n<62; n++)					if ((in = ch[cn].depot[n])!=0)						tmp++;
	for (n = 0; n<ST_PAGES*ST_SLOTS; n++) 	if ((in = st[cn].depot[n/ST_SLOTS][n%ST_SLOTS])!=0) tmp++;

	if (tmp) do_char_log(cn, 1, "You currently have %d items in your depot.\n", tmp);
}

static char *order = NULL;

int qsort_proc(const void *a, const void *b)
{
	int in, in2;
	char *o;

	in  = *((int*)a);
	in2 = *((int*)b);
	
	// Locked items stay where they are
	/*
	if ((it[in].flags & IF_ITEMLOCK) || (it[in2].flags & IF_ITEMLOCK))
	{
		return 0;
	}
	*/

	if (!in && !in2)
	{
		return 0;
	}

	if (in && !in2)
	{
		return -1;
	}
	if (!in && in2)
	{
		return 1;
	}

	for (o = order; *o; o++)
	{
		switch(*o)
		{
		case 'w':
			if ((it[in].flags & IF_WEAPON) && !(it[in2].flags & IF_WEAPON))
			{
				return -1;
			}
			if (!(it[in].flags & IF_WEAPON) && (it[in2].flags & IF_WEAPON))
			{
				return 1;
			}
			break;

		case 'a':
			if ((it[in].flags & IF_ARMOR) && !(it[in2].flags & IF_ARMOR))
			{
				return -1;
			}
			if (!(it[in].flags & IF_ARMOR) && (it[in2].flags & IF_ARMOR))
			{
				return 1;
			}
			break;

		case 'p':
			if ((it[in].flags & IF_USEDESTROY) && !(it[in2].flags & IF_USEDESTROY))
			{
				return -1;
			}
			if (!(it[in].flags & IF_USEDESTROY) && (it[in2].flags & IF_USEDESTROY))
			{
				return 1;
			}
			break;

		case 'h':
			if (it[in].hp[0]>it[in2].hp[0])
			{
				return -1;
			}
			if (it[in].hp[0]<it[in2].hp[0])
			{
				return 1;
			}
			break;

		case 'e':
			if (it[in].end[0]>it[in2].end[0])
			{
				return -1;
			}
			if (it[in].end[0]<it[in2].end[0])
			{
				return 1;
			}
			break;

		case 'm':
			if (it[in].mana[0]>it[in2].mana[0])
			{
				return -1;
			}
			if (it[in].mana[0]<it[in2].mana[0])
			{
				return 1;
			}
			break;

		case 'v':
			if (it[in].value>it[in2].value)
			{
				return -1;
			}
			if (it[in].value<it[in2].value)
			{
				return 1;
			}
			break;

		default:
			break;

		}
	}
	
	// Sort soulstones
	if (IS_SOULSTONE(in) && IS_SOULSTONE(in2))
	{
		if (it[in].data[1] > it[in2].data[1]) return -1;
		else if (it[in].data[1] < it[in2].data[1]) return 1;
		else return 0;
	}
	// Sort soul catalysts
	if (IS_SOULCAT(in) && IS_SOULCAT(in2))
	{
		if (it[in].data[4] < it[in2].data[4]) return -1;
		else if (it[in].data[4] > it[in2].data[4]) return 1;
		else return 0;
	}
	// Sort soul focuses
	if (IS_SOULFOCUS(in) && (IS_SOULSTONE(in2) || IS_SOULCAT(in2)))
	{
		return 1;
	}
	if (IS_SOULFOCUS(in2) && (IS_SOULSTONE(in) || IS_SOULCAT(in)))
	{
		return -1;
	}
	// Sort tarot cards
	if (IS_TAROT(in) && IS_TAROT(in2))
	{
		if (it[in].temp < it[in2].temp) return -1;
		else if (it[in].temp > it[in2].temp) return 1;
		else return 0;
	}

	// fall back to sort by value
	if (it[in].value>it[in2].value)
	{
		return -1;
	}
	if (it[in].value<it[in2].value)
	{
		return 1;
	}

	if (it[in].temp>it[in2].temp)
	{
		return 1;
	}
	if (it[in].temp<it[in2].temp)
	{
		return -1;
	}

	return 0;
}

void do_sort_depot(int cn, char *arg, char *arg2)
{
	int n, m, co;
	int temp = 0;
	char chname[40];
	
	if (strcmp(arg, "")!=0)
	{
		for (m=0; m<strlen(arg); m++) 
			arg[m] = tolower(arg[m]);
		for (n = 1; n<MAXCHARS; n++)
		{
			if (ch[n].used==USE_EMPTY || !IS_SANEPLAYER(n)) continue;
			strcpy(chname, ch[n].name); chname[0] = tolower(chname[0]);
			if (strcmp(arg, chname)==0)	// Character with this name exists
			{
				temp = n;
				break;
			}
		}
	}
	
	if (temp) // Sequence above succeeded, second arg is a sort #
	{
		order = arg2;
		co = temp;
		temp = 0;
		for (n = 80; n<89; n++)
		{
			if (ch[cn].data[n]==0) continue;
			for (m = 80; m<89; m++)
			{
				if (ch[co].data[m]==0) continue;
				if (ch[cn].data[n]==ch[co].data[m])
				{
					temp=1;
				}
			}
		}
		if (!temp)
		{
			do_char_log(cn, 0, "This is not one of your characters.\n");
			return;
		}
	}
	else // First arg is just a sort #
	{
		order = arg;
		co = cn;
	}
	
	if (IS_BUILDING(co))
	{
		do_char_log(cn, 1, "Not in build-mode, dude.");
		return;
	}
	
	for (n=0;n<ST_PAGES;n++)
		qsort(st[co].depot[n], ST_SLOTS, sizeof(int), qsort_proc);

	do_update_char(co);
}

void do_sort(int cn, char *arg)
{
	int n;
	
	if (IS_BUILDING(cn))
	{
		do_char_log(cn, 1, "Not in build-mode, dude.");
		return;
	}

	order = arg;
	
	// Set temporary item locks before qsort, removing any invalid ones beforehand
	/*
	for (n=0;n<MAXITEMS;n++) 
	{
		it[ch[cn].item[n]].flags &= ~IF_ITEMLOCK;
	//	if (ch[cn].item_lock[n])
		{
			it[ch[cn].item[n]].flags |= IF_ITEMLOCK;
		}
	}
	*/
	
	qsort(ch[cn].item, MAXITEMS, sizeof(int), qsort_proc);

	do_update_char(cn);
}

void do_depot(int cn, char *topic)
{
	int n, m, co;
	int temp = 0;
	char chname[40];
	
	if (strcmp(topic, "")!=0)
	{
		for (m=0; m<strlen(topic); m++) 
			topic[m] = tolower(topic[m]);
		for (n = 1; n<MAXCHARS; n++)
		{
			if (ch[n].used==USE_EMPTY || !IS_SANEPLAYER(n)) continue;
			strcpy(chname, ch[n].name); chname[0] = tolower(chname[0]);
			if (strcmp(topic, chname)==0)	// Character with this name exists
			{
				temp = n;
				break;
			}
		}
		if (!temp)
		{
			do_char_log(cn, 0, "That character doesn't exist.\n");
			return;
		}
	}
	
	if (temp)
	{
		co = temp;
		temp = 0;
		for (n = 80; n<89; n++)
		{
			if (ch[cn].data[n]==0) continue;
			for (m = 80; m<89; m++)
			{
				if (ch[co].data[m]==0) continue;
				if (ch[cn].data[n]==ch[co].data[m])
				{
					temp=1;
				}
			}
		}
		if (!temp)
		{
			do_char_log(cn, 0, "This is not one of your characters.\n");
			return;
		}
		do_char_log(cn, 1, "This is %s's depot. Anything you take or leave here will be transferred.\n", ch[co].name);
	}
	else
	{
		co = cn;
		do_char_log(cn, 1, "This is your bank depot. You can store up to 512 items here.\n");
	}
	
	do_look_depot(cn, co);
}

void do_lag(int cn, int lag)
{
	if (lag==0)
	{
		do_char_log(cn, 1, "Lag control turned off (was at %d).\n", ch[cn].data[19] / TICKS);
		ch[cn].data[19] = 0;
		return;
	}
	if (lag>20 || lag<3)
	{
		do_char_log(cn, 1, "Lag control needs a value between 3 and 20. Use 0 to turn it off.\n");
		return;
	}
	ch[cn].data[19] = lag * TICKS;
	do_char_log(cn, 1, "Lag control will turn you to stone if lag exceeds %d seconds.\n", lag);
}

void do_god_give(int cn, int co)
{
	int in;

	in = ch[cn].citem;

	if (!in)
	{
		do_char_log(cn, 0, "You have nothing under your mouse cursor!\n");
		return;
	}

	if (!god_give_char(in, co))
	{
		do_char_log(cn, 1, "god_give_char() returned error.\n");
		return;
	}
	do_char_log(cn, 1, "%s given to %s.\n", it[in].name, ch[co].name);
	chlog(cn, "IMP: Gave %s (t=%d) to %s (%d)", it[in].name, in, ch[co].name, co);
	ch[cn].citem = 0;
}

void do_gold(int cn, int val)
{
	chlog(cn, "trying to take %d gold from purse", val);
	
	if (ch[cn].citem)
	{
		do_char_log(cn, 0, "Please remove the item from your mouse cursor first.\n");
		return;
	}
	else
	{
		ch[cn].citem = 0;
	}
	if (val<1)
	{
		do_char_log(cn, 0, "That's not very much, is it?\n");
		return;
	}
	if (val>10000000)
	{
		do_char_log(cn, 0, "You can't hold that much in your hands!\n");
		return;
	}
	val *= 100;
	if (val>ch[cn].gold || val<0)
	{
		do_char_log(cn, 0, "You don't have that much gold!\n");
		return;
	}

	ch[cn].gold -= val;
	ch[cn].citem = 0x80000000 | val;

	do_update_char(cn);

	do_char_log(cn, 1, "You take %dG from your purse.\n", val / 100);
}

void do_emote(int cn, char *text)
{
	if (!text)
	{
		return;
	}

	if (strchr(text, '%'))
	{
		return;
	}

	if (ch[cn].flags & CF_SHUTUP)
	{
		do_char_log(cn, 0, "You feel guilty.\n");
		chlog(cn, "emote: feels guilty (%s)", text);
	}
	else if (ch[cn].flags & CF_INVISIBLE)   // JC: 091200: added anonymous emote
	{
		do_area_log(0, 0, ch[cn].x, ch[cn].y, 2, "Somebody %s.\n", text);
		chlog(cn, "emote(inv): %s", text);

	}
	else
	{
		do_area_log(0, 0, ch[cn].x, ch[cn].y, 2, "%s %s.\n", ch[cn].name, text);
		chlog(cn, "emote: %s", text);
	}
}

/*	added by SoulHunter 01.05.2000	*/
void do_create_note(int cn, char *text)
{
	int m, tmp = 132;                         // empty parchment template = 132

	if (!text)
	{
		return;                         // we wont create 'note' if we havent any text
	}
	if (strlen(text) >= 199)
	{
		return;                         // we wont create it if text is larger
	}
	// than size of description (200)

	chlog(cn, "created note: %s.", text);

	for (m = 0; m<MAXITEMS; m++) // looking for free space in inventory
	{
		if (ch[cn].item[m]==0)
		{
			tmp = god_create_item(tmp); // creating from template 132
			if (tmp)   // successful
			{
				it[tmp].temp = 0;       // clear template
				strcpy(&it[tmp].description[0], text); // copy new description
				it[tmp].flags |= IF_NOEXPIRE;
				it[tmp].carried = cn; // carried by <cn>
				ch[cn].item[m]  = tmp; // item is in inventory
			}

			do_update_char(cn);
			return;
		}
	}
	// failed to find free space
	do_char_log(cn, 0, "You failed to create a note. Inventory is full!\n");
	return;
}
/* --SH end */

int dbatoi_self(int cn, char *text)
{
	if (!text)
	{
		return( cn);
	}
	if (!*text)
	{
		return( cn);    // no text means self - easier to do here
	}
	if (isdigit(*text))
	{
		return( atoi(text));
	}
	else
	{
		return( do_lookup_char_self(text, cn));
	}
}

int dbatoi(char *text)
{
	if (!text)
	{
		return 0;
	}
	if (isdigit(*text))
	{
		return( atoi(text));
	}
	else
	{
		return( do_lookup_char(text));
	}
}

void do_become_purple(int cn)
{
	if (globs->ticker - ch[cn].data[65]<TICKS * 60 && !IS_PURPLE(cn))
	{
		if (IS_IN_PURP(ch[cn].x, ch[cn].y))
		{
			do_char_log(cn, 0, " \n");
			do_char_log(cn, 0, "You feel a god leave you. You feel alone. Scared. Unprotected.\n");
			do_char_log(cn, 0, " \n");
			do_char_log(cn, 0, "Another presence enters your mind. You feel hate. Lust. Rage. A Purple Cloud engulfs you.\n");
			do_char_log(cn, 0, " \n");
			do_char_log(cn, 0, "\"THE GOD OF THE PURPLE WELCOMES YOU, MORTAL! MAY YOU BE A GOOD SLAVE!\"\n");
			do_char_log(cn, 0, " \n");
			do_char_log(cn, 2, "Hardcore player flag set. Enjoy your terror.\n");
			do_char_log(cn, 0, " \n");
			ch[cn].kindred |= KIN_PURPLE;
			if (ch[cn].temple_x!=HOME_START_X)
			{
				ch[cn].temple_x = HOME_PURPLE_X;
				ch[cn].temple_y = HOME_PURPLE_Y;
			}

			do_update_char(cn);

			chlog(cn, "Converted to purple.");

			fx_add_effect(5, 0, ch[cn].x, ch[cn].y, 0);
		}
		else
		{
			do_char_log(cn, 0, "It seems like this only works while inside the Temple of the Purple One.\n");
		}
	}
	else
	{
		do_char_log(cn, 0, "Hmm. Nothing happened.\n");
	}
}

void do_stat(int cn)
{
	do_char_log(cn, 2, "items: %d/%d\n", globs->item_cnt, MAXITEM);
	do_char_log(cn, 2, "chars: %d/%d\n", globs->character_cnt, MAXCHARS);
	do_char_log(cn, 2, "effes: %d/%d\n", globs->effect_cnt, MAXEFFECT);

	do_char_log(cn, 2, "newmoon=%d\n", globs->newmoon);
	do_char_log(cn, 2, "fullmoon=%d\n", globs->fullmoon);
	do_char_log(cn, 2, "mdday=%d (%%28=%d)\n", globs->mdday, globs->mdday % 28);

	do_char_log(cn, 2, "mayhem=%s, looting=%s, close=%s, cap=%s, speedy=%s\n",
	            globs->flags & GF_MAYHEM ? "yes" : "no",
	            globs->flags & GF_LOOTING ? "yes" : "no",
	            globs->flags & GF_CLOSEENEMY ? "yes" : "no",
	            globs->flags & GF_CAP ? "yes" : "no",
	            globs->flags & GF_SPEEDY ? "yes" : "no");
	do_char_log(cn, 2, "stronghold=%s, newbs=%s, discord=%s\n",
				globs->flags & GF_STRONGHOLD ? "yes" : "no",
	            globs->flags & GF_NEWBS ? "yes" : "no",
				globs->flags & GF_DISCORD ? "yes" : "no");
}

void do_enter(int cn)
{
	ch[cn].flags &= ~(CF_NOWHO | CF_INVISIBLE);
	do_announce(cn, 0, "%s entered the game.\n", ch[cn].name);
}

void do_leave(int cn)
{
	do_announce(cn, 0, "%s left the game.\n", ch[cn].name);
	ch[cn].flags |= (CF_NOWHO | CF_INVISIBLE);
}

void do_npclist(int cn, char *name)
{
	int n, foundalive = 0, foundtemp = 0;

	if (!name)
	{
		do_char_log(cn, 0, "Gimme a name to work with, dude!\n");
		return;
	}
	if (strlen(name)<3 || strlen(name)>35)
	{
		do_char_log(cn, 0, "What kind of name is that, dude?\n");
		return;
	}

	for (n = 1; n<MAXCHARS; n++)
	{
		if (!ch[n].used)
		{
			continue;
		}
		if (ch[n].flags & CF_PLAYER)
		{
			continue;
		}
		if (!strstr(ch[n].name, name))
		{
			continue;
		}

		foundalive++;

		do_char_log(cn, 1, "C%4d %-20.20s %.20s\n",
		            n, ch[n].name, ch[n].description);
	}
	for (n = 1; n<MAXTCHARS; n++)
	{
		if (!ch_temp[n].used)
		{
			continue;
		}
		if (ch_temp[n].flags & CF_PLAYER)
		{
			continue;
		}
		if (!strstr(ch_temp[n].name, name))
		{
			continue;
		}

		foundtemp++;

		do_char_log(cn, 1, "T%4d %-20.20s %.20s\n",
		            n, ch_temp[n].name, ch_temp[n].description);
	}

	if (foundalive || foundtemp)
	{
		do_char_log(cn, 1, " \n");
	}
	do_char_log(cn, 1, "%d characters, %d templates by that name\n", foundalive, foundtemp);
}

void do_respawn(int cn, int co)
{
	if (co<1 || co>=MAXTCHARS)
	{
		do_char_log(cn, 0, "That template number is a bit strange, don't you think so, dude?\n");
		return;
	}
	globs->reset_char = co;
}

void do_list_net(int cn, int co)
{
	int n;

	do_char_log(cn, 1, "%s is know to log on from the following addresses:\n", ch[co].name);

	for (n = 80; n<90; n++)
	{
		do_char_log(cn, 1, "%d.%d.%d.%d\n", ch[co].data[n] & 255, (ch[co].data[n] >> 8) & 255, (ch[co].data[n] >> 16) & 255, (ch[co].data[n] >> 24) & 255);
	}
}

void do_list_all_flagged(int cn, unsigned long long flag)
{
	int n;

	for (n = 1; n<MAXCHARS; n++)
	{
		if (!ch[n].used || !IS_PLAYER(n) || !(ch[n].flags & flag))
		{
			continue;
		}
		do_char_log(cn, 1, "%04d %s\n", n, ch[n].name);
	}
}

void do_make_sstone_gear(int cn, int n, int val)
{
	int in;
	
	in = ch[cn].citem;
	
	if (!in || (in & 0x80000000))
	{
		do_char_log(cn, 1, "Invalid item.\n");
		return;
	}
	if (n == -1)   // bad skill number
	{
		return;
	}
	else if (!IS_SANESKILL(n))
	{
		do_char_log(cn, 0, "Skill number %d out of range.\n", n);
		return;
	}
	else if (val<0 || val>24)
	{
		do_char_log(cn, 0, "Skill amount %d out of range.\n", val);
		return;
	}
	
	it[in].skill[n][0] = val;
	it[in].skill[n][2] = val*5;
	
	it[in].min_rank = min(24, max(val-3, it[in].min_rank));
	it[in].value -= 1;
	it[in].power += val * 5;
	
	it[in].flags |= IF_UPDATE | IF_IDENTIFIED | IF_SOULSTONE;
	
	if (!HAS_ENCHANT(in, 34))
	{
		it[in].flags &= ~IF_NOREPAIR;
		if (it[in].flags & IF_WEAPON)		it[in].max_damage = it[in].power * 4000;
		else								it[in].max_damage = it[in].power * 1000;
	}
}

void do_become_gornkwai(int cn, int flag) // 0: become gorn // 1: become kwai
{
	int days;
	
	if ((IS_CLANGORN(cn) && flag) || (IS_CLANKWAI(cn) && !flag))
	{
		days = (globs->ticker - ch[cn].data[PCD_ATTACKTIME]) / (60 * TICKS) / 60 / 24;
		if (days < 14)
		{
			do_char_log(cn, 0, "You may not change clans for %u days.\n", 14 - days);
			return;
		}
	}
	
	if ((IS_CLANGORN(cn) && !flag) || (IS_CLANKWAI(cn) && flag))
	{
		do_char_log(cn, 1, "You're already a follower!\n");
		return;
	}
	
	if (flag && IS_IN_KWAI(ch[cn].x, ch[cn].y))
	{
		do_char_log(cn, 0, " \n");
		do_char_log(cn, 0, "\"THE GODDESS KWAI WELCOMES YOU, MORTAL!\"\n");
		do_char_log(cn, 0, " \n");
		do_char_log(cn, 2, "Kwai clan player flag set.\n");
		do_char_log(cn, 0, " \n");
		
		ch[cn].kindred |= KIN_CLANKWAI;
		ch[cn].kindred &= ~KIN_CLANGORN;
		ch[cn].data[PCD_ATTACKTIME] = 0;
		ch[cn].data[PCD_ATTACKVICT] = 0;
		ch[cn].temple_x = HOME_KWAI_X;
		ch[cn].temple_y = HOME_KWAI_Y;
		
		chlog(cn, "Converted to Kwai.");
		fx_add_effect(5, 0, ch[cn].x, ch[cn].y, 0);
	}
	else if (flag)
	{
		do_char_log(cn, 0, "It seems like this only works while inside the Temple of Kwai.\n");
	}
	else if (!flag && IS_IN_GORN(ch[cn].x, ch[cn].y))
	{
		do_char_log(cn, 0, " \n");
		do_char_log(cn, 0, "\"THE GOD GORN WELCOMES YOU, MORTAL!\"\n");
		do_char_log(cn, 0, " \n");
		do_char_log(cn, 2, "Gorn clan player flag set.\n");
		do_char_log(cn, 0, " \n");
		
		ch[cn].kindred |= KIN_CLANGORN;
		ch[cn].kindred &= ~KIN_CLANKWAI;
		ch[cn].data[PCD_ATTACKTIME] = 0;
		ch[cn].data[PCD_ATTACKVICT] = 0;
		ch[cn].temple_x = HOME_GORN_X;
		ch[cn].temple_y = HOME_GORN_Y;
		
		chlog(cn, "Converted to Gorn.");
		fx_add_effect(5, 0, ch[cn].x, ch[cn].y, 0);
	}
	else
	{
		do_char_log(cn, 0, "It seems like this only works while inside the Temple of Gorn.\n");
	}
}

void do_become_skua(int cn)
{
	int days;

	if (!IS_PURPLE(cn) && !IS_CLANKWAI(cn) && !IS_CLANGORN(cn))
	{
		do_char_log(cn, 0, "Hmm. Nothing happened.\n");
		return;
	}
	else
	{
		if (IS_CLANGORN(cn) || IS_CLANKWAI(cn))
		{
			days = (globs->ticker - ch[cn].data[PCD_ATTACKTIME]) / (60 * TICKS) / 60 / 24;
			if (days < 14)
			{
				do_char_log(cn, 0, "You may not leave your clan for %u days.\n", 14 - days);
				return;
			}
		}
		
		if (IS_IN_SKUA(ch[cn].x, ch[cn].y) || IS_IN_GORN(ch[cn].x, ch[cn].y) || IS_IN_KWAI(ch[cn].x, ch[cn].y))
		{
			if (IS_PURPLE(cn))
			{
				do_char_log(cn, 0, " \n");
				do_char_log(cn, 0, "You feel the presence of a god again. You feel protected.  Your desire to kill subsides.\n");
				do_char_log(cn, 0, " \n");
				do_char_log(cn, 0, "\"THE GOD SKUA WELCOMES YOU, MORTAL! YOUR BONDS OF SLAVERY ARE BROKEN!\"\n");
				do_char_log(cn, 0, " \n");
				do_char_log(cn, 2, "Hardcore player flag cleared.\n");
				do_char_log(cn, 0, " \n");
			}
			else if (IS_CLANGORN(cn) || IS_CLANKWAI(cn))
			{
				do_char_log(cn, 0, " \n");
				do_char_log(cn, 0, "\"THE GOD SKUA WELCOMES YOU, MORTAL!\"\n");
				do_char_log(cn, 0, " \n");
			}
			
			ch[cn].kindred &= ~KIN_PURPLE;
			ch[cn].kindred &= ~KIN_CLANKWAI;
			ch[cn].kindred &= ~KIN_CLANGORN;
			ch[cn].data[PCD_ATTACKTIME] = 0;
			ch[cn].data[PCD_ATTACKVICT] = 0;
			if (ch[cn].temple_x!=HOME_START_X)
			{
				ch[cn].temple_x = HOME_TEMPLE_X;
				ch[cn].temple_y = HOME_TEMPLE_Y;
			}
			chlog(cn, "Converted to skua.");
			fx_add_effect(5, 0, ch[cn].x, ch[cn].y, 0);
		}
		else
		{
			do_char_log(cn, 0, "It seems like this only works while inside the Temple of Skua.\n");
		}
	}
}

void do_allow_spectate(int cn)
{
	int n, nr;
	
	ch[cn].flags ^= CF_ALW_SPECT;

	if (ch[cn].flags & CF_ALW_SPECT)
	{
		do_char_log(cn, 1, "You may now be spectated by other players.\n");
	}
	else
	{
		do_char_log(cn, 1, "You will no longer be spectated by other players.\n");
	}
	
	if (!(ch[cn].flags & CF_ALW_SPECT))
	{
		for (n=1;n<MAXCHARS;n++)
		{
			if (ch[n].used==USE_EMPTY)
				continue;
			if (!IS_SANEPLAYER(n) || !IS_ACTIVECHAR(n))
				continue;
			nr = ch[n].player;
			if (player[nr].spectating == cn && !IS_GOD(n))
			{
				player[nr].spectating = 0;
				do_char_log(n, 0, "%s doesn't want anyone watching right now.\n", ch[cn].name);
			}
		}
	}

	if (ch[cn].flags & (CF_PLAYER))
	{
		chlog(cn, "Set allow-spectate to %s", (ch[cn].flags & CF_ALW_SPECT) ? "on" : "off");
	}
}

void do_spectate(int cn, int co)
{
	int nr;
	
	nr = ch[cn].player;
	if (co == 0 || co == cn)
	{
		player[nr].spectating = 0;
		do_char_log(cn, 0, "No longer spectating.\n");
		return;
	}
	else if (!IS_SANEPLAYER(co) || !IS_USEDCHAR(co))
	{
		do_char_log(cn, 0, "That's not a player!\n");
		return;
	}
	else if (!IS_ACTIVECHAR(co) || IS_GOD(co))
	{
		do_char_log(cn, 0, "%s is not available.\n", ch[co].name);
		return;
	}
	else if (!(ch[co].flags & CF_ALW_SPECT) && !IS_GOD(cn)) // && !is_incolosseum(co, 0))
	{
		do_char_log(cn, 0, "%s doesn't want anyone watching right now.\n", ch[co].name);
		return;
	}
	player[nr].spectating = co;
	
	do_char_log(cn, 0, "Now spectating %s. Use /spectate self to return.\n", ch[co].name);
	if (!IS_GOD(cn)) do_char_log(co, 9, "%s is watching you.\n", ch[cn].name);
}

void do_command(int cn, char *ptr)
{
	int n, m;
	int f_c, f_g, f_i, f_s, f_p, f_m, f_u, f_sh, f_gi, f_giu, f_gius, f_poh, f_pol, f_gg;
	char arg[10][40], *args[10];
	char *cmd;

	for (n = 0; n<10; n++)
	{
		args[n] = NULL;
		arg[n][0] = 0;
	}

	for (n = 0; n<10; n++)
	{
		m = 0;
		if (*ptr=='\"')
		{
			ptr++;
			while (*ptr && *ptr!='\"' && m<39)
			{
				arg[n][m++] = *ptr++;
			}
			if (*ptr=='"')
			{
				ptr++;
			}
		}
		else
		{
			while (isalnum(*ptr) && m<39)
			{
				arg[n][m++] = *ptr++;
			}
		}
		arg[n][m] = 0;
		while (isspace(*ptr))
		{
			ptr++;
		}
		if (!*ptr)
		{
			break;
		}
		args[n] = ptr;
	}

	cmd = arg[0];
	strlower(cmd);

	f_gg   = (ch[cn].flags & CF_GREATERGOD) != 0; // greater god
	f_c    = (ch[cn].flags & CF_CREATOR) != 0; // creator
	f_g    = (ch[cn].flags & CF_GOD) != 0;  // god
	f_i    = (ch[cn].flags & CF_IMP) != 0;  // imp
	f_s    = (ch[cn].flags & CF_STAFF) != 0; // staff
	f_p    = (ch[cn].flags & CF_PLAYER) != 0; // player
	f_u    = (ch[cn].flags & CF_USURP) != 0; // usurp
	f_m    = !f_p;                          // mob
	f_sh   = (ch[cn].flags & CF_SHUTUP) != 0; // shutup
	f_gi   = f_g || f_i;
	f_giu  = f_gi || f_u;
	f_gius = f_giu || f_s;
	f_poh  = (ch[cn].kindred & KIN_POH) !=0;
	f_pol  = ((ch[cn].kindred & KIN_POH_LEADER) || (ch[cn].flags & CF_GOD));

	switch (cmd[0])
	{
	case 'a':
		if (prefix(cmd, "afk") && f_p)
		{
			do_afk(cn, args[0]);
			return;
		}
		;
		if (prefix(cmd, "allow") && f_p)
		{
			do_allow(cn, dbatoi(arg[1]));
			return;
		}
		;
		if (prefix(cmd, "allowspectate") && f_p)
		{
			do_allow_spectate(cn);
			return;
		}
		;
		if (prefix(cmd, "allpoles"))
		{
			do_showpoles(cn, 1, arg[1]);
			return;
		}
		;
		if (prefix(cmd, "allquests"))
		{
			do_questlist(cn, 1, arg[1]);
			return;
		}
		;
		if (prefix(cmd, "amethyst"))
		{
			do_listrings(cn, "amethyst");
			return;
		}
		;
		if (prefix(cmd, "appraise") && (ch[cn].flags & CF_APPRAISE))
		{
			do_toggle_appraisal(cn);
			return;
		}
		;
		if (prefix(cmd, "aquamarine"))
		{
			do_listrings(cn, "aquamarine");
			return;
		}
		;
		if (prefix(cmd, "area") && IS_PROX_CLASS(cn))
		{
			do_toggle_aoe(cn);
			return;
		}
		;
		if (prefix(cmd, "armor"))
		{
			do_listarmors(cn, arg[1]);
			return;
		}
		;
		if (prefix(cmd, "army") && f_giu)
		{
			god_army(cn, atoi(arg[1]), arg[2], arg[3]);
			return;
		}
		;
		if (prefix(cmd, "announce") && f_gius)
		{
			do_server_announce(cn, cn, "%s\n", args[0]);
			return;
		}
		if (prefix(cmd, "addban") && f_gi)
		{
			god_add_ban(cn, dbatoi(arg[1]));
			return;
		}
		;
		if (prefix(cmd, "autoloot") && !f_m)
		{
			do_autoloot(cn);
			return;
		}
		;
		if (prefix(cmd, "axe"))
		{
			do_listweapons(cn, "axe");
			return;
		}
		;
		break;
	case 'b':
		if (prefix(cmd, "buffs"))
		{
			do_listbuffs(cn, cn);
			return;
		}
		;
		if (prefix(cmd, "belt"))
		{
			do_listbelts(cn, arg[1]);
			return;
		}
		;
		if (prefix(cmd, "beryl"))
		{
			do_listrings(cn, "beryl");
			return;
		}
		;
		if (prefix(cmd, "bow") && !f_sh) /*!*/
		{
			ch[cn].misc_action = DR_BOW;
			return;
		}
		;
		if (prefix(cmd, "bs"))
		{
			do_strongholdpoints(cn);
			return;
		}
		;
		if (prefix(cmd, "balance") && !f_m)
		{
			do_balance(cn);
			return;
		}
		if (prefix(cmd, "black") && f_g)
		{
			god_set_flag(cn, dbatoi(arg[1]), CF_BLACK);
			return;
		}
		if (prefix(cmd, "build") && f_c)
		{
			god_build(cn, atoi(arg[1]));
			return;
		}
		break;
	case 'c':
		if (prefix(cmd, "contract"))
		{
			do_showcontract(cn);
			return;
		}
		;
		if (prefix(cmd, "cap") && f_g)
		{
			set_cap(cn, atoi(arg[1]));
			return;
		}
		;
		if (prefix(cmd, "catalyst") && f_g)
		{
			make_catalyst(cn, atoi(arg[2]), atoi(arg[1]));
			return;
		}
		;
		if (prefix(cmd, "caution") && f_gius)
		{
			do_caution(cn, cn, "%s\n", args[0]);
			return;
		}
		if (prefix(cmd, "ccp") && f_i)
		{
			god_set_flag(cn, dbatoi_self(cn, arg[1]), CF_CCP);
			return;
		}
		;
		if (prefix(cmd, "chars"))
		{
			do_showchars(cn);
			return;
		}
		;
		if (prefix(cmd, "citrine"))
		{
			do_listrings(cn, "citrine");
			return;
		}
		;
		if (prefix(cmd, "claw"))
		{
			do_listweapons(cn, "claw");
			return;
		}
		;
		if (prefix(cmd, "cleanslot"))
		{
			break;
		}
		if (prefix(cmd, "cleanslots") && f_gg)
		{
			god_cleanslots(cn);
			return;
		}
		;
		if (prefix(cmd, "closenemey") && f_g)
		{
			god_set_gflag(cn, GF_CLOSEENEMY);
			return;
		}
		;
		if (prefix(cmd, "create") && f_g)
		{
			god_create(cn, atoi(arg[1]), atoi(arg[2]), atoi(arg[3]), atoi(arg[4]));
			return;
		}
		;
		if (prefix(cmd, "creator") && f_gg)
		{
			god_set_flag(cn, dbatoi_self(cn, arg[1]), CF_CREATOR);
			return;
		}
		;
		break;
	case 'd':
		if (prefix(cmd, "deposit") && !f_m)
		{
			if (!isdigit(arg[1][0]))
				do_deposit(cn, atoi(arg[2]), atoi(arg[3]), arg[1]);
			else
				do_deposit(cn, atoi(arg[1]), atoi(arg[2]), "");
			return;
		}
		;
		if (prefix(cmd, "depot") && !f_m)
		{
			do_depot(cn, arg[1]);
			return;
		}
		;
		if (prefix(cmd, "dagger"))
		{
			do_listweapons(cn, "dagger");
			return;
		}
		;
		if (prefix(cmd, "delban") && f_giu)
		{
			god_del_ban(cn, atoi(arg[1]));
			return;
		}
		;
		if (prefix(cmd, "dept") && f_g)
		{
			break;
		}
		if (prefix(cmd, "depth") && f_g)
		{
			god_set_depth(cn, dbatoi_self(cn, arg[1]), atoi(arg[2]), 0);
			return;
		}
		;
		if (prefix(cmd, "diamond"))
		{
			do_listrings(cn, "diamond");
			return;
		}
		;
		if (prefix(cmd, "diffi") && f_g)
		{
			extern int diffi;
			diffi = atoi(arg[1]);
			do_char_log(cn, 0, "Pent diffi is now %d.\n", diffi);
			return;
		}
		;
		if (prefix(cmd, "discord") && f_g)
		{
			god_set_gflag(cn, GF_DISCORD);
			return;
		}
		;
		if (prefix(cmd, "dualsword"))
		{
			do_listweapons(cn, "dualsword");
			return;
		}
		;
		break;
	case 'e':
		if (prefix(cmd, "effect") && f_g)
		{
			effectlist(cn);
			return;
		}
		;
		if (prefix(cmd, "emerald"))
		{
			do_listrings(cn, "emerald");
			return;
		}
		;
		if (prefix(cmd, "emote"))
		{
			do_emote(cn, args[0]);
			return;
		}
		;
		if (prefix(cmd, "enchant") && f_g)
		{
			set_enchantment(cn, atoi(arg[1]));
			return;
		}
		;
		if (prefix(cmd, "enemy") && f_giu)
		{
			do_enemy(cn, arg[1], arg[2]);
			return;
		}
		;
		if (prefix(cmd, "enter") && f_gi)
		{
			do_enter(cn);
			return;
		}
		;
		if (prefix(cmd, "exit") && f_u)
		{
			god_exit_usurp(cn);
			return;
		}
		;
		if (prefix(cmd, "eras") && f_g) /*!*/
		{
			break;
		}
		;
		if (prefix(cmd, "erase") && f_g) /*!*/
		{
			god_erase(cn, dbatoi(arg[1]), 0);
			return;
		}
		;
		break;
	case 'f':
		if (prefix(cmd, "fightback"))
		{
			do_fightback(cn);
			return;
		}
		if (prefix(cmd, "follow") && !f_m)
		{
			do_follow(cn, arg[1]);
			return;
		}
		if (prefix(cmd, "force") && f_giu)
		{
			god_force(cn, arg[1], args[1]);
			return;
		}
		break;
	case 'g':
		if (prefix(cmd, "gtell") && !f_m) /*!*/
		{
			do_gtell(cn, args[0]);
			return;
		}
		if (prefix(cmd, "garbage"))
		{
			do_trash(cn);
			return;
		}
		;
		if (prefix(cmd, "gcbuffs") && (IS_SEYAN_DU(cn) || IS_ANY_HARA(cn)))
		{
			do_listgcbuffs(cn, 0);
			return;
		}
		;
		if (prefix(cmd, "gcmax") && (IS_SEYAN_DU(cn) || IS_ANY_HARA(cn)))
		{
			do_listgcmax(cn, 0);
			return;
		}
		;
		if (prefix(cmd, "gctome") && !f_m)
		{
			do_gctome(cn);
			return;
		}
		;
		if (prefix(cmd, "gdept") && f_g)
		{
			break;
		}
		;
		if (prefix(cmd, "gdepth") && f_g)
		{
			god_set_depth(cn, dbatoi_self(cn, arg[1]), atoi(arg[2]), 1);
			return;
		}
		;
		if (prefix(cmd, "gold"))
		{
			do_gold(cn, atoi(arg[1]));
			return;
		}
		if (prefix(cmd, "golden") && f_g)
		{
			god_set_flag(cn, dbatoi(arg[1]), CF_GOLDEN);
			return;
		}
		if (prefix(cmd, "group") && !f_m)
		{
			do_group(cn, arg[1]);
			return;
		}
		if (prefix(cmd, "greataxe"))
		{
			do_listweapons(cn, "greataxe");
			return;
		}
		;
		if (prefix(cmd, "gargoyle") && f_gi)
		{
			god_gargoyle(cn);
			return;
		}
		if (prefix(cmd, "ggold") && f_g)
		{
			god_gold_char(cn, dbatoi_self(cn, arg[1]), atoi(arg[2]), arg[3]);
			return;
		}
		if (prefix(cmd, "give") && f_giu)
		{
			do_god_give(cn, dbatoi(arg[1]));
			return;
		}
		if (prefix(cmd, "goto") && f_giu) /*!*/
		{
			god_goto(cn, cn, arg[1], arg[2]);
			return;
		}
		if (prefix(cmd, "god") && f_g)
		{
			god_set_flag(cn, dbatoi_self(cn, arg[1]), CF_GOD);
			return;
		}
		if (prefix(cmd, "gorn"))
		{
			do_become_gornkwai(cn, 0);
			return;
		}
		;
		if (prefix(cmd, "greatergod") && f_gg)
		{
			god_set_flag(cn, dbatoi_self(cn, arg[1]), CF_GREATERGOD);
			return;
		}
		if (prefix(cmd, "greaterinv") && f_gg)
		{
			god_set_flag(cn, dbatoi_self(cn, arg[1]), CF_GREATERINV);
			return;
		}
		if (prefix(cmd, "grolm") && f_gi)
		{
			god_grolm(cn);
			return;
		}
		if (prefix(cmd, "grolminfo") && f_gi)
		{
			god_grolm_info(cn);
			return;
		}
		if (prefix(cmd, "grolmstart") && f_g)
		{
			god_grolm_start(cn);
			return;
		}
		break;
	case 'h':
		if (prefix(cmd, "help"))
		{
			do_help(cn, arg[1]);
			return;
		}
		break;
	case 'i':
		if (prefix(cmd, "ignore") && !f_m)
		{
			do_ignore(cn, arg[1], 0);
			return;
		}
		if (prefix(cmd, "iignore") && !f_m)
		{
			do_ignore(cn, arg[1], 1);
			return;
		}
		if (prefix(cmd, "iinfo") && f_g)
		{
			god_iinfo(cn, atoi(arg[1]));
			return;
		}
		if (prefix(cmd, "immortal") && f_u)
		{
			god_set_flag(cn, cn, CF_IMMORTAL);
			return;
		}
		if (prefix(cmd, "immortal") && f_g)
		{
			god_set_flag(cn, dbatoi_self(cn, arg[1]), CF_IMMORTAL);
			return;
		}
		if (prefix(cmd, "imp") && f_g)
		{
			god_set_flag(cn, dbatoi_self(cn, arg[1]), CF_IMP);
			return;
		}
		if (prefix(cmd, "info") && f_gius)
		{
			god_info(cn, dbatoi_self(cn, arg[1]));
			return;
		}
		if (prefix(cmd, "init") && f_g)
		{
			god_init_badnames();
			init_badwords();
			do_char_log(cn, 1, "Done.\n");
			return;
		}
		if (prefix(cmd, "infrared") && f_giu)
		{
			god_set_flag(cn, dbatoi_self(cn, arg[1]), CF_INFRARED);
			return;
		}
		if (prefix(cmd, "invisible") && f_giu)
		{
			god_set_flag(cn, dbatoi_self(cn, arg[1]), CF_INVISIBLE);
			return;
		}
		if (prefix(cmd, "ipshow") && f_giu)
		{
			do_list_net(cn, dbatoi(arg[1]));
			return;
		}
		if (prefix(cmd, "itell") && f_giu)
		{
			do_itell(cn, args[0]);
			return;
		}
		break;
	case 'k':
		if (prefix(cmd, "kick") && f_giu)
		{
			god_kick(cn, dbatoi(arg[1]));
			return;
		}
		;
		if (prefix(cmd, "knowspells") && (ch[cn].flags & CF_KNOWSPELL))
		{
			do_toggle_spellknowledge(cn);
			return;
		}
		;
		if (prefix(cmd, "kwai"))
		{
			do_become_gornkwai(cn, 1);
			return;
		}
		;
		break;
	case 'l':
		if (prefix(cmd, "lag") && !f_m)
		{
			do_lag(cn, atoi(arg[1]));
			return;
		}
		;
		if (prefix(cmd, "listskills"))
		{
			do_listskills(cn, arg[1]);
			return;
		}
		;
		if (prefix(cmd, "leave") && f_gi)
		{
			do_leave(cn);
			return;
		}
		if (prefix(cmd, "light") && f_c)
		{
			init_lights();
			return;
		}
		;
		if (prefix(cmd, "look") && f_gius)
		{
			do_look_char(cn, dbatoi_self(cn, arg[1]), 1, 0, 0);
			return;
		}
		;
		if (prefix(cmd, "lookdepot") && f_gg)
		{
			do_look_player_depot(cn, arg[1]);
			return;
		}
		;
		if (prefix(cmd, "lookinv") && f_gg)
		{
			do_look_player_inventory(cn, arg[1]);
			return;
		}
		;
		if (prefix(cmd, "lookequip") && f_gg)
		{
			do_look_player_equipment(cn, arg[1]);
			return;
		}
		;
		if (prefix(cmd, "looting") && f_g)
		{
			god_set_gflag(cn, GF_LOOTING);
			return;
		}
		;
		if (prefix(cmd, "lower") && f_g)
		{
			god_lower_char(cn, dbatoi_self(cn, arg[1]), atoi(arg[2]));
			return;
		}
		;
		if (prefix(cmd, "luck") && f_giu)
		{
			god_luck(cn, dbatoi_self(cn, arg[1]), atoi(arg[2]));
			return;
		}
		;
		if (prefix(cmd, "listban") && f_giu)
		{
			god_list_ban(cn);
			return;
		}
		;
		if (prefix(cmd, "listimps") && f_giu)
		{
			god_implist(cn);
			return;
		}
		;
		if (prefix(cmd, "listgolden") && f_giu)
		{
			do_list_all_flagged(cn, CF_GOLDEN);
			return;
		}
		;
		if (prefix(cmd, "listblack") && f_giu)
		{
			do_list_all_flagged(cn, CF_BLACK);
			return;
		}
		;
		break;
	case 'm':
		if (prefix(cmd, "mayhem") && f_g)
		{
			god_set_gflag(cn, GF_MAYHEM);
			return;
		}
		;
		if (prefix(cmd, "mark") && f_giu)
		{
			do_mark(cn, dbatoi(arg[1]), args[1]);
			return;
		}
		;
		if (prefix(cmd, "max"))
		{
			do_listmax(cn);
			return;
		}
		;
		if (prefix(cmd, "me"))
		{
			do_emote(cn, args[0]);
			return;
		}
		;
		if (prefix(cmd, "mirror") && f_giu)
		{
			god_mirror(cn, arg[1], arg[2]);
			return;
		}
		;
		if (prefix(cmd, "mailpas") && f_g)
		{
			break;
		}
		;
		if (prefix(cmd, "mailpass") && f_g)
		{
			god_mail_pass(cn, dbatoi(arg[1]));
			return;
		}
		break;
	case 'n':
		if (prefix(cmd, "noshout") && !f_m)
		{
			do_noshout(cn);
			return;
		}
		;
		if (prefix(cmd, "nostaff") && f_giu)
		{
			do_nostaff(cn);
			return;
		}
		;
		if (prefix(cmd, "notell") && !f_m)
		{
			do_notell(cn);
			return;
		}
		;
		if (prefix(cmd, "name") && f_giu)
		{
			god_set_name(cn, dbatoi(arg[1]), args[1]);
			return;
		}
		;
		if (prefix(cmd, "newbs") && f_g)
		{
			god_set_gflag(cn, GF_NEWBS);
			return;
		}
		;
		if (prefix(cmd, "nodesc") && f_giu)
		{
			god_reset_description(cn, dbatoi_self(cn, arg[1]));
			return;
		}
		;
		if (prefix(cmd, "nolist") && f_gi)
		{
			god_set_flag(cn, dbatoi(arg[1]), CF_NOLIST);
			return;
		}
		;
		if (prefix(cmd, "noluck") && f_giu)
		{
			god_luck(cn, dbatoi_self(cn, arg[1]), -atoi(arg[2]));
			return;
		}
		;
		if (prefix(cmd, "nowho") && f_gi)
		{
			god_set_flag(cn, dbatoi_self(cn, arg[1]), CF_NOWHO);
			return;
		}
		;
		if (prefix(cmd, "npclist") && f_giu)
		{
			do_npclist(cn, args[0]);
			return;
		}
		;
		break;
	case 'o':
		if (prefix(cmd, "opal"))
		{
			do_listrings(cn, "opal");
			return;
		}
		;
		if (prefix(cmd, "override") && !f_m)
		{
			do_override(cn);
			return;
		}
		;
	case 'p':
		if (prefix(cmd, "pentagrammas"))
		{
			show_pent_count(cn);
			return;
		}
		;
		if (prefix(cmd, "passwor") && f_g)
		{
			break;
		}
		;
		if (prefix(cmd, "password") && f_g)
		{
			god_change_pass(cn, dbatoi(arg[1]), arg[2]);
			return;
		}
		;
		if (prefix(cmd, "password"))
		{
			god_change_pass(cn, cn, arg[1]);
			return;
		}
		;
		/*
		if (prefix(cmd, "poh") && f_pol)
		{
			god_set_flag(cn, dbatoi(arg[1]), KIN_POH);
			return;
		}
		;
		if (prefix(cmd, "pol") && f_pol)
		{
			god_set_flag(cn, dbatoi(arg[1]), KIN_POH_LEADER);
			return;
		}
		;
		*/
		if (prefix(cmd, "poles"))
		{
			do_showpoles(cn, 0, arg[1]);
			return;
		}
		;
		if (prefix(cmd, "prof") && f_g)
		{
			god_set_flag(cn, cn, CF_PROF);
			return;
		}
		;
		if (prefix(cmd, "purple") && f_g)
		{
			god_set_purple(cn, dbatoi_self(cn, arg[1]));
			return;
		}
		;
		if (prefix(cmd, "purpl") && !f_g)
		{
			break;
		}
		;
		if (prefix(cmd, "purple") && !f_m&&!f_g)
		{
			do_become_purple(cn);
			return;
		}
		;
		if (prefix(cmd, "peras") && f_g)
		{
			break;
		}
		;
		if (prefix(cmd, "perase") && f_g)
		{
			god_erase(cn, dbatoi(arg[1]), 1);
			return;
		}
		;
		if (prefix(cmd, "pktcnt") && f_g)
		{
			pkt_list();
			return;
		}
		;
		if (prefix(cmd, "pktcl") && f_g)
		{
			cl_list();
			return;
		}
		;
		break;
	case 'q':
		if (prefix(cmd, "quest"))
		{
			do_questlist(cn, 0, arg[1]);
			return;
		}
		;
	case 'r':
		if (prefix(cmd, "rank"))
		{
			do_showrank(cn);
			return;
		}
		;
		if (prefix(cmd, "ranks"))
		{
			do_showranklist(cn);
			return;
		}
		;
		if (prefix(cmd, "rac"))
		{
			break;
		}
		;
		if (prefix(cmd, "rais"))
		{
			break;
		}
		;
		if (prefix(cmd, "resetnp"))
		{
			break;
		}
		;
		if (prefix(cmd, "resetite"))
		{
			break;
		}
		;
		if (prefix(cmd, "resetplaye"))
		{
			break;
		}
		;
		if (prefix(cmd, "resetticke"))
		{
			break;
		}
		;
		if (prefix(cmd, "raise") && f_giu)
		{
			god_raise_char(cn, dbatoi_self(cn, arg[1]), atoi(arg[2]), 0);
			return;
		}
		;
		if (prefix(cmd, "raisebs") && f_giu)
		{
			god_raise_char(cn, dbatoi_self(cn, arg[1]), atoi(arg[2]), 1);
			return;
		}
		;
		if (prefix(cmd, "raiseos") && f_giu)
		{
			god_raise_char(cn, dbatoi_self(cn, arg[1]), atoi(arg[2]), 2);
			return;
		}
		;
		if (prefix(cmd, "recall") && f_giu)
		{
			god_goto(cn, cn, "512", "512");
			return;
		}
		;
		if (prefix(cmd, "remsamenets") && f_giu)
		{
			char_remove_same_nets(cn, dbatoi_self(cn, arg[1]));
			return;
		}
		;
		if (prefix(cmd, "resetnpcs") && f_gg)
		{
			god_reset_npcs(cn);
			return;
		}
		;
		if (prefix(cmd, "resetitems") && f_gg)
		{
			god_reset_items(cn);
			return;
		}
		;
		if (prefix(cmd, "resetplayer") && f_gg)
		{
			god_reset_player(cn, atoi(arg[1]));
			return;
		}
		;
		if (prefix(cmd, "resetplayers") && f_gg)
		{
			god_reset_players(cn, atoi(arg[1]));
			return;
		}
		;
		if (prefix(cmd, "resetticker") && f_gg)
		{
			god_reset_ticker(cn);
			return;
		}
		;
		if (prefix(cmd, "respawn") && f_giu)
		{
			do_respawn(cn, atoi(arg[1]));
			return;
		}
		;
		if (prefix(cmd, "ring"))
		{
			do_listrings(cn, arg[1]);
			return;
		}
		;
		if (prefix(cmd, "ruby"))
		{
			do_listrings(cn, "ruby");
			return;
		}
		;
		break;
	case 's':
		if (prefix(cmd, "s"))
		{
			break;
		}
		;
		if (prefix(cmd, "sapphire"))
		{
			do_listrings(cn, "sapphire");
			return;
		}
		;
		if (prefix(cmd, "scbuffs") && (IS_SEYAN_DU(cn) || IS_ANY_HARA(cn)))
		{
			do_listgcbuffs(cn, 1);
			return;
		}
		;
		if (prefix(cmd, "scmax") && (IS_SEYAN_DU(cn) || IS_ANY_HARA(cn)))
		{
			do_listgcmax(cn, 1);
			return;
		}
		;
		if (prefix(cmd, "sense") && !f_m)
		{
			do_sense(cn);
			return;
		}
		;
		if (prefix(cmd, "setpoints") && f_giu)
		{
			god_give_points(cn, dbatoi_self(cn, arg[1]), atoi(arg[2]));
			return;
		}
		;
		if (prefix(cmd, "silence") && !f_m)
		{
			do_silence(cn);
			return;
		}
		;
		if (prefix(cmd, "shout"))
		{
			do_shout(cn, args[0]);
			return;
		}
		;
		if (prefix(cmd, "safe") && f_g)
		{
			god_set_flag(cn, dbatoi_self(cn, arg[1]), CF_SAFE);
			return;
		}
		;
		if (prefix(cmd, "save") && f_g)
		{
			god_save(cn, dbatoi_self(cn, arg[1]));
			return;
		}
		;
		if (prefix(cmd, "seen"))
		{
			do_seen(cn, arg[1]);
			return;
		}
		;
		if (prefix(cmd, "seeskills") && f_gius)
		{
			do_seeskills(cn, dbatoi_self(cn, arg[1]));
			return;
		}
		;
		if (prefix(cmd, "send") && f_g)
		{
			god_goto(cn, dbatoi(arg[1]), arg[2], arg[3]);
			return;
		}
		;
		if (prefix(cmd, "shrine") && IS_SEYAN_DU(cn))
		{
			do_showkwai(cn, arg[1]);
			return;
		}
		;
		if (prefix(cmd, "shutup") && f_gius)
		{
			god_shutup(cn, dbatoi_self(cn, arg[1]));
			return;
		}
		;
		if (prefix(cmd, "shield"))
		{
			do_listweapons(cn, "shield");
			return;
		}
		;
		if (prefix(cmd, "skill") && f_g)
		{
			god_skill(cn, dbatoi_self(cn, arg[1]), skill_lookup(arg[2]), atoi(arg[3]));
			return;
		}
		;
		if (prefix(cmd, "skua"))
		{
			do_become_skua(cn);
			return;
		}
		;
		if (prefix(cmd, "slap") && f_giu)
		{
			god_slap(cn, dbatoi_self(cn, arg[1]));
			return;
		}
		;
		if (prefix(cmd, "sort"))
		{
			do_sort(cn, arg[1]);
			return;
		}
		;
		if (prefix(cmd, "sortdepot"))
		{
			do_sort_depot(cn, arg[1], arg[2]);
			return;
		}
		;
		if (prefix(cmd, "soulstone") && f_g)
		{
			make_soulstone(cn, atoi(arg[1]));
			return;
		}
		;
		if (prefix(cmd, "spectate") && f_p)
		{
			do_spectate(cn, dbatoi_self(cn, arg[1]));
			return;
		}
		;
		if (prefix(cmd, "speedy") && f_g)
		{
			god_set_gflag(cn, GF_SPEEDY);
			return;
		}
		;
		if (prefix(cmd, "spellignore") && !f_m)
		{
			do_spellignore(cn);
			return;
		}
		;
		if (prefix(cmd, "spear"))
		{
			do_listweapons(cn, "spear");
			return;
		}
		;
		if (prefix(cmd, "sphalerite"))
		{
			do_listrings(cn, "sphalerite");
			return;
		}
		;
		if (prefix(cmd, "spinel"))
		{
			do_listrings(cn, "spinel");
			return;
		}
		;
		if (prefix(cmd, "sprite") && f_giu)
		{
			god_spritechange(cn, dbatoi(arg[1]), atoi(arg[2]));
			return;
		}
		;
		if (prefix(cmd, "sstone") && f_g)
		{
			do_make_sstone_gear(cn, skill_lookup(arg[1]), atoi(arg[2]));
			return;
		}
		if (prefix(cmd, "stell")&& f_gius)
		{
			do_stell(cn, args[0]);
			return;
		}
		;
		if (prefix(cmd, "stat") && f_g)
		{
			do_stat(cn);
			return;
		}
		;
		if (prefix(cmd, "staff"))
		{
			do_listweapons(cn, "staff");
			return;
		}
		;
		if (prefix(cmd, "staffer") && f_g)
		{
			god_set_flag(cn, dbatoi_self(cn, arg[1]), CF_STAFF);
			return;
		}
		;
		if (prefix(cmd, "steal") && f_gg)
		{
			do_steal_player(cn, arg[1], arg[2]);
			return;
		}
		;
		if (prefix(cmd, "stronghold") && f_g)
		{
			god_set_gflag(cn, GF_STRONGHOLD);
			return;
		}
		;
		if (prefix(cmd, "summon") && f_g)
		{
			god_summon(cn, arg[1], arg[2], arg[3]);
			return;
		}
		;
		
		if (prefix(cmd, "swap"))
		{
			do_swap_chars(cn);
			return;
		}
		;
		
		if (prefix(cmd, "sword"))
		{
			do_listweapons(cn, "sword");
			return;
		}
		;
		if (prefix(cmd, "sysoff") && !f_m)
		{
			do_sysoff(cn);
			return;
		}
		;
		break;
	case 't':
		if (prefix(cmd, "tell"))
		{
			do_tell(cn, arg[1], args[1]);
			return;
		}
		;
		if (prefix(cmd, "tarot"))
		{
			do_listtarots(cn, arg[1]);
			return;
		}
		;
		if (prefix(cmd, "tavern") && f_g && !f_m)
		{
			god_tavern(cn);
			return;
		}
		;
		if (prefix(cmd, "temple") && f_giu)
		{
			god_goto(cn, cn, "800", "800");
			return;
		}
		;
		if (prefix(cmd, "thrall") && f_giu)
		{
			god_thrall(cn, arg[1], arg[2], 0);
			return;
		}
		;
		if (prefix(cmd, "time"))
		{
			show_time(cn);
			return;
		}
		;
		if (prefix(cmd, "tinfo") && f_g)
		{
			god_tinfo(cn, atoi(arg[1]));
			return;
		}
		;
		if (prefix(cmd, "top") && f_g)
		{
			god_top(cn);
			return;
		}
		;
		if (prefix(cmd, "topaz"))
		{
			do_listrings(cn, "topaz");
			return;
		}
		;
		if (prefix(cmd, "trash"))
		{
			do_trash(cn);
			return;
		}
		;
		if (prefix(cmd, "twohander"))
		{
			do_listweapons(cn, "twohander");
			return;
		}
		;
		break;
	case 'u':
		if (prefix(cmd, "u"))
		{
			break;
		}
		;
		if (prefix(cmd, "unique") && f_g)
		{
			god_unique(cn);
			return;
		}
		;
		if (prefix(cmd, "usurp") && f_giu)
		{
			god_usurp(cn, dbatoi(arg[1]));
			return;
		}
		;
		break;
	case 'w':
		if (prefix(cmd, "who") && f_gius)
		{
			god_who(cn);
			return;
		}
		;
		if (prefix(cmd, "who"))
		{
			user_who(cn);
			return;
		}
		;
		if (prefix(cmd, "wave") && !f_sh)
		{
			ch[cn].misc_action = DR_WAVE;
			return;
		}
		;
		if (prefix(cmd, "weapon"))
		{
			do_listweapons(cn, arg[1]);
			return;
		}
		;
		if (prefix(cmd, "wipedeaths") && f_g)
		{
			do_wipe_deaths(cn, dbatoi_self(cn, arg[1]));
			return;
		}
		;
		if (prefix(cmd, "withdraw") && !f_m)
		{
			if (!isdigit(arg[1][0]))
				do_withdraw(cn, atoi(arg[2]), atoi(arg[3]), arg[1]);
			else
				do_withdraw(cn, atoi(arg[1]), atoi(arg[2]), "");
			return;
		}
		;
		if (prefix(cmd, "write") && f_giu)
		{
			do_create_note(cn, args[0]);
			return;
		}
		;
		break;
	case 'z':
		if (prefix(cmd, "zircon"))
		{
			do_listrings(cn, "zircon");
			return;
		}
		;
		break;
	}
	do_char_log(cn, 0, "Unknown command #%s\n", cmd);
}

void do_say(int cn, char *text)
{
	char *ptr;
	int   n, m, in;

	if (ch[cn].flags & CF_PLAYER)
	{
		player_analyser(cn, text);
	}

	if ((ch[cn].flags & CF_PLAYER) && *text!='|')
	{
		ch[cn].data[71] += CNTSAY;
		if (ch[cn].data[71]>MAXSAY)
		{
			do_char_log(cn, 0, "Oops, you're a bit too fast for me!\n");
			return;
		}
	}

	if (strcmp(text, "help")==0 && getrank(cn)<4)
	{
		do_char_log(cn, 0, "For a list of commands, use #help instead. If you need assistance, use #shout to ask everyone on the server.\n");
	}

	// direct log write from client
	if (*text=='|')
	{
		chlog(cn, "%s", text);
		return;
	}

	if (*text=='#' || *text=='/')
	{
		do_command(cn, text + 1);
		return;
	}

	ptr = text;

	if (ch[cn].flags & CF_SHUTUP)
	{
		do_char_log(cn, 0, "You try to say something, but you only produce a croaking sound.\n");
		return;
	}

	m = ch[cn].x + ch[cn].y * MAPX;
	if (map[m].flags & MF_UWATER)
	{
		for (n = 0; n<MAXBUFFS; n++)
		{
			if ((in = ch[cn].spell[n])!=0 && bu[in].temp==IT_GREENPILL) // speak underwater with a Green Pill
			{
				break;
			}
		}
		if (n==MAXBUFFS)
		{
			ptr = "Blub!";
		}
	}

	for (n = m = 0; text[n]; n++)
	{
		if (m==0 && isalpha(text[n]))
		{
			m++;
			continue;
		}
		if (m==1 && isalpha(text[n]))
		{
			continue;
		}
		if (m==1 && text[n]==':')
		{
			m++;
			continue;
		}
		if (m==2 && text[n]==' ')
		{
			m++;
			continue;
		}
		if (m==3 && text[n]=='"')
		{
			m++;
			break;
		}
		m = 0;
	}


	/* CS, 991113: Enable selective seeing of an invisible players' name */
	if (ch[cn].flags & (CF_PLAYER | CF_USURP))
	{
		do_area_say1(cn, ch[cn].x, ch[cn].y, ptr);
	}
	else
	{
		do_area_log(0, 0, ch[cn].x, ch[cn].y, 1, "%.30s: \"%.300s\"\n", ch[cn].name, ptr);
	}

	if (m==4)
	{
		god_slap(0, cn);
		chlog(cn, "Punished for trying to fake another character");
	}
	if (ch[cn].flags & (CF_PLAYER | CF_USURP))
	{
		chlog(cn, "Says \"%s\" %s", text, (ptr!=text ? ptr : ""));
	}

	/* support for riddles (lab 9) */
	(void) lab9_guesser_says(cn, text);
}

void process_options(int cn, char *buf)
{
	char *ptr = buf;
	int   s = 0;

	if (*buf=='#')
	{
		ptr++;
		s = atoi(ptr);
		while (isdigit(*ptr))
		{
			ptr++;
		}
		while (*ptr=='#')
		{
			ptr++;
		}

		memmove(buf, ptr, strlen(ptr) + 1);

		if (s)
		{
			do_area_sound(cn, 0, ch[cn].x, ch[cn].y, s);
		}
	}
}

void do_sayx(int cn, char *format, ...)
{
	va_list args;
	char buf[1024];

	va_start(args, format);
	vsprintf(buf, format, args);
	va_end(args);

	process_options(cn, buf);

	if (ch[cn].flags & (CF_PLAYER))
	{
		do_area_log(0, 0, ch[cn].x, ch[cn].y, 3, "%.30s: \"%.300s\"\n", ch[cn].name, buf);
	}
	else
	{
		do_area_log(0, 0, ch[cn].x, ch[cn].y, 1, "%.30s: \"%.300s\"\n", ch[cn].name, buf);
	}
}

int do_char_score(int cn)
{
	return((int)(sqrt(ch[cn].points_tot)) / 7 + 7);
}

void remove_enemy(int co)
{
	int n, m;

	for (n = 1; n<MAXCHARS; n++)
	{
		for (m = 0; m<4; m++)
		{
			if (ch[n].enemy[m]==co)
			{
				ch[n].enemy[m] = 0;
			}
		}
	}
}

// "get" functions

int get_attrib_score(int cn, int n)
{
	return ( (ch[cn].attrib[n][4] << 8) | ch[cn].attrib[n][5] );
}
void set_attrib_score(int cn, int z, int n)
{
	if (n<1)
	{
		n = 1;
	}
	else if (n>C_AT_CAP(cn, z))
	{
		n = C_AT_CAP(cn, z);
	}
	
	ch[cn].attrib[z][4] = (n >> 8) & 0xFF;
	ch[cn].attrib[z][5] = n & 0xFF;
}
int get_skill_score(int cn, int n)
{
	if (n > 49)
	{
		return min(300, max(1, (getrank(cn)+1)*8));
	}
	return ( (ch[cn].skill[n][4] << 8) | ch[cn].skill[n][5] );
}
void set_skill_score(int cn, int z, int n)
{
	if (n<1)
	{
		n = 1;
	}
	else if (n>C_AT_CAP(cn, 5))
	{
		n = C_AT_CAP(cn, 5);
	}
	
	ch[cn].skill[z][4] = (n >> 8) & 0xFF;
	ch[cn].skill[z][5] = n & 0xFF;
}

int get_fight_skill(int cn, int skill[50])
{
	int co=0, in, n, m[6];
	
	if (IS_COMP_TEMP(cn) && IS_SANECHAR(co = ch[cn].data[CHD_MASTER]) && ch[cn].data[1]==4)
		cn = co;
	
	in = ch[cn].worn[WN_RHAND];
	
	if (!in)
	{
		return min(C_AT_CAP(cn, 5), skill[SK_HAND]);
	}
	
	// Rather than pick the matching skill, pick the highest available one
	if (it[in].temp==IT_TW_HEAVENS || it[in].orig_temp==IT_SEYANSWORD)
	{
		m[0] = skill[SK_HAND];
		m[1] = skill[SK_DAGGER];
		m[2] = skill[SK_SWORD];
		m[3] = skill[SK_AXE];
		m[4] = skill[SK_STAFF];
		m[5] = skill[SK_TWOHAND];
		for (n = 1; n < 6; ++n) if (m[0] < m[n]) m[0] = m[n];
		return min(C_AT_CAP(cn, 5), m[0]);
	}
	
	if (IS_WPCLAW(in)) 
	{
		return min(C_AT_CAP(cn, 5), skill[SK_HAND]);
	}

	if (IS_WPSWORD(in))
	{
		return min(C_AT_CAP(cn, 5), skill[SK_SWORD]);
	}
	
	if (IS_WPSPEAR(in)) // Spear
	{
		return min(C_AT_CAP(cn, 5), skill[SK_DAGGER] > skill[SK_STAFF] ? skill[SK_DAGGER] : skill[SK_STAFF]);
	}
	
	if (IS_WPDAGGER(in))
	{
		return min(C_AT_CAP(cn, 5), skill[SK_DAGGER]);
	}
	if (IS_WPSTAFF(in))
	{
		return min(C_AT_CAP(cn, 5), skill[SK_STAFF]);
	}
	
	if (IS_WPGAXE(in)) // Greataxe
	{
		return min(C_AT_CAP(cn, 5), skill[SK_AXE] > skill[SK_TWOHAND] ? skill[SK_AXE] : skill[SK_TWOHAND]);
	}
	if (IS_WPAXE(in))
	{
		return min(C_AT_CAP(cn, 5), skill[SK_AXE]);
	}
	if (IS_WPTWOHAND(in))
	{
		return min(C_AT_CAP(cn, 5), skill[SK_TWOHAND]);
	}
	
	if (IS_WPSHIELD(in))
	{
		return min(C_AT_CAP(cn, 5), skill[SK_SHIELD]);
	}

	return min(C_AT_CAP(cn, 5), skill[SK_HAND]);
}

// Dual Wield and Shield skill checks
int get_combat_skill(int cn, int skill[50], int flag)
{
	int co, power, bonus=0;
	
	if (IS_COMP_TEMP(cn) && IS_SANECHAR(co = ch[cn].data[CHD_MASTER]) && ch[cn].data[1]==4)
		cn = co;
	
	if (flag)	power = min(C_AT_CAP(cn, 5), skill[SK_DUAL]);
	else		power = min(C_AT_CAP(cn, 5), skill[SK_SHIELD]);
	
	if (IS_PLAYER(cn))
	{
		if (IS_SEYAN_DU(cn) || IS_ANY_MERC(cn) || IS_BRAVER(cn))	bonus = power / 6;
		else if (!flag && T_ARTM_SK(cn, 12))						bonus = power / 8 + power /16;
		else														bonus = power / 8;
	}
	else															bonus = power /10;

	return bonus;
}
int get_offhand_skill(int cn, int skill[50], int flag)
{
	int co = 0, n = 0, in, in2, in3; 
	
	if (IS_COMP_TEMP(cn) && IS_SANECHAR(co = ch[cn].data[CHD_MASTER]) && ch[cn].data[1]==4)
		cn = co;
	
	in  = ch[cn].worn[WN_LHAND];
	in2 = ch[cn].worn[WN_BELT];
	in3 = ch[cn].worn[WN_RHAND];
	
	// Dual Shield
	if (!flag && in3 && (it[in3].flags & IF_OF_SHIELD))
	{
		n = get_combat_skill(cn, skill, flag)/2;
	}
	
	// Belt - Black Belt :: Shield parry bonus while offhand is empty
	if (!flag && !in && in2 && (it[in2].temp == IT_TW_BBELT))
	{
		return ( n + get_combat_skill(cn, skill, flag)/2 );
	}
	
	// No Gear? No bonus
	if (!in  || (flag 	&& !(it[in].flags & IF_OF_DUALSW ) && !(it[in].flags & IF_WP_DAGGER )) || 
				(!flag 	&& !(it[in].flags & IF_OF_SHIELD ) && !(it[in].temp==IT_WP_QUICKSILV))  )
	{
		return n;
	}
	
	return ( n + get_combat_skill(cn, skill, flag) );
}

// put in an item, see if we're wearing it in a charm slot.
int get_tarot(int cn, int in)
{
	int ch1=0, ch2=0, ch3=0, cc=0;
	
	// Let GC copy card effects (summ tree)
	if (IS_COMP_TEMP(cn) && IS_SANECHAR(cc = ch[cn].data[CHD_MASTER]) && (T_SUMM_SK(cc, 10) || ch[cn].data[1]==4))
		cn = cc;
	
	if (ch[cn].flags & CF_NOMAGIC) return 0;
	
	ch1 = ch[cn].worn[WN_CHARM];
	ch2 = ch[cn].worn[WN_CHARM2];
	
	if (IS_SINBINDER(ch[cn].worn[WN_LRING]) && it[ch[cn].worn[WN_LRING]].data[1]==1)
		ch3 = it[ch[cn].worn[WN_LRING]].data[2];
	if (IS_SINBINDER(ch[cn].worn[WN_RRING]) && it[ch[cn].worn[WN_RRING]].data[1]==1)
		ch3 = it[ch[cn].worn[WN_RRING]].data[2];
	
	if ((ch1 && it[ch1].temp==in) || (ch2 && it[ch2].temp==in) || (ch3==in)) 
		return 1;
	
	return 0;
}

// put in an item, see if we're wearing it in the left-hand slot.
int get_book(int cn, int in)
{
	int co=0, in2;
	
	if (IS_COMP_TEMP(cn) && IS_SANECHAR(co = ch[cn].data[CHD_MASTER]) && ch[cn].data[1]==4)
		cn = co;
	
	if (ch[cn].flags & CF_NOMAGIC) return 0;
	
	in2 = ch[cn].worn[WN_RHAND];
	
	if (in2 && (it[in2].temp == in || it[in2].orig_temp == in)) 
		return in2;
	
	in2 = ch[cn].worn[WN_LHAND];
	
	if (in2 && (it[in2].temp == in || it[in2].orig_temp == in)) 
		return in2;
	
	return 0;
}

// put in an item, see if we're wearing it in the amulet slot
int get_neck(int cn, int in)
{
	int co=0, in2;
	
	if (IS_COMP_TEMP(cn) && IS_SANECHAR(co = ch[cn].data[CHD_MASTER]) && ch[cn].data[1]==4)
		cn = co;
	
	in2 = ch[cn].worn[WN_NECK];
	
	if (in2 && (it[in2].temp == in || it[in2].orig_temp == in)) 
		return in2;
	
	return 0;
}

// put in an item, see if we're wearing that in any slot
int get_gear(int cn, int in)
{
	int co=0, n, in2;
	
	if (IS_COMP_TEMP(cn) && IS_SANECHAR(co = ch[cn].data[CHD_MASTER]) && ch[cn].data[1]==4)
		cn = co;
	
	for (n = 0; n <= WN_CHARM2; n++)
	{
		if ((in2 = ch[cn].worn[n]) && (it[in2].temp == in || it[in2].orig_temp == in))
			return in2;
	}
	
	return 0;
}

int get_enchantment(int cn, int in)
{
	int co=0, n, in2, m = 0;
	
	if (IS_COMP_TEMP(cn) && IS_SANECHAR(co = ch[cn].data[CHD_MASTER]) && ch[cn].data[1]==4)
		cn = co;
	
	for (n = 0; n <= WN_CHARM2; n++)
	{
		if ((in2 = ch[cn].worn[n]) && it[in2].enchantment == in) m += (IS_TWOHAND(in2)?2:1);
	}
	
	return m;
}

int has_item(int cn, int temp)
{
	int n, in;
	
	if (IS_BUILDING(cn)) return 0;
	
	in = ch[cn].citem;
	if (in & 0x80000000) ;
	else if (IS_SANEITEM(in) && it[in].temp==temp) return in;
	for (n = 0; n<MAXITEMS; n++)
	{
		in = ch[cn].item[n];
		if (in & 0x80000000) continue;
		if (IS_SANEITEM(in) && it[in].temp==temp)  return in;
	}
	for (n = 0; n<WN_CHARM2; n++)
	{
		in = ch[cn].worn[n];
		if (IS_SANEITEM(in) && it[in].temp==temp)  return in;
	}
	for (n = 0; n<12; n++)
	{
		in = ch[cn].alt_worn[n];
		if (IS_SANEITEM(in) && it[in].temp==temp)  return in;
	}
	/*
	for (n = 0; n<4; n++)
	{
		in = ch[cn].blacksmith[n];
		if (IS_SANEITEM(in) && it[in].temp==temp)  return in;
	}
	*/
	
	return 0;
}

//
int is_apotion(int in)
{
	static int potions[] = {
		IT_POT_M_HP, IT_POT_N_HP, IT_POT_G_HP, IT_POT_H_HP, 
		IT_POT_S_HP, IT_POT_C_HP, IT_POT_L_HP, IT_POT_D_HP, 
		IT_POT_M_EN, IT_POT_N_EN, IT_POT_G_EN, IT_POT_H_EN, 
		IT_POT_S_EN, IT_POT_C_EN, IT_POT_L_EN, IT_POT_D_EN, 
		IT_POT_M_MP, IT_POT_N_MP, IT_POT_G_MP, IT_POT_H_MP, 
		IT_POT_S_MP, IT_POT_C_MP, IT_POT_L_MP, IT_POT_D_MP, 
		IT_POT_VITA, IT_POT_CLAR, IT_POT_SAGE, IT_POT_LIFE, 
		IT_POT_T, IT_POT_O, IT_POT_PT, IT_POT_PO, 
		IT_POT_LAB2, IT_POT_GOLEM, 
		IT_POT_BRV, IT_POT_WIL, IT_POT_INT, IT_POT_AGL, IT_POT_STR, 
		IT_POT_EXHP, IT_POT_EXEN, IT_POT_EXMP, 
		IT_POT_PRE, IT_POT_EVA, IT_POT_MOB, IT_POT_FRE, IT_POT_MAR, 
		IT_POT_IMM, IT_POT_CLA, IT_POT_THO, IT_POT_BRU, IT_POT_RES, 
		IT_POT_APT, IT_POT_OFF, IT_POT_DEF, IT_POT_PER, IT_POT_STE, 
		IT_POT_RAIN };
	int tn, n;

	tn = it[in].temp;
	for (n = 0; n<ARRAYSIZE(potions); n++)
	{
		if (tn == potions[n])
		{
			return 1;
		}
	}
	return 0;
}

int is_ascroll(int in)
{
	int tn;
	tn = it[in].temp;
	return (((tn >= 1314) && (tn <= 1341)) || ((tn >= 182) && (tn <= 189)));
}

char itemvowelbuf[80];

char *itemvowel(int in, int flag) // flag 0 = name, flag 1 = reference
{
	char name[40];
	
	if (!IS_SANEITEM(in))
	{
		xlog("bad item in *itemvowel");
		return "";
	}
	
	if (flag)
		strcpy(name, it[in].reference);
	else
		strcpy(name, it[in].name);
	
	switch (name[0])
	{
		case 'A': case 'E': case 'I': case 'O': case 'U':
		case 'a': case 'e': case 'i': case 'o': case 'u': sprintf(itemvowelbuf, "an %s", name); break;
		default: sprintf(itemvowelbuf, "a %s", name); break;
	}
	
	return itemvowelbuf;
}

char *drvowel(int in, int perc, int pchk)
{
	if ((perc-100)>pchk)
	{
		if (it[in].orig_temp) switch (it_temp[it[in].orig_temp].name[0])
		{
			case 'A': case 'E': case 'I': case 'O': case 'U':
			case 'a': case 'e': case 'i': case 'o': case 'u': return "n";
			default: return "";
		}
		else if (it[in].temp) switch (it_temp[it[in].temp].name[0])
		{
			case 'A': case 'E': case 'I': case 'O': case 'U':
			case 'a': case 'e': case 'i': case 'o': case 'u': return "n";
			default: return "";
		}
	}
	else
	{
		if      (IS_EQHEAD(in))    return ""; // "helmet";
		else if (IS_EQNECK(in))    return ""; // "necklace";
		else if (IS_EQBODY(in))    return ""; // "body armor";
		else if (IS_EQARMS(in))    return ""; // "pair of gloves";
		else if (IS_EQBELT(in))    return ""; // "magical belt";
		else if (IS_EQCHARM(in))   return ""; // "tarot card";
		else if (IS_EQFEET(in))    return ""; // "pair of boots";
		else if (IS_EQWEAPON(in))  return ""; // "weapon";
		else if (IS_EQDUALSW(in))  return ""; // "weapon";
		else if (IS_EQSHIELD(in))  return ""; // "shield";
		else if (IS_EQCLOAK(in))   return ""; // "cloak";
		else if (IS_EQRING(in))    return ""; // "magical ring";
		else if (IS_SOULSTONE(in)) return ""; // "soulstone";
		else if (IS_SOULFOCUS(in)) return ""; // "soul focus";
		else if (IS_SOULCAT(in))   return ""; // "soul catalyst";
		else if (IS_CONTRACT(in))  return ""; // "contract";
		else if (IS_QUILL(in))     return ""; // "quill";
		else if (IS_GEMSTONE(in))  return ""; // "gemstone";
		else if (IS_POTION(in))    return ""; // "magical potion";
		else if (IS_SCROLL(in))    return ""; // "magical scroll";
	}
	return "n"; // "item";
}

char *drtype(int in, int perc, int pchk)
{
	if ((perc-100)>pchk)
	{
		if (it[in].orig_temp)      return it_temp[it[in].orig_temp].name;
		else if (it[in].temp)      return it_temp[it[in].temp].name;
	}
	else
	{
		if      (IS_EQHEAD(in))    return "helmet";
		else if (IS_EQNECK(in))    return "necklace";
		else if (IS_EQBODY(in))    return "body armor";
		else if (IS_EQARMS(in))    return "pair of gloves";
		else if (IS_EQBELT(in))    return "magical belt";
		else if (IS_EQCHARM(in))   return "tarot card";
		else if (IS_EQFEET(in))    return "pair of boots";
		else if (IS_EQWEAPON(in))  return "weapon";
		else if (IS_EQDUALSW(in))  return "weapon";
		else if (IS_EQSHIELD(in))  return "shield";
		else if (IS_EQCLOAK(in))   return "cloak";
		else if (IS_EQRING(in))    return "magical ring";
		else if (IS_SOULSTONE(in)) return "soulstone";
		else if (IS_SOULFOCUS(in)) return "soul focus";
		else if (IS_SOULCAT(in))   return "soul catalyst";
		else if (IS_CONTRACT(in))  return "contract";
		else if (IS_QUILL(in))     return "quill";
		else if (IS_GEMSTONE(in))  return "gemstone";
		else if (IS_POTION(in))    return "magical potion";
		else if (IS_SCROLL(in))    return "magical scroll";
	}
	return "item";
}

// For examining a corpse for special stuff at a glance with Sense Magic.
// msg must be a do_char_log() format string like "you see %s in the corpse.\n".
void do_ransack_corpse(int cn, int co, char *msg)
{
	int in, n, colr, perc, pchk=0;
	char dropped[100];
	
	if (!(ch[cn].flags & CF_SENSE)) return;
	
	perc = M_SK(cn, SK_PERCEPT);
	
	for (n = 0; n<20; n++)
	{
		if (!IS_SANEITEM(in = ch[co].worn[n])) continue;
		if (IS_RANSACKGEAR(in) && perc > RANDOM(200))
		{
			colr = -1; pchk = RANDOM(200);
			if      (IS_UNIQUE(in))      { colr = 9; sprintf(dropped, "%s %s (UNQ)", drvowel(in, perc, pchk), drtype(in, perc, pchk)); }
			else if (IS_OSIRWEAP(in))    { colr = 9; sprintf(dropped, "%s %s (OSI)", drvowel(in, perc, pchk), drtype(in, perc, pchk)); }
			else if (IS_SOULCHANTED(in)) { colr = 7; sprintf(dropped, "%s %s (S&E)", drvowel(in, perc, pchk), drtype(in, perc, pchk)); }
			else if (IS_SOULSTONED(in))  { colr = 7; sprintf(dropped, "%s %s (SS)",  drvowel(in, perc, pchk), drtype(in, perc, pchk)); }
			else if (IS_ENCHANTED(in))   { colr = 8; sprintf(dropped, "%s %s (EN)",  drvowel(in, perc, pchk), drtype(in, perc, pchk)); }
			else                         { colr = 0; sprintf(dropped, "%s %s",       drvowel(in, perc, pchk), drtype(in, perc, pchk)); }
			if (colr > -1) do_char_log(cn, colr, msg, dropped);
		}
	}
	for (n = 0; n<MAXITEMS; n++)
	{
		if (!IS_SANEITEM(in = ch[co].item[n])) continue;
		if (perc > RANDOM(200))
		{
			colr = -1; pchk = RANDOM(200);
			if      (IS_UNIQUE(in))      { colr = 9; sprintf(dropped, "%s %s (UNQ)", drvowel(in, perc, pchk), drtype(in, perc, pchk)); }
			else if (IS_OSIRWEAP(in))    { colr = 9; sprintf(dropped, "%s %s (OSI)", drvowel(in, perc, pchk), drtype(in, perc, pchk)); }
			else if (IS_SOULCHANTED(in)) { colr = 7; sprintf(dropped, "%s %s (S&E)", drvowel(in, perc, pchk), drtype(in, perc, pchk)); }
			else if (IS_SOULSTONED(in))  { colr = 7; sprintf(dropped, "%s %s (SS)",  drvowel(in, perc, pchk), drtype(in, perc, pchk)); }
			else if (IS_ENCHANTED(in))   { colr = 8; sprintf(dropped, "%s %s (EN)",  drvowel(in, perc, pchk), drtype(in, perc, pchk)); }
			else if (IS_SOULSTONE(in))   { colr = 5; sprintf(dropped, "%s %s",       drvowel(in, perc, pchk), drtype(in, perc, pchk)); }
			else if (IS_SOULFOCUS(in))   { colr = 5; sprintf(dropped, "%s %s",       drvowel(in, perc, pchk), drtype(in, perc, pchk)); }
			else if (IS_SOULCAT(in))     { colr = 5; sprintf(dropped, "%s %s",       drvowel(in, perc, pchk), drtype(in, perc, pchk)); }
			else if (IS_SCROLL(in))      { colr = 4; sprintf(dropped, "%s %s",       drvowel(in, perc, pchk), drtype(in, perc, pchk)); }
			else if (IS_POTION(in))      { colr = 6; sprintf(dropped, "%s %s",       drvowel(in, perc, pchk), drtype(in, perc, pchk)); }
			else if (IS_CONTRACT(in))    { colr = 1; sprintf(dropped, "%s %s",       drvowel(in, perc, pchk), drtype(in, perc, pchk)); }
			else if (IS_QUILL(in))       { colr = 1; sprintf(dropped, "%s %s",       drvowel(in, perc, pchk), drtype(in, perc, pchk)); }
			else if (IS_MAGICDROP(in))   { colr = 0; sprintf(dropped, "%s %s",       drvowel(in, perc, pchk), drtype(in, perc, pchk)); }
			if (colr > -1) do_char_log(cn, colr, msg, dropped);
		}
	}
}

void do_item_kills(int cn)
{
	int in;
	
	if (in = get_gear(cn, IT_WP_LAMEDARG)) it[in].data[0]++;
	if ((in = ch[cn].worn[WN_RHAND]) && ((IS_GODWEAPON(in) && it[in].stack < 10) || (IS_OSIRWEAP(in) && it[in].stack > 0)))
	{
		it[in].cost++;
		do_check_new_item_level(cn, in);
	}
	if ((in = ch[cn].worn[WN_LHAND]) && ((IS_GODWEAPON(in) && it[in].stack < 10) || (IS_OSIRWEAP(in) && it[in].stack > 0)))
	{
		it[in].cost++;
		do_check_new_item_level(cn, in);
	}
}

// note: cn may be zero!!
void do_char_killed(int cn, int co, int pentsolve)
{
	int n, in, x, y, z, temp = 0, m, tmp, tmpg, wimp, cc = 0, fn, r1, r2, rank, grouped = 0;
	unsigned long long mf1, mf2;
	unsigned char buf[3];
	int os;

	do_notify_char(co, NT_DIED, cn, 0, 0, 0);

	if (cn)
	{
		chlog(cn, "Killed %s (%d)", ch[co].name, co);
	}
	else
	{
		chlog(co, "Died");
	}
	
	mf1 = mf2 = map[XY2M(ch[co].x, ch[co].y)].flags;
	if (cn)
	{
		mf2 &= map[XY2M(ch[cn].x, ch[cn].y)].flags;
	}
	
	// Plague spreading
	if ((in = has_buff(co, SK_PLAGUE)) && IS_SANECHAR(bu[in].data[0]))
	{
		skill_plague(bu[in].data[0], co, bu[in].data[5]);
	}

	// hack for grolms
	if (ch[co].sprite==12240)
	{
		do_area_sound(co, 0, ch[co].x, ch[co].y, 17);
		char_play_sound(co, 17, -150, 0);
	}
	// hack for gargoyles
	else if (ch[co].sprite==18384 || ch[co].sprite==21456)
	{
		do_area_sound(co, 0, ch[co].x, ch[co].y, 18);
		char_play_sound(co, 18, -150, 0);
	}
	else if (IS_PLAYER(co) || ch[co].data[25]!=4) // Hack so Shiva doesn't groan on death
	{
		do_area_sound(co, 0, ch[co].x, ch[co].y, ch[co].sound + 2);
		char_play_sound(co, ch[co].sound + 2, -150, 0);
	}
	
	if (ch[co].gcm == 9 && ch[co].data[49])
	{
		it[ch[co].data[49]].active = TICKS * 60;
	}

	// cleanup for ghost companions
	if (IS_COMP_TEMP(co))
	{
		cc = ch[co].data[CHD_MASTER];
		if (IS_SANECHAR(cc))
		{
			if (ch[cc].data[PCD_COMPANION] == co)
			{
				ch[cc].data[PCD_COMPANION] = 0;
			}
			else if (ch[cc].data[PCD_SHADOWCOPY] == co)
			{
				ch[cc].data[PCD_SHADOWCOPY] = 0;
				remove_shadow(cc);
			}
		}
		ch[co].data[CHD_MASTER] = 0;
	}
	
	// Un-wedge doors
	if (!IS_PLAYER(co) && (ch[co].data[26])) npc_wedge_doors(ch[co].data[26], 0);
	
	// Special case for Pandium
	if (co && ch[co].temp==CT_PANDIUM)
	{
		if (ch[co].data[32])
		{
			char message[4][120];
			tmp = ch[co].data[1];
			for (n=0;n<ch[co].data[32];n++) 
			{
				if (IS_LIVINGCHAR(cc = ch[co].data[7+n]) && IS_PLAYER(cc) && is_atpandium(cc))
				{
					remove_buff(cc, SK_OPPRESSED);
					if (ch[co].data[32]==1)
					{
						tmpg = max(ch[cc].pandium_floor[0], tmp+1);
						ch[cc].pandium_floor[0] = tmpg;
					}
					else
					{
						tmpg = max(ch[cc].pandium_floor[1], tmp+1);
						ch[cc].pandium_floor[1] = tmpg;
					}
					if (tmpg > tmp+1) tmpg = 1;
					
					switch (n)
					{
						case  2:
							spawn_pandium_rewards(cc, tmpg-1, 283, 959);
							quick_teleport(cc, 283, 955);
							break;
						case  1:
							spawn_pandium_rewards(cc, tmpg-1, 273, 959);
							quick_teleport(cc, 273, 955);
							break;
						default:
							spawn_pandium_rewards(cc, tmpg-1, 293, 959);
							quick_teleport(cc, 293, 955);
							break;
					}
					if (tmp>=50 && tmp%5==0)
						do_char_log(cc, 9, "Pandium: \"Claim your crown.\"\n");
					else if (tmp==1 || tmp==10 || tmp==20 || tmp==30 || tmp==40)
						do_char_log(cc, 7, "Pandium: \"May we grow ever stronger.\"\n");
					else
						do_char_log(cc, 3, "Pandium: \"Rise ever higher.\"\n");
					if (tmp%10==0)
						sprintf(message[n], "%s", ch[cc].name);
				}
			}
			if (tmp%10==0)
			{
				if (ch[co].data[32] == 3)
					sprintf(message[3], "%s, %s, and %s defeated The Archon Pandium at depth %d!", message[0], message[1], message[2], tmp);
				else if (ch[co].data[32] == 2)
					sprintf(message[3], "%s and %s defeated The Archon Pandium at depth %d!", message[0], message[1], tmp);
				else
					sprintf(message[3], "%s defeated The Archon Pandium at depth %d!", message[0], tmp);
				if (globs->flags & GF_DISCORD) discord_ranked(message[3]);
			}
		}
	}
	
	// Special case for Gatekeeper
	if (co && ch[co].temp==CT_LAB20_KEEP)
	{
		if (ch[co].data[0] && IS_LIVINGCHAR(cc = ch[co].data[0]) && IS_PLAYER(cc) && IS_IN_TLG(ch[cc].x, ch[cc].y))
		{
			do_char_log(cc, 3, "Gatekeeper: \"%s\"\n", "Well done!");
			do_char_log(cc, 0, "You have solved the final part of the Labyrinth.\n");
			chlog(cc, "Solved Labyrinth Part 20");
			do_char_log(cc, 7, "You earned 1 skill point.\n");
			ch[cc].tree_points++;
			
			for (n = 0; n<MAXBUFFS; n++)
			{
				if ((m = ch[cc].spell[n]))
				{
					ch[cc].spell[n] = 0;
					bu[m].used = USE_EMPTY;
					do_char_log(cc, 1, "Your %s vanished.\n", bu[m].name);
				}
			}
			
			if (IS_PURPLE(cc)) { x = HOME_PURPLE_X; y = HOME_PURPLE_Y; }
			else if (IS_CLANKWAI(cc)) { x = HOME_KWAI_X; y = HOME_KWAI_Y; }
			else if (IS_CLANGORN(cc)) { x = HOME_GORN_X; y = HOME_GORN_Y; }
			else if (ch[cc].flags & CF_STAFF) { x = HOME_STAFF_X; y = HOME_STAFF_Y; }
			else { x = HOME_TEMPLE_X; y = HOME_TEMPLE_Y; }
			
			fx_add_effect(6, 0, ch[cc].x, ch[cc].y, 0);
			god_transfer_char(cc, x, y);
			char_play_sound(cc, ch[cc].sound + 22, -150, 0);
			fx_add_effect(6, 0, ch[cc].x, ch[cc].y, 0);

			ch[cc].temple_x = ch[cc].tavern_x = ch[cc].x;
			ch[cc].temple_y = ch[cc].tavern_y = ch[cc].y;
		}
	}

	// a player killed someone or something.
	if (cn && cn!=co && (ch[cn].flags & (CF_PLAYER)) && !(mf2 & MF_ARENA))
	{
		ch[cn].alignment -= ch[co].alignment / 50;
		if (ch[cn].alignment>7500)
		{
			ch[cn].alignment = 7500;
		}
		if (ch[cn].alignment<-7500)
		{
			ch[cn].alignment = -7500;
		}
		
		// Contract kill progress
		if (CONT_NUM(cn)) 
		{
			os = 0;
			switch (CONT_SCEN(cn))
			{
				case  1: if (IS_CON_NME(co)) { add_map_progress(CONT_NUM(cn)); os++; } break;
				case  2: if (IS_CON_DIV(co)) { add_map_progress(CONT_NUM(cn)); os++; } break;
				case  3: if (IS_CON_CRU(co)) { add_map_progress(CONT_NUM(cn)); os++; } break;
				case  8: if (IS_CON_COW(co)) { add_map_progress(CONT_NUM(cn)); os++; } break;
				case  9: if (IS_CON_UNI(co)) { add_map_progress(CONT_NUM(cn)); os++; } break;
				default: break;
			}
			if (os && CONT_PROG(cn)>=CONT_GOAL(cn)) 
				do_char_log(cn, 2, "That's all of them! You're good to go!\n");
			else if (os)
				do_char_log(cn, 1, "%d down, %d to go.\n", CONT_PROG(cn), (CONT_GOAL(cn)-CONT_PROG(cn)));
		}
		
		do_item_kills(cn);
		
		// becoming purple
		if (ch[co].temp==CT_PRIEST)   // add all other priests of the purple one here...
		{
			if (IS_PURPLE(cn))
			{
				do_char_log(cn, 1, "Ahh, that felt good!\n");
			}
			else
			{
				ch[cn].data[65] = globs->ticker;
				do_char_log(cn, 0, "So, you want to be a player killer, right?\n");
				do_char_log(cn, 0, "To join the purple one and be a killer, type #purple now.\n");
				fx_add_effect(6, 0, ch[cn].x, ch[cn].y, 0);
			}
		}

		if (!(ch[co].flags & (CF_PLAYER)) && ch[co].alignment==10000) // shopkeepers & questgivers
		{
			do_char_log(cn, 0, "You feel a god look into your soul. He seems to be angry.\n");

			ch[cn].data[40]++;
			if (ch[cn].data[40]<50)
			{
				tmp = -ch[cn].data[40] * 100;
			}
			else
			{
				tmp = -5000;
			}
			ch[cn].luck += tmp;
			chlog(cn, "Reduced luck by %d to %d for killing %s (%d, t=%d)", tmp, ch[cn].luck, ch[co].name, co, ch[co].temp);
		}

		// update statistics
		r1 = getrank(cn);
		r2 = getrank(co);
		
		ch[cn].data[23]++; // Kill Counter
		
		if (ch[co].flags & (CF_PLAYER))
		{
			ch[cn].data[29]++;
		}
		else
		{
			if (ch[co].class && !killed_class(cn, ch[co].class))
			{
				// Tutorial 3
				if (ch[cn].data[76]<(1<<3))
				{
					chlog(cn, "SV_SHOWMOTD tutorial 3");
					buf[0] = SV_SHOWMOTD;
					*(unsigned char*)(buf + 1) = 103;
					xsend(ch[cn].player, buf, 2);
				}
				do_char_log(cn, 0, "You just killed your first %s. Good job.\n", get_class_name(ch[co].class));
				do_give_exp(cn, do_char_score(co) * 25, 0, -1, 0);
			}
			for (n = 1; n<10; n++) if (ch[cn].data[n]) grouped = 1;
			if (ch[co].class && grouped)
			{
				// <group rewards>
				for (n = 1; n<MAXCHARS; n++)
				{
					if (ch[n].used==USE_EMPTY) continue;
					if (!(ch[n].flags & (CF_PLAYER | CF_USURP))) continue;
					if (isgroup(n, cn) && isgroup(cn, n) && isnearby(cn, n))
					{
						if (ch[co].class && !killed_class(n, ch[co].class))
						{
							do_char_log(n, 0, "Your group helped you kill your first %s. Cool.\n", get_class_name(ch[co].class));
							do_give_exp(n, do_char_score(co) * 25, 0, -1, 0);
						}
					}
				}
				// </group rewards>
			}
		}
		
		// Tutorial 4
		if (ch[co].temp==150&&ch[cn].data[76]<(1<<4))
		{
			chlog(cn, "SV_SHOWMOTD tutorial 4");
			buf[0] = SV_SHOWMOTD;
			*(unsigned char*)(buf + 1) = 104;
			xsend(ch[cn].player, buf, 2);
		}
		// Tutorial 5
		if (ch[co].temp==153&&ch[cn].data[76]<(1<<5))
		{
			chlog(cn, "SV_SHOWMOTD tutorial 5");
			buf[0] = SV_SHOWMOTD;
			*(unsigned char*)(buf + 1) = 105;
			xsend(ch[cn].player, buf, 2);
		}
	}
	
	// a follower (garg, ghost comp or whatever) killed someone or something.
	if (cn && cn!=co && !(ch[cn].flags & (CF_PLAYER)) && (cc = ch[cn].data[CHD_MASTER])!=0 && (ch[cc].flags & (CF_PLAYER)))
	{
		// Contract kill progress
		if (CONT_NUM(cc)) 
		{
			os = 0;
			switch (CONT_SCEN(cc))
			{
				case  1: if (IS_CON_NME(co)) { add_map_progress(CONT_NUM(cc)); os++; } break;
				case  2: if (IS_CON_DIV(co)) { add_map_progress(CONT_NUM(cc)); os++; } break;
				case  3: if (IS_CON_CRU(co)) { add_map_progress(CONT_NUM(cc)); os++; } break;
				case  8: if (IS_CON_COW(co)) { add_map_progress(CONT_NUM(cc)); os++; } break;
				case  9: if (IS_CON_UNI(co)) { add_map_progress(CONT_NUM(cc)); os++; } break;
				default: break;
			}
			if (os && CONT_PROG(cc)>=CONT_GOAL(cc)) 
				do_char_log(cc, 2, "That's all of them! You're good to go!\n");
			else if (os)
				do_char_log(cc, 1, "%d down, %d to go.\n", CONT_PROG(cc), (CONT_GOAL(cc)-CONT_PROG(cc)));
		}
		
		do_item_kills(cc);
		
		if (!(ch[co].flags & (CF_PLAYER)) && ch[co].alignment==10000)
		{
			do_char_log(cc, 0, "A goddess is about to turn your follower into a frog, but notices that you are responsible. You feel her do something to you. Nothing good, that's for sure.\n");

			ch[cc].data[40]++;
			if (ch[cc].data[40]<50)
			{
				tmp = -ch[cc].data[40] * 100;
			}
			else
			{
				tmp = -5000;
			}
			ch[cc].luck += tmp;
			chlog(cc, "Reduced luck by %d to %d for killing %s (%d, t=%d)", tmp, ch[cn].luck, ch[co].name, co, ch[co].temp);
		}
		do_area_notify(cc, co, ch[cc].x, ch[cc].y, NT_SEEHIT, cc, co, 0, 0);
		
		// update statistics
		r1 = getrank(cc);
		r2 = getrank(co);
		
		ch[cc].data[23]++; // Kill Counter
		
		if (ch[co].flags & (CF_PLAYER))
		{
			ch[cc].data[29]++;
		}
		else
		{
			if (ch[co].class && !killed_class(cc, ch[co].class))
			{
				// Tutorial 3
				if (ch[cc].data[76]<(1<<3))
				{
					chlog(cc, "SV_SHOWMOTD tutorial 3");
					buf[0] = SV_SHOWMOTD;
					*(unsigned char*)(buf + 1) = 103;
					xsend(ch[cc].player, buf, 2);
				}
				do_char_log(cc, 0, "Your companion helped you kill your first %s. Good job.\n", get_class_name(ch[co].class));
				do_give_exp(cc, do_char_score(co) * 25, 0, -1, 0);
			}
			for (n = 1; n<10; n++) if (ch[cc].data[n]) grouped = 1;
			if (ch[co].class && grouped)
			{
				// <group rewards>
				for (n = 1; n<MAXCHARS; n++)
				{
					if (ch[n].used==USE_EMPTY) continue;
					if (!(ch[n].flags & (CF_PLAYER | CF_USURP))) continue;
					if (isgroup(n, cc) && isgroup(cc, n) && (isnearby(cc, n) || isnearby(cn, n)))
					{
						if (ch[co].class && !killed_class(n, ch[co].class))
						{
							do_char_log(n, 0, "Your group helped you kill your first %s. Cool.\n", get_class_name(ch[co].class));
							do_give_exp(n, do_char_score(co) * 25, 0, -1, 0);
						}
					}
				}
				// </group rewards>
			}
		}
		
		// Tutorial 4
		if (ch[co].temp==150&&ch[cc].data[76]<(1<<4))
		{
			chlog(cc, "SV_SHOWMOTD tutorial 4");
			buf[0] = SV_SHOWMOTD;
			*(unsigned char*)(buf + 1) = 104;
			xsend(ch[cc].player, buf, 2);
		}
		// Tutorial 5
		if (ch[co].temp==153&&ch[cc].data[76]<(1<<5))
		{
			chlog(cc, "SV_SHOWMOTD tutorial 5");
			buf[0] = SV_SHOWMOTD;
			*(unsigned char*)(buf + 1) = 105;
			xsend(ch[cc].player, buf, 2);
		}
	}

	if (ch[co].flags & (CF_PLAYER))
	{
		if (ch[co].luck<0)
		{
			ch[co].luck = min(0, ch[co].luck + 10);
		}

		// set killed by message (buggy!)
		ch[co].data[14]++;
		if (cn)
		{
			if (ch[cn].flags & (CF_PLAYER))
			{
				ch[co].data[15] = cn | 0x10000;
			}
			else
			{
				ch[co].data[15] = ch[cn].temp;
			}
		}
		else
		{
			ch[co].data[15] = 0;
		}
		ch[co].data[16] = globs->mdday + globs->mdyear * 300;
		ch[co].data[17] = ch[co].x + ch[co].y * MAPX;
	}
	else if (ch[co].data[0] && IS_SANEITEM(ch[co].data[0]) && spawner_driver(it[ch[co].data[0]].driver)>-1)
	{
		it[ch[co].data[0]].cost = globs->ticker + TICKS * 60 * 2;
	}

	remove_enemy(co);

	if (ch[co].flags & (CF_PLAYER))
	{
		globs->players_died++;
	}
	else
	{
		globs->npcs_died++;
	}

	// remember template if we're to respawn this char
	if (ch[co].flags & CF_RESPAWN)
	{
		temp = ch[co].temp;
	}

	// really kill co:
	x = ch[co].x;
	y = ch[co].y;
	
	wimp = 0;
	tmpg = 0;
	
	remove_all_debuffs(co);

	if ((mf1 & MF_ARENA) || is_atpandium(co) || is_incolosseum(co, 0))
	{	// Arena death : full save, keep everything
		wimp = 2;
	}
	else if (!IS_PURPLE(co))
	{	// Skua death : Skua save, drop items + 5% exp and 50% gold
		wimp = 1;
	}
	else
	{	// Purple death : Purple save, drop equipment + 5% exp and 100% gold
		wimp = 0;
	}
	
	//xlog(" -- WIMP = %d", wimp);

	// drop items and money in original place
	if (ch[co].flags & (CF_PLAYER))
	{
		// Newbie death : full save, keep everything
		if (getrank(co)<5)
		{
			wimp = 2;
			do_char_log(co, 0, "You would have dropped your items, but seeing you're still inexperienced the gods kindly returned them. Stay safe!\n");
		}
		
		// Dying in the darkwood
		if (IS_IN_DW(x, y))
		{
			x = 512;
			y = 526;
		}
		
		// Dying in a contract - set grave X and Y to in front of Osiris instead.
		if (CONT_NUM(co))
		{
			x = 748;
			y = 989;
		}
		
		// Dying in the Braver quest
		if (IS_IN_BRAV(x, y))
		{
			x = 957;
			y = 519;
		}
		
		// player death: clone char to resurrect him
		for (cc = 1; cc<MAXCHARS; cc++)
		{
			if (ch[cc].used==USE_EMPTY)
			{
				break;
			}
		}
		if (cc==MAXCHARS)
		{
			chlog(co, "could not be cloned, all char slots full!");
			return; // BAD kludge! But what can we do?
		}

		ch[cc] = ch[co]; // CC refers to the body, while CO refers to the presently dying character
		
		for (n = 0; n<MAXITEMS; n++)
		{
			if (!(in = ch[co].item[n]))
			{
				continue;
			}
			if (!do_maygive(cn, 0, in))
			{
				it[in].used = USE_EMPTY;
				ch[co].item[n] = 0;
				ch[cc].item[n] = 0;
				continue;
			}
			if (wimp==1)
			{
				ch[co].item[n] = 0;
				it[in].carried = cc;
				chlog(co, "Dropped %s (t=%d) in Grave", it[in].name, it[in].temp);
			}
			else
			{
				ch[cc].item[n] = 0;
			}
		}

		if ((in = ch[co].citem)!=0)
		{
			if (!do_maygive(cn, 0, in))
			{
				it[in].used  = USE_EMPTY;
				ch[co].citem = 0;
				ch[cc].citem = 0;
			}
			else if (wimp==1)
			{
				ch[co].citem = 0;
				it[in].carried = cc;
				chlog(co, "Dropped %s (t=%d) in Grave", it[in].name, it[in].temp);
			}
			else
			{
				ch[cc].citem = 0;
			}
		}

		for (n = 0; n<20; n++)
		{
			if (!(in = ch[co].worn[n]))
			{
				continue;
			}
			if (!do_maygive(cn, 0, in))
			{
				it[in].used = USE_EMPTY;
				ch[co].worn[n] = 0;
				ch[cc].worn[n] = 0;
				continue;
			}
			if (n == WN_CHARM || n == WN_CHARM2)	// Skip tarot card, player keeps that.
			{
				ch[cc].worn[n] = 0;
				continue;
			}
			if (wimp==0)
			{
				ch[co].worn[n] = 0;
				it[in].carried = cc;
				chlog(co, "Dropped %s (t=%d) in Grave", it[in].name, it[in].temp);
			}
			else
			{
				ch[cc].worn[n] = 0;
			}
		}
		
		for (n = 0; n<12; n++)
		{
			if (!(in = ch[co].alt_worn[n]))
			{
				continue;
			}
			if (!do_maygive(cn, 0, in))
			{
				it[in].used = USE_EMPTY;
				ch[co].alt_worn[n] = 0;
				ch[cc].alt_worn[n] = 0;
				continue;
			}
			// Hacky alt-worn drop method.
			if (wimp==0)
			{
				ch[co].alt_worn[n] = 0;
				ch[cc].item[n] = in;
				it[in].carried = cc;
				chlog(co, "Dropped %s (t=%d) in Grave", it[in].name, it[in].temp);
			}
			ch[cc].alt_worn[n] = 0;
		}

		for (n = 0; n<MAXBUFFS; n++)
		{
			if (!(in = ch[co].spell[n]))
			{
				continue;
			}
			ch[co].spell[n] = ch[cc].spell[n] = 0;
			bu[in].used = USE_EMPTY;  // destroy spells all the time
		}
		clear_map_buffs(co, 1);

		// move evidence (body) away
		if (ch[co].x==ch[co].temple_x && ch[co].y==ch[co].temple_y)
		{
			god_transfer_char(co, ch[co].temple_x + 4, ch[co].temple_y + 4);
		}
		else
		{
			god_transfer_char(co, ch[co].temple_x, ch[co].temple_y);
		}

		ch[co].a_hp = ch[co].hp[5] * 500;              // come alive!
		ch[co].status = 0;

		ch[co].attack_cn = 0;
		ch[co].skill_nr  = 0;
		ch[co].goto_x = 0;
		ch[co].use_nr = 0;
		ch[co].misc_action = 0;
		ch[co].stunned = 0;
		ch[co].taunted = 0;
		ch[co].retry = 0;
		ch[co].current_enemy = 0;
		for (m = 0; m<4; m++)
		{
			ch[co].enemy[m] = 0;
		}
		plr_reset_status(co);
		
		ch[cc].gold  = 0;

		if (!(ch[co].flags & CF_GOD) && wimp<2) // real death
		{
			// Changed to negative exp
			rank = getrank(co);
			tmpg = rank2points(rank) - rank2points(rank-1);
			tmp = ch[co].points_tot - rank2points(rank-1);
			
			if (rank < 9)			// Sergeant
				tmpg = tmpg / 40;
			else if (rank < 15)		// Officer
				tmpg = tmpg / 20;
			else if (rank < 20)		// General
				tmpg = tmpg*3/40;
			else if (rank < 24)		// Noble
				tmpg = tmpg / 10;
			else					// Warlord
				tmpg = tmp / 5;
			
			tmp = min(tmp, tmpg);
			
			if (ch[co].gold)
			{
				if (wimp==0) 	tmpg = ch[co].gold;
				else			tmpg = ch[co].gold/2;
				ch[co].gold -= tmpg;
				ch[cc].gold  = tmpg;
			}
			else
			{
				tmpg = ch[cc].gold = 0;
			}
			
			if (tmp>0)
			{
				if (tmpg>0)
				{
					do_char_log(co, 0, "You lost %d exp and dropped %dG %dS.\n", tmp, tmpg/100, tmpg%100);
					chlog(co, "Lost %d exp and %dG %dS from death.", tmp, tmpg/100, tmpg%100);
				}
				else
				{
					do_char_log(co, 0, "You lost %d experience points.\n", tmp);
					chlog(co, "Lost %d exp from death.", tmp);
				}
				ch[co].points_tot -= tmp;
				ch[co].points -= tmp;
				ch[co].data[11] = 0;
			}
			else
			{
				if (tmpg>0)
					do_char_log(co, 0, 
					"You dropped %dG %dS. You would have lost experience points, but you're already at the minimum.\n", 
						tmpg/100, tmpg%100);
				else
					do_char_log(co, 0, 
					"You would have lost experience points, but you're already at the minimum.\n");
			}
		}

		do_update_char(co);

		plr_reset_status(cc);
		chlog(cc, "new player body");
		ch[cc].player = 0;
		ch[cc].flags  = CF_BODY;
		ch[cc].a_hp = 0;
		ch[cc].data[CHD_CORPSEOWNER] = co;
		ch[cc].data[99] = 1;
		ch[cc].data[98] = 0;

		ch[cc].attack_cn = 0;
		ch[cc].skill_nr  = 0;
		ch[cc].goto_x = 0;
		ch[cc].use_nr = 0;
		ch[cc].misc_action = 0;
		ch[cc].stunned = 0;
		ch[cc].taunted = 0;
		ch[cc].retry = 0;
		ch[cc].current_enemy = 0;
		for (m = 0; m<4; m++)
		{
			ch[cc].enemy[m] = 0;
		}
		do_update_char(cc);
		co = cc;
		plr_map_set(co);
	}
	else if (!(ch[co].flags & CF_LABKEEPER))// && !pentsolve)
	{
		// NPC death
		plr_reset_status(co);
		if (ch[co].flags & CF_USURP)
		{
			int nr, c2;

			c2 = ch[co].data[97];

			if (IS_SANECHAR(c2))
			{
				nr = ch[co].player;

				ch[c2].player = nr;
				player[nr].usnr = c2;
				ch[c2].flags &= ~(CF_CCP);
			}
			else
			{
				player_exit(ch[co].player);
			}
		}
		chlog(co, "new npc body");
		//
		for (n = 0; n<13; n++)
		{
			if (!(in = ch[co].worn[n]))
			{
				continue;
			}
			if (n == WN_CHARM || n == WN_CHARM2)	// Skip tarot card
			{
				ch[co].worn[n] = 0;
			}
		}
		//
		if (ch[co].flags & CF_RESPAWN)
		{
			ch[co].flags = CF_BODY | CF_RESPAWN;
		}
		else
		{
			ch[co].flags = CF_BODY;
		}
		if (ch[co].flags & CF_NOSLEEP)
		{
			ch[co].flags &= ~CF_NOSLEEP;
		}
		ch[co].a_hp = 0;
#ifdef KILLERONLY
		if (IS_SANECHAR(cc = ch[cn].data[CHD_MASTER])!=0 && (ch[cc].flags & (CF_PLAYER)))
		{
			ch[co].data[CHD_CORPSEOWNER] = cc;
		}
		else if (ch[cn].flags & (CF_PLAYER))
		{
			ch[co].data[CHD_CORPSEOWNER] = cn;
		}
		else
		{
			ch[co].data[CHD_CORPSEOWNER] = 0;
		}
#else
		ch[co].data[CHD_CORPSEOWNER] = 0;
#endif
		ch[co].data[99] = 0;
		ch[co].data[98] = 0;

		ch[co].attack_cn = 0;
		ch[co].skill_nr  = 0;
		ch[co].goto_x = 0;
		ch[co].use_nr = 0;
		ch[co].misc_action = 0;
		ch[co].stunned = 0;
		ch[co].taunted = 0;
		ch[co].retry = 0;
		ch[co].current_enemy = 0;
		if (pentsolve)
		{
			ch[co].gold = 0;
		}
		if (cn && IS_GLOB_MAYHEM)
		{
			ch[co].gold += ch[co].gold/5;
		}
		if (cn && (in = get_gear(cn, IT_MISERRING)) && it[in].active) // 50% more gold with Miser Ring
		{
			ch[co].gold = ch[co].gold*3/2;
		}
		for (m = 0; m<4; m++)
		{
			ch[co].enemy[m] = 0;
		}

		for (n = 0; n<MAXBUFFS; n++)
		{
			if (!(in = ch[co].spell[n]))
			{
				continue;
			}
			ch[co].spell[n] = 0;
			bu[in].used = USE_EMPTY;  // destroy spells all the time
		}
		// if killer is a player, check for special items in grave
		if (IS_SANEPLAYER(cn) || IS_PLAYER_COMP(cn))
		{
			if (IS_PLAYER_COMP(cn)) do_ransack_corpse(CN_OWNER(cn), co, " * Dropped a%s.\n");
			else                    do_ransack_corpse(cn, co, " * Dropped a%s.\n");
		}
		for (z = 40; z<MAXITEMS; z++)
		{
			if (ch[co].item[z]) break;
		}
		if (z!=MAXITEMS) qsort(ch[co].item, MAXITEMS, sizeof(int), qsort_proc);
		do_update_char(co);
	}
	else            // CF_LABKEEPER, or pent mob auto-poofed
	{
		if (IS_SANECHAR(cc = ch[co].data[PCD_COMPANION]) && CN_OWNER(cc) == co)
		{
			plr_map_remove(cc);
			god_destroy_items(cc);
			ch[cc].used = USE_EMPTY;
		}
		if (IS_SANECHAR(cc = ch[co].data[PCD_SHADOWCOPY]) && CN_OWNER(cc) == co)
		{
			plr_map_remove(cc);
			god_destroy_items(cc);
			ch[cc].used = USE_EMPTY;
		}
		plr_map_remove(co);
		god_destroy_items(co);
		ch[co].citem = 0;
		ch[co].gold  = 0;
		for (z = 0; z<MAXITEMS; z++)
		{
			ch[co].item[z] = 0;
		}
		for (z = 0; z<20; z++)
		{
			ch[co].worn[z] = 0;
		}
		for (z = 0; z<12; z++)
		{
			ch[co].alt_worn[z] = 0;
		}
		ch[co].used = USE_EMPTY;
		use_labtransfer2(cn, co);
		return;
	}
	
	// show death and tomb animations and schedule respawn
	fn = fx_add_effect(3, 0, x, y, co);
	fx[fn].data[3] = cn;
}

int do_char_can_flee(int cn)
{
	int per = 0, co, ste, m, chance;
	int n;

	for (m = 0; m<4; m++)
	{
		if ((co = ch[cn].enemy[m])!=0 && ch[co].current_enemy!=cn)
		{
			ch[cn].enemy[m] = 0;
		}
	}

	for (m = 0; m<4; m++)
	{
		if ((co = ch[cn].enemy[m])!=0 && ch[co].attack_cn!=cn)
		{
			ch[cn].enemy[m] = 0;
		}
	}

	if (!ch[cn].enemy[0] && !ch[cn].enemy[1] &&
	    !ch[cn].enemy[2] && !ch[cn].enemy[3])
	{
		return 1;
	}

	// You already succeeded in escaping recently, so give it a bit
	if (ch[cn].escape_timer > TICKS*2 || get_enchantment(cn, 46) || IS_COMP_TEMP(cn)) 
	{ 
		for (m = 0; m<4; m++)
		{
			ch[cn].enemy[m] = 0;
		}
		remove_enemy(cn);
		return 1; 
	}
	chance = 0;
	for (m = 0; m<4; m++)
	{
		if ((co = ch[cn].enemy[m])!=0 && (ch[co].temp == CT_PANDIUM || ch[co].temp == CT_SHADOW || ch[co].temp == CT_LAB20_KEEP))
		{
			ch[cn].enemy[m] = 0;
			chance = 1;
		}
	}
	if (chance) 
	{ 
		remove_enemy(cn);
		return 1; 
	}
	if (ch[cn].escape_timer || get_enchantment(cn, 46) || IS_COMP_TEMP(cn)) return 0;

	per  = 0;

	for (m = 0; m<4; m++)
	{
		if ((co = ch[cn].enemy[m])!=0)
		{
			per += M_SK(co, SK_PERCEPT);
		}
	}
	ste   = M_SK(cn, SK_STEALTH);

	//chance = 9 + (per - ste);
	chance=ste*15/per;
	
	if (chance< 0) chance =   0; 
	if (chance>18) chance =  18; 
	
	if ((RANDOM(20)<=chance && !ch[cn].taunted))
	{
		ch[cn].escape_timer = TICKS*3;
		if (!(ch[cn].flags & CF_SYS_OFF))
			do_char_log(cn, 1, "You manage to escape!\n");
		for (m = 0; m<4; m++)
		{
			ch[cn].enemy[m] = 0;
		}
		remove_enemy(cn);
		return 1;
	}

	ch[cn].escape_timer = TICKS;
	if (!(ch[cn].flags & CF_SYS_OFF))
		do_char_log(cn, 0, "You cannot escape!\n");

	return 0;
}

static void add_enemy(int cn, int co)
{
	if (ch[cn].enemy[0]!=co &&
	    ch[cn].enemy[1]!=co &&
	    ch[cn].enemy[2]!=co &&
	    ch[cn].enemy[3]!=co)
	{
		if (!ch[cn].enemy[0])
		{
			ch[cn].enemy[0] = co;
		}
		else if (!ch[cn].enemy[1])
		{
			ch[cn].enemy[1] = co;
		}
		else if (!ch[cn].enemy[2])
		{
			ch[cn].enemy[2] = co;
		}
		else if (!ch[cn].enemy[3])
		{
			ch[cn].enemy[3] = co;
		}
	}
}

void do_give_exp(int cn, int p, int gflag, int rank, int money)
{
	int n, c, co, s, v, master;

	if (p<0)
	{
		xlog("PANIC: do_give_exp got negative amount");
		return;
	}

	if (gflag)
	{
		if (ch[cn].flags & (CF_PLAYER))
		{
			for (n = 1, c = 1; n<10; n++)
			{
				if ((co = ch[cn].data[n])!=0 && isgroup(co, cn) && do_char_can_see(cn, co, 0))
				{
					c++;
				}
			}

			for (n = 1, s = 0, v = 0; n<10; n++)
			{
				if ((co = ch[cn].data[n])!=0 && isgroup(co, cn) && do_char_can_see(cn, co, 0))
				{
					do_give_exp(co, p / c, 0, rank, money / c);
					s += p / c;
					v += money / c;
				}
			}
			do_give_exp(cn, p - s, 0, rank, money - v);
		}
		else     // we're an NPC
		{
			if ((co = ch[cn].data[CHD_MASTER])!=0) // we are the follower of someone
			{
				do_give_exp(cn, p, 0, rank, 0);
				if (money) do_give_exp(co, 0, 0, -1, money);
				if ((master = ch[cn].data[CHD_MASTER])>0 && master<MAXCHARS && ch[master].points_tot>ch[cn].points_tot)
				{
					ch[cn].data[28] += scale_exps2(master, rank, p);
				}
				else
				{
					ch[cn].data[28] += scale_exps2(cn, rank, p);
				}
			}
		}
	}
	else
	{
		if (rank>=0 && rank<=24)
		{
			if ((master = ch[cn].data[CHD_MASTER])>0 && master<MAXCHARS && ch[master].points_tot>ch[cn].points_tot)
			{
				p = scale_exps2(master, rank, p);
			}
			else
			{
				p = scale_exps2(cn, rank, p);
			}
		}
		if (p && money>0)
		{
			chlog(cn, "Gets %d EXP and %d Money", p, money);
			ch[cn].points += p;
			ch[cn].points_tot += p;
			ch[cn].gold += money;
			
			if (!(ch[cn].flags & CF_SYS_OFF))
				do_char_log(cn, 2, "You get %d exp and %dG %dS.\n", p, money / 100, money % 100);
			do_notify_char(cn, NT_GOTEXP, p, 0, 0, 0);
			do_update_char(cn);
			do_check_new_level(cn, 1);
		}
		else if (p)
		{
			chlog(cn, "Gets %d EXP", p);
			ch[cn].points += p;
			ch[cn].points_tot += p;
			
			if (!(ch[cn].flags & CF_SYS_OFF))
				do_char_log(cn, 2, "You get %d experience points.\n", p);
			do_notify_char(cn, NT_GOTEXP, p, 0, 0, 0);
			do_update_char(cn);
			do_check_new_level(cn, 1);
		}
		else if (money>0)
		{
			chlog(cn, "Gets %d G", money);
			ch[cn].gold += money;
			if (!(ch[cn].flags & CF_SYS_OFF) && rank>=0)
				do_char_log(cn, 2, "You received %dG %dS.\n", money / 100, money % 100);
		}
	}
}

void do_give_bspoints(int cn, int p, int gflag)
{
	int n, c, co, s;

	if (p<1)
	{
		p=1;
	}
	
	// group distribution
	if (gflag)
	{
		if (ch[cn].flags & (CF_PLAYER))
		{
			for (n = 1, c = 0; n<10; n++)
			{
				if ((co = ch[cn].data[n])!=0 && isgroup(co, cn) && do_char_can_see(cn, co, 0))
					c+=75;
			}
			for (n = 1; n<10; n++)
			{
				if ((co = ch[cn].data[n])!=0 && isgroup(co, cn) && do_char_can_see(cn, co, 0))
					do_give_bspoints(co, p / max(1,c/100), 0);
			}
			do_give_bspoints(cn, p / max(1,c/100), 0);
		}
		else
		{
			// Give GC owner's group the points
			if ((co = ch[cn].data[CHD_MASTER])!=0 && IS_SANEPLAYER(co))
				do_give_bspoints(co, p, 1);
		}
	}
	else
	{
		// single distribution
		if (p)
		{
			chlog(cn, "Gets %d BSP", p);
			ch[cn].bs_points += p;
			//do_char_log(cn, 2, "You get %d stronghold points.\n", p);
		}
	}
}

void do_give_ospoints(int cn, int p, int gflag)
{
	int n, c, co, s;

	if (p<1)
	{
		p=1;
	}
	
	// group distribution
	if (gflag)
	{
		if (ch[cn].flags & (CF_PLAYER))
		{
			for (n = 1, c = 0; n<10; n++)
			{
				if ((co = ch[cn].data[n])!=0 && isgroup(co, cn) && do_char_can_see(cn, co, 0))
					c+=75;
			}
			for (n = 1; n<10; n++)
			{
				if ((co = ch[cn].data[n])!=0 && isgroup(co, cn) && do_char_can_see(cn, co, 0))
					do_give_ospoints(co, p / max(1,c/100), 0);
			}
			do_give_ospoints(cn, p / max(1,c/100), 0);
		}
		else
		{
			// Give GC owner's group the points
			if ((co = ch[cn].data[CHD_MASTER])!=0 && IS_SANEPLAYER(co))
				do_give_ospoints(co, p, 1);
		}
	}
	else
	{
		// single distribution
		if (p)
		{
			chlog(cn, "Gets %d OSP", p);
			ch[cn].os_points += p;
			//do_char_log(cn, 2, "You get %d stronghold points.\n", p);
		}
	}
}

int try_lucksave(int cn)
{
	if ((ch[cn].luck>=100 && RANDOM(10000)<5000 + ch[cn].luck) && !is_atpandium(cn) /* && IS_PURPLE(cn) */ )
		return 1;
	
	return 0;
}

void do_lucksave(int cn, char *deathtype)
{
	int in, n;
	
	ch[cn].a_hp  = ch[cn].hp[5] * 500;
	ch[cn].luck /= 2;
	do_char_log(cn, 0, "A god reached down and saved you from the %s. You must have done the gods a favor sometime in the past!\n", deathtype);
		do_area_log(cn, 0, ch[cn].x, ch[cn].y, 0, "A god reached down and saved %s from the %s.\n", ch[cn].reference, deathtype);
	
	// Removed spells upon save to prevent nasty scenarios like lingering poisons
	for (n = 0; n<MAXBUFFS; n++)
	{
		if ((in = ch[cn].spell[n])==0) continue;
		bu[in].used = USE_EMPTY;
		ch[cn].spell[n] = 0;
	}
	clear_map_buffs(cn, 1);
	ch[cn].data[11] = 0;
	
	fx_add_effect(6, 0, ch[cn].x, ch[cn].y, 0);
	god_transfer_char(cn, ch[cn].temple_x, ch[cn].temple_y);
	fx_add_effect(6, 0, ch[cn].x, ch[cn].y, 0);
	
	for (n = 0; n<4; n++) ch[cn].enemy[n] = 0;
	remove_enemy(cn);
	
	ch[cn].escape_timer = TICKS*5;
	
	chlog(cn, "Saved by the Gods (new luck=%d)", ch[cn].luck);
	ch[cn].data[44]++;
}

// dmg types: 0=normal 1=blast 2=hw/soku 3=gethit 4=surround 5=cleave 6=pulse 7=zephyr 8=leap 9=crit 13=gethit/10
// returns actual damage done
int do_hurt(int cn, int co, int dam, int type)
{
	int tmp = 0, cc, n, m, in, rank = 0, noexp = 0, halfexp = 0, kill_bsp = 0, kill_osp = 0, kill_bos = 0, money = 0;
	unsigned long long mf1, mf2;
	int hp_dam = 0, end_dam = 0, mana_dam = 0;
	int scorched = 0, guarded = 0, devRn = 0, devRo = 0, phalanx = 0, aggravate = 0;
	int offpot = 0, defpot = 0;
	int thorns = 0, crit_dam = 0, damtype = 250;
	int extradam = 0, priestess = 1, cullval=500;
	
	mf1 = mf2 = map[XY2M(ch[co].x, ch[co].y)].flags;
	if (cn)
	{
		mf1 |= map[XY2M(ch[cn].x, ch[cn].y)].flags;
		mf2 &= map[XY2M(ch[cn].x, ch[cn].y)].flags;
	}

	if (ch[co].flags & CF_BODY)
	{
		return 0;
	}
	
	// God Enchant :: Sacrifice 10% of current endurance on hit to deal that much additional damage
	if (get_enchantment(cn, 70) && (ch[cn].a_end-500)>0)
	{
		extradam = (ch[cn].a_end-500)/10000;
		dam += extradam;
		ch[cn].a_end -= extradam;
	}

	if ((ch[co].flags & (CF_PLAYER)) && type!=3 && type!=13 && type!=90 && type!=96)
	{
		item_damage_armor(co, dam);
	}

	if (!(ch[cn].flags & CF_PLAYER) && ch[cn].data[CHD_MASTER]==co)
	{
		noexp = 1;
	}

	// no exp for killing players
	if (ch[co].flags & CF_PLAYER)
	{
		noexp = 1;
	}

	// half exp for killing ghosts
	if (IS_COMP_TEMP(co) && !IS_THRALL(co))
	{
		halfexp = 1;
	}
	
	if (type==3 && (ch[co].temp == CT_PANDIUM || ch[co].temp == CT_SHADOW || (IS_PLAYER(cn) && IS_PLAYER(co))))
	{
		type = 13;
	}
	
	// Invidia
	if (get_gear(co, IT_TW_INVIDIA) && IS_SANECHAR(ch[co].data[PCD_COMPANION]) 
		&& ch[ch[co].data[PCD_COMPANION]].data[CHD_MASTER]==co && !(ch[ch[co].data[PCD_COMPANION]].flags & CF_BODY))
	{
		co = ch[co].data[PCD_COMPANION];
	}
	
	if (get_tarot(co, IT_CH_PREIST_R)) priestess = 2;
	
	// Loop to look for Magic Shield so we can damage it
	for (n = 0; n<MAXBUFFS; n++)
	{
		if ((in = ch[co].spell[n])!=0)
		{
			if (bu[in].temp==SK_MSHIELD)
			{
				if (IS_SEYA_OR_BRAV(co))
					tmp = (bu[in].active / 1536 + 1) * priestess;
				else
					tmp = (bu[in].active / 1024 + 1) * priestess;
				tmp = (dam + tmp - ch[co].armor) * 5;
				
				// Book - Great Divide :: half duration damage dealt to shield/shell
				if (get_book(co, IT_BOOK_GREA) || get_book(co, IT_IMBK_GREA)) tmp /= 2;
				if (m=st_skillcount(co, 72)) tmp = min(tmp, max(0, tmp*(100-m*10)/100));
				
				if (tmp>0)
				{
					if (priestess==1 && tmp>=bu[in].active)
					{
						ch[co].spell[n] = 0;
						bu[in].used = 0;
						do_update_char(co);
					}
					else
					{
						bu[in].active -= tmp;
						if (priestess==2 && bu[in].active < 1) bu[in].active = 1;
						if (IS_SEYA_OR_BRAV(co))
							bu[in].armor[1] = min(127, bu[in].active / 1536 + 1);
						else
							bu[in].armor[1] = min(127, bu[in].active / 1024 + 1);
						if (get_enchantment(co, 57)) bu[in].weapon[1] = bu[in].armor[1]/4;
						bu[in].power = bu[in].active / 256;
						do_update_char(co);
					}
				}
			}
		}
	}
	
	// Easy new method!
	if (cn) dam = dam * ch[cn].dmg_bonus / 10000;
	
	// Tarot - Strength - 20% more damage dealt
	if (get_tarot(cn, IT_CH_STRENGTH))
	{
		dam = dam*6/5;
	}
	
	if (type==3 || type==13 || type==90 || type==96)
	{
		dam = dam * ch[co].dmg_reduction / 10000;
		if (type!=23 && T_BRAV_SK(co, 6)) dam /= 2;
		dam *= DAM_MULT_THORNS; 						// Thorns
		if (type==13) dam = dam/10;
		if (type==90)
		{
			if (n=st_skillcount(cn, 90)) dam = dam*n*5/100;
			else dam = 0;
		}
		if (type==96)
		{
			if (n=st_skillcount(cn, 96)) dam = dam*n*5/100;
			else dam = 0;
		}
		
		if ((mf1 & MF_NOFIGHT) && !(IS_IN_BOJ(ch[cn].x, ch[cn].y) && get_gear(cn, IT_XIXDARKSUN)))
			dam = 0;
	}
	else
	{
		dam -= ch[co].armor;
		
		// Easy new method!
		dam = dam * ch[co].dmg_reduction / 10000;
		
		if (dam<0) dam = 0;
		else
		{
			switch (type)
			{
				case  1: damtype = DAM_MULT_BLAST;  break; // Blast
				case  2: damtype = DAM_MULT_HOLYW;  break; // Holy Water / Staff of Kill Undead
				case  5: damtype = DAM_MULT_CLEAVE; break; // Cleave
				case  6: damtype = DAM_MULT_PULSE;  break; // Pulse
				case  7: damtype = DAM_MULT_ZEPHYR; break; // Zephyr
				case  8: damtype = DAM_MULT_LEAP;   break; // Leap
				case 18: damtype = DAM_MULT_RLEAP;  break; // Random Leaps
				default: damtype = DAM_MULT_HIT;    break; // Hit / Surround Hit / Crit
			}
			if ((type==5 || type == 8 || type == 18) && !IS_PLAYER(cn) && IS_PLAYER(co)) damtype = damtype*4/5;
			dam *= damtype;
		}
	}
	
	if ((n = get_enchantment(co, 35)) && RANDOM(100)<n*8)
		dam /= 2;
	
	if (ch[co].flags & CF_IMMORTAL)
		dam = 0;
	
	if (type!=3 && type!=13)
	{
		do_area_notify(cn, co, ch[cn].x, ch[cn].y, NT_SEEHIT, cn, co, 0, 0);
		if ((type!=1 && type!=6 && type!=7) || !IS_IGNORING_SPELLS(co))
			do_notify_char(co, NT_GOTHIT, cn, dam / 1000, 0, 0);
		do_notify_char(cn, NT_DIDHIT, co, dam / 1000, 0, 0);
	}

	if (dam<1)
	{
		if ((type==0 || type==4 || type==5 || type==8 || type==18 || type==9) && ch[co].gethit_dam>0)
		{
			thorns = RANDOM(ch[co].gethit_dam)+1;
			if (ch[cn].to_parry > ch[co].to_hit) 
			{
				thorns = max(0, thorns - (ch[cn].to_parry - ch[co].to_hit));
			}
			if (thorns>0) do_hurt(co, cn, thorns, 3);
		}
		
		if (type!=2 && type!=3 && type!=13)
		{
			// Enchantments
			if (n = get_enchantment(cn, 49)) ch[cn].a_hp   += n*1000*((IS_USETWOHAND(cn))?2:1);
			if (n = get_enchantment(co, 73)) ch[co].a_hp   += n*2000;
			if (n = get_enchantment(cn, 42)) ch[cn].a_end  += n*1000*((IS_USETWOHAND(cn))?2:1);
			if (n = get_enchantment(co, 43)) ch[co].a_end  += n*2000;
			if (n = get_enchantment(cn, 38)) ch[cn].a_mana += n*1000*((IS_USETWOHAND(cn))?2:1);
			if (n = get_enchantment(co, 39)) ch[co].a_mana += n*2000;
		}
		
		// force to sane values
		if (ch[cn].a_hp>ch[cn].hp[5] * 1000)     ch[cn].a_hp   = ch[cn].hp[5]   * 1000;
		if (ch[cn].a_end>ch[cn].end[5] * 1000)   ch[cn].a_end  = ch[cn].end[5]  * 1000;
		if (ch[co].a_end>ch[co].end[5] * 1000)   ch[co].a_end  = ch[co].end[5]  * 1000;
		if (ch[cn].a_mana>ch[cn].mana[5] * 1000) ch[cn].a_mana = ch[cn].mana[5] * 1000;
		if (ch[co].a_mana>ch[co].mana[5] * 1000) ch[co].a_mana = ch[co].mana[5] * 1000;
		
		return 0;
	}

	// give some EXPs to the attacker for a successful blow:
	if (type!=2 && type!=3 && type!=13 && !noexp)
	{
		tmp = dam;
		if (ch[co].a_hp-500 < tmp) tmp = ch[co].a_hp-500;
		tmp /= 4000;
		
		if (ch[co].flags & CF_EXTRAEXP)  tmp = tmp * 2;
		if (ch[co].flags & CF_EXTRACRIT) tmp = tmp * 3/2;
		
		if (tmp>0 && cn)
		{
			tmp = scale_exps(cn, co, tmp);
			if (halfexp) tmp /= 4;
			if (tmp>0)
			{
				ch[cn].points += tmp;
				ch[cn].points_tot += tmp;
				do_check_new_level(cn, 1);
			}
		}
	}

	if (type!=1 && type!=6 && type!=7)
	{
		if (type==9) 
		{
			map[ch[co].x + ch[co].y * MAPX].flags |= MF_GFX_CRIT;
		}
		if (dam<10000)
		{
			map[ch[co].x + ch[co].y * MAPX].flags |= MF_GFX_INJURED;
			fx_add_effect(FX_INJURED, 8, ch[co].x, ch[co].y, 0);
		}
		else if (dam<30000)
		{
			map[ch[co].x + ch[co].y * MAPX].flags |= MF_GFX_INJURED | MF_GFX_INJURED1;
			fx_add_effect(FX_INJURED, 8, ch[co].x, ch[co].y, 0);
		}
		else if (dam<50000)
		{
			map[ch[co].x + ch[co].y * MAPX].flags |= MF_GFX_INJURED | MF_GFX_INJURED2;
			fx_add_effect(FX_INJURED, 8, ch[co].x, ch[co].y, 0);
		}
		else
		{
			map[ch[co].x + ch[co].y * MAPX].flags |= MF_GFX_INJURED | MF_GFX_INJURED1 | MF_GFX_INJURED2;
			fx_add_effect(FX_INJURED, 8, ch[co].x, ch[co].y, 0);
		}
	}
	
	if (type!=2 && type!=3 && type!=13)
	{
		// Gula - 20% damage dealt healed as hp
		if (get_gear(cn, IT_TW_GULA))
		{
			ch[cn].a_hp += dam/5;
		}
		
		// Lycan Tree - 8% damage dealt healed as hp/end/mana
		if (T_LYCA_SK(cn, 9))
		{
			ch[cn].a_hp   += dam*2/25;
			ch[cn].a_end  += dam*2/25;
			ch[cn].a_mana += dam*2/25;
		}
		
		// God Enchant :: 4% of damage dealt healed as hp
		if (get_enchantment(cn, 69))
		{
			ch[cn].a_hp   += dam/25;
		}
		
		if (n=st_skillcount(cn,105))
		{
			switch (RANDOM(3))
			{
				case  0: ch[cn].a_hp   += dam*n/50; break;
				case  1: ch[cn].a_end  += dam*n/50; break;
				default: ch[cn].a_mana += dam*n/50; break;
			}
		}
		if (n=st_skillcount(cn, 70))
		{
			if (IS_SANECHAR(cc = ch[cn].data[PCD_COMPANION]) && IS_PLAYER_GC(cc))
				ch[cc].a_hp   += dam*n/50;
			if (IS_SANECHAR(cc = ch[cn].data[PCD_SHADOWCOPY]) && IS_PLAYER_SC(cc))
				ch[cc].a_hp   += dam*n/50;
		}
	}
	
	hp_dam = dam;
	
	if (type == 5 && (get_gear(cn, IT_WP_BRONCHIT)) 
		           || get_gear(cn, IT_WB_BRONCHIT))	mana_dam += dam*20/100; // Weapon - Bronchitis :: 20% cleave damage also dealt to mana
	if (get_enchantment(cn, 65))					mana_dam += dam*20/100; // Gorn Enchantment :: 20% of damage dealt is also dealt to enemy mana.
	if (get_tarot(co, IT_CH_PREIST))				mana_dam += dam*20/100; // Damage taken dealt to mana instead ( 20+20+20 )
	if (get_enchantment(co, 23))					mana_dam += dam*20/100;
	if (T_ARHR_SK(co, 12))							mana_dam += dam*20/100;
	
	if (ch[co].a_mana - mana_dam<0) 	mana_dam = ch[co].a_mana;
	
	hp_dam -= mana_dam;	dam = hp_dam;
	
	if (get_gear(co, IT_TW_CLOAK))		end_dam += dam*10/100; // Damage taken dealt to endurance instead ( 10+20+20+50 )
	if (get_enchantment(co, 16))		end_dam += dam*20/100;
	if (T_SKAL_SK(co, 12))				end_dam += dam*20/100;
	if (get_tarot(co, IT_CH_WORLD_R))	end_dam += dam*50/100;
	
	if (ch[co].a_end - end_dam<0)		end_dam = ch[co].a_end;
		
	hp_dam -= end_dam;
	
	if (get_gear(co, IT_BONEARMOR))
	{
		ch[co].data[11] += hp_dam*30/100;
		hp_dam -= hp_dam*30/100;
	}
	
	if (hp_dam < 0) hp_dam = 0;
	
	// Culling strike!
	if ((type==5 && (n=st_skillcount(cn, 10))) || (type==1 && (n=st_skillcount(cn, 76))) || (type==9 && (n=st_skillcount(cn, 102))))
	{
		cullval = 500 + min(ch[co].hp[5]*200, max(0, ch[co].hp[5]*1000 - ch[co].hp[5]*1000*(100-n*2)/100));
	}
	
	if (ch[co].a_hp - hp_dam<cullval && !(mf2 & MF_ARENA) && try_lucksave(co))
	{
		do_lucksave(co, "killing blow");

		do_notify_char(cn, NT_DIDKILL, co, 0, 0, 0);
		do_area_notify(cn, co, ch[cn].x, ch[cn].y, NT_SEEKILL, cn, co, 0, 0);
	}
	else
	{
		ch[co].a_hp -= hp_dam;
		ch[co].a_end -= end_dam;
		ch[co].a_mana -= mana_dam;
	}
	
	if (type!=2 && type!=3 && type!=13)
	{
		// Enchantments
		if (n = get_enchantment(cn, 49)) ch[cn].a_hp   += n*1000*((IS_USETWOHAND(cn))?2:1);
		if (n = get_enchantment(co, 73)) ch[co].a_hp   += n*2000;
		if (n = get_enchantment(cn, 42)) ch[cn].a_end  += n*1000*((IS_USETWOHAND(cn))?2:1);
		if (n = get_enchantment(co, 43)) ch[co].a_end  += n*2000;
		if (n = get_enchantment(cn, 38)) ch[cn].a_mana += n*1000*((IS_USETWOHAND(cn))?2:1);
		if (n = get_enchantment(co, 39)) ch[co].a_mana += n*2000;
	}
	
	// force to sane values
	if (ch[cn].a_hp>ch[cn].hp[5] * 1000)     ch[cn].a_hp   = ch[cn].hp[5]   * 1000;
	if (ch[cn].a_end>ch[cn].end[5] * 1000)   ch[cn].a_end  = ch[cn].end[5]  * 1000;
	if (ch[co].a_end>ch[co].end[5] * 1000)   ch[co].a_end  = ch[co].end[5]  * 1000;
	if (ch[cn].a_mana>ch[cn].mana[5] * 1000) ch[cn].a_mana = ch[cn].mana[5] * 1000;
	if (ch[co].a_mana>ch[co].mana[5] * 1000) ch[co].a_mana = ch[co].mana[5] * 1000;

	if (ch[co].a_hp<12500 && ch[co].a_hp>=500 && getrank(co)<5)
	{
		do_char_log(co, 0, "You're almost dead... Use a potion, quickly!\n");
	}

	if (ch[co].a_hp<cullval)
	{
		char buf[50];
		strcpy(buf, ch[co].reference); buf[0] = toupper(buf[0]);
		do_area_log(cn, co, ch[cn].x, ch[cn].y, 0, "%s is dead!\n", buf);
		if (!(ch[cn].flags & CF_SYS_OFF))
			do_char_log(cn, 0, "You killed %s.\n", ch[co].reference);
		if (ch[cn].flags & CF_INVISIBLE)
		{
			do_char_log(co, 0, "Oh dear, that blow was fatal. Somebody killed you...\n");
		}
		else
		{
			do_char_log(co, 0, "Oh dear, that blow was fatal. %s killed you...\n", ch[cn].name);
		}
		do_notify_char(cn, NT_DIDKILL, co, 0, 0, 0);
		do_area_notify(cn, co, ch[cn].x, ch[cn].y, NT_SEEKILL, cn, co, 0, 0);
		chlog(cn, "Killed %s", ch[co].name);
		
		if (type!=2 && cn && !(mf2 & MF_ARENA) && !noexp)
		{
			if (ch[co].temp>=42 && ch[co].temp<=70) 
			{
				if (IS_SANEPLAYER(ch[cn].data[CHD_MASTER]))
					ch[ch[cn].data[CHD_MASTER]].data[77]++;
				else
					ch[cn].data[77]++;
			}
			tmp  = do_char_score(co);
			rank = getrank(co);

			for (n = 0; n<MAXBUFFS; n++) if ((in = ch[co].spell[n]))
			{
				if (!B_SK(co, SK_MEDIT) && (bu[in].temp==SK_PROTECT || bu[in].temp==SK_ENHANCE || bu[in].temp==SK_BLESS || bu[in].temp==SK_HASTE))
					tmp += tmp / 5;
				if (bu[in].temp==105) // map exp bonus
					tmp += tmp*bu[in].power*RATE_P_PLXP/100;
				if (bu[in].temp==106) // map bonus bsp
					kill_bsp = bu[in].power;
				if (bu[in].temp==107) // map bonus osp
					kill_osp = bu[in].power;
				if (bu[in].temp==108) // tree bonus osp
					kill_bos = bu[in].power;
			}
			if (IS_GLOB_MAYHEM) tmp += tmp / 5;
			if (ch[co].flags & CF_EXTRAEXP)  tmp = tmp * 2;
			if (ch[co].flags & CF_EXTRACRIT) tmp = tmp * 3/2;
		}
		
		if (type!=2 && cn && cn!=co && !(mf2 & MF_ARENA) && !noexp)
		{
			if ((in = get_gear(cn, IT_FORTERING)) && it[in].active) // 25% more exp with Forte Ring
			{
				tmp = tmp*4/3;
			}
			if ((IS_PLAYER(cn) || IS_PLAYER(ch[cn].data[CHD_MASTER])) && !IS_PLAYER(co) && ch[co].gold) { money = ch[co].gold; ch[co].gold = 0; }
			do_give_exp(cn, tmp, 1, rank, money);
			
			// stronghold points for contract
			if (kill_bsp)
				do_give_bspoints(cn, tmp*kill_bsp*RATE_P_ENBS/100, 1);
			// osiris points for contract
			if (kill_osp)
				do_give_ospoints(cn, tmp*kill_osp*RATE_P_ENOS/100, 1);
			// bonus osiris points from tree
			if (kill_bos)
				do_give_ospoints(cn, tmp/20, 1);
			
			// stronghold points based on the subdriver of the npc
			if ((ch[co].data[26]>=101 && ch[co].data[26]<=399) || ch[co].temp==347)
			{
				tmp = do_char_score(co);
				
				if (IS_GLOB_MAYHEM) tmp += tmp / 5;
				if (ch[co].flags & CF_EXTRAEXP)  tmp = tmp * 2;
				if (ch[co].flags & CF_EXTRACRIT) tmp = tmp * 3/2;
				
				tmp = max(1, tmp/20);
				
				if (!IS_PLAYER(cn) && ch[cn].data[CHD_MASTER] && IS_SANEPLAYER(ch[cn].data[CHD_MASTER]))
					ch[ch[cn].data[CHD_MASTER]].data[26]++;
				else
					ch[cn].data[26]++;
				
				do_give_bspoints(cn, tmp, 1);
			}
		}
		do_char_killed(cn, co, 0);

		ch[cn].cerrno = ERR_SUCCESS;
	}
	else
	{
		if ((type==0 || type==4 || type==5 || type==8 || type==18 || type==9) && ch[co].gethit_dam>0)
		{
			thorns = RANDOM(ch[co].gethit_dam)+1;
			if (ch[cn].to_parry > ch[co].to_hit) 
			{
				thorns = max(0, thorns - (ch[cn].to_parry - ch[co].to_hit));
			}
			if (thorns>0) do_hurt(co, cn, thorns, 3);
		}
		if ((type==3 || type==13) && st_skillcount(co, 90))
		{
			thorns = dam/1000;
			if (thorns>0) do_hurt(co, cn, thorns, 90);
		}
		if (type==9 && st_skillcount(co, 96))
		{
			thorns = dam/1000;
			if (thorns>0) do_hurt(co, cn, thorns, 96);
		}
	}

	return (dam / 1000);
}

int surcheck_mapflag(int m1, int m2, int flag)
{
	if (flag == MF_NOFIGHT)
	{
		if ((map[m1].flags & MF_NOFIGHT) || (map[m2].flags & MF_NOFIGHT)) return 1;
	}
	if (flag == MF_ARENA)
	{
		if ((map[m1].flags & MF_ARENA) && (map[m2].flags & MF_ARENA)) return 1;
	}
	return 0;
}

int do_inner_surround_checks(int cn, int co)
{
	if (cn == co)                                              return 0; // Ignore - we're related or the same
	if (IS_COMPANION(co) && co == ch[cn].data[PCD_COMPANION])  return 0; // Ignore our Ghost Companion
	if (IS_COMPANION(co) && co == ch[cn].data[PCD_SHADOWCOPY]) return 0; // Ignore our Shadow Copy
	if (IS_COMPANION(cn) && cn == ch[co].data[PCD_COMPANION])  return 0; // Ignore our master if we're their Ghost Companion
	if (IS_COMPANION(cn) && cn == ch[co].data[PCD_SHADOWCOPY]) return 0; // Ignore our master if we're their Shadow Copy
	if (IS_PLAYER(cn) && IS_PLAYER(co))
	{
		if (isgroup(cn, co) && isgroup(co, cn))                return 0; // Ignore group members
		if (is_atpandium(cn) && is_atpandium(co))              return 0; // Ignore if we're fighting Pandium
		if ((!IS_CLANKWAI(cn) && !IS_CLANGORN(cn)) || 
			(IS_CLANKWAI(cn)  && !IS_CLANGORN(co)) || 
			(IS_CLANGORN(cn)  && !IS_CLANKWAI(co)) )           return 0; // Ignore if we're not members of opposing clans
	}
	if (!IS_PLAYER(cn) && !IS_PLAYER(co))
	{
		if (ch[cn].data[CHD_GROUP] == ch[co].data[CHD_GROUP])  return 0; // Ignore members of my monster group.
		if (ch[co].temp == ch[cn].data[31])                    return 0; // Ignore the monster we're supposed to be protecting
		if (ch[cn].temp == ch[co].data[31])                    return 0; // Ignore monsters protecting us
		if (ch[cn].temp == 347 && ch[co].temp == 347)          return 0; // Special check to ignore other monsters in the colloseum
		if (ch[cn].temp==CT_SHADOW && ch[co].temp==CT_SHADOW)  return 0; // Special check for Pandium Shadows
		if (ch[cn].temp==CT_PANDIUM && ch[co].temp==CT_SHADOW) return 0; // Special check for Pandium v Shadows
		if (ch[cn].temp==CT_SHADOW && ch[co].temp==CT_PANDIUM) return 0; // Special check for Shadows v Pandium
	}
	return 1;
}

int do_surround_check(int cn, int co, int gethit) // cn is the attacker, co is the target of the attack. gethit is set if the check is for a "hit".
{
	int cnatk, coatk, cnm=0, com=0, n, m1, m2;
	
	if (cn==0 || co==0 || cn==co)                                        return 0; // Ignore null values.
	
	cnatk = ch[cn].attack_cn;                                                      // Get who we're currently attacking
	coatk = ch[co].attack_cn;                                                      // Get who they're currently attacking
	
	m1 = XY2M(ch[cn].x, ch[cn].y);                                                 // Get our map flags
	m2 = XY2M(ch[co].x, ch[co].y);                                                 // Get their map flags
	
	if (IS_COMPANION(cn)) cnm = ch[cn].data[CHD_MASTER];                           // Get our master - if we're a GC
	if (IS_COMPANION(co)) com = ch[co].data[CHD_MASTER];                           // Get their master - if they're a GC
	
	if (surcheck_mapflag(m1, m2, MF_NOFIGHT) && !IS_PLAYER(co) &&                  // Ignore targets in a no-fight zone, unless the target is a monster,
		!(IS_IN_BOJ(ch[cn].x, ch[cn].y) && get_gear(cn, IT_XIXDARKSUN))) return 0; //     and we are in Boj's domain in Lab 19 and have the Dark Sun Amulet on.
	if (strcmp(ch[co].name, "Announcer")==0)                             return 0; // Ignore arena announcers
	if (surcheck_mapflag(m1, m2, MF_ARENA))
	{
		if (IS_PLAYER(cn)  && IS_PLAYER(co))                             return 1; // Allow if both the attacker and defender are players in the arena.
		if (IS_PLAYER(cnm) && IS_PLAYER(co)  && co !=cnm)                return 1; // Allow if both are in the arena, and the attacker's master is a player.
		if (IS_PLAYER(cn)  && IS_PLAYER(com) && cn !=com)                return 1; // Allow if both are in the arena, and the defender's master is a player.
		if (IS_PLAYER(cnm) && IS_PLAYER(com) && cnm!=com)                return 1; // Allow if both are in the arena, and both masters are players (but not the same player).
	}
	if (co != cnatk && coatk != cn) // We're not attacking eachother.
	{
		if (ch[co].alignment==10000)                                     return 0; // Ignore friendly npcs unless we're fighting directly.
		if (strcmp(ch[co].name, "Gate Guard")==0)                        return 0; // Ignore Stronghold Gate Guards unless we're fighting directly.
		if (strcmp(ch[co].name, "Outpost Guard")==0)                     return 0; // Ignore Stronghold Outpost Guards unless we're fighting directly.
	}
	if (cnm)     // We have a master, check on their behalf.
	{   if (com) // They have a master, check on their behalf.
			if (!do_inner_surround_checks(cnm, com))                     return 0; // Ignore if we're related or friendly.
		if (!do_inner_surround_checks(cnm, co ))                         return 0; // Ignore if we're related or friendly.
	}   if (com) // They have a master, check on their behalf.
		if (!do_inner_surround_checks(cn , com))                         return 0; // Ignore if we're related or friendly.
	if (!do_inner_surround_checks(cn , co ))                             return 0; // Ignore if we're related or friendly.
	if (gethit)
	{
		if (!do_char_can_see(cn, co, 0))                                 return 0; // Ignore things we cannot see
		if (!may_attack_msg(cn, co, 0))                                  return 0; // Ignore things we cannot attack
		if (ch[co].flags & CF_IMMORTAL)                                  return 0; // Ignore immortals, gods, etc.
	}
	return 1;
}

int do_crit(int cn, int co, int dam, int msg)
{
	int die, crit_dice, crit_chance, crit_mult, crit_dam=0, crit_redc=100;
	int in;
	
	crit_dice 	= 10000;
	crit_chance = ch[cn].crit_chance;
	crit_mult   = ch[cn].crit_multi;
	
	die = RANDOM(crit_dice) + 1;
	
	if (get_gear(cn, IT_WP_VOLCANF) && has_buff(co, SK_SCORCH))
	{
		remove_buff(co, SK_SCORCH);
		die = 0;
	}
	
	if (die<=crit_chance)
	{
		crit_dam  = dam;
		crit_dam  = crit_dam * crit_mult / 100;
		crit_dam -= dam;
		
		do_area_sound(co, 0, ch[co].x, ch[co].y, ch[cn].sound + 8);
		char_play_sound(co, ch[cn].sound + 8, -150, 0);
		if (msg)
			do_char_log(cn, 0, "Critical hit!\n");
		
		if (in = get_neck(cn, IT_GAMBLERFAL))
		{
			ch[cn].a_hp += 20000;
			if (!it[in].active) do_update_char(cn);
			it[in].active = it[in].duration;
		}
	}
	
	if (get_enchantment(co, 47)) 	crit_redc -= 50;
	if (get_enchantment(co, 76)) 	crit_redc -= 50;
	if (T_BRAV_SK(co, 12)) 			crit_redc -= 50;
	
	if (crit_redc < 0) crit_redc = 0;
	
	return max(0, crit_dam*crit_redc/100);
}

void do_attack(int cn, int co, int surround) // surround = 2 means it's a SURROUND RATE hit
{
	int hit, dam = 0, die, m, mc, mb, odam = 0, cc = 0, surrmod = 0, sorb = 0;
	int chance, s1, s2, bonus = 0, diff, crit_dam=0, in=0, co_orig=-1;
	int surrDam, surrBonus, surrTotal, n, power=0, topdam = 0;
	int glv, glv_base = 60;
	int in2 = 0;

	if (!may_attack_msg(cn, co, 1))
	{
		chlog(cn, "Prevented from attacking %s (%d)", ch[co].name, co);
		ch[cn].attack_cn = 0;
		ch[cn].cerrno = ERR_FAILED;
		return;
	}

	if (ch[co].flags & CF_STONED)
	{
		ch[cn].attack_cn = 0;
		ch[cn].cerrno = ERR_FAILED;
		return;
	}

	if (ch[cn].current_enemy!=co)
	{
		ch[cn].current_enemy = co; // reset current_enemy whenever char does something different !!!

		chlog(cn, "Attacks %s (%d)", ch[co].name, co);
	}

	add_enemy(co, cn);
	
	// s1 = Attacker // s2 = Defender
	s1 = ch[cn].to_hit;
	s2 = ch[co].to_parry;
	
	if (IS_GLOB_MAYHEM)
	{
		if (!(ch[cn].flags & CF_PLAYER))
		{
			s1 += (getrank(cn)-4)/2;
		}
		if (!(ch[co].flags & CF_PLAYER))
		{
			s2 += (getrank(co)-4)/2;
		}
	}

	if (ch[cn].flags & (CF_PLAYER))
	{
		if (ch[cn].luck<0)
		{
			s1 += ch[cn].luck / 250 - 1;
		}
	}

	if (ch[co].flags & (CF_PLAYER))
	{
		if (ch[co].luck<0)
		{
			s2 += ch[co].luck / 250 - 1;
		}
	}
	
	if (!is_facing(co, cn)) sorb = 1;
	if (is_back(co, cn))    sorb = 2;

	// Outsider's Eye & tree skill
	if (!get_neck(co, IT_TW_OUTSIDE) && !T_WARR_SK(co, 10))
	{
		s2 -= 10*sorb;
	}
	
	// Stunned or not fighting & tree skill
	if ((ch[co].stunned==1 || !ch[co].attack_cn) && !T_SORC_SK(co, 10))
	{
		s2 -= 10;
	}
	
	diff = s1 - s2;

	if      (diff<-40)
	{
		chance = 1;
		bonus  = -16;
	}
	else if (diff<-36)
	{
		chance = 2;
		bonus  = -8;
	}
	else if (diff<-32)
	{
		chance = 3;
		bonus  = -4;
	}
	else if (diff<-28)
	{
		chance = 4;
		bonus  = -2;
	}
	else if (diff<-24)
	{
		chance = 5;
		bonus  = -1;
	}
	else if (diff<-20)
	{
		chance = 6;
	}
	else if (diff<-16)
	{
		chance = 7;
	}
	else if (diff<-12)
	{
		chance = 8;
	}
	else if (diff< -8)
	{
		chance = 9;
	}
	else if (diff< -4)
	{
		chance = 10;
	}
	else if (diff<  0)
	{
		chance = 11;
	}
	else if (diff== 0)
	{
		chance = 12;
	}
	else if (diff<  4)
	{
		chance = 13;
	}
	else if (diff<  8)
	{
		chance = 14;
	}
	else if (diff< 12)
	{
		chance = 15;
	}
	else if (diff< 16)
	{
		chance = 16;
		bonus  =  1;
	}
	else if (diff< 20)
	{
		chance = 17;
		bonus  =  2;
	}
	else if (diff< 24)
	{
		chance = 18;
		bonus  =  4;
	}
	else if (diff< 28)
	{
		chance = 19;
		bonus  =  6;
	}
	else if (diff< 32)
	{
		chance = 19;
		bonus  =  9;
	}
	else if (diff< 36)
	{
		chance = 19;
		bonus  = 12;
	}
	else if (diff< 40)
	{
		chance = 19;
		bonus  = 16;
	}
	else
	{
		chance = 19;
		bonus  = 20;
	}
	
	if (mb = get_enchantment(cn, 41)) chance += mb;
	if (mb = get_enchantment(co, 37)) chance -= mb;
	
	die = RANDOM(20) + 1;
	if (die<=chance)
	{
		hit = 1;
	}
	else
	{
		hit = 0;
	}
	
	if (IS_COMPANION(cn) && !IS_SHADOW(cn) && IS_SANECHAR(cc = ch[cn].data[CHD_MASTER]) && get_tarot(cc, IT_CH_HEIROP_R) && !RANDOM(5))
		hit = 0;
	
	if (n=st_skillcount(co, 64)*5) if (RANDOM(100)<n) hit = 0;
	
	if (get_tarot(co, IT_CH_WHEEL_R) && !RANDOM(4))
		hit = 1;
	
	if (B_SK(cn, SK_ZEPHYR)) power = spell_multiplier(M_SK(cn, SK_ZEPHYR), cn);
	
	if (hit)
	{
		dam = ch[cn].weapon + RANDOM(9);
		topdam = max(0, ch[cn].top_damage);
		
		if (in = has_buff(co, SK_CALM))
		{
			topdam = topdam - bu[in].data[3];
		}
		if (topdam>1)
		{
			// Tree
			if (T_LYCA_SK(cn, 6))
				dam += max(RANDOM(topdam), RANDOM(topdam));
			else
				dam += RANDOM(topdam);
		}
		
		// Tarot - Wheel.R - 20% less damage taken
		if (get_tarot(co, IT_CH_WHEEL_R))
		{
			dam = dam*4/5;
		}
		
		odam 	  = dam;
		dam 	 += bonus;
		
		// Critical hits!!
		// This is deliberately placed after setting odam, so that crits don't make SH dumb
		if (!get_gear(cn, IT_TW_IRA))
		{
			crit_dam = do_crit(cn, co, dam, 0);
		}
		
		// Special gloves
		if (!RANDOM(20)) // 5% chance
		{
			glv = glv_base + getrank(cn)*15/2;
			if (chance_compare(co, glv+glv/2+RANDOM(20), get_target_resistance(cn, co)+RANDOM(10), 0))
			{
				in = it[ch[cn].worn[WN_ARMS]].temp;
				if (in==IT_GL_SERPENT || get_enchantment(cn, 67)) 
				{ 
					if (spell_poison(cn, co, glv, 1))
					{
						if (!(ch[cn].flags & CF_SYS_OFF)) 
							do_char_log(cn, 0, "You poisoned your enemies!\n"); 
					}
					else in=0; 
				}
				if (in==IT_GL_BURNING || get_enchantment(cn, 60) || get_gear(cn, IT_WB_BURN2HND)) 
				{ 
					if (spell_scorch(cn, co, glv, 1))
					{	
						if (!(ch[cn].flags & CF_SYS_OFF)) 
							do_char_log(cn, 0, "You scorched your enemies!\n"); 
					}
					else in=0; 
				}
				if (in==IT_GL_SHADOW || get_enchantment(cn, 71))  
				{ 
					if (spell_blind(cn, co, glv, 0))
					{	
						if (!(ch[cn].flags & CF_SYS_OFF)) 
							do_char_log(cn, 0, "You blinded your enemies!\n"); 
					}
					else in=0; 
				}
				if (in==IT_GL_CHILLED || get_enchantment(cn, 59) || get_gear(cn, IT_WB_ICE2HND)) 
				{ 
					if (spell_slow(cn, co, glv, 1))
					{ 
						if (!(ch[cn].flags & CF_SYS_OFF)) 
							do_char_log(cn, 0, "You slowed your enemies!\n");   
					}
					else in=0; 
				}
				if (in==IT_GL_CURSED || get_enchantment(cn, 63))  
				{ 
					if (spell_curse(cn, co, glv, 1))
					{
						if (!(ch[cn].flags & CF_SYS_OFF)) 
							do_char_log(cn, 0, "You cursed your enemies!\n");   
					}
					else in=0; 
				}
				if (in==IT_GL_TITANS || get_enchantment(cn, 64) || get_gear(cn, IT_WB_LAVA2HND))  
				{ 
					if (spell_weaken(cn, co, glv, 1))
					{
						if (!(ch[cn].flags & CF_SYS_OFF)) 
							do_char_log(cn, 0, "You weakened your enemies!\n"); 
					}
					else in=0; 
				}
				if (in==IT_GL_BLVIPER || get_enchantment(cn, 72)) 
				{ 
					if (spell_frostburn(cn, co, glv))
					{
						if (!(ch[cn].flags & CF_SYS_OFF)) 
							do_char_log(cn, 0, "You glaciated your enemies!\n"); 
					}
					else in=0; 
				}
				if (in==IT_TW_DOUSER || get_enchantment(cn, 68))
				{ 
					if (spell_blind(cn, co, glv, 1))
					{	
						if (!(ch[cn].flags & CF_SYS_OFF)) 
							do_char_log(cn, 0, "You doused your enemies!\n"); 
					}
					else in=0; 
				}
			}
			if (ch[co].spellfail==1) ch[co].spellfail = 0;
		}
		if (!RANDOM(20)) // 5% chance
		{
			glv = glv_base*2 + getrank(cn)*15/2;
			if (chance_compare(co, glv+glv/2+RANDOM(20), get_target_resistance(cn, co)+RANDOM(10), 0))
			{
				in = it[ch[cn].worn[WN_RHAND]].temp;
				if (in==0) in = it[ch[cn].worn[WN_RHAND]].orig_temp;
				if (in==IT_TW_LUXURIA)
				{
					if (spell_warcry(cn, co, glv, 1)) 
					{
						if (!(ch[cn].flags & CF_SYS_OFF))
							do_char_log(cn, 0, "You stunned your enemies!\n"); 
					}
					else in=0; 
				}
			}
		}
		
		co_orig = co;
		
		// Weapon damage
		if (ch[cn].flags & (CF_PLAYER))
		{
			item_damage_weapon(cn, dam+crit_dam);
		}
		
		if (sorb && (n=st_skillcount(co, 58))) dam = dam * (100-n*5)/100;
		
		dam = do_hurt(cn, co, dam+crit_dam, crit_dam>0?9:0);
		
		if (dam<1)
		{
			do_area_sound(co, 0, ch[co].x, ch[co].y, ch[cn].sound + 3);
			char_play_sound(co, ch[cn].sound + 3, -150, 0);
		}
		else
		{
			do_area_sound(co, 0, ch[co].x, ch[co].y, ch[cn].sound + 4);
			char_play_sound(co, ch[cn].sound + 4, -150, 0);
		}
		
		// Check if the attacker has Zephyr
		in2 = has_spell(cn, SK_ZEPHYR);
		
		if (IS_LYCANTH(cn)) surround = 1;
		
		if (surround && (B_SK(cn, SK_SURROUND) || IS_WPSPEAR(ch[cn].worn[WN_RHAND])))
		{
			if (B_SK(cn, SK_SURROUND))
			{
				surrmod = M_SK(cn, SK_SURROUND);
			}
			else
			{
				surrmod = (M_SK(cn, SK_DAGGER) < M_SK(cn, SK_STAFF) ? M_SK(cn, SK_DAGGER) : M_SK(cn, SK_STAFF));
				if (mb=st_skillcount(cn, 66)) surrmod = surrmod + surrmod*mb/20;
			}
			
			if (IS_LYCANTH(cn))
			{
				surrDam = odam/2 + crit_dam/2;
				glv 	= (glv_base + getrank(cn)*10)/2;
			}
			else if (IS_SKALD(cn))
			{
				surrDam = odam + crit_dam;
				glv 	= (glv_base + getrank(cn)*10);
			}
			else
			{
				surrDam = odam*3/4 + crit_dam*3/4;
				glv 	= (glv_base + getrank(cn)*10)*3/4;
			}
			
			if (surround==1 && (IS_SEYA_OR_ARTM(cn) || get_gear(cn, IT_WP_CROSSBLAD) || 
				(IS_COMPANION(cn) && IS_SANECHAR(cc = ch[cn].data[CHD_MASTER]) && get_gear(cc, IT_TW_INVIDIA)) ) && 
				!(ch[cn].flags & CF_AREA_OFF))
			{
				int surraoe, j, x, y, xf, yf, xt, yt, xc, yc, obsi = 0, aoe_power;
				double tmp_a, tmp_h, tmp_s, tmp_g;
				
				j = 100 + st_skillcount(cn, 53)*5;
				
				aoe_power = M_SK(cn, SK_PROX)+15;
				obsi 	= ch[cn].aoe_bonus;
				obsi   += (IS_SEYAN_DU(cn)?2:0)+(IS_ARCHTEMPLAR(cn)?4:0)+(get_gear(cn, IT_WP_CROSSBLAD)?4:0)+((cc&&get_gear(cc, IT_TW_INVIDIA))?6:0);
				surraoe = ((aoe_power/PROX_CAP) + obsi) * (T_SORC_SK(cn, 5)?12:10)/10 * j/100;
				tmp_a	= (double)((aoe_power*100/PROX_CAP + obsi*100) * (T_SORC_SK(cn, 5)?12:10)/10 * j/100);
				tmp_h   = (double)((sqr(aoe_power*100/PROX_HIT-tmp_a)/500 + obsi*300) * (T_SORC_SK(cn, 5)?12:10)/10 * j/100);
				tmp_s   = (double)(surrDam);
				tmp_g   = (double)(glv);
				
				xc = ch[cn].x;
				yc = ch[cn].y;
				xf = max(1, xc - surraoe);
				yf = max(1, yc - surraoe);
				xt = min(MAPX - 1, xc + 1 + surraoe);
				yt = min(MAPY - 1, yc + 1 + surraoe);

				// Loop through each target
				for (x = xf; x<xt; x++) for (y = yf; y<yt; y++) 
				{
					// This makes the radius circular instead of square
					if (sqr(xc - x) + sqr(yc - y) > (sqr(tmp_a/100) + 1))
					{
						continue;
					}
					if ((co = map[x + y * MAPX].ch) && cn!=co)
					{
						// Adjust effectiveness by radius
						surrDam = (int)(double)(min(tmp_s, tmp_s / max(1, (
							sqr(abs(xc - x)) + sqr(abs(yc - y))) / (tmp_h/100))));
						glv		= (int)(double)(min(tmp_g, tmp_g / max(1, (
							sqr(abs(xc - x)) + sqr(abs(yc - y))) / (tmp_h/100))));
						
						// Hit the target
						remember_pvp(cn, co);
						if (!do_surround_check(cn, co, 1)) continue;
						if (surrmod + RANDOM(40)>=ch[co].to_parry)
						{
							surrBonus = 0;
							if ((surrmod-ch[co].to_parry)>0)
							{
								surrBonus = odam/4 * min(max(1,surrmod-ch[co].to_parry), 20)/20;
							}
							surrTotal = surrDam+surrBonus;
							if (co==co_orig) surrTotal = surrTotal/4*3;
							if (co!=co_orig && (mb=st_skillcount(cn, 46))) surrTotal = surrTotal*(100+mb*5)/100;
							do_hurt(cn, co, surrTotal, 4);
							if (chance_compare(co, glv+glv/2+RANDOM(20), get_target_resistance(cn, co)+RANDOM(16), 0) && co!=co_orig)
							{
								if (in==IT_GL_SERPENT || get_enchantment(cn, 67)) 
									spell_poison(cn, co, glv, 1);
								if (in==IT_GL_BURNING || get_enchantment(cn, 60) || get_gear(cn, IT_WB_BURN2HND)) 
									spell_scorch(cn, co, glv, 1);
								if (in==IT_GL_SHADOW  || get_enchantment(cn, 71)) 
									spell_blind(cn, co, glv, 0);
								if (in==IT_GL_CHILLED || get_enchantment(cn, 59) || get_gear(cn, IT_WB_ICE2HND)) 
									spell_slow(cn, co, glv, 1);
								if (in==IT_GL_CURSED  || get_enchantment(cn, 63)) 
									spell_curse(cn, co, glv, 1);
								if (in==IT_GL_TITANS  || get_enchantment(cn, 64) || get_gear(cn, IT_WB_LAVA2HND)) 
									spell_weaken(cn, co, glv, 1);
								if (in==IT_GL_BLVIPER || get_enchantment(cn, 72)) 
									spell_frostburn(cn, co, glv);
								if (in==IT_TW_DOUSER  || get_enchantment(cn, 68)) 
									spell_blind(cn, co, glv, 1);
								if (in==IT_TW_LUXURIA) 
									spell_warcry(cn, co, glv, 1);
								if (ch[co].spellfail==1) 
									ch[co].spellfail = 0;
							}
							if (in2 && co!=co_orig && !get_tarot(cn, IT_CH_DEATH_R))
							{
								spell_zephyr(cn, co, bu[in2].power/2 + bu[in2].power/4, 1);
							}
							else if (power && co!=co_orig && !get_tarot(cn, IT_CH_DEATH_R))
							{
								spell_zephyr(cn, co, power/2 + power/4, 1);
							}
						}
					}
				}
			}
			else
			{
				// Regular Surround-hit checks for surrounding targets
				m = ch[cn].x + ch[cn].y * MAPX;
				
				for (n=0; n<4; n++)
				{
					switch (n)
					{
						case 0: mc = m + 1; break;
						case 1: mc = m - 1; break;
						case 2: mc = m + MAPX; break;
						case 3: mc = m - MAPX; break;
					}
					if (IS_SANECHAR(co = map[mc].ch) && ch[co].attack_cn==cn)
					{
						if ((surround==1 && (surrmod + RANDOM(40)) >= ch[co].to_parry) ||
							(surround==2 && (surrmod + RANDOM(20)) >= ch[co].to_parry))
						{
							surrBonus = 0;
							if (surround==1 && (surrmod - ch[co].to_parry)>0)
							{
								surrBonus = odam/4 * min(max(1,surrmod-ch[co].to_parry), 20)/20;
							}
							if (surround==2 && (surrmod - ch[co].to_parry)>20)
							{
								surrBonus = odam/4 * min(max(1,surrmod-ch[co].to_parry-20), 20)/20;
							}
							surrTotal = surrDam + max(0, surrBonus);
							if (co==co_orig) surrTotal = surrTotal*3/4;
							if (co!=co_orig && (mb=st_skillcount(cn, 46))) surrTotal = surrTotal*(100+mb*5)/100;
							do_hurt(cn, co, surrTotal, 4);
							if (chance_compare(co, glv+glv/2+RANDOM(20), get_target_resistance(cn, co)+RANDOM(16), 0) && co!=co_orig)
							{
								if (in==IT_GL_SERPENT || get_enchantment(cn, 67)) 
									spell_poison(cn, co, glv, 1);
								if (in==IT_GL_BURNING || get_enchantment(cn, 60) || get_gear(cn, IT_WB_BURN2HND)) 
									spell_scorch(cn, co, glv, 1);
								if (in==IT_GL_SHADOW  || get_enchantment(cn, 71)) 
									spell_blind(cn, co, glv, 0);
								if (in==IT_GL_CHILLED || get_enchantment(cn, 59) || get_gear(cn, IT_WB_ICE2HND)) 
									spell_slow(cn, co, glv, 1);
								if (in==IT_GL_CURSED  || get_enchantment(cn, 63)) 
									spell_curse(cn, co, glv, 1);
								if (in==IT_GL_TITANS  || get_enchantment(cn, 64) || get_gear(cn, IT_WB_LAVA2HND)) 
									spell_weaken(cn, co, glv, 1);
								if (in==IT_GL_BLVIPER || get_enchantment(cn, 72)) 
									spell_frostburn(cn, co, glv);
								if (in==IT_TW_DOUSER  || get_enchantment(cn, 68)) 
									spell_blind(cn, co, glv, 1);
								if (in==IT_TW_LUXURIA) 
									spell_warcry(cn, co, glv, 1);
								if (ch[co].spellfail==1) 
									ch[co].spellfail = 0;
							}
							if (in2 && co!=co_orig && !get_tarot(cn, IT_CH_DEATH_R))
							{
								spell_zephyr(cn, co, bu[in2].power/2 + bu[in2].power/4, 1);
							}
							else if (power && co!=co_orig && !get_tarot(cn, IT_CH_DEATH_R))
							{
								spell_zephyr(cn, co, power/2 + power/4, 1);
							}
						}
					}
				}
			}
		}
		if (in2 && !get_tarot(cn, IT_CH_DEATH_R))
		{
			spell_zephyr(cn, co_orig, bu[in2].power, 1);
		}
		else if (power && !get_tarot(cn, IT_CH_DEATH_R))
		{
			spell_zephyr(cn, co_orig, power, 1);
		}
	}
	else
	{
		int thorns;
		
		do_area_sound(co, 0, ch[co].x, ch[co].y, ch[cn].sound + 5);
		char_play_sound(co, ch[cn].sound + 5, -150, 0);

		do_area_notify(cn, co, ch[cn].x, ch[cn].y, NT_SEEMISS, cn, co, 0, 0);
		do_notify_char(co, NT_GOTMISS, cn, 0, 0, 0);
		do_notify_char(cn, NT_DIDMISS, co, 0, 0, 0);
				
		// Tree - artm
		if ((thorns = RANDOM(ch[co].gethit_dam)+1)>0 && T_ARTM_SK(co, 6))
		{
			if (ch[cn].to_parry > ch[co].to_hit) 
			{
				thorns = max(0, thorns - (ch[cn].to_parry - ch[co].to_hit));
			}
			if (thorns>0) do_hurt(co, cn, thorns, 13);
		}
	}
	
	// Tarot - Death.R : Trigger Zephyr when attacked
	if (B_SK(co, SK_ZEPHYR) && get_tarot(co, IT_CH_DEATH_R))
	{
		power = spell_multiplier(M_SK(co, SK_ZEPHYR), co);
		
		if (in2 = has_spell(co, SK_ZEPHYR))
		{
			spell_zephyr(co, cn, bu[in2].power, 1);
		}
		else if (power && get_tarot(co, IT_CH_DEATH_R))
		{
			spell_zephyr(co, cn, power, 1);
		}
	}
}

int do_maygive(int cn, int co, int in)
{
	if (in<1 || in>=MAXITEM)
	{
		return 1;
	}
	if (it[in].temp==IT_LAGSCROLL)
	{
		return 0;
	}
	if ((it[in].temp==IT_COMMAND1 || it[in].temp==IT_COMMAND2 || it[in].temp==IT_COMMAND3 || it[in].temp==IT_COMMAND4) && IS_PLAYER(co))
	{
		return 0;
	}
	
	return 1;
}

void do_give(int cn, int co)
{
	int tmp, in;

	if (!ch[cn].citem)
	{
		ch[cn].cerrno = ERR_FAILED;
		return;
	}
	in = ch[cn].citem;

	ch[cn].cerrno = ERR_SUCCESS;

	do_update_char(cn);
	do_update_char(co);

	if (in & 0x80000000)
	{
		tmp = in & 0x7FFFFFFF;
		ch[co].gold += tmp;
		if (!(ch[cn].flags & CF_SYS_OFF))
			do_char_log(cn, 1, "You give the gold to %s.\n", ch[co].name);
		do_char_log(co, 0, "You got %dG %dS from %s.\n",
		            tmp / 100, tmp % 100, ch[cn].name);
		if (ch[cn].flags & (CF_PLAYER))
		{
			chlog(cn, "Gives %s (%d) %dG %dS", ch[co].name, co, tmp / 100, tmp % 100);
		}

		do_notify_char(co, NT_GIVE, cn, 0, tmp, 0);

		ch[cn].citem = 0;

		do_update_char(cn);

		return;
	}

	if (!do_maygive(cn, co, in))
	{
		do_char_log(cn, 0, "You're not allowed to do that!\n");
		ch[cn].misc_action = DR_IDLE;
		return;
	}

	chlog(cn, "Gives %s (%d) to %s (%d)", it[in].name, in, ch[co].name, co);

	if (it[in].driver==31 && (ch[co].flags & CF_UNDEAD))
	{
		if (ch[cn].flags & CF_NOMAGIC)
		{
			do_char_log(cn, 0, "It doesn't work! An evil aura is present.\n");
			ch[cn].misc_action = DR_IDLE;
			return;
		}
		if (ch[co].temp == CT_DRACULA)
			do_hurt(cn, co, it[in].data[0]*10, 2);
		else
			do_hurt(cn, co, it[in].data[0], 2);
		it[in].used  = USE_EMPTY;
		ch[cn].citem = 0;
		return;
	}
	
	/*
	if ((ch[co].flags & (CF_PLAYER)) && (it[in].flags & IF_SHOPDESTROY))
	{
		do_char_log(cn, 0, "Beware! The gods see what you're doing.\n");
	}
	*/

	if (ch[co].citem)
	{
		tmp = god_give_char(in, co);

		if (tmp)
		{
			ch[cn].citem = 0;
			if (!(ch[cn].flags & CF_SYS_OFF))
				do_char_log(cn, 1, "You give %s to %s.\n", it[in].name, ch[co].name);
		}
		else
		{
			ch[cn].misc_action = DR_IDLE;
		}
	}
	else
	{
		ch[cn].citem = 0;
		ch[co].citem = in;
		it[in].carried = co;

		do_update_char(cn);
		
		if (!(ch[cn].flags & CF_SYS_OFF))
			do_char_log(cn, 1, "You give %s to %s.\n", it[in].name, ch[co].name);
	}
	do_notify_char(co, NT_GIVE, cn, in, 0, 0);
}

int invis_level(int cn)
{
	if (ch[cn].flags & CF_GREATERINV)
	{
		return( 15);
	}
	if (ch[cn].flags & CF_GOD)
	{
		return( 10);
	}
	if (ch[cn].flags & (CF_IMP | CF_USURP))
	{
		return( 5);
	}
	if (ch[cn].flags & CF_STAFF)
	{
		return( 2);
	}
	return 1;
}

int is_walking(int cn)
{
	if (ch_base_status(ch[cn].status) >= 16 && ch_base_status(ch[cn].status) <= 152)
		return 1;
	
	return 0;
}

int do_char_can_see(int cn, int co, int flag)
{
	int d, d1, d2, light, rd;
	unsigned long long prof;

	if (cn==co)
	{
		return 1;
	}

	if (ch[co].used!=USE_ACTIVE)
	{
		return 0;
	}
	if ((ch[co].flags & CF_INVISIBLE) && invis_level(cn)<invis_level(co))
	{
		return 0;
	}
	if (ch[co].flags & CF_BODY)
	{
		return 0;
	}

	prof = prof_start();

	// raw distance:
	d1 = abs(ch[cn].x - ch[co].x);
	d2 = abs(ch[cn].y - ch[co].y);

	// Fixed the distance changes by dividing by (9/4), or 2.25
	rd = d = (d1*d1 + d2*d2) / (9/4);

	if (d>1000)
	{
		prof_stop(21, prof);
		return 0;
	}                                             // save some time...
	
	// x+, y-
	if ((ch[co].y <= ch[cn].y-16 && ch[co].x >= ch[cn].x+20) || (ch[co].y <= ch[cn].y-20 && ch[co].x >= ch[cn].x+16))
	{
		prof_stop(21, prof);
		return 0;
	}
	
	// modify by perception and stealth:
	if ((ch[co].alignment > 0 && !(ch[co].flags & CF_PLAYER) && !(B_SK(co, SK_STEALTH))) ||
		(isgroup(co, cn) && isgroup(cn, co)) || (flag==1))
	{
		d = 0;
	}
	else if (ch[co].mode==0)	// slow
	{
		d = (d * (M_SK(co, SK_STEALTH)*2)) / 10;
	}
	else if (ch[co].mode==1)	// normal
	{
		d = (d * (M_SK(co, SK_STEALTH)*2)) / 30;
	}
	else						// fast
	{
		d = (d * (M_SK(co, SK_STEALTH)*2)) / 90;
	}
	
	// Enchant & Idle
	if (get_enchantment(co, 28) && !is_walking(co))
	{
		d = d * 5/4;
	}
	// Enchant & Moving
	if (get_enchantment(co, 6) && is_walking(co))
	{
		d = d * 5/4;
	}

	d -= M_SK(cn, SK_PERCEPT) * 4;

	// modify by light:
	if (flag==0 && !(ch[cn].flags & CF_INFRARED))
	{
		light = max(map[ch[co].x + ch[co].y * MAPX].light, check_dlight(ch[co].x, ch[co].y));
		light = do_char_calc_light(cn, light);

		if (light==0)
		{
			prof_stop(21, prof);
			return 0;
		}
		if (light>64)
		{
			light = 64;
		}
		d += (64 - light) * 2;
	}

	if (rd<3 && d>70)
	{
		d = 70;
	}
	if (d>200)
	{
		prof_stop(21, prof);
		return 0;
	}

	if (!can_see(cn, ch[cn].x, ch[cn].y, ch[co].x, ch[co].y, TILEX/2))
	{
		prof_stop(21, prof);
		return 0;
	}

	prof_stop(21, prof);

	if (d<1)
	{
		return 1;
	}

	return(d);
}

int do_char_can_see_item(int cn, int in)
{
	int d, d1, d2, light, rd;
	unsigned long long prof;

	if (it[in].used!=USE_ACTIVE)
	{
		return 0;
	}

	// raw distance:
	d1 = abs(ch[cn].x - it[in].x);

	d2 = abs(ch[cn].y - it[in].y);
	
	// Fixed the distance changes by dividing by (9/4), or 2.25
	rd = d = (d1*d1 + d2*d2) / (9/4);

	if (d>1000)
	{
		return 0;   // save some time...
	}
	prof = prof_start();

	// modify by perception
	d += 50 - M_SK(cn, SK_PERCEPT) * 2;

	// modify by light:
	if (!(ch[cn].flags & CF_INFRARED))
	{
		light = max(map[it[in].x + it[in].y * MAPX].light, check_dlight(it[in].x, it[in].y));
		light = do_char_calc_light(cn, light);

		if (light==0)
		{
			prof_stop(22, prof);
			return 0;
		}
		if (light>64)
		{
			light = 64;
		}
		d += (64 - light) * 3;
	}

	if ((it[in].flags & IF_HIDDEN) && !IS_BUILDING(cn))
	{
		if (it[in].driver==57 && it[in].data[8] && IS_RB(cn)) return 1;
		else d += it[in].data[9];
	}
	else if (rd<3 && d>200)
	{
		d = 200;
	}

	if (d>200)
	{
		prof_stop(22, prof);
		return 0;
	}

	if (!can_see(cn, ch[cn].x, ch[cn].y, it[in].x, it[in].y, TILEX/2))
	{
		prof_stop(22, prof);
		return 0;
	}

	prof_stop(22, prof);

	if (d<1)
	{
		return 1;
	}

	return(d);
}

int do_add_stat(int v, int mar, int gem, int sku)
{
	int n;
	
	if (sku && v<0)		v = abs(v);
	if (gem)			v = v*7/5;
	if (mar && v<0)		v = v*2/3;
	
	return v;
}

void do_update_char(int cn)
{
	ch[cn].flags |= (CF_UPDATE | CF_SAVEME);
}
void really_update_char(int cn)
{
	int n, m, t, oldlight, z, sublight = 0, maxlight = 0, co=0, cz=0, bits=0;
	int hp = 0, end = 0, mana = 0, weapon = 0, armor = 0, light = 0, gethit = 0, infra = 0, coconut = 0, pigsblood = 0;
	int heal_hp, heal_end, heal_mana, tmpa = 0, tmpw = 0, act = 0;
	int tempWeapon = 0, tempArmor = 0, bbelt = 0, wbelt = 0;
	int isCurse1 = 0, isSlow1 = 0, isWeaken1 = 0, isCurse2 = 0, isSlow2 = 0, isWeaken2 = 0;
	int divCursed = 1, divSlowed = 1, divWeaken = 1;
	int hastePower = 0, slowPower = 0, hasteSpeed = 0, slowSpeed = 0, slow2Speed = 0, sickStacks = 0;
	int attrib[5];
	int attrib_ex[5];
	int skill[50];
	unsigned long long prof;
	int charmSpec = 0, gearSpec = 0, gemCheck = 0, gemSpec = 0, enchantSpec = 0, symSpec = 0, skuaSpec = 0;
	int moreBrv = 0, moreWil = 0, moreInt = 0, moreAgl = 0, moreStr = 0, moreEnd = 0, moreAtt = 0, moreGrd = 0, moreDmg = 0;
	int in=0;
	//int speedvalue_a = 0, speedvalue_b = 0, speedvalue_c = 0;
	int base_spd = 0, spd_move = 0, spd_attack = 0, spd_cast = 0, inunderdark = 0;
	int spell_mod = 0, spell_apt = 0, spell_cool = 0;
	int critical_b = 0, critical_c = 0, critical_m = 0;
	int hit_rate = 0, parry_rate = 0, loverSplit = 0;
	int damage_top = 0, ava_crit = 0, ava_mult = 0;
	int aoe = 0, tempCost = 10000, dmg_bns = 10000, dmg_rdc = 10000, reduc_bonus = 0;
	int suppression = 0, bcount=0, attaunt=0, labcmd=0, gcdivinity = 0, empty = 0, unarmed = 1, emptyring = 0;
	
	prof = prof_start();

	ch[cn].flags &= ~(CF_NOHPREG | CF_NOENDREG | CF_NOMANAREG);
	ch[cn].sprite_override = 0;

	m = ch[cn].x + ch[cn].y * MAPX;

	// No-magic zone check -- except if you have the sun ammy or dark-sun ammy equipped
	if (((map[m].flags & MF_NOMAGIC) && 
		!char_wears_item(cn, 466) && !char_wears_item(cn, 481) && !char_wears_item(cn, IT_AM_TRUESUN)) || 
		char_wears_item(cn, IT_AM_FALMOON))
	{
		if (!(ch[cn].flags & CF_NOMAGIC))
		{
			ch[cn].flags |= CF_NOMAGIC;
			remove_spells(cn);
			do_char_log(cn, 0, "You feel your magic fail.\n");
		}
	}
	else
	{
		if (ch[cn].flags & CF_NOMAGIC)
		{
			ch[cn].flags &= ~CF_NOMAGIC;
			do_update_char(cn);
			do_char_log(cn, 0, "You feel your magic return.\n");
		}
	}

	oldlight = ch[cn].light;

	for (n = 0; n<5; n++) attrib[n] = 0;
	
	heal_hp   = 0;
	heal_end  = 0;
	heal_mana = 0;
	
	ch[cn].hp[4] = 0;
	hp = 0;
	ch[cn].end[4] = 0;
	end = 0;
	ch[cn].mana[4] = 0;
	mana = 0;

	for (n = 0; n<MAXSKILL; n++) skill[n] = 0;

	ch[cn].armor = 0;
	armor = 0;
	ch[cn].weapon = 0;
	weapon = 0;
	ch[cn].gethit_dam = 0;
	gethit = 0;
	ch[cn].stunned = 0;
	ch[cn].taunted = 0;
	ch[cn].light = 0;
	light = 0;
	maxlight = 0;
	for (n = 0; n<3; n++) ch[cn].gigaregen[n] = 0;
	
	base_spd = spd_move = spd_attack = spd_cast = 0;
	spell_mod = spell_apt = spell_cool = 0;
	critical_b = critical_c = critical_m = 0;
	hit_rate = parry_rate = 0;
	damage_top = 0;
	dmg_bns = dmg_rdc = tempCost = 10000;
	
	if (IS_PLAYER_COMP(cn)) dmg_rdc = 5000;
	
	if (!(ch[cn].flags & CF_NOMAGIC))
	{
		if (get_gear(cn, IT_TW_MARCH))     gearSpec |= 4;
		if (get_gear(cn, IT_WP_GEMCUTTER)) gemCheck  = 1;
		if (get_gear(cn, IT_SIGN_SKUA))    skuaSpec  = 1;
	}
	
	if (ch[cn].worn[WN_RHAND]==0 && ch[cn].worn[WN_LHAND]==0) critical_b += 1;
	
	if (IS_SEYAN_DU(cn) && !get_gear(cn, IT_SEYANSWORD))
	{
		bits           = get_seyan_bits(cn);
		
		for (z = 0; z<5; z++) 
			attrib[z] += max(0,min( 5, (bits-17)/3));
		
		hit_rate      += max(0,min( 5, (bits- 2)/6));
		parry_rate    += max(0,min( 5, (bits- 2)/6));
		
		critical_c    += max(0,min(50, (bits- 2)*10/6));
		critical_m    += max(0,min(10, (bits- 2)*2/6));
		spell_mod     += max(0,min( 5, (bits- 2)/6));
	}
	
	if (has_item(cn, IT_COMMAND3))
	{
		labcmd = 1;
	}
	if (has_buff(cn, SK_DWLIGHT))
	{
		inunderdark = 1;
	}
	
	if (IS_COMP_TEMP(cn) && IS_SANECHAR(co = ch[cn].data[CHD_MASTER]) && ch[cn].data[1]==4) { cz=cn; cn=co; }
	
	// Loop through gear slots
	for (n = 0; n<13; n++)
	{
		if (!ch[cn].worn[n])
		{
			if (n!=WN_CHARM && n!=WN_CHARM2 && n!=WN_LHAND) empty++;
			if (n==WN_LRING || n==WN_RRING) emptyring++;
			if (IS_PLAYER(cn) && n==WN_RHAND) unarmed++;
			continue;
		}
		
		m = ch[cn].worn[n];
		
		if (it[m].active)	act = 1;
		else				act = 0;
		
		gemSpec = 0; symSpec = 0;
		
		// Stat bonuses that are only awarded outside of no-magic zones
		if (!(ch[cn].flags & CF_NOMAGIC))
		{
			if ((it[m].temp==IT_BL_SOLAR || it[m].temp==IT_BL_LUNAR || it[m].temp==IT_BL_ECLIPSE) && 
				(globs->mdtime<MD_HOUR*6 || globs->mdtime>MD_HOUR*18)) act = 1; // Night Time
			if (n == WN_LRING || n == WN_RRING)
			{
				if (gemCheck) gemSpec = 1;    // Gemcutter
				if (it[m].temp==IT_SIGN_SYMM) // Signet of Symmetry
				{
					if (n == WN_LRING) symSpec =  1;
					if (n == WN_RRING) symSpec = -1;
					if (ch[cn].worn[n+symSpec]) m = ch[cn].worn[n+symSpec];
					else continue;
				}
			}
			
			// Attributes
			for (z = 0; z<5; z++) attrib[z] += do_add_stat(it[m].attrib[z][act], 0, gemSpec, skuaSpec);
			
			hp   += do_add_stat(it[m].hp[act],   0, gemSpec, skuaSpec);
			end  += do_add_stat(it[m].end[act],  0, gemSpec, skuaSpec);
			mana += do_add_stat(it[m].mana[act], 0, gemSpec, skuaSpec);
			
			// Skills
			for (z = 0; z<MAXSKILL; z++) skill[z] += do_add_stat(it[m].skill[z][act], 0, gemSpec, skuaSpec);
			
			// Meta values
			base_spd   += do_add_stat(it[m].speed[act],      (gearSpec&4), gemSpec, skuaSpec);
			spd_move   += do_add_stat(it[m].move_speed[act], (gearSpec&4), gemSpec, skuaSpec);
			spd_attack += do_add_stat(it[m].atk_speed[act],  (gearSpec&4), gemSpec, skuaSpec);
			spd_cast   += do_add_stat(it[m].cast_speed[act], (gearSpec&4), gemSpec, skuaSpec);
			spell_mod  += do_add_stat(it[m].spell_mod[act],  0, gemSpec, skuaSpec);
			spell_apt  += do_add_stat(it[m].spell_apt[act],  0, gemSpec, skuaSpec);
			spell_cool += do_add_stat(it[m].cool_bonus[act], 0, gemSpec, skuaSpec);
			aoe        += do_add_stat(it[m].aoe_bonus[act],  0, gemSpec, skuaSpec);
			
			if (it[m].temp==IT_CH_TEMPER_R || (IS_SINBINDER(m) && it[m].data[2]==IT_CH_TEMPER_R))	charmSpec |=    1;
			if (it[m].temp==IT_CH_LOVERS   || (IS_SINBINDER(m) && it[m].data[2]==IT_CH_LOVERS)) 	charmSpec |=    2;
			if (it[m].temp==IT_CH_MAGI     || (IS_SINBINDER(m) && it[m].data[2]==IT_CH_MAGI))   	charmSpec |=    4;
			if (it[m].temp==IT_CH_HERMIT   || (IS_SINBINDER(m) && it[m].data[2]==IT_CH_HERMIT)) 	charmSpec |=    8;
			if (it[m].temp==IT_CH_FOOL_R   || (IS_SINBINDER(m) && it[m].data[2]==IT_CH_FOOL_R)) 	charmSpec |=   16;
			if (it[m].temp==IT_CH_STRENGTH || (IS_SINBINDER(m) && it[m].data[2]==IT_CH_STRENGTH))	charmSpec |=   32;
			if (it[m].temp==IT_CH_WHEEL    || (IS_SINBINDER(m) && it[m].data[2]==IT_CH_WHEEL))		charmSpec |=   64;
			if (it[m].temp==IT_CH_MAGI_R   || (IS_SINBINDER(m) && it[m].data[2]==IT_CH_MAGI_R))		charmSpec |=  128;
			if (it[m].temp==IT_CH_MOON_R   || (IS_SINBINDER(m) && it[m].data[2]==IT_CH_MOON_R))		charmSpec |=  256;
			if (it[m].temp==IT_CH_LOVERS_R || (IS_SINBINDER(m) && it[m].data[2]==IT_CH_LOVERS_R))	charmSpec |= 1024;
			if (it[m].temp==IT_CH_STRENG_R || (IS_SINBINDER(m) && it[m].data[2]==IT_CH_STRENG_R))	charmSpec |= 4096;
			if (it[m].temp==IT_CH_HANGED_R || (IS_SINBINDER(m) && it[m].data[2]==IT_CH_HANGED_R))	charmSpec |= 8192;
			
			if (it[m].temp==IT_BOOK_TRAV 	|| it[m].orig_temp==IT_BOOK_TRAV) 		gearSpec  |=    1;
			if (it[m].temp==IT_IMBK_TRAV 	|| it[m].orig_temp==IT_IMBK_TRAV) 		gearSpec  |=    1;
			if (it[m].temp==IT_TW_HEAVENS 	|| it[m].orig_temp==IT_TW_HEAVENS)		gearSpec  |=    2;
			if (it[m].temp==IT_WP_GILDSHINE || it[m].orig_temp==IT_WP_GILDSHINE)	gearSpec  |=    8;
			if (it[m].temp==IT_WB_GILDSHINE || it[m].orig_temp==IT_WB_GILDSHINE)	gearSpec  |=    8;
			if (it[m].temp==IT_WP_KELPTRID 	|| it[m].orig_temp==IT_WP_KELPTRID)		gearSpec  |=   16;
			if (it[m].temp==IT_WB_KELPTRID 	|| it[m].orig_temp==IT_WB_KELPTRID)		gearSpec  |=   16;
			if (it[m].temp==IT_WP_WHITEODA 	|| it[m].orig_temp==IT_WP_WHITEODA)		gearSpec  |=   32;
			if (it[m].temp==IT_WB_WHITEODA 	|| it[m].orig_temp==IT_WB_WHITEODA)		gearSpec  |=   32;
			if (it[m].temp==IT_WP_BLACKTAC 	|| it[m].orig_temp==IT_WP_BLACKTAC)		gearSpec  |=   64;
			if (it[m].temp==IT_WB_BLACKTAC 	|| it[m].orig_temp==IT_WB_BLACKTAC)		gearSpec  |=   64;
			if (it[m].temp==IT_WP_EXCALIBUR || it[m].orig_temp==IT_WP_EXCALIBUR)	gearSpec  |=  128;
			if (it[m].temp==IT_TW_AVARITIA 	|| it[m].orig_temp==IT_TW_AVARITIA)		gearSpec  |=  256;
			if (it[m].temp==IT_BOOK_HOLY 	|| it[m].orig_temp==IT_BOOK_HOLY)		gearSpec  |=  512;
			if (it[m].temp==IT_IMBK_HOLY 	|| it[m].orig_temp==IT_IMBK_HOLY)		gearSpec  |=  512;
			if (it[m].temp==IT_BOOK_PROD 	|| it[m].orig_temp==IT_BOOK_PROD)		gearSpec  |= 1024;
			if (it[m].temp==IT_IMBK_PROD 	|| it[m].orig_temp==IT_IMBK_PROD)		gearSpec  |= 1024;
			if (it[m].temp==IT_WB_BEINESTOC || it[m].orig_temp==IT_WB_BEINESTOC)	gearSpec  |= 2048;
			if (it[m].temp==IT_BOOK_GRAN 	|| it[m].orig_temp==IT_BOOK_GRAN) 		gearSpec  |= 4096;
			if (it[m].temp==IT_TW_SUPERBIA 	|| it[m].orig_temp==IT_TW_SUPERBIA) 	gearSpec  |= 8192;
			if (it[m].temp==IT_WB_LIONSPAWS || it[m].orig_temp==IT_WB_LIONSPAWS)	gearSpec  |=16384; // BRV/BRV/BRV
			if (it[m].temp==IT_WB_GULLOXI   || it[m].orig_temp==IT_WB_GULLOXI)		gearSpec  |=32768; // BRV/AGL/AGL
			if (it[m].temp==IT_WB_BARBSWORD || it[m].orig_temp==IT_WB_BARBSWORD)	gearSpec  |=65536; // BRV/STR/STR
			
			if (it[m].enchantment==  1)		enchantSpec |=    1;
			if (it[m].enchantment==  5)   { moreBrv+=3; if (IS_TWOHAND(m)) moreBrv+=3; }
			if (it[m].enchantment== 11)		enchantSpec |=    2;
			if (it[m].enchantment== 12)   { moreWil+=3; if (IS_TWOHAND(m)) moreWil+=3; }
			if (it[m].enchantment== 17)		infra = 15;
			if (it[m].enchantment== 18)   { moreEnd++; if (IS_TWOHAND(m)) moreEnd++; }
			if (it[m].enchantment== 19)   { moreInt+=3; if (IS_TWOHAND(m)) moreInt+=3; }
			if (it[m].enchantment== 24)		enchantSpec |=   16;
			if (it[m].enchantment== 25)   { moreAgl+=3; if (IS_TWOHAND(m)) moreAgl+=3; }
			if (it[m].enchantment== 26)		enchantSpec |=   32;
			if (it[m].enchantment== 29)   { moreStr+=3; if (IS_TWOHAND(m)) moreStr+=3; }
			if (it[m].enchantment== 40)		enchantSpec |=   64;
			if (it[m].enchantment== 44)		enchantSpec |=  128;
			if (it[m].enchantment== 48)		enchantSpec |=  256;
			if (it[m].enchantment== 50)		moreDmg++;
			if (it[m].enchantment== 51)		moreGrd++;
			if (it[m].enchantment== 55)		enchantSpec |= 1024;
			if (it[m].enchantment== 54)   { moreAtt+=2; if (IS_TWOHAND(m)) moreAtt+=2; }
			if (it[m].enchantment== 61)		enchantSpec |= 2048;
			if (it[m].enchantment== 62)		enchantSpec |= 4096;
			if (it[m].enchantment== 58)		enchantSpec |= 8192;
			if (it[m].enchantment== 74)		enchantSpec |= 16384;
			if (it[m].enchantment== 75)		enchantSpec |= 32768;
		}
		
		critical_b += do_add_stat(it[m].base_crit, 0, gemSpec, skuaSpec);
		
		if (it[m].temp == IT_TW_BBELT)
		{
			if (ch[cn].worn[WN_RHAND]==0)
			{
				critical_b += 2;
				bbelt=1;
				
				if (ch[cn].worn[WN_LHAND]==0)
				{
					critical_b += 2;
				}
			}
		}
		
		if (it[m].temp == IT_WHITEBELT && ch[cn].worn[WN_RHAND]==0 && ch[cn].worn[WN_LHAND]==0)
		{
			critical_b += 2;
			wbelt=1;
		}
		
		// WV, AV
		tempArmor  = do_add_stat(it[m].armor[act],      0, gemSpec, skuaSpec);
		tempWeapon = do_add_stat(it[m].weapon[act],     0, gemSpec, skuaSpec);
		gethit    += do_add_stat(it[m].gethit_dam[act], 0, gemSpec, skuaSpec);
		
		if ((!labcmd || it[m].temp==91) && !inunderdark)
		{
			maxlight     += do_add_stat(it[m].light[act], 0, gemSpec, skuaSpec);
			if (it[m].light[act]>light)
				light     = do_add_stat(it[m].light[act], 0, gemSpec, skuaSpec);
			else if (it[m].light[act]<0)
				sublight -= do_add_stat(it[m].light[act], 0, gemSpec, skuaSpec);
		}
		
		// More meta values
		critical_c += do_add_stat(it[m].crit_chance[act], 0, gemSpec, skuaSpec);
		critical_m += do_add_stat(it[m].crit_multi[act],  0, gemSpec, skuaSpec);
		hit_rate   += do_add_stat(it[m].to_hit[act],      0, gemSpec, skuaSpec);
		parry_rate += do_add_stat(it[m].to_parry[act],    0, gemSpec, skuaSpec);
		damage_top += do_add_stat(it[m].top_damage[act],  0, gemSpec, skuaSpec);
		dmg_bns     = dmg_bns * (200 + (do_add_stat(it[m].dmg_bonus[act],     0, gemSpec, skuaSpec)))/200;
		dmg_rdc     = dmg_rdc * (200 - (do_add_stat(it[m].dmg_reduction[act], 0, gemSpec, skuaSpec)))/200;
		//
		
		if (IS_WPDUALSW(m))
		{
			if (z=st_skillcount(cn, 30)) tempWeapon += tempWeapon*z*2/25;
			if (IS_ANY_TEMP(cn) && it[m].temp != IT_WP_FELLNIGHT)
			{
				tempWeapon /= 2;
			}
		}
		if (n == WN_LHAND && IS_WPDAGGER(m) && it[m].temp != IT_WP_QUICKSILV)
		{
			tempWeapon /= 2;
		}
		if (IS_WPSHIELD(m))
		{
			if (z=st_skillcount(cn, 21))  tempArmor += tempArmor *z*2/25;
			if (n == WN_RHAND)
			{
				tempWeapon += tempArmor;
				tempArmor  /= 2;
			}
			if (z=st_skillcount(cn, 18)) gethit     += tempArmor *z/10;
			if (z=st_skillcount(cn, 24)) parry_rate += tempArmor *z/20;
		}
		
		weapon += tempWeapon;
		armor  += tempArmor;
	}
	
	// GC may inherit tarots from owner
	if (IS_COMP_TEMP(cn) && IS_SANECHAR(co = ch[cn].data[CHD_MASTER]) && T_SUMM_SK(co, 10))
	{
		for (n = 0; n<13; n++)
		{
			if (!ch[co].worn[n]) continue;
			
			m = ch[co].worn[n];
			
			if (it[m].temp==IT_CH_TEMPER_R || (IS_SINBINDER(m) && it[m].data[2]==IT_CH_TEMPER_R))	charmSpec |=    1;
			if (it[m].temp==IT_CH_LOVERS   || (IS_SINBINDER(m) && it[m].data[2]==IT_CH_LOVERS)) 	charmSpec |=    2;
			if (it[m].temp==IT_CH_MAGI     || (IS_SINBINDER(m) && it[m].data[2]==IT_CH_MAGI))   	charmSpec |=    4;
			if (it[m].temp==IT_CH_HERMIT   || (IS_SINBINDER(m) && it[m].data[2]==IT_CH_HERMIT)) 	charmSpec |=    8;
			if (it[m].temp==IT_CH_FOOL_R   || (IS_SINBINDER(m) && it[m].data[2]==IT_CH_FOOL_R)) 	charmSpec |=   16;
			if (it[m].temp==IT_CH_STRENGTH || (IS_SINBINDER(m) && it[m].data[2]==IT_CH_STRENGTH))	charmSpec |=   32;
			if (it[m].temp==IT_CH_WHEEL    || (IS_SINBINDER(m) && it[m].data[2]==IT_CH_WHEEL))		charmSpec |=   64;
			if (it[m].temp==IT_CH_MAGI_R   || (IS_SINBINDER(m) && it[m].data[2]==IT_CH_MAGI_R))		charmSpec |=  128;
			if (it[m].temp==IT_CH_MOON_R   || (IS_SINBINDER(m) && it[m].data[2]==IT_CH_MOON_R))		charmSpec |=  256;
			if (it[m].temp==IT_CH_LOVERS_R || (IS_SINBINDER(m) && it[m].data[2]==IT_CH_LOVERS_R))	charmSpec |= 1024;
			if (it[m].temp==IT_CH_STRENG_R || (IS_SINBINDER(m) && it[m].data[2]==IT_CH_STRENG_R))	charmSpec |= 4096;
			if (it[m].temp==IT_CH_HANGED_R || (IS_SINBINDER(m) && it[m].data[2]==IT_CH_HANGED_R))	charmSpec |= 8192;
		}
	}
	
	// Feb 2020 - Store the current armor and weapon values from your gear, before other additions.
	// This will be used after the stats are updated for armor and weapon mastery
	tempArmor  = armor;
	tempWeapon = weapon;
	
	armor    += ch[cn].armor_bonus;
	weapon   += ch[cn].weapon_bonus;
	gethit   += ch[cn].gethit_bonus;
	if (!labcmd)
	{
		maxlight += ch[cn].light_bonus;
		light    += ch[cn].light_bonus;
	}
	
	suppression = 0;
	
	if (cz) cn = cz;
	
	// Check first for existing debuffs that conflict with other debuffs
	for (n = 0; n<MAXBUFFS; n++)
	{
		if (!ch[cn].spell[n]) continue;
		
		m = ch[cn].spell[n];
		
		// isCurse1 = 0, isSlow1 = 0, isWeaken1 = 0, isCurse2 = 0, isSlow2 = 0, isWeaken2 = 0;
		
		// Halves other debuff
		if (bu[m].temp==SK_CURSE  ) isCurse1  = bu[m].power;
		if (bu[m].temp==SK_SLOW   ) isSlow1   = bu[m].power;
		if (bu[m].temp==SK_WEAKEN ) isWeaken1 = bu[m].power;
		if (bu[m].temp==SK_CURSE2 ) isCurse2  = bu[m].power;
		if (bu[m].temp==SK_SLOW2  ) isSlow2   = bu[m].power;
		if (bu[m].temp==SK_WEAKEN2) isWeaken2 = bu[m].power;
	}
	
	if (isCurse1  >= isCurse2 )	isCurse1  = 0; else isCurse2  = 0;
	if (isSlow2   >= isSlow1  )	isSlow2   = 0; else isSlow1   = 0;
	if (isWeaken1 >= isWeaken2)	isWeaken1 = 0; else isWeaken2 = 0;
	
	if (IS_PLAYER_COMP(cn) && IS_SANECHAR(co = ch[cn].data[CHD_MASTER]))
	{
		for (n = 0; n<MAXBUFFS; n++)
		{
			if (!ch[co].spell[n]) continue;
			m = ch[co].spell[n];
			if ((ch[co].flags & CF_NOMAGIC) && !bu[m].data[4]) continue;
			
			// Lab 6 infrared potions
			if (bu[m].temp==635) infra |= 1;
			if (bu[m].temp==637) infra |= 2;
			if (bu[m].temp==639) infra |= 4;
			if (bu[m].temp==641) infra |= 8;
			
			// Coconut removes heatstroke
			if (bu[m].temp==205) coconut |= 1;
			if (bu[m].temp==206) coconut |= 2;
			
			if (bu[m].temp==SK_DIVINITY)
			{
				gcdivinity = m;
			}
		}
		if (gcdivinity && !has_buff(cn, SK_DIVINITY))
		{
			n = make_new_buff(cn, SK_DIVINITY, 3495, 300, 18000, 0);
			bu[n].armor[1]         = bu[gcdivinity].armor[1];
			bu[n].weapon[1]        = bu[gcdivinity].weapon[1];
			bu[n].to_hit[1]        = bu[gcdivinity].to_hit[1];
			bu[n].to_parry[1]      = bu[gcdivinity].to_parry[1];
			bu[n].dmg_bonus[1]     = bu[gcdivinity].dmg_bonus[1];
			bu[n].dmg_reduction[1] = bu[gcdivinity].dmg_reduction[1];
			add_spell(cn, n);
		}
		else if (!gcdivinity && has_buff(cn, SK_DIVINITY))
		{
			remove_buff(cn, SK_DIVINITY);
		}
	}
	
	for (n = 0; n<MAXBUFFS; n++)
	{
		if (!ch[cn].spell[n]) continue;
		m = ch[cn].spell[n];
		if ((ch[cn].flags & CF_NOMAGIC) && !bu[m].data[4]) continue;
		
		bcount++;
		divCursed = divSlowed = divWeaken = 1;
		
		if ((bu[m].temp==SK_CURSE  && isCurse1 ) || (bu[m].temp==SK_CURSE2  && isCurse2 )) divCursed = 2;
		if ((bu[m].temp==SK_SLOW   && isSlow1  ) || (bu[m].temp==SK_SLOW2   && isSlow2  )) divSlowed = 3;
		if ((bu[m].temp==SK_WEAKEN && isWeaken1) || (bu[m].temp==SK_WEAKEN2 && isWeaken2)) divWeaken = 2;
		
		for (z = 0; z<5; z++)
		{
			attrib[z] += bu[m].attrib[z][1] / max(1, divCursed);
		}
		
		hp   += bu[m].hp[1];
		end  += bu[m].end[1];
		mana += bu[m].mana[1];
		
		heal_hp   += bu[m].hp[1];
		heal_end  += bu[m].end[1];
		heal_mana += bu[m].mana[1];

		for (z = 0; z<MAXSKILL; z++)
		{
			skill[z] += bu[m].skill[z][1];
		}

		armor  += bu[m].armor[1]  / max(1, divWeaken);
		weapon += bu[m].weapon[1] / max(1, divWeaken);
		
		if (!labcmd)
		{
			maxlight += bu[m].light[1];
			if (bu[m].light[1]>light)
			{
				light = bu[m].light[1];
			}
			else if (bu[m].light[1]<0)
			{
				sublight -= bu[m].light[1];
			}
		}
		
		// Meta values
		base_spd   += ((gearSpec & 4) && (bu[m].speed[1]/max(1, divSlowed))<0) ? (bu[m].speed[1]/max(1, divSlowed))/2 : bu[m].speed[1]/max(1, divSlowed);
		spd_move   += ((gearSpec & 4) && bu[m].move_speed[1]<0) ? bu[m].move_speed[1]/2 : bu[m].move_speed[1];
		spd_attack += ((gearSpec & 4) && bu[m].atk_speed[1]<0) ? bu[m].atk_speed[1]/2 : bu[m].atk_speed[1];
		spd_cast   += ((gearSpec & 4) && bu[m].cast_speed[1]<0) ? bu[m].cast_speed[1]/2 : bu[m].cast_speed[1];
		spell_mod  += bu[m].spell_mod[1];
		spell_apt  += bu[m].spell_apt[1];
		spell_cool += bu[m].cool_bonus[1] * (bu[m].temp==SK_ARIA?2:1);
		critical_c += bu[m].crit_chance[1];
		critical_m += bu[m].crit_multi[1];
		hit_rate   += bu[m].to_hit[1];
		parry_rate += bu[m].to_parry[1];
		damage_top += bu[m].top_damage[1];
		//
		dmg_bns     = dmg_bns * (100 + bu[m].dmg_bonus[1]/2)/100;
		dmg_rdc     = dmg_rdc * (100 - bu[m].dmg_reduction[1]/2)/100;
		//
		if (bu[m].temp==SK_HEAL)
		{
			sickStacks = 3;
			
			if (charmSpec & 1) sickStacks++;
			if (gearSpec & 512) sickStacks--;
			
			sickStacks = min(sickStacks, bu[m].data[1]+1);
		}
		if (bu[m].temp==SK_WARCRY2)
		{
			// Boots - Commander's Roots :: change 'stun' into raw speed reduction.
			if (gearSpec & 4)
			{
				base_spd -= 150;
				ch[cn].stunned = 0;
			}
			else
			{
				ch[cn].stunned = 1;
			}
		}
		if (bu[m].temp > 100 && bu[m].data[3]==BUF_IT_MANA)
		{
			tempCost = tempCost * 85 / 100;
		}
		if ((bu[m].temp==SK_OPPRESSED || bu[m].temp==SK_OPPRESSED2) && bu[m].power>0)
		{
			suppression = -(bu[m].power);
		}
		if (bu[m].temp==SK_OPPRESSION && bu[m].power>0 && ch[cn].temp == CT_PANDIUM)
		{
			suppression = bu[m].power;
		}
		
		if (bu[m].temp==666 && bu[m].power==666) // Stunned for cutscene
		{
			ch[cn].stunned = 1;
		}
		
		if (bu[m].temp==SK_TAUNT && IS_SANECHAR(co = bu[m].data[0]))
		{
			if (T_ARTM_SK(co, 9)) attaunt=1;
			ch[cn].taunted = co;
			if (ch[cn].temp==CT_PANDIUM) ch[cn].taunted = 0; // Special case for Pandium to ignore persistant aggro
			
			if (bu[m].active >= bu[m].duration-1) // Fresh taunt
			{
				ch[cn].goto_x = 0;
				if (WILL_FIGHTBACK(cn))
				{
					ch[cn].attack_cn = co;
				}
				else
				{
					ch[cn].misc_action = DR_TURN;
					ch[cn].misc_target1 = ch[co].x;
					ch[cn].misc_target2 = ch[co].y;
				}
			}
		}

		if (bu[m].hp[0]<0)   ch[cn].flags |= CF_NOHPREG;
		if (bu[m].end[0]<0)  ch[cn].flags |= CF_NOENDREG;
		if (bu[m].mana[0]<0) ch[cn].flags |= CF_NOMANAREG;

		if (bu[m].sprite_override)
		{
			ch[cn].sprite_override = bu[m].sprite_override;
		}

		// Lab 6 infrared potions
		if (bu[m].temp==635) infra |= 1;
		if (bu[m].temp==637) infra |= 2;
		if (bu[m].temp==639) infra |= 4;
		if (bu[m].temp==641) infra |= 8;
		
		// Coconut removes heatstroke
		if (bu[m].temp==205) coconut |= 1;
		if (bu[m].temp==206) coconut |= 2;
		
		if (bu[m].data[3]==BUF_IT_PIGS) pigsblood = 1;
	}
	
	// Tree flat passive bonuses
	if (T_SEYA_SK(cn, 1)) weapon += 2;
	if (T_SEYA_SK(cn, 2)) { for (z = 0; z<5; z++) attrib[z] += 2; }
	if (T_SEYA_SK(cn, 3)) armor += 2;
	if (T_SEYA_SK(cn, 4)) dmg_bns = dmg_bns * (1000 + bcount*5)/1000;
	if (T_SEYA_SK(cn, 8)) moreAtt+=4;
	if (T_SEYA_SK(cn,12)) dmg_rdc = dmg_rdc * (1000 - bcount*5)/1000;
	//
	if (T_ARTM_SK(cn, 1)) gethit += 5;
	if (T_ARTM_SK(cn, 2)) attrib[AT_STR] += 4;
	if (T_ARTM_SK(cn, 3)) armor += 3;
	if (T_ARTM_SK(cn, 8)) moreStr+=3;
	//
	if (T_SKAL_SK(cn, 1)) weapon += 3;
	if (T_SKAL_SK(cn, 2)) attrib[AT_AGL] += 4;
	if (T_SKAL_SK(cn, 3)) end += 20;
	if (T_SKAL_SK(cn, 8)) moreAgl+=3;
	//
	if (T_WARR_SK(cn, 1)) spd_attack += 5;
	if (T_WARR_SK(cn, 2)) { attrib[AT_AGL] += 3; attrib[AT_STR] += 3; }
	if (T_WARR_SK(cn, 3)) spell_apt += 5;
	if (T_WARR_SK(cn, 6)) spell_mod += 2;
	if (T_WARR_SK(cn, 8)) { moreAgl+=3; moreStr+=3; }
	//
	if (T_SORC_SK(cn, 1)) aoe++;
	if (T_SORC_SK(cn, 2)) { attrib[AT_WIL] += 3; attrib[AT_INT] += 3; }
	if (T_SORC_SK(cn, 3)) spd_move += 5;
	if (T_SORC_SK(cn, 6)) spell_mod += 2;
	if (T_SORC_SK(cn, 8)) { moreWil+=3; moreInt+=3; }
	//
	if (T_SUMM_SK(cn, 1)) spd_cast += 5;
	if (T_SUMM_SK(cn, 2)) attrib[AT_WIL] += 4;
	if (T_SUMM_SK(cn, 3)) hp += 20;
	if (T_SUMM_SK(cn, 8)) moreWil+=3;
	//
	if (T_ARHR_SK(cn, 1)) spell_cool += 5;
	if (T_ARHR_SK(cn, 2)) attrib[AT_INT] += 4;
	if (T_ARHR_SK(cn, 3)) mana += 20;
	if (T_ARHR_SK(cn, 8)) moreInt+=3;
	//
	if (T_BRAV_SK(cn, 1)) hit_rate += 3;
	if (T_BRAV_SK(cn, 2)) attrib[AT_BRV] += 4;
	if (T_BRAV_SK(cn, 3)) parry_rate += 3;
	if (T_BRAV_SK(cn, 8)) moreBrv+=3;
	//
	if (T_LYCA_SK(cn, 1)) damage_top += 5;
	if (T_LYCA_SK(cn, 2)) { hp += 10; end += 10; mana += 10; }
	if (T_LYCA_SK(cn, 3)) spell_mod += 2;
	if (T_LYCA_SK(cn,11)) spell_mod += 3;
	//
	// Corruption Effects
	n = st_skillcount(cn,  1); weapon                          += n;
	n = st_skillcount(cn,  2); for (z = 0; z<5; z++) attrib[z] += n;
	n = st_skillcount(cn,  3); armor                           += n;
	n = st_skillcount(cn,  4); dmg_bns = dmg_bns * (1000 + bcount*n*2)/1000;
	n = st_skillcount(cn, 12); dmg_rdc = dmg_rdc * (1000 - bcount*n*2)/1000;
	//
	n = st_skillcount(cn, 13); gethit         += n*2;
	n = st_skillcount(cn, 14); attrib[AT_STR] += n*2;
	n = st_skillcount(cn, 20); moreStr        += n*2;
	//
	n = st_skillcount(cn, 26); attrib[AT_AGL] += n*2;
	n = st_skillcount(cn, 27); end            += n*10;
	n = st_skillcount(cn, 32); moreAgl        += n*2;
	//
	n = st_skillcount(cn, 37); spd_attack     += n*2;
	n = st_skillcount(cn, 38); attrib[AT_AGL] += n; attrib[AT_STR] += n;
	n = st_skillcount(cn, 39); spell_apt      += n*2;
	n = st_skillcount(cn, 44); moreAgl        += n; moreStr        += n; 
	//
	n = st_skillcount(cn, 49); aoe            += n;
	n = st_skillcount(cn, 50); attrib[AT_WIL] += n; attrib[AT_INT] += n;
	n = st_skillcount(cn, 51); spd_move       += n*2;
	n = st_skillcount(cn, 56); moreWil        += n; moreInt        += n;
	//
	n = st_skillcount(cn, 61); spd_cast       += n*2;
	n = st_skillcount(cn, 62); attrib[AT_WIL] += n;
	n = st_skillcount(cn, 63); hp             += n*10;
	n = st_skillcount(cn, 68); moreWil        += n*2;
	//
	n = st_skillcount(cn, 73); spell_cool     += n*2;
	n = st_skillcount(cn, 74); attrib[AT_INT] += n;
	n = st_skillcount(cn, 75); mana           += n*10;
	n = st_skillcount(cn, 80); moreInt        += n*2;
	//
	n = st_skillcount(cn, 85); hit_rate       += n;
	n = st_skillcount(cn, 86); attrib[AT_BRV] += n;
	n = st_skillcount(cn, 87); parry_rate     += n;
	n = st_skillcount(cn, 92); moreBrv        += n*2;
	//
	n = st_skillcount(cn, 97); damage_top     += n*2;
	n = st_skillcount(cn, 98); hp += n*5; end += n*5; mana += n*5;
	n = st_skillcount(cn,107); spell_mod      += n;
	//
	//
	
	// Special check for heatstroke removal
	if (coconut==3)
	{
		for (n = 0; n<MAXBUFFS; n++)
		{
			if (!ch[cn].spell[n]) continue;
			m = ch[cn].spell[n];
			if (bu[m].temp==206)
			{
				do_char_log(cn, 0, "%s was removed.\n", bu[m].name);
				bu[m].used = USE_EMPTY;
				ch[cn].spell[n] = 0;
				do_update_char(cn);
			}
		}
	}
	
	if (charmSpec & 16) // Tarot Fool.R - Average up the attributes
	{
		int foolaverage = 0;
		for (z = 0; z<5; z++)
		{
			foolaverage += (int)B_AT(cn, z) + (int)ch[cn].attrib[z][1] + attrib[z];
		}
		for (z = 0; z<5; z++)
		{
			attrib[z]  = foolaverage/5;
			attrib[z] +=  attrib[z]/25*2; // 8%
		}
	}
	else
	{
		for (z = 0; z<5; z++)
		{
			attrib[z] = (int)B_AT(cn, z) + (int)ch[cn].attrib[z][1] + attrib[z];
		}
	}
	
	bits = get_rebirth_bits(cn);
	
	for (z = 0; z<5; z++)
	{
		ch[cn].limit_break[z][1]  = max(-127, min(127, suppression));
		if (T_BRAV_SK(cn, 8) && z==AT_BRV) ch[cn].limit_break[z][1] += 10;
		if (T_SUMM_SK(cn, 8) && z==AT_WIL) ch[cn].limit_break[z][1] += 10;
		if (T_ARHR_SK(cn, 8) && z==AT_INT) ch[cn].limit_break[z][1] += 10;
		if (T_SKAL_SK(cn, 8) && z==AT_AGL) ch[cn].limit_break[z][1] += 10;
		if (T_ARTM_SK(cn, 8) && z==AT_STR) ch[cn].limit_break[z][1] += 10;
		//
		if ((n=st_skillcount(cn, 92)) && z==AT_BRV) ch[cn].limit_break[z][1] += n;
		if ((n=st_skillcount(cn, 68)) && z==AT_WIL) ch[cn].limit_break[z][1] += n;
		if ((n=st_skillcount(cn, 80)) && z==AT_INT) ch[cn].limit_break[z][1] += n;
		if ((n=st_skillcount(cn, 32)) && z==AT_AGL) ch[cn].limit_break[z][1] += n;
		if ((n=st_skillcount(cn, 20)) && z==AT_STR) ch[cn].limit_break[z][1] += n;
		//
		ch[cn].limit_break[z][1] += min(5,max(0,(bits+5-z)/6));
		
		if (it[ch[cn].worn[WN_HEAD]].temp==2921) ch[cn].limit_break[z][1] += 3;
		
		// if (IS_BLOODY(cn)) ch[cn].limit_break[z][1] += 33;
		
		// Enchant: More attributes
		if (z==0 && moreBrv) attrib[z] = attrib[z]*(100+moreBrv)/100;
		if (z==1 && moreWil) attrib[z] = attrib[z]*(100+moreWil)/100;
		if (z==2 && moreInt) attrib[z] = attrib[z]*(100+moreInt)/100;
		if (z==3 && moreAgl) attrib[z] = attrib[z]*(100+moreAgl)/100;
		if (z==4 && moreStr) attrib[z] = attrib[z]*(100+moreStr)/100;
		if (moreAtt) attrib[z] = attrib[z]*(100+moreAtt)/100;
		
		set_attrib_score(cn, z, attrib[z]);
	}
	ch[cn].limit_break[5][1]  = max(-127, min(127, suppression));
	ch[cn].limit_break[5][1] += min(5,max(0,bits/6));
	if (it[ch[cn].worn[WN_HEAD]].temp==2921) ch[cn].limit_break[5][1] += 3;
	if (n = st_skillcount(cn, 8)) ch[cn].limit_break[5][1] += n*2;
	// if (IS_BLOODY(cn)) ch[cn].limit_break[5][1] += 33;
	
	// Weapon - Fist of the Heavens :: best attribute times 1.2
	if (gearSpec & 2)
	{
		int bestattribute[5] = {0};
		for (n = 0; n<5; n++)
		{
			bestattribute[n] = attrib[n];
			for (m = 0; m<5; m++)
			{
				if (bestattribute[n] < attrib[m])
					bestattribute[n] = 0;
			}
		}
		for (z = 0; z<5; z++)
		{
			if (bestattribute[z])
				set_attrib_score(cn, z, attrib[z]*6/5);
		}
	}
	
	// God Enchant :: worst attribute times 1.2
	if (enchantSpec & 32768)
	{
		int worstattribute[5] = {0};
		for (n = 0; n<5; n++)
		{
			worstattribute[n] = attrib[n];
			for (m = 0; m<5; m++)
			{
				if (worstattribute[n] > attrib[m])
					worstattribute[n] = 0;
			}
		}
		for (z = 0; z<5; z++)
		{
			if (worstattribute[z])
				set_attrib_score(cn, z, attrib[z]*6/5);
		}
	}
	
	// Book - Grande Grimoire :: best attribute times 1.1
	if (gearSpec & 4096)
	{
		int bestattribute[5] = {0};
		for (n = 0; n<5; n++)
		{
			bestattribute[n] = attrib[n];
			for (m = 0; m<5; m++)
			{
				if (bestattribute[n] < attrib[m])
					bestattribute[n] = 0;
			}
		}
		for (z = 0; z<5; z++)
		{
			if (bestattribute[z])
				set_attrib_score(cn, z, attrib[z]*11/10);
		}
	}
	
	// Endurance
	end = (int)ch[cn].end[0] + (int)ch[cn].end[1] + end;
	if (moreEnd) end = end*(5+moreEnd)/5;
	if (T_SKAL_SK(cn, 11)) end = end*120/100;
	if (T_LYCA_SK(cn,  8)) end = end*110/100;
	if (n = st_skillcount(cn, 35)) end = end*(100+n*5)/100;
	if (n = st_skillcount(cn,104)) end = end*(100+n*3)/100;
	if (end<10)
	{
		end = 10;
	}
	ch[cn].end[4] = end;
	if (end>999)
	{
		end = 999;
	}
	ch[cn].end[5] = end;
	if (ch[cn].a_end < ch[cn].end[5]*1000) ch[cn].a_end += heal_end;
	
	// Mana
	mana = (int)ch[cn].mana[0] + (int)ch[cn].mana[1] + mana;
	if (T_ARHR_SK(cn, 11)) mana = mana*120/100;
	if (T_LYCA_SK(cn,  8)) mana = mana*110/100;
	if (n = st_skillcount(cn, 83)) mana = mana*(100+n*5)/100;
	if (n = st_skillcount(cn,104)) mana = mana*(100+n*3)/100;
	if (mana<10)
	{
		mana = 10;
	}
	ch[cn].mana[4] = mana;
	if (mana>999 && IS_PLAYER(cn))
	{
		reduc_bonus = 1000000/max(1, mana*1000/ 999);
		tempCost = tempCost * reduc_bonus/1000;
		if (T_ARHR_SK(cn, 10)) hp += (mana-999)/4;
		if (n=st_skillcount(cn, 82)) hp += (mana-999)*n/10;
		mana = 999;
	}
	ch[cn].mana[5] = mana;
	if (ch[cn].a_mana < ch[cn].mana[5]*1000) ch[cn].a_mana += heal_mana;
	
	// Hitpoints - Placed after Mana due to overcap from mana (tree)
	hp = (int)ch[cn].hp[0] + (int)ch[cn].hp[1] + hp;
	if (pigsblood & 1) hp = hp*115/100; // Pigs blood drink
	if (T_SUMM_SK(cn, 11)) hp = hp*120/100;
	if (T_LYCA_SK(cn,  8)) hp = hp*110/100;
	if (n = st_skillcount(cn, 71)) hp = hp*(100+n*5)/100;
	if (n = st_skillcount(cn,104)) hp = hp*(100+n*3)/100;
	if (hp<10)
	{
		hp = 10;
	}
	ch[cn].hp[4] = hp;
	if (hp>999 && IS_PLAYER(cn))
	{
		reduc_bonus = 1000000/max(1, hp*1000/ 999);
		dmg_rdc = dmg_rdc * reduc_bonus/1000;
		hp = 999;
	}
	ch[cn].hp[5] = hp;
	if (ch[cn].a_hp < ch[cn].hp[5]*1000) ch[cn].a_hp += heal_hp;
	
	if (ch[cn].flags & (CF_PLAYER))
	{
		if ((infra==15 || IS_LYCANTH(cn)) && !(ch[cn].flags & CF_INFRARED))
		{
			ch[cn].flags |= CF_INFRARED;
			if (infra==15) ch[cn].flags |= CF_IGN_SB;
			do_char_log(cn, 0, "You can see in the dark!\n");
			if (IS_PLAYER_GC(co = ch[cn].data[PCD_COMPANION]))
			{
				ch[co].flags |= CF_INFRARED;
				if (infra==15) ch[co].flags |= CF_IGN_SB;
			}
			if (IS_PLAYER_SC(co = ch[cn].data[PCD_SHADOWCOPY]))
			{
				ch[co].flags |= CF_INFRARED;
				if (infra==15) ch[co].flags |= CF_IGN_SB;
			}
		}
		else if (infra==15 && !(ch[cn].flags & CF_IGN_SB))
		{
			ch[cn].flags |= CF_IGN_SB;
			do_char_log(cn, 0, "Your darkvision improves!\n");
		}
		if (infra!=15 && !IS_LYCANTH(cn) && (ch[cn].flags & CF_INFRARED) && !(ch[cn].flags & CF_GOD))
		{
			ch[cn].flags &= ~CF_INFRARED;
			ch[cn].flags &= ~CF_IGN_SB;
			do_char_log(cn, 0, "You can no longer see in the dark!\n");
			if (IS_PLAYER_GC(co = ch[cn].data[PCD_COMPANION]))
			{
				ch[co].flags |= CF_INFRARED;
				ch[co].flags |= CF_IGN_SB;
			}
			if (IS_PLAYER_SC(co = ch[cn].data[PCD_SHADOWCOPY]))
			{
				ch[co].flags |= CF_INFRARED;
				ch[co].flags |= CF_IGN_SB;
			}
		}
		else if (infra!=15 && (ch[cn].flags & CF_IGN_SB) && !(ch[cn].flags & CF_GOD))
		{
			ch[cn].flags &= ~CF_IGN_SB;
			do_char_log(cn, 0, "Your darkvision returns to normal.\n");
		}
	}
	
	for (z = 0; z<MAXSKILL; z++)
	{
		if (IS_COMP_TEMP(cn) && IS_SANECHAR(co = ch[cn].data[CHD_MASTER]) && ch[cn].data[1]==4) { cz=cn; cn=co; }
		
		skill[z] = (int)B_SK(cn, z) + (int)ch[cn].skill[z][1] + skill[z];
		
		     if ((z==SK_SWORD) && (gearSpec & 65536))
			skill[z] += ((int)M_AT(cn, AT_BRV)+(int)M_AT(cn, AT_STR)+(int)M_AT(cn, AT_STR))/5;
		else if ((z==SK_HAND ) && (gearSpec & 16384))
			skill[z] += ((int)M_AT(cn, AT_BRV)+(int)M_AT(cn, AT_BRV)+(int)M_AT(cn, AT_BRV))/5;
		else if ((z==SK_AXE  ) && (gearSpec & 32768))
			skill[z] += ((int)M_AT(cn, AT_BRV)+(int)M_AT(cn, AT_AGL)+(int)M_AT(cn, AT_AGL))/5;
		
		else if ((z==0||z==2||z==3||z==4||z==5||z==6||z==16||z==33||z==37||z==38||z==40||z==41||z==48||z==49) && T_SKAL_SK(cn, 9))
			skill[z] += ((int)((M_AT(cn, AT_BRV)+M_AT(cn, AT_STR))/2)+(int)M_AT(cn, AT_AGL)+(int)M_AT(cn, AT_AGL))/5;
		else if ((z==0||z==2||z==3||z==4||z==5||z==6) && T_BRAV_SK(cn, 9))
			skill[z] += ((int)((M_AT(cn, AT_AGL)+M_AT(cn, AT_STR))/2)+(int)M_AT(cn, AT_BRV)+(int)M_AT(cn, AT_BRV))/5;
		else if (z==11||z==17||z==18||z==19||z==20||z==21||z==24||z==25||z==26||z==27||z==42||z==43||z==46||z==47)
		{
			n = st_skillcount(cn, 67);
			m = st_skillcount(cn, 81);
			if (T_SUMM_SK(cn, 7))
				skill[z] += ((int)((M_AT(cn, AT_BRV)+M_AT(cn, AT_INT))/2)+(int)M_AT(cn, AT_WIL)+(int)M_AT(cn, AT_WIL)
					+(int)(n*M_AT(cn, AT_WIL)/10)+(int)(m*M_AT(cn, AT_INT)/10))/5;
			else if (T_ARHR_SK(cn, 9))
				skill[z] += ((int)((M_AT(cn, AT_BRV)+M_AT(cn, AT_WIL))/2)+(int)M_AT(cn, AT_INT)+(int)M_AT(cn, AT_INT)
					+(int)(n*M_AT(cn, AT_WIL)/10)+(int)(m*M_AT(cn, AT_INT)/10))/5;
			else if ((z==11||z==17||z==18||z==21||z==47) && IS_LYCANTH(cn))
				skill[z] += ((int)M_AT(cn, AT_BRV)+(int)M_AT(cn, AT_WIL)+(int)M_AT(cn, AT_AGL)
					+(int)(n*M_AT(cn, AT_WIL)/10)+(int)(m*M_AT(cn, AT_INT)/10))/5;
			else
				skill[z] += ((int)M_AT(cn,skilltab[z].attrib[0])+(int)M_AT(cn,skilltab[z].attrib[1])+(int)M_AT(cn, skilltab[z].attrib[2])
					+(int)(n*M_AT(cn, AT_WIL)/10)+(int)(m*M_AT(cn, AT_INT)/10))/5;
		}
		else
			skill[z] += ((int)M_AT(cn,skilltab[z].attrib[0])+(int)M_AT(cn,skilltab[z].attrib[1])+(int)M_AT(cn,skilltab[z].attrib[2]))/5;
		
		if (z==SK_IMMUN && T_ARTM_SK(cn, 10))
		{
			skill[z] += skill[SK_RESIST]/5;
		}
		if (z==SK_IMMUN && (n=st_skillcount(cn, 22)))
		{
			skill[z] += skill[SK_RESIST]*n/25;
		}
		
		if ((charmSpec & 8) && (z==SK_RESIST||z==SK_IMMUN))
		{
			skill[z] = skill[z]*4/5;
		}
		
		if (cz) cn=cz;
		
		set_skill_score(cn, z, skill[z]);
		
		if (z==0 && bbelt)
		{
			weapon     += min(AT_CAP, skill[z])/2;
			tempWeapon += min(AT_CAP, skill[z])/2;
		}
		
		if (z==0 && wbelt)
		{
			weapon     += min(AT_CAP, skill[z])/3;
			tempWeapon += min(AT_CAP, skill[z])/3;
		}
	}
	
	if (IS_COMP_TEMP(cn) && IS_SANECHAR(co = ch[cn].data[CHD_MASTER]) && ch[cn].data[1]==4) { cz=cn; cn=co; }
	
	// Gear Mastery
	if (B_SK(cn, SK_GEARMAST))
	{
		if (tempWeapon)
		{
			if (IS_PLAYER(cn))
			{
				if (IS_SKALD(cn))							weapon += min(tempWeapon*2, M_SK(cn, SK_GEARMAST)/ 3);
				else if (IS_ARCHTEMPLAR(cn))				weapon += min(tempWeapon*2, M_SK(cn, SK_GEARMAST)/ 4);
				else if (IS_ANY_TEMP(cn) || IS_BRAVER(cn))	weapon += min(tempWeapon*2, M_SK(cn, SK_GEARMAST)/ 5);
				else										weapon += min(tempWeapon*2, M_SK(cn, SK_GEARMAST)/10);
			}	else										weapon += min(tempWeapon,   M_SK(cn, SK_GEARMAST)/ 6);
		}
		if (tempArmor)
		{
			if (IS_PLAYER(cn))
			{
				if (IS_ARCHTEMPLAR(cn))						armor += min(tempArmor*2, M_SK(cn, SK_GEARMAST)/ 3);
				else if (IS_SKALD(cn))						armor += min(tempArmor*2, M_SK(cn, SK_GEARMAST)/ 4);
				else if (IS_ANY_TEMP(cn) || IS_BRAVER(cn))	armor += min(tempArmor*2, M_SK(cn, SK_GEARMAST)/ 5);
				else										armor += min(tempArmor*2, M_SK(cn, SK_GEARMAST)/10);
			}	else										armor += min(tempArmor,   M_SK(cn, SK_GEARMAST)/ 6);
		}
	}
	
	if (cz) cn = cz;
	
	// Tactics
	if (B_SK(cn, SK_TACTICS))
	{
		if (IS_PLAYER(cn) && IS_ANY_HARA(cn))
			z = M_SK(cn, SK_TACTICS)/ 6;
		else if (IS_PLAYER(cn) && (IS_SORCERER(cn) || IS_LYCANTH(cn)))
			z = M_SK(cn, SK_TACTICS)/ 8;
		else
			z = M_SK(cn, SK_TACTICS)/10;
		
		// IT_CH_MOON_R :: 1% increased effect of tactics per 50 uncapped mana
		if (charmSpec & 256)
		{
			z = z + max(0, z * ch[cn].mana[4] / 5000);
			if (ch[cn].a_mana <= ch[cn].mana[5]*400)
			{
				n = max(1, ch[cn].mana[5]*400);
				mana = max(0, ch[cn].mana[5]*400 + 500 - ch[cn].a_mana);
				
				hit_rate   += max(0, min(z, z * mana / n));
				parry_rate += max(0, min(z, z * mana / n));
			}
		}
		else if (ch[cn].a_mana >= ch[cn].mana[5]*600)
		{
			n = max(1, ch[cn].mana[5]*1000 - ch[cn].mana[5]*600);
			mana = max(0, ch[cn].a_mana - ch[cn].mana[5]*600);
			
			hit_rate   += max(0, min(z, z * mana / n));
			parry_rate += max(0, min(z, z * mana / n));
		}
	}
	
	// Finesse
	if (B_SK(cn, SK_FINESSE) && ch[cn].a_hp >= ch[cn].hp[5]*600)
	{
		if (IS_PLAYER(cn) && IS_BRAVER(cn))
			z = M_SK(cn, SK_FINESSE)*3;
		else
			z = M_SK(cn, SK_FINESSE)*2;
		
		if (T_BRAV_SK(cn, 7))        z = z + (z * M_AT(cn, AT_BRV)/2000);
		if (n=st_skillcount(cn, 91)) z = z + (z * M_AT(cn, AT_BRV)*n/5000);
		
		n = max(1, ch[cn].hp[5]*1000 - ch[cn].hp[5]*600);
		hp = ch[cn].a_hp - ch[cn].hp[5]*600;
		
		dmg_bns = dmg_bns * (5000 + min(z, z * hp / n))/5000;
	}
	
	if (IS_COMP_TEMP(cn) && IS_SANECHAR(co = ch[cn].data[CHD_MASTER]) && ch[cn].data[1]==4) { cz=cn; cn=co; }
	
	// Ferocity
	if (IS_LYCANTH(cn))
	{	// PVT = 1 : 8		// WARLORD = 25 : 200
		z = M_SK(cn, SK_FEROC); // M_SK(cn, SK_FEROC)*(M_SK(cn, SK_FEROC)/8)*136/5000+8;
		weapon += (z/2 + z*empty/10)/4; //  1 = (  4+  8)/4 == btwn + 1 and +  3
		armor  += (z/2 + z*empty/10)/4; // 25 = (100+200)/4 == btwn +25 and + 75
		if (T_LYCA_SK(cn,  4)) critical_b += empty;
		if (T_LYCA_SK(cn, 12)) spell_mod += empty;
	}
	if (n = st_skillcount(cn,100)) critical_b += emptyring*n;
	if (n = st_skillcount(cn,108)) spell_mod  += emptyring*n;
	
	if (cz) cn = cz;
	
	// Maxlight takes your cumulative total of all light sources, minus the highest.
	// Light is then the highest single light value affecting you, plus half of whatever maxlight is beyond that.
	maxlight -= light;
	light += maxlight/2;
	light -= sublight;
	
	if (has_buff(cn, 215))
	{
		light = light/10;
	}
	if (enchantSpec & 1024)
	{
		light = 0;
	}
	if (light<0)
	{
		light = 0;
	}
	if (light>250)
	{
		light = 250;
	}
	ch[cn].light = light;
	
	// ******************************** Meta mods! ******************************** //
	// In addition to gear and spells above, these mods may have extra bonuses applied by skills or race.
	
	// "attrib" is used to store the base value, mainly for spell aptitude
	// "attrib_ex" is used to store the mod value, used by everything else.
	for (n=0;n<5;n++)
	{
		attrib[n]    = B_AT(cn, n);
		attrib_ex[n] = M_AT(cn, n);
	}
	
	// Avaritia
	if (gearSpec & 256)
	{
		ava_crit = ava_mult = attrib_ex[0];
		
		for (n=1;n<5;n++)
		{
			ava_crit = min(ava_crit, attrib_ex[n]);
			ava_mult = max(ava_mult, attrib_ex[n]);
		}
		
		ava_crit*=2;
		ava_mult/=2;
	}
	
	// Book - Traveler's Guide :: Higher effects of Braveness and Agility
	if (gearSpec & 1) 
	{
		if (M_AT(cn, AT_AGL) > M_AT(cn, AT_BRV))
		{
			attrib[AT_BRV]    = B_AT(cn, AT_AGL);
			attrib_ex[AT_BRV] = M_AT(cn, AT_AGL);
		}
		else
		{
			attrib[AT_AGL]    = B_AT(cn, AT_BRV);
			attrib_ex[AT_AGL] = M_AT(cn, AT_BRV);
		}
	}
	
	// Tarot - Magician :: Higher effects of Strength and Intuition
	if (charmSpec & 4)
	{
		if (M_AT(cn, AT_STR) > M_AT(cn, AT_INT))
		{
			attrib[AT_INT]    = B_AT(cn, AT_STR);
			attrib_ex[AT_INT] = M_AT(cn, AT_STR);
		}
		else
		{
			attrib[AT_STR]    = B_AT(cn, AT_INT);
			attrib_ex[AT_STR] = M_AT(cn, AT_INT);
		}
	}
	
	// Tree - Attribute mods
	if (T_SEYA_SK(cn, 7)) { m=0; for (z=0; z<5; z++) { m+=attrib_ex[z]; } hit_rate+=m/100; parry_rate+=m/100; }
	if (n = st_skillcount(cn,  7)) { m = (ch[cn].hp[5]*1000-ch[cn].a_hp)/1000; hit_rate+=m*n/100; parry_rate+=m*n/100; }
	
	/*
		ch[].mana_cost
	*/
	
	// Economize
	if (B_SK(cn, SK_ECONOM) && !(charmSpec & 128))
	{
		if (gearSpec & 1024) // Book: Great Prodigy
		{
			t = tempCost * M_SK(cn, SK_ECONOM) / 333;
		}
		else
		{
			t = tempCost * M_SK(cn, SK_ECONOM) / 444;
		}
		tempCost -= t;
	}
	
	if (tempCost > 20000) // Maximum 200% mana cost
	{
		tempCost = 20000;
	}
	if (tempCost < 0)
	{
		tempCost = 0;
	}
	ch[cn].mana_cost = tempCost;
	
	/*
		ch[].speed value
	*/
	
	// Weapon - Kelp Trident :: +30 speed while underwater
	if ((gearSpec & 16) && (map[ch[cn].x + ch[cn].y * MAPX].flags & MF_UWATER))
	{
		base_spd += 30;
	}
	
	base_spd = 120 + base_spd + (attrib_ex[AT_AGL] + attrib_ex[AT_STR]) / 8 + ch[cn].speed_mod;
	
	// Additional bonus via speed mode :: Slow, Normal, Fast
	if (ch[cn].mode==0) base_spd += 15;	// old: 14 + 2 = 16/36
	if (ch[cn].mode==1) base_spd += 30;	// old: 14 + 4 = 18/36
	if (ch[cn].mode==2) base_spd += 45;	// old: 14 + 6 = 20/36
	
	// Clamp base_speed between 1 and SPEED_CAP (300)
	if (base_spd > SPEED_CAP) 
	{
		base_spd = SPEED_CAP;
	}
	if (base_spd < 1) 
	{
		base_spd = 1;
	}	
	ch[cn].speed = SPEED_CAP - base_spd;
	// Table array is between 0 and 299 and stored in reverse order.
	// So we take 300, minus our bonus speed values above.
	
	/*
		ch[].move_speed value
	*/
	
	spd_move += 20;
	
	if (IS_GLOB_MAYHEM && !IS_PLAYER(cn))
		spd_move += 40;
	
	if (IS_BLOODY(cn) && ch[cn].data[42] == 1100)
		spd_move += 100;
	
	// Enchant: Move speed can't go below 150
	if (enchantSpec & 32)
	{
		if (base_spd + spd_move < 150)
			spd_move = 150 - (base_spd + spd_move);
	}
	// Enchant: 20% more Move speed
	if (enchantSpec & 64)
	{
		spd_move = ((base_spd + spd_move) * 6/5) - base_spd;
	}
	// Enchant: 2x Move Speed underwater
	if ((enchantSpec & 128) && (map[ch[cn].x + ch[cn].y * MAPX].flags & MF_UWATER))
	{
		spd_move = ((base_spd + spd_move) * 2) - base_spd;
	}
	
	// Tree - sorc
	if (T_SORC_SK(cn, 11))         spd_move = ((base_spd + spd_move) * 120/100) - base_spd;
	if (n = st_skillcount(cn, 59)) spd_move = ((base_spd + spd_move) * (100+n*3)/100) - base_spd;
	
	if (spd_move > 120)
	{
		spd_move = 120;
	}
	if (spd_move < -120)
	{
		spd_move = -120;
	}
	ch[cn].move_speed = spd_move;
	
	
	/*
		ch[].cast_speed value
		ch[].atk_speed value
	*/
	
	spd_cast   += attrib_ex[AT_WIL]/4;
	spd_attack += attrib_ex[AT_AGL]/4;
	
	// Tree - summ
	if (T_SUMM_SK(cn,  4)) spd_attack += spd_cast;
	
	// Tarot - Strength - 15% less cast speed
	if (charmSpec & 32)
	{
		spd_cast = (base_spd+spd_cast) * 85/100 - base_spd;
		if (T_SUMM_SK(cn,  4)) spd_attack = ((base_spd + spd_attack) * 85/100) - base_spd;
	}
	
	if (T_SUMM_SK(cn,  5)) 
	{
		spd_cast = ((base_spd + spd_cast) * 110/100) - spd_cast;
		if (T_SUMM_SK(cn,  4)) spd_attack = ((base_spd + spd_attack) * 110/100) - base_spd;
	}
	if (n = st_skillcount(cn, 65))
	{
		spd_cast = ((base_spd + spd_cast) * (100+n*3)/100) - spd_cast;
		if (T_SUMM_SK(cn,  4)) spd_attack = ((base_spd + spd_attack) * (100+n*3)/100) - base_spd;
	}
	
	// Clamp spd_cast between 0 and SPEED_CAP (300)
	if (spd_cast > 120)
	{
		spd_cast = 120;
	}
	if (spd_cast < -120)
	{
		spd_cast = -120;
	}
	ch[cn].cast_speed = spd_cast;
	
	// Tarot - Strength - 15% less attack speed
	if (charmSpec & 32)
	{
		spd_attack = ((base_spd + spd_attack) * 85/100) - base_spd;
	}
	
	// Tree - warr
	if (T_WARR_SK(cn,  5))         spd_attack = ((base_spd + spd_attack) * 110/100) - base_spd;
	if (n = st_skillcount(cn, 41)) spd_attack = ((base_spd + spd_attack) * (100+n*3)/100) - base_spd;
	
	// Clamp spd_attack between 0 and SPEED_CAP (300)
	if (spd_attack > 120)
	{
		spd_attack = 120;
	}
	if (spd_attack < -120)
	{
		spd_attack = -120;
	}
	ch[cn].atk_speed = spd_attack;
	
	
	/*
		ch[].spell_mod value
		
		100 is equal to 1.00 (or 1x power)
		Upper boundary is 3.00 (3x power) just because any more than that would be ridiculous.
	*/
	
	spell_mod += spell_race_mod(100, cn);
	
	if (enchantSpec & 8192)
	{
		spell_mod += light/25;
	}
	
	// Clamp spell_mod between 0 and 300
	if (spell_mod > 300)
	{
		spell_mod = 300;
	}
	if (spell_mod < 0)
	{
		spell_mod = 0;
	}
	ch[cn].spell_mod = spell_mod;
	
	
	/*
		ch[].spell_apt value
	*/
	
	spell_apt += (attrib[AT_WIL] + attrib[AT_INT]) * spell_race_mod(100, cn) / 100;
	
	if (gearSpec & 8192)           spell_apt = spell_apt/10;
	
	// Tree - warr
	if (T_WARR_SK(cn, 11))         spell_apt = spell_apt*120/100;
	if (n = st_skillcount(cn, 47)) spell_apt = spell_apt*(100+n*5)/100;
	//if (T_WARR_SK(cn, 12))         dmg_rdc = max(1000, dmg_rdc * (500 - spell_apt)/500);
	
	// Clamp spell_apt between 0 and 999
	if (spell_apt > 999)
	{
		spell_apt = 999;
	}
	if (spell_apt < 1)
	{
		spell_apt = 1;
	}
	ch[cn].spell_apt = spell_apt;
	
	
	/*
		ch[].cool_bonus value
		
		dur * 100 / var = skill exhaust
	*/
	
	spell_cool += attrib_ex[AT_INT]/6;
	
	// Tarot - Strength - 15% less cooldown
	if (charmSpec & 32)
	{
		spell_cool = spell_cool*85/100;
	}
	
	// Tarot - Magician.R : Economize instead improves cooldown rate
	if (B_SK(cn, SK_ECONOM) && (charmSpec & 128))
	{
		if (gearSpec & 1024) // Book: Great Prodigy
		{
			t = spell_cool * M_SK(cn, SK_ECONOM) / 333;
		}
		else
		{
			t = spell_cool * M_SK(cn, SK_ECONOM) / 444;
		}
		spell_cool += t;
	}
	
	// Tree - arhr
	if (T_ARHR_SK(cn,  5))         spell_cool = spell_cool*105/100;
	if (n = st_skillcount(cn, 77)) spell_cool = spell_cool*(100+n*2)/100;
	
	if ((n = has_buff(cn, SK_POISON)) && bu[n].data[8]==10) spell_cool = spell_cool*9/10;
	if ((n = has_buff(cn, SK_VENOM))  && bu[n].data[8]==10) spell_cool = spell_cool*9/10;
	
	// Clamp spell_cool between 0 and 900
	if (spell_cool > 900)
	{
		spell_cool = 900;
	}
	if (spell_cool < -75)
	{
		spell_cool = -75;
	}
	ch[cn].cool_bonus = 100 + spell_cool;
	
	
	/*
		ch[].dmg_bonus
		ch[].dmg_reduction
	*/
	
	m = ch[cn].worn[WN_RHAND];
	if ((IS_WPAXE(m) || IS_WPGAXE(m)) && (n = st_skillcount(cn, 33)))
	{
		dmg_bns = dmg_bns * (100 + n*2)/100;
	}
	if ((IS_WPSWORD(m) || IS_WPTWOHAND(m)) && (n = st_skillcount(cn, 93)))
	{
		dmg_rdc = dmg_rdc * (100 - n*2)/100;
	}
	
	// Monster bonus
	//if ((ch[cn].kindred & KIN_MONSTER) || IS_LYCANTH(cn))
	//	dmg_bns = dmg_bns * (100 + (getrank(cn)-4)/2)/100;
	
	// Enchant - [50] more damage dealt with hits; 2% per piece
	if (moreDmg)
		dmg_bns = dmg_bns * (100 + moreDmg*2)/100;
	
	// Sanity checks
	if (dmg_bns > 30000)	// Maximum 300% damage output
	{
		dmg_bns = 30000;
	}
	if (dmg_bns < 1000)	// Always deal at least 10% of damage
	{
		dmg_bns = 1000;
	}
	ch[cn].dmg_bonus = dmg_bns;
	
	// Safeguard
	if (B_SK(cn, SK_SAFEGRD))
	{
		dmg_rdc = dmg_rdc * (600 - M_SK(cn,SK_SAFEGRD))/600;
	}
	
	// Enchant - [51] less damage taken from hits; 2% per piece
	if (moreGrd)
		dmg_rdc = dmg_rdc * (100 - moreGrd*2)/100;
	
	// Monster bonus
	//if ((ch[cn].kindred & KIN_MONSTER) || IS_LYCANTH(cn))
	//	dmg_rdc = dmg_rdc * (100 - (getrank(cn)-4)/2)/100;
	
	if (dmg_rdc > 30000)	// Maximum 300% damage taken
	{
		dmg_rdc = 30000;
	}
	if (dmg_rdc < 1000)	// Always take at least 10% of damage
	{
		dmg_rdc = 1000;
	}
	ch[cn].dmg_reduction = dmg_rdc;
	
	
	/*
		ch[].crit_chance value
		
		Base crit chance is currently determined by what kind of weapon is equipped 
		It is further increased by "Precision" skill score.
		
		After this point, crit base is increased by a factor of 100,
		so that precision can have a more consistant effect.
	*/
	
	critical_c += attrib_ex[AT_BRV]*2;
	
	// Monster-related crit adjustments
	if (!IS_PLAYER(cn))
	{
		if (ch[cn].flags & CF_EXTRACRIT)
			critical_b = 4;
		else
			critical_b = 1;
	}
	
	critical_b *= 100;
	
	// Grant extra crit chance by crit bonus
	critical_b += critical_b * (critical_c+ava_crit)/100;
	
	if (B_SK(cn, SK_PRECISION))
	{
		n=100+st_skillcount(cn, 88)*5;
		critical_b += (critical_b * skill[SK_PRECISION] * (T_BRAV_SK(cn,4)?120:100)/100 * n/100)/PREC_CAP;
	}
	
	// Tarot - Wheel of Fortune :: Less crit chance, more crit multi
	if (charmSpec & 64)
	{
		critical_b = critical_b * 2/3;
	}
	
	// Clamp critical_c between 0 and 10000
	if (critical_b > 10000)
	{
		critical_b = 10000;
	}
	if (critical_b < 0)
	{
		critical_b = 0;
	}
	ch[cn].crit_chance = critical_b;
	
	
	/*
		ch[].crit_multi value
		
		Base crit multiplier is 1.25x
	*/
	
	critical_m += 25 + ava_mult;
	
	
	// Weapon - Gildshine :: Economize is granted as crit multi
	if (gearSpec & 8) 
	{
		critical_m += skill[SK_ECONOM];
	}
	
	// Tree - skal
	if (T_SKAL_SK(cn,  7))         critical_m += attrib_ex[AT_AGL]*3/10;
	if (n = st_skillcount(cn, 31)) critical_m += attrib_ex[AT_AGL]*n/10;
	
	// Tarot - Wheel of Fortune :: Less crit chance, more crit multi
	if (charmSpec & 64)
	{
		critical_m = (critical_m + 100) * 4/3 - 100;
	}
	
	// Clamp critical_m between 0 and 800
	if (critical_m > 800)
	{
		critical_m = 800;
	}
	if (critical_m < 0)
	{
		critical_m = 0;
	}
	ch[cn].crit_multi = 100 + critical_m;
	
	
	/*
		ch[].to_hit value & ch[].to_parry value
		
		Determined by base weapon skill, plus dual wield score if dual wielding.
		Determined by base weapon skill, plus shield score if using a shield.
	*/
	
	hit_rate += get_fight_skill(cn, skill);
	hit_rate += get_offhand_skill(cn, skill, 1);
	
	parry_rate += get_fight_skill(cn, skill);
	parry_rate += get_offhand_skill(cn, skill, 0);
	
	// Tarot - Lovers.R : Swaps hit/parry
	if (charmSpec & 1024)
	{
		loverSplit = (hit_rate + parry_rate) / 2;
		hit_rate   = loverSplit;
		parry_rate = loverSplit;
	}
	
	// Tarot - Strength.R : More WV, less hit
	if (charmSpec & 4096)
		hit_rate = hit_rate * 4/5;
	
	// Tree - brav
	if (T_BRAV_SK(cn,  5))         hit_rate   = hit_rate  *104/100;
	if (T_BRAV_SK(cn, 11))         parry_rate = parry_rate*104/100;
	if (n = st_skillcount(cn, 89)) hit_rate   = hit_rate  *(100+n)/100;
	if (n = st_skillcount(cn, 95)) parry_rate = parry_rate*(100+n)/100;
	
	// Spear power-up (summ tree)
	if ((m = ch[cn].worn[WN_RHAND]) && IS_WPSPEAR(m))
	{
		if (T_SUMM_SK(cn,  6))
		{
			hit_rate   = hit_rate  *6/5;
			parry_rate = parry_rate*6/5;
		}
	}
	
	// GC parent override (seya tree)
	if (IS_PLAYER_COMP(cn) && (m = CN_OWNER(cn)))
	{
		if (T_SEYA_SK(m, 9))
		{
			hit_rate   = ch[m].to_hit;
			parry_rate = ch[m].to_parry;
		}
		
		if (n = st_skillcount(m,  9))
		{
			hit_rate   += hit_rate   * (n*2) / 100;
			parry_rate += parry_rate * (n*2) / 100;
		}
		
		if (is_atpandium(cn) && is_atpandium(m))
		{
			hit_rate   = hit_rate  *11/10;
			parry_rate = parry_rate*11/10;
		}
	}
	
	if (attaunt)
		hit_rate = hit_rate * 19/20;
	
	if (enchantSpec & 2048) parry_rate += hit_rate/30;
	if (enchantSpec & 4096) hit_rate += parry_rate/30;
	
	if (gearSpec & 2048) // Improved Bien Estoc
	{
		if (hit_rate > parry_rate)	parry_rate = hit_rate;
		else						hit_rate = parry_rate;
	}
	
	// Clamp hit_rate between 0 and 999
	if (hit_rate > 999)
	{
		hit_rate = 999;
	}
	if (hit_rate < 0)
	{
		hit_rate = 0;
	}
	// Clamp parry_rate between 0 and 999
	if (parry_rate > 999)
	{
		parry_rate = 999;
	}
	if (parry_rate < 0)
	{
		parry_rate = 0;
	}
	
	ch[cn].to_hit   = hit_rate;
	ch[cn].to_parry = parry_rate;
	
	
	/*
		Weapon and Armor finalized
		
		This is moved down here due to fancy unique item functions
	*/
	
	// Weapon - Excalibur :: Additional WV from 20% of total attack speed
	if (gearSpec & 128)
	{
		weapon += (base_spd + spd_attack)/5;
	}
	
	// Weapon - White Odachi :: Additional AV by spellmod over 100
	if ((gearSpec & 32) && spell_mod > 100)
	{
		armor  += (spell_mod-100);
	}
	
	// Weapon - Black Tachi :: Additional WV by spellmod over 100
	if ((gearSpec & 64) && spell_mod > 100)
	{
		weapon += (spell_mod-100);
	}
	
	if (n=st_skillcount(cn, 15)) { m=0; for (z=0; z<5; z++) { m+=attrib_ex[z]; } armor +=(m*n)/200; }
	if (n=st_skillcount(cn, 25)) { m=0; for (z=0; z<5; z++) { m+=attrib_ex[z]; } weapon+=(m*n)/200; }
	
	// Tarot - Lovers
	if (charmSpec & 2)
	{
		loverSplit = (weapon + armor) / 2;
		weapon     = loverSplit;
		armor      = loverSplit;
	}
	// Tarot - Hermit
	if (charmSpec & 8)
		armor = armor * 23/20;
	// Tarot - Strength.R : More WV, less hit
	if (charmSpec & 4096)
		weapon = weapon * 6/5;
	// Tarot - Hanged.R : 12% less WV, 24% more Top Damage
	if (charmSpec & 8192)
		weapon = weapon * 22/25;
	// Tarot - Temperance.R : 6.25% more WV per stack of healing sickness on you
	if (charmSpec & 1)
		weapon += weapon * sickStacks/16;
	// Chest Armor Enchantment
	if (enchantSpec & 1) 
		armor = armor * 27/25;
	// Tarot - Reverse Heirophant : GC gets more WV/AV
	if (IS_COMPANION(cn) && !IS_SHADOW(cn) && IS_SANECHAR(co = ch[cn].data[CHD_MASTER]) && get_tarot(co, IT_CH_HEIROP_R))
	{
		armor  = armor  * 112/100;
		weapon = weapon * 112/100;
	}
	// Tree - % more wv/av
	if (T_SEYA_SK(cn,  5))       weapon = weapon * 106 / 100;
	if (T_SKAL_SK(cn,  5))       weapon = weapon * 109 / 100;
	if (T_SEYA_SK(cn, 11))       armor  = armor  * 106 / 100;
	if (T_ARTM_SK(cn, 11))       armor  = armor  * 109 / 100;
	//
	if (n=st_skillcount(cn, 23)) armor  = armor  * (100+n*3) / 100;
	if (n=st_skillcount(cn, 29)) weapon = weapon * (100+n*3) / 100;
	//
	if (n=st_skillcount(cn,  5)) tmpw = armor  * (n*2) / 100;
	if (n=st_skillcount(cn, 11)) tmpa = weapon * (n*2) / 100;
	//
	
	if (ch[cn].temp == CT_PIRATELORD && IS_SANEITEM(m = map[1331331].it) && !(it[in].active))
		armor += 100;
	
	armor += tmpa;
	if (armor<0)
	{
		armor = 0;
	}
	if (armor>300)
	{
		armor = 300;
	}
	ch[cn].armor = armor;
	
	// Enchant: AV as extra Resistance
	if (enchantSpec &  2) 
		set_skill_score(cn, SK_RESIST, skill[SK_RESIST] + armor/10);
	// Enchant: AV as extra Immunity
	if (enchantSpec & 16) 
		set_skill_score(cn, SK_IMMUN, skill[SK_IMMUN] + armor/10);
	
	weapon += tmpw;
	if (weapon<0)
	{
		weapon = 0;
	}
	if (weapon>300)
	{
		weapon = 300;
	}
	// God Enchant :: WV as Mana regen
	if (IS_PLAYER(cn) && (enchantSpec & 16384))
	{
		ch[cn].gigaregen[2] = weapon/20;
	}
	ch[cn].weapon = weapon;
	
	// Enchant: 25% more Thorns
	if (enchantSpec & 256) 
		gethit += gethit*3/10;
	
	// Tree: artm
	if (T_ARTM_SK(cn,  5))       gethit = gethit*120/100;
	if (n=st_skillcount(cn, 17)) gethit = gethit*(100+n*5)/100;
	
	if (gethit<0)
	{
		gethit = 0;
	}
	if (gethit>255)
	{
		gethit = 255;
	}
	ch[cn].gethit_dam = gethit;
	//
	
	if (aoe<-15)
	{
		aoe = -15;
	}
	if (aoe>15)
	{
		aoe = 15;
	}
	ch[cn].aoe_bonus = aoe;
	
	/*
		ch[].top_damage value
		
		Determined by STR/2. This is put into a RANDOM(), so "average damage" can be considered WV plus half of this number
	*/
	
	damage_top = damage_top + attrib_ex[AT_STR]*unarmed/2;
	
	// Tree - Lycan % Top Dmg
	if (T_LYCA_SK(cn,  5))       damage_top = damage_top*120/100;
	if (n=st_skillcount(cn,101)) damage_top = damage_top*(100+n*5)/100;
	
	// Tarot - Hanged.R : 12% less WV, 24% more Top Damage
	if (charmSpec & 8192)
		damage_top = damage_top * 31/25;
	
	// Clamp damage_top between 0 and 999
	if (damage_top > 999)
	{
		damage_top = 999;
	}
	if (damage_top < 0)
	{
		damage_top = 0;
	}
	ch[cn].top_damage = damage_top;
	
	// Force hp/end/mana to sane values
	if (ch[cn].a_hp>ch[cn].hp[5] * 1000)
	{
		ch[cn].a_hp = ch[cn].hp[5] * 1000;
	}
	if (ch[cn].a_end>ch[cn].end[5] * 1000)
	{
		ch[cn].a_end = ch[cn].end[5] * 1000;
	}
	if (ch[cn].a_mana>ch[cn].mana[5] * 1000)
	{
		ch[cn].a_mana = ch[cn].mana[5] * 1000;
	}
	
	// Adjust local light score
	if (oldlight!=ch[cn].light && ch[cn].used==USE_ACTIVE &&
	    ch[cn].x>0 && ch[cn].x<MAPX && ch[cn].y>0 && ch[cn].y<MAPY &&
	    map[ch[cn].x + ch[cn].y * MAPX].ch==cn)
	{
		do_add_light(ch[cn].x, ch[cn].y, ch[cn].light - oldlight);
	}
	
	do_update_permaspells(cn);
	
	prof_stop(7, prof);
}

void do_pmshield(int cn, int co)
{
	int in, ta_co_emp, power, n;
	
	power = M_SK(cn, SK_MSHIELD);
	power = spell_multiplier(power, cn);
	
	ta_co_emp = get_tarot(cn, IT_CH_EMPRESS);
		
	if ((in = has_buff(cn, SK_MSHIELD)) && !ta_co_emp)
	{
		bu[in].duration = SP_DUR_MSHIELD(power);
		return;
	}
	else if ((in = has_buff(cn, SK_MSHELL)) && ta_co_emp)
	{
		bu[in].duration = SP_DUR_MSHELL(power);
		return;
	}
	else
	{
		remove_buff(cn, SK_MSHIELD);
		remove_buff(cn, SK_MSHELL);
	}
	
	if (ta_co_emp)
	{
		in = make_new_buff(cn, SK_MSHELL, BUF_SPR_MSHELL, power, SP_DUR_MSHELL(power), 0);
		n = SK_MSHELL;
	}
	else
	{
		in = make_new_buff(cn, SK_MSHIELD, BUF_SPR_MSHIELD, power, SP_DUR_MSHIELD(power), 0);
		n = SK_MSHIELD;
	}
	
	if (!in) return;
	
	bu[in].active = 1;
	
	if (ta_co_emp)
	{
		bu[in].power = bu[in].active / 128;
		if (IS_SEYA_OR_BRAV(co)) 
		{
			bu[in].skill[SK_RESIST][1] = min(127, bu[in].active / 768 + 1);
			bu[in].skill[SK_IMMUN][1]  = min(127, bu[in].active / 768 + 1);
		}
		else
		{
			bu[in].skill[SK_RESIST][1] = min(127, bu[in].active / 512 + 1);
			bu[in].skill[SK_IMMUN][1]  = min(127, bu[in].active / 512 + 1);
		}
	}
	else
	{
		bu[in].power = bu[in].active / 256;
		if (IS_SEYA_OR_BRAV(co)) 
		{
			bu[in].armor[1]  = min(127, bu[in].active / 1536 + 1);
		}
		else
		{
			bu[in].armor[1]  = min(127, bu[in].active / 1024 + 1);
		}
		if (get_enchantment(co, 57)) bu[in].weapon[1] = bu[in].armor[1]/4;
	}
	
	add_spell(co, in);
}

int get_aria_wv(int cn, int in, int sk)
{
	int weapon = 0;
	
	if (T_SKAL_SK(cn, 6))
	{
		weapon = ch[cn].weapon;
		if (in && bu[in].weapon[1]) weapon -= bu[in].weapon[1];
		weapon = weapon*(100+(T_SKAL_SK(cn,4)?20:0)+sk*5)/100;
		weapon = weapon/10;
	}
	
	return weapon;
}

int get_aria_av(int cn, int in, int sk)
{
	int armor = 0;
	
	if (get_gear(cn, IT_SIGN_SONG))
	{
		armor = ch[cn].armor;
		if (in && bu[in].armor[1]) armor -= bu[in].armor[1];
		armor = armor*(100+(T_SKAL_SK(cn,4)?20:0)+sk*5)/100;
		armor = armor/10;
	}
	
	return armor;
}

void do_aria(int cn)
{
	int _aoe, _rad, n, j, x, y, xf, yf, xt, yt, xc, yc, aoe_power, in, power, co, weapon=0, armor=0;
	double tmp_a;
	
	n = st_skillcount(cn, 28);
	
	power = M_SK(cn, SK_ARIA);
	power = power*(100+(T_SKAL_SK(cn,4)?20:0)+n*5)/100;
	
	in = has_buff(cn, SK_ARIA);
	
	weapon = get_aria_wv(cn, in, n);
	armor  = get_aria_av(cn, in, n);
	
	if (!IS_SKALD(cn))    power /= 4; // Braver
	
	j = 100 + st_skillcount(cn, 53)*5;
	
	aoe_power = M_SK(cn, SK_PROX)+15;
	_rad      = PRXA_RAD + ch[cn].aoe_bonus;
	_aoe      = (aoe_power/(PROX_CAP*2) + _rad) * (T_SORC_SK(cn, 5)?12:10)/10 * j/100;
	tmp_a     = (double)((aoe_power*100/(PROX_CAP*2) + _rad*100) * (T_SORC_SK(cn, 5)?12:10)/10 * j/100);
	
	xc = ch[cn].x;
	yc = ch[cn].y;
	xf = max(1, xc - _aoe);
	yf = max(1, yc - _aoe);
	xt = min(MAPX - 1, xc + 1 + _aoe);
	yt = min(MAPY - 1, yc + 1 + _aoe);
	
	for (x = xf; x<xt; x++) for (y = yf; y<yt; y++) 
	{
		// This makes the radius circular instead of square
		if (sqr(xc - x) + sqr(yc - y) > (sqr(tmp_a/100) + 1))
		{
			continue;
		}
		if (IS_LIVINGCHAR(co = map[x + y * MAPX].ch) && do_char_can_see(cn, co, 0))
		{
			in = 0;
			if ((cn!=co) && do_surround_check(cn, co, 1)) 
			{
				aoe_power = spell_immunity(power, get_target_immunity(cn, co));
				// debuff version
				if (!(in = make_new_buff(cn, SK_ARIA2, BUF_SPR_ARIA2, aoe_power, SP_DUR_ARIA, 0))) 
					continue;
				
				bu[in].cool_bonus[1] = max(-127, -(aoe_power/4 + 1));
				bu[in].data[4] = 1; // Effects not removed by NMZ (SK_ARIA2)
			}
			else
			{
				// buff version
				if (!(in = make_new_buff(cn, SK_ARIA, BUF_SPR_ARIA, power, SP_DUR_ARIA, 0))) 
					continue;
				
				if (IS_SKALD(co))
					bu[in].dmg_bonus[1] = min(127, power/15);
				bu[in].weapon[1] = weapon;
				bu[in].armor[1]  = armor;
				
				bu[in].cool_bonus[1] = min(127, power/4 + 1);
				bu[in].data[4] = 1; // Effects not removed by NMZ (SK_ARIA)
			}
			if (co && in) add_spell(co, in);
		}
	}
}

void do_immolate(int cn, int in)
{
	int _aoe, _rad, j, x, y, xf, yf, xt, yt, xc, yc, aoe_power, in2 = 0, power, co, idx, nn;
	double tmp_a, tmp_s, tmp_h;
	
	j = 100 + st_skillcount(cn, 53)*5;
	
	tmp_s     = (double)(bu[in].power*3/(IS_PLAYER(cn)?2:3));
	aoe_power = M_SK(cn, SK_PROX)+15;
	_rad      = bu[in].data[3];
	_aoe      = (aoe_power/(PROX_CAP*2) + _rad) * (T_SORC_SK(cn, 5)?12:10)/10 * j/100;
	tmp_a     = (double)((aoe_power*100/(PROX_CAP*2) + _rad*100) * (T_SORC_SK(cn, 5)?12:10)/10 * j/100);
	tmp_h     = (double)((sqr(aoe_power*100/PROX_HIT-_aoe)/500+(_rad*300)) * (T_SORC_SK(cn, 5)?12:10)/10 * j/100);
	
	xc = ch[cn].x;
	yc = ch[cn].y;
	xf = max(1, xc - _aoe);
	yf = max(1, yc - _aoe);
	xt = min(MAPX - 1, xc + 1 + _aoe);
	yt = min(MAPY - 1, yc + 1 + _aoe);
	
	for (x = xf; x<xt; x++) for (y = yf; y<yt; y++) 
	{
		// This makes the radius circular instead of square
		if (sqr(xc - x) + sqr(yc - y) > (sqr(tmp_a/100) + 1))
		{
			continue;
		}
		if (IS_LIVINGCHAR(co = map[x + y * MAPX].ch) && do_char_can_see(cn, co, 0) && cn!=co)
		{
			in2 = 0;
			// Prevent from hurting enemies that don't want to hurt you atm
			if (!IS_PLAYER(co) && ch[co].data[25] != 1)
			{
				idx = cn | (char_id(cn) << 16);
				for (nn = MCD_ENEMY1ST; nn<=MCD_ENEMYZZZ; nn++)
				{
					if (ch[co].data[nn]==idx) break;
				}
				if (nn==MCD_ENEMYZZZ+1) continue;
			}
			//
			if (do_surround_check(cn, co, 1)) 
			{
				aoe_power = (int)(double)(min(tmp_s, tmp_s / max(1, (
							sqr(abs(xc - x)) + sqr(abs(yc - y))) / (tmp_h/100))));
				aoe_power = spell_immunity(aoe_power, get_target_immunity(cn, co));
				
				// debuff version
				if (!(in2 = make_new_buff(cn, SK_IMMOLATE2, BUF_SPR_FIRE, aoe_power, SP_DUR_ARIA, 0))) 
					continue;
				
				bu[in2].data[1] = max(100, 100 + (IS_PLAYER(cn))?(aoe_power*4):(aoe_power*3));
				bu[in2].data[4] = 1; // Effects not removed by NMZ (SK_IMMOLATE2)
			}
			if (co && in2) add_spell(co, in2);
		}
	}
}

void do_random_blast(int cn, int power)
{
	int _aoe, _rad, j, x, y, xf, yf, xt, yt, xc, yc, aoe_power, c = 0, co;
	double tmp_a;
	int catalog[64] = { 0 };
	
	if (!cn) return;
	
	power = power/2 + power/4;
	
	j = 100 + st_skillcount(cn, 53)*5;
	
	aoe_power = M_SK(cn, SK_PROX)+15;
	_rad      = PRXA_RAD + ch[cn].aoe_bonus;
	_aoe      = (aoe_power/(PROX_CAP*2) + _rad) * (T_SORC_SK(cn, 5)?12:10)/10 * j/100;
	tmp_a     = (double)((aoe_power*100/(PROX_CAP*2) + _rad*100) * (T_SORC_SK(cn, 5)?12:10)/10 * j/100);
	
	xc = ch[cn].x;
	yc = ch[cn].y;
	xf = max(1, xc - _aoe);
	yf = max(1, yc - _aoe);
	xt = min(MAPX - 1, xc + 1 + _aoe);
	yt = min(MAPY - 1, yc + 1 + _aoe);
	
	for (x = xf; x<xt; x++) for (y = yf; y<yt; y++) 
	{
		// This makes the radius circular instead of square
		if (sqr(xc - x) + sqr(yc - y) > (sqr(tmp_a/100) + 1))
		{
			continue;
		}
		if (IS_SANECHAR(co = map[x + y * MAPX].ch) && cn!=co)
		{
			if (!do_char_can_see(cn, co, 0)) continue;
			if (do_surround_check(cn, co, 1))
			{
				catalog[c] = co;
				c++;
			}
		}
	}
	
	if (c)
	{
		co = catalog[RANDOM(c)];
		if (co) spell_blast(cn, co, power, 0, 1);
	}
}

void do_update_permaspells(int cn)
{
	int n, in, power, tmp = 0, tmpa=0, weapon=0, armor=0;
	
	if (T_LYCA_SK(cn, 7))
		tmp  = (((ch[cn].hp[5]*1000 - ch[cn].a_hp)/1000) + ((ch[cn].end[5]*1000 - ch[cn].a_end)/1000) + ((ch[cn].mana[5]*1000 - ch[cn].a_mana)/1000))/2;
	if (n=st_skillcount(cn, 103))
		tmp += (((ch[cn].hp[5]*1000 - ch[cn].a_hp)/1000) + ((ch[cn].end[5]*1000 - ch[cn].a_end)/1000) + ((ch[cn].mana[5]*1000 - ch[cn].a_mana)/1000))*n/5;
	
	for (n = 0; n<MAXBUFFS; n++)
	{
		if ((in = ch[cn].spell[n]) && ((bu[in].flags & IF_PERMSPELL) || (bu[in].temp==SK_ARIA && (bu[in].data[0] == cn))))
		{
			switch (bu[in].temp)
			{
				case SK_ARIA:
					tmpa = st_skillcount(cn, 28);
					
					power = M_SK(cn, SK_ARIA);
					power = power*(100+(T_SKAL_SK(cn,4)?20:0)+tmpa*5)/100;
					
					weapon = get_aria_wv(cn, in, tmpa);
					armor  = get_aria_av(cn, in, tmpa);
					
					if (!IS_SKALD(cn))
						power /= 4;
					else
						bu[in].dmg_bonus[1]  = min(127, power/15);
					
					bu[in].power 	         = power;
					bu[in].weapon[1]         = weapon;
					bu[in].armor[1]          = armor;
					bu[in].cool_bonus[1]     = min(127, power/4 + 1);
					break;
				case SK_RAGE:
					power = M_SK(cn, SK_RAGE);
					power = skill_multiplier(power, cn);
					bu[in].power = power; power = power + (power * tmp / 5000);
					if (IS_SEYAN_DU(cn))
					{
						bu[in].top_damage[1] 	= min(127, power/ 6 + 5);
						bu[in].data[4]			= power/3;
					}
					else
					{
						bu[in].top_damage[1] 	= min(127, power/ 4 + 5);
						bu[in].data[4]			= power/2;
					}
					break;
				case SK_CALM:
					power = M_SK(cn, SK_RAGE);
					power = skill_multiplier(power, cn);
					bu[in].power = power; power = power + (power * tmp / 4000);
					if (IS_SEYAN_DU(cn))
					{
						bu[in].data[3] 			= min(127, power/ 6 + 5);
						bu[in].data[4]			= power/3;
					}
					else
					{
						bu[in].data[3] 			= min(127, power/ 4 + 5);
						bu[in].data[4]			= power/2;
					}
					break;
				case SK_LETHARGY:
					power = M_SK(cn, SK_LETHARGY);
					if (T_SORC_SK(cn, 7))          power = power + (power * M_AT(cn, AT_WIL)/2000);
					if (tmp=st_skillcount(cn, 55)) power = power + (power * M_AT(cn, AT_WIL)*tmp/5000);
					power = spell_multiplier(power, cn);
					bu[in].power = power;
					break;
				case SK_IMMOLATE:
					//if (bu[in].data[0]==0)
					//{
						power = ch[cn].hp[4] * 30 / 100;
						if (get_book(cn, IT_BOOK_BURN)) power = power + ch[cn].hp[4]/25;
					//}
					//else
					//{
					//	power = M_SK(bu[in].data[0], SK_PULSE);
					//	if (get_book(cn, IT_BOOK_BURN)) power = power + ch[cn].hp[5]/25;
					//	power = spell_multiplier(power, cn);
					//}
					bu[in].power = power;
					bu[in].data[3] = PRXP_RAD + ch[cn].aoe_bonus;
					break;
				default:
					break;
			}
		}
	}
}

// note: this calculates ALL normal endurance/hp changes.
//       further, it is called ONLY from tick()
void do_regenerate(int cn)
{
	unsigned long long prof;
	long long degendam = 0;
	unsigned long long mf1, mf2;
	int n, m, p, in, in2, nohp = 0, noend = 0, nomana = 0, halfhp = 0, halfend = 0, halfmana = 0, old;
	int hp = 0, end = 0, mana = 0, uwater = 0, gothp = 0, sunR = 0, worldR = 0, moonR = 0, money = 0;
	int race_reg = 0, race_res = 0, race_med = 0, cloakofshadows = 0;
	int degenpower = 0, tickcheck = 10000;
	int moonmult = 20, n1=0, n2=0, n3=0;
	int hpmult, endmult, manamult, rank=0;
	int co = -1, cc=0;
	int tmp = 0, kill_bsp = 0, kill_osp = 0, kill_bos = 0;
	int scorched = 0, guarded = 0, devRn = 0, devRo = 0, has_sld = 0, has_shl = 0, phalanx = 0, aggravate = 0;
	int offpot = 0, defpot = 0;
	int idle = 3, power = 0;
	char buf[50];
	
	strcpy(buf, ch[cn].reference); buf[0] = toupper(buf[0]);

	// gothp determines how much to counter degeneration effects while underwater.
	m = ch[cn].x + ch[cn].y * MAPX;
	mf1 = mf2 = map[m].flags;
	
	if (ch[cn].flags & CF_STONED)
		return;

	prof = prof_start();

	if ((ch[cn].flags & (CF_PLAYER)) || (IS_COMP_TEMP(cn) && (co = ch[cn].data[CHD_MASTER]) && IS_SANEPLAYER(co)))
	{
		n1 = st_skillcount(cn, 42)*10; // Full
		n2 = st_skillcount(cn, 54)*10; // New
		n3 = st_skillcount(cn, 99)* 5; // Half
	
		if (IS_GLOB_MAYHEM)				moonmult = 10;
		if (globs->fullmoon)			moonmult = (30*(100+n1+n3))/100;
		if (globs->newmoon)				moonmult = (40*(100+n2+n3))/100;
		
		race_reg = M_SK(cn, SK_REGEN) * moonmult / 20 + (B_SK(cn, SK_REGEN)?M_SK(cn, SK_REGEN):0) * ch[cn].hp[5]   / 2000;
		race_res = M_SK(cn, SK_REST)  * moonmult / 20 + (B_SK(cn, SK_REST) ?M_SK(cn, SK_REST) :0) * ch[cn].end[5]  / 1000;
		race_med = M_SK(cn, SK_MEDIT) * moonmult / 20 + (B_SK(cn, SK_MEDIT)?M_SK(cn, SK_MEDIT):0) * ch[cn].mana[5] / 2000;
		
		if (get_gear(co, IT_TW_INVIDIA)) nohp = 1;
	}
	else
	{
		race_reg = M_SK(cn, SK_REGEN) * moonmult / 30;
		race_res = M_SK(cn, SK_REST)  * moonmult / 30;
		race_med = M_SK(cn, SK_MEDIT) * moonmult / 30;
	}
	
	if (ch[cn].flags & CF_NOHPREG)		halfhp = 1;
	if (ch[cn].flags & CF_NOENDREG)		halfend = 1;
	if (ch[cn].flags & CF_NOMANAREG)	halfmana = 1;

	if (mf1 & MF_UWATER) 			uwater = 1;
	if (get_gear(cn, IT_TW_CLOAK))	cloakofshadows = 1;
	
	hpmult = endmult = manamult = moonmult;
	
	// Tarot - Moon :: While not full mana, life regen is mana regen
	if (get_tarot(cn, IT_CH_MOON) && (ch[cn].a_mana<ch[cn].mana[5] * 1000))
	{
		race_med += race_reg;	race_reg -= race_reg;
		manamult += hpmult;		hpmult   -= hpmult;
	}
	// Tarot - Sun :: While not full life, end regen is life regen
	if (get_tarot(cn, IT_CH_SUN) && (ch[cn].a_hp<ch[cn].hp[5] * 1000))
	{
		race_reg += race_res;	race_res -= race_res;
		hpmult   += endmult;	endmult  -= endmult;
	}
	// Tarot - World :: While not full end, mana regen is end regen
	if (get_tarot(cn, IT_CH_WORLD) && (ch[cn].a_end<ch[cn].end[5] * 1000))
	{
		race_res += race_med;	race_med -= race_med;
		endmult  += manamult;	manamult -= manamult;
	}
	
	if (get_tarot(cn, IT_CH_SUN_R))   sunR   = 1;
	if (get_tarot(cn, IT_CH_WORLD_R)) { worldR = 1; noend = 1; }
	if (get_tarot(cn, IT_CH_MOON_R))  moonR  = 1;
	
	// Meditate added to Hitpoints
	if (get_enchantment(cn, 36))
	{
		race_reg += race_med/2;
		hpmult   += manamult/2;
	}
	// Rest added to mana
	if (get_enchantment(cn, 45))
	{
		race_med += race_res/2;
		manamult += endmult/2;
	}
	
	// Special non-ankh amulets
	if (in = ch[cn].worn[WN_NECK])
	{
		switch (it[in].temp)
		{
			case IT_AM_BLOODS: 
				race_med /= 2;
				race_reg *= 2;
				break;
			case IT_AM_VERDANT: 
				race_reg /= 2;
				race_res *= 2;
				break;
			case IT_AM_SEABREZ: 
				race_res /= 2;
				race_med *= 2;
				break;
			default:
				break;
		}
	}
	
	if (get_gear(cn, IT_BT_NATURES))
		idle = 4;
	
	// Set up basic values to be attributed to player hp/end/mana
	//   These are the "standing" state values and will be divided down when applied to walk/fight states
	hp   = race_reg + hpmult   * 2;
	end  = race_res + endmult  * 3;
	mana = race_med + manamult * 1;
	
	if (ch[cn].stunned!=1)
	{
		switch (ch_base_status(ch[cn].status))
		{
			// STANDING STATES
			case 0: case 1: case 2: case 3: case 4: case 5: case 6: case 7:
					ch[cn].a_hp 			+= (nohp   ? 0 : (hp  / 8 * (sunR?1:8) * idle/3) / (halfhp   ? 2 : 1)); 
					gothp 					+= (nohp   ? 0 : (hp  /16 * (sunR?1:8) * idle/3) / (halfhp   ? 2 : 1));
					ch[cn].a_end 			+= (noend  ? 0 : (end / 8 * (sunR?1:8) * idle/3) / (halfend  ? 2 : 1));
					ch[cn].a_mana 			+= (nomana ? 0 : (mana/ 8 * (sunR?1:8) * idle/3) / (halfmana ? 2 : 1)); 
				break;
			
			// WALKING STATES
			case  16: case  24: case  32: case  40: case  48: case  60: case  72: case  84:
			case  96: case 100: case 104: case 108: case 112: case 116: case 120: case 124:
			case 128: case 132: case 136: case 140: case 144: case 148: case 152:
				if (get_enchantment(cn, 32))
				{
					ch[cn].a_hp 			+= (nohp   ? 0 : (hp     ) / (halfhp   ? 2 : 1)); 
					gothp 					+= (nohp   ? 0 : (hp  / 2) / (halfhp   ? 2 : 1));
					if (worldR)
					{
						if (ch[cn].mode==2) // Fast
						{
							ch[cn].a_end 	+= (noend  ? 0 : (end    ) / (halfend  ? 2 : 1));
							ch[cn].a_mana 	+= (nomana ? 0 : (mana   ) / (halfmana ? 2 : 1)) - 40; 
						}
						else
						{
							ch[cn].a_end 	+= (noend  ? 0 : (end    ) / (halfend  ? 2 : 1));
							ch[cn].a_mana 	+= (nomana ? 0 : (mana   ) / (halfmana ? 2 : 1)); 
						}
					}
					else
					{
						if (ch[cn].mode==2) // Fast
							ch[cn].a_end 	+= (noend  ? 0 : (end    ) / (halfend  ? 2 : 1)) - 40;
						else
							ch[cn].a_end 	+= (noend  ? 0 : (end    ) / (halfend  ? 2 : 1));
						ch[cn].a_mana 		+= (nomana ? 0 : (mana   ) / (halfmana ? 2 : 1)); 
					}
				}
				else
				{
					ch[cn].a_hp 			+= (nohp   ? 0 : (hp  / 4) / (halfhp   ? 2 : 1)); 
					gothp 					+= (nohp   ? 0 : (hp  / 8) / (halfhp   ? 2 : 1));
					if (worldR)
					{
						if (ch[cn].mode==2) // Fast
						{
							ch[cn].a_end 	+= (noend  ? 0 : (end / 4) / (halfend  ? 2 : 1));
							ch[cn].a_mana 	+= (nomana ? 0 : (mana/ 4) / (halfmana ? 2 : 1)) - 40;
						}
						if (ch[cn].mode==1) // Normal
						{
							ch[cn].a_end 	+= (noend  ? 0 : (end / 4) / (halfend  ? 2 : 1));
							ch[cn].a_mana 	+= (nomana ? 0 : (mana/ 4) / (halfmana ? 2 : 1));
						}
						if (ch[cn].mode==0) // Slow
						{
							ch[cn].a_end 	+= (noend  ? 0 : (end / 2) / (halfend  ? 2 : 1));
							ch[cn].a_mana 	+= (nomana ? 0 : (mana/ 4) / (halfmana ? 2 : 1));
						}
					}
					else
					{
						if (ch[cn].mode==2) // Fast
							ch[cn].a_end 	+= (noend  ? 0 : (end / 4) / (halfend  ? 2 : 1)) - 40;
						if (ch[cn].mode==1) // Normal
							ch[cn].a_end 	+= (noend  ? 0 : (end / 4) / (halfend  ? 2 : 1));
						if (ch[cn].mode==0) // Slow
							ch[cn].a_end 	+= (noend  ? 0 : (end / 2) / (halfend  ? 2 : 1));
						ch[cn].a_mana 		+= (nomana ? 0 : (mana/ 4) / (halfmana ? 2 : 1));
					}
				}
				break;
			
			// FIGHTING STATES
			case 160: case 168: case 176: case 184:
				ch[cn].a_hp 				+= (nohp   ? 0 : (hp  / 8 * (sunR?8:1)) / (halfhp   ? 2 : 1)); 
				gothp 						+= (nohp   ? 0 : (hp  /16 * (sunR?8:1)) / (halfhp   ? 2 : 1));
				if (worldR)
				{
					if (ch[cn].status2==0 || ch[cn].status2==5 || ch[cn].status2==6) // Attacking
					{
						if (ch[cn].mode==2) // Fast
						{
							ch[cn].a_end 	+= (noend  ? 0 : (end / 8 * (sunR?8:1)) / (halfend  ? 2 : 1));
							ch[cn].a_mana 	+= (nomana ? 0 : (mana/ 8 * (sunR?8:1)) / (halfmana ? 2 : 1)) - 75;
						}
						if (ch[cn].mode==1) // Normal
						{
							ch[cn].a_end 	+= (noend  ? 0 : (end / 8 * (sunR?8:1)) / (halfend  ? 2 : 1));
							ch[cn].a_mana 	+= (nomana ? 0 : (mana/ 8 * (sunR?8:1)) / (halfmana ? 2 : 1)) - 25;
						}
						if (ch[cn].mode==0) // Slow
						{
							ch[cn].a_end 	+= (noend  ? 0 : (end / 4 * (sunR?4:1)) / (halfend  ? 2 : 1));
							ch[cn].a_mana 	+= (nomana ? 0 : (mana/ 8 * (sunR?8:1)) / (halfmana ? 2 : 1));
						}
					}
					else // Misc.
					{
						if (ch[cn].mode==2)		// Fast
						{
							ch[cn].a_end 	+= (noend  ? 0 : (end / 4 * (sunR?4:1)) / (halfend  ? 2 : 1));
							ch[cn].a_mana 	+= (nomana ? 0 : (mana/ 8 * (sunR?8:1)) / (halfmana ? 2 : 1)) - 50;
						}
						if (ch[cn].mode==1)		// Normal
						{
							ch[cn].a_end 	+= (noend  ? 0 : (end / 4 * (sunR?4:1)) / (halfend  ? 2 : 1));
							ch[cn].a_mana 	+= (nomana ? 0 : (mana/ 8 * (sunR?8:1)) / (halfmana ? 2 : 1));
						}
						if (ch[cn].mode==0)		// Slow
						{
							ch[cn].a_end 	+= (noend  ? 0 : (end / 2 * (sunR?2:1)) / (halfend  ? 2 : 1));
							ch[cn].a_mana 	+= (nomana ? 0 : (mana/ 8 * (sunR?8:1)) / (halfmana ? 2 : 1));
						}
					}
				}
				else
				{
					if (ch[cn].status2==0 || ch[cn].status2==5 || ch[cn].status2==6) // Attacking
					{
						if (ch[cn].mode==2) // Fast
							ch[cn].a_end 	+= (noend  ? 0 : (end / 8 * (sunR?8:1)) / (halfend  ? 2 : 1)) - 75;
						if (ch[cn].mode==1) // Normal
							ch[cn].a_end 	+= (noend  ? 0 : (end / 8 * (sunR?8:1)) / (halfend  ? 2 : 1)) - 25;
						if (ch[cn].mode==0) // Slow
							ch[cn].a_end 	+= (noend  ? 0 : (end / 4 * (sunR?4:1)) / (halfend  ? 2 : 1));
					}
					else // Misc.
					{
						if (ch[cn].mode==2)		// Fast
							ch[cn].a_end 	+= (noend  ? 0 : (end / 4 * (sunR?4:1)) / (halfend  ? 2 : 1)) - 50;
						if (ch[cn].mode==1)		// Normal
							ch[cn].a_end 	+= (noend  ? 0 : (end / 4 * (sunR?4:1)) / (halfend  ? 2 : 1));
						if (ch[cn].mode==0)		// Slow
							ch[cn].a_end 	+= (noend  ? 0 : (end / 2 * (sunR?2:1)) / (halfend  ? 2 : 1));
					}
					ch[cn].a_mana 			+= (nomana ? 0 : (mana/ 8 * (sunR?8:1)) / (halfmana ? 2 : 1));
				}
				break;
			
			default:
				fprintf(stderr, "do_regenerate(): unknown ch_base_status %d.\n", ch_base_status(ch[cn].status));
				break;
		}
	}
	
	if (ch[cn].flags & CF_UNDEAD)
	{
		hp = 450 + getrank(cn) * 25;
		
		// Special case for modular undead power via contracts
		if ((ch[cn].kindred & KIN_MONSTER) && ch[cn].data[47]>9)
		{
			hp = hp*ch[cn].data[47]/100;
		}
		if (ch[cn].temp == CT_DRACULA) hp = hp*10;
		
		ch[cn].a_hp	+= hp / (halfhp   ? 2 : 1);
		gothp 		+= hp / (halfhp   ? 4 : 2);
	}
	if (ch[cn].gigaregen[0])
	{
		hp = ch[cn].gigaregen[0] * 50;
		ch[cn].a_hp		+=   hp / (halfhp   ? 2 : 1);
		gothp 			+=   hp / (halfhp   ? 4 : 2);
	}
	if (ch[cn].gigaregen[1])
	{
		end = ch[cn].gigaregen[1] * 50;
		ch[cn].a_end	+=  end / (halfend  ? 2 : 1);
	}
	if (ch[cn].gigaregen[2])
	{
		mana = ch[cn].gigaregen[2] * 50;
		ch[cn].a_mana	+= mana / (halfmana ? 2 : 1);
	}
	if (ch[cn].temp == CT_PIRATELORD && (IS_SANEITEM(in = map[1331331].it) && !(it[in].active)))
	{
		ch[cn].a_hp	+= 20000;
		gothp 		+= 1;
	}
	
	// Special case for the Amulet of Ankhs
	if (in = ch[cn].worn[WN_NECK])
	{
		switch (it[in].temp)
		{
			case IT_ANKHAMULET: 
				ch[cn].a_hp   += (race_reg/ 8) / (halfhp   ? 2 : 1);
				ch[cn].a_end  += (race_res/ 8) / (halfend  ? 2 : 1);
				ch[cn].a_mana += (race_med/ 8) / (halfmana ? 2 : 1);
				break;
			case IT_AMBERANKH: 
				ch[cn].a_hp   += (race_reg/ 4) / (halfhp   ? 2 : 1);
				ch[cn].a_end  += (race_res/12) / (halfend  ? 2 : 1);
				ch[cn].a_mana += (race_med/12) / (halfmana ? 2 : 1);
				break;
			case IT_TURQUANKH: 
				ch[cn].a_hp   += (race_reg/12) / (halfhp   ? 2 : 1);
				ch[cn].a_end  += (race_res/ 4) / (halfend  ? 2 : 1);
				ch[cn].a_mana += (race_med/12) / (halfmana ? 2 : 1);
				break;
			case IT_GARNEANKH: 
				ch[cn].a_hp   += (race_reg/12) / (halfhp   ? 2 : 1);
				ch[cn].a_end  += (race_res/12) / (halfend  ? 2 : 1);
				ch[cn].a_mana += (race_med/ 4) / (halfmana ? 2 : 1);
				break;
			case IT_TRUEANKH: 
				ch[cn].a_hp   += (race_reg/ 4) / (halfhp   ? 2 : 1);
				ch[cn].a_end  += (race_res/ 4) / (halfend  ? 2 : 1);
				ch[cn].a_mana += (race_med/ 4) / (halfmana ? 2 : 1);
				break;
			case IT_GAMBLERFAL:
				if (it[in].active) 
				{
					it[in].active--;
					if (it[in].active==0)
						do_update_char(cn);
				}
			default:
				break;
		}
	}
	
	// force to sane values
	if (ch[cn].a_hp>ch[cn].hp[5] * 1000)
	{
		ch[cn].a_hp   = ch[cn].hp[5]   * 1000;
	}
	if (ch[cn].a_end>ch[cn].end[5] * 1000)
	{
		ch[cn].a_end  = ch[cn].end[5]  * 1000;
	}
	if (ch[cn].a_mana>ch[cn].mana[5] * 1000)
	{
		ch[cn].a_mana = ch[cn].mana[5] * 1000;
	}
	
	// Force do_update for Tactics at various mana increments
	if (B_SK(cn, SK_TACTICS) && ch[cn].a_mana >= ch[cn].mana[5]*600)
	{
		if (ch[cn].a_mana == (ch[cn].mana[5]*1000-1) ||
			ch[cn].a_mana == (ch[cn].mana[5]* 950-1) ||
			ch[cn].a_mana == (ch[cn].mana[5]* 900-1) ||
			ch[cn].a_mana == (ch[cn].mana[5]* 850-1) ||
			ch[cn].a_mana == (ch[cn].mana[5]* 800-1) ||
			ch[cn].a_mana == (ch[cn].mana[5]* 750-1) ||
			ch[cn].a_mana == (ch[cn].mana[5]* 700-1) ||
			ch[cn].a_mana == (ch[cn].mana[5]* 650-1) ||
			ch[cn].a_mana == (ch[cn].mana[5]* 600-1) ) do_update_char(cn);
	}

	if ((hp && ch[cn].a_hp<ch[cn].hp[5] * 900) || (mana && ch[cn].a_mana<ch[cn].mana[5] * 900))
	{
		ch[cn].data[92] = TICKS * 60;
	}
	
	if (worldR)
	{
		if (ch[cn].a_mana<500 && ch[cn].mode!=0)
		{
			ch[cn].mode = 0;
			do_update_char(cn);
			do_char_log(cn, 0, "You're exhausted.\n");
			if (ch[cn].a_mana<0) ch[cn].a_mana = 0;
		}
	}
	else if (ch[cn].a_end<500 && ch[cn].mode!=0)
	{
		ch[cn].mode = 0;
		do_update_char(cn);
		do_char_log(cn, 0, "You're exhausted.\n");
		if (ch[cn].a_end<0) ch[cn].a_end = 0;
	}
	
	if (B_SK(cn, SK_ARIA)) do_aria(cn);
	if (get_tarot(cn, IT_CH_PREIST_R) && B_SK(cn, SK_MSHIELD)) do_pmshield(cn, cn);
	if (IS_COMP_TEMP(cn) && IS_SANECHAR(cc = ch[cn].data[CHD_MASTER]) && T_SUMM_SK(cc, 10))
	{
		if (get_tarot(cc, IT_CH_PREIST_R) && B_SK(cc, SK_MSHIELD)) do_pmshield(cc, cn);
	}
	if (in = has_buff(cn, SK_IMMOLATE)) do_immolate(cn, in);
	
	// Tick down escape try
	if (ch[cn].escape_timer > 0) 
		ch[cn].escape_timer--;
	
	// Tick down success try
	if (ch[cn].escape_timer > TICKS && ch[cn].escape_timer < TICKS*2)
		ch[cn].escape_timer=0;
	
	// spell effects
	for (n = 0; n<MAXBUFFS; n++)
	{
		if ((in = ch[cn].spell[n])!=0)
		{
			if ((bu[in].flags & IF_PERMSPELL) || bu[in].temp==206)
			{
				if ((bu[in].temp==206 && !IS_IN_SUN(ch[cn].x, ch[cn].y)) ||
					(bu[in].temp==SK_SANGUINE && !IS_IN_SANG(ch[cn].x, ch[cn].y)))
					bu[in].active--;
				
				if (bu[in].temp==SK_DWLIGHT)
				{
					if (!IS_IN_DW(ch[cn].x, ch[cn].y))
					{
						bu[in].used = USE_EMPTY;
						ch[cn].spell[n] = 0;
						do_update_char(cn);
						continue;
					}
					if (bu[in].power < 1)
					{
						hp = max(0, min(10, map[m].light));
						bu[in].hp[0] -= (10 - hp);
						if (bu[in].hp[0]<-20000) bu[in].hp[0]=-20000;
					}
				}
				
				// Vantablack debuff - damage determined by light value
				if (bu[in].temp==215)
				{
					if (!IS_IN_VANTA(ch[cn].x, ch[cn].y))
					{
						do_char_log(cn, 1, "The vantablack curse was lifted.\n");
						bu[in].used = USE_EMPTY;
						ch[cn].spell[n] = 0;
						do_update_char(cn);
						continue;
					}
					hp = max(0, min(30, map[m].light));
					ch[cn].a_hp -= (30 - hp)*(30 - hp) + (30 - hp)*(30 - hp)*3*ch[cn].hp[5]/999;
				}
				
				/*
				if (bu[in].temp==SK_OPPRESSED2 && !IS_IN_ABYSS(ch[cn].x, ch[cn].y))
				{
					do_char_log(cn, 1, "The pressure on you was lifted.\n");
					bu[in].used = USE_EMPTY;
					ch[cn].spell[n] = 0;
					do_update_char(cn);
					continue;
				}
				*/
				
				if (bu[in].temp==SK_RAGE || bu[in].temp==SK_CALM)
				{
					if (bu[in].active>(bu[in].duration-TICKS*5)) bu[in].active--;
					if (bu[in].data[2]==1) bu[in].hp[0]   = -(ch[cn].a_hp  /2000);
					if (bu[in].data[2]==2) bu[in].end[0]  = -(ch[cn].a_end /2000);
					if (bu[in].data[2]==3) bu[in].mana[0] = -(ch[cn].a_mana/2000);
					
					if (ch[cn].a_end<500)
					{
						ch[cn].a_end  = 500;	bu[in].active = 0;
						chlog(cn, "%s ran out due to lack of endurance.", it[in].name);
					}
					if (ch[cn].a_mana<500)
					{
						ch[cn].a_mana  = 500;	bu[in].active = 0;
						chlog(cn, "%s ran out due to lack of mana.", it[in].name);
					}
				}
				
				if (bu[in].hp[0]!=-1)
				{
					degendam = bu[in].hp[0];
					if (degendam<0 && cloakofshadows)
					{
						ch[cn].a_mana 	+= degendam/10;
						degendam 		+= degendam/10;
					}
					ch[cn].a_hp += degendam;
				}
				if (bu[in].end[0]!=-1)
				{
					ch[cn].a_end += bu[in].end[0];
				}
				if (bu[in].mana[0]!=-1)
				{
					ch[cn].a_mana += bu[in].mana[0];
				}
				if (ch[cn].a_hp<1000 && bu[in].hp[0] && (bu[in].temp==SK_LETHARGY || bu[in].temp==SK_IMMOLATE || 
					(bu[in].temp==SK_RAGE && bu[in].data[2]==1) || (bu[in].temp==SK_CALM && bu[in].data[2]==1)))
				{
					ch[cn].a_hp = 1000;
					bu[in].active = 0;
					chlog(cn, "%s ran out due to lack of hitpoints.", it[in].name);
				}
				if (ch[cn].a_hp<500 && (bu[in].hp[0] < -1 || bu[in].temp==215 || bu[in].temp==SK_DWLIGHT))
				{
					if (ch[cn].flags & CF_IMMORTAL)
					{
						ch[cn].a_hp=500;
					}
					else
					{
						if (!(mf1 & MF_ARENA) && try_lucksave(cn))
						{
							do_lucksave(cn, bu[in].name);
						}
						else
						{
							chlog(cn, "killed by: %s", bu[in].name);
							do_char_log(cn, 0, "The %s killed you!\n", bu[in].name);
							do_area_log(cn, 0, ch[cn].x, ch[cn].y, 0, "The %s killed %s.\n", bu[in].name, ch[cn].reference);
							do_char_killed(0, cn, 0);
						}
						return;
					}
				}
				// 2375 = Shiva Toxicosis
				if (bu[in].temp != 2375)
				{
					if (ch[cn].a_end<500 && bu[in].end[0] < -1)
					{
						ch[cn].a_end  = 500;
						if (bu[in].temp != 206 && bu[in].temp != SK_SANGUINE)
						{
							bu[in].active = 0;
							chlog(cn, "%s ran out due to lack of endurance.", it[in].name);
						}
					}
					if (ch[cn].a_mana<500 && bu[in].mana[0] < -1)
					{
						ch[cn].a_mana = 500;
						if (bu[in].temp != 206 && bu[in].temp != SK_SANGUINE)
						{
							bu[in].active = 0;
							chlog(cn, "%s ran out due to lack of mana.", it[in].name);
						}
					}
				}
				
				if (bu[in].temp==SK_RAGE || bu[in].temp==SK_CALM)
				{
					tmp   = 0;
					power = bu[in].power;
					
					if (T_LYCA_SK(cn, 7))
						tmp  = (((ch[cn].hp[5]*1000 - ch[cn].a_hp)/1000) + ((ch[cn].end[5]*1000 - ch[cn].a_end)/1000) + ((ch[cn].mana[5]*1000 - ch[cn].a_mana)/1000))/2;
					if (m=st_skillcount(cn, 103))
						tmp += (((ch[cn].hp[5]*1000 - ch[cn].a_hp)/1000) + ((ch[cn].end[5]*1000 - ch[cn].a_end)/1000) + ((ch[cn].mana[5]*1000 - ch[cn].a_mana)/1000))*m/5;
					
					power = power + (power * tmp / 5000);
					
					if (bu[in].temp==SK_RAGE)
					{
						bu[in].top_damage[1] 	= min(127, power/ 4 + 5);
						bu[in].data[4] 			= power/2;
					}
					if (bu[in].temp==SK_CALM)
					{
						bu[in].data[3] 			= min(127, power/ 4 + 5);
						bu[in].data[4] 			= power/2;
					}
					do_update_char(cn);
				}
			}
			else
			{
				if ((bu[in].temp==SK_MSHIELD || bu[in].temp==SK_MSHELL) && get_tarot(cn, IT_CH_PREIST_R))
				{
					if (IS_CNSTANDING(cn)) 		bu[in].active += 64;
					else if (IS_CNWALKING(cn)) 	bu[in].active += 32;
					else 						bu[in].active += 16;
					if (bu[in].active>bu[in].duration) bu[in].active = bu[in].duration;
				}
				else
				{
					bu[in].active--;
					if ((bu[in].active%2==bu[in].duration%2) && bu[in].temp==SK_EXHAUST && bu[in].data[0]==SK_LEAP && bu[in].data[1]>0)
					{
						bu[in].data[1]--;
						skill_leap(cn, 1);
					}
					if (bu[in].active==TICKS*30 && bu[in].duration>=TICKS*60) // don't msg for skills shorter than 1m
					{
						if (ch[cn].flags & (CF_PLAYER | CF_USURP))
						{
							do_char_log(cn, 5, "%s is about to run out.\n", bu[in].name);
						}
						else
						{
							if (IS_COMP_TEMP(cn) && (co = ch[cn].data[CHD_MASTER]) && IS_SANEPLAYER(co) 
								&& (bu[in].temp==SK_BLESS || bu[in].temp==SK_PROTECT || bu[in].temp==SK_ENHANCE))
							{
								do_sayx(cn, "My spell %s is running out, %s.", bu[in].name, ch[co].name);
							}
						}
					}
				}
			}
			
			// Healing potions
			if (bu[in].temp==102 || bu[in].temp==SK_POME || bu[in].temp==SK_SOL)
			{
				if (bu[in].hp[0])
				{
					degendam = bu[in].hp[0];
					if (degendam<0 && cloakofshadows)
					{
						ch[cn].a_mana 	+= degendam/10;
						degendam 		+= degendam/10;
					}
					ch[cn].a_hp += degendam;
					if (ch[cn].a_hp>ch[cn].hp[5] * 1000) ch[cn].a_hp = ch[cn].hp[5] * 1000;
				}
				if (bu[in].end[0])
				{
					ch[cn].a_end += bu[in].end[0];
					if (ch[cn].a_end>ch[cn].end[5] * 1000) ch[cn].a_end = ch[cn].end[5] * 1000;
					if (ch[cn].a_end<0) ch[cn].a_end = 0;
				}
				if (bu[in].mana[0])
				{
					ch[cn].a_mana += bu[in].mana[0];
					if (ch[cn].a_mana>ch[cn].mana[5] * 1000) ch[cn].a_mana = ch[cn].mana[5] * 1000;
					if (ch[cn].a_mana<0) ch[cn].a_mana = 0;
				}
				if (ch[cn].a_hp<500)
				{
					if (ch[cn].flags & CF_IMMORTAL)
					{
						ch[cn].a_hp=500;
					}
					else
					{
						// reset spawn point
						ch[cn].temple_x = ch[cn].tavern_x = HOME_TEMPLE_X;
						ch[cn].temple_y = ch[cn].tavern_y = HOME_TEMPLE_Y;
						if (ch[cn].kindred & KIN_PURPLE)
						{
							ch[cn].temple_x = HOME_PURPLE_X;
							ch[cn].temple_y = HOME_PURPLE_Y;
						}
						// remove other poisons
						for (m=0;m<MAXITEMS;m++)
						{
							if ((in2 = ch[cn].item[m]) && it[in2].temp==IT_POT_DEATH)
								god_take_from_char(in2, cn);
						}
						// death
						do_area_log(cn, cn, ch[cn].x, ch[cn].y, 0, "%s died from a nasty poison.\n", buf);
						do_char_log(cn, 0, "Oh dear, that poison was fatal. You died...\n");
						chlog(cn, "Drank poison and died.", ch[cn].name);
						do_char_killed(0, cn, 0);
						return;
					}
				}
			}
			
			// Regen
			if (bu[in].temp==SK_REGEN)
			{
				ch[cn].a_hp += bu[in].hp[0];
				if (ch[cn].a_hp>ch[cn].hp[5] * 1000)
					ch[cn].a_hp = ch[cn].hp[5] * 1000;
			}
			
			// Slow and Curse2 Decay
			if ((bu[in].temp==SK_SLOW || bu[in].temp==SK_CURSE2) && bu[in].active>0 && bu[in].active <= (bu[in].duration-5) && (bu[in].active % 5))
			{
				p = bu[in].power;
				if (bu[in].active<=bu[in].duration*(bu[in].data[1]-(p/2))/max(1,(p-(p/2))))
				{
					bu[in].data[1] -= p / TICKS;
					if (bu[in].data[1] > p)		bu[in].data[1] = p;
					if (bu[in].data[1] < p / 2)	bu[in].data[1] = p / 2;
					p = bu[in].data[1];
					if (bu[in].temp==SK_SLOW)
					{
						if (get_enchantment(cn, 14)) p = p/5;
						bu[in].speed[1] 		= -(min(300, 10 + SLOWFORM(p)/2));
						bu[in].atk_speed[1] 	= -(min(127, 10 + SLOWFORM(p)/2));
						bu[in].cast_speed[1] 	= -(min(127, 10 + SLOWFORM(p)/2));
					}
					else if (bu[in].temp==SK_CURSE2)
					{
						if (get_enchantment(cn, 21)) p = p/5;
						for (m = 0; m<5; m++) 
						{
							bu[in].attrib[m][1] = -(5 + CURSE2FORM(p, (4 - m)));
						}
					}
					do_update_char(cn);
				}
			}
			
			// Poison & Bleed
			if (bu[in].temp==SK_POISON || bu[in].temp==SK_VENOM || bu[in].temp==SK_BLEED || bu[in].temp==SK_IMMOLATE2 || bu[in].temp==SK_PLAGUE)
			{
				co = bu[in].data[0];
				degenpower = bu[in].data[1];
				
				if (!IS_SANECHAR(co)) co = 0;
				if (co)
				{
					mf2 &= map[ch[co].x + ch[co].y * MAPX].flags;
				}
				if ((co && !(ch[co].flags & CF_STONED)) || !co)
				{
					if (degenpower<1) degenpower = 1;
					degendam = degenpower;
					
					degendam = spell_metabolism(degendam, get_target_metabolism(cn));
					
					if (co && (in2 = has_buff(co, SK_RAGE))) degendam = degendam * (2000 + bu[in2].data[4]) / 2000;
					
					// Easy new method!
					if (co) degendam = degendam * ch[co].dmg_bonus / 10000;
							degendam = degendam * ch[cn].dmg_reduction / 10000;
					
					if (in2 = has_buff(cn, SK_CALM)) degendam = degendam * (2000 - bu[in2].data[4]) / 2000;
					
					if (tmp = get_enchantment(cn, 53))
						degendam = degendam * max(25, 100-(tmp*15))/100;
					
					if (cloakofshadows)
					{
						ch[cn].a_mana 	-= degendam/10;
						degendam 		-= degendam/10;
					}
					
					if (ch[cn].a_hp - (degendam + gothp)<500 && !(mf2 & MF_ARENA) && try_lucksave(cn) && !(ch[cn].flags & CF_IMMORTAL))
					{
						switch (bu[in].temp)
						{
							case SK_POISON: 	do_lucksave(cn, "lethal poisoning"); 	break;
							case SK_VENOM: 		do_lucksave(cn, "lethal venom"); 		break;
							case SK_BLEED: 		do_lucksave(cn, "lethal bleeding"); 	break;
							case SK_IMMOLATE2: 	do_lucksave(cn, "lethal burning"); 		break;
							case SK_PLAGUE: 	do_lucksave(cn, "lethal plague");	 	break;
							default: break;
						}
					}
					else
					{
						ch[cn].a_hp -= degendam + gothp;
					}
					
					if (ch[cn].flags & CF_EXTRAEXP)  degendam = degendam * 2;
					if (ch[cn].flags & CF_EXTRACRIT) degendam = degendam * 3/2;
					
					tickcheck = max(1, 10000/max(1, degendam));
					
					if (co && (globs->ticker+cn)%tickcheck==0 && co!=cn)
					{
						if (!(mf2 & MF_ARENA))
						{
							ch[co].points += 1;
							ch[co].points_tot += 1;
							do_check_new_level(co, 1);
						}
						// God Enchant :: Restore DOT dealt as HP
						if (get_enchantment(co, 66))
						{
							ch[co].a_hp += 100;
							if (ch[co].a_hp > ch[co].hp[5] * 1000)
								ch[co].a_hp = ch[co].hp[5] * 1000;
						}
					}
					
					if (ch[cn].a_hp<500)
					{
						if (ch[cn].flags & CF_IMMORTAL)
						{
							ch[cn].a_hp=500;
						}
						else
						{
							tmp = 0;
							
							if (co)
							{
								switch (bu[in].temp)
								{
									case SK_POISON:
										do_area_log(co, cn, ch[co].x, ch[co].y, 0, "%s died from a nasty poison.\n", buf);
										if (!(ch[co].flags & CF_SYS_OFF))
											do_char_log(co, 0, "Your poison killed %s.\n", ch[cn].reference);
										if (ch[co].flags & CF_INVISIBLE)
											do_char_log(cn, 0, "Oh dear, that poison was fatal. Somebody killed you...\n");
										else
											do_char_log(cn, 0, "Oh dear, that poison was fatal. %s killed you...\n", ch[co].name);
										break;
									case SK_VENOM:
										do_area_log(co, cn, ch[co].x, ch[co].y, 0, "%s died from a nasty venom.\n", buf);
										if (!(ch[co].flags & CF_SYS_OFF))
											do_char_log(co, 0, "Your venom killed %s.\n", ch[cn].reference);
										if (ch[co].flags & CF_INVISIBLE)
											do_char_log(cn, 0, "Oh dear, that venom was fatal. Somebody killed you...\n");
										else
											do_char_log(cn, 0, "Oh dear, that venom was fatal. %s killed you...\n", ch[co].name);
										break;
									case SK_BLEED:
										do_area_log(co, cn, ch[co].x, ch[co].y, 0, "%s died from their bleeding wound.\n", buf);
										if (!(ch[co].flags & CF_SYS_OFF))
											do_char_log(co, 0, "Your bleed killed %s.\n", ch[cn].reference);
										if (ch[co].flags & CF_INVISIBLE)
											do_char_log(cn, 0, "Oh dear, that bleeding was fatal. Somebody killed you...\n");
										else
											do_char_log(cn, 0, "Oh dear, that bleeding was fatal. %s killed you...\n", ch[co].name);
										break;
									case SK_IMMOLATE2:
										do_area_log(co, cn, ch[co].x, ch[co].y, 0, "%s died from the terrible heat.\n", buf);
										if (!(ch[co].flags & CF_SYS_OFF))
											do_char_log(co, 0, "Your immolate killed %s.\n", ch[cn].reference);
										if (ch[co].flags & CF_INVISIBLE)
											do_char_log(cn, 0, "Oh dear, that heat was fatal. Somebody killed you...\n");
										else
											do_char_log(cn, 0, "Oh dear, that heat was fatal. %s killed you...\n", ch[co].name);
										break;
									case SK_PLAGUE:
										do_area_log(co, cn, ch[co].x, ch[co].y, 0, "%s died from an awful plague.\n", buf);
										if (!(ch[co].flags & CF_SYS_OFF))
											do_char_log(co, 0, "Your plague killed %s.\n", ch[cn].reference);
										if (ch[co].flags & CF_INVISIBLE)
											do_char_log(cn, 0, "Oh dear, that plague was fatal. Somebody killed you...\n");
										else
											do_char_log(cn, 0, "Oh dear, that plague was fatal. %s killed you...\n", ch[co].name);
										break;
									default: break;
								}
							}
							else
							{
								switch (bu[in].temp)
								{
									case SK_POISON:
										do_area_log(cn, cn, ch[cn].x, ch[cn].y, 0, "%s died from a nasty poison.\n", buf);
										do_char_log(cn, 0, "Oh dear, that poison was fatal. You died...\n");
										break;
									case SK_VENOM:
										do_area_log(cn, cn, ch[cn].x, ch[cn].y, 0, "%s died from a nasty venom.\n", buf);
										do_char_log(cn, 0, "Oh dear, that venom was fatal. You died...\n");
										break;
									case SK_BLEED:
										do_area_log(cn, cn, ch[cn].x, ch[cn].y, 0, "%s died from their bleeding wound.\n", buf);
										do_char_log(cn, 0, "Oh dear, that bleeding was fatal. You died...\n");
										break;
									case SK_IMMOLATE2:
										do_area_log(cn, cn, ch[cn].x, ch[cn].y, 0, "%s died from the terrible heat.\n", buf);
										do_char_log(cn, 0, "Oh dear, that heat was fatal. You died...\n");
										break;
									case SK_PLAGUE:
										do_area_log(cn, cn, ch[cn].x, ch[cn].y, 0, "%s died from an awful plague.\n", buf);
										do_char_log(cn, 0, "Oh dear, that plague was fatal. You died...\n");
										break;
									default: break;
								}
							}
							chlog(co, "Killed %s", ch[cn].name);
							if (co && !(mf2 & MF_ARENA))
							{
								if (ch[cn].temp>=42 && ch[cn].temp<=70)
								{
									if (IS_SANEPLAYER(ch[co].data[CHD_MASTER]))
										ch[ch[co].data[CHD_MASTER]].data[77]++;
									else
										ch[co].data[77]++;
								}
								tmp  = do_char_score(cn);
								rank = getrank(cn);
								
								for (m = 0; m<MAXBUFFS; m++) if ((in2 = ch[cn].spell[m])) 
								{
									if (!B_SK(cn, SK_MEDIT) && (bu[in2].temp==SK_PROTECT || bu[in2].temp==SK_ENHANCE || bu[in2].temp==SK_BLESS || bu[in2].temp==SK_HASTE))
										tmp += tmp / 5;
									if (bu[in2].temp==105) // map exp bonus
										tmp += tmp*bu[in2].power*RATE_P_PLXP/100;
									if (bu[in2].temp==106) // map bonus bsp
										kill_bsp = bu[in2].power;
									if (bu[in2].temp==107) // map bonus osp
										kill_osp = bu[in2].power;
									if (bu[in2].temp==108) // tree bonus osp
										kill_bos = bu[in2].power;
								}
								if (IS_GLOB_MAYHEM) tmp += tmp / 5;
								if (ch[cn].flags & CF_EXTRAEXP)  tmp = tmp * 2;
								if (ch[cn].flags & CF_EXTRACRIT) tmp = tmp * 3/2;
							}

							if (co && co!=cn && !(mf2 & MF_ARENA))
							{
								if ((in2 = get_gear(co, IT_FORTERING)) && it[in2].active) // 25% more exp with Forte Ring
								{
									tmp = tmp*4/3;
								}
								if ((IS_PLAYER(co) || IS_PLAYER(ch[co].data[CHD_MASTER])) && !IS_PLAYER(cn) && ch[cn].gold) { money = ch[cn].gold; ch[cn].gold = 0; }
								do_give_exp(co, tmp, 1, rank, money);
								
								// stronghold points for contract
								if (kill_bsp)
									do_give_bspoints(co, tmp*kill_bsp*RATE_P_ENBS/100, 1);
								// osiris points for contract
								if (kill_osp)
									do_give_ospoints(co, tmp*kill_osp*RATE_P_ENOS/100, 1);
								// bonus osiris points from tree
								if (kill_bos)
									do_give_ospoints(co, tmp/20, 1);
								
								// stronghold points based on the subdriver of the npc
								if ((ch[cn].data[26]>=101 && ch[cn].data[26]<=399) || ch[cn].temp==347)
								{
									tmp = do_char_score(cn);
									
									if (IS_GLOB_MAYHEM) tmp += tmp / 5;
									if (ch[cn].flags & CF_EXTRAEXP)  tmp = tmp * 2;
									if (ch[cn].flags & CF_EXTRACRIT) tmp = tmp * 3/2;
									
									tmp = max(1, tmp/20);
									
									if (!IS_PLAYER(co) && ch[co].data[CHD_MASTER] && IS_SANEPLAYER(ch[co].data[CHD_MASTER]))
										ch[ch[co].data[CHD_MASTER]].data[26]++;
									else
										ch[co].data[26]++;
									
									do_give_bspoints(co, tmp, 1);
								}
							}
							do_char_killed(co, cn, 0);
							
							return;
						}
					}
				}
			}
			
			// Blind/Douse for Signet of Storms
			if ((bu[in].temp==SK_BLIND || bu[in].temp==SK_DOUSE) && (tmp = bu[in].data[1]) && globs->ticker>bu[in].data[2] && (co = bu[in].data[0]))
			{
				if (co && get_gear(co, IT_SIGN_STOR))
				{
					if (in2 = has_spell(co, SK_ZEPHYR))
					{
						spell_zephyr(co, cn, bu[in2].power, 1);
					}
					else if (tmp)
					{
						spell_zephyr(co, cn, tmp, 1);
					}
				}
				bu[in].data[2] = globs->ticker + TICKS*5;
			}
			
			// Frostburn
			if (bu[in].temp==SK_FROSTB)
			{
				ch[cn].a_end  += bu[in].end[0];
				ch[cn].a_mana += bu[in].mana[0];
				if (ch[cn].a_end<0)  ch[cn].a_end  = 0;
				if (ch[cn].a_mana<0) ch[cn].a_mana = 0;
			}
			
			// Pulse
			if ((bu[in].temp==SK_PULSE || bu[in].temp==SK_PULSE2) && globs->ticker>bu[in].data[2] && (co = bu[in].data[0]))
			{
				int pulse_dam, pulse_aoe, pulse_rad, j, x, y, xf, yf, xt, yt, xc, yc, aoe_power, cc;
				double tmp_a, tmp_h, tmp_s;
				int idx, nn;
				
				cc 		= cn;
				tmp_s   = (double)(bu[in].power);
				
				j = 100 + st_skillcount(cc, 53)*5;
				
				aoe_power = M_SK(cc, SK_PROX)+15;
				pulse_rad = bu[in].data[3];
				pulse_aoe = (aoe_power/(PROX_CAP*2) + pulse_rad) * (T_SORC_SK(cc, 5)?12:10)/10 * j/100;
				tmp_a   = (double)((aoe_power*100/(PROX_CAP*2) + pulse_rad*100) * (T_SORC_SK(cc, 5)?12:10)/10 * j/100);
				tmp_h   = (double)((sqr(aoe_power*100/PROX_HIT-pulse_aoe)/500+(pulse_rad*300)) * (T_SORC_SK(cc, 5)?12:10)/10 * j/100);
				
				xc = ch[cn].x;
				yc = ch[cn].y;
				xf = max(1, xc - pulse_aoe);
				yf = max(1, yc - pulse_aoe);
				xt = min(MAPX - 1, xc + 1 + pulse_aoe);
				yt = min(MAPY - 1, yc + 1 + pulse_aoe);
				
				for (x = xf; x<xt; x++) for (y = yf; y<yt; y++) 
				{
					// This makes the radius circular instead of square
					if (sqr(xc - x) + sqr(yc - y) > (sqr(tmp_a/100) + 1))
					{
						continue;
					}
					if ((co = map[x + y * MAPX].ch) && ((cn!=co && cc!=co) || bu[in].temp==SK_PULSE2))
					{
						if (bu[in].temp==SK_PULSE)
						{
							// Prevent pulse from hitting enemies that don't want to hurt you atm
							idx = cn | (char_id(cn) << 16);
							for (nn = MCD_ENEMY1ST; nn<=MCD_ENEMYZZZ; nn++)
							{
								if (ch[co].data[nn]==idx) break;
							}
							if (nn==MCD_ENEMYZZZ+1) continue;
						}
						//
						pulse_dam = (int)(double)(min(tmp_s, tmp_s / max(1, (
							sqr(abs(xc - x)) + sqr(abs(yc - y))) / (tmp_h/100))));
						if (bu[in].temp==SK_PULSE)
						{
							remember_pvp(cn, co);
							if (do_surround_check(cn, co, 1))
							{
								do_hurt(cn, co, spell_immunity(pulse_dam, get_target_immunity(cn, co)) * 2, 6);
								spell_shock(cn, co, pulse_dam);
								
								check_gloves(cn, co, -1, RANDOM(20), RANDOM(20));
								
								char_play_sound(co, ch[cn].sound + 20, -150, 0);
								do_area_sound(co, 0, ch[co].x, ch[co].y, ch[cn].sound + 20);
								fx_add_effect(5, 0, ch[co].x, ch[co].y, 0);
							}
						}
						else if (bu[in].temp==SK_PULSE2 && !do_surround_check(cn, co, 1))
						{
							ch[co].a_hp += spell_multiplier(pulse_dam * DAM_MULT_PULSE / 2, cn);
							if (ch[co].a_hp > ch[co].hp[5] * 1000) ch[co].a_hp = ch[co].hp[5] * 1000;
							spell_charge(cn, co, pulse_dam);
							
							fx_add_effect(6, 0, ch[co].x, ch[co].y, 0);
						}
					}
				}
				
				// Set next tick schedule
				bu[in].data[2] = globs->ticker + bu[in].data[1];
			}
			
			// Blue pills in lab 7
			if (bu[in].temp==IT_BLUEPILL)
			{
				uwater = 0;
			}
			
			if (bu[in].temp==SK_MSHIELD)
			{
				old = bu[in].armor[1]; tmp = 1024;
				if (IS_SEYA_OR_BRAV(cn)) tmp += 256*2;
				if (has_shl) tmp += 256*3;
				bu[in].armor[1] = min(127, bu[in].active / tmp + 1);
				if (get_enchantment(cn, 57)) bu[in].weapon[1] = bu[in].armor[1]/4;
				bu[in].power = bu[in].active / 256;
				if (old!=bu[in].armor[1])
				{
					do_update_char(cn);
				}
			}
			if (bu[in].temp==SK_MSHELL)
			{
				old = bu[in].skill[SK_RESIST][1]; tmp = 512;
				if (IS_SEYA_OR_BRAV(cn)) tmp += 128*2;
				if (has_sld) tmp += 128*3;
				bu[in].skill[SK_RESIST][1] = min(127, bu[in].active / tmp + 1);
				bu[in].skill[SK_IMMUN][1]  = min(127, bu[in].active / tmp + 1);
				bu[in].power = bu[in].active / 128;
				if (old!=bu[in].skill[SK_RESIST][1])
				{
					do_update_char(cn);
				}
			}
			
			if (bu[in].temp==SK_ZEPHYR2 && (co = bu[in].data[0]))
			{
				p = 0;
				if (bu[in].data[2]>0) // Stored extra hit 2
				{
					bu[in].data[2]--;
					if (!bu[in].data[2])
					{
						tmp = do_hurt(co, cn, bu[in].power * 2, 7);
						p = 1;
						chlog(co, "Zephyr hit %s for %d damage", ch[cn].name, tmp);
					}
					bu[in].value--;
					bu[in].sprite[1] = min(6728, max(6726, 6726+bu[in].value-1));
					bu[in].flags |= IF_UPDATE;
				}
				if (bu[in].data[1]>0) // Stored extra hit 1
				{
					bu[in].data[1]--;
					if (!bu[in].data[1])
					{
						tmp = do_hurt(co, cn, bu[in].power * 2, 7);
						p = 1;
						chlog(co, "Zephyr hit %s for %d damage", ch[cn].name, tmp);
					}
					bu[in].value--;
					bu[in].sprite[1] = min(6728, max(6726, 6726+bu[in].value-1));
					bu[in].flags |= IF_UPDATE;
				}
				if (!bu[in].active) // Final hit of zephyr
				{
					tmp = do_hurt(co, cn, bu[in].power * 2, 7);
					p = 1;
					chlog(co, "Zephyr hit %s for %d damage", ch[cn].name, tmp);
				}
				if (p)
				{
					char_play_sound(cn, ch[co].sound + 20, -150, 0);
					do_area_sound(cn, 0, ch[cn].x, ch[cn].y, ch[co].sound + 20);
					fx_add_effect(5, 0, ch[cn].x, ch[cn].y, 0);
				}
			}
			
			if (!bu[in].active)
			{
				if ((bu[in].temp==SK_MSHIELD || bu[in].temp==SK_MSHELL) && get_tarot(cn, IT_CH_PREIST_R))
				{
					bu[in].active = 1;
					continue;
				}
				if (bu[in].temp==SK_RECALL && ch[cn].used==USE_ACTIVE)
				{
					int xo, yo;

					xo = ch[cn].x;
					yo = ch[cn].y;

					if (god_transfer_char(cn, bu[in].data[1], bu[in].data[2]))
					{
						if (!(ch[cn].flags & CF_INVISIBLE))
						{
							fx_add_effect(12, 0, xo, yo, 0);
							char_play_sound(cn, ch[cn].sound + 21, -150, 0);
							fx_add_effect(12, 0, ch[cn].x, ch[cn].y, 0);
						}
					}
					ch[cn].status = 0;
					ch[cn].attack_cn = 0;
					ch[cn].skill_nr  = 0;
					ch[cn].goto_x = 0;
					ch[cn].use_nr = 0;
					ch[cn].misc_action = 0;
					ch[cn].dir = DX_DOWN;
					clear_map_buffs(cn, 0);
					remove_buff(cn, SK_OPPRESSED);
					
					for (m = 0; m<MAXITEMS; m++)
					{
						if (IS_SANEUSEDITEM(in2 = ch[cn].item[m]) && (it[in2].flags & IF_IS_KEY) && it[in2].data[1])
						{
							ch[cn].item[m] = 0;
						//	ch[cn].item_lock[m] = 0;
							it[in2].used = USE_EMPTY;
							do_char_log(cn, 1, "Your %s vanished.\n", it[in2].reference);
						}
					}
					
					for (m = 0; m<4; m++) ch[cn].enemy[m] = 0;
					remove_enemy(cn);
				}
				else if (bu[in].temp!=SK_ZEPHYR2)
				{
					do_char_log(cn, 0, "%s ran out.\n", bu[in].name);
				}
				if (bu[in].temp==SK_EXHAUST && bu[in].data[0]==SK_BLAST)
					do_random_blast(cn, bu[in].data[1]);
				bu[in].used = USE_EMPTY;
				ch[cn].spell[n] = 0;
				do_update_char(cn);
			}
		}
	}
	
	// World R
	if (worldR)
	{	// 2% per frame = 40% per 20 frames  (100.000 / 50 = 2000 * 20 = 40.000)
		ch[cn].a_end -= (ch[cn].a_end / (50 + (end + race_res + endmult*3)/4));
	}
	// Moon R
	if (moonR)
	{	// 0.01% per frame = 0.2% per 20 frames  (100.000 / 10000 = 10 * 20 = 0.200)
		ch[cn].a_mana -= (ch[cn].a_mana /10000) * ch[cn].mana[4] / 50;
	}
	
	if (IS_PLAYER(cn))
	{
		if (uwater)
		{
			int waterlifeloss = spell_metabolism(250, get_target_metabolism(cn));
			
			if (in = has_buff(cn, SK_CALM)) 
				waterlifeloss = waterlifeloss * (1000 - bu[in].data[4]) / 1000;
			
			// Amulet of Waterbreathing quarters the result
			if (get_neck(cn, IT_BREATHAMMY))
				waterlifeloss /= 4;
			
			if (cloakofshadows)
			{
				ch[cn].a_mana 	-= waterlifeloss/10;
				waterlifeloss 	-= waterlifeloss/10;
			}
			
			ch[cn].a_hp -= waterlifeloss + gothp;
			
			if (ch[cn].a_hp<500)
			{
				if (!(mf1 & MF_ARENA) && try_lucksave(cn))
					do_lucksave(cn, "watery depths");
				else
					do_char_killed(0, cn, 0);
			}
		}
		if (ch[cn].data[11]>0)
		{
			int petri = ch[cn].data[11];
			
			if (petri >= 100)
				petri = petri/100;
			
			ch[cn].data[11] -= petri;
			
			if (cloakofshadows)
			{
				ch[cn].a_mana 	-= petri/10;
				petri 			-= petri/10;
			}
			
			ch[cn].a_hp -= petri;
			
			if (ch[cn].a_hp<500)
			{
				if (!(mf1 & MF_ARENA) && try_lucksave(cn))
					do_lucksave(cn, "lingering damage");
				else
					do_char_killed(0, cn, 0);
			}
		}
	}
	
	if (ch[cn].player && get_gear(cn, IT_TW_GULA))
	{
		if (ch[cn].a_hp>1500) ch[cn].a_hp -= 200 + gothp;
	}
	if (ch[cn].player && get_gear(cn, IT_TW_AVARITIA))
	{
		if (ch[cn].a_end>1500) ch[cn].a_end -= 200;	
	}
	if (ch[cn].player && get_gear(cn, IT_TW_IRA))
	{
		if (ch[cn].a_mana>1500) ch[cn].a_mana -= 200;
	}
	
	// force to sane values
	if (ch[cn].a_hp>ch[cn].hp[5] * 1000)		ch[cn].a_hp   = ch[cn].hp[5]   * 1000;
	if (ch[cn].a_end>ch[cn].end[5] * 1000)		ch[cn].a_end  = ch[cn].end[5]  * 1000;
	if (ch[cn].a_mana>ch[cn].mana[5] * 1000)	ch[cn].a_mana = ch[cn].mana[5] * 1000;
	
	if (ch[cn].a_end<0) 	ch[cn].a_end = 0;
	if (ch[cn].a_mana<0) 	ch[cn].a_mana = 0;

	// item tear and wear
	if (ch[cn].used==USE_ACTIVE && (ch[cn].flags & (CF_PLAYER)))
	{
		char_item_expire(cn);
	}

	prof_stop(8, prof);
}

int attrib_needed(int v, int diff)
{
	return(v * v * v * diff / 20);
}

int hp_needed(int v, int diff)
{
	return(v * diff);
}

//int end_needed(int v, int diff)
//{
//	return(v * diff / 2);
//}

int mana_needed(int v, int diff)
{
	return(v * diff);
}

int skill_needed(int v, int diff)
{
	return(max(v, v * v * v * diff / 40));
}

int do_raise_attrib(int cn, int nr)
{
	int p, v;

	v = B_AT(cn, nr);

	if (!v || v>=ch[cn].attrib[nr][2])
	{
		return 0;
	}

	p = attrib_needed(v, ch[cn].attrib[nr][3]);

	if (p>ch[cn].points)
	{
		return 0;
	}

	ch[cn].points -= p;
	B_AT(cn, nr)++;

	do_update_char(cn);
	return 1;
}

int do_raise_hp(int cn)
{
	int p, v;

	v = ch[cn].hp[0];

	if (!v || v>=ch[cn].hp[2])
	{
		return 0;
	}

	p = hp_needed(v, ch[cn].hp[3]);
	if (p>ch[cn].points)
	{
		return 0;
	}

	ch[cn].points -= p;
	ch[cn].hp[0]++;

	do_update_char(cn);
	return 1;
}

int do_lower_hp(int cn)
{
	int p, v;

	if (ch[cn].hp[0]<11)
	{
		return 0;
	}

	ch[cn].hp[0]--;

	v = ch[cn].hp[0];

	p = hp_needed(v, ch[cn].hp[3]);

	ch[cn].points_tot -= p;

	do_update_char(cn);
	return 1;
}

int do_lower_mana(int cn)
{
	int p, v;

	if (ch[cn].mana[0]<11)
	{
		return 0;
	}

	ch[cn].mana[0]--;

	v = ch[cn].mana[0];

	p = mana_needed(v, ch[cn].mana[3]);

	ch[cn].points_tot -= p;

	do_update_char(cn);
	return 1;
}

/*
int do_raise_end(int cn)
{
	int p, v;

	v = ch[cn].end[0];

	if (!v || v>=ch[cn].end[2])
	{
		return 0;
	}

	p = end_needed(v, ch[cn].end[3]);
	if (p>ch[cn].points)
	{
		return 0;
	}

	ch[cn].points -= p;
	ch[cn].end[0]++;

	do_update_char(cn);
	return 1;
}
*/

int do_raise_mana(int cn)
{
	int p, v;

	v = ch[cn].mana[0];

	if (!v || v>=ch[cn].mana[2])
	{
		return 0;
	}

	p = mana_needed(v, ch[cn].mana[3]);
	if (p>ch[cn].points)
	{
		return 0;
	}

	ch[cn].points -= p;
	ch[cn].mana[0]++;

	do_update_char(cn);
	return 1;
}

int do_raise_skill(int cn, int nr)
{
	int p, v;

	v = B_SK(cn, nr);

	if (!v || v>=ch[cn].skill[nr][2])
	{
		return 0;
	}

	p = skill_needed(v, ch[cn].skill[nr][3]);

	if (p>ch[cn].points)
	{
		return 0;
	}

	ch[cn].points -= p;
	B_SK(cn, nr)++;

	do_update_char(cn);
	return 1;
}

int do_item_value(int in)
{
	if (in<1 || in>=MAXITEM)
	{
		return 0;
	}
	return it[in].value;
}

int do_item_bsvalue(int cn, int temp)
{
	int n, in, v;
	
	if (!IS_SANEITEMPLATE(temp))
	{
		return 0;
	}
	
	if (temp==IT_CORRUPTOR)
	{
		v = 1;
		for (n=0;n<MAXITEMS;n++)
		{
			if ((in = ch[cn].item[n]) && IS_CORRUPTOR(in)) v += max(1, it[in].stack);
		}
		if ((in = ch[cn].citem) && IS_CORRUPTOR(in)) v += max(1, it[in].stack);
		if (v)
		{
			return it_temp[temp].data[9] * v;
		}
	}
	
	return it_temp[temp].data[9];
}

int do_item_xpvalue(int cn, int temp, int skl)
{
	int n, m = 0, mm = 0, p;
	
	if (!IS_SANEITEMPLATE(temp))
	{
		return 0;
	}
	
	p = it_temp[temp].data[9];
	
	if (p == 750000)		// Attribute scrolls
	{
		for (n=0; n<5; n++) 
		{
			m += ch[cn].attrib[n][1]; // m is how many scrolls have been used already
		}
		
		m += 25;
		
		p = m*m*m*m*3;
	}
	else if (p == 325000)	// Skill scrolls
	{
		for (n=0;n<50;n++) 
		{ 
			m += ch[cn].skill[n][1]; 			// m is used scrolls
		}
		if (skl > -1 && skl < 50)
		{
			mm = ch[cn].skill[skl][1];			// mm is used scrolls of the matching skill
		}
		
		m  += 33;
		mm += 17;
		
		p = m*m*m*5 + mm*mm*mm*mm*9;
	}
	
	return p;
}

int do_item_osvalue(int cn)
{
	int p;
	
	p = st_skill_pts_all(ch[cn].os_tree);
	
	if (p >= st_skill_pts_all(ch[cn].tree_points)) return 0;
	
	p++;
	
	return (p*p*p*345+1665);
}

void do_appraisal(int cn, int in)
{
	int v = it[in].value/2;
	
	if (!(ch[cn].flags & CF_APPRAISE)) return;
	if (ch[cn].flags & CF_APPR_OFF) return;
	if (!(it_temp[it[in].temp].flags & IF_TAKE)) return;
	if (!v) return;
	
	do_char_log(cn, 6, "You estimate this could sell for %dG %dS.\n", v/100, v%100);
}

void do_chestsense(int cn, int in)
{
	int one_hou = TICKS*60*60;
	int one_min = TICKS*60;
	
	if (it[in].driver!=42 && it[in].driver!=59 && it[in].driver!=103 && it[in].driver!=121) return;
	if (!it[in].active) return;

	// Remaining duration is higher than 30 minutes
	if (it[in].active > one_min*30)
	{
		do_char_log(cn, 6, "This chest will refill in %d hours and %d minutes.\n", it[in].active/one_hou, (it[in].active/one_min)%60);
	}
	else
	{
		do_char_log(cn, 6, "This chest will refill in %d minutes and %d seconds.\n", it[in].active/one_min, (it[in].active/TICKS)%60);
	}
}

void do_look_item(int cn, int in)
{
	int n, flag = 0, act;

	if (it[in].active)
	{
		act = 1;
	}
	else
	{
		act = 0;
	}

	for (n = 0; n<MAXITEMS; n++)
	{
		if (ch[cn].item[n]==in)
		{
			flag = 1;
			break;
		}
	}
	for (n = 0; n<20 && !flag; n++)
	{
		if (ch[cn].worn[n]==in)
		{
			flag = 1;
			break;
		}
	}
	for (n = 0; n<12 && !flag; n++)
	{
		if (ch[cn].alt_worn[n]==in)
		{
			flag = 1;
			break;
		}
	}
	if (!flag && !do_char_can_see_item(cn, in))
	{
		return;
	}

	if (it[in].flags & IF_LOOKSPECIAL)
	{
		look_driver(cn, in);
	}
	else if (it[in].driver==2) // Door
	{
		look_door(cn, in);
	}
	else
	{
		do_char_log(cn, 1, "%s\n", it[in].description);
		do_chestsense(cn, in);
		look_item_details(cn, in);
		// Remote scan of tombstones
		if (it[in].temp == IT_TOMBSTONE && it[in].data[0])
		{
			do_ransack_corpse(cn, it[in].data[0], " * Carries a%s.\n");
		}
		if (it[in].driver==57)
		{
			int percent;

			percent = min(100, 100 * (ch[cn].points_tot / 10) / (it[in].data[4] + 1));

			if (percent<50)
			{
				do_char_log(cn, 2, "You sense that it's far too early in your career to touch this pole.\n");
			}
			else if (percent<70)
			{
				do_char_log(cn, 2, "You sense that it's too early in your career to touch this pole.\n");
			}
			else if (percent<90)
			{
				do_char_log(cn, 2, "You sense that it's a bit early in your career to touch this pole.\n");
			}
		}
	}
}

int barter(int cn, int in, int flag) // flag=1 merchant is selling, flag=0 merchant is buying
{
	int pr, opr, brt = 0;
	int ctrank = 0;
		
	opr = do_item_value(in);
	
	// Hack for contracts
	if (flag && it[in].temp == MCT_CONTRACT)
	{
		ctrank = getrank(cn)-6;
		opr = opr * 10;
		opr = opr + opr * getrank(cn) / 9;
	}
	
	if (flag || !(it[in].flags & IF_NOMARKET))
	{
		brt = M_SK(cn, SK_ECONOM);
	}

	if (flag)
	{
		pr = (opr * 4 - (opr * brt) / 150)/2;
		if (pr<opr)
		{
			pr = opr;
		}
	}
	else
	{
		
		pr = opr / 2 + (opr * brt) / 600;
		if (pr>opr)
		{
			pr = opr;
		}
	}

	return max(1, pr);
}

void do_shop_char(int cn, int co, int nr)
{
	int in, pr, in2, flag = 0, tmp, n, stk, orgstk=0, orgval=0;
	unsigned char buf[256];
	int sk = -1, m = 0;

	if (co<=0 || co>=MAXCHARS || nr<0 || nr>=186)
	{
		return;
	}
	if (!(ch[co].flags & CF_MERCHANT) && !(ch[co].flags & CF_BODY) && ch[co].gcm!=9)
	{
		return;
	}
	if (!(ch[co].flags & CF_BODY) && !do_char_can_see(cn, co, 0) && ch[co].gcm!=9)
	{
		return;
	}
	if ((ch[co].flags & CF_BODY) && abs(ch[cn].x - ch[co].x) + abs(ch[cn].y - ch[co].y)>1)
	{
		return;
	}

	if ((in = ch[cn].citem)!=0 && (ch[co].flags & CF_MERCHANT))
	{
		if (in & 0x80000000)
		{
			do_char_log(cn, 0, "You want to sell money? Weird!\n");
			return;
		}
		
		// Reset items in shop to drivers 0~9
		for (n=0;n<10;n++)
		{
			in2 = ch[co].data[n];
			if ((it[in].flags & IF_ARMORS)   && (it_temp[in2].flags & IF_ARMORS))	flag = 1;
			if ((it[in].flags & IF_WEAPON)   && (it_temp[in2].flags & IF_WEAPON))	flag = 1;
			if ((it[in].flags & IF_JEWELERY) && (it_temp[in2].flags & IF_JEWELERY))	flag = 1;
			if ((it[in].flags & IF_MAGIC)    && (it_temp[in2].flags & IF_MAGIC))	flag = 1;
			if ((it[in].flags & IF_BOOK)     && (it_temp[in2].flags & IF_BOOK))		flag = 1;
			if ((it[in].flags & IF_MISC)     && (it_temp[in2].flags & IF_MISC))		flag = 1;
			if ((it[in].flags & IF_GEMSTONE) && (it_temp[in2].flags & IF_GEMSTONE))	flag = 1;
		}
		
		if (ch[co].flags & CF_BSPOINTS)
		{
			if (ch[co].temp == CT_TACTICIAN)
			{
				if (it[in].data[9] && it[in].data[8]==2)
				{
					ch[cn].citem = 0;
					ch[cn].bs_points += it[in].data[9]/5;

					chlog(cn, "Refunded %s", it[in].name);
					do_char_log(cn, 1, "You refunded the %s for %d points.\n", it[in].reference,  it[in].data[9]/5);
					
					it[in].x = 0;
					it[in].y = 0;
					it[in].carried = 0;
				}
				else
				{
					do_char_log(cn, 0, "%s doesn't buy those.\n", ch[co].name);
				}
			}
			else
			{
				do_char_log(cn, 0, "%s doesn't buy things.\n", ch[co].name);
			}
			return;
		}
		
		if (!flag && ch[co].temp != CT_CONTRACTOR)
		{
			do_char_log(cn, 0, "%s doesn't buy those.\n", ch[co].name);
			return;
		}
		pr = barter(cn, in, 0);

		if (ch[co].gold<pr/2)
		{
			do_char_log(cn, 0, "%s cannot afford that.\n", ch[co].reference);
			return;
		}
		ch[cn].citem = 0;

		ch[cn].gold += pr;
		
		if (ch[co].temp == CT_CONTRACTOR)
		{
			god_take_from_char(in, cn);
		}
		else
		{
			god_give_char(in, co);
		}
		chlog(cn, "Sold %s", it[in].name);
		do_char_log(cn, 1, "You sold a %s for %dG %dS.\n", it[in].reference, pr / 100, pr % 100);

		tmp = it[in].temp;
		if (tmp>0 && tmp<MAXTITEM)
		{
			it_temp[tmp].t_sold++;
		}
	}
	else
	{
		if (nr<62 || nr>=124)
		{
			stk = 0;
			if (nr>=124)
			{
				nr -= 124;
				stk = 10;
			}
			if (nr<40)
			{
				if ((in = ch[co].item[nr])!=0)
				{
					int in2 = 0;
					if (ch[co].flags & CF_BSPOINTS)
					{
						// Stronghold items reflected by player stats
						if (ch[co].temp == CT_TACTICIAN || ch[co].temp == get_nullandvoid(0))
						{
							in2 = change_bs_shop_item(cn, it[in].temp);
							if (!in2) return;
							pr = do_item_bsvalue(cn, in2);
							if (ch[cn].bs_points<pr)
							{
								do_char_log(cn, 0, "You cannot afford that.\n");
								return;
							}
						}
						// Casino items
						else if (ch[co].temp == CT_JESSICA)
						{
							// Random selection upon purchase
							in2 = change_casino_shop_item(it[in].temp);
							if (!in2) return;
							pr = do_item_bsvalue(cn, it[in].temp);
							if (ch[cn].tokens<pr)
							{
								do_char_log(cn, 0, "You cannot afford that.\n");
								return;
							}
						}
						// Contract point items
						else if (ch[co].temp == CT_ADHERENT || ch[co].temp == get_nullandvoid(1))
						{
							in2 = change_bs_shop_item(cn, it[in].temp);
							if (!in2) return;
							pr = do_item_bsvalue(cn, in2);
							if (in2 == IT_OS_SP)
							{
								pr = do_item_osvalue(cn);
								if (!pr) return;
							}
							if (ch[cn].os_points<pr)
							{
								do_char_log(cn, 0, "You cannot afford that.\n");
								return;
							}
						}
						// Experience point items
						else if (ch[co].temp == CT_EALDWULF || ch[co].temp == CT_ZANA)
						{
							in2 = change_bs_shop_item(cn, it[in].temp);
							if (!in2) return;
							if (ch[co].temp == CT_EALDWULF)
							{
								pr = do_item_xpvalue(cn, in2, -1);
							}
							else
							{
								if ((sk = change_xp_shop_item(cn, nr)) == -1) return;
								if (sk > -1 && ch[cn].skill[sk][1] >= 10) return;
								pr = do_item_xpvalue(cn, in2, sk);
							}
							if (ch[cn].points<pr)
							{
								do_char_log(cn, 0, "You cannot afford that.\n");
								return;
							}
						}
						else return;
					}
					else if (ch[co].flags & CF_MERCHANT)
					{
						int canstk = 0;

						for (n = 0; n<10; n++)
						{
							if (it[in].temp==ch[co].data[n])
								canstk++;
						}
						
						if (stk && (it[in].flags & IF_STACKABLE) && canstk)
						{
							pr = barter(cn, in, 1)*stk;
						}
						else
						{
							pr = barter(cn, in, 1);
							stk = 0;
						}
						if (ch[cn].gold<pr)
						{
							do_char_log(cn, 0, "You cannot afford that.\n");
							return;
						}
						else if (stk && (it[in].flags & IF_STACKABLE) && canstk)
						{
							orgstk		  = it[in].stack;
							orgval        = it[in].value;
							it[in].stack  = stk;
							it[in].value *= stk;
							it[in].flags |= IF_UPDATE;
						}
					}
					else
					{
						pr = 0;
					}
					if ((ch[co].flags & CF_BSPOINTS) && !(ch[co].flags & CF_BODY))
					{
						if (ch[co].temp == CT_TACTICIAN || ch[co].temp == get_nullandvoid(0))
						{
							// Checks for a magic item variant
							in2 = get_special_item(cn, in2, 0, 0, 0);
							
							if (god_give_char(in2, cn))
							{
								ch[cn].bs_points -= pr;
								it[in2].data[8] = 2;
								
								if (it[in2].driver==48) it[in2].stack = it[in2].data[2];
								
								if (ch[co].temp == CT_NULLAN) 
									god_take_from_char(in, co);

								chlog(cn, "Bought %s", it[in2].name);
								if (!(ch[cn].flags & CF_SYS_OFF))
									do_char_log(cn, 1, "You bought a %s for %d Points.\n", it[in2].reference, pr);

								tmp = it[in2].temp;
								if (tmp>0 && tmp<MAXTITEM)
								{
									it_temp[tmp].t_bought++;
								}
							}
							else
							{
								do_char_log(cn, 0, "You cannot buy the %s because your inventory is full.\n", it[in].reference);
							}
						}
						else if (ch[co].temp == CT_JESSICA)
						{
							in2 = get_special_item(cn, in2, 0, 0, 0);
							
							if (god_give_char(in2, cn))
							{
								ch[cn].tokens -= pr;
								it[in2].data[8] = 3;
								
								if (it[in2].driver==48) it[in2].stack = it[in2].data[2];

								chlog(cn, "Bought %s", it[in2].name);
								if (!(ch[cn].flags & CF_SYS_OFF))
									do_char_log(cn, 1, "You bought a %s for %d Tokens.\n", it[in2].reference, pr);

								tmp = it[in2].temp;
								if (tmp>0 && tmp<MAXTITEM)
								{
									it_temp[tmp].t_bought++;
								}
							}
							else
							{
								do_char_log(cn, 0, "You cannot buy the %s because your inventory is full.\n", it[in].reference);
							}
						}
						else if (ch[co].temp == CT_ADHERENT || ch[co].temp == get_nullandvoid(1))
						{
							in2 = get_special_item(cn, in2, 0, 0, 0);
							
							if (god_give_char(in2, cn))
							{
								ch[cn].os_points -= pr;
								it[in2].data[8] = 4;
								
								if (it[in2].driver==48) it[in2].stack = it[in2].data[2];
								
								if (ch[co].temp == CT_NULLAN) 
									god_take_from_char(in, co);

								chlog(cn, "Bought %s", it[in2].name);
								if (!(ch[cn].flags & CF_SYS_OFF))
									do_char_log(cn, 1, "You bought a %s for %d Points.\n", it[in2].reference, pr);

								tmp = it[in2].temp;
								if (tmp>0 && tmp<MAXTITEM)
								{
									it_temp[tmp].t_bought++;
								}
								// Auto-use scroll
								if (it[in2].driver==134) use_driver(cn, in2, 1);
							}
							else
							{
								do_char_log(cn, 0, "You cannot buy the %s because your inventory is full.\n", it[in].reference);
							}
						}
						else if (ch[co].temp == CT_EALDWULF || ch[co].temp == CT_ZANA)
						{
							in2 = get_special_item(cn, in2, 0, 0, 0);
							if (god_give_char(in2, cn))
							{
								ch[cn].points -= pr;
								if (sk) it[in2].data[1] = sk;
								it[in2].data[8] = 5;
								
								if (it[in2].driver==48) it[in2].stack = it[in2].data[2];

								chlog(cn, "Bought %s", it[in2].name);
								if (!(ch[cn].flags & CF_SYS_OFF))
									do_char_log(cn, 1, "You bought a %s for %d Points.\n", it[in2].reference, pr);

								tmp = it[in2].temp;
								if (tmp>0 && tmp<MAXTITEM)
								{
									it_temp[tmp].t_bought++;
								}
								// Super hacky auto-use of scrolls
								if (it[in2].driver==110)
								{
									m = 0;
									if (it[in2].data[0] == 5)
									{
										m = ch[cn].skill[sk][1];
									}
									else
									{
										for (n=0;n<5;n++) m += ch[cn].attrib[n][1];
									}
									if (m < 10)
									{
										use_driver(cn, in2, 1);
									}
								}
							}
							else
							{
								do_char_log(cn, 0, "You cannot buy the %s because your inventory is full.\n", it[in].reference);
							}
						}
					}
					else
					{
						// Buying/taking normally
						god_take_from_char(in, co);
						
						if (god_give_char(in, cn))
						{
							if (ch[co].flags & CF_MERCHANT)
							{
								ch[cn].gold = max(0, ch[cn].gold - pr);
								
								if (it[in].driver==48) it[in].stack = it[in].data[2];

								chlog(cn, "Bought %s", it[in].name);
								if (!(ch[cn].flags & CF_SYS_OFF))
								{
									if (stk && (it[in].flags & IF_STACKABLE))
										do_char_log(cn, 1, "You bought %d %s's for %dG %dS.\n", stk, it[in].reference, pr / 100, pr % 100);
									else
										do_char_log(cn, 1, "You bought a %s for %dG %dS.\n", it[in].reference, pr / 100, pr % 100);
								}

								tmp = it[in].temp;
								if (tmp>0 && tmp<MAXTITEM)
								{
									it_temp[tmp].t_bought++;
								}
							}
							else
							{
								chlog(cn, "Took %s", it[in].name);
								if (!(ch[cn].flags & CF_SYS_OFF))
									do_char_log(cn, 1, "You took a %s.\n", it[in].reference);
								if ((ch[co].flags & CF_BODY) && ch[co].item[40])
									qsort(ch[co].item, MAXITEMS, sizeof(int), qsort_proc);
								//chlog(cn, "  Took the item...");
								if (ch[co].gcm==9)
								{
									chlog(cn, "  Triggered special chest");
									god_destroy_items(co);
									do_force_recall(cn);
									ch[co].used = USE_EMPTY; // do_char_killed(0, co, 0);
									/*
									buf[0] = SV_CLOSESHOP;
									xsend(ch[cn].player, buf, 1);
									*/
								}
							}
						}
						else
						{
							if (orgstk)
							{
								it[in].stack  = orgstk;
								it[in].value  = orgval;
								it[in].flags |= IF_UPDATE;
							}
							god_give_char(in, co);
							if (ch[co].flags & CF_MERCHANT)
							{
								do_char_log(cn, 0, "You cannot buy the %s because your inventory is full.\n", it[in].reference);
							}
							else
							{
								do_char_log(cn, 0, "You cannot take the %s because your inventory is full.\n", it[in].reference);
							}
						}
					}
				}
			}
			else if (nr<60)
			{
				if ((ch[co].flags & CF_BODY) && (in = ch[co].worn[nr - 40])!=0)
				{
					god_take_from_char(in, co);
					if (god_give_char(in, cn))
					{
						if (it[in].driver==48) it[in].stack = it[in].data[2];
						chlog(cn, "Took %s", it[in].name);
						if (!(ch[cn].flags & CF_SYS_OFF))
							do_char_log(cn, 1, "You took a %s.\n", it[in].reference);
					}
					else
					{
						god_give_char(in, co);
						do_char_log(cn, 0, "You cannot take the %s because your inventory is full.\n", it[in].reference);
					}
				}
			}
			else if (nr==60)
			{
				if ((ch[co].flags & CF_BODY) && (in = ch[co].citem)!=0)
				{
					god_take_from_char(in, co);
					if (god_give_char(in, cn))
					{
						if (it[in].driver==48) it[in].stack = it[in].data[2];
						chlog(cn, "Took %s", it[in].name);
						if (!(ch[cn].flags & CF_SYS_OFF))
							do_char_log(cn, 1, "You took a %s.\n", it[in].reference);
					}
					else
					{
						god_give_char(in, co);
						do_char_log(cn, 0, "You cannot take the %s because your inventory is full.\n", it[in].reference);
					}
				}
			}
			else
			{
				if ((ch[co].flags & CF_BODY) && ch[co].gold)
				{
					ch[cn].gold += ch[co].gold;
					chlog(cn, "Took %dG %dS", ch[co].gold / 100, ch[co].gold % 100);
					if (!(ch[cn].flags & CF_SYS_OFF))
						do_char_log(cn, 1, "You took %dG %dS.\n", ch[co].gold / 100, ch[co].gold % 100);
					ch[co].gold = 0;
				}
			}
		}
		else
		{
			nr -= 62;

			if (nr<40)
			{
				if ((in = ch[co].item[nr])!=0)
				{
					if ((ch[co].flags & CF_BSPOINTS) && !(ch[co].flags & CF_BODY))
					{
						int in2;
						if (ch[co].temp == CT_TACTICIAN || ch[co].temp == get_nullandvoid(0))
						{
							in2 = change_bs_shop_item(cn, it[in].temp);
							if (!in2) return;
							do_char_log(cn, 1, "%s:\n", it_temp[in2].name);
							do_char_log(cn, 1, "%s\n", it_temp[in2].description);
						}
						else if (ch[co].temp == CT_JESSICA)
						{
							do_char_log(cn, 1, "%s:\n", it[in].name);
							do_char_log(cn, 1, "%s\n", it[in].description);
						}
						else if (ch[co].temp == CT_ADHERENT || ch[co].temp == get_nullandvoid(1))
						{
							in2 = change_bs_shop_item(cn, it[in].temp);
							if (in2 == IT_OS_SP && do_item_osvalue(cn)==0) return;
							if (!in2) return;
							do_char_log(cn, 1, "%s:\n", it_temp[in2].name);
							do_char_log(cn, 1, "%s\n", it_temp[in2].description);
						}
						else if (ch[co].temp == CT_EALDWULF || ch[co].temp == CT_ZANA)
						{
							in2 = change_bs_shop_item(cn, it[in].temp);
							if (ch[co].temp == CT_ZANA && (sk = change_xp_shop_item(cn, nr)) == -1) return;
							if (ch[co].temp == CT_ZANA && sk > -1 && ch[cn].skill[sk][1] >= 10) return;
							if (!in2) return;
							do_char_log(cn, 1, "%s:\n", it_temp[in2].name);
							do_char_log(cn, 1, "%s\n", it_temp[in2].description);
							if (ch[co].temp == CT_ZANA)
							{
								int mm = 0; m = 0;
								for (n=0; n<50; n++) 
								{
									if (ch[cn].skill[n][0]) m++;
									if (m > nr) break;
								}
								m = 0;
								do_char_log(cn, 8, "Grants an implicit +1 to %s.\n", skilltab[n].name);
								do_char_log(cn, 1, " \n");
								for (n=0;n<50;n++) { m += ch[cn].skill[n][1]; mm = mm + (ch[cn].skill[n][0]?1:0); }
								do_char_log(cn, 6, "You have used %d out of %d greater skill scrolls.\n", m, mm);
							}
						}
						else return;
					}
					else
					{						
						do_char_log(cn, 1, "%s:\n", it[in].name);
						if (it[in].flags & IF_LOOKSPECIAL)
						{
							look_driver(cn, in);
						}
						else
						{
							do_char_log(cn, 1, "%s\n", it[in].description);
							look_item_details(cn, in);
						}
					}
				}
			}
			else if (nr<61)
			{
				if ((ch[co].flags & CF_BODY) && (in = ch[co].worn[nr - 40])!=0)
				{
					if (it[in].flags & IF_LOOKSPECIAL)
					{
						look_driver(cn, in);
					}
					else
					{
						do_char_log(cn, 1, "%s\n", it[in].description);
						look_item_details(cn, in);
					}
				}
			}
			else
			{
				if ((ch[co].flags & CF_BODY) && (in = ch[co].citem)!=0)
				{
					if (it[in].flags & IF_LOOKSPECIAL)
					{
						look_driver(cn, in);
					}
					else
					{
						do_char_log(cn, 1, "%s\n", it[in].description);
						look_item_details(cn, in);
					}
				}
			}
		}
	}
	if ((ch[co].flags & CF_MERCHANT) && !(ch[co].flags & CF_BSPOINTS) && ch[co].temp!=CT_NULLAN && ch[co].temp!=CT_DVOID)
	{
		update_shop(co);
	}
	do_look_char(cn, co, 0, 0, 1);
}

void move_smith_item(int cn)
{
	ch[cn].blacksmith[3] = ch[cn].blacksmith[0];
	ch[cn].blacksmith[0] = 0;
	
	it[ch[cn].blacksmith[3]].flags |= IF_IDENTIFIED;
	
	do_update_char(cn);
}

void smith_mage_item(int in, int spr)
{
	int n;
	
	switch (it[in].sprite[0])
	{
		case IT_SPR_DAGG_STEL:
			it[in].weapon[0]           +=   6;
			it[in].to_parry[0]         +=   2;
			it[in].attrib[AT_WIL][2]    =  18;
			it[in].attrib[AT_AGL][2]    =  14;
			it[in].skill[SK_DAGGER][2] +=  10;
			it[in].power               +=  10;
			it[in].value               *=   4;
			break;
		case IT_SPR_DAGG_GOLD:
			it[in].weapon[0]           +=   6;
			it[in].to_parry[0]         +=   2;
			it[in].attrib[AT_WIL][2]    =  30;
			it[in].attrib[AT_AGL][2]    =  16;
			it[in].skill[SK_DAGGER][2] +=  10;
			it[in].power               +=  10;
			it[in].value               *=   4;
			break;
		case IT_SPR_DAGG_EMER:
			it[in].weapon[0]           +=   6;
			it[in].to_parry[0]         +=   2;
			it[in].attrib[AT_WIL][2]    =  48;
			it[in].attrib[AT_AGL][2]    =  20;
			it[in].skill[SK_DAGGER][2] +=  10;
			it[in].power               +=  10;
			it[in].value               *=   3;
			break;
		case IT_SPR_DAGG_CRYS:
			it[in].weapon[0]           +=   6;
			it[in].to_parry[0]         +=   2;
			it[in].attrib[AT_WIL][2]    =  72;
			it[in].attrib[AT_AGL][2]    =  24;
			it[in].skill[SK_DAGGER][2] +=  10;
			it[in].power               +=  10;
			it[in].value               *=   2;
			break;
		case IT_SPR_DAGG_TITN:
			it[in].weapon[0]           +=   4;
			it[in].attrib[AT_WIL][2]    =  87;
			it[in].attrib[AT_AGL][2]    =  27;
			it[in].skill[SK_DAGGER][2] +=  10;
			it[in].power               +=  15;
			it[in].value = it[in].value * 3/2;
			it[in].atk_speed[0]        +=   3; it[in].cast_speed[0]       +=   3;
			break;
		case IT_SPR_DAGG_DAMA:
			it[in].attrib[AT_WIL][2]    = 102; it[in].attrib[AT_WIL][0]   +=   4;
			it[in].attrib[AT_AGL][2]    =  30; it[in].attrib[AT_AGL][0]   +=   2;
			it[in].value = it[in].value * 4/3;
			it[in].atk_speed[0]        -=   3; it[in].cast_speed[0]       -=   3;
			break;
		//
		case IT_SPR_STAF_STEL:
			it[in].weapon[0]           +=   4;
			it[in].attrib[AT_INT][0]   +=   1;
			it[in].attrib[AT_INT][2]    =  21;
			it[in].attrib[AT_STR][2]    =  14;
			it[in].skill[SK_STAFF][2]  +=  10;
			it[in].power               +=  10;
			it[in].value               *=   4;
			break;
		case IT_SPR_STAF_GOLD:
			it[in].weapon[0]           +=   4;
			it[in].attrib[AT_INT][0]   +=   1;
			it[in].attrib[AT_INT][2]    =  35;
			it[in].attrib[AT_STR][2]    =  16;
			it[in].skill[SK_STAFF][2]  +=  10;
			it[in].power               +=  10;
			it[in].value               *=   4;
			break;
		case IT_SPR_STAF_EMER:
			it[in].weapon[0]           +=   4;
			it[in].attrib[AT_INT][0]   +=   1;
			it[in].attrib[AT_INT][2]    =  56;
			it[in].attrib[AT_STR][2]    =  20;
			it[in].skill[SK_STAFF][2]  +=  10;
			it[in].power               +=  10;
			it[in].value               *=   3;
			break;
		case IT_SPR_STAF_CRYS:
			it[in].weapon[0]           +=   4;
			it[in].attrib[AT_INT][0]   +=   1;
			it[in].attrib[AT_INT][2]    =  84;
			it[in].attrib[AT_STR][2]    =  24;
			it[in].skill[SK_STAFF][2]  +=  10;
			it[in].power               +=  10;
			it[in].value               *=   2;
			break;
		case IT_SPR_STAF_TITN:
			it[in].weapon[0]           +=   2;
			it[in].attrib[AT_INT][2]    = 102;
			it[in].attrib[AT_STR][2]    =  27;
			it[in].skill[SK_STAFF][2]  +=  10;
			it[in].power               +=  15;
			it[in].value = it[in].value * 3/2;
			it[in].atk_speed[0]        +=   3; it[in].cast_speed[0]       +=   3;
			break;
		case IT_SPR_STAF_DAMA:
			it[in].attrib[AT_INT][2]    = 120; it[in].attrib[AT_INT][0]   +=   4;
			it[in].attrib[AT_STR][2]    =  30; it[in].attrib[AT_STR][0]   +=   2;
			it[in].value = it[in].value * 4/3;
			it[in].atk_speed[0]        -=   3; it[in].cast_speed[0]       -=   3;
			break;
		//
		case IT_SPR_SPEA_STEL: // SK_DAGGER SK_STAFF
			it[in].weapon[0]           +=  10;
			it[in].armor[0]            +=   1;
			it[in].attrib[AT_WIL][0]   +=   2;
			it[in].to_hit[0]           +=   2;
			it[in].attrib[AT_WIL][2]    =  22;
			it[in].attrib[AT_STR][2]    =  14;
			it[in].skill[SK_DAGGER][2] +=   8;
			it[in].skill[SK_STAFF][2]  +=   8;
			it[in].power               +=  10;
			it[in].value               *=   4;
			break;
		case IT_SPR_SPEA_GOLD:
			it[in].weapon[0]           +=  10;
			it[in].armor[0]            +=   1;
			it[in].attrib[AT_WIL][0]   +=   2;
			it[in].to_hit[0]           +=   2;
			it[in].attrib[AT_WIL][2]    =  34;
			it[in].attrib[AT_STR][2]    =  16;
			it[in].skill[SK_DAGGER][2] +=   8;
			it[in].skill[SK_STAFF][2]  +=   8;
			it[in].power               +=  10;
			it[in].value               *=   4;
			break;
		case IT_SPR_SPEA_EMER:
			it[in].weapon[0]           +=  10;
			it[in].armor[0]            +=   1;
			it[in].attrib[AT_WIL][0]   +=   2;
			it[in].to_hit[0]           +=   2;
			it[in].attrib[AT_WIL][2]    =  52;
			it[in].attrib[AT_STR][2]    =  20;
			it[in].skill[SK_DAGGER][2] +=   8;
			it[in].skill[SK_STAFF][2]  +=   8;
			it[in].power               +=  10;
			it[in].value               *=   3;
			break;
		case IT_SPR_SPEA_CRYS:
			it[in].weapon[0]           +=  10;
			it[in].armor[0]            +=   1;
			it[in].attrib[AT_WIL][0]   +=   2;
			it[in].to_hit[0]           +=   2;
			it[in].attrib[AT_WIL][2]    =  76;
			it[in].attrib[AT_STR][2]    =  24;
			it[in].skill[SK_DAGGER][2] +=   8;
			it[in].skill[SK_STAFF][2]  +=   8;
			it[in].power               +=  10;
			it[in].value               *=   2;
			break;
		case IT_SPR_SPEA_TITN:
			it[in].weapon[0]           +=   5;
			it[in].attrib[AT_WIL][2]    =  90;
			it[in].attrib[AT_STR][2]    =  27;
			it[in].skill[SK_DAGGER][2] +=   8;
			it[in].skill[SK_STAFF][2]  +=   8;
			it[in].power               +=  15;
			it[in].value = it[in].value * 3/2;
			it[in].atk_speed[0]        +=   3; it[in].cast_speed[0]       +=   3;
			break;
		case IT_SPR_SPEA_DAMA:
			it[in].attrib[AT_WIL][2]    = 105; it[in].attrib[AT_WIL][2]   +=   2;
			                                   it[in].attrib[AT_INT][2]   +=   2;
			                                   it[in].attrib[AT_AGL][2]   +=   1;
			it[in].attrib[AT_STR][2]    =  30; it[in].attrib[AT_STR][2]   +=   1;
			it[in].value = it[in].value * 4/3;
			it[in].atk_speed[0]        -=   3; it[in].cast_speed[0]       -=   3;
			break;
		//
		case IT_SPR_SHIE_STEL: // SK_SHIELD
			it[in].armor[0]            +=   4;
			it[in].attrib[AT_BRV][2]    =  18;
			it[in].skill[SK_SHIELD][2] +=  12;
			it[in].power               +=  10;
			it[in].value               *=   4;
			break;
		case IT_SPR_SHIE_GOLD:
			it[in].armor[0]            +=   4;
			it[in].attrib[AT_BRV][2]    =  28;
			it[in].skill[SK_SHIELD][2] +=  12;
			it[in].power               +=  10;
			it[in].value               *=   4;
			break;
		case IT_SPR_SHIE_EMER:
			it[in].armor[0]            +=   4;
			it[in].attrib[AT_BRV][2]    =  42;
			it[in].skill[SK_SHIELD][2] +=  12;
			it[in].power               +=  10;
			it[in].value               *=   3;
			break;
		case IT_SPR_SHIE_CRYS:
			it[in].armor[0]            +=   4;
			it[in].attrib[AT_BRV][2]    =  60;
			it[in].skill[SK_SHIELD][2] +=  12;
			it[in].power               +=  10;
			it[in].value               *=   2;
			break;
		case IT_SPR_SHIE_TITN:
			it[in].armor[0]            +=   2;
			it[in].attrib[AT_BRV][2]    =  71;
			it[in].skill[SK_SHIELD][2] +=  12;
			it[in].power               +=  15;
			it[in].value               *= 3/2;
			it[in].atk_speed[0]        +=   3; it[in].cast_speed[0]       +=   3;
			break;
		case IT_SPR_SHIE_DAMA:
			it[in].attrib[AT_BRV][2]    =  82; it[in].attrib[AT_BRV][0]   +=   2;
			                                   it[in].attrib[AT_WIL][0]   +=   2;
			                                   it[in].attrib[AT_STR][0]   +=   2;
			it[in].value               *= 4/3;
			it[in].atk_speed[0]        -=   3; it[in].cast_speed[0]       -=   3;
			break;
		//
		case IT_SPR_SWOR_STEL: // SK_SWORD
			it[in].weapon[0]           +=   8;
			it[in].attrib[AT_AGL][2]    =  22;
			it[in].attrib[AT_STR][2]    =  16;
			it[in].skill[SK_SWORD][2]  +=  12;
			it[in].power               +=  10;
			it[in].value               *=   4;
			break;
		case IT_SPR_SWOR_GOLD:
			it[in].weapon[0]           +=   8;
			it[in].attrib[AT_AGL][2]    =  30;
			it[in].attrib[AT_STR][2]    =  22;
			it[in].skill[SK_SWORD][2]  +=  12;
			it[in].power               +=  10;
			it[in].value               *=   4;
			break;
		case IT_SPR_SWOR_EMER:
			it[in].weapon[0]           +=   8;
			it[in].attrib[AT_AGL][2]    =  40;
			it[in].attrib[AT_STR][2]    =  30;
			it[in].skill[SK_SWORD][2]  +=  12;
			it[in].power               +=  10;
			it[in].value               *=   3;
			break;
		case IT_SPR_SWOR_CRYS:
			it[in].weapon[0]           +=   8;
			it[in].attrib[AT_AGL][2]    =  52;
			it[in].attrib[AT_STR][2]    =  40;
			it[in].skill[SK_SWORD][2]  +=  12;
			it[in].power               +=  10;
			it[in].value               *=   2;
			break;
		case IT_SPR_SWOR_TITN:
			it[in].weapon[0]           +=   4;
			it[in].attrib[AT_AGL][2]    =  60;
			it[in].attrib[AT_STR][2]    =  53;
			it[in].skill[SK_SWORD][2]  +=  12;
			it[in].power               +=  15;
			it[in].value               *= 3/2;
			it[in].atk_speed[0]        +=   3; it[in].cast_speed[0]       +=   3;
			break;
		case IT_SPR_SWOR_DAMA:
			                                   it[in].attrib[AT_BRV][0]   +=   2;
			it[in].attrib[AT_AGL][2]    =  68; it[in].attrib[AT_AGL][0]   +=   2;
			it[in].attrib[AT_STR][2]    =  66; it[in].attrib[AT_STR][0]   +=   2;
			it[in].value               *= 4/3;
			it[in].atk_speed[0]        -=   3; it[in].cast_speed[0]       -=   3;
			break;
		//
		case IT_SPR_DUAL_STEL: // SK_DUAL
			it[in].weapon[0]           +=   6;
			it[in].attrib[AT_AGL][2]    =  16;
			it[in].attrib[AT_STR][2]    =  22;
			it[in].skill[SK_DUAL][2]   +=  15;
			it[in].power               +=  10;
			it[in].value               *=   4;
			break;
		case IT_SPR_DUAL_GOLD:
			it[in].weapon[0]           +=   6;
			it[in].attrib[AT_AGL][2]    =  22;
			it[in].attrib[AT_STR][2]    =  30;
			it[in].skill[SK_DUAL][2]   +=  15;
			it[in].power               +=  10;
			it[in].value               *=   4;
			break;
		case IT_SPR_DUAL_EMER:
			it[in].weapon[0]           +=   6;
			it[in].attrib[AT_AGL][2]    =  30;
			it[in].attrib[AT_STR][2]    =  40;
			it[in].skill[SK_DUAL][2]   +=  15;
			it[in].power               +=  10;
			it[in].value               *=   3;
			break;
		case IT_SPR_DUAL_CRYS:
			it[in].weapon[0]           +=   6;
			it[in].attrib[AT_AGL][2]    =  40;
			it[in].attrib[AT_STR][2]    =  52;
			it[in].skill[SK_DUAL][2]   +=  15;
			it[in].power               +=  10;
			it[in].value               *=   2;
			break;
		case IT_SPR_DUAL_TITN:
			it[in].weapon[0]           +=   3;
			it[in].attrib[AT_AGL][2]    =  53;
			it[in].attrib[AT_STR][2]    =  60;
			it[in].skill[SK_DUAL][2]   +=  15;
			it[in].power               +=  15;
			it[in].value = it[in].value * 3/2;
			it[in].atk_speed[0]        +=   3; it[in].cast_speed[0]       +=   3;
			break;
		case IT_SPR_DUAL_DAMA:
			                                   it[in].attrib[AT_BRV][0]   +=   2;
			it[in].attrib[AT_AGL][2]    =  66; it[in].attrib[AT_AGL][0]   +=   2;
			it[in].attrib[AT_STR][2]    =  68; it[in].attrib[AT_STR][0]   +=   2;
			it[in].value               *= 4/3;
			it[in].atk_speed[0]        -=   3; it[in].cast_speed[0]       -=   3;
			break;
		//
		case IT_SPR_CLAW_STEL: // SK_HAND
			it[in].weapon[0]           +=  10;
			it[in].top_damage[0]       +=   6;
			it[in].attrib[AT_AGL][2]    =  25;
			it[in].attrib[AT_STR][2]    =  18;
			it[in].skill[SK_HAND][2]   +=  16;
			it[in].power               +=  10;
			it[in].value               *=   4;
			break;
		case IT_SPR_CLAW_GOLD:
			it[in].weapon[0]           +=  10;
			it[in].top_damage[0]       +=   6;
			it[in].attrib[AT_AGL][2]    =  40;
			it[in].attrib[AT_STR][2]    =  28;
			it[in].skill[SK_HAND][2]   +=  16;
			it[in].power               +=  10;
			it[in].value               *=   4;
			break;
		case IT_SPR_CLAW_EMER:
			it[in].weapon[0]           +=  10;
			it[in].top_damage[0]       +=   6;
			it[in].attrib[AT_AGL][2]    =  60;
			it[in].attrib[AT_STR][2]    =  42;
			it[in].skill[SK_HAND][2]   +=  16;
			it[in].power               +=  10;
			it[in].value               *=   3;
			break;
		case IT_SPR_CLAW_CRYS:
			it[in].weapon[0]           +=  10;
			it[in].top_damage[0]       +=   6;
			it[in].attrib[AT_AGL][2]    =  85;
			it[in].attrib[AT_STR][2]    =  60;
			it[in].skill[SK_HAND][2]   +=  16;
			it[in].power               +=  10;
			it[in].value               *=   2;
			break;
		case IT_SPR_CLAW_TITN:
			it[in].weapon[0]           +=   5;
			it[in].top_damage[0]       +=   2;
			it[in].attrib[AT_AGL][2]    = 100;
			it[in].attrib[AT_STR][2]    =  71;
			it[in].skill[SK_HAND][2]   +=  16;
			it[in].power               +=  15;
			it[in].value = it[in].value * 3/2;
			it[in].atk_speed[0]        +=   3; it[in].cast_speed[0]       +=   3;
			break;
		case IT_SPR_CLAW_DAMA:
			it[in].attrib[AT_AGL][2]    = 100; it[in].attrib[AT_AGL][0]   +=   4;
			it[in].attrib[AT_STR][2]    =  71; it[in].attrib[AT_STR][0]   +=   2;
			it[in].value               *= 4/3;
			it[in].atk_speed[0]        -=   3; it[in].cast_speed[0]       -=   3;
			break;
		//
		case IT_SPR_AXXE_STEL: // SK_AXE
			it[in].weapon[0]           +=  10;
			it[in].attrib[AT_AGL][2]    =  18;
			it[in].attrib[AT_STR][2]    =  22;
			it[in].skill[SK_AXE][2]    +=  16;
			it[in].power               +=  10;
			it[in].value               *=   4;
			break;
		case IT_SPR_AXXE_GOLD:
			it[in].weapon[0]           +=  10;
			it[in].attrib[AT_AGL][2]    =  28;
			it[in].attrib[AT_STR][2]    =  34;
			it[in].skill[SK_AXE][2]    +=  16;
			it[in].power               +=  10;
			it[in].value               *=   4;
			break;
		case IT_SPR_AXXE_EMER:
			it[in].weapon[0]           +=  10;
			it[in].attrib[AT_AGL][2]    =  42;
			it[in].attrib[AT_STR][2]    =  50;
			it[in].skill[SK_AXE][2]    +=  16;
			it[in].power               +=  10;
			it[in].value               *=   3;
			break;
		case IT_SPR_AXXE_CRYS:
			it[in].weapon[0]           +=  10;
			it[in].attrib[AT_AGL][2]    =  60;
			it[in].attrib[AT_STR][2]    =  74;
			it[in].skill[SK_AXE][2]    +=  16;
			it[in].power               +=  10;
			it[in].value               *=   2;
			break;
		case IT_SPR_AXXE_TITN:
			it[in].weapon[0]           +=   6;
			it[in].attrib[AT_AGL][2]    =  71;
			it[in].attrib[AT_STR][2]    =  88;
			it[in].skill[SK_AXE][2]    +=  16;
			it[in].power               +=  15;
			it[in].value = it[in].value * 3/2;
			it[in].atk_speed[0]        +=   3; it[in].cast_speed[0]       +=   3;
			break;
		case IT_SPR_AXXE_DAMA:
			it[in].attrib[AT_AGL][2]    =  82; it[in].attrib[AT_AGL][0]   +=   2;
			it[in].attrib[AT_STR][2]    = 102; it[in].attrib[AT_STR][0]   +=   4;
			it[in].value               *= 4/3;
			it[in].atk_speed[0]        -=   3; it[in].cast_speed[0]       -=   3;
			break;
		//
		case IT_SPR_THSW_STEL: // SK_TWOHAND
			it[in].weapon[0]           +=  12;
			it[in].crit_multi[0]       +=   5;
			it[in].atk_speed[0]        +=   1;
			it[in].attrib[AT_AGL][2]    =  26;
			it[in].attrib[AT_STR][2]    =  20;
			it[in].skill[SK_TWOHAND][2]+=  18;
			it[in].power               +=  10;
			it[in].value               *=   4;
			break;
		case IT_SPR_THSW_GOLD:
			it[in].weapon[0]           +=  12;
			it[in].crit_multi[0]       +=   5;
			it[in].atk_speed[0]        +=   1;
			it[in].attrib[AT_AGL][2]    =  40;
			it[in].attrib[AT_STR][2]    =  32;
			it[in].skill[SK_TWOHAND][2]+=  18;
			it[in].power               +=  10;
			it[in].value               *=   4;
			break;
		case IT_SPR_THSW_EMER:
			it[in].weapon[0]           +=  12;
			it[in].crit_multi[0]       +=   5;
			it[in].atk_speed[0]        +=   1;
			it[in].attrib[AT_AGL][2]    =  58;
			it[in].attrib[AT_STR][2]    =  48;
			it[in].skill[SK_TWOHAND][2]+=  18;
			it[in].power               +=  10;
			it[in].value               *=   3;
			break;
		case IT_SPR_THSW_CRYS:
			it[in].weapon[0]           +=  12;
			it[in].crit_multi[0]       +=   5;
			it[in].atk_speed[0]        +=   1;
			it[in].attrib[AT_AGL][2]    =  80;
			it[in].attrib[AT_STR][2]    =  68;
			it[in].skill[SK_TWOHAND][2]+=  18;
			it[in].power               +=  10;
			it[in].value               *=   2;
			break;
		case IT_SPR_THSW_TITN:
			it[in].weapon[0]           +=   7;
			it[in].attrib[AT_AGL][2]    =  92;
			it[in].attrib[AT_STR][2]    =  80;
			it[in].skill[SK_TWOHAND][2]+=  18;
			it[in].power               +=  15;
			it[in].value = it[in].value * 3/2;
			it[in].atk_speed[0]        +=   3; it[in].cast_speed[0]       +=   3;
			break;
		case IT_SPR_THSW_DAMA:
			it[in].attrib[AT_AGL][2]    = 105; it[in].attrib[AT_AGL][0]   +=   4;
			it[in].attrib[AT_STR][2]    =  92; it[in].attrib[AT_STR][0]   +=   2;
			it[in].value               *= 4/3;
			it[in].atk_speed[0]        -=   3; it[in].cast_speed[0]       -=   3;
			break;
		//
		case IT_SPR_GAXE_STEL: // SK_TWOHAND SK_AXE
			it[in].weapon[0]           +=  14;
			it[in].armor[0]            +=   1;
			it[in].attrib[AT_AGL][2]    =  20;
			it[in].attrib[AT_STR][2]    =  24;
			it[in].skill[SK_AXE][2]    +=  15;
			it[in].skill[SK_TWOHAND][2]+=  15;
			it[in].power               +=  10;
			it[in].value               *=   4;
			break;
		case IT_SPR_GAXE_GOLD:
			it[in].weapon[0]           +=  14;
			it[in].armor[0]            +=   1;
			it[in].attrib[AT_AGL][2]    =  32;
			it[in].attrib[AT_STR][2]    =  40;
			it[in].skill[SK_AXE][2]    +=  15;
			it[in].skill[SK_TWOHAND][2]+=  15;
			it[in].power               +=  10;
			it[in].value               *=   4;
			break;
		case IT_SPR_GAXE_EMER:
			it[in].weapon[0]           +=  14;
			it[in].armor[0]            +=   1;
			it[in].attrib[AT_AGL][2]    =  48;
			it[in].attrib[AT_STR][2]    =  62;
			it[in].skill[SK_AXE][2]    +=  15;
			it[in].skill[SK_TWOHAND][2]+=  15;
			it[in].power               +=  10;
			it[in].value               *=   3;
			break;
		case IT_SPR_GAXE_CRYS:
			it[in].weapon[0]           +=  14;
			it[in].armor[0]            +=   1;
			it[in].attrib[AT_AGL][2]    =  68;
			it[in].attrib[AT_STR][2]    =  90;
			it[in].skill[SK_AXE][2]    +=  15;
			it[in].skill[SK_TWOHAND][2]+=  15;
			it[in].power               +=  10;
			it[in].value               *=   2;
			break;
		case IT_SPR_GAXE_TITN:
			it[in].weapon[0]           +=   6;
			it[in].attrib[AT_AGL][2]    =  80;
			it[in].attrib[AT_STR][2]    = 107;
			it[in].skill[SK_AXE][2]    +=  15;
			it[in].skill[SK_TWOHAND][2]+=  15;
			it[in].power               +=  15;
			it[in].value = it[in].value * 3/2;
			break;
		case IT_SPR_GAXE_DAMA:
			it[in].attrib[AT_AGL][2]    =  92; it[in].attrib[AT_AGL][0]   +=   2;
			it[in].attrib[AT_STR][2]    = 124; it[in].attrib[AT_STR][0]   +=   4;
			it[in].value               *= 4/3;
			break;
		//
		case IT_SPR_HELM_BRNZ:         it[in].armor[0] += 1;
			it[in].attrib[AT_AGL][2] = it[in].attrib[AT_STR][2] = 24;
			it[in].power = 20;         it[in].value =  100;
			break;
		case IT_SPR_HELM_STEL:         it[in].armor[0] += 1;
			it[in].attrib[AT_AGL][2] = it[in].attrib[AT_STR][2] = 34;
			it[in].power = 30;         it[in].value =  250;
			break;
		case IT_SPR_HELM_GOLD:         it[in].armor[0] += 1;
			it[in].attrib[AT_AGL][2] = it[in].attrib[AT_STR][2] = 48;
			it[in].power = 40;         it[in].value =  500;
			break;
		case IT_SPR_HELM_EMER:         it[in].armor[0] += 1;
			it[in].attrib[AT_AGL][2] = it[in].attrib[AT_STR][2] = 64;
			it[in].power = 50;         it[in].value = 1000;
			break;
		case IT_SPR_HELM_CRYS:         it[in].armor[0] += 1;
			it[in].attrib[AT_AGL][2] = it[in].attrib[AT_STR][2] = 84;
			it[in].power = 60;         it[in].value = 2500;
			break;
		case IT_SPR_HELM_TITN:
			it[in].armor[0] += 1;
			for (n=0;n<5;n++) it[in].attrib[n][0] += 1;
			it[in].end[0] += 5;
			it[in].attrib[AT_AGL][2] = it[in].attrib[AT_STR][2] = 105;
			it[in].power = 75;         it[in].value = 5000;
			break;
		case IT_SPR_HELM_CAST:         it[in].armor[0] += 1;
			it[in].attrib[AT_WIL][2] = it[in].attrib[AT_INT][2] = 60;
			it[in].power = 45;         it[in].value = 1200;
			break;
		case IT_SPR_HELM_ADEP:         it[in].armor[0] += 1;
			it[in].mana[0] += 10;      it[in].cast_speed[0] += 1;
			it[in].attrib[AT_WIL][2] = it[in].attrib[AT_INT][2] = 90;
			it[in].power = 60;         it[in].value = 3000;
			break;
		case IT_SPR_HELM_LIZR:         it[in].armor[0] += 2;
			it[in].weapon[0] -= 1;     it[in].top_damage[0] -= 1;
			it[in].spell_apt[0] += 1;  it[in].atk_speed[0] += 2;
			it[in].attrib[AT_WIL][2] = 72;
			it[in].attrib[AT_AGL][2] = 90;
			it[in].attrib[AT_STR][2] = 0;
			it[in].power = 75;         it[in].value = 4000;
			break;
		case IT_SPR_HELM_MIDN:         it[in].armor[0] += 2;
			it[in].move_speed[0] -= 2; it[in].skill[SK_STEALTH][0] -= 2;
			it[in].cool_bonus[0] += 2; it[in].skill[SK_PERCEPT][0] += 2;
			it[in].attrib[AT_WIL][2] = 90;
			it[in].attrib[AT_INT][2] = 0;
			it[in].attrib[AT_AGL][2] = 72;
			it[in].power = 75;         it[in].value = 4000;
			break;
		//
		case IT_SPR_BODY_BRNZ:         it[in].armor[0] += 2;
			it[in].attrib[AT_AGL][2] = it[in].attrib[AT_STR][2] = 24;
			it[in].power = 20;         it[in].value =  100;
			break;
		case IT_SPR_BODY_STEL:         it[in].armor[0] += 2;
			it[in].attrib[AT_AGL][2] = it[in].attrib[AT_STR][2] = 34;
			it[in].power = 30;         it[in].value =  250;
			break;
		case IT_SPR_BODY_GOLD:         it[in].armor[0] += 2;
			it[in].attrib[AT_AGL][2] = it[in].attrib[AT_STR][2] = 48;
			it[in].power = 40;         it[in].value =  500;
			break;
		case IT_SPR_BODY_EMER:         it[in].armor[0] += 2;
			it[in].attrib[AT_AGL][2] = it[in].attrib[AT_STR][2] = 64;
			it[in].power = 50;         it[in].value = 1000;
			break;
		case IT_SPR_BODY_CRYS:         it[in].armor[0] += 2;
			it[in].attrib[AT_AGL][2] = it[in].attrib[AT_STR][2] = 84;
			it[in].power = 60;         it[in].value = 2500;
			break;
		case IT_SPR_BODY_TITN:
			it[in].armor[0] += 2;
			for (n=0;n<5;n++) it[in].attrib[n][0] += 2;
			it[in].end[0] += 10;
			it[in].attrib[AT_AGL][2] = it[in].attrib[AT_STR][2] = 105;
			it[in].power = 75;         it[in].value = 5000;
			break;
		//
		case IT_SPR_BODY_CAST:         it[in].armor[0] += 2;
			it[in].attrib[AT_WIL][2] = it[in].attrib[AT_INT][2] = 60;
			it[in].power = 45;         it[in].value = 1200;
			break;
		case IT_SPR_BODY_ADEP:         it[in].armor[0] += 2;
			it[in].mana[0] += 20;      it[in].cast_speed[0] += 2;
			it[in].attrib[AT_WIL][2] = it[in].attrib[AT_INT][2] = 90;
			it[in].power = 60;         it[in].value = 3000;
			break;
		case IT_SPR_BODY_LIZR:         it[in].armor[0] += 4;
			it[in].weapon[0] -= 2;     it[in].top_damage[0] -= 2;
			it[in].spell_apt[0] += 2;  it[in].atk_speed[0] += 4;
			it[in].attrib[AT_WIL][2] = 72;
			it[in].attrib[AT_AGL][2] = 90;
			it[in].attrib[AT_STR][2] = 0;
			it[in].power = 75;         it[in].value = 4000;
			break;
		case IT_SPR_BODY_MIDN:         it[in].armor[0] += 4;
			it[in].move_speed[0] -= 4; it[in].skill[SK_STEALTH][0] -= 4;
			it[in].cool_bonus[0] += 4; it[in].skill[SK_PERCEPT][0] += 4;
			it[in].attrib[AT_WIL][2] = 90;
			it[in].attrib[AT_INT][2] = 0;
			it[in].attrib[AT_AGL][2] = 72;
			it[in].power = 75;         it[in].value = 4000;
			break;
		//
		default: break;
	}
	
	it[in].sprite[0] = spr;
}

int smith_grade_in(int in)
{
	int n;
	
	if ( n = it[in].orig_temp) ;
	else n = it[in].temp;
	
	switch (n)
	{
		case IT_WP_GLASSSHNK:   return IT_WB_GLASSSHNK;
		case IT_WP_STONEDAGG:   return IT_WB_STONEDAGG;
		case IT_WP_ARGHAKNIFE:  return IT_WB_ARGHAKNIFE;
		case IT_WP_LIFESPRIG:   return IT_WB_LIFESPRIG;
		case IT_WP_SPIDERFANG:  return IT_WB_SPIDERFANG;
		case IT_WP_THERAMSAY:   return IT_WB_THERAMSAY;
		case IT_WP_MAGEMASH:    return IT_WB_MAGEMASH;
		case IT_WP_BLOODLET:    return IT_WB_BLOODLET;
		case IT_WP_CULTPRAYER:  return IT_WB_CULTPRAYER;
		//
		case IT_WP_JANESOBLIT:  return IT_WB_JANESOBLIT;
		case IT_WP_VIOLETGAZE:  return IT_WB_VIOLETGAZE;
		case IT_WP_RATTANBO:    return IT_WB_RATTANBO;
		case IT_WP_SEKHMETS:    return IT_WB_SEKHMETS;
		case IT_WP_SHIVASCEPT:  return IT_WB_SHIVASCEPT;
		case IT_WP_KUROKO:      return IT_WB_KUROKO;
		case IT_WP_PUTRIDIRE:   return IT_WB_PUTRIDIRE;
		//
		case IT_WP_GOLDGLAIVE:  return IT_WB_GOLDGLAIVE;
		case IT_WP_KELPTRID:    return IT_WB_KELPTRID;
		case IT_WP_LIZARDNAGI:  return IT_WB_LIZARDNAGI;
		case IT_WP_WAHUSHIYA:   return IT_WB_WAHUSHIYA;
		case IT_WP_SIGNOFSKUA:  return IT_WB_SIGNOFSKUA;
		case IT_WP_COBALTLANC:  return IT_WB_COBALTLANC;
		//
		case IT_WP_OAKBUCKLER:  return IT_WB_OAKBUCKLER;
		case IT_WP_RUBYKITE:    return IT_WB_RUBYKITE;
		case IT_WP_TEMPHEATER:  return IT_WB_TEMPHEATER;
		case IT_WP_PERIDOTKIT:  return IT_WB_PERIDOTKIT;
		case IT_WP_ROYALTARGE:  return IT_WB_ROYALTARGE;
		case IT_WP_FROSTGLASS:  return IT_WB_FROSTGLASS;
		case IT_WP_ARCHTOWER:   return IT_WB_ARCHTOWER;
		case IT_WP_PHALANX:     return IT_WB_PHALANX;
		case IT_WP_RISINGPHO:   return IT_WB_RISINGPHO;
		//
		case IT_WP_BARBSWORD:   return IT_WB_BARBSWORD;
		case IT_WP_SWRDSTLTH:   return IT_WB_SWRDSTLTH;
		case IT_WP_QARMZI:      return IT_WB_QARMZI;
		case IT_WP_LIZKATANA:   return IT_WB_LIZKATANA;
		case IT_WP_DEFENDER:    return IT_WB_DEFENDER;
		case IT_WP_WHITEODA:    return IT_WB_WHITEODA;
		//
		case IT_WP_SPELLBLADE:  return IT_WB_SPELLBLADE;
		case IT_WP_GLITCLEAVR:  return IT_WB_GLITCLEAVR;
		case IT_WP_BEINESTOC:   return IT_WB_BEINESTOC;
		case IT_WP_LIZWAKIZA:   return IT_WB_LIZWAKIZA;
		case IT_WP_FELLNIGHT:   return IT_WB_FELLNIGHT;
		case IT_WP_SAVEQUEEN:   return IT_WB_SAVEQUEEN;
		case IT_WP_BLACKTAC:    return IT_WB_BLACKTAC;
		//
		case IT_WP_RUSTSPIKES:  return IT_WB_RUSTSPIKES;
		case IT_WP_ANCIENTORN:  return IT_WB_ANCIENTORN;
		case IT_WP_BRASSKNUCK:  return IT_WB_BRASSKNUCK;
		case IT_WP_NEICLAW:     return IT_WB_NEICLAW;
		case IT_WP_LIONSPAWS:   return IT_WB_LIONSPAWS;
		case IT_WP_IVORYKNUCK:  return IT_WB_IVORYKNUCK;
		case IT_WP_SHADTALONS:  return IT_WB_SHADTALONS;
		case IT_WP_CRIMRIP:     return IT_WB_CRIMRIP;
		//
		case IT_WP_SPIKECLUB:   return IT_WB_SPIKECLUB;
		case IT_WP_THEBUTCHER:  return IT_WB_THEBUTCHER;
		case IT_WP_RUBYAXE:     return IT_WB_RUBYAXE;
		case IT_WP_GULLOXI:     return IT_WB_GULLOXI;
		case IT_WP_LIZONO:      return IT_WB_LIZONO;
		case IT_WP_AJAXBLUE:    return IT_WB_AJAXBLUE;
		case IT_WP_PEARLAXE:    return IT_WB_PEARLAXE;
		case IT_WP_CRESSUN:     return IT_WB_CRESSUN;
		//
		case IT_WP_DECOSWORD:   return IT_WB_DECOSWORD;
		case IT_WP_PREISTRAZR:  return IT_WB_PREISTRAZR;
		case IT_WP_LAVA2HND:    return IT_WB_LAVA2HND;
		case IT_WP_GILDSHINE:   return IT_WB_GILDSHINE;
		case IT_WP_BURN2HND:    return IT_WB_BURN2HND;
		case IT_WP_VERDTRUNK:   return IT_WB_VERDTRUNK;
		case IT_WP_ICE2HND:     return IT_WB_ICE2HND;
		case IT_WP_PHANTASM:    return IT_WB_PHANTASM;
		//
		case IT_WP_SERASLAB:    return IT_WB_SERASLAB;
		case IT_WP_HEADSMAN:    return IT_WB_HEADSMAN;
		case IT_WP_GELSAW:      return IT_WB_GELSAW;
		case IT_WP_BARBBATTLE:  return IT_WB_BARBBATTLE;
		case IT_WP_NEONSHINE:   return IT_WB_NEONSHINE;
		case IT_WP_ONYXFRANC:   return IT_WB_ONYXFRANC;
		case IT_WP_BRONCHIT:    return IT_WB_BRONCHIT;
		case IT_WP_VIKINGMALT:  return IT_WB_VIKINGMALT;
		//
		case IT_BOOK_ALCH:      return IT_IMBK_ALCH;
		case IT_BOOK_HOLY:      return IT_IMBK_HOLY;
		case IT_BOOK_INT1:      return IT_IMBK_INT1;
		case IT_BOOK_TRAV:      return IT_IMBK_TRAV;
		case IT_BOOK_ADVA:      return IT_IMBK_ADVA;
		case IT_BOOK_INT2:      return IT_IMBK_INT2;
		case IT_BOOK_SWOR:      return IT_IMBK_SWOR;
		case IT_BOOK_MALT:      return IT_IMBK_MALT;
		case IT_BOOK_DAMO:      return IT_IMBK_DAMO;
		case IT_BOOK_SHIV:      return IT_IMBK_SHIV;
		case IT_BOOK_BISH:      return IT_IMBK_BISH;
		case IT_BOOK_INT3:      return IT_IMBK_INT3;
		case IT_BOOK_GREA:      return IT_IMBK_GREA;
		case IT_BOOK_PROD:      return IT_IMBK_PROD;
		//
		default: break;
	}
	return 0;
}

int smith_cost(int in, int flag)	// 0=whet 1=clay 2=mage 3=grade
{
	int v = 1;
	
	switch (flag)
	{
		case  1: break;
		case  3: v = min(10, max(1, 10 - it[in].power/10)); break;
		default: v = min(10, max(1,      it[in].power/10)); break;
	}
	
	return v;
}

int is_valid_smith_item(int in)
{
	if (it[in].flags & IF_WEAPON) return  1;
	if (it[in].flags & IF_ARMORS)
	{
		if (IS_EQARMS(in) || IS_EQCLOAK(in) || IS_EQFEET(in))
			return -2;
		else
			return -1;
	}
	return 0;
}

int is_valid_clay_item_check(int in, int in2)
{
	if (it[in].sprite[0] == it[in2].sprite[0]) return -1;
	
	if (((it[in].flags & IF_SINGLEAGE) && !(it[in2].flags & IF_SINGLEAGE)) || (!(it[in].flags & IF_SINGLEAGE) && (it[in2].flags & IF_SINGLEAGE)))
		return -2;
	
	if ((it[in].temp==IT_TW_HEAVENS || it[in].orig_temp==IT_SEYANSWORD) && (it[in2].flags & IF_WEAPON))
		return 1;
	
	if (( (it[in].flags & IF_WP_AXE)     &&  (it[in2].flags & IF_WP_AXE)    )  && ( (it[in].flags & IF_WP_TWOHAND) &&  (it[in2].flags & IF_WP_TWOHAND)))
		return 1;
	
	if ((!(it[in].flags & IF_WP_AXE)     && !(it[in2].flags & IF_WP_AXE)    )  && ( (it[in].flags & IF_WP_TWOHAND) &&  (it[in2].flags & IF_WP_TWOHAND)))
		return 1;
	
	if (( (it[in].flags & IF_WP_AXE)     &&  (it[in2].flags & IF_WP_AXE)    )  && (!(it[in].flags & IF_WP_TWOHAND) && !(it[in2].flags & IF_WP_TWOHAND)))
		return 1;
	
	if (( (it[in].flags & IF_WP_STAFF)  &&  (it[in2].flags & IF_WP_STAFF)   )  && ( (it[in].flags & IF_WP_DAGGER) &&  (it[in2].flags & IF_WP_DAGGER)  ))
		return 1;
	
	if ((!(it[in].flags & IF_WP_STAFF)  && !(it[in2].flags & IF_WP_STAFF)   )  && ( (it[in].flags & IF_WP_DAGGER) &&  (it[in2].flags & IF_WP_DAGGER)  ))
		return 1;
	
	if (( (it[in].flags & IF_WP_STAFF)  &&  (it[in2].flags & IF_WP_STAFF)   )  && (!(it[in].flags & IF_WP_DAGGER) && !(it[in2].flags & IF_WP_DAGGER)  ))
		return 1;
	
	if (  (it[in].flags & IF_WP_SWORD)  &&  (it[in2].flags & IF_WP_SWORD)   )
		return 1;
	
	if (  (it[in].flags & IF_OF_DUALSW) &&  (it[in2].flags & IF_OF_DUALSW)  )
		return 1;
	
	if (  (it[in].flags & IF_OF_SHIELD) &&  (it[in2].flags & IF_OF_SHIELD)  )
		return 1;
	
	if (  (it[in].flags & IF_WP_CLAW)   &&  (it[in2].flags & IF_WP_CLAW)    )
		return 1;
	
	if (  (it[in].flags & IF_ARMOR)     &&  (it[in2].flags & IF_ARMOR)      )
	{
		if ((it[in].placement & PL_HEAD)  && (it[in2].placement & PL_HEAD) )
			return 1;
		
		if ((it[in].placement & PL_CLOAK) && (it[in2].placement & PL_CLOAK))
			return 1;
		
		if ((it[in].placement & PL_BODY)  && (it[in2].placement & PL_BODY) )
			return 1;
		
		if ((it[in].placement & PL_ARMS)  && (it[in2].placement & PL_ARMS) )
			return 1;
		
		if ((it[in].placement & PL_FEET)  && (it[in2].placement & PL_FEET) )
			return 1;
	}
	
	return 0;
}

int is_valid_clay_item(int cn, int in, int in2)
{
	int v;
	
	if ((v = is_valid_clay_item_check(in, in2)) > 0) return 1;
	
	if (v ==  0) do_char_log(cn, 0, "That won't work. These items must be the same type.\n");
	if (v == -1) do_char_log(cn, 0, "Those items look awfully similar already, don't they?\n");
	if (v == -2) do_char_log(cn, 0, "That won't work. One of these items is too complex to work with.\n");
	
	return 0;
}

int is_valid_mage_item(int in)
{
	int spr = 0; // Return the next tier's sprite if possible
	
	switch(it[in].sprite[0])
	{
		case IT_SPR_DAGG_STEL: spr = IT_SPR_DAGG_GOLD; break;
		case IT_SPR_STAF_STEL: spr = IT_SPR_STAF_GOLD; break;
		case IT_SPR_SPEA_STEL: spr = IT_SPR_SPEA_GOLD; break;
		case IT_SPR_SHIE_STEL: spr = IT_SPR_SHIE_GOLD; break;
		case IT_SPR_SWOR_STEL: spr = IT_SPR_SWOR_GOLD; break;
		case IT_SPR_DUAL_STEL: spr = IT_SPR_DUAL_GOLD; break;
		case IT_SPR_AXXE_STEL: spr = IT_SPR_AXXE_GOLD; break;
		case IT_SPR_THSW_STEL: spr = IT_SPR_THSW_GOLD; break;
		case IT_SPR_GAXE_STEL: spr = IT_SPR_GAXE_GOLD; break;
		case IT_SPR_CLAW_STEL: spr = IT_SPR_CLAW_GOLD; break;
		//
		case IT_SPR_DAGG_GOLD: spr = IT_SPR_DAGG_EMER; break;
		case IT_SPR_STAF_GOLD: spr = IT_SPR_STAF_EMER; break;
		case IT_SPR_SPEA_GOLD: spr = IT_SPR_SPEA_EMER; break;
		case IT_SPR_SHIE_GOLD: spr = IT_SPR_SHIE_EMER; break;
		case IT_SPR_SWOR_GOLD: spr = IT_SPR_SWOR_EMER; break;
		case IT_SPR_DUAL_GOLD: spr = IT_SPR_DUAL_EMER; break;
		case IT_SPR_AXXE_GOLD: spr = IT_SPR_AXXE_EMER; break;
		case IT_SPR_THSW_GOLD: spr = IT_SPR_THSW_EMER; break;
		case IT_SPR_GAXE_GOLD: spr = IT_SPR_GAXE_EMER; break;
		case IT_SPR_CLAW_GOLD: spr = IT_SPR_CLAW_EMER; break;
		//
		case IT_SPR_DAGG_EMER: spr = IT_SPR_DAGG_CRYS; break;
		case IT_SPR_STAF_EMER: spr = IT_SPR_STAF_CRYS; break;
		case IT_SPR_SPEA_EMER: spr = IT_SPR_SPEA_CRYS; break;
		case IT_SPR_SHIE_EMER: spr = IT_SPR_SHIE_CRYS; break;
		case IT_SPR_SWOR_EMER: spr = IT_SPR_SWOR_CRYS; break;
		case IT_SPR_DUAL_EMER: spr = IT_SPR_DUAL_CRYS; break;
		case IT_SPR_AXXE_EMER: spr = IT_SPR_AXXE_CRYS; break;
		case IT_SPR_THSW_EMER: spr = IT_SPR_THSW_CRYS; break;
		case IT_SPR_GAXE_EMER: spr = IT_SPR_GAXE_CRYS; break;
		case IT_SPR_CLAW_EMER: spr = IT_SPR_CLAW_CRYS; break;
		//
		case IT_SPR_DAGG_CRYS: spr = IT_SPR_DAGG_TITN; break;
		case IT_SPR_STAF_CRYS: spr = IT_SPR_STAF_TITN; break;
		case IT_SPR_SPEA_CRYS: spr = IT_SPR_SPEA_TITN; break;
		case IT_SPR_SHIE_CRYS: spr = IT_SPR_SHIE_TITN; break;
		case IT_SPR_SWOR_CRYS: spr = IT_SPR_SWOR_TITN; break;
		case IT_SPR_DUAL_CRYS: spr = IT_SPR_DUAL_TITN; break;
		case IT_SPR_AXXE_CRYS: spr = IT_SPR_AXXE_TITN; break;
		case IT_SPR_THSW_CRYS: spr = IT_SPR_THSW_TITN; break;
		case IT_SPR_GAXE_CRYS: spr = IT_SPR_GAXE_TITN; break;
		case IT_SPR_CLAW_CRYS: spr = IT_SPR_CLAW_TITN; break;
		//
		case IT_SPR_DAGG_TITN: spr = IT_SPR_DAGG_DAMA; break;
		case IT_SPR_STAF_TITN: spr = IT_SPR_STAF_DAMA; break;
		case IT_SPR_SPEA_TITN: spr = IT_SPR_SPEA_DAMA; break;
		case IT_SPR_SHIE_TITN: spr = IT_SPR_SHIE_DAMA; break;
		case IT_SPR_SWOR_TITN: spr = IT_SPR_SWOR_DAMA; break;
		case IT_SPR_DUAL_TITN: spr = IT_SPR_DUAL_DAMA; break;
		case IT_SPR_AXXE_TITN: spr = IT_SPR_AXXE_DAMA; break;
		case IT_SPR_THSW_TITN: spr = IT_SPR_THSW_DAMA; break;
		case IT_SPR_GAXE_TITN: spr = IT_SPR_GAXE_DAMA; break;
		case IT_SPR_CLAW_TITN: spr = IT_SPR_CLAW_DAMA; break;
		//
		case IT_SPR_DAGG_DAMA: spr = IT_SPR_DAGG_ADAM; break;
		case IT_SPR_STAF_DAMA: spr = IT_SPR_STAF_ADAM; break;
		case IT_SPR_SPEA_DAMA: spr = IT_SPR_SPEA_ADAM; break;
		case IT_SPR_SHIE_DAMA: spr = IT_SPR_SHIE_ADAM; break;
		case IT_SPR_SWOR_DAMA: spr = IT_SPR_SWOR_ADAM; break;
		case IT_SPR_DUAL_DAMA: spr = IT_SPR_DUAL_ADAM; break;
		case IT_SPR_AXXE_DAMA: spr = IT_SPR_AXXE_ADAM; break;
		case IT_SPR_THSW_DAMA: spr = IT_SPR_THSW_ADAM; break;
		case IT_SPR_GAXE_DAMA: spr = IT_SPR_GAXE_ADAM; break;
		case IT_SPR_CLAW_DAMA: spr = IT_SPR_CLAW_ADAM; break;
		//
		case IT_SPR_HELM_BRNZ: spr = IT_SPR_HELM_STEL; break;
		case IT_SPR_BODY_BRNZ: spr = IT_SPR_BODY_STEL; break;
		case IT_SPR_HELM_STEL: spr = IT_SPR_HELM_GOLD; break;
		case IT_SPR_BODY_STEL: spr = IT_SPR_BODY_GOLD; break;
		case IT_SPR_HELM_GOLD: spr = IT_SPR_HELM_EMER; break;
		case IT_SPR_BODY_GOLD: spr = IT_SPR_BODY_EMER; break;
		case IT_SPR_HELM_EMER: spr = IT_SPR_HELM_CRYS; break;
		case IT_SPR_BODY_EMER: spr = IT_SPR_BODY_CRYS; break;
		case IT_SPR_HELM_CRYS: spr = IT_SPR_HELM_TITN; break;
		case IT_SPR_BODY_CRYS: spr = IT_SPR_BODY_TITN; break;
		case IT_SPR_HELM_TITN: spr = IT_SPR_HELM_ADAM; break;
		case IT_SPR_BODY_TITN: spr = IT_SPR_BODY_ADAM; break;
		//
		case IT_SPR_HELM_CAST: spr = IT_SPR_HELM_ADEP; break;
		case IT_SPR_BODY_CAST: spr = IT_SPR_BODY_ADEP; break;
		case IT_SPR_HELM_ADEP: spr = IT_SPR_HELM_WIZR; break;
		case IT_SPR_BODY_ADEP: spr = IT_SPR_BODY_WIZR; break;
		//
		case IT_SPR_HELM_LIZR: spr = IT_SPR_HELM_AZUR; break;
		case IT_SPR_BODY_LIZR: spr = IT_SPR_BODY_AZUR; break;
		case IT_SPR_HELM_MIDN: spr = IT_SPR_HELM_IVOR; break;
		case IT_SPR_BODY_MIDN: spr = IT_SPR_BODY_IVOR; break;
		//
		default: break;
	}
	return spr;
}

void do_consume_smithstones(int cn, int v)
{
	int in = ch[cn].blacksmith[1];
	
	if (v >= it[in].stack)
	{
		ch[cn].blacksmith[1] = 0;
		it[in].used = USE_EMPTY;
	}
	else
	{
		use_consume_item(cn, in, v);
	}
}

void do_bs_whetstone(int cn, int in, int in2, int flag)
{
	int n, v;
	
	if (!IS_SANEITEM(in))
	{
		do_char_log(cn, 0, "You must give the blacksmith an item to smith first!\n");
		return;
	}
	if (it[in].flags & IF_WHETSTONED)
	{
		do_char_log(cn, 0, "This item has already been improved by a whetstone.\n");
		return;
	}
	
	if (flag > 6 || flag < 4)
	{
		do_char_log(cn, 0, "Sorry?\n");
		return;
	}
	
	n = is_valid_smith_item(in);
	
	if (n == 0 || n == -2)
	{
		do_char_log(cn, 0, "That won't work. This can only be used with weapons, shields, helmets, and body armors.\n");
		return;
	}
	
	if (it[in2].stack < (v = smith_cost(in, 0)))
	{
		do_char_log(cn, 0, "You'll need more whetstones to improve this item! (Need %d).\n", v);
		return;
	}
	
	if (n>0)	// Weapon
	{
		switch (flag)
		{
			case  4:	// Grind		Hit +2 / -1 WV
				do_char_log(cn, 2, "The blacksmith improved the item's Hit Score by 2, at the cost of 1 Weapon Value.\n");
				it[in].to_hit[0]+=2; it[in].to_hit[1]+=2;
				it[in].weapon[0]-=1; it[in].weapon[1]-=1;
				break;
			case  5:	// Hone			Hit +1
				do_char_log(cn, 2, "The blacksmith improved the item's Hit Score by 1.\n");
				it[in].to_hit[0]+=1; it[in].to_hit[1]+=1;
				break;
			default:	// Sharpen		Hit -1 / +2 WV
				do_char_log(cn, 2, "The blacksmith improved the item's Weapon Value by 2, at the cost of 1 Hit Score.\n");
				it[in].weapon[0]+=2; it[in].weapon[1]+=2;
				it[in].to_hit[0]-=1; it[in].to_hit[1]-=1;
				break;
		}
	}
	else		// Armor
	{
		switch (flag)
		{
			case  4:	// Smooth		Par +2 / -1 AV
				do_char_log(cn, 2, "The blacksmith improved the item's Parry Score by 2, at the cost of 1 Armor Value.\n");
				it[in].to_parry[0] +=2; it[in].to_parry[1] +=2;
				it[in].armor[0]    -=1; it[in].armor[1]    -=1;
				break;
			case  5:	// Polish		Par +1
				do_char_log(cn, 2, "The blacksmith improved the item's Parry Score by 1.\n");
				it[in].to_parry[0]+=1; it[in].to_parry[1]+=1;
				break;
			default:	// Thicken		Par -1 / +2 AV
				do_char_log(cn, 2, "The blacksmith improved the item's Armor Value by 2, at the cost of 1 Parry Score.\n");
				it[in].armor[0]   +=2; it[in].armor[1]   +=2;
				it[in].to_parry[0]-=1; it[in].to_parry[1]-=1;
				break;
		}
	}
	it[in].flags |= IF_WHETSTONED;
	char_play_sound(cn, ch[cn].sound + 20, -100, 0);
	
	do_consume_smithstones(cn, v);
	move_smith_item(cn);
}

void do_bs_claystone(int cn, int in, int in2, int in3)
{
	int v;
	
	if (!IS_SANEITEM(in))
	{
		do_char_log(cn, 0, "You must give the blacksmith an item to transform first!\n");
		return;
	}
	if (!IS_SANEITEM(in3))
	{
		do_char_log(cn, 0, "You must give the blacksmith an item to transform this into first!\n");
		return;
	}
	
	if (!is_valid_clay_item(cn, in, in3))
		return;
	
	if (it[in2].stack < (v = smith_cost(in, 1)))
	{
		do_char_log(cn, 0, "You'll need more claystones to transform this item! (Need %d).\n", v);
		return;
	}
	
	it[in].sprite[0] = it[in3].sprite[0];
	it[in].sprite[1] = it[in3].sprite[1];
	ch[cn].blacksmith[2] = 0;
	
	do_char_log(cn, 2, "The blacksmith transformed the appearance if your %s to that of your %s.\n", it[in].name, it[in3].name);
	char_play_sound(cn, ch[cn].sound + 20, -100, 0);
	
	do_consume_smithstones(cn, v);
	move_smith_item(cn);
}

void do_bs_magestone(int cn, int in, int in2)
{
	int v, spr;
	
	if (!IS_SANEITEM(in))
	{
		do_char_log(cn, 0, "You must give the blacksmith a magic item to upgrade first!\n");
		return;
	}
	
	if (!(spr = is_valid_mage_item(in)))
	{
		do_char_log(cn, 0, "This item cannot be upgraded with this stone.\n", v);
		return;
	}
	
	if (it[in2].stack < (v = smith_cost(in, 2)))
	{
		do_char_log(cn, 0, "You'll need more magestones to upgrade this item! (Need %d).\n", v);
		return;
	}
	
	do_char_log(cn, 2, "The blacksmith upgraded the item into one of the next tier.\n");
	char_play_sound(cn, ch[cn].sound + 20, -100, 0);
	smith_mage_item(in, spr);
	
	do_consume_smithstones(cn, v);
	move_smith_item(cn);
}

void do_bs_gradestone(int cn, int in, int in2)
{
	int v, in3;
	
	if (!IS_SANEITEM(in))
	{
		do_char_log(cn, 0, "You must give the blacksmith an item to improve first!\n");
		return;
	}
	
	if (!(in3 = smith_grade_in(in)))
	{
		do_char_log(cn, 0, "This item cannot be improved with this stone.\n", v);
		return;
	}
	
	if (it[in].flags & IF_DIRTY)
	{
		do_char_log(cn, 0, "This item is beyond the blacksmith's ability to improve it.\n");
		return;
	}
	
	if (it[in2].stack < (v = smith_cost(in, 3)))
	{
		do_char_log(cn, 0, "You'll need more gradestones to improved this item! (Need %d).\n", v);
		return;
	}
	
	in3 = god_create_item(in3);
	ch[cn].blacksmith[3] = in3;
	ch[cn].blacksmith[0] = it[in3].x = it[in3].y = 0;
	it[in3].carried = cn;
	
	do_char_log(cn, 2, "The blacksmith improved your %s.\n", it[in].name);
	char_play_sound(cn, ch[cn].sound + 20, -100, 0);
	
	do_consume_smithstones(cn, v);
	do_update_char(cn);
}

void do_blacksmith(int cn, int co, int nr)
{
	int in[4], n, flag=0;
	
	if (co<=0 || co>=MAXCHARS || nr<0 || nr>=24)	return;
	if (!(ch[co].flags & CF_MERCHANT) || !do_char_can_see(cn, co, 0))
	{
		ch[cn].smithnum = 0;
		return;
	}
	
	// nr+16 = flag 2 = ctrl+click held item = put back into player inventory
	// nr+8  = flag 1 = right click = check action
	// nr    = flag 0 = item/button slot#
	
	if (nr >= 16)
	{
		nr -= 16; flag = 2;
	}
	else if (nr >= 8)
	{
		nr -=  8; flag = 1;
	}
	
	for (n = 0; n < 4; n++) in[n] = ch[cn].blacksmith[n];
	
	if (nr == 4) // Leftmost smith button ( <null> : Grind : Smooth )
	{
		if (IS_SANEITEM(in[1]) && it[in[1]].temp == IT_SM_WHET)
		{
			if (flag == 1)
			{
				if (IS_SANEITEM(in[0]))
				{
					if (it[in[0]].flags & IF_ARMORS)
						do_char_log(cn, 1, "This will Smooth a helmet, body armor, or shield, improving its Parry Score by 2 at the cost of 1 Armor Value.\n");
					else
						do_char_log(cn, 1, "This will Grind a weapon, improving its Hit Score by 2 at the cost of 1 Weapon Value.\n");
				}
				else
					do_char_log(cn, 1, "If given a weapon, this will Grind the weapon, improving its Hit Score by 2 at the cost of 1 Weapon Value. If given a helmet, body armor, or shield, this button will change to instead Smooth the item, improving its Parry Score by 2 at the cost of 1 Armor Value.\n");
				return;
			}
			do_bs_whetstone(cn, in[0], in[1], nr);
		}
		return;
	}
	if (nr == 5) // Middle smith button ( Smith! : Hone : Polish )
	{
		if (IS_SANEITEM(in[1]))
		{
			if (flag == 1)
			{
				switch (it[in[1]].temp)
				{
					case IT_SM_WHET:
						if (IS_SANEITEM(in[0]))
						{
							if (it[in[0]].flags & IF_ARMORS)
								do_char_log(cn, 1, "This will Polish a helmet, body armor, or shield, improving its Parry Score by 1.\n");
							else
								do_char_log(cn, 1, "This will Hone a weapon, improving its Hit Score by 1.\n");
						}
						else
							do_char_log(cn, 1, "If given a weapon, this will Hone the weapon, improving its Hit Score by 1. If given a helmet, body armor, or shield, this button will change to instead Polish the item, improving its Parry Score by 1.\n");
						break;
					case IT_SM_CLAY:
						do_char_log(cn, 1, "If given two items of the same type, this will transform the appearance of the item on the left to match the appearance of the item in the middle slot. The item in the middle slot is lost in the process.\n");
						break;
					case IT_SM_MAGE:
						do_char_log(cn, 1, "If given a magic weapon or armor, this will upgrade the weapon or armor to its next tier.\n");
						break;
					case IT_SM_GRAD:
						do_char_log(cn, 1, "If given certain weapons, shields, or books, this will improve the item's properties. Modifiers such as Enchantments or Soulstones will be lost in the process.\n");
						break;
					default:
						break;
				}
				return;
			}
			switch (it[in[1]].temp)
			{
				case IT_SM_WHET:
					do_bs_whetstone(cn, in[0], in[1], nr);
					break;
				case IT_SM_CLAY:
					do_bs_claystone(cn, in[0], in[1], in[2]);
					break;
				case IT_SM_MAGE:
					do_bs_magestone(cn, in[0], in[1]);
					break;
				case IT_SM_GRAD:
					do_bs_gradestone(cn, in[0], in[1]);
					break;
				default:
					break;
			}
			return;
		}
		else if (flag == 1)
		{
			do_char_log(cn, 1, "When given an item to smith and a material to work with, this button can be clicked to confirm the process.\n");
			return;
		}
		do_char_log(cn, 0, "You must give the blacksmith a material to work with first!\n");
		return;
	}
	if (nr == 6) // Rightmost smith button ( <null> : Sharpen : Thicken )
	{
		if (IS_SANEITEM(in[1]) && it[in[1]].temp == IT_SM_WHET)
		{
			if (flag == 1)
			{
				if (IS_SANEITEM(in[0]))
				{
					if (it[in[0]].flags & IF_ARMORS)
						do_char_log(cn, 1, "This will Thicken a helmet, body armor, or shield, improving its Armor Value by 2 at the cost of 1 Parry Score.\n");
					else
						do_char_log(cn, 1, "This will Sharpen a weapon, improving its Weapon Value by 2 at the cost of 1 Hit Score.\n");
				}
				else
					do_char_log(cn, 1, "If given a weapon, this will Sharpen the weapon, improving its Weapon Value by 2 at the cost of 1 Hit Score. If given a helmet, body armor, or shield, this button will change to instead Thicken the item, improving its Armor Value by 2 at the cost of 1 Parry Score.\n");
				return;
			}
			do_bs_whetstone(cn, in[0], in[1], nr);
		}
		return;
	}
	if (nr == 7) return; // Bad ID
	
	if (IS_SANEITEM(in[nr])) // Item in the slot
	{
		if (flag == 2) // ctrl+click held item = put back into player inventory
		{
			for (n = 0; n<MAXITEMS; n++) if (!ch[cn].item[n]) break;
			if (n==MAXITEMS)
			{
				do_char_log(cn, 0, "Your backpack is full!\n");
				return;
			}
			ch[cn].item[n] = in[nr];
			ch[cn].blacksmith[nr] = 0;
			return;
		}
		if (flag == 1) // right click = check action
		{
			do_char_log(cn, 1, "%s:\n", it[in[nr]].name);
			if (it[in[nr]].flags & IF_LOOKSPECIAL)
			{
				look_driver(cn, in[nr]);
			}
			else
			{
				do_char_log(cn, 1, "%s\n", it[in[nr]].description);
				look_item_details(cn, in[nr]);
			}
			return;
		}
	}
	else
	{
		in[nr] = 0;
	}
	if ((n = ch[cn].citem))
	{
		if (nr == 3)
		{
			do_char_log(cn, 0, "That doesn't go there.\n");
			return;
		}
		if (n & 0x80000000)
		{
			do_char_log(cn, 0, "While very grateful, the blacksmith can't do anything with your pile of money.\n");
			return;
		}
		if (((it[n].temp == IT_SM_WHET || it[n].temp == IT_SM_CLAY || it[n].temp == IT_SM_MAGE || it[n].temp == IT_SM_GRAD) && nr != 1) ||
			((it[n].temp != IT_SM_WHET && it[n].temp != IT_SM_CLAY && it[n].temp != IT_SM_MAGE && it[n].temp != IT_SM_GRAD) && nr == 1) )
		{
			do_char_log(cn, 0, "That doesn't go there.\n");
			return;
		}
		if ((it[n].temp != IT_SM_WHET && it[n].temp != IT_SM_CLAY && it[n].temp != IT_SM_MAGE && it[n].temp != IT_SM_GRAD) && (is_valid_smith_item(n) == 0))
		{
			do_char_log(cn, 0, "This item cannot be smithed.\n");
			return;
		}
	}
	ch[cn].blacksmith[nr] = ch[cn].citem;
	ch[cn].citem = in[nr];
}

void do_waypoint(int cn, int nr)
{
	extern struct waypoint waypoint[MAXWPS];
	int i, j, m, n, in, in2;
	
	// Check that the waypoint ID from the client is valid
	if (nr<0 || nr>=64)
	{
		return;
	}
	
	// Check that player is near a waypoint object
	for (i=-2;i<=2;i++)
	{
		for (j=-2;j<=2;j++)
		{
			m = (ch[cn].x + i) + (ch[cn].y + j) * MAPX;
			in = map[m].it;
			// Check if the map object has the driver we want
			if (in && it[in].driver == 88)
				break;
			else
				in = 0;
		}
		// Check if the map object has the driver we want
		if (in && it[in].driver == 88)
			break;
		else
			in = 0;
	}
	
	// If there is no waypoint object nearby, return
	if (!in)
	{
		return;
	}

	// Check that we know this waypoint
	if (!(ch[cn].waypoints&(1<<nr%32)))
	{
		do_char_log(cn, 0, "You must find and use this waypoint in the world before you can return to it!\n");
		return;
	}
	
	if (nr>=32)
	{
		do_char_log(cn, 1, "This will send you to %s.\n", waypoint[nr-32].desc);
	}
	else
	{
		if (!(ch[cn].flags & CF_SYS_OFF))
			do_char_log(cn, 1, "The waypoint whisked you away to %s.\n", waypoint[nr%32].name);
		clear_map_buffs(cn, 1);
		
		for (n = 0; n<MAXITEMS; n++)
		{
			if (IS_SANEUSEDITEM(in2 = ch[cn].item[n]) && (it[in2].flags & IF_IS_KEY) && it[in2].data[1])
			{
				ch[cn].item[n] = 0;
			//	ch[cn].item_lock[n] = 0;
				it[in2].used = USE_EMPTY;
				do_char_log(cn, 1, "Your %s vanished.\n", it[in2].reference);
			}
		}
		
		fx_add_effect(6, 0, ch[cn].x, ch[cn].y, 0);
		god_transfer_char(cn, waypoint[nr%32].x, waypoint[nr%32].y);
		char_play_sound(cn, ch[cn].sound + 21, -100, 0);
		fx_add_effect(6, 0, ch[cn].x, ch[cn].y, 0);
	}
}

void do_treeupdate(int cn, int nr)
{
	int i, j, m=0, n;
	int v = 100000;
	
	// Check that the ID from the client is valid
	if (nr<0 || nr>=72)
	{
		return;
	}
	
	if      (nr>=36)             m = 9;
	else if (IS_SEYAN_DU(cn))    m = 0;
	else if (IS_ARCHTEMPLAR(cn)) m = 1;
	else if (IS_SKALD(cn))       m = 2;
	else if (IS_WARRIOR(cn))     m = 3;
	else if (IS_SORCERER(cn))    m = 4;
	else if (IS_SUMMONER(cn))    m = 5;
	else if (IS_ARCHHARAKIM(cn)) m = 6;
	else if (IS_BRAVER(cn))      m = 7;
	else                         m = 8;
	
	n = ch[cn].citem;
	if (IS_CORRUPTOR(n))
	{
		if (!(n = it[n].data[0]))
		{
			do_char_log(cn, 0, "It doesn't have any effect. Maybe use it on its own first?\n");
			return;
		}
	}
	else
	{
		n = 0;
	}
	if (n<1 || n>NUM_CORR) n = 0;
	
	if (nr>=60)
	{
		nr -= 60;
		if (!T_OS_TREE(cn, nr+1))
		{
			return;
		}
		if ((nr==0 && (T_OS_TREE(cn, 4) || T_OS_TREE(cn, 5) || T_OS_TREE(cn, 6))) ||
			(nr==1 && (T_OS_TREE(cn, 7) || T_OS_TREE(cn, 8) || T_OS_TREE(cn, 9))) ||
			(nr==2 && (T_OS_TREE(cn,10) || T_OS_TREE(cn,11) || T_OS_TREE(cn,12))) )
		{
			do_char_log(cn, 0, "You must de-allocate all attached nodes before you can de-allocate this one.\n");
			return;
		}
		if (ch[cn].gold <= v)
		{
			do_char_log(cn, 0, "You don't have enough money to do that.\n");
			return;
		}
		if (!(ch[cn].flags & CF_SYS_OFF))
			do_char_log(cn, 5, "You de-allocated %s for 1000G.\n", sk_tree[m][nr].name);
		
		ch[cn].os_tree &= ~(1u<<(15-nr));
		ch[cn].os_tree++;
		
		ch[cn].gold -= v;
		char_play_sound(cn, ch[cn].sound + 25, -100, 0);
		do_update_char(cn);
	}
	else if (nr>=48)
	{
		nr -= 48;
		do_char_log(cn, 5, "%s\n", sk_tree[m][nr].name);
		do_char_log(cn, 1, "%s\n", sk_tree[m][nr].desc);
		if (T_OS_TREE(cn, nr+1))
			do_char_log(cn, 4, "SHIFT + Left click to de-allocate for 1000G.\n");
		else
			do_char_log(cn, 6, "Left click to allocate.\n");
	}
	else if (nr>=36)
	{
		nr -= 36;
		if (n)
		{
			do_char_log(cn, 0, "Try as you might, you can't corrupt this tree.\n");
			return;
		}
		if (T_OS_TREE(cn, nr+1))
		{
			do_char_log(cn, 5, "You have %s allocated already!\n", sk_tree[m][nr].name);
			return;
		}
		if (nr>=3 && nr<=5 && !T_OS_TREE(cn, 1))
		{
			do_char_log(cn, 0, "You must allocate the previous node (%s) before you can allocate this one.\n", sk_tree[m][0].name);
			return;
		}
		if (nr>=6 && nr<=8 && !T_OS_TREE(cn, 2))
		{
			do_char_log(cn, 0, "You must allocate the previous node (%s) before you can allocate this one.\n", sk_tree[m][1].name);
			return;
		}
		if (nr>=9 && nr<=11 && !T_OS_TREE(cn, 3))
		{
			do_char_log(cn, 0, "You must allocate the previous node (%s) before you can allocate this one.\n", sk_tree[m][2].name);
			return;
		}
		// Check that we have points available to spend
		if (!(st_skill_pts_have(ch[cn].os_tree)))
		{
			do_char_log(cn, 0, "You do not have any skill points available to allocate.\n");
			return;
		}
		if (!(ch[cn].flags & CF_SYS_OFF))
			do_char_log(cn, 1, "You allocated %s.\n", sk_tree[m][nr].name);
		
		ch[cn].os_tree--;
		ch[cn].os_tree |= (1u<<(15-nr));
		
		char_play_sound(cn, ch[cn].sound + 25, -100, 0);
		do_update_char(cn);
	}
	else if (nr>=24)
	{
		nr -= 24;
		if (!T_SK(cn, nr+1))
		{
			return;
		}
		if ((nr==0 && (T_SK(cn, 4) || T_SK(cn, 5) || T_SK(cn, 6))) ||
			(nr==1 && (T_SK(cn, 7) || T_SK(cn, 8) || T_SK(cn, 9))) ||
			(nr==2 && (T_SK(cn,10) || T_SK(cn,11) || T_SK(cn,12))) )
		{
			do_char_log(cn, 0, "You must de-allocate all attached nodes before you can de-allocate this one.\n");
			return;
		}
		if (ch[cn].gold <= v)
		{
			do_char_log(cn, 0, "You don't have enough money to do that.\n");
			return;
		}
		if (!(ch[cn].flags & CF_SYS_OFF))
		{
			if (n = ch[cn].tree_node[nr])
				do_char_log(cn, 5, "You de-allocated %s for 1000G.\n", sk_corrupt[n].name);
			else
				do_char_log(cn, 5, "You de-allocated %s for 1000G.\n", sk_tree[m][nr].name);
		}
		
		ch[cn].tree_points &= ~(1u<<(15-nr));
		ch[cn].tree_points++;
		
		ch[cn].gold -= v;
		char_play_sound(cn, ch[cn].sound + 25, -100, 0);
		do_update_char(cn);
	}
	else if (nr>=12)
	{
		nr -= 12;
		if (n = ch[cn].tree_node[nr])
		{
			do_char_log(cn, 5, "%s\n", sk_corrupt[n-1].name);
			do_char_log(cn, 1, "%s\n", sk_corrupt[n-1].desc);
		}
		else
		{
			do_char_log(cn, 5, "%s\n", sk_tree[m][nr].name);
			do_char_log(cn, 1, "%s\n", sk_tree[m][nr].desc);
		}
		if (T_SK(cn, nr+1))
			do_char_log(cn, 4, "SHIFT + Left click to de-allocate for 1000G.\n");
		else
			do_char_log(cn, 6, "Left click to allocate.\n");
	}
	else
	{
		if (n)
		{
			ch[cn].tree_node[nr] = n;
			do_char_log(cn, 4, "You feel changed.\n");
			char_play_sound(cn, ch[cn].sound + 25, -100, 0);
			do_update_char(cn);
			use_consume_item(cn, ch[cn].citem, 1);
			return;
		}
		if (T_SK(cn, nr+1))
		{
			if (n = ch[cn].tree_node[nr])
				do_char_log(cn, 5, "You have %s allocated already!\n", sk_corrupt[n-1].name);
			else
				do_char_log(cn, 5, "You have %s allocated already!\n", sk_tree[m][nr].name);
			return;
		}
		if (nr>=3 && nr<=5 && !T_SK(cn, 1))
		{
			if (n = ch[cn].tree_node[0])
				do_char_log(cn, 0, "You must allocate the previous node (%s) before you can allocate this one.\n", sk_corrupt[n-1].name);
			else
				do_char_log(cn, 0, "You must allocate the previous node (%s) before you can allocate this one.\n", sk_tree[m][0].name);
			return;
		}
		if (nr>=6 && nr<=8 && !T_SK(cn, 2))
		{
			if (n = ch[cn].tree_node[1])
				do_char_log(cn, 0, "You must allocate the previous node (%s) before you can allocate this one.\n", sk_corrupt[n-1].name);
			else
				do_char_log(cn, 0, "You must allocate the previous node (%s) before you can allocate this one.\n", sk_tree[m][1].name);
			return;
		}
		if (nr>=9 && nr<=11 && !T_SK(cn, 3))
		{
			if (n = ch[cn].tree_node[2])
				do_char_log(cn, 0, "You must allocate the previous node (%s) before you can allocate this one.\n", sk_corrupt[n-1].name);
			else
				do_char_log(cn, 0, "You must allocate the previous node (%s) before you can allocate this one.\n", sk_tree[m][2].name);
			return;
		}
		// Check that we have points available to spend
		if (!(st_skill_pts_have(ch[cn].tree_points)))
		{
			do_char_log(cn, 0, "You do not have any skill points available to allocate.\n");
			return;
		}
		if (!(ch[cn].flags & CF_SYS_OFF))
		{
			if (n = ch[cn].tree_node[nr])
				do_char_log(cn, 1, "You allocated %s.\n", sk_corrupt[n-1].name);
			else
				do_char_log(cn, 1, "You allocated %s.\n", sk_tree[m][nr].name);
		}
		
		ch[cn].tree_points--;
		ch[cn].tree_points |= (1u<<(15-nr));
		
		char_play_sound(cn, ch[cn].sound + 25, -100, 0);
		do_update_char(cn);
	}
}

int do_add_depot(int cn, int in, int nr)
{
	int n=0;
	
	/*
//	for (n = 0; n<62; n++) if (!ch[cn].depot[n]) break;
	if (n==62) return 0;
	
//	ch[cn].depot[n] = in;
	*/
	
	if (nr>=ST_PAGES*ST_SLOTS) nr -= ST_PAGES*ST_SLOTS;
	nr = nr / ST_SLOTS;	if (nr<0 || nr>=ST_PAGES) return 0;
	
	for (n = 0; n<ST_SLOTS; n++) if (!st[cn].depot[nr][n]) break;
	if (n==ST_SLOTS) return 0;
	
	st[cn].depot[nr][n] = in;
	
	it[in].carried = cn;
	do_update_char(cn);

	return 1;
}

void do_depot_char(int cn, int co, int nr)
{
	int in, n, m, temp=0;

	if (co<=0 || co>=MAXCHARS || nr<0 || nr>=ST_PAGES*ST_SLOTS*2)
	{
		return;
	}
	for (n = 80; n<89; n++)
	{
		if (ch[cn].data[n]==0) continue;
		for (m = 80; m<89; m++)
		{
			if (ch[co].data[m]==0) continue;
			if (ch[cn].data[n]==ch[co].data[m])
			{
				temp=1;
			}
		}
	}
	if (!temp)
	{
		return;
	}
	/*
	if (cn!=co)
	{
		return;
	}
	*/

	if (!(map[ch[cn].x + ch[cn].y * MAPX].flags & MF_BANK) && !(ch[cn].flags & CF_GOD))
	{
		do_char_log(cn, 0, "You cannot access the depot outside a bank.\n");
		return;
	}

	if ((in = ch[cn].citem)!=0)
	{

		if (in & 0x80000000)
		{
			do_char_log(cn, 0, "Use #deposit to put money in the bank!\n");
			return;
		}

		if (!do_maygive(cn, 0, in) || (it[in].flags & IF_NODEPOT))
		{
			do_char_log(cn, 0, "You are not allowed to do that!\n");
			return;
		}

		if (do_add_depot(co, in, nr))
		{
			ch[cn].citem = 0;
			if (!(ch[cn].flags & CF_SYS_OFF))
				do_char_log(cn, 1, "You deposited %s.\n", it[in].reference);
			chlog(cn, "Deposited %s", it[in].name);
			do_update_char(cn);
		}
	}
	else
	{
		/*
		if (nr<62)
		{
//	//	//	if ((in = ch[co].depot[nr])!=0)
			{
				if (god_give_char(in, cn))
				{
//	//	//	//	//	ch[co].depot[nr] = 0;
					if (!(ch[cn].flags & CF_SYS_OFF))
						do_char_log(cn, 1, "You took the %s from the depot.\n", it[in].reference);
					chlog(cn, "Took %s from depot", it[in].name);
					do_update_char(co);
				}
				else
				{
					do_char_log(cn, 0, "You cannot take the %s because your inventory is full.\n", it[in].reference);
				}
			}
		}
		else
		{	nr -= 62;
//	//	//	if ((in = ch[co].depot[nr])!=0)
			{
				do_char_log(cn, 1, "%s:\n", it[in].name);
				if (it[in].flags & IF_LOOKSPECIAL)
				{
					look_driver(cn, in);
				}
				else
				{
					do_char_log(cn, 1, "%s\n", it[in].description);
					look_item_details(cn, in);
				}
			}
		}
		*/
		if (nr<ST_PAGES*ST_SLOTS)
		{
			if ((in = st[co].depot[nr/ST_SLOTS][nr%ST_SLOTS])!=0)
			{
				if (god_give_char(in, cn))
				{
					st[co].depot[nr/ST_SLOTS][nr%ST_SLOTS] = 0;
					if (!(ch[cn].flags & CF_SYS_OFF))
						do_char_log(cn, 1, "You took the %s from the depot.\n", it[in].reference);
					chlog(cn, "Took %s from depot", it[in].name);
					do_update_char(co);
				}
				else
				{
					do_char_log(cn, 0, "You cannot take the %s because your inventory is full.\n", it[in].reference);
				}
			}
		}
		else
		{	nr -= ST_PAGES*ST_SLOTS;
			if ((in = st[co].depot[nr/ST_SLOTS][nr%ST_SLOTS])!=0)
			{
				do_char_log(cn, 1, "%s:\n", it[in].name);
				if (it[in].flags & IF_LOOKSPECIAL)
				{
					look_driver(cn, in);
				}
				else
				{
					do_char_log(cn, 1, "%s\n", it[in].description);
					look_item_details(cn, in);
				}
			}
		}
	}
}

void do_wipe_deaths(int cn, int co) // dbatoi_self(cn, arg[1])
{
	ch[co].data[14] = 0;
	do_char_log(cn, 0, "Done.\n");
}

void do_look_char(int cn, int co, int godflag, int autoflag, int lootflag)
{
	int p, n, nr, hp_diff = 0, end_diff = 0, mana_diff = 0, in, pr, spr, m, ss, en;
	char buf[16], *killer;
	int sk = -1, lex = 0;

	if (co<=0 || co>=MAXCHARS)
	{
		return;
	}

	if ((ch[co].flags & CF_BODY) && abs(ch[cn].x - ch[co].x) + abs(ch[cn].y - ch[co].y)>1)
	{
		return;
	}
	if ((ch[co].flags & CF_BODY) && !lootflag)
	{
		return;
	}

	if (godflag || (ch[co].flags & CF_BODY) || (ch[co].gcm==9))
	{
		p = 1;
	}
	else if (player[ch[cn].player].spectating)
	{
		p = do_char_can_see(player[ch[cn].player].spectating, co, 0);
	}
	else
	{
		p = do_char_can_see(cn, co, 0);
	}
	if (!p)
	{
		return;
	}

	if (!autoflag && !(ch[co].flags & CF_MERCHANT) && !(ch[co].flags & CF_BODY) && ch[co].gcm != 9)
	{
		if ((ch[cn].flags & CF_PLAYER) && !autoflag)
		{
			ch[cn].data[71] += CNTSAY;
			if (ch[cn].data[71]>MAXSAY)
			{
				do_char_log(cn, 0, "Oops, you're a bit too fast for me!\n");
				return;
			}
		}

		if (ch[co].description[0])
		{
			do_char_log(cn, 1, "%s\n", ch[co].description);
		}
		else
		{
			do_char_log(cn, 1, "You see %s.\n", ch[co].reference);
		}

		if (IS_PLAYER(co) && ch[co].data[0])
		{
			if (ch[co].text[0][0])
			{
				do_char_log(cn, 0, "%s is away from keyboard; Message:\n", ch[co].name);
				do_char_log(cn, 3, "  \"%s\"\n", ch[co].text[0]);
			}
			else
			{
				do_char_log(cn, 0, "%s is away from keyboard.\n", ch[co].name);
			}
		}

		if ((ch[co].flags & (CF_PLAYER)) && IS_PURPLE(co))
		{
			do_char_log(cn, 0, "%s is a follower of the Purple One.\n", ch[co].reference);
		}

		if (!godflag && cn!=co && (ch[cn].flags & (CF_PLAYER)) && !(ch[cn].flags & CF_INVISIBLE)&& !(ch[cn].flags & CF_SHUTUP))
		{
			do_char_log(co, 1, "%s looks at you.\n", ch[cn].name);
		}

		if ((ch[co].flags & (CF_PLAYER)) && ch[co].data[14] && !(ch[co].flags & CF_GOD))
		{
			if (!ch[co].data[15])
			{
				killer = "unknown causes";
			}
			else if (ch[co].data[15]>=MAXCHARS)
			{
				killer = ch[ch[co].data[15] & 0xffff].reference;
			}
			else
			{
				killer = ch_temp[ch[co].data[15]].reference;
			}

			do_char_log(cn, 5, "%s died %d times, the last time on the day %d of the year %d, killed by %s %s.\n",
			            ch[co].reference,
			            ch[co].data[14],
			            ch[co].data[16] % 300, ch[co].data[16] / 300+GAMEYEAR,
			            killer,
			            get_area_m(ch[co].data[17] % MAPX, ch[co].data[17] / MAPX, 1));
		}

		if ((ch[co].flags & (CF_PLAYER)) && ch[co].data[44] && !(ch[co].flags & CF_GOD))
		{
			do_char_log(cn, 1, "%s was saved from death %d times.\n",
			            ch[co].reference,
			            ch[co].data[44]);
		}
		
		if ((ch[co].flags & (CF_PLAYER)) && (ch[co].pandium_floor[0]>1 || ch[co].pandium_floor[1]>1) && 
			(!(ch[co].flags & CF_GOD) || (ch[cn].flags & CF_GOD)))
		{
			if (ch[co].pandium_floor[0]>1 && ch[co].pandium_floor[0]>=ch[co].pandium_floor[1])
			{
				do_char_log(cn, 9, "%s has defeated The Archon Pandium at depth %d.\n",
			            ch[co].reference, ch[co].pandium_floor[0]-1);
			}
			else if (ch[co].pandium_floor[0]>1 && ch[co].pandium_floor[1]>1)
			{
				do_char_log(cn, 9, "%s has defeated The Archon Pandium at depth %d in a group, and at depth %d alone.\n",
			            ch[co].reference, ch[co].pandium_floor[1]-1, ch[co].pandium_floor[0]-1);
			}
			else
			{
				do_char_log(cn, 9, "%s has defeated The Archon Pandium at depth %d in a group.\n",
			            ch[co].reference, ch[co].pandium_floor[1]-1);
			}
		}

		if ((ch[co].flags & (CF_PLAYER)) && (ch[co].kindred & (KIN_POH)))
		{
			if (ch[co].kindred & KIN_POH_LEADER)
			{
				do_char_log(cn, 8, "%s is a Leader among the Purples of Honor.\n", ch[co].reference);
			}
			else
			{
				do_char_log(cn, 8, "%s is a Purple of Honor.\n", ch[co].reference);
			}
		}

		if (ch[co].text[3][0] && (ch[co].flags & CF_PLAYER))
		{
			do_char_log(cn, 0, "%s\n", ch[co].text[3]);
		}
	}

	nr = ch[cn].player;

	buf[0] = SV_LOOK1;
	if (p<=75)
	{
		*(unsigned short*)(buf + 1)  = (ch[co].worn[0] ? it[ch[co].worn[0]].sprite[0] : 0);
		*(unsigned short*)(buf + 3)  = (ch[co].worn[2] ? it[ch[co].worn[2]].sprite[0] : 0);
		*(unsigned short*)(buf + 5)  = (ch[co].worn[3] ? it[ch[co].worn[3]].sprite[0] : 0);
		*(unsigned short*)(buf + 7)  = (ch[co].worn[5] ? it[ch[co].worn[5]].sprite[0] : 0);
		*(unsigned short*)(buf + 9)  = (ch[co].worn[6] ? it[ch[co].worn[6]].sprite[0] : 0);
		*(unsigned short*)(buf + 11) = (ch[co].worn[7] ? it[ch[co].worn[7]].sprite[0] : 0);
		*(unsigned short*)(buf + 13) = (ch[co].worn[8] ? it[ch[co].worn[8]].sprite[0] : 0);
		*(unsigned char*)(buf + 15)  = autoflag;
	}
	else
	{
		*(unsigned short*)(buf + 1)  = 35;
		*(unsigned short*)(buf + 3)  = 35;
		*(unsigned short*)(buf + 5)  = 35;
		*(unsigned short*)(buf + 7)  = 35;
		*(unsigned short*)(buf + 9)  = 35;
		*(unsigned short*)(buf + 11) = 35;
		*(unsigned short*)(buf + 13) = 35;
		*(unsigned char*)(buf + 15)  = autoflag;
	}
	xsend(nr, buf, 16);

	buf[0] = SV_LOOK2;

	if (p<=75)
	{
		*(unsigned short*)(buf + 1)  = (ch[co].worn[9] ? it[ch[co].worn[9]].sprite[0] : 0);
		*(unsigned short*)(buf + 13) = (ch[co].worn[10] ? it[ch[co].worn[10]].sprite[0] : 0);
	}
	else
	{
		*(unsigned short*)(buf + 1)  = 35;
		*(unsigned short*)(buf + 13) = 35;
	}

	*(unsigned short*)(buf + 3) = ch[co].sprite;
	*(unsigned int*)(buf + 5) = ch[co].points_tot;
	if (p>75)
	{
		hp_diff   = ch[co].hp[5] / 2 - RANDOM(ch[co].hp[5] + 1);
		end_diff  = ch[co].end[5] / 2 - RANDOM(ch[co].end[5] + 1);
		mana_diff = ch[co].mana[5] / 2 - RANDOM(ch[co].mana[5] + 1);
	}
	*(unsigned int*)(buf + 9) = ch[co].hp[5] + hp_diff;
	xsend(nr, buf, 16);

	buf[0] = SV_LOOK3;
	*(unsigned short*)(buf + 1)  = ch[co].end[5] + end_diff;
	*(unsigned short*)(buf + 3)  = (ch[co].a_hp + 500) / 1000 + hp_diff;
	*(unsigned short*)(buf + 5)  = (ch[co].a_end + 500) / 1000 + end_diff;
	*(unsigned short*)(buf + 7)  = co;
	*(unsigned short*)(buf + 9)  = (unsigned short)char_id(co);
	*(unsigned short*)(buf + 11) = ch[co].mana[5] + mana_diff;
	*(unsigned short*)(buf + 13) = (ch[co].a_mana + 500) / 1000 + mana_diff;

	xsend(nr, buf, 16);

	buf[0] = SV_LOOK4;
	if (p<=75)
	{
		*(unsigned short*)(buf + 1)  = (ch[co].worn[1] ? it[ch[co].worn[1]].sprite[0] : 0);
		*(unsigned short*)(buf + 3)  = (ch[co].worn[4] ? it[ch[co].worn[4]].sprite[0] : 0);
		*(unsigned short*)(buf + 10) = (ch[co].worn[11] ? it[ch[co].worn[11]].sprite[0] : 0);
		*(unsigned short*)(buf + 12) = (ch[co].worn[12] ? it[ch[co].worn[12]].sprite[0] : 0);
		*(unsigned short*)(buf + 14) = (ch[co].worn[13] ? it[ch[co].worn[13]].sprite[0] : 0);
	}
	else
	{
		*(unsigned short*)(buf + 1)  = 35;
		*(unsigned short*)(buf + 3)  = 35;
		*(unsigned short*)(buf + 10) = 35;
		*(unsigned short*)(buf + 12) = 35;
		*(unsigned short*)(buf + 14) = 35;
	}
	if (((ch[co].flags & (CF_MERCHANT | CF_BODY)) || (ch[co].gcm==9)) && !autoflag)
	{
		lex |= 1;
		if ((in = ch[cn].citem)!=0)
		{
			if (ch[co].flags & CF_BSPOINTS)
			{
				int in2;
				// Stronghold items reflected by player stats
				if (ch[co].temp == CT_TACTICIAN || ch[co].temp == get_nullandvoid(0))
				{
					in2 = change_bs_shop_item(cn, it[in].temp);
					pr = do_item_bsvalue(cn, in2);
				}
				/*
				// Casino items
				else if (ch[co].temp == CT_JESSICA)
				{
					pr = do_item_bsvalue(cn, it[in].temp);
				}
				// Contract items
				else if (ch[co].temp == CT_ADHERENT || ch[co].temp == get_nullandvoid(1))
				{
					in2 = change_bs_shop_item(cn, it[in].temp);
					pr = do_item_bsvalue(cn, in2);
				}
				// Exp items
				else if (ch[co].temp == CT_EALDWULF || ch[co].temp == CT_ZANA)
				{
					in2 = change_bs_shop_item(cn, it[in].temp);
					pr = do_item_xpvalue(cn, in2, 0);
				}
				*/
				else pr = 0;
			}
			else if (ch[co].flags & CF_MERCHANT)
				pr = barter(cn, in, 0);
			else
				pr = 0;
		}
		else
		{
			pr = 0;
		}
		*(unsigned int*)(buf + 6) = pr;
	}
	if (IS_SHADOW(co)) lex |= 2;
	if (IS_BLOODY(co)) lex |= 4;
	
	*(unsigned char*)(buf + 5) = lex;

	xsend(nr, buf, 16);

	buf[0] = SV_LOOK5;
	for (n = 0; n<15; n++)
	{
		buf[n + 1] = ch[co].name[n];
	}
	xsend(nr, buf, 16);

	if ((ch[co].flags & CF_MERCHANT) && !autoflag && (ch[co].temp == CT_BLACKSMITH1 || ch[co].temp == CT_BLACKSMITH2))
	{
		buf[0] = SV_LOOK8;
		if (IS_SANEITEM(ch[cn].blacksmith[0]) && (it[ch[cn].blacksmith[0]].flags & IF_ARMORS))
			*(unsigned char *)(buf + 1) = 5;
		else
			*(unsigned char *)(buf + 1) = 4;
		*(unsigned short*)(buf + 2) = (unsigned short)(ch[cn].smithnum = co);
		*(unsigned short*)(buf + 4) = 0;
		*(unsigned char *)(buf + 6) = 0;
		*(unsigned char *)(buf + 7) = 0;
		xsend(nr, buf, 8);
	}
	else if (((ch[co].flags & (CF_MERCHANT | CF_BODY)) || (ch[co].gcm==9)) && !autoflag)
	{
		for (n = 0; n<MAXITEMS; n += 2)
		{
			buf[0] = SV_LOOK6; // Shop/Grave
			buf[1] = n;
			for (m = n; m<min(MAXITEMS, n + 2); m++)
			{
				if ((in = ch[co].item[m])!=0)
				{
					spr = it[in].sprite[0];
					if (it[in].flags & IF_SOULSTONE) ss = 1; else ss = 0;
					if (it[in].flags & IF_ENCHANTED) en = 1; else en = 0;
					if (ch[co].flags & CF_BSPOINTS)
					{
						int in2;
						// Stronghold items reflected by player stats
						if (ch[co].temp == CT_TACTICIAN || ch[co].temp == get_nullandvoid(0))
						{
							in2 = change_bs_shop_item(cn, it[in].temp);
							if (in2)
							{
								pr = do_item_bsvalue(cn, in2);
								spr = get_special_spr(in2, it_temp[in2].sprite[0]);
							}
							else
							{
								ss = en = spr = pr = 0;
							}
						}
						// Casino items
						else if (ch[co].temp == CT_JESSICA)
						{
							pr = do_item_bsvalue(cn, it[in].temp);
						}
						// Contract items
						else if (ch[co].temp == CT_ADHERENT || ch[co].temp == get_nullandvoid(1))
						{
							in2 = change_bs_shop_item(cn, it[in].temp);
							if (in2 == IT_OS_SP && do_item_osvalue(cn)==0) in2 = 0;
							if (in2)
							{
								pr = do_item_bsvalue(cn, in2);
								if (in2 == IT_OS_SP) pr = do_item_osvalue(cn);
								spr = get_special_spr(in2, it_temp[in2].sprite[0]);
							}
							else
							{
								ss = en = spr = pr = 0;
							}
						}
						// Experience items
						else if (ch[co].temp == CT_EALDWULF || ch[co].temp == CT_ZANA)
						{
							in2 = change_bs_shop_item(cn, it[in].temp);
							if (ch[co].temp == CT_ZANA && (sk = change_xp_shop_item(cn, m)) == -1)
							{
								in2 = 0;
							}
							if (in2)
							{
								if (ch[co].temp == CT_ZANA && sk > -1 && ch[cn].skill[sk][1] >= 10)
								{
									ss = en = spr = pr = 0;
								}
								else
								{
									pr = do_item_xpvalue(cn, in2, sk);
									spr = get_special_spr(in2, it_temp[in2].sprite[0]);
								}
							}
							else
							{
								ss = en = spr = pr = 0;
							}
						}
						else pr = 0;
					}
					else if (ch[co].flags & CF_MERCHANT)
						pr = barter(cn, in, 1);
					else
						pr = 0;
				}
				else
				{
					ss = en = spr = pr = 0;
				}
				pr = min((1<<30)-1, pr); if (ss) pr |= 1<<30; if (en) pr |= 1<<31;
				*(unsigned short*)(buf + 2 + (m - n) * 6) = spr;
				*(unsigned int*)(buf + 4 + (m - n) * 6) = pr;
				*(unsigned char*)(buf + 14 + (m - n)) = 0;
				if (IS_SOULCAT(in)) *(unsigned char*)(buf + 14 + (m - n)) = it[in].data[4];
			}
			xsend(nr, buf, 16);
		}

		for (n = 0; n<20; n += 2)
		{
			buf[0] = SV_LOOK6; // Shop/Grave
			buf[1] = n + 40;
			for (m = n; m<min(20, n + 2); m++)
			{
				if ((in = ch[co].worn[m])!=0 && (ch[co].flags & CF_BODY))
				{
					spr = it[in].sprite[0];
					if (it[in].flags & IF_SOULSTONE) ss = 1; else ss = 0;
					if (it[in].flags & IF_ENCHANTED) en = 1; else en = 0;
					pr  = 0;
				}
				else
				{
					ss = en = spr = pr = 0;
				}
				pr = min((1<<30)-1, pr); if (ss) pr |= 1<<30; if (en) pr |= 1<<31;
				*(unsigned short*)(buf + 2 + (m - n) * 6) = spr;
				*(unsigned int*)(buf + 4 + (m - n) * 6) = pr;
				*(unsigned char*)(buf + 14 + (m - n)) = 0; // free bits
			}
			xsend(nr, buf, 16);
		}

		buf[0] = SV_LOOK6; // Shop/Grave
		buf[1] = 60;
		if ((in = ch[co].citem)!=0 && (ch[co].flags & CF_BODY))
		{
			spr = it[in].sprite[0];
			if (it[in].flags & IF_SOULSTONE) ss = 1; else ss = 0;
			if (it[in].flags & IF_ENCHANTED) en = 1; else en = 0;
			pr  = 0;
		}
		else
		{
			ss = en = spr = pr = 0;
		}
		pr = min((1<<30)-1, pr); if (ss) pr |= 1<<30; if (en) pr |= 1<<31;
		*(unsigned short*)(buf + 2 + 0 * 6) = spr;
		*(unsigned int*)(buf + 4 + 0 * 6) = pr;

		if (ch[co].gold && (ch[co].flags & CF_BODY))
		{
			if (ch[co].gold>999999)
				spr = 121;
			else if (ch[co].gold>99999)
				spr = 120;
			else if (ch[co].gold>9999)
				spr = 41;
			else if (ch[co].gold>999)
				spr = 40;
			else if (ch[co].gold>99)
				spr = 39;
			else if (ch[co].gold>9)
				spr = 38;
			else
				spr = 37;
			pr = 0;
		}
		else
		{
			spr = pr = 0;
		}
		*(unsigned short*)(buf + 2 + 1 * 6) = spr;
		*(unsigned int*)(buf + 4 + 1 * 6) = pr;
		if ((ch[co].flags & CF_BSPOINTS) && !(ch[co].flags & CF_BODY)) // For the Black Stronghold point shop
		{
			if (ch[co].temp == CT_TACTICIAN || ch[co].temp == get_nullandvoid(0))
				*(unsigned char*)(buf + 14) = 101;
			else if (ch[co].temp == CT_JESSICA)
				*(unsigned char*)(buf + 14) = 102;
			else if (ch[co].temp == CT_ADHERENT || ch[co].temp == get_nullandvoid(1))
				*(unsigned char*)(buf + 14) = 103;
			else if (ch[co].temp == CT_EALDWULF || ch[co].temp == CT_ZANA)
				*(unsigned char*)(buf + 14) = 104;
			else
				*(unsigned char*)(buf + 14) = 0;
		}
		else
			*(unsigned char*)(buf + 14) = 0;
		xsend(nr, buf, 16);
	}

	if ((ch[cn].flags & (CF_GOD | CF_IMP | CF_USURP)) && !autoflag && !lootflag && 
		!(ch[co].flags & CF_MERCHANT) && !(ch[co].flags & CF_BODY) && !(ch[co].flags & CF_GOD) && ch[co].gcm != 9)
	{
		do_char_log(cn, 3, "This is char %d, created from template %d, pos %d,%d\n", co, ch[co].temp, ch[co].x, ch[co].y);
		if (ch[co].flags & CF_GOLDEN)
			do_char_log(cn, 3, "Golden List.\n");
		if (ch[co].flags & CF_BLACK)
			do_char_log(cn, 3, "Black List.\n");
	}
}

void do_look_depot(int cn, int co)
{
	int n, nr, in, pr, spr, m, temp=0, ss, en, ca, cr, lex = 0, stack=0;
	char buf[16];

	if (co<=0 || co>=MAXCHARS)
	{
		return;
	}
	for (n = 80; n<89; n++)
	{
		if (ch[cn].data[n]==0) continue;
		for (m = 80; m<89; m++)
		{
			if (ch[co].data[m]==0) continue;
			if (ch[cn].data[n]==ch[co].data[m])
			{
				temp=1;
			}
		}
	}
	if (!temp)
	{
		return;
	}
	/*
	if (cn!=co)
	{
		return;
	}
	*/

	if (!(map[ch[cn].x + ch[cn].y * MAPX].flags & MF_BANK) && !(ch[cn].flags & CF_GOD))
	{
		do_char_log(cn, 0, "You cannot access your depot outside a bank.\n");
		return;
	}

	nr = ch[cn].player;

	
	buf[0] = SV_LOOK1;
	*(unsigned short*)(buf + 1)  = 35;
	*(unsigned short*)(buf + 3)  = 35;
	*(unsigned short*)(buf + 5)  = 35;
	*(unsigned short*)(buf + 7)  = 35;
	*(unsigned short*)(buf + 9)  = 35;
	*(unsigned short*)(buf + 11) = 35;
	*(unsigned short*)(buf + 13) = 35;
	*(unsigned char*)(buf + 15)  = 0;
	xsend(nr, buf, 16);

	buf[0] = SV_LOOK2;

	*(unsigned short*)(buf + 1)  = 35;
	*(unsigned short*)(buf + 13) = 35;
	*(unsigned short*)(buf + 3) = ch[co].sprite;
	*(unsigned int*)(buf + 5) = ch[co].points_tot;
	*(unsigned int*)(buf + 9) = ch[co].hp[5];
	xsend(nr, buf, 16);

	buf[0] = SV_LOOK3;
	*(unsigned short*)(buf + 1)  = ch[co].end[5];
	*(unsigned short*)(buf + 3)  = (ch[co].a_hp + 500) / 1000;
	*(unsigned short*)(buf + 5)  = (ch[co].a_end + 500) / 1000;
	*(unsigned short*)(buf + 7)  = co | 0x8000;						// 0x8000 tells the client this is a depot for the return packet
	*(unsigned short*)(buf + 9)  = (unsigned short)char_id(co);
	*(unsigned short*)(buf + 11) = ch[co].mana[5];
	*(unsigned short*)(buf + 13) = (ch[co].a_mana + 500) / 1000;
	xsend(nr, buf, 16);

	buf[0] = SV_LOOK4;
	*(unsigned short*)(buf + 1)  = 35;
	*(unsigned short*)(buf + 3)  = 35;
	*(unsigned short*)(buf + 10) = 35;
	*(unsigned short*)(buf + 12) = 35;
	*(unsigned short*)(buf + 14) = 35;
	lex = 1;
	if (IS_SHADOW(co)) lex |= 2;
	if (IS_BLOODY(co)) lex |= 4;
	
	*(unsigned char*)(buf + 5) = lex;

	// CS, 000205: Check for sane item (not money)
	if (IS_SANEITEM(in = ch[cn].citem))
	{
		pr = 0;
	}
	else
	{
		pr = 0;
	}

	*(unsigned int*)(buf + 6) = pr;
	xsend(nr, buf, 16);

	
	buf[0] = SV_LOOK5;
	for (n = 0; n<15; n++)
	{
		buf[n + 1] = ch[co].name[n];
	}
	xsend(nr, buf, 16);
	
	/*
	for (n = 0; n<62; n += 2)
	{
		buf[0] = SV_LOOK6; // Depot (old)
		buf[1] = n;
		for (m = n; m<min(62, n + 2); m++)
		{
//	//	//	if ((in = ch[co].depot[m])!=0)
			{
				spr = it[in].sprite[0];
				if (it[in].flags & IF_SOULSTONE) ss = 1; else ss = 0;
				if (it[in].flags & IF_ENCHANTED) en = 1; else en = 0;
				pr  = 0;
			}
			else
			{
				ss = en = spr = pr = 0;
			}
			pr = min((1<<30)-1, pr); if (ss) pr |= 1<<30; if (en) pr |= 1<<31;
			*(unsigned short*)(buf + 2 + (m - n) * 6) = spr;
			*(unsigned int*)(buf + 4 + (m - n) * 6) = pr;
			*(unsigned char*)(buf + 14 + (m - n)) = 0;
			if (IS_SOULCAT(in)) *(unsigned char*)(buf + 14 + (m - n)) = it[in].data[4]; // 14 & 15
		}
		xsend(nr, buf, 16);
	}
	*/
	for (n = 0; n<ST_PAGES*ST_SLOTS; n++)
	{
		buf[0] = SV_LOOK7;		// Depot (new)
		buf[1] = n/ST_SLOTS;
		buf[2] = n%ST_SLOTS;
		
		if ((in = st[co].depot[n/ST_SLOTS][n%ST_SLOTS])!=0)
		{
			spr = it[in].sprite[0];
			if (it[in].flags & IF_SOULSTONE) ss = 1; else ss = 0;
			if (it[in].flags & IF_ENCHANTED) en = 2; else en = 0;
			if (it[in].flags & IF_CORRUPTED) cr = 4; else cr = 0;
			if (IS_SOULCAT(in)) ca = it[in].data[4]; else ca = 0;
			if (it[in].stack) stack = it[in].stack;  else stack = 0;
		}
		else
		{
			spr = ss = en = cr = ca = stack = 0;
		}
		*(unsigned short*)(buf + 3) = spr;
		*(unsigned char *)(buf + 5) = stack;
		*(unsigned char *)(buf + 6) = ss + en + cr;
		*(unsigned char *)(buf + 7) = ca;
		
		xsend(nr, buf, 8);
	}
}

// DEBUG ADDED: JC 07/11/2001
void do_look_player_depot(int cn, char *cv)
{
	int in, m;
	int count = 0;

	int co = dbatoi(cv);
	if (!IS_SANECHAR(co))
	{
		do_char_log(cn, 0, "Bad character: %s!\n", cv);
		return;
	}
	
	do_char_log(cn, 1, "Depot contents for : %s\n", ch[co].name);
	do_char_log(cn, 1, "-----------------------------------\n");
	/*
//	for (m = 0; m<62; m++) if ((in = ch[co].depot[m])!=0)
	{
		do_char_log(cn, 1, "0 %6d: %s\n", in, it[in].name);
		count++;
	}
	*/
	for (m = 0; m<ST_PAGES*ST_SLOTS; m++) if ((in = st[co].depot[m/ST_SLOTS][m%ST_SLOTS])!=0)
	{
		do_char_log(cn, 1, "%d %6d: %s\n", m/ST_SLOTS+1, in, it[in].name);
		count++;
	}

	do_char_log(cn, 1, " \n");
	do_char_log(cn, 1, "Total : %d items.\n", count);
}

void do_look_player_inventory(int cn, char *cv)
{
	int n, in;
	int count = 0;

	int co = dbatoi(cv);
	if (!IS_SANECHAR(co))
	{
		do_char_log(cn, 0, "Bad character: %s!\n", cv);
		return;
	}
	do_char_log(cn, 1, "Inventory contents for : %s\n", ch[co].name);
	do_char_log(cn, 1, "-----------------------------------\n");

	for (n = 0; n<MAXITEMS; n++)
	{
		if ((in = ch[co].item[n])!=0)
		{
			do_char_log(cn, 1, "%6d: %s\n", in, it[in].name);
			count++;
		}
	}

	do_char_log(cn, 1, " \n");
	do_char_log(cn, 1, "Total : %d items.\n", count);
}

void do_look_player_equipment(int cn, char *cv)
{
	int n, in;
	int count = 0;

	int co = dbatoi(cv);
	if (!IS_SANECHAR(co))
	{
		do_char_log(cn, 0, "Bad character: %s!\n", cv);
		return;
	}
	do_char_log(cn, 1, "Equipment for : %s\n", ch[co].name);
	do_char_log(cn, 1, "-----------------------------------\n");

	for (n = 0; n<20; n++)
	{
		if ((in = ch[co].worn[n])!=0)
		{
			do_char_log(cn, 1, "%6d: %s\n", in, it[in].name);
			count++;
		}
	}
	do_char_log(cn, 1, " \n");
	for (n = 0; n<12; n++)
	{
		if ((in = ch[co].alt_worn[n])!=0)
		{
			do_char_log(cn, 1, "%6d: %s\n", in, it[in].name);
			count++;
		}
	}

	do_char_log(cn, 1, " \n");
	do_char_log(cn, 1, "Total : %d items.\n", count);
}


void do_steal_player(int cn, char *cv, char *ci)
{
	int n;
	int i_index  = 0;
	char found_i = 0;
	char found_d = 0;
	char found_e = 0;

	int co = dbatoi(cv);
	int in = atoi(ci);

	if (!IS_SANECHAR(co))
	{
		do_char_log(cn, 0, "Bad character: %s!\n", cv);
		return;
	}
	else if (in == 0)
	{
		return;
	}

	//look through depot and inventory for this item
	for (n = 0; n<MAXITEMS; n++) if (in==ch[co].item[n])
	{
		i_index = n;
		found_i = !(0);
		break;
	}
	if (!found_i)
	{
		/*
//	//	for (n = 0; n<62; n++) if (in==ch[co].depot[n])
		{
			i_index = n;
			found_d = !(0);
			break;
		}
		*/
		for (n = 0; n<ST_PAGES*ST_SLOTS; n++) if (in==st[co].depot[n/ST_SLOTS][n%ST_SLOTS])
		{
			i_index = n;
			found_d = !(0);
			break;
		}
	}
	if (!found_i && !found_d)
	{
		for (n = 0; n<20; n++) if (in==ch[co].worn[n])
		{
			i_index = n;
			found_e = !(0);
			break;
		}
		for (n = 0; n<12; n++) if (in==ch[co].alt_worn[n])
		{
			i_index = n;
			found_e = !(0);
			break;
		}
	}
	if (found_i | found_d | found_e)
	{
		if (god_give_char(in, cn))
		{
			if (found_i)      ch[co].item[i_index]  = 0;
//	//	//	else if (found_d) ch[co].depot[i_index] = 0;
			else if (found_d) st[co].depot[i_index/ST_SLOTS][i_index%ST_SLOTS] = 0;
			else if (found_e) ch[co].worn[i_index]  = 0;
			do_char_log(cn, 1, "You stole %s from %s.", it[in].reference, ch[co].name);
		}
		else
		{
			do_char_log(cn, 0, "You cannot take the %s because your inventory is full.\n", it[in].reference);
		}
	}
	else
	{
		do_char_log(cn, 0, "Item not found.\n");
	}
}

static inline void map_add_light(int x, int y, int v)
{
	register unsigned int m;
	// if (x<0 || x>=MAPX || y<0 || y>=MAPY || v==0) return;

	m = x + y * MAPX;

	map[m].light += v;

	if (map[m].light<0)
	{
//              xlog("Error in light computations at %d,%d (+%d=%d).",x,y,v,map[m].light);
		map[m].light = 0;
	}
}

void do_add_light(int xc, int yc, int stren)
{
	int x, y, xs, ys, xe, ye, v, d, flag;
	unsigned long long prof;

	prof = prof_start();
	
	if (IS_GLOB_MAYHEM) stren = stren / 3 * 2;

	map_add_light(xc, yc, stren);

	if (stren<0)
	{
		flag  = 1;
		stren = -stren;
	}
	else
	{
		flag = 0;
	}

	xs = max(0, xc - LIGHTDIST);
	ys = max(0, yc - LIGHTDIST);
	xe = min(MAPX - 1, xc + 1 + LIGHTDIST);
	ye = min(MAPY - 1, yc + 1 + LIGHTDIST);

	for (y = ys; y<ye; y++)
	{
		for (x = xs; x<xe; x++)
		{
			if (x==xc && y==yc)
			{
				continue;
			}
			if ((xc - x) * (xc - x) + (yc - y) * (yc - y)>(LIGHTDIST * LIGHTDIST + 1))
			{
				continue;
			}
			if ((v = can_see(0, xc, yc, x, y, LIGHTDIST))!=0)
			{
				d = stren / (v * (abs(xc - x) + abs(yc - y)));
				if (flag)
				{
					map_add_light(x, y, -d);
				}
				else
				{
					map_add_light(x, y, d);
				}
			}
		}
	}
	prof_stop(9, prof);
}

// will put citem into item[X], X being the first free slot.
// returns the slot number on success, -1 otherwise.
int do_store_item(int cn)
{
	int n;

	if (ch[cn].citem & 0x80000000)
	{
		return -1;
	}

	for (n = 0; n<MAXITEMS; n++)
	{
		if (!ch[cn].item[n])
		{
			break;
		}
	}

	if (n==MAXITEMS)
	{
		return -1;
	}

	ch[cn].item[n] = ch[cn].citem;
	ch[cn].citem = 0;

	do_update_char(cn);

	return(n);

}

int do_check_fool(int cn, int in)
{
	int m;
	static char *at_text[5] = {
		"not brave enough", 
		"not determined enough", 
		"not intuitive enough", 
		"not agile enough", 
		"not strong enough"
	};
	static char *at_names[5] = {
		"Braveness", 
		"Willpower", 
		"Intuition", 
		"Agility", 
		"Strength"
	};
	
	if (it[in].min_rank>getrank(cn))
	{
		do_char_log(cn, 0, "You're not experienced enough to use that.\n");
		do_char_log(cn, 0, "(Need a rank of %s)\n",rank_name[it[in].min_rank]);
		return -1;
	}
	for (m = 0; m<5; m++)
	{
		if ((unsigned char)it[in].attrib[m][2]>B_AT(cn, m))
		{
			do_char_log(cn, 0, "You're %s to use that.\n\n", at_text[m]);
			do_char_log(cn, 0, "(Need %d base %s)\n", (unsigned char)it[in].attrib[m][2],at_names[m]);
			return -1;
		}
	}
	if (it[in].hp[2]>ch[cn].hp[0])
	{
		do_char_log(cn, 0, "You don't have enough life force to use that.\n");
		do_char_log(cn, 0, "(Need %d base Hitpoints)\n",it[in].hp[2]);
		return -1;
	}
	if (it[in].mana[2]>ch[cn].mana[0])
	{
		do_char_log(cn, 0, "You don't have enough mana to use that.\n");
		do_char_log(cn, 0, "(Need %d base Mana)\n",it[in].mana[2]);
		return -1;
	}
	for (m = 0; m<MAXSKILL; m++)
	{
		if (it[in].skill[m][2] && !B_SK(cn, m))
		{
			do_char_log(cn, 0, "You don't know how to use that.\n");
			do_char_log(cn, 0, "(Need %s)\n",skilltab[m].name);
			return -1;
		}
		if ((unsigned char)it[in].skill[m][2]>B_SK(cn, m))
		{
			do_char_log(cn, 0, "You're not skilled enough to use that.\n");
			do_char_log(cn, 0, "(Need %d base %s)\n",(unsigned char)it[in].skill[m][2],skilltab[m].name);
			return -1;
		}
	}
	
	return 1;
}

int do_swap_item(int cn, int n)
{
	int tmp, in, m;
	static char *at_text[5] = {
		"not brave enough", 
		"not determined enough", 
		"not intuitive enough", 
		"not agile enough", 
		"not strong enough"
	};
	static char *at_names[5] = {
		"Braveness", 
		"Willpower", 
		"Intuition", 
		"Agility", 
		"Strength"
	};

	if (ch[cn].citem & 0x80000000)
	{
		return -1;
	}

	if (n<0 || n>19)
	{
		return -1;        // sanity check

	}
	tmp = ch[cn].citem;

	// check prerequisites:
	if (tmp)
	{
		if (it[tmp].driver==40 && it[tmp].data[0]!=cn)
		{
			do_char_log(cn, 0, "The goddess Kwai frowns at your attempt to use another one's %s.\n", it[tmp].reference);
			return -1;
		}
		if (it[tmp].driver==52 && it[tmp].data[0]!=cn)
		{
			if (it[tmp].data[0]==0)
			{
				char buf[300];

				it[tmp].data[0] = cn;

				sprintf(buf, "%s Engraved in it are the letters \"%s\".",
				        it[tmp].description, ch[cn].name);
				if (strlen(buf)<200)
				{
					strcpy(it[tmp].description, buf);
				}
			}
			else
			{
				do_char_log(cn, 0, "The gods frown at your attempt to wear another one's %s.\n", it[tmp].reference);
				return -1;
			}
		}
		for (m = 0; m<5; m++)
		{
			if ((unsigned char)it[tmp].attrib[m][2]>B_AT(cn, m))
			{
				do_char_log(cn, 0, "You're %s to use that.\n\n", at_text[m]);
				do_char_log(cn, 0, "(Need %d base %s)\n", (unsigned char)it[tmp].attrib[m][2],at_names[m]);
				return -1;
			}
		}
		for (m = 0; m<MAXSKILL; m++)
		{
			if (it[tmp].skill[m][2] && !B_SK(cn, m))
			{
				do_char_log(cn, 0, "You don't know how to use that.\n");
				do_char_log(cn, 0, "(Need %s)\n",skilltab[m].name);
				return -1;
			}
			if ((unsigned char)it[tmp].skill[m][2]>B_SK(cn, m))
			{
				do_char_log(cn, 0, "You're not skilled enough to use that.\n");
				do_char_log(cn, 0, "(Need %d base %s)\n",(unsigned char)it[tmp].skill[m][2],skilltab[m].name);
				return -1;
			}
		}
		if (it[tmp].hp[2]>ch[cn].hp[0])
		{
			do_char_log(cn, 0, "You don't have enough life force to use that.\n");
			do_char_log(cn, 0, "(Need %d base Hitpoints)\n",it[tmp].hp[2]);
			return -1;
		}
		/*
		if (it[tmp].end[2]>ch[cn].end[0])
		{
			do_char_log(cn, 0, "You don't have enough endurance to use that.\n");
			return -1;
		}
		*/
		if (it[tmp].mana[2]>ch[cn].mana[0])
		{
			do_char_log(cn, 0, "You don't have enough mana to use that.\n");
			do_char_log(cn, 0, "(Need %d base Mana)\n",it[tmp].mana[2]);
			return -1;
		}

		if ((IS_SKUAWEAP(tmp)   &&  IS_PURPLE(cn)) ||
			(it[tmp].temp==3201 &&  IS_PURPLE(cn)) ||
		    (IS_PURPWEAP(tmp)   && !IS_PURPLE(cn)) ||
			(it[tmp].temp==3202 && !IS_PURPLE(cn)) ||
		    (it[tmp].driver==40 && !IS_SEYAN_DU(cn)))
		{
			do_char_log(cn, 0, "Ouch. That hurt.\n");
			return -1;
		}

		if (it[tmp].min_rank>getrank(cn))
		{
			do_char_log(cn, 0, "You're not experienced enough to use that.\n");
			do_char_log(cn, 0, "(Need a rank of %s)\n",rank_name[it[tmp].min_rank]);
			return -1;
		}

		// check for correct placement:
		switch(n)
		{
		case    WN_HEAD:
			if (!(it[tmp].placement & PL_HEAD))
			{
				return -1;
			}
			else
			{
				break;
			}
		case    WN_NECK:
			if (!(it[tmp].placement & PL_NECK))
			{
				return -1;
			}
			else
			{
				break;
			}
		case    WN_BODY:
			if (!(it[tmp].placement & PL_BODY))
			{
				return -1;
			}
			else
			{
				break;
			}
		case    WN_ARMS:
			if (!(it[tmp].placement & PL_ARMS))
			{
				return -1;
			}
			else
			{
				break;
			}
		case    WN_BELT:
			if (!(it[tmp].placement & PL_BELT))
			{
				return -1;
			}
			else
			{
				break;
			}
		case    WN_CHARM:
		case    WN_CHARM2:
			if (!(it[tmp].placement & PL_CHARM))
			{
				return -1;
			}
			else
			{
				break;
			}
		case    WN_FEET:
			if (!(it[tmp].placement & PL_FEET))
			{
				return -1;
			}
			else
			{
				break;
			}
		case    WN_LHAND:
			if (!(it[tmp].placement & PL_SHIELD))
			{
				return -1;
			}
			if ((in = ch[cn].worn[WN_RHAND])!=0 && IS_TWOHAND(in))
			{
				return -1;
			}
			break;
		case    WN_RHAND:
			if (!(it[tmp].placement & PL_WEAPON) && !((it[tmp].flags & IF_OF_SHIELD) && IS_ARCHTEMPLAR(cn)))
			{
				return -1;
			}
			if (IS_TWOHAND(tmp) && ch[cn].worn[WN_LHAND])
			{
				return -1;
			}
			break;
		case    WN_CLOAK:
			if (!(it[tmp].placement & PL_CLOAK))
			{
				return -1;
			}
			else
			{
				break;
			}
		case    WN_RRING:
		case    WN_LRING:
			if (!(it[tmp].placement & PL_RING))
			{
				return -1;
			}
			else if (n==WN_RRING && (in = ch[cn].worn[WN_LRING])!=0 && IS_TWOHAND(in))
			{
				return -1;
			}
			else if (n==WN_LRING && (in = ch[cn].worn[WN_RRING])!=0 && IS_TWOHAND(in))
			{
				return -1;
			}
			else if (n==WN_RRING && IS_TWOHAND(tmp) && ch[cn].worn[WN_LRING])
			{
				return -1;
			}
			else if (n==WN_LRING && IS_TWOHAND(tmp) && ch[cn].worn[WN_RRING])
			{
				return -1;
			}
			else
			{
				break;
			}
		default:
			return -1;
		}
	}
	
	// Special case for charms - cannot remove one without help from NPC Bishop
	if (tmp && (n == WN_CHARM || n == WN_CHARM2))
	{
		do_char_log(cn, 0, "You cannot equip a card yourself. Seek the Bishop in the Temple of Skua for assistance.\n");
		return -1;
	}
	else if (n == WN_CHARM || n == WN_CHARM2)
	{
		do_char_log(cn, 0, "You cannot remove an equipped card yourself. Seek the Bishop in the Temple of Skua for assistance.\n");
		return -1;
	}
	
	// Special case for Ice Lotus
	if ((tmp && IS_ONLYONERING(tmp)) && 
		((n == WN_RRING && (it[tmp].temp == it[ch[cn].worn[WN_LRING]].temp || it[tmp].temp == it[ch[cn].worn[WN_LRING]].orig_temp) && IS_ONLYONERING(ch[cn].worn[WN_LRING])) || 
		 (n == WN_LRING && (it[tmp].temp == it[ch[cn].worn[WN_RRING]].temp || it[tmp].temp == it[ch[cn].worn[WN_RRING]].orig_temp) && IS_ONLYONERING(ch[cn].worn[WN_RRING]))))
	{
		do_char_log(cn, 4, "You may only equip one %s at a time.\n", it[tmp].name);
		return -1;
	}
	
	// Special case for Sinbinder
	if (tmp && IS_SINBINDER(tmp))
	{
		do_char_log(cn, 0, "You cannot equip this ring yourself. Seek the Priest in the Temple of the Purple One for assistance.\n");
		return -1;
	}
	else if ((n==WN_RRING && IS_SINBINDER(ch[cn].worn[WN_RRING])) || 
		(n==WN_LRING && IS_SINBINDER(ch[cn].worn[WN_LRING])))
	{
		do_char_log(cn, 0, "You cannot remove this ring yourself. Seek the Priest in the Temple of the Purple One for assistance.\n");
		return -1;
	}
	
	// Deactivate Gambler Fallacy upon removal
	if (n==WN_NECK && it[ch[cn].worn[WN_NECK]].temp==IT_GAMBLERFAL)
		it[ch[cn].worn[WN_NECK]].active = 0;
	
	// Deactivate Immolate when removing Rising Phoenix
	if ((n==WN_RHAND && (it[ch[cn].worn[WN_RHAND]].temp==IT_WP_RISINGPHO || it[ch[cn].worn[WN_RHAND]].orig_temp==IT_WP_RISINGPHO 
		              || it[ch[cn].worn[WN_RHAND]].temp==IT_WB_RISINGPHO || it[ch[cn].worn[WN_RHAND]].orig_temp==IT_WB_RISINGPHO)) || 
		(n==WN_LHAND && (it[ch[cn].worn[WN_LHAND]].temp==IT_WP_RISINGPHO || it[ch[cn].worn[WN_LHAND]].orig_temp==IT_WP_RISINGPHO 
		              || it[ch[cn].worn[WN_LHAND]].temp==IT_WB_RISINGPHO || it[ch[cn].worn[WN_LHAND]].orig_temp==IT_WB_RISINGPHO)))
	{
		if (has_buff(cn, SK_IMMOLATE))
		{
			do_char_log(cn, 1, "Immolate no longer active.\n");
			remove_buff(cn, SK_IMMOLATE);
		}
	}
	
	ch[cn].citem = ch[cn].worn[n];
	ch[cn].worn[n] = tmp;

	do_update_char(cn);

	return(n);
}

/* Check if cn may attack co. if (msg), tell cn if not. */
int may_attack_msg(int cn, int co, int msg)
{
	int cc=0, m1, m2;
	
	if (!IS_SANECHAR(cn) || !IS_SANECHAR(co))
		return -1;
	
	// unsafe gods may attack anyone
	if ((ch[cn].flags & CF_GOD && !(ch[cn].flags & CF_SAFE)))
		return 1;
	
	// unsafe gods may be attacked by anyone!
	if ((ch[co].flags & CF_GOD && !(ch[co].flags & CF_SAFE)))
		return 1;
	
	// player GC? act as if he would try to attack the master of the GC instead
	/*
	if (IS_COMPANION(cn) && ch[cn].data[64]==0)
	{
		if (!IS_SANECHAR(ch[cn].data[CHD_MASTER]))
			return 1;
		if (strcmp(get_area(cn, 0), get_area(ch[cn].data[CHD_MASTER], 0))!=0)
			return 1;
		cn = ch[cn].data[CHD_MASTER];
	}
	*/
	if (IS_COMPANION(cn) && ch[cn].data[64]==0 && IS_SANECHAR(ch[cn].data[CHD_MASTER]))
		cc = ch[cn].data[CHD_MASTER];

	// NPCs may attack anyone, anywhere
	if ((cc && !IS_PLAYER(cc)) || (!cc && !IS_PLAYER(cn)))
		return 1;

	// Check for NOFIGHT
	m1 = XY2M(ch[cn].x, ch[cn].y);
	m2 = XY2M(ch[co].x, ch[co].y);
	if (((map[m1].flags | map[m2].flags) & MF_NOFIGHT) && !(IS_IN_BOJ(ch[cn].x, ch[cn].y) && get_gear(cn, IT_XIXDARKSUN)))
	{
		if (msg) do_char_log(cn, 0, "You can't attack anyone here!\n");
		return 0;
	}
	
	if (ch[co].temp==CT_ANNOU1 || ch[co].temp==CT_ANNOU2 || ch[co].temp==CT_ANNOU3 || ch[co].temp==CT_ANNOU4 || ch[co].temp==CT_ANNOU5)
	{
		if (msg) do_char_log(cn, 0, "Come on, you're better than that.\n");
		return 0;
	}
	
	if (ch[co].temp==CT_NULLAN || ch[co].temp==CT_DVOID)
	{
		if (msg) do_char_log(cn, 0, "Oh. Well then.\n");
		fx_add_effect(12, 0, ch[co].x, ch[co].y, 0);
		reset_char(ch[co].temp);
		return 0;
	}
	
	if (IS_THRALL(cn) || IS_THRALL(co))
		return 1;

	// player GC? act as if he would try to attack the master of the GC instead
	if (IS_COMPANION(co))
	{
		co = ch[co].data[CHD_MASTER];
		if (!IS_SANECHAR(co))
			return 1;             // um, lets him try to kill this GC - it's got bad values anway
	}

	// Check for plr-npc (OK)
	if (!IS_PLAYER(cn) || !IS_PLAYER(co))
		return 1;

	// Both are players. Check for Arena (OK)
	if (map[m1].flags & map[m2].flags & MF_ARENA)
		return 1;

	// Check clan warfare
	if (((!IS_CLANKWAI(cn) && !IS_CLANGORN(cn)) || (IS_CLANKWAI(cn) && !IS_CLANGORN(co)) || (IS_CLANGORN(cn) && !IS_CLANKWAI(co))) || 
		(cc && ((!IS_CLANKWAI(cc) && !IS_CLANGORN(cc)) || (IS_CLANKWAI(cc) && !IS_CLANGORN(co)) || (IS_CLANGORN(cc) && !IS_CLANKWAI(co))) ))
	{
		if (msg) do_char_log(cn, 0, "You can't attack other players! You're not a member of an opposing clan.\n");
		return 0;
	}

	// Check if victim is purple
	if (!IS_CLANKWAI(co) && !IS_CLANGORN(co))
	{
		if (msg) do_char_log(cn, 0, "You can't attack %s! %s's not a member of an opposing clan.\n", ch[co].name, IS_FEMALE(co) ? "She" : "He");
		return 0;
	}

	if ((!cc && abs(getrank(cn) - getrank(co))>3) || (cc && abs(getrank(cc) - getrank(co))>3))
	{
		if (msg) do_char_log(cn, 0, "You're not allowed to attack %s. The rank difference is too large.\n", ch[co].name);
		return 0;
	}

	return 1;
}

#

void do_check_new_item_level(int cn, int in)
{
	int temp, n, m, bonus = 1, rank, weapon=0, armor=0, ench=0;
	int parr=0, intu=0, will=0, topd=0, mult=0, atks=0, xtra=0;
	int stre=0, agil=0, brav=0;
	char osir[20]; 
	
	if (!IS_PLAYER(cn)) return;
	if (!(temp = it[in].orig_temp)) return;
	if (IS_TWOHAND(in)) bonus = 2;
	
	if (IS_GODWEAPON(in))
	{
		xlog("Rolling God blessing for %s", it[in].name);
		
		rank = getitemrank(in, 0);
		
		if (rank < 2) return;
		if (it[in].stack >= rank) return;
		
		armor  = it_temp[temp].armor[0];
		weapon = it_temp[temp].weapon[0];
		parr   = it_temp[temp].to_parry[0];
		intu   = it_temp[temp].attrib[AT_INT][0];
		will   = it_temp[temp].attrib[AT_WIL][0];
		topd   = it_temp[temp].top_damage[0];
		mult   = it_temp[temp].crit_multi[0];
		atks   = it_temp[temp].atk_speed[0];
		
		
		if ((temp >= 284 && temp <= 292) || temp == 1779) // Steel					2
		{
			if (armor && weapon)
			{
				armor  = max(rank-1,  armor*5/2 * rank*rank/100) + 1 + 1 * bonus;
				weapon = max(rank-1, weapon*5/2 * rank*rank/100) + 1 + 1 * bonus;
			}
			else if (armor) 
				armor  = max(rank-1,  armor*5/2 * rank*rank/100) + 2 + 2 * bonus;
			else if (weapon)
				weapon = max(rank-1, weapon*5/2 * rank*rank/100) + 2 + 2 * bonus;
			
			if (parr) parr = parr*5/2 * rank*rank/100;
			if (intu) intu = intu*5/2 * rank*rank/100;
			if (will) will = will*5/2 * rank*rank/100;
			if (topd) topd = topd*5/2 * rank*rank/100;
			if (mult) mult = mult*5/2 * rank*rank/100;
			if (atks) atks = atks*5/2 * rank*rank/100;
			
			xtra = 4 * rank*rank/100;
		}
		else if ((temp >= 523 && temp <= 531) || temp == 1780) // Gold				3
		{
			if (armor && weapon)
			{
				armor  = max(rank-1,  armor*4/3 * rank*rank/100) + 1 + 1 * bonus;
				weapon = max(rank-1, weapon*4/3 * rank*rank/100) + 1 + 1 * bonus;
			}
			else if (armor) 
				armor  = max(rank-1,  armor*4/3 * rank*rank/100) + 2 + 2 * bonus;
			else if (weapon)
				weapon = max(rank-1, weapon*4/3 * rank*rank/100) + 2 + 2 * bonus;
			
			if (parr) parr = parr*4/3 * rank*rank/100;
			if (intu) intu = intu*4/3 * rank*rank/100;
			if (will) will = will*4/3 * rank*rank/100;
			if (topd) topd = topd*4/3 * rank*rank/100;
			if (mult) mult = mult*4/3 * rank*rank/100;
			if (atks) atks = atks*4/3 * rank*rank/100;
			
			xtra = 4 * rank*rank/100;
		}
		else if ((temp >= 532 && temp <= 540) || temp == 1781) // Emerald			4
		{
			if (armor && weapon)
			{
				armor  = max(rank/2,  armor*3/4 * rank*rank/100) + 1 + 1 * bonus;
				weapon = max(rank/2, weapon*3/4 * rank*rank/100) + 1 + 1 * bonus;
			}
			else if (armor) 
				armor  = max(rank/2,  armor*3/4 * rank*rank/100) + 2 + 2 * bonus;
			else if (weapon)
				weapon = max(rank/2, weapon*3/4 * rank*rank/100) + 2 + 2 * bonus;
			
			if (parr) parr = parr*3/4 * rank*rank/100;
			if (intu) intu = intu*3/4 * rank*rank/100;
			if (will) will = will*3/4 * rank*rank/100;
			if (topd) topd = topd*3/4 * rank*rank/100;
			if (mult) mult = mult*3/4 * rank*rank/100;
			if (atks) atks = atks*3/4 * rank*rank/100;
			
			xtra = 4 * rank*rank/100;
		}
		else if ((temp >= 541 && temp <= 549) || temp == 1782) // Crystal			5
		{
			if (armor && weapon)
			{
				armor  = max(rank/2,  armor*2/5 * rank*rank/100) + 1 + 1 * bonus;
				weapon = max(rank/2, weapon*2/5 * rank*rank/100) + 1 + 1 * bonus;
			}
			else if (armor) 
				armor  = max(rank/2,  armor*2/5 * rank*rank/100) + 2 + 2 * bonus;
			else if (weapon)
				weapon = max(rank/2, weapon*2/5 * rank*rank/100) + 2 + 2 * bonus;
			
			if (parr) parr = parr*2/5 * rank*rank/100;
			if (intu) intu = intu*2/5 * rank*rank/100;
			if (will) will = will*2/5 * rank*rank/100;
			if (topd) topd = topd*2/5 * rank*rank/100;
			if (mult) mult = mult*2/5 * rank*rank/100;
			if (atks) atks = atks*2/5 * rank*rank/100;
			
			xtra = 4 * rank*rank/100;
		}
		else if ((temp >= 572 && temp <= 580) || temp == 1783) // Titanium			6
		{
			if (armor && weapon)
			{
				armor  = max((rank+1)/3,  armor/6 * rank*rank/100) + 1 + 1 * bonus;
				weapon = max((rank+1)/3, weapon/6 * rank*rank/100) + 1 + 1 * bonus;
			}
			else if (armor) 
				armor  = max((rank+1)/3,  armor/6 * rank*rank/100) + 2 + 2 * bonus;
			else if (weapon)
				weapon = max((rank+1)/3, weapon/6 * rank*rank/100) + 2 + 2 * bonus;
			
			if (parr) parr = parr/6 * rank*rank/100;
			if (intu) intu = intu/6 * rank*rank/100;
			if (will) will = will/6 * rank*rank/100;
			if (topd) topd = topd/6 * rank*rank/100;
			if (mult) mult = mult/6 * rank*rank/100;
			if (atks) atks = atks/6 * rank*rank/100;
			
			xtra = 4 * rank*rank/100;
		}
		else if ((temp >= 693 && temp <= 701) || temp == 1784) // Adamantium		6.5
		{
			if (armor && weapon)
			{
				armor  = max((rank+1)/3,  armor/13 * rank*rank/100) + 1 + 1 * bonus;
				weapon = max((rank+1)/3, weapon/13 * rank*rank/100) + 1 + 1 * bonus;
			}
			else if (armor) 
				armor  = max((rank+1)/3,  armor/13 * rank*rank/100) + 2 + 2 * bonus;
			else if (weapon)
				weapon = max((rank+1)/3, weapon/13 * rank*rank/100) + 2 + 2 * bonus;
			
			if (IS_WPDAGGER(in))  will -= 4;
			if (IS_WPSTAFF(in))   intu -= 4;
			if (IS_WPSPEAR(in)) { will -= 2; intu -= 2; }
			if (IS_WPSHIELD(in))  will -= 2;
			
			if (parr) parr = parr/6 * rank*rank/100;
			if (intu) intu = intu/6 * rank*rank/100;
			if (will) will = will/6 * rank*rank/100;
			if (topd) topd = topd/6 * rank*rank/100;
			if (mult) mult = mult/6 * rank*rank/100;
			if (atks) atks = atks/6 * rank*rank/100;
		}
		else
		{
			// How did we get here?
		}
		if (armor)  it[in].ss_armor          = armor;
		if (weapon) it[in].ss_weapon         = weapon;
		if (xtra)
		{
			if      (IS_WPGAXE(in) || IS_WPAXE(in))       { stre += xtra;   agil += xtra/2; }
			else if (IS_WPTWOHAND(in) || IS_WPCLAW(in))   { agil += xtra;   stre += xtra/2; }
			else if (IS_WPSWORD(in) || IS_WPDUALSW(in))   { brav += xtra/2; agil += xtra/2;   stre += xtra/2; }
			else if (IS_WPSHIELD(in))                     { brav += xtra/2; will += xtra/2;   stre += xtra/2; }
			else if (IS_WPSPEAR(in))                      { will += xtra/2; intu += xtra/2;   agil += xtra/4;   stre += xtra/4; }
			else if (IS_WPSTAFF(in))                      { intu += xtra;   stre += xtra/2; }
			else if (IS_WPDAGGER(in))                     { will += xtra;   agil += xtra/2; }
		}
		if (armor)  it[in].armor[0]          = it_temp[temp].armor[0]          + armor;
		if (weapon) it[in].weapon[0]         = it_temp[temp].weapon[0]         + weapon;
		if (parr)   it[in].to_parry[0]       = it_temp[temp].to_parry[0]       + parr;
		if (brav)   it[in].attrib[AT_BRV][0] = it_temp[temp].attrib[AT_BRV][0] + brav;
		if (will)   it[in].attrib[AT_WIL][0] = it_temp[temp].attrib[AT_WIL][0] + will;
		if (intu)   it[in].attrib[AT_INT][0] = it_temp[temp].attrib[AT_INT][0] + intu;
		if (agil)   it[in].attrib[AT_AGL][0] = it_temp[temp].attrib[AT_AGL][0] + agil;
		if (stre)   it[in].attrib[AT_STR][0] = it_temp[temp].attrib[AT_STR][0] + stre;
		if (topd)   it[in].top_damage[0]     = it_temp[temp].top_damage[0]     + topd;
		if (mult)   it[in].crit_multi[0]     = it_temp[temp].crit_multi[0]     + mult;
		if (atks)   it[in].atk_speed[0]      = it_temp[temp].atk_speed[0]      + atks;
		
		if (IS_SKUAWEAP(in))
		{
			it[in].speed[0]      = it_temp[temp].speed[0]      +  7 * bonus * rank/10 +  9 * bonus/2;
			ench = 57 + RANDOM(4);
		}
		else if (IS_GORNWEAP(in))
		{
			it[in].spell_mod[0]  = it_temp[temp].spell_mod[0]  +  2 * bonus * rank/10 +  3 * bonus/2;
			ench = 65 + RANDOM(4);
		}
		else if (IS_KWAIWEAP(in))
		{
			it[in].to_hit[0]     = it_temp[temp].to_hit[0]     +  3 * bonus * rank/20 +  2 * bonus;
			it[in].to_parry[0]   = it_temp[temp].to_parry[0]   +  3 * bonus * rank/20 +  2 * bonus;
			ench = 61 + RANDOM(4);
		}
		else if (IS_PURPWEAP(in))
		{
			it[in].top_damage[0] = it_temp[temp].top_damage[0] + 30 * bonus * rank/10 + 15 * bonus;
			ench = 69 + RANDOM(4);
		}
		
		char_play_sound(cn, ch[cn].sound + 23, -50, 0);
		do_char_log(cn, 9, "Your %s rose a level!\n", it[in].name);
		do_char_log(cn, 9, "It is now level %d.\n", rank);
		it[in].stack = rank;
		it[in].min_rank = 1+rank*3/2;
		it[in].power = it_temp[temp].power + (300 - it_temp[temp].power)*rank/10;
		it[in].current_damage = 0;
		it[in].current_age[0] = 0;
		it[in].current_age[1] = 0;
		it[in].damage_state = 0;
		it[in].max_damage = 0;
		it[in].flags &= ~(IF_LEGACY);
		
		if (rank == 10)
		{
			if (IS_OFFHAND(in)) ench = 73 + RANDOM(4);
			
			it[in].flags &= ~(IF_KWAI_UNI | IF_GORN_UNI | IF_PURP_UNI);
			it[in].enchantment = ench;
			it[in].flags |= IF_ENCHANTED | IF_LOOKSPECIAL;
			do_char_log(cn, 2, "Impressed with your efforts, the gods' fleeting blessing has revealed a special enchantment...\n");
		}
	}
	else if (IS_OSIRWEAP(in))
	{
		xlog("Rolling Osiris blessing for %s", it[in].name);
		
		rank = getitemrank(in, it[in].data[0]);
		
		if (it[in].stack == 0) return;
		if (it[in].data[4] >= rank) return;
		
		if (!it[in].data[1])
		{
			m = 1+RANDOM(50+5+20+4); it[in].data[1] = m;
			for (n=0;n<99;n++) { m = 1+50+RANDOM(5+20+4); if ((it[in].data[2] = m)!=it[in].data[1]) break; }
			for (n=0;n<99;n++) { m = 1+50+5+RANDOM(20+4); if ((it[in].data[3] = m)!=it[in].data[1] && m!=it[in].data[2]) break; }
		}
		
		m = 1+RANDOM(3);
		
		xlog(" ** m = %d (%d)", m, it[in].data[m]);
		
		if ((it[in].data[m]-1)<50)
		{
			n = it[in].data[m]-1;
			if (is_half_soul(n)) it[in].skill[n][0] += 1 + 1 * bonus;
			else 				 it[in].skill[n][0] += 1 + 2 * bonus;
			strcpy(osir, skilltab[n].name);
		}
		else if ((it[in].data[m]-1)<55)
		{
			n = it[in].data[m]-51;
			it[in].attrib[n][0] += 1 + 1 * bonus;
			strcpy(osir, at_name[n]);
		}
		else if ((it[in].data[m]-1)>=55+ 0 && (it[in].data[m]-1)<=55+ 4)
		{
			it[in].aoe_bonus[0] += 1 * bonus;
			strcpy(osir, "Area of Effect");
		}
		else if ((it[in].data[m]-1)>=55+ 5 && (it[in].data[m]-1)<=55+ 8)
		{
			it[in].atk_speed[0] += 1 + 2 * bonus;
			strcpy(osir, "Attack Speed");
		}
		else if ((it[in].data[m]-1)>=55+ 9 && (it[in].data[m]-1)<=55+12)
		{
			it[in].cast_speed[0] += 2 + 2 * bonus;
			strcpy(osir, "Cast Speed");
		}
		else if ((it[in].data[m]-1)>=55+13 && (it[in].data[m]-1)<=55+15)
		{
			it[in].cool_bonus[0] += 1 + 1 * bonus;
			strcpy(osir, "Cooldown Rate");
		}
		else if ((it[in].data[m]-1)>=55+16 && (it[in].data[m]-1)<=55+17)
		{
			it[in].crit_chance[0] += 10 * bonus;
			strcpy(osir, "Crit Chance");
		}
		else if ((it[in].data[m]-1)>=55+18 && (it[in].data[m]-1)<=55+19)
		{
			it[in].crit_multi[0] += 5 * bonus;
			strcpy(osir, "Crit Multiplier");
		}
		else if ((it[in].data[m]-1)==75)
		{
			it[in].aoe_bonus[0] += 1;
			strcpy(osir, "Base Crit Chance");
		}
		else if ((it[in].data[m]-1)==76)
		{
			it[in].weapon[0] += 2 * bonus;
			strcpy(osir, "Weapon Value");
		}
		else if ((it[in].data[m]-1)==77)
		{
			it[in].armor[0] += 2 * bonus;
			strcpy(osir, "Armor Value");
		}
		else if ((it[in].data[m]-1)==78)
		{
			it[in].dmg_bonus[0] += 1 + 1 * bonus;
			strcpy(osir, "Bonus Damage");
		}
		else if ((it[in].data[m]-1)==79)
		{
			it[in].dmg_reduction[0] += 1 + 1 * bonus;
			strcpy(osir, "Damage Reduction");
		}
		else // Fail state
		{
			it[in].spell_apt[0] += 1 + 2 * bonus;
			strcpy(osir, "Spell Aptitude");
		}
		
		char_play_sound(cn, ch[cn].sound + 23, -50, 0);
		switch (RANDOM(6))
		{
			case  1: do_char_log(cn, 9, "Osiris blessed your %s with extra %s.\n", it[in].name, osir); break;
			case  2: do_char_log(cn, 9, "Osiris tapped your %s and gave it extra %s.\n", it[in].name, osir); break;
			case  3: do_char_log(cn, 9, "Osiris honed some extra %s onto your %s.\n", osir, it[in].name); break;
			case  4: do_char_log(cn, 9, "Osiris nodded, and extra %s appeared on your %s.\n", osir, it[in].name); break;
			case  5: do_char_log(cn, 9, "Osiris winked, and your %s was granted extra %s.\n", it[in].name, osir); break;
			default: do_char_log(cn, 9, "Osiris stitched some extra %s onto your %s.\n", osir, it[in].name); break;
		}
		it[in].stack--;
		it[in].data[4]++;
		it[in].power += 15;
	}
	else return;
	
	it[in].flags |= IF_UPDATE;
	do_update_char(cn);
}

void do_check_new_level(int cn, int announce)
{
	int hp = 0, mana = 0, attri = 0, diff, rank, temp, n, oldrank, bits;

	if (!IS_PLAYER(cn))
	{
		return;
	}

	rank = getrank(cn);

	if (ch[cn].data[45]<rank)
	{
		chlog(cn, "gained level (%d -> %d)", ch[cn].data[45], rank);
		
		if (IS_ANY_MERC(cn) || IS_SEYAN_DU(cn) || IS_BRAVER(cn))
		{
			hp   = 10;
			mana = 10;
		}
		else if (IS_ANY_TEMP(cn) || (IS_LYCANTH(cn)&&!IS_SHIFTED(cn)))
		{
			hp   = 15;
			mana = 5;
		}
		else if (IS_ANY_HARA(cn) || (IS_LYCANTH(cn)&&IS_SHIFTED(cn)))
		{
			hp   = 5;
			mana = 15;
		}
		if (rank >= 20)
		{
			attri = 1;
		}

		diff = rank - ch[cn].data[45];
		oldrank = ch[cn].data[45];
		ch[cn].data[45] = rank;

		if (diff==1)
		{
			if (hp && oldrank<=20)
			{
				do_char_log(cn, 0, 
					"You rose a level! Congratulations! You received %d extra hitpoints and %d mana.\n",
					hp, mana);
			}
			if (oldrank<20 && rank>=20)
			{
				do_char_log(cn, 0, 
					"You've entered the ranks of nobility! Great work! Your maximum attributes each rose by %d.\n",
					attri);
			}
			else if (attri)
			{
				do_char_log(cn, 0, 
					"You rose a level! Congratulations! Your maximum attributes each rose by %d.\n",
					attri);
			}
		}
		else
		{
			if (hp && oldrank<=20)
			{
				do_char_log(cn, 0, 
					"You rose %d levels! Congratulations! You received %d extra hitpoints and %d mana.\n",
					diff, hp * diff, mana * diff);
			}
			diff = rank - max(19,oldrank);
			if (oldrank<20 && rank>=20)
			{
				do_char_log(cn, 0, 
					"You've entered the ranks of nobility! Great work! Your maximum attributes each rose by %d.\n",
					attri * diff);
			}
			else if (attri)
			{
				do_char_log(cn, 0, 
					"You rose %d levels! Congratulations! Your maximum attributes each rose by %d.\n",
					diff, attri * diff);
			}
		}
		char_play_sound(cn, ch[cn].sound + 23, -50, 0);
		
		if (rank>=5 && oldrank<5) // Warn players that death can happen now!
		{
			do_char_log(cn, 0, "Confident with your progress, the gods will no longer return your items when you die. Be careful!\n");
		}
		
		if (announce)
		{
			/* Announce the player's new rank */
			if (IS_PURPLE(cn))			temp = CT_PRIEST;
			else if (IS_CLANKWAI(cn))	temp = CT_KWAIVICAR;
			else if (IS_CLANGORN(cn))	temp = CT_GORNPASTOR;
			else						temp = CT_BISHOP;
			// Find a character with appropriate template
			for (n = 1; n<MAXCHARS; n++)
			{
				if (ch[n].used!=USE_ACTIVE)	continue;
				if (ch[n].flags & CF_BODY)	continue;
				if (ch[n].temp == temp)		break;
			}
			// Have him yell it out
			if (n<MAXCHARS)
			{
				char message[100];
				sprintf(message, "Hear ye, hear ye! %s has attained the rank of %s!",
						ch[cn].name, rank_name[rank]);
				do_shout(n, message);
				if (globs->flags & GF_DISCORD) discord_ranked(message);
			}
		}

		ch[cn].hp[1]   = hp * min(20,rank);
		ch[cn].mana[1] = mana * min(20,rank);
		
		if (attri)
		{
			bits = get_rebirth_bits(cn);
			temp = ch[cn].temp;
			for (n = 0; n<5; n++) 
				ch[cn].attrib[n][2] = ch_temp[temp].attrib[n][2] + attri*min(5, max(0,rank-19)) + min(5,max(0,(bits+5-n)/6));
		}
		
		do_update_char(cn);
	}
}

/* CS, 991103: Tell when a certain player last logged on. */
void do_seen(int cn, char *cco)
{
	int co;
	time_t last_date, current_date;
	int days, hoursbefore;
	char *when;
	char  interval[50];

	if (!*cco)
	{
		do_char_log(cn, 0, "When was WHO last seen?\n");
		return;
	}

	// numeric only for deities
	if (isdigit(*cco) && ((ch[cn].flags & (CF_IMP | CF_GOD | CF_USURP)) == 0))
	{
		co = 0;
	}
	else
	{
		co = dbatoi_self(cn, cco);
	}

	if (!co)
	{
		do_char_log(cn, 0, "I've never heard of %s.\n", cco);
		return;
	}

	if (!IS_PLAYER(co))
	{
		do_char_log(cn, 0, "%s is not a player.\n", ch[co].name);
		return;
	}

	if (!(ch[cn].flags & CF_GOD) && (ch[co].flags & CF_GOD))
	{
		do_char_log(cn, 0, "No one knows when the gods where last seen.\n");
		return;
	}

	if (ch[cn].flags & (CF_IMP | CF_GOD))
	{
		time_t last, now;
		struct tm tlast, tnow, *tmp;

		last = max(ch[co].login_date, ch[co].logout_date);
		now  = time(NULL);

		tmp = localtime(&last);
		tlast = *tmp;
		tmp  = localtime(&now);
		tnow = *tmp;

		do_char_log(cn, 2, "%s was last seen on %04d-%02d-%02d %02d:%02d:%02d (time now: %04d-%02d-%02d %02d:%02d:%02d)\n",
		            ch[co].name,
		            tlast.tm_year + 1900,
		            tlast.tm_mon + 1,
		            tlast.tm_mday,
		            tlast.tm_hour,
		            tlast.tm_min,
		            tlast.tm_sec,
		            tnow.tm_year + 1900,
		            tnow.tm_mon + 1,
		            tnow.tm_mday,
		            tnow.tm_hour,
		            tnow.tm_min,
		            tnow.tm_sec);

		if (ch[co].used==USE_ACTIVE && !(ch[co].flags & CF_INVISIBLE))
		{
			do_char_log(cn, 2, "PS: %s is online right now!\n", ch[co].name);
		}
	}
	else
	{
		last_date = max(ch[co].login_date, ch[co].logout_date) / MD_DAY;
		current_date = time(NULL) / MD_DAY;
		days = current_date - last_date;
		switch (days)
		{
			case 0:
				last_date = max(ch[co].login_date, ch[co].logout_date) / MD_HOUR;
				current_date = time(NULL) / MD_HOUR;
				hoursbefore = current_date - last_date;
				switch (hoursbefore)
				{
					case 0: when = "a few minutes ago"; break;
					case 1:	when = "an hour ago"; break;
					case 2: case 3: case 4:	when = "a few hours ago"; break;
					default: when = "earlier today"; break;
				}
				break;
			case 1:	when = "yesterday";	break;
			case 2:	when = "the day before yesterday";	break;
			default:
				sprintf(interval, "%d days ago", days);
				when = interval;
				break;
		}
		do_char_log(cn, 1, "%s was last seen %s.\n", ch[co].name, when);
	}
}

/* CS, 991204: Do not fight back if spelled */
void do_spellignore(int cn)
{
	ch[cn].flags ^= CF_SPELLIGNORE;
	if (ch[cn].flags & CF_SPELLIGNORE)
	{
		do_char_log(cn, 1, "Now ignoring spell attacks.\n");
	}
	else
	{
		do_char_log(cn, 1, "Now reacting to spell attacks.\n");
	}
}


/* CS, 000209: Remember PvP attacks */
void remember_pvp(int cn, int co)
{
	unsigned long long mf;
	mf = map[XY2M(ch[cn].x, ch[cn].y)].flags;
	mf &= map[XY2M(ch[co].x, ch[co].y)].flags;
	if (mf & MF_ARENA)
	{
		return;            // Arena attacks don't count
	}
	/* Substitute masters for companions, some sanity checks */
	if (!IS_SANEUSEDCHAR(cn))
	{
		return;
	}
	if (IS_COMPANION(cn))
	{
		cn = ch[cn].data[CHD_MASTER];
	}
	if (!IS_SANEPLAYER(cn))
	{
		return;
	}
	if (!IS_OPP_CLAN(cn, co))
	{
		return;
	}

	if (!IS_SANEUSEDCHAR(co))
	{
		return;
	}
	if (IS_COMPANION(co))
	{
		co = ch[co].data[CHD_MASTER];
	}
	if (!IS_SANEPLAYER(co))
	{
		return;
	}

	if (cn == co)
	{
		return;
	}

	ch[cn].data[PCD_ATTACKTIME] = globs->ticker;
	ch[cn].data[PCD_ATTACKVICT] = co;
}

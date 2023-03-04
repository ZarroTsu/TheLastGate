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
		if (do_char_can_see(npcs[j], cn))
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
			do_char_log(cn, 3, "  \"%s\"\n", ch[co].text[0]);
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
	do_char_log(co, 3, "%s", buf);

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
		it[ch[cn].worn[WN_LHAND]].temp==IT_WP_RISINGPHO || it[ch[cn].worn[WN_LHAND]].orig_temp==IT_WP_RISINGPHO)
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
	
	/*
	if (ch[cn].a_end<10000)
	{
		do_char_log(cn, 0, "You're too exhausted to shout!\n");
		return;
	}
	*/

	if (ch[cn].flags & CF_SHUTUP)
	{
		do_char_log(cn, 0, "You try to shout, but you only produce a croaking sound.\n");
		return;
	}

	/*
	ch[cn].a_end -= 10000;
	*/

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
				do_char_log(co, 2, "%s group-tells: \"%s\"\n", ch[cn].name, text);
				found = 1;
			}
		}
	}
	if (found)
	{
		do_char_log(cn, 2, "Told the group: \"%s\"\n", text);
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
			do_char_log(cn, 1, "#weapon <type>         list weapon stats.\n");
			do_char_log(cn, 1, "#who                   see who's online.\n");
			do_char_log(cn, 1, "#zircon                list zircon rings.\n");
			if (ch[cn].flags & (CF_POH_LEADER))
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
			do_char_log(cn, 1, "#wave                  you'll wave.\n");
		}
		else if (strcmp(topic, "4")==0)
		{
			pagenum = 4;
			do_char_log(cn, 1, "The following commands are available (PAGE 4):\n");
			do_char_log(cn, 1, " \n");
			//                 "!        .         .   |     .         .        !"
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
			do_char_log(cn, 1, "#sort <order>          sort inventory.\n");
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
			do_char_log(cn, 1, "#lag <seconds>         lag control.\n");
			do_char_log(cn, 1, "#listskills <page>     list skill attributes.\n");
			do_char_log(cn, 1, "#max                   list character maximums.\n");
			do_char_log(cn, 1, "#noshout               you won't hear shouts.\n");
			do_char_log(cn, 1, "#notell                you won't hear tells.\n");
			do_char_log(cn, 1, "#opal                  list opal rings.\n");
			do_char_log(cn, 1, "#override              you always spell self.\n");
			do_char_log(cn, 1, "#poles <page>          lists unattained poles.\n");
			do_char_log(cn, 1, "#quest <page>          list available quests.\n");
		}
		else if (strcmp(topic, "2")==0)
		{
			pagenum = 2;
			do_char_log(cn, 1, "The following commands are available (PAGE 2):\n");
			do_char_log(cn, 1, " \n");
			//                 "!        .         .   |     .         .        !"
			do_char_log(cn, 1, "#buff                  display buff timers.\n");
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
		do_char_log(cn, (B_AT(co, n)>=ch[co].attrib[n][2]+(archbonus-n*2)/10)?2:1, 
		"%20s  %3d  %3d\n", 
		at_name[n], B_AT(co, n), ch[co].attrib[n][2]+(archbonus-n*2)/10);
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
				do_char_log(cn, (B_SK(co, n)>=ch[co].skill[n][2]+archbonus*3/20)?2:1, 
					"%20s  %3d  %3d\n", 
					skilltab[n].name, B_SK(co, n), ch[co].skill[n][2]+archbonus*3/20);
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
	if (strcmp(topic, "3")==0)
	{
		pagenum = 3;
		do_char_log(cn, 1, "Now listing skill attributes (PAGE 3):\n");
		do_char_log(cn, 1, " \n");
		//                 "!        .         .   |     .         .        !"
		do_char_log(cn, 1, "Aria                   BRV + AGL + AGL\n");
		do_char_log(cn, 5, "Bartering              INT + INT + AGL\n");
		do_char_log(cn, 1, "Companion Mastery      BRV + WIL + WIL\n");
		do_char_log(cn, 5, "Concentrate            WIL + WIL + WIL\n");
		do_char_log(cn, 1, "Gear Mastery           BRV + AGL + STR\n");
		do_char_log(cn, 5, "Immunity               WIL + AGL + STR\n");
		do_char_log(cn, 1, "Perception             INT + INT + AGL\n");
		do_char_log(cn, 5, "Precision              BRV + BRV + WIL\n");
		do_char_log(cn, 1, "Proximity              BRV + WIL + INT\n");
		do_char_log(cn, 5, "Resistance             BRV + WIL + STR\n");
		do_char_log(cn, 1, "Safeguard              BRV + STR + STR\n");
		do_char_log(cn, 5, "Stealth                INT + AGL + AGL\n");
		do_char_log(cn, 1, "Surround Hit           BRV + AGL + STR\n");
		do_char_log(cn, 5, "Swimming               BRV + WIL + AGL\n");
	}
	else if (strcmp(topic, "2")==0)
	{
		pagenum = 2;
		do_char_log(cn, 1, "Now listing skill attributes (PAGE 2):\n");
		do_char_log(cn, 1, " \n");
		//                 "!        .         .   |     .         .        !"
		do_char_log(cn, 1, "Blast                  BRV + INT + INT\n");
		do_char_log(cn, 5, "Bless                  BRV + WIL + WIL\n");
		do_char_log(cn, 1, "Curse                  BRV + INT + INT\n");
		do_char_log(cn, 5, "Dispel                 BRV + WIL + INT\n");
		do_char_log(cn, 1, "Enhance                BRV + WIL + WIL\n");
		do_char_log(cn, 5, "Ghost Companion        BRV + WIL + INT\n");
		do_char_log(cn, 1, "Haste                  BRV + WIL + AGL\n");
		do_char_log(cn, 5, "Heal                   BRV + INT + STR\n");
		do_char_log(cn, 1, "Identify               BRV + WIL + INT\n");
		do_char_log(cn, 5, "Lethargy               BRV + WIL + INT\n");
		do_char_log(cn, 1, "Magic Shield           BRV + WIL + WIL\n");
		do_char_log(cn, 5, "Poison                 BRV + INT + INT\n");
		do_char_log(cn, 1, "Protect                BRV + WIL + WIL\n");
		do_char_log(cn, 5, "Pulse                  BRV + INT + INT\n");
		do_char_log(cn, 1, "Shadow Copy            BRV + WIL + WIL\n");
		do_char_log(cn, 5, "Slow                   BRV + INT + INT\n");
		do_char_log(cn, 1, "Zephyr                 BRV + AGL + STR\n");
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
		do_char_log(cn, 5, "Hand to Hand           AGL + AGL + STR\n");
		do_char_log(cn, 1, "Axe                    AGL + STR + STR\n");
		do_char_log(cn, 5, "Dagger                 WIL + WIL + AGL\n");
		do_char_log(cn, 1, "Dual Wield             BRV + AGL + STR\n");
		do_char_log(cn, 5, "Shield                 BRV + WIL + STR\n");
		do_char_log(cn, 1, "Staff                  INT + INT + STR\n");
		do_char_log(cn, 5, "Sword                  BRV + AGL + STR\n");
		do_char_log(cn, 1, "Two-Handed             AGL + AGL + STR\n");
		do_char_log(cn, 5, "Blind                  BRV + INT + AGL\n");
		do_char_log(cn, 1, "Cleave                 BRV + STR + STR\n");
		do_char_log(cn, 5, "Leap                   BRV + AGL + AGL\n");
		do_char_log(cn, 1, "Rage                   BRV + BRV + STR\n");
		do_char_log(cn, 5, "Repair                 INT + STR + STR\n");
		do_char_log(cn, 1, "Taunt                  BRV + STR + STR\n");
		do_char_log(cn, 5, "Warcry                 BRV + STR + STR\n");
		do_char_log(cn, 1, "Weaken                 BRV + AGL + AGL\n");
	}
	do_char_log(cn, 1, " \n");
	do_char_log(cn, 2, "Showing page %d of 3. #listskills <x> to swap.\n", pagenum);
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
		do_char_log(cn, 1, "   Weapon   |   Requirements: |     Gives:     \n");
		do_char_log(cn, 1, "    Tier    | AGL | STR | Skl | WV | CrM | ASp \n");
		do_char_log(cn, 1, "------------+-----+-----+-----+----+-----+-----\n");
		do_char_log(cn, 1, "Bronze      |   1 |   1 |   1 | 10 |     |   1 \n");
		do_char_log(cn, 1, "Steel       |  15 |  12 |  16 | 20 |   5 |   2 \n");
		do_char_log(cn, 1, "Gold        |  25 |  18 |  32 | 30 |  10 |   3 \n");
		do_char_log(cn, 1, "Emerald     |  40 |  28 |  48 | 40 |  15 |   4 \n");
		do_char_log(cn, 1, "Crystal     |  60 |  42 |  64 | 50 |  20 |   5 \n");
		do_char_log(cn, 1, "Titanium    |  85 |  60 |  80 | 60 |  25 |   6 \n");
		do_char_log(cn, 1, " \n");
		do_char_log(cn, 2, "* Claws will use both hand slots.\n");
		do_char_log(cn, 2, "* Claws have a critical hit chance of 3%%.\n");
		do_char_log(cn, 1, " \n");
	}
	else if (strcmp(topic, "1")==0 
		|| strcmp(topic, "DAGGER")==0 || strcmp(topic, "DAGGERS")==0
		|| strcmp(topic, "dagger")==0 || strcmp(topic, "daggers")==0)
	{
		do_char_log(cn, 1, "Now listing DAGGER weapons:\n");
		do_char_log(cn, 1, " \n");
		//                 "!        .         .         .         .        !"
		do_char_log(cn, 1, "   Weapon   |   Requirements:   |   Gives:  \n");
		do_char_log(cn, 1, "    Tier    | WIL | AGL | Skill |  WV | WIL \n");
		do_char_log(cn, 1, "------------+-----+-----+-------+-----+-----\n");
		do_char_log(cn, 1, "Bronze      |   1 |   1 |     1 |   6 |     \n");
		do_char_log(cn, 1, "Steel       |  12 |  12 |    10 |  12 |   1 \n");
		do_char_log(cn, 1, "Gold        |  18 |  14 |    20 |  18 |   2 \n");
		do_char_log(cn, 1, "Emerald     |  30 |  16 |    30 |  24 |   3 \n");
		do_char_log(cn, 1, "Crystal     |  48 |  20 |    40 |  30 |   4 \n");
		do_char_log(cn, 1, "Titanium    |  72 |  24 |    50 |  36 |   5 \n");
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
		do_char_log(cn, 1, "   Weapon   |  Requirements:  |     Gives:     \n");
		do_char_log(cn, 1, "    Tier    | WIL | STR | Skl | WV | AV/BR/H/P \n");
		do_char_log(cn, 1, "------------+-----+-----+-----+----+-----------\n");
		do_char_log(cn, 1, "Steel       |  16 |  12 |   8 | 20 |         4 \n");
		do_char_log(cn, 1, "Gold        |  22 |  14 |  16 | 30 |         6 \n");
		do_char_log(cn, 1, "Emerald     |  34 |  16 |  24 | 40 |         8 \n");
		do_char_log(cn, 1, "Crystal     |  52 |  20 |  32 | 50 |        10 \n");
		do_char_log(cn, 1, "Titanium    |  76 |  24 |  40 | 60 |        12 \n");
		do_char_log(cn, 1, " \n");
		do_char_log(cn, 2, "* Spears will use both hand slots.\n");
		do_char_log(cn, 0, "* Requires both Dagger and Staff skills.\n");
		do_char_log(cn, 1, "* Fighting uses the higher of either skill.\n");
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
		do_char_log(cn, 1, "Bronze      |   1 |     1 |    3   \n");
		do_char_log(cn, 1, "Steel       |  12 |    12 |    6   \n");
		do_char_log(cn, 1, "Gold        |  18 |    24 |    9   \n");
		do_char_log(cn, 1, "Emerald     |  28 |    36 |   12   \n");
		do_char_log(cn, 1, "Crystal     |  42 |    48 |   15   \n");
		do_char_log(cn, 1, "Titanium    |  60 |    60 |   18   \n");
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
		do_char_log(cn, 1, "   Weapon   |   Requirements:   |   Gives:  \n");
		do_char_log(cn, 1, "    Tier    | AGL | STR | Skill |  WV | STR \n");
		do_char_log(cn, 1, "------------+-----+-----+-------+-----+-----\n");
		do_char_log(cn, 1, "Bronze      |   1 |   1 |     1 |  10 |     \n");
		do_char_log(cn, 1, "Steel       |  12 |  14 |    16 |  20 |   1 \n");
		do_char_log(cn, 1, "Gold        |  18 |  22 |    32 |  30 |   2 \n");
		do_char_log(cn, 1, "Emerald     |  28 |  34 |    48 |  40 |   3 \n");
		do_char_log(cn, 1, "Crystal     |  42 |  50 |    64 |  50 |   4 \n");
		do_char_log(cn, 1, "Titanium    |  60 |  74 |    80 |  60 |   5 \n");
		do_char_log(cn, 1, " \n");
	}
	else if (strcmp(topic, "8")==0 
		|| strcmp(topic, "TWOHANDER")==0 || strcmp(topic, "TWOHANDERS")==0
		|| strcmp(topic, "twohander")==0 || strcmp(topic, "twohanders")==0)
	{
		do_char_log(cn, 1, "Now listing TWOHANDER weapons:\n");
		do_char_log(cn, 1, " \n");
		//                 "!        .         .         .         .        !"
		do_char_log(cn, 1, "   Weapon   |   Requirements:   |   Gives:  \n");
		do_char_log(cn, 1, "    Tier    | AGL | STR | Skill |  WV | AGL \n");
		do_char_log(cn, 1, "------------+-----+-----+-------+-----+-----\n");
		do_char_log(cn, 1, "Bronze      |   1 |   1 |     1 |  12 |     \n");
		do_char_log(cn, 1, "Steel       |  16 |  12 |    18 |  24 |   1 \n");
		do_char_log(cn, 1, "Gold        |  26 |  20 |    36 |  36 |   2 \n");
		do_char_log(cn, 1, "Emerald     |  40 |  32 |    54 |  48 |   3 \n");
		do_char_log(cn, 1, "Crystal     |  58 |  48 |    72 |  60 |   4 \n");
		do_char_log(cn, 1, "Titanium    |  80 |  68 |    90 |  72 |   5 \n");
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
		do_char_log(cn, 1, "    Tier    | AGL | STR | Skill |  WV | Top \n");
		do_char_log(cn, 1, "------------+-----+-----+-------+-----+-----\n");
		do_char_log(cn, 1, "Steel       |  12 |  14 |    15 |  28 |   4 \n");
		do_char_log(cn, 1, "Gold        |  20 |  24 |    30 |  42 |   6 \n");
		do_char_log(cn, 1, "Emerald     |  32 |  40 |    45 |  56 |   8 \n");
		do_char_log(cn, 1, "Crystal     |  48 |  62 |    60 |  70 |  10 \n");
		do_char_log(cn, 1, "Titanium    |  68 |  90 |    75 |  84 |  12 \n");
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
	do_char_log(cn, 1, "Green   |                |  5 |  1             \n");
	do_char_log(cn, 1, "Gold    |          15    | 10 | -1        3 -1 \n");
	do_char_log(cn, 1, "Lth/Sil | 12 16 16 12 12 | 10 |  3     1 -1    \n");
	do_char_log(cn, 1, "Lth/Gld | 12 12 12 16 16 | 15 |     1 -2  1  2 \n");
	do_char_log(cn, 1, "Sil/Gld | 18 24 24 18 18 | 15 |  2  2  2 -1 -1 \n");
	do_char_log(cn, 1, "Sil/Red | 18 18 18 24 24 | 20 | -1 -1     2  3 \n");
	do_char_log(cn, 1, "Gld/Sil | 30 40 40 30 30 | 20 |  2  4  4 -3 -2 \n");
	do_char_log(cn, 1, "Gld/Red | 30 30 30 40 40 | 25 | -3 -2 -2  5  6 \n");
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
		do_char_log(cn, 1, "  Silver, Small    |  LCp  |   1 |   3 \n");
		do_char_log(cn, 1, "  Silver, Medium   |  Sgt  |   2 |   4 \n");
		do_char_log(cn, 1, "  Silver, Big      |  MSg  |   3 |   5 \n");
		do_char_log(cn, 1, "    Gold, Medium   |  MSg  |   3 |   7 \n");
		do_char_log(cn, 1, "    Gold, Big      |  SgM  |   4 |   8 \n");
		do_char_log(cn, 1, "    Gold, Huge     | 1Lieu |   5 |   9 \n");
		do_char_log(cn, 1, "Platinum, Big      | 1Lieu |   5 |  11 \n");
		do_char_log(cn, 1, "Platinum, Huge     | Major |   6 |  12 \n");
		do_char_log(cn, 1, "Platinum, Flawless | Colnl |   7 |  13 \n");
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
		do_char_log(cn, 1, "    Gold, Big      | 2Lieu |   7 |  13 \n");
		do_char_log(cn, 1, "    Gold, Huge     | Captn |   9 |  15 \n");
		do_char_log(cn, 1, "Platinum, Big      | Captn |   9 |  18 \n");
		do_char_log(cn, 1, "Platinum, Huge     | LtCol |  12 |  21 \n");
		do_char_log(cn, 1, "Platinum, Flawless | BrGen |  15 |  24 \n");
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
		do_char_log(cn, 1, "    Gold, Big      | 2Lieu |  10 |  16 \n");
		do_char_log(cn, 1, "    Gold, Huge     | Captn |  15 |  21 \n");
		do_char_log(cn, 1, "Platinum, Big      | Captn |  15 |  24 \n");
		do_char_log(cn, 1, "Platinum, Huge     | LtCol |  20 |  29 \n");
		do_char_log(cn, 1, "Platinum, Flawless | BrGen |  25 |  34 \n");
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
		do_char_log(cn, 1, "    Gold, Big      | 2Lieu |   7 |  13 \n");
		do_char_log(cn, 1, "    Gold, Huge     | Captn |   9 |  15 \n");
		do_char_log(cn, 1, "Platinum, Big      | Captn |   9 |  18 \n");
		do_char_log(cn, 1, "Platinum, Huge     | LtCol |  12 |  21 \n");
		do_char_log(cn, 1, "Platinum, Flawless | BrGen |  15 |  24 \n");
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
		do_char_log(cn, 1, "    Gold, Big      | 2Lieu |   7 |  13 \n");
		do_char_log(cn, 1, "    Gold, Huge     | Captn |   9 |  15 \n");
		do_char_log(cn, 1, "Platinum, Big      | Captn |   9 |  18 \n");
		do_char_log(cn, 1, "Platinum, Huge     | LtCol |  12 |  21 \n");
		do_char_log(cn, 1, "Platinum, Flawless | BrGen |  15 |  24 \n");
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
		do_char_log(cn, 1, "    Gold, Huge     | 1Lieu |  24 |  30 \n");
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
		do_char_log(cn, 1, "    Gold, Huge     | 1Lieu |   4 |   6 \n");
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
		do_char_log(cn, 1, "    Gold, Huge     | 1Lieu |   4 |   6 \n");
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
	if (!kwai[ 6]) { m++; if (m<=t && m>n) do_char_log(cn, 1, "Mausoleum XI\n"); }
	if (!kwai[31]) { m++; if (m<=t && m>n) do_char_log(cn, 1, "Onyx Gargoyle Nest\n"); }
	//
	do_char_log(cn, 1, " \n");
	do_char_log(cn, 2, "Showing page %d of %d. #shrine <x> to swap.\n", page, max(1,min(2, (m-1)/16+1)));
	do_char_log(cn, 1, " \n");
}

int display_pole(int cn, int flag, int page, int m, int pole, char *qtxt, int exp)
{
	if (ch[cn].rebirth & 1) exp = exp/2;
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
	if ((ch[cn].rebirth & 1) && (flag || !pole))
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
	m = display_pole(cn, flag, page, m, pole3[ 0], "Lab I, Grolm Gorge",        5000);
	m = display_pole(cn, flag, page, m, pole3[ 1], "Lab I, Grolm Gorge",        5000);
	m = display_pole(cn, flag, page, m, pole3[ 2], "Lab I, Grolm Gorge",        5000);
	m = disp_rb_pole(cn, flag, page, m, poleR[ 1], "Lab I, Grolm Gorge");
	m = display_pole(cn, flag, page, m, pole4[ 8], "Earth Pentagram Quest",     6000);
	m = display_pole(cn, flag, page, m, pole1[ 6], "Arachnid Den",              6000);
	m = display_pole(cn, flag, page, m, pole2[14], "Underground I",             6000);
	m = display_pole(cn, flag, page, m, pole3[ 3], "Lab II, Lizard Gorge",      6500);
	m = display_pole(cn, flag, page, m, pole3[ 4], "Lab II, Lizard Gorge",      6500);
	m = display_pole(cn, flag, page, m, pole3[ 5], "Lab II, Lizard Gorge",      6500);
	m = disp_rb_pole(cn, flag, page, m, poleR[ 2], "Lab II, Lizard Gorge");
	m = display_pole(cn, flag, page, m, pole2[15], "Underground I",             6500);
	m = display_pole(cn, flag, page, m, pole2[16], "Underground I",             7000);
	m = display_pole(cn, flag, page, m, pole1[ 7], "Butler's Mansion",          7500);
	m = display_pole(cn, flag, page, m, pole2[17], "Underground I",             7500);
	m = display_pole(cn, flag, page, m, pole1[ 1], "Azrael's Throne Room",      8000);
	m = display_pole(cn, flag, page, m, pole5[ 2], "Strange Forest",            8000);
	m = display_pole(cn, flag, page, m, pole5[ 3], "Strange Forest",            8000);
	m = display_pole(cn, flag, page, m, pole2[18], "Underground I",             8000);
	m = display_pole(cn, flag, page, m, pole4[ 9], "Earth Pentagram Quest",     8000);
	m = display_pole(cn, flag, page, m, pole1[ 8], "Bell House Basement",       9000);
	m = display_pole(cn, flag, page, m, pole3[ 6], "Lab III, Undead Gorge",     9450);
	m = display_pole(cn, flag, page, m, pole3[ 7], "Lab III, Undead Gorge",     9450);
	m = display_pole(cn, flag, page, m, pole3[ 8], "Lab III, Undead Gorge",     9450);
	m = disp_rb_pole(cn, flag, page, m, poleR[ 3], "Lab III, Undead Gorge");
	m = display_pole(cn, flag, page, m, pole2[19], "Underground I",            10000);
	m = display_pole(cn, flag, page, m, pole2[ 7], "Stronghold, North",        10000);
	m = display_pole(cn, flag, page, m, pole4[10], "Earth Pentagram Quest",    10000);
	m = display_pole(cn, flag, page, m, pole1[ 9], "Webbed Bush",              12000);
	m = display_pole(cn, flag, page, m, pole4[11], "Earth Pentagram Quest",    12000);
	m = display_pole(cn, flag, page, m, pole1[10], "Aston Mines, Level II",    12500);
	m = display_pole(cn, flag, page, m, pole3[ 9], "Lab IV, Caster Gorge",     13800);
	m = display_pole(cn, flag, page, m, pole3[10], "Lab IV, Caster Gorge",     13800);
	m = display_pole(cn, flag, page, m, pole3[11], "Lab IV, Caster Gorge",     13800);
	m = disp_rb_pole(cn, flag, page, m, poleR[ 4], "Lab IV, Caster Gorge");
	m = display_pole(cn, flag, page, m, pole1[11], "Astonia Penitentiary",     15000);
	m = display_pole(cn, flag, page, m, pole4[12], "Earth Pentagram Quest",    15000);
	m = display_pole(cn, flag, page, m, pole1[16], "Forgotten Canyon",         16000);
	m = display_pole(cn, flag, page, m, pole1[13], "The Mad Hermit's House",   17500);
	m = display_pole(cn, flag, page, m, pole1[19], "Abandoned Archives",       18000);
	m = display_pole(cn, flag, page, m, pole4[13], "Fire Pentagram Quest",     18000);
	m = display_pole(cn, flag, page, m, pole3[12], "Lab V, Knight Gorge",      19500);
	m = display_pole(cn, flag, page, m, pole3[13], "Lab V, Knight Gorge",      19500);
	m = display_pole(cn, flag, page, m, pole3[14], "Lab V, Knight Gorge",      19500);
	m = disp_rb_pole(cn, flag, page, m, poleR[ 5], "Lab V, Knight Gorge");
	m = display_pole(cn, flag, page, m, pole4[14], "Fire Pentagram Quest",     21000);
	m = display_pole(cn, flag, page, m, pole1[17], "Jagged Pass Cellar",       22500);
	m = display_pole(cn, flag, page, m, pole5[ 4], "Autumn Meadow",            24000);
	m = display_pole(cn, flag, page, m, pole5[ 5], "Autumn Meadow",            24000);
	m = display_pole(cn, flag, page, m, pole2[20], "Underground II",           24000);
	m = display_pole(cn, flag, page, m, pole4[15], "Fire Pentagram Quest",     24000);
	m = display_pole(cn, flag, page, m, pole1[12], "Grolm Laboratory",         25000);
	m = display_pole(cn, flag, page, m, pole2[21], "Underground II",           26000);
	m = display_pole(cn, flag, page, m, pole3[15], "Lab VI, Desert Gorge",     26500);
	m = display_pole(cn, flag, page, m, pole3[16], "Lab VI, Desert Gorge",     26500);
	m = display_pole(cn, flag, page, m, pole3[17], "Lab VI, Desert Gorge",     26500);
	m = disp_rb_pole(cn, flag, page, m, poleR[ 6], "Lab VI, Desert Gorge");
	m = display_pole(cn, flag, page, m, pole2[22], "Underground II",           28000);
	m = display_pole(cn, flag, page, m, pole4[16], "Fire Pentagram Quest",     28000);
	m = display_pole(cn, flag, page, m, pole1[14], "Lavender Lakebed",         30000);
	m = display_pole(cn, flag, page, m, pole2[23], "Underground II",           30000);
	m = display_pole(cn, flag, page, m, pole2[24], "Underground II",           32000);
	m = display_pole(cn, flag, page, m, pole4[17], "Fire Pentagram Quest",     32000);
	m = display_pole(cn, flag, page, m, pole1[18], "Winding Valley",           32500);
	m = display_pole(cn, flag, page, m, pole5[ 6], "Scorpion Burrow",          34000);
	m = display_pole(cn, flag, page, m, pole3[18], "Lab VII, Light/Dark Gor",  34750);
	m = display_pole(cn, flag, page, m, pole3[19], "Lab VII, Light/Dark Gor",  34750);
	m = display_pole(cn, flag, page, m, pole3[20], "Lab VII, Light/Dark Gor",  34750);
	m = disp_rb_pole(cn, flag, page, m, poleR[ 7], "Lab VII, Light/Dark Gor");
	m = display_pole(cn, flag, page, m, pole5[ 9], "Old Well",                 36000);
	m = display_pole(cn, flag, page, m, pole4[18], "Fire Pentagram Quest",     36000);
	m = display_pole(cn, flag, page, m, pole2[25], "Underground II",           40000);
	m = display_pole(cn, flag, page, m, pole2[ 0], "Gargoyle Nest",            40000);
	m = display_pole(cn, flag, page, m, pole4[19], "Fire Pentagram Quest",     40000);
	m = display_pole(cn, flag, page, m, pole3[21], "Lab VIII, Underwater Go",  44200);
	m = display_pole(cn, flag, page, m, pole3[22], "Lab VIII, Underwater Go",  44200);
	m = display_pole(cn, flag, page, m, pole3[23], "Lab VIII, Underwater Go",  44200);
	m = disp_rb_pole(cn, flag, page, m, poleR[ 8], "Lab VIII, Underwater Go");
	m = display_pole(cn, flag, page, m, pole1[21], "Southern Swamp",           45000);
	m = display_pole(cn, flag, page, m, pole4[20], "Jungle Pentagram Quest",   45000);
	m = display_pole(cn, flag, page, m, pole2[ 1], "Gargoyle Nest",            50000);
	m = display_pole(cn, flag, page, m, pole1[23], "Crumbling Cathedral",      50000);
	m = display_pole(cn, flag, page, m, pole4[21], "Jungle Pentagram Quest",   50000);
	m = display_pole(cn, flag, page, m, pole3[24], "Lab IX, Riddle Gorge",     54800);
	m = display_pole(cn, flag, page, m, pole3[25], "Lab IX, Riddle Gorge",     54800);
	m = display_pole(cn, flag, page, m, pole3[26], "Lab IX, Riddle Gorge",     54800);
	m = disp_rb_pole(cn, flag, page, m, poleR[ 9], "Lab IX, Riddle Gorge");
	m = display_pole(cn, flag, page, m, pole2[ 8], "Black Stronghold",         50000);
	m = display_pole(cn, flag, page, m, pole4[22], "Jungle Pentagram Quest",   55000);
	m = display_pole(cn, flag, page, m, pole1[24], "Skitter Hatchery",         60000);
	m = display_pole(cn, flag, page, m, pole2[ 2], "Gargoyle Nest",            60000);
	m = display_pole(cn, flag, page, m, pole4[23], "Jungle Pentagram Quest",   60000);
	m = display_pole(cn, flag, page, m, pole1[25], "Thug's Camp",              64000);
	m = display_pole(cn, flag, page, m, pole4[24], "Jungle Pentagram Quest",   66000);
	m = display_pole(cn, flag, page, m, pole3[27], "Lab X, Forest Gorge",      66500);
	m = display_pole(cn, flag, page, m, pole3[28], "Lab X, Forest Gorge",      66500);
	m = display_pole(cn, flag, page, m, pole3[29], "Lab X, Forest Gorge",      66500);
	m = disp_rb_pole(cn, flag, page, m, poleR[10], "Lab X, Forest Gorge");
	m = display_pole(cn, flag, page, m, pole5[10], "Buried Brush",             68000);
	m = display_pole(cn, flag, page, m, pole1[26], "Lizard Temple",            70000);
	m = display_pole(cn, flag, page, m, pole5[ 7], "Scorpion Hive",            72000);
	m = display_pole(cn, flag, page, m, pole2[26], "Underground III",          72000);
	m = display_pole(cn, flag, page, m, pole4[25], "Ice Pentagram Quest",      72000);
	m = display_pole(cn, flag, page, m, pole1[15], "Aston Mines, Level III",   75000);
	m = display_pole(cn, flag, page, m, pole5[ 8], "Outset Den",               76000);
	m = display_pole(cn, flag, page, m, pole2[27], "Underground III",          78000);
	m = display_pole(cn, flag, page, m, pole4[26], "Ice Pentagram Quest",      78000);
	m = display_pole(cn, flag, page, m, pole3[30], "Lab XI, Seasons Gorge",    79250);
	m = display_pole(cn, flag, page, m, pole3[31], "Lab XI, Seasons Gorge",    79250);
	m = display_pole(cn, flag, page, m, pole4[ 0], "Lab XI, Seasons Gorge",    79250);
	m = disp_rb_pole(cn, flag, page, m, poleR[11], "Lab XI, Seasons Gorge");
	m = display_pole(cn, flag, page, m, pole2[ 3], "Gargoyle Nest",            80000);
	m = display_pole(cn, flag, page, m, pole1[22], "Northern Mountains",       80000);
	m = display_pole(cn, flag, page, m, pole2[ 4], "Ice Gargoyle Nest",        80000);
	m = display_pole(cn, flag, page, m, pole2[28], "Underground III",          84000);
	m = display_pole(cn, flag, page, m, pole4[27], "Ice Pentagram Quest",      84000);
	m = display_pole(cn, flag, page, m, pole1[27], "The Emerald Cavern",       90000);
	m = display_pole(cn, flag, page, m, pole1[28], "The Emerald Cavern",       90000);
	m = display_pole(cn, flag, page, m, pole2[29], "Underground III",          90000);
	m = display_pole(cn, flag, page, m, pole2[ 9], "Stronghold, South",        90000);
	m = display_pole(cn, flag, page, m, pole4[28], "Sea Pentagram Quest",      92000);
	m = display_pole(cn, flag, page, m, pole4[ 1], "Lab XII, Ascent Gorge",    93000);
	m = display_pole(cn, flag, page, m, pole4[ 2], "Lab XII, Ascent Gorge",    93000);
	m = display_pole(cn, flag, page, m, pole4[ 3], "Lab XII, Ascent Gorge",    93000);
	m = disp_rb_pole(cn, flag, page, m, poleR[12], "Lab XII, Ascent Gorge");
	m = display_pole(cn, flag, page, m, pole2[30], "Underground III",          96000);
	m = display_pole(cn, flag, page, m, pole5[14], "Hollow Trench",            96000);
	m = display_pole(cn, flag, page, m, pole1[20], "Forbidden Tomes",         100000);
	m = display_pole(cn, flag, page, m, pole2[ 5], "Ice Gargoyle Nest",       100000);
	m = display_pole(cn, flag, page, m, pole4[29], "Sea Pentagram Quest",     100000);
	m = display_pole(cn, flag, page, m, pole5[11], "Platinum Mines II",       105000);
	m = display_pole(cn, flag, page, m, pole6[ 0], "Lab XIV, Miner's Gorge",  107700);
	m = display_pole(cn, flag, page, m, pole6[ 1], "Lab XIV, Miner's Gorge",  107700);
	m = display_pole(cn, flag, page, m, pole6[ 2], "Lab XIV, Miner's Gorge",  107700);
	m = disp_rb_pole(cn, flag, page, m, poleR[13], "Lab XIV, Miner's Gorge");
	m = display_pole(cn, flag, page, m, pole4[30], "Onyx Pentagram Quest",    110000);
	m = display_pole(cn, flag, page, m, pole5[13], "Smuggler's Hovel",        112000);
	m = display_pole(cn, flag, page, m, pole5[18], "The Basalt Ziggurat",     115000);
	m = display_pole(cn, flag, page, m, pole2[31], "Underground III",         120000);
	m = display_pole(cn, flag, page, m, pole2[ 6], "Ice Gargoyle Nest",       120000);
	m = display_pole(cn, flag, page, m, pole5[ 0], "Seagrel King's Quarters", 120000);
	m = display_pole(cn, flag, page, m, pole1[30], "Onyx Gargoyle Nest",      120000);
	m = display_pole(cn, flag, page, m, pole4[31], "Onyx Pentagram Quest",    120000);
	m = disp_rb_pole(cn, flag, page, m, poleR[24], "Onyx Pentagram Quest");
	m = display_pole(cn, flag, page, m, pole6[ 3], "Lab XV, Vantablack Gorg", 123300);
	m = display_pole(cn, flag, page, m, pole6[ 4], "Lab XV, Vantablack Gorg", 123300);
	m = display_pole(cn, flag, page, m, pole6[ 5], "Lab XV, Vantablack Gorg", 123300);
	m = disp_rb_pole(cn, flag, page, m, poleR[14], "Lab XV, Vantablack Gorg");
	m = display_pole(cn, flag, page, m, pole5[ 1], "The Volcano",             125000);
	m = display_pole(cn, flag, page, m, pole2[11], "Tower X",                 125000);
	m = display_pole(cn, flag, page, m, pole5[12], "Merlin's Laboratory",     128000);
	m = display_pole(cn, flag, page, m, pole2[10], "Black Stronghold Baseme", 130000);
	m = disp_rb_pole(cn, flag, page, m, poleR[21], "Black Stronghold Baseme");
	m = display_pole(cn, flag, page, m, pole5[19], "The Widow's Nest",        130000);
	m = display_pole(cn, flag, page, m, pole5[20], "Wellspring Chasm",        135000);
	m = display_pole(cn, flag, page, m, pole5[15], "Cold Cavern",             136000);
	m = display_pole(cn, flag, page, m, pole6[ 6], "Lab XVI, Pirate's Gorge", 139750);
	m = display_pole(cn, flag, page, m, pole6[ 7], "Lab XVI, Pirate's Gorge", 139750);
	m = display_pole(cn, flag, page, m, pole6[ 8], "Lab XVI, Pirate's Gorge", 139750);
	m = disp_rb_pole(cn, flag, page, m, poleR[15], "Lab XVI, Pirate's Gorge");
	m = display_pole(cn, flag, page, m, pole5[16], "Seppuku House",           148000);
	m = display_pole(cn, flag, page, m, pole1[29], "The Emerald Palace",      150000);
	m = disp_rb_pole(cn, flag, page, m, poleR[22], "The Emerald Palace");
	m = display_pole(cn, flag, page, m, pole6[ 9], "Lab XVII, Gargoyle's Go", 157000);
	m = display_pole(cn, flag, page, m, pole6[10], "Lab XVII, Gargoyle's Go", 157000);
	m = display_pole(cn, flag, page, m, pole6[11], "Lab XVII, Gargoyle's Go", 157000);
	m = disp_rb_pole(cn, flag, page, m, poleR[16], "Lab XVII, Gargoyle's Go");
	m = display_pole(cn, flag, page, m, pole1[31], "Onyx Gargoyle Nest",      160000);
	m = display_pole(cn, flag, page, m, pole6[12], "Lab XVIII, Commandment ", 175000);
	m = display_pole(cn, flag, page, m, pole6[13], "Lab XVIII, Commandment ", 175000);
	m = display_pole(cn, flag, page, m, pole6[14], "Lab XVIII, Commandment ", 175000);
	m = disp_rb_pole(cn, flag, page, m, poleR[17], "Lab XVIII, Commandment ");
	m = display_pole(cn, flag, page, m, pole6[15], "Lab XIX, Divinity Gorge", 193700);
	m = display_pole(cn, flag, page, m, pole6[16], "Lab XIX, Divinity Gorge", 193700);
	m = display_pole(cn, flag, page, m, pole6[17], "Lab XIX, Divinity Gorge", 193700);
	m = disp_rb_pole(cn, flag, page, m, poleR[18], "Lab XIX, Divinity Gorge");
	m = display_pole(cn, flag, page, m, pole5[17], "The Obsidian Fortress",   200000);
	m = disp_rb_pole(cn, flag, page, m, poleR[23], "The Obsidian Fortress");
	m = display_pole(cn, flag, page, m, pole2[12], "Tower XX",                275000);
	m = disp_rb_pole(cn, flag, page, m, poleR[19], "Tower XX");
	m = display_pole(cn, flag, page, m, pole2[13], "Abyss X",                 600000);
	m = disp_rb_pole(cn, flag, page, m, poleR[20], "Abyss X");
	//
//	m = display_pole(cn, flag, page, m, pole5[21], "", 0);
//	m = display_pole(cn, flag, page, m, pole5[22], "", 0);
//	m = display_pole(cn, flag, page, m, pole5[23], "", 0);
//	m = display_pole(cn, flag, page, m, pole5[24], "", 0);
//	m = display_pole(cn, flag, page, m, pole5[25], "", 0);
//	m = display_pole(cn, flag, page, m, pole5[26], "", 0);
//	m = display_pole(cn, flag, page, m, pole5[27], "", 0);
//	m = display_pole(cn, flag, page, m, pole5[28], "", 0);
//	m = display_pole(cn, flag, page, m, pole5[29], "", 0);
//	m = display_pole(cn, flag, page, m, pole5[30], "", 0);
//	m = display_pole(cn, flag, page, m, pole5[31], "", 0);
	//
//	m = display_pole(cn, flag, page, m, pole6[18], "", 0);
//	m = display_pole(cn, flag, page, m, pole6[19], "", 0);
//	m = display_pole(cn, flag, page, m, pole6[20], "", 0);
//	m = display_pole(cn, flag, page, m, pole6[21], "", 0);
//	m = display_pole(cn, flag, page, m, pole6[22], "", 0);
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
//	m = disp_rb_pole(cn, flag, page, m, poleR[25], "");
//	m = disp_rb_pole(cn, flag, page, m, poleR[26], "");
//	m = disp_rb_pole(cn, flag, page, m, poleR[27], "");
//	m = disp_rb_pole(cn, flag, page, m, poleR[28], "");
//	m = disp_rb_pole(cn, flag, page, m, poleR[29], "");
//	m = disp_rb_pole(cn, flag, page, m, poleR[30], "");
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
	int spi = 0, sp2 = 0, sco = 0, sc2 = 0, sc3 = 0, gre = 0, bla = 0, hou = 0;
	int quest1[26]={0};
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
	
	if (B_SK(cn, SK_BARTER))															quest1[ 0] = 1;	// ( Jamil )
	if (IS_ANY_MERC(cn) || IS_ANY_HARA(cn) || B_SK(cn, SK_TAUNT))						quest1[ 1] = 1;	// ( Inga )
	if (IS_BRAVER(cn)   || IS_ANY_HARA(cn) || IS_ANY_TEMP(cn) || B_SK(cn, SK_POISON))	quest1[24] = 1;	// ( Inga )
	if (IS_BRAVER(cn)   || IS_ANY_TEMP(cn) || IS_ANY_MERC(cn) || B_SK(cn, SK_CONCEN))	quest1[25] = 1;	// ( Inga )
	if (IS_BRAVER(cn)   || IS_ANY_TEMP(cn) || B_SK(cn, SK_ENHANCE))						quest1[ 2] = 1;	// ( Sirjan )
	if (IS_ANY_MERC(cn) || IS_ANY_HARA(cn) || B_SK(cn, SK_MSHIELD))						quest1[ 3] = 1;	// ( Sirjan )
	if (IS_ANY_MERC(cn) || IS_ANY_HARA(cn) || B_SK(cn, SK_WEAKEN))						quest1[ 4] = 1;	// ( Amity )
	if (IS_BRAVER(cn)   || IS_ANY_TEMP(cn) || B_SK(cn, SK_SLOW))						quest1[ 5] = 1;	// ( Amity )
	if (B_SK(cn, SK_REPAIR))															quest1[ 6] = 1;	// ( Jefferson )
	if (ch[cn].flags & CF_LOCKPICK)														quest1[ 7] = 1;	// ( Steven )
	if (IS_BRAVER(cn)   || IS_ANY_HARA(cn)	|| B_SK(cn, SK_IMMUN))						quest1[ 8] = 1;	// ( Ingrid )
	if (IS_ANY_TEMP(cn) || IS_ANY_MERC(cn) || B_SK(cn, SK_DISPEL))						quest1[ 9] = 1;	// ( Ingrid )
	if (IS_ANY_MERC(cn) || IS_ANY_HARA(cn) || B_SK(cn, SK_SURROUND))					quest1[10] = 1;	// ( Leopold )
	if (IS_BRAVER(cn)   || IS_ANY_TEMP(cn) || B_SK(cn, SK_CURSE))						quest1[11] = 1;	// ( Leopold )
	if (B_SK(cn, SK_HEAL))																quest1[12] = 1;	// ( Gunther )
	if (ch[cn].flags & CF_SENSE)														quest1[13] = 1;	// ( Manfred )
	if (B_SK(cn, SK_RESIST))															quest1[14] = 1;	// ( Serena )
	if (B_SK(cn, SK_BLESS))																quest1[15] = 1;	// ( Cirrus )
	if (B_SK(cn, SK_REST))																quest1[17] = 1;	// ( Gordon )
	if (IS_ANY_HARA(cn) || B_SK(cn, SK_SHIELD))											quest1[18] = 1;	// ( Edna )
	if (IS_BRAVER(cn)   || IS_ANY_TEMP(cn) || IS_ANY_MERC(cn) || B_SK(cn, SK_STAFF))	quest1[19] = 1;	// ( Edna )
	if (B_SK(cn, SK_IDENT))																quest1[20] = 1;	// ( Nasir )
	if (ch[cn].temple_x!=HOME_START_X)													quest1[21] = 1;	// Get to Aston
	if (ch[cn].flags & CF_APPRAISE)														quest1[22] = 1; // ( Richie )
	if (B_SK(cn, SK_SWIM))																quest1[23] = 1;	// ( Lucci )
	if ((IS_ANY_MERC(cn) || IS_SEYAN_DU(cn)) && B_SK(cn, SK_HASTE))						quest2[16] = 1; // ( Regal )
	
	//
	if ((B_SK(cn, SK_WARCRY) 	&& IS_ARCHTEMPLAR(cn)))		quest1[16] = 1;
	if ((B_SK(cn, SK_LEAP) 		&& IS_SKALD(cn)))			quest1[16] = 1;
	if ((B_SK(cn, SK_ZEPHYR) 	&& IS_WARRIOR(cn)))			quest1[16] = 1;
	if ((B_SK(cn, SK_LETHARGY) 	&& IS_SORCERER(cn)))		quest1[16] = 1;
	if ((B_SK(cn, SK_SHADOW) 	&& IS_SUMMONER(cn)))		quest1[16] = 1;
	if ((B_SK(cn, SK_PULSE) 	&& IS_ARCHHARAKIM(cn)))		quest1[16] = 1;
	if ((B_SK(cn, SK_RAGE) 		&& IS_BRAVER(cn)))			quest1[16] = 1;
	//
	if (IS_SEYAN_DU(cn))
	{
		n = 0;
		if (B_SK(cn, SK_WARCRY))   n++;		if (B_SK(cn, SK_LEAP))     n++;
		if (B_SK(cn, SK_SHADOW))   n++;		if (B_SK(cn, SK_LETHARGY)) n++;
		if (B_SK(cn, SK_PULSE))    n++;		if (B_SK(cn, SK_ZEPHYR))   n++;
		if (B_SK(cn, SK_RAGE))     n++;		if (n>=2)  		quest1[16] = 1;
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
	if (nei && (ch[cn].rebirth & 1)) 				  nrb = 1;
	else if (nei)									  nlb = 1;
	
	if (getrank(cn)< 7 && !ast) spi = 2;
	if (getrank(cn)< 7 &&  ast) sp2 = 2;
	if (getrank(cn)> 9 && getrank(cn)<14) sco = 2;
	if (getrank(cn)>11 && getrank(cn)<17) gre = 2;
	if (getrank(cn)>13 && getrank(cn)<16) sc2 = 2;
	if (getrank(cn)>15 && getrank(cn)<18) sc3 = 2;
	if (getrank(cn)>16) hou = 2;
	if (getrank(cn)>16) bla = 2;
	
	do_char_log(cn, 1, "Now listing available quests (PAGE %d):\n", page);
	do_char_log(cn, 1, " \n");
	
	n = (page-1)*16;
	t = page*16;
	m = 0;
	//
	m = display_quest(cn, flag, page, m,   1, quest1[ 0],  0, "Jamil",    "Bluebird Tav", "*Barteri",    100);
	m = display_quest(cn, flag, page, m,   1, quest1[ 1],  1, "Inga",     "First Street", "*Taunt",      200);
	m = display_quest(cn, flag, page, m,   1, quest1[24],  1, "Inga",     "First Street", "*Poison",     200);
	m = display_quest(cn, flag, page, m,   1, quest1[25],  1, "Inga",     "First Street", "*Concent",    200);
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
	m = display_quest(cn, flag, page, m,   1, quest1[19],  4, "Edna",     "Shore Cresce", "*Staff",     3300);
	m = display_quest(cn, flag, page, m,   1, quest1[20],  4, "Nasir",    "Shore Cresce", "*Identif",   3800);
	m = display_quest(cn, flag, page, m, spi,          0,  4, "Garna",    "First Street", "@Gold",       125);
	m = display_quest(cn, flag, page, m,   1, quest1[21],  5, "Guard Ca", "South Aston",  "Gold",       8000);
	//
	m = display_quest(cn, flag, page, m,!ast, quest2[ 0],  5, "???",      "Aston",        "???",        4000);
	//
	m = display_quest(cn, flag, page, m, sp2,          0,  4, "Gene",     "South End",    "@Gold",       125);
	m = display_quest(cn, flag, page, m, ast, quest2[ 0],  5, "April",    "Marble Lane",  "Potion",     4000);
	m = display_quest(cn, flag, page, m, ast, quest1[22],  5, "Richie",   "Marble Lane",  "*Apprais",   5000);
	m = display_quest(cn, flag, page, m, ast, quest2[ 1],  5, "Alphonse", "Rose Street",  "Ring",       7000);
	m = display_quest(cn, flag, page, m, ast, questP[ 1],  6, "Lab I",    "Grolm Gorge",  "-",         20000);
	m = display_quest(cn, flag, page, m, ast, questP[ 2],  6, "Lab II",   "Lizard Gorge", "-",         25000);
	m = display_quest(cn, flag, page, m, ast, quest2[11],  7, "Robin",    "South End",    "Potion",     8000);
	m = display_quest(cn, flag, page, m, ast, quest2[ 2],  7, "Cherri",   "Merchant's W", "Amulet",    10000);
	m = display_quest(cn, flag, page, m, ast, questP[ 3],  7, "Lab III",  "Undead Gorge", "-",         35000);
	m = display_quest(cn, flag, page, m, ast, questP[ 4],  7, "Lab IV",   "Caster Gorge", "-",         50000);
	m = display_quest(cn, flag, page, m, ast, quest2[ 3],  8, "Rocky",    "Merchant's W", "Ring",      12500);
	m = display_quest(cn, flag, page, m, ast, quest2[ 4],  8, "Bradley",  "Astonia Peni", "Belt",      15000);
	m = display_quest(cn, flag, page, m, ast, quest2[13],  8, "Oscar",    "Temple Stree", "Amulet",    16000);
	m = display_quest(cn, flag, page, m, ast, questP[ 5],  8, "Lab V",    "Knight Gorge", "-",         70000);
	m = display_quest(cn, flag, page, m, ast, quest2[ 6],  9, "Elric",    "Merchant's W", "Potion",    17500);
	m = display_quest(cn, flag, page, m, ast, quest2[14],  9, "Castor",   "Marble Lane",  "Book",      18000);
	m = display_quest(cn, flag, page, m, ast, quest2[15],  9, "Grover",   "Marble Lane",  "Amulet",    20000);
	m = display_quest(cn, flag, page, m, ast, questP[ 6],  9, "Lab VI",   "Desert Gorge", "-",         95000);
	m = display_quest(cn, flag, page, m, ast, quest2[ 5], 10, "Roxie",    "Temple Stree", "Amulet",    22500);
	m = display_quest(cn, flag, page, m, ast, quest1[23], 10, "Lucci",    "Temple Stree", "*Swimmin",  25000);
	if (IS_ANY_MERC(cn) || IS_SEYAN_DU(cn))
		m = display_quest(cn, flag, page, m, ast, quest2[16], 10, "Regal","Winding Vall", "*Haste",    30000);
	else
		m = display_quest(cn, flag, page, m, ast, quest2[16], 10, "Regal","Winding Vall", "Book",      30000);
	
	m = display_quest(cn, flag, page, m, ast, questP[ 7], 10, "Lab VII",  "Light/Dark G", "-",        125000);
	m = display_quest(cn, flag, page, m, sco,          0, 11, "Faiza",    "Temple Stree", "@Gold",       750);
	m = display_quest(cn, flag, page, m, ast, quest2[10], 11, "Donna",    "Merchant's W", "Tarot",     32000);
	m = display_quest(cn, flag, page, m, ast, quest2[ 9], 11, "Gomez",    "Temple Stree", "Tarot",     40000);
	m = display_quest(cn, flag, page, m, ast, quest3[ 0], 11, "Oswald",   "Aston Farms",  "Amulet",    40000);
	m = display_quest(cn, flag, page, m, ast, questP[ 8], 11, "Lab VIII", "Underwater G", "-",        160000);
	m = display_quest(cn, flag, page, m, ast, questP[ 9], 11, "Lab IX",   "Riddle Gorge", "-",        200000);
	m = display_quest(cn, flag, page, m, gre,          0, 12, "Rose",     "Blue Ogre Ta", "@Gold",     10000);
	m = display_quest(cn, flag, page, m, ast, quest2[ 7], 12, "Marline",  "Marble Lane",  "Tarot",     42500);
	m = display_quest(cn, flag, page, m, ast, quest2[22], 12, "Ludolf",   "Aston Farms",  "Tarot",     45000);
	m = display_quest(cn, flag, page, m, ast, quest2[23], 12, "Flanders", "Aston Farms",  "Tarot",     50000);
	m = display_quest(cn, flag, page, m, ast, questP[10], 12, "Lab X",    "Forest Gorge", "-",        245000);
	m = display_quest(cn, flag, page, m, ast, questP[11], 12, "Lab XI",   "Seasons Gorg", "-",        295000);
	m = display_quest(cn, flag, page, m, ast, quest2[12], 13, "Monica",   "South End",    "Potion",    20000);
	m = display_quest(cn, flag, page, m, ast, quest2[ 8], 13, "Rufus",    "Temple Stree", "Tarot",     60000);
	m = display_quest(cn, flag, page, m, ast, quest2[24], 13, "Topham",   "Temple Stree", "Gold",      64000);
	m = display_quest(cn, flag, page, m, ast, questP[12], 13, "Lab XII",  "Ascent Gorge", "-",        350000);
	m = display_quest(cn, flag, page, m, ast, questP[13], 13, "Lab XIII", "Gatekeeper's", "+Arch",         0);
	m = display_quest(cn, flag, page, m, sc2,          0, 14, "Faiza",    "Temple Stree", "@Gold",      1000);
	m = display_quest(cn, flag, page, m, ast, quest2[17], 14, "Shera",    "Bulwark Aven", "Cloak",     75000);
	m = display_quest(cn, flag, page, m, ast, quest2[18], 14, "Tacticia", "West Gate",    "Spear",     75000);
	m = display_quest(cn, flag, page, m, ast, quest2[21], 15, "Aster",    "Aston Farms",  "Tarot",     75000);
	m = display_quest(cn, flag, page, m, ast, quest3[ 1], 15, "Maude",    "Aston Farms",  "Gold",      78000);
	//
	ars = (ast && (IS_ARCHTEMPLAR(cn) || (IS_SEYAN_DU(cn) && !quest1[16] && !B_SK(cn, SK_WARCRY))));
	m = display_quest(cn, flag, page, m, ars, quest1[16], 15, "Jamie",    "Bulwark Aven", "*Warcry",   80000);
	ars = (ast && (IS_SKALD(cn) || (IS_SEYAN_DU(cn) && !quest1[16] && !B_SK(cn, SK_LEAP))));
	m = display_quest(cn, flag, page, m, ars, quest1[16], 15, "Ellis",    "Bulwark Aven", "*Leap",     80000);
	ars = (ast && (IS_SUMMONER(cn) || (IS_SEYAN_DU(cn) && !quest1[16] && !B_SK(cn, SK_SHADOW))));
	m = display_quest(cn, flag, page, m, ars, quest1[16], 15, "Roger",    "Bulwark Aven", "*Shadow",   80000);
	ars = (ast && (IS_SORCERER(cn) || (IS_SEYAN_DU(cn) && !quest1[16] && !B_SK(cn, SK_LETHARGY))));
	m = display_quest(cn, flag, page, m, ars, quest1[16], 15, "Boris",    "Bulwark Aven", "*Letharg",  80000);
	ars = (ast && (IS_ARCHHARAKIM(cn) || (IS_SEYAN_DU(cn) && !quest1[16] && !B_SK(cn, SK_PULSE))));
	m = display_quest(cn, flag, page, m, ars, quest1[16], 15, "Kaleigh",  "Bulwark Aven", "*Pulse",    80000);
	ars = (ast && (IS_WARRIOR(cn) || (IS_SEYAN_DU(cn) && !quest1[16] && !B_SK(cn, SK_ZEPHYR))));
	m = display_quest(cn, flag, page, m, ars, quest1[16], 15, "Terri",    "Bulwark Aven", "*Zephyr",   80000);
	ars = (ast && (IS_BRAVER(cn) || (IS_SEYAN_DU(cn) && !quest1[16] && !B_SK(cn, SK_RAGE))));
	m = display_quest(cn, flag, page, m, ars, quest1[16], 15, "Sierra",   "Bulwark Aven", "*Rage",     80000);
	//
	m = display_quest(cn, flag, page, m,!liz, quest2[25], 15, "???",      "Beryl Jungle", "???",       90000);
	//
	m = display_quest(cn, flag, page, m, liz, quest2[26], 15, "Tsulu",    "Settlement",   "Amulet",    90000);
	m = display_quest(cn, flag, page, m, sc3,          0, 16, "Faiza",    "Temple Stree", "@Gold",      1875);
	m = display_quest(cn, flag, page, m, ast, quest2[19], 16, "Danica",   "South End",    "Potion",    50000);
	m = display_quest(cn, flag, page, m, liz, quest2[27], 16, "Shafira",  "Settlement",   "Amulet",    90000);
	m = display_quest(cn, flag, page, m, liz, quest2[25], 16, "Navarre",  "Settlement",   "Amulet",    90000);
	//
	m = display_quest(cn, flag, page, m,!nei, quest3[ 2], 16, "???",      "Neiseer",      "???",      100000);
	//
	m = display_quest(cn, flag, page, m, nei, quest3[ 2], 16, "Brenna",   "Victory Road", "Tarot",    100000); // #135
	m = display_quest(cn, flag, page, m, nei, quest3[ 8], 16, "Rikus",    "Victory Road", "Tarot",    105000); // #141
	m = display_quest(cn, flag, page, m, nlb, questP[14], 16, "Lab XIV",  "Miner's Gorg", "+1 SP",    400000);
	m = display_quest(cn, flag, page, m, nrb, questP[14], 16, "Lab XIV",  "Miner's Gorg", "-",        400000);
	m = display_quest(cn, flag, page, m, hou,          0, 17, "Dwyn",     "Titan Street", "@Gold",      5000);
	m = display_quest(cn, flag, page, m, nei, quest3[ 4], 17, "Jasper",   "Ravaged Prai", "Tarot",    125000); // #137
	m = display_quest(cn, flag, page, m, nei, quest3[ 3], 17, "Wicker",   "Warlock Way",  "Scroll",   135000); // #136
	m = display_quest(cn, flag, page, m, nei, quest3[ 9], 17, "Charlotte","Warlock Way",  "Tarot",    135000); // #142
	m = display_quest(cn, flag, page, m, nei, quest3[10], 17, "Marco",    "Warlock Way",  "Tarot",    140000); // #143
	m = display_quest(cn, flag, page, m, liz, quest2[28], 17, "Dracus",   "Settlement",   "Amulet",   150000);
	m = display_quest(cn, flag, page, m, liz, quest2[30], 17, "Makira",   "Settlement",   "Belt",     150000);
	m = display_quest(cn, flag, page, m, nlb, questP[15], 17, "Lab XV",   "Vantablack G", "+1 SP",    445000);
	m = display_quest(cn, flag, page, m, nrb, questP[15], 17, "Lab XV",   "Vantablack G", "-",        445000);
	m = display_quest(cn, flag, page, m, bla,          0, 18, "Zorani",   "Settlement",   "@Gold",    120000);
	m = display_quest(cn, flag, page, m, ast, quest2[20], 18, "Blanche",  "Bulwark Aven", "Cloak",    150000);
	m = display_quest(cn, flag, page, m, nei, quest3[ 5], 18, "Soyala",   "Last Avenue",  "Tarot",    160000); // #138
	m = display_quest(cn, flag, page, m, liz, quest2[31], 18, "Vora",     "Settlement",   "Belt",     175000);
	m = display_quest(cn, flag, page, m, nlb, questP[16], 18, "Lab XVI",  "Pirate's Gor", "+1 SP",    485000);
	m = display_quest(cn, flag, page, m, nrb, questP[16], 18, "Lab XVI",  "Pirate's Gor", "-",        485000);
	m = display_quest(cn, flag, page, m, nei, quest3[ 6], 19, "Runa",     "Last Avenue",  "Belt",     175000); // #139
	m = display_quest(cn, flag, page, m, nei, quest3[ 7], 19, "Zephan",   "Last Avenue",  "Belt",     180000); // #140
	m = display_quest(cn, flag, page, m, liz, quest2[29], 19, "Rassa",    "Settlement",   "Cloak",    225000);
	m = display_quest(cn, flag, page, m, nlb, questP[17], 19, "Lab XVII", "Gargoyle's G", "+1 SP",    520000);
	m = display_quest(cn, flag, page, m, nrb, questP[17], 19, "Lab XVII", "Gargoyle's G", "-",        520000);
	m = display_quest(cn, flag, page, m, nlb, questP[18], 20, "Lab XVIII","Commandment ", "+1 SP",    550000);
	m = display_quest(cn, flag, page, m, nrb, questP[18], 20, "Lab XVIII","Commandment ", "-",        550000);
	m = display_quest(cn, flag, page, m, nlb, questP[19], 21, "Lab XIX",  "Divinity Gor", "+1 SP",    575000);
	m = display_quest(cn, flag, page, m, nrb, questP[19], 21, "Lab XIX",  "Divinity Gor", "-",        575000);
	m = display_quest(cn, flag, page, m, nlb, questP[20], 22, "Lab XX",   "Final Gorge",  "+1 SP",         0);
	//
	m = display_quest(cn, flag, page, m,!arc,        questZ[ 0], 24, "???",      "Burning Plai", "???",    0);
	//
	m = display_quest(cn, flag, page, m, arc,        questZ[ 0], 24, "Pandium",  "The Archon", "Depth  1", 0);
	m = display_quest(cn, flag, page, m, questZ[ 0], questZ[ 1], 24, "Pandium",  "The Archon", "Depth  5", 0);
	m = display_quest(cn, flag, page, m, questZ[ 1], questZ[ 2], 24, "Pandium",  "The Archon", "Depth 10", 0);
	m = display_quest(cn, flag, page, m, questZ[ 2], questZ[ 3], 24, "Pandium",  "The Archon", "Depth 20", 0);
	m = display_quest(cn, flag, page, m, questZ[ 3], questZ[ 4], 24, "Pandium",  "The Archon", "Depth 50", 0);
	m = display_quest(cn, flag, page, m, questZ[ 4], questZ[ 5], 24, "Pandium",  "The Archon", "Depth 99", 0);
	//
//	m = display_quest(cn, flag, page, m, , quest3[11], 0, "", "", "", 0);
//	m = display_quest(cn, flag, page, m, , quest3[12], 0, "", "", "", 0);
//	m = display_quest(cn, flag, page, m, , quest3[13], 0, "", "", "", 0);
//	m = display_quest(cn, flag, page, m, , quest3[14], 0, "", "", "", 0);
//	m = display_quest(cn, flag, page, m, , quest3[15], 0, "", "", "", 0);
//	m = display_quest(cn, flag, page, m, , quest3[16], 0, "", "", "", 0);
//	m = display_quest(cn, flag, page, m, , quest3[17], 0, "", "", "", 0);
//	m = display_quest(cn, flag, page, m, , quest3[18], 0, "", "", "", 0);
//	m = display_quest(cn, flag, page, m, , quest3[19], 0, "", "", "", 0);
//	m = display_quest(cn, flag, page, m, , quest3[20], 0, "", "", "", 0);
//	m = display_quest(cn, flag, page, m, , quest3[21], 0, "", "", "", 0);
//	m = display_quest(cn, flag, page, m, , quest3[22], 0, "", "", "", 0);
//	m = display_quest(cn, flag, page, m, , quest3[23], 0, "", "", "", 0);
//	m = display_quest(cn, flag, page, m, , quest3[24], 0, "", "", "", 0);
//	m = display_quest(cn, flag, page, m, , quest3[25], 0, "", "", "", 0);
//	m = display_quest(cn, flag, page, m, , quest3[26], 0, "", "", "", 0);
//	m = display_quest(cn, flag, page, m, , quest3[27], 0, "", "", "", 0);
//	m = display_quest(cn, flag, page, m, , quest3[28], 0, "", "", "", 0);
//	m = display_quest(cn, flag, page, m, , quest3[29], 0, "", "", "", 0);
//	m = display_quest(cn, flag, page, m, , quest3[30], 0, "", "", "", 0);
//	m = display_quest(cn, flag, page, m, , quest3[31], 0, "", "", "", 0);
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

void do_listbuffs(int cn, int co)
{
	int n, in, flag = 0;

	if (cn!=co)
		do_char_log(cn, 1, "Your companion's active buffs and debuffs:\n");
	else
		do_char_log(cn, 1, "Your active buffs and debuffs:\n");
	do_char_log(cn, 1, " \n");
	for (n = 0; n<MAXBUFFS; n++)
	{
		if ((in = ch[co].spell[n])!=0)
		{
			do_char_log(cn, 1, "%s for %dm %ds power of %d\n",
				bu[in].name, bu[in].active / (TICKS * 60), (bu[in].active / TICKS) % 60, bu[in].power);
			flag = 1;
		}
	}
	if (!flag)
	{
		do_char_log(cn, 1, "None.\n");
	}
	do_char_log(cn, 1, " \n");
}

void do_listgcbuffs(int cn, int shadow)
{
	int co=0, n, in, flag = 0, m = PCD_COMPANION;
	
	if (shadow) 
		m = PCD_SHADOWCOPY;
	
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
	if (ch[cn].data[11])
	{
		ch[cn].data[11] = 0;
		do_char_log(cn, 1, "Auto-Fightback enabled.\n");
	}
	else
	{
		ch[cn].data[11] = 1;
		do_char_log(cn, 1, "Auto-Fightback disabled.\n");
	}
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

int get_depot_cost(int cn)
{
	int n, in, tmp = 0;

	for (n = 0; n<62; n++)
	{
		if ((in = ch[cn].depot[n])!=0)
		{
			tmp += 1; //do_depot_cost(in);
		}
	}

	return(tmp);
}

void do_balance(int cn)
{
	int m, tmp;

	m = ch[cn].x + ch[cn].y * MAPX;
	if (!(map[m].flags & MF_BANK))
	{
		do_char_log(cn, 0, "Sorry, balance works only in banks.\n");
		return;
	}
	do_char_log(cn, 1, "Your balance is %dG %dS.\n", ch[cn].data[13] / 100, ch[cn].data[13] % 100);

	tmp = get_depot_cost(cn);

	if (tmp)
	{
		//do_char_log(cn, 1, "The rent for your depot is %dG %dS per Astonian day or %dG %dS per Earth day.\n",
		//            tmp / 100, tmp % 100, (tmp * TICKS) / 100, (tmp * TICKS) % 100);
		do_char_log(cn, 1, "You currently have %d items in your depot.\n", tmp);
	}
}

static char *order = NULL;

int qsort_proc(const void *a, const void *b)
{
	int in, in2;
	char *o;

	in  = *((int*)a);
	in2 = *((int*)b);
	
	// Locked items stay where they are
	if ((it[in].flags & IF_ITEMLOCK) || (it[in2].flags & IF_ITEMLOCK))
	{
		return 0;
	}

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
	
	qsort(ch[co].depot, 62, sizeof(int), qsort_proc);

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
		if (ch[cn].item_lock[n])
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
		do_char_log(cn, 1, "This is your bank depot. You can store up to 62 items here. Storage is free of charge.\n");
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
	if (globs->ticker - ch[cn].data[67]<TICKS * 60 && !IS_PURPLE(cn))
	{
		if ((ch[cn].x >= 549 && ch[cn].x <= 585 && ch[cn].y >= 448 && ch[cn].y <= 462)
		 || (ch[cn].x >= 564 && ch[cn].x <= 575 && ch[cn].y >= 463 && ch[cn].y <= 474))
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
	
	if (flag && (ch[cn].x >= 685 && ch[cn].x <= 729 && ch[cn].y >= 848 && ch[cn].y <= 864)
			 || (ch[cn].x >= 699 && ch[cn].x <= 715 && ch[cn].y >= 832 && ch[cn].y <= 868))
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
	else if (!flag && (ch[cn].x >= 773 && ch[cn].x <= 817 && ch[cn].y >= 780 && ch[cn].y <= 796)
				   || (ch[cn].x >= 787 && ch[cn].x <= 803 && ch[cn].y >= 775 && ch[cn].y <= 812))
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
		
		if ((ch[cn].x >= 499 && ch[cn].x <= 531 && ch[cn].y >= 504 && ch[cn].y <= 520)  // Skua
		 || (ch[cn].x >= 505 && ch[cn].x <= 519 && ch[cn].y >= 492 && ch[cn].y <= 535)  // Skua
		 || (ch[cn].x >= 773 && ch[cn].x <= 817 && ch[cn].y >= 780 && ch[cn].y <= 796)  // Gorn
		 || (ch[cn].x >= 787 && ch[cn].x <= 803 && ch[cn].y >= 775 && ch[cn].y <= 812)  // Gorn
		 || (ch[cn].x >= 685 && ch[cn].x <= 729 && ch[cn].y >= 848 && ch[cn].y <= 864)  // Kwai
		 || (ch[cn].x >= 699 && ch[cn].x <= 715 && ch[cn].y >= 832 && ch[cn].y <= 868)) // Kwai
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
	f_poh  = (ch[cn].flags & CF_POH) !=0;
	f_pol  = (ch[cn].flags & (CF_POH_LEADER | CF_GOD)) !=0;

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
		if (prefix(cmd, "poh") && f_pol)
		{
			god_set_flag(cn, dbatoi(arg[1]), CF_POH);
			return;
		}
		;
		if (prefix(cmd, "pol") && f_pol)
		{
			god_set_flag(cn, dbatoi(arg[1]), CF_POH_LEADER);
			return;
		}
		;
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
		if (prefix(cmd, "recall") && f_giu)
		{
			god_goto(cn, cn, "512", "512");
			return;
		}
		;
		if (prefix(cmd, "remsamenets") && f_giu)
		{
			char_remove_same_nets(cn, dbatoi_self(cn, arg[1]));
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
	int in, n, m[6];
	
	in = ch[cn].worn[WN_RHAND];
	
	if (!in)
	{
		return min(C_AT_CAP(cn, 5), skill[SK_HAND]);
	}
	
	if (it[in].temp==IT_TW_HEAVENS || it[in].orig_temp==IT_SEYANSWORD) // Rather than pick the matching skill, pick the highest available one
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
	
	if (it[in].flags & IF_WP_CLAW) 
	{
		return min(C_AT_CAP(cn, 5), skill[SK_HAND]);
	}

	if (it[in].flags & IF_WP_SWORD)
	{
		return min(C_AT_CAP(cn, 5), skill[SK_SWORD]);
	}
	
	if ((it[in].flags & IF_WP_DAGGER) && (it[in].flags & IF_WP_STAFF)) // Spear
	{
		return min(C_AT_CAP(cn, 5), skill[SK_DAGGER] > skill[SK_STAFF] ? skill[SK_DAGGER] : skill[SK_STAFF]);
	}
	
	if (it[in].flags & IF_WP_DAGGER)
	{
		return min(C_AT_CAP(cn, 5), skill[SK_DAGGER]);
	}
	if (it[in].flags & IF_WP_STAFF)
	{
		return min(C_AT_CAP(cn, 5), skill[SK_STAFF]);
	}
	
	if ((it[in].flags & IF_WP_AXE) && (it[in].flags & IF_WP_TWOHAND)) // Greataxe
	{
		return min(C_AT_CAP(cn, 5), skill[SK_AXE] > skill[SK_TWOHAND] ? skill[SK_AXE] : skill[SK_TWOHAND]);
	}
	if (it[in].flags & IF_WP_AXE)
	{
		return min(C_AT_CAP(cn, 5), skill[SK_AXE]);
	}
	if (it[in].flags & IF_WP_TWOHAND)
	{
		return min(C_AT_CAP(cn, 5), skill[SK_TWOHAND]);
	}
	
	if (it[in].flags & IF_OF_SHIELD)
	{
		return min(C_AT_CAP(cn, 5), skill[SK_SHIELD]);
	}

	return min(C_AT_CAP(cn, 5), skill[SK_HAND]);
}

// Combat Mastery, Dual Wield and Shield skill checks
int get_combat_skill(int cn, int skill[50], int flag)
{
	int power, bonus=0;

	if (flag>0)
	{
		power = min(C_AT_CAP(cn, 5), skill[SK_DUAL]);
		
		if (IS_SEYAN_DU(cn) || IS_ANY_MERC(cn) || IS_BRAVER(cn))
		{
			bonus = power/6;
		}
		else if (flag==2)
		{
			bonus = power/8;
		}
		else
		{	
			bonus = power/10;
		}
	}
	else
	{
		power = min(C_AT_CAP(cn, 5), skill[SK_SHIELD]);
		
		if (IS_SEYAN_DU(cn) || IS_ANY_MERC(cn) || IS_BRAVER(cn))
		{
			bonus = power/6;
		}
		else if (flag==-2)
		{
			// Tree
			if (T_ARTM_SK(cn, 12))
				bonus = power/8 + power/16;
			else
				bonus = power/8;
		}
		else
		{
			// Tree
			if (T_ARTM_SK(cn, 12))
				bonus = power/10 + power/20;
			else
				bonus = power/10;
		}
	}

	return (bonus);
}
int get_offhand_skill(int cn, int skill[50], int flag)
{
	int n, in, in2, in3; 
	
	n = 0;
	
	in  = ch[cn].worn[WN_LHAND];
	in2 = ch[cn].worn[WN_BELT];
	in3 = ch[cn].worn[WN_RHAND];
	
	// Dual Shield
	if (in3 && (it[in3].flags & IF_OF_SHIELD) && !flag)
	{
		n = get_combat_skill(cn, skill, 2);
	}
	
	// Belt - Black Belt :: Shield parry bonus while offhand is empty
	if (in2 && it[in2].temp == IT_TW_BBELT && !in && !flag)
	{
		return n+get_combat_skill(cn, skill, flag)/2;
	}
	
	// No Gear? No bonus
	if (!in || 
		(flag && !(it[in].flags & IF_OF_DUALSW)) || 
		(!flag && !(it[in].flags & IF_OF_SHIELD)))
	{
		if (!flag)
			return n;
		else
			return 0;
	}
	
	// Otherwise...
	if (n && !flag)
		return n+get_combat_skill(cn, skill, -2);
	else
		return get_combat_skill(cn, skill, flag);
}

// put in an item, see if we're wearing it in a charm slot.
int get_tarot(int cn, int in)
{
	int ch1=0, ch2=0, ch3=0, cc=0;
	
	if (ch[cn].flags & CF_NOMAGIC) return 0;
	
	// Let GC copy card effects (summ tree)
	if (IS_COMP_TEMP(cn) && IS_SANECHAR(cc = ch[cn].data[CHD_MASTER]) && T_SUMM_SK(cc, 10))
	{
		cn = cc;
	}
	
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
	int in2;
	
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
	int in2;
	
	in2 = ch[cn].worn[WN_NECK];
	
	if (in2 && (it[in2].temp == in || it[in2].orig_temp == in)) 
		return in2;
	
	return 0;
}

// put in an item, see if we're wearing that in any slot
int get_gear(int cn, int in)
{
	int n, in2;
	
	for (n = 0; n <= WN_CHARM2; n++)
	{
		if ((in2 = ch[cn].worn[n]) && (it[in2].temp == in || it[in2].orig_temp == in))
			return in2;
	}
	
	return 0;
}

int get_enchantment(int cn, int in)
{
	int n, in2, m = 0;
	
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
	
	return 0;
}

//
int is_potion(int in)
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

int is_scroll(int in)
{
	int tn;
	tn = it[in].temp;
	return (((tn >= 1314) && (tn <= 1341)) || ((tn >= 182) && (tn <= 189)));
}

// For examining a corpse for special stuff at a glance with Sense Magic.
// msg must be a do_char_log() format string like "you see %s in the corpse.\n".
void do_ransack_corpse(int cn, int co, char *msg)
{
	int in, n, sm, sm2;
	char dropname[20], dropped[100];
	
	if (!(ch[cn].flags & CF_SENSE))
		return;
	
	sm  = M_SK(cn, SK_PERCEPT);
	sm2 = sm-100;

	// Check gear slots for SS/Enchanted/Unique items
	for (n = 0; n<10; n++)
	{
		if (n==WN_NECK || n==WN_BELT || n==WN_CHARM) continue;
		if ((in = ch[co].worn[n]) && sm > RANDOM(200) && (IS_SOULSTONED(in) || IS_ENCHANTED(in) || is_unique(in)))
		{
			switch(n)
			{
				case WN_HEAD: strcpy(dropname, "helmet"); break;
				case WN_BODY: strcpy(dropname, "body armor"); break;
				case WN_ARMS: strcpy(dropname, "pair of gloves"); break;
				case WN_FEET: strcpy(dropname, "pair of boots"); break;
				case WN_LHAND: 
					if (it[in].flags & IF_OF_DUALSW) 	strcpy(dropname, "weapon"); 
					else 								strcpy(dropname, "shield"); 
					break;
				case WN_RHAND: strcpy(dropname, "weapon"); break;
				case WN_CLOAK: strcpy(dropname, "cloak"); break;
				default: break;
			}
			if (is_unique(in))
			{
				if (sm2>RANDOM(200)) 	sprintf(dropped, "a unique %s", it[in].name);
				else 					sprintf(dropped, "a unique %s", dropname);
			}
			else if (IS_SOULSTONED(in) && IS_ENCHANTED(in))
			{
				if (sm2>RANDOM(200)) 	sprintf(dropped, "a soulstoned and enchanted %s", it[in].name);
				else 					sprintf(dropped, "a soulstoned and enchanted %s", dropname);
			}
			else if (IS_SOULSTONED(in))
			{
				if (sm2>RANDOM(200)) 	sprintf(dropped, "a soulstoned %s", it[in].name);
				else 					sprintf(dropped, "a soulstoned %s", dropname);
			}
			else if (IS_ENCHANTED(in))
			{
				if (sm2>RANDOM(200)) 	sprintf(dropped, "an enchanted %s", it[in].name);
				else 					sprintf(dropped, "an enchanted %s", dropname);
			}
			do_char_log(cn, 0, msg, dropped);
		}
	}
	// Check other accessory slots
	if ((in = ch[co].worn[WN_NECK]) && sm > RANDOM(200))
	{
		if (sm2>RANDOM(200))
			sprintf(dropped, "a %s", it[in].name);
		else
			strcpy(dropped, "a magical amulet");
		do_char_log(cn, 0, msg, dropped);
	}
	if ((in = ch[co].worn[WN_BELT]) && sm > RANDOM(200))
	{
		if (sm2>RANDOM(200))
			sprintf(dropped, "a %s", it[in].name);
		else
			strcpy(dropped, "a magical belt");
		do_char_log(cn, 0, msg, dropped);
	}
	if ((in = ch[co].worn[WN_LRING]) && !IS_SOULSTONE(in) && !IS_GEMSTONE(in) && sm > RANDOM(200))
	{
		if (sm2>RANDOM(200))
			sprintf(dropped, "a %s", it[in].name);
		else
			strcpy(dropped, "a magical ring");
		do_char_log(cn, 0, msg, dropped);
	}
	if ((in = ch[co].worn[WN_RRING]) && !IS_SOULSTONE(in) && !IS_GEMSTONE(in) && sm > RANDOM(200))
	{
		if (sm2>RANDOM(200))
			sprintf(dropped, "a %s", it[in].name);
		else
			strcpy(dropped, "a magical ring");
		do_char_log(cn, 0, msg, dropped);
	}
	// Check for items in inventory
	for (n = 0; n<MAXITEMS; n++)
	{
		if (!(in = ch[co].item[n]))
		{
			continue;
		}
		if(!(it[in].flags & IF_MAGIC))
		{
			continue;                      // this item havent 'magic' flag
		}
		if (is_unique(in) && sm > RANDOM(200))
		{
			if (sm2>RANDOM(200))
				sprintf(dropped, "a unique %s", it[in].name);
			else
			{
				if ((it[in].placement & PL_WEAPON) || (it[in].flags & IF_OF_DUALSW))
					strcpy(dropped, "a unique weapon");
				else
					strcpy(dropped, "a unique shield");
			}
			do_char_log(cn, 0, msg, dropped);
			continue;
		}
		if (is_scroll(in) && sm > RANDOM(200))
		{
			if (sm2>RANDOM(200))
				sprintf(dropped, "a %s", it[in].name);
			else
				strcpy(dropped, "a magical scroll");
			do_char_log(cn, 0, msg, dropped);
			continue;
		}
		if (IS_SOULSTONE(in) && sm > RANDOM(200))
		{
			do_char_log(cn, 0, msg, "a soulstone");
			continue;
		}
		if (IS_SOULFOCUS(in) && sm > RANDOM(200))
		{
			do_char_log(cn, 0, msg, "a soul focus");
			continue;
		}
		if (IS_SOULCAT(in) && sm > RANDOM(200))
		{
			do_char_log(cn, 0, msg, "a soul catalyst");
			continue;
		}
		if (IS_CONTRACT(in) && sm > RANDOM(200))
		{
			do_char_log(cn, 0, msg, "a contract");
			continue;
		}
		if (IS_QUILL(in) && sm > RANDOM(200))
		{
			do_char_log(cn, 0, msg, "a quill");
			continue;
		}
		if (IS_GEMSTONE(in) && sm > RANDOM(200))
		{
			if (sm2>RANDOM(200))
				sprintf(dropped, "a %s", it[in].name);
			else
				strcpy(dropped, "a magical gem");
			do_char_log(cn, 0, msg, dropped);
		}
		if (is_potion(in) && sm > RANDOM(200))
		{
			if (sm2>RANDOM(200))
				sprintf(dropped, "a %s", it[in].name);
			else
				strcpy(dropped, "a magical potion");
			do_char_log(cn, 0, msg, dropped);
			continue;
		}
		if ((it[in].placement & PL_NECK) && sm > RANDOM(200))
		{
			if (sm2>RANDOM(200))
				sprintf(dropped, "a %s", it[in].name);
			else
				strcpy(dropped, "a magical amulet");
			do_char_log(cn, 0, msg, dropped);
			continue;
		}
		if ((it[in].placement & PL_BELT) && sm > RANDOM(200))
		{
			if (sm2>RANDOM(200))
				sprintf(dropped, "a %s", it[in].name);
			else
				strcpy(dropped, "a magical belt");
			do_char_log(cn, 0, msg, dropped);
			continue;
		}
		if ((it[in].placement & PL_RING) && !IS_SOULSTONE(in) && !IS_GEMSTONE(in) && sm > RANDOM(200))
		{
			if (sm2>RANDOM(200))
				sprintf(dropped, "a %s", it[in].name);
			else
				strcpy(dropped, "a magical ring");
			do_char_log(cn, 0, msg, dropped);
			continue;
		}
		if ((it[in].placement & PL_CHARM) && sm > RANDOM(200))
		{
			if (sm2>RANDOM(200))
				sprintf(dropped, "a %s", it[in].name);
			else
				strcpy(dropped, "a tarot card");
			do_char_log(cn, 0, msg, dropped);
			continue;
		}
	}
	
	//chlog(cn, "Sense magic message sent OK");
}

// note: cn may be zero!!
void do_char_killed(int cn, int co, int pentsolve)
{
	int n, in, x, y, temp = 0, m, tmp, tmpg, wimp, cc = 0, fn, r1, r2, rank, grouped = 0;
	unsigned long long mf;
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

	mf = map[ch[co].x + ch[co].y * MAPX].flags;
	if (cn)
	{
		mf &= map[ch[cn].x + ch[cn].y * MAPX].flags;
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
	if (cn && cn!=co && (ch[cn].flags & (CF_PLAYER)) && !(mf & MF_ARENA))
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
		
		if (n = get_gear(cn, IT_WP_LAMEDARG)) it[n].data[0]++;		
		
		// becoming purple
		if (ch[co].temp==CT_PRIEST)   // add all other priests of the purple one here...
		{
			if (IS_PURPLE(cn))
			{
				do_char_log(cn, 1, "Ahh, that felt good!\n");
			}
			else
			{
				ch[cn].data[67] = globs->ticker;
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
				do_give_exp(cn, do_char_score(co) * 25, 0, -1);
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
							do_give_exp(n, do_char_score(co) * 25, 0, -1);
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
				do_give_exp(cc, do_char_score(co) * 25, 0, -1);
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
							do_give_exp(n, do_char_score(co) * 25, 0, -1);
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

	if ((mf & MF_ARENA) || is_atpandium(co) || is_incolosseum(co, 0))
	{
		// Arena death : full save, keep everything
		wimp = 2;
	}
	else if (!IS_PURPLE(co))
	{
		// Skua death : Skua save, drop items + 5% exp and 50% gold
		wimp = 1;
	}
	else
	{
		// Purple death : Purple save, drop equipment + 5% exp and 100% gold
		wimp = 0;
	}

	// drop items and money in original place
	if (ch[co].flags & (CF_PLAYER))
	{
		// Newbie death : full save, keep everything
		if (getrank(co)<5)
		{
			wimp = 2;
			do_char_log(co, 0, "You would have dropped your items, but seeing you're still inexperienced the gods kindly returned them. Stay safe!\n");
		}
		
		// Dying in a contract - set grave X and Y to in front of Osiris instead.
		if (CONT_NUM(co))
		{
			x = 748;
			y = 989;
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
			else
			{
				if (wimp==1)
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
			ch[cc].alt_worn[n] = 0;
			// Hacky alt-worn drop method.
			if (wimp==0)
			{
				ch[cc].item[n] = in;
				it[in].carried = cc;
				ch[co].alt_worn[n] = 0;
				chlog(co, "Dropped %s (t=%d) in Grave", it[in].name, it[in].temp);
			}
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

		ch[co].a_hp = 10000;              // come alive! (10hp)
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
			if (IS_PLAYER_COMP(cn))
				do_ransack_corpse(CN_OWNER(cn), co, "You notice %s tumble into the grave of your companion's victim.\n");
			else
				do_ransack_corpse(cn, co, "You notice %s tumble into the grave of your victim.\n");
		}
		do_update_char(co);
	}
	else            // CF_LABKEEPER, or pent mob auto-poofed
	{
		int z;

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

void do_give_exp(int cn, int p, int gflag, int rank)
{
	int n, c, co, s, master;

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
				if ((co = ch[cn].data[n])!=0 && isgroup(co, cn) && do_char_can_see(cn, co))
				{
					c++;
				}
			}

			for (n = 1, s = 0; n<10; n++)
			{
				if ((co = ch[cn].data[n])!=0 && isgroup(co, cn) && do_char_can_see(cn, co))
				{
					do_give_exp(co, p / c, 0, rank);
					s += p / c;
				}
			}
			do_give_exp(cn, p - s, 0, rank);
		}
		else     // we're an NPC
		{
			if ((co = ch[cn].data[CHD_MASTER])!=0) // we are the follower of someone
			{
				do_give_exp(cn, p, 0, rank);
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
		if (p)
		{
			chlog(cn, "Gets %d EXP", p);
			ch[cn].points += p;
			ch[cn].points_tot += p;
			
			if (!(ch[cn].flags & CF_SYS_OFF))
				do_char_log(cn, 2, "You get %d experience points.\n", p);

			//do_char_log(cn, 1, "Experience until next rank: %d\n", points_tolevel(ch[cn].points_tot));
			do_notify_char(cn, NT_GOTEXP, p, 0, 0, 0);
			do_update_char(cn);
			do_check_new_level(cn, 1);
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
				if ((co = ch[cn].data[n])!=0 && isgroup(co, cn) && do_char_can_see(cn, co))
					c+=75;
			}
			for (n = 1; n<10; n++)
			{
				if ((co = ch[cn].data[n])!=0 && isgroup(co, cn) && do_char_can_see(cn, co))
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
				if ((co = ch[cn].data[n])!=0 && isgroup(co, cn) && do_char_can_see(cn, co))
					c+=75;
			}
			for (n = 1; n<10; n++)
			{
				if ((co = ch[cn].data[n])!=0 && isgroup(co, cn) && do_char_can_see(cn, co))
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
	int tmp = 0, n, in, rank = 0, noexp = 0, halfexp = 0, kill_bsp = 0, kill_osp = 0;
	unsigned long long mf;
	int hp_dam = 0, end_dam = 0, mana_dam = 0;
	int scorched = 0, guarded = 0, devRn = 0, devRo = 0, phalanx = 0, aggravate = 0;
	int offpot = 0, defpot = 0;
	int thorns = 0, crit_dam = 0;
	
	mf = map[ch[co].x + ch[co].y * MAPX].flags;
	if (cn)
	{
		mf |= map[ch[cn].x + ch[cn].y * MAPX].flags;
	}

	if (ch[co].flags & CF_BODY)
	{
		return 0;
	}

	if ((ch[co].flags & (CF_PLAYER)) && type!=3)
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
	
	// Invidia
	if (get_gear(co, IT_TW_INVIDIA) && IS_SANECHAR(ch[co].data[PCD_COMPANION]) 
		&& ch[ch[co].data[PCD_COMPANION]].data[CHD_MASTER]==co && !(ch[ch[co].data[PCD_COMPANION]].flags & CF_BODY))
	{
		co = ch[co].data[PCD_COMPANION];
	}
	
	// Loop to look for Magic Shield so we can damage it
	for (n = 0; n<MAXBUFFS; n++)
	{
		if ((in = ch[co].spell[n])!=0)
		{
			if (bu[in].temp==SK_MSHIELD)
			{
				if (IS_SEYA_OR_BRAV(co))
					tmp = bu[in].active / 1536 + 1;
				else
					tmp = bu[in].active / 1024 + 1;
				tmp = (dam + tmp - ch[co].armor) * 5;
				
				// Book - Great Divide :: half duration damage dealt to shield/shell
				if (get_book(cn, IT_BOOK_SHIV)) tmp /= 2;

				if (tmp>0)
				{
					if (tmp>=bu[in].active)
					{
						ch[co].spell[n] = 0;
						bu[in].used = 0;
						do_update_char(co);
					}
					else
					{
						bu[in].active -= tmp;
						if (IS_SEYA_OR_BRAV(co))
							bu[in].armor[1] = min(127, bu[in].active / 1536 + 1);
						else
							bu[in].armor[1] = min(127, bu[in].active / 1024 + 1);
						bu[in].power = bu[in].active / 256;
						do_update_char(co);
					}
				}
			}
		}
	}
	
	// Easy new method!
	if (cn) dam = dam * ch[cn].dmg_bonus / 10000;
	
	if (type==3 || type==13)
	{
		dam = dam * ch[co].dmg_reduction / 10000;
		if (T_BRAV_SK(co, 6)) dam /= 2;
		dam *= DAM_MULT_THORNS; 						// Thorns
		if (type==13) dam = dam/10;
		
		if ((mf & MF_NOFIGHT) && !(IS_IN_BOJ(ch[cn].x, ch[cn].y) && get_gear(cn, IT_XIXDARKSUN)))
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
				case  1: dam *= DAM_MULT_BLAST;    break; // Blast
				case  2: dam *= DAM_MULT_HOLYW;    break; // Holy Water / Staff of Kill Undead
				case  5: dam *= DAM_MULT_CLEAVE;   break; // Cleave
				case  6: dam *= DAM_MULT_PULSE;    break; // Pulse
				case  7: dam *= DAM_MULT_ZEPHYR;   break; // Zephyr
				case  8: dam *= DAM_MULT_LEAP;     break; // Leap
				default: dam *= DAM_MULT_HIT;      break; // Hit / Surround Hit / Crit
			}
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
		if ((type==0 || type==4 || type==5 || type==8 || type==9) && ch[co].gethit_dam>0)
		{
			thorns = ch[co].gethit_dam;
			do_hurt(co, cn, thorns/2 + RANDOM(thorns/2)+1, 3);
		}
		
		// Enchantments
		if (n = get_enchantment(cn, 49)) ch[cn].a_hp   += n*1000*((IS_USETWOHAND(cn))?2:1);
		if (n = get_enchantment(cn, 42)) ch[cn].a_end  += n*1000*((IS_USETWOHAND(cn))?2:1);
		if (n = get_enchantment(co, 43)) ch[co].a_end  += n*2000;
		if (n = get_enchantment(cn, 38)) ch[cn].a_mana += n*1000*((IS_USETWOHAND(cn))?2:1);
		if (n = get_enchantment(co, 39)) ch[co].a_mana += n*2000;
		
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
	
	// Gula
	if (get_gear(cn, IT_TW_GULA))
	{
		ch[cn].a_hp += dam/8;
	}
	
	hp_dam = dam;
	
	// Tarot - High Priestess - damage dealt to mana instead
	if (get_tarot(co, IT_CH_PREIST))
	{
		mana_dam += dam/10;
		
		if (ch[co].a_mana - mana_dam<500)
			mana_dam = (ch[co].a_mana-500);
		
		hp_dam -= mana_dam*2;
	}
	if (get_enchantment(co, 23))
	{
		mana_dam += dam/20;
		
		if (ch[co].a_mana - mana_dam<500)
			mana_dam = (ch[co].a_mana-500);
		
		hp_dam -= mana_dam;
	}
	if (T_ARHR_SK(co, 12))
	{
		mana_dam += dam/10;
		
		if (ch[co].a_mana - mana_dam<500)
			mana_dam = (ch[co].a_mana-500);
		
		hp_dam -= mana_dam;
	}
	
	// Cloak - Cloak of Shadows - damage dealt to end instead
	if (get_gear(co, IT_TW_CLOAK))
	{
		end_dam += dam/10;
		
		if (ch[co].a_end - end_dam<500)
			end_dam = (ch[co].a_end-500);
		
		hp_dam -= end_dam*2;
	}
	if (get_enchantment(co, 16))
	{
		end_dam += dam/20;
		
		if (ch[co].a_end - end_dam<500)
			end_dam = (ch[co].a_end-500);
		
		hp_dam -= end_dam;
	}
	if (T_SKAL_SK(co, 12))
	{
		end_dam += dam/10;
		
		if (ch[co].a_end - end_dam<500)
			end_dam = (ch[co].a_end-500);
		
		hp_dam -= end_dam;
	}
	
	// Weapon - Bronchitis :: 20% cleave damage also dealt to mana
	if (type == 5 && get_gear(cn, IT_WP_BRONCHIT))
	{
		mana_dam += hp_dam/5;
	}
	
	if (ch[co].a_hp - hp_dam<500 && !(mf & MF_ARENA) && try_lucksave(co))
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
	
	// Enchantments
	if (n = get_enchantment(cn, 49)) ch[cn].a_hp   += n*1000*((IS_USETWOHAND(cn))?2:1);
	if (n = get_enchantment(cn, 42)) ch[cn].a_end  += n*1000*((IS_USETWOHAND(cn))?2:1);
	if (n = get_enchantment(co, 43)) ch[co].a_end  += n*2000;
	if (n = get_enchantment(cn, 38)) ch[cn].a_mana += n*1000*((IS_USETWOHAND(cn))?2:1);
	if (n = get_enchantment(co, 39)) ch[co].a_mana += n*2000;
	
	// force to sane values
	if (ch[cn].a_hp>ch[cn].hp[5] * 1000)     ch[cn].a_hp   = ch[cn].hp[5]   * 1000;
	if (ch[cn].a_end>ch[cn].end[5] * 1000)   ch[cn].a_end  = ch[cn].end[5]  * 1000;
	if (ch[co].a_end>ch[co].end[5] * 1000)   ch[co].a_end  = ch[co].end[5]  * 1000;
	if (ch[cn].a_mana>ch[cn].mana[5] * 1000) ch[cn].a_mana = ch[cn].mana[5] * 1000;
	if (ch[co].a_mana>ch[co].mana[5] * 1000) ch[co].a_mana = ch[co].mana[5] * 1000;

	if (ch[co].a_hp<10000 && ch[co].a_hp>=500 && getrank(co)<5)
	{
		do_char_log(co, 0, "You're almost dead... Use a potion, quickly!\n");
	}

	if (ch[co].a_hp<500)
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
		
		if (type!=2 && cn && !(mf & MF_ARENA) && !noexp)
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
			}
			if (IS_GLOB_MAYHEM) tmp += tmp / 5;
			if (ch[co].flags & CF_EXTRAEXP)  tmp = tmp * 2;
			if (ch[co].flags & CF_EXTRACRIT) tmp = tmp * 3/2;
		}
		
		if (type!=2 && cn && cn!=co && !(mf & MF_ARENA) && !noexp)
		{
			if ((in = get_gear(cn, IT_FORTERING)) && it[in].active) // 25% more exp with Forte Ring
			{
				tmp = tmp*4/3;
			}
			do_give_exp(cn, tmp, 1, rank);
			
			// stronghold points for contract
			if (kill_bsp)
				do_give_bspoints(cn, tmp*kill_bsp*RATE_P_ENBS/100, 1);
			// osiris points for contract
			if (kill_osp)
				do_give_ospoints(cn, tmp*kill_osp*RATE_P_ENOS/100, 1);
			
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
		if ((type==0 || type==4 || type==5 || type==8 || type==9) && ch[co].gethit_dam>0)
		{
			thorns = ch[co].gethit_dam;
			do_hurt(co, cn, thorns/2 + RANDOM(thorns/2)+1, 3);
		}
	}

	return (dam / 1000);
}

int do_surround_check(int cn, int co, int gethit)
{
	int cc, n;
	
	cc = ch[cn].attack_cn;
	
	if (cn==0 || co==0 || cn==co) return 0;
	if (!IS_PLAYER(cn) && ch[cn].data[CHD_GROUP] == ch[co].data[CHD_GROUP]) return 0;	// Same Group
	if (ch[cn].attack_cn!=co && ch[co].alignment==10000) return 0;	// Ignore friendly npcs unless targeted
	if (ch[co].attack_cn!=cn && ch[cn].alignment==10000) return 0;	// Ignore friendly npcs unless targeted
	if (ch[cn].attack_cn!=co && (strcmp(ch[co].name, "Gate Guard")==0 || strcmp(ch[co].name, "Outpost Guard")==0)) return 0;	// Ignore BS gate npcs
	if (ch[co].attack_cn!=cn && (strcmp(ch[cn].name, "Gate Guard")==0 || strcmp(ch[cn].name, "Outpost Guard")==0)) return 0;	// Ignore BS gate npcs
	if (ch[cn].data[PCD_COMPANION] == co || ch[cn].data[PCD_SHADOWCOPY] == co) return 0; // Ignore own GC/SC
	
	if (!IS_PLAYER(cn) && !IS_PLAYER(cc) && !IS_COMPANION(cc) && (IS_PLAYER(co) || IS_COMPANION(co))) return 0;
	if (!IS_PLAYER(cn) && (IS_PLAYER(cc) || IS_COMPANION(cc)) && !IS_PLAYER(co) && !IS_COMPANION(co)) return 0;
	
	if (ch[cn].data[CHD_MASTER] == co || ch[co].data[CHD_MASTER] == cn) return 0;	// Ignore master
	
	if (IS_COMPANION(cn) && ch[cn].data[CHD_MASTER])
	{
		if (ch[ch[cn].data[CHD_MASTER]].data[PCD_COMPANION] == co) return 0;
		if (ch[ch[cn].data[CHD_MASTER]].data[PCD_SHADOWCOPY] == co) return 0;
		if (!IS_PLAYER(ch[cn].data[CHD_MASTER]) && ch[ch[cn].data[CHD_MASTER]].data[CHD_GROUP] == ch[co].data[CHD_GROUP]) return 0;
	}
	if (IS_COMPANION(co) && ch[co].data[CHD_MASTER])
	{
		if (ch[ch[co].data[CHD_MASTER]].data[PCD_COMPANION] == cn) return 0;
		if (ch[ch[co].data[CHD_MASTER]].data[PCD_SHADOWCOPY] == cn) return 0;
		if (!IS_PLAYER(ch[co].data[CHD_MASTER]) && ch[ch[co].data[CHD_MASTER]].data[CHD_GROUP] == ch[cn].data[CHD_GROUP]) return 0;
	}
	
	if ((ch[cn].flags & CF_PLAYER) && (ch[co].flags & CF_PLAYER))
	{
		if (isgroup(cn, co) && isgroup(co, cn)) return 0;	// Ignore group members
		if (is_atpandium(cn) && is_atpandium(co)) return 0; // Ignore if we're fighting Pandium
	}
	
	if (!(ch[cn].flags & CF_PLAYER) && ch[co].temp==ch[cn].data[31]) return 0;
	
	if (gethit && (!do_char_can_see(cn, co) || !may_attack_msg(cn, co, 0) || ch[co].flags & CF_IMMORTAL)) return 0;
	if (gethit && ch[co].data[CHD_MASTER]) if (gethit && !may_attack_msg(cn, ch[co].data[CHD_MASTER], 0)) return 0;
	
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
	if (T_BRAV_SK(co, 12)) 			crit_redc -= 50;
	
	return max(0, crit_dam*crit_redc/100);
}

void do_attack(int cn, int co, int surround) // surround = 2 means it's a SURROUND RATE hit
{
	int hit, dam = 0, die, m, mc, odam = 0, cc = 0;
	int chance, s1, s2, bonus = 0, diff, crit_dam=0, in=0, co_orig=-1;
	int surrDam, surrBonus, surrTotal, n;
	int glv, glv_base = 120;
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

	// Outsider's Eye & tree skill
	if (!get_neck(co, IT_TW_OUTSIDE) && !T_WARR_SK(co, 10))
	{
		if (!is_facing(co, cn))
		{
			s2 -= 10;
		}
		if (is_back(co, cn))
		{
			s2 -= 10;
		}
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
	
	if (m = get_enchantment(cn, 41)) chance += m;
	if (m = get_enchantment(co, 37)) chance -= m;
	
	die = RANDOM(20) + 1;
	if (die<=chance)
	{
		hit = 1;
	}
	else
	{
		hit = 0;
	}
	
	if (IS_COMPANION(cn) && !(ch[cn].flags & CF_SHADOWCOPY) && IS_SANECHAR(cc = ch[cn].data[CHD_MASTER]) && get_tarot(cc, IT_CH_HEIROP_R) && !RANDOM(5))
		hit = 0;
	
	if (get_tarot(co, IT_CH_WHEEL_R) && !RANDOM(4))
		hit = 1;

	if (hit)
	{
		dam = ch[cn].weapon + RANDOM(9);
		if (ch[cn].top_damage>3)
		{
			// Tree
			if (T_SKAL_SK(cn, 6))
				dam += max(RANDOM(ch[cn].top_damage), RANDOM(ch[cn].top_damage));
			else
				dam += RANDOM(ch[cn].top_damage);
		}
		
		// Tarot - Strength - 20% more damage
		if (get_tarot(cn, IT_CH_STRENGTH))
		{
			dam = dam*6/5;
		}
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
			glv = glv_base;
			if (chance_compare(co, glv_base+glv_base/2+RANDOM(20), get_target_resistance(cn, co)+RANDOM(10), 0))
			{
				in = it[ch[cn].worn[WN_ARMS]].temp;
				if (in==IT_GL_SERPENT) 
				{ 
					if (spell_poison(cn, co, glv, 1))
					{
						if (!(ch[cn].flags & CF_SYS_OFF)) 
							do_char_log(cn, 0, "You poisoned your enemies!\n"); 
					}
					else in=0; 
				}
				if (in==IT_GL_BURNING) 
				{ 
					if (spell_scorch(cn, co, glv, 1))
					{	
						if (!(ch[cn].flags & CF_SYS_OFF)) 
							do_char_log(cn, 0, "You scorched your enemies!\n"); 
					}
					else in=0; 
				}
				if (in==IT_GL_SHADOW)  
				{ 
					if (spell_blind(cn, co, glv, 0))
					{	
						if (!(ch[cn].flags & CF_SYS_OFF)) 
							do_char_log(cn, 0, "You blinded your enemies!\n"); 
					}
					else in=0; 
				}
				if (in==IT_GL_CHILLED) 
				{ 
					if (spell_slow(cn, co, glv, 1))
					{ 
						if (!(ch[cn].flags & CF_SYS_OFF)) 
							do_char_log(cn, 0, "You slowed your enemies!\n");   
					}
					else in=0; 
				}
				if (in==IT_GL_CURSED)  
				{ 
					if (spell_curse(cn, co, glv, 1))
					{
						if (!(ch[cn].flags & CF_SYS_OFF)) 
							do_char_log(cn, 0, "You cursed your enemies!\n");   
					}
					else in=0; 
				}
				if (in==IT_GL_TITANS)  
				{ 
					if (spell_weaken(cn, co, glv, 1))
					{
						if (!(ch[cn].flags & CF_SYS_OFF)) 
							do_char_log(cn, 0, "You weakened your enemies!\n"); 
					}
					else in=0; 
				}
				if (in==IT_GL_BLVIPER) 
				{ 
					if (spell_frostburn(cn, co, glv))
					{
						if (!(ch[cn].flags & CF_SYS_OFF)) 
							do_char_log(cn, 0, "You glaciated your enemies!\n"); 
					}
					else in=0; 
				}
				if (in==IT_TW_DOUSER)  
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
			glv_base += 120;
			glv = glv_base;
			if (chance_compare(co, glv_base+glv_base/2+RANDOM(20), get_target_resistance(cn, co)+RANDOM(10), 0))
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
		
		if (surround && B_SK(cn, SK_SURROUND))
		{
			if (IS_SKALD(cn))
			{
				surrDam = odam + crit_dam/2;
				glv 	= glv_base;
			}
			else
			{
				surrDam = odam/4*3 + crit_dam/2;
				glv 	= glv_base/4*3;
			}
			
			if (surround==1 && (IS_SEYA_OR_ARTM(cn) || get_gear(cn, IT_WP_CROSSBLAD)) && 
				!(ch[cn].flags & CF_AREA_OFF))
			{
				int surraoe, x, y, xf, yf, xt, yt, xc, yc, obsi = 0, aoe_power;
				double tmp_a, tmp_h, tmp_s, tmp_g;
				
				aoe_power = M_SK(cn, SK_PROX)+15;
				obsi 	= ch[cn].aoe_bonus;
				obsi   += (IS_ARCHTEMPLAR(cn)?4:0)+(get_gear(cn, IT_WP_CROSSBLAD)?4:0);
				surraoe = (aoe_power/PROX_CAP) + obsi;
				tmp_a	= (double)(aoe_power*100/PROX_CAP + obsi*100);
				tmp_h   = (double)(sqr(aoe_power*100/PROX_HIT-tmp_a)/500 + obsi*300);
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
						if (M_SK(cn, SK_SURROUND) + RANDOM(40)>=ch[co].to_parry)
						{
							surrBonus = 0;
							if ((M_SK(cn, SK_SURROUND)-ch[co].to_parry)>0)
							{
								surrBonus = odam/4 * min(max(1,M_SK(cn, SK_SURROUND)-ch[co].to_parry), 20)/20;
							}
							surrTotal = surrDam+surrBonus;
							if (co==co_orig) surrTotal = surrTotal/4*3;
							do_hurt(cn, co, surrTotal, 4);
							if (chance_compare(co, glv_base+glv_base/2+RANDOM(20), get_target_resistance(cn, co)+RANDOM(16), 0) && co!=co_orig)
							{
								if (in==IT_GL_SERPENT) spell_poison(cn, co, glv, 1);
								if (in==IT_GL_BURNING) spell_scorch(cn, co, glv, 1);
								if (in==IT_GL_SHADOW ) spell_blind(cn, co, glv, 0);
								if (in==IT_GL_CHILLED) spell_slow(cn, co, glv, 1);
								if (in==IT_GL_CURSED ) spell_curse(cn, co, glv, 1);
								if (in==IT_GL_TITANS ) spell_weaken(cn, co, glv, 1);
								if (in==IT_GL_BLVIPER) spell_frostburn(cn, co, glv);
								if (in==IT_TW_DOUSER ) spell_blind(cn, co, glv, 1);
								if (in==IT_TW_LUXURIA) spell_warcry(cn, co, glv, 1);
								if (ch[co].spellfail==1) ch[co].spellfail = 0;
							}
							if (in2 && co!=co_orig && !get_tarot(cn, IT_CH_DEATH_R))
							{
								spell_zephyr(cn, co, bu[in2].power/2 + bu[in2].power/4, 1);
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
					if ((co = map[mc].ch)!=0 && ch[co].attack_cn==cn)
					{
						if (	(surround==1 && M_SK(cn, SK_SURROUND) + RANDOM(40)>=ch[co].to_parry) 
							|| 	(surround==2 && M_SK(cn, SK_SURROUND) + RANDOM(20)>=ch[co].to_parry))
						{
							surrBonus = 0;
							if (surround==1 && (M_SK(cn, SK_SURROUND)-ch[co].to_parry)>0)
							{
								surrBonus = odam/4 * min(max(1,M_SK(cn, SK_SURROUND)-ch[co].to_parry), 20)/20;
							}
							if (surround==2 && (M_SK(cn, SK_SURROUND)-ch[co].to_parry)>20)
							{
								surrBonus = odam/4 * min(max(1,M_SK(cn, SK_SURROUND)-ch[co].to_parry-20), 20)/20;
							}
							surrTotal = surrDam+surrBonus;
							if (co==co_orig) 
							{
								if (IS_SKALD(cn))
									surrTotal = surrTotal/2;
								else
									surrTotal = surrTotal/4*3;
							}
							do_hurt(cn, co, surrTotal, 4);
							if (chance_compare(co, glv_base+glv_base/2+RANDOM(20), get_target_resistance(cn, co)+RANDOM(16), 0) && co!=co_orig)
							{
								if (in==IT_GL_SERPENT) spell_poison(cn, co, glv, 1);
								if (in==IT_GL_BURNING) spell_scorch(cn, co, glv, 1);
								if (in==IT_GL_SHADOW ) spell_blind(cn, co, glv, 0);
								if (in==IT_GL_CHILLED) spell_slow(cn, co, glv, 1);
								if (in==IT_GL_CURSED ) spell_curse(cn, co, glv, 1);
								if (in==IT_GL_TITANS ) spell_weaken(cn, co, glv, 1);
								if (in==IT_GL_BLVIPER) spell_frostburn(cn, co, glv);
								if (in==IT_TW_DOUSER ) spell_blind(cn, co, glv, 1);
								if (in==IT_TW_LUXURIA) spell_warcry(cn, co, glv, 1);
							}
							if (in2 && co!=co_orig && !get_tarot(cn, IT_CH_DEATH_R))
							{
								spell_zephyr(cn, co, bu[in2].power/2 + bu[in2].power/4, 1);
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
	}
	else
	{
		int thorns;
		
		do_area_sound(co, 0, ch[co].x, ch[co].y, ch[cn].sound + 5);
		char_play_sound(co, ch[cn].sound + 5, -150, 0);

		do_area_notify(cn, co, ch[cn].x, ch[cn].y, NT_SEEMISS, cn, co, 0, 0);
		do_notify_char(co, NT_GOTMISS, cn, 0, 0, 0);
		do_notify_char(cn, NT_DIDMISS, co, 0, 0, 0);
		
		// Tarot - Death.R : Trigger, imm/res, and damage flip-flop
		if (get_tarot(co, IT_CH_DEATH_R) && (in2 = has_spell(co, SK_ZEPHYR)))
		{
			spell_zephyr(co, cn, bu[in2].power, 1);
		}
		
		// Tree - artm
		if ((thorns = ch[co].gethit_dam)>0 && T_ARTM_SK(co, 6))
		{
			do_hurt(co, cn, thorns/2 + RANDOM(thorns/2)+1, 13);
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
		return 0;                      // lag scroll

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

int do_char_can_see(int cn, int co)
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
		(isgroup(co, cn) && isgroup(cn, co)))
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
	if (!(ch[cn].flags & CF_INFRARED))
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
		if (it[in].driver==57 && it[in].data[8] && (ch[cn].rebirth & 1)) return 1;
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

void do_update_char(int cn)
{
	ch[cn].flags |= (CF_UPDATE | CF_SAVEME);
}
void really_update_char(int cn)
{
	int n, m, t, oldlight, z, sublight = 0, maxlight = 0, co=0, bits=0;
	int hp = 0, end = 0, mana = 0, weapon = 0, armor = 0, light = 0, gethit = 0, infra = 0, coconut = 0, pigsblood = 0, priestess = 0;
	int heal_hp, heal_end, heal_mana;
	int tempWeapon = 0, tempArmor = 0, bbelt = 0, wbelt = 0;
	int isCurse1 = 0, isSlow1 = 0, isWeaken1 = 0, isCurse2 = 0, isSlow2 = 0, isWeaken2 = 0;
	int divCursed = 1, divSlowed = 1, divWeaken = 1;
	int hastePower = 0, slowPower = 0, hasteSpeed = 0, slowSpeed = 0, slow2Speed = 0, sickStacks = 0;
	int attrib[5];
	int attrib_ex[5];
	int skill[50];
	unsigned long long prof;
	int charmSpec = 0, gearSpec = 0, nightSpec = 0, gemCheck = 0, gemSpec = 0, enchantSpec = 0;
	int moreBrv = 0, moreWil = 0, moreInt = 0, moreAgl = 0, moreStr = 0, moreEnd = 0, moreAtt = 0, moreGrd = 0, moreDmg = 0;
	int in=0;
	//int speedvalue_a = 0, speedvalue_b = 0, speedvalue_c = 0;
	int base_spd = 0, spd_move = 0, spd_attack = 0, spd_cast = 0;
	int spell_mod = 0, spell_apt = 0, spell_cool = 0;
	int critical_b = 0, critical_c = 0, critical_m = 0;
	int hit_rate = 0, parry_rate = 0, loverSplit = 0;
	int damage_top = 0, ava_crit = 0, ava_mult = 0;
	int aoe = 0, tempCost = 10000, dmg_bns = 10000, dmg_rdc = 10000, reduc_bonus = 0;
	int suppression = 0, bcount=0, attaunt=0, labcmd=0, gcdivinity = 0;
	
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
	if (IS_PLAYER(cn)) ch[cn].data[25] = 0;
	
	base_spd = spd_move = spd_attack = spd_cast = 0;
	spell_mod = spell_apt = spell_cool = 0;
	critical_b = critical_c = critical_m = 0;
	hit_rate = parry_rate = 0;
	damage_top = 0;
	dmg_bns = dmg_rdc = tempCost = 10000;
	
	if (get_gear(cn, IT_TW_MARCH)) gearSpec |= 4;
	if (get_gear(cn, IT_WP_GEMCUTTER)) gemCheck = 1;
	
	if (ch[cn].worn[WN_RHAND]==0 && ch[cn].worn[WN_LHAND]==0) critical_b += 1;
	
	if (IS_SEYAN_DU(cn) && !get_gear(cn, IT_SEYANSWORD))
	{
		bits           = get_seyan_bits(cn);
		
		for (z = 0; z<5; z++) 
			attrib[z] += max(0,min( 5, (bits-17)/3)  );
		
		hit_rate      += max(0,min( 5, (bits- 2)/6)  );
		parry_rate    += max(0,min( 5, (bits- 2)/6)  );
		
		critical_c    += max(0,min(40, (bits- 2)*4/3));
		critical_m    += max(0,min( 5, (bits- 2)/6)  );
		spell_mod     += max(0,min( 5, (bits- 2)/6)  );
	}
	
	if (has_item(cn, IT_COMMAND3))
	{
		labcmd = 1;
	}
	
	// Loop through gear slots
	for (n = 0; n<20; n++)
	{
		if (!ch[cn].worn[n]) continue;
		
		m = ch[cn].worn[n];

		// Stat bonuses are only awarded outside of no-magic zones
		if (!(ch[cn].flags & CF_NOMAGIC))
		{
			nightSpec = 0; gemSpec = 0;
			if ((it[m].temp==IT_BL_SOLAR || it[m].temp==IT_BL_LUNAR || it[m].temp==IT_BL_ECLIPSE) && 
				(globs->mdtime<MD_HOUR*6 || globs->mdtime>MD_HOUR*18)) // Night Time
			{
				nightSpec = 1;
			}
			if ((n == WN_LRING || n == WN_RRING) && gemCheck) // Gemcutter
			{
				gemSpec = 1;
			}
			
			// If the item is sparked/active/whatever
			if (it[m].active || nightSpec)
			{
				// Attributes
				for (z = 0; z<5; z++) attrib[z] += (it[m].attrib[z][1])*(gemSpec?7:5)/5;
				
				hp   += (it[m].hp[1])*(gemSpec?7:5)/5;
				end  += (it[m].end[1])*(gemSpec?7:5)/5;
				mana += (it[m].mana[1])*(gemSpec?7:5)/5;
				
				// Skills
				for (z = 0; z<MAXSKILL; z++) skill[z] += (it[m].skill[z][1])*(gemSpec?7:5)/5;
				
				// Meta values
				base_spd   += (((gearSpec&4)&&it[m].speed[1]<0)?it[m].speed[1]*2/3:it[m].speed[1])*(gemSpec?7:5)/5;
				spd_move   += (((gearSpec&4)&&it[m].move_speed[1]<0)?it[m].move_speed[1]*2/3:it[m].move_speed[1])*(gemSpec?7:5)/5;
				spd_attack += (((gearSpec&4)&&it[m].atk_speed[1]<0)?it[m].atk_speed[1]*2/3:it[m].atk_speed[1])*(gemSpec?7:5)/5;
				spd_cast   += (((gearSpec&4)&&it[m].cast_speed[1]<0)?it[m].cast_speed[1]*2/3:it[m].cast_speed[1])*(gemSpec?7:5)/5;
				spell_mod  += (it[m].spell_mod[1])*(gemSpec?7:5)/5;
				spell_apt  += (it[m].spell_apt[1])*(gemSpec?7:5)/5;
				spell_cool += (it[m].cool_bonus[1])*(gemSpec?7:5)/5;
				aoe += (it[m].aoe_bonus[1])*(gemSpec?7:5)/5;
			}
			// Otherwise, give normal stat boosts
			else
			{
				// Attributes
				for (z = 0; z<5; z++) attrib[z] += (it[m].attrib[z][0])*(gemSpec?7:5)/5;
				
				hp   += (it[m].hp[0])*(gemSpec?7:5)/5;
				end  += (it[m].end[0])*(gemSpec?7:5)/5;
				mana += (it[m].mana[0])*(gemSpec?7:5)/5;
				
				// Skills
				for (z = 0; z<MAXSKILL; z++) skill[z] += (it[m].skill[z][0])*(gemSpec?7:5)/5;
				
				// Meta values
				base_spd   += (((gearSpec&4)&&it[m].speed[0]<0)?it[m].speed[0]*2/3:it[m].speed[0])*(gemSpec?7:5)/5;
				spd_move   += (((gearSpec&4)&&it[m].move_speed[0]<0)?it[m].move_speed[0]*2/3:it[m].move_speed[0])*(gemSpec?7:5)/5;
				spd_attack += (((gearSpec&4)&&it[m].atk_speed[0]<0)?it[m].atk_speed[0]*2/3:it[m].atk_speed[0])*(gemSpec?7:5)/5;
				spd_cast   += (((gearSpec&4)&&it[m].cast_speed[0]<0)?it[m].cast_speed[0]*2/3:it[m].cast_speed[0])*(gemSpec?7:5)/5;
				spell_mod  += (it[m].spell_mod[0])*(gemSpec?7:5)/5;
				spell_apt  += (it[m].spell_apt[0])*(gemSpec?7:5)/5;
				spell_cool += (it[m].cool_bonus[0])*(gemSpec?7:5)/5;
				aoe += (it[m].aoe_bonus[0])*(gemSpec?7:5)/5;
			}
			
			if (it[m].temp==IT_CH_PREIST   || (IS_SINBINDER(m) && it[m].data[2]==IT_CH_PREIST)) 	charmSpec |=    1;
			if (it[m].temp==IT_CH_LOVERS   || (IS_SINBINDER(m) && it[m].data[2]==IT_CH_LOVERS)) 	charmSpec |=    2;
			if (it[m].temp==IT_CH_MAGI     || (IS_SINBINDER(m) && it[m].data[2]==IT_CH_MAGI))   	charmSpec |=    4;
			if (it[m].temp==IT_CH_HERMIT   || (IS_SINBINDER(m) && it[m].data[2]==IT_CH_HERMIT)) 	charmSpec |=    8;
			if (it[m].temp==IT_CH_FOOL_R   || (IS_SINBINDER(m) && it[m].data[2]==IT_CH_FOOL_R)) 	charmSpec |=   16;
			if (it[m].temp==IT_CH_STRENGTH || (IS_SINBINDER(m) && it[m].data[2]==IT_CH_STRENGTH))	charmSpec |=   32;
			if (it[m].temp==IT_CH_WHEEL    || (IS_SINBINDER(m) && it[m].data[2]==IT_CH_WHEEL))		charmSpec |=   64;
			if (it[m].temp==IT_CH_MAGI_R   || (IS_SINBINDER(m) && it[m].data[2]==IT_CH_MAGI_R))		charmSpec |=  128;
			if (it[m].temp==IT_CH_PREIST_R || (IS_SINBINDER(m) && it[m].data[2]==IT_CH_PREIST_R))	charmSpec |=  256;
			if (it[m].temp==IT_CH_LOVERS_R || (IS_SINBINDER(m) && it[m].data[2]==IT_CH_LOVERS_R))	charmSpec |= 1024;
			if (it[m].temp==IT_CH_STRENG_R || (IS_SINBINDER(m) && it[m].data[2]==IT_CH_STRENG_R))	charmSpec |= 4096;
			if (it[m].temp==IT_CH_HANGED_R || (IS_SINBINDER(m) && it[m].data[2]==IT_CH_HANGED_R))	charmSpec |=16384;
			if (it[m].temp==IT_CH_TEMPER_R || (IS_SINBINDER(m) && it[m].data[2]==IT_CH_TEMPER_R))	charmSpec |=32768;
			
			if (it[m].temp==IT_BOOK_TRAV 	|| it[m].orig_temp==IT_BOOK_TRAV) 		gearSpec  |=    1;
			if (it[m].temp==IT_TW_HEAVENS 	|| it[m].orig_temp==IT_TW_HEAVENS)		gearSpec  |=    2;
			if (it[m].temp==IT_WP_GILDSHINE || it[m].orig_temp==IT_WP_GILDSHINE)	gearSpec  |=    8;
			if (it[m].temp==IT_WP_KELPTRID 	|| it[m].orig_temp==IT_WP_KELPTRID)		gearSpec  |=   16;
			if (it[m].temp==IT_WP_WHITEODA 	|| it[m].orig_temp==IT_WP_WHITEODA)		gearSpec  |=   32;
			if (it[m].temp==IT_WP_BLACKTAC 	|| it[m].orig_temp==IT_WP_BLACKTAC)		gearSpec  |=   64;
			if (it[m].temp==IT_WP_EXCALIBUR || it[m].orig_temp==IT_WP_EXCALIBUR)	gearSpec  |=  128;
			if (it[m].temp==IT_TW_AVARITIA 	|| it[m].orig_temp==IT_TW_AVARITIA)		gearSpec  |=  256;
			if (it[m].temp==IT_BOOK_HOLY 	|| it[m].orig_temp==IT_BOOK_HOLY)		gearSpec  |=  512;
			if (it[m].temp==IT_BOOK_PROD 	|| it[m].orig_temp==IT_BOOK_PROD)		gearSpec  |= 1024;
			if (it[m].temp==IT_BONEARMOR 	|| it[m].orig_temp==IT_BONEARMOR)		gearSpec  |= 2048;
			
			if (it[m].enchantment==  1)		enchantSpec |=    1;
			if (it[m].enchantment==  5)   { moreBrv++; if (IS_TWOHAND(m)) moreBrv++; }
			if (it[m].enchantment== 11)		enchantSpec |=    2;
			if (it[m].enchantment== 12)   { moreWil++; if (IS_TWOHAND(m)) moreWil++; }
			if (it[m].enchantment== 17)		infra = 15;
			if (it[m].enchantment== 18)   { moreEnd++; if (IS_TWOHAND(m)) moreEnd++; }
			if (it[m].enchantment== 19)   { moreInt++; if (IS_TWOHAND(m)) moreInt++; }
			if (it[m].enchantment== 24)		enchantSpec |=   16;
			if (it[m].enchantment== 25)   { moreAgl++; if (IS_TWOHAND(m)) moreAgl++; }
			if (it[m].enchantment== 26)		enchantSpec |=   32;
			if (it[m].enchantment== 29)   { moreStr++; if (IS_TWOHAND(m)) moreStr++; }
			if (it[m].enchantment== 40)		enchantSpec |=   64;
			if (it[m].enchantment== 44)		enchantSpec |=  128;
			if (it[m].enchantment== 48)		enchantSpec |=  256;
			if (it[m].enchantment== 50)		moreDmg++;
			if (it[m].enchantment== 51)		moreGrd++;
			if (it[m].enchantment== 55)		enchantSpec |= 1024;
			if (it[m].enchantment== 54)   { moreAtt++; if (IS_TWOHAND(m)) moreAtt++; }
		}
		
		critical_b += it[m].base_crit*(gemSpec?7:5)/5;
		
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
		
		// Regular item bonuses
		if (it[m].active)
		{
			armor  += it[m].armor[1]*(gemSpec?7:5)/5;
			
			// Special case for Templars with Dual Swords...
			if ((it[m].flags & IF_OF_DUALSW) &&	IS_ANY_TEMP(cn))
			{
				weapon += it[m].weapon[1]/2*(gemSpec?7:5)/5;
			}
			else if (n == WN_LHAND && (it[m].flags & IF_WP_DAGGER)) // ...and daggers...
			{
				weapon += it[m].weapon[1]/2*(gemSpec?7:5)/5;
			}
			else if (n == WN_RHAND && (it[m].flags & IF_OF_SHIELD)) // Special case for AT dual shield memes
			{
				weapon += it[m].armor[1]*2*(gemSpec?7:5)/5;
			}
			else
			{
				weapon += it[m].weapon[1]*(gemSpec?7:5)/5;
			}
			
			gethit += it[m].gethit_dam[1]*(gemSpec?7:5)/5;
			
			if (!labcmd || it[m].temp==91)
			{
				maxlight += it[m].light[1]*(gemSpec?7:5)/5;
				if (it[m].light[1]>light)
				{
					light = it[m].light[1]*(gemSpec?7:5)/5;
				}
				else if (it[m].light[1]<0)
				{
					sublight -= it[m].light[1]*(gemSpec?7:5)/5;
				}
			}
			
			// Meta values
			critical_c += it[m].crit_chance[1]*(gemSpec?7:5)/5;
			critical_m += it[m].crit_multi[1]*(gemSpec?7:5)/5;
			hit_rate   += it[m].to_hit[1]*(gemSpec?7:5)/5;
			parry_rate += it[m].to_parry[1]*(gemSpec?7:5)/5;
			damage_top += it[m].top_damage[1]*(gemSpec?7:5)/5;
			dmg_bns     = dmg_bns * (100 + (bu[m].dmg_bonus[1]*(gemSpec?7:5)/5)/2)/100;
			dmg_rdc     = dmg_rdc * (100 - (bu[m].dmg_reduction[1]*(gemSpec?7:5)/5)/2)/100;
		}
		else
		{
			armor  += it[m].armor[0]*(gemSpec?7:5)/5;
			
			// Special case for Templars with Dual Swords...
			if ((it[m].flags & IF_OF_DUALSW) && IS_ANY_TEMP(cn) && it[m].temp != IT_WP_FELLNIGHT)
			{
				weapon += it[m].weapon[0]/2*(gemSpec?7:5)/5;
			}
			else if (n == WN_LHAND && (it[m].flags & IF_WP_DAGGER)) // ...and daggers...
			{
				weapon += it[m].weapon[0]/2*(gemSpec?7:5)/5;
			}
			else if (n == WN_RHAND && (it[m].flags & IF_OF_SHIELD)) // Special case for AT dual shield memes
			{
				weapon += it[m].armor[0]*2*(gemSpec?7:5)/5;
			}
			else
			{
				weapon += it[m].weapon[0]*(gemSpec?7:5)/5;
			}
			
			gethit += it[m].gethit_dam[0]*(gemSpec?7:5)/5;
			
			if (!labcmd || it[m].temp==91)
			{
				maxlight += it[m].light[0]*(gemSpec?7:5)/5;
				if (it[m].light[0]>light)
				{
					light = it[m].light[0]*(gemSpec?7:5)/5;
				}
				else if (it[m].light[0]<0)
				{
					sublight -= it[m].light[0]*(gemSpec?7:5)/5;
				}
			}
			
			// Meta values
			critical_c += it[m].crit_chance[0]*(gemSpec?7:5)/5;
			critical_m += it[m].crit_multi[0]*(gemSpec?7:5)/5;
			hit_rate   += it[m].to_hit[0]*(gemSpec?7:5)/5;
			parry_rate += it[m].to_parry[0]*(gemSpec?7:5)/5;
			damage_top += it[m].top_damage[0]*(gemSpec?7:5)/5;
			dmg_bns     = dmg_bns * (100 + (bu[m].dmg_bonus[0]*(gemSpec?7:5)/5)/2)/100;
			dmg_rdc     = dmg_rdc * (100 - (bu[m].dmg_reduction[0]*(gemSpec?7:5)/5)/2)/100;
		}
	}
	
	// GC may inherit tarots from owner
	if (IS_PLAYER_COMP(cn) && (co = CN_OWNER(cn)) && T_SUMM_SK(co, 10))
	{
		for (n = 0; n<20; n++)
		{
			if (!ch[co].worn[n]) continue;
			
			m = ch[co].worn[n];
			
			if (it[m].temp==IT_CH_PREIST   || (IS_SINBINDER(m) && it[m].data[2]==IT_CH_PREIST)) 	charmSpec |=    1;
			if (it[m].temp==IT_CH_LOVERS   || (IS_SINBINDER(m) && it[m].data[2]==IT_CH_LOVERS)) 	charmSpec |=    2;
			if (it[m].temp==IT_CH_MAGI     || (IS_SINBINDER(m) && it[m].data[2]==IT_CH_MAGI))   	charmSpec |=    4;
			if (it[m].temp==IT_CH_HERMIT   || (IS_SINBINDER(m) && it[m].data[2]==IT_CH_HERMIT)) 	charmSpec |=    8;
			if (it[m].temp==IT_CH_FOOL_R   || (IS_SINBINDER(m) && it[m].data[2]==IT_CH_FOOL_R)) 	charmSpec |=   16;
			if (it[m].temp==IT_CH_STRENGTH || (IS_SINBINDER(m) && it[m].data[2]==IT_CH_STRENGTH))	charmSpec |=   32;
			if (it[m].temp==IT_CH_WHEEL    || (IS_SINBINDER(m) && it[m].data[2]==IT_CH_WHEEL))		charmSpec |=   64;
			if (it[m].temp==IT_CH_MAGI_R   || (IS_SINBINDER(m) && it[m].data[2]==IT_CH_MAGI_R))		charmSpec |=  128;
			if (it[m].temp==IT_CH_PREIST_R || (IS_SINBINDER(m) && it[m].data[2]==IT_CH_PREIST_R))	charmSpec |=  256;
			if (it[m].temp==IT_CH_LOVERS_R || (IS_SINBINDER(m) && it[m].data[2]==IT_CH_LOVERS_R))	charmSpec |= 1024;
			if (it[m].temp==IT_CH_STRENG_R || (IS_SINBINDER(m) && it[m].data[2]==IT_CH_STRENG_R))	charmSpec |= 4096;
			if (it[m].temp==IT_CH_HANGED_R || (IS_SINBINDER(m) && it[m].data[2]==IT_CH_HANGED_R))	charmSpec |=16384;
			if (it[m].temp==IT_CH_TEMPER_R || (IS_SINBINDER(m) && it[m].data[2]==IT_CH_TEMPER_R))	charmSpec |=32768;
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
	
	if (IS_PLAYER_COMP(cn) && (co = CN_OWNER(cn)))
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
		spell_cool += bu[m].cool_bonus[1];
		if (bu[m].temp==SK_ARIA || bu[m].temp==SK_ARIA2) spell_cool += bu[m].cool_bonus[1];
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
			
			if (charmSpec & 32768) sickStacks++;
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
		
		if (bu[m].temp==SK_TAUNT)
		{
			if (T_ARTM_SK(bu[m].data[0], 9)) attaunt=1;
			ch[cn].taunted = bu[m].data[0];
			if (ch[cn].temp==CT_PANDIUM) ch[cn].taunted = 0; // Special case for Pandium to ignore persistant aggro
			
			if (bu[m].active >= bu[m].duration-1) // Fresh taunt
			{
				ch[cn].attack_cn = bu[m].data[0];
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
	if (T_SEYA_SK(cn,  1)) weapon += 2;
	if (T_SEYA_SK(cn,  2)) { for (z = 0; z<5; z++) attrib[z] += 2; }
	if (T_SEYA_SK(cn,  3)) armor += 2;
	if (T_SEYA_SK(cn,  4)) dmg_bns = dmg_bns * (1000 + bcount*5)/1000;
	if (T_SEYA_SK(cn,  8)) moreAtt++;
	if (T_SEYA_SK(cn, 12)) dmg_rdc = dmg_rdc * (1000 - bcount*5)/1000;
	//
	if (T_ARTM_SK(cn,  1)) gethit += 5;
	if (T_ARTM_SK(cn,  2)) attrib[AT_STR] += 4;
	if (T_ARTM_SK(cn,  3)) armor += 3;
	//
	if (T_SKAL_SK(cn,  1)) damage_top += 5;
	if (T_SKAL_SK(cn,  2)) attrib[AT_AGL] += 4;
	if (T_SKAL_SK(cn,  3)) end += 20;
	//
	if (T_WARR_SK(cn,  1)) spd_attack += 5;
	if (T_WARR_SK(cn,  2)) { attrib[AT_AGL] += 3; attrib[AT_STR] += 3; }
	if (T_WARR_SK(cn,  3)) spell_apt += 5;
	if (T_WARR_SK(cn,  6)) spell_mod += 2;
	if (T_WARR_SK(cn,  8)) { moreAgl++; moreStr++; }
	//
	if (T_SORC_SK(cn,  1)) aoe++;
	if (T_SORC_SK(cn,  2)) { attrib[AT_WIL] += 3; attrib[AT_INT] += 3; }
	if (T_SORC_SK(cn,  3)) spd_move += 5;
	if (T_SORC_SK(cn,  6)) spell_mod += 2;
	if (T_SORC_SK(cn,  8)) { moreWil++; moreInt++; }
	//
	if (T_SUMM_SK(cn,  1)) spd_cast += 5;
	if (T_SUMM_SK(cn,  2)) attrib[AT_WIL] += 4;
	if (T_SUMM_SK(cn,  3)) hp += 20;
	//
	if (T_ARHR_SK(cn,  1)) spell_cool += 5;
	if (T_ARHR_SK(cn,  2)) attrib[AT_INT] += 4;
	if (T_ARHR_SK(cn,  3)) mana += 20;
	//
	if (T_BRAV_SK(cn,  1)) hit_rate += 3;
	if (T_BRAV_SK(cn,  2)) attrib[AT_BRV] += 4;
	if (T_BRAV_SK(cn,  3)) parry_rate += 3;
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
		ch[cn].limit_break[z][1] += min(5,max(0,(bits+5-z)/6));
		
		// Enchant: More attributes
		if (z==0 && moreBrv) attrib[z] = attrib[z]*(100+moreBrv*3)/100;
		if (z==1 && moreWil) attrib[z] = attrib[z]*(100+moreWil*3)/100;
		if (z==2 && moreInt) attrib[z] = attrib[z]*(100+moreInt*3)/100;
		if (z==3 && moreAgl) attrib[z] = attrib[z]*(100+moreAgl*3)/100;
		if (z==4 && moreStr) attrib[z] = attrib[z]*(100+moreStr*3)/100;
		if (moreAtt) attrib[z] = attrib[z]*(100+moreAtt*2)/100;
		
		set_attrib_score(cn, z, attrib[z]);
	}
	ch[cn].limit_break[5][1]  = max(-127, min(127, suppression));
	ch[cn].limit_break[5][1] += min(5,max(0,bits/6));
	
	// Weapon - Fist of the Heavens :: Doubles best attribute
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
	
	// Endurance
	end = (int)ch[cn].end[0] + (int)ch[cn].end[1] + end;
	if (moreEnd) end = end*(5+moreEnd)/5;
	if (T_SKAL_SK(cn, 11)) end = end*120/100;
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
	if (ch[cn].a_end < ch[cn].end[5]) ch[cn].a_end += heal_end;
	
	// Mana
	mana = (int)ch[cn].mana[0] + (int)ch[cn].mana[1] + mana;
	if (T_ARHR_SK(cn, 11)) mana = mana*120/100;
	if (mana<10)
	{
		mana = 10;
	}
	ch[cn].mana[4] = mana;
	// Tarot - Priestess.R : HP/Mana limits reduced, overcap inverted, damage increased
	if (charmSpec & 256)
	{
		if (mana>800 && IS_PLAYER(cn))
		{
			priestess += (mana-800);
			tempCost = tempCost * (2000+priestess)/2000; // 1% per 20
			if (T_ARHR_SK(cn, 10)) hp += (mana-800)/2;
			mana = 800;
		}
	}
	else
	{
		if (mana>999 && IS_PLAYER(cn))
		{
			reduc_bonus = 1000000/max(1, mana*1000/ 999);
			tempCost = tempCost * reduc_bonus/1000;
			if (T_ARHR_SK(cn, 10)) hp += (mana-999)/2;
			mana = 999;
		}
	}
	ch[cn].mana[5] = mana;
	if (ch[cn].a_mana < ch[cn].mana[5]) ch[cn].a_mana += heal_mana;
	
	// Hitpoints - Placed after Mana due to overcap from mana (tree)
	hp = (int)ch[cn].hp[0] + (int)ch[cn].hp[1] + hp;
	if (pigsblood & 1) hp = hp*115/100; // Pigs blood drink
	if (charmSpec & 1) hp = hp*90/100; // Tarot - High Priestess
	if (T_SUMM_SK(cn, 11)) hp = hp*120/100;
	if (hp<10)
	{
		hp = 10;
	}
	ch[cn].hp[4] = hp;
	// Tarot - Priestess.R : HP/Mana limits reduced, overcap inverted, damage increased
	if (charmSpec & 256)
	{
		if (hp>800 && IS_PLAYER(cn))
		{
			priestess += (hp-800);
			dmg_rdc = dmg_rdc * (5000 + priestess)/5000; // 1% per 50
			hp = 800;
		}
	}
	else
	{
		if (hp>999 && IS_PLAYER(cn))
		{
			reduc_bonus = 1000000/max(1, hp*1000/ 999);
			dmg_rdc = dmg_rdc * reduc_bonus/1000;
			hp = 999;
		}
	}
	ch[cn].hp[5] = hp;
	if (ch[cn].a_hp < ch[cn].hp[5]) ch[cn].a_hp += heal_hp;
	

	if (ch[cn].flags & (CF_PLAYER))
	{
		if (infra==15 && !(ch[cn].flags & CF_INFRARED))
		{
			ch[cn].flags |= CF_INFRARED;
			ch[cn].flags |= CF_IGN_SB;
			do_char_log(cn, 0, "You can see in the dark!\n");
		}
		if (infra!=15 && (ch[cn].flags & CF_INFRARED) && !(ch[cn].flags & CF_GOD))
		{
			ch[cn].flags &= ~CF_INFRARED;
			ch[cn].flags &= ~CF_IGN_SB;
			do_char_log(cn, 0, "You can no longer see in the dark!\n");
		}
	}

	for (z = 0; z<MAXSKILL; z++)
	{
		skill[z] = (int)B_SK(cn, z) + (int)ch[cn].skill[z][1] + skill[z];
		
		if ((z==0||z==2||z==3||z==4||z==5||z==6||z==16||z==33||z==38||z==40||z==41||z==48||z==49)
			&& T_SKAL_SK(cn, 9))
			skill[z] += ((int)((M_AT(cn, AT_BRV)+M_AT(cn, AT_STR))/2)+(int)M_AT(cn, AT_AGL)+(int)M_AT(cn, AT_AGL))/5;
		else if ((z==0||z==2||z==3||z==4||z==5||z==6)
			&& T_BRAV_SK(cn, 9))
			skill[z] += ((int)((M_AT(cn, AT_AGL)+M_AT(cn, AT_STR))/2)+(int)M_AT(cn, AT_BRV)+(int)M_AT(cn, AT_BRV))/5;
		else if ((z==11||z==17||z==18||z==19||z==20||z==21||z==22||z==24||z==25||z==26||z==27||z==42||z==43||z==46||z==47)
			&& T_SUMM_SK(cn, 7))
			skill[z] += ((int)((M_AT(cn, AT_BRV)+M_AT(cn, AT_INT))/2)+(int)M_AT(cn, AT_WIL)+(int)M_AT(cn, AT_WIL))/5;
		else if ((z==11||z==17||z==18||z==19||z==20||z==21||z==22||z==24||z==25||z==26||z==27||z==42||z==43||z==46||z==47)
			&& T_ARHR_SK(cn, 9))
			skill[z] += ((int)((M_AT(cn, AT_BRV)+M_AT(cn, AT_WIL))/2)+(int)M_AT(cn, AT_INT)+(int)M_AT(cn, AT_INT))/5;
		else
			skill[z] += ((int)M_AT(cn, skilltab[z].attrib[0]) +
						 (int)M_AT(cn, skilltab[z].attrib[1]) +
						 (int)M_AT(cn, skilltab[z].attrib[2])) / 5;
		
		if (z==SK_IMMUN && T_ARTM_SK(cn, 10))
		{
			skill[z] += skill[SK_RESIST]/5;
		}
		
		if ((charmSpec & 8) && (z==SK_RESIST||z==SK_IMMUN))
		{
			skill[z] = skill[z]*4/5;
		}
		
		set_skill_score(cn, z, skill[z]);
		
		if (z==0 && bbelt)
		{
			weapon     += min(AT_CAP, skill[z])/2;
			tempWeapon += min(AT_CAP, skill[z])/2;
		}
		
		if (z==0 && wbelt)
		{
			weapon     += min(AT_CAP, skill[z])*2/5;
			tempWeapon += min(AT_CAP, skill[z])*2/5;
		}
	}
	
	// Gear Mastery
	if (B_SK(cn, SK_GEARMAST))
	{
		if (tempWeapon)
		{
			if (IS_PLAYER(cn) && IS_SKALD(cn))
				weapon += min(tempWeapon*3/2, M_SK(cn, SK_GEARMAST)/10*3);
			else if (IS_PLAYER(cn) && (IS_ANY_TEMP(cn) || IS_BRAVER(cn)))
				weapon += min(tempWeapon*6/5, M_SK(cn, SK_GEARMAST)/5);
			else if (IS_PLAYER(cn) && IS_WARRIOR(cn))
				weapon += min(tempWeapon*3/4, M_SK(cn, SK_GEARMAST)/10);
			else
				weapon += min(tempWeapon, M_SK(cn, SK_GEARMAST)/20*3);
		}
		if (tempArmor)
		{
			if (IS_PLAYER(cn) && IS_ARCHTEMPLAR(cn))
				armor += min(tempArmor*3/2, M_SK(cn, SK_GEARMAST)/10*3);
			else if (IS_PLAYER(cn) && (IS_ANY_TEMP(cn) || IS_BRAVER(cn)))
				armor += min(tempArmor*6/5, M_SK(cn, SK_GEARMAST)/5);
			else if (IS_PLAYER(cn) && IS_WARRIOR(cn))
				armor += min(tempArmor*3/4, M_SK(cn, SK_GEARMAST)/10);
			else
				armor += min(tempArmor, M_SK(cn, SK_GEARMAST)/20*3);
		}
	}
	
	// Maxlight takes your cumulative total of all light sources, minus the highest.
	// Light is then the highest single light value affecting you, plus half of whatever maxlight is beyond that.
	maxlight -= light;
	light += maxlight/2;

	light -= sublight;
	if (has_buff(cn, 215))
	{
		light = light/10;
	}
	if ((enchantSpec & 1024))
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
	
	/*
		ch[].mana_cost
	*/
	
	// Concentrate
	if (B_SK(cn, SK_CONCEN) && !(charmSpec & 128))
	{
		if (gearSpec & 1024) // Book: Great Prodigy
		{
			t = tempCost * M_SK(cn, SK_CONCEN) / 333;
		}
		else
		{
			t = tempCost * M_SK(cn, SK_CONCEN) / 444;
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
	if (T_SORC_SK(cn, 11)) spd_move = ((base_spd + spd_move) * 120/100) - base_spd;
	
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
	if (T_SUMM_SK(cn,  5)) 
	{
		spd_cast = ((base_spd + spd_cast) * 110/100) - spd_cast;
		if (T_SUMM_SK(cn,  4)) spd_attack = ((base_spd + spd_attack) * 110/100) - base_spd;
	}
	
	// Tarot - Strength - 12% less cast speed
	if (charmSpec & 32)
	{
		spd_cast = (base_spd+spd_cast) * 88/100 - base_spd;
		if (T_SUMM_SK(cn,  4)) spd_attack = ((base_spd + spd_attack) * 88/100) - base_spd;
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
	
	// Tarot - Strength - 12% less attack speed
	if (charmSpec & 32)
	{
		spd_attack = ((base_spd + spd_attack) * 88/100) - base_spd;
	}
	
	// Tree - warr
	if (T_WARR_SK(cn,  5)) spd_attack = ((base_spd + spd_attack) * 110/100) - base_spd;
	
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
	
	// Tree - warr
	if (T_WARR_SK(cn, 11)) spell_apt = spell_apt*120/100;
	if (T_WARR_SK(cn, 12)) dmg_rdc = dmg_rdc * (10000 - spell_apt*5)/10000;
	
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
	
	// Tarot - Magician.R : Concentrate instead improves cooldown rate
	if (B_SK(cn, SK_CONCEN) && (charmSpec & 128))
	{
		if (gearSpec & 1024) // Book: Great Prodigy
		{
			t = spell_cool * M_SK(cn, SK_CONCEN) / 333;
		}
		else
		{
			t = spell_cool * M_SK(cn, SK_CONCEN) / 444;
		}
		spell_cool += t;
	}
	
	// Tree - arhr
	if (T_ARHR_SK(cn,  5)) spell_cool = spell_cool*110/100;
	
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
	
	// Monster bonus
	if (ch[cn].kindred & KIN_MONSTER)
		dmg_bns = dmg_bns * (100 + (getrank(cn)-4)/2)/100;
	
	// Tarot - Priestess.R : HP/Mana limits reduced, overcap inverted, damage increased
	if (charmSpec & 256)
	{
		dmg_bns = dmg_bns * (10000 + priestess)/10000; // 1% per 100
	}
	
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
	if (ch[cn].kindred & KIN_MONSTER)
		dmg_rdc = dmg_rdc * (100 - (getrank(cn)-4)/2)/100;
	
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
		critical_b += (critical_b * skill[SK_PRECISION] * (T_BRAV_SK(cn,4)?115:100)/100)/PREC_CAP;
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
	
	
	// Weapon - Gildshine :: Bartering is granted as crit multi
	if (gearSpec & 8) 
	{
		critical_m += skill[SK_BARTER]*2;
	}
	
	// Tree - skal
	if (T_SKAL_SK(cn,  7)) critical_m += attrib_ex[AT_AGL]/4;
	
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
	if (T_BRAV_SK(cn,  5)) hit_rate   = hit_rate  *104/100;
	if (T_BRAV_SK(cn, 11)) parry_rate = parry_rate*104/100;
	
	// Spear power-up (summ tree)
	if ((m = ch[cn].worn[WN_RHAND]) && (it[m].flags & IF_WP_DAGGER) && (it[m].flags & IF_WP_STAFF) && T_SUMM_SK(cn,  6))
	{
		hit_rate   = hit_rate  *6/5;
		parry_rate = parry_rate*6/5;
	}
	
	// GC parent override (seya tree)
	if (IS_PLAYER_COMP(cn) && (m = CN_OWNER(cn)) && T_SEYA_SK(m, 9))
	{
		hit_rate   = ch[m].to_hit;
		parry_rate = ch[m].to_parry;
	}
	
	if (attaunt)
		hit_rate = hit_rate * 19/20;
	
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
	
	// Weapon - Excalibur :: Additional WV from 12% of total attack speed
	if (gearSpec & 128)
	{
		weapon += (base_spd + spd_attack)*12/100;
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
	if (charmSpec & 16384)
		weapon = weapon * 22/25;
	// Tarot - Temperance.R : 6.25% more WV per stack of healing sickness on you
	if (charmSpec & 32768)
		weapon += weapon * sickStacks/16;
	// Chest Armor Enchantment
	if (enchantSpec & 1) 
		armor = armor * 27/25;
	// Tarot - Reverse Heirophant : GC gets more WV/AV
	if (IS_COMPANION(cn) && !(ch[cn].flags & CF_SHADOWCOPY) && IS_SANECHAR(co = ch[cn].data[CHD_MASTER]) && get_tarot(co, IT_CH_HEIROP_R))
	{
		armor  = armor  * 112/100;
		weapon = weapon * 112/100;
	}
	// Tree - % more wv/av
	if (T_SEYA_SK(cn,  5)) weapon = weapon * 106 / 100;
	if (T_SEYA_SK(cn, 11)) armor = armor * 106 / 100;
	if (T_ARTM_SK(cn, 11)) armor = armor * 109 / 100;
	//
	
	if (ch[cn].temp == CT_PIRATELORD && IS_SANEITEM(m = map[1331331].it) && !(it[in].active))
		armor += 100;
	
	if (armor<0)
	{
		armor = 0;
	}
	if (armor>300)
	{
		armor = 300;
	}
	if (IS_PLAYER(cn) && (gearSpec & 2048)) // Bone Armor version 2
	{
		ch[cn].data[25] = armor;
		armor = 0;
	}
	ch[cn].armor = armor;
	
	// Enchant: AV as extra Resistance
	if (enchantSpec &  2) 
		set_skill_score(cn, SK_RESIST, skill[SK_RESIST] + armor/10);
	// Enchant: AV as extra Immunity
	if (enchantSpec & 16) 
		set_skill_score(cn, SK_IMMUN, skill[SK_IMMUN] + armor/10);
	
	if (weapon<0)
	{
		weapon = 0;
	}
	if (weapon>300)
	{
		weapon = 300;
	}
	ch[cn].weapon = weapon;
	
	// Enchant: 50% more Thorns
	if (enchantSpec & 256) 
		gethit += gethit/2;
	
	// Tree: artm
	if (T_ARTM_SK(cn,  5)) gethit = gethit*120/100;
	
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
	
	if (aoe<-8)
	{
		aoe = -8;
	}
	if (aoe>8)
	{
		aoe = 8;
	}
	ch[cn].aoe_bonus = aoe;
	
	/*
		ch[].top_damage value
		
		Determined by STR/2. This is put into a RANDOM(), so "average damage" can be considered WV plus half of this number
	*/
	
	damage_top = damage_top + attrib_ex[AT_STR]/2;
	
	// Tarot - Hanged.R : 12% less WV, 24% more Top Damage
	if (charmSpec & 16384)
		damage_top = damage_top * 31/25;
	
	// Tree - skal
	if (T_SKAL_SK(cn,  5)) damage_top = damage_top*120/100;
	
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

void do_aria(int cn)
{
	int _aoe, _rad, x, y, xf, yf, xt, yt, xc, yc, aoe_power, in2, power, co;
	double tmp_a;
	
	if (IS_SKALD(cn))
	{
		power = M_SK(cn, SK_ARIA)*(T_SKAL_SK(cn,4)?120:100)/100;
	}
	else // Braver
	{
		power = M_SK(cn, SK_ARIA)/4;
	}
	
	aoe_power = M_SK(cn, SK_PROX)+15;
	_rad      = PRXA_RAD + ch[cn].aoe_bonus;
	_aoe      = aoe_power/(PROX_CAP*2) + _rad;
	tmp_a     = (double)(aoe_power*100/(PROX_CAP*2) + _rad*100);
	
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
		if (IS_LIVINGCHAR(co = map[x + y * MAPX].ch) && do_char_can_see(cn, co))
		{
			in2 = 0;
			if ((cn!=co) && do_surround_check(cn, co, 1)) 
			{
				aoe_power = spell_immunity(power, get_target_immunity(cn, co));
				// debuff version
				if (!(in2 = make_new_buff(cn, SK_ARIA2, BUF_SPR_ARIA2, aoe_power, SP_DUR_ARIA, 0))) 
					continue;
				
				bu[in2].cool_bonus[1] = max(-127, -(aoe_power/4 + 1));
				bu[in2].data[4] = 1; // Effects not removed by NMZ (SK_ARIA2)
			}
			else
			{
				// buff version
				if (!(in2 = make_new_buff(cn, SK_ARIA, BUF_SPR_ARIA, power, SP_DUR_ARIA, 0))) 
					continue;
				
				if (IS_SKALD(co)) bu[in2].dmg_bonus[1] = min(127, power/15);
				
				bu[in2].cool_bonus[1] = min(127, power/4 + 1);
				bu[in2].data[4] = 1; // Effects not removed by NMZ (SK_ARIA)
			}
			if (co && in2) add_spell(co, in2);
		}
	}
}

void do_immolate(int cn, int in)
{
	int _aoe, _rad, x, y, xf, yf, xt, yt, xc, yc, aoe_power, in2 = 0, power, co, idx, nn;
	double tmp_a, tmp_s, tmp_h;
	
	tmp_s     = (double)(bu[in].power*2/(IS_PLAYER(cn)?2:3));
	aoe_power = M_SK(cn, SK_PROX)+15;
	_rad      = bu[in].data[3];
	_aoe      = aoe_power/(PROX_CAP*2) + _rad;
	tmp_a     = (double)(aoe_power*100/(PROX_CAP*2) + _rad*100);
	tmp_h     = (double)(sqr(aoe_power*100/PROX_HIT-_aoe)/500+(_rad*300));
	
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
		if (IS_LIVINGCHAR(co = map[x + y * MAPX].ch) && do_char_can_see(cn, co) && cn!=co)
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

void do_update_permaspells(int cn)
{
	int n, in, power;
	
	for (n = 0; n<MAXBUFFS; n++)
	{
		if ((in = ch[cn].spell[n]) && (bu[in].flags & IF_PERMSPELL))
		{
			switch (bu[in].temp)
			{
				case SK_RAGE:
					power = M_SK(cn, SK_RAGE);
					power = skill_multiplier(power, cn);
					bu[in].power = power;
					if (bu[in].top_damage[1])	bu[in].top_damage[1] = power/3 + 1;
					else						bu[in].weapon[1]     = power/6 + 2;
					break;
				case SK_LETHARGY:
					power = M_SK(cn, SK_LETHARGY);
					power = spell_multiplier(power, cn);
					bu[in].power = power;
					break;
				case SK_IMMOLATE:
					if (bu[in].data[0]==0)
					{
						power = ch[cn].hp[5]*3/10;
						if (get_book(cn, IT_BOOK_BURN)) power = power + ch[cn].hp[5]/25;
					}
					else
					{
						power = M_SK(bu[in].data[0], SK_PULSE);
						if (get_book(cn, IT_BOOK_BURN)) power = power + ch[cn].hp[5]/25;
						power = spell_multiplier(power, cn);
					}
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
	int n, m, p, in, in2, nohp = 0, noend = 0, nomana = 0, halfhp = 0, halfend = 0, halfmana = 0, old, mf;
	int hp = 0, end = 0, mana = 0, uwater = 0, gothp = 0;
	int race_reg = 0, race_res = 0, race_med = 0;
	int degenpower = 0, tickcheck = 8000;
	int moonmult = 20;
	int hpmult, endmult, manamult, rank=0;
	int co = -1;
	int tmp = 0, kill_bsp = 0, kill_osp = 0;
	int moonR = 0, sunR = 0, worldR = 0;
	int scorched = 0, guarded = 0, devRn = 0, devRo = 0, has_sld = 0, has_shl = 0, phalanx = 0, aggravate = 0;
	int offpot = 0, defpot = 0;
	int idle = 3;
	char buf[50];
	
	strcpy(buf, ch[cn].reference); buf[0] = toupper(buf[0]);

	// gothp determines how much to counter degeneration effects while underwater.
	m = ch[cn].x + ch[cn].y * MAPX;
	mf = map[m].flags;
	
	if (ch[cn].flags & CF_STONED)
		return;

	prof = prof_start();

	if ((ch[cn].flags & (CF_PLAYER)) || (IS_COMP_TEMP(cn) && (co = ch[cn].data[CHD_MASTER]) && IS_SANEPLAYER(co)))
	{
		if (IS_GLOB_MAYHEM)				moonmult = 10;
		if (globs->fullmoon)			moonmult = 30;
		if (globs->newmoon)				moonmult = 40;
		
		race_reg = M_SK(cn, SK_REGEN) * moonmult / 20 + (B_SK(cn, SK_REGEN)?M_SK(cn, SK_REGEN):0) * ch[cn].hp[5]   / 2000;
		race_res = M_SK(cn, SK_REST)  * moonmult / 20 + (B_SK(cn, SK_REST)?M_SK(cn, SK_REST):0  ) * ch[cn].end[5]  / 1000;
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

	if (mf & MF_UWATER) 		uwater = 1;
	
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
	
	if (get_tarot(cn, IT_CH_MOON_R))  moonR  = 1;
	if (get_tarot(cn, IT_CH_SUN_R))   sunR 	 = 1;
	if (get_tarot(cn, IT_CH_WORLD_R)) worldR = 1;
	
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
					ch[cn].a_hp 			+= (nohp   ? 0 : (hp      * (sunR  ?0:1) * idle/3) / (halfhp   ? 2 : 1)); 
					gothp 					+= (nohp   ? 0 : (hp  / 2 * (sunR  ?0:1) * idle/3) / (halfhp   ? 2 : 1));
					ch[cn].a_end 			+= (noend  ? 0 : (end     * (worldR?0:1) * idle/3) / (halfend  ? 2 : 1));
					ch[cn].a_mana 			+= (nomana ? 0 : (mana    * (moonR ?0:1) * idle/3) / (halfmana ? 2 : 1)); 
				break;
			
			// WALKING STATES
			case  16: case  24: case  32: case  40: case  48: case  60: case  72: case  84:
			case  96: case 100: case 104: case 108: case 112: case 116: case 120: case 124:
			case 128: case 132: case 136: case 140: case 144: case 148: case 152:
				if (get_enchantment(cn, 32))
				{
					ch[cn].a_hp 			+= (nohp   ? 0 : (hp     ) / (halfhp   ? 2 : 1)); 
					gothp 					+= (nohp   ? 0 : (hp  / 2) / (halfhp   ? 2 : 1));
					if (ch[cn].mode==2) // Fast
					{
						ch[cn].a_end 		+= (noend  ? 0 : (end    ) / (halfend  ? 2 : 1)) - 40;
					}
					else
					{
						ch[cn].a_end 		+= (noend  ? 0 : (end    ) / (halfend  ? 2 : 1));
					}
					ch[cn].a_mana 			+= (nomana ? 0 : (mana   ) / (halfmana ? 2 : 1)); 
				}
				else
				{
					ch[cn].a_hp 			+= (nohp   ? 0 : (hp  / 4) / (halfhp   ? 2 : 1)); 
					gothp 					+= (nohp   ? 0 : (hp  / 8) / (halfhp   ? 2 : 1));
					if (ch[cn].mode==2) // Fast
					{
						ch[cn].a_end 		+= (noend  ? 0 : (end / 4) / (halfend  ? 2 : 1)) - 40;
					}
					if (ch[cn].mode==1) // Normal
					{
						ch[cn].a_end 		+= (noend  ? 0 : (end / 4) / (halfend  ? 2 : 1));
					}
					if (ch[cn].mode==0) // Slow
					{
						ch[cn].a_end 		+= (noend  ? 0 : (end / 2) / (halfend  ? 2 : 1));
					}
					ch[cn].a_mana 			+= (nomana ? 0 : (mana/ 4) / (halfmana ? 2 : 1));
				}
				break;
			
			// FIGHTING STATES
			case 160: case 168: case 176: case 184:
				ch[cn].a_hp 			+= (nohp   ? 0 : (hp  / 8 * (sunR  ?8:1)) / (halfhp   ? 2 : 1)); 
				gothp 					+= (nohp   ? 0 : (hp  /16 * (sunR  ?8:1)) / (halfhp   ? 2 : 1));
				if (ch[cn].status2==0 || ch[cn].status2==5 || ch[cn].status2==6) // Attacking
				{
					if (ch[cn].mode==2) // Fast
					{
						ch[cn].a_end 	+= (noend  ? 0 : (end / 8 * (worldR?8:1)) / (halfend  ? 2 : 1)) - 75;
					}
					if (ch[cn].mode==1) // Normal
					{
						ch[cn].a_end 	+= (noend  ? 0 : (end / 8 * (worldR?8:1)) / (halfend  ? 2 : 1)) - 25;
					}
					if (ch[cn].mode==0) // Slow
					{
						ch[cn].a_end 	+= (noend  ? 0 : (end / 4 * (worldR?4:1)) / (halfend  ? 2 : 1));
					}
				}
				else // Misc.
				{
					if (ch[cn].mode==2)		// Fast
					{
						ch[cn].a_end 	+= (noend  ? 0 : (end / 4 * (worldR?4:1)) / (halfend  ? 2 : 1)) - 50;
					}
					if (ch[cn].mode==1)		// Normal
					{
						ch[cn].a_end 	+= (noend  ? 0 : (end / 4 * (worldR?4:1)) / (halfend  ? 2 : 1));
					}
					if (ch[cn].mode==0)		// Slow
					{
						ch[cn].a_end 	+= (noend  ? 0 : (end / 2 * (worldR?2:1)) / (halfend  ? 2 : 1));
					}
				}
				ch[cn].a_mana 			+= (nomana ? 0 : (mana/ 8 * (moonR ?8:1)) / (halfmana ? 2 : 1));
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
		gothp 		+= hp/4+hp/2;
	}
	if (IS_PLAYER(cn) && ch[cn].data[25])
	{
		hp = ch[cn].data[25] * 25 / 2;
		
		ch[cn].a_hp	+= hp / (halfhp   ? 2 : 1);
		gothp 		+= hp/2;
	}
	if (ch[cn].temp == CT_PIRATELORD && IS_SANEITEM(in = map[1331331].it) && !(it[in].active))
	{
		ch[cn].a_hp	+= 10000;
		gothp 		+= 1;
	}
	
	// Special case for the Amulet of Ankhs
	if (in = ch[cn].worn[WN_NECK])
	{
		switch (it[in].temp)
		{
			case IT_ANKHAMULET: 
				ch[cn].a_hp   += (race_reg/12) / (halfhp   ? 2 : 1);
				ch[cn].a_end  += (race_res/12) / (halfend  ? 2 : 1);
				ch[cn].a_mana += (race_med/12) / (halfmana ? 2 : 1);
				break;
			case IT_AMBERANKH: 
				ch[cn].a_hp   += (race_reg/ 6) / (halfhp   ? 2 : 1);
				ch[cn].a_end  += (race_res/24) / (halfend  ? 2 : 1);
				ch[cn].a_mana += (race_med/24) / (halfmana ? 2 : 1);
				break;
			case IT_TURQUANKH: 
				ch[cn].a_hp   += (race_reg/24) / (halfhp   ? 2 : 1);
				ch[cn].a_end  += (race_res/ 6) / (halfend  ? 2 : 1);
				ch[cn].a_mana += (race_med/24) / (halfmana ? 2 : 1);
				break;
			case IT_GARNEANKH: 
				ch[cn].a_hp   += (race_reg/24) / (halfhp   ? 2 : 1);
				ch[cn].a_end  += (race_res/24) / (halfend  ? 2 : 1);
				ch[cn].a_mana += (race_med/ 6) / (halfmana ? 2 : 1);
				break;
			case IT_TRUEANKH: 
				ch[cn].a_hp   += (race_reg/ 6) / (halfhp   ? 2 : 1);
				ch[cn].a_end  += (race_res/ 6) / (halfend  ? 2 : 1);
				ch[cn].a_mana += (race_med/ 6) / (halfmana ? 2 : 1);
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

	if ((hp && ch[cn].a_hp<ch[cn].hp[5] * 900) || (mana && ch[cn].a_mana<ch[cn].mana[5] * 900))
	{
		ch[cn].data[92] = TICKS * 60;
	}

	if (ch[cn].a_end<1500 && ch[cn].mode!=0)
	{
		ch[cn].mode = 0;
		do_update_char(cn);
		do_char_log(cn, 0, "You're exhausted.\n");
	}
	
	if (B_SK(cn, SK_ARIA)) do_aria(cn);
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
				if (bu[in].temp==206 && !IS_IN_SUN(ch[cn].x, ch[cn].y))
					bu[in].active--;
				
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
					hp = map[m].light;
					if (hp<1)	hp = 0;
					if (hp>30)	hp = 30;
					ch[cn].a_hp -= (30 - hp)*(30 - hp)*3;
				}
				
				if (bu[in].temp==SK_OPPRESSED2 && !IS_IN_ABYSS(ch[cn].x, ch[cn].y))
				{
					do_char_log(cn, 1, "The pressure on you was lifted.\n");
					bu[in].used = USE_EMPTY;
					ch[cn].spell[n] = 0;
					do_update_char(cn);
					continue;
				}
				
				if (bu[in].hp[0]!=-1)
				{
					ch[cn].a_hp += bu[in].hp[0];
				}
				if (bu[in].end[0]!=-1)
				{
					ch[cn].a_end += bu[in].end[0];
				}
				if (bu[in].mana[0]!=-1)
				{
					ch[cn].a_mana += bu[in].mana[0];
				}
				if (ch[cn].a_hp<1000 && bu[in].hp[0] && (bu[in].temp==SK_LETHARGY || bu[in].temp==SK_IMMOLATE))
				{
					ch[cn].a_hp = 1000;
					bu[in].active = 0;
					chlog(cn, "%s ran out due to lack of hitpoints.", it[in].name);
				}
				if (ch[cn].a_hp<500 && (bu[in].hp[0] < -1 || bu[in].temp==215))
				{
					if (ch[cn].flags & CF_IMMORTAL)
					{
						ch[cn].a_hp=500;
					}
					else
					{
						if (!(mf & MF_ARENA) && try_lucksave(cn))
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
				if (ch[cn].a_end<500 && bu[in].end[0] < -1)
				{
					ch[cn].a_end  = 500;
					if (bu[in].temp != 206)
					{
						bu[in].active = 0;
						chlog(cn, "%s ran out due to lack of endurance.", it[in].name);
					}
				}
				if (ch[cn].a_mana<500 && bu[in].mana[0] < -1)
				{
					ch[cn].a_mana = 500;
					if (bu[in].temp != 206)
					{
						bu[in].active = 0;
						chlog(cn, "%s ran out due to lack of mana.", it[in].name);
					}
				}
			}
			else
			{
				bu[in].active--;
				if (bu[in].active==TICKS*30 && bu[in].duration>=TICKS*60) // don't msg for skills shorter than 1m
				{
					if (ch[cn].flags & (CF_PLAYER | CF_USURP))
					{
						do_char_log(cn, 0, "%s is about to run out.\n", bu[in].name);
					}
					else
					{
						if (IS_COMP_TEMP(cn) && (co = ch[cn].data[CHD_MASTER]) && IS_SANEPLAYER(co) 
							&& (bu[in].temp==SK_BLESS || bu[in].temp==SK_PROTECT || bu[in].temp==SK_ENHANCE))
						{
							do_sayx(cn, "My spell %s is running out, %s.",
							        bu[in].name, ch[co].name);
						}
					}
				}
			}
			
			// Healing potions
			if (bu[in].temp==102 || bu[in].temp==SK_POME || bu[in].temp==SK_SOL)
			{
				if (bu[in].hp[0])
				{
					ch[cn].a_hp += bu[in].hp[0];
					if (ch[cn].a_hp>ch[cn].hp[5] * 1000)
						ch[cn].a_hp = ch[cn].hp[5] * 1000;
				}
				if (bu[in].end[0])
				{
					ch[cn].a_end += bu[in].end[0];
					if (ch[cn].a_end>ch[cn].end[5] * 1000)
						ch[cn].a_end = ch[cn].end[5] * 1000;
					if (ch[cn].a_end<500)
						ch[cn].a_end = 500;
				}
				if (bu[in].mana[0])
				{
					ch[cn].a_mana += bu[in].mana[0];
					if (ch[cn].a_mana>ch[cn].mana[5] * 1000)
						ch[cn].a_mana = ch[cn].mana[5] * 1000;
					if (ch[cn].a_mana<500)
						ch[cn].a_mana = 500;
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
			if ((bu[in].temp==SK_SLOW || bu[in].temp==SK_CURSE2) && bu[in].active>0)
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
			if (bu[in].temp==SK_POISON || bu[in].temp==SK_VENOM || bu[in].temp==SK_BLEED || bu[in].temp==SK_IMMOLATE2)
			{
				co = bu[in].data[0];
				degenpower = bu[in].data[1];
				
				if (!IS_SANECHAR(co)) co = 0;
				if ((co && !(ch[co].flags & CF_STONED)) || !co)
				{
					if (degenpower<1) degenpower = 1;
					degendam = degenpower;
					
					// Easy new method!
					if (co) degendam = degendam * ch[co].dmg_bonus / 10000;
							degendam = degendam * ch[cn].dmg_reduction / 10000;
					
					if (tmp = get_enchantment(cn, 53))
						degendam = degendam * max(25, 100-(tmp*15))/100;
					
					if (ch[cn].a_hp - (degendam + gothp)<500 && !(mf & MF_ARENA) && try_lucksave(cn) && !(ch[cn].flags & CF_IMMORTAL))
					{
						switch (bu[in].temp)
						{
							case SK_POISON: 	do_lucksave(cn, "lethal poisoning"); 	break;
							case SK_VENOM: 		do_lucksave(cn, "lethal venom"); 		break;
							case SK_BLEED: 		do_lucksave(cn, "lethal bleeding"); 	break;
							case SK_IMMOLATE2: 	do_lucksave(cn, "lethal burning"); 		break;
							default: break;
						}
					}
					else
					{
						ch[cn].a_hp -= degendam + gothp;
					}
					
					if (ch[cn].flags & CF_EXTRAEXP)  degendam = degendam * 2;
					if (ch[cn].flags & CF_EXTRACRIT) degendam = degendam * 3/2;
					
					tickcheck = max(1, 7500/max(1, degendam));
					
					if (co && (globs->ticker+cn)%tickcheck==0 && co!=cn && !(mf & MF_ARENA))
					{
						ch[co].points += 1;
						ch[co].points_tot += 1;
						do_check_new_level(co, 1);
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
									default: break;
								}
							}
							chlog(co, "Killed %s", ch[cn].name);
							if (co && !(mf & MF_ARENA))
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
								}
								if (IS_GLOB_MAYHEM) tmp += tmp / 5;
								if (ch[cn].flags & CF_EXTRAEXP)  tmp = tmp * 2;
								if (ch[cn].flags & CF_EXTRACRIT) tmp = tmp * 3/2;
							}

							if (co && co!=cn && !(mf & MF_ARENA))
							{
								if ((in2 = get_gear(co, IT_FORTERING)) && it[in2].active) // 25% more exp with Forte Ring
								{
									tmp = tmp*4/3;
								}
								do_give_exp(co, tmp, 1, rank);
								
								// stronghold points for contract
								if (kill_bsp)
									do_give_bspoints(co, tmp*kill_bsp*RATE_P_ENBS/100, 1);
								// osiris points for contract
								if (kill_osp)
									do_give_ospoints(co, tmp*kill_osp*RATE_P_ENOS/100, 1);
								
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
			
			// Frostburn
			if (bu[in].temp==SK_FROSTB)
			{
				ch[cn].a_end  += bu[in].end[0];
				ch[cn].a_mana += bu[in].mana[0];
				if (ch[cn].a_end<500)
				{
					ch[cn].a_end  = 500;
				}
				if (ch[cn].a_mana<500)
				{
					ch[cn].a_mana = 500;
				}
			}
			
			// Pulse
			if (bu[in].temp==SK_PULSE && globs->ticker>bu[in].data[2] && (co = bu[in].data[0]))
			{
				int pulse_dam, pulse_aoe, pulse_rad, x, y, xf, yf, xt, yt, xc, yc, aoe_power, cc;
				double tmp_a, tmp_h, tmp_s;
				int idx, nn;
				
				cc 		= cn;
				tmp_s   = (double)(bu[in].power);
				
				aoe_power = M_SK(cc, SK_PROX)+15;
				pulse_rad = bu[in].data[3];
				pulse_aoe = aoe_power/(PROX_CAP*2) + pulse_rad;
				tmp_a   = (double)(aoe_power*100/(PROX_CAP*2) + pulse_rad*100);
				tmp_h   = (double)(sqr(aoe_power*100/PROX_HIT-pulse_aoe)/500+(pulse_rad*300));
				
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
					if ((co = map[x + y * MAPX].ch) && cn!=co && cc!=co)
					{
						// Prevent pulse from hitting enemies that don't want to hurt you atm
						idx = cn | (char_id(cn) << 16);
						for (nn = MCD_ENEMY1ST; nn<=MCD_ENEMYZZZ; nn++)
						{
							if (ch[co].data[nn]==idx) break;
						}
						if (nn==MCD_ENEMYZZZ+1) continue;
						//
						pulse_dam = (int)(double)(min(tmp_s, tmp_s / max(1, (
							sqr(abs(xc - x)) + sqr(abs(yc - y))) / (tmp_h/100))));
						remember_pvp(cn, co);
						if (do_surround_check(cn, co, 1))
						{
							do_hurt(cn, co, spell_immunity(pulse_dam, get_target_immunity(cn, co)) * 2, 6);
							
							char_play_sound(co, ch[cn].sound + 20, -150, 0);
							do_area_sound(co, 0, ch[co].x, ch[co].y, ch[cn].sound + 20);
							fx_add_effect(5, 0, ch[co].x, ch[co].y, 0);
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
					
					for (m = 0; m<4; m++) ch[cn].enemy[m] = 0;
					remove_enemy(cn);
				}
				else if (bu[in].temp!=SK_ZEPHYR2)
				{
					do_char_log(cn, 0, "%s ran out.\n", bu[in].name);
				}
				bu[in].used = USE_EMPTY;
				ch[cn].spell[n] = 0;
				do_update_char(cn);
			}
		}
	}

	if (uwater && (ch[cn].flags & (CF_PLAYER)))
	{
		int waterlifeloss = 250;

		// Swimming! Was never actually in the base game!
		if (B_SK(cn, SK_SWIM))
			waterlifeloss = (250 - M_SK(cn, SK_SWIM)/6*5);
		
		// Amulet of Waterbreathing halves the result
		if (get_neck(cn, IT_BREATHAMMY))
			waterlifeloss /= 2;
		
		ch[cn].a_hp -= waterlifeloss + gothp;
		
		if (ch[cn].a_hp<500)
		{
			if (!(mf & MF_ARENA) && try_lucksave(cn))
				do_lucksave(cn, "watery depths");
			else
				do_char_killed(0, cn, 0);
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

int do_item_bsvalue(int temp)
{
	if (!IS_SANEITEMPLATE(temp))
	{
		return 0;
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

void do_appraisal(int cn, int in)
{
	int v = it[in].value/2;
	
	if (!(ch[cn].flags & CF_APPRAISE)) return;
	if (ch[cn].flags & CF_APPR_OFF) return;
	if (!(it_temp[it[in].temp].flags & IF_TAKE)) return;
	if (!v) return;
	
	do_char_log(cn, 2, "You estimate this could sell for %dG %dS.\n", v/100, v%100);
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
		do_char_log(cn, 2, "This chest will refill in %d hours and %d minutes.\n", it[in].active/one_hou, (it[in].active/one_min)%60);
	}
	else
	{
		do_char_log(cn, 2, "This chest will refill in %d minutes and %d seconds.\n", it[in].active/one_min, (it[in].active/TICKS)%60);
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
			do_ransack_corpse(cn, it[in].data[0], "In the tombstone you notice %s!\n");
		}
		if (it[in].driver==57)
		{
			int percent;

			percent = min(100, 100 * (ch[cn].points_tot / 10) / (it[in].data[4] + 1));

			if (percent<50)
			{
				do_char_log(cn, 2, "You sense that it's far too early in your career to touch this pole.\n");
			}
			else if (percent<75)
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
		brt = M_SK(cn, SK_BARTER);
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
	int sk = -1, m = 0;

	if (co<=0 || co>=MAXCHARS || nr<0 || nr>=186)
	{
		return;
	}
	if (!(ch[co].flags & CF_MERCHANT) && !(ch[co].flags & CF_BODY) && ch[co].gcm!=9)
	{
		return;
	}
	if (!(ch[co].flags & CF_BODY) && !do_char_can_see(cn, co) && ch[co].gcm!=9)
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

		if (ch[co].gold<pr)
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
						if (ch[co].temp == CT_TACTICIAN)
						{
							in2 = change_bs_shop_item(cn, it[in].temp);
							if (!in2) return;
							pr = do_item_bsvalue(in2);
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
							pr = do_item_bsvalue(it[in].temp);
							if (ch[cn].tokens<pr)
							{
								do_char_log(cn, 0, "You cannot afford that.\n");
								return;
							}
						}
						// Contract point items
						else if (ch[co].temp == CT_ADHERENT)
						{
							in2 = change_bs_shop_item(cn, it[in].temp);
							if (!in2) return;
							pr = do_item_bsvalue(in2);
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
						if (ch[co].temp == CT_TACTICIAN)
						{
							// Checks for a magic item variant
							in2 = get_special_item(cn, in2, 0, 0, 0);
							
							if (god_give_char(in2, cn))
							{
								ch[cn].bs_points -= pr;
								it[in2].data[8] = 2;
								
								if (it[in2].driver==48) it[in2].stack = it[in2].data[2];

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
						else if (ch[co].temp == CT_ADHERENT)
						{
							in2 = get_special_item(cn, in2, 0, 0, 0);
							
							if (god_give_char(in2, cn))
							{
								ch[cn].os_points -= pr;
								it[in2].data[8] = 4;
								
								if (it[in2].driver==48) it[in2].stack = it[in2].data[2];

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
										m = ch[cn].attrib[sk][1];
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
								do_char_log(cn, 1, "You took a %s.\n", it[in].reference);
								chlog(cn, "  Took the item...");
								if (ch[co].gcm==9)
								{
									chlog(cn, "  Triggered special chest");
									god_destroy_items(co);
									do_force_recall(cn);
									do_char_killed(0, co, 0);
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
						if (ch[co].temp == CT_TACTICIAN)
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
						else if (ch[co].temp == CT_ADHERENT)
						{
							in2 = change_bs_shop_item(cn, it[in].temp);
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
								int m = 0, mm = 0;
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
						/*
						if (it[in].flags & IF_SOULSTONE)
						{
							item_info(cn, in, 1);
						}
						*/
					}
				}
			}
			else if (nr<61)
			{
				if ((ch[co].flags & CF_BODY) && (in = ch[co].worn[nr - 40])!=0)
				{
					do_char_log(cn, 1, "%s:\n", it[in].name);
					do_char_log(cn, 1, "%s\n", it[in].description);
				}
			}
			else
			{
				if ((ch[co].flags & CF_BODY) && (in = ch[co].citem)!=0)
				{
					do_char_log(cn, 1, "%s:\n", it[in].name);
					do_char_log(cn, 1, "%s\n", it[in].description);
				}
			}
		}
	}
	if ((ch[co].flags & CF_MERCHANT) && !(ch[co].flags & CF_BSPOINTS))
	{
		update_shop(co);
	}
	do_look_char(cn, co, 0, 0, 1);
}

void do_waypoint(int cn, int nr)
{
	extern struct waypoint waypoint[MAXWPS];
	int i, j, m, in;
	
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
		
		fx_add_effect(6, 0, ch[cn].x, ch[cn].y, 0);
		god_transfer_char(cn, waypoint[nr%32].x, waypoint[nr%32].y);
		char_play_sound(cn, ch[cn].sound + 21, -100, 0);
		fx_add_effect(6, 0, ch[cn].x, ch[cn].y, 0);
	}
}

void do_treeupdate(int cn, int nr)
{
	//extern struct waypoint waypoint[MAXWPS];
	int i, j, m=0, in;
	int v = 100000;
	
	// Check that the waypoint ID from the client is valid
	if (nr<0 || nr>=36)
	{
		return;
	}
	
	if (IS_SEYAN_DU(cn))         m = 0;
	else if (IS_ARCHTEMPLAR(cn)) m = 1;
	else if (IS_SKALD(cn))       m = 2;
	else if (IS_WARRIOR(cn))     m = 3;
	else if (IS_SORCERER(cn))    m = 4;
	else if (IS_SUMMONER(cn))    m = 5;
	else if (IS_ARCHHARAKIM(cn)) m = 6;
	else                         m = 7;
	
	if (nr>=24)
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
			do_char_log(cn, 5, "You de-allocated %s for 1000G.\n", sk_tree[m][nr].name);
		
		ch[cn].tree_points &= ~(1u<<(15-nr));
		ch[cn].tree_points++;
		
		ch[cn].gold -= v;
		char_play_sound(cn, ch[cn].sound + 25, -100, 0);
		do_update_char(cn);
	}
	else if (nr>=12)
	{
		nr -= 12;
		do_char_log(cn, 5, "%s\n", sk_tree[m][nr].name);
		do_char_log(cn, 1, "%s\n", sk_tree[m][nr].desc);
		if (T_SK(cn, nr+1))
			do_char_log(cn, 4, "SHIFT + Left click to de-allocate for 1000G.\n");
		else
			do_char_log(cn, 6, "Left click to allocate.\n");
	}
	else
	{
		if (T_SK(cn, nr+1))
		{
			do_char_log(cn, 5, "You have %s allocated already!\n", sk_tree[m][nr].name);
			return;
		}
		if (nr>=3 && nr<=5 && !T_SK(cn, 1))
		{
			do_char_log(cn, 0, "You must allocate the previous node (%s) before you can allocate this one.\n", sk_tree[m][0].name);
			return;
		}
		if (nr>=6 && nr<=8 && !T_SK(cn, 2))
		{
			do_char_log(cn, 0, "You must allocate the previous node (%s) before you can allocate this one.\n", sk_tree[m][1].name);
			return;
		}
		if (nr>=9 && nr<=11 && !T_SK(cn, 3))
		{
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
			do_char_log(cn, 1, "You allocated %s.\n", sk_tree[m][nr].name);
		
		ch[cn].tree_points--;
		ch[cn].tree_points |= (1u<<(15-nr));
		
		char_play_sound(cn, ch[cn].sound + 25, -100, 0);
		do_update_char(cn);
	}
}

int do_depot_cost(int in)
{
	int cost;
	
	return (0);
	
	cost  = 1;
	cost += it[in].value / 1600;
	cost += (it[in].power * it[in].power * it[in].power) / 16000;

	if (it[in].flags & IF_LABYDESTROY)
	{
		cost += 20000;
	}

	return(cost);
}

int do_add_depot(int cn, int in)
{
	int n;

	for (n = 0; n<62; n++)
	{
		if (!ch[cn].depot[n])
		{
			break;
		}
	}

	if (n==62)
	{
		return 0;
	}

	ch[cn].depot[n] = in;
	it[in].carried = cn;
	do_update_char(cn);

	return 1;
}

void do_pay_depot(int cn)
{
	int c, in, n, lv, ln, tmp;

	while (1)
	{
		c = get_depot_cost(cn);

		if (c>ch[cn].data[13])
		{
			lv = 99999999;
			ln = -1;
			for (n = 0; n<62; n++)
			{
				if ((in = ch[cn].depot[n])!=0)
				{
					tmp = do_item_value(in);
					if (tmp<lv)
					{
						lv = tmp;
						ln = n;
					}
				}
			}
			if (ln==-1)
			{
				chlog(cn, "PANIC: depot forced sale failed!");
				return;
			}
			ch[cn].data[13] += lv / 2;
			in = ch[cn].depot[ln];
			it[in].used = USE_EMPTY;
			ch[cn].depot[ln] = 0;
			ch[cn].depot_sold++;
			chlog(cn, "Bank sold %s for %dG %dS to pay for depot (slot %d)",
			      it[in].name, (lv / 2) / 100, (lv / 2) % 100, ln);
		}
		else
		{
			ch[cn].data[13] -= c;
			ch[cn].depot_cost += c;
			break;
		}
	}
}

void do_depot_char(int cn, int co, int nr)
{
	int in, pr = 0, n, m, temp=0;

	if (co<=0 || co>=MAXCHARS || nr<0 || nr>=124)
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

		pr = do_depot_cost(in);

		if (do_add_depot(co, in))
		{
			ch[cn].citem = 0;
			if (!(ch[cn].flags & CF_SYS_OFF))
				do_char_log(cn, 1, "You deposited %s.\n", it[in].reference);
			/*
			do_char_log(cn, 1, "You deposited %s. The rent is %dG %dS per Astonian day or %dG %dS per Earth day.\n",
			            it[in].reference,
			            pr / 100, pr % 100, (pr * 18) / 100, (pr * 18) % 100);
			*/
			chlog(cn, "Deposited %s", it[in].name);
			do_update_char(cn);
		}
	}
	else
	{
		if (nr<62)
		{
			if ((in = ch[co].depot[nr])!=0)
			{
				if (god_give_char(in, cn))
				{
					ch[co].depot[nr] = 0;
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
		{
			nr -= 62;

			if ((in = ch[co].depot[nr])!=0)
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
	int sk = -1;

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
		p = do_char_can_see(player[ch[cn].player].spectating, co);
	}
	else
	{
		p = do_char_can_see(cn, co);
	}
	if (!p)
	{
		return;
	}

	if (!autoflag && !(ch[co].flags & CF_MERCHANT) && !(ch[co].flags & CF_BODY) && !(ch[co].gcm==9))
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
			            ch[co].data[16] % 300, ch[co].data[16] / 300,
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

		if ((ch[co].flags & (CF_PLAYER)) && (ch[co].flags & (CF_POH)))
		{
			if (ch[co].flags & CF_POH_LEADER)
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
		*(unsigned char*)(buf + 5) = 1;
		if ((in = ch[cn].citem)!=0)
		{
			if (ch[co].flags & CF_BSPOINTS)
			{
				int in2;
				// Stronghold items reflected by player stats
				if (ch[co].temp == CT_TACTICIAN)
				{
					in2 = change_bs_shop_item(cn, it[in].temp);
					pr = do_item_bsvalue(in2);
				}
				/*
				// Casino items
				else if (ch[co].temp == CT_JESSICA)
				{
					pr = do_item_bsvalue(it[in].temp);
				}
				// Contract items
				else if (ch[co].temp == CT_ADHERENT)
				{
					in2 = change_bs_shop_item(cn, it[in].temp);
					pr = do_item_bsvalue(in2);
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
	else
	{
		*(unsigned char*)(buf + 5) = 0;
	}

	xsend(nr, buf, 16);

	buf[0] = SV_LOOK5;
	for (n = 0; n<15; n++)
	{
		buf[n + 1] = ch[co].name[n];
	}
	xsend(nr, buf, 16);

	if (((ch[co].flags & (CF_MERCHANT | CF_BODY)) || (ch[co].gcm==9)) && !autoflag)
	{
		for (n = 0; n<MAXITEMS; n += 2)
		{
			buf[0] = SV_LOOK6;
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
						if (ch[co].temp == CT_TACTICIAN)
						{
							in2 = change_bs_shop_item(cn, it[in].temp);
							if (in2)
							{
								pr = do_item_bsvalue(in2);
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
							pr = do_item_bsvalue(it[in].temp);
						}
						// Contract items
						else if (ch[co].temp == CT_ADHERENT)
						{
							in2 = change_bs_shop_item(cn, it[in].temp);
							if (in2)
							{
								pr = do_item_bsvalue(in2);
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
			buf[0] = SV_LOOK6;
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

		buf[0] = SV_LOOK6;
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
			if (ch[co].temp == CT_TACTICIAN)
				*(unsigned char*)(buf + 14) = 101;
			else if (ch[co].temp == CT_JESSICA)
				*(unsigned char*)(buf + 14) = 102;
			else if (ch[co].temp == CT_ADHERENT)
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
		!(ch[co].flags & CF_MERCHANT) && !(ch[co].flags & CF_BODY) && !(ch[co].flags & CF_GOD))
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
	int n, nr, in, pr, spr, m, temp=0, ss, en;
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
	*(unsigned short*)(buf + 7)  = co | 0x8000;
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
	*(unsigned char*)(buf + 5) = 1;

	// CS, 000205: Check for sane item (not money)
	if (IS_SANEITEM(in = ch[cn].citem))
	{
		pr = 0;
		//pr = TICKS * do_depot_cost(in);
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

	for (n = 0; n<62; n += 2)
	{
		buf[0] = SV_LOOK6;
		buf[1] = n;
		for (m = n; m<min(62, n + 2); m++)
		{
			if ((in = ch[co].depot[m])!=0)
			{
				spr = it[in].sprite[0];
				if (it[in].flags & IF_SOULSTONE) ss = 1; else ss = 0;
				if (it[in].flags & IF_ENCHANTED) en = 1; else en = 0;
				pr  = 0;
				//pr  = TICKS * do_depot_cost(in);
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
			// 14 & 15 NOTE: 62~14 checks for BS
		}
		xsend(nr, buf, 16);
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

	for(m = 0; m<62; m++)
	{
		if ((in = ch[co].depot[m])!=0)
		{
			do_char_log(cn, 1, "%6d: %s\n", in, it[in].name);
			count++;
		}
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
	for (n = 0; n<MAXITEMS; n++)
	{
		if (in==ch[co].item[n])
		{
			i_index = n;
			found_i = !(0);
			break;
		}
	}

	if (!found_i)
	{
		for (n = 0; n<62; n++)
		{
			if (in==ch[co].depot[n])
			{
				i_index = n;
				found_d = !(0);
				break;
			}
		}
	}

	if (!found_i && !found_d)
	{
		for (n = 0; n<20; n++)
		{
			if (in==ch[co].worn[n])
			{
				i_index = n;
				found_e = !(0);
				break;
			}
		}
		for (n = 0; n<12; n++)
		{
			if (in==ch[co].alt_worn[n])
			{
				i_index = n;
				found_e = !(0);
				break;
			}
		}
	}

	if (found_i | found_d | found_e)
	{
		if (god_give_char(in, cn))
		{
			if (found_i)
			{
				ch[co].item[i_index] = 0;
			}
			else if (found_d)
			{
				ch[co].depot[i_index] = 0;
			}
			else if (found_e)
			{
				ch[co].worn[i_index] = 0;
			}

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

	for (m = 0; m<5; m++)
	{
		if ((unsigned char)it[in].attrib[m][2]>B_AT(cn, m))
		{
			do_char_log(cn, 0, "You're %s to use that.\n\n", at_text[m]);
			do_char_log(cn, 0, "(Need %d base %s)\n", (unsigned char)it[in].attrib[m][2],at_names[m]);
			return -1;
		}
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
	if (it[in].min_rank>getrank(cn))
	{
		do_char_log(cn, 0, "You're not experienced enough to use that.\n");
		do_char_log(cn, 0, "(Need a rank of %s)\n",rank_name[it[in].min_rank]);
		return -1;
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
		if (it[tmp].temp==IT_SEYANSWORD && it[tmp].data[0]!=cn)
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

		if (((it[tmp].flags & IF_KWAI_UNI) && (it[tmp].flags & IF_GORN_UNI) && IS_PURPLE(cn)) ||
		    ((it[tmp].flags & IF_PURP_UNI) && !IS_PURPLE(cn)) ||
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
	if ((tmp && it[tmp].temp==IT_ICELOTUS) && 
		((n == WN_RRING && it[ch[cn].worn[WN_LRING]].temp==IT_ICELOTUS) || 
		 (n == WN_LRING && it[ch[cn].worn[WN_RRING]].temp==IT_ICELOTUS)))
	{
		do_char_log(cn, 4, "You may only equip one Ice Lotus at a time.\n");
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
	if ((n==WN_RHAND && (it[ch[cn].worn[WN_RHAND]].temp==IT_WP_RISINGPHO || it[ch[cn].worn[WN_RHAND]].orig_temp==IT_WP_RISINGPHO)) || 
		(n==WN_LHAND && (it[ch[cn].worn[WN_LHAND]].temp==IT_WP_RISINGPHO || it[ch[cn].worn[WN_LHAND]].orig_temp==IT_WP_RISINGPHO)))
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
	int m1, m2;

	if (!IS_SANECHAR(cn) || !IS_SANECHAR(co))
	{
		return -1;
	}

	// unsafe gods may attack anyone
	if ((ch[cn].flags & CF_GOD && !(ch[cn].flags & CF_SAFE)))
	{
		return 1;
	}

	// unsafe gods may be attacked by anyone!
	if ((ch[co].flags & CF_GOD && !(ch[co].flags & CF_SAFE)))
	{
		return 1;
	}

	// player GC? act as if he would try to attack the master of the GC instead
	if (IS_COMPANION(cn) && ch[cn].data[64]==0)
	{
		cn = ch[cn].data[CHD_MASTER];
		if (!IS_SANECHAR(cn))
		{
			return 1;             // um, lets him try to kill this GC - it's got bad values anway
		}
	}

	// NPCs may attack anyone, anywhere
	if (!IS_PLAYER(cn))
	{
		return 1;
	}

	// Check for NOFIGHT
	m1 = XY2M(ch[cn].x, ch[cn].y);
	m2 = XY2M(ch[co].x, ch[co].y);
	if (((map[m1].flags | map[m2].flags) & MF_NOFIGHT) && !(IS_IN_BOJ(ch[cn].x, ch[cn].y) && get_gear(cn, IT_XIXDARKSUN)))
	{
		if (msg)
		{
			do_char_log(cn, 0, "You can't attack anyone here!\n");
		}
		return 0;
	}
	
	if (ch[co].temp==CT_ANNOU1 || ch[co].temp==CT_ANNOU2 || ch[co].temp==CT_ANNOU3 || 
		ch[co].temp==CT_ANNOU4 || ch[co].temp==CT_ANNOU5)
	{
		if (msg)
		{
			do_char_log(cn, 0, "Come on, you're better than that.\n");
		}
		return 0;
	}
	
	if (IS_THRALL(cn) || IS_THRALL(co))
	{
		return 1;
	}

	// player GC? act as if he would try to attack the master of the GC instead
	if (IS_COMPANION(co))
	{
		co = ch[co].data[CHD_MASTER];
		if (!IS_SANECHAR(co))
		{
			return 1;             // um, lets him try to kill this GC - it's got bad values anway
		}
	}

	// Check for plr-npc (OK)
	if (!IS_PLAYER(cn) || !IS_PLAYER(co))
	{
		return 1;
	}

	// Both are players. Check for Arena (OK)
	if (map[m1].flags & map[m2].flags & MF_ARENA)
	{
		return 1;
	}

	// Check if aggressor is purple
	if ((!IS_CLANKWAI(cn) && !IS_CLANGORN(cn)) ||
		(IS_CLANKWAI(cn) && !IS_CLANGORN(co)) ||
		(IS_CLANGORN(cn) && !IS_CLANKWAI(co)))
	{
		if (msg)
		{
			do_char_log(cn, 0, "You can't attack other players! You're not a member of an opposing clan.\n");
		}
		return 0;
	}

	// Check if victim is purple
	if (!IS_CLANKWAI(co) && !IS_CLANGORN(co))
	{
		if (msg)
		{
			do_char_log(cn, 0, "You can't attack %s! %s's not a member of an opposing clan.\n",
			            ch[co].name,
			            IS_FEMALE(co) ? "She" : "He");
		}
		return 0;
	}

	if (abs(getrank(cn) - getrank(co))>3)
	{
		if (msg)
		{
			do_char_log(cn, 0, "You're not allowed to attack %s. The rank difference is too large.\n", ch[co].name);
		}
		return 0;
	}

	return 1;
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
		else if (IS_ANY_TEMP(cn))
		{
			hp   = 15;
			mana = 5;
		}
		else if (IS_ANY_HARA(cn))
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
	int mf;
	mf = map[XY2M(ch[cn].x, ch[cn].y)].flags;
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

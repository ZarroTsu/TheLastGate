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
	int x, y, cc, m;
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
				if (cc!=cn && cc!=co)
				{
					if (!ch[cc].player && ch[cc].temp!=15)
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
	int  j, m, cc;
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
		return( 0);
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
				return( 1);
			}
		}
	}


	for (n = 50; n<59; n++)
	{
		if (ch[co].data[n]==cn)
		{
			return( 1);
		}
	}

	return(0);
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
	ch[cn].flags ^= CF_SENSE;
	
	if (ch[cn].flags & CF_SENSE)
	{
		do_char_log(cn, 1, "You will no longer see sense-magic spell messages.\n");
	}
	else
	{
		do_char_log(cn, 1, "You will see sense-magic spell messages again.\n");
	}
	
	if (ch[cn].flags & (CF_PLAYER))
	{
		chlog(cn, "Set sense to %s", (ch[cn].flags & CF_SENSE) ? "on" : "off");
	}
}

void do_silence(int cn)
{
	int cc;
	ch[cn].flags ^= CF_SILENCE;
	
	if ((cc = ch[cn].data[CHD_COMPANION]) && IS_SANECHAR(cc))
	{
		ch[cc].flags ^= CF_SILENCE;
	}
	if ((cc = ch[cn].data[CHD_SHADOWCOPY]) && IS_SANECHAR(cc))
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

	if ((in2 = ch[cn].citem)==0)
	{
		do_char_log(cn, 1, "Hold the item you'd like to dispose under your cursor first.\n");
		return;
	}
	if (in2 & 0x80000000)
	{
		val = in2 & 0x7fffffff;
		ch[cn].citem = 0;

		do_char_log(cn, 1, "You disposed of %u gold and %u silver.\n", val / 100, val % 100);
	}
	else
	{
		ch[cn].citem = 0;
		it[in2].used = USE_EMPTY;

		do_char_log(cn, 1, "You disposed of the %s.\n", it[in2].reference);
	}
}

void do_swap_gear(int cn)
{
	int n, in, in2;
	
	for (n=0; n<12; n++)
	{
		if (n==WN_CHARM||n==WN_CHARM2) continue; // don't swap cards!
		in = ch[cn].alt_worn[n];
		it[in].carried = cn;
		
		in2 = ch[cn].worn[n];
		it[in2].carried = cn;
		
		ch[cn].worn[n] = in;
		ch[cn].alt_worn[n] = in2;
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

	for (n = CHD_MINGROUP; n<=CHD_MAXGROUP; n++)
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
		if (strcmp(topic, "5")==0)
		{
			pagenum = 5;
			do_char_log(cn, 1, "The following commands are available (PAGE 5):\n");
			do_char_log(cn, 1, " \n");
			//                 "!        .         .   |     .         .        !"
			do_char_log(cn, 1, "#topaz                 list topaz rings.\n");
			do_char_log(cn, 1, "#trash                 delete item from cursor.\n");
			do_char_log(cn, 1, "#twohander             list twohander stats.\n");
			do_char_log(cn, 1, "#wave                  you'll wave.\n");
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
		else if (strcmp(topic, "4")==0)
		{
			pagenum = 4;
			do_char_log(cn, 1, "The following commands are available (PAGE 4):\n");
			do_char_log(cn, 1, " \n");
			//                 "!        .         .   |     .         .        !"
			do_char_log(cn, 1, "#sapphire              list sapphire rings.\n");
			do_char_log(cn, 1, "#sense                 disable enemy spell msgs.\n");
			do_char_log(cn, 1, "#silence               you won't hear enemies.\n");
			do_char_log(cn, 1, "#seen <player>         when last seen here?.\n");
			do_char_log(cn, 1, "#shield                list shield stats.\n");
			do_char_log(cn, 1, "#shout <text>          to all players.\n");
			do_char_log(cn, 1, "#skua                  leave purple.\n");
			do_char_log(cn, 1, "#sort <order>          sort inventory.\n");
			do_char_log(cn, 1, "#sortdepot <order>     sort depot.\n");
			do_char_log(cn, 1, "#spear                 list spear stats.\n");
			do_char_log(cn, 1, "#spellignore           don't attack if spelled.\n");
			do_char_log(cn, 1, "#spinel                list spinel rings.\n");
			do_char_log(cn, 1, "#staff                 list staff stats.\n");
			do_char_log(cn, 1, "#sword                 list sword stats.\n");
			do_char_log(cn, 1, "#tell <player> <text>  tells player text.\n");
		}
		else if (strcmp(topic, "3")==0)
		{
			pagenum = 3;
			do_char_log(cn, 1, "The following commands are available (PAGE 3):\n");
			do_char_log(cn, 1, " \n");
			//                 "!        .         .   |     .         .        !"
			do_char_log(cn, 1, "#ignore <player>       ignore that player.\n");
			do_char_log(cn, 1, "#iignore <player>      ignore normal talk too.\n");
			if (ch[cn].kindred & KIN_SEYAN_DU)
				do_char_log(cn, 1, "#kwai <page>           list unattained shrines.\n");
			do_char_log(cn, 1, "#lag <seconds>         lag control.\n");
			do_char_log(cn, 1, "#listskills <page>     list skill attributes.\n");
			do_char_log(cn, 1, "#max                   list character maximums.\n");
			do_char_log(cn, 1, "#noshout               you won't hear shouts.\n");
			do_char_log(cn, 1, "#notell                you won't hear tells.\n");
			do_char_log(cn, 1, "#opal                  list opal rings.\n");
			do_char_log(cn, 1, "#poles <page>          lists unattained poles.\n");
			do_char_log(cn, 1, "#quest <page>          list available quests.\n");
			do_char_log(cn, 1, "#rank                  show exp for next rank.\n");
			do_char_log(cn, 1, "#ranks                 show exp for all ranks.\n");
			do_char_log(cn, 1, "#ring <type>           list ring stats.\n");
			do_char_log(cn, 1, "#ruby                  list ruby rings.\n");
		}
		else if (strcmp(topic, "2")==0)
		{
			pagenum = 2;
			do_char_log(cn, 1, "The following commands are available (PAGE 2):\n");
			do_char_log(cn, 1, " \n");
			//                 "!        .         .   |     .         .        !"
			do_char_log(cn, 1, "#citrine               list citrine rings.\n");
			do_char_log(cn, 1, "#claw                  list claw stats.\n");
			do_char_log(cn, 1, "#dagger                list dagger stats.\n");
			do_char_log(cn, 1, "#diamond               list diamond rings.\n");
			do_char_log(cn, 1, "#dualsword             list dualsword stats.\n");
			do_char_log(cn, 1, "#emerald               list emerald rings.\n");
			do_char_log(cn, 1, "#fightback             toggle auto-fightback.\n");
			do_char_log(cn, 1, "#follow <player|self>  you'll follow player.\n");
			do_char_log(cn, 1, "#garbage               delete item from cursor.\n");
			if (ch[cn].kindred & (KIN_HARAKIM | KIN_SEYAN_DU | KIN_SUMMONER | KIN_ARCHHARAKIM))
				do_char_log(cn, 1, "#gcmax                 list ghostcomp maximums.\n");
			do_char_log(cn, 1, "#gctome                gc travels with you.\n");
			do_char_log(cn, 1, "#gold <amount>         get X gold coins.\n");
			do_char_log(cn, 1, "#greataxe              list greataxe stats.\n");
			do_char_log(cn, 1, "#group <player>        group with player.\n");
			do_char_log(cn, 1, "#gtell <message>       tell to your group.\n");
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
			if (ch[cn].skill[SK_PROX][0])
				do_char_log(cn, 1, "#area                  toggle area skills.\n");
			do_char_log(cn, 1, "#armor                 list armor stats.\n");
			do_char_log(cn, 1, "#autoloot              automatic grave looting.\n");
			do_char_log(cn, 1, "#axe                   list axe stats.\n");
			do_char_log(cn, 1, "#beryl                 list beryl rings.\n");
			do_char_log(cn, 1, "#bow                   you'll bow.\n");
			do_char_log(cn, 1, "#bs                    display BS points.\n");
			do_char_log(cn, 1, "#buff                  display buff timers.\n");
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
		do_char_log(cn, 2, "Showing page %d of 5. #help <x> to swap page.\n", pagenum);
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
		do_char_log(cn, (ch[cn].attrib[n][0]==ch[cn].attrib[n][2])?2:1, 
		"%20s  %3d  %3d\n", 
		at_name[n], ch[cn].attrib[n][0], ch[cn].attrib[n][2]);
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
			do_char_log(cn, (ch[cn].skill[n][0]==ch[cn].skill[n][2])?2:1, 
				"%20s  %3d  %3d\n", 
				skilltab[n].name, ch[cn].skill[n][0], ch[cn].skill[n][2]);
	}
	do_char_log(cn, 1, " \n");
}

void do_listgcmax(int cn)
{
	int n, co=0, archbonus=0;
	
	if (co = ch[cn].data[CHD_COMPANION])
	{
		if (!IS_SANECHAR(co) || ch[co].data[63]!=cn || (ch[co].flags & CF_BODY) || ch[co].used==USE_EMPTY)
		{
			co = 0;
		}
	}
	if (!co)
	{
		do_char_log(cn, 0, "You must summon a new companion first.\n");
	}
	if (ch[cn].skill[SK_GCMASTERY][0]) 
		archbonus 	= get_skill_score(cn, SK_GCMASTERY)/6;
	
	
	//                 "!        .         .   |     .         .        !"
	do_char_log(cn, 1, "Now listing skill maximums for your ghost:\n");
	do_char_log(cn, 1, " \n");
	//
	for (n=0;n<5;n++)
	{
		do_char_log(cn, (ch[co].attrib[n][0]==ch[co].attrib[n][2])?2:1, 
		"%20s  %3d  %3d\n", 
		at_name[n], ch[co].attrib[n][0], ch[co].attrib[n][2]+archbonus);
	}
	//
	do_char_log(cn, (ch[co].hp[0]==ch[co].hp[2])?2:1, 
		"%20s  %3d  %3d\n", 
		"Hitpoints", ch[co].hp[0], ch[co].hp[2]);
	do_char_log(cn, (ch[co].mana[0]==ch[co].mana[2])?2:1, 
		"%20s  %3d  %3d\n", 
		"Mana",      ch[co].mana[0], ch[co].mana[2]);
	//
	for (n=0;n<50;n++)
	{
		if (ch[co].skill[n][2]) 
			do_char_log(cn, (ch[co].skill[n][0]==ch[co].skill[n][2])?2:1, 
				"%20s  %3d  %3d\n", 
				skilltab[n].name, ch[co].skill[n][0], ch[co].skill[n][2]+archbonus);
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
		do_char_log(cn, 1, "Weapon Mastery         BRV + AGL + AGL\n");
		do_char_log(cn, 3, "Armor Mastery          BRV + STR + STR\n");
		do_char_log(cn, 1, "Bartering              BRV + INT + INT\n");
		do_char_log(cn, 3, "Companion Mastery      BRV + WIL + INT\n");
		do_char_log(cn, 1, "Concentrate            WIL + WIL + WIL\n");
		do_char_log(cn, 3, "Immunity               BRV + AGL + STR\n");
		do_char_log(cn, 1, "Perception             INT + INT + AGL\n");
		do_char_log(cn, 3, "Precision              BRV + AGL + STR\n");
		do_char_log(cn, 1, "Proximity              BRV + BRV + BRV\n");
		do_char_log(cn, 3, "Resistance             BRV + WIL + STR\n");
		do_char_log(cn, 1, "Sense Magic            WIL + WIL + STR\n");
		do_char_log(cn, 3, "Stealth                INT + AGL + AGL\n");
		do_char_log(cn, 1, "Surround Hit           BRV + AGL + STR\n");
		do_char_log(cn, 3, "Swimming               WIL + AGL + STR\n");
	}
	else if (strcmp(topic, "2")==0)
	{
		pagenum = 2;
		do_char_log(cn, 1, "Now listing skill attributes (PAGE 2):\n");
		do_char_log(cn, 1, " \n");
		//                 "!        .         .   |     .         .        !"
		do_char_log(cn, 1, "Blast                  BRV + INT + INT\n");
		do_char_log(cn, 3, "Bless                  BRV + WIL + WIL\n");
		do_char_log(cn, 1, "Curse                  BRV + INT + INT\n");
		do_char_log(cn, 3, "Dispel                 BRV + WIL + INT\n");
		do_char_log(cn, 1, "Enhance                BRV + WIL + WIL\n");
		do_char_log(cn, 3, "Ghost Companion        BRV + WIL + INT\n");
		do_char_log(cn, 1, "Haste                  BRV + WIL + AGL\n");
		do_char_log(cn, 3, "Heal                   BRV + INT + STR\n");
		do_char_log(cn, 1, "Identify               BRV + WIL + INT\n");
		do_char_log(cn, 3, "Light                  BRV + WIL + INT\n");
		do_char_log(cn, 1, "Magic Shield           BRV + WIL + WIL\n");
		do_char_log(cn, 3, "Poison                 BRV + INT + INT\n");
		do_char_log(cn, 1, "Protect                BRV + WIL + WIL\n");
		do_char_log(cn, 3, "Pulse                  BRV + INT + INT\n");
		do_char_log(cn, 1, "Razor                  BRV + AGL + AGL\n");
		do_char_log(cn, 3, "Recall                 BRV + WIL + INT\n");
		do_char_log(cn, 1, "Shadow Copy            BRV + WIL + WIL\n");
		do_char_log(cn, 3, "Slow                   BRV + INT + INT\n");
	}
	else
	{
		pagenum = 1;
		do_char_log(cn, 1, "Now listing skill attributes (PAGE 1):\n");
		do_char_log(cn, 1, " \n");
		//                 "!        .         .   |     .         .        !"
		do_char_log(cn, 1, "Regenerate             STR + STR + STR\n");
		do_char_log(cn, 3, "Rest                   AGL + AGL + AGL\n");
		do_char_log(cn, 1, "Meditate               INT + INT + INT\n");
		do_char_log(cn, 3, "Hand to Hand           AGL + AGL + STR\n");
		do_char_log(cn, 1, "Axe                    AGL + STR + STR\n");
		do_char_log(cn, 3, "Dagger                 WIL + WIL + AGL\n");
		do_char_log(cn, 1, "Dual Wield             BRV + AGL + STR\n");
		do_char_log(cn, 3, "Shield                 BRV + WIL + STR\n");
		do_char_log(cn, 1, "Staff                  INT + INT + STR\n");
		do_char_log(cn, 3, "Sword                  BRV + AGL + STR\n");
		do_char_log(cn, 1, "Two-Handed             AGL + AGL + STR\n");
		do_char_log(cn, 3, "Blind                  BRV + WIL + AGL\n");
		do_char_log(cn, 1, "Cleave                 BRV + STR + STR\n");
		do_char_log(cn, 3, "Leap                   BRV + AGL + STR\n");
		do_char_log(cn, 1, "Repair                 INT + AGL + STR\n");
		do_char_log(cn, 3, "Taunt                  BRV + STR + STR\n");
		do_char_log(cn, 1, "Warcry                 BRV + STR + STR\n");
		do_char_log(cn, 3, "Weaken                 BRV + AGL + AGL\n");
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
		do_char_log(cn, 1, "   Weapon   |   Requirements:   |   Gives:  \n");
		do_char_log(cn, 1, "    Tier    | AGL | STR | Skill |  WV | AGL \n");
		do_char_log(cn, 1, "------------+-----+-----+-------+-----+-----\n");
		do_char_log(cn, 1, "Bronze      |   1 |   1 |     1 |  10 |   1 \n");
		do_char_log(cn, 1, "Steel       |  15 |  12 |    16 |  20 |   2 \n");
		do_char_log(cn, 1, "Gold        |  25 |  18 |    32 |  30 |   3 \n");
		do_char_log(cn, 1, "Emerald     |  40 |  28 |    48 |  40 |   4 \n");
		do_char_log(cn, 1, "Crystal     |  60 |  42 |    64 |  50 |   5 \n");
		do_char_log(cn, 1, "Titanium    |  85 |  60 |    80 |  60 |   6 \n");
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
		do_char_log(cn, 1, "Bronze      |   1 |   1 |     1 |   4 |     \n");
		do_char_log(cn, 1, "Steel       |  12 |  12 |    10 |   8 |   1 \n");
		do_char_log(cn, 1, "Gold        |  18 |  14 |    20 |  12 |   1 \n");
		do_char_log(cn, 1, "Emerald     |  30 |  16 |    30 |  16 |   2 \n");
		do_char_log(cn, 1, "Crystal     |  48 |  20 |    40 |  20 |   2 \n");
		do_char_log(cn, 1, "Titanium    |  72 |  24 |    50 |  24 |   3 \n");
		do_char_log(cn, 1, " \n");
		do_char_log(cn, 2, "* Daggers can be used in the off-hand slot.\n");
		do_char_log(cn, 0, "* WV is reduced by 25%% in the off-hand slot.\n");
		do_char_log(cn, 2, "* Daggers have a critical hit chance of 2%%.\n");
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
		do_char_log(cn, 1, "Bronze      |   1 |   1 |     1 |   2 |   1 \n");
		do_char_log(cn, 1, "Steel       |  14 |  12 |    10 |   4 |   2 \n");
		do_char_log(cn, 1, "Gold        |  21 |  14 |    20 |   6 |   3 \n");
		do_char_log(cn, 1, "Emerald     |  35 |  16 |    30 |   8 |   4 \n");
		do_char_log(cn, 1, "Crystal     |  56 |  20 |    40 |  10 |   5 \n");
		do_char_log(cn, 1, "Titanium    |  84 |  24 |    50 |  12 |   6 \n");
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
		do_char_log(cn, 1, "    Tier    | WIL | STR | Skl |  WV | AV | BRV \n");
		do_char_log(cn, 1, "------------+-----+-----+-----+-----+----+-----\n");
		do_char_log(cn, 1, "Steel       |  16 |  12 |   8 |  18 |  2 |   4 \n");
		do_char_log(cn, 1, "Gold        |  22 |  14 |  16 |  27 |  3 |   6 \n");
		do_char_log(cn, 1, "Emerald     |  34 |  16 |  24 |  36 |  4 |   8 \n");
		do_char_log(cn, 1, "Crystal     |  52 |  20 |  32 |  45 |  5 |  10 \n");
		do_char_log(cn, 1, "Titanium    |  76 |  24 |  40 |  54 |  6 |  12 \n");
		do_char_log(cn, 1, " \n");
		do_char_log(cn, 2, "* Spears will use both hand slots.\n");
		do_char_log(cn, 0, "* Requires both Dagger and Staff skills.\n");
		do_char_log(cn, 2, "* Fighting uses the higher of either skill.\n");
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
		do_char_log(cn, 1, "Bronze      |  60 |  60 |     1 |    8   \n");
		do_char_log(cn, 1, "Steel       |  64 |  64 |    15 |   16   \n");
		do_char_log(cn, 1, "Gold        |  68 |  68 |    30 |   24   \n");
		do_char_log(cn, 1, "Emerald     |  74 |  74 |    45 |   32   \n");
		do_char_log(cn, 1, "Crystal     |  80 |  80 |    60 |   40   \n");
		do_char_log(cn, 1, "Titanium    |  88 |  88 |    75 |   48   \n");
		do_char_log(cn, 1, " \n");
		do_char_log(cn, 2, "* Dual-swords use the off-hand slot.\n");
		do_char_log(cn, 0, "* WV is halved when used by Arch-Templars.\n");
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
		do_char_log(cn, 1, "Steel       |  12 |  15 |    16 |  20 |   1 \n");
		do_char_log(cn, 1, "Gold        |  18 |  25 |    32 |  30 |   1 \n");
		do_char_log(cn, 1, "Emerald     |  28 |  40 |    48 |  40 |   2 \n");
		do_char_log(cn, 1, "Crystal     |  42 |  60 |    64 |  50 |   2 \n");
		do_char_log(cn, 1, "Titanium    |  60 |  85 |    80 |  60 |   3 \n");
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
		do_char_log(cn, 1, "    Tier    | AGL | STR | Skill |  WV | Mlt \n");
		do_char_log(cn, 1, "------------+-----+-----+-------+-----+-----\n");
		do_char_log(cn, 1, "Bronze      |   1 |   1 |     1 |  12 |     \n");
		do_char_log(cn, 1, "Steel       |  16 |  12 |    18 |  24 |   5 \n");
		do_char_log(cn, 1, "Gold        |  26 |  20 |    36 |  36 |  10 \n");
		do_char_log(cn, 1, "Emerald     |  40 |  32 |    54 |  48 |  15 \n");
		do_char_log(cn, 1, "Crystal     |  58 |  48 |    72 |  60 |  20 \n");
		do_char_log(cn, 1, "Titanium    |  80 |  68 |    90 |  72 |  25 \n");
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
		do_char_log(cn, 1, "Steel       |  12 |  14 |    15 |  28 |   2 \n");
		do_char_log(cn, 1, "Gold        |  20 |  24 |    30 |  42 |   4 \n");
		do_char_log(cn, 1, "Emerald     |  32 |  40 |    45 |  56 |   6 \n");
		do_char_log(cn, 1, "Crystal     |  48 |  62 |    60 |  70 |   8 \n");
		do_char_log(cn, 1, "Titanium    |  68 |  90 |    75 |  84 |  10 \n");
		do_char_log(cn, 1, " \n");
		do_char_log(cn, 2, "* Greataxes will use both hand slots.\n");
		do_char_log(cn, 0, "* Requires both Axe and Two-Handed skills.\n");
		do_char_log(cn, 2, "* Fighting uses the higher of either skill.\n");
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
		do_char_log(cn, 1, "  Silver, Small    |  LCp  |   0 |   2 \n");
		do_char_log(cn, 1, "  Silver, Medium   |  Sgt  |   1 |   3 \n");
		do_char_log(cn, 1, "  Silver, Big      |  MSg  |   2 |   4 \n");
		do_char_log(cn, 1, "    Gold, Medium   |  MSg  |   3 |   6 \n");
		do_char_log(cn, 1, "    Gold, Big      |  SgM  |   4 |   7 \n");
		do_char_log(cn, 1, "    Gold, Huge     | 1Lieu |   5 |   8 \n");
		do_char_log(cn, 1, "Platinum, Big      | 1Lieu |   6 |  10 \n");
		do_char_log(cn, 1, "Platinum, Huge     | Major |   7 |  11 \n");
		do_char_log(cn, 1, "Platinum, Flawless | Colnl |   8 |  12 \n");
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
		do_char_log(cn, 1, "  Silver, Small    |  LCp  |   2 |   6 \n");
		do_char_log(cn, 1, "  Silver, Medium   |  Sgt  |   3 |   7 \n");
		do_char_log(cn, 1, "  Silver, Big      |  MSg  |   4 |   8 \n");
		do_char_log(cn, 1, "    Gold, Medium   |  SSg  |   6 |  12 \n");
		do_char_log(cn, 1, "    Gold, Big      |  1Sg  |   8 |  14 \n");
		do_char_log(cn, 1, "    Gold, Huge     | 2Lieu |  10 |  16 \n");
		do_char_log(cn, 1, "Platinum, Big      |  SgM  |  12 |  20 \n");
		do_char_log(cn, 1, "Platinum, Huge     | 1Lieu |  16 |  24 \n");
		do_char_log(cn, 1, "Platinum, Flawless | Captn |  20 |  28 \n");
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
		do_char_log(cn, 1, "  Silver, Small    |  LCp  |   2 |   6 \n");
		do_char_log(cn, 1, "  Silver, Medium   |  Sgt  |   3 |   7 \n");
		do_char_log(cn, 1, "  Silver, Big      |  MSg  |   4 |   8 \n");
		do_char_log(cn, 1, "    Gold, Medium   |  SSg  |   6 |  12 \n");
		do_char_log(cn, 1, "    Gold, Big      |  1Sg  |   8 |  14 \n");
		do_char_log(cn, 1, "    Gold, Huge     | 2Lieu |  10 |  16 \n");
		do_char_log(cn, 1, "Platinum, Big      |  SgM  |  12 |  20 \n");
		do_char_log(cn, 1, "Platinum, Huge     | 1Lieu |  16 |  24 \n");
		do_char_log(cn, 1, "Platinum, Flawless | Captn |  20 |  28 \n");
		do_char_log(cn, 1, " \n");
	}
	else if (strcmp(topic, "3")==0 || strcmp(topic, "AMETHYST")==0 || strcmp(topic, "amethyst")==0)
	{
		do_char_log(cn, 1, "Now listing RUBY rings:\n");
		do_char_log(cn, 1, " \n");
		//                 "!        .         .         .         .        !"
		do_char_log(cn, 1, "     Ring Tier     | Need  | Intuition \n");
		do_char_log(cn, 1, "   Metal, Gem Size | Rank  | OFF |  ON \n");
		do_char_log(cn, 1, "-------------------+-------+-----+-----\n");
		do_char_log(cn, 1, "  Silver, Small    |  LCp  |   2 |   6 \n");
		do_char_log(cn, 1, "  Silver, Medium   |  Sgt  |   3 |   7 \n");
		do_char_log(cn, 1, "  Silver, Big      |  MSg  |   4 |   8 \n");
		do_char_log(cn, 1, "    Gold, Medium   |  SSg  |   6 |  12 \n");
		do_char_log(cn, 1, "    Gold, Big      |  1Sg  |   8 |  14 \n");
		do_char_log(cn, 1, "    Gold, Huge     | 2Lieu |  10 |  16 \n");
		do_char_log(cn, 1, "Platinum, Big      |  SgM  |  12 |  20 \n");
		do_char_log(cn, 1, "Platinum, Huge     | 1Lieu |  16 |  24 \n");
		do_char_log(cn, 1, "Platinum, Flawless | Captn |  20 |  28 \n");
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
		do_char_log(cn, 1, "  Silver, Small    |  LCp  |   2 |   6 \n");
		do_char_log(cn, 1, "  Silver, Medium   |  Sgt  |   3 |   7 \n");
		do_char_log(cn, 1, "  Silver, Big      |  MSg  |   4 |   8 \n");
		do_char_log(cn, 1, "    Gold, Medium   |  SSg  |   6 |  12 \n");
		do_char_log(cn, 1, "    Gold, Big      |  1Sg  |   8 |  14 \n");
		do_char_log(cn, 1, "    Gold, Huge     | 2Lieu |  10 |  16 \n");
		do_char_log(cn, 1, "Platinum, Big      |  SgM  |  12 |  20 \n");
		do_char_log(cn, 1, "Platinum, Huge     | 1Lieu |  16 |  24 \n");
		do_char_log(cn, 1, "Platinum, Flawless | Captn |  20 |  28 \n");
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
		do_char_log(cn, 1, "  Silver, Small    |  LCp  |   2 |   6 \n");
		do_char_log(cn, 1, "  Silver, Medium   |  Sgt  |   3 |   7 \n");
		do_char_log(cn, 1, "  Silver, Big      |  MSg  |   4 |   8 \n");
		do_char_log(cn, 1, "    Gold, Medium   |  SSg  |   6 |  12 \n");
		do_char_log(cn, 1, "    Gold, Big      |  1Sg  |   8 |  14 \n");
		do_char_log(cn, 1, "    Gold, Huge     | 2Lieu |  10 |  16 \n");
		do_char_log(cn, 1, "Platinum, Big      |  SgM  |  12 |  20 \n");
		do_char_log(cn, 1, "Platinum, Huge     | 1Lieu |  16 |  24 \n");
		do_char_log(cn, 1, "Platinum, Flawless | Captn |  20 |  28 \n");
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
		do_char_log(cn, 1, "  Silver, Big      |  1Sg  |   2 |   4 \n");
		do_char_log(cn, 1, "    Gold, Big      | 2Lieu |   4 |   7 \n");
		do_char_log(cn, 1, "    Gold, Huge     | Captn |   5 |   8 \n");
		do_char_log(cn, 1, "Platinum, Big      | Captn |   6 |  10 \n");
		do_char_log(cn, 1, "Platinum, Huge     | LtCol |   8 |  12 \n");
		do_char_log(cn, 1, "Platinum, Flawless | BrGen |  10 |  14 \n");
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
		do_char_log(cn, 1, "  Silver, Big      |  1Sg  |   4 |   8 \n");
		do_char_log(cn, 1, "    Gold, Big      | 2Lieu |   8 |  14 \n");
		do_char_log(cn, 1, "    Gold, Huge     | Captn |  10 |  16 \n");
		do_char_log(cn, 1, "Platinum, Big      | Captn |  12 |  20 \n");
		do_char_log(cn, 1, "Platinum, Huge     | LtCol |  16 |  24 \n");
		do_char_log(cn, 1, "Platinum, Flawless | BrGen |  20 |  28 \n");
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
		do_char_log(cn, 1, "  Silver, Big      |  1Sg  |   2 |   4 \n");
		do_char_log(cn, 1, "    Gold, Big      | 2Lieu |   4 |   7 \n");
		do_char_log(cn, 1, "    Gold, Huge     | Captn |   5 |   8 \n");
		do_char_log(cn, 1, "Platinum, Big      | Captn |   6 |  10 \n");
		do_char_log(cn, 1, "Platinum, Huge     | LtCol |   8 |  12 \n");
		do_char_log(cn, 1, "Platinum, Flawless | BrGen |  10 |  14 \n");
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
		do_char_log(cn, 1, "  Silver, Big      |  1Sg  |   6 |  12 \n");
		do_char_log(cn, 1, "    Gold, Big      | 2Lieu |  12 |  21 \n");
		do_char_log(cn, 1, "    Gold, Huge     | Captn |  15 |  24 \n");
		do_char_log(cn, 1, "Platinum, Big      | Captn |  18 |  30 \n");
		do_char_log(cn, 1, "Platinum, Huge     | LtCol |  24 |  36 \n");
		do_char_log(cn, 1, "Platinum, Flawless | BrGen |  30 |  42 \n");
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
		do_char_log(cn, 1, "    Gold, Huge     | Captn |  20 |  60 \n");
		do_char_log(cn, 1, "Platinum, Huge     | LtCol |  40 | 100 \n");
		do_char_log(cn, 1, "Platinum, Flawless | BrGen |  60 | 120 \n");
		do_char_log(cn, 1, " \n");
	}
	else if (strcmp(topic, "11")==0 || strcmp(topic, "ZIRCON")==0 || strcmp(topic, "zircon")==0)
	{
		do_char_log(cn, 1, "Now listing ZIRCON rings:\n");
		do_char_log(cn, 1, " \n");
		//                 "!        .         .         .         .        !"
		do_char_log(cn, 1, "     Ring Tier     | Need  |  Reflect  \n");
		do_char_log(cn, 1, "  Metal, Gem Size  | Rank  | OFF |  ON \n");
		do_char_log(cn, 1, "-------------------+-------+-----+-----\n");
		do_char_log(cn, 1, "    Gold, Huge     | Captn |   2 |   6 \n");
		do_char_log(cn, 1, "Platinum, Huge     | LtCol |   4 |  10 \n");
		do_char_log(cn, 1, "Platinum, Flawless | BrGen |   6 |  12 \n");
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
		do_char_log(cn, 1, " \n");
	}
}

void do_strongholdpoints(int cn)
{
	int n, wavenum=0, waveprog=0;
	do_char_log(cn, 2, "You currently have %d Black Stronghold Points.\n",stronghold_points(cn));
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
		do_char_log(cn, 3, "Wave %d progress: %3d%%\n", ch[n].data[3], waveprog/100);
	}
}

void do_showrank(int cn)
{
	if (points2rank(ch[cn].points_tot)>=RANKS-1)
	{
		do_char_log(cn, 2, "You are at the maximum rank. Good job.\n");
	}
	else
	{
		do_char_log(cn, 2, "You need %d exp to rank to %s.\n",
			rank2points(points2rank(ch[cn].points_tot)) - ch[cn].points_tot,
			rank_name[points2rank(ch[cn].points_tot)+1]);
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
	if (!kwai[29]) { m++; if (m<=t && m>n) do_char_log(cn, 1, "Lab XIII, Final Gorge\n"); }
	if (!kwai[11]) { m++; if (m<=t && m>n) do_char_log(cn, 1, "Northern Mountains\n"); }
	if (!kwai[ 5]) { m++; if (m<=t && m>n) do_char_log(cn, 1, "Stronghold, South\n"); }
	if (!kwai[ 9]) { m++; if (m<=t && m>n) do_char_log(cn, 1, "Underground III\n"); }
	if (!kwai[16]) { m++; if (m<=t && m>n) do_char_log(cn, 1, "Jungle Pentagram Quest\n"); }
	if (!kwai[ 6]) { m++; if (m<=t && m>n) do_char_log(cn, 1, "Mausoleum XI\n"); }
	//
	if (!kwai[13]) { m++; if (m<=t && m>n) do_char_log(cn, 1, "(Not yet available)\n"); } 		// Ice Gargoyle Nest
	if (!kwai[30]) { m++; if (m<=t && m>n) do_char_log(cn, 1, "(Not yet available)\n"); } 		// 
	if (!kwai[31]) { m++; if (m<=t && m>n) do_char_log(cn, 1, "(Not yet available)\n"); } 		// 
	//
	do_char_log(cn, 1, " \n");
	do_char_log(cn, 2, "Showing page %d of %d. #kwai <x> to swap.\n", page, min(2, m/16+1));
	do_char_log(cn, 1, " \n");
}

void do_showpoles(int cn, int flag, char *topic) // flag 1 = all, 0 = unattained
{
	int n, m, t, page = 1;
	int pole1[32]={0};
	int pole2[32]={0};
	int pole3[32]={0};
	int pole4[32]={0};

	if (strcmp(topic, "2")==0) page = 2;
	if (strcmp(topic, "3")==0) page = 3;
	if (strcmp(topic, "4")==0) page = 4;
	if (strcmp(topic, "5")==0) page = 5;
	if (strcmp(topic, "6")==0) page = 6;
	if (strcmp(topic, "7")==0) page = 7;
	if (strcmp(topic, "8")==0) page = 8;

	for (n=0;n<32;n++)
	{
		if (ch[cn].data[46] & (1 << n)) pole1[n] = 1;
		if (ch[cn].data[47] & (1 << n)) pole2[n] = 1;
		if (ch[cn].data[48] & (1 << n)) pole3[n] = 1;
		if (ch[cn].data[49] & (1 << n)) pole4[n] = 1;
	}

	do_char_log(cn, 1, "Now listing poles (PAGE %d):\n", page);
	do_char_log(cn, 1, " \n");
	
	n = (page-1)*16;
	t = page*16;
	m = 0;
	//
	if (flag || !pole1[ 0]) { m++; if (m<=t && m>n) do_char_log(cn, (pole1[ 0])?3:1, "Thieves House Cellar     %6d\n", (pole1[ 0])?0:   200); }
	if (flag || !pole4[ 4]) { m++; if (m<=t && m>n) do_char_log(cn, (pole4[ 4])?3:1, "Novice Pentagram Quest   %6d\n", (pole4[ 4])?0:  1000); }
	if (flag || !pole1[ 2]) { m++; if (m<=t && m>n) do_char_log(cn, (pole1[ 2])?3:1, "Weeping Woods            %6d\n", (pole1[ 2])?0:  2000); }
	if (flag || !pole1[ 3]) { m++; if (m<=t && m>n) do_char_log(cn, (pole1[ 3])?3:1, "Weeping Woods            %6d\n", (pole1[ 3])?0:  2000); }
	if (flag || !pole4[ 5]) { m++; if (m<=t && m>n) do_char_log(cn, (pole4[ 5])?3:1, "Novice Pentagram Quest   %6d\n", (pole4[ 5])?0:  2000); }
	if (flag || !pole4[ 6]) { m++; if (m<=t && m>n) do_char_log(cn, (pole4[ 6])?3:1, "Novice Pentagram Quest   %6d\n", (pole4[ 6])?0:  3000); }
	if (flag || !pole1[ 4]) { m++; if (m<=t && m>n) do_char_log(cn, (pole1[ 4])?3:1, "Spider's Den             %6d\n", (pole1[ 4])?0:  4000); }
	if (flag || !pole1[ 5]) { m++; if (m<=t && m>n) do_char_log(cn, (pole1[ 5])?3:1, "Old Manor                %6d\n", (pole1[ 5])?0:  4000); }
	if (flag || !pole4[ 7]) { m++; if (m<=t && m>n) do_char_log(cn, (pole4[ 7])?3:1, "Earth Pentagram Quest    %6d\n", (pole4[ 7])?0:  4000); }
	if (flag || !pole3[ 0]) { m++; if (m<=t && m>n) do_char_log(cn, (pole3[ 0])?3:1, "Lab I, Grolm Gorge       %6d\n", (pole3[ 0])?0:  5000); }
	if (flag || !pole3[ 1]) { m++; if (m<=t && m>n) do_char_log(cn, (pole3[ 1])?3:1, "Lab I, Grolm Gorge       %6d\n", (pole3[ 1])?0:  5000); }
	if (flag || !pole3[ 2]) { m++; if (m<=t && m>n) do_char_log(cn, (pole3[ 2])?3:1, "Lab I, Grolm Gorge       %6d\n", (pole3[ 2])?0:  5000); }
	if (flag || !pole4[ 8]) { m++; if (m<=t && m>n) do_char_log(cn, (pole4[ 8])?3:1, "Earth Pentagram Quest    %6d\n", (pole4[ 8])?0:  6000); }
	if (flag || !pole1[ 6]) { m++; if (m<=t && m>n) do_char_log(cn, (pole1[ 6])?3:1, "Arachnid Den             %6d\n", (pole1[ 6])?0:  6000); }
	if (flag || !pole2[14]) { m++; if (m<=t && m>n) do_char_log(cn, (pole2[14])?3:1, "Underground I            %6d\n", (pole2[14])?0:  6000); }
	if (flag || !pole3[ 3]) { m++; if (m<=t && m>n) do_char_log(cn, (pole3[ 3])?3:1, "Lab II, Lizard Gorge     %6d\n", (pole3[ 3])?0:  6500); }
	if (flag || !pole3[ 4]) { m++; if (m<=t && m>n) do_char_log(cn, (pole3[ 4])?3:1, "Lab II, Lizard Gorge     %6d\n", (pole3[ 4])?0:  6500); }
	if (flag || !pole3[ 5]) { m++; if (m<=t && m>n) do_char_log(cn, (pole3[ 5])?3:1, "Lab II, Lizard Gorge     %6d\n", (pole3[ 5])?0:  6500); }
	if (flag || !pole2[15]) { m++; if (m<=t && m>n) do_char_log(cn, (pole2[15])?3:1, "Underground I            %6d\n", (pole2[15])?0:  6500); }
	if (flag || !pole1[ 7]) { m++; if (m<=t && m>n) do_char_log(cn, (pole1[ 7])?3:1, "Butler's Mansion         %6d\n", (pole1[ 7])?0:  7000); }
	if (flag || !pole2[16]) { m++; if (m<=t && m>n) do_char_log(cn, (pole2[16])?3:1, "Underground I            %6d\n", (pole2[16])?0:  7000); }
	if (flag || !pole2[17]) { m++; if (m<=t && m>n) do_char_log(cn, (pole2[17])?3:1, "Underground I            %6d\n", (pole2[17])?0:  7500); }
	if (flag || !pole1[ 1]) { m++; if (m<=t && m>n) do_char_log(cn, (pole1[ 1])?3:1, "Azrael's Throne Room     %6d\n", (pole1[ 1])?0:  8000); }
	if (flag || !pole1[ 8]) { m++; if (m<=t && m>n) do_char_log(cn, (pole1[ 8])?3:1, "Bell House Basement      %6d\n", (pole1[ 8])?0:  8000); }
	if (flag || !pole2[18]) { m++; if (m<=t && m>n) do_char_log(cn, (pole2[18])?3:1, "Underground I            %6d\n", (pole2[18])?0:  8000); }
	if (flag || !pole4[ 9]) { m++; if (m<=t && m>n) do_char_log(cn, (pole4[ 9])?3:1, "Earth Pentagram Quest    %6d\n", (pole4[ 9])?0:  8000); }
	if (flag || !pole3[ 6]) { m++; if (m<=t && m>n) do_char_log(cn, (pole3[ 6])?3:1, "Lab III, Undead Gorge    %6d\n", (pole3[ 6])?0:  9450); }
	if (flag || !pole3[ 7]) { m++; if (m<=t && m>n) do_char_log(cn, (pole3[ 7])?3:1, "Lab III, Undead Gorge    %6d\n", (pole3[ 7])?0:  9450); }
	if (flag || !pole3[ 8]) { m++; if (m<=t && m>n) do_char_log(cn, (pole3[ 8])?3:1, "Lab III, Undead Gorge    %6d\n", (pole3[ 8])?0:  9450); }
	if (flag || !pole2[19]) { m++; if (m<=t && m>n) do_char_log(cn, (pole2[19])?3:1, "Underground I            %6d\n", (pole2[19])?0: 10000); }
	if (flag || !pole1[ 9]) { m++; if (m<=t && m>n) do_char_log(cn, (pole1[ 9])?3:1, "Webbed Bush              %6d\n", (pole1[ 9])?0: 10000); }
	if (flag || !pole2[ 7]) { m++; if (m<=t && m>n) do_char_log(cn, (pole2[ 7])?3:1, "Stronghold, North        %6d\n", (pole2[ 7])?0: 10000); }
	if (flag || !pole4[10]) { m++; if (m<=t && m>n) do_char_log(cn, (pole4[10])?3:1, "Earth Pentagram Quest    %6d\n", (pole4[10])?0: 10000); }
	if (flag || !pole1[10]) { m++; if (m<=t && m>n) do_char_log(cn, (pole1[10])?3:1, "Aston Mines, Level II    %6d\n", (pole1[10])?0: 12000); }
	if (flag || !pole4[11]) { m++; if (m<=t && m>n) do_char_log(cn, (pole4[11])?3:1, "Earth Pentagram Quest    %6d\n", (pole4[11])?0: 12000); }
	if (flag || !pole3[ 9]) { m++; if (m<=t && m>n) do_char_log(cn, (pole3[ 9])?3:1, "Lab IV, Caster Gorge     %6d\n", (pole3[ 9])?0: 13800); }
	if (flag || !pole3[10]) { m++; if (m<=t && m>n) do_char_log(cn, (pole3[10])?3:1, "Lab IV, Caster Gorge     %6d\n", (pole3[10])?0: 13800); }
	if (flag || !pole3[11]) { m++; if (m<=t && m>n) do_char_log(cn, (pole3[11])?3:1, "Lab IV, Caster Gorge     %6d\n", (pole3[11])?0: 13800); }
	if (flag || !pole1[11]) { m++; if (m<=t && m>n) do_char_log(cn, (pole1[11])?3:1, "Astonia Penitentiary     %6d\n", (pole1[11])?0: 14000); }
	if (flag || !pole1[16]) { m++; if (m<=t && m>n) do_char_log(cn, (pole1[16])?3:1, "Forgotten Canyon         %6d\n", (pole1[16])?0: 15000); }
	if (flag || !pole4[12]) { m++; if (m<=t && m>n) do_char_log(cn, (pole4[12])?3:1, "Earth Pentagram Quest    %6d\n", (pole4[12])?0: 15000); }
	if (flag || !pole1[13]) { m++; if (m<=t && m>n) do_char_log(cn, (pole1[13])?3:1, "The Mad Hermit's House   %6d\n", (pole1[13])?0: 16000); }
	if (flag || !pole1[19]) { m++; if (m<=t && m>n) do_char_log(cn, (pole1[19])?3:1, "Abandoned Archives       %6d\n", (pole1[19])?0: 18000); }
	if (flag || !pole4[13]) { m++; if (m<=t && m>n) do_char_log(cn, (pole4[13])?3:1, "Fire Pentagram Quest     %6d\n", (pole4[13])?0: 18000); }
	if (flag || !pole3[12]) { m++; if (m<=t && m>n) do_char_log(cn, (pole3[12])?3:1, "Lab V, Knight Gorge      %6d\n", (pole3[12])?0: 19500); }
	if (flag || !pole3[13]) { m++; if (m<=t && m>n) do_char_log(cn, (pole3[13])?3:1, "Lab V, Knight Gorge      %6d\n", (pole3[13])?0: 19500); }
	if (flag || !pole3[14]) { m++; if (m<=t && m>n) do_char_log(cn, (pole3[14])?3:1, "Lab V, Knight Gorge      %6d\n", (pole3[14])?0: 19500); }
	if (flag || !pole1[17]) { m++; if (m<=t && m>n) do_char_log(cn, (pole1[17])?3:1, "Jagged Pass Cellar       %6d\n", (pole1[17])?0: 20000); }
	if (flag || !pole1[12]) { m++; if (m<=t && m>n) do_char_log(cn, (pole1[12])?3:1, "Grolm Laboratory         %6d\n", (pole1[12])?0: 20000); }
	if (flag || !pole4[14]) { m++; if (m<=t && m>n) do_char_log(cn, (pole4[14])?3:1, "Fire Pentagram Quest     %6d\n", (pole4[14])?0: 21000); }
	if (flag || !pole2[20]) { m++; if (m<=t && m>n) do_char_log(cn, (pole2[20])?3:1, "Underground II           %6d\n", (pole2[20])?0: 24000); }
	if (flag || !pole4[15]) { m++; if (m<=t && m>n) do_char_log(cn, (pole4[15])?3:1, "Fire Pentagram Quest     %6d\n", (pole4[15])?0: 24000); }
	if (flag || !pole1[14]) { m++; if (m<=t && m>n) do_char_log(cn, (pole1[14])?3:1, "Violet Lakebed           %6d\n", (pole1[14])?0: 25000); }
	if (flag || !pole2[21]) { m++; if (m<=t && m>n) do_char_log(cn, (pole2[21])?3:1, "Underground II           %6d\n", (pole2[21])?0: 26000); }
	if (flag || !pole3[15]) { m++; if (m<=t && m>n) do_char_log(cn, (pole3[15])?3:1, "Lab VI, Desert Gorge     %6d\n", (pole3[15])?0: 26500); }
	if (flag || !pole3[16]) { m++; if (m<=t && m>n) do_char_log(cn, (pole3[16])?3:1, "Lab VI, Desert Gorge     %6d\n", (pole3[16])?0: 26500); }
	if (flag || !pole3[17]) { m++; if (m<=t && m>n) do_char_log(cn, (pole3[17])?3:1, "Lab VI, Desert Gorge     %6d\n", (pole3[17])?0: 26500); }
	if (flag || !pole2[22]) { m++; if (m<=t && m>n) do_char_log(cn, (pole2[22])?3:1, "Underground II           %6d\n", (pole2[22])?0: 28000); }
	if (flag || !pole4[16]) { m++; if (m<=t && m>n) do_char_log(cn, (pole4[16])?3:1, "Fire Pentagram Quest     %6d\n", (pole4[16])?0: 28000); }
	if (flag || !pole2[23]) { m++; if (m<=t && m>n) do_char_log(cn, (pole2[23])?3:1, "Underground II           %6d\n", (pole2[23])?0: 30000); }
	if (flag || !pole2[ 8]) { m++; if (m<=t && m>n) do_char_log(cn, (pole2[ 8])?3:1, "Black Stronghold         %6d\n", (pole2[ 8])?0: 30000); }
	if (flag || !pole2[24]) { m++; if (m<=t && m>n) do_char_log(cn, (pole2[24])?3:1, "Underground II           %6d\n", (pole2[24])?0: 32000); }
	if (flag || !pole4[17]) { m++; if (m<=t && m>n) do_char_log(cn, (pole4[17])?3:1, "Fire Pentagram Quest     %6d\n", (pole4[17])?0: 32000); }
	if (flag || !pole1[18]) { m++; if (m<=t && m>n) do_char_log(cn, (pole1[18])?3:1, "Winding Valley           %6d\n", (pole1[18])?0: 32500); }
	if (flag || !pole3[18]) { m++; if (m<=t && m>n) do_char_log(cn, (pole3[18])?3:1, "Lab VII, Light/Dark Gor  %6d\n", (pole3[18])?0: 34750); }
	if (flag || !pole3[19]) { m++; if (m<=t && m>n) do_char_log(cn, (pole3[19])?3:1, "Lab VII, Light/Dark Gor  %6d\n", (pole3[19])?0: 34750); }
	if (flag || !pole3[20]) { m++; if (m<=t && m>n) do_char_log(cn, (pole3[20])?3:1, "Lab VII, Light/Dark Gor  %6d\n", (pole3[20])?0: 34750); }
	if (flag || !pole4[18]) { m++; if (m<=t && m>n) do_char_log(cn, (pole4[18])?3:1, "Fire Pentagram Quest     %6d\n", (pole4[18])?0: 36000); }
	if (flag || !pole2[25]) { m++; if (m<=t && m>n) do_char_log(cn, (pole2[25])?3:1, "Underground II           %6d\n", (pole2[25])?0: 40000); }
	if (flag || !pole2[ 0]) { m++; if (m<=t && m>n) do_char_log(cn, (pole2[ 0])?3:1, "Gargoyle Nest            %6d\n", (pole2[ 0])?0: 40000); }
	if (flag || !pole4[19]) { m++; if (m<=t && m>n) do_char_log(cn, (pole4[19])?3:1, "Fire Pentagram Quest     %6d\n", (pole4[19])?0: 40000); }
	if (flag || !pole3[21]) { m++; if (m<=t && m>n) do_char_log(cn, (pole3[21])?3:1, "Lab VIII, Underwater Go  %6d\n", (pole3[21])?0: 44200); }
	if (flag || !pole3[22]) { m++; if (m<=t && m>n) do_char_log(cn, (pole3[22])?3:1, "Lab VIII, Underwater Go  %6d\n", (pole3[22])?0: 44200); }
	if (flag || !pole3[23]) { m++; if (m<=t && m>n) do_char_log(cn, (pole3[23])?3:1, "Lab VIII, Underwater Go  %6d\n", (pole3[23])?0: 44200); }
	if (flag || !pole1[21]) { m++; if (m<=t && m>n) do_char_log(cn, (pole1[21])?3:1, "Southern Swamp           %6d\n", (pole1[21])?0: 45000); }
	if (flag || !pole4[20]) { m++; if (m<=t && m>n) do_char_log(cn, (pole4[20])?3:1, "Jungle Pentagram Quest   %6d\n", (pole4[20])?0: 45000); }
	if (flag || !pole2[ 1]) { m++; if (m<=t && m>n) do_char_log(cn, (pole2[ 1])?3:1, "Gargoyle Nest            %6d\n", (pole2[ 1])?0: 50000); }
	if (flag || !pole4[21]) { m++; if (m<=t && m>n) do_char_log(cn, (pole4[21])?3:1, "Jungle Pentagram Quest   %6d\n", (pole4[21])?0: 50000); }
	if (flag || !pole3[24]) { m++; if (m<=t && m>n) do_char_log(cn, (pole3[24])?3:1, "Lab IX, Riddle Gorge     %6d\n", (pole3[24])?0: 54800); }
	if (flag || !pole3[25]) { m++; if (m<=t && m>n) do_char_log(cn, (pole3[25])?3:1, "Lab IX, Riddle Gorge     %6d\n", (pole3[25])?0: 54800); }
	if (flag || !pole3[26]) { m++; if (m<=t && m>n) do_char_log(cn, (pole3[26])?3:1, "Lab IX, Riddle Gorge     %6d\n", (pole3[26])?0: 54800); }
	if (flag || !pole4[22]) { m++; if (m<=t && m>n) do_char_log(cn, (pole4[22])?3:1, "Jungle Pentagram Quest   %6d\n", (pole4[22])?0: 55000); }
	if (flag || !pole1[15]) { m++; if (m<=t && m>n) do_char_log(cn, (pole1[15])?3:1, "Aston Mines, Level III   %6d\n", (pole1[15])?0: 60000); }
	if (flag || !pole2[ 2]) { m++; if (m<=t && m>n) do_char_log(cn, (pole2[ 2])?3:1, "Gargoyle Nest            %6d\n", (pole2[ 2])?0: 60000); }
//	if (flag || !pole2[10]) { m++; if (m<=t && m>n) do_char_log(cn, (pole2[10])?3:1, "Tower X                  %6d\n", (pole2[10])?0: 60000); }
	if (flag || !pole4[23]) { m++; if (m<=t && m>n) do_char_log(cn, (pole4[23])?3:1, "Jungle Pentagram Quest   %6d\n", (pole4[23])?0: 60000); }
	if (flag || !pole4[24]) { m++; if (m<=t && m>n) do_char_log(cn, (pole4[24])?3:1, "Jungle Pentagram Quest   %6d\n", (pole4[24])?0: 66000); }
	if (flag || !pole3[27]) { m++; if (m<=t && m>n) do_char_log(cn, (pole3[27])?3:1, "Lab X, Forest Gorge      %6d\n", (pole3[27])?0: 66500); }
	if (flag || !pole3[28]) { m++; if (m<=t && m>n) do_char_log(cn, (pole3[28])?3:1, "Lab X, Forest Gorge      %6d\n", (pole3[28])?0: 66500); }
	if (flag || !pole3[29]) { m++; if (m<=t && m>n) do_char_log(cn, (pole3[29])?3:1, "Lab X, Forest Gorge      %6d\n", (pole3[29])?0: 66500); }
	if (flag || !pole2[26]) { m++; if (m<=t && m>n) do_char_log(cn, (pole2[26])?3:1, "Underground III          %6d\n", (pole2[26])?0: 72000); }
	if (flag || !pole4[25]) { m++; if (m<=t && m>n) do_char_log(cn, (pole4[25])?3:1, "Ice Pentagram Quest      %6d\n", (pole4[25])?0: 72000); }
	if (flag || !pole2[27]) { m++; if (m<=t && m>n) do_char_log(cn, (pole2[27])?3:1, "Underground III          %6d\n", (pole2[27])?0: 78000); }
	if (flag || !pole4[26]) { m++; if (m<=t && m>n) do_char_log(cn, (pole4[26])?3:1, "Ice Pentagram Quest      %6d\n", (pole4[26])?0: 78000); }
	if (flag || !pole3[30]) { m++; if (m<=t && m>n) do_char_log(cn, (pole3[30])?3:1, "Lab XI, Seasons Gorge    %6d\n", (pole3[30])?0: 79250); }
	if (flag || !pole3[31]) { m++; if (m<=t && m>n) do_char_log(cn, (pole3[31])?3:1, "Lab XI, Seasons Gorge    %6d\n", (pole3[31])?0: 79250); }
	if (flag || !pole4[ 0]) { m++; if (m<=t && m>n) do_char_log(cn, (pole4[ 0])?3:1, "Lab XI, Seasons Gorge    %6d\n", (pole4[ 0])?0: 79250); }
	if (flag || !pole2[ 3]) { m++; if (m<=t && m>n) do_char_log(cn, (pole2[ 3])?3:1, "Gargoyle Nest            %6d\n", (pole2[ 3])?0: 80000); }
	if (flag || !pole1[22]) { m++; if (m<=t && m>n) do_char_log(cn, (pole1[22])?3:1, "Northern Mountains       %6d\n", (pole1[22])?0: 80000); }
//	if (flag || !pole2[ 4]) { m++; if (m<=t && m>n) do_char_log(cn, (pole2[ 4])?3:1, "Ice Gargoyle Nest        %6d\n", (pole2[ 4])?0: 80000); }
	if (flag || !pole2[28]) { m++; if (m<=t && m>n) do_char_log(cn, (pole2[28])?3:1, "Underground III          %6d\n", (pole2[28])?0: 84000); }
	if (flag || !pole4[27]) { m++; if (m<=t && m>n) do_char_log(cn, (pole4[27])?3:1, "Ice Pentagram Quest      %6d\n", (pole4[27])?0: 84000); }
	if (flag || !pole2[29]) { m++; if (m<=t && m>n) do_char_log(cn, (pole2[29])?3:1, "Underground III          %6d\n", (pole2[29])?0: 90000); }
	if (flag || !pole2[ 9]) { m++; if (m<=t && m>n) do_char_log(cn, (pole2[ 9])?3:1, "Stronghold, South        %6d\n", (pole2[ 9])?0: 90000); }
//	if (flag || !pole2[11]) { m++; if (m<=t && m>n) do_char_log(cn, (pole2[11])?3:1, "Tower XX                 %6d\n", (pole2[11])?0: 90000); }
	if (flag || !pole4[28]) { m++; if (m<=t && m>n) do_char_log(cn, (pole4[28])?3:1, "Underwater Pentagram Qu  %6d\n", (pole4[28])?0: 92000); }
	if (flag || !pole4[ 1]) { m++; if (m<=t && m>n) do_char_log(cn, (pole4[ 1])?3:1, "Lab XII, Ascent Gorge    %6d\n", (pole4[ 1])?0: 93000); }
	if (flag || !pole4[ 2]) { m++; if (m<=t && m>n) do_char_log(cn, (pole4[ 2])?3:1, "Lab XII, Ascent Gorge    %6d\n", (pole4[ 2])?0: 93000); }
	if (flag || !pole4[ 3]) { m++; if (m<=t && m>n) do_char_log(cn, (pole4[ 3])?3:1, "Lab XII, Ascent Gorge    %6d\n", (pole4[ 3])?0: 93000); }
	if (flag || !pole2[30]) { m++; if (m<=t && m>n) do_char_log(cn, (pole2[30])?3:1, "Underground III          %6d\n", (pole2[30])?0: 96000); }
	if (flag || !pole1[20]) { m++; if (m<=t && m>n) do_char_log(cn, (pole1[20])?3:1, "Forbidden Tomes          %6d\n", (pole1[20])?0:100000); }
//	if (flag || !pole2[ 5]) { m++; if (m<=t && m>n) do_char_log(cn, (pole2[ 5])?3:1, "Ice Gargoyle Nest        %6d\n", (pole2[ 5])?0:100000); }
	if (flag || !pole4[29]) { m++; if (m<=t && m>n) do_char_log(cn, (pole4[29])?3:1, "Underwater Pentagram Qu  %6d\n", (pole4[29])?0:100000); }
	if (flag || !pole4[30]) { m++; if (m<=t && m>n) do_char_log(cn, (pole4[30])?3:1, "Onyx Pentagram Quest     %6d\n", (pole4[30])?0:110000); }
	if (flag || !pole2[31]) { m++; if (m<=t && m>n) do_char_log(cn, (pole2[31])?3:1, "Underground III          %6d\n", (pole2[31])?0:120000); }
//	if (flag || !pole2[ 6]) { m++; if (m<=t && m>n) do_char_log(cn, (pole2[ 6])?3:1, "Ice Gargoyle Nest        %6d\n", (pole2[ 6])?0:120000); }
//	if (flag || !pole2[12]) { m++; if (m<=t && m>n) do_char_log(cn, (pole2[12])?3:1, "Abyss V                  %6d\n", (pole2[12])?0:120000); }
	if (flag || !pole4[31]) { m++; if (m<=t && m>n) do_char_log(cn, (pole4[31])?3:1, "Onyx Pentagram Quest     %6d\n", (pole4[31])?0:120000); }
//	if (flag || !pole2[13]) { m++; if (m<=t && m>n) do_char_log(cn, (pole2[13])?3:1, "Abyss X                  %6d\n", (pole2[13])?0:150000); }
	
	//

//	if (flag || !pole1[23]) { m++; if (m<=t && m>n) do_char_log(cn, (pole1[23])?3:1, "                         %6d\n", (pole1[23])?0:     0); }
//	if (flag || !pole1[24]) { m++; if (m<=t && m>n) do_char_log(cn, (pole1[24])?3:1, "                         %6d\n", (pole1[24])?0:     0); }
//	if (flag || !pole1[25]) { m++; if (m<=t && m>n) do_char_log(cn, (pole1[25])?3:1, "                         %6d\n", (pole1[25])?0:     0); }
//	if (flag || !pole1[26]) { m++; if (m<=t && m>n) do_char_log(cn, (pole1[26])?3:1, "                         %6d\n", (pole1[26])?0:     0); }
//	if (flag || !pole1[27]) { m++; if (m<=t && m>n) do_char_log(cn, (pole1[27])?3:1, "                         %6d\n", (pole1[27])?0:     0); }
//	if (flag || !pole1[28]) { m++; if (m<=t && m>n) do_char_log(cn, (pole1[28])?3:1, "                         %6d\n", (pole1[28])?0:     0); }
//	if (flag || !pole1[29]) { m++; if (m<=t && m>n) do_char_log(cn, (pole1[29])?3:1, "                         %6d\n", (pole1[29])?0:     0); }
//	if (flag || !pole1[30]) { m++; if (m<=t && m>n) do_char_log(cn, (pole1[30])?3:1, "                         %6d\n", (pole1[30])?0:     0); }
//	if (flag || !pole1[31]) { m++; if (m<=t && m>n) do_char_log(cn, (pole1[31])?3:1, "                         %6d\n", (pole1[31])?0:     0); }
	//
	do_char_log(cn, 1, " \n");
	do_char_log(cn, 2, "Showing page %d of %d. #poles <x> to swap.\n", page, min(8,m/16+1));
	do_char_log(cn, 1, " \n");
}

void do_questlist(int cn, int flag, char *topic) // flag 1 = all, 0 = unattained
{
	int n, m, t, page = 1, ex = 0;
	int ast = 1;
	int quest1[24]={0};
	int quest2[32]={0}; // ch[cn].data[72]
	
	if (strcmp(topic, "2")==0) page = 2;
	if (strcmp(topic, "3")==0) page = 3;
	if (strcmp(topic, "4")==0) page = 4;
	
	if (ch[cn].skill[12][0]) 			quest1[ 0] = 1;	//    12	Bartering			( Jamil )
	if (ch[cn].skill[15][0]) 			quest1[ 1] = 1;	//    15	Recall				( Inga )
	if ((ch[cn].kindred & (KIN_TEMPLAR | KIN_ARCHTEMPLAR | KIN_BRAWLER))
		|| ch[cn].skill[18][0]) 		quest1[ 2] = 1;	// 18/38	* Enhance			( Sirjan )
	if ((ch[cn].kindred & (KIN_HARAKIM | KIN_ARCHHARAKIM | KIN_SUMMONER | KIN_MERCENARY | KIN_WARRIOR | KIN_SORCERER))
		|| ch[cn].skill[38][0]) 		quest1[ 3] = 1;	// 18/38	* Weapon Mastery	( Sirjan )
	if ((ch[cn].kindred & (KIN_HARAKIM | KIN_ARCHHARAKIM | KIN_SUMMONER | KIN_MERCENARY | KIN_WARRIOR | KIN_SORCERER))
		|| ch[cn].skill[41][0]) 		quest1[ 4] = 1;	// 41/19   	* Weaken			( Amity )
	if ((ch[cn].kindred & (KIN_TEMPLAR | KIN_ARCHTEMPLAR | KIN_BRAWLER))
		|| ch[cn].skill[19][0]) 		quest1[ 5] = 1;	// 41/19   	* Slow				( Amity )
	if (ch[cn].skill[13][0]) 			quest1[ 6] = 1;	//    13	Repair				( Jefferson )
	if (ch[cn].flags & CF_LOCKPICK) 	quest1[ 7] = 1;	//     * 	Lockpick			( Steven )
	if ((ch[cn].kindred & (KIN_HARAKIM | KIN_ARCHHARAKIM | KIN_SUMMONER))
		|| ch[cn].skill[32][0]) 		quest1[ 8] = 1;	// 32/25	* Immunity			( Ingrid )
	if ((ch[cn].kindred & (KIN_TEMPLAR | KIN_ARCHTEMPLAR | KIN_BRAWLER | KIN_MERCENARY | KIN_WARRIOR | KIN_SORCERER))
		|| ch[cn].skill[25][0]) 		quest1[ 9] = 1;	// 32/25	* Dispel			( Ingrid )
	if ((ch[cn].kindred & (KIN_HARAKIM | KIN_ARCHHARAKIM | KIN_SUMMONER | KIN_MERCENARY | KIN_WARRIOR | KIN_SORCERER))
		|| ch[cn].skill[33][0]) 		quest1[10] = 1;	// 33/20   	* Surround Hit		( Leopold )
	if ((ch[cn].kindred & (KIN_TEMPLAR | KIN_ARCHTEMPLAR | KIN_BRAWLER))
		|| ch[cn].skill[20][0]) 		quest1[11] = 1;	// 33/20   	* Curse				( Leopold )
	if (ch[cn].skill[26][0]) 			quest1[12] = 1;	//    26	Heal				( Gunther )
	if (ch[cn].skill[31][0]) 			quest1[13] = 1;	//    31	Sense				( Manfred )
	if (ch[cn].skill[23][0]) 			quest1[14] = 1;	//    23	Resist				( Serena )
	if (ch[cn].skill[21][0]) 			quest1[15] = 1;	//    21   	Bless				( Cirrus )
	if (ch[cn].skill[29][0]) 			quest1[17] = 1;	//    29	Rest				( Gordon )
	if ((ch[cn].kindred & (KIN_HARAKIM | KIN_ARCHHARAKIM | KIN_SUMMONER))
		|| ch[cn].skill[16][0]) 		quest1[18] = 1;	// 16/ 5	* Shield			( Edna )
	if ((ch[cn].kindred & (KIN_TEMPLAR | KIN_ARCHTEMPLAR | KIN_BRAWLER | KIN_MERCENARY | KIN_WARRIOR | KIN_SORCERER))
		|| ch[cn].skill[5][0]) 			quest1[19] = 1;	// 16/ 5	* Staff				( Edna )
	if (ch[cn].skill[22][0]) 			quest1[20] = 1;	//    22	Identify			( Nasir )
	if (ch[cn].temple_x!=HOME_START_X) 	quest1[21] = 1;	// 			Get to Aston
	if (ch[cn].flags & CF_APPRAISE) 	quest1[22] = 1; //     * 	Appraisal			( Richie )
	if (ch[cn].skill[10][0]) 			quest1[23] = 1;	//    10   	Swimming			( Lucci )	
	if (	(ch[cn].skill[35][0] && (ch[cn].kindred & (KIN_SEYAN_DU | KIN_ARCHTEMPLAR)))
		||	(ch[cn].skill[49][0] && (ch[cn].kindred & (KIN_SEYAN_DU | KIN_WARRIOR)))
		|| 	(ch[cn].skill[46][0] && (ch[cn].kindred & (KIN_SEYAN_DU | KIN_SUMMONER)))
		|| 	(ch[cn].skill[42][0] && (ch[cn].kindred & (KIN_SEYAN_DU | KIN_SORCERER)))
		|| 	(ch[cn].skill[43][0] && (ch[cn].kindred & (KIN_SEYAN_DU | KIN_ARCHHARAKIM)))
		|| 	(ch[cn].skill[7][0]  && (ch[cn].kindred & (KIN_SEYAN_DU | KIN_BRAWLER)))	)
		quest1[16] = 1;
	
	for (n=0;n<32;n++)
	{
		if (ch[cn].data[72] & (1 << n)) quest2[n] = 1;
	}
	
	if (ch[cn].temple_x==HOME_START_X) ast = 0;

	do_char_log(cn, 1, "Now listing available quests (PAGE %d):\n", page);
	do_char_log(cn, 1, " \n");
	
	n = (page-1)*16;
	t = page*16;
	m = 0;

	//															                                  "!      v .       v .   |     . v       . v      !"
	if (flag || !quest1[ 0]) { m++; ex=   100; if (m<=t && m>n) do_char_log(cn, (quest1[ 0])?3:1, " Pvt   Jamil     Bluebird Tav  Barterin  %6d\n", (quest1[ 0])?ex/4:ex); }
	if (flag || !quest1[ 1]) { m++; ex=   200; if (m<=t && m>n) do_char_log(cn, (quest1[ 1])?3:1, " PFC   Inga      First Street  Recall    %6d\n", (quest1[ 1])?ex/4:ex); }
	if (flag || !quest1[ 2]) { m++; ex=   300; if (m<=t && m>n) do_char_log(cn, (quest1[ 2])?3:1, " PFC   Sirjan    First Street  Enhance   %6d\n", (quest1[ 2])?ex/4:ex); }
	if (flag || !quest1[ 3]) { m++; ex=   300; if (m<=t && m>n) do_char_log(cn, (quest1[ 3])?3:1, " PFC   Sirjan    First Street  Weapon M  %6d\n", (quest1[ 3])?ex/4:ex); }
	if (flag || !quest1[ 4]) { m++; ex=   450; if (m<=t && m>n) do_char_log(cn, (quest1[ 4])?3:1, " LCp   Amity     Lynbore Libr  Weaken    %6d\n", (quest1[ 4])?ex/4:ex); }
	if (flag || !quest1[ 5]) { m++; ex=   450; if (m<=t && m>n) do_char_log(cn, (quest1[ 5])?3:1, " LCp   Amity     Lynbore Libr  Slow      %6d\n", (quest1[ 5])?ex/4:ex); }
	if (flag || !quest1[ 6]) { m++; ex=   600; if (m<=t && m>n) do_char_log(cn, (quest1[ 6])?3:1, " LCp   Jefferso  Second Stree  Repair    %6d\n", (quest1[ 6])?ex/4:ex); }
	if (flag || !quest1[ 7]) { m++; ex=   800; if (m<=t && m>n) do_char_log(cn, (quest1[ 7])?3:1, " LCp   Steven    Second Stree  Lockpick  %6d\n", (quest1[ 7])?ex/4:ex); }
	if (flag || !quest1[ 8]) { m++; ex=  1000; if (m<=t && m>n) do_char_log(cn, (quest1[ 8])?3:1, " Cpl   Ingrid    Castle Way    Immunity  %6d\n", (quest1[ 8])?ex/4:ex); }
	if (flag || !quest1[ 9]) { m++; ex=  1000; if (m<=t && m>n) do_char_log(cn, (quest1[ 9])?3:1, " Cpl   Ingrid    Castle Way    Dispel    %6d\n", (quest1[ 9])?ex/4:ex); }
	if (flag || !quest1[10]) { m++; ex=  1250; if (m<=t && m>n) do_char_log(cn, (quest1[10])?3:1, " Cpl   Leopold   Castle Way    Surround  %6d\n", (quest1[10])?ex/4:ex); }
	if (flag || !quest1[11]) { m++; ex=  1250; if (m<=t && m>n) do_char_log(cn, (quest1[11])?3:1, " Cpl   Leopold   Castle Way    Curse     %6d\n", (quest1[11])?ex/4:ex); }
	if (flag || !quest1[12]) { m++; ex=  1500; if (m<=t && m>n) do_char_log(cn, (quest1[12])?3:1, " Cpl   Gunther   Castle Way    Heal      %6d\n", (quest1[12])?ex/4:ex); }
	if (flag || !quest1[13]) { m++; ex=  1800; if (m<=t && m>n) do_char_log(cn, (quest1[13])?3:1, " Cpl   Manfred   Silver Avenu  Sense Ma  %6d\n", (quest1[13])?ex/4:ex); }
	if (flag || !quest1[14]) { m++; ex=  2100; if (m<=t && m>n) do_char_log(cn, (quest1[14])?3:1, " Sgt   Serena    Second Stree  Resistan  %6d\n", (quest1[14])?ex/4:ex); }
	if (flag || !quest1[15]) { m++; ex=  2450; if (m<=t && m>n) do_char_log(cn, (quest1[15])?3:1, " Sgt   Cirrus    Bluebird Tav  Bless     %6d\n", (quest1[15])?ex/4:ex); }
	if (flag || !quest1[17]) { m++; ex=  2800; if (m<=t && m>n) do_char_log(cn, (quest1[17])?3:1, " Sgt   Gordon    Shore Cresce  Rest      %6d\n", (quest1[17])?ex/4:ex); }
	if (flag || !quest1[18]) { m++; ex=  3300; if (m<=t && m>n) do_char_log(cn, (quest1[18])?3:1, " Sgt   Edna      Shore Cresce  Shield    %6d\n", (quest1[18])?ex/4:ex); }
	if (flag || !quest1[19]) { m++; ex=  3300; if (m<=t && m>n) do_char_log(cn, (quest1[19])?3:1, " Sgt   Edna      Shore Cresce  Staff     %6d\n", (quest1[19])?ex/4:ex); }
	if (flag || !quest1[20]) { m++; ex=  3800; if (m<=t && m>n) do_char_log(cn, (quest1[20])?3:1, " Sgt   Nasir     Shore Cresce  Identify  %6d\n", (quest1[20])?ex/4:ex); }
	if (flag || !quest1[21]) { m++; ex=  8000; if (m<=t && m>n) do_char_log(cn, (quest1[21])?3:1, " SSg   Guard Ca  South Aston   Gold      %6d\n", (quest1[21])?ex/4:ex); }
	//															                                  "!      ^ .       ^ .   |     . ^       . ^      !"
	if (ast)
	{
	//														                                      "!      v .       v .   |     . v       . v      !"
	if (flag || !quest2[ 0]) { m++; ex=  4000; if (m<=t && m>n) do_char_log(cn, (quest2[ 0])?3:1, " SSg   April     Marble Lane   Potion    %6d\n", (quest2[ 0])?ex/4:ex); }
	if (flag || !quest1[22]) { m++; ex=  5000; if (m<=t && m>n) do_char_log(cn, (quest1[22])?3:1, " SSg   Richie    Marble Lane   Appraisa  %6d\n", (quest1[22])?ex/4:ex); }
	if (flag || !quest2[ 1]) { m++; ex=  6000; if (m<=t && m>n) do_char_log(cn, (quest2[ 1])?3:1, " SSg   Alphonse  Rose Street   Ring      %6d\n", (quest2[ 1])?ex/4:ex); }
	if (flag || !quest2[11]) { m++; ex=  8000; if (m<=t && m>n) do_char_log(cn, (quest2[11])?3:1, " 1Sg   Robin     South End     Potion    %6d\n", (quest2[11])?ex/4:ex); }
	if (flag || !quest2[ 2]) { m++; ex=  8000; if (m<=t && m>n) do_char_log(cn, (quest2[ 2])?3:1, " 1Sg   Cherri    Merchant's W  Amulet    %6d\n", (quest2[ 2])?ex/4:ex); }
	if (flag || !quest2[ 3]) { m++; ex= 10000; if (m<=t && m>n) do_char_log(cn, (quest2[ 3])?3:1, " SgM   Rocky     Merchant's W  Ring      %6d\n", (quest2[ 3])?ex/4:ex); }
	if (flag || !quest2[ 4]) { m++; ex= 12000; if (m<=t && m>n) do_char_log(cn, (quest2[ 4])?3:1, " SgM   Bradley   Astonia Peni  Belt      %6d\n", (quest2[ 4])?ex/4:ex); }
	if (flag || !quest2[13]) { m++; ex= 14000; if (m<=t && m>n) do_char_log(cn, (quest2[13])?3:1, " SgM   Oscar     Temple Stree  Amulet    %6d\n", (quest2[13])?ex/4:ex); }
	if (flag || !quest2[ 6]) { m++; ex= 15000; if (m<=t && m>n) do_char_log(cn, (quest2[ 6])?3:1, "2Lieu  Elric     Merchant's W  Potion    %6d\n", (quest2[ 6])?ex/4:ex); }
	if (flag || !quest2[14]) { m++; ex= 16000; if (m<=t && m>n) do_char_log(cn, (quest2[14])?3:1, "2Lieu  Castor    Marble Lane   Book      %6d\n", (quest2[14])?ex/4:ex); }
	if (flag || !quest2[15]) { m++; ex= 20000; if (m<=t && m>n) do_char_log(cn, (quest2[15])?3:1, "2Lieu  Grover    Marble Lane   Amulet    %6d\n", (quest2[15])?ex/4:ex); }
	if (flag || !quest2[ 5]) { m++; ex= 20000; if (m<=t && m>n) do_char_log(cn, (quest2[ 5])?3:1, "1Lieu  Roxie     Temple Stree  Amulet    %6d\n", (quest2[ 5])?ex/4:ex); }
	if (flag || !quest1[23]) { m++; ex= 24000; if (m<=t && m>n) do_char_log(cn, (quest1[23])?3:1, "1Lieu  Lucci     Temple Stree  Swimming  %6d\n", (quest1[23])?ex/4:ex); }
	if (flag || !quest2[16]) { m++; ex= 25000; if (m<=t && m>n) do_char_log(cn, (quest2[16])?3:1, "1Lieu  Regal     Winding Vall  Book      %6d\n", (quest2[16])?ex/4:ex); }
	if (flag || !quest2[10]) { m++; ex= 26000; if (m<=t && m>n) do_char_log(cn, (quest2[10])?3:1, "Captn  Donna     Merchant's W  Tarot     %6d\n", (quest2[10])?ex/4:ex); }
	if (flag || !quest2[ 9]) { m++; ex= 30000; if (m<=t && m>n) do_char_log(cn, (quest2[ 9])?3:1, "Captn  Gomez     Temple Stree  Tarot     %6d\n", (quest2[ 9])?ex/4:ex); }
	if (flag || !quest2[ 7]) { m++; ex= 32000; if (m<=t && m>n) do_char_log(cn, (quest2[ 7])?3:1, "Major  Marline   Marble Lane   Tarot     %6d\n", (quest2[ 7])?ex/4:ex); }
	if (flag || !quest2[12]) { m++; ex= 20000; if (m<=t && m>n) do_char_log(cn, (quest2[12])?3:1, "LtCol  Monica    South End     Potion    %6d\n", (quest2[12])?ex/4:ex); }
	if (flag || !quest2[ 8]) { m++; ex= 32000; if (m<=t && m>n) do_char_log(cn, (quest2[ 8])?3:1, "LtCol  Rufus     Temple Stree  Tarot     %6d\n", (quest2[ 8])?ex/4:ex); }
	if (flag || !quest2[17]) { m++; ex= 40000; if (m<=t && m>n) do_char_log(cn, (quest2[17])?3:1, "Colnl  Shera     Bulwark Aven  Cloak     %6d\n", (quest2[17])?ex/4:ex); }
	if (flag || !quest2[18]) { m++; ex= 45000; if (m<=t && m>n) do_char_log(cn, (quest2[18])?3:1, "Colnl  Tacticia  West Gate     Spear     %6d\n", (quest2[18])?ex/4:ex); }
	if (flag || !quest2[19]) { m++; ex= 50000; if (m<=t && m>n) do_char_log(cn, (quest2[19])?3:1, "Genrl  Danica    South End     Potion    %6d\n", (quest2[19])?ex/4:ex); }
	//                                                                                            "!      ^ .       ^ .   |     . ^       . ^      !"
	}
	if (flag || !quest1[16])
	{
	//														                                      "!      v .       v .   |     . v       . v      !"
		if ((ch[cn].kindred & KIN_ARCHTEMPLAR) 	|| ((ch[cn].kindred & KIN_SEYAN_DU) && !quest1[16]))
	                         { m++; ex= 50000; if (m<=t && m>n) do_char_log(cn, (quest1[16])?3:1, "BrGen  Jamie     Bulwark Aven  * Warcry  %6d\n", (quest1[16])?ex/4:ex); }
		if ((ch[cn].kindred & KIN_WARRIOR) 		|| ((ch[cn].kindred & KIN_SEYAN_DU) && !quest1[16]))
	                         { m++; ex= 50000; if (m<=t && m>n) do_char_log(cn, (quest1[16])?3:1, "BrGen  Terri     Bulwark Aven  * Leap    %6d\n", (quest1[16])?ex/4:ex); }
		if ((ch[cn].kindred & KIN_SUMMONER) 	|| ((ch[cn].kindred & KIN_SEYAN_DU) && !quest1[16]))
	                         { m++; ex= 50000; if (m<=t && m>n) do_char_log(cn, (quest1[16])?3:1, "BrGen  Roger     Bulwark Aven  * Shadow  %6d\n", (quest1[16])?ex/4:ex); }
		if ((ch[cn].kindred & KIN_SORCERER) 	|| ((ch[cn].kindred & KIN_SEYAN_DU) && !quest1[16]))
	                         { m++; ex= 50000; if (m<=t && m>n) do_char_log(cn, (quest1[16])?3:1, "BrGen  Boris     Bulwark Aven  * Poison  %6d\n", (quest1[16])?ex/4:ex); }
		if ((ch[cn].kindred & KIN_ARCHHARAKIM) 	|| ((ch[cn].kindred & KIN_SEYAN_DU) && !quest1[16]))
	                         { m++; ex= 50000; if (m<=t && m>n) do_char_log(cn, (quest1[16])?3:1, "BrGen  Kaleigh   Bulwark Aven  * Pulse   %6d\n", (quest1[16])?ex/4:ex); }
		if ((ch[cn].kindred & KIN_BRAWLER) 	|| ((ch[cn].kindred & KIN_SEYAN_DU) && !quest1[16]))
	                         { m++; ex= 50000; if (m<=t && m>n) do_char_log(cn, (quest1[16])?3:1, "BrGen  Ellis     Bulwark Aven  * Razor   %6d\n", (quest1[16])?ex/4:ex); }
	//                                                                                            "!      ^ .       ^ .   |     . ^       . ^      !"
	}
	
	do_char_log(cn, 1, " \n");
	do_char_log(cn, 2, "Showing page %d of %d. #quest <x> to swap.\n", page, min(4,m/16+1));
	do_char_log(cn, 1, " \n");
}

void do_listbuffs(int cn)
{
	int n, in, flag = 0;

	do_char_log(cn, 1, "Your active buffs and debuffs:\n");
	do_char_log(cn, 1, " \n");
	for (n = 0; n<MAXBUFFS; n++)
	{
		if ((in = ch[cn].spell[n])!=0)
		{
			do_char_log(cn, 1, "%s for %dm %ds power of %d\n",
				bu[in].name, bu[in].active / (18 * 60), (bu[in].active / 18) % 60, bu[in].power);
			flag = 1;
		}
	}
	if (!flag)
	{
		do_char_log(cn, 1, "None.\n");
	}
	do_char_log(cn, 1, " \n");
}

// -------- //

void do_afk(int cn, char *msg)
{
	if (ch[cn].data[CHD_AFK])
	{
		do_char_log(cn, 1, "Back.\n");
		ch[cn].data[CHD_AFK] = 0;
	}
	else
	{
		ch[cn].data[CHD_AFK] = 1;
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
	
	ch[cn].data[CHD_ALLOW] = co;
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
			return( 1);
		}
	}

	return(0);
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

		switch(max(points2rank(ch[cn].points_tot), points2rank(ch[co].points_tot)))
		{
		case 21:
			allow = GROUP_RANGE+1;
			break;
		case 22:
			allow = GROUP_RANGE+2;
			break;
		case 23:
			allow = GROUP_RANGE+3;
			break;
		default:
			allow = GROUP_RANGE;
			break;
		}

		if (abs(tmp = rankdiff(cn, co))>allow)
		{
			do_char_log(cn, 0, "Sorry, you cannot group with %s; he is %d ranks %s you. Maximum distance is %d.\n",
			            ch[co].name, abs(tmp), tmp>0 ? "above" : "below", GROUP_RANGE);
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
		//            tmp / 100, tmp % 100, (tmp * 18) / 100, (tmp * 18) % 100);
		do_char_log(cn, 1, "You currently have %d items in your depot.\n", tmp);
	}

	if (ch[cn].depot_sold)
	{
		do_char_log(cn, 1, "The bank sold %d items from your depot to cover the costs.\n", ch[cn].depot_sold);
		ch[cn].depot_sold = 0;
	}

	if (ch[cn].depot_cost)
	{
		do_char_log(cn, 1, "%dG %dS were deducted from your bank account as rent for your depot.\n",
		            ch[cn].depot_cost / 100, ch[cn].depot_cost % 100);
		ch[cn].depot_cost = 0;
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
		return( 0);
	}

	if (!in && !in2)
	{
		return( 0);
	}

	if (in && !in2)
	{
		return( -1);
	}
	if (!in && in2)
	{
		return( 1);
	}

	for (o = order; *o; o++)
	{
		switch(*o)
		{
		case 'w':
			if ((it[in].flags & IF_WEAPON) && !(it[in2].flags & IF_WEAPON))
			{
				return( -1);
			}
			if (!(it[in].flags & IF_WEAPON) && (it[in2].flags & IF_WEAPON))
			{
				return( 1);
			}
			break;

		case 'a':
			if ((it[in].flags & IF_ARMOR) && !(it[in2].flags & IF_ARMOR))
			{
				return( -1);
			}
			if (!(it[in].flags & IF_ARMOR) && (it[in2].flags & IF_ARMOR))
			{
				return( 1);
			}
			break;

		case 'p':
			if ((it[in].flags & IF_USEDESTROY) && !(it[in2].flags & IF_USEDESTROY))
			{
				return( -1);
			}
			if (!(it[in].flags & IF_USEDESTROY) && (it[in2].flags & IF_USEDESTROY))
			{
				return( 1);
			}
			break;

		case 'h':
			if (it[in].hp[0]>it[in2].hp[0])
			{
				return( -1);
			}
			if (it[in].hp[0]<it[in2].hp[0])
			{
				return( 1);
			}
			break;

		case 'e':
			if (it[in].end[0]>it[in2].end[0])
			{
				return( -1);
			}
			if (it[in].end[0]<it[in2].end[0])
			{
				return( 1);
			}
			break;

		case 'm':
			if (it[in].mana[0]>it[in2].mana[0])
			{
				return( -1);
			}
			if (it[in].mana[0]<it[in2].mana[0])
			{
				return( 1);
			}
			break;

		case 'v':
			if (it[in].value>it[in2].value)
			{
				return( -1);
			}
			if (it[in].value<it[in2].value)
			{
				return( 1);
			}
			break;

		default:
			break;

		}
	}
	
	// Sort soulstones
	if (it[in].driver == 68 && it[in2].driver == 68)
	{
		if (it[in].data[1] > it[in2].data[1]) return -1;
		else if (it[in].data[1] < it[in2].data[1]) return 1;
		else return 0;
	}


	// fall back to sort by value
	if (it[in].value>it[in2].value)
	{
		return( -1);
	}
	if (it[in].value<it[in2].value)
	{
		return( 1);
	}

	if (it[in].temp>it[in2].temp)
	{
		return( 1);
	}
	if (it[in].temp<it[in2].temp)
	{
		return( -1);
	}

	return(0);
}

void do_sort_depot(int cn, char *arg)
{
	if (IS_BUILDING(cn))
	{
		do_char_log(cn, 1, "Not in build-mode, dude.");
		return;
	}

	order = arg;
	
	qsort(ch[cn].depot, 62, sizeof(int), qsort_proc);

	do_update_char(cn);
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
	for (n=0;n<40;n++) 
	{
		it[ch[cn].item[n]].flags &= ~IF_ITEMLOCK;
		if (ch[cn].item_lock[n])
		{
			it[ch[cn].item[n]].flags |= IF_ITEMLOCK;
		}
	}
	
	qsort(ch[cn].item, 40, sizeof(int), qsort_proc);

	do_update_char(cn);
}

void do_depot(int cn, char *topic)
{
	int n, m, co;
	int temp = 0;
	
	if (strcmp(topic, "")!=0)
	{
		for (n = 1; n<MAXCHARS; n++)
		{
			if (ch[n].used!=USE_EMPTY && strcmp(toupper(topic), toupper(ch[n].name))==0)	// Character with this name exists
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
				if (ch[cn].data[m]==0) continue;
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
	if (val<1)
	{
		do_char_log(cn, 0, "That's not very much, is it?\n");
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

	for (m = 0; m<40; m++) // looking for free space in inventory
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
		return( 0);
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
	if (globs->ticker - ch[cn].data[67]<TICKS * 60 && !(ch[cn].kindred & KIN_PURPLE))
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
			do_char_log(cn, 2, "Player killing flag set. May you enjoy the killing.\n");
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

void do_make_soulstone(int cn, int cexp)
{
	int in, rank;

	in = god_create_item(1146);
	if (in)
	{
		rank = points2rank(cexp);
		
		if (rank > 18)
			rank = 18;

		sprintf(it[in].name, "Soulstone");
		sprintf(it[in].reference, "soulstone");
		sprintf(it[in].description, "Level %d soulstone, holding %d exp.", rank, cexp);

		it[in].data[0] = rank;
		it[in].data[1] = cexp;
		it[in].temp = 0;
		it[in].driver = 68;

		god_give_char(in, cn);
	}
}

void do_become_skua(int cn)
{
	int days;

	if (!(ch[cn].kindred & KIN_PURPLE))
	{
		do_char_log(cn, 0, "Hmm. Nothing happened.\n");
		return;
	}
	else
	{
		days = (globs->ticker - ch[cn].data[CHD_ATTACKTIME]) / (60 * TICKS) / 60 / 24;
		if (days < 30)
		{
			do_char_log(cn, 0, "You have %u days of penance left.\n", 30 - days);
			return;
		}
		
		if ((ch[cn].x >= 499 && ch[cn].x <= 531 && ch[cn].y >= 504 && ch[cn].y <= 520)
		 || (ch[cn].x >= 505 && ch[cn].x <= 519 && ch[cn].y >= 492 && ch[cn].y <= 535))
		{
			do_char_log(cn, 0, " \n");
			do_char_log(cn, 0, "You feel the presence of a god again. You feel protected.  Your desire to kill subsides.\n");
			do_char_log(cn, 0, " \n");
			do_char_log(cn, 0, "\"THE GOD SKUA WELCOMES YOU, MORTAL! YOUR BONDS OF SLAVERY ARE BROKEN!\"\n");
			do_char_log(cn, 0, " \n");
			do_char_log(cn, 2, "Player killing flag cleared.\n");
			do_char_log(cn, 0, " \n");

			ch[cn].kindred &= ~KIN_PURPLE;
			ch[cn].data[CHD_ATTACKTIME] = 0;
			ch[cn].data[CHD_ATTACKVICT] = 0;
			if (ch[cn].temple_x!=HOME_START_X)
			{
				ch[cn].temple_x = HOME_TEMPLE_X;
				ch[cn].temple_y = HOME_TEMPLE_Y;
			}
			chlog(cn, "Converted to skua. (%u days elapsed)", days);
			fx_add_effect(5, 0, ch[cn].x, ch[cn].y, 0);
		}
		else
		{
			do_char_log(cn, 0, "It seems like this only works while inside the Temple of Skua.\n");
		}
	}
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
		if (prefix(cmd, "area") && ch[cn].skill[SK_PROX][0])
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
		if (prefix(cmd, "announce") && f_gius)
		{
			do_announce(cn, cn, "%s\n", args[0]);
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
			do_listbuffs(cn);
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
		if (prefix(cmd, "cap") && f_g)
		{
			set_cap(cn, atoi(arg[1]));
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
			god_create(cn, atoi(arg[1]));
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
		if (prefix(cmd, "gcmax") && (ch[cn].kindred & (KIN_HARAKIM | KIN_SEYAN_DU | KIN_SUMMONER | KIN_ARCHHARAKIM)))
		{
			do_listgcmax(cn);
			return;
		}
		;
		if (prefix(cmd, "gctome") && !f_m)
		{
			do_gctome(cn);
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
		if (prefix(cmd, "kwai") && (ch[cn].kindred & KIN_SEYAN_DU))
		{
			do_showkwai(cn, arg[1]);
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
			god_raise_char(cn, dbatoi_self(cn, arg[1]), atoi(arg[2]));
			return;
		}
		;
		if (prefix(cmd, "recall") && f_giu)
		{
			god_goto(cn, cn, "512", "512");
			return;
		}
		;
		if (prefix(cmd, "resetnpcs") && f_gg)
		{
			god_reset_npcs(cn);
			return;
		}
		;
		if (prefix(cmd, "resetplayer") && f_gg)
		{
			god_reset_player(cn, atoi(arg[1]));
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
		if (prefix(cmd, "sense") && !f_m)
		{
			do_sense(cn);
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
		if (prefix(cmd, "send") && f_g)
		{
			god_goto(cn, dbatoi(arg[1]), arg[2], arg[3]);
			return;
		}
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
			do_sort_depot(cn, arg[1]);
			return;
		}
		;
		if (prefix(cmd, "soulstone") && f_g)
		{
			do_make_soulstone(cn, atoi(arg[1]));
			return;
		}
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
		if (prefix(cmd, "summon") && f_g)
		{
			god_summon(cn, arg[1], arg[2], arg[3]);
			return;
		}
		;
		if (prefix(cmd, "sword"))
		{
			do_listweapons(cn, "sword");
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
			god_thrall(cn, arg[1], arg[2]);
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

	if (strcmp(text, "help")==0&&points2rank(ch[cn].points_tot)<4)
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
	else if (n>AT_CAP)
	{
		n = AT_CAP;
	}
	
	ch[cn].attrib[z][4] = (n >> 8) & 0xFF;
	ch[cn].attrib[z][5] = n & 0xFF;
}
int get_skill_score(int cn, int n)
{
	return ( (ch[cn].skill[n][4] << 8) | ch[cn].skill[n][5] );
}
void set_skill_score(int cn, int z, int n)
{
	if (n<1)
	{
		n = 1;
	}
	else if (n>AT_CAP)
	{
		n = AT_CAP;
	}
	
	ch[cn].skill[z][4] = (n >> 8) & 0xFF;
	ch[cn].skill[z][5] = n & 0xFF;
}

int get_fight_skill(int cn, int skill[50])
{
	int in;
	
	in = ch[cn].worn[WN_RHAND];
	
	if (!in || it[in].flags & IF_WP_CLAW) 
	{
		return min(AT_CAP, skill[SK_HAND]);
	}

	if (it[in].flags & IF_WP_SWORD)
	{
		return min(AT_CAP, skill[SK_SWORD]);
	}
	
	if ((it[in].flags & IF_WP_DAGGER) && (it[in].flags & IF_WP_STAFF)) // Spear
	{
		return min(AT_CAP, skill[SK_DAGGER] > skill[SK_STAFF] ? skill[SK_DAGGER] : skill[SK_STAFF]);
	}
	
	if (it[in].flags & IF_WP_DAGGER)
	{
		return min(AT_CAP, skill[SK_DAGGER]);
	}
	if (it[in].flags & IF_WP_STAFF)
	{
		return min(AT_CAP, skill[SK_STAFF]);
	}
	
	if ((it[in].flags & IF_WP_AXE) && (it[in].flags & IF_WP_TWOHAND)) // Greataxe
	{
		return min(AT_CAP, skill[SK_AXE] > skill[SK_TWOHAND] ? skill[SK_AXE] : skill[SK_TWOHAND]);
	}
	if (it[in].flags & IF_WP_AXE)
	{
		return min(AT_CAP, skill[SK_AXE]);
	}
	if (it[in].flags & IF_WP_TWOHAND)
	{
		return min(AT_CAP, skill[SK_TWOHAND]);
	}

	return min(AT_CAP, skill[SK_HAND]);
}

// Combat Mastery, Dual Wield and Shield skill checks
int get_combat_skill(int cn, int skill[50], int flag)
{
	int power, bonus=0;

	if (flag)
	{
		power = min(AT_CAP, skill[SK_DUAL]);
		
		if (ch[cn].kindred & (KIN_MERCENARY | KIN_WARRIOR | KIN_SORCERER | KIN_SEYAN_DU))
		{
			bonus = power/6;
		}
		else
		{	
			bonus = power/12;
		}
	}
	else
	{
		power = min(AT_CAP, skill[SK_SHIELD]);
		
		if (ch[cn].kindred & (KIN_MERCENARY | KIN_WARRIOR | KIN_SORCERER | KIN_SEYAN_DU))
		{
			bonus = power/6;
		}
		else
		{	
			bonus = power/12;
		}
	}

	return (bonus);
}
int get_offhand_skill(int cn, int skill[50], int flag)
{
	int in, in2; 
	
	in  = ch[cn].worn[WN_LHAND];
	in2 = ch[cn].worn[WN_BELT];
	
	// Belt - Black Belt :: Shield parry bonus while offhand is empty
	if (in2 && it[in2].temp == IT_TW_BBELT && !in && !flag)
	{
		return get_combat_skill(cn, skill, flag);
	}
	
	// No Gear? No bonus
	if (!in || 
		(flag && !(it[in].flags & IF_OF_DUALSW)) || 
		(!flag && !(it[in].flags & IF_OF_SHIELD)))
	{
		return 0;
	}
	
	// Otherwise...
	return get_combat_skill(cn, skill, flag);
}

// put in an item, see if we're wearing it in a charm slot.
int get_tarot(int cn, int in)
{
	int ch1, ch2;
	
	ch1 = ch[cn].worn[WN_CHARM];
	ch2 = ch[cn].worn[WN_CHARM2];
	
	if ((ch1 && it[ch1].temp==in) || (ch2 && it[ch2].temp==in)) 
		return 1;
	
	return 0;
}

// put in an item, see if we're wearing it in the left-hand slot.
int get_book(int cn, int in)
{
	int in2;
	
	in2 = ch[cn].worn[WN_LHAND];
	
	if (in2 && it[in2].temp == in) 
		return 1;
	
	return 0;
}

// put in an item, see if we're wearing it in the amulet slot
int get_neck(int cn, int in)
{
	int in2;
	
	in2 = ch[cn].worn[WN_NECK];
	
	if (in2 && it[in2].temp == in) 
		return 1;
	
	return 0;
}

// put in an item and a slot, see if we're wearing that in any slot
int get_gear(int cn, int in)
{
	int n, in2;
	
	for (n = 0; n <= WN_CHARM2; n++)
	{
		if ((in2 = ch[cn].worn[n]) && it[in2].temp == in)
			return 1;
	}
	
	return 0;
}

//

// For examining a corpse for special stuff at a glance with Sense Magic.
// msg must be a do_char_log() format string like "you see %s in the corpse.\n".
void do_ransack_corpse(int cn, int co, char *msg)
{
	int in, n, sm, sm2;
	char dropped[50];
	
	chlog(cn, "Trying for sense magic message");
	
	sm  = get_skill_score(cn, SK_SENSE);
	sm2 = sm-100;

	// Check for unique weapon in hands
	if ((in = ch[co].worn[WN_RHAND]) && is_unique(in) && sm > RANDOM(200))
	{
		if (sm2>RANDOM(200))
			sprintf(dropped, "a unique %s", it[in].name);
		else
			strcpy(dropped, "a unique weapon");
		do_char_log(cn, 0, msg, dropped);
	}
	if ((in = ch[co].worn[WN_LHAND]) && is_unique(in) && sm > RANDOM(200))
	{
		if (sm2>RANDOM(200))
			sprintf(dropped, "a unique %s", it[in].name);
		else
			strcpy(dropped, "a unique shield");
		do_char_log(cn, 0, msg, dropped);
	}
	// Check ring slots for soulstones
	if ((in = ch[co].worn[WN_LRING]) && is_soulstone(in) && sm > RANDOM(200))
	{
		do_char_log(cn, 0, msg, "a soulstone");
	}
	if ((in = ch[co].worn[WN_RRING]) && is_soulstone(in) && sm > RANDOM(200))
	{
		do_char_log(cn, 0, msg, "a soulstone");
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
	if ((in = ch[co].worn[WN_LRING]) && sm > RANDOM(200))
	{
		if (sm2>RANDOM(200))
			sprintf(dropped, "a %s", it[in].name);
		else
			strcpy(dropped, "a magical ring");
		do_char_log(cn, 0, msg, dropped);
	}
	if ((in = ch[co].worn[WN_RRING]) && sm > RANDOM(200))
	{
		if (sm2>RANDOM(200))
			sprintf(dropped, "a %s", it[in].name);
		else
			strcpy(dropped, "a magical ring");
		do_char_log(cn, 0, msg, dropped);
	}
	// Check for items in inventory
	/* SH 30.06.00 */
	for (n = 0; n<40; n++)
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
				if (it[in].placement & PL_WEAPON)
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
		if (is_soulstone(in) && sm > RANDOM(200))
		{
			do_char_log(cn, 0, msg, "a soulstone");
			continue;
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
		if ((it[in].placement & PL_RING) && sm > RANDOM(200))
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
	
	chlog(cn, "Sense magic message sent OK");
}

// note: cn may be zero!!
void do_char_killed(int cn, int co, int pentsolve)
{
	int n, in, x, y, temp = 0, m, tmp, tmpg, wimp, cc = 0, fn, r1, r2;
	unsigned long long mf;
	unsigned char buf[3];

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
	else
	{
		do_area_sound(co, 0, ch[co].x, ch[co].y, ch[co].sound + 2);
		char_play_sound(co, ch[co].sound + 2, -150, 0);
	}

	// cleanup for ghost companions
	if (IS_COMP_TEMP(co))
	{
		cc = ch[co].data[63];
		if (IS_SANECHAR(cc) && (ch[cc].data[64] == co))
		{
			ch[cc].data[64] = 0;
		}
		ch[co].data[63] = 0;
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

		// becoming purple
		if (ch[co].temp==CT_PRIEST)   // add all other priests of the purple one here...
		{
			if (ch[cn].kindred & KIN_PURPLE)
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
		r1 = points2rank(ch[cn].points_tot);
		r2 = points2rank(ch[co].points_tot);

		if (abs(r1 - r2)<3) // aprox. own rank
		{
			ch[cn].data[24]++;                              // overall counter
		}
		else if (r2>r1)     // above own rank
		{
			ch[cn].data[25]++;
		}
		else     // below own rank
		{
			ch[cn].data[23]++;
		}
		
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
	if (cn && cn!=co && !(ch[cn].flags & (CF_PLAYER)) && (cc = ch[cn].data[63])!=0 && (ch[cc].flags & (CF_PLAYER)))
	{
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
		r1 = points2rank(ch[cc].points_tot);
		r2 = points2rank(ch[co].points_tot);

		if (abs(r1 - r2)<3) // aprox. own rank
		{
			ch[cc].data[24]++;                              // overall counter
		}
		else if (r2>r1)     // above own rank
		{
			ch[cc].data[25]++;
		}
		else     // below own rank
		{
			ch[cc].data[23]++;
		}
		
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

	if (mf & MF_ARENA)
	{
		// Arena death : full save, keep everything
		wimp = 2;
	}
	else if (!(ch[co].kindred & KIN_PURPLE))
	{
		// Skua death : Skua save, lose exp and gold but keep items
		wimp = 1;
	}
	else
	{
		// Purple death : Purple save, lose items but keep exp and gold
		wimp = 0;
	}

	// drop items and money in original place
	if (ch[co].flags & (CF_PLAYER))
	{
		// Newbie death : full save, keep everything
		if (points2rank(ch[co].points_tot)<5)
		{
			wimp = 2;
			do_char_log(co, 0, "You would have dropped your items, but seeing you're still inexperienced the gods kindly returned them. Stay safe!\n");
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
		
		for (n = 0; n<40; n++)
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
			if (wimp==0)
			{
				ch[co].item[n] = 0;
				it[in].carried = cc;
				chlog(co, "Dropped %s (t=%d) in Grave", it[in].name, it[in].temp);
				tmpg = 1;
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
				if (wimp==0)
				{
					ch[co].citem = 0;
					it[in].carried = cc;
					chlog(co, "Dropped %s (t=%d) in Grave", it[in].name, it[in].temp);
					tmpg = 1;
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
				tmpg = 1;
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

		if (!(ch[co].flags & CF_GOD) && wimp==1) // Skua death
		{
			// Changed to negative exp
			tmp = (ch[co].points_tot - rank2points(points2rank(ch[co].points_tot)-1))/20;
			
			if (ch[co].gold)
			{
				tmpg = ch[co].gold;
				ch[co].gold = 0;
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
		else if (!(ch[co].flags & CF_GOD) && wimp==0) // Purple death
		{
			if (tmpg)
			{
				do_char_log(co, 0, "You dropped your items and equipped gear where you died.\n");
			}
			else
			{
				do_char_log(co, 0, "A pitiful death.\n");
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
		if (IS_SANECHAR(cc = ch[cn].data[63])!=0 && (ch[cc].flags & (CF_PLAYER)))
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
		if (IS_SANEPLAYER(cn))
		{
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
		for (z = 0; z<40; z++)
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
	fn = fx_add_effect(3, 0, ch[co].x, ch[co].y, co);
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
	if (ch[cn].escape_timer > TICKS*2) 
	{ 
		for (m = 0; m<4; m++)
		{
			ch[cn].enemy[m] = 0;
		}
		remove_enemy(cn);
		return 1; 
	}
	if (ch[cn].escape_timer) { return 0; }

	per  = 0;

	for (m = 0; m<4; m++)
	{
		if ((co = ch[cn].enemy[m])!=0)
		{
			per += get_skill_score(co, SK_PERCEPT);
		}
	}
	ste   = get_skill_score(cn, SK_STEALTH);

	//chance = 9 + (per - ste);
	chance=ste*15/per;
	
	if (chance< 0) chance =   0; 
	if (chance>18) chance =  18; 
	
	if (RANDOM(20)<=chance)
	{
		ch[cn].escape_timer = TICKS*3;
		do_char_log(cn, 1, "You manage to escape!\n");
		for (m = 0; m<4; m++)
		{
			ch[cn].enemy[m] = 0;
		}
		remove_enemy(cn);
		return 1;
	}

	ch[cn].escape_timer = TICKS;
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
			if ((co = ch[cn].data[63])!=0) // we are the follower of someone
			{
				do_give_exp(cn, p, 0, rank);
				if ((master = ch[cn].data[63])>0 && master<MAXCHARS && ch[master].points_tot>ch[cn].points_tot)
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
			if ((master = ch[cn].data[63])>0 && master<MAXCHARS && ch[master].points_tot>ch[cn].points_tot)
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
			do_char_log(cn, 2, "You get %d experience points.\n", p);

			//do_char_log(cn, 1, "Experience until next rank: %d\n", points_tolevel(ch[cn].points_tot));
			do_notify_char(cn, NT_GOTEXP, p, 0, 0, 0);
			do_update_char(cn);
			do_check_new_level(cn);
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
			if ((co = ch[cn].data[63])!=0 && IS_SANEPLAYER(co))
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

int try_lucksave(int cn)
{
	if ((ch[cn].luck>=100 && RANDOM(10000)<5000 + ch[cn].luck) /* && (ch[cn].kindred & KIN_PURPLE) */ )
		return 1;
	
	return 0;
}

void do_lucksave(int cn, char *deathtype)
{
	ch[cn].a_hp  = ch[cn].hp[5] * 500;
	ch[cn].luck /= 2;
	do_char_log(cn, 0, "A god reached down and saved you from the %s. You must have done the gods a favor sometime in the past!\n", deathtype);
		do_area_log(cn, 0, ch[cn].x, ch[cn].y, 0, "A god reached down and saved %s from the %s.\n", ch[cn].reference, deathtype);
	fx_add_effect(6, 0, ch[cn].x, ch[cn].y, 0);
	god_transfer_char(cn, ch[cn].temple_x, ch[cn].temple_y);
	fx_add_effect(6, 0, ch[cn].x, ch[cn].y, 0);

	chlog(cn, "Saved by the Gods (new luck=%d)", ch[cn].luck);
	ch[cn].data[44]++;
}

// dmg types: 0=normal 1=blast 2=hw/soku 3=gethit 4=surround 5=cleave 6=pulse 7=razor
// returns actual damage done
int do_hurt(int cn, int co, int dam, int type)
{
	int tmp = 0, n, in, rank = 0, noexp = 0, halfexp = 0;
	unsigned long long mf;
	int hp_dam = 0, end_dam = 0, mana_dam = 0;
	int scorched = 0, guarded = 0;
	int offpot = 0, defpot = 0;

	mf = map[ch[co].x + ch[co].y * MAPX].flags;
	if (cn)
	{
		mf |= map[ch[cn].x + ch[cn].y * MAPX].flags;
	}

	if (ch[co].flags & CF_BODY)
	{
		return( 0);
	}

	if (ch[co].flags & (CF_PLAYER))
	{
		item_damage_armor(co, dam);
	}

	if (!(ch[cn].flags & CF_PLAYER) && ch[cn].data[63]==co)
	{
		noexp = 1;
	}

	// no exp for killing players
	if (ch[co].flags & CF_PLAYER)
	{
		noexp = 1;
	}

	// half exp for killing ghosts
	if (IS_COMP_TEMP(co) && !(ch[co].flags & CF_THRALL))
	{
		halfexp = 1;
	}
	
	// Loop to look for Magic Shield so we can damage it
	// Also look for Scorch so Blast does more damage!
	for (n = 0; n<MAXBUFFS; n++)
	{
		if ((in = ch[co].spell[n])!=0)
		{
			if (bu[in].temp==SK_MSHIELD)
			{
				if (ch[co].kindred & KIN_SEYAN_DU) 
					tmp = bu[in].active / 1536 + 1;
				else
					tmp = bu[in].active / 1024 + 1;
				tmp = (dam + tmp - ch[co].armor) * 5;

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
						if (ch[co].kindred & KIN_SEYAN_DU) 
							bu[in].armor[1] = bu[in].active / 1536 + 1;
						else
							bu[in].armor[1] = bu[in].active / 1024 + 1;
						bu[in].power = bu[in].active / 256;
						do_update_char(co);
					}
				}
			}
			if (bu[in].temp==SK_SCORCH)	scorched = bu[in].power;
			if (bu[in].temp==SK_GUARD)	guarded = bu[in].power;
			
			if (bu[in].temp > 100 && bu[in].data[3]==IT_POT_DEF)	defpot = 1;
		}
		if ((in = ch[cn].spell[n])!=0)
		{
			if (bu[in].temp > 100 && bu[in].data[3]==IT_POT_OFF)	offpot = 1;
		}
	}
	
	if (offpot)		dam = dam * 11/10;
	if (defpot)		dam = dam *  9/10;
	if (scorched)	dam = dam * (1000 + scorched) / 1000;
	if (guarded)	dam = dam * (1000 - guarded ) / 1000;
	
	if (type==3)
	{
		dam *= 1000;
	}
	else
	{
		dam -= ch[co].armor;
		if (dam<0) dam = 0;
		else
		{
			switch (type)
			{
				case 0:
				case 4:
					dam *= 250;
					break;
				case 1:
					dam *= 650;
					break;
				case 6:
					dam *= 150;
					break;
				case 7:
					dam *=  50;
					break;
				default: // 2, 5
					dam *= 750;
					break;
			}
		}
	}

	if (ch[co].flags & CF_IMMORTAL)
	{
		dam = 0;
	}

	if (type!=3)
	{
		do_area_notify(cn, co, ch[cn].x, ch[cn].y, NT_SEEHIT, cn, co, 0, 0);
		do_notify_char(co, NT_GOTHIT, cn, dam / 1000, 0, 0);
		do_notify_char(cn, NT_DIDHIT, co, dam / 1000, 0, 0);
	}

	if (dam<1)
	{
		return 0;
	}

	// give some EXPs to the attacker for a successful blow:
	if (type!=2 && type!=3 && !noexp)
	{
		tmp = dam;
		if (ch[co].a_hp-500 < tmp) tmp = ch[co].a_hp-500;
		tmp /= 4000;
		
		if (ch[co].flags & CF_EXTRAEXP) tmp *= 2;
		
		if (tmp>0 && cn)
		{
			tmp = scale_exps(cn, co, tmp);
			if (halfexp) tmp /= 4;
			if (tmp>0)
			{
				ch[cn].points += tmp;
				ch[cn].points_tot += tmp;
				do_check_new_level(cn);
			}
		}
	}

	if (type!=1 && type!=6 && type!=7)
	{
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
	
	hp_dam = dam;
	
	// Tarot - High Priestess - 30% damage dealt to mana instead
	if (get_tarot(co, IT_CH_PREIST))
	{
		mana_dam = dam*30/100;
		
		if (ch[co].a_mana - mana_dam<500)
			mana_dam = (ch[co].a_mana-500);
		
		hp_dam -= mana_dam;
	}
	
	// Cloak - Cloak of Shadows - 20% of damage dealt to end instead
	if (get_gear(co, IT_TW_CLOAK))
	{
		end_dam = dam*20/100;
		
		if (ch[co].a_end - end_dam<500)
			end_dam = (ch[co].a_end-500);
		
		hp_dam -= end_dam;
	}
	
	// Weapon - Bronchitis :: 10% cleave damage also dealt to mana
	if (type == 5 && get_gear(cn, IT_WP_BRONCHIT))
	{
		mana_dam = hp_dam/10;
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

	if (ch[co].a_hp<8000 && ch[co].a_hp>=500)
	{
		do_char_log(co, 0, "You're almost dead... Give running a try!\n");
	}

	if (ch[co].a_hp<500)
	{
		do_area_log(cn, co, ch[cn].x, ch[cn].y, 0, "%s is dead!\n", ch[co].reference);
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

		if (type!=2 && cn && !(mf & MF_ARENA) && !noexp)
		{
			tmp  = do_char_score(co);
			rank = points2rank(ch[co].points_tot);

			if (!ch[co].skill[SK_MEDIT][0]) for (n = 0; n<MAXBUFFS; n++)	if ((in = ch[co].spell[n]))
			if (bu[in].temp==SK_PROTECT || bu[in].temp==SK_ENHANCE || bu[in].temp==SK_BLESS || bu[in].temp==SK_HASTE)
				tmp += tmp / 5;
			
			if (ch[co].flags & CF_EXTRAEXP)
				tmp *= 3;
		}
		do_char_killed(cn, co, 0);


		if (type!=2 && cn && cn!=co && !(mf & MF_ARENA) && !noexp)
		{
			do_give_exp(cn, tmp, 1, rank);
			
			// stronghold points based on the subdriver of the npc
			if (ch[co].data[26]>=101 && ch[co].data[26]<=399)
			{
				tmp = max(1, do_char_score(co)/20);
				
				if (ch[co].flags & CF_CANCRIT)
					tmp *= 3;
				else if (ch[co].flags & CF_EXTRAEXP)
					tmp *= 2;
				
				do_give_bspoints(cn, tmp, 1);
			}
		}

		ch[cn].cerrno = ERR_SUCCESS;
	}
	else
	{
		if ((type==0 || type==4 || type==5) && ch[co].gethit_dam>0)
		{
			do_hurt(co, cn, RANDOM(ch[co].gethit_dam) + 1, 3);
		}
	}

	return(dam / 1000);
}

int do_surround_check(int cn, int co, int gethit)
{
	int cc, n;
	
	cc = ch[cn].attack_cn;
	
	if (cn==0 || co==0 || cn==co) return 0;
	if (ch[cn].data[42] == ch[co].data[42]) return 0;	// Same Group
	if (ch[cn].attack_cn!=co && ch[co].alignment==10000) return 0;	// Ignore friendly npcs unless targeted
	if (ch[cn].attack_cn!=co && (strcmp(ch[co].name, "Gate Guard")==0 || strcmp(ch[co].name, "Outpost Guard")==0)) return 0;	// Ignore BS gate npcs
	if (ch[cn].data[CHD_COMPANION] == co || ch[cn].data[CHD_SHADOWCOPY] == co) return 0; // Ignore own GC/SC
	
	if (!IS_PLAYER(cn) && !IS_PLAYER(cc) && !IS_COMPANION(cc) && (IS_PLAYER(co) || IS_COMPANION(co))) return 0;
	if (!IS_PLAYER(cn) && (IS_PLAYER(cc) || IS_COMPANION(cc)) && !IS_PLAYER(co) && !IS_COMPANION(co)) return 0;
	
	if (ch[cn].data[63] == co || ch[co].data[63] == cn) return 0;	// Ignore master
	
	if (IS_COMPANION(cn) && ch[cn].data[63])
	{
		if (ch[ch[cn].data[63]].data[CHD_COMPANION] == co) return 0;
		if (ch[ch[cn].data[63]].data[CHD_SHADOWCOPY] == co) return 0;
	}
	if (IS_COMPANION(co) && ch[co].data[63])
	{
		if (ch[ch[co].data[63]].data[CHD_COMPANION] == cn) return 0;
		if (ch[ch[co].data[63]].data[CHD_SHADOWCOPY] == cn) return 0;
	}
	
	if ((ch[cn].flags & CF_PLAYER) && (ch[co].flags & CF_PLAYER)) 
		for (n = 1; n<10; n++) if (ch[cn].data[n]==co) return 0;	// Ignore group members
	
	if (!(ch[cn].flags & CF_PLAYER) && ch[co].temp==ch[cn].data[31]) return 0;
	
	if (gethit && (!do_char_can_see(cn, co) || !may_attack_msg(cn, co, 0) || ch[co].flags & CF_IMMORTAL)) return 0;
	if (gethit && ch[co].data[63]) if (gethit && !may_attack_msg(cn, ch[co].data[63], 0)) return 0;
	
	return 1;
}

void do_attack(int cn, int co, int surround) // surround = 2 means it's a SURROUND RATE hit
{
	int hit, dam = 0, die, m, mc, odam = 0;
	int chance, s1, s2, bonus = 0, diff, crit_dice, crit_chance, crit_mult, crit_dam=0, in=0, co_orig=-1;
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
	
	if (globs->flags & GF_MAYHEM)
	{
		if (!(ch[cn].flags & CF_PLAYER))
		{
			s1 += 10;
		}
		if (!(ch[co].flags & CF_PLAYER))
		{
			s2 += 10;
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

	// Outsider's Eye
	if (!get_neck(co, IT_TW_OUTSIDE))
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

	if (ch[co].stunned==1 || !ch[co].attack_cn) // Stunned or not fighting
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

	die = RANDOM(20) + 1;
	if (die<=chance)
	{
		hit = 1;
	}
	else
	{
		hit = 0;
	}

	if (hit)
	{
		dam = ch[cn].weapon + RANDOM(9);
		if (ch[cn].top_damage>3)
		{
			dam += RANDOM(ch[cn].top_damage);
		}
		
		// Tarot - Strength - 25% more damage
		if (get_tarot(cn, IT_CH_STRENGTH))
		{
			dam 		= (dam*125)/100;
			glv_base 	= (glv_base*125)/100;
		}
		
		odam 	  = dam;
		dam 	 += bonus;
		
		// Critical hits!! For sanity's sake, only players and their GCs can get them.
		// This is deliberately placed after setting odam, so that crits don't make SH dumb
		if ((ch[cn].flags & (CF_PLAYER | CF_CANCRIT)) || ch[cn].skill[SK_PRECISION][0])
		{
			crit_dice 	= 10000;
			crit_chance = ch[cn].crit_chance;
			crit_mult   = ch[cn].crit_multi;
			
			 // Book - Way of the Sword
			if (ch[cn].data[73])
			{
				crit_chance += ch[cn].data[73];
			}
			
			die = RANDOM(crit_dice) + 1;
			
			if (die<=crit_chance)
			{
				ch[cn].data[73]=0;
				crit_dam  = dam + 1+points2rank(ch[cn].points_tot);
				crit_dam *= crit_mult / 100;
				crit_dam -= dam;
				
				do_area_sound(co, 0, ch[co].x, ch[co].y, ch[cn].sound + 8);
				char_play_sound(co, ch[cn].sound + 8, -150, 0);
			}
		}
		
		// Special gloves
		if (!RANDOM(40)) // 2.5% chance
		{
			glv = glv_base;
			if (glv_base+RANDOM(20) > get_target_resistance(co)+RANDOM(16))
			{
				in = it[ch[cn].worn[WN_ARMS]].temp;
				if (in==IT_GL_SERPENT) { if (spell_poison(cn, co, glv, 1)) do_char_log(cn, 0, "You poisoned your enemies!\n"); else in=0; }
				if (in==IT_GL_BURNING) { if (spell_scorch(cn, co, glv, 1)) do_char_log(cn, 0, "You scorched your enemies!\n"); else in=0; }
				if (in==IT_GL_SHADOW)  { if (spell_blind(cn, co, glv)) do_char_log(cn, 0, "You blinded your enemies!\n"); else in=0; }
				if (in==IT_GL_CHILLED) { if (spell_slow(cn, co, glv, 1))   do_char_log(cn, 0, "You slowed your enemies!\n");   else in=0; }
				if (in==IT_GL_CURSED)  { if (spell_curse(cn, co, glv, 1))  do_char_log(cn, 0, "You cursed your enemies!\n");   else in=0; }
				if (in==IT_GL_TITANS)  { if (spell_weaken(cn, co, glv, 1))   do_char_log(cn, 0, "You weakened your enemies!\n"); else in=0; }
				if (in==IT_GL_BLVIPER) { if (spell_frostburn(cn, co, glv))   do_char_log(cn, 0, "You glaciated your enemies!\n"); else in=0; }
			}
		}
		
		co_orig = co;
		
		// Weapon damage
		if (ch[cn].flags & (CF_PLAYER))
		{
			item_damage_weapon(cn, dam+crit_dam);
		}
		
		dam = do_hurt(cn, co, dam+crit_dam, 0);
		
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
		
		// Check if the attacker has Razor
		in2 = has_spell(cn, SK_RAZOR);
		
		if (surround && ch[cn].skill[SK_SURROUND][0])
		{
			surrDam = odam/4*3 + crit_dam/2;
			glv 	= glv_base/4*3;
			
			if (surround==1 && (ch[cn].kindred & KIN_ARCHTEMPLAR) && ch[cn].skill[SK_PROX][0] && !(ch[cn].flags & CF_AREA_OFF))
			{
				int surraoe, x, y, xf, yf, xt, yt, xc, yc, obsi = 0;
				int aoe_power, tmp_h, tmp_s, tmp_g;
				
				aoe_power = get_skill_score(cn, SK_PROX)+15;
				obsi 	= (it[ch[cn].worn[WN_NECK]].temp == IT_AM_OBSIDI)?1:0;
				surraoe = aoe_power/PROX_CAP + obsi;
				tmp_h   = sqr(aoe_power/PROX_HIT-surraoe)/5 + obsi*3;
				tmp_s   = surrDam;
				tmp_g   = glv;
				
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
					if (sqr(xc - x) + sqr(yc - y) > (sqr(surraoe) + 1))
					{
						continue;
					}
					if ((co = map[x + y * MAPX].ch) && cn!=co)
					{
						// Adjust effectiveness by radius
						surrDam = min(tmp_s, tmp_s / max(1, (
							sqr(abs(xc - x)) + sqr(abs(yc - y))) / tmp_h));
						glv		= min(tmp_g, tmp_g / max(1, (
							sqr(abs(xc - x)) + sqr(abs(yc - y))) / tmp_h));
						
						// Hit the target
						remember_pvp(cn, co);
						if (!do_surround_check(cn, co, 1)) continue;
						if (get_skill_score(cn, SK_SURROUND) + RANDOM(40)>=ch[co].to_parry)
						{
							surrBonus = 0;
							if ((get_skill_score(cn, SK_SURROUND)-ch[co].to_parry)>0)
							{
								surrBonus = odam/4 * min(max(1,get_skill_score(cn, SK_SURROUND)-ch[co].to_parry), 20)/20;
							}
							surrTotal = surrDam+surrBonus;
							if (co==co_orig) surrTotal = surrTotal/4*3;
							do_hurt(cn, co, surrTotal, 4);
							if (glv_base+RANDOM(20) > get_target_resistance(co)+RANDOM(16) && co!=co_orig)
							{
								if (in==IT_GL_SERPENT) spell_poison(cn, co, glv, 1);
								if (in==IT_GL_BURNING) spell_scorch(cn, co, glv, 1);
								if (in==IT_GL_SHADOW ) spell_blind(cn, co, glv);
								if (in==IT_GL_CHILLED) spell_slow(cn, co, glv, 1);
								if (in==IT_GL_CURSED ) spell_curse(cn, co, glv, 1);
								if (in==IT_GL_TITANS ) spell_weaken(cn, co, glv, 1);
								if (in==IT_GL_BLVIPER) spell_frostburn(cn, co, glv);
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
						if (	(surround==1 && get_skill_score(cn, SK_SURROUND)  + RANDOM(40)>=ch[co].to_parry) 
							|| 	(surround==2 && get_skill_score(cn, SK_PROX) + RANDOM(20)>=ch[co].to_parry))
						{
							surrBonus = 0;
							if (surround==1 && (get_skill_score(cn, SK_SURROUND)-ch[co].to_parry)>0)
							{
								surrBonus = odam/4 * min(max(1,get_skill_score(cn, SK_SURROUND)-ch[co].to_parry), 20)/20;
							}
							if (surround==2 && (get_skill_score(cn, SK_PROX)-ch[co].to_parry)>0)
							{
								surrBonus = odam/4 * min(max(1,get_skill_score(cn, SK_PROX)-ch[co].to_parry), 40)/40;
							}
							surrTotal = surrDam+surrBonus;
							if (co==co_orig) surrTotal = surrTotal/4*3;
							do_hurt(cn, co, surrTotal, 4);
							if (glv_base+RANDOM(20) > get_target_resistance(co)+RANDOM(16) && co!=co_orig)
							{
								if (in==IT_GL_SERPENT) spell_poison(cn, co, glv, 1);
								if (in==IT_GL_BURNING) spell_scorch(cn, co, glv, 1);
								if (in==IT_GL_SHADOW ) spell_blind(cn, co, glv);
								if (in==IT_GL_CHILLED) spell_slow(cn, co, glv, 1);
								if (in==IT_GL_CURSED ) spell_curse(cn, co, glv, 1);
								if (in==IT_GL_TITANS ) spell_weaken(cn, co, glv, 1);
								if (in==IT_GL_BLVIPER) spell_frostburn(cn, co, glv);
							}
							if (in2 && co!=co_orig)
							{
								spell_razor(cn, co, bu[in2].power/2 + bu[in2].power/4, 1);
							}
						}
					}
				}
			}
		}
		if (in2)
		{
			spell_razor(cn, co_orig, bu[in2].power, 1);
		}
	}
	else
	{
		do_area_sound(co, 0, ch[co].x, ch[co].y, ch[cn].sound + 5);
		char_play_sound(co, ch[cn].sound + 5, -150, 0);

		do_area_notify(cn, co, ch[cn].x, ch[cn].y, NT_SEEMISS, cn, co, 0, 0);
		do_notify_char(co, NT_GOTMISS, cn, 0, 0, 0);
		do_notify_char(cn, NT_DIDMISS, co, 0, 0, 0);
		
		if ((ch[co].flags & CF_PLAYER) && get_book(co, IT_BOOK_SWOR)) // Book: Way of the Sword
		{
			// NOTE: A value of 100 is 1%. 10000 is 100%.
			ch[co].data[73]+=50;
			if (ch[co].data[73]>10000) ch[co].data[73]=10000;
		}
	}
}

int do_maygive(int cn, int co, int in)
{
	if (in<1 || in>=MAXITEM)
	{
		return( 1);
	}

	if (it[in].temp==IT_LAGSCROLL)
	{
		return( 0);                      // lag scroll

	}
	return(1);
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
			do_char_log(cn, 1, "You give %s to %s.\n",
			            it[in].name, ch[co].name);
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
	return(1);
}

int do_char_can_see(int cn, int co)
{
	int d, d1, d2, light, rd;
	unsigned long long prof;

	if (cn==co)
	{
		return( 1);
	}

	if (ch[co].used!=USE_ACTIVE)
	{
		return( 0);
	}
	if ((ch[co].flags & CF_INVISIBLE) && invis_level(cn)<invis_level(co))
	{
		return( 0);
	}
	if (ch[co].flags & CF_BODY)
	{
		return( 0);
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
		return( 0);
	}                                             // save some time...
	
	// x+, y-
	if ((ch[co].y <= ch[cn].y-16 && ch[co].x >= ch[cn].x+20) || (ch[co].y <= ch[cn].y-20 && ch[co].x >= ch[cn].x+16))
	{
		prof_stop(21, prof);
		return( 0);
	}
	
	// modify by perception and stealth:
	if (ch[co].alignment > 0 && !(ch[co].flags & CF_PLAYER) && !(ch[co].skill[SK_STEALTH][0]))
	{
		d = d/2;
	}
	else if (ch[co].mode==0)	// slow
	{
		d = (d * (get_skill_score(co, SK_STEALTH)*2)) / 10;
	}
	else if (ch[co].mode==1)	// normal
	{
		d = (d * (get_skill_score(co, SK_STEALTH)*2)) / 30;
	}
	else						// fast
	{
		d = (d * (get_skill_score(co, SK_STEALTH)*2)) / 90;
	}

	d -= get_skill_score(cn, SK_PERCEPT) * 4;

	// modify by light:
	if (!(ch[cn].flags & CF_INFRARED))
	{
		light = max(map[ch[co].x + ch[co].y * MAPX].light, check_dlight(ch[co].x, ch[co].y));
		light = do_char_calc_light(cn, light);

		if (light==0)
		{
			prof_stop(21, prof);
			return( 0);
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
		return( 0);
	}

	if (!can_see(cn, ch[cn].x, ch[cn].y, ch[co].x, ch[co].y, TILEX/2))
	{
		prof_stop(21, prof);
		return( 0);
	}

	prof_stop(21, prof);

	if (d<1)
	{
		return( 1);
	}

	return(d);
}

int do_char_can_see_item(int cn, int in)
{
	int d, d1, d2, light, rd;
	unsigned long long prof;

	if (it[in].used!=USE_ACTIVE)
	{
		return( 0);
	}

	// raw distance:
	d1 = abs(ch[cn].x - it[in].x);

	d2 = abs(ch[cn].y - it[in].y);
	
	// Fixed the distance changes by dividing by (9/4), or 2.25
	rd = d = (d1*d1 + d2*d2) / (9/4);

	if (d>1000)
	{
		return( 0);   // save some time...
	}
	prof = prof_start();

	// modify by perception
	d += 50 - get_skill_score(cn, SK_PERCEPT) * 2;

	// modify by light:
	if (!(ch[cn].flags & CF_INFRARED))
	{
		light = max(map[it[in].x + it[in].y * MAPX].light, check_dlight(it[in].x, it[in].y));
		light = do_char_calc_light(cn, light);

		if (light==0)
		{
			prof_stop(22, prof);
			return( 0);
		}
		if (light>64)
		{
			light = 64;
		}
		d += (64 - light) * 3;
	}

	if (it[in].flags & IF_HIDDEN)
	{
		d += it[in].data[9];
	}
	else if (rd<3 && d>200)
	{
		d = 200;
	}

	if (d>200)
	{
		prof_stop(22, prof);
		return( 0);
	}

	if (!can_see(cn, ch[cn].x, ch[cn].y, it[in].x, it[in].y, TILEX/2))
	{
		prof_stop(22, prof);
		return( 0);
	}

	prof_stop(22, prof);

	if (d<1)
	{
		return( 1);
	}

	return(d);
}

// Feb 2020 - Haste Bonus Table
/*
int get_haste_power(int power)
{
	int n, bonusMod;
	int tableRange[11] = { 12, 16, 20, 24, 28, 32, 36, 40, 44, 48, 52 };
	
	// 2 to start (due to halving when used), and reduced by a base amount
	bonusMod = 2;
	power -= 12;
	
	// Loop through the table above and add bonuses for each check we pass.
	// If we fail a check, any leftover is discarded
	for (n = 0; n < 11; n++)
	{
		if (power < tableRange[n])
		{
			break;
		}
		else
		{
			bonusMod++;
			power -= tableRange[n];
			if (power < 1) break;
		}
	}
	
	return bonusMod;
}
*/
//

void do_update_char(int cn)
{
	ch[cn].flags |= (CF_UPDATE | CF_SAVEME);
}
void really_update_char(int cn)
{
	int n, m, oldlight, z, sublight = 0, maxlight = 0;
	int hp = 0, end = 0, mana = 0, weapon = 0, armor = 0, light = 0, gethit = 0, infra = 0, coconut = 0, pigsblood = 0;
	int heal_hp, heal_end, heal_mana;
	int tempWeapon = 0, tempArmor = 0;
	int hastePower = 0, slowPower = 0, hasteSpeed = 0, slowSpeed = 0, slow2Speed = 0;
	int attrib[5];
	int attrib_ex[5];
	int skill[50];
	unsigned long long prof;
	int charmSpec = 0, gearSpec = 0;
	int in=0;
	//int speedvalue_a = 0, speedvalue_b = 0, speedvalue_c = 0;
	int base_spd = 0, spd_move = 0, spd_attack = 0, spd_cast = 0;
	int spell_mod = 0, spell_apt = 0, spell_cool = 0;
	int critical_b = 0, critical_c = 0, critical_m = 0;
	int hit_rate = 0, parry_rate = 0;
	int damage_top = 0;
	
	prof = prof_start();

	ch[cn].flags &= ~(CF_NOHPREG | CF_NOENDREG | CF_NOMANAREG);
	ch[cn].sprite_override = 0;

	m = ch[cn].x + ch[cn].y * MAPX;

	// No-magic zone check -- except if you have the sun ammy or dark-sun ammy equipped
	if ((map[m].flags & MF_NOMAGIC) && !char_wears_item(cn, 466) && !char_wears_item(cn, 481))
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

	for (n = 0; n<5; n++)
	{
		//ch[cn].attrib[n][4] = 0;
		attrib[n] = 0;
	}
	
	heal_hp   = 0;
	heal_end  = 0;
	heal_mana = 0;
	
	ch[cn].hp[4] = 0;
	hp = 0;
	ch[cn].end[4] = 0;
	end = 0;
	ch[cn].mana[4] = 0;
	mana = 0;

	for (n = 0; n<MAXSKILL; n++)
	{
		//ch[cn].skill[n][4] = 0;
		skill[n] = 0;
	}

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
	
	base_spd = spd_move = spd_attack = spd_cast = 0;
	spell_mod = spell_apt = spell_cool = 0;
	critical_b = critical_c = critical_m = 0;
	hit_rate = parry_rate = 0;
	damage_top = 0;
	
	// Loop through gear slots
	for (n = 0; n<20; n++)
	{
		if (!ch[cn].worn[n]) continue;
		
		m = ch[cn].worn[n];

		// Stat bonuses are only awarded outside of no-magic zones
		if (!(ch[cn].flags & CF_NOMAGIC))
		{
			// If the item is sparked/active/whatever
			if (it[m].active)
			{
				// Attributes
				for (z = 0; z<5; z++) attrib[z] += it[m].attrib[z][1];
				
				hp   += it[m].hp[1];
				end  += it[m].end[1];
				mana += it[m].mana[1];
				
				// Skills
				for (z = 0; z<MAXSKILL; z++) skill[z] += it[m].skill[z][1];
				
				// Meta values
				base_spd   += it[m].speed[1];
				spd_move   += it[m].move_speed[1];
				spd_attack += it[m].atk_speed[1];
				spd_cast   += it[m].cast_speed[1];
				spell_mod  += it[m].spell_mod[1];
				spell_apt  += it[m].spell_apt[1];
				spell_cool += it[m].cool_bonus[1];
			}
			// Otherwise, give normal stat boosts
			else
			{
				// Attributes
				for (z = 0; z<5; z++) attrib[z] += it[m].attrib[z][0];
				
				hp   += it[m].hp[0];
				end  += it[m].end[0];
				mana += it[m].mana[0];
				
				// Skills
				for (z = 0; z<MAXSKILL; z++) skill[z] += it[m].skill[z][0];
				
				// Meta values
				base_spd   += it[m].speed[0];
				spd_move   += it[m].move_speed[0];
				spd_attack += it[m].atk_speed[0];
				spd_cast   += it[m].cast_speed[0];
				spell_mod  += it[m].spell_mod[0];
				spell_apt  += it[m].spell_apt[0];
				spell_cool += it[m].cool_bonus[0];
			}
			
			if (it[m].temp==IT_CH_PREIST) 		charmSpec |=   1;
			if (it[m].temp==IT_CH_LOVERS) 		charmSpec |=   2;
			if (it[m].temp==IT_CH_MAGI)   		charmSpec |=   4;
			if (it[m].temp==IT_CH_HERMIT) 		charmSpec |=   8;
			
			if (it[m].temp==IT_CH_STRENGTH)		charmSpec |=  32;
			if (it[m].temp==IT_CH_WHEEL)		charmSpec |=  64;
			
			if (it[m].temp==IT_BOOK_TRAV) 		gearSpec  |=   1;
			if (it[m].temp==IT_TW_HEAVENS)		gearSpec  |=   2;
			if (it[m].temp==IT_TW_ROOTS)		gearSpec  |=   4;
			if (it[m].temp==IT_WP_GILDSHINE)	gearSpec  |=   8;
			if (it[m].temp==IT_WP_KELPTRID)		gearSpec  |=  16;
			if (it[m].temp==IT_WP_WHITEODA)		gearSpec  |=  32;
			if (it[m].temp==IT_WP_BLACKTAC)		gearSpec  |=  64;
			if (it[m].temp==IT_WP_EXCALIBUR)	gearSpec  |= 128;
		}
		
		critical_b += it[m].base_crit;
		
		// Regular item bonuses
		if (it[m].active)
		{
			armor  += it[m].armor[1];
			
			// Special case for Templars with Dual Swords...
			if ((it[m].flags & IF_OF_DUALSW) &&	(ch[cn].kindred & (KIN_TEMPLAR | KIN_ARCHTEMPLAR | KIN_BRAWLER)))
			{
				weapon += it[m].weapon[1]/2;
			}
			else if (n == WN_LHAND && (it[m].flags & IF_WP_DAGGER)) // ...and daggers...
			{
				weapon += (it[m].weapon[1]/4)*3;
			}
			else
			{
				weapon += it[m].weapon[1];
			}
			
			gethit += it[m].gethit_dam[1];
			
			maxlight += it[m].light[1];
			if (it[m].light[1]>light)
			{
				light = it[m].light[1];
			}
			else if (it[m].light[1]<0)
			{
				sublight -= it[m].light[1];
			}
			
			// Meta values
			critical_c += it[m].crit_chance[1];
			critical_m += it[m].crit_multi[1];
			hit_rate   += it[m].to_hit[1];
			parry_rate += it[m].to_parry[1];
			damage_top += it[m].top_damage[1];
		}
		else
		{
			armor  += it[m].armor[0];
			
			// Special case for Templars with Dual Swords... and daggers...
			if ((it[m].flags & IF_OF_DUALSW) && 
				(ch[cn].kindred & (KIN_TEMPLAR | KIN_ARCHTEMPLAR | KIN_BRAWLER)) &&
				it[m].temp != IT_WP_FELLNIGHT)
			{
				weapon += it[m].weapon[0]/2;
			}
			else if (n == WN_LHAND && (it[m].flags & IF_WP_DAGGER)) // ...and daggers...
			{
				weapon += (it[m].weapon[0]/4)*3;
			}
			else
			{
				weapon += it[m].weapon[0];
			}
			
			gethit += it[m].gethit_dam[0];
			
			
			maxlight += it[m].light[0];
			if (it[m].light[0]>light)
			{
				light = it[m].light[0];
			}
			else if (it[m].light[0]<0)
			{
				sublight -= it[m].light[0];
			}
			
			// Meta values
			critical_c += it[m].crit_chance[0];
			critical_m += it[m].crit_multi[0];
			hit_rate   += it[m].to_hit[0];
			parry_rate += it[m].to_parry[0];
			damage_top += it[m].top_damage[0];
		}
	}
	// Feb 2020 - Store the current armor and weapon values from your gear, before other additions.
	// This will be used after the stats are updated for armor and weapon mastery
	tempArmor  = armor;
	tempWeapon = weapon;
	
	armor    += ch[cn].armor_bonus;
	weapon   += ch[cn].weapon_bonus;
	gethit   += ch[cn].gethit_bonus;
	maxlight += ch[cn].light_bonus;
	light    += ch[cn].light_bonus;

	for (n = 0; n<MAXBUFFS; n++)
	{
		if (!ch[cn].spell[n])
			continue;
		
		m = ch[cn].spell[n];
		
		if ((ch[cn].flags & CF_NOMAGIC) && !bu[m].data[4])
			continue;

		for (z = 0; z<5; z++)
		{
			attrib[z] += bu[m].attrib[z][1];
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

		armor  += bu[m].armor[1];
		weapon += bu[m].weapon[1];
		
		maxlight += bu[m].light[1];
		if (bu[m].light[1]>light)
		{
			light = bu[m].light[1];
		}
		else if (bu[m].light[1]<0)
		{
			sublight -= bu[m].light[1];
		}
		
		// Meta values
		base_spd   += bu[m].speed[1];
		spd_move   += bu[m].move_speed[1];
		spd_attack += bu[m].atk_speed[1];
		spd_cast   += bu[m].cast_speed[1];
		spell_mod  += bu[m].spell_mod[1];
		spell_apt  += bu[m].spell_apt[1];
		spell_cool += bu[m].cool_bonus[1];
		critical_c += bu[m].crit_chance[1];
		critical_m += bu[m].crit_multi[1];
		hit_rate   += bu[m].to_hit[1];
		parry_rate += bu[m].to_parry[1];
		damage_top += bu[m].top_damage[1];
		
		if (bu[m].temp==SK_WARCRY2)
		{
			// Boots - Commander's Roots :: change 'stun' into raw speed reduction. This is halved later.
			if (gearSpec & 4)
			{
				base_spd -= 300;
			}
			else
			{
				ch[cn].stunned = 1;
			}
		}
		
		if (bu[m].temp==666) // Stunned for cutscene
		{
			ch[cn].stunned = 1;
		}
		
		if (bu[m].temp==SK_TAUNT)
		{
			ch[cn].taunted = bu[m].data[0];
			
			if (bu[m].active >= bu[m].duration-1) // Fresh taunt
			{
				ch[cn].attack_cn = bu[m].data[0];
			}
		}

		if (bu[m].hp[0]<0)
		{
			ch[cn].flags |= CF_NOHPREG;
		}
		if (bu[m].end[0]<0)
		{
			ch[cn].flags |= CF_NOENDREG;
		}
		if (bu[m].mana[0]<0)
		{
			ch[cn].flags |= CF_NOMANAREG;
		}

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

	for (z = 0; z<5; z++)
	{
		attrib[z] = (int)ch[cn].attrib[z][0] + (int)ch[cn].attrib[z][1] + attrib[z];
		set_attrib_score(cn, z, attrib[z]);
	}

	hp = (int)ch[cn].hp[0] + (int)ch[cn].hp[1] + hp;
	if (pigsblood & 1) hp = hp*115/100; // Pigs blood drink
	if (charmSpec & 1) hp = hp*80/100; // Tarot - High Priestess
	if (hp<10)
	{
		hp = 10;
	}
	if (hp>999)
	{
		hp = 999;
	}
	ch[cn].hp[5] = hp;
	if (ch[cn].a_hp < ch[cn].hp[5]) ch[cn].a_hp += heal_hp;

	end = (int)ch[cn].end[0] + (int)ch[cn].end[1] + end;
	if (end<10)
	{
		end = 10;
	}
	if (end>999)
	{
		end = 999;
	}
	ch[cn].end[5] = end;
	if (ch[cn].a_end < ch[cn].end[5]) ch[cn].a_end += heal_end;

	mana = (int)ch[cn].mana[0] + (int)ch[cn].mana[1] + mana;
	if (mana<10)
	{
		mana = 10;
	}
	if (mana>999)
	{
		mana = 999;
	}
	ch[cn].mana[5] = mana;
	if (ch[cn].a_mana < ch[cn].mana[5]) ch[cn].a_mana += heal_mana;

	if (ch[cn].flags & (CF_PLAYER))
	{
		if (infra==15 && !(ch[cn].flags & CF_INFRARED))
		{
			ch[cn].flags |= CF_INFRARED;
			do_char_log(cn, 0, "You can see in the dark!\n");
		}
		if (infra!=15 && (ch[cn].flags & CF_INFRARED) && !(ch[cn].flags & CF_GOD))
		{
			ch[cn].flags &= ~CF_INFRARED;
			do_char_log(cn, 0, "You can no longer see in the dark!\n");
		}
	}

	for (z = 0; z<MAXSKILL; z++)
	{
		skill[z] = (int)ch[cn].skill[z][0] + (int)ch[cn].skill[z][1] + skill[z];

		skill[z] += ((int)get_attrib_score(cn, skilltab[z].attrib[0]) +
		             (int)get_attrib_score(cn, skilltab[z].attrib[1]) +
		             (int)get_attrib_score(cn, skilltab[z].attrib[2])) / 5;

		if ((charmSpec & 8) && (z==SK_RESIST||z==SK_IMMUN))
		{
			skill[z] = (skill[z]*90)/100;
		}
		
		set_skill_score(cn, z, skill[z]);
	}
	// Feb 2020 - Armor Mastery Bonus
	if (tempArmor && ch[cn].skill[SK_ARMORM][0])
	{
		// Templar classes get a better bonus
		if ((ch[cn].kindred & (KIN_TEMPLAR | KIN_ARCHTEMPLAR | KIN_BRAWLER)) && !(ch[cn].kindred & KIN_MONSTER))
		{
			armor += min(tempArmor, get_skill_score(cn, SK_ARMORM)/4); //	/5
		}
		else if (ch[cn].kindred & KIN_MONSTER)
		{
			armor += min(tempArmor, get_skill_score(cn, SK_ARMORM)/6); //	*3/20
		}
		else
		{
			armor += min(tempArmor, get_skill_score(cn, SK_ARMORM)/6); //	/10
		}
	}
	// Feb 2020 - Weapon Mastery Bonus
	if (tempWeapon && ch[cn].skill[SK_WEAPONM][0])
	{
		// Templar classes get a better bonus
		if ((ch[cn].kindred & (KIN_TEMPLAR | KIN_ARCHTEMPLAR | KIN_BRAWLER)) && !(ch[cn].kindred & KIN_MONSTER))
		{
			weapon += min(tempWeapon, get_skill_score(cn, SK_WEAPONM)/4);
		}
		else if (ch[cn].kindred & KIN_MONSTER)
		{
			weapon += min(tempWeapon, get_skill_score(cn, SK_WEAPONM)/6);
		}
		else
		{
			weapon += min(tempWeapon, get_skill_score(cn, SK_WEAPONM)/6);
		}
	}
	
	// Maxlight takes your cumulative total of all light sources, minus the highest.
	// Light is then the highest single light value affecting you, plus half of whatever maxlight is beyond that.
	maxlight -= light;
	light += maxlight/2;

	light -= sublight;
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
		attrib[n]    = ch[cn].attrib[n][0];
		attrib_ex[n] = get_attrib_score(cn, n);
	}
	
	// Book - Traveler's Guide :: Swap effects of Braveness and Agility
	if (gearSpec & 1) 
	{
		attrib[AT_BRV] = ch[cn].attrib[AT_AGL][0];
		attrib[AT_AGL] = ch[cn].attrib[AT_BRV][0];
		
		attrib_ex[AT_BRV] = get_attrib_score(cn, AT_AGL);
		attrib_ex[AT_AGL] = get_attrib_score(cn, AT_BRV);
	}
	
	// Tarot - Magician :: Swap effects of Strength and Intuition
	if (charmSpec & 4)
	{
		attrib[AT_INT] = ch[cn].attrib[AT_STR][0];
		attrib[AT_STR] = ch[cn].attrib[AT_INT][0];
		
		attrib_ex[AT_INT] = get_attrib_score(cn, AT_STR);
		attrib_ex[AT_STR] = get_attrib_score(cn, AT_INT);
	}
	
	/*
		ch[].speed value
	*/
	
	// Boots - Commander's Roots :: halves all speed reductions
	if (gearSpec & 4) 
	{
		if (base_spd<0) 	base_spd 	/= 2;
		if (spd_move<0) 	spd_move 	/= 2;
		if (spd_attack<0) 	spd_attack 	/= 2;
		if (spd_cast<0) 	spd_cast 	/= 2;
	}
	
	// Weapon - Kelp Trident :: +10 speed while underwater
	if ((gearSpec & 16) && (map[ch[cn].x + ch[cn].y * MAPX].flags & MF_UWATER))
	{
		base_spd += 10;
	}
	
	base_spd = 120 + base_spd + (attrib_ex[AT_AGL] + attrib_ex[AT_STR]) / 6 + ch[cn].speed_mod;
	
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
		ch[].atk_speed value
	*/
	
	spd_attack += attrib_ex[AT_AGL]/4;
	
	// Tarot - Strength - 20% less attack speed
	if (charmSpec & 32)
	{
		spd_attack = spd_attack * 80 / 100;
	}
	
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
		ch[].cast_speed value
	*/
	
	spd_cast += attrib_ex[AT_BRV]/4;
	
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
	
	spell_apt += (attrib[AT_WIL] + attrib[AT_INT]) * spell_mod / 100;
	
	// Clamp spell_apt between 0 and 999
	if (spell_apt > 999)
	{
		spell_apt = 999;
	}
	if (spell_apt < 0)
	{
		spell_apt = 0;
	}
	ch[cn].spell_apt = spell_apt;
	
	
	/*
		ch[].cool_bonus value
		
		dur * 100 / var = skill exhaust
	*/
	
	spell_cool += attrib_ex[AT_INT]/2;
	
	// Clamp spell_cool between 0 and 900
	if (spell_cool > 900)
	{
		spell_cool = 900;
	}
	if (spell_cool < 0)
	{
		spell_cool = 0;
	}
	ch[cn].cool_bonus = 100 + spell_cool;
	
	//
	
	/*
		ch[].crit_chance value
		
		Base crit chance is currently determined by what kind of weapon is equipped 
		It is further increased by "Precision" skill score.
		
		After this point, crit base is increased by a factor of 100,
		so that precision can have a more consistant effect.
	*/
	
	critical_b *= 100;
	
	// Grant extra crit chance by crit bonus
	critical_b += critical_b * critical_c/100;
	
	if (ch[cn].skill[SK_PRECISION][0])
	{
		critical_b += (critical_b * skill[SK_PRECISION])/PREC_CAP;
	}
	
	// Tarot - Wheel of Fortune :: Less crit chance, more crit multi
	if (charmSpec & 64)
	{
		critical_b /= 2;
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
		
		Base crit multiplier is 2x
	*/
	
	critical_m += 100;
	
	// Weapon - Gildshine :: A third of Bartering is granted as crit multi
	if (gearSpec & 8) 
	{
		critical_m += min(AT_CAP/3, skill[SK_BARTER]/3);
	}
	
	// Tarot - Wheel of Fortune :: Less crit chance, more crit multi
	if (charmSpec & 64)
	{
		critical_m = critical_m * 150/100;
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
		ch[].to_hit value
		
		Determined by base weapon skill, plus dual wield score if dual wielding.
	*/
	
	hit_rate += get_fight_skill(cn, skill);
	hit_rate += get_offhand_skill(cn, skill, 1);
	
	// Clamp hit_rate between 0 and 999
	if (hit_rate > 999)
	{
		hit_rate = 999;
	}
	if (hit_rate < 0)
	{
		hit_rate = 0;
	}
	ch[cn].to_hit = hit_rate;
	
	
	/*
		ch[].to_parry value
		
		Determined by base weapon skill, plus shield score if using a shield.
	*/
	
	parry_rate += get_fight_skill(cn, skill);
	parry_rate += get_offhand_skill(cn, skill, 0);
	
	// Clamp parry_rate between 0 and 999
	if (parry_rate > 999)
	{
		parry_rate = 999;
	}
	if (parry_rate < 0)
	{
		parry_rate = 0;
	}
	ch[cn].to_parry = parry_rate;
	
	
	/*
		ch[].top_damage value
		
		Determined by STR/2. This is put into a RANDOM(), so "average damage" can be considered WV plus half of this number
	*/
	
	damage_top += attrib_ex[AT_STR]/2;
	
	// Weapon - Fist of the Heavens :: Doubles the effectiveness of top-end damage
	if (gearSpec & 2) 
	{
		damage_top *= 2;
	}
	
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
	
	/*
		Weapon and Armor finalized
		
		This is moved down here due to fancy unique item functions
	*/
	
	// Weapon - Excalibur :: Additional WV from 10% of total attack speed
	if (gearSpec & 128)
	{
		weapon += min(SPEED_CAP, base_spd + spd_attack)/10;
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
		int loverSplit = (weapon + armor) / 2;
		
		weapon = loverSplit;
		armor  = loverSplit;
	}
	// Tarot - Hermit
	if (charmSpec & 8)
	{
		armor = (armor*115)/100;
	}
	//
	
	if (armor<0)
	{
		armor = 0;
	}
	if (armor>250)
	{
		armor = 250;
	}
	ch[cn].armor = armor;

	if (weapon<0)
	{
		weapon = 0;
	}
	if (weapon>250)
	{
		weapon = 250;
	}
	ch[cn].weapon = weapon;

	if (gethit<0)
	{
		gethit = 0;
	}
	if (gethit>250)
	{
		gethit = 250;
	}
	ch[cn].gethit_dam = gethit;
	//
	
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

	prof_stop(7, prof);
}

// note: this calculates ALL normal endurance/hp changes.
//       further, it is called ONLY from tick()
void do_regenerate(int cn)
{
	unsigned long long prof;
	int n, m, p, in, in2, nohp = 0, noend = 0, nomana = 0, old, mf;
	int hp = 0, end = 0, mana = 0, uwater = 0, gothp = 0;
	int race_reg = 0, race_res = 0, race_med = 0;
	int degenpower = 0, tickcheck = 6000;
	int moonmult = 20;
	int hpmult, endmult, manamult;
	int co = -1;
	int tmp = 0;

	// gothp determines how much to counter degeneration effects while underwater.
	
	mf = map[ch[cn].x + ch[cn].y * MAPX].flags;
	
	if (ch[cn].flags & CF_STONED)
		return;

	prof = prof_start();

	if ((ch[cn].flags & (CF_PLAYER)) || (IS_COMP_TEMP(cn) && (co = ch[cn].data[63]) && IS_SANEPLAYER(co)))
	{
		if (globs->flags & GF_MAYHEM)	moonmult = 10;
		if (globs->fullmoon)			moonmult = 30;
		if (globs->newmoon)				moonmult = 40;
		
		race_reg = get_skill_score(cn, SK_REGEN) * moonmult / 20;
		race_res = get_skill_score(cn, SK_REST)  * moonmult / 20;
		race_med = get_skill_score(cn, SK_MEDIT) * moonmult / 20;
	}
	else
	{
		race_reg = get_skill_score(cn, SK_REGEN) * moonmult / 30;
		race_res = get_skill_score(cn, SK_REST)  * moonmult / 30;
		race_med = get_skill_score(cn, SK_MEDIT) * moonmult / 30;
	}

	if (ch[cn].flags & CF_NOHPREG)		nohp = 1;
	if (ch[cn].flags & CF_NOENDREG)		noend = 1;
	if (ch[cn].flags & CF_NOMANAREG)	nomana = 1;

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
	
	// Set up basic values to be attributed to player hp/end/mana
	//   These are the "standing" state values and will be divided down when applied to walk/fight states
	hp   = (ch[cn].skill[SK_REGEN][0]?race_reg:0) + hpmult   * 2;
	end  = (ch[cn].skill[SK_REST][0] ?race_res:0) + endmult  * 3;
	mana = (ch[cn].skill[SK_MEDIT][0]?race_med:0) + manamult * 1;
	
	if (ch[cn].stunned!=1)
	{
		switch(ch_base_status(ch[cn].status))
		{
			// STANDING STATES
			case 0: case 1: case 2: case 3: case 4: case 5: case 6: case 7:
				ch[cn].a_hp 			+= (nohp   ? 0 : hp     ); 
				gothp 					+= (nohp   ? 0 : hp  / 2);
				ch[cn].a_end 			+= (noend  ? 0 : end    );
				ch[cn].a_mana 			+= (nomana ? 0 : mana   ); 
				break;
			
			// WALKING STATES
			case  16: case  24: case  32: case  40: case  48: case  60: case  72: case  84:
			case  96: case 100: case 104: case 108: case 112: case 116: case 120: case 124:
			case 128: case 132: case 136: case 140: case 144: case 148: case 152:
				ch[cn].a_hp 			+= (nohp   ? 0 : hp  / 4); 
				gothp 					+= (nohp   ? 0 : hp  / 8);
				if (ch[cn].mode==2) // Fast
				{
					ch[cn].a_end 		+= (noend  ? 0 : end / 4) - 40;
				}
				if (ch[cn].mode==1) // Normal
				{
					ch[cn].a_end 		+= (noend  ? 0 : end / 4);
				}
				if (ch[cn].mode==0) // Slow
				{
					ch[cn].a_end 		+= (noend  ? 0 : end / 2);
				}
				ch[cn].a_mana 			+= (nomana ? 0 : mana/ 4);
				break;
			
			// FIGHTING STATES
			case 160: case 168: case 176: case 184:
				ch[cn].a_hp 			+= (nohp   ? 0 : hp  / 8); 
				gothp 					+= (nohp   ? 0 : hp  /16);
				if (ch[cn].status2==0 || ch[cn].status2==5 || ch[cn].status2==6) // Attacking
				{
					if (ch[cn].mode==2) // Fast
					{
						ch[cn].a_end 	+= (noend  ? 0 : end / 8) - 60;
					}
					if (ch[cn].mode==1) // Normal
					{
						ch[cn].a_end 	+= (noend  ? 0 : end / 8) - 20;
					}
					if (ch[cn].mode==0) // Slow
					{
						ch[cn].a_end 	+= (noend  ? 0 : end / 4);
					}
				}
				else // Misc.
				{
					if (ch[cn].mode==2)		// Fast
					{
						ch[cn].a_end 	+= (noend  ? 0 : end / 4) - 40;
					}
					if (ch[cn].mode==1)		// Normal
					{
						ch[cn].a_end 	+= (noend  ? 0 : end / 4);
					}
					if (ch[cn].mode==0)		// Slow
					{
						ch[cn].a_end 	+= (noend  ? 0 : end / 2);
					}
				}
				ch[cn].a_mana 			+= (nomana ? 0 : mana/ 8);
				break;
			
			default:
				fprintf(stderr, "do_regenerate(): unknown ch_base_status %d.\n", ch_base_status(ch[cn].status));
				break;
		}
	}
	
	if (ch[cn].flags & CF_UNDEAD)
	{
		hp = 450 + points2rank(ch[cn].points_tot) * 25;
		
		ch[cn].a_hp	+= hp;
		gothp 		+= hp/2;
	}
	
	// Special case for the Amulet of Ankhs
	if (in = ch[cn].worn[WN_NECK])
	{
		switch (it[in].temp)
		{
			case IT_ANKHAMULET: 
				ch[cn].a_hp   += ch[cn].skill[SK_REGEN][0]?race_reg/12:0;
				ch[cn].a_end  += ch[cn].skill[SK_REST][0] ?race_res/12:0;
				ch[cn].a_mana += ch[cn].skill[SK_MEDIT][0]?race_med/12:0;
				break;
			case IT_AMBERANKH: 
				ch[cn].a_hp   += ch[cn].skill[SK_REGEN][0]?race_reg/ 6:0;
				ch[cn].a_end  += ch[cn].skill[SK_REST][0] ?race_res/24:0;
				ch[cn].a_mana += ch[cn].skill[SK_MEDIT][0]?race_med/24:0;
				break;
			case IT_TURQUANKH: 
				ch[cn].a_hp   += ch[cn].skill[SK_REGEN][0]?race_reg/24:0;
				ch[cn].a_end  += ch[cn].skill[SK_REST][0] ?race_res/ 6:0;
				ch[cn].a_mana += ch[cn].skill[SK_MEDIT][0]?race_med/24:0;
				break;
			case IT_GARNEANKH: 
				ch[cn].a_hp   += ch[cn].skill[SK_REGEN][0]?race_reg/24:0;
				ch[cn].a_end  += ch[cn].skill[SK_REST][0] ?race_res/24:0;
				ch[cn].a_mana += ch[cn].skill[SK_MEDIT][0]?race_med/ 6:0;
				break;
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
			if (bu[in].flags & IF_PERMSPELL || bu[in].temp==206)
			{
				if (bu[in].temp==206 && !(bu[in].flags & IF_PERMSPELL))
					bu[in].active--;
				
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
				if (ch[cn].a_hp<500)
				{
					if (try_lucksave(cn))
					{
						do_lucksave(cn, bu[in].name);
					}
					else
					{
						chlog(cn, "killed by spell: %s", bu[in].name);
						do_char_log(cn, 0, "The %s killed you!\n", bu[in].name);
						do_area_log(cn, 0, ch[cn].x, ch[cn].y, 0, "The %s killed %s.\n", bu[in].name, ch[cn].reference);
						do_char_killed(0, cn, 0);
					}
					return;
				}
				if (ch[cn].a_end<500)
				{
					ch[cn].a_end  = 500;
					if (bu[in].temp != 206)
					{
						bu[in].active = 0;
						chlog(cn, "%s ran out due to lack of endurance", it[in].name);
					}
				}
				if (ch[cn].a_mana<500)
				{
					ch[cn].a_mana = 500;
					if (bu[in].temp != 206)
					{
						bu[in].active = 0;
						chlog(cn, "%s ran out due to lack of mana", it[in].name);
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
						if (IS_COMP_TEMP(cn) && (co = ch[cn].data[63]) && IS_SANEPLAYER(co) 
							&& (bu[in].temp==SK_BLESS || bu[in].temp==SK_PROTECT || bu[in].temp==SK_ENHANCE))
						{
							do_sayx(cn, "My spell %s is running out, %s.",
							        bu[in].name, ch[co].name);
						}
					}
				}
			}
			
			// Healing potions
			if (bu[in].temp==102)
			{
				if (bu[in].hp[0]>0)
				{
					ch[cn].a_hp += bu[in].hp[0];
				}
				if (bu[in].end[0]>0)
				{
					ch[cn].a_end += bu[in].end[0];
				}
				if (bu[in].mana[0]>0)
				{
					ch[cn].a_mana += bu[in].mana[0];
				}
			}
			
			// Regen
			if (bu[in].temp==SK_REGEN)
			{
				ch[cn].a_hp += bu[in].hp[0];
				if (ch[cn].a_hp>ch[cn].hp[5] * 1000)
				{
					ch[cn].a_hp = ch[cn].hp[5] * 1000;
				}
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
					if (bu[in].temp==SK_SLOW)
					{
						bu[in].speed[1] 		= -(min(120, 15 + SLOWFORM(bu[in].data[1])/2));
						bu[in].move_speed[1] 	= -(min(120, 15 + SLOWFORM(bu[in].data[1])/2));
						bu[in].atk_speed[1] 	= -(min(120, 15 + SLOWFORM(bu[in].data[1])/2));
						bu[in].cast_speed[1] 	= -(min(120, 15 + SLOWFORM(bu[in].data[1])/2));
					}
					else if (bu[in].temp==SK_CURSE2)
					{
						for (n = 0; n<5; n++) 
						{
							bu[in].attrib[n][1] = -CURSE2FORM(bu[in].data[1], 4 - n);
						}
					}
					do_update_char(cn);
				}
			}
			
			// Poison & Bleed
			if (bu[in].temp==SK_POISON || bu[in].temp==SK_BLEED)
			{
				co 			= bu[in].data[0];
				degenpower 	= bu[in].data[1];
				
				if (!IS_SANEPLAYER(co)) co = 0;
				if (degenpower<1) degenpower = 1;
				
				if (ch[cn].a_hp - (degenpower + gothp)<500 && !(mf & MF_ARENA) && try_lucksave(cn))
				{
					switch (bu[in].temp)
					{
						case SK_POISON: 	do_lucksave(cn, "lethal poisoning"); 	break;
						case SK_BLEED: 		do_lucksave(cn, "lethal bleeding"); 	break;
					}
				}
				else
				{
					ch[cn].a_hp -= degenpower + gothp;
				}
				
				tickcheck = 6000/max(1,degenpower);
				
				if (co && globs->ticker%tickcheck==0 && co!=cn && !(mf & MF_ARENA))
				{
					ch[co].points += 1;
					ch[co].points_tot += 1;
					do_check_new_level(co);
				}
				
				if (ch[cn].a_hp<500)
				{
					tmp = 0;
					
					if (co)
					{
						switch (bu[in].temp)
						{
							case SK_POISON:
								do_area_log(co, cn, ch[co].x, ch[co].y, 0, "%s died from a nasty poison.\n", ch[cn].reference);
								do_char_log(co, 0, "Your poison killed %s.\n", ch[cn].reference);
								if (ch[co].flags & CF_INVISIBLE)
									do_char_log(cn, 0, "Oh dear, that poison was fatal. Somebody killed you...\n");
								else
									do_char_log(cn, 0, "Oh dear, that poison was fatal. %s killed you...\n", ch[co].name);
								break;
							case SK_BLEED:
								do_area_log(co, cn, ch[co].x, ch[co].y, 0, "%s died from their bleeding wound.\n", ch[cn].reference);
								do_char_log(co, 0, "Your bleed killed %s.\n", ch[cn].reference);
								if (ch[co].flags & CF_INVISIBLE)
									do_char_log(cn, 0, "Oh dear, that bleeding was fatal. Somebody killed you...\n");
								else
									do_char_log(cn, 0, "Oh dear, that bleeding was fatal. %s killed you...\n", ch[co].name);
								break;
						}
					}
					else
					{
						switch (bu[in].temp)
						{
							case SK_POISON:
								do_area_log(cn, cn, ch[cn].x, ch[cn].y, 0, "%s died from a nasty poison.\n", ch[cn].reference);
								do_char_log(cn, 0, "Oh dear, that poison was fatal. You died...\n");
								break;
							case SK_BLEED:
								do_area_log(cn, cn, ch[cn].x, ch[cn].y, 0, "%s died from their bleeding wound.\n", ch[cn].reference);
								do_char_log(cn, 0, "Oh dear, that bleeding was fatal. You died...\n");
								break;
						}
					}
					if (co && !(mf & MF_ARENA))
					{
						tmp = do_char_score(cn);
						
						if (!ch[cn].skill[SK_MEDIT][0]) for (m = 0; m<MAXBUFFS; m++) if ((in2 = ch[cn].spell[m])) 
						if (bu[in2].temp==SK_PROTECT || bu[in2].temp==SK_ENHANCE || bu[in2].temp==SK_BLESS || bu[in2].temp==SK_HASTE)
							tmp += tmp / 5;
						
						if (ch[cn].flags & CF_EXTRAEXP)
							tmp *= 3;
					}
					do_char_killed(co, cn, 0);

					if (co && co!=cn && !(mf & MF_ARENA))
					{
						do_give_exp(co, tmp, 1, points2rank(ch[cn].points_tot));
						
						// stronghold points based on the subdriver of the npc
						if (ch[cn].data[26]>=101 && ch[cn].data[26]<=399)
						{
							tmp = max(1, do_char_score(cn)/20);
							
							if (ch[cn].flags & CF_CANCRIT)
								tmp *= 3;
							else if (ch[cn].flags & CF_EXTRAEXP)
								tmp *= 2;
							
							do_give_bspoints(co, tmp, 1);
						}
					}
					return;
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
				int pulse_dam, pulse_aoe, pulse_rad, x, y, xf, yf, xt, yt, xc, yc;
				int aoe_power, tmp_h, tmp_s;
				
				aoe_power = get_skill_score(cn, SK_PROX)+15;
				pulse_rad = bu[in].data[3];
				pulse_aoe = aoe_power/(PROX_CAP*2) + pulse_rad;
				tmp_h   = sqr(aoe_power/PROX_HIT-pulse_aoe)/5+(pulse_rad*3);
				tmp_s   = bu[in].power;
				
				xc = ch[cn].x;
				yc = ch[cn].y;
				xf = max(1, xc - pulse_aoe);
				yf = max(1, yc - pulse_aoe);
				xt = min(MAPX - 1, xc + 1 + pulse_aoe);
				yt = min(MAPY - 1, yc + 1 + pulse_aoe);
				
				for (x = xf; x<xt; x++) for (y = yf; y<yt; y++) 
				{
					// This makes the radius circular instead of square
					if (sqr(xc - x) + sqr(yc - y) > (sqr(pulse_aoe) + 1))
					{
						continue;
					}
					if ((co = map[x + y * MAPX].ch) && cn!=co)
					{
						pulse_dam = min(tmp_s, tmp_s / max(1, (
							sqr(abs(xc - x)) + sqr(abs(yc - y))) / tmp_h));
						remember_pvp(cn, co);
						if (do_surround_check(cn, co, 1))
						{
							do_hurt(cn, co, spell_immunity(pulse_dam, get_target_immunity(co)) * 2, 6);
							
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
				old = bu[in].armor[1];
				if (ch[cn].kindred & KIN_SEYAN_DU) 
					bu[in].armor[1] = bu[in].active / 1536 + 1;
				else
					bu[in].armor[1] = bu[in].active / 1024 + 1;
				bu[in].power = bu[in].active / 256;
				if (old!=bu[in].armor[1])
				{
					do_update_char(cn);
				}
			}
			if (bu[in].temp==SK_MSHELL)
			{
				old = bu[in].skill[SK_RESIST][1];
				if (ch[cn].kindred & KIN_SEYAN_DU)
				{
					bu[in].skill[SK_RESIST][1] = bu[in].active / 768 + 1;
					bu[in].skill[SK_IMMUN][1]  = bu[in].active / 768 + 1;
				}
				else
				{
					bu[in].skill[SK_RESIST][1] = bu[in].active / 512 + 1;
					bu[in].skill[SK_IMMUN][1]  = bu[in].active / 512 + 1;
				}
				bu[in].power = bu[in].active / 128;
				if (old!=bu[in].skill[SK_RESIST][1])
				{
					do_update_char(cn);
				}
			}
			
			if (bu[in].temp==SK_RAZOR2 && (co = bu[in].data[0]))
			{
				int raz_sound = 0, raz_power;
				int crit_dice, crit_chance, crit_mult, crit_dam = 0, die;
				
				raz_power = bu[in].power * 2;
				
				chlog(co, "Razor charged with %d base damage", raz_power);
				
				if ((bu[in].data[2]==1 || bu[in].data[1]==1 || !bu[in].active) &&
					((ch[co].flags & (CF_PLAYER | CF_CANCRIT)) || ch[co].skill[SK_PRECISION][0]))
				{
					crit_dice 	= 10000;
					crit_chance = ch[co].crit_chance;
					crit_mult   = ch[co].crit_multi;
					
					 // Book - Way of the Sword
					if (ch[co].data[73])
					{
						crit_chance += ch[co].data[73];
					}
					
					die = RANDOM(crit_dice) + 1;
					
					if (die<=crit_chance)
					{
						ch[co].data[73]=0;
						crit_dam  = raz_power + 1+points2rank(ch[co].points_tot);
						crit_dam *= crit_mult / 100;
						crit_dam -= raz_power;
					}
				}
				
				if (bu[in].data[2]>0) // Stored extra hit 2
				{
					bu[in].data[2]--;
					if (!bu[in].data[2])
					{
						tmp = do_hurt(co, cn, raz_power+crit_dam, 6);
						raz_sound = 1;
						chlog(co, "Razor hit %s for %d damage", ch[cn].name, tmp);
					}
				}
				if (bu[in].data[1]>0) // Stored extra hit 1
				{
					bu[in].data[1]--;
					if (!bu[in].data[1])
					{
						tmp = do_hurt(co, cn, raz_power+crit_dam, 6);
						raz_sound = 1;
						chlog(co, "Razor hit %s for %d damage", ch[cn].name, tmp);
					}
				}
				if (!bu[in].active) // Final hit of razor
				{
					tmp = do_hurt(co, cn, raz_power+crit_dam, 6);
					raz_sound = 1;
					chlog(co, "Razor hit %s for %d damage", ch[cn].name, tmp);
				}
				if (raz_sound)
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
				}
				else if (bu[in].temp!=SK_RAZOR2)
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
		if (ch[cn].skill[SK_SWIM][0])
			waterlifeloss = (250 - get_skill_score(cn, SK_SWIM)/3*2);
		
		// Amulet of Waterbreathing halves the result
		if (get_neck(cn, IT_BREATHAMMY))
			waterlifeloss /= 2;
		
		ch[cn].a_hp -= waterlifeloss + gothp;
		
		if (ch[cn].a_hp<500)
		{
			do_char_killed(0, cn, 0);
		}
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

	v = ch[cn].attrib[nr][0];

	if (!v || v>=ch[cn].attrib[nr][2])
	{
		return( 0);
	}

	p = attrib_needed(v, ch[cn].attrib[nr][3]);

	if (p>ch[cn].points)
	{
		return( 0);
	}

	ch[cn].points -= p;
	ch[cn].attrib[nr][0]++;

	do_update_char(cn);
	return(1);
}

int do_raise_hp(int cn)
{
	int p, v;

	v = ch[cn].hp[0];

	if (!v || v>=ch[cn].hp[2])
	{
		return( 0);
	}

	p = hp_needed(v, ch[cn].hp[3]);
	if (p>ch[cn].points)
	{
		return( 0);
	}

	ch[cn].points -= p;
	ch[cn].hp[0]++;

	do_update_char(cn);
	return(1);
}

int do_lower_hp(int cn)
{
	int p, v;

	if (ch[cn].hp[0]<11)
	{
		return( 0);
	}

	ch[cn].hp[0]--;

	v = ch[cn].hp[0];

	p = hp_needed(v, ch[cn].hp[3]);

	ch[cn].points_tot -= p;

	do_update_char(cn);
	return(1);
}

int do_lower_mana(int cn)
{
	int p, v;

	if (ch[cn].mana[0]<11)
	{
		return( 0);
	}

	ch[cn].mana[0]--;

	v = ch[cn].mana[0];

	p = mana_needed(v, ch[cn].mana[3]);

	ch[cn].points_tot -= p;

	do_update_char(cn);
	return(1);
}

/*
int do_raise_end(int cn)
{
	int p, v;

	v = ch[cn].end[0];

	if (!v || v>=ch[cn].end[2])
	{
		return( 0);
	}

	p = end_needed(v, ch[cn].end[3]);
	if (p>ch[cn].points)
	{
		return( 0);
	}

	ch[cn].points -= p;
	ch[cn].end[0]++;

	do_update_char(cn);
	return(1);
}
*/

int do_raise_mana(int cn)
{
	int p, v;

	v = ch[cn].mana[0];

	if (!v || v>=ch[cn].mana[2])
	{
		return( 0);
	}

	p = mana_needed(v, ch[cn].mana[3]);
	if (p>ch[cn].points)
	{
		return( 0);
	}

	ch[cn].points -= p;
	ch[cn].mana[0]++;

	do_update_char(cn);
	return(1);
}

int do_raise_skill(int cn, int nr)
{
	int p, v;

	v = ch[cn].skill[nr][0];

	if (!v || v>=ch[cn].skill[nr][2])
	{
		return( 0);
	}

	p = skill_needed(v, ch[cn].skill[nr][3]);

	if (p>ch[cn].points)
	{
		return( 0);
	}

	ch[cn].points -= p;
	ch[cn].skill[nr][0]++;

	do_update_char(cn);
	return(1);
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

void do_appraisal(int cn, int in)
{
	if (!(ch[cn].flags & CF_APPRAISE)) return;
	if (ch[cn].flags & CF_APPR_OFF) return;
	if (!it[in].value) return;
	
	do_char_log(cn, 2, "You estimate this item is worth %dG %dS.\n", it[in].value / 100, it[in].value % 100);
}

void do_chestsense(int cn, int in)
{
	int one_hou = TICKS*60*60;
	int one_min = TICKS*60;
	
	if (it[in].driver!=42 && it[in].driver!=59) return;
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

	for (n = 0; n<40; n++)
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
		
	opr = do_item_value(in);
	
	if (flag || !(it[in].flags & IF_NOMARKET))
	{
		brt = get_skill_score(cn, SK_BARTER);
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

	return(pr);
}

void do_shop_char(int cn, int co, int nr)
{
	int in, pr, in2, flag = 0, tmp, n;

	if (co<=0 || co>=MAXCHARS || nr<0 || nr>=124)
	{
		return;
	}
	if (!(ch[co].flags & CF_MERCHANT) && !(ch[co].flags & CF_BODY))
	{
		return;
	}
	if (!(ch[co].flags & CF_BODY) && !do_char_can_see(cn, co))
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
		for (n=0;n<9;n++)
		{
			in2 = ch[co].data[n];
			if ((it[in].flags & IF_ARMORS) && (it_temp[in2].flags & IF_ARMORS))		flag = 1;
			if ((it[in].flags & IF_WEAPON) && (it_temp[in2].flags & IF_WEAPON))		flag = 1;
			if ((it[in].flags & IF_JEWELERY) && (it_temp[in2].flags & IF_JEWELERY))	flag = 1;
			if ((it[in].flags & IF_MAGIC) && (it_temp[in2].flags & IF_MAGIC))		flag = 1;
			if ((it[in].flags & IF_BOOK) && (it_temp[in2].flags & IF_BOOK))			flag = 1;
			if ((it[in].flags & IF_MISC) && (it_temp[in2].flags & IF_MISC))			flag = 1;
			if ((it[in].flags & IF_GEMSTONE) && (it_temp[in2].flags & IF_GEMSTONE))	flag = 1;
		}
		
		if (ch[co].flags & CF_BSPOINTS)
		{
			do_char_log(cn, 0, "%s doesn't buy things.\n", ch[co].name);
			return;
		}
		
		if (!flag)
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

		god_give_char(in, co);
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
		if (nr<62)
		{
			if (nr<40)
			{
				if ((in = ch[co].item[nr])!=0)
				{
					int in2 = 0;
					if (ch[co].flags & CF_BSPOINTS)
					{
						// Adjust certain items into others based on player
						in2 = change_bs_shop_item(cn, it[in].temp);
						if (!in2) return;
						pr = do_item_bsvalue(in2);
						if (stronghold_points(cn)<pr)
						{
							do_char_log(cn, 0, "You cannot afford that.\n");
							return;
						}
					}
					else if (ch[co].flags & CF_MERCHANT)
					{
						pr = barter(cn, in, 1);
						if (ch[cn].gold<pr)
						{
							do_char_log(cn, 0, "You cannot afford that.\n");
							return;
						}
					}
					else
					{
						pr = 0;
					}
					if ((ch[co].flags & CF_BSPOINTS) && !(ch[co].flags & CF_BODY))
					{
						// Checks for a magic item variant
						in2 = get_special_item(in2);
						
						if (god_give_char(in2, cn))
						{
							ch[cn].bs_points -= pr;

							chlog(cn, "Bought %s", it[in2].name);
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
					else
					{
						// Buying/taking normally
						god_take_from_char(in, co);
						if (god_give_char(in, cn))
						{
							if (ch[co].flags & CF_MERCHANT)
							{
								ch[cn].gold -= pr;

								chlog(cn, "Bought %s", it[in].name);
								do_char_log(cn, 1, "You bought a %s for %dG %dS.\n", it[in].reference, pr / 100, pr % 100);

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
							}
						}
						else
						{
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
						chlog(cn, "Took %s", it[in].name);
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
						chlog(cn, "Took %s", it[in].name);
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
						in2 = change_bs_shop_item(cn, it[in].temp);
						if (!in2) return;
						do_char_log(cn, 1, "%s:\n", it_temp[in2].name);
						do_char_log(cn, 1, "%s\n", it_temp[in2].description);
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
						}
						if (it[in].flags & IF_SOULSTONE)
						{
							item_info(cn, in, 1);
						}
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
		do_char_log(cn, 1, "The waypoint whisked you away to %s.\n", waypoint[nr%32].name);
		
		fx_add_effect(6, 0, ch[cn].x, ch[cn].y, 0);
		god_transfer_char(cn, waypoint[nr%32].x, waypoint[nr%32].y);
		char_play_sound(cn, ch[cn].sound + 21, -100, 0);
		fx_add_effect(6, 0, ch[cn].x, ch[cn].y, 0);
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
		return( 0);
	}

	ch[cn].depot[n] = in;
	it[in].carried = cn;
	do_update_char(cn);

	return(1);
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
	int in, pr = 0;

	if (co<=0 || co>=MAXCHARS || nr<0 || nr>=124)
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
				do_char_log(cn, 1, "%s\n", it[in].description);
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
	int p, n, nr, hp_diff = 0, end_diff = 0, mana_diff = 0, in, pr, spr, m;
	char buf[16], *killer;

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

	if (godflag || (ch[co].flags & CF_BODY))
	{
		p = 1;
	}
	else
	{
		p = do_char_can_see(cn, co);
	}
	if (!p)
	{
		return;
	}

	if (!autoflag && !(ch[co].flags & CF_MERCHANT) && !(ch[co].flags & CF_BODY))
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

		if ((ch[co].flags & (CF_PLAYER)) && (ch[co].kindred & KIN_PURPLE))
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

			do_char_log(cn, 1, "%s died %d times, the last time on the day %d of the year %d, killed by %s %s.\n",
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

		if ((ch[co].flags & (CF_PLAYER)) && (ch[co].flags & (CF_POH)))
		{
			if (ch[co].flags & CF_POH_LEADER)
			{
				do_char_log(cn, 0, "%s is a Leader among the Purples of Honor.\n", ch[co].reference);
			}
			else
			{
				do_char_log(cn, 0, "%s is a Purple of Honor.\n", ch[co].reference);
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
	if ((ch[co].flags & (CF_MERCHANT | CF_BODY)) && !autoflag)
	{
		*(unsigned char*)(buf + 5) = 1;
		if ((in = ch[cn].citem)!=0)
		{
			if (ch[co].flags & CF_BSPOINTS)
			{
				int in2;
				// Adjust certain items into others based on player
				in2 = change_bs_shop_item(cn, it[in].temp);
				pr = do_item_bsvalue(in2);
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

	if ((ch[co].flags & (CF_MERCHANT | CF_BODY)) && !autoflag)
	{
		for (n = 0; n<40; n += 2)
		{
			buf[0] = SV_LOOK6;
			buf[1] = n;
			for (m = n; m<min(40, n + 2); m++)
			{
				if ((in = ch[co].item[m])!=0)
				{
					spr = it[in].sprite[0];
					if (ch[co].flags & CF_BSPOINTS)
					{
						int in2;
						// Adjust certain items into others based on player
						in2 = change_bs_shop_item(cn, it[in].temp);
						if (in2)
						{
							pr = do_item_bsvalue(in2);
							spr = get_special_spr(in2, it_temp[in2].sprite[0]);
						}
						else
						{
							spr = pr = 0;
						}
					}
					else if (ch[co].flags & CF_MERCHANT)
						pr = barter(cn, in, 1);
					else
						pr = 0;
				}
				else
				{
					spr = pr = 0;
				}
				*(unsigned short*)(buf + 2 + (m - n) * 6) = spr;
				*(unsigned int*)(buf + 4 + (m - n) * 6) = pr;
			}
			if ((ch[co].flags & CF_BSPOINTS) && !(ch[co].flags & CF_BODY)) // For the Black Stronghold point shop
				*(unsigned char*)(buf + 14) = 1;
			else
				*(unsigned char*)(buf + 14) = 0;
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
					pr  = 0;
				}
				else
				{
					spr = pr = 0;
				}
				*(unsigned short*)(buf + 2 + (m - n) * 6) = spr;
				*(unsigned int*)(buf + 4 + (m - n) * 6) = pr;
			}
			if ((ch[co].flags & CF_BSPOINTS) && !(ch[co].flags & CF_BODY)) // For the Black Stronghold point shop
				*(unsigned char*)(buf + 14) = 1;
			else
				*(unsigned char*)(buf + 14) = 0;
			xsend(nr, buf, 16);
		}

		buf[0] = SV_LOOK6;
		buf[1] = 60;
		if ((in = ch[co].citem)!=0 && (ch[co].flags & CF_BODY))
		{
			spr = it[in].sprite[0];
			pr  = 0;
		}
		else
		{
			spr = pr = 0;
		}
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
			*(unsigned char*)(buf + 14) = 1;
		else
			*(unsigned char*)(buf + 14) = 0;
		xsend(nr, buf, 16);
	}

	if ((ch[cn].flags & (CF_GOD | CF_IMP | CF_USURP)) && !autoflag && !(ch[co].flags & CF_MERCHANT) && !(ch[co].flags & CF_BODY) && !(ch[co].flags & CF_GOD))
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
	int n, nr, in, pr, spr, m;
	char buf[16];

	if (co<=0 || co>=MAXCHARS)
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
				pr  = 0;
				//pr  = TICKS * do_depot_cost(in);
			}
			else
			{
				spr = pr = 0;
			}
			*(unsigned short*)(buf + 2 + (m - n) * 6) = spr;
			*(unsigned int*)(buf + 4 + (m - n) * 6) = pr;
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

	for (n = 0; n<40; n++)
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
	for (n = 0; n<40; n++)
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
		return( -1);
	}

	for (n = 0; n<40; n++)
	{
		if (!ch[cn].item[n])
		{
			break;
		}
	}

	if (n==40)
	{
		return( -1);
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
		if (it[in].attrib[m][2]>ch[cn].attrib[m][0])
		{
			do_char_log(cn, 0, "You're %s to use that.\n\n", at_text[m]);
			do_char_log(cn, 0, "(Need %d base %s)\n", it[in].attrib[m][2],at_names[m]);
			return(-1);
		}
	}
	for (m = 0; m<MAXSKILL; m++)
	{
		if (it[in].skill[m][2] && !ch[cn].skill[m][0])
		{
			do_char_log(cn, 0, "You don't know how to use that.\n");
			do_char_log(cn, 0, "(Need %s)\n",skilltab[m].name);
			return(-1);
		}
		if (it[in].skill[m][2]>ch[cn].skill[m][0])
		{
			do_char_log(cn, 0, "You're not skilled enough to use that.\n");
			do_char_log(cn, 0, "(Need %d base %s)\n",it[in].skill[m][2],skilltab[m].name);
			return(-1);
		}
	}
	if (it[in].hp[2]>ch[cn].hp[0])
	{
		do_char_log(cn, 0, "You don't have enough life force to use that.\n");
		do_char_log(cn, 0, "(Need %d base Hitpoints)\n",it[in].hp[2]);
		return(-1);
	}
	if (it[in].mana[2]>ch[cn].mana[0])
	{
		do_char_log(cn, 0, "You don't have enough mana to use that.\n");
		do_char_log(cn, 0, "(Need %d base Mana)\n",it[in].mana[2]);
		return(-1);
	}
	if (it[in].min_rank>points2rank(ch[cn].points_tot))
	{
		do_char_log(cn, 0, "You're not experienced enough to use that.\n");
		do_char_log(cn, 0, "(Need a rank of %s)\n",rank_name[it[in].min_rank]);
		return(-1);
	}
	
	return(1);
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
		return( -1);
	}

	if (n<0 || n>19)
	{
		return( -1);        // sanity check

	}
	tmp = ch[cn].citem;

	// check prerequisites:
	if (tmp)
	{
		if (it[tmp].temp==IT_SEYANSWORD && it[tmp].data[0]!=cn)
		{
			do_char_log(cn, 0, "The goddess Kwai frowns at your attempt to use another one's %s.\n", it[tmp].reference);
			return(-1);
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
				return(-1);
			}
		}
		for (m = 0; m<5; m++)
		{
			if (it[tmp].attrib[m][2]>ch[cn].attrib[m][0])
			{
				do_char_log(cn, 0, "You're %s to use that.\n\n", at_text[m]);
				do_char_log(cn, 0, "(Need %d base %s)\n", it[tmp].attrib[m][2],at_names[m]);
				return(-1);
			}
		}
		for (m = 0; m<MAXSKILL; m++)
		{
			if (it[tmp].skill[m][2] && !ch[cn].skill[m][0])
			{
				do_char_log(cn, 0, "You don't know how to use that.\n");
				do_char_log(cn, 0, "(Need %s)\n",skilltab[m].name);
				return(-1);
			}
			if (it[tmp].skill[m][2]>ch[cn].skill[m][0])
			{
				do_char_log(cn, 0, "You're not skilled enough to use that.\n");
				do_char_log(cn, 0, "(Need %d base %s)\n",it[tmp].skill[m][2],skilltab[m].name);
				return(-1);
			}
		}
		if (it[tmp].hp[2]>ch[cn].hp[0])
		{
			do_char_log(cn, 0, "You don't have enough life force to use that.\n");
			do_char_log(cn, 0, "(Need %d base Hitpoints)\n",it[tmp].hp[2]);
			return(-1);
		}
		/*
		if (it[tmp].end[2]>ch[cn].end[0])
		{
			do_char_log(cn, 0, "You don't have enough endurance to use that.\n");
			return(-1);
		}
		*/
		if (it[tmp].mana[2]>ch[cn].mana[0])
		{
			do_char_log(cn, 0, "You don't have enough mana to use that.\n");
			do_char_log(cn, 0, "(Need %d base Mana)\n",it[tmp].mana[2]);
			return(-1);
		}

		if (((it[tmp].flags & IF_KWAI_UNI) && (it[tmp].flags & IF_GORN_UNI) && (ch[cn].kindred & KIN_PURPLE)) ||
		    ((it[tmp].flags & IF_PURP_UNI) && !(ch[cn].kindred & KIN_PURPLE)) ||
		    (it[tmp].driver==40 && !(ch[cn].kindred & KIN_SEYAN_DU)))
		{
			do_char_log(cn, 0, "Ouch. That hurt.\n");
			return(-1);
		}

		if (it[tmp].min_rank>points2rank(ch[cn].points_tot))
		{
			do_char_log(cn, 0, "You're not experienced enough to use that.\n");
			do_char_log(cn, 0, "(Need a rank of %s)\n",rank_name[it[tmp].min_rank]);
			return(-1);
		}

		// check for correct placement:
		switch(n)
		{
		case    WN_HEAD:
			if (!(it[tmp].placement & PL_HEAD))
			{
				return( -1);
			}
			else
			{
				break;
			}
		case    WN_NECK:
			if (!(it[tmp].placement & PL_NECK))
			{
				return( -1);
			}
			else
			{
				break;
			}
		case    WN_BODY:
			if (!(it[tmp].placement & PL_BODY))
			{
				return( -1);
			}
			else
			{
				break;
			}
		case    WN_ARMS:
			if (!(it[tmp].placement & PL_ARMS))
			{
				return( -1);
			}
			else
			{
				break;
			}
		case    WN_BELT:
			if (!(it[tmp].placement & PL_BELT))
			{
				return( -1);
			}
			else
			{
				break;
			}
		case    WN_CHARM:
		case    WN_CHARM2:
			if (!(it[tmp].placement & PL_CHARM))
			{
				return( -1);
			}
			else
			{
				break;
			}
		case    WN_FEET:
			if (!(it[tmp].placement & PL_FEET))
			{
				return( -1);
			}
			else
			{
				break;
			}
		case    WN_LHAND:
			if (!(it[tmp].placement & PL_SHIELD))
			{
				return( -1);
			}
			if ((in = ch[cn].worn[WN_RHAND])!=0 && (it[in].placement & PL_TWOHAND))
			{
				return( -1);
			}
			break;
		case    WN_RHAND:
			if (!(it[tmp].placement & PL_WEAPON))
			{
				return( -1);
			}
			if ((it[tmp].placement & PL_TWOHAND) && ch[cn].worn[WN_LHAND])
			{
				return( -1);
			}
			break;
		case    WN_CLOAK:
			if (!(it[tmp].placement & PL_CLOAK))
			{
				return( -1);
			}
			else
			{
				break;
			}
		case    WN_RRING:
		case    WN_LRING:
			if (!(it[tmp].placement & PL_RING))
			{
				return( -1);
			}
			else
			{
				break;
			}
		default:
			return( -1);
		}
	}
	
	// Special case for charms - cannot remove one without help from NPC Bishop
	if (tmp && (n == WN_CHARM || n == WN_CHARM2))
	{
		do_char_log(cn, 0, "You cannot equip a card yourself. Seek the Bishop in the Temple of Skua for assistance.\n");
		return( -1);
	}
	else if (n == WN_CHARM || n == WN_CHARM2)
	{
		do_char_log(cn, 0, "You cannot remove an equipped card yourself. Seek the Bishop in the Temple of Skua for assistance.\n");
		return( -1);
	}
	
	ch[cn].citem = ch[cn].worn[n];
	ch[cn].worn[n] = tmp;

	do_update_char(cn);

	return(n);
}

/* replaced by may_attack_message()
   int do_mayattack(int cn,int co)
   {
        int m1,m2,f;

        if (!(ch[cn].flags&CF_PLAYER)) return 1;
        if (!(ch[co].flags&CF_PLAYER)) return 1;

        m1=ch[cn].x+ch[cn].y*MAPX;
        m2=ch[co].x+ch[co].y*MAPX;
        f=map[m1].flags&map[m2].flags;  // make sure we take only flags present on both player's location

        if (f&MF_ARENA) return 1;

        if ((ch[cn].kindred&KIN_PURPLE) && (ch[co].kindred&KIN_PURPLE)) return 1;

        return 0;
   }
 */

/* Check if cn may attack co. if (msg), tell cn if not. */
int may_attack_msg(int cn, int co, int msg)
{
	int m1, m2;

	if (!IS_SANECHAR(cn) || !IS_SANECHAR(co))
	{
		return( 1);
	}

	// unsafe gods may attack anyone
	if ((ch[cn].flags & CF_GOD && !(ch[cn].flags & CF_SAFE)))
	{
		return(1);
	}

	// unsafe gods may be attacked by anyone!
	if ((ch[co].flags & CF_GOD && !(ch[co].flags & CF_SAFE)))
	{
		return(1);
	}

	// player GC? act as if he would try to attack the master of the GC instead
	if (IS_COMPANION(cn) && ch[cn].data[64]==0)
	{
		cn = ch[cn].data[63];
		if (!IS_SANECHAR(cn))
		{
			return( 1);             // um, lets him try to kill this GC - it's got bad values anway
		}
	}

	// NPCs may attack anyone, anywhere
	if (!IS_PLAYER(cn))
	{
		return( 1);
	}

	// Check for NOFIGHT
	m1 = XY2M(ch[cn].x, ch[cn].y);
	m2 = XY2M(ch[co].x, ch[co].y);
	if ((map[m1].flags | map[m2].flags) & MF_NOFIGHT)
	{
		if (msg)
		{
			do_char_log(cn, 0, "You can't attack anyone here!\n");
		}
		return(0);
	}

	// player GC? act as if he would try to attack the master of the GC instead
	if (IS_COMPANION(co))
	{
		co = ch[co].data[63];
		if (!IS_SANECHAR(co))
		{
			return( 1);             // um, lets him try to kill this GC - it's got bad values anway
		}
	}

	// Check for plr-npc (OK)
	if (!IS_PLAYER(cn) || !IS_PLAYER(co))
	{
		return( 1);
	}

	// Both are players. Check for Arena (OK)
	if (map[m1].flags & map[m2].flags & MF_ARENA)
	{
		return( 1);
	}

	// Check if aggressor is purple
	if (!IS_PURPLE(cn))
	{
		if (msg)
		{
			do_char_log(cn, 0, "You can't attack other players! You're not a follower of the Purple One.\n");
		}
		return(0);
	}

	// Check if victim is purple
	if (!IS_PURPLE(co))
	{
		if (msg)
		{
			do_char_log(cn, 0, "You can't attack %s! %s's not a follower of the Purple One.\n",
			            ch[co].name,
			            IS_FEMALE(co) ? "She" : "He");
		}
		return(0);
	}

	if (abs(points2rank(ch[cn].points_tot) - points2rank(ch[co].points_tot))>3)
	{
		if (msg)
		{
			do_char_log(cn, 0, "You're not allowed to attack %s. The rank difference is too large.\n", ch[co].name);
		}
		return(0);
	}

	return(1);
}

void do_check_new_level(int cn)
{
	int hp = 0, mana = 0, attri = 0, diff, rank, temp, n, oldrank;

	if (!IS_PLAYER(cn))
	{
		return;
	}

	rank = points2rank(ch[cn].points_tot);

	if (ch[cn].data[45]<rank)
	{
		chlog(cn, "gained level (%d -> %d)", ch[cn].data[45], rank);
		
		if (ch[cn].kindred & (KIN_MERCENARY | KIN_SORCERER | KIN_WARRIOR | KIN_SEYAN_DU))
		{
			hp   = 10;
			mana = 10;
		}
		else if (ch[cn].kindred & (KIN_TEMPLAR | KIN_ARCHTEMPLAR | KIN_BRAWLER))
		{
			hp   = 15;
			mana = 5;
		}
		else if (ch[cn].kindred & (KIN_HARAKIM | KIN_ARCHHARAKIM | KIN_SUMMONER))
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
		
		/* CS, 991203: Announce the player's new rank */
		temp = (ch[cn].kindred & KIN_PURPLE) ? CT_PRIEST : CT_BISHOP;
		// Find a character with appropriate template
		for (n = 1; n<MAXCHARS; n++)
		{
			if (ch[n].used!=USE_ACTIVE)
			{
				continue;
			}
			if (ch[n].flags & CF_BODY)
			{
				continue;
			}
			if (ch[n].temp == temp)
			{
				break;
			}
		}
		// Have him yell it out
		if (n<MAXCHARS)
		{
			char message[100];
			sprintf(message, "Hear ye, hear ye! %s has attained the rank of %s!",
			        ch[cn].name, rank_name[rank]);
			do_shout(n, message);
		}

		ch[cn].hp[1]   = hp * min(20,rank);
		ch[cn].mana[1] = mana * min(20,rank);
		if (attri)
		{
			temp = ch[cn].temp;
			for (n = 0; n<5; n++) 
				ch[cn].attrib[n][2] = ch_temp[temp].attrib[n][2] + attri * min(5, max(0,rank-19));
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
		last_date = max(ch[co].login_date, ch[co].logout_date) / (24 * 3600);
		current_date = time(NULL) / (24 * 3600);
		days = current_date - last_date;
		switch (days)
		{
			case 0:
				last_date = max(ch[co].login_date, ch[co].logout_date) / (1 * 3600);
				current_date = time(NULL) / (1 * 3600);
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
	if (!IS_PURPLE(cn))
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

	ch[cn].data[CHD_ATTACKTIME] = globs->ticker;
	ch[cn].data[CHD_ATTACKVICT] = co;
}

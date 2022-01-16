/*************************************************************************

   This file is part of 'Mercenaries of Astonia v2'
   Copyright (c) 1997-2001 Daniel Brockhaus (joker@astonia.com)
   All rights reserved.

 **************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/mman.h>
#include <errno.h>

#include "gendefs.h"
#include "data.h"
#include "driver.h"
#include "macros.h"

struct map *map;
struct character *ch;
struct item *bu;
struct item *it;
struct effect *fx;
struct global *globs;

static int load(void)
{
	int handle;

	handle = open(DATDIR "/map.dat", O_RDONLY);
	if (handle==-1)
	{
		fprintf(stderr, "map.dat does not exist.\n");
		return -1;
	}

	map = mmap(NULL, MAPSIZE, PROT_READ, MAP_SHARED, handle, 0);
	if (map==(void*)-1)
	{
		fprintf(stderr, "cannot mmap map.dat.\n");
		return -1;
	}
	close(handle);

	handle = open(DATDIR "/char.dat", O_RDONLY);
	if (handle==-1)
	{
		fprintf(stderr, "char.dat does not exist.\n");
		return -1;
	}

	ch = mmap(NULL, CHARSIZE, PROT_READ, MAP_SHARED, handle, 0);
	if (ch==(void*)-1)
	{
		fprintf(stderr, "cannot mmap char.dat.\n");
		return -1;
	}
	close(handle);

	//
	handle = open(DATDIR "/buff.dat", O_RDONLY);
	if (handle==-1)
	{
		fprintf(stderr, "buff.dat does not exist.\n");
		return -1;
	}

	bu = mmap(NULL, BUFFSIZE, PROT_READ, MAP_SHARED, handle, 0);
	if (bu==(void*)-1)
	{
		fprintf(stderr, "cannot mmap buff.dat.\n");
		return -1;
	}
	close(handle);
	//

	handle = open(DATDIR "/item.dat", O_RDONLY);
	if (handle==-1)
	{
		fprintf(stderr, "item.dat does not exist.\n");
		return -1;
	}

	it = mmap(NULL, ITEMSIZE, PROT_READ, MAP_SHARED, handle, 0);
	if (it==(void*)-1)
	{
		fprintf(stderr, "cannot mmap item.dat.\n");
		return -1;
	}
	close(handle);

	handle = open(DATDIR "/effect.dat", O_RDONLY);
	if (handle==-1)
	{
		fprintf(stderr, "effect.dat does not exist.\n");
		return -1;
	}

	fx = mmap(NULL, EFFECTSIZE, PROT_READ, MAP_SHARED, handle, 0);
	if (fx==(void*)-1)
	{
		fprintf(stderr, "cannot mmap effect.dat.\n");
		return -1;
	}
	close(handle);

	handle = open(DATDIR "/global.dat", O_RDONLY);
	if (handle==-1)
	{
		fprintf(stderr, "global.dat does not exist.\n");
		return -1;
	}

	globs = mmap(NULL, sizeof(struct global), PROT_READ, MAP_SHARED, handle, 0);
	if (globs==(void*)-1)
	{
		fprintf(stderr, "cannot mmap global.dat.\n");
		return -1;
	}
	close(handle);

	return 0;
}

static void unload(void)
{
	if (munmap(map, MAPSIZE))
	{
		perror("munmap(map)");
	}
	if (munmap(ch, CHARSIZE))
	{
		perror("munmap(ch)");
	}
	if (munmap(it, ITEMSIZE))
	{
		perror("munmap(it)");
	}
	if (munmap(fx, EFFECTSIZE))
	{
		perror("munmap(fx)");
	}
	if (munmap(globs, sizeof(struct global)))
	{
		perror("munmap(globs)");
	}
}

static char *_rank[25] = {
	"Private",
	"Private First Class",
	"Lance Corporal",
	"Corporal",
	"Sergeant",
	"Staff Sergeant",
	"Master Sergeant",
	"First Sergeant",
	"Sergeant Major",
	"Second Lieutenant",
	"First Lieutenant",
	"Captain",
	"Major",
	"Lieutenant Colonel",
	"Colonel",
	"Brigadier General",
	"Major General",
	"Lieutenant General",
	"General",
	"Field Marshal",
	"Knight of Astonia",
	"Baron of Astonia",
	"Earl of Astonia",
	"Marquess of Astonia",
	"Warlord of Astonia"
};

static int points2rank(int v)
{
	if (v<      250)	return 0; // Private
	if (v<     1750)	return 1; // Private FIrst Class
	if (v<     7000)	return( 2); // Lance Corporal
	if (v<    21000)	return( 3); // Corporal
	if (v<    52500)	return( 4); // Sergeant
	if (v<   115500)	return( 5); // Staff Sergeant
	if (v<   231000)	return( 6); // Master Sergeant
	if (v<   429000)	return( 7); // First Sergeant
	if (v<   750750)	return( 8); // Sergeant Major
	if (v<  1251250)	return( 9); // Second Leiutenant
	if (v<  2002000)	return(10); // First Leiutenant
	if (v<  3094000)	return(11); // Captain
	if (v<  4641000)	return(12); // Major
	if (v<  6783000)	return(13); // Lieutenant Colonel
	if (v<  9690000)	return(14); // Colonel
	if (v< 13566000)	return(15); // Brigadier General
	if (v< 18653250)	return(16); // Major General
	if (v< 25236750)	return(17); // Lieutenant General
	if (v< 33649000)	return(18); // General
	if (v< 44275000)	return(19); // Field Marshal
	if (v< 57557500)	return(20); // Knight
	if (v< 74002500)	return(21); // Baron
	if (v< 94185000)	return(22); // Earl
	if (v<118755000)	return(22); // Earl
						return(23); // Warlord
}

static char *rank(int pts)
{
	return(_rank[points2rank(pts)]);
}

static char *racename(int kin)
{
	if (kin & KIN_MERCENARY)	return("Mercenary");
	if (kin & KIN_TEMPLAR)		return("Templar");
	if (kin & KIN_HARAKIM)		return("Harakim");
	if (kin & KIN_SEYAN_DU)		return("Seyan'Du");
	if (kin & KIN_ARCHTEMPLAR)	return("Archtemplar");
	if (kin & KIN_ARCHHARAKIM)	return("Archharakim");
	if (kin & KIN_SORCERER)		return("Sorcerer");
	if (kin & KIN_WARRIOR)		return("Warrior");
	if (kin & KIN_SKALD)		return("Skald");
	if (kin & KIN_SUMMONER)		return("Summoner");
	if (kin & KIN_BRAVER)		return("Braver");

	return("Monster");
}

static void who(void)
{
	int n, shown = 0;

	printf("<b>Active Players</b><br><br>\n");

	printf("<table cellpadding=0 cellspacing=0 border=0>\n");
	printf("<tr><td>Name &nbsp; &nbsp;</td><td>Race &nbsp; &nbsp;</td><td>Rank &nbsp; &nbsp;</td><td>Active</td></tr>\n");
	printf("<tr><td colspan=4><hr></td></tr>");

	for (n = 1; n<MAXCHARS && shown<50; n++)
	{
		if (ch[n].used!=USE_ACTIVE)
		{
			continue;
		}
		if (!(ch[n].flags & (CF_PLAYER)))
		{
			continue;
		}
		if (ch[n].flags & (CF_INVISIBLE | CF_NOLIST | CF_NOWHO))
		{
			continue;
		}

		printf("<tr><td><a href=\"/cgi-bin/info.cgi?cn=%d\">%s</a> &nbsp; &nbsp; </td><td>%s &nbsp; &nbsp; </td><td>%s &nbsp; &nbsp; </td><td>%s</td></tr>\n",
		       n, ch[n].name, racename(ch[n].kindred), rank(ch[n].points_tot), (ch[n].used==USE_ACTIVE ? "Yes" : "No"));
		shown++;
	}
	if (shown==0)
	{
		printf("<tr><td colspan=4>No players active</td></tr>\n");
	}
	printf("</table><br>\n");
	printf("<ul>\n");
	printf("<li><a href=\"/cgi-bin/info.cgi?top\">Top Players</a></li>");
	printf("<li><a href=\"/cgi-bin/info.cgi?gods\">Gods</a></li>");
	printf("<li><a href=\"/cgi-bin/info.cgi?staff\">Staff</a></li>");
	printf("<li><a href=\"/cgi-bin/info.cgi?xtop\">Top Experience Players</a></li>");
	printf("<li><a href=\"/cgi-bin/info.cgi?dtop\">Top Deaths Players</a></li>");
	printf("</ul>\n");

}

static void gods(void)
{
	int n, shown = 0;

	printf("<b>Ye Gods</b><br><br>\n");

	printf("<table cellpadding=0 cellspacing=0 border=0>\n");
	printf("<tr><td>Name &nbsp; &nbsp;</td><td>Race &nbsp; &nbsp;</td><td>Rank &nbsp; &nbsp;</td><td>Active</td></tr>\n");
	printf("<tr><td colspan=4><hr></td></tr>");

	for (n = 1; n<MAXCHARS && shown<50; n++)
	{
		if (ch[n].used==USE_EMPTY)
		{
			continue;
		}
		if (!(ch[n].flags & (CF_PLAYER)))
		{
			continue;
		}
		if (!(ch[n].flags & CF_GOD))
		{
			continue;
		}
		if (ch[n].flags & CF_NOLIST)
		{
			continue;
		}

		printf("<tr><td><a href=\"/cgi-bin/info.cgi?cn=%d\">%s</a> &nbsp; &nbsp; </td><td>%s &nbsp; &nbsp; </td><td>%s &nbsp; &nbsp; </td><td>%s</td></tr>\n",
		       n, ch[n].name, racename(ch[n].kindred), rank(ch[n].points_tot),
		       (ch[n].used==USE_ACTIVE && !(ch[n].flags & (CF_INVISIBLE | CF_NOWHO)) ? "Yes" : "No"));
		shown++;
	}
	if (shown==0)
	{
		printf("<tr><td colspan=4>Despair: There are no gods!</td></tr>\n");
	}
	printf("</table><br>\n");
}

static void staff(void)
{
	int n, shown = 0;

	printf("<b>Ye Staff</b><br><br>\n");

	printf("<table cellpadding=0 cellspacing=0 border=0>\n");
	printf("<tr><td>Name &nbsp; &nbsp;</td><td>Race &nbsp; &nbsp;</td><td>Rank &nbsp; &nbsp;</td><td>Active</td></tr>\n");
	printf("<tr><td colspan=4><hr></td></tr>");

	for (n = 1; n<MAXCHARS && shown<50; n++)
	{
		if (ch[n].used==USE_EMPTY)
		{
			continue;
		}
		if (!(ch[n].flags & (CF_PLAYER)))
		{
			continue;
		}
		if (ch[n].flags & CF_GOD)
		{
			continue;
		}
		if (!(ch[n].flags & CF_STAFF))
		{
			continue;
		}

		printf("<tr><td><a href=\"/cgi-bin/info.cgi?cn=%d\">%s</a> &nbsp; &nbsp; </td><td>%s &nbsp; &nbsp; </td><td>%s &nbsp; &nbsp; </td><td>%s</td></tr>\n",
		       n, ch[n].name, racename(ch[n].kindred), rank(ch[n].points_tot),
		       (ch[n].used==USE_ACTIVE && !(ch[n].flags & (CF_INVISIBLE | CF_NOWHO)) ? "Yes" : "No"));
		shown++;
	}
	if (shown==0)
	{
		printf("<tr><td colspan=4>No staffers yet.</td></tr>\n");
	}
	printf("</table><br>\n");
}

static void top(void)
{
	int n, m;
	int b[25], nr[25];

	for (m = 0; m<25; m++)
	{
		b[m]  = -1;
		nr[m] = -1;
	}

	printf("<b>Top Characters</b><br><br>\n");
	for (n = 1; n<MAXCHARS; n++)
	{
		if (ch[n].used==USE_EMPTY)
		{
			continue;
		}
		if (!(ch[n].flags & (CF_PLAYER)))
		{
			continue;
		}
		if (ch[n].flags & (CF_GOD | CF_NOLIST))
		{
			continue;
		}

		for (m = 0; m<25; m++)
		{
			if (ch[n].points_tot>b[m])
			{
				if (m<24)
				{
					memmove(&b[m + 1], &b[m], (24 - m) * sizeof(int));
					memmove(&nr[m + 1], &nr[m], (24 - m) * sizeof(int));
				}
				b[m]  = ch[n].points_tot;
				nr[m] = n;
				break;
			}
		}
	}

	printf("<table cellpadding=0 cellspacing=0 border=0>\n");
	printf("<tr><td>Name &nbsp; &nbsp;</td><td>Race &nbsp; &nbsp;</td><td>Rank &nbsp; &nbsp;</td><td>Active</td></tr>\n");
	printf("<tr><td colspan=4><hr></td></tr>");
	for (m = 0; m<25; m++)
	{
		if (nr[m]==-1)
		{
			continue;
		}
		printf("<tr><td><a href=\"/cgi-bin/info.cgi?cn=%d\">%s</a> &nbsp; &nbsp; </td><td>%s &nbsp; &nbsp; </td><td>%s &nbsp; &nbsp; </td><td>%s</td></tr>\n",
		       nr[m], ch[nr[m]].name, racename(ch[nr[m]].kindred), rank(ch[nr[m]].points_tot),
		       (ch[nr[m]].used==USE_ACTIVE && !(ch[nr[m]].flags & (CF_INVISIBLE | CF_NOWHO)) ? "Yes" : "No"));
	}
	printf("</table><br>\n");
}

static void xtop(int srank)
{
	int n, m;
	int b[25], nr[25];
	int ppm[MAXCHARS];

	for (m = 1; m<MAXCHARS; m++)
	{
		ppm[m] = (ch[m].points_tot + 1) / ((ch[m].total_online_time / TICKS / 60) + 1);
	}

	for (m = 0; m<25; m++)
	{
		b[m]  = -1;
		nr[m] = -1;
	}

	printf("<b>Top Characters</b><br><br>\n");
	for (n = 1; n<MAXCHARS; n++)
	{
		if (ch[n].used==USE_EMPTY)
		{
			continue;
		}
		if (!(ch[n].flags & (CF_PLAYER)))
		{
			continue;
		}
		if (ch[n].flags & (CF_GOD | CF_NOLIST))
		{
			continue;
		}
		if (srank && points2rank(ch[n].points_tot)!=srank)
		{
			continue;
		}

		for (m = 0; m<25; m++)
		{
			if (ppm[n]>b[m])
			{
				if (m<24)
				{
					memmove(&b[m + 1], &b[m], (24 - m) * sizeof(int));
					memmove(&nr[m + 1], &nr[m], (24 - m) * sizeof(int));
				}
				b[m]  = ppm[n];
				nr[m] = n;
				break;
			}
		}
	}

	printf("<table cellpadding=0 cellspacing=0 border=0>\n");
	printf("<tr><td>Name &nbsp; &nbsp;</td><td>Race &nbsp; &nbsp;</td><td>Rank &nbsp; &nbsp;</td><td>Active &nbsp; &nbsp; </td><td>PPM</td></tr>\n");
	printf("<tr><td colspan=5><hr></td></tr>");
	for (m = 0; m<25; m++)
	{
		if (nr[m]==-1)
		{
			continue;
		}
		printf("<tr><td><a href=\"/cgi-bin/info.cgi?cn=%d\">%s</a> &nbsp; &nbsp; </td><td>%s &nbsp; &nbsp; </td><td>%s &nbsp; &nbsp; </td><td>%s &nbsp; &nbsp; </td><td>%d</td></tr>\n",
		       nr[m], ch[nr[m]].name, racename(ch[nr[m]].kindred), rank(ch[nr[m]].points_tot),
		       (ch[nr[m]].used==USE_ACTIVE && !(ch[nr[m]].flags & (CF_INVISIBLE | CF_NOWHO)) ? "Yes" : "No"), ppm[nr[m]]);
	}
	printf("</table><br>\n");
}

static void dtop(int srank)
{
	int n, m;
	int b[25], nr[25];

	for (m = 0; m<25; m++)
	{
		b[m]  = 99999999;
		nr[m] = -1;
	}

	printf("<b>Top Characters</b><br><br>\n");
	for (n = 1; n<MAXCHARS; n++)
	{
		if (ch[n].used==USE_EMPTY)
		{
			continue;
		}
		if (!(ch[n].flags & (CF_PLAYER)))
		{
			continue;
		}
		if (ch[n].flags & (CF_GOD | CF_NOLIST))
		{
			continue;
		}
		if (srank && points2rank(ch[n].points_tot)!=srank)
		{
			continue;
		}

		for (m = 0; m<25; m++)
		{
			if (ch[n].data[14] + ch[n].data[44]<b[m])
			{
				if (m<24)
				{
					memmove(&b[m + 1], &b[m], (24 - m) * sizeof(int));
					memmove(&nr[m + 1], &nr[m], (24 - m) * sizeof(int));
				}
				b[m]  = ch[n].data[14] + ch[n].data[44];
				nr[m] = n;
				break;
			}
		}
	}

	printf("<table cellpadding=0 cellspacing=0 border=0>\n");
	printf("<tr><td>Name &nbsp; &nbsp;</td><td>Race &nbsp; &nbsp;</td><td>Rank &nbsp; &nbsp;</td><td>Active &nbsp; &nbsp; </td><td>Deaths+Saves</td></tr>\n");
	printf("<tr><td colspan=5><hr></td></tr>");
	for (m = 0; m<25; m++)
	{
		if (nr[m]==-1)
		{
			continue;
		}
		printf("<tr><td><a href=\"/cgi-bin/info.cgi?cn=%d\">%s</a> &nbsp; &nbsp; </td><td>%s &nbsp; &nbsp; </td><td>%s &nbsp; &nbsp; </td><td>%s &nbsp; &nbsp; </td><td>%d</td></tr>\n",
		       nr[m], ch[nr[m]].name, racename(ch[nr[m]].kindred), rank(ch[nr[m]].points_tot),
		       (ch[nr[m]].used==USE_ACTIVE && !(ch[nr[m]].flags & (CF_INVISIBLE | CF_NOWHO)) ? "Yes" : "No"), ch[nr[m]].data[14] + ch[nr[m]].data[44]);
	}
	printf("</table><br>\n");
}

static void char_info(int cn)
{
	int n;
	char *gender, *Gender;

	if (!(ch[cn].flags & CF_PLAYER))
	{
		printf("Access denied!");
		return;
	}

	if (IS_MALE(cn))
	{
		gender = "he";
		Gender = "He";
	}
	else
	{
		gender = "she";
		Gender = "She";
	}

	printf("%s <b>%s</b><br><br>", rank(ch[cn].points_tot), ch[cn].name);
	printf("%s<br><br>", ch[cn].description);

	if (!(ch[cn].flags & CF_GOD))
	{
		printf("%s died %d times, was saved by the gods %d times and\n",
		       ch[cn].name,
		       ch[cn].data[14],
		       ch[cn].data[44]);
		printf("solved %d parts of the Labyrinth.\n",
		       ch[cn].data[20]);

		if (ch[cn].data[25]>ch[cn].data[24] + ch[cn].data[23])
		{
			printf("%s is said to be very brave.\n", Gender);
		}
		else if (ch[cn].data[25] + ch[cn].data[24]>ch[cn].data[23])
		{
			printf("%s is said to be brave.\n", Gender);
		}
		else
		{
			printf("%s is said to be cautious.\n", Gender);
		}

		n = (ch[cn].gold + ch[cn].data[13]) / 100;

		if (n>1000000)
		{
			printf("Rumor has it %s is extremely rich.\n", ch[cn].name);
		}
		else if (n>100000)
		{
			printf("Rumor has it %s is very rich.\n", ch[cn].name);
		}
		else if (n>10000)
		{
			printf("Rumor has it %s is rich.\n", ch[cn].name);
		}
		else if (n>1000)
		{
			printf("Rumor has it %s is fairly rich.\n", ch[cn].name);
		}

		if (IS_PURPLE(cn))
		{
			printf("%s is a follower of the Purple One and killed %d fellow players.\n",
			       Gender, ch[cn].data[29]);
		}
	}
}

/*static char *stars(int cnt)
   {
        static char buf[80];

        for (buf[cnt+1]=0; cnt>-1; cnt--) buf[cnt]='X';

        return buf;
   }*/

#define T_WEEK     (60*60*24*7)
#define ALLOWED_GB (35.0)

static void info(int cn)
{
	int cap;
	time_t t;
	double proz1, proz2;

	if (cn)
	{
		char_info(cn);
		return;
	}

	t = time(NULL);

	printf("<b>Statistics</b><br><br>\n");

	printf("<table>");

	printf("<tr><td colspan=3><hr></td></tr>");
	printf("<tr><td>Characters: </td><td> %d </td><td> (%4.0f%%)</td></tr>", globs->character_cnt, 100.0 / MAXCHARS * globs->character_cnt);
	printf("<tr><td>Awake: </td><td> %d </td><td> (%4.0f%%)</td></tr>", globs->awake, 100.0 / globs->character_cnt * globs->awake);
	printf("<tr><td>Items: </td><td> %d </td><td> (%4.0f%%)</td></tr>", globs->item_cnt, 100.0 / MAXITEM * globs->item_cnt);
	printf("<tr><td>Effects: </td><td> %d </td><td> (%4.0f%%)</td></tr>", globs->effect_cnt, 100.0 / MAXEFFECT * globs->effect_cnt);
	printf("<tr><td colspan=3><hr></td></tr>");

	proz1 = 100.0 / ((t - globs->transfer_reset_time) * (ALLOWED_GB / 30 / 24 / 60 / 60)) * ((globs->recv + globs->send) / (1024.0 * 1024 * 1024));
	proz2 = globs->load_avg / 100.0;

	//printf("<tr><td>Bandwidth usage: </td><td> %.4fGB recv, %.4fGB send</td></tr>",(globs->recv/(1024.0*1024.0*1024.0)),(globs->send/(1024.0*1024.0*1024.0)));
	printf("<tr><td>Allowed traffic: </td><td> %.3fGB</td></tr>", (double)(t - globs->transfer_reset_time) * (ALLOWED_GB / 30 / 24 / 60 / 60));
	printf("<tr><td>Actual traffic: </td><td> %.3fGB </td><td> (%.2f%%)</td></tr>", (globs->recv + globs->send) / (1024.0 * 1024 * 1024), proz1);
	printf("<tr><td colspan=3><hr></td></tr>");

	printf("<tr><td>CPU usage: </td><td> (current)</td><td> %ld%% </td></tr>", globs->load);
	printf("<tr><td>CPU usage: </td><td> (avg)</td><td> %.2f%% </td></tr>", proz2);
	printf("<tr><td colspan=3><hr></td></tr>");

	/*cap=globs->cap*100/proz1;

	   printf("<tr><td>Suggested CAP: </td><td> %d</td></tr>",cap);
	   printf("<tr><td colspan=3><hr></td></tr>");*/

	printf("<tr><td>Uptime: </td><td>%lldh %lldm %llds</td></tr>",
	       (globs->uptime / (TICKS * 60 * 60)),
	       (globs->uptime / (TICKS * 60) % 60),
	       (globs->uptime / (TICKS)) % 60);
	//printf("<tr><td>Total online time: </td><td>%lldh %lldm %llds</td></tr>",(globs->total_online_time/(TICKS*60*60)),(globs->total_online_time/(TICKS*60)%60),(globs->total_online_time/(TICKS))%60);
	printf("<tr><td colspan=3><hr></td></tr>");

	printf("<tr><td>Players online: </td><td> %d</td></tr>", globs->players_online);
	if (globs->uptime && globs->total_online_time)
	{
		printf("<tr><td>Average players online: </td><td>%.2f</td></tr>\n",
		       (float)(globs->total_online_time) / (float)(globs->uptime));
	}
	if (globs->queuesize>0)
	{
		printf("<tr><td>Players waiting for login: </td><td> %d</td></tr>", globs->queuesize);
	}
	else
	{
		printf("<tr><td>Free player slots: </td><td> %d</td></tr>", -globs->queuesize);
	}
	printf("<tr><td colspan=3><hr></td></tr>");

	printf("</table><br><br>");

	printf("Today is the %d%s%s%s%s day of the Year %d. It is %d:%02d Astonian Standard Time.<br><br>",
	       globs->mdday,
	       (globs->mdday==1 ? "st" : ""),
	       (globs->mdday==2 ? "nd" : ""),
	       (globs->mdday==3 ? "rd" : ""),
	       (globs->mdday>3 ? "th" : ""),
	       globs->mdyear, globs->mdtime / 3600, (globs->mdtime / 60) % 60);

	if (globs->mdday % 28 + 1==1)
	{
		printf("New Moon tonight!<br><br>");
	}
	else if (globs->mdday % 28 + 1<15)
	{
		printf("The Moon is growing.<br><br>");
	}
	else if (globs->mdday % 28 + 1==15)
	{
		printf("Full Moon tonight!<br><br>");
	}
	else
	{
		printf("The moon is dwindling.<br><br>");
	}

	fflush(stdout);
	cap = globs->ticker;
	sleep(1);
	if (cap==globs->ticker)
	{
		printf("Server seems to be down.");
	}
	else
	{
		printf("Server seems to be up.");
	}
}

static void hog(void)
{
	int n, m;
	int b[25], nr[25];

	for (m = 0; m<25; m++)
	{
		b[m]  = -1;
		nr[m] = -1;
	}

	printf("<b>Top CPU Usage</b><br><br>\n");
	for (n = 1; n<MAXCHARS; n++)
	{
		if (ch[n].used==USE_EMPTY)
		{
			continue;
		}

		for (m = 0; m<25; m++)
		{
			if (ch[n].data[98]>b[m])
			{
				if (m<24)
				{
					memmove(&b[m + 1], &b[m], (24 - m) * sizeof(int));
					memmove(&nr[m + 1], &nr[m], (24 - m) * sizeof(int));
				}
				b[m]  = ch[n].data[98];
				nr[m] = n;
				break;
			}
		}
	}

	printf("<table cellpadding=0 cellspacing=0 border=0>\n");
	printf("<tr><td>Nr &nbsp; &nbsp;</td><td>Name &nbsp; &nbsp;</td><td>Time &nbsp; &nbsp;</td></tr>\n");
	printf("<tr><td colspan=4><hr></td></tr>");
	for (m = 0; m<25; m++)
	{
		if (nr[m]==-1)
		{
			continue;
		}
		printf("<tr><td>%d &nbsp; &nbsp; </td><td>%s &nbsp; &nbsp; </td><td>%d &nbsp; &nbsp; </td></tr>\n",
		       nr[m], ch[nr[m]].name, ch[nr[m]].data[98]);
	}
	printf("</table><br>\n");

}

void effects(void)
{
	int n;

	for (n = 0; n<MAXEFFECT; n++)
	{
		if (fx[n].used!=USE_ACTIVE)
		{
			continue;
		}
		printf("%d: Type=%d, Dur=%d, Data=%d,%d,%d<br>\n",
		       n, fx[n].type, fx[n].duration, fx[n].data[0], fx[n].data[1], fx[n].data[2]);
	}
}

/*static char *xname(char *ptr)
   {
        int len;
        static char buf[80],text[120];

        if (*ptr=='*') {
                len=strlen(ptr);
                memcpy(buf,ptr+1,len-2);
                buf[len-2]=0;
                sprintf(text,"<b>%s</b>",buf);
                return text;
        }
        return ptr;
   }*/

static char *query;

int main(int argc, char *args[])
{
	int cn = 0, srank = 0;

	query = getenv("QUERY_STRING");
	if (query && !strncmp(query, "cn=", 3))
	{
		cn = atoi(query + 3);
	}
	if (query && !strncmp(query, "rank=", 5))
	{
		srank = atoi(query + 5);
	}

	chdir("/home/merc");

	printf("Content-Type: text/html\n\n");
	printf("<html><head><title>Server Info</title></head>\n");
	printf("<BODY TEXT=#D7D700 BGCOLOR=#264A9F LINK=#FFFFBB VLINK=#CCCC00 ALINK=#FFFF9D background=/gfx/back4.gif>\n");
	printf("<center>\n");
	printf("<table width=\"100%%\">\n<tr>\n");
	printf(
		"    <table width=\"100%%\">\n"
		"        <tr>\n"
		"            <td align=\"center\"><a href=\"https://github.com/dylanyaga/openMerc/\"><img src=\"/gfx/logo.gif\" width=\"100\" height=\"60\" border=\"0\"></a></td>\n"
		"            <td align=\"center\">\n"
		"                <a href=\"/\">Home</a>\n"
		"                <a href=\"/manual.html\">Manual</a>\n"
		"                <a href=\"/terms.html\">Terms</a>\n"
		"                <a href=\"/download.html\">Download</a>\n"
		"                <a href=\"/contact.html\">Contact</a>\n"
		"                <a href=\"/cgi-bin/info.cgi\">Server&nbsp;Status</a>\n"
		"                <a href=\"/cgi-bin/info.cgi?who\">Who's&nbsp;Online</a>\n"
		"                <a href=\"/bugs.html\">Bugs</a>\n"
		"                <a href=\"/changes.html\">Changes</a>\n"
		"                <a href=\"/creators.html\">Creators</a>\n"
		"                <a href=\"/links.html\">Links</a>\n"
		"                <a href=\"/privacy.html\">Privacy</a>\n"
		"            </td>\n"
		"            <td align=\"center\"><a href=\"https://github.com/dylanyaga/openMerc/\"><img src=\"/gfx/logo.gif\" width=\"100\" height=\"60\" border=\"0\"></a></td>\n"
		"        </tr>\n"
		"    </table>");
	printf("</td></tr></table>");
	printf("<hr width=80%% color=\"#808000\"><br>\n");
	printf("<table width=60%%><tr><td>\n");

	if (load())
	{
		printf("<b>Cannot access server data. Exiting... (%s)</b></td></tr></table>", strerror(errno));
		exit(0);
	}

	if (!args[1])
	{
		info(cn);
	}
	if (args[1])
	{
		if (strcmp(args[1], "who")==0)
		{
			who();
		}
		else if (strcmp(args[1], "top")==0)
		{
			top();
		}
		else if (strcmp(args[1], "hog")==0)
		{
			hog();
		}
		else if (strcmp(args[1], "gods")==0)
		{
			gods();
		}
		else if (strcmp(args[1], "staff")==0)
		{
			staff();
		}
		else if (strcmp(args[1], "effects")==0)
		{
			effects();
		}
		else if (strcmp(args[1], "xtop")==0)
		{
			xtop(srank);
		}
		else if (strcmp(args[1], "dtop")==0)
		{
			dtop(srank);
		}
		else
		{
			printf("Internal error... (%s)\n", args[1]);
		}
	}

	unload();

	printf("</td></tr></table><br>");

	printf(
		"<hr width=80%% color=\"#808000\"><br>\n\n"
		"All material on this server is based on the Mercenaries of Astonia engine by Daniel Brockhaus.\n"
		"</center><br><br>\n"
		"</body></html>");

	return 0;
}

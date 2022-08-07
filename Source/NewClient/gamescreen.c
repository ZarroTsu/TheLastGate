// **************************************************************** 
//
//  The Last Gate - SDL2 Client by Zachary "ZarroTsu" Baillie
//  Based on the Mercenaries of Astonia engine by Daniel Brockhaus
//  
//  gamescreen.c
//  Core gameplay state; displays game GUI and gameplay
//
// **************************************************************** 
#include "common.h"
#include "gamescreen.h"

void GAM_AddLog(char *text, char font)
{
	int n, panic = 0;
	static int flag = 0;

	if (!flag) 
	{
		for (n = 0; n < XLL*60; n++) 
		{
			logtext[0][n] = 0;
		}
		for (n = 0; n < XLL; n++) 
		{
			logfont[n] = 0;
		}
		flag = 1;
	}

	if (strlen(text) < 1)	return;
	
	// This loop moves previous line of text into the next line of text
	// with line breaks in mind, this arranges the text lines backwards.
	while (panic++ < XLL) 
	{
		memmove(logtext[1], logtext[0], XLL*60-60);
		memmove(&logfont[1], &logfont[0], XLL-1);
		memcpy(logtext[0], text, min(60-1, strlen(text)+1));
		logfont[0] = font;
		logtext[0][60-1] = 0;
		if (strlen(text) < XS-1) return;
		for (n = XS-1; n>0; n--) if (logtext[0][n] == ' ') break;
		if (n != 0) 
		{
			logtext[0][n] = 0; text += n+1;
		} 
		else
		{
			text += XS-1;
		}
	}
}

void GAM_AddMOTD(char *text, char font)
{
	int n, panic = 0;
	static int flag = 0;
	static int currline = 0;
	
	if (!flag) 
	{
		for (n=0; n<MLL*60; n++) 
		{
			motdtext[0][n] = 0;
		}
		for (n=0; n<MLL; n++) 
		{
			motdfont[n] = 0;
		}
		flag = 1;
	}

	if (strlen(text) < 1) return;
	
	// Tracking the text array with currline, this should pass to the next line
	// this will go until currline exceeds 60. As a failsafe, it then resets to 0.
	while (panic++ < MLL)
	{
		if (currline >= 59) currline = 0;
		
		memcpy(motdtext[currline], text, min(60-1, strlen(text)+1));
		motdfont[currline] = font;
		motdtext[currline][60-1] = 0;
		
		if (strlen(text) < XMS-1) break;
		for (n = XMS-1; n > 0; n--) if (motdtext[currline][n] == ' ') break;
		if (n!=0) 
		{
			motdtext[currline][n] = 0;
			text += n+1;
		} 
		else
		{
			text += XMS-1;
		}
		currline++;
	}
	currline++;
}

void GAM_Log(char font, char *format, ...)
{
	va_list args;
	char buf[1024];

	va_start(args, format);
	vsprintf(buf, format, args);
	GAM_AddLog(buf, font);
	va_end(args);
}

void GAM_MOTD(char font, char *format, ...)
{
	va_list args;
	char buf[1024];

	va_start(args, format);
	vsprintf(buf, format, args);
	GAM_AddMOTD(buf, font);
	va_end(args);
}

void GAM_Init(void)
{
	int n;
	
	// Initialize Map
	map = calloc(RENDERDIST*RENDERDIST*sizeof(struct cmap), 1);

	for (n = 0; n < RENDERDIST*RENDERDIST; n++)
	{
		map[n].ba_sprite = SPR_EMPTY;
	}
	
	// Initialize Player
	memset(&pl, 0, sizeof(struct cplayer));
	
	GAM_Log(1, "GAM_Init: OK\n");
	GAM_Log(2, "Render GAM_Log: OK\n");
}

void GAM_Update(void) {}

void GAM_RenderGUI(void)
{
	int y;
	
	// Draw the GUI background image
	DRA_Sprite(1, 0, 0);
	
	DRA_Text(442, 114, 1, "Money");
	
	DRA_Text(144, 260, 1, "Update");
	
	DRA_Text(1112, 261, 1, "Weapon Value");
	DRA_Text(1112, 261+1*14, 1, "Armor Value");
	DRA_Text(1112, 261+2*14, 1, "Experience");
	
	DRA_Text(9, 8+5*14, 1, "Hitpoints");
	DRA_Text(9, 8+6*14, 1, "Mana");
	
	DRA_Rectangle(DRA_NewRectangle(1109, 300, 165, 6), COLOR_GUI_BLU);
	DRA_Rectangle(DRA_NewRectangle(1109, 300, 100, 6), COLOR_GUI_YEL);
	
	for (y=0; y<LL; y++) 
	{
		DRA_Text(977, 8+y*10, logfont[LL-y-1+logstart], logtext[LL-y-1+logstart]);
	}
}

void GAM_Render(void) 
{
	GAM_RenderGUI();
}

void GAM_Clean(void) {}

/* END OF FILE */
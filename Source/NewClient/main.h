// **************************************************************** 
//
//  The Last Gate - SDL2 Client by Zachary "ZarroTsu" Baillie
//  Based on the Mercenaries of Astonia engine by Daniel Brockhaus
//  
//  main.h
//  Header information specific to main.c
//
// **************************************************************** 
#ifndef MAIN_H
#define MAIN_H

// Client application name and server host - this may be different when launched locally.
// HOMECOPY is defined in common.h
#ifdef HOMECOPY
	#define APP_NAME		"The Last Gate Dev"
	#define SRV_HOST		"10.0.0.66"
#else // HOMECOPY
	#define APP_NAME		"The Last Gate"
	#define SRV_HOST		"thelastgate.ddns.net"
#endif // HOMECOPY

// Application version - Checked by server to exclude obsolete clients
#define APP_VERSION			0x000900

// Application links - Vanilla option button links
#define APP_GITHUB			"https://github.com/ZarroTsu/TheLastGate"
#define APP_DISCORD			"https://discord.gg/jJbPv2R"

// Application launch values - used by SDL to draw the screen.
#define APP_X				SDL_WINDOWPOS_CENTERED
#define APP_Y				SDL_WINDOWPOS_CENTERED
#define APP_W				1280
#define APP_H				720

SDL_Window* m_window;
SDL_Renderer* m_renderer;
SDL_Rect m_fullScreen = {0, 0, APP_W, APP_H};
SDL_Point m_mouse_position;

char isRunning;
int FPS;

#endif // MAIN_H
/* END OF FILE */
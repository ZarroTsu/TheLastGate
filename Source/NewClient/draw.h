// **************************************************************** 
//
//  The Last Gate - SDL2 Client by Zachary "ZarroTsu" Baillie
//  Based on the Mercenaries of Astonia engine by Daniel Brockhaus
//  
//  draw.h
//  Header information specific to draw.c
//
// **************************************************************** 
#ifndef DRAW_H
#define DRAW_H

#define FONT_SPRITE		18100
#define FONT_W			6
#define FONT_H			10
#define FONT_OFFSET		32
#define FONT_LIMIT		127

#define FN_RED			0
#define FN_YEL			1
#define FN_GRE			2
#define FN_BLU			3

SDL_Surface* d_surface;

struct Sprite {
	SDL_Texture* texture;
	char loaded;
} d_spr[MAXSPRITE];

extern SDL_Renderer* m_renderer;
extern SCE_Scene sceneCurrent;

#endif // DRAW_H
/* END OF FILE */
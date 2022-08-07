// **************************************************************** 
//
//  The Last Gate - SDL2 Client by Zachary "ZarroTsu" Baillie
//  Based on the Mercenaries of Astonia engine by Daniel Brockhaus
//  
//  draw.c
//  Handles draw-related functions
//
// **************************************************************** 
#include "common.h"
#include "draw.h"

SDL_Rect DRA_NewRectangle(int x, int y, int w, int h)
{
	SDL_Rect tmpRect;
	
	tmpRect.x = x;
	tmpRect.y = y;
	tmpRect.w = w;
	tmpRect.h = h;
	
	return tmpRect;
}

SDL_Point DRA_NewPoint(int x, int y)
{
	SDL_Point tmpPoint;
	
	tmpPoint.x = x;
	tmpPoint.y = y;
	
	return tmpPoint;
}

SDL_Rect DRA_GetSpriteSize(SDL_Texture *tx)
{
    SDL_Rect size;
    SDL_QueryTexture(tx, NULL, NULL, &size.w, &size.h);
    return size;
}

// Draw a rectangle at the given SDL_Rect, using the given color code in HEX
void DRA_Rectangle(SDL_Rect rect, unsigned int c)
{
	Uint8 r, g, b, a;
	
	r = (c >>24) & 0xff;
	g = (c >>16) & 0xff;
	b = (c >> 8) & 0xff;
	a =  c       & 0xff;
	
	SDL_SetRenderDrawColor(m_renderer, r, g, b, a);
	SDL_RenderFillRect(m_renderer, &rect);
}

// Load a sprite using a given number via temporary surface (d_surface)
void DRA_LoadSprite(int nr)
{
	char buf[80];
	
	if (d_spr[nr].loaded) return;
	
	sprintf(buf, "gfx/%05d.png", nr);
	d_surface = IMG_Load(buf);
	
	if (!d_surface)
	{
		sprintf(buf, "gfx/%05d.bmp", nr);
		d_surface = IMG_Load(buf);
	}
	
	if (!d_surface)
	{
		printf("[ERR] D_LoadSprite: Cannot find SPR %05d\n", nr);
		return;
	}
	
	SDL_SetColorKey(d_surface, SDL_TRUE, SDL_MapRGB(d_surface->format, 0xFF, 0x00, 0xFF));
	
	d_spr[nr].texture = SDL_CreateTextureFromSurface(m_renderer, d_surface);
	d_spr[nr].loaded = 1;
	
	printf("[INF] D_LoadSprite: Loaded SPR %05d\n", nr);
	
	SDL_FreeSurface(d_surface);
}

// Draws a sprite by number with optional cropping to a target point on the screen
void DRA_SpriteCropped(int sprite, SDL_Rect crop, int x, int y)
{
	SDL_Rect dest;
	
	// Loads the sprite, passing the parent rectangle to crop the image
	DRA_LoadSprite(sprite);
	
	// X and Y target for the sprite render
	dest.x = x;
	dest.y = y;
	
	// Render single character
	if (crop.h && crop.w)
	{
		// W and H target for cropping
		dest.w = crop.w;
		dest.h = crop.h;
		
		SDL_RenderCopy(m_renderer, d_spr[sprite].texture, &crop, &dest);
	}
	else
	{
		// W and H target for cropping (set to same as texture with no crop target)
		dest.w = DRA_GetSpriteSize(d_spr[sprite].texture).w;
		dest.h = DRA_GetSpriteSize(d_spr[sprite].texture).h;
		
		SDL_RenderCopy(m_renderer, d_spr[sprite].texture, NULL, &dest);
	}
}


// Draws a sprite by number without cropping - this passes to D_DrawSpriteCrop() with a NULL rectangle
void DRA_Sprite(int sprite, int x, int y)
{
	SDL_Rect crect = {0, 0, 0, 0};
	DRA_SpriteCropped(sprite, crect, x, y);
}

// Draws a single character to screen with a given X and Y by loading a cropped image from a font sprite
void DRA_Char(int x, int y, int color, int c)
{
	int sprite;
	SDL_Rect crect;
	
	// Keep things sane
	if (c < 0 || c > FONT_LIMIT) 	return;
	if (color < 0 || color > 9) 	return;
	
	// Set sprite using base value plus font color
	sprite = FONT_SPRITE + color;
	
	// Removes unused ascii characters
	c -= FONT_OFFSET;
	
	// Sets up a local rectangle using the ascii char value and offsetting by static values
	crect = DRA_NewRectangle(c * FONT_W, 0, FONT_W, FONT_H);
	
	// Load the font sprite using the local rect to crop, and the local point to place it
	DRA_SpriteCropped(sprite, crect, x-1, y-1);
}

// Draws a string of text using repeated D_DrawChar() calls
void DRA_TextLoop(int x, int y, int color, char *text)
{
	// Loop through each character in *text
	while (*text)
	{
		DRA_Char(x, y, color, *text);		// Draw the current character
		text++;								// Advance to next character
		x += FONT_W;						// Increase target draw X by font width
	}
}

// Draws a string of text using a va_list to pass printf %% arguments along
void DRA_Text(int x, int y, int color, char *text, ...)
{
	va_list args;
	char buf[1024];
	
	va_start(args, text);
	vsprintf(buf, text, args);
	DRA_TextLoop(x, y, color, buf);
	va_end(args);
}

void DRA_Init(void)
{
	int n;
	
	for (n = 0; n < MAXSPRITE; n++)
	{
		d_spr[n].loaded = 0;
	}
}

void DRA_Clean(void)
{
	int n;
	
	for (n = 0; n < MAXSPRITE; n++)
	{
		d_spr[n].loaded = 0;
		SDL_DestroyTexture(d_spr[n].texture);
	}
}

/* END OF FILE */
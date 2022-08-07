// **************************************************************** 
//
//  The Last Gate - SDL2 Client by Zachary "ZarroTsu" Baillie
//  Based on the Mercenaries of Astonia engine by Daniel Brockhaus
//  
//  funcs.h
//  Function list for public sharing between c files
//
// **************************************************************** 
#ifndef FUNCS_H
#define FUNCS_H

// main.c


// characterscreen.c
void CHA_Init(void);
void CHA_Update(void);
void CHA_Render(void);
void CHA_Clean(void);

// draw.c
SDL_Rect DRA_NewRectangle(int x, int y, int w, int h);
SDL_Point DRA_NewPoint(int x, int y);
void DRA_Rectangle(SDL_Rect rect, unsigned int c);
void DRA_SpriteCropped(int sprite, SDL_Rect crop, int x, int y);
void DRA_Sprite(int sprite, int x, int y);
void DRA_Text(int x, int y, int color, char *text, ...);
void DRA_Init(void);
void DRA_Clean(void);

// filemanager.c
void FIL_Load_Options(void);
void FIL_Save_Options(void);

// gamescreen.c
void GAM_Log(char font, char *format, ...);
void GAM_MOTD(char font, char *format, ...);
void GAM_Init(void);
void GAM_Update(void);
void GAM_Render(void);
void GAM_Clean(void);

// inputmanager.c
void INP_Management(void);

// network.c

// openscreen.c
void OPE_Init(void);
void OPE_Update(void);
void OPE_Render(void);
void OPE_Clean(void);

// scenemanager.c
void SCE_Change(int target);
void SCE_RenderTransition(void);
void SCE_Update(void);
void SCE_Render(void);

// titlescreen.c
void TIT_Init(void);
void TIT_Update(void);
void TIT_Render(void);
void TIT_Clean(void);

#endif // FUNCS_H
/* END OF FILE */
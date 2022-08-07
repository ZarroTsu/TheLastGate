// ****************************************************************
//
//  The Last Gate - SDL2 Client by Zachary "ZarroTsu" Baillie
//  Based on the Mercenaries of Astonia engine by Daniel Brockhaus
//  
//  inputmanager.h
//  Header information specific to inputmanager.c
//
// ****************************************************************
#ifndef INPUTMANAGER_H
#define INPUTMANAGER_H

SDL_Event input;

struct INP_State
{
	char shift, ctrl, alt;
	SDL_Keycode keyPress;
	char leftClick, rightClick;
	Sint32 mouseX, mouseY;
} InputState;

extern char isRunning;
extern int sceneCurrent;

#endif // INPUTMANAGER_H
/* END OF FILE */
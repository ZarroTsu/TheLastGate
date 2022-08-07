// ****************************************************************
//
//  The Last Gate - SDL2 Client by Zachary "ZarroTsu" Baillie
//  Based on the Mercenaries of Astonia engine by Daniel Brockhaus
//  
//  inputmanager.c
//  Handles player input
//
// ****************************************************************
#include "common.h"
#include "inputmanager.h"

void INP_InitKeyboard(void)
{
	InputState.shift = 0;
	InputState.ctrl = 0;
	InputState.alt = 0;
	InputState.keyPress = NULL;
}

void INP_InitMouse(void)
{
	InputState.leftClick = 0;
	InputState.rightClick = 0;
	InputState.mouseX = 0;
	InputState.mouseY = 0;
}

char INP_MouseCoords(int x1, int y1, int x2, int y2)
{
	if (InputState.mouseX > x1 && InputState.mouseY > y1 && 
		InputState.mouseX < x2 && InputState.mouseY < y2)
	{
		return 1;
	}
	
	return 0;
}

void INP_GlobalKeyboard(void)
{
	INP_InitKeyboard();
	
	if (input.key.keysym.mod & KMOD_SHIFT) InputState.shift = 1;
	if (input.key.keysym.mod & KMOD_CTRL) InputState.ctrl = 1;
	if (input.key.keysym.mod & KMOD_ALT) InputState.alt = 1;
	
	InputState.keyPress = input.key.keysym.sym;
	
	switch (InputState.keyPress)
	{
		case SDLK_F1: SCE_Change(SCENE_OPENER); break;
		case SDLK_F2: SCE_Change(SCENE_TITLE); break;
		case SDLK_F3: SCE_Change(SCENE_CHARACTER); break;
		case SDLK_F4: SCE_Change(SCENE_GAMELOOP); break;
		
		default: break;
	}
}

void INP_GlobalMouse(void)
{
	INP_InitMouse();
	
	if (input.button.button == SDL_BUTTON_LEFT)
		InputState.leftClick = 1;
	else if (input.button.button == SDL_BUTTON_RIGHT)
		InputState.rightClick = 1;
	
	InputState.mouseX = input.button.x;
	InputState.mouseY = input.button.y;
}

void INP_Opener(void)
{
	
}

void INP_Title(void)
{
	
}

void INP_CharSelect(void)
{
	
}

void INP_Game(void)
{
	if (InputState.leftClick)
	{
		GAM_Log(1, "L. Click: %4d, %4d\n", InputState.mouseX, InputState.mouseY);
		InputState.leftClick = 0;
	}
	if (InputState.rightClick)
	{
		GAM_Log(1, "R. Click: %4d, %4d\n", InputState.mouseX, InputState.mouseY);
		InputState.rightClick = 0;
	}
}

void INP_Management(void)
{
	while (SDL_PollEvent(&input))
	{
		switch(input.type)
		{
			// SDL_QUIT is called when the user clicks the X on the client window
			case SDL_QUIT:
				isRunning = 0;
				break;
				
			case SDL_KEYDOWN:
				INP_GlobalKeyboard();
				switch (sceneCurrent)
				{
					case SCENE_GAMELOOP: 	INP_Game(); break;
					case SCENE_CHARACTER: 	INP_CharSelect(); break;
					case SCENE_TITLE: 		INP_Title(); break;
					default: 				INP_Opener(); break;
				}
				break;
				
			case SDL_MOUSEBUTTONUP:
				INP_GlobalMouse();
				switch (sceneCurrent)
				{
					case SCENE_GAMELOOP: 	INP_Game(); break;
					case SCENE_CHARACTER: 	INP_CharSelect(); break;
					case SCENE_TITLE: 		INP_Title(); break;
					default: 				INP_Opener(); break;
				}
				break;
				
			default: break;
		}
	}
}

/* END OF FILE */
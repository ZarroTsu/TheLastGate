// ****************************************************************
//
//  The Last Gate - SDL2 Client by Zachary "ZarroTsu" Baillie
//  Based on the Mercenaries of Astonia engine by Daniel Brockhaus
//  
//  scenemanager.c
//  Handles scene numbers and scene transitions
//
// ****************************************************************
#include "common.h"
#include "scenemanager.h"

void SCE_Change(int target)
{
	// No need to do anything if the scene matches the target
	if (sceneCurrent == target) return;
	
	sceneTransition = 1;
	sceneTarget = target;
}

void SCE_FadeOut(void)
{
	sceneTicker += SCENE_FADERATE;
	
	if (sceneTicker >= 255)
	{
		sceneTransition = 2;
		if (sceneCurrent != sceneTarget)
		{
			sceneCurrent = sceneTarget;
			SCE_Init();
		}
	}
}

void SCE_FadeIn(void)
{
	sceneTicker -= SCENE_FADERATE;
	
	if (sceneTicker <= 0)
	{
		sceneTransition = 0;
	}
}

void SCE_RenderTransition(void)
{
	if (sceneTicker)
	{
		SDL_SetRenderDrawColor(m_renderer, 0, 0, 0, min(sceneTicker, 255));
		SDL_RenderFillRect(m_renderer, &m_fullScreen);
	}
}

void SCE_Init(void)
{
	DRA_Clean();
	SCE_Clean();
	
	switch (sceneCurrent)
	{
		case SCENE_GAMELOOP: 	GAM_Init(); break;
		case SCENE_CHARACTER: 	CHA_Init(); break;
		case SCENE_TITLE: 		TIT_Init(); break;
		default: 				OPE_Init(); break;
	}
}

void SCE_Update(void)
{
	if (sceneTransition)
	{
		switch (sceneTransition)
		{
			case  1: SCE_FadeOut(); break;
			default: SCE_FadeIn();  break;
		}
		return;
	}
	
	switch (sceneCurrent)
	{
		case SCENE_GAMELOOP: 	GAM_Update(); break;
		case SCENE_CHARACTER: 	CHA_Update(); break;
		case SCENE_TITLE: 		TIT_Update(); break;
		default: 				OPE_Update(); break;
	}
}

void SCE_Render(void)
{
	switch (sceneCurrent)
	{
		case SCENE_GAMELOOP: 	GAM_Render(); break;
		case SCENE_CHARACTER: 	CHA_Render(); break;
		case SCENE_TITLE: 		TIT_Render(); break;
		default: 				OPE_Render(); break;
	}
}

void SCE_Clean(void)
{
	OPE_Clean();
	TIT_Clean();
	CHA_Clean();
	GAM_Clean();
}

/* END OF FILE */
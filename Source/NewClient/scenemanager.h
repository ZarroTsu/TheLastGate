// ****************************************************************
//
//  The Last Gate - SDL2 Client by Zachary "ZarroTsu" Baillie
//  Based on the Mercenaries of Astonia engine by Daniel Brockhaus
//  
//  scenemanager.h
//  Header information specific to scenemanager.c
//
// ****************************************************************
#ifndef SCENEMANAGER_H
#define SCENEMANAGER_H

#define SCENE_FADERATE		(255 / 15)

SCE_Scene sceneCurrent = SCENE_TITLE;

int sceneTarget = -1;
int sceneTransition = 0;
int sceneTicker = 0;

extern SDL_Renderer* m_renderer;
extern SDL_Rect m_fullScreen;

#endif // SCENEMANAGER_H
/* END OF FILE */
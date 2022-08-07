// **************************************************************** 
//
//  The Last Gate - SDL2 Client by Zachary "ZarroTsu" Baillie
//  Based on the Mercenaries of Astonia engine by Daniel Brockhaus
//  
//  main.c
//  Launch point and core gameplay loop
//
// **************************************************************** 
#include "common.h"
#include "main.h"

void MAI_Init_Window(void)
{
	int sdlFlags = 0;
	
	if (options.fullscreen) sdlFlags = SDL_WINDOW_FULLSCREEN;
	
	if (SDL_Init(SDL_INIT_EVERYTHING) == 0)
	{
		TTF_Init();
		m_window = SDL_CreateWindow(APP_NAME, APP_X, APP_Y, APP_W, APP_H, sdlFlags);
		m_renderer = SDL_CreateRenderer(m_window, -1, 0);
		SDL_SetRenderDrawBlendMode(m_renderer, SDL_BLENDMODE_BLEND);
		isRunning = 1;
	}
	else
	{
		isRunning = 0;
	}
}

void MAI_Init(void)
{
	FIL_Load_Options();
	MAI_Init_Window();
	DRA_Init();
}

void MAI_Update(void)
{
	SCE_Update();
}

void MAI_Render(void)
{
	SDL_RenderClear(m_renderer);				// Clear the renderer
	DRA_Rectangle(m_fullScreen, 0x000000FF);	// Solid background color
	//
	
	SCE_Render();
	
	DRA_Text(APP_W-(6*7+2), APP_H-(1*10+2), FPS<TICKS?0:1, "%3d FPS", FPS);
	
	//
	SCE_RenderTransition();						// Transition scenes (if applicable)
	SDL_RenderPresent(m_renderer);				// Present the renderer
}

void MAI_Clean(void)
{
	FIL_Save_Options();
	SCE_Clean();
	SDL_RenderClear(m_renderer);
	SDL_DestroyRenderer(m_renderer);
	SDL_DestroyWindow(m_window);
	SDL_Quit();
	TTF_Quit();
}

int main(int argc, char* args[])
{
	Uint64 frameStart;
	int frameTime;

	MAI_Init();
	
	while (isRunning)
	{
		frameStart = SDL_GetTicks64();
		
		INP_Management();
		MAI_Update();
		MAI_Render();
		
		frameTime = SDL_GetTicks64() - frameStart;
		
		if (frameTime > 0) FPS = min(TICKS, 1000 / frameTime);
		
		if (TICK > frameTime)
		{
			SDL_Delay(TICK - frameTime);
		}
	}
	
	MAI_Clean();
	
    return 0;
}

/* END OF FILE */
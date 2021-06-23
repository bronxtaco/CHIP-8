#include <SDL.h>
#include <stdio.h>

#include "c8e_constants.h"
#include "c8e_SDL.h"

// Constants
#define PIXEL_SIZE (20)
#define SCREEN_WIDTH (PIXEL_SIZE * WIDTH_PIXELS)
#define SCREEN_HEIGHT (PIXEL_SIZE * HEIGHT_PIXELS)
#define BACK_COLOUR 0x00, 0x00, 0x00
#define FORE_COLOUR 0xFF, 0xFF, 0xFF

c8e_SDL::c8e_SDL(const char* title)
{
	//The window we'll be rendering to
	SDL_Window* window = NULL;

	//The surface contained by the window
	SDL_Surface* screenSurface = NULL;

	//Initialize SDL
	if (SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
	}
	else
	{
		//Create window
		m_window = SDL_CreateWindow(title, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
		if (m_window == NULL)
		{
			printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
		}
		else
		{
			//Create renderer for window
			m_renderer = SDL_CreateRenderer(m_window, -1, SDL_RENDERER_ACCELERATED);
			if (m_renderer == NULL)
			{
				printf("Renderer could not be created! SDL Error: %s\n", SDL_GetError());
			}
			else
			{
				//Initialize renderer color
				SDL_SetRenderDrawColor(m_renderer, 0xFF, 0xFF, 0xFF, 0xFF);
			}

			//Get window surface
			screenSurface = SDL_GetWindowSurface(m_window);

			//Fill the surface white
			SDL_FillRect(screenSurface, NULL, SDL_MapRGB(screenSurface->format, BACK_COLOUR));

			//Update the surface
			SDL_UpdateWindowSurface(m_window);

			//Wait two seconds
			SDL_Delay(2000);
		}
	}
}

c8e_SDL::~c8e_SDL()
{
	//Destroy window
	SDL_DestroyWindow(m_window);

	//Quit SDL subsystems
	SDL_Quit();
}

void c8e_SDL::Render(bool* renderData)
{
	int xPos = 0;
	int yPos = 0;
	int count = 0;
	while (count < (WIDTH_PIXELS * HEIGHT_PIXELS))
	{
		if (renderData[count])
		{
			// Render rectangle
			SDL_Rect fillRect = { xPos * PIXEL_SIZE, yPos * PIXEL_SIZE, PIXEL_SIZE, PIXEL_SIZE };
			SDL_SetRenderDrawColor(m_renderer, FORE_COLOUR, 0xFF);
			SDL_RenderFillRect(m_renderer, &fillRect);
		}
		count++;
		if (count % WIDTH_PIXELS == 0)
		{
			xPos = 0;
			yPos++;
		}
		else
		{
			xPos++;
		}
	}

	//Update screen
	SDL_RenderPresent(m_renderer);
}

double c8e_SDL::GetDeltaTime()
{
	Uint64 currentTime = SDL_GetPerformanceCounter();
	double dt = ((currentTime - m_prevDelta) * 1000000 / (double)SDL_GetPerformanceFrequency());
	m_prevDelta = currentTime;
	return dt;
	
}
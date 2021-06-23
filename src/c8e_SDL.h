#pragma once

#include <SDL.h>

struct c8e_SDL
{
public:
	c8e_SDL(const char* title);
	~c8e_SDL();

	void Render(bool* renderData);
	double GetDeltaTime();

private:
	SDL_Window* m_window = NULL;
	SDL_Renderer* m_renderer = NULL;

	Uint64 m_prevDelta = SDL_GetPerformanceCounter();
};
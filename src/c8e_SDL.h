#pragma once

#include <SDL.h>

struct c8e_SDL
{
public:
	c8e_SDL(const char* title);
	~c8e_SDL();

	void Render(bool* renderData);
	double GetDeltaTime();
	bool* GetKeys();
	bool QuitEmulator() { return m_escape; }

	void PlaySound();
	void StopSound();

private:
	SDL_Window* m_window = NULL;
	SDL_Renderer* m_renderer = NULL;

	Uint64 m_prevDelta = SDL_GetPerformanceCounter();

	bool* m_keys;
	bool m_escape;
};
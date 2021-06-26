#include <SDL.h>
#include <SDL_audio.h>
#include <stdio.h>

#include "c8e_constants.h"
#include "c8e_SDL.h"

// Constants
#define PIXEL_SIZE (20)
#define SCREEN_WIDTH (PIXEL_SIZE * WIDTH_PIXELS)
#define SCREEN_HEIGHT (PIXEL_SIZE * HEIGHT_PIXELS)
#define BACK_COLOUR 0x00, 0x00, 0x00
#define FORE_COLOUR 0xff, 0xff, 0xff

#define AMPLITUDE (28000)
#define SAMPLE_RATE (44100)

c8e_SDL::c8e_SDL(const char* title)
{
	//The window we'll be rendering to
	SDL_Window* window = NULL;

	//The surface contained by the window
	SDL_Surface* screenSurface = NULL;

	//Initialize SDL
	if (SDL_Init(SDL_INIT_VIDEO|SDL_INIT_AUDIO) < 0)
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

			//Fill the surface with backcolor
			SDL_FillRect(screenSurface, NULL, SDL_MapRGB(screenSurface->format, BACK_COLOUR));

			//Update the surface
			SDL_UpdateWindowSurface(m_window);
		}

		// Initialize audio
		extern void audio_callback(void *user_data, Uint8 *raw_buffer, int bytes);

		int sample_nr = 0;

		SDL_AudioSpec want;
		want.freq = SAMPLE_RATE; // number of samples per second
		want.format = AUDIO_S16SYS; // sample type (here: signed short i.e. 16 bit)
		want.channels = 1; // only one channel
		want.samples = 2048; // buffer-size
		want.callback = audio_callback; // function SDL calls periodically to refill the buffer
		want.userdata = &sample_nr; // counter, keeping track of current sample number

		SDL_AudioSpec have;
		if (SDL_OpenAudio(&want, &have) != 0) SDL_LogError(SDL_LOG_CATEGORY_AUDIO, "Failed to open audio: %s", SDL_GetError());
		if (want.format != have.format) SDL_LogError(SDL_LOG_CATEGORY_AUDIO, "Failed to get the desired AudioSpec");
	}

	// Keyboard input state
	m_keys = (bool*)calloc(NUM_KEYS, sizeof(bool));
}

c8e_SDL::~c8e_SDL()
{
	free(m_keys);

	// Destroy window
	SDL_DestroyWindow(m_window);

	// Destroy sound
	SDL_CloseAudio();

	// Quit SDL subsystems
	SDL_Quit();
}

void c8e_SDL::Render(bool* renderData)
{
	int xPos = 0;
	int yPos = 0;
	int count = 0;
	while (count < (WIDTH_PIXELS * HEIGHT_PIXELS))
	{
		// Make a rectangle (pixel)
		SDL_Rect fillRect = { xPos * PIXEL_SIZE, yPos * PIXEL_SIZE, PIXEL_SIZE, PIXEL_SIZE };

		// Set pixel color
		if (renderData[count])
		{
			SDL_SetRenderDrawColor(m_renderer, FORE_COLOUR, 0xff);
		}
		else
		{
			SDL_SetRenderDrawColor(m_renderer, BACK_COLOUR, 0xff);
		}
		SDL_RenderFillRect(m_renderer, &fillRect);
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

bool* c8e_SDL::GetKeys()
{
	SDL_PumpEvents();
	const Uint8* keys = SDL_GetKeyboardState(NULL);

	m_keys[0x00] = keys[SDL_SCANCODE_X];
	m_keys[0x01] = keys[SDL_SCANCODE_1];
	m_keys[0x02] = keys[SDL_SCANCODE_2];
	m_keys[0x03] = keys[SDL_SCANCODE_3];
	m_keys[0x04] = keys[SDL_SCANCODE_Q];
	m_keys[0x05] = keys[SDL_SCANCODE_W];
	m_keys[0x06] = keys[SDL_SCANCODE_E];
	m_keys[0x07] = keys[SDL_SCANCODE_A];
	m_keys[0x08] = keys[SDL_SCANCODE_S];
	m_keys[0x09] = keys[SDL_SCANCODE_D];
	m_keys[0x0a] = keys[SDL_SCANCODE_Z];
	m_keys[0x0b] = keys[SDL_SCANCODE_C];
	m_keys[0x0c] = keys[SDL_SCANCODE_4];
	m_keys[0x0d] = keys[SDL_SCANCODE_R];
	m_keys[0x0e] = keys[SDL_SCANCODE_F];
	m_keys[0x0f] = keys[SDL_SCANCODE_V];

	m_escape = keys[SDL_SCANCODE_ESCAPE];

	return m_keys;
}

void c8e_SDL::PlaySound()
{
	SDL_PauseAudio(0);
}

void c8e_SDL::StopSound()
{
	SDL_PauseAudio(1);
}

void audio_callback(void *user_data, Uint8 *raw_buffer, int bytes)
{
	Sint16 *buffer = (Sint16*)raw_buffer;
	int length = bytes / 2; // 2 bytes per sample for AUDIO_S16SYS
	int &sample_nr(*(int*)user_data);

	for (int i = 0; i < length; i++, sample_nr++)
	{
		double time = (double)sample_nr / (double)SAMPLE_RATE;
		buffer[i] = (Sint16)(AMPLITUDE * sin(2.0f * M_PI * 441.0f * time)); // render 441 HZ sine wave
	}
}
#include <stdio.h>

#include "c8e_CPU.h"
#include "c8e_SDL.h"

// Constants
#define PROGRAM_TITLE "CHIP-8 Emulator"

int main(int argc, char* args[])
{
	// initialize
	c8e_SDL* sdl = new c8e_SDL(PROGRAM_TITLE);
	c8e_CPU* chip8 = new c8e_CPU();

	// run loop cycle
	for (;;)
	{
		chip8->UpdateInput(sdl->GetKeys());
		
		if (sdl->QuitEmulator())
		{
			break;
		}

		if (chip8->AdvanceTime())
		{
			sdl->Render(chip8->GetRenderData());
		}

		if (chip8->GetSoundActive())
		{
			// play sound
		}
	}

	// cleanup
	delete(sdl);
	delete(chip8);

	return 0;
}
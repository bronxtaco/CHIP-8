#include <stdio.h>

#include "c8e_CPU.h"
#include "c8e_SDL.h"

// Constants
#define PROGRAM_TITLE "CHIP-8 Emulator"

int main(int argc, char* args[])
{
	// initialize SDL
	c8e_SDL* sdl = new c8e_SDL(PROGRAM_TITLE);

	// initialize CPU
	c8e_CPU* cpu = new c8e_CPU();

	// run loop cycle
	for (;;)
	{
		if (cpu->ExecuteInstructionCycle())
		{
			sdl->Render(cpu->GetRenderData());
		}
	}

	// cleanup
	delete(sdl);
	delete(cpu);

	return 0;
}
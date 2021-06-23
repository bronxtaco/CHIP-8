#include <bitset>
#include <fstream>
#include <stdlib.h>
#include <string.h>
#include <vector>

#include "c8e_constants.h"
#include "c8e_CPU.h"

#define RAM_SIZE (4096)
#define PROGRAM_OFFSET (512)
#define STACK_SIZE (16)
#define FONT_OFFSET (80)


c8e_CPU::c8e_CPU()
{
	m_ram = (u8*)calloc(RAM_SIZE, sizeof(u8));
	m_pc = (u16*)(m_ram + PROGRAM_OFFSET);

	m_stack = (u16*)calloc(STACK_SIZE, sizeof(u16));
	m_stackPtr = m_stack;

	InitFont();

	m_renderData = (bool*) malloc((64 * 32) * sizeof(bool));
	for (int i = 0; i < (64 * 32); i++)
	{
		m_renderData[i] = i % 2 == 0;
	}

	LoadRom();
}

void c8e_CPU::InitFont()
{
	const unsigned char fontData[] = {
		0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
		0x20, 0x60, 0x20, 0x20, 0x70, // 1
		0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
		0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
		0x90, 0x90, 0xF0, 0x10, 0x10, // 4
		0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
		0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
		0xF0, 0x10, 0x20, 0x40, 0x40, // 7
		0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
		0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
		0xF0, 0x90, 0xF0, 0x90, 0x90, // A
		0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
		0xF0, 0x80, 0x80, 0x80, 0xF0, // C
		0xE0, 0x90, 0x90, 0x90, 0xE0, // D
		0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
		0xF0, 0x80, 0xF0, 0x80, 0x80  // F
	};
	for (int i = 0; i < (16 * 5); i++)
	{
		m_ram[FONT_OFFSET + i] = fontData[i];
	}
}

void c8e_CPU::LoadRom()
{
	std::ifstream file("IBMLogo.ch8", std::ios::binary | std::ios::ate);
	file.seekg(0, std::ios::end);
	std::streamsize size = file.tellg();
	file.seekg(0, std::ios::beg);

	file.read(m_ram + PROGRAM_OFFSET, size);
}

c8e_CPU::~c8e_CPU()
{
	free(m_ram);
	free(m_renderData);
}

bool c8e_CPU::ExecuteInstructionCycle()
{
	std::chrono::time_point<std::chrono::system_clock> now = std::chrono::system_clock::now();
	double dt = (now - m_prevDelta) / std::chrono::microseconds(1);
	m_prevDelta = now;

	double clockTick = 1000000 / m_clockspeed;
	double timerTick = 1000000 / m_timerspeed;

	m_clockCount += dt;
	m_delayCount += dt;
	m_soundCount += dt;

	if (m_clockCount >= clockTick)
	{
		// execute instruction cycle
		m_clockCount = fmod(m_clockCount, clockTick);

		u16 opcode = Fetch();
		Decode(opcode);
	}

	if (m_delayCount >= timerTick)
	{
		// update timers
		m_delayCount = fmod(m_delayCount, timerTick);
		//m_soundCount = fmod(m_soundCount, timerTick);
		return true;
	}
	else
	{
		return false;
	}
}

u16 c8e_CPU::Fetch()
{
	// get instruction at program counter
	u16 val = *m_pc;

	// advance program counter
	m_pc += 1;

	// flip the bits (endian?? ask matt)
	u16 temp = val >> 8;
	val = (val << 8) | temp;

	// return instruction
	return val;
}

void c8e_CPU::Decode(u16 opcode)
{
	u8 nibble1 = 0xf000 & opcode;
	u8 nibble2 = 0x0f00 & opcode;
	u8 nibble3 = 0x00f0 & opcode;
	u8 nibble4 = 0x000f & opcode;
	switch (nibble1)
	{
	case 0x00:
		if (nibble3 == (u8)0xe0)
		{
			if (nibble4 == (u8)0x00)
			{
				ClearScreen();
			}
			else if (nibble4 == (u8)0xe0)
			{
				// subroutine return
			}
		}
		break;
	case 0x10:
		// jump
		//u16 jump = nibble4 | (nibble3 << 4) | (nibble2 << 8);
		m_pc = (u16*)((m_ram + PROGRAM_OFFSET) + (nibble4 | (nibble3 << 4) | (nibble2 << 8)));
		break;
	case 0x20:
		
		break;
	default:
		break;
	}
}

void c8e_CPU::ClearScreen()
{
	for (int i = 0; i < (64 * 32); i++)
	{
		m_renderData[i] = 0;
	}
}
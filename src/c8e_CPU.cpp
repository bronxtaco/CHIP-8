#include <bitset>
#include <cassert>
#include <fstream>
#include <stdlib.h>
#include <string.h>
#include <vector>

#include "c8e_constants.h"
#include "c8e_CPU.h"

#define RAM_SIZE (4096)
#define PROGRAM_OFFSET (512)
#define STACK_SIZE (16)
#define NUM_REGISTERS (16)
#define FONT_OFFSET (80)

// nibbles
#define _0 (u8)0x00
#define _1 (u8)0x10
#define _2 (u8)0x20
#define _3 (u8)0x30
#define _4 (u8)0x40
#define _5 (u8)0x50
#define _6 (u8)0x60
#define _7 (u8)0x70
#define _8 (u8)0x80
#define _9 (u8)0x90
#define _A (u8)0xa0
#define _B (u8)0xb0
#define _C (u8)0xc0
#define _D (u8)0xd0
#define _E (u8)0xe0
#define _F (u8)0xf0

// opcode decoders
#define _X ((u8)((n2 >> 4) & 0x0f))
#define _Y ((u8)((n3 >> 4) & 0x0f))
#define _N ((u8)((n4 >> 4) & 0x0f))
#define _NN ((u8)((n4 >> 4) & 0x0f) | n3)
#define _NNN ((u8)((n4 >> 4) & 0x0f) | n3 | (n2 << 4))

// VF
#define _VF (m_V[0x0f])

c8e_CPU::c8e_CPU()
{
	m_ram = (u8*)calloc(RAM_SIZE, sizeof(u8));
	m_pc = (u16*)(m_ram + PROGRAM_OFFSET);

	m_stack = (u16*)calloc(STACK_SIZE, sizeof(u16));
	m_stackIdx = 0;

	m_V = (u8*)calloc(NUM_REGISTERS, sizeof(u16));

	InitFont();

	m_renderData = (bool*) malloc((64 * 32) * sizeof(bool));
	ClearScreen();

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
	u8 n1 = (0xf000 & opcode) >> 8;
	u8 n2 = (0x0f00 & opcode) >> 4;
	u8 n3 = 0x00f0 & opcode;
	u8 n4 = (0x000f & opcode) << 4;
	switch (n1)
	{
		case _0:
		{
			if (n3 == _E)
			{
				if (n4 == _0) // Clear Screen
				{
					ClearScreen();
				}
				else if (n4 == _E) // Subroutine Return
				{

				}
				else
				{
					assert(true); // unhandled instruction!
				}
			}
			break;
		}
		case _1: // Jump
		{
			m_pc = (u16*)(m_ram + _NNN);
			break;
		}
		case _2: // Execute Subroutine
		{
			m_stack[m_stackIdx] = *m_pc;
			m_stackIdx += 1;
			m_pc = (u16*)(m_ram + _NNN);
			break;
		}
		case _6: // Set
		{
			m_V[_X] = _NN;
			break;
		}
		case _7: // Add
		{
			m_V[_X] += _NN;
			break;
		}
		case _A: // Set index
		{
			m_I = (u16*)(m_ram + _NNN);
			break;
		}
		case _D: // Display
		{
			int _x = m_V[_X] % WIDTH_PIXELS;
			int _y = m_V[_Y];
			u8* _i = (u8*)m_I;
			bool setFlag = false;

			for (int y = 0; y < _N; y++)
			{
				u8 drawMask = 0x80;
				for (int x = 0; x < 8; x++)
				{
					if (*_i & drawMask)
					{
						int renderPos = (_x + x) + ((_y + y) * WIDTH_PIXELS);
						m_renderData[renderPos] = !m_renderData[renderPos];
						if (!m_renderData[renderPos])
						{
							setFlag = true;
						}
					}
					drawMask = (drawMask >> 1);
					if (x == 0)
					{
						drawMask = drawMask ^ 0x80;
					}
				}
				_i++;
			}
			_VF = setFlag;
			break;
		}
		default:
		{
			assert(true); // unhandled instruction!
			break;
		}
	}
}

void c8e_CPU::ClearScreen()
{
	for (int i = 0; i < (64 * 32); i++)
	{
		m_renderData[i] = 0;
	}
}
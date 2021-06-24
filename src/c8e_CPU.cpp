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
#define _0 0x00
#define _1 0x01
#define _2 0x02
#define _3 0x03
#define _4 0x04
#define _5 0x05
#define _6 0x06
#define _7 0x07
#define _8 0x08
#define _9 0x09
#define _A 0x0a
#define _B 0x0b
#define _C 0x0c
#define _D 0x0d
#define _E 0x0e
#define _F 0x0f

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

	file.read((char*)m_ram + PROGRAM_OFFSET, size);
}

c8e_CPU::~c8e_CPU()
{
	free(m_ram);
	free(m_stack);
	free(m_V);
	free(m_renderData);
}

bool c8e_CPU::ExecuteInstructionCycle()
{
	std::chrono::time_point<std::chrono::system_clock> now = std::chrono::system_clock::now();
	double dt = (double)((now - m_prevDelta) / std::chrono::microseconds(1));
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

	// return instruction
	return val;
}

#define _INSTRUCTION(val) ((val >> 4) & 0x0f)
#define _X(val) ((val >> 0) & 0x0f)
#define _Y(val) ((val >> 12)  & 0x0f)
#define _N(val) ((val >> 8) & 0x0f)
#define _NN(val) ((_Y(val) << 4) | _N(val))
#define _NNN(val) ((_X(val) << 8) | (_Y(val) << 4) | _N(val))

void c8e_CPU::Decode(u16 opcode)
{
	switch (_INSTRUCTION(opcode))
	{
		case _0:
		{
			if (_Y(opcode) == _E)
			{
				if (_N(opcode) == _0) // Clear Screen
				{
					ClearScreen();
				}
				else if (_N(opcode) == _E) // Subroutine Return
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
			m_pc = (u16*)(m_ram + _NNN(opcode));
			break;
		}
		case _2: // Execute Subroutine
		{
			m_stack[m_stackIdx] = *m_pc;
			m_stackIdx += 1;
			m_pc = (u16*)(m_ram + _NNN(opcode));
			break;
		}
		case _6: // Set
		{
			m_V[_X(opcode)] = _NN(opcode);
			break;
		}
		case _7: // Add
		{
			m_V[_X(opcode)] += _NN(opcode);
			break;
		}
		case _A: // Set index
		{
			m_I = (u16*)(m_ram + _NNN(opcode));
			break;
		}
		case _D: // Display
		{
			int _x = m_V[_X(opcode)] % WIDTH_PIXELS;
			int _y = m_V[_Y(opcode)];
			u8* _i = (u8*)m_I;
			bool setFlag = false;

			for (int y = 0; y < _N(opcode); y++)
			{
				//int renderPos = _x + ((_y + y) * WIDTH_PIXELS);
				//m_renderData[renderPos] = m_renderData[renderPos] ^ 
				
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
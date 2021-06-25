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
#define FONT_HEIGHT (5)

c8e_CPU::c8e_CPU()
{
	m_ram = (u8*)calloc(RAM_SIZE, sizeof(u8));
	m_pc = (u16*)(m_ram + PROGRAM_OFFSET);

	m_stack = (u16**)calloc(STACK_SIZE, sizeof(u16*));
	m_stackIdx = 0;

	m_V = (u8*)calloc(NUM_REGISTERS, sizeof(u16));

	InitFont();

	m_renderData = (bool*) malloc((WIDTH_PIXELS * HEIGHT_PIXELS) * sizeof(bool));
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
	for (int i = 0; i < (16 * FONT_HEIGHT); i++)
	{
		m_ram[FONT_OFFSET + i] = fontData[i];
	}
}

void c8e_CPU::LoadRom()
{
	//const char* romName = "IBMLogo.ch8";
	//const char* romName = "bc_test.ch8";
	//const char* romName = "test_opcode.ch8";
	//const char* romName = "rockto.ch8";
	//const char* romName = "RPS.ch8";
	//const char* romName = "cavern.ch8";
	const char* romName = "chipquarium.ch8";
	std::ifstream file(romName, std::ios::binary | std::ios::ate);
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

bool c8e_CPU::AdvanceTime()
{
	std::chrono::time_point<std::chrono::system_clock> now = std::chrono::system_clock::now();
	double dt = (double)((now - m_prevDelta) / std::chrono::microseconds(1));
	m_prevDelta = now;

	double clockTick = 1000000 / m_clockspeed;
	double timerTick = 1000000 / m_timerspeed;

	m_clockCount += dt;
	m_timerCount += dt;

	if (m_clockCount >= clockTick)
	{
		// execute instruction cycle
		m_clockCount = fmod(m_clockCount, clockTick);

		u16 opcode = Fetch();
		Decode(opcode);
	}

	if (m_timerCount >= timerTick)
	{
		// update timers
		m_timerCount = fmod(m_timerCount, timerTick);
		if (m_delayCount)
		{
			m_delayCount--;
		}
		if (m_soundCount)
		{
			m_soundCount--;
		}
		return true; // Only render 60 times a second
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
#define _VF (m_V[0x0f])

void c8e_CPU::Decode(u16 opcode)
{
	switch (_INSTRUCTION(opcode))
	{
		case 0x00:
		{
			if (_Y(opcode) == 0x0e)
			{
				if (_N(opcode) == 0x00) // Clear Screen
				{
					ClearScreen();
				}
				else if (_N(opcode) == 0x0e) // Subroutine return (pop)
				{
					m_stackIdx -= 1;
					m_pc = m_stack[m_stackIdx];
				}
				else
				{
					assert(true); // unhandled instruction!
				}
			}
			break;
		}
		case 0x01: // Jump
		{
			m_pc = (u16*)(m_ram + _NNN(opcode));
			break;
		}
		case 0x02: // Call Subroutine (push)
		{
			m_stack[m_stackIdx] = m_pc;
			m_stackIdx += 1;
			m_pc = (u16*)&m_ram[_NNN(opcode)];
			break;
		}
		case 0x03: // Skip if equal to immediate
		{
			if (m_V[_X(opcode)] == _NN(opcode))
			{
				m_pc += 1;
			}
			break;
		}
		case 0x04: // Skip if not equal to immediate
		{
			if (m_V[_X(opcode)] != _NN(opcode))
			{
				m_pc += 1;
			}
			break;
		}
		case 0x05: // Skip if registers are equal
		{
			if (m_V[_X(opcode)] == m_V[_Y(opcode)])
			{
				m_pc += 1;
			}
			break;
		}
		case 0x09: // Skip if registers are not equal
		{
			if (m_V[_X(opcode)] != m_V[_Y(opcode)])
			{
				m_pc += 1;
			}
			break;
		}
		case 0x06: // Set
		{
			m_V[_X(opcode)] = _NN(opcode);
			break;
		}
		case 0x07: // Add
		{
			m_V[_X(opcode)] += _NN(opcode);
			break;
		}
		case 0x08: // Arithmetic instructions
		{
			switch (_N(opcode))
			{
				case 0x00: // Set
				{
					m_V[_X(opcode)] = m_V[_Y(opcode)];
					break;
				}
				case 0x01: // OR
				{
					m_V[_X(opcode)] = m_V[_X(opcode)] | m_V[_Y(opcode)];
					break;
				}
				case 0x02: // AND
				{
					m_V[_X(opcode)] = m_V[_X(opcode)] & m_V[_Y(opcode)];
					break;
				}
				case 0x03: // XOR
				{
					m_V[_X(opcode)] = m_V[_X(opcode)] ^ m_V[_Y(opcode)];
					break;
				}
				case 0x04: // Add
				{
					u8 val = m_V[_X(opcode)] + m_V[_Y(opcode)];
					_VF = (val < m_V[_X(opcode)]) || (val < m_V[_Y(opcode)]);
					m_V[_X(opcode)] = val;
					break;
				}
				case 0x05: // Subtraction (X - Y)
				{
					_VF = (m_V[_X(opcode)] >= m_V[_Y(opcode)]);
					m_V[_X(opcode)] = m_V[_X(opcode)] - m_V[_Y(opcode)];
					break;
				}
				case 0x07: // Subtraction (Y - X)
				{
					_VF = (m_V[_Y(opcode)] >= m_V[_X(opcode)]);
					m_V[_X(opcode)] = m_V[_Y(opcode)] - m_V[_X(opcode)];
					break;
				}
				case 0x06: // Shift right
				{
					_VF = m_V[_X(opcode)] & 0x01;
					m_V[_X(opcode)] = m_V[_X(opcode)] >> 1;
					break;
				}
				case 0x0e: // Shift left
				{
					_VF = (m_V[_X(opcode)] & 0x80) > 0;
					m_V[_X(opcode)] = m_V[_X(opcode)] << 1;
					break;
				}
				default:
				{
					assert(true); // unhandled instruction!
					break;
				}
			}
			break;
		}
		case 0x0a: // Set index
		{
			m_I = _NNN(opcode);
			break;
		}
		case 0x0b: // Jump with offset
		{
			m_pc = (u16*)m_ram[_NNN(opcode) + m_V[0]];
			break;
		}
		case 0x0c: // Random
		{
			u16 rnd = rand() % 256;
			m_V[_X(opcode)] = rnd & _NN(opcode);
			break;
		}
		case 0x0d: // Display
		{
			int _x = m_V[_X(opcode)] % WIDTH_PIXELS;
			int _y = m_V[_Y(opcode)] % HEIGHT_PIXELS;
			u8* _i = &m_ram[m_I];
			bool setFlag = false;

			for (int y = 0; y < _N(opcode); y++)
			{
				int currentY = _y + y;
				if (currentY >= HEIGHT_PIXELS) { break; }
				u8 drawMask = 0x80;
				for (int x = 0; x < 8; x++)
				{
					int currentX = _x + x;
					if (currentX >= WIDTH_PIXELS) { break; }
					if (_i[y] & drawMask)
					{
						int renderPos = currentX + (currentY * WIDTH_PIXELS);
						m_renderData[renderPos] = !m_renderData[renderPos];
						if (!m_renderData[renderPos])
						{
							setFlag = true;
						}
					}
					drawMask = (drawMask >> 1);
				}
			}
			_VF = setFlag;
			break;
		}
		case 0x0e: // Skip based on input
		{
			switch (_NN(opcode))
			{
				case 0x9e: // Skip if key pressed
				{
					if (m_input[m_V[_X(opcode)]])
					{
						m_pc += 1;
					}
					break;
				}
				case 0xa1: // Skip if key not pressed
				{
					if (!m_input[m_V[_X(opcode)]])
					{
						m_pc += 1;
					}
					break;
				}
				default:
				{
					assert(true); // unhandled instruction!
					break;
				}
			}
			break;
		}
		case 0x0f: // Miscellaneous
		{
			switch (_NN(opcode))
			{
				case 0x07: // Read delay timer
				{
					m_V[_X(opcode)] = m_delayCount;
					break;
				}
				case 0x15: // Set delay timer
				{
					m_delayCount = m_V[_X(opcode)];
					break;
				}
				case 0x18: // Set sound timer
				{
					m_soundCount = m_V[_X(opcode)];
					break;
				}
				case 0x0a: // Wait for input
				{
					for (u8 i = 0; i <= 0x0f; i++)
					{
						if (m_input[i])
						{
							m_V[_X(opcode)] = i;
							return;
						}
					}
					m_pc -= 1;
					break;
				}
				case 0x1e: // Add to index
				{
					u16 newOffset = m_I + m_V[_X(opcode)];
					_VF = newOffset < m_I;
					m_I = newOffset;
					break;
				}
				case 0x29: // Font character
				{
					u8 ch = ((m_V[_X(opcode)] & 0x0F) * FONT_HEIGHT);
					m_I = FONT_OFFSET + ch;
					break;
				}
				case 0x33: // Binary-coded decimal conversion
				{
					u8 dec = m_V[_X(opcode)];
					u8 dec1 = dec / 100;
					u8 dec2 = (dec % 100) / 10;
					u8 dec3 = (dec % 10);
					m_ram[m_I] = dec1;
					m_ram[m_I + 1] = dec2;
					m_ram[m_I + 2] = dec3;
					break;
				}
				case 0x55: // Store memory
				{
					for (int i = 0; i <= m_V[_X(opcode)]; i++)
					{
						m_ram[m_I + i] = m_V[i];
					}
					break;
				}
				case 0x65: // Load memory
				{
					for (int i = 0; i <= m_V[_X(opcode)]; i++)
					{
						m_V[i] = m_ram[m_I + i];
					}
					break;
				}
				default:
				{
					assert(true); // unhandled instruction!
					break;
				}
			}
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
	for (int i = 0; i < (WIDTH_PIXELS * HEIGHT_PIXELS); i++)
	{
		m_renderData[i] = 0;
	}
}
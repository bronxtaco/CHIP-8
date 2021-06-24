#pragma once

#include <chrono>

typedef unsigned char u8;
typedef unsigned short u16;

#define DEFAULT_CLOCKSPEED (700)
#define TIMERSPEED (60)

struct c8e_CPU
{
public:
	c8e_CPU();
	~c8e_CPU();

	int GetClockSpeed() { return m_clockspeed; }
	bool* GetRenderData() {	return m_renderData; }

	bool ExecuteInstructionCycle();

private:
	void InitFont();
	void LoadRom();

	void ClearScreen();

	u16 Fetch();
	void Decode(u16 opcode);

	std::chrono::time_point<std::chrono::system_clock> m_prevDelta = std::chrono::system_clock::now();
	int m_clockspeed = DEFAULT_CLOCKSPEED; // store in member variable so could be made variable, guide suggested 700
	double m_clockCount = 0;

	int m_timerspeed = TIMERSPEED;
	double m_delayCount = 0;
	double m_soundCount = 0;

	u8* m_ram; // memory
	u16* m_pc; // program counter

	u16* m_stack; // stack of addresses
	int m_stackIdx;

	u16* m_I; // index register

	u8* m_V; // variable registers

	bool* m_renderData; // array of booleans to render (true) or not (false)
};
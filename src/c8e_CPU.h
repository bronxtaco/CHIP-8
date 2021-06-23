#pragma once

#include <chrono>

typedef char u8;
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

	u8* m_ram;
	u16* m_pc;

	u16* m_stack;
	u16* m_stackPtr;
	bool* m_renderData;
};
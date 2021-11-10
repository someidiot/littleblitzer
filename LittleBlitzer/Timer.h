#pragma once

#include <iostream>
#include <windows.h>

class CTimer
{
public:
	CTimer(void);
	~CTimer(void);

	void Start();
	void Stop();
	double GetMS();		// Get ms elapsed time

private:
	LARGE_INTEGER m_nFreq;		// ticks per second
    LARGE_INTEGER m_nT1, m_nT2;	// ticks
};

#include "StdAfx.h"
#include "Timer.h"

CTimer::CTimer(void)
{
	// get ticks per second
    QueryPerformanceFrequency(&m_nFreq);
	m_nT1.QuadPart = 0;
	m_nT2.QuadPart = 0;
}

CTimer::~CTimer(void)
{
}

void CTimer::Start() {
	// start timer
    QueryPerformanceCounter(&m_nT1);
}

void CTimer::Stop() {
	// stop timer
    QueryPerformanceCounter(&m_nT2);
}

double CTimer::GetMS() {
    // compute and print the elapsed time in millisec
	if (m_nT2.QuadPart == 0) {
		LARGE_INTEGER i;
		QueryPerformanceCounter(&i);
		return (i.QuadPart - m_nT1.QuadPart) * 1000.0 / m_nFreq.QuadPart;
	} else {
		return (m_nT2.QuadPart - m_nT1.QuadPart) * 1000.0 / m_nFreq.QuadPart;
	}
}

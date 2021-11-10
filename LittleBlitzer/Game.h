#pragma once

class CGame
{
public:
	CGame(void);
	~CGame(void);

	long m_nBase;			// Base time in msec
	long m_nInc;			// Increment time in msec
	bool m_bPonder;			// Pondering on/off (1/0)

	void Start();
};

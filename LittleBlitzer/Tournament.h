#pragma once

#include "Engine.h"
#include "Board.h"
#include "Move.h"

#define MAX_START_POSITIONS	32000 // v2.71 10k to 20k

#define TC_FIXED_TPM	0
#define TC_BLITZ		1
#define TC_TOURNAMENT	2

class CTournament
{
public:
	CTournament(void);
	~CTournament(void);

	int  m_nType;			// Type of tournament (0 = Gauntlet, 1 = Round Robin)
	long m_nBase;			// Base time in msec
	long m_nInc;			// Increment time in msec
	long m_nRound;			// Round# (used to determine white/black)
	long m_nHash;			// Standard UCI options which all engines should understand
	bool m_bPonder;			// Pondering on/off (1/0)
	bool m_bOwnBook;
	int m_nVariant;			// Variant to play
	int m_nTC;				// Time Control (0=fixed tpm, 1=base+inc, 2=tournament) uses m_nBase and m_nInc for all values

	int m_nNumStartPositions;	// Num entries in the following array
	char **m_sStartPositions;	// Array of FEN starting positions to use
	int m_nRandomize;

	long m_nAdjMateScore;
	long m_nAdjMateMoves;
	long m_nAdjDrawMoves;

	bool m_bRunning;

	long m_nWastedTime;			// Cumulative time spent starting/stopping engines between actual games

	long m_nNumEngines;
	CEngine m_Engines[100];		// Engines used
	CEngine m_CurrEngines[2];
	CWnd *m_pWnd;				// Window to send result messages to
	int m_nThreadID;			// 0,1,2... index into global arrays

	//volatile bool m_bStopRequested;	// Set to true by GUI when Stop requested by user
	//volatile bool m_bPaused;		// Set to true by GUI when Pause requested by user

	void Start();
	void DumpIllegalMove(TBoard *b, TMove m, char *sStartingPosition, CString sMoveList, CEngine *e);
	void Abort();

};

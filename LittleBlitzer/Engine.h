#pragma once

#include "Common.h"
#include "Timer.h"

#define IO_BUFFER		16384
#define MAX_PARMS		100
//#define MAX_PARM_LEN	50

class CEngine
{
public:
	CEngine(void);
	~CEngine(void);

	tLock	m_nLockEngine;	// Protect against multiple threads doing the same thing

	//
	// Engine details
	//

	char *m_sPath;			// Location of engine
	char *m_sName;			// Results of UCI id command
	char *m_sLBName;		// LB_Name parameter if it exists
	char *m_sAuthor;
	long m_nHash;			// Standard UCI options which all engines should understand
	bool m_bPonder;
	bool m_bOwnBook;
	int m_nVariant;
	char **m_sParameterNames;
	char **m_sParameterValues;
	int m_nNumParameters;
	char m_sLastOutput[100];

	//
	// Communications
	//

	HANDLE m_nPipeRead[2];  // [READ|WRITE]
	HANDLE m_nPipeWrite[2]; // [READ|WRITE]
	int m_nBufferSize;		// Number of characters waiting in the buffer
	char *m_sBuffer[2];		 // [READ|WRITE][chars]
	HANDLE m_nProcess;		// Handle to engine process

	void Send(CString sLine);
	void Send(const char format[], ...);
	void GetLine(CString *sLine);
	bool IsDataWaiting();
	bool UpdateBuffer();
	void ProcessInput(CString sLine, long *nDepth, long *nNPS, long *nScore);
	int GetWords(CString sLine, CStringArray *sWords);

	//
	// Playing
	//

	bool Init();
	void NewGame();
	CString Search(CString sStartingPositionFEN, CString sMoves, int nTC, long nWhiteTime, long nBlackTime, long nWhiteInc, long nBlackInc, long nTimeOut, long *nDepth, long *nNPS, long *nScore, CTimer *t);
	void Stop();
	void Quit();

};

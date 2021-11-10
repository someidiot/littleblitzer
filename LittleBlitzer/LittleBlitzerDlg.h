
// LittleBlitzerDlg.h : header file
//

#pragma once

#include "Engine.h"
#include "Tournament.h"
#include "afxwin.h"
#include "Timer.h"
#include "Common.h"

// CLittleBlitzerDlg dialog
class CLittleBlitzerDlg : public CDialog
{
// Construction
public:
	CLittleBlitzerDlg(CWnd* pParent = NULL);	// standard constructor
	~CLittleBlitzerDlg();

// Dialog Data
	enum { IDD = IDD_LITTLEBLITZER_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:


	long m_nNumEngines;
	CEngine m_Engines[100];			// Engines used
	int m_nNumTournaments;			// Requested number of running tournaments
	volatile int m_nNumActiveTournaments;	// Actual number of running tournaments
	//CTournament m_Tournaments[MAX_THREADS];	// Parallel Tournaments (each running m_Tournaments.m_nRounds games)
	CTournament *m_Tournaments;	// Parallel Tournaments (each running m_Tournaments.m_nRounds games)
	int m_nStartPositionType;
	char m_sStartPositionFile[MAX_FEN_LEN];
	long m_nNumGames;
	tLock m_nLockGameNum;
	volatile long m_nGameNum;	// Game number to start next
	volatile long m_nNumGamesPlayed;	// Number of games completed
	volatile long *m_nResults[NUM_TYPES];	// [ResultType][Engine] counts
	volatile long *m_nWins;
	volatile long *m_nLosses;
	volatile long *m_nDraws;
	//volatile long *m_nScores;		// [Engine] scores
	volatile long *m_nGames;		// [Engine] count
	volatile double *m_dTotalTime;
	volatile double *m_dTotalSearches;
	volatile double m_dTotalGamesLen;	// time of all games
	volatile long *m_nTotalDepth;
	volatile long *m_nTotalDepthCount;
	volatile long long *m_nTotalNPS;
	volatile long long *m_nTotalNPSCount;

	bool m_bPaused;

	CTimer m_nTimeTaken;
	CEdit m_wndResults;

	bool InitPGN();
	void LoadEngineSettings();
	static UINT RunTournament(void *pParam);
	LRESULT OnGameDone(WPARAM wParam, LPARAM lParam);
	void UpdateResults();
	void UpdatePGN(TResult *r);
	CButton m_wndPause;
	CEdit m_wndEngineFile;
	afx_msg void OnBnClickedLoadEngines();
	afx_msg void OnBnClickedLoadTournament();
	afx_msg void OnBnClickedOpenEngines();
	afx_msg void OnBnClickedOpenTourn();
	afx_msg void OnBnClickedStart();
	afx_msg void OnBnClickedPause();
	CEdit m_wndTournFile;
	CEdit m_wndSavePGN;
	CButton m_wndStart;
	afx_msg void OnBnClickedCopytext();
	CStatic m_wndNumThreads;
	afx_msg void OnBnClickedInc();
	afx_msg void OnBnClickedDec();
	void UpdateNumTourneys();
	afx_msg void OnBnClickedCheck1();
	CButton m_wndChkLog;
	afx_msg void OnBnClickedChkLog();
	CButton m_wndLoadEngines;
	CButton m_wndLoadTournament;
	//void UpdateTournamentBox();
	void LoadEPDPositions(char *sPath);
	void LoadPGNPositions(char *sPath);
	int GetNextRound();
	afx_msg void OnBnClickedChkIllegal();
	afx_msg void OnBnClickedChkFullPGN();
	CButton m_wndDumpIllegalMoves;
	CButton m_wndFullPGN;
	char ReadLine(FILE *f, char *s);
};

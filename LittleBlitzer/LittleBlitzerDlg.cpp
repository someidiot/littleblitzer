
// LittleBlitzerDlg.cpp : implementation file
//

#include "stdafx.h"
#include "LittleBlitzer.h"
#include "LittleBlitzerDlg.h"
#include "Common.h"
#include "TournSettings.h"
#include "Board.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CString m_sResultsPath;

// CLittleBlitzerDlg dialog


CLittleBlitzerDlg::CLittleBlitzerDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CLittleBlitzerDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

	m_nGameNum = 0;
	m_nNumGamesPlayed = 0;
	m_nNumEngines = 0;
	m_nNumActiveTournaments = 0;
	m_nNumTournaments = 4;
	m_nNumGames = 50000;
	m_nStartPositionType = 0;

	for (int i=0;i<NUM_TYPES;i++)
		m_nResults[i] = 0;
	m_nWins = 0;
	m_nLosses = 0;
	m_nDraws = 0;
	m_nGames = 0;
	m_dTotalTime = NULL;
	m_dTotalSearches = NULL;
	m_nTotalDepth = NULL;
	m_nTotalDepthCount = NULL;
	m_nTotalNPS = NULL;
	m_nTotalNPSCount = NULL;
	m_dTotalGamesLen = 0;

	//m_Tournaments = (CTournament*)malloc(sizeof(CTournament)*MAX_THREADS);
	m_Tournaments = new CTournament[MAX_THREADS];

	m_Tournaments[0].m_nNumStartPositions = 0;
	m_Tournaments[0].m_sStartPositions = NULL;

	for (int i=0;i<100;i++)
		m_Engines[i].m_sPath = 0;

	srand(time(NULL));	// for CTournament::DumpIllegalMove


	LockInit(&m_nLockGameNum, NULL);
}

CLittleBlitzerDlg::~CLittleBlitzerDlg()
{
	for (int i=0;i<m_nNumTournaments;i++)
		m_Tournaments[i].Abort();
	//Sleep(1000);

	for (int i=0;i<NUM_TYPES;i++)
		delete m_nResults[i];
	delete m_nWins;
	delete m_nLosses;
	delete m_nDraws;
	delete m_nGames;
	delete m_dTotalTime;
	delete m_dTotalSearches;
	delete m_nTotalDepth;
	delete m_nTotalDepthCount;
	delete m_nTotalNPS;
	delete m_nTotalNPSCount;

	for (int w=0;w<m_Tournaments[0].m_nNumStartPositions;w++) {
		delete m_Tournaments[0].m_sStartPositions[w];
	}
	delete m_Tournaments[0].m_sStartPositions;
	delete m_Tournaments;

	//for (int i=0;i<m_nNumEngines;i++)
	//	delete m_Engines[i].m_sPath;

}

void CLittleBlitzerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_STATUS, m_wndResults);
	DDX_Control(pDX, IDC_PAUSE, m_wndPause);
	DDX_Control(pDX, IDC_ENGINE_FILE, m_wndEngineFile);
	DDX_Control(pDX, IDC_TOURN_FILE, m_wndTournFile);
	DDX_Control(pDX, IDC_SAVE_PGN, m_wndSavePGN);
	DDX_Control(pDX, IDC_START, m_wndStart);
	DDX_Control(pDX, IDC_NUMTHREADS, m_wndNumThreads);
	DDX_Control(pDX, IDC_CHK_LOG, m_wndChkLog);
	DDX_Control(pDX, IDC_LOAD_ENGINES, m_wndLoadEngines);
	DDX_Control(pDX, IDC_TOURNAMENT, m_wndLoadTournament);
	DDX_Control(pDX, IDC_CHK_ILLEGAL, m_wndDumpIllegalMoves);
	DDX_Control(pDX, IDC_CHK_FULLPGN, m_wndFullPGN);
}

BEGIN_MESSAGE_MAP(CLittleBlitzerDlg, CDialog)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_TOURNAMENT, &CLittleBlitzerDlg::OnBnClickedLoadTournament)
	ON_BN_CLICKED(IDC_START, &CLittleBlitzerDlg::OnBnClickedStart)

	ON_MESSAGE(GAME_DONE,	&CLittleBlitzerDlg::OnGameDone)
	//ON_MESSAGE(TOURN_DONE,  &CLittleBlitzerDlg::OnTournDone)

	ON_BN_CLICKED(IDC_PAUSE, &CLittleBlitzerDlg::OnBnClickedPause)
	ON_BN_CLICKED(IDC_OPEN_ENGINES, &CLittleBlitzerDlg::OnBnClickedOpenEngines)
	ON_BN_CLICKED(IDC_LOAD_ENGINES, &CLittleBlitzerDlg::OnBnClickedLoadEngines)
	ON_BN_CLICKED(IDC_OPEN_TOURN, &CLittleBlitzerDlg::OnBnClickedOpenTourn)
	ON_BN_CLICKED(IDC_COPYTEXT, &CLittleBlitzerDlg::OnBnClickedCopytext)
	ON_BN_CLICKED(IDC_INC, &CLittleBlitzerDlg::OnBnClickedInc)
	ON_BN_CLICKED(IDC_DEC, &CLittleBlitzerDlg::OnBnClickedDec)
	ON_BN_CLICKED(IDC_CHK_LOG, &CLittleBlitzerDlg::OnBnClickedChkLog)
	ON_BN_CLICKED(IDC_CHK_ILLEGAL, &CLittleBlitzerDlg::OnBnClickedChkIllegal)
	ON_BN_CLICKED(IDC_CHK_FULLPGN, &CLittleBlitzerDlg::OnBnClickedChkFullPGN)
END_MESSAGE_MAP()


// CLittleBlitzerDlg message handlers

BOOL CLittleBlitzerDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	//wchar_t sFileW[1024]; size_t dumb; // i hate ms unicode support
	char sFile[1024];
	GetModuleFileName(0, sFile, 1024);
	//wcstombs_s(&dumb, sFile, wcslen(sFileW)+1, sFileW, _TRUNCATE);
	char *sPath = GetFilePath(sFile);
	char sConfigPath[1024];
	sprintf(sConfigPath, _T("%s\\Engines.lbe"), sPath);
	m_wndEngineFile.SetWindowText((LPCTSTR)sConfigPath);
	sprintf(sConfigPath, _T("%s\\Tournament.lbt"), sPath);
	m_wndTournFile.SetWindowText((LPCTSTR)sConfigPath);
	sprintf(sConfigPath, _T("%s\\Results.pgn"), sPath);
	m_wndSavePGN.SetWindowText((LPCTSTR)sConfigPath);
	delete sPath;

	m_bPaused = false;

	m_wndResults.SetWindowText(_T("No engines loaded"));
	m_wndDumpIllegalMoves.SetCheck(g_bDumpIllegalMoves);
	m_wndFullPGN.SetCheck(true);
	g_bFullPGN = true;

	UpdateNumTourneys();

	CString s;
	s.Format(_T("LittleBlitzer %s"), _T(VERSION));
	this->SetWindowText(s);

	InitialiseArrays();

	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CLittleBlitzerDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CLittleBlitzerDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void CLittleBlitzerDlg::OnBnClickedLoadTournament()
{
	CString sPathName;
	m_wndTournFile.GetWindowText(sPathName);

	/*if (m_nNumEngines == 0) {
		// Need to know number of engines
		MessageBox(_T("Load engines first!"),_T("Error"));
		return;
	}*/

	FILE *f = fopen(sPathName.GetBuffer(), "rt");
	if (f) {

		m_nNumTournaments = 0;
		//m_nNumActiveTournaments = 0;
		//for (int i=0;i<NUM_TYPES;i++) {
		//	m_nResults[i] = new long[m_nNumEngines];
		//	memset((long *)m_nResults[i], 0, sizeof(long)*m_nNumEngines);
		//}
		//m_nGames = new long[m_nNumEngines];
		//memset((long *)m_nGames, 0, sizeof(long)*m_nNumEngines);
		//m_nWins = new long[m_nNumEngines];
		//memset((long *)m_nWins, 0, sizeof(long)*m_nNumEngines);
		//m_nLosses = new long[m_nNumEngines];
		//memset((long *)m_nLosses, 0, sizeof(long)*m_nNumEngines);
		//m_nDraws = new long[m_nNumEngines];
		//memset((long *)m_nDraws, 0, sizeof(long)*m_nNumEngines);
		//m_dTotalTime = new double[m_nNumEngines];
		//memset((double *)m_dTotalTime, 0, sizeof(double)*m_nNumEngines);
		//m_dTotalSearches = new double[m_nNumEngines];
		//memset((double *)m_dTotalSearches, 0, sizeof(double)*m_nNumEngines);
		//m_nTotalDepth = new long[m_nNumEngines];
		//memset((long *)m_nTotalDepth, 0, sizeof(long)*m_nNumEngines);
		//m_nTotalDepthCount = new long[m_nNumEngines];
		//memset((long *)m_nTotalDepthCount, 0, sizeof(long)*m_nNumEngines);
		//m_nTotalNPS = new long long[m_nNumEngines];
		//memset((long long *)m_nTotalNPS, 0, sizeof(long long)*m_nNumEngines);
		//m_nTotalNPSCount = new long long[m_nNumEngines];
		//memset((long long *)m_nTotalNPSCount, 0, sizeof(long long)*m_nNumEngines);


		char sParam[50];
		long nValue = 0;
		char sValue[100];

		while (fscanf(f, "%s:", sParam) != EOF) {
			fscanf(f, "%s", &sValue);
			if        (!stricmp(sParam, "Type:")) {			// Type of tournament (0 = Gauntlet, 1 = Round Robin)
				m_Tournaments[0].m_nType = atol(sValue);
			} else if (!stricmp(sParam, "TC:")) {			// Type of time control
				m_Tournaments[0].m_nTC = atol(sValue);
			} else if (!stricmp(sParam, "Base:")) {			// Base time in msec
				m_Tournaments[0].m_nBase = atol(sValue);
			} else if (!stricmp(sParam, "Inc:")) {			// Increment time in msec
				m_Tournaments[0].m_nInc = atol(sValue);
			} else if (!stricmp(sParam, "Rounds:")) {		// Rounds to play
				m_nNumGames = atol(sValue);
			} else if (!stricmp(sParam, "Hash:")) {			// Hash size in MB
				m_Tournaments[0].m_nHash = atol(sValue);
			} else if (!stricmp(sParam, "Ponder:")) {		// Pondering on/off (1/0)
				m_Tournaments[0].m_bPonder = atol(sValue);
			} else if (!stricmp(sParam, "OwnBook:")) {		// OwnBook on/off (1/0)
				m_Tournaments[0].m_bOwnBook = atol(sValue);
			} else if (!stricmp(sParam, "Variant:")) {		// Variant name (0=Standard, 1=FRC)
				m_Tournaments[0].m_nVariant = atol(sValue);
			} else if (!stricmp(sParam, "NumParallel:")) {	// Number of parallel tournaments to split the rounds into
				m_nNumTournaments = atol(sValue);
				m_nNumTournaments = MIN(MAX_THREADS, m_nNumTournaments);
				m_nNumTournaments = MAX(0, m_nNumTournaments);
			} else if (!stricmp(sParam, "AdjudicateMateScore:")) {		// Adjudication score threshold for win/loss
				m_Tournaments[0].m_nAdjMateScore = atol(sValue);
				if (m_Tournaments[0].m_nAdjMateScore < 0) m_Tournaments[0].m_nAdjMateScore *= -1; // only store positive score
			} else if (!stricmp(sParam, "AdjudicateMateMoves:")) {		// Number of moves for score to be above/below AdjudicateMateScore
				m_Tournaments[0].m_nAdjMateMoves = atol(sValue);
				m_Tournaments[0].m_nAdjMateMoves = MIN(m_Tournaments[0].m_nAdjMateMoves, 1000);	// not too big or the dynamic mem allocation slows it down too much
			} else if (!stricmp(sParam, "AdjudicateDrawMoves:")) {		// Number of moves before declaring draw
				m_Tournaments[0].m_nAdjDrawMoves = atol(sValue);
			} else if (!stricmp(sParam, "Position:")) {		// Starting position(s) to use
				if (sValue[0] == 'F' && sValue[1] == 'E' && sValue[2] == 'N') {
					// Load FEN string directly
					m_nStartPositionType = 1;
					m_sStartPositionFile[0] = 0;
					m_Tournaments[0].m_nNumStartPositions = 1;
					m_Tournaments[0].m_sStartPositions = new char*[1];

					char sFEN[128];
					strncpy(sFEN, sValue+4, 128);
					int nFENLen = strlen(sValue)-4;
					while (1) { // get any more characters after the whitespace in the FEN string (2.6)
						char c=fgetc(f);
						if (c == EOF || c == '\n') break;
						sFEN[nFENLen++] = c;
					}
					sFEN[nFENLen] = 0;

					m_Tournaments[0].m_sStartPositions[0] = new char[strlen(sFEN)+1];
					strcpy(m_Tournaments[0].m_sStartPositions[0], sFEN);
				} else if (sValue[0] == 'E' && sValue[1] == 'P' && sValue[2] == 'D') {
					// Load EPD file of positions
					m_nStartPositionType = 2;
					char *sPath = sValue+4;

					char sFile[128];
					strncpy(sFile, sPath, 128);
					int nFileLen = strlen(sPath);
					while (1) { // get any more characters after the whitespace in the string (2.71)
						char c=fgetc(f);
						if (c == EOF || c == '\n') break;
						sFile[nFileLen++] = c;
					}
					sFile[nFileLen] = 0;

					strcpy(m_sStartPositionFile, sFile);
					//LoadEPDPositions(sFile);
				} else if (sValue[0] == 'P' && sValue[1] == 'G' && sValue[2] == 'N') {
					// Load PGN file of positions
					m_nStartPositionType = 3;
					char *sPath = sValue+4;

					char sFile[128];
					strncpy(sFile, sPath, 128);
					int nFileLen = strlen(sPath);
					while (1) { // get any more characters after the whitespace in the string (2.71)
						char c=fgetc(f);
						if (c == EOF || c == '\n') break;
						sFile[nFileLen++] = c;
					}
					sFile[nFileLen] = 0;

					strcpy(m_sStartPositionFile, sFile);
					//LoadPGNPositions(sFile);
				} else {
					// default to standard opening
					m_nStartPositionType = 0;
					m_Tournaments[0].m_sStartPositions = new char*[1];
					m_Tournaments[0].m_sStartPositions[0] = new char[100];
					m_Tournaments[0].m_nNumStartPositions = 1;
					strcpy(m_Tournaments[0].m_sStartPositions[0], "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq -"); // v2.71 change to w move!
				}
			} else if (!stricmp(sParam, "Randomize:")) {		// Randomize EPD/PGN positions
				m_Tournaments[0].m_nRandomize = atol(sValue);
			}
		}
		fclose(f);

		for (int i=1;i<MAX_THREADS;i++) {
			m_Tournaments[i] = m_Tournaments[0];
		}

	} else {
		MessageBox(_T("Tournament file not found, will be created using defaults"),_T("Warning"));
	}




	CTournSettings dlg;
	dlg.m_nRounds = m_nNumGames;
	dlg.m_nParallel = m_nNumTournaments;
	dlg.m_nHash = m_Tournaments[0].m_nHash;
	dlg.m_nType = m_Tournaments[0].m_nType;
	dlg.m_nVariant = m_Tournaments[0].m_nVariant;
	dlg.m_nPosition = m_nStartPositionType;
	dlg.m_nAdjMateScore = m_Tournaments[0].m_nAdjMateScore;
	dlg.m_nAdjMateMoves = m_Tournaments[0].m_nAdjMateMoves;
	dlg.m_nAdjDrawMoves = m_Tournaments[0].m_nAdjDrawMoves;
	dlg.m_nTimeBase = m_Tournaments[0].m_nBase;
	dlg.m_nTimeInc = m_Tournaments[0].m_nInc;
	dlg.m_nPonder = m_Tournaments[0].m_bPonder;
	dlg.m_nOwnBook = m_Tournaments[0].m_bOwnBook;
	dlg.m_nTC = m_Tournaments[0].m_nTC;
	if (m_nStartPositionType == 0) {	// OPENING
		dlg.m_sStartingPosition[0] = 0;
	} else if (m_nStartPositionType == 1) {	// FEN
		strcpy(dlg.m_sStartingPosition, m_Tournaments[0].m_sStartPositions[0]);
	} else {
		strcpy(dlg.m_sStartingPosition, m_sStartPositionFile);
	}
	dlg.m_nRandomize = m_Tournaments[0].m_nRandomize;
	
	INT_PTR res = dlg.DoModal();
	if (res != IDOK) return;
	
	::SetCursor(AfxGetApp()->LoadStandardCursor(IDC_WAIT));

	m_nNumGames = dlg.m_nRounds;
	m_nNumTournaments = dlg.m_nParallel;
	m_Tournaments[0].m_nHash = dlg.m_nHash;
	m_Tournaments[0].m_nType = dlg.m_nType;
	m_Tournaments[0].m_nVariant = dlg.m_nVariant;
	m_Tournaments[0].m_nAdjMateScore = dlg.m_nAdjMateScore;
	m_Tournaments[0].m_nAdjMateMoves = dlg.m_nAdjMateMoves;
	m_Tournaments[0].m_nAdjDrawMoves = dlg.m_nAdjDrawMoves;
	m_Tournaments[0].m_nTC = dlg.m_nTC;
	m_Tournaments[0].m_nBase = dlg.m_nTimeBase;
	m_Tournaments[0].m_nInc = dlg.m_nTimeInc;
	m_Tournaments[0].m_bPonder = dlg.m_nPonder;
	m_Tournaments[0].m_bOwnBook = dlg.m_nOwnBook;
	m_nStartPositionType = dlg.m_nPosition;
	char sFEN[MAX_FEN_LEN];
	strcpy(sFEN, dlg.m_sStartingPosition);
	if (m_nStartPositionType == 1) {
		// Load FEN string directly
		m_nStartPositionType = 1;
		m_sStartPositionFile[0] = 0;
		m_Tournaments[0].m_nNumStartPositions = 1;
		m_Tournaments[0].m_sStartPositions = new char*[1];
		m_Tournaments[0].m_sStartPositions[0] = new char[strlen(sFEN)];
		strcpy(m_Tournaments[0].m_sStartPositions[0], sFEN);
	} else if (m_nStartPositionType == 2) {
		// Load EPD file of positions
		m_nStartPositionType = 2;
		strcpy(m_sStartPositionFile, sFEN);
		LoadEPDPositions(sFEN);
	} else if (m_nStartPositionType == 3) {
		// Load PGN file of positions
		m_nStartPositionType = 3;
		strcpy(m_sStartPositionFile, sFEN);
		LoadPGNPositions(sFEN);
	} else {
		// default to standard opening
		m_nStartPositionType = 0;
		m_Tournaments[0].m_sStartPositions = new char*[1];
		m_Tournaments[0].m_sStartPositions[0] = new char[100];
		m_Tournaments[0].m_nNumStartPositions = 1;
		strcpy(m_Tournaments[0].m_sStartPositions[0], "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq -"); // v2.71 change to w move!
	}
	m_Tournaments[0].m_nRandomize = dlg.m_nRandomize;
	
	for (int i=1;i<MAX_THREADS;i++) {
		m_Tournaments[i] = m_Tournaments[0];
	}
	//UpdateTournamentBox();

	//CString sPathName;
	m_wndTournFile.GetWindowText(sPathName);

	f = fopen(sPathName.GetBuffer(), "wt");
	if (!f) {
		MessageBox(_T("File not found!"),_T("Error"));
		return;
	}

	fprintf(f, "Type: %ld\n", m_Tournaments[0].m_nType);
	fprintf(f, "TC: %ld\n", m_Tournaments[0].m_nTC);
	fprintf(f, "Base: %ld\n", m_Tournaments[0].m_nBase);
	fprintf(f, "Inc: %ld\n", m_Tournaments[0].m_nInc);
	fprintf(f, "Rounds: %ld\n", m_nNumGames);
	fprintf(f, "Ponder: %ld\n", m_Tournaments[0].m_bPonder);
	fprintf(f, "OwnBook: %ld\n", m_Tournaments[0].m_bOwnBook);
	fprintf(f, "Hash: %ld\n", m_Tournaments[0].m_nHash);
	fprintf(f, "NumParallel: %ld\n", m_nNumTournaments);
	fprintf(f, "Variant: %ld\n", m_Tournaments[0].m_nVariant);
	char buf[MAX_FEN_LEN];
	if      (m_nStartPositionType==0) sprintf(buf, "OPENING");
	else if (m_nStartPositionType==1) sprintf(buf, "FEN:%s", sFEN);
	else if (m_nStartPositionType==2) sprintf(buf, "EPD:%s", sFEN);
	else if (m_nStartPositionType==3) sprintf(buf, "PGN:%s", sFEN);
	fprintf(f, "Position: %s\n",  buf);
	fprintf(f, "Randomize: %d\n",  m_Tournaments[0].m_nRandomize);
	fprintf(f, "AdjudicateMateScore: %ld\n", m_Tournaments[0].m_nAdjMateScore);
	fprintf(f, "AdjudicateMateMoves: %ld\n", m_Tournaments[0].m_nAdjMateMoves);
	fprintf(f, "AdjudicateDrawMoves: %ld\n", m_Tournaments[0].m_nAdjDrawMoves);

	fclose(f);


	//UpdateTournamentBox();
	UpdateNumTourneys();
	UpdateResults();

	::SetCursor(AfxGetApp()->LoadStandardCursor(IDC_ARROW));
}

void CLittleBlitzerDlg::LoadEPDPositions(char *sPath) {

	for (int w=0;w<m_Tournaments[0].m_nNumStartPositions;w++) {
		delete m_Tournaments[0].m_sStartPositions[w];
	}
	delete m_Tournaments[0].m_sStartPositions;
	m_Tournaments[0].m_nNumStartPositions = 0;

	m_Tournaments[0].m_sStartPositions = new char*[MAX_START_POSITIONS];
	FILE *fepd = fopen(sPath, "rt");
	if (fepd) {
		do {
			char sFEN[MAX_FEN_LEN];
			int i=0;
			char c=fgetc(fepd);
			if (c==EOF) break;
			while (c!='\n' && c!='\r') {
				if (c==';') {
					// read in rest of line but dont use
					while (c!='\n' && c!='\r' && c!=EOF) c = fgetc(fepd);
				} else {
					sFEN[i++] = c;
					c = fgetc(fepd);
				}
				if (c==EOF) break;
				if (i==MAX_FEN_LEN-1) break;
			}
			sFEN[i]=0;
			m_Tournaments[0].m_sStartPositions[m_Tournaments[0].m_nNumStartPositions] = new char[strlen(sFEN)+1];
			strcpy(m_Tournaments[0].m_sStartPositions[m_Tournaments[0].m_nNumStartPositions++], sFEN);
			if (c==EOF) break;
			if (m_Tournaments[0].m_nNumStartPositions==MAX_START_POSITIONS) break;
		} while (1);
		fclose(fepd);
	} else {
		// default to standard opening
		m_Tournaments[0].m_nNumStartPositions = 1;
		m_Tournaments[0].m_sStartPositions = new char*[1];
		m_Tournaments[0].m_sStartPositions[0] = new char[100];
		strcpy(m_Tournaments[0].m_sStartPositions[0], "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq -"); // v2.71 change to w move!
	}
}

char CLittleBlitzerDlg::ReadLine(FILE *f, char *s) {
	char c=fgetc(f);
	int i=0;
	while (c!='\n' && c!='\r') {
		if (c==';') {
			// read in rest of line but dont use
			while (c!='\n' && c!='\r' && c!=EOF) c = fgetc(f);
		} else {
			s[i++] = c;
			c = fgetc(f);
		}
		if (c==EOF) break;
		if (i==MAX_FEN_LEN-1) break;
	}
	s[i]=0;
	return c;
}

void CLittleBlitzerDlg::LoadPGNPositions(char *sPath) {

	for (int w=0;w<m_Tournaments[0].m_nNumStartPositions;w++) {
		delete m_Tournaments[0].m_sStartPositions[w];
	}
	delete m_Tournaments[0].m_sStartPositions;
	m_Tournaments[0].m_nNumStartPositions = 0;

	m_Tournaments[0].m_sStartPositions = new char*[MAX_START_POSITIONS];
	FILE *fepd = fopen(sPath, "rt");
	if (fepd) {
		do {
			char sLine[MAX_FEN_LEN];
			//int i=0;
			//char c=fgetc(fepd);
			char c = ReadLine(fepd,sLine);
			if (c==EOF) break;
			// read a line
			// process the line
			if (sLine[0]=='[' && sLine[1]=='F' && sLine[2]=='E' && sLine[3]=='N') {
				// [FEN "xxx"] tag = easy
				char sFEN[MAX_FEN_LEN];
				strncpy(sFEN, sLine+6, strlen(sLine)-8);
				sFEN[strlen(sLine)-8] = 0;
				m_Tournaments[0].m_sStartPositions[m_Tournaments[0].m_nNumStartPositions] = new char[strlen(sFEN)+1];
				strcpy(m_Tournaments[0].m_sStartPositions[m_Tournaments[0].m_nNumStartPositions++], sFEN);
			} else if (sLine[0]=='1' && sLine[1]=='.') {
				// 1. moves = ahhh shit
				char sGame[10240];
				sGame[0]=0;
				long len=0;
				do {
					int l=strlen(sLine);
					strncat(sGame, sLine, MIN(10240,l));
					c=ReadLine(fepd,sLine);
					if (c==EOF) break;
					if (sLine[0]==0) break; // assume end of game moves
					if (sLine[0]=='[') break; // assume new game tags
					len+=l;
					sGame[len++]=' ';
					sGame[len]=0;
				} while (1);
				char sFEN[MAX_FEN_LEN];
				GameMoves2FEN(sGame, sFEN);
				m_Tournaments[0].m_sStartPositions[m_Tournaments[0].m_nNumStartPositions] = new char[strlen(sFEN)+1];
				strcpy(m_Tournaments[0].m_sStartPositions[m_Tournaments[0].m_nNumStartPositions++], sFEN);
			}
			if (c==EOF) break;
			if (m_Tournaments[0].m_nNumStartPositions==MAX_START_POSITIONS) break;
		} while (1);
		fclose(fepd);
	} else {
		// default to standard opening
		m_Tournaments[0].m_nNumStartPositions = 1;
		m_Tournaments[0].m_sStartPositions = new char*[1];
		m_Tournaments[0].m_sStartPositions[0] = new char[100];
		strcpy(m_Tournaments[0].m_sStartPositions[0], "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq -"); // v2.71 change to w move!
	}
}

void CLittleBlitzerDlg::LoadEngineSettings() {
	
	CString sConfigPath;
	m_wndEngineFile.GetWindowText(sConfigPath);

	FILE *f = fopen(sConfigPath.GetBuffer(), "rt");
	if (!f) {
		m_nNumEngines = 0;
		MessageBox(_T("File not found!"),_T("Error"));
		return;
	}

	::SetCursor(AfxGetApp()->LoadStandardCursor(IDC_WAIT));

	m_wndResults.SetSel(0,-1);
	m_wndResults.ReplaceSel(_T("Loading engines..."));

	const int nMaxLen = 1024;
	char sEnginePath[nMaxLen];
	CEngine e;

	m_nNumEngines = 0;
	int c=0, i;
	char sLine[nMaxLen];
	CString sUpperLine;
	while (c != EOF) {
		i=0;
		sEnginePath[0] = 0;
		while ((c=fgetc(f))!=EOF && c!='\n' && c!='\r' && i<nMaxLen) sLine[i++] = c;
		sLine[i]=0;
		sUpperLine = sLine;
		sUpperLine.MakeUpper();

		if (sUpperLine.Find("ENGINE") == 0) {
			strncpy(sEnginePath,sLine+7,strlen(sLine)-6);
			e.m_sPath = new char [strlen(sEnginePath)+1];
			strcpy(e.m_sPath, sEnginePath);
			m_Engines[m_nNumEngines++] = e;
			m_Engines[m_nNumEngines-1].m_sParameterNames = new char *[MAX_PARMS];
			m_Engines[m_nNumEngines-1].m_sParameterValues = new char *[MAX_PARMS];
		} else if (sUpperLine.Find('=') != -1) {
			if (m_Engines[m_nNumEngines-1].m_nNumParameters < MAX_PARMS) {
				int pos = sUpperLine.Find('=');
				if (!strncmp(sUpperLine, "LB_NAME", pos)) {
					// Special parameter used to overwrite the engine's name
					//delete m_Engines[m_nNumEngines-1].m_sName;
					m_Engines[m_nNumEngines-1].m_sLBName = new char[strlen(sLine)-pos];
					strncpy(m_Engines[m_nNumEngines-1].m_sLBName, sLine+pos+1, strlen(sLine)-pos-1);
					m_Engines[m_nNumEngines-1].m_sLBName[strlen(sLine)-pos-1] = 0;
				} else {
					m_Engines[m_nNumEngines-1].m_sParameterNames[m_Engines[m_nNumEngines-1].m_nNumParameters] = new char[pos+1];
					m_Engines[m_nNumEngines-1].m_sParameterValues[m_Engines[m_nNumEngines-1].m_nNumParameters] = new char[strlen(sLine)-pos];
					strncpy(m_Engines[m_nNumEngines-1].m_sParameterNames[m_Engines[m_nNumEngines-1].m_nNumParameters], sLine, pos);
					m_Engines[m_nNumEngines-1].m_sParameterNames[m_Engines[m_nNumEngines-1].m_nNumParameters][pos] = 0;
					strncpy(m_Engines[m_nNumEngines-1].m_sParameterValues[m_Engines[m_nNumEngines-1].m_nNumParameters], sLine+pos+1, strlen(sLine)-pos-1);
					m_Engines[m_nNumEngines-1].m_sParameterValues[m_Engines[m_nNumEngines-1].m_nNumParameters][strlen(sLine)-pos-1] = 0;
					m_Engines[m_nNumEngines-1].m_nNumParameters++;
				}
			}
		}

	}
	fclose(f);

	/*
	m_nNumEngines = 0;
	int c=0, i;
	char sLine[nMaxLen];
	while (c != EOF) {
		i=0;
		sEnginePath[0] = 0;
		while ((c=fgetc(f))!=EOF && c!='\n' && c!='\r' && i<nMaxLen) sEnginePath[i++] = c;
		sEnginePath[i]=0;
		if (sEnginePath[0] == 0) break;	// empty line, ignore rest
		e.m_sPath = new char [strlen(sEnginePath)+1];
		strcpy(e.m_sPath, sEnginePath);
		m_Engines[m_nNumEngines++] = e;
	}
	fclose(f);*/

	//m_nNumGames = 0;

	// Test the engines and load names
	for (int i=0;i<m_nNumEngines;i++) {
		if (!m_Engines[i].Init()) {
			break;
		}
		m_Engines[i].Send(_T("quit"));
	}
	
	::SetCursor(AfxGetApp()->LoadStandardCursor(IDC_ARROW));
}

void CLittleBlitzerDlg::OnBnClickedStart()
{
	if (!InitPGN()) return;

	m_nGameNum = 0;
	m_nNumGamesPlayed = 0;

	m_nNumActiveTournaments = 0;
	for (int i=0;i<NUM_TYPES;i++) {
		m_nResults[i] = new long[m_nNumEngines];
		memset((long *)m_nResults[i], 0, sizeof(long)*m_nNumEngines);
	}
	m_nGames = new long[m_nNumEngines];
	memset((long *)m_nGames, 0, sizeof(long)*m_nNumEngines);
	m_nWins = new long[m_nNumEngines];
	memset((long *)m_nWins, 0, sizeof(long)*m_nNumEngines);
	m_nLosses = new long[m_nNumEngines];
	memset((long *)m_nLosses, 0, sizeof(long)*m_nNumEngines);
	m_nDraws = new long[m_nNumEngines];
	memset((long *)m_nDraws, 0, sizeof(long)*m_nNumEngines);
	m_dTotalTime = new double[m_nNumEngines];
	memset((double *)m_dTotalTime, 0, sizeof(double)*m_nNumEngines);
	m_dTotalSearches = new double[m_nNumEngines];
	memset((double *)m_dTotalSearches, 0, sizeof(double)*m_nNumEngines);
	m_nTotalDepth = new long[m_nNumEngines];
	memset((long *)m_nTotalDepth, 0, sizeof(long)*m_nNumEngines);
	m_nTotalDepthCount = new long[m_nNumEngines];
	memset((long *)m_nTotalDepthCount, 0, sizeof(long)*m_nNumEngines);
	m_nTotalNPS = new long long[m_nNumEngines];
	memset((long long *)m_nTotalNPS, 0, sizeof(long long)*m_nNumEngines);
	m_nTotalNPSCount = new long long[m_nNumEngines];
	memset((long long *)m_nTotalNPSCount, 0, sizeof(long long)*m_nNumEngines);


	m_nTimeTaken.Start();

	// Start one thread per m_Tournament.m_nNumParallel
	for (int x=0; x<MAX_THREADS; x++) {
		m_Tournaments[x].m_nNumEngines = m_nNumEngines;
		for (int e=0; e<m_nNumEngines; e++) {
			m_Tournaments[x].m_Engines[e] = m_Engines[e];
		}
		m_Tournaments[x].m_pWnd = this;
		m_Tournaments[x].m_nThreadID = x;
		m_Tournaments[x].m_nRound = 0;
		/*m_Tournaments[x].m_sStartPositions = new char*[MAX_START_POSITIONS];
		for (int w=0;w<m_Tournaments[x].m_nNumStartPositions;w++) {
			m_Tournaments[x].m_sStartPositions[w] = new char[strlen(m_Tournaments[0].m_sStartPositions[w])+1];
			strcpy(m_Tournaments[x].m_sStartPositions[w], m_Tournaments[0].m_sStartPositions[w]);
		}*/

		if (x < m_nNumTournaments) {
			m_nNumActiveTournaments++;
			m_Tournaments[x].m_nRound = GetNextRound();
			CWinThread *pThread = AfxBeginThread(RunTournament, &m_Tournaments[x]);
		}
	}
	m_nNumActiveTournaments = m_nNumTournaments;

	m_wndStart.EnableWindow(false);
	m_wndLoadEngines.EnableWindow(false);
	m_wndLoadTournament.EnableWindow(false);

	UpdateResults();
	UpdateNumTourneys();
}

int CLittleBlitzerDlg::GetNextRound() {
	Lock(&m_nLockGameNum);
	m_nGameNum++;
	Unlock(&m_nLockGameNum);
	return m_nGameNum-1;
}

UINT CLittleBlitzerDlg::RunTournament(void *pParam) {
	CTournament *t = (CTournament*)pParam;
	t->Start();
	return 0;
}

LRESULT CLittleBlitzerDlg::OnGameDone(WPARAM wParam, LPARAM lParam) {
	
	m_nNumGamesPlayed++;

	TResult *tResult = (TResult*)wParam;

	ASSERT(tResult->nWhite >= 0 && tResult->nWhite < m_nNumEngines);
	ASSERT(tResult->nBlack >= 0 && tResult->nBlack < m_nNumEngines);
	ASSERT(tResult->nWhite != tResult->nBlack);

	m_nGames[tResult->nWhite] ++;
	m_nGames[tResult->nBlack] ++;
	if (tResult->nResult == BLACK_MATES) {
		m_nWins[tResult->nBlack] ++;
		m_nLosses[tResult->nWhite] ++;
		m_nResults[tResult->nResult][tResult->nBlack] ++;
	} else if (tResult->nResult == WHITE_MATES) {
		m_nWins[tResult->nWhite] ++;
		m_nLosses[tResult->nBlack] ++;
		m_nResults[tResult->nResult][tResult->nWhite] ++;
	} else if (tResult->nResult == BLACK_TIMEOUT) {
		m_nWins[tResult->nWhite] ++;
		m_nLosses[tResult->nBlack] ++;
		m_nResults[tResult->nResult][tResult->nBlack] ++;
	} else if (tResult->nResult == WHITE_TIMEOUT) {
		m_nWins[tResult->nBlack] ++;
		m_nLosses[tResult->nWhite] ++;
		m_nResults[tResult->nResult][tResult->nWhite] ++;
	} else if (tResult->nResult == STALEMATE) {
		m_nDraws[tResult->nWhite] ++;
		m_nDraws[tResult->nBlack] ++;
		m_nResults[tResult->nResult][tResult->nWhite] ++;
		m_nResults[tResult->nResult][tResult->nBlack] ++;
	} else if (tResult->nResult == INSUF_MAT) {
		m_nDraws[tResult->nWhite] ++;
		m_nDraws[tResult->nBlack] ++;
		m_nResults[tResult->nResult][tResult->nWhite] ++;
		m_nResults[tResult->nResult][tResult->nBlack] ++;
	} else if (tResult->nResult == REPETITION) {
		m_nDraws[tResult->nWhite] ++;
		m_nDraws[tResult->nBlack] ++;
		m_nResults[tResult->nResult][tResult->nWhite] ++;
		m_nResults[tResult->nResult][tResult->nBlack] ++;
	} else if (tResult->nResult == FIFTY_MOVE) {
		m_nDraws[tResult->nWhite] ++;
		m_nDraws[tResult->nBlack] ++;
		m_nResults[tResult->nResult][tResult->nWhite] ++;
		m_nResults[tResult->nResult][tResult->nBlack] ++;
	} else if (tResult->nResult == WHITE_ILLEGAL) {
		m_nWins[tResult->nBlack] ++;
		m_nLosses[tResult->nWhite] ++;
		m_nResults[tResult->nResult][tResult->nWhite] ++;
	} else if (tResult->nResult == BLACK_ILLEGAL) {
		m_nWins[tResult->nWhite] ++;
		m_nLosses[tResult->nBlack] ++;
		m_nResults[tResult->nResult][tResult->nBlack] ++;
	} else if (tResult->nResult == ADJ_DRAW) {
		m_nDraws[tResult->nWhite] ++;
		m_nDraws[tResult->nBlack] ++;
		m_nResults[tResult->nResult][tResult->nWhite] ++;
		m_nResults[tResult->nResult][tResult->nBlack] ++;
	} else if (tResult->nResult == ADJ_WHITE_MATES) {
		m_nWins[tResult->nWhite] ++;
		m_nLosses[tResult->nBlack] ++;
		m_nResults[tResult->nResult][tResult->nBlack] ++;
	} else if (tResult->nResult == ADJ_BLACK_MATES) {
		m_nWins[tResult->nBlack] ++;
		m_nLosses[tResult->nWhite] ++;
		m_nResults[tResult->nResult][tResult->nWhite] ++;
	}
	m_dTotalTime[tResult->nWhite] += tResult->dTotalTime[WHITE];
	m_dTotalTime[tResult->nBlack] += tResult->dTotalTime[BLACK];
	m_dTotalGamesLen += tResult->dTotalTime[WHITE] + tResult->dTotalTime[BLACK];
	m_dTotalSearches[tResult->nWhite] += tResult->dTotalSearches[WHITE];
	m_dTotalSearches[tResult->nBlack] += tResult->dTotalSearches[BLACK];
	m_nTotalDepth[tResult->nWhite] += tResult->nTotalDepth[WHITE];
	m_nTotalDepth[tResult->nBlack] += tResult->nTotalDepth[BLACK];
	m_nTotalDepthCount[tResult->nWhite] += tResult->nTotalDepthCount[WHITE];
	m_nTotalDepthCount[tResult->nBlack] += tResult->nTotalDepthCount[BLACK];
	m_nTotalNPS[tResult->nWhite] += tResult->nTotalNPS[WHITE];
	m_nTotalNPS[tResult->nBlack] += tResult->nTotalNPS[BLACK];
	m_nTotalNPSCount[tResult->nWhite] += tResult->nTotalNPSCount[WHITE];
	m_nTotalNPSCount[tResult->nBlack] += tResult->nTotalNPSCount[BLACK];

	UpdateResults();
	UpdatePGN(tResult);

	// Start next game
	m_nNumActiveTournaments--;
	int id = (int)lParam;
	if (id < m_nNumTournaments && (m_nNumGamesPlayed+m_nNumActiveTournaments) < m_nNumGames) {
		m_nNumActiveTournaments++;
		m_Tournaments[id].m_bRunning = true;
		m_Tournaments[id].m_nRound = GetNextRound();
		CWinThread *pThread = AfxBeginThread(RunTournament, &m_Tournaments[id]);
	}
	UpdateNumTourneys();

	return 0;
}

bool CLittleBlitzerDlg::InitPGN() {

	m_wndSavePGN.GetWindowText(m_sResultsPath);

	// create new empty file
	FILE *fpgn;
	if (fpgn = fopen(m_sResultsPath.GetBuffer(), "rt")) {
		int r = MessageBox("The results file already exists, do you wish to append to the current file?\nYes = Append\nNo = Overwrite","WARNING",MB_YESNOCANCEL);
		if (r == 6) {
			// Append
		} else if (r == 7) {
			// Overwrite
			if (!(fpgn = fopen(m_sResultsPath.GetBuffer(), "wt")))
				return false;
			fclose(fpgn);
		} else if (r == 2) {
			// Cancel
			return false;
		}
	} else {
		// Create empty file
		if (!(fpgn = fopen(m_sResultsPath.GetBuffer(), "wt")))
			return false;
		fclose(fpgn);
	}

	return true;

}

void CLittleBlitzerDlg::UpdatePGN(TResult *r) {

	// Each game needs to be written as

	//	[White "Fischer, R."]
	//	[Black "Spassky, B."]
	//	[Result "1/2-1/2"]
	//
	//	1/2-1/2
	//

	char sResult[20];
	if (r->nResult == BLACK_MATES) {
		strcpy(sResult, "0-1");
	} else if (r->nResult == WHITE_MATES) {
		strcpy(sResult, "1-0");
	} else if (r->nResult == BLACK_TIMEOUT) {
		strcpy(sResult, "1-0");
	} else if (r->nResult == WHITE_TIMEOUT) {
		strcpy(sResult, "0-1");
	} else if (r->nResult == STALEMATE) {
		strcpy(sResult, "1/2-1/2");
	} else if (r->nResult == INSUF_MAT) {
		strcpy(sResult, "1/2-1/2");
	} else if (r->nResult == REPETITION) {
		strcpy(sResult, "1/2-1/2");
	} else if (r->nResult == FIFTY_MOVE) {
		strcpy(sResult, "1/2-1/2");
	} else if (r->nResult == WHITE_ILLEGAL) {
		strcpy(sResult, "0-1");
	} else if (r->nResult == BLACK_ILLEGAL) {
		strcpy(sResult, "1-0");
	} else if (r->nResult == ADJ_DRAW) {
		strcpy(sResult, "1/2-1/2");
	} else if (r->nResult == ADJ_WHITE_MATES) {
		strcpy(sResult, "1-0");
	} else if (r->nResult == ADJ_BLACK_MATES) {
		strcpy(sResult, "0-1");
	}
	FILE *fpgn;

	//if (!(fpgn = _wfopen(m_sResultsPath.GetBuffer(), _T("at"))))
	if (!(fpgn = fopen(m_sResultsPath.GetBuffer(), "at")))
		return; // crap, need some thread control here

	fprintf(fpgn, "[White \"%s\"]\n", m_Engines[r->nWhite].m_sName);
	fprintf(fpgn, "[Black \"%s\"]\n", m_Engines[r->nBlack].m_sName);
	fprintf(fpgn, "[Result \"%s\"]\n", sResult);

	if (g_bFullPGN) {
		// v2.7 - write entire game
		fprintf(fpgn, "[SetUp \"1\"]\n");
		fprintf(fpgn, "[FEN \"%s\"]\n", r->sFEN);
		fprintf(fpgn, "\n%s %s\n\n", r->sSAN, sResult);
		delete r->sSAN;
	} else {
		// Just put the result
		fprintf(fpgn, "\n%s\n\n", sResult);
	}

	fclose(fpgn);

}

void CLittleBlitzerDlg::UpdateResults() {
	double dElapsed = m_nTimeTaken.GetMS();
	double dRemaining = dElapsed / ((double)m_nNumGamesPlayed/(m_nNumGames?m_nNumGames:0.00001)) - dElapsed;
	CString s;
	s.Format("Games Completed = %ld of %ld (Avg game length = %.3lf sec)\r\n", m_nNumGamesPlayed, m_nNumGames, m_dTotalGamesLen/(m_nNumGamesPlayed?m_nNumGamesPlayed:1)/1000.0);

	
	CString sVariant, sBook, sTC, sStart, sAdj;
	sVariant.Format("%s",m_Tournaments[0].m_nVariant==VARIANT_960?"FRC/":"");
	sBook.Format("%s",m_Tournaments[0].m_bOwnBook?"Book/":"");
	if (m_Tournaments[0].m_nTC==TC_FIXED_TPM) {
		sTC.Format("%dms per move",m_Tournaments[0].m_nBase);
	} else if (m_Tournaments[0].m_nTC==TC_BLITZ) {
		sTC.Format("%dms+%dms",m_Tournaments[0].m_nBase,m_Tournaments[0].m_nInc);
	} else if (m_Tournaments[0].m_nTC==TC_TOURNAMENT) {
		sTC.Format("%dms in %d moves",m_Tournaments[0].m_nBase,m_Tournaments[0].m_nInc);
	}
	if (m_nStartPositionType == 1) {
		sStart.Format("FEN:%s",m_Tournaments[0].m_sStartPositions[0]);
	} else if (m_nStartPositionType == 2) {
		sStart.Format("EPD:%s(%d)",m_sStartPositionFile,m_Tournaments[0].m_nNumStartPositions);
	} else if (m_nStartPositionType == 3) { // 2.71
		sStart.Format("PGN:%s(%d)",m_sStartPositionFile,m_Tournaments[0].m_nNumStartPositions);
	}
	sAdj.Format("M %dcp for %d moves, D %d moves",m_Tournaments[0].m_nAdjMateScore,m_Tournaments[0].m_nAdjMateMoves,m_Tournaments[0].m_nAdjDrawMoves);
	s.AppendFormat("Settings = %s/%s%dMB/%s%s/%s/%s\r\n", m_Tournaments[0].m_nType==0?"Gauntlet":"RR",sVariant,m_Tournaments[0].m_nHash,sBook,sTC,sAdj,sStart);

	s.AppendFormat(_T("Time = %0.0lf sec elapsed, %0.0lf sec remaining\r\n"), dElapsed/1000, dRemaining/1000);

	for (int i=0;i<m_nNumEngines;i++) {
		s.AppendFormat(_T("%2d.  %-25s"), i+1, CString(m_Engines[i].m_sName).GetBuffer());
		if (m_nWins) {
			int nWins = m_nWins?m_nWins[i]:0;
			int nDraws = m_nDraws?m_nDraws[i]:0;
			int nLosses = m_nLosses?m_nLosses[i]:0;
			s.AppendFormat(_T("\t%0.1f/%ld\t%d-%d-%d  \t(L: m=%d t=%d i=%d a=%d)\t(D: r=%d i=%d f=%d s=%d a=%d)\t(tpm=%.1lf d=%.2lf nps=%ld)")
				,nWins+nDraws/2.0, m_nGames[i]
				,nWins, nLosses, nDraws
				,nLosses-m_nResults[WHITE_TIMEOUT][i]-m_nResults[BLACK_TIMEOUT][i]-m_nResults[WHITE_ILLEGAL][i]-m_nResults[BLACK_ILLEGAL][i]-m_nResults[ADJ_WHITE_MATES][i]-m_nResults[ADJ_BLACK_MATES][i], m_nResults[WHITE_TIMEOUT][i]+m_nResults[BLACK_TIMEOUT][i], m_nResults[WHITE_ILLEGAL][i]+m_nResults[BLACK_ILLEGAL][i], m_nResults[ADJ_WHITE_MATES][i]+m_nResults[ADJ_BLACK_MATES][i]
				,m_nResults[REPETITION][i], m_nResults[INSUF_MAT][i], m_nResults[FIFTY_MOVE][i], m_nResults[STALEMATE][i], m_nResults[ADJ_DRAW][i]
				,m_dTotalTime[i]/(m_dTotalSearches[i]==0?1:m_dTotalSearches[i]), (double)m_nTotalDepth[i]/(m_nTotalDepthCount[i]==0?1:m_nTotalDepthCount[i]), m_nTotalNPS[i]/(m_nTotalNPSCount[i]==0?1:m_nTotalNPSCount[i]) );
		}
		s.Append(_T("\r\n"));
	}

	m_wndResults.SetSel(0,-1);
	m_wndResults.ReplaceSel(s);

}

void CLittleBlitzerDlg::OnBnClickedPause()
{
	if (m_nNumEngines == 0) return;

	// Sets num threads to 0 and on resume will set it back to what it used to be

	static int old;
	if (m_bPaused) {
		m_nNumTournaments = old;
		m_wndPause.SetWindowText("Pause\n(zero threads)");
		for (int id=0;id<m_nNumTournaments;id++) {
			if (!m_Tournaments[id].m_bRunning) {
				m_Tournaments[id].m_nRound = GetNextRound();
				CWinThread *pThread = AfxBeginThread(RunTournament, &m_Tournaments[id]);
			}
			m_nNumActiveTournaments = m_nNumTournaments;
		}
	} else {
		old = m_nNumTournaments;
		m_nNumTournaments = 0;
		CString s;
		s.Format("Resume\n(%d threads)",old);
		m_wndPause.SetWindowText(s);
	}
	UpdateNumTourneys();

	m_bPaused = !m_bPaused;

}

void CLittleBlitzerDlg::OnBnClickedOpenEngines()
{
	char szFilters[] = "Engine Files (*.lbe)|*.lbe|All Files (*.*)|*.*||";
	CFileDialog fileDlg (TRUE, _T("lbe"), _T("*.lbe"), OFN_FILEMUSTEXIST, (LPCTSTR)szFilters, this);
	CString sPathName;

	if (fileDlg.DoModal() == IDOK) {
		sPathName = fileDlg.GetPathName();
		m_wndEngineFile.SetWindowText(sPathName);
	}
}

void CLittleBlitzerDlg::OnBnClickedLoadEngines()
{
	LoadEngineSettings();
	UpdateResults();
}

void CLittleBlitzerDlg::OnBnClickedOpenTourn()
{
	char szFilters[] = "Tournament Files (*.lbt)|*.lbt|All Files (*.*)|*.*||";
	CFileDialog fileDlg (TRUE, _T("lbt"), _T("*.lbt"), OFN_FILEMUSTEXIST | OFN_HIDEREADONLY, (LPCTSTR)szFilters, this);
	CString sPathName;

	if (fileDlg.DoModal() == IDOK) {
		sPathName = fileDlg.GetPathName();
		m_wndTournFile.SetWindowText(sPathName);
	}

	UpdateResults();
}

void CLittleBlitzerDlg::OnBnClickedCopytext()
{
	m_wndResults.SetSel(0,-1);
	m_wndResults.Copy();
	m_wndResults.SetSel(0,0);
}

void CLittleBlitzerDlg::OnBnClickedInc()
{
	if (m_nNumTournaments >= MAX_THREADS) return;

	m_nNumTournaments++;

	if (m_Tournaments[m_nNumTournaments-1].m_nNumEngines>0 && !m_Tournaments[m_nNumTournaments-1].m_bRunning) {
		m_nNumActiveTournaments++;
		m_Tournaments[m_nNumTournaments-1].m_nRound = GetNextRound();
		CWinThread *pThread = AfxBeginThread(RunTournament, &m_Tournaments[m_nNumTournaments-1]);
	}
	
	// Take out of paused state
	if (m_bPaused) {
		m_bPaused = false;
		m_wndPause.SetWindowText("Pause\n(zero threads)");
	}

	UpdateNumTourneys();

}

void CLittleBlitzerDlg::OnBnClickedDec()
{
	if (m_nNumTournaments <= 0) return;

	m_nNumTournaments--;
	UpdateNumTourneys();
}

void CLittleBlitzerDlg::UpdateNumTourneys() {
	CString s;
	s.Format(_T("Requested: %d\nActual: %d"),m_nNumTournaments,m_nNumActiveTournaments);
	m_wndNumThreads.SetWindowText(s.GetBuffer());
	m_wndFullPGN.EnableWindow(m_nNumActiveTournaments == 0); // can change this between games only
	//CFont f;
	//f.CreateFont(8,8,0,0,FW_BOLD,FALSE,FALSE,0,DEFAULT_CHARSET,  OUT_CHARACTER_PRECIS,CLIP_CHARACTER_PRECIS,DEFAULT_QUALITY, DEFAULT_PITCH,NULL);
	//m_wndNumThreads.SetFont(&f);
}

void CLittleBlitzerDlg::OnBnClickedChkLog()
{
	g_bLogging = m_wndChkLog.GetCheck();
}

void CLittleBlitzerDlg::OnBnClickedChkIllegal()
{
	g_bDumpIllegalMoves = m_wndChkLog.GetCheck();
}

void CLittleBlitzerDlg::OnBnClickedChkFullPGN()
{
	g_bFullPGN = m_wndFullPGN.GetCheck();
}
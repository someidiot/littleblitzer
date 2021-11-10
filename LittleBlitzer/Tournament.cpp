#include "StdAfx.h"
#include "Tournament.h"
#include "Common.h"
#include "Timer.h"
#include "Board.h"
#include "Move.h"
#include "MoveGen.h"
#include "Engine.h"

CTournament::CTournament(void)
{
	m_nType = 1;
	m_nTC = 1;
	m_nBase = 1000;
	m_nInc = 100;
	m_nRound = 0;
	m_nHash = 32;
	m_bPonder = false;
	m_bOwnBook = true;
	m_nVariant = 0;
	m_nNumStartPositions = 0;
	m_nNumEngines = 0;
	m_nWastedTime = 0;
	m_nAdjMateScore = 1000;
	m_nAdjMateMoves = 12;
	m_nAdjDrawMoves = 150;
	m_nRandomize = 0;

	m_pWnd = NULL;

	m_bRunning = false;

	//m_bStopRequested = false;
	//m_bPaused = false;
}

CTournament::~CTournament(void)
{
}

void CTournament::Start() {
// Start the tournament

	m_bRunning = true; // to avoid starting an already running tourney
	if (m_nNumEngines == 0) return;

	srand(clock());

	long nBase[2], nInc[2];
	double fTimeLeft[2];
	int nSTM = WHITE;
	CTimer t;
	CString sMove;
	CString sMoveList;
	int nMoveNum;

	long nStartTime = clock();

	TResult tResult;
	tResult.dTotalTime[WHITE]		= tResult.dTotalTime[BLACK]			= 0.0;
	tResult.dTotalSearches[WHITE]	= tResult.dTotalSearches[BLACK]		= 0.0;
	tResult.nTotalDepth[WHITE]		= tResult.nTotalDepth[BLACK]		= 0;
	tResult.nTotalDepthCount[WHITE] = tResult.nTotalDepthCount[BLACK]	= 0;
	tResult.nTotalNPS[WHITE]		= tResult.nTotalNPS[BLACK]			= 0;
	tResult.nTotalNPSCount[WHITE]	= tResult.nTotalNPSCount[BLACK]		= 0;

	long *nPrevScores[2];				// Circular array of previous scores for each side (used in adjudication detection)
	nPrevScores[WHITE] = new long[m_nAdjMateMoves];
	nPrevScores[BLACK] = new long[m_nAdjMateMoves];
	long nPrevScoresHead[2] = {0,0};	// index into nPrevScores[] of most recent score

	TBoard b;
	TUndoMove u;
	TMove m;
	int nWhite, nBlack;

	// m_nRound is set by calling class

	// Calculate next game opponents
	if (m_nType == 0) {
		// Gauntlet

		int nOpponent = 1+m_nRound%(m_nNumEngines-1);
		// 0,1,2,3 = white
		// 4,5,6,7 = black
		// 8,9,10,11 = white
		int nColour = (m_nRound/(m_nNumEngines-1)) & 1;
		nWhite = nColour==WHITE?0:nOpponent;
		nBlack = nColour==BLACK?0:nOpponent;

		// Pick a starting position
		if (m_nVariant == VARIANT_STD) {
			int idx;
			if (m_nRandomize) idx = rand() % m_nNumStartPositions;
			else idx = (m_nRound/(2*(m_nNumEngines-1))) % m_nNumStartPositions;
			LoadFEN(&b, m_sStartPositions[idx]);
		} else {
			CreateStartingPosition(&b, m_nVariant);
		}

	} else if (m_nType == 1) {
		// round robin (sort of)

		nWhite = (m_nRound / (m_nNumEngines-1)) % m_nNumEngines;
		int m = m_nRound % (m_nNumEngines-1);
		nBlack = (m>=nWhite)?1+m:m;

		// Pick a starting position
		if (m_nVariant == VARIANT_STD) {
			int idx;
			if (m_nRandomize) idx = rand() % m_nNumStartPositions;
			else idx = m_nRound % m_nNumStartPositions;
			LoadFEN(&b, m_sStartPositions[idx]);
			//LoadFEN(&b, "r4rk1/pp1nbppp/1qp1pn2/5b2/3P4/1Q3NP1/PP1NPPBP/R1B2RK1 w - - 0 0");
		} else {
			CreateStartingPosition(&b, m_nVariant);
		}
	}

	ASSERT(nWhite >= 0 && nWhite < m_nNumEngines);
	ASSERT(nBlack >= 0 && nBlack < m_nNumEngines);
	m_CurrEngines[WHITE] = m_Engines[nWhite];
	m_CurrEngines[BLACK] = m_Engines[nBlack];
	tResult.nWhite = nWhite;
	tResult.nBlack = nBlack;

	m_CurrEngines[WHITE].m_nHash = m_nHash;
	m_CurrEngines[WHITE].m_bPonder = m_bPonder;
	m_CurrEngines[WHITE].m_bOwnBook = m_bOwnBook;
	m_CurrEngines[WHITE].m_nVariant = m_nVariant;
	m_CurrEngines[WHITE].Init();
	m_CurrEngines[BLACK].m_nHash = m_nHash;
	m_CurrEngines[BLACK].m_bPonder = m_bPonder;
	m_CurrEngines[BLACK].m_bOwnBook = m_bOwnBook;
	m_CurrEngines[BLACK].m_nVariant = m_nVariant;
	m_CurrEngines[BLACK].Init();

	m_CurrEngines[WHITE].NewGame();
	m_CurrEngines[BLACK].NewGame();

	nBase[WHITE] = m_nBase;
	nBase[BLACK] = m_nBase;
	nInc[WHITE] = m_nInc;
	nInc[BLACK] = m_nInc;

	fTimeLeft[WHITE] = nBase[WHITE];
	fTimeLeft[BLACK] = nBase[BLACK];
	nMoveNum = 1;

	char sStartingPositionFEN[100];
	Board2FEN(&b, sStartingPositionFEN);
	if (g_bFullPGN) {
		strcpy(tResult.sFEN, sStartingPositionFEN);
	}
	g_nGameHalfMoveNum[m_nThreadID] = 0;
	nSTM = b.nSideToMove;

	//CString sGameMoves;
	char *sGameMoves;
	int nGameMovesLen = 0;
	int nGameMovesAlloc = 1024;
	sGameMoves = (char *)malloc(sizeof(char)*nGameMovesAlloc);
	sGameMoves[0] = 0; // v2.72 initialise in case no moves made
	//sGameMoves.Preallocate(1024);
	

	m_nWastedTime += clock()-nStartTime;

	while (true) { // Game loop

		long nDepth=0;
		long nNPS=0;
		long nScore=0;

		if (!m_bRunning) break;	// abort

		// Search

		//t.Start();
		CString sLine = m_CurrEngines[nSTM].Search(CString(sStartingPositionFEN), sMoveList, m_nTC, (long)fTimeLeft[WHITE], (long)fTimeLeft[BLACK], nInc[WHITE], nInc[BLACK], (long)fTimeLeft[nSTM], &nDepth, &nNPS, &nScore, &t);
		//t.Stop();

		CStringArray sWords;
		m_CurrEngines[nSTM].GetWords(sLine, &sWords);
		if (sWords.GetCount() > 1)
			sMove = sWords[1];
		//else
			//sMove = NULL;

		// Update clock
			
		if (m_nTC != TC_FIXED_TPM) { // not fixed tpm
			fTimeLeft[nSTM] -= t.GetMS();
		}
		tResult.dTotalTime[nSTM] += t.GetMS();
		tResult.dTotalSearches[nSTM] ++;
		if (nDepth > 0) {
			tResult.nTotalDepth[nSTM] += nDepth;
			tResult.nTotalDepthCount[nSTM] ++;
		}
		if (nNPS > 0) {
			tResult.nTotalNPS[nSTM] += nNPS;
			tResult.nTotalNPSCount[nSTM] ++;
		}

		Log("Took %.1lfms, Left[%c] = %.1lfms", t.GetMS(), nSTM==WHITE?'W':'B', fTimeLeft[nSTM]);

		if (fTimeLeft[nSTM] <= 0) {
			//TRACE("\n%s timeout\n", nSTM==0?"white":"black");
			Log("RESULT: TIMEOUT");
			tResult.nResult = nSTM==WHITE?WHITE_TIMEOUT:BLACK_TIMEOUT;
			break;
		}

		if (m_nTC == TC_BLITZ) {
			// Add increment
			fTimeLeft[nSTM] += nInc[nSTM];
			if (nInc[nSTM] > 0)	Log("Add inc %ldms, Left[%c] = %.1lfms", nInc[nSTM], nSTM==WHITE?'W':'B', fTimeLeft[nSTM]);
		} else if (m_nTC == TC_TOURNAMENT && nSTM == BLACK) {
			// Dec moves to go
			nInc[WHITE]--;
			if (nInc[WHITE] == 0) {
				nInc[WHITE] = m_nInc;
				fTimeLeft[WHITE] = nBase[WHITE];
				fTimeLeft[BLACK] = nBase[BLACK];
			}
		}


		// Check move
		if (!Move2Coord(&m, &b, sMove.GetBuffer(), m_nVariant)) {
			ASSERT(false);
		}
		if (!IsValidMoveQuick(m, &b, m_nVariant)) {	// Illegal move
			Log("RESULT: ILLEGAL MOVE");
			tResult.nResult = nSTM==WHITE?WHITE_ILLEGAL:BLACK_ILLEGAL;
			if (g_bDumpIllegalMoves)
				DumpIllegalMove(&b, m, sStartingPositionFEN, sMoveList, &m_CurrEngines[nSTM]);
			break;
		}

		// Update game moves for pgn (v2.7)
		if (g_bFullPGN) {
			char *sSAN = GetNotation(&b, m);
			if (nGameMovesLen+strlen(sSAN)+7 >= nGameMovesAlloc) {
				nGameMovesAlloc *= 2;
				sGameMoves = (char*)realloc(sGameMoves, sizeof(char)*nGameMovesAlloc);
			}
			sGameMoves[nGameMovesLen++] = ' ';
			if (nSTM == WHITE) {
				char sMoveNum[10];
				sprintf(sMoveNum, "%d. ", nMoveNum);
				strcpy(sGameMoves+nGameMovesLen, sMoveNum);
				nGameMovesLen+=strlen(sMoveNum);
				strcpy(sGameMoves+nGameMovesLen, sSAN);
				nGameMovesLen+=strlen(sSAN);
			} else {
				if (nGameMovesLen == 1) {
					char sMoveNum[10];
					sprintf(sMoveNum, "%d.. ", nMoveNum);
					strcpy(sGameMoves+nGameMovesLen, sMoveNum);
					nGameMovesLen+=strlen(sMoveNum);
				}
				strcpy(sGameMoves+nGameMovesLen, sSAN);
				nGameMovesLen+=strlen(sSAN);
			}
			sGameMoves[nGameMovesLen] = 0;
			delete sSAN;
		}

		// Check Adjudication
		if (g_nGameHalfMoveNum[m_nThreadID] >= m_nAdjDrawMoves*2) {
			Log("RESULT: DRAW BY ADJUDICATION (%d moves)", g_nGameHalfMoveNum[m_nThreadID]/2);
			tResult.nResult = ADJ_DRAW;
			break;
		}
		nPrevScores[nSTM][nPrevScoresHead[nSTM]++] = nScore;
		if (nPrevScoresHead[nSTM] == m_nAdjMateMoves)
			nPrevScoresHead[nSTM] = 0;
		if (nScore > m_nAdjMateScore) {
			// potential adjudication mate
			int i;
			bool bOK=true;
			for (i=0;i<m_nAdjMateMoves;i++) {
				if (nPrevScores[nSTM][i] < m_nAdjMateScore) bOK=false;
				if (nPrevScores[OPP(nSTM)][i] > -m_nAdjMateScore) bOK=false;
			}
			if (bOK) {
				Log("RESULT: MATE BY ADJUDICATION");
				tResult.nResult = nSTM==WHITE?ADJ_WHITE_MATES:ADJ_BLACK_MATES;
				break;
			}
		} else if (nScore < -m_nAdjMateScore) {
			// potential adjudication loss
			int i;
			bool bOK=true;
			for (i=0;i<m_nAdjMateMoves;i++) {
				if (nPrevScores[nSTM][i] > -m_nAdjMateScore) bOK=false;
				if (nPrevScores[OPP(nSTM)][i] < m_nAdjMateScore) bOK=false;
			}
			if (bOK) {
				Log("RESULT: MATE BY ADJUDICATION");
				//tResult.nResult = nSTM==WHITE?ADJ_WHITE_MATES:ADJ_BLACK_MATES;
				tResult.nResult = nSTM==BLACK?ADJ_WHITE_MATES:ADJ_BLACK_MATES;	// v2.71 - fix adjudication result
				break;
			}
		}

		// Update movelist

		//sMoveList.AppendFormat(_T(" %s"), sMove);
		sMoveList.AppendFormat(_T(" %s"), sMove.MakeLower()); // v2.73 - lower case move list to handle case sensitive engines
		if (nSTM == WHITE) {
			//TRACE("%d. %s", nMoveNum, sMove);
		} else {
			//TRACE("  %s\n", sMove);
			nMoveNum++;
		}

		// Update board

		MakeMove2(&b, m, &u, 0, 0, m_nVariant);

		// v2.7
		if (g_bFullPGN && (b.nSideToMove==WHITE && IsSquareAttackedBy(&b,GetBit(b.bbPieces[WHITE][PIECE_KING-1]),BLACK)) 
			|| (b.nSideToMove==BLACK && IsSquareAttackedBy(&b,GetBit(b.bbPieces[BLACK][PIECE_KING-1]),WHITE))) {

			if (nGameMovesLen+1 >= nGameMovesAlloc) {
				nGameMovesAlloc *= 2;
				sGameMoves = (char*)realloc(sGameMoves, sizeof(char)*nGameMovesAlloc);
			}
			sGameMoves[nGameMovesLen++] = '+';
			sGameMoves[nGameMovesLen] = 0;

		}

		// Check for end of game

		if (IsInsufficientMaterial(&b)) {
			Log("RESULT: DRAW BY INSUFFICIENT MATERIAL");
			tResult.nResult = INSUF_MAT;
			break;
		} else if (b.nFiftyMoveCount > 100) {
			Log("RESULT: DRAW BY FIFTY MOVES");
			tResult.nResult = FIFTY_MOVE;
			break;
		} else if (IsRepetition(&b, m_nThreadID)) {
			Log("RESULT: DRAW BY REPETITION");
			tResult.nResult = REPETITION;
			break;
		} else if (!IsAnyLegalMoves(&b, m_nVariant)) {
			if (IsSTMInCheck(&b)) {
				Log("RESULT: MATE");
				tResult.nResult = nSTM==WHITE?WHITE_MATES:BLACK_MATES;
				break;
			} else {
				Log("RESULT: DRAW BY STALEMATE");
				tResult.nResult = STALEMATE;
				break;
			}
		}

		// Change sides

		nSTM ^= 1;
		g_nGameHalfMoveNum[m_nThreadID]++;

	} // Game loop

	nStartTime = clock();

	//m_nRound++;

	//Sleep(500);
	tResult.sSAN = sGameMoves; // memory free'd by CLittleBlitzerDlg::OnGameDone()

	m_CurrEngines[WHITE].Quit();
	m_CurrEngines[BLACK].Quit();

	delete nPrevScores[WHITE];
	delete nPrevScores[BLACK];

	m_nWastedTime += clock()-nStartTime;

	m_bRunning = false;

	// Send result and start next game

	if (m_pWnd && m_pWnd->m_hWnd)
		m_pWnd->SendMessage(GAME_DONE, (WPARAM)&tResult, (LPARAM)m_nThreadID);

}

void CTournament::DumpIllegalMove(TBoard *b, TMove m, char *sStartingPosition, CString sMoveList, CEngine *e) {
	FILE *fout;
	char name[100];
	srand(clock());	// ok to do here cause the global hash values are already defined
	sprintf(name, "illegal_%s_%u", e->m_sName, rand());
	fout = fopen(name, "wt");
	char fen[100];
	Board2FEN(b, fen);
	//fprintf(fout,"Board: %s\n",fen);
	PrintBoard(b, fout);
	fprintf(fout, "Starting Position: %s\n", sStartingPosition);
	fprintf(fout, "Movelist: %s\n", sMoveList.GetBuffer());
	fprintf(fout,"Engine Output: %s\n", e->m_sLastOutput);

	fprintf(fout, "Test engine output with:\nposition fen %s moves%s\n", sStartingPosition, sMoveList.GetBuffer());

	fclose(fout);
}


void CTournament::Abort() {
	m_bRunning = false;

	m_CurrEngines[WHITE].Quit();
	m_CurrEngines[BLACK].Quit();
}

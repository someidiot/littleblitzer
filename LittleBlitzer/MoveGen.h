#pragma once

#include "Board.h"
#include "Move.h"
//#include "LittleThought.h"

//int GenerateAllMoves(TBoard *b, TMove *a_tMoveList);
int GenerateCaptureMoves(TBoard *b, TMove *a_tMoveList);
int GenerateNonCaptureMoves(TBoard *b, TMove *a_tMoveList, int nVariant);

int GenerateWhitePawnCaps(TBoard *b, TMove *a_tMoveList);
int GenerateWhiteKnightCaps(TBoard *b, TMove *a_tMoveList);
int GenerateWhiteKingCaps(TBoard *b, TMove *a_tMoveList);
int GenerateWhiteRookQueenCaps(TBoard *b, TMove *a_tMoveList);
int GenerateWhiteBishopQueenCaps(TBoard *b, TMove *a_tMoveList);
int GenerateWhitePawnNonCaps(TBoard *b, TMove *a_tMoveList);
int GenerateWhiteKnightNonCaps(TBoard *b, TMove *a_tMoveList);
int GenerateWhiteKingNonCaps(TBoard *b, TMove *a_tMoveList, int nVariant);
int GenerateWhiteRookQueenNonCaps(TBoard *b, TMove *a_tMoveList);
int GenerateWhiteBishopQueenNonCaps(TBoard *b, TMove *a_tMoveList);

int GenerateBlackPawnCaps(TBoard *b, TMove *a_tMoveList);
int GenerateBlackKnightCaps(TBoard *b, TMove *a_tMoveList);
int GenerateBlackKingCaps(TBoard *b, TMove *a_tMoveList);
int GenerateBlackRookQueenCaps(TBoard *b, TMove *a_tMoveList);
int GenerateBlackBishopQueenCaps(TBoard *b, TMove *a_tMoveList);
int GenerateBlackPawnNonCaps(TBoard *b, TMove *a_tMoveList);
int GenerateBlackKnightNonCaps(TBoard *b, TMove *a_tMoveList);
int GenerateBlackKingNonCaps(TBoard *b, TMove *a_tMoveList, int nVariant);
int GenerateBlackRookQueenNonCaps(TBoard *b, TMove *a_tMoveList);
int GenerateBlackBishopQueenNonCaps(TBoard *b, TMove *a_tMoveList);

bool IsAnyLegalMoves(TBoard *b, int nVariant);

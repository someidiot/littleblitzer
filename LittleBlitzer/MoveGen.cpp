#include "stdafx.h"
#include "MoveGen.h"
#include "Board.h"
#include "Move.h"
//#include "Utils.h"
//#include "Eval.h"

//int GenerateAllMoves(TBoard *b, TMove *a_tMoveList) {
//	int nNumMoves = GenerateCaptureMoves(b,a_tMoveList);
//	nNumMoves += GenerateNonCaptureMoves(b,a_tMoveList+nNumMoves);
//
//	return nNumMoves;
//}

int GenerateCaptureMoves(TBoard *b, TMove *a_tMoveList) {
// Generates a list of possible capture moves and appends them to the supplied move list. This does not
// check for illegal moves. Returns the number of moves found.
	int nNumMoves = 0;

	if (b->nSideToMove == WHITE) {
		nNumMoves += GenerateWhitePawnCaps(b, a_tMoveList);
		nNumMoves += GenerateWhiteKnightCaps(b, a_tMoveList + nNumMoves);
		nNumMoves += GenerateWhiteBishopQueenCaps(b, a_tMoveList + nNumMoves);
		nNumMoves += GenerateWhiteRookQueenCaps(b, a_tMoveList + nNumMoves);
		nNumMoves += GenerateWhiteKingCaps(b, a_tMoveList + nNumMoves);
	} else {
		nNumMoves += GenerateBlackPawnCaps(b, a_tMoveList);
		nNumMoves += GenerateBlackKnightCaps(b, a_tMoveList + nNumMoves);
		nNumMoves += GenerateBlackBishopQueenCaps(b, a_tMoveList + nNumMoves);
		nNumMoves += GenerateBlackRookQueenCaps(b, a_tMoveList + nNumMoves);
		nNumMoves += GenerateBlackKingCaps(b, a_tMoveList + nNumMoves);
	}

	return nNumMoves;
}

int GenerateNonCaptureMoves(TBoard *b, TMove *a_tMoveList, int nVariant) {
// Generates a list of possible non capture moves and appends them to the supplied move list. This does not
// check for illegal moves. Returns the number of moves found.
	int nNumMoves = 0;

	if (b->nSideToMove == WHITE) {
		nNumMoves += GenerateWhitePawnNonCaps(b, a_tMoveList);
		nNumMoves += GenerateWhiteKnightNonCaps(b, a_tMoveList + nNumMoves);
		nNumMoves += GenerateWhiteBishopQueenNonCaps(b, a_tMoveList + nNumMoves);
		nNumMoves += GenerateWhiteRookQueenNonCaps(b, a_tMoveList + nNumMoves);
		nNumMoves += GenerateWhiteKingNonCaps(b, a_tMoveList + nNumMoves, nVariant);
	} else {
		nNumMoves += GenerateBlackPawnNonCaps(b, a_tMoveList);
		nNumMoves += GenerateBlackKnightNonCaps(b, a_tMoveList + nNumMoves);
		nNumMoves += GenerateBlackBishopQueenNonCaps(b, a_tMoveList + nNumMoves);
		nNumMoves += GenerateBlackRookQueenNonCaps(b, a_tMoveList + nNumMoves);
		nNumMoves += GenerateBlackKingNonCaps(b, a_tMoveList + nNumMoves, nVariant);
	}

	return nNumMoves;
}


int GenerateWhitePawnCaps(TBoard *b, TMove *a_tMoveList) {
/* 
Generates a list of all capturing or promoting moves that can be made by white's pawns.
IN:		a_tMoveList		The move array to append to.
OUT:	int				Returns the number of moves found.

TODO: can speed up the >= <= with array lookup
TODO: can precalculate w|b for all move generators
*/
	BitBoard bbAttacks;
	int to, from;
	int nNumMoves=0;

	// Attack left
	bbAttacks = ((b->bbPieces[WHITE][PIECE_PAWN-1] & g_bbMaskNotFileA) << 9) & b->bbPieces[BLACK][PIECE_ALL-1];

	// Loop through each attacked square
	while (bbAttacks) {
		to = RemoveBit(bbAttacks);

		if (to >= H8) {
			// Pawn has promoted
			SET_MOVE(a_tMoveList[0], to-9, to, MOVE_TYPE_PROM, PIECE_QUEEN,  PIECE_PAWN, b->nPieces[to]&SQUARE_PIECE_MASK);
			SET_MOVE(a_tMoveList[1], to-9, to, MOVE_TYPE_PROM, PIECE_ROOK,   PIECE_PAWN, b->nPieces[to]&SQUARE_PIECE_MASK);
			SET_MOVE(a_tMoveList[2], to-9, to, MOVE_TYPE_PROM, PIECE_BISHOP, PIECE_PAWN, b->nPieces[to]&SQUARE_PIECE_MASK);
			SET_MOVE(a_tMoveList[3], to-9, to, MOVE_TYPE_PROM, PIECE_KNIGHT, PIECE_PAWN, b->nPieces[to]&SQUARE_PIECE_MASK);
			a_tMoveList += 4;
			nNumMoves += 4;
		} else {
			// Add the move to the array
			SET_MOVE(a_tMoveList[0], to-9, to, MOVE_TYPE_NORMAL, PIECE_NONE, PIECE_PAWN, b->nPieces[to]&SQUARE_PIECE_MASK);
			a_tMoveList ++;
			nNumMoves++;
		}
	}

	// Attack right
	bbAttacks = ((b->bbPieces[WHITE][PIECE_PAWN-1] & g_bbMaskNotFileH) << 7) & b->bbPieces[BLACK][PIECE_ALL-1];

	// Loop through each attacked square
	while (bbAttacks) {
		to = RemoveBit(bbAttacks);

		if (to >= H8) {
			SET_MOVE(a_tMoveList[0], to-7, to, MOVE_TYPE_PROM, PIECE_QUEEN,  PIECE_PAWN, b->nPieces[to]&SQUARE_PIECE_MASK);
			SET_MOVE(a_tMoveList[1], to-7, to, MOVE_TYPE_PROM, PIECE_ROOK,   PIECE_PAWN, b->nPieces[to]&SQUARE_PIECE_MASK);
			SET_MOVE(a_tMoveList[2], to-7, to, MOVE_TYPE_PROM, PIECE_BISHOP, PIECE_PAWN, b->nPieces[to]&SQUARE_PIECE_MASK);
			SET_MOVE(a_tMoveList[3], to-7, to, MOVE_TYPE_PROM, PIECE_KNIGHT, PIECE_PAWN, b->nPieces[to]&SQUARE_PIECE_MASK);
			a_tMoveList += 4;
			nNumMoves += 4;
		} else {
			// Add the move to the array
			SET_MOVE(a_tMoveList[0], to-7, to, MOVE_TYPE_NORMAL, PIECE_NONE, PIECE_PAWN, b->nPieces[to]&SQUARE_PIECE_MASK);
			a_tMoveList ++;
			nNumMoves++;
		}
	}

	// EnPassent captures
	if (b->nEPSquare != NULL_SQUARE) {
		bbAttacks = g_nnWhiteEPAttacks[b->nEPSquare] & b->bbPieces[WHITE][PIECE_PAWN-1];
		while (bbAttacks) {
			from = RemoveBit(bbAttacks);

			// Add the move to the array
			SET_MOVE(a_tMoveList[0], from, b->nEPSquare, MOVE_TYPE_EP, PIECE_NONE, PIECE_PAWN, PIECE_PAWN);
			a_tMoveList ++;
			nNumMoves++;
		}
	}

	// Non-capture Promotions
	bbAttacks = (b->bbPieces[WHITE][PIECE_PAWN-1] << 8) & g_bbMaskRank8 & (~(b->bbPieces[WHITE][PIECE_ALL-1] | b->bbPieces[BLACK][PIECE_ALL-1]));
	while (bbAttacks) {
		to = RemoveBit(bbAttacks);

		SET_MOVE(a_tMoveList[0], to-8, to, MOVE_TYPE_PROM, PIECE_QUEEN,  PIECE_PAWN, PIECE_NONE);
		SET_MOVE(a_tMoveList[1], to-8, to, MOVE_TYPE_PROM, PIECE_ROOK,   PIECE_PAWN, PIECE_NONE);
		SET_MOVE(a_tMoveList[2], to-8, to, MOVE_TYPE_PROM, PIECE_BISHOP, PIECE_PAWN, PIECE_NONE);
		SET_MOVE(a_tMoveList[3], to-8, to, MOVE_TYPE_PROM, PIECE_KNIGHT, PIECE_PAWN, PIECE_NONE);
		a_tMoveList += 4;
		nNumMoves += 4;
	}

	return nNumMoves;
}

int GenerateWhiteKnightCaps(TBoard *b, TMove *a_tMoveList) {
/* 
Generates a list of all capturing moves that can be made by white's knights.
IN:		a_tMoveList		The move array to append to.
OUT:	int				Returns the number of moves found.
*/
	BitBoard bbAttacks, bbPieces;
	int to, from;
	int nNumMoves=0;

	bbPieces = b->bbPieces[WHITE][PIECE_KNIGHT-1];
	while (bbPieces) {				// Loop through each piece on the board
		//from = GetBit(bbPieces);	// Get a square from the BitBoard
		//bbPieces ^= bit[from];		// Subtract that square from the BitBoard
		from = RemoveBit(bbPieces);

		// Calculate all squares attacked by the piece
		bbAttacks = g_bbKnightAttacks[from] & b->bbPieces[BLACK][PIECE_ALL-1];

		// Loop through each attacked square
		while (bbAttacks) {
			to = RemoveBit(bbAttacks);		// Get next bit

			// Add the move to the array
			SET_MOVE(a_tMoveList[0], from, to, MOVE_TYPE_NORMAL, PIECE_NONE, PIECE_KNIGHT, b->nPieces[to]&SQUARE_PIECE_MASK);
			a_tMoveList++;
			nNumMoves++;
		}
	}

 //   for (mask_from = Board->mp[WhiteKnight]; mask_from != 0; mask_from &= (mask_from-1)) {
 //     from = first_one(mask_from);
 //     for (mask_to = MaskKnightMoves[from] & mask_b; mask_to != 0; mask_to &= (mask_to-1)) {
 //       to = first_one(mask_to);
 //       list->move = (from << 6) | to;
 //       (list++)->score = (Board->square[to]) * 3 + 12;  // оценка взятия = capture * 3 + 12;
 //     }
 //   }

	return nNumMoves;
}

int GenerateWhiteKingCaps(TBoard *b, TMove *a_tMoveList) {
/*
Generates a list of all non capturing moves that can be made by white's king.
IN:		a_tMoveList		The move array to append to.
OUT:	int				Returns the number of moves found.
*/
	BitBoard bbAttacks;
	int to, from;
	int nNumMoves=0;

	from = GetBit(b->bbPieces[WHITE][PIECE_KING-1]);

	// Calculate all squares attacked by the piece
	bbAttacks = g_bbKingAttacks[from] & b->bbPieces[BLACK][PIECE_ALL-1];

	// Loop through each attacked square
	while (bbAttacks) {
		//to = GetBit(bbAttacks);		// Get a square from the BitBoard
		//bbAttacks ^= bit[to];		// Subtract that square from the BitBoard
		to = RemoveBit(bbAttacks);

		// Add the move to the array
		SET_MOVE(a_tMoveList[0], from, to, MOVE_TYPE_NORMAL, PIECE_NONE, PIECE_KING, b->nPieces[to]&SQUARE_PIECE_MASK);
		a_tMoveList++;
		//a_tMoveList = AddMove(a_tMoveList, from, to, SPECIAL_MOVE_NONE, 0); // TODO: how to score this?
		nNumMoves++;
	}

	return nNumMoves;
}

int GenerateWhiteRookQueenCaps(TBoard *b, TMove *a_tMoveList) {
/* 
Generates a list of all capturing moves that can be made by white's rooks and queen file/rank moves.
IN:		a_tMoveList		The move array to append to.
OUT:	int				Returns the number of moves found.
*/
	BitBoard bbAttacks, bbPieces;
	int to, from;
	int nNumMoves=0;

	bbPieces = b->bbPieces[WHITE][PIECE_ROOK-1]|b->bbPieces[WHITE][PIECE_QUEEN-1];

	// Loop through each piece on the board
	while (bbPieces) {
		//from = GetBit(bbPieces);	// Get a square from the BitBoard
		//bbPieces ^= bit[from];		// Subtract that square from the BitBoard
		from = RemoveBit(bbPieces);

		// Calculate all empty squares attacked by the piece
		bbAttacks = ROOK_MOVES(from) & b->bbPieces[BLACK][PIECE_ALL-1];

		// Loop through each attacked square
		while (bbAttacks) {
			//to = GetBit(bbAttacks);		// Get a square from the BitBoard
			//bbAttacks ^= bit[to];		// Subtract that square from the BitBoard
			to = RemoveBit(bbAttacks);

			// Add the move to the array
			SET_MOVE(a_tMoveList[0], from, to, MOVE_TYPE_NORMAL, PIECE_NONE, b->nPieces[from]&SQUARE_PIECE_MASK, b->nPieces[to]&SQUARE_PIECE_MASK);
			a_tMoveList++;
			//a_tMoveList = AddMove(a_tMoveList, from, to, SPECIAL_MOVE_NONE, SEE(b,from,to));
			nNumMoves++;
		}
	}

	return nNumMoves;
}

int GenerateWhiteBishopQueenCaps(TBoard *b, TMove *a_tMoveList) {
/* 
Generates a list of all capturing moves that can be made by white's bishops and queen diagonal moves.
IN:		a_tMoveList		The move array to append to.
OUT:	int				Returns the number of moves found.
*/
	BitBoard bbAttacks, bbPieces;
	int to, from;
	int nNumMoves=0;

	bbPieces = b->bbPieces[WHITE][PIECE_BISHOP-1]|b->bbPieces[WHITE][PIECE_QUEEN-1];

	// Loop through each piece on the board
	while (bbPieces) {
		//from = GetBit(bbPieces);	// Get a square from the BitBoard
		//bbPieces ^= bit[from];		// Subtract that square from the BitBoard
		from = RemoveBit(bbPieces);

		// Calculate all empty squares attacked by the piece
		bbAttacks = BISHOP_MOVES(from) & b->bbPieces[BLACK][PIECE_ALL-1];

		// Loop through each attacked square
		while (bbAttacks) {
			//to = GetBit(bbAttacks);		// Get a square from the BitBoard
			//bbAttacks ^= bit[to];		// Subtract that square from the BitBoard
			to = RemoveBit(bbAttacks);

			// Add the move to the array
			SET_MOVE(a_tMoveList[0], from, to, MOVE_TYPE_NORMAL, PIECE_NONE, b->nPieces[from]&SQUARE_PIECE_MASK, b->nPieces[to]&SQUARE_PIECE_MASK);
			a_tMoveList++;
			//a_tMoveList = AddMove(a_tMoveList, from, to, SPECIAL_MOVE_NONE, SEE(b,from,to));
			nNumMoves++;
		}
	}

	return nNumMoves;
}

int GenerateWhitePawnNonCaps(TBoard *b, TMove *a_tMoveList) {
/* 
Generates a list of all non capturing moves that can be made by white's pawns.
IN:		a_tMoveList		The move array to append to.
OUT:	int				Returns the number of moves found.

TODO: can speed up the >= <= with array lookup
TODO: can precalculate w|b for all move generators
*/
	BitBoard bbAttacks;
	int to;
	int nNumMoves=0;

	// Move all pawns forward one square (ignoring promotions)
	bbAttacks = ((b->bbPieces[WHITE][PIECE_PAWN-1] << 8) & g_bbMaskNotRank8) & ~(b->bbPieces[BLACK][PIECE_ALL-1] | b->bbPieces[WHITE][PIECE_ALL-1]);

	// Loop through each attacked square
	while (bbAttacks) {
		//to = GetBit(bbAttacks);		// Get a square from the BitBoard
		//bbAttacks ^= bit[to];		// Subtract that square from the BitBoard
		to = RemoveBit(bbAttacks);

		// Add the move to the array
		SET_MOVE(a_tMoveList[0], to-8, to, MOVE_TYPE_NORMAL, PIECE_NONE, PIECE_PAWN, PIECE_NONE);
		a_tMoveList++;
		//a_tMoveList = AddMove(a_tMoveList, to-8, to, SPECIAL_MOVE_NONE, g_nHistory[to-8][to]);
		nNumMoves++;

		// Look for 2-square pawn moves
		if (to >= H3 && to <= A3 && (bit[to+8] & ~(b->bbPieces[WHITE][PIECE_ALL-1] | b->bbPieces[BLACK][PIECE_ALL-1]))) {
			// Add the move to the array
			SET_MOVE(a_tMoveList[0], to-8, to+8, MOVE_TYPE_NORMAL, PIECE_NONE, PIECE_PAWN, PIECE_NONE);
			a_tMoveList++;
			//a_tMoveList = AddMove(a_tMoveList, to-8, to+8, SPECIAL_MOVE_NONE, g_nHistory[to-8][to+8]);
			nNumMoves++;
		}
	}

	return nNumMoves;
}

int GenerateWhiteKnightNonCaps(TBoard *b, TMove *a_tMoveList) {
/* 
Generates a list of all non capturing moves that can be made by white's knights.
IN:		a_tMoveList		The move array to append to.
OUT:	int				Returns the number of moves found.
*/
	BitBoard bbAttacks, bbPieces;
	int to, from;
	int nNumMoves=0;

	bbPieces = b->bbPieces[WHITE][PIECE_KNIGHT-1];

	// Loop through each piece on the board
	while (bbPieces) {
		//from = GetBit(bbPieces);	// Get a square from the BitBoard
		//bbPieces ^= bit[from];		// Subtract that square from the BitBoard
		from = RemoveBit(bbPieces);

		// Calculate all squares attacked by the piece
		bbAttacks = g_bbKnightAttacks[from] & ~(b->bbPieces[BLACK][PIECE_ALL-1] | b->bbPieces[WHITE][PIECE_ALL-1]);

		// Loop through each attacked square
		while (bbAttacks) {
			//to = GetBit(bbAttacks);		// Get a square from the BitBoard
			//bbAttacks ^= bit[to];		// Subtract that square from the BitBoard
			to = RemoveBit(bbAttacks);

			// Add the move to the array
			SET_MOVE(a_tMoveList[0], from, to, MOVE_TYPE_NORMAL, PIECE_NONE, PIECE_KNIGHT, PIECE_NONE);
			a_tMoveList++;
			//a_tMoveList = AddMove(a_tMoveList, from, to, SPECIAL_MOVE_NONE, g_nHistory[from][to]);
			nNumMoves++;
		}
	}

	return nNumMoves;
}

int GenerateWhiteKingNonCaps(TBoard *b, TMove *a_tMoveList, int nVariant) {
/* 
Generates a list of all non capturing moves that can be made by white's king (inc castling).
IN:		a_tMoveList		The move array to append to.
OUT:	int				Returns the number of moves found.
*/
	BitBoard bbAttacks;
	int to, from;
	int nNumMoves=0;

	from = GetBit(b->bbPieces[WHITE][PIECE_KING-1]);

	// Calculate all squares attacked by the piece
	bbAttacks = g_bbKingAttacks[from] & ~(b->bbPieces[BLACK][PIECE_ALL-1] | b->bbPieces[WHITE][PIECE_ALL-1]);

	// Loop through each attacked square
	while (bbAttacks) {
		//to = GetBit(bbAttacks);		// Get a square from the BitBoard
		//bbAttacks ^= bit[to];		// Subtract that square from the BitBoard
		to = RemoveBit(bbAttacks);

		// Add the move to the array
		SET_MOVE(a_tMoveList[0], from, to, MOVE_TYPE_NORMAL, PIECE_NONE, PIECE_KING, PIECE_NONE);
		a_tMoveList++;
		//a_tMoveList = AddMove(a_tMoveList, from, to, SPECIAL_MOVE_NONE, g_nHistory[from][to]);
		nNumMoves++;
	}

	// Castling moves
	if (b->nCastling & WHITE_KING_CASTLE) { // King and Rook are unmoved
		if (nVariant == VARIANT_960) {
			// Rules at http://en.wikipedia.org/wiki/Chess960
			int ksq1 = from;
			int ksq2 = G1; // final king sq
			int rsq1; for (rsq1=H1;rsq1<A1;rsq1++) { if (b->nPieces[rsq1] == WHITE_ROOK) break; } // find first R from H side
			int rsq2 = F1; // final rook sq
			bool illegal = false;
			for (int sq=MIN(ksq2,from);sq<=MAX(ksq2,from);sq++) {
				if (sq!=rsq1 && sq!=ksq1 && sq!=ksq2 && b->nPieces[sq]) { illegal = true; break; }	// king impeded
				if (IsSquareAttackedBy(b,sq,BLACK)) { illegal = true; break; }			// attacked
			}
			if (!illegal) for (int sq=MIN(rsq2,rsq1);sq<=MAX(rsq2,rsq1);sq++) {
				if (sq!=rsq1 && sq!=rsq2 && sq!=ksq1 && b->nPieces[sq]) illegal = true;	// rook impeded
			}
			if (rsq1 != rsq2 && b->nPieces[rsq2] && b->nPieces[rsq2]!=WHITE_KING) illegal = true;					// piece on final rook square other than king
			if (from != ksq2 && b->nPieces[ksq2] && 
				(b->nPieces[ksq2]!=WHITE_ROOK || (b->nPieces[ksq2]==WHITE_ROOK && ksq2!=rsq1))) illegal = true;		// piece on final king square other than castling rook
			if (!illegal) {
				SET_MOVE(a_tMoveList[0], from, G1, MOVE_TYPE_CASTLE, PIECE_NONE, PIECE_KING, PIECE_NONE);
				a_tMoveList++;
				nNumMoves++;
			}
		} else {
			if (!b->nPieces[F1] && !b->nPieces[G1]) { // no blocking pieces
				if (!IsSquareAttackedBy(b,E1,BLACK) && !IsSquareAttackedBy(b,F1,BLACK) && !IsSquareAttackedBy(b,G1,BLACK)) {
					SET_MOVE(a_tMoveList[0], E1, G1, MOVE_TYPE_CASTLE, PIECE_NONE, PIECE_KING, PIECE_NONE);
					a_tMoveList++;
					nNumMoves++;
				}
			}
		}
	}
	if (b->nCastling & WHITE_QUEEN_CASTLE) { // King and Rook are unmoved
		if (nVariant == VARIANT_960) {
			// Rules at http://en.wikipedia.org/wiki/Chess960
			int ksq1 = from;
			int ksq2 = C1; // final king sq
			int rsq1; for (rsq1=A1;rsq1>H1;rsq1--) { if (b->nPieces[rsq1] == WHITE_ROOK) break; } // find first R from A side
			int rsq2 = D1; // final rook sq
			bool illegal = false;
			for (int sq=MIN(ksq2,from);sq<=MAX(ksq2,from);sq++) {
				if (sq!=rsq1 && sq!=ksq1 && sq!=ksq2 && b->nPieces[sq]) { illegal = true; break; }	// king impeded
				if (IsSquareAttackedBy(b,sq,BLACK)) { illegal = true; break; }			// attacked
			}
			if (!illegal) for (int sq=MIN(rsq2,rsq1);sq<=MAX(rsq2,rsq1);sq++) {
				if (sq!=rsq1 && sq!=rsq2 && sq!=ksq1 && b->nPieces[sq]) illegal = true;	// rook impeded
			}
			if (rsq1 != rsq2 && b->nPieces[rsq2] && b->nPieces[rsq2]!=WHITE_KING) illegal = true;					// piece on final rook square other than king
			if (from != ksq2 && b->nPieces[ksq2] && 
				(b->nPieces[ksq2]!=WHITE_ROOK || (b->nPieces[ksq2]==WHITE_ROOK && ksq2!=rsq1))) illegal = true;		// piece on final king square other than castling rook
			if (FILE(rsq2) == FILE_B && (b->nPieces[A1] == BLACK_ROOK || b->nPieces[A1] == BLACK_QUEEN)) illegal = true;	// stupid 960 exception to look out for
			if (!illegal) {
				SET_MOVE(a_tMoveList[0], from, C1, MOVE_TYPE_CASTLE, PIECE_NONE, PIECE_KING, PIECE_NONE);
				a_tMoveList++;
				nNumMoves++;
			}
		} else {
			if (!b->nPieces[D1] && !b->nPieces[C1] && !b->nPieces[B1]) { // no blocking pieces
				if (!IsSquareAttackedBy(b,E1,BLACK) && !IsSquareAttackedBy(b,D1,BLACK) && !IsSquareAttackedBy(b,C1,BLACK)) {
					SET_MOVE(a_tMoveList[0], E1, C1, MOVE_TYPE_CASTLE, PIECE_NONE, PIECE_KING, PIECE_NONE);
					a_tMoveList++;
					nNumMoves++;
				}
			}
		}
	}
		
	return nNumMoves;
}

int GenerateWhiteRookQueenNonCaps(TBoard *b, TMove *a_tMoveList) {
/* 
Generates a list of all non capturing moves that can be made by white's rooks and queen file/rank moves.
IN:		a_tMoveList		The move array to append to.
OUT:	int				Returns the number of moves found.
*/
	BitBoard bbAttacks, bbPieces;
	int to, from;
	int nNumMoves=0;

	bbPieces = b->bbPieces[WHITE][PIECE_ROOK-1]|b->bbPieces[WHITE][PIECE_QUEEN-1];

	// Loop through each piece on the board
	while (bbPieces) {
		//from = GetBit(bbPieces);	// Get a square from the BitBoard
		//bbPieces ^= bit[from];		// Subtract that square from the BitBoard
		from = RemoveBit(bbPieces);

		// Calculate all empty squares attacked by the piece
		bbAttacks = ROOK_MOVES(from) & ~(b->bbPieces[BLACK][PIECE_ALL-1] | b->bbPieces[WHITE][PIECE_ALL-1]);

		// Loop through each attacked square
		while (bbAttacks) {
			//to = GetBit(bbAttacks);		// Get a square from the BitBoard
			//bbAttacks ^= bit[to];		// Subtract that square from the BitBoard
			to = RemoveBit(bbAttacks);

			// Add the move to the array
			SET_MOVE(a_tMoveList[0], from, to, MOVE_TYPE_NORMAL, PIECE_NONE, b->nPieces[from]&SQUARE_PIECE_MASK, PIECE_NONE);
			a_tMoveList++;
			//a_tMoveList = AddMove(a_tMoveList, from, to, SPECIAL_MOVE_NONE, g_nHistory[from][to]);
			nNumMoves++;
		}
	}

	return nNumMoves;
}

int GenerateWhiteBishopQueenNonCaps(TBoard *b, TMove *a_tMoveList) {
/* 
Generates a list of all non capturing moves that can be made by white's bishops and queen diagonal moves.
IN:		a_tMoveList		The move array to append to.
OUT:	int				Returns the number of moves found.
*/
	BitBoard bbAttacks, bbPieces;
	int to, from;
	int nNumMoves=0;

	bbPieces = b->bbPieces[WHITE][PIECE_BISHOP-1]|b->bbPieces[WHITE][PIECE_QUEEN-1];

	// Loop through each piece on the board
	while (bbPieces) {
		//from = GetBit(bbPieces);	// Get a square from the BitBoard
		//bbPieces ^= bit[from];		// Subtract that square from the BitBoard
		from = RemoveBit(bbPieces);

		// Calculate all empty squares attacked by the piece
		bbAttacks = BISHOP_MOVES(from) & ~(b->bbPieces[BLACK][PIECE_ALL-1] | b->bbPieces[WHITE][PIECE_ALL-1]);

		// Loop through each attacked square
		while (bbAttacks) {
			//to = GetBit(bbAttacks);		// Get a square from the BitBoard
			//bbAttacks ^= bit[to];		// Subtract that square from the BitBoard
			to = RemoveBit(bbAttacks);

			// Add the move to the array
			// TODO: can make this faster by precalculating the static shifted fields? need to profile
			SET_MOVE(a_tMoveList[0], from, to, MOVE_TYPE_NORMAL, PIECE_NONE, b->nPieces[from]&SQUARE_PIECE_MASK, PIECE_NONE);
			a_tMoveList++;
			//a_tMoveList = AddMove(a_tMoveList, from, to, SPECIAL_MOVE_NONE, g_nHistory[from][to]);
			nNumMoves++;
		}
	}

	return nNumMoves;
}

int GenerateBlackPawnCaps(TBoard *b, TMove *a_tMoveList) {
/* 
Generates a list of all capturing or promoting moves that can be made by black's pawns.
IN:		a_tMoveList		The move array to append to.
OUT:	int				Returns the number of moves found.

TODO: can speed up the >= <= with array lookup
TODO: can precalculate w|b for all move generators
*/
	BitBoard bbAttacks;
	int to, from;
	int nNumMoves=0;

	// Attack left
	bbAttacks = ((b->bbPieces[BLACK][PIECE_PAWN-1] & g_bbMaskNotFileA) >> 7) & b->bbPieces[WHITE][PIECE_ALL-1];

	// Loop through each attacked square
	while (bbAttacks) {
		//to = GetBit(bbAttacks);		// Get a square from the BitBoard
		//bbAttacks ^= bit[to];		// Subtract that square from the BitBoard
		to = RemoveBit(bbAttacks);

		if (to <= A1) {
			// Pawn has promoted
			SET_MOVE(a_tMoveList[0], to+7, to, MOVE_TYPE_PROM, PIECE_QUEEN,  PIECE_PAWN, b->nPieces[to]&SQUARE_PIECE_MASK);
			SET_MOVE(a_tMoveList[1], to+7, to, MOVE_TYPE_PROM, PIECE_ROOK,   PIECE_PAWN, b->nPieces[to]&SQUARE_PIECE_MASK);
			SET_MOVE(a_tMoveList[2], to+7, to, MOVE_TYPE_PROM, PIECE_BISHOP, PIECE_PAWN, b->nPieces[to]&SQUARE_PIECE_MASK);
			SET_MOVE(a_tMoveList[3], to+7, to, MOVE_TYPE_PROM, PIECE_KNIGHT, PIECE_PAWN, b->nPieces[to]&SQUARE_PIECE_MASK);
			a_tMoveList += 4;
			//a_tMoveList = AddMove(a_tMoveList, to+7, to, SPECIAL_MOVE_PROM_Q, SEE(b,to+7,to));
			//a_tMoveList = AddMove(a_tMoveList, to+7, to, SPECIAL_MOVE_PROM_R, SEE(b,to+7,to));
			//a_tMoveList = AddMove(a_tMoveList, to+7, to, SPECIAL_MOVE_PROM_B, SEE(b,to+7,to));
			//a_tMoveList = AddMove(a_tMoveList, to+7, to, SPECIAL_MOVE_PROM_N, SEE(b,to+7,to));
			nNumMoves += 4;
		} else {
			// Add the move to the array
			SET_MOVE(a_tMoveList[0], to+7, to, MOVE_TYPE_NORMAL, PIECE_NONE, PIECE_PAWN, b->nPieces[to]&SQUARE_PIECE_MASK);
			a_tMoveList ++;
			//a_tMoveList = AddMove(a_tMoveList, to+7, to, SPECIAL_MOVE_NONE, SEE(b,to+7,to));
			nNumMoves++;
		}
	}

	// Attack right
	bbAttacks = ((b->bbPieces[BLACK][PIECE_PAWN-1] & g_bbMaskNotFileH) >> 9) & b->bbPieces[WHITE][PIECE_ALL-1];

	// Loop through each attacked square
	while (bbAttacks) {
		//to = GetBit(bbAttacks);		// Get a square from the BitBoard
		//bbAttacks ^= bit[to];		// Subtract that square from the BitBoard
		to = RemoveBit(bbAttacks);

		if (to <= A1) {
			// Pawn has promoted
			SET_MOVE(a_tMoveList[0], to+9, to, MOVE_TYPE_PROM, PIECE_QUEEN,  PIECE_PAWN, b->nPieces[to]&SQUARE_PIECE_MASK);
			SET_MOVE(a_tMoveList[1], to+9, to, MOVE_TYPE_PROM, PIECE_ROOK,   PIECE_PAWN, b->nPieces[to]&SQUARE_PIECE_MASK);
			SET_MOVE(a_tMoveList[2], to+9, to, MOVE_TYPE_PROM, PIECE_BISHOP, PIECE_PAWN, b->nPieces[to]&SQUARE_PIECE_MASK);
			SET_MOVE(a_tMoveList[3], to+9, to, MOVE_TYPE_PROM, PIECE_KNIGHT, PIECE_PAWN, b->nPieces[to]&SQUARE_PIECE_MASK);
			a_tMoveList += 4;
			//a_tMoveList = AddMove(a_tMoveList, to+9, to, SPECIAL_MOVE_PROM_Q, SEE(b,to+9,to));
			//a_tMoveList = AddMove(a_tMoveList, to+9, to, SPECIAL_MOVE_PROM_R, SEE(b,to+9,to));
			//a_tMoveList = AddMove(a_tMoveList, to+9, to, SPECIAL_MOVE_PROM_B, SEE(b,to+9,to));
			//a_tMoveList = AddMove(a_tMoveList, to+9, to, SPECIAL_MOVE_PROM_N, SEE(b,to+9,to));
			nNumMoves += 4;
		} else {
			// Add the move to the array
			SET_MOVE(a_tMoveList[0], to+9, to, MOVE_TYPE_NORMAL, PIECE_NONE, PIECE_PAWN, b->nPieces[to]&SQUARE_PIECE_MASK);
			a_tMoveList ++;
			//a_tMoveList = AddMove(a_tMoveList, to+9, to, SPECIAL_MOVE_NONE, SEE(b,to+9,to));
			nNumMoves++;
		}
	}

	// EnPassent captures
	if (b->nEPSquare != NULL_SQUARE) {
		bbAttacks = g_nnBlackEPAttacks[b->nEPSquare] & b->bbPieces[BLACK][PIECE_PAWN-1];
		while (bbAttacks) {
			//from = GetBit(bbAttacks);	// Get a square from the BitBoard
			//bbAttacks ^= bit[from];		// Subtract that square from the BitBoard
			from = RemoveBit(bbAttacks);

			// Add the move to the array
			SET_MOVE(a_tMoveList[0], from, b->nEPSquare, MOVE_TYPE_EP, PIECE_NONE, PIECE_PAWN, PIECE_PAWN); 
			a_tMoveList ++;
			//a_tMoveList = AddMove(a_tMoveList, from, b->nEPSquare, SPECIAL_MOVE_EP, 0);
			nNumMoves++;
		}
	}

	// Non-capture Promotions
	bbAttacks = (b->bbPieces[BLACK][PIECE_PAWN-1] >> 8) & g_bbMaskRank1 & (~(b->bbPieces[BLACK][PIECE_ALL-1] | b->bbPieces[WHITE][PIECE_ALL-1]));
	while (bbAttacks) {
		//to = GetBit(bbAttacks);		// Get a square from the BitBoard
		//bbAttacks ^= bit[to];		// Subtract that square from the BitBoard
		to = RemoveBit(bbAttacks);

		SET_MOVE(a_tMoveList[0], to+8, to, MOVE_TYPE_PROM, PIECE_QUEEN,  PIECE_PAWN, PIECE_NONE);
		SET_MOVE(a_tMoveList[1], to+8, to, MOVE_TYPE_PROM, PIECE_ROOK,   PIECE_PAWN, PIECE_NONE);
		SET_MOVE(a_tMoveList[2], to+8, to, MOVE_TYPE_PROM, PIECE_BISHOP, PIECE_PAWN, PIECE_NONE);
		SET_MOVE(a_tMoveList[3], to+8, to, MOVE_TYPE_PROM, PIECE_KNIGHT, PIECE_PAWN, PIECE_NONE);
		a_tMoveList += 4;
		//a_tMoveList = AddMove(a_tMoveList, to+8, to, SPECIAL_MOVE_PROM_Q, SCORE_QUEEN);
		//a_tMoveList = AddMove(a_tMoveList, to+8, to, SPECIAL_MOVE_PROM_R, SCORE_ROOK);
		//a_tMoveList = AddMove(a_tMoveList, to+8, to, SPECIAL_MOVE_PROM_B, SCORE_BISHOP);
		//a_tMoveList = AddMove(a_tMoveList, to+8, to, SPECIAL_MOVE_PROM_N, SCORE_KNIGHT);
		nNumMoves += 4;
	}

	return nNumMoves;
}

int GenerateBlackKnightCaps(TBoard *b, TMove *a_tMoveList) {
/* 
Generates a list of all capturing moves that can be made by black's knights.
IN:		a_tMoveList		The move array to append to.
OUT:	int				Returns the number of moves found.
*/
	BitBoard bbAttacks, bbPieces;
	int to, from;
	int nNumMoves=0;

	bbPieces = b->bbPieces[BLACK][PIECE_KNIGHT-1];

	// Loop through each piece on the board
	while (bbPieces) {
		//from = GetBit(bbPieces);	// Get a square from the BitBoard
		//bbPieces ^= bit[from];		// Subtract that square from the BitBoard
		from = RemoveBit(bbPieces);

		// Calculate all squares attacked by the piece
		bbAttacks = g_bbKnightAttacks[from] & b->bbPieces[WHITE][PIECE_ALL-1];

		// Loop through each attacked square
		while (bbAttacks) {
			//to = GetBit(bbAttacks);		// Get a square from the BitBoard
			//bbAttacks ^= bit[to];		// Subtract that square from the BitBoard
			to = RemoveBit(bbAttacks);

			// Add the move to the array
			SET_MOVE(a_tMoveList[0], from, to, MOVE_TYPE_NORMAL, PIECE_NONE, PIECE_KNIGHT, b->nPieces[to]&SQUARE_PIECE_MASK);
			a_tMoveList ++;
			//a_tMoveList = AddMove(a_tMoveList, from, to, SPECIAL_MOVE_NONE, SEE(b,from,to));
			nNumMoves++;
		}
	}

	return nNumMoves;
}

int GenerateBlackKingCaps(TBoard *b, TMove *a_tMoveList) {
/* 
Generates a list of all non capturing moves that can be made by black's king.
IN:		a_tMoveList		The move array to append to.
OUT:	int				Returns the number of moves found.
*/
	BitBoard bbAttacks;
	int to, from;
	int nNumMoves=0;

	from = GetBit(b->bbPieces[BLACK][PIECE_KING-1]);

	// Calculate all squares attacked by the piece
	bbAttacks = g_bbKingAttacks[from] & b->bbPieces[WHITE][PIECE_ALL-1];

	// Loop through each attacked square
	while (bbAttacks) {
		//to = GetBit(bbAttacks);		// Get a square from the BitBoard
		//bbAttacks ^= bit[to];		// Subtract that square from the BitBoard
		to = RemoveBit(bbAttacks);

		// Add the move to the array
		SET_MOVE(a_tMoveList[0], from, to, MOVE_TYPE_NORMAL, PIECE_NONE, PIECE_KING, b->nPieces[to]&SQUARE_PIECE_MASK);
		a_tMoveList ++;
		//a_tMoveList = AddMove(a_tMoveList, from, to, SPECIAL_MOVE_NONE, 0);
		nNumMoves++;
	}

	return nNumMoves;
}

int GenerateBlackRookQueenCaps(TBoard *b, TMove *a_tMoveList) {
/* 
Generates a list of all capturing moves that can be made by black's rooks and queen file/rank moves.
IN:		a_tMoveList		The move array to append to.
OUT:	int				Returns the number of moves found.
*/
	BitBoard bbAttacks, bbPieces;
	int to, from;
	int nNumMoves=0;

	bbPieces = b->bbPieces[BLACK][PIECE_ROOK-1]|b->bbPieces[BLACK][PIECE_QUEEN-1];

	// Loop through each piece on the board
	while (bbPieces) {
		//from = GetBit(bbPieces);	// Get a square from the BitBoard
		//bbPieces ^= bit[from];		// Subtract that square from the BitBoard
		from = RemoveBit(bbPieces);

		// Calculate all empty squares attacked by the piece
		bbAttacks = ROOK_MOVES(from) & b->bbPieces[WHITE][PIECE_ALL-1];

		// Loop through each attacked square
		while (bbAttacks) {
			//to = GetBit(bbAttacks);		// Get a square from the BitBoard
			//bbAttacks ^= bit[to];		// Subtract that square from the BitBoard
			to = RemoveBit(bbAttacks);

			// Add the move to the array
			SET_MOVE(a_tMoveList[0], from, to, MOVE_TYPE_NORMAL, PIECE_NONE, b->nPieces[from]&SQUARE_PIECE_MASK, b->nPieces[to]&SQUARE_PIECE_MASK);
			a_tMoveList ++;
			//a_tMoveList = AddMove(a_tMoveList, from, to, SPECIAL_MOVE_NONE, SEE(b,from,to));
			nNumMoves++;
		}
	}

	return nNumMoves;
}

int GenerateBlackBishopQueenCaps(TBoard *b, TMove *a_tMoveList) {
/* 
Generates a list of all capturing moves that can be made by black's bishops and queen diagonal moves.
IN:		a_tMoveList		The move array to append to.
OUT:	int				Returns the number of moves found.
*/
	BitBoard bbAttacks, bbPieces;
	int to, from;
	int nNumMoves=0;

	bbPieces = b->bbPieces[BLACK][PIECE_BISHOP-1]|b->bbPieces[BLACK][PIECE_QUEEN-1];

	// Loop through each piece on the board
	while (bbPieces) {
		//from = GetBit(bbPieces);	// Get a square from the BitBoard
		//bbPieces ^= bit[from];		// Subtract that square from the BitBoard
		from = RemoveBit(bbPieces);

		// Calculate all empty squares attacked by the piece
		bbAttacks = BISHOP_MOVES(from) & b->bbPieces[WHITE][PIECE_ALL-1];

		// Loop through each attacked square
		while (bbAttacks) {
			//to = GetBit(bbAttacks);		// Get a square from the BitBoard
			//bbAttacks ^= bit[to];		// Subtract that square from the BitBoard
			to = RemoveBit(bbAttacks);

			// Add the move to the array
			SET_MOVE(a_tMoveList[0], from, to, MOVE_TYPE_NORMAL, PIECE_NONE, b->nPieces[from]&SQUARE_PIECE_MASK, b->nPieces[to]&SQUARE_PIECE_MASK);
			a_tMoveList ++;
			//a_tMoveList = AddMove(a_tMoveList, from, to, SPECIAL_MOVE_NONE, SEE(b,from,to));
			nNumMoves++;
		}
	}

	return nNumMoves;
}

int GenerateBlackPawnNonCaps(TBoard *b, TMove *a_tMoveList) {
/* 
Generates a list of all non capturing moves that can be made by black's pawns.
IN:		a_tMoveList		The move array to append to.
OUT:	int				Returns the number of moves found.

TODO: can speed up the >= <= with array lookup
TODO: can precalculate w|b for all move generators
*/
	BitBoard bbAttacks;
	int to;
	int nNumMoves=0;

	// Move all pawns forward one square (ignoring promotions)
	bbAttacks = ((b->bbPieces[BLACK][PIECE_PAWN-1] >> 8) & g_bbMaskNotRank1) & ~(b->bbPieces[WHITE][PIECE_ALL-1] | b->bbPieces[BLACK][PIECE_ALL-1]);

	// Loop through each attacked square
	while (bbAttacks) {
		//to = GetBit(bbAttacks);		// Get a square from the BitBoard
		//bbAttacks ^= bit[to];		// Subtract that square from the BitBoard
		to = RemoveBit(bbAttacks);

		// Add the move to the array
		SET_MOVE(a_tMoveList[0], to+8, to, MOVE_TYPE_NORMAL, PIECE_NONE, PIECE_PAWN, PIECE_NONE);
		a_tMoveList ++;
		//a_tMoveList = AddMove(a_tMoveList, to+8, to, SPECIAL_MOVE_NONE, g_nHistory[to+8][to]);
		nNumMoves++;

		// Look for 2-square pawn moves
		if (to >= H6 && to <= A6 && (bit[to-8] & ~(b->bbPieces[BLACK][PIECE_ALL-1] | b->bbPieces[WHITE][PIECE_ALL-1]))) {
			// Add the move to the array
			SET_MOVE(a_tMoveList[0], to+8, to-8, MOVE_TYPE_NORMAL, PIECE_NONE, PIECE_PAWN, PIECE_NONE);
			a_tMoveList ++;
			//a_tMoveList = AddMove(a_tMoveList, to+8, to-8, SPECIAL_MOVE_NONE, g_nHistory[to+8][to-8]);
			nNumMoves++;
		}
	}

	return nNumMoves;
}

int GenerateBlackKnightNonCaps(TBoard *b, TMove *a_tMoveList) {
/* 
Generates a list of all non capturing moves that can be made by black's knights.
IN:		a_tMoveList		The move array to append to.
OUT:	int				Returns the number of moves found.
*/
	BitBoard bbAttacks, bbPieces;
	int to, from;
	int nNumMoves=0;

	bbPieces = b->bbPieces[BLACK][PIECE_KNIGHT-1];

	// Loop through each piece on the board
	while (bbPieces) {
		//from = GetBit(bbPieces);	// Get a square from the BitBoard
		//bbPieces ^= bit[from];		// Subtract that square from the BitBoard
		from = RemoveBit(bbPieces);

		// Calculate all squares attacked by the piece
		bbAttacks = g_bbKnightAttacks[from] & ~(b->bbPieces[WHITE][PIECE_ALL-1] | b->bbPieces[BLACK][PIECE_ALL-1]);

		// Loop through each attacked square
		while (bbAttacks) {
			//to = GetBit(bbAttacks);		// Get a square from the BitBoard
			//bbAttacks ^= bit[to];		// Subtract that square from the BitBoard
			to = RemoveBit(bbAttacks);

			// Add the move to the array
			SET_MOVE(a_tMoveList[0], from, to, MOVE_TYPE_NORMAL, PIECE_NONE, PIECE_KNIGHT, PIECE_NONE);
			a_tMoveList ++;
			//a_tMoveList = AddMove(a_tMoveList, from, to, SPECIAL_MOVE_NONE, g_nHistory[from][to]);
			nNumMoves++;
		}
	}

	return nNumMoves;
}

int GenerateBlackKingNonCaps(TBoard *b, TMove *a_tMoveList, int nVariant) {
/* 
Generates a list of all non capturing moves that can be made by black's king (inc castling).
IN:		a_tMoveList		The move array to append to.
OUT:	int				Returns the number of moves found.
*/
	BitBoard bbAttacks;
	int to, from;
	int nNumMoves=0;

	from = GetBit(b->bbPieces[BLACK][PIECE_KING-1]);

	// Calculate all squares attacked by the piece
	bbAttacks = g_bbKingAttacks[from] & ~(b->bbPieces[WHITE][PIECE_ALL-1] | b->bbPieces[BLACK][PIECE_ALL-1]);

	// Loop through each attacked square
	while (bbAttacks) {
		//to = GetBit(bbAttacks);		// Get a square from the BitBoard
		//bbAttacks ^= bit[to];		// Subtract that square from the BitBoard
		to = RemoveBit(bbAttacks);

		// Add the move to the array
		SET_MOVE(a_tMoveList[0], from, to, MOVE_TYPE_NORMAL, PIECE_NONE, PIECE_KING, PIECE_NONE);
		a_tMoveList ++;
		//a_tMoveList = AddMove(a_tMoveList, from, to, SPECIAL_MOVE_NONE, g_nHistory[from][to]);
		nNumMoves++;
	}

	// Castling moves
	if (b->nCastling & BLACK_KING_CASTLE) { // King and Rook are unmoved
		if (nVariant == VARIANT_960) {
			// Rules at http://en.wikipedia.org/wiki/Chess960
			int ksq1 = from;
			int ksq2 = G8; // final king sq
			int rsq1; for (rsq1=H8;rsq1<A8;rsq1++) { if (b->nPieces[rsq1] == BLACK_ROOK) break; } // find first R from H side
			int rsq2 = F8; // final rook sq
			bool illegal = false;
			for (int sq=MIN(ksq2,from);sq<=MAX(ksq2,from);sq++) {
				if (sq!=rsq1 && sq!=ksq1 && sq!=ksq2 && b->nPieces[sq]) { illegal = true; break; }	// king impeded
				if (IsSquareAttackedBy(b,sq,WHITE)) { illegal = true; break; }			// attacked
			}
			if (!illegal) for (int sq=MIN(rsq2,rsq1);sq<=MAX(rsq2,rsq1);sq++) {
				if (sq!=rsq1 && sq!=rsq2 && sq!=ksq1 && b->nPieces[sq]) illegal = true;	// rook impeded
			}
			if (rsq1 != rsq2 && b->nPieces[rsq2] && b->nPieces[rsq2]!=BLACK_KING) illegal = true;					// piece on final rook square other than king
			if (from != ksq2 && b->nPieces[ksq2] && 
				(b->nPieces[ksq2]!=BLACK_ROOK || (b->nPieces[ksq2]==BLACK_ROOK && ksq2!=rsq1))) illegal = true;		// piece on final king square other than castling rook
			if (!illegal) {
				SET_MOVE(a_tMoveList[0], from, G8, MOVE_TYPE_CASTLE, PIECE_NONE, PIECE_KING, PIECE_NONE);
				a_tMoveList++;
				nNumMoves++;
			}
		} else {
			if (!b->nPieces[F8] && !b->nPieces[G8]) { // no blocking pieces
				if (!IsSquareAttackedBy(b,E8,WHITE) && !IsSquareAttackedBy(b,F8,WHITE) && !IsSquareAttackedBy(b,G8,WHITE)) {
					SET_MOVE(a_tMoveList[0], E8, G8, MOVE_TYPE_CASTLE, PIECE_NONE, PIECE_KING, PIECE_NONE);
					a_tMoveList ++;
					nNumMoves++;
				}
			}
		}
	}
	if (b->nCastling & BLACK_QUEEN_CASTLE) { // King and Rook are unmoved
		if (nVariant == VARIANT_960) {
			// Rules at http://en.wikipedia.org/wiki/Chess960
			int ksq1 = from;
			int ksq2 = C8; // final king sq
			int rsq1; for (rsq1=A8;rsq1>H8;rsq1--) { if (b->nPieces[rsq1] == BLACK_ROOK) break; } // find first R from A side
			int rsq2 = D8; // final rook sq
			bool illegal = false;
			for (int sq=MIN(ksq2,from);sq<=MAX(ksq2,from);sq++) {
				if (sq!=rsq1 && sq!=ksq1 && sq!=ksq2 && b->nPieces[sq]) { illegal = true; break; }	// king impeded
				if (IsSquareAttackedBy(b,sq,WHITE)) { illegal = true; break; }			// attacked
			}
			if (!illegal) for (int sq=MIN(rsq2,rsq1);sq<=MAX(rsq2,rsq1);sq++) {
				if (sq!=rsq1 && sq!=rsq2 && sq!=ksq1 && b->nPieces[sq]) illegal = true;	// rook impeded
			}
			if (rsq1 != rsq2 && b->nPieces[rsq2] && b->nPieces[rsq2]!=BLACK_KING) illegal = true;					// piece on final rook square other than king
			if (from != ksq2 && b->nPieces[ksq2] && 
				(b->nPieces[ksq2]!=BLACK_ROOK || (b->nPieces[ksq2]==BLACK_ROOK && ksq2!=rsq1))) illegal = true;		// piece on final king square other than castling rook
			if (rsq1==B8 && (b->nPieces[A8] == WHITE_ROOK || b->nPieces[A8] == WHITE_QUEEN)) illegal = true;	// stupid 960 exception to look out for
			if (!illegal) {
				SET_MOVE(a_tMoveList[0], from, C8, MOVE_TYPE_CASTLE, PIECE_NONE, PIECE_KING, PIECE_NONE);
				a_tMoveList++;
				nNumMoves++;
			}
		} else {
			if (!b->nPieces[D8] && !b->nPieces[C8] && !b->nPieces[B8]) { // no blocking pieces
				if (!IsSquareAttackedBy(b,E8,WHITE) && !IsSquareAttackedBy(b,D8,WHITE) && !IsSquareAttackedBy(b,C8,WHITE)) {
					SET_MOVE(a_tMoveList[0], E8, C8, MOVE_TYPE_CASTLE, PIECE_NONE, PIECE_KING, PIECE_NONE);
					a_tMoveList ++;
					nNumMoves++;
				}
			}
		}
	}
		
	return nNumMoves;
}

int GenerateBlackRookQueenNonCaps(TBoard *b, TMove *a_tMoveList) {
/* 
Generates a list of all non capturing moves that can be made by black's rooks and queen file/rank moves.
IN:		a_tMoveList		The move array to append to.
OUT:	int				Returns the number of moves found.
*/
	BitBoard bbAttacks, bbPieces;
	int to, from;
	int nNumMoves=0;

	bbPieces = b->bbPieces[BLACK][PIECE_ROOK-1]|b->bbPieces[BLACK][PIECE_QUEEN-1];

	// Loop through each piece on the board
	while (bbPieces) {
		//from = GetBit(bbPieces);	// Get a square from the BitBoard
		//bbPieces ^= bit[from];		// Subtract that square from the BitBoard
		from = RemoveBit(bbPieces);

		// Calculate all empty squares attacked by the piece
		bbAttacks = ROOK_MOVES(from) & ~(b->bbPieces[WHITE][PIECE_ALL-1] | b->bbPieces[BLACK][PIECE_ALL-1]);

		// Loop through each attacked square
		while (bbAttacks) {
			//to = GetBit(bbAttacks);		// Get a square from the BitBoard
			//bbAttacks ^= bit[to];		// Subtract that square from the BitBoard
			to = RemoveBit(bbAttacks);

			// Add the move to the array
			SET_MOVE(a_tMoveList[0], from, to, MOVE_TYPE_NORMAL, PIECE_NONE, b->nPieces[from]&SQUARE_PIECE_MASK, PIECE_NONE);
			a_tMoveList ++;
			//a_tMoveList = AddMove(a_tMoveList, from, to, SPECIAL_MOVE_NONE, g_nHistory[from][to]);
			nNumMoves++;
		}
	}

	return nNumMoves;
}

int GenerateBlackBishopQueenNonCaps(TBoard *b, TMove *a_tMoveList) {
/* 
Generates a list of all non capturing moves that can be made by black's bishops and queen diagonal moves.
IN:		a_tMoveList		The move array to append to.
OUT:	int				Returns the number of moves found.
*/
	BitBoard bbAttacks, bbPieces;
	int to, from;
	int nNumMoves=0;

	bbPieces = b->bbPieces[BLACK][PIECE_BISHOP-1]|b->bbPieces[BLACK][PIECE_QUEEN-1];

	// Loop through each piece on the board
	while (bbPieces) {
		//from = GetBit(bbPieces);	// Get a square from the BitBoard
		//bbPieces ^= bit[from];		// Subtract that square from the BitBoard
		from = RemoveBit(bbPieces);

		// Calculate all empty squares attacked by the piece
		bbAttacks = BISHOP_MOVES(from) & ~(b->bbPieces[WHITE][PIECE_ALL-1] | b->bbPieces[BLACK][PIECE_ALL-1]);

		// Loop through each attacked square
		while (bbAttacks) {
			//to = GetBit(bbAttacks);		// Get a square from the BitBoard
			//bbAttacks ^= bit[to];		// Subtract that square from the BitBoard
			to = RemoveBit(bbAttacks);

			// Add the move to the array
			SET_MOVE(a_tMoveList[0], from, to, MOVE_TYPE_NORMAL, PIECE_NONE, b->nPieces[from]&SQUARE_PIECE_MASK, PIECE_NONE);
			a_tMoveList ++;
			//a_tMoveList = AddMove(a_tMoveList, from, to, SPECIAL_MOVE_NONE, g_nHistory[from][to]);
			nNumMoves++;
		}
	}

	return nNumMoves;
}

bool IsAnyLegalMoves(TBoard *b, int nVariant) {

	int nNumMoves = 0;
	TMove tMoveList[50];
	//TBoard b2;
	//bool nIsIllegal;
	TUndoMove undo;
	//memcpy(&b2,b,sizeof(TBoard));

	if (b->nSideToMove == WHITE) {
		nNumMoves = GenerateWhitePawnNonCaps(b, tMoveList);
		for (int i=0;i<nNumMoves;i++) {
			MakeMove2(b, tMoveList[i], &undo, 0, 0, nVariant);
			if (IsSquareAttackedBy(b, GetBit(b->bbPieces[WHITE][PIECE_KING-1]), BLACK)) {
				UnMakeMove(b, tMoveList[i], &undo, 0, nVariant);
			} else {
				UnMakeMove(b, tMoveList[i], &undo, 0, nVariant);
				return true;
			}
		}
		nNumMoves = GenerateWhitePawnCaps(b, tMoveList);
		for (int i=0;i<nNumMoves;i++) {
			MakeMove2(b, tMoveList[i], &undo, 0, 0, nVariant);
			if (IsSquareAttackedBy(b, GetBit(b->bbPieces[WHITE][PIECE_KING-1]), BLACK)) {
				UnMakeMove(b, tMoveList[i], &undo, 0, nVariant);
			} else {
				UnMakeMove(b, tMoveList[i], &undo, 0, nVariant);
				return true;
			}
		}
		nNumMoves = GenerateWhiteKnightNonCaps(b, tMoveList);
		for (int i=0;i<nNumMoves;i++) {
			MakeMove2(b, tMoveList[i], &undo, 0, 0, nVariant);
			if (IsSquareAttackedBy(b, GetBit(b->bbPieces[WHITE][PIECE_KING-1]), BLACK)) {
				UnMakeMove(b, tMoveList[i], &undo, 0, nVariant);
			} else {
				UnMakeMove(b, tMoveList[i], &undo, 0, nVariant);
				return true;
			}
		}
		nNumMoves = GenerateWhiteKnightCaps(b, tMoveList);
		for (int i=0;i<nNumMoves;i++) {
			MakeMove2(b, tMoveList[i], &undo, 0, 0, nVariant);
			if (IsSquareAttackedBy(b, GetBit(b->bbPieces[WHITE][PIECE_KING-1]), BLACK)) {
				UnMakeMove(b, tMoveList[i], &undo, 0, nVariant);
			} else {
				UnMakeMove(b, tMoveList[i], &undo, 0, nVariant);
				return true;
			}
		}
		nNumMoves = GenerateWhiteKingNonCaps(b, tMoveList, nVariant);
		for (int i=0;i<nNumMoves;i++) {
			MakeMove2(b, tMoveList[i], &undo, 0, 0, nVariant);
			if (IsSquareAttackedBy(b, GetBit(b->bbPieces[WHITE][PIECE_KING-1]), BLACK)) {
				UnMakeMove(b, tMoveList[i], &undo, 0, nVariant);
			} else {
				UnMakeMove(b, tMoveList[i], &undo, 0, nVariant);
				return true;
			}
		}
		nNumMoves = GenerateWhiteKingCaps(b, tMoveList);
		for (int i=0;i<nNumMoves;i++) {
			MakeMove2(b, tMoveList[i], &undo, 0, 0, nVariant);
			if (IsSquareAttackedBy(b, GetBit(b->bbPieces[WHITE][PIECE_KING-1]), BLACK)) {
				UnMakeMove(b, tMoveList[i], &undo, 0, nVariant);
			} else {
				UnMakeMove(b, tMoveList[i], &undo, 0, nVariant);
				return true;
			}
		}
		nNumMoves = GenerateWhiteRookQueenNonCaps(b, tMoveList);
		for (int i=0;i<nNumMoves;i++) {
			MakeMove2(b, tMoveList[i], &undo, 0, 0, nVariant);
			if (IsSquareAttackedBy(b, GetBit(b->bbPieces[WHITE][PIECE_KING-1]), BLACK)) {
				UnMakeMove(b, tMoveList[i], &undo, 0, nVariant);
			} else {
				UnMakeMove(b, tMoveList[i], &undo, 0, nVariant);
				return true;
			}
		}
		nNumMoves = GenerateWhiteRookQueenCaps(b, tMoveList);
		for (int i=0;i<nNumMoves;i++) {
			MakeMove2(b, tMoveList[i], &undo, 0, 0, nVariant);
			if (IsSquareAttackedBy(b, GetBit(b->bbPieces[WHITE][PIECE_KING-1]), BLACK)) {
				UnMakeMove(b, tMoveList[i], &undo, 0, nVariant);
			} else {
				UnMakeMove(b, tMoveList[i], &undo, 0, nVariant);
				return true;
			}
		}
		nNumMoves = GenerateWhiteBishopQueenNonCaps(b, tMoveList);
		for (int i=0;i<nNumMoves;i++) {
			MakeMove2(b, tMoveList[i], &undo, 0, 0, nVariant);
			if (IsSquareAttackedBy(b, GetBit(b->bbPieces[WHITE][PIECE_KING-1]), BLACK)) {
				UnMakeMove(b, tMoveList[i], &undo, 0, nVariant);
			} else {
				UnMakeMove(b, tMoveList[i], &undo, 0, nVariant);
				return true;
			}
		}
		nNumMoves = GenerateWhiteBishopQueenCaps(b, tMoveList);
		for (int i=0;i<nNumMoves;i++) {
			MakeMove2(b, tMoveList[i], &undo, 0, 0, nVariant);
			if (IsSquareAttackedBy(b, GetBit(b->bbPieces[WHITE][PIECE_KING-1]), BLACK)) {
				UnMakeMove(b, tMoveList[i], &undo, 0, nVariant);
			} else {
				UnMakeMove(b, tMoveList[i], &undo, 0, nVariant);
				return true;
			}
		}
	} else {
		nNumMoves = GenerateBlackPawnNonCaps(b, tMoveList);
		for (int i=0;i<nNumMoves;i++) {
			MakeMove2(b, tMoveList[i], &undo, 0, 0, nVariant);
			if (IsSquareAttackedBy(b, GetBit(b->bbPieces[BLACK][PIECE_KING-1]), WHITE)) {
				UnMakeMove(b, tMoveList[i], &undo, 0, nVariant);
			} else {
				UnMakeMove(b, tMoveList[i], &undo, 0, nVariant);
				return true;
			}
		}
		nNumMoves = GenerateBlackPawnCaps(b, tMoveList);
		for (int i=0;i<nNumMoves;i++) {
			MakeMove2(b, tMoveList[i], &undo, 0, 0, nVariant);
			if (IsSquareAttackedBy(b, GetBit(b->bbPieces[BLACK][PIECE_KING-1]), WHITE)) {
				UnMakeMove(b, tMoveList[i], &undo, 0, nVariant);
			} else {
				UnMakeMove(b, tMoveList[i], &undo, 0, nVariant);
				return true;
			}
		}
		nNumMoves = GenerateBlackKnightNonCaps(b, tMoveList);
		for (int i=0;i<nNumMoves;i++) {
			MakeMove2(b, tMoveList[i], &undo, 0, 0, nVariant);
			if (IsSquareAttackedBy(b, GetBit(b->bbPieces[BLACK][PIECE_KING-1]), WHITE)) {
				UnMakeMove(b, tMoveList[i], &undo, 0, nVariant);
			} else {
				UnMakeMove(b, tMoveList[i], &undo, 0, nVariant);
				return true;
			}
		}
		nNumMoves = GenerateBlackKnightCaps(b, tMoveList);
		for (int i=0;i<nNumMoves;i++) {
			MakeMove2(b, tMoveList[i], &undo, 0, 0, nVariant);
			if (IsSquareAttackedBy(b, GetBit(b->bbPieces[BLACK][PIECE_KING-1]), WHITE)) {
				UnMakeMove(b, tMoveList[i], &undo, 0, nVariant);
			} else {
				UnMakeMove(b, tMoveList[i], &undo, 0, nVariant);
				return true;
			}
		}
		nNumMoves = GenerateBlackKingNonCaps(b, tMoveList, nVariant);
		for (int i=0;i<nNumMoves;i++) {
			MakeMove2(b, tMoveList[i], &undo, 0, 0, nVariant);
			if (IsSquareAttackedBy(b, GetBit(b->bbPieces[BLACK][PIECE_KING-1]), WHITE)) {
				UnMakeMove(b, tMoveList[i], &undo, 0, nVariant);
			} else {
				UnMakeMove(b, tMoveList[i], &undo, 0, nVariant);
				return true;
			}
		}
		nNumMoves = GenerateBlackKingCaps(b, tMoveList);
		for (int i=0;i<nNumMoves;i++) {
			MakeMove2(b, tMoveList[i], &undo, 0, 0, nVariant);
			if (IsSquareAttackedBy(b, GetBit(b->bbPieces[BLACK][PIECE_KING-1]), WHITE)) {
				UnMakeMove(b, tMoveList[i], &undo, 0, nVariant);
			} else {
				UnMakeMove(b, tMoveList[i], &undo, 0, nVariant);
				return true;
			}
		}
		nNumMoves = GenerateBlackRookQueenNonCaps(b, tMoveList);
		for (int i=0;i<nNumMoves;i++) {
			MakeMove2(b, tMoveList[i], &undo, 0, 0, nVariant);
			if (IsSquareAttackedBy(b, GetBit(b->bbPieces[BLACK][PIECE_KING-1]), WHITE)) {
				UnMakeMove(b, tMoveList[i], &undo, 0, nVariant);
			} else {
				UnMakeMove(b, tMoveList[i], &undo, 0, nVariant);
				return true;
			}
		}
		nNumMoves = GenerateBlackRookQueenCaps(b, tMoveList);
		for (int i=0;i<nNumMoves;i++) {
			MakeMove2(b, tMoveList[i], &undo, 0, 0, nVariant);
			if (IsSquareAttackedBy(b, GetBit(b->bbPieces[BLACK][PIECE_KING-1]), WHITE)) {
				UnMakeMove(b, tMoveList[i], &undo, 0, nVariant);
			} else {
				UnMakeMove(b, tMoveList[i], &undo, 0, nVariant);
				return true;
			}
		}
		nNumMoves = GenerateBlackBishopQueenNonCaps(b, tMoveList);
		for (int i=0;i<nNumMoves;i++) {
			MakeMove2(b, tMoveList[i], &undo, 0, 0, nVariant);
			if (IsSquareAttackedBy(b, GetBit(b->bbPieces[BLACK][PIECE_KING-1]), WHITE)) {
				UnMakeMove(b, tMoveList[i], &undo, 0, nVariant);
			} else {
				UnMakeMove(b, tMoveList[i], &undo, 0, nVariant);
				return true;
			}
		}
		nNumMoves = GenerateBlackBishopQueenCaps(b, tMoveList);
		for (int i=0;i<nNumMoves;i++) {
			MakeMove2(b, tMoveList[i], &undo, 0, 0, nVariant);
			if (IsSquareAttackedBy(b, GetBit(b->bbPieces[BLACK][PIECE_KING-1]), WHITE)) {
				UnMakeMove(b, tMoveList[i], &undo, 0, nVariant);
			} else {
				UnMakeMove(b, tMoveList[i], &undo, 0, nVariant);
				return true;
			}
		}
	}

	return false;
}

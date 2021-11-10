#include "StdAfx.h"
#include <stdio.h>
#include <conio.h>
#include "Common.h"
#include "Board.h"
#include <math.h>
//#include "LittleThought.h"
//#include "Utils.h"
#include "Hash.h"
//#include "Eval.h"
//#include "fast_memcpy.h"

BitBoard bit[64];					// Single bit BitBoards
BitBoard g_bbPawnAttacks[2][64];	// [COLOR][SQUARE] Stores the squares attacked by a pawn at the indexed square.
BitBoard g_bbWhitePawnAttacks[64];	// Stores the squares attacked by a white pawn at the indexed square.
BitBoard g_bbBlackPawnAttacks[64];	// Stores the squares attacked by a black pawn at the indexed square.
BitBoard g_nnWhiteEPAttacks[64];	// Stores the squares attacked by EnPassent by a white pawn at the indexed square.
BitBoard g_nnBlackEPAttacks[64];	// Stores the squares attacked by EnPassent by a black pawn at the indexed square.
BitBoard g_bbKnightAttacks[64];		// Stores the squares attacked by a knight at the indexed square.
BitBoard g_bbKingAttacks[64];		// Stores the squares attacked by a king at the indexed square.
BitBoard g_bbRankAttacks[64][256];	// Attacked squares for each square and possible combination of pieces on files of that square's rank.
BitBoard g_bbFileAttacks[64][256];	// Attacked squares for each square and possible combination of pieces on ranks of that square's file.
BitBoard g_bbDiagA8H1Attacks[64][256]; // Attacked squares for each square and possible combination of pieces on A8H1 diagonals of that square.
BitBoard g_bbDiagH8A1Attacks[64][256]; // Attacked squares for each square and possible combination of pieces on H8A1 diagonals of that square.
int g_nR45LMask[64];				// Required to mask the shifted, rotated boards as each diagonal is a different length
int g_nR45RMask[64];				// Required to mask the shifted, rotated boards as each diagonal is a different length
int g_nPieceValues[23];				// Values of color|piece
//BitBoard g_bbCastles[64];			// Update castling rights
int g_nDirections[64][64];			// Gives the direction (as a single square offset) of [from][to].
BitBoard g_bbDir8Attacks[64];		// Ray of all bits from [sq] up
BitBoard g_bbDir7Attacks[64];		// Ray of all bits from [sq] up-right
BitBoard g_bbDirM1Attacks[64];		// Ray of all bits from [sq] right
BitBoard g_bbDirM9Attacks[64];		// Ray of all bits from [sq] down-right
BitBoard g_bbDirM8Attacks[64];		// Ray of all bits from [sq] down
BitBoard g_bbDirM7Attacks[64];		// Ray of all bits from [sq] down-left
BitBoard g_bbDir1Attacks[64];		// Ray of all bits from [sq] left
BitBoard g_bbDir9Attacks[64];		// Ray of all bits from [sq] up-left
BitBoard g_bbBetween[64][64];		// All bits between [from] and [to] are set
BitBoard g_bbMaskIsoPawns[8];		// Bits are set either side of the specified file
int g_nDistance[64][64];			// The distance between 2 squares
BitBoard g_bbPassedPawnMask[2][64];	// All bits are set in neighbouring files in front of the specified square
#ifdef CACHE_MOVES
	TMovesCache g_tMovesCache[64];
#endif
// An array that stores the 64-bit hash value for each board played.
// The first part of the array stores the fixed positions that have been played in the game.
// The rest of the array is a FILO stack which stores the positions as they are encountered
// in the search tree. If three repetitions of the same position are found with the same side
// to move, the position is a draw.
U64 g_nRepetitionHashes[600][MAX_THREADS];
int g_nRepetitionListHead[MAX_THREADS];
int g_nGameHalfMoveNum[MAX_THREADS];

void SetBitBoards(TBoard *b);

int GetDistance(int a, int b) {
// Return distance of square a from b. Distance = max(rank diff, file diff)
// TODO: precalculate and put in array [64][64]?
	int dx, dy;
	dx = abs((a % 8) - (b % 8));
	dy = abs((a / 8) - (b / 8));
	if (dx<dy) {
		return dy;
	} else {
		return dx;
	}
}

void InitialiseArrays() {

	/*for (int i=0;i<600;i++) {
		g_nRepetitionHashes[i] = (U64*)malloc(1000);
	}*/

	int i, j, k;
	BitBoard bb = 1;

	for (i=0;i<64;i++) {
		bit[i] = bb;
		bb <<= 1;
	}

	// g_bbCastles array
	//memset(g_bbCastles, 0, sizeof(g_bbCastles[0])*64);
	//g_bbCastles[0] = WHITE_KING_CASTLE;
	//g_bbCastles[7] = WHITE_QUEEN_CASTLE;
	//g_bbCastles[56] = BLACK_KING_CASTLE;
	//g_bbCastles[63] = BLACK_QUEEN_CASTLE;

	// Initialise Attack boards

	// g_bbPawnAttacks[WHITE] array
	for (i=0;i<64;i++) {
		g_bbPawnAttacks[WHITE][i] = 0;
		if (FILE(i) < 7) g_bbPawnAttacks[WHITE][i] |= bit[i+9];
		if (FILE(i) > 0) g_bbPawnAttacks[WHITE][i] |= bit[i+7];
	}

	// g_bbPawnAttacks[BLACK] array
	for (i=0;i<64;i++) {
		g_bbPawnAttacks[BLACK][i] = 0;
		if (FILE(i) < 7) g_bbPawnAttacks[BLACK][i] |= bit[i-7];
		if (FILE(i) > 0) g_bbPawnAttacks[BLACK][i] |= bit[i-9];
	}

	// g_bbWhitePawnAttacks array
	for (i=0;i<64;i++) {
		g_bbWhitePawnAttacks[i] = 0;
		if (FILE(i) < 7) g_bbWhitePawnAttacks[i] |= bit[i+9];
		if (FILE(i) > 0) g_bbWhitePawnAttacks[i] |= bit[i+7];
	}

	// g_bbBlackPawnAttacks array
	for (i=0;i<64;i++) {
		g_bbBlackPawnAttacks[i] = 0;
		if (FILE(i) < 7) g_bbBlackPawnAttacks[i] |= bit[i-7];
		if (FILE(i) > 0) g_bbBlackPawnAttacks[i] |= bit[i-9];
	}

	// g_nnWhiteEPAttacks array
	for (i=0;i<64;i++) {
		g_nnWhiteEPAttacks[i] = 0;
		if (i>=H6 && i<=B6) g_nnWhiteEPAttacks[i] |= bit[i-7];
		if (i>=G6 && i<=A6) g_nnWhiteEPAttacks[i] |= bit[i-9];
	}

	// g_nnBlackEPAttacks array
	for (i=0;i<64;i++) {
		g_nnBlackEPAttacks[i] = 0;
		if (i>=G3 && i<=A3) g_nnBlackEPAttacks[i] |= bit[i+7];
		if (i>=H3 && i<=B3) g_nnBlackEPAttacks[i] |= bit[i+9];
	}

	// g_bbKnightAttacks array
	int KnightOffsets[8] = {-17,-15,-6,10,17,15,6,-10};
	for (i=0;i<64;i++) {
		g_bbKnightAttacks[i] = 0;
		for (j=0;j<8;j++) {
			if ((i+KnightOffsets[j]>=0) && (i+KnightOffsets[j]<64) && (GetDistance(i, i+KnightOffsets[j]) < 3)) {
				g_bbKnightAttacks[i] |= bit[i+KnightOffsets[j]];
			}
		}
	}

	// g_bbKingAttacks array
	int KingOffsets[8] = {9,8,7,1,-1,-7,-8,-9};
	for (i=0;i<64;i++) {
		g_bbKingAttacks[i] = 0;
		for (j=0;j<8;j++) {
			if ((i+KingOffsets[j]>=0) && (i+KingOffsets[j]<64) && (GetDistance(i, i+KingOffsets[j]) <= 1)) {
				g_bbKingAttacks[i] |= bit[i+KingOffsets[j]];
			}
		}
	}

	// g_bbRankAttacks array
	int occupied[8];
	int file, rank;
	for (i=0;i<64;i++) {
		file = FILE(i);
		rank = RANK(i);
		// j is an 8-bit number where each bit represents a file - if it is set, the file is occupied.
		for (j=0;j<256;j++) { // 256 possible combinations of occupied squares on a rank
			for (k=0;k<8;k++) occupied[k] = j & (1<<k);	// For a given j, calculate which files will be occupied
			g_bbRankAttacks[i][j] = 0;					// Ignore the from square
			for (k=file+1;k<8;k++) {					// look left until blocked
				g_bbRankAttacks[i][j] |= bit[k+8*rank];	// Set this bit as it is free to attack (or first occupied square in this direction)
				if (occupied[k]) break;					// no more squares to attack
			}
			for (k=file-1;k>=0;k--) {					// look right until blocked
				g_bbRankAttacks[i][j] |= bit[k+8*rank];	// Set this bit as it is free to attack (or first occupied square in this direction)
				if (occupied[k]) break;					// no more squares to attack
			}
		}
	}

	// g_bbFileAttacks array
	for (i=0;i<64;i++) {
		file = FILE(i);
		rank = RANK(i);
		// j is an 8-bit number where each bit represents a rank - if it is set, the rank is occupied.
		for (j=0;j<256;j++) { // 256 possible combinations of occupied squares on a file
			for (k=0;k<8;k++) occupied[7-k] = j & (1<<k);	// For a given j, calculate which files will be occupied
			g_bbFileAttacks[i][j] = 0;					// Ignore the from square
			for (k=rank+1;k<8;k++) {					// look up until blocked
				g_bbFileAttacks[i][j] |= bit[8*k+file];	// Set this bit as it is free to attack (or first occupied square in this direction)
				if (occupied[k]) break;					// no more squares to attack
			}
			for (k=rank-1;k>=0;k--) {					// look down until blocked
				g_bbFileAttacks[i][j] |= bit[8*k+file];	// Set this bit as it is free to attack (or first occupied square in this direction)
				if (occupied[k]) break;					// no more squares to attack
			}
		}
	}

	// g_bbDiagA8H1Attacks array
	int dist;
	for (i=0;i<64;i++) {
		file = FILE(i);
		rank = RANK(i);
		// Each diagonal is a different length
		for (j=0;j<pow(2.0,g_nLengthA8H1diagonals[i]);j++) {
			for (k=0;k<g_nLengthA8H1diagonals[i];k++) {
				occupied[g_nLengthA8H1diagonals[i]-1-k] = j & (1<<k); // non-zero if a piece occupies position k
			}
			g_bbDiagA8H1Attacks[i][j] = 0; // Set the attacking square
			if (rank < file) {
				dist = rank;
			} else {
				dist = file;
			}
			for (k=dist+1;k<g_nLengthA8H1diagonals[i];k++) { // look up-left until blocked
				g_bbDiagA8H1Attacks[i][j] |= bit[i + (k-dist)*9];
				if (occupied[k]) break; // no more squares to attack
			}
			for (k=dist-1;k>=0;k--) { // look down-right until blocked
				g_bbDiagA8H1Attacks[i][j] |= bit[i - (dist-k)*9];
				if (occupied[k]) break; // no more squares to attack
			}
		}
	}

	// g_bbDiagH8A1Attacks array
	for (i=0;i<64;i++) {
		file = FILE(i);
		rank = RANK(i);
		// Each diagonal is a different length
		for (j=0;j<pow(2.0,g_nLengthH8A1diagonals[i]);j++) {
			for (k=0;k<g_nLengthH8A1diagonals[i];k++) {
				occupied[k] = j & (1<<k); // non-zero if a piece occupies position k
			}
			g_bbDiagH8A1Attacks[i][j] = 0; // Set the attacking square
			if (rank < 7-file) {
				dist = rank;
			} else {
				dist = 7-file;
			}
			for (k=dist+1;k<g_nLengthH8A1diagonals[i];k++) { // look up-right until blocked
				g_bbDiagH8A1Attacks[i][j] |= bit[i + (k-dist)*7];
				if (occupied[k]) break; // no more squares to attack
			}
			for (k=dist-1;k>=0;k--) { // look down-left until blocked
				g_bbDiagH8A1Attacks[i][j] |= bit[i - (dist-k)*7];
				if (occupied[k]) break; // no more squares to attack
			}
		}
	}

	// g_nR45LMask array
	for (i=0;i<64;i++) {
		g_nR45LMask[i] = (int)pow(2.0,g_nLengthA8H1diagonals[i]) - 1;
	}

	// g_nR45RMask array
	for (i=0;i<64;i++) {
		g_nR45RMask[i] = (int)pow(2.0,g_nLengthH8A1diagonals[i]) - 1;
	}

	// g_nDirections array (used for SEE)
	memset(g_nDirections, 0, 64*64*sizeof(int));
	for (i=0;i<64;i++) {
		for (j=RANK(i)+1,k=i+8;j<8;j++,k+=8)	g_nDirections[i][k] = 8;	// up
		for (j=FILE(i)-1,k=i-1;j>=0;j--,k--)	g_nDirections[i][k] = -1;	// right
		for (j=RANK(i)-1,k=i-8;j>=0;j--,k-=8)	g_nDirections[i][k] = -8;	// down
		for (j=FILE(i)+1,k=i+1;j<8;j++,k++)		g_nDirections[i][k] = 1;	// left

		for (j=MIN(7-RANK(i),FILE(i))-1,k=i+7;j>=0;j--,k+=7)	g_nDirections[i][k] = 7;	// up-right
		for (j=MIN(RANK(i),FILE(i))-1,k=i-9;j>=0;j--,k-=9)		g_nDirections[i][k] = -9;	// down-right
		for (j=MIN(RANK(i),7-FILE(i))-1,k=i-7;j>=0;j--,k-=7)	g_nDirections[i][k] = -7;	// down-left
		for (j=MIN(7-RANK(i),7-FILE(i))-1,k=i+9;j>=0;j--,k+=9)	g_nDirections[i][k] = 9;	// up-left
	}

	// g_bbDirAttacks arrays
	memset(g_bbDir1Attacks, 0, 64*sizeof(BitBoard));
	memset(g_bbDir7Attacks, 0, 64*sizeof(BitBoard));
	memset(g_bbDir8Attacks, 0, 64*sizeof(BitBoard));
	memset(g_bbDir9Attacks, 0, 64*sizeof(BitBoard));
	memset(g_bbDirM1Attacks, 0, 64*sizeof(BitBoard));
	memset(g_bbDirM7Attacks, 0, 64*sizeof(BitBoard));
	memset(g_bbDirM8Attacks, 0, 64*sizeof(BitBoard));
	memset(g_bbDirM9Attacks, 0, 64*sizeof(BitBoard));
	for (i=0;i<64;i++) {
		for (j=RANK(i)+1;j<8;j++)	g_bbDir8Attacks[i]  |= bit[FILE(i)+8*j];
		for (j=FILE(i)-1;j>=0;j--)	g_bbDirM1Attacks[i] |= bit[8*RANK(i)+j];
		for (j=RANK(i)-1;j>=0;j--)	g_bbDirM8Attacks[i] |= bit[FILE(i)+8*j];
		for (j=FILE(i)+1;j<8;j++)	g_bbDir1Attacks[i]  |= bit[8*RANK(i)+j];
		for (j=MIN(7-RANK(i),FILE(i));j>0;j--)		g_bbDir7Attacks[i]  |= bit[i+7*j];
		for (j=MIN(RANK(i),FILE(i));j>0;j--)		g_bbDirM9Attacks[i] |= bit[i-9*j];
		for (j=MIN(RANK(i),7-FILE(i));j>0;j--)		g_bbDirM7Attacks[i] |= bit[i-7*j];
		for (j=MIN(7-RANK(i),7-FILE(i));j>0;j--)	g_bbDir9Attacks[i]  |= bit[i+9*j];
	}

	int sq;
	for (i=0;i<64;i++) {
		for (j=0;j<64;j++) {
			g_bbBetween[i][j] = -1; // Set all invalid cases to all ones
		}
		// for this from square (i), follow each direction ray and mark the to squares (sq)
		bb = g_bbDir1Attacks[i];
		while (bb) {
			sq = GetBit(bb);
			bb ^= bit[sq];
			g_bbBetween[i][sq] = g_bbDir1Attacks[i] ^ g_bbDir1Attacks[sq-1];
		}
		bb = g_bbDirM1Attacks[i];
		while (bb) {
			sq = GetBit(bb);
			bb ^= bit[sq];
			g_bbBetween[i][sq] = g_bbDirM1Attacks[i] ^ g_bbDirM1Attacks[sq+1];
		}
		bb = g_bbDir8Attacks[i];
		while (bb) {
			sq = GetBit(bb);
			bb ^= bit[sq];
			g_bbBetween[i][sq] = g_bbDir8Attacks[i] ^ g_bbDir8Attacks[sq-8];
		}
		bb = g_bbDirM8Attacks[i];
		while (bb) {
			sq = GetBit(bb);
			bb ^= bit[sq];
			g_bbBetween[i][sq] = g_bbDirM8Attacks[i] ^ g_bbDirM8Attacks[sq+8];
		}
		bb = g_bbDir7Attacks[i];
		while (bb) {
			sq = GetBit(bb);
			bb ^= bit[sq];
			g_bbBetween[i][sq] = g_bbDir7Attacks[i] ^ g_bbDir7Attacks[sq-7];
		}
		bb = g_bbDirM7Attacks[i];
		while (bb) {
			sq = GetBit(bb);
			bb ^= bit[sq];
			g_bbBetween[i][sq] = g_bbDirM7Attacks[i] ^ g_bbDirM7Attacks[sq+7];
		}
		bb = g_bbDir9Attacks[i];
		while (bb) {
			sq = GetBit(bb);
			bb ^= bit[sq];
			g_bbBetween[i][sq] = g_bbDir9Attacks[i] ^ g_bbDir9Attacks[sq-9];
		}
		bb = g_bbDirM9Attacks[i];
		while (bb) {
			sq = GetBit(bb);
			bb ^= bit[sq];
			g_bbBetween[i][sq] = g_bbDirM9Attacks[i] ^ g_bbDirM9Attacks[sq+9];
		}
	}

	// g_bbMaskIsoPawns array - Bits set either side of the referenced column
	g_bbMaskIsoPawns[0] = g_bbMaskFile[1];
	g_bbMaskIsoPawns[7] = g_bbMaskFile[6];
	for (i=1;i<7;i++) {
		g_bbMaskIsoPawns[i] = g_bbMaskFile[i-1] | g_bbMaskFile[i+1];
	}

	int dx, dy;
	for (i=0;i<64;i++) {
		for (j=0;j<64;j++) {
			dx = abs((i % 8) - (j % 8));
			dy = abs((i / 8) - (j / 8));
			if (dx<dy) {
				g_nDistance[i][j] =  dy;
			} else {
				g_nDistance[i][j] =  dx;
			}
		}
	}


	// Fill the random number tables for hashing
	for (i=0;i<64;i++) {
		// Setup empty squares with zero
		//g_nHashValues[i][SQUARE_EMPTY] = 0;
		g_nHashValues[i][WHITE][0] = 0;		// v1.04.13
		//g_nHashValues[i][BLACK][0] = 0;		// v1.04.13

		//for (j=1;j<23;j++) {
		//	// Create the random number table
		//	g_nHashValues[i][j] = GetRandomNum();
		//}

		// v1.04.13
		for (j=1;j<=6;j++) {
			// Create the random number table
			g_nHashValues[i][WHITE][j] = GetRandomNum();
			g_nHashValues[i][BLACK][j] = GetRandomNum();
		}
	}

	// Other random numbers
	g_nHashWhiteKingCastle = GetRandomNum();
	g_nHashWhiteQueenCastle = GetRandomNum();
	g_nHashBlackKingCastle = GetRandomNum();
	g_nHashBlackQueenCastle = GetRandomNum();
	g_nHashWhiteToMove = GetRandomNum();
	for (i=0;i<64;i++)
		g_nHashEPMove[i] = GetRandomNum();
	g_nHash50Move[0] = 0;
	for (i=1;i<100;i++)
		g_nHash50Move[i] = GetRandomNum();


}


void ZeroBoard(TBoard *b) {
// Erases all contents of the board.
	memset(b,0,sizeof(*b));
}

void CreateStartingPosition(TBoard *b, int nVariant) {
// This function sets the board to the starting position.
	int i;

	InitialiseArrays();
	ZeroBoard(b);

	for (i=8;i<=15;i++) {
		b->nPieces[i] =	PIECE_WHITE | PIECE_PAWN;
	}
	for (i=48;i<=55;i++) {
		b->nPieces[i] =	PIECE_BLACK | PIECE_PAWN;
	}
	if (nVariant == VARIANT_STD) {
		b->nPieces[7] =		PIECE_WHITE | PIECE_ROOK;
		b->nPieces[6] =		PIECE_WHITE | PIECE_KNIGHT;
		b->nPieces[5] =		PIECE_WHITE | PIECE_BISHOP;
		b->nPieces[4] =		PIECE_WHITE | PIECE_QUEEN;
		b->nPieces[3] =		PIECE_WHITE | PIECE_KING;
		b->nPieces[2] =		PIECE_WHITE | PIECE_BISHOP;
		b->nPieces[1] =		PIECE_WHITE | PIECE_KNIGHT;
		b->nPieces[0] =		PIECE_WHITE | PIECE_ROOK;

		b->nPieces[63] =	PIECE_BLACK | PIECE_ROOK;
		b->nPieces[62] =	PIECE_BLACK | PIECE_KNIGHT;
		b->nPieces[61] =	PIECE_BLACK | PIECE_BISHOP;
		b->nPieces[60] =	PIECE_BLACK | PIECE_QUEEN;
		b->nPieces[59] =	PIECE_BLACK | PIECE_KING;
		b->nPieces[58] =	PIECE_BLACK | PIECE_BISHOP;
		b->nPieces[57] =	PIECE_BLACK | PIECE_KNIGHT;
		b->nPieces[56] =	PIECE_BLACK | PIECE_ROOK;

	} else if (nVariant == VARIANT_960) {
		// Bodlaender's method
		srand(time(NULL));
		int r,c;
		r = rand() % 4;
		b->nPieces[A1-2*r] = PIECE_BISHOP;
		r = rand() % 4;
		b->nPieces[B1-2*r] = PIECE_BISHOP;
		r = rand() % 6;
		for (int s=A1,c=0;s>=H1;s--) {
			if (!b->nPieces[s]) {
				if (c==r) { b->nPieces[s] = PIECE_QUEEN; break; }	// r'th empty position from left
				else c++;
			}
		}
		r = rand() % 5;
		for (int s=A1,c=0;s>=H1;s--) {
			if (!b->nPieces[s]) {
				if (c==r) { b->nPieces[s] = PIECE_KNIGHT; break; }	// r'th empty position from left
				else c++;
			}
		}
		r = rand() % 4;
		for (int s=A1,c=0;s>=H1;s--) {
			if (!b->nPieces[s]) {
				if (c==r) { b->nPieces[s] = PIECE_KNIGHT; break; }	// r'th empty position from left
				else c++;
			}
		}
		for (int s=A1;s>=H1;s--) if (!b->nPieces[s]) { b->nPieces[s] = PIECE_ROOK; break; }
		for (int s=A1;s>=H1;s--) if (!b->nPieces[s]) { b->nPieces[s] = PIECE_KING; break; }
		for (int s=A1;s>=H1;s--) if (!b->nPieces[s]) { b->nPieces[s] = PIECE_ROOK; break; }

		for (int s=A1;s>=H1;s--) b->nPieces[s]    = b->nPieces[s] | PIECE_WHITE;
		for (int s=A1;s>=H1;s--) b->nPieces[56+s] = (b->nPieces[s] & SQUARE_PIECE_MASK) | PIECE_BLACK;
	}
	
	for (int s=A1;s>=H1;s--) ASSERT(b->nPieces[s]);



	//b->nWhiteMaterial = 8*SCORE_PAWN + 2*SCORE_ROOK + 2*SCORE_KNIGHT + 2*SCORE_BISHOP + SCORE_QUEEN;
	//b->nBlackMaterial = 8*SCORE_PAWN + 2*SCORE_ROOK + 2*SCORE_KNIGHT + 2*SCORE_BISHOP + SCORE_QUEEN;

	b->nSideToMove = WHITE;
	b->nEPSquare = NULL_SQUARE;
	b->nFiftyMoveCount = 0;
	b->nCastling = WHITE_KING_CASTLE | BLACK_KING_CASTLE | WHITE_QUEEN_CASTLE | BLACK_QUEEN_CASTLE;

	SetBitBoards(b);

	//if (nVariant == VARIANT_960) {
		memset(b->g_bbCastles, 0, sizeof(BitBoard)*64);
		BitBoard r = b->bbPieces[WHITE][PIECE_ROOK-1];
		int r1 = GetBlackBit(r); // Left
		int r2 = GetWhiteBit(r); // Right
		b->g_bbCastles[r2] = WHITE_KING_CASTLE;
		b->g_bbCastles[r1] = WHITE_QUEEN_CASTLE;
		r = b->bbPieces[BLACK][PIECE_ROOK-1];
		r1 = GetBlackBit(r); // Left
		r2 = GetWhiteBit(r); // Right
		b->g_bbCastles[r2] = BLACK_KING_CASTLE;
		b->g_bbCastles[r1] = BLACK_QUEEN_CASTLE;
	//}

	//CalculateHashValues(b);
}

void Board2FEN(TBoard *b, char *sFEN) {
	int i=63,x,y;
	int c=0;
	int empty=0;

	for (y=0;y<8;y++) {
		for (x=0;x<8;x++) {
			if (b->nPieces[i] != SQUARE_EMPTY) {
				if (empty) {
					sFEN[c++] = 48+empty;
					empty=0;
				}
				sFEN[c++]=g_nPieceChars[b->nPieces[i]];
			} else {
				empty++;
			}
			i--;
		}
		if (empty) {
			sFEN[c++] = 48+empty;
			empty=0;
		}
		if (y<7) sFEN[c++] = '/';
	}
	sFEN[c++] = ' ';

	// Side to move
	if (b->nSideToMove==WHITE) sFEN[c++] = 'w';
	else sFEN[c++] = 'b';
	sFEN[c++] = ' ';

	// Castling
	if ((b->nCastling&0x0F) == NO_CASTLE) {
		sFEN[c++] = '-';
	} else {
		if (b->nCastling & WHITE_KING_CASTLE) sFEN[c++] = 'K';
		if (b->nCastling & WHITE_QUEEN_CASTLE) sFEN[c++] = 'Q';
		if (b->nCastling & BLACK_KING_CASTLE) sFEN[c++] = 'k';
		if (b->nCastling & BLACK_QUEEN_CASTLE) sFEN[c++] = 'q';
	}
	sFEN[c++] = ' ';

	// En Passant Target Square
	if (b->nEPSquare == NULL_SQUARE) {
		sFEN[c++] = '-';
	} else {
		sFEN[c++] = 'h' - FILE(b->nEPSquare);
		sFEN[c++] = '1' + RANK(b->nEPSquare); 
	}
	sFEN[c++] = ' ';

	// 50 Moves and Full Moves
	sprintf(&sFEN[c], "%d %d", b->nFiftyMoveCount, 0);

	//return sFEN;
}

void PrintBoard(TBoard *b, FILE *f) {
// Prints a board to the screen in ASCII format.
	int x, y;
	//TEvalInfo e;

	fprintf(f,"--Start Board-----------------------------------------------------------\n");
	fprintf(f,"bR =  0x%016I64X wR =  0x%016I64X Castling = \t0x%X\n",b->bbPieces[BLACK][PIECE_ROOK-1],b->bbPieces[WHITE][PIECE_ROOK-1],b->nCastling);
	fprintf(f,"bB =  0x%016I64X wB =  0x%016I64X EPSquare = \t%02d\n",b->bbPieces[BLACK][PIECE_BISHOP-1],b->bbPieces[WHITE][PIECE_BISHOP-1],b->nEPSquare);
	fprintf(f,"bN =  0x%016I64X wN =  0x%016I64X White Mat = \t%d\n",b->bbPieces[BLACK][PIECE_KNIGHT-1],b->bbPieces[WHITE][PIECE_KNIGHT-1],0);
	fprintf(f,"bQ =  0x%016I64X wQ =  0x%016I64X Black Mat = \t%d\n",b->bbPieces[BLACK][PIECE_QUEEN-1],b->bbPieces[WHITE][PIECE_QUEEN-1],0);
	fprintf(f,"bK =  0x%016I64X wK =  0x%016I64X 50 Moves = \t%d\n",b->bbPieces[BLACK][PIECE_KING-1],b->bbPieces[WHITE][PIECE_KING-1],b->nFiftyMoveCount);
	fprintf(f,"bP =  0x%016I64X wP =  0x%016I64X Side To Move = %c\n",b->bbPieces[BLACK][PIECE_PAWN-1],b->bbPieces[WHITE][PIECE_PAWN-1],(b->nSideToMove==WHITE ? 'W':'B'));
	fprintf(f,"bA =  0x%016I64X wA =  0x%016I64X Half Move = \t%d\n",b->bbPieces[BLACK][PIECE_ALL-1],b->bbPieces[WHITE][PIECE_ALL-1],0);
	//fprintf(f,"45L = 0x%016I64X hv =  0x%016I64X Full Move = \t%d\n",b->bbRotate45L,b->nTransHash,0);
	//fprintf(f,"45R = 0x%016I64X ph =  0x%016I64X Eval = \t%d\n",b->bbRotate45R,b->nPawnHash,0);
	//fprintf(f,"90 =  0x%016I64X rh =  0x%016I64X\n",b->bbRotate90,b->nTransHash^g_nHash50Move[b->nFiftyMoveCount]);
	char sFEN[100];
	Board2FEN(b,sFEN);
	fprintf(f,"FEN = %s\n", sFEN);

	if (f == stdout) {
		HANDLE hOut;
		hOut = GetStdHandle(STD_OUTPUT_HANDLE);

		printf("  ");
		for (x=0;x<8;x++) {
			printf(" %c",'a'+x);
		}
		printf("\n");

		int sq=0;
		int bw = BACKGROUND_RED|BACKGROUND_GREEN|BACKGROUND_BLUE|BACKGROUND_INTENSITY;
		int bb = BACKGROUND_RED|BACKGROUND_GREEN|BACKGROUND_BLUE;
		int fw = FOREGROUND_RED;
		int fb = 0;
		for (y=7;y>=0;y--) {

			printf("%c ",'1'+y);

			for (x=7;x>=0;x--,sq++) {
				if ((b->nPieces[y*8+x] & SQUARE_COLOR_MASK) == PIECE_WHITE) {
					SetConsoleTextAttribute(hOut, sq%2?fw|bb:fw|bw);
				} else {
					SetConsoleTextAttribute(hOut, sq%2?fb|bb:fb|bw);
				}
				printf(" %c",g_nPieceChars[b->nPieces[y*8+x]]);
			}
			SetConsoleTextAttribute(hOut, FOREGROUND_RED|FOREGROUND_GREEN|FOREGROUND_BLUE);

			printf(" %c ",'1'+y);
			for (x=7;x>=0;x--) {
				printf(" %02d",y*8 + x);
			}
			printf("   ");
			for (x=0;x<8;x++) {
				printf(" %c%d", 'a'+x, y+1);
			}

			sq++;
			printf("\n");
		}

		printf("  ");
		for (x=0;x<8;x++) {
			printf(" %c",'a'+x);
		}
		printf("\n");

		SetConsoleTextAttribute(hOut, FOREGROUND_RED|FOREGROUND_GREEN|FOREGROUND_BLUE);
	} else {
		fprintf(f,"  ");
		for (x=0;x<8;x++) {
			fprintf(f," %c",'a'+x);
		}
		fprintf(f,"\n   ---------------\n");

		for (y=7;y>=0;y--) {
			fprintf(f,"%c|",'1'+y);
			for (x=7;x>=0;x--) {
				fprintf(f," %c",g_nPieceChars[b->nPieces[y*8+x]]);
			}
			fprintf(f," |%c ",'1'+y);
			for (x=7;x>=0;x--) {
				fprintf(f," %02d",y*8 + x);
			}
			fprintf(f,"   ");
			for (x=0;x<8;x++) {
				fprintf(f," %c%d", 'a'+x, y+1);
			}
			fprintf(f,"\n");
		}

		
		fprintf(f,"   ---------------\n  ");
		for (x=0;x<8;x++) {
			fprintf(f," %c",'a'+x);
		}
		fprintf(f,"\n");
	}

	//fprintf(f,"FEN %s\n",GetFEN(b));
	fprintf(f,"--End Board-------------------------------------------------------------\n");
}

int IsSquareAttackedBy(TBoard *b, int sq, int color) {
/* 
Checks if the square sq is being attacked by a piece of color color and returns the first piece
if finds that is attacking it, or 0 if the square is not being attacked. Note that this does not
check for EnPassent (but it could be made to if required?).
IN:		sq		The square to check for attacks at.
		color	Check for attacks by this color (WHITE or BLACK).
OUT:	int		The type of a piece found to be attacking the square.
*/
	// TODO: is this function redundant now?
	// Pawn attacks
	if (b->bbPieces[color][PIECE_PAWN-1] & g_bbPawnAttacks[OPP(color)][sq]) return PIECE_PAWN;

	// Knight attacks
	if (b->bbPieces[color][PIECE_KNIGHT-1] & g_bbKnightAttacks[sq]) return PIECE_KNIGHT;

	// King attacks
	if (b->bbPieces[color][PIECE_KING-1] & g_bbKingAttacks[sq]) return PIECE_KING;

	// Bishop attacks
	if ((b->bbPieces[color][PIECE_BISHOP-1]|b->bbPieces[color][PIECE_QUEEN-1]) & BISHOP_MOVES(sq)) return PIECE_BISHOP;

	// Rook attacks
	if ((b->bbPieces[color][PIECE_ROOK-1]|b->bbPieces[color][PIECE_QUEEN-1]) & ROOK_MOVES(sq)) return PIECE_ROOK;

	return 0;
}

int IsSTMInCheck(TBoard *b) {
/*
Checks if the side to move is in check. Returns (one of) the piece giving check.
*/
	if (b->nSideToMove == WHITE) {
		return IsSquareAttackedBy(b, GetBit(b->bbPieces[WHITE][PIECE_KING-1]), BLACK);
	} else {
		return IsSquareAttackedBy(b, GetBit(b->bbPieces[BLACK][PIECE_KING-1]), WHITE);
	}
}

int CountSquareAttackedBy(TBoard *b, int sq, int color, int *nNumAttacks) {
/* 
Checks if the square sq is being attacked by a piece of color color and returns the square of the
first piece if finds that is attacking it, or -1 if the square is not being attacked. It also returns
the number of pieces attacking the square in *nNumAttacks.
Note that this does not check for EnPassent (but it could be made to if required?) or King attacks (as
its only used for check evasions so sq is occupied by the King?)
IN:		sq		The square to check for attacks at.
		color	Check for attacks by this color (WHITE or BLACK).
OUT:	int		The square attacking sq
		nNumAttacks	The number of attacking pieces
*/
	BitBoard bb;
	int nFrom = -1;
	*nNumAttacks = 0;

	ASSERT((b->nPieces[sq]&SQUARE_PIECE_MASK) == PIECE_KING);

	if (color == WHITE) {
		bb =  (b->bbPieces[WHITE][PIECE_PAWN-1] & g_bbBlackPawnAttacks[sq])
			| (b->bbPieces[WHITE][PIECE_KNIGHT-1] & g_bbKnightAttacks[sq])
			| ((b->bbPieces[WHITE][PIECE_BISHOP-1]|b->bbPieces[WHITE][PIECE_QUEEN-1]) & BISHOP_MOVES(sq))
			| ((b->bbPieces[WHITE][PIECE_ROOK-1]|b->bbPieces[WHITE][PIECE_QUEEN-1]) & ROOK_MOVES(sq));

		if (bb) {
			*nNumAttacks += CountBits(bb);
			nFrom = GetBit(bb);
		}
	} else {
		bb =  (b->bbPieces[BLACK][PIECE_PAWN-1] & g_bbWhitePawnAttacks[sq])
			| (b->bbPieces[BLACK][PIECE_KNIGHT-1] & g_bbKnightAttacks[sq])
			| ((b->bbPieces[BLACK][PIECE_BISHOP-1]|b->bbPieces[BLACK][PIECE_QUEEN-1]) & BISHOP_MOVES(sq))
			| ((b->bbPieces[BLACK][PIECE_ROOK-1]|b->bbPieces[BLACK][PIECE_QUEEN-1]) & ROOK_MOVES(sq));

		if (bb) {
			*nNumAttacks += CountBits(bb);
			nFrom = GetBit(bb);
		}
	}

	ASSERT(nFrom > -1);

	return nFrom;
}

BitBoard SquareAttacks(TBoard *b, int sq) {
/* Returns a bit board of all occupied squares attacking sq (not including EP).
IN:		b			The board to check
		sq			The square to check
OUT:	bbAttacks	The resulting bitboard
ASSUMES: None
*/
	BitBoard bbAttacks;

	// Must include King attacks as its a valid capturing piece, as long as it doesnt put
	// itself in check.
	bbAttacks = g_bbKingAttacks[sq] & (b->bbPieces[BLACK][PIECE_KING-1] | b->bbPieces[WHITE][PIECE_KING-1]);
	bbAttacks |= g_bbKnightAttacks[sq] & (b->bbPieces[BLACK][PIECE_KNIGHT-1] | b->bbPieces[WHITE][PIECE_KNIGHT-1]);
	bbAttacks |= BISHOP_MOVES(sq) & ((b->bbPieces[BLACK][PIECE_BISHOP-1]|b->bbPieces[BLACK][PIECE_QUEEN-1]) | (b->bbPieces[WHITE][PIECE_BISHOP-1]|b->bbPieces[WHITE][PIECE_QUEEN-1]));
	bbAttacks |= ROOK_MOVES(sq) & ((b->bbPieces[BLACK][PIECE_ROOK-1]|b->bbPieces[BLACK][PIECE_QUEEN-1]) | (b->bbPieces[WHITE][PIECE_ROOK-1]|b->bbPieces[WHITE][PIECE_QUEEN-1]));
	if (sq>A2) {
		bbAttacks |= bit[sq-7] & b->bbPieces[WHITE][PIECE_PAWN-1] & g_bbMaskNotFileH;
		bbAttacks |= bit[sq-9] & b->bbPieces[WHITE][PIECE_PAWN-1] & g_bbMaskNotFileA;
	}
	if (sq<H7) {
		bbAttacks |= bit[sq+9] & b->bbPieces[BLACK][PIECE_PAWN-1] & g_bbMaskNotFileH;
		bbAttacks |= bit[sq+7] & b->bbPieces[BLACK][PIECE_PAWN-1] & g_bbMaskNotFileA;
	}

	return bbAttacks;
}

//BitBoard SquareSlidingAttacks(TBoard *b, int sq) {
///* Returns a bit board of all occupied squares attacking sq via sliding attack (i.e. R/B/Q).
//IN:		b			The board to check
//		sq			The square to check
//OUT:	bbAttacks	The resulting bitboard
//ASSUMES: None
//*/
//	BitBoard bbAttacks;
//
//	bbAttacks = BISHOP_MOVES(sq) & (b->(b->bbPieces[WHITE][PIECE_BISHOP-1]|b->bbPieces[WHITE][PIECE_QUEEN-1]) | b->(b->bbPieces[BLACK][PIECE_BISHOP-1]|b->bbPieces[BLACK][PIECE_QUEEN-1]));
//	bbAttacks |= ROOK_MOVES(sq) & (b->(b->bbPieces[WHITE][PIECE_ROOK-1]|b->bbPieces[WHITE][PIECE_QUEEN-1]) | b->(b->bbPieces[BLACK][PIECE_ROOK-1]|b->bbPieces[BLACK][PIECE_QUEEN-1]));
//
//	return bbAttacks;
//}

void ValidateBoard(TBoard *b) {
	// Validate nPieces with BitBoards
	int i;
	for (i=0;i<64;i++) {
		switch (b->nPieces[i]) {
			case WHITE_PAWN:
				if (!(b->bbPieces[WHITE][PIECE_PAWN-1] & bit[i])) {
					PrintBoard(b,stdout);
					ASSERT(0);
				}
				break;
			case WHITE_ROOK:
				if (!(b->bbPieces[WHITE][PIECE_ROOK-1] & bit[i])) {
					PrintBoard(b,stdout);
					ASSERT(0);
				}
				if (!((b->bbPieces[WHITE][PIECE_ROOK-1]|b->bbPieces[WHITE][PIECE_QUEEN-1]) & bit[i])) {
					PrintBoard(b,stdout);
					ASSERT(0);
				}
				break;
			case WHITE_BISHOP:
				if (!(b->bbPieces[WHITE][PIECE_BISHOP-1] & bit[i])) {
					PrintBoard(b,stdout);
					ASSERT(0);
				}
				if (!((b->bbPieces[WHITE][PIECE_BISHOP-1]|b->bbPieces[WHITE][PIECE_QUEEN-1]) & bit[i])) {
					PrintBoard(b,stdout);
					ASSERT(0);
				}
				break;
			case WHITE_KNIGHT:
				if (!(b->bbPieces[WHITE][PIECE_KNIGHT-1] & bit[i])) {
					PrintBoard(b,stdout);
					ASSERT(0);
				}
				break;
			case WHITE_QUEEN:
				if (!(b->bbPieces[WHITE][PIECE_QUEEN-1] & bit[i])) {
					PrintBoard(b,stdout);
					ASSERT(0);
				}
				if (!((b->bbPieces[WHITE][PIECE_ROOK-1]|b->bbPieces[WHITE][PIECE_QUEEN-1]) & bit[i])) {
					PrintBoard(b,stdout);
					ASSERT(0);
				}
				if (!((b->bbPieces[WHITE][PIECE_BISHOP-1]|b->bbPieces[WHITE][PIECE_QUEEN-1]) & bit[i])) {
					PrintBoard(b,stdout);
					ASSERT(0);
				}
				break;
			case WHITE_KING:
				if (!(b->bbPieces[WHITE][PIECE_KING-1] & bit[i])) {
					PrintBoard(b,stdout);
					ASSERT(0);
				}
				break;
			case BLACK_PAWN:
				if (!(b->bbPieces[BLACK][PIECE_PAWN-1] & bit[i])) {
					PrintBoard(b,stdout);
					ASSERT(0);
				}
				break;
			case BLACK_ROOK:
				if (!(b->bbPieces[BLACK][PIECE_ROOK-1] & bit[i])) {
					PrintBoard(b,stdout);
					ASSERT(0);
				}
				if (!((b->bbPieces[BLACK][PIECE_ROOK-1]|b->bbPieces[BLACK][PIECE_QUEEN-1]) & bit[i])) {
					PrintBoard(b,stdout);
					ASSERT(0);
				}
				break;
			case BLACK_BISHOP:
				if (!(b->bbPieces[BLACK][PIECE_BISHOP-1] & bit[i])) {
					PrintBoard(b,stdout);
					ASSERT(0);
				}
				break;
				if (!((b->bbPieces[BLACK][PIECE_BISHOP-1]|b->bbPieces[BLACK][PIECE_QUEEN-1]) & bit[i])) {
					PrintBoard(b,stdout);
					ASSERT(0);
				}
			case BLACK_KNIGHT:
				if (!(b->bbPieces[BLACK][PIECE_KNIGHT-1] & bit[i])) {
					PrintBoard(b,stdout);
					ASSERT(0);
				}
				break;
			case BLACK_QUEEN:
				if (!(b->bbPieces[BLACK][PIECE_QUEEN-1] & bit[i])) {
					PrintBoard(b,stdout);
					ASSERT(0);
				}
				break;
				if (!((b->bbPieces[BLACK][PIECE_ROOK-1]|b->bbPieces[BLACK][PIECE_QUEEN-1]) & bit[i])) {
					PrintBoard(b,stdout);
					ASSERT(0);
				}
				break;
				if (!((b->bbPieces[BLACK][PIECE_BISHOP-1]|b->bbPieces[BLACK][PIECE_QUEEN-1]) & bit[i])) {
					PrintBoard(b,stdout);
					ASSERT(0);
				}
				break;
			case BLACK_KING:
				if (!(b->bbPieces[BLACK][PIECE_KING-1] & bit[i])) {
					PrintBoard(b,stdout);
					ASSERT(0);
				}
				break;
			case SQUARE_EMPTY:
				if ((b->bbPieces[WHITE][PIECE_ALL-1] | b->bbPieces[BLACK][PIECE_ALL-1]) & bit[i]) {
					PrintBoard(b,stdout);
					ASSERT(0);
				}
				break;
		}
	}
	
//	__int32 hash;//, cs;
	TBoard b2;
	//memcpy(&b2,b,sizeof(b2));

	//CalculateHashValues(&b2);
	//if ((b2.nTransHash != b->nTransHash)) {
	//	PrintBoard(b, stdout);
	//	PrintBoard(&b2, stdout);
 //		ASSERT(0);
	//}
	//if ((b2.nPawnHash != b->nPawnHash)) {
	//	PrintBoard(b, stdout);
	//	PrintBoard(&b2, stdout);
	//	ASSERT(0);
	//}
	//if ((b2.nMaterialHash != b->nMaterialHash)) {
	//	PrintBoard(b, stdout);
	//	PrintBoard(&b2, stdout);
	//	ASSERT(0);
	//}

	int nMatWhite=0, nMatBlack=0;
	for (i=0;i<64;i++) {
		if ((b->nPieces[i] & SQUARE_COLOR_MASK) == PIECE_WHITE) {
			nMatWhite += g_nPieceValues[b->nPieces[i]];
		} else {
			nMatBlack -= g_nPieceValues[b->nPieces[i]];
		}
	}
	//if ((nMatWhite != b->nWhiteMaterial) || (nMatBlack != b->nBlackMaterial)) {
	//	PrintBoard(b,stdout);
	//	ASSERT(0);
	//}

	//hash = b->pawnHash;
	//cs = b->pawnCheck;
	//CalculateHashValues(b2);
	//if ((b->pawnHash != hash) || (b->pawnCheck != cs)) {
	//	PrintBoard(b, stdout);
	//	PrintBoard(b2, stdout);
	//	ASSERT(0);
	//}
	//b->pawnHash = hash;
	//b->pawnCheck = cs;

	// Validate the bbRotate90 board
	BitBoard bb = 0;
	for (i=0;i<64;i++) {
		if ((b->bbPieces[BLACK][PIECE_ALL-1] | b->bbPieces[WHITE][PIECE_ALL-1]) & bit[i]) { // square is occupied
			bb |= bit[g_nRotate90Map[i]];
		}
	}
	if (bb != b->bbRotate90) {
		PrintBoard(b,stdout);
		ASSERT(0);
	}

	// Validate the bbRotate45L board
	bb = 0;
	for (i=0;i<64;i++) {
		if ((b->bbPieces[BLACK][PIECE_ALL-1] | b->bbPieces[WHITE][PIECE_ALL-1]) & bit[i]) { // square is occupied
			bb |= bit[g_nRotate45LMap[i]];
		}
	}
	if (bb != b->bbRotate45L) {
		PrintBoard(b,stdout);
		ASSERT(0);
	}

	// Validate the bbRotate45R board
	bb = 0;
	for (i=0;i<64;i++) {
		if ((b->bbPieces[BLACK][PIECE_ALL-1] | b->bbPieces[WHITE][PIECE_ALL-1]) & bit[i]) { // square is occupied
			bb |= bit[g_nRotate45RMap[i]];
		}
	}
	if (bb != b->bbRotate45R) {
		PrintBoard(b,stdout);
		ASSERT(0);
	}

	// Verify BitBoards
	ZeroBoard(&b2);

	for (i=0;i<64;i++) {
		switch (b->nPieces[i]) {
			case WHITE_PAWN:
				b2.bbPieces[WHITE][PIECE_ALL-1] ^= bit[i];
				b2.bbPieces[WHITE][PIECE_PAWN-1] ^= bit[i];
				break;
			case WHITE_KNIGHT:
				b2.bbPieces[WHITE][PIECE_ALL-1] ^= bit[i];
				b2.bbPieces[WHITE][PIECE_KNIGHT-1] ^= bit[i];
				break;
			case WHITE_BISHOP:
				b2.bbPieces[WHITE][PIECE_ALL-1] ^= bit[i];
				b2.bbPieces[WHITE][PIECE_BISHOP-1] ^= bit[i];
				//b2.(b->bbPieces[WHITE][PIECE_BISHOP-1]|b->bbPieces[WHITE][PIECE_QUEEN-1]) ^= bit[i];
				break;
			case WHITE_ROOK:
				b2.bbPieces[WHITE][PIECE_ALL-1] ^= bit[i];
				b2.bbPieces[WHITE][PIECE_ROOK-1] ^= bit[i];
				//b2.(b->bbPieces[WHITE][PIECE_ROOK-1]|b->bbPieces[WHITE][PIECE_QUEEN-1]) ^= bit[i];
				break;
			case WHITE_QUEEN:
				b2.bbPieces[WHITE][PIECE_ALL-1] ^= bit[i];
				b2.bbPieces[WHITE][PIECE_QUEEN-1] ^= bit[i];
				//b2.(b->bbPieces[WHITE][PIECE_ROOK-1]|b->bbPieces[WHITE][PIECE_QUEEN-1]) ^= bit[i];
				//b2.(b->bbPieces[WHITE][PIECE_BISHOP-1]|b->bbPieces[WHITE][PIECE_QUEEN-1]) ^= bit[i];
				break;
			case WHITE_KING:
				b2.bbPieces[WHITE][PIECE_ALL-1] ^= bit[i];
				b2.bbPieces[WHITE][PIECE_KING-1] ^= bit[i];
				break;
			case BLACK_PAWN:
				b2.bbPieces[BLACK][PIECE_ALL-1] ^= bit[i];
				b2.bbPieces[BLACK][PIECE_PAWN-1] ^= bit[i];
				break;
			case BLACK_KNIGHT:
				b2.bbPieces[BLACK][PIECE_ALL-1] ^= bit[i];
				b2.bbPieces[BLACK][PIECE_KNIGHT-1] ^= bit[i];
				break;
			case BLACK_BISHOP:
				b2.bbPieces[BLACK][PIECE_ALL-1] ^= bit[i];
				b2.bbPieces[BLACK][PIECE_BISHOP-1] ^= bit[i];
				//b2.(b->bbPieces[BLACK][PIECE_BISHOP-1]|b->bbPieces[BLACK][PIECE_QUEEN-1]) ^= bit[i];
				break;
			case BLACK_ROOK:
				b2.bbPieces[BLACK][PIECE_ALL-1] ^= bit[i];
				b2.bbPieces[BLACK][PIECE_ROOK-1] ^= bit[i];
				//b2.(b->bbPieces[BLACK][PIECE_ROOK-1]|b->bbPieces[BLACK][PIECE_QUEEN-1]) ^= bit[i];
				break;
			case BLACK_QUEEN:
				b2.bbPieces[BLACK][PIECE_ALL-1] ^= bit[i];
				b2.bbPieces[BLACK][PIECE_QUEEN-1] ^= bit[i];
				//b2.(b->bbPieces[BLACK][PIECE_ROOK-1]|b->bbPieces[BLACK][PIECE_QUEEN-1]) ^= bit[i];
				//b2.(b->bbPieces[BLACK][PIECE_BISHOP-1]|b->bbPieces[BLACK][PIECE_QUEEN-1]) ^= bit[i];
				break;
			case BLACK_KING:
				b2.bbPieces[BLACK][PIECE_ALL-1] ^= bit[i];
				b2.bbPieces[BLACK][PIECE_KING-1] ^= bit[i];
				break;
		}
	}
	if (b2.bbPieces[WHITE][PIECE_ALL-1]!=b->bbPieces[WHITE][PIECE_ALL-1] || b2.bbPieces[WHITE][PIECE_PAWN-1]!=b->bbPieces[WHITE][PIECE_PAWN-1]
		|| b2.bbPieces[WHITE][PIECE_KNIGHT-1]!=b->bbPieces[WHITE][PIECE_KNIGHT-1] || b2.bbPieces[WHITE][PIECE_ROOK-1]!=b->bbPieces[WHITE][PIECE_ROOK-1]
		|| b2.bbPieces[WHITE][PIECE_BISHOP-1]!=b->bbPieces[WHITE][PIECE_BISHOP-1] || b2.bbPieces[WHITE][PIECE_QUEEN-1]!=b->bbPieces[WHITE][PIECE_QUEEN-1]
		|| b2.bbPieces[WHITE][PIECE_KING-1]!=b->bbPieces[WHITE][PIECE_KING-1]) {
		PrintBoard(b,stdout);
		ASSERT(0);
	}
	if (b2.bbPieces[BLACK][PIECE_ALL-1]!=b->bbPieces[BLACK][PIECE_ALL-1] || b2.bbPieces[BLACK][PIECE_PAWN-1]!=b->bbPieces[BLACK][PIECE_PAWN-1]
		|| b2.bbPieces[BLACK][PIECE_KNIGHT-1]!=b->bbPieces[BLACK][PIECE_KNIGHT-1] || b2.bbPieces[BLACK][PIECE_ROOK-1]!=b->bbPieces[BLACK][PIECE_ROOK-1]
		|| b2.bbPieces[BLACK][PIECE_BISHOP-1]!=b->bbPieces[BLACK][PIECE_BISHOP-1] || b2.bbPieces[BLACK][PIECE_QUEEN-1]!=b->bbPieces[BLACK][PIECE_QUEEN-1]
		|| b2.bbPieces[BLACK][PIECE_KING-1]!=b->bbPieces[BLACK][PIECE_KING-1]) {
		PrintBoard(b,stdout);
		ASSERT(0);
	}
}


void SetBitBoards(TBoard *b) {
// Uses nPieces[] to set the bitboards in b.
	int i;

	b->bbPieces[WHITE][PIECE_PAWN-1] = 0;
	b->bbPieces[WHITE][PIECE_KNIGHT-1] = 0;
	b->bbPieces[WHITE][PIECE_BISHOP-1] = 0;
	b->bbPieces[WHITE][PIECE_ROOK-1] = 0;
	b->bbPieces[WHITE][PIECE_QUEEN-1] = 0;
	b->bbPieces[WHITE][PIECE_KING-1] = 0;
	b->bbPieces[BLACK][PIECE_PAWN-1] = 0;
	b->bbPieces[BLACK][PIECE_KNIGHT-1] = 0;
	b->bbPieces[BLACK][PIECE_BISHOP-1] = 0;
	b->bbPieces[BLACK][PIECE_ROOK-1] = 0;
	b->bbPieces[BLACK][PIECE_QUEEN-1] = 0;
	b->bbPieces[BLACK][PIECE_KING-1] = 0;
				
	for (i=0;i<64;i++) {
		switch (b->nPieces[i]) {
			case WHITE_PAWN:	b->bbPieces[WHITE][PIECE_PAWN-1] ^= bit[i];	break;
			case WHITE_KNIGHT:	b->bbPieces[WHITE][PIECE_KNIGHT-1] ^= bit[i];	break;
			case WHITE_BISHOP:	b->bbPieces[WHITE][PIECE_BISHOP-1] ^= bit[i];	break;
			case WHITE_ROOK:	b->bbPieces[WHITE][PIECE_ROOK-1] ^= bit[i];	break;
			case WHITE_QUEEN:	b->bbPieces[WHITE][PIECE_QUEEN-1] ^= bit[i];	break;
			case WHITE_KING:	b->bbPieces[WHITE][PIECE_KING-1] ^= bit[i];		break;
			case BLACK_PAWN:	b->bbPieces[BLACK][PIECE_PAWN-1] ^= bit[i];	break;
			case BLACK_KNIGHT:	b->bbPieces[BLACK][PIECE_KNIGHT-1] ^= bit[i];	break;
			case BLACK_BISHOP:	b->bbPieces[BLACK][PIECE_BISHOP-1] ^= bit[i];	break;
			case BLACK_ROOK:	b->bbPieces[BLACK][PIECE_ROOK-1] ^= bit[i];	break;
			case BLACK_QUEEN:	b->bbPieces[BLACK][PIECE_QUEEN-1] ^= bit[i];	break;
			case BLACK_KING:	b->bbPieces[BLACK][PIECE_KING-1] ^= bit[i];		break;
		}
	}

	//b->(b->bbPieces[WHITE][PIECE_ROOK-1]|b->bbPieces[WHITE][PIECE_QUEEN-1]) = b->bbPieces[WHITE][PIECE_ROOK-1] | b->bbPieces[WHITE][PIECE_QUEEN-1];
	//b->(b->bbPieces[BLACK][PIECE_ROOK-1]|b->bbPieces[BLACK][PIECE_QUEEN-1]) = b->bbPieces[BLACK][PIECE_ROOK-1] | b->bbPieces[BLACK][PIECE_QUEEN-1];
	//b->(b->bbPieces[WHITE][PIECE_BISHOP-1]|b->bbPieces[WHITE][PIECE_QUEEN-1]) = b->bbPieces[WHITE][PIECE_BISHOP-1] | b->bbPieces[WHITE][PIECE_QUEEN-1];
	//b->(b->bbPieces[BLACK][PIECE_BISHOP-1]|b->bbPieces[BLACK][PIECE_QUEEN-1]) = b->bbPieces[BLACK][PIECE_BISHOP-1] | b->bbPieces[BLACK][PIECE_QUEEN-1];

	b->bbPieces[WHITE][PIECE_ALL-1] = b->bbPieces[WHITE][PIECE_PAWN-1] | b->bbPieces[WHITE][PIECE_KNIGHT-1] | (b->bbPieces[WHITE][PIECE_BISHOP-1]|b->bbPieces[WHITE][PIECE_QUEEN-1]) | (b->bbPieces[WHITE][PIECE_ROOK-1]|b->bbPieces[WHITE][PIECE_QUEEN-1]) | b->bbPieces[WHITE][PIECE_KING-1];
	b->bbPieces[BLACK][PIECE_ALL-1] = b->bbPieces[BLACK][PIECE_PAWN-1] | b->bbPieces[BLACK][PIECE_KNIGHT-1] | (b->bbPieces[BLACK][PIECE_BISHOP-1]|b->bbPieces[BLACK][PIECE_QUEEN-1]) | (b->bbPieces[BLACK][PIECE_ROOK-1]|b->bbPieces[BLACK][PIECE_QUEEN-1]) | b->bbPieces[BLACK][PIECE_KING-1];

	b->bbRotate90 = 0;
	for (i=0;i<64;i++) {
		if ((b->bbPieces[BLACK][PIECE_ALL-1] | b->bbPieces[WHITE][PIECE_ALL-1]) & bit[i]) { // square is occupied
			b->bbRotate90 |= bit[g_nRotate90Map[i]];
		}
	}

	b->bbRotate45L = 0;
	for (i=0;i<64;i++) {
		if ((b->bbPieces[BLACK][PIECE_ALL-1] | b->bbPieces[WHITE][PIECE_ALL-1]) & bit[i]) { // square is occupied
			b->bbRotate45L |= bit[g_nRotate45LMap[i]];
		}
	}

	b->bbRotate45R = 0;
	for (i=0;i<64;i++) {
		if ((b->bbPieces[BLACK][PIECE_ALL-1] | b->bbPieces[WHITE][PIECE_ALL-1]) & bit[i]) { // square is occupied
			b->bbRotate45R |= bit[g_nRotate45RMap[i]];
		}
	}
}

void FlipBoard(TBoard *b) {
// Interchange white and black pieces
	int p;

	// Change piece colours
	for (int i=0;i<64;i++) {
		if (b->nPieces[i]) 
			b->nPieces[i] ^= SQUARE_COLOR_MASK;
	}
	b->nSideToMove = OPP(b->nSideToMove);
	//m = b->nWhiteMaterial;
	//b->nWhiteMaterial = b->nBlackMaterial;
	//b->nBlackMaterial = m;

	// Swap piece squares
	for (int i=0;i<32;i++) {
		p = b->nPieces[i];
		b->nPieces[i] = b->nPieces[FLIP_RANK(i)];
		b->nPieces[FLIP_RANK(i)] = p;
	}

	// Swap castling rights
	int c = b->nCastling;
	b->nCastling = 0;
	if (c & WHITE_KING_CASTLE) b->nCastling |= BLACK_KING_CASTLE;
	if (c & WHITE_QUEEN_CASTLE) b->nCastling |= BLACK_QUEEN_CASTLE;
	if (c & BLACK_KING_CASTLE) b->nCastling |= WHITE_KING_CASTLE;
	if (c & BLACK_QUEEN_CASTLE) b->nCastling |= WHITE_QUEEN_CASTLE;
	if (c & WHITE_HAS_CASTLED) b->nCastling |= BLACK_HAS_CASTLED;
	if (c & BLACK_HAS_CASTLED) b->nCastling |= WHITE_HAS_CASTLED;

	// Move EP square
	if (b->nEPSquare > -1)
		b->nEPSquare = (FLIP_RANK(b->nEPSquare));

	SetBitBoards(b);

	//CalculateHashValues(b);
}

void FlopBoard(TBoard *b) {
// Flip the board sideways
	int p;

	// Swap piece squares
	for (int i=0;i<64;i++) {
		if (FILE(i)<=3) {
			p = b->nPieces[i];
			b->nPieces[i] = b->nPieces[RANK(i)*8+7-FILE(i)];
			b->nPieces[RANK(i)*8+7-FILE(i)] = p;
		}
	}

	// Move EP square
	if (b->nEPSquare > -1)
		b->nEPSquare = (RANK(b->nEPSquare))*8+7-FILE(b->nEPSquare);

	SetBitBoards(b);
}

bool IsInsufficientMaterial(TBoard *b) {
	if (b->bbPieces[WHITE][PIECE_PAWN-1]) return false;
	if ((b->bbPieces[WHITE][PIECE_ROOK-1]|b->bbPieces[WHITE][PIECE_QUEEN-1])) return false;
	if (b->bbPieces[BLACK][PIECE_PAWN-1]) return false;
	if ((b->bbPieces[BLACK][PIECE_ROOK-1]|b->bbPieces[BLACK][PIECE_QUEEN-1])) return false;

	// no rooks, queens or pawns
	int wn = CountBits(b->bbPieces[WHITE][PIECE_KNIGHT-1]);
	int bn = CountBits(b->bbPieces[BLACK][PIECE_KNIGHT-1]);
	int wb = CountBits(b->bbPieces[WHITE][PIECE_BISHOP-1]);
	int bb = CountBits(b->bbPieces[BLACK][PIECE_BISHOP-1]);

	if (wb+wn+bb+bn <= 1) return true;

	return false;
}

bool IsRepetition(TBoard *b, int nThreadID) {
	int i, num, nTargetCount;
	U64 nCurrentHash = (b->nTransHash^g_nHash50Move[b->nFiftyMoveCount]); // Take out the 50 move rule as it doesnt count in the repetition hashes

	// Put the current position in the list (FILO during search).
	g_nRepetitionHashes[g_nGameHalfMoveNum[nThreadID]][nThreadID] = nCurrentHash;

	nTargetCount = 3;

	// Search for draw by repetition
	num = 1;
	for (i=g_nGameHalfMoveNum[nThreadID]-1;i>=g_nRepetitionListHead[nThreadID];i--) {
		if (g_nRepetitionHashes[i][nThreadID] == nCurrentHash) {
			num++;
			if (num == nTargetCount) 
				return true;
		}
	}

	return false;
}

void LoadFEN(TBoard *b, const char a_sFEN[]) {
/* FEN standard http://en.wikipedia.org/wiki/FEN
	A FEN record contains 6 fields. The separator between fields is a space. The fields are:

	1. Piece placement (from white's perspective). Each rank is described, starting with rank 8 and ending with rank 1; within each rank, the contents of each square are described from file a through file h. White pieces are designated using upper-case letters ("KQRBNP"), Black by lowercase ("kqrbnp"). Blank squares are noted using digits 1 through 8 (the number of blank squares), and "/" separate ranks.
	2. Active color. "w" means white moves next, "b" means black.
	3. Castling availability. If neither side can castle, this is "-". Otherwise, this has one or more letters: "K" (White can castle kingside), "Q" (White can castle queenside), "k" (Black can castle kingside), and/or "q" (Black can castle queenside).
	4. En passant target square. If there's no en passant target square, this is "-". If a pawn has just made a 2-square move, this is the position "behind" the pawn.
	5. Halfmove clock: This is the number of halfmoves since the last pawn advance or capture. This is used to determine if a draw can be claimed under the fifty move rule.
	6. Fullmove number: The number of the full move. It starts at 1, and is incremented after Black's move.

   eg: 1r6/5kp1/RqQb1p1p/1p1PpP2/1Pp1B3/2P4P/6P1/5K2 b - - 0 45
*/

	char *str;
	char tmpStr[MAX_FEN_LEN];
	int index;
	int i, j;
	char sFEN[MAX_FEN_LEN];

	strncpy(sFEN,a_sFEN,MAX_FEN_LEN-1);
	sFEN[MAX_FEN_LEN-1] = 0;

	str = strchr(sFEN,'/');
	strcpy(tmpStr, str+1);

	index = 63;
	//InitialiseArrays();
	ZeroBoard(b);

	// Piece Placement Data
	for (j=0;j<8;j++) {
		for (i=0;i<str-sFEN;i++) {

			ASSERT(index>=0);
			if (index<0) break;

			switch (sFEN[i]) {
			case '1': index-=1; break;
			case '2': index-=2; break;
			case '3': index-=3; break;
			case '4': index-=4; break;
			case '5': index-=5; break;
			case '6': index-=6; break;
			case '7': index-=7; break;
			case '8': index-=8; break;
			case 'P': 
				b->nPieces[index] = PIECE_WHITE | PIECE_PAWN;
				b->bbPieces[WHITE][PIECE_ALL-1] |= bit[index];
				b->bbPieces[WHITE][PIECE_PAWN-1] |=  bit[index];
				b->bbRotate45L |= bit[g_nRotate45LMap[index]];
				b->bbRotate45R |= bit[g_nRotate45RMap[index]];
				b->bbRotate90 |= bit[g_nRotate90Map[index--]];
				b->nMaterial[WHITE] += SCORE_PAWN;
				break;
			case 'N': 
				b->nPieces[index] = PIECE_WHITE | PIECE_KNIGHT;
				b->bbPieces[WHITE][PIECE_ALL-1] |= bit[index];
				b->bbPieces[WHITE][PIECE_KNIGHT-1] |= bit[index];
				b->bbRotate45L |= bit[g_nRotate45LMap[index]];
				b->bbRotate45R |= bit[g_nRotate45RMap[index]];
				b->bbRotate90 |= bit[g_nRotate90Map[index--]];
				b->nMaterial[WHITE] += SCORE_KNIGHT;
				break;
			case 'B': 
				b->nPieces[index] = PIECE_WHITE | PIECE_BISHOP;
				b->bbPieces[WHITE][PIECE_ALL-1] |= bit[index];
				b->bbPieces[WHITE][PIECE_BISHOP-1] |= bit[index];
				b->bbRotate45L |= bit[g_nRotate45LMap[index]];
				b->bbRotate45R |= bit[g_nRotate45RMap[index]];
				b->bbRotate90 |= bit[g_nRotate90Map[index--]];
				b->nMaterial[WHITE] += SCORE_BISHOP;
				break;
			case 'R': 
				b->nPieces[index] = PIECE_WHITE | PIECE_ROOK;
				b->bbPieces[WHITE][PIECE_ALL-1] |= bit[index];
				b->bbPieces[WHITE][PIECE_ROOK-1] |= bit[index];
				b->bbRotate45L |= bit[g_nRotate45LMap[index]];
				b->bbRotate45R |= bit[g_nRotate45RMap[index]];
				b->bbRotate90 |= bit[g_nRotate90Map[index--]];
				b->nMaterial[WHITE] += SCORE_ROOK;
				break;
			case 'Q': 
				b->nPieces[index] = PIECE_WHITE | PIECE_QUEEN;
				b->bbPieces[WHITE][PIECE_ALL-1] |= bit[index];
				b->bbPieces[WHITE][PIECE_QUEEN-1] |= bit[index];
				b->bbRotate45L |= bit[g_nRotate45LMap[index]];
				b->bbRotate45R |= bit[g_nRotate45RMap[index]];
				b->bbRotate90 |= bit[g_nRotate90Map[index--]];
				b->nMaterial[WHITE] += SCORE_QUEEN;
				break;
			case 'K':
				b->nPieces[index] = PIECE_WHITE | PIECE_KING;
				b->bbPieces[WHITE][PIECE_ALL-1] |= bit[index];
				b->bbPieces[WHITE][PIECE_KING-1] |= bit[index];
				b->bbRotate45L |= bit[g_nRotate45LMap[index]];
				b->bbRotate45R |= bit[g_nRotate45RMap[index]];
				b->bbRotate90 |= bit[g_nRotate90Map[index]];
				index--;
				break;
			case 'p': 
				b->nPieces[index] = PIECE_BLACK | PIECE_PAWN;
				b->bbPieces[BLACK][PIECE_ALL-1] |= bit[index];
				b->bbPieces[BLACK][PIECE_PAWN-1] |= bit[index];
				b->bbRotate45L |= bit[g_nRotate45LMap[index]];
				b->bbRotate45R |= bit[g_nRotate45RMap[index]];
				b->bbRotate90 |= bit[g_nRotate90Map[index--]];
				b->nMaterial[BLACK] += SCORE_PAWN;
				break;
			case 'n': 
				b->nPieces[index] = PIECE_BLACK | PIECE_KNIGHT;
				b->bbPieces[BLACK][PIECE_ALL-1] |= bit[index];
				b->bbPieces[BLACK][PIECE_KNIGHT-1] |= bit[index];
				b->bbRotate45L |= bit[g_nRotate45LMap[index]];
				b->bbRotate45R |= bit[g_nRotate45RMap[index]];
				b->bbRotate90 |= bit[g_nRotate90Map[index--]];
				b->nMaterial[BLACK] += SCORE_KNIGHT;
				break;
			case 'b': 
				b->nPieces[index] = PIECE_BLACK | PIECE_BISHOP;
				b->bbPieces[BLACK][PIECE_ALL-1] |= bit[index];
				b->bbPieces[BLACK][PIECE_BISHOP-1] |= bit[index];
				b->bbRotate45L |= bit[g_nRotate45LMap[index]];
				b->bbRotate45R |= bit[g_nRotate45RMap[index]];
				b->bbRotate90 |= bit[g_nRotate90Map[index--]];
				b->nMaterial[BLACK] += SCORE_BISHOP;
				break;
			case 'r': 
				b->nPieces[index] = PIECE_BLACK | PIECE_ROOK;
				b->bbPieces[BLACK][PIECE_ALL-1] |= bit[index];
				b->bbPieces[BLACK][PIECE_ROOK-1] |= bit[index];
				b->bbRotate45L |= bit[g_nRotate45LMap[index]];
				b->bbRotate45R |= bit[g_nRotate45RMap[index]];
				b->bbRotate90 |= bit[g_nRotate90Map[index--]];
				b->nMaterial[BLACK] += SCORE_ROOK;
				break;
			case 'q': 
				b->nPieces[index] = PIECE_BLACK | PIECE_QUEEN;
				b->bbPieces[BLACK][PIECE_ALL-1] |= bit[index];
				b->bbPieces[BLACK][PIECE_QUEEN-1] |= bit[index];
				b->bbRotate45L |= bit[g_nRotate45LMap[index]];
				b->bbRotate45R |= bit[g_nRotate45RMap[index]];
				b->bbRotate90 |= bit[g_nRotate90Map[index--]];
				b->nMaterial[BLACK] += SCORE_QUEEN;
				break;
			case 'k':
				b->nPieces[index] = PIECE_BLACK | PIECE_KING;
				b->bbPieces[BLACK][PIECE_ALL-1] |= bit[index];
				b->bbPieces[BLACK][PIECE_KING-1] |= bit[index];
				b->bbRotate45L |= bit[g_nRotate45LMap[index]];
				b->bbRotate45R |= bit[g_nRotate45RMap[index]];
				b->bbRotate90 |= bit[g_nRotate90Map[index]];
				index--;
				break;
			}
		}


		// Get position of next line break
		strcpy(tmpStr, str+1);
		strcpy(sFEN, tmpStr);
		str = strchr(sFEN,'/');
		if (str==NULL) {
			// If there is no slash, look for a space instead
			str = strchr(sFEN, ' ');
			if (j==6 && str==NULL) {
				// bad fen
				break;
			}
		}
	}

	// Active Color
	if (sFEN[0] == 'w') {
		b->nSideToMove = WHITE;
	} else {
		b->nSideToMove = BLACK;
	}

	if (sFEN[1] == '\0') {
		b->nEPSquare = NULL_SQUARE;
	} else {
		// Castling Availability
		strcpy(tmpStr, sFEN+2);
		i=0;
		while (strchr("KQkqabcdefghABCDEFGH-", tmpStr[i])) {
			if (tmpStr[i] == '-')		{ i++; break; }
			else if (tmpStr[i] == 'K')	{ b->nCastling |= WHITE_KING_CASTLE; }
			else if (tmpStr[i] == 'Q')	{ b->nCastling |= WHITE_QUEEN_CASTLE; }
			else if (tmpStr[i] == 'k')	{ b->nCastling |= BLACK_KING_CASTLE; }
			else if (tmpStr[i] == 'q')	{ b->nCastling |= BLACK_QUEEN_CASTLE; }
			else if (tmpStr[i] >= 'A' && tmpStr[i] <= 'H') {	// Chess960 castling uses the Rook file
				int rookFile, kingFile = -1;
				for (int sq = G1; sq <= B1; sq++)
					if (b->nPieces[sq] == WHITE_KING)
						kingFile = FILE(sq);
				rookFile = FILE_A - FILE(tmpStr[i] - 'A');
				if (rookFile > kingFile) {
					b->nCastling |= WHITE_QUEEN_CASTLE;
				} else {
					b->nCastling |= WHITE_KING_CASTLE;
				}
			}
			else if (tmpStr[i] >= 'a' && tmpStr[i] <= 'h') {	// Chess960 castling uses the Rook file
				int rookFile, kingFile = -1;
				for (int sq = G8; sq <= B8; sq++)
					if (b->nPieces[sq] == BLACK_KING)
						kingFile = FILE(sq);
				rookFile = FILE_A - FILE(tmpStr[i] - 'a');
				if (rookFile > kingFile) {
					b->nCastling |= BLACK_QUEEN_CASTLE;
				} else {
					b->nCastling |= BLACK_KING_CASTLE;
				}
			}

			i++;
		}
		
		// En Passant Target Square
		strcpy(sFEN,tmpStr+1+i);
		if ((sFEN[0] == '-') || (sFEN[0] == '\0') || (tmpStr[i] == '\0')) {
			b->nEPSquare = NULL_SQUARE;
			strcpy(tmpStr, sFEN+2);
			if (sFEN[1] == '\0') tmpStr[0] = '\0';
		} else {
			b->nEPSquare = 'h' - sFEN[0] + (sFEN[1]-'1') * 8;
			strcpy(tmpStr, sFEN+3);
			if (sFEN[2] == '\0') tmpStr[0] = '\0';
		}

		// Fifty Move Counter
		b->nFiftyMoveCount = 0;
		if (tmpStr[0] == '\0') {
			b->nFiftyMoveCount = 0;
			sFEN[0] = '\0';
		} else if (tmpStr[1] >= '0' && tmpStr[1] <= '9') {
			b->nFiftyMoveCount = (tmpStr[0]-'0') * 10 + (tmpStr[1]-'0');
			strcpy(sFEN, tmpStr+3);
		} else {
			b->nFiftyMoveCount = tmpStr[0]-'0';
			strcpy(sFEN, tmpStr+2);
		}

		// Full Move Number
		/*g->nGameMoveNum = 1;
		if (sFEN[0] == '\0') {
			g->nGameMoveNum = 1;
		} else if (sFEN[1] >= '0' && sFEN[1] <= '9') {
			g->nGameMoveNum = (sFEN[0]-'0') * 10 + (sFEN[1]-'0');
			if (sFEN[2] >= '0' && sFEN[2] <= '9') {
				g->nGameMoveNum *= 10;
				g->nGameMoveNum += sFEN[2]-'0';
			}
		} else {
			g->nGameMoveNum = sFEN[0]-'0';
		}
		if (g->nGameMoveNum < 1) g->nGameMoveNum = 1; // v1.06 protect against stupid tournament managers
		g->nGameHalfMoveNum = (g->nGameMoveNum-1)*2 + (b->nSideToMove==WHITE?0:1);*/
	}

	//CalculateHashValues(b);

}

#include "stdafx.h"
#include "Move.h"
#include "Board.h"
#include "Common.h"
#include "Hash.h"

/*void MakeMove(TBoard *b, TBoard *b2, TMove m, int nCurrentDepth, int nThreadID) {
// Applies the move m to board b and returns the new board in b2 (does not modify b).
	int nSideToMove = b->nSideToMove;	// Speed up access to data
	int nFromSquare = GET_MOVE_FROM(m);
	int nToSquare = GET_MOVE_TO(m);
	int nMoveType = GET_MOVE_TYPE(m);
	int nPromPiece = GET_MOVE_PROM(m);
	int nMovedSquare = b->nPieces[nFromSquare];			// Save piece+color
	int nMovedPiece = nMovedSquare & SQUARE_PIECE_MASK;		// Save piece without color
	int nCapturedSquare = b->nPieces[nToSquare];			// Save piece+color
	int nCapturedPiece = nCapturedSquare & SQUARE_PIECE_MASK;		// Save piece without color
	BitBoard bbFromTo;		// Bits in the From and To squares for quick modifications to the bitboards
	int nCastleBefore, nCastleAfter;
	
	ASSERT_MOVE(m);
	ASSERT(m);
	ASSERT(nMovedPiece);
	//ASSERT(IsValidMove(m, b));
	ASSERT((b->nPieces[nFromSquare] & SQUARE_COLOR_MASK) != (b->nPieces[nToSquare] & SQUARE_COLOR_MASK));
	ASSERT(nFromSquare>=0);
	ASSERT(nFromSquare<64);
	ASSERT(nToSquare>=0);
	ASSERT(nToSquare<64);

	// Copy source board to dest board.
	memcpy(b2, b, sizeof(*b));
	//CopyBoard(b2, b);

	nCastleBefore = b2->nCastling;
	b2->nPieces[nToSquare]   = b2->nPieces[nFromSquare];
	b2->nPieces[nFromSquare] = SQUARE_EMPTY;
	if (nCapturedPiece || (nMoveType == MOVE_TYPE_EP)) {
		// Reset the current fifty move counter if there is a capture
		b2->nTransHash ^= g_nHash50Move[b2->nFiftyMoveCount];	// Turn off the old 50 move count
		b2->nFiftyMoveCount = 0;
	} else {
		b2->nTransHash ^= g_nHash50Move[b2->nFiftyMoveCount];	// Turn off the old 50 move count
		b2->nFiftyMoveCount++;
		b2->nTransHash ^= g_nHash50Move[b2->nFiftyMoveCount];	// Turn on the new 50 move count
	}
	if (b2->nEPSquare != NULL_SQUARE) {
		b2->nTransHash ^= g_nHashEPMove[b2->nEPSquare];
	}
	b2->nEPSquare = NULL_SQUARE; // Must reset every move unless a two-rank pawn move.

	// Update the board's hash value and checksum
	b2->nTransHash ^= GET_HASH(nFromSquare, nMovedSquare);// g_nHashValues[nFromSquare][nMovedSquare];
	b2->nTransHash ^= GET_HASH(nToSquare, nMovedSquare);
	b2->nTransHash ^= GET_HASH(nToSquare, nCapturedSquare);
	b2->nTransHash ^= g_nHashWhiteToMove;
	b2->nPawnHash ^= g_nHashWhiteToMove;

	// Change the bit boards
	bbFromTo = bit[nFromSquare] | bit[nToSquare]; // Contains two set bits
	if (nSideToMove == WHITE) {
		// Update all white specific data

		b2->bbPieces[WHITE][PIECE_ALL-1] ^= bbFromTo;
		b2->nMaterial[BLACK] -= g_nPieceValues[nCapturedPiece];

		switch (nMovedPiece) {
			case PIECE_PAWN:
				// Check for a valid EnPassent square.
				if (nToSquare - nFromSquare == 16) {
					// pawn moved two ranks
					b2->nEPSquare = nToSquare - 8;
					b2->nTransHash ^= g_nHashEPMove[b2->nEPSquare];
				}
				// Check for an EP move
				if (nMoveType == MOVE_TYPE_EP) {
					// Remove the captured pawn
					b2->nMaterialHash ^= GET_HASH(CountBits(b2->bbPieces[BLACK][PIECE_PAWN-1]), BLACK_PAWN);
					b2->bbPieces[BLACK][PIECE_PAWN-1] ^= bit[nToSquare-8];
					b2->nMaterialHash ^= GET_HASH(CountBits(b2->bbPieces[BLACK][PIECE_PAWN-1]), BLACK_PAWN);
					b2->bbPieces[BLACK][PIECE_ALL-1] ^= bit[nToSquare-8];
					b2->bbRotate90 ^= bit[g_nRotate90Map[nToSquare-8]];
					b2->bbRotate45L ^= bit[g_nRotate45LMap[nToSquare-8]];
					b2->bbRotate45R ^= bit[g_nRotate45RMap[nToSquare-8]];
					b2->nPieces[nToSquare-8] = SQUARE_EMPTY;
					b2->nTransHash ^= GET_HASH(nToSquare-8, BLACK_PAWN);
					b2->nPawnHash ^= GET_HASH(nToSquare-8, BLACK_PAWN);
					b2->nMaterial[BLACK] -= SCORE_PAWN;
				}
				// Update Pawn Hash
				b2->nPawnHash ^= GET_HASH(nFromSquare, WHITE_PAWN);
				// Check for a promotion
				if (nToSquare >= H8) {
					b2->nMaterialHash ^= GET_HASH(CountBits(b2->bbPieces[WHITE][PIECE_PAWN-1]), WHITE_PAWN);
					b2->bbPieces[WHITE][PIECE_PAWN-1] ^= bit[nFromSquare];	// Remove the promoted pawn
					b2->nMaterialHash ^= GET_HASH(CountBits(b2->bbPieces[WHITE][PIECE_PAWN-1]), WHITE_PAWN);
					switch (nPromPiece) {
						case PIECE_QUEEN:
							b2->nMaterialHash ^= GET_HASH(CountBits(b2->bbPieces[WHITE][PIECE_QUEEN-1]), WHITE_QUEEN);
							b2->bbPieces[WHITE][PIECE_QUEEN-1] ^= bit[nToSquare];
							//(b->bbPieces[WHITE][PIECE_BISHOP-1]|b->bbPieces[WHITE][PIECE_QUEEN-1]) ^= bit[nToSquare];
							//(b->bbPieces[WHITE][PIECE_ROOK-1]|b->bbPieces[WHITE][PIECE_QUEEN-1]) ^= bit[nToSquare];
							b2->nPieces[nToSquare] = WHITE_QUEEN;
							b2->nTransHash ^= GET_HASH(nToSquare, WHITE_PAWN);
							b2->nTransHash ^= GET_HASH(nToSquare, WHITE_QUEEN);
							b2->nMaterial[WHITE] += SCORE_QUEEN - SCORE_PAWN;
							b2->nMaterialHash ^= GET_HASH(CountBits(b2->bbPieces[WHITE][PIECE_QUEEN-1]), WHITE_QUEEN);
							break;
						case PIECE_ROOK:
							b2->nMaterialHash ^= GET_HASH(CountBits(b2->bbPieces[WHITE][PIECE_ROOK-1]), WHITE_ROOK);
							b2->bbPieces[WHITE][PIECE_ROOK-1] ^= bit[nToSquare];
							//b2->(b->bbPieces[WHITE][PIECE_ROOK-1]|b->bbPieces[WHITE][PIECE_QUEEN-1]) ^= bit[nToSquare];
							b2->nPieces[nToSquare] = WHITE_ROOK;
							b2->nTransHash ^= GET_HASH(nToSquare, WHITE_PAWN);
							b2->nTransHash ^= GET_HASH(nToSquare, WHITE_ROOK);
							b2->nMaterial[WHITE] += SCORE_ROOK - SCORE_PAWN;
							b2->nMaterialHash ^= GET_HASH(CountBits(b2->bbPieces[WHITE][PIECE_ROOK-1]), WHITE_ROOK);
							break;
						case PIECE_BISHOP:
							b2->nMaterialHash ^= GET_HASH(CountBits(b2->bbPieces[WHITE][PIECE_BISHOP-1]), WHITE_BISHOP);
							b2->bbPieces[WHITE][PIECE_BISHOP-1] ^= bit[nToSquare];
							//b2->(b->bbPieces[WHITE][PIECE_BISHOP-1]|b->bbPieces[WHITE][PIECE_QUEEN-1]) ^= bit[nToSquare];
							b2->nPieces[nToSquare] = WHITE_BISHOP;
							b2->nTransHash ^= GET_HASH(nToSquare, WHITE_PAWN);
							b2->nTransHash ^= GET_HASH(nToSquare, WHITE_BISHOP);
							b2->nMaterial[WHITE] += SCORE_BISHOP - SCORE_PAWN;
							b2->nMaterialHash ^= GET_HASH(CountBits(b2->bbPieces[WHITE][PIECE_BISHOP-1]), WHITE_BISHOP);
							break;
						case PIECE_KNIGHT:
							b2->nMaterialHash ^= GET_HASH(CountBits(b2->bbPieces[WHITE][PIECE_KNIGHT-1]), WHITE_KNIGHT);
							b2->bbPieces[WHITE][PIECE_KNIGHT-1] ^= bit[nToSquare];
							b2->nPieces[nToSquare] = WHITE_KNIGHT;
							b2->nTransHash ^= GET_HASH(nToSquare, WHITE_PAWN);
							b2->nTransHash ^= GET_HASH(nToSquare, WHITE_KNIGHT);
							b2->nMaterial[WHITE] += SCORE_KNIGHT - SCORE_PAWN;
							b2->nMaterialHash ^= GET_HASH(CountBits(b2->bbPieces[WHITE][PIECE_KNIGHT-1]), WHITE_KNIGHT);
							break;
					#ifdef PARANOID
						default: ASSERT(0);
					#endif
					}
				} else { // no promotion
					// xor to clear from square and set to square
					b2->bbPieces[WHITE][PIECE_PAWN-1] ^= bbFromTo;
					// Update Pawn Hash
					b2->nPawnHash ^= GET_HASH(nToSquare, WHITE_PAWN);
				}
				b2->nTransHash ^= g_nHash50Move[b2->nFiftyMoveCount];	// Turn off the new 50 move count
				// Reset the fifty move counter because a pawn moved.
				b2->nFiftyMoveCount = 0;
				// Reset the repetition detector
				//g_nRepetitionListHead = g_nGameHalfMoveNum+nCurrentDepth; // v0.99 - +depth
				break;

			case PIECE_BISHOP:
				// xor to clear from square and set to square
				b2->bbPieces[WHITE][PIECE_BISHOP-1] ^= bbFromTo;
				//b2->(b->bbPieces[WHITE][PIECE_BISHOP-1]|b->bbPieces[WHITE][PIECE_QUEEN-1]) ^= bbFromTo;
				break;

			case PIECE_KNIGHT:
				// xor to clear from square and set to square
				b2->bbPieces[WHITE][PIECE_KNIGHT-1] ^= bbFromTo;
				break;

			case PIECE_ROOK:
				// xor to clear from square and set to square
				b2->bbPieces[WHITE][PIECE_ROOK-1] ^= bbFromTo;
				//b2->(b->bbPieces[WHITE][PIECE_ROOK-1]|b->bbPieces[WHITE][PIECE_QUEEN-1]) ^= bbFromTo;
				b2->nCastling &= ~b2->g_bbCastles[nFromSquare];
				break;

			case PIECE_QUEEN:
				// xor to clear from square and set to square
				b2->bbPieces[WHITE][PIECE_QUEEN-1] ^= bbFromTo;
				//b2->(b->bbPieces[WHITE][PIECE_BISHOP-1]|b->bbPieces[WHITE][PIECE_QUEEN-1]) ^= bbFromTo;
				//b2->(b->bbPieces[WHITE][PIECE_ROOK-1]|b->bbPieces[WHITE][PIECE_QUEEN-1]) ^= bbFromTo;
				break;

			case PIECE_KING:
				// xor to clear from square and set to square
				b2->bbPieces[WHITE][PIECE_KING-1] ^= bbFromTo;
				b2->nCastling &= WHITE_CANT_CASTLE;
				if (nMoveType==MOVE_TYPE_CASTLE && nToSquare==G1) {
					// Castling King side - so move rook too
					b2->bbPieces[WHITE][PIECE_ROOK-1] ^= MOVE_WHITE_ROOK_CASTLE_K;
					//b2->(b->bbPieces[WHITE][PIECE_ROOK-1]|b->bbPieces[WHITE][PIECE_QUEEN-1]) ^= MOVE_WHITE_ROOK_CASTLE_K;
					b2->bbPieces[WHITE][PIECE_ALL-1] ^= MOVE_WHITE_ROOK_CASTLE_K;
					b2->bbRotate90 ^= MOVE_WHITE_ROOK_CASTLE_K_90;
					b2->bbRotate45L ^= MOVE_WHITE_ROOK_CASTLE_K_45L;
					b2->bbRotate45R ^= MOVE_WHITE_ROOK_CASTLE_K_45R;
					b2->nPieces[nToSquare-1] = SQUARE_EMPTY;
					b2->nPieces[nToSquare+1] = WHITE_ROOK;
					b2->nTransHash ^= GET_HASH(nToSquare-1, WHITE_ROOK);
					b2->nTransHash ^= GET_HASH(nToSquare+1, WHITE_ROOK);
					b2->nCastling |= WHITE_HAS_CASTLED;
				} else if (nMoveType==MOVE_TYPE_CASTLE && nToSquare==C1) {
					// Castling Queen side - so move rook too
					b2->bbPieces[WHITE][PIECE_ROOK-1] ^= MOVE_WHITE_ROOK_CASTLE_Q;
					//b2->(b->bbPieces[WHITE][PIECE_ROOK-1]|b->bbPieces[WHITE][PIECE_QUEEN-1]) ^= MOVE_WHITE_ROOK_CASTLE_Q;
					b2->bbPieces[WHITE][PIECE_ALL-1] ^= MOVE_WHITE_ROOK_CASTLE_Q;
					b2->bbRotate90 ^= MOVE_WHITE_ROOK_CASTLE_Q_90;
					b2->bbRotate45L ^= MOVE_WHITE_ROOK_CASTLE_Q_45L;
					b2->bbRotate45R ^= MOVE_WHITE_ROOK_CASTLE_Q_45R;
					b2->nPieces[nToSquare+2] = SQUARE_EMPTY;
					b2->nPieces[nToSquare-1] = WHITE_ROOK;
					b2->nTransHash ^= GET_HASH(nToSquare-1, WHITE_ROOK);
					b2->nTransHash ^= GET_HASH(nToSquare+2, WHITE_ROOK);
					b2->nCastling |= WHITE_HAS_CASTLED;
				}
				break;

			#ifdef PARANOID
			default:
				PrintBoard(b,stdout);
				ASSERT(0);
			#endif
		}

		//int x;
		switch (nCapturedPiece) {
			case SQUARE_EMPTY: break;
			case PIECE_PAWN:
				//x=CountBits(b2->bbPieces[BLACK][PIECE_PAWN-1]);
				b2->nMaterialHash ^= GET_HASH(CountBits(b2->bbPieces[BLACK][PIECE_PAWN-1]), BLACK_PAWN);
				b2->bbPieces[BLACK][PIECE_ALL-1] ^= bit[nToSquare];
				b2->bbPieces[BLACK][PIECE_PAWN-1] ^= bit[nToSquare];
				b2->nPawnHash ^= GET_HASH(nToSquare, BLACK_PAWN);
				//x=CountBits(b2->bbPieces[BLACK][PIECE_PAWN-1]);
				b2->nMaterialHash ^= GET_HASH(CountBits(b2->bbPieces[BLACK][PIECE_PAWN-1]), BLACK_PAWN);
				break;
			case PIECE_KNIGHT:
				b2->nMaterialHash ^= GET_HASH(CountBits(b2->bbPieces[BLACK][PIECE_KNIGHT-1]), BLACK_KNIGHT);
				b2->bbPieces[BLACK][PIECE_ALL-1] ^= bit[nToSquare];
				b2->bbPieces[BLACK][PIECE_KNIGHT-1] ^= bit[nToSquare];
				b2->nMaterialHash ^= GET_HASH(CountBits(b2->bbPieces[BLACK][PIECE_KNIGHT-1]), BLACK_KNIGHT);
				break;
			case PIECE_BISHOP:
				b2->nMaterialHash ^= GET_HASH(CountBits(b2->bbPieces[BLACK][PIECE_BISHOP-1]), BLACK_BISHOP);
				b2->bbPieces[BLACK][PIECE_ALL-1] ^= bit[nToSquare];
				b2->bbPieces[BLACK][PIECE_BISHOP-1] ^= bit[nToSquare];
				//b2->(b->bbPieces[BLACK][PIECE_BISHOP-1]|b->bbPieces[BLACK][PIECE_QUEEN-1]) ^= bit[nToSquare];
				b2->nMaterialHash ^= GET_HASH(CountBits(b2->bbPieces[BLACK][PIECE_BISHOP-1]), BLACK_BISHOP);
				break;
			case PIECE_ROOK:
				b2->nMaterialHash ^= GET_HASH(CountBits(b2->bbPieces[BLACK][PIECE_ROOK-1]), BLACK_ROOK);
				b2->bbPieces[BLACK][PIECE_ALL-1] ^= bit[nToSquare];
				b2->bbPieces[BLACK][PIECE_ROOK-1] ^= bit[nToSquare];
				//b2->(b->bbPieces[BLACK][PIECE_ROOK-1]|b->bbPieces[BLACK][PIECE_QUEEN-1]) ^= bit[nToSquare];
				b2->nCastling &= ~b2->g_bbCastles[nToSquare];
				b2->nMaterialHash ^= GET_HASH(CountBits(b2->bbPieces[BLACK][PIECE_ROOK-1]), BLACK_ROOK);
				break;
			case PIECE_QUEEN:
				b2->nMaterialHash ^= GET_HASH(CountBits(b2->bbPieces[BLACK][PIECE_QUEEN-1]), BLACK_QUEEN);
				b2->bbPieces[BLACK][PIECE_ALL-1] ^= bit[nToSquare];
				//b2->(b->bbPieces[BLACK][PIECE_BISHOP-1]|b->bbPieces[BLACK][PIECE_QUEEN-1]) ^= bit[nToSquare];
				//b2->(b->bbPieces[BLACK][PIECE_ROOK-1]|b->bbPieces[BLACK][PIECE_QUEEN-1]) ^= bit[nToSquare];
				b2->bbPieces[BLACK][PIECE_QUEEN-1] ^= bit[nToSquare];
				b2->nMaterialHash ^= GET_HASH(CountBits(b2->bbPieces[BLACK][PIECE_QUEEN-1]), BLACK_QUEEN);
				break;
			#ifdef _DEBUG
			case PIECE_KING:
				// oh crap, that wasnt supposed to happen...
				PrintBoard(b,stdout);
				ASSERT(0);
			default:
				PrintBoard(b,stdout);
				ASSERT(0);
			#endif
		}

		// Blacks turn to move now
		b2->nSideToMove = BLACK;

	} else {
		// Update all black specific data

		b2->bbPieces[BLACK][PIECE_ALL-1] ^= bbFromTo;
		b2->nMaterial[WHITE] -= g_nPieceValues[nCapturedPiece];

		switch (nMovedPiece) {
			case PIECE_PAWN:
				// Check for a valid EnPassent square.
				if (nFromSquare - nToSquare == 16) {
					// pawn moved two ranks
					b2->nEPSquare = nToSquare + 8;
					b2->nTransHash ^= g_nHashEPMove[b2->nEPSquare];
				}
				// Check for an EP move
				if (nMoveType == MOVE_TYPE_EP) {
					b2->nMaterialHash ^= GET_HASH(CountBits(b2->bbPieces[WHITE][PIECE_PAWN-1]), WHITE_PAWN);
					b2->bbPieces[WHITE][PIECE_PAWN-1] ^= bit[nToSquare+8];
					b2->nMaterialHash ^= GET_HASH(CountBits(b2->bbPieces[WHITE][PIECE_PAWN-1]), WHITE_PAWN);
					b2->bbPieces[WHITE][PIECE_ALL-1] ^= bit[nToSquare+8];
					b2->bbRotate90 ^= bit[g_nRotate90Map[nToSquare+8]];
					b2->bbRotate45L ^= bit[g_nRotate45LMap[nToSquare+8]];
					b2->bbRotate45R ^= bit[g_nRotate45RMap[nToSquare+8]];
					b2->nPieces[nToSquare+8] = SQUARE_EMPTY;
					b2->nTransHash ^= GET_HASH(nToSquare+8, WHITE_PAWN);
					b2->nPawnHash ^= GET_HASH(nToSquare+8, WHITE_PAWN);
					b2->nMaterial[WHITE] -= SCORE_PAWN;
				}
				// Update Pawn Hash
				b2->nPawnHash ^= GET_HASH(nFromSquare, BLACK_PAWN);
				// Check for a promotion
				if (nToSquare <= A1) {
					b2->nMaterialHash ^= GET_HASH(CountBits(b2->bbPieces[BLACK][PIECE_PAWN-1]), BLACK_PAWN);
					b2->bbPieces[BLACK][PIECE_PAWN-1] ^= bit[nFromSquare]; // Remove the promoted pawn
					b2->nMaterialHash ^= GET_HASH(CountBits(b2->bbPieces[BLACK][PIECE_PAWN-1]), BLACK_PAWN);
					switch (nPromPiece) {
						case PIECE_QUEEN:
							b2->nMaterialHash ^= GET_HASH(CountBits(b2->bbPieces[BLACK][PIECE_QUEEN-1]), BLACK_QUEEN);
							b2->bbPieces[BLACK][PIECE_QUEEN-1] ^= bit[nToSquare];
							//b2->(b->bbPieces[BLACK][PIECE_BISHOP-1]|b->bbPieces[BLACK][PIECE_QUEEN-1]) ^= bit[nToSquare];
							//b2->(b->bbPieces[BLACK][PIECE_ROOK-1]|b->bbPieces[BLACK][PIECE_QUEEN-1]) ^= bit[nToSquare];
							b2->nPieces[nToSquare] = BLACK_QUEEN;
							b2->nTransHash ^= GET_HASH(nToSquare, BLACK_PAWN);
							b2->nTransHash ^= GET_HASH(nToSquare, BLACK_QUEEN);
							b2->nMaterial[BLACK] += SCORE_QUEEN - SCORE_PAWN;
							b2->nMaterialHash ^= GET_HASH(CountBits(b2->bbPieces[BLACK][PIECE_QUEEN-1]), BLACK_QUEEN);
							break;
						case PIECE_ROOK:
							b2->nMaterialHash ^= GET_HASH(CountBits(b2->bbPieces[BLACK][PIECE_ROOK-1]), BLACK_ROOK);
							b2->bbPieces[BLACK][PIECE_ROOK-1] ^= bit[nToSquare];
							//b2->(b->bbPieces[BLACK][PIECE_ROOK-1]|b->bbPieces[BLACK][PIECE_QUEEN-1]) ^= bit[nToSquare];
							b2->nPieces[nToSquare] = BLACK_ROOK;
							b2->nTransHash ^= GET_HASH(nToSquare, BLACK_PAWN);
							b2->nTransHash ^= GET_HASH(nToSquare, BLACK_ROOK);
							b2->nMaterial[BLACK] += (SCORE_ROOK - SCORE_PAWN);
							b2->nMaterialHash ^= GET_HASH(CountBits(b2->bbPieces[BLACK][PIECE_ROOK-1]), BLACK_ROOK);
							break;
						case PIECE_BISHOP:
							b2->nMaterialHash ^= GET_HASH(CountBits(b2->bbPieces[BLACK][PIECE_BISHOP-1]), BLACK_BISHOP);
							b2->bbPieces[BLACK][PIECE_BISHOP-1] ^= bit[nToSquare];
							//b2->(b->bbPieces[BLACK][PIECE_BISHOP-1]|b->bbPieces[BLACK][PIECE_QUEEN-1]) ^= bit[nToSquare];
							b2->nPieces[nToSquare] = BLACK_BISHOP;
							b2->nTransHash ^= GET_HASH(nToSquare, BLACK_PAWN);
							b2->nTransHash ^= GET_HASH(nToSquare, BLACK_BISHOP);
							b2->nMaterial[BLACK] += (SCORE_BISHOP - SCORE_PAWN);
							b2->nMaterialHash ^= GET_HASH(CountBits(b2->bbPieces[BLACK][PIECE_BISHOP-1]), BLACK_BISHOP);
							break;
						case PIECE_KNIGHT:
							b2->nMaterialHash ^= GET_HASH(CountBits(b2->bbPieces[BLACK][PIECE_KNIGHT-1]), BLACK_KNIGHT);
							b2->bbPieces[BLACK][PIECE_KNIGHT-1] ^= bit[nToSquare];
							b2->nPieces[nToSquare] = BLACK_KNIGHT;
							b2->nTransHash ^= GET_HASH(nToSquare, BLACK_PAWN);
							b2->nTransHash ^= GET_HASH(nToSquare, BLACK_KNIGHT);
							b2->nMaterial[BLACK] += (SCORE_KNIGHT - SCORE_PAWN);
							b2->nMaterialHash ^= GET_HASH(CountBits(b2->bbPieces[BLACK][PIECE_KNIGHT-1]), BLACK_KNIGHT);
							break;
					#ifdef _DEBUG
						default: ASSERT(0);
					#endif
					}
				} else { // no promotion
					// xor to clear from square and set to square
					b2->bbPieces[BLACK][PIECE_PAWN-1] ^= bbFromTo;
					// Update Pawn Hash
					b2->nPawnHash ^= GET_HASH(nToSquare, BLACK_PAWN);
				}
				b2->nTransHash ^= g_nHash50Move[b2->nFiftyMoveCount];	// Turn off the new 50 move count
				// Reset the fifty move counter because a pawn moved.
				b2->nFiftyMoveCount = 0;
				// Reset the repetition detector
				//g_nRepetitionListHead = g_nGameHalfMoveNum+nCurrentDepth; // v0.99 - +depth
				break;

			case PIECE_BISHOP:
				// xor to clear from square and set to square
				b2->bbPieces[BLACK][PIECE_BISHOP-1] ^= bbFromTo;
				//b2->(b->bbPieces[BLACK][PIECE_BISHOP-1]|b->bbPieces[BLACK][PIECE_QUEEN-1]) ^= bbFromTo;
				break;

			case PIECE_KNIGHT:
				// xor to clear from square and set to square
				b2->bbPieces[BLACK][PIECE_KNIGHT-1] ^= bbFromTo;
				break;

			case PIECE_ROOK:
				// xor to clear from square and set to square
				b2->bbPieces[BLACK][PIECE_ROOK-1] ^= bbFromTo;
				//b2->(b->bbPieces[BLACK][PIECE_ROOK-1]|b->bbPieces[BLACK][PIECE_QUEEN-1]) ^= bbFromTo;
				b2->nCastling &= ~b2->g_bbCastles[nFromSquare];
				break;

			case PIECE_QUEEN:
				// xor to clear from square and set to square
				b2->bbPieces[BLACK][PIECE_QUEEN-1] ^= bbFromTo;
				//b2->(b->bbPieces[BLACK][PIECE_BISHOP-1]|b->bbPieces[BLACK][PIECE_QUEEN-1]) ^= bbFromTo;
				//b2->(b->bbPieces[BLACK][PIECE_ROOK-1]|b->bbPieces[BLACK][PIECE_QUEEN-1]) ^= bbFromTo;
				break;

			case PIECE_KING:
				// xor to clear from square and set to square
				b2->bbPieces[BLACK][PIECE_KING-1] ^= bbFromTo;
				b2->nCastling &= BLACK_CANT_CASTLE;
				if (nMoveType==MOVE_TYPE_CASTLE && nToSquare==G8) {
					// Castling King side - so move rook too
					b2->bbPieces[BLACK][PIECE_ROOK-1] ^= MOVE_BLACK_ROOK_CASTLE_K;
					//b2->(b->bbPieces[BLACK][PIECE_ROOK-1]|b->bbPieces[BLACK][PIECE_QUEEN-1]) ^= MOVE_BLACK_ROOK_CASTLE_K;
					b2->bbPieces[BLACK][PIECE_ALL-1] ^= MOVE_BLACK_ROOK_CASTLE_K;
					b2->bbRotate90 ^= MOVE_BLACK_ROOK_CASTLE_K_90;
					b2->bbRotate45L ^= MOVE_BLACK_ROOK_CASTLE_K_45L;
					b2->bbRotate45R ^= MOVE_BLACK_ROOK_CASTLE_K_45R;
					b2->nPieces[nToSquare-1] = SQUARE_EMPTY;
					b2->nPieces[nToSquare+1] = BLACK_ROOK;
					b2->nTransHash ^= GET_HASH(nToSquare-1, BLACK_ROOK);
					b2->nTransHash ^= GET_HASH(nToSquare+1, BLACK_ROOK);
					b2->nCastling |= BLACK_HAS_CASTLED;
				} else if (nMoveType==MOVE_TYPE_CASTLE && nToSquare==C8) {
					// Castling Queen side - so move rook too
					b2->bbPieces[BLACK][PIECE_ROOK-1] ^= MOVE_BLACK_ROOK_CASTLE_Q;
					//b2->(b->bbPieces[BLACK][PIECE_ROOK-1]|b->bbPieces[BLACK][PIECE_QUEEN-1]) ^= MOVE_BLACK_ROOK_CASTLE_Q;
					b2->bbPieces[BLACK][PIECE_ALL-1] ^= MOVE_BLACK_ROOK_CASTLE_Q;
					b2->bbRotate90 ^= MOVE_BLACK_ROOK_CASTLE_Q_90;
					b2->bbRotate45L ^= MOVE_BLACK_ROOK_CASTLE_Q_45L;
					b2->bbRotate45R ^= MOVE_BLACK_ROOK_CASTLE_Q_45R;
					b2->nPieces[nToSquare+2] = SQUARE_EMPTY;
					b2->nPieces[nToSquare-1] = BLACK_ROOK;
					b2->nTransHash ^= GET_HASH(nToSquare-1, BLACK_ROOK);
					b2->nTransHash ^= GET_HASH(nToSquare+2, BLACK_ROOK);
					b2->nCastling |= BLACK_HAS_CASTLED;
				}
				break;

			#ifdef PARANOID
			default:
				PrintBoard(b,stdout);
				ASSERT(0);
			#endif
		}


		switch (nCapturedPiece) {
			case SQUARE_EMPTY: break;
			case PIECE_PAWN:
				b2->nMaterialHash ^= GET_HASH(CountBits(b2->bbPieces[WHITE][PIECE_PAWN-1]), WHITE_PAWN);
				b2->bbPieces[WHITE][PIECE_ALL-1] ^= bit[nToSquare];
				b2->bbPieces[WHITE][PIECE_PAWN-1] ^= bit[nToSquare];
				b2->nPawnHash ^= GET_HASH(nToSquare, WHITE_PAWN);
				b2->nMaterialHash ^= GET_HASH(CountBits(b2->bbPieces[WHITE][PIECE_PAWN-1]), WHITE_PAWN);
				break;
			case PIECE_KNIGHT:
				b2->nMaterialHash ^= GET_HASH(CountBits(b2->bbPieces[WHITE][PIECE_KNIGHT-1]), WHITE_KNIGHT);
				b2->bbPieces[WHITE][PIECE_ALL-1] ^= bit[nToSquare];
				b2->bbPieces[WHITE][PIECE_KNIGHT-1] ^= bit[nToSquare];
				b2->nMaterialHash ^= GET_HASH(CountBits(b2->bbPieces[WHITE][PIECE_KNIGHT-1]), WHITE_KNIGHT);
				break;
			case PIECE_BISHOP:
				b2->nMaterialHash ^= GET_HASH(CountBits(b2->bbPieces[WHITE][PIECE_BISHOP-1]), WHITE_BISHOP);
				b2->bbPieces[WHITE][PIECE_ALL-1] ^= bit[nToSquare];
				b2->bbPieces[WHITE][PIECE_BISHOP-1] ^= bit[nToSquare];
				//b2->(b->bbPieces[WHITE][PIECE_BISHOP-1]|b->bbPieces[WHITE][PIECE_QUEEN-1]) ^= bit[nToSquare];
				b2->nMaterialHash ^= GET_HASH(CountBits(b2->bbPieces[WHITE][PIECE_BISHOP-1]), WHITE_BISHOP);
				break;
			case PIECE_ROOK:
				b2->nMaterialHash ^= GET_HASH(CountBits(b2->bbPieces[WHITE][PIECE_ROOK-1]), WHITE_ROOK);
				b2->bbPieces[WHITE][PIECE_ALL-1] ^= bit[nToSquare];
				b2->bbPieces[WHITE][PIECE_ROOK-1] ^= bit[nToSquare];
				//b2->(b->bbPieces[WHITE][PIECE_ROOK-1]|b->bbPieces[WHITE][PIECE_QUEEN-1]) ^= bit[nToSquare];
				b2->nCastling &= ~b2->g_bbCastles[nToSquare];
				b2->nMaterialHash ^= GET_HASH(CountBits(b2->bbPieces[WHITE][PIECE_ROOK-1]), WHITE_ROOK);
				break;
			case PIECE_QUEEN:
				b2->nMaterialHash ^= GET_HASH(CountBits(b2->bbPieces[WHITE][PIECE_QUEEN-1]), WHITE_QUEEN);
				b2->bbPieces[WHITE][PIECE_ALL-1] ^= bit[nToSquare];
				//b2->(b->bbPieces[WHITE][PIECE_BISHOP-1]|b->bbPieces[WHITE][PIECE_QUEEN-1]) ^= bit[nToSquare];
				//b2->(b->bbPieces[WHITE][PIECE_ROOK-1]|b->bbPieces[WHITE][PIECE_QUEEN-1]) ^= bit[nToSquare];
				b2->bbPieces[WHITE][PIECE_QUEEN-1] ^= bit[nToSquare];
				b2->nMaterialHash ^= GET_HASH(CountBits(b2->bbPieces[WHITE][PIECE_QUEEN-1]), WHITE_QUEEN);
				break;
			#ifdef _DEBUG
			case PIECE_KING:
				// oh crap, that wasnt supposed to happen...
				PrintBoard(b,stdout);
				ASSERT(0);
				break;
			default:
				PrintBoard(b,stdout);
				ASSERT(0);
			#endif
		}

		// Whites turn to move now
		b2->nSideToMove = WHITE;
	}

	if (nCapturedPiece != SQUARE_EMPTY) {
		// A piece was captured, so dont invert the target square as there is still a piece there
		b2->bbRotate90 ^= bit[g_nRotate90Map[nFromSquare]];
		b2->bbRotate45L ^= bit[g_nRotate45LMap[nFromSquare]];
		b2->bbRotate45R ^= bit[g_nRotate45RMap[nFromSquare]];
		// Reset the repetition detector
		//g_nRepetitionListHead = g_nGameHalfMoveNum+nCurrentDepth; // v0.99 - +depth
	} else {
		b2->bbRotate90 ^= (bit[g_nRotate90Map[nFromSquare]] | bit[g_nRotate90Map[nToSquare]]);
		b2->bbRotate45L ^= (bit[g_nRotate45LMap[nFromSquare]] | bit[g_nRotate45LMap[nToSquare]]);
		b2->bbRotate45R ^= (bit[g_nRotate45RMap[nFromSquare]] | bit[g_nRotate45RMap[nToSquare]]);
	}

	// Check for changes to the castling status and update the hash values accordingly
	nCastleAfter = b2->nCastling ^ nCastleBefore;
	if (nCastleAfter) {
		if (nCastleAfter & WHITE_KING_CASTLE) {
			b2->nTransHash ^= g_nHashWhiteKingCastle;
		}
		if (nCastleAfter & WHITE_QUEEN_CASTLE) {
			b2->nTransHash ^= g_nHashWhiteQueenCastle;
		}
		if (nCastleAfter & BLACK_KING_CASTLE) {
			b2->nTransHash ^= g_nHashBlackKingCastle;
		}
		if (nCastleAfter & BLACK_QUEEN_CASTLE) {
			b2->nTransHash ^= g_nHashBlackQueenCastle;
		}
	}

	#ifdef _DEBUG
	ValidateBoard(b2);
	#endif

}*/

bool IsValidMoveQuick(TMove m, TBoard *b, int nVariant) {
// Quick check to see if the move is valid for this board.


	// No piece being moved!
	if (GET_MOVE_MOVED(m) == 0)
		return false;

	// No move stored (eg no hashed move, initial killer moves)
	//if (m->nFromSquare == 0 && m->nToSquare == 0)
	if (m == 0)
		return false;

	int nFromSquare = GET_MOVE_FROM(m);

	// Moving piece is inconsistent
	if ((b->nPieces[nFromSquare]&SQUARE_PIECE_MASK) != GET_MOVE_MOVED(m))
		return false;

	int nToSquare = GET_MOVE_TO(m);

	// Captures same color piece
	if ((b->nPieces[nToSquare] & SQUARE_COLOR_MASK) == (b->nPieces[nFromSquare] & SQUARE_COLOR_MASK) 
		&& !(GET_MOVE_TYPE(m)==MOVE_TYPE_CASTLE)) // Chess960 might trigger this as the to sq isnt real
		return false;

	// Moving wrong color
	if (((b->nPieces[nFromSquare] & SQUARE_COLOR_MASK)>>4) != b->nSideToMove)
		return false;

	switch (b->nPieces[nFromSquare]) {
		case WHITE_PAWN:
			if (nToSquare-nFromSquare == 16) {
				if ((nFromSquare < H2) || (nFromSquare > A2)) return false;
				if (b->nPieces[nFromSquare+8]) return false; // Cant jump a piece
				if (b->nPieces[nToSquare]) return false; //Target square must not be occupied
			} else if (nToSquare-nFromSquare == 8) {
				if (b->nPieces[nToSquare]) return false; //Target square must not be occupied
				if ((nToSquare > A7) && (GET_MOVE_TYPE(m) != MOVE_TYPE_PROM)) return false;
			} else {
				// Attack move?
				if (GET_MOVE_TYPE(m)==MOVE_TYPE_EP) {
					if (b->nEPSquare != nToSquare) return false;
					if (b->nPieces[nToSquare-8] != BLACK_PAWN) return false;
				} else if (!(g_bbWhitePawnAttacks[nFromSquare] & bit[nToSquare] & b->bbPieces[BLACK][PIECE_ALL-1])) {
					// No piece to attack
					return false;
				} else if ((nToSquare >= H8) && (GET_MOVE_TYPE(m) != MOVE_TYPE_PROM)) {
					// Promotion without special move flag
					return false;
				}
			}
			break;

		case BLACK_PAWN:
			if (nToSquare-nFromSquare == -16) {
				if ((nFromSquare < H7) || (nFromSquare > A7)) return false;
				if (b->nPieces[nFromSquare-8]) return false; // Cant jump a piece
				if (b->nPieces[nToSquare]) return false; //Target square must not be occupied
			} else if (nToSquare-nFromSquare == -8) {
				if (b->nPieces[nToSquare]) return false; //Target square must not be occupied
				if ((nToSquare < H2) && (GET_MOVE_TYPE(m) != MOVE_TYPE_PROM)) return false;
			} else {
				// Attack move?
				if (GET_MOVE_TYPE(m)==MOVE_TYPE_EP) {
					if (b->nEPSquare != nToSquare) return false;
					if (b->nPieces[nToSquare+8] != WHITE_PAWN) return false;
				} else if (!(g_bbBlackPawnAttacks[nFromSquare] & bit[nToSquare] & b->bbPieces[WHITE][PIECE_ALL-1])) {
					// No piece to attack
					return false;
				} else if ((nToSquare <= A1) && (GET_MOVE_TYPE(m) != MOVE_TYPE_PROM)) {
					// Promotion without special move flag
					return false;
				}
			}
			break;

		case WHITE_BISHOP:
		case BLACK_BISHOP:
			if (!(BISHOP_MOVES(nFromSquare) & bit[nToSquare])) return false;
			break;

		case WHITE_ROOK:
		case BLACK_ROOK:
			if (!(ROOK_MOVES(nFromSquare) & bit[nToSquare])) return false;
			break;

		case WHITE_QUEEN:
		case BLACK_QUEEN:
			if (!((BISHOP_MOVES(nFromSquare)|ROOK_MOVES(nFromSquare)) & bit[nToSquare])) return false;
			//if (g_bbBetween[m->nFromSquare][m->nToSquare] & (b->bbPieces[BLACK][PIECE_ALL-1]|b->bbPieces[WHITE][PIECE_ALL-1])) return false;
			break;

		case WHITE_KNIGHT:
		case BLACK_KNIGHT:
			if (!(g_bbKnightAttacks[nFromSquare] & bit[nToSquare])) return false;
			break;

		case WHITE_KING:
			if (nVariant == VARIANT_960) {
				if (GET_MOVE_TYPE(m)==MOVE_TYPE_CASTLE && nToSquare==G1) {
					if (!(b->nCastling & WHITE_KING_CASTLE)) return false;
					int ksq1 = nFromSquare;
					int ksq2 = G1; // final king sq
					int rsq1; for (rsq1=H1;rsq1<A1;rsq1++) { if (b->nPieces[rsq1] == WHITE_ROOK) break; } // find first R from H side
					int rsq2 = F1; // final rook sq
					for (int sq=MIN(ksq2,ksq1);sq<=MAX(ksq2,ksq1);sq++) {
						if (sq!=rsq1 && sq!=ksq1 && sq!=ksq2 && b->nPieces[sq]) { return false; }	// king impeded
						if (IsSquareAttackedBy(b,sq,BLACK)) { return false; }			// attacked
					}
					for (int sq=MIN(rsq2,rsq1);sq<=MAX(rsq2,rsq1);sq++) {
						if (sq!=rsq1 && sq!=rsq2 && sq!=ksq1 && b->nPieces[sq]) return false;	// rook impeded
					}
					if (rsq1 != rsq2 && b->nPieces[rsq2] && b->nPieces[rsq2]!=WHITE_KING) return false;					// piece on final rook square other than king
					if (ksq1 != ksq2 && b->nPieces[ksq2] && 
						(b->nPieces[ksq2]!=WHITE_ROOK || (b->nPieces[ksq2]==WHITE_ROOK && ksq2!=rsq1))) return false;		// piece on final king square other than castling rook
				} else if (GET_MOVE_TYPE(m)==MOVE_TYPE_CASTLE && nToSquare==C1) {
					if (!(b->nCastling & WHITE_QUEEN_CASTLE)) return false;
					int ksq1 = nFromSquare;
					int ksq2 = C1; // final king sq
					int rsq1; for (rsq1=A1;rsq1>H1;rsq1--) { if (b->nPieces[rsq1] == WHITE_ROOK) break; } // find first R from A side
					int rsq2 = D1; // final rook sq
					for (int sq=MIN(ksq2,ksq1);sq<=MAX(ksq2,ksq1);sq++) {
						if (sq!=rsq1 && sq!=ksq1 && sq!=ksq2 && b->nPieces[sq]) { return false; }	// king impeded
						if (IsSquareAttackedBy(b,sq,BLACK)) { return false; }			// attacked
					}
					for (int sq=MIN(rsq2,rsq1);sq<=MAX(rsq2,rsq1);sq++) {
						if (sq!=rsq1 && sq!=rsq2 && sq!=ksq1 && b->nPieces[sq]) return false;	// rook impeded
					}
					if (rsq1 != rsq2 && b->nPieces[rsq2] && b->nPieces[rsq2]!=WHITE_KING) return false;					// piece on final rook square other than king
					if (ksq1 != ksq2 && b->nPieces[ksq2] && 
						(b->nPieces[ksq2]!=WHITE_ROOK || (b->nPieces[ksq2]==WHITE_ROOK && ksq2!=rsq1))) return false;		// piece on final king square other than castling rook
					if (FILE(rsq2) == FILE_B && (b->nPieces[A1] == BLACK_ROOK || b->nPieces[A1] == BLACK_QUEEN)) return false;	// stupid 960 exception to look out for
				}
			} else if (GET_MOVE_TYPE(m)==MOVE_TYPE_CASTLE && nToSquare==G1) {
				if (nFromSquare != E1) return false;
				if (nToSquare != G1) return false;
				if (b->nPieces[F1]) return false;
				if (b->nPieces[G1]) return false;
				if (b->nPieces[H1] != WHITE_ROOK) return false;
				if (IsSquareAttackedBy(b,E1,BLACK)) return false;
				if (IsSquareAttackedBy(b,F1,BLACK)) return false;
				if (IsSquareAttackedBy(b,G1,BLACK)) return false;
				if (!(b->nCastling & WHITE_KING_CASTLE)) return false;
			} else if (GET_MOVE_TYPE(m)==MOVE_TYPE_CASTLE && nToSquare==C1) {
				if (nFromSquare != E1) return false;
				if (nToSquare != C1) return false;
				if (b->nPieces[D1]) return false;
				if (b->nPieces[C1]) return false;
				if (b->nPieces[B1]) return false;
				if (b->nPieces[A1] != WHITE_ROOK) return false;
				if (IsSquareAttackedBy(b,E1,BLACK)) return false;
				if (IsSquareAttackedBy(b,D1,BLACK)) return false;
				if (IsSquareAttackedBy(b,C1,BLACK)) return false;
				if (!(b->nCastling & WHITE_QUEEN_CASTLE)) return false;
			} else {
				if (!(g_bbKingAttacks[nFromSquare] & bit[nToSquare])) return false;
			}
			break;

		case BLACK_KING:
			if (nVariant == VARIANT_960) {
				if (GET_MOVE_TYPE(m)==MOVE_TYPE_CASTLE && nToSquare==G8) {
					if (!(b->nCastling & BLACK_KING_CASTLE)) return false;
					int ksq1 = nFromSquare;
					int ksq2 = G8; // final king sq
					int rsq1; for (rsq1=H8;rsq1<A8;rsq1++) { if (b->nPieces[rsq1] == BLACK_ROOK) break; } // find first R from H side
					int rsq2 = F8; // final rook sq
					for (int sq=MIN(ksq2,ksq1);sq<=MAX(ksq2,ksq1);sq++) {
						if (sq!=rsq1 && sq!=ksq1 && sq!=ksq2 && b->nPieces[sq]) { return false; }	// king impeded
						if (IsSquareAttackedBy(b,sq,WHITE)) { return false; }			// attacked
					}
					for (int sq=MIN(rsq2,rsq1);sq<=MAX(rsq2,rsq1);sq++) {
						if (sq!=rsq1 && sq!=rsq2 && sq!=ksq1 && b->nPieces[sq]) return false;	// rook impeded
					}
					if (rsq1 != rsq2 && b->nPieces[rsq2] && b->nPieces[rsq2]!=BLACK_KING) return false;					// piece on final rook square other than king
					if (ksq1 != ksq2 && b->nPieces[ksq2] && 
						(b->nPieces[ksq2]!=BLACK_ROOK || (b->nPieces[ksq2]==BLACK_ROOK && ksq2!=rsq1))) return false;		// piece on final king square other than castling rook
				} else if (GET_MOVE_TYPE(m)==MOVE_TYPE_CASTLE && nToSquare==C8) {
					if (!(b->nCastling & BLACK_QUEEN_CASTLE)) return false;
					int ksq1 = nFromSquare;
					int ksq2 = C8; // final king sq
					int rsq1; for (rsq1=A8;rsq1>H8;rsq1--) { if (b->nPieces[rsq1] == BLACK_ROOK) break; } // find first R from A side
					int rsq2 = D8; // final rook sq
					for (int sq=MIN(ksq2,ksq1);sq<=MAX(ksq2,ksq1);sq++) {
						if (sq!=rsq1 && sq!=ksq1 && sq!=ksq2 && b->nPieces[sq]) { return false; }	// king impeded
						if (IsSquareAttackedBy(b,sq,WHITE)) { return false; }			// attacked
					}
					for (int sq=MIN(rsq2,rsq1);sq<=MAX(rsq2,rsq1);sq++) {
						if (sq!=rsq1 && sq!=rsq2 && sq!=ksq1 && b->nPieces[sq]) return false;	// rook impeded
					}
					if (rsq1 != rsq2 && b->nPieces[rsq2] && b->nPieces[rsq2]!=BLACK_KING) return false;					// piece on final rook square other than king
					if (ksq1 != ksq2 && b->nPieces[ksq2] && 
						(b->nPieces[ksq2]!=BLACK_ROOK || (b->nPieces[ksq2]==BLACK_ROOK && ksq2!=rsq1))) return false;		// piece on final king square other than castling rook
					if (rsq1==B8 && (b->nPieces[A8] == WHITE_ROOK || b->nPieces[A8] == WHITE_QUEEN)) return false;	// stupid 960 exception to look out for
				}
			} else if (GET_MOVE_TYPE(m)==MOVE_TYPE_CASTLE && nToSquare==G8) {
				if (nFromSquare != E8) return false;
				if (nToSquare != G8) return false;
				if (b->nPieces[F8]) return false;
				if (b->nPieces[G8]) return false;
				if (b->nPieces[H8] != BLACK_ROOK) return false;
				if (IsSquareAttackedBy(b,E8,WHITE)) return false;
				if (IsSquareAttackedBy(b,F8,WHITE)) return false;
				if (IsSquareAttackedBy(b,G8,WHITE)) return false;
				if (!(b->nCastling & BLACK_KING_CASTLE)) return false;
			} else if (GET_MOVE_TYPE(m)==MOVE_TYPE_CASTLE && nToSquare==C8) {
				if (nFromSquare != E8) return false;
				if (nToSquare != C8) return false;
				if (b->nPieces[D8]) return false;
				if (b->nPieces[C8]) return false;
				if (b->nPieces[B8]) return false;
				if (b->nPieces[A8] != BLACK_ROOK) return false;
				if (IsSquareAttackedBy(b,E8,WHITE)) return false;
				if (IsSquareAttackedBy(b,D8,WHITE)) return false;
				if (IsSquareAttackedBy(b,C8,WHITE)) return false;
				if (!(b->nCastling & BLACK_QUEEN_CASTLE)) return false;
			} else {
				if (!(g_bbKingAttacks[nFromSquare] & bit[nToSquare])) return false;
			}
			break;

	}

	// Captured piece is inconsistent
	if ((GET_MOVE_TYPE(m) != MOVE_TYPE_EP) && (GET_MOVE_TYPE(m) != MOVE_TYPE_CASTLE/*for Chess960*/) && (GET_MOVE_CAPTURED(m) != (b->nPieces[nToSquare]&SQUARE_PIECE_MASK)))
		return false;

	// Hopefully by now its proven to be valid

	ASSERT_MOVE(m);
	
	return true;

	PrintBoard(b, stdout);
}


bool Move2Coord(TMove *m, TBoard *b, char *sMove, int nVariant) {
// Create move in m from coord notation in sMove sent via UCI.
// eg a7a8q
	*m = 0;

	int nToSquare;
	int nFromSquare;
	int nMoveType = 0;
	int nPromPiece = 0;

	int nSTM = b->nSideToMove;

	if (!stricmp(sMove,"O-O")) {
		nMoveType = MOVE_TYPE_CASTLE;
		if (nVariant == VARIANT_STD) {
			nFromSquare = nSTM==WHITE ? E1 : E8;
			nToSquare   = nSTM==WHITE ? G1 : G8;
		} else if (nVariant == VARIANT_960) {
			nFromSquare = GetBit(b->bbPieces[nSTM][PIECE_KING-1]);
			nToSquare   = nSTM==WHITE ? G1 : G8;
			//nToSquare = LSB(b->bbPieces[nSTM][PIECE_KING-1]|(nSTM==WHITE?g_bbMaskRank1:g_bbMaskRank8));
		}
	} else if (!stricmp(sMove,"O-O-O")) {
		nMoveType = MOVE_TYPE_CASTLE;
		if (nVariant == VARIANT_STD) {
			nFromSquare = nSTM==WHITE ? E1 : E8;
			nToSquare   = nSTM==WHITE ? C1 : C8;
		} else if (nVariant == VARIANT_960) {
			nFromSquare = GetBit(b->bbPieces[nSTM][PIECE_KING-1]);
			nToSquare   = nSTM==WHITE ? C1 : C8;
			//nToSquare = MSB(b->bbPieces[nSTM][PIECE_KING-1]|(nSTM==WHITE?g_bbMaskRank1:g_bbMaskRank8));
		}
	} else {
		if (((sMove[2]|32) < 'a') || ((sMove[2]|32) > 'h')) return false;
		nToSquare = 'h' - (sMove[2]|32);
		if ((sMove[3] < '1') || (sMove[3] > '8')) return false;
		nToSquare += 8*(sMove[3]-'1');

		if (((sMove[0]|32) < 'a') || ((sMove[0]|32) > 'h')) return false;
		nFromSquare = 'h' - (sMove[0]|32);
		if ((sMove[1] < '1') || (sMove[1] > '8')) return false;
		nFromSquare += 8*(sMove[1]-'1');
	}
	
	// Promotion
	switch (sMove[4]|32) {
		case 'q':
			nMoveType = MOVE_TYPE_PROM;
			nPromPiece = PIECE_QUEEN;
			break;
		case 'r':
			nMoveType = MOVE_TYPE_PROM;
			nPromPiece = PIECE_ROOK;
			break;
		case 'b':
			nMoveType = MOVE_TYPE_PROM;
			nPromPiece = PIECE_BISHOP;
			break;
		case 'n':
			nMoveType = MOVE_TYPE_PROM;
			nPromPiece = PIECE_KNIGHT;
			break;
		case '\0':
			break;
	}
	
	// Castle
	if (nVariant == VARIANT_960) {
		if ((b->nPieces[nFromSquare] == WHITE_KING) && (b->nPieces[nToSquare] == WHITE_ROOK)) {	// KxR
			nMoveType = MOVE_TYPE_CASTLE;
			int r1 = GetBlackBit(b->bbPieces[WHITE][PIECE_ROOK-1] & g_bbMaskRank1);	// find first R from A side
			int nr = CountBits(b->bbPieces[WHITE][PIECE_ROOK-1] & g_bbMaskRank1);
			if (nr==1) { // only one possibility, check the castling flags
				if (b->nCastling & WHITE_KING_CASTLE) nToSquare = G1;
				else nToSquare = C1;
			} else if (nr==2) { // check if the rook is on left or right
				if (nToSquare == r1)
					nToSquare = C1;
				else
					nToSquare = G1;
			}
		} else if ((b->nPieces[nFromSquare] == BLACK_KING) && (b->nPieces[nToSquare] == BLACK_ROOK)) {	// KxR
			nMoveType = MOVE_TYPE_CASTLE;
			int r1 = GetBlackBit(b->bbPieces[BLACK][PIECE_ROOK-1] & g_bbMaskRank8);	// find first R from A side
			int nr = CountBits(b->bbPieces[BLACK][PIECE_ROOK-1] & g_bbMaskRank8);
			if (nr==1) { // only one possibility, check the castling flags
				if (b->nCastling & BLACK_KING_CASTLE) nToSquare = G8;
				else nToSquare = C8;
			} else if (nr==2) { // check if the rook is on left or right
				if (nToSquare == r1)
					nToSquare = C8;
				else
					nToSquare = G8;
			}
		}
	} else {
		if ((b->nPieces[nFromSquare] == WHITE_KING) && (nFromSquare == E1) && (nToSquare == G1)) {
			nMoveType = MOVE_TYPE_CASTLE;
		} else
		if ((b->nPieces[nFromSquare] == WHITE_KING) && (nFromSquare == E1) && (nToSquare == C1)) {
			nMoveType = MOVE_TYPE_CASTLE;
		} else
		if ((b->nPieces[nFromSquare] == BLACK_KING) && (nFromSquare == E8) && (nToSquare == G8)) {
			nMoveType = MOVE_TYPE_CASTLE;
		} else
		if ((b->nPieces[nFromSquare] == BLACK_KING) && (nFromSquare == E8) && (nToSquare == C8)) {
			nMoveType = MOVE_TYPE_CASTLE;
		}
	}

	// EP
	int nCap = 0;
	if ((b->nPieces[nFromSquare] == WHITE_PAWN) && (nToSquare-nFromSquare != 8) && (nToSquare-nFromSquare != 16) && (b->nPieces[nToSquare] == SQUARE_EMPTY)) {
		nMoveType = MOVE_TYPE_EP;
		nCap = PIECE_PAWN;
	}
	if ((b->nPieces[nFromSquare] == BLACK_PAWN) && (nFromSquare-nToSquare != 8) && (nFromSquare-nToSquare != 16) && (b->nPieces[nToSquare] == SQUARE_EMPTY)) {
		nMoveType = MOVE_TYPE_EP;
		nCap = PIECE_PAWN;
	}

	if (!nCap && nMoveType != MOVE_TYPE_CASTLE) { // Chess960 might look like a capture for some castling moves
		nCap = b->nPieces[nToSquare]&SQUARE_PIECE_MASK;
	}

	SET_MOVE(*m, nFromSquare, nToSquare, nMoveType, nPromPiece, b->nPieces[nFromSquare]&SQUARE_PIECE_MASK, nCap);

	return true;
}


char *GetNotation(TBoard *b, TMove m) {
/* 
This function generates the standard algebraic notation for a move. Note that this function is
   time consuming and shouldnt be called within the main search.
IN:		m			The move to make
		b			The board to make the move on
OUT:	m.notation	The string representing the algebraic notation of the move
ASSUMES: The move has not yet been made on the board.
*/
	register short pos=0;
	//BoardType tmp;
	char *sNotation;
	sNotation = new char[8]; // Freed by calling function
	int nFromSquare = GET_MOVE_FROM(m);
	int nToSquare = GET_MOVE_TO(m);

	//if (m->nFromSquare == m->nToSquare) {
	if (m == 0) {
		sNotation[0] = '\0';
		return sNotation;
	}

	if (GET_MOVE_TYPE(m)==MOVE_TYPE_CASTLE && FILE(GET_MOVE_TO(m))==1) {
		strcpy(sNotation,"O-O");
		return sNotation;
	} else if (GET_MOVE_TYPE(m)==MOVE_TYPE_CASTLE && FILE(GET_MOVE_TO(m))==5) {
		strcpy(sNotation,"O-O-O");
		return sNotation;
	}

	switch (GET_MOVE_MOVED(m)) {
		case PIECE_PAWN: 
			if (b->nPieces[nToSquare] || (GET_MOVE_TYPE(m)==MOVE_TYPE_EP)) sNotation[pos++] = 'h' - FILE(nFromSquare);
			break;
		case PIECE_KNIGHT: 
			sNotation[pos++] = 'N';
			if (b->nSideToMove == WHITE) {
				if (CountBits(g_bbKnightAttacks[nToSquare] & b->bbPieces[WHITE][PIECE_KNIGHT-1] & g_bbMaskRank[RANK(nFromSquare)]) > 1) 
					sNotation[pos++] = 'h' - FILE(nFromSquare);
				else if (CountBits(g_bbKnightAttacks[nToSquare] & b->bbPieces[WHITE][PIECE_KNIGHT-1] & g_bbMaskFile[FILE(nFromSquare)]) > 1) 
					sNotation[pos++] = '1' + RANK(nFromSquare); // v2.71
				else if (CountBits(g_bbKnightAttacks[nToSquare] & b->bbPieces[WHITE][PIECE_KNIGHT-1]) > 1)
					sNotation[pos++] = 'h' - FILE(nFromSquare);	// either descriptor will do
			} else {
				if (CountBits(g_bbKnightAttacks[nToSquare] & b->bbPieces[BLACK][PIECE_KNIGHT-1] & g_bbMaskRank[RANK(nFromSquare)]) > 1) 
					sNotation[pos++] = 'h' - FILE(nFromSquare);
				else if (CountBits(g_bbKnightAttacks[nToSquare] & b->bbPieces[BLACK][PIECE_KNIGHT-1] & g_bbMaskFile[FILE(nFromSquare)]) > 1) 
					sNotation[pos++] = '1' + RANK(nFromSquare);
				else if (CountBits(g_bbKnightAttacks[nToSquare] & b->bbPieces[BLACK][PIECE_KNIGHT-1]) > 1) 
					sNotation[pos++] = 'h' - FILE(nFromSquare);
			}
			break;
		case PIECE_BISHOP: 
			sNotation[pos++] = 'B'; 
			if (b->nSideToMove == WHITE) {
				if (CountBits(BISHOP_MOVES(nToSquare) & b->bbPieces[WHITE][PIECE_BISHOP-1] & g_bbMaskRank[RANK(nFromSquare)]) > 1) 
					sNotation[pos++] = 'h' - FILE(nFromSquare);
				else if (CountBits(BISHOP_MOVES(nToSquare) & b->bbPieces[WHITE][PIECE_BISHOP-1] & g_bbMaskFile[FILE(nFromSquare)]) > 1) 
					sNotation[pos++] = '1' + RANK(nFromSquare);
				else if (CountBits(BISHOP_MOVES(nToSquare) & b->bbPieces[WHITE][PIECE_BISHOP-1]) > 1)
					sNotation[pos++] = 'h' - FILE(nFromSquare);
			} else {
				if (CountBits(BISHOP_MOVES(nToSquare) & b->bbPieces[BLACK][PIECE_BISHOP-1] & g_bbMaskRank[RANK(nFromSquare)]) > 1) 
					sNotation[pos++] = 'h' - FILE(nFromSquare);
				else if (CountBits(BISHOP_MOVES(nToSquare) & b->bbPieces[BLACK][PIECE_BISHOP-1] & g_bbMaskFile[FILE(nFromSquare)]) > 1) 
					sNotation[pos++] = '1' + RANK(nFromSquare);
				else if (CountBits(BISHOP_MOVES(nToSquare) & b->bbPieces[BLACK][PIECE_BISHOP-1]) > 1) 
					sNotation[pos++] = 'h' - FILE(nFromSquare);
			}
			break;
		case PIECE_ROOK:
			sNotation[pos++] = 'R'; 
			if (b->nSideToMove == WHITE) {
				if (CountBits(ROOK_MOVES(nToSquare) & b->bbPieces[WHITE][PIECE_ROOK-1] & g_bbMaskRank[RANK(nFromSquare)]) > 1) 
					sNotation[pos++] = 'h' - FILE(nFromSquare);
				else if (CountBits(ROOK_MOVES(nToSquare) & b->bbPieces[WHITE][PIECE_ROOK-1] & g_bbMaskFile[FILE(nFromSquare)]) > 1) 
					sNotation[pos++] = '1' + RANK(nFromSquare);
				else if (CountBits(ROOK_MOVES(nToSquare) & b->bbPieces[WHITE][PIECE_ROOK-1]) > 1) 
					sNotation[pos++] = 'h' - FILE(nFromSquare);
			} else {
				if (CountBits(ROOK_MOVES(nToSquare) & b->bbPieces[BLACK][PIECE_ROOK-1] & g_bbMaskRank[RANK(nFromSquare)]) > 1) 
					sNotation[pos++] = 'h' - FILE(nFromSquare);
				else if (CountBits(ROOK_MOVES(nToSquare) & b->bbPieces[BLACK][PIECE_ROOK-1] & g_bbMaskFile[FILE(nFromSquare)]) > 1) 
					sNotation[pos++] = '1' + RANK(nFromSquare);
				else if (CountBits(ROOK_MOVES(nToSquare) & b->bbPieces[BLACK][PIECE_ROOK-1]) > 1) 
					sNotation[pos++] = 'h' - FILE(nFromSquare);
			}
			break;
		case PIECE_QUEEN: 
			sNotation[pos++] = 'Q'; 
			if (b->nSideToMove == WHITE) {
				if (CountBits((ROOK_MOVES(nToSquare) | BISHOP_MOVES(nToSquare)) & b->bbPieces[WHITE][PIECE_QUEEN-1] & g_bbMaskRank[RANK(nFromSquare)]) > 1) 
					sNotation[pos++] = 'h' - FILE(nFromSquare);
				else if (CountBits((ROOK_MOVES(nToSquare) | BISHOP_MOVES(nToSquare)) & b->bbPieces[WHITE][PIECE_QUEEN-1] & g_bbMaskFile[FILE(nFromSquare)]) > 1) 
					sNotation[pos++] = '1' + RANK(nFromSquare);
				else if (CountBits((ROOK_MOVES(nToSquare) | BISHOP_MOVES(nToSquare)) & b->bbPieces[WHITE][PIECE_QUEEN-1]) > 1) 
					sNotation[pos++] = 'h' - FILE(nFromSquare);
			} else {
				if (CountBits((ROOK_MOVES(nToSquare) | BISHOP_MOVES(nToSquare)) & b->bbPieces[BLACK][PIECE_QUEEN-1] & g_bbMaskRank[RANK(nFromSquare)]) > 1) 
					sNotation[pos++] = 'h' - FILE(nFromSquare);
				else if (CountBits((ROOK_MOVES(nToSquare) | BISHOP_MOVES(nToSquare)) & b->bbPieces[BLACK][PIECE_QUEEN-1] & g_bbMaskFile[FILE(nFromSquare)]) > 1) 
					sNotation[pos++] = '1' + RANK(nFromSquare);
				else if (CountBits((ROOK_MOVES(nToSquare) | BISHOP_MOVES(nToSquare)) & b->bbPieces[BLACK][PIECE_QUEEN-1]) > 1) 
					sNotation[pos++] = 'h' - FILE(nFromSquare);
			}
			break;
		case PIECE_KING: 
			sNotation[pos++] = 'K'; 
			break;
	}

	if (GET_MOVE_CAPTURED(m)) {
		// Capture
		sNotation[pos++] = 'x';
	}

	// Destination Square
	sNotation[pos++] = 'h' - FILE(nToSquare);
	sNotation[pos++] = '1' + RANK(nToSquare);

	// Promotion
	if (GET_MOVE_MOVED(m) == PIECE_PAWN) {
		if ((nToSquare<H2) || (nToSquare>A7)) {
			switch (GET_MOVE_PROM(m)) {
				case PIECE_QUEEN: sNotation[pos++] = '='; sNotation[pos++] = 'Q'; break;
				case PIECE_ROOK: sNotation[pos++] = '='; sNotation[pos++] = 'R'; break;
				case PIECE_BISHOP: sNotation[pos++] = '='; sNotation[pos++] = 'B'; break;
				case PIECE_KNIGHT: sNotation[pos++] = '='; sNotation[pos++] = 'N'; break;
				default: sNotation[pos++] = '?'; break;
			}
		}
	}

	// Check given
	/*int nOldRepListHead;
	nOldRepListHead = g_tThreads.tData[0].nRepetitionListHead;
	TUndoMove u;
	MakeMove2(b,m,&u,0,0,);
	g_tThreads.tData[0].nRepetitionListHead = nOldRepListHead;
	if ((b->nSideToMove==WHITE && IsSquareAttackedBy(b,GetBit(b->bbPieces[WHITE][PIECE_KING-1]),BLACK)) 
		|| (b->nSideToMove==BLACK && IsSquareAttackedBy(b,GetBit(b->bbPieces[BLACK][PIECE_KING-1]),WHITE))) {
		sNotation[pos++] = '+';
	}
	UnMakeMove(b,m,&u,0);*/

	sNotation[pos++] = '\0';

	return sNotation;
}

void MakeMove2(TBoard *b, TMove m, TUndoMove *undo, int nCurrentDepth, int nThreadID, int nVariant) {
// Applies the move m to board b.
// In:
//	b - pointer to board to move on
//	m - move to make
// Out:
//  b - Board after move is made
//	nEP - EP square prior to making move (used for unmake)
//	nCastle - Castling status prior to making move (used for unmake)
//	nCapPc - Captured piece (used for unmake)


	int nSideToMove = b->nSideToMove;
	int nFromSquare = GET_MOVE_FROM(m);
	int nToSquare = GET_MOVE_TO(m);
	int nMoveType = GET_MOVE_TYPE(m);
	int nPromPiece = GET_MOVE_PROM(m);
	int nMovedSquare = b->nPieces[nFromSquare];						// Save piece+color
	int nMovedPiece = nMovedSquare & SQUARE_PIECE_MASK;				// Save piece without color
	int nCapturedSquare = b->nPieces[nToSquare];					// Save piece+color
	int nCapturedPiece = nCapturedSquare & SQUARE_PIECE_MASK;		// Save piece without color
	if (nVariant==VARIANT_960) {
		// Castling move where King doesnt have to actually move (from==tq) or swaps squares with its Rook
		if (nCapturedPiece==PIECE_KING)	nCapturedPiece = 0;
		else if (nSideToMove==WHITE && nCapturedSquare==WHITE_ROOK) nCapturedPiece = 0;
		else if (nSideToMove==BLACK && nCapturedSquare==BLACK_ROOK) nCapturedPiece = 0;
	}
	BitBoard bbFromTo;		// Bits in the From and To squares for quick modifications to the bitboards
	int nCastleBefore, nCastleAfter;

	ASSERT(b->nSideToMove == (nMovedSquare & SQUARE_COLOR_MASK)>>4);
	ASSERT_MOVE(m);
	ASSERT(m);
	ASSERT(nMovedPiece);
	//ASSERT(IsValidMove(m, b));
	ASSERT(nVariant==VARIANT_960 || nMoveType==MOVE_TYPE_CASTLE || (b->nPieces[nFromSquare] & SQUARE_COLOR_MASK) != (b->nPieces[nToSquare] & SQUARE_COLOR_MASK));
	ASSERT(nFromSquare>=0);
	ASSERT(nFromSquare<64);
	ASSERT(nToSquare>=0);
	ASSERT(nToSquare<64);

	//#ifdef _DEBUG
	//ValidateBoard(b);
	//#endif

	// Save the variables required to undo this move
	undo->nEPSquare = b->nEPSquare;
	undo->nCastleFlags = b->nCastling;
	undo->nCapturedPiece = nCapturedSquare;
	undo->n50Moves = b->nFiftyMoveCount;
	undo->nTransHash = b->nTransHash;
	undo->nPawnHash = b->nPawnHash;
	undo->nMaterialHash = b->nMaterialHash;
	undo->nRepListHead = g_nRepetitionListHead[nThreadID];
	undo->nMaterial[WHITE] = b->nMaterial[WHITE];
	undo->nMaterial[BLACK] = b->nMaterial[BLACK];
	//undo->nInCheck = b->nInCheck;
	
	//#ifdef _DEBUG
	// Copy source board to dest board.
	//TBoard bx;
	//memcpy(&bx, b, sizeof(*b));
	//#endif

	nCastleBefore = b->nCastling;
	if (nCapturedPiece || (nMoveType == MOVE_TYPE_EP)) {
		// Reset the current fifty move counter if there is a capture
		b->nTransHash ^= g_nHash50Move[b->nFiftyMoveCount];	// Turn off the old 50 move count
		b->nFiftyMoveCount = 0;
	} else {
		b->nTransHash ^= g_nHash50Move[b->nFiftyMoveCount];	// Turn off the old 50 move count
		b->nFiftyMoveCount++;
		b->nTransHash ^= g_nHash50Move[b->nFiftyMoveCount];	// Turn on the new 50 move count
	}
	if (b->nEPSquare != NULL_SQUARE) {
		b->nTransHash ^= g_nHashEPMove[b->nEPSquare];
	}
	b->nEPSquare = NULL_SQUARE; // Must reset every move unless a two-rank pawn move.

	// Update the board's hash value and checksum
	b->nTransHash ^= GET_HASH_COL(nFromSquare, nSideToMove, nMovedPiece);// g_nHashValues[nFromSquare][nMovedSquare];
	b->nTransHash ^= GET_HASH_COL(nToSquare, nSideToMove, nMovedPiece);
	b->nTransHash ^= GET_HASH_COL(nToSquare, OPP(nSideToMove), nCapturedPiece);
	b->nTransHash ^= g_nHashWhiteToMove;
	b->nPawnHash ^= g_nHashWhiteToMove;

	// Change the bit boards
	if (nFromSquare != nToSquare) {
		bbFromTo = bit[nFromSquare] | bit[nToSquare]; // Contains two set bits
		b->nPieces[nToSquare]   = b->nPieces[nFromSquare];
		b->nPieces[nFromSquare] = SQUARE_EMPTY;
	} else {
		bbFromTo = 0; // Chess960 castling where King is already on final square
	}

	if (nSideToMove == WHITE) {
		// Update all white specific data

		b->bbPieces[WHITE][PIECE_ALL-1] ^= bbFromTo;
		b->nMaterial[BLACK] -= g_nPieceValues[nCapturedPiece];

		// v1.05.09 - condense logic and remove some branches
		b->bbPieces[WHITE][nMovedPiece-1] ^= bbFromTo;

		switch (nMovedPiece) {
			case PIECE_PAWN:
				// Check for a valid EnPassent square.
				if (nToSquare - nFromSquare == 16) {
					// pawn moved two ranks
					b->nEPSquare = nToSquare - 8;
					b->nTransHash ^= g_nHashEPMove[b->nEPSquare];
				}
				// Check for an EP move
				if (nMoveType == MOVE_TYPE_EP) {
					// Remove the captured pawn
					b->nMaterialHash ^= GET_HASH(CountBits(b->bbPieces[BLACK][PIECE_PAWN-1]), BLACK_PAWN);
					b->bbPieces[BLACK][PIECE_PAWN-1] ^= bit[nToSquare-8];
					b->nMaterialHash ^= GET_HASH(CountBits(b->bbPieces[BLACK][PIECE_PAWN-1]), BLACK_PAWN);
					b->bbPieces[BLACK][PIECE_ALL-1] ^= bit[nToSquare-8];
					b->bbRotate90 ^= bit[g_nRotate90Map[nToSquare-8]];
					b->bbRotate45L ^= bit[g_nRotate45LMap[nToSquare-8]];
					b->bbRotate45R ^= bit[g_nRotate45RMap[nToSquare-8]];
					b->nPieces[nToSquare-8] = SQUARE_EMPTY;
					b->nTransHash ^= GET_HASH(nToSquare-8, BLACK_PAWN);
					b->nPawnHash ^= GET_HASH(nToSquare-8, BLACK_PAWN);
					b->nMaterial[BLACK] -= SCORE_PAWN;
				}
				// Update Pawn Hash
				b->nPawnHash ^= GET_HASH(nFromSquare, WHITE_PAWN);
				// Check for a promotion
				if (nToSquare >= H8) {
					b->nMaterialHash ^= GET_HASH(CountBits(b->bbPieces[WHITE][PIECE_PAWN-1]), WHITE_PAWN);
					b->bbPieces[WHITE][PIECE_PAWN-1] ^= bit[nToSquare];	// Remove the promoted pawn
					b->nMaterialHash ^= GET_HASH(CountBits(b->bbPieces[WHITE][PIECE_PAWN-1]), WHITE_PAWN);

					b->nMaterialHash ^= GET_HASH(CountBits(b->bbPieces[WHITE][nPromPiece-1]), PIECE_WHITE | nPromPiece);
					b->bbPieces[WHITE][nPromPiece-1] ^= bit[nToSquare];
					b->nPieces[nToSquare] = PIECE_WHITE | nPromPiece;
					b->nTransHash ^= GET_HASH(nToSquare, WHITE_PAWN);
					b->nTransHash ^= GET_HASH(nToSquare, PIECE_WHITE | nPromPiece);
					//b->nMaterial[WHITE] += g_nPieceScore[nPromPiece] - SCORE_PAWN;
					b->nMaterialHash ^= GET_HASH(CountBits(b->bbPieces[WHITE][nPromPiece-1]), PIECE_WHITE | nPromPiece);
				} else { // no promotion
					// Update Pawn Hash
					b->nPawnHash ^= GET_HASH(nToSquare, WHITE_PAWN);
				}
				b->nTransHash ^= g_nHash50Move[b->nFiftyMoveCount];	// Turn off the new 50 move count
				// Reset the fifty move counter because a pawn moved.
				b->nFiftyMoveCount = 0;
				// Reset the repetition detector
				//g_tThreads.tData[nThreadID].nRepetitionListHead = g_tGame.nGameHalfMoveNum+nCurrentDepth; // v0.99 - +depth
				break;

			case PIECE_ROOK:

				b->nCastling &= ~b->g_bbCastles[nFromSquare];
				break;

			case PIECE_KING:

				b->nCastling &= WHITE_CANT_CASTLE;
				if (nVariant==VARIANT_960) {
					// From = Ksq, To = A8 or H8 to indicate which side, need to determine which rook to use
					if (nMoveType==MOVE_TYPE_CASTLE && nToSquare==G1) {
						int rsq = GetWhiteBit(b->bbPieces[WHITE][PIECE_ROOK-1] & g_bbMaskRank1);	// find first R from H side
						undo->nOrigRookSq = rsq;
						b->bbPieces[WHITE][PIECE_ROOK-1] ^= bit[rsq]^bit[F1]; // Use ^ instead of | in case rsq and F1 are the same square
						b->bbPieces[WHITE][PIECE_ALL-1]  ^= bit[rsq]^bit[F1];
						b->bbRotate90  ^= bit[g_nRotate90Map[rsq]]  ^ bit[g_nRotate90Map[F1]];
						b->bbRotate45L ^= bit[g_nRotate45LMap[rsq]] ^ bit[g_nRotate45LMap[F1]];
						b->bbRotate45R ^= bit[g_nRotate45RMap[rsq]] ^ bit[g_nRotate45RMap[F1]];
						if (rsq!=nToSquare) b->nPieces[rsq] = SQUARE_EMPTY;
						b->nPieces[F1]  = WHITE_ROOK;
						b->nTransHash  ^= GET_HASH(rsq, WHITE_ROOK);
						b->nTransHash  ^= GET_HASH(F1,  WHITE_ROOK);
						b->nCastling   |= WHITE_HAS_CASTLED;

					} else if (nMoveType==MOVE_TYPE_CASTLE && nToSquare==C1) {
						int rsq = GetBlackBit(b->bbPieces[WHITE][PIECE_ROOK-1] & g_bbMaskRank1);	// find first R from A side
						undo->nOrigRookSq = rsq;
						b->bbPieces[WHITE][PIECE_ROOK-1] ^= bit[rsq]^bit[D1];
						b->bbPieces[WHITE][PIECE_ALL-1]  ^= bit[rsq]^bit[D1];
						b->bbRotate90  ^= bit[g_nRotate90Map[rsq]]  ^ bit[g_nRotate90Map[D1]];
						b->bbRotate45L ^= bit[g_nRotate45LMap[rsq]] ^ bit[g_nRotate45LMap[D1]];
						b->bbRotate45R ^= bit[g_nRotate45RMap[rsq]] ^ bit[g_nRotate45RMap[D1]];
						if (rsq!=nToSquare) b->nPieces[rsq]	= SQUARE_EMPTY;
						b->nPieces[D1]  = WHITE_ROOK;
						b->nTransHash  ^= GET_HASH(rsq, WHITE_ROOK);
						b->nTransHash  ^= GET_HASH(D1,  WHITE_ROOK);
						b->nCastling   |= WHITE_HAS_CASTLED;
					}
				} else {
					if (nMoveType==MOVE_TYPE_CASTLE && nToSquare==G1) {
						// Castling King side - so move rook too
						b->bbPieces[WHITE][PIECE_ROOK-1] ^= MOVE_WHITE_ROOK_CASTLE_K;
						b->bbPieces[WHITE][PIECE_ALL-1] ^= MOVE_WHITE_ROOK_CASTLE_K;
						b->bbRotate90 ^= MOVE_WHITE_ROOK_CASTLE_K_90;
						b->bbRotate45L ^= MOVE_WHITE_ROOK_CASTLE_K_45L;
						b->bbRotate45R ^= MOVE_WHITE_ROOK_CASTLE_K_45R;
						b->nPieces[nToSquare-1] = SQUARE_EMPTY;
						b->nPieces[nToSquare+1] = WHITE_ROOK;
						b->nTransHash ^= GET_HASH(nToSquare-1, WHITE_ROOK);
						b->nTransHash ^= GET_HASH(nToSquare+1, WHITE_ROOK);
						b->nCastling |= WHITE_HAS_CASTLED;

					} else if (nMoveType==MOVE_TYPE_CASTLE && nToSquare==C1) {
						// Castling Queen side - so move rook too
						b->bbPieces[WHITE][PIECE_ROOK-1] ^= MOVE_WHITE_ROOK_CASTLE_Q;
						b->bbPieces[WHITE][PIECE_ALL-1] ^= MOVE_WHITE_ROOK_CASTLE_Q;
						b->bbRotate90 ^= MOVE_WHITE_ROOK_CASTLE_Q_90;
						b->bbRotate45L ^= MOVE_WHITE_ROOK_CASTLE_Q_45L;
						b->bbRotate45R ^= MOVE_WHITE_ROOK_CASTLE_Q_45R;
						b->nPieces[nToSquare+2] = SQUARE_EMPTY;
						b->nPieces[nToSquare-1] = WHITE_ROOK;
						b->nTransHash ^= GET_HASH(nToSquare-1, WHITE_ROOK);
						b->nTransHash ^= GET_HASH(nToSquare+2, WHITE_ROOK);
						b->nCastling |= WHITE_HAS_CASTLED;
					}
				}
				break;

		}

		// v1.05.09 - reduce branches
		if (nCapturedPiece > 0) {

			b->nMaterialHash ^= GET_HASH_COL(CountBits(b->bbPieces[BLACK][nCapturedPiece-1]), BLACK, nCapturedPiece);
			b->bbPieces[BLACK][PIECE_ALL-1] ^= bit[nToSquare];
			b->bbPieces[BLACK][nCapturedPiece-1] ^= bit[nToSquare];
			b->nMaterialHash ^= GET_HASH_COL(CountBits(b->bbPieces[BLACK][nCapturedPiece-1]), BLACK, nCapturedPiece);

			switch (nCapturedPiece) {
				case PIECE_PAWN:
					b->nPawnHash ^= GET_HASH(nToSquare, BLACK_PAWN);
					break;
				case PIECE_ROOK:
					b->nCastling &= ~b->g_bbCastles[nToSquare];
					break;
				#ifdef _DEBUG
				case PIECE_KING:
					// oh crap, that wasnt supposed to happen...
					PrintBoard(b,stdout);
					ASSERT(0);
				#endif
			}
		}

		// Blacks turn to move now
		b->nSideToMove = BLACK;

	} else {
		// Update all black specific data

		b->bbPieces[BLACK][PIECE_ALL-1] ^= bbFromTo;
		b->nMaterial[WHITE] -= g_nPieceValues[nCapturedPiece];

		// v1.05.09 - condense logic and remove some branches
		b->bbPieces[BLACK][nMovedPiece-1] ^= bbFromTo;
		
		switch (nMovedPiece) {
			case PIECE_PAWN:
				// Check for a valid EnPassent square.
				if (nFromSquare - nToSquare == 16) {
					// pawn moved two ranks
					b->nEPSquare = nToSquare + 8;
					b->nTransHash ^= g_nHashEPMove[b->nEPSquare];
				}
				// Check for an EP move
				if (nMoveType == MOVE_TYPE_EP) {
					b->nMaterialHash ^= GET_HASH(CountBits(b->bbPieces[WHITE][PIECE_PAWN-1]), WHITE_PAWN);
					b->bbPieces[WHITE][PIECE_PAWN-1] ^= bit[nToSquare+8];
					b->nMaterialHash ^= GET_HASH(CountBits(b->bbPieces[WHITE][PIECE_PAWN-1]), WHITE_PAWN);
					b->bbPieces[WHITE][PIECE_ALL-1] ^= bit[nToSquare+8];
					b->bbRotate90 ^= bit[g_nRotate90Map[nToSquare+8]];
					b->bbRotate45L ^= bit[g_nRotate45LMap[nToSquare+8]];
					b->bbRotate45R ^= bit[g_nRotate45RMap[nToSquare+8]];
					b->nPieces[nToSquare+8] = SQUARE_EMPTY;
					b->nTransHash ^= GET_HASH(nToSquare+8, WHITE_PAWN);
					b->nPawnHash ^= GET_HASH(nToSquare+8, WHITE_PAWN);
					b->nMaterial[WHITE] -= SCORE_PAWN;
				}
				// Update Pawn Hash
				b->nPawnHash ^= GET_HASH(nFromSquare, BLACK_PAWN);
				// Check for a promotion
				if (nToSquare <= A1) {
					b->nMaterialHash ^= GET_HASH(CountBits(b->bbPieces[BLACK][PIECE_PAWN-1]), BLACK_PAWN);
					b->bbPieces[BLACK][PIECE_PAWN-1] ^= bit[nToSquare]; // Remove the promoted pawn
					b->nMaterialHash ^= GET_HASH(CountBits(b->bbPieces[BLACK][PIECE_PAWN-1]), BLACK_PAWN);

					b->nMaterialHash ^= GET_HASH(CountBits(b->bbPieces[BLACK][nPromPiece-1]), PIECE_BLACK | nPromPiece);
					b->bbPieces[BLACK][nPromPiece-1] ^= bit[nToSquare];
					b->nPieces[nToSquare] = PIECE_BLACK | nPromPiece;
					b->nTransHash ^= GET_HASH(nToSquare, BLACK_PAWN);
					b->nTransHash ^= GET_HASH(nToSquare, PIECE_BLACK | nPromPiece);
					//b->nMaterial[BLACK] += g_nPieceScore[nPromPiece] - SCORE_PAWN;
					b->nMaterialHash ^= GET_HASH(CountBits(b->bbPieces[BLACK][nPromPiece-1]), PIECE_BLACK | nPromPiece);
				} else { // no promotion
					// Update Pawn Hash
					b->nPawnHash ^= GET_HASH(nToSquare, BLACK_PAWN);
				}
				b->nTransHash ^= g_nHash50Move[b->nFiftyMoveCount];	// Turn off the new 50 move count
				// Reset the fifty move counter because a pawn moved.
				b->nFiftyMoveCount = 0;
				// Reset the repetition detector
				//g_tThreads.tData[nThreadID].nRepetitionListHead = g_tGame.nGameHalfMoveNum+nCurrentDepth; // v0.99 - +depth
				break;

			case PIECE_ROOK:
				b->nCastling &= ~b->g_bbCastles[nFromSquare];
				break;

			case PIECE_KING:
				b->nCastling &= BLACK_CANT_CASTLE;
				if (nVariant==VARIANT_960) {
					// From = Ksq, To = A8 or H8 to indicate which side, need to determine which rook to use
					if (nMoveType==MOVE_TYPE_CASTLE && nToSquare==G8) {
						// Castling King side - so move rook too
						int rsq = GetWhiteBit(b->bbPieces[BLACK][PIECE_ROOK-1] & g_bbMaskRank8);	// find first R from H side
						undo->nOrigRookSq = rsq;
						b->bbPieces[BLACK][PIECE_ROOK-1] ^= bit[rsq]^bit[F8];
						b->bbPieces[BLACK][PIECE_ALL-1]  ^= bit[rsq]^bit[F8];
						b->bbRotate90  ^= bit[g_nRotate90Map[rsq]]  ^ bit[g_nRotate90Map[F8]];
						b->bbRotate45L ^= bit[g_nRotate45LMap[rsq]] ^ bit[g_nRotate45LMap[F8]];
						b->bbRotate45R ^= bit[g_nRotate45RMap[rsq]] ^ bit[g_nRotate45RMap[F8]];
						if (rsq!=nToSquare) b->nPieces[rsq] = SQUARE_EMPTY;
						b->nPieces[F8]  = BLACK_ROOK;
						b->nTransHash  ^= GET_HASH(rsq, BLACK_ROOK);
						b->nTransHash  ^= GET_HASH(F8,  BLACK_ROOK);
						b->nCastling   |= BLACK_HAS_CASTLED;
					} else if (nMoveType==MOVE_TYPE_CASTLE && nToSquare==C8) {
						// Castling Queen side - so move rook too
						int rsq = GetBlackBit(b->bbPieces[BLACK][PIECE_ROOK-1] & g_bbMaskRank8);	// find first R from A side
						undo->nOrigRookSq = rsq;
						b->bbPieces[BLACK][PIECE_ROOK-1] ^= bit[rsq]^bit[D8];
						b->bbPieces[BLACK][PIECE_ALL-1]  ^= bit[rsq]^bit[D8];
						b->bbRotate90  ^= bit[g_nRotate90Map[rsq]]  ^ bit[g_nRotate90Map[D8]];
						b->bbRotate45L ^= bit[g_nRotate45LMap[rsq]] ^ bit[g_nRotate45LMap[D8]];
						b->bbRotate45R ^= bit[g_nRotate45RMap[rsq]] ^ bit[g_nRotate45RMap[D8]];
						if (rsq!=nToSquare) b->nPieces[rsq] = SQUARE_EMPTY;
						b->nPieces[D8]  = BLACK_ROOK;
						b->nTransHash  ^= GET_HASH(rsq, BLACK_ROOK);
						b->nTransHash  ^= GET_HASH(D8,  BLACK_ROOK);
						b->nCastling   |= BLACK_HAS_CASTLED;
					}
				} else {
					if (nMoveType==MOVE_TYPE_CASTLE && nToSquare==G8) {
						// Castling King side - so move rook too
						b->bbPieces[BLACK][PIECE_ROOK-1] ^= MOVE_BLACK_ROOK_CASTLE_K;
						b->bbPieces[BLACK][PIECE_ALL-1] ^= MOVE_BLACK_ROOK_CASTLE_K;
						b->bbRotate90 ^= MOVE_BLACK_ROOK_CASTLE_K_90;
						b->bbRotate45L ^= MOVE_BLACK_ROOK_CASTLE_K_45L;
						b->bbRotate45R ^= MOVE_BLACK_ROOK_CASTLE_K_45R;
						b->nPieces[nToSquare-1] = SQUARE_EMPTY;
						b->nPieces[nToSquare+1] = BLACK_ROOK;
						b->nTransHash ^= GET_HASH(nToSquare-1, BLACK_ROOK);
						b->nTransHash ^= GET_HASH(nToSquare+1, BLACK_ROOK);
						b->nCastling |= BLACK_HAS_CASTLED;
					} else if (nMoveType==MOVE_TYPE_CASTLE && nToSquare==C8) {
						// Castling Queen side - so move rook too
						b->bbPieces[BLACK][PIECE_ROOK-1] ^= MOVE_BLACK_ROOK_CASTLE_Q;
						b->bbPieces[BLACK][PIECE_ALL-1] ^= MOVE_BLACK_ROOK_CASTLE_Q;
						b->bbRotate90 ^= MOVE_BLACK_ROOK_CASTLE_Q_90;
						b->bbRotate45L ^= MOVE_BLACK_ROOK_CASTLE_Q_45L;
						b->bbRotate45R ^= MOVE_BLACK_ROOK_CASTLE_Q_45R;
						b->nPieces[nToSquare+2] = SQUARE_EMPTY;
						b->nPieces[nToSquare-1] = BLACK_ROOK;
						b->nTransHash ^= GET_HASH(nToSquare-1, BLACK_ROOK);
						b->nTransHash ^= GET_HASH(nToSquare+2, BLACK_ROOK);
						b->nCastling |= BLACK_HAS_CASTLED;
					}
				}
				break;

		}

		// v1.05.09 - reduce branches
		if (nCapturedPiece > 0) {

			b->nMaterialHash ^= GET_HASH_COL(CountBits(b->bbPieces[WHITE][nCapturedPiece-1]), WHITE, nCapturedPiece);
			b->bbPieces[WHITE][PIECE_ALL-1] ^= bit[nToSquare];
			b->bbPieces[WHITE][nCapturedPiece-1] ^= bit[nToSquare];
			b->nMaterialHash ^= GET_HASH_COL(CountBits(b->bbPieces[WHITE][nCapturedPiece-1]), WHITE, nCapturedPiece);

			switch (nCapturedPiece) {
				case PIECE_PAWN:
					b->nPawnHash ^= GET_HASH_COL(nToSquare, WHITE, PIECE_PAWN);
					break;
				case PIECE_ROOK:
					b->nCastling &= ~b->g_bbCastles[nToSquare];
					break;
				#ifdef _DEBUG
				case PIECE_KING:
					// oh crap, that wasnt supposed to happen...
					PrintBoard(b,stdout);
					ASSERT(0);
					break;
				#endif
			}
		}

		// Whites turn to move now
		b->nSideToMove = WHITE;
	}

	if (nCapturedPiece != SQUARE_EMPTY) {
		// A piece was captured, so dont invert the target square as there is still a piece there
		b->bbRotate90  ^= bit[g_nRotate90Map[nFromSquare]];
		b->bbRotate45L ^= bit[g_nRotate45LMap[nFromSquare]];
		b->bbRotate45R ^= bit[g_nRotate45RMap[nFromSquare]];
		// Reset the repetition detector
		//g_nRepetitionListHead[nThreadID] = g_tGame.nGameHalfMoveNum+nCurrentDepth; // v0.99 - +depth
	} else if (nFromSquare != nToSquare) { // Can happen in Chess960 castling move
		b->bbRotate90  ^= (bit[g_nRotate90Map[nFromSquare]]  | bit[g_nRotate90Map[nToSquare]]);
		b->bbRotate45L ^= (bit[g_nRotate45LMap[nFromSquare]] | bit[g_nRotate45LMap[nToSquare]]);
		b->bbRotate45R ^= (bit[g_nRotate45RMap[nFromSquare]] | bit[g_nRotate45RMap[nToSquare]]);
	}

	// Check for changes to the castling status and update the hash values accordingly
	nCastleAfter = b->nCastling ^ nCastleBefore;
	if (nCastleAfter) {
		if (nCastleAfter & WHITE_KING_CASTLE) {
			b->nTransHash ^= g_nHashWhiteKingCastle;
		}
		if (nCastleAfter & WHITE_QUEEN_CASTLE) {
			b->nTransHash ^= g_nHashWhiteQueenCastle;
		}
		if (nCastleAfter & BLACK_KING_CASTLE) {
			b->nTransHash ^= g_nHashBlackKingCastle;
		}
		if (nCastleAfter & BLACK_QUEEN_CASTLE) {
			b->nTransHash ^= g_nHashBlackQueenCastle;
		}
	}

	#ifdef _DEBUG
	ValidateBoard(b);
	//TBoard by;
	//memcpy(&by,b,sizeof(TBoard));
	//UnMakeMove(&by,m,undo,nThreadID);
	//ASSERT(!memcmp(&bx,&by,sizeof(TBoard)));
	#endif


}

void UnMakeMove(TBoard *b, TMove m, TUndoMove *undo, int nThreadID, int nVariant) {
// Undo the move m on board b.

	//#ifdef _DEBUG
	//ValidateBoard(b);
	//#endif

	b->nSideToMove = OPP(b->nSideToMove);
	int nSideToMove = b->nSideToMove;
	int nFromSquare = GET_MOVE_FROM(m);
	int nToSquare = GET_MOVE_TO(m);
	int nMoveType = GET_MOVE_TYPE(m);
	int nPromPiece = GET_MOVE_PROM(m);
	int nMovedSquare = b->nPieces[nToSquare];						// Save piece+color
	int nMovedPiece = nMovedSquare & SQUARE_PIECE_MASK;				// Save piece without color
	BitBoard bbFromTo;		// Bits in the From and To squares for quick modifications to the bitboards
	//int nCastleBefore, nCastleAfter;

	ASSERT_MOVE(m);
	ASSERT(m);
	ASSERT(nMovedPiece);
	//ASSERT(IsValidMove(m, b));
	ASSERT(nFromSquare>=0);
	ASSERT(nFromSquare<64);
	ASSERT(nToSquare>=0);
	ASSERT(nToSquare<64);
	ASSERT(b->nSideToMove == (nMovedSquare & SQUARE_COLOR_MASK)>>4);

	b->nEPSquare = undo->nEPSquare;
	b->nCastling = undo->nCastleFlags;
	int nCapturedSquare = undo->nCapturedPiece;
	int nCapturedPiece = nCapturedSquare & SQUARE_PIECE_MASK;		// Save piece without color
	//if (g_tGame.bChess960 && nCapturedPiece==PIECE_KING) nCapturedPiece = 0;	// Castling move where King doesnt have to actually move (from==tq)
	if (nVariant == VARIANT_960) {
		// Castling move where King doesnt have to actually move (from==tq) or swaps squares with its Rook
		if (nCapturedPiece==PIECE_KING)	nCapturedPiece = 0;
		else if (nSideToMove==WHITE && nCapturedSquare==WHITE_ROOK) nCapturedPiece = 0;
		else if (nSideToMove==BLACK && nCapturedSquare==BLACK_ROOK) nCapturedPiece = 0;
	}
	b->nFiftyMoveCount = undo->n50Moves;
	b->nTransHash = undo->nTransHash;
	b->nPawnHash = undo->nPawnHash;
	b->nMaterialHash = undo->nMaterialHash;
	g_nRepetitionListHead[nThreadID] = undo->nRepListHead;
	b->nMaterial[WHITE] = undo->nMaterial[WHITE];
	b->nMaterial[BLACK] = undo->nMaterial[BLACK];
	//b->nInCheck = undo->nInCheck;


	if (nMoveType == MOVE_TYPE_PROM) {
		nMovedPiece = PIECE_PAWN;
	}

	//nCastleBefore = b->nCastling;
	if (nFromSquare != nToSquare) {
		bbFromTo = bit[nFromSquare] | bit[nToSquare]; // Contains two set bits
		b->nPieces[nFromSquare] = b->nPieces[nToSquare];
		b->nPieces[nToSquare]   = nCapturedSquare;
	} else {
		bbFromTo = 0;
	}

	// Change the bit boards
	if (nSideToMove == WHITE) {
		// Update all white specific data

		b->bbPieces[WHITE][PIECE_ALL-1] ^= bbFromTo;

		// v1.05.09 - condense logic and remove some branches
		b->bbPieces[WHITE][nMovedPiece-1] ^= bbFromTo;

		switch (nMovedPiece) {
			case PIECE_PAWN:
				// Check for an EP move
				if (nMoveType == MOVE_TYPE_EP) {
					// Remove the captured pawn
					b->bbPieces[BLACK][PIECE_PAWN-1] ^= bit[nToSquare-8];
					b->bbPieces[BLACK][PIECE_ALL-1] ^= bit[nToSquare-8];
					b->bbRotate90 ^= bit[g_nRotate90Map[nToSquare-8]];
					b->bbRotate45L ^= bit[g_nRotate45LMap[nToSquare-8]];
					b->bbRotate45R ^= bit[g_nRotate45RMap[nToSquare-8]];
					b->nPieces[nToSquare-8] = BLACK_PAWN;
				}
				// Check for a promotion
				if (nToSquare >= H8) {
					b->bbPieces[WHITE][PIECE_PAWN-1] ^= bit[nToSquare];	// Remove the promoted pawn
					b->bbPieces[WHITE][nPromPiece-1] ^= bit[nToSquare];
					b->nPieces[nFromSquare] = PIECE_WHITE | PIECE_PAWN;
				}
				break;

			case PIECE_KING:
				if (nVariant == VARIANT_960) {
					if (nMoveType==MOVE_TYPE_CASTLE && nToSquare==G1) {
						// Castling King side - so move rook too
						int rsq = undo->nOrigRookSq;
						b->bbPieces[WHITE][PIECE_ROOK-1] ^= bit[rsq]^bit[F1];
						b->bbPieces[WHITE][PIECE_ALL-1]  ^= bit[rsq]^bit[F1];
						b->bbRotate90  ^= bit[g_nRotate90Map[rsq]]  ^ bit[g_nRotate90Map[F1]];
						b->bbRotate45L ^= bit[g_nRotate45LMap[rsq]] ^ bit[g_nRotate45LMap[F1]];;
						b->bbRotate45R ^= bit[g_nRotate45RMap[rsq]] ^ bit[g_nRotate45RMap[F1]];;
						b->nPieces[rsq] = WHITE_ROOK;
						if (nFromSquare!=F1 && rsq!=F1) b->nPieces[F1]  = SQUARE_EMPTY;
					} else if (nMoveType==MOVE_TYPE_CASTLE && nToSquare==C1) {
						// Castling Queen side - so move rook too
						int rsq = undo->nOrigRookSq;
						b->bbPieces[WHITE][PIECE_ROOK-1] ^= bit[rsq]^bit[D1];
						b->bbPieces[WHITE][PIECE_ALL-1]  ^= bit[rsq]^bit[D1];
						b->bbRotate90  ^= bit[g_nRotate90Map[rsq]]  ^ bit[g_nRotate90Map[D1]];
						b->bbRotate45L ^= bit[g_nRotate45LMap[rsq]] ^ bit[g_nRotate45LMap[D1]];;
						b->bbRotate45R ^= bit[g_nRotate45RMap[rsq]] ^ bit[g_nRotate45RMap[D1]];;
						if (nFromSquare!=D1 && rsq!=D1) b->nPieces[D1]  = SQUARE_EMPTY;
						b->nPieces[rsq] = WHITE_ROOK;
					}
				} else {
					if (nMoveType==MOVE_TYPE_CASTLE && nToSquare==G1) {
						// Castling King side - so move rook too
						b->bbPieces[WHITE][PIECE_ROOK-1] ^= MOVE_WHITE_ROOK_CASTLE_K;
						b->bbPieces[WHITE][PIECE_ALL-1]  ^= MOVE_WHITE_ROOK_CASTLE_K;
						b->bbRotate90  ^= MOVE_WHITE_ROOK_CASTLE_K_90;
						b->bbRotate45L ^= MOVE_WHITE_ROOK_CASTLE_K_45L;
						b->bbRotate45R ^= MOVE_WHITE_ROOK_CASTLE_K_45R;
						b->nPieces[nToSquare-1] = WHITE_ROOK;
						b->nPieces[nToSquare+1] = SQUARE_EMPTY;
					} else if (nMoveType==MOVE_TYPE_CASTLE && nToSquare==C1) {
						// Castling Queen side - so move rook too
						b->bbPieces[WHITE][PIECE_ROOK-1] ^= MOVE_WHITE_ROOK_CASTLE_Q;
						b->bbPieces[WHITE][PIECE_ALL-1]  ^= MOVE_WHITE_ROOK_CASTLE_Q;
						b->bbRotate90  ^= MOVE_WHITE_ROOK_CASTLE_Q_90;
						b->bbRotate45L ^= MOVE_WHITE_ROOK_CASTLE_Q_45L;
						b->bbRotate45R ^= MOVE_WHITE_ROOK_CASTLE_Q_45R;
						b->nPieces[nToSquare-1] = SQUARE_EMPTY;
						b->nPieces[nToSquare+2] = WHITE_ROOK;
					}
				}
				break;

		}

		// v1.05.09 - reduce branches
		if (nCapturedPiece > 0) {
			b->bbPieces[BLACK][PIECE_ALL-1] ^= bit[nToSquare];
			b->bbPieces[BLACK][nCapturedPiece-1] ^= bit[nToSquare];
		}

	} else {
		// Update all black specific data

		b->bbPieces[BLACK][PIECE_ALL-1] ^= bbFromTo;

		// v1.05.09 - condense logic and remove some branches
		b->bbPieces[BLACK][nMovedPiece-1] ^= bbFromTo;
		
		switch (nMovedPiece) {
			case PIECE_PAWN:
				// Check for an EP move
				if (nMoveType == MOVE_TYPE_EP) {
					b->bbPieces[WHITE][PIECE_PAWN-1] ^= bit[nToSquare+8];
					b->bbPieces[WHITE][PIECE_ALL-1] ^= bit[nToSquare+8];
					b->bbRotate90 ^= bit[g_nRotate90Map[nToSquare+8]];
					b->bbRotate45L ^= bit[g_nRotate45LMap[nToSquare+8]];
					b->bbRotate45R ^= bit[g_nRotate45RMap[nToSquare+8]];
					b->nPieces[nToSquare+8] = WHITE_PAWN;
				}
				// Check for a promotion
				if (nToSquare <= A1) {
					b->bbPieces[BLACK][PIECE_PAWN-1] ^= bit[nToSquare]; // Remove the promoted pawn
					b->bbPieces[BLACK][nPromPiece-1] ^= bit[nToSquare];
					b->nPieces[nFromSquare] = PIECE_BLACK | PIECE_PAWN;
				}
				break;

			case PIECE_KING:
				if (nVariant == VARIANT_960) {
					if (nMoveType==MOVE_TYPE_CASTLE && nToSquare==G8) {
						// Castling King side - so move rook too
						int rsq = undo->nOrigRookSq;
						b->bbPieces[BLACK][PIECE_ROOK-1] ^= bit[rsq]^bit[F8];
						b->bbPieces[BLACK][PIECE_ALL-1]  ^= bit[rsq]^bit[F8];
						b->bbRotate90  ^= bit[g_nRotate90Map[rsq]]  ^ bit[g_nRotate90Map[F8]];
						b->bbRotate45L ^= bit[g_nRotate45LMap[rsq]] ^ bit[g_nRotate45LMap[F8]];;
						b->bbRotate45R ^= bit[g_nRotate45RMap[rsq]] ^ bit[g_nRotate45RMap[F8]];;
						if (nFromSquare!=F8 && rsq!=F8) b->nPieces[F8]  = SQUARE_EMPTY;
						b->nPieces[rsq] = BLACK_ROOK;
					} else if (nMoveType==MOVE_TYPE_CASTLE && nToSquare==C8) {
						// Castling Queen side - so move rook too
						int rsq = undo->nOrigRookSq;
						b->bbPieces[BLACK][PIECE_ROOK-1] ^= bit[rsq]^bit[D8];
						b->bbPieces[BLACK][PIECE_ALL-1]  ^= bit[rsq]^bit[D8];
						b->bbRotate90  ^= bit[g_nRotate90Map[rsq]]  ^ bit[g_nRotate90Map[D8]];
						b->bbRotate45L ^= bit[g_nRotate45LMap[rsq]] ^ bit[g_nRotate45LMap[D8]];;
						b->bbRotate45R ^= bit[g_nRotate45RMap[rsq]] ^ bit[g_nRotate45RMap[D8]];;
						if (nFromSquare!=D8 && rsq!=D8) b->nPieces[D8]  = SQUARE_EMPTY;
						b->nPieces[rsq] = BLACK_ROOK;
					}
				} else {
					if (nMoveType==MOVE_TYPE_CASTLE && nToSquare==G8) {
						// Castling King side - so move rook too
						b->bbPieces[BLACK][PIECE_ROOK-1] ^= MOVE_BLACK_ROOK_CASTLE_K;
						b->bbPieces[BLACK][PIECE_ALL-1] ^= MOVE_BLACK_ROOK_CASTLE_K;
						b->bbRotate90 ^= MOVE_BLACK_ROOK_CASTLE_K_90;
						b->bbRotate45L ^= MOVE_BLACK_ROOK_CASTLE_K_45L;
						b->bbRotate45R ^= MOVE_BLACK_ROOK_CASTLE_K_45R;
						b->nPieces[nToSquare-1] = BLACK_ROOK;
						b->nPieces[nToSquare+1] = SQUARE_EMPTY;
					} else if (nMoveType==MOVE_TYPE_CASTLE && nToSquare==C8) {
						// Castling Queen side - so move rook too
						b->bbPieces[BLACK][PIECE_ROOK-1] ^= MOVE_BLACK_ROOK_CASTLE_Q;
						b->bbPieces[BLACK][PIECE_ALL-1] ^= MOVE_BLACK_ROOK_CASTLE_Q;
						b->bbRotate90 ^= MOVE_BLACK_ROOK_CASTLE_Q_90;
						b->bbRotate45L ^= MOVE_BLACK_ROOK_CASTLE_Q_45L;
						b->bbRotate45R ^= MOVE_BLACK_ROOK_CASTLE_Q_45R;
						b->nPieces[nToSquare+2] = BLACK_ROOK;
						b->nPieces[nToSquare-1] = SQUARE_EMPTY;
					}
				}
				break;

		}

		// v1.05.09 - reduce branches
		if (nCapturedPiece > 0) {
			b->bbPieces[WHITE][PIECE_ALL-1] ^= bit[nToSquare];
			b->bbPieces[WHITE][nCapturedPiece-1] ^= bit[nToSquare];
		}

	}

	if (nCapturedPiece != SQUARE_EMPTY) {
		// A piece was captured, so dont invert the target square as there is still a piece there
		b->bbRotate90  ^= bit[g_nRotate90Map[nFromSquare]];
		b->bbRotate45L ^= bit[g_nRotate45LMap[nFromSquare]];
		b->bbRotate45R ^= bit[g_nRotate45RMap[nFromSquare]];
	} else if (nFromSquare != nToSquare) {
		b->bbRotate90  ^= (bit[g_nRotate90Map[nFromSquare]]  | bit[g_nRotate90Map[nToSquare]]);
		b->bbRotate45L ^= (bit[g_nRotate45LMap[nFromSquare]] | bit[g_nRotate45LMap[nToSquare]]);
		b->bbRotate45R ^= (bit[g_nRotate45RMap[nFromSquare]] | bit[g_nRotate45RMap[nToSquare]]);
	}

	#ifdef _DEBUG
	ValidateBoard(b);
	// unmake
	// assert b=bx
	#endif

}


void GameMoves2FEN(char *sMoves, char *sFEN) {
	// tokenise
	// if word is move, apply to board
	char *sWord;
	TBoard b1, *b;
	b=&b1; // just to use macros that assume b is a pointer
	CreateStartingPosition(b, VARIANT_STD);
	int stm = WHITE;
	//char x[10000];
	//strcpy(x,sMoves);

	// Nf3 e5 Nge7 bxc3 O-O Ngxe7 O-O-O a8=Q Rxa8 N2d4 Ra1+ axb8=Q+
	
	sWord = strtok(sMoves, " \n\r\t.");
	while (sWord) {
		if ((sWord[0] >= 'a' && sWord[0] <= 'z') || (sWord[0] >= 'A' && sWord[0] <= 'Z')) {
			// assume move
			TMove m;
			int file1=-1, rank1=-1, file2=-1, rank2=-1;
			int i=0, capture=0, check=0, prom=0;
			int piece=0, from=-1, to=-1, type=MOVE_TYPE_NORMAL;
			BitBoard mask, frpc=0;
			TUndoMove u;

			if      (sWord[i] == 'N') piece = PIECE_KNIGHT;
			else if (sWord[i] == 'B') piece = PIECE_BISHOP;
			else if (sWord[i] == 'R') piece = PIECE_ROOK;
			else if (sWord[i] == 'Q') piece = PIECE_QUEEN;
			else if (sWord[i] == 'K') piece = PIECE_KING;
			else if (sWord[i] == 'O') piece = PIECE_KING;
			else if (sWord[i] >= 'a' && sWord[i] <= 'h') { piece = PIECE_PAWN; i--; }
			else { ASSERT(false); }
			i++;

			if (sWord[i] == 'x') { capture=1; i++; } // Nxc3
			
			if (sWord[i] >= 'a' && sWord[i] <= 'h') { file1 = 'h'-sWord[i]; i++; }
			if (sWord[i] >= '1' && sWord[i] <= '8') { rank1 = sWord[i]-'1'; i++; }

			if (sWord[i] == 'x') { capture=1; i++; } // bxc3

			if      (sWord[i] == '+') { check=1; i++; }
			else if (sWord[i] == '=') { prom=1; i++; }

			if (sWord[i] >= 'a' && sWord[i] <= 'h') { file2 = 'h'-sWord[i]; i++; }
			if (sWord[i] >= '1' && sWord[i] <= '8') { rank2 = sWord[i]-'1'; i++; }

			if      (sWord[i] == '+') { check=1; i++; }
			else if (sWord[i] == '=') { prom=1; i++; }

			if      (sWord[i] == 'N') prom = PIECE_KNIGHT;
			else if (sWord[i] == 'B') prom = PIECE_BISHOP;
			else if (sWord[i] == 'R') prom = PIECE_ROOK;
			else if (sWord[i] == 'Q') prom = PIECE_QUEEN;

			if (file1>-1 && rank1>-1) {			// Ne7 e4
				to = rank1*8+file1;
				mask = 0xFFFFFFFFFFFFFFFF;
			} else if (file1>-1 && rank1==-1) {	// Nge7
				to = rank2*8+file2;
				mask = g_bbMaskFile[file1];
			} else if (file1==-1 && rank1>-1) {	// N2d4
				to = rank2*8+file2;
				mask = g_bbMaskRank[rank1];
			}

			if (sWord[0]=='O' && sWord[2]=='O') {
				type=MOVE_TYPE_CASTLE;
				from=GetBit(b->bbPieces[stm][PIECE_KING-1]);
				if (sWord[3]=='-' && sWord[4]=='O') to=(stm?C8:C1); else to=(stm?G8:G1);
			}

			if (piece == PIECE_PAWN)   {
				if (capture) {
					if (b->bbPieces[stm][piece-1]) 
						from = GetBit(g_bbPawnAttacks[OPP(stm)][to] & b->bbPieces[stm][piece-1] & mask);
					else // EP
						from = stm?to+8:to-8;
 				} else if (stm==WHITE && b->nPieces[to-8]) {
					from = to-8;
				} else if (stm==WHITE && b->nPieces[to-16]) {
					from = to-16;
				} else if (stm==BLACK && b->nPieces[to+8]) {
					from = to+8;
				} else if (stm==BLACK && b->nPieces[to+16]) {
					from = to+16;
				}
			} else if (piece == PIECE_KNIGHT) frpc = g_bbKnightAttacks[to]    & b->bbPieces[stm][piece-1] & mask;
			else if (piece == PIECE_BISHOP)   frpc = BISHOP_MOVES(to)         & b->bbPieces[stm][piece-1] & mask;
			else if (piece == PIECE_ROOK)     frpc = ROOK_MOVES(to)           & b->bbPieces[stm][piece-1] & mask;
			else if (piece == PIECE_QUEEN)    frpc = QUEEN_MOVES(to)          & b->bbPieces[stm][piece-1] & mask;
			else if (piece == PIECE_KING && type!=MOVE_TYPE_CASTLE) frpc = g_bbKingAttacks[to]      & b->bbPieces[stm][piece-1] & mask;

			if (prom>0) type=MOVE_TYPE_PROM;
			if (capture) capture = b->nPieces[to]&SQUARE_PIECE_MASK;

			if (CountBits(frpc)>1) {
				// multiple potential pieces, only one should be legal
				do {
					from = RemoveBit(frpc);
					SET_MOVE(m, from, to, type, prom, piece, capture);
					MakeMove2(b, m, &u, 0, 0, VARIANT_STD);
					if (IsSquareAttackedBy(b, GetBit(b->bbPieces[stm][PIECE_KING-1]), OPP(stm))) {
						UnMakeMove(b, m, &u, 1, VARIANT_STD);
						continue; // illegal
					} else 
						break;
				} while (frpc);
				ASSERT(from>-1);
			} else {
				if (frpc) from = GetBit(frpc);
				ASSERT(from>-1);
				SET_MOVE(m, from, to, type, prom, piece, capture);
				MakeMove2(b, m, &u, 0, 0, VARIANT_STD);
			}

			stm=OPP(stm);
			ASSERT(stm==b->nSideToMove);

		}
		sWord = strtok(NULL, " \n\r\t.");
	}

	Board2FEN(b, sFEN);
}


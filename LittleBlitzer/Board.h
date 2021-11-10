#pragma once

#include <stdio.h>
#include "Common.h"
#include "Types.h"
//#include "Thread.h"

/* The board is represented as a bitboard (one bit per square) with square indices as follows

rnbqkbnr	8	63 62 61 60 59 58 57 56		Black
pppppppp	7	55 54 53 52 51 50 49 48
........	6	47 46 45 44 43 42 41 40
........	5	39 38 37 36 35 34 33 32
........	4	31 30 29 28 27 26 25 24
........	3	23 22 21 20 19 18 17 16
PPPPPPPP	2	15 14 13 12 11 10 09 08
RNBQKBNR	1	07 06 05 04 03 02 01 00		White
				 A  B  C  D  E  F  G  H
*/

//#define WHITE	0
//#define BLACK	1

#define MAX_FEN_LEN			500

enum { WHITE, BLACK };

// Used to determine the opposite color
//#define OPP(c) ((c==WHITE) ? BLACK : WHITE)
#define OPP(c) ((c) ^ 1)				// v1.04.17

// A BitBoard contains one bit per square.
//typedef U64 BitBoard;

// Masks applied to Castling field to retrieve all valid castling types.
// 00000001 = Can castle White King side
// 00000010 = Can castle White Queen side
// 00000100 = Can castle Black King side
// 00001000 = Can castle Black Queen side
// 00010000 = White has castled
// 00100000 = Black has castled
#define NO_CASTLE				0x00
#define WHITE_KING_CASTLE		0x01
#define WHITE_QUEEN_CASTLE		0x02
#define BLACK_KING_CASTLE		0x04
#define BLACK_QUEEN_CASTLE		0x08
#define WHITE_CANT_CASTLE		0xFC
#define BLACK_CANT_CASTLE		0xF3
#define WHITE_CAN_CASTLE		0x03
#define BLACK_CAN_CASTLE		0x0C
#define WHITE_HAS_CASTLED		0x10
#define BLACK_HAS_CASTLED		0x20
#define ALL_CASTLE_BITS			0x3F
#define CAN_CASTLE				0x0F

#define NULL_SQUARE	-1 // Doesn't represent any square.

// This array holds one of the four castling masks in each of the four corners. It is used to 
// update castling rights. When a rook moves, castling on that side is immediately disallowed. 
// To determine which side to disallow (K or Q), use this array as follows.
// eg b->nCastling ^= g_bbCastles[nFromSquare]
//extern CACHE_ALIGN BitBoard g_bbCastles[64];


typedef CACHE_ALIGN struct {
	// 464 bytes
	// 320 bytes v1.04.11

	// v1.05.09 - condense to single array and lose the 4 x BQ+RQ bitboards
	BitBoard bbPieces[2][7];	// [COLOR][PIECE_TYPE-1]

	//BitBoard bbPieces[WHITE][PIECE_PAWN-1];		// BitBoards to store various piece positions
	//BitBoard bbPieces[WHITE][PIECE_KNIGHT-1];
	//BitBoard bbPieces[WHITE][PIECE_BISHOP-1];
	//BitBoard bbPieces[WHITE][PIECE_ROOK-1];
	//BitBoard bbPieces[WHITE][PIECE_QUEEN-1];
	//BitBoard bbPieces[WHITE][PIECE_KING-1];
	//BitBoard (b->bbPieces[WHITE][PIECE_BISHOP-1]|b->bbPieces[WHITE][PIECE_QUEEN-1]);
	//BitBoard bbPieces[WHITE][PIECE_ROOK-1]Queens;
	//BitBoard bbPieces[WHITE][PIECE_ALL-1];

	//BitBoard bbPieces[BLACK][PIECE_PAWN-1];
	//BitBoard bbPieces[BLACK][PIECE_KNIGHT-1];
	//BitBoard bbPieces[BLACK][PIECE_BISHOP-1];
	//BitBoard bbPieces[BLACK][PIECE_ROOK-1];
	//BitBoard bbPieces[BLACK][PIECE_QUEEN-1];
	//BitBoard bbPieces[BLACK][PIECE_KING-1];
	//BitBoard (b->bbPieces[BLACK][PIECE_BISHOP-1]|b->bbPieces[BLACK][PIECE_QUEEN-1]);
	//BitBoard (b->bbPieces[BLACK][PIECE_ROOK-1]|b->bbPieces[BLACK][PIECE_QUEEN-1]);
	//BitBoard bbPieces[BLACK][PIECE_ALL-1];

	BitBoard bbRotate90;	// All pieces rotated 90d clockwise
	BitBoard bbRotate45L;	// for sliding pieces (ie rooks, bishops, queens)
	BitBoard bbRotate45R;

	U8 nSideToMove;			// WHITE or BLACK
	S8 nEPSquare;			// The En Passent attack square, if any (0 to 63,-1)
	U8 nFiftyMoveCount;		// The number of moves towards the 50 move count. 
	U8 nCastling;			// Mask this with castling masks to determine castling rights.
							// Rights are removed once the pieces are moved for the first
							// time. This mask does not include checks like blocking pieces.
							// 00000001 = Can castle White King side
							// 00000010 = Can castle White Queen side
							// 00000100 = Can castle Black King side
							// 00001000 = Can castle Black Queen side
							// 00010000 = White has castled
							// 00100000 = Black has castled

	U64 nTransHash;			// The 64-bit TT hash value for the board (used to index the hash table)

	// unused by blitzer
	U8 nPieces[64];			// Stores one of the PIECE constants // v1.04.11 - change int to U8
	U64 nPawnHash;			// The 64-bit pawn hash value for the board (used to index the pawn hash table)
	U64 nMaterialHash;		// The 64-bit material hash value for the board (used to index the material hash table)
	int nMaterial[2];
	//int nWhiteMaterial;		// The total value of all White pieces
	//int nBlackMaterial;		// The total value of all Black pieces
	int nPieceSquare[2];	// v1.04.12 - removed unused field

	//U8 nInCheck;			// Indicates that the side to move's King is under attack (value = attacking piece)

	// Need this here for Chess960 multi threaded tournaments (diff threads have diff values)
	BitBoard g_bbCastles[64];			// Update castling rights

} TBoard;


// Constants
#define WHITE_HALF_BOARD		0x00000000FFFFFFFF
#define BLACK_HALF_BOARD		0xFFFFFFFF00000000

const BitBoard g_bbMaskNotFileA = 0x7F7F7F7F7F7F7F7F;	// All squares except far left file A
const BitBoard g_bbMaskNotFileH = 0xFEFEFEFEFEFEFEFE;	// All squares except far right file H
const BitBoard g_bbMaskNotRank8 = 0x00FFFFFFFFFFFFFF;	// All squares except back rank 8
const BitBoard g_bbMaskNotRank1 = 0xFFFFFFFFFFFFFF00;	// All squares except front rank 1
const BitBoard g_bbMaskRank8 =    0xFF00000000000000;	// Only squares in back rank 8
const BitBoard g_bbMaskRank7 =	  0x00FF000000000000;	// Only squares in rank 7
const BitBoard g_bbMaskRank2 =	  0x000000000000FF00;	// Only squares in rank 2
const BitBoard g_bbMaskRank1 =	  0x00000000000000FF;	// Only squares in front rank 1

// Bitboard of a specified file, e.g. g_bbMaskFile[3];
// . . . . X . . .
// . . . . X . . .
// . . . . X . . .
// . . . . X . . .
// . . . . X . . .
// . . . . X . . .
// . . . . X . . .
// . . . . X . . .
const BitBoard g_bbMaskFile[8] =  {0x0101010101010101, 0x0202020202020202, 0x0404040404040404, 0x0808080808080808,
								   0x1010101010101010, 0x2020202020202020, 0x4040404040404040, 0x8080808080808080};
const BitBoard g_bbMaskRank[8] =  {0x00000000000000FF, 0x000000000000FF00, 0x0000000000FF0000, 0x00000000FF000000,
								   0x000000FF00000000, 0x0000FF0000000000, 0x00FF000000000000, 0xFF00000000000000};

// Bitboard of files A and H
// X . . . . . . X
// X . . . . . . X
// X . . . . . . X
// X . . . . . . X
// X . . . . . . X
// X . . . . . . X
// X . . . . . . X
// X . . . . . . X
const BitBoard g_bbMaskFileAH =	0x8181818181818181;

// Bits are set either side of the specified file
extern BitBoard g_bbMaskIsoPawns[8];

// In a square, the piece type takes up 3 bits and the color the 4th/5th bits.
// To check a piece use (square & SQUARE_PIECE_MASK) == PIECE_PAWN to PIECE_KING
// To check a color use (square & SQUARE_COLOR_MASK) == PIECE_WHITE or PIECE_BLACK
#define SQUARE_PIECE_MASK		0x07
#define SQUARE_COLOR_MASK		0x18
#define SQUARE_EMPTY			0x00
//#define PIECE_NONE				0x00
//#define PIECE_PAWN				0x01
//#define PIECE_KNIGHT			0x02
//#define PIECE_BISHOP			0x03
//#define PIECE_ROOK				0x04
//#define PIECE_QUEEN				0x05
//#define PIECE_KING				0x06
#define PIECE_WHITE				0x08
#define PIECE_BLACK				0x10

enum {	PIECE_NONE,				// 0
		PIECE_PAWN,				// 1
		PIECE_KNIGHT,			// 2
		PIECE_BISHOP,			// 3
		PIECE_ROOK,				// 4
		PIECE_QUEEN,			// 5
		PIECE_KING,				// 6
		PIECE_ALL				// 7
};

// Some extra helper constants to speed up operations
#define WHITE_PAWN				(PIECE_WHITE | PIECE_PAWN)
#define WHITE_KNIGHT			(PIECE_WHITE | PIECE_KNIGHT)
#define WHITE_BISHOP			(PIECE_WHITE | PIECE_BISHOP)
#define WHITE_ROOK				(PIECE_WHITE | PIECE_ROOK)
#define WHITE_QUEEN				(PIECE_WHITE | PIECE_QUEEN)
#define WHITE_KING				(PIECE_WHITE | PIECE_KING)
#define BLACK_PAWN				(PIECE_BLACK | PIECE_PAWN)
#define BLACK_KNIGHT			(PIECE_BLACK | PIECE_KNIGHT)
#define BLACK_BISHOP			(PIECE_BLACK | PIECE_BISHOP)
#define BLACK_ROOK				(PIECE_BLACK | PIECE_ROOK)
#define BLACK_QUEEN				(PIECE_BLACK | PIECE_QUEEN)
#define BLACK_KING				(PIECE_BLACK | PIECE_KING)


// Square constants
enum {
	H1, G1, F1, E1, D1, C1, B1, A1,
	H2, G2, F2, E2, D2, C2, B2, A2,
	H3, G3, F3, E3, D3, C3, B3, A3,
	H4, G4, F4, E4, D4, C4, B4, A4,
	H5, G5, F5, E5, D5, C5, B5, A5,
	H6, G6, F6, E6, D6, C6, B6, A6,
	H7, G7, F7, E7, D7, C7, B7, A7,
	H8, G8, F8, E8, D8, C8, B8, A8
};

#define LIGHT_SQUARES	0xAA55AA55AA55AA55	// BitBoard of only light coloured squares
#define DARK_SQUARES	0x55AA55AA55AA55AA	// BitBoard of only dark coloured squares

#define LIGHT_WHITE_PROM_FILES	0xAA
#define DARK_WHITE_PROM_FILES	0x55
#define LIGHT_BLACK_PROM_FILES	0x55
#define DARK_BLACK_PROM_FILES	0xAA

// This array simply holds one bit in the indexed position.
// eg bit[0]  = 0x0000000000000001
//    bit[1]  = 0x0000000000000002
//    bit[62] = 0x4000000000000000
//    bit[63] = 0x8000000000000000
extern CACHE_ALIGN BitBoard bit[64];

// Arrays used to generate moves
extern CACHE_ALIGN BitBoard g_bbPawnAttacks[2][64];		// [COLOR][SQUARE] Stores the squares attacked by a pawn at the indexed square.
extern CACHE_ALIGN BitBoard g_bbWhitePawnAttacks[64];	// Stores the squares attacked by a white pawn at the indexed square.
extern CACHE_ALIGN BitBoard g_bbBlackPawnAttacks[64];	// Stores the squares attacked by a black pawn at the indexed square.
extern CACHE_ALIGN BitBoard g_nnWhiteEPAttacks[64];		// Stores the squares attacked by EnPassent by a white pawn at the indexed square.
extern CACHE_ALIGN BitBoard g_nnBlackEPAttacks[64];		// Stores the squares attacked by EnPassent by a black pawn at the indexed square.
extern CACHE_ALIGN BitBoard g_bbKnightAttacks[64];		// Stores the squares attacked by a knight at the indexed square.
extern CACHE_ALIGN BitBoard g_bbKingAttacks[64];		// Stores the squares attacked by a king at the indexed square.

// g_bbRankAttacks[sq][state] stores the rank attack BitBoard for a piece on sq where state
// is an 8-bit representation of the occupation of pieces on the square's rank.
// For Example:
// . r . . r . k .		Here the white queen is at square 10, and the		. . . . . . . .
// . p q b b p p p		state of that rank (2nd rank) is:					. . . . . . . .
// p . . p . n . .			(.PP..QPP) 01100111 = 103						. . . . . . . .
// . . . . p . . .															. . . . . . . .
// P . . . P P . .		So we check g_bbRankAttacks[10][103] which is:		. . . . . . . .
// . . N . B B . .			0x0000000000003A00 (shown on right)				. . . . . . . .
// . P P . . Q P P															. . X X X Q X .
// R . . . . R . K															. . . . . . . .
extern CACHE_ALIGN BitBoard g_bbRankAttacks[64][256];	// Attacked squares for each square and possible combination of pieces on files of that square's rank.

// g_bbFileAttacks[sq][state] stores the file attack BitBoard for a piece on sq where state
// is an 8-bit representation of the occupation of pieces on the square's file.
// State is calculated using the bbRotate90 BitBoard to get the file in consecutive bits.
//		State = bbRotate90 >> g_nR90RankShift[sq]
// For Example:
// . r . . r . k .		Here the white queen is at square 10, and the		. . . . . . . .
// . p q b b p p p		state of that file (6th file) is:					. . . . . . . .
// p . . p . n . .			(.pn.PBQR) 11110110 = 246						. . . . . . . .
// . . . . p . . .															. . . . . . . .
// P . . . P P . .		So we check g_bbFileAttacks[10][246] which is:		. . . . . . . .
// . . N . B B . .			0x0000000000040004 (shown on right)				. . . . . X . .
// . P P . . Q P P															. . . . . Q . .
// R . . . . R . K															. . . . . X . .
extern CACHE_ALIGN BitBoard g_bbFileAttacks[64][256];	// Attacked squares for each square and possible combination of pieces on ranks of that square's file.

// g_bbDiagA8H1Attacks[sq][state] stores the file attack BitBoard for a piece on sq where state
// is an 8-bit representation of the occupation of pieces on the square's diagonal.
// State is calculated using the bbRotate45L BitBoard to get the diagonal into consecutive bits.
//		State = (bbRotate45L >> g_nR45LShift[sq]) & g_nR45LMask[sq]
// For Example:
// Original				Rotate45L
// . r . . r . k .		K P B P . . p .		So the white queen is at square		. . . . . . . .
// . p q b b p p p		P . P p p q r R		10 which has a state of:			. . . . . . . .
// p . . p . n . .		. . . . b . . .			(QB.....) 110000 = 48			. . . . . . . .
// . . . . p . . .		. . n b . . P .											. . . . . . . .
// P . . . P P . .		. . p r . P . P		So g_bbDiagA8H1Attacks[10][48] is:	. . . . . . . .
// . . N . B B . .		. p . . . N . .			0x0000000000080002 -->			. . . . X . . .
// . P P . . Q P P		p k R . . . . p											. . . . . . . .
// R . . . . R . K		. . Q B . . . .											. . . . . . X .
extern CACHE_ALIGN BitBoard g_bbDiagA8H1Attacks[64][256]; // Attacked squares for each square and possible combination of pieces on A8H1 diagonals of that square.

// g_bbDiagH8A1Attacks[sq][state] stores the file attack BitBoard for a piece on sq where state
// is an 8-bit representation of the occupation of pieces on the square's diagonal.
// State is calculated using the bbRotate45R BitBoard to get the diagonal into consecutive bits.
//		State = (bbRotate45R >> g_nR45RShift[sq]) & g_nR4RLMask[sq]
// For Example:
// Original				Rotate45R
// . r . . r . k .		K k p . . . . .		So the white queen is at square		. . . . . . . .
// . p q b b p p p		P . . b p . . .		10 which has a state of:			. . . . . . . .
// p . . p . n . .		. P R r b . . P			(..Q.) 0010 = 2					. . . . . . . .
// . . . . p . . .		. . Q . . q . .											. . . . . . . .
// P . . . P P . .		. . B . . . p p		So g_bbDiagH8A1Attacks[10][2] is:	. . . . . . . X
// . . N . B B . .		. . P B . . r .			0x0000000001020008 -->			. . . . . . X .
// . P P . . Q P P		p . . P . P . .											. . . . . . . .
// R . . . . R . K		. p n p . N P R											. . . . X . . .
extern CACHE_ALIGN BitBoard g_bbDiagH8A1Attacks[64][256]; // Attacked squares for each square and possible combination of pieces on H8A1 diagonals of that square.
extern CACHE_ALIGN int g_nR45LMask[64];					// Required to mask the shifted, rotated boards as each diagonal is a different length
extern CACHE_ALIGN int g_nR45RMask[64];					// Required to mask the shifted, rotated boards as each diagonal is a different length

// Arrays used for SEE calculations
extern CACHE_ALIGN int g_nDirections[64][64];		// Gives the direction (as a single square offset) of [from][to].
extern CACHE_ALIGN BitBoard g_bbDir8Attacks[64];	// Ray of all bits from [sq] up (excluding sq)
extern CACHE_ALIGN BitBoard g_bbDir7Attacks[64];	// Ray of all bits from [sq] up-right (excluding sq)
extern CACHE_ALIGN BitBoard g_bbDirM1Attacks[64];	// Ray of all bits from [sq] right (excluding sq)
extern CACHE_ALIGN BitBoard g_bbDirM9Attacks[64];	// Ray of all bits from [sq] down-right (excluding sq)
extern CACHE_ALIGN BitBoard g_bbDirM8Attacks[64];	// Ray of all bits from [sq] down (excluding sq)
extern CACHE_ALIGN BitBoard g_bbDirM7Attacks[64];	// Ray of all bits from [sq] down-left (excluding sq)
extern CACHE_ALIGN BitBoard g_bbDir1Attacks[64];	// Ray of all bits from [sq] left (excluding sq)
extern CACHE_ALIGN BitBoard g_bbDir9Attacks[64];	// Ray of all bits from [sq] up-left (excluding sq)

extern CACHE_ALIGN BitBoard g_bbBetween[64][64];	// All bits between [from] and [to] are set (exclusive)

// g_bbPassedPawnMask[colour][sq] - All bits are set in neighbouring files in front of the specified square.
// Can be used to check for passed pawns or knight outposts.
// For Example: g_bbPassedPawnMask[WHITE][26]
// . . . . . . . .
// . . . . X . X .
// . . . . X . X .
// . . . . X . X .
// . . . . . s . .
// . . . . . . . .
// . . . . . . . .
// . . . . . . . .
extern CACHE_ALIGN BitBoard g_bbPassedPawnMask[2][64];

//#define FILE(sq) (sq%8)					// 0 to 7 file number (0 == FILE_H)
//#define RANK(sq) ((int)(sq/8))			// 0 to 7 rank number
#define FILE(sq) (sq&7)					// 0 to 7 file number (0 == FILE_H)		// v1.04.17
#define RANK(sq) ((int)(sq>>3))			// 0 to 7 rank number					// v1.04.17

// Mirror the square by rank (horizontally) e.g. G2 -> G7
#define FLIP_RANK(sq)	(7-RANK(sq))*8+FILE(sq)

// The color of the square (0-63) = WHITE or BLACK
#define SQ_COLOR(sq)	((RANK(sq)&1)^(FILE(sq)&1))

#define FILE_A	7
#define FILE_B	6
#define FILE_C	5
#define FILE_D	4
#define FILE_E	3
#define FILE_F	2
#define FILE_G	1
#define FILE_H	0

#define RANK_1	0
#define RANK_2	1
#define RANK_3	2
#define RANK_4	3
#define RANK_5	4
#define RANK_6	5
#define RANK_7	6
#define RANK_8	7

//// Macro to return a BitBoard of all the squares a rook at sq is attacking. Used for move generation.
//#define ROOK_MOVES(sq) ( \
//	g_bbRankAttacks[sq][((b->bbPieces[BLACK][PIECE_ALL-1] | b->bbPieces[WHITE][PIECE_ALL-1]) >> g_nRankShift[sq]) & 0xFF]	\
//	| g_bbFileAttacks[sq][(b->bbRotate90 >> g_nR90RankShift[sq]) & 0xFF]				\
//)
//
//// Macro to return a BitBoard of all the squares a bishop at sq is attacking. Used for move generation.
//#define BISHOP_MOVES(sq) ( \
//	g_bbDiagA8H1Attacks[sq][(b->bbRotate45L >> g_nR45LShift[sq]) & g_nR45LMask[sq]]		\
//	| g_bbDiagH8A1Attacks[sq][(b->bbRotate45R >> g_nR45RShift[sq]) & g_nR45RMask[sq]]	\
//)




// Used to build the bbRotate90 bit board (by turning the full board 90d clockwise)
// There are several sites which discuss various implementations of bitboards:
// http://supertech.lcs.mit.edu/~heinz/dt/node8.html
// http://atlanchess.com/html/rotated_bitboards.html
const CACHE_ALIGN unsigned char g_nRotate90Map[64] = {
	7,15,23,31,39,47,55,63,
	6,14,22,30,38,46,54,62,
	5,13,21,29,37,45,53,61,
	4,12,20,28,36,44,52,60,
	3,11,19,27,35,43,51,59,
	2,10,18,26,34,42,50,58,
	1, 9,17,25,33,41,49,57,
	0, 8,16,24,32,40,48,56 
};

// Rotate the board left by 45d so that the a8-h1 (\) diagonal is now horizontal. Then slice the diamond
// across the middle, move the smaller half to the top and then squash it back into a square shape. Now all of
// the \ diagonals are stored horizontally (which was the goal!).
const CACHE_ALIGN unsigned char g_nRotate45LMap[64] = {
	A8, B1, C2, D3, E4, F5, G6, H7,
	A7, B8, C1, D2, E3, F4, G5, H6,
	A6, B7, C8, D1, E2, F3, G4, H5,
	A5, B6, C7, D8, E1, F2, G3, H4,
	A4, B5, C6, D7, E8, F1, G2, H3,
	A3, B4, C5, D6, E7, F8, G1, H2,
	A2, B3, C4, D5, E6, F7, G8, H1,
	A1, B2, C3, D4, E5, F6, G7, H8
};

// Rotate the board left by 45d so that the h8-a1 (/) diagonal is now horizontal. Then slice the diamond
// across the middle, move the smaller half to the bottom and then squash it back into a square shape. Now all of
// the / diagonals are stored horizontally (which was the goal!).
const CACHE_ALIGN unsigned char g_nRotate45RMap[64] = {
	A8, B7, C6, D5, E4, F3, G2, H1,
	A7, B6, C5, D4, E3, F2, G1, H8,
	A6, B5, C4, D3, E2, F1, G8, H7,
	A5, B4, C3, D2, E1, F8, G7, H6,
	A4, B3, C2, D1, E8, F7, G6, H5,
	A3, B2, C1, D8, E7, F6, G5, H4,
	A2, B1, C8, D7, E6, F5, G4, H3,
	A1, B8, C7, D6, E5, F4, G3, H2
};

// The number of bits required to shift the entire rank of the full board containing the 
// indexed square to rank 1 (8*rank).
const CACHE_ALIGN unsigned char g_nRankShift[64] = {
	 0, 0, 0, 0, 0, 0, 0, 0,
	 8, 8, 8, 8, 8, 8, 8, 8,
	16,16,16,16,16,16,16,16,
	24,24,24,24,24,24,24,24,
	32,32,32,32,32,32,32,32,
	40,40,40,40,40,40,40,40,
	48,48,48,48,48,48,48,48,
	56,56,56,56,56,56,56,56
};

// The number of bits required to shift the entire file of the bbRotate90 board containing 
// the indexed square to file A (8*file).
const CACHE_ALIGN unsigned char g_nR90RankShift[64] = {
	 0, 8,16,24,32,40,48,56,
	 0, 8,16,24,32,40,48,56,
	 0, 8,16,24,32,40,48,56,
	 0, 8,16,24,32,40,48,56,
	 0, 8,16,24,32,40,48,56,
	 0, 8,16,24,32,40,48,56,
	 0, 8,16,24,32,40,48,56,
	 0, 8,16,24,32,40,48,56
};

// The number of bits required to shift the bbRotate45L board to get the \ diagonal of the 
// indexed square to the LSBs.
// v1.04.18
//const CACHE_ALIGN int g_nR45LShift[64] = {
const CACHE_ALIGN unsigned char g_nR45LShift[64] = {
	56, 0, 8,16,24,32,40,48,
	49,56, 0, 8,16,24,32,40,
	42,49,56, 0, 8,16,24,32,
	35,42,49,56, 0, 8,16,24,
	28,35,42,49,56, 0, 8,16,
	21,28,35,42,49,56, 0, 8,
	14,21,28,35,42,49,56, 0,
	 7,14,21,28,35,42,49,56
};

// The number of bits required to shift the bbRotate45R board to get the / diagonal of the
// indexed square to the LSBs.
const CACHE_ALIGN unsigned char g_nR45RShift[64] = {
	63,54,45,36,27,18, 9, 0,
	54,45,36,27,18, 9, 0,56,
	45,36,27,18, 9, 0,56,48,
	36,27,18, 9, 0,56,48,40,
	27,18, 9, 0,56,48,40,32,
	18, 9, 0,56,48,40,32,24,
	 9, 0,56,48,40,32,24,16,
	 0,56,48,40,32,24,16, 8
};

// The length of each diagonal is different so use this to easily calculate how many bits to look at
const unsigned char g_nLengthA8H1diagonals[64] = {
			8,7,6,5,4,3,2,1,
			7,8,7,6,5,4,3,2,
			6,7,8,7,6,5,4,3,
			5,6,7,8,7,6,5,4,
			4,5,6,7,8,7,6,5,
			3,4,5,6,7,8,7,6,
			2,3,4,5,6,7,8,7,
			1,2,3,4,5,6,7,8
};

// The length of each diagonal is different so use this to easily calculate how many bits to look at
const unsigned char g_nLengthH8A1diagonals[64] = { 
			1,2,3,4,5,6,7,8,
			2,3,4,5,6,7,8,7,
			3,4,5,6,7,8,7,6,
			4,5,6,7,8,7,6,5,
			5,6,7,8,7,6,5,4,
			6,7,8,7,6,5,4,3,
			7,8,7,6,5,4,3,2,
			8,7,6,5,4,3,2,1
};

// Used to determine if a pushed pawn is about to promote
const unsigned char g_nPushedPawnExt[64] = { 
			0,0,0,0,0,0,0,0,
			1,1,1,1,1,1,1,1,
			0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,
			1,1,1,1,1,1,1,1,
			0,0,0,0,0,0,0,0
};

// The character for each piece
//const char g_nPieceChars[23] = {'.',' ',' ',' ',' ',' ',' ',' ',' ','P','N','B','R','Q','K',' ',' ','p','n','b','r','q','k'};
const char g_nPieceChars[23] = {' ',' ',' ',' ',' ',' ',' ',' ',' ','P','N','B','R','Q','K',' ',' ','p','n','b','r','q','k'};

// This array holds the scores of each piece indexed by the PIECE_* constants.
// eg g_nPieceValues[PIECE_ROOK | PIECE_WHITE] == SCORE_ROOK == 500
// eg g_nPieceValues[PIECE_ROOK | PIECE_BLACK] == -SCORE_ROOK == -500
extern CACHE_ALIGN int g_nPieceValues[23];

// The distance between 2 squares
extern CACHE_ALIGN int g_nDistance[64][64];

// An array that stores the 64-bit hash value for each board played.
// The first part of the array stores the fixed positions that have been played in the game.
// The rest of the array is a FILO stack which stores the positions as they are encountered
// in the search tree. If three repetitions of the same position are found with the same side
// to move, the position is a draw.
extern U64 g_nRepetitionHashes[600][MAX_THREADS];
extern int g_nRepetitionListHead[MAX_THREADS];
extern int g_nGameHalfMoveNum[MAX_THREADS];

#define SCORE_PAWN				100
#define SCORE_KNIGHT			325 // v1.01 from 300
#define SCORE_BISHOP			325
#define SCORE_ROOK				500
#define SCORE_QUEEN				975 // v1.01 from 900
#define SCORE_KING				0
#define MAX_PIECE_SCORE			8*SCORE_PAWN + 2*SCORE_KNIGHT + 2*SCORE_BISHOP + 2*SCORE_ROOK + SCORE_QUEEN


// Define this to cache move generator results for sliding pieces
//#define CACHE_MOVES // this does not work with threads!!!

// TODO: have sep hash per thread... or just use last file/rank per thread like crafty does

#ifdef CACHE_MOVES

	typedef struct {
		U64 nRookHash;
		U64 nBishopHash;
		BitBoard bbRookMoves;
		BitBoard bbBishopMoves;
	} TMovesCache;
	extern TMovesCache g_tMovesCache[64];

	#define ROOK_MOVES(sq)		( ROOK_MOVES_FN(b,sq) )
	#define BISHOP_MOVES(sq)	( BISHOP_MOVES_FN(b,sq) )
	#define QUEEN_MOVES(sq)		(ROOK_MOVES(sq) | BISHOP_MOVES(sq))

	// Cache the move generators.
	// About 1-2% improvement with few sliders. Up to 10% improvement with lots of sliders on board.
	__forceinline U64 ROOK_MOVES_FN(TBoard *b, int sq) {
		BitBoard bb;
		//if (g_tThreads.nNumThreads == 1) {
			if (g_tMovesCache[sq].nRookHash == b->nTransHash) {
				bb = g_tMovesCache[sq].bbRookMoves;
			} else {
				g_tMovesCache[sq].nRookHash = b->nTransHash;
				bb = g_bbRankAttacks[sq][((b->bbPieces[BLACK][PIECE_ALL-1] | b->bbPieces[WHITE][PIECE_ALL-1]) >> g_nRankShift[sq]) & 0xFF] | g_bbFileAttacks[sq][(b->bbRotate90 >> g_nR90RankShift[sq]) & 0xFF];
				g_tMovesCache[sq].bbRookMoves = bb;
			}
		//} else {
		//	bb = g_bbRankAttacks[sq][((b->bbPieces[BLACK][PIECE_ALL-1] | b->bbPieces[WHITE][PIECE_ALL-1]) >> g_nRankShift[sq]) & 0xFF] | g_bbFileAttacks[sq][(b->bbRotate90 >> g_nR90RankShift[sq]) & 0xFF];
		//}

		return bb;
	}

	__forceinline U64 BISHOP_MOVES_FN(TBoard *b, int sq) {
		BitBoard bb;
		//if (g_tThreads.nNumThreads == 1) {
			if (g_tMovesCache[sq].nBishopHash == b->nTransHash) {
				bb = g_tMovesCache[sq].bbBishopMoves;
			} else {
				g_tMovesCache[sq].nBishopHash = b->nTransHash;
				bb = g_bbDiagA8H1Attacks[sq][(b->bbRotate45L >> g_nR45LShift[sq]) & g_nR45LMask[sq]] | g_bbDiagH8A1Attacks[sq][(b->bbRotate45R >> g_nR45RShift[sq]) & g_nR45RMask[sq]];
				g_tMovesCache[sq].bbBishopMoves = bb;
			}
		//} else {
		//	bb = g_bbDiagA8H1Attacks[sq][(b->bbRotate45L >> g_nR45LShift[sq]) & g_nR45LMask[sq]] | g_bbDiagH8A1Attacks[sq][(b->bbRotate45R >> g_nR45RShift[sq]) & g_nR45RMask[sq]];
		//}

		return bb;
	}

#else

	// Macro to return a BitBoard of all the squares a rook at sq is attacking. Used for move generation.
	#define ROOK_MOVES(sq) ( \
		g_bbRankAttacks[sq][((b->bbPieces[BLACK][PIECE_ALL-1] | b->bbPieces[WHITE][PIECE_ALL-1]) >> g_nRankShift[sq]) & 0xFF]	\
		| g_bbFileAttacks[sq][(b->bbRotate90 >> g_nR90RankShift[sq]) & 0xFF]				\
	)

	// Macro to return a BitBoard of all the squares a bishop at sq is attacking. Used for move generation.
	#define BISHOP_MOVES(sq) ( \
		g_bbDiagA8H1Attacks[sq][(b->bbRotate45L >> g_nR45LShift[sq]) & g_nR45LMask[sq]]		\
		| g_bbDiagH8A1Attacks[sq][(b->bbRotate45R >> g_nR45RShift[sq]) & g_nR45RMask[sq]]	\
	)
	#define QUEEN_MOVES(sq)		(ROOK_MOVES(sq) | BISHOP_MOVES(sq))


#endif

void CreateStartingPosition(TBoard *b, int nVariant);	// Sets up the board to the starting game position
void PrintBoard(TBoard *b, FILE *f);
void PrintBoardNoEval(TBoard *b, FILE *f);
void ZeroBoard(TBoard *b);
//void LoadFENFile(TBoard *b, TGame *g, char *sFileName);
//void LoadFEN(TBoard *b, TGame *g, const char sFEN[]);
int IsSquareAttackedBy(TBoard *b, int sq, int color);
int IsSTMInCheck(TBoard *b);
int CountSquareAttackedBy(TBoard *b, int sq, int color, int *nNumAttacks);
BitBoard SquareAttacks(TBoard *b, int sq);
void ValidateBoard(TBoard *b);
void CalculateHashValues(TBoard *b);
void FlipBoard(TBoard *b);
void FlopBoard(TBoard *b);
void Board2FEN(TBoard *b, char *sFEN);
void SetBitBoards(TBoard *b);
bool IsInsufficientMaterial(TBoard *b);
bool IsRepetition(TBoard *b, int nThreadID);
void LoadFEN(TBoard *b, const char a_sFEN[]);
void InitialiseArrays();

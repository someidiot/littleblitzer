#pragma once

#include "Board.h"

// Maximum number of moves at a position
#define MAX_MOVES	218

// A Move type consists of 23 bits of information, some mandatory, others for convenience/performance
// #Bits   Starting   Field
// 6       0          To square (0-63)
// 6       6          From square (0-63)
// 2       12         Type of move (Normal, Castle, Promotion, EP)
// 3       14         Promotion type (if type==promotion, N,B,R,Q)
// 3       17         Moved piece (1-6) without color
// 3       20         Captured piece (0-6) (for EP, == PIECE_PAWN) without color
// 9       23         UNUSED
typedef unsigned __int32 TMove;

#define GET_MOVE_FROM(m)		(int)(((m)&0x0FC0)>>6)
#define GET_MOVE_TO(m)			(int)((m)&0x003F)
#define GET_MOVE_TYPE(m)		(int)(((m)&0x3000)>>12)
#define GET_MOVE_PROM(m)		(int)(((m)&0x1C000)>>14)
#define GET_MOVE_MOVED(m)		(int)(((m)&0xE0000)>>17)
#define GET_MOVE_CAPTURED(m)	(int)(((m)&0x700000)>>20)
//#define GET_MOVE_SCORE(m)		(__int32)(((m)&0xFFFFFFFF00000000)>>32)

#define MOVE_NULL		0

// Set all bit fields in a 32-bit TMove value

#define SET_MOVE(m,fr,to,ty,pr,mo,ca)		\
	(m) = (TMove)to							\
		| (((TMove)fr)<<6)					\
		| (((TMove)ty)<<12)					\
		| (((TMove)pr)<<14)					\
		| (((TMove)mo)<<17)					\
		| (((TMove)ca)<<20)

#define MOVE_TYPE_NORMAL	0
#define MOVE_TYPE_CASTLE	1
#define MOVE_TYPE_PROM		2
#define MOVE_TYPE_EP		3

#define ASSERT_RANGE(v,a,b) ASSERT((v)>=(a) && (v)<=(b))

#define ASSERT_MOVE(m)								\
		ASSERT_RANGE(GET_MOVE_MOVED(m),    1, 6);	\
		ASSERT_RANGE(GET_MOVE_FROM(m),     0, 63);	\
		ASSERT_RANGE(GET_MOVE_TO(m),       0, 63);	\
		ASSERT_RANGE(GET_MOVE_TYPE(m),     0, 3);	\
		ASSERT_RANGE(GET_MOVE_CAPTURED(m), 0, 6);

// A Move List consists of a list of moves for a given board, and the fields required to generate
// the list dynamically with pointers to appropriate entries in the list.
typedef struct {
	TMove tMoves[MAX_MOVES];
	int nScores[MAX_MOVES];
	TMove tMovesSearched[MAX_MOVES]; // Moves that are searched are copied here for history scoring
	TMove tHashMove;
	TMove tTriedKiller1;	// Store the killer move attempted from this movelist. Other threads may change it so need to remember what we already searched.
	TMove tTriedKiller2;	// Store the killer move attempted from this movelist. Other threads may change it so need to remember what we already searched.
	int nNumMoves;
	int nNumGoodMoves;	// Number of good/even capture moves
	int nNumBadMoves;	// Number of bad capture moves
	TMove *pCurrentMove;
	//TMove *pEndMove;	// The last move in the current move generator
	int *pCurrentScore;
	int nStateOfGeneration;
	int nNumUsedMoves;		// The number of generated moves that have been searched (or illegal, or pruned)
} TMoveList;



// Used to move the rook in a castling move
#define MOVE_WHITE_ROOK_CASTLE_K		0x0000000000000005
#define MOVE_WHITE_ROOK_CASTLE_Q		0x0000000000000090
#define MOVE_BLACK_ROOK_CASTLE_K		0x0500000000000000
#define MOVE_BLACK_ROOK_CASTLE_Q		0x9000000000000000

#define MOVE_WHITE_ROOK_CASTLE_K_90		0x0000000000800080
#define MOVE_WHITE_ROOK_CASTLE_Q_90		0x8000008000000000
#define MOVE_BLACK_ROOK_CASTLE_K_90		0x0000000000010001
#define MOVE_BLACK_ROOK_CASTLE_Q_90		0x0100000100000000

#define MOVE_WHITE_ROOK_CASTLE_K_45L	0x8000000000002000	// bits 13 and 63
#define MOVE_WHITE_ROOK_CASTLE_Q_45L	0x0001000008000000	// bits 48 and 27
#define MOVE_BLACK_ROOK_CASTLE_K_45L	0x0000000000200080	// bits 7 and 21
#define MOVE_BLACK_ROOK_CASTLE_Q_45L	0x0100000800000000	// bits 35 and 56

#define MOVE_WHITE_ROOK_CASTLE_K_45R	0x8000200000000000	// bits 63 and 45
#define MOVE_WHITE_ROOK_CASTLE_Q_45R	0x0000000008000001	// bits 0 and 27
#define MOVE_BLACK_ROOK_CASTLE_K_45R	0x0020000000000080	// bits 7 and 53
#define MOVE_BLACK_ROOK_CASTLE_Q_45R	0x0000000800000100	// bits 8 and 35

// Move Scores:
#define MOVE_SCORE_WORST			-32000	// Lowest score possible (used for sorting to end of move lists)
#define MOVE_SCORE_CAPTURE_PENALTY	-10000
#define MOVE_SCORE_NONE				0
#define MOVE_SCORE_HASH				1		// must be > 0 to allow potential extensions
#define MOVE_SCORE_CAPTURE_BONUS	10000
#define MOVE_SCORE_PREV_BEST		10000
//#define MOVE_HISTORY_SCORE_MAX		32000
#define MOVE_SCORE_KILLER_2			MOVE_HISTORY_SCORE_MAX + 10
#define MOVE_SCORE_KILLER_1			MOVE_HISTORY_SCORE_MAX + 20

// Constants for Move Generator State
#define MG_HASH					1
#define MG_NO_HASH				2
#define MG_GENERATE_EVASIONS	3
#define MG_GENERATE_CAPTURES	4
#define MG_CAPTURES				5
#define MG_KILLERS_1			6
#define MG_KILLERS_2			7
#define MG_GENERATE_NONCAPS		8
#define MG_NONCAPS				9
#define MG_GET_LOSING_CAPTURES	10
#define MG_NEXT_LOSING_CAPTURE	11
#define MG_NEXT_MOVE			12
#define MG_DONE					13
#define MG_GEN_ROOT_MOVES		14

// Information required to undo a move
typedef struct {
	S8  nEPSquare;
	U8  nCastleFlags;
	int nCapturedPiece;
	U8  n50Moves;
	U64 nTransHash;
	U64 nPawnHash;
	U64 nMaterialHash;
	int nRepListHead;
	int nMaterial[2];
	U8  nInCheck;
	// Chess960 specific
	int nOrigRookSq;	// Original Rook square before a castling move was made
} TUndoMove;


//void MakeMove(TBoard *b, TBoard *b2, TMove m, int nCurrentDepth, int nThreadID);
void MakeMove2(TBoard *b, TMove m, TUndoMove *undo, int nCurrentDepth, int nThreadID, int nVariant);
bool IsValidMoveQuick(TMove m, TBoard *b, int nVariant);
bool Move2Coord(TMove *m, TBoard *b, char *sMove, int nVariant);
void UnMakeMove(TBoard *b, TMove m, TUndoMove *undo, int nThreadID, int nVariant);
char *GetNotation(TBoard *b, TMove m);
void GameMoves2FEN(char *sMoves, char *sFEN);

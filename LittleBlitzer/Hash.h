#pragma once

#include <math.h>
#include "Move.h"
//#include "LittleThought.h"
//#include "Stats.h"
//#include "Types.h"

// Masks to apply to nFields of TTranspositionHash struct
#define HASH_FIELDS_DEPTH			0x00000000000000FF
#define HASH_FIELDS_TIMESTAMP		0x0000000000000700
#define HASH_FIELDS_TYPE			0x0000000000001800
#define HASH_FIELDS_SCORE			0x000000001FFFE000
#define HASH_FIELDS_MOVE			0x000FFFFFE0000000
#define HASH_FIELDS_THREAT			0x0010000000000000
#define HASH_FIELDS_SINGLE			0x0020000000000000

// Number of bits to shift the masked nFields property
#define HASH_SHIFT_DEPTH		0
#define HASH_SHIFT_TIMESTAMP	8
#define HASH_SHIFT_TYPE			11
#define HASH_SHIFT_SCORE		13
#define HASH_SHIFT_MOVE			29
#define HASH_SHIFT_THREAT		52
#define HASH_SHIFT_SINGLE		53

// Masks for the type field
#define HASH_ENTRY_EXACT		1
#define HASH_ENTRY_ALPHA_CUT	2
#define HASH_ENTRY_BETA_CUT		3

// 12 bytes per entry, 5 entries fits within a single 64-byte cache line
#define NUM_TRANS_ENTRIES		4	// Number of hash entries per trans table entry. Use multiple because they fit in the 
									// same cache line and are "free" lookups.

// Structure for the Hash Table used as a Transposition Table. We want to keep
// this pretty small as there will be millions of entries.
typedef struct {
	// TODO: should this be U64?
	U32 nKey[NUM_TRANS_ENTRIES];			
	// The lower 20-something bits of the boards hash value is used to index the table.
	// The higher 32-bits of the boards hash value are used to verify this position is the correct one.
	// Which makes the possibility of collisions rare enough to not have to worry about.
	U64 nFields[NUM_TRANS_ENTRIES];
	// #Bits	Starting	Field Name		Field Description
	// 8		0			depth			The depth this entry has been searched
	// 3		8			timestamp		Used between moves to indicate an old position in the table (0=new,1=old)
	// 2		11			type			1 = exact score, 2 = fail low bound (true value is at most score), 3 = fail high bound (true value is at least score)
	// 16		13			score			The final score of the searched position (subject to the type field)
	// 23       29			move			The best searched move
	// 1        52			mate threat		Indicates if there is a mate threat at this position (used to extend, avoid null move)
	// 1		53			single reply	Indicates that there is only a single legal move in this position (used to extend)
	// 10       55			UNUSED
} TTranspositionHash;

// The Transposition Table is a hash table storing recently examined board positions and their
// values to reduce the required searching performed. The larger this table, the more useful
// it will be.
// There are two separate Transposition Tables. 
// - g_aTransTable uses a replacement scheme where positions that have been searched deeper have priority
// - g_aTransReplace uses an always replace scheme regardless of what positions are being overridden. However, 
//   this table is only used when probes/writes to the first table fail.
extern TTranspositionHash *g_aTransTable;
extern U64 g_nNumTransEntries;	// Number of entries in the depth-priority transposition hash table
//extern TTranspositionHash *g_aTransReplace;
//extern U64 g_nNumTTEntriesReplace;	// Number of entries in the always-replace transposition hash table

//#define HASH_SPLIT	50				// Give this % of the requested memory to the 1st trans table and the rest to the 2nd

// TODO: dont shift if in 64-bit. use 64-bit ints everywhere... same for moves

// Macros to help retrieve the hash entry fields
#define GET_HASH_DEPTH(f)			(int)(((f)&HASH_FIELDS_DEPTH)>>HASH_SHIFT_DEPTH)
#define GET_HASH_TIMESTAMP(f)		(int)(((f)&HASH_FIELDS_TIMESTAMP)>>HASH_SHIFT_TIMESTAMP)
#define GET_HASH_TYPE(f)			(int)(((f)&HASH_FIELDS_TYPE)>>HASH_SHIFT_TYPE)
#define GET_HASH_SCORE(f)			(__int32)((((f)&HASH_FIELDS_SCORE)>>HASH_SHIFT_SCORE)-INFINITY)
#define GET_HASH_MOVE(f)			(__int32)(((f)&HASH_FIELDS_MOVE)>>HASH_SHIFT_MOVE)
#define GET_HASH_THREAT(f)			(__int32)(((f)&HASH_FIELDS_THREAT)>>HASH_SHIFT_THREAT)
#define GET_HASH_SINGLE(f)			(__int32)(((f)&HASH_FIELDS_SINGLE)>>HASH_SHIFT_SINGLE)

// Macros to help set the hash entry fields. These assume nFields has been initialised to 
// zero beforehand, and that v only has the appropriate number of bits set.
#define SET_HASH_TYPE(f,v)			((f) |= (((U64)v)<<HASH_SHIFT_TYPE))
#define SET_HASH_DEPTH(f,v)			((f) |= ((U64)v))
#define SET_HASH_SCORE(f,v)			((f) |= (((U64)v+INFINITY)<<HASH_SHIFT_SCORE))
#define SET_HASH_TIMESTAMP(f,v)		((f) |= (((U64)v)<<HASH_SHIFT_TIMESTAMP))
#define SET_HASH_MOVE(f,v)			((f) |= (((U64)v)<<HASH_SHIFT_MOVE))
#define SET_HASH_THREAT(f,v)		((f) |= (((U64)v)<<HASH_SHIFT_THREAT))
#define SET_HASH_SINGLE(f,v)		((f) |= (((U64)v)<<HASH_SHIFT_SINGLE))

// Macro to clear the timestamp, same as SET_HASH_TIMESTAMP(f,0) (mark entry as new)
//#define CLEAR_HASH_TIMESTAMP(f)		((f) &= (~HASH_FIELDS_TIMESTAMP))

//#define GET_HASH(sq, pc)			(g_nHashValues[sq][pc])
#define GET_HASH(sq, pc)			(g_nHashValues[sq][((pc)&SQUARE_COLOR_MASK)>>4][(pc)&SQUARE_PIECE_MASK]) // v1.04.13 - reduce to 7k
#define GET_HASH_COL(sq, col, pc)	(g_nHashValues[sq][col][pc])

// The random number table used to calculate the index into the hash table.
// This is equivalent to the lower 32-bits of the 64-bit hash value.
// g_nHashValues[square num][piece num (inc color)]
//extern U64 g_nHashValues[64][23];
extern U64 g_nHashValues[64][2][7]; // v1.04.13 - change to [sq][color][pc] to reduce array size

// v1.04.13 - reduce array size
// Overlap hash values to conserve space. Credit to H.G.Muller for the idea in Micro-Max.
// g_nHashValues[] = 0x242376239452135234523
// idx 0           =   xxxxxxxxxxxxxxxx
// idx 1           =     xxxxxxxxxxxxxxxx
// idx 2           =       xxxxxxxxxxxxxxxx
// Max index is 63 + 1<<6 + 6<<7 = 895 + leave 7 bytes for last indexed U64 value = 902 entries.
//extern U8 g_nHashValues[902]; 

// Other random numbers used to generate a hash number for a board.
// v1.04.12 - replaced U64 with U8 to improve cache performance
extern unsigned char g_nHashWhiteKingCastle;
extern unsigned char g_nHashWhiteQueenCastle;
extern unsigned char g_nHashBlackKingCastle;
extern unsigned char g_nHashBlackQueenCastle;
extern unsigned char g_nHashEPMove[64];
extern unsigned char g_nHash50Move[100];
extern unsigned char g_nHashWhiteToMove;

// Structure for the Hash Table used as a Pawn Table
// TODO: only use 32 bit for key and store double pawns to help with pawn bonus
typedef struct {
	U64 nKey;			// The hash value used to index the hash table and verify this position
	S16 nScore;			// The value of this entry
	
	U8 nPawnCountW;		// Number of white pawns
	U8 nPawnCountB;		// Number of black pawns
	U8 nPassedW;		// 1-bit per file indicating if a passed pawn exists
	U8 nPassedB;		// 1-bit per file indicating if a passed pawn exists
	U8 nIsolated[2];	// 1-bit per file indicating if an isolated pawn exists
	U8 nDoubled[2];		// 1-bit per file indicating if a doubled pawn exists
	
	U8 nFilesUsed[2];	// 1-bit per file indicating if one or more pawn exists
	
	// can fit another 12 bytes in spare space

} TPawnHash;

// The Pawn Hash Table stores information about a board's pawn structure. It is used to
// cache the information for speedier access in the evaluation routines. This table is no
// where near as big as the Transposition Table, as the pawn structure changes far less
// frequently and so does not need as many entries. Even using one entry will give a high
// hit ratio.
extern TPawnHash *g_aPawnTable;

// The number of entries in the Pawn Hash Table.
extern U64 g_nNumPawnEntries;

// Structure for the Hash Table used as a Eval Cache Table
typedef struct {
	U64 nKey;		// The hash value used to index the hash table and verify this position
	S16 nScore;		// The value of this entry
} TEvalHash;

extern TEvalHash *g_aEvalTable;
extern U64 g_nNumEvalEntries;

void InitRand();
unsigned long Get32BitRandomNum(bool bReset = false);
U64 Get64BitRandomNum();
U64 GetRandomNum();

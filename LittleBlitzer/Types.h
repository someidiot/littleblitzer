#pragma once

// TODO: support other compilers.... one day... maybe
typedef char S8;				// Signed   8 bit
typedef unsigned char U8;		// Unsigned 8 bit
typedef __int16 S16;			// Signed   16 bit
typedef unsigned __int16 U16;	// Unsigned 16 bit (0 to 65536)
typedef unsigned long U32;		// Unsigned 32 bit
typedef unsigned __int64 U64;	// Unsigned 64 bit


// Data type ranges used for debugging
#define MAX_S32		2147483647 // eg long
#define MIN_S16		-32768
#define MAX_S16		32767
#define MIN_U16		0
#define MAX_U16		65536
#define MIN_S8		-128
#define MAX_S8		127

#define CACHE_LINE  64
#define CACHE_ALIGN __declspec(align(CACHE_LINE))

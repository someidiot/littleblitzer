#include "StdAfx.h"
#include "Common.h"

FILE *fLog = NULL;
char sLogPath[1024];

bool g_bLogging = false;
bool g_bDumpIllegalMoves = true;
bool g_bFullPGN = false;

char *GetFilePath(char *sFilePath) {
// Give path/file return path
	char *pLast = strrchr(sFilePath, '\\');
	char *sPath = 0;
	if (pLast) {
		sPath = new char[pLast - sFilePath + 1];
		strncpy(sPath, sFilePath, pLast-sFilePath);
		sPath[pLast-sFilePath] = 0;
	}
	return sPath;
}

wchar_t *GetFilePath(wchar_t *sFilePath) {
// Give path/file return path
	wchar_t *pLast = wcsrchr(sFilePath, '\\');
	wchar_t *sPath = 0;
	if (pLast) {
		sPath = new wchar_t[pLast - sFilePath + 1];
		wcsncpy(sPath, sFilePath, pLast-sFilePath);
		sPath[pLast-sFilePath] = 0;
	}
	return sPath;
}

void ResetLog() {
	//static FILE *fLog = NULL;
	//static char sLogPath[1024];
	//if (!fLog) {
		char sFile[1024];
		//GetModuleFileName(0, (LPWCH)sFile, 1024);
		GetModuleFileNameA(0, (LPSTR)sFile, 1024);
		char *sPath = GetFilePath(sFile);
		sprintf(sLogPath, "%s\\LittleBlitzer.log", sPath);
		delete sPath;

		fLog = fopen(sLogPath,"wt");
		fclose(fLog);
	//}
}

void Log(const char format[], ...) {
	if (!g_bLogging) return;
	if (!fLog) {
		ResetLog();
	}
	char buf[16*1024];
	va_list arg_list;
	va_start(arg_list,format);
	vsprintf(buf,format,arg_list);
	va_end(arg_list);

	fLog = fopen(sLogPath,"at");
	if (!fLog) return;
	if (buf[strlen(buf)-1]=='\n' || buf[strlen(buf)-1]=='\r')
		fprintf(fLog, "%s", buf);
	else
		fprintf(fLog, "%s\n", buf);
	fclose(fLog);
}

unsigned int LSB(BitBoard b) {
	// Assumes b is not zero.
	
	ASSERT(b);

	//__asm
	//{
	//	bsf eax,[b+4]
	//	xor eax,32
	//	bsf eax,[b]
	//}
	
	// v1.04.18
	unsigned long index; 
	if (b & 0xffffffff) { 
		_BitScanForward(&index, b & 0xffffffff); 
		return index;
	} 
	else { 
		_BitScanForward(&index, b >> 32); 
		return index + 32;
	}

	// Faster on AMD Phenom, intrinsics faster on Intel
	//return (unsigned int)(bsf_index[((b & 0-b) * debruijn) >> 58]);
}

unsigned int MSB(BitBoard b) {
	// Assumes b is not zero.

	ASSERT(b);

	//__asm
	//{
	//	bsr eax,[b]
	//	sub eax,32
	//	bsr eax,[b+4]
	//	add eax,32
	//}

	// v1.04.18
	unsigned long index; 
	if (b >> 32) { 
		_BitScanReverse(&index, b >> 32); 
		return index + 32; 
	} 
	else { 
		_BitScanReverse(&index, b & 0xffffffff); 
		return index; 
	} 
}

unsigned int CountBits(BitBoard bb) {

	// v1.04.18 - This is 4x faster on AMD which support it!!
	//return (unsigned int)__popcnt64(bb);

	unsigned int w=(unsigned int)(bb>>32),v=(unsigned int)bb;
	v=v-((v>>1) & 0x55555555);
	w=w-((w>>1) & 0x55555555);
	v=(v & 0x33333333)+((v>>2) & 0x33333333);
	w=(w & 0x33333333)+((w>> 2) & 0x33333333);
	v=(v+(v>>4)) & 0x0F0F0F0F;
	w=(w+(w>>4)) & 0x0F0F0F0F;
	v=((v+w)*0x01010101)>>24; 
	return v;
}

int RemoveBit(BitBoard &bb) {
// Equivalent to the following but without a table lookup and potential cache miss
//	sq = GetBit(bb);	// Get a square from the BitBoard
//	bb ^= bit[sq];	// Subtract that square from the BitBoard
	//ASSERT(bb);
	BitBoard tbb;
	int sq = GetBit(tbb = bb & -bb);
	bb ^= tbb;
	return sq;
}

BOOL CreateChildProcess(char *sPath, HANDLE hIn, HANDLE hOut, HANDLE *hProcess) 
{ 
   char szCmdline[1024];
   strcpy(szCmdline, sPath);
   PROCESS_INFORMATION piProcInfo; 
   STARTUPINFO siStartInfo;
   BOOL bFuncRetn = FALSE; 
 
// Set up members of the PROCESS_INFORMATION structure. 
 
   ZeroMemory( &piProcInfo, sizeof(PROCESS_INFORMATION) );
 
// Set up members of the STARTUPINFO structure. 
 
   ZeroMemory( &siStartInfo, sizeof(STARTUPINFO) );
   siStartInfo.cb = sizeof(STARTUPINFO); 
   siStartInfo.hStdError = hOut;
   siStartInfo.hStdOutput = hOut;
   siStartInfo.hStdInput = hIn;
   siStartInfo.dwFlags |= STARTF_USESTDHANDLES;
 
// Create the child process.

   char *p = strrchr(szCmdline, '\\');
   ASSERT(p);
   char sCWD[1024];
   ASSERT(p-szCmdline > 0);
   strncpy(sCWD, szCmdline, p-szCmdline);
   sCWD[p-szCmdline] = 0;
    
	wchar_t szCmdline_W[1024];
	wchar_t sCWD_W[1024];
	size_t dumb;
	mbstowcs_s(&dumb, szCmdline_W, strlen(szCmdline)+1, szCmdline, 1024);
	mbstowcs_s(&dumb, sCWD_W, strlen(sCWD)+1, sCWD, 1024);
	
	bFuncRetn = CreateProcess(NULL, 
	   //_T("C:\\Projects\\LittleThought\\Release\\LittleThought-1.05.48.exe"),
      szCmdline,     // command line 
      NULL,          // process security attributes 
      NULL,          // primary thread security attributes 
      TRUE,          // handles are inherited 
      CREATE_NO_WINDOW,             // creation flags 
      NULL,          // use parent's environment 
	  //_T("C:\\Projects\\LittleThought\\Release"),
      sCWD,  // cwd same as exe
      &siStartInfo,  // STARTUPINFO pointer 
      &piProcInfo);  // receives PROCESS_INFORMATION 
   
   // keep this one for killing the process later
   *hProcess = piProcInfo.hProcess;

   if (bFuncRetn == 0) {
	   int n = GetLastError();
	   TRACE("CreateProcess failed: %d\n",n);
   } else {
	   // dont need this one
      CloseHandle(piProcInfo.hThread);
   }

   return bFuncRetn;
}

wchar_t *ConvertCharToWCharBecauseMSDontProvideOne(char *str) {
	wchar_t w[1024];
	size_t dumb;
	mbstowcs_s(&dumb, w, strlen(str)+1, str, 1024);
	return w;
}
#include "StdAfx.h"
#include "Engine.h"
#include "Common.h"
#include "Timer.h"
#include "Tournament.h"
#include <io.h>
#include <fcntl.h>
#include <winsock2.h>

CEngine::CEngine(void)
{
	m_sPath = 0;
	m_sName = 0;
	m_sLBName = 0;
	m_sAuthor = 0;
	m_nHash = 0;
	m_bPonder = false;
	m_bOwnBook = false;
	m_nVariant = VARIANT_STD;

	m_nNumParameters = 0;

	m_nBufferSize = 0;
	m_sBuffer[READ] = 0;
	m_sBuffer[WRITE] = 0;

	m_nPipeRead[READ] = 0;
	m_nPipeRead[WRITE] = 0;
	m_nPipeWrite[READ] = 0;
	m_nPipeWrite[WRITE] = 0;

	m_sLastOutput[0] = 0;

	LockInit(&m_nLockEngine, NULL);
}


CEngine::~CEngine(void)
{
	//Quit();
	//LockFree(&m_nLockEngine);

	//delete m_sBuffer[READ];
	//delete m_sName;
	//delete m_sAuthor;
	//delete m_sPath;

	//m_sName = 0;
	//m_sAuthor = 0;
	//m_sBuffer[READ] = 0;

}

bool CEngine::Init() {
// Establish communications with the engine and initialise it with appropriate tournament parameters.

	if (m_sPath[0] == 0) return false;

	// ms-help://MS.VSCC.v90/MS.MSDNQTR.v90.en/dllproc/base/creating_a_child_process_with_redirected_input_and_output.htm

	m_sBuffer[READ] = new char[IO_BUFFER+1];
	//m_sBuffer[WRITE] = new char[IO_BUFFER+1];

	// Create a pipe for the child process's STDOUT. 
	//HANDLE hChildStdinRd, hChildStdinWr, hChildStdoutRd, hChildStdoutWr;
	// hChildStdinRd  = m_nPipeWrite[READ] <- used by child process
	// hChildStdinWr  = m_nPipeWrite[WRITE]
	// hChildStdoutRd = m_nPipeRead[READ]
	// hChildStdoutWr = m_nPipeRead[WRITE] <- used by child process
	HANDLE hStdout;
	SECURITY_ATTRIBUTES saAttr; 
	BOOL fSuccess; 

	// Set the bInheritHandle flag so pipe handles are inherited. 

	saAttr.nLength = sizeof(SECURITY_ATTRIBUTES); 
	saAttr.bInheritHandle = TRUE; 
	saAttr.lpSecurityDescriptor = NULL; 
// Get the handle to the current STDOUT. 
 
   hStdout = GetStdHandle(STD_OUTPUT_HANDLE); 
 
// Create a pipe for the child process's STDOUT. 
 
   if (! CreatePipe(&m_nPipeRead[READ], &m_nPipeRead[WRITE], &saAttr, 0)) {
		TRACE("Stdout pipe creation failed\n");
		CString s;
		s.Format(_T("Count not create read pipe for %s"),m_sPath);
		MessageBox(NULL,s,_T("Error"),MB_OK);
		return false;
   }

// Ensure the read handle to the pipe for STDOUT is not inherited.

   SetHandleInformation( m_nPipeRead[READ], HANDLE_FLAG_INHERIT, 0);
   DWORD mode = PIPE_READMODE_BYTE|PIPE_NOWAIT;
   if (!SetNamedPipeHandleState(m_nPipeRead[READ], &mode, NULL, NULL)) {
		TRACE("SetNamedPipeHandleState failed\n");
		CString s;
		s.Format(_T("Count not set named pipe for %s"),m_sPath);
		MessageBox(NULL,s,_T("Error"),MB_OK);
		return false;
   }

// Create a pipe for the child process's STDIN. 
 
	if (! CreatePipe(&m_nPipeWrite[READ], &m_nPipeWrite[WRITE], &saAttr, 0)) {
		TRACE("Stdin pipe creation failed\n"); 
		CString s;
		s.Format(_T("Count not create write pipe for %s"),m_sPath);
		MessageBox(NULL,s,_T("Error"),MB_OK);
		return false;
	}

// Ensure the write handle to the pipe for STDIN is not inherited. 
 
   SetHandleInformation( m_nPipeWrite[WRITE], HANDLE_FLAG_INHERIT, 0);
 
// Now create the child process. 
   
	fSuccess = CreateChildProcess(m_sPath, m_nPipeWrite[READ], m_nPipeRead[WRITE], &m_nProcess);
	if (! fSuccess) {
		TRACE("Create process failed\n"); 
		CString s;
		s.Format(_T("Could not load process %s (%s)"),m_sPath,GetLastError());
		MessageBox(NULL,s,_T("Error"),MB_OK);
		return false;
	}
 
   m_nBufferSize = 0;

	CString sLine;
	Send("uci");

	// Ignore any title crap until get to first proper uci response
	do {
		GetLine(&sLine);
		//TRACE("> %s\n",sLine);
	//} while (!wcsstr(sLine.GetBuffer(), _T("id name")));
	} while (sLine.Find("id name") == -1);
	long a,b,c;
	ProcessInput(sLine,&a,&b,&c);

	// Process options until ready
	do {
		GetLine(&sLine);
		//TRACE("> %s\n",sLine);
		long a,b;
		ProcessInput(sLine,&a,&b,&c);
	} while (sLine.Find("uciok") == -1);

	Send("setoption name Hash value %d", m_nHash?m_nHash:1); // dont initialise with 0 or some engines crash
	Send("setoption name Ponder value %s", m_bPonder?"true":"false");
	Send("setoption name OwnBook value %s", m_bOwnBook?"true":"false");
	if (m_nVariant == VARIANT_960) {
		Send("setoption name UCI_Chess960 value true");
	}

	// Send customised UCI options
	for (int i=0;i<m_nNumParameters;i++) {
		Send("setoption name %s value %s", m_sParameterNames[i], m_sParameterValues[i]);
	}

	Send("isready");

	do {
		GetLine(&sLine);
		//TRACE("> %s\n",sLine);
	} while (sLine.Find("readyok") == -1);


	return true;
	
}

void CEngine::NewGame() {
	CString sLine;

	Send("ucinewgame");

	Send("isready");
	do {	// TODO: chance of hanging here if engine crashes
		GetLine(&sLine);
		Sleep(10);
	} while (sLine.Find("readyok") == -1);
}

CString CEngine::Search(CString sStartingPositionFEN, CString sMoves, int nTC, long nWhiteTime, long nBlackTime, long nWhiteInc, long nBlackInc, long nTimeOut, long *nDepth, long *nNPS, long *nScore, CTimer *t) {
	CString sLine;
	//CTimer t;
	CString s;

	CString startpos;
	if (!sStartingPositionFEN.Compare(_T("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 0"))) {
		// Because some programs dont like using fen as the starting position
		startpos.AppendFormat(_T("startpos"));
	} else {
		startpos.AppendFormat(_T("fen %s"), sStartingPositionFEN);
	}

	if (sMoves.GetLength() > 0) {
		s.AppendFormat(_T("position %s moves%s"), startpos, sMoves);
		Send(s);
	} else {
		s.AppendFormat(_T("position %s"), startpos);
		Send(s);
	}
	if (nTC == TC_FIXED_TPM) {
		s.Format(_T("go movetime %ld"), nWhiteTime);
	} else if (nTC == TC_BLITZ) {
		s.Format(_T("go wtime %ld btime %ld winc %ld binc %ld"), nWhiteTime, nBlackTime, nWhiteInc, nBlackInc);
	} else {
		s.Format(_T("go movestogo %ld wtime %ld btime %ld"), nWhiteInc, nWhiteTime, nBlackTime);
	}

	t->Start();
	Send(s);

	do {
		GetLine(&sLine);
		t->Stop();	// continuously update end time to avoid ProcessInput overhead
		ProcessInput(sLine, nDepth, nNPS, nScore);
	} while (sLine.Find("bestmove")==-1 
		&& (/*nTC==TC_FIXED_TPM || */t->GetMS() <= nTimeOut+1000)
		); // small buffer for accuracy errors?
	// 1 sec buffer should be heaps. If its that inaccurate it deserves to lose.
	if (t->GetMS() > nTimeOut+1000) {
		ASSERT(false);
	}

	strncpy(m_sLastOutput, sLine.GetBuffer(), 100);
	m_sLastOutput[99] = 0;

	// return asap
	return sLine;
	
	/*t.Stop();

	CStringArray sWords;
	GetWords(sLine, &sWords);

	if (sWords.GetCount() > 1)
		return sWords[1];
	else
		return NULL;*/
}

void CEngine::Stop() {
	Send("stop");
}

void CEngine::Quit() {

	// Multiple threads may try to stop the engine if the user quits the app
	Lock(&m_nLockEngine);
	
	// Kill em
	Send("stop");
	Send("quit");

	// Wait until child process exits (30 sec timeout)
    DWORD r = WaitForSingleObject( m_nProcess, 30000 );
	if (r == WAIT_TIMEOUT) {
		//char msg[100];
		//sprintf(msg,"Process is hanging: %s. Press OK to kill.", m_sName);
		//AfxMessageBox(msg);

		// Long enough, kill em
		BOOL b = TerminateProcess(m_nProcess, 1);
		if (!b) {
			// 5 = ERROR_ACCESS_DENIED (already dead)
			// 6 = ERROR_INVALID_HANDLE (oops, handle already destroyed?)
			DWORD e = GetLastError();
			if (e != 5) {
				char err[100];
				sprintf(err,(const char*)"Terminate %s errored: %d",m_sName,e);
				AfxMessageBox((LPCTSTR)err);
			}
		}
	}

	if (m_sBuffer)
		delete m_sBuffer[READ];
	delete m_sName;
	delete m_sAuthor;

	if (m_sBuffer)
		m_sBuffer[READ] = 0;
	m_sName = 0;
	m_sAuthor = 0;

	// Close the pipe handle so the child process stops reading. 
 
	// v2.75 - uncomment to prevent limit of 2048 pipes per process? why was this commented out?
	CloseHandle(m_nPipeWrite[WRITE]);
	CloseHandle(m_nPipeRead[READ]);

	//if (!CloseHandle(m_nPipeWrite[WRITE])) 
		//TRACE("Close pipe failed\n"); 
	//if (!CloseHandle(m_nPipeRead[READ])) 
		//TRACE("Close pipe failed\n"); 

	Unlock(&m_nLockEngine);

}

void CEngine::ProcessInput(CString sLine, long *nDepth, long *nNPS, long *nScore) {
	int pos = 0;
	CStringArray sWords;

	if (sLine.GetLength() == 0) return;

	int nNumWords = GetWords(sLine, &sWords);

	if (!sWords.GetAt(0).CompareNoCase(_T("info"))) {
		for (int i=1;i<sWords.GetCount();i++) {
			CString s = sWords.GetAt(i);
			if (!s.CompareNoCase(_T("depth"))) {
				CString s2 = sWords.GetAt(i+1);
				*nDepth = atol(s2.GetBuffer());
			} else if (!s.CompareNoCase(_T("nps"))) {
				CString s2 = sWords.GetAt(i+1);
				*nNPS = atol(s2.GetBuffer());
			} else if (!s.CompareNoCase(_T("cp"))) {
				CString s2 = sWords.GetAt(i+1);
				*nScore = atol(s2.GetBuffer());
			} else if (!s.CompareNoCase(_T("mate"))) {
				CString s2 = sWords.GetAt(i+1);
				*nScore = atol(s2.GetBuffer())*2;
				if (*nScore < 0) *nScore = -30000-*nScore;
				else *nScore = 30000-*nScore;
			}
		}
	} else if (!sWords.GetAt(0).CompareNoCase(_T("id"))) {
		if (!sWords.GetAt(1).CompareNoCase(_T("name"))) {
			int nLen = 0;
			for (int i=2;i<sWords.GetSize();i++) {
				nLen += sWords.GetAt(i).GetLength() + 1;
			}
			if (m_sLBName) { // use this instead
				m_sName = new char[strlen(m_sLBName)+1];
				strncpy(m_sName, m_sLBName, strlen(m_sLBName));
				m_sName[strlen(m_sLBName)] = 0;
			} else {
				m_sName = new char[nLen+1];
				m_sName[0] = 0;
				for (int i=0;i<sWords.GetSize()-2;i++) {
					CString s = sWords.GetAt(2+i);
					strcat(m_sName, s.GetBuffer());
					strcat(m_sName, " ");
				}
				m_sName[nLen-1] = 0;
			}
		} else if (!sWords.GetAt(1).CompareNoCase(_T("author"))) {
			CString s = sWords.GetAt(2);
			//char asc[1024];	size_t dumb;
			//wcstombs_s(&dumb, asc, s.GetLength()+1, s.GetBuffer(), 1024);
			m_sAuthor = new char[s.GetLength()+1];
			strcpy(m_sAuthor, s.GetBuffer());
		}
	} else if (!sWords.GetAt(0).CompareNoCase(_T("option"))) {
		// who cares
	}

	sWords.RemoveAll();
}

int CEngine::GetWords(CString sLine, CStringArray *sWords) {
	CString sWord;
	int nNumWords = 0;
	int pos = 0;
	sWord = sLine.Tokenize(_T(" "), pos);
	while (pos != -1) {
		sWords->Add(sWord);
		nNumWords ++;
		sWord = sLine.Tokenize(_T(" "), pos);
	}

	return nNumWords;
}

void CEngine::Send(const char format[], ...) {
	char buf[4096];
	va_list arg_list;
	va_start(arg_list,format);
	vsprintf(buf,format,arg_list);
	va_end(arg_list);

	CString sLine(buf);
	Send(sLine);
}

void CEngine::Send(CString sLine) {

	DWORD dwWritten; 

	// Read from a file and write its contents to a pipe. 
	if (sLine[sLine.GetLength()-1] != '\n') {
		sLine.Append(_T("\n"));
	}

	//char *sLineA = new char[sLine.GetLength()+1];
	//size_t dumb;
	//wcstombs_s(&dumb, sLineA, sLine.GetLength()+1, sLine.GetBuffer(), 1024);

	Log("-->(%s) %s", m_sName, sLine);

	if (! WriteFile(m_nPipeWrite[WRITE], sLine, sLine.GetLength(), &dwWritten, NULL)) {
		//TRACE("WriteFile crap\n"); 
		//delete sLineA;
		return;
	}

	//delete sLineA;
}

void CEngine::GetLine(CString *sLine) {
// Waits for a line to be available and then returns it.

	//char buf[1024];
	//int res;
	while (!IsDataWaiting() 
		&& UpdateBuffer());

	if (!m_nBufferSize) return;

	// Line is ready

	char *p = (char *)memchr(m_sBuffer[READ], '\n', m_nBufferSize);
	if (!p) return;

	int nLen = p-m_sBuffer[READ]+1;
	ASSERT(nLen>0);
	int n=nLen;
	if (m_sBuffer[READ][nLen-1] == '\r' || m_sBuffer[READ][nLen-1] == '\n')	n--;
	if (m_sBuffer[READ][nLen-2] == '\r' || m_sBuffer[READ][nLen-2] == '\n')	n--;

	//wchar_t sLineW[1024];
	//size_t dumb;
	//mbstowcs_s(&dumb, sLineW, n+1, m_sBuffer[READ], n);

	//m_sBuffer[READ][nLen]=0;
	char logbuf[10240];
	strncpy(logbuf,m_sBuffer[READ],nLen);
	logbuf[nLen-1]=0;
	Log("<--(%s) %s", m_sName, logbuf);

	sLine->SetString(m_sBuffer[READ], n); // ignore \r\n
	for (int i=0;i<m_nBufferSize-nLen;i++) {
		m_sBuffer[READ][i] = m_sBuffer[READ][i+nLen];
	}
	m_nBufferSize -= nLen;
	m_sBuffer[READ][m_nBufferSize] = 0;

}

bool CEngine::IsDataWaiting() {
// Check if an entire line is waiting in the buffer.
	if (!memchr(m_sBuffer[READ], '\n', m_nBufferSize))
		return false;
	else
		return true;
}

bool CEngine::UpdateBuffer() {
// Copy data from input pipe to buffer.

	int nSize = IO_BUFFER - m_nBufferSize;

   DWORD dwRead;//, dwWritten; 
 
	// Read from a file and write its contents to a pipe. 

   if (!ReadFile(m_nPipeRead[READ], m_sBuffer[READ] + m_nBufferSize, nSize, &dwRead, NULL)) {
	   DWORD err = GetLastError();
	   
	   // dwRead could be 0
	   if (dwRead == 0) {
		   // no data ready, give the cpu a break
		   Sleep(10);
	   }
		//TRACE("crap\n");
		return false;
   }
   ASSERT(dwRead > 0);

	//int chars = read(m_nPipeRead[READ], m_sBuffer[READ] + m_nBufferSize, nSize);
	//ASSERT(chars >= 0);
	m_nBufferSize += dwRead;
	ASSERT(m_nBufferSize <= IO_BUFFER);

	return true;
}



// LittleBlitzer.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


#define VERSION	"2.75"


// CLittleBlitzerApp:
// See LittleBlitzer.cpp for the implementation of this class
//

class CLittleBlitzerApp : public CWinAppEx
{
public:
	CLittleBlitzerApp();

// Overrides
	public:
	virtual BOOL InitInstance();

// Implementation

	DECLARE_MESSAGE_MAP()
};

extern CLittleBlitzerApp theApp;
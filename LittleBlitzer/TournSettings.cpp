// TournSettings.cpp : implementation file
//

#include "stdafx.h"
#include "LittleBlitzer.h"
#include "TournSettings.h"
#include "afxdialogex.h"


// CTournSettings dialog

IMPLEMENT_DYNAMIC(CTournSettings, CDialogEx)

CTournSettings::CTournSettings(CWnd* pParent /*=NULL*/)
	: CDialogEx(CTournSettings::IDD, pParent)
	, m_nType(0)
	, m_nVariant(0)
	, m_nPosition(0)
	, m_nTC(0)
{
	m_nRounds = 0;
	m_nParallel = 0;
	m_nHash = 0;
	m_nType = 0;
	m_nAdjMateScore = 0;
	m_nAdjMateMoves = 0;
	m_nAdjDrawMoves = 0;
	m_nTimeBase = 0;
	m_nTimeInc = 0;
	m_nPonder = 0;
	m_nOwnBook = 0;
	m_nVariant = 0;
	m_nPosition = 0;
	m_nRandomize = 0;
}

CTournSettings::~CTournSettings()
{
}

void CTournSettings::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_ROUNDS, m_wndRounds);
	DDX_Control(pDX, IDC_PARALLEL, m_wndParallel);
	DDX_Control(pDX, IDC_HASH, m_wndHash);
	DDX_Control(pDX, IDC_MATESCORE, m_wndAdjMateScore);
	DDX_Control(pDX, IDC_MATEMOVES, m_wndMateMoves);
	DDX_Control(pDX, IDC_DRAWMOVES, m_wndDrawMoves);
	DDX_Control(pDX, IDC_TIMEBASE, m_wndTimeBase);
	DDX_Control(pDX, IDC_TIMEINC, m_wndTimeInc);
	DDX_Control(pDX, IDC_PONDER, m_wndPonder);
	DDX_Control(pDX, IDC_OWNBOOK, m_wndOwnBook);
	DDX_Control(pDX, IDC_STARTING, m_wndPosition);
	DDX_Radio(pDX, IDC_GAUNTLET, m_nType);
	DDX_Radio(pDX, IDC_STANDARD, m_nVariant);
	DDX_Radio(pDX, IDC_OPENING, m_nPosition);
	DDX_Radio(pDX, IDC_FIXEDTPM, m_nTC);
	DDX_Control(pDX, IDC_TC_LBL_1, m_wndTC1);
	DDX_Control(pDX, IDC_TC_LBL_2, m_wndTC2);
	DDX_Control(pDX, IDC_TC_LBL_3, m_wndTC3);
	DDX_Control(pDX, IDC_TC_LBL_4, m_wndTC4);
	DDX_Control(pDX, IDC_RANDOMIZE, m_wndRandomize);
}


BEGIN_MESSAGE_MAP(CTournSettings, CDialogEx)
	ON_BN_CLICKED(IDC_OPENING, &CTournSettings::OnBnClickedOpening)
	ON_BN_CLICKED(IDC_FEN, &CTournSettings::OnBnClickedFen)
	ON_BN_CLICKED(IDC_EPD, &CTournSettings::OnBnClickedEpd)
	ON_BN_CLICKED(IDC_PGN, &CTournSettings::OnBnClickedPgn)
	ON_BN_CLICKED(IDOK, &CTournSettings::OnBnClickedOk)
	ON_BN_CLICKED(IDC_FIXEDTPM, &CTournSettings::OnBnClickedFixedtpm)
	ON_BN_CLICKED(IDC_BLITZ, &CTournSettings::OnBnClickedBlitz)
	ON_BN_CLICKED(IDC_TOURN, &CTournSettings::OnBnClickedTourn)
END_MESSAGE_MAP()



BOOL CTournSettings::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	CString s;
	s.Format("%ld",m_nRounds);
	m_wndRounds.SetWindowText(s);
	s.Format("%ld",m_nParallel);
	m_wndParallel.SetWindowText(s);
	s.Format("%ld",m_nHash);
	m_wndHash.SetWindowText(s);
	s.Format("%ld",m_nAdjMateScore);
	m_wndAdjMateScore.SetWindowText(s);
	s.Format("%ld",m_nAdjMateMoves);
	m_wndMateMoves.SetWindowText(s);
	s.Format("%ld",m_nAdjDrawMoves);
	m_wndDrawMoves.SetWindowText(s);
	s.Format("%ld",m_nTimeBase);
	m_wndTimeBase.SetWindowText(s);
	s.Format("%ld",m_nTimeInc);
	m_wndTimeInc.SetWindowText(s);
	m_wndPonder.SetCheck(m_nPonder);
	m_wndOwnBook.SetCheck(m_nOwnBook);

	m_wndPosition.EnableWindow(m_nPosition != 0);
	m_wndPosition.SetWindowTextA(m_sStartingPosition);
	m_wndRandomize.EnableWindow(m_nPosition != 0);
	m_wndRandomize.SetCheck(m_nRandomize);

	UpdateTC();

	return FALSE;  // return TRUE unless you set the focus to a control
}


void CTournSettings::OnBnClickedOpening()
{
	m_nPosition = 0;
	m_wndPosition.EnableWindow(m_nPosition != 0);
	m_wndRandomize.EnableWindow(false);
}


void CTournSettings::OnBnClickedFen()
{
	m_nPosition = 1;
	m_wndPosition.EnableWindow(m_nPosition != 0);
	m_wndRandomize.EnableWindow(false);
}


void CTournSettings::OnBnClickedEpd()
{
	m_nPosition = 2;
	m_wndPosition.EnableWindow(m_nPosition != 0);
	m_wndRandomize.EnableWindow(m_nPosition != 0);
}


void CTournSettings::OnBnClickedPgn()
{
	m_nPosition = 3;
	m_wndPosition.EnableWindow(m_nPosition != 0);
	m_wndRandomize.EnableWindow(m_nPosition != 0);
}


void CTournSettings::OnBnClickedOk()
{
	CString s;
	m_wndRounds.GetWindowTextA(s);
	m_nRounds = atol(s);
	m_wndParallel.GetWindowTextA(s);
	m_nParallel = atol(s);
	m_wndHash.GetWindowTextA(s);
	m_nHash = atol(s);
	m_wndAdjMateScore.GetWindowTextA(s);
	m_nAdjMateScore = atol(s);
	m_wndMateMoves.GetWindowTextA(s);
	m_nAdjMateMoves = atol(s);
	m_wndDrawMoves.GetWindowTextA(s);
	m_nAdjDrawMoves = atol(s);
	m_wndTimeBase.GetWindowTextA(s);
	m_nTimeBase = atol(s);
	m_wndTimeInc.GetWindowTextA(s);
	m_nTimeInc = atol(s);
	m_nPonder = m_wndPonder.GetCheck();
	m_nOwnBook = m_wndOwnBook.GetCheck();
	m_wndPosition.GetWindowText(m_sStartingPosition,MAX_FEN_LEN);
	m_nRandomize = m_wndRandomize.GetCheck();

	CDialogEx::OnOK();
}



void CTournSettings::OnBnClickedFixedtpm()
{
	m_nTC = 0;
	UpdateTC();
}


void CTournSettings::OnBnClickedBlitz()
{
	m_nTC = 1;
	UpdateTC();
}


void CTournSettings::OnBnClickedTourn()
{
	m_nTC = 2;
	UpdateTC();
}

void CTournSettings::UpdateTC() {
	if (m_nTC == 0) {
		// Fixed TPM
		m_wndTC1.SetWindowText("Time:");
		m_wndTC2.ShowWindow(false);
		m_wndTC3.SetWindowText("ms");
		m_wndTC4.ShowWindow(false);
		m_wndTimeInc.ShowWindow(false);
	} else if (m_nTC == 1) {
		// Blitz base+inc
		m_wndTC1.SetWindowText("Base:");
		m_wndTC2.SetWindowText("Inc:");
		m_wndTC2.ShowWindow(true);
		m_wndTC3.SetWindowText("ms");
		m_wndTC4.SetWindowText("ms");
		m_wndTC4.ShowWindow(true);
		m_wndTimeInc.ShowWindow(true);
	} else {
		// Tournament time for moves
		m_wndTC1.SetWindowText("Time:");
		m_wndTC2.SetWindowText("Moves:");
		m_wndTC2.ShowWindow(true);
		m_wndTC3.SetWindowText("ms for");
		m_wndTC4.SetWindowText("repeating");
		m_wndTC4.ShowWindow(true);
		m_wndTimeInc.ShowWindow(true);
	}
}
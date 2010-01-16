// RosterWeek.cpp : implementation file
//

#include "stdafx.h"
extern "C" {
#include "TMSHeader.h"
}

#include "TMS.h"
#include "RosterWeek.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CRosterWeek dialog


CRosterWeek::CRosterWeek(CWnd* pParent /*=NULL*/)
	: CDialog(CRosterWeek::IDD, pParent)
{
	//{{AFX_DATA_INIT(CRosterWeek)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CRosterWeek::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CRosterWeek)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CRosterWeek, CDialog)
	//{{AFX_MSG_MAP(CRosterWeek)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDHELP, OnHelp)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CRosterWeek message handlers

BOOL CRosterWeek::OnInitDialog() 
{
	CDialog::OnInitDialog();
//
//  Set up a pointer to the control
//
  pEditWEEK = (CEdit *)GetDlgItem(ROSTERWEEK_WEEK);
//
//  Default to week m_RosterWeek
//
  sprintf(tempString, "%ld", m_RosterWeek + 1);
  pEditWEEK->SetWindowText(tempString);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CRosterWeek::OnClose() 
{
	// TODO: Add your message handler code here and/or call default
	
	CDialog::OnClose();
}

void CRosterWeek::OnCancel() 
{
	// TODO: Add extra cleanup here
	
	CDialog::OnCancel();
}

void CRosterWeek::OnHelp() 
{
	// TODO: Add your control notification handler code here
	
}

void CRosterWeek::OnOK() 
{
  long tempLong;

  pEditWEEK->GetWindowText(tempString, TEMPSTRING_LENGTH);

  tempLong = atol(tempString);

  if(tempLong < 1 || tempLong > ROSTER_MAX_WEEKS + 1)
  {
    return;
  }

  m_RosterWeek = tempLong - 1;
	
	CDialog::OnOK();
}

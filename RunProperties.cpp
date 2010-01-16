//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
// RunProperties.cpp : implementation file
//

#include "stdafx.h"
extern "C"
{
#include "TMSHeader.h"
#include "cistms.h"
}
#include "RunProperties.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CRunProperties dialog


CRunProperties::CRunProperties(CWnd* pParent, long runNumber)
	: CDialog(CRunProperties::IDD, pParent)
{
  m_RunNumber = runNumber;

	//{{AFX_DATA_INIT(CRunProperties)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CRunProperties::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CRunProperties)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CRunProperties, CDialog)
	//{{AFX_MSG_MAP(CRunProperties)
	ON_BN_CLICKED(IDHELP, OnHelp)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDSHOWDIRECTIONS, OnShowdirections)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CRunProperties message handlers

BOOL CRunProperties::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
  pStaticREASON = (CStatic *)GetDlgItem(RUNPROPERTIES_REASON);
  pButtonSHOWDIRECTIONS = (CButton *)GetDlgItem(IDSHOWDIRECTIONS);

  int nI, nJ;
  int index = NO_RECORD;
  long cutAsRuntype = NO_RECORD;

  for(nI = 0; nI < MAXRUNSINRUNLIST; nI++)
  {
    if(RUNLIST[nI].runNumber == m_RunNumber)
    {
      index = nI;
      for(nJ = 0; nJ < MAXRUNSINRUNLIST; nJ++)
      {
        if(m_RunNumber == RUNLISTDATA[nJ].runNumber)
        {
          cutAsRuntype = RUNLISTDATA[nJ].cutAsRuntype;
          break;
        }
      }
      break;
    }
  }
//
// Must have found it
//
  if(index == NO_RECORD)
  {
    OnCancel();
    return TRUE;
  }
//
//  Set the title
//
  sprintf(tempString, "Run %ld Properties", m_RunNumber);
  this->SetWindowText(tempString);
//
//  Cost the run
//
  bGenerateTravelInstructions = TRUE;
  RunCoster(&RUNLIST[index].run, cutAsRuntype, &m_COST);
  bGenerateTravelInstructions = FALSE;
//
//  Dynamic travels?
//
  if(bUseDynamicTravels)
  {
    if(bUseCISPlan)
      pButtonSHOWDIRECTIONS->EnableWindow(TRUE);
    else
      pButtonSHOWDIRECTIONS->EnableWindow(numTravelInstructions > 0);
  }
//
//  RunCoster reason
//
  pStaticREASON->SetWindowText(runcosterReason);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CRunProperties::OnOK() 
{
	CDialog::OnOK();
}


void CRunProperties::OnCancel() 
{
	// TODO: Add extra cleanup here
	
	CDialog::OnCancel();
}

void CRunProperties::OnHelp() 
{
	// TODO: Add your control notification handler code here
	
}

void CRunProperties::OnClose() 
{
	CDialog::OnClose();
}

void CRunProperties::OnShowdirections() 
{
  if(bUseCISPlan)
  {
    DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_TRAVELINSTRUCTIONS),
          m_hWnd, (DLGPROC)TRAVELINSTRUCTIONSMsgProc, (LPARAM)&m_COST);
  }
  else
  {
    DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_TRAVELINSTRUCTIONS),
          m_hWnd, (DLGPROC)TRAVELINSTRUCTIONSMsgProc, (LPARAM)&m_RunNumber);
  }
}

//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
// ReassignRuntype.cpp : implementation file
//

#include "stdafx.h"
extern "C"
{
#include "TMSHeader.h"
}
#include "ReassignRuntype.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CReassignRuntype dialog


CReassignRuntype::CReassignRuntype(CWnd* pParent, long runNumber)
	: CDialog(CReassignRuntype::IDD, pParent)
{
  m_RunNumber = runNumber;
	//{{AFX_DATA_INIT(CReassignRuntype)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CReassignRuntype::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CReassignRuntype)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CReassignRuntype, CDialog)
	//{{AFX_MSG_MAP(CReassignRuntype)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDHELP, OnHelp)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CReassignRuntype message handlers

BOOL CReassignRuntype::OnInitDialog() 
{
	CDialog::OnInitDialog();

  pStaticCURRENT = (CStatic *)GetDlgItem(REASSIGNRUNTYPE_CURRENT);
  pComboBoxRUNTYPES = (CComboBox *)GetDlgItem(REASSIGNRUNTYPE_RUNTYPES);

//
//  Get the current runtype/number of pieces
//
  int nI;

  m_RUNLISTDATAIndex = NO_RECORD;
  for(nI = 0; nI < MAXRUNSINRUNLIST; nI++)
  {
    if(m_RunNumber == RUNLISTDATA[nI].runNumber)
    {
      m_CurrentRuntype = RUNLISTDATA[nI].cutAsRuntype;
      m_RUNLISTDATAIndex = nI;
      break;
    }
  }
//
//  Must have found it
//
  if(m_RUNLISTDATAIndex == NO_RECORD)
  {
    TMSError(m_hWnd, MB_ICONSTOP, ERROR_287 , (HANDLE)NULL);
    OnCancel();
    return TRUE;
  }
//
//  If it's UNCLASSIFIED_RUNTYPE he can't change it
//
//  if(m_CurrentRuntype == UNCLASSIFIED_RUNTYPE)
//  {
//    TMSError(m_hWnd, MB_ICONSTOP, ERROR_286 , (HANDLE)NULL);
//    OnCancel();
//    return TRUE;
//  }
  
//  int type = LOWORD(m_CurrentRuntype);
//  int slot = HIWORD(m_CurrentRuntype);
//  int numPieces = RUNTYPE[type][slot].numPieces;

//  pStaticCURRENT->SetWindowText(RUNTYPE[type][slot].localName);

  int type;
  int slot;
  int numPieces;

  if(m_CurrentRuntype == UNCLASSIFIED_RUNTYPE)
  {
    type = NO_RECORD;
    slot = NO_RECORD;
    numPieces = NO_RECORD;
    pStaticCURRENT->SetWindowText("Unclassified / Illegal");
  }
  else
  {
    type = LOWORD(m_CurrentRuntype);
    slot = HIWORD(m_CurrentRuntype);
    numPieces = RUNTYPE[type][slot].numPieces;
    pStaticCURRENT->SetWindowText(RUNTYPE[type][slot].localName);
  }

//
//  Run types - only display those that correspond to the number of pieces
//
  int nJ, nK;

  for(nI = 0; nI < NUMRUNTYPES; nI++)
  {
    for(nJ = 0; nJ < NUMRUNTYPESLOTS; nJ++)
    {
      if(RUNTYPE[nI][nJ].flags & RTFLAGS_INUSE)
      {
        if(numPieces == NO_RECORD || RUNTYPE[nI][nJ].numPieces == numPieces)
        {
          nK = pComboBoxRUNTYPES->AddString(RUNTYPE[nI][nJ].localName);
          pComboBoxRUNTYPES->SetItemData(nK, MAKELONG(nI, nJ));
          if(nI == type && nJ == slot)
          {
            pComboBoxRUNTYPES->SetCurSel(nK);
          }
        }
      }
    }
  }

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CReassignRuntype::OnOK() 
{
  long selectedRuntype;
  int  nI;

  nI = pComboBoxRUNTYPES->GetCurSel();
  selectedRuntype = pComboBoxRUNTYPES->GetItemData(nI);
  if(selectedRuntype == m_CurrentRuntype)
  {
    OnCancel();
  }
  else
  {
    RUNLISTDATA[m_RUNLISTDATAIndex].cutAsRuntype = selectedRuntype;
    CDialog::OnOK();
  }
}

void CReassignRuntype::OnCancel() 
{
	CDialog::OnCancel();
}

void CReassignRuntype::OnClose() 
{
	OnCancel();
}

void CReassignRuntype::OnHelp() 
{
	// TODO: Add your control notification handler code here
	
}

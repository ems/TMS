//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
// AddDirection.cpp : implementation file
//

#include "stdafx.h"
extern "C"{
#include "TMSHeader.h"
}

#include "tms.h"
#include "AddDialogs.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CAddDirection dialog


CAddDirection::CAddDirection(CWnd* pParent /*=NULL*/, long *pUpdateRecordID)
	: CDialog(CAddDirection::IDD, pParent)
{
  m_pUpdateRecordID = pUpdateRecordID;
  m_bInit = TRUE;

	//{{AFX_DATA_INIT(CAddDirection)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CAddDirection::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAddDirection)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CAddDirection, CDialog)
	//{{AFX_MSG_MAP(CAddDirection)
	ON_BN_CLICKED(ADDDIRECTION_CC, OnCc)
	ON_CBN_SELENDOK(ADDDIRECTION_COMMENTCODE, OnSelendokCommentcode)
	ON_BN_CLICKED(ADDDIRECTION_NA, OnNa)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDHELP, OnHelp)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAddDirection message handlers

void CAddDirection::OnCc() 
{
  pComboBoxCOMMENTCODE->SetCurSel(0);
}

void CAddDirection::OnSelendokCommentcode() 
{
  if(pButtonNA->GetCheck())
  {
    pButtonNA->SetCheck(FALSE);
    pButtonCC->SetCheck(TRUE);
  }
}

void CAddDirection::OnNa() 
{
  pComboBoxCOMMENTCODE->SetCurSel(-1);
}

void CAddDirection::OnClose() 
{
  OnCancel();
}

BOOL CAddDirection::OnInitDialog() 
{
	CDialog::OnInitDialog();

  CString s;
  BOOL    bFound;
  int     numComments;
  int     nI;
//
//  Set up the pointers to the controls
//	
  pEditABBRNAME = (CEdit *)GetDlgItem(ADDDIRECTION_ABBRNAME);
  pEditLONGNAME = (CEdit *)GetDlgItem(ADDDIRECTION_LONGNAME);
  pButtonNA = (CButton *)GetDlgItem(ADDDIRECTION_NA);
  pButtonCC = (CButton *)GetDlgItem(ADDDIRECTION_CC);
  pComboBoxCOMMENTCODE = (CComboBox *)GetDlgItem(ADDDIRECTION_COMMENTCODE);
	
//
//  Comment codes combo box
//
  numComments = SetUpCommentList(m_hWnd, ADDDIRECTION_COMMENTCODE);
  if(numComments == 0)
  {
    pButtonCC->EnableWindow(FALSE);
    pComboBoxCOMMENTCODE->EnableWindow(FALSE);
  }
//
//  Are we updating?
//
  if(*m_pUpdateRecordID == NO_RECORD)
    pButtonNA->SetCheck(TRUE);
//
//  Yes - Get the record
//
  else
  {
    DIRECTIONSKey0.recordID = *m_pUpdateRecordID;
    btrieve(B_GETEQUAL, TMS_DIRECTIONS, &DIRECTIONS, &DIRECTIONSKey0, 0);
//
//  Abbreviated name
//
    s = GetDatabaseString(DIRECTIONS.abbrName, DIRECTIONS_ABBRNAME_LENGTH);
    pEditABBRNAME->SetWindowText(s);
//
//  Long name
//
    s = GetDatabaseString(DIRECTIONS.longName, DIRECTIONS_LONGNAME_LENGTH);
    pEditLONGNAME->SetWindowText(s);
//
//  Comment
//
    if(DIRECTIONS.COMMENTSrecordID == NO_RECORD)
      pButtonNA->SetCheck(TRUE);
    else
    {
      for(bFound = FALSE, nI = 0; nI < numComments; nI++)
      {
        if((long)pComboBoxCOMMENTCODE->GetItemData(nI) == DIRECTIONS.COMMENTSrecordID)
        {
          pComboBoxCOMMENTCODE->SetCurSel(nI);
          bFound = TRUE;
          break;
        }
      }
      CButton* pb = (bFound ? pButtonCC : pButtonNA);
      pb->SetCheck(TRUE);
    }
  }

  m_bInit = FALSE;

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CAddDirection::OnCancel() 
{
	CDialog::OnCancel();
}

void CAddDirection::OnHelp() 
{
  ::WinHelp(m_hWnd, szarHelpFile, HELP_CONTEXT, The_Directions_Table);
}

void CAddDirection::OnOK() 
{
  CString s;
  int     nI;
  int     rcode2;
  int     opCode;
//
//  Abbreviated name can't be blank
//
  pEditABBRNAME->GetWindowText(s);
  if(s.IsEmpty())
  {
    TMSError(m_hWnd, MB_ICONSTOP, ERROR_003, pEditABBRNAME->m_hWnd);
    return;
  }
//
//  Neither can the long name
//
  pEditABBRNAME->GetWindowText(s);
  if(s.IsEmpty())
  {
    TMSError(m_hWnd, MB_ICONSTOP, ERROR_004, pEditLONGNAME->m_hWnd);
    return;
  }
//
//  Insert / update?
//
  if(*m_pUpdateRecordID == NO_RECORD)
  {
    rcode2 = btrieve(B_GETLAST, TMS_DIRECTIONS, &DIRECTIONS, &DIRECTIONSKey0, 0);
    DIRECTIONS.recordID = AssignRecID(rcode2, DIRECTIONS.recordID);
    opCode = B_INSERT;
  }
  else
  {
    DIRECTIONSKey0.recordID = *m_pUpdateRecordID;
    btrieve(B_GETEQUAL, TMS_DIRECTIONS, &DIRECTIONS, &DIRECTIONSKey0, 0);
    opCode = B_UPDATE;
  }
//
//  Set up DIRECTIONS
//
//  Comment code
//
  if(pButtonNA->GetCheck())
    BUSTYPES.COMMENTSrecordID = NO_RECORD;
  else
  {
    nI = (int)pComboBoxCOMMENTCODE->GetCurSel();
    BUSTYPES.COMMENTSrecordID = (nI == CB_ERR ? NO_RECORD : pComboBoxCOMMENTCODE->GetItemData(nI));
  }
//
//  Abbreviated name
//  
  pEditABBRNAME->GetWindowText(s);
  PutDatabaseString(DIRECTIONS.abbrName, s, DIRECTIONS_ABBRNAME_LENGTH);
//
//  Long name
//
  pEditLONGNAME->GetWindowText(s);
  PutDatabaseString(DIRECTIONS.longName, s, DIRECTIONS_LONGNAME_LENGTH);
//
//  Reserved and flags
//
  DIRECTIONS.flags = 0L;
  memset(DIRECTIONS.reserved, 0x00, DIRECTIONS_RESERVED_LENGTH);
//
//  Insert / update
//
  btrieve(opCode, TMS_DIRECTIONS, &DIRECTIONS, &DIRECTIONSKey0, 0);
  *m_pUpdateRecordID = DIRECTIONS.recordID;
	
	CDialog::OnOK();
}

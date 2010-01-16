//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
// AddDivision.cpp : implementation file
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
// CAddDivision dialog


CAddDivision::CAddDivision(CWnd* pParent /*=NULL*/, long *pUpdateRecordID)
	: CDialog(CAddDivision::IDD, pParent)
{
  m_pUpdateRecordID = pUpdateRecordID;
  m_bInit = TRUE;

	//{{AFX_DATA_INIT(CAddDivision)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CAddDivision::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAddDivision)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CAddDivision, CDialog)
	//{{AFX_MSG_MAP(CAddDivision)
	ON_BN_CLICKED(ADDDIVISION_CC, OnCc)
	ON_CBN_SELENDOK(ADDDIVISION_COMMENTCODE, OnSelendokCommentcode)
	ON_BN_CLICKED(ADDDIVISION_NA, OnNa)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDHELP, OnHelp)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAddDivision message handlers

void CAddDivision::OnCc() 
{
  pComboBoxCOMMENTCODE->SetCurSel(0);
}

void CAddDivision::OnSelendokCommentcode() 
{
  if(pButtonNA->GetCheck())
  {
    pButtonNA->SetCheck(FALSE);
    pButtonCC->SetCheck(TRUE);
  }
}

void CAddDivision::OnNa() 
{
  pComboBoxCOMMENTCODE->SetCurSel(-1);
}

BOOL CAddDivision::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
  CString s;
  BOOL    bFound;
  int     nI;
  int     numComments;
//
//  Set up the pointers to the controls
//
  pEditNUMBER = (CEdit *)GetDlgItem(ADDDIVISION_NUMBER);
  pEditNAME = (CEdit *)GetDlgItem(ADDDIVISION_NAME);
  pButtonNA = (CButton *)GetDlgItem(ADDDIVISION_NA);
  pButtonCC = (CButton *)GetDlgItem(ADDDIVISION_CC);
  pComboBoxCOMMENTCODE = (CComboBox *)GetDlgItem(ADDDIVISION_COMMENTCODE);
//
//  Comment codes combo box
//
  numComments = SetUpCommentList(m_hWnd, ADDDIVISION_COMMENTCODE);
  if(numComments == 0)
  {
    pButtonCC->EnableWindow(FALSE);
    pComboBoxCOMMENTCODE->EnableWindow(FALSE);
  }
//
//  Are we updating?
//
  if(*m_pUpdateRecordID == NO_RECORD)
  {
    pButtonNA->SetCheck(TRUE);
    pEditNUMBER->SetWindowText("");
  }
//
//  Get the record
//
   else
  {
    DIVISIONSKey0.recordID = *m_pUpdateRecordID;
    btrieve(B_GETEQUAL, TMS_DIVISIONS, &DIVISIONS, &DIVISIONSKey0, 0);
//
//  Number
//
    s.Format("%ld", DIVISIONS.number);
    pEditNUMBER->SetWindowText(s);
//
//  Name
//
    s = GetDatabaseString(DIVISIONS.name, DIVISIONS_NAME_LENGTH);
    pEditNAME->SetWindowText(s);
//
//  Comment
//
    if(DIVISIONS.COMMENTSrecordID == NO_RECORD)
      pButtonNA->SetCheck(TRUE);
    else
    {
      for(bFound = FALSE, nI = 0; nI < numComments; nI++)
      {
        if((long)pComboBoxCOMMENTCODE->GetItemData(nI) == DIVISIONS.COMMENTSrecordID)
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

void CAddDivision::OnClose() 
{
  OnCancel();
}

void CAddDivision::OnCancel() 
{
	CDialog::OnCancel();
}

void CAddDivision::OnHelp() 
{
  ::WinHelp(m_hWnd, szarHelpFile, HELP_CONTEXT, The_Divisions_Table);
}

void CAddDivision::OnOK() 
{
  CString s;
  int     opCode;
  int     nI;
  int     rcode2;
//
//  Number cant be blank or zero
//
  pEditNUMBER->GetWindowText(s);
  if(s.IsEmpty() || s == "0")
  {
    TMSError(m_hWnd, MB_ICONSTOP, ERROR_077, pEditNUMBER->m_hWnd);
    return;
  }
//
//  Name
//
  pEditNAME->GetWindowText(s);
  if(s.IsEmpty())
  {
    TMSError(m_hWnd, MB_ICONSTOP, ERROR_078, pEditNAME->m_hWnd);
    return;
  }
//
//  Comment code
//
  if(pButtonNA->GetCheck())
    DIVISIONS.COMMENTSrecordID = NO_RECORD;
  else
  {
    nI = (int)pComboBoxCOMMENTCODE->GetCurSel();
    DIVISIONS.COMMENTSrecordID = (nI == CB_ERR ? NO_RECORD : pComboBoxCOMMENTCODE->GetItemData(nI));
  }
//
//  Insert / update?
//
  if(*m_pUpdateRecordID == NO_RECORD)
  {
    rcode2 = btrieve(B_GETLAST, TMS_DIVISIONS, &DIVISIONS, &DIVISIONSKey0, 0);
    DIVISIONS.recordID = AssignRecID(rcode2, DIVISIONS.recordID);
    opCode = B_INSERT;
  }
  else
  {
    DIVISIONSKey0.recordID = *m_pUpdateRecordID;
    btrieve(B_GETEQUAL, TMS_DIVISIONS, &DIVISIONS, &DIVISIONSKey0, 0);
    opCode = B_UPDATE;
  }
//
//  Set up the balance of DIVISIONS
//
//  Comment code
//
  if(pButtonNA->GetCheck())
    DIVISIONS.COMMENTSrecordID = NO_RECORD;
  else
  {
    nI = (int)pComboBoxCOMMENTCODE->GetCurSel();
    DIVISIONS.COMMENTSrecordID = (nI == CB_ERR ? NO_RECORD : pComboBoxCOMMENTCODE->GetItemData(nI));
  }
//
//  Number
//
  pEditNUMBER->GetWindowText(s);
  DIVISIONS.number = atol(s);
//
//  Name
//
  pEditNAME->GetWindowText(s);
  PutDatabaseString(DIVISIONS.name, s, DIVISIONS_NAME_LENGTH);
//
//  Flags and reserved
//
//
//  Flags
//
  DIVISIONS.flags = 0;
  memset(DIVISIONS.reserved, 0x00, DIVISIONS_RESERVED_LENGTH);
//
//  Insert / update and leave
//
  btrieve(opCode, TMS_DIVISIONS, &DIVISIONS, &DIVISIONSKey0, 0);
  *m_pUpdateRecordID = DIVISIONS.recordID;
 
	CDialog::OnOK();
}

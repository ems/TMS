//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
// AddBus.cpp : implementation file
//

#include "stdafx.h"
extern "C"{
#include "TMSHeader.h"
}
#include "tms.h"
#include "AVLInterface.h"
#include "AddDialogs.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CAddBus dialog


CAddBus::CAddBus(CWnd* pParent  /*=NULL*/, long* pUpdateRecordID)
	: CDialog(CAddBus::IDD, pParent)
{
  m_pUpdateRecordID = pUpdateRecordID;

	//{{AFX_DATA_INIT(CAddBus)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CAddBus::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAddBus)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CAddBus, CDialog)
	//{{AFX_MSG_MAP(CAddBus)
	ON_CBN_SELENDOK(ADDBUS_COMMENTCODE, OnSelendokCommentcode)
	ON_BN_CLICKED(ADDBUS_CC, OnCc)
	ON_BN_CLICKED(ADDBUS_NA, OnNa)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDHELP, OnHelp)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAddBus message handlers

BOOL CAddBus::OnInitDialog() 
{
  BOOL   bFound;
  int    numComments;
  int    numBustypes;
  int    nI;

  CString s;
  CString fmt;

	CDialog::OnInitDialog();
	
//
//  Set up the pointers to the controls
//
  pComboBoxBUSTYPE = (CComboBox *)GetDlgItem(ADDBUS_BUSTYPE);
  pEditNUMBER = (CEdit *)GetDlgItem(ADDBUS_NUMBER);
  pButtonRETIRED = (CButton *)GetDlgItem(ADDBUS_RETIRED);
  pButtonNA = (CButton *)GetDlgItem(ADDBUS_NA);
  pButtonCC = (CButton *)GetDlgItem(ADDBUS_CC);
  pComboBoxCOMMENTCODE = (CComboBox *)GetDlgItem(ADDBUS_COMMENTCODE);
//
//  Bustypes combo box
//
  numBustypes = SetUpBustypeList(m_hWnd, ADDBUS_BUSTYPE);
  if(numBustypes == 0)
  {
    TMSError(m_hWnd, MB_ICONSTOP, ERROR_130, (HANDLE)NULL);
    OnCancel();

    return TRUE;
  }
//
//  Comment codes combo box
//
  numComments = SetUpCommentList(m_hWnd, ADDBUS_COMMENTCODE);
  if(numComments == 0)
  {
    pButtonCC->EnableWindow(FALSE);
    pComboBoxCOMMENTCODE->EnableWindow(FALSE);
  }
//
//  Are we updating or not?
//
  if(*m_pUpdateRecordID == NO_RECORD)
  {
    pButtonNA->SetCheck(TRUE);
    pEditNUMBER->SetWindowText("");
  }
//
//  Yes - Get the record
//
  else
  {
    BUSESKey0.recordID = *m_pUpdateRecordID;
    btrieve(B_GETEQUAL, TMS_BUSES, &BUSES, &BUSESKey0, 0);
//
//  Bustype
//
    for(bFound = FALSE, nI = 0; nI < numBustypes; nI++)
    {
      if((long)pComboBoxBUSTYPE->GetItemData(nI) == BUSES.BUSTYPESrecordID)
      {
        pComboBoxBUSTYPE->SetCurSel(nI);
        bFound = TRUE;
        break;
      }
    }
//
//  Number
//
    s = GetDatabaseString(BUSES.number, BUSES_NUMBER_LENGTH);
    pEditNUMBER->SetWindowText(s);
//
//  Flags
//
    if(BUSES.flags & BUSES_FLAG_RETIRED)
    {
      pButtonRETIRED->SetCheck(TRUE);
    }
//
//  Comment
//
    if(BUSES.COMMENTSrecordID == NO_RECORD)
    {
      pButtonNA->SetCheck(TRUE);
    }
    else
    {
      for(bFound = FALSE, nI = 0; nI < numComments; nI++)
      {
        if((long)pComboBoxCOMMENTCODE->GetItemData(nI) == BUSES.COMMENTSrecordID)
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
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}


void CAddBus::OnSelendokCommentcode() 
{
  if(pButtonNA->GetCheck())
  {
    pButtonNA->SetCheck(FALSE);
    pButtonCC->SetCheck(TRUE);
  }
}

void CAddBus::OnCc() 
{
  pComboBoxCOMMENTCODE->SetCurSel(0);
}

void CAddBus::OnNa() 
{
  pComboBoxCOMMENTCODE->SetCurSel(-1);
}

void CAddBus::OnClose() 
{
  OnCancel();
}

void CAddBus::OnCancel() 
{
	CDialog::OnCancel();
}

void CAddBus::OnHelp() 
{
  ::WinHelp(m_hWnd, szarHelpFile, HELP_CONTEXT, The_Vehicle_Types_Table);
}

void CAddBus::OnOK() 
{
  CString s;
  int     rcode2;
  int     opCode;
  int     nI;
//
//  The number can't be blank
//
//  Number
//
  pEditNUMBER->GetWindowText(s);
  if(s.IsEmpty())
  {
    TMSError(m_hWnd, MB_ICONSTOP, ERROR_302, pEditNUMBER->m_hWnd);
    return;
  }
//
//  A Bustype must be selected
//
  nI = (int)pComboBoxBUSTYPE->GetCurSel();
  if(nI == CB_ERR)
  {
    TMSError(m_hWnd, MB_ICONSTOP, ERROR_303, pComboBoxBUSTYPE->m_hWnd);
    return;
  }
//
//  Insert / update?
//
  if(*m_pUpdateRecordID == NO_RECORD)
  {
    rcode2 = btrieve(B_GETLAST, TMS_BUSES, &BUSES, &BUSESKey0, 0);
    BUSES.recordID = AssignRecID(rcode2, BUSES.recordID);
    opCode = B_INSERT;
  }
  else
  {
    BUSESKey0.recordID = *m_pUpdateRecordID;
    rcode2 = btrieve(B_GETEQUAL, TMS_BUSES, &BUSES, &BUSESKey0, 0);
    if(rcode2 != 0)
    {
      s.LoadString(ERROR_338);
      sprintf(tempString, s, rcode2);
      MessageBeep(MB_ICONSTOP);
      MessageBox(tempString, TMS, MB_OK | MB_ICONSTOP);
    }
    opCode = B_UPDATE;
  }
//
//  Set up the balance of BUSES
//
  nI = pComboBoxBUSTYPE->GetCurSel();
  if(nI < 0)
  {
    BUSES.BUSTYPESrecordID = NO_RECORD;
  }
  else
  {
    BUSES.BUSTYPESrecordID = pComboBoxBUSTYPE->GetItemData(nI);
  }
//
//  Number
//
  pEditNUMBER->GetWindowText(s);
  PutDatabaseString(BUSES.number, s, BUSES_NUMBER_LENGTH);
//
//  Retired?
//
  BUSES.flags = (pButtonRETIRED->GetCheck() ? BUSES_FLAG_RETIRED : 0);
//
//  Comment code
//
  if(pButtonNA->GetCheck())
  {
    BUSES.COMMENTSrecordID = NO_RECORD;
  }
  else
  {
    nI = (int)pComboBoxCOMMENTCODE->GetCurSel();
    BUSES.COMMENTSrecordID = (nI == CB_ERR ? NO_RECORD : pComboBoxCOMMENTCODE->GetItemData(nI));
  }
//
//  Reserved
//
  memset(BUSES.reserved, 0x00, BUSES_RESERVED_LENGTH);
//
//  Insert / update
//
  rcode2 = btrieve(opCode, TMS_BUSES, &BUSES, &BUSESKey0, 0);
  if(rcode2 != 0)
  {
    s.LoadString(ERROR_337);
    sprintf(tempString, s, rcode2);
    MessageBeep(MB_ICONSTOP);
    MessageBox(tempString, TMS, MB_OK | MB_ICONSTOP);
  }
  *m_pUpdateRecordID = BUSES.recordID;
//
//  Send the data out in real time
//
  if(m_bUseStrategicMapping && opCode == B_INSERT)
  {
    m_AddBus(BUSES.recordID);
  }
	
	CDialog::OnOK();
}

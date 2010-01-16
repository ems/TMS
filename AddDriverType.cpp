//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
// AddDriverType.cpp : implementation file
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
// CAddDriverType dialog


CAddDriverType::CAddDriverType(CWnd* pParent /*=NULL*/, long *pUpdateRecordID)
	: CDialog(CAddDriverType::IDD, pParent)
{
  m_pUpdateRecordID = pUpdateRecordID;
  m_bInit = TRUE;

	//{{AFX_DATA_INIT(CAddDriverType)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CAddDriverType::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAddDriverType)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CAddDriverType, CDialog)
	//{{AFX_MSG_MAP(CAddDriverType)
	ON_WM_CLOSE()
	ON_BN_CLICKED(ADDDRIVERTYPE_CC, OnCc)
	ON_BN_CLICKED(ADDDRIVERTYPE_NA, OnNa)
	ON_CBN_SELENDOK(ADDDRIVERTYPE_COMMENTCODE, OnSelendokCommentcode)
	ON_BN_CLICKED(IDHELP, OnHelp)
	ON_BN_CLICKED(ADDDRIVERTYPE_REGULARBOX, OnRegularbox)
	ON_BN_CLICKED(ADDDRIVERTYPE_OVERTIMEBOX, OnOvertimebox)
	ON_BN_CLICKED(ADDDRIVERTYPE_SPREADPREMIUMBOX, OnSpreadpremiumbox)
	ON_BN_CLICKED(ADDDRIVERTYPE_MAKEUPTIMEBOX, OnMakeuptimebox)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAddDriverType message handlers

BOOL CAddDriverType::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
  CString s;
  BOOL    bFound;
  int     nI;
  int     numComments;
//
//  Set up the pointers to the controls
//
  pEditNUMBER = (CEdit *)GetDlgItem(ADDDRIVERTYPE_NUMBER);
  pEditNAME = (CEdit *)GetDlgItem(ADDDRIVERTYPE_NAME);
  pButtonREGULARBOX = (CButton *)GetDlgItem(ADDDRIVERTYPE_REGULARBOX);
  pEditREGULAR = (CEdit *)GetDlgItem(ADDDRIVERTYPE_REGULAR);
  pButtonOVERTIMEBOX = (CButton *)GetDlgItem(ADDDRIVERTYPE_OVERTIMEBOX);
  pEditOVERTIME = (CEdit *)GetDlgItem(ADDDRIVERTYPE_OVERTIME);
  pButtonSPREADPREMIUMBOX = (CButton *)GetDlgItem(ADDDRIVERTYPE_SPREADPREMIUMBOX);
  pEditSPREADPREMIUM = (CEdit *)GetDlgItem(ADDDRIVERTYPE_SPREADPREMIUM);
  pButtonMAKEUPTIMEBOX = (CButton *)GetDlgItem(ADDDRIVERTYPE_MAKEUPTIMEBOX);
  pEditMAKEUPTIME = (CEdit *)GetDlgItem(ADDDRIVERTYPE_MAKEUPTIME);
  pButtonNA = (CButton *)GetDlgItem(ADDDRIVERTYPE_NA);
  pButtonCC = (CButton *)GetDlgItem(ADDDRIVERTYPE_CC);
  pComboBoxCOMMENTCODE = (CComboBox *)GetDlgItem(ADDDRIVERTYPE_COMMENTCODE);
//
//  Comment codes combo box
//
  numComments = SetUpCommentList(m_hWnd, ADDDRIVERTYPE_COMMENTCODE);
  if(numComments == 0)
  {
    pButtonCC->EnableWindow(FALSE);
    pComboBoxCOMMENTCODE->EnableWindow(FALSE);
  }
//
//  Disable the payroll code edit controls
//
   pEditREGULAR->EnableWindow(FALSE);
   pEditOVERTIME->EnableWindow(FALSE);
   pEditSPREADPREMIUM->EnableWindow(FALSE);
   pEditMAKEUPTIME->EnableWindow(FALSE);
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
    DRIVERTYPESKey0.recordID = *m_pUpdateRecordID;
    btrieve(B_GETEQUAL, TMS_DRIVERTYPES, &DRIVERTYPES, &DRIVERTYPESKey0, 0);
//
//  Number
//
    s.Format("%ld", DRIVERTYPES.number);
    pEditNUMBER->SetWindowText(s);
//
//  Name
//
    s = GetDatabaseString(DRIVERTYPES.name, DRIVERTYPES_NAME_LENGTH);
    pEditNAME->SetWindowText(s);
//
//  Payroll codes
//
//  Regular
//
    if(DRIVERTYPES.flags & DRIVERTYPES_FLAG_REGULAR)
    {
      strncpy(tempString, DRIVERTYPES.payrollCodes[DRIVERTYPES_PAYROLLCODE_REGULAR], DRIVERS_PAYROLLCODE_LENGTH);
      trim(tempString, DRIVERS_PAYROLLCODE_LENGTH);
      pButtonREGULARBOX->SetCheck(TRUE);
      pEditREGULAR->EnableWindow(TRUE);
      pEditREGULAR->SetWindowText(tempString);
    }
//
//  Overtime
//
    if(DRIVERTYPES.flags & DRIVERTYPES_FLAG_OVERTIME)
    {
      strncpy(tempString, DRIVERTYPES.payrollCodes[DRIVERTYPES_PAYROLLCODE_OVERTIME], DRIVERS_PAYROLLCODE_LENGTH);
      trim(tempString, DRIVERS_PAYROLLCODE_LENGTH);
      pButtonOVERTIMEBOX->SetCheck(TRUE);
      pEditOVERTIME->EnableWindow(TRUE);
      pEditOVERTIME->SetWindowText(tempString);
    }
//
//  Spread premium
//
    if(DRIVERTYPES.flags & DRIVERTYPES_FLAG_SPREADPREMIUM)
    {
      strncpy(tempString, DRIVERTYPES.payrollCodes[DRIVERTYPES_PAYROLLCODE_SPREADPREMIUM], DRIVERS_PAYROLLCODE_LENGTH);
      trim(tempString, DRIVERS_PAYROLLCODE_LENGTH);
      pButtonSPREADPREMIUMBOX->SetCheck(TRUE);
      pEditSPREADPREMIUM->EnableWindow(TRUE);
      pEditSPREADPREMIUM->SetWindowText(tempString);
    }
//
// Makeup time
//
    if(DRIVERTYPES.flags & DRIVERTYPES_FLAG_MAKEUPTIME)
    {
      strncpy(tempString, DRIVERTYPES.payrollCodes[DRIVERTYPES_PAYROLLCODE_MAKEUPTIME], DRIVERS_PAYROLLCODE_LENGTH);
      trim(tempString, DRIVERS_PAYROLLCODE_LENGTH);
      pButtonMAKEUPTIMEBOX->SetCheck(TRUE);
      pEditMAKEUPTIME->EnableWindow(TRUE);
      pEditMAKEUPTIME->SetWindowText(tempString);
    }
//
//  Comment
//
    if(DRIVERTYPES.COMMENTSrecordID == NO_RECORD)
    {
      pButtonNA->SetCheck(TRUE);
    }
    else
    {
      for(bFound = FALSE, nI = 0; nI < numComments; nI++)
      {
        if((long)pComboBoxCOMMENTCODE->GetItemData(nI) == DRIVERTYPES.COMMENTSrecordID)
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

void CAddDriverType::OnClose() 
{
  OnCancel();
}

void CAddDriverType::OnCc() 
{
  pComboBoxCOMMENTCODE->SetCurSel(0);
}

void CAddDriverType::OnNa() 
{
  pComboBoxCOMMENTCODE->SetCurSel(-1);
}

void CAddDriverType::OnSelendokCommentcode() 
{
  if(pButtonNA->GetCheck())
  {
    pButtonNA->SetCheck(FALSE);
    pButtonCC->SetCheck(TRUE);
  }
}

void CAddDriverType::OnCancel() 
{
	CDialog::OnCancel();
}

void CAddDriverType::OnHelp() 
{
  ::WinHelp(m_hWnd, szarHelpFile, HELP_CONTEXT, The_Driver_Types_Table);
}


void CAddDriverType::OnRegularbox() 
{
  pEditREGULAR->EnableWindow(pButtonREGULARBOX->GetCheck());
}

void CAddDriverType::OnOvertimebox() 
{
  pEditOVERTIME->EnableWindow(pButtonOVERTIMEBOX->GetCheck());
}

void CAddDriverType::OnSpreadpremiumbox() 
{
  pEditSPREADPREMIUM->EnableWindow(pButtonSPREADPREMIUMBOX->GetCheck());
}

void CAddDriverType::OnMakeuptimebox() 
{
  pEditMAKEUPTIME->EnableWindow(pButtonMAKEUPTIMEBOX->GetCheck());
}

void CAddDriverType::OnOK() 
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
    DRIVERTYPES.COMMENTSrecordID = NO_RECORD;
  else
  {
    nI = (int)pComboBoxCOMMENTCODE->GetCurSel();
    DRIVERTYPES.COMMENTSrecordID = (nI == CB_ERR ? NO_RECORD : pComboBoxCOMMENTCODE->GetItemData(nI));
  }
//
//  Insert / update?
//
  if(*m_pUpdateRecordID == NO_RECORD)
  {
    rcode2 = btrieve(B_GETLAST, TMS_DRIVERTYPES, &DRIVERTYPES, &DRIVERTYPESKey0, 0);
    DRIVERTYPES.recordID = AssignRecID(rcode2, DRIVERTYPES.recordID);
    opCode = B_INSERT;
  }
  else
  {
    DRIVERTYPESKey0.recordID = *m_pUpdateRecordID;
    btrieve(B_GETEQUAL, TMS_DRIVERTYPES, &DRIVERTYPES, &DRIVERTYPESKey0, 0);
    opCode = B_UPDATE;
  }
//
//  Set up the balance of DRIVERTYPES
//
//  Comment code
//
  if(pButtonNA->GetCheck())
    DRIVERTYPES.COMMENTSrecordID = NO_RECORD;
  else
  {
    nI = (int)pComboBoxCOMMENTCODE->GetCurSel();
    DRIVERTYPES.COMMENTSrecordID = (nI == CB_ERR ? NO_RECORD : pComboBoxCOMMENTCODE->GetItemData(nI));
  }
//
//  Number
//
  pEditNUMBER->GetWindowText(s);
  DRIVERTYPES.number = atol(s);
//
//  Name
//
  pEditNAME->GetWindowText(s);
  PutDatabaseString(DRIVERTYPES.name, s, DRIVERTYPES_NAME_LENGTH);
//
//  Flags and Payroll Codes
//
  DRIVERTYPES.flags = 0;
  memset(DRIVERTYPES.payrollCodes, 0x00, DRIVERS_PAYROLLCODE_LENGTH * DRIVERS_NUMPAYROLLCODES);
//
//  Regular time
//
  if(pButtonREGULARBOX->GetCheck())
  {
    DRIVERTYPES.flags |= DRIVERTYPES_FLAG_REGULAR;
    pEditREGULAR->GetWindowText(tempString, TEMPSTRING_LENGTH);
    pad(tempString, DRIVERS_PAYROLLCODE_LENGTH);
    strncpy(DRIVERTYPES.payrollCodes[DRIVERTYPES_PAYROLLCODE_REGULAR], tempString, DRIVERS_PAYROLLCODE_LENGTH);
  }
//
//  Overtime
//
  if(pButtonOVERTIMEBOX->GetCheck())
  {
    DRIVERTYPES.flags |= DRIVERTYPES_FLAG_OVERTIME;
    pEditOVERTIME->GetWindowText(tempString, TEMPSTRING_LENGTH);
    pad(tempString, DRIVERS_PAYROLLCODE_LENGTH);
    strncpy(DRIVERTYPES.payrollCodes[DRIVERTYPES_PAYROLLCODE_OVERTIME], tempString, DRIVERS_PAYROLLCODE_LENGTH);
  }
//
//  Spread premium
//
  if(pButtonSPREADPREMIUMBOX->GetCheck())
  {
    DRIVERTYPES.flags |= DRIVERTYPES_FLAG_SPREADPREMIUM;
    pEditSPREADPREMIUM->GetWindowText(tempString, TEMPSTRING_LENGTH);
    pad(tempString, DRIVERS_PAYROLLCODE_LENGTH);
    strncpy(DRIVERTYPES.payrollCodes[DRIVERTYPES_PAYROLLCODE_SPREADPREMIUM], tempString, DRIVERS_PAYROLLCODE_LENGTH);
  }
//
//  Makeup time
//
  if(pButtonMAKEUPTIMEBOX->GetCheck())
  {
    DRIVERTYPES.flags |= DRIVERTYPES_FLAG_MAKEUPTIME;
    pEditMAKEUPTIME->GetWindowText(tempString, TEMPSTRING_LENGTH);
    pad(tempString, DRIVERS_PAYROLLCODE_LENGTH);
    strncpy(DRIVERTYPES.payrollCodes[DRIVERTYPES_PAYROLLCODE_MAKEUPTIME], tempString, DRIVERS_PAYROLLCODE_LENGTH);
  }
//
//  Insert / update and leave
//
  btrieve(opCode, TMS_DRIVERTYPES, &DRIVERTYPES, &DRIVERTYPESKey0, 0);
  *m_pUpdateRecordID = DRIVERTYPES.recordID;
	
	CDialog::OnOK();
}

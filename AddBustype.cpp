//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
// AddBustype.cpp : implementation file
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
// CAddBustype dialog


CAddBustype::CAddBustype(CWnd* pParent /*=NULL*/, long* pUpdateRecordID)
	: CDialog(CAddBustype::IDD, pParent)
{
  m_pUpdateRecordID = pUpdateRecordID;
  m_bInit = TRUE;

	//{{AFX_DATA_INIT(CAddBustype)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CAddBustype::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAddBustype)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CAddBustype, CDialog)
	//{{AFX_MSG_MAP(CAddBustype)
	ON_CBN_SELENDOK(ADDBUSTYPE_COMMENTCODE, OnSelendokCommentcode)
	ON_BN_CLICKED(ADDBUSTYPE_CC, OnCc)
	ON_BN_CLICKED(ADDBUSTYPE_NA, OnNa)
	ON_EN_CHANGE(ADDBUSTYPE_SEAT, OnChangeSeat)
	ON_EN_CHANGE(ADDBUSTYPE_STAND, OnChangeStand)
	ON_BN_CLICKED(IDHELP, OnHelp)
	ON_WM_CLOSE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAddBustype message handlers

BOOL CAddBustype::OnInitDialog() 
{
  BOOL   bFound;
  int    numComments;
  int    nI;

  CString s;
  CString fmt;

	CDialog::OnInitDialog();
//
//  Set up the pointers to the controls
//
  pEditNUMBER = (CEdit *)GetDlgItem(ADDBUSTYPE_NUMBER);
  pEditFLEETNAME = (CEdit *)GetDlgItem(ADDBUSTYPE_FLEETNAME);
  pEditNAME = (CEdit *)GetDlgItem(ADDBUSTYPE_NAME);
  pButtonACCESSIBLE = (CButton *)GetDlgItem(ADDBUSTYPE_ACCESSIBLE);
  pButtonCARRIER = (CButton *)GetDlgItem(ADDBUSTYPE_CARRIER);
  pButtonSURFACE = (CButton *)GetDlgItem(ADDBUSTYPE_SURFACE);
  pButtonRAIL = (CButton *)GetDlgItem(ADDBUSTYPE_RAIL);
  pButtonCAPACITY = (CButton *)GetDlgItem(ADDBUSTYPE_CAPACITY);
  pEditSEAT = (CEdit *)GetDlgItem(ADDBUSTYPE_SEAT);
  pEditSTAND = (CEdit *)GetDlgItem(ADDBUSTYPE_STAND);
  pButtonNA = (CButton *)GetDlgItem(ADDBUSTYPE_NA);
  pButtonCC = (CButton *)GetDlgItem(ADDBUSTYPE_CC);
  pComboBoxCOMMENTCODE = (CComboBox *)GetDlgItem(ADDBUSTYPE_COMMENTCODE);
//
//  Comment codes combo box
//
  numComments = SetUpCommentList(m_hWnd, ADDBUSTYPE_COMMENTCODE);
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
    BUSTYPESKey0.recordID = *m_pUpdateRecordID;
    btrieve(B_GETEQUAL, TMS_BUSTYPES, &BUSTYPES, &BUSTYPESKey0, 0);
//
//  Number
//
    s.Format("%ld", BUSTYPES.number);
    pEditNUMBER->SetWindowText(s);
//
//  Name
//
    s = GetDatabaseString(BUSTYPES.name, BUSTYPES_NAME_LENGTH);
    pEditNAME->SetWindowText(s);
//
//  Fleet name
//
    s = GetDatabaseString(BUSTYPES.fleetName, BUSTYPES_FLEETNAME_LENGTH);
    pEditFLEETNAME->SetWindowText(s);
//
//  Capacity
//
    fmt.LoadString(TEXT_240);
    s.Format(fmt, BUSTYPES.capacitySeated + BUSTYPES.capacityStanding);
    pButtonCAPACITY->SetWindowText(s);
//
//  Seated
//
    s.Format("%ld", BUSTYPES.capacitySeated);
    pEditSEAT->SetWindowText(s);
//
//  Standing
//
    s.Format("%ld", BUSTYPES.capacityStanding);
    pEditSTAND->SetWindowText(s);
//
//  Accessible
//
    if(BUSTYPES.flags & BUSTYPES_FLAG_ACCESSIBLE)
      pButtonACCESSIBLE->SetCheck(TRUE);
//
//  Carrier equipped
//
    if(BUSTYPES.flags & BUSTYPES_FLAG_CARRIER)
      pButtonCARRIER->SetCheck(TRUE);
//
//  Surface vehicle
//
    if(BUSTYPES.flags & BUSTYPES_FLAG_SURFACE)
      pButtonSURFACE->SetCheck(TRUE);
//
//  Rail vehicle
//
    if(BUSTYPES.flags & BUSTYPES_FLAG_RAIL)
      pButtonRAIL->SetCheck(TRUE);
//
//  Comment
//
    if(BUSTYPES.COMMENTSrecordID == NO_RECORD)
      pButtonNA->SetCheck(TRUE);
    else
    {
      for(bFound = FALSE, nI = 0; nI < numComments; nI++)
      {
        if((long)pComboBoxCOMMENTCODE->GetItemData(nI) == BUSTYPES.COMMENTSrecordID)
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

void CAddBustype::OnSelendokCommentcode() 
{
  if(pButtonNA->GetCheck())
  {
    pButtonNA->SetCheck(FALSE);
    pButtonCC->SetCheck(TRUE);
  }
}

void CAddBustype::OnCc() 
{
  pComboBoxCOMMENTCODE->SetCurSel(0);
}

void CAddBustype::OnNa() 
{
  pComboBoxCOMMENTCODE->SetCurSel(-1);
}

void CAddBustype::OnChangeSeat() 
{
  OnChangeStand();
}

void CAddBustype::OnChangeStand() 
{
  CString s;
  CString fmt;
  int     numberSeated;
  int     numberStanding;

  if(!m_bInit)  // We end up here during intialization
  {
    pEditSEAT->GetWindowText(s);
    numberSeated = atoi(s);
    pEditSTAND->GetWindowText(s);
    numberStanding = atoi(s);
    fmt.LoadString(TEXT_240);
    s.Format(fmt, (long)(numberSeated + numberStanding));
    pButtonCAPACITY->SetWindowText(s);
  }
}

void CAddBustype::OnOK() 
{
  CString s;
  int     rcode2;
  int     opCode;
  int     nI;
//
//  The number and name can't be blank
//
//  Number
//
  pEditNUMBER->GetWindowText(s);
  if(s.IsEmpty() || atol(s) == 0)
  {
    TMSError(m_hWnd, MB_ICONSTOP, ERROR_187, pEditNUMBER->m_hWnd);
    return;
  }
//
//  Name
//
  pEditNAME->GetWindowText(s);
  if(s.IsEmpty())
  {
    TMSError(m_hWnd, MB_ICONSTOP, ERROR_100, pEditNAME->m_hWnd);
    return;
  }
//
//  Insert / update?
//
  if(*m_pUpdateRecordID == NO_RECORD)
  {
    rcode2 = btrieve(B_GETLAST, TMS_BUSTYPES, &BUSTYPES, &BUSTYPESKey0, 0);
    BUSTYPES.recordID = AssignRecID(rcode2, BUSTYPES.recordID);
    opCode = B_INSERT;
  }
  else
  {
    BUSTYPESKey0.recordID = *m_pUpdateRecordID;
    btrieve(B_GETEQUAL, TMS_BUSTYPES, &BUSTYPES, &BUSTYPESKey0, 0);
    opCode = B_UPDATE;
  }
//
//  Set up the balance of BUSTYPES
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
//  Number
//
  pEditNUMBER->GetWindowText(s);
  BUSTYPES.number = atol(s);
//
//  Name
//
  pEditNAME->GetWindowText(s);
  PutDatabaseString(BUSTYPES.name, s, BUSTYPES_NAME_LENGTH);
//
//  Fleet name
//
  pEditFLEETNAME->GetWindowText(s);
  PutDatabaseString(BUSTYPES.fleetName, s, BUSTYPES_FLEETNAME_LENGTH);
//
//  Capacity seated
//
  pEditSEAT->GetWindowText(s);
  BUSTYPES.capacitySeated = atoi(s);
//
//  Capacity standing
//
  pEditSTAND->GetWindowText(s);
  BUSTYPES.capacityStanding = atoi(s);
//
//  Flags
//
  BUSTYPES.flags = 0;
  if(pButtonACCESSIBLE->GetCheck())
    BUSTYPES.flags |= BUSTYPES_FLAG_ACCESSIBLE;
  if(pButtonCARRIER->GetCheck())
    BUSTYPES.flags |= BUSTYPES_FLAG_CARRIER;
  if(pButtonSURFACE->GetCheck())
    BUSTYPES.flags |= BUSTYPES_FLAG_SURFACE;
  if(pButtonRAIL->GetCheck())
    BUSTYPES.flags |= BUSTYPES_FLAG_RAIL;
//
//  Reserved
//
  memset(BUSTYPES.reserved, 0x00, BUSTYPES_RESERVED_LENGTH);
//
//  Insert / update and leave
//
  btrieve(opCode, TMS_BUSTYPES, &BUSTYPES, &BUSTYPESKey0, 0);
  *m_pUpdateRecordID = BUSTYPES.recordID;
	
  CDialog::OnOK();
}

void CAddBustype::OnHelp() 
{
  ::WinHelp(m_hWnd, szarHelpFile, HELP_CONTEXT, The_Vehicle_Types_Table);
}

void CAddBustype::OnCancel() 
{
	CDialog::OnCancel();
}

void CAddBustype::OnClose() 
{
  OnCancel();
}


//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
// AddCrewOnly.cpp : implementation file
//

#include "stdafx.h"
extern "C"
{
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
// CAddCrewOnly dialog


CAddCrewOnly::CAddCrewOnly(CWnd* pParent /*=NULL*/, long* pUpdateRecordID, PDISPLAYINFO pDI)
	: CDialog(CAddCrewOnly::IDD, pParent)
{
  m_pUpdateRecordID = pUpdateRecordID;
  m_pDI = pDI;

	//{{AFX_DATA_INIT(CAddCrewOnly)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CAddCrewOnly::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAddCrewOnly)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CAddCrewOnly, CDialog)
	//{{AFX_MSG_MAP(CAddCrewOnly)
	ON_EN_CHANGE(CREWONLY_PIECENUMBER, OnChangePiecenumber)
	ON_BN_CLICKED(CREWONLY_NA, OnNa)
	ON_BN_CLICKED(CREWONLY_CC, OnCc)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDHELP, OnHelp)
	ON_CBN_SELENDOK(CREWONLY_COMMENTCODE, OnSelendokCommentcode)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAddCrewOnly message handlers

BOOL CAddCrewOnly::OnInitDialog() 
{
  CString s;
  BOOL bFound;
  int serviceIndex;
  int rcode2;
  int numComments;
  int nI, nJ, nK;
 
	CDialog::OnInitDialog();

//
//  Set up the handles to the controls
//	
  pEditPIECENUMBER = (CEdit *)GetDlgItem(CREWONLY_PIECENUMBER);
  pEditRUNNUMBER = (CEdit *)GetDlgItem(CREWONLY_RUNNUMBER);
  pComboBoxRUNTYPE = (CComboBox *)GetDlgItem(CREWONLY_RUNTYPE);
  pComboBoxSTARTNODE = (CComboBox *)GetDlgItem(CREWONLY_STARTNODE);
  pEditSTARTTIME = (CEdit *)GetDlgItem(CREWONLY_STARTTIME);
  pComboBoxENDNODE = (CComboBox *)GetDlgItem(CREWONLY_ENDNODE);
  pEditENDTIME = (CEdit *)GetDlgItem(CREWONLY_ENDTIME);
  pButtonNA = (CButton *)GetDlgItem(CREWONLY_NA);
  pButtonCC = (CButton *)GetDlgItem(CREWONLY_CC);
  pComboBoxCOMMENTCODE = (CComboBox *)GetDlgItem(CREWONLY_COMMENTCODE);
	
//
//  Display the runtype list
//
  pComboBoxRUNTYPE->ResetContent();
  for(nI = 0; nI < NUMRUNTYPES; nI++)
  {
    for(nJ = 0; nJ < NUMRUNTYPESLOTS; nJ++)
    {
      if(RUNTYPE[nI][nJ].flags & RTFLAGS_INUSE &&
            RUNTYPE[nI][nJ].flags & RTFLAGS_CREWONLY)
      {
        nK = pComboBoxRUNTYPE->AddString(RUNTYPE[nI][nJ].localName);
        pComboBoxRUNTYPE->SetItemData(nK, MAKELPARAM(nI, nJ));
      }
    }
  }
//
//  There has to be at least one "Crew Only" runtype defined
//
  if(pComboBoxRUNTYPE->GetCount() == 0)
  {
    TMSError(m_hWnd, MB_ICONSTOP, ERROR_312, (HANDLE)NULL);
    OnCancel();
    return TRUE;
  }
//
//  Display the possible nodes
//
//  Set up the service index
//
  SERVICESKey0.recordID = m_pDI->fileInfo.serviceRecordID;
  btrieve(B_GETEQUAL, TMS_SERVICES, &SERVICES, &SERVICESKey0, 0);
  serviceIndex = SERVICES.number - 1;
  if(serviceIndex < 0 || serviceIndex > NODES_RELIEFLABELS_LENGTH - 1)
  {
    serviceIndex = 0;
  }
//
//  Cycle through the nodes
//
  NODESKey1.flags = 0;
  memset(&NODESKey1.abbrName, 0x00, NODES_ABBRNAME_LENGTH);
  rcode2 = btrieve(B_GETGREATER, TMS_NODES, &NODES, &NODESKey1, 1);
  while(rcode2 == 0 && NODES.flags <= NODES_FLAG_GARAGE)
  {
    if(!(NODES.flags & NODES_FLAG_STOP))
    {
      if((NODES.flags & NODES_FLAG_GARAGE) || (NODES.reliefLabels[serviceIndex] != ' ' &&
            NODES.reliefLabels[serviceIndex] != '-' && NODES.reliefLabels[serviceIndex] != 0))
      {
        strncpy(tempString, NODES.abbrName, NODES_ABBRNAME_LENGTH);
        trim(tempString, NODES_ABBRNAME_LENGTH);
        nI = pComboBoxSTARTNODE->AddString(tempString);
        pComboBoxSTARTNODE->SetItemData(nI, NODES.recordID);
        nI = pComboBoxENDNODE->AddString(tempString);
        pComboBoxENDNODE->SetItemData(nI, NODES.recordID);
      }
    }
    rcode2 = btrieve(B_GETNEXT, TMS_NODES, &NODES, &NODESKey1, 1);
  }

//
//  Comment codes combo box
//
  numComments = SetUpCommentList(m_hWnd, CREWONLY_COMMENTCODE);
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
  }
//
//  Yes - Get the record
//
  else
  {
    CREWONLYKey0.recordID = *m_pUpdateRecordID;
    btrieve(B_GETEQUAL, TMS_CREWONLY, &CREWONLY, &CREWONLYKey0, 0);
//
//  Comment
//
    if(CREWONLY.COMMENTSrecordID == NO_RECORD)
      pButtonNA->SetCheck(TRUE);
    else
    {
      for(bFound = FALSE, nI = 0; nI < numComments; nI++)
      {
        if((long)pComboBoxCOMMENTCODE->GetItemData(nI) == CREWONLY.COMMENTSrecordID)
        {
          pComboBoxCOMMENTCODE->SetCurSel(nI);
          bFound = TRUE;
          break;
        }
      }
      CButton* pb = (bFound ? pButtonCC : pButtonNA);
      pb->SetCheck(TRUE);
    }
//
//  Run number
//
    s.Format("%ld", CREWONLY.runNumber);
    pEditRUNNUMBER->SetWindowText(s);
//
//  Piece number
//
    s.Format("%ld", CREWONLY.pieceNumber);
    pEditPIECENUMBER->SetWindowText(s);
//
//  Runtype
//
    nJ = pComboBoxRUNTYPE->GetCount();
    for(nI = 0; nI < nJ; nI++)
    {
      if((long)pComboBoxRUNTYPE->GetItemData(nI) == CREWONLY.cutAsRuntype)
      {
        pComboBoxRUNTYPE->SetCurSel(nI);
        break;
      }
    }
//
//  Start node
//
    nJ = pComboBoxSTARTNODE->GetCount();
    for(nI = 0; nI < nJ; nI++)
    {
      if((long)pComboBoxSTARTNODE->GetItemData(nI) == CREWONLY.startNODESrecordID)
      {
        pComboBoxSTARTNODE->SetCurSel(nI);
        break;
      }
    }
//
//  Start time
//
    pEditSTARTTIME->SetWindowText(Tchar(CREWONLY.startTime));
//
//  End node
//
    nJ = pComboBoxENDNODE->GetCount();
    for(nI = 0; nI < nJ; nI++)
    {
      if((long)pComboBoxENDNODE->GetItemData(nI) == CREWONLY.endNODESrecordID)
      {
        pComboBoxENDNODE->SetCurSel(nI);
        break;
      }
    }
//
//  End time
//
    pEditENDTIME->SetWindowText(Tchar(CREWONLY.endTime));
  }

 	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CAddCrewOnly::OnChangePiecenumber() 
{
	// TODO: If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.
	
	// TODO: Add your control notification handler code here
	
}

void CAddCrewOnly::OnSelendokCommentcode() 
{
  if(pButtonNA->GetCheck())
  {
    pButtonNA->SetCheck(FALSE);
    pButtonCC->SetCheck(TRUE);
  }
}

void CAddCrewOnly::OnCc() 
{
  pComboBoxCOMMENTCODE->SetCurSel(0);
}

void CAddCrewOnly::OnNa() 
{
  pComboBoxCOMMENTCODE->SetCurSel(-1);
}


void CAddCrewOnly::OnClose() 
{
  OnCancel();
}

void CAddCrewOnly::OnOK() 
{
  CString s;
  int  rcode2;
  int  opCode;
  int  nI;
  long runNumber;
  long pieceNumber;
  long cutAsRuntype;
  long startNODESrecordID;
  long startTime;
  long endNODESrecordID;
  long endTime;

//
//  Nothing can be blank or zero
//
//  Run number
//
  pEditRUNNUMBER->GetWindowText(s);
  runNumber = atol(s);
  if(s.IsEmpty() || runNumber == 0)
  {
    TMSError(m_hWnd, MB_ICONSTOP, ERROR_305, pEditRUNNUMBER->m_hWnd);
    return;
  }
//
//  Piece number
//
  pEditPIECENUMBER->GetWindowText(s);  
  pieceNumber = atol(s);
  if(s.IsEmpty() || pieceNumber == 0)
  {
    TMSError(m_hWnd, MB_ICONSTOP, ERROR_306, pEditPIECENUMBER->m_hWnd);
    return;
  }
//
//  Runtype
//
  nI = pComboBoxRUNTYPE->GetCurSel();
  if(nI == CB_ERR)
  {
    TMSError(m_hWnd, MB_ICONSTOP, ERROR_307, pComboBoxRUNTYPE->m_hWnd);
    return;
  }
  cutAsRuntype = pComboBoxRUNTYPE->GetItemData(nI);
//
//  Start node
//
  nI = pComboBoxSTARTNODE->GetCurSel();
  if(nI == CB_ERR)
  {
    TMSError(m_hWnd, MB_ICONSTOP, ERROR_308, pComboBoxSTARTNODE->m_hWnd);
    return;
  }
  startNODESrecordID = pComboBoxSTARTNODE->GetItemData(nI);
//
//  Start time
//
  pEditSTARTTIME->GetWindowText(s);
  if(s.IsEmpty())
  {
    TMSError(m_hWnd, MB_ICONSTOP, ERROR_309, pEditSTARTTIME->m_hWnd);
    return;
  }
  strcpy(tempString, s);
  startTime = cTime(tempString);
//
//  End node
//
  nI = pComboBoxENDNODE->GetCurSel();
  if(nI == CB_ERR)
  {
    TMSError(m_hWnd, MB_ICONSTOP, ERROR_310, pComboBoxENDNODE->m_hWnd);
    return;
  }
  endNODESrecordID = pComboBoxENDNODE->GetItemData(nI);
//
//  Start time
//
  pEditENDTIME->GetWindowText(s);
  if(s.IsEmpty())
  {
    TMSError(m_hWnd, MB_ICONSTOP, ERROR_311, pEditENDTIME->m_hWnd);
    return;
  }
  strcpy(tempString, s);
  endTime = cTime(tempString);
//
//  Build the record
//
  memset(&CREWONLY, 0x00, sizeof(CREWONLYDef));
//
//  Insert / update?
//
  if(*m_pUpdateRecordID == NO_RECORD)
  {
    rcode2 = btrieve(B_GETLAST, TMS_CREWONLY, &CREWONLY, &CREWONLYKey0, 0);
    CREWONLY.recordID = AssignRecID(rcode2, CREWONLY.recordID);
    CREWONLY.SERVICESrecordID = m_ServiceRecordID;
    CREWONLY.DIVISIONSrecordID = m_DivisionRecordID;
    opCode = B_INSERT;
  }
  else
  {
    CREWONLYKey0.recordID = *m_pUpdateRecordID;
    btrieve(B_GETEQUAL, TMS_CREWONLY, &CREWONLY, &CREWONLYKey0, 0);
    opCode = B_UPDATE;
  }
//
//  Set up the balance of CREWONLY
//
//  Comment code
//
  if(pButtonNA->GetCheck())
    CREWONLY.COMMENTSrecordID = NO_RECORD;
  else
  {
    nI = (int)pComboBoxCOMMENTCODE->GetCurSel();
    CREWONLY.COMMENTSrecordID =
          (nI == CB_ERR ? NO_RECORD : pComboBoxCOMMENTCODE->GetItemData(nI));
  }
//
//  Run number
//
  CREWONLY.runNumber = runNumber;
//
//  Piece number
//
  CREWONLY.pieceNumber = pieceNumber;
//
//  Runtype
//
  CREWONLY.cutAsRuntype = cutAsRuntype;
//
//  Start node
//
  CREWONLY.startNODESrecordID = startNODESrecordID;
//
//  Start time
//
  CREWONLY.startTime = startTime;
//
//  End node
//
  CREWONLY.endNODESrecordID = endNODESrecordID;
//
//  End time
//
  CREWONLY.endTime = endTime;
//
//  Insert / update and leave
//
  btrieve(opCode, TMS_CREWONLY, &CREWONLY, &CREWONLYKey0, 0);
  *m_pUpdateRecordID = CREWONLY.recordID;
	
	CDialog::OnOK();
}

void CAddCrewOnly::OnCancel() 
{
	// TODO: Add extra cleanup here
	
	CDialog::OnCancel();
}

void CAddCrewOnly::OnHelp() 
{
	// TODO: Add your control notification handler code here
	
}

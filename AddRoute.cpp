// AddRoute.cpp : implementation file
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
// CAddRoute dialog


CAddRoute::CAddRoute(CWnd* pParent /*=NULL*/, long *pUpdateRecordID)
	: CDialog(CAddRoute::IDD, pParent)
{
  m_pUpdateRecordID = pUpdateRecordID;

	//{{AFX_DATA_INIT(CAddRoute)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CAddRoute::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAddRoute)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CAddRoute, CDialog)
	//{{AFX_MSG_MAP(CAddRoute)
	ON_WM_CLOSE()
	ON_BN_CLICKED(ADDROUTE_BIDIRECTIONAL, OnBidirectional)
	ON_BN_CLICKED(ADDROUTE_CC, OnCc)
	ON_BN_CLICKED(ADDROUTE_NA, OnNa)
	ON_CBN_SELENDOK(ADDROUTE_COMMENTCODE, OnSelendokCommentcode)
	ON_BN_CLICKED(ADDROUTE_CIRCULAR, OnCircular)
	ON_BN_CLICKED(ADDROUTE_IBBAYNA, OnIbbayna)
	ON_BN_CLICKED(ADDROUTE_IBBAY, OnIbbay)
	ON_CBN_SELENDOK(ADDROUTE_IBBAYLIST, OnSelendokIbbaylist)
	ON_BN_CLICKED(ADDROUTE_OBBAYNA, OnObbayna)
	ON_BN_CLICKED(ADDROUTE_OBBAY, OnObbay)
	ON_CBN_SELENDOK(ADDROUTE_OBBAYLIST, OnSelendokObbaylist)
	ON_BN_CLICKED(IDHELP, OnHelp)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAddRoute message handlers

BOOL CAddRoute::OnInitDialog() 
{
	CDialog::OnInitDialog();

  BOOL bFound;
  int  nI;
//
//  Set up the pointers to the controls
//
  pEditNUMBER = (CEdit *)GetDlgItem(ADDROUTE_NUMBER);
  pEditNAME = (CEdit *)GetDlgItem(ADDROUTE_NAME);
  pButtonBIDIRECTIONAL = (CButton *)GetDlgItem(ADDROUTE_BIDIRECTIONAL);
  pButtonCIRCULAR = (CButton *)GetDlgItem(ADDROUTE_CIRCULAR);
  pComboBoxOUTBOUND = (CComboBox *)GetDlgItem(ADDROUTE_OUTBOUND);
  pComboBoxINBOUND = (CComboBox *)GetDlgItem(ADDROUTE_INBOUND);
  pButtonSHUTTLE = (CButton *)GetDlgItem(ADDROUTE_SHUTTLE);
  pButtonSCHOOL = (CButton *)GetDlgItem(ADDROUTE_SCHOOL);
  pButtonRGRP = (CButton *)GetDlgItem(ADDROUTE_RGRP);
  pButtonOBBAYNA = (CButton *)GetDlgItem(ADDROUTE_OBBAYNA);
  pButtonOBBAY = (CButton *)GetDlgItem(ADDROUTE_OBBAY);
  pComboBoxOBBAYLIST = (CComboBox *)GetDlgItem(ADDROUTE_OBBAYLIST);
  pButtonIBGROUPBOX = (CButton *)GetDlgItem(ADDROUTE_IBGROUPBOX);
  pButtonIBBAYNA = (CButton *)GetDlgItem(ADDROUTE_IBBAYNA);
  pButtonIBBAY = (CButton *)GetDlgItem(ADDROUTE_IBBAY);
  pComboBoxIBBAYLIST = (CComboBox *)GetDlgItem(ADDROUTE_IBBAYLIST);
  pButtonNA = (CButton *)GetDlgItem(ADDROUTE_NA);
  pButtonCC = (CButton *)GetDlgItem(ADDROUTE_CC);
  pComboBoxCOMMENTCODE = (CComboBox *)GetDlgItem(ADDROUTE_COMMENTCODE);
//
//  Set up directions - Can't have none
//
  int numDirections = SetUpDirectionList(this->m_hWnd, ADDROUTE_OUTBOUND, ADDROUTE_INBOUND);

  if(numDirections == 0)
  {
    OnCancel();
    return TRUE;
  } 
//
//  Set up stops
//
  int numStops = 0;
  int rcode2;

  rcode2 = btrieve(B_GETFIRST, TMS_NODES, &NODES, &NODESKey2, 2);
  while(rcode2 == 0)
  {
    if(NODES.flags & NODES_FLAG_STOP)
    {
      strncpy(tempString, NODES.abbrName, NODES_ABBRNAME_LENGTH);
      trim(tempString, NODES_ABBRNAME_LENGTH);
      nI = pComboBoxOBBAYLIST->AddString(tempString);
      pComboBoxOBBAYLIST->SetItemData(nI, NODES.recordID);
      nI = pComboBoxIBBAYLIST->AddString(tempString);
      pComboBoxIBBAYLIST->SetItemData(nI, NODES.recordID);
      numStops++;
    }
    rcode2 = btrieve(B_GETNEXT, TMS_NODES, &NODES, &NODESKey2, 2);
  }
  if(numStops == 0)
  {
    pButtonOBBAY->EnableWindow(FALSE);
    pComboBoxOBBAYLIST->EnableWindow(FALSE);
    pButtonIBBAY->EnableWindow(FALSE);
    pComboBoxIBBAYLIST->EnableWindow(FALSE);
  }
//
//  Comment codes combo box
//
  int numComments = SetUpCommentList(this->m_hWnd, ADDROUTE_COMMENTCODE);

  if(numComments == 0)
  {
    pButtonCC->EnableWindow(FALSE);
    pComboBoxCOMMENTCODE->EnableWindow(FALSE);
  }
//
//  Are we updating?
//
//  No
//
  if(*m_pUpdateRecordID == NO_RECORD)
  {
    pButtonBIDIRECTIONAL->SetCheck(TRUE);
    pButtonSHUTTLE->SetCheck(FALSE);
    pButtonSCHOOL->SetCheck(FALSE);
    pButtonRGRP->SetCheck(FALSE);
    pButtonOBBAYNA->SetCheck(TRUE);
    pButtonIBBAYNA->SetCheck(TRUE);
    pButtonNA->SetCheck(TRUE);
  }
//
//  Yes
//
  else
  {
//
//  Get the data for the route
//
    ROUTESKey0.recordID = *m_pUpdateRecordID;
    btrieve(B_GETEQUAL, TMS_ROUTES, &ROUTES, &ROUTESKey0, 0);
//
//  Number
//
    strncpy(tempString, ROUTES.number, ROUTES_NUMBER_LENGTH);
    trim(tempString, ROUTES_NUMBER_LENGTH);
    pEditNUMBER->SetWindowText(tempString);
//
//  Name
//
    strncpy(tempString, ROUTES.name, ROUTES_NAME_LENGTH);
    trim(tempString, ROUTES_NAME_LENGTH);
    pEditNAME->SetWindowText(tempString);
//
//  Direction(s)
//
    for(nI = 0; nI < numDirections; nI++)
    {
      if((long)pComboBoxOUTBOUND->GetItemData(nI) == ROUTES.DIRECTIONSrecordID[0])
      {
        pComboBoxOUTBOUND->SetCurSel(nI);
        break;
      }
    }
    if(ROUTES.DIRECTIONSrecordID[1] == NO_RECORD)
    {
      pButtonCIRCULAR->SetCheck(TRUE);
      pComboBoxINBOUND->EnableWindow(FALSE);
      pButtonIBGROUPBOX->EnableWindow(FALSE);
      pButtonIBBAYNA->SetCheck(TRUE);
      pButtonIBBAYNA->EnableWindow(FALSE);
      pButtonIBBAY->EnableWindow(FALSE);
      pComboBoxIBBAYLIST->EnableWindow(FALSE);
    }
    else
    {
      pButtonBIDIRECTIONAL->SetCheck(TRUE);
      for(nI = 0; nI < numDirections; nI++)
      {
        if((long)pComboBoxINBOUND->GetItemData(nI) == ROUTES.DIRECTIONSrecordID[1])
        {
          pComboBoxINBOUND->SetCurSel(nI);
          break;
        }
      }
    }
//
//  Employee shuttle
//
    pButtonSHUTTLE->SetCheck(ROUTES.flags & ROUTES_FLAG_EMPSHUTTLE);
//
//  School route
//
    pButtonSCHOOL->SetCheck(ROUTES.flags & ROUTES_FLAG_SCHOOL);
//
//  RGRP
//
    pButtonRGRP->SetCheck(ROUTES.flags & ROUTES_FLAG_RGRP);
//
//  Outbound bay information
//
    if(ROUTES.OBBayNODESrecordID <= 0)  // Legacy - normally check for NO_RECORD
    {
      pButtonOBBAYNA->SetCheck(TRUE);
    }
    else
    {
      pButtonOBBAY->SetCheck(TRUE);
      for(nI = 0; nI < numStops; nI++)
      {
        if((short int)pComboBoxOBBAYLIST->GetItemData(nI) == ROUTES.OBBayNODESrecordID)
        {
          pComboBoxOBBAYLIST->SetCurSel(nI);
          break;
        }
      }
    }
//
//  Inbound bay information
//
    if(ROUTES.IBBayNODESrecordID <= 0)  // Legacy - normally check for NO_RECORD
    {
      pButtonIBBAYNA->SetCheck(TRUE);
    }
    else
    {
      pButtonIBBAY->SetCheck(TRUE);
      for(nI = 0; nI < numStops; nI++)
      {
        if((short int)pComboBoxIBBAYLIST->GetItemData(nI) == ROUTES.IBBayNODESrecordID)
        {
          pComboBoxIBBAYLIST->SetCurSel(nI);
          break;
        }
      }
    }
//
//  Comment
//
    if(ROUTES.COMMENTSrecordID == NO_RECORD)
    {
      pButtonNA->SetCheck(TRUE);
    }
    else
    {
      for(bFound = FALSE, nI = 0; nI < numComments; nI++)
      {
        if((long)pComboBoxCOMMENTCODE->GetItemData(nI) == ROUTES.COMMENTSrecordID)
        {
          pComboBoxCOMMENTCODE->SetCurSel(nI);
          bFound = TRUE;
          break;
        }
      }
      if(bFound)
      {
        pButtonCC->SetCheck(TRUE);
      }
      else
      {
        pButtonNA->SetCheck(TRUE);
      }
    }
  }
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CAddRoute::OnClose() 
{
  OnCancel();
}

void CAddRoute::OnCc() 
{
  pComboBoxCOMMENTCODE->SetCurSel(0);
}

void CAddRoute::OnNa() 
{
  pComboBoxCOMMENTCODE->SetCurSel(-1);
}

void CAddRoute::OnSelendokCommentcode() 
{
  if(pButtonNA->GetCheck())
  {
    pButtonNA->SetCheck(FALSE);
    pButtonCC->SetCheck(TRUE);
  }
}

void CAddRoute::OnBidirectional() 
{
  pComboBoxINBOUND->EnableWindow(TRUE);
  pButtonIBGROUPBOX->EnableWindow(TRUE);
  pButtonIBBAYNA->SetCheck(TRUE);
  pButtonIBBAYNA->EnableWindow(TRUE);
  pButtonIBBAY->SetCheck(FALSE);
  pButtonIBBAY->EnableWindow(TRUE);
  pComboBoxIBBAYLIST->SetCurSel(-1);
  pComboBoxIBBAYLIST->EnableWindow(TRUE);
}

void CAddRoute::OnCircular() 
{
  pComboBoxINBOUND->SetCurSel(-1);
  pComboBoxINBOUND->EnableWindow(FALSE);
  pButtonIBGROUPBOX->EnableWindow(FALSE);
  pButtonIBBAYNA->SetCheck(TRUE);
  pButtonIBBAYNA->EnableWindow(FALSE);
  pButtonIBBAY->SetCheck(FALSE);
  pButtonIBBAY->EnableWindow(FALSE);
  pComboBoxIBBAYLIST->SetCurSel(-1);
  pComboBoxIBBAYLIST->EnableWindow(FALSE);
}

void CAddRoute::OnObbayna() 
{
  pComboBoxOBBAYLIST->SetCurSel(-1);
}

void CAddRoute::OnObbay() 
{
  pComboBoxOBBAYLIST->SetCurSel(0);
}

void CAddRoute::OnSelendokObbaylist() 
{
  if(pButtonOBBAYNA->GetCheck())
  {
    pButtonOBBAYNA->SetCheck(FALSE);
    pButtonOBBAY->SetCheck(TRUE);
  }
}

void CAddRoute::OnIbbayna() 
{
  pComboBoxIBBAYLIST->SetCurSel(-1);
}

void CAddRoute::OnIbbay() 
{
  pComboBoxIBBAYLIST->SetCurSel(0);
}

void CAddRoute::OnSelendokIbbaylist() 
{
  if(pButtonIBBAYNA->GetCheck())
  {
    pButtonIBBAYNA->SetCheck(FALSE);
    pButtonIBBAY->SetCheck(TRUE);
  }
}

void CAddRoute::OnCancel() 
{
	CDialog::OnCancel();
}

void CAddRoute::OnHelp() 
{
}

void CAddRoute::OnOK() 
{
  long COMMENTSrecordID;
  long OBBayNODESrecordID;
  long IBBayNODESrecordID;
  char szNumber[ROUTES_NUMBER_LENGTH + 1];
  char szName[ROUTES_NAME_LENGTH + 1];
  int  nI, nJ, nK;
  int  rcode2;
  int  opCode;
//
//  Route Number
//
  pEditNUMBER->GetWindowText(szNumber, ROUTES_NUMBER_LENGTH);
  if(strcmp(szNumber, "") == 0)
  {
    TMSError(this->m_hWnd, MB_ICONSTOP, ERROR_009, pEditNUMBER->m_hWnd);
    return;
  }
//
//  Route Name
//
  pEditNAME->GetWindowText(szName, ROUTES_NAME_LENGTH);
  if(strcmp(szName, "") == 0)
  {
    TMSError(this->m_hWnd, MB_ICONSTOP, ERROR_012, pEditNAME->m_hWnd);
    return;
  }
//
//  Outbound direction - A direction must have been selected
//
  nI = pComboBoxOUTBOUND->GetCurSel();
  if(nI == CB_ERR)
  {
    TMSError(this->m_hWnd, MB_ICONSTOP, ERROR_013, pComboBoxOUTBOUND->m_hWnd);
    return;
  }
//
//  Inbound direction - If selected, it can't be the same as outbound
//
  nJ = pComboBoxINBOUND->GetCurSel();
  if(nJ == nI)
  {
    TMSError(this->m_hWnd, MB_ICONSTOP, ERROR_014, pComboBoxINBOUND->m_hWnd);
    return;
  }
//
//  Comment code
//
  if(pButtonNA->GetCheck())
  {
    COMMENTSrecordID = NO_RECORD;
  }
  else
  {
    nK = pComboBoxCOMMENTCODE->GetCurSel();
    COMMENTSrecordID = (nK == CB_ERR ? NO_RECORD : pComboBoxCOMMENTCODE->GetItemData(nK));
  }
//
//  Outbound bay
//
  if(pButtonOBBAYNA->GetCheck())
  {
    OBBayNODESrecordID = NO_RECORD;
  }
  else
  {
    nK = pComboBoxOBBAYLIST->GetCurSel();
    OBBayNODESrecordID = (nK == CB_ERR ? NO_RECORD : pComboBoxOBBAYLIST->GetItemData(nK));
  }
//
//  Inbound bay
//
  if(pButtonIBBAYNA->GetCheck())
  {
    IBBayNODESrecordID = NO_RECORD;
  }
  else
  {
    nK = pComboBoxIBBAYLIST->GetCurSel();
    IBBayNODESrecordID = (nK == CB_ERR ? NO_RECORD : pComboBoxIBBAYLIST->GetItemData(nK));
  }
//
//  Are we updating or inserting?
//
  if(*m_pUpdateRecordID == NO_RECORD)
  {
    rcode2 = btrieve(B_GETLAST, TMS_ROUTES, &ROUTES, &ROUTESKey0, 0);
    ROUTES.recordID = AssignRecID(rcode2, ROUTES.recordID);
    opCode = B_INSERT;
  }
  else
  {
    ROUTESKey0.recordID = updateRecordID;
    btrieve(B_GETEQUAL, TMS_ROUTES, &ROUTES, &ROUTESKey0, 0);
    opCode = B_UPDATE;
  }
//
//  Set up ROUTES
//
//  Number
//
  strncpy(ROUTES.number, szNumber, ROUTES_NUMBER_LENGTH);
  pad(ROUTES.number, ROUTES_NUMBER_LENGTH);
//
//  Name
//
  strncpy(ROUTES.name, szName, ROUTES_NAME_LENGTH);
  pad(ROUTES.name, ROUTES_NAME_LENGTH);
//
//  Directions
//
  ROUTES.DIRECTIONSrecordID[0] = pComboBoxOUTBOUND->GetItemData(nI);
  ROUTES.DIRECTIONSrecordID[1] = (nJ == CB_ERR ? NO_RECORD : pComboBoxOUTBOUND->GetItemData(nJ));
//
//  Flags
//
  ROUTES.flags = 0;
  if(pButtonSHUTTLE->GetCheck())
  {
    ROUTES.flags |= ROUTES_FLAG_EMPSHUTTLE;
  }
  if(pButtonSCHOOL->GetCheck())
  {
    ROUTES.flags |= ROUTES_FLAG_SCHOOL;
  }
  if(pButtonRGRP->GetCheck())
  {
    ROUTES.flags |= ROUTES_FLAG_RGRP;
  }
//
//  Comments
//
  ROUTES.COMMENTSrecordID = COMMENTSrecordID;
//
//  Bays
//
  ROUTES.OBBayNODESrecordID = (short int)OBBayNODESrecordID;
  ROUTES.IBBayNODESrecordID = (short int)IBBayNODESrecordID;
//
//  Insert / Update
//
  btrieve(opCode, TMS_ROUTES, &ROUTES, &ROUTESKey0, 0);
//
//  Make adjustments to the global route data variables
//
  if(*m_pUpdateRecordID == m_RouteRecordID)
  {
    strcpy(m_RouteNumber, szNumber);
    strcpy(m_RouteName, szName);
  }
  *m_pUpdateRecordID = ROUTES.recordID;
	
	CDialog::OnOK();
}

//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
// AddConnection.cpp : implementation file
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


void CAddConnection::DisplayActiveRoute(CComboBox *pCB, long routeRecordID)
{
  int nI;

  for(nI = 0; nI < m_numRoutes; nI++)
  {
    if((long)pCB->GetItemData(nI) == routeRecordID)
    {
      pCB->SetCurSel(nI);
      break;
    }
  }
}

void CAddConnection::DisplayActiveService(CComboBox *pCB, long serviceRecordID)
{
  int nI;

  for(nI = 0; nI < m_numServices; nI++)
  {
    if((long)pCB->GetItemData(nI) == serviceRecordID)
    {
      pCB->SetCurSel(nI);
      break;
    }
  }
}

BOOL CAddConnection::DisplayActivePattern(CComboBox *pCBRte, CComboBox *pCBSer,
      CComboBox *pCBPat, long patternnamesRecordID, int controlID)
{
  long routeRecordID;
  long serviceRecordID;
  int  nI;
  int  nJ;
  int  numPatterns;

  nI = pCBRte->GetCurSel();
  if(nI == CB_ERR)
  {
    return(FALSE);
  }

  nJ = pCBSer->GetCurSel();
  if(nJ == CB_ERR)
  {
    return(FALSE);
  }

  routeRecordID = pCBRte->GetItemData(nI);
  serviceRecordID = pCBSer->GetItemData(nJ);
  numPatterns = SetUpPatternList(m_hWnd, controlID, routeRecordID, serviceRecordID, 0, TRUE, TRUE);
  numPatterns += SetUpPatternList(m_hWnd, controlID, routeRecordID, serviceRecordID, 1, FALSE, TRUE);
  if(patternnamesRecordID == NO_RECORD)
  {
    pCBPat->SetCurSel(-1);
  }
  else
  {
    for(nI = 0; nI < numPatterns; nI++)
    {
      if((long)pCBPat->GetItemData(nI) == patternnamesRecordID)
      {
        pCBPat->SetCurSel(nI);
        break;
      }
    }
  }
  return(TRUE);
}


/////////////////////////////////////////////////////////////////////////////
// CAddConnection dialog


CAddConnection::CAddConnection(CWnd* pParent, long *pUpdateRecordID, PDISPLAYINFO pDI)
	: CDialog(CAddConnection::IDD, pParent)
{
  if(pDI == NULL)
  {
    OnCancel();
  }
  else
  {
    m_pDI = pDI;
    m_pUpdateRecordID = pUpdateRecordID;
    m_bInit = TRUE;
  }

	//{{AFX_DATA_INIT(CAddConnection)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CAddConnection::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAddConnection)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CAddConnection, CDialog)
	//{{AFX_MSG_MAP(CAddConnection)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDHELP, OnHelp)
	ON_BN_CLICKED(ADDCONNECTION_CC, OnCc)
	ON_BN_CLICKED(ADDCONNECTION_NA, OnNa)
	ON_CBN_SELENDOK(ADDCONNECTION_COMMENTCODE, OnSelendokCommentcode)
	ON_BN_CLICKED(ADDCONNECTION_FROMALLPATTERNS, OnFromallpatterns)
	ON_BN_CLICKED(ADDCONNECTION_FROMALLROUTES, OnFromallroutes)
	ON_BN_CLICKED(ADDCONNECTION_FROMALLSERVICES, OnFromallservices)
	ON_BN_CLICKED(ADDCONNECTION_TOALLPATTERNS, OnToallpatterns)
	ON_BN_CLICKED(ADDCONNECTION_TOALLROUTES, OnToallroutes)
	ON_BN_CLICKED(ADDCONNECTION_TOALLSERVICES, OnToallservices)
	ON_CBN_EDITCHANGE(ADDCONNECTION_FROMROUTE, OnEditchangeFromroute)
	ON_CBN_EDITCHANGE(ADDCONNECTION_FROMSERVICE, OnEditchangeFromservice)
	ON_CBN_EDITCHANGE(ADDCONNECTION_TOROUTE, OnEditchangeToroute)
	ON_CBN_EDITCHANGE(ADDCONNECTION_TOSERVICE, OnEditchangeToservice)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAddConnection message handlers

BOOL CAddConnection::OnInitDialog() 
{
	CDialog::OnInitDialog();

  CString s;
  BOOL bFound;
  BOOL bPickedNodes;
  int  rcode2;
  int  numComments;
  int  nI;

//
//  Set up pointers to the controls
//
  pComboBoxFROMNODE = (CComboBox *)GetDlgItem(ADDCONNECTION_FROMNODE);
  pComboBoxTONODE = (CComboBox *)GetDlgItem(ADDCONNECTION_TONODE);
  pEditCONNECTIONTIME = (CEdit *)GetDlgItem(ADDCONNECTION_CONNECTIONTIME);
  pEditDISTANCE = (CEdit *)GetDlgItem(ADDCONNECTION_DISTANCE);
  pButtonAPPLYTOALL = (CButton *)GetDlgItem(ADDCONNECTION_APPLYTOALL);
  pButtonFROMALLROUTES = (CButton *)GetDlgItem(ADDCONNECTION_FROMALLROUTES);
  pComboBoxFROMROUTE = (CComboBox *)GetDlgItem(ADDCONNECTION_FROMROUTE);
  pButtonFROMALLSERVICES = (CButton *)GetDlgItem(ADDCONNECTION_FROMALLSERVICES);
  pComboBoxFROMSERVICE = (CComboBox *)GetDlgItem(ADDCONNECTION_FROMSERVICE);
  pButtonFROMALLPATTERNS = (CButton *)GetDlgItem(ADDCONNECTION_FROMALLPATTERNS);
  pComboBoxFROMPATTERN = (CComboBox *)GetDlgItem(ADDCONNECTION_FROMPATTERN);
  pButtonTOALLROUTES = (CButton *)GetDlgItem(ADDCONNECTION_TOALLROUTES);
  pComboBoxTOROUTE = (CComboBox *)GetDlgItem(ADDCONNECTION_TOROUTE);
  pButtonTOALLSERVICES = (CButton *)GetDlgItem(ADDCONNECTION_TOALLSERVICES);
  pComboBoxTOSERVICE = (CComboBox *)GetDlgItem(ADDCONNECTION_TOSERVICE);
  pButtonTOALLPATTERNS = (CButton *)GetDlgItem(ADDCONNECTION_TOALLPATTERNS);
  pComboBoxTOPATTERN = (CComboBox *)GetDlgItem(ADDCONNECTION_TOPATTERN);
  pEditFROMTIMEOFDAY = (CEdit *)GetDlgItem(ADDCONNECTION_FROMTIMEOFDAY);
  pEditTOTIMEOFDAY = (CEdit *)GetDlgItem(ADDCONNECTION_TOTIMEOFDAY);
  pButtonONEWAY = (CButton *)GetDlgItem(ADDCONNECTION_ONEWAY);
  pButtonTWOWAY = (CButton *)GetDlgItem(ADDCONNECTION_TWOWAY);
  pButtonRUNNINGTIME = (CButton *)GetDlgItem(ADDCONNECTION_RUNNINGTIME);
  pButtonTRAVELTIME = (CButton *)GetDlgItem(ADDCONNECTION_TRAVELTIME);
  pButtonDEADHEADTIME = (CButton *)GetDlgItem(ADDCONNECTION_DEADHEADTIME);
  pButtonEQUIV = (CButton *)GetDlgItem(ADDCONNECTION_EQUIV);
  pButtonAVLEQUIVALENCE = (CButton *)GetDlgItem(ADDCONNECTION_AVLEQUIVALENCE);
  pButtonNA = (CButton *)GetDlgItem(ADDCONNECTION_NA);
  pButtonCC = (CButton *)GetDlgItem(ADDCONNECTION_CC);
  pComboBoxCOMMENTCODE = (CComboBox *)GetDlgItem(ADDCONNECTION_COMMENTCODE);
  pStaticCOUNTER = (CStatic *)GetDlgItem(ADDCONNECTION_COUNTER);
//
//  Comment codes combo box
//
  numComments = SetUpCommentList(m_hWnd, ADDCONNECTION_COMMENTCODE);
  if(numComments == 0)
  {
    pButtonCC->EnableWindow(FALSE);
    pComboBoxCOMMENTCODE->EnableWindow(FALSE);
  }
//
//  There has to be at least one entry in the Routes Table
//
  m_numRoutes = SetUpRouteList(m_hWnd, ADDCONNECTION_FROMROUTE, NO_RECORD);
  if(m_numRoutes == 0)
  {
    TMSError(m_hWnd, MB_ICONSTOP, ERROR_008, (HANDLE)NULL);
    OnCancel();
    return(TRUE);
  }
  SetUpRouteList(m_hWnd, ADDCONNECTION_TOROUTE, NO_RECORD);
//
//  There has to be at least one entry in the Services Table
//
  m_numServices = SetUpServiceList(m_hWnd, ADDCONNECTION_FROMSERVICE, NO_RECORD);
  if(m_numServices == 0)
  {
    TMSError(m_hWnd, MB_ICONSTOP, ERROR_007, (HANDLE)NULL);
    OnCancel();
    return(TRUE);
  }
  SetUpServiceList(m_hWnd, ADDCONNECTION_TOSERVICE, NO_RECORD);
//
//  Are we updating?
//
  bPickedNodes = FALSE;
  if(*m_pUpdateRecordID != NO_RECORD)
  {
    CONNECTIONSKey0.recordID = *m_pUpdateRecordID;
    btrieve(B_GETEQUAL, TMS_CONNECTIONS, &CONNECTIONS, &CONNECTIONSKey0, 0);
  }
//
//  No update, but if there's a record highlighted on the insert,
//  set the from and to nodes as the ones to display
//
  else
  {
    if(m_pDI->baggage != NO_RECORD)
    {
      bPickedNodes = TRUE;
      CONNECTIONSKey0.recordID = m_pDI->baggage;
      btrieve(B_GETEQUAL, TMS_CONNECTIONS, &CONNECTIONS, &CONNECTIONSKey0, 0);
    }
  }
//
//  There has to be at least one entry in the Nodes Table
//
  rcode2 = btrieve(B_GETFIRST, TMS_NODES, &NODES, &NODESKey2, 2);
  if(rcode2 != 0)
  {
    TMSError(m_hWnd, MB_ICONSTOP, ERROR_024, (HANDLE)NULL);
    OnCancel();
    return(TRUE);
  }
//
//  There were nodes.  Populate the from and to node combo boxes
//
  while(rcode2 == 0)
  {
    if(!(NODES.flags & NODES_FLAG_STOP))
    {
      s = GetDatabaseString(NODES.abbrName, NODES_ABBRNAME_LENGTH);
      nI = pComboBoxFROMNODE->AddString(s);
      pComboBoxFROMNODE->SetItemData(nI, NODES.recordID);
      if((*m_pUpdateRecordID != NO_RECORD || bPickedNodes) &&
            CONNECTIONS.fromNODESrecordID == NODES.recordID)
      {
        pComboBoxFROMNODE->SetCurSel(nI);
      }
      nI = pComboBoxTONODE->AddString(s);
      pComboBoxTONODE->SetItemData(nI, NODES.recordID);
      if((*m_pUpdateRecordID != NO_RECORD || bPickedNodes) &&
            CONNECTIONS.toNODESrecordID == NODES.recordID)
      {
        pComboBoxTONODE->SetCurSel(nI);
      }
    }
    rcode2 = btrieve(B_GETNEXT, TMS_NODES, &NODES, &NODESKey2, 2);
  }
//
//  From route
//
  if(*m_pUpdateRecordID == NO_RECORD)
  {
    DisplayActiveRoute(pComboBoxFROMROUTE, m_pDI->fileInfo.routeRecordID);
  }
  else
  {
    if(CONNECTIONS.fromROUTESrecordID != NO_RECORD)
    {
      DisplayActiveRoute(pComboBoxFROMROUTE, CONNECTIONS.fromROUTESrecordID);
    }
    else
    {
      pButtonFROMALLROUTES->SetCheck(TRUE);
      pComboBoxFROMROUTE->EnableWindow(FALSE);
    }
  }
//
//  From service
//
  if(*m_pUpdateRecordID == NO_RECORD)
  {
    DisplayActiveService(pComboBoxFROMSERVICE, m_pDI->fileInfo.serviceRecordID);
  }
  else
  {
    if(CONNECTIONS.fromSERVICESrecordID != NO_RECORD)
    {
      DisplayActiveService(pComboBoxFROMSERVICE, CONNECTIONS.fromSERVICESrecordID);
    }
    else
    {
      pButtonFROMALLSERVICES->SetCheck(TRUE);
      pComboBoxFROMSERVICE->EnableWindow(FALSE);
    }
  }
//
//  From Pattern
//
  if(*m_pUpdateRecordID == NO_RECORD || CONNECTIONS.fromPATTERNNAMESrecordID == NO_RECORD)
  {
    pButtonFROMALLPATTERNS->SetCheck(TRUE);
    pComboBoxFROMPATTERN->EnableWindow(FALSE);
  }
  else
  {
    DisplayActivePattern(pComboBoxFROMROUTE, pComboBoxFROMSERVICE,
          pComboBoxFROMPATTERN, CONNECTIONS.fromPATTERNNAMESrecordID, ADDCONNECTION_FROMPATTERN);
  }
//
//  To route
//
  if(*m_pUpdateRecordID == NO_RECORD)
  {
    DisplayActiveRoute(pComboBoxTOROUTE, m_pDI->fileInfo.routeRecordID);
  }
  else
  {
    if(CONNECTIONS.toROUTESrecordID != NO_RECORD)
    {
      DisplayActiveRoute(pComboBoxTOROUTE, CONNECTIONS.toROUTESrecordID);
    }
    else
    {
      pButtonTOALLROUTES->SetCheck(TRUE);
      pComboBoxTOROUTE->EnableWindow(FALSE);
    }
  }
//
//  To service
//
  if(*m_pUpdateRecordID == NO_RECORD)
  {
    DisplayActiveService(pComboBoxTOSERVICE, m_pDI->fileInfo.serviceRecordID);
  }
  else
  {
    if(CONNECTIONS.toSERVICESrecordID != NO_RECORD)
    {
      DisplayActiveService(pComboBoxTOSERVICE, CONNECTIONS.toSERVICESrecordID);
    }
    else
    {
      pButtonTOALLSERVICES->SetCheck(TRUE);
      pComboBoxTOSERVICE->EnableWindow(FALSE);
    }
  }
//
//  To Pattern
//
  if(*m_pUpdateRecordID == NO_RECORD || CONNECTIONS.toPATTERNNAMESrecordID == NO_RECORD)
  {
    pButtonTOALLPATTERNS->SetCheck(TRUE);
    pComboBoxTOPATTERN->EnableWindow(FALSE);
  }
  else
  {
    DisplayActivePattern(pComboBoxTOROUTE, pComboBoxTOSERVICE,
          pComboBoxTOPATTERN, CONNECTIONS.toPATTERNNAMESrecordID, ADDCONNECTION_TOPATTERN);
  }
//
//  On update, display the current connection time,
//  distance, and from and to times if they exist
//
  if(*m_pUpdateRecordID != NO_RECORD)
  {
//
//  Connection time
//
    s.Format("%ld", CONNECTIONS.connectionTime / 60);
    if(CONNECTIONS.connectionTime % 60 != 0)
    {
      s += "+";
    }
    pEditCONNECTIONTIME->SetWindowText(s);
//
//  Distance
//
    if(CONNECTIONS.distance != NO_RECORD)
    {
      s.Format("%5.2f", (float)CONNECTIONS.distance);
      pEditDISTANCE->SetWindowText(s);
    }
//
//  From / to time of day
//
    pEditFROMTIMEOFDAY->SetWindowText(Tchar(CONNECTIONS.fromTimeOfDay));
    pEditTOTIMEOFDAY->SetWindowText(Tchar(CONNECTIONS.toTimeOfDay));
  }
//
//  1 or 2 way
//
  if(*m_pUpdateRecordID != NO_RECORD && (CONNECTIONS.flags & CONNECTIONS_FLAG_TWOWAY))
  {
    pButtonTWOWAY->SetCheck(TRUE);
  }
  else
  {
    pButtonONEWAY->SetCheck(TRUE);
  }
//
//  Running, Travel, and Deadhead Time checkboxes,
//  and From/To Nodes Equivalent checkbox
//
  if(*m_pUpdateRecordID != NO_RECORD)
  {
    if(CONNECTIONS.flags & CONNECTIONS_FLAG_RUNNINGTIME)
    {
      pButtonRUNNINGTIME->SetCheck(TRUE);
    }
    if(CONNECTIONS.flags & CONNECTIONS_FLAG_TRAVELTIME)
    {
      pButtonTRAVELTIME->SetCheck(TRUE);
    }
    if(CONNECTIONS.flags & CONNECTIONS_FLAG_DEADHEADTIME)
    {
      pButtonDEADHEADTIME->SetCheck(TRUE);
    }
    if(CONNECTIONS.flags & CONNECTIONS_FLAG_EQUIVALENT)
    {
      pButtonEQUIV->SetCheck(TRUE);
    }
    if(CONNECTIONS.flags & CONNECTIONS_FLAG_AVLEQUIVALENT)
    {
      pButtonAVLEQUIVALENCE->SetCheck(TRUE);
    }
  }
  else
  {
    pButtonRUNNINGTIME->SetCheck(TRUE);
    pButtonTRAVELTIME->SetCheck(FALSE);
    pButtonDEADHEADTIME->SetCheck(FALSE);
    pButtonEQUIV->SetCheck(FALSE);
    pButtonAVLEQUIVALENCE->SetCheck(FALSE);
  }
//
//  Comment
//
  if(*m_pUpdateRecordID != NO_RECORD)
  {
    for(bFound = FALSE, nI = 0; nI < numComments; nI++)
    {
      if((long)pComboBoxCOMMENTCODE->GetItemData(nI) == CONNECTIONS.COMMENTSrecordID)
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
  else
  {
    pButtonNA->SetCheck(TRUE);
  }

  m_bInit = FALSE;

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CAddConnection::OnClose() 
{
  OnCancel();
}

void CAddConnection::OnCancel() 
{
	CDialog::OnCancel();
}

void CAddConnection::OnHelp() 
{
  ::WinHelp(m_hWnd, szarHelpFile, HELP_CONTEXT, The_Connection_Times_Table);
}

void CAddConnection::OnCc() 
{
  pComboBoxCOMMENTCODE->SetCurSel(0);
}

void CAddConnection::OnNa() 
{
  pComboBoxCOMMENTCODE->SetCurSel(-1);
}

void CAddConnection::OnSelendokCommentcode() 
{
  if(pButtonNA->GetCheck())
  {
    pButtonNA->SetCheck(FALSE);
    pButtonCC->SetCheck(TRUE);
  }
}

void CAddConnection::OnFromallroutes() 
{
  if(pButtonFROMALLROUTES->GetCheck())
  {
    pComboBoxFROMROUTE->SetCurSel(-1);
    pComboBoxFROMROUTE->EnableWindow(FALSE);
    pButtonFROMALLPATTERNS->SetCheck(TRUE);
    pComboBoxFROMPATTERN->SetCurSel(-1);
    pComboBoxFROMPATTERN->EnableWindow(FALSE);
  }
  else
  {
    DisplayActiveRoute(pComboBoxFROMROUTE, m_pDI->fileInfo.routeRecordID);
    pComboBoxFROMROUTE->EnableWindow(TRUE);
  }
}

void CAddConnection::OnFromallservices() 
{
  if(pButtonFROMALLSERVICES->GetCheck())
  {
    pComboBoxFROMSERVICE->SetCurSel(-1);
    pComboBoxFROMSERVICE->EnableWindow(FALSE);
    pButtonFROMALLPATTERNS->SetCheck(TRUE);
    pComboBoxFROMPATTERN->SetCurSel(-1);
    pComboBoxFROMPATTERN->EnableWindow(FALSE);
  }
  else
  {
    DisplayActiveService(pComboBoxFROMSERVICE, m_pDI->fileInfo.serviceRecordID);
    pComboBoxFROMSERVICE->EnableWindow(TRUE);
  }
}

void CAddConnection::OnFromallpatterns() 
{
  if(pButtonFROMALLPATTERNS->GetCheck())
  {
    pComboBoxFROMPATTERN->SetCurSel(-1);
    pComboBoxFROMPATTERN->EnableWindow(FALSE);
  }
  else
  {
    if(DisplayActivePattern(pComboBoxFROMROUTE, pComboBoxFROMSERVICE,
          pComboBoxFROMPATTERN, NO_RECORD, ADDCONNECTION_FROMPATTERN))
    {
      pComboBoxFROMPATTERN->EnableWindow(TRUE);
    }
    else
    {
      pButtonFROMALLPATTERNS->SetCheck(TRUE);
      pComboBoxFROMPATTERN->SetCurSel(-1);
      pComboBoxFROMPATTERN->EnableWindow(FALSE);
    }
  }
}

void CAddConnection::OnToallroutes() 
{
  if(pButtonTOALLROUTES->GetCheck())
  {
    pComboBoxTOROUTE->SetCurSel(-1);
    pComboBoxTOROUTE->EnableWindow(FALSE);
    pButtonTOALLPATTERNS->SetCheck(TRUE);
    pComboBoxTOPATTERN->SetCurSel(-1);
    pComboBoxTOPATTERN->EnableWindow(FALSE);
  }
  else
  {
    DisplayActiveRoute(pComboBoxTOROUTE, m_pDI->fileInfo.routeRecordID);
    pComboBoxTOROUTE->EnableWindow(TRUE);
  }
}

void CAddConnection::OnToallservices() 
{
  if(pButtonTOALLSERVICES->GetCheck())
  {
    pComboBoxTOSERVICE->SetCurSel(-1);
    pComboBoxTOSERVICE->EnableWindow(FALSE);
    pButtonTOALLPATTERNS->SetCheck(TRUE);
    pComboBoxTOPATTERN->SetCurSel(-1);
    pComboBoxTOPATTERN->EnableWindow(FALSE);
  }
  else
  {
    DisplayActiveService(pComboBoxTOSERVICE, m_pDI->fileInfo.serviceRecordID);
    pComboBoxTOSERVICE->EnableWindow(TRUE);
  }
}

void CAddConnection::OnToallpatterns() 
{
  if(pButtonTOALLPATTERNS->GetCheck())
  {
    pComboBoxTOPATTERN->SetCurSel(-1);
    pComboBoxTOPATTERN->EnableWindow(FALSE);
  }
  else
  {
    if(DisplayActivePattern(pComboBoxTOROUTE, pComboBoxTOSERVICE,
          pComboBoxTOPATTERN, NO_RECORD, ADDCONNECTION_TOPATTERN))
    {
      pComboBoxTOPATTERN->EnableWindow(TRUE);
    }
    else
    {
      pButtonTOALLPATTERNS->SetCheck(TRUE);
      pComboBoxTOPATTERN->SetCurSel(-1);
      pComboBoxTOPATTERN->EnableWindow(FALSE);
    }
  }
}


void CAddConnection::OnEditchangeFromroute() 
{
  pButtonFROMALLPATTERNS->SetCheck(TRUE);
  pComboBoxFROMPATTERN->SetCurSel(-1);
  pComboBoxFROMPATTERN->EnableWindow(FALSE);
}

void CAddConnection::OnEditchangeFromservice() 
{
  pButtonFROMALLPATTERNS->SetCheck(TRUE);
  pComboBoxFROMPATTERN->SetCurSel(-1);
  pComboBoxFROMPATTERN->EnableWindow(FALSE);
}

void CAddConnection::OnEditchangeToroute() 
{
  pButtonTOALLPATTERNS->SetCheck(TRUE);
  pComboBoxTOPATTERN->SetCurSel(-1);
  pComboBoxTOPATTERN->EnableWindow(FALSE);
}

void CAddConnection::OnEditchangeToservice() 
{
  pButtonTOALLPATTERNS->SetCheck(TRUE);
  pComboBoxTOPATTERN->SetCurSel(-1);
  pComboBoxTOPATTERN->EnableWindow(FALSE);
}

void CAddConnection::OnOK() 
{
  CString s;
  float distance;
  BOOL  bAddToList;
  BOOL  bFound;
  long  fromNodeRecordID;
  long  toNodeRecordID;
  long  connectionTime;
  int   nI;
  int   opCode;
  int   rcode2;
//
//  Perform error checks
//
//  From Node
//
  nI = pComboBoxFROMNODE->GetCurSel();
  if(nI == CB_ERR)
  {
    TMSError(m_hWnd, MB_ICONSTOP, ERROR_025, pComboBoxFROMNODE->m_hWnd);
    return;
  }
  fromNodeRecordID = pComboBoxFROMNODE->GetItemData(nI);
//
//  To Node
//
  nI = pComboBoxTONODE->GetCurSel();
  if(nI == CB_ERR)
  {
    TMSError(m_hWnd, MB_ICONSTOP, ERROR_026, pComboBoxTONODE->m_hWnd);
    return;
  }
  toNodeRecordID = pComboBoxTONODE->GetItemData(nI);
//
//  Connection Time
//
  pEditCONNECTIONTIME->GetWindowText(s);
  connectionTime = atol(s) * 60;
  if(s.Remove('+'))
  {
    connectionTime += 30;
  }
  if(connectionTime < 0)
  {
    TMSError(m_hWnd, MB_ICONSTOP, ERROR_027, pEditCONNECTIONTIME->m_hWnd);
    return;
  }
//
//  Distance
//
  pEditDISTANCE->GetWindowText(s);
  if(s.IsEmpty())
  {
    distance = NO_RECORD;
  }
  else
  {
    distance = (float)atof(s);
    if(distance < 0)
    {
      TMSError(m_hWnd, MB_ICONSTOP, ERROR_126, pEditDISTANCE->m_hWnd);
      return;
    }
  }
//
//  OK - Passed, the critical checks - Establish a record ID
//
  if(*m_pUpdateRecordID != NO_RECORD)
  {
    CONNECTIONSKey0.recordID = *m_pUpdateRecordID;
    btrieve(B_GETEQUAL, TMS_CONNECTIONS, &CONNECTIONS, &CONNECTIONSKey0, 0);
    opCode = B_UPDATE;
    bAddToList = FALSE;
  }
//
//  See if we have to add the connnection to the global CONN structure.
//
  else
  {
    CONNECTIONSKey1.fromNODESrecordID = fromNodeRecordID;
    CONNECTIONSKey1.toNODESrecordID = toNodeRecordID;
    CONNECTIONSKey1.fromTimeOfDay = NO_TIME;
    rcode2 = btrieve(B_GETGREATEROREQUAL, TMS_CONNECTIONS, &CONNECTIONS, &CONNECTIONSKey1, 1);
    bAddToList = (rcode2 != 0 ||
          CONNECTIONS.fromNODESrecordID != fromNodeRecordID ||
          CONNECTIONS.toNODESrecordID != toNodeRecordID) &&
          CONN.numConnections < MAXCONNECTIONSKEPT - 1;
    rcode2 = btrieve(B_GETLAST, TMS_CONNECTIONS, &CONNECTIONS, &CONNECTIONSKey0, 0);
    CONNECTIONS.recordID = AssignRecID(rcode2, CONNECTIONS.recordID);
    opCode = B_INSERT;
  }
//
//  From Node
//
  CONNECTIONS.fromNODESrecordID = fromNodeRecordID;
//
//  To Node
//
  CONNECTIONS.toNODESrecordID = toNodeRecordID;
//
//  Connection Time - Converted to seconds above
//
  CONNECTIONS.connectionTime = connectionTime;
//
//  Distance
//
  CONNECTIONS.distance = distance;
//
//  From Route
//
  if(pButtonFROMALLROUTES->GetCheck())
  {
    CONNECTIONS.fromROUTESrecordID = NO_RECORD;
  }
  else
  {
    nI = pComboBoxFROMROUTE->GetCurSel();
    if(nI == CB_ERR)
    {
      CONNECTIONS.fromROUTESrecordID = NO_RECORD;
    }
    else
    {
      CONNECTIONS.fromROUTESrecordID = pComboBoxFROMROUTE->GetItemData(nI);
    }
  }
//
//  From Service
//
  if(pButtonFROMALLSERVICES->GetCheck())
  {
    CONNECTIONS.fromSERVICESrecordID = NO_RECORD;
  }
  else
  {
    nI = pComboBoxFROMSERVICE->GetCurSel();
    if(nI == CB_ERR)
    {
      CONNECTIONS.fromSERVICESrecordID = NO_RECORD;
    }
    else
    {
      CONNECTIONS.fromSERVICESrecordID = pComboBoxFROMSERVICE->GetItemData(nI);
    }
  }
//
//  From Pattern
//
  if(pButtonFROMALLPATTERNS->GetCheck())
  {
    CONNECTIONS.fromPATTERNNAMESrecordID = NO_RECORD;
  }
  else
  {
    nI = pComboBoxFROMPATTERN->GetCurSel();
    if(nI == CB_ERR)
    {
      CONNECTIONS.fromPATTERNNAMESrecordID = NO_RECORD;
    }
    else
    {
      CONNECTIONS.fromPATTERNNAMESrecordID = pComboBoxFROMPATTERN->GetItemData(nI);
    }
  }
//
//  To Route
//
  if(pButtonTOALLROUTES->GetCheck())
  {
    CONNECTIONS.toROUTESrecordID = NO_RECORD;
  }
  else
  {
    nI = pComboBoxTOROUTE->GetCurSel();
    if(nI == CB_ERR)
    {
      CONNECTIONS.toROUTESrecordID = NO_RECORD;
    }
    else
    {
      CONNECTIONS.toROUTESrecordID = pComboBoxTOROUTE->GetItemData(nI);
    }
  }
//
//  To Service
//
  if(pButtonTOALLSERVICES->GetCheck())
  {
    CONNECTIONS.toSERVICESrecordID = NO_RECORD;
  }
  else
  {
    nI = pComboBoxTOSERVICE->GetCurSel();
    if(nI == CB_ERR)
    {
      CONNECTIONS.toSERVICESrecordID = NO_RECORD;
    }
    else
    {
      CONNECTIONS.toSERVICESrecordID = pComboBoxTOSERVICE->GetItemData(nI);
    }
  }
//
//  To Pattern
//
  if(pButtonTOALLPATTERNS->GetCheck())
  {
    CONNECTIONS.toPATTERNNAMESrecordID = NO_RECORD;
  }
  else
  {
    nI = pComboBoxTOPATTERN->GetCurSel();
    if(nI == CB_ERR)
    {
      CONNECTIONS.toPATTERNNAMESrecordID = NO_RECORD;
    }
    else
    {
      CONNECTIONS.toPATTERNNAMESrecordID = pComboBoxTOPATTERN->GetItemData(nI);
    }
  }
//
//  From time of day
//
  pEditFROMTIMEOFDAY->GetWindowText(s);
  if(s.IsEmpty())
  {
    CONNECTIONS.fromTimeOfDay = NO_TIME;
  }
  else
  {
    strcpy(tempString, s);
    CONNECTIONS.fromTimeOfDay = cTime(tempString);
  }
//
//  To time of day
//
  pEditTOTIMEOFDAY->GetWindowText(s);
  if(s.IsEmpty())
  {
    CONNECTIONS.toTimeOfDay = CONNECTIONS.fromTimeOfDay;
  }
  else
  {
    strcpy(tempString, s);
    CONNECTIONS.toTimeOfDay = cTime(tempString);
  }
//
//  Flags
//
  CONNECTIONS.flags = 0;
//
//  One or Two-Way
//
  if(pButtonTWOWAY->GetCheck())
  {
    CONNECTIONS.flags |= CONNECTIONS_FLAG_TWOWAY;
  }
//
//  Running time flag
//
  if(pButtonRUNNINGTIME->GetCheck())
  {
    CONNECTIONS.flags |= CONNECTIONS_FLAG_RUNNINGTIME;
  }
//
//  Travel time flag
//
  if(pButtonTRAVELTIME->GetCheck())
  {
    CONNECTIONS.flags |= CONNECTIONS_FLAG_TRAVELTIME;
  }
//
//  Deadhead time flag
//
  if(pButtonDEADHEADTIME->GetCheck())
  {
    CONNECTIONS.flags |= CONNECTIONS_FLAG_DEADHEADTIME;
  }
//
//  From/To Nodes Equivalent
//
  if(pButtonEQUIV->GetCheck())
  {
    CONNECTIONS.flags |= CONNECTIONS_FLAG_EQUIVALENT;
  }
//
//  From/To Nodes AVL Equivalent
//
  if(pButtonAVLEQUIVALENCE->GetCheck())
  {
    CONNECTIONS.flags |= CONNECTIONS_FLAG_AVLEQUIVALENT;
  }
//
//  Comment
//
  if(pButtonNA->GetCheck())
  {
    CONNECTIONS.COMMENTSrecordID = NO_RECORD;
  }
  else
  {
    nI = pComboBoxCOMMENTCODE->GetCurSel();
    if(nI == CB_ERR)
    {
      CONNECTIONS.COMMENTSrecordID = NO_RECORD;
    }
    else
    {
      CONNECTIONS.COMMENTSrecordID = pComboBoxCOMMENTCODE->GetItemData(nI);
    }
  }
//
//  Forward pointer to ROUTINGS
//
  CONNECTIONS.ROUTINGSrecordID = NO_RECORD;
//
//  Update the record
//
  if(opCode == B_UPDATE)
  {
    rcode2 = btrieve(opCode, TMS_CONNECTIONS, &CONNECTIONS, &CONNECTIONSKey0, 0);
    if(rcode2 != 0)
    {
      sprintf(tempString, "Insert/Update failed on Connections Table.  rcode2 was %d", rcode2);
      MessageBox(tempString, TMS, MB_OK);
    }
    *m_pUpdateRecordID = CONNECTIONS.recordID;
  }
//
//  Insert the record
//
//  First, make sure there isn't any contention
  else
  {
    CONNECTIONSDef tempCONN;
    long newRecID = CONNECTIONS.recordID;
    int  counter = 0;
    while(counter < 500)
    {
      sprintf(tempString, "%d", counter);
      pStaticCOUNTER->SetWindowText(tempString);
      rcode2 = btrieve(B_GETLAST, TMS_CONNECTIONS, &tempCONN, &CONNECTIONSKey0, 0);
      CONNECTIONS.recordID = AssignRecID(rcode2, tempCONN.recordID);
      if(CONNECTIONS.recordID == newRecID)
      {
        rcode2 = btrieve(opCode, TMS_CONNECTIONS, &CONNECTIONS, &CONNECTIONSKey0, 0);
        if(rcode2 == 0)
        {
          *m_pUpdateRecordID = CONNECTIONS.recordID;
          break;
        }
      }
      counter++;
      newRecID = tempCONN.recordID;
    }
  }
//
//  Add it to the list, if necessary
//
  bFound = FALSE;
  if(bAddToList)
  {
    for(nI = 0; nI < CONN.numConnections; nI++)
    {
      if((bFound = CONN.startRecordID[nI] == CONNECTIONS.recordID) == TRUE)
      {
        break;
      }
    }
  }
  if(!bFound)
  {
    CONN.startRecordID[CONN.numConnections++] = CONNECTIONS.recordID;
  }
//
//  Was "Apply to All" checked?
//
  if(pButtonAPPLYTOALL->GetCheck())
  {
    CONNECTIONSKey1.fromNODESrecordID = fromNodeRecordID;
    CONNECTIONSKey1.toNODESrecordID = toNodeRecordID;
    CONNECTIONSKey1.fromTimeOfDay = NO_TIME;
    rcode2 = btrieve(B_GETGREATEROREQUAL, TMS_CONNECTIONS, &CONNECTIONS, &CONNECTIONSKey1, 1);
    while(rcode2 == 0 &&
          CONNECTIONS.fromNODESrecordID == fromNodeRecordID &&
          CONNECTIONS.toNODESrecordID == toNodeRecordID)
    {
      CONNECTIONS.distance = distance;
      btrieve(B_UPDATE, TMS_CONNECTIONS, &CONNECTIONS, &CONNECTIONSKey1, 1);
      rcode2 = btrieve(B_GETNEXT, TMS_CONNECTIONS, &CONNECTIONS, &CONNECTIONSKey1, 1);
    }
  }
//
//  Done
//
  m_bEstablishRUNTIMES = TRUE;

	CDialog::OnOK();
}

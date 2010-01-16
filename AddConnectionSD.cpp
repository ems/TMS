// AddConnectionSD.cpp : implementation file
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
// CAddConnectionSD dialog


CAddConnectionSD::CAddConnectionSD(CWnd* pParent, AddConnectionSDDataDef* pAddConnectionSDData)
	: CDialog(CAddConnectionSD::IDD, pParent)
{
  m_pAddConnectionSDData = pAddConnectionSDData;

	//{{AFX_DATA_INIT(CAddConnectionSD)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CAddConnectionSD::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAddConnectionSD)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CAddConnectionSD, CDialog)
	//{{AFX_MSG_MAP(CAddConnectionSD)
	ON_WM_CLOSE()
	ON_CBN_SELCHANGE(ADDCONNECTIONSD_FROMNODE, OnSelchangeFromnode)
	ON_CBN_SELCHANGE(ADDCONNECTIONSD_TONODE, OnSelchangeTonode)
	ON_EN_CHANGE(ADDCONNECTIONSD_DISTANCE, OnChangeDistance)
	ON_BN_CLICKED(ADDCONNECTIONSD_ALLPATTERNS, OnAllpatterns)
	ON_CBN_SELCHANGE(ADDCONNECTIONSD_PATTERN, OnSelchangePattern)
	ON_BN_CLICKED(ADDCONNECTIONSD_ROUNDUP, OnRoundup)
	ON_BN_CLICKED(ADDCONNECTIONSD_ROUNDDOWN, OnRounddown)
	ON_BN_CLICKED(ADDCONNECTIONSD_HALFMINUTES, OnHalfminutes)
	ON_BN_CLICKED(ADDCONNECTIONSD_FULLMINUTES, OnFullminutes)
	ON_BN_CLICKED(IDCALCULATE, OnCalculate)
	ON_BN_CLICKED(IDHELP, OnHelp)
	ON_BN_CLICKED(ADDCONNECTIONSD_ALLROUTES, OnAllroutes)
	ON_BN_CLICKED(ADDCONNECTIONSD_ALLSERVICES, OnAllservices)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAddConnectionSD message handlers

BOOL CAddConnectionSD::OnInitDialog() 
{
	CDialog::OnInitDialog();
//
//  Set up pointers to the controls
//
  pComboBoxFROMNODE = (CComboBox *)GetDlgItem(ADDCONNECTIONSD_FROMNODE);
  pComboBoxTONODE = (CComboBox *)GetDlgItem(ADDCONNECTIONSD_TONODE);
  pEditDISTANCE = (CEdit *)GetDlgItem(ADDCONNECTIONSD_DISTANCE);
  pButtonALLROUTES = (CButton *)GetDlgItem(ADDCONNECTIONSD_ALLROUTES);
  pComboBoxROUTE = (CComboBox *)GetDlgItem(ADDCONNECTIONSD_ROUTE);
  pButtonALLSERVICES = (CButton *)GetDlgItem(ADDCONNECTIONSD_ALLSERVICES);
  pComboBoxSERVICE = (CComboBox *)GetDlgItem(ADDCONNECTIONSD_SERVICE);
  pButtonALLPATTERNS = (CButton *)GetDlgItem(ADDCONNECTIONSD_ALLPATTERNS);
  pComboBoxPATTERN = (CComboBox *)GetDlgItem(ADDCONNECTIONSD_PATTERN);
  pButtonRUNNINGTIME = (CButton *)GetDlgItem(ADDCONNECTIONSD_RUNNINGTIME);
  pButtonTRAVELTIME = (CButton *)GetDlgItem(ADDCONNECTIONSD_TRAVELTIME);
  pButtonDEADHEADTIME = (CButton *)GetDlgItem(ADDCONNECTIONSD_DEADHEADTIME);
  pButtonONEWAY = (CButton *)GetDlgItem(ADDCONNECTIONSD_ONEWAY);
  pButtonTWOWAY = (CButton *)GetDlgItem(ADDCONNECTIONSD_TWOWAY);
  pSDIGridGRID = (CSDIGrid *)GetDlgItem(ADDCONNECTIONSD_GRID);
  pButtonROUNDUP = (CButton *)GetDlgItem(ADDCONNECTIONSD_ROUNDUP);
  pButtonROUNDDOWN = (CButton *)GetDlgItem(ADDCONNECTIONSD_ROUNDDOWN);
  pButtonHALFMINUTES = (CButton *)GetDlgItem(ADDCONNECTIONSD_HALFMINUTES);
  pButtonFULLMINUTES = (CButton *)GetDlgItem(ADDCONNECTIONSD_FULLMINUTES);
  pButtonIDCALCULATE = (CButton *)GetDlgItem(IDCALCULATE);
//
//  There has to be at least one entry in the Routes Table
//
  int numRoutes = SetUpRouteList(m_hWnd, ADDCONNECTIONSD_ROUTE, NO_RECORD);

  if(numRoutes == 0)
  {
    TMSError(m_hWnd, MB_ICONSTOP, ERROR_008, (HANDLE)NULL);
    OnCancel();
    return(TRUE);
  }
//
//  There has to be at least one entry in the Services Table
//
  int numServices = SetUpServiceList(m_hWnd, ADDCONNECTIONSD_SERVICE, NO_RECORD);

  if(numServices == 0)
  {
    TMSError(m_hWnd, MB_ICONSTOP, ERROR_007, (HANDLE)NULL);
    OnCancel();
    return(TRUE);
  }
//
//  There has to be at least one entry in the Nodes Table
//
  int rcode2 = btrieve(B_GETFIRST, TMS_NODES, &NODES, &NODESKey2, 2);

  if(rcode2 != 0)
  {
    TMSError(m_hWnd, MB_ICONSTOP, ERROR_024, (HANDLE)NULL);
    OnCancel();
    return(TRUE);
  }
//
//  There were nodes.  Populate the from and to node combo boxes
//
  CString s;
  int     nI;

  while(rcode2 == 0)
  {
    if(!(NODES.flags & NODES_FLAG_STOP))
    {
      s = GetDatabaseString(NODES.abbrName, NODES_ABBRNAME_LENGTH);
      nI = pComboBoxFROMNODE->AddString(s);
      pComboBoxFROMNODE->SetItemData(nI, NODES.recordID);
      nI = pComboBoxTONODE->AddString(s);
      pComboBoxTONODE->SetItemData(nI, NODES.recordID);
    }
    rcode2 = btrieve(B_GETNEXT, TMS_NODES, &NODES, &NODESKey2, 2);
  }
//
//  Route
//
  for(nI = 0; nI < numRoutes; nI++)
  {
    if((long)pComboBoxROUTE->GetItemData(nI) == m_pAddConnectionSDData->pDI->fileInfo.routeRecordID)
    {
      pComboBoxROUTE->SetCurSel(nI);
      break;
    }
  }
//
//  Service
//
  for(nI = 0; nI < numServices; nI++)
  {
    if((long)pComboBoxSERVICE->GetItemData(nI) == m_pAddConnectionSDData->pDI->fileInfo.serviceRecordID)
    {
      pComboBoxSERVICE->SetCurSel(nI);
      break;
    }
  }
//
//  Pattern
//
  pButtonALLPATTERNS->SetCheck(TRUE);
  pComboBoxPATTERN->EnableWindow(FALSE);
//
//  1 or 2 way
//
  pButtonONEWAY->SetCheck(TRUE);
//
//  Running, Travel, and Deadhead Time checkboxes
//
  pButtonRUNNINGTIME->SetCheck(TRUE);
  pButtonTRAVELTIME->SetCheck(FALSE);
  pButtonDEADHEADTIME->SetCheck(FALSE);
//
//  Round up/down
//
  pButtonROUNDUP->SetCheck(TRUE);
  pButtonROUNDDOWN->SetCheck(FALSE);
//
//  Full/half minutes
//
  pButtonHALFMINUTES->SetCheck(FALSE);
  pButtonFULLMINUTES->SetCheck(TRUE);
//
//  Set up the DataGrid
//
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CAddConnectionSD::OnClose() 
{
  OnCancel();
}

//
//  OnSelchangeFromnode()
//
void CAddConnectionSD::OnSelchangeFromnode() 
{
  float longitude[2];
  float latitude[2];
//
//  Detemine what we're dealing with
//
//  First, get the from and to node record IDs
//
  long fromNode, toNode;
  int  nI;

  nI = pComboBoxFROMNODE->GetCurSel();
  if(nI == CB_ERR)
  {
    return;
  }
  fromNode = pComboBoxFROMNODE->GetItemData(nI);

  nI = pComboBoxTONODE->GetCurSel();
  if(nI == CB_ERR)
  {
    return;
  }
  toNode = pComboBoxTONODE->GetItemData(nI);
//
//  Enable the Calculate button
//
  pButtonIDCALCULATE->EnableWindow(TRUE);
//
//  Case 1 - An entry on the Connections Table with hardwired distance
//
//  Try two ways through the table: from:to and to:from
//
  float distance = NO_RECORD;
  BOOL  bFound = FALSE;
  int   rcode2;

  CONNECTIONSKey1.fromNODESrecordID = fromNode;
  CONNECTIONSKey1.toNODESrecordID = toNode;
  CONNECTIONSKey1.fromTimeOfDay = NO_TIME;
  rcode2 = btrieve(B_GETGREATER, TMS_CONNECTIONS, &CONNECTIONS, &CONNECTIONSKey1, 1);
  if(rcode2 == 0)
  {
    if(CONNECTIONS.fromNODESrecordID == fromNode && 
          CONNECTIONS.toNODESrecordID == toNode)
    {
      if(CONNECTIONS.distance > 0.0)
      {
        bFound = TRUE;
      }
    }
  }
  if(!bFound)
  {
    CONNECTIONSKey1.fromNODESrecordID = toNode;
    CONNECTIONSKey1.toNODESrecordID = fromNode;
    CONNECTIONSKey1.fromTimeOfDay = NO_TIME;
    rcode2 = btrieve(B_GETGREATER, TMS_CONNECTIONS, &CONNECTIONS, &CONNECTIONSKey1, 1);
    if(rcode2 == 0)
    {
      if(CONNECTIONS.fromNODESrecordID == toNode && 
            CONNECTIONS.toNODESrecordID == fromNode)
      {
        if(CONNECTIONS.distance > 0.0)
        {
          bFound = TRUE;
        }
      }
    }
  }
//
//  Nope. Get the from and to nodes' lat/long
//
  if(!bFound)
  {
    NODESKey0.recordID = fromNode;
    rcode2 = btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
    if(rcode2 == 0 && NODES.latitude != 0.0 && NODES.longitude != 0.0)
    {
      longitude[0] = NODES.longitude;
      latitude[0]  = NODES.latitude;
      NODESKey0.recordID = toNode;
      rcode2 = btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
      if(rcode2 == 0 && NODES.latitude != 0.0 && NODES.longitude != 0.0)
      {
        longitude[1] = NODES.longitude;
        latitude[1]  = NODES.latitude;
        distance = (float)GreatCircleDistance(longitude[0], latitude[0], longitude[1], latitude[1]);
        bFound = TRUE;
      }
    }
  }
//
//  If we found one, set the control to its value
//
  if(bFound && distance > 0)
  {
    sprintf(tempString, "%10.6f", distance);
    pEditDISTANCE->SetWindowText(tempString);
  }
}

void CAddConnectionSD::OnSelchangeTonode() 
{
  OnSelchangeFromnode();
}

void CAddConnectionSD::OnChangeDistance() 
{
  if(pButtonIDCALCULATE->IsWindowEnabled())
  {
    OnCalculate();
  }
}

void CAddConnectionSD::OnAllroutes() 
{
  if(pButtonALLROUTES->GetCheck())
  {
    pComboBoxROUTE->SetCurSel(-1);
    pComboBoxROUTE->EnableWindow(FALSE);
    pButtonALLPATTERNS->SetCheck(TRUE);
    pComboBoxPATTERN->SetCurSel(-1);
    pComboBoxPATTERN->EnableWindow(FALSE);
  }
  else
  {
    int numRoutes = pComboBoxROUTE->GetCount();
    int nI;

    for(nI = 0; nI < numRoutes; nI++)
    {
      if((long)pComboBoxROUTE->GetItemData(nI) == m_pAddConnectionSDData->pDI->fileInfo.routeRecordID)
      {
        pComboBoxROUTE->SetCurSel(nI);
        break;
      }
    }
    pComboBoxROUTE->EnableWindow(TRUE);
  }
}

void CAddConnectionSD::OnAllservices() 
{
  if(pButtonALLSERVICES->GetCheck())
  {
    pComboBoxSERVICE->SetCurSel(-1);
    pComboBoxSERVICE->EnableWindow(FALSE);
    pButtonALLPATTERNS->SetCheck(TRUE);
    pComboBoxPATTERN->SetCurSel(-1);
    pComboBoxPATTERN->EnableWindow(FALSE);
  }
  else
  {
    int numServices = pComboBoxSERVICE->GetCount();
    int nI;

    for(nI = 0; nI < numServices; nI++)
    {
      if((long)pComboBoxSERVICE->GetItemData(nI) == m_pAddConnectionSDData->pDI->fileInfo.serviceRecordID)
      {
        pComboBoxSERVICE->SetCurSel(nI);
        break;
      }
    }
    pComboBoxSERVICE->EnableWindow(TRUE);
  }
}

void CAddConnectionSD::OnAllpatterns() 
{
  if(pButtonALLPATTERNS->GetCheck())
  {
    pComboBoxPATTERN->EnableWindow(FALSE);
  }
  else
  {
    long routeRecordID;
    long serviceRecordID;
    int  nI;

    nI = pComboBoxROUTE->GetCurSel();
    if(nI == CB_ERR)
    {
      return;
    }
    routeRecordID = pComboBoxROUTE->GetItemData(nI);

    nI = pComboBoxSERVICE->GetCurSel();
    if(nI == CB_ERR)
    {
      return;
    }
    serviceRecordID = pComboBoxSERVICE->GetItemData(nI);

    SetUpPatternList(m_hWnd, ADDCONNECTIONSD_PATTERN, routeRecordID, serviceRecordID, 0, TRUE, TRUE);
    SetUpPatternList(m_hWnd, ADDCONNECTIONSD_PATTERN, routeRecordID, serviceRecordID, 1, FALSE, TRUE);
    pComboBoxPATTERN->SetCurSel(0);
    pComboBoxPATTERN->EnableWindow(TRUE);
  }
  OnSelchangeFromnode();
}

//
//  OnSelchangePattern()
//
void CAddConnectionSD::OnSelchangePattern() 
{
  OnSelchangeFromnode();
}

void CAddConnectionSD::OnRoundup() 
{
  if(pButtonIDCALCULATE->IsWindowEnabled())
  {
    OnCalculate();
  }
}

void CAddConnectionSD::OnRounddown() 
{
  if(pButtonIDCALCULATE->IsWindowEnabled())
  {
    OnCalculate();
  }
}

void CAddConnectionSD::OnHalfminutes() 
{
  if(pButtonIDCALCULATE->IsWindowEnabled())
  {
    OnCalculate();
  }
}

void CAddConnectionSD::OnFullminutes() 
{
  if(pButtonIDCALCULATE->IsWindowEnabled())
  {
    OnCalculate();
  }
}

void CAddConnectionSD::OnCalculate() 
{
  CString s;
  short nI;
  float distance;
  float speed;
  BOOL  bRoundUp = pButtonROUNDUP->GetCheck();
  BOOL  bHalfMinutes = pButtonHALFMINUTES->GetCheck();
  float time;
  long  timeInSeconds, result, divisor;

  pEditDISTANCE->GetWindowText(s);
  distance = (float)atof(s);

  for(nI = 1; nI <= 96; nI++)
  {
    s = pSDIGridGRID->GetCellContents(nI, 1);
    if(s == "")
    {
      break;
    }
    s = pSDIGridGRID->GetCellContents(nI, 3);
    speed = (float)atof(s);
//
//  Detemine the time
//
    time = (float)((distance / speed) * 60);
    timeInSeconds = (long)(time * 60);
//
//  Round up/down and half minutes
//
    divisor = bHalfMinutes ? 30 : 60;
    result = timeInSeconds % divisor;
    if(bRoundUp)
    {
      timeInSeconds += (divisor - result);
    }
    else
    {
      timeInSeconds -= result;
    }
    s.Format("%ld%s", (long)timeInSeconds / 60, (timeInSeconds % 30 > 0 ? "+" : " "));
    pSDIGridGRID->SetCellContents(nI, 4, (LPCSTR)s);
  }
}

void CAddConnectionSD::OnCancel() 
{
	CDialog::OnCancel();
}

void CAddConnectionSD::OnHelp() 
{
}

void CAddConnectionSD::OnOK() 
{
  CString s;
  short   nI, nJ;
  float   distance;
  BOOL    bAddToList, bFound;
  long    connectionTime;
  long    fromNodeRecordID, toNodeRecordID;
  int     rcode2;
//
//  Cycle through all the rows
//
  for(nI = 1; nI <= 96; nI++)
  {
//
//  A blank from time of day means we're done
//
    s = pSDIGridGRID->GetCellContents(nI, 1);
    if(s == "")
    {
      break;
    }
//
//  Make this time of day and the next one upper case
//
    s.MakeUpper();
    pSDIGridGRID->SetCellContents(nI, 1, (LPCTSTR)s);
    s = pSDIGridGRID->GetCellContents(nI, 2);
    s.MakeUpper();
    pSDIGridGRID->SetCellContents(nI, 2, (LPCTSTR)s);
//
//  We're not interested in the average speed
//
//  Get the connection time
//
    s = pSDIGridGRID->GetCellContents(nI, 4);
    connectionTime = atol(s) * 60;
    if(s.Remove('+'))
    {
      connectionTime += 30;
    }
//
//  Get the from and to nodes
//
    nJ = pComboBoxFROMNODE->GetCurSel();
    if(nJ == CB_ERR)
    {
      return;
    }
    fromNodeRecordID = pComboBoxFROMNODE->GetItemData(nJ);
//
//  To Node
//
    nJ = pComboBoxTONODE->GetCurSel();
    if(nJ == CB_ERR)
    {
      return;
    }
    toNodeRecordID = pComboBoxTONODE->GetItemData(nJ);
//
//  Build the record
//
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
    pEditDISTANCE->GetWindowText(s);
    distance = (float)atof(s);
    CONNECTIONS.distance = distance;
//
//  From/to Route
//
    if(pButtonALLROUTES->GetCheck())
    {
      CONNECTIONS.fromROUTESrecordID = NO_RECORD;
      CONNECTIONS.toROUTESrecordID = NO_RECORD;
    }
    else
    {
      nJ = pComboBoxROUTE->GetCurSel();
      if(nJ == CB_ERR)
      {
        CONNECTIONS.fromROUTESrecordID = NO_RECORD;
        CONNECTIONS.toROUTESrecordID = NO_RECORD;
      }
      else
      {
        CONNECTIONS.fromROUTESrecordID = pComboBoxROUTE->GetItemData(nJ);
        CONNECTIONS.toROUTESrecordID = pComboBoxROUTE->GetItemData(nJ);
      }
    }
//
//  From/to Service
//
    if(pButtonALLSERVICES->GetCheck())
    {
      CONNECTIONS.fromSERVICESrecordID = NO_RECORD;
      CONNECTIONS.toSERVICESrecordID = NO_RECORD;
    }
    else
    {
      nJ = pComboBoxSERVICE->GetCurSel();
      if(nJ == CB_ERR)
      {
        CONNECTIONS.fromSERVICESrecordID = NO_RECORD;
        CONNECTIONS.toSERVICESrecordID = NO_RECORD;
      }
      else
      {
        CONNECTIONS.fromSERVICESrecordID = pComboBoxSERVICE->GetItemData(nJ);
        CONNECTIONS.toSERVICESrecordID = pComboBoxSERVICE->GetItemData(nJ);
      }
    }
//
//  From/to Pattern
//
    if(pButtonALLPATTERNS->GetCheck())
    {
      CONNECTIONS.fromPATTERNNAMESrecordID = NO_RECORD;
      CONNECTIONS.toPATTERNNAMESrecordID = NO_RECORD;
    }
    else
    {
      nJ = pComboBoxPATTERN->GetCurSel();
      if(nJ == CB_ERR)
      {
        CONNECTIONS.fromPATTERNNAMESrecordID = NO_RECORD;
        CONNECTIONS.toPATTERNNAMESrecordID = NO_RECORD;
      }
      else
      {
        CONNECTIONS.fromPATTERNNAMESrecordID = pComboBoxPATTERN->GetItemData(nJ);
        CONNECTIONS.toPATTERNNAMESrecordID = pComboBoxPATTERN->GetItemData(nJ);
      }
    }
//
//  From time of day
//
    s = pSDIGridGRID->GetCellContents(nI, 1);
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
    s = pSDIGridGRID->GetCellContents(nI, 2);
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
//  Forward pointer to ROUTINGS
//
    CONNECTIONS.ROUTINGSrecordID = NO_RECORD;
//
//  Insert the record
//
//  First, make sure there isn't any contention
//
    CONNECTIONSDef tempCONN;
    long newRecID = CONNECTIONS.recordID;
    int  counter = 0;

    while(counter < 500)
    {
      rcode2 = btrieve(B_GETLAST, TMS_CONNECTIONS, &tempCONN, &CONNECTIONSKey0, 0);
      CONNECTIONS.recordID = AssignRecID(rcode2, tempCONN.recordID);
      if(CONNECTIONS.recordID == newRecID)
      {
        rcode2 = btrieve(B_INSERT, TMS_CONNECTIONS, &CONNECTIONS, &CONNECTIONSKey0, 0);
        if(rcode2 == 0)
        {
          m_pAddConnectionSDData->recordIDs[nI - 1] = CONNECTIONS.recordID;
          m_pAddConnectionSDData->numRecs = nI;
          break;
        }
      }
      counter++;
      newRecID = tempCONN.recordID;
    }
//
//  Add it to the list, if necessary
//
    bFound = FALSE;
    if(bAddToList)
    {
      for(nJ = 0; nJ < CONN.numConnections; nJ++)
      {
        if((bFound = CONN.startRecordID[nJ] == CONNECTIONS.recordID) == TRUE)
        {
          break;
        }
      }
    }
    if(!bFound)
    {
      CONN.startRecordID[CONN.numConnections++] = CONNECTIONS.recordID;
    }
  }  // nI
//
//  Done
//
  m_bEstablishRUNTIMES = TRUE;

	CDialog::OnOK();
}

// Block.cpp : implementation file
//

#include "stdafx.h"
extern "C"{
#include "TMSHeader.h"
}
#include <math.h>
#include "tms.h"
#include "Block.h"

BOOL BlockDlg(PDISPLAYINFO);

extern "C" {

BOOL BlockDialog(PDISPLAYINFO pDI)
{
  return(BlockDlg(pDI));
}
}  // extern "C"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

BOOL BlockDlg(PDISPLAYINFO pDI)
{
  CBlock dlg(NULL, pDI);

  return(dlg.DoModal());
}

int sort_trips(const void *a, const void *b)
{
  TripDataDef *pa, *pb;
  pa = (TripDataDef *)a;
  pb = (TripDataDef *)b;
  return(pa->startTime < pb->startTime ? -1 : pa->startTime > pb->startTime ? 1 : 0);
}

int sort_blocks(const void *a, const void *b)
{
  TripDataDef *pa, *pb;
  pa = (TripDataDef *)a;
  pb = (TripDataDef *)b;

  if(pa->RGRPROUTESrecordID == pb->RGRPROUTESrecordID)
  {
    if(pa->blockNumber == pb->blockNumber)
    {
      return(pa->startTime > pb->startTime ? 1 : pa->startTime < pb->startTime ? -1 : 0);
    }
    else
    {
      return(pa->blockNumber > pb->blockNumber ? 1 : -1);
    }
  }
  else
  {
    return(pa->RGRPROUTESrecordID > pb->RGRPROUTESrecordID ? 1 : -1);
  }
}

/////////////////////////////////////////////////////////////////////////////
// CBlock dialog


CBlock::CBlock(CWnd* pParent, PDISPLAYINFO pDI)
	: CDialog(CBlock::IDD, pParent)
{
  m_pDI = pDI;

  m_bInitializing = TRUE;
  m_pTripData = NULL;

	//{{AFX_DATA_INIT(CBlock)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CBlock::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CBlock)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CBlock, CDialog)
	//{{AFX_MSG_MAP(CBlock)
	ON_WM_CLOSE()
	ON_BN_CLICKED(BLOCK_DEADHEADALL, OnDeadheadall)
	ON_BN_CLICKED(BLOCK_DEADHEADING, OnDeadheading)
	ON_BN_CLICKED(BLOCK_DEADHEADTHOSE, OnDeadheadthose)
	ON_BN_CLICKED(BLOCK_INTERLINING, OnInterlining)
	ON_NOTIFY(NM_DBLCLK, BLOCK_TRIPS, OnDblclkTrips)
	ON_NOTIFY(NM_CLICK, BLOCK_BLOCKS, OnClickBlocks)
	ON_NOTIFY(NM_DBLCLK, BLOCK_BLOCKS, OnDblclkBlocks)
	ON_NOTIFY(NM_RCLICK, BLOCK_BLOCKS, OnRclickBlocks)
	ON_NOTIFY(NM_DBLCLK, BLOCK_PARTNERS, OnDblclkPartners)
	ON_BN_CLICKED(IDLAYOVER, OnLayover)
	ON_BN_CLICKED(IDSAVE, OnSave)
	ON_BN_CLICKED(IDHELP, OnHelp)
	ON_COMMAND(IDNEWBLOCK, OnNewblock)
	ON_COMMAND(IDADD, OnAdd)
	ON_COMMAND(IDREMOVE, OnRemove)
	ON_COMMAND(IDUNHOOKBLOCK, OnUnhookblock)
	ON_COMMAND(IDRENUMBER, OnRenumber)
	ON_COMMAND(IDSUMMARY, OnSummary)
	ON_COMMAND(IDTAKETRIP, OnTaketrip)
	ON_EN_CHANGE(BLOCK_DEADHEADMINS, OnChangeDeadheadmins)
	ON_WM_CONTEXTMENU()
	ON_BN_CLICKED(BLOCK_SERVICEWRAP, OnServicewrap)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBlock message handlers

BOOL CBlock::OnInitDialog() 
{
	CDialog::OnInitDialog();

//
//  Set up pointers to the controls
//	
  pListCtrlTRIPS = (CListCtrl *)GetDlgItem(BLOCK_TRIPS);
  pListCtrlBLOCKS = (CListCtrl *)GetDlgItem(BLOCK_BLOCKS);
  pListCtrlPARTNERS = (CListCtrl *)GetDlgItem(BLOCK_PARTNERS);
  pButtonDEADHEADING = (CButton *)GetDlgItem(BLOCK_DEADHEADING);
  pButtonINTERLINING = (CButton *)GetDlgItem(BLOCK_INTERLINING);
  pButtonSERVICEWRAP = (CButton *)GetDlgItem(BLOCK_SERVICEWRAP);
  pButtonDEADHEADALL = (CButton *)GetDlgItem(BLOCK_DEADHEADALL);
  pEditDEADHEADMINS = (CEdit *)GetDlgItem(BLOCK_DEADHEADMINS);
  pButtonDEADHEADTHOSE = (CButton *)GetDlgItem(BLOCK_DEADHEADTHOSE);
//
//  Extend the listbox controls
//
  DWORD dwExStyles = pListCtrlTRIPS->GetExtendedStyle();
  pListCtrlTRIPS->SetExtendedStyle(dwExStyles | LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP); // | LVS_EX_GRIDLINES);

  dwExStyles = pListCtrlBLOCKS->GetExtendedStyle();
  pListCtrlBLOCKS->SetExtendedStyle(dwExStyles | LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP); // | LVS_EX_GRIDLINES);

  dwExStyles = pListCtrlPARTNERS->GetExtendedStyle();
  pListCtrlPARTNERS->SetExtendedStyle(dwExStyles | LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP); // | LVS_EX_GRIDLINES);

  if(m_pDI == NULL)
  {
    OnCancel();
    return TRUE;
  }

  m_pPARMS = (PARMSDef *)(m_pDI->otherData);

  if(m_pPARMS == NULL)
  {
    OnCancel();
    return TRUE;
  }
//
//  Allocate space for m_pTripData
//
  HCURSOR hSaveCursor = SetCursor(hCursorWait);

  m_maxTrips = 1500;
  m_pTripData = (TripDataDef *)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, (DWORD)m_maxTrips * sizeof(TripDataDef));
  if(m_pTripData == NULL)
  {
    AllocationError(__FILE__, __LINE__, FALSE);
    SetCursor(hSaveCursor);
    OnCancel();
    return TRUE;
  }
//
//  Set up the trip data - PICKEDTRIPS is set up in addblock()
//
  GenerateTripDef GTResults;
  long minutes;
  long percent;
  int  nI;
  int  rcode2;

  m_pTRIPSChunk  = m_pPARMS->bStandard ? &TRIPS.standard : &TRIPS.dropback;
  m_bEstablishRUNTIMES = TRUE;
  m_numConnectionAlerts = 0;
  for(m_numTrips = 0, nI = 0; nI < m_numPICKEDTRIPS; nI++)
  {
    TRIPSKey1.ROUTESrecordID = m_pPICKEDTRIPS[nI].ROUTESrecordID;
    TRIPSKey1.SERVICESrecordID = m_pPICKEDTRIPS[nI].SERVICESrecordID;
    TRIPSKey1.directionIndex = NO_RECORD;
    TRIPSKey1.tripSequence = NO_RECORD;
    rcode2 = btrieve(B_GETGREATEROREQUAL, TMS_TRIPS, &TRIPS, &TRIPSKey1, 1);
    while(rcode2 == 0 &&
          (m_pPICKEDTRIPS[nI].ROUTESrecordID == NO_RECORD ||
                TRIPS.ROUTESrecordID == m_pPICKEDTRIPS[nI].ROUTESrecordID) &&
          (m_pPICKEDTRIPS[nI].SERVICESrecordID == NO_RECORD ||
                TRIPS.SERVICESrecordID == m_pPICKEDTRIPS[nI].SERVICESrecordID))
    {
      if(m_numTrips >= m_maxTrips)
      {
        m_maxTrips += 1000;
        m_pTripData = (TripDataDef *)HeapReAlloc(GetProcessHeap(),
              HEAP_ZERO_MEMORY, m_pTripData, (DWORD)m_maxTrips * sizeof(TripDataDef));
        if(m_pTripData == NULL)
        {
          AllocationError(__FILE__, __LINE__, FALSE);
          SetCursor(hSaveCursor);
          OnCancel();
          return TRUE;
        }
      }
      m_pTripData[m_numTrips].TRIPSrecordID = TRIPS.recordID;
//
//  Generate the trip
//
      GenerateTrip(TRIPS.ROUTESrecordID, TRIPS.SERVICESrecordID,
            TRIPS.directionIndex, TRIPS.PATTERNNAMESrecordID,
            TRIPS.timeAtMLP, GENERATETRIP_FLAG_DISPLAYERRORS, &GTResults);
//
//  RGRP
//
      m_pTripData[m_numTrips].RGRPROUTESrecordID = m_pTRIPSChunk->RGRPROUTESrecordID;
      if(m_pTripData[m_numTrips].RGRPROUTESrecordID == NO_RECORD)
      {
        strcpy(m_pTripData[m_numTrips].szRGRPRouteName, "");
      }
      else
      {
        ROUTESKey0.recordID = m_pTripData[m_numTrips].RGRPROUTESrecordID;
        btrieve(B_GETEQUAL, TMS_ROUTES, &ROUTES, &ROUTESKey0, 0);
        strncpy(m_pTripData[m_numTrips].szRGRPRouteName, ROUTES.number, ROUTES_NUMBER_LENGTH);
        trim(m_pTripData[m_numTrips].szRGRPRouteName, ROUTES_NUMBER_LENGTH);
      }
//
//  Route
//
      m_pTripData[m_numTrips].ROUTESrecordID = TRIPS.ROUTESrecordID;
      ROUTESKey0.recordID = m_pTripData[m_numTrips].ROUTESrecordID;
      btrieve(B_GETEQUAL, TMS_ROUTES, &ROUTES, &ROUTESKey0, 0);
      strncpy(m_pTripData[m_numTrips].szRouteName, ROUTES.number, ROUTES_NUMBER_LENGTH);
      trim(m_pTripData[m_numTrips].szRouteName, ROUTES_NUMBER_LENGTH);
//
//  Service
//
      m_pTripData[m_numTrips].SERVICESrecordID = TRIPS.SERVICESrecordID;
      SERVICESKey0.recordID = m_pTripData[m_numTrips].SERVICESrecordID;
      btrieve(B_GETEQUAL, TMS_SERVICES, &SERVICES, &SERVICESKey0, 0);
      strncpy(m_pTripData[m_numTrips].szServiceName, SERVICES.name, SERVICES_NAME_LENGTH);
      trim(m_pTripData[m_numTrips].szServiceName, SERVICES_NAME_LENGTH);
//
//  Direction
//
      m_pTripData[m_numTrips].directionIndex = TRIPS.directionIndex;
      DIRECTIONSKey0.recordID = ROUTES.DIRECTIONSrecordID[m_pTripData[m_numTrips].directionIndex];
      btrieve(B_GETEQUAL, TMS_DIRECTIONS, &DIRECTIONS, &DIRECTIONSKey0, 0);
      strncpy(m_pTripData[m_numTrips].szDirectionAbbrName, DIRECTIONS.abbrName, DIRECTIONS_ABBRNAME_LENGTH);
      trim(m_pTripData[m_numTrips].szDirectionAbbrName, DIRECTIONS_ABBRNAME_LENGTH);
//
//  Pattern name
//
      m_pTripData[m_numTrips].PATTERNNAMESrecordID = TRIPS.PATTERNNAMESrecordID;
      PATTERNNAMESKey0.recordID = m_pTripData[m_numTrips].PATTERNNAMESrecordID;
      btrieve(B_GETEQUAL, TMS_PATTERNNAMES, &PATTERNNAMES, &PATTERNNAMESKey0, 0);
      strncpy(m_pTripData[m_numTrips].szPatternName, PATTERNNAMES.name, PATTERNNAMES_NAME_LENGTH);
      trim(m_pTripData[m_numTrips].szPatternName, PATTERNNAMES_NAME_LENGTH);
//
//  Vehicle
//
      m_pTripData[m_numTrips].BUSTYPESrecordID = TRIPS.BUSTYPESrecordID;
      if(m_pTripData[m_numTrips].BUSTYPESrecordID == NO_RECORD)
      {
        strcpy(tempString, " ");
      }
      else
      {
        BUSTYPESKey0.recordID = m_pTripData[m_numTrips].BUSTYPESrecordID;
        btrieve(B_GETEQUAL, TMS_BUSTYPES, &BUSTYPES, &BUSTYPESKey0, 0);
        strncpy(m_pTripData[m_numTrips].szBustypeName, BUSTYPES.name, BUSTYPES_NAME_LENGTH);
        trim(m_pTripData[m_numTrips].szBustypeName, BUSTYPES_NAME_LENGTH);
      }
//
//  Start node
//
      m_pTripData[m_numTrips].startNODESrecordID = GTResults.firstNODESrecordID;
      NODESKey0.recordID = m_pTripData[m_numTrips].startNODESrecordID;
      btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
      strncpy(m_pTripData[m_numTrips].szStartNodeAbbrName, NODES.abbrName, NODES_ABBRNAME_LENGTH);
      trim(m_pTripData[m_numTrips].szStartNodeAbbrName, NODES_ABBRNAME_LENGTH);
//
//  Start time
//
      m_pTripData[m_numTrips].startTime = GTResults.firstNodeTime;
//
//  Last time
//
      m_pTripData[m_numTrips].endTime = GTResults.lastNodeTime;
//
//  Last node
//
      m_pTripData[m_numTrips].endNODESrecordID = GTResults.lastNODESrecordID;
      NODESKey0.recordID = m_pTripData[m_numTrips].endNODESrecordID;
      btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
      strncpy(m_pTripData[m_numTrips].szEndNodeAbbrName, NODES.abbrName, NODES_ABBRNAME_LENGTH);
      trim(m_pTripData[m_numTrips].szEndNodeAbbrName, NODES_ABBRNAME_LENGTH);
//
//  Block number
//
      m_pTripData[m_numTrips].blockNumber = m_pTRIPSChunk->blockNumber;
//
//  Internally maintained layover and deadhead
//
      m_pTripData[m_numTrips].lay = NO_TIME;
      m_pTripData[m_numTrips].dhd = NO_TIME;
//
//  Flags
//
      m_pTripData[m_numTrips].flags = TRIPS.flags;
//
//  Index back into m_pTripData
//
      m_pTripData[m_numTrips].indexIntoTripData = NO_RECORD;  // Filled in after the sort
//
//  Which layover are we pointing to?
//
      m_pLayover = m_pPARMS->bStandard ? &StandardLayover : &DropbackLayover;
//
//  Figure out the minimum layover
//
      if(m_pTRIPSChunk->layoverMin != NO_TIME)
      {
        m_pTripData[m_numTrips].layoverMin = m_pTRIPSChunk->layoverMin;
      }
      else
      {
        minutes = m_pLayover->minimum.minutes  * 60;
        percent = (GTResults.lastNodeTime - GTResults.firstNodeTime) * m_pLayover->minimum.percent / 100;
        if(m_pLayover->minimum.type == 0)
        {
          m_pTripData[m_numTrips].layoverMin = minutes;
        }
        else if(m_pLayover->minimum.type == 1)
        {
          m_pTripData[m_numTrips].layoverMin = percent;
        }
        else if(m_pLayover->minimum.type == 2)
        {
          m_pTripData[m_numTrips].layoverMin = m_pLayover->minimum.lower ? min(minutes, percent) : max(minutes, percent);
        }
      }
//
//  Figure out the maximum layover
//
      if(m_pTRIPSChunk->layoverMax != NO_TIME)
      {
        m_pTripData[m_numTrips].layoverMax = m_pTRIPSChunk->layoverMax;
      }
      else
      {
        minutes = m_pLayover->maximum.minutes * 60;
        percent = (GTResults.lastNodeTime - GTResults.firstNodeTime) * m_pLayover->maximum.percent / 100;
        if(m_pLayover->maximum.type == 0)
        {
          m_pTripData[m_numTrips].layoverMax = minutes;
        }
        else if(m_pLayover->maximum.type == 1)
        {
          m_pTripData[m_numTrips].layoverMax = percent;
        }
        else if(m_pLayover->maximum.type == 2)
        {
          m_pTripData[m_numTrips].layoverMax = m_pLayover->maximum.lower ? min(minutes, percent) : max(minutes, percent);
        }
      }
      m_numTrips++;
      rcode2 = btrieve(B_GETNEXT, TMS_TRIPS, &TRIPS, &TRIPSKey1, 1);
    }
  }
  SetCursor(hSaveCursor);
//
//  Now, allocate space for m_pBlockData
//
  m_pBlockData = (TripDataDef *)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, (DWORD)m_numTrips * sizeof(TripDataDef));
  if(m_pBlockData == NULL)
  {
    AllocationError(__FILE__, __LINE__, FALSE);
    OnCancel();
    return TRUE;
  }
//
//  If m_numConnectionAlerts is non-zero, blow him off
//
  if(m_numConnectionAlerts != 0)
  {
    TMSError(m_hWnd, MB_ICONSTOP, ERROR_173, (HANDLE)NULL);
    OnCancel();
    return TRUE;
  }
//
//  Reflect the parameter settings onto the dialog
//
  m_bDeadheading = m_pPARMS->bDeadheading;
  m_bInterlining = m_pPARMS->bInterlining;
  m_bServiceWrap = m_pPARMS->bServiceWrap;

  if(m_bDeadheading)
  {
    pButtonDEADHEADING->SetCheck(TRUE);
  }
  if(m_bInterlining)
  {
    pButtonINTERLINING->SetCheck(TRUE);
  }
  if(m_bServiceWrap)
  {
    pButtonSERVICEWRAP->SetCheck(TRUE);
  }
  if(m_pPARMS->deadheadMins == NO_TIME)
  {
    pButtonDEADHEADALL->SetCheck(TRUE);
    pEditDEADHEADMINS->SetWindowText("");
  }
  else
  {
    pButtonDEADHEADTHOSE->SetCheck(TRUE);
    sprintf(tempString, "%ld", m_pPARMS->deadheadMins);
    pEditDEADHEADMINS->SetWindowText(tempString);
  }
//
//  Set the alerts to 0
//
  m_numConnectionAlerts = 0;
//
//  Set up the handles to the popup menus
//
  m_hMenu1 = LoadMenu(hInst, MAKEINTRESOURCE(IDR_MANBLOCK1));
  m_hMenu1 = GetSubMenu(m_hMenu1, 0);
  m_hMenu2 = LoadMenu(hInst, MAKEINTRESOURCE(IDR_MANBLOCK2));
  m_hMenu2 = GetSubMenu(m_hMenu2, 0);
  m_hMenu3 = LoadMenu(hInst, MAKEINTRESOURCE(IDR_MANBLOCK3));
  m_hMenu3 = GetSubMenu(m_hMenu3, 0);
//
//  Set up the list control titles
//
//  Trips
//
  LVCOLUMN LVC;
  CString  s;

  LVC.mask = LVCF_FMT | LVCF_TEXT | LVCF_WIDTH;
  LVC.fmt = LVCFMT_CENTER;
//
//  "Vehicle"
//
  LVC.cx = 100;
  s.LoadString(TEXT_342);
  strcpy(tempString, s);
  LVC.pszText = tempString;
  pListCtrlTRIPS->InsertColumn(0, &LVC);
//
//  "Route"
//
  LVC.cx = 50;
  s.LoadString(TEXT_343);
  strcpy(tempString, s);
  LVC.pszText = tempString;
  pListCtrlTRIPS->InsertColumn(1, &LVC);
//
//  "Service"
//
  LVC.cx = 100;
  s.LoadString(TEXT_344);
  strcpy(tempString, s);
  LVC.pszText = tempString;
  pListCtrlTRIPS->InsertColumn(2, &LVC);
//
//  "Dir"
//
  LVC.cx = 40;
  s.LoadString(TEXT_345);
  strcpy(tempString, s);
  LVC.pszText = tempString;
  pListCtrlTRIPS->InsertColumn(3, &LVC);
//
//  "Pattern"
//
  LVC.cx = 100;
  s.LoadString(TEXT_346);
  strcpy(tempString, s);
  LVC.pszText = tempString;
  pListCtrlTRIPS->InsertColumn(4, &LVC);
//
//  "FNode"
//
  LVC.cx = 50;
  s.LoadString(TEXT_347);
  strcpy(tempString, s);
  LVC.pszText = tempString;
  pListCtrlTRIPS->InsertColumn(5, &LVC);
//
//  "FTime"
//
  LVC.cx = 50;
  s.LoadString(TEXT_348);
  strcpy(tempString, s);
  LVC.pszText = tempString;
  pListCtrlTRIPS->InsertColumn(6, &LVC);
//
//  "TTime"
//
  LVC.cx = 50;
  s.LoadString(TEXT_349);
  strcpy(tempString, s);
  LVC.pszText = tempString;
  pListCtrlTRIPS->InsertColumn(7, &LVC);
//
//  "TNode"
//
  LVC.cx = 50;
  s.LoadString(TEXT_350);
  strcpy(tempString, s);
  LVC.pszText = tempString;
  pListCtrlTRIPS->InsertColumn(8, &LVC);
//
//  Blocks
//
//  "RGRP"
//
  LVC.cx = 50;
  s.LoadString(TEXT_351);
  strcpy(tempString, s);
  LVC.pszText = tempString;
  pListCtrlBLOCKS->InsertColumn(0, &LVC);
//
//  "Block"
//
  LVC.cx = 50;
  s.LoadString(TEXT_352);
  strcpy(tempString, s);
  LVC.pszText = tempString;
  pListCtrlBLOCKS->InsertColumn(1, &LVC);
//
//  "Vehicle"
//
  LVC.cx = 100;
  s.LoadString(TEXT_353);
  strcpy(tempString, s);
  LVC.pszText = tempString;
  pListCtrlBLOCKS->InsertColumn(2, &LVC);
//
//  "Route"
//
  LVC.cx = 50;
  s.LoadString(TEXT_354);
  strcpy(tempString, s);
  LVC.pszText = tempString;
  pListCtrlBLOCKS->InsertColumn(3, &LVC);
//
//  "Service"
//
  LVC.cx = 100;
  s.LoadString(TEXT_355);
  strcpy(tempString, s);
  LVC.pszText = tempString;
  pListCtrlBLOCKS->InsertColumn(4, &LVC);
//
//  "Dir"
//
  LVC.cx = 40;
  s.LoadString(TEXT_356);
  strcpy(tempString, s);
  LVC.pszText = tempString;
  pListCtrlBLOCKS->InsertColumn(5, &LVC);
//
//  "Pattern"
//
  LVC.cx = 100;
  s.LoadString(TEXT_374);
  strcpy(tempString, s);
  LVC.pszText = tempString;
  pListCtrlBLOCKS->InsertColumn(6, &LVC);
//
//  "FNode"
//
  LVC.cx = 50;
  s.LoadString(TEXT_357);
  strcpy(tempString, s);
  LVC.pszText = tempString;
  pListCtrlBLOCKS->InsertColumn(7, &LVC);
//
//  "FTime"
//
  LVC.cx = 50;
  s.LoadString(TEXT_358);
  strcpy(tempString, s);
  LVC.pszText = tempString;
  pListCtrlBLOCKS->InsertColumn(8, &LVC);
//
//  "TTime"
//
  LVC.cx = 50;
  s.LoadString(TEXT_359);
  strcpy(tempString, s);
  LVC.pszText = tempString;
  pListCtrlBLOCKS->InsertColumn(9, &LVC);
//
//  "TNode"
//
  LVC.cx = 50;
  s.LoadString(TEXT_360);
  strcpy(tempString, s);
  LVC.pszText = tempString;
  pListCtrlBLOCKS->InsertColumn(10, &LVC);
//
//  "Lay"
//
  LVC.cx = 40;
  s.LoadString(TEXT_361);
  strcpy(tempString, s);
  LVC.pszText = tempString;
  pListCtrlBLOCKS->InsertColumn(11, &LVC);
//
//  "Dhd"
//
  LVC.cx = 40;
  s.LoadString(TEXT_362);
  strcpy(tempString, s);
  LVC.pszText = tempString;
  pListCtrlBLOCKS->InsertColumn(12, &LVC);
//
//  Partners
//
//  "Vehicle"
//
  LVC.cx = 100;
  s.LoadString(TEXT_365);
  strcpy(tempString, s);
  LVC.pszText = tempString;
  pListCtrlPARTNERS->InsertColumn(0, &LVC);
//
//  "Route"
//
  LVC.cx = 50;
  s.LoadString(TEXT_366);
  strcpy(tempString, s);
  LVC.pszText = tempString;
  pListCtrlPARTNERS->InsertColumn(1, &LVC);
//
//  "Service"
//
  LVC.cx = 100;
  s.LoadString(TEXT_367);
  strcpy(tempString, s);
  LVC.pszText = tempString;
  pListCtrlPARTNERS->InsertColumn(2, &LVC);
//
//  "Dir"
//
  LVC.cx = 40;
  s.LoadString(TEXT_368);
  strcpy(tempString, s);
  LVC.pszText = tempString;
  pListCtrlPARTNERS->InsertColumn(3, &LVC);
//
//  "Pattern"
//
  LVC.cx = 100;
  s.LoadString(TEXT_369);
  strcpy(tempString, s);
  LVC.pszText = tempString;
  pListCtrlPARTNERS->InsertColumn(4, &LVC);
//
//  "FNode"
//
  LVC.cx = 50;
  s.LoadString(TEXT_370);
  strcpy(tempString, s);
  LVC.pszText = tempString;
  pListCtrlPARTNERS->InsertColumn(5, &LVC);
//
//  "FTime"
//
  LVC.cx = 50;
  s.LoadString(TEXT_371);
  strcpy(tempString, s);
  LVC.pszText = tempString;
  pListCtrlPARTNERS->InsertColumn(6, &LVC);
//
//  "TTime"
//
  LVC.cx = 50;
  s.LoadString(TEXT_372);
  strcpy(tempString, s);
  LVC.pszText = tempString;
  pListCtrlPARTNERS->InsertColumn(7, &LVC);
//
//  "TNode"
//
  LVC.cx = 50;
  s.LoadString(TEXT_373);
  strcpy(tempString, s);
  LVC.pszText = tempString;
  pListCtrlPARTNERS->InsertColumn(8, &LVC);
//
//  "Lay"
//
  LVC.cx = 40;
  s.LoadString(TEXT_363);
  strcpy(tempString, s);
  LVC.pszText = tempString;
  pListCtrlPARTNERS->InsertColumn(9, &LVC);
//
//  "Dhd"
//
  LVC.cx = 40;
  s.LoadString(TEXT_364);
  strcpy(tempString, s);
  LVC.pszText = tempString;
  pListCtrlPARTNERS->InsertColumn(10, &LVC);
//
//  Sort and display the trip data
//
  qsort((void *)m_pTripData, m_numTrips, sizeof(TripDataDef), sort_trips);
  for(nI = 0; nI < m_numTrips; nI++)
  {
    m_pTripData[nI].indexIntoTripData = nI;
  }
  DrawTrips(NO_RECORD);
//
//  Display the blocks
//
  DrawBlocks(TRUE, 0);
//
//  Get the starting block number
//
  m_lastBlockNumber = GetNewBlockNumber(m_pDI);

  m_bInitializing = FALSE;

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CBlock::OnContextMenu(CWnd* pWnd, CPoint point) 
{
  HMENU hMenu = NULL;
  BOOL  bEnable;

  if(pWnd == pListCtrlTRIPS)
  {
    hMenu = m_hMenu1;
    if(pListCtrlTRIPS->GetNextItem(-1, LVNI_ALL | LVNI_SELECTED) < 0)
    {
      EnableMenuItem(hMenu, 0, MF_BYPOSITION	| MF_GRAYED);  // Start new block
      EnableMenuItem(hMenu, 1, MF_BYPOSITION	| MF_GRAYED);  // Add to block
    }
    else
    {
      EnableMenuItem(hMenu, 0, MF_BYPOSITION	| MF_ENABLED);
      if(pListCtrlBLOCKS->GetNextItem(-1, LVNI_ALL | LVNI_SELECTED) < 0)
      {
        EnableMenuItem(hMenu, 1, MF_BYPOSITION	| MF_GRAYED);  // Add to block
      }
      else
      {
        EnableMenuItem(hMenu, 1, MF_BYPOSITION	| MF_ENABLED);
      }
    }
  }
  else if(pWnd == pListCtrlBLOCKS)
  {
    hMenu = m_hMenu2;
    if(pListCtrlBLOCKS->GetNextItem(-1, LVNI_ALL | LVNI_SELECTED) < 0)
    {
      bEnable = MF_BYPOSITION | MF_GRAYED;
    }
    else
    {
      bEnable = MF_BYPOSITION | MF_ENABLED;
    }
    EnableMenuItem(hMenu, 0, bEnable);  // Remove this trip from the block
    EnableMenuItem(hMenu, 1, bEnable);  // Unhook all trips in this block
    EnableMenuItem(hMenu, 2, bEnable);  // Renumber this block
    EnableMenuItem(hMenu, 3, bEnable);  // Summarize this entire block
  }
  else if(pWnd == pListCtrlPARTNERS)
  {
    hMenu = m_hMenu3;
    if(pListCtrlPARTNERS->GetNextItem(-1, LVNI_ALL | LVNI_SELECTED) < 0)
    {
      EnableMenuItem(hMenu, 0, MF_BYPOSITION	| MF_GRAYED);  // Take trip
    }
    else
    {
      EnableMenuItem(hMenu, 0, MF_BYPOSITION	| MF_ENABLED);
    }
  }
  if(hMenu != NULL)
  {
    TrackPopupMenu(hMenu, 0, point.x, point.y, 0, m_hWnd, NULL);
  }
}

//
//  DrawTrips() - Put trip(s) in the TRIPS box.
//
void CBlock::DrawTrips(int indexToDisplay)
{
  LVITEM LVI;
  int    displayPosition = NO_RECORD;
  int    pos;
  int    nI;

//
//  Kill any existing entries
//
//  pListCtrlTRIPS->DeleteAllItems();

  int numInTrips = pListCtrlTRIPS->GetItemCount();

  for(nI = 0; nI < numInTrips; nI++)
  {
    pListCtrlTRIPS->DeleteItem(0);
  }
//
//  Build the display
//
  pos = 0;
  for(nI = 0; nI < m_numTrips; nI++)
  {
    if(m_pTripData[nI].blockNumber > 0)
    {
      continue;
    }
    if(m_pTripData[nI].flags & TRIPDATA_FLAG_PARTNER)
    {
      continue;
    }
    if(m_pTripData[nI].flags & TRIPDATA_FLAG_SERVICEWRAP_PLUS)
    {
      m_pTripData[nI].startTime -= 86400;
      m_pTripData[nI].endTime -= 86400;
      m_pTripData[nI].flags -= TRIPDATA_FLAG_SERVICEWRAP_PLUS;
    }
    else if(m_pTripData[nI].flags & TRIPDATA_FLAG_SERVICEWRAP_MINUS)
    {
      m_pTripData[nI].startTime += 86400;
      m_pTripData[nI].endTime += 86400;
      m_pTripData[nI].flags -= TRIPDATA_FLAG_SERVICEWRAP_MINUS;
    }
    if(nI == indexToDisplay)
    {
      displayPosition = pos;
    }
//
//  Vehicle
//
    LVI.mask = LVIF_PARAM | LVIF_TEXT;
    LVI.iItem = pos;
    LVI.iSubItem = 0;
    LVI.lParam = nI;
    LVI.pszText = m_pTripData[nI].szBustypeName;
    LVI.iItem = pListCtrlTRIPS->InsertItem(&LVI);
//
//  Route
//
    LVI.mask = LVIF_TEXT;
    LVI.iSubItem = 1;
    LVI.pszText = m_pTripData[nI].szRouteName;
    pListCtrlTRIPS->SetItem(&LVI);
//
//  Service
//
    LVI.mask = LVIF_TEXT;
    LVI.iSubItem = 2;
    LVI.pszText = m_pTripData[nI].szServiceName;
    pListCtrlTRIPS->SetItem(&LVI);
//
//  Direction
//
    LVI.mask = LVIF_TEXT;
    LVI.iSubItem = 3;
    LVI.pszText = m_pTripData[nI].szDirectionAbbrName;
    pListCtrlTRIPS->SetItem(&LVI);
//
//  Pattern name
//
    LVI.mask = LVIF_TEXT;
    LVI.iSubItem = 4;
    LVI.pszText = m_pTripData[nI].szPatternName;
    pListCtrlTRIPS->SetItem(&LVI);
//
//  From node
//
    LVI.mask = LVIF_TEXT;
    LVI.iSubItem = 5;
    LVI.pszText = m_pTripData[nI].szStartNodeAbbrName;
    pListCtrlTRIPS->SetItem(&LVI);
//
//  From time
//
    strcpy(tempString, Tchar(m_pTripData[nI].startTime));
    LVI.mask = LVIF_TEXT;
    LVI.iSubItem = 6;
    LVI.pszText = tempString;
    pListCtrlTRIPS->SetItem(&LVI);
//
//  To time
//
    strcpy(tempString, Tchar(m_pTripData[nI].endTime));
    LVI.mask = LVIF_TEXT;
    LVI.iSubItem = 7;
    LVI.pszText = tempString;
    pListCtrlTRIPS->SetItem(&LVI);
//
//  To node
//
    LVI.mask = LVIF_TEXT;
    LVI.iSubItem = 8;
    LVI.pszText = m_pTripData[nI].szEndNodeAbbrName;
    pListCtrlTRIPS->SetItem(&LVI);
//
//  Done - increment pos
//
    pos++;
  }
//
//  Display a specific item?
//
  if(displayPosition != NO_RECORD)
  {
    pListCtrlTRIPS->EnsureVisible(displayPosition, FALSE);
  }
}

//
//  DrawBlocks() - Put trip(s) in the BLOCKS box
//
void CBlock::DrawBlocks(BOOL bRebuild, int indexToDisplay)
{
  LVITEM LVI;
  long   blockNumber = m_pTripData[indexToDisplay].blockNumber;
  int    nI;
  int    reposition;

//
//  Are we rebuilding the entire list?
//
  if(bRebuild)
  {
//
//  Traverse the trip data and move any trips that are blocked out to m_pBlockData.
//
    m_numBlockedTrips = 0;
    for(nI = 0; nI < m_numTrips; nI++)
    {
      if(m_pTripData[nI].blockNumber > 0)
      {
        memcpy(&m_pBlockData[m_numBlockedTrips], &m_pTripData[nI], sizeof(TripDataDef));
//        if(m_pBlockData[m_numBlockedTrips].flags & TRIPDATA_FLAG_SERVICEWRAP_PLUS)
//        {
//          m_pBlockData[m_numBlockedTrips].startTime += 86400;
//          m_pBlockData[m_numBlockedTrips].endTime += 86400;
//        }
//        else if(m_pBlockData[m_numBlockedTrips].flags & TRIPDATA_FLAG_SERVICEWRAP_MINUS)
//        {
//          m_pBlockData[m_numBlockedTrips].startTime -= 86400;
//          m_pBlockData[m_numBlockedTrips].endTime -= 86400;
//        }
        m_numBlockedTrips++;
      }
    } 
//
//  Delete all existing items
//
//    pListCtrlBLOCKS->DeleteAllItems();
    int numInBlocks = pListCtrlBLOCKS->GetItemCount();

    for(nI = 0; nI < numInBlocks; nI++)
    {
      pListCtrlBLOCKS->DeleteItem(0);
    }
//
//  And sort
//
    qsort((void *)m_pBlockData, m_numBlockedTrips, sizeof(TripDataDef), sort_blocks);
//
//  Set the layover and deadhead times
//
    EstablishDhdAndLay(NO_RECORD);
  }
//
//  No rebuild  
//
  else
  {
//
//  Add it to the list
//
    memcpy(&m_pBlockData[m_numBlockedTrips], &m_pTripData[indexToDisplay], sizeof(TripDataDef));
//    if(m_pBlockData[m_numBlockedTrips].flags & TRIPDATA_FLAG_SERVICEWRAP_PLUS)
//    {
//      m_pBlockData[m_numBlockedTrips].startTime += 86400;
//      m_pBlockData[m_numBlockedTrips].endTime += 86400;
//    }
//    else if(m_pBlockData[m_numBlockedTrips].flags & TRIPDATA_FLAG_SERVICEWRAP_MINUS)
//    {
//      m_pBlockData[m_numBlockedTrips].startTime -= 86400;
//      m_pBlockData[m_numBlockedTrips].endTime -= 86400;
//    }
    m_numBlockedTrips++;
//
//  And sort
//
    qsort((void *)m_pBlockData, m_numBlockedTrips, sizeof(TripDataDef), sort_blocks);
//
//  Set the layover and deadhead times
//
    EstablishDhdAndLay(blockNumber);
  }
//
//  Now display
//
  int displayRecord = 0;

//
//  Are we (re)displaying the whole list, or just a part?
//
  int nImin;

//  if(bRebuild)
//  {
    nImin = 0;
//  }
//  else
//  {
//    nImin = m_numBlockedTrips - 1;
//  }
//
//  Cycle through
//
  for(nI = nImin; nI < m_numBlockedTrips; nI++)
  {
//
//  If the time was adjusted for the sort, set it back
//
    if(bRebuild || m_pBlockData[nI].indexIntoTripData == indexToDisplay)
    {
//
//  RGRP
//
      LVI.mask = LVIF_PARAM | LVIF_TEXT;
      LVI.iItem = nI;
      LVI.iSubItem = 0;
      LVI.lParam = m_pBlockData[nI].indexIntoTripData;
      LVI.pszText = m_pBlockData[nI].szRGRPRouteName;
      LVI.iItem = pListCtrlBLOCKS->InsertItem(&LVI);
//
//  Block number
//
      sprintf(tempString, "%ld", m_pBlockData[nI].blockNumber);
      LVI.mask = LVIF_TEXT;
      LVI.iSubItem = 1;
      LVI.pszText = tempString;
      pListCtrlBLOCKS->SetItem(&LVI);
//
//  Bus type
//
      LVI.mask = LVIF_TEXT;
      LVI.iSubItem = 2;
      LVI.pszText = m_pBlockData[nI].szBustypeName;
      pListCtrlBLOCKS->SetItem(&LVI);
//
//  Route
//
      LVI.mask = LVIF_TEXT;
      LVI.iSubItem = 3;
      LVI.pszText = m_pBlockData[nI].szRouteName;
      pListCtrlBLOCKS->SetItem(&LVI);
//
//  Service
//
      LVI.mask = LVIF_TEXT;
      LVI.iSubItem = 4;
      LVI.pszText = m_pBlockData[nI].szServiceName;
      pListCtrlBLOCKS->SetItem(&LVI);
//
//  Direction
//
      LVI.mask = LVIF_TEXT;
      LVI.iSubItem = 5;
      LVI.pszText = m_pBlockData[nI].szDirectionAbbrName;
      pListCtrlBLOCKS->SetItem(&LVI);
//
//  Pattern name
//
      LVI.mask = LVIF_TEXT;
      LVI.iSubItem = 6;
      LVI.pszText = m_pBlockData[nI].szPatternName;
      pListCtrlBLOCKS->SetItem(&LVI);
//
//  From node
//
      LVI.mask = LVIF_TEXT;
      LVI.iSubItem = 7;
      LVI.pszText = m_pBlockData[nI].szStartNodeAbbrName;
      pListCtrlBLOCKS->SetItem(&LVI);
//
//  From time
//
      strcpy(tempString, "");
      if((m_pBlockData[nI].flags & TRIPDATA_FLAG_SERVICEWRAP_PLUS) ||
            (m_pBlockData[nI].flags & TRIPDATA_FLAG_SERVICEWRAP_MINUS))
      {
        strcat(tempString, "*");
      }
      strcat(tempString, Tchar(m_pBlockData[nI].startTime));
      LVI.mask = LVIF_TEXT;
      LVI.iSubItem = 8;
      LVI.pszText = tempString;
      pListCtrlBLOCKS->SetItem(&LVI);
//
//  To time
//
      strcpy(tempString, Tchar(m_pBlockData[nI].endTime));
      LVI.mask = LVIF_TEXT;
      LVI.iSubItem = 9;
      LVI.pszText = tempString;
      pListCtrlBLOCKS->SetItem(&LVI);
//
//  To node
//
      LVI.mask = LVIF_TEXT;
      LVI.iSubItem = 10;
      LVI.pszText = m_pBlockData[nI].szEndNodeAbbrName;
      pListCtrlBLOCKS->SetItem(&LVI);
    }
//
//  Layover time
//
    if(bRebuild || m_pBlockData[nI].blockNumber == blockNumber)
    {
      LVI.iItem = nI;
      if(m_pBlockData[nI].lay == NO_TIME)
      {
        strcpy(tempString, "");
      }
      else
      {
        sprintf(tempString, "%d", m_pBlockData[nI].lay / 60);
      }
      LVI.mask = LVIF_TEXT;
      LVI.iSubItem = 11;
      LVI.pszText = tempString;
      pListCtrlBLOCKS->SetItem(&LVI);
//
//  Deadhead time
//
      if(m_pBlockData[nI].dhd == NO_TIME)
      {
        strcpy(tempString, "");
      }
      else
      {
        sprintf(tempString, "%d", m_pBlockData[nI].dhd / 60);
      }
      LVI.mask = LVIF_TEXT;
      LVI.iSubItem = 12;
      LVI.pszText = tempString;
      pListCtrlBLOCKS->SetItem(&LVI);
    }
//
//  Determine the display index on a partial rebuild
//
    if(!bRebuild)
    {
      reposition = nI;
    }
  }
//  if(m_pBlockData[nI].indexIntoTripData == indexToDisplay)
//  {
//    displayRecord = nI;
//  }
//
//  Position the list to the record to be displayed and set its selection state to "Selected"
//
  if(bRebuild)
  {
    reposition = indexToDisplay;
  }
  if(!m_bInitializing)
  {
    pListCtrlBLOCKS->EnsureVisible(reposition, FALSE);
    ListView_SetItemState(pListCtrlBLOCKS->m_hWnd, reposition, LVIS_SELECTED, LVIS_SELECTED);
//
//  Display its partners
//
    if(indexToDisplay != NO_RECORD)
    {
      DrawPartners(reposition);
    }
  }
}

//
//  DrawPartners() - Redraw the Partners list control
//
void CBlock::DrawPartners(int blockedTripIndex)
{
//
//  Check the highlighted blocked trip
//
  if(blockedTripIndex < 0)
  {
    return;
  } 
//
//  Clear out the previous selections in the partners list control
//
  ClearPartners();
//
//  Determine those trips that can hook with the selected (blocked) trip
//
  GetConnectionTimeDef GCTData;
  float distance;
  long  deadheadTime;
  long  layoverTime;
  long  tempLong;
  BOOL  bCase[3];
  int   nI;

  for(nI = 0; nI < m_numTrips; nI++)
  {
    if(m_pTripData[nI].blockNumber != 0)
    {
      continue;
    }
    if(m_pTripData[nI].flags & TRIPDATA_FLAG_SERVICEWRAP_PLUS)
    {
      m_pTripData[nI].startTime -= 86400;
      m_pTripData[nI].endTime -= 86400;
      m_pTripData[nI].flags -= TRIPDATA_FLAG_SERVICEWRAP_PLUS;
    }
    else if(m_pTripData[nI].flags & TRIPDATA_FLAG_SERVICEWRAP_MINUS)
    {
      m_pTripData[nI].startTime += 86400;
      m_pTripData[nI].endTime += 86400;
      m_pTripData[nI].flags -= TRIPDATA_FLAG_SERVICEWRAP_MINUS;
    }
    if(m_pBlockData[blockedTripIndex].ROUTESrecordID != m_pTripData[nI].ROUTESrecordID && !m_bInterlining)
    {
      continue;
    }
    bCase[0] = FALSE;
    bCase[1] = FALSE;
    bCase[2] = FALSE;
    if(m_pBlockData[blockedTripIndex].endTime + m_pBlockData[blockedTripIndex].layoverMin <= m_pTripData[nI].startTime &&
          m_pBlockData[blockedTripIndex].endTime + m_pBlockData[blockedTripIndex].layoverMax >= m_pTripData[nI].startTime)
    {
      bCase[0] = TRUE;
    }
    else if(m_bServiceWrap &&
          m_pBlockData[blockedTripIndex].endTime + m_pBlockData[blockedTripIndex].layoverMin <= m_pTripData[nI].startTime + 86400 &&
          m_pBlockData[blockedTripIndex].endTime + m_pBlockData[blockedTripIndex].layoverMax >= m_pTripData[nI].startTime + 86400)
    {
      bCase[1] = TRUE;
    }
    else if(m_bServiceWrap &&
          m_pBlockData[blockedTripIndex].endTime + m_pBlockData[blockedTripIndex].layoverMin <= m_pTripData[nI].startTime - 86400 &&
          m_pBlockData[blockedTripIndex].endTime + m_pBlockData[blockedTripIndex].layoverMax >= m_pTripData[nI].startTime - 86400)
    {
      bCase[2] = TRUE;
    }

    if(bCase[0] || bCase[1] || bCase[2])
    {
      deadheadTime = 0;
      if(!NodesEquivalent(m_pBlockData[blockedTripIndex].endNODESrecordID, m_pTripData[nI].startNODESrecordID, &tempLong))
      {
        if(!m_bDeadheading)
        {
          continue;
        }
        else
        {
          GCTData.fromROUTESrecordID = m_pBlockData[blockedTripIndex].ROUTESrecordID;
          GCTData.fromSERVICESrecordID = m_pBlockData[blockedTripIndex].SERVICESrecordID;
          GCTData.fromPATTERNNAMESrecordID = m_pBlockData[blockedTripIndex].PATTERNNAMESrecordID;
          GCTData.fromNODESrecordID = m_pBlockData[blockedTripIndex].endNODESrecordID;
          GCTData.timeOfDay = m_pBlockData[blockedTripIndex].endTime;
          GCTData.toROUTESrecordID = m_pTripData[nI].ROUTESrecordID;
          GCTData.toSERVICESrecordID = m_pTripData[nI].SERVICESrecordID;
          GCTData.toPATTERNNAMESrecordID = m_pTripData[nI].PATTERNNAMESrecordID;
          GCTData.toNODESrecordID = m_pTripData[nI].startNODESrecordID;
          distance = 0;
          deadheadTime = GetConnectionTime(GCT_FLAG_DEADHEADTIME, &GCTData, &distance);
          distance = (float)fabs((double)distance);
          if(deadheadTime == NO_TIME || deadheadTime < 0)
          {
            continue;
          }
        }
      }
      if(bCase[0])
      {
        layoverTime = m_pTripData[nI].startTime - m_pBlockData[blockedTripIndex].endTime - deadheadTime;
      }
      else if(bCase[1])
      {
        layoverTime = m_pTripData[nI].startTime + 86400 - m_pBlockData[blockedTripIndex].endTime - deadheadTime;
      }
      else if(bCase[2])
      {
        layoverTime = m_pTripData[nI].startTime - 86400 - m_pBlockData[blockedTripIndex].endTime - deadheadTime;
      }
      if(layoverTime >= 0)
      {
        m_pTripData[nI].flags = TRIPDATA_FLAG_PARTNER;
        if(bCase[1])
        {
          m_pTripData[nI].startTime += 86400;
          m_pTripData[nI].endTime += 86400;
          m_pTripData[nI].flags |= TRIPDATA_FLAG_SERVICEWRAP_PLUS;
        }
        else if(bCase[2])
        {
          m_pTripData[nI].startTime -= 86400;
          m_pTripData[nI].endTime -= 86400;
          m_pTripData[nI].flags |= TRIPDATA_FLAG_SERVICEWRAP_MINUS;
        }
        m_pTripData[nI].lay = layoverTime;
        m_pTripData[nI].dhd = deadheadTime;
      }
    }
  }
//
//  Go through m_pTripData and display all those flagged as partners
//
  LVITEM LVI;
  int pos = 0;

  for(nI = 0; nI < m_numTrips; nI++)
  {
    if(!(m_pTripData[nI].flags & TRIPDATA_FLAG_PARTNER))
    {
      continue;
    }
//
//  Bus type
//
    LVI.iItem = pos;
    LVI.mask = LVIF_TEXT | LVIF_PARAM;
    LVI.iSubItem = 0;
    LVI.pszText = m_pTripData[nI].szBustypeName;
    LVI.lParam = m_pTripData[nI].indexIntoTripData;
    pListCtrlPARTNERS->InsertItem(&LVI);
//
//  Route
//
    LVI.mask = LVIF_TEXT;
    LVI.iSubItem = 1;
    LVI.pszText = m_pTripData[nI].szRouteName;
    pListCtrlPARTNERS->SetItem(&LVI);
//
//  Service
//
    LVI.mask = LVIF_TEXT;
    LVI.iSubItem = 2;
    LVI.pszText = m_pTripData[nI].szServiceName;
    pListCtrlPARTNERS->SetItem(&LVI);
//
//  Direction
//
    LVI.mask = LVIF_TEXT;
    LVI.iSubItem = 3;
    LVI.pszText = m_pTripData[nI].szDirectionAbbrName;
    pListCtrlPARTNERS->SetItem(&LVI);
//
//  Pattern name
//
    LVI.mask = LVIF_TEXT;
    LVI.iSubItem = 4;
    LVI.pszText = m_pTripData[nI].szPatternName;
    pListCtrlPARTNERS->SetItem(&LVI);
//
//  From node
//
    LVI.mask = LVIF_TEXT;
    LVI.iSubItem = 5;
    LVI.pszText = m_pTripData[nI].szStartNodeAbbrName;
    pListCtrlPARTNERS->SetItem(&LVI);
//
//  From time
//
    strcpy(tempString, Tchar(m_pTripData[nI].startTime));
    LVI.mask = LVIF_TEXT;
    LVI.iSubItem = 6;
    LVI.pszText = tempString;
    pListCtrlPARTNERS->SetItem(&LVI);
//
//  To time
//
    strcpy(tempString, Tchar(m_pTripData[nI].endTime));
    LVI.mask = LVIF_TEXT;
    LVI.iSubItem = 7;
    LVI.pszText = tempString;
    pListCtrlPARTNERS->SetItem(&LVI);
//
//  To node
//
    LVI.mask = LVIF_TEXT;
    LVI.iSubItem = 8;
    LVI.pszText = m_pTripData[nI].szEndNodeAbbrName;
    pListCtrlPARTNERS->SetItem(&LVI);
//
//  Layover time
//
    if(m_pTripData[nI].lay == NO_TIME)
    {
      strcpy(tempString, "");
    }
    else
    {
      sprintf(tempString, "%d", m_pTripData[nI].lay / 60);
    }
    LVI.mask = LVIF_TEXT;
    LVI.iSubItem = 9;
    LVI.pszText = tempString;
    pListCtrlPARTNERS->SetItem(&LVI);
//
//  Deadhead time
//
    if(m_pTripData[nI].dhd == NO_TIME)
    {
      strcpy(tempString, "");
    }
    else
    {
      sprintf(tempString, "%d", m_pTripData[nI].dhd / 60);
    }
    LVI.mask = LVIF_TEXT;
    LVI.iSubItem = 10;
    LVI.pszText = tempString;
    pListCtrlPARTNERS->SetItem(&LVI);
//
//  Go to the next row
//
    pos++;
  }
//
//  Redraw the trips
//
  DrawTrips(NO_RECORD);
}

//
//  ClearPartners() - Delete everything in the Partners list control and reset the internal flag
//
void CBlock::ClearPartners()
{
  long partnerIndex;
  int  numInPartners = pListCtrlPARTNERS->GetItemCount();
  int  nI;
  
  for(nI = 0; nI < numInPartners; nI++)
  {
    partnerIndex = pListCtrlPARTNERS->GetItemData(nI);
    if(partnerIndex < 0)
    {
      continue;
    }
    m_pTripData[partnerIndex].flags &= ~TRIPDATA_FLAG_PARTNER;
  }
//  pListCtrlPARTNERS->DeleteAllItems();

  for(nI = 0; nI < numInPartners; nI++)
  {
    pListCtrlPARTNERS->DeleteItem(0);
  }
}

//
//  OnClose() - Treat an "X" as a cancel
//
void CBlock::OnClose() 
{
  OnCancel();
}

//
//  OnDeadheadall() - Allow all potential deadheads
//
void CBlock::OnDeadheadall() 
{
//  pEditDEADHEADMINS->SetWindowText("");

  ParameterChange();
}

//
//  OnDeadheading() - Allow deadheading in general
//
void CBlock::OnDeadheading() 
{
  m_bDeadheading = pButtonDEADHEADING->GetCheck();

  pButtonDEADHEADALL->EnableWindow(m_bDeadheading);
  pButtonDEADHEADTHOSE->EnableWindow(m_bDeadheading);
  pEditDEADHEADMINS->EnableWindow(m_bDeadheading);
  if(!m_bDeadheading)
  {
//    pEditDEADHEADMINS->SetWindowText("");
  }

  ParameterChange();
}

//
//  OnDeadheadthose() - Radio button to restrict deadheading (See OnChangeDeadheadmins())
//
void CBlock::OnDeadheadthose() 
{
  pEditDEADHEADMINS->SetFocus();
}

//
//  OnChangeDeadheadmins() - Allow the user to specify the upper limit of deadheading (in minutes)
//
void CBlock::OnChangeDeadheadmins() 
{
  if(!m_bInitializing)
  {
    pButtonDEADHEADALL->SetCheck(FALSE);
    pButtonDEADHEADTHOSE->SetCheck(TRUE);
    ParameterChange();
  }
}

//
//  OnInterlining() - Toggle between alloing interlining and not
//
void CBlock::OnInterlining() 
{
  m_bInterlining = pButtonINTERLINING->GetCheck();
  ParameterChange();
}

//
//  OnDblclkTrips() - Force an OnNewblock() call on a double left click
//
void CBlock::OnDblclkTrips(NMHDR* pNMHDR, LRESULT* pResult) 
{
  OnNewblock();

	*pResult = 0;
}

//
//  OnClickBlocks() - Highlight an entire blocked trip row on a left mouse click
//
void CBlock::OnClickBlocks(NMHDR* pNMHDR, LRESULT* pResult) 
{
//
//  Get the selected blocked trip and determine its index into m_pTripData
//
  int selectedBlock;
  int blockIndex;

  blockIndex = GetIndex(pListCtrlBLOCKS, NO_RECORD, &selectedBlock);
  if(blockIndex < 0 || selectedBlock < 0)
  {
    return;
  }
  DrawPartners(selectedBlock);
	
	*pResult = 0;
}

//
//  OnDblclkBlocks() - Force an OnRemove() call on a double left click
//
void CBlock::OnDblclkBlocks(NMHDR* pNMHDR, LRESULT* pResult) 
{
  OnRemove();
  	
	*pResult = 0;
}

//
//  OnRclickBlocks() - Highlight an entire blocked trip row before popping up the context menu
//
void CBlock::OnRclickBlocks(NMHDR* pNMHDR, LRESULT* pResult) 
{
//
//  Get the selected blocked trip and determine its index into m_pTripData
//
  int selectedBlock;
  int blockIndex;

  blockIndex = GetIndex(pListCtrlBLOCKS, NO_RECORD, &selectedBlock);
  if(blockIndex < 0 || selectedBlock < 0)
  {
    return;
  }
  DrawPartners(selectedBlock);

	*pResult = 0;
}

//
//  OnDblclkPartners() - Force an OnTakeTrip() call on a double left click
//
void CBlock::OnDblclkPartners(NMHDR* pNMHDR, LRESULT* pResult) 
{
  OnTaketrip();
  	
	*pResult = 0;
}

//
//  OnLayover() - Allow the user to enter new min/max layover parameters during blocking
//
void CBlock::OnLayover() 
{
  int  nI;
  int  nRc;

  nI = m_pPARMS->bStandard ? TMS_STANDARDBLOCKS : TMS_DROPBACKBLOCKS;
  nRc = DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_LAYOVER), m_hWnd, (DLGPROC)LAYOVERMsgProc, (LPARAM)nI);
  
  if(nRc)
  {
    ParameterChange();
  }
}

//
//  OnNewblock() - Move a trip from the TRIPS box to the BLOCKS box
//
void CBlock::OnNewblock() 
{
  int selectedTrip;
  int tripIndex;

  tripIndex = GetIndex(pListCtrlTRIPS, NO_RECORD, &selectedTrip);
  if(tripIndex < 0 || selectedTrip < 0)
  {
    return;
  }
//
//  And Set the new block number
//
  m_pTripData[tripIndex].blockNumber = ++m_lastBlockNumber;
  m_pTripData[tripIndex].RGRPROUTESrecordID = m_pDI->fileInfo.routeRecordID;
  ROUTESKey0.recordID = m_pTripData[tripIndex].RGRPROUTESrecordID;
  btrieve(B_GETEQUAL, TMS_ROUTES, &ROUTES, &ROUTESKey0, 0);
  strncpy(m_pTripData[tripIndex].szRGRPRouteName, ROUTES.number, ROUTES_NUMBER_LENGTH);
  trim(m_pTripData[tripIndex].szRGRPRouteName, ROUTES_NUMBER_LENGTH);
//
//  Remove the trip from the Trips list control
//
  pListCtrlTRIPS->DeleteItem(selectedTrip);
//
//  Redisplay the blocks
//
  DrawBlocks(FALSE, tripIndex);
//
//  Position to the newly added trip
//
  int nI;

  for(nI = 0; nI < m_numBlockedTrips; nI++)
  {
    if((int)pListCtrlBLOCKS->GetItemData(nI) == tripIndex)
    {
      pListCtrlBLOCKS->EnsureVisible(nI, FALSE);
      ListView_SetItemState(pListCtrlBLOCKS->m_hWnd, nI, LVIS_SELECTED, LVIS_SELECTED);
      DrawPartners(nI);
      pListCtrlBLOCKS->SetFocus();
      break;
    }
  }
}

//
//  OnAdd() - Add the highlighted trip to the highlighted block
//
void CBlock::OnAdd() 
{
  int selectedTrip;
  int tripIndex;

  tripIndex = GetIndex(pListCtrlTRIPS, NO_RECORD, &selectedTrip);
  if(tripIndex < 0 || selectedTrip < 0)
  {
    return;
  }
//
//  Get the selected blocked trip and determine its index into m_pTripData
//
  int selectedBlock;
  int blockIndex;

  blockIndex = GetIndex(pListCtrlBLOCKS, NO_RECORD, &selectedBlock);
  if(blockIndex < 0 || selectedBlock < 0)
  {
    return;
  }
//
//  Set the block number of the selected trip
//
  m_pTripData[tripIndex].blockNumber = m_pTripData[blockIndex].blockNumber;
  m_pTripData[tripIndex].RGRPROUTESrecordID = m_pTripData[blockIndex].RGRPROUTESrecordID;
  ROUTESKey0.recordID = m_pTripData[tripIndex].RGRPROUTESrecordID;
  btrieve(B_GETEQUAL, TMS_ROUTES, &ROUTES, &ROUTESKey0, 0);
  strncpy(m_pTripData[tripIndex].szRGRPRouteName, ROUTES.number, ROUTES_NUMBER_LENGTH);
  trim(m_pTripData[tripIndex].szRGRPRouteName, ROUTES_NUMBER_LENGTH);
//
//  Remove the trip from the trips list control
//
  pListCtrlTRIPS->DeleteItem(selectedTrip);
//
//  Redisplay the blocks
//
  DrawBlocks(TRUE, 0);
//
//  Position to the newly added trip
//
  int nI;

  for(nI = 0; nI < m_numBlockedTrips; nI++)
  {
    if((int)pListCtrlBLOCKS->GetItemData(nI) == tripIndex)
    {
      pListCtrlBLOCKS->EnsureVisible(nI, FALSE);
      ListView_SetItemState(pListCtrlBLOCKS->m_hWnd, nI, LVIS_SELECTED, LVIS_SELECTED);
      DrawPartners(nI);
      pListCtrlBLOCKS->SetFocus();
      break;
    }
  }
}

//
//  OnRemove() - Move a trip from the BLOCKS box back to the TRIPS box
//
void CBlock::OnRemove() 
{
//
//  Get the selected blocked trip and determine its index into m_pTripData
//
  int selectedBlock;
  int blockIndex;

  blockIndex = GetIndex(pListCtrlBLOCKS, NO_RECORD, &selectedBlock);
  if(blockIndex < 0 || selectedBlock < 0)
  {
    return;
  }
//
//  Delete it from the blocks list control and clear the block number in m_pTripData
//
  long blockNumber = m_pTripData[blockIndex].blockNumber;

  pListCtrlBLOCKS->DeleteItem(selectedBlock);
  m_pTripData[blockIndex].lay = NO_TIME;
  m_pTripData[blockIndex].dhd = NO_TIME;
  m_pTripData[blockIndex].blockNumber = 0;
  m_pTripData[blockIndex].RGRPROUTESrecordID = NO_RECORD;
  if(m_pTripData[blockIndex].flags & TRIPDATA_FLAG_SERVICEWRAP_PLUS)
  {
    m_pTripData[blockIndex].startTime -= 86400;
    m_pTripData[blockIndex].endTime -= 86400;
    m_pTripData[blockIndex].flags -= TRIPDATA_FLAG_SERVICEWRAP_PLUS;
  }
  else if(m_pTripData[blockIndex].flags & TRIPDATA_FLAG_SERVICEWRAP_MINUS)
  {
    m_pTripData[blockIndex].startTime += 86400;
    m_pTripData[blockIndex].endTime += 86400;
    m_pTripData[blockIndex].flags -= TRIPDATA_FLAG_SERVICEWRAP_MINUS;
  }
  strcpy(m_pTripData[blockIndex].szRGRPRouteName, "");
//
//  Clear the Partners list control and draw the trips
//
  ClearPartners();
//  qsort((void *)m_pTripData, m_numTrips, sizeof(TripDataDef), sort_trips);
  DrawTrips(NO_RECORD);
//
//  Fix up the arrays
//
  int nI, nJ;

//  m_numBlockedTrips = 0;
//  for(nI = 0; nI < m_numTrips; nI++)
//  {
//    if(m_pTripData[nI].blockNumber > 0)
//    {
//      memcpy(&m_pBlockData[m_numBlockedTrips], &m_pTripData[nI], sizeof(TripDataDef));
//      m_numBlockedTrips++;
//    }
//  } 
//
//  And sort
//
//  qsort((void *)m_pBlockData, m_numBlockedTrips, sizeof(TripDataDef), sort_blocks);
//
  for(nI = selectedBlock; nI < pListCtrlBLOCKS->GetItemCount(); nI++)
  {
    memcpy(&m_pBlockData[nI], &m_pBlockData[nI + 1], sizeof(TripDataDef));
  }
  m_numBlockedTrips--;

//
//  Fix the layovers and deadheads
//
  EstablishDhdAndLay(blockNumber);
//
//  Fix the display
//
  LVITEM LVI;

  nJ = pListCtrlBLOCKS->GetItemCount();
  for(nI = 0; nI < nJ; nI++)
  {
    if(m_pBlockData[nI].blockNumber == blockNumber)
    {
//
//  Layover time
//
      if(m_pBlockData[nI].lay == NO_TIME)
      {
        strcpy(tempString, "");
      }
      else
      {
        sprintf(tempString, "%d", m_pBlockData[nI].lay / 60);
      }
      LVI.iItem = nI;
      LVI.mask = LVIF_TEXT;
      LVI.iSubItem = 11;
      LVI.pszText = tempString;
      pListCtrlBLOCKS->SetItem(&LVI);
//
//  Deadhead time
//
      if(m_pBlockData[nI].dhd == NO_TIME)
      {
        strcpy(tempString, "");
      }
      else
      {
        sprintf(tempString, "%d", m_pBlockData[nI].dhd / 60);
      }
      LVI.mask = LVIF_TEXT;
      LVI.iSubItem = 12;
      LVI.pszText = tempString;
      pListCtrlBLOCKS->SetItem(&LVI);
    }
  }
}

//
//  OnUnhookblock() - Nuke all the trips on the highlighted block
//
void CBlock::OnUnhookblock() 
{
//
//  Get the selected blocked trip and determine its index into m_pTripData
//
  int selectedBlock;
  int blockIndex;

  blockIndex = GetIndex(pListCtrlBLOCKS, NO_RECORD, &selectedBlock);
  if(blockIndex < 0 || selectedBlock < 0)
  {
    return;
  }

  long blockNumber = m_pTripData[blockIndex].blockNumber;
//
//  Start at the end of the box looking for the last occurrence
//  of the block number to unhook and then start deleting
//
  int nI;
  int numInBlocks = pListCtrlBLOCKS->GetItemCount();

  for(nI = numInBlocks - 1; nI >= 0; nI--)
  {
    blockIndex = pListCtrlBLOCKS->GetItemData(nI);
    if(blockIndex < 0)
    {
      continue;
    }
    if(m_pTripData[blockIndex].blockNumber == blockNumber)
    {
      pListCtrlBLOCKS->DeleteItem(nI);
      m_pTripData[blockIndex].lay = NO_TIME;
      m_pTripData[blockIndex].dhd = NO_TIME;
      m_pTripData[blockIndex].blockNumber = 0;
      m_pTripData[blockIndex].RGRPROUTESrecordID = NO_RECORD;
      if(m_pTripData[blockIndex].flags & TRIPDATA_FLAG_SERVICEWRAP_PLUS)
      {
        m_pTripData[blockIndex].startTime -= 86400;
        m_pTripData[blockIndex].endTime -= 86400;
        m_pTripData[blockIndex].flags -= TRIPDATA_FLAG_SERVICEWRAP_PLUS;
      }
      else if(m_pTripData[blockIndex].flags & TRIPDATA_FLAG_SERVICEWRAP_MINUS)
      {
        m_pTripData[blockIndex].startTime += 86400;
        m_pTripData[blockIndex].endTime += 86400;
        m_pTripData[blockIndex].flags -= TRIPDATA_FLAG_SERVICEWRAP_MINUS;
      }
      strcpy(m_pTripData[blockIndex].szRGRPRouteName, "");
    }
  }
  m_numBlockedTrips = 0;
  for(nI = 0; nI < m_numTrips; nI++)
  {
    if(m_pTripData[nI].blockNumber > 0)
    {
      memcpy(&m_pBlockData[m_numBlockedTrips], &m_pTripData[nI], sizeof(TripDataDef));
      m_numBlockedTrips++;
    }
  } 
//
//  Clear the Partners list control and draw the trips
//
  ClearPartners();
  DrawTrips(NO_RECORD);
  DrawBlocks(TRUE, blockIndex);
}

//
//  OnRenumber() - Renumber the block whose trip is highlighted
//
void CBlock::OnRenumber() 
{
//
//  Get the selected blocked trip and determine its index into m_pTripData
//
  int selectedBlock;
  int blockIndex;

  blockIndex = GetIndex(pListCtrlBLOCKS, NO_RECORD, &selectedBlock);
  if(blockIndex < 0 || selectedBlock < 0)
  {
    return;
  }

  long blockNumber = m_pTripData[blockIndex].blockNumber;
//
//  Fire up the Renumber dialog
//
  RENUMBERPARAMDef RENUM;

  strcpy(RENUM.type, "block");
  RENUM.number = blockNumber;
  if(DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_RENUMBER), m_hWnd, (DLGPROC)RENUMBERMsgProc, (LPARAM)&RENUM) != IDOK)
  {
    return;
  }
  
  long newBlockNumber = RENUM.number;
//
//  Change the number in m_pTripData and redraw the blocks
//
  int nI;

  for(nI = 0; nI < m_numTrips; nI++)
  {
    if(m_pTripData[nI].blockNumber == blockNumber)
    {
      m_pTripData[nI].blockNumber = newBlockNumber;
    }
  }
  DrawBlocks(TRUE, 0);
//
//  Position to the originally selected blocked trip
//
  for(nI = 0; nI < m_numBlockedTrips; nI++)
  {
    if((int)pListCtrlBLOCKS->GetItemData(nI) == blockIndex)
    {
      pListCtrlBLOCKS->EnsureVisible(nI, FALSE);
      ListView_SetItemState(pListCtrlBLOCKS->m_hWnd, nI, LVIS_SELECTED, LVIS_SELECTED);
      DrawPartners(nI);
      pListCtrlBLOCKS->SetFocus();
      break;
    }
  }
}

//
//  OnSummary() - Display a block summary for the block whose trip is currently highlighted
//
void CBlock::OnSummary() 
{
//
//  Get the selected blocked trip and determine its index into m_pTripData
//
  int blockIndex = GetIndex(pListCtrlBLOCKS, NO_RECORD);
  
  if(blockIndex < 0)
  {
    return;
  }

  long blockNumber = m_pTripData[blockIndex].blockNumber;
//
//  Get this block's information
//
  SUMMARIZEBLOCKParameterDef SBParms;

  SBParms.blockNumber = blockNumber;
  SBParms.assignedToNODESrecordID = NO_RECORD;
  SBParms.totalPlat = 0;
  SBParms.totalLay = 0;
  SBParms.totalILDhd = 0;
  SBParms.totalGADhd = 0;
  SBParms.numTrips = 0;

  BOOL bFoundFirst;
  int  numInBlocks = pListCtrlBLOCKS->GetItemCount();
  int  nI;
//
//  Build up the summary parameters for the Block Summary dialog
//
  for(bFoundFirst = FALSE, nI = 0; nI < numInBlocks; nI++)
  {
    if(m_pBlockData[nI].blockNumber != blockNumber)
    {
      if(bFoundFirst)
      {
        break;
      }
      else
      {
        continue;
      }
    }
    bFoundFirst = TRUE;
    SBParms.totalPlat += (m_pBlockData[nI].endTime - m_pBlockData[nI].startTime);
    if(m_pBlockData[nI].lay != NO_TIME)
    {
      SBParms.totalLay += m_pBlockData[nI].lay;
    }
    if(m_pBlockData[nI].dhd != NO_TIME)
    {
      SBParms.totalILDhd += m_pBlockData[nI].dhd;
    }
    SBParms.numTrips++;
  }
//
//  Fire up the dialog
//
  DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_SUMMARIZEBLOCK), m_hWnd, (DLGPROC)SUMMARIZEBLOCKMsgProc, (LPARAM)&SBParms);
}

//
//  OnTaketrip() - Move a trip from the PARTNERS box to the BLOCKS box
//
void CBlock::OnTaketrip() 
{
//
//  Get the selected blocked trip and determine its index into m_pTripData
//
  int blockIndex = GetIndex(pListCtrlBLOCKS, NO_RECORD);
  
  if(blockIndex < 0)
  {
    return;
  }

  long blockNumber = m_pTripData[blockIndex].blockNumber;
//
//  So what are we getting?
//
  int partnerIndex = GetIndex(pListCtrlPARTNERS, NO_RECORD);

  if(partnerIndex < 0)
  {
    return;
  }
//
//  Clear/reset the partner list and redraw the blocks
//
  ClearPartners();
  m_pTripData[partnerIndex].blockNumber = blockNumber;
  m_pTripData[partnerIndex].RGRPROUTESrecordID = m_pTripData[blockIndex].RGRPROUTESrecordID;
  ROUTESKey0.recordID = m_pTripData[partnerIndex].RGRPROUTESrecordID;
  btrieve(B_GETEQUAL, TMS_ROUTES, &ROUTES, &ROUTESKey0, 0);
  strncpy(m_pTripData[partnerIndex].szRGRPRouteName, ROUTES.number, ROUTES_NUMBER_LENGTH);
  trim(m_pTripData[partnerIndex].szRGRPRouteName, ROUTES_NUMBER_LENGTH);
//  DrawBlocks(TRUE, 0);
  DrawBlocks(FALSE, partnerIndex);
//
//  Fix the display
//
/*
  LVITEM LVI;
  int    nI, nJ;

  for(nI = 0; nI < m_numBlockedTrips - 1; nI++)
  {
    if(m_pBlockData[nI].blockNumber == blockNumber)
    {
      for(nJ = 0; nJ < pListCtrlBLOCKS->GetItemCount(); nJ++)
      {
//
//  Layover time
//
        if(m_pBlockData[nJ].lay == NO_TIME)
        {
          strcpy(tempString, "");
        }
        else
        {
          sprintf(tempString, "%d", m_pBlockData[nJ].lay / 60);
        }
        LVI.iItem = nJ;
        LVI.mask = LVIF_TEXT;
        LVI.iSubItem = 11;
        LVI.pszText = tempString;
        pListCtrlBLOCKS->SetItem(&LVI);
//
//  Deadhead time
//
        if(m_pBlockData[nJ].dhd == NO_TIME)
        {
          strcpy(tempString, "");
        }
        else
        {
          sprintf(tempString, "%d", m_pBlockData[nJ].dhd / 60);
        }
        LVI.mask = LVIF_TEXT;
        LVI.iSubItem = 12;
        LVI.pszText = tempString;
        pListCtrlBLOCKS->SetItem(&LVI);
      }
    }
  }
*/
//
//  Make the partner trip the selected trip in the blocks list control
//
  long numInBlocks = pListCtrlBLOCKS->GetItemCount();
  int  nI;

  for(nI = 0; nI < numInBlocks; nI++)
  {
    if((long)pListCtrlBLOCKS->GetItemData(nI) == partnerIndex)
    {
      pListCtrlBLOCKS->SetFocus();
      pListCtrlBLOCKS->EnsureVisible(nI, FALSE);
      pListCtrlBLOCKS->SetItemState(nI, LVIS_SELECTED, LVIS_SELECTED);
      DrawPartners(nI);
      break;
    }
  }
}

//
//  OnSave() - Update TMS_TRIPS with those records in the TRIPS and BLOCKS boxes,
//  but only update those that need updating.
//
void CBlock::OnSave() 
{
  HCURSOR saveCursor = SetCursor(hCursorWait);
  int  blockIndex;
  int  tripIndex;
  int  numInBlocks = pListCtrlBLOCKS->GetItemCount();
  int  numInTrips  = pListCtrlTRIPS->GetItemCount();
  int  nI;
//
//  Go through the blocks
//
  for(nI = 0; nI < numInBlocks; nI++)
  {
    blockIndex = pListCtrlBLOCKS->GetItemData(nI);
    if(blockIndex < 0)
    {
      return;
    }
    TRIPSKey0.recordID = m_pTripData[blockIndex].TRIPSrecordID;
    btrieve(B_GETEQUAL, TMS_TRIPS, &TRIPS, &TRIPSKey0, 0);
    if(m_pTRIPSChunk->blockNumber != m_pTripData[blockIndex].blockNumber)
    {
      m_pTRIPSChunk->assignedToNODESrecordID = NO_RECORD;
      m_pTRIPSChunk->RGRPROUTESrecordID = m_pDI->fileInfo.routeRecordID;
      m_pTRIPSChunk->SGRPSERVICESrecordID = m_pDI->fileInfo.serviceRecordID;
      m_pTRIPSChunk->blockNumber = m_pTripData[blockIndex].blockNumber;
      m_pTRIPSChunk->blockSequence = m_pTripData[blockIndex].startTime;
      if(m_pTripData[blockIndex].flags & TRIPDATA_FLAG_SERVICEWRAP_PLUS)
      {
        if(!(TRIPS.flags & TRIPS_FLAG_SERVICEWRAP_PLUS))
        {
          TRIPS.timeAtMLP += 86400;
          m_pTRIPSChunk->blockSequence += 86400;
          TRIPS.flags = TRIPS_FLAG_SERVICEWRAP_PLUS;
        }
      }
      else if(m_pTripData[blockIndex].flags & TRIPDATA_FLAG_SERVICEWRAP_MINUS)
      {
        if(!(TRIPS.flags & TRIPS_FLAG_SERVICEWRAP_MINUS))
        {
          TRIPS.timeAtMLP -= 86400;
          m_pTRIPSChunk->blockSequence -= 86400;
          TRIPS.flags = TRIPS_FLAG_SERVICEWRAP_MINUS;
        }
      }
      m_pTRIPSChunk->POGNODESrecordID = NO_RECORD;
      m_pTRIPSChunk->PIGNODESrecordID = NO_RECORD;
      btrieve(B_UPDATE, TMS_TRIPS, &TRIPS, &TRIPSKey0, 0);
    }
  }
//
//  Go through the trips
//
  for(nI = 0; nI < numInTrips; nI++)
  {
//    tripIndex = GetIndex(pListCtrlTRIPS, NO_RECORD);
    tripIndex = pListCtrlTRIPS->GetItemData(nI);
    if(tripIndex < 0)
    {
      return;
    }
    TRIPSKey0.recordID = m_pTripData[tripIndex].TRIPSrecordID;
    btrieve(B_GETEQUAL, TMS_TRIPS, &TRIPS, &TRIPSKey0, 0);
    if(m_pTRIPSChunk->RGRPROUTESrecordID != NO_RECORD ||
          m_pTRIPSChunk->SGRPSERVICESrecordID != NO_RECORD ||
          m_pTRIPSChunk->blockNumber != 0 ||
          m_pTRIPSChunk->POGNODESrecordID != NO_RECORD ||
          m_pTRIPSChunk->PIGNODESrecordID != NO_RECORD)
    {
      m_pTRIPSChunk->assignedToNODESrecordID = NO_RECORD;
      m_pTRIPSChunk->RGRPROUTESrecordID = NO_RECORD;
      m_pTRIPSChunk->SGRPSERVICESrecordID = NO_RECORD;
      m_pTRIPSChunk->blockNumber = 0;
      m_pTRIPSChunk->POGNODESrecordID = NO_RECORD;
      m_pTRIPSChunk->PIGNODESrecordID = NO_RECORD;
      m_pTRIPSChunk->blockSequence = NO_TIME;
      if(TRIPS.flags & TRIPS_FLAG_SERVICEWRAP_PLUS)
      {
        TRIPS.timeAtMLP -= 86400;
        TRIPS.tripSequence -= 86400;
        TRIPS.flags -= TRIPS_FLAG_SERVICEWRAP_PLUS;
      }
      else if(TRIPS.flags & TRIPS_FLAG_SERVICEWRAP_MINUS)
      {
        TRIPS.timeAtMLP += 86400;
        TRIPS.tripSequence += 86400;
        TRIPS.flags -= TRIPS_FLAG_SERVICEWRAP_MINUS;
      }
      btrieve(B_UPDATE, TMS_TRIPS, &TRIPS, &TRIPSKey0, 0);
    }
  }
//
//  Restore the cursor
//
  SetCursor(saveCursor);
}

void CBlock::OnCancel() 
{
  CString s;

  s.LoadString(ERROR_159);
  MessageBeep(MB_ICONQUESTION);
  if(MessageBox(s, TMS, MB_ICONQUESTION | MB_YESNO) == IDYES)
  {
    FreeMem();
	  CDialog::OnCancel();
  }
}

void CBlock::OnHelp() 
{
  ::WinHelp(m_hWnd, szarHelpFile, HELP_CONTEXT, Interactive_Blocking);
}

void CBlock::OnOK() 
{
  OnSave();
  FreeMem();
  	
	CDialog::OnOK();
}

//
//  GetIndex() - Return the ItemData of the selected list control item.
//               This value was set to this item's index into m_pTripData.
//
int CBlock::GetIndex(CListCtrl* pLC, int item)
{
  int selection = (item == NO_RECORD ? pLC->GetNextItem(-1, LVNI_ALL | LVNI_SELECTED) : item);
  
  if(selection >= 0)
  {
    return((int)pLC->GetItemData(selection));
  }

  return(NO_RECORD);
}

//
//  GetIndex() - Return the ItemData of the selected list control item.
//               This value was set to this item's index into m_pTripData.
//
int CBlock::GetIndex(CListCtrl* pLC, int item, int* selection)
{
  *selection = (item == NO_RECORD ? pLC->GetNextItem(-1, LVNI_ALL | LVNI_SELECTED) : item);
  
  if(*selection >= 0)
  {
    return((int)pLC->GetItemData(*selection));
  }

  return(NO_RECORD);
}

//
//  FreeMem() - Gracefully free the m_pTripData and m_pBlockData memory allocations
//
void CBlock::FreeMem()
{
  TMSHeapFree(m_pTripData);
  TMSHeapFree(m_pBlockData);
}

//
//  Establish DhdAndLay() - (Re)build values in m_pBlockedData
//
void CBlock::EstablishDhdAndLay(long blockNumber)
{
  GetConnectionTimeDef GCTData;
  float distance;
  long  tempLong;
  int   nI;

//
//  Clear out previous values
//
  for(nI = 0; nI < m_numBlockedTrips; nI++)
  {
    if(blockNumber == NO_RECORD || (blockNumber != NO_RECORD && m_pBlockData[nI].blockNumber == blockNumber))
    {
      m_pBlockData[nI].dhd = NO_TIME;
      m_pBlockData[nI].lay = NO_TIME;
    }
  }
//
//  Recalculate
//
  for(nI = 0; nI < m_numBlockedTrips - 1; nI++)
  {
    if(blockNumber == NO_RECORD || (blockNumber != NO_RECORD && m_pBlockData[nI].blockNumber == blockNumber))
    {
      if(m_pBlockData[nI].blockNumber == m_pBlockData[nI + 1].blockNumber)
      {
        if(!NodesEquivalent(m_pBlockData[nI].endNODESrecordID, m_pBlockData[nI + 1].startNODESrecordID, &tempLong))
        {
          if(m_bDeadheading)
          {
            GCTData.fromROUTESrecordID = m_pBlockData[nI].ROUTESrecordID;
            GCTData.fromSERVICESrecordID = m_pBlockData[nI].SERVICESrecordID;
            GCTData.fromPATTERNNAMESrecordID = m_pBlockData[nI].PATTERNNAMESrecordID;
            GCTData.fromNODESrecordID = m_pBlockData[nI].endNODESrecordID;
            GCTData.timeOfDay = m_pBlockData[nI].endTime;
            GCTData.toROUTESrecordID = m_pBlockData[nI + 1].ROUTESrecordID;
            GCTData.toSERVICESrecordID = m_pBlockData[nI + 1].SERVICESrecordID;
            GCTData.toPATTERNNAMESrecordID = m_pBlockData[nI + 1].PATTERNNAMESrecordID;
            GCTData.toNODESrecordID = m_pBlockData[nI + 1].startNODESrecordID;
            distance = 0;
            m_pBlockData[nI].dhd = GetConnectionTime(GCT_FLAG_DEADHEADTIME, &GCTData, &distance);
            distance = (float)fabs((double)distance);
          }
        }
        if(m_pBlockData[nI].dhd == NO_TIME)
        {
          m_pBlockData[nI].lay = m_pBlockData[nI + 1].startTime - m_pBlockData[nI].endTime;
        }
        else
        {
          m_pBlockData[nI].lay = m_pBlockData[nI + 1].startTime - m_pBlockData[nI].endTime - m_pBlockData[nI].dhd;
        }
      }
    }
  }
}

void CBlock::ParameterChange()
{
  long minutes;
  long percent;
  long blockIndex;
  int  selectedBlock;
  int  nI, nJ;

  for(nI = 0; nI < m_numTrips; nI++)
  {
    TRIPSKey0.recordID = m_pTripData[nI].TRIPSrecordID;
    btrieve(B_GETEQUAL, TMS_TRIPS, &TRIPS, &TRIPSKey0, 0);
//
//  Figure out the minimum layover
//
    if(m_pTRIPSChunk->layoverMin != NO_TIME)
    {
      m_pTripData[nI].layoverMin = m_pTRIPSChunk->layoverMin;
    }
    else
    {
      minutes = m_pLayover->minimum.minutes  * 60;
      percent = (m_pTripData[nI].endTime - m_pTripData[nI].startTime) * m_pLayover->minimum.percent / 100;
      if(m_pLayover->minimum.type == 0)
      {
        m_pTripData[nI].layoverMin = minutes;
      }
      else if(m_pLayover->minimum.type == 1)
      {
        m_pTripData[nI].layoverMin = percent;
      }
      else if(m_pLayover->minimum.type == 2)
      {
        m_pTripData[nI].layoverMin = m_pLayover->minimum.lower ? min(minutes, percent) : max(minutes, percent);
      }
    }
//
//  Figure out the maximum layover
//
    if(m_pTRIPSChunk->layoverMax != NO_TIME)
    {
      m_pTripData[nI].layoverMax = m_pTRIPSChunk->layoverMax;
    }
    else
    {
      minutes = m_pLayover->maximum.minutes * 60;
      percent = (m_pTripData[nI].endTime - m_pTripData[nI].startTime) * m_pLayover->maximum.percent / 100;
      if(m_pLayover->maximum.type == 0)
      {
        m_pTripData[nI].layoverMax = minutes;
      }
      else if(m_pLayover->maximum.type == 1)
      {
        m_pTripData[nI].layoverMax = percent;
      }
      else if(m_pLayover->maximum.type == 2)
      {
        m_pTripData[nI].layoverMax = m_pLayover->maximum.lower ? min(minutes, percent) : max(minutes, percent);
      }
    }
  }
//
//  Tell the trips in the blocks box about the change
//
  for(nI = 0; nI < pListCtrlBLOCKS->GetItemCount(); nI++)
  {
    nJ = m_pBlockData[nI].indexIntoTripData;
    if(nJ < 0)
    {
      continue;
    }
    m_pBlockData[nI].layoverMin = m_pTripData[nJ].layoverMin;
    m_pBlockData[nI].layoverMax = m_pTripData[nJ].layoverMax;
  }
//
//  Update the Partners list
//
  blockIndex = GetIndex(pListCtrlBLOCKS, NO_RECORD, &selectedBlock);
  if(blockIndex < 0 || selectedBlock < 0)
  {
    ClearPartners();
  }
  else
  {
    DrawPartners(selectedBlock);
  }
}

void CBlock::OnServicewrap() 
{
  m_bServiceWrap = pButtonSERVICEWRAP->GetCheck();
  ParameterChange();
}

// DailyOpsBP.cpp : implementation file
//

#include "stdafx.h"
extern "C"{
#include "TMSHeader.h"
}  // EXTERN C

#include "tms.h"
#include "DailyOpsHeader.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDailyOpsBP dialog


CDailyOpsBP::CDailyOpsBP(CWnd* pParent, BlockInfoDef* pBlockInfo, CTime DailyOpsDate)   // standard constructor
	: CDialog(CDailyOpsBP::IDD, pParent)
{
  m_pBlockInfo = pBlockInfo;
  m_DailyOpsDate = DailyOpsDate;

	//{{AFX_DATA_INIT(CDailyOpsBP)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CDailyOpsBP::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDailyOpsBP)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDailyOpsBP, CDialog)
	//{{AFX_MSG_MAP(CDailyOpsBP)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDHELP, OnHelp)
	ON_BN_CLICKED(IDMODIFY, OnModify)
	ON_NOTIFY(NM_CLICK, DAILYOPSBP_TRIPLIST, OnClickTriplist)
	ON_NOTIFY(NM_DBLCLK, DAILYOPSBP_TRIPLIST, OnDblclkTriplist)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDailyOpsBP message handlers

BOOL CDailyOpsBP::OnInitDialog() 
{
	CDialog::OnInitDialog();
//
//  Set up pointers to the controls
//
  pListCtrlTRIPLIST	= (CListCtrl *)GetDlgItem(DAILYOPSBP_TRIPLIST);
  pButtonIDMODIFY = (CButton *)GetDlgItem(IDMODIFY);
//
//  Extend its style
//
  DWORD dwExStyles = pListCtrlTRIPLIST->GetExtendedStyle();
  pListCtrlTRIPLIST->SetExtendedStyle(dwExStyles | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_INFOTIP);
//
//  Set the title of the dialog
//
  CString sRGRP;
  CString sSGRP;

  ROUTESKey0.recordID = m_pBlockInfo->RGRPROUTESrecordID;
  btrieve(B_GETEQUAL, TMS_ROUTES, &ROUTES, &ROUTESKey0, 0);
  strncpy(tempString, ROUTES.number, ROUTES_NUMBER_LENGTH);
  trim(tempString, ROUTES_NUMBER_LENGTH);
  sRGRP = tempString;
  sRGRP += " - ";
  strncpy(tempString, ROUTES.name, ROUTES_NAME_LENGTH);
  trim(tempString, ROUTES_NAME_LENGTH);
  sRGRP += tempString;

  SERVICESKey0.recordID = m_pBlockInfo->SGRPSERVICESrecordID;
  btrieve(B_GETEQUAL, TMS_SERVICES, &SERVICES, &SERVICESKey0, 0);
  strncpy(tempString, SERVICES.name, SERVICES_NAME_LENGTH);
  trim(tempString, SERVICES_NAME_LENGTH);
  sSGRP = tempString;

  sprintf(tempString, "Block number %ld Properties : Blocked on RGRP %s, SGRP %s",
        m_pBlockInfo->blockNumber, sRGRP, sSGRP);
  SetWindowText(tempString);
//
//  Set the headers and columns on the list control
//
  LVCOLUMN LVC;

  LVC.mask = LVCF_FMT | LVCF_TEXT | LVCF_WIDTH;

  LVC.cx = 45;
  LVC.fmt = LVCFMT_CENTER;
  LVC.pszText = "Trip #";
  pListCtrlTRIPLIST->InsertColumn(0, &LVC);

  LVC.cx = 70;
  LVC.fmt = LVCFMT_CENTER;
  LVC.pszText = "Status";
  pListCtrlTRIPLIST->InsertColumn(1, &LVC);

  LVC.cx = 45;
  LVC.fmt = LVCFMT_CENTER;
  LVC.pszText = "Run";
  pListCtrlTRIPLIST->InsertColumn(2, &LVC);

  LVC.cx = 50;
  LVC.fmt = LVCFMT_CENTER;
  LVC.pszText = "Roster";
  pListCtrlTRIPLIST->InsertColumn(3, &LVC);

  LVC.cx = 160;
  LVC.fmt = LVCFMT_CENTER;
  LVC.pszText = "Rostered Operator";
  pListCtrlTRIPLIST->InsertColumn(4, &LVC);

  LVC.cx = 45;
  LVC.fmt = LVCFMT_CENTER;
  LVC.pszText = "Rlf At";
  pListCtrlTRIPLIST->InsertColumn(5, &LVC);

  LVC.cx = 45;
  LVC.fmt = LVCFMT_CENTER;
  LVC.pszText = "Time";
  pListCtrlTRIPLIST->InsertColumn(6, &LVC);

  LVC.cx = 45;
  LVC.fmt = LVCFMT_CENTER;
  LVC.pszText = "POG";
  pListCtrlTRIPLIST->InsertColumn(7, &LVC);

  LVC.cx = 45;
  LVC.fmt = LVCFMT_CENTER;
  LVC.pszText = "POT";
  pListCtrlTRIPLIST->InsertColumn(8, &LVC);

  LVC.cx = 45;
  LVC.fmt = LVCFMT_CENTER;
  LVC.pszText = "Route";
  pListCtrlTRIPLIST->InsertColumn(9, &LVC);
 
  LVC.cx = 74;
  LVC.fmt = LVCFMT_CENTER;
  LVC.pszText = "Pattern";
  pListCtrlTRIPLIST->InsertColumn(10, &LVC);
 
  LVC.cx = 45;
  LVC.fmt = LVCFMT_CENTER;
  LVC.pszText = "FNode";
  pListCtrlTRIPLIST->InsertColumn(11, &LVC);
 
  LVC.cx = 45;
  LVC.fmt = LVCFMT_CENTER;
  LVC.pszText = "FTime";
  pListCtrlTRIPLIST->InsertColumn(12, &LVC);
 
  LVC.cx = 45;
  LVC.fmt = LVCFMT_CENTER;
  LVC.pszText = "TTime";
  pListCtrlTRIPLIST->InsertColumn(13, &LVC);
 
  LVC.cx = 45;
  LVC.fmt = LVCFMT_CENTER;
  LVC.pszText = "TNode";
  pListCtrlTRIPLIST->InsertColumn(14, &LVC);
 
  LVC.cx = 45;
  LVC.fmt = LVCFMT_CENTER;
  LVC.pszText = "PIG";
  pListCtrlTRIPLIST->InsertColumn(15, &LVC);

  LVC.cx = 45;
  LVC.fmt = LVCFMT_CENTER;
  LVC.pszText = "PIT";
  pListCtrlTRIPLIST->InsertColumn(16, &LVC);
//
// Figure out the service day
//
  int  today;
  char szHolidayName[DAILYOPS_DATENAME_LENGTH + 1];
  long yyyy = m_DailyOpsDate.GetYear();
  long mm   = m_DailyOpsDate.GetMonth();
  long dd   = m_DailyOpsDate.GetDay();
  long dateToUse = (yyyy * 10000) + (mm * 100) + dd;
  long SERVICESrecordID = CDailyOps::DetermineServiceDay(dateToUse, FALSE, &today, szHolidayName);
//
//  Build the m_TripInfo structure
//
  int keyNumber = 2;

  TRIPSKey0.recordID = m_pBlockInfo->TRIPSrecordID;
  btrieve(B_GETEQUAL, TMS_TRIPS, &TRIPS, &TRIPSKey0, 0);

  GenerateTripDef GTResults;
  BLOCKSDef *pTRIPSChunk = keyNumber == 2 ? &TRIPS.standard : &TRIPS.dropback;
  long startTRIPSrecordID = NO_RECORD;
  long endTRIPSrecordID = NO_RECORD;
  long previousRunNumber = NO_RECORD;
  long previousRosterNumber = NO_RECORD;
  long RUNSrecordID;
  long runNumber;
  char *ptr = NULL;
  long assignedToNODESrecordID = TRIPS.standard.assignedToNODESrecordID;
  long reliefAtNODESrecordID;
  long patternIndex;
  BOOL bFound;
  BOOL bCrewOnly;
  int  rcode2;
  int  nI;

  m_numInTripInfo = 0;
  TRIPSKey2.assignedToNODESrecordID = assignedToNODESrecordID;
  TRIPSKey2.RGRPROUTESrecordID = m_pBlockInfo->RGRPROUTESrecordID;
  TRIPSKey2.SGRPSERVICESrecordID = m_pBlockInfo->SGRPSERVICESrecordID;
  TRIPSKey2.blockNumber = m_pBlockInfo->blockNumber;
  TRIPSKey2.blockSequence = NO_RECORD;
  rcode2 = btrieve(B_GETGREATER, TMS_TRIPS, &TRIPS, &TRIPSKey2, keyNumber);
  while(rcode2 == 0 &&
        pTRIPSChunk->assignedToNODESrecordID == assignedToNODESrecordID &&
        pTRIPSChunk->RGRPROUTESrecordID == m_pBlockInfo->RGRPROUTESrecordID &&
        pTRIPSChunk->SGRPSERVICESrecordID == m_pBlockInfo->SGRPSERVICESrecordID &&
        pTRIPSChunk->blockNumber == m_pBlockInfo->blockNumber)
  {
    GenerateTrip(TRIPS.ROUTESrecordID, TRIPS.SERVICESrecordID,
          TRIPS.directionIndex, TRIPS.PATTERNNAMESrecordID,
          TRIPS.timeAtMLP, GENERATETRIP_FLAG_DISPLAYERRORS, &GTResults);

    reliefAtNODESrecordID = NO_RECORD;
//
//  TRIPSrecordID
//
    m_TripInfo[m_numInTripInfo].TRIPSrecordID = TRIPS.recordID;
//
//  Trip number
//
    m_TripInfo[m_numInTripInfo].tripNumber = TRIPS.tripNumber;
//
//  Status
//
    m_TripInfo[m_numInTripInfo].flags = TRIPINFO_FLAG_OK;
//
//  Run number
//
//  If need be, locate the trip in the Runs Table
//
    for(nI = 0; nI < MAXRELIEFSPERTRIP; nI++)
    {
      m_TripInfo[m_numInTripInfo].runNumber[nI] = NO_RECORD;
    }
    reliefAtNODESrecordID = NO_RECORD;
    bFound = FALSE;
    if(startTRIPSrecordID == NO_RECORD || TRIPS.recordID == endTRIPSrecordID)
    {
      RUNSKey1.DIVISIONSrecordID = m_DailyOpsRUNSDivisionInEffect;
      RUNSKey1.SERVICESrecordID = m_pBlockInfo->SGRPSERVICESrecordID;
      RUNSKey1.runNumber = NO_RECORD;
      RUNSKey1.pieceNumber = NO_RECORD;
      rcode2 = btrieve(B_GETGREATER, TMS_RUNS, &RUNS, &RUNSKey1, 1);
      while(rcode2 == 0 &&
            RUNS.DIVISIONSrecordID == m_DailyOpsRUNSDivisionInEffect &&
            RUNS.SERVICESrecordID == m_pBlockInfo->SGRPSERVICESrecordID)
      {
        if(RUNS.start.TRIPSrecordID == TRIPS.recordID)
        {
          startTRIPSrecordID = RUNS.start.TRIPSrecordID;
          endTRIPSrecordID = RUNS.end.TRIPSrecordID;
          bFound = TRUE;
          break;
        }
        rcode2 = btrieve(B_GETNEXT, TMS_RUNS, &RUNS, &RUNSKey1, 1);
      }
    }
    if(bFound)
    {
      RUNSrecordID = RUNS.recordID;
      runNumber = RUNS.runNumber;
      if(previousRunNumber != NO_RECORD)
      {
        m_TripInfo[m_numInTripInfo].runNumber[0] = previousRunNumber;
        m_TripInfo[m_numInTripInfo].runNumber[1] = RUNS.runNumber;
        reliefAtNODESrecordID = RUNS.start.NODESrecordID;
      }
      else
      {
        m_TripInfo[m_numInTripInfo].runNumber[0] = RUNS.runNumber;
      }
      previousRunNumber = RUNS.runNumber;
    }
    else
    {
      m_TripInfo[m_numInTripInfo].runNumber[0] = previousRunNumber;
    }
//
//  Roster number
//
    m_TripInfo[m_numInTripInfo].rosterNumber[0] = NO_RECORD;
    m_TripInfo[m_numInTripInfo].rosterNumber[1] = NO_RECORD;
    if(bFound)
    {
      if(previousRosterNumber != NO_RECORD)
      {
        m_TripInfo[m_numInTripInfo].rosterNumber[0] = previousRosterNumber;
      }
      ROSTERKey1.DIVISIONSrecordID = m_DailyOpsROSTERDivisionInEffect;
      ROSTERKey1.rosterNumber = NO_RECORD;
      rcode2 = btrieve(B_GETGREATER, TMS_ROSTER, &ROSTER, &ROSTERKey1, 1);
      while(rcode2 == 0 &&
            ROSTER.DIVISIONSrecordID == m_DailyOpsROSTERDivisionInEffect)
      {
        bCrewOnly = (ROSTER.WEEK[m_RosterWeek].flags & (1 << today));
        if(!bCrewOnly)
        {
          RUNSKey0.recordID = ROSTER.WEEK[m_RosterWeek].RUNSrecordIDs[today];
          rcode2 = btrieve(B_GETEQUAL, TMS_RUNS, &RUNS, &RUNSKey0, 0);
          if(rcode2 == 0 && RUNS.runNumber == runNumber)
          {
            if(ROSTER.rosterNumber != previousRosterNumber && previousRosterNumber != NO_RECORD)
            {
              m_TripInfo[m_numInTripInfo].rosterNumber[0] = previousRosterNumber;
              m_TripInfo[m_numInTripInfo].rosterNumber[1] = ROSTER.rosterNumber;
            }
            else
            {
              m_TripInfo[m_numInTripInfo].rosterNumber[0] = ROSTER.rosterNumber;
            }
            previousRosterNumber = ROSTER.rosterNumber;
            break;
          }
        }
        rcode2 = btrieve(B_GETNEXT, TMS_ROSTER, &ROSTER, &ROSTERKey1, 1);
      }
    }
    else
    {
      m_TripInfo[m_numInTripInfo].rosterNumber[0] = previousRosterNumber;
    }
//
//  Operator
//
    for(nI = 0; nI < 2; nI++)
    {
      m_TripInfo[m_numInTripInfo].DRIVERSrecordID[nI] = NO_RECORD;
      if(m_TripInfo[m_numInTripInfo].rosterNumber[nI] == NO_RECORD)
      {
        continue;
      }
      ROSTERKey1.DIVISIONSrecordID = m_DailyOpsROSTERDivisionInEffect;
      ROSTERKey1.rosterNumber = m_TripInfo[m_numInTripInfo].rosterNumber[nI];
      rcode2 = btrieve(B_GETEQUAL, TMS_ROSTER, &ROSTER, &ROSTERKey1, 1);
      if(rcode2 == 0)
      {
        m_TripInfo[m_numInTripInfo].DRIVERSrecordID[nI] = ROSTER.DRIVERSrecordID;
      }
    }
//
//  Relief information
//
//  Relief At
//
    m_TripInfo[m_numInTripInfo].reliefAtNODESrecordID = reliefAtNODESrecordID;
    m_TripInfo[m_numInTripInfo].reliefAtTime = NO_TIME;
    if(reliefAtNODESrecordID != NO_RECORD)
    {
//
//  Relief time
//
//  Find the node on the pattern
//
      strcpy(tempString, "");
      patternIndex = 0;
      PATTERNSKey2.ROUTESrecordID = TRIPS.ROUTESrecordID;
      PATTERNSKey2.SERVICESrecordID = TRIPS.SERVICESrecordID;
      PATTERNSKey2.PATTERNNAMESrecordID = TRIPS.PATTERNNAMESrecordID;
      PATTERNSKey2.directionIndex = TRIPS.directionIndex;
      PATTERNSKey2.nodeSequence = NO_RECORD;
      rcode2 = btrieve(B_GETGREATER, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
      while(rcode2 == 0 &&
            PATTERNS.ROUTESrecordID == TRIPS.ROUTESrecordID &&
            PATTERNS.SERVICESrecordID == TRIPS.SERVICESrecordID &&
            PATTERNS.PATTERNNAMESrecordID == TRIPS.PATTERNNAMESrecordID &&
            PATTERNS.directionIndex == TRIPS.directionIndex)
      {
        if(!(PATTERNS.flags & PATTERNS_FLAG_BUSSTOP))
        {
          if(PATTERNS.NODESrecordID == reliefAtNODESrecordID)
          {
            m_TripInfo[m_numInTripInfo].reliefAtTime = GTResults.tripTimes[patternIndex];
            break;
          }
          patternIndex++;
        }
        rcode2 = btrieve(B_GETNEXT, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
      }
    }
//
//  POG and POT
//
    m_TripInfo[m_numInTripInfo].POGNODESrecordID = pTRIPSChunk->POGNODESrecordID;
    m_TripInfo[m_numInTripInfo].POTime = NO_TIME;
    if(pTRIPSChunk->POGNODESrecordID != NO_RECORD)
    {
      m_TripInfo[m_numInTripInfo].POTime = m_pBlockInfo->POTime;
    }
//
//  Route
//
    m_TripInfo[m_numInTripInfo].ROUTESrecordID = TRIPS.ROUTESrecordID;
//
//  Pattern
//
    m_TripInfo[m_numInTripInfo].PATTERNNAMESrecordID = TRIPS.PATTERNNAMESrecordID;
//
//  FNode
//
    m_TripInfo[m_numInTripInfo].fromNODESrecordID = GTResults.firstNODESrecordID;
//
//  FTime
//
    m_TripInfo[m_numInTripInfo].fromTime = GTResults.firstNodeTime;
//
//  TTime
//
    m_TripInfo[m_numInTripInfo].toTime = GTResults.lastNodeTime;
//
//  TNode
//
    m_TripInfo[m_numInTripInfo].toNODESrecordID = GTResults.lastNODESrecordID;
//
//  PIG and PIT
//
    m_TripInfo[m_numInTripInfo].PIGNODESrecordID = pTRIPSChunk->PIGNODESrecordID;
    m_TripInfo[m_numInTripInfo].PITime = NO_TIME;
    if(pTRIPSChunk->PIGNODESrecordID != NO_RECORD)
    {
      m_TripInfo[m_numInTripInfo].PITime = m_pBlockInfo->PITime;
    }
//
//  Get the next record
//
    m_numInTripInfo++;
    rcode2 = btrieve(B_GETNEXT, TMS_TRIPS, &TRIPS, &TRIPSKey2, keyNumber);
  }
//
//  Display the structure
//
  for(nI = 0; nI < m_numInTripInfo; nI++)
  {
    DisplayRow(nI);
  }

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CDailyOpsBP::DisplayRow(int rowNumber)
{
  LVITEM LVI;
  char   szRosterNumber[2][16];
  int    nI;
//
//  Trip number
//
  if(m_TripInfo[rowNumber].tripNumber > 0)
  {
    sprintf(tempString, "%ld", m_TripInfo[rowNumber].tripNumber);
  }
  else
  {
    strcpy(tempString, "");
  }
  LVI.mask = LVIF_TEXT | LVIF_PARAM;
  LVI.lParam = TRIPS.recordID;
  LVI.iItem = rowNumber;
  LVI.iSubItem = 0;
  LVI.pszText = tempString;
  LVI.iItem = pListCtrlTRIPLIST->InsertItem(&LVI);
//
//  Status
//
  if(m_TripInfo[rowNumber].flags & TRIPINFO_FLAG_OK)
  {
    strcpy(tempString, "OK");
  }
  else if(m_TripInfo[rowNumber].flags & TRIPINFO_FLAG_LATELEAVING)
  {
    strcpy(tempString, "Late dep");
  }
  else if(m_TripInfo[rowNumber].flags & TRIPINFO_FLAG_LATEARRIVING)
  {
    strcpy(tempString, "Late arr");
  }
  else if(m_TripInfo[rowNumber].flags & TRIPINFO_FLAG_DROPPED)
  {
    strcpy(tempString, "Dropped");
  }
  else if(m_TripInfo[rowNumber].flags & TRIPINFO_FLAG_REPATTERNED)
  {
    strcpy(tempString, "Repatterned");
  }
  LVI.mask = LVIF_TEXT;
  LVI.iItem = rowNumber;
  LVI.iSubItem = 1;
  LVI.pszText = tempString;
  LVI.iItem = pListCtrlTRIPLIST->SetItem(&LVI);
//
//  Run number
//
  if(m_TripInfo[rowNumber].runNumber[1] == NO_RECORD)
  {
    sprintf(tempString, "%ld", m_TripInfo[rowNumber].runNumber[0]);
  }
  else
  {
    sprintf(tempString, "%ld/%ld", m_TripInfo[rowNumber].runNumber[0], m_TripInfo[rowNumber].runNumber[1]);
  }
  LVI.mask = LVIF_TEXT;
  LVI.iItem = rowNumber;
  LVI.iSubItem = 2;
  LVI.pszText = tempString;
  LVI.iItem = pListCtrlTRIPLIST->SetItem(&LVI);
//
//  Roster
//
  for(nI = 0; nI < 2; nI++)
  {
    if(m_TripInfo[rowNumber].rosterNumber[nI] == NO_RECORD)
    {
      strcpy(szRosterNumber[nI], "-");
    }
    else
    {
      sprintf(szRosterNumber[nI], "%ld", m_TripInfo[rowNumber].rosterNumber[nI]);
    }
  }
  if(m_TripInfo[rowNumber].rosterNumber[1] == NO_RECORD)
  {
    strcpy(tempString, szRosterNumber[0]);
  }
  else
  {
    sprintf(tempString, "%s/%s", szRosterNumber[0], szRosterNumber[1]);
  }
  LVI.mask = LVIF_TEXT;
  LVI.iItem = rowNumber;
  LVI.iSubItem = 3;
  LVI.pszText = tempString;
  LVI.iItem = pListCtrlTRIPLIST->SetItem(&LVI);
//
//  Operator
//
  if(m_TripInfo[rowNumber].DRIVERSrecordID[0] == NO_RECORD)
  {
    strcpy(tempString, "-");
  }
  else
  {
    strcpy(tempString, BuildOperatorString(m_TripInfo[rowNumber].DRIVERSrecordID[0]));
  }
  if(m_TripInfo[rowNumber].DRIVERSrecordID[1] != NO_RECORD)
  {
    strcat(tempString, "/");
    if(m_TripInfo[rowNumber].DRIVERSrecordID[0] == NO_RECORD)
    {
      strcat(tempString, "-");
    }
    else
    {
      strcat(tempString, BuildOperatorString(m_TripInfo[rowNumber].DRIVERSrecordID[1]));
    }
  }
  LVI.mask = LVIF_TEXT;
  LVI.iItem = rowNumber;
  LVI.iSubItem = 4;
  LVI.pszText = tempString;
  LVI.iItem = pListCtrlTRIPLIST->SetItem(&LVI);
//
//  Relief information
//
//  Relief At
//
  if(m_TripInfo[rowNumber].reliefAtNODESrecordID != NO_RECORD)
  {
    NODESKey0.recordID = m_TripInfo[rowNumber].reliefAtNODESrecordID;
    btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
    strncpy(tempString, NODES.abbrName, NODES_ABBRNAME_LENGTH);
    trim(tempString, NODES_ABBRNAME_LENGTH);
    LVI.mask = LVIF_TEXT;
    LVI.iItem = rowNumber;
    LVI.iSubItem = 5;
    LVI.pszText = tempString;
    LVI.iItem = pListCtrlTRIPLIST->SetItem(&LVI);
//
//  Relief time
//
//  Find the node on the pattern
//
    strcpy(tempString, Tchar(m_TripInfo[rowNumber].reliefAtTime));
    LVI.mask = LVIF_TEXT;
    LVI.iItem = rowNumber;
    LVI.iSubItem = 6;
    LVI.pszText = tempString;
    LVI.iItem = pListCtrlTRIPLIST->SetItem(&LVI);
  }
//
//  POG and POT
//
  if(m_TripInfo[rowNumber].POGNODESrecordID != NO_RECORD)
  {
//
//  POG
//
    NODESKey0.recordID = m_TripInfo[rowNumber].POGNODESrecordID;
    btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
    strncpy(tempString, NODES.abbrName, NODES_ABBRNAME_LENGTH);
    trim(tempString, NODES_ABBRNAME_LENGTH);
    LVI.mask = LVIF_TEXT;
    LVI.iItem = rowNumber;
    LVI.iSubItem = 7;
    LVI.pszText = tempString;
    LVI.iItem = pListCtrlTRIPLIST->SetItem(&LVI);
//
//  POT
//
    sprintf(tempString, "%s", Tchar(m_TripInfo[rowNumber].POTime));
    LVI.mask = LVIF_TEXT;
    LVI.iItem = rowNumber;
    LVI.iSubItem = 8;
    LVI.pszText = tempString;
    LVI.iItem = pListCtrlTRIPLIST->SetItem(&LVI);
  }
//
//  Route
//
  ROUTESKey0.recordID = m_TripInfo[rowNumber].ROUTESrecordID;
  btrieve(B_GETEQUAL, TMS_ROUTES, &ROUTES, &ROUTESKey0, 0);
  strncpy(tempString, ROUTES.number, ROUTES_NUMBER_LENGTH);
  trim(tempString, ROUTES_NUMBER_LENGTH);
  LVI.mask = LVIF_TEXT;
  LVI.iItem = rowNumber;
  LVI.iSubItem = 9;
  LVI.pszText = tempString;
  LVI.iItem = pListCtrlTRIPLIST->SetItem(&LVI);
//
//  Pattern
//
  PATTERNNAMESKey0.recordID = m_TripInfo[rowNumber].PATTERNNAMESrecordID;
  btrieve(B_GETEQUAL, TMS_PATTERNNAMES, &PATTERNNAMES, &PATTERNNAMESKey0, 0);
  strncpy(tempString, PATTERNNAMES.name, PATTERNNAMES_NAME_LENGTH);
  trim(tempString, PATTERNNAMES_NAME_LENGTH);
  LVI.mask = LVIF_TEXT;
  LVI.iItem = rowNumber;
  LVI.iSubItem = 10;
  LVI.pszText = tempString;
  LVI.iItem = pListCtrlTRIPLIST->SetItem(&LVI);
//
//  FNode
//
  NODESKey0.recordID = m_TripInfo[rowNumber].fromNODESrecordID;
  btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
  strncpy(tempString, NODES.abbrName, NODES_ABBRNAME_LENGTH);
  trim(tempString, NODES_ABBRNAME_LENGTH);
  LVI.mask = LVIF_TEXT;
  LVI.iItem = rowNumber;
  LVI.iSubItem = 11;
  LVI.pszText = tempString;
  LVI.iItem = pListCtrlTRIPLIST->SetItem(&LVI);
//
//  FTime
//
  sprintf(tempString, "%s", Tchar(m_TripInfo[rowNumber].fromTime));
  LVI.mask = LVIF_TEXT;
  LVI.iItem = rowNumber;
  LVI.iSubItem = 12;
  LVI.pszText = tempString;
  LVI.iItem = pListCtrlTRIPLIST->SetItem(&LVI);
//
//  TTime
//
  sprintf(tempString, "%s", Tchar(m_TripInfo[rowNumber].toTime));
  LVI.mask = LVIF_TEXT;
  LVI.iItem = rowNumber;
  LVI.iSubItem = 13;
  LVI.pszText = tempString;
  LVI.iItem = pListCtrlTRIPLIST->SetItem(&LVI);
//
//  TNode
//
  NODESKey0.recordID = m_TripInfo[rowNumber].toNODESrecordID;
  btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
  strncpy(tempString, NODES.abbrName, NODES_ABBRNAME_LENGTH);
  trim(tempString, NODES_ABBRNAME_LENGTH);
  LVI.mask = LVIF_TEXT;
  LVI.iItem = rowNumber;
  LVI.iSubItem = 14;
  LVI.pszText = tempString;
  LVI.iItem = pListCtrlTRIPLIST->SetItem(&LVI);
//
//  PIG and PIT
//
  if(m_TripInfo[rowNumber].PIGNODESrecordID != NO_RECORD)
  {
//
//  PIG
//
    NODESKey0.recordID = m_TripInfo[rowNumber].PIGNODESrecordID;
    btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
    strncpy(tempString, NODES.abbrName, NODES_ABBRNAME_LENGTH);
    trim(tempString, NODES_ABBRNAME_LENGTH);
    LVI.mask = LVIF_TEXT;
    LVI.iItem = rowNumber;
    LVI.iSubItem = 15;
    LVI.pszText = tempString;
    LVI.iItem = pListCtrlTRIPLIST->SetItem(&LVI);
//
//  PIT
//
    sprintf(tempString, "%s", Tchar(m_TripInfo[rowNumber].PITime));
    LVI.mask = LVIF_TEXT;
    LVI.iItem = rowNumber;
    LVI.iSubItem = 16;
    LVI.pszText = tempString;
    LVI.iItem = pListCtrlTRIPLIST->SetItem(&LVI);
  }
}

void CDailyOpsBP::OnClose() 
{
	CDialog::OnClose();
}

void CDailyOpsBP::OnHelp() 
{
}

void CDailyOpsBP::OnOK() 
{
	CDialog::OnOK();
}

void CDailyOpsBP::OnModify() 
{
  LVITEM LVI;
  int nI = pListCtrlTRIPLIST->GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);

  if(nI < 0)
  {
    return;
  }
  else
  {
    LVI.mask = LVIF_PARAM;
    LVI.iItem = nI;
    LVI.iSubItem = 0;
    pListCtrlTRIPLIST->GetItem(&LVI);
    TRIPS.recordID = LVI.lParam;
  }
}

void CDailyOpsBP::OnClickTriplist(NMHDR* pNMHDR, LRESULT* pResult) 
{
  int nI = pListCtrlTRIPLIST->GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);

  pButtonIDMODIFY->EnableWindow(nI > 0 ? TRUE : FALSE);
	
	*pResult = 0;
}

void CDailyOpsBP::OnDblclkTriplist(NMHDR* pNMHDR, LRESULT* pResult) 
{
	OnModify();

	*pResult = 0;
}

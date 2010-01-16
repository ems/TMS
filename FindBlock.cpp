//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
// FindBlock.cpp : implementation file
//

#include "stdafx.h"
extern "C"
{
#include "TMSHeader.h"
}
#include <math.h>
#include "FindBlock.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define BLOCKS_SUBITEM_BEFOREAFTER 0
#define BLOCKS_SUBITEM_DEADHEAD    1
#define BLOCKS_SUBITEM_LAYOVER     2
#define BLOCKS_SUBITEM_RGRP        3
#define BLOCKS_SUBITEM_BLOCKNUMBER 4
#define BLOCKS_SUBITEM_FROMNODE    5
#define BLOCKS_SUBITEM_FROMTIME    6
#define BLOCKS_SUBITEM_TOTIME      7
#define BLOCKS_SUBITEM_TONODE      8

/////////////////////////////////////////////////////////////////////////////
// CFindBlock dialog


CFindBlock::CFindBlock(CWnd* pParent, long TRIPSrecordID)
	: CDialog(CFindBlock::IDD, pParent)
{
  m_TRIPSrecordID = TRIPSrecordID;

	//{{AFX_DATA_INIT(CFindBlock)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CFindBlock::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CFindBlock)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CFindBlock, CDialog)
	//{{AFX_MSG_MAP(CFindBlock)
	ON_BN_CLICKED(IDHELP, OnHelp)
	ON_WM_CLOSE()
	ON_NOTIFY(NM_CLICK, FINDBLOCK_BLOCKS, OnClickBlocks)
	ON_NOTIFY(NM_DBLCLK, FINDBLOCK_BLOCKS, OnDblclkBlocks)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFindBlock message handlers

BOOL CFindBlock::OnInitDialog() 
{
	CDialog::OnInitDialog();

  pButtonTRIPGROUPBOX = (CButton *)GetDlgItem(FINDBLOCK_TRIPGROUPBOX);
  pListCtrlTRIP = (CListCtrl *)GetDlgItem(FINDBLOCK_TRIP);
  pListCtrlBLOCKS = (CListCtrl *)GetDlgItem(FINDBLOCK_BLOCKS);
  pButtonHOOK = (CButton *)GetDlgItem(IDOK);

  char nodeName[NODES_ABBRNAME_LENGTH + 1];
  int  rcode2;
  int  pos;
  int  nI;

  DWORD dwExStyles = pListCtrlTRIP->GetExtendedStyle();
  pListCtrlTRIP->SetExtendedStyle(dwExStyles | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_INFOTIP);
  dwExStyles = pListCtrlBLOCKS->GetExtendedStyle();
  pListCtrlBLOCKS->SetExtendedStyle(dwExStyles | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_INFOTIP);
//
//  Display the trip
//
  LVCOLUMN LVC;
//
//  Get the trip
//
  TRIPSKey0.recordID = m_TRIPSrecordID;
  btrieve(B_GETEQUAL, TMS_TRIPS, &TRIPS, &TRIPSKey0, 0);
//
//  It can't have a block number
//
  if(TRIPS.standard.blockNumber > 0)
  {
    TMSError(NULL, MB_ICONSTOP, ERROR_288, (HANDLE)NULL);
    OnCancel();
    return TRUE;
  }
//
//  Set globals
//
  m_ROUTESrecordID = TRIPS.ROUTESrecordID;
  m_SERVICESrecordID = TRIPS.SERVICESrecordID;
  m_PATTERNNAMESrecordID = TRIPS.PATTERNNAMESrecordID;
//
//  Clear out the previous ListCtrl
//
  pListCtrlTRIP->DeleteAllItems();

  CHeaderCtrl* pCtrl = pListCtrlTRIP->GetHeaderCtrl();

  if(pCtrl != NULL)
  {
    int nCount = pCtrl->GetItemCount();
    for(nI = 0; nI < nCount; nI++)
    {
      pCtrl->DeleteItem(0);
    }
  }
//
//  Set up the list control
//
  GenerateTripDef GTResults;

  int numNodes = GenerateTrip(TRIPS.ROUTESrecordID, TRIPS.SERVICESrecordID,
        TRIPS.directionIndex, TRIPS.PATTERNNAMESrecordID,
        TRIPS.timeAtMLP, GENERATETRIP_FLAG_DISPLAYERRORS, &GTResults);
 
  m_firstNODESrecordID = GTResults.firstNODESrecordID;
  m_firstNodeTime = GTResults.firstNodeTime;
  m_lastNODESrecordID = GTResults.lastNODESrecordID;
  m_lastNodeTime = GTResults.lastNodeTime;

  pos = 0;
//
//  Set the header to the pattern of the trip
//
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
      NODESKey0.recordID = PATTERNS.NODESrecordID;
      btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
      strncpy(nodeName, NODES.abbrName, NODES_ABBRNAME_LENGTH);
      trim(nodeName, NODES_ABBRNAME_LENGTH);
      LVC.mask = LVCF_FMT | LVCF_TEXT | LVCF_WIDTH;
      LVC.fmt = LVCFMT_LEFT;
      LVC.cx = 50;
      LVC.pszText = nodeName;
      pListCtrlTRIP->InsertColumn(pos, &LVC);
      pos++;
    }
    rcode2 = btrieve(B_GETNEXT, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
  }
//
//  Display the times
//
  LVITEM LVI;

  LVI.iItem = 0;
  LVI.mask = LVIF_TEXT;
  pos = 0;
//
//  Do the trip times
//
  strcpy(tempString, Tchar(GTResults.tripTimes[0]));
  LVI.pszText = tempString;
  LVI.iSubItem = pos;
  if(pos == 0)
    pListCtrlTRIP->InsertItem(&LVI);
  else
  {
    pListCtrlTRIP->SetItem(&LVI);
  }
  pos++;
  for(nI = 1; nI < numNodes; nI++)
  {  
    LVI.iSubItem = pos;
    strcpy(tempString, Tchar(GTResults.tripTimes[nI]));
    LVI.pszText = tempString;
    pListCtrlTRIP->SetItem(&LVI);
    pos++;
  }
//
//  Show the route, service, direction, and pattern on the group box
//
  char routeNumberAndName[ROUTES_NUMBER_LENGTH + 3 + ROUTES_NAME_LENGTH + 1];
  char serviceName[SERVICES_NAME_LENGTH + 1];
  char directionName[DIRECTIONS_LONGNAME_LENGTH + 1];
  char patternName[PATTERNNAMES_NAME_LENGTH + 1];

  ROUTESKey0.recordID = TRIPS.ROUTESrecordID;
  btrieve(B_GETEQUAL, TMS_ROUTES, &ROUTES, &ROUTESKey0, 0);
  strncpy(routeNumberAndName, ROUTES.number, ROUTES_NUMBER_LENGTH);
  trim(routeNumberAndName, ROUTES_NUMBER_LENGTH);
  strcat(routeNumberAndName, " - ");
  strncpy(tempString, ROUTES.name, ROUTES_NAME_LENGTH);
  trim(tempString, ROUTES_NAME_LENGTH);
  strcat(routeNumberAndName, tempString);

  SERVICESKey0.recordID = TRIPS.SERVICESrecordID;
  btrieve(B_GETEQUAL, TMS_SERVICES, &SERVICES, &SERVICESKey0, 0);
  strncpy(serviceName, SERVICES.name, SERVICES_NAME_LENGTH);
  trim(serviceName, SERVICES_NAME_LENGTH);

  DIRECTIONSKey0.recordID = ROUTES.DIRECTIONSrecordID[TRIPS.directionIndex];
  btrieve(B_GETEQUAL, TMS_DIRECTIONS, &DIRECTIONS, &DIRECTIONSKey0, 0);
  strncpy(directionName, DIRECTIONS.longName, DIRECTIONS_LONGNAME_LENGTH);
  trim(directionName, DIRECTIONS_LONGNAME_LENGTH);

  PATTERNNAMESKey0.recordID = TRIPS.PATTERNNAMESrecordID;
  btrieve(B_GETEQUAL, TMS_PATTERNNAMES, &PATTERNNAMES, &PATTERNNAMESKey0, 0);
  strncpy(patternName, PATTERNNAMES.name, PATTERNNAMES_NAME_LENGTH);
  trim(patternName, PATTERNNAMES_NAME_LENGTH);

  sprintf(tempString, "This trip is a %s %s %s on pattern %s",
        serviceName, directionName, routeNumberAndName, patternName);
  pButtonTRIPGROUPBOX->SetWindowText(tempString);
//
//  Set up the BLOCKS list control
//
//  Columns
//
#define NUMTITLES 9
  char *Titles[] = {"B/A",  "Dhd", "Lay", "RGRP", "Blk #", "FNode",  "FTime", "TTime", "TNode"};
//
//  Set up the list control
//
  LVC.mask = LVCF_FMT | LVCF_TEXT | LVCF_WIDTH;
  LVC.fmt = LVCFMT_LEFT;
  for(nI = 0; nI < NUMTITLES; nI++)
  {
    LVC.cx = 50;
    LVC.pszText = Titles[nI];
    pListCtrlBLOCKS->InsertColumn(nI, &LVC);
  }
//
//  Get the list of routes
//
  m_ROUTESrecordIDs.RemoveAll();
  rcode2 = btrieve(B_GETFIRST, TMS_ROUTES, &ROUTES, &ROUTESKey1, 1);
  while(rcode2 == 0)
  {
    m_ROUTESrecordIDs.Add(ROUTES.recordID);
    rcode2 = btrieve(B_GETNEXT, TMS_ROUTES, &ROUTES, &ROUTESKey1, 1);
  }
//
//  Display the eligible blocks
//
  DisplayBlocks();
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

//
//  OnOK() - Process the hook request
//

void CFindBlock::OnOK() 
{
  POSITION pos = pListCtrlBLOCKS->GetFirstSelectedItemPosition();
  int  nItem;
//
//  Determine the selection(s)
//
  if(pos != NULL)
  {
    nItem = pListCtrlBLOCKS->GetNextSelectedItem(pos);
//
//  Get the trip
//
    TRIPSKey0.recordID = m_TRIPSrecordID;
    btrieve(B_GETEQUAL, TMS_TRIPS, &TRIPS, &TRIPSKey0, 0);
//
//  Drop the values in
//
//  Block number
//
    CString s;
    s = pListCtrlBLOCKS->GetItemText(nItem, BLOCKS_SUBITEM_BLOCKNUMBER); 
    TRIPS.standard.blockNumber = atol(s);
//
//  BLOCKNAMESrecordID
//
    TRIPS.standard.BLOCKNAMESrecordID = NO_RECORD;
//
//  POGNODESrecordID
//
    TRIPS.standard.POGNODESrecordID = NO_RECORD;
//
//  PIGNODESrecordID
//
    TRIPS.standard.PIGNODESrecordID = NO_RECORD;
//
//  RGRPROUTESrecordID
//
    s = pListCtrlBLOCKS->GetItemText(nItem, BLOCKS_SUBITEM_RGRP);
    strcpy(ROUTESKey1.number, s);
    pad(ROUTESKey1.number, ROUTES_NUMBER_LENGTH);
    btrieve(B_GETEQUAL, TMS_ROUTES, &ROUTES, &ROUTESKey1, 1);
    TRIPS.standard.RGRPROUTESrecordID = ROUTES.recordID;
//
//  SGRPSERVICESrecordID
//
    TRIPS.standard.SGRPSERVICESrecordID = m_SERVICESrecordID;
//
//  assignedToNODESrecordID
//
    TRIPS.standard.assignedToNODESrecordID = pListCtrlBLOCKS->GetItemData(nItem);
//
//  Leave layoverMin and layoverMax the way they were
//
//  blockSequence
//
    TRIPS.standard.blockSequence = m_firstNodeTime;
//
//  Update the trip
//
    btrieve(B_UPDATE, TMS_TRIPS, &TRIPS, &TRIPSKey0, 0);
//
//  And tell him
//
    s.LoadString(TEXT_262);
    sprintf(tempString, s, TRIPS.standard.blockNumber);
    MessageBeep(MB_ICONINFORMATION);
    MessageBox(tempString, TMS, MB_ICONINFORMATION | MB_OK);

    CDialog::OnOK();
  }
}

//
//  OnCancel()
//

void CFindBlock::OnCancel() 
{
	CDialog::OnCancel();
}

//
//  OnClose()
//

void CFindBlock::OnClose() 
{
	CDialog::OnClose();
}

//
//  OnHelp()
//

void CFindBlock::OnHelp() 
{
	// TODO: Add your control notification handler code here
	
}

void CFindBlock::DisplayBlocks()
{
  GenerateTripDef GTResults;  
  GetConnectionTimeDef GCTData;
  CUIntArray blockNumbers;
  CUIntArray TRIPSrecordIDs;
  CUIntArray fromTime;
  CUIntArray fromNode;
  CUIntArray toNode;
  CUIntArray toTime;
  CUIntArray ROUTESrecordIDs;
  CUIntArray SERVICESrecordIDs;
  CUIntArray PATTERNNAMESrecordIDs;
  PassToAddDef PTAPending, PTA;
  float distance;
  BOOL bFirst;
  BOOL bGotAfter;
  long assignedToNODESrecordID;
  long previousBlock;
  long statbarTotal;
  long equivalentTravelTime;
  long dhd;
  long layover;
  int rcode2;
  int nI, nJ, nK;
  int numTrips;
  int pendingIndex;
//
//  Loop through the routes
//
  StatusBarStart(hWndMain, "Cycling through the routes");
  statbarTotal = m_ROUTESrecordIDs.GetSize();
  for(nI = 0; nI < statbarTotal; nI++)
  {
    ROUTESKey0.recordID = m_ROUTESrecordIDs.GetAt(nI);
    btrieve(B_GETEQUAL, TMS_ROUTES, &ROUTES, &ROUTESKey0, 0);
    if(StatusBarAbort())
      goto done;
    strncpy(tempString, ROUTES.number, ROUTES_NUMBER_LENGTH);
    trim(tempString, ROUTES_NUMBER_LENGTH);
    strcat(tempString, " - ");
    strncpy(szarString, ROUTES.name, ROUTES_NAME_LENGTH);
    trim(szarString, ROUTES_NAME_LENGTH);
    strcat(tempString, szarString);
    StatusBarText(tempString);
    StatusBar((long)nI, (long)statbarTotal);
//
//  Get all the trips on this route group
//
    m_bEstablishRUNTIMES = TRUE;
    NODESKey1.flags = NODES_FLAG_GARAGE;
    memset(NODESKey1.abbrName, 0x00, NODES_ABBRNAME_LENGTH);
    rcode2 = btrieve(B_GETGREATEROREQUAL, TMS_NODES, &NODES, &NODESKey1, 1);
    if(rcode2 != 0)  // No Garages
    {
      TMSError(NULL, MB_ICONSTOP, ERROR_045, (HANDLE)NULL);
      goto done;
    }
    bFirst = TRUE;
    while(rcode2 == 0 && NODES.flags & NODES_FLAG_GARAGE)
    {
//
//  Cycle through the blocks
//
      assignedToNODESrecordID = bFirst ? NO_RECORD : NODES.recordID;
      TRIPSKey2.assignedToNODESrecordID = assignedToNODESrecordID;
      TRIPSKey2.RGRPROUTESrecordID = ROUTES.recordID;
      TRIPSKey2.SGRPSERVICESrecordID = m_SERVICESrecordID;
      TRIPSKey2.blockNumber = 1;
      TRIPSKey2.blockSequence = NO_TIME;
      rcode2 = btrieve(B_GETGREATEROREQUAL, TMS_TRIPS, &TRIPS, &TRIPSKey2, 2);
//
//  Get a list of block numbers for this assignedToNODESrecordID / Route / Service
//
      blockNumbers.RemoveAll();
      previousBlock = NO_RECORD;
      while(rcode2 == 0 &&
            TRIPS.standard.assignedToNODESrecordID == assignedToNODESrecordID &&
            TRIPS.standard.RGRPROUTESrecordID == ROUTES.recordID &&
            TRIPS.standard.SGRPSERVICESrecordID == m_SERVICESrecordID)
      {
        if(StatusBarAbort())
          goto done;
        if(previousBlock != TRIPS.standard.blockNumber)
        {
          blockNumbers.Add(TRIPS.standard.blockNumber);
          previousBlock = TRIPS.standard.blockNumber;
        }
        rcode2 = btrieve(B_GETNEXT, TMS_TRIPS, &TRIPS, &TRIPSKey2, 2);
      }
//
//  Get trip data for exch trip on each block
//
      for(nJ = 0; nJ < blockNumbers.GetSize(); nJ++)
      {
//
//  Clear out the arrays
//
        TRIPSrecordIDs.RemoveAll();
        fromTime.RemoveAll();
        fromNode.RemoveAll();
        toNode.RemoveAll();
        toTime.RemoveAll();
        ROUTESrecordIDs.RemoveAll();
        SERVICESrecordIDs.RemoveAll();
        PATTERNNAMESrecordIDs.RemoveAll();
//
//  Key into the blocks
//
        TRIPSKey2.assignedToNODESrecordID = assignedToNODESrecordID;
        TRIPSKey2.RGRPROUTESrecordID = ROUTES.recordID;
        TRIPSKey2.SGRPSERVICESrecordID = m_SERVICESrecordID;
        TRIPSKey2.blockNumber = blockNumbers.GetAt(nJ);
        TRIPSKey2.blockSequence = NO_TIME;
        rcode2 = btrieve(B_GETGREATEROREQUAL, TMS_TRIPS, &TRIPS, &TRIPSKey2, 2);
        if(TRIPSKey2.blockNumber == 160)
        {
          int i=1;
        }
        while(rcode2 == 0 &&
              TRIPS.standard.assignedToNODESrecordID == assignedToNODESrecordID &&
              TRIPS.standard.RGRPROUTESrecordID == ROUTES.recordID &&
              TRIPS.standard.SGRPSERVICESrecordID == m_SERVICESrecordID &&
              TRIPS.standard.blockNumber == (long)blockNumbers.GetAt(nJ))
        {
          if(StatusBarAbort())
            goto done;
//
//  Generate the trip and save the values
//
          GenerateTrip(TRIPS.ROUTESrecordID, TRIPS.SERVICESrecordID,
                TRIPS.directionIndex, TRIPS.PATTERNNAMESrecordID,
                TRIPS.timeAtMLP, GENERATETRIP_FLAG_DISPLAYERRORS, &GTResults);
          TRIPSrecordIDs.Add(TRIPS.recordID);
          fromTime.Add(GTResults.firstNodeTime);
          fromNode.Add(GTResults.firstNODESrecordID);
          toNode.Add(GTResults.lastNODESrecordID);
          toTime.Add(GTResults.lastNodeTime);
          ROUTESrecordIDs.Add(TRIPS.ROUTESrecordID);
          SERVICESrecordIDs.Add(TRIPS.SERVICESrecordID);
          PATTERNNAMESrecordIDs.Add(TRIPS.PATTERNNAMESrecordID);
          rcode2 = btrieve(B_GETNEXT, TMS_TRIPS, &TRIPS, &TRIPSKey2, 2);
        }
//
//  Cycle through the saved values to see where we can put this trip on this block
//
        bGotAfter = FALSE;
        pendingIndex = NO_TIME;
        numTrips = TRIPSrecordIDs.GetSize();
        long ft, tt;
        for(nK = 0; nK < numTrips; nK++)
        {
//
//  See if our trip will fit before this one
//
          ft = fromTime.GetAt(nK);
          if(m_lastNodeTime < (long)fromTime.GetAt(nK) && (bGotAfter || numTrips == 1))
          {
//
//  Only do it, though, if the one before this was our pending entry
//  ("after" one trip and "before" the next means there's no existing trip)
//
            if(nK - 1 == pendingIndex || nK == numTrips - 1)
            {
//
//  And if we can get there from here
//
              dhd = 0;
              if(!NodesEquivalent(m_lastNODESrecordID,
                    fromNode.GetAt(nK), &equivalentTravelTime))
              {
                GCTData.fromROUTESrecordID = m_ROUTESrecordID;
                GCTData.fromSERVICESrecordID = m_SERVICESrecordID;
                GCTData.fromPATTERNNAMESrecordID = m_PATTERNNAMESrecordID;
                GCTData.fromNODESrecordID = m_lastNODESrecordID;
                GCTData.timeOfDay = m_lastNodeTime;
                GCTData.toROUTESrecordID = ROUTESrecordIDs.GetAt(nK);
                GCTData.toSERVICESrecordID = SERVICESrecordIDs.GetAt(nK);
                GCTData.toPATTERNNAMESrecordID = PATTERNNAMESrecordIDs.GetAt(nK);
                GCTData.toNODESrecordID = fromNode.GetAt(nK);
                dhd = GetConnectionTime(GCT_FLAG_DEADHEADTIME, &GCTData, &distance);
                distance = (float)fabs((double)distance);
              }
              layover = fromTime.GetAt(nK) - m_lastNodeTime;
              if(dhd != NO_TIME)
                layover -= dhd;
//
//  If the layover is >= 0, we'll consider it
//
              if(layover >= 0 && fromTime.GetAt(nK) - m_lastNodeTime <= 3600)
              {
                if(nK - 1 == pendingIndex)
                  AddToList(&PTAPending);
                TRIPSKey0.recordID = TRIPSrecordIDs.GetAt(nK);
                btrieve(B_GETEQUAL, TMS_TRIPS, &TRIPS, &TRIPSKey0, 0);
                PTA.assignedToNODESrecordID = assignedToNODESrecordID;
                PTA.RGRPROUTESrecordID = TRIPS.standard.RGRPROUTESrecordID;
                PTA.blockNumber = TRIPS.standard.blockNumber;
                PTA.layover = layover;
                PTA.dhd = dhd;
                PTA.fromTime = fromTime.GetAt(nK);
                PTA.fromNode = fromNode.GetAt(nK);
                PTA.toNode = toNode.GetAt(nK);
                PTA.toTime = toTime.GetAt(nK);
                PTA.flags = PASSTOADD_FLAG_BEFORE;
                AddToList(&PTA);
              }
              break;
            }
          }
//
//  The pending entry was more than one trip away - clear out bGotAfter
//
          else
          {
            bGotAfter = FALSE;
          }
//
//  See if our trip will fit after this one
//
          tt = toTime.GetAt(nK);
          if(m_firstNodeTime >= (long)toTime.GetAt(nK) && !bGotAfter)
          {
//
//  And if we can get there from here
//
            dhd = 0;
            if(!NodesEquivalent(m_firstNODESrecordID,
                  toNode.GetAt(nK), &equivalentTravelTime))
            {
              GCTData.fromROUTESrecordID = ROUTESrecordIDs.GetAt(nK);
              GCTData.fromSERVICESrecordID = SERVICESrecordIDs.GetAt(nK);
              GCTData.fromPATTERNNAMESrecordID = PATTERNNAMESrecordIDs.GetAt(nK);
              GCTData.fromNODESrecordID = toNode.GetAt(nK);
              GCTData.timeOfDay = m_firstNodeTime;
              GCTData.toROUTESrecordID = m_ROUTESrecordID;
              GCTData.toSERVICESrecordID = m_SERVICESrecordID;
              GCTData.toPATTERNNAMESrecordID = m_PATTERNNAMESrecordID;
              GCTData.toNODESrecordID = m_firstNODESrecordID;
              dhd = GetConnectionTime(GCT_FLAG_DEADHEADTIME, &GCTData, &distance);
              distance = (float)fabs((double)distance);
            }
            layover = m_firstNodeTime - toTime.GetAt(nK);
            if(dhd != NO_TIME)
              layover -= dhd;
//
//  If the layover is >= 0, we'll consider it
//
            if(layover >= 0)
            {
              TRIPSKey0.recordID = TRIPSrecordIDs.GetAt(nK);
              btrieve(B_GETEQUAL, TMS_TRIPS, &TRIPS, &TRIPSKey0, 0);
              PTAPending.assignedToNODESrecordID = assignedToNODESrecordID;
              PTAPending.RGRPROUTESrecordID = TRIPS.standard.RGRPROUTESrecordID;
              PTAPending.blockNumber = TRIPS.standard.blockNumber;
              PTAPending.layover = layover;
              PTAPending.dhd = dhd;
              PTAPending.fromTime = fromTime.GetAt(nK);
              PTAPending.fromNode = fromNode.GetAt(nK);
              PTAPending.toNode = toNode.GetAt(nK);
              PTAPending.toTime = toTime.GetAt(nK);
              PTAPending.flags = PASSTOADD_FLAG_AFTER;
              pendingIndex = nK;
              bGotAfter = TRUE;
              if(nK == numTrips - 1)
                AddToList(&PTAPending);
            }
          }
        }
      }
      if(bFirst)
        bFirst = FALSE;
      else
        rcode2 = btrieve(B_GETNEXT, TMS_NODES, &NODES, &NODESKey1, 1);
    }
  }
  done:
    StatusBarEnd();
}

void CFindBlock::AddToList(PassToAddDef *pPTA)
{
//
//  Display the times
//
  LVITEM LVI;

  LVI.iItem = pListCtrlBLOCKS->GetItemCount();
//
//  Before / After
//
  LVI.mask = LVIF_TEXT | LVIF_PARAM;
  LVI.lParam = pPTA->assignedToNODESrecordID;
  if(pPTA->flags & PASSTOADD_FLAG_BEFORE)
    strcpy(tempString, "Before");
  else
    strcpy(tempString, "After");
  LVI.pszText = tempString;
  LVI.iSubItem = BLOCKS_SUBITEM_BEFOREAFTER;
  pListCtrlBLOCKS->InsertItem(&LVI);
#define BLOCKS_SUBITEM_BEFOREAFTER 0
#define BLOCKS_SUBITEM_DEADHEAD    1
#define BLOCKS_SUBITEM_LAYOVER     2
#define BLOCKS_SUBITEM_RGRP        3
#define BLOCKS_SUBITEM_
#define BLOCKS_SUBITEM_
#define BLOCKS_SUBITEM_
#define BLOCKS_SUBITEM_
#define BLOCKS_SUBITEM_
#define BLOCKS_SUBITEM_
//
//  Deadhead
//
  LVI.mask = LVIF_TEXT;
  if(pPTA->dhd == NO_TIME)
    strcpy(tempString, "Undef");
  else
    strcpy(tempString, chhmm(pPTA->dhd));
  LVI.pszText = tempString;
  LVI.iSubItem = BLOCKS_SUBITEM_DEADHEAD;
  pListCtrlBLOCKS->SetItem(&LVI);
//
//  Layover
//
  strcpy(tempString, chhmm(pPTA->layover));
  LVI.pszText = tempString;
  LVI.iSubItem = BLOCKS_SUBITEM_LAYOVER;
  pListCtrlBLOCKS->SetItem(&LVI);
//
//  RGRP
//
  ROUTESKey0.recordID = pPTA->RGRPROUTESrecordID;
  btrieve(B_GETEQUAL, TMS_ROUTES, &ROUTES, &ROUTESKey0, 0);
  strncpy(tempString, ROUTES.number, ROUTES_NUMBER_LENGTH);
  trim(tempString, ROUTES_NUMBER_LENGTH);
  LVI.pszText = tempString;
  LVI.iSubItem = BLOCKS_SUBITEM_RGRP;
  pListCtrlBLOCKS->SetItem(&LVI);
//
//  Block number
//
  sprintf(tempString, "%ld", pPTA->blockNumber);
  LVI.pszText = tempString;
  LVI.iSubItem = BLOCKS_SUBITEM_BLOCKNUMBER;
  pListCtrlBLOCKS->SetItem(&LVI);
//
//  From node
//
  NODESKey0.recordID = pPTA->fromNode;
  btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
  strncpy(tempString, NODES.abbrName, NODES_ABBRNAME_LENGTH);
  trim(tempString, NODES_ABBRNAME_LENGTH);
  LVI.pszText = tempString;
  LVI.iSubItem = BLOCKS_SUBITEM_FROMNODE;
  pListCtrlBLOCKS->SetItem(&LVI);
//
//  From time
//
  strcpy(tempString, Tchar(pPTA->fromTime));
  LVI.pszText = tempString;
  LVI.iSubItem = BLOCKS_SUBITEM_FROMTIME;
  pListCtrlBLOCKS->SetItem(&LVI);
//
//  To time
//
  strcpy(tempString, Tchar(pPTA->toTime));
  LVI.pszText = tempString;
  LVI.iSubItem = BLOCKS_SUBITEM_TOTIME;
  pListCtrlBLOCKS->SetItem(&LVI);
//
//  To node
//
  NODESKey0.recordID = pPTA->toNode;
  btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
  strncpy(tempString, NODES.abbrName, NODES_ABBRNAME_LENGTH);
  trim(tempString, NODES_ABBRNAME_LENGTH);
  LVI.pszText = tempString;
  LVI.iSubItem = BLOCKS_SUBITEM_TONODE;
  pListCtrlBLOCKS->SetItem(&LVI);
}

void CFindBlock::OnClickBlocks(NMHDR* pNMHDR, LRESULT* pResult) 
{
  pButtonHOOK->EnableWindow(TRUE);
	
	*pResult = 0;
}

void CFindBlock::OnDblclkBlocks(NMHDR* pNMHDR, LRESULT* pResult) 
{
  OnOK();
	
	*pResult = 0;
}

// DailyOpsRS.cpp : implementation file
//

#include "stdafx.h"
extern "C"{
#include "TMSHeader.h"
#include "cistms.h"
}
#include <math.h>
#include "tms.h"
#include "DailyOpsHeader.h"

extern "C"
{
int sort_RP(const void *a, const void *b)
{
  RPDef *pa, *pb;
  pa = (RPDef *)a;
  pb = (RPDef *)b;

  return(pa->time < pb->time ? -1 : pa->time > pb->time ? 1 : 0);
}
}

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDailyOpsRS dialog

CDailyOpsRS::CDailyOpsRS(CWnd* pParent, RunSplitDef* pPassedData)
	: CDialog(CDailyOpsRS::IDD, pParent)
{
  if(pPassedData == NULL)
  {
    OnCancel();
    return;
  }
  m_pPassedData = pPassedData;
  m_firstIndex = NO_RECORD;
  m_lastIndex = NO_RECORD;

  
	//{{AFX_DATA_INIT(CDailyOpsRS)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CDailyOpsRS::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDailyOpsRS)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDailyOpsRS, CDialog)
	//{{AFX_MSG_MAP(CDailyOpsRS)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDADD, OnAdd)
	ON_BN_CLICKED(IDREMOVE, OnRemove)
	ON_BN_CLICKED(IDHELP, OnHelp)
	ON_NOTIFY(NM_CLICK, DAILYOPSRS_POTENTIALSPLITS, OnClickPotentialsplits)
	ON_NOTIFY(NM_RCLICK, DAILYOPSRS_POTENTIALSPLITS, OnRclickPotentialsplits)
	ON_CBN_SELCHANGE(DAILYOPSRS_STARTSPLIT, OnSelchangeStartsplit)
	ON_CBN_SELCHANGE(DAILYOPSRS_ENDSPLIT, OnSelchangeEndsplit)
	ON_NOTIFY(NM_CLICK, DAILYOPSRS_CURRENTSPLITS, OnClickCurrentsplits)
	ON_BN_CLICKED(IDPREVIEW, OnPreview)
	ON_BN_CLICKED(IDDELETE, OnDelete)
	ON_BN_CLICKED(IDINSERT, OnInsert)
	ON_BN_CLICKED(IDREMOVEALL, OnRemoveall)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDailyOpsRS message handlers

BOOL CDailyOpsRS::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
//
//  Set up poiners to the controls
//
  pListCtrlPOTENTIALSPLITS = (CListCtrl *)GetDlgItem(DAILYOPSRS_POTENTIALSPLITS);
  pListCtrlCURRENTSPLITS = (CListCtrl *)GetDlgItem(DAILYOPSRS_CURRENTSPLITS);
  pComboBoxSTARTSPLIT = (CComboBox *)GetDlgItem(DAILYOPSRS_STARTSPLIT);
  pComboBoxENDSPLIT = (CComboBox *)GetDlgItem(DAILYOPSRS_ENDSPLIT);
  pButtonIDREMOVE = (CButton *)GetDlgItem(IDREMOVE);
  pButtonIDCANCEL = (CButton *)GetDlgItem(IDCANCEL);
//
//  Initialize m_RP
//
  int nI;

  for(nI = 0; nI < RPMAX; nI++)
  {
    m_RP[nI].TRIPSrecordID = NO_RECORD;
    m_RP[nI].NODESrecordID = NO_RECORD;
    m_RP[nI].time = NO_TIME;
    m_RP[nI].flags = 0;
    m_RP[nI].startDRIVERSrecordID = NO_RECORD;
    m_RP[nI].endDRIVERSrecordID = NO_RECORD;
    m_RP[nI].DAILYOPSrecordID = NO_RECORD;
  }
//
//  What type of run is this?
//
  m_bCrewOnly = (m_pPassedData->flags & RUNSPLIT_FLAGS_CREWONLY);
//
//  Get the run details
//
  long runNumber;
  int  rcode2;

  if(m_bCrewOnly)
  {
    CREWONLY.recordID = m_pPassedData->RUNSrecordID;
    rcode2 = btrieve(B_GETEQUAL, TMS_CREWONLY, &CREWONLY, &CREWONLYKey0, 0);
    runNumber = CREWONLY.runNumber;
  }
  else
  {
    RUNSKey0.recordID = m_pPassedData->RUNSrecordID;
    rcode2 = btrieve(B_GETEQUAL, TMS_RUNS, &RUNS, &RUNSKey0, 0);
    runNumber = RUNS.runNumber;
  }
  if(rcode2 != 0)
  {
    sprintf(tempString, "Invalid RUNSrecordID passed to DailyOpsRS\n\nValue was %ld", m_pPassedData->RUNSrecordID);
    MessageBox(tempString, TMS, MB_OK);
    OnCancel(); 
    return TRUE;
  }
//
//  Put the run number in the title.  On an automatic,
//  add the driver's name and a notification to the bar.
//
  GetWindowText(szFormatString, SZFORMATSTRING_LENGTH);
  sprintf(tempString, szFormatString, runNumber);
  if(m_pPassedData->flags & RUNSPLIT_FLAGS_AUTOMATIC)
  {
    strcat(tempString, " - Due to absence of ");
    strcat(tempString, BuildOperatorString(m_pPassedData->DRIVERSrecordID));
    pButtonIDCANCEL->EnableWindow(FALSE);
  }
  SetWindowText(tempString);
//
//  Set the styles for the list controls
//
  DWORD dwExStyles;

  dwExStyles = pListCtrlPOTENTIALSPLITS->GetExtendedStyle();
  pListCtrlPOTENTIALSPLITS->SetExtendedStyle(dwExStyles | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_INFOTIP);

  dwExStyles = pListCtrlCURRENTSPLITS->GetExtendedStyle();
  pListCtrlCURRENTSPLITS->SetExtendedStyle(dwExStyles | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_INFOTIP);
//
//  Set up the headers on the potential splits list control
//
  LVCOLUMN LVC;

  LVC.mask = LVCF_FMT | LVCF_TEXT | LVCF_WIDTH;
  LVC.fmt = LVCFMT_CENTER;
  LVC.cx = 42;
  LVC.pszText = "Status";
  pListCtrlPOTENTIALSPLITS->InsertColumn(0, &LVC);

  LVC.cx = 50;
  LVC.pszText = "Relief";
  pListCtrlPOTENTIALSPLITS->InsertColumn(1, &LVC);

  LVC.cx = 50;
  LVC.pszText = "At";
  pListCtrlPOTENTIALSPLITS->InsertColumn(2, &LVC);
//
//  Set up the headers on the current splits list control
//
  LVC.mask = LVCF_FMT | LVCF_TEXT | LVCF_WIDTH;
  LVC.fmt = LVCFMT_CENTER;
  LVC.cx = 25;
  LVC.pszText = "";
  pListCtrlCURRENTSPLITS->InsertColumn(0, &LVC);
 
  LVC.cx = 50;
  LVC.pszText = "Relief";
  pListCtrlCURRENTSPLITS->InsertColumn(1, &LVC);

  LVC.cx = 50;
  LVC.pszText = "At";
  pListCtrlCURRENTSPLITS->InsertColumn(2, &LVC);

  LVC.cx = 50;
  LVC.pszText = "Relief";
  pListCtrlCURRENTSPLITS->InsertColumn(3, &LVC);
  
  LVC.cx = 50;
  LVC.pszText = "At";
  pListCtrlCURRENTSPLITS->InsertColumn(4, &LVC);

  GenerateTripDef      GTResults;
  GetConnectionTimeDef GCTData;
  float distance;
  BOOL  bFound;
  BOOL  bStartOfPiece;
  BOOL  bEndOfPiece;
  BOOL  bCrewOnly;
  long  pieceNumber;
  long  serviceRecordID;
  long  runtype;
  long  dhdTime;
  long  serviceNumber;
  long  assignedToNODESrecordID;
  long  RGRPROUTESrecordID;
  long  SGRPSERVICESrecordID;
  long  blockNumber;
  int   counter;
  int   serviceIndex;

  m_numRP = 0;
//
//  Crew only runs - get the start/end timepoints and times
//
  if(m_bCrewOnly)
  {
    m_RP[m_numRP].NODESrecordID = CREWONLY.startNODESrecordID;
    m_RP[m_numRP].time = CREWONLY.startTime;
    m_RP[m_numRP].flags = RP_FLAGS_STARTOFPIECE;
    m_RP[m_numRP].TRIPSrecordID = NO_RECORD;
    m_numRP++;
    m_RP[m_numRP].NODESrecordID = CREWONLY.endNODESrecordID;
    m_RP[m_numRP].time = CREWONLY.endTime;
    m_RP[m_numRP].flags = RP_FLAGS_ENDOFPIECE;
    m_RP[m_numRP].TRIPSrecordID = NO_RECORD;
    m_numRP++;
  }
//
//  Regular runs are a little more complicated
//
  else
  {
//
//  Determine the service index/number
//
    SERVICESKey0.recordID = m_pPassedData->SERVICESrecordIDInEffect;
    btrieve(B_GETEQUAL, TMS_SERVICES, &SERVICES, &SERVICESKey0, 0);
    serviceNumber = SERVICES.number;
//
//  Set up the index into the node labels string
//
    serviceIndex = m_ServiceNumber - 1;
    if(serviceIndex < 0 || serviceIndex > NODES_RELIEFLABELS_LENGTH - 1)
    {
      serviceIndex = 0;
    }
//
//  Get some pertinent run information
//
    PROPOSEDRUNDef PROPOSEDRUN;
    COSTDef        COST;

    GetRunElements(NULL, &RUNS, &PROPOSEDRUN, &COST, TRUE);
    if(m_bCrewOnly)
    {
      CREWONLY.recordID = m_pPassedData->RUNSrecordID;
      rcode2 = btrieve(B_GETEQUAL, TMS_CREWONLY, &CREWONLY, &CREWONLYKey0, 0);
      runNumber = CREWONLY.runNumber;
    }
    else
    {
      RUNSKey0.recordID = m_pPassedData->RUNSrecordID;
      rcode2 = btrieve(B_GETEQUAL, TMS_RUNS, &RUNS, &RUNSKey0, 0);
      runNumber = RUNS.runNumber;
    }
//
//  Cycle through the run
//
    runNumber = RUNS.runNumber;
    serviceRecordID = RUNS.SERVICESrecordID;
    runtype = RUNS.cutAsRuntype;
    btrieve(B_GETPOSITION, TMS_RUNS, &RUNS, &RUNSKey0, 0);
    rcode2 = btrieve(B_GETDIRECT, TMS_RUNS, &RUNS, &RUNSKey1, 1);
    while(rcode2 == 0 &&
          runNumber == RUNS.runNumber &&
          serviceRecordID == RUNS.SERVICESrecordID)
    {
      pieceNumber = RUNS.pieceNumber - 1;
//
//  Get the report at location and time
//
      if(COST.TRAVEL[pieceNumber].startNODESrecordID == NO_RECORD)
      {
        m_RP[m_numRP].NODESrecordID = PROPOSEDRUN.piece[pieceNumber].fromNODESrecordID;
      }
      else
      {
        m_RP[m_numRP].NODESrecordID = COST.TRAVEL[pieceNumber].startNODESrecordID;
      }
      m_RP[m_numRP].time = PROPOSEDRUN.piece[pieceNumber].fromTime -
              COST.PIECECOST[pieceNumber].reportTime - COST.TRAVEL[pieceNumber].startTravelTime;
      if(PROPOSEDRUN.piece[pieceNumber].prior.startTime != NO_TIME)
      {
        m_RP[m_numRP].time -= PROPOSEDRUN.piece[pieceNumber].prior.startTime;
      }
      m_RP[m_numRP].flags = RP_FLAGS_STARTOFPIECE;
      m_RP[m_numRP].TRIPSrecordID = NO_RECORD;
      m_numRP++;
//
//  Generate the start trip
//
      TRIPSKey0.recordID = RUNS.start.TRIPSrecordID;
      btrieve(B_GETEQUAL, TMS_TRIPS, &TRIPS, &TRIPSKey0, 0);
      GenerateTrip(TRIPS.ROUTESrecordID, TRIPS.SERVICESrecordID,
            TRIPS.directionIndex, TRIPS.PATTERNNAMESrecordID,
            TRIPS.timeAtMLP, GENERATETRIP_FLAG_DISPLAYERRORS | GENERATETRIP_FLAG_GETLABELS, &GTResults);
//
//  Is the node a pullout?
//
      bFound = FALSE;
//      bStartOfPiece = TRUE;
      if(TRIPS.standard.POGNODESrecordID == RUNS.start.NODESrecordID)
      {
        GCTData.fromPATTERNNAMESrecordID = TRIPS.PATTERNNAMESrecordID;
        GCTData.toPATTERNNAMESrecordID = TRIPS.PATTERNNAMESrecordID;
        GCTData.fromROUTESrecordID = TRIPS.standard.RGRPROUTESrecordID;
        GCTData.fromSERVICESrecordID = TRIPS.standard.SGRPSERVICESrecordID;
        GCTData.toROUTESrecordID = TRIPS.standard.RGRPROUTESrecordID;
        GCTData.toSERVICESrecordID = TRIPS.standard.SGRPSERVICESrecordID;
        GCTData.fromNODESrecordID = TRIPS.standard.POGNODESrecordID;
        GCTData.toNODESrecordID = GTResults.firstNODESrecordID;
        GCTData.timeOfDay = GTResults.firstNodeTime;
        dhdTime = GetConnectionTime(GCT_FLAG_DEADHEADTIME, &GCTData, &distance);
        m_RP[m_numRP].NODESrecordID = RUNS.start.NODESrecordID;
        m_RP[m_numRP].time = GTResults.firstNodeTime - (dhdTime == NO_TIME ? 0 : dhdTime);
//        m_RP[m_numRP].flags = RP_FLAGS_STARTOFPIECE;
        m_RP[m_numRP].flags = 0;
        m_RP[m_numRP].TRIPSrecordID = TRIPS.recordID;
        m_numRP++;
        bFound = TRUE;
//        bStartOfPiece = FALSE;
      }
//
//  Find any relief points on the trip
//
      PATTERNSKey2.ROUTESrecordID = TRIPS.ROUTESrecordID;
      PATTERNSKey2.SERVICESrecordID = TRIPS.SERVICESrecordID;
      PATTERNSKey2.directionIndex = TRIPS.directionIndex;
      PATTERNSKey2.PATTERNNAMESrecordID = TRIPS.PATTERNNAMESrecordID;
      PATTERNSKey2.nodeSequence = NO_RECORD;
      rcode2 = btrieve(B_GETGREATEROREQUAL, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
      counter = 0;
//
//  Ensure that if there wasn't a P/O that we're starting at the right node
//
      while(rcode2 == 0 &&
            PATTERNS.ROUTESrecordID == TRIPS.ROUTESrecordID &&
            PATTERNS.SERVICESrecordID == TRIPS.SERVICESrecordID &&
            PATTERNS.directionIndex == TRIPS.directionIndex &&
            PATTERNS.PATTERNNAMESrecordID == TRIPS.PATTERNNAMESrecordID)
      {
        if(!(PATTERNS.flags & PATTERNS_FLAG_BUSSTOP))
        {
          if(bFound || PATTERNS.NODESrecordID == RUNS.start.NODESrecordID)
          {
            bFound = TRUE;  // In case we've hit PATTERNS.NODESrecordID == startNODESrecordID
            NODESKey0.recordID = PATTERNS.NODESrecordID;
            btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
            if(CheckNodeLabel(GTResults.labelledNODESrecordIDs[counter], &NODES, (int)serviceIndex))
            {
              m_RP[m_numRP].NODESrecordID = PATTERNS.NODESrecordID;
              m_RP[m_numRP].time = GTResults.tripTimes[counter];
//              if(bStartOfPiece)
//              {
//                m_RP[m_numRP].flags = RP_FLAGS_STARTOFPIECE;
//                bStartOfPiece = FALSE;
//              }
//              else
//              {
                m_RP[m_numRP].flags = 0;
//              }
              m_RP[m_numRP].TRIPSrecordID = TRIPS.recordID;
              m_numRP++;
            }
          }
          counter++;
        }
        rcode2 = btrieve(B_GETNEXT, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
      }
//
//  Does the first trip pull in?
//
      bFound = FALSE;
      if(TRIPS.standard.PIGNODESrecordID == RUNS.end.NODESrecordID)
      {
        GCTData.fromPATTERNNAMESrecordID = TRIPS.PATTERNNAMESrecordID;
        GCTData.toPATTERNNAMESrecordID = TRIPS.PATTERNNAMESrecordID;
        GCTData.fromROUTESrecordID = TRIPS.standard.RGRPROUTESrecordID;
        GCTData.fromSERVICESrecordID = TRIPS.standard.SGRPSERVICESrecordID;
        GCTData.toROUTESrecordID = TRIPS.standard.RGRPROUTESrecordID;
        GCTData.toSERVICESrecordID = TRIPS.standard.SGRPSERVICESrecordID;
        GCTData.fromNODESrecordID = GTResults.lastNODESrecordID;
        GCTData.toNODESrecordID = TRIPS.standard.PIGNODESrecordID;
        GCTData.timeOfDay = GTResults.lastNodeTime;
        dhdTime = GetConnectionTime(GCT_FLAG_DEADHEADTIME, &GCTData, &distance);
        m_RP[m_numRP].NODESrecordID = RUNS.end.NODESrecordID;
        m_RP[m_numRP].time = GTResults.lastNodeTime + (dhdTime == NO_TIME ? 0 : dhdTime);
//        m_RP[m_numRP].flags = RP_FLAGS_ENDOFPIECE;
        m_RP[m_numRP].flags = 0;
        m_RP[m_numRP].TRIPSrecordID = TRIPS.recordID;
        m_numRP++;
        bFound = TRUE;
      }
//
//  Continue through the rest of the block unless we've already pulled-in
//
      if(!bFound)
      {
        TRIPSKey0.recordID = RUNS.start.TRIPSrecordID;
        btrieve(B_GETEQUAL, TMS_TRIPS, &TRIPS, &TRIPSKey0, 0);
        btrieve(B_GETPOSITION, TMS_TRIPS, &TRIPS, &TRIPSKey0, 0);
        rcode2 = btrieve(B_GETDIRECT, TMS_TRIPS, &TRIPS, &TRIPSKey2, 2);
        assignedToNODESrecordID = TRIPS.standard.assignedToNODESrecordID;
        RGRPROUTESrecordID = TRIPS.standard.RGRPROUTESrecordID;
        SGRPSERVICESrecordID = TRIPS.standard.SGRPSERVICESrecordID;
        blockNumber = TRIPS.standard.blockNumber;
        rcode2 = btrieve(B_GETNEXT, TMS_TRIPS, &TRIPS, &TRIPSKey2, 2);
        bEndOfPiece = FALSE;
        while(rcode2 == 0 &&
              TRIPS.standard.assignedToNODESrecordID == assignedToNODESrecordID &&
              TRIPS.standard.RGRPROUTESrecordID == RGRPROUTESrecordID &&
              TRIPS.standard.SGRPSERVICESrecordID == SGRPSERVICESrecordID &&
              TRIPS.standard.blockNumber == blockNumber)
        {
//
//  Generate the trip
//
          GenerateTrip(TRIPS.ROUTESrecordID, TRIPS.SERVICESrecordID,
                TRIPS.directionIndex, TRIPS.PATTERNNAMESrecordID,
                TRIPS.timeAtMLP, GENERATETRIP_FLAG_DISPLAYERRORS | GENERATETRIP_FLAG_GETLABELS, &GTResults);
//
//  Find any relief points on the trip
//
          PATTERNSKey2.ROUTESrecordID = TRIPS.ROUTESrecordID;
          PATTERNSKey2.SERVICESrecordID = TRIPS.SERVICESrecordID;
          PATTERNSKey2.directionIndex = TRIPS.directionIndex;
          PATTERNSKey2.PATTERNNAMESrecordID = TRIPS.PATTERNNAMESrecordID;
          PATTERNSKey2.nodeSequence = NO_RECORD;
          rcode2 = btrieve(B_GETGREATEROREQUAL, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
          counter = 0;
          while(rcode2 == 0 &&
                PATTERNS.ROUTESrecordID == TRIPS.ROUTESrecordID &&
                PATTERNS.SERVICESrecordID == TRIPS.SERVICESrecordID &&
                PATTERNS.directionIndex == TRIPS.directionIndex &&
                PATTERNS.PATTERNNAMESrecordID == TRIPS.PATTERNNAMESrecordID)
          {
            if(!(PATTERNS.flags & PATTERNS_FLAG_BUSSTOP))
            {
              if(TRIPS.recordID == RUNS.end.TRIPSrecordID &&
                    PATTERNS.NODESrecordID == RUNS.end.NODESrecordID)
              {
                m_RP[m_numRP].NODESrecordID = PATTERNS.NODESrecordID;
                m_RP[m_numRP].time = GTResults.tripTimes[counter];
//                m_RP[m_numRP].flags = RP_FLAGS_ENDOFPIECE;
                m_RP[m_numRP].flags = 0;
                bEndOfPiece = TRUE;
                m_RP[m_numRP].TRIPSrecordID = TRIPS.recordID;
                m_numRP++;
                break;
              }
              else
              {
                NODESKey0.recordID = PATTERNS.NODESrecordID;
                btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
                if(CheckNodeLabel(GTResults.labelledNODESrecordIDs[counter], &NODES, (int)serviceIndex))
                {
                  m_RP[m_numRP].NODESrecordID = PATTERNS.NODESrecordID;
                  m_RP[m_numRP].time = GTResults.tripTimes[counter];
                  m_RP[m_numRP].flags = 0;
                  m_RP[m_numRP].TRIPSrecordID = TRIPS.recordID;
                  m_numRP++;
                }
              }
              counter++;
            }
            rcode2 = btrieve(B_GETNEXT, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
          }
//
//  Check for a pull-in
//
          if(TRIPS.standard.PIGNODESrecordID == RUNS.end.NODESrecordID)
          {
            GCTData.fromPATTERNNAMESrecordID = TRIPS.PATTERNNAMESrecordID;
            GCTData.toPATTERNNAMESrecordID = TRIPS.PATTERNNAMESrecordID;
            GCTData.fromROUTESrecordID = TRIPS.standard.RGRPROUTESrecordID;
            GCTData.fromSERVICESrecordID = TRIPS.standard.SGRPSERVICESrecordID;
            GCTData.toROUTESrecordID = TRIPS.standard.RGRPROUTESrecordID;
            GCTData.toSERVICESrecordID = TRIPS.standard.SGRPSERVICESrecordID;
            GCTData.fromNODESrecordID = GTResults.lastNODESrecordID;
            GCTData.toNODESrecordID = TRIPS.standard.PIGNODESrecordID;
            GCTData.timeOfDay = GTResults.lastNodeTime;
            dhdTime = GetConnectionTime(GCT_FLAG_DEADHEADTIME, &GCTData, &distance);
            m_RP[m_numRP].NODESrecordID = RUNS.end.NODESrecordID;
            m_RP[m_numRP].time = GTResults.lastNodeTime + (dhdTime == NO_TIME ? 0 : dhdTime);
//            m_RP[m_numRP].flags = RP_FLAGS_ENDOFPIECE;
            m_RP[m_numRP].flags = 0;
            bEndOfPiece = TRUE;
            m_RP[m_numRP].TRIPSrecordID = TRIPS.recordID;
            m_numRP++;
          }
//
//  If we've hit the end of the piece, we're done
//
          if(bEndOfPiece)
          {
//
//  Get the turnin location and time if they differ from where we are
//
            if(COST.TRAVEL[pieceNumber].endNODESrecordID == NO_RECORD)
            {
              m_RP[m_numRP - 1].flags = RP_FLAGS_ENDOFPIECE;
            }
            else
            {
              if(COST.TRAVEL[pieceNumber].endNODESrecordID == NO_RECORD)
              {
                m_RP[m_numRP].NODESrecordID = PROPOSEDRUN.piece[pieceNumber].toNODESrecordID;
              }
              else
              {
                m_RP[m_numRP].NODESrecordID = COST.TRAVEL[pieceNumber].endNODESrecordID;
              }
              m_RP[m_numRP].time = PROPOSEDRUN.piece[pieceNumber].toTime +
                    COST.PIECECOST[pieceNumber].turninTime + COST.TRAVEL[pieceNumber].endTravelTime;
              if(PROPOSEDRUN.piece[pieceNumber].after.endTime != NO_TIME)
              {
                m_RP[m_numRP].time += PROPOSEDRUN.piece[pieceNumber].after.endTime;
              }
              m_RP[m_numRP].flags = RP_FLAGS_ENDOFPIECE;
              m_RP[m_numRP].TRIPSrecordID = NO_RECORD;
              m_numRP++;
            }
            break;
          }
//
//  Get the next trip in the block
//
          rcode2 = btrieve(B_GETNEXT, TMS_TRIPS, &TRIPS, &TRIPSKey2, 2);
        }
      }
//
//  Get the next piece of the run
//
      rcode2 = btrieve(B_GETNEXT, TMS_RUNS, &RUNS, &RUNSKey1, 1);
    }
  }
//
//  Put any inseted points into the list
//
  int currentRP = m_numRP;

  DAILYOPSKey1.recordTypeFlag = DAILYOPS_FLAG_OPENWORK;
  DAILYOPSKey1.recordFlags = DAILYOPS_FLAG_RUNSPLITINSERT;
  DAILYOPSKey1.pertainsToDate = m_pPassedData->pertainsToDate;
  DAILYOPSKey1.pertainsToTime = NO_TIME;
  rcode2 = btrieve(B_GETGREATEROREQUAL, TMS_DAILYOPS, &DAILYOPS, &DAILYOPSKey1, 1);
  while(rcode2 == 0 &&
        DAILYOPS.recordTypeFlag == DAILYOPS_FLAG_OPENWORK &&
        DAILYOPS.pertainsToDate == m_pPassedData->pertainsToDate)
  {
    if(DAILYOPS.recordFlags & DAILYOPS_FLAG_RUNSPLITINSERT)
    {
      if(DAILYOPS.DAILYOPSrecordID == NO_RECORD)  // not deleted
      {
        bCrewOnly = DAILYOPS.DOPS.OpenWork.splitStartTRIPSrecordID;  // used on a run split insert
        if(m_bCrewOnly == bCrewOnly)
        {
          if(DAILYOPS.DOPS.OpenWork.RUNSrecordID == m_pPassedData->RUNSrecordID)
          {
            m_RP[m_numRP].NODESrecordID = DAILYOPS.DOPS.OpenWork.splitStartNODESrecordID;
            m_RP[m_numRP].time = DAILYOPS.DOPS.OpenWork.splitStartTime;
            m_RP[m_numRP].TRIPSrecordID = NO_RECORD;
            m_RP[m_numRP].DAILYOPSrecordID = DAILYOPS.recordID;
            m_RP[m_numRP].flags |= RP_FLAGS_INSERTED;
            m_numRP++;
          }
        }
      }
    }
    rcode2 = btrieve(B_GETNEXT, TMS_DAILYOPS, &DAILYOPS, &DAILYOPSKey1, 1);
  }
//
//  If there were any additions, sort the list
//
  if(m_numRP != currentRP)
  {
    qsort((void *)m_RP, m_numRP, sizeof(RPDef), sort_RP);
  }
//
//  Get the current splits
//
  LVITEM  LVI;
  BOOL    bRecordFlag = (m_bCrewOnly ? DAILYOPS_FLAG_RUNSPLITCREWONLY : DAILYOPS_FLAG_RUNSPLIT);
  int     numCurrent = 0;
  int     startIndex, endIndex;

  DAILYOPSKey1.recordTypeFlag = DAILYOPS_FLAG_OPENWORK;
  DAILYOPSKey1.recordFlags = 0; //bRecordFlag;
  DAILYOPSKey1.pertainsToDate = m_pPassedData->pertainsToDate;
  DAILYOPSKey1.pertainsToTime = NO_TIME;
  rcode2 = btrieve(B_GETGREATEROREQUAL, TMS_DAILYOPS, &DAILYOPS, &DAILYOPSKey1, 1);
  while(rcode2 == 0 &&
        DAILYOPS.recordTypeFlag == DAILYOPS_FLAG_OPENWORK &&
        DAILYOPS.pertainsToDate == m_pPassedData->pertainsToDate)
  {
    if(DAILYOPS.recordFlags & bRecordFlag)
    {
      if(!ANegatedRecord(DAILYOPS.recordID, 1))
//      if(DAILYOPS.DAILYOPSrecordID == NO_RECORD &&
//            !(DAILYOPS.recordFlags & DAILYOPS_FLAG_OPENWORKASSIGN))  // not unsplit prior to this
      {
        if(DAILYOPS.DOPS.OpenWork.RUNSrecordID == m_pPassedData->RUNSrecordID)
        {
//
//  Get the start of the split
//
          for(startIndex = NO_RECORD, nI = 0; nI < m_numRP; nI++)
          {
            if(DAILYOPS.DOPS.OpenWork.splitStartNODESrecordID == m_RP[nI].NODESrecordID &&
                  DAILYOPS.DOPS.OpenWork.splitStartTime == m_RP[nI].time)
            {
              startIndex = nI;
              break;
            }
          }
//
//  Get the end of the split
//
          if(startIndex != NO_RECORD)
          {
            for(endIndex = NO_RECORD, nI = startIndex + 1; nI < m_numRP; nI++)
            {
              if(DAILYOPS.DOPS.OpenWork.splitEndNODESrecordID == m_RP[nI].NODESrecordID &&
                    DAILYOPS.DOPS.OpenWork.splitEndTime == m_RP[nI].time)
              {
                endIndex = nI;
                break;
              }
            }
//
//  Got both start and end
//
            if(endIndex != NO_RECORD)
            {
//
//  Set the flags
//
              for(nI = startIndex; nI <= endIndex; nI++)
              {
                if(nI == startIndex)
                {
                  m_RP[nI].flags |= RP_FLAGS_SPLITATSTART;
                  m_RP[nI].startDRIVERSrecordID = DAILYOPS.DRIVERSrecordID;
                }
                else if(nI == endIndex)
                {
                  m_RP[nI].flags |= RP_FLAGS_SPLITATEND;
                  m_RP[nI].endDRIVERSrecordID = DAILYOPS.DRIVERSrecordID;
                }
                else
                {
                  m_RP[nI].flags |= RP_FLAGS_SPLITMIDDLE;
                  m_RP[nI].startDRIVERSrecordID = DAILYOPS.DRIVERSrecordID;
                  m_RP[nI].endDRIVERSrecordID = DAILYOPS.DRIVERSrecordID;
                }
              }
//
//  Display it in the current splits
//
              LVI.mask = LVIF_TEXT | LVIF_PARAM;
              LVI.iItem = numCurrent;
              LVI.iSubItem = 0;
              itoa(numCurrent + 1, tempString, 10);
              LVI.pszText = tempString;
              LVI.lParam = MAKELONG(startIndex, endIndex);
              LVI.iItem = pListCtrlCURRENTSPLITS->InsertItem(&LVI);
//
//  Node
//
              NODESKey0.recordID = DAILYOPS.DOPS.OpenWork.splitStartNODESrecordID;
              btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
              strncpy(szarString, NODES.abbrName, NODES_ABBRNAME_LENGTH);
              trim(szarString, NODES_ABBRNAME_LENGTH);
              LVI.mask = LVIF_TEXT;
              LVI.iItem = numCurrent;
              LVI.iSubItem = 1;
              LVI.pszText = szarString;
              LVI.iItem = pListCtrlCURRENTSPLITS->SetItem(&LVI);
//
//  At
//
              strcpy(tempString, Tchar(DAILYOPS.DOPS.OpenWork.splitStartTime));
              LVI.mask = LVIF_TEXT;
              LVI.iItem = numCurrent;
              LVI.iSubItem = 2;
              LVI.pszText = tempString;
              LVI.iItem = pListCtrlCURRENTSPLITS->SetItem(&LVI);
//
//  Node
//
              NODESKey0.recordID = DAILYOPS.DOPS.OpenWork.splitEndNODESrecordID;
              btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
              strncpy(szarString, NODES.abbrName, NODES_ABBRNAME_LENGTH);
              trim(szarString, NODES_ABBRNAME_LENGTH);
              LVI.mask = LVIF_TEXT;
              LVI.iItem = numCurrent;
              LVI.iSubItem = 3;
              LVI.pszText = szarString;
              LVI.iItem = pListCtrlCURRENTSPLITS->SetItem(&LVI);
//
//  At
//
              strcpy(tempString, Tchar(DAILYOPS.DOPS.OpenWork.splitEndTime));
              LVI.mask = LVIF_TEXT;
              LVI.iItem = numCurrent;
              LVI.iSubItem = 4;
              LVI.pszText = tempString;
              LVI.iItem = pListCtrlCURRENTSPLITS->SetItem(&LVI);
              numCurrent++;
            }
          }
        }
      }
    }
    rcode2 = btrieve(B_GETNEXT, TMS_DAILYOPS, &DAILYOPS, &DAILYOPSKey1, 1);
  }
//
//  List all the potential split points and times
//
  DisplayPotentials();
//
//  Dropdowns
//
  DisplayDropdowns();
//
//  When we come in with the AUTOMATIC flag, it means that an
//  absence has been registered and we're generating this run
//  split, well, automatically,  Position the selections to be
//  as close to the start and end of the incoming times as 
//  possible, and fire up an OnPreview() to see where it sits
//
  if(m_pPassedData->flags & RUNSPLIT_FLAGS_AUTOMATIC)
  {
/*
    startIndex = NO_RECORD;
    endIndex = NO_RECORD;
    for(nI = 0; nI < m_numRP; nI++)
    {  
      if(m_pPassedData->fromTime <= m_RP[nI].time && startIndex == NO_RECORD)
      {
        startIndex = nI;
      }
      endIndex = nI;
      if(m_pPassedData->toTime <= m_RP[nI].time)
      {
        break;
      }
    }
    for(nI = startIndex; nI <= endIndex; nI++)
    {
      if(nI == startIndex)
      {
        m_RP[nI].flags |= RP_FLAGS_SPLITATSTART;
      }
      else if(nI == endIndex)
      {
        m_RP[nI].flags |= RP_FLAGS_SPLITATEND;
      }
      else
      {
        m_RP[nI].flags |= RP_FLAGS_SPLITMIDDLE;
      }
    }
*/   
    OnPreview();
  }

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CDailyOpsRS::OnClose() 
{
  if(m_pPassedData->flags & RUNSPLIT_FLAGS_AUTOMATIC)
  {
    MessageBeep(MB_ICONINFORMATION);
    MessageBox("Cancel is not permitted on an automatic run split", TMS, MB_OK | MB_ICONINFORMATION);
  }
  else
  {
    OnCancel();
  }
}


void CDailyOpsRS::OnHelp() 
{
}

void CDailyOpsRS::OnCancel() 
{
	CDialog::OnCancel();
}

void CDailyOpsRS::DisplayPotentials()
{
  LVITEM LVI;
  int    nI;

  pListCtrlPOTENTIALSPLITS->DeleteAllItems();

  for(nI = 0; nI < m_numRP; nI++)
  {  
//
//  Flag
//
    if(m_RP[nI].flags & RP_FLAGS_STARTOFPIECE)
    {
      strcpy(tempString, "Start");
    }
    else if(m_RP[nI].flags & RP_FLAGS_ENDOFPIECE)
    {
      strcpy(tempString, "End");
    }
    else if((m_RP[nI].flags & RP_FLAGS_SPLITATEND) && (m_RP[nI].flags & RP_FLAGS_SPLITATSTART))
    {
      strcpy(tempString, "X/X");
    }
    else if(m_RP[nI].flags & RP_FLAGS_SPLITMIDDLE)
    {
      strcpy(tempString, "X");
    }
    else
    {
      if(m_RP[nI].flags & RP_FLAGS_SPLITATEND)
      {
        strcpy(tempString, "X/-");
      }
      else if(m_RP[nI].flags & RP_FLAGS_SPLITATSTART)
      {
        strcpy(tempString, "-/X");
      }
      else
      {
        strcpy(tempString, "");
      }
    }
    LVI.mask = LVIF_TEXT | LVIF_PARAM;
    LVI.iItem = nI;
    LVI.iSubItem = 0;
    LVI.pszText = tempString;
    LVI.lParam = nI;
    LVI.iItem = pListCtrlPOTENTIALSPLITS->InsertItem(&LVI);
//
//  Node
//
    NODESKey0.recordID = m_RP[nI].NODESrecordID;
    btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
    strncpy(szarString, NODES.abbrName, NODES_ABBRNAME_LENGTH);
    trim(szarString, NODES_ABBRNAME_LENGTH);
    LVI.mask = LVIF_TEXT;
    LVI.iItem = nI;
    LVI.iSubItem = 1;
    LVI.pszText = szarString;
    LVI.iItem = pListCtrlPOTENTIALSPLITS->SetItem(&LVI);
//
//  At
//
    strcpy(tempString, Tchar(m_RP[nI].time));
    LVI.mask = LVIF_TEXT;
    LVI.iItem = nI;
    LVI.iSubItem = 2;
    LVI.pszText = tempString;
    LVI.iItem = pListCtrlPOTENTIALSPLITS->SetItem(&LVI);
  }
}

void CDailyOpsRS::DisplayCurrents(int startIndex, int endIndex)
{
  LVITEM LVI;
  int    numCurrent;

  numCurrent = pListCtrlCURRENTSPLITS->GetItemCount();

  LVI.mask = LVIF_TEXT | LVIF_PARAM;
  LVI.iItem = numCurrent;
  LVI.iSubItem = 0;
  itoa(numCurrent + 1, tempString, 10);
  LVI.pszText = tempString;
  LVI.lParam = MAKELONG(startIndex, endIndex);
  LVI.iItem = pListCtrlCURRENTSPLITS->InsertItem(&LVI);
//
//  Node
//
  NODESKey0.recordID = m_RP[startIndex].NODESrecordID;
  btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
  strncpy(szarString, NODES.abbrName, NODES_ABBRNAME_LENGTH);
  trim(szarString, NODES_ABBRNAME_LENGTH);
  LVI.mask = LVIF_TEXT;
  LVI.iItem = numCurrent;
  LVI.iSubItem = 1;
  LVI.pszText = szarString;
  LVI.iItem = pListCtrlCURRENTSPLITS->SetItem(&LVI);
//
//  At
//
  strcpy(tempString, Tchar(m_RP[startIndex].time));
  LVI.mask = LVIF_TEXT;
  LVI.iItem = numCurrent;
  LVI.iSubItem = 2;
  LVI.pszText = tempString;
  LVI.iItem = pListCtrlCURRENTSPLITS->SetItem(&LVI);
//
//  Node
//
  NODESKey0.recordID = m_RP[endIndex].NODESrecordID;
  btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
  strncpy(szarString, NODES.abbrName, NODES_ABBRNAME_LENGTH);
  trim(szarString, NODES_ABBRNAME_LENGTH);
  LVI.mask = LVIF_TEXT;
  LVI.iItem = numCurrent;
  LVI.iSubItem = 3;
  LVI.pszText = szarString;
  LVI.iItem = pListCtrlCURRENTSPLITS->SetItem(&LVI);
//
//  At
//
  strcpy(tempString, Tchar(m_RP[endIndex].time));
  LVI.mask = LVIF_TEXT;
  LVI.iItem = numCurrent;
  LVI.iSubItem = 4;
  LVI.pszText = tempString;
  LVI.iItem = pListCtrlCURRENTSPLITS->SetItem(&LVI);
}

void CDailyOpsRS::DisplayDropdowns()
{
  int nI, nJ;

  pComboBoxSTARTSPLIT->ResetContent();
  pComboBoxENDSPLIT->ResetContent();

  for(nI = 0; nI < m_numRP; nI++)
  {  
    NODESKey0.recordID = m_RP[nI].NODESrecordID;
    btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
    strncpy(szarString, NODES.abbrName, NODES_ABBRNAME_LENGTH);
    trim(szarString, NODES_ABBRNAME_LENGTH);
    strcpy(tempString, szarString);
    strcat(tempString, " at ");
    strcat(tempString, Tchar(m_RP[nI].time));
//
//  Start point dropdown
//
    nJ = pComboBoxSTARTSPLIT->AddString(tempString);
    pComboBoxSTARTSPLIT->SetItemData(nJ, nI);
//
//  End point dropdown
//
    nJ = pComboBoxENDSPLIT->AddString(tempString);
    pComboBoxENDSPLIT->SetItemData(nJ, nI);
  }
}

void CDailyOpsRS::OnAdd() 
{
  int startSplit, endSplit;
  int startIndex, endIndex;

  startSplit = pComboBoxSTARTSPLIT->GetCurSel();
  endSplit = pComboBoxENDSPLIT->GetCurSel();
  if(startSplit == CB_ERR || endSplit == CB_ERR)
  {
    return;
  }

  startIndex = pComboBoxSTARTSPLIT->GetItemData(startSplit);
  endIndex = pComboBoxENDSPLIT->GetItemData(endSplit);
  
  DisplayCurrents(startIndex, endIndex);
//
//  Adjust the POTENTIALSPLITS listing
//
  int nI;

  for(nI = startIndex; nI <= endIndex; nI++)
  {
    if(nI == startIndex)
    {
      m_RP[nI].flags |= RP_FLAGS_SPLITATSTART;
    }
    else if(nI == endIndex)
    {
      m_RP[nI].flags |= RP_FLAGS_SPLITATEND;
    }
    else
    {
      m_RP[nI].flags |= RP_FLAGS_SPLITMIDDLE;
    }
  }
  DisplayPotentials();
//
//  Reset the ComboBox selections
//
  pComboBoxSTARTSPLIT->SetCurSel(-1);
  pComboBoxENDSPLIT->SetCurSel(-1);
  m_firstIndex = NO_RECORD;
  m_lastIndex = NO_RECORD;
}

void CDailyOpsRS::OnRemove() 
{
	int nI = pListCtrlCURRENTSPLITS->GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);

  if(nI >= 0)
  {
    long tempLong;
    int  startIndex, endIndex;
    int  nJ;

    tempLong = pListCtrlCURRENTSPLITS->GetItemData(nI);
    startIndex = LOWORD(tempLong);
    endIndex = HIWORD(tempLong);
    for(nJ = startIndex; nJ <= endIndex; nJ++)
    {
      if(nJ == startIndex)
      {
        m_RP[nJ].flags -= RP_FLAGS_SPLITATSTART;
      }
      else if(nJ == endIndex)
      {
        m_RP[nJ].flags -= RP_FLAGS_SPLITATEND;
      }
      else
      {
        m_RP[nJ].flags -= RP_FLAGS_SPLITMIDDLE;
      }
    }
    pListCtrlCURRENTSPLITS->DeleteItem(nI);
    DisplayPotentials();
  }
}

//
//  OnPreview() - Show him what'll happen if he presses OK now
//
void CDailyOpsRS::OnPreview() 
{
  BOOL bEndPieceFlag;
  int  startIndex, endIndex;
  int  nI, nJ;
//
//  Cycle through the list.  Anything that's a start/end of piece
//  will automatically be set out as a split.
//
  pListCtrlCURRENTSPLITS->DeleteAllItems();
  startIndex = NO_RECORD;
  endIndex = NO_RECORD;
  for(nI = 0; nI < m_numRP; nI++)
  {
    bEndPieceFlag = FALSE;
    if(startIndex == nI)
    {
      continue;
    }
//
//  The first one
//
    if(nI == 0)
    {
      startIndex = 0;
    }
//
//  The last one
//
    else if(nI == m_numRP - 1)
    {
      endIndex = m_numRP - 1;
    }
//
//  The rest
//
    else
    {
//
//  A split start or a split end
//
      if((m_RP[nI].flags & RP_FLAGS_SPLITATSTART) || (m_RP[nI].flags & RP_FLAGS_SPLITATEND))
      {
        endIndex = nI;
      }
//
//  The end of a piece that isn't in the middle of a current split
//
      if((m_RP[nI].flags & RP_FLAGS_ENDOFPIECE) && !(m_RP[nI].flags & RP_FLAGS_SPLITMIDDLE))
      {
        endIndex = nI;
        bEndPieceFlag = TRUE;
      }
    }
//
//  If we've got a start and end index, the set the flags
//
    if(startIndex != NO_RECORD && endIndex != NO_RECORD)
    {
      for(nJ = startIndex; nJ <= endIndex; nJ++)
      {
        if(nJ == startIndex)
        {
          m_RP[nJ].flags |= RP_FLAGS_SPLITATSTART;
        }
        else if(nJ == endIndex)
        {
          m_RP[nJ].flags |= RP_FLAGS_SPLITATEND;
        }
        else
        {
          m_RP[nJ].flags |= RP_FLAGS_SPLITMIDDLE;
        }
      }
      DisplayCurrents(startIndex, endIndex);
      startIndex = (bEndPieceFlag ? endIndex + 1 : endIndex);
      endIndex = NO_RECORD;
    }
  }
  DisplayPotentials();
}

void CDailyOpsRS::OnClickPotentialsplits(NMHDR* pNMHDR, LRESULT* pResult) 
{
	int nI = pListCtrlPOTENTIALSPLITS->GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);
  int nJ;

  if(nI >= 0)
  {
    nJ = pComboBoxSTARTSPLIT->GetItemData(nI);
    if(m_lastIndex != NO_RECORD && nJ >= m_lastIndex)
    {
      MessageBeep(MB_ICONSTOP);
      pComboBoxSTARTSPLIT->SetCurSel(m_PreviousStartPosition);
    }
    else
    {
      m_firstIndex = nJ;
      m_PreviousStartPosition = nI;
      pComboBoxSTARTSPLIT->SetCurSel(nI);
    }
  }
	
	*pResult = 0;
}

void CDailyOpsRS::OnRclickPotentialsplits(NMHDR* pNMHDR, LRESULT* pResult) 
{
	int nI = pListCtrlPOTENTIALSPLITS->GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);
  int nJ;

  if(nI >= 0)
  {
    nJ = pComboBoxENDSPLIT->GetItemData(nI);
    if(nJ <= m_firstIndex)
    {
      MessageBeep(MB_ICONSTOP);
      pComboBoxENDSPLIT->SetCurSel(m_PreviousEndPosition);
    }
    else
    {
      m_lastIndex = nJ;
      m_PreviousEndPosition = nI;
      pComboBoxENDSPLIT->SetCurSel(nI);
    }
  }
	
	*pResult = 0;
}

void CDailyOpsRS::OnSelchangeStartsplit() 
{
  int nI = pComboBoxSTARTSPLIT->GetCurSel();
  int nJ;

  if(nI == CB_ERR)
  {
    return;
  }

  nJ = pComboBoxSTARTSPLIT->GetItemData(nI);
  if(nJ >= m_lastIndex)
  {
    MessageBeep(MB_ICONSTOP);
    pComboBoxSTARTSPLIT->SetCurSel(m_PreviousStartPosition);
  }
  else
  {
    m_firstIndex = nJ;
    m_PreviousStartPosition = nI;
  }
}

void CDailyOpsRS::OnSelchangeEndsplit() 
{
  int nI = pComboBoxENDSPLIT->GetCurSel();
  int nJ;

  if(nI == CB_ERR)
  {
    return;
  }

  nJ = pComboBoxENDSPLIT->GetItemData(nI);
  if(nJ <= m_firstIndex)
  {
    MessageBeep(MB_ICONSTOP);
    pComboBoxENDSPLIT->SetCurSel(m_PreviousEndPosition);
  }
  else
  {
    m_lastIndex = nJ;
    m_PreviousEndPosition = nI;
  }
}

void CDailyOpsRS::OnClickCurrentsplits(NMHDR* pNMHDR, LRESULT* pResult) 
{
	*pResult = 0;
}


void CDailyOpsRS::OnDelete() 
{
	int nI = pListCtrlPOTENTIALSPLITS->GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);
  int nJ;

  if(nI >= 0)
  {
//
//  It has to have been previously inserted to be deleted
//
    if(!(m_RP[nI].flags & RP_FLAGS_INSERTED))
    {
      MessageBeep(MB_ICONSTOP);
      MessageBox("You can't delete a non-inserted split point", TMS, MB_ICONSTOP | MB_OK);
      return;
    }
//
//  Can't delete it when it's marked as the start of a split
//
    if(m_RP[nI].flags & RP_FLAGS_SPLITATSTART)
    {
      MessageBeep(MB_ICONSTOP);
      MessageBox("You can't delete a split point that's marked as the start of a split", TMS, MB_ICONSTOP | MB_OK);
      return;
    }
//
//  Can't delete it when it's marked as the end of a split
//
    if(m_RP[nI].flags & RP_FLAGS_SPLITATEND)
    {
      MessageBeep(MB_ICONSTOP);
      MessageBox("You can't delete a split point that's marked as the end of a split", TMS, MB_ICONSTOP | MB_OK);
      return;
    }
//
//  Negate the record
//
    CTime SystemTime = CTime::GetCurrentTime();;
    long  recordID;
    long  UNRUNSPLITINSERTrecordID;
    int   rcode2;
    rcode2 = btrieve(B_GETLAST, TMS_DAILYOPS, &DAILYOPS, &DAILYOPSKey0, 0);
    recordID = AssignRecID(rcode2, DAILYOPS.recordID);
    memset(&DAILYOPS, 0x00, sizeof(DAILYOPSDef));
    DAILYOPS.recordID = recordID;
    DAILYOPS.entryDateAndTime = SystemTime.GetTime();
    DAILYOPS.pertainsToDate = m_pPassedData->pertainsToDate;
    DAILYOPS.pertainsToTime = NO_TIME;
    DAILYOPS.userID = m_UserID;
    DAILYOPS.recordTypeFlag = DAILYOPS_FLAG_OPENWORK;
    DAILYOPS.recordFlags = DAILYOPS_FLAG_UNRUNSPLITINSERT;
    DAILYOPS.DAILYOPSrecordID = NO_RECORD;
    DAILYOPS.DRIVERSrecordID = NO_RECORD;
    rcode2 = btrieve(B_INSERT, TMS_DAILYOPS, &DAILYOPS, &DAILYOPSKey0, 0);
    if(rcode2 == 0)
    {
      m_LastDAILYOPSRecordID = DAILYOPS.recordID;
    }
    UNRUNSPLITINSERTrecordID = DAILYOPS.recordID;
    DAILYOPSKey0.recordID = m_RP[nI].DAILYOPSrecordID;
    rcode2 = btrieve(B_GETEQUAL, TMS_DAILYOPS, &DAILYOPS, &DAILYOPSKey0, 0);
    rcode2 = btrieve(B_GETPOSITION, TMS_DAILYOPS, &DAILYOPS, &DAILYOPSKey0, 0);
    rcode2 = btrieve(B_GETDIRECT, TMS_DAILYOPS, &DAILYOPS, &DAILYOPSKey1, 1);
    DAILYOPS.DAILYOPSrecordID = UNRUNSPLITINSERTrecordID;
    rcode2 = btrieve(B_UPDATE, TMS_DAILYOPS, &DAILYOPS, &DAILYOPSKey1, 1);
//
//  Fix the incore array
//
    for(nJ = nI; nJ < m_numRP - 1; nJ++)
    {
      memcpy(&m_RP[nJ], &m_RP[nJ + 1], sizeof(RPDef));
    }
    m_numRP--;
//
//  Fix the display
//
    DisplayPotentials();
    DisplayDropdowns();
  }
}

void CDailyOpsRS::OnInsert() 
{
  CTime SystemTime = CTime::GetCurrentTime();;
  long NODESrecordID;
  long time;
  long recordID;
	int  nI;
  int  rcode2;

  CDailyOpsRSO dlg(this, &NODESrecordID, &time);
  
  if(dlg.DoModal() == IDOK)
  {
//
//  Can't go before the first one
//
    if(time <= m_RP[0].time)
    {
      MessageBeep(MB_ICONSTOP);
      MessageBox("The time can't be earlier than the start time", TMS, MB_ICONSTOP | MB_OK);
      return;
    }
//
//  Can't go after the last one
//
    if(time >= m_RP[m_numRP - 1].time)
    {
      MessageBeep(MB_ICONSTOP);
      MessageBox("Time time can't be later than the end time", TMS, MB_ICONSTOP | MB_OK);
      return;
    }
//
//  Build a new record and insert it
//
    rcode2 = btrieve(B_GETLAST, TMS_DAILYOPS, &DAILYOPS, &DAILYOPSKey0, 0);
    recordID = AssignRecID(rcode2, DAILYOPS.recordID);
    memset(&DAILYOPS, 0x00, sizeof(DAILYOPSDef));
    DAILYOPS.recordID = recordID;
    DAILYOPS.entryDateAndTime = SystemTime.GetTime();
    DAILYOPS.pertainsToDate = m_pPassedData->pertainsToDate;
    DAILYOPS.pertainsToTime = NO_TIME;
    DAILYOPS.userID = m_UserID;
    DAILYOPS.recordTypeFlag = DAILYOPS_FLAG_OPENWORK;
    DAILYOPS.recordFlags = DAILYOPS_FLAG_RUNSPLITINSERT;
    DAILYOPS.DAILYOPSrecordID = NO_RECORD;
    DAILYOPS.DRIVERSrecordID = NO_RECORD;
    DAILYOPS.DOPS.OpenWork.RUNSrecordID = m_pPassedData->RUNSrecordID;
    DAILYOPS.DOPS.OpenWork.splitStartTRIPSrecordID = (long)m_bCrewOnly;
    DAILYOPS.DOPS.OpenWork.splitStartNODESrecordID = NODESrecordID;
    DAILYOPS.DOPS.OpenWork.splitStartTime = time;
    DAILYOPS.DOPS.OpenWork.splitEndNODESrecordID = NO_RECORD;
    DAILYOPS.DOPS.OpenWork.splitEndTime = NO_RECORD;
    rcode2 = btrieve(B_INSERT, TMS_DAILYOPS, &DAILYOPS, &DAILYOPSKey0, 0);
    if(rcode2 == 0)
    {
      m_LastDAILYOPSRecordID = DAILYOPS.recordID;
    }
//
//  Find where it should go
//
    int pos = m_numRP;;

    for(nI = 0; nI < m_numRP; nI++)
    {
      if(time < m_RP[nI].time)
      {
        pos = nI;
        break;
      }
    }
    for(nI = m_numRP; nI > pos; nI--)
    {
      m_RP[nI].NODESrecordID = m_RP[nI - 1].NODESrecordID;
      m_RP[nI].time = m_RP[nI - 1].time;
      m_RP[nI].flags = m_RP[nI - 1].flags;
    }
    m_RP[pos].NODESrecordID = NODESrecordID;
    m_RP[pos].time = time;
    m_RP[pos].flags = RP_FLAGS_INSERTED;
    m_numRP++;
    DisplayPotentials();
    DisplayDropdowns();
  }
}

//
//  OnOK() - Delete existing splits for this run and add in the new ones
//
void CDailyOpsRS::OnOK() 
{
  CTime SystemTime = CTime::GetCurrentTime();;
  long  recordID, DAILYOPSrecordID;
  long  tempLong, startIndex, endIndex;
  long  recordFlags;
  BOOL  bRecordFlag = (m_bCrewOnly ? DAILYOPS_FLAG_RUNSPLITCREWONLY : DAILYOPS_FLAG_RUNSPLIT);
  int   rcode2;
  int   nI;
//
//  "UnRunSplit" (delete) existing splits
//
  DAILYOPSKey1.recordTypeFlag = DAILYOPS_FLAG_OPENWORK;
  DAILYOPSKey1.recordFlags = bRecordFlag;
  DAILYOPSKey1.pertainsToDate = m_pPassedData->pertainsToDate;
  DAILYOPSKey1.pertainsToTime = NO_TIME;
  rcode2 = btrieve(B_GETGREATEROREQUAL, TMS_DAILYOPS, &DAILYOPS, &DAILYOPSKey1, 1);
  while(rcode2 == 0 &&
        DAILYOPS.recordTypeFlag == DAILYOPS_FLAG_OPENWORK &&
        DAILYOPS.pertainsToDate == m_pPassedData->pertainsToDate)
  {
    if(DAILYOPS.recordFlags & DAILYOPS_FLAG_OPENWORKASSIGN ||
          ((DAILYOPS.recordFlags & DAILYOPS_FLAG_RUNSPLIT) && !m_bCrewOnly) ||
          ((DAILYOPS.recordFlags & DAILYOPS_FLAG_RUNSPLITCREWONLY) && m_bCrewOnly))
    {
      recordFlags = DAILYOPS.recordFlags;
      if(DAILYOPS.DOPS.OpenWork.RUNSrecordID == m_pPassedData->RUNSrecordID)
      {
        if(!ANegatedRecord(DAILYOPS.recordID, 1))  // not unsplit prior to this
        {
          DAILYOPSrecordID = DAILYOPS.recordID;
          rcode2 = btrieve(B_GETLAST, TMS_DAILYOPS, &DAILYOPS, &DAILYOPSKey0, 0);
          recordID = AssignRecID(rcode2, DAILYOPS.recordID);
          memset(&DAILYOPS, 0x00, sizeof(DAILYOPSDef));
          DAILYOPS.recordID = recordID;
          DAILYOPS.entryDateAndTime = SystemTime.GetTime();
          DAILYOPS.pertainsToDate = m_pPassedData->pertainsToDate;
          DAILYOPS.pertainsToTime = DAILYOPS.recordID;
          DAILYOPS.userID = m_UserID;
          DAILYOPS.recordTypeFlag = DAILYOPS_FLAG_OPENWORK;
          DAILYOPS.recordFlags =
                (recordFlags & DAILYOPS_FLAG_OPENWORKASSIGN ? DAILYOPS_FLAG_OPENWORKCLEAR : DAILYOPS_FLAG_UNRUNSPLIT);
          DAILYOPS.DAILYOPSrecordID = DAILYOPSrecordID;
          DAILYOPS.DRIVERSrecordID = NO_RECORD;
          rcode2 = btrieve(B_INSERT, TMS_DAILYOPS, &DAILYOPS, &DAILYOPSKey0, 0);
          if(rcode2 == 0)
          {
            m_LastDAILYOPSRecordID = DAILYOPS.recordID;
          }
          DAILYOPSKey0.recordID = DAILYOPSrecordID;
          rcode2 = btrieve(B_GETEQUAL, TMS_DAILYOPS, &DAILYOPS, &DAILYOPSKey0, 0);
          DAILYOPS.DAILYOPSrecordID = recordID;
          rcode2 = btrieve(B_UPDATE, TMS_DAILYOPS, &DAILYOPS, &DAILYOPSKey0, 0);
          rcode2 = btrieve(B_GETPOSITION, TMS_DAILYOPS, &DAILYOPS, &DAILYOPSKey0, 0);
          rcode2 = btrieve(B_GETDIRECT, TMS_DAILYOPS, &DAILYOPS, &DAILYOPSKey1, 1);
        }
      }
    }
    rcode2 = btrieve(B_GETNEXT, TMS_DAILYOPS, &DAILYOPS, &DAILYOPSKey1, 1);
  }
//
//  Build the run split records.  OnPreview() will set all the flags that are missing.
//
  m_pPassedData->numAdded = 0;
  if(pListCtrlCURRENTSPLITS->GetItemCount() > 0)
  {
    OnPreview();
    m_pPassedData->numAdded = pListCtrlCURRENTSPLITS->GetItemCount();
    for(nI = 0; nI < m_pPassedData->numAdded; nI++)
    {
      tempLong = pListCtrlCURRENTSPLITS->GetItemData(nI);
      startIndex = LOWORD(tempLong);
      endIndex = HIWORD(tempLong);
//
//  Clear the record
//
      rcode2 = btrieve(B_GETLAST, TMS_DAILYOPS, &DAILYOPS, &DAILYOPSKey0, 0);
      recordID = AssignRecID(rcode2, DAILYOPS.recordID);
      memset(&DAILYOPS, 0x00, sizeof(DAILYOPSDef));
//
//  Build a new record
//
      DAILYOPS.recordID = recordID;
      DAILYOPS.entryDateAndTime = SystemTime.GetTime();
      DAILYOPS.pertainsToDate = m_pPassedData->pertainsToDate;
      DAILYOPS.pertainsToTime = DAILYOPS.recordID;
      DAILYOPS.userID = m_UserID;
      DAILYOPS.recordTypeFlag = DAILYOPS_FLAG_OPENWORK;
      DAILYOPS.recordFlags = (m_bCrewOnly ? DAILYOPS_FLAG_RUNSPLITCREWONLY : DAILYOPS_FLAG_RUNSPLIT);
      DAILYOPS.DAILYOPSrecordID = NO_RECORD;
      DAILYOPS.DRIVERSrecordID = NO_RECORD;
      DAILYOPS.DOPS.OpenWork.RUNSrecordID = m_pPassedData->RUNSrecordID;
      DAILYOPS.DOPS.OpenWork.splitStartTRIPSrecordID = m_RP[startIndex].TRIPSrecordID;
      DAILYOPS.DOPS.OpenWork.splitStartNODESrecordID = m_RP[startIndex].NODESrecordID;
      DAILYOPS.DOPS.OpenWork.splitStartTime = m_RP[startIndex].time;
      DAILYOPS.DOPS.OpenWork.splitEndNODESrecordID = m_RP[endIndex].NODESrecordID;
      DAILYOPS.DOPS.OpenWork.splitEndTime = m_RP[endIndex].time;
      rcode2 = btrieve(B_INSERT, TMS_DAILYOPS, &DAILYOPS, &DAILYOPSKey0, 0);
      if(rcode2 == 0)
      {
        m_LastDAILYOPSRecordID = DAILYOPS.recordID;
      }
      m_pPassedData->DAILYOPSrecordIDs[nI] = DAILYOPS.recordID;
    }
//
//  If we can, go through an assign the driver back to whatever's left of his pieces
//
    if(m_pPassedData->DRIVERSrecordID > 0)
    {
      for(nI = 0; nI < m_pPassedData->numAdded; nI++)
      {
        tempLong = pListCtrlCURRENTSPLITS->GetItemData(nI);
        startIndex = LOWORD(tempLong);
        endIndex = HIWORD(tempLong);
        if(m_pPassedData->toTime < m_RP[startIndex].time || m_pPassedData->fromTime > m_RP[endIndex].time)
        {
//
//  Clear the record
//
          rcode2 = btrieve(B_GETLAST, TMS_DAILYOPS, &DAILYOPS, &DAILYOPSKey0, 0);
          recordID = AssignRecID(rcode2, DAILYOPS.recordID);
          memset(&DAILYOPS, 0x00, sizeof(DAILYOPSDef));
//
//  Build a new record
//
          DAILYOPS.recordID = recordID;
          DAILYOPS.entryDateAndTime = SystemTime.GetTime();
          DAILYOPS.pertainsToDate = m_pPassedData->pertainsToDate;
          DAILYOPS.pertainsToTime = DAILYOPS.recordID;
          DAILYOPS.userID = m_UserID;
          DAILYOPS.recordTypeFlag = DAILYOPS_FLAG_OPENWORK;
          DAILYOPS.recordFlags = DAILYOPS_FLAG_OPENWORKASSIGN;
          if(m_bCrewOnly)
          {
            DAILYOPS.recordFlags |= DAILYOPS_FLAG_CREWONLY;
          }
          DAILYOPS.DRIVERSrecordID = m_pPassedData->DRIVERSrecordID;
          DAILYOPS.DOPS.OpenWork.RUNSrecordID = m_pPassedData->RUNSrecordID;
          DAILYOPS.DOPS.OpenWork.splitStartTRIPSrecordID = m_RP[startIndex].TRIPSrecordID;
          DAILYOPS.DOPS.OpenWork.splitStartNODESrecordID = m_RP[startIndex].NODESrecordID;
          DAILYOPS.DOPS.OpenWork.splitStartTime = m_RP[startIndex].time;
          DAILYOPS.DOPS.OpenWork.splitEndNODESrecordID = m_RP[endIndex].NODESrecordID;
          DAILYOPS.DOPS.OpenWork.splitEndTime = m_RP[endIndex].time;
          DAILYOPS.recordFlags |= (m_bCrewOnly ? DAILYOPS_FLAG_RUNSPLITCREWONLY : DAILYOPS_FLAG_RUNSPLIT);
          rcode2 = btrieve(B_INSERT, TMS_DAILYOPS, &DAILYOPS, &DAILYOPSKey0, 0);
          if(rcode2 == 0)
          {
            m_LastDAILYOPSRecordID = DAILYOPS.recordID;
          }
          m_pPassedData->DAILYOPSrecordIDs[nI] = DAILYOPS.recordID;
        }
      }
    }
  }

	
	CDialog::OnOK();
}

void CDailyOpsRS::OnRemoveall() 
{
  int nI;

  for(nI = 0; nI <= m_numRP; nI++)
  {
    if(m_RP[nI].flags & RP_FLAGS_SPLITATSTART)
    {
      m_RP[nI].flags -= RP_FLAGS_SPLITATSTART;
    }
    if(m_RP[nI].flags & RP_FLAGS_SPLITATEND)
    {
      m_RP[nI].flags -= RP_FLAGS_SPLITATEND;
    }
    if(m_RP[nI].flags & RP_FLAGS_SPLITMIDDLE)
    {
      m_RP[nI].flags -= RP_FLAGS_SPLITMIDDLE;
    }
  }
  pListCtrlCURRENTSPLITS->DeleteAllItems();
  DisplayPotentials();
}

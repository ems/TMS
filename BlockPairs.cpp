// BlockPairs.cpp : implementation file
//

#include "stdafx.h"
extern "C" {
#include "TMSHeader.h"
}

#include "TMS.h"
#include <math.h>
#include "BlockPairs.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CBlockPairs dialog


CBlockPairs::CBlockPairs(CWnd* pParent /*=NULL*/, DISPLAYINFO* pDI)
	: CDialog(CBlockPairs::IDD, pParent)
{
  m_pDI = pDI;

	//{{AFX_DATA_INIT(CBlockPairs)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CBlockPairs::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CBlockPairs)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CBlockPairs, CDialog)
	//{{AFX_MSG_MAP(CBlockPairs)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDHELP, OnHelp)
	ON_BN_CLICKED(BLOCKPAIRS_MIN, OnMin)
	ON_BN_CLICKED(BLOCKPAIRS_MAX, OnMax)
	ON_BN_CLICKED(BLOCKPAIRS_DHNO, OnDhno)
	ON_BN_CLICKED(BLOCKPAIRS_DHYES, OnDhyes)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBlockPairs message handlers

BOOL CBlockPairs::OnInitDialog() 
{
	CDialog::OnInitDialog();
//
//  Make sure there's a pointer to the open table
//
  if(m_pDI == NULL)
  {
    OnCancel();
    return TRUE;
  }

  m_numConnectionAlerts = 0;

//
//  Set up pointers to the controls
//
  pButtonSTANDARD = (CButton *)GetDlgItem(BLOCKPAIRS_STANDARD);
  pButtonDROPBACK = (CButton *)GetDlgItem(BLOCKPAIRS_DROPBACK);
  pButtonMIN = (CButton *)GetDlgItem(BLOCKPAIRS_MIN);
  pButtonMAX = (CButton *)GetDlgItem(BLOCKPAIRS_MAX);
  pEditMINVEHICLES = (CEdit *)GetDlgItem(BLOCKPAIRS_MINVEHICLES);
  pEditMAXVEHICLES = (CEdit *)GetDlgItem(BLOCKPAIRS_MAXVEHICLES);
  pButtonILYES = (CButton *)GetDlgItem(BLOCKPAIRS_ILYES);
  pButtonILNO = (CButton *)GetDlgItem(BLOCKPAIRS_ILNO);
  pButtonDHYES = (CButton *)GetDlgItem(BLOCKPAIRS_DHYES);
  pButtonDHNO = (CButton *)GetDlgItem(BLOCKPAIRS_DHNO);
  pButtonDEADHEADSTOUSE = (CButton *)GetDlgItem(BLOCKPAIRS_DEADHEADSTOUSE);
  pButtonDEADHEADALL = (CButton *)GetDlgItem(BLOCKPAIRS_DEADHEADALL);
  pButtonDEADHEADTHOSE = (CButton *)GetDlgItem(BLOCKPAIRS_DEADHEADTHOSE);
  pEditDEADHEADMINS = (CEdit *)GetDlgItem(BLOCKPAIRS_DEADHEADMINS);
  pStaticTEXT_MINUTES = (CStatic *)GetDlgItem(BLOCKPAIRS_TEXT_MINUTES);
//
//  Default to "Min" vechicle requirements, but with a blank field
//
  pButtonMIN->SetCheck(TRUE);
  pEditMINVEHICLES->SetWindowText("");
  pEditMAXVEHICLES->SetWindowText("");
//
//  Default to allowing deadheading and interlining
//
  pButtonILYES->SetCheck(TRUE);
  pButtonDHYES->SetCheck(TRUE);
  pButtonDEADHEADALL->SetCheck(TRUE);
//
//  Clear out edit controls associated with a spin control
//
  pEditDEADHEADMINS->SetWindowText("");
//
//  Default to this table's blocking technique
//
  if(m_pDI->fileInfo.fileNumber == TMS_STANDARDBLOCKS)
  {
    pButtonSTANDARD->SetCheck(TRUE);
    pButtonDROPBACK->EnableWindow(FALSE);
  }
  else
  {
    pButtonDROPBACK->SetCheck(TRUE);
    pButtonSTANDARD->EnableWindow(FALSE);
  }

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CBlockPairs::OnClose() 
{
  OnCancel();
}

void CBlockPairs::OnHelp() 
{
}

void CBlockPairs::OnCancel() 
{
	CDialog::OnCancel();
}


void CBlockPairs::OnMin() 
{
  pEditMINVEHICLES->EnableWindow(TRUE);
  pEditMAXVEHICLES->EnableWindow(FALSE);
}

void CBlockPairs::OnMax() 
{
  pEditMINVEHICLES->EnableWindow(FALSE);
  pEditMAXVEHICLES->EnableWindow(TRUE);
}


void CBlockPairs::OnDhno() 
{
  Dhwhich(FALSE);
}

void CBlockPairs::OnDhyes() 
{
  Dhwhich(TRUE);
}

void CBlockPairs::Dhwhich(BOOL bEnable)
{
  pButtonDEADHEADALL->EnableWindow(bEnable);
  pButtonDEADHEADSTOUSE->EnableWindow(bEnable);
  pButtonDEADHEADTHOSE->EnableWindow(bEnable);
  pEditDEADHEADMINS->EnableWindow(bEnable);
  pStaticTEXT_MINUTES->EnableWindow(bEnable);
}

typedef struct PAIRSStruct
{
  long blockNumber;
  long TRIPSrecordIDs[50];
} PAIRSDef;


void CBlockPairs::OnOK() 
{
  PAIRSDef PAIRS[1000];
  GenerateTripDef      GTResults;
  GetConnectionTimeDef GCTData;
  PARMSDef PARMS;
  BLOCKSDef  *pTRIPSChunk;
  LAYOVERDef *pLayover;
  CString s;
  float distance;
  long previousBlock;
  long dhd;
  long minutes;
  long percent;
  long lastBlock;
  long lastTRIPSrecordID;
  int  maxVehicles;
  int  minVehicles;
  int  layoverType;
  int  numPairs;
  int  rcode2;
  int  nI, nJ;
  int  numTrips;
  int  index;


  m_bNetworkMode = FALSE;
//
//  Get the parameters
//
  PARMS.bStandard = pButtonSTANDARD->GetCheck();
  PARMS.bDropback = pButtonDROPBACK->GetCheck();
//
//  At least one of Standard or Dropback must be chosen prior to pressing Optimal
//
  if(!PARMS.bStandard && !PARMS.bDropback)
  {
    TMSError(this->m_hWnd, MB_ICONSTOP, ERROR_135, pButtonSTANDARD->m_hWnd);
    return;
  }
//
//  Get the rest of the parameters
//
  PARMS.bInterlining = pButtonILYES->GetCheck();
  PARMS.bDeadheading = pButtonDHYES->GetCheck();
  if(!PARMS.bDeadheading)
  {
    PARMS.bDeadheadAll = FALSE;
    PARMS.maxDeadheadTime = NO_TIME;
  }
  else
  {
    pEditDEADHEADMINS->GetWindowText(tempString, TEMPSTRING_LENGTH);
    if(strcmp(tempString, "") == 0)
    {
      PARMS.bDeadheadAll = TRUE;
      PARMS.maxDeadheadTime = NO_TIME;
    }
    else
    {
      PARMS.bDeadheadAll = FALSE;
      PARMS.maxDeadheadTime = atol(tempString) * 60;
    }
  }
//
//  Get the minimum and maximum number of vehicles parameters.
//
  if(pButtonMIN->GetCheck())
  {
    maxVehicles = NO_TIME;
    pEditMINVEHICLES->GetWindowText(tempString, TEMPSTRING_LENGTH);
    minVehicles = (strcmp(tempString, "") == 0 ? NO_TIME : atol(tempString));
  }
  else
  {
    minVehicles = NO_TIME;
    pEditMAXVEHICLES->GetWindowText(tempString, TEMPSTRING_LENGTH);
    maxVehicles = (strcmp(tempString, "") == 0 ? NO_TIME : atol(tempString));
  }
//
//  Tell him what's going on
//
  if(PARMS.bStandard)
  {
    StatusBarStart(this->m_hWnd, "Optimal Standard Blocking Status");
    pTRIPSChunk = &TRIPS.standard;
    pLayover = &StandardLayover;
    layoverType = TMS_STANDARDBLOCKS;
  }
  else
  {
    StatusBarStart(this->m_hWnd, "Optimal Dropback Blocking Status");
    pTRIPSChunk = &TRIPS.dropback;
    pLayover = &DropbackLayover;
    layoverType = TMS_DROPBACKBLOCKS;
  }
  StatusBar(-1L, -1L);
  StatusBarText("Initializing...");
//
//  Initialize the interlining option
//
  BLSetOptions(pButtonILYES->GetCheck());
//
//  Generate trip data into TRIPINFO
//
  StatusBarText("Building trip data...");
  StatusBar(0L, 0L);
  BLInitNetwork();
//
//  Build the list of trip "pairs".  A trip pair is defined, really, as one
//  or more trips hooked together into a block in the current blocks table.
//
  TRIPSKey2.assignedToNODESrecordID = NO_RECORD;
  TRIPSKey2.RGRPROUTESrecordID = m_pDI->fileInfo.routeRecordID;
  TRIPSKey2.SGRPSERVICESrecordID = m_pDI->fileInfo.serviceRecordID;
  TRIPSKey2.blockNumber = NO_RECORD;
  TRIPSKey2.blockSequence = NO_RECORD;
  numPairs = 0;
  numTrips = 0;
  previousBlock = NO_RECORD;
  rcode2 = btrieve(B_GETGREATER, TMS_TRIPS, &TRIPS, &TRIPSKey2, 2);
  while(rcode2 == 0 &&
        pTRIPSChunk->RGRPROUTESrecordID == m_pDI->fileInfo.routeRecordID &&
        pTRIPSChunk->SGRPSERVICESrecordID == m_pDI->fileInfo.serviceRecordID)
  {
    if(previousBlock != pTRIPSChunk->blockNumber)
    {
      if(previousBlock != NO_RECORD)
      {
        PAIRS[numPairs].TRIPSrecordIDs[numTrips] = NO_RECORD;
        numPairs++;
        numTrips = 0;
      }
      previousBlock = pTRIPSChunk->blockNumber;
    }
    PAIRS[numPairs].blockNumber = pTRIPSChunk->blockNumber;
    PAIRS[numPairs].TRIPSrecordIDs[numTrips] = TRIPS.recordID;
    numTrips++;
    rcode2 = btrieve(B_GETNEXT, TMS_TRIPS, &TRIPS, &TRIPSKey2, 2);
  }
  if(previousBlock != NO_RECORD)
  {
    PAIRS[numPairs].TRIPSrecordIDs[numTrips] = NO_RECORD;
    numPairs++;
  }
//
//  Go through the "pairs" to setup input to the blocker
//
  for(nI = 0; nI < numPairs; nI++)
  {
    StatusBar((long)nI, (long)numPairs);
    if(StatusBarAbort())
    {
      goto OptimalBlockCleanup;
    }
    TRIPSKey0.recordID = PAIRS[nI].TRIPSrecordIDs[0];
    btrieve(B_GETEQUAL, TMS_TRIPS, &TRIPS, &TRIPSKey0, 0);
    nJ = GenerateTrip(TRIPS.ROUTESrecordID, TRIPS.SERVICESrecordID,
          TRIPS.directionIndex, TRIPS.PATTERNNAMESrecordID,
          TRIPS.timeAtMLP, GENERATETRIP_FLAG_DISPLAYERRORS, &GTResults);
//
//  If we've got a one-node trip, fool the blocker
//
    if(GTResults.firstNODESrecordID == GTResults.lastNODESrecordID &&
          GTResults.firstNodeTime == GTResults.lastNodeTime)
    {
      GTResults.lastNodeTime++;
    }
//
//  Save the data on a valid trip
//
    if(nJ == 0)
    {
      continue;
    }
    TRIPINFODef TRIPINFOrec;
    TRIPINFOrec.TRIPSrecordID = nI;
    TRIPINFOrec.BUSTYPESrecordID = TRIPS.BUSTYPESrecordID;
    TRIPINFOrec.ROUTESrecordID = TRIPS.ROUTESrecordID;
//
//  If we're doing a dropback, and the standard block pulls out or in,
//  then replace the start/end node/time with the pullout/in gar/time.
//
//  Check pullouts
//
    if(PARMS.bStandard || TRIPS.standard.POGNODESrecordID == NO_RECORD)
    {
      TRIPINFOrec.startNODESrecordID = GTResults.firstNODESrecordID;
      TRIPINFOrec.startTime = GTResults.firstNodeTime;
    }
    else
    {
      GCTData.fromPATTERNNAMESrecordID = TRIPS.PATTERNNAMESrecordID;
      GCTData.toPATTERNNAMESrecordID = TRIPS.PATTERNNAMESrecordID;
      GCTData.fromROUTESrecordID = TRIPS.ROUTESrecordID;
      GCTData.fromSERVICESrecordID = TRIPS.SERVICESrecordID;
      GCTData.toROUTESrecordID = TRIPS.ROUTESrecordID;
      GCTData.toSERVICESrecordID = TRIPS.SERVICESrecordID;
      GCTData.fromNODESrecordID = TRIPS.standard.POGNODESrecordID;
      GCTData.toNODESrecordID = GTResults.firstNODESrecordID;
      GCTData.timeOfDay = GTResults.firstNodeTime;
      dhd = GetConnectionTime(GCT_FLAG_DEADHEADTIME, &GCTData, &distance);
      if(dhd == NO_TIME)
      {
        dhd = 0;
      }
      TRIPINFOrec.startNODESrecordID = TRIPS.standard.POGNODESrecordID;
      TRIPINFOrec.startTime = GTResults.firstNodeTime - dhd;
    }
//
//  Get the last trip in the pair
//
    lastTRIPSrecordID = PAIRS[nI].TRIPSrecordIDs[0];
    for(nJ = 0; nJ < 50; nJ++)
    {
      if(PAIRS[nI].TRIPSrecordIDs[nJ] == NO_RECORD)
      {
        break;
      }
      lastTRIPSrecordID = PAIRS[nI].TRIPSrecordIDs[nJ];
    }
//
//  If necessary
//
    if(PAIRS[nI].TRIPSrecordIDs[0] != lastTRIPSrecordID)
    {
      TRIPSKey0.recordID = lastTRIPSrecordID;
      btrieve(B_GETEQUAL, TMS_TRIPS, &TRIPS, &TRIPSKey0, 0);
      nJ = GenerateTrip(TRIPS.ROUTESrecordID, TRIPS.SERVICESrecordID,
            TRIPS.directionIndex, TRIPS.PATTERNNAMESrecordID,
            TRIPS.timeAtMLP, GENERATETRIP_FLAG_DISPLAYERRORS, &GTResults);
      if(nJ == 0)
      {
        continue;
      }
    }
//
//  Check pullins
//
    if(PARMS.bStandard || TRIPS.standard.PIGNODESrecordID == NO_RECORD)
    {
      TRIPINFOrec.endNODESrecordID = GTResults.lastNODESrecordID;
      TRIPINFOrec.endTime = GTResults.lastNodeTime;
    }
    else
    {
      GCTData.fromPATTERNNAMESrecordID = TRIPS.PATTERNNAMESrecordID;
      GCTData.toPATTERNNAMESrecordID = TRIPS.PATTERNNAMESrecordID;
      GCTData.fromROUTESrecordID = TRIPS.ROUTESrecordID;
      GCTData.fromSERVICESrecordID = TRIPS.SERVICESrecordID;
      GCTData.toROUTESrecordID = TRIPS.ROUTESrecordID;
      GCTData.toSERVICESrecordID = TRIPS.SERVICESrecordID;
      GCTData.fromNODESrecordID = GTResults.lastNODESrecordID;
      GCTData.toNODESrecordID = TRIPS.standard.PIGNODESrecordID;
      GCTData.timeOfDay = GTResults.lastNodeTime;
      dhd = GetConnectionTime(GCT_FLAG_DEADHEADTIME, &GCTData, &distance);
      if(dhd == NO_TIME)
      {
        dhd = 0;
      }
      TRIPINFOrec.endNODESrecordID = TRIPS.standard.PIGNODESrecordID;
      TRIPINFOrec.endTime = GTResults.lastNodeTime + dhd;
    }
//
//  Figure out the minimum layover
//
    if(pTRIPSChunk->layoverMin != NO_TIME)
    {
      TRIPINFOrec.layoverMin = pTRIPSChunk->layoverMin;
    }
    else
    {
      minutes = pLayover->minimum.minutes  * 60;
      percent = (GTResults.lastNodeTime - GTResults.firstNodeTime) * pLayover->minimum.percent / 100;
      if(pLayover->minimum.type == 0)
      {
        TRIPINFOrec.layoverMin = minutes;
      }
      else if(pLayover->minimum.type == 1)
      {
        TRIPINFOrec.layoverMin = percent;
      }
      else if(pLayover->minimum.type == 2)
      {
        TRIPINFOrec.layoverMin = pLayover->minimum.lower ? min(minutes, percent) : max(minutes, percent);
      }
    }
//
//  Figure out the maximum layover
//
    if(pTRIPSChunk->layoverMax != NO_TIME)
    {
      TRIPINFOrec.layoverMax = pTRIPSChunk->layoverMax;
    }
    else
    {
      minutes = pLayover->maximum.minutes * 60;
      percent = (GTResults.lastNodeTime - GTResults.firstNodeTime) * pLayover->maximum.percent / 100;
      if(pLayover->maximum.type == 0)
      {
        TRIPINFOrec.layoverMax = minutes;
      }
      else if(pLayover->maximum.type == 1)
      {
        TRIPINFOrec.layoverMax = percent;
      }
      else if(pLayover->maximum.type == 2)
      {
        TRIPINFOrec.layoverMax = pLayover->maximum.lower ? min(minutes, percent) : max(minutes, percent);
      }
    }
//
//  And generate the trip arc
//
    BLGenerateTripArc( &TRIPINFOrec );
  }
//
//  If m_numConnectionAlerts is non-zero, blow him off
//
  if(m_numConnectionAlerts != 0)
  {
    TMSError(this->m_hWnd, MB_ICONSTOP, ERROR_173, (HANDLE)NULL);
    goto OptimalBlockCleanup;
  }
//
//  Generate the wait arcs between the trips
//
  StatusBarText("Generating wait arcs...");
  StatusBar(-1L, -1L);
  BLGenerateWaitArcs(FALSE);
//
//  Generate deadhead and equivalence data into DEADHEADINFO
//
  rcode2 = btrieve(B_GETFIRST, TMS_CONNECTIONS, &CONNECTIONS, &CONNECTIONSKey0, 0);
  numDeadheadsForBlocker = 0;
  while(rcode2 == 0)
  {
    if(StatusBarAbort())
    {
      goto OptimalBlockCleanup;
    }
    if(PARMS.bDeadheading)
    {
      if((CONNECTIONS.flags & CONNECTIONS_FLAG_DEADHEADTIME) &&
            (PARMS.bDeadheadAll || CONNECTIONS.connectionTime <= PARMS.maxDeadheadTime))
      {
        DEADHEADINFODef DEADHEADINFOrec;
        DEADHEADINFOrec.fromTimeOfDay = CONNECTIONS.fromTimeOfDay;
        DEADHEADINFOrec.toTimeOfDay = CONNECTIONS.toTimeOfDay;
        DEADHEADINFOrec.fromNODESrecordID = CONNECTIONS.fromNODESrecordID;
        DEADHEADINFOrec.toNODESrecordID = CONNECTIONS.toNODESrecordID;
        DEADHEADINFOrec.flags = CONNECTIONS.flags;
        DEADHEADINFOrec.deadheadTime = CONNECTIONS.connectionTime;
        BLGenerateDeadheadArcs( &DEADHEADINFOrec );
        numDeadheadsForBlocker++;
      }
    }
    if(CONNECTIONS.flags & CONNECTIONS_FLAG_EQUIVALENT)
    {
      DEADHEADINFODef DEADHEADINFOrec;
      DEADHEADINFOrec.fromTimeOfDay = CONNECTIONS.fromTimeOfDay;
      DEADHEADINFOrec.toTimeOfDay = CONNECTIONS.toTimeOfDay;
      DEADHEADINFOrec.fromNODESrecordID = CONNECTIONS.fromNODESrecordID;
      DEADHEADINFOrec.toNODESrecordID = CONNECTIONS.toNODESrecordID;
      DEADHEADINFOrec.flags = CONNECTIONS.flags;
      DEADHEADINFOrec.deadheadTime = 0;
      BLGenerateDeadheadArcs( &DEADHEADINFOrec );
      numDeadheadsForBlocker++;
    }
    rcode2 = btrieve(B_GETNEXT, TMS_CONNECTIONS, &CONNECTIONS, &CONNECTIONSKey0, 0);
  }
  if(StatusBarAbort())
  {
    goto OptimalBlockCleanup;
  }
//
//  Call the blocker
//
  m_numTripsForBlocker = numPairs;
  m_pBLOCKSDATA = (BLOCKSDATADef *)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(BLOCKSDATADef) * numPairs); 
  if(m_pBLOCKSDATA == NULL)
  {
    AllocationError(__FILE__, __LINE__, FALSE);
    goto OptimalBlockCleanup;
  }
  StatusBarText("Optimal blocking in progress...");
  sblock(minVehicles, maxVehicles);
  if(StatusBarAbort())
  {
    goto OptimalBlockCleanup;
  }
//
//  Update the file with the results
//
  lastBlock = 0;
  StatusBarText("Blocking complete - writing results...");
  StatusBarEnableCancel(FALSE);
  for(nJ = 0; nJ < numPairs; nJ++)
  {
    StatusBar((long)nJ, (long)numPairs);
    index = (int)m_pBLOCKSDATA[nJ].TRIPSrecordID;
    TRIPSKey0.recordID = PAIRS[index].TRIPSrecordIDs[0];
    numTrips = 0;
    rcode2 = btrieve(B_GETEQUAL, TMS_TRIPS, &TRIPS, &TRIPSKey0, 0);
    for(;;)
    {
      if(PAIRS[index].TRIPSrecordIDs[numTrips] == NO_RECORD)
      {
        break;
      }
      if(rcode2 != 0)
      {
        TMSError(this->m_hWnd, MB_ICONSTOP, ERROR_015, (HANDLE)NULL);
        goto OptimalBlockCleanup;
      }
      pTRIPSChunk->blockNumber = m_pBLOCKSDATA[nJ].blockNumber;
      btrieve(B_UPDATE, TMS_TRIPS, &TRIPS, &TRIPSKey0, 0);
      numTrips++;
      if(PAIRS[index].TRIPSrecordIDs[numTrips] == NO_RECORD)
      {
        break;
      }
      TRIPSKey0.recordID = PAIRS[index].TRIPSrecordIDs[numTrips];
      rcode2 = btrieve(B_GETEQUAL, TMS_TRIPS, &TRIPS, &TRIPSKey0, 0);
    }
  }

  OptimalBlockCleanup:
    BLCleanup();
    TMSHeapFree(m_pBLOCKSDATA);
    m_numTripsForBlocker = 0;
    StatusBarEnd();
	
	CDialog::OnOK();
}

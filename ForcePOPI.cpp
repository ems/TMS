// ForcePOPI.cpp : implementation file
//

#include "stdafx.h"
extern "C" {
#include "TMSHeader.h"
}
#pragma warning (disable: 4786)
#include "TMS.h"
#include "ForcePOPI.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CForcePOPI dialog


CForcePOPI::CForcePOPI(CWnd* pParent, PDISPLAYINFO pDI)
	: CDialog(CForcePOPI::IDD, pParent)
{
  m_pDI = pDI;

	//{{AFX_DATA_INIT(CForcePOPI)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CForcePOPI::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CForcePOPI)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CForcePOPI, CDialog)
	//{{AFX_MSG_MAP(CForcePOPI)
	ON_WM_CLOSE()
	ON_CBN_SELCHANGE(FORCEPOPI_GARAGELIST, OnSelchangeGaragelist)
	ON_BN_CLICKED(FORCEPOPI_TOCLOSEST, OnToclosest)
	ON_BN_CLICKED(FORCEPOPI_TOGARAGE, OnTogarage)
	ON_BN_CLICKED(IDHELP, OnHelp)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CForcePOPI message handlers

BOOL CForcePOPI::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
  pButtonTOCLOSEST = (CButton *)GetDlgItem(FORCEPOPI_TOCLOSEST);
  pButtonTOGARAGE = (CButton *)GetDlgItem(FORCEPOPI_TOGARAGE);
  pComboBoxGARAGELIST = (CComboBox *)GetDlgItem(FORCEPOPI_GARAGELIST);
	
//
//  Set up the list of garages
//
  int numNodes = SetUpNodeList(this->m_hWnd, FORCEPOPI_GARAGELIST, TRUE);
  
  if(numNodes == 0)
  {
    TMSError(this->m_hWnd, MB_OK, ERROR_045, (HANDLE)NULL);
    OnCancel();
  }
  else
  {
    pButtonTOCLOSEST->SetCheck(TRUE);
    pComboBoxGARAGELIST->SetCurSel(-1);
  }

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CForcePOPI::OnClose() 
{
  OnCancel();
}

void CForcePOPI::OnSelchangeGaragelist() 
{
  pButtonTOCLOSEST->SetCheck(FALSE);
  pButtonTOGARAGE->SetCheck(TRUE);
}

void CForcePOPI::OnToclosest() 
{
  pComboBoxGARAGELIST->SetCurSel(-1);
}

void CForcePOPI::OnTogarage() 
{
  pComboBoxGARAGELIST->SetCurSel(0);
}

void CForcePOPI::OnCancel() 
{
	CDialog::OnCancel();
}

void CForcePOPI::OnHelp() 
{
}

long CForcePOPI::GetClosest(long NODESrecordID)
{
  GetConnectionTimeDef GCTData;
  float distance;
  long  bestGarage;
  long  bestTime;
  long  deadheadTime;
  int   nI;
  int   numGarages;
//
//  Cycle through the garages in the combobox
//
  bestTime = NO_TIME;
  bestGarage = NO_RECORD;
  numGarages = pComboBoxGARAGELIST->GetCount();
  for(nI = 0; nI < numGarages; nI++)
  {
    GCTData.fromPATTERNNAMESrecordID = NO_RECORD;
    GCTData.toPATTERNNAMESrecordID = NO_RECORD;
    GCTData.fromROUTESrecordID = NO_RECORD;
    GCTData.fromSERVICESrecordID = NO_RECORD;
    GCTData.toROUTESrecordID = NO_RECORD;
    GCTData.toSERVICESrecordID = NO_RECORD;
    GCTData.fromNODESrecordID = pComboBoxGARAGELIST->GetItemData(nI);
    GCTData.toNODESrecordID = NODESrecordID;
    GCTData.timeOfDay = NO_TIME;
    deadheadTime = GetConnectionTime(GCT_FLAG_DEADHEADTIME, &GCTData, &distance);
    distance = (float)fabs((double)distance);
    if(deadheadTime == NO_TIME)
    {
      continue;
    }
    if(bestTime == NO_TIME || deadheadTime < bestTime)
    {
      bestGarage = pComboBoxGARAGELIST->GetItemData(nI);
      bestTime = deadheadTime;
    }
  } 
  return(bestGarage); 
}

void CForcePOPI::OnOK() 
{
  long garageNODESrecordID;
  long RUNSrecordID;
  long previousNODESrecordID;
  long previousTRIPSrecordID;
  long startNODESrecordID[10];
  long endNODESrecordID[10];
  long runNumber;
  BOOL bGetClosest;
  int  nI;
  int  rcode2;
  int  numPieces;
  int  arrayPieceNumber;
//
//  Get the garage
//
//  Closest
//
  if(pButtonTOCLOSEST->GetCheck())
  {
    bGetClosest = TRUE;
  }
//
//  Specific garage
//
  else
  {
    nI = pComboBoxGARAGELIST->GetCurSel();
    if(nI == CB_ERR)
    {
      return;
    }
    garageNODESrecordID = pComboBoxGARAGELIST->GetItemData(nI);
    bGetClosest = FALSE;
  }
//
//  Cycle through the runs
//
  RUNSKey1.DIVISIONSrecordID = m_pDI->fileInfo.divisionRecordID;
  RUNSKey1.SERVICESrecordID = m_pDI->fileInfo.serviceRecordID;
  RUNSKey1.runNumber = NO_RECORD;
  RUNSKey1.pieceNumber = NO_RECORD;
  rcode2 = btrieve(B_GETGREATER, TMS_RUNS, &RUNS, &RUNSKey1, 1);
//
//  Cycle through the runs
//
  while(rcode2 == 0 && 
        RUNS.DIVISIONSrecordID == m_pDI->fileInfo.divisionRecordID &&
        RUNS.SERVICESrecordID == m_pDI->fileInfo.serviceRecordID)
  {
    RUNSrecordID = RUNS.recordID;
//
//  If this is piece 1, see if any of the other pieces start/end at a garage
//  this will tell us if we have to start or finish the previous/next piece
//  at the same place
//
    arrayPieceNumber = RUNS.pieceNumber - 1;
    if(RUNS.pieceNumber == 1)
    {
      numPieces = 0;
      runNumber = RUNS.runNumber;
      while(rcode2 == 0 && 
            RUNS.DIVISIONSrecordID == m_pDI->fileInfo.divisionRecordID &&
            RUNS.SERVICESrecordID == m_pDI->fileInfo.serviceRecordID &&
            RUNS.runNumber == runNumber)
      {
        TRIPSKey0.recordID = RUNS.start.TRIPSrecordID;
        btrieve(B_GETEQUAL, TMS_TRIPS, &TRIPS, &TRIPSKey0, 0);
        startNODESrecordID[numPieces] = TRIPS.standard.POGNODESrecordID;
        TRIPSKey0.recordID = RUNS.end.TRIPSrecordID;
        btrieve(B_GETEQUAL, TMS_TRIPS, &TRIPS, &TRIPSKey0, 0);
        endNODESrecordID[numPieces] = TRIPS.standard.PIGNODESrecordID;
        numPieces++;
        rcode2 = btrieve(B_GETNEXT, TMS_RUNS, &RUNS, &RUNSKey1, 1);
      }
      RUNSKey0.recordID = RUNSrecordID;
      rcode2 = btrieve(B_GETEQUAL, TMS_RUNS, &RUNS, &RUNSKey0, 0);
      rcode2 = btrieve(B_GETPOSITION, TMS_RUNS, &RUNS, &RUNSKey0, 0);
      rcode2 = btrieve(B_GETDIRECT, TMS_RUNS, &RUNS, &RUNSKey1, 1);
    }
//
//  Save what we were
//
    previousTRIPSrecordID = RUNS.start.TRIPSrecordID;
    previousNODESrecordID = RUNS.start.NODESrecordID;
//
//  Get the start trip
//
    TRIPSKey0.recordID = RUNS.start.TRIPSrecordID;
    btrieve(B_GETEQUAL, TMS_TRIPS, &TRIPS, &TRIPSKey0, 0);
//
//  Only go through this if the start of the piece isn't a P/O or we're on the second or greater piece
//
    if(RUNS.start.NODESrecordID != TRIPS.standard.POGNODESrecordID || RUNS.pieceNumber > 1)
    {
//
//  Only establish the start garage on piece 1.  Use the previous piece's end for other pieces
//
      if(RUNS.pieceNumber == 1)
      {
        if(bGetClosest)
        {
          garageNODESrecordID = GetClosest(RUNS.end.NODESrecordID);
        }
      }
//
//  garageNODESrecordID is a fall-through from the previous piece
//
//
//  If this was already P/O, then change the garage on the trip and update the run
//
      if(RUNS.start.NODESrecordID == TRIPS.standard.POGNODESrecordID)
      {
        TRIPS.standard.POGNODESrecordID = garageNODESrecordID;
        btrieve(B_UPDATE, TMS_TRIPS, &TRIPS, &TRIPSKey0, 0);
        RUNS.start.NODESrecordID = garageNODESrecordID;
        btrieve(B_UPDATE, TMS_RUNS, &RUNS, &RUNSKey1, 1);
      }
//
//  Not a P/O - Change the run to be a P/O on the next trip in the block and the trip to be a P/O
//
      else
      {
        rcode2 = btrieve(B_GETPOSITION, TMS_TRIPS, &TRIPS, &TRIPSKey0, 0);
        rcode2 = btrieve(B_GETDIRECT, TMS_TRIPS, &TRIPS, &TRIPSKey2, 2);
        rcode2 = btrieve(B_GETNEXT, TMS_TRIPS, &TRIPS, &TRIPSKey2, 2);
        RUNS.start.TRIPSrecordID = TRIPS.recordID;
        RUNS.start.NODESrecordID = garageNODESrecordID;
        btrieve(B_UPDATE, TMS_RUNS, &RUNS, &RUNSKey1, 1);
        TRIPS.standard.POGNODESrecordID = garageNODESrecordID;
        btrieve(B_UPDATE, TMS_TRIPS, &TRIPS, &TRIPSKey2, 2);
//
//  Find the run that finished where we originally started
//
        rcode2 = btrieve(B_GETFIRST, TMS_RUNS, &RUNS, &RUNSKey0, 0);
        while(rcode2 == 0)
        {
          if(RUNS.end.NODESrecordID == previousNODESrecordID &&
                RUNS.end.TRIPSrecordID == previousTRIPSrecordID)
          {
            TRIPSKey0.recordID = previousTRIPSrecordID;
            rcode2 = btrieve(B_GETEQUAL, TMS_TRIPS, &TRIPS, &TRIPSKey0, 0);
            rcode2 = btrieve(B_GETPOSITION, TMS_TRIPS, &TRIPS, &TRIPSKey0, 0);
            rcode2 = btrieve(B_GETDIRECT, TMS_TRIPS, &TRIPS, &TRIPSKey2, 2);
//
//  Change the trip to P/I
//
            TRIPS.standard.PIGNODESrecordID = garageNODESrecordID;
            rcode2 = btrieve(B_UPDATE, TMS_TRIPS, &TRIPS, &TRIPSKey2, 2);
//
//  Change the run to that P/I
//
            RUNS.end.NODESrecordID = garageNODESrecordID;
            rcode2 = btrieve(B_UPDATE, TMS_RUNS, &RUNS, &RUNSKey0, 0);
            break;
          }
          rcode2 = btrieve(B_GETNEXT, TMS_RUNS, &RUNS, &RUNSKey0, 0);
        }
//
//  Re-establish the position into RUNS
//
        RUNSKey0.recordID = RUNSrecordID;
        rcode2 = btrieve(B_GETEQUAL, TMS_RUNS, &RUNS, &RUNSKey0, 0);
        rcode2 = btrieve(B_GETPOSITION, TMS_RUNS, &RUNS, &RUNSKey0, 0);
        rcode2 = btrieve(B_GETDIRECT, TMS_RUNS, &RUNS, &RUNSKey1, 1);
      }
    }
//
//  Get the end trip
//
    TRIPSKey0.recordID = RUNS.end.TRIPSrecordID;
    btrieve(B_GETEQUAL, TMS_TRIPS, &TRIPS, &TRIPSKey0, 0);
//
//  Only go through this if the end of the piece isn't a P/I
//
//  Track the garage for use in the start of the next piece
//
    if(RUNS.end.NODESrecordID == TRIPS.standard.PIGNODESrecordID)
    {
      garageNODESrecordID = TRIPS.standard.PIGNODESrecordID;
    }
    else
    {
      if(arrayPieceNumber + 1 < numPieces && startNODESrecordID[arrayPieceNumber + 1] != NO_RECORD)
      {
        garageNODESrecordID = startNODESrecordID[arrayPieceNumber + 1];
      }
      else
      {
        if(bGetClosest)
        {
          garageNODESrecordID = GetClosest(RUNS.end.NODESrecordID);
        }
      }
//
//  Save where we were
//
      previousTRIPSrecordID = RUNS.end.TRIPSrecordID;
      previousNODESrecordID = RUNS.end.NODESrecordID;
//
//  Change the run to be a P/I
//
      RUNS.end.NODESrecordID = garageNODESrecordID;
      btrieve(B_UPDATE, TMS_RUNS, &RUNS, &RUNSKey1, 1);
//
//  Change the trip to P/I
//
      TRIPS.standard.PIGNODESrecordID = garageNODESrecordID;
      btrieve(B_UPDATE, TMS_TRIPS, &TRIPS, &TRIPSKey0, 0);
//
//  Find the run that starts there
//
      rcode2 = btrieve(B_GETFIRST, TMS_RUNS, &RUNS, &RUNSKey0, 0);
      while(rcode2 == 0)
      {
        if(RUNS.start.NODESrecordID == previousNODESrecordID &&
              RUNS.start.TRIPSrecordID == previousTRIPSrecordID)
        {
          rcode2 = btrieve(B_GETPOSITION, TMS_TRIPS, &TRIPS, &TRIPSKey0, 0);
          rcode2 = btrieve(B_GETDIRECT, TMS_TRIPS, &TRIPS, &TRIPSKey2, 2);
          rcode2 = btrieve(B_GETNEXT, TMS_TRIPS, &TRIPS, &TRIPSKey2, 2);
//
//  Change the trip to P/O
//
          TRIPS.standard.POGNODESrecordID = garageNODESrecordID;
          rcode2 = btrieve(B_UPDATE, TMS_TRIPS, &TRIPS, &TRIPSKey2, 2);
//
//  Change the run to that P/O and trip
//
          RUNS.start.NODESrecordID = garageNODESrecordID;
          RUNS.start.TRIPSrecordID = TRIPS.recordID;
          rcode2 = btrieve(B_UPDATE, TMS_RUNS, &RUNS, &RUNSKey0, 0);
          break;
        }
        rcode2 = btrieve(B_GETNEXT, TMS_RUNS, &RUNS, &RUNSKey0, 0);
      }
//
//  Re-establish the position into RUNS
//
      RUNSKey0.recordID = RUNSrecordID;
      rcode2 = btrieve(B_GETEQUAL, TMS_RUNS, &RUNS, &RUNSKey0, 0);
      rcode2 = btrieve(B_GETPOSITION, TMS_RUNS, &RUNS, &RUNSKey0, 0);
      rcode2 = btrieve(B_GETDIRECT, TMS_RUNS, &RUNS, &RUNSKey1, 1);
    }
    rcode2 = btrieve(B_GETNEXT, TMS_RUNS, &RUNS, &RUNSKey1, 1);
  }

	CDialog::OnOK();
}

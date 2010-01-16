//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
// CutRuns.cpp : implementation file
//

#include "stdafx.h"
extern "C"
{
#include "TMSHeader.h"
#include "cistms.h"

int sort_pieces(const void *a, const void *b)
{
  PROPOSEDRUNPIECESDef *pa, *pb;
  pa = (PROPOSEDRUNPIECESDef *)a;
  pb = (PROPOSEDRUNPIECESDef *)b;
  return(pa->fromTime < pb->fromTime ? -1 : pa->fromTime > pb->fromTime ? 1 : 0);
}

}
#include "CutRuns.h"
#include "ReassignRuntype.h"
#include "RunProperties.h"

extern void EstablishVRBitmaps(CImageList *, VRCOLORSDef *);

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define NUMTITLES 12

int sort_RUNLIST(const void *a, const void *b)
{
  RUNLISTDef *pa, *pb;

  pa = (RUNLISTDef *)a;
  pb = (RUNLISTDef *)b;
  
  return(pa->runNumber < pb->runNumber ? -1 : pa->runNumber > pb->runNumber ? 1 : 0);
}


/////////////////////////////////////////////////////////////////////////////
// CCutRuns dialog


CCutRuns::CCutRuns(CWnd* pParent, VRCOLORSDef* pVRColors)
	: CDialog(CCutRuns::IDD, pParent)
{
  m_pVRColors = pVRColors;
  m_nCaptionSize = GetSystemMetrics(SM_CYCAPTION);

  //{{AFX_DATA_INIT(CCutRuns)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CCutRuns::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CCutRuns)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CCutRuns, CDialog)
	//{{AFX_MSG_MAP(CCutRuns)
	ON_NOTIFY(NM_RCLICK, CUTRUNS_LIST, OnRclickList)
	ON_WM_SIZE()
	ON_WM_PAINT()
	ON_COMMAND(VRR_DESELECTALL, OnDeselectall)
	ON_COMMAND(VRR_FREEZE, OnFreeze)
	ON_COMMAND(VRR_HOOKRUNS, OnHookruns)
	ON_COMMAND(VRR_IMPROVE, OnImprove)
	ON_COMMAND(VRR_PROFILE, OnProfile)
	ON_COMMAND(VRR_PROPERTIES, OnProperties)
	ON_COMMAND(VRR_RENUMBER, OnRenumber)
	ON_COMMAND(VRR_SELECTALL, OnSelectall)
	ON_COMMAND(VRR_SHIFT, OnShift)
	ON_COMMAND(VRR_SUMMARY, OnSummary)
	ON_COMMAND(VRR_SWAP, OnSwap)
	ON_COMMAND(VRR_UNDORUN, OnUndorun)
	ON_COMMAND(VRR_UNHOOK, OnUnhook)
	ON_NOTIFY(NM_DBLCLK, CUTRUNS_LIST, OnDblclkList)
	ON_COMMAND(VRR_REASSIGNRUNTYPE, OnReassignruntype)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

BOOL CCutRuns::OnInitDialog() 
{
	CDialog::OnInitDialog();

  int  nI;

  pListCtrlLIST = (CListCtrl *)GetDlgItem(CUTRUNS_LIST);
  
  DWORD dwExStyles = pListCtrlLIST->GetExtendedStyle();

  pListCtrlLIST->SetExtendedStyle(dwExStyles | LVS_EX_FULLROWSELECT | LVS_EX_HEADERDRAGDROP | LVS_EX_GRIDLINES | LVS_EX_INFOTIP);
//
//  Set up the list control
//
  long defaultMask;

  defaultMask = VRC_COLUMN_RUNTYPE       | VRC_COLUMN_RUNNUMBER    | VRC_COLUMN_PIECENUMBER   |
                VRC_COLUMN_BLOCKNUMBER   | VRC_COLUMN_REPORTAT     | VRC_COLUMN_REPORTTIME    |
                VRC_COLUMN_STARTTRAVEL   | VRC_COLUMN_ONAT         | VRC_COLUMN_ONLOC         |
                VRC_COLUMN_OFFAT         | VRC_COLUMN_OFFLOC       | VRC_COLUMN_ENDTRAVEL     |
                VRC_COLUMN_TURNINTIME    | VRC_COLUMN_PLATFORMTIME | VRC_COLUMN_TOTALTRAVEL   |
                VRC_COLUMN_PAIDBREAKS    | VRC_COLUMN_MAKEUPTIME   | VRC_COLUMN_OVERTIME      |
                VRC_COLUMN_SPREADPREMIUM | VRC_COLUMN_PAYTIME      | VRC_COLUMN_PAYTOPLATFORM |
                VRC_COLUMN_PLATFORMTOPAY | VRC_COLUMN_SPREADTIME;
  
//  m_ColumnMask = GetPrivateProfileInt(userName, "VRRColumnMask", defaultMask, TMSINIFile);
  m_ColumnMask = defaultMask;

//  Get the column titles from the VRRC menu
//
  if(m_ColumnMenu == NULL)
  {
    CString s;
    m_ColumnMenu.LoadMenu(IDR_VRRC);
    CMenu* pMenu = m_ColumnMenu.GetSubMenu(0);

    LVCOLUMN LVC;
    LVC.mask = LVCF_FMT | LVCF_TEXT | LVCF_WIDTH;
    LVC.fmt = LVCFMT_LEFT;
    LVC.cx = 45;
    LVC.pszText = "Status";
    pListCtrlLIST->InsertColumn(0, &LVC);

    for(nI = 0; nI < (int)pMenu->GetMenuItemCount(); nI++)
    {
      if(m_ColumnMask & (1<<nI))
      {
        pMenu->GetMenuString(nI, s, MF_BYPOSITION);
        LVC.cx = (s.Find(':') >= 0 ? 60 : 50);  // Give extra space to Pay:Plat and Plat:Pay ratio columns
        strcpy(tempString, s);
        LVC.pszText = tempString;
        pListCtrlLIST->InsertColumn(nI + 1, &LVC);
      }
    }
    m_ColumnMenu.DestroyMenu();
  }

//
//  Display the runs
//
  ShowRuns(&m_VRColors);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
//
//  SatHello() - Receive the pParent
//
void CCutRuns::SayHello(CWnd* pParent)
{
  m_pVRDlg = pParent;
}


/////////////////////////////////////////////////////////////////////////////
// CCutRuns message handlers

void CCutRuns::OnSize(UINT nType, int cx, int cy) 
{
	CDialog::OnSize(nType, cx, cy);
	
  if(IsWindow(pListCtrlLIST->m_hWnd))
  {
    CRect aRect;
    GetClientRect(&aRect);
    pListCtrlLIST->MoveWindow(aRect);
  }
}

void CCutRuns::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	
	// TODO: Add your message handler code here
	
	// Do not call CDialog::OnPaint() for painting messages
}

void CCutRuns::ShowRuns(VRCOLORSDef* pVRC)
{
  COSTDef COST;
  BOOL bFound;
  long tempLong;
  int RLDIndex;
  int nI;
  int numInternalRuns = AssembleRuns();
  int previousLast = pListCtrlLIST->GetItemCount();


  pVRC = &m_VRColors;

  if(m_ImageList.GetSafeHandle() != NULL)
    m_ImageList.DeleteImageList();
  m_ImageList.Create(16, 15, ILC_COLORDDB, 0, 4);

  EstablishVRBitmaps(&m_ImageList, pVRC);
  
  pListCtrlLIST->SetImageList(&m_ImageList, LVSIL_SMALL);

//
//  Empty the displayed run list.
//
  HCURSOR hSaveCursor = ::SetCursor(hCursorWait);

  pListCtrlLIST->LockWindowUpdate();
  if(previousLast > 0)
    previousLast--;
  pListCtrlLIST->DeleteAllItems();
//
//  Display all the cut runs.
//
  int position = 0;
  qsort((void *)RUNLIST, numInternalRuns, sizeof(RUNLISTDef), sort_RUNLIST);
  for(nI = 0; nI < numInternalRuns; nI++)
  {
//
//  Cost the run
//
    for(bFound = FALSE, RLDIndex = 0; RLDIndex < MAXRUNSINRUNLIST; RLDIndex++)
    {
      if(RUNLIST[nI].runNumber == RUNLISTDATA[RLDIndex].runNumber)
      {
        bFound = TRUE;
        break;
      }
    }
    tempLong = RunCoster(&RUNLIST[nI].run,
          (bFound ? RUNLISTDATA[RLDIndex].cutAsRuntype : NO_RUNTYPE), &COST);
    if(bFound)
    {
      if(RUNLISTDATA[RLDIndex].cutAsRuntype == NO_RUNTYPE)
        RUNLISTDATA[RLDIndex].cutAsRuntype = tempLong;
    }
    else
    {
      for(RLDIndex = 0; RLDIndex < MAXRUNSINRUNLIST; RLDIndex++)
      {
        if(RUNLISTDATA[RLDIndex].runNumber == NO_RECORD)
        {
          RUNLISTDATA[RLDIndex].runNumber = RUNLIST[nI].runNumber;
          RUNLISTDATA[RLDIndex].cutAsRuntype = tempLong;
          break;
        }
      }
    }
    DisplayRun(&position, &RUNLISTDATA[RLDIndex], &RUNLIST[nI], &COST, FALSE);
  }
//
//  Position previousLast
//
  pListCtrlLIST->EnsureVisible(previousLast, TRUE);
  pListCtrlLIST->UnlockWindowUpdate();

  ::SetCursor(hSaveCursor);
}

void CCutRuns::DisplayRun(int *position, RUNLISTDATADef *pRLD,
      RUNLISTDef *pRL, COSTDef *pC, BOOL bSetTopIndex)
{
  int nI;
  int pos = *position;
  LVITEM LVI;
  long totalTravelTime = 0;
//
//  Loop through the pieces
//
  for(nI = 0; nI < pRL->run.numPieces; nI++)
  {
//
//  Status
//
    LVI.mask = LVIF_IMAGE | LVIF_PARAM;
    LVI.iItem = pos;
    LVI.iSubItem = 0;
    if(nI == 0)
      LVI.iImage = pRLD->frozenFlag ? BITMAP_FROZENRUN :
            pC->TOTAL.payTime == 0 ? BITMAP_ILLEGALRUN : BITMAP_LEGALRUN;
    else
      LVI.iImage = BITMAP_BLANK;
    LVI.lParam = pRL->runNumber;
    LVI.iItem = pListCtrlLIST->InsertItem(&LVI);
//
//  Go through the individual columns
//
    LVI.iSubItem = 1;
//
//  Runtype
//
    if(m_ColumnMask & VRC_COLUMN_RUNTYPE)
    {
      if(nI == 0 && pRLD->cutAsRuntype != UNCLASSIFIED_RUNTYPE)
      {
        int type = LOWORD(pRLD->cutAsRuntype);
        int slot = HIWORD(pRLD->cutAsRuntype);
        LVI.mask = LVIF_TEXT;
        LVI.pszText = RUNTYPE[type][slot].localName;
        pListCtrlLIST->SetItem(&LVI);
      }
      LVI.iSubItem++;
    }
//
//  Run number
//
    if(m_ColumnMask & VRC_COLUMN_RUNNUMBER)
    {
      sprintf(tempString, "%d", pRL->runNumber);
      LVI.mask = LVIF_TEXT;
      LVI.pszText = tempString;
      pListCtrlLIST->SetItem(&LVI);
      LVI.iSubItem++;
    }
//
//  Piece number
//
    if(m_ColumnMask & VRC_COLUMN_PIECENUMBER)
    {
      sprintf(tempString, "%d", nI + 1);
      LVI.mask = LVIF_TEXT;
      LVI.pszText = tempString;
      pListCtrlLIST->SetItem(&LVI);
      LVI.iSubItem++;
    }
//
//  Block number
//
    if(m_ColumnMask & VRC_COLUMN_BLOCKNUMBER)
    {
      TRIPSKey0.recordID = pRL->run.piece[nI].fromTRIPSrecordID;
      btrieve(B_GETEQUAL, TMS_TRIPS, &TRIPS, &TRIPSKey0, 0);
      sprintf(tempString, "%ld", TRIPS.standard.blockNumber);
      LVI.mask = LVIF_TEXT;
      LVI.pszText = tempString;
      pListCtrlLIST->SetItem(&LVI);
      LVI.iSubItem++;
    }
//
//  Report at
//
    if(m_ColumnMask & VRC_COLUMN_REPORTAT)
    {
      strcpy(tempString,
            Tchar(pRL->run.piece[nI].fromTime -
                  pC->PIECECOST[nI].reportTime - pC->TRAVEL[nI].startTravelTime));
      LVI.mask = LVIF_TEXT;
      LVI.pszText = tempString;
      pListCtrlLIST->SetItem(&LVI);
      LVI.iSubItem++;
    }
//
//  Report time
//
    if(m_ColumnMask & VRC_COLUMN_REPORTTIME)
    {
      strcpy(tempString, chhmm(pC->PIECECOST[nI].reportTime));
      LVI.mask = LVIF_TEXT;
      LVI.pszText = tempString;
      pListCtrlLIST->SetItem(&LVI);
      LVI.iSubItem++;
    }
//
//  Start travel
//
    if(m_ColumnMask & VRC_COLUMN_STARTTRAVEL)
    {
      strcpy(tempString, chhmm(pC->TRAVEL[nI].startTravelTime));
      LVI.mask = LVIF_TEXT;
      LVI.pszText = tempString;
      pListCtrlLIST->SetItem(&LVI);
      LVI.iSubItem++;
      totalTravelTime  += pC->TRAVEL[nI].startTravelTime == NO_TIME ? 0 : pC->TRAVEL[nI].startTravelTime;
    }
//
//  On time
//
    if(m_ColumnMask & VRC_COLUMN_ONAT)
    {
      strcpy(tempString, Tchar(pRL->run.piece[nI].fromTime));
      LVI.mask = LVIF_TEXT;
      LVI.pszText = tempString;
      pListCtrlLIST->SetItem(&LVI);
      LVI.iSubItem++;
    }
//
//  On location
//
    if(m_ColumnMask & VRC_COLUMN_ONLOC)
    {
      LVI.mask = LVIF_TEXT;
      LVI.pszText = NodeAbbrFromRecID(pRL->run.piece[nI].fromNODESrecordID);
      pListCtrlLIST->SetItem(&LVI);
      LVI.iSubItem++;
    }
//
//  Off time
//
    if(m_ColumnMask & VRC_COLUMN_OFFAT)
    {
      strcpy(tempString, Tchar(pRL->run.piece[nI].toTime));
      LVI.mask = LVIF_TEXT;
      LVI.pszText = tempString;
      pListCtrlLIST->SetItem(&LVI);
      LVI.iSubItem++;
    }
//
//  Off location
//
    if(m_ColumnMask & VRC_COLUMN_OFFLOC)
    {
      LVI.mask = LVIF_TEXT;
      LVI.pszText = NodeAbbrFromRecID(pRL->run.piece[nI].toNODESrecordID);
      pListCtrlLIST->SetItem(&LVI);
      LVI.iSubItem++;
    }
//
//  End travel
//
    if(m_ColumnMask & VRC_COLUMN_ENDTRAVEL)
    {
      strcpy(tempString, chhmm(pC->TRAVEL[nI].endTravelTime));
      LVI.mask = LVIF_TEXT;
      LVI.pszText = tempString;
      pListCtrlLIST->SetItem(&LVI);
      LVI.iSubItem++;
      totalTravelTime += pC->TRAVEL[nI].endTravelTime == NO_TIME ? 0 : pC->TRAVEL[nI].endTravelTime;
    }
//
//  Turn-in time
//
    if(m_ColumnMask & VRC_COLUMN_TURNINTIME)
    {
      strcpy(tempString, chhmm(pC->PIECECOST[nI].turninTime));
      LVI.mask = LVIF_TEXT;
      LVI.pszText = tempString;
      pListCtrlLIST->SetItem(&LVI);
      LVI.iSubItem++;
    }
//
//  Platform time
//
    if(m_ColumnMask & VRC_COLUMN_PLATFORMTIME)
    {
      strcpy(tempString, chhmm(pC->PIECECOST[nI].platformTime));
      LVI.mask = LVIF_TEXT;
      LVI.pszText = tempString;
      pListCtrlLIST->SetItem(&LVI);
      LVI.iSubItem++;
    }
//
//  Total travel
//
    if(m_ColumnMask & VRC_COLUMN_TOTALTRAVEL)
    {
      long start = (pC->TRAVEL[nI].startTravelTime < 0 ? 0 : pC->TRAVEL[nI].startTravelTime);
      long end   = (pC->TRAVEL[nI].endTravelTime   < 0 ? 0 : pC->TRAVEL[nI].endTravelTime);

      strcpy(tempString, chhmm(start + end));
      LVI.mask = LVIF_TEXT;
      LVI.pszText = tempString;
      pListCtrlLIST->SetItem(&LVI);
      LVI.iSubItem++;
    }
//
//  Paid breaks
//
    if(m_ColumnMask & VRC_COLUMN_PAIDBREAKS)
    {
      strcpy(tempString, chhmm(pC->PIECECOST[nI].paidBreak));
      LVI.mask = LVIF_TEXT;
      LVI.pszText = tempString;
      pListCtrlLIST->SetItem(&LVI);
      LVI.iSubItem++;
    }
//
//  Makeup time
//
    if(m_ColumnMask & VRC_COLUMN_MAKEUPTIME)
    {
      if(pRL->run.numPieces == 1)
      {
        strcpy(tempString, chhmm(pC->TOTAL.makeUpTime));
      }
      else
      {
        strcpy(tempString, chhmm(pC->PIECECOST[nI].makeUpTime));
      }
      LVI.mask = LVIF_TEXT;
      LVI.pszText = tempString;
      pListCtrlLIST->SetItem(&LVI);
      LVI.iSubItem++;
    }
//
//  Overtime
//
    if(m_ColumnMask & VRC_COLUMN_OVERTIME)
    {
      if(pRL->run.numPieces == 1)
      {
        strcpy(tempString, chhmm(pC->TOTAL.overTime));
        LVI.mask = LVIF_TEXT;
        LVI.pszText = tempString;
        pListCtrlLIST->SetItem(&LVI);
      }
      LVI.iSubItem++;
    }
//
//  Spread Premium
//
    if(m_ColumnMask & VRC_COLUMN_SPREADPREMIUM)
    {
      if(pRL->run.numPieces == 1)
      {
        strcpy(tempString, chhmm(pC->spreadOvertime));
        LVI.mask = LVIF_TEXT;
        LVI.pszText = tempString;
        pListCtrlLIST->SetItem(&LVI);
      }
      LVI.iSubItem++;
    }
//
//  Pay time
//
    if(m_ColumnMask & VRC_COLUMN_PAYTIME)
    {
      if(pRL->run.numPieces == 1)
      {
        strcpy(tempString, chhmm(pC->TOTAL.payTime));
        LVI.mask = LVIF_TEXT;
        LVI.pszText = tempString;
        pListCtrlLIST->SetItem(&LVI);
      }
      LVI.iSubItem++;
    }
//
//  Pay to platform
//
    if(m_ColumnMask & VRC_COLUMN_PAYTOPLATFORM)
    {
      if(pRL->run.numPieces == 1)
      {
        if(pC->TOTAL.platformTime == 0)
          strcpy(tempString, "");
        else
          sprintf(tempString, "%7.4f", (float)pC->TOTAL.payTime / (float)pC->TOTAL.platformTime);
        LVI.mask = LVIF_TEXT;
        LVI.pszText = tempString;
        pListCtrlLIST->SetItem(&LVI);
      }
      LVI.iSubItem++;
    }
//
//  Platform to pay
//
    if(m_ColumnMask & VRC_COLUMN_PLATFORMTOPAY)
    {
      if(pRL->run.numPieces == 1)
      {
        if(pC->TOTAL.payTime == 0)
          strcpy(tempString, "");
        else
          sprintf(tempString, "%7.4f", (float)pC->TOTAL.platformTime / (float)pC->TOTAL.payTime);
        LVI.mask = LVIF_TEXT;
        LVI.pszText = tempString;
        pListCtrlLIST->SetItem(&LVI);
      }
      LVI.iSubItem++;
    }
//
//  Spread Time
//
    if(m_ColumnMask & VRC_COLUMN_SPREADTIME)
    {
      if(pRL->run.numPieces == 1)
      {
        strcpy(tempString, (pC->spreadTime < 0 ? "" : chhmm(pC->spreadTime))); 
        LVI.mask = LVIF_TEXT;
        LVI.pszText = tempString;
        pListCtrlLIST->SetItem(&LVI);
      }
      LVI.iSubItem++;
    }
//
//  Increment position
//
    pos++;
  }
//
//  Provide a sum if more than one piece
//
  if(pRL->run.numPieces > 1)
  {
//
//  Status
//
    LVI.mask = LVIF_IMAGE | LVIF_PARAM;
    LVI.iItem = pos;
    LVI.iSubItem = 0;
    LVI.iImage = BITMAP_BLANK;
    LVI.lParam = pRL->runNumber;
    LVI.iItem = pListCtrlLIST->InsertItem(&LVI);
//
//  Go through the columns again
//
    LVI.iSubItem = 1;
//
//  Runtype
//
    if(m_ColumnMask & VRC_COLUMN_RUNTYPE)
    {
      LVI.iSubItem++;
    }
//
//  Run number
//
    if(m_ColumnMask & VRC_COLUMN_RUNNUMBER)
    {
      sprintf(tempString, "%d", pRL->runNumber);
      LVI.mask = LVIF_TEXT;
      LVI.pszText = tempString;
      pListCtrlLIST->SetItem(&LVI);
      LVI.iSubItem++;
    }
//
//  Piece number
//
    if(m_ColumnMask & VRC_COLUMN_PIECENUMBER)
    {
      LVI.mask = LVIF_TEXT;
      LVI.pszText = "Total";
      pListCtrlLIST->SetItem(&LVI);
      LVI.iSubItem++;
    }
//
//  Block number - not totalled
//
    if(m_ColumnMask & VRC_COLUMN_BLOCKNUMBER)
    {
      LVI.iSubItem++;
    }
//
//  Report at - not totalled
//
    if(m_ColumnMask & VRC_COLUMN_REPORTAT)
    {
      LVI.iSubItem++;
    }
//
//  Report time
//
    if(m_ColumnMask & VRC_COLUMN_REPORTTIME)
    {
      strcpy(tempString, chhmm(pC->TOTAL.reportTime));
      LVI.mask = LVIF_TEXT;
      LVI.pszText = tempString;
      pListCtrlLIST->SetItem(&LVI);
      LVI.iSubItem++;
    }
//
//  Start travel - not totalled
//
    if(m_ColumnMask & VRC_COLUMN_STARTTRAVEL)
    {
      LVI.iSubItem++;
    }
//
//  On location - not totalled
//
    if(m_ColumnMask & VRC_COLUMN_ONLOC)
    {
      LVI.iSubItem++;
    }
//
//  On time - not totalled
//
    if(m_ColumnMask & VRC_COLUMN_ONAT)
    {
      LVI.iSubItem++;
    }
//
//  Off time - not totalled
//
    if(m_ColumnMask & VRC_COLUMN_OFFAT)
    {
      LVI.iSubItem++;
    }
//
//  Off location - not totalled
//
    if(m_ColumnMask & VRC_COLUMN_OFFLOC)
    {
      LVI.iSubItem++;
    }
//
//  End travel - not totalled
//
    if(m_ColumnMask & VRC_COLUMN_ENDTRAVEL)
    {
      LVI.iSubItem++;
    }
//
//  Turn-in time
//
    if(m_ColumnMask & VRC_COLUMN_TURNINTIME)
    {
      strcpy(tempString, chhmm(pC->TOTAL.turninTime));
      LVI.mask = LVIF_TEXT;
      LVI.pszText = tempString;
      pListCtrlLIST->SetItem(&LVI);
      LVI.iSubItem++;
    }
//
//  Platform time
//
    if(m_ColumnMask & VRC_COLUMN_PLATFORMTIME)
    {
      strcpy(tempString, chhmm(pC->TOTAL.platformTime));
      LVI.mask = LVIF_TEXT;
      LVI.pszText = tempString;
      pListCtrlLIST->SetItem(&LVI);
      LVI.iSubItem++;
    }
//
//  Total travel
//
    if(m_ColumnMask & VRC_COLUMN_TOTALTRAVEL)
    {
      strcpy(tempString, chhmm(totalTravelTime));
      LVI.mask = LVIF_TEXT;
      LVI.pszText = tempString;
      pListCtrlLIST->SetItem(&LVI);
      LVI.iSubItem++;
    }
//
//  Paid breaks
//
    if(m_ColumnMask & VRC_COLUMN_PAIDBREAKS)
    {
      strcpy(tempString, chhmm(pC->TOTAL.paidBreak));
      LVI.mask = LVIF_TEXT;
      LVI.pszText = tempString;
      pListCtrlLIST->SetItem(&LVI);
      LVI.iSubItem++;
    }
//
//  Makeup time
//
    if(m_ColumnMask & VRC_COLUMN_MAKEUPTIME)
    {
      strcpy(tempString, chhmm(pC->TOTAL.makeUpTime));
      LVI.mask = LVIF_TEXT;
      LVI.pszText = tempString;
      pListCtrlLIST->SetItem(&LVI);
      LVI.iSubItem++;
    }
//
//  Overtime
//
    if(m_ColumnMask & VRC_COLUMN_OVERTIME)
    {
      strcpy(tempString, chhmm(pC->TOTAL.overTime));
      LVI.mask = LVIF_TEXT;
      LVI.pszText = tempString;
      pListCtrlLIST->SetItem(&LVI);
      LVI.iSubItem++;
    }
//
//  Spread premium
//
    if(m_ColumnMask & VRC_COLUMN_SPREADPREMIUM)
    {
      strcpy(tempString, chhmm(pC->spreadOvertime));
      LVI.mask = LVIF_TEXT;
      LVI.pszText = tempString;
      pListCtrlLIST->SetItem(&LVI);
      LVI.iSubItem++;
    }
//
//  Pay time
//
    if(m_ColumnMask & VRC_COLUMN_PAYTIME)
    {
      strcpy(tempString, chhmm(pC->TOTAL.payTime));
      LVI.mask = LVIF_TEXT;
      LVI.pszText = tempString;
      pListCtrlLIST->SetItem(&LVI);
      LVI.iSubItem++;
    }
//
//  Pay to platform
//
    if(m_ColumnMask & VRC_COLUMN_PAYTOPLATFORM)
    {
      if(pC->TOTAL.platformTime == 0)
        strcpy(tempString, "");
      else
        sprintf(tempString, "%7.4f", (float)pC->TOTAL.payTime / (float)pC->TOTAL.platformTime);
      LVI.mask = LVIF_TEXT;
      LVI.pszText = tempString;
      pListCtrlLIST->SetItem(&LVI);
      LVI.iSubItem++;
    }
//
//  Platform to pay
//
    if(m_ColumnMask & VRC_COLUMN_PLATFORMTOPAY)
    {
      if(pC->TOTAL.payTime == 0)
        strcpy(tempString, "");
      else
        sprintf(tempString, "%7.4f", (float)pC->TOTAL.platformTime / (float)pC->TOTAL.payTime);
      LVI.mask = LVIF_TEXT;
      LVI.pszText = tempString;
      pListCtrlLIST->SetItem(&LVI);
      LVI.iSubItem++;
    }
//
//  Spread time
//
    if(m_ColumnMask & VRC_COLUMN_SPREADTIME)
    {
      strcpy(tempString, (pC->spreadTime < 0 ? "" : chhmm(pC->spreadTime))); 
      LVI.mask = LVIF_TEXT;
      LVI.pszText = tempString;
      pListCtrlLIST->SetItem(&LVI);
      LVI.iSubItem++;
    }
//
//  Increment position
//
    pos++;
  }
//
//  Draw the separator
//
/*
  int numCols = LVI.iSubItem;

  strcpy(tempString, "...............");
  LVI.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM;
  LVI.iItem = pos;
  LVI.iSubItem = 0;
  LVI.pszText = tempString;
  LVI.lParam = pRL->runNumber;
  LVI.iImage = BITMAP_BLANK;
  LVI.iItem = pListCtrlLIST->InsertItem(&LVI);
  for(nI = 1; nI < numCols; nI++)
  {
    LVI.mask = LVIF_TEXT;
    LVI.iSubItem = nI;
    LVI.pszText = tempString;
    pListCtrlLIST->SetItem(&LVI);
  }
*/
  LVI.mask = LVIF_IMAGE | LVIF_PARAM;
  LVI.iItem = pos;
  LVI.iSubItem = 0;
  LVI.iImage = BITMAP_BLANK;
  LVI.lParam = NO_RECORD;
  LVI.iItem = pListCtrlLIST->InsertItem(&LVI);
//
//  Increment position
//
  pos++;
//
//  All done
//
  *position = pos;

  return;
}

BOOL CCutRuns::DestroyWindow() 
{
  m_ImageList.DeleteImageList();
	pListCtrlLIST->DestroyWindow();

	return CDialog::DestroyWindow();
}

void CCutRuns::OnDeselectall() 
{
  int nI;
  int numRows;
  
  numRows = pListCtrlLIST->GetItemCount();

  for(nI = 0; nI < numRows; nI++)
  {
    if(pListCtrlLIST->GetItemState(nI, LVIS_SELECTED) == LVIS_SELECTED)
      pListCtrlLIST->SetItemState(nI, !LVIS_SELECTED, LVIS_SELECTED);
  }
}

void CCutRuns::OnFreeze() 
{
  CUIntArray runNumbers;
  long runNumber;
  long previousRunNumber;
  int  nI, nJ;
  int  nItem;
  
  POSITION pos = pListCtrlLIST->GetFirstSelectedItemPosition();
  
  if(pos == NULL)
    return;

  previousRunNumber = NO_RECORD;
  runNumbers.RemoveAll();
//
//  Save a list of run numbers
//
  while(pos)
  {
    nItem = pListCtrlLIST->GetNextSelectedItem(pos);
    runNumber = pListCtrlLIST->GetItemData(nItem);
    if(runNumber == NO_RECORD)
      continue;
    if(runNumber == previousRunNumber)
      continue;
    previousRunNumber = runNumber;
    runNumbers.Add(runNumber);
  }

  int numRuns = runNumbers.GetSize();
  if(runNumbers.GetSize() == 0)
    return;
//
//  Loop through the runs
//
  for(nI = 0; nI < numRuns; nI++)
  {
    runNumber = runNumbers.GetAt(nI);
    for(nJ = 0; nJ < MAXRUNSINRUNLIST; nJ++)
    {
      if(RUNLISTDATA[nJ].runNumber == runNumber)
      {
        RUNLISTDATA[nJ].frozenFlag = !RUNLISTDATA[nJ].frozenFlag;
        break;
      }
    }
  }
//
//  Update the displays
//
  AssembleRuns();  // Bring the internal lists up to date
  ShowRuns(m_pVRColors);
  m_pVRDlg->InvalidateRect(NULL, TRUE);
  SetRunAtTop(runNumbers.GetAt(0));
}

void CCutRuns::OnHookruns() 
{
  CUIntArray runNumbers;
  BOOL bFound;
  long runNumber;
  long previousRunNumber;
  int  nI, nJ, nK;
  int  nItem;
  
  POSITION pos = pListCtrlLIST->GetFirstSelectedItemPosition();
  
  if(pos == NULL)
    return;

  previousRunNumber = NO_RECORD;
  runNumbers.RemoveAll();
//
//  Save a list of run numbers
//
  while(pos)
  {
    nItem = pListCtrlLIST->GetNextSelectedItem(pos);
    runNumber = pListCtrlLIST->GetItemData(nItem);
    if(runNumber == NO_RECORD)
      continue;
    if(runNumber == previousRunNumber)
      continue;
    previousRunNumber = runNumber;
    runNumbers.Add(runNumber);
  }

  int numRuns = runNumbers.GetSize();
  if(numRuns == 0)
    return;
  if(numRuns > MAXPIECES)
    return;
//
//  Get the runs to be hooked and place them into a local copy of PROPOSEDRUN
//
  PROPOSEDRUNDef PR;

  PR.numPieces = 0;
  for(nI = 0; nI < numRuns; nI++)
  {
    for(nJ = 0; nJ < MAXRUNSINRUNLIST; nJ++)
    {
      if((long)runNumbers.GetAt(nI) == RUNLIST[nJ].runNumber)
      {
        for(nK = 0; nK < RUNLIST[nJ].run.numPieces; nK++)
        {
          memcpy(&PR.piece[PR.numPieces++], &RUNLIST[nJ].run.piece[nK], sizeof(PROPOSEDRUNPIECESDef)); 
        }
        break;
      }
    }
  }
//
//  Got all the pieces, now sort them in on time order
//
//  Note: sort_pieces() appears in MANCUT.c
//
  qsort((void *)PR.piece, PR.numPieces, sizeof(PROPOSEDRUNPIECESDef), sort_pieces);
//
//  Ensure that none of the pieces overlap.  If any of them do,
//  display the pertinent data of the first overlap and break out.
//
  CString s;

  for(bFound = FALSE, nI = 0; nI < PR.numPieces - 1; nI++)
  {
    if(PR.piece[nI].toTime > PR.piece[nI + 1].fromTime)
    {
      strcpy(tempString, "\t");
      for(nJ = nI; nJ <= nI + 1; nJ++)
      {
        strcat(tempString, NodeAbbrFromRecID(PR.piece[nJ].fromNODESrecordID));
        strcat(tempString, "\t");
        strcat(tempString, Tchar(PR.piece[nJ].fromTime));
        strcat(tempString, "\t");
        strcat(tempString, Tchar(PR.piece[nJ].toTime));
        strcat(tempString, "\t");
        NODESKey0.recordID = PR.piece[nJ].toNODESrecordID;
        btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
        strncpy(szarString, NODES.abbrName, NODES_ABBRNAME_LENGTH);
        trim(szarString, NODES_ABBRNAME_LENGTH);
        strcat(tempString, szarString);
        strcat(tempString, "\n\t");
      }
      s.LoadString(ERROR_152);
      sprintf(szarString, s, tempString);
      MessageBeep(MB_ICONSTOP);
      MessageBox(szarString, TMS, MB_ICONSTOP | MB_OK);
      bFound = TRUE;
      break;
    }
  }
  if(bFound)
    return;
//
//  See if there's a travel entry for the end of a piece to the start of the next
//
  GetConnectionTimeDef GCTData;
  float distance;
  long fromNode;
  long fromTrip;
  long toNode;
  long toTrip;
  tod_t equivalentTravelTime;
  tod_t travelTime;
  tod_t waitTime;
  tod_t startTime;
  tod_t endTime;

  for(nI = 0; nI < PR.numPieces - 1; nI++)
  {
    fromNode = PR.piece[nI].toNODESrecordID;
    fromTrip = PR.piece[nI].toTRIPSrecordID;
    toNode = PR.piece[nI + 1].fromNODESrecordID;
    toTrip = PR.piece[nI + 1].fromTRIPSrecordID;
    if(!NodesEquivalent(fromNode, toNode, &equivalentTravelTime))
    {
      if(bUseDynamicTravels)
      {
        if(bUseCISPlan)
        {
          CISplanReliefConnect(&startTime, &endTime, &waitTime, 0, fromNode, toNode,
                PR.piece[nI + 1].fromTime, FALSE, TRIPS.SERVICESrecordID);
          travelTime = (endTime - startTime) + waitTime;
        }
        else
        {
          travelTime = GetDynamicTravelTime(TRUE, fromNode, toNode,
                TRIPS.SERVICESrecordID, PR.piece[nI + 1].fromTime, &waitTime);
        }
      }
      else
      {
        GCTData.fromNODESrecordID = fromNode;
        TRIPSKey0.recordID = fromTrip;
        btrieve(B_GETEQUAL, TMS_TRIPS, &TRIPS, &TRIPSKey0, 0);
        GCTData.fromROUTESrecordID = TRIPS.ROUTESrecordID;
        GCTData.fromSERVICESrecordID = TRIPS.SERVICESrecordID;
        GCTData.fromPATTERNNAMESrecordID = TRIPS.PATTERNNAMESrecordID;
        GCTData.toNODESrecordID = toNode;
        TRIPSKey0.recordID = toTrip;
        btrieve(B_GETEQUAL, TMS_TRIPS, &TRIPS, &TRIPSKey0, 0);
        GCTData.toROUTESrecordID = TRIPS.ROUTESrecordID;
        GCTData.toSERVICESrecordID = TRIPS.SERVICESrecordID;
        GCTData.toPATTERNNAMESrecordID = TRIPS.PATTERNNAMESrecordID;
        GCTData.timeOfDay = PR.piece[nI].toTime;
        travelTime = GetConnectionTime(GCT_FLAG_TRAVELTIME, &GCTData, &distance);
      }
      if(travelTime == NO_TIME || PR.piece[nI].toTime + travelTime > PR.piece[nI + 1].fromTime)
      {
        s.LoadString(travelTime == NO_TIME ? ERROR_153 : ERROR_217);
        sprintf(szarString, s, NodeAbbrFromRecID(fromNode));
        strcat(szarString, NodeAbbrFromRecID(toNode));
        MessageBeep(MB_ICONINFORMATION);
        MessageBox(szarString, TMS, MB_ICONINFORMATION | MB_OK);
      }
    }
  }
//
//  Ok, we made it this far, let's try the hook.
//
  int nRc = DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_COSTHOOKEDRUNAS),
        m_hWnd, (DLGPROC)COSTHOOKEDRUNASMsgProc, (LPARAM)PR.numPieces);
  if(!nRc)
  {
    return;
  }
//
//  Determine the indexes into the RELIEFPOINTS structure
//
  int runStart[MAXPIECES];
  int runEnd[MAXPIECES];

  bFound = FALSE;
  for(nI = 0; nI < PR.numPieces; nI++)
  {
    runStart[nI] = NO_RECORD;
    runEnd[nI] = NO_RECORD;
    for(bFound = FALSE, nJ = 0; nJ < m_numRELIEFPOINTS; nJ++)
    {
      if(m_pRELIEFPOINTS[nJ].NODESrecordID == PR.piece[nI].fromNODESrecordID &&
            m_pRELIEFPOINTS[nJ].TRIPSrecordID == PR.piece[nI].fromTRIPSrecordID &&
            runStart[nI] == NO_RECORD)
      { 
        runStart[nI] = nJ;
      }
      else if(m_pRELIEFPOINTS[nJ].NODESrecordID == PR.piece[nI].toNODESrecordID &&
            m_pRELIEFPOINTS[nJ].TRIPSrecordID == PR.piece[nI].toTRIPSrecordID &&
            runStart[nI] != NO_RECORD)
      {
        runEnd[nI] = nJ;
        bFound = TRUE;
        break;
      }
    }
    if(!bFound)
      break;
  }
  if(!bFound)
    return;
//
//  Determine the indexes into the RUNLISTDATA structure
//
  int runlistIndex[MAXPIECES];

  for(nI = 0; nI < numRuns; nI++)
  {
    runNumber = runNumbers.GetAt(nI);
    for(nJ = 0; nJ < MAXRUNSINRUNLIST; nJ++)
    {
      if(RUNLISTDATA[nJ].runNumber == runNumber)
      {
        runlistIndex[nI] = nJ;
        break;
      }
    }
  }
//
//  Modify the run and piece numbers in the RELIEFPOINTS structure.
//  The resulting run gets the lowest of the run numbers.
//
  int newRunNumber = runNumbers.GetAt(0);

  for(nI = 0; nI < PR.numPieces; nI++)
  {
    for(nJ = runStart[nI]; nJ <= runEnd[nI]; nJ++)
    {
      if(nJ != runEnd[nI])
      {
        m_pRELIEFPOINTS[nJ].start.recordID = NO_RECORD;
        m_pRELIEFPOINTS[nJ].start.runNumber = newRunNumber;
        m_pRELIEFPOINTS[nJ].start.pieceNumber = nI + 1;
      }
      if(nJ != runStart[nI])
      {
        m_pRELIEFPOINTS[nJ].end.recordID = NO_RECORD;
        m_pRELIEFPOINTS[nJ].end.runNumber = newRunNumber;
        m_pRELIEFPOINTS[nJ].end.pieceNumber = nI + 1;
      }
    }
  }
//
//  Modify the first run in, and delete the other runs from, RUNLISTDATA
//
  RUNLISTDATA[runlistIndex[0]].runNumber = newRunNumber;
  RUNLISTDATA[runlistIndex[0]].cutAsRuntype = hookedRunRuntype;
  for(nI = 1; nI < numRuns; nI++)
  {
    RUNLISTDATA[runlistIndex[nI]].runNumber = NO_RECORD;
    RUNLISTDATA[runlistIndex[nI]].cutAsRuntype = NO_RUNTYPE;
    RUNLISTDATA[runlistIndex[nI]].COMMENTSrecordID = NO_RECORD;
    RUNLISTDATA[runlistIndex[nI]].frozenFlag = FALSE;
  }
//
//  Update the displays
//
  AssembleRuns();  // Bring the internal lists up to date
  ShowRuns(m_pVRColors);
  m_pVRDlg->InvalidateRect(NULL, TRUE);
  SetRunAtTop(runNumbers.GetAt(0));
}

//
//  OnImprove() - Improve the runcut
//
void CCutRuns::OnImprove() 
{
  long savings;
  HCURSOR hSaveCursor = ::SetCursor(hCursorWait);
  int  improvements = RuncutImprove(&savings);

  if(improvements > 0)
    ShowRuns(m_pVRColors);
  ::SetCursor(hSaveCursor);
//  sprintf(tempString, "Found %ld improvements.\n\nSaved %ld:%02ld:%02ld pay time.",
//        improvements, savings / (60L * 60L), (savings / 60L) % 60L, savings % 60L);
  sprintf(tempString, "Found %ld improvements.\n\nSaved %s pay time.",
        improvements, chhmm(savings));
  MessageBox(tempString, "TMS Runcut Improve", MB_OK | MB_ICONINFORMATION );
}

void CCutRuns::OnProfile() 
{
  DialogBox(hInst, MAKEINTRESOURCE(IDD_PROFILE), m_hWnd, (DLGPROC)PROFILEMsgProc);
}

void CCutRuns::OnProperties() 
{
  BOOL bStop = FALSE;
  long runNumber;
  long previousRunNumber;
  int  nItem;
  int  nI;

  POSITION pos = pListCtrlLIST->GetFirstSelectedItemPosition();
  if(pos == NULL)
    return;
  
  bGenerateTravelInstructions = TRUE;
  previousRunNumber = NO_RECORD;
  while(pos)
  {
    nItem = pListCtrlLIST->GetNextSelectedItem(pos);
    runNumber = pListCtrlLIST->GetItemData(nItem);
    if(runNumber == NO_RECORD)
      continue;
    if(runNumber == previousRunNumber)
      continue;
    previousRunNumber = runNumber;
    for(nI = 0; nI < MAXRUNSINRUNLIST; nI++)
    {
      if(RUNLIST[nI].runNumber == runNumber)
      {
        CRunProperties dlg(this, runNumber);

        if(dlg.DoModal() != IDOK)
          bStop = TRUE;
        break;
      }
    }
    if(bStop)
      break;
  }
  bGenerateTravelInstructions = FALSE;
}

void CCutRuns::OnRenumber() 
{
  long runNumber;
  long previousRunNumber;
  int  nRc;
  int  nItem;
  
  POSITION pos = pListCtrlLIST->GetFirstSelectedItemPosition();
  
  if(pos == NULL)
    return;
  previousRunNumber = NO_RECORD;
  
  while(pos)
  {
    nItem = pListCtrlLIST->GetNextSelectedItem(pos);
    runNumber = pListCtrlLIST->GetItemData(nItem);
    if(runNumber == NO_RECORD)
      continue;
    if(runNumber == previousRunNumber)
      continue;
    previousRunNumber = runNumber;
//
//  Fire up the renumber dialog
//
//    specificRun = 0;
    specificRun = runNumber;
    nRc = DialogBox(hInst, MAKEINTRESOURCE(IDD_RENUMRUNS), m_hWnd, (DLGPROC)RENUMRUNSMsgProc);
    if(!nRc)
      break;
  }
  AssembleRuns();  // Bring the internal lists up to date
  ShowRuns(m_pVRColors);
  m_pVRDlg->InvalidateRect(NULL, TRUE);
}

void CCutRuns::OnSelectall() 
{
  int nI;
  int numRows;
  
  numRows = pListCtrlLIST->GetItemCount();

  for(nI = 0; nI < numRows; nI++)
  {
    pListCtrlLIST->SetItemState(nI, LVIS_SELECTED, LVIS_SELECTED);
  }
}

void CCutRuns::OnShift() 
{
  CUIntArray runNumbers;
  long runNumber;
  long previousRunNumber;
  BOOL bFound;
  int  nI, nJ;
  int  nItem;
  int  nRc;
  
  POSITION pos = pListCtrlLIST->GetFirstSelectedItemPosition();
  
  if(pos == NULL)
  {
    MessageBox("pos was NULL in OnShift", TMS, MB_OK);
    return;
  }

  previousRunNumber = NO_RECORD;
  runNumbers.RemoveAll();
//
//  Save a list of run numbers
//
  while(pos)
  {
    nItem = pListCtrlLIST->GetNextSelectedItem(pos);
    runNumber = pListCtrlLIST->GetItemData(nItem);
    if(runNumber == NO_RECORD)
      continue;
    if(runNumber == previousRunNumber)
      continue;
    previousRunNumber = runNumber;
    runNumbers.Add(runNumber);
  }

  int numRuns = runNumbers.GetSize();
  if(runNumbers.GetSize() == 0)
  {
    MessageBox("runNumbers.GetSize() returned 0", TMS, MB_OK);
    return;
  }
//
//  Loop through the runs
//
  bGenerateTravelInstructions = TRUE;
  for(nI = 0; nI < numRuns; nI++)
  {
    runNumber = runNumbers.GetAt(nI);
//
//  If it's frozen, skip it
//
    for(bFound = FALSE, nJ = 0; nJ < MAXRUNSINRUNLIST; nJ++)
    {
      if(RUNLISTDATA[nJ].runNumber == runNumber)
      {
        bFound = RUNLISTDATA[nJ].frozenFlag;
        break;
      }
    }
    if(bFound)
      continue;
//
//  Not frozen - continue.
//
    nRc = 1;
    for(nJ = 0; nJ < MAXRUNSINRUNLIST; nJ++)
    {
      if(RUNLIST[nJ].runNumber == runNumber)
      {
        nRc = DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_SHIFTRUN),
              m_hWnd, (DLGPROC)SHIFTRUNMsgProc, (LPARAM)&nJ);
        break;
      }
      if(!nRc)
        break;
    }
  }
  AssembleRuns();  // Bring the internal lists up to date
  ShowRuns(m_pVRColors);
  m_pVRDlg->InvalidateRect(NULL, TRUE);
  bGenerateTravelInstructions = FALSE;
  SetRunAtTop(runNumbers.GetAt(0));
}

void CCutRuns::OnSummary() 
{
  RunSummary();
}

void CCutRuns::OnSwap() 
{
  CUIntArray runNumbers;
  long runNumber;
  long previousRunNumber;
  int  nI, nJ;
  int  nItem;
  
  POSITION pos = pListCtrlLIST->GetFirstSelectedItemPosition();
  
  if(pos == NULL)
    return;

  previousRunNumber = NO_RECORD;
  runNumbers.RemoveAll();
//
//  Save a list of run numbers
//
  while(pos)
  {
    nItem = pListCtrlLIST->GetNextSelectedItem(pos);
    runNumber = pListCtrlLIST->GetItemData(nItem);
    if(runNumber == NO_RECORD)
      continue;
    if(runNumber == previousRunNumber)
      continue;
    previousRunNumber = runNumber;
    runNumbers.Add(runNumber);
  }

  int numRuns = runNumbers.GetSize();
  if(numRuns == 0)
    return;
  if(numRuns != 2)
  {
    TMSError(m_hWnd, MB_ICONSTOP, ERROR_239, (HANDLE)NULL);
    return;
  }
//
//  Get the two indexes into RUNLIST
//
  int runIndexes[2];

  for(nI = 0; nI < numRuns; nI++)
  {
    runNumber = runNumbers.GetAt(nI);
    for(nJ = 0; nJ < MAXRUNSINRUNLIST; nJ++)
    {
      if(RUNLIST[nJ].runNumber == runNumber)
      {
        runIndexes[nI] = nJ;
        break;
      }
    }
  }
//
//  Call the swap dialog
//
  if(DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_SWAPPIECES),
        m_hWnd, (DLGPROC)SWAPPIECESMsgProc, (LPARAM)&runIndexes))
  {
    AssembleRuns();  // Bring the internal lists up to date
    ShowRuns(m_pVRColors);
    m_pVRDlg->InvalidateRect(NULL, TRUE);
    bGenerateTravelInstructions = FALSE;
    SetRunAtTop(runNumbers.GetAt(0));
  }
}

void CCutRuns::OnUndorun() 
{
  CUIntArray runNumbers;
  long runNumber;
  long previousRunNumber;
  BOOL bFound;
  int  nI, nJ;
  int  nItem;
  
  POSITION pos = pListCtrlLIST->GetFirstSelectedItemPosition();
  
  if(pos == NULL)
    return;

  previousRunNumber = NO_RECORD;
  runNumbers.RemoveAll();
//
//  Save a list of run numbers
//
  while(pos)
  {
    nItem = pListCtrlLIST->GetNextSelectedItem(pos);
    runNumber = pListCtrlLIST->GetItemData(nItem);
    if(runNumber == NO_RECORD)
      continue;
    if(runNumber == previousRunNumber)
      continue;
    previousRunNumber = runNumber;
    runNumbers.Add(runNumber);
  }

  int numRuns = runNumbers.GetSize();
  if(runNumbers.GetSize() == 0)
    return;
//
//  Loop through the runs
//
  for(nI = 0; nI < numRuns; nI++)
  {
    runNumber = runNumbers.GetAt(nI);
//
//  If it's frozen, skip it
//
    for(bFound = FALSE, nJ = 0; nJ < MAXRUNSINRUNLIST; nJ++)
    {
      if(RUNLISTDATA[nJ].runNumber == runNumber)
      {
        bFound = RUNLISTDATA[nJ].frozenFlag;
        break;
      }
    }
    if(bFound)
      continue;
//
//  Not frozen - continue.
//
//  Zero out the run's data in the RELIEFPOINTS structure
//
    for(nJ = 0; nJ < m_numRELIEFPOINTS; nJ++)
    {
      if(m_pRELIEFPOINTS[nJ].start.runNumber == runNumber)
      {
        m_pRELIEFPOINTS[nJ].start.runNumber = NO_RECORD;
        m_pRELIEFPOINTS[nJ].start.pieceNumber = NO_RECORD;
        m_pRELIEFPOINTS[nJ].start.recordID = NO_RECORD;
      }
      if(m_pRELIEFPOINTS[nJ].end.runNumber == runNumber)
      {
        m_pRELIEFPOINTS[nJ].end.runNumber = NO_RECORD;
        m_pRELIEFPOINTS[nJ].end.pieceNumber = NO_RECORD;
        m_pRELIEFPOINTS[nJ].end.recordID = NO_RECORD;
      }
    }
//
//  And delete it fron RUNLISTDATA
//
    for(nJ = 0; nJ < MAXRUNSINRUNLIST; nJ++)
    {
      if(RUNLISTDATA[nJ].runNumber == runNumber)
      {
        RUNLISTDATA[nJ].runNumber = NO_RECORD;
        RUNLISTDATA[nJ].cutAsRuntype = NO_RUNTYPE;
        RUNLISTDATA[nJ].COMMENTSrecordID = NO_RECORD;
        RUNLISTDATA[nJ].frozenFlag = FALSE;
        break;
      }
    }
  }
//
//  And update the displays
//
  AssembleRuns();  // Bring the internal lists up to date
  ShowRuns(m_pVRColors);
  m_pVRDlg->InvalidateRect(NULL, TRUE);
  SetRunAtTop(runNumbers.GetAt(0));
}

void CCutRuns::OnUnhook() 
{
  CUIntArray runNumbers;
  long runNumber;
  long previousRunNumber;
  BOOL bFound;
  int  nI, nJ;
  int  nItem;
  int  maxPieceNumber;
  
  POSITION pos = pListCtrlLIST->GetFirstSelectedItemPosition();
  
  if(pos == NULL)
    return;

  previousRunNumber = NO_RECORD;
  runNumbers.RemoveAll();
//
//  Save a list of run numbers
//
  while(pos)
  {
    nItem = pListCtrlLIST->GetNextSelectedItem(pos);
    runNumber = pListCtrlLIST->GetItemData(nItem);
    if(runNumber == NO_RECORD)
      continue;
    if(runNumber == previousRunNumber)
      continue;
    previousRunNumber = runNumber;
    runNumbers.Add(runNumber);
  }

  int numRuns = runNumbers.GetSize();
  if(runNumbers.GetSize() == 0)
    return;
//
//  Loop through the runs
//
  for(nI = 0; nI < numRuns; nI++)
  {
    runNumber = runNumbers.GetAt(nI);
//
//  If it's frozen, skip it
//
    for(bFound = FALSE, nJ = 0; nJ < MAXRUNSINRUNLIST; nJ++)
    {
      if(RUNLISTDATA[nJ].runNumber == runNumber)
      {
        bFound = RUNLISTDATA[nJ].frozenFlag;
        break;
      }
    }
    if(bFound)
      continue;
//
//  Not frozen - continue.
//  Modify the run's data in the RELIEFPOINTS structure
//
    maxPieceNumber = 0;
    for(nJ = 0; nJ < m_numRELIEFPOINTS; nJ++)
    {
      if(m_pRELIEFPOINTS[nJ].start.runNumber == runNumber)
      {
        m_pRELIEFPOINTS[nJ].start.runNumber = m_GlobalRunNumber + m_pRELIEFPOINTS[nJ].start.pieceNumber - 1;
        if(m_pRELIEFPOINTS[nJ].start.pieceNumber > maxPieceNumber)
        {
          maxPieceNumber = m_pRELIEFPOINTS[nJ].start.pieceNumber;
        }
        m_pRELIEFPOINTS[nJ].start.pieceNumber = 1;
      }
      if(m_pRELIEFPOINTS[nJ].end.runNumber == runNumber)
      {
        m_pRELIEFPOINTS[nJ].end.runNumber = m_GlobalRunNumber + m_pRELIEFPOINTS[nJ].end.pieceNumber - 1;
        if(m_pRELIEFPOINTS[nJ].end.pieceNumber > maxPieceNumber)
        {
          maxPieceNumber = m_pRELIEFPOINTS[nJ].end.pieceNumber;
        }
        m_pRELIEFPOINTS[nJ].end.pieceNumber = 1;
      }
    }
    m_GlobalRunNumber += maxPieceNumber;
  }
//
//  And update the displays
//
  AssembleRuns();  // Bring the internal lists up to date
  ShowRuns(m_pVRColors);
  m_pVRDlg->InvalidateRect(NULL, TRUE);
}

void CCutRuns::SetRunAtTop(long runNumber)
{
  int numEntries = pListCtrlLIST->GetItemCount();
  int nBottom; 
  int nI;

  for(nI = 0; nI < numEntries; nI++)
  {
    if((long)pListCtrlLIST->GetItemData(nI) == runNumber)
    {
      nBottom = pListCtrlLIST->GetCountPerPage() + nI;
      pListCtrlLIST->EnsureVisible(nBottom, TRUE);
      pListCtrlLIST->EnsureVisible(nI, FALSE);
      break;
    }
  }
}

void CCutRuns::OnRclickList(NMHDR* pNMHDR, LRESULT* pResult) 
{
  if(m_ContextMenu != NULL)
    return;
 
  m_ContextMenu.LoadMenu(IDR_VRR);
  CMenu* pMenu = m_ContextMenu.GetSubMenu(0);
//
//  Get the current mouse location and convert it to client coordinates.
//
  DWORD pos = GetMessagePos();
  CPoint pt(LOWORD(pos), HIWORD(pos));

  pMenu->TrackPopupMenu(TPM_LEFTALIGN, pt.x, pt.y, FromHandle(m_hWnd), NULL);
  m_ContextMenu.DestroyMenu();

	*pResult = 0;
}

//
//  OnNotify() - Munged to intercept a right-click on the header
//
BOOL CCutRuns::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult) 
{
	HD_NOTIFY	*pHDN = (HD_NOTIFY*)lParam;

	LPNMHDR pNH = (LPNMHDR) lParam; 
//
//  wParam is zero for Header ctrl
//
	if(wParam == 0 && pNH->code == NM_RCLICK)
	{
    if(m_ColumnMenu == NULL)
    {
      m_ColumnMenu.LoadMenu(IDR_VRRC);
      CMenu* pMenu = m_ColumnMenu.GetSubMenu(0);
//
//  Get the current mouse location and convert it to client coordinates.
//
      DWORD pos = GetMessagePos();
      CPoint pt(LOWORD(pos), HIWORD(pos));

      pMenu->TrackPopupMenu(TPM_LEFTALIGN, pt.x, pt.y, FromHandle(m_hWnd), NULL);
      m_ColumnMenu.DestroyMenu();
    }
	}
	
	return CDialog::OnNotify(wParam, lParam, pResult);
}


void CCutRuns::OnDblclkList(NMHDR* pNMHDR, LRESULT* pResult) 
{
  OnProperties();
  	
	*pResult = 0;
}

void CCutRuns::OnReassignruntype() 
{
  CUIntArray runNumbers;
  long runNumber;
  long previousRunNumber;
  BOOL bFound;
  BOOL bDidOne;
  int  nI, nJ;
  int  nItem;
  
  POSITION pos = pListCtrlLIST->GetFirstSelectedItemPosition();
  
  if(pos == NULL)
  {
    return;
  }

  previousRunNumber = NO_RECORD;
  runNumbers.RemoveAll();
//
//  Save a list of run numbers
//
  while(pos)
  {
    nItem = pListCtrlLIST->GetNextSelectedItem(pos);
    runNumber = pListCtrlLIST->GetItemData(nItem);
    if(runNumber == NO_RECORD)
    {
      continue;
    }
    if(runNumber == previousRunNumber)
    {
      continue;
    }
    previousRunNumber = runNumber;
    runNumbers.Add(runNumber);
  }

  int numRuns = runNumbers.GetSize();
  if(numRuns == 0)
  {
    return;
  }
//
//  Loop through the runs
//
  bDidOne = FALSE;
  for(nI = 0; nI < numRuns; nI++)
  {
    runNumber = runNumbers.GetAt(nI);
//
//  If it's frozen, skip it
//
    for(bFound = FALSE, nJ = 0; nJ < MAXRUNSINRUNLIST; nJ++)
    {
      if(RUNLISTDATA[nJ].runNumber == runNumber)
      {
        bFound = RUNLISTDATA[nJ].frozenFlag;
        break;
      }
    }
    if(bFound)
    {
      continue;
    }
//
//  Not frozen - continue.
//  
    CReassignRuntype dlg(this, runNumber);

    if(dlg.DoModal() != IDOK)
    {
      break;
    }
    bDidOne = TRUE;
  }
//
//  Any changes?
//
  if(bDidOne)
  {
    AssembleRuns();  // Bring the internal lists up to date
    ShowRuns(m_pVRColors);
    m_pVRDlg->InvalidateRect(NULL, TRUE);
    bGenerateTravelInstructions = FALSE;
    SetRunAtTop(runNumbers.GetAt(0));
  }

}

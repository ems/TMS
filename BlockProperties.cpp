//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
// BlockProperties.cpp : implementation file
//

#include "stdafx.h"
extern "C"
{
#include "TMSHeader.h"
}
#include <math.h>

#include "ResizingDialog.h"
#include "FlatSB.h"
#include "CutRuns.h"
#include "BlockProperties.h"
#include "VisualRuncutter.h"
#include "ColorBtn.h"
#include "VRColors.h"
#include "ManualCut.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define NUMTITLES 5

extern void EstablishVRBitmaps(CImageList *, VRCOLORSDef *);
extern void EstablishBPBitmaps(CImageList *, VRCOLORSDef *);
extern int sort_RUNLIST(const void *, const void *);  // Defined in cutruns.cpp


/////////////////////////////////////////////////////////////////////////////
// CBlockProperties dialog


CBlockProperties::CBlockProperties(CWnd* pParent, long blockNumber, CCutRuns *pCRDlg)
	: CResizingDialog(CBlockProperties::IDD, pParent)
{
  m_blockNumber = blockNumber;
  m_pCRDlg = pCRDlg;
  m_pVRDlg = pParent;

  m_pImageList = new CImageList;

	//{{AFX_DATA_INIT(CBlockProperties)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
//
//  Hold the dialog controls in place during resize
//
  CDWordArray c_info;
  SetControlInfo(IDOK,         ANCHORE_BOTTOM);
  SetControlInfo(IDCANCEL,     ANCHORE_BOTTOM);
  SetControlInfo(IDHELP,       ANCHORE_BOTTOM);
  SetControlInfo(BLOCKPROPERTIES_LIST, ANCHORE_LEFT | RESIZE_HOR);
  SetControlInfo(BLOCKPROPERTIES_TRIPINFO, ANCHORE_LEFT | RESIZE_HOR);
  SetControlInfo(BLOCKPROPERTIES_TRIPDATA, ANCHORE_LEFT | RESIZE_HOR);
}


void CBlockProperties::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CBlockProperties)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CBlockProperties, CDialog)
	//{{AFX_MSG_MAP(CBlockProperties)
	ON_BN_CLICKED(IDHELP, OnHelp)
	ON_WM_CLOSE()
	ON_NOTIFY(NM_CLICK, BLOCKPROPERTIES_LIST, OnClickList)
	ON_NOTIFY(NM_RCLICK, BLOCKPROPERTIES_LIST, OnRclickList)
	ON_WM_PAINT()
	ON_WM_SIZE()
	ON_WM_CREATE()
	ON_WM_GETMINMAXINFO()
	ON_BN_CLICKED(IDCLOSE, OnCloseButton)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBlockProperties message handlers


BOOL CBlockProperties::OnInitDialog() 
{
	CResizingDialog::OnInitDialog();

  sprintf(tempString, "Block %ld Properties", m_blockNumber);
  SetWindowText(tempString);
	
  pListCtrlLIST = (CListCtrl *)GetDlgItem(BLOCKPROPERTIES_LIST);
  pStaticTRIPINFO = (CStatic *)GetDlgItem(BLOCKPROPERTIES_TRIPINFO);
  pListCtrlTRIPDATA = (CListCtrl *)GetDlgItem(BLOCKPROPERTIES_TRIPDATA);

  DWORD dwExStyles = pListCtrlLIST->GetExtendedStyle();
  pListCtrlLIST->SetExtendedStyle(dwExStyles | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_INFOTIP);
  dwExStyles = pListCtrlTRIPDATA->GetExtendedStyle();
  pListCtrlTRIPDATA->SetExtendedStyle(dwExStyles | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_INFOTIP);
//
//  Set up the LIST list control
//
//  Columns
//

  char *Titles[] = {"Status", "Run", "Relief At",  "Relief Time", "Block Time"};
  int  nI;
//
//  Set up the list control
//
  LVCOLUMN LVC;
  LVC.mask = LVCF_FMT | LVCF_TEXT | LVCF_WIDTH;
  LVC.fmt = LVCFMT_LEFT;
  for(nI = 0; nI < NUMTITLES; nI++)
  {
    LVC.cx = nI == 0 ? 50 : 70;
    LVC.pszText = Titles[nI];
    pListCtrlLIST->InsertColumn(nI, &LVC);
  }

  if(!ShowReliefPoints())
    OnCancel();

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}


void CBlockProperties::OnCloseButton() 
{
  delete m_pImageList;
  DestroyWindow();  
}

void CBlockProperties::OnHelp() 
{

}

void CBlockProperties::OnClose() 
{
  delete m_pImageList;
  DestroyWindow();  
}

void CBlockProperties::PostNcDestroy() 
{
//  delete this;
}

BOOL CBlockProperties::ShowReliefPoints()
{
//
//  Can anything that was there (we could be faking a repaint)
//
  pListCtrlLIST->DeleteAllItems();
//
//  Establish the colors for the image list
//
  if(m_pImageList->GetSafeHandle() != NULL)
  {
    if(m_pImageList->GetImageCount() != 0)
      m_pImageList->DeleteImageList();
  }
  m_pImageList->Create(16, 15, ILC_COLORDDB, 0, 24);

  VRCOLORSDef VRC;

  EstablishBPBitmaps(m_pImageList, &VRC);
  
  pListCtrlLIST->SetImageList(m_pImageList, LVSIL_SMALL);
//
//  Set up run details
//
  int nI;
  int numInternalRuns = AssembleRuns();
  
  qsort((void *)RUNLIST, numInternalRuns, sizeof(RUNLISTDef), sort_RUNLIST);

//
//  Items in columns
//
  LVITEM LVI;
  BOOL   bFound;
//
//  Find the block in RELIEFPOINTS
//
  for(bFound = FALSE, nI = 0; nI < m_numRELIEFPOINTS; nI++)
  {
    if(m_pRELIEFPOINTS[nI].blockNumber == m_blockNumber)
    {
      bFound = TRUE;
      break;
    }
  }
  if(!bFound)
  {
    TMSError(m_hWnd, MB_ICONSTOP, ERROR_283, (HANDLE)NULL);
    return FALSE;
  }
//
//  Got the block - spit it out
//
  COSTDef COST;
  BOOL bFrozen;
  long tempLong;
  int  pos = 0;
  int  index = nI;
  int  RLDIndex;
  int  leftImage, rightImage;

  long previousRunNumber = NO_RECORD;
  long firstTimeInBlock = m_pRELIEFPOINTS[index].time;

  leftImage = BITMAP_BLANK;
  rightImage = BITMAP_BLANK;
  bFrozen = FALSE;

  while(m_pRELIEFPOINTS[index].blockNumber == m_blockNumber)
  {
    if(index == m_numRELIEFPOINTS - 1 || m_pRELIEFPOINTS[index + 1].blockNumber != m_blockNumber)
      rightImage = BITMAP_BLANK;
    else
    {
      if(previousRunNumber != m_pRELIEFPOINTS[index].start.runNumber)
      {
        previousRunNumber = m_pRELIEFPOINTS[index].start.runNumber;
//
//  Find the run and cost it
//
        bFound = FALSE;
        if(previousRunNumber != NO_RECORD)
        {
          for(bFound = FALSE, nI = 0; nI < numInternalRuns; nI++)
          {
            if(RUNLIST[nI].runNumber == m_pRELIEFPOINTS[index].start.runNumber)
            {
              bFound = TRUE;
              break;
            }
          }
        }
//
//  Cost the run
//
        bFrozen = FALSE;
        if(bFound)
        {
          for(bFound = FALSE, RLDIndex = 0; RLDIndex < MAXRUNSINRUNLIST; RLDIndex++)
          {
            if(RUNLIST[nI].runNumber == RUNLISTDATA[RLDIndex].runNumber)
            {
              bFound = TRUE;
              bFrozen = RUNLISTDATA[RLDIndex].frozenFlag;
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
            for(bFound = FALSE, RLDIndex = 0; RLDIndex < MAXRUNSINRUNLIST; RLDIndex++)
            {
              if(RUNLISTDATA[RLDIndex].runNumber == NO_RECORD)
              {
                RUNLISTDATA[RLDIndex].runNumber = RUNLIST[nI].runNumber;
                RUNLISTDATA[RLDIndex].cutAsRuntype = tempLong;
                break;
              }
            }
          }
        }
      }
//
//  Determine the right side image
//
      if(bFrozen)
        rightImage = BITMAP_FROZENRUN;
      else
      {
        if(previousRunNumber == NO_RECORD)
          rightImage = BITMAP_UNCUT;
        else
        {
          if(COST.TOTAL.payTime == 0)
            rightImage = BITMAP_ILLEGALRUN;
          else
            rightImage = BITMAP_LEGALRUN;
        }
      }
    }

//
//  Continue with the ListCtrl
//
    LVI.iItem = pos;
    LVI.lParam = index;
//
//  Status
//
//  Determine the left/right sides
//
    LVI.mask = LVIF_IMAGE | LVIF_PARAM;
    LVI.iSubItem = 0;
    LVI.iImage = leftImage * 5 + rightImage;
    LVI.iItem = pListCtrlLIST->InsertItem(&LVI);
//
//  Run number
//
    if(m_pRELIEFPOINTS[index].start.runNumber == m_pRELIEFPOINTS[index].end.runNumber)
    {
      if(m_pRELIEFPOINTS[index].end.runNumber == NO_RECORD)
        strcpy(tempString, "-");
      else if(m_pRELIEFPOINTS[index].end.runNumber == SECURELOCATION_RUNNUMBER)
        strcpy(tempString, "SL");
      else
        sprintf(tempString, "%ld", m_pRELIEFPOINTS[index].end.runNumber);
    }
    else
    {
      if(m_pRELIEFPOINTS[index].end.runNumber == NO_RECORD)
        strcpy(tempString, "-/");
      else if(m_pRELIEFPOINTS[index].end.runNumber == SECURELOCATION_RUNNUMBER)
        strcpy(tempString, "SL/");
      else
        sprintf(tempString, "%ld/", m_pRELIEFPOINTS[index].end.runNumber);
      if(m_pRELIEFPOINTS[index].start.runNumber == NO_RECORD)
        strcat(tempString, "-");
      else if(m_pRELIEFPOINTS[index].start.runNumber == SECURELOCATION_RUNNUMBER)
        strcat(tempString, "SL");
      else
      {
        sprintf(szarString, "%ld", m_pRELIEFPOINTS[index].start.runNumber);
        strcat(tempString, szarString);
      }
    }
    LVI.mask = LVIF_TEXT;
    LVI.iSubItem = 1;
    LVI.pszText = tempString;
    pListCtrlLIST->SetItem(&LVI);
//
//  Relief point name
//
    LVI.mask = LVIF_TEXT;
    LVI.iSubItem = 2;
    LVI.pszText = m_pRELIEFPOINTS[index].nodeName;
    pListCtrlLIST->SetItem(&LVI);
//
//  Relief point time
//
    sprintf(tempString, "%s", Tchar(m_pRELIEFPOINTS[index].time));
    LVI.mask = LVIF_TEXT;
    LVI.iSubItem = 3;
    LVI.pszText = tempString;
    pListCtrlLIST->SetItem(&LVI);
//
//  Time in block
//
    sprintf(tempString, "%s", chhmm(m_pRELIEFPOINTS[index].time - firstTimeInBlock));
    LVI.mask = LVIF_TEXT;
    LVI.iSubItem = 4;
    LVI.pszText = tempString;
    pListCtrlLIST->SetItem(&LVI);
//
//  Loop back
//
    index++;
    pos++;
    leftImage = rightImage;
  }

  return(TRUE);
}

void CBlockProperties::OnClickList(NMHDR* pNMHDR, LRESULT* pResult) 
{
  float distance;
  char nodeName[NODES_ABBRNAME_LENGTH + 1];
  long deadheadTime;
  int  rcode2;
  int  pos;

  POSITION position = pListCtrlLIST->GetFirstSelectedItemPosition();
//
//  Determine the selection(s)
//
  if(position == NULL)
  {
    return;
  }
//
//  Display the trip
//
  LVCOLUMN LVC;
  int nItem = pListCtrlLIST->GetNextSelectedItem(position);
  int nI = pListCtrlLIST->GetItemData(nItem);
//
//  Get the trip
//
  TRIPSKey0.recordID = m_pRELIEFPOINTS[nI].TRIPSrecordID;
  btrieve(B_GETEQUAL, TMS_TRIPS, &TRIPS, &TRIPSKey0, 0);
//
//  Show the route, service, direction, and pattern
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

  sprintf(tempString, "Trip info: %s %s %s on pattern %s",
        serviceName, directionName, routeNumberAndName, patternName);
  pStaticTRIPINFO->SetWindowText(tempString);

//
//  Clear out the previous ListCtrl
//
  pListCtrlTRIPDATA->DeleteAllItems();

  CHeaderCtrl* pCtrl = pListCtrlTRIPDATA->GetHeaderCtrl();

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
  GetConnectionTimeDef GCTData;

  int numNodes = GenerateTrip(TRIPS.ROUTESrecordID, TRIPS.SERVICESrecordID,
        TRIPS.directionIndex, TRIPS.PATTERNNAMESrecordID,
        TRIPS.timeAtMLP, GENERATETRIP_FLAG_DISPLAYERRORS, &GTResults);

  pos = 0;
//
//  Is this a pullout?
//
  long POTime = NO_TIME;
  
  if(TRIPS.standard.POGNODESrecordID != NO_RECORD)
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
    deadheadTime = GetConnectionTime(GCT_FLAG_DEADHEADTIME, &GCTData, &distance);
    distance = (float)fabs((double)distance);
    if(deadheadTime != NO_TIME)
    {
      POTime = GTResults.firstNodeTime - deadheadTime;
      NODESKey0.recordID = TRIPS.standard.POGNODESrecordID;
      btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
      strncpy(nodeName, NODES.abbrName, NODES_ABBRNAME_LENGTH);
      trim(nodeName, NODES_ABBRNAME_LENGTH);
      LVC.mask = LVCF_FMT | LVCF_TEXT | LVCF_WIDTH;
      LVC.fmt = LVCFMT_LEFT;
      LVC.cx = 50;
      LVC.pszText = nodeName;
      pListCtrlTRIPDATA->InsertColumn(pos, &LVC);
      pos++;
    }
  }
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
      pListCtrlTRIPDATA->InsertColumn(pos, &LVC);
      pos++;
    }
    rcode2 = btrieve(B_GETNEXT, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
  }
//  
//  Is this a pull-in?
//
  long PITime = NO_TIME;
  if(TRIPS.standard.PIGNODESrecordID != NO_RECORD)
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
    deadheadTime = GetConnectionTime(GCT_FLAG_DEADHEADTIME, &GCTData, &distance);
    distance = (float)fabs((double)distance);
    if(deadheadTime != NO_TIME)
    {
      PITime = GTResults.lastNodeTime + deadheadTime;
      NODESKey0.recordID = TRIPS.standard.PIGNODESrecordID;
      btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
      strncpy(nodeName, NODES.abbrName, NODES_ABBRNAME_LENGTH);
      trim(nodeName, NODES_ABBRNAME_LENGTH);
      LVC.mask = LVCF_FMT | LVCF_TEXT | LVCF_WIDTH;
      LVC.fmt = LVCFMT_LEFT;
      LVC.cx = 50;
      LVC.pszText = nodeName;
      pListCtrlTRIPDATA->InsertColumn(pos, &LVC);
    }
  }
//
//  Display the times
//
  LVITEM LVI;

  LVI.iItem = 0;
  LVI.mask = LVIF_TEXT;
  pos = 0;
//
//  POT?
//
  if(POTime != NO_TIME)
  {
    LVI.iSubItem = pos;
    strcpy(tempString, Tchar(POTime));
    LVI.pszText = tempString;
    pListCtrlTRIPDATA->InsertItem(&LVI);
    pos++;
  }
//
//  Do the trip times
//
  strcpy(tempString, Tchar(GTResults.tripTimes[0]));
  LVI.pszText = tempString;
  LVI.iSubItem = pos;
  if(pos == 0)
    pListCtrlTRIPDATA->InsertItem(&LVI);
  else
  {
    pListCtrlTRIPDATA->SetItem(&LVI);
  }
  pos++;
  for(nI = 1; nI < numNodes; nI++)
  {  
    LVI.iSubItem = pos;
    strcpy(tempString, Tchar(GTResults.tripTimes[nI]));
    LVI.pszText = tempString;
    pListCtrlTRIPDATA->SetItem(&LVI);
    pos++;
  }
//
//  PIT?
//
  if(PITime != NO_TIME)
  {
    LVI.iSubItem = pos;
    strcpy(tempString, Tchar(PITime));
    LVI.pszText = tempString;
    pListCtrlTRIPDATA->SetItem(&LVI);
  }

	*pResult = 0;
}

//
//  OnRclickList() - Cut a run manually
//
void CBlockProperties::OnRclickList(NMHDR* pNMHDR, LRESULT* pResult) 
{
  POSITION pos = pListCtrlLIST->GetFirstSelectedItemPosition();
  BOOL bOK;
  int  nItem;
  int  nItemCount;
  int  firstIndex;
  int  lastIndex;
  int  nI;
//
//  Determine the selection(s)
//
  if(pos != NULL)
  {
    nItemCount = 1;
    nItem = pListCtrlLIST->GetNextSelectedItem(pos);
    firstIndex = pListCtrlLIST->GetItemData(nItem);
    while(pos)
    {
      nItem = pListCtrlLIST->GetNextSelectedItem(pos);
      nItemCount++;
      lastIndex = pListCtrlLIST->GetItemData(nItem); 
    }
//
//  If just one selection, force two
//
    if(nItemCount <= 2)
      lastIndex = firstIndex;
//
//  Make sure we're not encroaching on an existing run
//
    for(bOK = TRUE, nI = firstIndex; nI <= lastIndex; nI++)
    {
      if(nI == firstIndex)
      {
        if(IsFromReliefPointMatched(m_pRELIEFPOINTS[nI]))
          bOK = FALSE;
      }
      else if(nI == lastIndex)
      {
        if(IsToReliefPointMatched(m_pRELIEFPOINTS[nI]))
          bOK = FALSE;
      }
      else
      {
        if(IsReliefPointInRun(m_pRELIEFPOINTS[nI]))
          bOK = FALSE;
      }
      if(!bOK)
      {
        TMSError(m_hWnd, MB_ICONSTOP, ERROR_285, (HANDLE)NULL);
        break;
      }
    }
//
//  Ok - let's put this together and see what we've got
//
    if(bOK)
    {
      CManualCut dlg(this, firstIndex, lastIndex);

      if(dlg.DoModal() == IDOK)
      {
        AssembleRuns();
        ShowReliefPoints();
        m_pVRDlg->InvalidateRect(NULL, FALSE);
        
        VRCOLORSDef VRC;
        COLORREF cRed = RGB(255, 0, 0);
        COLORREF cGreen = RGB(0, 255, 0);
        COLORREF cBlue = RGB(0, 0, 255);
        COLORREF cYellow = RGB(255, 255, 0);
   
        VRC.illegal = GetPrivateProfileInt(userName, "VRColorIllegal", cRed, TMSINIFile);
        VRC.cut = GetPrivateProfileInt(userName, "VRColorCut", cGreen, TMSINIFile);
        VRC.frozen = GetPrivateProfileInt(userName, "VRColorFrozen", cBlue, TMSINIFile);
        VRC.uncut = GetPrivateProfileInt(userName, "VRColorUncut", cYellow, TMSINIFile);
        m_pCRDlg->ShowRuns(&VRC);
      }
    }
  }

	*pResult = 0;
}

void CBlockProperties::OnPaint() 
{
  CResizingDialog::OnPaint();

	CPaintDC dc(this); // device context for painting
	
	// TODO: Add your message handler code here
	
	// Do not call CDialog::OnPaint() for painting messages
}

void CBlockProperties::OnSize(UINT nType, int cx, int cy) 
{
  CResizingDialog::OnSize(nType, cx, cy);
	
	
	// TODO: Add your message handler code here
	
}


int CBlockProperties::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CResizingDialog::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	// TODO: Add your specialized creation code here
	
	return 0;
}

//
//  OnGetMinMaxInfo() - Subclass for the resizing class
//

void CBlockProperties::OnGetMinMaxInfo(MINMAXINFO FAR* lpMMI) 
{
	// TODO: Add your message handler code here and/or call default
	
	CResizingDialog::OnGetMinMaxInfo(lpMMI);
}

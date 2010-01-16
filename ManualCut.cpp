//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
// ManualCut.cpp : implementation file
//

#include "stdafx.h"
extern "C"
{
#include "TMSHeader.h"
}

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

/////////////////////////////////////////////////////////////////////////////
// CManualCut dialog


CManualCut::CManualCut(CWnd* pParent /*=NULL*/, int firstIndex, int lastIndex)
	: CDialog(CManualCut::IDD, pParent)
{
  m_firstIndex = firstIndex;
  m_lastIndex = lastIndex;
  m_CutAsRuntype = NO_RECORD;
  m_PreviousStartPosition = NO_RECORD;
  m_PreviousEndPosition = NO_RECORD;

	//{{AFX_DATA_INIT(CManualCut)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CManualCut::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CManualCut)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CManualCut, CDialog)
	//{{AFX_MSG_MAP(CManualCut)
	ON_CBN_SELCHANGE(MANUALCUT_RUNTYPE, OnSelchangeRuntype)
	ON_BN_CLICKED(IDHELP, OnHelp)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDSHOWDIRECTIONS, OnShowdirections)
	ON_BN_CLICKED(IDWHY, OnWhy)
	ON_CBN_SELCHANGE(MANUALCUT_STARTRELIEF, OnSelchangeStartrelief)
	ON_CBN_SELCHANGE(MANUALCUT_ENDRELIEF, OnSelchangeEndrelief)
	ON_NOTIFY(NM_CLICK, MANUALCUT_POTENTIALRELIEFS, OnClickPotentialreliefs)
	ON_NOTIFY(NM_RCLICK, MANUALCUT_POTENTIALRELIEFS, OnRclickPotentialreliefs)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CManualCut message handlers

BOOL CManualCut::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
  pListCtrlPOTENTIALRELIEFS = (CListCtrl *)GetDlgItem(MANUALCUT_POTENTIALRELIEFS);
  pComboBoxSTARTRELIEF = (CComboBox *)GetDlgItem(MANUALCUT_STARTRELIEF);
  pComboBoxENDRELIEF = (CComboBox *)GetDlgItem(MANUALCUT_ENDRELIEF);
  pComboBoxRUNTYPE = (CComboBox *)GetDlgItem(MANUALCUT_RUNTYPE);
  pStaticREPORTAT = (CStatic *)GetDlgItem(MANUALCUT_REPORTAT);
  pStaticONAT = (CStatic *)GetDlgItem(MANUALCUT_ONAT);
  pStaticONLOC = (CStatic *)GetDlgItem(MANUALCUT_ONLOC);
  pStaticOFFAT = (CStatic *)GetDlgItem(MANUALCUT_OFFAT);
  pStaticOFFLOC = (CStatic *)GetDlgItem(MANUALCUT_OFFLOC);
  pStaticPLATFORMTIME = (CStatic *)GetDlgItem(MANUALCUT_PLATFORMTIME);
  pStaticREPORTTIME = (CStatic *)GetDlgItem(MANUALCUT_REPORTTIME);
  pStaticSTARTTRAVEL = (CStatic *)GetDlgItem(MANUALCUT_STARTTRAVEL);
  pStaticENDTRAVEL = (CStatic *)GetDlgItem(MANUALCUT_ENDTRAVEL);
  pStaticTOTALTRAVEL = (CStatic *)GetDlgItem(MANUALCUT_TOTALTRAVEL);
  pStaticTURNINTIME = (CStatic *)GetDlgItem(MANUALCUT_TURNINTIME);
  pStaticMAKEUPTIME = (CStatic *)GetDlgItem(MANUALCUT_MAKEUPTIME);
  pStaticOVERTIME = (CStatic *)GetDlgItem(MANUALCUT_OVERTIME);
  pStaticSPREADTIME = (CStatic *)GetDlgItem(MANUALCUT_SPREADTIME);
  pStaticPAYTIME = (CStatic *)GetDlgItem(MANUALCUT_PAYTIME);
  pStaticPAYTOPLATFORM = (CStatic *)GetDlgItem(MANUALCUT_PAYTOPLATFORM);
  pStaticPLATFORMTOPAY = (CStatic *)GetDlgItem(MANUALCUT_PLATFORMTOPAY);
  pButtonTRAVELINSTRUCTIONS = (CButton *)GetDlgItem(IDSHOWDIRECTIONS);
  pButtonWHY = (CButton *)GetDlgItem(IDWHY);
//
//  Set the styles for the list control
//
  DWORD dwExStyles = pListCtrlPOTENTIALRELIEFS->GetExtendedStyle();
  pListCtrlPOTENTIALRELIEFS->SetExtendedStyle(dwExStyles | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_INFOTIP);
//
//  Set up the headers on the list control
//
  LVCOLUMN LVC;

  LVC.mask = LVCF_FMT | LVCF_TEXT | LVCF_WIDTH;
  LVC.fmt = LVCFMT_CENTER;
  LVC.cx = 44;
  LVC.pszText = "Rlf";
  pListCtrlPOTENTIALRELIEFS->InsertColumn(0, &LVC);

  LVC.cx = 45;
  LVC.pszText = "At";
  pListCtrlPOTENTIALRELIEFS->InsertColumn(1, &LVC);

//
//  See if we can back it up to the beginning of the uncut work
//
  long blockNumber = m_pRELIEFPOINTS[m_firstIndex].blockNumber;
  int  firstDisplayed = m_firstIndex;
  int  nI, nJ, nK;

  for(nI = m_firstIndex - 1; nI >= 0; nI--)
  {
    if(m_pRELIEFPOINTS[nI].start.runNumber != NO_RECORD ||
          m_pRELIEFPOINTS[nI].blockNumber != blockNumber)
    {
      firstDisplayed = nI + 1;
      break;
    }
  }
//
//  See if we can go forward to the end of the encut work
//
  BOOL bFound;
  int  lastDisplayed = m_lastIndex;

  for(bFound = FALSE, nI = m_lastIndex + 1; nI < m_numRELIEFPOINTS; nI++)
  {
    if(m_pRELIEFPOINTS[nI].end.runNumber != NO_RECORD ||
          m_pRELIEFPOINTS[nI].blockNumber != blockNumber)
    {
      lastDisplayed = nI - 1;
      bFound = TRUE;
      break;
    }
  }
  if(!bFound)
  {
    lastDisplayed = m_numRELIEFPOINTS - 1;
  }
//
//  If firstIndex and lastIndex came in the same, it
//  means that a single click got us here.  If this
//  was the case, assume the entire range.
//
  if(m_firstIndex == m_lastIndex)
  {
    m_firstIndex = firstDisplayed;
    m_lastIndex = lastDisplayed;
  }
//
//  Ok - let's see what we've got
//
//  Set up the potential reliefs list box, as 
//  well as the start and end relief dropdowns
//
  LVITEM  LVI;

  for(nJ = 0, nI = firstDisplayed; nI <= lastDisplayed; nI++)
  {  
//
//  List control
//
//  Location
//
    strcpy(tempString, m_pRELIEFPOINTS[nI].nodeName);
    LVI.mask = LVIF_TEXT | LVIF_PARAM;
    LVI.iItem = nJ;
    LVI.iSubItem = 0;
    LVI.pszText = tempString;
    LVI.lParam = nI;
    LVI.iItem = pListCtrlPOTENTIALRELIEFS->InsertItem(&LVI);
//
//  At
//
    strcpy(tempString, Tchar(m_pRELIEFPOINTS[nI].time));
    LVI.mask = LVIF_TEXT;
    LVI.iItem = nJ;
    LVI.iSubItem = 1;
    LVI.pszText = tempString;
    LVI.iItem = pListCtrlPOTENTIALRELIEFS->SetItem(&LVI);

    strcpy(tempString, m_pRELIEFPOINTS[nI].nodeName);
    strcat(tempString, " at ");
    strcat(tempString, Tchar(m_pRELIEFPOINTS[nI].time));
//
//  Start relief dropdown
//
    pComboBoxSTARTRELIEF->AddString(tempString);
    pComboBoxSTARTRELIEF->SetItemData(nJ, nI);
//
//  End relief dropdown
//
    pComboBoxENDRELIEF->AddString(tempString);
    pComboBoxENDRELIEF->SetItemData(nJ, nI);
    nJ++;
  }
//
//  Populate the combo box with all types of runs that have one piece
//
  for(nI = 0; nI < NUMRUNTYPES; nI++)
  { 
    for(nJ = 0; nJ < NUMRUNTYPESLOTS; nJ++)
    {
      if(!(RUNTYPE[nI][nJ].flags & RTFLAGS_INUSE))
      {
        continue;
      }
      if(RUNTYPE[nI][nJ].numPieces != 1)
      {
        continue;
      }
      nK = pComboBoxRUNTYPE->AddString(RUNTYPE[nI][nJ].localName);
      pComboBoxRUNTYPE->SetItemData(nK, MAKELONG(nI, nJ));
    }
  }
//
//  Set the first and last items (respectively)
//  in the Start Relief and End Relief dropdowns
//
  pComboBoxSTARTRELIEF->SetCurSel(0);
  m_PreviousStartPosition = 0;
  nI = pComboBoxENDRELIEF->GetCount();
  pComboBoxENDRELIEF->SetCurSel(nI - 1);
  m_PreviousEndPosition = nI - 1;
//
//  Show the run stats
//
  DisplayRunCharacteristics();


	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CManualCut::OnSelchangeRuntype() 
{
  int nI;

  nI = pComboBoxRUNTYPE->GetCurSel();
  if(nI != CB_ERR)
  {
    m_CutAsRuntype = pComboBoxRUNTYPE->GetItemData(nI);
    DisplayRunCharacteristics();
  }
	
}

void CManualCut::OnOK() 
{
	int nI;

  for(nI = m_firstIndex; nI <= m_lastIndex; nI++)
  {
    if(nI != m_lastIndex)
    {
      m_pRELIEFPOINTS[nI].start.recordID = NO_RECORD;
      m_pRELIEFPOINTS[nI].start.runNumber = m_GlobalRunNumber;
      m_pRELIEFPOINTS[nI].start.pieceNumber = 1;
    }
    if(nI != m_firstIndex)
    {
      m_pRELIEFPOINTS[nI].end.recordID = NO_RECORD;
      m_pRELIEFPOINTS[nI].end.runNumber = m_GlobalRunNumber;
      m_pRELIEFPOINTS[nI].end.pieceNumber = 1;
    }
  }
  long currentSelection = pComboBoxRUNTYPE->GetCurSel();
  long cutAsRuntype = pComboBoxRUNTYPE->GetItemData(currentSelection);

  for(nI = 0; nI < MAXRUNSINRUNLIST; nI++)
  {
    if(RUNLISTDATA[nI].runNumber == NO_RECORD)
    {
      RUNLISTDATA[nI].runNumber = m_GlobalRunNumber;
      RUNLISTDATA[nI].cutAsRuntype = cutAsRuntype;
      break;
    }
  }
  m_GlobalRunNumber++;

	CDialog::OnOK();
}

void CManualCut::OnCancel() 
{
	
	CDialog::OnCancel();
}

void CManualCut::OnHelp() 
{
	// TODO: Add your control notification handler code here
	
}

void CManualCut::OnClose() 
{
	
	CDialog::OnClose();
}

void CManualCut::DisplayRunCharacteristics()
{
  PROPOSEDRUNDef PROPOSEDRUN;
  int nI, nJ;
//
//  Set up PROPOSEDRUN
//
  PROPOSEDRUN.numPieces = 1;
  PROPOSEDRUN.piece[0].fromTime = m_pRELIEFPOINTS[m_firstIndex].time;
  PROPOSEDRUN.piece[0].fromNODESrecordID = m_pRELIEFPOINTS[m_firstIndex].NODESrecordID;
  PROPOSEDRUN.piece[0].fromTRIPSrecordID = m_pRELIEFPOINTS[m_firstIndex].TRIPSrecordID;
  PROPOSEDRUN.piece[0].toTime = m_pRELIEFPOINTS[m_lastIndex].time;
  PROPOSEDRUN.piece[0].toNODESrecordID = m_pRELIEFPOINTS[m_lastIndex].NODESrecordID;
  PROPOSEDRUN.piece[0].toTRIPSrecordID = m_pRELIEFPOINTS[m_lastIndex].TRIPSrecordID;
  PROPOSEDRUN.piece[0].prior.startTime = NO_TIME;
  PROPOSEDRUN.piece[0].prior.endTime = NO_TIME;
  PROPOSEDRUN.piece[0].after.startTime = NO_TIME;
  PROPOSEDRUN.piece[0].after.endTime = NO_TIME;
//
//  Cost the run
//
  long tempLong;

  tempLong = RunCoster(&PROPOSEDRUN, m_CutAsRuntype, &m_COST);
//
//  Dynamic travels?
//
  if(bUseDynamicTravels)
  {
    if(bUseCISPlan)
    {
      pButtonTRAVELINSTRUCTIONS->EnableWindow(TRUE);
    }
    else
    {
      pButtonTRAVELINSTRUCTIONS->EnableWindow(numTravelInstructions > 0);
    }
  }
//
//  Determine the runtype
//
  if(m_CutAsRuntype == NO_RECORD)
  {
    m_CutAsRuntype = tempLong;
    nJ = pComboBoxRUNTYPE->GetCount();
    for(nI = 0; nI < nJ; nI++)
    {
      if((long)pComboBoxRUNTYPE->GetItemData(nI) == m_CutAsRuntype)
      {
         pComboBoxRUNTYPE->SetCurSel(nI);
         break;
      } 
    }
  }
//
//  Spit out the stats
//
  char nodeName[NODES_ABBRNAME_LENGTH + 1];
//
//  Report at time
//
  tempLong = PROPOSEDRUN.piece[0].fromTime - m_COST.PIECECOST[0].reportTime - m_COST.TRAVEL[0].startTravelTime;
  pStaticREPORTAT->SetWindowText(Tchar(tempLong));
//
//  Start time
//
  pStaticONAT->SetWindowText(Tchar(PROPOSEDRUN.piece[0].fromTime));
//
//  Start location
//
  NODESKey0.recordID = PROPOSEDRUN.piece[0].fromNODESrecordID;
  btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
  strncpy(nodeName, NODES.abbrName, NODES_ABBRNAME_LENGTH);
  trim(nodeName, NODES_ABBRNAME_LENGTH);
  pStaticONLOC->SetWindowText(nodeName);
//
//  Off time
//
  pStaticOFFAT->SetWindowText(Tchar(PROPOSEDRUN.piece[0].toTime));
//
//  Off location
//
  NODESKey0.recordID = PROPOSEDRUN.piece[0].toNODESrecordID;
  btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
  strncpy(nodeName, NODES.abbrName, NODES_ABBRNAME_LENGTH);
  trim(nodeName, NODES_ABBRNAME_LENGTH);
  pStaticOFFLOC->SetWindowText(nodeName);
//
//  Platform time
//
  pStaticPLATFORMTIME->SetWindowText(chhmm(m_COST.TOTAL.platformTime));
//
//  Report time
//
  pStaticREPORTTIME->SetWindowText(chhmm(m_COST.TOTAL.reportTime));
//
//  Start of piece travel
//
  pStaticSTARTTRAVEL->SetWindowText(chhmm(m_COST.TRAVEL[0].startTravelTime));
//
//  End of piece travel
//
  pStaticENDTRAVEL->SetWindowText(chhmm(m_COST.TRAVEL[0].endTravelTime));
//
//  Total travel
//
  tempLong = m_COST.TRAVEL[0].startTravelTime + m_COST.TRAVEL[0].endTravelTime;
  pStaticTOTALTRAVEL->SetWindowText(chhmm(tempLong));
//
//  Turn-in time
//
  pStaticTURNINTIME->SetWindowText(chhmm(m_COST.TOTAL.turninTime));
//
//  Makeup time
//
  pStaticMAKEUPTIME->SetWindowText(chhmm(m_COST.TOTAL.makeUpTime));
//
//  Overtime
//
  pStaticOVERTIME->SetWindowText(chhmm(m_COST.TOTAL.overTime));
//
//  Spread time
//
  pStaticSPREADTIME->SetWindowText(chhmm(m_COST.spreadOvertime));
//
//  Pay time
//
  strcpy(tempString, " ");
  strcat(tempString, chhmm(m_COST.TOTAL.payTime));
  pStaticPAYTIME->SetWindowText(tempString);
//
//  Pay to platform
//
  if(m_COST.TOTAL.platformTime == 0)
  {
    strcpy(tempString, "");
  }
  else
  {
    sprintf(tempString, "%7.4f", (float)m_COST.TOTAL.payTime / (float)m_COST.TOTAL.platformTime);
  }
  pStaticPAYTOPLATFORM->SetWindowText(tempString);
//
//  Platform to pay
//
  if(m_COST.TOTAL.payTime == 0)
  {
    strcpy(tempString, "");
  }
  else
  {
    sprintf(tempString, "%7.4f", (float)m_COST.TOTAL.platformTime / (float)m_COST.TOTAL.payTime);
  }
  pStaticPLATFORMTOPAY->SetWindowText(tempString);
//
// But why?
//
  pButtonWHY->EnableWindow(m_COST.TOTAL.payTime == 0);
}

void CManualCut::OnShowdirections() 
{
  if(bUseCISPlan)
  {
    DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_TRAVELINSTRUCTIONS),
          m_hWnd, (DLGPROC)TRAVELINSTRUCTIONSMsgProc, (LPARAM)&m_COST);
  }
  else
  {
    long runNumber = 0;
    DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_TRAVELINSTRUCTIONS),
          m_hWnd, (DLGPROC)TRAVELINSTRUCTIONSMsgProc, (LPARAM)&runNumber);
  }
	
}

void CManualCut::OnWhy() 
{
  MessageBeep(MB_ICONINFORMATION);
  MessageBox(runcosterReason, TMS, MB_ICONINFORMATION | MB_OK); 
}

void CManualCut::OnSelchangeStartrelief() 
{
  int nI = pComboBoxSTARTRELIEF->GetCurSel();
  int nJ;

  if(nI == CB_ERR)
  {
    return;
  }

  nJ = pComboBoxSTARTRELIEF->GetItemData(nI);
  if(nJ >= m_lastIndex)
  {
    MessageBeep(MB_ICONSTOP);
    pComboBoxSTARTRELIEF->SetCurSel(m_PreviousStartPosition);
  }
  else
  {
    m_firstIndex = nJ;
    m_PreviousStartPosition = nI;
    DisplayRunCharacteristics();
  }
}

void CManualCut::OnSelchangeEndrelief() 
{
  int nI = pComboBoxENDRELIEF->GetCurSel();
  int nJ;

  if(nI == CB_ERR)
  {
    return;
  }

  nJ = pComboBoxENDRELIEF->GetItemData(nI);
  if(nJ <= m_firstIndex)
  {
    MessageBeep(MB_ICONSTOP);
    pComboBoxENDRELIEF->SetCurSel(m_PreviousEndPosition);
  }
  else
  {
    m_lastIndex = nJ;
    m_PreviousEndPosition = nI;
    DisplayRunCharacteristics();
  }
}

void CManualCut::OnClickPotentialreliefs(NMHDR* pNMHDR, LRESULT* pResult) 
{
	int nI = pListCtrlPOTENTIALRELIEFS->GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);
  int nJ;

  if(nI >= 0)
  {
    nJ = pComboBoxSTARTRELIEF->GetItemData(nI);
    if(nJ >= m_lastIndex)
    {
      MessageBeep(MB_ICONSTOP);
      pComboBoxSTARTRELIEF->SetCurSel(m_PreviousStartPosition);
    }
    else
    {
      m_firstIndex = nJ;
      m_PreviousStartPosition = nI;
      pComboBoxSTARTRELIEF->SetCurSel(nI);
      DisplayRunCharacteristics();
    }
  }

	*pResult = 0;
}

void CManualCut::OnRclickPotentialreliefs(NMHDR* pNMHDR, LRESULT* pResult) 
{
	int nI = pListCtrlPOTENTIALRELIEFS->GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);
  int nJ;

  if(nI >= 0)
  {
    nJ = pComboBoxENDRELIEF->GetItemData(nI);
    if(nJ <= m_firstIndex)
    {
      MessageBeep(MB_ICONSTOP);
      pComboBoxENDRELIEF->SetCurSel(m_PreviousEndPosition);
    }
    else
    {
      m_lastIndex = nJ;
      m_PreviousEndPosition = nI;
      DisplayRunCharacteristics();
      pComboBoxENDRELIEF->SetCurSel(nI);
    }
  }

	*pResult = 0;
}

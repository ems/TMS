// RosterMerge.cpp : implementation file
//

#include "stdafx.h"
extern "C"{
#include "TMSHeader.h"
}
#include "tms.h"
#include "Roster.h"
#include "DaysOffPatterns.h"
#include "RosterParms.h"
#include "RosterMerge.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CRosterMerge dialog


CRosterMerge::CRosterMerge(CWnd* pParent /*=NULL*/, int recordIndex, long DIVISIONSrecordID)
	: CDialog(CRosterMerge::IDD, pParent)
{
  m_RecordIndex = recordIndex;
  m_DIVISIONSrecordID = DIVISIONSrecordID;

	//{{AFX_DATA_INIT(CRosterMerge)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CRosterMerge::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CRosterMerge)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CRosterMerge, CDialog)
	//{{AFX_MSG_MAP(CRosterMerge)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDHELP, OnHelp)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CRosterMerge message handlers

BOOL CRosterMerge::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
//
//  Set up pointers to the list controls
//
  pListCtrlRECORD = (CListCtrl *)GetDlgItem(ROSTERMERGE_RECORD);
  pListCtrlWEEK1ROSTER = (CListCtrl *)GetDlgItem(ROSTERMERGE_WEEK1ROSTER);
  
  DWORD dwExStyles = pListCtrlRECORD->GetExtendedStyle();
  pListCtrlRECORD->SetExtendedStyle(dwExStyles | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_INFOTIP);
  
  dwExStyles = pListCtrlWEEK1ROSTER->GetExtendedStyle();
  pListCtrlWEEK1ROSTER->SetExtendedStyle(dwExStyles | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_INFOTIP);
//
//  Build the list control titles
//
  LVCOLUMN LVC;
  LVC.mask = LVCF_FMT | LVCF_TEXT | LVCF_WIDTH;
  LVC.fmt = LVCFMT_CENTER;
  CString s;
  BOOL bFound;
  long weekPayTime;
  long overtimePay;
  int  col = 0;
  int  nI, nJ, nK, nL;
  int  serviceIndex, runIndex;
//
//  "Number"
//
  LVC.cx = 50;
  s.LoadString(TEXT_072);
  strcpy(tempString, s);
  LVC.pszText = tempString;
  pListCtrlRECORD->InsertColumn(col, &LVC);
  pListCtrlWEEK1ROSTER->InsertColumn(col, &LVC);
//
//  Cycle through the days of the week
//
  LVC.cx = 100;
  for(nI = 0; nI < ROSTER_MAX_DAYS; nI++)
  {
    col++;
    s.LoadString(TEXT_009 + nI);
    strcpy(tempString, s);
    LVC.pszText = tempString;
    pListCtrlRECORD->InsertColumn(col, &LVC);
    pListCtrlWEEK1ROSTER->InsertColumn(col, &LVC);
  }
//
//  "Hours"
//
  LVC.cx = 56;
  col++;
  s.LoadString(TEXT_073);
  strcpy(tempString, s);
  LVC.pszText = tempString;
  pListCtrlRECORD->InsertColumn(col, &LVC);
  pListCtrlWEEK1ROSTER->InsertColumn(col, &LVC);
//
//  "O/T"
//
  col++;
  s.LoadString(TEXT_075);
  strcpy(tempString, s);
  LVC.pszText = tempString;
  pListCtrlRECORD->InsertColumn(col, &LVC);
  pListCtrlWEEK1ROSTER->InsertColumn(col, &LVC);
//
//  "Pay"
//
  col++;
  s.LoadString(TEXT_076);
  strcpy(tempString, s);
  LVC.pszText = tempString;
  pListCtrlRECORD->InsertColumn(col, &LVC);
  pListCtrlWEEK1ROSTER->InsertColumn(col, &LVC);
//
//  Populate the "record"
//
  LVITEM LVI;
//
//  Roster number
//
  sprintf(tempString, "%ld", m_pCOMBINED[m_RecordIndex].rosterNumber);
  LVI.mask = LVIF_PARAM | LVIF_TEXT;
  LVI.iItem = 0;
  LVI.iSubItem = 0;
  LVI.lParam = m_RecordIndex;
  LVI.pszText = tempString;
  LVI.iItem = pListCtrlRECORD->InsertItem(&LVI);
//
//  Go through the days - the data's actually held in m_pRRLIST
//
  LVI.iSubItem = 1;
  weekPayTime = 0;
  overtimePay = 0;
  for(nJ = 0; nJ < ROSTER_MAX_DAYS; nJ++)
  {
//
//  If the day is NO_RECORD, display OFF if it's a rostered run
//
    s = "";
    if(m_pCOMBINED[m_RecordIndex].RUNSrecordID[nJ] == NO_RECORD)
    {
      if(m_pCOMBINED[m_RecordIndex].frozen)
      {
        s.LoadString(TEXT_074);
        LVI.mask = LVIF_TEXT;
        strcpy(tempString, s);
        LVI.pszText = tempString;
        pListCtrlRECORD->SetItem(&LVI);
      }
    }
//
//  The day wasn't NO_RECORD
//
    else
    {
      nK = 0;
      bFound = FALSE;
      while(m_pRRLIST[nK].SERVICESrecordID != NO_RECORD)
      {
        for(nL = 0; nL < m_pRRLIST[nK].numRuns; nL++)
        {
          if((m_pRRLIST[nK].pData[nL].RUNSrecordID == m_pCOMBINED[m_RecordIndex].RUNSrecordID[nJ]) &&
                (m_pRRLIST[nK].pData[nL].flags == m_pCOMBINED[m_RecordIndex].flags[nJ]))
          {
            serviceIndex = nK;
            runIndex = nL;
            bFound = TRUE;
            break;
          }
        }
        if(bFound)
        {
          break;
        }
        nK++;
      }
//
//  Display data as per selection
//
      if(bFound)
      {
//
//  Run number
//
        sprintf(szarString, "%ld", m_pRRLIST[serviceIndex].pData[runIndex].runNumber);
        s += szarString;
        s += "-";
//
//  On location
//
        NODESKey0.recordID = m_pRRLIST[serviceIndex].pData[runIndex].onNODESrecordID;
        btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
        strncpy(tempString, NODES.abbrName, NODES_ABBRNAME_LENGTH);
        trim(tempString, NODES_ABBRNAME_LENGTH);
        s += tempString;
        s += "@";
//
//  On time
//
        s += Tchar(m_pRRLIST[serviceIndex].pData[runIndex].onTime);
        s += "/";
//
//  Off location
//
        NODESKey0.recordID = m_pRRLIST[serviceIndex].pData[runIndex].offNODESrecordID;
        btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
        strncpy(tempString, NODES.abbrName, NODES_ABBRNAME_LENGTH);
        trim(tempString, NODES_ABBRNAME_LENGTH);
        s += tempString;
        s += "@";
//
//  Off time
//
        s += Tchar(m_pRRLIST[serviceIndex].pData[runIndex].offTime);
        s += "(";
//
//  Run type
//
        nK = (short int)LOWORD(m_pRRLIST[serviceIndex].pData[runIndex].runtype);
        nL = (short int)HIWORD(m_pRRLIST[serviceIndex].pData[runIndex].runtype);
        s += RUNTYPE[nK][nL].localName;
        s += " Pays ";
//
//  Pay time
//
        s += chhmm(m_pRRLIST[serviceIndex].pData[runIndex].payTime);
        s += ")";
        weekPayTime += m_pRRLIST[serviceIndex].pData[runIndex].payTime;
//
//  Add it in
//
        LVI.mask = LVIF_TEXT;
        strcpy(tempString, s);
        LVI.pszText = tempString;
        pListCtrlRECORD->SetItem(&LVI);
      }
    }
    LVI.iSubItem++;
  }
//
//  Weekly pay
//
  LVI.mask = LVIF_TEXT;
  strcpy(tempString, chhmm(weekPayTime));
  LVI.pszText = tempString;
  pListCtrlRECORD->SetItem(&LVI);
  LVI.iSubItem++;
//
//  Overtime
//
  if(weekPayTime > OVERTIME.weeklyAfter[WEEK5DAYS])
  {
    overtimePay = (long)((weekPayTime - OVERTIME.weeklyAfter[WEEK5DAYS]) * (OVERTIME.weeklyRate[WEEK5DAYS] - 1));
  }
  else
  {
    overtimePay = 0;
  }
  LVI.mask = LVIF_TEXT;
  strcpy(tempString, chhmm(overtimePay));
  LVI.pszText = tempString;
  pListCtrlRECORD->SetItem(&LVI);
  LVI.iSubItem++;
  weekPayTime += overtimePay;
//
//  Weekly pay including overtime
//
  LVI.mask = LVIF_TEXT;
  strcpy(tempString, chhmm(weekPayTime));
  LVI.pszText = tempString;
  pListCtrlRECORD->SetItem(&LVI);
//
//  Now build up and show the week 1 roster
//
  PROPOSEDRUNDef PROPOSEDRUN;
  COSTDef COST;
  long runNumber;
  long serviceRecordID;
  long runtype;
  long onTime, offTime;
  int  itemNumber = 0;
  int  rcode2;
  int  pieceNumber;

  ROSTERKey1.DIVISIONSrecordID = m_DIVISIONSrecordID;
  ROSTERKey1.rosterNumber = NO_RECORD;
  rcode2 = btrieve(B_GETGREATER, TMS_ROSTER, &ROSTER, &ROSTERKey1, 1);
  while(rcode2 == 0 &&
        ROSTER.DIVISIONSrecordID == m_DIVISIONSrecordID)
  {
//
//  Roster Number
//
    sprintf(tempString, "%ld", ROSTER.rosterNumber);
    LVI.mask = LVIF_PARAM | LVIF_TEXT;
    LVI.iItem = itemNumber;
    LVI.iSubItem = 0;
    LVI.lParam = ROSTER.rosterNumber;
    LVI.pszText = tempString;
    LVI.iItem = pListCtrlWEEK1ROSTER->InsertItem(&LVI);
//
//  Cycle through the days
//
    weekPayTime = 0;
    for(nI = 0; nI < ROSTER_MAX_DAYS; nI++)
    {
      LVI.iSubItem = nI + 1;
      LVI.mask = LVIF_TEXT;
//
//  Is he off today?
//
      if(ROSTER.WEEK[0].RUNSrecordIDs[nI] == NO_RECORD)
      {
        s.LoadString(TEXT_074);
        strcpy(tempString, s);
        LVI.pszText = tempString;
        pListCtrlWEEK1ROSTER->SetItem(&LVI);
        continue;
      }
//
//  Crew only run?
//
      if(ROSTER.WEEK[0].flags & (1 << nI))
      {
        CREWONLYKey0.recordID = ROSTER.WEEK[0].RUNSrecordIDs[nI];
        rcode2 = btrieve(B_GETEQUAL, TMS_CREWONLY, &CREWONLY, &CREWONLYKey0, 0);
//
//  Cost the run
//
        if(rcode2 == 0)
        {
          pieceNumber = 0;
          runNumber = CREWONLY.runNumber;
          serviceRecordID = CREWONLY.SERVICESrecordID;
          runtype = CREWONLY.cutAsRuntype;
          while(rcode2 == 0 &&
                runNumber == CREWONLY.runNumber &&
                serviceRecordID == CREWONLY.SERVICESrecordID)
          {
//
//  Set up PROPOSEDRUN
//
            PROPOSEDRUN.piece[pieceNumber].fromTime = CREWONLY.startTime;
            PROPOSEDRUN.piece[pieceNumber].fromNODESrecordID = CREWONLY.startNODESrecordID;
            PROPOSEDRUN.piece[pieceNumber].fromTRIPSrecordID = NO_RECORD;
            PROPOSEDRUN.piece[pieceNumber].toTime = CREWONLY.endTime;
            PROPOSEDRUN.piece[pieceNumber].toNODESrecordID = CREWONLY.endNODESrecordID;
            PROPOSEDRUN.piece[pieceNumber].toTRIPSrecordID = NO_RECORD;
            PROPOSEDRUN.piece[pieceNumber].prior.startTime = NO_TIME;
            PROPOSEDRUN.piece[pieceNumber].prior.endTime = NO_TIME;
            PROPOSEDRUN.piece[pieceNumber].after.startTime = NO_TIME;
            PROPOSEDRUN.piece[pieceNumber].after.endTime = NO_TIME;
//
//  Get the next (crewonly) run record
//
            rcode2 = btrieve(B_GETNEXT, TMS_CREWONLY, &CREWONLY, &CREWONLYKey1, 1);
            pieceNumber++;
          }
//
//  Cost the (crewonly) run
//
          if(pieceNumber == 0)  // Run didn't set up
          {
            MessageBeep(MB_ICONSTOP);
            LoadString(hInst, TEXT_067, szarString, sizeof(szarString));
            LoadString(hInst, ERROR_149, szFormatString, sizeof(szFormatString));
            sprintf(tempString, szFormatString, runNumber, szarString);
            MessageBox(tempString, TMS, MB_ICONSTOP | MB_OK);
            continue;
          }
          PROPOSEDRUN.numPieces = pieceNumber;
          RunCoster(&PROPOSEDRUN, runtype, &COST);
        } // rcode2
      }  // Crewonly
//
//  Regular run
//
      else
      {
        RUNSKey0.recordID = ROSTER.WEEK[0].RUNSrecordIDs[nI];
        rcode2 = btrieve(B_GETEQUAL, TMS_RUNS, &RUNS, &RUNSKey0, 0);
//
//  Cost the run
//
        if(rcode2 == 0)
        {
          pieceNumber = 0;
          runNumber = RUNS.runNumber;
          serviceRecordID = RUNS.SERVICESrecordID;
          runtype = RUNS.cutAsRuntype;
          while(rcode2 == 0 &&
                runNumber == RUNS.runNumber &&
                serviceRecordID == RUNS.SERVICESrecordID)
          {
            RunStartAndEnd(RUNS.start.TRIPSrecordID, RUNS.start.NODESrecordID,
                  RUNS.end.TRIPSrecordID, RUNS.end.NODESrecordID, &onTime, &offTime);
//
//  Set up PROPOSEDRUN
//
            PROPOSEDRUN.piece[pieceNumber].fromTime = onTime;
            PROPOSEDRUN.piece[pieceNumber].fromNODESrecordID = RUNS.start.NODESrecordID;
            PROPOSEDRUN.piece[pieceNumber].fromTRIPSrecordID = RUNS.start.TRIPSrecordID;
            PROPOSEDRUN.piece[pieceNumber].toTime = offTime;
            PROPOSEDRUN.piece[pieceNumber].toNODESrecordID = RUNS.end.NODESrecordID;
            PROPOSEDRUN.piece[pieceNumber].toTRIPSrecordID = RUNS.end.TRIPSrecordID;
            PROPOSEDRUN.piece[pieceNumber].prior.startTime = RUNS.prior.startTime;
            PROPOSEDRUN.piece[pieceNumber].prior.endTime = RUNS.prior.endTime;
            PROPOSEDRUN.piece[pieceNumber].after.startTime = RUNS.after.startTime;
            PROPOSEDRUN.piece[pieceNumber].after.endTime = RUNS.after.endTime;
//
//  Get the next run record
//
            rcode2 = btrieve(B_GETNEXT, TMS_RUNS, &RUNS, &RUNSKey1, 1);
            pieceNumber++;
          }
//
//  Cost the run
//
          if(pieceNumber == 0)  // Run didn't set up
          {
            MessageBeep(MB_ICONSTOP);
            LoadString(hInst, TEXT_067, szarString, sizeof(szarString));
            LoadString(hInst, ERROR_149, szFormatString, sizeof(szFormatString));
            sprintf(tempString, szFormatString, runNumber, szarString);
            MessageBox(tempString, TMS, MB_ICONSTOP | MB_OK);
            continue;
          }
          PROPOSEDRUN.numPieces = pieceNumber;
          RunCoster(&PROPOSEDRUN, runtype, &COST);
        }
      }
//
//  Display it
//
//
//  Run number
//
      sprintf(szarString, "%ld", runNumber);
      s = szarString;
      s += "-";
//
//  On location
//
      NODESKey0.recordID = PROPOSEDRUN.piece[0].fromNODESrecordID;
      btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
      strncpy(tempString, NODES.abbrName, NODES_ABBRNAME_LENGTH);
      trim(tempString, NODES_ABBRNAME_LENGTH);
      s += tempString;
      s += "@";
//
//  On time
//
      s += Tchar(PROPOSEDRUN.piece[0].fromTime);
      s += "/";
//
//  Off location
//
      NODESKey0.recordID = PROPOSEDRUN.piece[pieceNumber - 1].toNODESrecordID;
      btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
      strncpy(tempString, NODES.abbrName, NODES_ABBRNAME_LENGTH);
      trim(tempString, NODES_ABBRNAME_LENGTH);
      s += tempString;
      s += "@";
//
//  Off time
//
      s += Tchar(PROPOSEDRUN.piece[pieceNumber - 1].toTime);
      s += "(";
//
//  Run type
//
      if(runtype >= 0)
      {
        nK = (short int)LOWORD(runtype);
        nL = (short int)HIWORD(runtype);
        s += RUNTYPE[nK][nL].localName;
      }
      else
      {
        s += "Unclassified";
      }
      s += " Pays ";
//
//  Pay time
//
      s += chhmm(COST.TOTAL.payTime);
      s += ")";
      weekPayTime += COST.TOTAL.payTime;
//
//  Add it in to the control
//
      strcpy(tempString, s);
      LVI.pszText = tempString;
      pListCtrlWEEK1ROSTER->SetItem(&LVI);
    }  // nI
//
//  Weekly pay
//
    LVI.iSubItem++;
    LVI.mask = LVIF_TEXT;
    strcpy(tempString, chhmm(weekPayTime));
    LVI.pszText = tempString;
    pListCtrlWEEK1ROSTER->SetItem(&LVI);
//
//  Overtime
//
    if(weekPayTime > OVERTIME.weeklyAfter[WEEK5DAYS])
    {
      overtimePay = (long)((weekPayTime - OVERTIME.weeklyAfter[WEEK5DAYS]) * (OVERTIME.weeklyRate[WEEK5DAYS] - 1));
    }
    else
    {
      overtimePay = 0;
    }
    LVI.iSubItem++;
    LVI.mask = LVIF_TEXT;
    strcpy(tempString, chhmm(overtimePay));
    LVI.pszText = tempString;
    pListCtrlWEEK1ROSTER->SetItem(&LVI);
    weekPayTime += overtimePay;
//
//  Weekly pay including overtime
//
    LVI.iSubItem++;
    LVI.mask = LVIF_TEXT;
    strcpy(tempString, chhmm(weekPayTime));
    LVI.pszText = tempString;
    pListCtrlWEEK1ROSTER->SetItem(&LVI);
//
//  Get the next roster record
//
    itemNumber++;
    rcode2 = btrieve(B_GETNEXT, TMS_ROSTER, &ROSTER, &ROSTERKey1, 1);
  }  // while rcode2==0 on roster

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CRosterMerge::OnClose() 
{
  OnCancel();
}

void CRosterMerge::OnHelp() 
{
}

void CRosterMerge::OnCancel() 
{
	CDialog::OnCancel();
}

void CRosterMerge::OnOK() 
{
  long rosterNumber;
  int  nItem;
//
//  Get the selection
//  
  POSITION pos = pListCtrlWEEK1ROSTER->GetFirstSelectedItemPosition();
  
  if(pos == NULL)
  {
    MessageBeep(MB_ICONSTOP);
    MessageBox("Please select a week 1 roster prior to pressing OK", TMS, MB_ICONSTOP | MB_OK);
    return;
  }
//
//  Get the roster number
//
  nItem = pListCtrlWEEK1ROSTER->GetNextSelectedItem(pos);
  rosterNumber = pListCtrlWEEK1ROSTER->GetItemData(nItem);
//
//  Set the roster number for the incoming record
//
  m_pCOMBINED[m_RecordIndex].rosterNumber = rosterNumber;

	CDialog::OnOK();
}

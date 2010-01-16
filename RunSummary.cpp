//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
// RunSummary.cpp : implementation file
//

#include "stdafx.h"
extern "C"
{
#include "TMSHeader.h"
}
#include "RunSummary.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static RUNSUMMARYDef RS[NUMRUNTYPES][NUMRUNTYPESLOTS];
static RUNSUMMARYDef UNCLASSIFIED;
static RUNSUMMARYDef NOTPAYINGOUT;
static RUNSUMMARYDef TOTAL;
static int numDisplayed;
static int totalRuns;

void CRunSummary::OutputRow(char* pszRuntype, RUNSUMMARYDef* pRS, BOOL bShowLines)
{
  LVITEM LVI;
  int    ratioPay;
  char  *pszLines = "-------";

  memset(&LVI, 0x00, sizeof(LVITEM));
  LVI.mask = LVIF_TEXT;
  LVI.iItem = numDisplayed;
//
//  Runtype
//
  LVI.iSubItem = 0;
  LVI.pszText = bShowLines ? "" : pszRuntype;
  LVI.iItem = pListCtrlLIST->InsertItem(&LVI);
//
//  Number of runs
//
  LVI.mask = LVIF_TEXT;
  LVI.iSubItem = 1;
  if(bShowLines)
  {
    LVI.pszText = pszLines;
  }
  else
  {
    itoa(pRS->numRuns, tempString, 10);
    LVI.pszText = tempString;
  }
  pListCtrlLIST->SetItem(&LVI);
//
//  Platform time
//
  LVI.mask = LVIF_TEXT;
  LVI.iSubItem = 2;
  LVI.pszText = bShowLines ? pszLines : chhmm(pRS->platformTime);
  pListCtrlLIST->SetItem(&LVI);
//
//  Report time
//
  LVI.mask = LVIF_TEXT;
  LVI.iSubItem = 3;
  LVI.pszText = bShowLines ? pszLines : chhmm(pRS->reportTime);
  pListCtrlLIST->SetItem(&LVI);
//
//  Travel time
//
  LVI.mask = LVIF_TEXT;
  LVI.iSubItem = 4;
  LVI.pszText = bShowLines ? pszLines : chhmm(pRS->travelTime);
  pListCtrlLIST->SetItem(&LVI);
//
//  Turnin time
//
  LVI.mask = LVIF_TEXT;
  LVI.iSubItem = 5;
  LVI.pszText = bShowLines ? pszLines : chhmm(pRS->turninTime);
  pListCtrlLIST->SetItem(&LVI);
//
//  MakeUp time
//
  LVI.mask = LVIF_TEXT;
  LVI.iSubItem = 6;
  LVI.pszText = bShowLines ? pszLines : chhmm(pRS->makeUpTime);
  pListCtrlLIST->SetItem(&LVI);
//
//  Paid breaks
//
  LVI.mask = LVIF_TEXT;
  LVI.iSubItem = 7;
  LVI.pszText = bShowLines ? pszLines : chhmm(pRS->paidBreaks);
  pListCtrlLIST->SetItem(&LVI);
//
//  Spread OT
//
  LVI.mask = LVIF_TEXT;
  LVI.iSubItem = 8;
  LVI.pszText = bShowLines ? pszLines : chhmm(pRS->spreadOvertime);
  pListCtrlLIST->SetItem(&LVI);
//
//  Overtime
//
  LVI.mask = LVIF_TEXT;
  LVI.iSubItem = 9;
  LVI.pszText = bShowLines ? pszLines : chhmm(pRS->overTime);
  pListCtrlLIST->SetItem(&LVI);
//
//  Pay time
//
  LVI.mask = LVIF_TEXT;
  LVI.iSubItem = 10;
  LVI.pszText = bShowLines ? pszLines : chhmm(pRS->payTime);
  pListCtrlLIST->SetItem(&LVI);
//
//  Ratio
//
  if(bShowLines)
  {
    LVI.pszText = pszLines;
  }
  else
  {
    if(pRS->payTime == 0)
    {
      ratioPay = 0;
    }
    else
    {
      ratioPay = pRS->platformTime;
      if(m_bReport)
      {
        ratioPay += pRS->reportTime;
        ratioPay += pRS->turninTime;
      }
      if(m_bTravel)
      {
        ratioPay += pRS->travelTime;
      }
      if(m_bMakeup)
      {
        ratioPay += pRS->makeUpTime;
      }
      if(m_bOvertime)
      {
        ratioPay += pRS->overTime;
      }
      if(m_bSpreadOT)
      {
        ratioPay += pRS->spreadOvertime;
      }
      if(m_bPaidBreaks)
      {
        ratioPay += pRS->paidBreaks;
      }
    }
    if(m_Ratio == RUNSUMMARY_PLATFORMTOPAY)
    {
      strcpy(tempString, floatString(pRS->platformTime, ratioPay, FALSE));
    }
    else
    {
      strcpy(tempString, floatString(ratioPay, pRS->platformTime, FALSE));
    }
    LVI.pszText = tempString;
  }
  LVI.mask = LVIF_TEXT;
  LVI.iSubItem = 11;
  pListCtrlLIST->SetItem(&LVI);
//
// Percentage of cut
//
  LVI.mask = LVIF_TEXT;
  LVI.iSubItem = 12;
  if(bShowLines)
  {
    LVI.pszText = pszLines;
  }
  else
  {
    strcpy(tempString, floatString(pRS->numRuns, totalRuns, TRUE));
    LVI.pszText = tempString;
  }
  pListCtrlLIST->SetItem(&LVI);
//
//  All done
//
  numDisplayed++;
}


/////////////////////////////////////////////////////////////////////////////
// CRunSummary dialog


CRunSummary::CRunSummary(CWnd* pParent, /*=NULL*/ PDISPLAYINFO pDI)
	: CDialog(CRunSummary::IDD, pParent)
{
  m_pDI = pDI;
	//{{AFX_DATA_INIT(CRunSummary)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CRunSummary::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CRunSummary)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CRunSummary, CDialog)
	//{{AFX_MSG_MAP(CRunSummary)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDHELP, OnHelp)
	ON_BN_CLICKED(RUNSUMMARY_UNCLASSIFIED, OnUnclassified)
	ON_BN_CLICKED(RUNSUMMARY_PAYTOPLATFORM, OnPaytoplatform)
	ON_BN_CLICKED(RUNSUMMARY_PLATFORMTOPAY, OnPlatformtopay)
	ON_BN_CLICKED(RUNSUMMARY_REPORT, OnReport)
	ON_BN_CLICKED(RUNSUMMARY_TRAVEL, OnTravel)
	ON_BN_CLICKED(RUNSUMMARY_MAKEUP, OnMakeup)
	ON_BN_CLICKED(RUNSUMMARY_OVERTIME, OnOvertime)
	ON_BN_CLICKED(IDPRINT, OnPrint)
	ON_BN_CLICKED(RUNSUMMARY_SPREADOT, OnSpreadot)
	ON_BN_CLICKED(RUNSUMMARY_PAIDBREAKS, OnPaidbreaks)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CRunSummary message handlers

void CRunSummary::OnClose() 
{
	// TODO: Add your message handler code here and/or call default
	
	CDialog::OnClose();
}

BOOL CRunSummary::OnInitDialog() 
{
	CDialog::OnInitDialog();
//
//  Set up the pointers to the controls
//
  pListCtrlLIST = (CListCtrl *)GetDlgItem(RUNSUMMARY_LIST);
  pButtonUNCLASSIFIED = (CButton *)GetDlgItem(RUNSUMMARY_UNCLASSIFIED);
  pButtonPAYTOPLATFORM = (CButton *)GetDlgItem(RUNSUMMARY_PAYTOPLATFORM);
  pButtonPLATFORMTOPAY = (CButton *)GetDlgItem(RUNSUMMARY_PLATFORMTOPAY);
  pButtonREPORT = (CButton *)GetDlgItem(RUNSUMMARY_REPORT);
  pButtonTRAVEL = (CButton *)GetDlgItem(RUNSUMMARY_TRAVEL);
  pButtonMAKEUP = (CButton *)GetDlgItem(RUNSUMMARY_MAKEUP);
  pButtonOVERTIME = (CButton *)GetDlgItem(RUNSUMMARY_OVERTIME);
  pButtonSPREADOT = (CButton *)GetDlgItem(RUNSUMMARY_SPREADOT);
  pButtonPAIDBREAKS = (CButton *)GetDlgItem(RUNSUMMARY_PAIDBREAKS);

  DWORD dwExStyles = pListCtrlLIST->GetExtendedStyle();
  pListCtrlLIST->SetExtendedStyle(dwExStyles | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_INFOTIP);
//
//  Get anyexisting values from TMS.INI
//
  m_bShowUnclassified = GetPrivateProfileInt(userName, "ShowUnclassifiedRuns", FALSE, TMSINIFile);
  m_Ratio = GetPrivateProfileInt(userName, "SelectedRatio", RUNSUMMARY_PLATFORMTOPAY, TMSINIFile);
  m_bReport = GetPrivateProfileInt(userName, "RunSummaryReport", TRUE, TMSINIFile);
  m_bTravel = GetPrivateProfileInt(userName, "RunSummaryTravel", TRUE, TMSINIFile);
  m_bMakeup = GetPrivateProfileInt(userName, "RunSummaryMakeup", TRUE, TMSINIFile);
  m_bOvertime = GetPrivateProfileInt(userName, "RunSummaryOvertime", TRUE, TMSINIFile);
  m_bSpreadOT = GetPrivateProfileInt(userName, "RunSummarySpreadOT", TRUE, TMSINIFile);
  m_bPaidBreaks = GetPrivateProfileInt(userName, "RunSummaryPaidBreaks", TRUE, TMSINIFile);
//
//  And show them on the dialog
//
  pButtonUNCLASSIFIED->SetCheck(m_bShowUnclassified);
  CButton* x = (CButton *)GetDlgItem(m_Ratio);
  x->SetCheck(TRUE);
  pButtonREPORT->SetCheck(m_bReport);
  pButtonTRAVEL->SetCheck(m_bTravel);
  pButtonMAKEUP->SetCheck(m_bMakeup);
  pButtonOVERTIME->SetCheck(m_bOvertime);
  pButtonSPREADOT->SetCheck(m_bSpreadOT);
  pButtonPAIDBREAKS->SetCheck(m_bPaidBreaks);
//
//  Get the run data
//
  memset(&RS, 0x00, sizeof(RS));
  memset(&UNCLASSIFIED, 0x00, sizeof(UNCLASSIFIED));
  memset(&NOTPAYINGOUT, 0x00, sizeof(NOTPAYINGOUT));
  totalRuns = 0;
  DoCalc();
//
//  Ok - one way or another, we've got everything.
//
//  Set up the list control headers
//
  LVCOLUMN LVC;
  
  LVC.mask = LVCF_FMT | LVCF_TEXT | LVCF_WIDTH;
  LVC.fmt = LVCFMT_LEFT;
  LVC.cx = 90;
  LVC.pszText = "Runtype";
  pListCtrlLIST->InsertColumn(0, &LVC);
  LVC.fmt = LVCFMT_RIGHT;
  LVC.cx = 55;
  LVC.pszText = "#Runs";
  pListCtrlLIST->InsertColumn(1, &LVC);
  LVC.cx = 55;
  LVC.pszText = "Plat";
  pListCtrlLIST->InsertColumn(2, &LVC);
  LVC.cx = 55;
  LVC.pszText = "Rpt";
  pListCtrlLIST->InsertColumn(3, &LVC);
  LVC.cx = 55;
  LVC.pszText = "Trv";
  pListCtrlLIST->InsertColumn(4, &LVC);
  LVC.cx = 55;
  LVC.pszText = "T/I";
  pListCtrlLIST->InsertColumn(5, &LVC);
  LVC.cx = 55;
  LVC.pszText = "MkUp";
  pListCtrlLIST->InsertColumn(6, &LVC);
  LVC.cx = 55;
  LVC.pszText = "PdBrk";
  pListCtrlLIST->InsertColumn(7, &LVC);
  LVC.cx = 55;
  LVC.pszText = "SpdPrm";
  pListCtrlLIST->InsertColumn(8, &LVC);
  LVC.cx = 55;
  LVC.pszText = "O/T";
  pListCtrlLIST->InsertColumn(9, &LVC);
  LVC.cx = 55;
  LVC.pszText = "Pay";
  pListCtrlLIST->InsertColumn(10, &LVC);
  LVC.cx = 55;
  LVC.pszText = "Ratio";
  pListCtrlLIST->InsertColumn(11, &LVC);
  LVC.cx = 65;
  LVC.pszText = "%Cut";
  pListCtrlLIST->InsertColumn(12, &LVC);
//
//  Set up the body of the list control
//
  DoDisplay();
//
//  Set the title
//
  GetWindowText(szFormatString, sizeof(szFormatString));
  sprintf(tempString, szFormatString, m_ServiceName, m_DivisionName);
  SetWindowText(tempString);
//
//  All done
//
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CRunSummary::OnHelp() 
{
  ::WinHelp(m_hWnd, szarHelpFile, HELP_CONTEXT, Run_summary);
}

void CRunSummary::OnOK() 
{
//
//  Save the dialog selections in the TMS.INI file
//
//  Display unclassified runs
//
  itoa(pButtonUNCLASSIFIED->GetCheck() ? 1 : 0, tempString, 10); 
  WritePrivateProfileString(userName, "ShowUnclassifiedRuns", tempString, TMSINIFile);
//
//  Ratio selection
//
  if(pButtonPAYTOPLATFORM->GetCheck())
  {
    itoa(RUNSUMMARY_PAYTOPLATFORM, tempString, 10);
  }
  else
  {
    itoa(RUNSUMMARY_PLATFORMTOPAY, tempString, 10);
  }
  WritePrivateProfileString(userName, "SelectedRatio", tempString, TMSINIFile);
//
//  Report time
//
  itoa(pButtonREPORT->GetCheck() ? 1 : 0, tempString, 10);
  WritePrivateProfileString(userName, "RunSummaryReport", tempString, TMSINIFile);
//
//  Travel time
//
  itoa(pButtonTRAVEL->GetCheck() ? 1 : 0, tempString, 10);
  WritePrivateProfileString(userName, "RunSummaryTravel", tempString, TMSINIFile);
//
//  Makeup time
//
  itoa(pButtonMAKEUP->GetCheck() ? 1 : 0, tempString, 10);
  WritePrivateProfileString(userName, "RunSummaryMakeup", tempString, TMSINIFile);
//
//  Overtime
//
  itoa(pButtonOVERTIME->GetCheck() ? 1 : 0, tempString, 10);
  WritePrivateProfileString(userName, "RunSummaryOvertime", tempString, TMSINIFile);
//
//  Spread overtime
//
  itoa(pButtonSPREADOT->GetCheck() ? 1 : 0, tempString, 10);
  WritePrivateProfileString(userName, "RunSummarySpreadOT", tempString, TMSINIFile);
//
//  Paid breaks
//
  itoa(pButtonPAIDBREAKS->GetCheck() ? 1: 0, tempString, 10);
  WritePrivateProfileString(userName, "RunSummarySpreadOT", tempString, TMSINIFile);
//
//  All done
//
	CDialog::OnOK();
}

void CRunSummary::DoCalc()
{
  RUNSUMMARYDef *RSPtr;
  PROPOSEDRUNDef PROPOSEDRUN;
  COSTDef COST;
  HCURSOR hCursor;
  long *RUNSrecordIDs;
  int numRuns;
  int nI;
  int nJ;
  int nK;
  int rcode2;
  int runType;
  int numPieces;
//
//  Set the hourglass
//
  hCursor = SetCursor(hCursorWait);
//
//  From within the runcutter...
//
  if(m_pDI == NULL)
  {
//
//  Look at all the runs in RUNLIST
//
    numRuns = AssembleRuns();
    for(nI = 0; nI < numRuns; nI++)
    {
//
//  Cost the run
//
      for(runType = NO_RECORD, nJ = 0; nJ < MAXRUNSINRUNLIST; nJ++)
      {
        if(RUNLIST[nI].runNumber == RUNLISTDATA[nJ].runNumber)
        {
          runType = RUNLISTDATA[nJ].cutAsRuntype;
          break;
        }
      }
      RunCoster(&RUNLIST[nI].run, runType, &COST);
      if(runType == NO_RECORD)
      {
        RSPtr = m_bShowUnclassified ? &UNCLASSIFIED : NULL;
      }
      else
      {
        nJ = (short int)LOWORD(runType);
        nK = (short int)HIWORD(runType);
        if(COST.TOTAL.payTime == 0)
        {
          RSPtr = &NOTPAYINGOUT;
        }
        else
        {
          RSPtr = &RS[nJ][nK];
        }
      }
      if(RSPtr)
      {
        RSPtr->numRuns++;
        RSPtr->platformTime += COST.TOTAL.platformTime;
        RSPtr->reportTime += COST.TOTAL.reportTime;
        RSPtr->turninTime += COST.TOTAL.turninTime;
        RSPtr->overTime += COST.TOTAL.overTime;
        RSPtr->makeUpTime += COST.TOTAL.makeUpTime;
        RSPtr->payTime += COST.TOTAL.payTime;
        RSPtr->spreadOvertime += COST.spreadOvertime;
        RSPtr->paidBreaks += COST.TOTAL.paidBreak;
        for(nJ = 0; nJ < RUNLIST[nI].run.numPieces; nJ++)
        {
          RSPtr->travelTime += (COST.TRAVEL[nJ].startTravelTime + COST.TRAVEL[nJ].endTravelTime);
        }
        totalRuns++;
      }
    }
  }
//
//  From the RUNS file
//
  else
  {
    char dummy[256];

    rcode2 = btrieve(B_STAT, TMS_RUNS, &BSTAT, dummy, 0);
    if(rcode2 == 0 && BSTAT.numRecords > 0)
    {
      RUNSrecordIDs = (long *)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(long) * BSTAT.numRecords); 
      if(RUNSrecordIDs == NULL)
      {
        AllocationError(__FILE__, __LINE__, FALSE);
        goto done;
      }
      RUNSKey1.DIVISIONSrecordID = m_pDI->fileInfo.divisionRecordID;
      RUNSKey1.SERVICESrecordID = m_pDI->fileInfo.serviceRecordID;
      RUNSKey1.runNumber = NO_RECORD;
      RUNSKey1.pieceNumber = NO_RECORD;
      numRuns = 0;
      rcode2 = btrieve(B_GETGREATEROREQUAL, TMS_RUNS, &RUNS, &RUNSKey1, 1);
      while(rcode2 == 0 && 
            RUNS.DIVISIONSrecordID == m_pDI->fileInfo.divisionRecordID &&
            RUNS.SERVICESrecordID == m_pDI->fileInfo.serviceRecordID)
      {
        if(RUNS.pieceNumber == 1)
        {
          RUNSrecordIDs[numRuns] = RUNS.recordID;
          numRuns++;
        }
        rcode2 = btrieve(B_GETNEXT, TMS_RUNS, &RUNS, &RUNSKey1, 1);
      }
      for(nI = 0; nI < numRuns; nI++)
      {
        RUNSKey0.recordID = RUNSrecordIDs[nI];
        btrieve(B_GETEQUAL, TMS_RUNS, &RUNS, &RUNSKey0, 0);
        numPieces = GetRunElements(m_hWnd, &RUNS, &PROPOSEDRUN, &COST, TRUE);
        if(RUNSVIEW[0].runType == NO_RECORD)
        {
          RSPtr = m_bShowUnclassified ? &UNCLASSIFIED : NULL;
        }
        else
        {
          nJ = (short int)LOWORD(RUNSVIEW[0].runType);
          nK = (short int)HIWORD(RUNSVIEW[0].runType);
          if(COST.TOTAL.payTime == 0)
          {
            RSPtr = &NOTPAYINGOUT;
          }
          else
          {
            RSPtr = &RS[nJ][nK];
          }
        }
        if(RSPtr)
        {
          RSPtr->numRuns++;
          for(nJ = 0; nJ < numPieces; nJ++)
          {
            RSPtr->platformTime += RUNSVIEW[nJ].platformTime;
            RSPtr->reportTime += RUNSVIEW[nJ].reportTime;
            RSPtr->travelTime += RUNSVIEW[nJ].travelTime;
            RSPtr->turninTime += RUNSVIEW[nJ].turninTime;
          }
          RSPtr->overTime += RUNSVIEW[numPieces - 1].overTime;
          RSPtr->makeUpTime += RUNSVIEW[numPieces - 1].makeUpTime;
          RSPtr->spreadOvertime += RUNSVIEW[numPieces - 1].spreadOT;
          RSPtr->paidBreaks += RUNSVIEW[numPieces - 1].paidBreaks;
          RSPtr->payTime += RUNSVIEW[numPieces - 1].payTime;
          totalRuns++;
        }
      }
      TMSHeapFree(RUNSrecordIDs);
    }
  }
//
//  All done
//
  done:
    SetCursor(hCursor);

}

void CRunSummary::DoDisplay()
{
  int nI, nJ;

  numDisplayed = 0;

  pListCtrlLIST->DeleteAllItems();
  memset(&TOTAL, 0x00, sizeof(TOTAL));
//
//  Show anything with one or more runs
//
  pListCtrlLIST->SetItemCount(NUMRUNTYPES * NUMRUNTYPESLOTS + 1);
  for(nI = 0; nI < NUMRUNTYPES; nI++)
  { 
    for(nJ = 0; nJ < NUMRUNTYPESLOTS; nJ++)
    {
      if(RS[nI][nJ].numRuns == 0)
      {
        continue;
      }
      OutputRow(RUNTYPE[nI][nJ].localName, &RS[nI][nJ], FALSE);
      TOTAL.numRuns += RS[nI][nJ].numRuns;
      TOTAL.platformTime += RS[nI][nJ].platformTime;
      TOTAL.reportTime += RS[nI][nJ].reportTime;
      TOTAL.turninTime += RS[nI][nJ].turninTime;
      TOTAL.travelTime += RS[nI][nJ].travelTime;
      TOTAL.makeUpTime += RS[nI][nJ].makeUpTime;
      TOTAL.overTime += RS[nI][nJ].overTime;
      TOTAL.spreadOvertime += RS[nI][nJ].spreadOvertime;
      TOTAL.paidBreaks += RS[nI][nJ].paidBreaks;
      TOTAL.payTime += RS[nI][nJ].payTime;
    }
  }
//
//  Unclassified?
//
  if(m_bShowUnclassified)
  {
    OutputRow("Unclassified", &UNCLASSIFIED, FALSE);
    TOTAL.numRuns += UNCLASSIFIED.numRuns;
    TOTAL.platformTime += UNCLASSIFIED.platformTime;
    TOTAL.reportTime += UNCLASSIFIED.reportTime;
    TOTAL.turninTime += UNCLASSIFIED.turninTime;
    TOTAL.travelTime += UNCLASSIFIED.travelTime;
    TOTAL.makeUpTime += UNCLASSIFIED.makeUpTime;
    TOTAL.overTime += UNCLASSIFIED.overTime;
    TOTAL.spreadOvertime += UNCLASSIFIED.spreadOvertime;
    TOTAL.paidBreaks += UNCLASSIFIED.paidBreaks;
    TOTAL.payTime += UNCLASSIFIED.payTime;
  }
//
//  Totals
//
  OutputRow("", NULL, TRUE);
  OutputRow("Totals:", &TOTAL, FALSE);
//
//  Not paying out
//
  if(NOTPAYINGOUT.numRuns != 0)
  {
    OutputRow("Illegal", &NOTPAYINGOUT, FALSE);
  }
}

void RunSummaryDialog(void)
{
  CRunSummary dlg(NULL, NULL);

  dlg.DoModal();

}


extern "C"
{
  void RunSummary(void)
  {
    RunSummaryDialog();
  }
}

void CRunSummary::OnUnclassified() 
{
  m_bShowUnclassified = pButtonUNCLASSIFIED->GetCheck();
  DoDisplay();
}

void CRunSummary::OnPaytoplatform() 
{
  if(pButtonPAYTOPLATFORM->GetCheck())
  {
    m_Ratio = RUNSUMMARY_PAYTOPLATFORM;
  }
  DoDisplay();	
}

void CRunSummary::OnPlatformtopay() 
{
  if(pButtonPLATFORMTOPAY->GetCheck())
  {
    m_Ratio = RUNSUMMARY_PLATFORMTOPAY;	
  }
  DoDisplay();
}

void CRunSummary::OnReport() 
{
	m_bReport = pButtonREPORT->GetCheck();
  DoDisplay();
}

void CRunSummary::OnTravel() 
{
  m_bTravel = pButtonTRAVEL->GetCheck();
  DoDisplay();	
}

void CRunSummary::OnMakeup() 
{
  m_bMakeup = pButtonMAKEUP->GetCheck();
  DoDisplay();	
}

void CRunSummary::OnOvertime() 
{
  m_bOvertime = pButtonOVERTIME->GetCheck();
  DoDisplay();	
}

void CRunSummary::OnSpreadot() 
{
  m_bSpreadOT = pButtonSPREADOT->GetCheck();
  DoDisplay();
}

void CRunSummary::OnPaidbreaks() 
{
  m_bPaidBreaks = pButtonPAIDBREAKS->GetCheck();
  DoDisplay();
}

void CRunSummary::OnPrint() 
{
  PrintWindowToDC(this->m_hWnd, "Run Summary");
}


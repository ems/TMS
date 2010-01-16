// SuggestSuspension.cpp : implementation file
//

#include "stdafx.h"
extern "C"{
#include "TMSHeader.h"
}  // EXTERN C

#include "TMS.h"

#include "AddDialogs.h"
#include "AddDiscipline.h"
#include "Discipline.h"
#include "DisciplineLetter.h"
#include "SuggestSuspension.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSuggestSuspension dialog


CSuggestSuspension::CSuggestSuspension(CWnd* pParent /*=NULL*/, long* pDRIVERSrecordID, long* pStartDate)
	: CDialog(CSuggestSuspension::IDD, pParent)
{
  m_pStartDate = pStartDate;
  m_pDRIVERSrecordID = pDRIVERSrecordID;

	//{{AFX_DATA_INIT(CSuggestSuspension)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CSuggestSuspension::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSuggestSuspension)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSuggestSuspension, CDialog)
	//{{AFX_MSG_MAP(CSuggestSuspension)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDHELP, OnHelp)
	ON_NOTIFY(NM_DBLCLK, SUGGESTSUSPENSION_LIST, OnDblclkList)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSuggestSuspension message handlers

BOOL CSuggestSuspension::OnInitDialog() 
{
	CDialog::OnInitDialog();
//
//  Set up pointers to the controls
//
  pListCtrlLIST = (CListCtrl *)GetDlgItem(SUGGESTSUSPENSION_LIST);
  
  DWORD dwExStyles = pListCtrlLIST->GetExtendedStyle();
  pListCtrlLIST->SetExtendedStyle(dwExStyles | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_INFOTIP);
//
//  Show the hourgalss
//
  HCURSOR hSaveCursor = SetCursor(hCursorWait);
//
//  Set up the columns on the list control
//
//
//  Set up the list controls
//
  LVCOLUMN LVC;
  LVC.mask = LVCF_FMT | LVCF_TEXT | LVCF_WIDTH;

  LVC.cx = 120;
  LVC.fmt = LVCFMT_CENTER;
  LVC.pszText = "Date";
  pListCtrlLIST->InsertColumn(0, &LVC);

  LVC.cx = 60;
  LVC.pszText = "Run #";
  pListCtrlLIST->InsertColumn(1, &LVC);

  LVC.cx = 60;
  LVC.pszText = "On at";
  pListCtrlLIST->InsertColumn(2, &LVC);

  LVC.cx = 60;
  LVC.pszText = "Off at";
  pListCtrlLIST->InsertColumn(3, &LVC);
 
  LVC.cx = 60;
  LVC.pszText = "Pay";
  pListCtrlLIST->InsertColumn(4, &LVC);
//
//  Get his data from the roster
//
  typedef struct RosterPayStruct
  {
    int  num;
    long RUNSrecordIDs[10];
    long runNumber[10];
    long onTime[10];
    long offTime[10];
    long payTime[10];
    long flags[10];
  } RosterPayDef;

  RosterPayDef RD[7];

  int nI, nJ;

  for(nI = 0; nI < 7; nI++)
  {
    RD[nI].num = 0;
  }

  int rcode2;

  ROSTERKey1.DIVISIONSrecordID = m_DivisionRecordID;
  ROSTERKey1.rosterNumber = NO_RECORD;
  rcode2 = btrieve(B_GETGREATER, TMS_ROSTER, &ROSTER, &ROSTERKey1, 1);
  while(rcode2 == 0 &&
        ROSTER.DIVISIONSrecordID == m_DivisionRecordID)
  {
    if(ROSTER.DRIVERSrecordID == *m_pDRIVERSrecordID)
    {
      for(nI = 0; nI < 7; nI++)
      {
        if(ROSTER.WEEK[m_RosterWeek].RUNSrecordIDs[nI] == NO_RECORD)
        {
          continue;
        }
        nJ = RD[nI].num;
        RD[nI].RUNSrecordIDs[nJ] = ROSTER.WEEK[m_RosterWeek].RUNSrecordIDs[nI];
        RD[nI].flags[nJ] = ROSTER.WEEK[m_RosterWeek].flags;
        RD[nI].num++;
      }
    }
    rcode2 = btrieve(B_GETNEXT, TMS_ROSTER, &ROSTER, &ROSTERKey1, 1);
  }
//
//  Fill up the rest of the structure with run details
//
  PROPOSEDRUNDef PROPOSEDRUN;
  COSTDef COST;
  int     numPieces;

  for(nI = 0; nI < 7; nI++)
  {
    for(nJ = 0; nJ < RD[nI].num; nJ++)
    {
      RUNSKey0.recordID = RD[nI].RUNSrecordIDs[nJ];
      btrieve(B_GETEQUAL, TMS_RUNS, &RUNS, &RUNSKey0, 0);
      btrieve(B_GETPOSITION, TMS_RUNS, &RUNS, &RUNSKey0, 0);
      btrieve(B_GETDIRECT, TMS_RUNS, &RUNS, &RUNSKey1, 1);
      RD[nI].runNumber[nJ] = RUNS.runNumber;
      numPieces = GetRunElements(hWndMain, &RUNS, &PROPOSEDRUN, &COST, TRUE);
      RD[nI].onTime[nJ] = RUNSVIEW[0].runOnTime;
      RD[nI].offTime[nJ] = RUNSVIEW[numPieces - 1].runOffTime;
      RD[nI].payTime[nJ] = COST.TOTAL.payTime;
    }
  }
//
//  Start with today
//
  long yyyy, mm, dd;
  int indexToRoster[7] = {6, 0, 1, 2, 3, 4, 5};
  CTime firstDay = CTime::GetCurrentTime();
  long day = firstDay.GetDay();
  long month = firstDay.GetMonth();
  long year = firstDay.GetYear();
  long fDay = year * 10000 + month * 100 + day;
//
//  End with the "until date" on this pick
//
  GetYMD(untilDate, &yyyy, &mm, &dd);
  CTime lastDay(yyyy, mm, dd, 0, 0, 0);
//
//  Go through the assignments for each day
//
  CTime currentDate = firstDay;
  currentDate -= CTimeSpan(1, 0, 0, 0);
//
//  Display up until the untilDate
//
  LVITEM LVI;
  int    iItem;
  CString s;
  int today;

  for(;;)
  {
    currentDate += CTimeSpan(1, 0, 0, 0);
    day = currentDate.GetDay();
    month = currentDate.GetMonth();
    year = currentDate.GetYear();
    LVI.lParam = year * 10000 + month * 100 + day;
    if(LVI.lParam > untilDate)
    {
      break;
    }
    today = indexToRoster[currentDate.GetDayOfWeek() - 1];
 	  s = currentDate.Format(_T("%d-%b-%Y (%a)"));
//
//  Cycle through the number of Roster entries for today (typically 1)
//
    for(nI = 0; nI < RD[today].num; nI++)
    {
//
//  Output the date
//
      strcpy(tempString, s);
      if(IsHeAwayToday(LVI.lParam))
      {
        strcat(tempString, " *");
      }
      
      iItem = pListCtrlLIST->GetItemCount();

      LVI.mask = LVIF_TEXT | LVIF_PARAM;
      LVI.iItem = iItem;
      LVI.iSubItem = 0;
      LVI.pszText = tempString;
      LVI.iItem = pListCtrlLIST->InsertItem(&LVI);
//
//  Run number
//
      sprintf(tempString, "%ld", RD[today].runNumber[nI]);

      LVI.mask = LVIF_TEXT;
      LVI.iItem = iItem;
      LVI.iSubItem = 1;
      LVI.pszText = tempString;
      LVI.iItem = pListCtrlLIST->SetItem(&LVI);
//
//  On time
//
      strcpy(tempString, Tchar(RD[today].onTime[nI]));

      LVI.mask = LVIF_TEXT;
      LVI.iItem = iItem;
      LVI.iSubItem = 2;
      LVI.pszText = tempString;
      LVI.iItem = pListCtrlLIST->SetItem(&LVI);
//
//  Off time
//
      strcpy(tempString, Tchar(RD[today].offTime[nI]));

      LVI.mask = LVIF_TEXT;
      LVI.iItem = iItem;
      LVI.iSubItem = 3;
      LVI.pszText = tempString;
      LVI.iItem = pListCtrlLIST->SetItem(&LVI);
//
//  Pay time
//
      strcpy(tempString, chhmm(RD[today].payTime[nI]));

      LVI.mask = LVIF_TEXT;
      LVI.iItem = iItem;
      LVI.iSubItem = 4;
      LVI.pszText = tempString;
      LVI.iItem = pListCtrlLIST->SetItem(&LVI);
    }
  }
//
//  Restore the cursor
//
  SetCursor(hSaveCursor);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CSuggestSuspension::OnClose() 
{
  OnCancel();
}

void CSuggestSuspension::OnCancel() 
{
	// TODO: Add extra cleanup here
	
	CDialog::OnCancel();
}

void CSuggestSuspension::OnHelp() 
{
	// TODO: Add your control notification handler code here
	
}

void CSuggestSuspension::OnOK() 
{
	int nI = pListCtrlLIST->GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);

  if(nI < 0)
  {
    return;
  }

  LVITEM  LVI;
 
  LVI.mask = LVIF_PARAM;
  LVI.iItem = nI;
  LVI.iSubItem = 0;
  pListCtrlLIST->GetItem(&LVI);

  *m_pStartDate = LVI.lParam;

	CDialog::OnOK();
}

void CSuggestSuspension::OnDblclkList(NMHDR* pNMHDR, LRESULT* pResult) 
{
	*pResult = 0;

  OnOK();
}

BOOL CSuggestSuspension::IsHeAwayToday(long date)
{
//
//  See if he's away for a regularly scheduled date, or portion thereof
//
  CTime time = CTime::GetCurrentTime();
  long day = time.GetDay();
  long month = time.GetMonth();
  long year = time.GetYear();
  int  rcode2;

  DAILYOPSKey1.recordTypeFlag = DAILYOPS_FLAG_ABSENCE;
  DAILYOPSKey1.pertainsToDate = (year - 1) * 10000 + month * 100 + day;
  DAILYOPSKey1.pertainsToTime = 0;
  DAILYOPSKey1.recordFlags = 0;
  rcode2 = btrieve(B_GETGREATER, TMS_DAILYOPS, &DAILYOPS, &DAILYOPSKey1, 1);
  while(rcode2 == 0 &&
        (DAILYOPS.pertainsToDate <= date) &&
        (DAILYOPS.recordTypeFlag & DAILYOPS_FLAG_ABSENCE))
  {
//
//  Examine just those records with the date in question
//
    if(DAILYOPS.DRIVERSrecordID == *m_pDRIVERSrecordID &&
          date >= DAILYOPS.pertainsToDate &&
          date <= DAILYOPS.DOPS.Absence.untilDate &&
          (DAILYOPS.recordFlags & DAILYOPS_FLAG_ABSENCEREGISTER))
    {
      if(!ANegatedRecord(DAILYOPS.recordID, 1))
      {
        return(TRUE);
      }
    }
    rcode2 = btrieve(B_GETPREVIOUS, TMS_DAILYOPS, &DAILYOPS, &DAILYOPSKey1, 1);
  }
//
//  All done
//
  return(FALSE);
}

// DailyOpsSO.cpp : implementation file
//

#include "stdafx.h"
extern "C"{
#include "TMSHeader.h"
}  // EXTERN C

#include "tms.h"
#include "DailyOpsHeader.h"
#include "msmask.h"
#include "BitmapCtrl.h"
#include "AddDriver01.h"
#include "AddDriver02.h"
#include "AddDriver03.h"
#include "AddDriver04.h"
#include "AddDriverPropSheet.h"
#include "AddDriver.h"

int LocateIndexes(long SERVICESrecordID, long RUNSrecordID, long flags, int *pServiceIndex, int *pRunIndex);

#define AVAILABLE_FLAG_UNASSIGNED       0x0001
#define AVAILABLE_FLAG_STANDBY          0x0002
#define AVAILABLE_FLAG_DAYOFF           0x0004
#define AVAILABLE_FLAG_BEFORE           0x0008
#define AVAILABLE_FLAG_AFTER            0x0010
#define AVAILABLE_FLAG_INELIGIBLE       0x0020
#define AVAILABLE_FLAG_DAYTIME          0x0040
#define AVAILABLE_FLAG_NIGHTTIME        0x0080
#define AVAILABLE_FLAG_NOTENOUGHOFFTIME 0x0100
#define AVAILABLE_FLAG_ABSENT           0x0200
#define AVAILABLE_FLAG_DEASSIGNED       0x0400

//
//  Sort function to the "Available Drivers" ComboBox
//
extern "C"
{
int sort_AvailableDrivers(const void *a, const void *b)
{
  AvailableDriversDef *pa, *pb;
  pa = (AvailableDriversDef *)a;
  pb = (AvailableDriversDef *)b;

  if(pa->seniorityDate != pb->seniorityDate)
  {
    return(pa->seniorityDate < pb->seniorityDate ? -1 : pa->seniorityDate > pb->seniorityDate ? 1 : 0);
  }
  else
  {
    return(pa->senioritySort < pb->senioritySort ? -1 : pa->senioritySort > pb->senioritySort ? 1 : 0);
  }
}
int eliminate_AvailableDrivers(const void *a, const void *b)
{
  AvailableDriversDef *pa, *pb;
  pa = (AvailableDriversDef *)a;
  pb = (AvailableDriversDef *)b;

  return(pa->DRIVERSrecordID > pb->DRIVERSrecordID ? -1 : pa->DRIVERSrecordID < pb->DRIVERSrecordID ? 1 : 0);
}
}
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static BOOL bForwardSort[7];

int CALLBACK DOSOListViewCompareProc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
  AvailableDriversDef *p1 = (AvailableDriversDef *)lParam1;
  AvailableDriversDef *p2 = (AvailableDriversDef *)lParam2;
  int  iResult = 0;

  if(p1 && p2)
  {
    switch(lParamSort)
    {
      case 0:  // Operator
        iResult = bForwardSort[lParamSort] ?
              lstrcmp(p1->szDriver, p2->szDriver) :
              lstrcmp(p2->szDriver, p1->szDriver);
        break;

      case 1:  // Status flag
        iResult = bForwardSort[lParamSort] ? 
              lstrcmp(p1->szDriverType, p2->szDriverType) :
              lstrcmp(p2->szDriverType, p1->szDriverType);
        break;

      case 2:  // Status flag
        iResult = bForwardSort[lParamSort] ? 
              lstrcmp(p1->szFlags, p2->szFlags) :
              lstrcmp(p2->szFlags, p1->szFlags);
        break;

      case 3:  // Seniority Date
        if(bForwardSort[lParamSort])
        {
          if(p1->seniorityDate != p2->seniorityDate)
          {
            iResult = (p1->seniorityDate < p2->seniorityDate ? -1 :
                       p1->seniorityDate > p2->seniorityDate ? 1 : 0);
          }
          else
          {
            iResult = (p1->senioritySort < p2->senioritySort ? -1 :
                       p1->senioritySort > p2->senioritySort ? 1 : 0);
          }
        }
        else
        {
          if(p1->seniorityDate != p2->seniorityDate)
          {
            iResult = (p1->seniorityDate > p2->seniorityDate ? -1 :
                       p1->seniorityDate < p2->seniorityDate ? 1 : 0);
          }
          else
          {
            iResult = (p1->senioritySort > p2->senioritySort ? -1 :
                       p1->senioritySort < p2->senioritySort ? 1 : 0);
          }
        }
        break;

      case 4:  // Rostered
        if(bForwardSort[lParamSort])
        {
          iResult = (p1->rosteredTime < p2->rosteredTime ? -1 :
                     p1->rosteredTime > p2->rosteredTime ? 1 : 0);
        }
        else
        {
          iResult = (p1->rosteredTime > p2->rosteredTime ? -1 :
                     p1->rosteredTime < p2->rosteredTime ? 1 : 0);
        }
        break;

      case 5:  // Assigned
        if(bForwardSort[lParamSort])
        {
          iResult = (p1->openWorkTime < p2->openWorkTime ? -1 :
                     p1->openWorkTime > p2->openWorkTime ? 1 : 0);
        }
        else
        {
          iResult = (p1->openWorkTime > p2->openWorkTime ? -1 :
                     p1->openWorkTime < p2->openWorkTime ? 1 : 0);
        }
        break;

      case 6:  // Total
        if(bForwardSort[lParamSort])
        {
          iResult = (p1->totalTime < p2->totalTime ? -1 :
                     p1->totalTime > p2->totalTime ? 1 : 0);
        }
        else
        {
          iResult = (p1->totalTime > p2->totalTime ? -1 :
                     p1->totalTime < p2->totalTime ? 1 : 0);
        }
        break;
    }
  }

  return(iResult);
}
/////////////////////////////////////////////////////////////////////////////
// CDailyOpsSO dialog


CDailyOpsSO::CDailyOpsSO(CWnd* pParent /*=NULL*/,
                         DailyOpsSOPassedDataDef* pPassedData)
	: CDialog(CDailyOpsSO::IDD, pParent)
{
  m_pPassedData = pPassedData;
  m_Index = NO_RECORD;

	//{{AFX_DATA_INIT(CDailyOpsSO)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CDailyOpsSO::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDailyOpsSO)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDailyOpsSO, CDialog)
	//{{AFX_MSG_MAP(CDailyOpsSO)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDHELP, OnHelp)
	ON_NOTIFY(NM_CLICK, DAILYOPSSO_LIST, OnClickList)
	ON_NOTIFY(NM_DBLCLK, DAILYOPSSO_LIST, OnDblclkList)
	ON_NOTIFY(LVN_COLUMNCLICK, DAILYOPSSO_LIST, OnColumnclickList)
	ON_BN_CLICKED(IDPRINT, OnPrint)
	ON_NOTIFY(NM_RCLICK, DAILYOPSSO_LIST, OnRclickList)
	ON_BN_CLICKED(DAILYOPSSO_SHOWINELIGIBLE, OnShowineligible)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDailyOpsSO message handlers

BOOL CDailyOpsSO::OnInitDialog() 
{
	CDialog::OnInitDialog();

  HCURSOR hCursorSave = SetCursor(hCursorWait);
//
//  Set up pointers to the controls
//
  pStaticDESCRIPTION = (CStatic *)GetDlgItem(DAILYOPSSO_DESCRIPTION);
  pListCtrlLIST = (CListCtrl *)GetDlgItem(DAILYOPSSO_LIST);
  pButtonSHOWINELIGIBLE = (CButton *)GetDlgItem(DAILYOPSSO_SHOWINELIGIBLE);
//
//  Default to not showing ineligible drivers
//
  pButtonSHOWINELIGIBLE->SetCheck(FALSE);
//
//  Display the description of what we're filling
//
  if(m_pPassedData->flags & DAILYOPSSO_FLAGS_RUNSPLIT)
  {
    sprintf(tempString, "This portion of run %ld starts at %s, finishes at ", m_pPassedData->runNumber, Tchar(m_pPassedData->runStartTime));
  }
  else
  {
    sprintf(tempString, "Run %ld starts at %s, finishes at ", m_pPassedData->runNumber, Tchar(m_pPassedData->runStartTime));
  }
  strcat(tempString, Tchar(m_pPassedData->runEndTime));
  strcat(tempString, ", and pays ");
  strcat(tempString, (m_pPassedData->payTime == NO_TIME ? "TBD" : chhmm(m_pPassedData->payTime)));
  pStaticDESCRIPTION->SetWindowText(tempString);
//
//  Set the sort flags
//
  int nI, nJ, nK;

  for(nI = 0; nI < 5; nI++)
  {
    bForwardSort[nI] = TRUE;
  }
//
//  Get the open work monitor date
//
  long yyyy, mm, dd;

  GetYMD(m_pPassedData->date, &yyyy, &mm, &dd);
    
  CTime workDate(yyyy, mm, dd, 0, 0, 0);

  int  today;
  long SERVICESrecordID = CDailyOps::DetermineServiceDay(m_pPassedData->date, FALSE, &today, tempString);
  int  serviceIndex, runIndex;
  long flags;

  CString s;

  GetWindowText(szFormatString, SZFORMATSTRING_LENGTH);
  sprintf(tempString, szFormatString, workDate.Format("%A, %B %d, %Y"));
  SetWindowText(tempString);
//
//  Set up the list control
//
  DWORD dwExStyles = pListCtrlLIST->GetExtendedStyle();
  pListCtrlLIST->SetExtendedStyle(dwExStyles | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_INFOTIP);

  LVCOLUMN LVC;
  LVC.mask = LVCF_FMT | LVCF_TEXT | LVCF_WIDTH;

  LVC.cx = 160;
  if(!(m_pPassedData->flags & DAILYOPSSO_FLAGS_SHOWHOURS))
  {
    LVC.cx += 65;
  }
  LVC.fmt = LVCFMT_LEFT;
  LVC.pszText = "Operator";
  pListCtrlLIST->InsertColumn(0, &LVC);

  LVC.cx = 60;
  if(!(m_pPassedData->flags & DAILYOPSSO_FLAGS_SHOWHOURS))
  {
    LVC.cx += 65;
  }
  LVC.pszText = "Position";
  pListCtrlLIST->InsertColumn(1, &LVC);

  LVC.cx = 260;
  if(!(m_pPassedData->flags & DAILYOPSSO_FLAGS_SHOWHOURS))
  {
    LVC.cx += 65;
  }
  LVC.pszText = "Status";
  pListCtrlLIST->InsertColumn(2, &LVC);

  LVC.cx = 80;
  LVC.pszText = "Seniority";
  pListCtrlLIST->InsertColumn(3, &LVC);
  
  if(m_pPassedData->flags & DAILYOPSSO_FLAGS_SHOWHOURS)
  {
    LVC.cx = 65;
    LVC.fmt = LVCFMT_LEFT;
    LVC.pszText = "Rostered";
    pListCtrlLIST->InsertColumn(4, &LVC);

    LVC.cx = 65;
    LVC.fmt = LVCFMT_LEFT;
    LVC.pszText = "Assigned";
    pListCtrlLIST->InsertColumn(5, &LVC);

    LVC.cx = 65;
    LVC.fmt = LVCFMT_LEFT;
    LVC.pszText = "Total";
    pListCtrlLIST->InsertColumn(6, &LVC);
  }

//
//  Get the operators who aren't working today
//
  num_AvailableDrivers = 0;

  BOOL bFound;
  int  rcode2;
//
//  Unassigned drivers first
//
  rcode2 = btrieve(B_GETFIRST, TMS_DRIVERS, &DRIVERS, &DRIVERSKey2, 2);
  while(rcode2 == 0)
  {
    if(ConsideringThisDriverType(DRIVERS.DRIVERTYPESrecordID))
    {
      bFound = FALSE;
      ROSTERKey2.DRIVERSrecordID = DRIVERS.recordID;
      rcode2 = btrieve(B_GETEQUAL, TMS_ROSTER, &ROSTER, &ROSTERKey2, 2);
      while(rcode2 == 0 &&
            ROSTER.DRIVERSrecordID == DRIVERS.recordID)
      {
        if(ROSTER.DIVISIONSrecordID == m_DailyOpsROSTERDivisionInEffect)
        {
          bFound = TRUE;
          break;
        }
        rcode2 = btrieve(B_GETNEXT, TMS_ROSTER, &ROSTER, &ROSTERKey2, 2);
      }
      if(!bFound)
      {
        m_AvailableDrivers[num_AvailableDrivers].DRIVERSrecordID = DRIVERS.recordID;
        if(DRIVERS.flags & DRIVERS_FLAG_DAYTIMEEXTRABOARD)
        {
          m_AvailableDrivers[num_AvailableDrivers].flags = AVAILABLE_FLAG_DAYTIME;
        }
        else if(DRIVERS.flags & DRIVERS_FLAG_NIGHTTIMEEXTRABOARD)
        {
          m_AvailableDrivers[num_AvailableDrivers].flags = AVAILABLE_FLAG_NIGHTTIME;
        }
        else
        {
          m_AvailableDrivers[num_AvailableDrivers].flags = AVAILABLE_FLAG_UNASSIGNED;
        }
        num_AvailableDrivers++;
      }
    }
    rcode2 = btrieve(B_GETNEXT, TMS_DRIVERS, &DRIVERS, &DRIVERSKey2, 2);
  }
//
//  Those on crewonly/standby next - go through the roster
//
  ROSTERKey1.DIVISIONSrecordID = m_DailyOpsROSTERDivisionInEffect;
  ROSTERKey1.rosterNumber = NO_RECORD;
  rcode2 = btrieve(B_GETGREATER, TMS_ROSTER, &ROSTER, &ROSTERKey1, 1);
  while(rcode2 == 0 &&
        ROSTER.DIVISIONSrecordID == m_DailyOpsROSTERDivisionInEffect)
  {
    if(ROSTER.DRIVERSrecordID != NO_RECORD &&
          ROSTER.WEEK[m_RosterWeek].RUNSrecordIDs[today] != NO_RECORD && ROSTER.WEEK[m_RosterWeek].flags & (1 << today))
    {
      DRIVERSKey0.recordID = ROSTER.DRIVERSrecordID;
      btrieve(B_GETEQUAL, TMS_DRIVERS, &DRIVERS, &DRIVERSKey0, 0);
      if(ConsideringThisDriverType(DRIVERS.DRIVERTYPESrecordID))
      {
        bFound = FALSE;
        for(nI = 0; nI < num_AvailableDrivers; nI++)
        {
          if(ROSTER.DRIVERSrecordID == m_AvailableDrivers[nI].DRIVERSrecordID)
          {
            bFound = TRUE;
            break;
          }
        }
        if(!bFound)
        {
          m_AvailableDrivers[num_AvailableDrivers].DRIVERSrecordID = ROSTER.DRIVERSrecordID;
          if(DRIVERS.flags & DRIVERS_FLAG_DAYTIMEEXTRABOARD)
          {
            m_AvailableDrivers[num_AvailableDrivers].flags = AVAILABLE_FLAG_DAYTIME;
          }
          else if(DRIVERS.flags & DRIVERS_FLAG_NIGHTTIMEEXTRABOARD)
          {
            m_AvailableDrivers[num_AvailableDrivers].flags = AVAILABLE_FLAG_NIGHTTIME;
          }
          else
          {
            m_AvailableDrivers[num_AvailableDrivers].flags = AVAILABLE_FLAG_STANDBY;
          }
          num_AvailableDrivers++;
        }
      }
    }
    rcode2 = btrieve(B_GETNEXT, TMS_ROSTER, &ROSTER, &ROSTERKey1, 1);
  }
//
//  Drivers with the day off
//
  rcode2 = btrieve(B_GETFIRST, TMS_DRIVERS, &DRIVERS, &DRIVERSKey2, 2);
  while(rcode2 == 0)
  {
    if(ConsideringThisDriverType(DRIVERS.DRIVERTYPESrecordID))
    {
      bFound = FALSE;
      ROSTERKey2.DRIVERSrecordID = DRIVERS.recordID;
      rcode2 = btrieve(B_GETEQUAL, TMS_ROSTER, &ROSTER, &ROSTERKey2, 2);
      while(rcode2 == 0 &&
            ROSTER.DRIVERSrecordID == DRIVERS.recordID)
      {
        if(ROSTER.DIVISIONSrecordID == m_DailyOpsROSTERDivisionInEffect)
        {
          if(ROSTER.WEEK[m_RosterWeek].RUNSrecordIDs[today] != NO_RECORD)
          {
            bFound = TRUE;
            break;
          }
        }
        rcode2 = btrieve(B_GETNEXT, TMS_ROSTER, &ROSTER, &ROSTERKey2, 2);
      }
//
//  Don't add them if they're already in the list
//
      if(!bFound)
      {
        for(nI = 0; nI < num_AvailableDrivers; nI++)
        {
          if(m_AvailableDrivers[nI].DRIVERSrecordID == DRIVERS.recordID)
          {
            bFound = TRUE;
            break;
          }
        }
        if(!bFound)
        {
          m_AvailableDrivers[num_AvailableDrivers].DRIVERSrecordID = DRIVERS.recordID;
          m_AvailableDrivers[num_AvailableDrivers].flags = AVAILABLE_FLAG_DAYOFF;
          num_AvailableDrivers++;
        }
      }
    }
    rcode2 = btrieve(B_GETNEXT, TMS_DRIVERS, &DRIVERS, &DRIVERSKey2, 2);
  }
//
//  Now those that finish before the run in question starts
//
  ROSTERKey1.DIVISIONSrecordID = m_DailyOpsROSTERDivisionInEffect;
  ROSTERKey1.rosterNumber = NO_RECORD;
  rcode2 = btrieve(B_GETGREATER, TMS_ROSTER, &ROSTER, &ROSTERKey1, 1);
  while(rcode2 == 0 &&
        ROSTER.DIVISIONSrecordID == m_DailyOpsROSTERDivisionInEffect)
  {
    if(ROSTER.DRIVERSrecordID != NO_RECORD &&
          ROSTER.WEEK[m_RosterWeek].RUNSrecordIDs[today] != NO_RECORD)
    {
      DRIVERSKey0.recordID = ROSTER.DRIVERSrecordID;
      rcode2 = btrieve(B_GETEQUAL, TMS_DRIVERS, &DRIVERS, &DRIVERSKey0, 0);
      if(rcode2 == 0 && ConsideringThisDriverType(DRIVERS.DRIVERTYPESrecordID))
      {
        flags = (ROSTER.WEEK[m_RosterWeek].flags & (1 << today)) ? RDATA_FLAG_CREWONLY : 0;
        rcode2 = LocateIndexes(SERVICESrecordID, ROSTER.WEEK[m_RosterWeek].RUNSrecordIDs[today], flags, &serviceIndex, &runIndex);
        if(rcode2 == 0)
        {
          if(m_pIncoreRuns[serviceIndex].pRuns[runIndex].endTime <= m_pPassedData->runStartTime)
          {
            bFound = FALSE;
            for(nI = 0; nI < num_AvailableDrivers; nI++)
            {
              if(ROSTER.DRIVERSrecordID == m_AvailableDrivers[nI].DRIVERSrecordID)
              {
                bFound = TRUE;
                break;
              }
            }
            if(!bFound)
            {
              m_AvailableDrivers[num_AvailableDrivers].DRIVERSrecordID = ROSTER.DRIVERSrecordID;
              m_AvailableDrivers[num_AvailableDrivers].flags = AVAILABLE_FLAG_BEFORE;
              m_AvailableDrivers[num_AvailableDrivers].time[0] = m_pIncoreRuns[serviceIndex].pRuns[runIndex].endTime;
              m_AvailableDrivers[num_AvailableDrivers].runNumber = m_pIncoreRuns[serviceIndex].pRuns[runIndex].runNumber;
              num_AvailableDrivers++;
            }
          }
          else
          {
            bFound = FALSE;
            for(nI = 0; nI < num_AvailableDrivers; nI++)
            {
              if(ROSTER.DRIVERSrecordID == m_AvailableDrivers[nI].DRIVERSrecordID)
              {
                bFound = TRUE;
                break;
              }
            }
            if(!bFound)
            {
              m_AvailableDrivers[num_AvailableDrivers].DRIVERSrecordID = ROSTER.DRIVERSrecordID;
              m_AvailableDrivers[num_AvailableDrivers].flags = AVAILABLE_FLAG_INELIGIBLE;
              m_AvailableDrivers[num_AvailableDrivers].time[0] = m_pIncoreRuns[serviceIndex].pRuns[runIndex].startTime;
              m_AvailableDrivers[num_AvailableDrivers].time[1] = m_pIncoreRuns[serviceIndex].pRuns[runIndex].endTime;
              m_AvailableDrivers[num_AvailableDrivers].runNumber = m_pIncoreRuns[serviceIndex].pRuns[runIndex].runNumber;
              num_AvailableDrivers++;
            }
          }
        }
      }
    }
    rcode2 = btrieve(B_GETNEXT, TMS_ROSTER, &ROSTER, &ROSTERKey1, 1);
  }
//
//  Now those that start after the run in question finishes
//
  ROSTERKey1.DIVISIONSrecordID = m_DailyOpsROSTERDivisionInEffect;
  ROSTERKey1.rosterNumber = NO_RECORD;
  rcode2 = btrieve(B_GETGREATER, TMS_ROSTER, &ROSTER, &ROSTERKey1, 1);
  while(rcode2 == 0 &&
        ROSTER.DIVISIONSrecordID == m_DailyOpsROSTERDivisionInEffect)
  {
    if(ROSTER.DRIVERSrecordID != NO_RECORD &&
          ROSTER.WEEK[m_RosterWeek].RUNSrecordIDs[today] != NO_RECORD)
    {
      DRIVERSKey0.recordID = ROSTER.DRIVERSrecordID;
      rcode2 = btrieve(B_GETEQUAL, TMS_DRIVERS, &DRIVERS, &DRIVERSKey0, 0);
      if(rcode2 == 0 && ConsideringThisDriverType(DRIVERS.DRIVERTYPESrecordID))
      {
        flags = (ROSTER.WEEK[m_RosterWeek].flags & (1 << today)) ? RDATA_FLAG_CREWONLY : 0;
        rcode2 = LocateIndexes(SERVICESrecordID, ROSTER.WEEK[m_RosterWeek].RUNSrecordIDs[today], flags, &serviceIndex, &runIndex);
        if(rcode2 == 0)
        {
          if(m_pIncoreRuns[serviceIndex].pRuns[runIndex].startTime >= m_pPassedData->runEndTime)
          {
            bFound = FALSE;
            for(nI = 0; nI < num_AvailableDrivers; nI++)
            {
              if(ROSTER.DRIVERSrecordID == m_AvailableDrivers[nI].DRIVERSrecordID)
              {
                m_AvailableDrivers[nI].flags = AVAILABLE_FLAG_AFTER;
                bFound = TRUE;
                break;
              }
            }
            if(!bFound)
            {
              m_AvailableDrivers[num_AvailableDrivers].DRIVERSrecordID = ROSTER.DRIVERSrecordID;
              m_AvailableDrivers[num_AvailableDrivers].flags = AVAILABLE_FLAG_AFTER;
              m_AvailableDrivers[num_AvailableDrivers].time[0] = m_pIncoreRuns[serviceIndex].pRuns[runIndex].startTime;
              m_AvailableDrivers[num_AvailableDrivers].runNumber = m_pIncoreRuns[serviceIndex].pRuns[runIndex].runNumber;
              num_AvailableDrivers++;
            }
          }
          else
          {
            bFound = FALSE;
            for(nI = 0; nI < num_AvailableDrivers; nI++)
            {
              if(ROSTER.DRIVERSrecordID == m_AvailableDrivers[nI].DRIVERSrecordID)
              {
                bFound = TRUE;
                break;
              }
            }
            if(!bFound)
            {
              m_AvailableDrivers[num_AvailableDrivers].DRIVERSrecordID = ROSTER.DRIVERSrecordID;
              m_AvailableDrivers[num_AvailableDrivers].flags = AVAILABLE_FLAG_INELIGIBLE;
              m_AvailableDrivers[num_AvailableDrivers].time[0] = m_pIncoreRuns[serviceIndex].pRuns[runIndex].startTime;
              m_AvailableDrivers[num_AvailableDrivers].time[1] = m_pIncoreRuns[serviceIndex].pRuns[runIndex].endTime;
              m_AvailableDrivers[num_AvailableDrivers].runNumber = m_pIncoreRuns[serviceIndex].pRuns[runIndex].runNumber;
              num_AvailableDrivers++;
            }
          }
        }
      }
    }
    rcode2 = btrieve(B_GETNEXT, TMS_ROSTER, &ROSTER, &ROSTERKey1, 1);
  }
//
//  Look for any deassignments.
//
  DAILYOPSKey1.recordTypeFlag = DAILYOPS_FLAG_OPERATOR;
  DAILYOPSKey1.pertainsToDate = m_pPassedData->date;
  DAILYOPSKey1.pertainsToTime = 0;
  DAILYOPSKey1.recordFlags = 0;
  bFound = FALSE;
  rcode2 = btrieve(B_GETGREATEROREQUAL, TMS_DAILYOPS, &DAILYOPS, &DAILYOPSKey1, 1);
  while(rcode2 == 0 &&
        (DAILYOPS.recordTypeFlag & DAILYOPS_FLAG_OPERATOR) &&
         DAILYOPS.pertainsToDate == m_pPassedData->date)
  {
    if(DAILYOPS.recordFlags & DAILYOPS_FLAG_OPERATORDEASSIGN)
    {
//
//  Add to the list?
//
      if(!ANegatedRecord(DAILYOPS.recordID, 1))
      {
        DRIVERSKey0.recordID = DAILYOPS.DRIVERSrecordID;
        rcode2 = btrieve(B_GETEQUAL, TMS_DRIVERS, &DRIVERS, &DRIVERSKey0, 0);
        if(DRIVERS.recordID == 9)
        {
          int xx = 1;
        }
        if(rcode2 == 0 && ConsideringThisDriverType(DRIVERS.DRIVERTYPESrecordID))
        {
          for(nI = 0; nI < num_AvailableDrivers; nI++)
          {
            if(m_AvailableDrivers[nI].DRIVERSrecordID == DAILYOPS.DRIVERSrecordID)
            {
              m_AvailableDrivers[nI].flags = AVAILABLE_FLAG_UNASSIGNED;
              bFound = TRUE;
              break;
            }
          }
          if(!bFound)
          {
            m_AvailableDrivers[num_AvailableDrivers].DRIVERSrecordID = DAILYOPS.DRIVERSrecordID;
            m_AvailableDrivers[num_AvailableDrivers].flags = AVAILABLE_FLAG_UNASSIGNED;
            num_AvailableDrivers++;
          }
        }
      }
    }
    rcode2 = btrieve(B_GETNEXT, TMS_DAILYOPS, &DAILYOPS, &DAILYOPSKey1, 1);
  }
//
//  Go through the eligible list to ensure minimum off time isn't violated
//
  long previousSignOff;
  long tempLong;
  long yesterday;
  long yesterdaySERVICESrecordID;
  long dateToUse;

  yesterday = today - 1;
  if(today < 0)
  {
    today = 6;
  }

  GetYMD(m_pPassedData->date, &yyyy, &mm, &dd);
  
  CTimeSpan oneDay(1, 0, 0, 0);
    
  CTime yesterdayDate(yyyy, mm, dd, 0, 0, 0);

  yesterdayDate -= oneDay;
  yyyy = yesterdayDate.GetYear();
  mm   = yesterdayDate.GetMonth();
  dd   = yesterdayDate.GetDay();
  dateToUse = (yyyy * 10000) + (mm * 100) + dd;

  for(nI = 0; nI < num_AvailableDrivers; nI++)
  {
    if(m_AvailableDrivers[nI].flags & AVAILABLE_FLAG_INELIGIBLE)
    {
      continue;
    }
    if(m_AvailableDrivers[nI].DRIVERSrecordID == 9)
    {
      int xx = 1;
    }
//
//  What did he do yesterday?
//
    previousSignOff = NO_TIME;
    ROSTERKey2.DRIVERSrecordID = m_AvailableDrivers[nI].DRIVERSrecordID;
    rcode2 = btrieve(B_GETEQUAL, TMS_ROSTER, &ROSTER, &ROSTERKey2, 2);
    while(rcode2 == 0 && 
          ROSTER.DRIVERSrecordID == m_AvailableDrivers[nI].DRIVERSrecordID)
    {
      if(ROSTER.DIVISIONSrecordID == m_DailyOpsROSTERDivisionInEffect)
      {
        flags = (ROSTER.WEEK[m_RosterWeek].flags & (1 << yesterday)) ? RDATA_FLAG_CREWONLY : 0;
        rcode2 = LocateIndexes(SERVICESrecordID,
              ROSTER.WEEK[m_RosterWeek].RUNSrecordIDs[yesterday], flags, &serviceIndex, &runIndex);
        if(rcode2 == 0)
        {
          if(previousSignOff == NO_TIME)
          {
            previousSignOff = m_pIncoreRuns[serviceIndex].pRuns[runIndex].endTime;
          }
          else
          {
            if(previousSignOff < m_pIncoreRuns[serviceIndex].pRuns[runIndex].endTime)
            {
              previousSignOff = m_pIncoreRuns[serviceIndex].pRuns[runIndex].endTime;
            }
          }
        } 
      }
      rcode2 = btrieve(B_GETNEXT, TMS_ROSTER, &ROSTER, &ROSTERKey2, 2);
    }
//
//  Was any work assigned in DailyOps?
//
    DAILYOPSKey1.recordTypeFlag = DAILYOPS_FLAG_OPENWORK;
    DAILYOPSKey1.pertainsToDate = dateToUse;
//    DAILYOPSKey1.pertainsToTime = 999999;
//    DAILYOPSKey1.recordFlags = 255;
//    rcode2 = btrieve(B_GETLESSTHAN, TMS_DAILYOPS, &DAILYOPS, &DAILYOPSKey1, 1);
    DAILYOPSKey1.pertainsToTime = NO_RECORD;
    DAILYOPSKey1.recordFlags = NO_RECORD;
    rcode2 = btrieve(B_GETGREATER, TMS_DAILYOPS, &DAILYOPS, &DAILYOPSKey1, 1);
    while(rcode2 == 0 &&
          (DAILYOPS.recordTypeFlag & DAILYOPS_FLAG_OPENWORK) &&
           DAILYOPS.pertainsToDate == dateToUse)
    {
      if(DAILYOPS.recordFlags & DAILYOPS_FLAG_OPENWORKASSIGN)
      {
        if(!ANegatedRecord(DAILYOPS.recordID, 1))
        {
          if(DAILYOPS.DRIVERSrecordID == m_AvailableDrivers[nI].DRIVERSrecordID)
          {
            if(DAILYOPS.DOPS.OpenWork.splitStartNODESrecordID > 0)
            { 
              if(previousSignOff == NO_TIME)
              {
                previousSignOff = DAILYOPS.DOPS.OpenWork.splitEndTime;
              }
              else
              {
                if(previousSignOff < DAILYOPS.DOPS.OpenWork.splitEndTime)
                {
                  previousSignOff = DAILYOPS.DOPS.OpenWork.splitEndTime;
                }
              }
            }
            else
            {
              flags = (DAILYOPS.recordFlags & DAILYOPS_FLAG_CREWONLY) ? RDATA_FLAG_CREWONLY : 0;
              if(flags)
              {
                CREWONLYKey0.recordID = DAILYOPS.DOPS.OpenWork.RUNSrecordID;
                rcode2 = btrieve(B_GETEQUAL, TMS_CREWONLY, &CREWONLY, &CREWONLYKey0, 0);
                if(rcode2 == 0)
                {
                  yesterdaySERVICESrecordID = CREWONLY.SERVICESrecordID;
                }
              }
              else
              {
                RUNSKey0.recordID = DAILYOPS.DOPS.OpenWork.RUNSrecordID;
                rcode2 = btrieve(B_GETEQUAL, TMS_RUNS, &RUNS, &RUNSKey0, 0);
                if(rcode2 == 0)
                {
                  yesterdaySERVICESrecordID = RUNS.SERVICESrecordID;
                }
              }
              rcode2 = LocateIndexes(yesterdaySERVICESrecordID,
                    DAILYOPS.DOPS.OpenWork.RUNSrecordID, flags, &serviceIndex, &runIndex);
              if(rcode2 == 0)
              {
                if(previousSignOff == NO_TIME)
                {
                  previousSignOff = m_pIncoreRuns[serviceIndex].pRuns[runIndex].endTime;
                }
                else
                {
                  if(previousSignOff < m_pIncoreRuns[serviceIndex].pRuns[runIndex].endTime)
                  {
                    previousSignOff = m_pIncoreRuns[serviceIndex].pRuns[runIndex].endTime;
                  }
                }
              }
            }
          }
        }
      }
//      rcode2 = btrieve(B_GETPREVIOUS, TMS_DAILYOPS, &DAILYOPS, &DAILYOPSKey1, 1);             
      rcode2 = btrieve(B_GETNEXT, TMS_DAILYOPS, &DAILYOPS, &DAILYOPSKey1, 1);             
    }
//
//  Compare that to the signon time
//
    if(previousSignOff == NO_TIME)
    {
      continue;
    }
    tempLong = 86400L + m_pPassedData->runStartTime - previousSignOff;
    if(tempLong < ROSTERPARMS.minOffTime)
    {
      m_AvailableDrivers[nI].flags = AVAILABLE_FLAG_NOTENOUGHOFFTIME;
      m_AvailableDrivers[nI].time[0] = tempLong;
    }
  }
//
//  Finally, nuke anyone who's absent
//
  for(nI = 0; nI < num_AvailableDrivers; nI++)
  {
    if(m_AvailableDrivers[nI].DRIVERSrecordID == 9)
    {
      int xx = 1;
    }
    for(nJ = 0; nJ < m_pPassedData->numAbsent; nJ++)
    {
      if(m_pPassedData->pAbsentList[nJ].DRIVERSrecordID == m_AvailableDrivers[nI].DRIVERSrecordID)
      {
        bFound = FALSE;
        if(m_pPassedData->pAbsentList[nJ].fromTime < m_pPassedData->runStartTime)
        {
          if(m_pPassedData->pAbsentList[nJ].toTime > m_pPassedData->runStartTime)
          {
            bFound = TRUE;
          }
        }
        else
        {
          if(m_pPassedData->pAbsentList[nJ].fromTime < m_pPassedData->runEndTime)
          {
            bFound = TRUE;
          }
        }
        if(bFound)
        {
          m_AvailableDrivers[nI].flags = AVAILABLE_FLAG_ABSENT;
          m_AvailableDrivers[nI].time[0] = m_pPassedData->pAbsentList[nJ].fromTime;
          m_AvailableDrivers[nI].time[1] = m_pPassedData->pAbsentList[nJ].toTime;
          break;
        }
      }
    }
  }
//
//  If anyone was marked absent (for any period of time outside this run's
//  start/end time) take them off their roster for the rest of the day
//
  for(nI = 0; nI < num_AvailableDrivers; nI++)
  {
    for(nJ = 0; nJ < m_pPassedData->numAbsent; nJ++)
    {
      if(m_pPassedData->pAbsentList[nJ].DRIVERSrecordID == m_AvailableDrivers[nI].DRIVERSrecordID)
      {
        if(!(m_AvailableDrivers[nI].flags & AVAILABLE_FLAG_ABSENT))
        {
          m_AvailableDrivers[nI].flags = AVAILABLE_FLAG_DEASSIGNED;
        }
      }
    }
  }
//
//  Get the seniority dates and fill the driver text string and text flags
//
  for(nI = 0; nI < num_AvailableDrivers; nI++)
  {
    DRIVERSKey0.recordID = m_AvailableDrivers[nI].DRIVERSrecordID;
    rcode2 = btrieve(B_GETEQUAL, TMS_DRIVERS, &DRIVERS, &DRIVERSKey0, 0);
    m_AvailableDrivers[nI].seniorityDate = DRIVERS.seniorityDate;
    m_AvailableDrivers[nI].senioritySort = DRIVERS.senioritySort;
    strcpy(m_AvailableDrivers[nI].szDriver, BuildOperatorString(NO_RECORD));
//
// Driver type
//
    DRIVERTYPESKey0.recordID = DRIVERS.DRIVERTYPESrecordID;
    rcode2 = btrieve(B_GETEQUAL, TMS_DRIVERTYPES, &DRIVERTYPES, &DRIVERTYPESKey0, 0);
    if(rcode2 != 0)
    {
      strcpy(m_AvailableDrivers[nI].szDriverType, "");
    }
    else
    {
      strncpy(tempString, DRIVERTYPES.name, DRIVERTYPES_NAME_LENGTH);
      trim(tempString, DRIVERTYPES_NAME_LENGTH);
      strcpy(m_AvailableDrivers[nI].szDriverType, tempString);
    }
//
//  Availability
//
    if(m_AvailableDrivers[nI].flags & AVAILABLE_FLAG_UNASSIGNED)
    {
      s = "Unassigned";
    }
    else if(m_AvailableDrivers[nI].flags & AVAILABLE_FLAG_STANDBY)
    {
      s = "Standby";
    }
    else if(m_AvailableDrivers[nI].flags & AVAILABLE_FLAG_DAYOFF)
    {
      s = "Day Off";
    }
    else if(m_AvailableDrivers[nI].flags & AVAILABLE_FLAG_BEFORE)
    {
      s = "Done Before";
      sprintf(tempString, " (On run %ld finishing at %s)",
            m_AvailableDrivers[nI].runNumber, Tchar(m_AvailableDrivers[nI].time[0]));
      s += tempString;
    }
    else if(m_AvailableDrivers[nI].flags & AVAILABLE_FLAG_AFTER)
    {
      s = "Starts After";
      sprintf(tempString, " (On run %ld starting at %s)",
            m_AvailableDrivers[nI].runNumber, Tchar(m_AvailableDrivers[nI].time[0]));
      s += tempString;
    }
    else if(m_AvailableDrivers[nI].flags & AVAILABLE_FLAG_NOTENOUGHOFFTIME)
    {
      s = "Ineligible";
      sprintf(tempString, " (Off time only %s)", chhmm(m_AvailableDrivers[nI].time[0]));
      s += tempString;
      m_AvailableDrivers[nI].flags |= AVAILABLE_FLAG_INELIGIBLE;
    }
    else if(m_AvailableDrivers[nI].flags & AVAILABLE_FLAG_ABSENT)
    {
      s = "Ineligible";
      if(m_AvailableDrivers[nI].time[0] == 0 && m_AvailableDrivers[nI].time[1] >= T1159P)
      {
        strcpy(tempString, " (Absent all day)");
      }
      else
      {
        sprintf(szarString, Tchar(m_AvailableDrivers[nI].time[1]));
        sprintf(tempString, " (Absent between %s and %s)", Tchar(m_AvailableDrivers[nI].time[0]), szarString);
      }
      s += tempString;
      m_AvailableDrivers[nI].flags |= AVAILABLE_FLAG_INELIGIBLE;
    }
    else if(m_AvailableDrivers[nI].flags & AVAILABLE_FLAG_DEASSIGNED)
    {
      s = "Deassigned";
    }
    else if(m_AvailableDrivers[nI].flags & AVAILABLE_FLAG_INELIGIBLE)
    {
      s = "Ineligible";
      strcpy(szarString, Tchar(m_AvailableDrivers[nI].time[1]));
      sprintf(tempString, " (Rostered on run %ld, %s to %s)",
            m_AvailableDrivers[nI].runNumber, Tchar(m_AvailableDrivers[nI].time[0]), szarString);
      s += tempString;
    }
    strcpy(m_AvailableDrivers[nI].szFlags, s);
  }
//
//  Zero out the times for each driver
//
  for(nI = 0; nI < num_AvailableDrivers; nI++)
  {
    m_AvailableDrivers[nI].rosteredTime = 0;
    m_AvailableDrivers[nI].openWorkTime = 0;
    m_AvailableDrivers[nI].extraboardSequence = NO_RECORD;
  }
//
//  Extablish the rostered time for each driver this week
//
  for(nI = 0; nI < num_AvailableDrivers; nI++)
  {
    for(nJ = 0; nJ < m_pPassedData->numRD; nJ++)
    {
      if(m_AvailableDrivers[nI].DRIVERSrecordID == m_pPassedData->pRD[nJ].DRIVERSrecordID)
      {
        m_AvailableDrivers[nI].rosteredTime += m_pPassedData->pRD[nJ].time;
        break;
      }
    }
  }
//
//  Calculate the assigned time (from open work) for each driver this week
//
  CalculateAssignedTime(m_pPassedData->date);
//
//  If there's a overlap on this "to be assigned" run,
//  kick the driver out of the available drivers list
//
  BOOL bFound1, bFound2;

  for(nI = 0; nI < m_numAssignments[today]; nI++)
  {
    bFound1 = FALSE;
    bFound2 = FALSE;
    for(nJ = 0; nJ < num_AvailableDrivers; nJ++)
    {
      if(m_Assignments[today][nI].DRIVERSrecordID == m_AvailableDrivers[nJ].DRIVERSrecordID)
      {
        if(m_Assignments[today][nI].onTime <= m_pPassedData->runStartTime)
        {
          if(m_Assignments[today][nI].offTime > m_pPassedData->runStartTime)
          {
            bFound1 = TRUE;
          }
        }
        else
        {
          if(m_Assignments[today][nI].onTime <= m_pPassedData->runEndTime)
          {
            bFound2 = TRUE;
          }
        }
        if(bFound1 || bFound2)
        {
          strcpy(m_AvailableDrivers[nJ].szFlags, "Ineligible (");
          strcpy(szarString, Tchar(m_Assignments[today][nI].onTime));
          sprintf(tempString, "Assigned on run %ld, %s to %s)",
                m_Assignments[today][nI].runNumber, szarString, Tchar(m_Assignments[today][nI].offTime));
          strcat(m_AvailableDrivers[nJ].szFlags, tempString);
          m_AvailableDrivers[nJ].flags = AVAILABLE_FLAG_INELIGIBLE;
          break;
        }
      }
    }
  }
//
//  Pick out the pay times
//
  if(m_pPassedData->flags & DAILYOPSSO_FLAGS_SHOWHOURS)
  {
    for(nI = 0; nI < ROSTER_MAX_DAYS; nI++)
    {
      for(nJ = 0; nJ < m_numAssignments[nI]; nJ++)
      {
        bFound = FALSE;
        if(m_Assignments[nI][nJ].DRIVERSrecordID != NO_RECORD)
        {
          for(nK = 0; nK < num_AvailableDrivers; nK++)
          {
            if(m_AvailableDrivers[nK].DRIVERSrecordID == m_Assignments[nI][nJ].DRIVERSrecordID)
            {
              m_AvailableDrivers[nK].openWorkTime += m_Assignments[nI][nJ].pay;
            }
          }
        }
      }
    }
//
//  Keep the total in the structure in case he wants to sort on it
//
    for(nI = 0; nI < num_AvailableDrivers; nI++)
    {
      m_AvailableDrivers[nI].totalTime = m_AvailableDrivers[nI].rosteredTime + m_AvailableDrivers[nI].openWorkTime;
    }
  }
//
//  Sort the list by DRIVERSrecordID - NO_RECORDs will bubble to the bottom
//
  qsort((void *)m_AvailableDrivers, num_AvailableDrivers, sizeof(AvailableDriversDef), eliminate_AvailableDrivers);
  for(nI = 0; nI < num_AvailableDrivers; nI++)
  {
    if(m_AvailableDrivers[nI].DRIVERSrecordID == NO_RECORD)
    {
      num_AvailableDrivers = nI;
      break;
    }
  }
//
//  Sort the list by seniorityDate and senioritySort
//
  qsort((void *)m_AvailableDrivers, num_AvailableDrivers, sizeof(AvailableDriversDef), sort_AvailableDrivers);
//
//  Attach the position within the extraboard(s) where applicable
//
  CTime SystemTime = CTime::GetCurrentTime();

//
//  Daytime
//
  DAILYOPSKey1.recordTypeFlag = DAILYOPS_FLAG_ROSTER;
  DAILYOPSKey1.pertainsToDate = m_pPassedData->date;
  DAILYOPSKey1.pertainsToTime = SystemTime.GetHour() * 3600 + SystemTime.GetMinute() * 60 + SystemTime.GetSecond();
  DAILYOPSKey1.recordFlags = 0;
  rcode2 = btrieve(B_GETLESSTHANOREQUAL, TMS_DAILYOPS, &DAILYOPS, &DAILYOPSKey1, 1);
  bFound = FALSE;
  while(rcode2 == 0 &&
        (DAILYOPS.recordTypeFlag & DAILYOPS_FLAG_ROSTER))
  {
    if(DAILYOPS.recordFlags & DAILYOPS_FLAG_ROSTERSETNEXTDAYTIME)
    {
      bFound = TRUE;
      break;
    }
    rcode2 = btrieve(B_GETPREVIOUS, TMS_DAILYOPS, &DAILYOPS, &DAILYOPSKey1, 1);
  }
  if(bFound)
  {
    m_NextDaytimeIndex = DAILYOPS.DOPS.RosterTemplate.nextDaytime;
  }
  else
  {
    m_NextDaytimeIndex = 1;
  }
//
//  Nighttime
//
  DAILYOPSKey1.recordTypeFlag = DAILYOPS_FLAG_ROSTER;
  DAILYOPSKey1.pertainsToDate = m_pPassedData->date;
  DAILYOPSKey1.pertainsToTime = SystemTime.GetHour() * 3600 + SystemTime.GetMinute() * 60 + SystemTime.GetSecond();
  DAILYOPSKey1.recordFlags = 0;
  rcode2 = btrieve(B_GETLESSTHANOREQUAL, TMS_DAILYOPS, &DAILYOPS, &DAILYOPSKey1, 1);
  bFound = FALSE;
  while(rcode2 == 0 &&
        (DAILYOPS.recordTypeFlag & DAILYOPS_FLAG_ROSTER))
  {
    if(DAILYOPS.recordFlags & DAILYOPS_FLAG_ROSTERSETNEXTNIGHTTIME)
    {
      bFound = TRUE;
      break;
    }
    rcode2 = btrieve(B_GETPREVIOUS, TMS_DAILYOPS, &DAILYOPS, &DAILYOPSKey1, 1);
  }
  if(bFound)
  {
    m_NextNighttimeIndex = DAILYOPS.DOPS.RosterTemplate.nextNighttime;
  }
  else
  {
    m_NextNighttimeIndex = 1;
  }
//
//  Cycle through the drivers to set out their position
//
  int currentDaytimeIndex = 1;
  int currentDaytimeValue = 1;
  int currentNighttimeIndex = 1;
  int currentNighttimeValue = 1;

//
//  Daytime
//
  for(nI = 0; nI < num_AvailableDrivers; nI++)
  {
//    if((m_AvailableDrivers[nI].flags & AVAILABLE_FLAG_DAYTIME) &&
//          !(m_AvailableDrivers[nI].flags & AVAILABLE_FLAG_INELIGIBLE))
    if(m_AvailableDrivers[nI].flags & AVAILABLE_FLAG_DAYTIME)
    {
      if(currentDaytimeIndex > m_NextDaytimeIndex)
      {
        m_AvailableDrivers[nI].extraboardSequence = currentDaytimeValue;
        currentDaytimeValue++;
      }
      else
      {
        currentDaytimeIndex++;
      }
    }
  }
//
//  Stamp the ones we missed
//
  for(nI = 0; nI < num_AvailableDrivers; nI++)
  {
//    if((m_AvailableDrivers[nI].flags & AVAILABLE_FLAG_DAYTIME) &&
//          !(m_AvailableDrivers[nI].flags & AVAILABLE_FLAG_INELIGIBLE))
    if(m_AvailableDrivers[nI].flags & AVAILABLE_FLAG_DAYTIME)
    {
      if(m_AvailableDrivers[nI].extraboardSequence != NO_RECORD)
      {
        break;
      }
      else
      {
        m_AvailableDrivers[nI].extraboardSequence = currentDaytimeValue;
        currentDaytimeValue++;
      }
    }
  }
//
//  Nighttime
//
  for(nI = 0; nI < num_AvailableDrivers; nI++)
  {
    if((m_AvailableDrivers[nI].flags & AVAILABLE_FLAG_NIGHTTIME) &&
          !(m_AvailableDrivers[nI].flags & AVAILABLE_FLAG_INELIGIBLE))
    {
      if(currentNighttimeIndex >= m_NextNighttimeIndex)
      {
        m_AvailableDrivers[nI].extraboardSequence = currentNighttimeValue;
        currentNighttimeValue++;
      }
    }
  }
//
//  Stamp the ones we missed
//
  for(nI = 0; nI < num_AvailableDrivers; nI++)
  {
    if((m_AvailableDrivers[nI].flags & AVAILABLE_FLAG_NIGHTTIME) &&
          !(m_AvailableDrivers[nI].flags & AVAILABLE_FLAG_INELIGIBLE))
    {
      if(m_AvailableDrivers[nI].extraboardSequence != NO_RECORD)
      {
        break;
      }
      else
      {
        m_AvailableDrivers[nI].extraboardSequence = currentNighttimeValue;
        currentNighttimeValue++;
      }
    }
  }
//
//  Set out the text string for the flags
//
  for(nI = 0; nI < num_AvailableDrivers; nI++)
  {
    if((m_AvailableDrivers[nI].flags & AVAILABLE_FLAG_DAYTIME) &&
          !(m_AvailableDrivers[nI].flags & AVAILABLE_FLAG_INELIGIBLE))
    {
      sprintf(m_AvailableDrivers[nI].szFlags, "Daytime extraboard - Pos %3d", m_AvailableDrivers[nI].extraboardSequence);
    }
    else if((m_AvailableDrivers[nI].flags & AVAILABLE_FLAG_NIGHTTIME) &&
          !(m_AvailableDrivers[nI].flags & AVAILABLE_FLAG_INELIGIBLE))
    {
      sprintf(m_AvailableDrivers[nI].szFlags, "Nighttime extraboard - Pos %3d", m_AvailableDrivers[nI].extraboardSequence);
    }
  }
//
//  And show 'em
//
  DisplayAvailableDrivers();

  SetCursor(hCursorSave);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CDailyOpsSO::OnClose() 
{
	// TODO: Add your message handler code here and/or call default
	
	CDialog::OnClose();
}

void CDailyOpsSO::OnCancel() 
{
	// TODO: Add extra cleanup here
	
	CDialog::OnCancel();
}

void CDailyOpsSO::OnHelp() 
{
	// TODO: Add your control notification handler code here
	
}

void CDailyOpsSO::OnOK() 
{
  if(m_Index == NO_RECORD)
  {
    return;
  }
  AvailableDriversDef *pAD = (AvailableDriversDef *)pListCtrlLIST->GetItemData(m_Index);

  if(pAD->flags & AVAILABLE_FLAG_INELIGIBLE)
  {
    MessageBeep(MB_ICONSTOP);
    return;
  }

  m_pPassedData->selectedOperator = pAD->DRIVERSrecordID;
//
//  If necessary, reset the "next" daytime and nighttime extraboard rotation number
//
  CTime time = CTime::GetCurrentTime();
  long  datenow = time.GetYear() * 10000 + time.GetMonth() * 100 + time.GetDay();
  long  timenow = time.GetHour() * 3600 + time.GetMinute() * 60 + time.GetSecond();
  int   rcode2;
//
//  Daytime
//
  if(pAD->flags & AVAILABLE_FLAG_DAYTIME)
  {
    rcode2 = btrieve(B_GETLAST, TMS_DAILYOPS, &DAILYOPS, &DAILYOPSKey0, 0);
    DAILYOPS.recordID = AssignRecID(rcode2, DAILYOPS.recordID);
    DAILYOPS.entryDateAndTime = time.GetTime();
    DAILYOPS.pertainsToDate = datenow;
    DAILYOPS.pertainsToTime = timenow;
    DAILYOPS.recordTypeFlag = DAILYOPS_FLAG_ROSTER;
    DAILYOPS.recordFlags = DAILYOPS_FLAG_ROSTERSETNEXTDAYTIME;
    DAILYOPS.userID = m_UserID;
    DAILYOPS.DRIVERSrecordID = ROSTER.DRIVERSrecordID;
    DAILYOPS.DAILYOPSrecordID = NO_RECORD;
    memset(DAILYOPS.DOPS.associatedData, 0x00, DAILYOPS_ASSOCIATEDDATA_LENGTH);
    DAILYOPS.DOPS.RosterTemplate.nextDaytime = pAD->extraboardSequence + 1;
    rcode2 = btrieve(B_INSERT, TMS_DAILYOPS, &DAILYOPS, &DAILYOPSKey0, 0);
  }
//
//  Nighttime
//
  else if(pAD->flags & AVAILABLE_FLAG_NIGHTTIME)
  {
    rcode2 = btrieve(B_GETLAST, TMS_DAILYOPS, &DAILYOPS, &DAILYOPSKey0, 0);
    DAILYOPS.recordID = AssignRecID(rcode2, DAILYOPS.recordID);
    DAILYOPS.entryDateAndTime = time.GetTime();
    DAILYOPS.pertainsToDate = datenow;
    DAILYOPS.pertainsToTime = timenow;
    DAILYOPS.recordTypeFlag = DAILYOPS_FLAG_ROSTER;
    DAILYOPS.recordFlags = DAILYOPS_FLAG_ROSTERSETNEXTNIGHTTIME;
    DAILYOPS.userID = m_UserID;
    DAILYOPS.DRIVERSrecordID = ROSTER.DRIVERSrecordID;
    DAILYOPS.DAILYOPSrecordID = NO_RECORD;
    memset(DAILYOPS.DOPS.associatedData, 0x00, DAILYOPS_ASSOCIATEDDATA_LENGTH);
    DAILYOPS.DOPS.RosterTemplate.nextNighttime = pAD->extraboardSequence + 1;
    rcode2 = btrieve(B_INSERT, TMS_DAILYOPS, &DAILYOPS, &DAILYOPSKey0, 0);
  }
	
  CDialog::OnOK();
}

void CDailyOpsSO::OnClickList(NMHDR* pNMHDR, LRESULT* pResult) 
{
  POSITION position = pListCtrlLIST->GetFirstSelectedItemPosition();
  
  if(position == NULL)
  {
    return;
  }
  m_Index = pListCtrlLIST->GetNextSelectedItem(position);
	
	*pResult = 0;
}

void CDailyOpsSO::OnDblclkList(NMHDR* pNMHDR, LRESULT* pResult) 
{
  POSITION position = pListCtrlLIST->GetFirstSelectedItemPosition();
  
  if(position == NULL)
  {
    return;
  }
  m_Index = pListCtrlLIST->GetNextSelectedItem(position);

  OnOK();
	
	*pResult = 0;
}

void CDailyOpsSO::DisplayAvailableDrivers()
{
  pListCtrlLIST->DeleteAllItems();
//
//  Populate the list control
//
  CString s;
  LVITEM LVI;
  BOOL bShowIneligible = pButtonSHOWINELIGIBLE->GetCheck();
  long yyyy, mm, dd;
  int  iItem = 0;
  int  nI;

  for(nI = 0; nI < num_AvailableDrivers; nI++)
  {
    if(!bShowIneligible && (m_AvailableDrivers[nI].flags & AVAILABLE_FLAG_INELIGIBLE))
    {
      continue;
    }
//
//  Operator
//
    LVI.mask = LVIF_TEXT | LVIF_PARAM;
    LVI.iItem = iItem;
    LVI.iSubItem = 0;
    LVI.pszText = m_AvailableDrivers[nI].szDriver;
    LVI.lParam = (LPARAM)&m_AvailableDrivers[nI];
    LVI.iItem = pListCtrlLIST->InsertItem(&LVI);
//
//  Driver type
//
    LVI.mask = LVIF_TEXT;
    LVI.iItem = iItem;
    LVI.iSubItem = 1;
    LVI.pszText = m_AvailableDrivers[nI].szDriverType;
    LVI.iItem = pListCtrlLIST->SetItem(&LVI);
//
//  Status
//
    LVI.mask = LVIF_TEXT;
    LVI.iItem = iItem;
    LVI.iSubItem = 2;
    LVI.pszText = m_AvailableDrivers[nI].szFlags;
    LVI.iItem = pListCtrlLIST->SetItem(&LVI);
//
//  Seniority date
//
    LVI.mask = LVIF_TEXT;
    LVI.iItem = iItem;
    LVI.iSubItem = 3;
    GetYMD(m_AvailableDrivers[nI].seniorityDate, &yyyy, &mm, &dd);
    sprintf(tempString, "%4ld/%02ld/%02ld", yyyy, mm, dd);
    LVI.pszText = tempString;
    LVI.iItem = pListCtrlLIST->SetItem(&LVI);
//
//  Hours
//
    if(m_pPassedData->flags & DAILYOPSSO_FLAGS_SHOWHOURS)
    {

//
//  Rostered Hours
//
      LVI.mask = LVIF_TEXT;
      LVI.iItem = iItem;
      LVI.iSubItem = 4;
      strcpy(tempString, chhmm(m_AvailableDrivers[nI].rosteredTime));
      LVI.pszText = tempString;
      LVI.iItem = pListCtrlLIST->SetItem(&LVI);
//
//  Assigned (from open work) Hours
//
      LVI.mask = LVIF_TEXT;
      LVI.iItem = iItem;
      LVI.iSubItem = 5;
      strcpy(tempString, chhmm(m_AvailableDrivers[nI].openWorkTime));
      LVI.pszText = tempString;
      LVI.iItem = pListCtrlLIST->SetItem(&LVI);
//
//  Total Hours
//
      LVI.mask = LVIF_TEXT;
      LVI.iItem = iItem;
      LVI.iSubItem = 6;
      strcpy(tempString, chhmm(m_AvailableDrivers[nI].totalTime));
      LVI.pszText = tempString;
      LVI.iItem = pListCtrlLIST->SetItem(&LVI);
    }
//
//  Set the next item
//
    iItem++;
  }
}

void CDailyOpsSO::OnColumnclickList(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;

  if(pNMListView)
  {
    ListView_SortItems(pNMListView->hdr.hwndFrom,
          DOSOListViewCompareProc, (LPARAM)(pNMListView->iSubItem));
    bForwardSort[pNMListView->iSubItem] = !bForwardSort[pNMListView->iSubItem];
  }

	*pResult = 0;
}


void CDailyOpsSO::OnPrint() 
{
  PrintWindowToDC(this->m_hWnd, "Operator Selection");
}

void CDailyOpsSO::OnRclickList(NMHDR* pNMHDR, LRESULT* pResult) 
{
  POSITION position = pListCtrlLIST->GetFirstSelectedItemPosition();
  
  if(position == NULL)
  {
    return;
  }

  long recordID;

  m_Index = pListCtrlLIST->GetNextSelectedItem(position);
  pListCtrlLIST->GetItemData(m_Index);
  
  CAddDriver dlg(NULL, &recordID);
  dlg.DoModal();
	
	*pResult = 0;
}

void CDailyOpsSO::OnShowineligible() 
{
  DisplayAvailableDrivers();
}

void CDailyOpsSO::CalculateAssignedTime(long startDate)
{
  long dateToUse;
  int  nI, nJ, nK;
  int  rcode2;

//
//  If the passed date wasn't a Monday, back up to it.
//
  long yyyy, mm, dd;

  GetYMD(startDate, &yyyy, &mm, &dd);
  
  CTime mondayDate(yyyy, mm, dd, 0, 0, 0);
  CTimeSpan oneDay(1, 0, 0, 0);
  
  for(;;)
  {
    if(mondayDate.GetDayOfWeek() == 2)
    {
      break;
    }
    mondayDate -= oneDay;
  }
//
//  Absences for each day
//
typedef struct ABSENTStruct
{
  long DRIVERSrecordID;
  long fromTime;
  long toTime;
} ABSENTDef;

  ABSENTDef ABSENT[ROSTER_MAX_DAYS][500];
  BOOL bFound;
  int  numAbsent[ROSTER_MAX_DAYS];

  CTime workDate = mondayDate;

  for(nI = 0; nI < ROSTER_MAX_DAYS; nI++)
  {
    yyyy = workDate.GetYear();
    mm   = workDate.GetMonth();
    dd   = workDate.GetDay();
    dateToUse = (yyyy * 10000) + (mm * 100) + dd;
    if(!(m_pPassedData->flags & DAILYOPSSO_FLAGS_SHOWHOURS) && dateToUse != startDate)
    {
      continue;
    }
    numAbsent[nI] = 0;
    DAILYOPSKey1.recordTypeFlag = DAILYOPS_FLAG_ABSENCE;
    DAILYOPSKey1.pertainsToDate = dateToUse - 10000;
    DAILYOPSKey1.pertainsToTime = 0;
    DAILYOPSKey1.recordFlags = 0;
    bFound = FALSE;
    rcode2 = btrieve(B_GETGREATEROREQUAL, TMS_DAILYOPS, &DAILYOPS, &DAILYOPSKey1, 1);
    while(rcode2 == 0 &&
          (DAILYOPS.recordTypeFlag & DAILYOPS_FLAG_ABSENCE) &&
           DAILYOPS.pertainsToDate <= dateToUse)
    {
      if(dateToUse >= DAILYOPS.pertainsToDate &&
            dateToUse <= DAILYOPS.DOPS.Absence.untilDate &&
            (DAILYOPS.recordFlags & DAILYOPS_FLAG_ABSENCEREGISTER))
      {
        if(!ANegatedRecord(DAILYOPS.recordID, 1))
        {
          ABSENT[nI][numAbsent[nI]].DRIVERSrecordID = DAILYOPS.DRIVERSrecordID;
          ABSENT[nI][numAbsent[nI]].fromTime = DAILYOPS.pertainsToTime;
          ABSENT[nI][numAbsent[nI]].toTime = DAILYOPS.DOPS.Absence.untilTime;
//
//  Set the fromTime to 0 if we're in a date range
//
          if(dateToUse > DAILYOPS.pertainsToDate)
          {
            ABSENT[nI][numAbsent[nI]].fromTime = 0;
          }
//
//  Set the toTime to 1159P if we're in a date range
//
          if(dateToUse < DAILYOPS.DOPS.Absence.untilDate)
          {
            ABSENT[nI][numAbsent[nI]].toTime = T1159P;
          }
//
//  If the toTime is 1159P, add extra time to it to ensure
//  we don't get a forced "run split" if the run goes later
//
          if(ABSENT[nI][numAbsent[nI]].toTime >= T1159P)
          {
            ABSENT[nI][numAbsent[nI]].toTime += T1159P;
          }
          numAbsent[nI]++;
        }
      }
      rcode2 = btrieve(B_GETNEXT, TMS_DAILYOPS, &DAILYOPS, &DAILYOPSKey1, 1);
    }
    workDate += oneDay;
  }
//
//  Check the audit trail for any assignments that have been made for today
//
  PROPOSEDRUNDef PROPOSEDRUN;
  COSTDef COST;
  BOOL bCrewOnly;
  long cutAsRuntype;
  int  numPieces;

  workDate = mondayDate;

  for(nI = 0; nI < ROSTER_MAX_DAYS; nI++)
  {
    m_numAssignments[nI] = 0;
    yyyy = workDate.GetYear();
    mm   = workDate.GetMonth();
    dd   = workDate.GetDay();
    dateToUse = (yyyy * 10000) + (mm * 100) + dd;
    DAILYOPSKey1.recordTypeFlag = DAILYOPS_FLAG_OPENWORK;
    DAILYOPSKey1.pertainsToDate = dateToUse;
    DAILYOPSKey1.pertainsToTime = 999999;
    DAILYOPSKey1.recordFlags = 255;
    rcode2 = btrieve(B_GETLESSTHAN, TMS_DAILYOPS, &DAILYOPS, &DAILYOPSKey1, 1);
    while(rcode2 == 0 &&
          (DAILYOPS.recordTypeFlag & DAILYOPS_FLAG_OPENWORK) &&
           DAILYOPS.pertainsToDate == dateToUse)
    {
      if(DAILYOPS.recordFlags & DAILYOPS_FLAG_OPENWORKASSIGN && !ANegatedRecord(DAILYOPS.recordID, 1))
      {
        bFound = FALSE;
        for(nJ = 0; nJ < m_numAssignments[nI]; nJ++)
        {
          if(m_Assignments[nI][nJ].RUNSrecordID == DAILYOPS.DOPS.OpenWork.RUNSrecordID)
          {
            if((DAILYOPS.recordFlags & DAILYOPS_FLAG_CREWONLY) &&
                  (m_Assignments[nI][nJ].flags & DAILYOPS_FLAG_CREWONLY))
            {
              bFound = TRUE;
              break;
            }
            else if((DAILYOPS.recordFlags & DAILYOPS_FLAG_RUNSPLIT) ||
                  (DAILYOPS.recordFlags & DAILYOPS_FLAG_RUNSPLITCREWONLY)) 
            {
              if(DAILYOPS.DOPS.OpenWork.splitStartTime == m_Assignments[nI][nJ].onTime &&
                    DAILYOPS.DOPS.OpenWork.splitStartNODESrecordID == m_Assignments[nI][nJ].onNODESrecordID)
              {
                bFound = TRUE;
                break;
              }
            }
            else
            {
              bFound = TRUE;
              break;
            }
          }
        }
        if(!bFound)
        {
          if((DAILYOPS.recordFlags & DAILYOPS_FLAG_RUNSPLITCREWONLY) ||
                (DAILYOPS.recordFlags & DAILYOPS_FLAG_CREWONLY))
          {
            CREWONLYKey0.recordID = DAILYOPS.DOPS.OpenWork.RUNSrecordID;
            rcode2 = btrieve(B_GETEQUAL, TMS_CREWONLY, &CREWONLY, &CREWONLYKey0, 0);
            m_Assignments[nI][m_numAssignments[nI]].runNumber = (rcode2 == 0 ? CREWONLY.runNumber : NO_RECORD);
          }
          else
          {
            RUNSKey0.recordID = DAILYOPS.DOPS.OpenWork.RUNSrecordID;
            rcode2 = btrieve(B_GETEQUAL, TMS_RUNS, &RUNS, &RUNSKey0, 0);
            m_Assignments[nI][m_numAssignments[nI]].runNumber = (rcode2 == 0 ? RUNS.runNumber : NO_RECORD);
          }
          m_Assignments[nI][m_numAssignments[nI]].DRIVERSrecordID = DAILYOPS.DRIVERSrecordID;
          m_Assignments[nI][m_numAssignments[nI]].RUNSrecordID = DAILYOPS.DOPS.OpenWork.RUNSrecordID;
          m_Assignments[nI][m_numAssignments[nI]].flags = DAILYOPS.recordFlags;
          if((DAILYOPS.recordFlags & DAILYOPS_FLAG_RUNSPLIT) ||
                (DAILYOPS.recordFlags & DAILYOPS_FLAG_RUNSPLITCREWONLY))
          {
            m_Assignments[nI][m_numAssignments[nI]].cutAsRuntype = RUNSPLITRUNTYPE;
            m_Assignments[nI][m_numAssignments[nI]].onNODESrecordID = DAILYOPS.DOPS.OpenWork.splitStartNODESrecordID;
            m_Assignments[nI][m_numAssignments[nI]].onTime = DAILYOPS.DOPS.OpenWork.splitStartTime;
            m_Assignments[nI][m_numAssignments[nI]].offNODESrecordID = DAILYOPS.DOPS.OpenWork.splitEndNODESrecordID;
            m_Assignments[nI][m_numAssignments[nI]].offTime = DAILYOPS.DOPS.OpenWork.splitEndTime;
            m_Assignments[nI][m_numAssignments[nI]].pay = 0;
          }
          else
          {
            bCrewOnly = DAILYOPS.recordFlags & DAILYOPS_FLAG_CREWONLY;
            cutAsRuntype = CDailyOps::SetupRun(DAILYOPS.DOPS.OpenWork.RUNSrecordID, bCrewOnly, &PROPOSEDRUN);
            numPieces = PROPOSEDRUN.numPieces;
            RunCoster(&PROPOSEDRUN, cutAsRuntype, &COST);
            m_Assignments[nI][m_numAssignments[nI]].cutAsRuntype = cutAsRuntype;
            m_Assignments[nI][m_numAssignments[nI]].onNODESrecordID = PROPOSEDRUN.piece[0].fromNODESrecordID;
            m_Assignments[nI][m_numAssignments[nI]].onTime = PROPOSEDRUN.piece[0].fromTime;
            m_Assignments[nI][m_numAssignments[nI]].offNODESrecordID = PROPOSEDRUN.piece[numPieces - 1].toNODESrecordID;
            m_Assignments[nI][m_numAssignments[nI]].offTime = PROPOSEDRUN.piece[numPieces - 1].toTime;
            m_Assignments[nI][m_numAssignments[nI]].pay = COST.TOTAL.payTime;
          }
          m_numAssignments[nI]++;
        }
      }
      rcode2 = btrieve(B_GETPREVIOUS, TMS_DAILYOPS, &DAILYOPS, &DAILYOPSKey1, 1);             
    }
    workDate += oneDay;
  }
//
//  Apply any recosting to run splits
//
  workDate = mondayDate;

  for(nI = 0; nI < ROSTER_MAX_DAYS; nI++)
  {
    yyyy = workDate.GetYear();
    mm   = workDate.GetMonth();
    dd   = workDate.GetDay();
    dateToUse = (yyyy * 10000) + (mm * 100) + dd;
    for(nJ = 0; nJ < m_numAssignments[nI]; nJ++)
    {
      if(m_Assignments[nI][nJ].cutAsRuntype != RUNSPLITRUNTYPE || m_Assignments[nI][nJ].pay != 0)
      {
        continue;
      }
      DAILYOPSKey1.recordTypeFlag = DAILYOPS_FLAG_OPENWORK;
      DAILYOPSKey1.pertainsToDate = dateToUse;
      DAILYOPSKey1.pertainsToTime = 999999;
      DAILYOPSKey1.recordFlags = DAILYOPS_FLAG_RECOSTRUN;
      rcode2 = btrieve(B_GETLESSTHAN, TMS_DAILYOPS, &DAILYOPS, &DAILYOPSKey1, 1);
      while(rcode2 == 0 &&
            DAILYOPS.recordTypeFlag == DAILYOPS_FLAG_OPENWORK &&
           (DAILYOPS.recordFlags & DAILYOPS_FLAG_RECOSTRUN) &&
            DAILYOPS.pertainsToDate == dateToUse)
      {
        if(DAILYOPS.DOPS.OpenWork.RUNSrecordID == m_Assignments[nI][nJ].RUNSrecordID)
        {
          if(DAILYOPS.DAILYOPSrecordID == NO_RECORD)
          {
            if(DAILYOPS.DOPS.OpenWork.splitStartTime == m_Assignments[nI][nJ].onTime &&
                  DAILYOPS.DOPS.OpenWork.splitStartNODESrecordID == m_Assignments[nI][nJ].onNODESrecordID &&
                  DAILYOPS.DOPS.OpenWork.splitEndTime == m_Assignments[nI][nJ].offTime &&
                  DAILYOPS.DOPS.OpenWork.splitEndNODESrecordID == m_Assignments[nI][nJ].offNODESrecordID)
            {
              m_Assignments[nI][nJ].pay = DAILYOPS.DOPS.OpenWork.payTime;
              break;
            }
          }
        }
        rcode2 = btrieve(B_GETPREVIOUS, TMS_DAILYOPS, &DAILYOPS, &DAILYOPSKey1, 1);
      }
    }
    workDate += oneDay;
  }
//
//  Cycle through m_Assignments and match it to the absence array to see if the assigned
//  operator is here today.  If they aren't present, then set the pay time to 0.
//
  workDate = mondayDate;

  for(nI = 0; nI < ROSTER_MAX_DAYS; nI++)
  {
    yyyy = workDate.GetYear();
    mm   = workDate.GetMonth();
    dd   = workDate.GetDay();
    dateToUse = (yyyy * 10000) + (mm * 100) + dd;
    for(nJ = 0; nJ < m_numAssignments[nI]; nJ++)
    {
      for(bFound = FALSE, nK = 0; nK < numAbsent[nI]; nK++)
      {
//
//  Is the operator away?
//
        if(m_Assignments[nI][nJ].DRIVERSrecordID == ABSENT[nI][nK].DRIVERSrecordID)
        {
//
//  Now - is the operator away at any time during the span of the run?
//
//  Six possibilities:
//
//               |-------Run---------|
//   1:       |-Away-|
//   2:                |-Away-|
//   3:                         |-Away-|                
//   4:|-Away-|
//   5:                                |-Away-|
//   6:       |--------Away------------|
//
//   Cases 1, 2, 3, and 6 are handled here.  We don't care about 4 and 5.
//
          if(ABSENT[nI][nK].fromTime <= m_Assignments[nI][nJ].onTime)
          {
            if(ABSENT[nI][nK].toTime >= m_Assignments[nI][nJ].onTime)
            {
              m_Assignments[nI][nJ].pay = 0;
              break;
            }
          }
          else
          {
            if(ABSENT[nI][nK].fromTime <= m_Assignments[nI][nJ].offTime)
            {
              m_Assignments[nI][nJ].pay = 0;
              break;
            }
          }
        }
      }
    }
    workDate += oneDay;
  }
}

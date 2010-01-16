// DailyOps.cpp : implementation file
//

//#define DAILYOPSNOTIFY

#include "stdafx.h"

long YMDOffset(long year, long month, long day, long offset);

extern "C"{
#include "TMSHeader.h"
#include "cistms.h"

BOOL ProcessAbsenceRules(DAILYOPSDef *pDAILYOPS, BOOL bRegister);

//
//  Sort function for the Chronology box
//
typedef struct CHRONStruct
{
  long time;
  char text[124];
} CHRONDef;

int sort_Chronology(const void*a, const void *b)
{
  CHRONDef *pa, *pb;
  pa = (CHRONDef *)a;
  pb = (CHRONDef *)b;

  return(pa->time < pb->time ? -1 :
         pa->time > pb->time ?  1 : 0);
}

//
//  Sort function for the weekly assignment report
//
typedef struct WeeklyStruct
{
  long DRIVERSrecordID;
  long seniorityDate;
  long runNumber[7][4];
  long payTime[7][4];
  long cutAsRuntype[7][4];
  long onNODESrecordID[7];
  long onTime[7];
  long offNODESrecordID[7];
  long offTime[7];
} WeeklyDef;

#define BUSROWSDISPLAYED 5
#define MAXWEEKLY        500

int sort_Weekly(const void *a, const void *b)
{
  WeeklyDef *pa, *pb; 
  pa = (WeeklyDef *)a;
  pb = (WeeklyDef *)b;
  return(pa->seniorityDate < pb->seniorityDate ? -1 :
         pa->seniorityDate > pb->seniorityDate ?  1 : 0);
}

//
//  Sort function for the "Assigned" bus list control
//  Item 6 is the PI Time
//
static int CALLBACK SortAssigned(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
  CListCtrl* pListCtrl = (CListCtrl*)lParamSort;
  char       PIString[16];
  long       PITime1, PITime2;
  BOOL       bFound;
  int        nI, numItems;

  numItems = pListCtrl->GetItemCount();

  for(bFound = FALSE, nI = 0; nI < numItems; nI++)
  {
    if((long)pListCtrl->GetItemData(nI) == lParam1)
    {
      strcpy(PIString, pListCtrl->GetItemText(nI, 6));
      PITime1 = cTime(PIString);
      bFound = TRUE;
    }
  }
  if(!bFound)
  {
    return(0);
  }

  for(bFound = FALSE, nI = 0; nI < numItems; nI++)
  {
    if((long)pListCtrl->GetItemData(nI) == lParam2)
    {
      strcpy(PIString, pListCtrl->GetItemText(nI, 6));
      PITime2 = cTime(PIString);
      bFound = TRUE;
    }
  }
  if(!bFound)
  {
    return(0);
  }

  return(PITime1 < PITime2 ? -1 : PITime1 > PITime2 ? 1 : 0);
}

//
//  Sort function for the "To be Assigned" bus list control
//
int sort_BlockInfo(const void *a, const void *b)
{
  BlockInfoDef *pa, *pb; 
  pa = (BlockInfoDef *)a;
  pb = (BlockInfoDef *)b;
  return(pa->POTime < pb->POTime ? -1 : pa->POTime > pb->POTime ? 1 : 0);
}


char m_TitleString[128];
char strTitle[128];

//
//  YMDOffset
//
long YMDO(long year, long month, long day, long offset)
{
  return YMDOffset(year, month, day, offset);
}

//
//  ANegatingRecord()
//
BOOL ANegatingRecord(DAILYOPSDef *pDAILYOPS)
{
  return(pDAILYOPS->DAILYOPSrecordID == NO_RECORD ? FALSE : TRUE);
}

//
//  ANegatedRecord()
//

BOOL ANegatedRecord(long recordID, int keyNumber)
{
  union DAILYOPSKeys
  {
    TMSKey0Def      Key0;
    DAILYOPSKey1Def Key1;
    DAILYOPSKey2Def Key2;
    DAILYOPSKey3Def Key3;
    DAILYOPSKey4Def Key4;
    DAILYOPSKey5Def Key5;
  };
  union DAILYOPSKeys Keys;
  DAILYOPSDef     DAILYOPS;
  DAILYOPSKey2Def DAILYOPSKey2;
  long  absRecID;
  BOOL  bFound;
  int   rcode2;

  rcode2 = btrieve(B_GETPOSITION, TMS_DAILYOPS, &absRecID, &Keys, keyNumber);
  if(rcode2 != 0)
  {
    return(0);
  }
  else
  {
    DAILYOPSKey2.DAILYOPSrecordID = recordID;
    bFound = (btrieve(B_GETEQUAL, TMS_DAILYOPS, &DAILYOPS, &DAILYOPSKey2, 2) == 0);
    DAILYOPS.recordID = absRecID;
    rcode2 = btrieve(B_GETDIRECT, TMS_DAILYOPS, &DAILYOPS, &Keys, keyNumber);
    if(rcode2 != 0)
    {
      return(0);
    }
    else
    {
      return(bFound ? 1 : 0);
    }
  }
}

}  // EXTERN C

#include <math.h>
#include "tms.h"
#include "AVLInterface.h"
#include "DailyOpsHeader.h"

int LocateIndexes(long SERVICESrecordID, long RUNSrecordID, long flags, int *pServiceIndex, int *pRunIndex)
{
  int  nI, nJ;

  for(nI = 0; nI < m_IncoreServices.numRecords; nI++)
  {
    if(m_IncoreServices.pRecordIDs[nI] == SERVICESrecordID)
    {
      for(nJ = 0; nJ < m_pIncoreRuns[nI].numRecords; nJ++)
      {
        if(m_pIncoreRuns[nI].pRuns[nJ].recordID == RUNSrecordID &&
              m_pIncoreRuns[nI].pRuns[nJ].flags == flags)
        {
          *pServiceIndex = nI;
          *pRunIndex = nJ;
          return(0);
        }
      }
      break;
    }
  }
  return(NO_RECORD);
}


//
//  Sort function for the Pegboard
//
int sort_Pegboard(const void *a, const void *b)
{
  PEGBOARDDef *pa, *pb;
  pa = (PEGBOARDDef *)a;
  pb = (PEGBOARDDef *)b;

  if(pa == NULL || pb == NULL)
  {
    return(0);
  }

  if(m_bPegboardSortFlags & PEGBOARD_FLAG_SORTBYBUS)
  {
    return(strcmp(pa->szBusNumber, pb->szBusNumber));
  }
  else if(m_bPegboardSortFlags & PEGBOARD_FLAG_SORTBYLOCATION)
  {
    return(strcmp(pa->szLocationNodeName, pb->szLocationNodeName));
  }
  else
  {
    if(pa->indexToBlockInfo == NO_RECORD || pb->indexToBlockInfo == NO_RECORD)
    {
      return(pa->indexToBlockInfo < pb->indexToBlockInfo ? 1 : pa->indexToBlockInfo > pb->indexToBlockInfo ? -1 : 0);   // Force to the end
    }

    BlockInfoDef *pBIa = &m_BlockInfo[pa->indexToBlockInfo];
    BlockInfoDef *pBIb = &m_BlockInfo[pb->indexToBlockInfo];

    if(pBIa == NULL || pBIb == NULL)
    {
      return(0);
    }


    if(m_bPegboardSortFlags & PEGBOARD_FLAG_SORTBYBLOCK)
    {
      return(pBIa->blockNumber < pBIb->blockNumber ? -1 : pBIa->blockNumber > pBIb->blockNumber ? 1 : 0);
    }
    else if(m_bPegboardSortFlags & PEGBOARD_FLAG_SORTBYRUN)
    {
      return(pBIa->startingRunNumber < pBIb->startingRunNumber ? -1 : pBIa->startingRunNumber > pBIb->startingRunNumber ? 1 : 0);
    }
    else if(m_bPegboardSortFlags & PEGBOARD_FLAG_SORTBYROSTER)
    {
      return(pBIa->startingRosterNumber < pBIb->startingRosterNumber ? -1 : pBIa->startingRosterNumber > pBIb->startingRosterNumber ? 1 : 0);
    }
    else
    {
      return(pBIa->PITime < pBIb->PITime ? -1 : pBIa->PITime > pBIb->PITime ? 1 : 0);
    }
  }
}

int sort_CheckIn(const void *a, const void *b)
{
  CHECKINDef *pa, *pb; 
  pa = (CHECKINDef *)a;
  pb = (CHECKINDef *)b;
  return(pa->reportAtTime < pb->reportAtTime ? -1 :
         pa->reportAtTime > pb->reportAtTime ?  1 : 0);
}

long YMDOffset(long year, long month, long day, long offset)
{
  CTime today(year, month, day, 0, 0, 0);
  today += CTimeSpan(offset, 0, 0, 0);

  long y, m, d;

  y = today.GetYear();
  m = today.GetMonth();
  d = today.GetDay();

  return(y * 10000 + m * 100 + d);
}

CTime m_SystemTime;
int   m_LastMinute;

void CALLBACK CDailyOps::TimerProc_Wrapper(HWND hWnd, UINT nMsg, UINT nIDEvent, DWORD dwTime)
{
  CDailyOps *pDO = (CDailyOps *)pObject;
  pDO->TimerProc(hWnd, nMsg, nIDEvent, dwTime);
}


void CALLBACK CDailyOps::TimerProc(HWND hWnd, UINT nMsg, UINT nIDEvent, DWORD dwTime)
{
  int rcode2;

  m_SystemTime = CTime::GetCurrentTime();

 	CString strTime = m_SystemTime.Format(_T("%A, %B %d, %Y at %I:%M:%S%p"));

  sprintf(strTitle, m_TitleString, strTime);
  ::SetWindowText(hWnd, strTitle);
//
//  Check to see if we need to suspend due to inactivity
//
  if(!m_bInitInProgress)
  {
    if(m_bCheckElapsed && !m_bAlreadyAsking)
    {
      if(m_SystemTime > m_PreviousTime + CTimeSpan(0, 0, 0, m_MaxElapse))
      {
        m_bAlreadyAsking = TRUE;
        m_Authentication.bAuthenticated = FALSE;
        m_Authentication.flags = SUPERVISOR_FLAG_DAILYOPS;
        if(!DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_SUPERVISOR),
              hWndMain, (DLGPROC)SUPERVISORMsgProc, (LPARAM)&m_Authentication))
        {
          OnCancel();
          return;
        }
        else
        {
          if(m_Authentication.bAuthenticated)
          {
            if(m_Authentication.bDisabled)
            {
              m_bCheckElapsed = FALSE;
            }
            else
            {
              sprintf(tempString, "%s is now signed in", m_Supervisors[m_UserID].szName);
              MessageBeep(MB_ICONINFORMATION);
              MessageBox(tempString, TMS, MB_OK);
              m_PreviousTime = m_SystemTime;
              DailyOpsBuildRecord(&DAILYOPS, DAILYOPS_FLAG_DISPATCHER);
              DAILYOPS.pertainsToDate = m_DailyOpsDateYYYYMMDD;
              DAILYOPS.pertainsToTime = m_DailyOpsDate.GetHour() * 3600 + m_DailyOpsDate.GetMinute() * 60 + m_DailyOpsDate.GetSecond();
              DAILYOPS.DRIVERSrecordID = NO_RECORD;
              DAILYOPS.DAILYOPSrecordID = NO_RECORD;
              DAILYOPS.recordFlags = DAILYOPS_FLAG_DISPATCHERSIGNIN;
              rcode2 = btrieve(B_INSERT, TMS_DAILYOPS, &DAILYOPS, &DAILYOPSKey0, 0);
              if(rcode2 == 0)
              {
                m_LastDAILYOPSRecordID = DAILYOPS.recordID;
              }
            }
          }
        }
        m_bAlreadyAsking = FALSE;
      }
    }
  }
//
//  Check for an update
//
  if(!m_bInitInProgress)
  {
    DAILYOPSDef DO;
    TMSKey0Def  DOKey0;
    rcode2 = btrieve(B_GETLAST, TMS_DAILYOPS, &DO, &DOKey0, 0);

    if(rcode2 == 0)
    {
      if(DO.recordID != m_LastDAILYOPSRecordID)
      {
        m_bMessageDisplay = FALSE;
        CDailyOps::ProcessExternalCommands();
        m_bMessageDisplay = TRUE;
      }
    }
  }
//
//  Position to the correct entry in the chronology
//
  long time = (m_SystemTime.GetHour() * 3600) + (m_SystemTime.GetMinute() * 60) + m_SystemTime.GetSecond();
  long itemTime;
  int  nI, nJ;

  nJ = pListBoxCHRONOLOGY->GetCount();

  for(nI = m_CurrentChronologyLine; nI < nJ; nI++)
  {
    itemTime = (long)pListBoxCHRONOLOGY->GetItemData(nI);
    if(time > itemTime)
    {
      pListBoxCHRONOLOGY->SetCurSel(nI);
      MessageBeep(MB_ICONINFORMATION);
      m_CurrentChronologyLine = nI + 1;
      break;
    }
  }
//
//  Check the check-ins every minute, if we're monitoring them
//
#ifdef DAILYOPSNOTIFY
  if(m_bMonitorCheckins)
  {
    DAILYOPSDef     DAILYOPS;
    TMSKey0Def      DAILYOPSKey0;
    DRIVERSDef      DRIVERS;
    TMSKey0Def      DRIVERSKey0;
    CString         s;
    char szDriver[DRIVERS_FIRSTNAME_LENGTH + 1 + DRIVERS_LASTNAME_LENGTH + 1];
    char szReportTime[16];
    char szSex[16];
    char tempString[TEMPSTRING_LENGTH];
    int  thisMinute = m_SystemTime.GetMinute();
    int  nI;

    if(thisMinute != m_LastMinute)
    {
      m_LastMinute = thisMinute;
      int now = m_SystemTime.GetHour() * 3600 + thisMinute * 60;
//
//  See if a first notification is required
//
      for(nI = m_CurrentCheckIn; nI < m_numInCheckIn; nI++)
      {
        if(m_CheckIn[nI].flags != 0)  // Any flag means he's checked-in, we're ignoring, or he's rec'd 1st or 2nd notification already
        {
          continue;
        }
        if(m_CheckIn[nI].reportAtTime - TIMEOF1STNOTIFICATION <= now)
        {
          if(m_CheckIn[nI].DRIVERSrecordID == NO_RECORD)
          {
            MessageBeep(MB_ICONINFORMATION);
            MessageBox("An open piece of work is scheduled out and has not been assigned", TMS, MB_ICONINFORMATION | MB_OK);
            m_CheckIn[nI].flags |= CHECKIN_FLAG_1STNOTIFICATION;
          }
          else
          {
            DRIVERSKey0.recordID = m_CheckIn[nI].DRIVERSrecordID;
            rcode2 = btrieve(B_GETEQUAL, TMS_DRIVERS, &DRIVERS, &DRIVERSKey0, 0);
            if(rcode2 != 0)
            {
              strcpy(szDriver, "?");
              DRIVERS.sex = 0;
            }
            else
            {
              strncpy(tempString, DRIVERS.firstName, DRIVERS_FIRSTNAME_LENGTH);
              trim(tempString, DRIVERS_FIRSTNAME_LENGTH);
              strcpy(szDriver, tempString);
              strcat(szDriver, " ");
              strncpy(tempString, DRIVERS.lastName, DRIVERS_LASTNAME_LENGTH);
              trim(tempString, DRIVERS_LASTNAME_LENGTH);
              strcat(szDriver, tempString);
            }
            strcpy(szReportTime, Tchar(m_CheckIn[nI].reportAtTime));
            strcpy(szSex, (DRIVERS.sex == 0 ? "him" : (DRIVERS.sex == 1 ? "her" : "them")));
            sprintf(tempString,
                  "%s\n\nFirst alert\n\n%s is due to report at %s\nand is not showing as checked-in\n\nDo you want to check %s in now?",
                  strTime, szDriver, szReportTime, szSex);
            MessageBeep(MB_ICONQUESTION);
            if(MessageBox(tempString, TMS, MB_ICONQUESTION | MB_YESNO) == IDYES)
            {
              DailyOpsBuildRecord(&DAILYOPS, DAILYOPS_FLAG_OPERATOR);
              DAILYOPS.pertainsToDate = m_DailyOpsDateYYYYMMDD;
              DAILYOPS.pertainsToTime = m_DailyOpsDate.GetHour() * 3600 + m_DailyOpsDate.GetMinute() * 60 + m_DailyOpsDate.GetSecond();
              DAILYOPS.DRIVERSrecordID = m_CheckIn[nI].DRIVERSrecordID;
              DAILYOPS.DAILYOPSrecordID = NO_RECORD;
              DAILYOPS.recordFlags = DAILYOPS_FLAG_OPERATORCHECKIN;
              rcode2 = btrieve(B_INSERT, TMS_DAILYOPS, &DAILYOPS, &DAILYOPSKey0, 0);
              if(rcode2 == 0)
              {
                m_LastDAILYOPSRecordID = DAILYOPS.recordID;
              }
              s.LoadString(TEXT_312);
              sprintf(tempString, s, m_SystemTime.Format(_T("%I:%M:%S%p")), BuildOperatorString(m_CheckIn[nI].DRIVERSrecordID));
              pEditMESSAGES->SetSel(-1, -1);
              pEditMESSAGES->ReplaceSel(tempString);
              m_CheckIn[nI].flags |= CHECKIN_FLAG_CHECKEDIN;
            }
            m_CheckIn[nI].flags |= CHECKIN_FLAG_1STNOTIFICATION;
          }
        }
      }
//
//  See if a second notification is required
//
    }
  }
#endif
}

//
//  DailyOpsBuildRecord()
//
//  Build the framework of a DAILYOPS record
//
//  Database verify: 11-Jan-07
//
void DailyOpsBuildRecord(DAILYOPSDef* pDO, int recordTypeFlag)
{
  DAILYOPSDef localDAILYOPS;
  TMSKey0Def  localDAILYOPSKey0;
  int   rcode2;
//
//  RecordID
//
  rcode2 = btrieve(B_GETLAST, TMS_DAILYOPS, &localDAILYOPS, &localDAILYOPSKey0, 0);
  pDO->recordID = AssignRecID(rcode2, localDAILYOPS.recordID);
//
//  Date and Time of Entry
//
  pDO->entryDateAndTime = m_SystemTime.GetTime();
//
// Pertains to date/time
//
  pDO->pertainsToDate = 0;
  pDO->pertainsToTime = NO_TIME;
//
//  user ID
//
  pDO->userID = m_UserID;
//
//  flags
//
  pDO->recordTypeFlag = (char)recordTypeFlag;
  pDO->recordFlags = 0;
//
//  DAILYOPSrecordID
//
  pDO->DAILYOPSrecordID = NO_RECORD;
//
//  DRIVERSrecordID
//
  pDO->DRIVERSrecordID = NO_RECORD;
//
//  Clear out the rest of the record
//
  memset(pDO->DOPS.associatedData, 0x00, DAILYOPS_ASSOCIATEDDATA_LENGTH);
}

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//
//  Sort functions for the Open Work Monitor
//
static BOOL bSortForward[7];

int CALLBACK DOOWListViewCompareProc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
  OPENWORKDef *p1 = (OPENWORKDef *)lParam1;
  OPENWORKDef *p2 = (OPENWORKDef *)lParam2;
  long number[2];
  char szString[2][DRIVERS_BADGENUMBER_LENGTH + DRIVERS_LASTNAME_LENGTH + DRIVERS_FIRSTNAME_LENGTH + 6];

  int  iResult = 0;

  if(p1 && p2)
  {
    switch(lParamSort)
    {
      case 0:  // Block number
        number[0] = p1->blockNumber;
        number[1] = p2->blockNumber;
        break;

      case 1:  // Run number
        number[0] = p1->runNumber;
        number[1] = p2->runNumber;
        break;

      case 2:  // Runtype
        strcpy(szString[0], p1->szCutAsRuntype);
        strcpy(szString[1], p2->szCutAsRuntype);
        break;

      case 3:  // On location
        strcpy(szString[0], p1->szOnNodesAbbrName);
        strcpy(szString[1], p2->szOnNodesAbbrName);
        break;

      case 4:  // On time
        number[0] = p1->onTime;
        number[1] = p2->onTime;
        break;

      case 5:  // Off location
        strcpy(szString[0], p1->szOffNodesAbbrName);
        strcpy(szString[1], p2->szOffNodesAbbrName);
        break;

      case 6:  // Off time
        number[0] = p1->offTime;
        number[1] = p2->offTime;
        break;

      case 7:  // Pay time
        number[0] = p1->pay;
        number[1] = p2->pay;
        break;

      case 8:  // Operator
        strcpy(szString[0], p1->szDriver);
        strcpy(szString[1], p2->szDriver);
        break;
    }

    switch(lParamSort)
    {
      case 0:  // Numerics
      case 1:
      case 4:
      case 6:
      case 7:
        if(bSortForward[lParamSort])
        {
          iResult = (number[0] < number[1] ? -1 :
                     number[0] > number[1] ? 1 : 0);
        }
        else
        {
          iResult = (number[0] > number[1] ? -1 :
                     number[0] < number[1] ? 1 : 0);
        }
        break;

      case 2:  // Alphas
      case 3:
      case 5:
      case 8: 
        iResult = bSortForward[lParamSort] ? 
              lstrcmp(szString[0], szString[1]) :
              lstrcmp(szString[1], szString[0]);
        break;
    }
  }

  return(iResult);
}
/////////////////////////////////////////////////////////////////////////////
// CDailyOps dialog


CDailyOps::CDailyOps(CWnd* pParent /*=NULL*/)
	: CDialog(CDailyOps::IDD, pParent)
{
  pListCtrlBLOCKLIST = NULL;
  m_BlockInfoShowFlags = BLOCKINFO_FLAG_DISPLAYALL;

  pFlatSBSCROLLBAR = NULL;
  m_pPEGBOARD = NULL;
  m_bPegboardShowFlags = PEGBOARD_FLAG_SHOWALL;
  m_bPegboardSortFlags = PEGBOARD_FLAG_SORTBYBUS;
  m_bMessageDisplay = TRUE;
  m_LastDAILYOPSRecordID = NO_RECORD;
  m_CurrentChronologyLine = 0;
  strcpy(m_szHolidayName, "");
  m_SERVICESrecordIDInEffect = NO_RECORD;
  m_PreviousTime = CTime::GetCurrentTime();
  m_MaxElapse = 600;  // 10 minutes
  m_PreviousTime -= CTimeSpan(0, 0, 0, m_MaxElapse);  // Force an intial validation
  m_bCheckElapsed = FALSE;
  m_bAlreadyAsking = FALSE;
  strcpy(m_Authentication.szMessage, "DailyOps Validation Required");
	//{{AFX_DATA_INIT(CDailyOps)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CDailyOps::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDailyOps)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDailyOps, CDialog)
	//{{AFX_MSG_MAP(CDailyOps)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDHELP, OnHelp)
	ON_NOTIFY(NM_RCLICK, DAILYOPS_OPENWORKLIST, OnRclickOpenworklist)
	ON_NOTIFY(NM_CLICK, DAILYOPS_BLOCKLIST, OnClickBlocklist)
	ON_NOTIFY(NM_CLICK, DAILYOPS_OPENWORKLIST, OnClickOpenworklist)
	ON_BN_CLICKED(IDCORRECTOPERATOR, OnCorrectoperator)
	ON_COMMAND(DAILYOPSOWM_PROPERTIES, OnOWMProperties)
	ON_COMMAND(IDREPORT, OnReport)
	ON_BN_CLICKED(IDCORRECTABSENCE, OnCorrectabsence)
	ON_COMMAND(IDSHOW, OnShow)
	ON_NOTIFY(NM_DBLCLK, DAILYOPS_OPENWORKLIST, OnDblclkOpenworklist)
	ON_BN_CLICKED(DAILYOPS_CHECKIN, OnCheckin)
	ON_BN_CLICKED(DAILYOPS_CHECKOUT, OnCheckout)
	ON_BN_CLICKED(DAILYOPS_EXTRATRIP, OnExtratrip)
	ON_COMMAND(IDSHOWHOURS, OnShowhours)
	ON_BN_CLICKED(IDCHECKINCHECKOUT, OnCheckincheckoutOperator)
	ON_WM_PAINT()
	ON_WM_CONTEXTMENU()
	ON_COMMAND(PEGBOARD_ALL, OnAll)
	ON_COMMAND(PEGBOARD_JUSTASSIGNED, OnJustassigned)
	ON_COMMAND(PEGBOARD_JUSTAVAILABLE, OnJustavailable)
	ON_COMMAND(PEGBOARD_JUSTOUTOFSERVICE, OnJustoutofservice)
	ON_COMMAND(PEGBOARD_CLEARALL, OnClearall)
	ON_COMMAND(PEGBOARD_PUTBACKINSERVICE, OnPutbackinservice)
	ON_COMMAND(PEGBOARD_ASSIGNBUS, OnAssignbus)
	ON_COMMAND(PEGBOARD_RETURN, OnReturn)
	ON_COMMAND(PEGBOARD_SWAP, OnSwap)
	ON_COMMAND(PEGBOARD_BLOCKNUMBER, OnBlocknumber)
	ON_COMMAND(PEGBOARD_RUNNUMBER, OnRunnumber)
	ON_COMMAND(PEGBOARD_ROSTERNUMBER, OnRosternumber)
	ON_COMMAND(PEGBOARD_PULLINTIME, OnPullintime)
	ON_COMMAND(PEGBOARD_BUSNUMBER, OnBusnumber)
	ON_WM_LBUTTONDBLCLK()
	ON_NOTIFY(NM_RCLICK, DAILYOPS_BLOCKLIST, OnRclickBlocklist)
	ON_COMMAND(DAILYOPSBLOCKS_DROPPED, OnDropped)
	ON_COMMAND(DAILYOPSBLOCKS_RETURNED, OnReturned)
	ON_COMMAND(DAILYOPSBLOCKS_TBA, OnTba)
	ON_COMMAND(DAILYOPSBLOCKS_PROPERTIES, OnBlocksProperties)
	ON_COMMAND(DAILYOPSBLOCKS_ASSIGNED, OnAssigned)
	ON_COMMAND(DAILYOPSBLOCKS_ALL, OnAllBlocks)
	ON_NOTIFY(NM_DBLCLK, DAILYOPS_BLOCKLIST, OnDblclkBlocklist)
	ON_COMMAND(DAILYOPSBLOCKS_DROP, OnDrop)
	ON_COMMAND(PEGBOARD_SETLOCATION, OnSetlocation)
	ON_BN_CLICKED(IDFUNCTIONS, OnFunctions)
	ON_COMMAND(DAILYOPSBLOCKS_OPERATORASSIGNMENTS, OnOperatorassignments)
	ON_COMMAND(PEGBOARD_MARKASSHORTSHIFT, OnMarkasshortshift)
	ON_COMMAND(PEGBOARD_MARKASCHARTER, OnMarkascharter)
	ON_COMMAND(PEGBOARD_MARKASSIGHTSEEING, OnMarkassightseeing)
	ON_COMMAND(DAILYOPSOWM_RUNSPLIT, OnRunsplit)
	ON_COMMAND(DAILYOPSOWM_RECOST, OnRecost)
	ON_COMMAND(PEGBOARD_BUSLOCATION, OnBuslocation)
	ON_BN_CLICKED(IDREGISTER, OnRegister)
	ON_BN_CLICKED(IDDEASSIGN, OnDeassign)
	ON_LBN_SELCHANGE(DAILYOPS_CHRONOLOGY, OnSelchangeChronology)
	ON_LBN_DBLCLK(DAILYOPS_CHRONOLOGY, OnDblclkChronology)
	ON_COMMAND(DAILYOPSOWM_INCLUDEHOURS, OnIncludehours)
	ON_EN_CHANGE(DAILYOPS_OVERTIMETIME, OnChangeOvertimetime)
	ON_BN_CLICKED(DAILYOPS_OPENWORKDISPLAY, OnOpenworkdisplay)
	ON_BN_CLICKED(IDCHECKIN, OnCheckin)
	ON_BN_CLICKED(IDCHECKOUT, OnCheckout)
	ON_NOTIFY(LVN_COLUMNCLICK, DAILYOPS_OPENWORKLIST, OnColumnclickOpenworklist)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

BEGIN_EVENTSINK_MAP(CDailyOps, CDialog)
    //{{AFX_EVENTSINK_MAP(CDailyOps)
	ON_EVENT(CDailyOps, DAILYOPS_SCROLLBAR, 1 /* Change */, OnChangeScrollbar, VTS_NONE)
	ON_EVENT(CDailyOps, DAILYOPS_SCROLLBAR, 2 /* Scroll */, OnScrollScrollbar, VTS_NONE)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDailyOps message handlers

BOOL CDailyOps::OnInitDialog() 
{
	CDialog::OnInitDialog();
  
//
//  Set up pointers to the controls
//
//  Messages and Chronology
//
  pEditMESSAGES = (CEdit *)GetDlgItem(DAILYOPS_MESSAGES);
  pListBoxCHRONOLOGY = (CListBox *)GetDlgItem(DAILYOPS_CHRONOLOGY);
  CFont* pFont = pEditMESSAGES->GetFont();
  TEXTMETRIC tm;
  CDC* pDC = pEditMESSAGES->GetDC();
  CFont* pOldFont = pDC->SelectObject(pFont);
  pDC->GetTextMetrics(&tm);
  pDC->SelectObject(pOldFont);
  CRect rect(0, 0, 100, 1);
  MapDialogRect(rect);
  pEditMESSAGES->SetTabStops((14 * tm.tmAveCharWidth * 100) / rect.Width());
  pListBoxCHRONOLOGY->SetTabStops((6 * tm.tmAveCharWidth * 100) / rect.Width());

//
//  Vehicle assignment
//
  pButtonVEHICLEASSIGNMENT = (CButton *)GetDlgItem(DAILYOPS_VEHICLEASSIGNMENT);
  pListCtrlBLOCKLIST = (CListCtrl *)GetDlgItem(DAILYOPS_BLOCKLIST);
  pFlatSBSCROLLBAR = (CFlatSB *)GetDlgItem(DAILYOPS_SCROLLBAR);

//
//  Operator
//
  pComboBoxOPERATOR = (CComboBox *)GetDlgItem(DAILYOPS_OPERATOR);

//
//  Operator Absence
//
  pButtonIDREGISTER = (CButton *)GetDlgItem(IDREGISTER);
  pButtonIDCORRECTABSENCE = (CButton *)GetDlgItem(IDCORRECTABSENCE);

//
//  Operator check-in/out and deassign
//
  pButtonCHECKIN = (CButton *)GetDlgItem(DAILYOPS_CHECKIN);
  pButtonCHECKOUT = (CButton *)GetDlgItem(DAILYOPS_CHECKOUT);
  pButtonDEASSIGN = (CButton *)GetDlgItem(DAILYOPS_DEASSIGN);

//
//  Extras
//
  pButtonOVERTIME = (CButton *)GetDlgItem(DAILYOPS_OVERTIME);
  pButtonEXTRATRIP = (CButton *)GetDlgItem(DAILYOPS_EXTRATRIP);
  pEditOVERTIMETIME = (CEdit *)GetDlgItem(DAILYOPS_OVERTIMETIME);
  pButtonIDCHECKINCHECKOUT = (CButton *)GetDlgItem(IDCHECKINCHECKOUT);
  pButtonIDCORRECTOPERATOR = (CButton *)GetDlgItem(IDCORRECTOPERATOR);
  pComboBoxEXTRAREASON = (CComboBox *)GetDlgItem(DAILYOPS_EXTRAREASON);
//
//  Open Work Monitor
//
  pButtonOPENWORKMONITOR = (CButton *)GetDlgItem(DAILYOPS_OPENWORKMONITOR);
  pListCtrlOPENWORKLIST = (CListCtrl *)GetDlgItem(DAILYOPS_OPENWORKLIST);
  pButtonOPENWORKDISPLAY = (CButton *)GetDlgItem(DAILYOPS_OPENWORKDISPLAY);
//
//  Standard controls
//
  pButtonIDOK = (CButton *)GetDlgItem(IDOK);
  pButtonIDHELP = (CButton *)GetDlgItem(IDHELP);
//
//  Debugging
//
  pStaticXY = (CStatic *)GetDlgItem(DAILYOPS_XY);
//
//  Set up the bus portion
//
  DWORD dwExStyles = pListCtrlBLOCKLIST->GetExtendedStyle();
  pListCtrlBLOCKLIST->SetExtendedStyle(dwExStyles | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_INFOTIP);

  m_bOverrideInProgress = FALSE;
  m_bInitInProgress = TRUE;
//
//  Set up the semaphore
//
  pObject = this;

  m_bSemaphoreUpdate = FALSE;
//
//  Get the current date and time and put it on the group box
//  
  m_SystemTime = CTime::GetCurrentTime();
  m_LastMinute = m_SystemTime.GetMinute();
	
  CString strTime = m_SystemTime.Format(_T("%A, %B %d, %Y at %I:%M:%S%p"));
//
//  Set up the timer to update the date/time
//
  GetWindowText(m_TitleString, sizeof(m_TitleString));
  sprintf(strTitle, m_TitleString, strTime);
  SetWindowText(strTitle);
  pTimer = SetTimer(NULL, 1000, TimerProc_Wrapper);
//
//  Initialize the m_pRData structure
//
  char dummy[256];
  long maxRuns;
  int  rcode2;

  rcode2 = btrieve(B_STAT, TMS_RUNS, &BSTAT, dummy, 0);
  if(rcode2 != 0 || BSTAT.numRecords == 0)
  {
    TMSError((HWND)NULL, MB_ICONSTOP, ERROR_266, (HANDLE)NULL);
    Cleanup();
    return TRUE;
  }
  maxRuns = BSTAT.numRecords * 2;
  m_pRData = (RDataDef *)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(RDataDef) * maxRuns); 
  if(m_pRData == NULL)
  {
    AllocationError(__FILE__, __LINE__, FALSE);
    Cleanup();
    return TRUE;
  }
  m_pRDataHold = (RDataDef *)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(RDataDef) * maxRuns); 
  if(m_pRDataHold == NULL)
  {
    AllocationError(__FILE__, __LINE__, FALSE);
    Cleanup();
    return TRUE;
  }

  int nI;
  int nJ;
  int nK;

//
//  Set now as the date and time
//
  m_DailyOpsDate = m_SystemTime;
  m_DailyOpsDateYYYYMMDD = m_SystemTime.GetYear() * 10000 + m_SystemTime.GetMonth() * 100 + m_SystemTime.GetDay();

//
//  Make sure a roster template's been established
//
//  Database verify: 11-Jan-07
//
  CString s;
  char    szMostRecent[16];
  long    year, month, day;
  BOOL    bFound;

  DAILYOPSKey1.recordTypeFlag = DAILYOPS_FLAG_ROSTER;
  DAILYOPSKey1.pertainsToDate = m_DailyOpsDateYYYYMMDD;
  DAILYOPSKey1.pertainsToTime = m_SystemTime.GetHour() * 3600 + m_SystemTime.GetMinute() * 60 + m_SystemTime.GetSecond();
  DAILYOPSKey1.recordFlags = 0;
  rcode2 = btrieve(B_GETLESSTHANOREQUAL, TMS_DAILYOPS, &DAILYOPS, &DAILYOPSKey1, 1);
  bFound = FALSE;
  while(rcode2 == 0 &&
        (DAILYOPS.recordTypeFlag & DAILYOPS_FLAG_ROSTER))
  {
    if(DAILYOPS.recordFlags & DAILYOPS_FLAG_ROSTERESTABLISH)
    {
      bFound = TRUE;
      break;
    }
    rcode2 = btrieve(B_GETPREVIOUS, TMS_DAILYOPS, &DAILYOPS, &DAILYOPSKey1, 1);
  }
  if(bFound)
  {
    GetYMD(DAILYOPS.pertainsToDate, &year, &month, &day);
    sprintf(szMostRecent, "%02ld-%s-%04ld", day, szMonth[month - 1], year);
    m_DailyOpsROSTERDivisionInEffect = DAILYOPS.DOPS.RosterTemplate.ROSTERDIVISIONSrecordID;
    m_DailyOpsRUNSDivisionInEffect = DAILYOPS.DOPS.RosterTemplate.RUNSDIVISIONSrecordID;
    m_MostRecentTemplate = DAILYOPS.pertainsToDate;
  }
  else
  {
    s.LoadString(ERROR_317);
    sprintf(tempString, s, m_SystemTime.Format(_T("%I:%M:%S%p")));
    pEditMESSAGES->SetSel(-1, -1);
    pEditMESSAGES->ReplaceSel(tempString);
    LoadString(hInst, TEXT_302, szMostRecent, sizeof(szMostRecent));
    m_DailyOpsROSTERDivisionInEffect = m_DivisionRecordID;
    m_DailyOpsRUNSDivisionInEffect = m_DivisionRecordID;
    m_MostRecentTemplate = NO_RECORD;
    pButtonOPENWORKMONITOR->EnableWindow(FALSE);
    pListCtrlOPENWORKLIST->EnableWindow(FALSE);

  }
  LoadString(hInst, TEXT_300, tempString, TEMPSTRING_LENGTH);
  LoadString(hInst, TEXT_304, szFormatString, SZFORMATSTRING_LENGTH);
  sprintf(szarString, szFormatString, tempString, szMostRecent);
  pButtonOPENWORKMONITOR->SetWindowText(szarString);
//
//  Allocate space for the incore services
//
//  Get the number of services
//
  rcode2 = btrieve(B_STAT, TMS_SERVICES, &BSTAT, dummy, 0);
  if(rcode2 != 0 || BSTAT.numRecords == 0)
  {
    TMSError((HWND)NULL, MB_ICONSTOP, ERROR_007, (HANDLE)NULL);
    Cleanup();
    return TRUE;
  }
  m_IncoreServices.numRecords = BSTAT.numRecords;
//
//  Allocate space for the service recordIDs
//
  m_IncoreServices.pRecordIDs = (long *)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(long) * m_IncoreServices.numRecords); 
  if(m_IncoreServices.pRecordIDs == NULL)
  {
    AllocationError(__FILE__, __LINE__, FALSE);
    Cleanup();
    return TRUE;
  }
//
//  Allocate space for the incore runs pointers
//
  m_pIncoreRuns = (IncoreDef *)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(IncoreDef) * m_IncoreServices.numRecords); 
  if(m_pIncoreRuns == NULL)
  {
    AllocationError(__FILE__, __LINE__, FALSE);
    Cleanup();
    return TRUE;
  }
//
//  Read the services into the m_IncoreServices structure
//  allocate space for the m_m_pIncoreRuns pointers
//
  nI = 0;
  rcode2 = btrieve(B_GETFIRST, TMS_SERVICES, &SERVICES, &SERVICESKey0, 0);
  while(rcode2 == 0)
  {
//
//  Runs
//
    RUNSKey1.DIVISIONSrecordID = m_DailyOpsRUNSDivisionInEffect;
    RUNSKey1.SERVICESrecordID = SERVICES.recordID;
    RUNSKey1.runNumber = NO_RECORD;
    RUNSKey1.runNumber = NO_RECORD;
    nJ = 0;
    rcode2 = btrieve(B_GETGREATER, TMS_RUNS, &RUNS, &RUNSKey1, 1);
    while(rcode2 == 0 &&
          RUNS.DIVISIONSrecordID == m_DailyOpsRUNSDivisionInEffect &&
          RUNS.SERVICESrecordID == SERVICES.recordID)
    {
      if(RUNS.pieceNumber == 1)
      {
        nJ++;
      }
      rcode2 = btrieve(B_GETNEXT, TMS_RUNS, &RUNS, &RUNSKey1, 1);
    }
//
//  ...and crew-only runs (note fall-through of nJ)
//
    CREWONLYKey1.DIVISIONSrecordID = m_DailyOpsRUNSDivisionInEffect;
    CREWONLYKey1.SERVICESrecordID = SERVICES.recordID;
    CREWONLYKey1.runNumber = NO_RECORD;
    CREWONLYKey1.runNumber = NO_RECORD;
    rcode2 = btrieve(B_GETGREATER, TMS_CREWONLY, &CREWONLY, &CREWONLYKey1, 1);
    while(rcode2 == 0 &&
          CREWONLY.DIVISIONSrecordID == m_DailyOpsRUNSDivisionInEffect &&
          CREWONLY.SERVICESrecordID == SERVICES.recordID)
    {
      rcode2 = btrieve(B_GETNEXT, TMS_CREWONLY, &CREWONLY, &CREWONLYKey1, 1);
      nJ++;  // There's no guarantee that he'll label his crewonlys as starting at piece 1 so don't take chances
    }
//
//  Allocate space for the incore runs recordIDs
//
    m_pIncoreRuns[nI].pRecordIDs = (long *)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(long) * nJ); 
    if(m_pIncoreRuns[nI].pRecordIDs == NULL)
    {
      AllocationError(__FILE__, __LINE__, FALSE);
      Cleanup();
      return TRUE;
    }
//
//  Allocate space for the incore run details
//
    m_pIncoreRuns[nI].pRuns = (RunsDef *)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(RunsDef) * nJ); 
    if(m_pIncoreRuns[nI].pRuns == NULL)
    {
      AllocationError(__FILE__, __LINE__, FALSE);
      Cleanup();
      return TRUE;
    }
    m_pIncoreRuns[nI].numRecords = nJ;
//
//  Services
//
    m_IncoreServices.pRecordIDs[nI] = SERVICES.recordID;
    rcode2 = btrieve(B_GETNEXT, TMS_SERVICES, &SERVICES, &SERVICESKey0, 0);
    nI++;
  }
//
//  Read the runs into m_pIncoreRuns and fill the RunsStruct structure
//
  PROPOSEDRUNDef PROPOSEDRUN;
  COSTDef COST;

  for(nI = 0; nI < m_IncoreServices.numRecords; nI++)
  {
    RUNSKey1.DIVISIONSrecordID = m_DailyOpsRUNSDivisionInEffect;
    RUNSKey1.SERVICESrecordID = m_IncoreServices.pRecordIDs[nI];
    RUNSKey1.runNumber = NO_RECORD;
    RUNSKey1.runNumber = NO_RECORD;
    nJ = 0;
    rcode2 = btrieve(B_GETGREATER, TMS_RUNS, &RUNS, &RUNSKey1, 1);
    while(rcode2 == 0 && 
          RUNS.DIVISIONSrecordID == m_DailyOpsRUNSDivisionInEffect &&
          RUNS.SERVICESrecordID == m_IncoreServices.pRecordIDs[nI])
    {
      if(RUNS.pieceNumber == 1)
      {
        m_pIncoreRuns[nI].pRuns[nJ].recordID = RUNS.recordID;
        m_pIncoreRuns[nI].pRuns[nJ].runNumber = RUNS.runNumber;
        m_pIncoreRuns[nI].pRuns[nJ].cutAsRuntype = RUNS.cutAsRuntype;
        SetupRun(m_pIncoreRuns[nI].pRuns[nJ].recordID, FALSE, &PROPOSEDRUN);
        m_pIncoreRuns[nI].pRuns[nJ].numPieces = PROPOSEDRUN.numPieces;
        RunCoster(&PROPOSEDRUN, m_pIncoreRuns[nI].pRuns[nJ].cutAsRuntype, &COST);
        m_pIncoreRuns[nI].pRuns[nJ].startTRIPSrecordID = PROPOSEDRUN.piece[0].fromTRIPSrecordID;
        m_pIncoreRuns[nI].pRuns[nJ].startNODESrecordID = PROPOSEDRUN.piece[0].fromNODESrecordID;
        NODESKey0.recordID = m_pIncoreRuns[nI].pRuns[nJ].startNODESrecordID;
        rcode2 = btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
        if(rcode2 == 0)
        {
          strncpy(m_pIncoreRuns[nI].pRuns[nJ].szStartNodeName, NODES.abbrName, NODES_ABBRNAME_LENGTH);
          trim(m_pIncoreRuns[nI].pRuns[nJ].szStartNodeName, NODES_ABBRNAME_LENGTH);
        }
        else
        {
          strcpy(m_pIncoreRuns[nI].pRuns[nJ].szStartNodeName, "?");
        }
        m_pIncoreRuns[nI].pRuns[nJ].startTime = PROPOSEDRUN.piece[0].fromTime;
        nK = PROPOSEDRUN.numPieces - 1;
        m_pIncoreRuns[nI].pRuns[nJ].endTRIPSrecordID = PROPOSEDRUN.piece[nK].fromTRIPSrecordID;
        m_pIncoreRuns[nI].pRuns[nJ].endNODESrecordID = PROPOSEDRUN.piece[nK].toNODESrecordID;
        NODESKey0.recordID = m_pIncoreRuns[nI].pRuns[nJ].endNODESrecordID;
        rcode2 = btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
        if(rcode2 == 0)
        {
          strncpy(m_pIncoreRuns[nI].pRuns[nJ].szEndNodeName, NODES.abbrName, NODES_ABBRNAME_LENGTH);
          trim(m_pIncoreRuns[nI].pRuns[nJ].szEndNodeName, NODES_ABBRNAME_LENGTH);
        }
        else
        {
          strcpy(m_pIncoreRuns[nI].pRuns[nJ].szEndNodeName, "?");
        }
        m_pIncoreRuns[nI].pRuns[nJ].endTime = PROPOSEDRUN.piece[nK].toTime;
        m_pIncoreRuns[nI].pRuns[nJ].payTime = COST.TOTAL.payTime;
        m_pIncoreRuns[nI].pRuns[nJ].flags = 0;
        RUNSKey0.recordID = m_pIncoreRuns[nI].pRuns[nJ].recordID;
        rcode2 = btrieve(B_GETEQUAL, TMS_RUNS, &RUNS, &RUNSKey0, 0);
        rcode2 = btrieve(B_GETPOSITION, TMS_RUNS, &RUNS, &RUNSKey0, 0);
        rcode2 = btrieve(B_GETDIRECT, TMS_RUNS, &RUNS, &RUNSKey1, 1);
        nJ++;
      }
      rcode2 = btrieve(B_GETNEXT, TMS_RUNS, &RUNS, &RUNSKey1, 1);
    }
//
//  Read the crew-only runs into m_pRData
//
    long pieceNumber;
    long RUNSrecordID;
    long runNumber;
    long serviceRecordID;

    CREWONLYKey1.DIVISIONSrecordID = m_DailyOpsRUNSDivisionInEffect;
    CREWONLYKey1.SERVICESrecordID = m_IncoreServices.pRecordIDs[nI];;
    CREWONLYKey1.runNumber = NO_RECORD;
    CREWONLYKey1.pieceNumber = NO_RECORD;
    rcode2 = btrieve(B_GETGREATER, TMS_CREWONLY, &CREWONLY, &CREWONLYKey1, 1);
    while(rcode2 == 0 && 
          CREWONLY.DIVISIONSrecordID == m_DailyOpsRUNSDivisionInEffect &&
          CREWONLY.SERVICESrecordID == m_IncoreServices.pRecordIDs[nI])
    {
      if(CREWONLY.pieceNumber == 1)
      {
        m_pIncoreRuns[nI].pRuns[nJ].recordID = CREWONLY.recordID;
        m_pIncoreRuns[nI].pRuns[nJ].runNumber = CREWONLY.runNumber;
        m_pIncoreRuns[nI].pRuns[nJ].cutAsRuntype = CREWONLY.cutAsRuntype;
        pieceNumber = 0;
        RUNSrecordID = CREWONLY.recordID;
        runNumber = CREWONLY.runNumber;
        serviceRecordID = CREWONLY.SERVICESrecordID;
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
        m_pIncoreRuns[nI].pRuns[nJ].numPieces = pieceNumber;
        RunCoster(&PROPOSEDRUN, m_pIncoreRuns[nI].pRuns[nJ].cutAsRuntype, &COST);
        m_pIncoreRuns[nI].pRuns[nJ].startTRIPSrecordID = NO_RECORD;
        m_pIncoreRuns[nI].pRuns[nJ].startNODESrecordID = PROPOSEDRUN.piece[0].fromNODESrecordID;
        NODESKey0.recordID = m_pIncoreRuns[nI].pRuns[nJ].startNODESrecordID;
        rcode2 = btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
        if(rcode2 == 0)
        {
          strncpy(m_pIncoreRuns[nI].pRuns[nJ].szStartNodeName, NODES.abbrName, NODES_ABBRNAME_LENGTH);
          trim(m_pIncoreRuns[nI].pRuns[nJ].szStartNodeName, NODES_ABBRNAME_LENGTH);
        }
        else
        {
          strcpy(m_pIncoreRuns[nI].pRuns[nJ].szStartNodeName, "?");
        }
        nK = pieceNumber - 1;
        m_pIncoreRuns[nI].pRuns[nJ].startTime = PROPOSEDRUN.piece[0].fromTime;
        m_pIncoreRuns[nI].pRuns[nJ].endTRIPSrecordID = NO_RECORD;
        m_pIncoreRuns[nI].pRuns[nJ].endNODESrecordID = PROPOSEDRUN.piece[nK].toNODESrecordID;
        NODESKey0.recordID = m_pIncoreRuns[nI].pRuns[nJ].endNODESrecordID;
        rcode2 = btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
        if(rcode2 == 0)
        {
          strncpy(m_pIncoreRuns[nI].pRuns[nJ].szEndNodeName, NODES.abbrName, NODES_ABBRNAME_LENGTH);
          trim(m_pIncoreRuns[nI].pRuns[nJ].szEndNodeName, NODES_ABBRNAME_LENGTH);
        }
        else
        {
          strcpy(m_pIncoreRuns[nI].pRuns[nJ].szEndNodeName, "?");
        }
        m_pIncoreRuns[nI].pRuns[nJ].endTime = PROPOSEDRUN.piece[nK].toTime;
        m_pIncoreRuns[nI].pRuns[nJ].payTime = COST.TOTAL.payTime;
        m_pIncoreRuns[nI].pRuns[nJ].flags = RDATA_FLAG_CREWONLY;
        CREWONLYKey0.recordID = m_pIncoreRuns[nI].pRuns[nJ].recordID;
        rcode2 = btrieve(B_GETEQUAL, TMS_CREWONLY, &CREWONLY, &CREWONLYKey0, 0);
        rcode2 = btrieve(B_GETPOSITION, TMS_CREWONLY, &CREWONLY, &CREWONLYKey0, 0);
        rcode2 = btrieve(B_GETDIRECT, TMS_CREWONLY, &CREWONLY, &CREWONLYKey1, 1);
        nJ++;
      }
      rcode2 = btrieve(B_GETNEXT, TMS_CREWONLY, &CREWONLY, &CREWONLYKey1, 1);
    }
  }
//
//  Set up the bustypes
// 	
  if(SetUpBustypeList(m_hWnd, DAILYOPS_BUSTYPES) == 0)
  {
    TMSError(m_hWnd, MB_ICONSTOP, ERROR_130, (HANDLE)NULL);
    Cleanup();
    return TRUE;
  }
//
//  Start the status bar
//
  LoadString(hInst, TEXT_270, tempString, TEMPSTRING_LENGTH);
  StatusBarStart(hWndMain, tempString);
  LoadString(hInst, TEXT_271, tempString, TEMPSTRING_LENGTH);
  StatusBarText(tempString);
//
//  Set up the operator portion
//
  LoadString(hInst, TEXT_272, tempString, TEMPSTRING_LENGTH);
  StatusBarText(tempString);
//
//  Load the drivers
//
  SetupDriverComboBox();
//
//  Set up the absence portion
//
  COleVariant v;
  CString inputLine;
  
  LoadString(hInst, TEXT_273, tempString, TEMPSTRING_LENGTH);
  StatusBarText(tempString);
//
//  Set up the extra time reasons
//
  for(nI = 0; nI < m_numExtraTimeReasons; nI++)
  {
    nJ = pComboBoxEXTRAREASON->AddString(m_ExtraTimeReasons[nI].szText);
    pComboBoxEXTRAREASON->SetItemData(nJ, m_ExtraTimeReasons[nI].ruleNumber);
  }
//
//  Set up the Open Work Monitor
//
  dwExStyles = pListCtrlOPENWORKLIST->GetExtendedStyle();
  pListCtrlOPENWORKLIST->SetExtendedStyle(dwExStyles | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_INFOTIP);
//
//  Open Work list control
//
  LVCOLUMN LVC;

  LVC.mask = LVCF_FMT | LVCF_TEXT | LVCF_WIDTH;
  LVC.cx = 60;
  LVC.pszText = "Block";
  pListCtrlOPENWORKLIST->InsertColumn(0, &LVC);

  LVC.cx = 50;
  LVC.fmt = LVCFMT_CENTER;
  LVC.pszText = "Run";
  pListCtrlOPENWORKLIST->InsertColumn(1, &LVC);

  LVC.cx = 85;
  LVC.pszText = "Runtype";
  pListCtrlOPENWORKLIST->InsertColumn(2, &LVC);

  LVC.cx = 40;
  LVC.pszText = "On";
  pListCtrlOPENWORKLIST->InsertColumn(3, &LVC);

  LVC.cx = 45;
  LVC.pszText = "At";
  pListCtrlOPENWORKLIST->InsertColumn(4, &LVC);
 
  LVC.cx = 40;
  LVC.pszText = "Off";
  pListCtrlOPENWORKLIST->InsertColumn(5, &LVC);

  LVC.cx = 45;
  LVC.pszText = "At";
  pListCtrlOPENWORKLIST->InsertColumn(6, &LVC);
 
  LVC.cx = 48;
  LVC.pszText = "Pay";
  pListCtrlOPENWORKLIST->InsertColumn(7, &LVC);

  LVC.cx = 208;
  LVC.pszText = "Assigned to";
  pListCtrlOPENWORKLIST->InsertColumn(8, &LVC);
//
//  Default to displaying both open and available work
//
  pButtonOPENWORKDISPLAY->SetCheck(TRUE);
  m_bOpenWorkDisplay = TRUE;
//
//  Display any open work
//
  m_numAbsent = RefreshAbsenceList(m_DailyOpsDateYYYYMMDD, m_AbsentList);
  PopulateOpenWorkMonitor(NO_RECORD, TRUE, TRUE);
  m_SelectedOWRow = 0;
//
//  Set up the blocks and pegboard.  This has to be done after
//  we set up the open work montitor 'cause m_OpenWork has
//  information we'll next when setting up m_BlockInfo.
//
  LVC.mask = LVCF_FMT | LVCF_TEXT | LVCF_WIDTH;
//
//  Block list
//
  LVC.cx = 20;
  LVC.fmt = LVCFMT_CENTER;
  LVC.pszText = "*";
  pListCtrlBLOCKLIST->InsertColumn(0, &LVC);

  LVC.cx = 54;
  LVC.fmt = LVCFMT_CENTER;
  LVC.pszText = "Vehicle";
  pListCtrlBLOCKLIST->InsertColumn(1, &LVC);

  LVC.cx = 45;
  LVC.fmt = LVCFMT_CENTER;
  LVC.pszText = "Block";
  pListCtrlBLOCKLIST->InsertColumn(2, &LVC);

  LVC.cx = 45;
  LVC.fmt = LVCFMT_CENTER;
  LVC.pszText = "Run";
  pListCtrlBLOCKLIST->InsertColumn(3, &LVC);

  LVC.cx = 50;
  LVC.fmt = LVCFMT_CENTER;
  LVC.pszText = "Roster";
  pListCtrlBLOCKLIST->InsertColumn(4, &LVC);

  LVC.cx = 160;
  LVC.fmt = LVCFMT_CENTER;
  LVC.pszText = "Starting Operator";
  pListCtrlBLOCKLIST->InsertColumn(5, &LVC);

  LVC.cx = 45;
  LVC.fmt = LVCFMT_CENTER;
  LVC.pszText = "Out";
  pListCtrlBLOCKLIST->InsertColumn(6, &LVC);

  LVC.cx = 50;
  LVC.fmt = LVCFMT_CENTER;
  LVC.pszText = "From";
  pListCtrlBLOCKLIST->InsertColumn(7, &LVC);
 
  LVC.cx = 50;
  LVC.fmt = LVCFMT_CENTER;
  LVC.pszText = "To";
  pListCtrlBLOCKLIST->InsertColumn(8, &LVC);
 
  LVC.cx = 45;
  LVC.fmt = LVCFMT_CENTER;
  LVC.pszText = "In";
  pListCtrlBLOCKLIST->InsertColumn(9, &LVC);
 
  LVC.cx = 45;
  LVC.fmt = LVCFMT_RIGHT;
  LVC.pszText = "Dist";
  pListCtrlBLOCKLIST->InsertColumn(10, &LVC);
//
//  Set up the blocks and buses
//
  LoadString(hInst, TEXT_380, tempString, TEMPSTRING_LENGTH);
  StatusBarText(tempString);
//
//  Set the maximum value for the scroll bar and the jump increment
//
  short int nChange = BUSROWSDISPLAYED - 1;

  rcode2 = btrieve(B_STAT, TMS_BUSES, &BSTAT, dummy, 0);
  m_numBuses = (int)(rcode2 == 0 ? BSTAT.numRecords : 0);

  nI = (m_numBuses / 10 + 1) - BUSROWSDISPLAYED;
  nI = (nI < 0) ? 0 : nI;
  pFlatSBSCROLLBAR->SetMax(max(nI, nChange));
  pFlatSBSCROLLBAR->SetLargeChange(nChange);
  pFlatSBSCROLLBAR->Refresh();
//
//  Allocate space for the PEGBOARD structure
//
  m_pPEGBOARD = (PEGBOARDDef *)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(PEGBOARDDef) * m_numBuses); 
  if(m_pPEGBOARD == NULL)
  {
    AllocationError(__FILE__, __LINE__, FALSE);
    return(TRUE);
  }
//
//  Get the PEGBOARD structure set up
//
//  Get the bus inventory
//
  nI = 0;
  rcode2 = btrieve(B_GETFIRST, TMS_BUSES, &BUSES, &BUSESKey1, 1);
  while(rcode2 == 0)
  {
    if(!(BUSES.flags & BUSES_FLAG_RETIRED))
    {
      m_pPEGBOARD[nI].BUSESrecordID = BUSES.recordID;
      strncpy(m_pPEGBOARD[nI].szBusNumber, BUSES.number, BUSES_NUMBER_LENGTH);
      trim(m_pPEGBOARD[nI].szBusNumber, BUSES_NUMBER_LENGTH);
      m_pPEGBOARD[nI].DAILYOPSrecordID = NO_RECORD;
      m_pPEGBOARD[nI].DRIVERSrecordID = NO_RECORD;
      m_pPEGBOARD[nI].indexToBlockInfo = NO_RECORD;
      m_pPEGBOARD[nI].flags = 0;
      m_pPEGBOARD[nI].swapReasonIndex = NO_RECORD;
      m_pPEGBOARD[nI].locationNODESrecordID = NO_RECORD;
      strcpy(m_pPEGBOARD[nI].szLocationNodeName, "");
      nI++;
    }
    rcode2 = btrieve(B_GETNEXT, TMS_BUSES, &BUSES, &BUSESKey1, 1);
  }
  m_numBuses = nI;
//
//  Set the starting position of the display
//
  m_FirstBusRowDisplayed = 0;
  m_LastBusIndex = NO_RECORD;
//
//  Set up the pens
//
  m_Pens[DO_AVAILABLE].CreatePen(         PS_SOLID, 1,     m_PBCOLORS.crAvailable);
  m_Pens[DO_INUSE].CreatePen(             PS_SOLID, 1,     m_PBCOLORS.crAssigned); 
  m_Pens[DO_OUTOFSERVICE].CreatePen(      PS_SOLID, 1,     m_PBCOLORS.crOutOfService);
  m_Pens[DO_SHORTSHIFT].CreatePen(        PS_SOLID, 1,     m_PBCOLORS.crShortShift);
  m_Pens[DO_CHARTERRESERVE].CreatePen(    PS_SOLID, 1,     m_PBCOLORS.crCharterReserve);
  m_Pens[DO_SIGHTSEEINGRESERVE].CreatePen(PS_SOLID, 1,     m_PBCOLORS.crSightseeingReserve);
  m_Pens[DO_THICKTXT].CreatePen(          PS_SOLID, THICK, RGB(  0,   0,   0));  // Black thick
  m_Pens[DO_TXT].CreatePen(               PS_SOLID, 1,     RGB(  0,   0,   0));  // Black thin
  m_Pens[DO_BORDER].CreatePen(            PS_SOLID, 1,     RGB(255, 255, 255));  // White thin
//
//  And brushes
//
  LOGBRUSH logBrush;
    
  logBrush.lbStyle = BS_SOLID;
  logBrush.lbHatch = 0;

  logBrush.lbColor = m_PBCOLORS.crAvailable;
  m_Brushes[DO_AVAILABLE].CreateBrushIndirect(&logBrush);
  logBrush.lbColor = m_PBCOLORS.crAssigned;
  m_Brushes[DO_INUSE].CreateBrushIndirect(&logBrush);
  logBrush.lbColor = m_PBCOLORS.crOutOfService;
  m_Brushes[DO_OUTOFSERVICE].CreateBrushIndirect(&logBrush);
  logBrush.lbColor = m_PBCOLORS.crShortShift;
  m_Brushes[DO_SHORTSHIFT].CreateBrushIndirect(&logBrush);
  logBrush.lbColor = m_PBCOLORS.crCharterReserve;
  m_Brushes[DO_CHARTERRESERVE].CreateBrushIndirect(&logBrush);
  logBrush.lbColor = m_PBCOLORS.crSightseeingReserve;
  m_Brushes[DO_SIGHTSEEINGRESERVE].CreateBrushIndirect(&logBrush);
//
//  Calculate the rostered time for each driver this week
//
  for(nI = 0; nI < MAX_ROSTEREDDRIVERS; nI++)
  {
    m_RosteredDrivers[nI].DRIVERSrecordID = NO_RECORD;
    m_RosteredDrivers[nI].time = 0;
  }

  long YMD, yyyy, mm, dd;
  long SERVICESrecordID;
  long flags;
  int  serviceIndex, runIndex;
  int  today;

  m_numRosteredDrivers = 0;
  rcode2 = btrieve(B_GETFIRST, TMS_DRIVERS, &DRIVERS, &DRIVERSKey0, 0);
  while(rcode2 == 0)
  {
    ROSTERKey2.DRIVERSrecordID = DRIVERS.recordID;
    rcode2 = btrieve(B_GETEQUAL, TMS_ROSTER, &ROSTER, &ROSTERKey2, 2);
    while(rcode2 == 0 &&
          ROSTER.DRIVERSrecordID == DRIVERS.recordID)
    {
      if(ROSTER.DIVISIONSrecordID == m_DailyOpsROSTERDivisionInEffect)
      {
        bFound = FALSE;
        for(nI = 0; nI < m_numRosteredDrivers; nI++)
        {
          if(m_RosteredDrivers[nI].DRIVERSrecordID == DRIVERS.recordID)
          {
            bFound = TRUE;
            break;
          }
        }
        if(!bFound)
        {
          nI = m_numRosteredDrivers;
          m_RosteredDrivers[nI].DRIVERSrecordID = DRIVERS.recordID;
          m_numRosteredDrivers++;
        }

        GetYMD(m_DailyOpsDateYYYYMMDD, &yyyy, &mm, &dd);
//
//  Back up to Monday if we're not there already
//    
        CTime spanDate(yyyy, mm, dd, 0, 0, 0);

        while(spanDate.GetDayOfWeek() != 1)
        {
          spanDate -= CTimeSpan(1, 0, 0, 0);
        }
//
//  Go through the assignments for each day
//
        CTime currentDate = spanDate;
        currentDate -= CTimeSpan(1, 0, 0, 0);
        for(nJ = 0; nJ < ROSTER_MAX_DAYS; nJ++)
        {
          currentDate += CTimeSpan(1, 0, 0, 0);
          YMD = currentDate.GetYear() * 10000 + currentDate.GetMonth() * 100 + currentDate.GetDay();
          SERVICESrecordID = DetermineServiceDay(YMD, FALSE, &today, tempString);
          if(ROSTER.WEEK[m_RosterWeek].RUNSrecordIDs[today] != NO_RECORD)
          {
            RUNSKey0.recordID = ROSTER.WEEK[m_RosterWeek].RUNSrecordIDs[today];
            flags = (ROSTER.WEEK[m_RosterWeek].flags & (1 << today)) ? RDATA_FLAG_CREWONLY : 0;

            rcode2 = LocateIndexes(SERVICESrecordID, ROSTER.WEEK[m_RosterWeek].RUNSrecordIDs[today], flags, &serviceIndex, &runIndex);
            if(rcode2 == 0)
            {
              m_RosteredDrivers[nI].time += m_pIncoreRuns[serviceIndex].pRuns[runIndex].payTime;
            }
          }
        }
      }
      rcode2 = btrieve(B_GETNEXT, TMS_ROSTER, &ROSTER, &ROSTERKey2, 2);
    }
    rcode2 = btrieve(B_GETNEXT, TMS_DRIVERS, &DRIVERS, &DRIVERSKey0, 0);
  }
//
//  Set up the CheckIn structure to fire alerts on no-shows
//
#ifdef DAILYOPSNOTIFY
  m_numInCheckIn = 0;
  if(m_bMonitorCheckins)
  {
    LoadString(hInst, TEXT_405, tempString, TEMPSTRING_LENGTH);
    StatusBarText(tempString);
    m_bCheckCheckIns = TRUE;

//
//  Read the run details into m_CheckIn
//
//  PopulateOpenWorkMonitor has established our service day
//
    PROPOSEDRUNDef PROPOSEDRUN;
    COSTDef COST;
    long    RUNSrecordID;
    long    CREWONLYrecordID;
    long    cutAsRuntype;
    long    DRIVERSrecordID;
    int     numPieces;

    RUNSKey1.DIVISIONSrecordID = m_DailyOpsRUNSDivisionInEffect;
    RUNSKey1.SERVICESrecordID = m_SERVICESrecordIDInEffect;
    RUNSKey1.runNumber = NO_RECORD;
    RUNSKey1.pieceNumber = NO_RECORD;
    rcode2 = btrieve(B_GETGREATER, TMS_RUNS, &RUNS, &RUNSKey1, 1);
    while(rcode2 == 0 && 
          RUNS.DIVISIONSrecordID == m_DailyOpsRUNSDivisionInEffect &&
          RUNS.SERVICESrecordID == m_SERVICESrecordIDInEffect)
    {
      if(RUNS.pieceNumber == 1)
      {
        RUNSrecordID = RUNS.recordID;
        cutAsRuntype = RUNS.cutAsRuntype;
        DRIVERSrecordID = NO_RECORD;
        ROSTERKey1.DIVISIONSrecordID = m_DailyOpsROSTERDivisionInEffect;
        ROSTERKey1.rosterNumber = NO_RECORD;
        rcode2 = btrieve(B_GETGREATER, TMS_ROSTER, &ROSTER, &ROSTERKey1, 1);
        while(rcode2 == 0 &&
              ROSTER.DIVISIONSrecordID == m_DailyOpsROSTERDivisionInEffect)
        {   
          if(ROSTER.WEEK[m_RosterWeek].RUNSrecordIDs[m_Today] == RUNSrecordID)
          {
            DRIVERSrecordID = ROSTER.DRIVERSrecordID;
            break;
          }
          rcode2 = btrieve(B_GETNEXT, TMS_ROSTER, &ROSTER, &ROSTERKey1, 1);
        }
        SetupRun(RUNSrecordID, FALSE, &PROPOSEDRUN);
        numPieces = PROPOSEDRUN.numPieces;
        RunCoster(&PROPOSEDRUN, cutAsRuntype, &COST);
        for(nI = 0; nI < numPieces; nI++)
        {
          m_CheckIn[m_numInCheckIn].runNumber = RUNS.runNumber;
          m_CheckIn[m_numInCheckIn].pieceNumber = nI + 1;
          m_CheckIn[m_numInCheckIn].reportAtTime = PROPOSEDRUN.piece[nI].fromTime - COST.PIECECOST[nI].reportTime;
          m_CheckIn[m_numInCheckIn].reportAtLocation = PROPOSEDRUN.piece[nI].fromNODESrecordID;
          m_CheckIn[m_numInCheckIn].DRIVERSrecordID = DRIVERSrecordID;
          m_CheckIn[m_numInCheckIn].flags = 0;
          m_numInCheckIn++;
        }
        RUNSKey0.recordID = RUNSrecordID;
        rcode2 = btrieve(B_GETEQUAL, TMS_RUNS, &RUNS, &RUNSKey0, 0);
        rcode2 = btrieve(B_GETPOSITION, TMS_RUNS, &RUNS, &RUNSKey0, 0);
        rcode2 = btrieve(B_GETDIRECT, TMS_RUNS, &RUNS, &RUNSKey1, 1);
      }
      rcode2 = btrieve(B_GETNEXT, TMS_RUNS, &RUNS, &RUNSKey1, 1);
    }
//
//  Read the crew-only runs into m_CheckIn
//
    CREWONLYKey1.DIVISIONSrecordID = m_DailyOpsRUNSDivisionInEffect;
    CREWONLYKey1.SERVICESrecordID = m_SERVICESrecordIDInEffect;
    CREWONLYKey1.runNumber = NO_RECORD;
    CREWONLYKey1.pieceNumber = NO_RECORD;
    rcode2 = btrieve(B_GETGREATER, TMS_CREWONLY, &CREWONLY, &CREWONLYKey1, 1);
    while(rcode2 == 0 && 
          CREWONLY.DIVISIONSrecordID == m_DailyOpsRUNSDivisionInEffect &&
          CREWONLY.SERVICESrecordID == m_SERVICESrecordIDInEffect)
    {
      if(CREWONLY.pieceNumber == 1)
      {
        CREWONLYrecordID = CREWONLY.recordID;
        cutAsRuntype = CREWONLY.cutAsRuntype;
        DRIVERSrecordID = NO_RECORD;
        ROSTERKey1.DIVISIONSrecordID = m_DailyOpsROSTERDivisionInEffect;
        ROSTERKey1.rosterNumber = NO_RECORD;
        rcode2 = btrieve(B_GETGREATER, TMS_ROSTER, &ROSTER, &ROSTERKey1, 1);
        while(rcode2 == 0 &&
              ROSTER.DIVISIONSrecordID == m_DailyOpsROSTERDivisionInEffect)
        {   
          if(ROSTER.WEEK[m_RosterWeek].RUNSrecordIDs[m_Today] == CREWONLYrecordID)
          {
            DRIVERSrecordID = ROSTER.DRIVERSrecordID;
            break;
          }
          rcode2 = btrieve(B_GETNEXT, TMS_ROSTER, &ROSTER, &ROSTERKey1, 1);
        }
        SetupRun(CREWONLYrecordID, TRUE, &PROPOSEDRUN);
        numPieces = PROPOSEDRUN.numPieces;
        RunCoster(&PROPOSEDRUN, cutAsRuntype, &COST);
        for(nI = 0; nI < numPieces; nI++)
        {
          m_CheckIn[m_numInCheckIn].runNumber = CREWONLY.runNumber;
          m_CheckIn[m_numInCheckIn].pieceNumber = nI + 1;
          m_CheckIn[m_numInCheckIn].reportAtTime = PROPOSEDRUN.piece[nI].fromTime - COST.PIECECOST[nI].reportTime;
          m_CheckIn[m_numInCheckIn].reportAtLocation = PROPOSEDRUN.piece[nI].fromNODESrecordID;
          m_CheckIn[m_numInCheckIn].DRIVERSrecordID = DRIVERSrecordID;
          m_CheckIn[m_numInCheckIn].flags = 0;
          m_numInCheckIn++;
        }
        CREWONLYKey0.recordID = CREWONLYrecordID;
        rcode2 = btrieve(B_GETEQUAL, TMS_CREWONLY, &CREWONLY, &CREWONLYKey0, 0);
        rcode2 = btrieve(B_GETPOSITION, TMS_CREWONLY, &CREWONLY, &CREWONLYKey0, 0);
        rcode2 = btrieve(B_GETDIRECT, TMS_CREWONLY, &CREWONLY, &CREWONLYKey1, 1);
      }
      rcode2 = btrieve(B_GETNEXT, TMS_CREWONLY, &CREWONLY, &CREWONLYKey1, 1);
    }
//
//  Adjust m_CheckIn for what's in the Audit Trail
//
//  Check-in today
//
    long absRecID;
    BOOL bFound;
    BOOL bAdjust;

    for(nI = 0; nI < m_numInCheckIn; nI++)
    {
      DAILYOPSKey3.DRIVERSrecordID = m_CheckIn[nI].DRIVERSrecordID;
      DAILYOPSKey3.recordTypeFlag = DAILYOPS_FLAG_OPERATOR;
      DAILYOPSKey3.pertainsToDate = m_DailyOpsDateYYYYMMDD;
      DAILYOPSKey3.pertainsToTime = NO_RECORD;
      DAILYOPSKey3.recordFlags = 0;
      bFound = FALSE;
      bAdjust = FALSE;
      rcode2 = btrieve(B_GETGREATER, TMS_DAILYOPS, &DAILYOPS, &DAILYOPSKey3, 3);
      while(rcode2 == 0 &&
            (DAILYOPS.DRIVERSrecordID == m_CheckIn[nI].DRIVERSrecordID) &&
            (DAILYOPS.recordTypeFlag & DAILYOPS_FLAG_OPERATOR) &&
            (DAILYOPS.pertainsToDate == m_DailyOpsDateYYYYMMDD))
      {
        if(DAILYOPS.recordFlags & DAILYOPS_FLAG_OPERATORCHECKIN)
        {
          if(!ANegatedRecord(DAILYOPS.recordID, 3))
          {
            bAdjust = TRUE;
            break;
          }
        }
        rcode2 = btrieve(B_GETNEXT, TMS_DAILYOPS, &DAILYOPS, &DAILYOPSKey3, 3);
      }
      if(bAdjust)
      {
        m_CheckIn[nI].flags |= CHECKIN_FLAG_CHECKEDIN;
      }
    }
//
//  Sort the pieces
//
    qsort((void *)m_CheckIn, m_numInCheckIn, sizeof(CHECKINDef), sort_CheckIn);

//
//  Bounce into m_CheckIn so we're not checking on check-ins prior to now
//
    long now = m_SystemTime.GetHour() * 3600 + m_SystemTime.GetMinute() * 60;

    m_CurrentCheckIn = m_numInCheckIn;
    for(nI = 0; nI < m_numInCheckIn; nI++)
    {
      if(m_CheckIn[nI].reportAtTime - TIMEOF1STNOTIFICATION >= now)
      {
        m_CurrentCheckIn = nI;
        break;
      }
      m_CheckIn[nI].flags |= CHECKIN_FLAG_IGNOREENTRY;
    }
  }
#endif
//
//  Set up the Chronology
//
  DetermineServiceDay(m_DailyOpsDateYYYYMMDD, FALSE, &today, tempString);
  m_Today = today;

  StatusBarText("Setting up the chronology...");
  TMSRPTPassedDataDef PassedData;

  PassedData.flags = TMSRPT_PASSEDDATAFLAG_FROMDAILYOPS;
  PassedData.dayOfWeek = m_Today;
  PassedData.DIVISIONSrecordID = m_DailyOpsROSTERDivisionInEffect;
  PassedData.date = m_DailyOpsDateYYYYMMDD;

  TMSRPT50(&PassedData);

  CHRONDef *pCHRON;
  FILE     *fp;
  int      numLines;
  long     time = (m_SystemTime.GetHour() * 3600) + (m_SystemTime.GetMinute() * 60) + m_SystemTime.GetSecond();

  if(PassedData.numDataFiles > 0)
  {
    pCHRON = (CHRONDef *)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(CHRONDef) * PassedData.numDataFiles); 
    if(pCHRON == NULL)
    {
      AllocationError(__FILE__, __LINE__, FALSE);
      Cleanup();
      return TRUE;
    }

    strcpy(tempString, szReportsTempFolder);
    strcat(tempString, "\\TMSRPT50D.txt");

    fp = fopen(tempString, "r");
    numLines = 0;
    if(fp != NULL)
    {
      while(fgets(szarString, sizeof(szarString), fp))
      {
        strcpy(tempString, strtok(szarString, "\t"));
        pCHRON[numLines].time = atol(tempString);
        strcpy(pCHRON[numLines].text, strtok(NULL, "\n"));
        numLines++;
      }
    }

    qsort((void *)pCHRON, numLines, sizeof(CHRONDef), sort_Chronology);

    for(nI = 0; nI < numLines; nI++)
    {
      nJ = pListBoxCHRONOLOGY->AddString(pCHRON[nI].text);
      pListBoxCHRONOLOGY->SetItemData(nJ, pCHRON[nI].time);
    }
    
    for(m_CurrentChronologyLine = 0, nI = 0; nI < numLines; nI++)
    {
      if(time < pCHRON[nI].time)
      {
        break;
      }
      else
      {
        m_CurrentChronologyLine = nI;
      }
    }
    
    TMSHeapFree(pCHRON);
  }
  

  StatusBarEnd();
//
//  Show what's in effect
//
//  Roster division
//
  DIVISIONSKey0.recordID = m_DailyOpsROSTERDivisionInEffect;
  rcode2 = btrieve(B_GETEQUAL, TMS_DIVISIONS, &DIVISIONS, &DIVISIONSKey0, 0);
  if(rcode2 != 0)
  {
    strcpy(szarString, "?");
  }
  else
  {
    strncpy(szarString, DIVISIONS.name, DIVISIONS_NAME_LENGTH);
    trim(szarString, DIVISIONS_NAME_LENGTH);
  }
  sprintf(tempString, "%s\tRostering division in effect: %s\r\n",
        m_SystemTime.Format(_T("%I:%M:%S%p")), szarString);
  pEditMESSAGES->SetSel(-1, -1);
  pEditMESSAGES->ReplaceSel(tempString);
//
//  Roster week
//
  sprintf(tempString, "%s\tRoster week in effect: %ld\r\n",
        m_SystemTime.Format(_T("%I:%M:%S%p")), m_RosterWeek + 1);
  pEditMESSAGES->SetSel(-1, -1);
  pEditMESSAGES->ReplaceSel(tempString);
//
//  Runs division
//
  DIVISIONSKey0.recordID = m_DailyOpsRUNSDivisionInEffect;
  rcode2 = btrieve(B_GETEQUAL, TMS_DIVISIONS, &DIVISIONS, &DIVISIONSKey0, 0);
  if(rcode2 != 0)
  {
    strcpy(szarString, "?");
  }
  else
  {
    strncpy(szarString, DIVISIONS.name, DIVISIONS_NAME_LENGTH);
    trim(szarString, DIVISIONS_NAME_LENGTH);
  }
  sprintf(tempString, "%s\tRuns division in effect: %s\r\n",
        m_SystemTime.Format(_T("%I:%M:%S%p")), szarString);
  pEditMESSAGES->SetSel(-1, -1);
  pEditMESSAGES->ReplaceSel(tempString);
//
//  Today is...
//
  sprintf(tempString, "%s\tToday is %s\r\n",
        m_SystemTime.Format(_T("%I:%M:%S%p")), m_szHolidayName);
  pEditMESSAGES->SetSel(-1, -1);
  pEditMESSAGES->ReplaceSel(tempString);
//
//  Service in effect
//
  SERVICESKey0.recordID = m_SERVICESrecordIDInEffect;
  rcode2 = btrieve(B_GETEQUAL, TMS_SERVICES, &SERVICES, &SERVICESKey0, 0);
  if(rcode2 != 0)
  {
    strcpy(szarString, "?");
  }
  else
  {
    strncpy(szarString, SERVICES.name, SERVICES_NAME_LENGTH);
    trim(szarString, SERVICES_NAME_LENGTH);
  }
  sprintf(tempString, "%s\tService in effect: %s\r\n",
        m_SystemTime.Format(_T("%I:%M:%S%p")), szarString);
  pEditMESSAGES->SetSel(-1, -1);
  pEditMESSAGES->ReplaceSel(tempString);
//
//  Set the last record
//
  rcode2 = btrieve(B_GETLAST, TMS_DAILYOPS, &DAILYOPS, &DAILYOPSKey0, 0);
  m_LastDAILYOPSRecordID = DAILYOPS.recordID;
//
//  Set the rectangle of the viewing area, its viewing
//  extents, and the latest and earliest block times 
//
  EstablishViewingArea();
//
//  Populate the block lists
//
  PopulateVehicleAssignment();
  m_SelectedBlockRow = NO_RECORD;
//
//  Default to "Extra time" in the "Extras" group box
//
  pButtonOVERTIME->SetCheck(TRUE);
//
//  Default to including hours when assigning operators to open work
//
  m_bIncludeHours = TRUE;
//
//  Show a summary of absences registered for today 
//
  MessageBeep(MB_ICONINFORMATION);
  CDailyOpsAB dlg(this, m_DailyOpsDateYYYYMMDD, TRUE);
  dlg.DoModal();
//
//  Set the mouse free
//
  ReleaseCapture();

  m_bInitInProgress = FALSE; 

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CDailyOps::OnClose() 
{
  Cleanup();

	CDialog::OnClose();
}

void CDailyOps::OnHelp() 
{
  m_PreviousTime = m_SystemTime;
}

void CDailyOps::OnRclickOpenworklist(NMHDR* pNMHDR, LRESULT* pResult) 
{
  m_PreviousTime = m_SystemTime;

  ProcessSelectedItem(pListCtrlOPENWORKLIST);

  m_menu.LoadMenu(IDR_DAILYOPSOWM);
  CMenu* pMenu = m_menu.GetSubMenu(0);
//
//  Get the current mouse location and convert it to client coordinates.
//
  DWORD pos = GetMessagePos();
  CPoint pt(LOWORD(pos), HIWORD(pos));

  if(pMenu)
  {
    if(m_bIncludeHours)
    {
      pMenu->CheckMenuItem(4, MF_BYPOSITION | MF_CHECKED);
    }
    else
    {
      pMenu->CheckMenuItem(4, MF_BYPOSITION | MF_UNCHECKED);
    }
    pMenu->TrackPopupMenu(TPM_LEFTALIGN, pt.x, pt.y, FromHandle(m_hWnd), NULL);
  }
  m_menu.DestroyMenu();

	*pResult = 0;
}


int CDailyOps::ProcessSelectedItem(CListCtrl* pLC)
{
  m_PreviousTime = m_SystemTime;

	int nI = pLC->GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);

  if(nI < 0)
  {
    return(nI);
  }

  LVITEM  LVI;
  CString s;
 
  LVI.mask = LVIF_PARAM;
  LVI.iItem = nI;
  LVI.iSubItem = 0;
  pLC->GetItem(&LVI);
//
//  "To Be Assigned" bus list
//
  if(pLC == pListCtrlBLOCKLIST)
  {
    m_SelectedBlockRow = nI;
    m_SelectedBlockIndex = LVI.lParam;
  }
//
//  "Open Work Monitor" list
//
  else if(pLC == pListCtrlOPENWORKLIST)
  {
    m_SelectedOWRow = nI;
  }

  return(0);
}

void CDailyOps::OnClickBlocklist(NMHDR* pNMHDR, LRESULT* pResult) 
{
  m_PreviousTime = m_SystemTime;

  ProcessSelectedItem(pListCtrlBLOCKLIST);
	
	*pResult = 0;
}


void CDailyOps::OnClickOpenworklist(NMHDR* pNMHDR, LRESULT* pResult) 
{
  m_PreviousTime = m_SystemTime;

  ProcessSelectedItem(pListCtrlOPENWORKLIST);
	
	*pResult = 0;
}


long CDailyOps::GetBusAssignedRecord(int index, BOOL bAddOne)
{
  m_PreviousTime = m_SystemTime;

  DAILYOPSDef     DO;
  TMSKey0Def      DOK0;
  DAILYOPSKey1Def DOK1;
  CString s;
  int     rcode2;

  if(index == NO_RECORD)
  {
    if(m_bMessageDisplay)
    {
      s.LoadString(ERROR_316);
      MessageBeep(MB_ICONSTOP);
      MessageBox(s, TMS, MB_ICONSTOP | MB_OK);
    }
    return(NO_RECORD);
  }
//
//  Get where it was assigned
//
//  Database verify: 11-Jan-07
//
  long DAILYOPSrecordID = NO_RECORD;

  DOK1.recordTypeFlag = DAILYOPS_FLAG_BUS;
  DOK1.pertainsToDate = m_DailyOpsDateYYYYMMDD;
  DOK1.pertainsToTime = m_LastDAILYOPSRecordID + 1;
  DOK1.recordFlags = 0;
  rcode2 = btrieve(B_GETLESSTHANOREQUAL, TMS_DAILYOPS, &DO, &DOK1, 1);
  while(rcode2 == 0 &&
        (DO.recordTypeFlag & DAILYOPS_FLAG_BUS) &&
        (DO.pertainsToDate == m_DailyOpsDateYYYYMMDD))
  {
    if(DO.DOPS.Bus.BUSESrecordID == m_BlockInfo[index].BUSESrecordID)
    {
      DAILYOPSrecordID = DO.recordID;
      break;
    }
    rcode2 = btrieve(B_GETPREVIOUS, TMS_DAILYOPS, &DO, &DOK1, 1);
  }
//
//  Couldn't find it - add an assignment record to show we don't have one (this should never happen)
//
//  Database verify: 11-Jan-07
//
  if(DAILYOPSrecordID == NO_RECORD && bAddOne)
  {
    if(m_bMessageDisplay)
    {
      s.LoadString(ERROR_315);
      sprintf(tempString, s, m_BlockInfo[index].BUSESrecordID);
      MessageBeep(MB_ICONINFORMATION);
      MessageBox(tempString, TMS, MB_OK);
    }
    DailyOpsBuildRecord(&DO, DAILYOPS_FLAG_BUS);
    DO.recordFlags = DAILYOPS_FLAG_BUSASSIGNMENT | DAILYOPS_FLAG_BUSMISSEDASSIGNMENT;
    DO.pertainsToDate = m_DailyOpsDateYYYYMMDD;
//    DO.pertainsToTime = m_DailyOpsDate.GetHour() * 3600 + m_DailyOpsDate.GetMinute() * 60 + m_DailyOpsDate.GetSecond();
    DO.pertainsToTime = DO.recordID;
    DO.DOPS.Bus.BUSESrecordID = m_BlockInfo[index].BUSESrecordID;
    rcode2 = btrieve(B_INSERT, TMS_DAILYOPS, &DO, &DOK0, 0);
    if(rcode2 == 0)
    {
      DAILYOPSrecordID = DO.recordID;
      m_LastDAILYOPSRecordID = DO.recordID;
    }
  }
//
//  All done
//
  return(DAILYOPSrecordID);
}

void CDailyOps::OnCheckincheckoutOperator() 
{
  m_PreviousTime = m_SystemTime;

  CString s;
  int rcode2;
  int nI = pComboBoxOPERATOR->GetCurSel();

  if(nI == CB_ERR)
  {
    return;
  }

  long DRIVERSrecordID = pComboBoxOPERATOR->GetItemData(nI);
//
//  Extra time
//
//  Database verify: 11-Jan-07
//
  if(pButtonOVERTIME->GetCheck())
  {
    long extraTime;

    pEditOVERTIMETIME->GetWindowText(tempString, TEMPSTRING_LENGTH);
    extraTime = (strcmp(tempString, "") == 0 ? 0 : thhmm(tempString));
    if(extraTime != 0)
    {
      long reasonIndex;
//
//  Get the reason
//
      nI = pComboBoxEXTRAREASON->GetCurSel();
      if(nI == CB_ERR)
      {
        nI = 0;
      }
      reasonIndex = pComboBoxEXTRAREASON->GetItemData(nI);

      DailyOpsBuildRecord(&DAILYOPS, DAILYOPS_FLAG_OPERATOR);
      DAILYOPS.pertainsToDate = m_DailyOpsDateYYYYMMDD;
      DAILYOPS.pertainsToTime = m_DailyOpsDate.GetHour() * 3600 + m_DailyOpsDate.GetMinute() * 60 + m_DailyOpsDate.GetSecond();
      DAILYOPS.recordFlags = DAILYOPS_FLAG_OVERTIME;
      DAILYOPS.DRIVERSrecordID = DRIVERSrecordID;
      DAILYOPS.DOPS.Operator.timeAdjustment = extraTime;
      DAILYOPS.DOPS.Operator.extraTimeReasonIndex = (char)reasonIndex;
      rcode2 = btrieve(B_INSERT, TMS_DAILYOPS, &DAILYOPS, &DAILYOPSKey0, 0);
      if(rcode2 == 0)
      {
        m_LastDAILYOPSRecordID = DAILYOPS.recordID;
      }
      s.LoadString(TEXT_314);
      sprintf(tempString, s, m_SystemTime.Format(_T("%I:%M:%S%p")),
            BuildOperatorString(DRIVERSrecordID), chhmm(DAILYOPS.DOPS.Operator.timeAdjustment));
      pEditMESSAGES->SetSel(-1, -1);
      pEditMESSAGES->ReplaceSel(tempString);
    }
  }
//
//  Extra Trip
//
//  Database verify: 11-Jan-07
//
  else if(pButtonEXTRATRIP->GetCheck())
  {
    DailyOpsBuildRecord(&DAILYOPS, DAILYOPS_FLAG_OPERATOR);
    CAddExtraTrip dlg(this, &DAILYOPS);  // This builds most of the record

    if(dlg.DoModal() == IDOK)
    {
      DAILYOPS.pertainsToDate = m_DailyOpsDateYYYYMMDD;
      DAILYOPS.pertainsToTime = m_DailyOpsDate.GetHour() * 3600 + m_DailyOpsDate.GetMinute() * 60 + m_DailyOpsDate.GetSecond();
      DAILYOPS.recordFlags = DAILYOPS_FLAG_EXTRATRIP;
      DAILYOPS.DRIVERSrecordID = DRIVERSrecordID;
      rcode2 = btrieve(B_INSERT, TMS_DAILYOPS, &DAILYOPS, &DAILYOPSKey0, 0);
      if(rcode2 == 0)
      {
        m_LastDAILYOPSRecordID = DAILYOPS.recordID;
      }
      s.LoadString(TEXT_315);
      sprintf(tempString, s, m_SystemTime.Format(_T("%I:%M:%S%p")),
            BuildOperatorString(DRIVERSrecordID), chhmm(DAILYOPS.DOPS.Operator.timeAdjustment));
      pEditMESSAGES->SetSel(-1, -1);
      pEditMESSAGES->ReplaceSel(tempString);
    }
  }
}

void CDailyOps::OnCorrectoperator() 
{
  m_PreviousTime = m_SystemTime;

  int nI = pComboBoxOPERATOR->GetCurSel();

  if(nI == CB_ERR)
  {
    return;
  }

  long DRIVERSrecordID;
  BOOL bChanged;

  DRIVERSrecordID = pComboBoxOPERATOR->GetItemData(nI);

  CDailyOpsSS dlg(this, DRIVERSrecordID, DAILYOPS_FLAG_OPERATOR, &bChanged);

  dlg.DoModal();

  if(bChanged)
  {
    m_numAbsent = RefreshAbsenceList(m_DailyOpsDateYYYYMMDD, m_AbsentList);
    PopulateOpenWorkMonitor(NO_RECORD, FALSE, TRUE);
    PopulateVehicleAssignment();
  }
}

void CDailyOps::RefreshBlockList(int index)
{
  LVITEM LVI;
  BOOL   bFound;
  int    nI;
  int    rcode2;
  int    counter;
  int    nIStart, nIEnd;
  int    reposition;

//
//  Clear it first, on index of NO_RECORD
//
  if(index == NO_RECORD)
  {
    reposition = pListCtrlBLOCKLIST->GetTopIndex();
    if(reposition < 0)
    {
      reposition = 0;
    }
    pListCtrlBLOCKLIST->DeleteAllItems();
  }
  else
  {
    reposition = index;
  }
//
//  Cycle through
//
  if(index == NO_RECORD)
  {
    counter = 0;
    nIStart = 0;
    nIEnd = m_numBlocksInBlockInfo;
  }
//
//  Loop through all displayed items getting param and match
//  it to index - the display row is the loop variant
//
  else
  {
    for(bFound = FALSE, nI = 0; nI < m_numBlocksInBlockInfo; nI++)
    {
      LVI.mask = LVIF_PARAM;
      LVI.iItem = nI;
      LVI.iSubItem = 0;
      pListCtrlBLOCKLIST->GetItem(&LVI);
      if(index == LVI.lParam)
      {
        pListCtrlBLOCKLIST->DeleteItem(nI);
        counter = nI;
        nIStart = index;
        nIEnd   = index + 1;
        bFound = TRUE;
        break;
      }
    }
    if(!bFound)
    {
      return;
    }
  }
  for(nI = nIStart; nI < nIEnd; nI++)
  {
    if(!(m_BlockInfoShowFlags & BLOCKINFO_FLAG_DISPLAYALL))
    {
      if(index == NO_RECORD && m_BlockInfo[nI].flags != m_BlockInfoShowFlags)
      {
        continue;
      }
    }
    {
//
//  Status
//
      if(m_BlockInfo[nI].flags & BLOCKINFO_FLAG_ASSIGNED)
      {
        strcpy(tempString, "A");
      }
      else if(m_BlockInfo[nI].flags & BLOCKINFO_FLAG_BUSRETURN)
      {
        strcpy(tempString, "R");
      }
      else if(m_BlockInfo[nI].flags & BLOCKINFO_FLAG_BLOCKDROP)
      {
        strcpy(tempString, "D");
      }
      else
      {
        strcpy(tempString, "");
      }
      LVI.mask = LVIF_TEXT | LVIF_PARAM;
      LVI.lParam = nI;
      LVI.iItem = counter;
      LVI.iSubItem = 0;
      LVI.pszText = tempString;
      LVI.iItem = pListCtrlBLOCKLIST->InsertItem(&LVI);
//
//  Vehicle
//
      BUSESKey0.recordID = m_BlockInfo[nI].BUSESrecordID;
      rcode2 = btrieve(B_GETEQUAL, TMS_BUSES, &BUSES, &BUSESKey0, 0);
      if(rcode2 != 0)
      {
        strcpy(tempString, "");
      }
      else
      {
        strncpy(tempString, BUSES.number, BUSES_NUMBER_LENGTH);
        trim(tempString, BUSES_NUMBER_LENGTH);
      }
      LVI.mask = LVIF_TEXT;
      LVI.iItem = counter;
      LVI.iSubItem = 1;
      LVI.pszText = tempString;
      LVI.iItem = pListCtrlBLOCKLIST->SetItem(&LVI);
//
//  Block number
//
      LVI.mask = LVIF_TEXT;
      LVI.iItem = counter;
      LVI.iSubItem = 2;
      sprintf(tempString, "%ld", m_BlockInfo[nI].blockNumber);
      LVI.pszText = tempString;
      LVI.iItem = pListCtrlBLOCKLIST->SetItem(&LVI);
//
//  Run number
//
      LVI.mask = LVIF_TEXT;
      LVI.iItem = counter;
      LVI.iSubItem = 3;
      if(m_BlockInfo[nI].startingRunNumber > 0)
      {
        sprintf(tempString, "%ld", m_BlockInfo[nI].startingRunNumber);
      }
      else
      {
        strcpy(tempString, "- - -");
      }
      LVI.pszText = tempString;
      LVI.iItem = pListCtrlBLOCKLIST->SetItem(&LVI);
//
//  Roster number
//
      LVI.mask = LVIF_TEXT;
      LVI.iItem = counter;
      LVI.iSubItem = 4;
      if(m_BlockInfo[nI].startingRosterNumber > 0)
      {
        sprintf(tempString, "%ld", m_BlockInfo[nI].startingRosterNumber);
      }
      else
      {
        strcpy(tempString, "- - -");
      }
      LVI.pszText = tempString;
      LVI.iItem = pListCtrlBLOCKLIST->SetItem(&LVI);
//
//  Operator 
//
      LVI.mask = LVIF_TEXT;
      LVI.iItem = counter;
      LVI.iSubItem = 5;
      if(m_BlockInfo[nI].startingDRIVERSrecordID > 0)
      {
        sprintf(tempString, "%s", BuildOperatorString(m_BlockInfo[nI].startingDRIVERSrecordID));
      }
      else
      {
//        if(m_BlockInfo[nI].startingRosterNumber > 0)
//        {
//          strcpy(tempString, "No operator assigned");
//        }
//        else
//        {
//          strcpy(tempString, "- - -");
//        }
        strcpy(tempString, "Open");
      }
      LVI.pszText = tempString;
      LVI.iItem = pListCtrlBLOCKLIST->SetItem(&LVI);
//
//  POTime
//
      LVI.mask = LVIF_TEXT;
      LVI.iItem = counter;
      LVI.iSubItem = 6;
      strcpy(tempString, Tchar(m_BlockInfo[nI].POTime));
      LVI.pszText = tempString;
      LVI.iItem = pListCtrlBLOCKLIST->SetItem(&LVI);
//
//  POG Node
//
      NODESKey0.recordID = m_BlockInfo[nI].POGNODESrecordID;
      rcode2 = btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
      if(rcode2 != 0)
      {
        strcpy(tempString, "?");
      }
      else
      {
        strncpy(tempString, NODES.abbrName, NODES_ABBRNAME_LENGTH);
        trim(tempString, NODES_ABBRNAME_LENGTH);
      }
      LVI.mask = LVIF_TEXT;
      LVI.iItem = counter;
      LVI.iSubItem = 7;
      LVI.pszText = tempString;
      LVI.iItem = pListCtrlBLOCKLIST->SetItem(&LVI);
//
//  PIG Node
//
      NODESKey0.recordID = m_BlockInfo[nI].PIGNODESrecordID;
      rcode2 = btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
      if(rcode2 != 0)
      {
        strcpy(tempString, "?");
      }
      else
      {
        strncpy(tempString, NODES.abbrName, NODES_ABBRNAME_LENGTH);
        trim(tempString, NODES_ABBRNAME_LENGTH);
      }
      LVI.mask = LVIF_TEXT;
      LVI.iItem = counter;
      LVI.iSubItem = 8;
      LVI.pszText = tempString;
      LVI.iItem = pListCtrlBLOCKLIST->SetItem(&LVI);
//
//  PITime
//
      LVI.mask = LVIF_TEXT;
      LVI.iItem = counter;
      LVI.iSubItem = 9;
      strcpy(tempString, Tchar(m_BlockInfo[nI].PITime));
      LVI.pszText = tempString;
      LVI.iItem = pListCtrlBLOCKLIST->SetItem(&LVI);
//
//  Distance
//
      LVI.mask = LVIF_TEXT;
      LVI.iItem = counter;
      LVI.iSubItem = 10;
      sprintf(tempString, "%6.2f", m_BlockInfo[nI].distance);
      LVI.pszText = tempString;
      LVI.iItem = pListCtrlBLOCKLIST->SetItem(&LVI);
//
//  Cycle through
//
      counter++;
    }
  }
//
//  Reposition the list
//
  UINT flag = LVIS_SELECTED | LVIS_FOCUSED;
   
  pListCtrlBLOCKLIST->SetItemState(reposition, flag, flag);
  pListCtrlBLOCKLIST->EnsureVisible(reposition, FALSE);
}

void CDailyOps::PopulateOpenWorkMonitor(long date, BOOL bRefresh, BOOL bDisplay)
{
  CString s;
  long numRuns;
  BOOL bFound;
  char szHolidayName[DAILYOPS_DATENAME_LENGTH + 1];
  int  days[ROSTER_MAX_DAYS] = {TEXT_009, TEXT_010, TEXT_011, TEXT_012, TEXT_013, TEXT_014, TEXT_015};
  int  nI;
  int  nJ;
  int  nK;
  int  nL;
  int  rcode2;
  int  today;
  int  serviceIndex;
  int  runIndex;
  long flags;
  long dateToUse = (date == NO_RECORD ? m_DailyOpsDateYYYYMMDD : date);

  if(!pListCtrlOPENWORKLIST->IsWindowEnabled())
  {
    return;
  }

  m_SERVICESrecordIDInEffect = DetermineServiceDay(dateToUse, bDisplay, &today, szHolidayName);
  m_Today = today;
  strcpy(m_szHolidayName, szHolidayName);
//
//  Read the runs into m_pRData
//
  if(!bRefresh)
  {
    memcpy(m_pRData, m_pRDataHold, sizeof(RDataDef) * m_numRunsInRDataHold);
    numRuns = m_numRunsInRDataHold;
  }
  else
  {
    RUNSKey1.DIVISIONSrecordID = m_DailyOpsRUNSDivisionInEffect;
    RUNSKey1.SERVICESrecordID = m_SERVICESrecordIDInEffect;
    RUNSKey1.runNumber = NO_RECORD;
    RUNSKey1.pieceNumber = NO_RECORD;
    rcode2 = btrieve(B_GETGREATER, TMS_RUNS, &RUNS, &RUNSKey1, 1);
    numRuns = 0;
    flags = 0;
    while(rcode2 == 0 && 
          RUNS.DIVISIONSrecordID == m_DailyOpsRUNSDivisionInEffect &&
          RUNS.SERVICESrecordID == m_SERVICESrecordIDInEffect)
    {
      if(RUNS.pieceNumber == 1)
      {
        rcode2 = LocateIndexes(m_SERVICESrecordIDInEffect, RUNS.recordID, flags, &serviceIndex, &runIndex);
        if(rcode2 == 0)
        {
          m_pRData[numRuns].recordID = m_pIncoreRuns[serviceIndex].pRuns[runIndex].recordID;
          m_pRData[numRuns].runNumber = m_pIncoreRuns[serviceIndex].pRuns[runIndex].runNumber;
          m_pRData[numRuns].cutAsRuntype = m_pIncoreRuns[serviceIndex].pRuns[runIndex].cutAsRuntype;
          m_pRData[numRuns].onNODESrecordID = m_pIncoreRuns[serviceIndex].pRuns[runIndex].startNODESrecordID;
          m_pRData[numRuns].onTime = m_pIncoreRuns[serviceIndex].pRuns[runIndex].startTime;
          m_pRData[numRuns].offNODESrecordID = m_pIncoreRuns[serviceIndex].pRuns[runIndex].endNODESrecordID;
          m_pRData[numRuns].offTime = m_pIncoreRuns[serviceIndex].pRuns[runIndex].endTime;
          m_pRData[numRuns].DRIVERSrecordID = NO_RECORD;
          m_pRData[numRuns].pay = m_pIncoreRuns[serviceIndex].pRuns[runIndex].payTime;
          m_pRData[numRuns].flags = 0;
          RUNSKey0.recordID = m_pRData[numRuns].recordID;
          rcode2 = btrieve(B_GETEQUAL, TMS_RUNS, &RUNS, &RUNSKey0, 0);
          rcode2 = btrieve(B_GETPOSITION, TMS_RUNS, &RUNS, &RUNSKey0, 0);
          rcode2 = btrieve(B_GETDIRECT, TMS_RUNS, &RUNS, &RUNSKey1, 1);
          numRuns++;
        }
      }
      rcode2 = btrieve(B_GETNEXT, TMS_RUNS, &RUNS, &RUNSKey1, 1);
    }
//
//  Read the crew-only runs into m_pRData
//
    CREWONLYKey1.DIVISIONSrecordID = m_DailyOpsRUNSDivisionInEffect;
    CREWONLYKey1.SERVICESrecordID = m_SERVICESrecordIDInEffect;
    CREWONLYKey1.runNumber = NO_RECORD;
    CREWONLYKey1.pieceNumber = NO_RECORD;
    rcode2 = btrieve(B_GETGREATER, TMS_CREWONLY, &CREWONLY, &CREWONLYKey1, 1);
    while(rcode2 == 0 && 
          CREWONLY.DIVISIONSrecordID == m_DailyOpsRUNSDivisionInEffect &&
          CREWONLY.SERVICESrecordID == m_SERVICESrecordIDInEffect)
    {
      if(CREWONLY.pieceNumber == 1)
      {
        rcode2 = LocateIndexes(m_SERVICESrecordIDInEffect, CREWONLY.recordID, RDATA_FLAG_CREWONLY, &serviceIndex, &runIndex);
        if(rcode2 == 0)
        {
          m_pRData[numRuns].recordID = m_pIncoreRuns[serviceIndex].pRuns[runIndex].recordID;
          m_pRData[numRuns].runNumber = m_pIncoreRuns[serviceIndex].pRuns[runIndex].runNumber;
          m_pRData[numRuns].cutAsRuntype = m_pIncoreRuns[serviceIndex].pRuns[runIndex].cutAsRuntype;
          m_pRData[numRuns].onNODESrecordID = m_pIncoreRuns[serviceIndex].pRuns[runIndex].startNODESrecordID;
          m_pRData[numRuns].onTime = m_pIncoreRuns[serviceIndex].pRuns[runIndex].startTime;
          m_pRData[numRuns].offNODESrecordID = m_pIncoreRuns[serviceIndex].pRuns[runIndex].endNODESrecordID;
          m_pRData[numRuns].offTime = m_pIncoreRuns[serviceIndex].pRuns[runIndex].endTime;
          m_pRData[numRuns].DRIVERSrecordID = NO_RECORD;
          m_pRData[numRuns].pay = m_pIncoreRuns[serviceIndex].pRuns[runIndex].payTime;
          m_pRData[numRuns].flags = RDATA_FLAG_CREWONLY;
          CREWONLYKey0.recordID = m_pRData[numRuns].recordID;
          rcode2 = btrieve(B_GETEQUAL, TMS_CREWONLY, &CREWONLY, &CREWONLYKey0, 0);
          rcode2 = btrieve(B_GETPOSITION, TMS_CREWONLY, &CREWONLY, &CREWONLYKey0, 0);
          rcode2 = btrieve(B_GETDIRECT, TMS_CREWONLY, &CREWONLY, &CREWONLYKey1, 1);
          numRuns++;
        }
      }
      rcode2 = btrieve(B_GETNEXT, TMS_CREWONLY, &CREWONLY, &CREWONLYKey1, 1);
    }
//
//  Look at the roster to see who's got what.  Assign
//  the rostered driver into the m_pRDATA structure.
//
    BOOL bCase1;

    ROSTERKey1.DIVISIONSrecordID = m_DailyOpsROSTERDivisionInEffect;
    ROSTERKey1.rosterNumber = NO_RECORD;
    rcode2 = btrieve(B_GETGREATER, TMS_ROSTER, &ROSTER, &ROSTERKey1, 1);
    while(rcode2 == 0 &&
          ROSTER.DIVISIONSrecordID == m_DailyOpsROSTERDivisionInEffect)
    {
      if(ROSTER.WEEK[m_RosterWeek].RUNSrecordIDs[m_Today] != NO_RECORD)
      {
        for(nI = 0; nI < numRuns; nI++)
        {
          if(ROSTER.WEEK[m_RosterWeek].RUNSrecordIDs[m_Today] == m_pRData[nI].recordID)
          {
            if(ROSTER.WEEK[m_RosterWeek].flags & (1<<m_Today))
            {
              bCase1 = (m_pRData[nI].flags & RDATA_FLAG_CREWONLY);
            }
            else
            {
              bCase1 = (m_pRData[nI].flags == 0);
            }
            if(bCase1)
            {
              m_pRData[nI].DRIVERSrecordID = ROSTER.DRIVERSrecordID;
              m_pRData[nI].rosterNumber = ROSTER.rosterNumber;
              m_pRData[nI].ROSTERrecordID = ROSTER.recordID;
              break;
            }
          }
        }
      }
      rcode2 = btrieve(B_GETNEXT, TMS_ROSTER, &ROSTER, &ROSTERKey1, 1);
    }
//
//  Save this so we don't have to do it again (or at least until the date changes)
//
    memcpy(m_pRDataHold, m_pRData, sizeof(RDataDef) * numRuns);
    m_numRunsInRDataHold = numRuns;
  }
//
//  Look for any run splits
//
//  The way the list is constructed, we can safely add a flag
//  to it (RDATA_FLAG_SUPERCEDED).  That way we can nuke out
//  record at the end.  Run Splits re-use the recordID, but 
//  there's no chance of a collision, as the original piece
//  of work always shows up first in the list.
//
  int numRunsNow = numRuns;

  DAILYOPSKey1.recordTypeFlag = DAILYOPS_FLAG_OPENWORK;
  DAILYOPSKey1.pertainsToDate = dateToUse;
  DAILYOPSKey1.pertainsToTime = NO_TIME;
  DAILYOPSKey1.recordFlags = 0;
  rcode2 = btrieve(B_GETGREATER, TMS_DAILYOPS, &DAILYOPS, &DAILYOPSKey1, 1);
  while(rcode2 == 0 &&
        (DAILYOPS.recordTypeFlag & DAILYOPS_FLAG_OPENWORK) &&
         DAILYOPS.pertainsToDate == dateToUse)
  {
    if(((DAILYOPS.recordFlags & DAILYOPS_FLAG_RUNSPLIT) ||
                (DAILYOPS.recordFlags & DAILYOPS_FLAG_RUNSPLITCREWONLY)))
    {
      if(!ANegatedRecord(DAILYOPS.recordID, 1))
      {
        for(nI = 0; nI < numRunsNow; nI++)
        {
          if(DAILYOPS.DOPS.OpenWork.RUNSrecordID == m_pRData[nI].recordID)
          { 
            for(bFound = FALSE, nJ = numRunsNow; nJ < numRuns; nJ++)
            {
              if(m_pRData[nJ].runNumber == m_pRData[nI].runNumber &&
                  m_pRData[nJ].onNODESrecordID == DAILYOPS.DOPS.OpenWork.splitStartNODESrecordID &&
                  m_pRData[nJ].onTime == DAILYOPS.DOPS.OpenWork.splitStartTime)
              {
                if(m_pRData[nJ].DRIVERSrecordID == NO_RECORD &&
                      DAILYOPS.recordFlags & DAILYOPS_FLAG_OPENWORKASSIGN)
                {
                  m_pRData[numRuns].DRIVERSrecordID = DAILYOPS.DRIVERSrecordID;
                }
                bFound = TRUE;
                break;
              }
            }
            if(!bFound)
            {
              m_pRData[nI].flags |= RDATA_FLAG_SUPERCEDED;
              m_pRData[numRuns].recordID = DAILYOPS.DOPS.OpenWork.RUNSrecordID;
              m_pRData[numRuns].runNumber = m_pRData[nI].runNumber;
              m_pRData[numRuns].cutAsRuntype = RUNSPLITRUNTYPE;
              m_pRData[numRuns].flags = m_pRData[nI].flags - RDATA_FLAG_SUPERCEDED;
              m_pRData[numRuns].onNODESrecordID = DAILYOPS.DOPS.OpenWork.splitStartNODESrecordID;
              m_pRData[numRuns].onTime = DAILYOPS.DOPS.OpenWork.splitStartTime;
              m_pRData[numRuns].offNODESrecordID = DAILYOPS.DOPS.OpenWork.splitEndNODESrecordID;
              m_pRData[numRuns].offTime = DAILYOPS.DOPS.OpenWork.splitEndTime;
              m_pRData[numRuns].DRIVERSrecordID = DAILYOPS.DRIVERSrecordID; //m_pRData[nI].DRIVERSrecordID;
              m_pRData[numRuns].rosterNumber = m_pRData[nI].rosterNumber;
              m_pRData[numRuns].ROSTERrecordID = m_pRData[nI].ROSTERrecordID;
              numRuns++;
              break;
            }
          }
        }
      }
    }
    rcode2 = btrieve(B_GETNEXT, TMS_DAILYOPS, &DAILYOPS, &DAILYOPSKey1, 1);
  }
//
//  Gotta have runs
//
  if(numRuns == 0)
  {
    m_numInOpenWork = 0;
    return;
  }
//
//  Take out the records that have been superceded
//
  for(nI = 0; nI < numRunsNow; nI++)
  {
    if(m_pRData[nI].flags & RDATA_FLAG_SUPERCEDED)
    {
      m_pRData[nI].recordID = NO_RECORD;
    }
  }
//
//  Look for any deassignments.
//
  DAILYOPSKey1.recordTypeFlag = DAILYOPS_FLAG_OPERATOR;
  DAILYOPSKey1.pertainsToDate = dateToUse;
  DAILYOPSKey1.pertainsToTime = 0;
  DAILYOPSKey1.recordFlags = 0;
  bFound = FALSE;
  rcode2 = btrieve(B_GETGREATEROREQUAL, TMS_DAILYOPS, &DAILYOPS, &DAILYOPSKey1, 1);
  while(rcode2 == 0 &&
        (DAILYOPS.recordTypeFlag & DAILYOPS_FLAG_OPERATOR) &&
         DAILYOPS.pertainsToDate == dateToUse)
  {
    if((DAILYOPS.recordFlags & DAILYOPS_FLAG_OPERATORDEASSIGN) &&
          !ANegatedRecord(DAILYOPS.recordID, 1))
    {
      for(nI = 0; nI < numRuns; nI++)
      {
        if(m_pRData[nI].DRIVERSrecordID == DAILYOPS.DRIVERSrecordID)
        {
          m_pRData[nI].DRIVERSrecordID = NO_RECORD;
        }
      }
    }
    rcode2 = btrieve(B_GETNEXT, TMS_DAILYOPS, &DAILYOPS, &DAILYOPSKey1, 1);
  }
//
//  Cycle through m_pRData and match it to the absence array to see if the assigned
//  operator is here today.  If they are present, then kill the run from the list by
//  setting the recordID portion to NO_RECORD.
//
  BOOL bShownRunSplit = FALSE;
  
  numRunsNow = numRuns;

  for(nI = 0; nI < numRunsNow; nI++)
  {
    if(m_pRData[nI].recordID == NO_RECORD || m_pRData[nI].DRIVERSrecordID == NO_RECORD)
    {
      continue;
    }
    if(m_pRData[nI].cutAsRuntype == RUNSPLITRUNTYPE)
    {
      continue;
    }
    if(m_pRData[nI].DRIVERSrecordID == 17)
    {
      int xx = 1;
    }
    for(bFound = FALSE, nJ = 0; nJ < m_numAbsent; nJ++)
    {
//
//  Is the operator away?
//
      if(m_pRData[nI].DRIVERSrecordID == m_AbsentList[nJ].DRIVERSrecordID)
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
        if(m_AbsentList[nJ].fromTime <= m_pRData[nI].onTime)
        {
          if(m_AbsentList[nJ].toTime >= m_pRData[nI].onTime)
          {
            bFound = TRUE;
            break;
          }
        }
        else
        {
          if(m_AbsentList[nJ].fromTime <= m_pRData[nI].offTime)
          {
            bFound = TRUE;
            break;
          }
        }
      }
    }
//
//  Not found?  Set the recordID to NO_RECORD if there's a driver assigned to the roster
//
    if(!bFound)
    { 
      if(m_pRData[nI].DRIVERSrecordID != NO_RECORD)
      {  
        m_pRData[nI].recordID = NO_RECORD;
      }
    }
//
//  Found.  Automatically generate a run split to accommodate the absence.
//  Only do this, though, if there's an under or overlap in the times.  Exact
//  matches mean a) the entire run is already open, or b) a run split was
//  already generated for this case.  Note that we know nJ from the break.
//
    else
    {
      BOOL bGenerateRunSplit = FALSE;
//
//  This is the "entire" or "existing run split" scenario
//
      if(m_AbsentList[nJ].fromTime <= m_pRData[nI].onTime && m_AbsentList[nJ].toTime >= m_pRData[nI].offTime)
      {
        m_pRData[nI].DRIVERSrecordID = NO_RECORD;
      }
//
//  Check for case 1 (from the above diagram)
//
      else if(m_AbsentList[nJ].fromTime <= m_pRData[nI].onTime && m_AbsentList[nJ].toTime < m_pRData[nI].offTime)
      {
        bGenerateRunSplit = TRUE;
      }
//
//  Check for case 2 (from the above diagram)
//
      else if(m_AbsentList[nJ].fromTime > m_pRData[nI].onTime && m_AbsentList[nJ].toTime < m_pRData[nI].offTime)
      {
        bGenerateRunSplit = TRUE;
      }
//
//  Check for case 3 (from the above diagram)
//
      else if(m_AbsentList[nJ].fromTime >= m_pRData[nI].onTime && m_AbsentList[nJ].toTime > m_pRData[nI].offTime) 
      {
        bGenerateRunSplit = TRUE;
      }
//
//  Generate the run split
//
      if(bGenerateRunSplit && m_pRData[nI].cutAsRuntype != RUNSPLITRUNTYPE)  // Don't split a runsplit
      {
        if(!bShownRunSplit)
        {
          MessageBeep(MB_ICONINFORMATION);
          MessageBox("At least one run split will be automatically\ngenerated to account for partial absences", TMS, MB_OK | MB_ICONINFORMATION);
          bShownRunSplit = TRUE;
        }

        RunSplitDef PassedData;

        PassedData.DRIVERSrecordID = m_pRData[nI].DRIVERSrecordID;
        PassedData.RUNSrecordID = m_pRData[nI].recordID;
        PassedData.SERVICESrecordIDInEffect = m_SERVICESrecordIDInEffect;
        PassedData.pertainsToDate = dateToUse;
        PassedData.fromTime = m_AbsentList[nJ].fromTime;
        PassedData.toTime = m_AbsentList[nJ].toTime;
        PassedData.flags = RUNSPLIT_FLAGS_AUTOMATIC;
        if(m_pRData[nI].flags & RDATA_FLAG_CREWONLY)
        {
          PassedData.flags |= RUNSPLIT_FLAGS_CREWONLY;
        }
        PassedData.numAdded = 0;

	      CDailyOpsRS dlg(this, &PassedData);
        dlg.DoModal();
//
//  Add the run splits into the structure
//
        for(nK = 0; nK < PassedData.numAdded; nK++)
        {
          DAILYOPSKey0.recordID = PassedData.DAILYOPSrecordIDs[nK];
          rcode2 = btrieve(B_GETEQUAL, TMS_DAILYOPS, &DAILYOPS, &DAILYOPSKey0, 0);
          if(rcode2 == 0)
          {
            for(nL = 0; nL < numRunsNow; nL++)
            {
              if(DAILYOPS.DOPS.OpenWork.RUNSrecordID == m_pRData[nL].recordID)
              {
                m_pRData[nL].flags |= RDATA_FLAG_SUPERCEDED;
                m_pRData[numRuns].recordID = DAILYOPS.DOPS.OpenWork.RUNSrecordID;
                m_pRData[numRuns].runNumber = m_pRData[nL].runNumber;
                m_pRData[numRuns].cutAsRuntype = RUNSPLITRUNTYPE;
                m_pRData[numRuns].flags = m_pRData[nL].flags - RDATA_FLAG_SUPERCEDED;
                m_pRData[numRuns].onNODESrecordID = DAILYOPS.DOPS.OpenWork.splitStartNODESrecordID;
                m_pRData[numRuns].onTime = DAILYOPS.DOPS.OpenWork.splitStartTime;
                m_pRData[numRuns].offNODESrecordID = DAILYOPS.DOPS.OpenWork.splitEndNODESrecordID;
                m_pRData[numRuns].offTime = DAILYOPS.DOPS.OpenWork.splitEndTime;
                m_pRData[numRuns].DRIVERSrecordID = m_pRData[nL].DRIVERSrecordID;
                m_pRData[numRuns].rosterNumber = m_pRData[nL].rosterNumber;
                m_pRData[numRuns].ROSTERrecordID = m_pRData[nL].ROSTERrecordID;
                numRuns++;
                break;
              }
            }
          }  
        }
//
//  Take out the records that have been superceded
//
        for(nK = 0; nK < numRunsNow; nK++)
        {
          if(m_pRData[nK].flags & RDATA_FLAG_SUPERCEDED)
          {
            m_pRData[nK].recordID = NO_RECORD;
          }
        }
      }
    }
  }
//
//  Construct the m_OpenWork structure
//
  LVITEM  LVI;
  long    row = 0;

  if(date == NO_RECORD && bDisplay)
  {
    pListCtrlOPENWORKLIST->DeleteAllItems();
  }
  m_numInOpenWork = 0;
//
//  Glean m_OpenWork from m_pRData
//
  for(nI = 0; nI < numRuns; nI++)
  {
    if(m_pRData[nI].recordID == NO_RECORD)
    {
      continue;
    }
    m_OpenWork[m_numInOpenWork].TRIPSrecordID = NO_RECORD;
    m_OpenWork[m_numInOpenWork].RUNSrecordID = m_pRData[nI].recordID;
    m_OpenWork[m_numInOpenWork].runNumber = m_pRData[nI].runNumber;
    m_OpenWork[m_numInOpenWork].cutAsRuntype = m_pRData[nI].cutAsRuntype;
    m_OpenWork[m_numInOpenWork].onNODESrecordID = m_pRData[nI].onNODESrecordID;
    m_OpenWork[m_numInOpenWork].onTime = m_pRData[nI].onTime;
    m_OpenWork[m_numInOpenWork].offNODESrecordID = m_pRData[nI].offNODESrecordID;
    m_OpenWork[m_numInOpenWork].offTime = m_pRData[nI].offTime;
    m_OpenWork[m_numInOpenWork].pay = m_pRData[nI].pay;
    m_OpenWork[m_numInOpenWork].DRIVERSrecordID = NO_RECORD;
    m_OpenWork[m_numInOpenWork].rosterNumber = m_pRData[nI].rosterNumber;
    m_OpenWork[m_numInOpenWork].ROSTERrecordID = m_pRData[nI].ROSTERrecordID;
    m_OpenWork[m_numInOpenWork].flags = m_pRData[nI].flags;
    m_OpenWork[m_numInOpenWork].propertyFlags = 0;
    m_numInOpenWork++;
  }
//
//  Apply any recosting to run splits
//
  for(nI = 0; nI < m_numInOpenWork; nI++)
  {
    if(m_OpenWork[nI].cutAsRuntype != RUNSPLITRUNTYPE || m_OpenWork[nI].pay != 0)
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
                   DAILYOPS.pertainsToDate == dateToUse)
    {
      if(DAILYOPS.DAILYOPSrecordID == NO_RECORD)
      {
        if(DAILYOPS.DOPS.OpenWork.RUNSrecordID == m_OpenWork[nI].RUNSrecordID &&
              (DAILYOPS.recordFlags & DAILYOPS_FLAG_RECOSTRUN))
        {
          if(DAILYOPS.DOPS.OpenWork.splitStartTime == m_OpenWork[nI].onTime &&
                DAILYOPS.DOPS.OpenWork.splitStartNODESrecordID == m_OpenWork[nI].onNODESrecordID &&
                DAILYOPS.DOPS.OpenWork.splitEndTime == m_OpenWork[nI].offTime &&
                DAILYOPS.DOPS.OpenWork.splitEndNODESrecordID == m_OpenWork[nI].offNODESrecordID)
          {
            m_OpenWork[nI].pay = DAILYOPS.DOPS.OpenWork.payTime;
            m_OpenWork[nI].propertyFlags |= OPENWORK_PROPERTYFLAG_RUNWASRECOSTED;
            break;
          }
        }
      }
      rcode2 = btrieve(B_GETPREVIOUS, TMS_DAILYOPS, &DAILYOPS, &DAILYOPSKey1, 1);
    }
  }
//
//  Check the audit trail for any assignments that have been made for today
//
  DAILYOPSKey1.recordTypeFlag = DAILYOPS_FLAG_OPENWORK;
  DAILYOPSKey1.pertainsToDate = dateToUse;
//  DAILYOPSKey1.pertainsToTime = 999999;
//  DAILYOPSKey1.recordFlags = 255;
//  rcode2 = btrieve(B_GETLESSTHAN, TMS_DAILYOPS, &DAILYOPS, &DAILYOPSKey1, 1);
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
        for(nI = 0; nI < m_numInOpenWork; nI++)
        {
          if(DAILYOPS.DOPS.OpenWork.RUNSrecordID == m_OpenWork[nI].RUNSrecordID)
          {
            if(m_OpenWork[nI].cutAsRuntype == RUNSPLITRUNTYPE)
            {
              if(DAILYOPS.DOPS.OpenWork.splitStartTime != m_OpenWork[nI].onTime ||
                    DAILYOPS.DOPS.OpenWork.splitStartNODESrecordID != m_OpenWork[nI].onNODESrecordID)
              {
                continue;
              }
            }
            m_OpenWork[nI].DRIVERSrecordID = DAILYOPS.DRIVERSrecordID;
            m_OpenWork[nI].propertyFlags |= OPENWORK_PROPERTYFLAG_ASSIGNEDTODAY;
            break;
          }
        }
      }
    }
//    rcode2 = btrieve(B_GETPREVIOUS, TMS_DAILYOPS, &DAILYOPS, &DAILYOPSKey1, 1);             
    rcode2 = btrieve(B_GETNEXT, TMS_DAILYOPS, &DAILYOPS, &DAILYOPSKey1, 1);             
  }
//
//  See if someone who was assigned a run is absent during the time they're required.
//
  for(nI = 0; nI < m_numInOpenWork; nI++)
  {
    if(!(m_OpenWork[nI].propertyFlags & OPENWORK_PROPERTYFLAG_ASSIGNEDTODAY))
    {
      continue;
    }
    if(m_OpenWork[nI].DRIVERSrecordID == NO_RECORD)
    {
      continue;
    }
    for(nJ = 0; nJ < m_numAbsent; nJ++)
    {
      if(m_OpenWork[nI].DRIVERSrecordID == m_AbsentList[nJ].DRIVERSrecordID)
      {
        if(m_AbsentList[nJ].fromTime <= m_OpenWork[nI].onTime && m_AbsentList[nJ].toTime >= m_OpenWork[nI].offTime)
        {
          m_OpenWork[nI].DRIVERSrecordID = NO_RECORD;
          continue;
        }
      }
    }
  }
//
//  Add some extra data for display/sorting purposes
//
  for(nI = 0; nI < m_numInOpenWork; nI++)
  {
//
//  Block number
//
    RUNSKey0.recordID = m_OpenWork[nI].RUNSrecordID;
    rcode2 = btrieve(B_GETEQUAL, TMS_RUNS, &RUNS, &RUNSKey0, 0);
    if(rcode2 != 0)
    {
      m_OpenWork[nI].blockNumber = NO_RECORD;
    }
    else
    {
      if(m_OpenWork[nI].cutAsRuntype == RUNSPLITRUNTYPE)
      {
        TRIPSKey0.recordID = m_OpenWork[nI].TRIPSrecordID;
      }
      else
      {
        TRIPSKey0.recordID = RUNS.start.TRIPSrecordID;
      }
      rcode2 = btrieve(B_GETEQUAL, TMS_TRIPS, &TRIPS, &TRIPSKey0, 0);
      if(rcode2 != 0)
      {
        m_OpenWork[nI].blockNumber = NO_RECORD;
      }
      else
      {
        m_OpenWork[nI].blockNumber = TRIPS.standard.blockNumber;
      }
    }
//
//  Runtype
//
    if(m_OpenWork[nI].cutAsRuntype == RUNSPLITRUNTYPE)
    {
      strcpy(m_OpenWork[nI].szCutAsRuntype, "*Run Split*");
    }
    else
    {
      nK = (short int)LOWORD(m_OpenWork[nI].cutAsRuntype);
      nL = (short int)HIWORD(m_OpenWork[nI].cutAsRuntype);
      if(nK >= 0 && nK < NUMRUNTYPES && nL >= 0 && nL < NUMRUNTYPESLOTS)
      {
        strcpy(m_OpenWork[nI].szCutAsRuntype, RUNTYPE[nK][nL].localName);
      }
      else 
      {
        strcpy(m_OpenWork[nI].szCutAsRuntype, "");
      }
    }

//
//  "On" node name
//
    if(m_OpenWork[nI].onNODESrecordID == NO_RECORD)
    {
      strcpy(m_OpenWork[nI].szOnNodesAbbrName, "");
    }
    else
    {
      NODESKey0.recordID = m_OpenWork[nI].onNODESrecordID;
      rcode2 = btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
      if(rcode2 != 0)
      {
        strcpy(m_OpenWork[nI].szOnNodesAbbrName, "");
      }
      else
      {
        strncpy(m_OpenWork[nI].szOnNodesAbbrName, NODES.abbrName, NODES_ABBRNAME_LENGTH);
        trim(m_OpenWork[nI].szOnNodesAbbrName, NODES_ABBRNAME_LENGTH);
      }
    }
//
//  "Off" node name
//
    if(m_OpenWork[nI].offNODESrecordID == NO_RECORD)
    {
      strcpy(m_OpenWork[nI].szOffNodesAbbrName, "");
    }
    else
    {
      NODESKey0.recordID = m_OpenWork[nI].offNODESrecordID;
      rcode2 = btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
      if(rcode2 != 0)
      {
        strcpy(m_OpenWork[nI].szOffNodesAbbrName, "");
      }
      else
      {
        strncpy(m_OpenWork[nI].szOffNodesAbbrName, NODES.abbrName, NODES_ABBRNAME_LENGTH);
        trim(m_OpenWork[nI].szOffNodesAbbrName, NODES_ABBRNAME_LENGTH);
      }
    }
//
//  Operator
//
    if(m_OpenWork[nI].DRIVERSrecordID == NO_RECORD)
    {
      strcpy(m_OpenWork[nI].szDriver, "");
    }
    else
    {
      strcpy(m_OpenWork[nI].szDriver, BuildOperatorString(m_OpenWork[nI].DRIVERSrecordID));
    }

  }
//
//  Display the values in the list control 
//
  if(date == NO_RECORD)
  {
    if(bDisplay)
    {
      int displayRow = 0;

      for(nI = 0; nI < m_numInOpenWork; nI++)
      {
//
//  Check to see if we're displaying everything, or just open work
//
        if(!m_bOpenWorkDisplay && m_OpenWork[nI].DRIVERSrecordID != NO_RECORD)
        {
          continue;
        }
//
//  Block number
//
        LVI.mask = LVIF_TEXT | LVIF_PARAM;
        if(m_OpenWork[nI].blockNumber == NO_RECORD)
        {
          strcpy(tempString, "-");
        }
        else
        {
          sprintf(tempString, "%ld", m_OpenWork[nI].blockNumber);
        }
        LVI.mask = LVIF_TEXT | LVIF_PARAM;
        LVI.iItem = displayRow;
        LVI.iSubItem = 0;
        LVI.pszText = tempString;
//        LVI.lParam = m_OpenWork[nI].RUNSrecordID;
        LVI.lParam = (LPARAM)&m_OpenWork[nI];
        LVI.iItem = pListCtrlOPENWORKLIST->InsertItem(&LVI);
//
//  Run number
//
        LVI.mask = LVIF_TEXT;
        LVI.iItem = displayRow;
        LVI.iSubItem = 1;
        sprintf(tempString, "%ld", m_OpenWork[nI].runNumber);
        LVI.pszText = tempString;
        LVI.iItem = pListCtrlOPENWORKLIST->SetItem(&LVI);
//
//  Runtype
//
        LVI.mask = LVIF_TEXT;
        LVI.iItem = displayRow;
        LVI.iSubItem = 2;
        LVI.pszText = m_OpenWork[nI].szCutAsRuntype;
        LVI.iItem = pListCtrlOPENWORKLIST->SetItem(&LVI);
//
//  On
//
        LVI.mask = LVIF_TEXT;
        LVI.iItem = displayRow;
        LVI.iSubItem = 3;
        LVI.pszText = m_OpenWork[nI].szOnNodesAbbrName;
        LVI.iItem = pListCtrlOPENWORKLIST->SetItem(&LVI);
//
//  At
//
        LVI.mask = LVIF_TEXT;
        LVI.iItem = displayRow;
        LVI.iSubItem = 4;
        strcpy(tempString, Tchar(m_OpenWork[nI].onTime));
        LVI.pszText = tempString;
        LVI.iItem = pListCtrlOPENWORKLIST->SetItem(&LVI);
//
//  Off
//
        LVI.mask = LVIF_TEXT;
        LVI.iItem = displayRow;
        LVI.iSubItem = 5;
        LVI.pszText = m_OpenWork[nI].szOffNodesAbbrName;
        LVI.iItem = pListCtrlOPENWORKLIST->SetItem(&LVI);
//
//  At
//
        LVI.mask = LVIF_TEXT;
        LVI.iItem = displayRow;
        LVI.iSubItem = 6;
        strcpy(tempString, Tchar(m_OpenWork[nI].offTime));
        LVI.pszText = tempString;
        LVI.iItem = pListCtrlOPENWORKLIST->SetItem(&LVI);
//
//  Pay
//
        LVI.mask = LVIF_TEXT;
        LVI.iItem = displayRow;
        LVI.iSubItem = 7;
        if(m_OpenWork[nI].cutAsRuntype == RUNSPLITRUNTYPE && m_OpenWork[nI].pay == 0)
        {
          strcpy(tempString, "TBD");
        }
        else
        {
          strcpy(tempString, chhmm(m_OpenWork[nI].pay));
        }
        if(m_OpenWork[nI].propertyFlags & OPENWORK_PROPERTYFLAG_RUNWASRECOSTED)
        {
          strcat(tempString, " *");
        }
        LVI.pszText = tempString;
        LVI.iItem = pListCtrlOPENWORKLIST->SetItem(&LVI);
//
//  Operator
//
        LVI.mask = LVIF_TEXT;
        LVI.iItem = displayRow;
        LVI.iSubItem = 8;
        LVI.pszText = m_OpenWork[nI].szDriver;
        LVI.iItem = pListCtrlOPENWORKLIST->SetItem(&LVI);

        displayRow++;
      }
    }
  }
  m_numRunsInRData = numRuns;
}

void CDailyOps::OnOWMProperties() 
{
  m_PreviousTime = m_SystemTime;

	int nI = pListCtrlOPENWORKLIST->GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);

  if(nI < 0)
  {
    return;
  }

  OPENWORKDef *pOW;
  GetOWMPointer(nI, &pOW);

  long RUNSrecordID = pOW->RUNSrecordID;
  int  rcode2;

  RUNSKey0.recordID = RUNSrecordID;
  rcode2 = btrieve(B_GETEQUAL, TMS_RUNS, &RUNS, &RUNSKey0, 0);
  if(rcode2 == 0 && RUNS.pieceNumber == 1)
  {
    ANALYZERUNPassedDataDef ANALYZERUNPassedData;

    ANALYZERUNPassedData.flags = ANALYZERUN_FLAGS_CHILDWND;
    ANALYZERUNPassedData.longValue = RUNS.recordID;
    DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_ANALYZERUN),
          m_hWnd, (DLGPROC)ANALYZERUNMsgProc, (LPARAM)&ANALYZERUNPassedData);
  }
}

long CDailyOps::SetupRun(long RUNSrecordID, BOOL bCrewOnly, PROPOSEDRUNDef* pPR)
{
  long runNumber;
  long divisionRecordID;
  long serviceRecordID;
  long pieceNumber;
  long onTime;
  long offTime;
  long cutAsRuntype;
  int  rcode2;

  if(bCrewOnly)
  {
    CREWONLYKey0.recordID = RUNSrecordID;
    rcode2 = btrieve(B_GETEQUAL, TMS_CREWONLY, &CREWONLY, &CREWONLYKey0, 0);
    rcode2 = btrieve(B_GETPOSITION, TMS_CREWONLY, &CREWONLY, &CREWONLYKey0, 0);
    rcode2 = btrieve(B_GETDIRECT, TMS_CREWONLY, &CREWONLY, &CREWONLYKey1, 1);
    runNumber = CREWONLY.runNumber;
    divisionRecordID = CREWONLY.DIVISIONSrecordID;
    serviceRecordID = CREWONLY.SERVICESrecordID;
    pieceNumber = 0;
    cutAsRuntype = CREWONLY.cutAsRuntype;
    while(rcode2 == 0 &&
          CREWONLY.runNumber == runNumber && 
          CREWONLY.SERVICESrecordID == serviceRecordID)
    {
      if(CREWONLY.DIVISIONSrecordID == divisionRecordID)
      {
        pPR->piece[pieceNumber].fromTime = CREWONLY.startTime;
        pPR->piece[pieceNumber].fromNODESrecordID = CREWONLY.startNODESrecordID;
        pPR->piece[pieceNumber].fromTRIPSrecordID = NO_RECORD;
        pPR->piece[pieceNumber].toTime = CREWONLY.endTime;
        pPR->piece[pieceNumber].toNODESrecordID = CREWONLY.endNODESrecordID;
        pPR->piece[pieceNumber].toTRIPSrecordID = NO_RECORD;
//
//  Get the next (crewonly) run record
//
        rcode2 = btrieve(B_GETNEXT, TMS_CREWONLY, &CREWONLY, &CREWONLYKey1, 1);
        pieceNumber++;
      }
    }
  }
  else
  {
    RUNSKey0.recordID = RUNSrecordID;
    rcode2 = btrieve(B_GETEQUAL, TMS_RUNS, &RUNS, &RUNSKey0, 0);
    rcode2 = btrieve(B_GETPOSITION, TMS_RUNS, &RUNS, &RUNSKey0, 0);
    rcode2 = btrieve(B_GETDIRECT, TMS_RUNS, &RUNS, &RUNSKey1, 1);
    runNumber = RUNS.runNumber;
    divisionRecordID = RUNS.DIVISIONSrecordID;
    serviceRecordID = RUNS.SERVICESrecordID;
    pieceNumber = 0;
    cutAsRuntype = RUNS.cutAsRuntype;
    while(rcode2 == 0 &&
          RUNS.runNumber == runNumber && 
          RUNS.SERVICESrecordID == serviceRecordID)
    {
      if(RUNS.DIVISIONSrecordID == divisionRecordID)
      {
        RunStartAndEnd(RUNS.start.TRIPSrecordID, RUNS.start.NODESrecordID,
              RUNS.end.TRIPSrecordID, RUNS.end.NODESrecordID, &onTime, &offTime);
        pPR->piece[(int)pieceNumber].fromTime = onTime;
        pPR->piece[(int)pieceNumber].fromNODESrecordID = RUNS.start.NODESrecordID;
        pPR->piece[(int)pieceNumber].fromTRIPSrecordID = RUNS.start.TRIPSrecordID;
        pPR->piece[(int)pieceNumber].toTime = offTime;
        pPR->piece[(int)pieceNumber].toNODESrecordID = RUNS.end.NODESrecordID;
        pPR->piece[(int)pieceNumber].toTRIPSrecordID = RUNS.end.TRIPSrecordID;
        pieceNumber++;
      }
      rcode2 = btrieve(B_GETNEXT, TMS_RUNS, &RUNS, &RUNSKey1, 1);
    }
  }
  pPR->numPieces = (int)pieceNumber;

  return(cutAsRuntype);
}
//
// OnRegister() - Register an absence
//
void CDailyOps::OnRegister() 
{
  m_PreviousTime = m_SystemTime;

  DAILYOPSALDef PassedData;
  CString s;
  int  nI;
//
//  Get the operator
//
  nI = pComboBoxOPERATOR->GetCurSel();
  if(nI == CB_ERR)
  {
    return;
  }
  PassedData.DRIVERSrecordID = pComboBoxOPERATOR->GetItemData(nI);
//
//  Record the absence/lateness
//
  PassedData.SystemTime = m_SystemTime;

  CDailyOpsAL dlg(this, &PassedData);

  if(dlg.DoModal() == IDOK)
  {
    m_numAbsent = RefreshAbsenceList(m_DailyOpsDateYYYYMMDD, m_AbsentList);
//
//  Deassign him from the work
//
/*
  DailyOpsBuildRecord(&DAILYOPS, DAILYOPS_FLAG_OPERATOR);
  DAILYOPS.pertainsToDate = m_DailyOpsDateYYYYMMDD;
  DAILYOPS.pertainsToTime = m_DailyOpsDate.GetHour() * 3600 + m_DailyOpsDate.GetMinute() * 60 + m_DailyOpsDate.GetSecond();
  DAILYOPS.recordFlags = DAILYOPS_FLAG_OPERATORDEASSIGN;
  DAILYOPS.DRIVERSrecordID = PassedData.DRIVERSrecordID;
  rcode2 = btrieve(B_INSERT, TMS_DAILYOPS, &DAILYOPS, &DAILYOPSKey0, 0);
  if(rcode2 == 0)
  {
    m_LastDAILYOPSRecordID = DAILYOPS.recordID;
    s.LoadString(TEXT_398);
    sprintf(tempString, s, m_SystemTime.Format(_T("%I:%M:%S%p")), BuildOperatorString(PassedData.DRIVERSrecordID));
    pEditMESSAGES->SetSel(-1, -1);
    pEditMESSAGES->ReplaceSel(tempString);
  }
*/
//
//  Update the Open Work monitor
//
//  If we're registering an all-day absence, we don't need any of the
//  logic contained in PopulateVehicleAssignment().  All that needs
//  to be done is to fix th pointer and the display
//
    if(m_DailyOpsDateYYYYMMDD >= PassedData.fromDate && m_DailyOpsDateYYYYMMDD <= PassedData.toDate)
    {
      if(PassedData.fromTime == 0 && PassedData.toTime >= T1159P)
      {
        for(nI = 0; nI < m_numBlocksInBlockInfo; nI++)
        {
          if(m_BlockInfo[nI].startingDRIVERSrecordID == PassedData.DRIVERSrecordID)
          {
            m_BlockInfo[nI].startingDRIVERSrecordID = NO_RECORD;
            RefreshBlockList(nI);
          }
        }
      }
      else
      {
        PopulateVehicleAssignment();
      }
      PopulateOpenWorkMonitor(NO_RECORD, FALSE, TRUE);
    }
  }
}

void CDailyOps::OnDrop() 
{
  m_PreviousTime = m_SystemTime;

  CString s;
  long DAILYOPSrecordID;
  int  rcode2;
//
//  Process either an "undrop" or a "drop"
//
//  "Undrop"
//
  if(m_BlockInfo[m_SelectedBlockIndex].flags & BLOCKINFO_FLAG_BLOCKDROP)
  {
    DAILYOPSKey1.recordTypeFlag = DAILYOPS_FLAG_BUS;
    DAILYOPSKey1.pertainsToDate = m_DailyOpsDateYYYYMMDD;
    DAILYOPSKey1.pertainsToTime = NO_TIME;
    DAILYOPSKey1.recordFlags = 0;
    rcode2 = btrieve(B_GETGREATER, TMS_DAILYOPS, &DAILYOPS, &DAILYOPSKey1, 1);
    while(rcode2 == 0 &&
          DAILYOPS.recordTypeFlag == DAILYOPS_FLAG_BUS &&
          DAILYOPS.pertainsToDate == m_DailyOpsDateYYYYMMDD)
    {
      if(DAILYOPS.recordFlags == DAILYOPS_FLAG_BLOCKDROP)
      {
        if(DAILYOPS.DOPS.Bus.TRIPSrecordID == m_BlockInfo[m_SelectedBlockIndex].TRIPSrecordID &&
              DAILYOPS.DOPS.Bus.RGRPROUTESrecordID == m_BlockInfo[m_SelectedBlockIndex].RGRPROUTESrecordID &&
              DAILYOPS.DOPS.Bus.SGRPSERVICESrecordID == m_BlockInfo[m_SelectedBlockIndex].SGRPSERVICESrecordID)
        {
          m_BlockInfo[m_SelectedBlockIndex].flags &= ~BLOCKINFO_FLAG_BLOCKDROP;
          DAILYOPSrecordID = DAILYOPS.recordID;
          DailyOpsBuildRecord(&DAILYOPS, DAILYOPS_FLAG_BUS);
          DAILYOPS.pertainsToDate = m_DailyOpsDateYYYYMMDD;
//          DAILYOPS.pertainsToTime = m_DailyOpsDate.GetHour() * 3600 + m_DailyOpsDate.GetMinute() * 60 + m_DailyOpsDate.GetSecond();
          DAILYOPS.pertainsToTime = DAILYOPS.recordID;
          DAILYOPS.DAILYOPSrecordID = DAILYOPSrecordID;
          DAILYOPS.recordFlags = DAILYOPS_FLAG_BLOCKUNDROP;
          DAILYOPS.DOPS.Bus.TRIPSrecordID = m_BlockInfo[m_SelectedBlockIndex].TRIPSrecordID;
          DAILYOPS.DOPS.Bus.RGRPROUTESrecordID = m_BlockInfo[m_SelectedBlockIndex].RGRPROUTESrecordID;
          DAILYOPS.DOPS.Bus.SGRPSERVICESrecordID = m_BlockInfo[m_SelectedBlockIndex].SGRPSERVICESrecordID;
          rcode2 = btrieve(B_INSERT, TMS_DAILYOPS, &DAILYOPS, &DAILYOPSKey0, 0);
          if(rcode2 == 0)
          {
            m_LastDAILYOPSRecordID = DAILYOPS.recordID;
          }
          break;
        }
      }
      rcode2 = btrieve(B_GETNEXT, TMS_DAILYOPS, &DAILYOPS, &DAILYOPSKey1, 1);
    }
  }
  else
  {
    m_BlockInfo[m_SelectedBlockIndex].flags = BLOCKINFO_FLAG_BLOCKDROP;
//
//  Record the drop
//
//  Database verify: 11-Jan-07
//
    DailyOpsBuildRecord(&DAILYOPS, DAILYOPS_FLAG_BUS);
    DAILYOPS.pertainsToDate = m_DailyOpsDateYYYYMMDD;
//    DAILYOPS.pertainsToTime = m_DailyOpsDate.GetHour() * 3600 + m_DailyOpsDate.GetMinute() * 60 + m_DailyOpsDate.GetSecond();
    DAILYOPS.pertainsToTime = DAILYOPS.recordID;
    DAILYOPS.recordFlags = DAILYOPS_FLAG_BLOCKDROP;
    DAILYOPS.DOPS.Bus.TRIPSrecordID = m_BlockInfo[m_SelectedBlockIndex].TRIPSrecordID;
    DAILYOPS.DOPS.Bus.RGRPROUTESrecordID = m_BlockInfo[m_SelectedBlockIndex].RGRPROUTESrecordID;
    DAILYOPS.DOPS.Bus.SGRPSERVICESrecordID = m_BlockInfo[m_SelectedBlockIndex].SGRPSERVICESrecordID;
    rcode2 = btrieve(B_INSERT, TMS_DAILYOPS, &DAILYOPS, &DAILYOPSKey0, 0);
    if(rcode2 == 0)
    {
      m_LastDAILYOPSRecordID = DAILYOPS.recordID;
    }
  }
//
//  Update the display
//
  RefreshBlockList(m_SelectedBlockRow);
}

void CDailyOps::OnReport() 
{
  m_PreviousTime = m_SystemTime;

  long date = NO_RECORD;

  CPickDate dlg(this, &date);

  if(dlg.DoModal() != IDOK)
  {
    return;
  }

  long yyyy, mm, dd;

  GetYMD(date, &yyyy, &mm, &dd);
  
  CTime spanDate(yyyy, mm, dd, 0, 0, 0);

  if(spanDate.GetDayOfWeek() != 2)
  {
    MessageBox("Please select a Monday", TMS, MB_OK);
    return;
  }

  CString s;

  int   days[ROSTER_MAX_DAYS] = {TEXT_009, TEXT_010, TEXT_011,
                                 TEXT_012, TEXT_013, TEXT_014, TEXT_015};
  int   nI;
  int   nJ;
  int   nK;
  int   nL;
  int   nM;
  int   nN;
  int   rcode2;
  char  outputString[512];
  HANDLE hOutputFile;
  DWORD  dwBytesWritten;
  BOOL   bShowTotalHours;

//
//  Open the output file
//
  strcpy(tempString, szReportsTempFolder);
  strcat(tempString, "\\DOMRPT01.txt");
  hOutputFile = CreateFile(tempString, GENERIC_WRITE, 0, NULL,
        CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
  if(hOutputFile == INVALID_HANDLE_VALUE)
  {
    LoadString(hInst, ERROR_202, szFormatString, sizeof(szFormatString));
    sprintf(szarString, szFormatString, tempString);
    MessageBeep(MB_ICONSTOP);
    MessageBox(szarString, TMS, MB_ICONSTOP);
    return;
  }

  int seq = 0;
//
//  Set up the header
//
  char line[5][256];
  char header[5][256];

  strcpy(header[0], "\"Day             |");
  strcpy(header[1], "\"=================");
  strcpy(header[2], "\"Date            |");     
  strcpy(header[3], "\"=================");              
  CTime currentDate = spanDate;
  int indexToRoster[7] = {6, 0, 1, 2, 3, 4, 5};
  char  szDays[7][4] = {"Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun"};

  currentDate -= CTimeSpan(1, 0, 0, 0);
  for(nI = 0; nI < ROSTER_MAX_DAYS; nI++)
  {
    currentDate += CTimeSpan(1, 0, 0, 0);
    s = currentDate.Format(_T("%d-%b-%y"));
    strcat(header[0], "     ");
    strcat(header[0], szDays[nI]);
    strcat(header[0], "    |");
    strcat(header[1], "============|");
    strcat(header[2], "  ");
    strcat(header[2], s);
    strcat(header[2], " |");
    strcat(header[3], "============|");
  }
//
//  Add a column for total hours
//
  strcat(header[0], "\"\r\n");
  strcat(header[1], "============|\"\r\n");
  strcat(header[2], " Total Hours|\"\r\n");
  strcat(header[3], "============|\"\r\n");

  WeeklyDef *pWeekly = NULL;
  int currentMaxWeekly = MAXWEEKLY;
  int numWeekly;
  
  pWeekly = (WeeklyDef *)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(WeeklyDef) * currentMaxWeekly); 
  if(pWeekly == NULL)
  {
    AllocationError(__FILE__, __LINE__, FALSE);
    return;
  }
//
//  Clear the structure
//
  for(nI = 0; nI < currentMaxWeekly; nI++)
  {
    for(nJ = 0; nJ < 7; nJ++)
    {
      for(nK = 0; nK < 4; nK++)
      {
        pWeekly[nI].runNumber[nJ][nK] = NO_RECORD;
        pWeekly[nI].payTime[nJ][nK] = 0;
      }
    }
  }
//
//  See if he wants the total hours printed out
//
  s.LoadString(TEXT_311);
  MessageBeep(MB_ICONQUESTION);
  bShowTotalHours = (MessageBox(s, TMS, MB_YESNO) == IDYES);
//
//  Cycle through the week's work
//
  HCURSOR hCursor = ::SetCursor(hCursorWait);
  BOOL bFound;
  long YMD;
  int  driverIndex;
  int  runIndex;
  int  t;

  currentDate = spanDate;
  currentDate -= CTimeSpan(1, 0, 0, 0);
  numWeekly = 0;
  for(t = 0; t < ROSTER_MAX_DAYS; t++)
  {
    currentDate += CTimeSpan(1, 0, 0, 0);
    YMD = currentDate.GetYear() * 10000 + currentDate.GetMonth() * 100 + currentDate.GetDay();
    sprintf(tempString, "%s\tProcessing: %ld\r\n", m_SystemTime.Format(_T("%I:%M:%S%p")), YMD);
    pEditMESSAGES->SetSel(-1, -1);
    pEditMESSAGES->ReplaceSel(tempString);
    m_numAbsent = RefreshAbsenceList(YMD, m_AbsentList);
    PopulateOpenWorkMonitor(YMD, TRUE, TRUE);
//
//  Save the structure
//
    for(nJ = 0; nJ < m_numInOpenWork; nJ++)
    {
      bFound = FALSE;
      if(m_OpenWork[nJ].DRIVERSrecordID != NO_RECORD)
      {
        for(nK = 0; nK < numWeekly; nK++)
        {
          if(pWeekly[nK].DRIVERSrecordID == m_OpenWork[nJ].DRIVERSrecordID)
          {
            bFound = TRUE;
            driverIndex = nK;
            runIndex = NO_RECORD;
            for(nL = 0; nL < 4; nL++)
            {
              if(pWeekly[nK].runNumber[t][nL] == NO_RECORD)
              {
                runIndex = nL;
                break;
              }
            }
            if(runIndex == NO_RECORD)
            {
              bFound = FALSE;
            }
            break;
          }
        }
      }
      if(!bFound)
      {
        driverIndex = numWeekly;
        runIndex = 0;
        pWeekly[driverIndex].DRIVERSrecordID = m_OpenWork[nJ].DRIVERSrecordID;
      }
      pWeekly[driverIndex].runNumber[t][runIndex] = m_OpenWork[nJ].runNumber;
      pWeekly[driverIndex].cutAsRuntype[t][runIndex] = m_OpenWork[nJ].cutAsRuntype;
      pWeekly[driverIndex].payTime[t][runIndex] = m_OpenWork[nJ].pay;
      if(!bFound)
      {
        numWeekly++;
        if(numWeekly >= currentMaxWeekly)
        {
          nK = currentMaxWeekly;
          currentMaxWeekly += 250;
          pWeekly = (WeeklyDef *)HeapReAlloc(GetProcessHeap(),
                HEAP_ZERO_MEMORY, pWeekly, sizeof(WeeklyDef) * currentMaxWeekly); 
          if(pWeekly == NULL)
          {
            AllocationError(__FILE__, __LINE__, TRUE);
            return;
          }
          for(nL = nK; nL < currentMaxWeekly; nL++)
          {
            for(nM = 0; nM < 7; nM++)
            {
              for(nN = 0; nN < 4; nN++)
              {
                pWeekly[nL].runNumber[nM][nN] = NO_RECORD;
                pWeekly[nL].payTime[nM][nN] = 0;
              }
            }
          }
        }
      }
    }
  }
//
//  Sort the file based on seniority date.  If the driver doesn't have a
//  driver type of 1 (assume full-time), then add 10000000 (1,000 years)
//  to the seniority date to bubble him to the bottom
//
  for(nI = 0; nI < numWeekly; nI++)
  {
    DRIVERSKey0.recordID = pWeekly[nI].DRIVERSrecordID;
    if(DRIVERSKey0.recordID == NO_RECORD)
    {
      pWeekly[nI].seniorityDate = 50000000 + nI;
    }
    else
    {
      rcode2 = btrieve(B_GETEQUAL, TMS_DRIVERS, &DRIVERS, &DRIVERSKey0, 0);
      if(rcode2 == 0)
      {
        pWeekly[nI].seniorityDate = DRIVERS.seniorityDate;
        if(DRIVERS.DRIVERTYPESrecordID != 1)
        {
          pWeekly[nI].seniorityDate += 10000000;
        }
      }
      else
      {
        pWeekly[nI].seniorityDate = 10000000;
      }
    }
  }
  qsort((void *)pWeekly, numWeekly, sizeof(WeeklyDef), sort_Weekly);

//
//  Write out the text file
//
  char badgeNumber[DRIVERS_BADGENUMBER_LENGTH + 1];
  char lastName[DRIVERS_LASTNAME_LENGTH + 1];
  char firstName[DRIVERS_FIRSTNAME_LENGTH + 1];
  long totalHours;
  BOOL bDoneOne = FALSE;

  for(nI = 0; nI < numWeekly; nI++)
  {
//
//  Write the header every nine assignments
//
    if(nI % 9 == 0)
    {
      if(bDoneOne)  // Dump out two blank lines at the end of a page
      {
        sprintf(outputString, "%d\t\"*Run indicates a partial run.  Please see dispatcher for details.\"\r\n", seq++);
        WriteFile(hOutputFile, (LPCVOID *)outputString, strlen(outputString), &dwBytesWritten, NULL);
        sprintf(outputString, "%d\t\"\"\r\n", seq++);
        WriteFile(hOutputFile, (LPCVOID *)outputString, strlen(outputString), &dwBytesWritten, NULL);
      }
      bDoneOne = TRUE;
      for(nJ = 0; nJ < 4; nJ++)
      {
        sprintf(outputString, "%d\t%s", seq++, header[nJ]);
        WriteFile(hOutputFile, (LPCVOID *)outputString, strlen(outputString), &dwBytesWritten, NULL);
      }
    }
    if(pWeekly[nI].DRIVERSrecordID == NO_RECORD)
    {
      strcpy(badgeNumber, "Unassigned");
      pad(badgeNumber, DRIVERS_BADGENUMBER_LENGTH);
      memset(firstName, ' ', DRIVERS_BADGENUMBER_LENGTH);
      memset(lastName, ' ', DRIVERS_BADGENUMBER_LENGTH);
    }
    else
    {
      DRIVERSKey0.recordID = pWeekly[nI].DRIVERSrecordID;
      rcode2 = btrieve(B_GETEQUAL, TMS_DRIVERS, &DRIVERS, &DRIVERSKey0, 0);
      if(rcode2 != 0)
      {
        strcpy(badgeNumber, "Not found");
        pad(badgeNumber, DRIVERS_BADGENUMBER_LENGTH);
      }
      else
      {
        strncpy(badgeNumber, DRIVERS.badgeNumber, DRIVERS_BADGENUMBER_LENGTH);
        strncpy(firstName, DRIVERS.firstName, DRIVERS_FIRSTNAME_LENGTH);
        strncpy(lastName, DRIVERS.lastName, DRIVERS_LASTNAME_LENGTH);
      }
    }
    badgeNumber[DRIVERS_BADGENUMBER_LENGTH] = '\0';
    firstName[DRIVERS_BADGENUMBER_LENGTH] = '\0';
    lastName[DRIVERS_BADGENUMBER_LENGTH] = '\0';
    strcpy(line[0],"\"");
    strcat(line[0], badgeNumber);
    strcat(line[0], "|");
    strcpy(line[1],"\"");
    strcat(line[1], lastName);
    strcat(line[1], "|");
    strcpy(line[2],"\"");
    strcat(line[2], firstName);
    strcat(line[2], "|");
    strcpy(line[3], "\"                |");
//
//  Go through each day
//
    for(nJ = 0; nJ < ROSTER_MAX_DAYS; nJ++)
    {
      for(nK = 0; nK < 4; nK++)
      {
        if(pWeekly[nI].runNumber[nJ][nK] == NO_RECORD)
        {
          strcat(line[nK], "            |");
        }
        else
        {
          sprintf(tempString, "%sRun: %6ld|",
                (pWeekly[nI].cutAsRuntype[nJ][nK] == RUNSPLITRUNTYPE ? "*" : " "),
                 pWeekly[nI].runNumber[nJ][nK]);
          strcat(line[nK], tempString);
        }
      }
    }
    for(nJ = 0; nJ < 4; nJ++)
    {
      if(nJ == 2 && bShowTotalHours)
      {
        totalHours = 0;

        for(nK = 0; nK < 7; nK++)
        {
          for(nL = 0; nL < 4; nL++)
          {
            if(pWeekly[nI].runNumber[nK][nL] == NO_RECORD)
            {
              break;
            }
            totalHours += pWeekly[nI].payTime[nK][nL];
          }
        }
        strcat(line[2], "    ");
        strcat(line[2], chhmm(totalHours));
        if(totalHours >= 36000)  // 10:00
        {
          strcat(line[2], "   |\"\r\n");
        }
        else
        {
          strcat(line[2], "    |\"\r\n");
        }
      }
      else
      {
        strcat(line[nJ], "            |\"\r\n");
      }
      sprintf(outputString, "%d\t%s", seq++, line[nJ]);
      WriteFile(hOutputFile, (LPCVOID *)outputString, strlen(outputString), &dwBytesWritten, NULL);
    }
    strcpy(tempString, "\"-----------------");              
    for(nJ = 0; nJ < ROSTER_MAX_DAYS + 1; nJ++)
    {
      strcat(tempString, "------------|");
    }
    strcat(tempString, "\"\r\n");
    sprintf(outputString, "%d\t%s", seq++, tempString);
    WriteFile(hOutputFile, (LPCVOID *)outputString, strlen(outputString), &dwBytesWritten, NULL);
  }

  CloseHandle(hOutputFile);
//
//  Reset the Open Work Monitor
//
  m_numAbsent = RefreshAbsenceList(m_DailyOpsDateYYYYMMDD, m_AbsentList);
  PopulateOpenWorkMonitor(NO_RECORD, TRUE, TRUE);
  SetCursor(hCursor);
//
//  Fire the report
//
  CString sRegKey = "Software\\ODBC\\ODBC.INI\\TMS MSJet Text";
  FILE  *fp;
  DWORD  type = REG_SZ;
  HKEY hKey;
  long result;
  char szReportName[TMSRPT_REPORTNAME_LENGTH + 1];
  char szReportFileName[TMSRPT_REPORTFILENAME_LENGTH + 1];
  int  thisReport;

//
//  Make sure the template exists (Report 51)
//
  for(thisReport = NO_RECORD, nI = 0; nI < TMSRPT_MAX_REPORTSDEFINED; nI++)
  {
    if(TMSRPT[nI].originalReportNumber == 51)
    {
      thisReport = nI;
      break;
    }
  }
  if(thisReport == NO_RECORD || 
        strcmp(TMSRPT[thisReport].szReportFileName[0], "") == 0)
  {
    fp = NULL;
  }
  else
  {
    fp = fopen(TMSRPT[thisReport].szReportFileName[0], "r");
  }
  if(fp == NULL)
  {
    s.LoadString(ERROR_197);
    sprintf(tempString, s, TMSRPT[thisReport].szReportFileName[0]);
    MessageBeep(MB_ICONSTOP);
    MessageBox(tempString, TMS, MB_OK);
  }
//
//  It does...
//
  else
  {

    strcpy(szReportName, TMSRPT[thisReport].szReportName);
    strcpy(szReportFileName, TMSRPT[thisReport].szReportFileName[0]);
    fclose(fp);

    result = RegOpenKeyEx(HKEY_CURRENT_USER, sRegKey, 0, KEY_SET_VALUE, &hKey);
    if(result != 0)
    {
      LoadString(hInst, ERROR_240, szFormatString, sizeof(szFormatString));
      sprintf(tempString, szFormatString, sRegKey);
      MessageBeep(MB_ICONSTOP);
      MessageBox(tempString, TMS, MB_OK);
    }
//
//  Set the registry entries so Crystal knows where to look
//
    else
    {
      strcpy(szarString, szReportsTempFolder);
      RegSetValueEx(hKey, "DefaultDir", 0, type, (LPBYTE)szarString, strlen(szarString));
//
//  Copy the Schema.INI file from the install directory into the temporary report folder.
//  When we do this each time, we ensure that he's getting the most recent version
//
      char fileName[2][MAX_PATH];

      for(nJ = 0; nJ < 2; nJ++)
      {
        strcpy(fileName[nJ], nJ == 0 ? szInstallDirectory : szarString);
        strcat(fileName[nJ], "\\Schema.INI");
      }
      if(GetFileAttributes(fileName[0]) == 0xFFFFFFFF)  // Couldn't find INI file in szInstallDirectory
      {
        TMSError(NULL, MB_ICONSTOP, ERROR_218, (HANDLE)NULL);
      }
//
//  Found the INI file
//
      else
      {
        CopyFile(fileName[0], fileName[1], FALSE);
        StatusBarStart(hWndMain, "Activating Crystal Interface");
        StatusBarText("Processing...");

        CCrystal dlg(this, szReportFileName, szReportName);
        dlg.DoModal();

        StatusBarEnd();
      }
    }
  }
//
//  All done
//
  TMSHeapFree(pWeekly);
}

//
//  Unregister a registered absence
//
void CDailyOps::OnCorrectabsence() 
{
  m_PreviousTime = m_SystemTime;

  int  nI = pComboBoxOPERATOR->GetCurSel();;

  if(nI == CB_ERR)
  {
    return;
  }

  long DRIVERSrecordID = pComboBoxOPERATOR->GetItemData(nI);
  BOOL bChanged;

  CDailyOpsSS dlg(this, DRIVERSrecordID, DAILYOPS_FLAG_ABSENCE, &bChanged);

  dlg.DoModal();
  
  if(bChanged)
  {
    m_numAbsent = RefreshAbsenceList(m_DailyOpsDateYYYYMMDD, m_AbsentList);
    PopulateOpenWorkMonitor(NO_RECORD, FALSE, TRUE);
    PopulateVehicleAssignment();
  }
}

//
//  Show a summary of absences registered for today 
//
void CDailyOps::OnShow() 
{
  m_PreviousTime = m_SystemTime;

  CDailyOpsAB dlg(this, m_DailyOpsDateYYYYMMDD, FALSE);

  dlg.DoModal();
}

void CDailyOps::OnDblclkOpenworklist(NMHDR* pNMHDR, LRESULT* pResult) 
{
  m_PreviousTime = m_SystemTime;

  ProcessSelectedItem(pListCtrlOPENWORKLIST);
	
//
//  If there's an operator, remove him.
//  If there's no operator, add one.
//

  OPENWORKDef *pOW;
  GetOWMPointer(m_SelectedOWRow, &pOW);

  BOOL bRemove = (pOW->DRIVERSrecordID != NO_RECORD);
  int  response;
//
//  Remove
//
  if(bRemove)
  {
    long DAILYOPSrecordID = NO_RECORD;
    long RUNSrecordID = pOW->RUNSrecordID;
    int  rcode2;

    CString s;

    if(!m_bMessageDisplay)
    {
      response = IDYES;
    }
    else
    {
      s.LoadString(ERROR_330);
      MessageBeep(MB_ICONQUESTION);
      response = MessageBox(s, TMS, MB_OK | MB_ICONQUESTION | MB_YESNO);
    }
    if(response == IDYES)
    {
//
//  Find the earlier assignment and update to show the clear
//
      DAILYOPSKey1.recordTypeFlag = DAILYOPS_FLAG_OPENWORK;
      DAILYOPSKey1.pertainsToDate = m_DailyOpsDateYYYYMMDD + 1;
      DAILYOPSKey1.pertainsToTime = NO_TIME;
      DAILYOPSKey1.recordFlags = 0;
      rcode2 = btrieve(B_GETLESSTHAN, TMS_DAILYOPS, &DAILYOPS, &DAILYOPSKey1, 1);
      while(rcode2 == 0 &&
            (DAILYOPS.recordTypeFlag & DAILYOPS_FLAG_OPENWORK) &&
            DAILYOPS.pertainsToDate == m_DailyOpsDateYYYYMMDD)
      {
        if((DAILYOPS.recordFlags & DAILYOPS_FLAG_OPENWORKASSIGN) &&
              DAILYOPS.DOPS.OpenWork.RUNSrecordID == RUNSrecordID)
        {
          if(pOW->cutAsRuntype == RUNSPLITRUNTYPE)
          {
            if(pOW->onTime == DAILYOPS.DOPS.OpenWork.splitStartTime &&
                  pOW->offTime == DAILYOPS.DOPS.OpenWork.splitEndTime)
            {
              DAILYOPSrecordID = DAILYOPS.recordID;
              break;
            }
          }
          else
          {
            DAILYOPSrecordID = DAILYOPS.recordID;
            break;
          }
        }
        rcode2 = btrieve(B_GETPREVIOUS, TMS_DAILYOPS, &DAILYOPS, &DAILYOPSKey1, 1);
      }
//
//  Record the clear in the audit trail
//  Database verify: 11-Jan-07
//
      DailyOpsBuildRecord(&DAILYOPS, DAILYOPS_FLAG_OPENWORK);
      DAILYOPS.pertainsToDate = m_DailyOpsDateYYYYMMDD;
      DAILYOPS.recordFlags = DAILYOPS_FLAG_OPENWORKCLEAR;
      DAILYOPS.DRIVERSrecordID = pOW->DRIVERSrecordID;
      DAILYOPS.DAILYOPSrecordID = DAILYOPSrecordID;
      DAILYOPS.DOPS.OpenWork.RUNSrecordID = RUNSrecordID;
      rcode2 = btrieve(B_INSERT, TMS_DAILYOPS, &DAILYOPS, &DAILYOPSKey0, 0);
      if(rcode2 == 0)
      {
        m_LastDAILYOPSRecordID = DAILYOPS.recordID;
      }
      DAILYOPSrecordID = DAILYOPS.recordID;
//
//  Clear the name on the display and disable the Clear button
//  
      pListCtrlOPENWORKLIST->SetItemText(m_SelectedOWRow, 8, "");
      pOW->DRIVERSrecordID = NO_RECORD;
      response = IDOK;  // fake out for later
    }
  }
//
//  Assign
//
  else
  {
    DailyOpsSOPassedDataDef SOPassedData;

    SOPassedData.pRD = m_RosteredDrivers;
    SOPassedData.numRD = m_numRosteredDrivers;
    SOPassedData.date = m_DailyOpsDateYYYYMMDD;
    SOPassedData.runNumber = pOW->runNumber;
    SOPassedData.runStartTime = pOW->onTime;
    SOPassedData.runEndTime = pOW->offTime;
    SOPassedData.payTime = pOW->pay;
    SOPassedData.flags = (pOW->cutAsRuntype == RUNSPLITRUNTYPE ? DAILYOPSSO_FLAGS_RUNSPLIT : 0);
    SOPassedData.pAbsentList = m_AbsentList;
    SOPassedData.numAbsent = m_numAbsent;

    if(m_bIncludeHours)
    {
      SOPassedData.flags |= DAILYOPSSO_FLAGS_SHOWHOURS;
    }
    SOPassedData.selectedOperator = NO_RECORD;

    CDailyOpsSO dlg(this, &SOPassedData);
//
//  Reset the Open Work Monitor Date
//
//    PopulateOpenWorkMonitor(NO_RECORD, TRUE);
//
//  Process the OK
//
    response = dlg.DoModal();
    if(response == IDOK)
    {
      m_SelectedOperator = SOPassedData.selectedOperator;

      long DRIVERSrecordID = m_SelectedOperator;
//
//  Build up the operator's name
//
      strcpy(tempString, BuildOperatorString(DRIVERSrecordID));
//
//  Assign it to the row
//  
      LVITEM  LVI;
      int     rcode2;
  
      LVI.mask = LVIF_TEXT;
      LVI.iItem = m_SelectedOWRow;
      LVI.iSubItem = 8;
      LVI.pszText = tempString;
      LVI.iItem = pListCtrlOPENWORKLIST->SetItem(&LVI);
//
//  Build up the entry for the audit trail
//
      BOOL bCrewOnly = pOW->flags & RDATA_FLAG_CREWONLY;
//
//  Database verify: 11-Jan-07
//
      DailyOpsBuildRecord(&DAILYOPS, DAILYOPS_FLAG_OPENWORK);
      DAILYOPS.pertainsToDate = m_DailyOpsDateYYYYMMDD;
      DAILYOPS.pertainsToTime = DAILYOPS.recordID;
      DAILYOPS.recordFlags = DAILYOPS_FLAG_OPENWORKASSIGN;
      DAILYOPS.DOPS.OpenWork.RUNSrecordID = pOW->RUNSrecordID;
      if(bCrewOnly)
      {
        DAILYOPS.recordFlags |= DAILYOPS_FLAG_CREWONLY;
      }
      DAILYOPS.DRIVERSrecordID = DRIVERSrecordID;
      if(pOW->cutAsRuntype == RUNSPLITRUNTYPE) // Run split
      {
        DAILYOPS.DOPS.OpenWork.splitStartNODESrecordID = pOW->onNODESrecordID;
        DAILYOPS.DOPS.OpenWork.splitStartTime = pOW->onTime;
        DAILYOPS.DOPS.OpenWork.splitEndNODESrecordID = pOW->offNODESrecordID;
        DAILYOPS.DOPS.OpenWork.splitEndTime = pOW->offTime;
        DAILYOPS.recordFlags |= (bCrewOnly ? DAILYOPS_FLAG_RUNSPLITCREWONLY : DAILYOPS_FLAG_RUNSPLIT);
      }
      rcode2 = btrieve(B_INSERT, TMS_DAILYOPS, &DAILYOPS, &DAILYOPSKey0, 0);
      if(rcode2 == 0)
      {
        m_LastDAILYOPSRecordID = DAILYOPS.recordID;
      }
      pOW->DRIVERSrecordID = DRIVERSrecordID;
    }
  }

  if(response == IDOK)
  {
    int nI;
    int rcode2;

    int indexToRoster[7] = {6, 0, 1, 2, 3, 4, 5};
    int today = indexToRoster[m_DailyOpsDate.GetDayOfWeek() - 1];

    for(nI = 0; nI < m_numBlocksInBlockInfo; nI++)
    {
      if(pOW->RUNSrecordID == m_BlockInfo[nI].startingRUNSrecordID)
      {
        if(bRemove)
        {
          m_BlockInfo[nI].startingDRIVERSrecordID = NO_RECORD;
        }
        else
        {
          m_BlockInfo[nI].startingDRIVERSrecordID = pOW->DRIVERSrecordID;
          ROSTERKey1.DIVISIONSrecordID = m_DailyOpsROSTERDivisionInEffect;
          ROSTERKey1.rosterNumber = NO_RECORD;
          rcode2 = btrieve(B_GETGREATER, TMS_ROSTER, &ROSTER, &ROSTERKey1, 1);
          while(rcode2 == 0 &&
                ROSTER.DIVISIONSrecordID == m_DailyOpsROSTERDivisionInEffect)
          {
            if(m_BlockInfo[nI].startingRUNSrecordID == ROSTER.WEEK[m_RosterWeek].RUNSrecordIDs[today])
            {
              m_BlockInfo[nI].startingRosterNumber = ROSTER.rosterNumber;
              m_BlockInfo[nI].startingROSTERrecordID = ROSTER.recordID;
              break;
            }
            rcode2 = btrieve(B_GETNEXT, TMS_ROSTER, &ROSTER, &ROSTERKey1, 1);
          }
        }
        RefreshBlockList(nI);
      }
    }
  }

  *pResult = 0;
}

void CDailyOps::OnExtratrip() 
{
  m_PreviousTime = m_SystemTime;
}

void CDailyOps::OnShowhours() 
{
  m_PreviousTime = m_SystemTime;

  OPENWORKDef *pOW;
  GetOWMPointer(m_SelectedOWRow, &pOW);

  DailyOpsSOPassedDataDef SOPassedData;

  SOPassedData.date = m_DailyOpsDateYYYYMMDD;
  SOPassedData.runStartTime = NO_TIME;
  SOPassedData.runEndTime = NO_TIME;
  SOPassedData.selectedOperator = NO_RECORD;

  SOPassedData.pRD = m_RosteredDrivers;
  SOPassedData.numRD = m_numRosteredDrivers;
  SOPassedData.date = m_DailyOpsDateYYYYMMDD;
  SOPassedData.runNumber = pOW->runNumber;
  SOPassedData.runStartTime = pOW->onTime;
  SOPassedData.runEndTime = pOW->offTime;
  SOPassedData.payTime = pOW->pay;
  SOPassedData.flags = (pOW->cutAsRuntype == RUNSPLITRUNTYPE ? DAILYOPSSO_FLAGS_RUNSPLIT : 0);
  SOPassedData.selectedOperator = NO_RECORD;
    SOPassedData.pAbsentList = m_AbsentList;
    SOPassedData.numAbsent = m_numAbsent;
  if(m_bIncludeHours)
  {
    SOPassedData.flags |= DAILYOPSSO_FLAGS_SHOWHOURS;
  }

  CDailyOpsSO dlg(this, &SOPassedData);

  dlg.DoModal();
}

void CDailyOps::OnConsider() 
{
  m_PreviousTime = m_SystemTime;
}

void CDailyOps::SetupDriverComboBox()
{
  char szDriver[DRIVERS_LASTNAME_LENGTH + 2 + DRIVERS_FIRSTNAME_LENGTH + 1];
  int  numDrivers;
  int  rcode2;

  pComboBoxOPERATOR->ResetContent();

  rcode2 = btrieve(B_GETFIRST, TMS_DRIVERS, &DRIVERS, &DRIVERSKey1, 1);
  while(rcode2 == 0)
  {
    if(ConsideringThisDriverType(DRIVERS.DRIVERTYPESrecordID))
    {
      strncpy(tempString, DRIVERS.lastName, DRIVERS_LASTNAME_LENGTH);
      trim(tempString, DRIVERS_LASTNAME_LENGTH);
      strcpy(szDriver, tempString);
      strcat(szDriver, ", ");
      strncpy(tempString, DRIVERS.firstName, DRIVERS_FIRSTNAME_LENGTH);
      trim(tempString, DRIVERS_FIRSTNAME_LENGTH);
      strcat(szDriver, tempString);
      numDrivers = pComboBoxOPERATOR->AddString(szDriver);
      if(numDrivers != CB_ERR)
      {
        pComboBoxOPERATOR->SetItemData(numDrivers, DRIVERS.recordID);
      }
    }
    rcode2 = btrieve(B_GETNEXT, TMS_DRIVERS, &DRIVERS, &DRIVERSKey1, 1);
  }
  pComboBoxOPERATOR->SetCurSel(-1);

}


void CDailyOps::PopulateVehicleAssignment()
{
//
//  Clear out the list
//
  pListCtrlBLOCKLIST->DeleteAllItems();
//
//  Initialize m_BlockInfo
//
  int nI, nJ;

  for(nI = 0; nI < MAXINBLOCKINFO; nI++)
  {
    m_BlockInfo[nI].POTime = NO_TIME;
    m_BlockInfo[nI].POGNODESrecordID = NO_RECORD;
    m_BlockInfo[nI].blockNumber = NO_RECORD;
    m_BlockInfo[nI].TRIPSrecordID = NO_RECORD;
    m_BlockInfo[nI].BUSTYPESrecordID = NO_RECORD;
    m_BlockInfo[nI].BUSESrecordID = NO_RECORD;
    m_BlockInfo[nI].RGRPROUTESrecordID = NO_RECORD;
    m_BlockInfo[nI].SGRPSERVICESrecordID = NO_RECORD;
    m_BlockInfo[nI].PITime = NO_TIME;
    m_BlockInfo[nI].PIGNODESrecordID = NO_RECORD;
    m_BlockInfo[nI].distance = 0.0;
    m_BlockInfo[nI].flags = 0;
    m_BlockInfo[nI].startingRunNumber = NO_RECORD;
    m_BlockInfo[nI].startingRUNSrecordID = NO_RECORD;
    m_BlockInfo[nI].startingRosterNumber = NO_RECORD;
    m_BlockInfo[nI].startingROSTERrecordID = NO_RECORD;
    m_BlockInfo[nI].startingDRIVERSrecordID = NO_RECORD;
    m_BlockInfo[nI].pszDriver = NULL;
  }

//
//  Figure out what day of the week we are
//
//  int   indexToRoster[7] = {6, 0, 1, 2, 3, 4, 5};
//  int   today = indexToRoster[m_DailyOpsDate.GetDayOfWeek() - 1];
//  long  SERVICESrecordID = ROSTERPARMS.serviceDays[today];

  CString s;

  if(m_SERVICESrecordIDInEffect == NO_RECORD)
  {
    s.LoadString(ERROR_318);  // No service today
    MessageBeep(MB_ICONINFORMATION);
    MessageBox(s, TMS, MB_ICONINFORMATION | MB_OK);
    return;
  }
//
//  This could take a while
//
  HCURSOR hSaveCursor = SetCursor(hCursorWait);
//
//  Go through the blocks
//
  GenerateTripDef      GTResults;
  GetConnectionTimeDef GCTData;
  BLOCKSDef *pTRIPSChunk;
  float cumDistance;
  float distance;
  BOOL bFirst;
  BOOL bFound;
  BOOL bFoundAll;
  long assignedToNODESrecordID;
  long previousBlock;
  long PITime;
  long PIGNode;
  long deadheadTime;
  long BUSESrecordID;
  long SERVICESrecordIDFromTRIPS = m_SERVICESrecordIDInEffect;
  long SERVICESrecordIDsFromTRIPS[10];
  int  keyNumber = 2;
  int  numBlocks = 0;
  int  rcode2;
  int  pegboardIndex;
  int  numSERVICESrecordIDsFromTRIPS = 0;

//
//  See if we're dealing with a "copied" runcut, and build
//  a list of unique services to cycle through with later
//
  RUNSKey1.DIVISIONSrecordID = m_DailyOpsRUNSDivisionInEffect;
  RUNSKey1.SERVICESrecordID = m_SERVICESrecordIDInEffect;
  RUNSKey1.runNumber = NO_RECORD;
  RUNSKey1.pieceNumber = NO_RECORD;
  rcode2 = btrieve(B_GETGREATER, TMS_RUNS, &RUNS, &RUNSKey1, 1);
  while(rcode2 == 0 && 
        RUNS.DIVISIONSrecordID == m_DailyOpsRUNSDivisionInEffect &&
        RUNS.SERVICESrecordID == m_SERVICESrecordIDInEffect)
  {
    TRIPSKey0.recordID = RUNS.start.TRIPSrecordID;
    rcode2 = btrieve(B_GETEQUAL, TMS_TRIPS, &TRIPS, &TRIPSKey0, 0);
    if(rcode2 == 0)
    {
      for(bFound = FALSE, nI = 0; nI < numSERVICESrecordIDsFromTRIPS; nI++)
      {
        if(TRIPS.SERVICESrecordID == SERVICESrecordIDsFromTRIPS[nI])
        {
          bFound = TRUE;
          break;
        }
      }
      if(!bFound)
      {
        SERVICESrecordIDsFromTRIPS[numSERVICESrecordIDsFromTRIPS] = TRIPS.SERVICESrecordID;
        numSERVICESrecordIDsFromTRIPS++;
      }
      SERVICESrecordIDFromTRIPS = TRIPS.SERVICESrecordID;
    }
    rcode2 = btrieve(B_GETNEXT, TMS_RUNS, &RUNS, &RUNSKey1, 1);
  }
//
//  Determine the path through the blocks
//
  pTRIPSChunk = keyNumber == 2 ? &TRIPS.standard : &TRIPS.dropback;
//
//  Cycle through the services
//
  numBlocks = 0;
  for(nI = 0; nI < numSERVICESrecordIDsFromTRIPS; nI++)
  {  
    NODESKey1.flags = NODES_FLAG_GARAGE;
    memset(NODESKey1.abbrName, 0x00, NODES_ABBRNAME_LENGTH);
    rcode2 = btrieve(B_GETGREATEROREQUAL, TMS_NODES, &NODES, &NODESKey1, 1);
//
//  Loop through the garages
//
    bFirst = TRUE;
    cumDistance = 0;
    while(rcode2 == 0 && NODES.flags & NODES_FLAG_GARAGE)
    {
      assignedToNODESrecordID = bFirst ? NO_RECORD : NODES.recordID;
      rcode2 = btrieve(B_GETFIRST, TMS_ROUTES, &ROUTES, &ROUTESKey1, 1);
      previousBlock = NO_RECORD;
      while(rcode2 == 0)
      {
        TRIPSKey2.assignedToNODESrecordID = assignedToNODESrecordID;
        TRIPSKey2.RGRPROUTESrecordID = ROUTES.recordID;
        TRIPSKey2.SGRPSERVICESrecordID = SERVICESrecordIDsFromTRIPS[nI];
        TRIPSKey2.blockNumber = NO_RECORD;
        TRIPSKey2.blockSequence = NO_TIME;
        rcode2 = btrieve(B_GETGREATER, TMS_TRIPS, &TRIPS, &TRIPSKey2, keyNumber);
        while(rcode2 == 0 &&
              pTRIPSChunk->assignedToNODESrecordID == assignedToNODESrecordID &&
              pTRIPSChunk->RGRPROUTESrecordID == ROUTES.recordID &&
              pTRIPSChunk->SGRPSERVICESrecordID == SERVICESrecordIDsFromTRIPS[nI])
        {
          GenerateTrip(TRIPS.ROUTESrecordID, TRIPS.SERVICESrecordID,
                TRIPS.directionIndex, TRIPS.PATTERNNAMESrecordID,
                TRIPS.timeAtMLP, GENERATETRIP_FLAG_DISPLAYERRORS, &GTResults);
//
//  PO Time and Location
//
          if(pTRIPSChunk->blockNumber != previousBlock)
          {
            m_BlockInfo[numBlocks].POTime = GTResults.firstNodeTime;
            if(pTRIPSChunk->POGNODESrecordID == NO_RECORD)
            {
              m_BlockInfo[numBlocks].POGNODESrecordID = GTResults.firstNODESrecordID;
            }
            else
            {
              GCTData.fromPATTERNNAMESrecordID = TRIPS.PATTERNNAMESrecordID;
              GCTData.toPATTERNNAMESrecordID = TRIPS.PATTERNNAMESrecordID;
              GCTData.fromROUTESrecordID = TRIPS.ROUTESrecordID;
              GCTData.fromSERVICESrecordID = TRIPS.SERVICESrecordID;
              GCTData.toROUTESrecordID = TRIPS.ROUTESrecordID;
              GCTData.toSERVICESrecordID = TRIPS.SERVICESrecordID;
              GCTData.fromNODESrecordID = pTRIPSChunk->POGNODESrecordID;
              GCTData.toNODESrecordID = GTResults.firstNODESrecordID;
              GCTData.timeOfDay = GTResults.firstNodeTime;
              deadheadTime = GetConnectionTime(GCT_FLAG_DEADHEADTIME, &GCTData, &distance);
              distance = (float)fabs((double)distance);
              if(deadheadTime != NO_TIME)
              {
                m_BlockInfo[numBlocks].POTime -= deadheadTime;
              }
              m_BlockInfo[numBlocks].POGNODESrecordID = pTRIPSChunk->POGNODESrecordID;
            }
// 
//  Starting run number
//
            m_BlockInfo[numBlocks].startingRunNumber = 0;
            RUNSKey1.DIVISIONSrecordID = m_DailyOpsRUNSDivisionInEffect;
            RUNSKey1.SERVICESrecordID = m_SERVICESrecordIDInEffect; //SERVICESrecordIDsFromTRIPS[nI];
            RUNSKey1.runNumber = NO_RECORD;
            RUNSKey1.pieceNumber = NO_RECORD;
            rcode2 = btrieve(B_GETGREATER, TMS_RUNS, &RUNS, &RUNSKey1, 1);
            while(rcode2 == 0 && 
                  RUNS.DIVISIONSrecordID == m_DailyOpsRUNSDivisionInEffect &&
                  RUNS.SERVICESrecordID == m_SERVICESrecordIDInEffect) //SERVICESrecordIDsFromTRIPS[nI])
            {
              if(RUNS.start.TRIPSrecordID == TRIPS.recordID &&
                    RUNS.start.NODESrecordID == m_BlockInfo[numBlocks].POGNODESrecordID)
              {
                m_BlockInfo[numBlocks].startingRunNumber = RUNS.runNumber;
                m_BlockInfo[numBlocks].startingRUNSrecordID = RUNS.recordID;
                break;
              }
              rcode2 = btrieve(B_GETNEXT, TMS_RUNS, &RUNS, &RUNSKey1, 1);
            }
//
//  Is this starting run part of open work?
//
            for(bFound = FALSE, nJ = 0; nJ < m_numInOpenWork; nJ++)
            {
//              if(m_BlockInfo[numBlocks].startingRUNSrecordID == m_OpenWork[nJ].RUNSrecordID)
              if(m_BlockInfo[numBlocks].startingRunNumber == m_OpenWork[nJ].runNumber)
              {
                m_BlockInfo[numBlocks].startingRosterNumber = m_OpenWork[nJ].rosterNumber;
                m_BlockInfo[numBlocks].startingROSTERrecordID = m_OpenWork[nJ].ROSTERrecordID;
                m_BlockInfo[numBlocks].startingDRIVERSrecordID = m_OpenWork[nJ].DRIVERSrecordID;
                bFound = TRUE;
                break;
              }
            }
//
//  If not found, look to the roster
//
            if(!bFound && m_BlockInfo[numBlocks].startingRUNSrecordID != NO_RECORD)
            {
              ROSTERKey1.DIVISIONSrecordID = m_DailyOpsROSTERDivisionInEffect;
              ROSTERKey1.rosterNumber = NO_RECORD;
              rcode2 = btrieve(B_GETGREATER, TMS_ROSTER, &ROSTER, &ROSTERKey1, 1);
              while(rcode2 == 0 &&
                    ROSTER.DIVISIONSrecordID == m_DailyOpsROSTERDivisionInEffect)
              {
                RUNSKey0.recordID = ROSTER.WEEK[m_RosterWeek].RUNSrecordIDs[m_Today];
                rcode2 = btrieve(B_GETEQUAL, TMS_RUNS, &RUNS, &RUNSKey0, 0);
                if(rcode2 == 0 && m_BlockInfo[numBlocks].startingRunNumber == RUNS.runNumber)
                {
                  m_BlockInfo[numBlocks].startingRosterNumber = ROSTER.rosterNumber;
                  m_BlockInfo[numBlocks].startingROSTERrecordID = ROSTER.recordID;
                  m_BlockInfo[numBlocks].startingDRIVERSrecordID = ROSTER.DRIVERSrecordID;
                  break;
                }
                rcode2 = btrieve(B_GETNEXT, TMS_ROSTER, &ROSTER, &ROSTERKey1, 1);
              }
            }
//
//  PI Time and Location
//
            if(previousBlock != NO_RECORD)
            {
              m_BlockInfo[numBlocks - 1].PITime = PITime;
              m_BlockInfo[numBlocks - 1].PIGNODESrecordID = PIGNode;
              m_BlockInfo[numBlocks - 1].distance = cumDistance;
              cumDistance = distance;
            }
            previousBlock = pTRIPSChunk->blockNumber;
            m_BlockInfo[numBlocks].blockNumber = pTRIPSChunk->blockNumber;
            m_BlockInfo[numBlocks].TRIPSrecordID = TRIPS.recordID;
            m_BlockInfo[numBlocks].RGRPROUTESrecordID = pTRIPSChunk->RGRPROUTESrecordID;
            m_BlockInfo[numBlocks].SGRPSERVICESrecordID = pTRIPSChunk->SGRPSERVICESrecordID;
            m_BlockInfo[numBlocks].BUSTYPESrecordID = TRIPS.BUSTYPESrecordID;
            numBlocks++;
          }
          cumDistance += GTResults.tripDistance;
          if(pTRIPSChunk->PIGNODESrecordID == NO_RECORD)
          {
            PITime = GTResults.lastNodeTime;
            PIGNode = GTResults.lastNODESrecordID;
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
            GCTData.toNODESrecordID = pTRIPSChunk->PIGNODESrecordID;
            GCTData.timeOfDay = GTResults.lastNodeTime;
            deadheadTime = GetConnectionTime(GCT_FLAG_DEADHEADTIME, &GCTData, &distance);
            distance = (float)fabs((double)distance);
            if(deadheadTime != NO_TIME)
            {
              PITime = GTResults.lastNodeTime + deadheadTime;
            }
            PIGNode = pTRIPSChunk->PIGNODESrecordID;
          }
          rcode2 = btrieve(B_GETNEXT, TMS_TRIPS, &TRIPS, &TRIPSKey2, keyNumber);
        }
//
//  ...and distance
//
        if(previousBlock != NO_RECORD)
        {
          m_BlockInfo[numBlocks - 1].PITime = PITime;
          m_BlockInfo[numBlocks - 1].PIGNODESrecordID = PIGNode;
          m_BlockInfo[numBlocks - 1].distance = cumDistance + distance;
          cumDistance = 0;
        }
        previousBlock = NO_RECORD;
        rcode2 = btrieve(B_GETNEXT, TMS_ROUTES, &ROUTES, &ROUTESKey1, 1);
      }
      if(bFirst)
      {
        bFirst = FALSE;
      }
      else
      {
        rcode2 = btrieve(B_GETNEXT, TMS_NODES, &NODES, &NODESKey1, 1);
      }
    }
  }  // nI through numSERVICESrecordIDsFromTRIPS
//
//  Establish the BLOCKINFO_FLAG_TOBEASSIGNED flag to start with
//
  for(nI = 0; nI < numBlocks; nI++)
  {
    m_BlockInfo[nI].flags = BLOCKINFO_FLAG_TOBEASSIGNED;
  }
//
//  Look for any dropped blocks for today
//
  DAILYOPSKey1.recordTypeFlag = DAILYOPS_FLAG_BUS;
  DAILYOPSKey1.pertainsToDate = m_DailyOpsDateYYYYMMDD;
  DAILYOPSKey1.pertainsToTime = NO_TIME;
  DAILYOPSKey1.recordFlags = 0;
  rcode2 = btrieve(B_GETGREATER, TMS_DAILYOPS, &DAILYOPS, &DAILYOPSKey1, 1);
  while(rcode2 == 0 &&
        (DAILYOPS.recordTypeFlag & DAILYOPS_FLAG_BUS) &&
        (DAILYOPS.pertainsToDate == m_DailyOpsDateYYYYMMDD))
  {
    if(DAILYOPS.recordFlags & DAILYOPS_FLAG_BLOCKDROP)
    {
//
//  Undropped?
//
      if(!ANegatedRecord(DAILYOPS.recordID, 1))
      {
        for(nI = 0; nI < numBlocks; nI++)
        {
          DAILYOPS.DOPS.Bus.TRIPSrecordID = m_BlockInfo[nI].TRIPSrecordID;
          DAILYOPS.DOPS.Bus.RGRPROUTESrecordID = m_BlockInfo[nI].RGRPROUTESrecordID;
          DAILYOPS.DOPS.Bus.SGRPSERVICESrecordID = m_BlockInfo[nI].SGRPSERVICESrecordID;
          m_BlockInfo[nI].flags = BLOCKINFO_FLAG_BLOCKDROP;
          break;
        }
      }
    }
//
//  Get the next record
//
    rcode2 = btrieve(B_GETNEXT, TMS_DAILYOPS, &DAILYOPS, &DAILYOPSKey1, 1);
  }  // while
//
//  Sort m_BlockInfo on POTime
//
  qsort((void *)m_BlockInfo, numBlocks, sizeof(BlockInfoDef), sort_BlockInfo);
//
//  Buses
//
//  Assume everything's in service
//
  for(nI = 0; nI < m_numBuses; nI++)
  {
    m_pPEGBOARD[nI].flags = PEGBOARD_FLAG_BUSISINSERVICE;
  }
//
//  Get the locations of the buses
//
  for(nI = 0; nI < m_numBuses; nI++)
  {
    DAILYOPSKey1.recordTypeFlag = DAILYOPS_FLAG_BUS;
    DAILYOPSKey1.pertainsToDate = m_DailyOpsDateYYYYMMDD;
    DAILYOPSKey1.pertainsToTime = m_LastDAILYOPSRecordID + 1;
    DAILYOPSKey1.recordFlags = 0;
    rcode2 = btrieve(B_GETLESSTHAN, TMS_DAILYOPS, &DAILYOPS, &DAILYOPSKey1, 1);
    while(rcode2 == 0 &&
          (DAILYOPS.recordTypeFlag & DAILYOPS_FLAG_BUS))
    {
      if(DAILYOPS.DOPS.Bus.BUSESrecordID == m_pPEGBOARD[nI].BUSESrecordID)
      {
        if(DAILYOPS.recordFlags & DAILYOPS_FLAG_SETBUSLOCATION)
        {
          m_pPEGBOARD[nI].locationNODESrecordID = DAILYOPS.DOPS.Bus.locationNODESrecordID;
          break;
        }
      }
      rcode2 = btrieve(B_GETPREVIOUS, TMS_DAILYOPS, &DAILYOPS, &DAILYOPSKey1, 1);
    }
  }
//
//  Database verify: 06-Feb-07
// 
//  What's the last thing that happened to this bus?
//
  BOOL bDidSomething;

  DAILYOPSKey1.recordTypeFlag = DAILYOPS_FLAG_BUS;
  DAILYOPSKey1.pertainsToDate = m_DailyOpsDateYYYYMMDD;
  DAILYOPSKey1.pertainsToTime = m_LastDAILYOPSRecordID + 1;
  DAILYOPSKey1.recordFlags = 0;
  BUSESrecordID = NO_RECORD;
  rcode2 = btrieve(B_GETLESSTHAN, TMS_DAILYOPS, &DAILYOPS, &DAILYOPSKey1, 1);
  while(rcode2 == 0 &&
        (DAILYOPS.recordTypeFlag & DAILYOPS_FLAG_BUS))
  {
//
//  Have we hit the shut-up point?
//
    for(bFoundAll = TRUE, nI = 0; nI < m_numBuses; nI++)
    {
      if(m_pPEGBOARD[nI].flags & PEGBOARD_FLAG_FOUNDINAUDITTRAIL)
      {
        continue;
      }
      bFoundAll = FALSE;
      break;
    }
    if(bFoundAll)
    {
      break;
    }
//
//  Have we processed this bus yet?  If not - get the index into the pegboard
//
    for(bFound = FALSE, nI = 0; nI < m_numBuses; nI++)
    {
      if(m_pPEGBOARD[nI].BUSESrecordID == DAILYOPS.DOPS.Bus.BUSESrecordID)
      {
        if(m_pPEGBOARD[nI].flags & PEGBOARD_FLAG_FOUNDINAUDITTRAIL)
        {
          bFound = TRUE;
        }
        else
        {
          pegboardIndex = nI;
        }
        break;
      }
    }
//
//  No - process it
//
    if(!bFound)
    {
      bDidSomething = TRUE;
      while(DAILYOPS.recordFlags > 0 && bDidSomething)
      {
        bDidSomething = FALSE;
//
//  Look for an assignment
//
        if(DAILYOPS.recordFlags & DAILYOPS_FLAG_BUSASSIGNMENT)
        {
          DAILYOPS.recordFlags -= DAILYOPS_FLAG_BUSASSIGNMENT;
          bDidSomething = TRUE;
          if(DAILYOPS.pertainsToDate == m_DailyOpsDateYYYYMMDD)
          {
//
//  Go through the blocks
//
            for(nJ = 0; nJ < numBlocks; nJ++)
            {
              if(DAILYOPS.DOPS.Bus.TRIPSrecordID == m_BlockInfo[nJ].TRIPSrecordID)
              {
                m_BlockInfo[nJ].flags = BLOCKINFO_FLAG_ASSIGNED;
                m_BlockInfo[nJ].BUSESrecordID = DAILYOPS.DOPS.Bus.BUSESrecordID;
                m_BlockInfo[nJ].TRIPSrecordID = DAILYOPS.DOPS.Bus.TRIPSrecordID;
                m_BlockInfo[nJ].RGRPROUTESrecordID = DAILYOPS.DOPS.Bus.RGRPROUTESrecordID;
                m_BlockInfo[nJ].SGRPSERVICESrecordID = DAILYOPS.DOPS.Bus.SGRPSERVICESrecordID;
                m_pPEGBOARD[pegboardIndex].flags = PEGBOARD_FLAG_BUSISASSIGNED;
                m_pPEGBOARD[pegboardIndex].DAILYOPSrecordID = DAILYOPS.recordID;
                if(m_pPEGBOARD[pegboardIndex].locationNODESrecordID == NO_RECORD)
                {
                  if(DAILYOPS.DOPS.Bus.locationNODESrecordID > 0)
                  {
                    m_pPEGBOARD[pegboardIndex].locationNODESrecordID = DAILYOPS.DOPS.Bus.locationNODESrecordID;
                  }
                }
                m_pPEGBOARD[pegboardIndex].indexToBlockInfo = nJ;
                m_pPEGBOARD[pegboardIndex].flags |= PEGBOARD_FLAG_FOUNDINAUDITTRAIL;
                break;
              }
            }
          }
        }
//
//  Look for a return
//
        else if(DAILYOPS.recordFlags & DAILYOPS_FLAG_BUSRETURN)
        {
          DAILYOPS.recordFlags -= DAILYOPS_FLAG_BUSRETURN;
          bDidSomething = TRUE;
          if(DAILYOPS.pertainsToDate == m_DailyOpsDateYYYYMMDD)
          {
//
//  Go through the blocks
//
            for(nJ = 0; nJ < numBlocks; nJ++)
            {
              if(DAILYOPS.DOPS.Bus.TRIPSrecordID == m_BlockInfo[nJ].TRIPSrecordID)
              {
                m_BlockInfo[nJ].flags = BLOCKINFO_FLAG_BUSRETURN;
//                m_pPEGBOARD[pegboardIndex].locationNODESrecordID = m_BlockInfo[nJ].PIGNODESrecordID;
                m_pPEGBOARD[pegboardIndex].flags &= ~PEGBOARD_FLAG_BUSISASSIGNED;
                m_pPEGBOARD[pegboardIndex].DAILYOPSrecordID = NO_RECORD;
                if(DAILYOPS.DOPS.Bus.locationNODESrecordID > 0)
                {
                  m_pPEGBOARD[pegboardIndex].locationNODESrecordID = DAILYOPS.DOPS.Bus.locationNODESrecordID;
                }
                m_pPEGBOARD[pegboardIndex].flags |= PEGBOARD_FLAG_FOUNDINAUDITTRAIL;
                break;
              }
            }
          }
        }
//
//  Was the bus marked out of service?
//
        if(DAILYOPS.recordFlags & DAILYOPS_FLAG_BUSMARKEDOUTOFSERVICE)
        {
          DAILYOPS.recordFlags -= DAILYOPS_FLAG_BUSMARKEDOUTOFSERVICE;
          bDidSomething = TRUE;
          if(!(m_pPEGBOARD[pegboardIndex].flags & PEGBOARD_FLAG_BUSWASPUTINTOSERVICE))
          {
            m_pPEGBOARD[pegboardIndex].flags = PEGBOARD_FLAG_BUSMARKEDOUTOFSERVICE;
            m_pPEGBOARD[pegboardIndex].swapReasonIndex = DAILYOPS.DOPS.Bus.swapReasonIndex;
            m_pPEGBOARD[pegboardIndex].flags |= PEGBOARD_FLAG_FOUNDINAUDITTRAIL;
          }
        }
//
//  Or moved back into service?
//
        else if(DAILYOPS.recordFlags & DAILYOPS_FLAG_BUSISINSERVICE)
        {
          m_pPEGBOARD[pegboardIndex].flags |= PEGBOARD_FLAG_BUSWASPUTINTOSERVICE;
          m_pPEGBOARD[pegboardIndex].flags |= PEGBOARD_FLAG_FOUNDINAUDITTRAIL;
          DAILYOPS.recordFlags -= DAILYOPS_FLAG_BUSISINSERVICE;
          bDidSomething = TRUE;
        }
//
//  Was the bus marked as a short shift?
//
        if(DAILYOPS.recordFlags & DAILYOPS_FLAG_BUSMARKEDSHORTSHIFT)
        {
          m_pPEGBOARD[pegboardIndex].flags |= PEGBOARD_FLAG_BUSMARKEDASSHORTSHIFT;
          m_pPEGBOARD[pegboardIndex].untilTime = DAILYOPS.DOPS.Bus.untilTime;
          m_pPEGBOARD[pegboardIndex].flags |= PEGBOARD_FLAG_FOUNDINAUDITTRAIL;
          DAILYOPS.recordFlags -= DAILYOPS_FLAG_BUSMARKEDSHORTSHIFT;
          bDidSomething = TRUE;
        }
//
//  Or unmarked?
//
        else if(DAILYOPS.recordFlags & DAILYOPS_FLAG_BUSUNMARKEDSHORTSHIFT)
        {
          m_pPEGBOARD[pegboardIndex].flags &= ~PEGBOARD_FLAG_BUSMARKEDASSHORTSHIFT;
          m_pPEGBOARD[pegboardIndex].untilTime = NO_TIME;
          m_pPEGBOARD[pegboardIndex].flags |= PEGBOARD_FLAG_FOUNDINAUDITTRAIL;
          DAILYOPS.recordFlags -= DAILYOPS_FLAG_BUSUNMARKEDSHORTSHIFT;
          bDidSomething = TRUE;
        }
//
//  Was the bus marked as a charter?
//
        if(DAILYOPS.recordFlags & DAILYOPS_FLAG_BUSMARKEDASCHARTER)
        {
          m_pPEGBOARD[pegboardIndex].flags |= PEGBOARD_FLAG_BUSMARKEDASCHARTER;
          m_pPEGBOARD[pegboardIndex].flags |= PEGBOARD_FLAG_FOUNDINAUDITTRAIL;
          DAILYOPS.recordFlags -= DAILYOPS_FLAG_BUSMARKEDASCHARTER;
          bDidSomething = TRUE;
        }
//
//  Or unmarked?
//
        else if(DAILYOPS.recordFlags & DAILYOPS_FLAG_BUSUNMARKEDASCHARTER)
        {
          m_pPEGBOARD[pegboardIndex].flags &= ~PEGBOARD_FLAG_BUSMARKEDASCHARTER;
          m_pPEGBOARD[pegboardIndex].flags |= PEGBOARD_FLAG_FOUNDINAUDITTRAIL;
          DAILYOPS.recordFlags -= DAILYOPS_FLAG_BUSUNMARKEDASCHARTER;
          bDidSomething = TRUE;
        }
//
//  Was the bus marked as "sight-seeing"?
//
        if(DAILYOPS.recordFlags & DAILYOPS_FLAG_BUSMARKEDASSIGHTSEEING)
        {
          m_pPEGBOARD[pegboardIndex].flags |= PEGBOARD_FLAG_BUSMARKEDASSIGHTSEEING;
          m_pPEGBOARD[pegboardIndex].flags |= PEGBOARD_FLAG_FOUNDINAUDITTRAIL;
          DAILYOPS.recordFlags -= DAILYOPS_FLAG_BUSMARKEDASSIGHTSEEING;
          bDidSomething = TRUE;
        }
//
//  Or unmarked?
//
        else if(DAILYOPS.recordFlags & DAILYOPS_FLAG_BUSUNMARKEDASSIGHTSEEING)
        {
          m_pPEGBOARD[pegboardIndex].flags &= ~PEGBOARD_FLAG_BUSMARKEDASSIGHTSEEING;
          m_pPEGBOARD[pegboardIndex].flags |= PEGBOARD_FLAG_FOUNDINAUDITTRAIL;
          DAILYOPS.recordFlags -= DAILYOPS_FLAG_BUSUNMARKEDASSIGHTSEEING;
          bDidSomething = TRUE;
        }
//
//  Get the stuff we don't care about here out of the way
//
//  Swap
//
        if(DAILYOPS.recordFlags & DAILYOPS_FLAG_BUSSWAP)
        {
          DAILYOPS.recordFlags -= DAILYOPS_FLAG_BUSSWAP;
          bDidSomething = TRUE;
        }
//
//  Set location
//
        if(DAILYOPS.recordFlags & DAILYOPS_FLAG_SETBUSLOCATION)
        {
          DAILYOPS.recordFlags -= DAILYOPS_FLAG_SETBUSLOCATION;
          m_pPEGBOARD[pegboardIndex].locationNODESrecordID = DAILYOPS.DOPS.Bus.locationNODESrecordID;
          bDidSomething = TRUE;
        }
//
//  Marked into service
//
        if(DAILYOPS.recordFlags & DAILYOPS_FLAG_BUSMARKEDINSERVICE)
        {
          DAILYOPS.recordFlags -= DAILYOPS_FLAG_BUSMARKEDINSERVICE;
          bDidSomething = TRUE;
        }
//
//  Bus clear all
//
        if(DAILYOPS.recordFlags & DAILYOPS_FLAG_BUSCLEARALL)
        {
          DAILYOPS.recordFlags -= DAILYOPS_FLAG_BUSCLEARALL;
          bDidSomething = TRUE;
        }
//
//  Block drop
//
        if(DAILYOPS.recordFlags & DAILYOPS_FLAG_BLOCKDROP)
        {
          DAILYOPS.recordFlags -= DAILYOPS_FLAG_BLOCKDROP;
          bDidSomething = TRUE;
        }
//
//  Block undrop
//
        if(DAILYOPS.recordFlags & DAILYOPS_FLAG_BLOCKUNDROP)
        {
          DAILYOPS.recordFlags -= DAILYOPS_FLAG_BLOCKUNDROP;
          bDidSomething = TRUE;
        }
      }  // while > 0
    }
//
//  Get the previous record
//
    rcode2 = btrieve(B_GETPREVIOUS, TMS_DAILYOPS, &DAILYOPS, &DAILYOPSKey1, 1);
  }  // while
//
//  Put the blocks in the Block list control
//
  m_numBlocksInBlockInfo = numBlocks;
  RefreshBlockList(NO_RECORD);
//
//  Restore the cursor
//
  InvalidateRect(&m_PegboardRect, TRUE);
  SetCursor(hSaveCursor);
}

void CDailyOps::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	
//
//  Set up the font
//
  CFont   Font;
  LOGFONT lf;

  memset(&lf, 0x00, sizeof(LOGFONT));
  lf.lfHeight = -MulDiv(7, dc.GetDeviceCaps(LOGPIXELSY), 72);
  lf.lfWeight = FW_THIN;
  lf.lfCharSet = ANSI_CHARSET;
  lf.lfPitchAndFamily = DEFAULT_PITCH | FF_SWISS;
  strcpy(lf.lfFaceName, "Calibri");
  Font.CreateFontIndirect(&lf);
//
//  Get the average character width
//
  TEXTMETRIC tm;

  dc.GetTextMetrics(&tm);
  if(tm.tmAveCharWidth <= 0)
  {
    tm.tmAveCharWidth = 4;
  }
//
//  Set up the rest of the device context's properties
//
  dc.SelectObject(Font);
  dc.SetBkMode(TRANSPARENT);
  dc.SetTextColor(RGB(0, 0, 0));
//
//  Set up the clipping region
//
  CRgn rgn;
  
  rgn.CreateRectRgn(m_PegboardRect.left, m_PegboardRect.top, m_PegboardRect.right, m_PegboardRect.bottom);
  dc.SelectClipRgn(&rgn);
//
//  Display the boxes
//
  RECT r;
  int  rcode2;
  int  nI, nJ;
  int  row, col;
  int  start = m_FirstBusRowDisplayed * 10;

  
  for(nJ = 0, nI = start; nI < m_numBuses; nI++)
  {
    if(nJ >= 50)
    {
      break;
    }
    if(m_bPegboardShowFlags == PEGBOARD_FLAG_SHOWALL ||
          ((m_bPegboardShowFlags & PEGBOARD_FLAG_SHOWASSIGNED) && (m_pPEGBOARD[nI].flags & PEGBOARD_FLAG_BUSISASSIGNED)) ||
          ((m_bPegboardShowFlags & PEGBOARD_FLAG_SHOWAVAILABLE) && (m_pPEGBOARD[nI].flags & PEGBOARD_FLAG_BUSISINSERVICE)) ||
          ((m_bPegboardShowFlags & PEGBOARD_FLAG_SHOWOUTOFSERVICE) && (m_pPEGBOARD[nI].flags & PEGBOARD_FLAG_BUSMARKEDOUTOFSERVICE)))
    {
      m_LastBusIndex = nI;

      row = nJ / 10;
      col = nJ % 10;

      r.top = m_PegboardRect.top + (row * 60) + row;
      r.left = m_PegboardRect.left + (col * 60) + col;
      r.bottom = r.top + 60;
      r.right = r.left + 60;
//
//  The box
//
      m_pPEGBOARD[nI].rect.top = r.top;
      m_pPEGBOARD[nI].rect.left = r.left;
      m_pPEGBOARD[nI].rect.bottom = r.bottom;
      m_pPEGBOARD[nI].rect.right = r.right;
//
//  The pen and brush
//
      dc.SelectObject(m_Pens[DO_TXT]);
      if(m_pPEGBOARD[nI].flags & PEGBOARD_FLAG_BUSISASSIGNED)
      {
        dc.SelectObject(m_Brushes[DO_INUSE]);
      }
      else if(m_pPEGBOARD[nI].flags & PEGBOARD_FLAG_BUSISINSERVICE)
      {
        if(m_pPEGBOARD[nI].flags & PEGBOARD_FLAG_BUSMARKEDASSHORTSHIFT)
        {
          dc.SelectObject(m_Brushes[DO_SHORTSHIFT]);
        }
        else if(m_pPEGBOARD[nI].flags & PEGBOARD_FLAG_BUSMARKEDASCHARTER)
        {
          dc.SelectObject(m_Brushes[DO_CHARTERRESERVE]);
        }
        else if(m_pPEGBOARD[nI].flags & PEGBOARD_FLAG_BUSMARKEDASSIGHTSEEING)
        {
          dc.SelectObject(m_Brushes[DO_SIGHTSEEINGRESERVE]);
        }
        else
        {
          dc.SelectObject(m_Brushes[DO_AVAILABLE]);
        }
      }
      else
      {
        dc.SelectObject(m_Brushes[DO_OUTOFSERVICE]);
      }
//
//  The box
//
      if(m_pPEGBOARD[nI].flags & PEGBOARD_FLAG_HIGHLIGHT)
      {
        dc.SelectObject(m_Pens[DO_BORDER]);
      }
      dc.Rectangle(&r);
//
//  Bus number and location
//
      strcpy(tempString, m_pPEGBOARD[nI].szBusNumber);
      if(m_pPEGBOARD[nI].locationNODESrecordID != NO_RECORD)
      {
        strcat(tempString, (m_pPEGBOARD[nI].flags & PEGBOARD_FLAG_BUSISASSIGNED) ? " o/o " : " @ ");
        NODESKey0.recordID = m_pPEGBOARD[nI].locationNODESrecordID;
        rcode2 = btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
        if(rcode2 == 0)
        {
          strncpy(szarString, NODES.abbrName, NODES_ABBRNAME_LENGTH);
          trim(szarString, NODES_ABBRNAME_LENGTH);
        }
        else
        {
          strcpy(szarString, "?");
        }
        strcat(tempString, szarString);
        strcpy(m_pPEGBOARD[nI].szLocationNodeName, szarString);
      }
      dc.SelectObject(m_Pens[DO_THICKTXT]);
      dc.TextOut(r.left + 2, r.top, tempString, strlen(tempString));
//
//  Block, run, roster
//
      if(m_pPEGBOARD[nI].flags & PEGBOARD_FLAG_BUSISASSIGNED)
      {
        sprintf(tempString, " Block: %ld",  m_BlockInfo[m_pPEGBOARD[nI].indexToBlockInfo].blockNumber);
        dc.TextOut(r.left + 2, r.top + 10,  tempString, strlen(tempString));

        if(m_BlockInfo[m_pPEGBOARD[nI].indexToBlockInfo].startingRunNumber <= 0)
        {
          strcpy(tempString, " Run: - - -");
        }
        else
        {
          sprintf(tempString, " Run: %ld",    m_BlockInfo[m_pPEGBOARD[nI].indexToBlockInfo].startingRunNumber);
        }
        dc.TextOut(r.left + 2, r.top + 20,  tempString, strlen(tempString));

        if(m_BlockInfo[m_pPEGBOARD[nI].indexToBlockInfo].startingRosterNumber <= 0)
        {
          strcpy(tempString, " Roster: - - -");
        }
        else
        {
          sprintf(tempString, " Roster: %ld", m_BlockInfo[m_pPEGBOARD[nI].indexToBlockInfo].startingRosterNumber);
        }
        dc.TextOut(r.left + 2, r.top + 30,  tempString, strlen(tempString));
        sprintf(tempString, " P/I: %s",   Tchar(m_BlockInfo[m_pPEGBOARD[nI].indexToBlockInfo].PITime));
        dc.TextOut(r.left + 2, r.top + 40,  tempString, strlen(tempString));
      }
//
//  Out of service reason
//
      else if(m_pPEGBOARD[nI].flags & PEGBOARD_FLAG_BUSMARKEDOUTOFSERVICE)
      {
        if(m_pPEGBOARD[nI].swapReasonIndex != NO_RECORD)
        {
          char *ptr;
          int pos = 20;
        
          strcpy(tempString, m_BusSwapReasons[m_pPEGBOARD[nI].swapReasonIndex].szText);
          ptr = strtok(tempString, " ");
          while(ptr)
          {
            dc.TextOut(r.left + 2, r.top + pos, ptr, strlen(ptr));
            ptr = strtok(NULL, " ");
            pos += 10;
          }
        }
      }
//
//  Short shift until time
//
      else if(m_pPEGBOARD[nI].flags & PEGBOARD_FLAG_BUSMARKEDASSHORTSHIFT)
      {
        if(m_pPEGBOARD[nI].untilTime != NO_TIME)
        {
          sprintf(tempString, " Until: %s",  Tchar(m_pPEGBOARD[nI].untilTime));
          dc.TextOut(r.left + 2, r.top + 20,  tempString, strlen(tempString));
        }
      }
//
//  Charter text
//
      else if(m_pPEGBOARD[nI].flags & PEGBOARD_FLAG_BUSMARKEDASCHARTER)
      {
        strcpy(tempString, " Reserved for");
        dc.TextOut(r.left + 2, r.top + 20,  tempString, strlen(tempString));
        strcpy(tempString, " Charter");
        dc.TextOut(r.left + 2, r.top + 30,  tempString, strlen(tempString));
      }
//
//  Short shift until time
//
      else if(m_pPEGBOARD[nI].flags & PEGBOARD_FLAG_BUSMARKEDASSIGHTSEEING)
      {
        strcpy(tempString, " Reserved for");
        dc.TextOut(r.left + 2, r.top + 20,  tempString, strlen(tempString));
        strcpy(tempString, " Sight-seeing");
        dc.TextOut(r.left + 2, r.top + 30,  tempString, strlen(tempString));
      }
//
//  Add one to the number of buses displayed and loop back
//
      nJ++;
    }  // displayflag
  }
}

void CDailyOps::EstablishViewingArea()
{
  RECT r;
  RECT tempRect;

  if(pFlatSBSCROLLBAR == NULL || !IsWindow(pFlatSBSCROLLBAR->m_hWnd))
  {
    return;
  }

  if(pListCtrlBLOCKLIST == NULL || !IsWindow(pListCtrlBLOCKLIST->m_hWnd))
  {
    return;
  }

  GetWindowRect(&m_rcDialog);
  pListCtrlBLOCKLIST->GetWindowRect(&tempRect);

  r.left = tempRect.left;

  pFlatSBSCROLLBAR->GetWindowRect(&tempRect);
  r.top = tempRect.top - m_PegboardMagicNumber; // Magic number for display offset
  r.right = tempRect.left;
  r.bottom = tempRect.bottom;
//
//  Set up the window extents
//
  m_PegboardRect = r;
  m_rcRepaint = m_PegboardRect;
  m_rcRepaint.top = 0;
  m_rcRepaint.left = 0;
  m_rcRepaint.bottom += 4;
}


void CDailyOps::OnOK() 
{
  Cleanup();

	CDialog::OnOK();
}

#define PBMENUPOS_ASSIGNBUS        0
#define PBMENUPOS_RETURN           1 
#define PBMENUPOS_SWAP             2 
#define PBMENUPOS_SEPARATOR1       3 
#define PBMENUPOS_PUTBACKINSERVICE 4
#define PBMENUPOS_POPUPSHORTSHIFT  5
#define PBMENUPOS_SETLOCATION      6
#define PBMENUPOS_SEPARATOR2       7 
#define PBMENUPOS_CLEARALL         8
#define PBMENUPOS_SEPARATOR3       9 
#define PBMENUPOS_POPUPSHOW       10
#define PBMENUPOS_POPUPSORTBY     11
#define PBMENUPOS_SEPARATOR4      12 
#define PBMENUPOS_PROPERTIES      13

void CDailyOps::OnContextMenu(CWnd* pWnd, CPoint point) 
{
  m_PreviousTime = m_SystemTime;

  if(m_menu != NULL)
  {
    return;
  }
//
//  point comes in in screen coords.  Translate it
//  to dialog coords, which is what it should come
//  in in the first place.  Jeez.
//
  pWnd->ScreenToClient(&point);
//
//  If it's outside the drawing region, don't bother
//
  m_PegboardIndex = NO_RECORD;

  if(point.x < m_PegboardRect.left || point.x > m_PegboardRect.right ||
        point.y < m_PegboardRect.top || point.y > m_PegboardRect.bottom)
  {
    return;
  }
//
//  Unhighlight and highlighted square
//
  int  nI;
  int  start = m_FirstBusRowDisplayed * 10;

  for(nI = start; nI < m_numBuses; nI++)
  {
    if(nI > m_LastBusIndex)
    {
      break;
    }
    if(m_pPEGBOARD[nI].flags & PEGBOARD_FLAG_HIGHLIGHT)
    {
      m_pPEGBOARD[nI].flags &= ~PEGBOARD_FLAG_HIGHLIGHT;
      InvalidateRect(&m_pPEGBOARD[nI].rect, TRUE);
    }
  }
//
//  Locate the highlighted square
//
  BOOL bFoundHighlightedSquare;

  for(bFoundHighlightedSquare = FALSE, nI = start; nI < m_numBuses; nI++)
  {
    if(nI > m_LastBusIndex)
    {
      break;
    }
    if(m_bPegboardShowFlags == PEGBOARD_FLAG_SHOWALL ||
          ((m_bPegboardShowFlags & PEGBOARD_FLAG_SHOWASSIGNED) && (m_pPEGBOARD[nI].flags & PEGBOARD_FLAG_BUSISASSIGNED)) ||
          ((m_bPegboardShowFlags & PEGBOARD_FLAG_SHOWAVAILABLE) && (m_pPEGBOARD[nI].flags & PEGBOARD_FLAG_BUSISINSERVICE)) ||
          ((m_bPegboardShowFlags & PEGBOARD_FLAG_SHOWOUTOFSERVICE) && (m_pPEGBOARD[nI].flags & PEGBOARD_FLAG_BUSMARKEDOUTOFSERVICE)))
    {
      if(point.x >= m_pPEGBOARD[nI].rect.left && point.x <= m_pPEGBOARD[nI].rect.right &&
            point.y >= m_pPEGBOARD[nI].rect.top && point.y <= m_pPEGBOARD[nI].rect.bottom)
      {
        m_pPEGBOARD[nI].flags |= PEGBOARD_FLAG_HIGHLIGHT;
        m_PegboardIndex = nI;
        InvalidateRect(&m_pPEGBOARD[nI].rect, TRUE);
        bFoundHighlightedSquare = TRUE;
        break;
      }
    }
  }
//
//  Deal with the menu
//
  m_menu.LoadMenu(IDR_PEGBOARD_CONTEXT);

  CMenu* pMenu = m_menu.GetSubMenu(0);

  if(!pMenu)
  {
    return;
  }

  LVITEM LVI;
//
//  See if a block is highlighted in the blocks list control
//
  nI = pListCtrlBLOCKLIST->GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);
  if(nI < 0)
  {
    m_SelectedBlockIndex = NO_RECORD;
    m_SelectedBlockRow = NO_RECORD;
  }
  else
  {
    LVI.mask = LVIF_PARAM;
    LVI.iItem = nI;
    LVI.iSubItem = 0;
    pListCtrlBLOCKLIST->GetItem(&LVI);
    m_SelectedBlockIndex = LVI.lParam;
    m_SelectedBlockRow = nI;
  }
//
//  Disable stuff if nothing's highlighted in the blocks list control
//
  if(!bFoundHighlightedSquare)
  {
    pMenu->EnableMenuItem(PEGBOARD_SETLOCATION, MF_BYCOMMAND | MF_GRAYED);
    pMenu->EnableMenuItem(PEGBOARD_CLEARALL, MF_BYCOMMAND | MF_GRAYED);
    pMenu->EnableMenuItem(PEGBOARD_PROPERTIES, MF_BYCOMMAND | MF_GRAYED);
  }
//
//  Either disable the "Assign" menu item, or show it with the highlighted bus
//
  if(!bFoundHighlightedSquare || m_SelectedBlockRow == NO_RECORD ||
        (m_pPEGBOARD[m_PegboardIndex].flags & PEGBOARD_FLAG_BUSMARKEDOUTOFSERVICE))
  {
    pMenu->ModifyMenu(PEGBOARD_ASSIGNBUS, MF_BYCOMMAND, PEGBOARD_ASSIGNBUS, "Assign");
    pMenu->EnableMenuItem(PEGBOARD_ASSIGNBUS, MF_BYCOMMAND | MF_GRAYED);
  }
  else
  {
    sprintf(tempString, "Assign block %ld to bus %s",
          m_BlockInfo[m_SelectedBlockIndex].blockNumber, m_pPEGBOARD[m_PegboardIndex].szBusNumber);
    pMenu->ModifyMenu(PEGBOARD_ASSIGNBUS, MF_BYCOMMAND, PEGBOARD_ASSIGNBUS, tempString);
    pMenu->EnableMenuItem(PEGBOARD_ASSIGNBUS, MF_BYCOMMAND | MF_ENABLED);
  }
//
//  Either disable "Return" or show the bus number to return
//
  if(bFoundHighlightedSquare && (m_pPEGBOARD[m_PegboardIndex].flags & PEGBOARD_FLAG_BUSISASSIGNED))
  {
    sprintf(tempString, "Return bus %s", m_pPEGBOARD[m_PegboardIndex].szBusNumber);
    pMenu->ModifyMenu(PEGBOARD_RETURN, MF_BYCOMMAND, PEGBOARD_RETURN, tempString);
    pMenu->EnableMenuItem(PEGBOARD_RETURN, MF_BYCOMMAND | MF_ENABLED);
  }
  else
  {
    pMenu->ModifyMenu(PEGBOARD_RETURN, MF_BYCOMMAND, PEGBOARD_RETURN, "Return");
    pMenu->EnableMenuItem(PEGBOARD_RETURN, MF_BYCOMMAND | MF_GRAYED);
  }
//
//  Either disable "Swap" or show the bus number to return
//
  if(bFoundHighlightedSquare && (m_pPEGBOARD[m_PegboardIndex].flags & PEGBOARD_FLAG_BUSISASSIGNED))
  {
    sprintf(tempString, "Swap bus %s", m_pPEGBOARD[m_PegboardIndex].szBusNumber);
    pMenu->ModifyMenu(PEGBOARD_SWAP, MF_BYCOMMAND, PEGBOARD_SWAP, tempString);
    pMenu->EnableMenuItem(PEGBOARD_SWAP, MF_BYCOMMAND | MF_ENABLED);
  }
  else
  {
    pMenu->ModifyMenu(PEGBOARD_SWAP, MF_BYCOMMAND, PEGBOARD_SWAP, "Swap");
    pMenu->EnableMenuItem(PEGBOARD_SWAP, MF_BYCOMMAND | MF_GRAYED);
  }
//
//  Show either "Take this bus out of service" or "Put this bus back in service"
//
  if(!bFoundHighlightedSquare || (m_pPEGBOARD[m_PegboardIndex].flags & PEGBOARD_FLAG_BUSISASSIGNED))
  {
    sprintf(tempString, "Put bus in/out of service", m_pPEGBOARD[m_PegboardIndex].szBusNumber);
    pMenu->ModifyMenu(PEGBOARD_PUTBACKINSERVICE, MF_BYCOMMAND, PEGBOARD_PUTBACKINSERVICE, tempString);
    pMenu->EnableMenuItem(PEGBOARD_PUTBACKINSERVICE, MF_BYCOMMAND | MF_GRAYED);
  }
  else
  {
    if(m_pPEGBOARD[m_PegboardIndex].flags & PEGBOARD_FLAG_BUSISINSERVICE)
    {
      sprintf(tempString, "Take bus %s out of service", m_pPEGBOARD[m_PegboardIndex].szBusNumber);
    }
    else
    {
      sprintf(tempString, "Put bus %s back into service", m_pPEGBOARD[m_PegboardIndex].szBusNumber);
    }
    pMenu->ModifyMenu(PEGBOARD_PUTBACKINSERVICE, MF_BYCOMMAND, PEGBOARD_PUTBACKINSERVICE, tempString);
    pMenu->EnableMenuItem(PEGBOARD_PUTBACKINSERVICE, MF_BYCOMMAND | MF_ENABLED);
  }
//
//  For the "Mark as" menu items, there's a state table:
//
//  +--------------+-----------------------+-----------------------+-----------------------+                       
//  |    State     |        Mark as        |        Mark as        |        Mark As        |
//  |              |      Short Shift      |    Charter Reserve    |  Sightseeing Reserve  |
//  +--------------+-----------------------+-----------------------+-----------------------+ 
//  |  Available   | Enabled               | Enabled               | Enabled               |
//  |              | w/Bus# as part of text| w/Bus# as part of text| w/Bus# as part of text|
//  +--------------+-----------------------+-----------------------+-----------------------+ 
//  |  Inuse       | Disabled              | Disabled              | Disabled              |
//  |              | w/no Bus# in text     | w/no Bus# in text     | w/no Bus# in text     |
//  +--------------+-----------------------+-----------------------+-----------------------+ 
//  |  O-o-S       | Disabled              | Disabled              | Disabled              |
//  |              | w/no Bus# in text     | w/no Bus# in text     | w/no Bus# in text     |
//  +--------------+-----------------------+-----------------------+-----------------------+ 
//  |  Marked as   | Enabled               | Disabled              | Disabled              |
//  | Short Shift  | w/Bus# & unmark text  | w/no Bus# in text     | w/no Bus# in text     |
//  +--------------+-----------------------+-----------------------+-----------------------+ 
//  |  Marked as   | Disabled              | Enabled               | Disabled              |
//  | Charter Res  | w/no Bus# in text     | w/Bus# & unmark text  | w/no Bus# in text     |
//  +--------------+-----------------------+-----------------------+-----------------------+ 
//  |  Marked as   | Disabled              | Disabled              | Enabled               |
//  | SSeeing Res  | w/no Bus# in text     | w/no Bus# in text     | w/Bus# & unmark text  |
//  +--------------+-----------------------+-----------------------+-----------------------+ 
//
  char szStatus[9][64];
  int  greyed[3];
  int  statusIndex[3];
//
//  Text
//  
  strcpy(szStatus[0], "As a short shift...");
  if(bFoundHighlightedSquare)
  {
    sprintf(szStatus[1], "Unmark %s as a short shift...", m_pPEGBOARD[m_PegboardIndex].szBusNumber);
    sprintf(szStatus[2], "Mark %s as a short shift...", m_pPEGBOARD[m_PegboardIndex].szBusNumber);
  }
  strcpy(szStatus[3], "As reserved for charter");
  if(bFoundHighlightedSquare)
  {
    sprintf(szStatus[4], "Unmark %s as reserved for charter", m_pPEGBOARD[m_PegboardIndex].szBusNumber);
    sprintf(szStatus[5], "Mark %s as reserved for charter", m_pPEGBOARD[m_PegboardIndex].szBusNumber);
  }
  strcpy(szStatus[6], "As reserved for sight-seeing");
  if(bFoundHighlightedSquare)
  {
    sprintf(szStatus[7], "Unmark %s as reserved for sight-seeing", m_pPEGBOARD[m_PegboardIndex].szBusNumber);
    sprintf(szStatus[8], "Mark %s as reserved for sight-seeing", m_pPEGBOARD[m_PegboardIndex].szBusNumber);
  }
//
// Determine the dynamic mark/unmark popup values
//
  if(m_pPEGBOARD[m_PegboardIndex].flags & PEGBOARD_FLAG_BUSISASSIGNED)
  {
    greyed[0] = MF_GRAYED;
    greyed[1] = MF_GRAYED;
    greyed[2] = MF_GRAYED;
    statusIndex[0] = 0;
    statusIndex[1] = 3;
    statusIndex[2] = 6;
  }
  else if(m_pPEGBOARD[m_PegboardIndex].flags & PEGBOARD_FLAG_BUSMARKEDOUTOFSERVICE)
  {
    greyed[0] = MF_GRAYED;
    greyed[1] = MF_GRAYED;
    greyed[2] = MF_GRAYED;
    statusIndex[0] = 0;
    statusIndex[1] = 3;
    statusIndex[2] = 6;
  }
  else if(m_pPEGBOARD[m_PegboardIndex].flags & PEGBOARD_FLAG_BUSMARKEDASSHORTSHIFT)
  {
    greyed[0] = MF_ENABLED;
    greyed[1] = MF_GRAYED;
    greyed[2] = MF_GRAYED;
    statusIndex[0] = 1;
    statusIndex[1] = 3;
    statusIndex[2] = 6;
  }
  else if(m_pPEGBOARD[m_PegboardIndex].flags & PEGBOARD_FLAG_BUSMARKEDASCHARTER)
  {
    greyed[0] = MF_GRAYED;
    greyed[1] = MF_ENABLED;
    greyed[2] = MF_GRAYED;
    statusIndex[0] = 0;
    statusIndex[1] = 4;
    statusIndex[2] = 6;
  }
  else if(m_pPEGBOARD[m_PegboardIndex].flags & PEGBOARD_FLAG_BUSMARKEDASSIGHTSEEING)
  {
    greyed[0] = MF_GRAYED;
    greyed[1] = MF_GRAYED;
    greyed[2] = MF_ENABLED;
    statusIndex[0] = 0;
    statusIndex[1] = 3;
    statusIndex[2] = 7;
  }
  else // Available
  {
    greyed[0] = MF_ENABLED;
    greyed[1] = MF_ENABLED;
    greyed[2] = MF_ENABLED;
    statusIndex[0] = 2;
    statusIndex[1] = 5;
    statusIndex[2] = 8;
  }
//
//  Modify the menu
//
  pMenu->ModifyMenu(PEGBOARD_MARKASSHORTSHIFT, MF_BYCOMMAND, PEGBOARD_MARKASSHORTSHIFT, szStatus[statusIndex[0]]);
  pMenu->ModifyMenu(PEGBOARD_MARKASCHARTER, MF_BYCOMMAND, PEGBOARD_MARKASCHARTER, szStatus[statusIndex[1]]);
  pMenu->ModifyMenu(PEGBOARD_MARKASSIGHTSEEING, MF_BYCOMMAND, PEGBOARD_MARKASSIGHTSEEING, szStatus[statusIndex[2]]);
  pMenu->EnableMenuItem(PEGBOARD_MARKASSHORTSHIFT, MF_BYCOMMAND | greyed[0]); 
  pMenu->EnableMenuItem(PEGBOARD_MARKASCHARTER, MF_BYCOMMAND | greyed[1]); 
  pMenu->EnableMenuItem(PEGBOARD_MARKASSIGHTSEEING, MF_BYCOMMAND | greyed[2]); 
//
//  Check the selected "show" mode
//
  CMenu* pMenu2 = pMenu->GetSubMenu(PBMENUPOS_POPUPSHOW);

  if(pMenu2)
  {
    pMenu2->CheckMenuItem(PEGBOARD_ALL, MF_BYCOMMAND | MF_UNCHECKED);
    pMenu2->CheckMenuItem(PEGBOARD_JUSTASSIGNED, MF_BYCOMMAND | MF_UNCHECKED);
    pMenu2->CheckMenuItem(PEGBOARD_JUSTAVAILABLE, MF_BYCOMMAND | MF_UNCHECKED);
    pMenu2->CheckMenuItem(PEGBOARD_JUSTOUTOFSERVICE, MF_BYCOMMAND | MF_UNCHECKED);
    if(m_bPegboardShowFlags & PEGBOARD_FLAG_SHOWALL)
    {
      pMenu2->CheckMenuItem(PEGBOARD_ALL, MF_BYCOMMAND | MF_CHECKED);
    }
    else if(m_bPegboardShowFlags & PEGBOARD_FLAG_SHOWASSIGNED)
    {
      pMenu2->CheckMenuItem(PEGBOARD_JUSTASSIGNED, MF_BYCOMMAND | MF_CHECKED);
    }
    else if(m_bPegboardShowFlags & PEGBOARD_FLAG_SHOWAVAILABLE)
    {
      pMenu2->CheckMenuItem(PEGBOARD_JUSTAVAILABLE, MF_BYCOMMAND | MF_CHECKED);
    }
    else if(m_bPegboardShowFlags & PEGBOARD_FLAG_SHOWOUTOFSERVICE)
    {
      pMenu2->CheckMenuItem(PEGBOARD_JUSTOUTOFSERVICE, MF_BYCOMMAND | MF_CHECKED);
    }
  }
//
//  Check the selected "sort" mode
//
  pMenu2 = pMenu->GetSubMenu(PBMENUPOS_POPUPSORTBY);
  if(pMenu2)
  {
    pMenu2->CheckMenuItem(PEGBOARD_BUSLOCATION, MF_BYCOMMAND | MF_UNCHECKED);
    pMenu2->CheckMenuItem(PEGBOARD_BUSNUMBER, MF_BYCOMMAND | MF_UNCHECKED);
    pMenu2->CheckMenuItem(PEGBOARD_BLOCKNUMBER, MF_BYCOMMAND | MF_UNCHECKED);
    pMenu2->CheckMenuItem(PEGBOARD_RUNNUMBER, MF_BYCOMMAND | MF_UNCHECKED);
    pMenu2->CheckMenuItem(PEGBOARD_ROSTERNUMBER, MF_BYCOMMAND | MF_UNCHECKED);
    pMenu2->CheckMenuItem(PEGBOARD_PULLINTIME, MF_BYCOMMAND | MF_UNCHECKED);
    if(m_bPegboardSortFlags & PEGBOARD_FLAG_SORTBYLOCATION)
    {
      pMenu2->CheckMenuItem(PEGBOARD_BUSLOCATION, MF_BYCOMMAND | MF_CHECKED);
    }
    else if(m_bPegboardSortFlags & PEGBOARD_FLAG_SORTBYBUS)
    {
      pMenu2->CheckMenuItem(PEGBOARD_BUSNUMBER, MF_BYCOMMAND | MF_CHECKED);
    }
    else if(m_bPegboardSortFlags & PEGBOARD_FLAG_SORTBYBLOCK)
    {
      pMenu2->CheckMenuItem(PEGBOARD_BLOCKNUMBER, MF_BYCOMMAND | MF_CHECKED);
    }
    else if(m_bPegboardSortFlags & PEGBOARD_FLAG_SORTBYRUN)
    {
      pMenu2->CheckMenuItem(PEGBOARD_RUNNUMBER, MF_BYCOMMAND | MF_CHECKED);
    }
    else if(m_bPegboardSortFlags & PEGBOARD_FLAG_SORTBYROSTER)
    {
      pMenu2->CheckMenuItem(PEGBOARD_ROSTERNUMBER, MF_BYCOMMAND | MF_CHECKED);
    }
    else if(m_bPegboardSortFlags & PEGBOARD_FLAG_SORTBYPITIME)
    {
      pMenu2->CheckMenuItem(PEGBOARD_PULLINTIME, MF_BYCOMMAND | MF_CHECKED);
    }
  }
  pMenu->TrackPopupMenu(TPM_LEFTALIGN, point.x, point.y, pWnd, NULL);
  m_menu.DestroyMenu();
}

void CDailyOps::OnAll() 
{
  m_PreviousTime = m_SystemTime;

  m_bPegboardShowFlags = PEGBOARD_FLAG_SHOWALL;
  InvalidateRect(&m_PegboardRect, TRUE);
}

void CDailyOps::OnJustassigned() 
{
  m_PreviousTime = m_SystemTime;

  m_bPegboardShowFlags = PEGBOARD_FLAG_SHOWASSIGNED;
  InvalidateRect(&m_PegboardRect, TRUE);
}

void CDailyOps::OnJustavailable() 
{
  m_PreviousTime = m_SystemTime;

  m_bPegboardShowFlags = PEGBOARD_FLAG_SHOWAVAILABLE;
  InvalidateRect(&m_PegboardRect, TRUE);
}

void CDailyOps::OnJustoutofservice() 
{
  m_PreviousTime = m_SystemTime;

  m_bPegboardShowFlags = PEGBOARD_FLAG_SHOWOUTOFSERVICE;
  InvalidateRect(&m_PegboardRect, TRUE);
}

void CDailyOps::OnClearall() 
{
  m_PreviousTime = m_SystemTime;

  CString s;
  int rcode2;
  int response;

  if(!m_bMessageDisplay)
  {
    response = IDYES;
  }
  else
  {
    s.LoadString(TEXT_303);  // Are you sure?
    MessageBeep(MB_ICONQUESTION);
    response = MessageBox(s, TMS, MB_YESNO | MB_ICONQUESTION);
    if(response == IDYES)
    {
      s.LoadString(ERROR_366);  //  Are you *really* sure?
      MessageBeep(MB_ICONQUESTION);
      response = MessageBox(s, TMS, MB_YESNO | MB_ICONQUESTION);
    }
  }
  if(response != IDYES)
  {
    return;
  }
//
//  Tell the audit trail that we did a clear all - 
//  it'll make the multiple clears make more sense
//
  int nI;

  DailyOpsBuildRecord(&DAILYOPS, DAILYOPS_FLAG_BUS);
  DAILYOPS.pertainsToDate = m_DailyOpsDateYYYYMMDD;
//  DAILYOPS.pertainsToTime = m_DailyOpsDate.GetHour() * 3600 + m_DailyOpsDate.GetMinute() * 60 + m_DailyOpsDate.GetSecond();
  DAILYOPS.pertainsToTime = DAILYOPS.recordID;
  DAILYOPS.recordFlags = DAILYOPS_FLAG_BUSCLEARALL;
  rcode2 = btrieve(B_INSERT, TMS_DAILYOPS, &DAILYOPS, &DAILYOPSKey0, 0);
  if(rcode2 == 0)
  {
    m_LastDAILYOPSRecordID = DAILYOPS.recordID;
  }
//
//  Now do it
//
  for(nI = 0; nI < m_numBuses; nI++)
  {
    if(m_pPEGBOARD[nI].flags & PEGBOARD_FLAG_BUSISASSIGNED)
    {
      m_pPEGBOARD[nI].flags &= ~PEGBOARD_FLAG_BUSISASSIGNED;
      m_pPEGBOARD[nI].flags |= PEGBOARD_FLAG_BUSISINSERVICE;
      m_BlockInfo[m_pPEGBOARD[nI].indexToBlockInfo].flags &= ~BLOCKINFO_FLAG_ASSIGNED;
      m_BlockInfo[m_pPEGBOARD[nI].indexToBlockInfo].flags |= BLOCKINFO_FLAG_BUSRETURN;
      m_pPEGBOARD[nI].locationNODESrecordID = m_BlockInfo[m_pPEGBOARD[nI].indexToBlockInfo].PIGNODESrecordID;
      m_BlockInfo[m_pPEGBOARD[nI].indexToBlockInfo].BUSESrecordID = NO_RECORD;
//
//  Record the return all as individual returns
//
      DailyOpsBuildRecord(&DAILYOPS, DAILYOPS_FLAG_BUS);
      DAILYOPS.pertainsToDate = m_DailyOpsDateYYYYMMDD;
//      DAILYOPS.pertainsToTime = m_DailyOpsDate.GetHour() * 3600 + m_DailyOpsDate.GetMinute() * 60 + m_DailyOpsDate.GetSecond();
      DAILYOPS.pertainsToTime = DAILYOPS.recordID;
      DAILYOPS.recordFlags = DAILYOPS_FLAG_BUSRETURN | DAILYOPS_FLAG_SETBUSLOCATION;
      DAILYOPS.DAILYOPSrecordID = m_pPEGBOARD[nI].DAILYOPSrecordID;
      DAILYOPS.DOPS.Bus.BUSESrecordID = m_pPEGBOARD[nI].BUSESrecordID;
      DAILYOPS.DOPS.Bus.TRIPSrecordID = m_BlockInfo[m_pPEGBOARD[nI].indexToBlockInfo].TRIPSrecordID;
      DAILYOPS.DOPS.Bus.locationNODESrecordID = m_BlockInfo[m_pPEGBOARD[nI].indexToBlockInfo].PIGNODESrecordID;
      rcode2 = btrieve(B_INSERT, TMS_DAILYOPS, &DAILYOPS, &DAILYOPSKey0, 0);
      if(rcode2 == 0)
      {
        m_LastDAILYOPSRecordID = DAILYOPS.recordID;
      }
    }
  }
//
//  Refresh the Block list and pegboard displays
//
  InvalidateRect(&m_PegboardRect, TRUE);
  RefreshBlockList(NO_RECORD);
}

void CDailyOps::OnPutbackinservice() 
{
  m_PreviousTime = m_SystemTime;

  if(m_PegboardIndex == NO_RECORD)
  {
    return;
  }

  int rcode2;
//
//  If it's in service, put it out
//
  if(m_pPEGBOARD[m_PegboardIndex].flags & PEGBOARD_FLAG_BUSISINSERVICE)
  {
    DAILYOPSBSPassedDataDef OOSPassedData;

    CDailyOpsOOS dlg(this, &OOSPassedData);

    if(dlg.DoModal() == IDOK)
    {
      m_pPEGBOARD[m_PegboardIndex].flags &= ~PEGBOARD_FLAG_BUSISINSERVICE;
      m_pPEGBOARD[m_PegboardIndex].flags |= PEGBOARD_FLAG_BUSMARKEDOUTOFSERVICE;
      m_pPEGBOARD[m_PegboardIndex].swapReasonIndex = (long)OOSPassedData.swapReasonIndex;
//
//  Tell the audit trail that this one's out of service
//
//  Database verify: 11-Jan-07
//
      DailyOpsBuildRecord(&DAILYOPS, DAILYOPS_FLAG_BUS);
      DAILYOPS.pertainsToDate = m_DailyOpsDateYYYYMMDD;
//      DAILYOPS.pertainsToTime = m_DailyOpsDate.GetHour() * 3600 + m_DailyOpsDate.GetMinute() * 60 + m_DailyOpsDate.GetSecond();
      DAILYOPS.pertainsToTime = DAILYOPS.recordID;
      DAILYOPS.recordFlags = DAILYOPS_FLAG_BUSMARKEDOUTOFSERVICE;
      DAILYOPS.DOPS.Bus.BUSESrecordID = m_pPEGBOARD[m_PegboardIndex].BUSESrecordID;
      DAILYOPS.DOPS.Bus.swapReasonIndex = OOSPassedData.swapReasonIndex;
      DAILYOPS.DOPS.Bus.PAX = OOSPassedData.PAX;
      DAILYOPS.DOPS.Bus.locationNODESrecordID = OOSPassedData.locationNODESrecordID;
      DAILYOPS.DOPS.Bus.untilTime = OOSPassedData.time;
      rcode2 = btrieve(B_INSERT, TMS_DAILYOPS, &DAILYOPS, &DAILYOPSKey0, 0);
      if(rcode2 == 0)
      {
        m_LastDAILYOPSRecordID = DAILYOPS.recordID;
      }
    }
  }
//
//  If it's out of service, put it in
//
  else
  {
    m_pPEGBOARD[m_PegboardIndex].flags &= ~PEGBOARD_FLAG_BUSMARKEDOUTOFSERVICE;
    m_pPEGBOARD[m_PegboardIndex].flags |= PEGBOARD_FLAG_BUSISINSERVICE | DAILYOPS_FLAG_BUSMARKEDINSERVICE;
    m_pPEGBOARD[m_PegboardIndex].swapReasonIndex = NO_RECORD;
//
//  Tell the audit trail that this one's in service
//
//  Database verify: 11-Jan-07
//
    DailyOpsBuildRecord(&DAILYOPS, DAILYOPS_FLAG_BUS);
    DAILYOPS.pertainsToDate = m_DailyOpsDateYYYYMMDD;
//    DAILYOPS.pertainsToTime = m_DailyOpsDate.GetHour() * 3600 + m_DailyOpsDate.GetMinute() * 60 + m_DailyOpsDate.GetSecond();
    DAILYOPS.pertainsToTime = DAILYOPS.recordID;
    DAILYOPS.recordFlags = DAILYOPS_FLAG_BUSISINSERVICE | DAILYOPS_FLAG_BUSMARKEDINSERVICE;
    DAILYOPS.DOPS.Bus.BUSESrecordID = m_pPEGBOARD[m_PegboardIndex].BUSESrecordID;
    rcode2 = btrieve(B_INSERT, TMS_DAILYOPS, &DAILYOPS, &DAILYOPSKey0, 0);
    if(rcode2 == 0)
    {
      m_LastDAILYOPSRecordID = DAILYOPS.recordID;
    }
  }
//
//  Fix the display
//
  InvalidateRect(&m_PegboardRect, TRUE);
}

void CDailyOps::OnAssignbus() 
{
  m_PreviousTime = m_SystemTime;

  int  rcode2;

  BOOL bGoodToGo = m_pPEGBOARD[m_PegboardIndex].locationNODESrecordID == NO_RECORD ||
        m_pPEGBOARD[m_PegboardIndex].locationNODESrecordID == m_BlockInfo[m_SelectedBlockIndex].POGNODESrecordID;
  
  if(!bGoodToGo)
  {
    if(m_bMessageDisplay)
    {
      strcpy(tempString, "According to the system, this vehicle isn't at the Pull-out location\n\nOk to Proceed?");
      MessageBeep(MB_ICONQUESTION);
      if(MessageBox(tempString, TMS, MB_YESNO | MB_ICONQUESTION) != IDYES)
      {
        return;
      }
    }
  }
  m_BlockInfo[m_SelectedBlockIndex].BUSESrecordID = m_pPEGBOARD[m_PegboardIndex].BUSESrecordID;
  m_BlockInfo[m_SelectedBlockIndex].flags = BLOCKINFO_FLAG_ASSIGNED;
//
//  Record the assignment
//
//  Database verify: 11-Jan-07
//
  DailyOpsBuildRecord(&DAILYOPS, DAILYOPS_FLAG_BUS);
  DAILYOPS.recordFlags = DAILYOPS_FLAG_BUSASSIGNMENT;
  if(m_bOverrideInProgress)
  {
    DAILYOPS.recordFlags |= DAILYOPS_FLAG_BUSOVERRIDE;
  }
  DAILYOPS.pertainsToDate = m_DailyOpsDateYYYYMMDD;
//  DAILYOPS.pertainsToTime = m_DailyOpsDate.GetHour() * 3600 + m_DailyOpsDate.GetMinute() * 60 + m_DailyOpsDate.GetSecond();
  DAILYOPS.pertainsToTime = DAILYOPS.recordID;
  DAILYOPS.DOPS.Bus.BUSESrecordID = m_BlockInfo[m_SelectedBlockIndex].BUSESrecordID;
  DAILYOPS.DOPS.Bus.TRIPSrecordID = m_BlockInfo[m_SelectedBlockIndex].TRIPSrecordID;
  DAILYOPS.DOPS.Bus.RGRPROUTESrecordID = m_BlockInfo[m_SelectedBlockIndex].RGRPROUTESrecordID;
  DAILYOPS.DOPS.Bus.SGRPSERVICESrecordID = m_BlockInfo[m_SelectedBlockIndex].SGRPSERVICESrecordID;
  DAILYOPS.DOPS.Bus.newBUSESrecordID = NO_RECORD;
  DAILYOPS.DOPS.Bus.swapReasonIndex = NO_RECORD;
  DAILYOPS.DOPS.Bus.locationNODESrecordID = m_pPEGBOARD[m_PegboardIndex].locationNODESrecordID;
  DAILYOPS.DOPS.Bus.RUNSrecordID = m_BlockInfo[m_SelectedBlockIndex].startingRUNSrecordID;
  rcode2 = btrieve(B_INSERT, TMS_DAILYOPS, &DAILYOPS, &DAILYOPSKey0, 0);
  if(rcode2 == 0)
  {
    m_LastDAILYOPSRecordID = DAILYOPS.recordID;
  }
//
//  Send the data out in real time
//
  if(m_bUseStrategicMapping || m_bUseConnexionz)
  {
    char szParm6[32];
    HCURSOR hSaveCursor = SetCursor(hCursorWait);
    long SystemDate = m_SystemTime.GetYear() * 10000 + m_SystemTime.GetMonth() * 100 + m_SystemTime.GetDay();
    long pieceNumber;

    if(m_DailyOpsDateYYYYMMDD == SystemDate)
    {
      sprintf(szParm6, "%4d-%02d-%02dT%02d:%02d:%02d", 
            m_SystemTime.GetYear(), m_SystemTime.GetMonth(), m_SystemTime.GetDay(),
            m_SystemTime.GetHour(), m_SystemTime.GetMinute(), m_SystemTime.GetSecond());
    }
    else
    {
      long year, month, day;
      long hours, minutes;

      GetYMD(m_DailyOpsDateYYYYMMDD, &year, &month, &day);
      hours = m_BlockInfo[m_SelectedBlockIndex].POTime / 3600;
      minutes = (m_BlockInfo[m_SelectedBlockIndex].POTime / 60) % 60;

      sprintf(szParm6, "%4d-%02d-%02dT%02d:%02d:00", year, month, day, hours, minutes); 
    }
    RUNSKey0.recordID = m_BlockInfo[m_SelectedBlockIndex].startingRUNSrecordID;
    rcode2 = btrieve(B_GETEQUAL, TMS_RUNS, &RUNS, &RUNSKey0, 0);
    pieceNumber = (rcode2 == 0 ? RUNS.pieceNumber : 1);
    m_AddAssignment(DAILYOPS.recordID, m_BlockInfo[m_SelectedBlockIndex].startingDRIVERSrecordID, RUNS.recordID,
           pieceNumber, m_BlockInfo[m_SelectedBlockIndex].startingRunNumber, DAILYOPS.DOPS.Bus.BUSESrecordID, szParm6);
    SetCursor(hSaveCursor);
  }
//
//  Update the row from the Block list box and tell m_pPEGBOARD about it
//
  RefreshBlockList(m_SelectedBlockIndex);
  m_pPEGBOARD[m_PegboardIndex].flags = PEGBOARD_FLAG_BUSISASSIGNED;
  m_pPEGBOARD[m_PegboardIndex].DAILYOPSrecordID = DAILYOPS.recordID;
  m_pPEGBOARD[m_PegboardIndex].indexToBlockInfo = m_SelectedBlockIndex;
//
//  Fix the display
//
  InvalidateRect(&m_PegboardRect, TRUE);
}


void CDailyOps::OnReturn() 
{
  m_PreviousTime = m_SystemTime;

  CString s;
  int  rcode2;
//
//  Record the return
//
//  Database verify: 11-Jan-07
//
  DailyOpsBuildRecord(&DAILYOPS, DAILYOPS_FLAG_BUS);
  DAILYOPS.pertainsToDate = m_DailyOpsDateYYYYMMDD;
//  DAILYOPS.pertainsToTime = m_DailyOpsDate.GetHour() * 3600 + m_DailyOpsDate.GetMinute() * 60 + m_DailyOpsDate.GetSecond();
  DAILYOPS.pertainsToTime = DAILYOPS.recordID;
  DAILYOPS.recordFlags = DAILYOPS_FLAG_BUSRETURN | DAILYOPS_FLAG_SETBUSLOCATION;
  DAILYOPS.DAILYOPSrecordID = m_pPEGBOARD[m_PegboardIndex].DAILYOPSrecordID;
  DAILYOPS.DOPS.Bus.BUSESrecordID = m_pPEGBOARD[m_PegboardIndex].BUSESrecordID;
  DAILYOPS.DOPS.Bus.TRIPSrecordID = m_BlockInfo[m_pPEGBOARD[m_PegboardIndex].indexToBlockInfo].TRIPSrecordID;
  DAILYOPS.DOPS.Bus.locationNODESrecordID = m_BlockInfo[m_pPEGBOARD[m_PegboardIndex].indexToBlockInfo].PIGNODESrecordID;
  rcode2 = btrieve(B_INSERT, TMS_DAILYOPS, &DAILYOPS, &DAILYOPSKey0, 0);
  if(rcode2 == 0)
  {
    m_LastDAILYOPSRecordID = DAILYOPS.recordID;
  }
//
//  Send the data out in real time
//
  if(m_bUseStrategicMapping)
  {
    HCURSOR hSaveCursor = SetCursor(hCursorWait);

    m_DeleteAssignment(DAILYOPS.DAILYOPSrecordID);
    SetCursor(hSaveCursor);
  }
//
//  Update the m_BlockInfo structure
//
  m_BlockInfo[m_pPEGBOARD[m_PegboardIndex].indexToBlockInfo].flags &= ~BLOCKINFO_FLAG_ASSIGNED;
  m_BlockInfo[m_pPEGBOARD[m_PegboardIndex].indexToBlockInfo].flags |= BLOCKINFO_FLAG_BUSRETURN;
  m_BlockInfo[m_pPEGBOARD[m_PegboardIndex].indexToBlockInfo].BUSESrecordID = NO_RECORD;
//
//  Set the location
//
  m_pPEGBOARD[m_PegboardIndex].locationNODESrecordID = m_BlockInfo[m_pPEGBOARD[m_PegboardIndex].indexToBlockInfo].PIGNODESrecordID;
//
//  Clear out bustype and bus
//
  m_pPEGBOARD[m_PegboardIndex].flags &= ~PEGBOARD_FLAG_BUSISASSIGNED;
  m_pPEGBOARD[m_PegboardIndex].flags |= PEGBOARD_FLAG_BUSISINSERVICE;
  m_pPEGBOARD[m_PegboardIndex].DAILYOPSrecordID = NO_RECORD;
//
//  Fix the display
//
  InvalidateRect(&m_PegboardRect, TRUE);
//
  LVITEM LVI;
  int item = NO_RECORD;
  int nI;

  for(nI = 0; nI < m_numBlocksInBlockInfo; nI++)
  {
    LVI.mask = LVIF_PARAM;
    LVI.iItem = nI;
    LVI.iSubItem = 0;
    pListCtrlBLOCKLIST->GetItem(&LVI);
    if(m_pPEGBOARD[m_PegboardIndex].indexToBlockInfo == LVI.lParam)
    {
      item = nI;
      break;
    }
  }
  RefreshBlockList(item);
}

//
//  OnSwap() - Swap out a vehicle
//
void CDailyOps::OnSwap() 
{
  m_PreviousTime = m_SystemTime;

  CString s;
  int  rcode2;
  int  index = m_pPEGBOARD[m_PegboardIndex].indexToBlockInfo;

  if(index < 0)
  {
    return;
  }
//
//  Record the swap
//
  DAILYOPSBSPassedDataDef BSPassedData;

  CDailyOpsBS dlg(this, &BSPassedData);

  if(dlg.DoModal() != IDOK)
  {
    return;
  }

  DailyOpsBuildRecord(&DAILYOPS, DAILYOPS_FLAG_BUS);
  DAILYOPS.recordFlags = DAILYOPS_FLAG_BUSSWAP;
  DAILYOPS.pertainsToDate = m_DailyOpsDateYYYYMMDD;
//  DAILYOPS.pertainsToTime = m_DailyOpsDate.GetHour() * 3600 + m_DailyOpsDate.GetMinute() * 60 + m_DailyOpsDate.GetSecond();
  DAILYOPS.pertainsToTime = DAILYOPS.recordID;
  if(BSPassedData.bMarkOutOfService)
  {
    DAILYOPS.recordFlags |= DAILYOPS_FLAG_BUSMARKEDOUTOFSERVICE;
  }
  DAILYOPS.DAILYOPSrecordID = m_pPEGBOARD[m_PegboardIndex].DAILYOPSrecordID;
  DAILYOPS.DRIVERSrecordID = NO_RECORD;
  DAILYOPS.DOPS.Bus.TRIPSrecordID = m_BlockInfo[index].TRIPSrecordID;
  DAILYOPS.DOPS.Bus.BUSESrecordID = m_pPEGBOARD[m_PegboardIndex].BUSESrecordID;
  DAILYOPS.DOPS.Bus.swapReasonIndex = BSPassedData.swapReasonIndex;
  DAILYOPS.DOPS.Bus.PAX = BSPassedData.PAX;
  DAILYOPS.DOPS.Bus.locationNODESrecordID = BSPassedData.locationNODESrecordID;
  DAILYOPS.DOPS.Bus.untilTime = BSPassedData.time;
  rcode2 = btrieve(B_INSERT, TMS_DAILYOPS, &DAILYOPS, &DAILYOPSKey0, 0);
  if(rcode2 == 0)
  {
    m_LastDAILYOPSRecordID = DAILYOPS.recordID;
  }
//
//  Adjust the pegboard
//
  m_pPEGBOARD[m_PegboardIndex].DAILYOPSrecordID = NO_RECORD;
  m_pPEGBOARD[m_PegboardIndex].flags &= ~PEGBOARD_FLAG_BUSISASSIGNED;
  if(BSPassedData.bMarkOutOfService)
  {
    m_pPEGBOARD[m_PegboardIndex].swapReasonIndex = (long)BSPassedData.swapReasonIndex;
    m_pPEGBOARD[m_PegboardIndex].flags |= PEGBOARD_FLAG_BUSMARKEDOUTOFSERVICE;
    m_pPEGBOARD[m_PegboardIndex].flags &= ~PEGBOARD_FLAG_BUSISINSERVICE;
  }
  else
  {
    m_pPEGBOARD[m_PegboardIndex].flags &= ~PEGBOARD_FLAG_BUSISASSIGNED;
    m_pPEGBOARD[m_PegboardIndex].flags |= PEGBOARD_FLAG_BUSISINSERVICE;
  }
//
//  And add it back to the Block list box
//
  m_BlockInfo[index].flags = BLOCKINFO_FLAG_TOBEASSIGNED | BLOCKINFO_FLAG_BUSRETURN;
//
//  Fix the display
//
  InvalidateRect(&m_PegboardRect, TRUE);
//
  LVITEM LVI;
  int item = NO_RECORD;
  int nI;

  for(nI = 0; nI < m_numBlocksInBlockInfo; nI++)
  {
    LVI.mask = LVIF_PARAM;
    LVI.iItem = nI;
    LVI.iSubItem = 0;
    pListCtrlBLOCKLIST->GetItem(&LVI);
    if(m_pPEGBOARD[m_PegboardIndex].indexToBlockInfo == LVI.lParam)
    {
      item = nI;
      break;
    }
  }
  RefreshBlockList(item);
}


void CDailyOps::OnBuslocation() 
{
  m_PreviousTime = m_SystemTime;

  SortPegboard(PEGBOARD_FLAG_SORTBYLOCATION);
}

void CDailyOps::OnBusnumber() 
{
  m_PreviousTime = m_SystemTime;

  SortPegboard(PEGBOARD_FLAG_SORTBYBUS);
}

void CDailyOps::OnBlocknumber() 
{
  m_PreviousTime = m_SystemTime;

  SortPegboard(PEGBOARD_FLAG_SORTBYBLOCK);
}

void CDailyOps::OnRunnumber() 
{
  m_PreviousTime = m_SystemTime;

  SortPegboard(PEGBOARD_FLAG_SORTBYRUN);
}

void CDailyOps::OnRosternumber() 
{
  m_PreviousTime = m_SystemTime;

  SortPegboard(PEGBOARD_FLAG_SORTBYROSTER);
}

void CDailyOps::OnPullintime() 
{
  m_PreviousTime = m_SystemTime;

  SortPegboard(PEGBOARD_FLAG_SORTBYPITIME);
}

void CDailyOps::SortPegboard(long flag)
{
  m_PreviousTime = m_SystemTime;

  m_bPegboardSortFlags = flag;
  qsort((void *)m_pPEGBOARD, m_numBuses, sizeof(PEGBOARDDef), sort_Pegboard);
  m_FirstBusRowDisplayed = 0;
  InvalidateRect(&m_PegboardRect, TRUE);
}

void CDailyOps::OnLButtonDblClk(UINT nFlags, CPoint point) 
{
  m_PreviousTime = m_SystemTime;
//
//  point comes in in dialog coords.  This should come as 
//  no surprize, as the right-click for the context menu
//  comes in as screen coords.
//
//
//  If it's outside the drawing region, don't bother
//
  if(point.x >= m_PegboardRect.left && point.x <= m_PegboardRect.right &&
        point.y >= m_PegboardRect.top && point.y <= m_PegboardRect.bottom)
  {
    LVITEM LVI;
    BOOL bFound;
    int  nI, nJ;
    int  start = m_FirstBusRowDisplayed * 10;
//
//  Locate the highlighted square
//
    for(bFound = FALSE, nI = start; nI < m_numBuses; nI++)
    {
      if(nI > m_LastBusIndex)
      {
        break;
      }
      if(point.x >= m_pPEGBOARD[nI].rect.left && point.x <= m_pPEGBOARD[nI].rect.right &&
            point.y >= m_pPEGBOARD[nI].rect.top && point.y <= m_pPEGBOARD[nI].rect.bottom)
      {
        for(nJ = start; nJ < m_LastBusIndex; nJ++)
        {
          if(m_pPEGBOARD[nJ].flags & PEGBOARD_FLAG_HIGHLIGHT)
          {
            m_pPEGBOARD[nJ].flags &= ~PEGBOARD_FLAG_HIGHLIGHT;
            InvalidateRect(&m_pPEGBOARD[nJ].rect, TRUE);
          }
        }
        m_pPEGBOARD[nI].flags |= PEGBOARD_FLAG_HIGHLIGHT;
        m_PegboardIndex = nI;
        InvalidateRect(&m_pPEGBOARD[nI].rect, TRUE);
        bFound = TRUE;
        break;
      }
    }
//
//  Found it.  Check on the flags
//
    if(bFound)
    {
//
//  If it's out of service, put it in service
//
      if(m_pPEGBOARD[m_PegboardIndex].flags & PEGBOARD_FLAG_BUSMARKEDOUTOFSERVICE)
      {
        OnPutbackinservice();
      }
//
//  If it's assigned, return it
//
      else if(m_pPEGBOARD[m_PegboardIndex].flags & PEGBOARD_FLAG_BUSISASSIGNED)
      {
        if(!m_bMessageDisplay)
        {
          OnReturn();
        }
        else
        {
          sprintf(tempString, "Return bus %s?", m_pPEGBOARD[m_PegboardIndex].szBusNumber);
          MessageBeep(MB_ICONQUESTION);
          if(MessageBox(tempString, TMS, MB_YESNO | MB_ICONQUESTION) == IDYES)
          {
            OnReturn();
          }
        }
      }
//
//  If it's not assigned and there's a row highlighted in the Block list, assign it
//
      else if(!(m_pPEGBOARD[m_PegboardIndex].flags & PEGBOARD_FLAG_BUSISASSIGNED))
      {
        nI = pListCtrlBLOCKLIST->GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);
        if(nI < 0)
        {
          m_SelectedBlockIndex = NO_RECORD;
          m_SelectedBlockRow = NO_RECORD;
        }
        else
        {
          LVI.mask = LVIF_PARAM;
          LVI.iItem = nI;
          LVI.iSubItem = 0;
          pListCtrlBLOCKLIST->GetItem(&LVI);
          m_SelectedBlockIndex = LVI.lParam;
          m_SelectedBlockRow = nI;
        }
        if(m_SelectedBlockIndex != NO_RECORD)
        {
          if((m_pPEGBOARD[m_PegboardIndex].flags & PEGBOARD_FLAG_BUSMARKEDASSHORTSHIFT) &&
                !(m_BlockInfo[m_SelectedBlockIndex].flags & BLOCKINFO_FLAG_ASSIGNED))
          {
            if(m_BlockInfo[m_SelectedBlockIndex].PITime > m_pPEGBOARD[m_PegboardIndex].untilTime)
            {
              if(!m_bMessageDisplay)
              {
                OnAssignbus();
              }
              else
              {
                CString s;

                s.LoadString(TEXT_383);
                MessageBeep(MB_ICONQUESTION);
                if(MessageBox(s, TMS, MB_ICONQUESTION | MB_YESNO) == IDYES)
                {
                  OnAssignbus();
                }
              }
            }
            else
            {
              OnAssignbus();
            }
          }
          else if(m_BlockInfo[m_SelectedBlockIndex].flags & BLOCKINFO_FLAG_ASSIGNED)
          {
            if(!m_bMessageDisplay)
            {
              OnAssignbus();
            }
            else
            {
              MessageBeep(MB_ICONINFORMATION);
              for(nJ = 0; nJ < m_numBuses; nJ++)
              {
                if(m_pPEGBOARD[nJ].indexToBlockInfo == m_SelectedBlockIndex)
                {
                  sprintf(tempString, "This block has already been assigned to bus %s", m_pPEGBOARD[nJ].szBusNumber);
                  MessageBox(tempString, TMS, MB_ICONINFORMATION | MB_OK);
                  break; 
                }
              }
            }
          }
          else
          {
            OnAssignbus();
          }
        }
      }
    }
  }
	
	CDialog::OnLButtonDblClk(nFlags, point);
}

void CDailyOps::OnRclickBlocklist(NMHDR* pNMHDR, LRESULT* pResult) 
{
  m_PreviousTime = m_SystemTime;

  m_menu.LoadMenu(IDR_DAILYOPSBLOCKS);
  CMenu* pMenu = m_menu.GetSubMenu(0);
//
//  Get the current mouse location and convert it to client coordinates.
//
  DWORD pos = GetMessagePos();
  CPoint pt(LOWORD(pos), HIWORD(pos));

  if(pMenu)
  {
//
//  See what's highlighted
//
    LVITEM LVI;
    int nI = pListCtrlBLOCKLIST->GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);

    if(nI < 0)
    {
      m_SelectedBlockIndex = NO_RECORD;
      m_SelectedBlockRow = NO_RECORD;
    }
    else
    {
      LVI.mask = LVIF_PARAM;
      LVI.iItem = nI;
      LVI.iSubItem = 0;
      pListCtrlBLOCKLIST->GetItem(&LVI);
      m_SelectedBlockIndex = LVI.lParam;
      m_SelectedBlockRow = nI;
    }

    if(m_SelectedBlockIndex != NO_RECORD)
    {
      if(m_BlockInfo[m_SelectedBlockIndex].flags & BLOCKINFO_FLAG_BLOCKDROP)
      {
        pMenu->ModifyMenu(DAILYOPSBLOCKS_DROP, MF_BYCOMMAND, DAILYOPSBLOCKS_DROP, "Undrop");
      }
      else
      {
        pMenu->ModifyMenu(DAILYOPSBLOCKS_DROP, MF_BYCOMMAND, DAILYOPSBLOCKS_DROP, "Drop");
      }
    }
//
//  Set the display checkmarks
//
    CMenu* pMenu2 = pMenu->GetSubMenu(0);
    if(pMenu2)
    {
      if(m_BlockInfoShowFlags & BLOCKINFO_FLAG_DISPLAYALL)
      {
        pMenu2->CheckMenuItem(DAILYOPSBLOCKS_ALL, MF_BYCOMMAND | MF_CHECKED);
      }
      else if(m_BlockInfoShowFlags & BLOCKINFO_FLAG_TOBEASSIGNED)
      {
        pMenu2->CheckMenuItem(DAILYOPSBLOCKS_TBA, MF_BYCOMMAND | MF_CHECKED);
      }
      else if(m_BlockInfoShowFlags & BLOCKINFO_FLAG_ASSIGNED)
      {
        pMenu2->CheckMenuItem(DAILYOPSBLOCKS_ASSIGNED, MF_BYCOMMAND | MF_CHECKED);
      }
      else if(m_BlockInfoShowFlags & BLOCKINFO_FLAG_BUSRETURN)
      {
        pMenu2->CheckMenuItem(DAILYOPSBLOCKS_RETURNED, MF_BYCOMMAND | MF_CHECKED);
      }
      else if(m_BlockInfoShowFlags & BLOCKINFO_FLAG_BLOCKDROP)
      {
        pMenu2->CheckMenuItem(DAILYOPSBLOCKS_DROPPED, MF_BYCOMMAND | MF_CHECKED);
      }
    }
//
//  See if the one we're on has been dropped already.  If so,
//  then the menu item becomes "Undrop". If not, it's "Drop".
//
    pMenu->TrackPopupMenu(TPM_LEFTALIGN, pt.x, pt.y, FromHandle(m_hWnd), NULL);
  }
  m_menu.DestroyMenu();

	*pResult = 0;
}

void CDailyOps::OnDropped() 
{
  m_PreviousTime = m_SystemTime;

  m_BlockInfoShowFlags = BLOCKINFO_FLAG_BLOCKDROP;
  RefreshBlockList(NO_RECORD);
}

void CDailyOps::OnReturned() 
{
  m_PreviousTime = m_SystemTime;

  m_BlockInfoShowFlags = BLOCKINFO_FLAG_BUSRETURN;
  RefreshBlockList(NO_RECORD);
}

void CDailyOps::OnTba() 
{
  m_PreviousTime = m_SystemTime;

  m_BlockInfoShowFlags = BLOCKINFO_FLAG_TOBEASSIGNED;
  RefreshBlockList(NO_RECORD);
}

void CDailyOps::OnAssigned() 
{
  m_PreviousTime = m_SystemTime;

  m_BlockInfoShowFlags = BLOCKINFO_FLAG_ASSIGNED;
  RefreshBlockList(NO_RECORD);
}

void CDailyOps::OnAllBlocks() 
{
  m_PreviousTime = m_SystemTime;

	m_BlockInfoShowFlags = BLOCKINFO_FLAG_DISPLAYALL;
  RefreshBlockList(NO_RECORD);
}

void CDailyOps::OnBlocksProperties() 
{
  m_PreviousTime = m_SystemTime;

  LVITEM LVI;
  int nI = pListCtrlBLOCKLIST->GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);

  if(nI < 0)
  {
    m_SelectedBlockIndex = NO_RECORD;
    m_SelectedBlockRow = NO_RECORD;
  }
  else
  {
    LVI.mask = LVIF_PARAM;
    LVI.iItem = nI;
    LVI.iSubItem = 0;
    pListCtrlBLOCKLIST->GetItem(&LVI);
    m_SelectedBlockIndex = LVI.lParam;
    m_SelectedBlockRow = nI;
  }

  if(m_SelectedBlockIndex == NO_RECORD)
  {
    return;
  }

  CDailyOpsBP dlg(this, &m_BlockInfo[m_SelectedBlockIndex], m_DailyOpsDate);

  dlg.DoModal();
}

//
//  OnChangeScrollbar()
//

void CDailyOps::OnChangeScrollbar() 
{
  m_PreviousTime = m_SystemTime;

  m_FirstBusRowDisplayed = pFlatSBSCROLLBAR->GetValue();
  InvalidateRect(&m_PegboardRect, TRUE);
}

//
//  On ScrollScrollbar()
//

void CDailyOps::OnScrollScrollbar() 
{
  m_PreviousTime = m_SystemTime;

  m_FirstBusRowDisplayed = pFlatSBSCROLLBAR->GetValue();
  InvalidateRect(&m_PegboardRect, TRUE);
}

void CDailyOps::OnDblclkBlocklist(NMHDR* pNMHDR, LRESULT* pResult) 
{
  m_PreviousTime = m_SystemTime;

	OnBlocksProperties();
	
	*pResult = 0;
}

void CDailyOps::OnSetlocation() 
{
  m_PreviousTime = m_SystemTime;

  CDailyOpsBL dlg(this, &m_pPEGBOARD[m_PegboardIndex].locationNODESrecordID);

  if(dlg.DoModal() == IDOK)
  {
    int rcode2;

    DailyOpsBuildRecord(&DAILYOPS, DAILYOPS_FLAG_BUS);
    DAILYOPS.recordFlags = DAILYOPS_FLAG_SETBUSLOCATION;
    DAILYOPS.pertainsToDate = m_DailyOpsDateYYYYMMDD;
//    DAILYOPS.pertainsToTime = m_DailyOpsDate.GetHour() * 3600 + m_DailyOpsDate.GetMinute() * 60 + m_DailyOpsDate.GetSecond();
    DAILYOPS.pertainsToTime = DAILYOPS.recordID;
    DAILYOPS.DAILYOPSrecordID = NO_RECORD;
    DAILYOPS.DRIVERSrecordID = NO_RECORD;
    DAILYOPS.DOPS.Bus.BUSESrecordID = m_pPEGBOARD[m_PegboardIndex].BUSESrecordID;
    DAILYOPS.DOPS.Bus.locationNODESrecordID = m_pPEGBOARD[m_PegboardIndex].locationNODESrecordID;
    rcode2 = btrieve(B_INSERT, TMS_DAILYOPS, &DAILYOPS, &DAILYOPSKey0, 0);
    if(rcode2 == 0)
    {
      m_LastDAILYOPSRecordID = DAILYOPS.recordID;
    }
    InvalidateRect(&m_PegboardRect, TRUE);
  }
}

//
//  ProcessExternalCommands() - Reflect what's happened elsewhere
//
void CDailyOps::ProcessExternalCommands()
{
  m_PreviousTime = m_SystemTime;

  CString s;
  HCURSOR hSaveCursor;
  BOOL bFound;
  long DAILYOPSrecordID;
  int  nI;
  int  pegboardIndexSave;
  int  blockIndexSave;
  int  rcode2;
//
//  Let him know what's going on
//
  s.LoadString(TEXT_381);
  sprintf(tempString, s, m_SystemTime.Format(_T("%I:%M:%S%p")));
  pEditMESSAGES->SetSel(-1, -1);
  pEditMESSAGES->ReplaceSel(tempString);
//
//  Go through the DailyOps recordIDs from m_LastDAILYOPSRecordID to the end
//
  DAILYOPSKey0.recordID = m_LastDAILYOPSRecordID;
  rcode2 = btrieve(B_GETGREATER, TMS_DAILYOPS, &DAILYOPS, &DAILYOPSKey0, 0);
  while(rcode2 == 0)
  {
    DAILYOPSrecordID = DAILYOPS.recordID;
//
//  Only process if we're on the same date
//
    if(DAILYOPS.pertainsToDate == m_DailyOpsDateYYYYMMDD)
    {
//
//  Since DAILYOPS.recordFlags could be ORed with something, we can't just use
//  all switch/cases to get this done.  Instead, "while" our way through parts of it
//
      BOOL bRefreshBlockList = FALSE;
      BOOL bRefreshPegboard = FALSE;
      long recordFlags = DAILYOPS.recordFlags;
//
//  Switch on the record type
//
      switch(DAILYOPS.recordTypeFlag)
      {
//
//  Roster
//
        case DAILYOPS_FLAG_ROSTER:
          switch(DAILYOPS.recordFlags)
          {
            case DAILYOPS_FLAG_ROSTERESTABLISH:
              break;
          }
          break;
//
//  Bus
//
        case DAILYOPS_FLAG_BUS:
          blockIndexSave = m_SelectedBlockIndex;
          pegboardIndexSave = m_PegboardIndex;
//
//  Locate a block in m_BlockInfo
//
          for(m_SelectedBlockIndex = NO_RECORD, nI = 0; nI < m_numBlocksInBlockInfo; nI++)
          {
            if(DAILYOPS.DOPS.Bus.TRIPSrecordID = m_BlockInfo[nI].TRIPSrecordID)
            {
              m_SelectedBlockIndex = nI;
              bFound = TRUE;
              break;
            }
          }
//
//  Locate a bus m_pPEGBOARD
//
          for(m_PegboardIndex = NO_RECORD, nI = 0; nI < m_numBuses; nI++)
          {
            if(DAILYOPS.DOPS.Bus.BUSESrecordID == m_pPEGBOARD[nI].BUSESrecordID)
            {
              m_PegboardIndex = nI;
              bFound = TRUE;
              break;
            }
          }
//
//  Perform a shadow execution of the command.  The NO_RECORD
//  comparisons should never be required, but users are ingenious.
//
          while(recordFlags > 0)
          {
//
//  DAILYOPS_FLAG_BUSASSIGNMENT
//
            if(recordFlags & DAILYOPS_FLAG_BUSASSIGNMENT)
            {
              if(m_SelectedBlockIndex != NO_RECORD)
              {
                m_BlockInfo[m_SelectedBlockIndex].BUSESrecordID = m_pPEGBOARD[m_PegboardIndex].BUSESrecordID;
                m_BlockInfo[m_SelectedBlockIndex].flags = BLOCKINFO_FLAG_ASSIGNED;
                bRefreshBlockList = TRUE;
              }
              if(m_PegboardIndex != NO_RECORD)
              {
                m_pPEGBOARD[m_PegboardIndex].flags = PEGBOARD_FLAG_BUSISASSIGNED;
                m_pPEGBOARD[m_PegboardIndex].DAILYOPSrecordID = DAILYOPS.recordID;
                m_pPEGBOARD[m_PegboardIndex].indexToBlockInfo = m_SelectedBlockIndex;
                bRefreshPegboard = TRUE;
              }
              recordFlags &= ~DAILYOPS_FLAG_BUSASSIGNMENT;
            }
//
//  DAILYOPS_FLAG_BUSSWAP
//
            else if(recordFlags & DAILYOPS_FLAG_BUSSWAP)
            {
              if(m_SelectedBlockIndex != NO_RECORD)
              {
                m_BlockInfo[m_SelectedBlockIndex].flags = BLOCKINFO_FLAG_TOBEASSIGNED | BLOCKINFO_FLAG_BUSRETURN;
                bRefreshBlockList = TRUE;
              }
              if(m_PegboardIndex != NO_RECORD)
              {
                m_pPEGBOARD[m_PegboardIndex].DAILYOPSrecordID = NO_RECORD;
                m_pPEGBOARD[m_PegboardIndex].flags &= ~PEGBOARD_FLAG_BUSISASSIGNED;
                if(DAILYOPS.recordFlags & DAILYOPS_FLAG_BUSMARKEDOUTOFSERVICE)
                {
                  m_pPEGBOARD[m_PegboardIndex].swapReasonIndex = DAILYOPS.DOPS.Bus.swapReasonIndex;
                  m_pPEGBOARD[m_PegboardIndex].flags |= PEGBOARD_FLAG_BUSMARKEDOUTOFSERVICE;
                  m_pPEGBOARD[m_PegboardIndex].flags &= ~PEGBOARD_FLAG_BUSISINSERVICE;
                  recordFlags &= ~DAILYOPS_FLAG_BUSMARKEDOUTOFSERVICE;
                }
                else
                {
                  m_pPEGBOARD[m_PegboardIndex].flags &= ~PEGBOARD_FLAG_BUSISASSIGNED;
                  m_pPEGBOARD[m_PegboardIndex].flags |= PEGBOARD_FLAG_BUSISINSERVICE;
                }
                bRefreshPegboard = TRUE;
              }
              recordFlags &= ~DAILYOPS_FLAG_BUSSWAP;
            }
//
//  DAILYOPS_FLAG_BUSRETURN
//
            else if(recordFlags & DAILYOPS_FLAG_BUSRETURN)
            {
              if(m_SelectedBlockIndex != NO_RECORD)
              {
                m_BlockInfo[m_SelectedBlockIndex].flags &= ~BLOCKINFO_FLAG_ASSIGNED;
                m_BlockInfo[m_SelectedBlockIndex].flags |= BLOCKINFO_FLAG_BUSRETURN;
                m_BlockInfo[m_SelectedBlockIndex].BUSESrecordID = NO_RECORD;
                bRefreshBlockList = TRUE;
              }
              if(m_PegboardIndex != NO_RECORD)
              {
                m_pPEGBOARD[m_PegboardIndex].locationNODESrecordID = m_BlockInfo[m_SelectedBlockIndex].PIGNODESrecordID;
                m_pPEGBOARD[m_PegboardIndex].flags &= ~PEGBOARD_FLAG_BUSISASSIGNED;
                m_pPEGBOARD[m_PegboardIndex].flags |= PEGBOARD_FLAG_BUSISINSERVICE;
                m_pPEGBOARD[m_PegboardIndex].DAILYOPSrecordID = NO_RECORD;
                bRefreshPegboard = TRUE;
              }
              recordFlags &= ~DAILYOPS_FLAG_BUSRETURN;
            }
//
//  DAILYOPS_FLAG_BLOCKDROP
//
            else if(recordFlags & DAILYOPS_FLAG_BLOCKDROP)
            {
              if(m_SelectedBlockIndex != NO_RECORD)
              {
                m_BlockInfo[m_SelectedBlockIndex].flags = BLOCKINFO_FLAG_BLOCKDROP;
                bRefreshBlockList = TRUE;
              }
              recordFlags &= ~DAILYOPS_FLAG_BLOCKDROP;
            }
//
//  DAILYOPS_FLAG_BLOCKUNDROP
//
            else if(recordFlags & DAILYOPS_FLAG_BLOCKUNDROP)
            {
              if(m_SelectedBlockIndex != NO_RECORD)
              {
                m_BlockInfo[m_SelectedBlockIndex].flags &= ~BLOCKINFO_FLAG_BLOCKDROP;
                bRefreshBlockList = TRUE;
              }
              recordFlags &= ~DAILYOPS_FLAG_BLOCKUNDROP;
            }
//
//  DAILYOPS_FLAG_BUSMARKEDOUTOFSERVICE
//
            else if(recordFlags & DAILYOPS_FLAG_BUSMARKEDOUTOFSERVICE)
            {
              if(m_PegboardIndex != NO_RECORD)
              {
                m_pPEGBOARD[m_PegboardIndex].flags &= ~PEGBOARD_FLAG_BUSISINSERVICE;
                m_pPEGBOARD[m_PegboardIndex].flags |= PEGBOARD_FLAG_BUSMARKEDOUTOFSERVICE;
                m_pPEGBOARD[m_PegboardIndex].swapReasonIndex = DAILYOPS.DOPS.Bus.swapReasonIndex;
                bRefreshPegboard = TRUE;
              }
              recordFlags &= ~DAILYOPS_FLAG_BUSMARKEDOUTOFSERVICE;
            }
//
//  DAILYOPS_FLAG_BUSMARKEDINSERVICE
//
            else if(recordFlags & DAILYOPS_FLAG_BUSMARKEDINSERVICE)
            {
              if(m_PegboardIndex != NO_RECORD)
              {
                m_pPEGBOARD[m_PegboardIndex].flags &= ~PEGBOARD_FLAG_BUSMARKEDOUTOFSERVICE;
                m_pPEGBOARD[m_PegboardIndex].flags |= PEGBOARD_FLAG_BUSISINSERVICE;
                m_pPEGBOARD[m_PegboardIndex].swapReasonIndex = NO_RECORD;
                bRefreshPegboard = TRUE;
              }
              recordFlags &= ~DAILYOPS_FLAG_BUSMARKEDINSERVICE;
            }
//
//  DAILYOPS_FLAG_SETBUSLOCATION
//
            else if(recordFlags & DAILYOPS_FLAG_SETBUSLOCATION)
            {
              if(m_PegboardIndex != NO_RECORD)
              {
                m_pPEGBOARD[m_PegboardIndex].locationNODESrecordID = DAILYOPS.DOPS.Bus.locationNODESrecordID;
                bRefreshPegboard = TRUE;
              }
              recordFlags &= ~DAILYOPS_FLAG_SETBUSLOCATION;
            }
//
//  DAILYOPS_FLAG_BUSMARKEDSHORTSHIFT
//
            else if(recordFlags & DAILYOPS_FLAG_BUSMARKEDSHORTSHIFT)
            {
              if(m_PegboardIndex != NO_RECORD)
              {
                m_pPEGBOARD[m_PegboardIndex].flags |= PEGBOARD_FLAG_BUSMARKEDASSHORTSHIFT;
                m_pPEGBOARD[m_PegboardIndex].untilTime = DAILYOPS.DOPS.Bus.untilTime;
                bRefreshPegboard = TRUE;
              }
              recordFlags &= ~DAILYOPS_FLAG_BUSMARKEDSHORTSHIFT;
            }
//
//  DAILYOPS_FLAG_BUSUNMARKEDSHORTSHIFT
//
            else if(recordFlags & DAILYOPS_FLAG_BUSUNMARKEDSHORTSHIFT)
            {
              if(m_PegboardIndex != NO_RECORD)
              {
                m_pPEGBOARD[m_PegboardIndex].flags &= ~PEGBOARD_FLAG_BUSMARKEDASSHORTSHIFT;
                bRefreshPegboard = TRUE;
              }
              recordFlags &= ~DAILYOPS_FLAG_BUSUNMARKEDSHORTSHIFT;
            }
//
//  DAILYOPS_FLAG_BUSMARKEDASCHARTER
//
            else if(recordFlags & DAILYOPS_FLAG_BUSMARKEDASCHARTER)
            {
              if(m_PegboardIndex != NO_RECORD)
              {
                m_pPEGBOARD[m_PegboardIndex].flags |= PEGBOARD_FLAG_BUSMARKEDASCHARTER;
                bRefreshPegboard = TRUE;
              }
              recordFlags &= ~DAILYOPS_FLAG_BUSMARKEDASCHARTER;
            }
//
//  DAILYOPS_FLAG_BUSUNMARKEDASCHARTER
//
            else if(recordFlags & DAILYOPS_FLAG_BUSUNMARKEDASCHARTER)
            {
              if(m_PegboardIndex != NO_RECORD)
              {
                m_pPEGBOARD[m_PegboardIndex].flags &= ~PEGBOARD_FLAG_BUSMARKEDASCHARTER;
                bRefreshPegboard = TRUE;
              }
              recordFlags &= ~DAILYOPS_FLAG_BUSUNMARKEDASCHARTER;
            }
//
//  DAILYOPS_FLAG_BUSMARKEDASSIGHTSEEING
//
            else if(recordFlags & DAILYOPS_FLAG_BUSMARKEDASSIGHTSEEING)
            {
              if(m_PegboardIndex != NO_RECORD)
              {
                m_pPEGBOARD[m_PegboardIndex].flags |= PEGBOARD_FLAG_BUSMARKEDASSIGHTSEEING;
                bRefreshPegboard = TRUE;
              }
              recordFlags &= ~DAILYOPS_FLAG_BUSMARKEDASSIGHTSEEING;
            }
//
//  DAILYOPS_FLAG_BUSUNMARKEDASSIGHTSEEING
//
            else if(recordFlags & DAILYOPS_FLAG_BUSUNMARKEDASSIGHTSEEING)
            {
              if(m_PegboardIndex != NO_RECORD)
              {
                m_pPEGBOARD[m_PegboardIndex].flags &= ~PEGBOARD_FLAG_BUSMARKEDASSIGHTSEEING;
                bRefreshPegboard = TRUE;
              }
              recordFlags &= ~DAILYOPS_FLAG_BUSUNMARKEDASSIGHTSEEING;
            }
//
//  Fall through - nothing done on this loop so exit
//
            else
            {
              break;
            }
          } // while recordFlags
//
//  Fix the display
//
          if(bRefreshPegboard)
          {
            InvalidateRect(&m_PegboardRect, TRUE);
          }
          if(bRefreshBlockList)
          {
            RefreshBlockList(m_SelectedBlockIndex);
          }
//
//  Restore the indexes into the blocks and the pegboard
//
          m_PegboardIndex = pegboardIndexSave;
          m_SelectedBlockIndex = blockIndexSave;
          break;
//
//  Operator
//
        case DAILYOPS_FLAG_OPERATOR:
          switch(DAILYOPS.recordFlags)
          {
            case DAILYOPS_FLAG_OPERATORCHECKIN:
              break;

            case DAILYOPS_FLAG_OPERATORCHECKOUT:
              break;

            case DAILYOPS_FLAG_OPERATORUNCHECK:
              break;

            case DAILYOPS_FLAG_OPERATORDEASSIGN:
              hSaveCursor = ::SetCursor(hCursorWait);
              PopulateOpenWorkMonitor(NO_RECORD, FALSE, TRUE);
              PopulateVehicleAssignment();
              ::SetCursor(hSaveCursor);
              break;
 
            case DAILYOPS_FLAG_OVERTIME:
              break;

            case DAILYOPS_FLAG_EXTRATRIP:
              break;
          }
          break;
//
//  Absence
//
        case DAILYOPS_FLAG_ABSENCE:
          switch(DAILYOPS.recordFlags)
          {
            case DAILYOPS_FLAG_ABSENCEREGISTER:
            case DAILYOPS_FLAG_ABSENCEUNREGISTER:
              hSaveCursor = ::SetCursor(hCursorWait);
              m_numAbsent = RefreshAbsenceList(m_DailyOpsDateYYYYMMDD, m_AbsentList);
              PopulateOpenWorkMonitor(NO_RECORD, FALSE, TRUE);
              PopulateVehicleAssignment();
              ::SetCursor(hSaveCursor);
              break;
          }
          break;
//
//  Open work
//
        case DAILYOPS_FLAG_OPENWORK:
          OPENWORKDef *pOW;

          for(bFound = FALSE, nI = 0; nI < m_numInOpenWork;nI++)
          {
            GetOWMPointer(nI, &pOW);
            if(DAILYOPS.DOPS.OpenWork.RUNSrecordID == pOW->RUNSrecordID)
            {
              if(pOW->cutAsRuntype == RUNSPLITRUNTYPE)
              {
                if(DAILYOPS.DOPS.OpenWork.splitStartTime != pOW->onTime ||
                      DAILYOPS.DOPS.OpenWork.splitStartNODESrecordID != pOW->onNODESrecordID)
                {
                  continue;
                }
              }
              bFound = TRUE;
              break;
            }
          }
          if(bFound)
          {
            switch(DAILYOPS.recordFlags)
            {
//
//  DAILYOPS_FLAG_OPENWORKASSIGN
//
              case DAILYOPS_FLAG_OPENWORKASSIGN:
                strcpy(tempString, BuildOperatorString(DAILYOPS.DRIVERSrecordID));
               
                LVITEM  LVI;
  
                LVI.mask = LVIF_TEXT;
                LVI.iItem = nI;
                LVI.iSubItem = 8;
                LVI.pszText = tempString;
                LVI.iItem = pListCtrlOPENWORKLIST->SetItem(&LVI);
                m_OpenWork[nI].DRIVERSrecordID = DAILYOPS.DRIVERSrecordID;
                break;
//
//  DAILYOPS_FLAG_OPENWORKCLEAR
//
              case DAILYOPS_FLAG_OPENWORKCLEAR:
                pListCtrlOPENWORKLIST->SetItemText(nI, 8, "");
                m_OpenWork[nI].DRIVERSrecordID = NO_RECORD;
                break;
            }
          }
          break;
//
//  Service
//
        case DAILYOPS_FLAG_SERVICE:
          switch(DAILYOPS.recordFlags)
          {
            case DAILYOPS_FLAG_SERVICEOVERRIDE:
              break;

            case DAILYOPS_FLAG_SERVICERESTORE:
              break;

            case DAILYOPS_FLAG_MAKEALLWORKOPEN:
              break;
          }
          break;
      }
    }
//
//  Get the next record
//
    m_LastDAILYOPSRecordID = DAILYOPSrecordID;
    DAILYOPSKey0.recordID = m_LastDAILYOPSRecordID;
    rcode2 = btrieve(B_GETGREATER, TMS_DAILYOPS, &DAILYOPS, &DAILYOPSKey0, 0);
  }
//
//  Let him know we're done
//
  s.LoadString(TEXT_382);
  sprintf(tempString, s, m_SystemTime.Format(_T("%I:%M:%S%p")));
  pEditMESSAGES->SetSel(-1, -1);
  pEditMESSAGES->ReplaceSel(tempString);
}

void CDailyOps::OnFunctions() 
{
  m_PreviousTime = m_SystemTime;

  DAILYOPSFUPassedDataDef PassedData;
  long saveWeekNumber = m_RosterWeek;
  int  rcode2;

  PassedData.date = m_DailyOpsDateYYYYMMDD;
  PassedData.flags = 0;
  PassedData.numInBlockInfo = m_numBlocksInBlockInfo;
  PassedData.BlockInfo = m_BlockInfo;
  PassedData.numInOpenWork = m_numInOpenWork;
  PassedData.OpenWork = m_OpenWork;

  CDailyOpsFU dlg(this, &PassedData);

  dlg.DoModal();

//
//  Cancel or no action required
//
  if(PassedData.flags == 0)
  {
    return;
  }
//
//  Date change
//
  else if(PassedData.flags & DAILYOPSFU_FLAG_CHANGEDATE)
  {
    char szMostRecent[16];
    long date;
    long yyyy, mm, dd;
    long year, month, day;

    date = PassedData.date;

    GetYMD(date, &yyyy, &mm, &dd);
    
    CTime workDate(yyyy, mm, dd, 0, 0, 0);
 	  CString strTime = workDate.Format(_T("%A, %B %d, %Y"));

    if(m_MostRecentTemplate != NO_RECORD)
    {
      GetYMD(m_MostRecentTemplate, &year, &month, &day);
      sprintf(szMostRecent, "%02ld-%s-%04ld", day, szMonth[month - 1], year);
    }
//
//  Refresh the absences
//
    m_numAbsent = RefreshAbsenceList(date, m_AbsentList);
//
//  Look to see if this is a holiday and if it is, ensure that service is running
//
    LoadString(hInst, TEXT_304, szFormatString, SZFORMATSTRING_LENGTH);
    sprintf(szarString, szFormatString, strTime, szMostRecent);
    pButtonOPENWORKMONITOR->SetWindowText(szarString);
    m_DailyOpsDate = workDate;
    m_DailyOpsDateYYYYMMDD = yyyy * 10000 + mm * 100 + dd;
    PopulateOpenWorkMonitor(NO_RECORD, TRUE, TRUE);

    LoadString(hInst, TEXT_376, szFormatString, SZFORMATSTRING_LENGTH);
    sprintf(szarString, szFormatString, strTime);
    pButtonVEHICLEASSIGNMENT->SetWindowText(szarString);
    PopulateVehicleAssignment();

 	  strTime = workDate.Format(_T("%a, %b %d, %Y"));
    sprintf(tempString, "%s\t\"Today\" changed to %s\r\n",
          m_SystemTime.Format(_T("%I:%M:%S%p")), strTime);
    pEditMESSAGES->SetSel(-1, -1);
    pEditMESSAGES->ReplaceSel(tempString);

    SERVICESKey0.recordID = m_SERVICESrecordIDInEffect;
    rcode2 = btrieve(B_GETEQUAL, TMS_SERVICES, &SERVICES, &SERVICESKey0, 0);
    if(rcode2 != 0)
    {
      strcpy(szarString, "?");
    }
    else
    {
      strncpy(szarString, SERVICES.name, SERVICES_NAME_LENGTH);
      trim(szarString, SERVICES_NAME_LENGTH);
    }
    sprintf(tempString, "%s\tService in effect: %s\r\n",
          m_SystemTime.Format(_T("%I:%M:%S%p")), szarString);
    pEditMESSAGES->SetSel(-1, -1);
    pEditMESSAGES->ReplaceSel(tempString);
  }

//
//  Driver to consider change
//
  else if(PassedData.flags & DAILYOPSFU_FLAG_CHANGECONSIDER)
  {
    SetupDriverComboBox();
    sprintf(tempString, "%s\tDrivers to consider changed\r\n",
          m_SystemTime.Format(_T("%I:%M:%S%p")));
    pEditMESSAGES->SetSel(-1, -1);
    pEditMESSAGES->ReplaceSel(tempString);

  }
//
//  Roster week change
//
  else if(PassedData.flags & DAILYOPSFU_FLAG_CHANGEWEEK)
  {
    if(m_RosterWeek == saveWeekNumber)  // OK'd, but no change
    {
      return;
    }
//
//  Let him know
//
    sprintf(tempString, "%s\tRoster week in effect: %ld\r\n",
          m_SystemTime.Format(_T("%I:%M:%S%p")), m_RosterWeek + 1);
    pEditMESSAGES->SetSel(-1, -1);
    pEditMESSAGES->ReplaceSel(tempString);
//
//  Refresh the displays
//
    m_numAbsent = RefreshAbsenceList(m_DailyOpsDateYYYYMMDD, m_AbsentList);
    PopulateOpenWorkMonitor(NO_RECORD, TRUE, TRUE);
    PopulateVehicleAssignment();
  }
//
//  Color change
//
  else if(PassedData.flags & DAILYOPSFU_FLAG_CHANGECOLOR)
  {

    m_Pens[DO_AVAILABLE].DeleteObject();
    m_Pens[DO_INUSE].DeleteObject();
    m_Pens[DO_OUTOFSERVICE].DeleteObject();
    m_Pens[DO_SHORTSHIFT].DeleteObject();
    m_Pens[DO_CHARTERRESERVE].DeleteObject();
    m_Pens[DO_SIGHTSEEINGRESERVE].DeleteObject();
    m_Brushes[DO_AVAILABLE].DeleteObject();
    m_Brushes[DO_INUSE].DeleteObject();
    m_Brushes[DO_OUTOFSERVICE].DeleteObject();
    m_Brushes[DO_SHORTSHIFT].DeleteObject();
    m_Brushes[DO_CHARTERRESERVE].DeleteObject();
    m_Brushes[DO_SIGHTSEEINGRESERVE].DeleteObject();

    m_Pens[DO_AVAILABLE].CreatePen(         PS_SOLID, 1,     m_PBCOLORS.crAvailable);
    m_Pens[DO_INUSE].CreatePen(             PS_SOLID, 1,     m_PBCOLORS.crAssigned); 
    m_Pens[DO_OUTOFSERVICE].CreatePen(      PS_SOLID, 1,     m_PBCOLORS.crOutOfService);
    m_Pens[DO_SHORTSHIFT].CreatePen(        PS_SOLID, 1,     m_PBCOLORS.crShortShift);
    m_Pens[DO_CHARTERRESERVE].CreatePen(    PS_SOLID, 1,     m_PBCOLORS.crCharterReserve);
    m_Pens[DO_SIGHTSEEINGRESERVE].CreatePen(PS_SOLID, 1,     m_PBCOLORS.crSightseeingReserve);
  
    LOGBRUSH logBrush;
    
    logBrush.lbStyle = BS_SOLID;
    logBrush.lbHatch = 0;

    logBrush.lbColor = m_PBCOLORS.crAvailable;
    m_Brushes[DO_AVAILABLE].CreateBrushIndirect(&logBrush);
    logBrush.lbColor = m_PBCOLORS.crAssigned;
    m_Brushes[DO_INUSE].CreateBrushIndirect(&logBrush);
    logBrush.lbColor = m_PBCOLORS.crOutOfService;
    m_Brushes[DO_OUTOFSERVICE].CreateBrushIndirect(&logBrush);
    logBrush.lbColor = m_PBCOLORS.crShortShift;
    m_Brushes[DO_SHORTSHIFT].CreateBrushIndirect(&logBrush);
    logBrush.lbColor = m_PBCOLORS.crCharterReserve;
    m_Brushes[DO_CHARTERRESERVE].CreateBrushIndirect(&logBrush);
    logBrush.lbColor = m_PBCOLORS.crSightseeingReserve;
    m_Brushes[DO_SIGHTSEEINGRESERVE].CreateBrushIndirect(&logBrush);

    InvalidateRect(&m_PegboardRect, TRUE);
  }
}

//
//  OnOperatorassignments() - Clear every rostered operator for today
//
void CDailyOps::OnOperatorassignments() 
{
  m_PreviousTime = m_SystemTime;

  BOOL bFound;
  int  rcode2;

  CString s;

  if(m_bMessageDisplay)
  {
    s.LoadString(ERROR_340);  // Are you sure?
    MessageBeep(MB_ICONQUESTION);
    if(MessageBox(s, TMS, MB_ICONQUESTION | MB_YESNO) != IDYES)
    {
      return;
    }
    s.LoadString(ERROR_366);  // Are you really sure?
    MessageBeep(MB_ICONQUESTION);
    if(MessageBox(s, TMS, MB_ICONQUESTION | MB_YESNO) != IDYES)
    {
      return;
    }
  }
//
//  Cycle through the roster
//
  rcode2 = btrieve(B_GETFIRST, TMS_ROSTER, &ROSTER, &ROSTERKey0, 0);
  while(rcode2 == 0)
  {
//
//  Need a driver
//
    if(ROSTER.DRIVERSrecordID != NO_RECORD)
    {
//
//  Need a run
//
      if(ROSTER.WEEK[m_RosterWeek].RUNSrecordIDs[m_Today] != NO_RECORD)
      {
//
//  No point clearing if we've already cleared him
//
        DAILYOPSKey1.recordTypeFlag = DAILYOPS_FLAG_OPERATOR;
        DAILYOPSKey1.pertainsToDate = m_DailyOpsDateYYYYMMDD;
        DAILYOPSKey1.pertainsToTime = 0;
        DAILYOPSKey1.recordFlags = 0;
        bFound = FALSE;
        rcode2 = btrieve(B_GETGREATEROREQUAL, TMS_DAILYOPS, &DAILYOPS, &DAILYOPSKey1, 1);
        while(rcode2 == 0 &&
              (DAILYOPS.recordTypeFlag & DAILYOPS_FLAG_OPERATOR) &&
               DAILYOPS.pertainsToDate == m_DailyOpsDateYYYYMMDD)
        {
          if((DAILYOPS.recordFlags & DAILYOPS_FLAG_OPERATORDEASSIGN) &&
                !ANegatedRecord(DAILYOPS.recordID, 1))
          {
            if(DAILYOPS.DRIVERSrecordID == ROSTER.DRIVERSrecordID)
            {
              bFound = TRUE;
              break;
            }
          }
          rcode2 = btrieve(B_GETNEXT, TMS_DAILYOPS, &DAILYOPS, &DAILYOPSKey1, 1);
        }
//
//  Record the "clear assignment"
//
        if(!bFound)
        {
          DailyOpsBuildRecord(&DAILYOPS, DAILYOPS_FLAG_OPERATOR);

          DAILYOPS.recordFlags = DAILYOPS_FLAG_OPERATORDEASSIGN;
          DAILYOPS.pertainsToDate = m_DailyOpsDateYYYYMMDD;
          DAILYOPS.pertainsToTime = 0;
          DAILYOPS.DAILYOPSrecordID = NO_RECORD;
          DAILYOPS.DRIVERSrecordID = ROSTER.DRIVERSrecordID;
//
//  Build the rest of the record
//
          DAILYOPS.DOPS.Absence.untilDate = m_DailyOpsDateYYYYMMDD;
          DAILYOPS.DOPS.Absence.untilTime = T1159P;
          DAILYOPS.DOPS.Absence.reasonIndex = NO_RECORD;
//
//  Time used/lost
//
          DAILYOPS.DOPS.Absence.timeLost = 0;
//
//  Add the record
//
          rcode2 = btrieve(B_INSERT, TMS_DAILYOPS, &DAILYOPS, &DAILYOPSKey0, 0);
          if(rcode2 == 0)
          {
            m_LastDAILYOPSRecordID = DAILYOPS.recordID;
          }
        }
      }
    }
    rcode2 = btrieve(B_GETNEXT, TMS_ROSTER, &ROSTER, &ROSTERKey0, 0);
  }
//
//  Update the Open Work monitor and the vehicle assignment list
//
  PopulateOpenWorkMonitor(NO_RECORD, FALSE, TRUE);
  PopulateVehicleAssignment();
}

//
//  DetermineServiceDay() - Return the service date of "date"
//
long CDailyOps::DetermineServiceDay(long date, BOOL bNeedDateWithService, int* today, char* pszHolidayName)
{
  BOOL bServiceFromRoster;
  int  rcode2;
  int  nI;

  DAILYOPSKey1.recordTypeFlag = (char)DAILYOPS_FLAG_DATE;
  DAILYOPSKey1.pertainsToDate = date;
  DAILYOPSKey1.pertainsToTime = m_LastDAILYOPSRecordID + 1;
  DAILYOPSKey1.recordFlags = 0;
  bServiceFromRoster = TRUE;
  rcode2 = btrieve(B_GETLESSTHAN, TMS_DAILYOPS, &DAILYOPS, &DAILYOPSKey1, 1);
  while(rcode2 == 0 &&
        (DAILYOPS.recordTypeFlag & DAILYOPS_FLAG_DATE) &&
         DAILYOPS.pertainsToDate == date)
  {
    if(DAILYOPS.recordFlags & DAILYOPS_FLAG_DATESET)
    {
      bServiceFromRoster = ANegatedRecord(DAILYOPS.recordID, 1);
      if(!bServiceFromRoster)
      {
        break;
      }
    }
    rcode2 = btrieve(B_GETPREVIOUS, TMS_DAILYOPS, &DAILYOPS, &DAILYOPSKey1, 1);
  }
//
//  Get the service from the roster
//
  strcpy(pszHolidayName, "not a holiday");
  if(bServiceFromRoster)
  {
//
//  Figure out what day of the week we are
//
    int  indexToRoster[7] = {6, 0, 1, 2, 3, 4, 5};
    long yyyy, mm, dd;

    GetYMD(date, &yyyy, &mm, &dd);
    
    CTime workDate(yyyy, mm, dd, 0, 0, 0);
  
    *today = indexToRoster[workDate.GetDayOfWeek() - 1];
    return(ROSTERPARMS.serviceDays[*today]);
  }
//
//  Get the service from the DAILYOPS record
//
  else
  {
    if(DAILYOPS.DOPS.Date.SERVICESrecordID == NO_RECORD)
    {
      if(bNeedDateWithService)
      {
        long date = NO_RECORD;

        MessageBeep(MB_ICONINFORMATION);
//        MessageBox("The date selected is a holiday with no service\n\nPlease select another date",
//              TMS, MB_ICONINFORMATION | MB_OK);

        while(date == NO_RECORD)
        {
          CPickDate dlg(NULL, &date);

          dlg.DoModal();
        }
        return(DetermineServiceDay(date, TRUE, today, pszHolidayName));
      }
      else
      {
        *today = NO_RECORD;
        return(NO_RECORD);
      }
    }
    else
    {
      strncpy(pszHolidayName, DAILYOPS.DOPS.Date.name, DAILYOPS_DATENAME_LENGTH);
      trim(pszHolidayName, DAILYOPS_DATENAME_LENGTH);
      for(*today = NO_RECORD, nI = 0; nI < 7; nI++)
      {
        if(DAILYOPS.DOPS.Date.SERVICESrecordID == ROSTERPARMS.serviceDays[nI])
        {
          *today = nI;
          break;
        }
      }
      return(DAILYOPS.DOPS.Date.SERVICESrecordID);
    }
  }
}

void CDailyOps::OnMarkasshortshift() 
{
  m_PreviousTime = m_SystemTime;

  if(m_PegboardIndex == NO_RECORD)
  {
    return;
  }

  int rcode2;
//
//  If it's marked as a short shift, unmark it
//
  if(m_pPEGBOARD[m_PegboardIndex].flags & PEGBOARD_FLAG_BUSMARKEDASSHORTSHIFT)
  {
    m_pPEGBOARD[m_PegboardIndex].flags &= ~PEGBOARD_FLAG_BUSMARKEDASSHORTSHIFT;
//
//  Tell the audit trail that this one's no longer a short shift
//
//  Database verify: 11-Jan-07
//
    DailyOpsBuildRecord(&DAILYOPS, DAILYOPS_FLAG_BUS);
    DAILYOPS.pertainsToDate = m_DailyOpsDateYYYYMMDD;
//    DAILYOPS.pertainsToTime = m_DailyOpsDate.GetHour() * 3600 + m_DailyOpsDate.GetMinute() * 60 + m_DailyOpsDate.GetSecond();
    DAILYOPS.pertainsToTime = DAILYOPS.recordID;
    DAILYOPS.recordFlags = DAILYOPS_FLAG_BUSUNMARKEDSHORTSHIFT;
    DAILYOPS.DOPS.Bus.BUSESrecordID = m_pPEGBOARD[m_PegboardIndex].BUSESrecordID;
    rcode2 = btrieve(B_INSERT, TMS_DAILYOPS, &DAILYOPS, &DAILYOPSKey0, 0);
    if(rcode2 == 0)
    {
      m_LastDAILYOPSRecordID = DAILYOPS.recordID;
    }
  }
//
//  It's not already marked - mark it now
//
  else
  {
    long untilTime;

    CUntilTime dlg(this, &untilTime);

    if(dlg.DoModal() == IDOK)
    {
      m_pPEGBOARD[m_PegboardIndex].flags |= PEGBOARD_FLAG_BUSMARKEDASSHORTSHIFT;
      m_pPEGBOARD[m_PegboardIndex].untilTime = untilTime;
//
//  Tell the audit trail that this one's marked
//
//  Database verify: 11-Jan-07
//
      DailyOpsBuildRecord(&DAILYOPS, DAILYOPS_FLAG_BUS);
      DAILYOPS.pertainsToDate = m_DailyOpsDateYYYYMMDD;
//      DAILYOPS.pertainsToTime = m_DailyOpsDate.GetHour() * 3600 + m_DailyOpsDate.GetMinute() * 60 + m_DailyOpsDate.GetSecond();
      DAILYOPS.pertainsToTime = DAILYOPS.recordID;
      DAILYOPS.recordFlags = DAILYOPS_FLAG_BUSMARKEDSHORTSHIFT;
      DAILYOPS.DOPS.Bus.BUSESrecordID = m_pPEGBOARD[m_PegboardIndex].BUSESrecordID;
      DAILYOPS.DOPS.Bus.untilTime = untilTime;
      rcode2 = btrieve(B_INSERT, TMS_DAILYOPS, &DAILYOPS, &DAILYOPSKey0, 0);
      if(rcode2 == 0)
      {
        m_LastDAILYOPSRecordID = DAILYOPS.recordID;
      }
    }
  }
//
//  Fix the display
//
  InvalidateRect(&m_PegboardRect, TRUE);
}

//
//  Cleanup() - Free allocated memory and release resources
//
void CDailyOps::Cleanup()
{
  int nI;

  TMSHeapFree(m_pPEGBOARD);
  TMSHeapFree(m_pRData);
 
  for(nI = 0; nI < DO_LASTPEN; nI++)
  {
    m_Pens[nI].Detach();
    m_Pens[nI].DeleteObject();
  }

  if(m_IncoreServices.pRecordIDs != NULL)
  {
    TMSHeapFree(m_IncoreServices.pRecordIDs);
  } 
  if(m_pIncoreRuns)
  {
    for(nI = 0; nI < m_IncoreServices.numRecords; nI++)
    {
      if(m_pIncoreRuns[nI].pRuns)
      {
        TMSHeapFree(m_pIncoreRuns[nI].pRuns);
      }
    }
  }
}

void CDailyOps::OnMarkascharter() 
{
  m_PreviousTime = m_SystemTime;

  if(m_PegboardIndex == NO_RECORD)
  {
    return;
  }

  int rcode2;
//
//  If it's marked as a charter, unmark it
//
  if(m_pPEGBOARD[m_PegboardIndex].flags & PEGBOARD_FLAG_BUSMARKEDASCHARTER)
  {
    m_pPEGBOARD[m_PegboardIndex].flags &= ~PEGBOARD_FLAG_BUSMARKEDASCHARTER;
//
//  Tell the audit trail that this one's no longer a short shift
//
//  Database verify: 11-Jan-07
//
    DailyOpsBuildRecord(&DAILYOPS, DAILYOPS_FLAG_BUS);
    DAILYOPS.pertainsToDate = m_DailyOpsDateYYYYMMDD;
//    DAILYOPS.pertainsToTime = m_DailyOpsDate.GetHour() * 3600 + m_DailyOpsDate.GetMinute() * 60 + m_DailyOpsDate.GetSecond();
    DAILYOPS.pertainsToTime = DAILYOPS.recordID;
    DAILYOPS.recordFlags = DAILYOPS_FLAG_BUSUNMARKEDASCHARTER;
    DAILYOPS.DOPS.Bus.BUSESrecordID = m_pPEGBOARD[m_PegboardIndex].BUSESrecordID;
    rcode2 = btrieve(B_INSERT, TMS_DAILYOPS, &DAILYOPS, &DAILYOPSKey0, 0);
    if(rcode2 == 0)
    {
      m_LastDAILYOPSRecordID = DAILYOPS.recordID;
    }
  }
//
//  It's not already marked - mark it now
//
  else
  {
    m_pPEGBOARD[m_PegboardIndex].flags |= PEGBOARD_FLAG_BUSMARKEDASCHARTER;
//
//  Tell the audit trail that this one's marked
//
//  Database verify: 11-Jan-07
//
    DailyOpsBuildRecord(&DAILYOPS, DAILYOPS_FLAG_BUS);
    DAILYOPS.pertainsToDate = m_DailyOpsDateYYYYMMDD;
//    DAILYOPS.pertainsToTime = m_DailyOpsDate.GetHour() * 3600 + m_DailyOpsDate.GetMinute() * 60 + m_DailyOpsDate.GetSecond();
    DAILYOPS.pertainsToTime = DAILYOPS.recordID;
    DAILYOPS.recordFlags = DAILYOPS_FLAG_BUSMARKEDASCHARTER;
    DAILYOPS.DOPS.Bus.BUSESrecordID = m_pPEGBOARD[m_PegboardIndex].BUSESrecordID;
    rcode2 = btrieve(B_INSERT, TMS_DAILYOPS, &DAILYOPS, &DAILYOPSKey0, 0);
    if(rcode2 == 0)
    {
      m_LastDAILYOPSRecordID = DAILYOPS.recordID;
    }
  }
//
//  Fix the display
//
  InvalidateRect(&m_PegboardRect, TRUE);
}

void CDailyOps::OnMarkassightseeing() 
{
  m_PreviousTime = m_SystemTime;

  if(m_PegboardIndex == NO_RECORD)
  {
    return;
  }

  int rcode2;
//
//  If it's marked as sight-seeing, unmark it
//
  if(m_pPEGBOARD[m_PegboardIndex].flags & PEGBOARD_FLAG_BUSMARKEDASSIGHTSEEING)
  {
    m_pPEGBOARD[m_PegboardIndex].flags &= ~PEGBOARD_FLAG_BUSMARKEDASSIGHTSEEING;
//
//  Tell the audit trail that this one's no longer a short shift
//
//  Database verify: 11-Jan-07
//
    DailyOpsBuildRecord(&DAILYOPS, DAILYOPS_FLAG_BUS);
    DAILYOPS.pertainsToDate = m_DailyOpsDateYYYYMMDD;
//    DAILYOPS.pertainsToTime = m_DailyOpsDate.GetHour() * 3600 + m_DailyOpsDate.GetMinute() * 60 + m_DailyOpsDate.GetSecond();
    DAILYOPS.pertainsToTime = DAILYOPS.recordID;
    DAILYOPS.recordFlags = DAILYOPS_FLAG_BUSUNMARKEDASSIGHTSEEING;
    DAILYOPS.DOPS.Bus.BUSESrecordID = m_pPEGBOARD[m_PegboardIndex].BUSESrecordID;
    rcode2 = btrieve(B_INSERT, TMS_DAILYOPS, &DAILYOPS, &DAILYOPSKey0, 0);
    if(rcode2 == 0)
    {
      m_LastDAILYOPSRecordID = DAILYOPS.recordID;
    }
  }
//
//  It's not already marked - mark it now
//
  else
  {
    m_pPEGBOARD[m_PegboardIndex].flags |= PEGBOARD_FLAG_BUSMARKEDASSIGHTSEEING;
//
//  Tell the audit trail that this one's marked
//
//  Database verify: 11-Jan-07
//
    DailyOpsBuildRecord(&DAILYOPS, DAILYOPS_FLAG_BUS);
    DAILYOPS.pertainsToDate = m_DailyOpsDateYYYYMMDD;
//    DAILYOPS.pertainsToTime = m_DailyOpsDate.GetHour() * 3600 + m_DailyOpsDate.GetMinute() * 60 + m_DailyOpsDate.GetSecond();
    DAILYOPS.pertainsToTime = DAILYOPS.recordID;
    DAILYOPS.recordFlags = DAILYOPS_FLAG_BUSMARKEDASSIGHTSEEING;
    DAILYOPS.DOPS.Bus.BUSESrecordID = m_pPEGBOARD[m_PegboardIndex].BUSESrecordID;
    rcode2 = btrieve(B_INSERT, TMS_DAILYOPS, &DAILYOPS, &DAILYOPSKey0, 0);
    if(rcode2 == 0)
    {
      m_LastDAILYOPSRecordID = DAILYOPS.recordID;
    }
  }
//
//  Fix the display
//
  InvalidateRect(&m_PegboardRect, TRUE);
}

//
//  Manual Runsplit
//
void CDailyOps::OnRunsplit() 
{
  m_PreviousTime = m_SystemTime;

  OPENWORKDef *pOW;
  GetOWMPointer(m_SelectedOWRow, &pOW);

  RunSplitDef PassedData;

  PassedData.RUNSrecordID =  pOW->RUNSrecordID;
  PassedData.SERVICESrecordIDInEffect = m_SERVICESrecordIDInEffect;
  PassedData.pertainsToDate = m_DailyOpsDateYYYYMMDD;
  PassedData.fromTime = NO_TIME;
  PassedData.toTime = NO_TIME;
  PassedData.flags = (pOW->flags & RDATA_FLAG_CREWONLY ? RUNSPLIT_FLAGS_CREWONLY : 0);

	CDailyOpsRS dlg(this, &PassedData);

  if(dlg.DoModal() == IDOK)
  {
    PopulateOpenWorkMonitor(NO_RECORD, FALSE, TRUE);
  }
}

void CDailyOps::OnRecost() 
{
  m_PreviousTime = m_SystemTime;

  OPENWORKDef *pOW;
  GetOWMPointer(m_SelectedOWRow, &pOW);

  CRecost dlg(NULL, pOW, m_DailyOpsDateYYYYMMDD);

  if(dlg.DoModal() == IDOK)
  {
    LVITEM LVI;

    LVI.mask = LVIF_TEXT;
    LVI.iItem = m_SelectedOWRow;
    LVI.iSubItem = 7;
    strcpy(tempString, chhmm(pOW->pay));

    if(pOW->propertyFlags & OPENWORK_PROPERTYFLAG_RUNWASRECOSTED)
    {
      strcat(tempString, " *");
    }
    LVI.pszText = tempString;
    LVI.iItem = pListCtrlOPENWORKLIST->SetItem(&LVI);
  }
}

//
//  OnCheckin()
//
void CDailyOps::OnCheckin() 
{
  m_PreviousTime = m_SystemTime;

  CString s;
  int rcode2;
  int nI = pComboBoxOPERATOR->GetCurSel();

  if(nI == CB_ERR)
  {
    return;
  }

  long DRIVERSrecordID = pComboBoxOPERATOR->GetItemData(nI);

  DailyOpsBuildRecord(&DAILYOPS, DAILYOPS_FLAG_OPERATOR);
  DAILYOPS.pertainsToDate = m_DailyOpsDateYYYYMMDD;
  DAILYOPS.pertainsToTime = m_DailyOpsDate.GetHour() * 3600 + m_DailyOpsDate.GetMinute() * 60 + m_DailyOpsDate.GetSecond();
  DAILYOPS.DRIVERSrecordID = DRIVERSrecordID;
  DAILYOPS.DAILYOPSrecordID = NO_RECORD;
  DAILYOPS.recordFlags = DAILYOPS_FLAG_OPERATORCHECKIN;
  rcode2 = btrieve(B_INSERT, TMS_DAILYOPS, &DAILYOPS, &DAILYOPSKey0, 0);
  if(rcode2 == 0)
  {
    m_LastDAILYOPSRecordID = DAILYOPS.recordID;
  }
  s.LoadString(TEXT_312);
  sprintf(tempString, s, m_SystemTime.Format(_T("%I:%M:%S%p")), BuildOperatorString(DRIVERSrecordID));
  pEditMESSAGES->SetSel(-1, -1);
  pEditMESSAGES->ReplaceSel(tempString);

  pComboBoxOPERATOR->SetFocus();
}

//
//  OnCheckout()
//
void CDailyOps::OnCheckout() 
{
  m_PreviousTime = m_SystemTime;

  CString s;
  int rcode2;
  int nI = pComboBoxOPERATOR->GetCurSel();

  if(nI == CB_ERR)
  {
    return;
  }

  long DRIVERSrecordID = pComboBoxOPERATOR->GetItemData(nI);

  DailyOpsBuildRecord(&DAILYOPS, DAILYOPS_FLAG_OPERATOR);
  DAILYOPS.pertainsToDate = m_DailyOpsDateYYYYMMDD;
   DAILYOPS.pertainsToTime = m_DailyOpsDate.GetHour() * 3600 + m_DailyOpsDate.GetMinute() * 60 + m_DailyOpsDate.GetSecond();
  DAILYOPS.DRIVERSrecordID = DRIVERSrecordID;
  DAILYOPS.DAILYOPSrecordID = NO_RECORD;
  DAILYOPS.recordFlags = DAILYOPS_FLAG_OPERATORCHECKOUT;
  rcode2 = btrieve(B_INSERT, TMS_DAILYOPS, &DAILYOPS, &DAILYOPSKey0, 0);
  if(rcode2 == 0)
  {
    m_LastDAILYOPSRecordID = DAILYOPS.recordID;
  }
  s.LoadString(TEXT_313);
  sprintf(tempString, s, m_SystemTime.Format(_T("%I:%M:%S%p")), BuildOperatorString(DRIVERSrecordID));
  pEditMESSAGES->SetSel(-1, -1);
  pEditMESSAGES->ReplaceSel(tempString);

  pComboBoxOPERATOR->SetFocus();
}

//
//  OnDeassign() - Run deassignment
//
void CDailyOps::OnDeassign() 
{
  m_PreviousTime = m_SystemTime;

  CString s;
  int rcode2;
  int nI = pComboBoxOPERATOR->GetCurSel();

  if(nI == CB_ERR)
  {
    return;
  }

  long DRIVERSrecordID = pComboBoxOPERATOR->GetItemData(nI);

  DailyOpsBuildRecord(&DAILYOPS, DAILYOPS_FLAG_OPERATOR);
  DAILYOPS.pertainsToDate = m_DailyOpsDateYYYYMMDD;
  DAILYOPS.pertainsToTime = m_DailyOpsDate.GetHour() * 3600 + m_DailyOpsDate.GetMinute() * 60 + m_DailyOpsDate.GetSecond();
  DAILYOPS.recordFlags = DAILYOPS_FLAG_OPERATORDEASSIGN;
  DAILYOPS.DRIVERSrecordID = DRIVERSrecordID;
  rcode2 = btrieve(B_INSERT, TMS_DAILYOPS, &DAILYOPS, &DAILYOPSKey0, 0);
  if(rcode2 == 0)
  {
    m_LastDAILYOPSRecordID = DAILYOPS.recordID;
    PopulateOpenWorkMonitor(NO_RECORD, FALSE, TRUE);
//    PopulateVehicleAssignment();
//
//  Fix the pointer and the display
//
    for(nI = 0; nI < m_numBlocksInBlockInfo; nI++)
    {
      if(m_BlockInfo[nI].startingDRIVERSrecordID == DRIVERSrecordID)
      {
        m_BlockInfo[nI].startingDRIVERSrecordID = NO_RECORD;
        RefreshBlockList(nI);
      }
    }
    s.LoadString(TEXT_398);
    sprintf(tempString, s, m_SystemTime.Format(_T("%I:%M:%S%p")), BuildOperatorString(DRIVERSrecordID));
    pEditMESSAGES->SetSel(-1, -1);
    pEditMESSAGES->ReplaceSel(tempString);
  }

  pComboBoxOPERATOR->SetFocus();
}

void CDailyOps::OnSelchangeChronology() 
{
  m_PreviousTime = m_SystemTime;

  pListBoxCHRONOLOGY->SetCurSel(-1);
}

void CDailyOps::OnDblclkChronology() 
{
  m_PreviousTime = m_SystemTime;

  pListBoxCHRONOLOGY->SetCurSel(-1);
}

void CDailyOps::OnIncludehours() 
{
  m_PreviousTime = m_SystemTime;

  m_bIncludeHours = !m_bIncludeHours;
}

void CDailyOps::OnChangeOvertimetime() 
{
  m_PreviousTime = m_SystemTime;

  if(pButtonEXTRATRIP->GetCheck())
  {
    pButtonEXTRATRIP->SetCheck(FALSE);
    pButtonOVERTIME->SetCheck(TRUE);
  }
}

int CDailyOps::RefreshAbsenceList(long dateToUse, ABSENTDef* pAbsentList)
{
  BOOL bFound;
  int  rcode2;
  int  numAbsent;

  numAbsent = 0;
//
//  Build a list of who isn't here today
//
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
        pAbsentList[numAbsent].DRIVERSrecordID = DAILYOPS.DRIVERSrecordID;
        pAbsentList[numAbsent].fromTime = DAILYOPS.pertainsToTime;
        pAbsentList[numAbsent].toTime = DAILYOPS.DOPS.Absence.untilTime;
//
//  Set the fromTime to 0 if we're in a date range
//
        if(dateToUse > DAILYOPS.pertainsToDate)
        {
          pAbsentList[numAbsent].fromTime = 0;
        }
//
//  Set the toTime to 1159P if we're in a date range
//
        if(dateToUse < DAILYOPS.DOPS.Absence.untilDate)
        {
          pAbsentList[numAbsent].toTime = T1159P;
        }
//
//  If the toTime is 1159P, add extra time to it to ensure
//  we don't get a forced "run split" if the run goes later
//
        if(pAbsentList[numAbsent].toTime >= T1159P)
        {
          pAbsentList[numAbsent].toTime += T1159P;
        }
//
//  Jam in the rest of the data
//
        pAbsentList[numAbsent].reasonIndex = DAILYOPS.DOPS.Absence.reasonIndex;
        pAbsentList[numAbsent].timeLost = DAILYOPS.DOPS.Absence.timeLost;
        pAbsentList[numAbsent].paidTime[0] = DAILYOPS.DOPS.Absence.paidTime[0];
        pAbsentList[numAbsent].paidTime[1] = DAILYOPS.DOPS.Absence.paidTime[1];
        pAbsentList[numAbsent].unpaidTime = DAILYOPS.DOPS.Absence.unpaidTime;
        numAbsent++;
      }
    }
    rcode2 = btrieve(B_GETNEXT, TMS_DAILYOPS, &DAILYOPS, &DAILYOPSKey1, 1);
  }
  return(numAbsent);
}

void CDailyOps::OnOpenworkdisplay() 
{
  m_PreviousTime = m_SystemTime;

  m_bOpenWorkDisplay = pButtonOPENWORKDISPLAY->GetCheck();
  PopulateOpenWorkMonitor(NO_RECORD, FALSE, TRUE);
}

void CDailyOps::OnColumnclickOpenworklist(NMHDR* pNMHDR, LRESULT* pResult) 
{
  m_PreviousTime = m_SystemTime;

	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;

  if(pNMListView)
  {
    ListView_SortItems(pNMListView->hdr.hwndFrom,
          DOOWListViewCompareProc, (LPARAM)(pNMListView->iSubItem));
    bSortForward[pNMListView->iSubItem] = !bSortForward[pNMListView->iSubItem];
  }
	
	*pResult = 0;
}

void CDailyOps::GetOWMPointer(int row, OPENWORKDef** pOW)
{
  m_PreviousTime = m_SystemTime;

  LVITEM  LVI;
  CString s;
 
  LVI.mask = LVIF_PARAM;
  LVI.iItem = row;
  LVI.iSubItem = 0;
  pListCtrlOPENWORKLIST->GetItem(&LVI);
  *pOW = (OPENWORKDef *)LVI.lParam;
}

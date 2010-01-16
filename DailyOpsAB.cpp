// DailyOpsAB.cpp : implementation file
//

#include "stdafx.h"
extern "C"{
#include "TMSHeader.h"
}  // EXTERN C

#include "tms.h"
#include "DailyOpsHeader.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDailyOpsAB dialog


CDailyOpsAB::CDailyOpsAB(CWnd* pParent /*=NULL*/, long date, BOOL bAutomatic)
	: CDialog(CDailyOpsAB::IDD, pParent)
{
  m_Date = date;
  m_bInitializing = TRUE;
  m_bAutomatic = bAutomatic;

	//{{AFX_DATA_INIT(CDailyOpsAB)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CDailyOpsAB::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDailyOpsAB)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDailyOpsAB, CDialog)
	//{{AFX_MSG_MAP(CDailyOpsAB)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDPRINT, OnPrint)
	ON_BN_CLICKED(DAILYOPSAB_SHOWOFF, OnShowoff)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDailyOpsAB message handlers

BEGIN_EVENTSINK_MAP(CDailyOpsAB, CDialog)
    //{{AFX_EVENTSINK_MAP(CDailyOpsAB)
	ON_EVENT(CDailyOpsAB, DAILYOPSAB_DATE, 2 /* Change */, OnChangeDate, VTS_NONE)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

BOOL CDailyOpsAB::OnInitDialog() 
{
	CDialog::OnInitDialog();

  HCURSOR hSaveCursor = SetCursor(hCursorWait);

//
//  Set up the pointers to the controls
//
  pListCtrlLIST = (CListCtrl *)GetDlgItem(DAILYOPSAB_LIST);
  pDTPickerDATE = (CDTPicker *)GetDlgItem(DAILYOPSAB_DATE);
  pButtonSHOWOFF = (CButton *)GetDlgItem(DAILYOPSAB_SHOWOFF);
//
//  Set today's date on the Date and Time picker
//
  COleVariant v;
  long year, month, day;

  if(m_Date == NO_RECORD)
  {
    CTime time = CTime::GetCurrentTime();
  
    day = time.GetDay();
    month = time.GetMonth();
    year = time.GetYear();
  }
  else
  {
    GetYMD(m_Date, &year, &month, &day);
  }

  v = year;
  pDTPickerDATE->SetYear(v);
  v = month;
  pDTPickerDATE->SetMonth(v);
  v = day;
  pDTPickerDATE->SetDay(v);
//
//  Set up the list controls
//
  DWORD dwExStyles = pListCtrlLIST->GetExtendedStyle();
  pListCtrlLIST->SetExtendedStyle(dwExStyles | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_INFOTIP);

  LVCOLUMN LVC;
  LVC.mask = LVCF_FMT | LVCF_TEXT | LVCF_WIDTH;

  LVC.cx = 128;
  LVC.fmt = LVCFMT_LEFT;
  LVC.pszText = "Entry Date/Time";
  pListCtrlLIST->InsertColumn(0, &LVC);

  LVC.cx = 164;
  LVC.pszText = "Operator";
  pListCtrlLIST->InsertColumn(1, &LVC);
  
  LVC.cx = 100;
  LVC.fmt = LVCFMT_LEFT;
  LVC.pszText = "Reason";
  pListCtrlLIST->InsertColumn(2, &LVC);

  LVC.cx = 128;
  LVC.pszText = "From Date/Time";
  pListCtrlLIST->InsertColumn(3, &LVC);
 
  LVC.cx = 128;
  LVC.pszText = "To Date/Time";
  pListCtrlLIST->InsertColumn(4, &LVC);
//
//  Default to showing drivers with the day off
//
  pButtonSHOWOFF->SetCheck(TRUE);
//
//  Set the date to force a refresh
//
  OnChangeDate();

  SetCursor(hSaveCursor);
  m_bInitializing = FALSE;

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CDailyOpsAB::OnClose() 
{
  OnCancel();
}

void CDailyOpsAB::OnCancel() 
{
	CDialog::OnCancel();
}

void CDailyOpsAB::OnChangeDate() 
{
//
//  Cycle through the entries for this operator
//
  LVITEM  LVI;
  CString s;
  CTime  time = CTime::GetCurrentTime();
  long   thisDate;
  long   startDate;
  long   year, month, day;
  char   lastName[DRIVERS_LASTNAME_LENGTH + 1];
  char   firstName[DRIVERS_FIRSTNAME_LENGTH + 1];
  char   badgeNumber[DRIVERS_BADGENUMBER_LENGTH + 1];
  int    iItem = 0;
  int    rcode2;

//
//  Clear the control
//
  pListCtrlLIST->DeleteAllItems();
//
//  Get the date
//
//  Date
//
  COleVariant v;

  v = pDTPickerDATE->GetYear();
  year = v.lVal;
  v = pDTPickerDATE->GetMonth();
  month = v.lVal;
  v = pDTPickerDATE->GetDay();
  day = v.lVal;

//
//  Database verify 11-Jan-07
//
  thisDate = year * 10000 + month * 100 + day;
  startDate = (year - 1) * 10000 + month * 100 + day;
  DAILYOPSKey1.recordTypeFlag = DAILYOPS_FLAG_ABSENCE;
  DAILYOPSKey1.pertainsToDate = startDate;
  DAILYOPSKey1.pertainsToTime = NO_TIME;
  DAILYOPSKey1.recordFlags = 0;
  rcode2 = btrieve(B_GETGREATER, TMS_DAILYOPS, &DAILYOPS, &DAILYOPSKey1, 1);
  while(rcode2 == 0 &&
        (DAILYOPS.recordTypeFlag & DAILYOPS_FLAG_ABSENCE) &&
         DAILYOPS.pertainsToDate <= thisDate)
  {
    if(thisDate >= DAILYOPS.pertainsToDate &&
          thisDate <= DAILYOPS.DOPS.Absence.untilDate &&
          (DAILYOPS.recordFlags & DAILYOPS_FLAG_ABSENCEREGISTER))
    {
      if(!ANegatedRecord(DAILYOPS.recordID, 1))
      {
//
//  Date/time of entry
//
        LVI.mask = LVIF_TEXT | LVIF_PARAM;
        LVI.iItem = iItem;
        LVI.iSubItem = 0;
        sprintf(tempString, "%s", ctime(&DAILYOPS.entryDateAndTime));
        LVI.pszText = tempString;
        LVI.lParam = DAILYOPS.recordID;
        LVI.iItem = pListCtrlLIST->InsertItem(&LVI);
//
//  Operator
//
        LVI.mask = LVIF_TEXT;
        LVI.iItem = iItem;
        LVI.iSubItem = 1;
        DRIVERSKey0.recordID = DAILYOPS.DRIVERSrecordID;
        rcode2 = btrieve(B_GETEQUAL, TMS_DRIVERS, &DRIVERS, &DRIVERSKey0, 0);
        if(rcode2 != 0)
        {
          strcpy(tempString, "Driver not found");
        }
        else
        {
          strncpy(badgeNumber, DRIVERS.badgeNumber, DRIVERS_BADGENUMBER_LENGTH);
          trim(badgeNumber, DRIVERS_BADGENUMBER_LENGTH);
          strncpy(lastName, DRIVERS.lastName, DRIVERS_LASTNAME_LENGTH);
          trim(lastName, DRIVERS_LASTNAME_LENGTH);
          strncpy(firstName, DRIVERS.firstName, DRIVERS_FIRSTNAME_LENGTH);
          trim(firstName, DRIVERS_FIRSTNAME_LENGTH);
          strcpy(tempString, badgeNumber);
          strcat(tempString, " - ");
          strcat(tempString, lastName);
          strcat(tempString, ", ");
          strcat(tempString, firstName);
        }
        LVI.pszText = tempString;
        LVI.iItem = pListCtrlLIST->SetItem(&LVI);
//
//  Reason
//
        LVI.mask = LVIF_TEXT;
        LVI.iItem = iItem;
        LVI.iSubItem = 2;
        if(DAILYOPS.DOPS.Absence.reasonIndex == NO_RECORD)
        {
          strcpy(tempString, "Assignment cleared manually");
        }
        else
        {
          strcpy(tempString, m_AbsenceReasons[DAILYOPS.DOPS.Absence.reasonIndex].szText);
        }
        LVI.pszText = tempString;
        LVI.iItem = pListCtrlLIST->SetItem(&LVI);
//
//  From Date and time
//
        GetYMD(DAILYOPS.pertainsToDate, &year, &month, &day);
        LVI.mask = LVIF_TEXT;
        LVI.iItem = iItem;
        LVI.iSubItem = 3;
        sprintf(tempString, "%02ld-%s-%04ld at %s",
              day, szMonth[month - 1], year, Tchar(DAILYOPS.pertainsToTime));
        LVI.pszText = tempString;
        LVI.iItem = pListCtrlLIST->SetItem(&LVI);
//
//  To Date and time
//
        GetYMD(DAILYOPS.DOPS.Absence.untilDate, &year, &month, &day);
        LVI.mask = LVIF_TEXT;
        LVI.iItem = iItem;
        LVI.iSubItem = 4;
        sprintf(tempString, "%02ld-%s-%04ld at %s",
              day, szMonth[month - 1], year, Tchar(DAILYOPS.DOPS.Absence.untilTime));
        LVI.pszText = tempString;
        LVI.iItem = pListCtrlLIST->SetItem(&LVI);
//
//  Set up the next row
//
        iItem++;
      }
    }
    rcode2 = btrieve(B_GETNEXT, TMS_DAILYOPS, &DAILYOPS, &DAILYOPSKey1, 1);
  }
//
//  Display operators with an off day?
//
  if(m_bAutomatic || !pButtonSHOWOFF->GetCheck())
  {
    return;
  }
  GetYMD(thisDate, &year, &month, &day);
//
//  Get the list of drivers and assume they're not working
//
  long DRIVERSrecordIDs[500];
  int  numDrivers = 0;

  rcode2 = btrieve(B_GETFIRST, TMS_DRIVERS, &DRIVERS, &DRIVERSKey2, 2);
  while(rcode2 == 0)
  {
    if(ConsideringThisDriverType(DRIVERS.DRIVERTYPESrecordID))
    {
      DRIVERSrecordIDs[numDrivers] = DRIVERS.recordID;
      numDrivers++;
    }
    rcode2 = btrieve(B_GETNEXT, TMS_DRIVERS, &DRIVERS, &DRIVERSKey2, 2);
  }
//
//  Eliminate those who are working sometime today
//
  CTime workDate(year, month, day, 0, 0, 0);
  int nI, nJ;
  int indexToRoster[7] = {6, 0, 1, 2, 3, 4, 5};
  int today = indexToRoster[workDate.GetDayOfWeek() - 1];

  ROSTERKey1.DIVISIONSrecordID = m_DailyOpsROSTERDivisionInEffect;
  ROSTERKey1.rosterNumber = NO_RECORD;
  rcode2 = btrieve(B_GETGREATER, TMS_ROSTER, &ROSTER, &ROSTERKey1, 1);
  while(rcode2 == 0 &&
        ROSTER.DIVISIONSrecordID == m_DailyOpsROSTERDivisionInEffect)
  {
    if(ROSTER.DRIVERSrecordID != NO_RECORD && ROSTER.WEEK[m_RosterWeek].RUNSrecordIDs[today] != NO_RECORD)
    {
      for(nI = 0; nI < numDrivers; nI++)
      {
        if(DRIVERSrecordIDs[nI] != NO_RECORD)
        {
          if(ROSTER.DRIVERSrecordID == DRIVERSrecordIDs[nI])
          {
            DRIVERSrecordIDs[nI] = NO_RECORD;
            break;
          }
        }
      }
    }
    rcode2 = btrieve(B_GETNEXT, TMS_ROSTER, &ROSTER, &ROSTERKey1, 1);
  }
//
//  Eliminate those that are already marked as absent
//
  int numItems = pListCtrlLIST->GetItemCount();

  for(nI = 0; nI < numItems; nI++)
  {
//
//  Determine the operator
//
    DAILYOPSKey0.recordID = pListCtrlLIST->GetItemData(nI);
    rcode2 = btrieve(B_GETEQUAL,TMS_DAILYOPS, &DAILYOPS, &DAILYOPSKey0, 0);
    if(rcode2 == 0)
    {
      for(nJ = 0; nJ < numDrivers; nJ++)
      {
        if(DRIVERSrecordIDs[nJ] != NO_RECORD)
        {
          if(DRIVERSrecordIDs[nJ] == DAILYOPS.DRIVERSrecordID)
          {
            DRIVERSrecordIDs[nJ] = NO_RECORD;
            break;
          }
        }
      }
    }
  }
//
//  Spit out those that remain on the list
//
  for(nI = 0; nI < numDrivers; nI++)
  {
    if(DRIVERSrecordIDs[nI] == NO_RECORD)
    {
      continue;
    }
//
//  Date/time of entry
//
    LVI.mask = LVIF_TEXT | LVIF_PARAM;
    LVI.iItem = iItem;
    LVI.iSubItem = 0;
    LVI.pszText = "";
    LVI.lParam = NO_RECORD;
    LVI.iItem = pListCtrlLIST->InsertItem(&LVI);
//
//  Operator
//
    LVI.mask = LVIF_TEXT;
    LVI.iItem = iItem;
    LVI.iSubItem = 1;
    DRIVERSKey0.recordID = DRIVERSrecordIDs[nI];
    rcode2 = btrieve(B_GETEQUAL, TMS_DRIVERS, &DRIVERS, &DRIVERSKey0, 0);
    if(rcode2 != 0)
    {
      strcpy(tempString, "Driver not found");
    }
    else
    {
      strncpy(badgeNumber, DRIVERS.badgeNumber, DRIVERS_BADGENUMBER_LENGTH);
      trim(badgeNumber, DRIVERS_BADGENUMBER_LENGTH);
      strncpy(lastName, DRIVERS.lastName, DRIVERS_LASTNAME_LENGTH);
      trim(lastName, DRIVERS_LASTNAME_LENGTH);
      strncpy(firstName, DRIVERS.firstName, DRIVERS_FIRSTNAME_LENGTH);
      trim(firstName, DRIVERS_FIRSTNAME_LENGTH);
      strcpy(tempString, badgeNumber);
      strcat(tempString, " - ");
      strcat(tempString, lastName);
      strcat(tempString, ", ");
      strcat(tempString, firstName);
    }
    LVI.pszText = tempString;
    LVI.iItem = pListCtrlLIST->SetItem(&LVI);
//
//  Reason
//
    LVI.mask = LVIF_TEXT;
    LVI.iItem = iItem;
    LVI.iSubItem = 2;
    strcpy(tempString, "Day Off");
    LVI.pszText = tempString;
    LVI.iItem = pListCtrlLIST->SetItem(&LVI);
//
//  Set up the next row
//
    iItem++;
  }
}

void CDailyOpsAB::OnOK() 
{
	CDialog::OnOK();
}

void CDailyOpsAB::OnPrint() 
{
  PrintWindowToDC(this->m_hWnd, "Daily Absence Report");
}

void CDailyOpsAB::OnShowoff() 
{
  if(!m_bInitializing)
  {
    OnChangeDate();
  }
}

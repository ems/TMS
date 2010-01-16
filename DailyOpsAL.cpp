// DailyOpsAL.cpp : implementation file
//

#include "stdafx.h"
extern "C"{
#include "TMSHeader.h"

BOOL ProcessAbsenceRules(DAILYOPSDef *pDAILYOPS, BOOL bRegister);

}  // EXTERN C

#include "tms.h"
#include "DailyOpsHeader.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDailyOpsAL dialog


CDailyOpsAL::CDailyOpsAL(CWnd* pParent, DAILYOPSALDef* pPassedData)
	: CDialog(CDailyOpsAL::IDD, pParent)
{
  m_pPassedData = pPassedData;

	//{{AFX_DATA_INIT(CDailyOpsAL)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CDailyOpsAL::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDailyOpsAL)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDailyOpsAL, CDialog)
	//{{AFX_MSG_MAP(CDailyOpsAL)
	ON_BN_CLICKED(DAILYOPSAL_ALLDAY, OnAllday)
	ON_BN_CLICKED(DAILYOPSAL_SPECIFICTIMES, OnSpecifictimes)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDHELP, OnHelp)
	ON_BN_CLICKED(IDNOW1, OnNow1)
	ON_BN_CLICKED(IDNOW2, OnNow2)
	ON_BN_CLICKED(IDCALCULATE, OnCalculate)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


BEGIN_EVENTSINK_MAP(CDailyOpsAL, CDialog)
    //{{AFX_EVENTSINK_MAP(CDailyOpsAL)
	ON_EVENT(CDailyOpsAL, DAILYOPSAL_FROMDATE, 2 /* Change */, OnChangeFromdate, VTS_NONE)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDailyOpsAL message handlers

BOOL CDailyOpsAL::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
//
//  Set up the pointers to the controls
//
  pStaticWHO = (CStatic *)GetDlgItem(DAILYOPSAL_WHO);
  pDTPickerFROMDATE = (CDTPicker *)GetDlgItem(DAILYOPSAL_FROMDATE);
  pDTPickerUNTILDATE = (CDTPicker *)GetDlgItem(DAILYOPSAL_UNTILDATE);
  pButtonALLDAY = (CButton *)GetDlgItem(DAILYOPSAL_ALLDAY);
  pButtonSPECIFICTIMES = (CButton *)GetDlgItem(DAILYOPSAL_SPECIFICTIMES);
  pStaticFROMTIME_TITLE = (CStatic *)GetDlgItem(DAILYOPSAL_FROMTIME_TITLE);
  pDTPickerFROMTIME = (CDTPicker *)GetDlgItem(DAILYOPSAL_FROMTIME);
  pButtonIDNOW1 = (CButton *)GetDlgItem(IDNOW1);
  pStaticUNTILTIME_TITLE = (CStatic *)GetDlgItem(DAILYOPSAL_UNTILTIME_TITLE);
  pDTPickerUNTILTIME = (CDTPicker *)GetDlgItem(DAILYOPSAL_UNTILTIME);
  pButtonIDNOW2 = (CButton *)GetDlgItem(IDNOW2);
  pListBoxREASON = (CListBox *)GetDlgItem(DAILYOPSAL_REASON);
  pEditTIMEUSED = (CEdit *)GetDlgItem(DAILYOPSAL_TIMEUSED);
  pButtonIDCALCULATE = (CButton *)GetDlgItem(IDCALCULATE);
  pEditNOTES = (CEdit *)GetDlgItem(DAILYOPSAL_NOTES);
//
//  Display the operator in the window
//
  CString s = BuildOperatorString(m_pPassedData->DRIVERSrecordID);

  pStaticWHO->SetWindowText(s);
//
//  Set today's date on the Date and Time pickers
//
  COleVariant v;
  long day = m_pPassedData->SystemTime.GetDay();
  long month = m_pPassedData->SystemTime.GetMonth();
  long year = m_pPassedData->SystemTime.GetYear();

  v = year;
  pDTPickerFROMDATE->SetYear(v);
  pDTPickerUNTILDATE->SetYear(v);
  
  v = month;
  pDTPickerFROMDATE->SetMonth(v);
  pDTPickerUNTILDATE->SetMonth(v);
  
  v = day;
  pDTPickerFROMDATE->SetDay(v);
  pDTPickerUNTILDATE->SetDay(v);
//
//  Set up the absence reasons
//
  int nI, nJ; 
  for(nI = 0; nI < m_numAbsenceReasons; nI++)
  {
    nJ = pListBoxREASON->AddString(m_AbsenceReasons[nI].szText);
    pListBoxREASON->SetItemData(nJ, nI);
  }
//
//  Default to "All day"
//
  pButtonALLDAY->SetCheck(TRUE);
  
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CDailyOpsAL::OnClose() 
{
  OnCancel();
}

void CDailyOpsAL::OnCancel() 
{
	CDialog::OnCancel();
}

void CDailyOpsAL::OnHelp() 
{
}

//
//  OnAllDay() - Disable the controls under the "Specific times" radio button
//
void CDailyOpsAL::OnAllday() 
{
  pStaticFROMTIME_TITLE->EnableWindow(FALSE);
  pDTPickerFROMTIME->EnableWindow(FALSE);
  pButtonIDNOW1->EnableWindow(FALSE);
  pStaticUNTILTIME_TITLE->EnableWindow(FALSE);
  pDTPickerUNTILTIME->EnableWindow(FALSE);
  pButtonIDNOW2->EnableWindow(FALSE);
}

//
//  OnSpecifictimes() - Enable the controls under the "Specific times" radio button
//
void CDailyOpsAL::OnSpecifictimes() 
{
  pStaticFROMTIME_TITLE->EnableWindow(TRUE);
  pDTPickerFROMTIME->EnableWindow(TRUE);
  pButtonIDNOW1->EnableWindow(TRUE);
  pStaticUNTILTIME_TITLE->EnableWindow(TRUE);
  pDTPickerUNTILTIME->EnableWindow(TRUE);
  pButtonIDNOW2->EnableWindow(TRUE);
}

void CDailyOpsAL::OnChangeFromdate() 
{
  COleVariant v;
  long year, month, day;
//
//  Get the from date
//
  v = pDTPickerFROMDATE->GetYear();
  year = v.lVal;
  v = pDTPickerFROMDATE->GetMonth();
  month = v.lVal;
  v = pDTPickerFROMDATE->GetDay();
  day = v.lVal;
//
//  Set the until date
  v = year;
  pDTPickerUNTILDATE->SetYear(v);
  v = month;
  pDTPickerUNTILDATE->SetMonth(v);
  v = day;
  pDTPickerUNTILDATE->SetDay(v);
}

void CDailyOpsAL::OnNow1() 
{
  SetTimeToNow(pDTPickerFROMTIME);
}

void CDailyOpsAL::OnNow2() 
{
  SetTimeToNow(pDTPickerUNTILTIME);
}

void CDailyOpsAL::SetTimeToNow(CDTPicker* pDTP)
{
  COleVariant v;

  v = (long)m_pPassedData->SystemTime.GetHour();
  pDTP->SetHour(v);
  v = (long)m_pPassedData->SystemTime.GetMinute();
  pDTP->SetMinute(v);
  v = (long)m_pPassedData->SystemTime.GetSecond();
  pDTP->SetSecond(v);
}

void CDailyOpsAL::OnCalculate() 
{
/*
//
//  Determine his "off hours"
//
  typedef struct RunPayStruct
  {
    int  num;
    long RUNSrecordIDs[10];
    long payTime[10];
    long flags[10];
  } RunPayDef;

  RunPayDef RD[7];

  int nI, nJ;

//
//  Get the reason
//
  nI = pListBoxREASON->GetCurSel();
  if(nI == CB_ERR)
  {
    return;
  }

  int reasonIndex = (int)pListBoxREASON->GetItemData(nI);
  
  BOOL bLookToRoster = (reasonIndex != 5);  // Vacation
  for(nI = 0; nI < 7; nI++)
  {
    RD[nI].num = 0;
  }

  int rcode2;
//
//  Get the driver
//

  long DRIVERSrecordID = m_pPassedData->DRIVERSrecordID;

  HCURSOR hSaveCursor = ::SetCursor(hCursorWait);

//
//  Get his data from the roster
//
  ROSTERKey1.DIVISIONSrecordID = m_DailyOpsROSTERDivisionInEffect;
  ROSTERKey1.rosterNumber = NO_RECORD;
  rcode2 = btrieve(B_GETGREATER, TMS_ROSTER, &ROSTER, &ROSTERKey1, 1);
  while(rcode2 == 0 &&
        ROSTER.DIVISIONSrecordID == m_DailyOpsROSTERDivisionInEffect)
  {
    if(ROSTER.DRIVERSrecordID == DRIVERSrecordID)
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
  long    eightHours = 8 * 60 * 60;

  for(nI = 0; nI < 7; nI++)
  {
    for(nJ = 0; nJ < RD[nI].num; nJ++)
    {
      RUNSKey0.recordID = RD[nI].RUNSrecordIDs[nJ];
      rcode2 = btrieve(B_GETEQUAL, TMS_RUNS, &RUNS, &RUNSKey0, 0);
      rcode2 = btrieve(B_GETPOSITION, TMS_RUNS, &RUNS, &RUNSKey0, 0);
      rcode2 = btrieve(B_GETDIRECT, TMS_RUNS, &RUNS, &RUNSKey1, 1);
      numPieces = GetRunElements(hWndMain, &RUNS, &PROPOSEDRUN, &COST, TRUE);
      RD[nI].payTime[nJ] = (bLookToRoster ? COST.TOTAL.payTime : eightHours);
      if(!bLookToRoster)
      {
        break;
      }
    }
  }
  COleVariant v;
  long fYear, fMonth, fDay;
  long tDate, tYear, tMonth, tDay;
//
//  Get the from date
//
  v = pDTPickerFROMDATE->GetYear();
  fYear = v.lVal;
  v = pDTPickerFROMDATE->GetMonth();
  fMonth = v.lVal;
  v = pDTPickerFROMDATE->GetDay();
  fDay = v.lVal;
//
//  Get the until date
//
  v = pDTPickerUNTILDATE->GetYear();
  tYear = v.lVal;
  v = pDTPickerUNTILDATE->GetMonth();
  tMonth = v.lVal;
  v = pDTPickerUNTILDATE->GetDay();
  tDay = v.lVal;
  tDate = tYear * 10000 + tMonth * 100 + tDay;
//
//  Start with today
//
  int indexToRoster[7] = {6, 0, 1, 2, 3, 4, 5};
//
//  Go through the assignments for each day
//
  CTime currentDate(fYear, fMonth, fDay, 0, 0, 0);
  currentDate -= CTimeSpan(1, 0, 0, 0);
//
//  Display up until the untilDate
//
  int today;
  long totalPayTime = 0;
  long day, month, year;

  for(;;)
  {
    currentDate += CTimeSpan(1, 0, 0, 0);
    day = currentDate.GetDay();
    month = currentDate.GetMonth();
    year = currentDate.GetYear();
    if(year * 10000 + month * 100 + day > tDate)
    {
      break;
    }
    today = indexToRoster[currentDate.GetDayOfWeek() - 1];
//
//  Cycle through the number of Roster entries for today (typically 1)
//
    for(nI = 0; nI < RD[today].num; nI++)
    {
      totalPayTime += RD[today].payTime[nI];
    }
  }
//
//  Now go through the open work monitor
//
//  Determine his "off hours"
//
//  Go through the assignments for each day up until the untilDate
//
  CTime currentDate2(fYear, fMonth, fDay, 0, 0, 0);
  long  date;

  currentDate2 -= CTimeSpan(1, 0, 0, 0);
  for(;;)
  {
    currentDate2 += CTimeSpan(1, 0, 0, 0);
    day = currentDate2.GetDay();
    month = currentDate2.GetMonth();
    year = currentDate2.GetYear();
    date = year * 10000 + month * 100 + day;
    if(date > tDate)
    {
      break;
    }
    PopulateOpenWorkMonitor(date, TRUE, FALSE);
    for(nI = 0; nI < m_numInOpenWork; nI++)
    {
      if(m_OpenWork[nI].DRIVERSrecordID == DRIVERSrecordID)
      {
        totalPayTime += m_OpenWork[nI].pay;
      }
    }
  }
//
//  Reset the open work monitor structures
//
  PopulateOpenWorkMonitor(NO_RECORD, TRUE, FALSE);
//
//  Set the value in the control
//
  sprintf(tempString, "%s", chhmm(totalPayTime));
  pEditTIMEUSED->SetWindowText(tempString);
//
//  All done
//
  ::SetCursor(hSaveCursor);
*/
}

void CDailyOpsAL::OnOK() 
{
  COleVariant v;
  CString s;
  long year, month, day;
  long hours, minutes, seconds;
  long reasonIndex;
  int  nI;
  int  rcode2;
//
//  Get the reason
//
  nI = pListBoxREASON->GetCurSel();
  if(nI == CB_ERR)
  {
    return;
  }
  reasonIndex = pListBoxREASON->GetItemData(nI);
//
//  Get the from date and time
//
//  Date
//
  v = pDTPickerFROMDATE->GetYear();
  year = v.lVal;
  v = pDTPickerFROMDATE->GetMonth();
  month = v.lVal;
  v = pDTPickerFROMDATE->GetDay();
  day = v.lVal;
  m_pPassedData->fromDate = year * 10000 + month * 100 + day;
//
//  Time
//
  v = pDTPickerFROMTIME->GetHour();
  hours = v.lVal;
  v = pDTPickerFROMTIME->GetMinute();
  minutes = v.lVal;
  v = pDTPickerFROMTIME->GetSecond();
  seconds = v.lVal;
  m_pPassedData->fromTime = hours * 3600 + minutes * 60 + seconds;
//
//  Get the to date and time
//
//  Date
//
  v = pDTPickerUNTILDATE->GetYear();
  year = v.lVal;
  v = pDTPickerUNTILDATE->GetMonth();
  month = v.lVal;
  v = pDTPickerUNTILDATE->GetDay();
  day = v.lVal;
  m_pPassedData->toDate = year * 10000 + month * 100 + day;
//
//  Time
//
  v = pDTPickerUNTILTIME->GetHour();
  hours = v.lVal;
  v = pDTPickerUNTILTIME->GetMinute();
  minutes = v.lVal;
  v = pDTPickerUNTILTIME->GetSecond();
  seconds = v.lVal;
  m_pPassedData->toTime = hours * 3600 + minutes * 60 + seconds;
//
//  Make sure that the fromTime is less than the toTime
//
  if(m_pPassedData->fromDate == m_pPassedData->toDate && m_pPassedData->fromTime > m_pPassedData->toTime)
  {
    TMSError(this->m_hWnd, MB_ICONSTOP, ERROR_245, (HANDLE)NULL);
    return;
  }
//
//  Record the assignment
//
  DailyOpsBuildRecord(&DAILYOPS, DAILYOPS_FLAG_ABSENCE);

  DAILYOPS.recordFlags = DAILYOPS_FLAG_ABSENCEREGISTER;
  DAILYOPS.pertainsToDate = m_pPassedData->fromDate;
  DAILYOPS.pertainsToTime = m_pPassedData->fromTime;
  DAILYOPS.DAILYOPSrecordID = NO_RECORD;
  DAILYOPS.DRIVERSrecordID = m_pPassedData->DRIVERSrecordID;
//
//  Build the rest of the record
//
  DAILYOPS.DOPS.Absence.untilDate = m_pPassedData->toDate;
  DAILYOPS.DOPS.Absence.untilTime = m_pPassedData->toTime;
  DAILYOPS.DOPS.Absence.reasonIndex = reasonIndex;
//
//  Time used/lost
//
  pEditTIMEUSED->GetWindowText(tempString, TEMPSTRING_LENGTH);
  DAILYOPS.DOPS.Absence.timeLost = thhmm(tempString);
  if(!ProcessAbsenceRules(&DAILYOPS, TRUE))
  {
    return;
  }
//
//  Were there any notes?
//
  pEditNOTES->GetWindowText(s);
  if(s != "")
  {
//
//  Set up the OFFTIME record
//
//  RecordID
//
    rcode2 = btrieve(B_GETLAST, TMS_OFFTIME, &OFFTIME, &OFFTIMEKey0, 0);
    OFFTIME.recordID = AssignRecID(rcode2, OFFTIME.recordID);
//
//  DailyOps recordID
//
    OFFTIME.DAILYOPSrecordID = DAILYOPS.recordID;
//
//  Text
//
    strcpy(tempString, s);
    pad(tempString, OFFTIME_TEXT_LENGTH);
    strncpy(OFFTIME.text, tempString, OFFTIME_TEXT_LENGTH);
//
//  Add the record
//
    rcode2 = btrieve(B_INSERT, TMS_OFFTIME, &OFFTIME, &OFFTIMEKey0, 0);
    if(rcode2 == 0)
    {
//      m_pPassedData->lastDAILYOPSrecordID = DAILYOPS.recordID;
    }
  }
	
	CDialog::OnOK();
}


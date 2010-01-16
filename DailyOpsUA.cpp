// DailyOpsUA.cpp : implementation file
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
// CDailyOpsUA dialog


CDailyOpsUA::CDailyOpsUA(CWnd* pParent /*=NULL*/, long* pDAILYOPSrecordID)
	: CDialog(CDailyOpsUA::IDD, pParent)
{
  m_pDAILYOPSrecordID = pDAILYOPSrecordID;
	//{{AFX_DATA_INIT(CDailyOpsUA)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CDailyOpsUA::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDailyOpsUA)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDailyOpsUA, CDialog)
	//{{AFX_MSG_MAP(CDailyOpsUA)
	ON_BN_CLICKED(IDHELP, OnHelp)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDNOW1, OnNow1)
	ON_BN_CLICKED(IDNOW2, OnNow2)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDailyOpsUA message handlers

BOOL CDailyOpsUA::OnInitDialog() 
{
	CDialog::OnInitDialog();
//
//  Set up pointers to the controls
//
  pButtonNAME = (CButton *)GetDlgItem(DAILYOPSUA_NAME);
  pDTPickerFROMDATE = (CDTPicker *)GetDlgItem(DAILYOPSUA_FROMDATE);
  pDTPickerFROMTIME = (CDTPicker *)GetDlgItem(DAILYOPSUA_FROMTIME);
  pButtonIDNOW1 = (CButton *)GetDlgItem(IDNOW1);
  pDTPickerUNTILDATE = (CDTPicker *)GetDlgItem(DAILYOPSUA_UNTILDATE);
  pDTPickerUNTILTIME = (CDTPicker *)GetDlgItem(DAILYOPSUA_UNTILTIME);
  pButtonIDNOW2 = (CButton *)GetDlgItem(IDNOW2);;
  pComboBoxREASON = (CComboBox *)GetDlgItem(DAILYOPSUA_REASON);
  pEditTIMEUSED = (CEdit *)GetDlgItem(DAILYOPSUA_TIMEUSED);
  pEditNOTES = (CEdit *)GetDlgItem(DAILYOPSUA_NOTES);
  pButtonPAID = (CButton *)GetDlgItem(DAILYOPSUA_PAID);
  pButtonNOTPAID = (CButton *)GetDlgItem(DAILYOPSUA_NOTPAID);
  pButtonNOTSUREIFPAID = (CButton *)GetDlgItem(DAILYOPSUA_NOTSUREIFPAID);
//
//  Fill in the controls
//
  DAILYOPSKey0.recordID = *m_pDAILYOPSrecordID;
  btrieve(B_GETEQUAL, TMS_DAILYOPS, &DAILYOPS, &DAILYOPSKey0, 0);
//
//  Driver name
//
  CString s;

  m_DRIVERSrecordID = DAILYOPS.DRIVERSrecordID;
  DRIVERSKey0.recordID = m_DRIVERSrecordID;
  btrieve(B_GETEQUAL, TMS_DRIVERS, &DRIVERS, &DRIVERSKey0, 0);
  s = GetDatabaseString(DRIVERS.lastName, DRIVERS_LASTNAME_LENGTH);
  strcpy(tempString, s);
  strcat(tempString, ", ");
  s = GetDatabaseString(DRIVERS.firstName, DRIVERS_FIRSTNAME_LENGTH);
  strcat(tempString, s);
  pButtonNAME->SetWindowText(tempString);

//
//  From date
//
  COleVariant v;
  long year, month, day;

  GetYMD(DAILYOPS.pertainsToDate, &year, &month, &day);
  v = year;
  pDTPickerFROMDATE->SetYear(v);
  v = month;
  pDTPickerFROMDATE->SetMonth(v);
  v = day;
  pDTPickerFROMDATE->SetDay(v);
//
//  From time
//
  long hours, minutes, seconds;

  GetHMS(DAILYOPS.pertainsToTime, &hours, &minutes, &seconds);
  v = hours;
  pDTPickerFROMTIME->SetHour(v);
  v = minutes;
  pDTPickerFROMTIME->SetMinute(v);
  v = seconds;
  pDTPickerFROMTIME->SetSecond(v);
//
//  To date
//
  GetYMD(DAILYOPS.DOPS.Absence.untilDate, &year, &month, &day);
  v = year;
  pDTPickerUNTILDATE->SetYear(v);
  v = month;
  pDTPickerUNTILDATE->SetMonth(v);
  v = day;
  pDTPickerUNTILDATE->SetDay(v);
//
//  To time
//
  GetHMS(DAILYOPS.DOPS.Absence.untilTime, &hours, &minutes, &seconds);
  v = hours;
  pDTPickerUNTILTIME->SetHour(v);
  v = minutes;
  pDTPickerUNTILTIME->SetMinute(v);
  v = seconds;
  pDTPickerUNTILTIME->SetSecond(v);
//
//  Category
//
//
//  Set up the absence reasons
//
  int nI, nJ;

  for(nI = 0; nI < m_numAbsenceReasons; nI++)
  {
    nJ = pComboBoxREASON->AddString(m_AbsenceReasons[nI].szText);
    pComboBoxREASON->SetItemData(nJ, nI);
    if(nI == DAILYOPS.DOPS.Absence.reasonIndex)
    {
      pComboBoxREASON->SetCurSel(nJ);
    }
  }
//
//  Time used/lost
//
  sprintf(tempString, "%s", chhmm(DAILYOPS.DOPS.Absence.timeLost));
  pEditTIMEUSED->SetWindowText(tempString);
//
//  Notes
//
  int rcode2;

  OFFTIMEKey1.DAILYOPSrecordID = *m_pDAILYOPSrecordID;
  rcode2 = btrieve(B_GETEQUAL, TMS_OFFTIME, &OFFTIME, &OFFTIMEKey1, 1);
  m_bGotOfftime = (rcode2 == 0);
  if(m_bGotOfftime)
  {
    strncpy(tempString, OFFTIME.text, OFFTIME_TEXT_LENGTH);
    trim(tempString, OFFTIME_TEXT_LENGTH);
    pEditNOTES->SetWindowText(tempString);
  }
//
//  Paid?
//
  if(DAILYOPS.recordFlags & DAILYOPS_FLAG_ABSENCEPAID)
  {
    pButtonPAID->SetCheck(TRUE);
  }
  else if(DAILYOPS.recordFlags & DAILYOPS_FLAG_NOTSUREIFPAID)
  {
    pButtonNOTSUREIFPAID->SetCheck(TRUE);
  }
  else
  {
    pButtonNOTPAID->SetCheck(TRUE);
  }

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CDailyOpsUA::OnHelp() 
{
	// TODO: Add your control notification handler code here
	
}

void CDailyOpsUA::OnClose() 
{
  OnCancel();
}

void CDailyOpsUA::OnCancel() 
{
	// TODO: Add extra cleanup here
	
	CDialog::OnCancel();
}

void CDailyOpsUA::OnOK() 
{
//
//  Unregister the previous absence
//
//  Establish "now"
//
//  Database verify: 11-Jan-07
//
  int   rcode2;
  CTime time = CTime::GetCurrentTime();
//
//  Get the driver
//
  long DRIVERSrecordID;

  DAILYOPSKey0.recordID = *m_pDAILYOPSrecordID;
  rcode2 = btrieve(B_GETEQUAL, TMS_DAILYOPS, &DAILYOPS, &DAILYOPSKey0, 0);
  DRIVERSrecordID = (rcode2 == 0 ? DAILYOPS.DRIVERSrecordID : NO_RECORD);
  rcode2 = btrieve(B_GETLAST, TMS_DAILYOPS, &DAILYOPS, &DAILYOPSKey0, 0);
  DAILYOPS.recordID = AssignRecID(rcode2, DAILYOPS.recordID);
  DAILYOPS.entryDateAndTime = time.GetTime();
  DAILYOPS.recordTypeFlag = DAILYOPS_FLAG_ABSENCE;
  DAILYOPS.userID = m_UserID;
  DAILYOPS.recordFlags = DAILYOPS_FLAG_ABSENCEUNREGISTER;
  DAILYOPS.DRIVERSrecordID = DRIVERSrecordID;
  DAILYOPS.DAILYOPSrecordID = *m_pDAILYOPSrecordID;
  memset(DAILYOPS.DOPS.associatedData, 0x00, DAILYOPS_ASSOCIATEDDATA_LENGTH);
  rcode2 = btrieve(B_INSERT, TMS_DAILYOPS, &DAILYOPS, &DAILYOPSKey0, 0);
  if(rcode2 == 0)
  {
    m_LastDAILYOPSRecordID = DAILYOPS.recordID;
  }
//
//  Register the new absence
//
  COleVariant v;
//
//  Get the from date and time
//
//  Date
//
  long year, month, day;

  v = pDTPickerFROMDATE->GetYear();
  year = v.lVal;
  v = pDTPickerFROMDATE->GetMonth();
  month = v.lVal;
  v = pDTPickerFROMDATE->GetDay();
  day = v.lVal;
  
  long fromDate = year * 10000 + month * 100 + day;
//
//  Time
//
  long hours, minutes, seconds;

  v = pDTPickerFROMTIME->GetHour();
  hours = v.lVal;
  v = pDTPickerFROMTIME->GetMinute();
  minutes = v.lVal;
  v = pDTPickerFROMTIME->GetSecond();
  seconds = v.lVal;
  
  long fromTime = hours * 3600 + minutes * 60 + seconds;
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
  
  long toDate = year * 10000 + month * 100 + day;
//
//  Time
//
  v = pDTPickerUNTILTIME->GetHour();
  hours = v.lVal;
  v = pDTPickerUNTILTIME->GetMinute();
  minutes = v.lVal;
  v = pDTPickerUNTILTIME->GetSecond();
  seconds = v.lVal;
  
  long toTime = hours * 3600 + minutes * 60 + seconds;
//
//  Time used/lost
//
  pEditTIMEUSED->GetWindowText(tempString, TEMPSTRING_LENGTH);
  
  long timeUsed = thhmm(tempString);
//
//  Get the reason
//
  int nI, reasonIndex;

  nI = pComboBoxREASON->GetCurSel();
  if(nI == CB_ERR)
  {
    return;
  }
  reasonIndex = pComboBoxREASON->GetItemData(nI);
//
//  Record the assignment
//
  DailyOpsBuildRecord(&DAILYOPS, DAILYOPS_FLAG_ABSENCE);
  DAILYOPS.recordFlags = DAILYOPS_FLAG_ABSENCEREGISTER;
  DAILYOPS.pertainsToDate = fromDate;
  DAILYOPS.pertainsToTime = fromTime;
  DAILYOPS.DAILYOPSrecordID = NO_RECORD;
  DAILYOPS.DRIVERSrecordID = m_DRIVERSrecordID;
//
//  Paid?
//
  if(pButtonPAID->GetCheck())
  {
    DAILYOPS.recordFlags |= DAILYOPS_FLAG_ABSENCEPAID;
  }
  else if(pButtonNOTSUREIFPAID->GetCheck())
  {
    DAILYOPS.recordFlags |= DAILYOPS_FLAG_NOTSUREIFPAID;
  }
//
//  Insert the new record
//
  CString s;

  DAILYOPS.DOPS.Absence.untilDate = toDate;
  DAILYOPS.DOPS.Absence.untilTime = toTime;
  DAILYOPS.DOPS.Absence.reasonIndex = reasonIndex;
  DAILYOPS.DOPS.Absence.timeLost = timeUsed;
  rcode2 = btrieve(B_INSERT, TMS_DAILYOPS, &DAILYOPS, &DAILYOPSKey0, 0);
  if(rcode2 == 0)
  {
    m_LastDAILYOPSRecordID = DAILYOPS.recordID;
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
  }
//
//  Set the DailyOps record ID to the new one so we display ok on the outside
//
  *m_pDAILYOPSrecordID = DAILYOPS.recordID;

	CDialog::OnOK();
}

void CDailyOpsUA::SetTimeToNow(CDTPicker* pDTP)
{
  COleVariant v;
  CTime time = CTime::GetCurrentTime();

  v = (long)time.GetHour();
  pDTP->SetHour(v);
  v = (long)time.GetMinute();
  pDTP->SetMinute(v);
  v = (long)time.GetSecond();
  pDTP->SetSecond(v);
}

void CDailyOpsUA::OnNow1() 
{
  SetTimeToNow(pDTPickerFROMTIME);
}

void CDailyOpsUA::OnNow2() 
{
  SetTimeToNow(pDTPickerUNTILTIME);
}


// AddHoliday.cpp : implementation file
//

#include "stdafx.h"
extern "C"{
#include "TMSHeader.h"
}

#include "tms.h"
#include "AddDialogs.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CAddHoliday dialog


CAddHoliday::CAddHoliday(CWnd* pParent /*=NULL*/, long* pRecordID)
	: CDialog(CAddHoliday::IDD, pParent)
{
  m_pRecordID = pRecordID;

	//{{AFX_DATA_INIT(CAddHoliday)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CAddHoliday::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAddHoliday)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CAddHoliday, CDialog)
	//{{AFX_MSG_MAP(CAddHoliday)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDHELP, OnHelp)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAddHoliday message handlers

BOOL CAddHoliday::OnInitDialog() 
{
	CDialog::OnInitDialog();
//
//  Set up pointers to the controls
//
  pDTPickerDATE = (CDTPicker *)GetDlgItem(ADDHOLIDAY_DATE);
  pEditDESCRIPTION = (CEdit *)GetDlgItem(ADDHOLIDAY_DESCRIPTION);
  pListBoxSERVICE = (CListBox *)GetDlgItem(ADDHOLIDAY_SERVICE);
//
//  Set up the services list box
//
  int rcode2;

  rcode2 = btrieve(B_GETFIRST, TMS_SERVICES, &SERVICES, &SERVICESKey1, 1);
  if(rcode2 != 0)
  {
    TMSError(this->m_hWnd, MB_ICONSTOP, ERROR_007, (HANDLE)NULL);
    OnCancel();
    return TRUE;
  }
//
//  Add "No Service" to all the list boxes
//
  int nI, nJ;

  LoadString(hInst, TEXT_061, tempString, TEMPSTRING_LENGTH);
  nI = pListBoxSERVICE->AddString(tempString);
  pListBoxSERVICE->SetItemData(nI, NO_RECORD);
//
//  And the services...
//
  while(rcode2 == 0)
  {
    strncpy(tempString, SERVICES.name, SERVICES_NAME_LENGTH);
    trim(tempString, SERVICES_NAME_LENGTH);
    nI = pListBoxSERVICE->AddString(tempString);
    pListBoxSERVICE->SetItemData(nI, SERVICES.recordID);
    rcode2 = btrieve(B_GETNEXT, TMS_SERVICES, &SERVICES, &SERVICESKey1, 1);
  }
//
//  Populate the dailog
//
  COleVariant v;
  long  day;
  long  month;
  long  year;
//
//  New record? Set today's date on the Date and Time pickers
//
  if(*m_pRecordID == NO_RECORD)
  {
    CTime time = CTime::GetCurrentTime();
    long  day = time.GetDay();
    long  month = time.GetMonth();
    long  year = time.GetYear();

    v = year;
    pDTPickerDATE->SetYear(v);
    v = month;
    pDTPickerDATE->SetMonth(v);
    v = day;
    pDTPickerDATE->SetDay(v);
//
//  And default to No Service
//
    pListBoxSERVICE->SetCurSel(0);
  }
//
//  Update
//
  else
  {
    DAILYOPSKey0.recordID = *m_pRecordID;
    btrieve(B_GETEQUAL, TMS_DAILYOPS, &DAILYOPS, &DAILYOPSKey0, 0);
//
//  Set the date on the calendar control
//
    GetYMD(DAILYOPS.pertainsToDate, &year, &month, &day);
    v = year;
    pDTPickerDATE->SetYear(v);
    v = month;
    pDTPickerDATE->SetMonth(v);
    v = day;
    pDTPickerDATE->SetDay(v);
//
//  Description
//
    strncpy(tempString, DAILYOPS.DOPS.Date.name, DAILYOPS_DATENAME_LENGTH);
    trim(tempString, DAILYOPS_DATENAME_LENGTH);
    pEditDESCRIPTION->SetWindowText(tempString);
//
//  Service
//
    nI = pListBoxSERVICE->GetCount();
    for(nJ = 0; nJ < nI; nJ++)
    {
      if(DAILYOPS.DOPS.Date.SERVICESrecordID == (long)pListBoxSERVICE->GetItemData(nJ))
      {
        pListBoxSERVICE->SetCurSel(nJ);
        break;
      }
    }
  }
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CAddHoliday::OnClose() 
{
  OnCancel();
}

void CAddHoliday::OnCancel() 
{
	CDialog::OnCancel();
}

void CAddHoliday::OnHelp() 
{
}

void CAddHoliday::OnOK() 
{
  CTime time = CTime::GetCurrentTime();
  int rcode2;
//
//  On an update, "Unregister" the previous record
//
  if(*m_pRecordID != NO_RECORD)
  {
    rcode2 = btrieve(B_GETLAST, TMS_DAILYOPS, &DAILYOPS, &DAILYOPSKey0, 0);
    DAILYOPS.recordID = AssignRecID(rcode2, DAILYOPS.recordID);
    DAILYOPS.entryDateAndTime = time.GetTime();
    DAILYOPS.pertainsToDate = NO_RECORD;
    DAILYOPS.pertainsToTime = NO_TIME;
    DAILYOPS.recordTypeFlag = (char)DAILYOPS_FLAG_DATE;
    DAILYOPS.recordFlags = DAILYOPS_FLAG_DATECLEAR;
    DAILYOPS.userID = m_UserID;
    DAILYOPS.DRIVERSrecordID = NO_RECORD;
    DAILYOPS.DAILYOPSrecordID = *m_pRecordID;
    memset(DAILYOPS.DOPS.associatedData, 0x00, DAILYOPS_ASSOCIATEDDATA_LENGTH);
    rcode2 = btrieve(B_INSERT, TMS_DAILYOPS, &DAILYOPS, &DAILYOPSKey0, 0);
    if(rcode2 == 0)
    {
      m_LastDAILYOPSRecordID = DAILYOPS.recordID;
    }
  }
//
//  Save the new/updated record
//
//  Date
//
  VARIANT v;
  long year, month, day;

  v = pDTPickerDATE->GetYear();
  year = v.lVal;
  v = pDTPickerDATE->GetMonth();
  month = v.lVal;
  v = pDTPickerDATE->GetDay();
  day = v.lVal;
//
//  Description
//
  pEditDESCRIPTION->GetWindowText(tempString, TEMPSTRING_LENGTH);
  pad(tempString, DAILYOPS_DATENAME_LENGTH);
//
//  Service
//
  int nI = pListBoxSERVICE->GetCurSel();

  if(nI == LB_ERR)
  {
    return;
  }
//
//  Build and insert the record
//
  rcode2 = btrieve(B_GETLAST, TMS_DAILYOPS, &DAILYOPS, &DAILYOPSKey0, 0);
  DAILYOPS.recordID = AssignRecID(rcode2, DAILYOPS.recordID);
  DAILYOPS.entryDateAndTime = time.GetTime();
  DAILYOPS.pertainsToDate = year * 10000 + month * 100 + day;
  DAILYOPS.pertainsToTime = NO_TIME;
  DAILYOPS.recordTypeFlag = (char)DAILYOPS_FLAG_DATE;
  DAILYOPS.recordFlags = DAILYOPS_FLAG_DATESET;
  DAILYOPS.userID = m_UserID;
  DAILYOPS.DRIVERSrecordID = NO_RECORD;
  DAILYOPS.DAILYOPSrecordID = *m_pRecordID;
  memset(DAILYOPS.DOPS.associatedData, 0x00, DAILYOPS_ASSOCIATEDDATA_LENGTH);
  DAILYOPS.DOPS.Date.SERVICESrecordID = pListBoxSERVICE->GetItemData(nI);
  strncpy(DAILYOPS.DOPS.Date.name, tempString, DAILYOPS_DATENAME_LENGTH);
  rcode2 = btrieve(B_INSERT, TMS_DAILYOPS, &DAILYOPS, &DAILYOPSKey0, 0);
  if(rcode2 == 0)
  {
    m_LastDAILYOPSRecordID = DAILYOPS.recordID;
  }

	
	CDialog::OnOK();
}

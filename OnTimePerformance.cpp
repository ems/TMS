//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
// OnTimePerformance.cpp : implementation file
//

#include "stdafx.h"
extern "C"
{
#include "TMSHeader.h"
}
#include "_line3dh.h"
#include "AddDialogs.h"
#include "OnTimePerformance.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

int TMSRPT36FilterDialog(TMSRPT36PassedDataDef *pPD)
{
  COnTimePerformance dlg(NULL, pPD);

  return (dlg.DoModal());

}

int GetDOW(long date)
{
  long year, month, day;

  GetYMD(date, &year, &month, &day);

  COleDateTime thisDate(year, month, day, 0, 0, 0);

  return(thisDate.GetDayOfWeek());
}

extern "C"
{
  BOOL TMSRPT36Filter(TMSRPT36PassedDataDef *pPD)
  {
    return(TMSRPT36FilterDialog(pPD) == IDOK);
  }

  int GetDayFromDate(long date)
  {
    return(GetDOW(date));
  }
}
/////////////////////////////////////////////////////////////////////////////
// COnTimePerformance dialog


COnTimePerformance::COnTimePerformance(CWnd* pParent, TMSRPT36PassedDataDef *pPD)
	: CDialog(COnTimePerformance::IDD, pParent)
{
  m_pPD = pPD;

	//{{AFX_DATA_INIT(COnTimePerformance)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void COnTimePerformance::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(COnTimePerformance)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(COnTimePerformance, CDialog)
	//{{AFX_MSG_MAP(COnTimePerformance)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDHELP, OnHelp)
	ON_BN_CLICKED(ONTIMEPERF_ALLSERVICES, OnAllservices)
	ON_BN_CLICKED(ONTIMEPERF_ALLDAYS, OnAlldays)
	ON_BN_CLICKED(ONTIMEPERF_ALLTIMEPOINTS, OnAlltimepoints)
	ON_BN_CLICKED(ONTIMEPERF_ALLROUTES, OnAllroutes)
	ON_BN_CLICKED(ONTIMEPERF_ALLDAY, OnAllday)
	ON_BN_CLICKED(ONTIMEPERF_ALLOPERATORS, OnAlloperators)
	ON_BN_CLICKED(ONTIMEPERF_DONOTEXCLUDE, OnDonotexclude)
	ON_BN_CLICKED(ONTIMEPERF_ALLBUSTYPES, OnAllbustypes)
	ON_BN_CLICKED(ONTIMEPERF_ALLPASSENGERLOADS, OnAllpassengerloads)
	ON_BN_CLICKED(ONTIMEPERF_ALLROADSUPERVISORS, OnAllroadsupervisors)
	ON_CBN_SELCHANGE(ONTIMEPERF_JUSTBUSTYPELIST, OnSelchangeJustbustypelist)
	ON_CBN_SELCHANGE(ONTIMEPERF_JUSTDAYLIST, OnSelchangeJustdaylist)
	ON_CBN_SELCHANGE(ONTIMEPERF_JUSTEXCLUDELIST, OnSelchangeJustexcludelist)
	ON_CBN_SELCHANGE(ONTIMEPERF_JUSTOPERATORLIST, OnSelchangeJustoperatorlist)
	ON_CBN_SELCHANGE(ONTIMEPERF_JUSTPASSENGERLOADLIST, OnSelchangeJustpassengerloadlist)
	ON_CBN_SELCHANGE(ONTIMEPERF_JUSTROADSUPERVISORLIST, OnSelchangeJustroadsupervisorlist)
	ON_CBN_SELCHANGE(ONTIMEPERF_JUSTROUTELIST, OnSelchangeJustroutelist)
	ON_CBN_SELCHANGE(ONTIMEPERF_JUSTSERVICELIST, OnSelchangeJustservicelist)
	ON_CBN_SELCHANGE(ONTIMEPERF_JUSTTIMEPOINTLIST, OnSelchangeJusttimepointlist)
	ON_BN_CLICKED(ONTIMEPERF_JUSTBUSTYPE, OnJustbustype)
	ON_BN_CLICKED(ONTIMEPERF_JUSTDAY, OnJustday)
	ON_BN_CLICKED(ONTIMEPERF_JUSTEXCLUDE, OnJustexclude)
	ON_BN_CLICKED(ONTIMEPERF_JUSTOPERATOR, OnJustoperator)
	ON_BN_CLICKED(ONTIMEPERF_JUSTPASSENGERLOAD, OnJustpassengerload)
	ON_BN_CLICKED(ONTIMEPERF_JUSTROADSUPERVISOR, OnJustroadsupervisor)
	ON_BN_CLICKED(ONTIMEPERF_JUSTROUTE, OnJustroute)
	ON_BN_CLICKED(ONTIMEPERF_JUSTSERVICE, OnJustservice)
	ON_BN_CLICKED(ONTIMEPERF_JUSTTIMEPOINT, OnJusttimepoint)
	ON_BN_CLICKED(ONTIMEPERF_FROM, OnFrom)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// COnTimePerformance message handlers

BOOL COnTimePerformance::OnInitDialog() 
{
	CDialog::OnInitDialog();

  COleVariant v;
  CString s;
  char szDriver[DRIVERS_LASTNAME_LENGTH + 2 + DRIVERS_FIRSTNAME_LENGTH + 1];
  long day;
  long month;
  long year;
  int  numRoutes;
  int  numServices;
  int  numBustypes;
  int  numDrivers;
  int  rcode2;
  int  nI, nJ;

  pDTPickerFROMDATE = (CDTPicker *)GetDlgItem(ONTIMEPERF_FROMDATE);
  pDTPickerTODATE = (CDTPicker *)GetDlgItem(ONTIMEPERF_TODATE);
  pButtonALLSERVICES = (CButton *)GetDlgItem(ONTIMEPERF_ALLSERVICES);
  pButtonJUSTSERVICE = (CButton *)GetDlgItem(ONTIMEPERF_JUSTSERVICE);
  pComboBoxJUSTSERVICELIST = (CComboBox *)GetDlgItem(ONTIMEPERF_JUSTSERVICELIST);
  pButtonALLDAYS = (CButton *)GetDlgItem(ONTIMEPERF_ALLDAYS);
  pButtonJUSTDAY = (CButton *)GetDlgItem(ONTIMEPERF_JUSTDAY);
  pComboBoxJUSTDAYLIST = (CComboBox *)GetDlgItem(ONTIMEPERF_JUSTDAYLIST);
  pButtonALLROUTES = (CButton *)GetDlgItem(ONTIMEPERF_ALLROUTES);
  pButtonJUSTROUTE = (CButton *)GetDlgItem(ONTIMEPERF_JUSTROUTE);
  pComboBoxJUSTROUTELIST = (CComboBox *)GetDlgItem(ONTIMEPERF_JUSTROUTELIST);
  pButtonOB = (CButton *)GetDlgItem(ONTIMEPERF_OB);
  pButtonIB = (CButton *)GetDlgItem(ONTIMEPERF_IB);
  pButtonBOTHDIRECTIONS = (CButton *)GetDlgItem(ONTIMEPERF_BOTHDIRECTIONS);
  pButtonALLTIMEPOINTS = (CButton *)GetDlgItem(ONTIMEPERF_ALLTIMEPOINTS);
  pButtonJUSTTIMEPOINT = (CButton *)GetDlgItem(ONTIMEPERF_JUSTTIMEPOINT);
  pComboBoxJUSTTIMEPOINTLIST = (CComboBox *)GetDlgItem(ONTIMEPERF_JUSTTIMEPOINTLIST);
  pButtonALLDAY = (CButton *)GetDlgItem(ONTIMEPERF_ALLDAY);
  pButtonFROM = (CButton *)GetDlgItem(ONTIMEPERF_FROM);
  pEditFROMTIME = (CEdit *)GetDlgItem(ONTIMEPERF_FROMTIME);
  pEditTOTIME = (CEdit *)GetDlgItem(ONTIMEPERF_TOTIME);
  pButtonALLOPERATORS = (CButton *)GetDlgItem(ONTIMEPERF_ALLOPERATORS);
  pButtonJUSTOPERATOR = (CButton *)GetDlgItem(ONTIMEPERF_JUSTOPERATOR);
  pComboBoxJUSTOPERATORLIST = (CComboBox *)GetDlgItem(ONTIMEPERF_JUSTOPERATORLIST);
  pButtonALLBUSTYPES = (CButton *)GetDlgItem(ONTIMEPERF_ALLBUSTYPES);
  pButtonJUSTBUSTYPE = (CButton *)GetDlgItem(ONTIMEPERF_JUSTBUSTYPE);
  pComboBoxJUSTBUSTYPELIST = (CComboBox *)GetDlgItem(ONTIMEPERF_JUSTBUSTYPELIST);
  pButtonDONOTEXCLUDE = (CButton *)GetDlgItem(ONTIMEPERF_DONOTEXCLUDE);
  pButtonJUSTEXCLUDE = (CButton *)GetDlgItem(ONTIMEPERF_JUSTEXCLUDE);
  pComboBoxJUSTEXCLUDELIST = (CComboBox *)GetDlgItem(ONTIMEPERF_JUSTEXCLUDELIST);
  pButtonALLPASSENGERLOADS = (CButton *)GetDlgItem(ONTIMEPERF_ALLPASSENGERLOADS);
  pButtonJUSTPASSENGERLOAD = (CButton *)GetDlgItem(ONTIMEPERF_JUSTPASSENGERLOAD);
  pComboBoxJUSTPASSENGERLOADLIST = (CComboBox *)GetDlgItem(ONTIMEPERF_JUSTPASSENGERLOADLIST);
  pButtonALLROADSUPERVISORS = (CButton *)GetDlgItem(ONTIMEPERF_ALLROADSUPERVISORS);
  pButtonJUSTROADSUPERVISOR = (CButton *)GetDlgItem(ONTIMEPERF_JUSTROADSUPERVISOR);
  pComboBoxJUSTROADSUPERVISORLIST = (CComboBox *)GetDlgItem(ONTIMEPERF_JUSTROADSUPERVISORLIST);
//
//  Date range.  Default from 01-Sep-02 to today.
//
  year = 2002;
  v = year;
  pDTPickerFROMDATE->SetYear(v);
  month = 9;
  v = month;
  pDTPickerFROMDATE->SetMonth(v);
  day = 1;
  v = day;
  pDTPickerFROMDATE->SetDay(v);
//
//  Set today's date on the Date and Time picker
//
  CTime time = CTime::GetCurrentTime();

  day = time.GetDay();
  month = time.GetMonth();
  year = time.GetYear();
  v = year;
  pDTPickerTODATE->SetYear(v);
  v = month;
  pDTPickerTODATE->SetMonth(v);
  v = day;
  pDTPickerTODATE->SetDay(v);
//
//  Services
//
  pButtonALLSERVICES->SetCheck(TRUE);
  pButtonALLDAYS->SetCheck(TRUE);
  numServices = SetUpServiceList(m_hWnd, ONTIMEPERF_JUSTSERVICELIST, NULL);
  if(numServices == 0)
  {
    TMSError(m_hWnd, MB_ICONSTOP, ERROR_007, (HANDLE)NULL);
    CDialog::OnCancel();
    return TRUE;
  }
  strcpy(m_szDays[0], "Sunday");
  strcpy(m_szDays[1], "Monday");
  strcpy(m_szDays[2], "Tuesday");
  strcpy(m_szDays[3], "Wednesday");
  strcpy(m_szDays[4], "Thursday");
  strcpy(m_szDays[5], "Friday");
  strcpy(m_szDays[6], "Saturday");
  for(nI = 0; nI < 6; nI++)
  {
    nJ = pComboBoxJUSTDAYLIST->AddString(m_szDays[nI]);
    if(nJ != CB_ERR)
      pComboBoxJUSTDAYLIST->SetItemData(nJ, nI);
  }

//
//  Routes
//
  pButtonALLROUTES->SetCheck(TRUE);
  pButtonBOTHDIRECTIONS->SetCheck(TRUE);
  numRoutes = SetUpRouteList(m_hWnd, ONTIMEPERF_JUSTROUTELIST, NULL);  
  if(numRoutes == 0)
  {
    TMSError(m_hWnd, MB_ICONSTOP, ERROR_008, (HANDLE)NULL);
    CDialog::OnCancel();
    return TRUE;
  }
//
//  Nodes
//
  pButtonALLTIMEPOINTS->SetCheck(TRUE);
  SetUpNodeList(m_hWnd, ONTIMEPERF_JUSTTIMEPOINTLIST, FALSE);
//
//  Time of day
//
  pButtonALLDAY->SetCheck(TRUE);
  pEditFROMTIME->EnableWindow(FALSE);
  pEditTOTIME->EnableWindow(FALSE);
//
//  Load the drivers
//
  pButtonALLOPERATORS->SetCheck(TRUE);
  pButtonDONOTEXCLUDE->SetCheck(TRUE);
  rcode2 = btrieve(B_GETFIRST, TMS_DRIVERS, &DRIVERS, &DRIVERSKey1, 1);
  while(rcode2 == 0)
  {
    strncpy(tempString, DRIVERS.lastName, DRIVERS_LASTNAME_LENGTH);
    trim(tempString, DRIVERS_LASTNAME_LENGTH);
    strcpy(szDriver, tempString);
    strcat(szDriver, ", ");
    strncpy(tempString, DRIVERS.firstName, DRIVERS_FIRSTNAME_LENGTH);
    trim(tempString, DRIVERS_FIRSTNAME_LENGTH);
    strcat(szDriver, tempString);
    numDrivers = pComboBoxJUSTOPERATORLIST->AddString(szDriver);
    if(numDrivers != CB_ERR)
      pComboBoxJUSTOPERATORLIST->SetItemData(numDrivers, DRIVERS.recordID);
    numDrivers = pComboBoxJUSTEXCLUDELIST->AddString(szDriver);
    if(numDrivers != CB_ERR)
      pComboBoxJUSTEXCLUDELIST->SetItemData(numDrivers, DRIVERS.recordID);
    rcode2 = btrieve(B_GETNEXT, TMS_DRIVERS, &DRIVERS, &DRIVERSKey1, 1);
  }
//
//  Bustypes
//
  pButtonALLBUSTYPES->SetCheck(TRUE);
  numBustypes = SetUpBustypeList(m_hWnd, ONTIMEPERF_JUSTBUSTYPELIST);
  if(numBustypes == 0)
  {
    pButtonJUSTBUSTYPE->EnableWindow(FALSE);
    pComboBoxJUSTBUSTYPELIST->EnableWindow(FALSE);
  }
//
//  Passenger load
//
  pButtonALLPASSENGERLOADS->SetCheck(TRUE);
  SetUpTextComboBox(m_PassengerLoads, m_numPassengerLoads, pComboBoxJUSTPASSENGERLOADLIST);
//
//  Road Supervisors
//
  pButtonALLROADSUPERVISORS->SetCheck(TRUE);
  SetUpTextComboBox(m_RoadSupervisors, m_numRoadSupervisors, pComboBoxJUSTROADSUPERVISORLIST);
//
//  All done
//
  ReleaseCapture();

  return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void COnTimePerformance::SetUpTextComboBox(GenericTextDef *pData, int numEntries, CComboBox *pComboBox)
{
  int  nI, nJ;
  
  pComboBox->ResetContent();

  for(nI = 0; nI < numEntries; nI++)
  {
    nJ = pComboBox->AddString(pData[nI].szText);
    pComboBox->SetItemData(nJ, pData[nI].number);
  }
  if(numEntries != 0)
  {
    pComboBox->SetCurSel(0);
  }
  else
  {
    pComboBox->EnableWindow(FALSE);
  }
}
void COnTimePerformance::OnClose() 
{
	// TODO: Add your message handler code here and/or call default
	
	CDialog::OnClose();
}

void COnTimePerformance::OnOK() 
{
  CString s;
  VARIANT v;
  long    year;
  long    month;
  long    day;
  int     nI;

  m_pPD->flags = 0;
  m_pPD->SERVICESrecordID = NO_RECORD;
  m_pPD->dayNumber = NO_RECORD;
  m_pPD->ROUTESrecordID = NO_RECORD;
  m_pPD->directionIndex = NO_RECORD;
  m_pPD->NODESrecordID = NO_RECORD;
  m_pPD->timeOfDayFrom = NO_TIME;
  m_pPD->timeOfDayTo = NO_TIME;
  m_pPD->DRIVERSrecordID = NO_RECORD;
  m_pPD->excludedDRIVERSrecordID = NO_RECORD;
  m_pPD->BUSTYPESrecordID = NO_RECORD;
  m_pPD->passengerLoadIndex = NO_RECORD;
  m_pPD->roadSupervisorIndex = NO_RECORD;
//
//  From date
//
  v = pDTPickerFROMDATE->GetYear();
  year = v.lVal;
  v = pDTPickerFROMDATE->GetMonth();
  month = v.lVal;
  v = pDTPickerFROMDATE->GetDay();
  day = v.lVal;
  m_pPD->fromDate = year * 10000 + month * 100 + day;
//
//  To date
//
  v = pDTPickerTODATE->GetYear();
  year = v.lVal;
  v = pDTPickerTODATE->GetMonth();
  month = v.lVal;
  v = pDTPickerTODATE->GetDay();
  day = v.lVal;
  m_pPD->toDate = year * 10000 + month * 100 + day;
//
//  Service
//
  if(pButtonALLSERVICES->GetCheck())
    m_pPD->flags |= TMSRPT36_FLAG_ALLSERVICES;
  else
  {
    nI = (int)pComboBoxJUSTSERVICELIST->GetCurSel();
    if(nI != CB_ERR)
      m_pPD->SERVICESrecordID = pComboBoxJUSTSERVICELIST->GetItemData(nI);
  }
//
//  Day number
//
  if(pButtonALLDAYS->GetCheck())
    m_pPD->flags |= TMSRPT36_FLAG_ALLSERVICEDAYS;
  else
  {
    nI = (int)pComboBoxJUSTDAYLIST->GetCurSel();
    if(nI != CB_ERR)
      m_pPD->dayNumber = pComboBoxJUSTDAYLIST->GetItemData(nI);
  }
//
//  Route
//
  if(pButtonALLROUTES->GetCheck())
    m_pPD->flags |= TMSRPT36_FLAG_ALLROUTES;
  else
  {
    nI = (int)pComboBoxJUSTROUTELIST->GetCurSel();
    if(nI != CB_ERR)
      m_pPD->ROUTESrecordID = pComboBoxJUSTROUTELIST->GetItemData(nI);
  }
//
//  Direction index
//
  if(pButtonBOTHDIRECTIONS->GetCheck())
    m_pPD->flags |= TMSRPT36_FLAG_BOTHDIRECTIONS;
  else
    m_pPD->directionIndex = pButtonOB->GetCheck() ? 0 : 1;
//
//  Node
//
  if(pButtonALLTIMEPOINTS->GetCheck())
    m_pPD->flags |= TMSRPT36_FLAG_ALLTIMEPOINTS;
  else
  {
    nI = (int)pComboBoxJUSTTIMEPOINTLIST->GetCurSel();
    if(nI != CB_ERR)
      m_pPD->NODESrecordID = pComboBoxJUSTTIMEPOINTLIST->GetItemData(nI);
  }
//
//  Time of day
//
  if(pButtonALLDAY->GetCheck())
    m_pPD->flags |= TMSRPT36_FLAG_ALLDAY;
  else
  {
    pEditFROMTIME->GetWindowText(tempString, sizeof(tempString));
    m_pPD->timeOfDayFrom = cTime(tempString);
    pEditTOTIME->GetWindowText(tempString, sizeof(tempString));
    m_pPD->timeOfDayTo = cTime(tempString);
  }
//
//  Driver
//
  if(pButtonALLOPERATORS->GetCheck())
    m_pPD->flags |= TMSRPT36_FLAG_ALLDRIVERS;
  else
  {
    nI = (int)pComboBoxJUSTOPERATORLIST->GetCurSel();
    if(nI != CB_ERR)
      m_pPD->DRIVERSrecordID = pComboBoxJUSTOPERATORLIST->GetItemData(nI);
  }
//
//  Exclude driver
//
  if(pButtonDONOTEXCLUDE->GetCheck())
    m_pPD->flags |= TMSRPT36_FLAG_DONOTEXCLUDE;
  else
  {
    nI = (int)pComboBoxJUSTEXCLUDELIST->GetCurSel();
    if(nI != CB_ERR)
      m_pPD->excludedDRIVERSrecordID = pComboBoxJUSTEXCLUDELIST->GetItemData(nI);
  }
//
//  Bustype
//
  if(pButtonALLBUSTYPES->GetCheck())
    m_pPD->flags |= TMSRPT36_FLAG_ALLBUSTYPES;
  else
  {
    nI = (int)pComboBoxJUSTBUSTYPELIST->GetCurSel();
    if(nI != CB_ERR)
      m_pPD->BUSTYPESrecordID = pComboBoxJUSTBUSTYPELIST->GetItemData(nI);
  }
//
//  Passenger load
//
  if(pButtonALLPASSENGERLOADS->GetCheck())
    m_pPD->flags |= TMSRPT36_FLAG_ALLPASSENGERLOADS;
  else
  {
    nI = (int)pComboBoxJUSTPASSENGERLOADLIST->GetCurSel();
    if(nI != CB_ERR)
    {
      if(pComboBoxJUSTPASSENGERLOADLIST->GetItemData(nI) == 0)  // "N/A"
        m_pPD->flags |= TMSRPT36_FLAG_ALLPASSENGERLOADS;
      else
        m_pPD->passengerLoadIndex = pComboBoxJUSTPASSENGERLOADLIST->GetItemData(nI);
    }
  }
//
//  Road supervisor
//
  if(pButtonALLROADSUPERVISORS->GetCheck())
    m_pPD->flags |= TMSRPT36_FLAG_ALLROADSUPERVISORS;
  else
  {
    nI = (int)pComboBoxJUSTROADSUPERVISORLIST->GetCurSel();
    if(nI != CB_ERR)
    {
      if(pComboBoxJUSTROADSUPERVISORLIST->GetItemData(nI) == 0)  // "N/A"
        m_pPD->flags |= TMSRPT36_FLAG_ALLROADSUPERVISORS;
      else
        m_pPD->roadSupervisorIndex = pComboBoxJUSTROADSUPERVISORLIST->GetItemData(nI);
    }
  }
//
//  All done
//
	CDialog::OnOK();
}

void COnTimePerformance::OnHelp() 
{
	// TODO: Add your control notification handler code here
	
}

void COnTimePerformance::OnCancel() 
{
	// TODO: Add extra cleanup here
	
	CDialog::OnCancel();
}

void COnTimePerformance::OnAllservices() 
{
  pComboBoxJUSTSERVICELIST->SetCurSel(-1);
}

void COnTimePerformance::OnAlldays() 
{
  pComboBoxJUSTDAYLIST->SetCurSel(-1);
}

void COnTimePerformance::OnAlltimepoints() 
{
  pComboBoxJUSTTIMEPOINTLIST->SetCurSel(-1);
}

void COnTimePerformance::OnAllroutes() 
{
  pComboBoxJUSTROUTELIST->SetCurSel(-1);
  pButtonOB->SetWindowText("OB");
  pButtonIB->SetWindowText("IB");
}

void COnTimePerformance::OnAllday() 
{
  pEditFROMTIME->EnableWindow(FALSE);
  pEditTOTIME->EnableWindow(FALSE);
}

void COnTimePerformance::OnAlloperators() 
{
  pComboBoxJUSTOPERATORLIST->SetCurSel(-1);
}

void COnTimePerformance::OnDonotexclude() 
{
  pComboBoxJUSTEXCLUDELIST->SetCurSel(-1);
}

void COnTimePerformance::OnAllbustypes() 
{
  pComboBoxJUSTBUSTYPELIST->SetCurSel(-1);
}

void COnTimePerformance::OnAllpassengerloads() 
{
  pComboBoxJUSTPASSENGERLOADLIST->SetCurSel(0);
}

void COnTimePerformance::OnAllroadsupervisors() 
{
  pComboBoxJUSTROADSUPERVISORLIST->SetCurSel(0);
}

void COnTimePerformance::OnSelchangeJustbustypelist() 
{
  pButtonALLBUSTYPES->SetCheck(FALSE);
  pButtonJUSTBUSTYPE->SetCheck(TRUE);
}

void COnTimePerformance::OnSelchangeJustdaylist() 
{
  pButtonALLDAYS->SetCheck(FALSE);
  pButtonJUSTDAY->SetCheck(TRUE);
}

void COnTimePerformance::OnSelchangeJustexcludelist() 
{
  pButtonDONOTEXCLUDE->SetCheck(FALSE);
  pButtonJUSTEXCLUDE->SetCheck(TRUE);
}

void COnTimePerformance::OnSelchangeJustoperatorlist() 
{
  pButtonALLOPERATORS->SetCheck(FALSE);
  pButtonJUSTOPERATOR->SetCheck(TRUE);
}

void COnTimePerformance::OnSelchangeJustpassengerloadlist() 
{
  pButtonALLPASSENGERLOADS->SetCheck(FALSE);
  pButtonJUSTPASSENGERLOAD->SetCheck(TRUE);
}

void COnTimePerformance::OnSelchangeJustroadsupervisorlist() 
{
  pButtonALLROADSUPERVISORS->SetCheck(FALSE);
  pButtonJUSTROADSUPERVISOR->SetCheck(TRUE);;
}

void COnTimePerformance::OnSelchangeJustroutelist() 
{
  int nI;
  int rcode2;

  pButtonALLROUTES->SetCheck(FALSE);
  pButtonJUSTROUTE->SetCheck(TRUE);
  pButtonIB->EnableWindow(TRUE);
  pButtonBOTHDIRECTIONS->EnableWindow(TRUE);
  
  nI = pComboBoxJUSTROUTELIST->GetCurSel();
  if(nI != CB_ERR)
  {
    ROUTESKey0.recordID = pComboBoxJUSTROUTELIST->GetItemData(nI);
    rcode2 = btrieve(B_GETEQUAL, TMS_ROUTES, &ROUTES, &ROUTESKey0, 0);
    if(rcode2 == 0)
    {
      DIRECTIONSKey0.recordID = ROUTES.DIRECTIONSrecordID[0];
      btrieve(B_GETEQUAL, TMS_DIRECTIONS, &DIRECTIONS, &DIRECTIONSKey0, 0);
      strncpy(tempString, DIRECTIONS.abbrName, DIRECTIONS_ABBRNAME_LENGTH);
      trim(tempString, DIRECTIONS_ABBRNAME_LENGTH);
      pButtonOB->SetWindowText(tempString);
      if(ROUTES.DIRECTIONSrecordID[1] == NO_RECORD)
      {
        pButtonOB->SetCheck(TRUE);
        pButtonIB->SetCheck(FALSE);
        pButtonBOTHDIRECTIONS->SetCheck(FALSE);
        pButtonIB->EnableWindow(FALSE);
        pButtonBOTHDIRECTIONS->EnableWindow(FALSE);
      }
      else
      {
        DIRECTIONSKey0.recordID = ROUTES.DIRECTIONSrecordID[1];
        btrieve(B_GETEQUAL, TMS_DIRECTIONS, &DIRECTIONS, &DIRECTIONSKey0, 0);
        strncpy(tempString, DIRECTIONS.abbrName, DIRECTIONS_ABBRNAME_LENGTH);
        trim(tempString, DIRECTIONS_ABBRNAME_LENGTH);
        pButtonIB->SetWindowText(tempString);
      }
    }
  }
}

void COnTimePerformance::OnSelchangeJustservicelist() 
{
  pButtonALLSERVICES->SetCheck(FALSE);
  pButtonJUSTSERVICE->SetCheck(TRUE);
}

void COnTimePerformance::OnSelchangeJusttimepointlist() 
{
  pButtonALLTIMEPOINTS->SetCheck(FALSE);
  pButtonJUSTTIMEPOINT->SetCheck(TRUE);
}

void COnTimePerformance::OnJustbustype() 
{
  pComboBoxJUSTBUSTYPELIST->SetCurSel(0);
}

void COnTimePerformance::OnJustday() 
{
  pButtonALLDAYS->SetCheck(TRUE);
  pButtonJUSTDAY->SetCheck(FALSE);
}

void COnTimePerformance::OnJustexclude() 
{
  pComboBoxJUSTEXCLUDELIST->SetCurSel(0);
}

void COnTimePerformance::OnJustoperator() 
{
  pComboBoxJUSTOPERATORLIST->SetCurSel(0);
}

void COnTimePerformance::OnJustpassengerload() 
{
}

void COnTimePerformance::OnJustroadsupervisor() 
{
}

void COnTimePerformance::OnJustroute() 
{
  pComboBoxJUSTROUTELIST->SetCurSel(0);
  OnSelchangeJustroutelist();
}

void COnTimePerformance::OnJustservice() 
{
  pComboBoxJUSTSERVICELIST->SetCurSel(0);
}

void COnTimePerformance::OnJusttimepoint() 
{
  pComboBoxJUSTTIMEPOINTLIST->SetCurSel(0);
}


void COnTimePerformance::OnFrom() 
{
  pEditFROMTIME->EnableWindow(TRUE);
  pEditTOTIME->EnableWindow(TRUE);
} 

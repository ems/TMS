//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
// CustomerComment.cpp : implementation file
//

#include "stdafx.h"
extern "C"
{
#include "TMSHeader.h"
}
#include "cc.h"

extern CStdioFile WeatherConditions;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CCustomerComment dialog


CCustomerComment::CCustomerComment(CWnd* pParent /*=NULL*/, CCFEDef* pCCFE)
	: CDialog(CCustomerComment::IDD, pParent)
{
  m_pCCFE = pCCFE;

	//{{AFX_DATA_INIT(CCustomerComment)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CCustomerComment::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CCustomerComment)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CCustomerComment, CDialog)
	//{{AFX_MSG_MAP(CCustomerComment)
	ON_BN_CLICKED(IDCLOSE, OnClose)
	ON_BN_CLICKED(IDHELP, OnHelp)
	ON_BN_CLICKED(IDNEW, OnNew)
	ON_BN_CLICKED(IDFIND, OnFind)
	ON_BN_CLICKED(CC_HISTORYNO, OnHistoryno)
	ON_BN_CLICKED(CC_HISTORYYES, OnHistoryyes)
	ON_BN_CLICKED(CC_HISTORYNOTSURE, OnHistorynotsure)
	ON_BN_CLICKED(CC_HISTORYTHISMANY, OnHistorythismany)
	ON_CBN_SELCHANGE(CC_ROUTE, OnSelchangeRoute)
	ON_BN_CLICKED(CC_NASPEEDING, OnNaspeeding)
	ON_BN_CLICKED(CC_SPEEDINGYES, OnSpeedingyes)
	ON_BN_CLICKED(IDUPDATECUSTOMER, OnUpdatecustomer)
	ON_BN_CLICKED(IDPROCESS, OnProcess)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCustomerComment message handlers

BOOL CCustomerComment::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
  pGroupBoxTRACKINGNUMBER = GetDlgItem(CC_TRACKINGNUMBER);
  pStaticRECEIVEDBY = (CStatic *)GetDlgItem(CC_RECEIVEDBY);
  pButtonIDUPDATECUSTOMER = (CButton *)GetDlgItem(IDUPDATECUSTOMER);
  pButtonIDFIND = (CButton *)GetDlgItem(IDFIND);
  pStaticNAME = (CStatic *)GetDlgItem(CC_NAME);
  pStaticADDRESS = (CStatic *)GetDlgItem(CC_ADDRESS);
  pStaticCITYSTATE = (CStatic *)GetDlgItem(CC_CITYSTATE);
  pStaticZIP = (CStatic *)GetDlgItem(CC_ZIP);
  pStaticDAYPHONE = (CStatic *)GetDlgItem(CC_DAYPHONE);
  pStaticNIGHTPHONE = (CStatic *)GetDlgItem(CC_NIGHTPHONE);
  pStaticFAX = (CStatic *)GetDlgItem(CC_FAX);
  pStaticEMAIL = (CStatic *)GetDlgItem(CC_EMAIL);
  pButtonHISTORYNO = (CButton *)GetDlgItem(CC_HISTORYNO);
  pButtonHISTORYYES = (CButton *)GetDlgItem(CC_HISTORYYES);
  pStaticHISTORYHOWMANYTIMES = (CStatic *)GetDlgItem(CC_HISTORYHOWMANYTIMES);
  pButtonHISTORYNOTSURE = (CButton *)GetDlgItem(CC_HISTORYNOTSURE);
  pButtonHISTORYTHISMANY = (CButton *)GetDlgItem(CC_HISTORYTHISMANY);
  pEditHISTORYTHISMANYNUMBER = (CEdit *)GetDlgItem(CC_HISTORYTHISMANYNUMBER);
  pDTPickerDETAILDATE = (CDTPicker *)GetDlgItem(CC_DETAILDATE);
  pDTPickerDETAILTIME = (CDTPicker *)GetDlgItem(CC_DETAILTIME);
  pEditVEHICLENUMBER = (CEdit *)GetDlgItem(CC_VEHICLENUMBER);
  pEditLOCATION = (CEdit *)GetDlgItem(CC_LOCATION);
  pComboBoxROUTE = (CComboBox *)GetDlgItem(CC_ROUTE);
  pStaticDIRECTION_TEXT = (CStatic *)GetDlgItem(CC_DIRECTION_TEXT);
  pButtonNADIRECTION = (CButton *)GetDlgItem(CC_NADIRECTION);
  pButtonOUTBOUND = (CButton *)GetDlgItem(CC_OUTBOUND);
  pButtonINBOUND = (CButton *)GetDlgItem(CC_INBOUND);
  pComboBoxWEATHER = (CComboBox *)GetDlgItem(CC_WEATHER);
  pButtonEMPLOYEEMALE = (CButton *)GetDlgItem(CC_EMPLOYEEMALE);
  pButtonEMPLOYEEFEMALE = (CButton *)GetDlgItem(CC_EMPLOYEEFEMALE);
  pButtonNAEMPLOYEE = (CButton *)GetDlgItem(CC_NAEMPLOYEE);
  pComboBoxEMPLOYEENAME = (CComboBox *)GetDlgItem(CC_EMPLOYEENAME);
  pEditEMPLOYEEDESCRIPTION = (CEdit *)GetDlgItem(CC_EMPLOYEEDESCRIPTION);
  pButtonNASPEEDING = (CButton *)GetDlgItem(CC_NASPEEDING);
  pButtonSPEEDINGNO = (CButton *)GetDlgItem(CC_SPEEDINGNO);
  pButtonSPEEDINGYES = (CButton *)GetDlgItem(CC_SPEEDINGYES);
  pEditSPEED = (CEdit *)GetDlgItem(CC_SPEED);
  pButtonNAATTENTION = (CButton *)GetDlgItem(CC_NAATTENTION);
  pButtonATTENTIONNO = (CButton *)GetDlgItem(CC_ATTENTIONNO);
  pButtonATTENTIONYES = (CButton *)GetDlgItem(CC_ATTENTIONYES);
  pButtonNATIMEDIFFERENCE = (CButton *)GetDlgItem(CC_NATIMEDIFFERENCE);
  pButtonLESSTHANTWO = (CButton *)GetDlgItem(CC_LESSTHANTWO);
  pButtonTWOTOFIVE = (CButton *)GetDlgItem(CC_TWOTOFIVE);
  pButtonMORETHANFIVE = (CButton *)GetDlgItem(CC_MORETHANFIVE);

  COleVariant v;
  char szDriver[DRIVERS_LASTNAME_LENGTH + 2 + DRIVERS_FIRSTNAME_LENGTH + 1];
  long flags;
  int  nI, nJ;
  int  numDrivers;
  int  numRoutes;
  int  rcode2;
  long day;
  long month;
  long year;
  long hours;
  long minutes;
  long seconds;

//
//  Set up the title bar
//
  GetWindowText(szFormatString, sizeof(szFormatString));
  strncpy(szarString, m_pCCFE->COMPLAINTS.referenceNumber, COMPLAINTS_REFERENCENUMBER_LENGTH);
  trim(szarString, COMPLAINTS_REFERENCENUMBER_LENGTH);
  sprintf(tempString, szFormatString, szarString);
  SetWindowText(tempString);
//
//  Put up the nature of the call
//
  if(m_pCCFE->flags & CCFE_FLAGS_NEW)
    flags = m_pCCFE->flags;
  else
  {
    COMPLAINTSKey0.recordID = m_pCCFE->COMPLAINTS.recordID;
    flags = m_pCCFE->COMPLAINTS.flags;
    GetYMD(m_pCCFE->COMPLAINTS.dateOfComplaint, &year, &month, &day);
    GetHMS(m_pCCFE->COMPLAINTS.timeOfComplaint, &hours, &minutes, &seconds);
    strncpy(tempString, m_pCCFE->COMPLAINTS.enteredBy, COMPLAINTS_ENTEREDBY_LENGTH);
    trim(tempString, COMPLAINTS_ENTEREDBY_LENGTH);
  }
//
//  Set up "received by"
//
  pStaticRECEIVEDBY->GetWindowText(szFormatString, sizeof(szFormatString));
  if(flags & CCFE_FLAGS_NEW)
  {
    if(flags & CCFE_FLAGS_COMPLAINT)
      nI = TEXT_156;
    else if(flags & CCFE_FLAGS_COMMENDATION)
      nI = TEXT_157;
    else if(flags & CCFE_FLAGS_SERVICEREQUEST)
      nI = TEXT_158;
    else if(flags & CCFE_FLAGS_MISCELLANEOUS)
      nI = TEXT_401;
  }
  else
  {
    if(m_pCCFE->COMPLAINTS.referenceNumber[0] == 'C')
      nI = TEXT_156;
    else if(m_pCCFE->COMPLAINTS.referenceNumber[0] == 'M')
      nI = TEXT_157;
    else if(m_pCCFE->COMPLAINTS.referenceNumber[0] == 'S')
      nI = TEXT_158;
    else if(m_pCCFE->COMPLAINTS.referenceNumber[0] == 'L')
      nI = TEXT_401;
  }
  
  CString s;
  s.LoadString(nI);
  if(m_pCCFE->flags & CCFE_FLAGS_NEW)
  {
    sprintf(szarString, szFormatString, s, m_pCCFE->szDateTime, m_pCCFE->szReceivedBy);
  }
  else
  {
    CTime time(year, month, day, hours, minutes, seconds);
    sprintf(szarString, szFormatString, s, 
          time.Format(_T("%A, %B %d, %Y at %I:%M:%S%p")), tempString);
  }
  pStaticRECEIVEDBY->SetWindowText(szarString);
//
//  If this is a new complaint or an anonymous complaintant, disable
//  button until he's selected one through the Find or New buttons
//
  if(m_pCCFE->COMPLAINTS.CUSTOMERSrecordID == NO_RECORD)
    pButtonIDUPDATECUSTOMER->EnableWindow(FALSE);
//
//  Load the drivers
//
//  Set the first driver to "Not known or N/A"
//
  s.LoadString(TEXT_238);
  numDrivers = pComboBoxEMPLOYEENAME->AddString(s);
  pComboBoxEMPLOYEENAME->SetItemData(numDrivers, NO_RECORD);
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
    numDrivers = pComboBoxEMPLOYEENAME->AddString(szDriver);
    if(numDrivers != CB_ERR)
      pComboBoxEMPLOYEENAME->SetItemData(numDrivers, DRIVERS.recordID);
    rcode2 = btrieve(B_GETNEXT, TMS_DRIVERS, &DRIVERS, &DRIVERSKey1, 1);
  }
  pComboBoxEMPLOYEENAME->SetCurSel(0);
//
//  Set up the route list
//
  numRoutes = SetUpRouteList(m_hWnd, CC_ROUTE, NO_RECORD);
  if(numRoutes == 0)
  {
    CDialog::OnCancel();
    return TRUE;
  }
//
//  Set the first route to "Not known or N/A"
//
  s.LoadString(TEXT_238);
  nI = pComboBoxROUTE->InsertString(0, s);
  pComboBoxROUTE->SetItemData(nI, NO_RECORD);
  pComboBoxROUTE->SetCurSel(0);
//
//  Set up the weather conditions
//
  if(m_numWeatherConditions == 0)
  {
    pComboBoxWEATHER->EnableWindow(FALSE);
  }
  else
  {
    for(nI = 0; nI < m_numWeatherConditions; nI++)
    {
      nJ = pComboBoxWEATHER->AddString(m_WeatherConditions[nI].szText);
      pComboBoxWEATHER->SetItemData(nJ, m_WeatherConditions[nI].number);
    }
  }
//
//  New complaints:
//
  if(m_pCCFE->flags & CCFE_FLAGS_NEW)
  {
//
//  And no customer yet
//
    m_customerRecordID = NO_RECORD;
//
//  Are there any customers
//
    rcode2 = btrieve(B_GETFIRST, TMS_CUSTOMERS, &CUSTOMERS, &CUSTOMERSKey0, 0);
    if(rcode2 != 0)
      pButtonIDFIND->EnableWindow(FALSE);
//
//  No route selected yet
//
    pStaticDIRECTION_TEXT->EnableWindow(FALSE);
    pButtonNADIRECTION->EnableWindow(FALSE);
    pButtonOUTBOUND->EnableWindow(FALSE);
    pButtonINBOUND->EnableWindow(FALSE);
//
//  Set today's date on the Date and Time picker
//
    CTime time = CTime::GetCurrentTime();
    day = time.GetDay();
    month = time.GetMonth();
    year = time.GetYear();

    v = year;
    pDTPickerDETAILDATE->SetYear(v);
    v = month;
    pDTPickerDETAILDATE->SetMonth(v);
    v = day;
    pDTPickerDETAILDATE->SetDay(v);
//
//  Default to "No History"
//
    pButtonHISTORYNO->SetCheck(TRUE);
    OnHistoryno();
//
//  Default weather conditions to "N/A"
//
    pComboBoxWEATHER->SetCurSel(0);
//
//  Default employee sex to "N/A or unknown"
//
    pButtonNAEMPLOYEE->SetCheck(TRUE);
//
//  Default "Driver speeding" to "N/A"
//
    pButtonNASPEEDING->SetCheck(TRUE);
    OnNaspeeding();
//
//  Default "Driver's attention" to "N/A"
//
    pButtonNAATTENTION->SetCheck(TRUE);
//
//  Default "Customer time difference" to "N/A"
//
    pButtonNATIMEDIFFERENCE->SetCheck(TRUE);
  }
//
//  Existing complaint - fill in the controls
//
  else
  {
    m_customerRecordID = m_pCCFE->COMPLAINTS.CUSTOMERSrecordID;
//
//  Customer data
//
    PopulateCustomerFields();
//
//  History
//
    if(!(m_pCCFE->COMPLAINTS.flags & COMPLAINTS_FLAG_FILEDPREVIOUS))
      pButtonHISTORYNO->SetCheck(TRUE);
    else
    {
      pButtonHISTORYYES->SetCheck(TRUE);
      if(m_pCCFE->COMPLAINTS.flags & COMPLAINTS_FLAG_NOTSUREHOWMANY)
        pButtonHISTORYNOTSURE->SetCheck(TRUE);
      else
      {
        pButtonHISTORYTHISMANY->SetCheck(TRUE);
        ltoa(m_pCCFE->COMPLAINTS.numberOfPreviousComplaints, tempString, 10);
        pEditHISTORYTHISMANYNUMBER->SetWindowText(tempString);
      }
    }
//
//  Date and time of occurrence
//
//  Date
//
    if(m_pCCFE->COMPLAINTS.dateOfOccurrence == 0)
    {
      TMSError(this->m_hWnd, MB_ICONINFORMATION | MB_OK, TEXT_377, (HANDLE)NULL);
    }
    else
    {
      GetYMD(m_pCCFE->COMPLAINTS.dateOfOccurrence, &year, &month, &day);
      v = year;
      pDTPickerDETAILDATE->SetYear(v);
      v = month;
      pDTPickerDETAILDATE->SetMonth(v);
      v = day;
      pDTPickerDETAILDATE->SetDay(v);
//
//  Time
//
      GetHMS(m_pCCFE->COMPLAINTS.timeOfOccurrence, &hours, &minutes, &seconds);
      v = hours;
      pDTPickerDETAILTIME->SetHour(v);
      v = minutes;
      pDTPickerDETAILTIME->SetMinute(v);
      v = seconds;
      pDTPickerDETAILTIME->SetSecond(v);
    }
//
//  Vehicle number
//
    strncpy(tempString, m_pCCFE->COMPLAINTS.busNumber, COMPLAINTS_BUSNUMBER_LENGTH);
    trim(tempString, COMPLAINTS_BUSNUMBER_LENGTH);
    pEditVEHICLENUMBER->SetWindowText(tempString);
//
//  Location
//
    strncpy(tempString, m_pCCFE->COMPLAINTS.location, COMPLAINTS_LOCATION_LENGTH);
    trim(tempString, COMPLAINTS_LOCATION_LENGTH);
    pEditLOCATION->SetWindowText(tempString);
//
//  Route
//
    long ROUTESrecordID = NO_RECORD;

    for(nI = 0; nI < numRoutes + 1; nI++) // +1 'cause "N/A" is the first route
    {
      ROUTESrecordID = pComboBoxROUTE->GetItemData(nI);
      if(ROUTESrecordID == m_pCCFE->COMPLAINTS.ROUTESrecordID)
      {
        pComboBoxROUTE->SetCurSel(nI);
        OnSelchangeRoute();
        pButtonNADIRECTION->SetCheck(FALSE);
        break;
      }
    }
//
//  Direction
//
    if(m_pCCFE->COMPLAINTS.directionIndex == NO_RECORD)
      pButtonNADIRECTION->SetCheck(TRUE);
    else if(m_pCCFE->COMPLAINTS.directionIndex == 0)
      pButtonOUTBOUND->SetCheck(TRUE);
    else
      pButtonINBOUND->SetCheck(TRUE);
//
//  Weather index
//
    if(m_pCCFE->COMPLAINTS.weatherIndex != NO_RECORD)
      pComboBoxWEATHER->SetCurSel(m_pCCFE->COMPLAINTS.weatherIndex);
//
//  Driver sex
//
    if(m_pCCFE->COMPLAINTS.flags & COMPLAINTS_FLAG_NOTSUREDRIVERSEX)
      pButtonNAEMPLOYEE->SetCheck(TRUE);
    else if(m_pCCFE->COMPLAINTS.flags & COMPLAINTS_FLAG_DRIVERMALE)
      pButtonEMPLOYEEMALE->SetCheck(TRUE);
    else
      pButtonEMPLOYEEFEMALE->SetCheck(TRUE);
//
//  Driver name
//
    long DRIVERSrecordID = NO_RECORD;

    for(nI = 0; nI < numDrivers; nI++)
    {
      DRIVERSrecordID = pComboBoxEMPLOYEENAME->GetItemData(nI);
      if(m_pCCFE->COMPLAINTS.DRIVERSrecordID == DRIVERSrecordID)
      {
        pComboBoxEMPLOYEENAME->SetCurSel(nI);
        break;
      }
    }
//
//  Driver description
//
    strncpy(tempString,
          m_pCCFE->COMPLAINTS.driverDescription, COMPLAINTS_DRIVERDESCRIPTION_LENGTH);
    trim(tempString, COMPLAINTS_DRIVERDESCRIPTION_LENGTH);
    pEditEMPLOYEEDESCRIPTION->SetWindowText(tempString);
//
//  Speeding
//
    if(m_pCCFE->COMPLAINTS.flags & COMPLAINTS_FLAG_NASPEEDING)
      pButtonNASPEEDING->SetCheck(TRUE);
    else if(!(m_pCCFE->COMPLAINTS.flags & COMPLAINTS_FLAG_DRIVERSPEEDINGYES))
      pButtonSPEEDINGNO->SetCheck(TRUE);
    else
    {
      pButtonSPEEDINGYES->SetCheck(TRUE);
      ltoa(m_pCCFE->COMPLAINTS.driverSpeed, tempString, 10);
      pEditSPEED->SetWindowText(tempString);
    }
//
//  Driver's attention
//
    if(m_pCCFE->COMPLAINTS.flags & COMPLAINTS_FLAG_NAATTENTION)
      pButtonNAATTENTION->SetCheck(TRUE);
    else if(m_pCCFE->COMPLAINTS.flags & COMPLAINTS_FLAG_ATTENTIONYES)
      pButtonATTENTIONYES->SetCheck(TRUE);
    else
      pButtonATTENTIONNO->SetCheck(TRUE);
//
//  Time difference
//
    if(m_pCCFE->COMPLAINTS.flags & COMPLAINTS_FLAG_LESSTHANTWO)
      pButtonLESSTHANTWO->SetCheck(TRUE);
    else if(m_pCCFE->COMPLAINTS.flags & COMPLAINTS_FLAG_TWOTOFIVE)
      pButtonTWOTOFIVE->SetCheck(TRUE);
    else if(m_pCCFE->COMPLAINTS.flags & COMPLAINTS_FLAG_MORETHANFIVE)
      pButtonMORETHANFIVE->SetCheck(TRUE);
    else
      pButtonNATIMEDIFFERENCE->SetCheck(TRUE);
  }
//
//  Make sure there's no junk in any new text entry
//
  strcpy(m_newEntry, "");
//
//  All done
//
  return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CCustomerComment::WinHelp(DWORD dwData, UINT nCmd) 
{
	// TODO: Add your specialized code here and/or call the base class
	
	CDialog::WinHelp(dwData, nCmd);
}

void CCustomerComment::OnClose() 
{
	OnCancel();
}

void CCustomerComment::OnCancel() 
{
  CString s;

  if(m_pCCFE->flags & CCFE_FLAGS_NEW)
  {
    if(s.LoadString(ERROR_140))  // This will mark this customer call as cancelled
    {
      if(MessageBox(s, TMS, MB_ICONQUESTION | MB_YESNO) == IDYES)
      {
        COMPLAINTSKey0.recordID = m_pCCFE->COMPLAINTS.recordID;
        recordLength[TMS_COMPLAINTS] = COMPLAINTS_TOTAL_LENGTH;
        btrieve(B_GETEQUAL, TMS_COMPLAINTS, pComplaintText, &COMPLAINTSKey0, 0);
        memcpy(&COMPLAINTS, pComplaintText, COMPLAINTS_FIXED_LENGTH);
        COMPLAINTS.flags |= COMPLAINTS_FLAG_CANCELLED;
        memcpy(pComplaintText, &COMPLAINTS, COMPLAINTS_FIXED_LENGTH);
        btrieve(B_UPDATE, TMS_COMPLAINTS, pComplaintText, &COMPLAINTSKey0, 0);
        recordLength[TMS_COMPLAINTS] = COMPLAINTS_FIXED_LENGTH;
        CDialog::OnCancel();
      }
    }
  }
  else
  {
    if(s.LoadString(ERROR_159))  // Are you sure?
    {
      if(MessageBox(s, TMS, MB_ICONQUESTION | MB_YESNO) == IDYES)
        CDialog::OnCancel();
    }
  }
}

void CCustomerComment::OnHelp() 
{
  ::WinHelp(m_hWnd, szarHelpFile, HELP_CONTEXT, The_Main_CCM_Dialog_1_);
}

void CCustomerComment::OnNew() 
{
  memset(&CUSTOMERS, 0x00, sizeof(CUSTOMERSDef));
  CUSTOMERS.recordID = NO_RECORD;

  CCustomer dlg(this, &CUSTOMERS);

  dlg.DoModal();

  if(CUSTOMERS.recordID == NO_RECORD)
    return;

  m_customerRecordID = CUSTOMERS.recordID;
  PopulateCustomerFields();
  pButtonIDFIND->EnableWindow(TRUE);  // Now there's at least one entry
  pButtonIDUPDATECUSTOMER->EnableWindow(TRUE);
}

void CCustomerComment::OnFind() 
{
  long recordID;

  CFindCustomer dlg(this, &recordID);

  dlg.DoModal();

  if(recordID != NO_RECORD)
  {
    m_customerRecordID = recordID;
    PopulateCustomerFields();
    pButtonIDUPDATECUSTOMER->EnableWindow(TRUE);
  }
}

void CCustomerComment::OnHistoryno() 
{
  pStaticHISTORYHOWMANYTIMES->EnableWindow(FALSE);
  pButtonHISTORYNOTSURE->EnableWindow(FALSE);
  pButtonHISTORYTHISMANY->EnableWindow(FALSE);
  pButtonHISTORYNOTSURE->SetCheck(FALSE);
  pButtonHISTORYTHISMANY->SetCheck(FALSE);
  pEditHISTORYTHISMANYNUMBER->SetWindowText("");
  pEditHISTORYTHISMANYNUMBER->EnableWindow(FALSE);
}

void CCustomerComment::OnHistoryyes() 
{
  pStaticHISTORYHOWMANYTIMES->EnableWindow(TRUE);
  pButtonHISTORYNOTSURE->EnableWindow(TRUE);
  pButtonHISTORYTHISMANY->EnableWindow(TRUE);
  pButtonHISTORYNOTSURE->SetCheck(TRUE);
  pButtonHISTORYTHISMANY->SetCheck(FALSE);
  pEditHISTORYTHISMANYNUMBER->SetWindowText("");
  pEditHISTORYTHISMANYNUMBER->EnableWindow(FALSE);
}

void CCustomerComment::OnHistorynotsure() 
{
  pEditHISTORYTHISMANYNUMBER->SetWindowText("");
  pEditHISTORYTHISMANYNUMBER->EnableWindow(FALSE);
}

void CCustomerComment::OnHistorythismany() 
{
  pEditHISTORYTHISMANYNUMBER->EnableWindow(TRUE);
}

void CCustomerComment::OnSelchangeRoute() 
{
  int nI = pComboBoxROUTE->GetCurSel();

  if(nI == CB_ERR)
  {
    pStaticDIRECTION_TEXT->EnableWindow(FALSE);
    pButtonNADIRECTION->SetCheck(FALSE);
    pButtonNADIRECTION->EnableWindow(FALSE);
    pButtonOUTBOUND->SetCheck(FALSE);
    pButtonOUTBOUND->EnableWindow(FALSE);
    pButtonINBOUND->SetCheck(FALSE);
    pButtonINBOUND->EnableWindow(FALSE);
  }
  else
  {
    pStaticDIRECTION_TEXT->EnableWindow(TRUE);
    pButtonNADIRECTION->EnableWindow(TRUE);
    pButtonNADIRECTION->SetCheck(TRUE);
    pButtonOUTBOUND->SetCheck(FALSE);
    pButtonINBOUND->SetCheck(FALSE);
    ROUTESKey0.recordID = pComboBoxROUTE->GetItemData(nI);
    if(btrieve(B_GETEQUAL, TMS_ROUTES, &ROUTES, &ROUTESKey0, 0) == 0)
    {
      CButton* pButton;
      for(int nJ = 0; nJ < 2; nJ++)
      {
        pButton = (nJ == 0 ? pButtonOUTBOUND : pButtonINBOUND);
        if(ROUTES.DIRECTIONSrecordID[nJ] == NO_RECORD)
        {
          pButton->SetWindowText("");
          pButton->EnableWindow(FALSE);
        }
        else
        {
          DIRECTIONSKey0.recordID = ROUTES.DIRECTIONSrecordID[nJ];
          btrieve(B_GETEQUAL, TMS_DIRECTIONS, &DIRECTIONS, &DIRECTIONSKey0, 0);
          strncpy(tempString, DIRECTIONS.longName, DIRECTIONS_LONGNAME_LENGTH);
          trim(tempString, DIRECTIONS_LONGNAME_LENGTH);
          pButton->SetWindowText(tempString);
          pButton->EnableWindow(TRUE);
        }
      }
    }
  }
}

void CCustomerComment::OnNaspeeding() 
{
  pEditSPEED->SetWindowText("");
  pEditSPEED->EnableWindow(FALSE);
}

void CCustomerComment::OnSpeedingyes() 
{
  pEditSPEED->EnableWindow(TRUE);	
}

void CCustomerComment::OnUpdatecustomer() 
{
  CUSTOMERSKey0.recordID = m_customerRecordID;
  btrieve(B_GETEQUAL, TMS_CUSTOMERS, &CUSTOMERS, &CUSTOMERSKey0, 0);

  CCustomer dlg(this, &CUSTOMERS);

  if(dlg.DoModal() == IDOK)
    PopulateCustomerFields();
}

void CCustomerComment::PopulateCustomerFields()
{
  CString s;
  int  rcode2;

  if(m_customerRecordID == NO_RECORD)
  {
    BlankCustomerFields();
    return;
  }

  CUSTOMERSKey0.recordID = m_customerRecordID;
  rcode2 = btrieve(B_GETEQUAL, TMS_CUSTOMERS, &CUSTOMERS, &CUSTOMERSKey0, 0);
  if(rcode2 != 0)
  {
    BlankCustomerFields();
    return;
  }

  strncpy(tempString, CUSTOMERS.salutation, CUSTOMERS_SALUTATION_LENGTH);
  trim(tempString, CUSTOMERS_SALUTATION_LENGTH);
  s = tempString;
  s += " ";
  strncpy(tempString, CUSTOMERS.firstName, CUSTOMERS_FIRSTNAME_LENGTH);
  trim(tempString, CUSTOMERS_FIRSTNAME_LENGTH);
  s += tempString;
  s += " ";
  strncpy(tempString, CUSTOMERS.lastName, CUSTOMERS_LASTNAME_LENGTH);
  trim(tempString, CUSTOMERS_LASTNAME_LENGTH);
  s += tempString;
  pStaticNAME->SetWindowText(s);

  strncpy(tempString, CUSTOMERS.streetAddress, CUSTOMERS_STREETADDRESS_LENGTH);
  trim(tempString, CUSTOMERS_STREETADDRESS_LENGTH);
  pStaticADDRESS->SetWindowText(tempString);

  strncpy(tempString, CUSTOMERS.city, CUSTOMERS_CITY_LENGTH);
  trim(tempString, CUSTOMERS_CITY_LENGTH);
  s = tempString;
  s += ", ";
  strncpy(tempString, CUSTOMERS.state, CUSTOMERS_STATE_LENGTH);
  trim(tempString, CUSTOMERS_STATE_LENGTH);
  s += tempString;
  pStaticCITYSTATE->SetWindowText(s);

  strncpy(tempString, CUSTOMERS.ZIP, CUSTOMERS_ZIP_LENGTH);
  trim(tempString, CUSTOMERS_ZIP_LENGTH);
  pStaticZIP->SetWindowText(tempString);

  strcpy(tempString, PhoneString(CUSTOMERS.dayTelephoneArea, CUSTOMERS.dayTelephoneNumber));
  pStaticDAYPHONE->SetWindowText(tempString);

  strcpy(tempString, PhoneString(CUSTOMERS.nightTelephoneArea, CUSTOMERS.nightTelephoneNumber));
  pStaticNIGHTPHONE->SetWindowText(tempString);
  
  strcpy(tempString, PhoneString(CUSTOMERS.faxTelephoneArea, CUSTOMERS.faxTelephoneNumber));
  pStaticFAX->SetWindowText(tempString);

  strncpy(tempString, CUSTOMERS.emailAddress, CUSTOMERS_EMAILADDRESS_LENGTH);
  trim(tempString, CUSTOMERS_EMAILADDRESS_LENGTH);
  pStaticEMAIL->SetWindowText(tempString);
}

void CCustomerComment::BlankCustomerFields()
{
  CString s;

  s.LoadString(TEXT_237);
  pStaticNAME->SetWindowText(s);
  pStaticADDRESS->SetWindowText("");
  pStaticCITYSTATE->SetWindowText("");
  pStaticZIP->SetWindowText("");
  pStaticDAYPHONE->SetWindowText("");
  pStaticNIGHTPHONE->SetWindowText("");
  pStaticFAX->SetWindowText("");
  pStaticEMAIL->SetWindowText("");
}

void CCustomerComment::OnProcess() 
{
//
//  Get the snapshot
//
  CString s;
  VARIANT v;
  long    year;
  long    month;
  long    day;
  long hours;
  long minutes;
  long seconds;
  int  nI;
//
//  Comment
//
  m_pCCFE->SNAPSHOT.COMMENTSrecordID = m_pCCFE->COMPLAINTS.COMMENTSrecordID;
//
//  Customer
//
  m_pCCFE->SNAPSHOT.CUSTOMERSrecordID = m_customerRecordID;
//
//  Number of previous complaints
//
  pEditHISTORYTHISMANYNUMBER->GetWindowText(s);
  m_pCCFE->SNAPSHOT.numberOfPreviousComplaints = atoi(s);
//
//  Date of Occurrence
//
  v = pDTPickerDETAILDATE->GetYear();
  year = v.lVal;
  v = pDTPickerDETAILDATE->GetMonth();
  month = v.lVal;
  v = pDTPickerDETAILDATE->GetDay();
  day = v.lVal;
  m_pCCFE->SNAPSHOT.dateOfOccurrence = year * 10000 + month * 100 + day;
//
//  Time of Occurrence
//
  v = pDTPickerDETAILTIME->GetHour();
  hours = v.lVal;
  v = pDTPickerDETAILTIME->GetMinute();
  minutes = v.lVal;
  v = pDTPickerDETAILTIME->GetSecond();
  seconds = v.lVal;
  m_pCCFE->SNAPSHOT.timeOfOccurrence = hours * 3600 + minutes * 60 + seconds;
//
//  Weather conditions
//
  nI = pComboBoxWEATHER->GetCurSel();
  m_pCCFE->SNAPSHOT.weatherIndex = pComboBoxWEATHER->GetItemData(nI);
  pComboBoxWEATHER->GetLBText(nI, tempString);
  if(strlen(tempString) > sizeof(m_pCCFE->szWeather))
    tempString[sizeof(m_pCCFE->szWeather) - 1] = '\0';
  strcpy(m_pCCFE->szWeather, tempString);
//
//  Bus number
//
  pEditVEHICLENUMBER->GetWindowText(tempString, TEMPSTRING_LENGTH);
  pad(tempString, COMPLAINTS_BUSNUMBER_LENGTH);
  strncpy(m_pCCFE->SNAPSHOT.busNumber, tempString, COMPLAINTS_BUSNUMBER_LENGTH);
//
//  Route record ID
//
  nI = pComboBoxROUTE->GetCurSel();
  m_pCCFE->SNAPSHOT.ROUTESrecordID =
        (nI <= 0 ? NO_RECORD : pComboBoxROUTE->GetItemData(nI));
//
//  Direction index
//
  if(pButtonNADIRECTION->GetCheck())
    m_pCCFE->SNAPSHOT.directionIndex = NO_RECORD;
  else
    m_pCCFE->SNAPSHOT.directionIndex = (pButtonOUTBOUND->GetCheck() ? 0 : 1);
//
//  Location
//
  pEditLOCATION->GetWindowText(tempString, TEMPSTRING_LENGTH);
  pad(tempString, COMPLAINTS_LOCATION_LENGTH);
  strncpy(m_pCCFE->SNAPSHOT.location, tempString, COMPLAINTS_LOCATION_LENGTH);
//
//  Driver record ID
//
  nI = pComboBoxEMPLOYEENAME->GetCurSel();
  m_pCCFE->SNAPSHOT.DRIVERSrecordID =
        (nI <= 0 ? NO_RECORD : pComboBoxEMPLOYEENAME->GetItemData(nI));
//
//  Driver description
//
  pEditEMPLOYEEDESCRIPTION->GetWindowText(tempString, TEMPSTRING_LENGTH);
  pad(tempString, COMPLAINTS_DRIVERDESCRIPTION_LENGTH);
  strncpy(m_pCCFE->SNAPSHOT.driverDescription, tempString, COMPLAINTS_DRIVERDESCRIPTION_LENGTH);
//
// Driver speed
//
  pEditSPEED->GetWindowText(s);
  m_pCCFE->SNAPSHOT.driverSpeed = atoi(s);  
//
//  Flags
//
  m_pCCFE->SNAPSHOT.flags = 0;
  if(m_pCCFE->flags & CCFE_FLAGS_NEW)
  {
    if(m_pCCFE->flags & CCFE_FLAGS_COMPLAINT)
      m_pCCFE->SNAPSHOT.flags = CCFE_FLAGS_COMPLAINT;
    else if(m_pCCFE->flags & CCFE_FLAGS_COMMENDATION)
      m_pCCFE->SNAPSHOT.flags = CCFE_FLAGS_COMMENDATION;
    else if(m_pCCFE->flags & CCFE_FLAGS_SERVICEREQUEST)
      m_pCCFE->SNAPSHOT.flags = CCFE_FLAGS_SERVICEREQUEST;
  }
//
//  Previous complaints
//
  if(pButtonHISTORYYES->GetCheck())
  {
    m_pCCFE->SNAPSHOT.flags |= COMPLAINTS_FLAG_FILEDPREVIOUS;
    if(pButtonHISTORYNOTSURE->GetCheck())
      m_pCCFE->SNAPSHOT.flags |= COMPLAINTS_FLAG_NOTSUREHOWMANY;
  }
//
//  Driver sex
//
  if(pButtonNAEMPLOYEE->GetCheck())
    m_pCCFE->SNAPSHOT.flags |= COMPLAINTS_FLAG_NOTSUREDRIVERSEX;
  else
  {
    if(pButtonEMPLOYEEMALE->GetCheck())
      m_pCCFE->SNAPSHOT.flags |= COMPLAINTS_FLAG_DRIVERMALE;
  }
//
//  Speeding
//
  if(pButtonNASPEEDING->GetCheck())
    m_pCCFE->SNAPSHOT.flags |= COMPLAINTS_FLAG_NASPEEDING;
  else
  {
    if(pButtonSPEEDINGYES->GetCheck())
      m_pCCFE->SNAPSHOT.flags |= COMPLAINTS_FLAG_DRIVERSPEEDINGYES;
  }
//
//  Attempt to get driver's attention
//
  if(pButtonNAATTENTION->GetCheck())
    m_pCCFE->SNAPSHOT.flags |= COMPLAINTS_FLAG_NAATTENTION;
  else
  {
    if(pButtonATTENTIONYES->GetCheck())
      m_pCCFE->SNAPSHOT.flags |= COMPLAINTS_FLAG_ATTENTIONYES;
  }
//
//  Time differential
//
  if(pButtonLESSTHANTWO->GetCheck())
    m_pCCFE->SNAPSHOT.flags |= COMPLAINTS_FLAG_LESSTHANTWO;
  else if(pButtonTWOTOFIVE->GetCheck())
    m_pCCFE->SNAPSHOT.flags |= COMPLAINTS_FLAG_TWOTOFIVE;
  else if(pButtonMORETHANFIVE->GetCheck())
    m_pCCFE->SNAPSHOT.flags |= COMPLAINTS_FLAG_MORETHANFIVE;
//
//  Fire up the dialog
//
  CCCProcess dlg(this, m_pCCFE, m_newEntry);

  if(dlg.DoModal() != IDBACK)
    CDialog::OnOK();
}

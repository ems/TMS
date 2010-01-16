// 
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
// AddDriver.cpp : implementation file
//

#include "stdafx.h"
extern "C"{
#include "TMSHeader.h"
}
#include "TMS.h"
#include "AVLInterface.h"
#include "AddDialogs.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CAddDriver dialog


CAddDriver::CAddDriver(CWnd* pParent /*=NULL*/, long *pUpdateRecordID)
	: CDialog(CAddDriver::IDD, pParent)
{
  m_pUpdateRecordID = pUpdateRecordID;
  m_bInit = TRUE;
	//{{AFX_DATA_INIT(CAddDriver)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CAddDriver::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAddDriver)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CAddDriver, CDialog)
	//{{AFX_MSG_MAP(CAddDriver)
	ON_WM_CLOSE()
	ON_BN_CLICKED(ADDDRIVER_CC, OnCc)
	ON_BN_CLICKED(ADDDRIVER_NACC, OnNacc)
	ON_CBN_SELENDOK(ADDDRIVER_COMMENTCODE, OnSelendokCommentcode)
	ON_BN_CLICKED(ADDDRIVER_DV, OnDv)
	ON_CBN_SELENDOK(ADDDRIVER_DRIVERTYPE, OnSelendokDrivertype)
	ON_BN_CLICKED(ADDDRIVER_NADT, OnNadt)
	ON_CBN_SELENDOK(ADDDRIVER_DIVISION, OnSelendokDivision)
	ON_BN_CLICKED(ADDDRIVER_NADV, OnNadv)
	ON_BN_CLICKED(IDHELP, OnHelp)
	ON_BN_CLICKED(ADDDRIVER_DT, OnDt)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAddDriver message handlers

BOOL CAddDriver::OnInitDialog() 
{
  COleVariant v;
  CString s;
  CString fileName;
  BOOL bFound;
  long day;
  long month;
  long year;
  int  nI;
  int  numComments;
  int  numDriverTypes;
  int  numDivisions;

	CDialog::OnInitDialog();

  m_dlgPropSheet.AddPage(&m_Page03);  // Personal Information
	m_dlgPropSheet.AddPage(&m_Page02);  // Company Information
	m_dlgPropSheet.AddPage(&m_Page01);  // License Data
  m_dlgPropSheet.AddPage(&m_Page04);  // Agency Information

	m_dlgPropSheet.Create(this, WS_CHILD | WS_VISIBLE, 0);
	m_dlgPropSheet.ModifyStyleEx (0, WS_EX_CONTROLPARENT);
	m_dlgPropSheet.ModifyStyle( 0, WS_TABSTOP );
  
  CRect rcSheet;
	GetDlgItem(ADDDRIVER_PSHEETAREA)->GetWindowRect(&rcSheet);
	ScreenToClient(&rcSheet);
	m_dlgPropSheet.SetWindowPos(NULL, rcSheet.left - 7, rcSheet.top - 7, 0, 0, 
	      SWP_NOZORDER | SWP_NOSIZE | SWP_NOACTIVATE );
//
//  Set up the pointers to the controls
//
//  Main page
//
  pButtonNADT = (CButton *)GetDlgItem(ADDDRIVER_NADT);
  pButtonDT = (CButton *)GetDlgItem(ADDDRIVER_DT);
  pComboBoxDRIVERTYPE = (CComboBox *)GetDlgItem(ADDDRIVER_DRIVERTYPE);
  pButtonNADV = (CButton *)GetDlgItem(ADDDRIVER_NADV);
  pButtonDV = (CButton *)GetDlgItem(ADDDRIVER_DV);
  pComboBoxDIVISION = (CComboBox *)GetDlgItem(ADDDRIVER_DIVISION);
  pButtonCC = (CButton *)GetDlgItem(ADDDRIVER_CC);
  pButtonNACC = (CButton *)GetDlgItem(ADDDRIVER_NACC);
  pComboBoxCOMMENTCODE = (CComboBox *)GetDlgItem(ADDDRIVER_COMMENTCODE);
//
//  Page 4 - Agency Data
//
  m_dlgPropSheet.SetActivePage(3);
  CPropertyPage* pps = m_dlgPropSheet.GetPage(3);
  pButtonSUPERVISOR = (CButton *)pps->GetDlgItem(ADDDRIVER_SUPERVISOR);
  pButtonSAFETYSENSITIVE = (CButton *)pps->GetDlgItem(ADDDRIVER_SAFETYSENSITIVE);
  pComboBoxEEOC = (CComboBox *)pps->GetDlgItem(ADDDRIVER_EEOC);
  pComboBoxUNION = (CComboBox *)pps->GetDlgItem(ADDDRIVER_UNION);
  pComboBoxSECTION = (CComboBox *)pps->GetDlgItem(ADDDRIVER_SECTION);
  pComboBoxPOSITION = (CComboBox *)pps->GetDlgItem(ADDDRIVER_POSITION);
//
//  Page 3 - License data
//
  m_dlgPropSheet.SetActivePage(2);
	pps = m_dlgPropSheet.GetPage(2);

  pEditLICENSENUMBER = (CEdit *)pps->GetDlgItem(ADDDRIVER_LICENSENUMBER);
  pDTPickerLICENSEEXPIRYDATE = (CDTPicker *)pps->GetDlgItem(ADDDRIVER_LICENSEEXPIRYDATE);
  pEditLICENSEPROVINCEOFISSUE = (CEdit *)pps->GetDlgItem(ADDDRIVER_LICENSEPROVINCEOFISSUE);
  pEditLICENSETYPE = (CEdit *)pps->GetDlgItem(ADDDRIVER_LICENSETYPE);
  pDTPickerPHYSICALEXPIRYDATE = (CDTPicker *)pps->GetDlgItem(ADDDRIVER_PHYSICALEXPIRYDATE);
  pEditBITMAPPATH = (CEdit *)pps->GetDlgItem(ADDDRIVER_BITMAPPATH);
	CRect brect;
	pps->GetDlgItem(ADDDRIVER_PHOTO)->GetWindowRect(brect);
	ScreenToClient(brect);
  m_bmpCtrl.Create(WS_CHILD, brect, pps);
//
//  Page 2 - Company information
//
  m_dlgPropSheet.SetActivePage(1);
	pps = m_dlgPropSheet.GetPage(1);

  pButtonHIREDATEBOX = (CButton *)pps->GetDlgItem(ADDDRIVER_HIREDATEBOX);
  pDTPickerHIREDATE = (CDTPicker *)pps->GetDlgItem(ADDDRIVER_HIREDATE);
  pButtonTRANSITHIREDATEBOX = (CButton *)pps->GetDlgItem(ADDDRIVER_TRANSITHIREDATEBOX);
  pDTPickerTRANSITHIREDATE = (CDTPicker *)pps->GetDlgItem(ADDDRIVER_TRANSITHIREDATE);
  pButtonFULLTIMEDATEBOX = (CButton *)pps->GetDlgItem(ADDDRIVER_FULLTIMEDATEBOX);
  pDTPickerFULLTIMEDATE = (CDTPicker *)pps->GetDlgItem(ADDDRIVER_FULLTIMEDATE);
  pButtonPROMOTIONDATEBOX = (CButton *)pps->GetDlgItem(ADDDRIVER_PROMOTIONDATEBOX);
  pDTPickerPROMOTIONDATE = (CDTPicker *)pps->GetDlgItem(ADDDRIVER_PROMOTIONDATE);
  pButtonSENDATEBOX = (CButton *)pps->GetDlgItem(ADDDRIVER_SENDATEBOX);
  pDTPickerSENDATE = (CDTPicker *)pps->GetDlgItem(ADDDRIVER_SENDATE);
  pButtonTERMINATIONDATEBOX = (CButton *)pps->GetDlgItem(ADDDRIVER_TERMINATIONDATEBOX);
  pDTPickerTERMINATIONDATE = (CDTPicker *)pps->GetDlgItem(ADDDRIVER_TERMINATIONDATE);
  pEditSENSORT = (CEdit *)pps->GetDlgItem(ADDDRIVER_SENSORT);
  pEditCREW = (CEdit *)pps->GetDlgItem(ADDDRIVER_CREW);
  pEditBADGE = (CEdit *)pps->GetDlgItem(ADDDRIVER_BADGE);
  pEditVACATION = (CEdit *)pps->GetDlgItem(ADDDRIVER_VACATION);
  pEditPENDINGVACATION = (CEdit *)pps->GetDlgItem(ADDDRIVER_PENDINGVACATION);
  pEditPERSONAL = (CEdit *)pps->GetDlgItem(ADDDRIVER_PERSONAL);
  pEditSICK = (CEdit *)pps->GetDlgItem(ADDDRIVER_SICK);
  pEditFLOAT = (CEdit *)pps->GetDlgItem(ADDDRIVER_FLOAT);
  pStaticVACATIONDAYS = (CStatic *)pps->GetDlgItem(ADDDRIVER_VACATIONDAYS);
  pStaticPENDINGVACATIONDAYS = (CStatic *)pps->GetDlgItem(ADDDRIVER_PENDINGVACATIONDAYS);
  pStaticPERSONALDAYS = (CStatic *)pps->GetDlgItem(ADDDRIVER_PERSONALDAYS);
  pStaticSICKDAYS = (CStatic *)pps->GetDlgItem(ADDDRIVER_SICKDAYS);
  pStaticFLOATDAYS = (CStatic *)pps->GetDlgItem(ADDDRIVER_FLOATDAYS);
//
//  Page 1 - Personal information
//
  m_dlgPropSheet.SetActivePage(0);
	pps = m_dlgPropSheet.GetPage(0);

  pEditLAST = (CEdit *)pps->GetDlgItem(ADDDRIVER_LAST);
  pEditFIRST = (CEdit *)pps->GetDlgItem(ADDDRIVER_FIRST);
  pEditINITIALS = (CEdit *)pps->GetDlgItem(ADDDRIVER_INITIALS);
  pEditADDRESS = (CEdit *)pps->GetDlgItem(ADDDRIVER_ADDRESS);
  pEditCITY = (CEdit *)pps->GetDlgItem(ADDDRIVER_CITY);
  pStaticPROVINCE_TEXT = (CStatic *)pps->GetDlgItem(ADDDRIVER_PROVINCE_TEXT);
  pEditPROVINCE = (CEdit *)pps->GetDlgItem(ADDDRIVER_PROVINCE);
  pStaticPOSTALCODE_TEXT = (CStatic *)pps->GetDlgItem(ADDDRIVER_POSTALCODE_TEXT);
  pEditPOSTALCODE = (CEdit *)pps->GetDlgItem(ADDDRIVER_POSTALCODE);
  pMSMaskPHONE = (CMSMask *)pps->GetDlgItem(ADDDRIVER_PHONE);
  pMSMaskFAX = (CMSMask *)pps->GetDlgItem(ADDDRIVER_FAX);
  pMSMaskEMERGENCY = (CMSMask *)pps->GetDlgItem(ADDDRIVER_EMERGENCY);
  pEditEMERGENCYCONTACT = (CEdit *)pps->GetDlgItem(ADDDRIVER_EMERGENCYCONTACT);
  pStaticSIN_TEXT = (CStatic *)pps->GetDlgItem(ADDDRIVER_SIN_TEXT);
  pEditSIN = (CEdit *)pps->GetDlgItem(ADDDRIVER_SIN);
  pEditEMAIL = (CEdit *)pps->GetDlgItem(ADDDRIVER_EMAIL);
  pDTPickerDATEOFBIRTH = (CDTPicker *)pps->GetDlgItem(ADDDRIVER_DATEOFBIRTH);
  pComboBoxSEX = (CComboBox *)pps->GetDlgItem(ADDDRIVER_SEX);
  pComboBoxRACE = (CComboBox *)pps->GetDlgItem(ADDDRIVER_RACE);
//
//  Comment codes combo box
//
  numComments = SetUpCommentList(m_hWnd, ADDDRIVER_COMMENTCODE);
  if(numComments == 0)
  {
    pButtonCC->EnableWindow(FALSE);
    pComboBoxCOMMENTCODE->EnableWindow(FALSE);
  }
//
//  Driver types combo box
//
  numDriverTypes = SetUpDriverTypeList(m_hWnd, ADDDRIVER_DRIVERTYPE, NO_RECORD);
  if(numDriverTypes == 0)
  {
    pButtonDT->EnableWindow(FALSE);
    pComboBoxDRIVERTYPE->EnableWindow(FALSE);
  }
//
//  Divisions combo box
//
  numDivisions = SetUpDivisionList(m_hWnd, ADDDRIVER_DIVISION, NO_RECORD);
  if(numDivisions == 0)
  {
    pButtonDV->EnableWindow(FALSE);
    pComboBoxDIVISION->EnableWindow(FALSE);
  }
//
//  PROVINCE_TEXT display
//
  GetPrivateProfileString((LPSTR)userName, (LPSTR)"StateDisplay", "State",
        tempString, TEMPSTRING_LENGTH, szDatabaseFileName);
  s = tempString;
  s += ":";
  pStaticPROVINCE_TEXT->SetWindowText(s);
//
//  POSTALCODE_TEXT display
//
  GetPrivateProfileString((LPSTR)userName, (LPSTR)"ZIPDisplay", "ZIP",
        tempString, TEMPSTRING_LENGTH, szDatabaseFileName);
  s = tempString;
  s += ":";
  pStaticPOSTALCODE_TEXT->SetWindowText(s);
//
//  SIN_TEXT display
//
  GetPrivateProfileString((LPSTR)userName, (LPSTR)"SSNDisplay", "SSN",
        tempString, TEMPSTRING_LENGTH, szDatabaseFileName);
  s = tempString;
  s += ":";
  pStaticSIN_TEXT->SetWindowText(s);
//
//  Are we updating?
//
  CTime time = CTime::GetCurrentTime();
//
//  No
//
  if(*m_pUpdateRecordID == NO_RECORD)
  {
    pButtonNACC->SetCheck(TRUE);
    pButtonNADV->SetCheck(TRUE);
    pButtonNADT->SetCheck(TRUE);
//
//  Set today's date on the Date and Time pickers
//
    day = time.GetDay();
    month = time.GetMonth();
    year = time.GetYear();

    v = year;
    pDTPickerHIREDATE->SetYear(v);
    pDTPickerTRANSITHIREDATE->SetYear(v);
    pDTPickerFULLTIMEDATE->SetYear(v);
    pDTPickerPROMOTIONDATE->SetYear(v);
    pDTPickerSENDATE->SetYear(v);
    pDTPickerTERMINATIONDATE->SetYear(v);
    pDTPickerDATEOFBIRTH->SetYear(v);
    v = month;
    pDTPickerHIREDATE->SetMonth(v);
    pDTPickerTRANSITHIREDATE->SetMonth(v);
    pDTPickerFULLTIMEDATE->SetMonth(v);
    pDTPickerPROMOTIONDATE->SetMonth(v);
    pDTPickerSENDATE->SetMonth(v);
    pDTPickerTERMINATIONDATE->SetMonth(v);
    pDTPickerDATEOFBIRTH->SetMonth(v);
    v = day;
    pDTPickerHIREDATE->SetDay(v);
    pDTPickerTRANSITHIREDATE->SetDay(v);
    pDTPickerFULLTIMEDATE->SetDay(v);
    pDTPickerPROMOTIONDATE->SetDay(v);
    pDTPickerSENDATE->SetDay(v);
    pDTPickerTERMINATIONDATE->SetDay(v);
    pDTPickerDATEOFBIRTH->SetDay(v);
//
//  Race and sex
//
    pComboBoxSEX->SetCurSel(0);
    pComboBoxRACE->SetCurSel(0);
//
//  Supervisor
//
    pButtonSUPERVISOR->SetCheck(FALSE);
//
//  Saftey sensitive
//
    pButtonSAFETYSENSITIVE->SetCheck(FALSE);
//
//  EEOC
//
    pComboBoxEEOC->SetCurSel(0);
//
//  Labour Union
//
    pComboBoxUNION->SetCurSel(0);
//
//  Section
//
    pComboBoxSECTION->SetCurSel(0);
//
//  Position
//
    pComboBoxPOSITION->SetCurSel(0);
  }
//
//  Yes - get the record
//
  else
  {
    DRIVERSKey0.recordID = *m_pUpdateRecordID;
    btrieve(B_GETEQUAL, TMS_DRIVERS, &DRIVERS, &DRIVERSKey0, 0);
//
//  Comment
//
    if(DRIVERS.COMMENTSrecordID == NO_RECORD)
      pButtonNACC->SetCheck(TRUE);
    else
    {
      for(bFound = FALSE, nI = 0; nI < numComments; nI++)
      {
        if((long)pComboBoxCOMMENTCODE->GetItemData(nI) == DRIVERS.COMMENTSrecordID)
        {
          pComboBoxCOMMENTCODE->SetCurSel(nI);
          bFound = TRUE;
          break;
        }
      }
      CButton* pb = (bFound ? pButtonCC : pButtonNACC);
      pb->SetCheck(TRUE);
    }
//
//  Division
//
    if(DRIVERS.DIVISIONSrecordID == NO_RECORD)
      pButtonNADV->SetCheck(TRUE);
    else
    {
      for(bFound = FALSE, nI = 0; nI < numDivisions; nI++)
      {
        if((long)pComboBoxDIVISION->GetItemData(nI) == DRIVERS.DIVISIONSrecordID)
        {
          pComboBoxDIVISION->SetCurSel(nI);
          bFound = TRUE;
          break;
        }
      }
      CButton* pb = (bFound ? pButtonDV : pButtonNADV);
      pb->SetCheck(TRUE);
    }
//
//  Driver type
//
    if(DRIVERS.DRIVERTYPESrecordID == NO_RECORD)
      pButtonNADT->SetCheck(TRUE);
    else
    {
      for(bFound = FALSE, nI = 0; nI < numDriverTypes; nI++)
      {
        if((long)pComboBoxDRIVERTYPE->GetItemData(nI) == DRIVERS.DRIVERTYPESrecordID)
        {
          pComboBoxDRIVERTYPE->SetCurSel(nI);
          bFound = TRUE;
          break;
        }
      }
      CButton* pb = (bFound ? pButtonDT : pButtonNADT);
      pb->SetCheck(TRUE);
    }
//
//  License information
//
    s = GetDatabaseString(DRIVERS.licenseNumber, DRIVERS_LICENSENUMBER_LENGTH);
    pEditLICENSENUMBER->SetWindowText(s);
    if(DRIVERS.licenseExpiryDate != 0)
    {
      GetYMD(DRIVERS.licenseExpiryDate, &year, &month, &day);
      if((year >= 1900 && year <= 2038) && (month >= 1 && month <= 12) && (day >= 1 && day <= 31))
      {
        v = year;
        pDTPickerLICENSEEXPIRYDATE->SetYear(v);
        v = month;
        pDTPickerLICENSEEXPIRYDATE->SetMonth(v);
        v = day;
        pDTPickerLICENSEEXPIRYDATE->SetDay(v);
      }
    }
    s = GetDatabaseString(DRIVERS.licenseProvinceOfIssue, DRIVERS_PROVINCE_LENGTH);
    pEditLICENSEPROVINCEOFISSUE->SetWindowText(s);
    s = GetDatabaseString(DRIVERS.licenseType, DRIVERS_LICENSETYPE_LENGTH);
    pEditLICENSETYPE->SetWindowText(s);
    if(DRIVERS.physicalExpiryDate != 0)
    {
      GetYMD(DRIVERS.physicalExpiryDate, &year, &month, &day);
      if((year >= 1900 && year <= 2038) && (month >= 1 && month <= 12) && (day >= 1 && day <= 31))
      {
        v = year;
        pDTPickerPHYSICALEXPIRYDATE->SetYear(v);
        v = month;
        pDTPickerPHYSICALEXPIRYDATE->SetMonth(v);
        v = day;
        pDTPickerPHYSICALEXPIRYDATE->SetDay(v);
      }
    }
    fileName = szDatabaseRoot;
    fileName += "\\Common\\";
    s = GetDatabaseString(DRIVERS.badgeNumber, DRIVERS_BADGENUMBER_LENGTH);
    fileName += s;
    fileName += ".";
    s = GetDatabaseString(DRIVERS.bitmapFileExt, DRIVERS_BADGENUMBER_LENGTH);
    pEditBITMAPPATH->SetWindowText(s);
    if(strcmp(s, "") != 0)
    {
		  m_bmpCtrl.LoadFile(s);
    }
//
//  Last Name, First Name, Initials
//
    s = GetDatabaseString(DRIVERS.lastName, DRIVERS_LASTNAME_LENGTH);
    pEditLAST->SetWindowText(s);
    s = GetDatabaseString(DRIVERS.firstName, DRIVERS_FIRSTNAME_LENGTH);
    pEditFIRST->SetWindowText(s);
    s = GetDatabaseString(DRIVERS.initials, DRIVERS_INITIALS_LENGTH);
    pEditINITIALS->SetWindowText(s);
//
//  Badge number, crew number
//
    s = GetDatabaseString(DRIVERS.badgeNumber, DRIVERS_BADGENUMBER_LENGTH);
    pEditBADGE->SetWindowText(s);
    s.Format("%ld", DRIVERS.crewNumber);
    pEditCREW->SetWindowText(s);
//
//  Address, City, Province, Postal Code
//
    s = GetDatabaseString(DRIVERS.streetAddress, DRIVERS_STREETADDRESS_LENGTH);
    pEditADDRESS->SetWindowText(s);
    s = GetDatabaseString(DRIVERS.city, DRIVERS_CITY_LENGTH);
    pEditCITY->SetWindowText(s);
    s = GetDatabaseString(DRIVERS.province, DRIVERS_PROVINCE_LENGTH);
    pEditPROVINCE->SetWindowText(s);
    s = GetDatabaseString(DRIVERS.postalCode, DRIVERS_POSTALCODE_LENGTH);
    pEditPOSTALCODE->SetWindowText(s);
//
//  Telephone and cell area code and number and email
//
    if(DRIVERS.telephoneNumber != 0)
    {
      pMSMaskPHONE->SetText(PhoneString(DRIVERS.telephoneArea, DRIVERS.telephoneNumber));
    }
    if(DRIVERS.faxNumber != 0)
    {
      pMSMaskFAX->SetText(PhoneString(DRIVERS.faxArea, DRIVERS.faxNumber));
    }
//
//  Emergency phone and contact
//
    if(DRIVERS.emergencyNumber > 0 && DRIVERS.emergencyNumber <= 9999999 &&
          DRIVERS.emergencyArea > 0 && DRIVERS.emergencyArea <= 999)
    {
      pMSMaskEMERGENCY->SetText(PhoneString(DRIVERS.emergencyArea, DRIVERS.emergencyNumber));
    }
//
//  Email
//
    s = GetDatabaseString(DRIVERS.emailAddress, DRIVERS_EMAILADDRESS_LENGTH);
    pEditEMAIL->SetWindowText(s);
//
//  Hire date
//
    if(DRIVERS.hireDate == 0)
    {
      pButtonHIREDATEBOX->SetCheck(FALSE);
      pDTPickerHIREDATE->EnableWindow(FALSE);
    }
    else
    {
      GetYMD(DRIVERS.hireDate, &year, &month, &day);
      if((year >= 1900 && year <= 2038) && (month >= 1 && month <= 12) && (day >= 1 && day <= 31))
      {
        v = year;
        pDTPickerHIREDATE->SetYear(v);
        v = month;
        pDTPickerHIREDATE->SetMonth(v);
        v = day;
        pDTPickerHIREDATE->SetDay(v);
        pButtonHIREDATEBOX->SetCheck(TRUE);
      }
    }
//
//  Transit hire date
//
    if(DRIVERS.transitHireDate == 0)
    {
      pButtonTRANSITHIREDATEBOX->SetCheck(FALSE);
      pDTPickerTRANSITHIREDATE->EnableWindow(FALSE);
    }
    else
    {
      GetYMD(DRIVERS.transitHireDate, &year, &month, &day);
      if((year >= 1900 && year <= 2038) && (month >= 1 && month <= 12) && (day >= 1 && day <= 31))
      {
        v = year;
        pDTPickerTRANSITHIREDATE->SetYear(v);
        v = month;
        pDTPickerTRANSITHIREDATE->SetMonth(v);
        v = day;
        pDTPickerTRANSITHIREDATE->SetDay(v);
        pButtonTRANSITHIREDATEBOX->SetCheck(TRUE);
      }
    }
//
//  Full-time date
//
    if(DRIVERS.fullTimeDate == 0)
    {
      pButtonFULLTIMEDATEBOX->SetCheck(FALSE);
      pDTPickerFULLTIMEDATE->EnableWindow(FALSE);
    }
    else
    {
      GetYMD(DRIVERS.fullTimeDate, &year, &month, &day);
      if((year >= 1900 && year <= 2038) && (month >= 1 && month <= 12) && (day >= 1 && day <= 31))
      {
        v = year;
        pDTPickerFULLTIMEDATE->SetYear(v);
        v = month;
        pDTPickerFULLTIMEDATE->SetMonth(v);
        v = day;
        pDTPickerFULLTIMEDATE->SetDay(v);
        pButtonFULLTIMEDATEBOX->SetCheck(TRUE);
      }
    }
//
//  Promotion date
//
    if(DRIVERS.promotionDate == 0)
    {
      pButtonPROMOTIONDATEBOX->SetCheck(FALSE);
      pDTPickerPROMOTIONDATE->EnableWindow(FALSE);
    }
    else
    {
      GetYMD(DRIVERS.promotionDate, &year, &month, &day);
      if((year >= 1900 && year <= 2038) && (month >= 1 && month <= 12) && (day >= 1 && day <= 31))
      {
        v = year;
        pDTPickerPROMOTIONDATE->SetYear(v);
        v = month;
        pDTPickerPROMOTIONDATE->SetMonth(v);
        v = day;
        pDTPickerPROMOTIONDATE->SetDay(v);
        pButtonPROMOTIONDATEBOX->SetCheck(TRUE);
      }
    }
//
//  Seniority date
//
    if(DRIVERS.seniorityDate == 0)
    {
      pButtonSENDATEBOX->SetCheck(FALSE);
      pDTPickerSENDATE->EnableWindow(FALSE);
    }
    else
    {
      GetYMD(DRIVERS.seniorityDate, &year, &month, &day);
      if((year >= 1900 && year <= 2038) && (month >= 1 && month <= 12) && (day >= 1 && day <= 31))
      {
        v = year;
        pDTPickerSENDATE->SetYear(v);
        v = month;
        pDTPickerSENDATE->SetMonth(v);
        v = day;
        pDTPickerSENDATE->SetDay(v);
        pButtonSENDATEBOX->SetCheck(TRUE);
      }
    }
//
//  Termination date
//
    if(DRIVERS.terminationDate == 0)
    {
      pButtonTERMINATIONDATEBOX->SetCheck(FALSE);
      pDTPickerTERMINATIONDATE->EnableWindow(FALSE);
    }
    else
    {
      GetYMD(DRIVERS.terminationDate, &year, &month, &day);
      if((year >= 1900 && year <= 2038) && (month >= 1 && month <= 12) && (day >= 1 && day <= 31))
      {
        v = year;
        pDTPickerTERMINATIONDATE->SetYear(v);
        v = month;
        pDTPickerTERMINATIONDATE->SetMonth(v);
        v = day;
        pDTPickerTERMINATIONDATE->SetDay(v);
        pButtonTERMINATIONDATEBOX->SetCheck(TRUE);
      }
    }
//
//  Seniority sort
//
    s.Format("%d", DRIVERS.senioritySort);
    pEditSENSORT->SetWindowText(s);
//
//  Vacation, personal, sick, and float time
//
    long hoursInADay = 8;
    int  numDays;

    pEditVACATION->SetWindowText(chhmm(DRIVERS.vacationTime));
    numDays = DRIVERS.vacationTime / (hoursInADay * 3600);
    sprintf(tempString, "%d + %s", numDays, chhmm(DRIVERS.vacationTime - (numDays * (hoursInADay * 3600))));
    pStaticVACATIONDAYS->SetWindowText(tempString);

    pEditPENDINGVACATION->SetWindowText(chhmm(DRIVERS.pendingVacation));
    numDays = DRIVERS.pendingVacation / (hoursInADay * 3600);
    sprintf(tempString, "%d + %s", numDays, chhmm(DRIVERS.pendingVacation - (numDays * (hoursInADay * 3600))));
    pStaticPENDINGVACATIONDAYS->SetWindowText(tempString);

    pEditPERSONAL->SetWindowText(chhmm(DRIVERS.personalTime));
    numDays = DRIVERS.personalTime / (hoursInADay * 3600);
    sprintf(tempString, "%d + %s", numDays, chhmm(DRIVERS.personalTime - (numDays * (hoursInADay * 3600))));
    pStaticPERSONALDAYS->SetWindowText(tempString);

    pEditSICK->SetWindowText(chhmm(DRIVERS.sickTime));
    numDays = DRIVERS.sickTime / (hoursInADay * 3600);
    sprintf(tempString, "%d + %s", numDays, chhmm(DRIVERS.sickTime - (numDays * (hoursInADay * 3600))));
    pStaticSICKDAYS->SetWindowText(tempString);

    pEditFLOAT->SetWindowText(chhmm(DRIVERS.floatTime));
    numDays = DRIVERS.floatTime / (hoursInADay * 3600);
    sprintf(tempString, "%d + %s", numDays, chhmm(DRIVERS.floatTime - (numDays * (hoursInADay * 3600))));
    pStaticFLOATDAYS->SetWindowText(tempString);
//
//  SIN
//
    s = GetDatabaseString(DRIVERS.SIN, DRIVERS_SIN_LENGTH);
    pEditSIN->SetWindowText(s);
//
//  Date of Birth
//
    if(DRIVERS.dateOfBirth == 0) // Update on a record saved prior to addition of this field
    {
      day = time.GetDay();
      month = time.GetMonth();
      year = time.GetYear();
    }
    else
    {
      GetYMD(DRIVERS.dateOfBirth, &year, &month, &day);
    }
    if((year >= 1900 && year <= 2038) && (month >= 1 && month <= 12) && (day >= 1 && day <= 31))
    {
      v = year;
      pDTPickerDATEOFBIRTH->SetYear(v);
      v = month;
      pDTPickerDATEOFBIRTH->SetMonth(v);
      v = day;
      pDTPickerDATEOFBIRTH->SetDay(v);
    }
//
//  Race and Sex
//
    pComboBoxRACE->SetCurSel((int)DRIVERS.race);
    pComboBoxSEX->SetCurSel((int)DRIVERS.sex);
//
//  Supervisor
//
    pButtonSUPERVISOR->SetCheck((DRIVERS.flags & DRIVERS_FLAG_SUPERVISOR));
//
//  Saftey sensitive
//
    pButtonSAFETYSENSITIVE->SetCheck((DRIVERS.flags & DRIVERS_FLAG_SAFETYSENSITIVE));
//
//  EEOC
//
    pComboBoxEEOC->SetCurSel((int)DRIVERS.EEOC);
//
//  Labour Union
//
    pComboBoxUNION->SetCurSel((int)DRIVERS.labourUnion);
//
//  Section
//
    pComboBoxSECTION->SetCurSel((int)DRIVERS.section);
//
//  Position
//
    pComboBoxPOSITION->SetCurSel((int)DRIVERS.position);
  }
	
  m_bInit = FALSE;
  m_dlgPropSheet.SetActivePage(0);
  pEditLAST->SetFocus();

	return FALSE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CAddDriver::OnClose() 
{
  OnCancel();
}

void CAddDriver::OnCc() 
{
  pComboBoxCOMMENTCODE->SetCurSel(0);
}

void CAddDriver::OnNacc() 
{
  pComboBoxCOMMENTCODE->SetCurSel(-1);
}

void CAddDriver::OnSelendokCommentcode() 
{
  if(pButtonNACC->GetCheck())
  {
    pButtonNACC->SetCheck(FALSE);
    pButtonCC->SetCheck(TRUE);
  }
}

void CAddDriver::OnDt() 
{
  pComboBoxDRIVERTYPE->SetCurSel(0);
}

void CAddDriver::OnNadt() 
{
  pComboBoxDRIVERTYPE->SetCurSel(-1);
}

void CAddDriver::OnSelendokDrivertype() 
{
  if(pButtonNADT->GetCheck())
  {
    pButtonNADT->SetCheck(FALSE);
    pButtonDT->SetCheck(TRUE);
  }
}

void CAddDriver::OnDv() 
{
  pComboBoxDIVISION->SetCurSel(0);
}

void CAddDriver::OnNadv() 
{
  pComboBoxDIVISION->SetCurSel(-1);
}

void CAddDriver::OnSelendokDivision() 
{
  if(pButtonNADV->GetCheck())
  {
    pButtonNADV->SetCheck(FALSE);
    pButtonDV->SetCheck(TRUE);
  }
}

void CAddDriver::OnCancel() 
{
	CDialog::OnCancel();
}

void CAddDriver::OnHelp() 
{
  ::WinHelp(m_hWnd, szarHelpFile, HELP_CONTEXT, The_Drivers_Table);
}

void CAddDriver::OnOK() 
{
  COleVariant v;
  CString s;
  long day;
  long month;
  long year;
  int  rcode2;
  int  opCode;
  int  nI;
//
//  Add or update
//
  if(*m_pUpdateRecordID == NO_RECORD)
  {
    rcode2 = btrieve(B_GETLAST, TMS_DRIVERS, &DRIVERS, &DRIVERSKey0, 0);
    DRIVERS.recordID = AssignRecID(rcode2, DRIVERS.recordID);
    opCode = B_INSERT;
  }
  else
  {
    DRIVERSKey0.recordID = *m_pUpdateRecordID;
    btrieve(B_GETEQUAL, TMS_DRIVERS, &DRIVERS, &DRIVERSKey0, 0);
    opCode = B_UPDATE;
  }
//
//  Flags
//
  DRIVERS.flags = 0;
//
//  Comment code
//
  if(pButtonNACC->GetCheck())
  {
    DRIVERS.COMMENTSrecordID = NO_RECORD;
  }
  else
  {
    nI = (int)pComboBoxCOMMENTCODE->GetCurSel();
    DRIVERS.COMMENTSrecordID = (nI == CB_ERR ? NO_RECORD : pComboBoxCOMMENTCODE->GetItemData(nI));
  }
//
//  Division
//
  if(pButtonNADV->GetCheck())
  {
    DRIVERS.DIVISIONSrecordID = NO_RECORD;
  }
  else
  {
    nI = (int)pComboBoxDIVISION->GetCurSel();
    DRIVERS.DIVISIONSrecordID = (nI == CB_ERR ? NO_RECORD : pComboBoxDIVISION->GetItemData(nI));
  }
//
//  Driver type
//
  if(pButtonNADT->GetCheck())
  {
    DRIVERS.DRIVERTYPESrecordID = NO_RECORD;
  }
  else
  {
    nI = (int)pComboBoxDRIVERTYPE->GetCurSel();
    DRIVERS.DRIVERTYPESrecordID = (nI == CB_ERR ? NO_RECORD : pComboBoxDRIVERTYPE->GetItemData(nI));
  }
//
//  Last Name, First Name, Initials
//
  pEditLAST->GetWindowText(s);
  PutDatabaseString(DRIVERS.lastName, s, DRIVERS_LASTNAME_LENGTH);
  pEditFIRST->GetWindowText(s);
  PutDatabaseString(DRIVERS.firstName, s, DRIVERS_FIRSTNAME_LENGTH);
  pEditINITIALS->GetWindowText(s);
  PutDatabaseString(DRIVERS.initials, s, DRIVERS_INITIALS_LENGTH);
//
//  Badge number, crew number
//
  pEditBADGE->GetWindowText(s);
  PutDatabaseString(DRIVERS.badgeNumber, s, DRIVERS_BADGENUMBER_LENGTH);
  pEditCREW->GetWindowText(s);
  DRIVERS.crewNumber = atol(s);
//
//  Address, City, Province, Postal Code
//
  pEditADDRESS->GetWindowText(s);
  PutDatabaseString(DRIVERS.streetAddress, s, DRIVERS_STREETADDRESS_LENGTH);
  pEditCITY->GetWindowText(s);
  PutDatabaseString(DRIVERS.city, s, DRIVERS_CITY_LENGTH);
  pEditPROVINCE->GetWindowText(s);
  PutDatabaseString(DRIVERS.province, s, DRIVERS_PROVINCE_LENGTH);
  pEditPOSTALCODE->GetWindowText(s);
  PutDatabaseString(DRIVERS.postalCode, s, DRIVERS_POSTALCODE_LENGTH);
//
//  Telephone area code and number
//
//  This is a masked field: "(999) 999-9999"
//                           01234567890123
//
  s = pMSMaskPHONE->GetFormattedText();
  DRIVERS.telephoneArea = atol(s.Mid(1,3));
  DRIVERS.telephoneNumber = atol(s.Mid(6,3)) * 10000 + atol(s.Mid(10,4));
//
//  Cell area code and number
//
//  This is a masked field: "(999) 999-9999"
//                           01234567890123
//
  s = pMSMaskFAX->GetFormattedText();
  DRIVERS.faxArea = atol(s.Mid(1,3));
  DRIVERS.faxNumber = atol(s.Mid(6,3)) * 10000 + atol(s.Mid(10,4));
//
//  Emergency area code and number
//
//  This is a masked field: "(999) 999-9999"
//                           01234567890123
//
  s = pMSMaskFAX->GetFormattedText();
  DRIVERS.emergencyArea = atol(s.Mid(1,3));
  DRIVERS.emergencyNumber = atol(s.Mid(6,3)) * 10000 + atol(s.Mid(10,4));
//
//  Emergency contact
//
  pEditEMERGENCYCONTACT->GetWindowText(s);
  PutDatabaseString(DRIVERS.emergencyContact, s, DRIVERS_EMERGENCYCONTACT_LENGTH);
//
//  Email address
//
  pEditEMAIL->GetWindowText(s);
  PutDatabaseString(DRIVERS.emailAddress, s, DRIVERS_EMAILADDRESS_LENGTH);
//
//  Hire date
//
  if(pButtonHIREDATEBOX->GetCheck())
  {
    v = pDTPickerHIREDATE->GetYear();
    year = v.lVal;
    v = pDTPickerHIREDATE->GetMonth();
    month = v.lVal;
    v = pDTPickerHIREDATE->GetDay();
    day = v.lVal;
    DRIVERS.hireDate = year * 10000 + month * 100 + day;
  }
  else
  {
    DRIVERS.hireDate = 0;
  }
//
//  Transit hire date
//
  if(pButtonTRANSITHIREDATEBOX->GetCheck())
  {
    v = pDTPickerTRANSITHIREDATE->GetYear();
    year = v.lVal;
    v = pDTPickerTRANSITHIREDATE->GetMonth();
    month = v.lVal;
    v = pDTPickerTRANSITHIREDATE->GetDay();
    day = v.lVal;
    DRIVERS.transitHireDate = year * 10000 + month * 100 + day;
  }
  else
  {
    DRIVERS.transitHireDate = 0;
  }
//
//  Full-time date
//
  if(pButtonFULLTIMEDATEBOX->GetCheck())
  {
    v = pDTPickerFULLTIMEDATE->GetYear();
    year = v.lVal;
    v = pDTPickerFULLTIMEDATE->GetMonth();
    month = v.lVal;
    v = pDTPickerFULLTIMEDATE->GetDay();
    day = v.lVal;
    DRIVERS.fullTimeDate = year * 10000 + month * 100 + day;
  }
  else
  {
    DRIVERS.fullTimeDate = 0;
  }
//
//  Promotion date
//
  if(pButtonPROMOTIONDATEBOX->GetCheck())
  {
    v = pDTPickerPROMOTIONDATE->GetYear();
    year = v.lVal;
    v = pDTPickerPROMOTIONDATE->GetMonth();
    month = v.lVal;
    v = pDTPickerPROMOTIONDATE->GetDay();
    day = v.lVal;
    DRIVERS.promotionDate = year * 10000 + month * 100 + day;
  }
  else
  {
    DRIVERS.promotionDate = 0;
  }
//
//  Seniority date
//
  if(pButtonSENDATEBOX->GetCheck())
  {
    v = pDTPickerSENDATE->GetYear();
    year = v.lVal;
    v = pDTPickerSENDATE->GetMonth();
    month = v.lVal;
    v = pDTPickerSENDATE->GetDay();
    day = v.lVal;
    DRIVERS.seniorityDate = year * 10000 + month * 100 + day;
  }
  else
  {
    DRIVERS.seniorityDate = 0;
  }
//
//  Termination date
//
  if(pButtonTERMINATIONDATEBOX->GetCheck())
  {
    v = pDTPickerTERMINATIONDATE->GetYear();
    year = v.lVal;
    v = pDTPickerTERMINATIONDATE->GetMonth();
    month = v.lVal;
    v = pDTPickerTERMINATIONDATE->GetDay();
    day = v.lVal;
    DRIVERS.terminationDate = year * 10000 + month * 100 + day;
  }
  else
  {
    DRIVERS.terminationDate = 0;
  }
//
//  Seniority sort
//
  pEditSENSORT->GetWindowText(s);
  DRIVERS.senioritySort = atoi(s);
//
//  Vacation, personal, sick, and float days 
//
  pEditVACATION->GetWindowText(tempString, TEMPSTRING_LENGTH);
  DRIVERS.vacationTime = thhmm(tempString);

  pEditPENDINGVACATION->GetWindowText(tempString, TEMPSTRING_LENGTH);
  DRIVERS.pendingVacation = thhmm(tempString);

  pEditPERSONAL->GetWindowText(tempString, TEMPSTRING_LENGTH);
  DRIVERS.personalTime = thhmm(tempString);

  pEditSICK->GetWindowText(tempString, TEMPSTRING_LENGTH);
  DRIVERS.sickTime = thhmm(tempString);

  pEditFLOAT->GetWindowText(tempString, TEMPSTRING_LENGTH);
  DRIVERS.floatTime = thhmm(tempString);
//
//  SIN
//
  pEditSIN->GetWindowText(s);
  PutDatabaseString(DRIVERS.SIN, s, DRIVERS_SIN_LENGTH);
//
//  Date of birth
//
  v = pDTPickerDATEOFBIRTH->GetYear();
  year = v.lVal;
  v = pDTPickerDATEOFBIRTH->GetMonth();
  month = v.lVal;
  v = pDTPickerDATEOFBIRTH->GetDay();
  day = v.lVal;
  DRIVERS.dateOfBirth = year * 10000 + month * 100 + day;
//
//  Race and sex
//
  DRIVERS.race = pComboBoxRACE->GetCurSel();
  DRIVERS.sex = pComboBoxSEX->GetCurSel();
//
//  Safety Sensitive
//
  if(pButtonSAFETYSENSITIVE->GetCheck())
  {
    DRIVERS.flags |= DRIVERS_FLAG_SAFETYSENSITIVE;
  }
//
//  Supervisor
//
  if(pButtonSUPERVISOR->GetCheck())
  {
    DRIVERS.flags |= DRIVERS_FLAG_SUPERVISOR;
  }
//
//  EEOC
//
  DRIVERS.EEOC = pComboBoxEEOC->GetCurSel();
//
//  Labour Union
//
  DRIVERS.labourUnion = pComboBoxUNION->GetCurSel();
//
//  Section
//
  DRIVERS.section = pComboBoxSECTION->GetCurSel();
//
//  Position
//
  DRIVERS.position = pComboBoxPOSITION->GetCurSel();
//
//  License information
//
  pEditLICENSENUMBER->GetWindowText(s);
  PutDatabaseString(DRIVERS.licenseNumber, s, DRIVERS_LICENSENUMBER_LENGTH);
  v = pDTPickerLICENSEEXPIRYDATE->GetYear();
  year = v.lVal;
  v = pDTPickerLICENSEEXPIRYDATE->GetMonth();
  month = v.lVal;
  v = pDTPickerLICENSEEXPIRYDATE->GetDay();
  day = v.lVal;
  DRIVERS.licenseExpiryDate = year * 10000 + month * 100 + day;
  pEditLICENSEPROVINCEOFISSUE->GetWindowText(s);
  PutDatabaseString(DRIVERS.licenseProvinceOfIssue, s, DRIVERS_PROVINCE_LENGTH);
  pEditLICENSETYPE->GetWindowText(s);
  PutDatabaseString(DRIVERS.licenseType, s, DRIVERS_LICENSETYPE_LENGTH);
  v = pDTPickerPHYSICALEXPIRYDATE->GetYear();
  year = v.lVal;
  v = pDTPickerPHYSICALEXPIRYDATE->GetMonth();
  month = v.lVal;
  v = pDTPickerPHYSICALEXPIRYDATE->GetDay();
  day = v.lVal;
  DRIVERS.physicalExpiryDate = year * 10000 + month * 100 + day;
  pEditBITMAPPATH->GetWindowText(s);
  PutDatabaseString(DRIVERS.bitmapFileExt, s.Right(3), DRIVERS_BITMAPFILEEXT_LENGTH);
//
//  Update / Insert and leave
//
  memset(DRIVERS.reserved1, 0x00, DRIVERS_RESERVED1_LENGTH);
  memset(DRIVERS.reserved2, 0x00, DRIVERS_RESERVED2_LENGTH);
  btrieve(opCode, TMS_DRIVERS, &DRIVERS, &DRIVERSKey0, 0);
  *m_pUpdateRecordID = DRIVERS.recordID;
//
//  Send the data out in real time
//
  if(m_bUseStrategicMapping && opCode == B_INSERT)
  {
    m_AddDriver(BUSES.recordID);
  }
	
	CDialog::OnOK();
}

//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
// Customer.cpp : implementation file
//

#include "stdafx.h"
extern "C"
{
#include "TMSHeader.h"
}
#include "cc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CCustomer dialog

CCustomer::CCustomer(CWnd* pParent /*=NULL*/, CUSTOMERSDef* pCUSTOMERS)
	: CDialog(CCustomer::IDD, pParent)
{
  m_pCUSTOMERS = pCUSTOMERS;
	//{{AFX_DATA_INIT(CCustomer)
	//}}AFX_DATA_INIT
}


void CCustomer::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CCustomer)
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CCustomer, CDialog)
	//{{AFX_MSG_MAP(CCustomer)
	ON_BN_CLICKED(IDHELP, OnHelp)
	ON_WM_CLOSE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCustomer message handlers

BOOL CCustomer::OnInitDialog() 
{
	CDialog::OnInitDialog();

  pEditSALUTATION = (CEdit *)GetDlgItem(CUSTOMER_SALUTATION);
  pEditFIRSTNAME = (CEdit *)GetDlgItem(CUSTOMER_FIRSTNAME);
  pEditINITIALS = (CEdit *)GetDlgItem(CUSTOMER_INITIALS);
  pEditLASTNAME = (CEdit *)GetDlgItem(CUSTOMER_LASTNAME);
  pEditADDRESS = (CEdit *)GetDlgItem(CUSTOMER_ADDRESS);
  pEditCITY = (CEdit *)GetDlgItem(CUSTOMER_CITY);
  pEditSTATE = (CEdit *)GetDlgItem(CUSTOMER_STATE);
  pEditZIP = (CEdit *)GetDlgItem(CUSTOMER_ZIP);
  pEditEMAIL = (CEdit *)GetDlgItem(CUSTOMER_EMAIL);
  pMSMaskDAYPHONE = (CMSMask *)GetDlgItem(CUSTOMER_DAYPHONE);
  pMSMaskNIGHTPHONE = (CMSMask *)GetDlgItem(CUSTOMER_NIGHTPHONE);
  pMSMaskFAX = (CMSMask *)GetDlgItem(CUSTOMER_FAX);
//
//  STATE_TEXT display
//
  CStatic *pStaticSTATE_TEXT = (CStatic *)GetDlgItem(CUSTOMER_STATE_TEXT);
  GetPrivateProfileString((LPSTR)userName, (LPSTR)"StateDisplay", "State",
        tempString, TEMPSTRING_LENGTH, szDatabaseFileName);
  if(tempString[strlen(tempString) - 1] != ':')
    strcat(tempString, ":");
  pStaticSTATE_TEXT->SetWindowText(tempString);
//
//  ZIP_TEXT display
//
  CStatic *pStaticZIP_TEXT = (CStatic *)GetDlgItem(CUSTOMER_ZIP_TEXT);
  GetPrivateProfileString((LPSTR)userName, (LPSTR)"ZIPDisplay", "ZIP",
        tempString, TEMPSTRING_LENGTH, szDatabaseFileName);
  if(tempString[strlen(tempString) - 1] != ':')
    strcat(tempString, ":");
  pStaticZIP_TEXT->SetWindowText(tempString);
//
//  Incoming?
//
  if(m_pCUSTOMERS->recordID != NO_RECORD)
  {
    strncpy(tempString, m_pCUSTOMERS->salutation, CUSTOMERS_SALUTATION_LENGTH);
    trim(tempString, CUSTOMERS_SALUTATION_LENGTH);
    pEditSALUTATION->SetWindowText(tempString);

    strncpy(tempString, m_pCUSTOMERS->firstName, CUSTOMERS_FIRSTNAME_LENGTH); 
    trim(tempString, CUSTOMERS_FIRSTNAME_LENGTH);
    pEditFIRSTNAME->SetWindowText(tempString);

    strncpy(tempString, m_pCUSTOMERS->initials, CUSTOMERS_INITIALS_LENGTH);
    trim(tempString, CUSTOMERS_INITIALS_LENGTH);
    pEditINITIALS->SetWindowText(tempString);

    strncpy(tempString, m_pCUSTOMERS->lastName, CUSTOMERS_LASTNAME_LENGTH);
    trim(tempString, CUSTOMERS_LASTNAME_LENGTH);
    pEditLASTNAME->SetWindowText(tempString);

    strncpy(tempString, m_pCUSTOMERS->streetAddress, CUSTOMERS_STREETADDRESS_LENGTH);
    trim(tempString, CUSTOMERS_STREETADDRESS_LENGTH);
    pEditADDRESS->SetWindowText(tempString);

    strncpy(tempString, m_pCUSTOMERS->city, CUSTOMERS_CITY_LENGTH);
    trim(tempString, CUSTOMERS_CITY_LENGTH);
    pEditCITY->SetWindowText(tempString);

    strncpy(tempString, m_pCUSTOMERS->state, CUSTOMERS_STATE_LENGTH);
    trim(tempString, CUSTOMERS_STATE_LENGTH);
    pEditSTATE->SetWindowText(tempString);

    strncpy(tempString, m_pCUSTOMERS->ZIP, CUSTOMERS_ZIP_LENGTH);
    trim(tempString, CUSTOMERS_ZIP_LENGTH);
    pEditZIP->SetWindowText(tempString);

    strncpy(tempString, m_pCUSTOMERS->emailAddress, CUSTOMERS_EMAILADDRESS_LENGTH);
    trim(tempString, CUSTOMERS_EMAILADDRESS_LENGTH);
    pEditEMAIL->SetWindowText(tempString);

    pMSMaskDAYPHONE->SetText(
          PhoneString(m_pCUSTOMERS->dayTelephoneArea, m_pCUSTOMERS->dayTelephoneNumber));

    pMSMaskNIGHTPHONE->SetText(
          PhoneString(m_pCUSTOMERS->nightTelephoneArea, m_pCUSTOMERS->nightTelephoneNumber));

    pMSMaskFAX->SetText(
          PhoneString(m_pCUSTOMERS->faxTelephoneArea, m_pCUSTOMERS->faxTelephoneNumber));
  }

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CCustomer::OnCancel() 
{
  CString s;

  if(s.LoadString(ERROR_159))  // Are you sure?
  {
    if(MessageBox(s, TMS, MB_ICONQUESTION | MB_YESNO) == IDYES)
      CDialog::OnCancel();
  }
}

void CCustomer::OnOK() 
{
  BOOL bInsert = m_pCUSTOMERS->recordID == NO_RECORD;
  int  rcode2;
//
//  Validate certain fields (actually, ensure they're non-blank)
//
//  Salutation
//
  CString salutation;
  pEditSALUTATION->GetWindowText(salutation);
//  if(salutation == "")
//  {
//    TMSError(NULL, MB_ICONSTOP, ERROR_256, pEditSALUTATION->m_hWnd);
//    return;
//  }
//
//  Last name
//
  CString lastName;
  pEditLASTNAME->GetWindowText(lastName);
//  if(lastName == "")
//  {
//    TMSError(NULL, MB_ICONSTOP, ERROR_257, pEditLASTNAME->m_hWnd);
//    return;
//  }
//
//  Address
//
  CString address;
  pEditADDRESS->GetWindowText(address);
//  if(address == "")
//  {
//    TMSError(NULL, MB_ICONSTOP, ERROR_258, pEditADDRESS->m_hWnd);
//    return;
//  }
//
//  City
//
  CString city;
  pEditCITY->GetWindowText(city);
//  if(city == "")
//  {
//    TMSError(NULL, MB_ICONSTOP, ERROR_259, pEditCITY->m_hWnd);
//    return;
//  }
//
//  State
//
  CString state;
  pEditSTATE->GetWindowText(state);
//
//  Passed the tests
//
//  Build the record
//
//  recordID
//
  if(bInsert)
  {
    rcode2 = btrieve(B_GETLAST, TMS_CUSTOMERS, &CUSTOMERS, &CUSTOMERSKey0, 0);
    m_pCUSTOMERS->recordID = AssignRecID(rcode2, m_pCUSTOMERS->recordID);
  }
//
//  COMMENTSrecordID	
//
  if(bInsert)
    m_pCUSTOMERS->COMMENTSrecordID = NO_RECORD;
//
//  salutation
//
  strncpy(m_pCUSTOMERS->salutation, salutation, CUSTOMERS_SALUTATION_LENGTH);
  pad(m_pCUSTOMERS->salutation, CUSTOMERS_SALUTATION_LENGTH);
//
//  lastName
//
  strncpy(m_pCUSTOMERS->lastName, lastName, CUSTOMERS_LASTNAME_LENGTH);
  pad(m_pCUSTOMERS->lastName, CUSTOMERS_LASTNAME_LENGTH);
//
//  firstName
//
  pEditFIRSTNAME->GetWindowText(tempString, TEMPSTRING_LENGTH);
  strncpy(m_pCUSTOMERS->firstName, tempString, CUSTOMERS_FIRSTNAME_LENGTH);
  pad(m_pCUSTOMERS->firstName, CUSTOMERS_FIRSTNAME_LENGTH);
//
//  initials
//
  pEditINITIALS->GetWindowText(tempString, TEMPSTRING_LENGTH);
  strncpy(m_pCUSTOMERS->initials, tempString, CUSTOMERS_INITIALS_LENGTH);
  pad(m_pCUSTOMERS->initials, CUSTOMERS_INITIALS_LENGTH);
//
//  streetAddress
//
  strncpy(m_pCUSTOMERS->streetAddress, address, CUSTOMERS_STREETADDRESS_LENGTH);
  pad(m_pCUSTOMERS->streetAddress, CUSTOMERS_STREETADDRESS_LENGTH);
//
//  city
//
  strncpy(m_pCUSTOMERS->city, city, CUSTOMERS_CITY_LENGTH);
  pad(m_pCUSTOMERS->city, CUSTOMERS_CITY_LENGTH);
//
//  state
//
  strncpy(m_pCUSTOMERS->state, state, CUSTOMERS_STATE_LENGTH);
  pad(m_pCUSTOMERS->state, CUSTOMERS_STATE_LENGTH);
//
//  ZIP
//
  pEditZIP->GetWindowText(tempString, TEMPSTRING_LENGTH);
  strncpy(m_pCUSTOMERS->ZIP, tempString, CUSTOMERS_ZIP_LENGTH);
  pad(m_pCUSTOMERS->ZIP, CUSTOMERS_ZIP_LENGTH);
//
//  dayTelephoneArea and dayTelephoneNumber
//
//  This is a masked field: "(999) 999-9999"
//                           01234567890123
//
  CString dayPhone = pMSMaskDAYPHONE->GetFormattedText();
  m_pCUSTOMERS->dayTelephoneArea = atoi(dayPhone.Mid(1,3));
  m_pCUSTOMERS->dayTelephoneNumber = atoi(dayPhone.Mid(6,3)) * 10000 + atoi(dayPhone.Mid(10,4));
//
//  nightTelephoneArea and nightTelephoneNumber
//
  CString nightPhone = pMSMaskNIGHTPHONE->GetFormattedText();
  m_pCUSTOMERS->nightTelephoneArea = atoi(nightPhone.Mid(1,3));
  m_pCUSTOMERS->nightTelephoneNumber = atoi(nightPhone.Mid(6,3)) * 10000 + atoi(nightPhone.Mid(10,4));
//
//  faxTelephoneArea and faxTelephoneNumber
//
  CString faxPhone = pMSMaskFAX->GetFormattedText();
  m_pCUSTOMERS->faxTelephoneArea = atoi(faxPhone.Mid(1,3));
  m_pCUSTOMERS->faxTelephoneNumber = atoi(faxPhone.Mid(6,3)) * 10000 + atoi(faxPhone.Mid(10,4));
//
//  emailAddress
//
  pEditEMAIL->GetWindowText(tempString, TEMPSTRING_LENGTH);
  strncpy(m_pCUSTOMERS->emailAddress, tempString, CUSTOMERS_EMAILADDRESS_LENGTH);
  pad(m_pCUSTOMERS->emailAddress, CUSTOMERS_EMAILADDRESS_LENGTH);
//
//  reserved
//
  memset(&m_pCUSTOMERS->reserved, 0x00, CUSTOMERS_RESERVED_LENGTH);
//
//  flags
//
  m_pCUSTOMERS->flags = 0;
//
//  Write the record
//
  rcode2 = btrieve((bInsert ? B_INSERT : B_UPDATE), TMS_CUSTOMERS, &CUSTOMERS, &CUSTOMERSKey0, 0);

	CDialog::OnOK();
}

void CCustomer::OnHelp() 
{
  ::WinHelp(m_hWnd, szarHelpFile, HELP_CONTEXT, The_Customer_Information_Dialog);
}

void CCustomer::OnClose() 
{
  OnCancel();
}

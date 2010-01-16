//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
// CCSupervisor.cpp : implementation file
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
// CCCSupervisor dialog


CCCSupervisor::CCCSupervisor(CWnd* pParent, COMPLAINTSDef* pCOMPLAINTS, CCFEDef* pCCFE)
	: CDialog(CCCSupervisor::IDD, pParent)
{
  m_pCOMPLAINTS = pCOMPLAINTS;
  strcpy(m_szReceivedBy, pCCFE->szReceivedBy);
  strcpy(m_szDateTime, pCCFE->szDateTime);

	//{{AFX_DATA_INIT(CCCSupervisor)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CCCSupervisor::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CCCSupervisor)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CCCSupervisor, CDialog)
	//{{AFX_MSG_MAP(CCCSupervisor)
	ON_BN_CLICKED(IDHELP, OnHelp)
	ON_BN_CLICKED(IDFINALIZE, OnFinalize)
	ON_BN_CLICKED(CCSUPERVISOR_OTHER, OnOther)
	ON_BN_CLICKED(CCSUPERVISOR_NOVIOLATION, OnNoviolation)
	ON_BN_CLICKED(CCSUPERVISOR_INFORMATION, OnInformation)
	ON_BN_CLICKED(CCSUPERVISOR_INSUFFICIENT, OnInsufficient)
	ON_BN_CLICKED(CCSUPERVISOR_COMMENDATION, OnCommendation)
	ON_BN_CLICKED(IDPRINT, OnPrint)
	ON_BN_CLICKED(IDSCAN, OnScan)
	ON_BN_CLICKED(IDVIEW, OnView)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCCSupervisor message handlers

BOOL CCCSupervisor::OnInitDialog() 
{
	CDialog::OnInitDialog();

  COleVariant v;
  CString s;
  CTime time = CTime::GetCurrentTime();
  long day = time.GetDay();
  long month = time.GetMonth();
  long year = time.GetYear();
  long hours = 0;
  long minutes = 0;
  long seconds = 0;
  long dd, mm, yyyy;

	
  pComboBoxDEPARTMENT = (CComboBox *)GetDlgItem(CCSUPERVISOR_DEPARTMENT);
  pComboBoxASSIGNEDTO = (CComboBox *)GetDlgItem(CCSUPERVISOR_ASSIGNEDTO);
  pComboBoxCLASSIFICATIONCODE = (CComboBox *)GetDlgItem(CCSUPERVISOR_CLASSIFICATIONCODE);
  pComboBoxCATEGORY = (CComboBox *)GetDlgItem(CCSUPERVISOR_CATEGORY);
  pButtonDAMAGE = (CButton *)GetDlgItem(CCSUPERVISOR_DAMAGE);
  pButtonVIOLATION = (CButton *)GetDlgItem(CCSUPERVISOR_VIOLATION);
  pComboBoxEMPLOYEENAME = (CComboBox *)GetDlgItem(CCSUPERVISOR_EMPLOYEENAME);
  pDTPickerDATEPRESENTED = (CDTPicker *)GetDlgItem(CCSUPERVISOR_DATEPRESENTED);
  pButtonSIMILARCOMMENTS = (CButton *)GetDlgItem(CCSUPERVISOR_SIMILARCOMMENTS);
  pButtonPHONE = (CButton *)GetDlgItem(CCSUPERVISOR_PHONE);
  pButtonLETTER = (CButton *)GetDlgItem(CCSUPERVISOR_LETTER);
  pButtonEMAIL = (CButton *)GetDlgItem(CCSUPERVISOR_EMAIL);
  pEditCONVERSATION = (CEdit *)GetDlgItem(CCSUPERVISOR_CONVERSATION);
  pComboBoxACTIONCODE = (CComboBox *)GetDlgItem(CCSUPERVISOR_ACTIONCODE);
  pButtonNOVIOLATION = (CButton *)GetDlgItem(CCSUPERVISOR_NOVIOLATION);
  pButtonINFORMATION = (CButton *)GetDlgItem(CCSUPERVISOR_INFORMATION);
  pButtonINSUFFICIENT = (CButton *)GetDlgItem(CCSUPERVISOR_INSUFFICIENT);
  pButtonCOMMENDATION = (CButton *)GetDlgItem(CCSUPERVISOR_COMMENDATION);
  pButtonOTHER = (CButton *)GetDlgItem(CCSUPERVISOR_OTHER);
  pEditOTHERTEXT = (CEdit *)GetDlgItem(CCSUPERVISOR_OTHERTEXT);
  pButtonOPERATIONS = (CButton *)GetDlgItem(CCSUPERVISOR_OPERATIONS);
  pButtonMAINTENANCE = (CButton *)GetDlgItem(CCSUPERVISOR_MAINTENANCE);
  pButtonFINANCE = (CButton *)GetDlgItem(CCSUPERVISOR_FINANCE);
  pButtonDEVELOPMENT = (CButton *)GetDlgItem(CCSUPERVISOR_DEVELOPMENT);
  pButtonSPECIALSERVICES = (CButton *)GetDlgItem(CCSUPERVISOR_SPECIALSERVICES);
  pButtonCOMMUNITYRELATIONS = (CButton *)GetDlgItem(CCSUPERVISOR_COMMUNITYRELATIONS);
  pDTPickerRETAINUNTIL = (CDTPicker *)GetDlgItem(CCSUPERVISOR_RETAINUNTIL);
  pStaticDATECLOSED = (CStatic *)GetDlgItem(CCSUPERVISOR_DATECLOSED);
//
//  Set the title
//
  GetWindowText(szFormatString, sizeof(szFormatString));
  strncpy(szarString, m_pCOMPLAINTS->referenceNumber, COMPLAINTS_REFERENCENUMBER_LENGTH);
  trim(szarString, COMPLAINTS_REFERENCENUMBER_LENGTH);
  sprintf(tempString, szFormatString, szarString);
  SetWindowText(tempString);
//
//  Set up the drop downs
//
  PopulateComboBox(m_ActionCodes, m_numActionCodes, pComboBoxACTIONCODE);
  PopulateComboBox(m_Categories, m_numCategories, pComboBoxCATEGORY);
  PopulateComboBox(m_ClassificationCodes, m_numClassificationCodes, pComboBoxCLASSIFICATIONCODE);
  PopulateComboBox(m_Departments, m_numDepartments, pComboBoxDEPARTMENT);
//
//  Set up the referrals
//
  int nI;

  s.LoadString(TEXT_246);
  nI = pComboBoxASSIGNEDTO->AddString(s);
  pComboBoxASSIGNEDTO->SetCurSel(nI);

  if(m_numReferrals == 0)
  {
    pComboBoxASSIGNEDTO->EnableWindow(FALSE);
  }
  else
  {
    for(nI = 0; nI < m_numReferrals; nI++)
    {
      pComboBoxASSIGNEDTO->AddString(m_Referrals[nI].szName);
      if(strcmp(m_Referrals[nI].szName, m_pCOMPLAINTS->assignedTo) == 0)
      {
        pComboBoxASSIGNEDTO->SetCurSel(nI);
      }
    }
  }
//
//  Load the drivers
//
//  Set the first driver to "Not known or N/A"
//
  int numDrivers;
  int rcode2;
  BOOL bGotDriver = FALSE;

  s.LoadString(TEXT_238);
  numDrivers = pComboBoxEMPLOYEENAME->AddString(s);
  pComboBoxEMPLOYEENAME->SetItemData(numDrivers, NO_RECORD);
  pComboBoxEMPLOYEENAME->SetCurSel(0);
  rcode2 = btrieve(B_GETFIRST, TMS_DRIVERS, &DRIVERS, &DRIVERSKey1, 1);
  while(rcode2 == 0)
  {
    strncpy(tempString, DRIVERS.lastName, DRIVERS_LASTNAME_LENGTH);
    trim(tempString, DRIVERS_LASTNAME_LENGTH);
    strcpy(szarString, tempString);
    strcat(szarString, ", ");
    strncpy(tempString, DRIVERS.firstName, DRIVERS_FIRSTNAME_LENGTH);
    trim(tempString, DRIVERS_FIRSTNAME_LENGTH);
    strcat(szarString, tempString);
    numDrivers = pComboBoxEMPLOYEENAME->AddString(szarString);
    if(numDrivers != CB_ERR)
    {
      pComboBoxEMPLOYEENAME->SetItemData(numDrivers, DRIVERS.recordID);
      if(!bGotDriver)
      {
        if(m_pCOMPLAINTS->supDRIVERSrecordID == DRIVERS.recordID ||
              m_pCOMPLAINTS->DRIVERSrecordID == DRIVERS.recordID)
        {
          pComboBoxEMPLOYEENAME->SetCurSel(numDrivers);
          bGotDriver = TRUE;
        }
      }
    }
    rcode2 = btrieve(B_GETNEXT, TMS_DRIVERS, &DRIVERS, &DRIVERSKey1, 1);
  }
//
//  Default radio buttons
//
  if(!(m_pCOMPLAINTS->supFlags & COMPLAINTS_SUPFLAG_NOVIOLATION) &&
        !(m_pCOMPLAINTS->supFlags & COMPLAINTS_SUPFLAG_INFORMATIONONLY) &&
        !(m_pCOMPLAINTS->supFlags & COMPLAINTS_SUPFLAG_INSUFFICIENT) &&
        !(m_pCOMPLAINTS->supFlags & COMPLAINTS_SUPFLAG_COMMENDATION) &&
        !(m_pCOMPLAINTS->supFlags & COMPLAINTS_SUPFLAG_OTHER))
  {
    if(m_pCOMPLAINTS->referenceNumber[0] == 'S')
      pButtonINFORMATION->SetCheck(TRUE);
    else if(m_pCOMPLAINTS->referenceNumber[0] == 'M')
      pButtonCOMMENDATION->SetCheck(TRUE);
    else
      pButtonINSUFFICIENT->SetCheck(TRUE);
  }
//
//  Was a letter sent?
//
  if(m_pCOMPLAINTS->flags & COMPLAINTS_FLAG_LETTERSENT)
    pButtonLETTER->SetCheck(TRUE);
//
//  Do all the flags
//
  if(m_pCOMPLAINTS->supFlags & COMPLAINTS_SUPFLAG_DAMAGE)
    pButtonDAMAGE->SetCheck(TRUE);
  if(m_pCOMPLAINTS->supFlags & COMPLAINTS_SUPFLAG_VIOLATION)
    pButtonVIOLATION->SetCheck(TRUE);
  if(m_pCOMPLAINTS->supFlags & COMPLAINTS_SUPFLAG_RECEIVEDSIMILAR)
    pButtonSIMILARCOMMENTS->SetCheck(TRUE);
  if(m_pCOMPLAINTS->supFlags & COMPLAINTS_SUPFLAG_TELEPHONE)
    pButtonPHONE->SetCheck(TRUE);
  if(m_pCOMPLAINTS->supFlags & COMPLAINTS_SUPFLAG_LETTER)
    pButtonLETTER->SetCheck(TRUE);
  if(m_pCOMPLAINTS->supFlags & COMPLAINTS_SUPFLAG_EMAIL)
    pButtonEMAIL->SetCheck(TRUE);
  if(m_pCOMPLAINTS->supFlags & COMPLAINTS_SUPFLAG_NOVIOLATION)
    pButtonNOVIOLATION->SetCheck(TRUE);
  if(m_pCOMPLAINTS->supFlags & COMPLAINTS_SUPFLAG_INFORMATIONONLY)
    pButtonINFORMATION->SetCheck(TRUE);
  if(m_pCOMPLAINTS->supFlags & COMPLAINTS_SUPFLAG_INSUFFICIENT)
    pButtonINSUFFICIENT->SetCheck(TRUE);
  if(m_pCOMPLAINTS->supFlags & COMPLAINTS_SUPFLAG_COMMENDATION)
    pButtonCOMMENDATION->SetCheck(TRUE);
  if(m_pCOMPLAINTS->supFlags & COMPLAINTS_SUPFLAG_OTHER)
    pButtonOTHER->SetCheck(TRUE);
  if(m_pCOMPLAINTS->supFlags & COMPLAINTS_SUPFLAG_OPERATIONS)
    pButtonOPERATIONS->SetCheck(TRUE);
  if(m_pCOMPLAINTS->supFlags & COMPLAINTS_SUPFLAG_DEVELOPMENT)
    pButtonDEVELOPMENT->SetCheck(TRUE);
  if(m_pCOMPLAINTS->supFlags & COMPLAINTS_SUPFLAG_MAINTENANCE)
    pButtonMAINTENANCE->SetCheck(TRUE);
  if(m_pCOMPLAINTS->supFlags & COMPLAINTS_SUPFLAG_FINANCE)
    pButtonFINANCE->SetCheck(TRUE);
  if(m_pCOMPLAINTS->supFlags & COMPLAINTS_SUPFLAG_SPECIALSERVICES)
    pButtonSPECIALSERVICES->SetCheck(TRUE);
  if(m_pCOMPLAINTS->supFlags & COMPLAINTS_SUPFLAG_COMMRELATIONS)
    pButtonCOMMUNITYRELATIONS->SetCheck(TRUE);
//
//  The "Other" text
//
  pEditOTHERTEXT->EnableWindow(pButtonOTHER->GetCheck());
//
//  Dropdowns
//
  int numEntries;
//
//  Department
//
  if(m_pCOMPLAINTS->departmentIndex != NO_RECORD)
  {
    numEntries = pComboBoxDEPARTMENT->GetCount();
    for(nI = 0; nI < numEntries; nI++)
    {
      if(m_pCOMPLAINTS->departmentIndex == (int)pComboBoxDEPARTMENT->GetItemData(nI))
      {
        pComboBoxDEPARTMENT->SetCurSel(nI);
        break;
      }
    }
  }
//
//  Classification code
//
  if(m_pCOMPLAINTS->classificationIndex != NO_RECORD)
  {
    numEntries = pComboBoxCLASSIFICATIONCODE->GetCount();
    for(nI = 0; nI < numEntries; nI++)
    {
      if(m_pCOMPLAINTS->classificationIndex == (int)pComboBoxCLASSIFICATIONCODE->GetItemData(nI))
      {
        pComboBoxCLASSIFICATIONCODE->SetCurSel(nI);
        break;
      }
    }
  }
//
//  Category
//
  if(m_pCOMPLAINTS->categoryIndex != NO_RECORD)
  {
    numEntries = pComboBoxCATEGORY->GetCount();
    for(nI = 0; nI < numEntries; nI++)
    {
      if(m_pCOMPLAINTS->categoryIndex == (int)pComboBoxCATEGORY->GetItemData(nI))
      {
        pComboBoxCATEGORY->SetCurSel(nI);
        break;
      }
    }
  }
//
//  Action taken
//
  if(m_pCOMPLAINTS->actionIndex != NO_RECORD)
  {
    numEntries = pComboBoxACTIONCODE->GetCount();
    for(nI = 0; nI < numEntries; nI++)
    {
      if(m_pCOMPLAINTS->actionIndex == (int)pComboBoxACTIONCODE->GetItemData(nI))
      {
        pComboBoxACTIONCODE->SetCurSel(nI);
        break;
      }
    }
  }
//
//  Date presented to employee
//
  if(m_pCOMPLAINTS->datePresented == 0)
  {
    yyyy = year;
    mm = month;
    dd = day;
  }
  else
  {
    GetYMD(m_pCOMPLAINTS->datePresented, &yyyy, &mm, &dd);
  }
  v = yyyy;
  pDTPickerDATEPRESENTED->SetYear(v);
  v = mm;
  pDTPickerDATEPRESENTED->SetMonth(v);
  v = dd;
  pDTPickerDATEPRESENTED->SetDay(v);
//
//  Retain until
//
  if(m_pCOMPLAINTS->retainUntil == 0)
  {
    yyyy = year + 1;
    mm = month;
    dd = day;
  }
  else
  {
    GetYMD(m_pCOMPLAINTS->retainUntil, &yyyy, &mm, &dd);
  }
  v = yyyy;
  pDTPickerRETAINUNTIL->SetYear(v);
  v = mm;
  pDTPickerRETAINUNTIL->SetMonth(v);
  v = dd;
  pDTPickerRETAINUNTIL->SetDay(v);
//
//  Date closed
//
  if(m_pCOMPLAINTS->dateClosed != 0)
  {
    GetYMD(m_pCOMPLAINTS->dateClosed, &yyyy, &mm, &dd);
    
    CTime dateClosed(yyyy, mm, dd, hours, minutes, seconds);
    strcpy(tempString, dateClosed.Format(_T("%A, %B %d, %Y")));
    pStaticDATECLOSED->SetWindowText(tempString);
  }
//
//  Text fields
//
//  Memo of conversation
//
  char szMemo[COMPLAINTS_CONVERSATION_LENGTH + 1];

  strncpy(szMemo, m_pCOMPLAINTS->conversation, COMPLAINTS_CONVERSATION_LENGTH);
  trim(szMemo, COMPLAINTS_CONVERSATION_LENGTH);
  pEditCONVERSATION->SetWindowText(szMemo);

//
//  "Other" text
//
  if(pButtonOTHER->GetCheck())
  {
    char szOther[COMPLAINTS_OTHER_LENGTH + 1];

    strncpy(szOther, m_pCOMPLAINTS->other, COMPLAINTS_OTHER_LENGTH);
    trim(szOther, COMPLAINTS_OTHER_LENGTH);
    pEditOTHERTEXT->SetWindowText(szOther);
  }
//
//  All done
//
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CCCSupervisor::OnOK() 
{
  SaveRecord();
	
	CDialog::OnOK();
}

void CCCSupervisor::OnCancel() 
{
	CDialog::OnCancel();
}

void CCCSupervisor::OnHelp() 
{
}

void CCCSupervisor::PopulateComboBox(GenericTextDef *pData, int numEntries, CComboBox *pComboBox)
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

void CCCSupervisor::OnFinalize() 
{
  CString s;
  long day;
  long month;
  long year;

  if(s.LoadString(TEXT_211))
  {
    MessageBeep(MB_ICONQUESTION);
    if(MessageBox(s, TMS, MB_ICONQUESTION | MB_YESNO) != IDYES)
      return;
    SaveRecord();
    COMPLAINTSKey0.recordID = m_pCOMPLAINTS->recordID;
    recordLength[TMS_COMPLAINTS] = COMPLAINTS_TOTAL_LENGTH;
    btrieve(B_GETEQUAL, TMS_COMPLAINTS, pComplaintText, &COMPLAINTSKey0, 0);
    memcpy(&COMPLAINTS, pComplaintText, COMPLAINTS_FIXED_LENGTH);
    COMPLAINTS.flags |= COMPLAINTS_FLAG_CLOSED;
    CTime time = CTime::GetCurrentTime();
    year = time.GetYear();
    month = time.GetMonth();
    day = time.GetDay();
    COMPLAINTS.dateClosed = year * 10000 + month * 100 + day;
    memcpy(pComplaintText, &COMPLAINTS, COMPLAINTS_FIXED_LENGTH);
    s.LoadString(TEXT_160);
    sprintf(tempString, s, m_szReceivedBy, m_szDateTime);
    strcat(&pComplaintText[COMPLAINTS_FIXED_LENGTH], tempString);
    btrieve(B_UPDATE, TMS_COMPLAINTS, pComplaintText, &COMPLAINTSKey0, 0);
    recordLength[TMS_COMPLAINTS] = COMPLAINTS_FIXED_LENGTH;
    CDialog::OnOK();
  }
}

void CCCSupervisor::SaveRecord()
{
  COleVariant v;
  CString s;
  long day;
  long month;
  long year;
  int  nI;
//
// Refresh record and position
//
  COMPLAINTSKey0.recordID = m_pCOMPLAINTS->recordID;
  recordLength[TMS_COMPLAINTS] = COMPLAINTS_TOTAL_LENGTH;
  btrieve(B_GETEQUAL, TMS_COMPLAINTS, pComplaintText, &COMPLAINTSKey0, 0);
  memcpy(&COMPLAINTS, pComplaintText, COMPLAINTS_FIXED_LENGTH);
//
//  Set up the supervisor flags
//
  COMPLAINTS.supFlags = 0;
  if(pButtonDAMAGE->GetCheck())
    COMPLAINTS.supFlags |= COMPLAINTS_SUPFLAG_DAMAGE;
  if(pButtonVIOLATION->GetCheck())
    COMPLAINTS.supFlags |= COMPLAINTS_SUPFLAG_VIOLATION;
  if(pButtonSIMILARCOMMENTS->GetCheck())
    COMPLAINTS.supFlags |= COMPLAINTS_SUPFLAG_RECEIVEDSIMILAR;
  if(pButtonPHONE->GetCheck())
    COMPLAINTS.supFlags |= COMPLAINTS_SUPFLAG_TELEPHONE;
  if(pButtonLETTER->GetCheck())
    COMPLAINTS.supFlags |= COMPLAINTS_SUPFLAG_LETTER;
  if(pButtonEMAIL->GetCheck())
    COMPLAINTS.supFlags |= COMPLAINTS_SUPFLAG_EMAIL;
  if(pButtonNOVIOLATION->GetCheck())
    COMPLAINTS.supFlags |= COMPLAINTS_SUPFLAG_NOVIOLATION;
  if(pButtonINFORMATION->GetCheck())
    COMPLAINTS.supFlags |= COMPLAINTS_SUPFLAG_INFORMATIONONLY;
  if(pButtonINSUFFICIENT->GetCheck())
    COMPLAINTS.supFlags |= COMPLAINTS_SUPFLAG_INSUFFICIENT;
  if(pButtonCOMMENDATION->GetCheck())
    COMPLAINTS.supFlags |= COMPLAINTS_SUPFLAG_COMMENDATION;
  if(pButtonOTHER->GetCheck())
    COMPLAINTS.supFlags |= COMPLAINTS_SUPFLAG_OTHER;
  if(pButtonOPERATIONS->GetCheck())
    COMPLAINTS.supFlags |= COMPLAINTS_SUPFLAG_OPERATIONS;
  if(pButtonDEVELOPMENT->GetCheck())
    COMPLAINTS.supFlags |= COMPLAINTS_SUPFLAG_DEVELOPMENT;
  if(pButtonMAINTENANCE->GetCheck())
    COMPLAINTS.supFlags |= COMPLAINTS_SUPFLAG_MAINTENANCE;
  if(pButtonFINANCE->GetCheck())
    COMPLAINTS.supFlags |= COMPLAINTS_SUPFLAG_FINANCE;
  if(pButtonSPECIALSERVICES->GetCheck())
    COMPLAINTS.supFlags |= COMPLAINTS_SUPFLAG_SPECIALSERVICES;
  if(pButtonCOMMUNITYRELATIONS->GetCheck())
    COMPLAINTS.supFlags |= COMPLAINTS_SUPFLAG_COMMRELATIONS;
//
//  Department
//
  nI = pComboBoxDEPARTMENT->GetCurSel();
  if(nI == CB_ERR)
    COMPLAINTS.departmentIndex = NO_RECORD;
  else
    COMPLAINTS.departmentIndex = pComboBoxDEPARTMENT->GetItemData(nI);
//
//  Assigned to
//
  nI = pComboBoxASSIGNEDTO->GetCurSel();
  if(nI == CB_ERR)
    memset(COMPLAINTS.assignedTo, ' ', COMPLAINTS_ASSIGNEDTO_LENGTH);
  else
  {
    pComboBoxASSIGNEDTO->GetLBText(nI, s);
    strcpy(tempString, s);
    pad(tempString, COMPLAINTS_ASSIGNEDTO_LENGTH);
    strncpy(COMPLAINTS.assignedTo, tempString, COMPLAINTS_ASSIGNEDTO_LENGTH);
  }
//
//  Classification code
//
  nI = pComboBoxCLASSIFICATIONCODE->GetCurSel();
  if(nI == CB_ERR)
    COMPLAINTS.classificationIndex = NO_RECORD;
  else
    COMPLAINTS.classificationIndex = pComboBoxCLASSIFICATIONCODE->GetItemData(nI);
//
//  Category
//
  nI = pComboBoxCATEGORY->GetCurSel();
  if(nI == CB_ERR)
    COMPLAINTS.categoryIndex = NO_RECORD;
  else
    COMPLAINTS.categoryIndex = pComboBoxCATEGORY->GetItemData(nI);
//
//  Employee name
//
  nI = pComboBoxEMPLOYEENAME->GetCurSel();
  if(nI == CB_ERR)
    COMPLAINTS.supDRIVERSrecordID = NO_RECORD;
  else
    COMPLAINTS.supDRIVERSrecordID = pComboBoxEMPLOYEENAME->GetItemData(nI);
//
//  Date presented
//
  v = pDTPickerDATEPRESENTED->GetYear();
  year = v.lVal;
  v = pDTPickerDATEPRESENTED->GetMonth();
  month = v.lVal;
  v = pDTPickerDATEPRESENTED->GetDay();
  day = v.lVal;
  COMPLAINTS.datePresented = year * 10000 + month * 100 + day;
//
//  Conversation
//
  pEditCONVERSATION->GetWindowText(s);
  nI = min(strlen(s), COMPLAINTS_CONVERSATION_LENGTH);
  strncpy(COMPLAINTS.conversation, s, nI);
  pad(COMPLAINTS.conversation, COMPLAINTS_CONVERSATION_LENGTH);
//
//  Action taken
//
  nI = pComboBoxACTIONCODE->GetCurSel();
  if(nI == CB_ERR)
    COMPLAINTS.actionIndex = NO_RECORD;
  else
    COMPLAINTS.actionIndex = pComboBoxACTIONCODE->GetItemData(nI);
//
//  "Other" text
//
  pEditOTHERTEXT->GetWindowText(s);
  nI = min(strlen(s), COMPLAINTS_OTHER_LENGTH);
  strncpy(COMPLAINTS.other, s, nI);
  pad(COMPLAINTS.other, COMPLAINTS_OTHER_LENGTH);
//
//  Retain until
//
  v = pDTPickerRETAINUNTIL->GetYear();
  year = v.lVal;
  v = pDTPickerRETAINUNTIL->GetMonth();
  month = v.lVal;
  v = pDTPickerRETAINUNTIL->GetDay();
  day = v.lVal;
  COMPLAINTS.retainUntil = year * 10000 + month * 100 + day;
//
//  Save it
//
  memcpy(pComplaintText, &COMPLAINTS, COMPLAINTS_FIXED_LENGTH);
  btrieve(B_UPDATE, TMS_COMPLAINTS, pComplaintText, &COMPLAINTSKey0, 0);
  recordLength[TMS_COMPLAINTS] = COMPLAINTS_FIXED_LENGTH;
}

void CCCSupervisor::OnOther() 
{
  pEditOTHERTEXT->EnableWindow(TRUE);
}

void CCCSupervisor::OnNoviolation() 
{
  ClearOtherText();
}

void CCCSupervisor::OnInformation() 
{
  ClearOtherText();
}

void CCCSupervisor::OnInsufficient() 
{
  ClearOtherText();
}

void CCCSupervisor::OnCommendation() 
{
  ClearOtherText();
}

void CCCSupervisor::ClearOtherText()
{
  pEditOTHERTEXT->EnableWindow(FALSE);
  pEditOTHERTEXT->SetWindowText("");
}

void CCCSupervisor::OnPrint() 
{
  PrintWindowToDC(this->m_hWnd, "Customer Comments Supervisor Print Screen");
}

void CCCSupervisor::OnScan() 
{
  strcpy(tempString, szInstallDirectory);
  strcat(tempString, "\\TWACK_32.exe ");
  strncpy(szarString, m_pCOMPLAINTS->referenceNumber, COMPLAINTS_REFERENCENUMBER_LENGTH);
  trim(szarString, COMPLAINTS_REFERENCENUMBER_LENGTH);
  strcat(tempString, szarString);
  WinExec(tempString, SW_NORMAL);
}

void CCCSupervisor::OnView() 
{
  GetPrivateProfileString("TMS", "CCMScanDirectory", "C:\\TMS\\Scanned Documents",
        tempString, TEMPSTRING_LENGTH, TMSINIFile);
  strcat(tempString, "\\");
  strncpy(szarString, m_pCOMPLAINTS->referenceNumber, COMPLAINTS_REFERENCENUMBER_LENGTH);
  trim(szarString, COMPLAINTS_REFERENCENUMBER_LENGTH);
  strcat(tempString, szarString);
  strcat(tempString, ".bmp");
  strcpy(szarString, "mspaint \"");
  strcat(szarString, tempString);
  strcat(szarString, "\"");
  WinExec(szarString, SW_NORMAL);
}

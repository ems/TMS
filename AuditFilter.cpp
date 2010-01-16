// AuditFilter.cpp : implementation file
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

BOOL AuditFilterCpp(AUDITFILTERSELECTIONSDef *pSelections)
{
  CAuditFilter dlg(NULL, pSelections);

  return(dlg.DoModal());
}

extern "C"{

BOOL AuditFilter(AUDITFILTERSELECTIONSDef *pSelections)
{
  return(AuditFilterCpp(pSelections));
}

}

/////////////////////////////////////////////////////////////////////////////
// CAuditFilter dialog


CAuditFilter::CAuditFilter(CWnd* pParent, AUDITFILTERSELECTIONSDef *pSelections)
	: CDialog(CAuditFilter::IDD, pParent)
{
  m_pSelections = pSelections;

	//{{AFX_DATA_INIT(CAuditFilter)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CAuditFilter::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAuditFilter)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CAuditFilter, CDialog)
	//{{AFX_MSG_MAP(CAuditFilter)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDHELP, OnHelp)
	ON_BN_CLICKED(AUDITFILTER_ALL, OnAll)
	ON_BN_CLICKED(AUDITFILTER_ROSTER, OnRoster)
	ON_BN_CLICKED(AUDITFILTER_VEHICLEASSIGNMENT, OnVehicleassignment)
	ON_BN_CLICKED(AUDITFILTER_OPERATORCHECKINCHECKOUT, OnOperatorcheckincheckout)
	ON_BN_CLICKED(AUDITFILTER_OPERATORABSENCE, OnOperatorabsence)
	ON_BN_CLICKED(AUDITFILTER_OPENWORK, OnOpenwork)
	ON_BN_CLICKED(AUDITFILTER_ALLOPERATORS, OnAlloperators)
	ON_BN_CLICKED(AUDITFILTER_OPERATORBUTTON, OnOperatorbutton)
	ON_BN_CLICKED(AUDITFILTER_ALLREASONS, OnAllreasons)
	ON_BN_CLICKED(AUDITFILTER_REASONBUTTON, OnReasonbutton)
	ON_CBN_SELCHANGE(AUDITFILTER_OPERATOR, OnSelchangeOperator)
	ON_CBN_SELCHANGE(AUDITFILTER_REASON, OnSelchangeReason)
	ON_BN_CLICKED(IDFROMTODAY, OnFromtoday)
	ON_BN_CLICKED(IDTOTODAY, OnTotoday)
	ON_BN_CLICKED(AUDITFILTER_REGISTRATIONDATE, OnRegistrationdate)
	ON_BN_CLICKED(AUDITFILTER_OCCURRENCEDATE, OnOccurrencedate)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAuditFilter message handlers

BOOL CAuditFilter::OnInitDialog() 
{
	CDialog::OnInitDialog();
 
//
//  Set up pointers to the controls
// 
  pButtonREGISTRATIONDATE = (CButton *)GetDlgItem(AUDITFILTER_REGISTRATIONDATE);
  pButtonOCCURRENCEDATE = (CButton *)GetDlgItem(AUDITFILTER_OCCURRENCEDATE);
  pDTPickerFROMDATE = (CDTPicker *)GetDlgItem(AUDITFILTER_FROMDATE);
  pStaticUNTIL_TITLE = (CStatic *)GetDlgItem(AUDITFILTER_UNTIL_TITLE);
  pDTPickerTODATE = (CDTPicker *)GetDlgItem(AUDITFILTER_TODATE);
  pButtonFROMTODAY = (CButton *)GetDlgItem(IDFROMTODAY);
  pButtonTOTODAY = (CButton *)GetDlgItem(IDTOTODAY);
  pButtonALL = (CButton *)GetDlgItem(AUDITFILTER_ALL);
  pButtonROSTER = (CButton *)GetDlgItem(AUDITFILTER_ROSTER);
  pButtonVEHICLEASSIGNMENT = (CButton *)GetDlgItem(AUDITFILTER_VEHICLEASSIGNMENT);
  pButtonOPERATORCHECKINCHECKOUT = (CButton *)GetDlgItem(AUDITFILTER_OPERATORCHECKINCHECKOUT);
  pButtonOPERATORABSENCE = (CButton *)GetDlgItem(AUDITFILTER_OPERATORABSENCE);
  pButtonOPENWORK = (CButton *)GetDlgItem(AUDITFILTER_OPENWORK);
  pButtonALLOPERATORS = (CButton *)GetDlgItem(AUDITFILTER_ALLOPERATORS);
  pButtonOPERATORBUTTON = (CButton *)GetDlgItem(AUDITFILTER_OPERATORBUTTON);
  pComboBoxOPERATOR = (CComboBox *)GetDlgItem(AUDITFILTER_OPERATOR);
  pButtonALLREASONS = (CButton *)GetDlgItem(AUDITFILTER_ALLREASONS);
  pButtonREASONBUTTON = (CButton *)GetDlgItem(AUDITFILTER_REASONBUTTON);
  pComboBoxREASON = (CComboBox *)GetDlgItem(AUDITFILTER_REASON);

//
//  Set today's date on the To Date picker
//
  SetDateToToday(pDTPickerTODATE);
//
//  Load the drivers
//
  char szDriver[DRIVERS_LASTNAME_LENGTH + 2 + DRIVERS_FIRSTNAME_LENGTH + 1];
  int  rcode2;
  int  numDrivers;

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
//
//  Set up the absence reasons
//
  int nI, nJ;

  for(nI = 0; nI < m_numAbsenceReasons; nI++)
  {
    nJ = pComboBoxREASON->AddString(m_AbsenceReasons[nI].szText);
    pComboBoxREASON->SetItemData(nJ, nI);
  }
//
//  Default to registration date
//
  pButtonREGISTRATIONDATE->SetCheck(TRUE);
//
//  Default to all
//
  pButtonALL->SetCheck(TRUE);
	
  ReleaseCapture();
  	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CAuditFilter::OnClose() 
{
	OnCancel();
}

void CAuditFilter::OnCancel() 
{
	CDialog::OnCancel();
}

void CAuditFilter::OnHelp() 
{
}

void CAuditFilter::OnOK() 
{
  COleVariant v;
  long day;
  long month;
  long year;
  int  nI;
//
//  Get the values
//
//  From date
//
  v = pDTPickerFROMDATE->GetYear();
  year = v.lVal;
  v = pDTPickerFROMDATE->GetMonth();
  month = v.lVal;
  v = pDTPickerFROMDATE->GetDay();
  day = v.lVal;
  m_pSelections->fromDate = year * 10000 + month * 100 + day;
//
//  To date
//
  v = pDTPickerTODATE->GetYear();
  year = v.lVal;
  v = pDTPickerTODATE->GetMonth();
  month = v.lVal;
  v = pDTPickerTODATE->GetDay();
  day = v.lVal;
  m_pSelections->toDate = year * 10000 + month * 100 + day;
//
//  flags
//
  m_pSelections->flags = 0;
  if(pButtonALL->GetCheck())
  {
    m_pSelections->flags = AUDITFILTER_FLAGS_ALL;
  }
  else if(pButtonROSTER->GetCheck())
  {
    m_pSelections->flags = AUDITFILTER_FLAGS_ROSTER;
  }
  else if(pButtonVEHICLEASSIGNMENT->GetCheck())
  {
    m_pSelections->flags = AUDITFILTER_FLAGS_BUS;
  }
  else if(pButtonOPERATORCHECKINCHECKOUT->GetCheck())
  {
    m_pSelections->flags = AUDITFILTER_FLAGS_OPERATOR;
  }
  else if(pButtonOPERATORABSENCE->GetCheck())
  {
    m_pSelections->flags = AUDITFILTER_FLAGS_ABSENCE;
  }
  else if(pButtonOPENWORK->GetCheck())
  {
    m_pSelections->flags = AUDITFILTER_FLAGS_OPENWORK;
  }
  if(pButtonREGISTRATIONDATE->GetCheck())
  {
    m_pSelections->flags |= AUDITFILTER_FLAGS_REGISTRATIONDATE;
  }
//
//  Is there "other data"?
//
  m_pSelections->DRIVERSrecordID = NO_RECORD;
  m_pSelections->reasonNumber = NO_RECORD;
  if(pButtonOPERATORABSENCE->GetCheck())
  {
    if(pButtonALLOPERATORS->GetCheck())
    {
      m_pSelections->flags |= AUDITFILTER_FLAGS_ALLOPERATORS;
    }
    else
    {
      nI = (int)pComboBoxOPERATOR->GetCurSel();
      if(nI != CB_ERR)
      {
        m_pSelections->DRIVERSrecordID = pComboBoxOPERATOR->GetItemData(nI);
      }
    }
    if(pButtonALLREASONS->GetCheck())
    {
      m_pSelections->flags |= AUDITFILTER_FLAGS_ALLREASONS;
    }
    else
    {
      nI = (int)pComboBoxREASON->GetCurSel();
      if(nI != CB_ERR)
      {
        m_pSelections->reasonNumber = pComboBoxREASON->GetItemData(nI);
      }
    }
  }

	CDialog::OnOK();
}

void CAuditFilter::OnAll() 
{
  EnableOtherData(FALSE);	
}

void CAuditFilter::OnRoster() 
{
  EnableOtherData(FALSE);	
}

void CAuditFilter::OnVehicleassignment() 
{
  EnableOtherData(FALSE);	
}

void CAuditFilter::OnOperatorcheckincheckout() 
{
  EnableOtherData(FALSE);	
}

void CAuditFilter::OnOperatorabsence() 
{
  EnableOtherData(TRUE);	
}

void CAuditFilter::OnOpenwork() 
{
  EnableOtherData(FALSE);	
}

void CAuditFilter::OnAlloperators() 
{
  pComboBoxOPERATOR->SetCurSel(-1);
}

void CAuditFilter::OnOperatorbutton() 
{
  pComboBoxOPERATOR->SetCurSel(0);
}

void CAuditFilter::OnAllreasons() 
{
  pComboBoxREASON->SetCurSel(-1);
}

void CAuditFilter::OnReasonbutton() 
{
  pComboBoxREASON->SetCurSel(0);
}

void CAuditFilter::OnSelchangeOperator() 
{
  pButtonALLOPERATORS->SetCheck(FALSE);
  pButtonOPERATORBUTTON->SetCheck(TRUE);
}

void CAuditFilter::OnSelchangeReason() 
{
  pButtonALLREASONS->SetCheck(FALSE);
  pButtonREASONBUTTON->SetCheck(TRUE);
}

void CAuditFilter::EnableOtherData(BOOL bEnable)
{
  pButtonALLOPERATORS->SetCheck(bEnable);
  pButtonALLREASONS->SetCheck(bEnable);

  pButtonOPERATORBUTTON->SetCheck(FALSE);
  pComboBoxOPERATOR->SetCurSel(-1);
  pButtonREASONBUTTON->SetCheck(FALSE);
  pComboBoxREASON->SetCurSel(-1);

  pButtonALLOPERATORS->EnableWindow(bEnable);
  pButtonOPERATORBUTTON->EnableWindow(bEnable);
  pComboBoxOPERATOR->EnableWindow(bEnable);
  pButtonALLREASONS->EnableWindow(bEnable);
  pButtonREASONBUTTON->EnableWindow(bEnable);
  pComboBoxREASON->EnableWindow(bEnable);
}

void CAuditFilter::OnFromtoday() 
{
  SetDateToToday(pDTPickerFROMDATE);
}

void CAuditFilter::OnTotoday() 
{
  SetDateToToday(pDTPickerTODATE);
}

void CAuditFilter::SetDateToToday(CDTPicker* pDTP)
{
  COleVariant v;
  CTime time = CTime::GetCurrentTime();

  long day = time.GetDay();
  long month = time.GetMonth();
  long year = time.GetYear();

  v = year;
  pDTPickerTODATE->SetYear(v);
  v = month;
  pDTPickerTODATE->SetMonth(v);
  v = day;
  pDTPickerTODATE->SetDay(v);

  v = year;
  pDTP->SetYear(v);
  v = month;
  pDTP->SetMonth(v);
  v = day;
  pDTP->SetDay(v);
}

void CAuditFilter::OnRegistrationdate() 
{
  pStaticUNTIL_TITLE->EnableWindow(TRUE);
  pDTPickerTODATE->EnableWindow(TRUE);
  pButtonTOTODAY->EnableWindow(TRUE);
}

void CAuditFilter::OnOccurrencedate() 
{
  pStaticUNTIL_TITLE->EnableWindow(FALSE);
  pDTPickerTODATE->EnableWindow(FALSE);
  pButtonTOTODAY->EnableWindow(FALSE);
}

// UnpaidTimeFilter.cpp : implementation file
//

#include "stdafx.h"
extern "C"{
#include "TMSHeader.h"

}  // EXTERN C

#include "tms.h"
#include "DTPicker.h"
#include "UnpaidTimeFilter.h"
#include "DriversToConsider.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//
//  Call the dialog
//
BOOL UnpaidTimeFilterDialog(UNPAIDTIMEPassedDataDef *pPassedData)
{
  CUnpaidTimeFilter dlg(NULL, pPassedData);

  return(dlg.DoModal());

}

//
//  Dispatch Sheet callable from C
//
extern "C"
{
  BOOL UnpaidTimeFilter(UNPAIDTIMEPassedDataDef *pPassedData)
  {
    return(UnpaidTimeFilterDialog(pPassedData));
  }
}

/////////////////////////////////////////////////////////////////////////////
// CUnpaidTimeFilter dialog


CUnpaidTimeFilter::CUnpaidTimeFilter(CWnd* pParent /*=NULL*/, UNPAIDTIMEPassedDataDef* pPassedData)
	: CDialog(CUnpaidTimeFilter::IDD, pParent)
{
  m_pPassedData = pPassedData;

	//{{AFX_DATA_INIT(CUnpaidTimeFilter)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CUnpaidTimeFilter::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CUnpaidTimeFilter)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CUnpaidTimeFilter, CDialog)
	//{{AFX_MSG_MAP(CUnpaidTimeFilter)
	ON_WM_CLOSE()
	ON_BN_CLICKED(UNPAIDTIMEFILTER_ALLOPERATORS, OnAlloperators)
	ON_BN_CLICKED(UNPAIDTIMEFILTER_OPERATORBUTTON, OnOperatorbutton)
	ON_BN_CLICKED(IDCONSIDER, OnConsider)
	ON_CBN_SELENDOK(UNPAIDTIMEFILTER_OPERATOR, OnSelendokOperator)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CUnpaidTimeFilter message handlers

BOOL CUnpaidTimeFilter::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
  pDTPickerFROMDATE = (CDTPicker *)GetDlgItem(UNPAIDTIMEFILTER_FROMDATE);
  pButtonALLOPERATORS = (CButton *)GetDlgItem(UNPAIDTIMEFILTER_ALLOPERATORS);
  pButtonOPERATORBUTTON = (CButton *)GetDlgItem(UNPAIDTIMEFILTER_OPERATORBUTTON);
  pComboBoxOPERATOR = (CComboBox *)GetDlgItem(UNPAIDTIMEFILTER_OPERATOR);
	
//
//  Set today's date on the Date and Time picker
//
  CTime time = CTime::GetCurrentTime();

  COleVariant v;
  long day = time.GetDay();
  long month = time.GetMonth();
  long year = time.GetYear();

  v = year;
  pDTPickerFROMDATE->SetYear(v);
  v = month;
  pDTPickerFROMDATE->SetMonth(v);
  v = day;
  pDTPickerFROMDATE->SetDay(v);
//
//  Set up the operator list
//
  pButtonALLOPERATORS->SetCheck(TRUE);
  SetupDriverList();
//
//  All done
//
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CUnpaidTimeFilter::OnClose() 
{
  OnCancel();
}

void CUnpaidTimeFilter::OnCancel() 
{
	CDialog::OnCancel();
}

void CUnpaidTimeFilter::OnOK() 
{
  int nI;
//
//  Operator
//
  if(pButtonALLOPERATORS->GetCheck())
  {
    m_pPassedData->DRIVERSrecordID = NO_RECORD;
  }
  else
  {
    nI = pComboBoxOPERATOR->GetCurSel();
    if(nI == CB_ERR)
    {
      m_pPassedData->DRIVERSrecordID = NO_RECORD;
    }
    else
    {
      m_pPassedData->DRIVERSrecordID = pComboBoxOPERATOR->GetItemData(nI);
    }
  }
//
//  Date
//
  COleVariant v;
  long day;
  long month;
  long year;

  v = pDTPickerFROMDATE->GetYear();
  year = v.lVal;
  v = pDTPickerFROMDATE->GetMonth();
  month = v.lVal;
  v = pDTPickerFROMDATE->GetDay();
  day = v.lVal;

  m_pPassedData->date = year * 10000 + month * 100 + day;

	CDialog::OnOK();
}

void CUnpaidTimeFilter::OnAlloperators() 
{
  pComboBoxOPERATOR->SetCurSel(-1);
}

void CUnpaidTimeFilter::OnOperatorbutton() 
{
  pComboBoxOPERATOR->SetCurSel(0);
}

void CUnpaidTimeFilter::OnSelendokOperator() 
{
  if(pButtonALLOPERATORS->GetCheck())
  {
    pButtonALLOPERATORS->SetCheck(FALSE);
    pButtonOPERATORBUTTON->SetCheck(TRUE);
  }
}

void CUnpaidTimeFilter::OnConsider() 
{
  CDriversToConsider dlg;

  if(dlg.DoModal())
  {
    SetupDriverList();
  }
}

void CUnpaidTimeFilter::SetupDriverList()
{
  char szDriver[DRIVERS_LASTNAME_LENGTH + 2 + DRIVERS_FIRSTNAME_LENGTH + 1];
  int  numDrivers;
  int  rcode2;

  pComboBoxOPERATOR->ResetContent();

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
  pComboBoxOPERATOR->SetCurSel(-1);

}

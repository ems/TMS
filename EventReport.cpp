// EventReport.cpp : implementation file
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

//
//  Call the dialog
//
BOOL DispatchSheetDialog(EVENTREPORTPassedDataDef *pPassedData)
{
  CEventReport dlg(NULL, pPassedData);

  return(dlg.DoModal());

}

//
//  Dispatch Sheet callable from C
//
extern "C"
{
  BOOL EventReport(EVENTREPORTPassedDataDef *pPassedData)
  {
    return(DispatchSheetDialog(pPassedData));
  }
}


/////////////////////////////////////////////////////////////////////////////
// CEventReport dialog


CEventReport::CEventReport(CWnd* pParent, EVENTREPORTPassedDataDef *pPassedData)
	: CDialog(CEventReport::IDD, pParent)
{
  m_pPassedData = pPassedData;

	//{{AFX_DATA_INIT(CEventReport)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CEventReport::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CEventReport)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CEventReport, CDialog)
	//{{AFX_MSG_MAP(CEventReport)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDHELP, OnHelp)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEventReport message handlers

BOOL CEventReport::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
  pComboBoxDIVISION = (CComboBox *)GetDlgItem(EVENTREPORT_DIVISION);
  pDTPickerDATE = (CDTPicker *)GetDlgItem(EVENTREPORT_DATE);
  pButtonREPORTTIME = (CButton *)GetDlgItem(EVENTREPORT_REPORTTIME);
  pButtonSTARTOFPIECETRAVEL = (CButton *)GetDlgItem(EVENTREPORT_STARTOFPIECETRAVEL);
  pButtonSTARTOFPIECE = (CButton *)GetDlgItem(EVENTREPORT_STARTOFPIECE);
  pButtonENDOFPIECETRAVEL = (CButton *)GetDlgItem(EVENTREPORT_ENDOFPIECETRAVEL);
  pButtonENDOFPIECE = (CButton *)GetDlgItem(EVENTREPORT_ENDOFPIECE);
  pButtonEXTRABOARD = (CButton *)GetDlgItem(EVENTREPORT_EXTRABOARD);

//
//  Set up the division list
//
  if((SetUpDivisionList(this->m_hWnd, EVENTREPORT_DIVISION, m_DivisionRecordID)) == 0)
  {
    OnCancel();
    return TRUE;
  }
//
//  Set today's date on the Date and Time picker
//
  COleVariant v;
  long day;
  long month;
  long year;

  CTime time = CTime::GetCurrentTime();
  day = time.GetDay();
  month = time.GetMonth();
  year = time.GetYear();

  v = year;
  pDTPickerDATE->SetYear(v);
  v = month;
  pDTPickerDATE->SetMonth(v);
  v = day;
  pDTPickerDATE->SetDay(v);

  m_pPassedData->todaysDate = year * 10000 + month * 100 + day;
//
//  Default everything to checked
//
  pButtonREPORTTIME->SetCheck(TRUE);
  pButtonSTARTOFPIECETRAVEL->SetCheck(TRUE);
  pButtonSTARTOFPIECE->SetCheck(TRUE);
  pButtonENDOFPIECE->SetCheck(TRUE);
  pButtonENDOFPIECETRAVEL->SetCheck(TRUE);
  pButtonEXTRABOARD->SetCheck(TRUE);
//
//  Done
//
  ReleaseCapture();

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CEventReport::OnClose() 
{
  OnCancel();
}

void CEventReport::OnCancel() 
{
	// TODO: Add extra cleanup here
	
	CDialog::OnCancel();
}

void CEventReport::OnHelp() 
{
	// TODO: Add your control notification handler code here
	
}

void CEventReport::OnOK() 
{
//
//  Extract the year and day of week from the date control
//
  COleVariant v;
  long year, month, day;
  long days[7] = {6, 0, 1, 2, 3, 4, 5};

  v = pDTPickerDATE->GetDay();
  day = v.lVal;
  v = pDTPickerDATE->GetMonth();
  month = v.lVal;
  v = pDTPickerDATE->GetYear();
  year = v.lVal;
  v = pDTPickerDATE->GetDayOfWeek();
  m_pPassedData->dayOfWeek = days[v.lVal - 1];
  m_pPassedData->date = year * 10000 + month * 100 + day;
//
//  Set up the date as a text string
//
  CTime   t(year, month, day, 0, 0, 0);
  CString x = t.Format(_T("%A, %B %d, %Y"));
  strcpy(m_pPassedData->szDate, x);
//
//  Get the rest of the selections
//
  m_pPassedData->flags = 0;
//
//  Division
//
  int nI = pComboBoxDIVISION->GetCurSel();

  m_pPassedData->DIVISIONSrecordID = (nI == CB_ERR ? NO_RECORD : pComboBoxDIVISION->GetItemData(nI));
//
//  Checkboxes
//
  
  if(pButtonREPORTTIME->GetCheck())
  {
    m_pPassedData->flags |= EVENTREPORT_FLAG_REPORTTIME;
  }
  if(pButtonSTARTOFPIECETRAVEL->GetCheck())
  {
    m_pPassedData->flags |= EVENTREPORT_FLAG_STARTOFPIECETRAVEL;
  }
  if(pButtonSTARTOFPIECE->GetCheck())
  {
    m_pPassedData->flags |= EVENTREPORT_FLAG_STARTOFPIECE;
  }
  if(pButtonENDOFPIECE->GetCheck())
  {
    m_pPassedData->flags |= EVENTREPORT_FLAG_ENDOFPIECE;
  }
  if(pButtonENDOFPIECETRAVEL->GetCheck())
  {
    m_pPassedData->flags |= EVENTREPORT_FLAG_ENDOFPIECETRAVEL;
  }
  if(pButtonEXTRABOARD->GetCheck())
  {
    m_pPassedData->flags |= EVENTREPORT_FLAG_EXTRABOARDDUTY;
  }

	CDialog::OnOK();
}

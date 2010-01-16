// DisciplineFilter.cpp : implementation file
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

BOOL DisciplineFilterCpp(DISCIPLINEFILTERSELECTIONSDef *pSelections)
{
  CDisciplineFilter dlg(NULL, pSelections);

  return(dlg.DoModal());
}

extern "C"{

BOOL DisciplineFilter(DISCIPLINEFILTERSELECTIONSDef *pSelections)
{
  return(DisciplineFilterCpp(pSelections));
}

}/////////////////////////////////////////////////////////////////////////////
// CDisciplineFilter dialog


CDisciplineFilter::CDisciplineFilter(CWnd* pParent, DISCIPLINEFILTERSELECTIONSDef *pSelections)
	: CDialog(CDisciplineFilter::IDD, pParent)
{
  m_pSelections = pSelections;
	//{{AFX_DATA_INIT(CDisciplineFilter)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CDisciplineFilter::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDisciplineFilter)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDisciplineFilter, CDialog)
	//{{AFX_MSG_MAP(CDisciplineFilter)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDHELP, OnHelp)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDisciplineFilter message handlers

BOOL CDisciplineFilter::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
//
//  Set up pointers to the controls
// 
  pDTPickerFROMDATE = (CDTPicker *)GetDlgItem(DISCIPLINEFILTER_FROMDATE);
  pDTPickerTODATE = (CDTPicker *)GetDlgItem(DISCIPLINEFILTER_TODATE);
	
//
//  Set today's date on the To Date picker
//
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

//
//  Set the from date to a year ago
//
  year--;
  v = year;
  pDTPickerFROMDATE->SetYear(v);
  v = month;
  pDTPickerFROMDATE->SetMonth(v);
  v = day;
  pDTPickerFROMDATE->SetDay(v);
  
	
  ReleaseCapture();
  	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CDisciplineFilter::OnClose() 
{
  OnCancel();
}

void CDisciplineFilter::OnCancel() 
{
	// TODO: Add extra cleanup here
	
	CDialog::OnCancel();
}

void CDisciplineFilter::OnHelp() 
{
	// TODO: Add your control notification handler code here
	
}

void CDisciplineFilter::OnOK() 
{
  COleVariant v;
  long day;
  long month;
  long year;
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

	CDialog::OnOK();
}

// DateRange.cpp : implementation file
//

#include "stdafx.h"
extern "C"{
#include "TMSHeader.h"

}  // EXTERN C

#include "tms.h"
#include "DTPicker.h"
#include "DateRange.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//
//  Call the dialog
//
BOOL DateRangeDialog(long *pFromDate, long *pToDate)
{
  CDateRange dlg(NULL, pFromDate, pToDate);

  return(dlg.DoModal());

}

//
//  Date Range callable from C
//
extern "C"
{
  BOOL GetDateRange(long *pFromDate, long *pToDate)
  {
    return(DateRangeDialog(pFromDate, pToDate));
  }
}

/////////////////////////////////////////////////////////////////////////////
// CDateRange dialog


CDateRange::CDateRange(CWnd* pParent /*=NULL*/, long *pFromDate, long* pToDate)
	: CDialog(CDateRange::IDD, pParent)
{
  m_pFromDate = pFromDate;
  m_pToDate = pToDate;
	//{{AFX_DATA_INIT(CDateRange)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CDateRange::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDateRange)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDateRange, CDialog)
	//{{AFX_MSG_MAP(CDateRange)
	ON_BN_CLICKED(IDFROMTODAY, OnFromtoday)
	ON_BN_CLICKED(IDTOTODAY, OnTotoday)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDHELP, OnHelp)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDateRange message handlers

BOOL CDateRange::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
  pDTPickerFROMDATE = (CDTPicker *)GetDlgItem(DATERANGE_FROMDATE);
  pDTPickerTODATE = (CDTPicker *)GetDlgItem(DATERANGE_TODATE);
	
  SetDateToToday(pDTPickerFROMDATE);
  SetDateToToday(pDTPickerTODATE);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CDateRange::OnFromtoday() 
{
  SetDateToToday(pDTPickerFROMDATE);
}

void CDateRange::OnTotoday() 
{
  SetDateToToday(pDTPickerTODATE);
}

void CDateRange::SetDateToToday(CDTPicker* pDTP)
{
  COleVariant v;
  CTime time = CTime::GetCurrentTime();

  long day = time.GetDay();
  long month = time.GetMonth();
  long year = time.GetYear();

  v = year;
  pDTP->SetYear(v);
  v = month;
  pDTP->SetMonth(v);
  v = day;
  pDTP->SetDay(v);
}

void CDateRange::OnClose() 
{
  OnCancel();
}

void CDateRange::OnCancel() 
{
	CDialog::OnCancel();
}

void CDateRange::OnHelp() 
{
}

void CDateRange::OnOK() 
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
  *m_pFromDate = year * 10000 + month * 100 + day;
//
//  To date
//
  v = pDTPickerTODATE->GetYear();
  year = v.lVal;
  v = pDTPickerTODATE->GetMonth();
  month = v.lVal;
  v = pDTPickerTODATE->GetDay();
  day = v.lVal;
  *m_pToDate = year * 10000 + month * 100 + day;
	
	CDialog::OnOK();
}

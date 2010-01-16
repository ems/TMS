// PickDate.cpp : implementation file
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
BOOL GetDateDialog(long *pDate)
{
  CPickDate dlg(NULL, pDate);

  return(dlg.DoModal());

}

//
//  Date Range callable from C
//
extern "C"
{
  BOOL PickDate(long *pDate)
  {
    return(GetDateDialog(pDate));
  }
}
/////////////////////////////////////////////////////////////////////////////
// CPickDate dialog


CPickDate::CPickDate(CWnd* pParent, long* pDate)
	: CDialog(CPickDate::IDD, pParent)
{
  m_pDate = pDate;
	//{{AFX_DATA_INIT(CPickDate)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CPickDate::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPickDate)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CPickDate, CDialog)
	//{{AFX_MSG_MAP(CPickDate)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDHELP, OnHelp)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPickDate message handlers

BOOL CPickDate::OnInitDialog() 
{
	CDialog::OnInitDialog();
//
//  Set up a pointer to the control
//
  pDTPickerDATE = (CDTPicker *)GetDlgItem(PICKDATE_DATE);

  COleVariant v;
  CTime time = CTime::GetCurrentTime();
  long day;
  long month;
  long year;
//
//  If we come in with NO_RECORD, set today's date on the Date and Time pickers
//
  if(*m_pDate == NO_RECORD)
  {
    day = time.GetDay();
    month = time.GetMonth();
    year = time.GetYear();
  }
//
//  Otherwise, set the date we came in with
//
  else
  {
    GetYMD(*m_pDate, &year, &month, &day);
  }

  v = year;
  pDTPickerDATE->SetYear(v);
  v = month;
  pDTPickerDATE->SetMonth(v);
  v = day;
  pDTPickerDATE->SetDay(v);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CPickDate::OnClose() 
{
	OnCancel();
}

void CPickDate::OnHelp() 
{
}

void CPickDate::OnCancel() 
{
	CDialog::OnCancel();
}

void CPickDate::OnOK() 
{
  COleVariant v;
  long day;
  long month;
  long year;

  v = pDTPickerDATE->GetYear();
  year = v.lVal;
  v = pDTPickerDATE->GetMonth();
  month = v.lVal;
  v = pDTPickerDATE->GetDay();
  day = v.lVal;

  *m_pDate = year * 10000 + month * 100 + day;

	CDialog::OnOK();
}

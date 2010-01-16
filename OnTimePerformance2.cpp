// OnTimePerformance2.cpp : implementation file
//

#include "stdafx.h"
extern "C"
{
#include "TMSHeader.h"
}
#include "_line3dh.h"
#include "AddDialogs.h"
#include "OnTimePerformance2.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

int TMSRPT48FilterDialog(TMSRPT48PassedDataDef *pPD)
{
  COnTimePerformance2 dlg(NULL, pPD);

  return (dlg.DoModal());

}

extern "C"
{
  BOOL TMSRPT48Filter(TMSRPT48PassedDataDef *pPD)
  {
    return(TMSRPT48FilterDialog(pPD) == IDOK);
  }
}

/////////////////////////////////////////////////////////////////////////////
// COnTimePerformance2 dialog


COnTimePerformance2::COnTimePerformance2(CWnd* pParent, TMSRPT48PassedDataDef *pPD)
	: CDialog(COnTimePerformance2::IDD, pParent)
{
  m_pPD = pPD;

	//{{AFX_DATA_INIT(COnTimePerformance2)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void COnTimePerformance2::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(COnTimePerformance2)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(COnTimePerformance2, CDialog)
	//{{AFX_MSG_MAP(COnTimePerformance2)
	ON_BN_CLICKED(IDHELP, OnHelp)
	ON_WM_CLOSE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// COnTimePerformance2 message handlers

BOOL COnTimePerformance2::OnInitDialog() 
{
	CDialog::OnInitDialog();

  COleVariant v;
  long month;
  long year;

  pDTPickerFROMDATE = (CDTPicker *)GetDlgItem(ONTIMEPERF2_FROMDATE);
  pDTPickerTODATE = (CDTPicker *)GetDlgItem(ONTIMEPERF2_TODATE);
//
//  Set today's date on the Date and Time pickers
//
  CTime time = CTime::GetCurrentTime();

  month = time.GetMonth();
  year = time.GetYear();
  v = year;
  pDTPickerFROMDATE->SetYear(v);
  pDTPickerTODATE->SetYear(v);
  v = month;
  pDTPickerFROMDATE->SetMonth(v);
  pDTPickerTODATE->SetMonth(v);

  ReleaseCapture();

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void COnTimePerformance2::OnCancel() 
{
	// TODO: Add extra cleanup here
	
	CDialog::OnCancel();
}

void COnTimePerformance2::OnHelp() 
{
	// TODO: Add your control notification handler code here
	
}

void COnTimePerformance2::OnClose() 
{
  OnCancel();
}

void COnTimePerformance2::OnOK() 
{
  VARIANT v;

//
//  From date
//
  v = pDTPickerFROMDATE->GetYear();
  m_pPD->fromYear = v.lVal;
  v = pDTPickerFROMDATE->GetMonth();
  m_pPD->fromMonth = v.lVal;
//
//  To date
//
  v = pDTPickerTODATE->GetYear();
  m_pPD->toYear = v.lVal;
  v = pDTPickerTODATE->GetMonth();
  m_pPD->toMonth = v.lVal;
	
	CDialog::OnOK();
}

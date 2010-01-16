// UntilTime.cpp : implementation file
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

/////////////////////////////////////////////////////////////////////////////
// CUntilTime dialog


CUntilTime::CUntilTime(CWnd* pParent /*=NULL*/, long* pTime)
	: CDialog(CUntilTime::IDD, pParent)
{
  m_pTime = pTime;

	//{{AFX_DATA_INIT(CUntilTime)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CUntilTime::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CUntilTime)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CUntilTime, CDialog)
	//{{AFX_MSG_MAP(CUntilTime)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDHELP, OnHelp)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CUntilTime message handlers

BOOL CUntilTime::OnInitDialog() 
{
	CDialog::OnInitDialog();
//
//  Set up a pointer to the control
//	
  pDTPickerTIME = (CDTPicker *)GetDlgItem(UNTILTIME_TIME);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CUntilTime::OnClose() 
{
  OnCancel();
}

void CUntilTime::OnCancel() 
{
	CDialog::OnCancel();
}

void CUntilTime::OnHelp() 
{
}

void CUntilTime::OnOK() 
{
  COleVariant v;
  long hours, minutes, seconds;
//
//  Time
//
  v = pDTPickerTIME->GetHour();
  hours = v.lVal;
  v = pDTPickerTIME->GetMinute();
  minutes = v.lVal;
  v = pDTPickerTIME->GetSecond();
  seconds = v.lVal;
  *m_pTime = hours * 3600 + minutes * 60 + seconds;
	
	CDialog::OnOK();
}


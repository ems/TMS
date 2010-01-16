// DailyOpsRL.cpp : implementation file
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
// CDailyOpsRL dialog


CDailyOpsRL::CDailyOpsRL(CWnd* pParent /*=NULL*/)
	: CDialog(CDailyOpsRL::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDailyOpsRL)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CDailyOpsRL::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDailyOpsRL)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDailyOpsRL, CDialog)
	//{{AFX_MSG_MAP(CDailyOpsRL)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDHELP, OnHelp)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDailyOpsRL message handlers

BOOL CDailyOpsRL::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CDailyOpsRL::OnClose() 
{
	// TODO: Add your message handler code here and/or call default
	
	CDialog::OnClose();
}

void CDailyOpsRL::OnCancel() 
{
	// TODO: Add extra cleanup here
	
	CDialog::OnCancel();
}

void CDailyOpsRL::OnHelp() 
{
	// TODO: Add your control notification handler code here
	
}

void CDailyOpsRL::OnOK() 
{
	// TODO: Add extra validation here
	
	CDialog::OnOK();
}

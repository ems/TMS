// Expiries.cpp : implementation file
//

#include "stdafx.h"
extern "C"{
#include "TMSHeader.h"

}  // EXTERN C

#include "tms.h"
#include "Expiries.h"
#include "DailyOpsHeader.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CExpiries dialog


CExpiries::CExpiries(CWnd* pParent, BOOL* pLicense, BOOL* pMedical, int* pDays)
	: CDialog(CExpiries::IDD, pParent)
{
  m_pLicense = pLicense;
  m_pMedical = pMedical;
  m_pDays = pDays;

	//{{AFX_DATA_INIT(CExpiries)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CExpiries::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CExpiries)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CExpiries, CDialog)
	//{{AFX_MSG_MAP(CExpiries)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDHELP, OnHelp)
	ON_BN_CLICKED(IDCONSIDER, OnConsider)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CExpiries message handlers

BOOL CExpiries::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
  pButtonCHECKLICENSE = (CButton *)GetDlgItem(EXPIRIES_CHECKLICENSE);
  pButtonCHECKMEDICAL = (CButton *)GetDlgItem(EXPIRIES_CHECKMEDICAL);
  pEditDAYS = (CEdit *)GetDlgItem(EXPIRES_DAYS);

  pButtonCHECKLICENSE->SetCheck(TRUE);
  pButtonCHECKMEDICAL->SetCheck(TRUE);
  pEditDAYS->SetWindowText("30");

	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CExpiries::OnClose() 
{
  OnCancel();
}

void CExpiries::OnCancel() 
{
	// TODO: Add extra cleanup here
	
	CDialog::OnCancel();
}

void CExpiries::OnHelp() 
{
	// TODO: Add your control notification handler code here
	
}

void CExpiries::OnConsider() 
{
  CDriversToConsider dlg;

  dlg.DoModal();
}

void CExpiries::OnOK() 
{
  *m_pLicense = pButtonCHECKLICENSE->GetCheck();
  *m_pMedical = pButtonCHECKMEDICAL->GetCheck();
  pEditDAYS->GetWindowText(tempString, TEMPSTRING_LENGTH);
  *m_pDays = atoi(tempString);
	
	CDialog::OnOK();
}

//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
// CCLogin.cpp : implementation file
//

#include "stdafx.h"
extern "C"
{
#include "TMSHeader.h"
}
#include "cc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CCCLogin dialog


CCCLogin::CCCLogin(CWnd* pParent /*=NULL*/)
	: CDialog(CCCLogin::IDD, pParent)
{
	//{{AFX_DATA_INIT(CCCLogin)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CCCLogin::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CCCLogin)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CCCLogin, CDialog)
	//{{AFX_MSG_MAP(CCCLogin)
	ON_BN_CLICKED(IDHELP, OnHelp)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCCLogin message handlers

BOOL CCCLogin::OnInitDialog() 
{
	CDialog::OnInitDialog();

  pEditNAME = (CEdit *)GetDlgItem(CCLOGIN_NAME);
  pEditPASSWORD = (CEdit *)GetDlgItem(CCLOGIN_PASSWORD);	
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CCCLogin::OnOK() 
{
  BOOL bSupervisor;
  char szName[SUPERVISORS_NAME_LENGTH], szPassword[SUPERVISORS_PASSWORD_LENGTH];
  int  nI;

  pEditNAME->GetWindowText(szName, SUPERVISORS_NAME_LENGTH);
  pEditPASSWORD->GetWindowText(szPassword, SUPERVISORS_PASSWORD_LENGTH);

  for(bSupervisor = FALSE, nI = 0; nI < m_numSupervisors; nI++)
  {
    if(strcmp(szName, m_Supervisors[nI].szName) == 0 &&
          strcmp(szPassword, m_Supervisors[nI].szPassword) == 0)
    {
      bSupervisor = TRUE;
      break;
    }
  }

  if(bSupervisor)
  {
  	CDialog::OnOK();
  }
  else
  {
    TMSError(NULL, MB_ICONSTOP, ERROR_196, (HANDLE)NULL);
  }
}

void CCCLogin::OnCancel() 
{
	// TODO: Add extra cleanup here
	
	CDialog::OnCancel();
}

void CCCLogin::OnHelp() 
{
	// TODO: Add your control notification handler code here
	
}

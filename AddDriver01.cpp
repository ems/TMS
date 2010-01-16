//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
// AddDriver01.cpp : implementation file
//

#include "stdafx.h"
extern "C"{
#include "TMSHeader.h"
}
#include "TMS.h"
#include "AddDialogs.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CAddDriver01 property page

IMPLEMENT_DYNCREATE(CAddDriver01, CPropertyPage)

CAddDriver01::CAddDriver01() : CPropertyPage(CAddDriver01::IDD)
{
	//{{AFX_DATA_INIT(CAddDriver01)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

CAddDriver01::~CAddDriver01()
{
}

void CAddDriver01::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAddDriver01)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CAddDriver01, CPropertyPage)
	//{{AFX_MSG_MAP(CAddDriver01)
	ON_BN_CLICKED(ADDDRIVER_PHOTO, OnPhoto)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAddDriver01 message handlers

BOOL CAddDriver01::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();
	
	CRect brect;
	GetDlgItem(ADDDRIVER_PHOTO)->GetWindowRect(brect);
	ScreenToClient(brect);
	m_bmpCtrl.Create(WS_CHILD|WS_VISIBLE,brect,this);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CAddDriver01::OnPhoto() 
{
  CEdit* pEditPATH = (CEdit *)GetDlgItem(ADDDRIVER_BITMAPPATH);

	CFileDialog dlg(TRUE,"*.bmp","",OFN_HIDEREADONLY,"Bitmap Files (*.bmp)|*.bmp||",this);
	dlg.m_ofn.lpstrTitle = "Select Bitmap";
	if(dlg.DoModal() == IDOK)
  {
		m_bmpCtrl.LoadFile(dlg.GetPathName());
    pEditPATH->SetWindowText(dlg.GetPathName());
  }
}

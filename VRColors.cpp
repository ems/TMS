//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
// VRColors.cpp : implementation file
//

#include "stdafx.h"
extern "C"
{
#include "TMSHeader.h"
}
#include "ColorBtn.h"
#include "VRColors.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CVRColors dialog


CVRColors::CVRColors(CWnd* pParent, VRCOLORSDef *pVRColors)
	: CDialog(CVRColors::IDD, pParent)
{
  m_pVRColors = pVRColors;
	//{{AFX_DATA_INIT(CVRColors)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CVRColors::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CVRColors)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CVRColors, CDialog)
	//{{AFX_MSG_MAP(CVRColors)
	ON_BN_CLICKED(IDHELP, OnHelp)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CVRColors message handlers


BOOL CVRColors::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
  m_cbCut.SubclassDlgItem(VRCOLORS_CUT, this);
  m_cbIllegal.SubclassDlgItem(VRCOLORS_ILLEGAL, this);
  m_cbFrozen.SubclassDlgItem(VRCOLORS_FROZEN, this);
  m_cbUncut.SubclassDlgItem(VRCOLORS_UNCUT, this);

  m_cbCut.currentcolor = m_pVRColors->cut;
  m_cbIllegal.currentcolor = m_pVRColors->illegal;
  m_cbFrozen.currentcolor = m_pVRColors->frozen;
  m_cbUncut.currentcolor = m_pVRColors->uncut;

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CVRColors::OnCancel() 
{
	CDialog::OnCancel();
}

void CVRColors::OnHelp() 
{
  ::WinHelp(m_hWnd, szarHelpFile, HELP_CONTEXT, Technical_Support);
}

void CVRColors::OnOK() 
{
  m_pVRColors->cut = m_cbCut.currentcolor;
  m_pVRColors->illegal = m_cbIllegal.currentcolor;
  m_pVRColors->frozen = m_cbFrozen.currentcolor;
  m_pVRColors->uncut = m_cbUncut.currentcolor;

	CDialog::OnOK();
}


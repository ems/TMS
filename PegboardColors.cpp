// PegboardColors.cpp : implementation file
//

#include "stdafx.h"
extern "C"
{
#include "TMSHeader.h"
}
#include "ColorBtn.h"
#include "PegboardColors.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CPegboardColors dialog


CPegboardColors::CPegboardColors(CWnd* pParent /*=NULL*/)
	: CDialog(CPegboardColors::IDD, pParent)
{
	//{{AFX_DATA_INIT(CPegboardColors)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CPegboardColors::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPegboardColors)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CPegboardColors, CDialog)
	//{{AFX_MSG_MAP(CPegboardColors)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDHELP, OnHelp)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPegboardColors message handlers

BOOL CPegboardColors::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
  m_cbAvailable.SubclassDlgItem(PEGBOARDCOLORS_AVAILABLE, this);
  m_cbAssigned.SubclassDlgItem(PEGBOARDCOLORS_ASSIGNED, this);
  m_cbOutOfService.SubclassDlgItem(PEGBOARDCOLORS_OUTOFSERVICE, this);
  m_cbShortShift.SubclassDlgItem(PEGBOARDCOLORS_SHORTSHIFT, this);
  m_cbCharterReserve.SubclassDlgItem(PEGBOARDCOLORS_CHARTERRESERVE, this);
  m_cbSightseeingReserve.SubclassDlgItem(PEGBOARDCOLORS_SIGHTSEEINGRESERVE, this);

  m_cbAvailable.currentcolor = m_PBCOLORS.crAvailable;
  m_cbAssigned.currentcolor = m_PBCOLORS.crAssigned;
  m_cbOutOfService.currentcolor = m_PBCOLORS.crOutOfService;
  m_cbShortShift.currentcolor = m_PBCOLORS.crShortShift;
  m_cbCharterReserve.currentcolor = m_PBCOLORS.crCharterReserve;
  m_cbSightseeingReserve.currentcolor = m_PBCOLORS.crSightseeingReserve;
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CPegboardColors::OnClose() 
{
  OnCancel();
}

void CPegboardColors::OnCancel() 
{
	CDialog::OnCancel();
}

void CPegboardColors::OnHelp() 
{
}

void CPegboardColors::OnOK() 
{
  m_PBCOLORS.crAvailable = m_cbAvailable.currentcolor;
  m_PBCOLORS.crAssigned = m_cbAssigned.currentcolor;
  m_PBCOLORS.crOutOfService = m_cbOutOfService.currentcolor;
  m_PBCOLORS.crShortShift = m_cbShortShift.currentcolor;
  m_PBCOLORS.crCharterReserve =m_cbCharterReserve.currentcolor;
  m_PBCOLORS.crSightseeingReserve = m_cbSightseeingReserve.currentcolor;
	
	CDialog::OnOK();
}

// RosterOrder.cpp : implementation file
//

#include "stdafx.h"
extern "C" {
#include "TMSHeader.h"
}

#include "TMS.h"
#include "RosterOrder.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CRosterOrder dialog


CRosterOrder::CRosterOrder(CWnd* pParent /*=NULL*/, DIVISIONDATADef* pData)
	: CDialog(CRosterOrder::IDD, pParent)
{
  m_pData = pData;
	//{{AFX_DATA_INIT(CRosterOrder)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CRosterOrder::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CRosterOrder)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CRosterOrder, CDialog)
	//{{AFX_MSG_MAP(CRosterOrder)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDHELP, OnHelp)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CRosterOrder message handlers

BOOL CRosterOrder::OnInitDialog() 
{
	CDialog::OnInitDialog();
//
//  Set up pointers to the controls
//	
  int nI, nJ, nK;
  
  pStaticWEEK_TITLE[0] = (CStatic *)GetDlgItem(ROSTERORDER_WEEK1_TITLE);
  pStaticWEEK_TITLE[1] = (CStatic *)GetDlgItem(ROSTERORDER_WEEK2_TITLE);
  pStaticWEEK_TITLE[2] = (CStatic *)GetDlgItem(ROSTERORDER_WEEK3_TITLE);
  pStaticWEEK_TITLE[3] = (CStatic *)GetDlgItem(ROSTERORDER_WEEK4_TITLE);
  pStaticWEEK_TITLE[4] = (CStatic *)GetDlgItem(ROSTERORDER_WEEK5_TITLE);
  pStaticWEEK_TITLE[5] = (CStatic *)GetDlgItem(ROSTERORDER_WEEK6_TITLE);
  pStaticWEEK_TITLE[6] = (CStatic *)GetDlgItem(ROSTERORDER_WEEK7_TITLE);
  pStaticWEEK_TITLE[7] = (CStatic *)GetDlgItem(ROSTERORDER_WEEK8_TITLE);
  pStaticWEEK_TITLE[8] = (CStatic *)GetDlgItem(ROSTERORDER_WEEK9_TITLE);
  pStaticWEEK_TITLE[9] = (CStatic *)GetDlgItem(ROSTERORDER_WEEK10_TITLE);

  pComboBoxPRIMARY = (CComboBox *)GetDlgItem(ROSTERORDER_PRIMARY);
  pComboBoxWEEK[0] = (CComboBox *)GetDlgItem(ROSTERORDER_WEEK1);
  pComboBoxWEEK[1] = (CComboBox *)GetDlgItem(ROSTERORDER_WEEK2);
  pComboBoxWEEK[2] = (CComboBox *)GetDlgItem(ROSTERORDER_WEEK3);
  pComboBoxWEEK[3] = (CComboBox *)GetDlgItem(ROSTERORDER_WEEK4);
  pComboBoxWEEK[4] = (CComboBox *)GetDlgItem(ROSTERORDER_WEEK5);
  pComboBoxWEEK[5] = (CComboBox *)GetDlgItem(ROSTERORDER_WEEK6);
  pComboBoxWEEK[6] = (CComboBox *)GetDlgItem(ROSTERORDER_WEEK7);
  pComboBoxWEEK[7] = (CComboBox *)GetDlgItem(ROSTERORDER_WEEK8);
  pComboBoxWEEK[8] = (CComboBox *)GetDlgItem(ROSTERORDER_WEEK9);
  pComboBoxWEEK[9] = (CComboBox *)GetDlgItem(ROSTERORDER_WEEK10);

//
//  Populate the dialog
//
  for(nI = 0; nI < m_pData->numDivisions; nI++)
  {
    nK = pComboBoxPRIMARY->AddString(m_pData->name[nI]);
    pComboBoxPRIMARY->SetItemData(nK, m_pData->DIVISIONSrecordIDs[nI]);
    for(nJ = 0; nJ < ROSTER_MAX_WEEKS; nJ++)
    {
      nK = pComboBoxWEEK[nJ]->AddString(m_pData->name[nI]);
      pComboBoxWEEK[nJ]->SetItemData(nK, m_pData->DIVISIONSrecordIDs[nI]);
    }
  }
//
//  Disable unused controls
//
  for(nI = 1; nI < ROSTER_MAX_WEEKS; nI++)
  {
    pStaticWEEK_TITLE[nI]->EnableWindow(m_pData->numDivisions > nI);
    pComboBoxWEEK[nI]->EnableWindow(m_pData->numDivisions > nI);
  }

  	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CRosterOrder::OnClose() 
{
	CDialog::OnClose();
}

void CRosterOrder::OnHelp() 
{
}

void CRosterOrder::OnCancel() 
{
	CDialog::OnCancel();
}

void CRosterOrder::OnOK() 
{
  int nI, nJ;

  nI = pComboBoxPRIMARY->GetCurSel();
  if(nI == CB_ERR)
  {
    return;
  }
  m_pData->DIVISIONSrecordIDs[0] = pComboBoxPRIMARY->GetItemData(nI);
  m_pData->numDivisions = 1;

  for(nI = 0; nI < ROSTER_MAX_WEEKS; nI++)
  {
    if(!pComboBoxWEEK[nI]->IsWindowEnabled())
    {
      m_pData->DIVISIONSrecordIDs[nI + 1] = NO_RECORD;
    }
    else
    {
      nJ = pComboBoxWEEK[nI]->GetCurSel();
      m_pData->DIVISIONSrecordIDs[nI + 1] = pComboBoxWEEK[nI]->GetItemData(nJ);
      m_pData->numDivisions++;
    }
  }
	
	CDialog::OnOK();
}

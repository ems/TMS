// DailyOpsRSO.cpp : implementation file
//

#include "stdafx.h"
extern "C"{
#include "TMSHeader.h"
#include "cistms.h"
}
#include <math.h>
#include "tms.h"
#include "DailyOpsHeader.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDailyOpsRSO dialog


CDailyOpsRSO::CDailyOpsRSO(CWnd* pParent /*=NULL*/, long *pNODESrecordID, long *pTime)
	: CDialog(CDailyOpsRSO::IDD, pParent)
{
  m_pNODESrecordID = pNODESrecordID;
  m_pTime = pTime;

	//{{AFX_DATA_INIT(CDailyOpsRSO)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CDailyOpsRSO::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDailyOpsRSO)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDailyOpsRSO, CDialog)
	//{{AFX_MSG_MAP(CDailyOpsRSO)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDHELP, OnHelp)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDailyOpsRSO message handlers

BOOL CDailyOpsRSO::OnInitDialog() 
{
	CDialog::OnInitDialog();

//
//  Set up pointers to the controls
//
  pComboBoxNEWNODELIST = (CComboBox *)GetDlgItem(DAILYOPSRSO_NEWNODELIST);
  pEditNEWTIME = (CEdit *)GetDlgItem(DAILYOPSRSO_NEWTIME);
//
//  Set up the list of nodes
//
  int nI, nJ;

  nI = SetUpNodeList(this->m_hWnd, DAILYOPSRSO_NEWNODELIST, FALSE);
  strcpy(tempString, "");
  for(nJ = 0; nJ < nI; nJ++)
  {
    if((long)pComboBoxNEWNODELIST->GetItemData(nJ) == *m_pNODESrecordID)
    {
      pComboBoxNEWNODELIST->SetCurSel(nJ);
      pComboBoxNEWNODELIST->GetLBText(nJ, tempString);
      break;
    }
  }

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CDailyOpsRSO::OnClose() 
{
  OnCancel();
}

void CDailyOpsRSO::OnCancel() 
{
	CDialog::OnCancel();
}

void CDailyOpsRSO::OnHelp() 
{
}

void CDailyOpsRSO::OnOK() 
{
  int nI;

  nI = pComboBoxNEWNODELIST->GetCurSel();

  if(nI < 0)
  {
    return;
  }

  *m_pNODESrecordID = (long)pComboBoxNEWNODELIST->GetItemData(nI);

  pEditNEWTIME->GetWindowText(tempString, TEMPSTRING_LENGTH);

  *m_pTime = cTime(tempString);

	CDialog::OnOK();
}

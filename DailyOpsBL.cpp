// DailyOpsBL.cpp : implementation file
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
// CDailyOpsBL dialog


CDailyOpsBL::CDailyOpsBL(CWnd* pParent, long* pLocationNODESrecordID)
	: CDialog(CDailyOpsBL::IDD, pParent)
{
  m_pLocationNODESrecordID = pLocationNODESrecordID;

	//{{AFX_DATA_INIT(CDailyOpsBL)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CDailyOpsBL::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDailyOpsBL)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDailyOpsBL, CDialog)
	//{{AFX_MSG_MAP(CDailyOpsBL)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDHELP, OnHelp)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDailyOpsBL message handlers

BOOL CDailyOpsBL::OnInitDialog() 
{
	CDialog::OnInitDialog();
//
//  Set up a pointer to the control
//
  pComboBoxLOCATION = (CComboBox *)GetDlgItem(DAILYOPSBL_LOCATION);	
//
//  Set up the garages
//
  int numGarages = SetUpNodeList(this->m_hWnd, DAILYOPSBL_LOCATION, TRUE);
  int selection = 0;

  if(*m_pLocationNODESrecordID != NO_RECORD)
  {
    int nI;
    int selection = 0;

    for(nI = 0; nI < numGarages; nI++)
    {
      if((long)pComboBoxLOCATION->GetItemData(nI) == *m_pLocationNODESrecordID)
      {
        selection = nI;
        break;
      }
    }
  }

  pComboBoxLOCATION->SetCurSel(selection);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CDailyOpsBL::OnClose() 
{
  OnCancel();
}

void CDailyOpsBL::OnCancel() 
{
	CDialog::OnCancel();
}

void CDailyOpsBL::OnHelp() 
{
}

void CDailyOpsBL::OnOK() 
{
  int nI = pComboBoxLOCATION->GetCurSel();

  if(nI == CB_ERR)
  {
    *m_pLocationNODESrecordID = NO_RECORD;
  }
  else
  {
    *m_pLocationNODESrecordID = pComboBoxLOCATION->GetItemData(nI);
  }
	
	CDialog::OnOK();
}

// DriversToConsider.cpp : implementation file
//

#include "stdafx.h"
extern "C"{
#include "TMSHeader.h"

BOOL ConsideringThisDriverType(long DRIVERTYPESrecordID)
{
  int rcode2;

  if(DRIVERTYPESrecordID == NO_RECORD)
  {
    return(TRUE);
  }

  DRIVERTYPESKey0.recordID = DRIVERTYPESrecordID;
  rcode2 = btrieve(B_GETEQUAL, TMS_DRIVERTYPES, &DRIVERTYPES, &DRIVERTYPESKey0, 0);
  if(rcode2 != 0)
  {
    return(FALSE);
  }

  return(driversToConsider & (1 << (DRIVERTYPES.number - 1)));
}
}  // EXTERN C

#include "tms.h"
#include "DailyOpsHeader.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDriversToConsider dialog


CDriversToConsider::CDriversToConsider(CWnd* pParent /*=NULL*/)
	: CDialog(CDriversToConsider::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDriversToConsider)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CDriversToConsider::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDriversToConsider)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDriversToConsider, CDialog)
	//{{AFX_MSG_MAP(CDriversToConsider)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDHELP, OnHelp)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDriversToConsider message handlers

BOOL CDriversToConsider::OnInitDialog() 
{
	CDialog::OnInitDialog();
//
//  Set up the pointer to the control
//
  pListBoxTYPES = (CListBox *)GetDlgItem(DRIVERSTOCONSIDER_TYPES);	
//
//  Go through the driver types and add them to the list
//
  int nI, nJ;
  int rcode2;

  rcode2 = btrieve(B_GETFIRST, TMS_DRIVERTYPES, &DRIVERTYPES, &DRIVERTYPESKey0, 0);
  while(rcode2 == 0)
  {
    strncpy(tempString, DRIVERTYPES.name, DRIVERTYPES_NAME_LENGTH);
    trim(tempString, DRIVERTYPES_NAME_LENGTH);
    nI = pListBoxTYPES->AddString(tempString);
    pListBoxTYPES->SetItemData(nI, DRIVERTYPES.number);
    rcode2 = btrieve(B_GETNEXT, TMS_DRIVERTYPES, &DRIVERTYPES, &DRIVERTYPESKey0, 0);
  }
//
//  Cycle through the list to show any currently selected driver types
//
  for(nI = 0; nI < pListBoxTYPES->GetCount(); nI++)
  {
    nJ = (int)pListBoxTYPES->GetItemData(nI);
    if(driversToConsider & (1 << (nJ - 1)))
    {
      pListBoxTYPES->SetSel(nI, TRUE);
    }
  }
//
//  All done
//
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CDriversToConsider::OnClose() 
{
  OnCancel();
}

void CDriversToConsider::OnCancel() 
{
	// TODO: Add extra cleanup here
	
	CDialog::OnCancel();
}

void CDriversToConsider::OnHelp() 
{
	// TODO: Add your control notification handler code here
	
}

void CDriversToConsider::OnOK() 
{
  int nI, nJ;

	driversToConsider = 0;

  for(nI = 0; nI < pListBoxTYPES->GetCount(); nI++)
  {
    if(pListBoxTYPES->GetSel(nI))
    {
      nJ = (int)pListBoxTYPES->GetItemData(nI);
      driversToConsider += (1 << (nJ - 1));
    }
  }

  m_bWorkrulesChanged = TRUE;
	
	CDialog::OnOK();
}

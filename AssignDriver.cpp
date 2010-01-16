// AssignDriver.cpp : implementation file
//

#include "stdafx.h"
extern "C"{
#include "TMSHeader.h"
}
#include "tms.h"
#include "AssignDriver.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CAssignDriver dialog


CAssignDriver::CAssignDriver(CWnd* pParent, long* pUpdateRecordID, ROSTERDef* pROSTER)
	: CDialog(CAssignDriver::IDD, pParent)
{
  m_pUpdateRecordID = pUpdateRecordID;
  m_pROSTER = pROSTER;
	//{{AFX_DATA_INIT(CAssignDriver)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CAssignDriver::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAssignDriver)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CAssignDriver, CDialog)
	//{{AFX_MSG_MAP(CAssignDriver)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDHELP, OnHelp)
	ON_LBN_DBLCLK(ASSIGNDRIVER_DRIVER, OnDblclkDriver)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAssignDriver message handlers

BOOL CAssignDriver::OnInitDialog() 
{
	CDialog::OnInitDialog();

  int  nI;
  int  rcode2;
//
//  Set up the pointer to the control
//
  pListBoxDRIVER = (CListBox *)GetDlgItem(ASSIGNDRIVER_DRIVER);
//
//  Build a list of drivers and add them to the listbox
//
  rcode2 = btrieve(B_GETFIRST, TMS_DRIVERS, &DRIVERS, &DRIVERSKey1, 1);
  while(rcode2 == 0)
  {
    if(ConsideringThisDriverType(DRIVERS.DRIVERTYPESrecordID))
    {
      strncpy(tempString, DRIVERS.badgeNumber, DRIVERS_BADGENUMBER_LENGTH);
      trim(tempString, DRIVERS_BADGENUMBER_LENGTH);
      strcat(tempString, "\t");
      strncpy(szarString, DRIVERS.lastName, DRIVERS_LASTNAME_LENGTH);
      trim(szarString, DRIVERS_LASTNAME_LENGTH);
      strcat(tempString, szarString);
      strcat(tempString, ", ");
      strncpy(szarString, DRIVERS.firstName, DRIVERS_FIRSTNAME_LENGTH);
      trim(szarString, DRIVERS_FIRSTNAME_LENGTH);
      strcat(tempString, szarString);
      strcat(tempString, " ");
      strncpy(szarString, DRIVERS.initials, DRIVERS_INITIALS_LENGTH);
      trim(szarString, DRIVERS_INITIALS_LENGTH);
      strcat(tempString, szarString);
      ROSTERKey2.DRIVERSrecordID = DRIVERS.recordID;
      rcode2 = btrieve(B_GETEQUAL, TMS_ROSTER, &ROSTER, &ROSTERKey2, 2);
      if(rcode2 == 0)
      {
        strcat(tempString, " *");
      }
      nI = pListBoxDRIVER->AddString(tempString);
      pListBoxDRIVER->SetItemData(nI, DRIVERS.recordID);
    }
    rcode2 = btrieve(B_GETNEXT, TMS_DRIVERS, &DRIVERS, &DRIVERSKey1, 1);
  }
//
//  There has to be at least one driver available
//
  if(pListBoxDRIVER->GetCount() == 0)
  {
    TMSError(m_hWnd, MB_ICONSTOP, ERROR_184, (HANDLE)NULL);
    OnCancel();
  }
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CAssignDriver::OnClose() 
{
  OnCancel();
}

void CAssignDriver::OnCancel() 
{
	CDialog::OnCancel();
}

void CAssignDriver::OnHelp() 
{
  ::WinHelp(m_hWnd, szarHelpFile, HELP_CONTEXT, Assigning_Drivers_to_a_Roster);
}

void CAssignDriver::OnDblclkDriver() 
{
	OnOK();
}

void CAssignDriver::OnOK() 
{
  long recordID;
  int  nI;
  int  rcode2;

  nI = pListBoxDRIVER->GetCurSel();
  if(nI == LB_ERR)
  {
    return;
  }
  recordID = pListBoxDRIVER->GetItemData(nI);
  if(m_pUpdateRecordID == NULL)
  {
    m_pROSTER->DRIVERSrecordID = recordID;
  }
  else
  {
    ROSTERKey0.recordID = *m_pUpdateRecordID;
    rcode2 = btrieve(B_GETEQUAL, TMS_ROSTER, &ROSTER, &ROSTERKey0, 0);
    if(rcode2 == 0)
    {
      ROSTER.DRIVERSrecordID = recordID;
      btrieve(B_UPDATE, TMS_ROSTER, &ROSTER, &ROSTERKey0, 0);
    }
  }
	
	CDialog::OnOK();
}

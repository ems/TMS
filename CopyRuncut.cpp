// CopyRuncut.cpp : implementation file
//

#include "stdafx.h"
extern "C" {
#include "TMSHeader.h"
}
#include "TMS.h"
#include "CopyRuncut.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CCopyRuncut dialog


CCopyRuncut::CCopyRuncut(CWnd* pParent /*=NULL*/)
	: CDialog(CCopyRuncut::IDD, pParent)
{
	//{{AFX_DATA_INIT(CCopyRuncut)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CCopyRuncut::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CCopyRuncut)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CCopyRuncut, CDialog)
	//{{AFX_MSG_MAP(CCopyRuncut)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDHELP, OnHelp)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCopyRuncut message handlers

BOOL CCopyRuncut::OnInitDialog() 
{
	CDialog::OnInitDialog();

//
//  Set up pointers to the controls
//
  pStaticDESTINATION = (CStatic *)GetDlgItem(COPYRUNCUT_DESTINATION);
  pComboBoxSERVICE = (CComboBox *)GetDlgItem(COPYRUNCUT_SERVICE);
  pComboBoxDIVISION = (CComboBox *)GetDlgItem(COPYRUNCUT_DIVISION);
//
//  Set up the service combo box
//
  int numServices = SetUpServiceList(m_hWnd, COPYRUNCUT_SERVICE, m_ServiceRecordID);

  if(numServices == 0)
  {
    OnCancel();
    return TRUE;
  }
//
//  Set up the division combo box
//
  int numDivisions = SetUpDivisionList(m_hWnd, COPYRUNCUT_DIVISION, m_DivisionRecordID);

  if(numDivisions == 0)
  {
    OnCancel();
    return TRUE;
  }
//
//  Display the destination string
//
  CString s;
  char szDivisionName[DIVISIONS_NAME_LENGTH + 1];
  char szServiceName[SERVICES_NAME_LENGTH + 1];

  pStaticDESTINATION->GetWindowText(s);

  SERVICESKey0.recordID = m_ServiceRecordID;
  btrieve(B_GETEQUAL, TMS_SERVICES, &SERVICES, &SERVICESKey0, 0);
  strncpy(szServiceName, SERVICES.name, SERVICES_NAME_LENGTH);
  trim(szServiceName, SERVICES_NAME_LENGTH);

  DIVISIONSKey0.recordID = m_DivisionRecordID;
  btrieve(B_GETEQUAL, TMS_DIVISIONS, &DIVISIONS, &DIVISIONSKey0, 0);
  strncpy(szDivisionName, DIVISIONS.name, DIVISIONS_NAME_LENGTH);
  trim(szDivisionName, DIVISIONS_NAME_LENGTH);

  sprintf(tempString, s, szServiceName, szDivisionName);
  pStaticDESTINATION->SetWindowText(tempString);
//
//  All done
//
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CCopyRuncut::OnClose() 
{
  OnCancel();
}

void CCopyRuncut::OnCancel() 
{
	CDialog::OnCancel();
}

void CCopyRuncut::OnHelp() 
{
}

void CCopyRuncut::OnOK() 
{
  RUNSDef RUNSREC;
  CString s;
  long originDIVISIONSrecordID;
  long originSERVICESrecordID;
  int  nI;
  int  rcode2;
//
//  Make sure he's not copying on top of himself
//
//  Get the service
//
  nI = (int)pComboBoxSERVICE->GetCurSel();
  originSERVICESrecordID = pComboBoxSERVICE->GetItemData(nI);
//
//  Get the division
//
  nI = (int)pComboBoxDIVISION->GetCurSel();
  originDIVISIONSrecordID = pComboBoxDIVISION->GetItemData(nI);

  if(originSERVICESrecordID == m_ServiceRecordID && originDIVISIONSrecordID == m_DivisionRecordID)
  {
    TMSError(m_hWnd, MB_ICONSTOP, ERROR_355, (HANDLE)NULL);
    return;
  }
//
//  Are there any existing runcut records here (in ser/div m_ServiceRecordID / m_DivisionRecordID)?
//
  RUNSKey1.DIVISIONSrecordID = m_DivisionRecordID;
  RUNSKey1.SERVICESrecordID = m_ServiceRecordID;
  RUNSKey1.runNumber = NO_RECORD;
  RUNSKey1.pieceNumber = NO_RECORD;
  rcode2 = btrieve(B_GETGREATER, TMS_RUNS, &RUNS, &RUNSKey1, 1);
  if(rcode2 == 0 &&
        RUNS.SERVICESrecordID == m_ServiceRecordID &&
        RUNS.DIVISIONSrecordID == m_DivisionRecordID)
  {
    MessageBeep(MB_ICONQUESTION);
    s.LoadString(ERROR_356);
    if(MessageBox(s, TMS, MB_ICONQUESTION | MB_OK) != IDOK)
    {
      return;
    }
  }
//
//  The critter went and done it...
//
  HCURSOR saveCursor = SetCursor(hCursorWait);
//
//  Delete the existing runs for this division
//
  RUNSKey1.DIVISIONSrecordID = m_DivisionRecordID;
  RUNSKey1.SERVICESrecordID = m_ServiceRecordID;
  RUNSKey1.runNumber = NO_RECORD;
  RUNSKey1.pieceNumber = NO_RECORD;
  rcode2 = btrieve(B_GETGREATER, TMS_RUNS, &RUNS, &RUNSKey1, 1);
  while(rcode2 == 0 &&
        RUNS.SERVICESrecordID == m_ServiceRecordID &&
        RUNS.DIVISIONSrecordID == m_DivisionRecordID)
  {
    btrieve(B_DELETE, TMS_RUNS, &RUNS, &RUNSKey1, 1);
    rcode2 = btrieve(B_GETNEXT, TMS_RUNS, &RUNS, &RUNSKey1, 1);
  }
//
//  Set the insertion point
//
  rcode2 = btrieve(B_GETLAST, TMS_RUNS, &RUNS, &RUNSKey0, 0);
  long insertRecordID = AssignRecID(rcode2, RUNS.recordID);
//
//  Cycle through the "origin" service and division
//
  RUNSKey1.DIVISIONSrecordID = originDIVISIONSrecordID;
  RUNSKey1.SERVICESrecordID = originSERVICESrecordID;
  RUNSKey1.runNumber = NO_RECORD;
  RUNSKey1.pieceNumber = NO_RECORD;
  rcode2 = btrieve(B_GETGREATER, TMS_RUNS, &RUNS, &RUNSKey1, 1);
  while(rcode2 == 0 &&
        RUNS.SERVICESrecordID == originSERVICESrecordID &&
        RUNS.DIVISIONSrecordID == originDIVISIONSrecordID)
  {
    btrieve(B_GETPOSITION, TMS_RUNS, &RUNSREC, &RUNSKey1, 1);
    RUNS.recordID = insertRecordID++;
    RUNS.SERVICESrecordID = m_ServiceRecordID;
    RUNS.DIVISIONSrecordID = m_DivisionRecordID;
    btrieve(B_INSERT, TMS_RUNS, &RUNS, &RUNSKey1, 1);
    btrieve(B_GETDIRECT, TMS_RUNS, &RUNSREC, &RUNSKey1, 1);
    rcode2 = btrieve(B_GETNEXT, TMS_RUNS, &RUNS, &RUNSKey1, 1);
  }
  SetCursor(saveCursor);
//
//  Let him know it's done
//
  MessageBeep(MB_ICONINFORMATION);
  s.LoadString(TEXT_153);
  MessageBox(tempString, TMS, MB_ICONINFORMATION);

	CDialog::OnOK();
}

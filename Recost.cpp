// Recost.cpp : implementation file
//

#include "stdafx.h"
extern "C"{
#include "TMSHeader.h"
#include "cistms.h"
}
#include "tms.h"
#include "DailyOpsHeader.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CRecost dialog


CRecost::CRecost(CWnd* pParent /*=NULL*/, OPENWORKDef* pOpenWork, long PertainsToDate)
	: CDialog(CRecost::IDD, pParent)
{
  m_pOpenWork = pOpenWork;
  m_PertainsToDate = PertainsToDate;
	//{{AFX_DATA_INIT(CRecost)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CRecost::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CRecost)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CRecost, CDialog)
	//{{AFX_MSG_MAP(CRecost)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDHELP, OnHelp)
	ON_BN_CLICKED(RECOST_RUNTYPE, OnRuntype)
	ON_BN_CLICKED(RECOST_VALUE, OnValue)
	ON_BN_CLICKED(RECOST_AUTOMATIC, OnAutomatic)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CRecost message handlers

BOOL CRecost::OnInitDialog() 
{
	CDialog::OnInitDialog();
//
//  Set up pointers to the controls
//
  pButtonAUTOMATIC = (CButton *)GetDlgItem(RECOST_AUTOMATIC);
  pButtonRUNTYPE = (CButton *)GetDlgItem(RECOST_RUNTYPE);
  pComboBoxRUNTYPELIST = (CComboBox *)GetDlgItem(RECOST_RUNTYPELIST);
  pButtonVALUE = (CButton *)GetDlgItem(RECOST_VALUE);
  pEditVALUEDATA = (CEdit *)GetDlgItem(RECOST_VALUEDATA);
//
//  Set the title
//
  RUNSKey0.recordID = m_pOpenWork->RUNSrecordID;
  btrieve(B_GETEQUAL, TMS_RUNS, &RUNS, &RUNSKey0, 0);
  sprintf(tempString, "Recost run %ld", RUNS.runNumber);
  SetWindowText(tempString);
//
//  Figure out the number of pieces in this run
//
  int nType;
  int nSlot;
  int numPieces;
//
//  Possibility 1: Valid runtype came in
//
  if(m_pOpenWork->cutAsRuntype >= 0)
  {
    nType = LOWORD(m_pOpenWork->cutAsRuntype);
    nSlot = HIWORD(m_pOpenWork->cutAsRuntype);
    numPieces = RUNTYPE[nType][nSlot].numPieces;
  }
//
//  Possibility 2: Invalid (or fake) runtype
//
  else
  {
    numPieces = 1;
  }
//
//  Build the list of runtypes
//
  int nI, nJ, nK;

  for(nI = 0; nI < NUMRUNTYPES; nI++)
  {
    for(nJ = 0; nJ < NUMRUNTYPESLOTS; nJ++)
    {
      if(RUNTYPE[nI][nJ].flags & RTFLAGS_INUSE && RUNTYPE[nI][nJ].numPieces == numPieces)
      {
        nK = pComboBoxRUNTYPELIST->AddString(RUNTYPE[nI][nJ].localName);
        pComboBoxRUNTYPELIST->SetItemData(nK, MAKELONG(nI, nJ));
      }
    }
  }
//
//  Make "Automatic" the default selection unless the
//  incoming cutAsRuntype is less than 0.  If that's the
//  case, then automatic costing isn't an option
//
  if(m_pOpenWork->cutAsRuntype < 0)
  {
    pButtonAUTOMATIC->EnableWindow(FALSE);
    pButtonRUNTYPE->SetCheck(TRUE);
    pComboBoxRUNTYPELIST->SetCurSel(0);
  }
  else
  {
    pButtonAUTOMATIC->SetCheck(TRUE);
    pComboBoxRUNTYPELIST->EnableWindow(FALSE);
  }
  pEditVALUEDATA->EnableWindow(FALSE);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CRecost::OnClose() 
{
	CDialog::OnCancel();
}

void CRecost::OnCancel() 
{
	CDialog::OnCancel();
}

void CRecost::OnHelp() 
{
}


void CRecost::OnAutomatic() 
{
  pComboBoxRUNTYPELIST->SetCurSel(-1);
  pComboBoxRUNTYPELIST->EnableWindow(FALSE);
  pEditVALUEDATA->SetWindowText("");
  pEditVALUEDATA->EnableWindow(FALSE);
}

void CRecost::OnRuntype() 
{
  pEditVALUEDATA->SetWindowText("");
  pEditVALUEDATA->EnableWindow(FALSE);
  pComboBoxRUNTYPELIST->EnableWindow(TRUE);
  pComboBoxRUNTYPELIST->SetCurSel(0);
}

void CRecost::OnValue() 
{
  pComboBoxRUNTYPELIST->SetCurSel(-1);
  pComboBoxRUNTYPELIST->EnableWindow(FALSE);
  pEditVALUEDATA->EnableWindow(TRUE);
}

void CRecost::OnOK() 
{
  long DAILYOPSrecordID;
  long recordID;
  long UNRECOSTrecordID;
  int  rcode2;

  CTime SystemTime;
    
  SystemTime = CTime::GetCurrentTime();
//
//  Delete the previous recosting
//
  DAILYOPSKey1.recordTypeFlag = DAILYOPS_FLAG_OPENWORK;
  DAILYOPSKey1.recordFlags = DAILYOPS_FLAG_RECOSTRUN;
  DAILYOPSKey1.pertainsToDate = m_PertainsToDate;
  DAILYOPSKey1.pertainsToTime = NO_TIME;
  rcode2 = btrieve(B_GETGREATEROREQUAL, TMS_DAILYOPS, &DAILYOPS, &DAILYOPSKey1, 1);
  while(rcode2 == 0 &&
        DAILYOPS.recordTypeFlag == DAILYOPS_FLAG_RECOSTRUN &&
        DAILYOPS.pertainsToDate == m_PertainsToDate)
  {
    if(DAILYOPS.DOPS.OpenWork.RUNSrecordID == m_pOpenWork->RUNSrecordID)
    {
      if(DAILYOPS.DAILYOPSrecordID == NO_RECORD)  // not unrecosted prior to this
      {
        DAILYOPSrecordID = DAILYOPS.recordID;
        rcode2 = btrieve(B_GETLAST, TMS_DAILYOPS, &DAILYOPS, &DAILYOPSKey0, 0);
        recordID = AssignRecID(rcode2, DAILYOPS.recordID);
        memset(&DAILYOPS, 0x00, sizeof(DAILYOPSDef));
        DAILYOPS.recordID = recordID;
        DAILYOPS.entryDateAndTime = SystemTime.GetTime();
        DAILYOPS.pertainsToDate = m_PertainsToDate;
        DAILYOPS.pertainsToTime = NO_TIME;
        DAILYOPS.userID = m_UserID;
        DAILYOPS.recordTypeFlag = DAILYOPS_FLAG_OPENWORK;
        DAILYOPS.recordFlags = DAILYOPS_FLAG_UNRECOSTRUN;
        DAILYOPS.DAILYOPSrecordID = NO_RECORD;
        DAILYOPS.DRIVERSrecordID = NO_RECORD;
        rcode2 = btrieve(B_INSERT, TMS_DAILYOPS, &DAILYOPS, &DAILYOPSKey0, 0);
        if(rcode2 == 0)
        {
          m_LastDAILYOPSRecordID = DAILYOPS.recordID;
        }
        UNRECOSTrecordID = DAILYOPS.recordID;
        DAILYOPSKey0.recordID = DAILYOPSrecordID;
        rcode2 = btrieve(B_GETEQUAL, TMS_DAILYOPS, &DAILYOPS, &DAILYOPSKey0, 0);
        rcode2 = btrieve(B_GETPOSITION, TMS_DAILYOPS, &DAILYOPS, &DAILYOPSKey0, 0);
        rcode2 = btrieve(B_GETDIRECT, TMS_DAILYOPS, &DAILYOPS, &DAILYOPSKey1, 1);
        DAILYOPS.DAILYOPSrecordID = UNRECOSTrecordID;
        rcode2 = btrieve(B_UPDATE, TMS_DAILYOPS, &DAILYOPS, &DAILYOPSKey1, 1);
      }
    }
    rcode2 = btrieve(B_GETNEXT, TMS_DAILYOPS, &DAILYOPS, &DAILYOPSKey1, 1);
  }
//
//  Process the selection
//
  PROPOSEDRUNDef PROPOSEDRUN;
  COSTDef        COST;
  long           tempLong;
  int            nI;
  
//
//  Automatic
//
  if(pButtonAUTOMATIC->GetCheck())
  {
    CDailyOps::SetupRun(m_pOpenWork->RUNSrecordID, FALSE, &PROPOSEDRUN);
    RunCoster(&PROPOSEDRUN, m_pOpenWork->cutAsRuntype, &COST);
    m_pOpenWork->pay = COST.TOTAL.payTime;
    if(m_pOpenWork->propertyFlags & OPENWORK_PROPERTYFLAG_RUNWASRECOSTED)
    {
      m_pOpenWork->propertyFlags -= OPENWORK_PROPERTYFLAG_RUNWASRECOSTED;
    }
  }
//
//  Runtype/Value
//
//  Runtype
//
  else
  {
    if(pButtonRUNTYPE->GetCheck())
    {
      nI = pComboBoxRUNTYPELIST->GetCurSel();
      if(nI == CB_ERR)
      {
        MessageBeep(MB_ICONSTOP);
        return;
      }
      PROPOSEDRUN.numPieces = 1;
      PROPOSEDRUN.piece[0].fromTime = m_pOpenWork->onTime;
      PROPOSEDRUN.piece[0].fromNODESrecordID = m_pOpenWork->onNODESrecordID;
      PROPOSEDRUN.piece[0].fromTRIPSrecordID = m_pOpenWork->TRIPSrecordID;
      PROPOSEDRUN.piece[0].toTime = m_pOpenWork->offTime;
      PROPOSEDRUN.piece[0].toNODESrecordID = m_pOpenWork->offNODESrecordID;
      PROPOSEDRUN.piece[0].toTRIPSrecordID = m_pOpenWork->TRIPSrecordID;
      tempLong = pComboBoxRUNTYPELIST->GetItemData(nI);
      RunCoster(&PROPOSEDRUN, tempLong, &COST);
      m_pOpenWork->pay = COST.TOTAL.payTime;
      if(m_pOpenWork->pay == 0)
      {
        MessageBeep(MB_ICONINFORMATION);
        sprintf(tempString, "This run will pay out as 0:00 because...\n\n%s", runcosterReason);
        MessageBox(tempString, TMS, MB_ICONINFORMATION | MB_OK);
      }
      m_pOpenWork->propertyFlags |= OPENWORK_PROPERTYFLAG_RUNWASRECOSTED;
    }
//
//  Value
//
    else
    {
      pEditVALUEDATA->GetWindowText(tempString, TEMPSTRING_LENGTH);
      tempLong = thhmm(tempString);
      m_pOpenWork->pay = tempLong;
      m_pOpenWork->propertyFlags |= OPENWORK_PROPERTYFLAG_RUNWASRECOSTED;
    }
//
//  Record the recost
//
    rcode2 = btrieve(B_GETLAST, TMS_DAILYOPS, &DAILYOPS, &DAILYOPSKey0, 0);
    recordID = AssignRecID(rcode2, DAILYOPS.recordID);
    memset(&DAILYOPS, 0x00, sizeof(DAILYOPSDef));
//
//  Build a new record
//
    DAILYOPS.recordID = recordID;
    DAILYOPS.entryDateAndTime = SystemTime.GetTime();
    DAILYOPS.pertainsToDate = m_PertainsToDate;
    DAILYOPS.pertainsToTime = NO_TIME;
    DAILYOPS.userID = m_UserID;
    DAILYOPS.recordTypeFlag = DAILYOPS_FLAG_OPENWORK;
    DAILYOPS.recordFlags = DAILYOPS_FLAG_RECOSTRUN;
    DAILYOPS.DAILYOPSrecordID = NO_RECORD;
    DAILYOPS.DOPS.OpenWork.RUNSrecordID = m_pOpenWork->RUNSrecordID;
    DAILYOPS.DOPS.OpenWork.splitStartTime = m_pOpenWork->onTime;
    DAILYOPS.DOPS.OpenWork.splitStartNODESrecordID = m_pOpenWork->onNODESrecordID;
    DAILYOPS.DOPS.OpenWork.splitEndTime = m_pOpenWork->offTime;
    DAILYOPS.DOPS.OpenWork.splitEndNODESrecordID = m_pOpenWork->offNODESrecordID;
    DAILYOPS.DOPS.OpenWork.payTime = m_pOpenWork->pay;
    rcode2 = btrieve(B_INSERT, TMS_DAILYOPS, &DAILYOPS, &DAILYOPSKey0, 0);
    if(rcode2 == 0)
    {
      m_LastDAILYOPSRecordID = DAILYOPS.recordID;
    }
  }

	CDialog::OnOK();
}

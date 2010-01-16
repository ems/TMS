// DisplayRoster.cpp : implementation file
//

#include "stdafx.h"
#include "stdafx.h"
extern "C"
{
#include "TMSHeader.h"
}
#include "tms.h"
#include "DisplayRoster.h"
#include "AssignDriver.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDisplayRoster dialog


CDisplayRoster::CDisplayRoster(CWnd* pParent /*=NULL*/, long* pRecordID)
	: CDialog(CDisplayRoster::IDD, pParent)
{
  m_pRecordID = pRecordID;

	//{{AFX_DATA_INIT(CDisplayRoster)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CDisplayRoster::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDisplayRoster)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDisplayRoster, CDialog)
	//{{AFX_MSG_MAP(CDisplayRoster)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDDELETE, OnDelete)
	ON_BN_CLICKED(IDHELP, OnHelp)
	ON_BN_CLICKED(DISPLAYROSTER_ALL, OnAll)
	ON_BN_CLICKED(IDASSIGN, OnAssign)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDisplayRoster message handlers

BOOL CDisplayRoster::OnInitDialog() 
{
	CDialog::OnInitDialog();

//
//  Set up pointers to the controls
//
  pButtonALL = (CButton *)GetDlgItem(DISPLAYROSTER_ALL);
  pButtonWEEK1 = (CButton *)GetDlgItem(DISPLAYROSTER_WEEK1);
  pButtonWEEK2 = (CButton *)GetDlgItem(DISPLAYROSTER_WEEK2);
  pButtonWEEK3 = (CButton *)GetDlgItem(DISPLAYROSTER_WEEK3);
  pButtonWEEK4 = (CButton *)GetDlgItem(DISPLAYROSTER_WEEK4);
  pButtonWEEK5 = (CButton *)GetDlgItem(DISPLAYROSTER_WEEK5);
  pButtonWEEK6 = (CButton *)GetDlgItem(DISPLAYROSTER_WEEK6);
  pButtonWEEK7 = (CButton *)GetDlgItem(DISPLAYROSTER_WEEK7);
  pButtonWEEK8 = (CButton *)GetDlgItem(DISPLAYROSTER_WEEK8);
  pButtonWEEK9 = (CButton *)GetDlgItem(DISPLAYROSTER_WEEK9);
  pButtonWEEK10 = (CButton *)GetDlgItem(DISPLAYROSTER_WEEK10);
  pListCtrlLIST = (CListCtrl *)GetDlgItem(DISPLAYROSTER_LIST);
  pButtonIDASSIGN = (CButton *)GetDlgItem(IDASSIGN);

  pButtonWEEKS[0] = pButtonWEEK1;
  pButtonWEEKS[1] = pButtonWEEK2;
  pButtonWEEKS[2] = pButtonWEEK3;
  pButtonWEEKS[3] = pButtonWEEK4;
  pButtonWEEKS[4] = pButtonWEEK5;
  pButtonWEEKS[5] = pButtonWEEK6;
  pButtonWEEKS[6] = pButtonWEEK7;
  pButtonWEEKS[7] = pButtonWEEK8;
  pButtonWEEKS[8] = pButtonWEEK9;
  pButtonWEEKS[9] = pButtonWEEK10;
//
//  Set up the list control styles
//
  DWORD dwExStyles = pListCtrlLIST->GetExtendedStyle();
  pListCtrlLIST->SetExtendedStyle(dwExStyles | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_INFOTIP);
//
//  Set up the column headers
//
  LVCOLUMN LVC;

  LVC.mask = LVCF_FMT | LVCF_TEXT | LVCF_WIDTH;
  LVC.cx = 1;
  LVC.fmt = LVCFMT_CENTER;
  LVC.pszText = "";
  pListCtrlLIST->InsertColumn(0, &LVC);
  
  LVC.mask = LVCF_FMT | LVCF_TEXT | LVCF_WIDTH;
  LVC.cx = 80;
  LVC.fmt = LVCFMT_CENTER;
  LVC.pszText = "Monday";
  pListCtrlLIST->InsertColumn(1, &LVC);

  LVC.cx = 80;
  LVC.fmt = LVCFMT_CENTER;
  LVC.pszText = "Tuesday";
  pListCtrlLIST->InsertColumn(2, &LVC);

  LVC.cx = 80;
  LVC.fmt = LVCFMT_CENTER;
  LVC.pszText = "Wednesday";
  pListCtrlLIST->InsertColumn(3, &LVC);

  LVC.cx = 80;
  LVC.fmt = LVCFMT_CENTER;
  LVC.pszText = "Thursday";
  pListCtrlLIST->InsertColumn(4, &LVC);

  LVC.cx = 80;
  LVC.fmt = LVCFMT_CENTER;
  LVC.pszText = "Friday";
  pListCtrlLIST->InsertColumn(5, &LVC);

  LVC.cx = 80;
  LVC.fmt = LVCFMT_CENTER;
  LVC.pszText = "Saturday";
  pListCtrlLIST->InsertColumn(6, &LVC);

  LVC.cx = 80;
  LVC.fmt = LVCFMT_CENTER;
  LVC.pszText = "Sunday";
  pListCtrlLIST->InsertColumn(7, &LVC);
//
//  Get the roster record
//
  ROSTERKey0.recordID = *m_pRecordID;
  btrieve(B_GETEQUAL, TMS_ROSTER, &m_ROSTER, &ROSTERKey0, 0);
//
//  Is it assigned?
//
  CString s;

  if(m_ROSTER.DRIVERSrecordID == NO_RECORD)
  {
    s = "Not assigned";
  }
  else
  {
    s = BuildOperatorString(m_ROSTER.DRIVERSrecordID);
  }
//
//  Set the window title
//
  GetWindowText(m_sFmt);

  sprintf(tempString, m_sFmt, m_ROSTER.rosterNumber, s);

  SetWindowText(tempString);
//
//  If it's already assigned, change the Assign button to Deassign
//
  if(m_ROSTER.DRIVERSrecordID != NO_RECORD)
  {
    pButtonIDASSIGN->SetWindowText("Deassign");
  }
//
//  Put up the current roster
//
  DisplayCurrent(TRUE);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CDisplayRoster::OnClose() 
{
  OnCancel();
}

void CDisplayRoster::OnCancel() 
{
	CDialog::OnCancel();
}


void CDisplayRoster::OnHelp() 
{
}

void CDisplayRoster::OnAll() 
{
  BOOL bSet = pButtonALL->GetCheck();
  int  nI;

  for(nI = 0; nI < ROSTER_MAX_WEEKS; nI++)
  {
    if(pButtonWEEKS[nI]->IsWindowEnabled())
    {
      pButtonWEEKS[nI]->SetCheck(bSet);
    }
  }
}

void CDisplayRoster::OnAssign() 
{
  CString s = "";

  if(m_ROSTER.DRIVERSrecordID == NO_RECORD)
  {
    CAssignDriver dlg(NULL, NULL, &m_ROSTER);

    if(dlg.DoModal())
    {
      s = BuildOperatorString(m_ROSTER.DRIVERSrecordID);
      pButtonIDASSIGN->SetWindowText("Deassign");
   }
  }
  else
  {
    m_ROSTER.DRIVERSrecordID = NO_RECORD;
    s = "Not assigned";
    pButtonIDASSIGN->SetWindowText("Assign...");
  }
  if(s != "")
  {
    sprintf(tempString, m_sFmt, m_ROSTER.rosterNumber, s);
    SetWindowText(tempString);
  }
}

void CDisplayRoster::OnDelete() 
{
  int nI, nJ;

  for(nI = 0; nI < ROSTER_MAX_WEEKS; nI++)
  {
    if(pButtonWEEKS[nI]->GetCheck())
    {
      for(nJ = 0; nJ < ROSTER_MAX_DAYS; nJ++)
      {
        m_ROSTER.WEEK[nI].RUNSrecordIDs[nJ] = NO_RECORD;
      }
      m_ROSTER.WEEK[nI].flags = 0;
      pButtonWEEKS[nI]->SetCheck(FALSE);
    }
  }

  DisplayCurrent(FALSE);
}

void CDisplayRoster::DisplayCurrent(BOOL bFirst)
{
  LVITEM LVI;
  BOOL   bNoDays;
  long   runNumber;
  int    nI, nJ;
  int    rcode2;

  for(nI = 0; nI < ROSTER_MAX_WEEKS; nI++)
  {
    bNoDays = TRUE;
    for(nJ = 0; nJ < ROSTER_MAX_DAYS; nJ++)
    {
      if(m_ROSTER.WEEK[nI].RUNSrecordIDs[nJ] > 0)
      {
        bNoDays = FALSE;
        break;
      }
    }

    if(bFirst)
    {
      LVI.mask = LVIF_TEXT;
      LVI.iItem = nI;
      LVI.iSubItem = 0;
      LVI.pszText = "";
      pListCtrlLIST->InsertItem(&LVI);
    }

    if(bNoDays)
    {
      pButtonWEEKS[nI]->EnableWindow(FALSE);
    }

    for(nJ = 0; nJ < ROSTER_MAX_DAYS; nJ++)
    {
      if(bNoDays)
      {
        strcpy(tempString, "-");
      }
      else
      {
        if(m_ROSTER.WEEK[nI].RUNSrecordIDs[nJ] == NO_RECORD)
        {
          strcpy(tempString, "Off");
        }
        else
        {
          if(m_ROSTER.WEEK[nI].flags & (1 << nJ))
          {
            CREWONLYKey0.recordID = m_ROSTER.WEEK[nI].RUNSrecordIDs[nJ];
            rcode2 = btrieve(B_GETEQUAL, TMS_CREWONLY, &CREWONLY, &CREWONLYKey0, 0);
            runNumber = (rcode2 == 0 ? CREWONLY.runNumber : -2);
          }
          else
          {
            RUNSKey0.recordID = m_ROSTER.WEEK[nI].RUNSrecordIDs[nJ];
            rcode2 = btrieve(B_GETEQUAL, TMS_RUNS, &RUNS, &RUNSKey0, 0);
            runNumber = (rcode2 == 0 ? RUNS.runNumber : -2);
          }
          if(runNumber == -2)
          {
            sprintf(tempString, "Not found");
          }
          else
          {
            sprintf(tempString, "%ld", runNumber);
          }
        }
      }
      LVI.mask = LVIF_TEXT;
      LVI.iItem = nI;
      LVI.iSubItem = nJ + 1;
      LVI.pszText = tempString;
      LVI.iItem = pListCtrlLIST->SetItem(&LVI);
    }
  }
}

void CDisplayRoster::OnOK() 
{
  ROSTERKey0.recordID = *m_pRecordID;

  btrieve(B_GETEQUAL, TMS_ROSTER, &ROSTER, &ROSTERKey0, 0);
  btrieve(B_UPDATE, TMS_ROSTER, &m_ROSTER, &ROSTERKey0, 0);
	
	CDialog::OnOK();
}


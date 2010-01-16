// SuspensionPending.cpp : implementation file
//
#include "stdafx.h"
extern "C"{
#include "TMSHeader.h"
}  // EXTERN C

#include "TMS.h"

#include "AddDialogs.h"
#include "SuspensionPending.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSuspensionPending dialog


CSuspensionPending::CSuspensionPending(CWnd* pParent /*=NULL*/, BOOL bSuspensionPending, long numRecordIDs, long* pRecordIDs)
	: CDialog(CSuspensionPending::IDD, pParent)
{

  m_numRecordIDs = numRecordIDs;
  m_pRecordIDs = pRecordIDs;
  m_bSuspensionPending = bSuspensionPending;

	//{{AFX_DATA_INIT(CSuspensionPending)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CSuspensionPending::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSuspensionPending)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSuspensionPending, CDialog)
	//{{AFX_MSG_MAP(CSuspensionPending)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDHELP, OnHelp)
	ON_BN_CLICKED(IDPRINT, OnPrint)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSuspensionPending message handlers

BOOL CSuspensionPending::OnInitDialog() 
{
	CDialog::OnInitDialog();

//
//  Set up the handle to the control
//	
  pListCtrlLIST = (CListCtrl *)GetDlgItem(SUSPENSIONPENDING_LIST);
  DWORD dwExStyles = pListCtrlLIST->GetExtendedStyle();
  pListCtrlLIST->SetExtendedStyle(dwExStyles | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_INFOTIP);
//
//  Set up the list controls
//
  LVCOLUMN LVC;
  LVC.mask = LVCF_FMT | LVCF_TEXT | LVCF_WIDTH;

  LVC.cx = 185;
  LVC.fmt = LVCFMT_LEFT;
  LVC.pszText = "Employee";
  pListCtrlLIST->InsertColumn(0, &LVC);

  LVC.cx = 120;
  LVC.fmt = LVCFMT_CENTER;
  LVC.pszText = "Entry Date";
  pListCtrlLIST->InsertColumn(1, &LVC);

  LVITEM LVI;
  BOOL bAbsenceFlags = DAILYOPS_FLAG_ABSENCETODISCIPLINE | DAILYOPS_FLAG_ABSENCEREGISTER;
  char szDriver[DRIVERS_LASTNAME_LENGTH + 2 + DRIVERS_FIRSTNAME_LENGTH + 1];
  long year;
  long month;
  long day;
  int  rcode2;
  int  iItem;
  int  nI;
//
//  If this isn't a suspension pending, change the title of the dialog
//
  if(!m_bSuspensionPending)
  {
    SetWindowText("Discipline Pending");
//
//  Cycle through
//
    for(nI = 0; nI < m_numRecordIDs; nI++)
    {
//
//  Locate the records in the btrieve file
//
      DAILYOPSKey0.recordID = m_pRecordIDs[nI];
      btrieve(B_GETEQUAL, TMS_DAILYOPS, &DAILYOPS, &DAILYOPSKey0, 0);
//
//  Driver
//
      DRIVERSKey0.recordID = DAILYOPS.DRIVERSrecordID;
      rcode2 = btrieve(B_GETEQUAL, TMS_DRIVERS, &DRIVERS, &DRIVERSKey0, 0);
      strncpy(tempString, DRIVERS.lastName, DRIVERS_LASTNAME_LENGTH);
      trim(tempString, DRIVERS_LASTNAME_LENGTH);
      strcpy(szDriver, tempString);
      strcat(szDriver, ", ");
      strncpy(tempString, DRIVERS.firstName, DRIVERS_FIRSTNAME_LENGTH);
      trim(tempString, DRIVERS_FIRSTNAME_LENGTH);
      strcat(szDriver, tempString);
      iItem = pListCtrlLIST->GetItemCount();
      LVI.mask = LVIF_TEXT;
      LVI.iItem = iItem;
      LVI.iSubItem = 0;
      LVI.pszText = szDriver;
      LVI.iItem = pListCtrlLIST->InsertItem(&LVI);
//
//  Date
//
      GetYMD(DAILYOPS.pertainsToDate, &year, &month, &day);
      sprintf(tempString, "%02ld-%s-%04ld", day, szMonth[month - 1], year);
      LVI.mask = LVIF_TEXT;
      LVI.iItem = iItem;
      LVI.iSubItem = 1;
      LVI.pszText = tempString;
      LVI.iItem = pListCtrlLIST->SetItem(&LVI);
    }
  }
//
//  Suspension pending
//
  else
  {
//
//  Cycle through
//
    for(nI = 0; nI < m_numRecordIDs; nI++)
    {
//
//  Locate the records in the btrieve file
//
      DISCIPLINEKey0.recordID = m_pRecordIDs[nI];
      btrieve(B_GETEQUAL, TMS_DISCIPLINE, &DISCIPLINE, &DISCIPLINEKey0, 0);
//
//  Driver
//
      DRIVERSKey0.recordID = DISCIPLINE.DRIVERSrecordID;
      rcode2 = btrieve(B_GETEQUAL, TMS_DRIVERS, &DRIVERS, &DRIVERSKey0, 0);
      strncpy(tempString, DRIVERS.lastName, DRIVERS_LASTNAME_LENGTH);
      trim(tempString, DRIVERS_LASTNAME_LENGTH);
      strcpy(szDriver, tempString);
      strcat(szDriver, ", ");
      strncpy(tempString, DRIVERS.firstName, DRIVERS_FIRSTNAME_LENGTH);
      trim(tempString, DRIVERS_FIRSTNAME_LENGTH);
      strcat(szDriver, tempString);
      iItem = pListCtrlLIST->GetItemCount();
      LVI.mask = LVIF_TEXT;
      LVI.iItem = iItem;
      LVI.iSubItem = 0;
      LVI.pszText = szDriver;
      LVI.iItem = pListCtrlLIST->InsertItem(&LVI);
//
//  Date
//
      struct tm *entryDateAndTime = localtime(&DISCIPLINE.entryDateAndTime);

      sprintf(tempString, "%02ld-%s-%04ld",
            entryDateAndTime->tm_mday, szMonth[entryDateAndTime->tm_mon], entryDateAndTime->tm_year + 1900);
      LVI.mask = LVIF_TEXT;
      LVI.iItem = iItem;
      LVI.iSubItem = 1;
      LVI.pszText = tempString;
      LVI.iItem = pListCtrlLIST->SetItem(&LVI);
    }
  }

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CSuspensionPending::OnClose() 
{
  OnOK();
}

void CSuspensionPending::OnHelp() 
{
	// TODO: Add your control notification handler code here
	
}

void CSuspensionPending::OnPrint() 
{
  PrintWindowToDC(this->m_hWnd, "Customer Comments Supervisor Print Screen");
}

void CSuspensionPending::OnOK() 
{
	CDialog::OnOK();
}

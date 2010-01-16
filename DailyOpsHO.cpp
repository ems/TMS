// DailyOpsHO.cpp : implementation file
//

#include "stdafx.h"
extern "C"{
#include "TMSHeader.h"
}  // EXTERN C

#include "tms.h"
#include "DailyOpsHeader.h"
#include "AddHoliday.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDailyOpsHO dialog


CDailyOpsHO::CDailyOpsHO(CWnd* pParent /*=NULL*/, long* pDate)
	: CDialog(CDailyOpsHO::IDD, pParent)
{
  m_pDate = pDate;

	//{{AFX_DATA_INIT(CDailyOpsHO)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CDailyOpsHO::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDailyOpsHO)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDailyOpsHO, CDialog)
	//{{AFX_MSG_MAP(CDailyOpsHO)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDINSERT, OnInsert)
	ON_BN_CLICKED(IDUPDATE, OnUpdate)
	ON_BN_CLICKED(IDREMOVE, OnRemove)
	ON_BN_CLICKED(IDHELP, OnHelp)
	ON_NOTIFY(NM_CLICK, DAILYOPSHO_LIST, OnClickList)
	ON_NOTIFY(NM_DBLCLK, DAILYOPSHO_LIST, OnDblclkList)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDailyOpsHO message handlers

BOOL CDailyOpsHO::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
//
//  Set up the pointers to the controls
//	
  pListCtrlLIST = (CListCtrl *)GetDlgItem(DAILYOPSHO_LIST);
  pButtonIDREMOVE = (CButton *)GetDlgItem(IDREMOVE);
  pButtonIDUPDATE = (CButton *)GetDlgItem(IDUPDATE);
//
//  Set up the list controls
//
  DWORD dwExStyles = pListCtrlLIST->GetExtendedStyle();
  pListCtrlLIST->SetExtendedStyle(dwExStyles | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_INFOTIP);

  LVCOLUMN LVC;
  LVC.mask = LVCF_FMT | LVCF_TEXT | LVCF_WIDTH;

  LVC.cx = 200;
  LVC.fmt = LVCFMT_LEFT;
  LVC.pszText = "Date";
  pListCtrlLIST->InsertColumn(0, &LVC);

  LVC.cx = 160;
  LVC.fmt = LVCFMT_LEFT;
  LVC.pszText = "Description";
  pListCtrlLIST->InsertColumn(1, &LVC);

  LVC.cx = 130;
  LVC.pszText = "Service";
  pListCtrlLIST->InsertColumn(2, &LVC);
//
//  Display anything that's there
//
  Display();

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CDailyOpsHO::OnClickList(NMHDR* pNMHDR, LRESULT* pResult) 
{
  pButtonIDREMOVE->EnableWindow(TRUE);
  pButtonIDUPDATE->EnableWindow(TRUE);
	
	*pResult = 0;
}

void CDailyOpsHO::OnDblclkList(NMHDR* pNMHDR, LRESULT* pResult) 
{
  OnUpdate();

	*pResult = 0;
}

void CDailyOpsHO::Display()
{
  LVITEM  LVI;
  BOOL   bFound;
  int    iItem = 0;
  int    rcode2;
  long   year, month, day;

  HCURSOR hSaveCursor = SetCursor(hCursorWait);

  pListCtrlLIST->DeleteAllItems();

  DAILYOPSKey1.recordTypeFlag = (char)DAILYOPS_FLAG_DATE;
  DAILYOPSKey1.pertainsToDate = *m_pDate - 10000;
  DAILYOPSKey1.pertainsToTime = NO_TIME;
  DAILYOPSKey1.recordFlags = 0;
  rcode2 = btrieve(B_GETGREATER, TMS_DAILYOPS, &DAILYOPS, &DAILYOPSKey1, 1);
  while(rcode2 == 0 &&
        (DAILYOPS.recordTypeFlag & DAILYOPS_FLAG_DATE))
  {
    bFound = FALSE;
    if(DAILYOPS.recordFlags & DAILYOPS_FLAG_DATESET)
    {
      if(!ANegatedRecord(DAILYOPS.recordID, 1))
      {
//
//  Pertains to date
//
        GetYMD(DAILYOPS.pertainsToDate, &year, &month, &day);

        CTime   t(year, month, day, 0, 0, 0);
        CString x = t.Format(_T("%A, %B %d, %Y"));

        LVI.mask = LVIF_TEXT | LVIF_PARAM;
        LVI.iItem = iItem;
        LVI.iSubItem = 0;
        sprintf(tempString, "%s", x);
        LVI.pszText = tempString;
        LVI.lParam = DAILYOPS.recordID;
        LVI.iItem = pListCtrlLIST->InsertItem(&LVI);
//
//  Description
//
        strncpy(tempString, DAILYOPS.DOPS.Date.name, DAILYOPS_DATENAME_LENGTH);
        trim(tempString, DAILYOPS_DATENAME_LENGTH);
        LVI.mask = LVIF_TEXT;
        LVI.iItem = iItem;
        LVI.iSubItem = 1;
        LVI.pszText = tempString;
        LVI.iItem = pListCtrlLIST->SetItem(&LVI);
//
//  Service
//
        SERVICESKey0.recordID = DAILYOPS.DOPS.Date.SERVICESrecordID;
        rcode2 = btrieve(B_GETEQUAL, TMS_SERVICES, &SERVICES, &SERVICESKey0, 0);
        if(rcode2 == 0)
        {
          strncpy(tempString, SERVICES.name, SERVICES_NAME_LENGTH);
          trim(tempString, SERVICES_NAME_LENGTH);
        }
        else
        {
          strcpy(tempString, "No Service");
        }
        LVI.mask = LVIF_TEXT;
        LVI.iItem = iItem;
        LVI.iSubItem = 2;
        LVI.pszText = tempString;
        LVI.iItem = pListCtrlLIST->SetItem(&LVI);
//
//  Set up the next row
//
        iItem++;
      }
    }
    rcode2 = btrieve(B_GETNEXT, TMS_DAILYOPS, &DAILYOPS, &DAILYOPSKey1, 1);
  }
//
//  Disable Remove and Update 'till he picks something
//
  pButtonIDREMOVE->EnableWindow(FALSE);
  pButtonIDUPDATE->EnableWindow(FALSE);
}


void CDailyOpsHO::OnClose() 
{
	CDialog::OnClose();
}

void CDailyOpsHO::OnInsert() 
{
  long DAILYOPSrecordID = NO_RECORD;

  CAddHoliday dlg(this, &DAILYOPSrecordID);

  if(dlg.DoModal() == IDOK)
  {
    Display();
  }
}

void CDailyOpsHO::OnUpdate() 
{
  int  nI = pListCtrlLIST->GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);
  
  if(nI < 0)
  {
    return;
  }

  long  DAILYOPSrecordID = pListCtrlLIST->GetItemData(nI);

  CAddHoliday dlg(this, &DAILYOPSrecordID);

  if(dlg.DoModal() == IDOK)
  {
    Display();
  }
}

void CDailyOpsHO::OnRemove() 
{
  int  nI = pListCtrlLIST->GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);
  
  if(nI < 0)
  {
    return;
  }

  long  DAILYOPSrecordID = pListCtrlLIST->GetItemData(nI);
  
  CTime time = CTime::GetCurrentTime();
  int rcode2;
//
//  "Unregister" this previous record
//
  rcode2 = btrieve(B_GETLAST, TMS_DAILYOPS, &DAILYOPS, &DAILYOPSKey0, 0);
  DAILYOPS.recordID = AssignRecID(rcode2, DAILYOPS.recordID);
  DAILYOPS.entryDateAndTime = time.GetTime();
  DAILYOPS.pertainsToDate = NO_RECORD;
  DAILYOPS.pertainsToTime = NO_TIME;
  DAILYOPS.recordTypeFlag = (char)DAILYOPS_FLAG_DATE;
  DAILYOPS.recordFlags = DAILYOPS_FLAG_DATECLEAR;
  DAILYOPS.userID = m_UserID;
  DAILYOPS.DRIVERSrecordID = NO_RECORD;
  DAILYOPS.DAILYOPSrecordID = DAILYOPSrecordID;
  memset(DAILYOPS.DOPS.associatedData, 0x00, DAILYOPS_ASSOCIATEDDATA_LENGTH);
  rcode2 = btrieve(B_INSERT, TMS_DAILYOPS, &DAILYOPS, &DAILYOPSKey0, 0);
  if(rcode2 == 0)
  {
    m_LastDAILYOPSRecordID = DAILYOPS.recordID;
  }
//
//  Kill the item from the list
//
  pListCtrlLIST->DeleteItem(nI);
//
//  Disable Remove and Update 'till he picks something
//
  pButtonIDREMOVE->EnableWindow(FALSE);
  pButtonIDUPDATE->EnableWindow(FALSE);
}

void CDailyOpsHO::OnHelp() 
{
	// TODO: Add your control notification handler code here
	
}

void CDailyOpsHO::OnOK() 
{
	CDialog::OnOK();
}


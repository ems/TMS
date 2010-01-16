// PayrollLayout.cpp : implementation file
//


#include "stdafx.h"

extern "C"{
#include "TMSHeader.h"
#include "cistms.h"
#include "TextPayrollLayout.h"

BOOL IsNumber(char *pStr);

int sort_OutputOrder(const void *a, const void *b)
{
  PayrollLayoutDef *pa, *pb;

  pa = (PayrollLayoutDef *)a;
  pb = (PayrollLayoutDef *)b;

  return(pa->position < pb->position ? -1 : pa->position > pb->position ? 1 : 0);
}
}

#include "tms.h"
#include "DailyOpsHeader.h"
#include "AddHoliday.h"
#include "RosterWeek.h"
#include "sdigrid.h"
#include "NumericEdit.h"
#include "Ridership.h"
#include "Fares.h"
#include "ColorBtn.h"
#include "PegboardColors.h"
#include "TextPayrollLayout.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CPayrollLayout dialog


CPayrollLayout::CPayrollLayout(CWnd* pParent /*=NULL*/)
	: CDialog(CPayrollLayout::IDD, pParent)
{
  m_bInitInProgress = TRUE;
	//{{AFX_DATA_INIT(CPayrollLayout)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CPayrollLayout::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPayrollLayout)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CPayrollLayout, CDialog)
	//{{AFX_MSG_MAP(CPayrollLayout)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDHELP, OnHelp)
	ON_NOTIFY(NM_CLICK, PAYROLLLAYOUT_LIST, OnClickList)
	ON_BN_CLICKED(PAYROLLLAYOUT_UNUSED, OnUnused)
	ON_BN_CLICKED(PAYROLLLAYOUT_USED, OnUsed)
	ON_EN_CHANGE(PAYROLLLAYOUT_POSITION, OnChangePosition)
	ON_EN_KILLFOCUS(PAYROLLLAYOUT_POSITION, OnKillfocusPosition)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPayrollLayout message handlers

BOOL CPayrollLayout::OnInitDialog() 
{
	CDialog::OnInitDialog();

//
//  Set up pointers to the controls
//
  pListCtrlLIST = (CListCtrl *)GetDlgItem(PAYROLLLAYOUT_LIST);
  pListCtrlOUTPUTORDER = (CListCtrl *)GetDlgItem(PAYROLLLAYOUT_OUTPUTORDER);
  pButtonUSED = (CButton *)GetDlgItem(PAYROLLLAYOUT_USED);
  pButtonUNUSED = (CButton *)GetDlgItem(PAYROLLLAYOUT_UNUSED);
  pEditPOSITION = (CEdit *)GetDlgItem(PAYROLLLAYOUT_POSITION);
//
//  Set the style of the list controls
//
  DWORD dwExStyles = pListCtrlLIST->GetExtendedStyle();
  pListCtrlLIST->SetExtendedStyle(dwExStyles | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_INFOTIP);
  
  dwExStyles = pListCtrlOUTPUTORDER->GetExtendedStyle();
  pListCtrlOUTPUTORDER->SetExtendedStyle(dwExStyles | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_INFOTIP);
//
//  And the columns
//
//  Field listing
//
  LVCOLUMN LVC;

  LVC.mask = LVCF_FMT | LVCF_TEXT | LVCF_WIDTH;
//
//  Field name
//
  LVC.cx = 132;
  LVC.fmt = LVCFMT_LEFT;
  LVC.pszText = "Field Name";
  pListCtrlLIST->InsertColumn(0, &LVC);
//
//  Position
//
  LVC.cx = 46;
  LVC.fmt = LVCFMT_CENTER;
  LVC.pszText = "Pos";
  pListCtrlLIST->InsertColumn(1, &LVC);
//
//  Output order listing
//
//  Field name
//
  LVC.cx = 46;
  LVC.fmt = LVCFMT_CENTER;
  LVC.pszText = "Pos";
  pListCtrlOUTPUTORDER->InsertColumn(0, &LVC);
//
//  Position
//
  LVC.cx = 132;
  LVC.fmt = LVCFMT_LEFT;
  LVC.pszText = "Field Name";
  pListCtrlOUTPUTORDER->InsertColumn(1, &LVC);
//
//  Populate the field listing list control
//
  if(m_numPayrollLayoutEntries == 0)
  {
    MessageBeep(MB_ICONSTOP);
    MessageBox("No layout entries have been defined\n\nPlease contact Schedule Masters, Inc.", TMS, MB_ICONSTOP | MB_OK);
    return TRUE;
  }

  LVITEM LVI;
  int nI;

  for(nI = 0; nI < m_numPayrollLayoutEntries; nI++)
  {
//
//  Field name
//
    LVI.mask = LVIF_TEXT | LVIF_PARAM;
    LVI.lParam = m_PayrollLayout[nI].index;
    LVI.iItem = nI;
    LVI.iSubItem = 0;
    LVI.pszText = m_PayrollLayout[nI].szText;
    LVI.iItem = pListCtrlLIST->InsertItem(&LVI);
//
//  Position
//
    LVI.mask = LVIF_TEXT;
    LVI.iItem = nI;
    LVI.iSubItem = 1;
    if(m_PayrollLayout[nI].position <= 0)
    {
      strcpy(tempString, "-");
    }
    else
    {
      sprintf(tempString, "%ld", m_PayrollLayout[nI].position);
    }
    LVI.pszText = tempString;
    LVI.iItem = pListCtrlLIST->SetItem(&LVI);
  }
//
//  Highlight the first row to save on error checking later on
//
  m_SelectedRow = 0;
  m_Index = m_PayrollLayout[0].index;
  pListCtrlLIST->SetItemState(m_Index, LVIS_SELECTED, LVIS_SELECTED);
  if(m_PayrollLayout[m_Index].position == NO_RECORD)
  {
    pButtonUNUSED->SetCheck(TRUE);
    pButtonUSED->SetCheck(FALSE);
  }
  else
  {
    pButtonUNUSED->SetCheck(FALSE);
    pButtonUSED->SetCheck(TRUE);
    sprintf(tempString, "%ld", m_PayrollLayout[m_Index].position);
    pEditPOSITION->SetWindowText(tempString);
  }
  RefreshOutputOrder();
  m_bInitInProgress = FALSE;

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CPayrollLayout::OnClose() 
{
  OnCancel();
}

void CPayrollLayout::OnCancel() 
{
	CDialog::OnCancel();
}

void CPayrollLayout::OnHelp() 
{
}

void CPayrollLayout::OnClickList(NMHDR* pNMHDR, LRESULT* pResult) 
{
  LVITEM LVI;
  int nI = pListCtrlLIST->GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);

  if(nI < 0)
  {
    return;
  }

  LVI.mask = LVIF_PARAM;
  LVI.iItem = nI;
  LVI.iSubItem = 0;
  pListCtrlLIST->GetItem(&LVI);
  m_Index = LVI.lParam;
  m_SelectedRow = nI;

  if(m_PayrollLayout[m_SelectedRow].position == NO_RECORD)
  {
    pButtonUNUSED->SetCheck(TRUE);
    pButtonUSED->SetCheck(FALSE);
    pEditPOSITION->SetWindowText("-");
  }
  else
  {
    pButtonUNUSED->SetCheck(FALSE);
    pButtonUSED->SetCheck(TRUE);
    sprintf(tempString, "%ld", m_PayrollLayout[m_SelectedRow].position);
    pEditPOSITION->SetWindowText(tempString);
  }

	*pResult = 0;
}

void CPayrollLayout::OnChangePosition() 
{
  if(m_bInitInProgress)
  {
    return;
  }

  LVITEM LVI;

  pButtonUNUSED->SetCheck(FALSE);
  pButtonUSED->SetCheck(TRUE);

  pEditPOSITION->GetWindowText(tempString, TEMPSTRING_LENGTH);
  if(IsNumber(tempString))
  {
    m_PayrollLayout[m_SelectedRow].position = atol(tempString);
    strcpy(szarString, tempString);
  }
  else
  {
    m_PayrollLayout[m_SelectedRow].position = NO_RECORD;
    strcpy(szarString, "-");
  }
  LVI.mask = LVIF_TEXT;
  LVI.iItem = m_SelectedRow;
  LVI.iSubItem = 1;
  LVI.pszText = szarString;
  LVI.iItem = pListCtrlLIST->SetItem(&LVI);

  RefreshOutputOrder();
}


void CPayrollLayout::OnKillfocusPosition() 
{
  pEditPOSITION->GetWindowText(tempString, TEMPSTRING_LENGTH);
  m_PayrollLayout[m_SelectedRow].position = atol(tempString);

  RefreshOutputOrder();
}

void CPayrollLayout::OnUnused() 
{
  pEditPOSITION->SetWindowText("");

  RefreshOutputOrder();
}

void CPayrollLayout::OnUsed() 
{
}

void CPayrollLayout::RefreshOutputOrder()
{
  PayrollLayoutDef PayrollLayout[MAXPAYROLLLAYOUTENTRIES];

  memcpy(&PayrollLayout, &m_PayrollLayout, m_numPayrollLayoutEntries * sizeof(PayrollLayoutDef));

  qsort((void *)PayrollLayout, m_numPayrollLayoutEntries, sizeof(PayrollLayoutDef), sort_OutputOrder);

  pListCtrlOUTPUTORDER->DeleteAllItems();

  LVITEM LVI;
  int    nI, nJ;

  for(nJ = 0, nI = 0; nI < m_numPayrollLayoutEntries; nI++)
  {
    if(PayrollLayout[nI].position <= 0)
    {
      continue;
    }
//
//  Position
//
    LVI.mask = LVIF_TEXT;
    LVI.iItem = nJ;
    LVI.iSubItem = 0;
    sprintf(tempString, "%ld", PayrollLayout[nI].position);
    LVI.pszText = tempString;
    LVI.iItem = pListCtrlOUTPUTORDER->InsertItem(&LVI);
//
//  Field name
//
    LVI.mask = LVIF_TEXT;
    LVI.iItem = nJ;
    LVI.iSubItem = 1;
    LVI.pszText = PayrollLayout[nI].szText;
    LVI.iItem = pListCtrlOUTPUTORDER->SetItem(&LVI);

    nJ++;
  }
}

void CPayrollLayout::OnOK() 
{
  int nI;

  CDatabase db;
  CString   s;
  CString   sRegKey = "Software\\ODBC\\ODBC.INI\\TMS Text Access Database"; 
  HKEY  hKey;
  long  result;

  result = RegOpenKeyEx(HKEY_CURRENT_USER, sRegKey, 0, KEY_SET_VALUE, &hKey);

  CString sADB = szDatabaseRoot;
  
  sADB += "\\Common\\";
  sADB += TEXT_DATABASE_FILENAME;

  DWORD type = REG_SZ;

  strcpy(tempString, sADB);
  RegSetValueEx(hKey, "DBQ", 0, type, (LPBYTE)tempString, strlen(tempString));
  db.OpenEx("DSN=TMS Text Access Database", CDatabase::noOdbcDialog );

  CTextPayrollLayout TextPayrollLayout(&db);
  
  TextPayrollLayout.Open(CRecordset::dynaset, NULL, CRecordset::none);
//
//  Save the payroll layout
//
  if(TextPayrollLayout.GetRecordCount() > 0)
  {
    for(nI = 0; nI < m_numPayrollLayoutEntries; nI++)
    {
      if(TextPayrollLayout.CanUpdate())
      {
        TextPayrollLayout.Move(m_PayrollLayout[nI].index + 1, SQL_FETCH_ABSOLUTE);
        TextPayrollLayout.Edit();
        TextPayrollLayout.m_Position = m_PayrollLayout[nI].position;
        TextPayrollLayout.Update();
      }
    }
  }
  TextPayrollLayout.Close( );
  db.Close();

	CDialog::OnOK();
}

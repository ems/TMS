//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
// FindCustomer.cpp : implementation file
//

#include "stdafx.h"
extern "C"
{
#include "TMSHeader.h"
}
#include "cc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CFindCustomer dialog

static BOOL       bSortForward[FINDCUSTOMER_NUMCOLUMNS];

int CALLBACK FCListViewCompareProc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
  FCSORTDef *p1 = (FCSORTDef *)lParam1;
  FCSORTDef *p2 = (FCSORTDef *)lParam2;
  int  iResult = 0;

  if(p1 && p2)
  {
    switch(lParamSort)
    {
      case 0:  // Surname
        iResult = bSortForward[lParamSort] ?
              lstrcmpi(p1->lastName, p2->lastName) :
              lstrcmpi(p2->lastName, p1->lastName);
        break;

      case 1:  // First name
        iResult = bSortForward[lParamSort] ?
              lstrcmpi(p1->firstName, p2->firstName) :
              lstrcmpi(p2->firstName, p1->firstName);
        break;

      case 2:  // Address
        iResult = bSortForward[lParamSort] ?
              lstrcmpi(p1->streetAddress, p2->streetAddress) :
              lstrcmpi(p2->streetAddress, p1->streetAddress);
        break;

      case 3:  // City
        iResult = bSortForward[lParamSort] ?
              lstrcmpi(p1->city, p2->city) :
              lstrcmpi(p2->city, p1->city);
        break;

      case 4:  // State
        iResult = bSortForward[lParamSort] ?
              lstrcmpi(p1->state, p2->state) :
              lstrcmpi(p2->state, p1->state);
        break;

      case 5:  // ZIP
        iResult = bSortForward[lParamSort] ?
              lstrcmpi(p1->ZIP, p2->ZIP) :
              lstrcmpi(p2->ZIP, p1->ZIP);
        break;

      case 6:  // Day phone
        iResult = bSortForward[lParamSort] ?
              lstrcmpi(p1->dayPhone, p2->dayPhone) :
              lstrcmpi(p2->dayPhone, p1->dayPhone);
        break;

      case 7:  // Night phone
        iResult = bSortForward[lParamSort] ?
              lstrcmpi(p1->nightPhone, p2->nightPhone) :
              lstrcmpi(p2->nightPhone, p1->nightPhone);
        break;

      case 8:  // Fax
        iResult = bSortForward[lParamSort] ?
              lstrcmpi(p1->faxPhone, p2->faxPhone) :
              lstrcmpi(p2->faxPhone, p1->faxPhone);
        break;

      case 9:  // Email address
        iResult = bSortForward[lParamSort] ?
              lstrcmpi(p1->emailAddress, p2->emailAddress) :
              lstrcmpi(p2->emailAddress, p1->emailAddress);
        break;

    }
  }

  return(iResult);
}




CFindCustomer::CFindCustomer(CWnd* pParent /*=NULL*/, long* pRecordID)
	: CDialog(CFindCustomer::IDD, pParent)
{
  m_pRecordID = pRecordID;
	//{{AFX_DATA_INIT(CFindCustomer)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CFindCustomer::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CFindCustomer)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CFindCustomer, CDialog)
	//{{AFX_MSG_MAP(CFindCustomer)
	ON_BN_CLICKED(IDGO, OnGo)
	ON_BN_CLICKED(IDHELP, OnHelp)
	ON_NOTIFY(LVN_COLUMNCLICK, FINDCUSTOMER_LIST, OnColumnclickList)
	ON_WM_CLOSE()
	ON_NOTIFY(NM_DBLCLK, FINDCUSTOMER_LIST, OnDblclkList)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFindCustomer message handlers

BOOL CFindCustomer::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
  pEditTEXT = (CEdit *)GetDlgItem(FINDCUSTOMER_TEXT);
  pListCtrlLIST = (CListCtrl *)GetDlgItem(FINDCUSTOMER_LIST);	

  DWORD dwExStyles = pListCtrlLIST->GetExtendedStyle();
  pListCtrlLIST->SetExtendedStyle(dwExStyles | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_INFOTIP);
//
//  Set up the list control
//
  LVCOLUMN LVC;
  
  LVC.mask = LVCF_FMT | LVCF_TEXT | LVCF_WIDTH;
  LVC.fmt = LVCFMT_LEFT;
  LVC.cx = 60;
  LVC.pszText = "Surname";
  pListCtrlLIST->InsertColumn(0, &LVC);
  
  LVC.cx = 60;
  LVC.pszText = "First name";
  pListCtrlLIST->InsertColumn(1, &LVC);

  LVC.cx = 125;
  LVC.pszText = "Address";
  pListCtrlLIST->InsertColumn(2, &LVC);

  LVC.cx = 60;
  LVC.pszText = "City";
  pListCtrlLIST->InsertColumn(3, &LVC);

  LVC.cx = 40;
  GetPrivateProfileString((LPSTR)userName, (LPSTR)"StateDisplay", "State",
        tempString, TEMPSTRING_LENGTH, szDatabaseFileName);
  LVC.pszText = tempString;
  pListCtrlLIST->InsertColumn(4, &LVC);

  LVC.cx = 40;
  GetPrivateProfileString((LPSTR)userName, (LPSTR)"ZIPDisplay", "ZIP",
        tempString, TEMPSTRING_LENGTH, szDatabaseFileName);
  LVC.pszText = tempString;
  pListCtrlLIST->InsertColumn(5, &LVC);

  LVC.cx = 75;
  LVC.pszText = "Day";
  pListCtrlLIST->InsertColumn(6, &LVC);

  LVC.cx = 75;
  LVC.pszText = "Night";
  pListCtrlLIST->InsertColumn(7, &LVC);

  LVC.cx = 75;
  LVC.pszText = "Fax";
  pListCtrlLIST->InsertColumn(8, &LVC);

  LVC.cx = 100;
  LVC.pszText = "Email";
  pListCtrlLIST->InsertColumn(9, &LVC);

  for(int nI = 0; nI < FINDCUSTOMER_NUMCOLUMNS; nI++)
  {
    bSortForward[nI] = TRUE;
  }
//
//  Allocate space for the structure
//
  m_maxFound = 250;
  m_pFCSORT = (FCSORTDef *)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(FCSORTDef) * m_maxFound); 
  if(m_pFCSORT == NULL)
  {
    AllocationError(__FILE__, __LINE__, FALSE);
  }

  m_bAllowSort = m_pFCSORT != NULL;

  return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CFindCustomer::OnGo() 
{
  CString s;
  CString match;
  FCSORTDef FCTemp;
  LVITEM LVI;
  int rcode2;
  int numFound;
//
//  Clear out the list control
//
  pListCtrlLIST->DeleteAllItems();
//
//  Get the find string and upper-case it
//
  pEditTEXT->GetWindowText(s);
  s.MakeUpper();
//
//  Search
//
  numFound = 0;
  rcode2 = btrieve(B_GETFIRST, TMS_CUSTOMERS, &CUSTOMERS, &CUSTOMERSKey1, 1);
  while(rcode2 == 0)
  {
    FCTemp.recordID = CUSTOMERS.recordID;
    strncpy(FCTemp.lastName, CUSTOMERS.lastName, CUSTOMERS_LASTNAME_LENGTH);
    trim(FCTemp.lastName, CUSTOMERS_LASTNAME_LENGTH);
    match = FCTemp.lastName;
    strncpy(FCTemp.firstName, CUSTOMERS.firstName, CUSTOMERS_FIRSTNAME_LENGTH);
    trim(FCTemp.firstName, CUSTOMERS_FIRSTNAME_LENGTH);
    match += " ";
    match += FCTemp.firstName;
    strncpy(FCTemp.streetAddress, CUSTOMERS.streetAddress, CUSTOMERS_STREETADDRESS_LENGTH);
    trim(FCTemp.streetAddress, CUSTOMERS_STREETADDRESS_LENGTH);
    match += " ";
    match += FCTemp.streetAddress;
    strncpy(FCTemp.city, CUSTOMERS.city, CUSTOMERS_CITY_LENGTH);
    trim(FCTemp.city, CUSTOMERS_CITY_LENGTH);
    match += " ";
    match += FCTemp.city;
    strncpy(FCTemp.state, CUSTOMERS.state, CUSTOMERS_STATE_LENGTH);
    trim(FCTemp.state, CUSTOMERS_STATE_LENGTH);
    match += " ";
    match += FCTemp.state;
    strncpy(FCTemp.ZIP, CUSTOMERS.ZIP, CUSTOMERS_ZIP_LENGTH);
    trim(FCTemp.ZIP, CUSTOMERS_ZIP_LENGTH);
    match += " ";
    match += FCTemp.ZIP;
    strcpy(FCTemp.dayPhone, PhoneString(CUSTOMERS.dayTelephoneArea, CUSTOMERS.dayTelephoneNumber));
    match += " ";
    match += FCTemp.dayPhone;
    strcpy(FCTemp.nightPhone, PhoneString(CUSTOMERS.nightTelephoneArea, CUSTOMERS.nightTelephoneNumber));
    match += " ";
    match += FCTemp.nightPhone;
    strcpy(FCTemp.faxPhone, PhoneString(CUSTOMERS.faxTelephoneArea, CUSTOMERS.faxTelephoneNumber));
    match += " ";
    match += FCTemp.faxPhone;
    strncpy(FCTemp.emailAddress, CUSTOMERS.emailAddress, CUSTOMERS_EMAILADDRESS_LENGTH);
    trim(FCTemp.emailAddress, CUSTOMERS_EMAILADDRESS_LENGTH);
    match += " ";
    match += FCTemp.emailAddress;
    
    match.MakeUpper();
    if(match.Find(s) >= 0)
    {
      if(numFound >= m_maxFound)
      {
        m_maxFound += 100;
        m_pFCSORT = (FCSORTDef *)HeapReAlloc(GetProcessHeap(),
              HEAP_ZERO_MEMORY, m_pFCSORT, sizeof(FCSORTDef) * m_maxFound); 
        if(m_pFCSORT == NULL )
        {
          AllocationError(__FILE__, __LINE__, TRUE);
        }
        m_bAllowSort = m_pFCSORT != NULL;
      }
      LVI.mask = LVIF_TEXT;
      LVI.iItem = numFound;
      LVI.iSubItem = 0;
      LVI.pszText = FCTemp.lastName;
      if(m_bAllowSort)
      {
        m_pFCSORT[numFound] = FCTemp;
        LVI.mask |= LVIF_PARAM;
        LVI.lParam = (LPARAM)&m_pFCSORT[numFound];
      }
      LVI.iItem = pListCtrlLIST->InsertItem(&LVI);

      LVI.mask = LVIF_TEXT;
      LVI.iSubItem = 1;
      LVI.pszText = FCTemp.firstName;
      pListCtrlLIST->SetItem(&LVI);

      LVI.mask = LVIF_TEXT;
      LVI.iSubItem = 2;
      LVI.pszText = FCTemp.streetAddress;
      pListCtrlLIST->SetItem(&LVI);

      LVI.mask = LVIF_TEXT;
      LVI.iSubItem = 3;
      LVI.pszText = FCTemp.city;
      pListCtrlLIST->SetItem(&LVI);

      LVI.mask = LVIF_TEXT;
      LVI.iSubItem = 4;
      LVI.pszText = FCTemp.state;
      pListCtrlLIST->SetItem(&LVI);

      LVI.mask = LVIF_TEXT;
      LVI.iSubItem = 5;
      LVI.pszText = FCTemp.ZIP;
      pListCtrlLIST->SetItem(&LVI);

      LVI.mask = LVIF_TEXT;
      LVI.iSubItem = 6;
      LVI.pszText = FCTemp.dayPhone;
      pListCtrlLIST->SetItem(&LVI);

      LVI.mask = LVIF_TEXT;
      LVI.iSubItem = 7;
      LVI.pszText = FCTemp.nightPhone;
      pListCtrlLIST->SetItem(&LVI);

      LVI.mask = LVIF_TEXT;
      LVI.iSubItem = 8;
      LVI.pszText = FCTemp.faxPhone;
      pListCtrlLIST->SetItem(&LVI);

      LVI.mask = LVIF_TEXT;
      LVI.iSubItem = 9;
      LVI.pszText = FCTemp.emailAddress;
      pListCtrlLIST->SetItem(&LVI);

      numFound++;
    }
    rcode2 = btrieve(B_GETNEXT, TMS_CUSTOMERS, &CUSTOMERS, &CUSTOMERSKey1, 1);
  }
}

void CFindCustomer::OnHelp() 
{
  ::WinHelp(m_hWnd, szarHelpFile, HELP_CONTEXT, The_Find_a_Customer_Dialog);
}

void CFindCustomer::OnCancel() 
{
	TMSHeapFree(m_pFCSORT);
  
  *m_pRecordID = NO_RECORD;

  CDialog::OnCancel();
}

void CFindCustomer::OnOK() 
{
	int nI = pListCtrlLIST->GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);

  if(nI >= 0)
  {
    int nJ = pListCtrlLIST->GetItemData(nI);
    FCSORTDef *pFC = (FCSORTDef *)nJ;

    *m_pRecordID = pFC->recordID;

  	TMSHeapFree(m_pFCSORT);
  
  	CDialog::OnOK();
  }
  else
  {
    CString s;

    s.LoadString(ERROR_069);
    MessageBeep(MB_ICONSTOP);
    MessageBox(s, TMS, MB_OK);
  }
}

void CFindCustomer::OnColumnclickList(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;

  if(pNMListView && m_bAllowSort)
  {
    ListView_SortItems(pNMListView->hdr.hwndFrom,
          FCListViewCompareProc, (LPARAM)(pNMListView->iSubItem));
    bSortForward[pNMListView->iSubItem] = !bSortForward[pNMListView->iSubItem];
  }
  
	*pResult = 0;
}

void CFindCustomer::OnClose() 
{
  OnCancel();
}


void CFindCustomer::OnDblclkList(NMHDR* pNMHDR, LRESULT* pResult) 
{
  OnOK();
  	
	*pResult = 0;
}


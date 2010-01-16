//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
// CCFrontEnd.cpp : implementation file
// 

#include "stdafx.h"
extern "C"{
#include "TMSHeader.h"
}

#include "cc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static BOOL bSortForward[3];


void CALLBACK EXPORT TimerProc(HWND hWnd, UINT nMsg, UINT nIDEvent, DWORD dwTime)
{
  CTime time = CTime::GetCurrentTime();
 	CString strTime = time.Format(_T("%A, %B %d, %Y at %I:%M:%S%p"));

  ::SetWindowText(hWnd, strTime);
}

int CALLBACK CCFEListViewCompareProc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
  CCFESORTDef *p1 = (CCFESORTDef *)lParam1;
  CCFESORTDef *p2 = (CCFESORTDef *)lParam2;
  int  iResult = 0;

  if(p1 && p2)
  {
    switch(lParamSort)
    {
      case 0:  // Reference number
        iResult = bSortForward[lParamSort] ?
              lstrcmpi(p1->szReferenceNumber, p2->szReferenceNumber) :
              lstrcmpi(p2->szReferenceNumber, p1->szReferenceNumber);
        break;

      case 1:  // Status
       iResult = bSortForward[lParamSort] ? 
              lstrcmpi(p1->szStatus, p2->szStatus) :
              lstrcmpi(p2->szStatus, p1->szStatus);
        break;

      case 2:  // Name
        iResult = bSortForward[lParamSort] ?
              lstrcmpi(p1->szName, p2->szName) : lstrcmpi(p2->szName, p1->szName);
        break;
    }
  }

  return(iResult);
}


/////////////////////////////////////////////////////////////////////////////
// CCCFrontEnd dialog


CCCFrontEnd::CCCFrontEnd(CWnd* pParent /*=NULL*/)
	: CDialog(CCCFrontEnd::IDD, pParent)
{
	//{{AFX_DATA_INIT(CCCFrontEnd)
	//}}AFX_DATA_INIT
}


void CCCFrontEnd::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CCCFrontEnd)
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CCCFrontEnd, CDialog)
	//{{AFX_MSG_MAP(CCCFrontEnd)
	ON_BN_CLICKED(IDHELP, OnHelp)
	ON_BN_CLICKED(CCFE_NEW, OnNew)
	ON_BN_CLICKED(CCFE_EXISTING, OnExisting)
	ON_NOTIFY(LVN_COLUMNCLICK, CCFE_LIST, OnColumnclickList)
	ON_EN_CHANGE(CCFE_JUMPTO, OnChangeJumpto)
	ON_NOTIFY(NM_DBLCLK, CCFE_LIST, OnDblclkList)
	ON_BN_CLICKED(IDREPORTS, OnReports)
	ON_BN_CLICKED(CCFE_DISPLAYCANCELLED, OnDisplaycancelled)
	ON_BN_CLICKED(IDREOPEN, OnReopen)
	ON_WM_CLOSE()
	ON_WM_TIMER()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCCFrontEnd message handlers

BOOL CCCFrontEnd::OnInitDialog() 
{
 	CDialog::OnInitDialog();

  CString s;
//
//  Default to no display of cancelled records
//
  m_bDisplayCancelled = FALSE;
//
//  Make all future memory pointers NULL
//
  pCCFESORT = NULL;
  CCFE.pCCREFER = NULL;
//
//  Set up the pointers to the controls
//
  pButtonNEW = (CButton *)GetDlgItem(CCFE_NEW);
  pButtonEXISTING = (CButton *)GetDlgItem(CCFE_EXISTING);
  pListCtrlLIST = (CListCtrl *)GetDlgItem(CCFE_LIST);
  pButtonTHISISA = (CButton *)GetDlgItem(CCFE_THISISA);
 	pWndDATETIME = GetDlgItem(CCFE_DATETIME);
  pWndRECEIVEDBY = GetDlgItem(CCFE_RECEIVEDBY);
  pButtonCOMPLAINT = (CButton *)GetDlgItem(CCFE_COMPLAINT);
  pButtonCOMMENDATION = (CButton *)GetDlgItem(CCFE_COMMENDATION);
  pButtonSERVICEREQUEST = (CButton *)GetDlgItem(CCFE_SERVICEREQUEST);
  pButtonMISCELLANEOUS = (CButton *)GetDlgItem(CCFE_MISCELLANEOUS);
  pStaticJUMPTO_TEXT = (CStatic *)GetDlgItem(CCFE_JUMPTO_TEXT);
  pEditJUMPTO = (CEdit *)GetDlgItem(CCFE_JUMPTO);
  pButtonDISPLAYCANCELLED = (CButton *)GetDlgItem(CCFE_DISPLAYCANCELLED);
  pButtonREOPEN = (CButton *)GetDlgItem(IDREOPEN);

  DWORD dwExStyles = pListCtrlLIST->GetExtendedStyle();
  pListCtrlLIST->SetExtendedStyle(dwExStyles | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_INFOTIP);
//
//  Set up the characters that the "referenceNumber" is prefixed by
//
//  Complaint, Commendation, or Service Request
//
  strncpy(cPrefix, "CMSL", 4);
//
//  Get the current date and time and put it on the group box
//  
  CTime time = CTime::GetCurrentTime();
 	CString strTime = time.Format(_T("%A, %B %d, %Y at %I:%M:%S%p"));
//
//  Set up the timer to update the date/time
//
  pWndDATETIME->SetWindowText(strTime);
 	pWndDATETIME->SetTimer(0x1000, 1000, TimerProc);
//
//  Display the user name
//
  pWndRECEIVEDBY->SetWindowText(userName);
//
//  Set up the list control
//
  LVCOLUMN LVC;
  LVC.mask = LVCF_FMT | LVCF_TEXT | LVCF_WIDTH;
  LVC.fmt = LVCFMT_LEFT;
  LVC.cx = 100;
  LVC.pszText = "Number";
  pListCtrlLIST->InsertColumn(0, &LVC);
  LVC.cx = 100;
  LVC.pszText = "Status";
  pListCtrlLIST->InsertColumn(1, &LVC);
  LVC.cx = 300;
  LVC.pszText = "Name";
  pListCtrlLIST->InsertColumn(2, &LVC);
  bSortForward[0] = TRUE;
  bSortForward[1] = TRUE;
  bSortForward[2] = TRUE;
  PopulateListCtrl();
//
//  Disable the list control for now
//
  pListCtrlLIST->EnableWindow(FALSE);
  pStaticJUMPTO_TEXT->EnableWindow(FALSE);
  pEditJUMPTO->EnableWindow(FALSE);

  pButtonNEW->SetCheck(TRUE);
  pButtonCOMPLAINT->SetCheck(TRUE);
  pButtonCOMPLAINT->SetFocus();
//
//  Set up the referral structure
//
  int nI;

  CCFE.numReferrals = m_numReferrals;

  if(m_numReferrals == 0)
  {
    CCFE.pCCREFER = NULL;
  }
  else
  {
    CCFE.pCCREFER = (CCREFERDef *)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(CCREFERDef) * m_numReferrals); 
    if(CCFE.pCCREFER == NULL)
    {
      AllocationError(__FILE__, __LINE__, FALSE);
    }
    else
    {
      for(nI = 0; nI < m_numReferrals; nI++)
      {
        strcpy(CCFE.pCCREFER[nI].szName, m_Referrals[nI].szName);
        strcpy(CCFE.pCCREFER[nI].szDepartment, m_Referrals[nI].szDepartment);
        strcpy(CCFE.pCCREFER[nI].szEmailAddress, m_Referrals[nI].szEmailAddress);
        CCFE.pCCREFER[nI].flags = 0;
      }
    }
  }

  ReleaseCapture();


 	return FALSE;  // return TRUE unless you set the focus to a control
                 // EXCEPTION: OCX Property Pages should return FALSE
}


void CCCFrontEnd::OnClose() 
{
  OnCancel();
}

void CCCFrontEnd::OnCancel() 
{
  TMSHeapFree(CCFE.pCCREFER);
  TMSHeapFree(pCCFESORT);
  	
	CDialog::OnCancel();
}

void CCCFrontEnd::OnHelp() 
{
  ::WinHelp(m_hWnd, szarHelpFile, HELP_CONTEXT, The_CCM_Front_End);
}


void CCCFrontEnd::OnNew() 
{
  BOOL bEnable = pButtonNEW->GetCheck();

  pListCtrlLIST->EnableWindow(!bEnable);
  pStaticJUMPTO_TEXT->EnableWindow(!bEnable);
  pEditJUMPTO->EnableWindow(!bEnable);
  pButtonTHISISA->EnableWindow(bEnable);
  pWndRECEIVEDBY->EnableWindow(bEnable);
  pButtonCOMPLAINT->EnableWindow(bEnable);
  pButtonCOMMENDATION->EnableWindow(bEnable);
  pButtonSERVICEREQUEST->EnableWindow(bEnable);
  pButtonMISCELLANEOUS->EnableWindow(bEnable);
}

void CCCFrontEnd::OnExisting() 
{
  BOOL bEnable = pButtonEXISTING->GetCheck();

  pListCtrlLIST->EnableWindow(bEnable);
  pStaticJUMPTO_TEXT->EnableWindow(bEnable);
  pEditJUMPTO->EnableWindow(bEnable);
  pButtonTHISISA->EnableWindow(!bEnable);
  pWndRECEIVEDBY->EnableWindow(!bEnable);
  pButtonCOMPLAINT->EnableWindow(!bEnable);
  pButtonCOMMENDATION->EnableWindow(!bEnable);
  pButtonSERVICEREQUEST->EnableWindow(!bEnable);
  pButtonMISCELLANEOUS->EnableWindow(!bEnable);
}

void CCCFrontEnd::OnColumnclickList(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;

  if(pNMListView && bAllowSort)
  {
    ListView_SortItems(pNMListView->hdr.hwndFrom,
          CCFEListViewCompareProc, (LPARAM)(pNMListView->iSubItem));
    bSortForward[pNMListView->iSubItem] = !bSortForward[pNMListView->iSubItem];
  }
  
  *pResult = 0;
}

void CCCFrontEnd::PopulateListCtrl()
{
//
//  Populate the list control
//
  CString s;
  LVITEM LVI;
  char   dummy[256];
  int    rcode2;
  int    nI;

  pListCtrlLIST->DeleteAllItems();

  rcode2 = btrieve(B_STAT, TMS_COMPLAINTS, &BSTAT, dummy, 0);
  if(rcode2 != 0 || BSTAT.numRecords == 0)
  {
    pButtonEXISTING->EnableWindow(FALSE);
  }
  else
  {
    if(BSTAT.numRecords > 0)
    {
      pCCFESORT = (CCFESORTDef *)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(CCFESORTDef) * BSTAT.numRecords); 
      if(pCCFESORT == NULL)
      {
        bAllowSort = FALSE;
        pButtonEXISTING->EnableWindow(FALSE);
        AllocationError(__FILE__, __LINE__, FALSE);
      }
      else
      {
        bAllowSort = TRUE;
        pListCtrlLIST->SetItemCount((int)BSTAT.numRecords);
        rcode2 = btrieve(B_GETFIRST, TMS_COMPLAINTS, &COMPLAINTS, &COMPLAINTSKey2, 2);
        for(nI = 0; nI < BSTAT.numRecords; nI++)
        {
          if(!((COMPLAINTS.flags & COMPLAINTS_FLAG_CANCELLED) && !m_bDisplayCancelled))
          {
            strncpy(tempString, COMPLAINTS.referenceNumber, COMPLAINTS_REFERENCENUMBER_LENGTH);
            trim(tempString, COMPLAINTS_REFERENCENUMBER_LENGTH);
//
//  Reference number
//
            LVI.mask = LVIF_TEXT;
            LVI.iItem = nI;
            LVI.iSubItem = 0;
            LVI.pszText = tempString;
            if(bAllowSort)
            {
              strcpy(pCCFESORT[nI].szReferenceNumber, tempString);
              LVI.mask |= LVIF_PARAM;
              LVI.lParam = (LPARAM)&pCCFESORT[nI];
            }
            LVI.iItem = pListCtrlLIST->InsertItem(&LVI);
//
//  Status
//
            LVI.mask = LVIF_TEXT;
            LVI.iSubItem = 1;
            if(COMPLAINTS.flags & COMPLAINTS_FLAG_CLOSED)
            {
              s.LoadString(TEXT_230);
            }
            else if(COMPLAINTS.flags & COMPLAINTS_FLAG_CANCELLED)
            {
              s.LoadString(TEXT_231);
            }
            else
            {
              s.LoadString(TEXT_229);
            }
            strcpy(tempString, s);
            LVI.pszText = tempString;
            if(bAllowSort)
            {
              strcpy(pCCFESORT[nI].szStatus, tempString);
            }
            pListCtrlLIST->SetItem(&LVI);
//
//  Name
//
            if(COMPLAINTS.CUSTOMERSrecordID == NO_RECORD)
            {
              s.LoadString(TEXT_237);
              strcpy(tempString, s);
            }
            else
            {
              CUSTOMERSKey0.recordID = COMPLAINTS.CUSTOMERSrecordID;
              btrieve(B_GETEQUAL, TMS_CUSTOMERS, &CUSTOMERS, &CUSTOMERSKey0, 0);
              strncpy(tempString, CUSTOMERS.lastName, CUSTOMERS_LASTNAME_LENGTH);
              trim(tempString, CUSTOMERS_LASTNAME_LENGTH);
              strcat(tempString, ", ");
              strncpy(szarString, CUSTOMERS.firstName, CUSTOMERS_FIRSTNAME_LENGTH);
              trim(szarString, CUSTOMERS_FIRSTNAME_LENGTH);
              strcat(tempString, szarString);
            }
            LVI.mask = LVIF_TEXT;
            LVI.iSubItem = 2;
            LVI.pszText = tempString;
            if(bAllowSort)
            {
              strcpy(pCCFESORT[nI].szName, tempString);
            }
            pListCtrlLIST->SetItem(&LVI);
          }
          rcode2 = btrieve(B_GETNEXT, TMS_COMPLAINTS, &COMPLAINTS, &COMPLAINTSKey2, 2);
        }
      }
    }
  }
}

void CCCFrontEnd::OnChangeJumpto() 
{
  CString s;

//
//  If the field is clear, just go to the top of the list and leave
//
  if(pEditJUMPTO->GetWindowText(tempString, TEMPSTRING_LENGTH) == 0)
  {
    pListCtrlLIST->EnsureVisible(0, FALSE);
    return;
  }
//
// Locate the string (or substring) s in the list control
//
  LVFINDINFO lvf;

  lvf.flags = LVFI_PARTIAL;
  lvf.psz = tempString;

  int pos = pListCtrlLIST->FindItem(&lvf);

  if(pos >= 0)
  {
    pListCtrlLIST->EnsureVisible(pos, FALSE);
    UINT flag = LVIS_SELECTED | LVIS_FOCUSED;
    pListCtrlLIST->SetItemState(pos, flag, flag);
  }
}

void CCCFrontEnd::OnDblclkList(NMHDR* pNMHDR, LRESULT* pResult) 
{
  OnOK();
  	
	*pResult = 0;
}

void CCCFrontEnd::OnReports() 
{
  CCCReports dlg;
  
  dlg.DoModal();	
}

void CCCFrontEnd::OnDisplaycancelled() 
{
  TMSHeapFree(pCCFESORT);
  m_bDisplayCancelled = pButtonDISPLAYCANCELLED->GetCheck();
  PopulateListCtrl();
}

void CCCFrontEnd::OnReopen() 
{
  CCCLogin login;
  CString  s;
  char     previousLog[COMPLAINTS_VARIABLE_LENGTH];
  int      rcode2;

  if(login.DoModal() == IDOK)
  {
    strncpy(COMPLAINTSKey2.referenceNumber,
          CCFE.COMPLAINTS.referenceNumber, COMPLAINTS_REFERENCENUMBER_LENGTH);
    recordLength[TMS_COMPLAINTS] = COMPLAINTS_TOTAL_LENGTH;
    rcode2 = btrieve(B_GETEQUAL, TMS_COMPLAINTS, pComplaintText, &COMPLAINTSKey2, 2);
    if(rcode2 == 0)
    {
      memcpy(&COMPLAINTS, pComplaintText, sizeof(COMPLAINTS));
      if(COMPLAINTS.flags & COMPLAINTS_FLAG_CLOSED)
      {
        COMPLAINTS.flags &= ~COMPLAINTS_FLAG_CLOSED;
        strcpy(previousLog, &pComplaintText[COMPLAINTS_FIXED_LENGTH]);
        pWndDATETIME->SendMessage(WM_GETTEXT,
              (WPARAM)CCFE_DATETIME_LENGTH, (LONG)(LPSTR)CCFE.szDateTime);
        pWndRECEIVEDBY->SendMessage(WM_GETTEXT,
              (WPARAM)CCFE_RECEIVEDBY_LENGTH, (LONG)(LPSTR)CCFE.szReceivedBy);
        s.LoadString(TEXT_233);
        sprintf(tempString, s, CCFE.szDateTime, CCFE.szReceivedBy);
        strcat(previousLog, tempString);
        s.LoadString(TEXT_234);
        sprintf(tempString, s, "status", "Closed", "Open");
        strcat(previousLog, tempString);
        memcpy(pComplaintText, &COMPLAINTS, COMPLAINTS_FIXED_LENGTH);
        strcpy(&pComplaintText[COMPLAINTS_FIXED_LENGTH], previousLog);
        btrieve(B_UPDATE, TMS_COMPLAINTS, pComplaintText, &COMPLAINTSKey2, 2);
        CCFE.COMPLAINTS = COMPLAINTS;
        CCFE.flags = COMPLAINTS.flags;
        pButtonREOPEN->EnableWindow(FALSE);
        PopulateListCtrl();
      }
    }
    recordLength[TMS_COMPLAINTS] = COMPLAINTS_FIXED_LENGTH;
  }
}

int CCCFrontEnd::ProcessSelectedItem()
{
	int nI = pListCtrlLIST->GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);

  if(nI < 0)
    return(nI);

  LVITEM LVI;
  int    rcode2;
 
  LVI.mask = LVIF_TEXT;
  LVI.iItem = nI;
  LVI.iSubItem = 0;
  LVI.pszText = tempString;
  LVI.cchTextMax = TEMPSTRING_LENGTH;
  pListCtrlLIST->GetItem(&LVI);
  pad(tempString, COMPLAINTS_REFERENCENUMBER_LENGTH);
  strncpy(COMPLAINTSKey2.referenceNumber, tempString, COMPLAINTS_REFERENCENUMBER_LENGTH);
  recordLength[TMS_COMPLAINTS] = COMPLAINTS_TOTAL_LENGTH;
  rcode2 = btrieve(B_GETEQUAL, TMS_COMPLAINTS, pComplaintText, &COMPLAINTSKey2, 2);
  recordLength[TMS_COMPLAINTS] = COMPLAINTS_FIXED_LENGTH;
  if(rcode2 != 0)
  {
    sprintf(tempString, "rcode2 = %d", rcode2);
    MessageBox(tempString, TMS, MB_OK);
    TMSError(NULL, MB_ICONSTOP, ERROR_261, (HANDLE)NULL);
    return(-rcode2);
  }
  memcpy(&COMPLAINTS, pComplaintText, sizeof(COMPLAINTS));
  CCFE.COMPLAINTS = COMPLAINTS;
  CCFE.flags = COMPLAINTS.flags;

  return(0);
}

void CCCFrontEnd::OnOK()   // Labelled "Continue" on the dialog
{
//
//  Date/time as long
//
  CTime time = CTime::GetCurrentTime();
  CCFE.dateTime = (long)time.GetTime();
//
//  Date/time as string
//
  pWndDATETIME->SendMessage(WM_GETTEXT,
        (WPARAM)CCFE_DATETIME_LENGTH, (LONG)(LPSTR)CCFE.szDateTime);
//
//  Received by
//
  pWndRECEIVEDBY->SendMessage(WM_GETTEXT,
        (WPARAM)CCFE_RECEIVEDBY_LENGTH, (LONG)(LPSTR)CCFE.szReceivedBy);
//
//  New complaint, commendation, or service request
//
  if(pButtonNEW->GetCheck())
  {
    memset(&CCFE.COMPLAINTS, 0x00, sizeof(COMPLAINTSDef));
    CCFE.flags = 0;
//
//  Prefix (referenceNumber[0])
//
    if(pButtonCOMPLAINT->GetCheck())
    {
      CCFE.flags |= CCFE_FLAGS_COMPLAINT;
      CCFE.COMPLAINTS.referenceNumber[0] = cPrefix[0];
    }
    else if(pButtonCOMMENDATION->GetCheck())
    {
      CCFE.flags |= CCFE_FLAGS_COMMENDATION;
      CCFE.COMPLAINTS.referenceNumber[0] = cPrefix[1];
    }
    else if(pButtonSERVICEREQUEST->GetCheck())
    {
      CCFE.flags |= CCFE_FLAGS_SERVICEREQUEST;
      CCFE.COMPLAINTS.referenceNumber[0] = cPrefix[2];
    }
    else if(pButtonMISCELLANEOUS->GetCheck())
    {
      CCFE.flags |= CCFE_FLAGS_MISCELLANEOUS;
      CCFE.COMPLAINTS.referenceNumber[0] = cPrefix[3];
    }
//
//  Year (referenceNumber[1] for 4)
//
    sprintf(&CCFE.COMPLAINTS.referenceNumber[1], "%4d", time.GetYear());
//
//  Month (referenceNunmber[5] for 1) as Jan=A, Feb=B, and so on
//
    CCFE.COMPLAINTS.referenceNumber[5] = 'A' + time.GetMonth() - 1;
//
//  Day (referenceNumber[6] for 2)
//
    sprintf(&CCFE.COMPLAINTS.referenceNumber[6], "%02d", time.GetDay());
//
//  See where we are
//
//  Get the last record (or at least part of it - ignore rcode2 == 22 [rec len])
//
    int rcode2;

    rcode2 = btrieve(B_GETLAST, TMS_COMPLAINTS, &COMPLAINTS, &COMPLAINTSKey0, 0);
    if(rcode2 != 0 && rcode2 != 22)
    {
      CCFE.COMPLAINTS.recordID = 1;
      strncpy(&CCFE.COMPLAINTS.referenceNumber[8], "A000", 4);
    }
    else
    {
      CCFE.COMPLAINTS.recordID = COMPLAINTS.recordID + 1;
//
//  Check a year rollover
//  Check a month rollover
//  Check on a day rollover
//
      if(strncmp(&CCFE.COMPLAINTS.referenceNumber[1], &COMPLAINTS.referenceNumber[1], 4) != 0 ||
            CCFE.COMPLAINTS.referenceNumber[5] != COMPLAINTS.referenceNumber[5] ||
            strncmp(&CCFE.COMPLAINTS.referenceNumber[6], &COMPLAINTS.referenceNumber[6], 2) != 0)
      {
        strncpy(&CCFE.COMPLAINTS.referenceNumber[8], "A000", 4);
      }
//
//  Check on a "thousands" rollover
//
      else if(strncmp(&COMPLAINTS.referenceNumber[9], "999", 3) == 0)
      {
        CCFE.COMPLAINTS.referenceNumber[8] = COMPLAINTS.referenceNumber[8] + 1;
        strncpy(&CCFE.COMPLAINTS.referenceNumber[9], "000", 3);
      }
//
//  None left - add one to the complaint number
//
      else
      {
        CCFE.COMPLAINTS.referenceNumber[8] = COMPLAINTS.referenceNumber[8];
        strncpy(tempString, &COMPLAINTS.referenceNumber[9], 3);
        tempString[3] = '\0';
        sprintf(&CCFE.COMPLAINTS.referenceNumber[9], "%03d", atoi(tempString) + 1);
      }
    }
//
//  Add the shell of the new record in
//
    CCFE.COMPLAINTS.COMMENTSrecordID = NO_RECORD;
    CCFE.COMPLAINTS.CUSTOMERSrecordID = NO_RECORD;
    CCFE.COMPLAINTS.ROUTESrecordID = NO_RECORD;
    CCFE.COMPLAINTS.directionIndex = NO_RECORD;
    CCFE.COMPLAINTS.DRIVERSrecordID = NO_RECORD;
    CCFE.COMPLAINTS.weatherIndex = NO_RECORD;
    CCFE.COMPLAINTS.typicalIndex = NO_RECORD;
    CCFE.COMPLAINTS.dateOfComplaint = time.GetYear() * 10000 + time.GetMonth() * 100 + time.GetDay();
    CCFE.COMPLAINTS.timeOfComplaint = time.GetHour() * 3600 + time.GetMinute() * 60 + time.GetSecond();
    strcpy(CCFE.COMPLAINTS.enteredBy, CCFE.szReceivedBy);
    CCFE.COMPLAINTS.flags = CCFE.flags;
    recordLength[TMS_COMPLAINTS] = COMPLAINTS_TOTAL_LENGTH;
    memcpy(pComplaintText, &CCFE.COMPLAINTS, COMPLAINTS_FIXED_LENGTH);
    memset(&pComplaintText[COMPLAINTS_FIXED_LENGTH], 0x00, COMPLAINTS_VARIABLE_LENGTH);
    rcode2 = btrieve(B_INSERT, TMS_COMPLAINTS, pComplaintText, &COMPLAINTSKey0, 0);
    if(rcode2 != 0)
    {
      sprintf(tempString, "Complaints insert: rcode2 = %d", rcode2);
      MessageBox(tempString, TMS, MB_OK);
    }
    recordLength[TMS_COMPLAINTS] = COMPLAINTS_FIXED_LENGTH;
    CCFE.flags |= CCFE_FLAGS_NEW;
  }
//
//  Not new - find out which.  ProcessSelectedItem()
//  sets up CCFE and returns 0 if OK
//
  else
  {
    if(ProcessSelectedItem() < 0)
    {
      return;
    }
  }
//
//  If this complaint is closed or was cancelled, just display the log
//
  if(CCFE.flags & COMPLAINTS_FLAG_CLOSED ||
        CCFE.flags & COMPLAINTS_FLAG_CANCELLED)
  {
    CCCDisplayLog dlg(this, &CCFE);
    dlg.DoModal();
  }
//
//  Fire up the main Customer Comment module
//
  else
  {
    CCustomerComment dlg(this, &CCFE);
    dlg.DoModal();
    TMSHeapFree(pCCFESORT);
    PopulateListCtrl();
  }
}

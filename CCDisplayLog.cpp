//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
// CCDisplayLog.cpp : implementation file
//

#include "stdafx.h"
extern "C"
{
#include "TMSHeader.h"
#include <mapi.h>
typedef ULONG (*LPFNMAPISENDMAIL)(ULONG, ULONG, MapiMessage*, FLAGS, ULONG);
}
#include "cc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static BOOL bSortForward[6];

int CALLBACK CCDisplayLogListViewCompareProc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
  CCREFERDef *p1 = (CCREFERDef *)lParam1;
  CCREFERDef *p2 = (CCREFERDef *)lParam2;
  int  iResult = 0;

  if(p1 && p2)
  {
    switch(lParamSort)
    {
      case 0:  // Name
        iResult = bSortForward[lParamSort] ?
              lstrcmpi(p1->szName, p2->szName) :
              lstrcmpi(p2->szName, p1->szName);
        break;

      case 1:  // Department
        iResult = bSortForward[lParamSort] ?
              lstrcmpi(p1->szDepartment, p2->szDepartment) :
              lstrcmpi(p2->szDepartment, p1->szDepartment);
        break;

      case 2:  // email address
        iResult = bSortForward[lParamSort] ?
              lstrcmpi(p1->szEmailAddress, p2->szEmailAddress) :
              lstrcmpi(p2->szEmailAddress, p1->szEmailAddress);
        break;

    }
  }

  return(iResult);
}

/////////////////////////////////////////////////////////////////////////////
// CCCDisplayLog dialog


CCCDisplayLog::CCCDisplayLog(CWnd* pParent, CCFEDef *pCCFE)
	: CDialog(CCCDisplayLog::IDD, pParent)
{
  m_pCCFE = pCCFE;
	//{{AFX_DATA_INIT(CCCDisplayLog)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CCCDisplayLog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CCCDisplayLog)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CCCDisplayLog, CDialog)
	//{{AFX_MSG_MAP(CCCDisplayLog)
	ON_BN_CLICKED(CCDISPLAYLOG_EMAIL, OnEmail)
	ON_NOTIFY(LVN_COLUMNCLICK, CCDISPLAYLOG_LIST, OnColumnclickList)
	ON_BN_CLICKED(IDHELP, OnHelp)
	ON_WM_CLOSE()
	ON_NOTIFY(NM_DBLCLK, CCDISPLAYLOG_LIST, OnDblclkList)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCCDisplayLog message handlers

void CCCDisplayLog::OnEmail() 
{
  if(pButtonEMAIL->GetCheck())
    pListCtrlLIST->EnableWindow(TRUE);
  else
    pListCtrlLIST->EnableWindow(FALSE);
}

void CCCDisplayLog::OnColumnclickList(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
 
  if(pNMListView)
  {
    ListView_SortItems(pNMListView->hdr.hwndFrom,
          CCDisplayLogListViewCompareProc, (LPARAM)(pNMListView->iSubItem));
    bSortForward[pNMListView->iSubItem] = !bSortForward[pNMListView->iSubItem];
  }
	
	*pResult = 0;
}

void CCCDisplayLog::OnOK() 
{
//
//  See if we have to print the log
//
  if(pButtonPRINTLOG->GetCheck())
  {
    PrintLog();
  }
//
//  See if we have to email someone
//
  if(pButtonEMAIL->GetCheck())
  {
  	int     nI = pListCtrlLIST->GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);

    if(nI >= 0)
    {
      CCREFERDef *p;
      int nJ;
      int numSelected = 0;
    
      while(nI >= 0)
      {
        nJ = pListCtrlLIST->GetItemData(nI);
        p = (CCREFERDef *)nJ;
        p->flags = CCREFER_FLAGS_SELECTED;
        nI = pListCtrlLIST->GetNextItem(nI, LVNI_ALL | LVNI_SELECTED);
        numSelected++;
      }

      LPFNMAPISENDMAIL m_lpfnMAPISendMail;
      HINSTANCE m_hlibMAPI;
      m_hlibMAPI = AfxLoadLibrary("mapi32.dll");
      m_lpfnMAPISendMail = (LPFNMAPISENDMAIL) GetProcAddress(m_hlibMAPI, "MAPISendMail");
 
      MapiMessage message;

      memset(&message, 0x00, sizeof(MapiMessage));
      message.lpszSubject = "Customer Complaint Module Test";

      CString s = m_previousLog;
      message.lpszNoteText = (char *)LPCTSTR(s);
      message.nRecipCount = numSelected;

      MapiRecipDesc *pRecipients = NULL;

      pRecipients = (MapiRecipDesc *)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(MapiRecipDesc) * numSelected); 
      if(pRecipients == NULL)
      {
        AllocationError(__FILE__, __LINE__, FALSE);
      }
      else
      {
        char *pszRecipientNames = NULL;
        int   nameLength = (CCREFER_NAME_LENGTH + 1);
       
        pszRecipientNames = (char *)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, nameLength * numSelected); 
        if(pszRecipientNames == NULL)
        {
          AllocationError(__FILE__, __LINE__, TRUE);
        }
        else
        {
          numSelected = 0;
          for(nI = 0; nI < m_pCCFE->numReferrals; nI++)
          {
            if(!(m_pCCFE->pCCREFER[nI].flags & CCREFER_FLAGS_SELECTED))
            {
              continue;
            }
            pRecipients[numSelected].ulRecipClass = MAPI_TO;
            strcpy(szarString, m_pCCFE->pCCREFER[nI].szName);
            strcpy(&pszRecipientNames[nameLength * numSelected], szarString);
            pRecipients[numSelected].lpszName = &pszRecipientNames[nameLength * numSelected];
            pRecipients[numSelected].lpszAddress = m_pCCFE->pCCREFER[nI].szEmailAddress;
            numSelected++;
          }
          message.lpRecips = pRecipients;
          message.nFileCount = 0;
          message.lpFiles = NULL;

          ULONG ulResult = m_lpfnMAPISendMail(0, 0, &message, MAPI_LOGON_UI, 0);
          TMSHeapFree(pszRecipientNames);
        }
        TMSHeapFree(pRecipients);
      }
      AfxFreeLibrary(m_hlibMAPI);
    }
  }
	CDialog::OnOK();
}

void CCCDisplayLog::OnCancel() 
{
	CDialog::OnCancel();
}

void CCCDisplayLog::OnHelp() 
{
  ::WinHelp(m_hWnd, szarHelpFile, HELP_CONTEXT, The_CCM_Front_End);
}

void CCCDisplayLog::OnClose() 
{
	OnCancel();
}


void CCCDisplayLog::PrintLog()
{
  TEXTMETRIC tm;
  DOCINFO di;
  HDC hDCPrn;
  int yChar;
//
//  Get the text from "Previous log"
//

  CPrintDialog *printDlg = new CPrintDialog(FALSE, PD_ALLPAGES | PD_RETURNDC, NULL);
//
//  Initialize some of the fields in PRINTDLG structure
//
  printDlg->m_pd.nMinPage = printDlg->m_pd.nMaxPage = 1;
  printDlg->m_pd.nFromPage = printDlg->m_pd.nToPage = 1;
//
//  Display Windows print dialog box
//
  printDlg->DoModal();
//
//  Obtain a handle to the device context
//
  hDCPrn = printDlg->GetPrinterDC();
  if(hDCPrn != NULL)
  {
    CDC *pDC = new CDC;

    pDC->Attach(hDCPrn);
    memset(&di, 0x00, sizeof(DOCINFO));
    di.cbSize = sizeof(DOCINFO);
    di.lpszDocName = "Customer Comment Log";
    pDC->StartDoc(&di);
    pDC->StartPage();  
//
//  Set the print alignment
//
    pDC->SetMapMode(MM_TEXT);
    if(pDC->GetTextMetrics(&tm))
      yChar = tm.tmHeight + tm.tmExternalLeading;
    else
      yChar = 1;
    pDC->SetTextAlign(TA_BASELINE);
//
//  Read from the edit control
//
    CString strText;
    int len, nI;
    int numLines = pEditCCDPREVIOUSLOG->GetLineCount();
    int printedLines = 0;
    int cyPage = ::GetDeviceCaps (hDCPrn, VERTRES) ;

    for(nI = 0; nI < numLines; nI++)
    {
      len = pEditCCDPREVIOUSLOG->LineLength(pEditCCDPREVIOUSLOG->LineIndex(nI));
      pEditCCDPREVIOUSLOG->GetLine(nI, strText.GetBuffer(len), len);
      strncpy(tempString, strText, len);
      tempString[len] = '\0';
      strText.ReleaseBuffer(len);
      pDC->TextOut(256, (nI * yChar) + 256, tempString, strlen(tempString));
      printedLines++;
      if((printedLines * yChar) + 256 > cyPage)
      {
        pDC->EndPage();
        pDC->StartPage();
        printedLines = 0;
      }
    }
    pDC->EndPage();    
    pDC->EndDoc();     
    pDC->Detach();     
    delete pDC;
  }

  delete printDlg;
}


BOOL CCCDisplayLog::OnInitDialog() 
{
	CDialog::OnInitDialog();

  CString s;
  int  nI;

//
//  Set up the pointers to the controls
//	
  pEditCCDPREVIOUSLOG = (CEdit *)GetDlgItem(CCDISPLAYLOG_PREVIOUSLOG);
  pButtonPRINTLOG = (CButton *)GetDlgItem(CCDISPLAYLOG_PRINTLOG);
  pButtonEMAIL = (CButton *)GetDlgItem(CCDISPLAYLOG_EMAIL);
  pListCtrlLIST = (CListCtrl *)GetDlgItem(CCDISPLAYLOG_LIST);

  DWORD dwExStyles = pListCtrlLIST->GetExtendedStyle();
  pListCtrlLIST->SetExtendedStyle(dwExStyles | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_INFOTIP);

//
//  Set up the window title
//
  s.LoadString(TEXT_232);
  strncpy(szarString, COMPLAINTS.referenceNumber, COMPLAINTS_REFERENCENUMBER_LENGTH);
  trim(szarString, COMPLAINTS_REFERENCENUMBER_LENGTH);
  sprintf(tempString, s, szarString);
  SetWindowText(tempString);
//
//  Display any log entries
//
//  Previous
//
  COMPLAINTSKey0.recordID = m_pCCFE->COMPLAINTS.recordID;
  recordLength[TMS_COMPLAINTS] = COMPLAINTS_TOTAL_LENGTH;
  btrieve(B_GETEQUAL, TMS_COMPLAINTS, pComplaintText, &COMPLAINTSKey0, 0);
  recordLength[TMS_COMPLAINTS] = COMPLAINTS_FIXED_LENGTH;
  strcpy(m_previousLog, &pComplaintText[COMPLAINTS_FIXED_LENGTH]);
  pEditCCDPREVIOUSLOG->SetWindowText(m_previousLog);
//
//  Set up the list control
//	
  LVCOLUMN LVC;
  LVITEM   LVI;

  LVC.mask = LVCF_FMT | LVCF_TEXT | LVCF_WIDTH;
  LVC.fmt = LVCFMT_LEFT;
  LVC.cx = 100;
  LVC.pszText = "Surname";
  pListCtrlLIST->InsertColumn(0, &LVC);
  LVC.cx = 100;
  LVC.pszText = "First name";
  pListCtrlLIST->InsertColumn(1, &LVC);
  LVC.cx = 200;
  LVC.pszText = "Department";
  pListCtrlLIST->InsertColumn(2, &LVC);
  LVC.cx = 200;
  LVC.pszText = "Email address";
  pListCtrlLIST->InsertColumn(3, &LVC);
  for(nI = 0; nI < 4; nI++)
  {
    bSortForward[nI] = TRUE;
  }
//
//  Populate the list control
//
  pListCtrlLIST->SetItemCount(m_pCCFE->numReferrals);
  for(nI = 0; nI < m_pCCFE->numReferrals; nI++)
  { 
    memset(&LVI, 0x00, sizeof(LVITEM));
    LVI.mask = LVIF_TEXT | LVIF_PARAM;
    if(m_pCCFE->pCCREFER[nI].flags & CCREFER_FLAGS_SELECTED)
    {
      LVI.state = LVIS_SELECTED;
      LVI.stateMask = LVIS_SELECTED;
      LVI.mask |= LVIF_STATE;
    }
    LVI.iItem = nI;
    LVI.iSubItem = 0;
    LVI.pszText = m_pCCFE->pCCREFER[nI].szName;
    LVI.lParam = (LPARAM)&m_pCCFE->pCCREFER[nI];
    LVI.iItem = pListCtrlLIST->InsertItem(&LVI);

    LVI.mask = LVIF_TEXT;
    LVI.iSubItem = 1;
    LVI.pszText = m_pCCFE->pCCREFER[nI].szDepartment;
    pListCtrlLIST->SetItem(&LVI);

    LVI.mask = LVIF_TEXT;
    LVI.iSubItem = 2;
    LVI.pszText = m_pCCFE->pCCREFER[nI].szEmailAddress;
    pListCtrlLIST->SetItem(&LVI);
  }
//
//  Disable the list control until pButtonEMAIL is checked
//
  pListCtrlLIST->EnableWindow(FALSE);
//
//  All done
//
	return FALSE; // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}


void CCCDisplayLog::OnDblclkList(NMHDR* pNMHDR, LRESULT* pResult) 
{
  OnOK();
  	
	*pResult = 0;
}


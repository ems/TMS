//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
// CCProcess.cpp : implementation file
//

#include "stdafx.h"
extern "C"
{
#include "TMSHeader.h"
}
#include "ExternalTextFiles.h"
#include "cc.h"
#include <mapi.h>
#include "imapi.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define MAXNAMES   100
#define NAMELENGTH CCREFER_SURNAME_LENGTH + 1 + CCREFER_FIRSTNAME_LENGTH + 1

static BOOL bSortForward[4];

int CALLBACK CCProcessListViewCompareProc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
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
// CCCProcess dialog


CCCProcess::CCCProcess(CWnd* pParent, CCFEDef *pCCFE, char *pCurrentLog)
	: CDialog(CCCProcess::IDD, pParent)
{
  m_pCCFE = pCCFE;
  m_pCurrentLog = pCurrentLog;

	//{{AFX_DATA_INIT(CCCProcess)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CCCProcess::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CCCProcess)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CCCProcess, CDialog)
	//{{AFX_MSG_MAP(CCCProcess)
	ON_BN_CLICKED(IDHELP, OnHelp)
	ON_WM_CLOSE()
	ON_NOTIFY(LVN_COLUMNCLICK, CCPROCESS_LIST, OnColumnclickList)
	ON_BN_CLICKED(CCPROCESS_EMAIL, OnEmail)
	ON_LBN_DBLCLK(CCPROCESS_TYPICAL, OnDblclkTypical)
	ON_NOTIFY(NM_DBLCLK, CCPROCESS_LIST, OnDblclkList)
	ON_BN_CLICKED(IDBACK, OnBack)
	ON_BN_CLICKED(IDSUPERVISOR, OnSupervisor)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCCProcess message handlers

BOOL CCCProcess::OnInitDialog() 
{
	CDialog::OnInitDialog();

  CString s;
  long year;
  long month;
  long day;
  long hours;
  long minutes;
  long seconds;
  int  nI, nJ, nK;
  int  rcode2;

//
//  Set up the pointers to the controls
//	
  pEditPREVIOUSLOG = (CEdit *)GetDlgItem(CCPROCESS_PREVIOUSLOG);
  pEditCURRENTLOG = (CEdit *)GetDlgItem(CCPROCESS_CURRENTLOG);
  pButtonPRINTLETTER = (CButton *)GetDlgItem(CCPROCESS_PRINTLETTER);
  pButtonPRINTLOG = (CButton *)GetDlgItem(CCPROCESS_PRINTLOG);
  pButtonPRINTFORREVIEW = (CButton *)GetDlgItem(CCPROCESS_PRINTFORREVIEW);
  pButtonEMAIL = (CButton *)GetDlgItem(CCPROCESS_EMAIL);
  pButtonSUPERVISOR = (CButton *)GetDlgItem(IDSUPERVISOR);
  pListCtrlLIST = (CListCtrl *)GetDlgItem(CCPROCESS_LIST);
  pListBoxTYPICAL = (CListBox *)GetDlgItem(CCPROCESS_TYPICAL);

  DWORD dwExStyles = pListCtrlLIST->GetExtendedStyle();
  pListCtrlLIST->SetExtendedStyle(dwExStyles | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_INFOTIP);
//
//  Set up the window title
//
  s.LoadString(TEXT_161);
  strncpy(szarString, COMPLAINTS.referenceNumber, COMPLAINTS_REFERENCENUMBER_LENGTH);
  trim(szarString, COMPLAINTS_REFERENCENUMBER_LENGTH);
  sprintf(tempString, s, szarString);
  SetWindowText(tempString);
//
//  Is this new?
//
//  No.
//
  if(!(m_pCCFE->flags & CCFE_FLAGS_NEW))
  {
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
//
//  Stamp today's date and this user as having viewed the record
//
    s.LoadString(TEXT_289);
    sprintf(tempString, s, m_pCCFE->szDateTime, m_pCCFE->szReceivedBy);
    strcat(m_previousLog, tempString);
//
//  Establish any differences and write them out to the log
//
    char szOld[64], szNew[64], *ptr;
    s.LoadString(TEXT_234);
//
//  Comment code
//
    BOOL bChanged = FALSE;

    if(m_pCCFE->COMPLAINTS.COMMENTSrecordID != m_pCCFE->SNAPSHOT.COMMENTSrecordID)
    {
      for(nI = 0; nI < 2; nI++)
      {
        if(nI == 0)
        {
          ptr = szOld;
          COMMENTSKey0.recordID = m_pCCFE->COMPLAINTS.COMMENTSrecordID;
        }
        else
        {
          ptr = szNew;
          COMMENTSKey0.recordID = m_pCCFE->SNAPSHOT.COMMENTSrecordID;
        }
        rcode2 = btrieve(B_GETEQUAL, TMS_COMMENTS, &COMMENTS, &COMMENTSKey0, 0);
        if(rcode2 != 0)
          strcpy(ptr, "");
        else
        {
          strncpy(ptr, COMMENTS.code, COMMENTS_CODE_LENGTH);
          trim(ptr, COMMENTS_CODE_LENGTH);
        }
      }
      sprintf(tempString, s, "comment code", szOld, szNew);
      strcat(m_previousLog, tempString);
      bChanged = TRUE;
    }
//
//  Customer
//
    if(m_pCCFE->COMPLAINTS.CUSTOMERSrecordID != m_pCCFE->SNAPSHOT.CUSTOMERSrecordID)
    {
      for(nI = 0; nI < 2; nI++)
      {
        if(nI == 0)
        {
          ptr = szOld;
          CUSTOMERSKey0.recordID = m_pCCFE->COMPLAINTS.CUSTOMERSrecordID;
        }
        else
        {
          ptr = szNew;
          CUSTOMERSKey0.recordID = m_pCCFE->SNAPSHOT.CUSTOMERSrecordID;
        }
        rcode2 = btrieve(B_GETEQUAL, TMS_CUSTOMERS, &CUSTOMERS, &CUSTOMERSKey0, 0);
        if(rcode2 != 0)
        {
          s.LoadString(TEXT_237);
          strcpy(ptr, s);
        }
        else
        {
          strncpy(ptr, CUSTOMERS.salutation, CUSTOMERS_SALUTATION_LENGTH);
          trim(ptr, CUSTOMERS_SALUTATION_LENGTH);
          strncpy(tempString, CUSTOMERS.firstName, CUSTOMERS_FIRSTNAME_LENGTH);
          trim(tempString, CUSTOMERS_FIRSTNAME_LENGTH);
          strcat(ptr, " ");
          strcat(ptr, tempString);
          strncpy(tempString, CUSTOMERS.lastName, CUSTOMERS_LASTNAME_LENGTH);
          trim(tempString, CUSTOMERS_LASTNAME_LENGTH);
          strcat(ptr, " ");
          strcat(ptr, tempString);
        }
      }
      sprintf(tempString, s, "customer", szOld, szNew);
      strcat(m_previousLog, tempString);
      bChanged = TRUE;
    }
//
//  Date/time of occurrence
//
    if(m_pCCFE->COMPLAINTS.dateOfOccurrence != m_pCCFE->SNAPSHOT.dateOfOccurrence ||
          m_pCCFE->COMPLAINTS.timeOfOccurrence != m_pCCFE->SNAPSHOT.timeOfOccurrence)
    {
      for(nI = 0; nI < 2; nI++)
      {
        if(nI == 0)
        {
          ptr = szOld;
          nJ = m_pCCFE->COMPLAINTS.dateOfOccurrence;
          nK = m_pCCFE->COMPLAINTS.timeOfOccurrence;
        }
        else
        {
          ptr = szNew;
          nJ = m_pCCFE->SNAPSHOT.dateOfOccurrence;
          nK = m_pCCFE->SNAPSHOT.timeOfOccurrence;
        }
        GetYMD((long)nJ, &year, &month, &day);
        GetHMS((long)nK, &hours, &minutes, &seconds);
        CTime   t(year, month, day, hours, minutes, seconds);
        CString x = t.Format(_T("%A, %B %d, %Y at %I:%M:%S%p"));
        strcpy(ptr, x);
      }
      sprintf(tempString, s, "date/time of occurrence", szOld, szNew);
      strcat(m_previousLog, tempString);
      bChanged = TRUE;
    }
//
//  Bus number
//
    if(strncmp(m_pCCFE->COMPLAINTS.busNumber, m_pCCFE->SNAPSHOT.busNumber, COMPLAINTS_BUSNUMBER_LENGTH) != 0)
    {
      for(nI = 0; nI < 2; nI++)
      {
        if(nI == 0)
        {
          ptr = szOld;
          strncpy(tempString, m_pCCFE->COMPLAINTS.busNumber, COMPLAINTS_BUSNUMBER_LENGTH);
        }
        else
        {
          ptr = szNew;
          strncpy(tempString, m_pCCFE->SNAPSHOT.busNumber, COMPLAINTS_BUSNUMBER_LENGTH);
        }
        trim(tempString, COMPLAINTS_BUSNUMBER_LENGTH);
        strcpy(ptr, tempString);
      }
      sprintf(tempString, s, "bus number", szOld, szNew);
      strcat(m_previousLog, tempString);
      bChanged = TRUE;
    }
//
//  Route
//
    if(m_pCCFE->COMPLAINTS.ROUTESrecordID != m_pCCFE->SNAPSHOT.ROUTESrecordID)
    {
      for(nI = 0; nI < 2; nI++)
      {
        if(nI == 0)
        {
          ptr = szOld;
          ROUTESKey0.recordID = m_pCCFE->COMPLAINTS.ROUTESrecordID;
        }
        else
        {
          ptr = szNew;
          ROUTESKey0.recordID = m_pCCFE->SNAPSHOT.ROUTESrecordID;
        }
        rcode2 = btrieve(B_GETEQUAL, TMS_ROUTES, &ROUTES, &ROUTESKey0, 0);
        if(rcode2 != 0)
        {
          s.LoadString(TEXT_173);
          strcpy(ptr, s);
        }
        else
        {
          strncpy(ptr, ROUTES.number, ROUTES_NUMBER_LENGTH);
          trim(ptr, ROUTES_NUMBER_LENGTH);
          strcat(ptr, " - ");
          strncpy(tempString, ROUTES.name, ROUTES_NAME_LENGTH);
          trim(tempString, ROUTES_NAME_LENGTH);
          strcat(ptr, tempString);
        }
      }
      sprintf(tempString, s, "route", szOld, szNew);
      strcat(m_previousLog, tempString);
      bChanged = TRUE;
    }
//
//  Direction index
//
    if(m_pCCFE->COMPLAINTS.directionIndex != m_pCCFE->SNAPSHOT.directionIndex)
    {
      for(nI = 0; nI < 2; nI++)
      {
        if(nI == 0)
        {
          ptr = szOld;
          nJ = m_pCCFE->COMPLAINTS.directionIndex;
        }
        else
        {
          ptr = szNew;
          nJ = m_pCCFE->SNAPSHOT.directionIndex;
        }
        if(m_pCCFE->COMPLAINTS.ROUTESrecordID == NO_RECORD || nJ == NO_RECORD)
          strcpy(ptr, "");
        else
        {
          ROUTESKey0.recordID = m_pCCFE->COMPLAINTS.ROUTESrecordID;
          btrieve(B_GETEQUAL, TMS_ROUTES, &ROUTES, &ROUTESKey0, 0);
          DIRECTIONSKey0.recordID = ROUTES.DIRECTIONSrecordID[nJ];
          btrieve(B_GETEQUAL, TMS_DIRECTIONS, &DIRECTIONS, &DIRECTIONSKey0, 0);
          strncpy(ptr, DIRECTIONS.longName, DIRECTIONS_LONGNAME_LENGTH);
          trim(ptr, DIRECTIONS_LONGNAME_LENGTH);
        }
      }
      sprintf(tempString, s, "direction", szOld, szNew);
      strcat(m_previousLog, tempString);
      bChanged = TRUE;
    }
//
//   Location
//
    if(strncmp(m_pCCFE->COMPLAINTS.location, m_pCCFE->SNAPSHOT.location, COMPLAINTS_LOCATION_LENGTH) != 0)
    {
      for(nI = 0; nI < 2; nI++)
      {
        if(nI == 0)
        {
          ptr = szOld;
          strncpy(ptr, m_pCCFE->COMPLAINTS.location, COMPLAINTS_LOCATION_LENGTH);
        }
        else
        {
          ptr = szNew;
          strncpy(ptr, m_pCCFE->SNAPSHOT.location, COMPLAINTS_LOCATION_LENGTH);
        }
        trim(ptr, COMPLAINTS_LOCATION_LENGTH);
      }
      sprintf(tempString, s, "location", szOld, szNew);
      strcat(m_previousLog, tempString);
      bChanged = TRUE;
    }
//
//  Driver recordID
//
    if(m_pCCFE->COMPLAINTS.DRIVERSrecordID != m_pCCFE->SNAPSHOT.DRIVERSrecordID)
    {
      for(nI = 0; nI < 2; nI++)
      {
        if(nI == 0)
        {
          ptr = szOld;
          DRIVERSKey0.recordID = m_pCCFE->COMPLAINTS.DRIVERSrecordID;
        }
        else
        {
          ptr = szNew;
          DRIVERSKey0.recordID = m_pCCFE->COMPLAINTS.DRIVERSrecordID;
        }
        rcode2 = btrieve(B_GETEQUAL, TMS_DRIVERS, &DRIVERS, &DRIVERSKey0, 0);
        if(rcode2 != 0)
        {
          s.LoadString(TEXT_173);
          strcpy(ptr, s);
        }
        else
        {
          strncpy(ptr, DRIVERS.badgeNumber, DRIVERS_BADGENUMBER_LENGTH);
          trim(ptr, DRIVERS_BADGENUMBER_LENGTH);
          strcat(ptr, " - ");
          strncpy(tempString, DRIVERS.firstName, DRIVERS_FIRSTNAME_LENGTH);
          trim(tempString, DRIVERS_FIRSTNAME_LENGTH);
          strcat(ptr, tempString);
          strcat(ptr, " ");
          strncpy(tempString, DRIVERS.lastName, DRIVERS_LASTNAME_LENGTH);
          trim(tempString, DRIVERS_LASTNAME_LENGTH);
          strcat(ptr, tempString);
        }
      }
      sprintf(tempString, s, "driver", szOld, szNew);
      strcat(m_previousLog, tempString);
      bChanged = TRUE;
    }
//
//  Driver description
//
    if(strncmp(m_pCCFE->COMPLAINTS.driverDescription,
          m_pCCFE->SNAPSHOT.driverDescription, COMPLAINTS_DRIVERDESCRIPTION_LENGTH) != 0)
    {
      for(nI = 0; nI < 2; nI ++)
      {
        if(nI == 0)
        {
          ptr = szNew;
          strncpy(ptr, m_pCCFE->COMPLAINTS.driverDescription, COMPLAINTS_DRIVERDESCRIPTION_LENGTH);
        }
        else
        {
          ptr = szOld;
          strncpy(ptr, m_pCCFE->SNAPSHOT.driverDescription, COMPLAINTS_DRIVERDESCRIPTION_LENGTH);
        }
        trim(ptr, COMPLAINTS_DRIVERDESCRIPTION_LENGTH);
      }
      sprintf(tempString, s, "driver description", szOld, szNew);
      strcat(m_previousLog, tempString);
      bChanged = TRUE;
    }
//
//  Driver speed
//
    if(m_pCCFE->COMPLAINTS.driverSpeed != m_pCCFE->SNAPSHOT.driverSpeed)
    {
      for(nI = 0; nI < 2; nI++)
      {
        if(nI == 0)
        {
          ltoa(m_pCCFE->COMPLAINTS.driverSpeed, szOld, 10);
        }
        else
        {
          ltoa(m_pCCFE->SNAPSHOT.driverSpeed, szNew, 10);
        }
      }
      sprintf(tempString, s, "driver speed", szOld, szNew);
      strcat(m_previousLog, tempString);
      bChanged = TRUE;
    }
//
//  Weather index
//
    if(m_pCCFE->COMPLAINTS.weatherIndex != m_pCCFE->SNAPSHOT.weatherIndex)
    {
      nJ = m_pCCFE->COMPLAINTS.weatherIndex;
      nK = m_pCCFE->SNAPSHOT.weatherIndex;
      strcpy(szOld, m_WeatherConditions[nJ].szText);
      strcpy(szNew, m_WeatherConditions[nK].szText);
      sprintf(tempString, s, "weather", szOld, szNew);
      strcat(m_previousLog, tempString);
      bChanged = TRUE;
    }
//
//  Flags
//
//  Filed previous
//
    if((m_pCCFE->COMPLAINTS.flags & COMPLAINTS_FLAG_FILEDPREVIOUS) &&
          !(m_pCCFE->SNAPSHOT.flags & COMPLAINTS_FLAG_FILEDPREVIOUS))
    {
    }
//
//  If anything was changed, put who did it in the log
//
    if(bChanged)
    {
      s.LoadString(TEXT_233);
      sprintf(tempString, s, m_pCCFE->szDateTime, m_pCCFE->szReceivedBy);
      strcat(m_previousLog, tempString);
    }
//
//  Display the updated log
//
    pEditPREVIOUSLOG->SetWindowText(m_previousLog);
  }
//
//  This is a new complaint.  Set up "previousLog"
//
  else
  {
    int flags = CCPROCESS_LOGFLAGS_ALL;
    SetupLog(flags);
    pEditPREVIOUSLOG->SetWindowText(m_previousLog);
  }
//
//  Current log entries
//
  pEditCURRENTLOG->SetWindowText(m_pCurrentLog);
  pEditCURRENTLOG->LimitText(COMPLAINTS_VARIABLE_LENGTH);
  pEditCURRENTLOG->SetSel(-1, -1, FALSE);  // Deselect everything
//
//  Set up the typical list box
//
  if(m_numTypical == 0)
  {
    TMSError(NULL, MB_ICONINFORMATION, ERROR_128, (HANDLE)NULL);
    pListBoxTYPICAL->EnableWindow(FALSE);
    pEditCURRENTLOG->SetFocus();
  }
  else
  {
    for(nI = 0; nI < m_numTypical; nI++)
    {
      nI = pListBoxTYPICAL->AddString(m_Typical[nI].szText);
      pListBoxTYPICAL->SetItemData(nI, m_Typical[nI].number);
    }
    pListBoxTYPICAL->SetCurSel(0);
    pListBoxTYPICAL->SetFocus();
  }
//
//  Set up the list control
//	
  LVCOLUMN LVC;
  LVITEM   LVI;

  LVC.mask = LVCF_FMT | LVCF_TEXT | LVCF_WIDTH;
  LVC.fmt = LVCFMT_LEFT;
  LVC.cx = 194;
  LVC.pszText = "Name";
  pListCtrlLIST->InsertColumn(0, &LVC);

  LVC.cx = 194;
  LVC.pszText = "Department";
  pListCtrlLIST->InsertColumn(1, &LVC);

  LVC.cx = 194;
  LVC.pszText = "Email address";
  pListCtrlLIST->InsertColumn(2, &LVC);
  for(nI = 0; nI < 3; nI++)
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
//  If this person is anonymous, he doesn't get the "Generic Letter"
//
  if(m_pCCFE->SNAPSHOT.CUSTOMERSrecordID == NO_RECORD)
    pButtonPRINTLETTER->EnableWindow(FALSE);
//
//  All done
//
	return FALSE; // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CCCProcess::OnCancel() 
{
  CString s;

  if(s.LoadString(ERROR_159))  // Are you sure?
  {
    if(MessageBox(s, TMS, MB_ICONQUESTION | MB_YESNO) == IDYES)
      CDialog::OnCancel();
  }
}

void CCCProcess::OnHelp() 
{
  ::WinHelp(m_hWnd, szarHelpFile, HELP_CONTEXT, The_Process_Call_Dialog	);
}

void CCCProcess::OnOK() 
{
  SaveRecord();

	CDialog::OnOK();
}

void CCCProcess::OnClose() 
{
  OnCancel();
}


void CCCProcess::OnColumnclickList(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
 
  if(pNMListView)
  {
    ListView_SortItems(pNMListView->hdr.hwndFrom,
          CCProcessListViewCompareProc, (LPARAM)(pNMListView->iSubItem));
    bSortForward[pNMListView->iSubItem] = !bSortForward[pNMListView->iSubItem];
  }
	
	*pResult = 0;
}

void CCCProcess::OnEmail() 
{
  if(pButtonEMAIL->GetCheck())
    pListCtrlLIST->EnableWindow(TRUE);
  else
    pListCtrlLIST->EnableWindow(FALSE);
}

void CCCProcess::PrintLog(BOOL bFromControl)
{
  TEXTMETRIC tm;
  DOCINFO di;
  char *ptr;
  char tempLog[sizeof(m_previousLog)];
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
    int numLines;
    int printedLines = 0;
    int cyPage = ::GetDeviceCaps (hDCPrn, VERTRES) ;
    int nI;

    if(bFromControl)
      numLines = pEditPREVIOUSLOG->GetLineCount();
    else
    {
      strcpy(tempLog, m_previousLog);
      strcat(tempLog, "\n\nEmployee response:\n");
      numLines = 0;
      for(nI = 0; nI < (int)strlen(tempLog); nI++)
      {
        if(tempLog[nI] == '\n')
          numLines++;
        else if(tempLog[nI] == '\r')
          tempLog[nI] = ' ';
      }
      strcpy(tempString, "Employee Response Form");
      pDC->TextOut(256, (printedLines * yChar) + 256, tempString, strlen(tempString));
      printedLines++;
      strcpy(tempString, "");
      pDC->TextOut(256, (printedLines * yChar) + 256, tempString, strlen(tempString));
      printedLines++;
    }

    strcpy(tempString, "Reference number: ");
    strncpy(szarString, m_pCCFE->COMPLAINTS.referenceNumber, COMPLAINTS_REFERENCENUMBER_LENGTH);
    trim(szarString, COMPLAINTS_REFERENCENUMBER_LENGTH);
    strcat(tempString, szarString);
    pDC->TextOut(256, (printedLines * yChar) + 256, tempString, strlen(tempString));
    printedLines++;
    strcpy(tempString, "");
    pDC->TextOut(256, (printedLines * yChar) + 256, tempString, strlen(tempString));
    printedLines++;

    CString strText;
    int     len;

    for(nI = 0; nI < numLines; nI++)
    {
      if(bFromControl)
      {
        len = pEditPREVIOUSLOG->LineLength(pEditPREVIOUSLOG->LineIndex(nI));
        pEditPREVIOUSLOG->GetLine(nI, strText.GetBuffer(len), len);
        strncpy(tempString, strText, len);
        tempString[len] = '\0';
        strText.ReleaseBuffer(len);
      }
      else
      {
        ptr = strtok(nI == 0 ? tempLog : NULL, "\n");
        if(ptr == NULL)
          break;
        strcpy(tempString, ptr);
      }
      pDC->TextOut(256, (printedLines * yChar) + 256, tempString, strlen(tempString));
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


void CCCProcess::PrintLetter()
{
  TEXTMETRIC tm;
  DOCINFO di;
  HDC hDCPrn;
  int yChar;
//
//  Get the Generic Letter text file
//
  CString letterFile = szInstallDirectory;
  CString inputLine;
  
  letterFile += "\\Generic Letter.txt";
  CStdioFile f(letterFile, CFile::modeRead | CFile::typeText);
  if(f == NULL)
  {
    CString s;
      
    s.LoadString(ERROR_264);
    sprintf(tempString, s, szInstallDirectory);
    MessageBeep(MB_ICONINFORMATION);
    MessageBox(tempString, TMS, MB_OK);
    return;
  }
//
//  Set up the Print dialog
//
  CPrintDialog *printDlg = new CPrintDialog(FALSE, PD_ALLPAGES | PD_RETURNDC, NULL);
//
//  Initialize some of the fields in PRINTDLG structure
//
  printDlg->m_pd.nMinPage = printDlg->m_pd.nMaxPage = 1;
  printDlg->m_pd.nFromPage = printDlg->m_pd.nToPage = 1;
//
//  Display the Windows print dialog box
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
//  Write out the customer's name
//
    int currentLine = 12;
    CUSTOMERSKey0.recordID = m_pCCFE->SNAPSHOT.CUSTOMERSrecordID;
    btrieve(B_GETEQUAL, TMS_CUSTOMERS, &CUSTOMERS, &CUSTOMERSKey0, 0);
    strncpy(tempString, CUSTOMERS.salutation, CUSTOMERS_SALUTATION_LENGTH);
    trim(tempString, CUSTOMERS_SALUTATION_LENGTH);
    strcat(tempString, " ");
    strncpy(szarString, CUSTOMERS.firstName, CUSTOMERS_FIRSTNAME_LENGTH);
    trim(szarString, CUSTOMERS_FIRSTNAME_LENGTH);
    strcat(tempString, szarString);
    strcat(tempString, " ");
    strncpy(szarString, CUSTOMERS.lastName, CUSTOMERS_LASTNAME_LENGTH);
    trim(szarString, CUSTOMERS_LASTNAME_LENGTH);
    strcat(tempString, szarString);
    pDC->TextOut(256, (yChar * currentLine++) + 256, tempString, strlen(tempString));
//
//  Address
//
    strncpy(tempString, CUSTOMERS.streetAddress, CUSTOMERS_STREETADDRESS_LENGTH);
    trim(tempString, CUSTOMERS_STREETADDRESS_LENGTH);
    pDC->TextOut(256, (yChar * currentLine++) + 256, tempString, strlen(tempString));
//
//  City, State
//
    strncpy(tempString, CUSTOMERS.city, CUSTOMERS_CITY_LENGTH);
    trim(tempString, CUSTOMERS_CITY_LENGTH);
    strcat(tempString, ", ");
    strncpy(szarString, CUSTOMERS.state, CUSTOMERS_STATE_LENGTH);
    trim(szarString, CUSTOMERS_STATE_LENGTH);
    strcat(tempString, szarString);
    pDC->TextOut(256, (yChar * currentLine++) + 256, tempString, strlen(tempString));
//
//  Zip
//
    strncpy(tempString, CUSTOMERS.ZIP, CUSTOMERS_ZIP_LENGTH);
    trim(tempString, CUSTOMERS_ZIP_LENGTH);
    pDC->TextOut(256, (yChar * currentLine++) + 256, tempString, strlen(tempString));
//
//  Today's date
//
    currentLine++;
    CTime time = CTime::GetCurrentTime();
    CString s;
    s.LoadString(TEXT_104 + time.GetMonth());
    sprintf(tempString, "%s %d, %d", s, time.GetDay(), time.GetYear());
    pDC->TextOut(256, (yChar * currentLine++) + 256, tempString, strlen(tempString));
//
//  Reference number
//
    currentLine++;
    strncpy(szarString, m_pCCFE->COMPLAINTS.referenceNumber, COMPLAINTS_REFERENCENUMBER_LENGTH);
    trim(szarString, COMPLAINTS_REFERENCENUMBER_LENGTH);
    sprintf(tempString, "Re: Our reference number %s", szarString);
    pDC->TextOut(256, (yChar * currentLine++) + 256, tempString, strlen(tempString));
//
//  Dear...
//
    currentLine += 2;
    strcpy(tempString, "Dear ");
    strncpy(szarString, CUSTOMERS.salutation, CUSTOMERS_SALUTATION_LENGTH);
    trim(szarString, CUSTOMERS_SALUTATION_LENGTH);
    strcat(tempString, szarString);
    strcat(tempString, " ");
    strncpy(szarString, CUSTOMERS.lastName, CUSTOMERS_LASTNAME_LENGTH);
    trim(szarString, CUSTOMERS_LASTNAME_LENGTH);
    strcat(tempString, szarString);
    strcat(tempString, ",");
    pDC->TextOut(256, (yChar * currentLine++) + 256, tempString);
//
//  Get the letter
//
    currentLine++;
    while(f.ReadString(inputLine))
    {
      if(strlen(inputLine) == 0 || inputLine[0] != '#')
        pDC->TextOut(256, (yChar * currentLine++) + 256, inputLine);
    }
//
//  Done
//
    f.Close();
    pDC->EndPage();    
    pDC->EndDoc();     
    pDC->Detach();     
    delete pDC;
  }

  delete printDlg;
}

void CCCProcess::OnDblclkTypical() 
{
  CString s;
  int  nI;

  nI = pListBoxTYPICAL->GetCurSel();
  pListBoxTYPICAL->GetText(nI, s);
  s += "\r\n";
  pEditCURRENTLOG->ReplaceSel(s, TRUE);
}


void CCCProcess::OnDblclkList(NMHDR* pNMHDR, LRESULT* pResult) 
{
  OnOK();
  	
	*pResult = 0;
}

void CCCProcess::OnBack()
{
  CString s;

  pEditCURRENTLOG->GetWindowText(m_pCurrentLog, COMPLAINTS_VARIABLE_LENGTH);

  CDialog::EndDialog(IDBACK);
}

void CCCProcess::PrintForReview()
{
  int  flags;

  char saveLog[COMPLAINTS_VARIABLE_LENGTH];
//
//  Save the "previous" log a set up a new one.  That way, none of the
//  edits made to the original record show up for the employee.
//  
  strcpy(saveLog, m_previousLog);
  flags =  CCPROCESS_LOGFLAGS_ALL;
  flags &= ~CCPROCESS_LOGFLAGS_CUSTOMER;
  flags &= ~CCPROCESS_LOGFLAGS_DESCRIPTION;
  SetupLog(flags);
//
//  Append to "previous"
//
  CString s;

  s.LoadString(TEXT_202);
  strcat(m_previousLog, s);
  pEditCURRENTLOG->GetWindowText(m_pCurrentLog, COMPLAINTS_VARIABLE_LENGTH);
  strcat(m_previousLog, m_pCurrentLog);
//
//  Print the log for the employee
//
  PrintLog(FALSE);
//
//  Restore the "Previous" log
//
  strcpy(m_previousLog, saveLog);
}

void CCCProcess::SetupLog(int flags)
{
  CString s;
  long year;
  long month;
  long day;
  long hours;
  long minutes;
  long seconds;
  int  nI;

  strcpy(m_previousLog, "");
//
//  Slowly I turn, step by step, closer and closer...
//
//  This is a...
//
  if(m_pCCFE->flags & CCFE_FLAGS_COMPLAINT)
    nI = TEXT_197;
  else if(m_pCCFE->flags & CCFE_FLAGS_COMMENDATION)
    nI = TEXT_198;
  else if(m_pCCFE->flags & CCFE_FLAGS_SERVICEREQUEST)
    nI = TEXT_199;
  else
    nI = TEXT_200;
  s.LoadString(nI);
  strcat(m_previousLog, s);
//
//  This call was taken by...
//
  if(flags & CCPROCESS_LOGFLAGS_TAKENBY)
  {
    s.LoadString(TEXT_162);
    sprintf(tempString, s, m_pCCFE->szReceivedBy, m_pCCFE->szDateTime);
    strcat(m_previousLog, tempString);
  }
//
//  ...and is from:
//
  if(flags & CCPROCESS_LOGFLAGS_CUSTOMER)
  {
    if(m_pCCFE->SNAPSHOT.CUSTOMERSrecordID == NO_RECORD)
    {
      s.LoadString(TEXT_201);
      strcat(m_previousLog, s);
    }
    else
    {
      CUSTOMERSKey0.recordID = m_pCCFE->SNAPSHOT.CUSTOMERSrecordID;
      btrieve(B_GETEQUAL, TMS_CUSTOMERS, &CUSTOMERS, &CUSTOMERSKey0, 0);
      strncpy(tempString, CUSTOMERS.salutation, CUSTOMERS_SALUTATION_LENGTH);
      trim(tempString, CUSTOMERS_SALUTATION_LENGTH);
      strcat(m_previousLog, tempString);
      strcat(m_previousLog, " ");
      strncpy(tempString, CUSTOMERS.firstName, CUSTOMERS_FIRSTNAME_LENGTH);
      trim(tempString, CUSTOMERS_FIRSTNAME_LENGTH);
      strcat(m_previousLog, tempString);
      strcat(m_previousLog, " ");
      strncpy(tempString, CUSTOMERS.lastName, CUSTOMERS_LASTNAME_LENGTH);
      trim(tempString, CUSTOMERS_LASTNAME_LENGTH);
      strcat(m_previousLog, tempString);
      strcat(m_previousLog, "\r\n");
      strncpy(tempString, CUSTOMERS.streetAddress, CUSTOMERS_STREETADDRESS_LENGTH);
      trim(tempString, CUSTOMERS_STREETADDRESS_LENGTH);
      strcat(m_previousLog, tempString);
      strcat(m_previousLog, "\r\n");
      strncpy(tempString, CUSTOMERS.city, CUSTOMERS_CITY_LENGTH);
      trim(tempString, CUSTOMERS_CITY_LENGTH);
      strcat(m_previousLog, tempString);
      strcat(m_previousLog, ", ");
      strncpy(tempString, CUSTOMERS.state, CUSTOMERS_STATE_LENGTH);
      trim(tempString, CUSTOMERS_STATE_LENGTH);
      strcat(m_previousLog, tempString);
      strcat(m_previousLog, "\r\n");
      strncpy(tempString, CUSTOMERS.ZIP, CUSTOMERS_ZIP_LENGTH);
      trim(tempString, CUSTOMERS_ZIP_LENGTH);
      strcat(m_previousLog, tempString);
      strcat(m_previousLog, "\r\n\r\n");
      s.LoadString(TEXT_163);
      strcat(m_previousLog, s);
      strcat(m_previousLog, " ");
      strcpy(tempString,
            PhoneString(CUSTOMERS.dayTelephoneArea, CUSTOMERS.dayTelephoneNumber));
      strcat(m_previousLog, tempString);
      strcat(m_previousLog, "\r\n");
      s.LoadString(TEXT_164);
      strcat(m_previousLog, s);
      strcat(m_previousLog, " ");
      strcpy(tempString,
            PhoneString(CUSTOMERS.nightTelephoneArea, CUSTOMERS.nightTelephoneNumber));
      strcat(m_previousLog, tempString);
      strcat(m_previousLog, "\r\n");
      s.LoadString(TEXT_165);
      strcat(m_previousLog, s);
      strcat(m_previousLog, " ");
      strcpy(tempString,
            PhoneString(CUSTOMERS.faxTelephoneArea, CUSTOMERS.faxTelephoneNumber));
      strcat(m_previousLog, tempString);
      strcat(m_previousLog, "\r\n");
      s.LoadString(TEXT_166);
      strcat(m_previousLog, s);
      strcat(m_previousLog, " ");
      strncpy(tempString, CUSTOMERS.emailAddress, CUSTOMERS_EMAILADDRESS_LENGTH);
      trim(tempString, CUSTOMERS_EMAILADDRESS_LENGTH);
      strcat(m_previousLog, tempString);
      strcat(m_previousLog, "\r\n\r\n");
    }
  }
//
//  History
//
  if(flags & CCPROCESS_LOGFLAGS_HISTORY)
  {
    s.LoadString(TEXT_167);
    strcat(m_previousLog, s);
    if(!(m_pCCFE->SNAPSHOT.flags & COMPLAINTS_FLAG_FILEDPREVIOUS))
    {
      s.LoadString(TEXT_168);
    }
    else
    {
      if(m_pCCFE->SNAPSHOT.flags & COMPLAINTS_FLAG_NOTSUREHOWMANY)
        s.LoadString(TEXT_169);
      else
      {
        if(m_pCCFE->SNAPSHOT.numberOfPreviousComplaints == 1)
        {
          s.LoadString(TEXT_207);
          strcpy(szarString, s);
        }
        else
        {
          s.LoadString(TEXT_208);
          strcpy(szFormatString, s);
          sprintf(szarString, szFormatString, m_pCCFE->SNAPSHOT.numberOfPreviousComplaints);
        }
        s.LoadString(TEXT_170);
        strcpy(szFormatString, s);
        sprintf(tempString, szFormatString, szarString);
        s = tempString;
      }
    }
    strcat(m_previousLog, s);
  }
//
//  Details
//
  s.LoadString(TEXT_171);
  strcat(m_previousLog, s);
//
//  Date and time
//
  if(flags & CCPROCESS_LOGFLAGS_DATETIME)
  {
    s.LoadString(TEXT_172);
    strcat(m_previousLog, s);
    nI = m_pCCFE->SNAPSHOT.dateOfOccurrence;
    GetYMD((long)nI, &year, &month, &day);
    nI = m_pCCFE->SNAPSHOT.timeOfOccurrence;
    GetHMS((long)nI, &hours, &minutes, &seconds);
    CTime   t(year, month, day, hours, minutes, seconds);
 	  s = t.Format(_T("%A, %B %d, %Y at %I:%M:%S%p"));
    strcat(m_previousLog, s);
    strcat(m_previousLog, "\r\n");
  }
//
//  Weather
//
  if(flags & CCPROCESS_LOGFLAGS_WEATHER)
  {
    s.LoadString(TEXT_239);
    strcat(m_previousLog, s);
    strcat(m_previousLog, m_pCCFE->szWeather);
    strcat(m_previousLog, "\r\n");
  }
//
//  Bus number
//
  if(flags & CCPROCESS_LOGFLAGS_BUSNUMBER)
  {
    s.LoadString(TEXT_174);
    strcat(m_previousLog, s);
    strncpy(tempString, m_pCCFE->SNAPSHOT.busNumber, COMPLAINTS_BUSNUMBER_LENGTH);
    trim(tempString, COMPLAINTS_BUSNUMBER_LENGTH);
    if(strcmp(tempString, "") == 0)
    {
      s.LoadString(TEXT_173);
      strcat(m_previousLog, s);
    }
    else
    {
      strcat(m_previousLog, tempString);
    }
    strcat(m_previousLog, "\r\n");
  }
//
//  Location
//
  if(flags & CCPROCESS_LOGFLAGS_LOCATION)
  {
    s.LoadString(TEXT_175);
    strcat(m_previousLog, s);
    strncpy(tempString, m_pCCFE->SNAPSHOT.location, COMPLAINTS_LOCATION_LENGTH);
    trim(tempString, COMPLAINTS_LOCATION_LENGTH);
    if(strcmp(tempString, "") == 0)
    {
      s.LoadString(TEXT_173);
      strcat(m_previousLog, s);
    }
    else
    {
      strcat(m_previousLog, tempString);
    }
    strcat(m_previousLog, "\r\n");
  }
//
//  Route
//
  if(flags & CCPROCESS_LOGFLAGS_ROUTE)
  {
    s.LoadString(TEXT_177);
    strcat(m_previousLog, s);
    if(m_pCCFE->SNAPSHOT.ROUTESrecordID == NO_RECORD)
    {
      s.LoadString(TEXT_173);
      strcat(m_previousLog, s);
    }
    else
    {
      ROUTESKey0.recordID = m_pCCFE->SNAPSHOT.ROUTESrecordID;
      btrieve(B_GETEQUAL, TMS_ROUTES, &ROUTES, &ROUTESKey0, 0);
      strncpy(tempString, ROUTES.number, ROUTES_NUMBER_LENGTH);
      trim(tempString, ROUTES_NUMBER_LENGTH);
      strcat(m_previousLog, tempString);
      strcat(m_previousLog, " - ");
      strncpy(tempString, ROUTES.name, ROUTES_NAME_LENGTH);
      trim(tempString, ROUTES_NAME_LENGTH);
      strcat(m_previousLog, tempString);
    }
    strcat(m_previousLog, "\r\n");
//
//  Direction
//
    s.LoadString(TEXT_178);
    strcat(m_previousLog, s);
    if(m_pCCFE->SNAPSHOT.ROUTESrecordID == NO_RECORD ||
          m_pCCFE->SNAPSHOT.directionIndex == NO_RECORD)
    {
      s.LoadString(TEXT_173);
      strcat(m_previousLog, s);
    }
    else
    {
      DIRECTIONSKey0.recordID = ROUTES.DIRECTIONSrecordID[m_pCCFE->SNAPSHOT.directionIndex];
      btrieve(B_GETEQUAL, TMS_DIRECTIONS, &DIRECTIONS, &DIRECTIONSKey0, 0);
      strncpy(tempString, DIRECTIONS.longName, DIRECTIONS_LONGNAME_LENGTH); 
      trim(tempString, DIRECTIONS_LONGNAME_LENGTH);
      strcat(m_previousLog, tempString); 
    }
    strcat(m_previousLog, "\r\n\r\n");
  }
//
//  Employee information
//
  s.LoadString(TEXT_179);
  strcat(m_previousLog, s);
//
//  Sex: (woo-hoo!)
//
  if(flags & CCPROCESS_LOGFLAGS_SEX)
  {
    if(m_pCCFE->SNAPSHOT.flags & COMPLAINTS_FLAG_NOTSUREDRIVERSEX)
    {
      s.LoadString(TEXT_182);
    }
    else
    {
      if(m_pCCFE->SNAPSHOT.flags & COMPLAINTS_FLAG_DRIVERMALE)
        s.LoadString(TEXT_180);
      else
        s.LoadString(TEXT_181);
    }
    strcat(m_previousLog, s);
    strcat(m_previousLog, "\r\n");
  }
//
//  Description
//
  if(flags & CCPROCESS_LOGFLAGS_DESCRIPTION)
  {
    s.LoadString(TEXT_183);
    strcat(m_previousLog, s);
    strncpy(tempString, m_pCCFE->SNAPSHOT.driverDescription, COMPLAINTS_DRIVERDESCRIPTION_LENGTH);
    trim(tempString, COMPLAINTS_DRIVERDESCRIPTION_LENGTH);
    if(strcmp(tempString, "") == 0)
    {
      s.LoadString(TEXT_173);
      strcat(m_previousLog, s);
    }
    else
    {
      strcat(m_previousLog, tempString);
    }
    strcat(m_previousLog, "\r\n");
  }
//
//  Name
//
  if(flags & CCPROCESS_LOGFLAGS_DRIVERNAME)
  {
    s.LoadString(TEXT_184);
    strcat(m_previousLog, s);
    if(m_pCCFE->SNAPSHOT.DRIVERSrecordID == NO_RECORD)
    {
      s.LoadString(TEXT_173);
      strcat(m_previousLog, s);
    }
    else
    {
      DRIVERSKey0.recordID = m_pCCFE->SNAPSHOT.DRIVERSrecordID;
      btrieve(B_GETEQUAL, TMS_DRIVERS, &DRIVERS, &DRIVERSKey0, 0);
      strncpy(tempString, DRIVERS.lastName, DRIVERS_LASTNAME_LENGTH);
      trim(tempString, DRIVERS_LASTNAME_LENGTH);
      strcat(m_previousLog, tempString);
      strcat(m_previousLog, ", ");
      strncpy(tempString, DRIVERS.firstName, DRIVERS_FIRSTNAME_LENGTH);
      trim(tempString, DRIVERS_FIRSTNAME_LENGTH);
      strcat(m_previousLog, tempString);
    } 
    strcat(m_previousLog, "\r\n\r\n");
  }
//
//  Misses and running hot
//
  s.LoadString(TEXT_185);
  strcat(m_previousLog, s);
//
//  Speeding?
//
  if(flags & CCPROCESS_LOGFLAGS_SPEEDING)
  {
    if(m_pCCFE->SNAPSHOT.flags & COMPLAINTS_FLAG_NASPEEDING)
    {
      s.LoadString(TEXT_205);
    }
    else
    {
      if(m_pCCFE->SNAPSHOT.flags & COMPLAINTS_FLAG_DRIVERSPEEDINGYES)
      {
        if(m_pCCFE->SNAPSHOT.driverSpeed == 0)
          s.LoadString(TEXT_188);
        else
        {
          strcpy(tempString, s);
          s.LoadString(TEXT_189);
          sprintf(tempString, s, m_pCCFE->SNAPSHOT.driverSpeed);
          s.LoadString((distanceMeasure == PREFERENCES_KILOMETERS) ? TEXT_203 : TEXT_204);
          strcat(tempString, s);
          s = tempString;
        }
      }
      else
      {
        s.LoadString(TEXT_187);
      }
    }
    s += "\r\n";
    strcat(m_previousLog, s);
  }
//
//  Driver's attention
//
  if(flags & CCPROCESS_LOGFLAGS_ATTENTION)
  {
    if(m_pCCFE->SNAPSHOT.flags & COMPLAINTS_FLAG_NAATTENTION)
      s.LoadString(TEXT_206);
    else
      s.LoadString((m_pCCFE->SNAPSHOT.flags & COMPLAINTS_FLAG_ATTENTIONYES) ? TEXT_191 : TEXT_190);
    strcat(m_previousLog, s);
  }
//
//  Time difference
//
  if(flags & CCPROCESS_LOGFLAGS_TIMEDIFFERENCE)
  {
    s.LoadString(TEXT_192);
    strcat(m_previousLog, s);
    if(m_pCCFE->SNAPSHOT.flags & COMPLAINTS_FLAG_LESSTHANTWO)
      s.LoadString(TEXT_193);
    else if(m_pCCFE->SNAPSHOT.flags & COMPLAINTS_FLAG_TWOTOFIVE)
      s.LoadString(TEXT_194);
    else if(m_pCCFE->SNAPSHOT.flags & COMPLAINTS_FLAG_MORETHANFIVE)
      s.LoadString(TEXT_195);
    else
    {
      s.LoadString(TEXT_026);
      s += "\r\n\r\n";
    }
    strcat(m_previousLog, s);
  }
}

void CCCProcess::OnSupervisor() 
{
  CCCLogin login;

  if(login.DoModal() == IDOK)
  {
    COMPLAINTSDef c;
    SaveRecord();

    memcpy(&c, &COMPLAINTS, sizeof(COMPLAINTSDef)); // COMPLAINTS created/updated in SaveRecord()
    CCCSupervisor dlg(this, &c, m_pCCFE);

    if(dlg.DoModal() == IDOK)
    	CDialog::OnOK();
  }
}

void::CCCProcess::SaveRecord()
{
  CCREFERDef *p;
  int  nI;
  int  nJ;
    
//
//  Get the record
//
  COMPLAINTSKey0.recordID = m_pCCFE->COMPLAINTS.recordID;
  recordLength[TMS_COMPLAINTS] = COMPLAINTS_TOTAL_LENGTH;
  btrieve(B_GETEQUAL, TMS_COMPLAINTS, pComplaintText, &COMPLAINTSKey0, 0);
  memcpy(&COMPLAINTS, pComplaintText, COMPLAINTS_FIXED_LENGTH);
//
//  Save the current log entry
//
  pEditCURRENTLOG->GetWindowText(m_pCurrentLog, COMPLAINTS_VARIABLE_LENGTH);
//
//  Append to the old
//
  CString s = m_previousLog;
  
  if(strcmp(m_pCurrentLog, "") != 0)
  {
    CString t;
    t.LoadString(TEXT_202);
    s += t;
    s += m_pCurrentLog;
  }
  strcpy(m_previousLog, s);
  pEditPREVIOUSLOG->SetWindowText(m_previousLog);
//
//  See if we have to print a letter
//
  if(pButtonPRINTLETTER->GetCheck())
  {
    PrintLetter();
  }
//
//  See if we have to print the log
//
  if(pButtonPRINTLOG->GetCheck())
  {
    PrintLog(TRUE);
  }
//
//  See if we have to print the log for employee review
//
  if(pButtonPRINTFORREVIEW->GetCheck())
  {
    PrintForReview();
  }
//
//  See if we have to email someone
//
  if(pButtonEMAIL->GetCheck())
  {
  	nI = pListCtrlLIST->GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);
    if(nI >= 0)
    {
      CIMapi mail;
      while(nI >= 0)
      {
        nJ = pListCtrlLIST->GetItemData(nI);
        p = (CCREFERDef *)nJ;
        p->flags = CCREFER_FLAGS_SELECTED;
        mail.To(p->szEmailAddress);
        nI = pListCtrlLIST->GetNextItem(nI, LVNI_ALL | LVNI_SELECTED);
      }
      strcpy(tempString, "TMS Customer Comment #");
      strncpy(szarString, COMPLAINTS.referenceNumber, COMPLAINTS_REFERENCENUMBER_LENGTH);
      trim(szarString, COMPLAINTS_REFERENCENUMBER_LENGTH);
      strcat(tempString, szarString);
      mail.Subject(tempString);
      mail.Text((char *)LPCTSTR(s));
      mail.Send();
    }
  }
//
//  Save everything worth saving
//
//  COMMENTS record ID
//
  COMPLAINTS.COMMENTSrecordID = m_pCCFE->COMPLAINTS.COMMENTSrecordID;
//
//  CUSTOMERS record ID
//
  COMPLAINTS.CUSTOMERSrecordID = m_pCCFE->SNAPSHOT.CUSTOMERSrecordID;
//
//  Number of previous complaints
//
  COMPLAINTS.numberOfPreviousComplaints = m_pCCFE->SNAPSHOT.numberOfPreviousComplaints;
//
//  Date of occurrence
//
  COMPLAINTS.dateOfOccurrence = m_pCCFE->SNAPSHOT.dateOfOccurrence;
//
//  Time of occurrence
//
  COMPLAINTS.timeOfOccurrence = m_pCCFE->SNAPSHOT.timeOfOccurrence;
//
//  Bus number
//
  strncpy(COMPLAINTS.busNumber, m_pCCFE->SNAPSHOT.busNumber, COMPLAINTS_BUSNUMBER_LENGTH);
//
//  ROUTES recordID
//
  COMPLAINTS.ROUTESrecordID = m_pCCFE->SNAPSHOT.ROUTESrecordID;
//
//  Direction
//
  COMPLAINTS.directionIndex = m_pCCFE->SNAPSHOT.directionIndex;
//
//  Location
//
  strncpy(COMPLAINTS.location, m_pCCFE->SNAPSHOT.location, COMPLAINTS_LOCATION_LENGTH);
//
//  DRIVERS recordID
//
  COMPLAINTS.DRIVERSrecordID = m_pCCFE->SNAPSHOT.DRIVERSrecordID;
//
//  Driver description
//
  strncpy(COMPLAINTS.driverDescription,
        m_pCCFE->SNAPSHOT.driverDescription, COMPLAINTS_DRIVERDESCRIPTION_LENGTH);
//
//  Driver speed
//
  COMPLAINTS.driverSpeed = m_pCCFE->SNAPSHOT.driverSpeed;
//
//  Weather index
//
  COMPLAINTS.weatherIndex = m_pCCFE->SNAPSHOT.weatherIndex;
//
//  Typical index
//
  COMPLAINTS.typicalIndex = m_pCCFE->SNAPSHOT.typicalIndex;
//
//  Flags
//
  COMPLAINTS.flags = m_pCCFE->SNAPSHOT.flags;
  if(pButtonPRINTLETTER->GetCheck())
    COMPLAINTS.flags |= COMPLAINTS_FLAG_LETTERSENT;
//
//  Copy to the structure
//
  memcpy(pComplaintText, &COMPLAINTS, COMPLAINTS_FIXED_LENGTH);
//
//  Add the text
//
  strcpy(&pComplaintText[COMPLAINTS_FIXED_LENGTH], s);  // See above
//
//  Update the record
//
  recordLength[TMS_COMPLAINTS] = COMPLAINTS_TOTAL_LENGTH;
  int rcode2 = btrieve(B_UPDATE, TMS_COMPLAINTS, pComplaintText, &COMPLAINTSKey0, 0);
  recordLength[TMS_COMPLAINTS] = COMPLAINTS_FIXED_LENGTH;
}

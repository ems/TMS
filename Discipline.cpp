//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
// Discipline.cpp : implementation file
//

#include "stdafx.h"
extern "C"{
#include "TMSHeader.h"
}  // EXTERN C

#include "TMS.h"

#include "DTPicker.h"
#include "AddDiscipline.h"
#include "Discipline.h"
#include "DisciplineLetter.h"
#include "DisciplineLetterTemplate.h"
#include "SuspensionPending.h"
#include "DriversToConsider.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDiscipline dialog


CDiscipline::CDiscipline(CWnd* pParent /*=NULL*/)
	: CDialog(CDiscipline::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDiscipline)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CDiscipline::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDiscipline)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDiscipline, CDialog)
	//{{AFX_MSG_MAP(CDiscipline)
	ON_NOTIFY(NM_CLICK, DISCIPLINE_HISTORY, OnClickHistory)
	ON_BN_CLICKED(IDNEW, OnNew)
	ON_BN_CLICKED(IDUPDATE, OnUpdate)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDHELP, OnHelp)
	ON_CBN_SELENDOK(DISCIPLINE_DRIVER, OnSelendokDriver)
	ON_NOTIFY(NM_DBLCLK, DISCIPLINE_HISTORY, OnDblclkHistory)
	ON_BN_CLICKED(IDDELETE, OnDelete)
	ON_BN_CLICKED(IDREPORT, OnReport)
	ON_BN_CLICKED(IDCONSIDER, OnConsider)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDiscipline message handlers

BOOL CDiscipline::OnInitDialog() 
{
	CDialog::OnInitDialog();
//
//  Set up the handles to the controls
//
  pComboBoxDRIVER = (CComboBox *)GetDlgItem(DISCIPLINE_DRIVER);
  pListCtrlHISTORY = (CListCtrl *)GetDlgItem(DISCIPLINE_HISTORY);
  pButtonNEW = (CButton *)GetDlgItem(IDNEW);
  pButtonUPDATE = (CButton *)GetDlgItem(IDUPDATE);
  pButtonDELETE = (CButton *)GetDlgItem(IDDELETE);
  pButtonREPORT = (CButton *)GetDlgItem(IDREPORT);
//
//  Load the drivers
//
  SetupDriverComboBox();
//
//  Set up the history list control
//
  DWORD dwExStyles = pListCtrlHISTORY->GetExtendedStyle();
  pListCtrlHISTORY->SetExtendedStyle(dwExStyles | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_INFOTIP);
//
//  Set up the list controls
//
  LVCOLUMN LVC;
  LVC.mask = LVCF_FMT | LVCF_TEXT | LVCF_WIDTH;
  LVC.fmt = LVCFMT_LEFT;

  LVC.cx = 80;
  LVC.pszText = "Date";
  pListCtrlHISTORY->InsertColumn(0, &LVC);

  LVC.cx = 70;
  LVC.pszText = "Category";
  pListCtrlHISTORY->InsertColumn(1, &LVC);
 
  LVC.cx = 70;
  LVC.pszText = "Type";
  pListCtrlHISTORY->InsertColumn(2, &LVC);
 
  LVC.cx = 125;
  LVC.pszText = "Action";
  pListCtrlHISTORY->InsertColumn(3, &LVC);

  LVC.cx = 200;
  LVC.pszText = "Comments";
  pListCtrlHISTORY->InsertColumn(4, &LVC);

  LVC.cx = 80;
  LVC.pszText = "Suspended";
  pListCtrlHISTORY->InsertColumn(5, &LVC);

  LVC.cx = 48;
  LVC.pszText = "For";
  pListCtrlHISTORY->InsertColumn(6, &LVC);

  LVC.cx = 50;
  LVC.pszText = "Active";
  pListCtrlHISTORY->InsertColumn(7, &LVC);
//
//  Disable the buttons until a selection is made
//
  pButtonNEW->EnableWindow(FALSE);
  pButtonUPDATE->EnableWindow(FALSE);
  pButtonDELETE->EnableWindow(FALSE);
  pButtonREPORT->EnableWindow(FALSE);
//
//  Allocate space for the recordIDs for deferred suspensions / absence issues
//
  long *pRecordIDs = NULL;
  long  maxRecordIDs = 100;

  pRecordIDs = (long *)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(long) * maxRecordIDs); 
  if(pRecordIDs == NULL)
  {
    AllocationError(__FILE__, __LINE__, FALSE);
    return TRUE;
  }
//
//  See if there are any deferred suspensions
//
  int  rcode2;
  int  numDeferred = 0;

  rcode2 = btrieve(B_GETFIRST, TMS_DISCIPLINE, &DISCIPLINE, &DISCIPLINEKey1, 1);
  while(rcode2 == 0)
  {
    if(DISCIPLINE.flags & DISCIPLINE_FLAG_DEFERSUSPENSION)
    {
      pRecordIDs[numDeferred] = DISCIPLINE.recordID;
      numDeferred++;
      if(numDeferred >= maxRecordIDs)
      {
        maxRecordIDs += 100;
        pRecordIDs = (long *)HeapReAlloc(GetProcessHeap(),
              HEAP_ZERO_MEMORY, pRecordIDs, sizeof(long) * maxRecordIDs); 
        if(pRecordIDs == NULL)
        {
          AllocationError(__FILE__, __LINE__, TRUE);
          return TRUE;
        }
      }
    }
    rcode2 = btrieve(B_GETNEXT, TMS_DISCIPLINE, &DISCIPLINE, &DISCIPLINEKey1, 1);
  }
  CString fmt;

  MessageBeep(MB_ICONINFORMATION);
  if(numDeferred == 0)
  {
    fmt.LoadString(TEXT_322);
    MessageBox(fmt, TMS, MB_OK);
  }
  else
  {
    if(numDeferred == 1)
    {
      LoadString(hInst, TEXT_329, tempString, TEMPSTRING_LENGTH);
    }
    else
    {
      fmt.LoadString(TEXT_321);
      sprintf(tempString, fmt, numDeferred);
    }
    if(MessageBox(tempString, TMS, MB_YESNO) == IDYES)
    {
      CSuspensionPending dlg(this, TRUE, numDeferred, pRecordIDs);

      dlg.DoModal();
    }
  }
//
//  Look in the audit trail to see if anyone needs processing
//
  CTime time = CTime::GetCurrentTime();
  long day = time.GetDay();
  long month = time.GetMonth();
  long year = time.GetYear();
  long stopAtDate = year * 10000 + month * 100 + day;
  int  numPending = 0;
  long startDate = ((year - 1) * 10000) + (month * 100) + day;

  DAILYOPSKey1.recordTypeFlag = DAILYOPS_FLAG_ABSENCE;
  DAILYOPSKey1.pertainsToDate = startDate;
  DAILYOPSKey1.pertainsToTime = NO_TIME;
  DAILYOPSKey1.recordFlags = 0;
  rcode2 = btrieve(B_GETGREATER, TMS_DAILYOPS, &DAILYOPS, &DAILYOPSKey1, 1);
  while(rcode2 == 0 &&
        (DAILYOPS.recordTypeFlag & DAILYOPS_FLAG_ABSENCE) &&
        (DAILYOPS.pertainsToDate <= stopAtDate))
  {
//
//  Examine just those records with the date in question
//
    if((DAILYOPS.recordFlags & DAILYOPS_FLAG_ABSENCETODISCIPLINE) &&
          (DAILYOPS.recordFlags & DAILYOPS_FLAG_ABSENCEREGISTER))
    {
//
//  And ony those that are still outstanding
//
      if(!(DAILYOPS.recordFlags & DAILYOPS_FLAG_DISCIPLINEMETED))
      {
        if(!ANegatedRecord(DAILYOPS.recordID, 1))
        {
          pRecordIDs[numPending] = DAILYOPS.recordID;
          numPending++;
          if(numPending >= maxRecordIDs)
          {
            maxRecordIDs += 100;
            pRecordIDs = (long *)HeapReAlloc(GetProcessHeap(),
                  HEAP_ZERO_MEMORY, pRecordIDs, sizeof(long) * maxRecordIDs); 
            if(pRecordIDs == NULL)
            {
              AllocationError(__FILE__, __LINE__, TRUE);
              return TRUE;
            }
          }
        }
      }
    }
    rcode2 = btrieve(B_GETNEXT, TMS_DAILYOPS, &DAILYOPS, &DAILYOPSKey1, 1);
  }
  MessageBeep(MB_ICONINFORMATION);
  if(numPending == 0)
  {
    fmt.LoadString(TEXT_325);
    MessageBox(fmt, TMS, MB_OK);
  }
  else
  {
    if(numPending == 1)
    {
      LoadString(hInst, TEXT_328, tempString, TEMPSTRING_LENGTH);
    }
    else
    {
      fmt.LoadString(TEXT_326);
      sprintf(tempString, fmt, numPending);
    }
    if(MessageBox(tempString, TMS, MB_YESNO) == IDYES)
    {
      CSuspensionPending dlg(this, FALSE, numPending, pRecordIDs);

      dlg.DoModal();
    }
  }
  TMSHeapFree(pRecordIDs);
//
//  Give us back mouse control
//  
  ReleaseCapture();
	
  return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CDiscipline::OnClickHistory(NMHDR* pNMHDR, LRESULT* pResult) 
{
  pButtonUPDATE->EnableWindow(TRUE);
  pButtonDELETE->EnableWindow(TRUE);
	
	*pResult = 0;
}

void CDiscipline::OnNew() 
{
  int nI = pComboBoxDRIVER->GetCurSel();

  if(nI != CB_ERR)
  {
    m_DRIVERSrecordID = pComboBoxDRIVER->GetItemData(nI);
    long updateRecordID = NO_RECORD;
    
    CAddDiscipline dlg(this, &m_DRIVERSrecordID, &updateRecordID);
    if(dlg.DoModal() == IDOK)
    {
      DisplayHistory();
    }
  }
}

void CDiscipline::OnSelendokDriver() 
{
  int nI;

  pListCtrlHISTORY->DeleteAllItems();

  nI = pComboBoxDRIVER->GetCurSel();
  if(nI != CB_ERR)
  {
    m_DRIVERSrecordID = pComboBoxDRIVER->GetItemData(nI);
    DisplayHistory();
  }

  pButtonNEW->EnableWindow(TRUE);
  pButtonREPORT->EnableWindow(TRUE);
}

void CDiscipline::OnUpdate() 
{
	int nI = pListCtrlHISTORY->GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);

  if(nI < 0)
    return;

  LVITEM LVI;
 
  LVI.mask = LVIF_PARAM;
  LVI.iItem = nI;
  LVI.iSubItem = 0;
  pListCtrlHISTORY->GetItem(&LVI);

  long DISCIPLINErecordID = LVI.lParam;

  nI = pComboBoxDRIVER->GetCurSel();
  if(nI != CB_ERR)
  {
    m_DRIVERSrecordID = pComboBoxDRIVER->GetItemData(nI);
    
    CAddDiscipline dlg(this, &m_DRIVERSrecordID, &DISCIPLINErecordID);
    if(dlg.DoModal() == IDOK)
    {
      DisplayHistory();
    }
  }
}

void CDiscipline::OnDblclkHistory(NMHDR* pNMHDR, LRESULT* pResult) 
{
  OnUpdate();

	*pResult = 0;
}

void CDiscipline::OnDelete() 
{
  CString s;

  s.LoadString(ERROR_340);
  MessageBeep(MB_ICONQUESTION);
  if(MessageBox(s, TMS, MB_YESNO | MB_ICONQUESTION) != IDYES)
  {
    return;
  }

	int nI = pListCtrlHISTORY->GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);

  if(nI < 0)
    return;

  LVITEM LVI;
  int    rcode2;
 
  LVI.mask = LVIF_PARAM;
  LVI.iItem = nI;
  LVI.iSubItem = 0;
  pListCtrlHISTORY->GetItem(&LVI);

  DISCIPLINEKey0.recordID = LVI.lParam;
  rcode2 = btrieve(B_GETEQUAL, TMS_DISCIPLINE, &DISCIPLINE, &DISCIPLINEKey0, 0);
  rcode2 = btrieve(B_DELETE, TMS_DISCIPLINE, &DISCIPLINE, &DISCIPLINEKey0, 0);

  pListCtrlHISTORY->DeleteItem(nI);
}

void CDiscipline::DisplayHistory()
{
  LVITEM LVI;
  CString s;
  CTime time = CTime::GetCurrentTime();
  long year, month, day;
  int rcode2;
  int itemNumber;
  int category, type, action;

  day = time.GetDay();
  month = time.GetMonth();
  year = time.GetYear();

  long today = year * 10000 + month * 100 + day;

  pListCtrlHISTORY->DeleteAllItems();

  DISCIPLINEKey1.DRIVERSrecordID = m_DRIVERSrecordID;
  DISCIPLINEKey1.dateOfOffense = NO_RECORD;
  DISCIPLINEKey1.timeOfOffense = NO_RECORD;
  rcode2 = btrieve(B_GETGREATER, TMS_DISCIPLINE, &DISCIPLINE, &DISCIPLINEKey1, 1);
  itemNumber = 0;
  while(rcode2 == 0 &&
        DISCIPLINE.DRIVERSrecordID == m_DRIVERSrecordID)
  {
//
//  Date of offense
//
    GetYMD(DISCIPLINE.dateOfOffense, &year, &month, &day);
    sprintf(tempString, "%02ld-%s-%04ld", day, szMonth[month - 1], year);
    LVI.mask = LVIF_TEXT | LVIF_PARAM;
    LVI.iItem = itemNumber;
    LVI.iSubItem = 0;
    LVI.pszText = tempString;
    LVI.lParam = DISCIPLINE.recordID;;
    LVI.iItem = pListCtrlHISTORY->InsertItem(&LVI);
//
//  Category
//
    category = DISCIPLINE.violationCategory;
    type = DISCIPLINE.violationType;
    action = DISCIPLINE.actionTaken;

    if(category != NO_RECORD)
    {
      LVI.mask = LVIF_TEXT;
      LVI.iItem = itemNumber;
      LVI.iSubItem = 1;
      LVI.pszText = m_ViolationCategories[category];
      LVI.iItem = pListCtrlHISTORY->SetItem(&LVI);
    }
//
//  Type
//
    if(category != NO_RECORD && type != NO_RECORD)
    {
      LVI.mask = LVIF_TEXT;
      LVI.iItem = itemNumber;
      LVI.iSubItem = 2;
      LVI.pszText = m_Violations[category][type].text;
      LVI.iItem = pListCtrlHISTORY->SetItem(&LVI);
    }
//
//  Action
//
    if(category != NO_RECORD && DISCIPLINE.violationType != NO_RECORD)
    {
      if(action != NO_RECORD)
      {
        LVI.mask = LVIF_TEXT;
        LVI.iItem = itemNumber;
        LVI.iSubItem = 3;
        LVI.pszText = m_ViolationActions[action];
        LVI.iItem = pListCtrlHISTORY->SetItem(&LVI);
      }
    }
//
//  Comments
//
    strncpy(tempString, DISCIPLINE.comments, DISCIPLINE_COMMENTS_LENGTH);
    trim(tempString, DISCIPLINE_COMMENTS_LENGTH);
    LVI.mask = LVIF_TEXT;
    LVI.iItem = itemNumber;
    LVI.iSubItem = 4;
    LVI.pszText = tempString;
    LVI.iItem = pListCtrlHISTORY->SetItem(&LVI);
//
//  Suspended
//
    if(DISCIPLINE.suspensionDate > 0)
    {
      GetYMD(DISCIPLINE.suspensionDate, &year, &month, &day);
      sprintf(tempString, "%02ld-%s-%04ld", day, szMonth[month - 1], year);
      LVI.mask = LVIF_TEXT;
      LVI.iItem = itemNumber;
      LVI.iSubItem = 5;
      LVI.pszText = tempString;
      LVI.iItem = pListCtrlHISTORY->SetItem(&LVI);
    }
//
//  Duration
//
    if(DISCIPLINE.suspensionDate > 0)
    {
      sprintf(tempString, "%5.2f", DISCIPLINE.suspensionDuration);
      LVI.mask = LVIF_TEXT;
      LVI.iItem = itemNumber;
      LVI.iSubItem = 6;
      LVI.pszText = tempString;
      LVI.iItem = pListCtrlHISTORY->SetItem(&LVI);
    }
//
//  Active?
//
    LVI.mask = LVIF_TEXT;
    LVI.iItem = itemNumber;
    LVI.iSubItem = 7;
    strcpy(tempString, (DISCIPLINE.dropoffDate >= today ? "Y" : "N"));
    LVI.pszText = tempString;
    LVI.iItem = pListCtrlHISTORY->SetItem(&LVI);
    
//
//  Get the next record
//
    rcode2 = btrieve(B_GETNEXT, TMS_DISCIPLINE, &DISCIPLINE, &DISCIPLINEKey1, 1);
    itemNumber++;
  }
}

void CDiscipline::OnClose() 
{
	// TODO: Add your message handler code here and/or call default
	
	CDialog::OnClose();
}

void CDiscipline::OnCancel() 
{
	// TODO: Add extra cleanup here
	
	CDialog::OnCancel();
}

void CDiscipline::OnHelp() 
{
	// TODO: Add your control notification handler code here
	
}

void CDiscipline::OnOK() 
{
	// TODO: Add extra validation here
	
	CDialog::OnOK();
}


void CDiscipline::OnReport() 
{
  TMSRPTPassedDataDef PassedData;
  CString s;
  CString sRegKey = "Software\\ODBC\\ODBC.INI\\TMS MSJet Text";
  FILE  *fp;
  DWORD  type = REG_SZ;
  char szReportName[TMSRPT_REPORTNAME_LENGTH + 1];
  char szReportFileName[TMSRPT_REPORTFILENAME_LENGTH + 1];
  HKEY hKey;
  long result;
  int  nI, nJ;
  long DRIVERSrecordID;
  int  thisReport;

  nI = pComboBoxDRIVER->GetCurSel();
  if(nI == CB_ERR)
  {
    return;
  }
  DRIVERSrecordID = pComboBoxDRIVER->GetItemData(nI);
//
//  Locate the (possible now sorted) report
//
  for(thisReport = NO_RECORD, nI = 0; nI < TMSRPT_MAX_REPORTSDEFINED; nI++)
  {
    if(TMSRPT[nI].originalReportNumber == 59)
    {
      thisReport = nI;
      break;
    }
  }
  if(thisReport == NO_RECORD)
  {
    return;
  }
//
//  Execute the report
//
  memset(&PassedData, 0x00, sizeof(PassedData));
  PassedData.DRIVERSrecordID = DRIVERSrecordID;
  if(!TMSRPT60(&PassedData))
  {
    return;
  }
  strcpy(szReportName, TMSRPT[thisReport].szReportName);
  strcpy(szReportFileName, TMSRPT[thisReport].szReportFileName[0]);
//
//  Make sure the template exists
//
  fp = fopen(szReportFileName, "r");
  if(fp == NULL)
  {
    s.LoadString(ERROR_197);
    sprintf(tempString, s, szReportFileName);
    MessageBeep(MB_ICONSTOP);
    MessageBox(tempString, TMS, MB_OK);
    return;
  }
//
//  It does exist
//
  fclose(fp);
  result = RegOpenKeyEx(HKEY_CURRENT_USER, sRegKey, 0, KEY_SET_VALUE, &hKey);
  if(result != 0)
  {
    LoadString(hInst, ERROR_240, szFormatString, sizeof(szFormatString));
    sprintf(tempString, szFormatString, sRegKey);
    MessageBeep(MB_ICONSTOP);
    MessageBox(tempString, TMS, MB_OK);
    return;
  }
//
//  Set the registry entries so Crystal knows where to look
//
  strcpy(szarString, szReportsTempFolder);
  RegSetValueEx(hKey, "DefaultDir", 0, type, (LPBYTE)szarString, strlen(szarString));
//
//  Copy the Schema.INI file from the install directory into the temporary report folder.
//  When we do this each time, we ensure that he's getting the most recent version
//
  char fileName[2][MAX_PATH];

  for(nJ = 0; nJ < 2; nJ++)
  {
    strcpy(fileName[nJ], nJ == 0 ? szInstallDirectory : szarString);
    strcat(fileName[nJ], "\\Schema.INI");
  }
  if(GetFileAttributes(fileName[0]) == 0xFFFFFFFF)  // Couldn't find INI file in szInstallDirectory
  {
    TMSError(NULL, MB_ICONSTOP, ERROR_218, (HANDLE)NULL);
    return;
  }
//
//  Found the INI file
//
  CopyFile(fileName[0], fileName[1], FALSE);

  CCrystal dlg(this, szReportFileName, szReportName);

  dlg.DoModal();
}

void CDiscipline::OnConsider() 
{
  CDriversToConsider dlg;

  if(dlg.DoModal() == IDOK)
  {
    SetupDriverComboBox();
  }
}


void CDiscipline::SetupDriverComboBox()
{
  char szDriver[DRIVERS_LASTNAME_LENGTH + 2 + DRIVERS_FIRSTNAME_LENGTH + 1];
  int  numDrivers;
  int  rcode2;

  pComboBoxDRIVER->ResetContent();

  rcode2 = btrieve(B_GETFIRST, TMS_DRIVERS, &DRIVERS, &DRIVERSKey1, 1);
  while(rcode2 == 0)
  {
    if(ConsideringThisDriverType(DRIVERS.DRIVERTYPESrecordID))
    {
      strncpy(tempString, DRIVERS.lastName, DRIVERS_LASTNAME_LENGTH);
      trim(tempString, DRIVERS_LASTNAME_LENGTH);
      strcpy(szDriver, tempString);
      strcat(szDriver, ", ");
      strncpy(tempString, DRIVERS.firstName, DRIVERS_FIRSTNAME_LENGTH);
      trim(tempString, DRIVERS_FIRSTNAME_LENGTH);
      strcat(szDriver, tempString);
      numDrivers = pComboBoxDRIVER->AddString(szDriver);
      if(numDrivers != CB_ERR)
      {
        pComboBoxDRIVER->SetItemData(numDrivers, DRIVERS.recordID);
      }
    }
    rcode2 = btrieve(B_GETNEXT, TMS_DRIVERS, &DRIVERS, &DRIVERSKey1, 1);
  }
  pComboBoxDRIVER->SetCurSel(-1);
}

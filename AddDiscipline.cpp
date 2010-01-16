// ADDDISCIPLINE.cpp : implementation file
//

#include "stdafx.h"
extern "C"{
#include "TMSHeader.h"
}  // EXTERN C

#include "TMS.h"

#include "AddDialogs.h"
#include "AddDiscipline.h"
#include "Discipline.h"
#include "DisciplineLetter.h"
#include "SuggestSuspension.h"

EXTERN void CALLBACK EXPORT TimerProc(HWND hWnd, UINT nMsg, UINT nIDEvent, DWORD dwTime);

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CAddDiscipline dialog


CAddDiscipline::CAddDiscipline(CWnd* pParent, long* pDRIVERSrecordID, long* pUpdateRecordID)
	: CDialog(CAddDiscipline::IDD, pParent)
{
  m_pDRIVERSrecordID = pDRIVERSrecordID;
  m_pUpdateRecordID = pUpdateRecordID;

	//{{AFX_DATA_INIT(CAddDiscipline)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CAddDiscipline::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAddDiscipline)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CAddDiscipline, CDialog)
	//{{AFX_MSG_MAP(CAddDiscipline)
	ON_BN_CLICKED(IDHELP, OnHelp)
	ON_CBN_SELCHANGE(ADDDISCIPLINE_CATEGORY, OnSelchangeCategory)
	ON_CBN_SELCHANGE(ADDDISCIPLINE_VIOLATION, OnSelchangeViolation)
	ON_NOTIFY(NM_CLICK, ADDDISCIPLINE_ACTIONLIST, OnClickActionlist)
	ON_BN_CLICKED(ADDDISCIPLINE_NOSUSPENSION, OnNosuspension)
	ON_BN_CLICKED(ADDDISCIPLINE_CC, OnCc)
	ON_BN_CLICKED(ADDDISCIPLINE_NA, OnNa)
	ON_CBN_SELENDOK(ADDDISCIPLINE_COMMENTCODE, OnSelendokCommentcode)
	ON_BN_CLICKED(IDLETTER, OnLetter)
	ON_CBN_SELENDOK(ADDDISCIPLINE_ACTUALACTION, OnSelendokActualaction)
	ON_BN_CLICKED(IDSUGGEST, OnSuggest)
	ON_BN_CLICKED(ADDDISCIPLINE_DEFERSUSPENSION, OnDefersuspension)
	ON_BN_CLICKED(ADDDISCIPLINE_SUSPENSIONDETAILS, OnSuspensiondetails)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAddDiscipline message handlers

BOOL CAddDiscipline::OnInitDialog() 
{
	CDialog::OnInitDialog();
//
//  Set up handles to the controls
//
  pButtonDRIVER = (CButton *)GetDlgItem(ADDDISCIPLINE_DRIVER);
  pDTPickerENTRYDATE = (CDTPicker *)GetDlgItem(ADDDISCIPLINE_ENTRYDATE);
  pDTPickerENTRYTIME = (CDTPicker *)GetDlgItem(ADDDISCIPLINE_ENTRYTIME);
  pDTPickerOFFENCEDATE = (CDTPicker *)GetDlgItem(ADDDISCIPLINE_OFFENCEDATE);
  pDTPickerOFFENCETIME = (CDTPicker *)GetDlgItem(ADDDISCIPLINE_OFFENCETIME);
  pComboBoxREPORTEDBY = (CComboBox *)GetDlgItem(ADDDISCIPLINE_REPORTEDBY);
  pComboBoxCATEGORY = (CComboBox *)GetDlgItem(ADDDISCIPLINE_CATEGORY);
  pComboBoxVIOLATION = (CComboBox *)GetDlgItem(ADDDISCIPLINE_VIOLATION);
  pListCtrlACTIONLIST = (CListCtrl *)GetDlgItem(ADDDISCIPLINE_ACTIONLIST);
  pComboBoxACTUALACTION = (CComboBox *)GetDlgItem(ADDDISCIPLINE_ACTUALACTION);
  pEditCOMMENTS = (CEdit *)GetDlgItem(ADDDISCIPLINE_COMMENTS);
  pDTPickerSUSPENSIONDATE = (CDTPicker *)GetDlgItem(ADDDISCIPLINE_SUSPENSIONDATE);
  pButtonNOSUSPENSION = (CButton *)GetDlgItem(ADDDISCIPLINE_NOSUSPENSION);
  pButtonDEFERSUSPENSION = (CButton *)GetDlgItem(ADDDISCIPLINE_DEFERSUSPENSION);
  pButtonSUSPENSIONDETAILS = (CButton *)GetDlgItem(ADDDISCIPLINE_SUSPENSIONDETAILS);
  pEditDURATION = (CEdit *)GetDlgItem(ADDDISCIPLINE_DURATION);
  pDTPickerDROPOFFDATE = (CDTPicker *)GetDlgItem(ADDDISCIPLINE_DROPOFFDATE);
  pButtonNA = (CButton *)GetDlgItem(ADDDISCIPLINE_NA);
  pButtonCC = (CButton *)GetDlgItem(ADDDISCIPLINE_CC);
  pComboBoxCOMMENTCODE = (CComboBox *)GetDlgItem(ADDDISCIPLINE_COMMENTCODE);
  pButtonSUGGEST = (CButton *)GetDlgItem(IDSUGGEST);
  pButtonHOURS = (CButton *)GetDlgItem(ADDDISCIPLINE_HOURS);
  pButtonDAYS = (CButton *)GetDlgItem(ADDDISCIPLINE_DAYS);
  pButtonSUSPENDEDWITHPAY = (CButton *)GetDlgItem(ADDDISCIPLINE_SUSPENDEDWITHPAY);
//
//  Get the driver
//
  CString s;
  
  DRIVERSKey0.recordID = *m_pDRIVERSrecordID;
  btrieve(B_GETEQUAL, TMS_DRIVERS, &DRIVERS, &DRIVERSKey0, 0);
  s = GetDatabaseString(DRIVERS.lastName, DRIVERS_LASTNAME_LENGTH);
  strcpy(tempString, s);
  strcat(tempString, ", ");
  s = GetDatabaseString(DRIVERS.firstName, DRIVERS_FIRSTNAME_LENGTH);
  strcat(tempString, s);
  s = GetDatabaseString(DRIVERS.initials, DRIVERS_INITIALS_LENGTH);
  if(s != "")
  {
    strcat(tempString, " ");
    strcat(tempString, s);
  }
  s = GetDatabaseString(DRIVERS.badgeNumber, DRIVERS_BADGENUMBER_LENGTH);
  if(s != "")
  {
    strcat(tempString, " (");
    strcat(tempString, s);
    strcat(tempString, ")");
  }
  GetWindowText(szarString, SZARSTRING_LENGTH);
  strcat(szarString, " - ");
  strcat(szarString, tempString);
  SetWindowText(szarString);
//
//  Get the current date and time and put it on the group box
//  
  CTime time = CTime::GetCurrentTime();
 	CString strTime = time.Format(_T("%A, %B %d, %Y at %I:%M:%S%p"));
//
//  Set up the timer to update the date/time
//
  pButtonDRIVER->SetWindowText(strTime);
 	pButtonDRIVER->SetTimer(0x1000, 1000, TimerProc);
//
//  Set up the action list control
//
  DWORD dwExStyles = pListCtrlACTIONLIST->GetExtendedStyle();
  pListCtrlACTIONLIST->SetExtendedStyle(dwExStyles | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_INFOTIP);
//
//  Set up the list controls
//
  LVCOLUMN LVC;
  LVC.mask = LVCF_FMT | LVCF_TEXT | LVCF_WIDTH;
  LVC.fmt = LVCFMT_LEFT;

  LVC.cx = 70;
  LVC.pszText = "Offense #";
  pListCtrlACTIONLIST->InsertColumn(0, &LVC);

  LVC.cx = 250;
  LVC.pszText = "Disciplinary Action";
  pListCtrlACTIONLIST->InsertColumn(1, &LVC);

  LVC.cx = 60;
  LVC.pszText = "Duration";
  pListCtrlACTIONLIST->InsertColumn(2, &LVC);

//
//  Set up "Reported by" as supervisors from the Drivers Table
//
  CString sup;
  int nI;
  int rcode2 = btrieve(B_GETFIRST, TMS_DRIVERS, &DRIVERS, &DRIVERSKey1, 1);

  while(rcode2 == 0)
  {
    if(DRIVERS.flags & DRIVERS_FLAG_SUPERVISOR)
    {
      sup = GetDatabaseString(DRIVERS.lastName, DRIVERS_LASTNAME_LENGTH);
      s = GetDatabaseString(DRIVERS.firstName, DRIVERS_FIRSTNAME_LENGTH);
      sup += ", ";
      sup += s;
      nI = pComboBoxREPORTEDBY->AddString(sup);
      pComboBoxREPORTEDBY->SetItemData(nI, DRIVERS.recordID);
    }
    rcode2 = btrieve(B_GETNEXT, TMS_DRIVERS, &DRIVERS, &DRIVERSKey1, 1);
  }
//
//  Comment codes combo box
//
  int numComments = SetUpCommentList(m_hWnd, ADDDISCIPLINE_COMMENTCODE);

  if(numComments == 0)
  {
    pButtonCC->EnableWindow(FALSE);
    pComboBoxCOMMENTCODE->EnableWindow(FALSE);
  }
//
//  Set up category combo box
//
  int nJ;
//
//  Categories
//
  for(nI = 0; nI < m_numViolationCategories; nI++)
  {
    if(m_numViolations[nI] != NO_RECORD)
    {
      nJ = pComboBoxCATEGORY->AddString(m_ViolationCategories[nI]);
      pComboBoxCATEGORY->SetItemData(nJ, nI);
    }
  }
//
//  Set up actual actions combo box
//
  for(nI = 1; nI < m_numViolationActions; nI++)
  {
    nJ = pComboBoxACTUALACTION->AddString(m_ViolationActions[nI]);
    pComboBoxACTUALACTION->SetItemData(nJ, nI);
  }
  pComboBoxACTUALACTION->SetCurSel(-1);
//
//  New record? Set today's date on the Date and Time pickers
//
  COleVariant v;

  if(*m_pUpdateRecordID == NO_RECORD)
  {
    long day = time.GetDay();
    long month = time.GetMonth();
    long year = time.GetYear();

    v = year;
    pDTPickerENTRYDATE->SetYear(v);
    pDTPickerOFFENCEDATE->SetYear(v);
    pDTPickerSUSPENSIONDATE->SetYear(v);
    pDTPickerDROPOFFDATE->SetYear(v);
    v = month;
    pDTPickerENTRYDATE->SetMonth(v);
    pDTPickerOFFENCEDATE->SetMonth(v);
    pDTPickerSUSPENSIONDATE->SetMonth(v);
    pDTPickerDROPOFFDATE->SetMonth(v);
    v = day;
    pDTPickerENTRYDATE->SetDay(v);
    pDTPickerOFFENCEDATE->SetDay(v);
    pDTPickerSUSPENSIONDATE->SetDay(v);
    pDTPickerDROPOFFDATE->SetDay(v);
  }
//
//  An update record
//
  else
  {
    int rcode2;

    DISCIPLINEKey0.recordID = *m_pUpdateRecordID;
    rcode2 = btrieve(B_GETEQUAL, TMS_DISCIPLINE, &DISCIPLINE, &DISCIPLINEKey0, 0);
//
//  Set the date and time pickers
//
    long year, month, day;
    long hours, minutes, seconds;
//
//  Date of entry
//
    struct tm *entryDateAndTime = localtime(&DISCIPLINE.entryDateAndTime);

    v = (long)(entryDateAndTime->tm_year + 1900);
    pDTPickerENTRYDATE->SetYear(v);
    v = (long)(entryDateAndTime->tm_mon + 1);
    pDTPickerENTRYDATE->SetMonth(v);
    v = (long)(entryDateAndTime->tm_mday);
    pDTPickerENTRYDATE->SetDay(v);
//
//  Time of entry
//
    v = (long)(entryDateAndTime->tm_hour);
    pDTPickerENTRYTIME->SetHour(v);
    v = (long)(entryDateAndTime->tm_min);
    pDTPickerENTRYTIME->SetMinute(v);
    v = (long)(entryDateAndTime->tm_sec);
    pDTPickerENTRYTIME->SetSecond(v);
//
//  Offence date
//
    GetYMD(DISCIPLINE.dateOfOffense, &year, &month, &day);
    v = year;
    pDTPickerOFFENCEDATE->SetYear(v);
    v = month;
    pDTPickerOFFENCEDATE->SetMonth(v);
    v = day;
    pDTPickerOFFENCEDATE->SetDay(v);
//
//  Offence time
//
    GetHMS(DISCIPLINE.timeOfOffense, &hours, &minutes, &seconds);
    v = hours;
    pDTPickerOFFENCETIME->SetHour(v);
    v = minutes;
    pDTPickerOFFENCETIME->SetMinute(v);
    v = seconds;
    pDTPickerOFFENCETIME->SetSecond(v);
//
//  Suspension date
//
    if(DISCIPLINE.suspensionDate > 0)
    {
      GetYMD(DISCIPLINE.suspensionDate, &year, &month, &day);
      v = year;
      pDTPickerSUSPENSIONDATE->SetYear(v);
      v = month;
      pDTPickerSUSPENSIONDATE->SetMonth(v);
      v = day;
      pDTPickerSUSPENSIONDATE->SetDay(v);
    }
//
//  Dropoff date.  If there is none, set it to a year from the offence date.
//
    if(DISCIPLINE.dropoffDate > 0)
    {
      GetYMD(DISCIPLINE.dropoffDate, &year, &month, &day);
      v = year;
      pDTPickerDROPOFFDATE->SetYear(v);
      v = month;
      pDTPickerDROPOFFDATE->SetMonth(v);
      v = day;
      pDTPickerDROPOFFDATE->SetDay(v);
    }
    else
    {
      GetYMD(DISCIPLINE.dateOfOffense, &year, &month, &day);
      year += 1;
      v = year;
      pDTPickerDROPOFFDATE->SetYear(v);
      v = month;
      pDTPickerDROPOFFDATE->SetMonth(v);
      v = day;
      pDTPickerDROPOFFDATE->SetDay(v);
    }
//
//  Set the rest of the controls
//
//  Reported by
//
    nJ = pComboBoxREPORTEDBY->GetCount();
    for(nI = 0; nI < nJ; nI++)
    {
      if((long)pComboBoxREPORTEDBY->GetItemData(nI) == DISCIPLINE.reportedByDRIVERSrecordID)
      {
        pComboBoxREPORTEDBY->SetCurSel(nI);
        break;
      }
    }
//
//  Category
//
    for(nI = 0; nI < pComboBoxCATEGORY->GetCount(); nI++)
    {
      if((long)pComboBoxCATEGORY->GetItemData(nI) == DISCIPLINE.violationCategory)
      {
        pComboBoxCATEGORY->SetCurSel(nI);
        break;
      }
    }
    OnSelchangeCategory();
//
//  Violation
//
    for(nI = 0; nI < pComboBoxVIOLATION->GetCount(); nI++)
    {
      if((long)pComboBoxVIOLATION->GetItemData(nI) == DISCIPLINE.violationType)
      {
        pComboBoxVIOLATION->SetCurSel(nI);
        break;
      }
    }
    OnSelchangeViolation();
//
//  Action taken
//
    BOOL bFound = FALSE;

    if(DISCIPLINE.actionTaken != NO_RECORD)
    {
      for(nI = 0; nI < pListCtrlACTIONLIST->GetItemCount(); nI++)
      {
        if((long)pListCtrlACTIONLIST->GetItemData(nI) == DISCIPLINE.actionTaken)
        {
          pListCtrlACTIONLIST->SetItemState(nI, LVIS_SELECTED, LVIS_SELECTED);
          bFound = TRUE;
          break;
        }
      }
    }
    if(!bFound)
    {
      nJ = pComboBoxACTUALACTION->GetCount();
      for(nI = 0; nI < nJ; nI++)
      {
        if((int)pComboBoxACTUALACTION->GetItemData(nI) == DISCIPLINE.actionTaken)
        {
          pComboBoxACTUALACTION->SetCurSel(nI);
          break;
        }
      }
    }
//
//  Comments
//
    strncpy(tempString, DISCIPLINE.comments, DISCIPLINE_COMMENTS_LENGTH);
    trim(tempString, DISCIPLINE_COMMENTS_LENGTH);
    pEditCOMMENTS->SetWindowText(tempString);
//
//  Suspension?
//
    if(DISCIPLINE.flags & DISCIPLINE_FLAG_SUSPENSION)
    {
      pButtonSUSPENSIONDETAILS->SetCheck(TRUE);
      sprintf(tempString, "%5.2f", DISCIPLINE.suspensionDuration);
      if(DISCIPLINE.flags & DISCIPLINE_FLAG_DURATIONINHOURS)
      {
        strcat(tempString, " D");
      }
      else
      {
        strcat(tempString, " H");
      }
      pEditDURATION->SetWindowText(tempString);
    }
    else
    {
      if(DISCIPLINE.flags & DISCIPLINE_FLAG_DEFERSUSPENSION)
      {
        pButtonDEFERSUSPENSION->SetCheck(TRUE);
      }
      else
      {
        pButtonNOSUSPENSION->SetCheck(TRUE);
      }
      pDTPickerSUSPENSIONDATE->EnableWindow(FALSE);
      pEditDURATION->EnableWindow(FALSE);
      pButtonSUGGEST->EnableWindow(FALSE);
      pButtonHOURS->EnableWindow(FALSE);
      pButtonDAYS->EnableWindow(FALSE);
      pButtonSUSPENDEDWITHPAY->EnableWindow(FALSE);
    }
  }
//
//  Not udpating
//
  if(*m_pUpdateRecordID == NO_RECORD)
  {
//
//  Make him choose a category
//
    pComboBoxCATEGORY->SetCurSel(-1);
//
//  And assume no suspension
//
    pButtonNOSUSPENSION->SetCheck(TRUE);
    pDTPickerSUSPENSIONDATE->EnableWindow(FALSE);
    pEditDURATION->EnableWindow(FALSE);
    pButtonSUGGEST->EnableWindow(FALSE);
    pButtonHOURS->EnableWindow(FALSE);
    pButtonDAYS->EnableWindow(FALSE);
    pButtonSUSPENDEDWITHPAY->EnableWindow(FALSE);
  }
//
//  Make sure a roster template's been established
//
//  Database verify: 11-Jan-07
//
  DAILYOPSKey1.recordTypeFlag = DAILYOPS_FLAG_ROSTER;
  DAILYOPSKey1.pertainsToDate = time.GetYear() * 10000 + time.GetMonth() * 100 + time.GetDay();
  DAILYOPSKey1.pertainsToTime = time.GetHour() * 3600 + time.GetMinute() * 60 + time.GetSecond();
  DAILYOPSKey1.recordFlags = 0;
  rcode2 = btrieve(B_GETLESSTHANOREQUAL, TMS_DAILYOPS, &DAILYOPS, &DAILYOPSKey1, 1);
  if(rcode2 == 0 && (DAILYOPS.recordTypeFlag & DAILYOPS_FLAG_ROSTER))
  {
  }
  else
  {
    s.LoadString(ERROR_343);
    MessageBeep(MB_ICONINFORMATION);
    MessageBox(s, TMS, MB_ICONINFORMATION | MB_OK);
    pButtonNOSUSPENSION->EnableWindow(FALSE);
    pButtonDEFERSUSPENSION->EnableWindow(FALSE);
    pButtonSUSPENSIONDETAILS->EnableWindow(FALSE);
    pDTPickerSUSPENSIONDATE->EnableWindow(FALSE);
    pEditDURATION->EnableWindow(FALSE);
    pButtonSUGGEST->EnableWindow(FALSE);
    pButtonHOURS->EnableWindow(FALSE);
    pButtonDAYS->EnableWindow(FALSE);
    pButtonSUSPENDEDWITHPAY->EnableWindow(FALSE);
  }

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CAddDiscipline::OnCancel() 
{
	// TODO: Add extra cleanup here
	
	CDialog::OnCancel();
}

void CAddDiscipline::OnHelp() 
{
	// TODO: Add your control notification handler code here
	
}

void CAddDiscipline::OnSelchangeCategory() 
{
  int nI, nJ;
  
  nI = pComboBoxCATEGORY->GetCurSel();

  if(nI == CB_ERR)
  {
    return;
  }

  int type = pComboBoxCATEGORY->GetItemData(nI);

  if(type == CB_ERR)
  {
    return;
  }
//
//  Violations
//
  pComboBoxVIOLATION->ResetContent();
  if(m_numViolations[type] != NO_RECORD)
  {
    for(nI = 0; nI < m_numViolations[type]; nI++)
    {
      nJ = pComboBoxVIOLATION->AddString(m_Violations[type][nI].text);
      pComboBoxVIOLATION->SetItemData(nJ, nI);
    }
    pComboBoxVIOLATION->SetCurSel(0);

    OnSelchangeViolation();
  }
}

void CAddDiscipline::OnSelchangeViolation() 
{
  LVITEM LVI;
  
  int nI;
  int category;
  int type;

  nI = (int)pComboBoxCATEGORY->GetCurSel();
  category = (int)pComboBoxCATEGORY->GetItemData(nI);

  nI = (int)pComboBoxVIOLATION->GetCurSel();
  type = (int)pComboBoxVIOLATION->GetItemData(nI);

  if(category < 0 || type < 0)
    return;

  pListCtrlACTIONLIST->DeleteAllItems();
  
  for(nI = 0; nI < m_Violations[category][type].numActions; nI++)
  {
//
//  Offense number
//
    LVI.mask = LVIF_TEXT | LVIF_PARAM;
    LVI.iItem = nI;
    LVI.iSubItem = 0;
    sprintf(tempString, "%d", nI + 1);
    LVI.pszText = tempString;
    LVI.lParam = m_Violations[category][type].actions[nI];
    LVI.iItem = pListCtrlACTIONLIST->InsertItem(&LVI);
//
//  Disciplinary Action
//
    LVI.mask = LVIF_TEXT;
    LVI.iItem = nI;
    LVI.iSubItem = 1;
    LVI.pszText = m_ViolationActions[m_Violations[category][type].actions[nI]];
    LVI.iItem = pListCtrlACTIONLIST->SetItem(&LVI);
//
//  Duration
//
    LVI.mask = LVIF_TEXT;
    LVI.iItem = nI;
    LVI.iSubItem = 2;
    if(m_Violations[category][type].duration == NO_RECORD)
      strcpy(tempString, "Permanent");
    else
      sprintf(tempString, "%ld", m_Violations[category][type].duration);
    LVI.pszText = tempString;
    LVI.iItem = pListCtrlACTIONLIST->SetItem(&LVI);
	}
}

void CAddDiscipline::OnClickActionlist(NMHDR* pNMHDR, LRESULT* pResult) 
{
	int nI = pListCtrlACTIONLIST->GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);

  if(nI < 0)
    return;

  pComboBoxACTUALACTION->SetCurSel(-1);

//
//  Set the dropoff date
//
  nI = (int)pComboBoxCATEGORY->GetCurSel();
  int category = (int)pComboBoxCATEGORY->GetItemData(nI);

  nI = (int)pComboBoxVIOLATION->GetCurSel();
  int type = (int)pComboBoxVIOLATION->GetItemData(nI);

  if(category >= 0 && type >= 0)
  {
    long duration = m_Violations[category][type].duration;
    
    if(duration > 0)
    {
      CTime time = CTime::GetCurrentTime();
      COleVariant v;

      time += CTimeSpan(duration, 0, 0, 0);

      long day = time.GetDay();
      long month = time.GetMonth();
      long year = time.GetYear();

      v = year;
      pDTPickerDROPOFFDATE->SetYear(v);
      v = month;
      pDTPickerDROPOFFDATE->SetMonth(v);
      v = day;
      pDTPickerDROPOFFDATE->SetDay(v);
    }
  }

	*pResult = 0;
}

void CAddDiscipline::OnNosuspension() 
{
  pDTPickerSUSPENSIONDATE->EnableWindow(FALSE);
  pEditDURATION->EnableWindow(FALSE);
  pButtonSUGGEST->EnableWindow(FALSE);
  pButtonHOURS->EnableWindow(FALSE);
  pButtonDAYS->EnableWindow(FALSE);
  pButtonSUSPENDEDWITHPAY->EnableWindow(FALSE);
}


void CAddDiscipline::OnDefersuspension() 
{
  OnNosuspension();
}

void CAddDiscipline::OnSuspensiondetails() 
{
  pDTPickerSUSPENSIONDATE->EnableWindow(TRUE);
  pEditDURATION->EnableWindow(TRUE);
  pButtonSUGGEST->EnableWindow(TRUE);
  pButtonHOURS->EnableWindow(TRUE);
  pButtonDAYS->EnableWindow(TRUE);
  pButtonSUSPENDEDWITHPAY->EnableWindow(TRUE);
}

void CAddDiscipline::OnCc() 
{
  pComboBoxCOMMENTCODE->SetCurSel(0);
}

void CAddDiscipline::OnNa() 
{
  pComboBoxCOMMENTCODE->SetCurSel(-1);
}

void CAddDiscipline::OnSelendokCommentcode() 
{
  if(pButtonNA->GetCheck())
  {
    pButtonNA->SetCheck(FALSE);
    pButtonCC->SetCheck(TRUE);
  }
}

void CAddDiscipline::OnOK() 
{
  VARIANT v;
  LVITEM  LVI;
  long year;
  long month;
  long day;
  long hours;
  long minutes;
  long seconds;
  int  nI;
  int  rcode2;
  int  opCode;

  if(*m_pUpdateRecordID == NO_RECORD)
  {
    rcode2 = btrieve(B_GETLAST, TMS_DISCIPLINE, &DISCIPLINE, &DISCIPLINEKey0, 0);
    DISCIPLINE.recordID = AssignRecID(rcode2, DISCIPLINE.recordID);
    opCode = B_INSERT;
  }
  else
  {
    DISCIPLINEKey0.recordID = *m_pUpdateRecordID;
    rcode2 = btrieve(B_GETEQUAL, TMS_DISCIPLINE, &DISCIPLINE, &DISCIPLINEKey0, 0);
    opCode = B_UPDATE;
  }
//
//  Comment code
//
  if(pButtonNA->GetCheck())
    DISCIPLINE.COMMENTSrecordID = NO_RECORD;
  else
  {
    nI = (int)pComboBoxCOMMENTCODE->GetCurSel();
    DISCIPLINE.COMMENTSrecordID = (nI == CB_ERR ? NO_RECORD : pComboBoxCOMMENTCODE->GetItemData(nI));
  }
//
//  Driver
//
  DISCIPLINE.DRIVERSrecordID = *m_pDRIVERSrecordID;
//
//  Date of entry
//
  v = pDTPickerENTRYDATE->GetYear();
  year = v.lVal;
  v = pDTPickerENTRYDATE->GetMonth();
  month = v.lVal;
  v = pDTPickerENTRYDATE->GetDay();
  day = v.lVal;
//
//  Time of entry
//
  v = pDTPickerENTRYTIME->GetHour();
  hours = v.lVal;
  v = pDTPickerENTRYTIME->GetMinute();
  minutes = v.lVal;
  v = pDTPickerENTRYTIME->GetSecond();
  seconds = v.lVal;

  struct tm DateTime;

  DateTime.tm_sec = seconds;
  DateTime.tm_min = minutes;
  DateTime.tm_hour = hours;
  DateTime.tm_mday = day;
  DateTime.tm_mon = month - 1;
  DateTime.tm_year = year - 1900;

  DISCIPLINE.entryDateAndTime = mktime(&DateTime);
//
//  Date of offense
//
  v = pDTPickerOFFENCEDATE->GetYear();
  year = v.lVal;
  v = pDTPickerOFFENCEDATE->GetMonth();
  month = v.lVal;
  v = pDTPickerOFFENCEDATE->GetDay();
  day = v.lVal;
  DISCIPLINE.dateOfOffense = year * 10000 + month * 100 + day;
//
//  Time of entry
//
  v = pDTPickerOFFENCETIME->GetHour();
  hours = v.lVal;
  v = pDTPickerOFFENCETIME->GetMinute();
  minutes = v.lVal;
  v = pDTPickerOFFENCETIME->GetSecond();
  seconds = v.lVal;
  DISCIPLINE.timeOfOffense = hours * 3600 + minutes * 60 + seconds;
//
//  Reported by
//
  nI = (int)pComboBoxREPORTEDBY->GetCurSel();
  if(nI == CB_ERR)
  {
    DISCIPLINE.reportedByDRIVERSrecordID = NO_RECORD;
  }
  else
  {
    DISCIPLINE.reportedByDRIVERSrecordID = (long)pComboBoxREPORTEDBY->GetItemData(nI);
  }
//
//  Violation Category
//
  nI = (int)pComboBoxCATEGORY->GetCurSel();
  if(nI == CB_ERR)
  {
    DISCIPLINE.violationCategory = NO_RECORD;
  }
  else
  {
    DISCIPLINE.violationCategory = (long)pComboBoxCATEGORY->GetItemData(nI);
  }
//
//  Violation type
//
  nI = (int)pComboBoxVIOLATION->GetCurSel();
  if(nI == CB_ERR)
  {
    DISCIPLINE.violationType = NO_RECORD;
  }
  else
  {
    DISCIPLINE.violationType = (long)pComboBoxVIOLATION->GetItemData(nI);
  }
//
//  Dropoff date
//
  v = pDTPickerDROPOFFDATE->GetYear();
  year = v.lVal;
  v = pDTPickerDROPOFFDATE->GetMonth();
  month = v.lVal;
  v = pDTPickerDROPOFFDATE->GetDay();
  day = v.lVal;
  DISCIPLINE.dropoffDate = year * 10000 + month * 100 + day;
//
// Comments
//
  pEditCOMMENTS->GetWindowText(tempString, TEMPSTRING_LENGTH);
  pad(tempString, DISCIPLINE_COMMENTS_LENGTH);
  strncpy(DISCIPLINE.comments, tempString, DISCIPLINE_COMMENTS_LENGTH);
//
//  Flags
//
  if(DISCIPLINE.violationCategory != NO_RECORD && DISCIPLINE.violationType != NO_RECORD)
  {
    DISCIPLINE.flags = (unsigned short int)m_Violations[DISCIPLINE.violationCategory][DISCIPLINE.violationType].flags;
  }
  else
  {
    DISCIPLINE.flags = 0;
  }
//
//  Suspension date / time
//
  BOOL bTellDailyOps;

  if(pButtonSUSPENSIONDETAILS->GetCheck())
  {
    v = pDTPickerSUSPENSIONDATE->GetYear();
    year = v.lVal;
    v = pDTPickerSUSPENSIONDATE->GetMonth();
    month = v.lVal;
    v = pDTPickerSUSPENSIONDATE->GetDay();
    day = v.lVal;
    DISCIPLINE.suspensionDate = year * 10000 + month * 100 + day;
    pEditDURATION->GetWindowText(tempString, TEMPSTRING_LENGTH);
    DISCIPLINE.suspensionDuration = (float)atof(tempString);
    DISCIPLINE.flags |= DISCIPLINE_FLAG_SUSPENSION;
    if(pButtonHOURS->GetCheck())
    {
      DISCIPLINE.flags |= DISCIPLINE_FLAG_DURATIONINHOURS;
    }
    if(pButtonSUSPENDEDWITHPAY->GetCheck())
    {
      DISCIPLINE.flags |= DISCIPLINE_FLAG_SUSPENDEDWITHPAY;
    }
    bTellDailyOps = TRUE;
  }
  else
  {
    DISCIPLINE.suspensionDate = NO_RECORD;
    DISCIPLINE.suspensionDuration = 0;
    if(pButtonDEFERSUSPENSION->GetCheck())
    {
      DISCIPLINE.flags |= DISCIPLINE_FLAG_DEFERSUSPENSION;
    }
    bTellDailyOps = FALSE;
  }
//
//  Action Taken
//
	nI = (int)pListCtrlACTIONLIST->GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);
  if(nI < 0)
  {
    DISCIPLINE.actionTaken = NO_RECORD;
  }
  else
  {
    LVI.mask = LVIF_PARAM;
    LVI.iItem = nI;
    LVI.iSubItem = 0;
    pListCtrlACTIONLIST->GetItem(&LVI);
    DISCIPLINE.actionTaken = LVI.lParam;
  }
//
//  If no action taken from the list, look to actual action
//
  if(DISCIPLINE.actionTaken == NO_RECORD)
  {
    nI = (int)pComboBoxACTUALACTION->GetCurSel();
    if(nI != CB_ERR)
    {
      DISCIPLINE.actionTaken = (long)pComboBoxACTUALACTION->GetItemData(nI);
      DISCIPLINE.flags |= DISCIPLINE_FLAG_ACTUALACTION;
    }
  }
//
//  If this discipline record was generated via daily ops, get and update the daily ops record
//
//  Database verify: 11-Jan-07
//
  if(opCode == B_UPDATE && DISCIPLINE.DAILYOPSrecordID > 0)
  {
    DAILYOPSKey0.recordID = DISCIPLINE.DAILYOPSrecordID;
    rcode2 = btrieve(B_GETEQUAL, TMS_DAILYOPS, &DAILYOPS, &DAILYOPSKey0, 0);
    if(rcode2 == 0)
    {
      DAILYOPS.recordFlags |= DAILYOPS_FLAG_DISCIPLINEMETED;
      rcode2 = btrieve(B_UPDATE, TMS_DAILYOPS, &DAILYOPS, &DAILYOPSKey0, 0);
    }
  }
//
//  DAILYOPSrecordID (formerly Reserved)
//
  else
  {
    DISCIPLINE.DAILYOPSrecordID = NO_RECORD;
  }
//
//  Update or insert
//
  rcode2 = btrieve(opCode, TMS_DISCIPLINE, &DISCIPLINE, &DISCIPLINEKey0, 0);
//
//  Tell daily ops?
//
//  Database verify: 11-Jan-07
//
  if(bTellDailyOps)
  {
    PROPOSEDRUNDef PROPOSEDRUN;
    CString s;

    s.LoadString(TEXT_323);

    MessageBeep(MB_ICONQUESTION);
    if(MessageBox(s, TMS, MB_YESNO) == IDYES)
    {
      long date, time;

      rcode2 = btrieve(B_GETLAST, TMS_DAILYOPS, &DAILYOPS, &DAILYOPSKey0, 0);
      DAILYOPS.recordID = AssignRecID(rcode2, DAILYOPS.recordID);
//
//  "Pertains to" Date/Time
//
      long yyyy, mm, dd;

      GetYMD(DISCIPLINE.suspensionDate, &yyyy, &mm, &dd);

      CTime suspensionDate(yyyy, mm, dd, 0, 0, 0);
      int indexToRoster[7] = {6, 0, 1, 2, 3, 4, 5};

      DAILYOPS.pertainsToDate = DISCIPLINE.suspensionDate;
      if(pButtonDAYS->GetCheck())
      {
        DAILYOPS.pertainsToTime = 0;
      }
      else
      {
        COSTDef COST;
        int  today = indexToRoster[suspensionDate.GetDayOfWeek() - 1];
        int  numPieces;
        long earliestRunStart = T1159P;

        ROSTERKey1.DIVISIONSrecordID = m_DivisionRecordID;
        ROSTERKey1.rosterNumber = NO_RECORD;
        rcode2 = btrieve(B_GETGREATER, TMS_ROSTER, &ROSTER, &ROSTERKey1, 1);
        while(rcode2 == 0)
        {
          if(ROSTER.DRIVERSrecordID == *m_pDRIVERSrecordID)
          {
            RUNSKey0.recordID = ROSTER.WEEK[m_RosterWeek].RUNSrecordIDs[today];
            btrieve(B_GETEQUAL, TMS_RUNS, &RUNS, &RUNSKey0, 0);
            btrieve(B_GETPOSITION, TMS_RUNS, &RUNS, &RUNSKey0, 0);
            btrieve(B_GETDIRECT, TMS_RUNS, &RUNS, &RUNSKey1, 1);
            numPieces = GetRunElements(hWndMain, &RUNS, &PROPOSEDRUN, &COST, TRUE);
            if(RUNSVIEW[0].runOnTime < earliestRunStart)
            {
              earliestRunStart = RUNSVIEW[0].runOnTime;
            }
          }
          rcode2 = btrieve(B_GETNEXT, TMS_ROSTER, &ROSTER, &ROSTERKey1, 1);
        }
        DAILYOPS.pertainsToTime = earliestRunStart;
      }
//
//  Date of entry
//
      v = pDTPickerENTRYDATE->GetYear();
      year = v.lVal;
      v = pDTPickerENTRYDATE->GetMonth();
      month = v.lVal;
      v = pDTPickerENTRYDATE->GetDay();
      day = v.lVal;
      date = year * 10000 + month * 100 + day;
//
//  Time of entry
//
      v = pDTPickerENTRYTIME->GetHour();
      hours = v.lVal;
      v = pDTPickerENTRYTIME->GetMinute();
      minutes = v.lVal;
      v = pDTPickerENTRYTIME->GetSecond();
      seconds = v.lVal;
      time = hours * 3600 + minutes * 60 + seconds;

      DAILYOPS.entryDateAndTime = TMSmktime(date, time);
//
//  Flags, userID
//
      DAILYOPS.recordTypeFlag = DAILYOPS_FLAG_ABSENCE;
      DAILYOPS.userID = m_UserID;
//
//  DAILYOPSrecordID and DRIVERSrecordID
//
      DAILYOPS.DAILYOPSrecordID = NO_RECORD;
      DAILYOPS.DRIVERSrecordID = *m_pDRIVERSrecordID;
//
//  Associated data (Absence)
//
      memset(DAILYOPS.DOPS.associatedData, 0x00, DAILYOPS_ASSOCIATEDDATA_LENGTH);
//
//  Until date/time
//
      float decimalMinutes;
      long  hour;
  
      hour = (int)DISCIPLINE.suspensionDuration;
      decimalMinutes = DISCIPLINE.suspensionDuration - hour;

      long durInSeconds = hour * 3600 + (long)((decimalMinutes * 60) * 60);

      if(pButtonDAYS->GetCheck())
      {
        suspensionDate += CTimeSpan((int)DISCIPLINE.suspensionDuration - 1, 0, 0, 0);
        dd = suspensionDate.GetDay();
        mm = suspensionDate.GetMonth();
        yyyy = suspensionDate.GetYear();
        DAILYOPS.DOPS.Absence.untilDate = yyyy * 10000 + mm * 100 + dd;
        DAILYOPS.DOPS.Absence.untilTime = T1159P;
      }
      else
      {
        DAILYOPS.DOPS.Absence.untilDate = DAILYOPS.pertainsToDate;
        DAILYOPS.DOPS.Absence.untilTime = DAILYOPS.pertainsToTime + durInSeconds;
      }
//
//  Time lost
//
      DAILYOPS.DOPS.Absence.timeLost = durInSeconds;
//
//  Reason index
//
      DAILYOPS.DOPS.Absence.reasonIndex = 7;  // Suspension
//
//  Flags
//
      DAILYOPS.recordFlags = DAILYOPS_FLAG_ABSENCEREGISTER | DAILYOPS_FLAG_ABSENCEFROMDISCIPLINE;
      if(pButtonSUSPENDEDWITHPAY->GetCheck())
      {
        DAILYOPS.recordFlags |= DAILYOPS_FLAG_ABSENCEPAID;
      }
//
//  Add it
//
      rcode2 = btrieve(B_INSERT, TMS_DAILYOPS, &DAILYOPS, &DAILYOPSKey0, 0);
      if(rcode2 == 0)
      {
        m_LastDAILYOPSRecordID = DAILYOPS.recordID;
      }
//
//  Set up the OFFTIME record
//
//  RecordID
//
      rcode2 = btrieve(B_GETLAST, TMS_OFFTIME, &OFFTIME, &OFFTIMEKey0, 0);
      OFFTIME.recordID = AssignRecID(rcode2, OFFTIME.recordID);
//
//  DailyOps recordID
//
      OFFTIME.DAILYOPSrecordID = DAILYOPS.recordID;
//
//  Text
//
      s.LoadString(TEXT_324);
      strcpy(tempString, s);
      pad(tempString, OFFTIME_TEXT_LENGTH);
      strncpy(OFFTIME.text, tempString, OFFTIME_TEXT_LENGTH);
//
//  Add the record
//
      rcode2 = btrieve(B_INSERT, TMS_OFFTIME, &OFFTIME, &OFFTIMEKey0, 0);
    }
  }
//
//  All done
//
	
	CDialog::OnOK();
}

void CAddDiscipline::OnLetter() 
{
  CDisciplineLetter dlg(this, m_pDRIVERSrecordID);

  if(dlg.DoModal() == IDOK)
  {
  }
}

void CAddDiscipline::OnSelendokActualaction() 
{
  int nI;
  int numRows = pListCtrlACTIONLIST->GetItemCount();

  for(nI = 0; nI < numRows; nI++)
  {
    if(pListCtrlACTIONLIST->GetItemState(nI, LVIS_SELECTED) == LVIS_SELECTED)
    {
      pListCtrlACTIONLIST->SetItemState(nI, !LVIS_SELECTED, LVIS_SELECTED);
    }
  }
//
//  Reset the dropoff date
//
  CTime time = CTime::GetCurrentTime();
  COleVariant v;

  long day = time.GetDay();
  long month = time.GetMonth();
  long year = time.GetYear();

  v = year;
  pDTPickerDROPOFFDATE->SetYear(v);
  v = month;
  pDTPickerDROPOFFDATE->SetMonth(v);
  v = day;
  pDTPickerDROPOFFDATE->SetDay(v);
}

void CAddDiscipline::OnSuggest() 
{
  long startDate;

  CSuggestSuspension dlg(this, m_pDRIVERSrecordID, &startDate);

  if(dlg.DoModal() == IDOK)
  {
    long day, month, year;

    GetYMD(startDate, &year, &month, &day);

    COleVariant v;

    v = year;
    pDTPickerSUSPENSIONDATE->SetYear(v);
    v = month;
    pDTPickerSUSPENSIONDATE->SetMonth(v);
    v = day;
    pDTPickerSUSPENSIONDATE->SetDay(v);
  }
}

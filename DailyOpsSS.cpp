// DailyOpsSS.cpp : implementation file
//

#include "stdafx.h"
extern "C"{
#include "TMSHeader.h"

BOOL ProcessAbsenceRules(DAILYOPSDef *pDAILYOPS, BOOL bRegister);

}  // EXTERN C

#include "tms.h"
#include "DailyOpsHeader.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDailyOpsSS dialog


CDailyOpsSS::CDailyOpsSS(CWnd* pParent, long DRIVERSrecordID, unsigned short int flags, BOOL* pbChanged)
	: CDialog(CDailyOpsSS::IDD, pParent)
{
  m_DRIVERSrecordID = DRIVERSrecordID;
  m_RecordTypeFlag = flags;
  m_pbChanged = pbChanged;
  *m_pbChanged = FALSE;
	//{{AFX_DATA_INIT(CDailyOpsSS)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CDailyOpsSS::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDailyOpsSS)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDailyOpsSS, CDialog)
	//{{AFX_MSG_MAP(CDailyOpsSS)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDHELP, OnHelp)
	ON_NOTIFY(NM_CLICK, DAILYOPSSS_LIST, OnClickList)
	ON_BN_CLICKED(IDREMOVE, OnRemove)
	ON_NOTIFY(NM_DBLCLK, DAILYOPSSS_LIST, OnDblclkList)
	ON_BN_CLICKED(IDUPDATE, OnUpdate)
	ON_BN_CLICKED(DAILYOPSSS_JUSTFROMTHELASTYEAR, OnJustfromthelastyear)
	ON_BN_CLICKED(DAILYOPSSS_ALL, OnAll)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDailyOpsSS message handlers

BOOL CDailyOpsSS::OnInitDialog() 
{
	CDialog::OnInitDialog();

  HCURSOR hSaveCursor = SetCursor(hCursorWait);
//
//  Set up the pointers to the controls
//	
  pListCtrlLIST = (CListCtrl *)GetDlgItem(DAILYOPSSS_LIST);
  pButtonIDREMOVE = (CButton *)GetDlgItem(IDREMOVE);
  pButtonIDUPDATE = (CButton *)GetDlgItem(IDUPDATE);
  pButtonJUSTFROMTHELASTYEAR = (CButton *)GetDlgItem(DAILYOPSSS_JUSTFROMTHELASTYEAR);
  pButtonALL = (CButton *)GetDlgItem(DAILYOPSSS_ALL);
//
//  Put the operator's name in the title
//
  CString s;

  DRIVERSKey0.recordID = m_DRIVERSrecordID;
  btrieve(B_GETEQUAL, TMS_DRIVERS, &DRIVERS, &DRIVERSKey0, 0);
  s = GetDatabaseString(DRIVERS.lastName, DRIVERS_LASTNAME_LENGTH);
  strcpy(tempString, s);
  strcat(tempString, ", ");
  s = GetDatabaseString(DRIVERS.firstName, DRIVERS_FIRSTNAME_LENGTH);
  strcat(tempString, s);
  GetWindowText(szarString, SZARSTRING_LENGTH);
  strcat(szarString, " - ");
  strcat(szarString, tempString);
  SetWindowText(szarString);
//
//  Set up the list controls
//
  DWORD dwExStyles = pListCtrlLIST->GetExtendedStyle();
  pListCtrlLIST->SetExtendedStyle(dwExStyles | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_INFOTIP);

  LVCOLUMN LVC;
  LVC.mask = LVCF_FMT | LVCF_TEXT | LVCF_WIDTH;

  LVC.cx = 160;
  LVC.fmt = LVCFMT_LEFT;
  LVC.pszText = "Entered/Updated";
  pListCtrlLIST->InsertColumn(0, &LVC);

  if(m_RecordTypeFlag & DAILYOPS_FLAG_ABSENCE)
  {
    LVC.cx = 150;
    LVC.fmt = LVCFMT_LEFT;
    LVC.pszText = "Reason";
    pListCtrlLIST->InsertColumn(1, &LVC);

    LVC.cx = 40;
    LVC.pszText = "Paid?";
    pListCtrlLIST->InsertColumn(2, &LVC);
  
    LVC.cx = 306;
    LVC.pszText = "Notes";
    pListCtrlLIST->InsertColumn(3, &LVC);
  
    LVC.cx = 90;
    LVC.pszText = "From";
    pListCtrlLIST->InsertColumn(4, &LVC);
 
    LVC.cx = 50;
    LVC.pszText = "Time";
    pListCtrlLIST->InsertColumn(5, &LVC);
	
    LVC.cx = 90;
    LVC.pszText = "To";
    pListCtrlLIST->InsertColumn(6, &LVC);

    LVC.cx = 50;
    LVC.pszText = "Time";
    pListCtrlLIST->InsertColumn(7, &LVC);
  }
//
//  Operator Check-in/out, extra time, and extra trip
//
  else
  {
    LVC.cx = 776;
    LVC.fmt = LVCFMT_LEFT;
    LVC.pszText = "Action";
    pListCtrlLIST->InsertColumn(1, &LVC);
  }
//
//  Set the default timeframe
//
  CTime  time = CTime::GetCurrentTime();

  pButtonJUSTFROMTHELASTYEAR->SetCheck(TRUE);
  m_StartDate = (time.GetYear() - 1)  * 10000 + time.GetMonth() * 100 + time.GetDay();
  Display();
//
//  Disable Remove and Update 'till he picks something
//
  pButtonIDREMOVE->EnableWindow(FALSE);
  pButtonIDUPDATE->EnableWindow(FALSE);

  SetCursor(hSaveCursor);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CDailyOpsSS::OnClose() 
{
  OnCancel();
}

void CDailyOpsSS::OnCancel() 
{
	// TODO: Add extra cleanup here
	
	CDialog::OnCancel();
}

void CDailyOpsSS::OnHelp() 
{
	// TODO: Add your control notification handler code here
	
}

void CDailyOpsSS::OnClickList(NMHDR* pNMHDR, LRESULT* pResult) 
{
  pButtonIDREMOVE->EnableWindow(TRUE);
  pButtonIDUPDATE->EnableWindow(TRUE);
	
	*pResult = 0;
}

void CDailyOpsSS::OnOK() 
{
	CDialog::OnOK();
}

void CDailyOpsSS::OnRemove() 
{
  CString s;

  s.LoadString(ERROR_340);
  MessageBeep(MB_ICONQUESTION);
  if(MessageBox(s, TMS, MB_YESNO) != IDYES)
  {
    pButtonIDREMOVE->EnableWindow(FALSE);
    pButtonIDUPDATE->EnableWindow(FALSE);
    return;
  }
  int  nI = pListCtrlLIST->GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);

  if(nI < 0)
  {
    pButtonIDREMOVE->EnableWindow(FALSE);
    pButtonIDUPDATE->EnableWindow(FALSE);
    return;
  }
//
//  Establish "now"
//
  CTime time = CTime::GetCurrentTime();
  long  datenow = time.GetYear() * 10000 + time.GetMonth() * 100 + time.GetDay();
  long  timenow = time.GetHour() * 3600 + time.GetMinute() * 60 + time.GetSecond();
  int   rcode2;

  long  DAILYOPSrecordID = pListCtrlLIST->GetItemData(nI);

  DAILYOPSKey0.recordID = DAILYOPSrecordID;
  btrieve(B_GETEQUAL, TMS_DAILYOPS, &DAILYOPS, &DAILYOPSKey0, 0);

  long paidTime[2];

  paidTime[0] = DAILYOPS.DOPS.Absence.paidTime[0];
  paidTime[1] = DAILYOPS.DOPS.Absence.paidTime[1];

  long  unpaidTime  = DAILYOPS.DOPS.Absence.unpaidTime;
  long  timeLost    = DAILYOPS.DOPS.Absence.timeLost;
  long  reasonIndex = DAILYOPS.DOPS.Absence.reasonIndex;

  rcode2 = btrieve(B_GETLAST, TMS_DAILYOPS, &DAILYOPS, &DAILYOPSKey0, 0);
  DAILYOPS.recordID = AssignRecID(rcode2, DAILYOPS.recordID);
  DAILYOPS.entryDateAndTime = time.GetTime();
  DAILYOPS.pertainsToDate = NO_RECORD;
  DAILYOPS.pertainsToTime = NO_TIME;
  DAILYOPS.recordTypeFlag = (char)m_RecordTypeFlag;
  DAILYOPS.userID = m_UserID;
  DAILYOPS.DRIVERSrecordID = m_DRIVERSrecordID;
  DAILYOPS.DAILYOPSrecordID = DAILYOPSrecordID;
  memset(DAILYOPS.DOPS.associatedData, 0x00, DAILYOPS_ASSOCIATEDDATA_LENGTH);
//
//  Clear an absence
//
//  Database verify: 11-Jan-07
//
  if(m_RecordTypeFlag & DAILYOPS_FLAG_ABSENCE)
  {
    DAILYOPS.recordFlags = DAILYOPS_FLAG_ABSENCEUNREGISTER;
    DAILYOPS.DOPS.Absence.reasonIndex = reasonIndex;
    DAILYOPS.DOPS.Absence.timeLost = timeLost;
    DAILYOPS.DOPS.Absence.paidTime[0] = paidTime[0];
    DAILYOPS.DOPS.Absence.paidTime[1] = paidTime[1];
    DAILYOPS.DOPS.Absence.unpaidTime = unpaidTime;
    if(ProcessAbsenceRules(&DAILYOPS, FALSE))
    {
      rcode2 = btrieve(B_INSERT, TMS_DAILYOPS, &DAILYOPS, &DAILYOPSKey0, 0);
      if(rcode2 == 0)
      {
        m_LastDAILYOPSRecordID = DAILYOPS.recordID;
      }
    }
  }
//
//  Clear an operator checkin-out/run swap/extra trip/extra time
//
//  Database verify: 11-Jan-07
//
  else
  {
    DAILYOPS.recordFlags = DAILYOPS_FLAG_OPERATORUNCHECK;
    rcode2 = btrieve(B_INSERT, TMS_DAILYOPS, &DAILYOPS, &DAILYOPSKey0, 0);
    if(rcode2 == 0)
    {
      m_LastDAILYOPSRecordID = DAILYOPS.recordID;
    }
  }

  pListCtrlLIST->DeleteItem(nI);
  *m_pbChanged = TRUE;
}

void CDailyOpsSS::OnDblclkList(NMHDR* pNMHDR, LRESULT* pResult) 
{
  OnUpdate();

	*pResult = 0;
}

void CDailyOpsSS::DisplayAbsenceRow(int iItem, DAILYOPSDef *pDAILYOPS)
{
  CString s;
  long year, month, day;
  int  rcode2;

//
//  Date of entry
//
  LVITEM  LVI;

  LVI.mask = LVIF_TEXT | LVIF_PARAM;
  LVI.iItem = iItem;
  LVI.iSubItem = 0;
  sprintf(tempString, "%s", ctime(&pDAILYOPS->entryDateAndTime));
  LVI.pszText = tempString;
  LVI.lParam = pDAILYOPS->recordID;
  LVI.iItem = pListCtrlLIST->InsertItem(&LVI);
//
//  Absence reason
//
  LVI.mask = LVIF_TEXT;
  LVI.iItem = iItem;
  LVI.iSubItem = 1;
  if(pDAILYOPS->DOPS.Absence.reasonIndex == NO_RECORD)
  {
    strcpy(tempString, "Assignment cleared manually");
  }
  else
  {
    strcpy(tempString, m_AbsenceReasons[pDAILYOPS->DOPS.Absence.reasonIndex].szText);
  }
  LVI.pszText = tempString;
  pListCtrlLIST->SetItem(&LVI);
//
//  Paid?
//
  if(pDAILYOPS->recordFlags & DAILYOPS_FLAG_ABSENCEPAID)
  {
    s = "Y";
  }
  else
  {
    s = (pDAILYOPS->recordFlags & DAILYOPS_FLAG_NOTSUREIFPAID) ? "U" : "N";
  }
  LVI.mask = LVIF_TEXT;
  LVI.iItem = iItem;
  LVI.iSubItem = 2;
  strcpy(tempString, s);
  LVI.pszText = tempString;
  pListCtrlLIST->SetItem(&LVI);
//
//  Notes
//
  OFFTIMEKey1.DAILYOPSrecordID = pDAILYOPS->recordID;
  rcode2 = btrieve(B_GETEQUAL, TMS_OFFTIME, &OFFTIME, &OFFTIMEKey1, 1);
  if(rcode2 == 0)
  {
    strncpy(tempString, OFFTIME.text, OFFTIME_TEXT_LENGTH);
    trim(tempString, OFFTIME_TEXT_LENGTH);
    LVI.mask = LVIF_TEXT;
    LVI.iItem = iItem;
    LVI.iSubItem = 3;
    LVI.pszText = tempString;
    pListCtrlLIST->SetItem(&LVI);
  }
//
//  From Date
//
  GetYMD(pDAILYOPS->pertainsToDate, &year, &month, &day);
  LVI.mask = LVIF_TEXT;
  LVI.iItem = iItem;
  LVI.iSubItem = 4;
  sprintf(tempString, "%02ld-%s-%04ld", day, szMonth[month - 1], year);
  LVI.pszText = tempString;
  pListCtrlLIST->SetItem(&LVI);
//
//  From time
//
  LVI.mask = LVIF_TEXT;
  LVI.iItem = iItem;
  LVI.iSubItem = 5;
  strcpy(tempString, Tchar(pDAILYOPS->pertainsToTime));
  LVI.pszText = tempString;
  pListCtrlLIST->SetItem(&LVI);
//
//  To Date
//
  GetYMD(pDAILYOPS->DOPS.Absence.untilDate, &year, &month, &day);
  LVI.mask = LVIF_TEXT;
  LVI.iItem = iItem;
  LVI.iSubItem = 6;
  sprintf(tempString, "%02ld-%s-%04ld", day, szMonth[month - 1], year);
  LVI.pszText = tempString;
  pListCtrlLIST->SetItem(&LVI);
//
//  To time
//
  LVI.mask = LVIF_TEXT;
  LVI.iItem = iItem;
  LVI.iSubItem = 7;
  strcpy(tempString, Tchar(pDAILYOPS->DOPS.Absence.untilTime));
  LVI.pszText = tempString;
  pListCtrlLIST->SetItem(&LVI);
}

void CDailyOpsSS::OnUpdate() 
{

  if(!(m_RecordTypeFlag & DAILYOPS_FLAG_ABSENCE))
  {
    return;
  }

  int  nI = pListCtrlLIST->GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);
  if(nI < 0)
  {
    return;
  }

  long  DAILYOPSrecordID = pListCtrlLIST->GetItemData(nI);

  CDailyOpsUA dlg(this, &DAILYOPSrecordID);

  if(dlg.DoModal() == IDOK)
  {
    pListCtrlLIST->DeleteItem(nI);
    DAILYOPSKey0.recordID = DAILYOPSrecordID;
    btrieve(B_GETEQUAL, TMS_DAILYOPS, &DAILYOPS, &DAILYOPSKey0, 0);
    DisplayAbsenceRow(nI, &DAILYOPS);
    *m_pbChanged = TRUE;
  }

}

void CDailyOpsSS::OnJustfromthelastyear() 
{
  CTime  time = CTime::GetCurrentTime();

  m_StartDate = (time.GetYear() - 1)  * 10000 + time.GetMonth() * 100 + time.GetDay();
  Display();
}

void CDailyOpsSS::OnAll() 
{
  m_StartDate = 19000101;
  Display();
}

//
//  Display() - Cycle through the entries for this operator
//
void CDailyOpsSS::Display()
{
  LVITEM  LVI;
  BOOL   bFound;
  int    iItem = 0;
  int    rcode2;
  long   year, month, day;

  HCURSOR hSaveCursor = SetCursor(hCursorWait);

  pListCtrlLIST->DeleteAllItems();

//
//  Database verify: 11-Jan-07
//
  DAILYOPSKey3.DRIVERSrecordID = m_DRIVERSrecordID;
  DAILYOPSKey3.pertainsToDate = m_StartDate;
  DAILYOPSKey3.pertainsToTime = NO_TIME;
  DAILYOPSKey3.recordTypeFlag = (char)m_RecordTypeFlag;
  DAILYOPSKey3.recordFlags = 0;
  rcode2 = btrieve(B_GETGREATER, TMS_DAILYOPS, &DAILYOPS, &DAILYOPSKey3, 3);
  while(rcode2 == 0 &&
        (DAILYOPS.DRIVERSrecordID == m_DRIVERSrecordID))
  {
    bFound = FALSE;
    if((DAILYOPS.recordTypeFlag & DAILYOPS_FLAG_ABSENCE) ||
          (DAILYOPS.recordTypeFlag & DAILYOPS_FLAG_OPERATOR))
    {
      if(!ANegatedRecord(DAILYOPS.recordID, 3) && !ANegatingRecord(&DAILYOPS))
      {
        if((m_RecordTypeFlag & DAILYOPS_FLAG_ABSENCE) &&
              (DAILYOPS.recordTypeFlag & DAILYOPS_FLAG_ABSENCE))
        {
          DisplayAbsenceRow(iItem, &DAILYOPS);
          iItem++;
        }
//
//  Check in/out
//
        else if((m_RecordTypeFlag & DAILYOPS_FLAG_OPERATOR) &&
              (DAILYOPS.recordTypeFlag & DAILYOPS_FLAG_OPERATOR))
        {
          GetYMD(DAILYOPS.pertainsToDate, &year, &month, &day);
//
//  Date of entry
//
          LVI.mask = LVIF_TEXT | LVIF_PARAM;
          LVI.iItem = iItem;
          LVI.iSubItem = 0;
          sprintf(tempString, "%s", ctime(&DAILYOPS.entryDateAndTime));
          LVI.pszText = tempString;
          LVI.lParam = DAILYOPS.recordID;
          LVI.iItem = pListCtrlLIST->InsertItem(&LVI);
//
//  In or out
//
          if(DAILYOPS.recordFlags & DAILYOPS_FLAG_OPERATORCHECKIN)
          {
            strcpy(tempString, "Checked in");
          }
          else if(DAILYOPS.recordFlags & DAILYOPS_FLAG_OPERATORCHECKOUT)
          {
            strcpy(tempString, "Checked out");
          }
//
//  Run swap
//
          else if(DAILYOPS.recordFlags & DAILYOPS_FLAG_OPERATORDEASSIGN)
          {
            strcpy(tempString, "Run swap / deassigned from run");
          }
//
//  Extra time
//
          else if(DAILYOPS.recordFlags & DAILYOPS_FLAG_OVERTIME)
          {
            sprintf(tempString, "Added time of %s", chhmm(DAILYOPS.DOPS.Operator.timeAdjustment));
            if(DAILYOPS.DOPS.Operator.extraTimeReasonIndex != NO_RECORD)
            {
              sprintf(szarString, ".  Reason: %s", m_ExtraTimeReasons[DAILYOPS.DOPS.Operator.extraTimeReasonIndex].szText);
              strcat(tempString, szarString);
            }
          }
//
//  Extra trip
//
          else if(DAILYOPS.recordFlags & DAILYOPS_FLAG_EXTRATRIP)
          {
            CString s;
            CString outputString;
            GenerateTripDef GTResults;

            GenerateTrip(DAILYOPS.DOPS.Operator.ROUTESrecordID, DAILYOPS.DOPS.Operator.SERVICESrecordID,
                  DAILYOPS.DOPS.Operator.directionIndex, DAILYOPS.DOPS.Operator.PATTERNNAMESrecordID,
                  DAILYOPS.DOPS.Operator.timeAtMLP, GENERATETRIP_FLAG_DISPLAYERRORS, &GTResults);
//
//  Build up the string
//
//  Route
//
            ROUTESKey0.recordID = DAILYOPS.DOPS.Operator.ROUTESrecordID;
            btrieve(B_GETEQUAL, TMS_ROUTES, &ROUTES, &ROUTESKey0, 0);
            s = GetDatabaseString(ROUTES.number, ROUTES_NUMBER_LENGTH);
            outputString = s;
            outputString += " - ";
            s = GetDatabaseString(ROUTES.name, ROUTES_NAME_LENGTH);
            outputString += s;
//
//  Service
//
            SERVICESKey0.recordID = DAILYOPS.DOPS.Operator.SERVICESrecordID;
            btrieve(B_GETEQUAL, TMS_SERVICES, &SERVICES, &SERVICESKey0, 0);
            s = GetDatabaseString(SERVICES.name, SERVICES_NAME_LENGTH);
            outputString += " (";
            outputString += s;
            outputString += ") : ";
//
//  From node
//
            NODESKey0.recordID = GTResults.firstNODESrecordID;
            btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
            s = GetDatabaseString(NODES.abbrName, NODES_ABBRNAME_LENGTH);
            outputString += s;
            outputString += " ";
//
//  From time
//
            outputString += Tchar(GTResults.firstNodeTime);
            outputString += " ";
//
//  To time
//
            outputString += Tchar(GTResults.lastNodeTime);
            outputString += " ";
//
//  To node
//
            NODESKey0.recordID = GTResults.lastNODESrecordID;
            btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
            s = GetDatabaseString(NODES.abbrName, NODES_ABBRNAME_LENGTH);
            outputString += s;
            outputString += " = ";
//
//  Trip time
//
            long totalTime = GTResults.lastNodeTime - GTResults.firstNodeTime;

            outputString += chhmm(totalTime);
//
//  Deadhead time?
//
            long deadheadTime = DAILYOPS.DOPS.Operator.timeAdjustment - totalTime;

            if(deadheadTime != 0)
            {
              outputString += " + deadhead of ";
              outputString += chhmm(deadheadTime);
              outputString += " = ";
              totalTime += deadheadTime;
              outputString += chhmm(totalTime);
            }
//
//  Set up tempString
//
            sprintf(tempString, "Added trip on route %s", outputString);
          }
//
//  Unknown flag (this shouldn't happen)
//
          else
          {
            strcpy(tempString, "");
          }
//
//  Display it
//
          LVI.mask = LVIF_TEXT;
          LVI.iItem = iItem;
          LVI.iSubItem = 1;
          LVI.pszText = tempString;
          LVI.iItem = pListCtrlLIST->SetItem(&LVI);
          iItem++;
        }
//
//  Set up the next row
//
      }
    }
    rcode2 = btrieve(B_GETNEXT, TMS_DAILYOPS, &DAILYOPS, &DAILYOPSKey3, 3);
  }

  SetCursor(hSaveCursor);
}

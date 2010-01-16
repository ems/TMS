// PayrollSelections.cpp : implementation file
//

#include "stdafx.h"
extern "C"{
#include "TMSHeader.h"

}  // EXTERN C

#include "tms.h"
#include "DailyOpsHeader.h"
#include "PayrollSelections.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CPayrollSelections dialog


CPayrollSelections::CPayrollSelections(CWnd* pParent, PayrollSelectionsDef* pPS)
	: CDialog(CPayrollSelections::IDD, pParent)
{
  m_pPS = pPS;

	//{{AFX_DATA_INIT(CPayrollSelections)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CPayrollSelections::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPayrollSelections)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CPayrollSelections, CDialog)
	//{{AFX_MSG_MAP(CPayrollSelections)
	ON_WM_CLOSE()
	ON_BN_CLICKED(PAYROLLSELECTIONS_CRYSTAL, OnCrystal)
	ON_BN_CLICKED(PAYROLLSELECTIONS_FILE, OnFile)
	ON_BN_CLICKED(IDHELP, OnHelp)
	ON_BN_CLICKED(IDCONSIDER, OnConsider)
	ON_BN_CLICKED(IDSELECTALL, OnSelectall)
	ON_EN_CHANGE(PAYROLLSELECTIONS_DAYSINPERIOD, OnChangeDaysinperiod)
	ON_BN_CLICKED(PAYROLLSELECTIONS_DAILYREPORT, OnDailyreport)
	ON_BN_CLICKED(PAYROLLSELECTIONS_PERIODREPORT, OnPeriodreport)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPayrollSelections message handlers

BOOL CPayrollSelections::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
//
//  Set up pointers to the controls
//
  pButtonDAILYREPORT = (CButton *)GetDlgItem(PAYROLLSELECTIONS_DAILYREPORT);
  pButtonPERIODREPORT = (CButton *)GetDlgItem(PAYROLLSELECTIONS_PERIODREPORT);
  pStaticDAYSINPERIOD_TEXT = (CStatic *)GetDlgItem(PAYROLLSELECTIONS_DAYSINPERIOD_TEXT);
  pEditDAYSINPERIOD = (CEdit *)GetDlgItem(PAYROLLSELECTIONS_DAYSINPERIOD);
  pButtonCRYSTAL = (CButton *)GetDlgItem(PAYROLLSELECTIONS_CRYSTAL);
  pButtonCRYSTAL_TITLE = (CButton *)GetDlgItem(PAYROLLSELECTIONS_CRYSTAL_TITLE);
  pButtonSHOWRUNDETAILS = (CButton *)GetDlgItem(PAYROLLSELECTIONS_SHOWRUNDETAILS);
  pButtonFILE = (CButton *)GetDlgItem(PAYROLLSELECTIONS_FILE);
  pButtonFILE_TITLE = (CButton *)GetDlgItem(PAYROLLSELECTIONS_FILE_TITLE);
  pButtonTABDELIMITED = (CButton *)GetDlgItem(PAYROLLSELECTIONS_TABDELIMITED);
  pButtonCSV = (CButton *)GetDlgItem(PAYROLLSELECTIONS_CSV);
  pEditDELIMITEDBY = (CEdit *)GetDlgItem(PAYROLLSELECTIONS_DELIMITEDBY);
  pButtonXML = (CButton *)GetDlgItem(PAYROLLSELECTIONS_XML);
  pListBoxOPERATORLIST = (CListBox *)GetDlgItem(PAYROLLSELECTIONS_OPERATORLIST);
  pDTPickerDATE = (CDTPicker *)GetDlgItem(PAYROLLSELECTIONS_DATE);
  pEditSTARTINGROSTERWEEK = (CEdit *)GetDlgItem(PAYROLLSELECTIONS_STARTINGROSTERWEEK);
  pEditLASTROSTERWEEK = (CEdit *)GetDlgItem(PAYROLLSELECTIONS_LASTROSTERWEEK);
  pButtonINCLUDECODES = (CButton *)GetDlgItem(PAYROLLSELECTIONS_INCLUDECODES);
  pButtonCOMBINEOUTPUT = (CButton *)GetDlgItem(PAYROLLSELECTIONS_COMBINEOUTPUT);
//
//  Default to the Daily Report
//
  pButtonDAILYREPORT->SetCheck(TRUE);
//
//  Default to Crystal and disable the file output area
//
  pButtonCRYSTAL->SetCheck(TRUE);
  pButtonSHOWRUNDETAILS->SetCheck(TRUE);
  OnCrystal();
//
//  Populate the list of operators
//
  PopulateOperatorList();
//
//  Set today's date on the Date/Time Picker
//
  COleVariant v;
  CTime time = CTime::GetCurrentTime();
  long  day = time.GetDay();
  long  month = time.GetMonth();
  long  year = time.GetYear();

  v = year;
  pDTPickerDATE->SetYear(v);
  v = month;
  pDTPickerDATE->SetMonth(v);
  v = day;
  pDTPickerDATE->SetDay(v);
//
//  Set the default delimiter character to a comma
//
  pEditDELIMITEDBY->SetWindowText(",");
//
//  Default the starting roster week to the currently selected globlal value
//
 sprintf(tempString, "%ld", m_RosterWeek + 1);
 pEditSTARTINGROSTERWEEK->SetWindowText(tempString);
 pEditLASTROSTERWEEK->SetWindowText(tempString);
//
//  Default to including payroll codes
//
  pButtonINCLUDECODES->SetCheck(TRUE);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CPayrollSelections::PopulateOperatorList()
{
  int  nI;
  int  rcode2;

  pListBoxOPERATORLIST->ResetContent();

  rcode2 = btrieve(B_GETFIRST, TMS_DRIVERS, &DRIVERS, &DRIVERSKey1, 1);
  while(rcode2 == 0)
  {
    if(ConsideringThisDriverType(DRIVERS.DRIVERTYPESrecordID))
    { 
      nI = pListBoxOPERATORLIST->AddString(BuildOperatorString(NO_RECORD));
      pListBoxOPERATORLIST->SetItemData(nI, DRIVERS.recordID);
    }
    rcode2 = btrieve(B_GETNEXT, TMS_DRIVERS, &DRIVERS, &DRIVERSKey1, 1);
  }
}

void CPayrollSelections::OnConsider() 
{
  CDriversToConsider dlg;

  if(dlg.DoModal() == IDOK)
  {
    PopulateOperatorList();
  }
}

void CPayrollSelections::OnSelectall() 
{
  pListBoxOPERATORLIST->SetSel(-1, TRUE);
}

void CPayrollSelections::OnClose() 
{
  OnCancel();
}

void CPayrollSelections::OnCrystal() 
{
  pButtonCRYSTAL_TITLE->EnableWindow(TRUE);
  pButtonSHOWRUNDETAILS->EnableWindow(TRUE);
  pButtonFILE_TITLE->EnableWindow(FALSE);
  pButtonTABDELIMITED->EnableWindow(FALSE);
  pButtonCSV->EnableWindow(FALSE);
  pButtonXML->EnableWindow(FALSE);
  pButtonTABDELIMITED->SetCheck(FALSE);
  pButtonCSV->SetCheck(FALSE);
  pEditDELIMITEDBY->EnableWindow(FALSE);
  pButtonXML->SetCheck(FALSE);
  pButtonCOMBINEOUTPUT->SetCheck(FALSE);
  pButtonCOMBINEOUTPUT->EnableWindow(FALSE);
}

void CPayrollSelections::OnFile() 
{
  pButtonCRYSTAL_TITLE->EnableWindow(FALSE);
  pButtonSHOWRUNDETAILS->EnableWindow(FALSE);
  pButtonSHOWRUNDETAILS->SetCheck(FALSE);
  pButtonFILE_TITLE->EnableWindow(TRUE);
  pButtonTABDELIMITED->EnableWindow(TRUE);
  pButtonCSV->EnableWindow(TRUE);
  pButtonXML->EnableWindow(TRUE);
  pButtonTABDELIMITED->SetCheck(TRUE);
  pButtonCSV->SetCheck(FALSE);
  pEditDELIMITEDBY->EnableWindow(TRUE);
  pButtonXML->SetCheck(FALSE);
  if(pButtonPERIODREPORT->GetCheck())
  {
    pEditDAYSINPERIOD->GetWindowText(tempString, TEMPSTRING_LENGTH);
    if(atol(tempString) > 1)
    {
      pButtonCOMBINEOUTPUT->SetCheck(TRUE);
      pButtonCOMBINEOUTPUT->EnableWindow(TRUE);
    }
  }
}

void CPayrollSelections::OnHelp() 
{
}

void CPayrollSelections::OnCancel() 
{
	CDialog::OnCancel();
}

void CPayrollSelections::OnOK() 
{
  int nI, nJ;
//
//  Operators
//  
  nI = pListBoxOPERATORLIST->GetSelCount();
  if(nI == 0)
  {
    pListBoxOPERATORLIST->SetSel(-1, TRUE);
  }
//
//  Allocate space
//
  nJ = pListBoxOPERATORLIST->GetSelCount();
  if(nJ == 0)
  {
    nJ = pListBoxOPERATORLIST->GetCount();
  }
  m_pPS->pSelectedRecordIDs = (long *)HeapAlloc(GetProcessHeap(),  HEAP_ZERO_MEMORY, sizeof(long) * nJ); 
  if(m_pPS->pSelectedRecordIDs == NULL)
  {
    AllocationError(__FILE__, __LINE__, FALSE);
    OnCancel();
    return;
  }
//
//  Record the selections
//
  m_pPS->numSelected = 0;
  nJ = pListBoxOPERATORLIST->GetCount();
  for(nI = 0; nI < nJ; nI++)
  {
    if(pListBoxOPERATORLIST->GetSel(nI))
    {
      m_pPS->pSelectedRecordIDs[m_pPS->numSelected] = pListBoxOPERATORLIST->GetItemData(nI);
      m_pPS->numSelected++;
    }
  }
//
//  Get the number of days in a period.
//  If it's less than 1, make it 7
//
  pEditDAYSINPERIOD->GetWindowText(tempString, TEMPSTRING_LENGTH);
  nI = atoi(tempString);
  if(nI <= 0)
  {
    nI = ROSTER_MAX_DAYS;
  }
  m_pPS->daysInPeriod = nI;
//
//  Get the starting roster week
//  If it's out of range, make it 1
//
  pEditSTARTINGROSTERWEEK->GetWindowText(tempString, TEMPSTRING_LENGTH);
  nI = atoi(tempString);
  if(nI < 1 || nI > ROSTER_MAX_WEEKS)
  {
    nI = 1;
  }
  m_pPS->startingRosterWeek = nI - 1;  // For the index
//
//  Get the last roster week
//  If it's out of range, make it 1
//
  pEditLASTROSTERWEEK->GetWindowText(tempString, TEMPSTRING_LENGTH);
  nI = atoi(tempString);
  if(nI < 1 || nI > ROSTER_MAX_WEEKS)
  {
    nI = 1;
  }
  m_pPS->lastRosterWeek = nI - 1;  // For the index
//
//  Determine the return flags
//
//  Daily or period?
//
  m_pPS->flags = (pButtonDAILYREPORT->GetCheck() ? PAYROLLSELECTIONS_FLAG_DAILY : PAYROLLSELECTIONS_FLAG_PERIOD);
//
//  Output selection
//
  if(pButtonCRYSTAL->GetCheck())
  {
    m_pPS->flags |= PAYROLLSELECTIONS_FLAG_CRYSTAL;
  }
  else
  {
    if(pButtonTABDELIMITED->GetCheck())
    {
      m_pPS->flags |= PAYROLLSELECTIONS_FLAG_TABDELIMITED;
    }
    else if(pButtonCSV->GetCheck())
    {
      m_pPS->flags |= PAYROLLSELECTIONS_FLAG_CSV;
      pEditDELIMITEDBY->GetWindowText(tempString, TEMPSTRING_LENGTH);
      if(tempString[0] == ' ')
      {
        tempString[0] = ',';  // default to a comma if he blanked it out
      }
      m_pPS->delimiter = tempString[0];
    }
    else
    {
      m_pPS->flags |= PAYROLLSELECTIONS_FLAG_XML;
    }
    if(pButtonCOMBINEOUTPUT->GetCheck())
    {
      m_pPS->flags |= PAYROLLSELECTIONS_FLAG_COMBINEOUTPUT;
    }
  }
//
//  Get the date
//
  COleVariant v;
  long year, month, day;

  v = pDTPickerDATE->GetYear();
  year = v.lVal;
  v = pDTPickerDATE->GetMonth();
  month = v.lVal;
  v = pDTPickerDATE->GetDay();
  day = v.lVal;

  m_pPS->startDateYYYYMMDD = (year * 10000) + (month * 100) + day;
//
//  Include payroll codes?
//
  if(pButtonINCLUDECODES->GetCheck())
  {
    m_pPS->flags |= PAYROLLSELECTIONS_FLAG_CODES;
  }
//
//  Show run details
//
  if(pButtonSHOWRUNDETAILS->GetCheck())
  {
    m_pPS->flags |= PAYROLLSELECTIONS_FLAG_SHOWRUNDETAILS;
  }
//
//  Update the workrules if necessary
//
  if(m_bWorkrulesChanged)
  {
    HCURSOR hSaveCursor = SetCursor(hCursorWait);
    writeWorkrules(hWndMain);
    SetCursor(hSaveCursor);
  }

//
//  All done
//
	CDialog::OnOK();
}

void CPayrollSelections::OnChangeDaysinperiod() 
{
  if(!IsWindow(pButtonFILE->m_hWnd))
  {
    return;
  }
  if(pButtonFILE->GetCheck())
  {
    pEditDAYSINPERIOD->GetWindowText(tempString, TEMPSTRING_LENGTH);

    BOOL bEnable = (atol(tempString) > 1);

    pButtonCOMBINEOUTPUT->EnableWindow(bEnable);
    pButtonCOMBINEOUTPUT->SetCheck(bEnable);
  }
}

void CPayrollSelections::OnDailyreport() 
{
  pStaticDAYSINPERIOD_TEXT->EnableWindow(FALSE);
  pEditDAYSINPERIOD->EnableWindow(FALSE);
  pEditDAYSINPERIOD->SetWindowText("");
  pButtonCOMBINEOUTPUT->SetCheck(FALSE);
  pButtonCOMBINEOUTPUT->EnableWindow(FALSE);
}

void CPayrollSelections::OnPeriodreport() 
{
  pStaticDAYSINPERIOD_TEXT->EnableWindow(TRUE);
  pEditDAYSINPERIOD->EnableWindow(TRUE);
  pEditDAYSINPERIOD->SetWindowText("7");
  if(pButtonFILE->GetCheck())
  {
    pButtonCOMBINEOUTPUT->SetCheck(TRUE);
    pButtonCOMBINEOUTPUT->EnableWindow(TRUE);
  }
}

//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
// CCReports.cpp : implementation file
//
// [CCMRPT01]  - accessed via "Summary" on the dialog [follows IDOK]
// FIELD1=ComplaintNumber,VARCHAR,12,0,12,0,
// FIELD2=Date,VARCHAR,10,0,10,0,
// FIELD3=Comment,VARCHAR,20,0,20,0,
// FIELD4=RouteNumber,VARCHAR,8,0,8,0,
// FIELD5=Location,VARCHAR,16,0,16,0,
// FIELD6=Employee,VARCHAR,16,0,16,0,
// FIELD7=Customer,VARCHAR,16,0,16,0,
// FIELD8=ReferredTo,VARCHAR,16,0,16,0,
// FIELD9=Action,VARCHAR,16,0,16,0,
// FIELD10=SortField,VARCHAR,30,0,30,0,
//
// [CCMRPT02] - accessed via "Detail" on the dialog [follows IDDETAIL]
//

#include "stdafx.h"
extern "C"
{
#include "TMSHeader.h"
}
#include "TMS.h"
#include "cc.h"
#include "WinReg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CCCReports dialog

extern "C"
{
int CCMRPT02_SortIndex;

int sort_CCMRPT02(const void *a, const void *b)
{
  CCMRPT02Def *pa, *pb;
  pa = (CCMRPT02Def *)a;
  pb = (CCMRPT02Def *)b;
  switch (CCMRPT02_SortIndex)
  {
    case 0:  // Reference number
    case 2:  // Classification
    case 3:  // Route nuber
    case 4:  // Location
    case 5:  // Employee
    case 6:  // Customer
    case 7:  // Referr to
    case 8:  // Action
      return(strcmp(pa->SortFields.szCatchAll, pb->SortFields.szCatchAll));
      break;
  }

  return(pa->SortFields.incidentDate < pb->SortFields.incidentDate ? -1 : pa->SortFields.incidentDate > pb->SortFields.incidentDate ? 1 : 0);
}
}  //extern "C"


CCCReports::CCCReports(CWnd* pParent /*=NULL*/)
	: CDialog(CCCReports::IDD, pParent)
{
  m_flags = 0;
  m_pCCMRPT02 = NULL;

	//{{AFX_DATA_INIT(CCCReports)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CCCReports::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CCCReports)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CCCReports, CDialog)
	//{{AFX_MSG_MAP(CCCReports)
	ON_BN_CLICKED(IDHELP, OnHelp)
	ON_BN_CLICKED(CCREPORTS_BYCUSTOMER, OnBycustomer)
	ON_BN_CLICKED(CCREPORTS_BYROUTE, OnByroute)
	ON_BN_CLICKED(CCREPORTS_BYCATEGORY, OnBycategory)
	ON_BN_CLICKED(CCREPORTS_BYCLASSIFICATIONCODE, OnByclassificationcode)
	ON_BN_CLICKED(CCREPORTS_BYDETERMINATIONCODE, OnBydeterminationcode)
	ON_BN_CLICKED(CCREPORTS_BYEMPLOYEE, OnByemployee)
	ON_BN_CLICKED(CCREPORTS_BYDEPARTMENT, OnBydepartment)
	ON_BN_CLICKED(CCREPORTS_UNRESOLVED, OnUnresolved)
	ON_BN_CLICKED(IDDETAIL, OnDetail)
	ON_BN_CLICKED(IDSUMMARY, OnSummary)
	ON_BN_CLICKED(CCREPORTS_BYREFERENCENUMBER, OnByreferencenumber)
	ON_BN_CLICKED(IDLISTING, OnListing)
	ON_WM_CLOSE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCCReports message handlers

BOOL CCCReports::OnInitDialog() 
{
	CDialog::OnInitDialog();

  pButtonBYREFERENCENUMBER = (CButton *)GetDlgItem(CCREPORTS_BYREFERENCENUMBER);
  pButtonBYCUSTOMER = (CButton *)GetDlgItem(CCREPORTS_BYCUSTOMER);
  pButtonBYROUTE = (CButton *)GetDlgItem(CCREPORTS_BYROUTE);
  pButtonBYCATEGORY = (CButton *)GetDlgItem(CCREPORTS_BYCATEGORY);
  pButtonBYCLASSIFICATIONCODE = (CButton *)GetDlgItem(CCREPORTS_BYCLASSIFICATIONCODE);
  pButtonBYDETERMINATIONCODE = (CButton *)GetDlgItem(CCREPORTS_BYDETERMINATIONCODE);
  pButtonBYEMPLOYEE = (CButton *)GetDlgItem(CCREPORTS_BYEMPLOYEE);
  pButtonBYDEPARTMENT = (CButton *)GetDlgItem(CCREPORTS_BYDEPARTMENT);
  pButtonUNRESOLVED = (CButton *)GetDlgItem(CCREPORTS_UNRESOLVED);
  pComboBoxREFERENCENUMBER = (CComboBox *)GetDlgItem(CCREPORTS_REFERENCENUMBER);
  pComboBoxCUSTOMER = (CComboBox *)GetDlgItem(CCREPORTS_CUSTOMER);
  pComboBoxROUTE = (CComboBox *)GetDlgItem(CCREPORTS_ROUTE);
  pComboBoxCATEGORY = (CComboBox *)GetDlgItem(CCREPORTS_CATEGORY);
  pComboBoxCLASSIFICATIONCODE = (CComboBox *)GetDlgItem(CCREPORTS_CLASSIFICATIONCODE);
  pComboBoxDETERMINATIONCODE = (CComboBox *)GetDlgItem(CCREPORTS_DETERMINATIONCODE);
  pComboBoxEMPLOYEE = (CComboBox *)GetDlgItem(CCREPORTS_EMPLOYEE);
  pComboBoxDEPARTMENT = (CComboBox *)GetDlgItem(CCREPORTS_DEPARTMENT);
  pComboBoxCB = (CComboBox *)GetDlgItem(CCREPORTS_CB);
  pDTPickerFROMDATE = (CDTPicker *)GetDlgItem(CCREPORTS_FROMDATE);
  pDTPickerUNTILDATE = (CDTPicker *)GetDlgItem(CCREPORTS_UNTILDATE);
  pComboBoxSORTBY = (CComboBox *)GetDlgItem(CCREPORTS_SORTBY);
  pEditLOG = (CEdit *)GetDlgItem(CCREPORTS_LOG);
//
//  Set up the drop downs
//
//  Reference number
//
  int rcode2;
  int nI;
  
  rcode2 = btrieve(B_GETFIRST, TMS_COMPLAINTS, &COMPLAINTS, &COMPLAINTSKey2, 2);
  while(rcode2 == 0 || rcode2 == 22)
  {
    strncpy(tempString, COMPLAINTS.referenceNumber, COMPLAINTS_REFERENCENUMBER_LENGTH);
    trim(tempString, COMPLAINTS_REFERENCENUMBER_LENGTH);
    nI = pComboBoxREFERENCENUMBER->AddString(tempString);
    pComboBoxREFERENCENUMBER->SetItemData(nI, COMPLAINTS.recordID);
    rcode2 = btrieve(B_GETNEXT, TMS_COMPLAINTS, &COMPLAINTS, &COMPLAINTSKey2, 2);
  }
  pComboBoxREFERENCENUMBER->SetCurSel(0);
//
//  Route
//
  int numRoutes = SetUpRouteList(m_hWnd, CCREPORTS_ROUTE, NO_RECORD);
  if(numRoutes == 0)
  {
    OnCancel();
    return TRUE;
  }
  pComboBoxROUTE->SetCurSel(0);
//
//  Employees
//
  int numEmployees = 0;
  
  rcode2 = btrieve(B_GETFIRST, TMS_DRIVERS, &DRIVERS, &DRIVERSKey1, 1);
  while(rcode2 == 0)
  {
    strncpy(tempString, DRIVERS.lastName, DRIVERS_LASTNAME_LENGTH);
    trim(tempString, DRIVERS_LASTNAME_LENGTH);
    strcat(tempString, ", ");
    strncpy(szarString, DRIVERS.firstName, DRIVERS_FIRSTNAME_LENGTH);
    trim(szarString, DRIVERS_FIRSTNAME_LENGTH);
    strcat(tempString, szarString);
    nI = pComboBoxEMPLOYEE->AddString(tempString);
    pComboBoxEMPLOYEE->SetItemData(nI, DRIVERS.recordID);
    rcode2 = btrieve(B_GETNEXT, TMS_DRIVERS, &DRIVERS, &DRIVERSKey1, 1);
    numEmployees++;
  }
  if(numEmployees == 0)
    pButtonBYEMPLOYEE->EnableWindow(FALSE);
  else
    pComboBoxEMPLOYEE->SetCurSel(0);
//
//  Customers
//
  CString s;

  s.LoadString(TEXT_247);  // "All customers"
  nI = pComboBoxCUSTOMER->AddString(s);
  pComboBoxCUSTOMER->SetItemData(nI, -2);
  s.LoadString(TEXT_237);  // "An anonymous caller"
  nI = pComboBoxCUSTOMER->AddString(s);
  pComboBoxCUSTOMER->SetItemData(nI, -1);
  rcode2 = btrieve(B_GETFIRST, TMS_CUSTOMERS, &CUSTOMERS, &CUSTOMERSKey1, 1);
  while(rcode2 == 0)
  {
    strncpy(tempString, CUSTOMERS.lastName, CUSTOMERS_LASTNAME_LENGTH);
    trim(tempString, CUSTOMERS_LASTNAME_LENGTH);
    strcat(tempString, ", ");
    strncpy(szarString, CUSTOMERS.firstName, CUSTOMERS_FIRSTNAME_LENGTH);
    trim(szarString, CUSTOMERS_FIRSTNAME_LENGTH);
    strcat(tempString, szarString);
    nI = pComboBoxCUSTOMER->AddString(tempString);
    pComboBoxCUSTOMER->SetItemData(nI, CUSTOMERS.recordID);
    rcode2 = btrieve(B_GETNEXT, TMS_CUSTOMERS, &CUSTOMERS, &CUSTOMERS, 1);
  }
  pComboBoxCUSTOMER->SetCurSel(0);
//
//  Text file based
//
  CCCSupervisor::PopulateComboBox(m_ActionCodes, m_numActionCodes, pComboBoxDETERMINATIONCODE);
  CCCSupervisor::PopulateComboBox(m_Categories, m_numCategories, pComboBoxCATEGORY);
  CCCSupervisor::PopulateComboBox(m_ClassificationCodes, m_numClassificationCodes, pComboBoxCLASSIFICATIONCODE);
  CCCSupervisor::PopulateComboBox(m_Departments, m_numDepartments, pComboBoxDEPARTMENT);
//
//  Default to "By reference number"
//
  BOOL bRetVal;

  pButtonBYREFERENCENUMBER->SetCheck(TRUE);
  pComboBoxREFERENCENUMBER->EnableWindow(TRUE);
  bRetVal = TRUE;
//
//  The "sort by" dropdown has its values defined in the
//  resource file.  Default to the first entry (Reference #)
//
  pComboBoxSORTBY->SetCurSel(0);
//
//  Set today's date on the Until Date picker
//
  COleVariant v;
  CTime time = CTime::GetCurrentTime();

  long day = time.GetDay();
  long month = time.GetMonth();
  long year = time.GetYear();

  v = year;
  pDTPickerUNTILDATE->SetYear(v);
  v = month;
  pDTPickerUNTILDATE->SetMonth(v);
  v = day;
  pDTPickerUNTILDATE->SetDay(v);

  m_pCCMRPT02 = NULL;

  return(bRetVal);
}

void CCCReports::OnOK() 
{
  CString s;
  CString INISection = "Reports";
  CString INITitleReportName = "CCReportName";
  CString INITitleDateRange = "CCDateRange";
  DWORD  tempStringLength = TEMPSTRING_LENGTH;
  DWORD  type = REG_SZ;
  HKEY  hKey;
  long  result;
  long  recordID;
  char  szReportName[TMSRPT_REPORTNAME_LENGTH + 1];
  char  szReportFileName[TMSRPT_REPORTFILENAME_LENGTH + 1];
  char  szReportDataFile[TMSRPT_REPORTFILENAME_LENGTH + 1];
  int   nI, nJ;
  int   rcode2;
//
//  Determine the report
//
  strcpy(szReportName, "");
  strcpy(szReportFileName, szReportTemplateFolder);
  if(m_flags & CCREPORTS_DETAIL)
  {
    strcat(szReportFileName, "\\CCMRPT03.rpt");
  }
  else if(m_flags & CCREPORTS_SUMMARY)
  {
    strcat(szReportFileName, "\\CCMRPT02.rpt");
  }
  else
  {
    strcat(szReportFileName, "\\CCMRPT01.rpt");
  }
  strcpy(szReportDataFile, szReportsTempFolder);
//
//  And how it's sorted
//
  m_sortIndex = pComboBoxSORTBY->GetCurSel();
  CCMRPT02_SortIndex = m_sortIndex;
//
//  Verify that the report template exists
//
  if(strcmp(szReportFileName, "") == 0)
  {
    OnCancel();
    return;
  }
//
//  There was a template - check it out
//
  FILE *fp;

  if((fp = fopen(szReportFileName, "r")) == NULL)
  {
    TMSError((HWND)NULL, MB_ICONSTOP, ERROR_182, (HANDLE)NULL);
    OnCancel();
    return;
  }
  fclose(fp);
//
//  Open the output file
//
  if(m_flags & CCREPORTS_DETAIL)
  {
    strcat(szReportDataFile, "\\CCMRPT03.txt");
  }
  else if(m_flags & CCREPORTS_SUMMARY)
  {
    strcat(szReportDataFile, "\\CCMRPT02.txt");
  }
  else
  {
    strcat(szReportDataFile, "\\CCMRPT01.txt");
  }
  m_hfOutputFile = _lcreat(szReportDataFile, 0);
  if(m_hfOutputFile == HFILE_ERROR)
  {
    CString error;

    error.LoadString(ERROR_202);
    sprintf(tempString, error, szReportDataFile);
    MessageBeep(MB_ICONSTOP);
    MessageBox(tempString, TMS, MB_ICONSTOP);
    OnCancel();
    return;
  }
//
//  Get the date range
//
  COleVariant v;
  long fromYear, fromMonth, fromDay;
  long untilYear, untilMonth, untilDay;

  v = pDTPickerFROMDATE->GetDay();
  fromDay = v.lVal;
  v = pDTPickerFROMDATE->GetMonth();
  fromMonth = v.lVal;
  v = pDTPickerFROMDATE->GetYear();
  fromYear = v.lVal;
  v = pDTPickerUNTILDATE->GetDay();
  untilDay = v.lVal;
  v = pDTPickerUNTILDATE->GetMonth();
  untilMonth = v.lVal;
  v = pDTPickerUNTILDATE->GetYear();
  untilYear = v.lVal;

  long fromDate = fromYear * 10000 + fromMonth * 100 + fromDay;
  long untilDate = untilYear * 10000 + untilMonth * 100 + untilDay;
//
//  By reference number
//
  if(pButtonBYREFERENCENUMBER->GetCheck())
  {
    strcat(szReportName, "by Specific Reference Number ");
    nI = pComboBoxREFERENCENUMBER->GetCurSel();
    COMPLAINTSKey0.recordID = pComboBoxREFERENCENUMBER->GetItemData(nI);
    rcode2 = btrieve(B_GETEQUAL, TMS_COMPLAINTS, &COMPLAINTS, &COMPLAINTSKey0, 0);
    if(rcode2 == 0 || rcode2 == 22)
    {
      GenerateOutputLine(&COMPLAINTS);
    }
  }
//
//  By customer
//
  else if(pButtonBYCUSTOMER->GetCheck())
  {
    strcat(szReportName, "Calls by Customer ");
    nI = pComboBoxCUSTOMER->GetCurSel();
    recordID = pComboBoxCUSTOMER->GetItemData(nI);
    if(recordID == -2) // All customers
    {
      rcode2 = btrieve(B_GETFIRST, TMS_COMPLAINTS, &COMPLAINTS, &COMPLAINTSKey1, 1);
      while(rcode2 == 0 || rcode2 == 22)
      {
        if(!(COMPLAINTS.flags & COMPLAINTS_FLAG_CANCELLED))
        {
          if(COMPLAINTS.dateOfOccurrence >= fromDate &&
                COMPLAINTS.dateOfOccurrence <= untilDate)
            GenerateOutputLine(&COMPLAINTS);
        }
        rcode2 = btrieve(B_GETNEXT, TMS_COMPLAINTS, &COMPLAINTS, &COMPLAINTSKey1, 1);
      }
    }
    else
    {
      COMPLAINTSKey1.CUSTOMERSrecordID = recordID;
      rcode2 = btrieve(B_GETGREATEROREQUAL, TMS_COMPLAINTS, &COMPLAINTS, &COMPLAINTSKey1, 1);
      while((rcode2 == 0 || rcode2 == 22) &&
            COMPLAINTS.CUSTOMERSrecordID == recordID)
      {
        if(!(COMPLAINTS.flags & COMPLAINTS_FLAG_CANCELLED))
        {
          if(COMPLAINTS.dateOfOccurrence >= fromDate &&
                COMPLAINTS.dateOfOccurrence <= untilDate)
            GenerateOutputLine(&COMPLAINTS);
        }
        rcode2 = btrieve(B_GETNEXT, TMS_COMPLAINTS, &COMPLAINTS, &COMPLAINTSKey1, 1);
      }
    }
  }
//
//  By route
//
  else if(pButtonBYROUTE->GetCheck())
  {
    strcat(szReportName, "Calls by Route ");
    nI = pComboBoxROUTE->GetCurSel();
    recordID = pComboBoxROUTE->GetItemData(nI);
    COMPLAINTSKey6.ROUTESrecordID = recordID;
    rcode2 = btrieve(B_GETGREATEROREQUAL, TMS_COMPLAINTS, &COMPLAINTS, &COMPLAINTSKey6, 6);
    while((rcode2 == 0 || rcode2 == 22) &&
          COMPLAINTS.ROUTESrecordID == recordID)
    {
      if(!(COMPLAINTS.flags & COMPLAINTS_FLAG_CANCELLED))
      {
        if(COMPLAINTS.dateOfOccurrence >= fromDate &&
              COMPLAINTS.dateOfOccurrence <= untilDate)
          GenerateOutputLine(&COMPLAINTS);
      }
      rcode2 = btrieve(B_GETNEXT, TMS_COMPLAINTS, &COMPLAINTS, &COMPLAINTSKey6, 6);
    }
  }
//
//  By category
//
  else if(pButtonBYCATEGORY->GetCheck())
  {
    strcat(szReportName, "Calls by Category ");
    nI = pComboBoxCATEGORY->GetCurSel();
    recordID = pComboBoxCATEGORY->GetItemData(nI);
    COMPLAINTSKey5.dateOfOccurrence = fromDate;
    rcode2 = btrieve(B_GETGREATEROREQUAL, TMS_COMPLAINTS, &COMPLAINTS, &COMPLAINTSKey5, 5);
    while((rcode2 == 0 || rcode2 == 22) &&
          COMPLAINTS.dateOfOccurrence >= fromDate &&
          COMPLAINTS.dateOfOccurrence <= untilDate)
    {
      if(!(COMPLAINTS.flags & COMPLAINTS_FLAG_CANCELLED))
      {
        if(COMPLAINTS.categoryIndex == recordID)
          GenerateOutputLine(&COMPLAINTS);
      }
      rcode2 = btrieve(B_GETNEXT, TMS_COMPLAINTS, &COMPLAINTS, &COMPLAINTSKey5, 5);
    }
  }
//
//  By classification code
//
  else if(pButtonBYCLASSIFICATIONCODE->GetCheck())
  {
    strcat(szReportName, "Calls by Classification ");
    nI = pComboBoxCLASSIFICATIONCODE->GetCurSel();
    recordID = pComboBoxCLASSIFICATIONCODE->GetItemData(nI);
    COMPLAINTSKey5.dateOfOccurrence = fromDate;
    rcode2 = btrieve(B_GETGREATEROREQUAL, TMS_COMPLAINTS, &COMPLAINTS, &COMPLAINTSKey5, 5);
    while((rcode2 == 0 || rcode2 == 22) &&
          COMPLAINTS.dateOfOccurrence >= fromDate &&
          COMPLAINTS.dateOfOccurrence <= untilDate)
    {
      if(!(COMPLAINTS.flags & COMPLAINTS_FLAG_CANCELLED))
      {
        if(COMPLAINTS.classificationIndex == recordID)
          GenerateOutputLine(&COMPLAINTS);
      }
      rcode2 = btrieve(B_GETNEXT, TMS_COMPLAINTS, &COMPLAINTS, &COMPLAINTSKey5, 5);
    }
  }
//
//  By action code
//
  else if(pButtonBYDETERMINATIONCODE->GetCheck())
  {
    strcat(szReportName, "Calls by Action ");
    nI = pComboBoxDETERMINATIONCODE->GetCurSel();
    recordID = pComboBoxDETERMINATIONCODE->GetItemData(nI);
    COMPLAINTSKey5.dateOfOccurrence = fromDate;
    rcode2 = btrieve(B_GETGREATEROREQUAL, TMS_COMPLAINTS, &COMPLAINTS, &COMPLAINTSKey5, 5);
    while((rcode2 == 0 || rcode2 == 22) &&
          COMPLAINTS.dateOfOccurrence >= fromDate &&
          COMPLAINTS.dateOfOccurrence <= untilDate)
    {
      if(!(COMPLAINTS.flags & COMPLAINTS_FLAG_CANCELLED))
      {
        if(COMPLAINTS.actionIndex == recordID)
          GenerateOutputLine(&COMPLAINTS);
      }
      rcode2 = btrieve(B_GETNEXT, TMS_COMPLAINTS, &COMPLAINTS, &COMPLAINTSKey5, 5);
    }
  }
//
//  By employee
//
  else if(pButtonBYEMPLOYEE->GetCheck())
  {
    strcat(szReportName, "Calls by Employee ");
    nI = pComboBoxEMPLOYEE->GetCurSel();
    recordID = pComboBoxEMPLOYEE->GetItemData(nI);
    COMPLAINTSKey7.supDRIVERSrecordID = recordID;
    rcode2 = btrieve(B_GETGREATEROREQUAL, TMS_COMPLAINTS, &COMPLAINTS, &COMPLAINTSKey7, 7);
    while((rcode2 == 0 || rcode2 == 22) &&
          COMPLAINTS.supDRIVERSrecordID == recordID)
    {
      if(!(COMPLAINTS.flags & COMPLAINTS_FLAG_CANCELLED))
      {
        if(COMPLAINTS.dateOfOccurrence >= fromDate &&
              COMPLAINTS.dateOfOccurrence <= untilDate)
          GenerateOutputLine(&COMPLAINTS);
      }
      rcode2 = btrieve(B_GETNEXT, TMS_COMPLAINTS, &COMPLAINTS, &COMPLAINTSKey7, 7);
    }
  }
//
//  By department
//
  else if(pButtonBYDEPARTMENT->GetCheck())
  {
    strcat(szReportName, "Calls by Department ");
    nI = pComboBoxDEPARTMENT->GetCurSel();
    recordID = pComboBoxDEPARTMENT->GetItemData(nI);
    COMPLAINTSKey5.dateOfOccurrence = fromDate;
    rcode2 = btrieve(B_GETGREATEROREQUAL, TMS_COMPLAINTS, &COMPLAINTS, &COMPLAINTSKey5, 5);
    while((rcode2 == 0 || rcode2 == 22) &&
          COMPLAINTS.dateOfOccurrence >= fromDate &&
          COMPLAINTS.dateOfOccurrence <= untilDate)
    {
      if(!(COMPLAINTS.flags & COMPLAINTS_FLAG_CANCELLED))
      {
        if(COMPLAINTS.actionIndex == recordID)
          GenerateOutputLine(&COMPLAINTS);
      }
      rcode2 = btrieve(B_GETNEXT, TMS_COMPLAINTS, &COMPLAINTS, &COMPLAINTSKey5, 5);
    }
  }
//
//  Unresolved
//
  else
  {
    strcat(szReportName, "Unresolved Calls ");
    COMPLAINTSKey5.dateOfOccurrence = fromDate;
    rcode2 = btrieve(B_GETGREATEROREQUAL, TMS_COMPLAINTS, &COMPLAINTS, &COMPLAINTSKey5, 5);
    while((rcode2 == 0 || rcode2 == 22) &&
          COMPLAINTS.dateOfOccurrence >= fromDate &&
          COMPLAINTS.dateOfOccurrence <= untilDate)
    {
      if(!(COMPLAINTS.flags & COMPLAINTS_FLAG_CANCELLED))
      {
        if(!(COMPLAINTS.flags & COMPLAINTS_FLAG_CLOSED))
          GenerateOutputLine(&COMPLAINTS);
      }
      rcode2 = btrieve(B_GETNEXT, TMS_COMPLAINTS, &COMPLAINTS, &COMPLAINTSKey5, 5);
    }
  }
//
//  Sort and Generate the output if DETAIL or SUMMARY
//
  if((m_flags & CCREPORTS_DETAIL) || (m_flags & CCREPORTS_SUMMARY))
  {
    qsort((void *)m_pCCMRPT02, m_numCCMRPT02, sizeof(CCMRPT02Def), sort_CCMRPT02);
    GenerateOutputLine02();
  }
//
//  Attach the "Sorted by..." string to the Report Name
//
  if(pButtonBYREFERENCENUMBER->GetCheck())
  {
    sprintf(tempString, "");
  }
  else
  {
    pComboBoxSORTBY->GetLBText(m_sortIndex, szarString);
    sprintf(tempString, "(Sorted by %s)", szarString);
  }
  strcat(szReportName, tempString);
  WritePrivateProfileString(INISection, INITitleReportName, szReportName, TMSINIFile);

//
//  Write out the date range
//
  if(pButtonBYREFERENCENUMBER->GetCheck())
  {
    sprintf(tempString, "Not applicable");
  }
  else
  {
    sprintf(tempString, "%ld/%ld/%ld - %ld/%ld/%ld",
          fromMonth, fromDay, fromYear, untilMonth, untilDay, untilYear);
  }
  WritePrivateProfileString(INISection, INITitleDateRange, tempString, TMSINIFile);
//
//  Close the file
//
  _lclose(m_hfOutputFile);
//
//  Fire the report
//
  CString sRegKey = "Software\\ODBC\\ODBC.INI\\TMS MSJet Text";

  result = RegOpenKeyEx(HKEY_CURRENT_USER, sRegKey, 0, KEY_SET_VALUE, &hKey);
  if(result != 0)
  {
    LoadString(hInst, ERROR_240, szFormatString, sizeof(szFormatString));
    sprintf(tempString, szFormatString, sRegKey);
    MessageBeep(MB_ICONSTOP);
    MessageBox(tempString, TMS, MB_OK);
  }
//
//  Set the registry entries so Crystal knows where to look
//
  else
  {
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
    }
//
//  Found the INI file
//
    else
    {
      CopyFile(fileName[0], fileName[1], FALSE);

      CCrystal dlg(this, szReportFileName, szReportName);

      dlg.DoModal();
    }
  }
}

void CCCReports::OnCancel() 
{
  TMSHeapFree(m_pCCMRPT02);
	CDialog::OnCancel();
}

void CCCReports::OnHelp() 
{
}

void CCCReports::OnByreferencenumber() 
{
  DisableDropdowns();
  pComboBoxREFERENCENUMBER->EnableWindow(TRUE);
}

void CCCReports::OnBycustomer() 
{
  DisableDropdowns();
  pComboBoxCUSTOMER->EnableWindow(TRUE);
}

void CCCReports::OnByroute() 
{
  DisableDropdowns();
  pComboBoxROUTE->EnableWindow(TRUE);
}

void CCCReports::OnBycategory() 
{
  DisableDropdowns();
  pComboBoxCATEGORY->EnableWindow(TRUE);
}

void CCCReports::OnByclassificationcode() 
{
  DisableDropdowns();
  pComboBoxCLASSIFICATIONCODE->EnableWindow(TRUE);
}

void CCCReports::OnBydeterminationcode() 
{
  DisableDropdowns();
  pComboBoxDETERMINATIONCODE->EnableWindow(TRUE);
}

void CCCReports::OnByemployee() 
{
  DisableDropdowns();
  pComboBoxEMPLOYEE->EnableWindow(TRUE);
}

void CCCReports::OnBydepartment() 
{
  DisableDropdowns();
  pComboBoxDEPARTMENT->EnableWindow(TRUE);
}

void CCCReports::OnUnresolved() 
{
  DisableDropdowns();
}

void CCCReports::DisableDropdowns()
{
  pComboBoxREFERENCENUMBER->EnableWindow(FALSE);
  pComboBoxCUSTOMER->EnableWindow(FALSE);
  pComboBoxROUTE->EnableWindow(FALSE);
  pComboBoxCATEGORY->EnableWindow(FALSE);
  pComboBoxCLASSIFICATIONCODE->EnableWindow(FALSE);
  pComboBoxDETERMINATIONCODE->EnableWindow(FALSE);
  pComboBoxEMPLOYEE->EnableWindow(FALSE);
  pComboBoxDEPARTMENT->EnableWindow(FALSE);
}

void CCCReports::GenerateOutputLine(COMPLAINTSDef *pCOMPLAINTS)
{
  if((m_flags & CCREPORTS_DETAIL) || (m_flags & CCREPORTS_SUMMARY))
  {
    StoreRelevantData(pCOMPLAINTS);
  }
  else
  {
    GenerateOutputLine01(pCOMPLAINTS);
  }
}

void CCCReports::GenerateOutputLine01(COMPLAINTSDef *pCOMPLAINTS)
{
  char outputString[TEMPSTRING_LENGTH];
  int  rcode2;
  int  nI;
//
//  Reference number
//
  CString referenceNumber;
  
  strncpy(tempString, pCOMPLAINTS->referenceNumber, ODBC_CCMRPT01_COMPLAINT_LENGTH);
  trim(tempString, ODBC_CCMRPT01_COMPLAINT_LENGTH);
  referenceNumber = tempString;
//
//  Incident date
//
  if(pCOMPLAINTS->dateOfOccurrence == 0)
  {
    return;
  }
  long year, month, day;

  GetYMD(pCOMPLAINTS->dateOfOccurrence, &year, &month, &day);

  CTime   t(year, month, day, 0, 0, 0);
  CString date = t.Format(_T("%m/%d/%Y"));
//
//  Classification
//
  CString classification = "";

  int numClassifications = pComboBoxCLASSIFICATIONCODE->GetCount();

  for(nI = 0; nI < numClassifications; nI++)
  {
    if((long)pComboBoxCLASSIFICATIONCODE->GetItemData(nI) == pCOMPLAINTS->classificationIndex)
    {
      pComboBoxCLASSIFICATIONCODE->GetLBText(nI, tempString);
      trim(tempString, ODBC_CCMRPT01_CLASSIFICATION_LENGTH);
      classification = tempString;
      break;
    } 
  }
//
//  Route
//
  CString routeNumber;

  ROUTESKey0.recordID = pCOMPLAINTS->ROUTESrecordID;
  rcode2 = btrieve(B_GETEQUAL, TMS_ROUTES, &ROUTES, &ROUTESKey0, 0);
  if(rcode2 != 0)
    routeNumber = "";
  else
  {
    strncpy(tempString, ROUTES.number, ODBC_CCMRPT01_ROUTENUMBER_LENGTH);
    trim(tempString, ODBC_CCMRPT01_ROUTENUMBER_LENGTH);
    routeNumber = tempString;
  }
//
//  Location
//
  CString location;

  strncpy(tempString, pCOMPLAINTS->location, ODBC_CCMRPT01_LOCATION_LENGTH);
  trim(tempString, ODBC_CCMRPT01_LOCATION_LENGTH);
  location = tempString;
//
//  Employee
//
  CString employee;

  DRIVERSKey0.recordID = pCOMPLAINTS->DRIVERSrecordID;
  rcode2 = btrieve(B_GETEQUAL, TMS_DRIVERS, &DRIVERS, &DRIVERSKey0, 0);
  if(rcode2 != 0)
    strcpy(tempString, "General/Unknown");
  else
  {
    strncpy(tempString, DRIVERS.lastName, DRIVERS_LASTNAME_LENGTH);
    trim(tempString, DRIVERS_LASTNAME_LENGTH);
    strcat(tempString, ", ");
    strncpy(szarString, DRIVERS.firstName, DRIVERS_FIRSTNAME_LENGTH);
    trim(szarString, DRIVERS_FIRSTNAME_LENGTH);
    strcat(tempString, szarString);
  }
  trim(tempString, ODBC_CCMRPT01_EMPLOYEE_LENGTH);
  employee = tempString;
//
//  Customer
//
  CString customer;

  CUSTOMERSKey0.recordID = pCOMPLAINTS->CUSTOMERSrecordID;
  rcode2 = btrieve(B_GETEQUAL, TMS_CUSTOMERS, &CUSTOMERS, &CUSTOMERSKey0, 0);
  if(rcode2 != 0)
    strcpy(tempString, "Anonymous/Unknown");
  else
  {
    strncpy(tempString, CUSTOMERS.lastName, CUSTOMERS_LASTNAME_LENGTH);
    trim(tempString, CUSTOMERS_LASTNAME_LENGTH);
    strcat(tempString, ", ");
    strncpy(szarString, CUSTOMERS.firstName, CUSTOMERS_FIRSTNAME_LENGTH);
    trim(szarString, CUSTOMERS_FIRSTNAME_LENGTH);
    strcat(tempString, szarString);
  }
  trim(tempString, ODBC_CCMRPT01_CUSTOMER_LENGTH);
  customer = tempString;
//
//  Referred to
//
  CString referredTo;

  strncpy(tempString, pCOMPLAINTS->assignedTo, COMPLAINTS_ASSIGNEDTO_LENGTH);
  trim(tempString, COMPLAINTS_ASSIGNEDTO_LENGTH);
  referredTo = tempString;
//
//  Action
//
  CString action = "";

  int numActionCodes = pComboBoxDETERMINATIONCODE->GetCount();

  for(nI = 0; nI < numActionCodes; nI++)
  {
    if((long)pComboBoxDETERMINATIONCODE->GetItemData(nI) == pCOMPLAINTS->actionIndex)
    {
      pComboBoxDETERMINATIONCODE->GetLBText(nI, tempString);
      trim(tempString, ODBC_CCMRPT01_ACTION_LENGTH);
      action = tempString;
      break;
    } 
  }
//
//  Sort field
//
  CString sortField;

  switch (m_sortIndex)
  {
    case 0:
      sortField = referenceNumber;
      break;

    case 1:
      sortField = date;
      break;

    case 2:
      sortField = classification;
      break;

    case 3:
      sortField = routeNumber;
      break;

    case 4:
      sortField = location;
      break;

    case 5:
      sortField = employee;
      break;

    case 6:
      sortField = customer;
      break;

    case 7:
      sortField = referredTo;
      break;

    case 8:
      sortField = action;
      break;
  }
  if(m_sortIndex >= 2)
  {
    strcpy(tempString, sortField);
    pad(tempString, 20);
    tempString[20] = '\0';
    strcat(tempString, date);
    sortField = tempString;
  }
//
//  Write it out
//
  sprintf(outputString, "%-12s\t%-10s\t%-20s\t%8s\t%-16s\t%-16s\t%-16s\t%-16s\t%-16s\t%30s\r\n", 
        referenceNumber, date, classification, routeNumber, location,
        employee, customer, referredTo, action, sortField);
  _lwrite(m_hfOutputFile, outputString, strlen(outputString));
}

void CCCReports::StoreRelevantData(COMPLAINTSDef *pCOMPLAINTS)
{
  int  rcode2;

  m_pCCMRPT02[m_numCCMRPT02].recordID = pCOMPLAINTS->recordID;

  switch (m_sortIndex)
  {
    case 0:  // Reference number
      strncpy(m_pCCMRPT02[m_numCCMRPT02].SortFields.referenceNumber, pCOMPLAINTS->referenceNumber, ODBC_CCMRPT01_COMPLAINT_LENGTH);
      trim(m_pCCMRPT02[m_numCCMRPT02].SortFields.referenceNumber, ODBC_CCMRPT01_COMPLAINT_LENGTH);
      break;

    case 1:  // Date
      m_pCCMRPT02[m_numCCMRPT02].SortFields.incidentDate = pCOMPLAINTS->dateOfOccurrence;
      break;

    case 2:  // Classification
      strcpy(m_pCCMRPT02[m_numCCMRPT02].SortFields.classification, "");
      break;

    case 3:  // Route number
      ROUTESKey0.recordID = pCOMPLAINTS->ROUTESrecordID;
      rcode2 = btrieve(B_GETEQUAL, TMS_ROUTES, &ROUTES, &ROUTESKey0, 0);
      if(rcode2 != 0)
      {
        strcpy(m_pCCMRPT02[m_numCCMRPT02].SortFields.routeNumber, "");
      }
      else
      {
        strncpy(m_pCCMRPT02[m_numCCMRPT02].SortFields.routeNumber, ROUTES.number, ODBC_CCMRPT01_ROUTENUMBER_LENGTH);
        trim(m_pCCMRPT02[m_numCCMRPT02].SortFields.routeNumber, ODBC_CCMRPT01_ROUTENUMBER_LENGTH);
      }
      break;

    case 4:  // Location
      strncpy(m_pCCMRPT02[m_numCCMRPT02].SortFields.location, pCOMPLAINTS->location, ODBC_CCMRPT01_LOCATION_LENGTH);
      trim(m_pCCMRPT02[m_numCCMRPT02].SortFields.location, ODBC_CCMRPT01_LOCATION_LENGTH);
      break;

    case 5:  // Employee
      DRIVERSKey0.recordID = pCOMPLAINTS->DRIVERSrecordID;
      rcode2 = btrieve(B_GETEQUAL, TMS_DRIVERS, &DRIVERS, &DRIVERSKey0, 0);
      if(rcode2 != 0)
      {
        strcpy(tempString, "General/Unknown");
      }
      else
      {
        strncpy(tempString, DRIVERS.lastName, DRIVERS_LASTNAME_LENGTH);
        trim(tempString, DRIVERS_LASTNAME_LENGTH);
        strcat(tempString, ", ");
        strncpy(szarString, DRIVERS.firstName, DRIVERS_FIRSTNAME_LENGTH);
        trim(szarString, DRIVERS_FIRSTNAME_LENGTH);
        strcat(tempString, szarString);
      }
      trim(tempString, ODBC_CCMRPT01_EMPLOYEE_LENGTH);
      strcpy(m_pCCMRPT02[m_numCCMRPT02].SortFields.employee, tempString);
      break;

    case 6:  // Customer
      CUSTOMERSKey0.recordID = pCOMPLAINTS->CUSTOMERSrecordID;
      rcode2 = btrieve(B_GETEQUAL, TMS_CUSTOMERS, &CUSTOMERS, &CUSTOMERSKey0, 0);
      if(rcode2 != 0)
      {
        strcpy(tempString, "Anonymous/Unknown");
      }
      else
      {
        strncpy(tempString, CUSTOMERS.lastName, CUSTOMERS_LASTNAME_LENGTH);
        trim(tempString, CUSTOMERS_LASTNAME_LENGTH);
        strcat(tempString, ", ");
        strncpy(szarString, CUSTOMERS.firstName, CUSTOMERS_FIRSTNAME_LENGTH);
        trim(szarString, CUSTOMERS_FIRSTNAME_LENGTH);
        strcat(tempString, szarString);
      }
      trim(tempString, ODBC_CCMRPT01_CUSTOMER_LENGTH);
      strcpy(m_pCCMRPT02[m_numCCMRPT02].SortFields.customer, tempString);
      break;

    case 7:  // Referred to
      strncpy(m_pCCMRPT02[m_numCCMRPT02].SortFields.referredTo, pCOMPLAINTS->assignedTo, COMPLAINTS_ASSIGNEDTO_LENGTH);
      trim(m_pCCMRPT02[m_numCCMRPT02].SortFields.referredTo, COMPLAINTS_ASSIGNEDTO_LENGTH);
      break;

    case 8:  // Action
      strcpy(m_pCCMRPT02[m_numCCMRPT02].SortFields.action, "");
      break;
  }
  m_numCCMRPT02++;
}

void CCCReports::GenerateOutputLine02()
{
  char outputString[TEMPSTRING_LENGTH];
  int  rcode2;
  int  nI, nJ;
  int  seq = 0;

  for(nI = 0; nI < m_numCCMRPT02; nI++)
  {
//
//  Get the record
//
    COMPLAINTSKey0.recordID = m_pCCMRPT02[nI].recordID;
    recordLength[TMS_COMPLAINTS] = COMPLAINTS_TOTAL_LENGTH;
    rcode2 = btrieve(B_GETEQUAL, TMS_COMPLAINTS, pComplaintText, &COMPLAINTSKey0, 0);
    recordLength[TMS_COMPLAINTS] = COMPLAINTS_FIXED_LENGTH;
    if(rcode2 != 0)
    {
      continue;
    }
    memcpy(&COMPLAINTS, pComplaintText, COMPLAINTS_FIXED_LENGTH);
//
//  Reference number
//
    if(COMPLAINTS.dateOfOccurrence == 0)
    {
      continue;
    }
    CString referenceNumber;

    strncpy(tempString, COMPLAINTS.referenceNumber, ODBC_CCMRPT01_COMPLAINT_LENGTH);
    trim(tempString, ODBC_CCMRPT01_COMPLAINT_LENGTH);
    referenceNumber = tempString;
    if(!(m_flags & CCREPORTS_SUMMARY))
    {
      sprintf(outputString, "%d\t%ld\t%16s: %s\r\n", seq++, COMPLAINTS.recordID, "Reference Number", referenceNumber);
      _lwrite(m_hfOutputFile, outputString, strlen(outputString));
    }
//
//  Summary
//
    if(m_flags & CCREPORTS_SUMMARY)
    {
//
//  Incident date
//
      long year, month, day;
 
      GetYMD(COMPLAINTS.dateOfOccurrence, &year, &month, &day);

      CTime   ti(year, month, day, 0, 0, 0);
      CString incidentDate = ti.Format(_T("%m/%d/%Y"));
//
//  Date entered into the system
//
      GetYMD(COMPLAINTS.dateOfComplaint, &year, &month, &day);

      CTime   te(year, month, day, 0, 0, 0);
      CString entryDate = te.Format(_T("%m/%d/%Y"));
//
//  Date closed
//
      CString closedDate = "Open";

      GetYMD(COMPLAINTS.dateClosed, &year, &month, &day);

      CTime tc;
 
      if(COMPLAINTS.flags & COMPLAINTS_FLAG_CLOSED)
      {
        CTime tt(year, month, day, 0, 0, 0);

        tc = tt;
        closedDate = tc.Format(_T("%m/%d/%Y"));
      }

//
//  Classification
//
      CString classification = "";

      int numClassifications = pComboBoxCLASSIFICATIONCODE->GetCount();

      for(nJ = 0; nJ < numClassifications; nJ++)
      {
        if((long)pComboBoxCLASSIFICATIONCODE->GetItemData(nJ) == COMPLAINTS.classificationIndex)
        {
          pComboBoxCLASSIFICATIONCODE->GetLBText(nJ, tempString);
          classification = tempString;
          break;
        } 
      }
//
//  Route
//
      CString routeNumber = "";

      ROUTESKey0.recordID = COMPLAINTS.ROUTESrecordID;
      rcode2 = btrieve(B_GETEQUAL, TMS_ROUTES, &ROUTES, &ROUTESKey0, 0);
      if(rcode2 != 0)
      {
        routeNumber = "";
      }
      else
      {
        strncpy(tempString, ROUTES.number, ROUTES_NUMBER_LENGTH);
        trim(tempString, ROUTES_NUMBER_LENGTH);
        routeNumber = tempString;
      }
//
//  Location
//
      CString location = "";

      strncpy(tempString, COMPLAINTS.location, COMPLAINTS_LOCATION_LENGTH);
      trim(tempString, COMPLAINTS_LOCATION_LENGTH);
      location = tempString;
//
//  Employee
//
      CString employee = "";
 
      DRIVERSKey0.recordID = COMPLAINTS.DRIVERSrecordID;
      rcode2 = btrieve(B_GETEQUAL, TMS_DRIVERS, &DRIVERS, &DRIVERSKey0, 0);
      if(rcode2 != 0)
      {
        strcpy(tempString, "General/Unknown");
      }
      else
      {
        strncpy(tempString, DRIVERS.lastName, DRIVERS_LASTNAME_LENGTH);
        trim(tempString, DRIVERS_LASTNAME_LENGTH);
        strcat(tempString, ", ");
        strncpy(szarString, DRIVERS.firstName, DRIVERS_FIRSTNAME_LENGTH);
        trim(szarString, DRIVERS_FIRSTNAME_LENGTH);
        strcat(tempString, szarString);
      }
      trim(tempString, DRIVERS_FIRSTNAME_LENGTH + DRIVERS_LASTNAME_LENGTH + 3);
      employee = tempString;
//
//  Customer
//
      CString customer = "";

      CUSTOMERSKey0.recordID = COMPLAINTS.CUSTOMERSrecordID;
      rcode2 = btrieve(B_GETEQUAL, TMS_CUSTOMERS, &CUSTOMERS, &CUSTOMERSKey0, 0);
      if(rcode2 != 0)
      {
        strcpy(tempString, "Anonymous/Unknown");
      }
      else
      {
        strncpy(tempString, CUSTOMERS.lastName, CUSTOMERS_LASTNAME_LENGTH);
        trim(tempString, CUSTOMERS_LASTNAME_LENGTH);
        strcat(tempString, ", ");
        strncpy(szarString, CUSTOMERS.firstName, CUSTOMERS_FIRSTNAME_LENGTH);
        trim(szarString, CUSTOMERS_FIRSTNAME_LENGTH);
        strcat(tempString, szarString);
      }
      trim(tempString, CUSTOMERS_FIRSTNAME_LENGTH + CUSTOMERS_LASTNAME_LENGTH + 3);
      customer = tempString;
//
//  Referred to
//
      CString referredTo = "";

      strncpy(tempString, COMPLAINTS.assignedTo, COMPLAINTS_ASSIGNEDTO_LENGTH);
      trim(tempString, COMPLAINTS_ASSIGNEDTO_LENGTH);
      referredTo = tempString;
//
//  Action
//
      CString action = "";

      int numActionCodes = pComboBoxDETERMINATIONCODE->GetCount();
      for(nJ = 0; nJ < numActionCodes; nJ++)
      {
        if((long)pComboBoxDETERMINATIONCODE->GetItemData(nJ) == COMPLAINTS.actionIndex)
        {
          pComboBoxDETERMINATIONCODE->GetLBText(nJ, tempString);
          action = tempString;
          break;
        } 
      }
//
//  Department
//
      CString department = "";

      int numDepartments = pComboBoxDEPARTMENT->GetCount();

      for(nJ = 0; nJ < numDepartments; nJ++)
      {
        if((long)pComboBoxDEPARTMENT->GetItemData(nJ) == COMPLAINTS.departmentIndex)
        {
          pComboBoxDEPARTMENT->GetLBText(nJ, tempString);
          department = tempString;
          break;
        }
      }
//
//  Days open
//
      CTimeSpan daysOpen;

      if(COMPLAINTS.flags & COMPLAINTS_FLAG_CLOSED)
      {
        daysOpen = tc - ti;
      }
      else
      {
        CTime today = CTime::GetCurrentTime();
        daysOpen = today - ti;
      }
//
//  Write it out
//
      sprintf(outputString, "%s\t", referenceNumber);

      strcat(outputString, entryDate);
      strcat(outputString, "\t");

      strcat(outputString, incidentDate);
      strcat(outputString, "\t");

      strcat(outputString, (COMPLAINTS.flags & COMPLAINTS_FLAG_CLOSED ? "Closed" : "Open"));
      strcat(outputString, "\t");

      strcat(outputString, department);
      strcat(outputString, "\t");

      strcat(outputString, referredTo);
      strcat(outputString, "\t");
      
      strcat(outputString, classification);
      strcat(outputString, "\t");
      
      strcat(outputString, routeNumber);
      strcat(outputString, "\t");

      strcat(outputString, location);
      strcat(outputString, "\t");

      strcat(outputString, employee);
      strcat(outputString, "\t");

      strcat(outputString, customer);
      strcat(outputString, "\t");

      strcat(outputString, action);
      strcat(outputString, "\t");

      strcat(outputString, closedDate);
      strcat(outputString, "\t");

      sprintf(tempString, "%ld\t%d\r\n", daysOpen.GetDays(), seq++);
      strcat(outputString, tempString);

      _lwrite(m_hfOutputFile, outputString, strlen(outputString));
    }
    else
    {
      CString strText;
      int len;
      int numLines = pEditLOG->GetLineCount();

      pEditLOG->SetWindowText(&pComplaintText[COMPLAINTS_FIXED_LENGTH]);
      for(nJ = 0; nJ < numLines; nJ++)
      {
        len = pEditLOG->LineLength(pEditLOG->LineIndex(nJ));
        pEditLOG->GetLine(nJ, strText.GetBuffer(len), len);
        strncpy(tempString, strText, len);
        tempString[len] = '\0';
        strText.ReleaseBuffer(len);
        sprintf(outputString, "%d\t%ld\t%s\r\n", seq++, COMPLAINTS.recordID, tempString);
        _lwrite(m_hfOutputFile, outputString, strlen(outputString));
      }
      sprintf(outputString, "%d\t%ld\r\n", seq++, COMPLAINTS.recordID);
      _lwrite(m_hfOutputFile, outputString, strlen(outputString));
//
//  Supervisory entries
//
      sprintf(outputString, "%d\t%ld\t%16s:\r\n", seq++, COMPLAINTS.recordID, "Supervisor Entry");
      _lwrite(m_hfOutputFile, outputString, strlen(outputString));
//
//  Department
//
      if(COMPLAINTS.departmentIndex != NO_RECORD)
      {
        int numEntries = pComboBoxDEPARTMENT->GetCount();

        for(nJ = 0; nJ < numEntries; nJ++)
        {
          if(COMPLAINTS.departmentIndex == (int)pComboBoxDEPARTMENT->GetItemData(nJ))
          {
            pComboBoxDEPARTMENT->GetLBText(nJ, tempString);
            sprintf(outputString, "%d\t%ld\t  Department: %s\r\n", seq++, COMPLAINTS.recordID, tempString);
            _lwrite(m_hfOutputFile, outputString, strlen(outputString));
            break;
          }
        }
      }
//
//  Assigned to
//
      strncpy(tempString, COMPLAINTS.assignedTo, COMPLAINTS_ASSIGNEDTO_LENGTH);
      trim(tempString, COMPLAINTS_ASSIGNEDTO_LENGTH);
      if(strcmp(tempString, "") != 0)
      {
        sprintf(outputString, "%d\t%ld\t  Assigned to: %s\r\n", seq++, COMPLAINTS.recordID, tempString);
        _lwrite(m_hfOutputFile, outputString, strlen(outputString));
      }
//
//  Classification code
//
      if(COMPLAINTS.classificationIndex != NO_RECORD)
      {
        CCCSupervisor::PopulateComboBox(m_ClassificationCodes, m_numClassificationCodes, pComboBoxCB);

        int numEntries = pComboBoxCB->GetCount();

        for(nJ = 0; nJ < numEntries; nJ++)
        {
          if(COMPLAINTS.classificationIndex == (int)pComboBoxCB->GetItemData(nJ))
          {
            pComboBoxCB->GetLBText(nJ, tempString);
            sprintf(outputString, "%d\t%ld\t  Classified as: %s\r\n", seq++, COMPLAINTS.recordID, tempString);
            _lwrite(m_hfOutputFile, outputString, strlen(outputString));
            break;
          }
        }
      }
//
//  Category
//
      if(COMPLAINTS.categoryIndex != NO_RECORD)
      {
        CCCSupervisor::PopulateComboBox(m_Categories, m_numCategories, pComboBoxCB);

        int numEntries = pComboBoxCB->GetCount();

        for(nJ = 0; nJ < numEntries; nJ++)
        {
          if(COMPLAINTS.categoryIndex == (int)pComboBoxCB->GetItemData(nJ))
          {
            pComboBoxCB->GetLBText(nJ, tempString);
            sprintf(outputString, "%d\t%ld\t  Categorized as: %s\r\n", seq++, COMPLAINTS.recordID, tempString);
            _lwrite(m_hfOutputFile, outputString, strlen(outputString));
            break;
          }
        }
      }
//
//  Loss/Damage/Injury
//
      if(COMPLAINTS.supFlags & COMPLAINTS_SUPFLAG_DAMAGE)
      {
        sprintf(outputString, "%d\t%ld\t  Resulted in damage / loss / injury\r\n", seq++, COMPLAINTS.recordID);
        _lwrite(m_hfOutputFile, outputString, strlen(outputString));
      }
//
//  Violation of policy/procedure
//
      if(COMPLAINTS.supFlags & COMPLAINTS_SUPFLAG_VIOLATION)
      {
        sprintf(outputString, "%d\t%ld\t  This customer comment represents a violation of policy or procedure\r\n", seq++, COMPLAINTS.recordID);
        _lwrite(m_hfOutputFile, outputString, strlen(outputString));
      }
//
//  Employee information
//
      if(COMPLAINTS.supDRIVERSrecordID != NO_RECORD)
      {
        DRIVERSKey0.recordID = COMPLAINTS.supDRIVERSrecordID;
        btrieve(B_GETEQUAL, TMS_DRIVERS, &DRIVERS, &DRIVERSKey0, 0);
        strncpy(tempString, DRIVERS.lastName, DRIVERS_LASTNAME_LENGTH);
        trim(tempString, DRIVERS_LASTNAME_LENGTH);
        strcpy(szarString, tempString);
        strcat(szarString, ", ");
        strncpy(tempString, DRIVERS.firstName, DRIVERS_FIRSTNAME_LENGTH);
        trim(tempString, DRIVERS_FIRSTNAME_LENGTH);
        strcat(szarString, tempString);
        sprintf(outputString, "%d\t%ld\t  Employee: %s\r\n", seq++, COMPLAINTS.recordID, szarString);
        _lwrite(m_hfOutputFile, outputString, strlen(outputString));
      }
//
//  Date presented to employee
//
      if(COMPLAINTS.datePresented > 0)
      {
        long yyyy, mm, dd;

        GetYMD(COMPLAINTS.datePresented, &yyyy, &mm, &dd);
        sprintf(outputString, "%d\t%ld\t  Date presented to employee (D/M/Y): %02ld/%02ld/%4ld\r\n", seq++, COMPLAINTS.recordID, dd, mm, yyyy);
        _lwrite(m_hfOutputFile, outputString, strlen(outputString));
      }
//
//  Employee has received similar comments
//
      if(COMPLAINTS.supFlags & COMPLAINTS_SUPFLAG_RECEIVEDSIMILAR)
      {
        sprintf(outputString, "%d\t%ld\t  Employee has received similar comments\r\n", seq++, COMPLAINTS.recordID);
        _lwrite(m_hfOutputFile, outputString, strlen(outputString));
      }
//
//  Customer contact
//
      BOOL bDidOne = FALSE;

      if(COMPLAINTS.supFlags & COMPLAINTS_SUPFLAG_TELEPHONE)
      {
        bDidOne = TRUE;
        sprintf(outputString, "%d\t%ld\t  Customer contacted via telephone\r\n", seq++, COMPLAINTS.recordID);
        _lwrite(m_hfOutputFile, outputString, strlen(outputString));
      }
      if(COMPLAINTS.supFlags & COMPLAINTS_SUPFLAG_LETTER)
      {
        bDidOne = TRUE;
        sprintf(outputString, "%d\t%ld\t  Customer contacted by letter\r\n", seq++, COMPLAINTS.recordID);
        _lwrite(m_hfOutputFile, outputString, strlen(outputString));
      }
      if(COMPLAINTS.supFlags & COMPLAINTS_SUPFLAG_EMAIL)
      {
        bDidOne = TRUE;
        sprintf(outputString, "%d\t%ld\t  Customer contacted via email\r\n", seq++, COMPLAINTS.recordID);
        _lwrite(m_hfOutputFile, outputString, strlen(outputString));
      }
      if(!bDidOne)
      {
        sprintf(outputString, "%d\t%ld\t  Customer was not contacted\r\n", seq++, COMPLAINTS.recordID);
        _lwrite(m_hfOutputFile, outputString, strlen(outputString));
      }
//
//  Memo of conversation
//
      char szMemo[COMPLAINTS_CONVERSATION_LENGTH + 1];

      strncpy(szMemo, COMPLAINTS.conversation, COMPLAINTS_CONVERSATION_LENGTH);
      trim(szMemo, COMPLAINTS_CONVERSATION_LENGTH);
      if(strcmp(szMemo, "") != 0)
      {
        CString strText;
        int len;
        int numLines = pEditLOG->GetLineCount();

        pEditLOG->SetWindowText(szMemo);
        sprintf(outputString, "%d\t%ld\t  Memo of conversation:\r\n", seq++, COMPLAINTS.recordID);
        _lwrite(m_hfOutputFile, outputString, strlen(outputString));
        for(nJ = 0; nJ < numLines; nJ++)
        {
          len = pEditLOG->LineLength(pEditLOG->LineIndex(nJ));
          pEditLOG->GetLine(nJ, strText.GetBuffer(len), len);
          strncpy(tempString, strText, len);
          tempString[len] = '\0';
          strText.ReleaseBuffer(len);
          sprintf(outputString, "%d\t%ld\t  %s\r\n", seq++, COMPLAINTS.recordID, tempString);
          _lwrite(m_hfOutputFile, outputString, strlen(outputString));
        }
      }
//
//  Action taken
//
      if(COMPLAINTS.actionIndex != NO_RECORD)
      {
        CCCSupervisor::PopulateComboBox(m_ActionCodes, m_numActionCodes, pComboBoxCB);

        int numEntries = pComboBoxCB->GetCount();

        for(nJ = 0; nJ < numEntries; nJ++)
        {
          if(COMPLAINTS.actionIndex == (int)pComboBoxCB->GetItemData(nJ))
          {
            pComboBoxCB->GetLBText(nJ, tempString);
            sprintf(outputString, "%d\t%ld\t  Action code: %s\r\n", seq++, COMPLAINTS.recordID, tempString);
            _lwrite(m_hfOutputFile, outputString, strlen(outputString));
            break;
          }
        }
      }
//
//  Default radio buttons
//
      if(COMPLAINTS.supFlags & COMPLAINTS_SUPFLAG_NOVIOLATION)
      {
        sprintf(outputString, "%d\t%ld\t  Action taken: No violation\r\n", seq++, COMPLAINTS.recordID, tempString);
        _lwrite(m_hfOutputFile, outputString, strlen(outputString));
      }
      else if(COMPLAINTS.supFlags & COMPLAINTS_SUPFLAG_INFORMATIONONLY)
      {
        sprintf(outputString, "%d\t%ld\t  Action taken: Information only\r\n", seq++, COMPLAINTS.recordID, tempString);
        _lwrite(m_hfOutputFile, outputString, strlen(outputString));
      }
      else if(COMPLAINTS.supFlags & COMPLAINTS_SUPFLAG_INSUFFICIENT)
      {
        sprintf(outputString, "%d\t%ld\t  Action taken: Insufficient information\r\n", seq++, COMPLAINTS.recordID, tempString);
        _lwrite(m_hfOutputFile, outputString, strlen(outputString));
      }
      else if(COMPLAINTS.supFlags & COMPLAINTS_SUPFLAG_COMMENDATION)
      {
        sprintf(outputString, "%d\t%ld\t  Action taken: Commendation\r\n", seq++, COMPLAINTS.recordID, tempString);
        _lwrite(m_hfOutputFile, outputString, strlen(outputString));
      }
      else if(COMPLAINTS.supFlags & COMPLAINTS_SUPFLAG_OTHER)
      {
        sprintf(outputString, "%d\t%ld\t  Action taken: Other:\r\n", seq++, COMPLAINTS.recordID, tempString);
        _lwrite(m_hfOutputFile, outputString, strlen(outputString));
        
        char szOther[COMPLAINTS_OTHER_LENGTH + 1];

        strncpy(szOther, COMPLAINTS.other, COMPLAINTS_OTHER_LENGTH);
        trim(szOther, COMPLAINTS_OTHER_LENGTH);
        if(strcmp(szOther, "") != 0)
        {
          CString strText;
          int len;

          pEditLOG->SetWindowText(szOther);
          int numLines = pEditLOG->GetLineCount();

          sprintf(outputString, "%d\t%ld\t  Memo of conversation:\r\n", seq++, COMPLAINTS.recordID);
          _lwrite(m_hfOutputFile, outputString, strlen(outputString));
          for(nJ = 0; nJ < numLines; nJ++)
          {
            len = pEditLOG->LineLength(pEditLOG->LineIndex(nJ));
            pEditLOG->GetLine(nJ, strText.GetBuffer(len), len);
            strncpy(tempString, strText, len);
            tempString[len] = '\0';
            strText.ReleaseBuffer(len);
            sprintf(outputString, "%d\t%ld\t  %s\r\n", seq++, COMPLAINTS.recordID, tempString);
            _lwrite(m_hfOutputFile, outputString, strlen(outputString));
          }
        }
      }


//
//  Complaint shared with...
//
  
      if(COMPLAINTS.supFlags & COMPLAINTS_SUPFLAG_OPERATIONS)
      {
        sprintf(outputString, "%d\t%ld\t  Comment shared with Operations\r\n", seq++, COMPLAINTS.recordID, tempString);
        _lwrite(m_hfOutputFile, outputString, strlen(outputString));
      }
      if(COMPLAINTS.supFlags & COMPLAINTS_SUPFLAG_DEVELOPMENT)
      {
        sprintf(outputString, "%d\t%ld\t  Comment shared with Development\r\n", seq++, COMPLAINTS.recordID, tempString);
        _lwrite(m_hfOutputFile, outputString, strlen(outputString));
      }
      if(COMPLAINTS.supFlags & COMPLAINTS_SUPFLAG_MAINTENANCE)
      {
        sprintf(outputString, "%d\t%ld\t  Comment shared with Maintenance\r\n", seq++, COMPLAINTS.recordID, tempString);
        _lwrite(m_hfOutputFile, outputString, strlen(outputString));
      }
      if(COMPLAINTS.supFlags & COMPLAINTS_SUPFLAG_FINANCE)
      {
        sprintf(outputString, "%d\t%ld\t  Comment shared with Finance\r\n", seq++, COMPLAINTS.recordID, tempString);
        _lwrite(m_hfOutputFile, outputString, strlen(outputString));
      }
      if(COMPLAINTS.supFlags & COMPLAINTS_SUPFLAG_SPECIALSERVICES)
      {
        sprintf(outputString, "%d\t%ld\t  Comment shared with Special Services\r\n", seq++, COMPLAINTS.recordID, tempString);
        _lwrite(m_hfOutputFile, outputString, strlen(outputString));
      }
      if(COMPLAINTS.supFlags & COMPLAINTS_SUPFLAG_COMMRELATIONS)
      {
        sprintf(outputString, "%d\t%ld\t  Comment shared with Community Relations\r\n", seq++, COMPLAINTS.recordID, tempString);
        _lwrite(m_hfOutputFile, outputString, strlen(outputString));
      }
//
//  Retain until
//
      if(COMPLAINTS.retainUntil > 0)
      {
        long yyyy, mm, dd;

        GetYMD(COMPLAINTS.retainUntil, &yyyy, &mm, &dd);
        sprintf(outputString, "%d\t%ld\t  Retain until (D/M/Y): %02ld/%02ld/%4ld\r\n", seq++, COMPLAINTS.recordID, dd, mm, yyyy);
        _lwrite(m_hfOutputFile, outputString, strlen(outputString));
      }
//
//  Date closed
//
      if(COMPLAINTS.dateClosed > 0)
      {
        long yyyy, mm, dd;

        GetYMD(COMPLAINTS.dateClosed, &yyyy, &mm, &dd);
        sprintf(outputString, "%d\t%ld\t  Date closed (D/M/Y): %02ld/%02ld/%4ld\r\n", seq++, COMPLAINTS.recordID, dd, mm, yyyy);
        _lwrite(m_hfOutputFile, outputString, strlen(outputString));
      }
    }
  }
//
//  Free the allocated memory
//
  TMSHeapFree(m_pCCMRPT02);
  m_pCCMRPT02 = NULL;

}

void CCCReports::OnDetail() 
{
  if(AllocateSpace())
  {
    m_flags = CCREPORTS_DETAIL;
    OnOK();
  }
}

void CCCReports::OnSummary() 
{
  if(AllocateSpace())
  {
    m_flags = CCREPORTS_SUMMARY;
    OnOK();
  }
}

BOOL CCCReports::AllocateSpace()
{
  char dummy[256];
  int  rcode2;

//
//  Allocate space for the structure
//
  rcode2 = btrieve(B_STAT, TMS_COMPLAINTS, &BSTAT, dummy, 0);
  if(rcode2 != 0 || BSTAT.numRecords == 0)
  {
    TMSError((HWND)NULL, MB_ICONSTOP, ERROR_354, (HANDLE)NULL);
    return(FALSE);
  }

  m_pCCMRPT02 = (CCMRPT02Def *)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(CCMRPT02Def) * BSTAT.numRecords); 
  if(m_pCCMRPT02 == NULL)
  {
    AllocationError(__FILE__, __LINE__, FALSE);
    return(FALSE);
  }

  m_numCCMRPT02 = 0;

  return(TRUE);
}

void CCCReports::OnListing() 
{
  m_flags = 0;
  OnOK();
}

void CCCReports::OnClose() 
{
  OnCancel();
}

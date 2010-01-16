// DailyOpsFU.cpp : implementation file
//
#include "stdafx.h"

extern "C"{
#include "TMSHeader.h"
#include "cistms.h"
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

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDailyOpsFU dialog


CDailyOpsFU::CDailyOpsFU(CWnd* pParent, DAILYOPSFUPassedDataDef* pPassedData)
	: CDialog(CDailyOpsFU::IDD, pParent)
{
  m_pPassedData = pPassedData;

	//{{AFX_DATA_INIT(CDailyOpsFU)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CDailyOpsFU::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDailyOpsFU)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDailyOpsFU, CDialog)
	//{{AFX_MSG_MAP(CDailyOpsFU)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDSETDATE, OnSetdate)
	ON_BN_CLICKED(IDAUDITTRAIL, OnAudittrail)
	ON_BN_CLICKED(IDHOLIDAYS, OnHolidays)
	ON_BN_CLICKED(IDFINISHED, OnFinished)
	ON_BN_CLICKED(IDCONSIDER, OnConsider)
	ON_BN_CLICKED(IDROSTERWEEK, OnRosterweek)
	ON_BN_CLICKED(IDRIDERSHIP, OnRidership)
	ON_BN_CLICKED(IDFARES, OnFares)
	ON_BN_CLICKED(IDCOLORS, OnColors)
	ON_BN_CLICKED(IDCHECKINLOCATIONS, OnCheckinlocations)
	ON_BN_CLICKED(IDEXTRABOARD, OnExtraboard)
	ON_BN_CLICKED(IDPAYROLLLAYOUT, OnPayrolllayout)
	ON_BN_CLICKED(IDGENERATEAVLDATA, OnGenerateavldata)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDailyOpsFU message handlers

BOOL CDailyOpsFU::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
  m_pPassedData->flags = 0;
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CDailyOpsFU::OnClose() 
{
	CDialog::OnClose();
}

void CDailyOpsFU::OnSetdate() 
{
  long date = NO_RECORD;;

  CPickDate dlg(this, &date);

  if(dlg.DoModal() == IDOK)
  {
    m_pPassedData->flags = DAILYOPSFU_FLAG_CHANGEDATE;
    m_pPassedData->date = date;
  }

	CDialog::OnOK();
}

void CDailyOpsFU::OnAudittrail() 
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
  int  thisReport;

//
//  Locate the (possible now sorted) report
//
  for(thisReport = NO_RECORD, nI = 0; nI < TMSRPT_MAX_REPORTSDEFINED; nI++)
  {
    if(TMSRPT[nI].originalReportNumber == 42)
    {
      thisReport = nI;
      break;
    }
  }
  if(thisReport == NO_RECORD)
  {
    return;
  }
  memset(&PassedData, 0x00, sizeof(PassedData));
  if(!TMSRPT43(&PassedData))
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

	CDialog::OnOK();
}

void CDailyOpsFU::OnHolidays() 
{
  long date = m_pPassedData->date;

  CDailyOpsHO dlg(this, &date);

  dlg.DoModal();

	CDialog::OnOK();
}

void CDailyOpsFU::OnFinished() 
{
	CDialog::OnOK();
}

void CDailyOpsFU::OnConsider() 
{
  CDriversToConsider dlg;

  if(dlg.DoModal() == IDOK)
  {
    m_pPassedData->flags = DAILYOPSFU_FLAG_CHANGECONSIDER;
  }

	CDialog::OnOK();
}

void CDailyOpsFU::OnRosterweek() 
{
  CRosterWeek dlg;

  if(dlg.DoModal() == IDOK)
  {
    m_pPassedData->flags = DAILYOPSFU_FLAG_CHANGEWEEK;
  }

	CDialog::OnOK();
}

void CDailyOpsFU::OnRidership() 
{
  CRidership dlg;

  dlg.DoModal();
}

void CDailyOpsFU::OnFares() 
{
  CFares dlg;

  dlg.DoModal();
}

void CDailyOpsFU::OnColors() 
{
  CPegboardColors dlg;

  if(dlg.DoModal() == IDOK)
  {
    m_pPassedData->flags = DAILYOPSFU_FLAG_CHANGECOLOR;
  }
}

void CDailyOpsFU::OnCheckinlocations() 
{
  CDailyOpsRL dlg;

  dlg.DoModal();
}

void CDailyOpsFU::OnExtraboard() 
{
  CDailyOpsXB dlg;

  dlg.DoModal();
}

void CDailyOpsFU::OnPayrolllayout() 
{
  CPayrollLayout dlg;

  dlg.DoModal();
}

//
//  Dump data for the Continental System
//

#define OUTPUT_FOLDER_NAME "AVL Interface Files"

void CDailyOpsFU::OnGenerateavldata() 
{
  HANDLE hOutputFile;
  DWORD dwBytesWritten;
  char  outputString[1024];
  char  szDivisionName[DIVISIONS_NAME_LENGTH + 1];
  char  szFileName[32];
  long  date = m_pPassedData->date - 20000000;  // Change to yymmdd
  long  blockNumber;
  BOOL  bFound;
  BOOL  bCrewOnly;
  int   rcode2;
  int   rosterOrder[] = {6, 0, 1, 2, 3, 4, 5};
  int   nI, nJ;
//
//  Get the division
//
  DIVISIONSKey0.recordID = m_DivisionRecordID;
  btrieve(B_GETEQUAL, TMS_DIVISIONS, &DIVISIONS, &DIVISIONSKey0, 0);
  strncpy(szDivisionName, DIVISIONS.name, DIVISIONS_NAME_LENGTH);
  trim(szDivisionName, DIVISIONS_NAME_LENGTH);
//
//  Set the output file path
//
  _mkdir(OUTPUT_FOLDER_NAME);  // Fall-through if it already exists
  chdir(OUTPUT_FOLDER_NAME);
//
//  Employee Data
//
//  File is EMFyymmdd.TXT
//
  strcpy(szFileName, "EMF");
  sprintf(tempString, "%06ld.TXT", date);
  strcat(szFileName, tempString);

  hOutputFile = CreateFile(szFileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

  if(hOutputFile == INVALID_HANDLE_VALUE)
  {
    MessageBeep(MB_ICONSTOP);
    sprintf(tempString, "Failed to create %s", szFileName);
    MessageBox(tempString, TMS, MB_ICONSTOP | MB_OK);
    goto done;
  }
//
//  Cycle through the drivers
//
  rcode2 = btrieve(B_GETFIRST, TMS_DRIVERS, &DRIVERS, &DRIVERSKey1, 1);
  while(rcode2 == 0)
  {
//
//  Logon ID (Badge number)
//
    strncpy(outputString, DRIVERS.badgeNumber, DRIVERS_BADGENUMBER_LENGTH);
    trim(outputString, DRIVERS_BADGENUMBER_LENGTH);
    strcat(outputString, ",");
//
//  Last name
//
    strncpy(tempString, DRIVERS.lastName, DRIVERS_LASTNAME_LENGTH);
    trim(tempString, DRIVERS_LASTNAME_LENGTH);
    sprintf(szarString, "\"%s\",", tempString);
    strcat(outputString, szarString);
//
//  First name
//
    strncpy(tempString, DRIVERS.firstName, DRIVERS_FIRSTNAME_LENGTH);
    trim(tempString, DRIVERS_FIRSTNAME_LENGTH);
    sprintf(szarString, "\"%s\",", tempString);
    strcat(outputString, szarString);
//
//  Middle name
//
    strncpy(tempString, DRIVERS.initials, DRIVERS_INITIALS_LENGTH);
    trim(tempString, DRIVERS_INITIALS_LENGTH);
    sprintf(szarString, "\"%s\",", tempString);
    strcat(outputString, szarString);
//
//  Division (not used)
//
    strcat(outputString, ",");
//
//  Department (not used)
//
    strcat(outputString, ",");
//
//  Activation date (not used)
//
    strcat(outputString, ",");
//
//  Deactivation date (not used)
//
    strcat(outputString, "\r\n");
//
//  Write it out and get the next record
//
    WriteFile(hOutputFile, (LPCVOID *)outputString, strlen(outputString), &dwBytesWritten, NULL);
    rcode2 = btrieve(B_GETNEXT, TMS_DRIVERS, &DRIVERS, &DRIVERSKey1, 1);
  }

  CloseHandle(hOutputFile);
//
//  Driver and Roster Signup Data
//
//  File is WEEKRUNSyymmdd.TXT
//
  strcpy(szFileName, "WEEKRUNS");
  sprintf(tempString, "%06ld.TXT", date);
  strcat(szFileName, tempString);

  hOutputFile = CreateFile(szFileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

  if(hOutputFile == INVALID_HANDLE_VALUE)
  {
    MessageBeep(MB_ICONSTOP);
    sprintf(tempString, "Failed to create %s", szFileName);
    MessageBox(tempString, TMS, MB_ICONSTOP | MB_OK);
    goto done;
  }
//
//  Cycle through the roster
//
  ROSTERKey1.DIVISIONSrecordID = m_DivisionRecordID;
  ROSTERKey1.rosterNumber = NO_RECORD;
  rcode2 = btrieve(B_GETGREATER, TMS_ROSTER, &ROSTER, &ROSTERKey1, 1);
  while(rcode2 == 0 &&
        ROSTER.DIVISIONSrecordID == m_DivisionRecordID)
  {
//
//  Date (YYYYMMDD)
//
    sprintf(outputString, "%ld,", m_pPassedData->date);
//
//  Roster number
//
    sprintf(tempString, "%ld,", ROSTER.rosterNumber);
    strcat(outputString, tempString);
//
//  Run names and service (service not used)
//
    for(nI = 0; nI < ROSTER_MAX_DAYS; nI++)
    {
      nJ = rosterOrder[nI];
      RUNSKey0.recordID = ROSTER.WEEK[m_RosterWeek].RUNSrecordIDs[nJ];
      rcode2 = btrieve(B_GETEQUAL, TMS_RUNS, &RUNS, &RUNSKey0, 0);
      if(rcode2 != 0)
      {
        strcat(outputString, "0,,");
      }
      else
      {
        sprintf(tempString, "%ld,,", RUNS.runNumber);
        strcat(outputString, tempString);
      }
    }
//
//  Logon ID (Badge number)
//
    DRIVERSKey0.recordID = ROSTER.DRIVERSrecordID;
    rcode2 = btrieve(B_GETEQUAL, TMS_DRIVERS, &DRIVERS, &DRIVERSKey0, 0);
    if(rcode2 != 0)
    {
      strcat(outputString, "0\r\n");
    }
    else
    {
      strncpy(tempString, DRIVERS.badgeNumber, DRIVERS_BADGENUMBER_LENGTH);
      trim(tempString, DRIVERS_BADGENUMBER_LENGTH);
      strcat(outputString, tempString);
      strcat(outputString, "\r\n");
    }
//
//  Write it out and get the next record
//
    WriteFile(hOutputFile, (LPCVOID *)outputString, strlen(outputString), &dwBytesWritten, NULL);
    rcode2 = btrieve(B_GETNEXT, TMS_ROSTER, &ROSTER, &ROSTERKey1, 1);
  }

  CloseHandle(hOutputFile);
//
//  Vehicle Signup Data
//
//  File is VEHICLES_yymmdd.TXT
//
  strcpy(szFileName, "VEHICLES_");
  sprintf(tempString, "%06ld.TXT", date);
  strcat(szFileName, tempString);

  hOutputFile = CreateFile(szFileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

  if(hOutputFile == INVALID_HANDLE_VALUE)
  {
    MessageBeep(MB_ICONSTOP);
    sprintf(tempString, "Failed to create %s", szFileName);
    MessageBox(tempString, TMS, MB_ICONSTOP | MB_OK);
    goto done;
  }
//
//  Go through BlockInfo
//
  for(nI = 0; nI < m_pPassedData->numInBlockInfo; nI++)
  {
//
//  Date
//
    sprintf(outputString, "%ld,", m_pPassedData->date);
//
//  Service type (not used)
//
    strcat(outputString, ",");
//
//  Block abbreviation (block number)
//
    sprintf(tempString, "\"%ld\",", m_pPassedData->BlockInfo[nI].blockNumber);
    strcat(outputString, tempString);
//
//  Vehicle (Bus number)
//
    BUSESKey0.recordID = m_pPassedData->BlockInfo[nI].BUSESrecordID;
    rcode2 = btrieve(B_GETEQUAL, TMS_BUSES, &BUSES, &BUSESKey0, 0);
    if(rcode2 != 0)
    {
      strcat(outputString, "0\r\n");
    }
    else
    {
      strncpy(tempString, BUSES.number, BUSES_NUMBER_LENGTH);
      trim(tempString, BUSES_NUMBER_LENGTH);
      sprintf(szarString, "\"%s\"\r\n", tempString);
      strcat(outputString, szarString);
    }
//
//  Write it out
//
    WriteFile(hOutputFile, (LPCVOID *)outputString, strlen(outputString), &dwBytesWritten, NULL);
  }

  CloseHandle(hOutputFile);
//
//  Daily Adjustments
//
//  File is RUNS_yymmdd.TXT
//
  strcpy(szFileName, "RUNS_");
  sprintf(tempString, "%06ld.TXT", date);
  strcat(szFileName, tempString);

  hOutputFile = CreateFile(szFileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

  if(hOutputFile == INVALID_HANDLE_VALUE)
  {
    MessageBeep(MB_ICONSTOP);
    sprintf(tempString, "Failed to create %s", szFileName);
    MessageBox(tempString, TMS, MB_ICONSTOP | MB_OK);
    goto done;
  }
//
//  Go through OpenWork
//
  for(nI = 0; nI < m_pPassedData->numInOpenWork; nI++)
  {
//
//  Date
//
    sprintf(outputString, "%ld,", m_pPassedData->date);
//
//  Run name
//
    sprintf(tempString, "\"%ld\",", m_pPassedData->OpenWork[nI].runNumber);
    strcat(outputString, tempString);
//
//  Driver number (badge number)
//
    DRIVERSKey0.recordID = m_pPassedData->OpenWork[nI].DRIVERSrecordID;
    rcode2 = btrieve(B_GETEQUAL, TMS_DRIVERS, &DRIVERS, &DRIVERSKey0, 0);
    if(rcode2 != 0)
    {
      strcat(outputString, "0,");
    }
    else
    {
      strncpy(tempString, DRIVERS.badgeNumber, DRIVERS_BADGENUMBER_LENGTH);
      trim(tempString, DRIVERS_BADGENUMBER_LENGTH);
      strcat(outputString, tempString);
      strcat(outputString, ",");
    }
//
//  Run type (RL - Regular run, RS - Reserve (extraboard) run)
//
    bCrewOnly = m_pPassedData->OpenWork[nI].flags & RDATA_FLAG_CREWONLY;
    strcat(outputString, (bCrewOnly ? "RS" : "RL"));
    strcat(outputString, ",");
//
//  Block abbreviation (block number)
//
    if(bCrewOnly)
    {
      blockNumber = 0;
    }
    else
    {
      RUNSKey0.recordID = m_pPassedData->OpenWork[nI].RUNSrecordID;
      rcode2 = btrieve(B_GETEQUAL, TMS_RUNS, &RUNS, &RUNSKey0, 0);
      if(rcode2 != 0)
      {
        blockNumber = 0;
      }
      else
      {
        TRIPSKey0.recordID = RUNS.start.TRIPSrecordID;
        rcode2 = btrieve(B_GETEQUAL, TMS_TRIPS, &TRIPS, &TRIPSKey0, 0);
        if(rcode2 != 0)
        {
          blockNumber = 0;
        }
        else
        {
          blockNumber = TRIPS.standard.blockNumber;
        }
      }
    }
    sprintf(tempString, "\"%ld\",", blockNumber);
    strcat(outputString, tempString);

//
//  Time on (MMMM - minutes past midnight)
//
    sprintf(tempString, "%ld,", m_pPassedData->OpenWork[nI].onTime / 60);
    strcat(outputString, tempString);
//
//  Time off (MMMM - minutes past midnight)
//
    sprintf(tempString, "%ld,", m_pPassedData->OpenWork[nI].offTime / 60);
    strcat(outputString, tempString);
//
//  Vehicle
//
    if(blockNumber == 0)
    {
      strcat(outputString, ",");
    }
    else
    {
      for(bFound = FALSE, nJ = 0; nJ < m_pPassedData->numInBlockInfo; nJ++)
      {
        if(m_pPassedData->BlockInfo[nJ].blockNumber == blockNumber)
        {
          bFound = TRUE;
          break;
        }
      }
      if(!bFound)
      {
        strcat(outputString, ",");
      }
      else
      {
        BUSESKey0.recordID = m_pPassedData->BlockInfo[nI].BUSESrecordID;
        rcode2 = btrieve(B_GETEQUAL, TMS_BUSES, &BUSES, &BUSESKey0, 0);
        if(rcode2 != 0)
        {
          strcat(outputString, ",");
        }
        else
        {
          strncpy(tempString, BUSES.number, BUSES_NUMBER_LENGTH);
          trim(tempString, BUSES_NUMBER_LENGTH);
          sprintf(szarString, "\"%s\",", tempString);
          strcat(outputString, szarString);
        }
      }
    }
//
//  Service type (not used)
//
    strcat(outputString, "\r\n");
//
//  Write it out
//
    WriteFile(hOutputFile, (LPCVOID *)outputString, strlen(outputString), &dwBytesWritten, NULL);
  }

  CloseHandle(hOutputFile);
//
//  Vehicle Data
//
//  File is VEHICLEDATA_yymmdd.TXT
//
  strcpy(szFileName, "VEHICLEDATA_");
  sprintf(tempString, "%06ld.TXT", date);
  strcat(szFileName, tempString);

  hOutputFile = CreateFile(szFileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

  if(hOutputFile == INVALID_HANDLE_VALUE)
  {
    MessageBeep(MB_ICONSTOP);
    sprintf(tempString, "Failed to create %s", szFileName);
    MessageBox(tempString, TMS, MB_ICONSTOP | MB_OK);
    goto done;
  }
//
//  Cycle through the buses
//
  rcode2 = btrieve(B_GETFIRST, TMS_BUSES, &BUSES, &BUSESKey1, 1);
  while(rcode2 == 0)
  {
//
//  RNET Address
//
    strcpy(outputString, ",");
//
//  Vehicle name
//
    strncpy(tempString, BUSES.number, BUSES_NUMBER_LENGTH);
    trim(tempString, BUSES_NUMBER_LENGTH);
    sprintf(szarString, "\"%s\",", tempString);
    strcat(outputString, szarString);
//
//  Fleet
//
    BUSTYPESKey0.recordID = BUSES.BUSTYPESrecordID;
    rcode2 = btrieve(B_GETEQUAL, TMS_BUSTYPES, &BUSTYPES, &BUSTYPESKey0, 0);
    if(rcode2 != 0)
    {
      strcat(outputString, ",");
    }
    else
    {
      strncpy(tempString, BUSTYPES.name, BUSTYPES_NAME_LENGTH);
      trim(tempString, BUSTYPES_NAME_LENGTH);
      sprintf(szarString, "\"%s\",", tempString);
      strcat(outputString, szarString);
    }
//
//  Garage
//
    sprintf(szarString, "\"%s\",", szDivisionName);
    strcat(outputString, szarString);
//
//  VIN
//
    strcat(outputString, ",");
//
//  License
//
    strcat(outputString, ",");
//
//  Make
//
    strcat(outputString, ",");
//
//  Model
//
    strcat(outputString, ",");
//
//  Series
//
    strcat(outputString, ",");
//
//  Year
//
    strcat(outputString, "\r\n");
//
//  Write it out and get the next record
//
    WriteFile(hOutputFile, (LPCVOID *)outputString, strlen(outputString), &dwBytesWritten, NULL);
    rcode2 = btrieve(B_GETNEXT, TMS_BUSES, &BUSES, &BUSESKey1, 1);
  }

  CloseHandle(hOutputFile);
//
//  All done
//
  MessageBeep(MB_ICONINFORMATION);
  MessageBox("AVL data gen complete", TMS, MB_ICONINFORMATION | MB_OK);

  done:
    chdir("..");  // Back to where we started from

	CDialog::OnOK();
}

// 
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2006 Schedule Masters, Inc.
//  All rights reserved.
//
//  TMSRPT79() - Kalamazoo Payroll Download to Excel
//

#include "stdafx.h"

extern "C" {
#include "TMSHeader.h"
#include "cistms.h"
#include "limits.h"
int SetRecordBodyForString(short int *, short int *, char *, BYTE *);
int SetRecordBodyForDouble(short int *, short int *, double *, BYTE *);
int SetRecordBodyForShortInteger(short int *, short int *, short int *, BYTE *);
}  // extern "C"

int  TMSRPT79Ex(TMSRPTPassedDataDef *);

extern "C" {
BOOL FAR TMSRPT79(TMSRPTPassedDataDef *pPassedData)
{
  return TMSRPT79Ex(pPassedData);
}
} // extern "C"

#include "TMS.h"
#include "DailyOpsHeader.h"

typedef struct TMSRPT79DriversStruct
{
  long DRIVERSrecordID;
  long hoursWorked;
  long leavePaid;
  long leaveType;
  long unpaid;
  long hoursScheduled;
  long ALAvailable;
  long ALPending;
  long violationCategory;
  long violationType;
  long FMLA;
} TMSRPT79DRIVERSDef;

BOOL FAR TMSRPT79Ex(TMSRPTPassedDataDef *pPassedData)
{
  TMSRPT79DRIVERSDef DriverData[500];
  PROPOSEDRUNDef     PROPOSEDRUN;
  COSTDef COST;
  HANDLE hOutputFile;
  BOOL  bKeepGoing = FALSE;
  BOOL  bFound;
  BOOL  bCrewOnly;
  char  *pszReportName;
  char  *pszOperator;
  char  dummy[256];
  char  szOutputFileName[MAX_PATH];
  BYTE  recordBody[512];
  long  year, month, day;
  long  fromDate;
  long  numDrivers;
  long  maxDrivers;
  long  m_SERVICESrecordIDInEffect;
  long  m_ROSTERDivisionInEffect;
  long  m_RUNSDivisionInEffect;
  long  serviceRecordID;
  long  cutAsRuntype;
  int   nI, nJ;
  int   rcode2;
  int   recordBodyLength;
  int   today;
  short int rowNumber;
  short int colNumber;

  CString s;

  pPassedData->nReportNumber = 78;
  pPassedData->numDataFiles = 1;
//
//  Get the date
//
  fromDate = NO_RECORD;
 
  CPickDate dlg(NULL, &fromDate);

  if(dlg.DoModal() != IDOK)
  {
    return(FALSE);
  }
  GetYMD(fromDate, &year, &month, &day);
//
//  Figure out what day of the week we are
//
  int  indexToRoster[7] = {6, 0, 1, 2, 3, 4, 5};

  CTime workDate(year, month, day, 0, 0, 0);
  
  today = indexToRoster[workDate.GetDayOfWeek() - 1];
  m_SERVICESrecordIDInEffect = ROSTERPARMS.serviceDays[today];
//
//  Make sure a roster template's been established
//
  DAILYOPSKey1.recordTypeFlag = DAILYOPS_FLAG_ROSTER;
  DAILYOPSKey1.pertainsToDate = year * 10000 + month * 100 + day;
  DAILYOPSKey1.pertainsToTime = 86400;  // Midnight
  DAILYOPSKey1.recordFlags = 0;
  rcode2 = btrieve(B_GETLESSTHANOREQUAL, TMS_DAILYOPS, &DAILYOPS, &DAILYOPSKey1, 1);
  if(rcode2 == 0 && (DAILYOPS.recordTypeFlag & DAILYOPS_FLAG_ROSTER))
  {
    m_ROSTERDivisionInEffect = DAILYOPS.DOPS.RosterTemplate.ROSTERDIVISIONSrecordID;
    m_RUNSDivisionInEffect = DAILYOPS.DOPS.RosterTemplate.RUNSDIVISIONSrecordID;
  }
  else
  {
    m_ROSTERDivisionInEffect = m_DivisionRecordID;
    m_RUNSDivisionInEffect = m_DivisionRecordID;
  }
//
//  Display the status bar
//
  for(nI = 0; nI < m_LastReport; nI++)
  {
    if(TMSRPT[nI].originalReportNumber == pPassedData->nReportNumber)
    {
      pszReportName = TMSRPT[nI].szReportName;
      break;
    }
  }
  StatusBarStart(hWndMain, pszReportName);
//
//  Open the output file
//
  sprintf(szOutputFileName, "%ld Payroll Download.xls", fromDate);
  hOutputFile = CreateFile(szOutputFileName, GENERIC_WRITE, 0, NULL,
        CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
  if(hOutputFile == INVALID_HANDLE_VALUE)
  {
    LoadString(hInst, ERROR_202, szFormatString, sizeof(szFormatString));
    sprintf(szarString, szFormatString, szOutputFileName);
    MessageBeep(MB_ICONSTOP);
    MessageBox((HWND)NULL, szarString, TMS, MB_ICONSTOP);
    goto deallocate;
  }
//
//  Set up the trip planner
//
  if(bUseDynamicTravels)
  {
    if(bUseCISPlan)
    {
      CISfree();
      if(!CISbuild(FALSE, TRUE))
      {
        goto deallocate;
      }
    }
    else
    {
      rcode2 = btrieve(B_GETFIRST, TMS_SERVICES, &SERVICES, &SERVICESKey1, 1);
      while(rcode2 == 0)
      {
        serviceRecordID = SERVICES.recordID;
        InitTravelMatrix(serviceRecordID, TRUE);
        SERVICESKey0.recordID = serviceRecordID;
        btrieve(B_GETEQUAL, TMS_SERVICES, &SERVICES, &SERVICESKey0, 0);
        btrieve(B_GETPOSITION, TMS_SERVICES, &SERVICES, &SERVICESKey0, 0);
        btrieve(B_GETDIRECT, TMS_SERVICES, &SERVICES, &SERVICESKey1, 1);
        rcode2 = btrieve(B_GETNEXT, TMS_SERVICES, &SERVICES, &SERVICESKey1, 1);
      }
    }
  }
//
//  Cycle through the drivers - Pass 1 build the framework
//
  StatusBarText("Building Driver List...");
  rcode2 = btrieve(B_STAT, TMS_DRIVERS, &BSTAT, dummy, 0);
  maxDrivers = BSTAT.numRecords;
  rcode2 = btrieve(B_GETFIRST, TMS_DRIVERS, &DRIVERS, &DRIVERSKey1, 1);
  numDrivers = 0;
  while(rcode2 == 0)
  {
    StatusBar(numDrivers, maxDrivers);
    if(ConsideringThisDriverType(DRIVERS.DRIVERTYPESrecordID))
    {
      DriverData[numDrivers].DRIVERSrecordID = DRIVERS.recordID;
      DriverData[numDrivers].hoursWorked = 0;
      DriverData[numDrivers].leavePaid = 0;
      DriverData[numDrivers].leaveType = NO_RECORD;
      DriverData[numDrivers].unpaid = 0;
      DriverData[numDrivers].hoursScheduled = 0;
      DriverData[numDrivers].ALAvailable = DRIVERS.vacationTime;
      DriverData[numDrivers].ALPending = 0;
      DriverData[numDrivers].violationCategory = NO_RECORD;
      DriverData[numDrivers].violationType = NO_RECORD;
      DriverData[numDrivers].FMLA = 0;
      numDrivers++;
    }
    rcode2 = btrieve(B_GETNEXT, TMS_DRIVERS, &DRIVERS, &DRIVERSKey1, 1);
  }
  if(numDrivers == 0)
  {
    s.LoadString(ERROR_367);
    MessageBeep(MB_ICONINFORMATION);
    MessageBox(NULL, s, TMS, MB_OK | MB_ICONINFORMATION);
    goto deallocate;
  }
//
//  Pass 2 - Get the nitty gritty
//
  StatusBarText("Processing...");
  for(nI = 0; nI < numDrivers; nI++)
  {      
    StatusBar(numDrivers, maxDrivers);
    if(StatusBarAbort())
    {
      goto deallocate;
    }
//
//  Scheduled hours
//
    DriverData[nI].hoursScheduled = 0;
    ROSTERKey2.DRIVERSrecordID = DriverData[nI].DRIVERSrecordID;
    rcode2 = btrieve(B_GETEQUAL, TMS_ROSTER, &ROSTER, &ROSTERKey2, 2);
    while(rcode2 == 0 &&
          ROSTER.DRIVERSrecordID == DriverData[nI].DRIVERSrecordID)
    {
      if(ROSTER.DIVISIONSrecordID == m_ROSTERDivisionInEffect &&
            ROSTER.WEEK[0].RUNSrecordIDs[today] != NO_RECORD)
      {
        bCrewOnly = ROSTER.WEEK[0].flags & RDATA_FLAG_CREWONLY;
        cutAsRuntype = CDailyOps::SetupRun(ROSTER.WEEK[0].RUNSrecordIDs[today], bCrewOnly, &PROPOSEDRUN);
        RunCoster(&PROPOSEDRUN, cutAsRuntype, &COST);
        DriverData[nI].hoursScheduled += COST.TOTAL.payTime;
      }
      rcode2 = btrieve(B_GETNEXT, TMS_ROSTER, &ROSTER, &ROSTERKey2, 2);
    }
//
//  DailyOps entries
//

    DAILYOPSKey3.DRIVERSrecordID = DriverData[nI].DRIVERSrecordID;
    DAILYOPSKey3.pertainsToDate = fromDate;
    DAILYOPSKey3.pertainsToTime = NO_TIME;
    DAILYOPSKey3.recordTypeFlag = 0;
    DAILYOPSKey3.recordFlags = 0;
    rcode2 = btrieve(B_GETGREATER, TMS_DAILYOPS, &DAILYOPS, &DAILYOPSKey3, 3);
    while(rcode2 == 0 &&
          DAILYOPS.DRIVERSrecordID == DriverData[nI].DRIVERSrecordID &&
          DAILYOPS.pertainsToDate == fromDate)
    {
      if(!ANegatedRecord(DAILYOPS.recordID, 3) && !ANegatingRecord(&DAILYOPS))
      {
//
//  Is this an absence record?
//
        if(DAILYOPS.recordTypeFlag & DAILYOPS_FLAG_ABSENCE)
        {
//
//  Leave paid
//
          DriverData[nI].leavePaid += DAILYOPS.DOPS.Absence.paidTime[0] + DAILYOPS.DOPS.Absence.paidTime[1];
//
//  Reason
//
          DriverData[nI].leaveType = DAILYOPS.DOPS.Absence.reasonIndex;
//
//  Unpaid time
//
          DriverData[nI].unpaid += DAILYOPS.DOPS.Absence.unpaidTime;
//
//  Discipline
//
          DISCIPLINEKey2.DRIVERSrecordID = DriverData[nI].DRIVERSrecordID;
          DISCIPLINEKey2.violationCategory = NO_RECORD;
          DISCIPLINEKey2.violationType = NO_RECORD;
          rcode2 = btrieve(B_GETGREATER, TMS_DISCIPLINE, &DISCIPLINE, &DISCIPLINEKey2, 2);
          bFound = FALSE;
          while(rcode2 == 0 &&
                DISCIPLINE.DRIVERSrecordID == DriverData[nI].DRIVERSrecordID)
          {
            if(DISCIPLINE.DAILYOPSrecordID == DAILYOPS.recordID)
            {
              bFound = TRUE;
              break;
            }
            rcode2 = btrieve(B_GETNEXT, TMS_DISCIPLINE, &DISCIPLINE, &DISCIPLINEKey2, 2);
          }
          if(bFound)
          {
            if(DISCIPLINE.actionTaken != NO_RECORD)
            {
              DriverData[nI].violationCategory = DISCIPLINE.violationCategory;
              DriverData[nI].violationType = DISCIPLINE.violationType;
            }
          } 
//
//  FMLA
//
          if(DAILYOPS.DOPS.Absence.reasonIndex == 4)  // Kalamazoo rule 4 is FMLA
          {
            DriverData[nI].FMLA +=
                  DAILYOPS.DOPS.Absence.paidTime[0] + DAILYOPS.DOPS.Absence.paidTime[1] + DAILYOPS.DOPS.Absence.unpaidTime;
          }
        }  // DAILYOPS_FLAG_ABSENCE
      }  // Not unregistered
      rcode2 = btrieve(B_GETNEXT, TMS_DAILYOPS, &DAILYOPS, &DAILYOPSKey3, 3);
    }  // while this driver in DAILYOPS
  }  // nI
//
//  Pass 3 - Dump it all out
//
//
//  Write out the header for the Excel file
//
  StatusBarText("Writing Excel file...");
  recordBody[0] = 0x02;
  recordBody[1] = 0x00;
  recordBody[2] = 0x10;
  recordBody[3] = 0x00;
  WriteExcel(EXCEL_BOF, hOutputFile, recordBody, 4);
//
//  Output the date
//
  s = workDate.Format(_T("%A, %B %d, %Y"));
  strcpy(tempString, s);
  rowNumber = 0;
  colNumber = 0;
  recordBodyLength = SetRecordBodyForString(&rowNumber, &colNumber, tempString, recordBody);
  WriteExcel(EXCEL_LABEL, hOutputFile, recordBody, recordBodyLength);
//
//  And the column headers
//
//  Employee
//
  rowNumber = 2;
  colNumber = 0;
  strcpy(tempString, "Employee");
  recordBodyLength = SetRecordBodyForString(&rowNumber, &colNumber, tempString, recordBody);
  WriteExcel(EXCEL_LABEL, hOutputFile, recordBody, recordBodyLength);
//
//  Hours Worked
//
  rowNumber = 2;
  colNumber = 1;
  strcpy(tempString, "Hrs. Worked");
  recordBodyLength = SetRecordBodyForString(&rowNumber, &colNumber, tempString, recordBody);
  WriteExcel(EXCEL_LABEL, hOutputFile, recordBody, recordBodyLength);
//
//  Leave paid
//
  rowNumber = 2;
  colNumber = 2;
  strcpy(tempString, "Leave Paid");
  recordBodyLength = SetRecordBodyForString(&rowNumber, &colNumber, tempString, recordBody);
  WriteExcel(EXCEL_LABEL, hOutputFile, recordBody, recordBodyLength);
//
//  Leave type
//
  rowNumber = 2;
  colNumber = 3;
  strcpy(tempString, "Leave Type");
  recordBodyLength = SetRecordBodyForString(&rowNumber, &colNumber, tempString, recordBody);
  WriteExcel(EXCEL_LABEL, hOutputFile, recordBody, recordBodyLength);
//
//  Unpaid
//
  rowNumber = 2;
  colNumber = 4;
  strcpy(tempString, "Unpaid");
  recordBodyLength = SetRecordBodyForString(&rowNumber, &colNumber, tempString, recordBody);
  WriteExcel(EXCEL_LABEL, hOutputFile, recordBody, recordBodyLength);
//
//  Hours scheduled
//
  rowNumber = 2;
  colNumber = 5;
  strcpy(tempString, "Hrs. Scheduled");
  recordBodyLength = SetRecordBodyForString(&rowNumber, &colNumber, tempString, recordBody);
  WriteExcel(EXCEL_LABEL, hOutputFile, recordBody, recordBodyLength);
//
//  Annual Leave Available
//
  rowNumber = 2;
  colNumber = 6;
  strcpy(tempString, "AL Available");
  recordBodyLength = SetRecordBodyForString(&rowNumber, &colNumber, tempString, recordBody);
  WriteExcel(EXCEL_LABEL, hOutputFile, recordBody, recordBodyLength);
//
//  Annual Leave Pending
//
  rowNumber = 2;
  colNumber = 7;
  strcpy(tempString, "AL Pending");
  recordBodyLength = SetRecordBodyForString(&rowNumber, &colNumber, tempString, recordBody);
  WriteExcel(EXCEL_LABEL, hOutputFile, recordBody, recordBodyLength);
//
//  Discipline
//
  rowNumber = 2;
  colNumber = 8;
  strcpy(tempString, "Discipline");
  recordBodyLength = SetRecordBodyForString(&rowNumber, &colNumber, tempString, recordBody);
  WriteExcel(EXCEL_LABEL, hOutputFile, recordBody, recordBodyLength);
//
//  FMLA
//
  rowNumber = 2;
  colNumber = 9;
  strcpy(tempString, "FMLA");
  recordBodyLength = SetRecordBodyForString(&rowNumber, &colNumber, tempString, recordBody);
  WriteExcel(EXCEL_LABEL, hOutputFile, recordBody, recordBodyLength);
//
//  And the data
//
  for(nI = 0; nI < numDrivers; nI++)
  {
    StatusBar((long)nI, numDrivers);
//
//  Driver
//
    rowNumber = nI + 3;
    colNumber = 0;
    pszOperator = BuildOperatorString(DriverData[nI].DRIVERSrecordID);
    recordBodyLength = SetRecordBodyForString(&rowNumber, &colNumber, pszOperator, recordBody);
    WriteExcel(EXCEL_LABEL, hOutputFile, recordBody, recordBodyLength);
//
//  Hours Worked
//
    rowNumber = nI + 3;
    colNumber = 1;
    DriverData[nI].hoursWorked = DriverData[nI].hoursScheduled - DriverData[nI].leavePaid - DriverData[nI].unpaid;
    strcpy(tempString, chhmm(DriverData[nI].hoursWorked));
    recordBodyLength = SetRecordBodyForString(&rowNumber, &colNumber, tempString, recordBody);
    WriteExcel(EXCEL_LABEL, hOutputFile, recordBody, recordBodyLength);
//
//  Leave paid
//
    rowNumber = nI + 3;
    colNumber = 2;
    strcpy(tempString, chhmm(DriverData[nI].leavePaid));
    recordBodyLength = SetRecordBodyForString(&rowNumber, &colNumber, tempString, recordBody);
    WriteExcel(EXCEL_LABEL, hOutputFile, recordBody, recordBodyLength);
//
//  Leave type
//
    rowNumber = nI + 3;
    colNumber = 3;
    strcpy(tempString, "");
    if(DAILYOPS.DOPS.Absence.reasonIndex != NO_RECORD)
    {
      for(bFound = FALSE, nJ = 0; nJ < m_numAbsenceReasons; nJ++)
      {
        if(DriverData[nI].leaveType == m_AbsenceReasons[nJ].ruleNumber)
        {
          bFound = TRUE;
          break;
        }
      }
      if(bFound)
      {
        strcpy(tempString, m_AbsenceReasons[nJ].szText);
      }
    }
    recordBodyLength = SetRecordBodyForString(&rowNumber, &colNumber, tempString, recordBody);
    WriteExcel(EXCEL_LABEL, hOutputFile, recordBody, recordBodyLength);
//
//  Unpaid
//
    rowNumber = nI + 3;
    colNumber = 4;
    strcpy(tempString, chhmm(DriverData[nI].unpaid));
    recordBodyLength = SetRecordBodyForString(&rowNumber, &colNumber, tempString, recordBody);
    WriteExcel(EXCEL_LABEL, hOutputFile, recordBody, recordBodyLength);
//
//  Hours scheduled
//
    rowNumber = nI + 3;
    colNumber = 5;
    strcpy(tempString, chhmm(DriverData[nI].hoursScheduled));
    recordBodyLength = SetRecordBodyForString(&rowNumber, &colNumber, tempString, recordBody);
    WriteExcel(EXCEL_LABEL, hOutputFile, recordBody, recordBodyLength);
//
//  Annual Leave Available
//
    rowNumber = nI + 3;
    colNumber = 6;
    strcpy(tempString, chhmm(DriverData[nI].ALAvailable));
    recordBodyLength = SetRecordBodyForString(&rowNumber, &colNumber, tempString, recordBody);
    WriteExcel(EXCEL_LABEL, hOutputFile, recordBody, recordBodyLength);
//
//  Annual Leave Pending
//
    rowNumber = nI + 3;
    colNumber = 7;
    strcpy(tempString, chhmm(DriverData[nI].ALAvailable));
    recordBodyLength = SetRecordBodyForString(&rowNumber, &colNumber, tempString, recordBody);
    WriteExcel(EXCEL_LABEL, hOutputFile, recordBody, recordBodyLength);
//
//  Discipline
//
    rowNumber = nI + 3;
    colNumber = 8;
    if(DriverData[nI].violationCategory == NO_RECORD || DriverData[nI].violationType == NO_RECORD)
    {
      strcpy(tempString, "");
    }
    else
    {
      strcpy(tempString, m_Violations[DriverData[nI].violationCategory][DriverData[nI].violationType].text);
    }
    recordBodyLength = SetRecordBodyForString(&rowNumber, &colNumber, tempString, recordBody);
    WriteExcel(EXCEL_LABEL, hOutputFile, recordBody, recordBodyLength);
//
//  FMLA
//
    rowNumber = nI + 3;
    colNumber = 9;
    strcpy(tempString, chhmm(DriverData[nI].FMLA));
    recordBodyLength = SetRecordBodyForString(&rowNumber, &colNumber, tempString, recordBody);
    WriteExcel(EXCEL_LABEL, hOutputFile, recordBody, recordBodyLength);
  }
//
//  Write out the EOF marker for the Excel file
//
  WriteExcel(EXCEL_EOF, hOutputFile, NULL, NO_RECORD);
//
//  All done
//
  sprintf(tempString, "%s\n\nwas written to folder\n\n%s", szOutputFileName, szDatabaseFileName);
  MessageBeep(MB_ICONINFORMATION);
  MessageBox(NULL, tempString, TMS, MB_OK | MB_ICONINFORMATION);
  bKeepGoing = TRUE;

  deallocate:
    CloseHandle(hOutputFile);
    StatusBarEnd();

  return(bKeepGoing);
}

//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
#include "TMSHeader.h"

//
//  TMSRPT54() - Download to Kalamazoo Payroll System
//
//  Date (mm/dd/yyyy)
//  Employee number
//  Payroll Code
//  Hours expressed in seconds
//

#define TMSRPT54_MAX_EMPLOYEENUMBERLENGTH 5
#define TMSRPT54_CRITICALILLNESS  0
#define TMSRPT54_HOLIDAYPAY       1
#define TMSRPT54_MILITARYLEAVE    2
#define TMSRPT54_OVERTIME         3 
#define TMSRPT54_REGULARPAY       4
#define TMSRPT54_ANNUALLEAVE      5
#define TMSRPT54_NUMCOMPONENTS    6

BOOL FAR TMSRPT54(TMSRPTPassedDataDef *pPassedData)
{
  REPORTPARMSDef REPORTPARMS;
  PROPOSEDRUNDef PROPOSEDRUN;
  COSTDef COST;
  HFILE hfOutputFile;
  BOOL  bKeepGoing = FALSE;
  BOOL  bRC;
  char  outputString[512];
  char  szDate[16];
  char  szEmployee[16];
  char  szRunNumber[7];
  char  *ptr;
  long  numPieces;
  long  time;
  long  platformTime;
  long  reportTime;
  long  turninTime;
  long  travelTime;
  long  dd,  mm, yyyy;
  int   nI;
  int   nJ;
  int   nK;
  int   rcode2;
  int   seq;
  int   numRosters;

  pPassedData->nReportNumber = 48;
  pPassedData->numDataFiles = 1;
//
//  See what he wants
//
  REPORTPARMS.nReportNumber = pPassedData->nReportNumber;
  REPORTPARMS.DIVISIONSrecordID = m_DivisionRecordID;
  REPORTPARMS.flags = RPFLAG_DIVISIONS;
  bRC = DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_RPTPARMS),
        hWndMain, (DLGPROC)RPTPARMSMsgProc, (LPARAM)&REPORTPARMS);
  if(!bRC)
  {
    return(FALSE);
  }
//
//  Check the list of routes, services, and divisions
//
  if(REPORTPARMS.numDivisions == 0)
  {
    goto deallocate;
  }
//
//  Open the output file
//
  strcpy(tempString, szDatabaseFileName);
  if((ptr = strrchr(tempString, '\\')) != NULL)
  {
    *ptr = '\0';
  }
  strcat(tempString, "\\Kalamazoo Payroll.txt");
  hfOutputFile = _lcreat(tempString, 0);
  if(hfOutputFile == HFILE_ERROR)
  {
    LoadString(hInst, ERROR_202, szFormatString, sizeof(szFormatString));
    sprintf(szarString, szFormatString, tempString);
    MessageBeep(MB_ICONSTOP);
    MessageBox((HWND)NULL, szarString, TMS, MB_ICONSTOP);
    goto deallocate;
  }
//
//  Fire up the status bar
//
  LoadString(hInst, TEXT_117, tempString, TEMPSTRING_LENGTH);
  StatusBarStart(hWndMain, tempString);
//
//  Determine the extent of the status bar
//
  rcode2 = btrieve(B_STAT, TMS_ROSTER, &BSTAT, outputString, 0);
  if(rcode2 != 0 || BSTAT.numRecords == 0)
  {
    TMSError((HWND)NULL, MB_ICONSTOP, ERROR_319, (HANDLE)NULL);
    goto deallocate;
  }
  numRosters = BSTAT.numRecords;
//
//  Get today's date
//
  if(CalendarDate(&dd, &mm, &yyyy) == NO_RECORD)
  {
    goto deallocate;
  }
//
//  Loop through all the divisions
//
  bKeepGoing = TRUE;
  seq = 0;
  for(nI = 0; nI < REPORTPARMS.numDivisions; nI++)
  {
    if(StatusBarAbort())
    {
      goto deallocate;
    }
    DIVISIONSKey0.recordID = REPORTPARMS.pDivisionList[nI];
    btrieve(B_GETEQUAL, TMS_DIVISIONS, &DIVISIONS, &DIVISIONSKey0, 0);
    strncpy(tempString, DIVISIONS.name, DIVISIONS_NAME_LENGTH);
    trim(tempString, DIVISIONS_NAME_LENGTH);
//
//  Cycle through the roster
//
    ROSTERKey1.DIVISIONSrecordID = DIVISIONS.recordID;
    ROSTERKey1.rosterNumber = NO_RECORD;
    rcode2 = btrieve(B_GETGREATER, TMS_ROSTER, &ROSTER, &ROSTERKey1, 1);
    while(rcode2 == 0 &&
          ROSTER.DIVISIONSrecordID == DIVISIONS.recordID)
    {
      if(StatusBarAbort())
      {
        goto deallocate;
      }
//
//  Get the employee - must be assigned
//
      StatusBar((long)(seq + 1), (long)numRosters);
      if(ROSTER.DRIVERSrecordID != NO_RECORD)
      {
        DRIVERSKey0.recordID = ROSTER.DRIVERSrecordID;
        btrieve(B_GETEQUAL, TMS_DRIVERS, &DRIVERS, &DRIVERSKey0, 0);
        strncpy(szEmployee, DRIVERS.badgeNumber, TMSRPT54_MAX_EMPLOYEENUMBERLENGTH);
        szEmployee[TMSRPT54_MAX_EMPLOYEENUMBERLENGTH] = '\0';
//
//  Loop through the runs
//
        for(nJ = 0; nJ < ROSTER_MAX_DAYS; nJ++)
        {
          if(StatusBarAbort())
          {
            goto deallocate;
          }
          if(ROSTER.WEEK[m_RosterWeek].RUNSrecordIDs[nJ] == NO_RECORD)
          {
            continue;
          }
//
//  Establish "today's" date
//
          strcpy(szDate, CalendarDateWithOffset(dd, mm, yyyy, nJ));
//
//  Get the characteristics of the run
//
          RUNSKey0.recordID = ROSTER.WEEK[m_RosterWeek].RUNSrecordIDs[nJ];
          rcode2 = btrieve(B_GETEQUAL, TMS_RUNS, &RUNS, &RUNSKey0, 0);
          if(rcode2 != 0)
          {
            continue;
          }
          sprintf(szRunNumber, "%6ld", RUNS.runNumber);
          numPieces = GetRunElements(NULL, &RUNS, &PROPOSEDRUN, &COST, TRUE);
          platformTime = 0;
          reportTime = 0;
          turninTime = 0;
          travelTime = 0;
          for(nK = 0; nK < numPieces; nK++)
          {
            platformTime += RUNSVIEW[nK].platformTime;
            reportTime += RUNSVIEW[nK].reportTime;
            turninTime += RUNSVIEW[nK].turninTime;
            travelTime += RUNSVIEW[nK].travelTime;
          }
//
//  Spit out the various components
//
          for(nK = 0; nK < TMSRPT54_NUMCOMPONENTS; nK++)
          {
//
//  Set up the output record
//
            strcpy(outputString, szDate);
            strcat(outputString, "\t");
            strcat(outputString, szEmployee);
            strcat(outputString, "\t");
            switch(nK)
            {
//
//  Critical illness
//
              case  TMSRPT54_CRITICALILLNESS:
                strcat(outputString, "BEH");
                time = 0;
                break;
//
//  Holiday pay
//
              case  TMSRPT54_HOLIDAYPAY:
                strcat(outputString, "BIH");
                time = 0;
                break;
//
//  Military leave
//
              case  TMSRPT54_MILITARYLEAVE:
                strcat(outputString, "BML");
                time = 0;
                break;
//
//  Overtime
//
              case  TMSRPT54_OVERTIME: 
                strcat(outputString, "BOT");
                time = (long)(RUNSVIEW[numPieces - 1].overTime / 1.5);
                break;
//
//  Regular pay
//
              case  TMSRPT54_REGULARPAY:
                strcat(outputString, "BRH");
                time = (long)(RUNSVIEW[numPieces - 1].payTime - RUNSVIEW[numPieces - 1].overTime);
                break;
//
//  Annual leave
//
              case  TMSRPT54_ANNUALLEAVE:
                strcat(outputString, "BAH");
                time = 0;
                break;
            }
//
//  Fill in the rest of the record
//
//  Time.  If it's 0, skip the write.
//
            if(time == 0)
            {
              continue;
            }
            sprintf(tempString, "\t%5ld\r\n", time);
            strcat(outputString, tempString);
//
//  Write the record
//
            _lwrite(hfOutputFile, outputString, strlen(outputString));
          }
        }
      }
//
//  Get the next roster record
//
      seq++;
      rcode2 = btrieve(B_GETNEXT, TMS_ROSTER, &ROSTER, &ROSTERKey1, 1);
    }
  }  // nI loop on divisions
  bKeepGoing = TRUE;
//
//  Free allocated memory
//
  deallocate:
    StatusBarEnd();
    _lclose(hfOutputFile);
    SetCursor(hCursorArrow);
    if(!bKeepGoing)
    {
      return(FALSE);
    }
//
//  Let him know
//
    strcpy(tempString, "Kalamazoo Payroll.txt was written to:\n");
    strcpy(szarString, szDatabaseFileName);
    if((ptr = strrchr(szarString, '\\')) != NULL)
    {
      *ptr = '\0';
    }
    strcat(tempString, szarString);
    MessageBox(hWndMain, tempString, TMS, MB_OK);
//
//  All done
//
  return(TRUE);
}

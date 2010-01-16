//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
#include "TMSHeader.h"

//
//  TMSRPT51() - Download to Fleet-Net Payroll System
//
//  Fleet-Net Payroll.TXT (Tab Delimited)
//
//  Date (mm/dd/yyyy)
//  Employee number
//  Payroll Code
//  Hours expressed in seconds
//  Run number
//

#define TMSRPT51_MAX_EMPLOYEENUMBERLENGTH 5
#define TMSRPT51_PLATFORMTIME  0
#define TMSRPT51_MAKEUPTIME    1
#define TMSRPT51_REPORTTIME    2
#define TMSRPT51_TURNINTIME    3 
#define TMSRPT51_TRAVELTIME    4
#define TMSRPT51_OVERTIME      5
#define TMSRPT51_NUMCOMPONENTS 6

BOOL FAR TMSRPT51(TMSRPTPassedDataDef *pPassedData)
{
  REPORTPARMSDef REPORTPARMS;
  PROPOSEDRUNDef PROPOSEDRUN;
  COSTDef COST;
  HFILE hfOutputFile;
  BOOL  bKeepGoing = FALSE;
  BOOL  bRC;
  BOOL  bCoach;
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
  REPORTPARMS.pDivisionList = NULL;
  REPORTPARMS.flags = RPFLAG_DIVISIONS;
  bRC = DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_RPTPARMS),
        hWndMain, (DLGPROC)RPTPARMSMsgProc, (LPARAM)&REPORTPARMS);
  if(!bRC)
    return(FALSE);
//
//  Check the list of routes, services, and divisions
//
  if(REPORTPARMS.numDivisions == 0)
    goto deallocate;
//
//  Open the output file
//
  strcpy(tempString, szDatabaseFileName);
  if((ptr = strrchr(tempString, '\\')) != NULL)
    *ptr = '\0';
  strcat(tempString, "\\Fleet-Net Payroll.txt");
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
    bCoach = (strstr(tempString, "Coach") > 0);
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
        strncpy(szEmployee, DRIVERS.badgeNumber, TMSRPT51_MAX_EMPLOYEENUMBERLENGTH);
        szEmployee[TMSRPT51_MAX_EMPLOYEENUMBERLENGTH] = '\0';
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
          for(nK = 0; nK < TMSRPT51_NUMCOMPONENTS; nK++)
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
//  Platform time
//
              case  TMSRPT51_PLATFORMTIME:
                strcat(outputString, (bCoach ? "PLT" : "PLV"));
                time = platformTime;
                break;
//
//  Makeup time
//
              case  TMSRPT51_MAKEUPTIME:
                strcat(outputString, (bCoach ? "GCS" : "GUV"));
                time = RUNSVIEW[numPieces - 1].makeUpTime;
                break;
//
//  Report time
//
              case  TMSRPT51_REPORTTIME:
                strcat(outputString, (bCoach ? "REC" : "RPV"));
                time = reportTime;
                break;
//
//  Turnin time
//
              case  TMSRPT51_TURNINTIME: 
                strcat(outputString, (bCoach ? "CHT" : "CHV"));
                time = turninTime;
                break;
//
//  Travel time
//
              case  TMSRPT51_TRAVELTIME:
                strcat(outputString, (bCoach ? "TRT" : "TRV"));
                time = travelTime;
                break;
//
//  Overtime
//
              case  TMSRPT51_OVERTIME:
                strcat(outputString, (bCoach ? "OCS" : "OVS"));
                time = (long)(RUNSVIEW[numPieces - 1].overTime / 1.5);
                break;
            }
//
//  Fill in the rest of the record
//
//  Time
//
            sprintf(tempString, "\t%5ld\t", time);
            strcat(outputString, tempString);
//
//  Run number
//
            strcat(outputString, szRunNumber);
            strcat(outputString, "\r\n");
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
    TMSHeapFree(REPORTPARMS.pDivisionList);
    StatusBarEnd();
    _lclose(hfOutputFile);
    SetCursor(hCursorArrow);
    if(!bKeepGoing)
      return(FALSE);
//
//  Let him know
//
    strcpy(tempString, "Fleet-Net Payroll.txt was written to:\n");
    strcpy(szarString, szDatabaseFileName);
    if((ptr = strrchr(szarString, '\\')) != NULL)
      *ptr = '\0';
    strcat(tempString, szarString);
    MessageBox(hWndMain, tempString, TMS, MB_OK);
//
//  All done
//
  return(TRUE);
}

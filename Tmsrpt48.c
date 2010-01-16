//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
//
//  TMSRPT48() - On-Time Performance Report (2)
//
//  From QETXT.INI
//
//    [TMSRPT48]
//    FILE=TMSRPT48.TXT
//    FLN=0
//    TT=Tab
//    FIELD1=Year,NUMERIC,4,0,4,0,
//    FIELD2=Month,NUMERIC,2,0,2,0,
//    FIELD3=Service,VARCHAR,32,0,32,0,
//    FIELD4=Route,VARCHAR,32,0,32,0,
//    FIELD5=Timepoint,VARCHAR,4,0,4,0,
//    FIELD6=OnTimeExact,NUMERIC,8,0,8,0,
//    FIELD7=OnTimeOntToFive,NUMERIC,8,0,8,0,
//    FIELD8=Early,NUMERIC,8,0,8,0,
//    FIELD9=Late,NUMERIC,8,0,8,0,
//    FIELD10=ServiceSort,NUMERIC,8,0,8,0,
//
#include "TMSHeader.h"

EXTERN

BOOL FAR TMSRPT48(TMSRPTPassedDataDef *pPassedData)
{
  TMSRPT48PassedDataDef TMSRPT48PassedData;
  HANDLE hOutputFile;
  DWORD  dwBytesWritten;
  BOOL  bRC;
  BOOL  bKeepGoing;
  char  dummy[256];
  char  outString[512];
  long  scheduledTime;
  long  startDate;
  long  endDate;
  long  numTimechecks;
  long  timechecksRead;
  int   year, month, day;
  int   rcode2;
  int   onTimeExact;
  int   onTimeOneToFive;
  int   early;
  int   late;

  pPassedData->nReportNumber = 47;
  pPassedData->numDataFiles = 1;
  bRC = TMSRPT48Filter(&TMSRPT48PassedData);
  if(!bRC)
    return(FALSE);
//
//  Open the output file
//
  strcpy(tempString, szReportsTempFolder);
  strcat(tempString, "\\TMSRPT48.TXT");
  hOutputFile = CreateFile(tempString, GENERIC_WRITE, 0, NULL,
        CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
  if(hOutputFile == INVALID_HANDLE_VALUE)
  {
    LoadString(hInst, ERROR_202, szFormatString, sizeof(szFormatString));
    sprintf(szarString, szFormatString, tempString);
    MessageBeep(MB_ICONSTOP);
    MessageBox((HWND)NULL, szarString, TMS, MB_ICONSTOP);
    goto deallocate;
  }
  strcpy(pPassedData->szReportDataFile[0], tempString);
//
//  Get the number of records in the TIMECHECKS file
//
  rcode2 = btrieve(B_STAT, TMS_TIMECHECKS, &BSTAT, dummy, 0);
  if(rcode2 != 0 || BSTAT.numRecords == 0)
  {
    TMSError(NULL, MB_ICONSTOP, ERROR_313, (HANDLE)NULL);
    SendMessage(NULL, WM_COMMAND, MAKEWPARAM(IDCANCEL, 0), (LPARAM)0);
    goto deallocate;
  }
  numTimechecks = BSTAT.numRecords;
//
//  Start the status bar
//
  LoadString(hInst, TEXT_117, tempString, TEMPSTRING_LENGTH);
  StatusBarStart(hWndMain, tempString);
//
//  Loop through the records
//
  startDate = TMSRPT48PassedData.fromYear * 10000 + TMSRPT48PassedData.fromMonth * 100;
  endDate = TMSRPT48PassedData.toYear * 10000 + (TMSRPT48PassedData.toMonth + 1) * 100;
  bKeepGoing = TRUE;
  TIMECHECKSKey1.checkDate = startDate;
  timechecksRead = 0;
  rcode2 = btrieve(B_GETGREATER, TMS_TIMECHECKS, &TIMECHECKS, &TIMECHECKSKey1, 1);
  while(rcode2 == 0 &&
        TIMECHECKS.checkDate >= startDate &&
        TIMECHECKS.checkDate <= endDate)
  {
    if(StatusBarAbort())
    {
      bKeepGoing = FALSE;
      break;
    }
    StatusBar(timechecksRead, numTimechecks);
//
//  Construct the output record
//
//  Date
//
    GetYMD(TIMECHECKS.checkDate, &year, &month, &day);
    sprintf(outString, "%04d\t%02d\t", year, month);
//
//  Service
//
    SERVICESKey0.recordID = TIMECHECKS.SERVICESrecordID;
    btrieve(B_GETEQUAL, TMS_SERVICES, &SERVICES, &SERVICESKey0, 0);
    strncpy(szarString, SERVICES.name, SERVICES_NAME_LENGTH);
    trim(szarString, SERVICES_NAME_LENGTH);
    strcat(outString, szarString);
    strcat(outString, "\t");
//
//  Route
//
    ROUTESKey0.recordID = TIMECHECKS.ROUTESrecordID;
    btrieve(B_GETEQUAL, TMS_ROUTES, &ROUTES, &ROUTESKey0, 0);
    strncpy(szarString, ROUTES.name, ROUTES_NAME_LENGTH);
    trim(szarString, ROUTES_NAME_LENGTH);
    strcat(outString, szarString);
    strcat(outString, "\t");
//
//  Timepoint
//
    NODESKey0.recordID = TIMECHECKS.NODESrecordID;
    btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
    strncpy(szarString, NODES.abbrName, NODES_ABBRNAME_LENGTH);
    trim(szarString, NODES_ABBRNAME_LENGTH);
    strcat(outString, szarString);
    strcat(outString, "\t");
//
//  Time of day - moved here 'cause other, easier, conditions earlier.  
//  If he passes this test, he's Jake.
//
    scheduledTime = GetTimeAtNode(TIMECHECKS.TRIPSrecordID,
          TIMECHECKS.NODESrecordID, TIMECHECKS.nodePositionInTrip, (TRIPSDef *)NULL);
//
//  Time events
//
    onTimeExact = 0;
    onTimeOneToFive = 0;
    early = 0;
    late = 0;
    if(scheduledTime == TIMECHECKS.actualTime)
    {
      onTimeExact = 1;
    }
    else if(scheduledTime > TIMECHECKS.actualTime)
    {
      early = 1;
    }
    else
    {
      if(scheduledTime + 300 > TIMECHECKS.actualTime)
      {
        onTimeOneToFive = 1;
      }
      else
      {
        late = 1;
      }
    }
    sprintf(tempString, "%d\t%d\t%d\t%d\t", onTimeExact, onTimeOneToFive, early, late);
    strcat(outString, tempString);
//
//  Service sort number
//
    sprintf(tempString, "%ld\r\n", SERVICES.number);
    strcat(outString, tempString);
//
//  Write it out
//
    WriteFile(hOutputFile, (LPCVOID *)outString,
              strlen(outString), &dwBytesWritten, NULL);
//
//  Get the next TIMECHECKS record
//
    rcode2 = btrieve(B_GETNEXT, TMS_TIMECHECKS, &TIMECHECKS, &TIMECHECKSKey1, 1);
    timechecksRead++;
  }  // while  (through TIMECHECKS)


//
//  Free allocated memory
//
  deallocate:
    StatusBarEnd();
    CloseHandle(hOutputFile);
    SetCursor(hCursorArrow);
    if(!bKeepGoing)
      return(FALSE);
//
//  All done
//
  return(TRUE);
}

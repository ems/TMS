//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
//
//  TMSRPT36() - On-Time Performance Report (1)
//
//  From QETXT.INI
//
//    [TMSRPT36]
//    FILE=TMSRPT36.TXT
//    FLN=0
//    TT=Tab
//    FIELD1=Date,VARCHAR,11,0,11,0,
//    FIELD2=Service,VARCHAR,32,0,32,0,
//    FIELD3=DayOfWeek,VARCHAR,3,0,3,0,
//    FIELD4=ScheduledTime,NUMERIC,8,0,8,0,
//    FIELD5=ActualTime,NUMERIC,8,0,8,0,
//    FIELD6=Route,VARCHAR,32,0,32,0,
//    FIELD7=Timepoint,VARCHAR,4,0,4,0,
//    FIELD8=Direction,VARCHAR,2,0,2,0,
//    FIELD9=RoadSupervisor,VARCHAR,32,0,32,0,
//    FIELD10=Driver,VARCHAR,66,0,66,0,
//    FIELD11=Bustype,VARCHAR,32,0,32,0,
//    FIELD12=PassengerLoad,32,0,32,0,
//    FIELD13=Weather,32,0,32,0,
//    FIELD14=Traffic,32,0,32,0,
//    FIELD15=OtherComments,32,0,32,0,
//
#include "TMSHeader.h"


BOOL FAR TMSRPT36(TMSRPTPassedDataDef *pPassedData)
{
  TMSRPT36PassedDataDef TMSRPT36PassedData;
  HANDLE hOutputFile;
  DWORD  dwBytesWritten;
  BOOL  bRC;
  BOOL  bKeepGoing;
  char  *daysOfTheWeek[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
  char  outString[512];
  char  dummy[256];
  long  numTimechecks;
  long  timechecksRead;
  long  timeOfDay;
  long  index;
  int   year, month, day;
  int   dayOfWeek;
  int   rcode2;

  pPassedData->nReportNumber = 35;
  pPassedData->numDataFiles = 1;
  bRC = TMSRPT36Filter(&TMSRPT36PassedData);
  if(!bRC)
    return(FALSE);
//
//  Open the output file
//
  strcpy(tempString, szReportsTempFolder);
  strcat(tempString, "\\TMSRPT36.TXT");
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
  bKeepGoing = TRUE;
  TIMECHECKSKey1.checkDate = TMSRPT36PassedData.fromDate;
  timechecksRead = 0;
  rcode2 = btrieve(B_GETGREATEROREQUAL, TMS_TIMECHECKS, &TIMECHECKS, &TIMECHECKSKey1, 1);
  while(rcode2 == 0 &&
        TIMECHECKS.checkDate >= TMSRPT36PassedData.fromDate &&
        TIMECHECKS.checkDate <= TMSRPT36PassedData.toDate)
  {
    if(StatusBarAbort())
    {
      bKeepGoing = FALSE;
      break;
    }
    StatusBar(timechecksRead, numTimechecks);
//
//  Service
//
    if(!(TMSRPT36PassedData.flags & TMSRPT36_FLAG_ALLSERVICES ||
          TMSRPT36PassedData.SERVICESrecordID == TIMECHECKS.SERVICESrecordID))
      goto NextRecord;
//
//  Route
//
    if(!(TMSRPT36PassedData.flags & TMSRPT36_FLAG_ALLROUTES ||
          TMSRPT36PassedData.ROUTESrecordID == TIMECHECKS.ROUTESrecordID))
      goto NextRecord;
//
//  Direction
//
    if(!(TMSRPT36PassedData.flags & TMSRPT36_FLAG_BOTHDIRECTIONS ||
          TMSRPT36PassedData.directionIndex == TIMECHECKS.directionIndex))
      goto NextRecord;
//
//  Timepoint
//
    if(!(TMSRPT36PassedData.flags & TMSRPT36_FLAG_ALLTIMEPOINTS ||
          TMSRPT36PassedData.NODESrecordID == TIMECHECKS.NODESrecordID))
      goto NextRecord;
//
//  Drivers
//
    if(!(TMSRPT36PassedData.flags & TMSRPT36_FLAG_ALLDRIVERS ||
          TMSRPT36PassedData.DRIVERSrecordID == TIMECHECKS.actualDRIVERSrecordID))
      goto NextRecord;
//
//  Exclusions
//
    if(!(TMSRPT36PassedData.flags & TMSRPT36_FLAG_DONOTEXCLUDE ||
          TMSRPT36PassedData.excludedDRIVERSrecordID != TIMECHECKS.actualDRIVERSrecordID))
      goto NextRecord;
//
//  Bustypes
//
    if(!(TMSRPT36PassedData.flags & TMSRPT36_FLAG_ALLBUSTYPES ||
          TMSRPT36PassedData.BUSTYPESrecordID == TIMECHECKS.actualBUSTYPESrecordID))
      goto NextRecord;
//
//  Passenger Load
//
    if(!(TMSRPT36PassedData.flags & TMSRPT36_FLAG_ALLPASSENGERLOADS ||
          TMSRPT36PassedData.passengerLoadIndex == TIMECHECKS.passengerLoadIndex))
      goto NextRecord;
//
//  Road Supervisors
//
    if(!(TMSRPT36PassedData.flags & TMSRPT36_FLAG_ALLROADSUPERVISORS ||
          TMSRPT36PassedData.roadSupervisorIndex == TIMECHECKS.supervisorIndex))
      goto NextRecord;
//
//  Day of the week - moved here 'cause other, easier, conditions earlier.
//
    dayOfWeek = GetDayFromDate(TIMECHECKS.checkDate);
    if(!(TMSRPT36PassedData.flags & TMSRPT36_FLAG_ALLSERVICEDAYS ||
          TMSRPT36PassedData.dayNumber == dayOfWeek))
      goto NextRecord;
//
//  Time of day - moved here 'cause other, easier, conditions earlier.  
//  If he passes this test, he's Jake.
//
    timeOfDay = GetTimeAtNode(TIMECHECKS.TRIPSrecordID,
          TIMECHECKS.NODESrecordID, TIMECHECKS.nodePositionInTrip, (TRIPSDef *)NULL);
    if(!(TMSRPT36PassedData.flags & TMSRPT36_FLAG_ALLDAY))
    {
      timeOfDay = GetTimeAtNode(TIMECHECKS.TRIPSrecordID,
            TIMECHECKS.NODESrecordID, TIMECHECKS.nodePositionInTrip, (TRIPSDef *)NULL);
      if(timeOfDay < TMSRPT36PassedData.timeOfDayFrom ||
            timeOfDay > TMSRPT36PassedData.timeOfDayTo)
        goto NextRecord;
    }
//
//  Passed the tests - construct the output record
//
//  Date
//
    GetYMD(TIMECHECKS.checkDate, &year, &month, &day);
    sprintf(outString, "%04d/%02d/%02d\t", year, month, day);
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
//  Day of week
//
    if(TMSRPT36PassedData.flags & TMSRPT36_FLAG_ALLSERVICEDAYS)
      dayOfWeek = GetDayFromDate(TIMECHECKS.checkDate);
    else
      dayOfWeek = TMSRPT36PassedData.dayNumber;
    strcat(outString, daysOfTheWeek[dayOfWeek - 1]);
    strcat(outString, "\t");
//
//  Scheduled time
//
    sprintf(szarString, "%ld\t", timeOfDay);
    strcat(outString, szarString);
//
//  Actual time
//
    sprintf(szarString, "%ld\t", TIMECHECKS.actualTime);
    strcat(outString, szarString);
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
//  Direction
//
    DIRECTIONSKey0.recordID = ROUTES.DIRECTIONSrecordID[TIMECHECKS.directionIndex];
    btrieve(B_GETEQUAL, TMS_DIRECTIONS, &DIRECTIONS, &DIRECTIONSKey0, 0);
    strncpy(szarString, DIRECTIONS.abbrName, DIRECTIONS_ABBRNAME_LENGTH);
    trim(szarString, DIRECTIONS_ABBRNAME_LENGTH);
    strcat(outString, szarString);
    strcat(outString, "\t");
//
//  Road Supervisor
//
    if(TIMECHECKS.supervisorIndex >= 0 && TIMECHECKS.supervisorIndex < MAXROADSUPERVISORS)
    {
      strcat(outString, m_RoadSupervisors[TIMECHECKS.supervisorIndex].szText);
    }
    strcat(outString, "\t");
//
//  Driver
//
    if(TIMECHECKS.actualDRIVERSrecordID != NO_RECORD)
    {
      DRIVERSKey0.recordID = TIMECHECKS.actualDRIVERSrecordID;
      btrieve(B_GETEQUAL, TMS_DRIVERS, &DRIVERS, &DRIVERSKey0, 0);
      strncpy(szarString, DRIVERS.lastName, DRIVERS_LASTNAME_LENGTH);
      trim(szarString, DRIVERS_LASTNAME_LENGTH);
      strcat(outString, szarString);
      strcat(outString, ", ");
      strncpy(szarString, DRIVERS.firstName, DRIVERS_FIRSTNAME_LENGTH);
      trim(szarString, DRIVERS_FIRSTNAME_LENGTH);
      strcat(outString, szarString);
    }
    strcat(outString, "\t");
//
//  Bustype
//
    if(TIMECHECKS.actualBUSTYPESrecordID != NO_RECORD)
    {
      BUSTYPESKey0.recordID = TIMECHECKS.actualBUSTYPESrecordID;
      btrieve(B_GETEQUAL, TMS_BUSTYPES, &BUSTYPES, &BUSTYPESKey0, 0);
      strncpy(szarString, BUSTYPES.name, BUSTYPES_NAME_LENGTH);
      trim(szarString, BUSTYPES_NAME_LENGTH);
      strcat(outString,szarString);
    }
    strcat(outString, "\t");
//
//  Passenger Load
//
    if(TIMECHECKS.passengerLoadIndex >= 0 && TIMECHECKS.passengerLoadIndex < MAXPASSENGERLOADS)
    {
      strcat(outString, m_PassengerLoads[TIMECHECKS.passengerLoadIndex].szText);
    }
    strcat(outString, "\t");
//
//  Weather
//
    index = (long)LOWORD(TIMECHECKS.weatherIndex);
    if(index >= 0 && index < MAXWEATHERCONDITIONS)
    {
      strcat(outString, m_WeatherConditions[index].szText);
    }
    strcat(outString, "\t");
//
//  Traffic
//
    index = (long)HIWORD(TIMECHECKS.weatherIndex);
    if(index >= 0 && index < MAXTRAFFICCONDITIONS)
    {
      strcat(outString, m_TrafficConditions[index].szText);
    }
    strcat(outString, "\t");
//
//  Other Comments
//
    if(TIMECHECKS.otherCommentsIndex >= 0 && TIMECHECKS.otherCommentsIndex < MAXOTHERCOMMENTS)
    {
      strcat(outString, m_OtherComments[TIMECHECKS.otherCommentsIndex].szText);
    }
    strcat(outString, "\r\n");
//
//  Write it out
//
    WriteFile(hOutputFile, (LPCVOID *)outString,
              strlen(outString), &dwBytesWritten, NULL);
//
//  Get the next TIMECHECKS record
//
    NextRecord:
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

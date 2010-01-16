//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
#include "TMSHeader.h"

//
//  Daily Operations Audit Trail
//
//  From QETXT.INI:
//
//    [TMSRPT43]
//    FILE=TMSRPT43.txt
//    FLN=0
//    TT=Tab
//    Charset=ANSI
//    DS=.
//    FIELD1=Type,VARCHAR,32,0,32,0,
//    FIELD2=Date,VARCHAR,32,0,32,0,
//    FIELD3=Time,VARCHAR,32,0,32,0,
//    FIELD4=Sequence,NUMERIC,8,0,8,0,
//    FIELD5=Text,VARCHAR,256,0,256,0,
//
BOOL FAR TMSRPT43(TMSRPTPassedDataDef *pPassedData)
{
  AUDITFILTERSELECTIONSDef AuditSelections;
  HANDLE hOutputFile;
  struct tm *entryDateAndTime;
  DWORD  dwBytesWritten;
  long  totalTime;
  BOOL  bDoAll;
  BOOL  bKeepGoing = FALSE;
  BOOL  bCheckin;
  BOOL  bDoneOne;
  BOOL  bGotOlder;
  char  busNumber[BUSES_NUMBER_LENGTH + 1];
  char  newBusNumber[BUSES_NUMBER_LENGTH + 1];
  char  routeNumber[ROUTES_NUMBER_LENGTH + 1];
  char  routeName[ROUTES_NAME_LENGTH + 1];
  char  serviceName[SERVICES_NAME_LENGTH + 1];
  char  lastName[DRIVERS_LASTNAME_LENGTH + 1];
  char  firstName[DRIVERS_FIRSTNAME_LENGTH + 1];
  char  badgeNumber[DRIVERS_BADGENUMBER_LENGTH + 1];
  char  outputString[512];
  long  year, month, day;
  long  blockNumber;
  long  fromYear, fromMonth, fromDay;
  long  toYear, toMonth, toDay;
  long  DAILYOPSrecordID;
  long  absRecID;
  long  seq;
  long  reasonIndex;
  long  tempLong;
  long  dateOfEntry;
  long  runNumber;
  int   rcode2;
  int   resID;

  pPassedData->nReportNumber = 42;
  pPassedData->numDataFiles = 1;
  totalTime = 0;
//
//  See what's to be filtered
//
  if(AuditFilter(&AuditSelections) != IDOK)
  {
    hOutputFile = NULL;
    goto deallocate;
  }
//
//  Open the output file
//
  strcpy(tempString, szReportsTempFolder);
  strcat(tempString, "\\TMSRPT43.txt");
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
  StatusBarStart(hWndMain, "Processing selections...");
  StatusBar(-1L, -1L);
  bKeepGoing = TRUE;
//
//  Set up the flags for retrieval
//
  tempLong = (AuditSelections.flags & AUDITFILTER_FLAGS_REGISTRATIONDATE) ? AuditSelections.fromDate : 0;
  DAILYOPSKey5.entryDateAndTime = TMSmktime(tempLong, 0L);
  DAILYOPSKey5.pertainsToDate = 0;
  DAILYOPSKey5.pertainsToTime = 0;
  DAILYOPSKey5.recordTypeFlag = 0;
  DAILYOPSKey5.recordFlags = 0;
//
//  Cycle through
//
  seq = 0;
  rcode2 = btrieve(B_GETGREATER, TMS_DAILYOPS, &DAILYOPS, &DAILYOPSKey5, 5);
  while(rcode2 == 0)
  {
    entryDateAndTime = localtime(&DAILYOPS.entryDateAndTime);
    dateOfEntry = (entryDateAndTime->tm_year + 1900) * 10000 + (entryDateAndTime->tm_mon + 1) * 100 + entryDateAndTime->tm_mday;
    if(AuditSelections.flags & AUDITFILTER_FLAGS_REGISTRATIONDATE)
    {
      if(dateOfEntry > AuditSelections.toDate)
      {
        break;
      }
    }
//
//  What are we doing?
//
    bDoAll = AuditSelections.flags & AUDITFILTER_FLAGS_ALL;
    bDoneOne = FALSE;
//
//  Build the output record
//
//  Type
//
    switch(DAILYOPS.recordTypeFlag)
    {
      case DAILYOPS_FLAG_ROSTER:
        resID = TEXT_305;
        break;
      case DAILYOPS_FLAG_BUS:
        resID = TEXT_306;
        break;
      case DAILYOPS_FLAG_OPERATOR:
        resID = TEXT_307;
        break;
      case DAILYOPS_FLAG_ABSENCE:
        resID = TEXT_308;
        break;
      case DAILYOPS_FLAG_OPENWORK:
        resID = TEXT_309;
        break;
      default:
        resID = TEXT_310;
        break;
    }
    LoadString(hInst, resID, tempString, TEMPSTRING_LENGTH);
    strcpy(outputString, tempString);
    strcat(outputString, "\t");
//
//  Date
//
    sprintf(tempString, "%02ld-%s-%04ld\t",
          entryDateAndTime->tm_mday, szMonth[entryDateAndTime->tm_mon], entryDateAndTime->tm_year + 1900);
    strcat(outputString, tempString);
//
//  Time
//
    tempLong = (entryDateAndTime->tm_hour * 3600) + (entryDateAndTime->tm_min * 60) + (entryDateAndTime->tm_sec);
    strcat(outputString, Tchar(tempLong));
    strcat(outputString, "\t");
//
//  Sequence
//
    sprintf(tempString, "%ld\t", seq++);
    strcat(outputString, tempString);
//
//  Roster
//
    if((bDoAll || (AuditSelections.flags & AUDITFILTER_FLAGS_ROSTER)) && !bDoneOne)
    {
      if(DAILYOPS.recordTypeFlag & DAILYOPS_FLAG_ROSTER)
      {
        if(DAILYOPS.DRIVERSrecordID == NO_RECORD)
        {
          ROSTERKey0.recordID = DAILYOPS.DOPS.RosterTemplate.ROSTERrecordID;
          btrieve(B_GETEQUAL, TMS_ROSTER, &ROSTER, &ROSTERKey0, 0);
          sprintf(tempString, "Roster number %ld established as Open", ROSTER.rosterNumber);
        }
        else
        {
          DRIVERSKey0.recordID = DAILYOPS.DRIVERSrecordID;
          rcode2 = btrieve(B_GETEQUAL, TMS_DRIVERS, &DRIVERS, &DRIVERSKey0, 0);
          if(rcode2 != 0)
          {
            strcpy(tempString, "*** Driver not found ***");
          }
          else
          {
            strncpy(badgeNumber, DRIVERS.badgeNumber, DRIVERS_BADGENUMBER_LENGTH);
            trim(badgeNumber, DRIVERS_BADGENUMBER_LENGTH);
            strncpy(lastName, DRIVERS.lastName, DRIVERS_LASTNAME_LENGTH);
            trim(lastName, DRIVERS_LASTNAME_LENGTH);
            strncpy(firstName, DRIVERS.firstName, DRIVERS_FIRSTNAME_LENGTH);
            trim(firstName, DRIVERS_FIRSTNAME_LENGTH);
            strcpy(tempString, badgeNumber);
            strcat(tempString, " - ");
            strcat(tempString, lastName);
            strcat(tempString, ", ");
            strcat(tempString, firstName);
          }
          strcat(outputString, tempString);
          ROSTERKey0.recordID = DAILYOPS.DOPS.RosterTemplate.ROSTERrecordID;
          btrieve(B_GETEQUAL, TMS_ROSTER, &ROSTER, &ROSTERKey0, 0);
          sprintf(tempString, " - Established as operator of roster number %ld", ROSTER.rosterNumber);
        }
        strcat(outputString, tempString);
        bDoneOne = TRUE;
      }
    }
//
//  Vehicle assignments
//
    if((bDoAll || (AuditSelections.flags & AUDITFILTER_FLAGS_BUS)) && !bDoneOne)
    {
      if(DAILYOPS.recordTypeFlag & DAILYOPS_FLAG_BUS)
      {
//
//  Get and format all the pertinent data
//
//  RGRP
//
        ROUTESKey0.recordID = DAILYOPS.DOPS.Bus.RGRPROUTESrecordID;
        btrieve(B_GETEQUAL, TMS_ROUTES, &ROUTES, &ROUTESKey0, 0);
        strncpy(routeNumber, ROUTES.number, ROUTES_NUMBER_LENGTH);
        trim(routeNumber, ROUTES_NUMBER_LENGTH);
        strncpy(routeName, ROUTES.name, ROUTES_NAME_LENGTH);
        trim(routeName, ROUTES_NAME_LENGTH);
//
//  SGRP
//
        SERVICESKey0.recordID = DAILYOPS.DOPS.Bus.SGRPSERVICESrecordID;
        btrieve(B_GETEQUAL, TMS_SERVICES, &SERVICES, &SERVICESKey0, 0);
        strncpy(serviceName, SERVICES.name, SERVICES_NAME_LENGTH);
        trim(serviceName, SERVICES_NAME_LENGTH);
//
//  Bus
//
        BUSESKey0.recordID = DAILYOPS.DOPS.Bus.BUSESrecordID;
        btrieve(B_GETEQUAL, TMS_BUSES, &BUSES, &BUSESKey0, 0);
        strncpy(busNumber, BUSES.number, BUSES_NUMBER_LENGTH);
        trim(busNumber, BUSES_NUMBER_LENGTH);
//
//  Block number
//
        TRIPSKey0.recordID = DAILYOPS.DOPS.Bus.TRIPSrecordID;
        btrieve(B_GETEQUAL, TMS_TRIPS, &TRIPS, &TRIPSKey0, 0);
        blockNumber = TRIPS.standard.blockNumber;
//
//  New (swapped in) bus
//
        BUSESKey0.recordID = DAILYOPS.DOPS.Bus.newBUSESrecordID;
        btrieve(B_GETEQUAL, TMS_BUSES, &BUSES, &BUSESKey0, 0);
        strncpy(newBusNumber, BUSES.number, BUSES_NUMBER_LENGTH);
        trim(newBusNumber, BUSES_NUMBER_LENGTH);
//
//  Run number
//
        RUNSKey0.recordID = DAILYOPS.DOPS.Bus.RUNSrecordID;
        btrieve(B_GETEQUAL, TMS_RUNS, &RUNS, &RUNSKey0, 0);
        runNumber = RUNS.runNumber;
//
//  Now, the various flags within bus
//
//  RecordID
//
        sprintf(tempString, "%ld  ", DAILYOPS.recordID);
        strcat(outputString, tempString);
//
//  Pertains to date/time
//
        sprintf(tempString, "%ld/%ld ", DAILYOPS.pertainsToDate, DAILYOPS.pertainsToTime);
        strcat(outputString, tempString);
//
//  DAILYOPS_FLAG_BUSASSIGNMENT
//

        if(DAILYOPS.recordFlags & DAILYOPS_FLAG_BUSASSIGNMENT)
        {
          LoadString(hInst, TEXT_274, szFormatString, SZFORMATSTRING_LENGTH);
          sprintf(tempString, szFormatString,
                busNumber, runNumber, blockNumber, routeNumber, routeName, serviceName);
          strcat(outputString, tempString);
          bDoneOne = TRUE;
        }
//
//  Bus swap
//
        else if(DAILYOPS.recordFlags & DAILYOPS_FLAG_BUSSWAP)
        {
          if(DAILYOPS.DOPS.Bus.swapReasonIndex >= 0 && DAILYOPS.DOPS.Bus.swapReasonIndex < MAXBUSSWAPREASONS)
          {
            strcpy(szarString, m_BusSwapReasons[MAXBUSSWAPREASONS].szText);
          }
          else
          {
            sprintf(szarString, "* Unknown reason (%d)", DAILYOPS.DOPS.Bus.swapReasonIndex);
          }
          LoadString(hInst, TEXT_276, szFormatString, SZFORMATSTRING_LENGTH);
          sprintf(tempString, szFormatString, busNumber, newBusNumber, szarString);
          strcat(outputString, tempString);
          bDoneOne = TRUE;
        }
//
//  Bus return
//
        else if(DAILYOPS.recordFlags & DAILYOPS_FLAG_BUSRETURN)
        {
          LoadString(hInst, TEXT_277, szFormatString, SZFORMATSTRING_LENGTH);
          sprintf(tempString, szFormatString, busNumber);
          strcat(outputString, tempString);
          bDoneOne = TRUE;
        }
//
//  Bus override
//
        else if(DAILYOPS.recordFlags & DAILYOPS_FLAG_BUSOVERRIDE)
        {
          sprintf(tempString, "Bus %s: Override", busNumber);
          strcat(outputString, tempString);
          bDoneOne = TRUE;
        }
//
//  Clear all
//
        else if(DAILYOPS.recordFlags & DAILYOPS_FLAG_BUSCLEARALL)
        {
          LoadString(hInst, TEXT_280, tempString, TEMPSTRING_LENGTH);
          strcat(outputString, tempString);
          bDoneOne = TRUE;
        }
//
//  Bus missed assignment
//
        else if(DAILYOPS.recordFlags & DAILYOPS_FLAG_BUSMISSEDASSIGNMENT)
        {
          sprintf(tempString, "Bus %s: Missed assignment", busNumber);
          strcat(outputString, tempString);
          bDoneOne = TRUE;
        }
//
//  Block drop
//
        else if(DAILYOPS.recordFlags & DAILYOPS_FLAG_BLOCKDROP)
        {
          strcat(outputString, "Block Drop ");
          bDoneOne = TRUE;
        }
//
//  Block undrop
//
        else if(DAILYOPS.recordFlags & DAILYOPS_FLAG_BLOCKUNDROP)
        {
          strcat(outputString, "Block Undrop ");
          bDoneOne = TRUE;
        }
//
//  Set location
//
        else if(DAILYOPS.recordFlags & DAILYOPS_FLAG_SETBUSLOCATION)
        {  
          NODESKey0.recordID = DAILYOPS.DOPS.Bus.locationNODESrecordID;
          btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
          strncpy(szarString, NODES.abbrName, NODES_ABBRNAME_LENGTH);
          trim(szarString, NODES_ABBRNAME_LENGTH);
          sprintf(tempString, "Bus %s: Set location to %s", busNumber, szarString);
          strcat(outputString, tempString);
          bDoneOne = TRUE;
        }
        else if(DAILYOPS.recordFlags & DAILYOPS_FLAG_BUSMARKEDSHORTSHIFT)
        {
          sprintf(tempString, "Bus %s: Marked as Short Shift", busNumber);
          strcat(outputString, tempString);
          bDoneOne = TRUE;
        }
        else if(DAILYOPS.recordFlags & DAILYOPS_FLAG_BUSUNMARKEDSHORTSHIFT)
        {
          sprintf(tempString, "Bus %s: Unmarked as Short Shift", busNumber);
          strcat(outputString, tempString);
          bDoneOne = TRUE;
        }
        else if(DAILYOPS.recordFlags & DAILYOPS_FLAG_BUSMARKEDASCHARTER)
        {
          sprintf(tempString, "Bus %s: Marked as Charter", busNumber);
          strcat(outputString, tempString);
          bDoneOne = TRUE;
        }
        else if(DAILYOPS.recordFlags & DAILYOPS_FLAG_BUSUNMARKEDASCHARTER)
        {
          sprintf(tempString, "Bus %s: Unmarked as Charter", busNumber);
          strcat(outputString, tempString);
          bDoneOne = TRUE;
        }
        else if(DAILYOPS.recordFlags & DAILYOPS_FLAG_BUSMARKEDASSIGHTSEEING)
        {
          sprintf(tempString, "Bus %s: Marked as Sightseeing", busNumber);
          strcat(outputString, tempString);
          bDoneOne = TRUE;
        }
        else if(DAILYOPS.recordFlags & DAILYOPS_FLAG_BUSUNMARKEDASSIGHTSEEING)
        {
          sprintf(tempString, "Bus %s: Unmarked as Sightseeing", busNumber);
          strcat(outputString, tempString);
          bDoneOne = TRUE;
        }
      }
    }
//
//  Operator check-ins and outs
//
    if((bDoAll || (AuditSelections.flags & AUDITFILTER_FLAGS_OPERATOR)) && !bDoneOne)
    {
      if(DAILYOPS.recordTypeFlag & DAILYOPS_FLAG_OPERATOR)
      {
//
//  Get and format all the pertinent data
//
        DRIVERSKey0.recordID = DAILYOPS.DRIVERSrecordID;
        rcode2 = btrieve(B_GETEQUAL, TMS_DRIVERS, &DRIVERS, &DRIVERSKey0, 0);
        if(rcode2 != 0)
        {
        }
        else
        {
          bCheckin = DAILYOPS.recordFlags & DAILYOPS_FLAG_OPERATORCHECKIN;
          strncpy(badgeNumber, DRIVERS.badgeNumber, DRIVERS_BADGENUMBER_LENGTH);
          trim(badgeNumber, DRIVERS_BADGENUMBER_LENGTH);
          strncpy(lastName, DRIVERS.lastName, DRIVERS_LASTNAME_LENGTH);
          trim(lastName, DRIVERS_LASTNAME_LENGTH);
          strncpy(firstName, DRIVERS.firstName, DRIVERS_FIRSTNAME_LENGTH);
          trim(firstName, DRIVERS_FIRSTNAME_LENGTH);
          strcpy(tempString, badgeNumber);
          strcat(tempString, " - ");
          strcat(tempString, lastName);
          strcat(tempString, ", ");
          strcat(tempString, firstName);
          if(DAILYOPS.pertainsToDate != NO_RECORD)
          {
            GetYMD(DAILYOPS.pertainsToDate, &year, &month, &day);
          }
        }
//
//  Check-in
//
        if(DAILYOPS.recordFlags & DAILYOPS_FLAG_OPERATORCHECKIN)
        {
          LoadString(hInst, TEXT_284, szFormatString, SZFORMATSTRING_LENGTH);
        }
//
//  Check-out
//
        else if(DAILYOPS.recordFlags & DAILYOPS_FLAG_OPERATORCHECKOUT)
        {
          LoadString(hInst, TEXT_285, szFormatString, SZFORMATSTRING_LENGTH);
        }
//
//  Uncheck (in/out)
//
        else if(DAILYOPS.recordFlags & DAILYOPS_FLAG_OPERATORUNCHECK)
        {
          DAILYOPSrecordID = DAILYOPS.DAILYOPSrecordID;
          btrieve(B_GETPOSITION, TMS_DAILYOPS, &absRecID, &DAILYOPSKey5, 5);
          DAILYOPSKey0.recordID = DAILYOPSrecordID;
          btrieve(B_GETEQUAL, TMS_DAILYOPS, &DAILYOPS, &DAILYOPSKey0, 0);
          if(DAILYOPS.recordFlags & DAILYOPS_FLAG_OPERATORCHECKIN)
          {
            LoadString(hInst, TEXT_286, szFormatString, SZFORMATSTRING_LENGTH);
          }
          else
          {
            LoadString(hInst, TEXT_287, szFormatString, SZFORMATSTRING_LENGTH);
          }
        }
//
//  Deassign
//
        else if(DAILYOPS.recordFlags & DAILYOPS_FLAG_OPERATORDEASSIGN)
        {
          LoadString(hInst, TEXT_402, szFormatString, SZFORMATSTRING_LENGTH);
        }
//
//  Overtime
//
        else if(DAILYOPS.recordFlags & DAILYOPS_FLAG_OVERTIME)
        {
          LoadString(hInst, TEXT_403, szFormatString, SZFORMATSTRING_LENGTH);
        }
//
//   Extra trip
//
        else if(DAILYOPS.recordFlags & DAILYOPS_FLAG_EXTRATRIP)
        {
          LoadString(hInst, TEXT_404, szFormatString, SZFORMATSTRING_LENGTH);
        }
//
//  Format it
//
        sprintf(szarString, szFormatString, tempString);
        strcat(outputString, szarString);
        bDoneOne = TRUE;
      }
    } 
//
//  Operator absences
//
    if((bDoAll || (AuditSelections.flags & AUDITFILTER_FLAGS_ABSENCE)) && !bDoneOne)
    {
      if(DAILYOPS.recordTypeFlag & DAILYOPS_FLAG_ABSENCE)
      {
//
//  Examine by registration date
//
        if(AuditSelections.flags & AUDITFILTER_FLAGS_REGISTRATIONDATE)
        {
//
//  Driver
//
          if(bDoAll ||
             (((AuditSelections.flags & AUDITFILTER_FLAGS_ALLOPERATORS) ||
                AuditSelections.DRIVERSrecordID == DAILYOPS.DRIVERSrecordID) &&
             ((AuditSelections.flags & AUDITFILTER_FLAGS_ALLREASONS) ||
                AuditSelections.reasonNumber == DAILYOPS.DOPS.Absence.reasonIndex)))
          {
            DRIVERSKey0.recordID = DAILYOPS.DRIVERSrecordID;
            rcode2 = btrieve(B_GETEQUAL, TMS_DRIVERS, &DRIVERS, &DRIVERSKey0, 0);
            if(rcode2 != 0)
            {
              strcpy(tempString, "Driver not found");
            }
            else
            {
              strncpy(badgeNumber, DRIVERS.badgeNumber, DRIVERS_BADGENUMBER_LENGTH);
              trim(badgeNumber, DRIVERS_BADGENUMBER_LENGTH);
              strncpy(lastName, DRIVERS.lastName, DRIVERS_LASTNAME_LENGTH);
              trim(lastName, DRIVERS_LASTNAME_LENGTH);
              strncpy(firstName, DRIVERS.firstName, DRIVERS_FIRSTNAME_LENGTH);
              trim(firstName, DRIVERS_FIRSTNAME_LENGTH);
              strcpy(tempString, badgeNumber);
              strcat(tempString, " - ");
              strcat(tempString, lastName);
              strcat(tempString, ", ");
              strcat(tempString, firstName);
              GetYMD(DAILYOPS.pertainsToDate, &year, &month, &day);
            }
            strcat(outputString, tempString);
//
//  From date/time, to date/time
//
            bGotOlder = FALSE;
            reasonIndex = DAILYOPS.DOPS.Absence.reasonIndex;
//
//  Get the older record if this is an unregister
//
            if(DAILYOPS.recordFlags & DAILYOPS_FLAG_ABSENCEUNREGISTER)
            {
              btrieve(B_GETPOSITION, TMS_DAILYOPS, &absRecID, &DAILYOPSKey5, 5);
              DAILYOPSKey0.recordID = DAILYOPS.DAILYOPSrecordID;
              btrieve(B_GETEQUAL, TMS_DAILYOPS, &DAILYOPS, &DAILYOPSKey0, 0);
              reasonIndex = DAILYOPS.DOPS.Absence.reasonIndex;
              bGotOlder = TRUE;
            }
            GetYMD(DAILYOPS.pertainsToDate, &fromYear, &fromMonth, &fromDay);
            GetYMD(DAILYOPS.DOPS.Absence.untilDate, &toYear, &toMonth, &toDay);
            sprintf(tempString, " - %02ld-%s-%04ld to %02ld-%s-%04ld",
                    fromDay, szMonth[fromMonth - 1], fromYear, toDay, szMonth[toMonth - 1], toYear);
            strcat(outputString, tempString);
//
//  Restore the position if necessary
//
            if(bGotOlder)
            {
              DAILYOPS.recordID = absRecID;
              btrieve(B_GETDIRECT, TMS_DAILYOPS, &DAILYOPS, &DAILYOPSKey5, 5);
            }
//
//  Register
//
            if(DAILYOPS.recordFlags & DAILYOPS_FLAG_ABSENCEREGISTER)
            {
              if(reasonIndex == NO_RECORD)
              {
                sprintf(tempString, " - Reason: Assignment cleared manually");
              }
              else
              {
                sprintf(tempString, " - Reason: %s", m_AbsenceReasons[reasonIndex].szText);
              }
              sprintf(szarString, " - Time: %s", chhmm(DAILYOPS.DOPS.Absence.timeLost));
              strcat(tempString, szarString);
              if(!(AuditSelections.flags & AUDITFILTER_FLAGS_ALLREASONS))
              {
                totalTime += DAILYOPS.DOPS.Absence.timeLost;
              }
            }
//
//  Unregister
//
            else
            {
              if(reasonIndex == NO_RECORD)
              {
                sprintf(tempString, " - Unregistered: Assignment cleared manually");
              }
              else
              {
                sprintf(tempString, " - Unregistered: %s", m_AbsenceReasons[reasonIndex].szText);
              }
              sprintf(szarString, " - Time returned: %s", chhmm(DAILYOPS.DOPS.Absence.timeLost));
              strcat(tempString, szarString);
              if(!(AuditSelections.flags & AUDITFILTER_FLAGS_ALLREASONS))
              {
                totalTime -= DAILYOPS.DOPS.Absence.timeLost;
              }
            }
            strcat(outputString, tempString);
            bDoneOne = TRUE;
          }
        }
//
//  Examine by absence date
//
        else
        {
          if(DAILYOPS.pertainsToDate <= AuditSelections.fromDate &&
                DAILYOPS.DOPS.Absence.untilDate >= AuditSelections.fromDate)
          {
//
//  "Registers"
//
            if(DAILYOPS.recordFlags & DAILYOPS_FLAG_ABSENCEREGISTER)
            {
              if(!ANegatedRecord(DAILYOPS.recordID, 5))
              {
//
//  Driver
//
                if(((AuditSelections.flags & AUDITFILTER_FLAGS_ALLOPERATORS) ||
                      AuditSelections.DRIVERSrecordID == DAILYOPS.DRIVERSrecordID) &&
                   ((AuditSelections.flags & AUDITFILTER_FLAGS_ALLREASONS) ||
                      AuditSelections.reasonNumber == DAILYOPS.DOPS.Absence.reasonIndex))
                {
                  DRIVERSKey0.recordID = DAILYOPS.DRIVERSrecordID;
                  rcode2 = btrieve(B_GETEQUAL, TMS_DRIVERS, &DRIVERS, &DRIVERSKey0, 0);
                  if(rcode2 != 0)
                  {
                    strcpy(tempString, "Driver not found");
                  }
                  else
                  {
                    strncpy(badgeNumber, DRIVERS.badgeNumber, DRIVERS_BADGENUMBER_LENGTH);
                    trim(badgeNumber, DRIVERS_BADGENUMBER_LENGTH);
                    strncpy(lastName, DRIVERS.lastName, DRIVERS_LASTNAME_LENGTH);
                    trim(lastName, DRIVERS_LASTNAME_LENGTH);
                    strncpy(firstName, DRIVERS.firstName, DRIVERS_FIRSTNAME_LENGTH);
                    trim(firstName, DRIVERS_FIRSTNAME_LENGTH);
                    strcpy(tempString, badgeNumber);
                    strcat(tempString, " - ");
                    strcat(tempString, lastName);
                    strcat(tempString, ", ");
                    strcat(tempString, firstName);
                    GetYMD(DAILYOPS.pertainsToDate, &year, &month, &day);
                  }
                  strcat(outputString, tempString);
//
//  From date/time, to date/time
//
                  bGotOlder = FALSE;
                  reasonIndex = DAILYOPS.DOPS.Absence.reasonIndex;
                  GetYMD(DAILYOPS.pertainsToDate, &fromYear, &fromMonth, &fromDay);
                  GetYMD(DAILYOPS.DOPS.Absence.untilDate, &toYear, &toMonth, &toDay);
                  sprintf(tempString, " - %02ld-%s-%04ld to %02ld-%s-%04ld",
                          fromDay, szMonth[fromMonth - 1], fromYear, toDay, szMonth[toMonth - 1], toYear);
                  strcat(outputString, tempString);
                  if(reasonIndex == NO_RECORD)
                  {
                    strcpy(tempString, " - Reason: Assignment cleared manually");
                  }
                  else
                  {
                    sprintf(tempString, " - Reason: %s", m_AbsenceReasons[reasonIndex].szText);
                  }
                  strcat(outputString, tempString);
                  bDoneOne = TRUE;
                }
              }
            }
          }
        }
      }
    }
//
//  Open work assignments
//
    if((bDoAll || (AuditSelections.flags & AUDITFILTER_FLAGS_OPENWORK)) && !bDoneOne)
    {
      if((DAILYOPS.recordFlags & DAILYOPS_FLAG_OPENWORKASSIGN) ||
            (DAILYOPS.recordFlags & DAILYOPS_FLAG_OPENWORKCLEAR))
      {
//
//  Driver
//
        DRIVERSKey0.recordID = DAILYOPS.DRIVERSrecordID;
        rcode2 = btrieve(B_GETEQUAL, TMS_DRIVERS, &DRIVERS, &DRIVERSKey0, 0);
        if(rcode2 != 0)
        {
          strcpy(tempString, "Driver not found");
        }
        else
        {
          strncpy(badgeNumber, DRIVERS.badgeNumber, DRIVERS_BADGENUMBER_LENGTH);
          trim(badgeNumber, DRIVERS_BADGENUMBER_LENGTH);
          strncpy(lastName, DRIVERS.lastName, DRIVERS_LASTNAME_LENGTH);
          trim(lastName, DRIVERS_LASTNAME_LENGTH);
          strncpy(firstName, DRIVERS.firstName, DRIVERS_FIRSTNAME_LENGTH);
          trim(firstName, DRIVERS_FIRSTNAME_LENGTH);
          strcpy(tempString, badgeNumber);
          strcat(tempString, " - ");
          strcat(tempString, lastName);
          strcat(tempString, ", ");
          strcat(tempString, firstName);
        }
        strcat(outputString, tempString);
        if(DAILYOPS.recordFlags & DAILYOPS_FLAG_CREWONLY)
        {
          CREWONLYKey0.recordID = DAILYOPS.DOPS.OpenWork.RUNSrecordID;
          btrieve(B_GETEQUAL, TMS_CREWONLY, &CREWONLY, &CREWONLYKey0, 0);
          SERVICESKey0.recordID = CREWONLY.SERVICESrecordID;
          runNumber = CREWONLY.runNumber;
        }
        else
        {
          RUNSKey0.recordID = DAILYOPS.DOPS.OpenWork.RUNSrecordID;
          btrieve(B_GETEQUAL, TMS_RUNS, &RUNS, &RUNSKey0, 0);
          SERVICESKey0.recordID = RUNS.SERVICESrecordID;
          runNumber = RUNS.runNumber;
        }
        btrieve(B_GETEQUAL, TMS_SERVICES, &SERVICES, &SERVICESKey0, 0);
        strncpy(szarString, SERVICES.name, SERVICES_NAME_LENGTH);
        trim(szarString, SERVICES_NAME_LENGTH);
        if(DAILYOPS.recordFlags & DAILYOPS_FLAG_OPENWORKASSIGN)
        {
          sprintf(tempString, " assigned to %s run number %ld", szarString, runNumber);
        }
        else if(DAILYOPS.recordFlags & DAILYOPS_FLAG_OPENWORKCLEAR)
        {
          sprintf(tempString, " cleared from %s run number %ld", szarString, runNumber);
        }
        strcat(outputString, tempString);
        bDoneOne = TRUE;
      }
//
//  Run split
//
      else if((DAILYOPS.recordFlags & DAILYOPS_FLAG_RUNSPLIT) ||
            (DAILYOPS.recordFlags & DAILYOPS_FLAG_RUNSPLITCREWONLY) ||
            (DAILYOPS.recordFlags & DAILYOPS_FLAG_UNRUNSPLIT))
      {
//
//  Run
//
        RUNSKey0.recordID = DAILYOPS.DOPS.OpenWork.RUNSrecordID;
        btrieve(B_GETEQUAL, TMS_RUNS, &RUNS, &RUNSKey0, 0);
        if(DAILYOPS.recordFlags & DAILYOPS_FLAG_RUNSPLIT)
        {
          sprintf(tempString, " Run %ld split at ", RUNS.runNumber);
          NODESKey0.recordID = DAILYOPS.DOPS.OpenWork.splitStartNODESrecordID;
          btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
          strncpy(szarString, NODES.abbrName, NODES_ABBRNAME_LENGTH);
          trim(szarString, NODES_ABBRNAME_LENGTH);
          strcat(tempString, szarString);
          strcat(tempString, " at ");
          strcat(tempString, Tchar(DAILYOPS.DOPS.OpenWork.splitStartTime));
          strcat(tempString, " and ");
          NODESKey0.recordID = DAILYOPS.DOPS.OpenWork.splitEndNODESrecordID;
          btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
          strncpy(szarString, NODES.abbrName, NODES_ABBRNAME_LENGTH);
          trim(szarString, NODES_ABBRNAME_LENGTH);
          strcat(tempString, szarString);
          strcat(tempString, " at ");
          strcat(tempString, Tchar(DAILYOPS.DOPS.OpenWork.splitEndTime));
        }
        else if(DAILYOPS.recordFlags & DAILYOPS_FLAG_RUNSPLITCREWONLY)
        {
          sprintf(tempString, " Crew-Only Run %ld split at ", CREWONLY.runNumber);
          NODESKey0.recordID = DAILYOPS.DOPS.OpenWork.splitStartNODESrecordID;
          btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
          strncpy(szarString, NODES.abbrName, NODES_ABBRNAME_LENGTH);
          trim(szarString, NODES_ABBRNAME_LENGTH);
          strcat(tempString, szarString);
          strcat(tempString, " at ");
          strcat(tempString, Tchar(DAILYOPS.DOPS.OpenWork.splitStartTime));
          strcat(tempString, " and ");
          NODESKey0.recordID = DAILYOPS.DOPS.OpenWork.splitEndNODESrecordID;
          btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
          strncpy(szarString, NODES.abbrName, NODES_ABBRNAME_LENGTH);
          trim(szarString, NODES_ABBRNAME_LENGTH);
          strcat(tempString, szarString);
          strcat(tempString, " at ");
          strcat(tempString, Tchar(DAILYOPS.DOPS.OpenWork.splitEndTime));
        }
        else
        {
          sprintf(tempString, " Run %ld was restored (unsplit)", RUNS.runNumber);
        }
        strcat(outputString, tempString);
        bDoneOne = TRUE;
      }
    }
//
//  Write it out
//
    if(bDoneOne)
    {
      strcat(outputString, "\r\n");
      WriteFile(hOutputFile, (LPCVOID *)outputString, strlen(outputString), &dwBytesWritten, NULL);
    }
//
//  Get the next entry in DAILYOPS
//
    rcode2 = btrieve(B_GETNEXT, TMS_DAILYOPS, &DAILYOPS, &DAILYOPSKey5, 5);
  }
//
//  Do a summary line if a specific absence type was asked for
//
    if((AuditSelections.flags & AUDITFILTER_FLAGS_ABSENCE) &&
          !(AuditSelections.flags & AUDITFILTER_FLAGS_ALLOPERATORS) &&
          !(AuditSelections.flags & AUDITFILTER_FLAGS_ALLREASONS))
    {
      sprintf(outputString, "Operator absence:\t\t\t\tTotal time: %s\r\n", chhmm(totalTime));
      WriteFile(hOutputFile, (LPCVOID *)outputString, strlen(outputString), &dwBytesWritten, NULL);
    }

//
//  Free allocated memory
//
  deallocate:
    StatusBarEnd();
    if(hOutputFile != NULL)
    {
      CloseHandle(hOutputFile);
    }
    SetCursor(hCursorArrow);

  return(bKeepGoing);
}

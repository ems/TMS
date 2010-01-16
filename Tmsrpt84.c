// 
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2006 Schedule Masters, Inc.
//  All rights reserved.
//
//
//  TMS Unload to the Strategic Mapping AVL System
//

#include "TMSHeader.h"
#include "cistms.h"
#include "limits.h"
#include <math.h>

int  distanceMeasureSave;

#define OUTPUT_FOLDER_NAME "Strategic Mapping Interface"

BOOL FAR TMSRPT84(TMSRPTPassedDataDef *pPassedData)
{
  GetConnectionTimeDef GCTData;
  GenerateTripDef  GTResults;
  BLOCKSDef       *pTRIPSChunk;
  double  prevLat, prevLon;
  HFILE   hfOutputFile;
  HFILE   hfErrorLog;
  float   distanceToHere;
  float   tripDistances[100];
  float   distance;
  struct  tm today;
  time_t  now;
  struct  tm tmED, tmUD;
  time_t  EDmktime;
  time_t  UDmktime;
  BOOL  bGotError;
  BOOL  bFinishedOK;
  BOOL  bFirst;
  BOOL  bServiceFromRoster;
  long  year, month, day;
  long  serviceRecordID;
  long  previousPattern;
  long  timeAtStop;
  long  assignedToNODESrecordID;
  long  blockNumber;
  long  fromNODESrecordID;
  long  toNODESrecordID;
  long  toTime;
  long  fromTime;
  long  dhd;
  long  date;
  long  lastDAILYOPSrecordID;
  char  outputString[1024];
  char  outputStringSave[1024];
  char *ptr;
  char *pszReportName;
  char  outputFileName[64];
  int   dayOfWeek;
  int   nI;
  int   rcode2;
  int   tripIndex;
  int   seq;

  bFinishedOK = FALSE;
//
//  Maintain the distance measure
//
  distanceMeasureSave = distanceMeasure;
  distanceMeasure = PREFERENCES_MILES;
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
        goto done;
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
//  Create the sub-folder for the output files and chdir into it
//
  nI = (_mkdir(OUTPUT_FOLDER_NAME) == 0) ? TEXT_333 : TEXT_334;
  LoadString(hInst, nI, szFormatString, SZFORMATSTRING_LENGTH);
  sprintf(tempString, szFormatString, OUTPUT_FOLDER_NAME);
  MessageBeep(MB_ICONINFORMATION);
  MessageBox(NULL, tempString, TMS, MB_OK);
  chdir(OUTPUT_FOLDER_NAME);
//
//  Open the error log and fire up the status bar
//
  hfErrorLog = _lcreat("error.log", 0);
  if(hfErrorLog == HFILE_ERROR)
  {
    TMSError((HWND)NULL, MB_ICONSTOP, ERROR_226, (HANDLE)NULL);
    goto done;
  }
  bGotError = FALSE;
  pPassedData->nReportNumber = 83;
  for(nI = 0; nI < m_LastReport; nI++)
  {
    if(TMSRPT[nI].originalReportNumber == pPassedData->nReportNumber)
    {
      pszReportName = TMSRPT[nI].szReportName;
      StatusBarStart(hWndMain, TMSRPT[nI].szReportName);
      break;
    }
  }
//
//  ====================
//  Date File - Date.csv
//  ====================
  if(StatusBarAbort())
  {
    goto done;
  }
  strcpy(outputFileName, "Date.csv");
  hfOutputFile = _lcreat(outputFileName, 0);
  if(hfOutputFile == HFILE_ERROR)
  {
    LoadString(hInst, ERROR_320, szFormatString, SZFORMATSTRING_LENGTH);
    sprintf(szarString, szFormatString, outputFileName);
    MessageBeep(MB_ICONSTOP);
    MessageBox(NULL, szarString, TMS, MB_OK | MB_ICONSTOP);
    goto done;
  }
  StatusBarText(outputFileName);
//
//  Write out the column headers
//
  strcpy(outputString, "\"UnloadDate\",\"EffectiveDate\",\"UntilDate\"\r\n");
  _lwrite(hfOutputFile, outputString, strlen(outputString));
//
//  Get today's date and time and write it/them out
//
  time(&now);
  today = *localtime(&now);
  sprintf(outputString, "\"%4d%02d%02d %02d%02d%02d\",\"%ld\",\"%ld\"\r\n",
        today.tm_year + 1900, today.tm_mon + 1, today.tm_mday,
        today.tm_hour, today.tm_min, today.tm_sec, effectiveDate, untilDate);
  _lwrite(hfOutputFile, outputString, strlen(outputString));
//
//  Close the file
//
  _lclose(hfOutputFile);
//
//  ==========================
//  Service File - Service.csv
//  ==========================
//
  if(StatusBarAbort())
  {
    goto done;
  }
  strcpy(outputFileName, "Service.csv");
  hfOutputFile = _lcreat(outputFileName, 0);
  if(hfOutputFile == HFILE_ERROR)
  {
    LoadString(hInst, ERROR_320, szFormatString, SZFORMATSTRING_LENGTH);
    sprintf(szarString, szFormatString, outputFileName);
    MessageBeep(MB_ICONSTOP);
    MessageBox(NULL, szarString, TMS, MB_OK | MB_ICONSTOP);
    goto done;
  }
  StatusBarText(outputFileName);
//
//  Write out the column headers
//
  strcpy(outputString, "\"ServiceCode\",\"ServiceName\"\r\n");
  _lwrite(hfOutputFile, outputString, strlen(outputString));
//
//  Cycle through the services
//
  rcode2 = btrieve(B_GETFIRST, TMS_SERVICES, &SERVICES, &SERVICESKey1, 1);
  while(rcode2 == 0)
  {
    if(StatusBarAbort())
    {
      goto done;
    }
    strncpy(tempString, SERVICES.name, SERVICES_NAME_LENGTH);
    trim(tempString, SERVICES_NAME_LENGTH);
//
//  ServiceCode (SERVICES.recordID) 
//  ServiceName (SERVICES.name)
//
    sprintf(outputString, "%ld,\"%s\"\r\n", SERVICES.recordID, tempString);
    _lwrite(hfOutputFile, outputString, strlen(outputString));
    rcode2 = btrieve(B_GETNEXT, TMS_SERVICES, &SERVICES, &SERVICESKey1, 1);
  }
//
//  Close the file
//
  _lclose(hfOutputFile);
//
//  ====================================
//  Service Dates File - ServiceDate.csv
//  ====================================
//
  if(StatusBarAbort())
  {
    goto done;
  }
  strcpy(outputFileName, "ServiceDate.csv");
  hfOutputFile = _lcreat(outputFileName, 0);
  if(hfOutputFile == HFILE_ERROR)
  {
    LoadString(hInst, ERROR_320, szFormatString, SZFORMATSTRING_LENGTH);
    sprintf(szarString, szFormatString, outputFileName);
    MessageBeep(MB_ICONSTOP);
    MessageBox(NULL, szarString, TMS, MB_OK | MB_ICONSTOP);
    goto done;
  }
  StatusBarText(outputFileName);
//
//  Write out the column headers
//
  strcpy(outputString, "\"Date\",\"ServiceCode\"\r\n");
  _lwrite(hfOutputFile, outputString, strlen(outputString));
//
//  Dump out the service day based on calendar days
//
//  Effective date
//
  GetYMD(effectiveDate, &year, &month, &day);
  memset(&tmED, 0x00, sizeof(tmED));
  tmED.tm_mday = day;
  tmED.tm_mon = month - 1;
  tmED.tm_year = year - 1900;
//
//  Until date
//
  GetYMD(untilDate, &year, &month, &day);
  memset(&tmUD, 0x00, sizeof(tmUD));
  tmUD.tm_mday = day;
  tmUD.tm_mon = month - 1;
  tmUD.tm_year = year - 1900;
//
//  Start at the effective date and go to the until date
//
  EDmktime = mktime(&tmED);
  if(EDmktime == (time_t)(-1))
  {
    LoadString(hInst, ERROR_328, szFormatString, SZFORMATSTRING_LENGTH);
    sprintf(outputString, szFormatString, 1);
    _lwrite(hfErrorLog, outputString, strlen(outputString));
    bGotError = TRUE;
  }
  else
  {
    UDmktime = mktime(&tmUD);
    while(EDmktime <= UDmktime)
    {
      if(StatusBarAbort())
      {
        goto done;
      }
//
//  Date
//
      date = ((tmED.tm_year + 1900) * 10000) + ((tmED.tm_mon + 1) * 100) + tmED.tm_mday;
      sprintf(tempString, "%ld,", date);
      strcpy(outputString, tempString);
//
//  Look to see if there's been a service/date override in DailyOps
//
      rcode2 = btrieve(B_GETLAST, TMS_DAILYOPS, &DAILYOPS, &DAILYOPSKey0, 0);
      lastDAILYOPSrecordID = (rcode2 == 0 ? DAILYOPS.recordID : 0);
      DAILYOPSKey1.recordTypeFlag = (char)DAILYOPS_FLAG_DATE;
      DAILYOPSKey1.pertainsToDate = date;
      DAILYOPSKey1.pertainsToTime = lastDAILYOPSrecordID + 1;
      DAILYOPSKey1.recordFlags = 0;
      bServiceFromRoster = TRUE;
      rcode2 = btrieve(B_GETLESSTHAN, TMS_DAILYOPS, &DAILYOPS, &DAILYOPSKey1, 1);
      while(rcode2 == 0 &&
            (DAILYOPS.recordTypeFlag & DAILYOPS_FLAG_DATE) &&
             DAILYOPS.pertainsToDate == date)
      {
        if(DAILYOPS.recordFlags & DAILYOPS_FLAG_DATESET)
        {
          bServiceFromRoster = ANegatedRecord(DAILYOPS.recordID, 1);
          if(!bServiceFromRoster)
          {
            break;
          }
        }
        rcode2 = btrieve(B_GETPREVIOUS, TMS_DAILYOPS, &DAILYOPS, &DAILYOPSKey1, 1);
      }
//
//  ServiceCode (SERVICES.number)
//
      if(bServiceFromRoster)
      {
        dayOfWeek = (tmED.tm_wday == 0 ? 6 : tmED.tm_wday - 1);
        SERVICESKey0.recordID = ROSTERPARMS.serviceDays[dayOfWeek];
        btrieve(B_GETEQUAL, TMS_SERVICES, &SERVICES, &SERVICESKey0, 0);
        sprintf(tempString, "%ld\r\n", SERVICES.number);
      }
      else
      {
        sprintf(tempString, "%ld\r\n", DAILYOPS.DOPS.Date.SERVICESrecordID);
      }
      strcat(outputString, tempString);
//
//  Write out the record
//
      _lwrite(hfOutputFile, outputString, strlen(outputString));
//
//  Figure out the next day
//
      tmED.tm_mday++;
      EDmktime = mktime(&tmED);
      if(EDmktime == (time_t)(-1))
      {
        LoadString(hInst, ERROR_328, szFormatString, SZFORMATSTRING_LENGTH);
        sprintf(outputString, szFormatString, 1);
        _lwrite(hfErrorLog, outputString, strlen(outputString));
        bGotError = TRUE;
      }
    }
  }
//
//  Close the file
//
  _lclose(hfOutputFile);
//
//  ===============================
//  Directions File - Direction.csv
//  ===============================
//
  if(StatusBarAbort())
  {
    goto done;
  }
  strcpy(outputFileName, "Direction.csv");
  hfOutputFile = _lcreat(outputFileName, 0);
  if(hfOutputFile == HFILE_ERROR)
  {
    LoadString(hInst, ERROR_320, szFormatString, SZFORMATSTRING_LENGTH);
    sprintf(szarString, szFormatString, outputFileName);
    MessageBeep(MB_ICONSTOP);
    MessageBox(NULL, szarString, TMS, MB_OK | MB_ICONSTOP);
    goto done;
  }
  StatusBarText(outputFileName);
//
//  Write out the column headers
//
  strcpy(outputString, "\"DirectionCode\",\"DirectionName\"\r\n");
  _lwrite(hfOutputFile, outputString, strlen(outputString));
//
//  Cycle through the directions
//
  rcode2 = btrieve(B_GETFIRST, TMS_DIRECTIONS, &DIRECTIONS, &DIRECTIONSKey1, 1);
  while(rcode2 == 0)
  {
    if(StatusBarAbort())
    {
      goto done;
    }
    strncpy(tempString, DIRECTIONS.longName, DIRECTIONS_LONGNAME_LENGTH);
    trim(tempString, DIRECTIONS_LONGNAME_LENGTH);
//
//  DirectionCode (DIRECTIONS.recordID) 
//  DirectionName (DIRECTIONS.name)
//
    sprintf(outputString, "%ld,\"%s\"\r\n", DIRECTIONS.recordID, tempString);
    _lwrite(hfOutputFile, outputString, strlen(outputString));
    rcode2 = btrieve(B_GETNEXT, TMS_DIRECTIONS, &DIRECTIONS, &DIRECTIONSKey1, 1);
  }
//
//  Close the file
//
  _lclose(hfOutputFile);
//
//  =======================
//  Routes File - Route.csv
//  =======================
//
  if(StatusBarAbort())
  {
    goto done;
  }
  strcpy(outputFileName, "Route.csv");
  hfOutputFile = _lcreat(outputFileName, 0);
  if(hfOutputFile == HFILE_ERROR)
  {
    LoadString(hInst, ERROR_320, szFormatString, SZFORMATSTRING_LENGTH);
    sprintf(szarString, szFormatString, outputFileName);
    MessageBeep(MB_ICONSTOP);
    MessageBox(NULL, szarString, TMS, MB_OK | MB_ICONSTOP);
    goto done;
  }
  StatusBarText(outputFileName);
//
//  Write out the column headers
//
  strcpy(outputString, "\"RouteCode\",\"RouteNumber\",\"RouteName\",\"Color\",\"OBDirectionCode\",\"IBDirectionCode\"\r\n");
  _lwrite(hfOutputFile, outputString, strlen(outputString));
//
//  Traverse the Routes Table
//
  rcode2 = btrieve(B_GETFIRST, TMS_ROUTES, &ROUTES, &ROUTESKey1, 1);
  while(rcode2 == 0)
  {
    if(StatusBarAbort())
    {
      goto done;
    }
//
//  RouteCode (ROUTES.recordID)
//
    sprintf(outputString, "%ld,", ROUTES.recordID);
//
//  RouteNumber (ROUTES.number)
//
    strncpy(tempString, ROUTES.number, ROUTES_NUMBER_LENGTH);
    trim(tempString, ROUTES_NUMBER_LENGTH);
    ptr = tempString;
    while(ptr && *ptr == ' ')
    {
      ptr++;
    }
    sprintf(szarString, "\"%s\",", ptr);
    strcat(outputString, szarString);
//
//  RouteName (ROUTES.name)
//
    strncpy(tempString, ROUTES.name, ROUTES_NAME_LENGTH);
    trim(tempString, ROUTES_NAME_LENGTH);
    sprintf(szarString, "\"%s\",", tempString);
    strcat(outputString, szarString);
//
//  Color (unused)
//  OBDirectionCode (ROUTES.DIRECTIONSrecordID[0])
//  IBDirectionCode (ROUTES.DIRECTIONSrecordID[1])
//
    sprintf(szarString, ",\"%ld\",\"%ld\"\r\n", ROUTES.DIRECTIONSrecordID[0], ROUTES.DIRECTIONSrecordID[1]);
    strcat(outputString, szarString);
//
//  Write out the record
//
    _lwrite(hfOutputFile, outputString, strlen(outputString));
//
//  Get the next route
//
    rcode2 = btrieve(B_GETNEXT, TMS_ROUTES, &ROUTES, &ROUTESKey1, 1);
  }
//
//  Close the file
//
  _lclose(hfOutputFile);
//
//  =========================
//  Bus Stops File - Stop.csv
//  =========================
//
  if(StatusBarAbort())
  {
    goto done;
  }
  strcpy(outputFileName, "Stop.csv");
  hfOutputFile = _lcreat(outputFileName, 0);
  if(hfOutputFile == HFILE_ERROR)
  {
    LoadString(hInst, ERROR_320, szFormatString, SZFORMATSTRING_LENGTH);
    sprintf(szarString, szFormatString, outputFileName);
    MessageBeep(MB_ICONSTOP);
    MessageBox(NULL, szarString, TMS, MB_OK | MB_ICONSTOP);
    goto done;
  }
  StatusBarText(outputFileName);
//
//  Write out the column headers
//
  strcpy(outputString, "\"StopCode\",\"ShortName\",\"Name\",\"AudioText\",\"Latitude\",\"Longitude\",\"IsTimePoint\"\r\n");
  _lwrite(hfOutputFile, outputString, strlen(outputString));
//
//  Cycle through the Nodes Table
//
  rcode2 = btrieve(B_GETFIRST, TMS_NODES, &NODES, &NODESKey0, 0);
  while(rcode2 == 0)
  {
//
//  StopCode (NODES.recordID)
//
    sprintf(outputString, "%ld,", NODES.recordID);
//
//  ShortName (NODES.longName)
//
    strncpy(tempString, NODES.longName, NODES_LONGNAME_LENGTH);
    trim(tempString, NODES_LONGNAME_LENGTH);
    sprintf(szarString, "\"%s\",", tempString);
    strcat(outputString, szarString);
//
//  Name (NODES.description)
//
    strncpy(tempString, NODES.intersection, NODES_DESCRIPTION_LENGTH);
    trim(tempString, NODES_DESCRIPTION_LENGTH);
    ptr = strstr(tempString, "&&");  // Get rid of the MapInfo convention
    if(ptr)
    {
      *ptr = 'a';
      *(ptr + 1) = 't';
    }
    sprintf(szarString, "\"%s\",", tempString);
    strcat(outputString, szarString);
//
//  AudioText (unused)
// 
    strcat(outputString, ",");
//
//  Latitude (NODES.latitude)
//
    sprintf(tempString, "%12.8f", NODES.latitude);
    ptr = tempString;
    while(ptr && *ptr == ' ')
    {
      ptr++;
    }
    sprintf(szarString, "\"%s\",", ptr);
    strcat(outputString, szarString);
//
//  Longitude (NODES.longitude)
//
    sprintf(tempString, "%12.8f", NODES.longitude);
    ptr = tempString;
    while(ptr && *ptr == ' ')
    {
      ptr++;
    }
    sprintf(szarString, "\"%s\",", ptr);
    strcat(outputString, szarString);
//
//  IsTimePoint (NODES.flags & NODES_FLAG_STOP)
//

    sprintf(szarString, "%d\r\n", ((NODES.flags & NODES_FLAG_STOP) ? 0 : 1));
    strcat(outputString, szarString);
//
//  Write it out and get the next node
//
    _lwrite(hfOutputFile, outputString, strlen(outputString));
    rcode2 = btrieve(B_GETNEXT, TMS_NODES, &NODES, &NODESKey0, 0);
  }
//
//  Close the file
//
  _lclose(hfOutputFile);
//
//  ====================================
//  Pattern Names File - PatternName.csv
//  ====================================
//
  if(StatusBarAbort())
  {
    goto done;
  }
  strcpy(outputFileName, "PatternName.csv");
  hfOutputFile = _lcreat(outputFileName, 0);
  if(hfOutputFile == HFILE_ERROR)
  {
    LoadString(hInst, ERROR_320, szFormatString, SZFORMATSTRING_LENGTH);
    sprintf(szarString, szFormatString, outputFileName);
    MessageBeep(MB_ICONSTOP);
    MessageBox(NULL, szarString, TMS, MB_OK | MB_ICONSTOP);
    goto done;
  }
  StatusBarText(outputFileName);
//
//  Write out the column headers
//
  strcpy(outputString, "\"PatternCode\",\"PatternName\"\r\n");
  _lwrite(hfOutputFile, outputString, strlen(outputString));
//
//  Cycle through the pattern names
//
  rcode2 = btrieve(B_GETFIRST, TMS_PATTERNNAMES, &PATTERNNAMES, &PATTERNNAMESKey1, 1);
  while(rcode2 == 0)
  {
    if(StatusBarAbort())
    {
      goto done;
    }
    strncpy(tempString, PATTERNNAMES.name, PATTERNNAMES_NAME_LENGTH);
    trim(tempString, PATTERNNAMES_NAME_LENGTH);
//
//  PatternCode (PATTERNNAMES.recordID) 
//  PatternName (PATTERNNAMES.name)
//
    sprintf(outputString, "%ld,\"%s\"\r\n", PATTERNNAMES.recordID, tempString);
    _lwrite(hfOutputFile, outputString, strlen(outputString));
    rcode2 = btrieve(B_GETNEXT, TMS_PATTERNNAMES, &PATTERNNAMES, &PATTERNNAMESKey1, 1);
  }
//
//  Close the file
//
  _lclose(hfOutputFile);
//
//  ===========================
//  Patterns File - Pattern.csv
//  ===========================
//
  if(StatusBarAbort())
  {
    goto done;
  }
  strcpy(outputFileName, "Pattern.csv");
  hfOutputFile = _lcreat(outputFileName, 0);
  if(hfOutputFile == HFILE_ERROR)
  {
    LoadString(hInst, ERROR_320, szFormatString, SZFORMATSTRING_LENGTH);
    sprintf(szarString, szFormatString, outputFileName);
    MessageBeep(MB_ICONSTOP);
    MessageBox(NULL, szarString, TMS, MB_OK | MB_ICONSTOP);
    goto done;
  }
  StatusBarText(outputFileName);
//
//  Write out the column headers
//
  strcpy(outputString, "\"RouteCode\",\"ServiceCode\",\"PatternCode\",\"DirectionCode\",\"StopCode\",\"PatternSeq\"\r\n");
  _lwrite(hfOutputFile, outputString, strlen(outputString));
//
//  Traverse the Routes Table
//
    rcode2 = btrieve(B_GETFIRST, TMS_ROUTES, &ROUTES, &ROUTESKey1, 1);
    while(rcode2 == 0)
    {
      if(StatusBarAbort())
      {
        goto done;
      }
      for(nI = 0; nI < 2; nI++)
      {
        if(ROUTES.DIRECTIONSrecordID[nI] == NO_RECORD)
        {
          continue;
        }
//
//  Go through all the services
//
        rcode2 = btrieve(B_GETFIRST, TMS_SERVICES, &SERVICES, &SERVICESKey0, 0);
        while(rcode2 == 0)
        {
//
//  RouteCode (ROUTES.recordID)
//  ServiceCode (SERVICES.recordID)
//
          sprintf(outputStringSave, "%ld,%ld,", ROUTES.recordID,SERVICES.recordID);
//
//  Go through all the patterns
//
          PATTERNSKey2.ROUTESrecordID = ROUTES.recordID;
          PATTERNSKey2.SERVICESrecordID = SERVICES.recordID;
          PATTERNSKey2.directionIndex = nI;
          PATTERNSKey2.PATTERNNAMESrecordID = NO_RECORD;
          PATTERNSKey2.nodeSequence = NO_RECORD;
          rcode2 = btrieve(B_GETGREATER, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
          previousPattern = NO_RECORD;
          while(rcode2 == 0 &&
                PATTERNS.ROUTESrecordID == ROUTES.recordID &&
                PATTERNS.SERVICESrecordID == SERVICES.recordID && 
                PATTERNS.directionIndex == nI)
          {
//
//  PatternCode (PATTERNS.PATTERNNAMESrecordID)
//  DirectionCode (ROUTES.DIRECTIONSrecordID[nI])
//
            strcpy(outputString, outputStringSave);
            sprintf(tempString, "%ld,%ld,", PATTERNS.PATTERNNAMESrecordID, ROUTES.DIRECTIONSrecordID[nI]);
            strcat(outputString, tempString);
            if(previousPattern != PATTERNS.PATTERNNAMESrecordID)
            {
              previousPattern = PATTERNS.PATTERNNAMESrecordID;
              seq = 1;
            }
//
//  StopCode (NODES.recordID)
//  PatternSeq (seq)
//
            NODESKey0.recordID = PATTERNS.NODESrecordID;
            rcode2 = btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);  // just in case some bad data slipped by
            if(rcode2 != 0)
            {
              sprintf(tempString, "0,%d\r\n", seq);
            }
            else
            {
              sprintf(tempString, "%ld,%d\r\n", NODES.recordID, seq);
            }
            strcat(outputString, tempString);
            seq++;
//
//  Write it out
//
            _lwrite(hfOutputFile, outputString, strlen(outputString));
//
//  Get the next node on the pattern
//
            rcode2 = btrieve(B_GETNEXT, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
//
//  On a pattern, direction, service, route change, write a termination record
//
            if(rcode2 != 0 ||
                  PATTERNS.ROUTESrecordID != ROUTES.recordID ||
                  PATTERNS.SERVICESrecordID != SERVICES.recordID || 
                  PATTERNS.directionIndex != nI ||
                  PATTERNS.PATTERNNAMESrecordID != previousPattern)
            {
              strcpy(outputString, outputStringSave);
              sprintf(tempString, "%ld,%d,-1,%d\r\n", previousPattern, nI, seq);
              strcat(outputString, tempString);
              _lwrite(hfOutputFile, outputString, strlen(outputString));
            }
          } // while on patterns
//
//  Get the next service
//
          rcode2 = btrieve(B_GETNEXT, TMS_SERVICES, &SERVICES, &SERVICESKey0, 0);
        }
      } // nI  (next direction)
//
//  Get the next route
//
      rcode2 = btrieve(B_GETNEXT, TMS_ROUTES, &ROUTES, &ROUTESKey1, 1);
    }
//
//  Close the file
//
  _lclose(hfOutputFile);
//
//  =====================
//  Trips File - trip.csv
//  =====================
//
  if(StatusBarAbort())
  {
    goto done;
  }
  strcpy(outputFileName, "Trip.csv");
  hfOutputFile = _lcreat(outputFileName, 0);
  if(hfOutputFile == HFILE_ERROR)
  {
    LoadString(hInst, ERROR_320, szFormatString, SZFORMATSTRING_LENGTH);
    sprintf(szarString, szFormatString, outputFileName);
    MessageBeep(MB_ICONSTOP);
    MessageBox(NULL, szarString, TMS, MB_OK | MB_ICONSTOP);
    goto done;
  }
  StatusBarText(outputFileName);
//
//  Write out the column headers
//
  strcpy(outputString, "\"RouteCode\",\"ServiceCode\",\"PatternCode\",\"Direction\",\"TripCode\",\"StopTime\",\"TripSeq\"\r\n");
  _lwrite(hfOutputFile, outputString, strlen(outputString));
//
//  Traverse the Routes Table
//
  rcode2 = btrieve(B_GETFIRST, TMS_ROUTES, &ROUTES, &ROUTESKey1, 1);
  while(rcode2 == 0)
  {
    if(StatusBarAbort())
    {
      goto done;
    }
//
//  Go through the services
//
    rcode2 = btrieve(B_GETFIRST, TMS_SERVICES, &SERVICES, &SERVICESKey1, 1);
    while(rcode2 == 0)
    {
      if(StatusBarAbort())
      {
        goto done;
      }
//
//  Loop through the directions
//
      for(nI = 0; nI < 2; nI++)
      {
        if(ROUTES.DIRECTIONSrecordID[nI] == NO_RECORD)
        {
          continue;
        }
//
//  Get the trips for this route/ser/dir
//
        TRIPSKey1.ROUTESrecordID = ROUTES.recordID;
        TRIPSKey1.SERVICESrecordID = SERVICES.recordID;
        TRIPSKey1.directionIndex = nI;
        TRIPSKey1.tripSequence = NO_RECORD;
        rcode2 = btrieve(B_GETGREATER, TMS_TRIPS, &TRIPS, &TRIPSKey1, 1);
        while(rcode2 == 0 &&
              TRIPS.ROUTESrecordID == ROUTES.recordID &&
              TRIPS.SERVICESrecordID == SERVICES.recordID &&
              TRIPS.directionIndex == nI)
        {
//
//  RouteCode (ROUTES.recordID)
//  ServiceCode (SERVICES.recordID)
//  PatternCode (TRIPS.PATTERNNAMESrecordID)
//  DirectionCode (ROUTES.DIRECTIONSrecordID[nI])
//  TripCode (TRIPS.recordID)
//
          sprintf(outputStringSave, "%ld,%ld,%ld,%ld,%ld",
                ROUTES.recordID, SERVICES.recordID, TRIPS.PATTERNNAMESrecordID,
                ROUTES.DIRECTIONSrecordID[nI], TRIPS.recordID);
          seq = 1;
//
//  Generate the trip
//           
          GenerateTrip(TRIPS.ROUTESrecordID, TRIPS.SERVICESrecordID,
                TRIPS.directionIndex, TRIPS.PATTERNNAMESrecordID,
                TRIPS.timeAtMLP, GENERATETRIP_FLAG_DISPLAYERRORS, &GTResults);
//
//  Go through the pattern twice
//
//  Pass 1 - Determine distances at each timepoint
//
          if(GTResults.tripDistance != 0.0)
          {
            PATTERNSKey2.ROUTESrecordID = ROUTES.recordID;
            PATTERNSKey2.SERVICESrecordID = SERVICES.recordID;
            PATTERNSKey2.directionIndex = nI;
            PATTERNSKey2.PATTERNNAMESrecordID = TRIPS.PATTERNNAMESrecordID;
            PATTERNSKey2.nodeSequence = NO_RECORD;
            rcode2 = btrieve(B_GETGREATER, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
            tripIndex = 0;
            distanceToHere = 0.0;
            while(rcode2 == 0 &&
                  PATTERNS.ROUTESrecordID == ROUTES.recordID &&
                  PATTERNS.SERVICESrecordID == SERVICES.recordID &&
                  PATTERNS.directionIndex == nI &&
                  PATTERNS.PATTERNNAMESrecordID == TRIPS.PATTERNNAMESrecordID)
            {
              NODESKey0.recordID = PATTERNS.NODESrecordID;
              rcode2 = btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
              if(PATTERNS.flags & PATTERNS_FLAG_BUSSTOP)
              {
                distanceToHere += (float)GreatCircleDistance(prevLon, prevLat, NODES.longitude, NODES.latitude);
              }
              else
              {
                tripDistances[tripIndex] = distanceToHere;
                tripIndex++;
              }
              prevLat = NODES.latitude;
              prevLon = NODES.longitude;
              rcode2 = btrieve(B_GETNEXT, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
            }
          }
//
//  Pass 2 - Determine time interpolations and output the records
//
          PATTERNSKey2.ROUTESrecordID = ROUTES.recordID;
          PATTERNSKey2.SERVICESrecordID = SERVICES.recordID;
          PATTERNSKey2.directionIndex = nI;
          PATTERNSKey2.PATTERNNAMESrecordID = TRIPS.PATTERNNAMESrecordID;
          PATTERNSKey2.nodeSequence = NO_RECORD;
          rcode2 = btrieve(B_GETGREATER, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
          tripIndex = 0;
          seq = 1;
          distanceToHere = 0.0;
          while(rcode2 == 0 &&
                PATTERNS.ROUTESrecordID == ROUTES.recordID &&
                PATTERNS.SERVICESrecordID == SERVICES.recordID &&
                PATTERNS.directionIndex == nI &&
                PATTERNS.PATTERNNAMESrecordID == TRIPS.PATTERNNAMESrecordID)
          {
            NODESKey0.recordID = PATTERNS.NODESrecordID;
            rcode2 = btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
            if(PATTERNS.flags & PATTERNS_FLAG_BUSSTOP)
            {
              if(GTResults.tripDistance == 0)
              {
                timeAtStop = GTResults.tripTimes[tripIndex - 1];
              }
              else
              {
                distanceToHere += (float)GreatCircleDistance(prevLon, prevLat, NODES.longitude, NODES.latitude);
                timeAtStop = (long)((GTResults.tripTimes[tripIndex] - GTResults.tripTimes[tripIndex - 1]) *
                      (distanceToHere / (tripDistances[tripIndex] - tripDistances[tripIndex - 1])));
                timeAtStop += (GTResults.tripTimes[tripIndex - 1]);
              }
            }
            else
            {
              timeAtStop = GTResults.tripTimes[tripIndex];
              distanceToHere = 0.0;
              tripIndex++;
            }
//
//  StopTime (Time at the stop, in seconds)
//  PatternSeq (seq)
//
            strcpy(outputString, outputStringSave);
            sprintf(szarString, ",%ld,%d\r\n", timeAtStop, seq);
            strcat(outputString, szarString);
            _lwrite(hfOutputFile, outputString, strlen(outputString));
            seq++;
//
//  Get the next pattern node
//
            prevLat = NODES.latitude;
            prevLon = NODES.longitude;
            rcode2 = btrieve(B_GETNEXT, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
//
//  On a pattern, direction, service, route change, write a termination record
//  terminate the record and write it out.
//
            if(rcode2 != 0 ||
                  PATTERNS.ROUTESrecordID != ROUTES.recordID ||
                  PATTERNS.SERVICESrecordID != SERVICES.recordID || 
                  PATTERNS.directionIndex != nI ||
                  PATTERNS.PATTERNNAMESrecordID != TRIPS.PATTERNNAMESrecordID)
            {
            strcpy(outputString, outputStringSave);
            sprintf(szarString, ",-1,%d\r\n", seq);
            strcat(outputString, szarString);
            _lwrite(hfOutputFile, outputString, strlen(outputString));
            }
          } // while on pattern
//
//  Get the next trip
//
          rcode2 = btrieve(B_GETNEXT, TMS_TRIPS, &TRIPS, &TRIPSKey1, 1);
        } // while on trips
      } // nI (directions on the route)
//
//  Get the next service
//
      rcode2 = btrieve(B_GETNEXT, TMS_SERVICES, &SERVICES, &SERVICESKey1, 1);
    }
//
//  Get the next route
//
    rcode2 = btrieve(B_GETNEXT, TMS_ROUTES, &ROUTES, &ROUTESKey1, 1);
  }
//
//  Close the file
//
  _lclose(hfOutputFile);
//
//  =======================
//  Blocks File - Block.csv
//  =======================
//
  pTRIPSChunk = &TRIPS.standard;
  strcpy(outputFileName, "Block.csv");
  hfOutputFile = _lcreat(outputFileName, 0);
  if(hfOutputFile == HFILE_ERROR)
  {
    LoadString(hInst, ERROR_320, szFormatString, SZFORMATSTRING_LENGTH);
    sprintf(szarString, szFormatString, outputFileName);
    MessageBeep(MB_ICONSTOP);
    MessageBox(NULL, szarString, TMS, MB_OK | MB_ICONSTOP);
    goto done;
  }
  StatusBarText(outputFileName);
//
//  Write out the column headers
//
  strcpy(outputString, "\"ServiceCode\",\"BlockCode\",\"BlockSeqNo\",\"PulloutStop\",\"PulloutTime\",\"TripCode\",\"PullinStop\",\"PullinTime\"\r\n");
  _lwrite(hfOutputFile, outputString, strlen(outputString));
//
//  Traverse the Routes Table
//
  rcode2 = btrieve(B_GETFIRST, TMS_ROUTES, &ROUTES, &ROUTESKey1, 1);
  while(rcode2 == 0)
  {
    if(StatusBarAbort())
    {
      goto done;
    }
//
//  Go through the services
//
    rcode2 = btrieve(B_GETFIRST, TMS_SERVICES, &SERVICES, &SERVICESKey1, 1);
    while(rcode2 == 0)
    {
      if(StatusBarAbort())
      {
        goto done;
      }
//
//  Get all the blocks on this route group
//
      NODESKey1.flags = NODES_FLAG_GARAGE;
      memset(NODESKey1.abbrName, 0x00, NODES_ABBRNAME_LENGTH);
      rcode2 = btrieve(B_GETGREATEROREQUAL, TMS_NODES, &NODES, &NODESKey1, 1);
      bFirst = TRUE;
      while(rcode2 == 0 && NODES.flags & NODES_FLAG_GARAGE)
      {
//
//  Cycle through the blocks
//
        assignedToNODESrecordID = bFirst ? NO_RECORD : NODES.recordID;
        TRIPSKey2.assignedToNODESrecordID = assignedToNODESrecordID;
        TRIPSKey2.RGRPROUTESrecordID = ROUTES.recordID;
        TRIPSKey2.SGRPSERVICESrecordID = SERVICES.recordID;
        TRIPSKey2.blockNumber = 1;
        TRIPSKey2.blockSequence = NO_TIME;
        rcode2 = btrieve(B_GETGREATEROREQUAL, TMS_TRIPS, &TRIPS, &TRIPSKey2, 2);
        blockNumber = NO_RECORD;
//
//  Get the first trip on the block
//
        while(rcode2 == 0 &&
              pTRIPSChunk->assignedToNODESrecordID == assignedToNODESrecordID &&
              pTRIPSChunk->RGRPROUTESrecordID == ROUTES.recordID &&
              pTRIPSChunk->SGRPSERVICESrecordID == SERVICES.recordID)
        {
          if(StatusBarAbort())
          {
            goto done;
          }
          if(pTRIPSChunk->blockNumber != blockNumber)
          {
            seq = 1;
            blockNumber = pTRIPSChunk->blockNumber;
          }
//
//  Gen the whole trip
//
          GenerateTrip(TRIPS.ROUTESrecordID, TRIPS.SERVICESrecordID,
                TRIPS.directionIndex, TRIPS.PATTERNNAMESrecordID,
                TRIPS.timeAtMLP, GENERATETRIP_FLAG_DISPLAYERRORS, &GTResults);
//
//  ServiceCode (SERVICES.recordID) 
//  BlockCode (blockNumber)
//  BlockSeqNo (seq)
//
          sprintf(outputString, "%ld,%ld,%d,", SERVICES.recordID, blockNumber, seq);
//
//  PulloutStop (pTRIPSChunk->POGNODESrecordID) and PulloutTime
//
//  Pullout?
//
          if(pTRIPSChunk->POGNODESrecordID == NO_RECORD)
          {
            strcat(outputString, ",,");
          }
          else
          {
            GCTData.fromPATTERNNAMESrecordID = TRIPS.PATTERNNAMESrecordID;
            GCTData.toPATTERNNAMESrecordID = TRIPS.PATTERNNAMESrecordID;
            GCTData.fromROUTESrecordID = pTRIPSChunk->RGRPROUTESrecordID;
            GCTData.fromSERVICESrecordID = pTRIPSChunk->SGRPSERVICESrecordID;
            GCTData.toROUTESrecordID = pTRIPSChunk->RGRPROUTESrecordID;
            GCTData.toSERVICESrecordID = pTRIPSChunk->SGRPSERVICESrecordID;
            GCTData.fromNODESrecordID = pTRIPSChunk->POGNODESrecordID;
            GCTData.toNODESrecordID = GTResults.firstNODESrecordID;
            GCTData.timeOfDay = GTResults.firstNodeTime;
            dhd = GetConnectionTime(GCT_FLAG_DEADHEADTIME, &GCTData, &distance);
            distance = (float)fabs((double)distance);
            if(dhd == NO_TIME)
            {
              dhd = 0;
            }
            fromNODESrecordID = pTRIPSChunk->POGNODESrecordID;
            fromTime = GTResults.firstNodeTime - dhd;
            sprintf(tempString, "%ld,%ld,", fromNODESrecordID, fromTime);
            strcat(outputString, tempString);
          }
//
//  TripCode (TRIPS.recordID)
//
          sprintf(tempString, "%ld,", TRIPS.recordID);
          strcat(outputString, tempString);
//
//  PullinStop (pTRIPSChunk->PIGNODESrecordID) and PullinTime
//
          if(pTRIPSChunk->PIGNODESrecordID == NO_RECORD)
          {
            strcat(outputString, ",");
          }
          else
          {
            GCTData.fromPATTERNNAMESrecordID = TRIPS.PATTERNNAMESrecordID;
            GCTData.toPATTERNNAMESrecordID = TRIPS.PATTERNNAMESrecordID;
            GCTData.fromROUTESrecordID = pTRIPSChunk->RGRPROUTESrecordID;
            GCTData.fromSERVICESrecordID = pTRIPSChunk->SGRPSERVICESrecordID;
            GCTData.toROUTESrecordID = pTRIPSChunk->RGRPROUTESrecordID;
            GCTData.toSERVICESrecordID = pTRIPSChunk->SGRPSERVICESrecordID;
            GCTData.fromNODESrecordID = GTResults.lastNODESrecordID;
            GCTData.toNODESrecordID = pTRIPSChunk->PIGNODESrecordID;
            GCTData.timeOfDay = GTResults.lastNodeTime;
            dhd = GetConnectionTime(GCT_FLAG_DEADHEADTIME, &GCTData, &distance);
            distance = (float)fabs((double)distance);
            if(dhd == NO_TIME)
            {
              dhd = 0;
            }
            toNODESrecordID = pTRIPSChunk->PIGNODESrecordID;
            toTime = GTResults.lastNodeTime + dhd;
            sprintf(tempString, "%ld,%ld", toNODESrecordID, toTime);
            strcat(outputString, tempString);
          }
          strcat(outputString, "\r\n");
//
//  Write it
//
          _lwrite(hfOutputFile, outputString, strlen(outputString));
          rcode2 = btrieve(B_GETNEXT, TMS_TRIPS, &TRIPS, &TRIPSKey2, 2);
          seq++;
        }  // while blocks on this garage node

//
//  Get the next garage node
//
        if(bFirst)
        {
          bFirst = FALSE;
        }
        else
        {
          rcode2 = btrieve(B_GETNEXT, TMS_NODES, &NODES, &NODESKey1, 1);
        }
      }  // while cycling through garage nodes
//
//  Get the next service
//
      rcode2 = btrieve(B_GETNEXT, TMS_SERVICES, &SERVICES, &SERVICESKey1, 1);
    }
//
//  Get the next route
//
    rcode2 = btrieve(B_GETNEXT, TMS_ROUTES, &ROUTES, &ROUTESKey1, 1);
  }  
//
//  Close the file
//
  _lclose(hfOutputFile);
//
//  ===================
//  Runs File - Run.csv
//  ===================
//
  strcpy(outputFileName, "Run.csv");
  hfOutputFile = _lcreat(outputFileName, 0);
  if(hfOutputFile == HFILE_ERROR)
  {
    LoadString(hInst, ERROR_320, szFormatString, SZFORMATSTRING_LENGTH);
    sprintf(szarString, szFormatString, outputFileName);
    MessageBeep(MB_ICONSTOP);
    MessageBox(NULL, szarString, TMS, MB_OK | MB_ICONSTOP);
    goto done;
  }
  StatusBarText(outputFileName);
//
//  Write out the column headers
//
  strcpy(outputString, "\"ServiceCode\",\"RunCode\",\"PieceNumber\",\"StartTrip\",\"StartStop\",\"EndTrip\",\"EndStop\"\r\n");
  _lwrite(hfOutputFile, outputString, strlen(outputString));
//
//  Traverse the Divisions Table
//
  rcode2 = btrieve(B_GETFIRST, TMS_DIVISIONS, &DIVISIONS, &DIVISIONSKey1, 1);
  while(rcode2 == 0)
  {
    if(StatusBarAbort())
    {
      goto done;
    }
//
//  Go through the services
//
    rcode2 = btrieve(B_GETFIRST, TMS_SERVICES, &SERVICES, &SERVICESKey1, 1);
    while(rcode2 == 0)
    {
      if(StatusBarAbort())
      {
        goto done;
      }
//
//  Get all the Runs on this division and service
//
      RUNSKey1.DIVISIONSrecordID = DIVISIONS.recordID;
      RUNSKey1.SERVICESrecordID = SERVICES.recordID;
      RUNSKey1.runNumber = NO_RECORD;
      RUNSKey1.pieceNumber = NO_RECORD;
      rcode2 = btrieve(B_GETGREATER, TMS_RUNS, &RUNS, &RUNSKey1, 1);
      while(rcode2 == 0 &&
            RUNS.DIVISIONSrecordID == DIVISIONS.recordID &&
            RUNS.SERVICESrecordID == SERVICES.recordID)
      {
//
//  ServiceCode (SERVICES.recordID)
//  RunCode (RUNS.runNumber)
//  PieceNumber (RUNS.pieceNumber)
//  StartTrip (RUNS.start.TRIPSrecordID)
//  StartStop (RUNS.start.NODESrecordID)
//  EndTrip (RUNS.end.TRIPSrecordID)
//  EndStop (RUNS.end.NODESrecordID)
//
        sprintf(outputString, "%ld,%ld,%ld,%ld,%ld,%ld,%ld\r\n",
              SERVICES.recordID, RUNS.runNumber, RUNS.pieceNumber,
              RUNS.start.TRIPSrecordID, RUNS.start.NODESrecordID,
              RUNS.end.TRIPSrecordID, RUNS.end.NODESrecordID);
        _lwrite(hfOutputFile, outputString, strlen(outputString));
        rcode2 = btrieve(B_GETNEXT, TMS_RUNS, &RUNS, &RUNSKey1, 1);
      }
//
//  Get the next service
//
      rcode2 = btrieve(B_GETNEXT, TMS_SERVICES, &SERVICES, &SERVICESKey1, 1);
    }
//
//  Get the next division
//
    rcode2 = btrieve(B_GETNEXT, TMS_DIVISIONS, &DIVISIONS, &DIVISIONSKey1, 1);
  }  
//
//  Close the file
//
  _lclose(hfOutputFile);
//
//  =========================
//  Drivers File - Driver.csv
//  =========================
//
  strcpy(outputFileName, "Driver.csv");
  hfOutputFile = _lcreat(outputFileName, 0);
  if(hfOutputFile == HFILE_ERROR)
  {
    LoadString(hInst, ERROR_320, szFormatString, SZFORMATSTRING_LENGTH);
    sprintf(szarString, szFormatString, outputFileName);
    MessageBeep(MB_ICONSTOP);
    MessageBox(NULL, szarString, TMS, MB_OK | MB_ICONSTOP);
    goto done;
  }
  StatusBarText(outputFileName);
//
//  Write out the column headers
//
  strcpy(outputString, "\"DriverCode\",\"FirstName\",\"LastName\",\"Password\"\r\n");
  _lwrite(hfOutputFile, outputString, strlen(outputString));
//
//  Traverse the Drivers Table
//
  rcode2 = btrieve(B_GETFIRST, TMS_DRIVERS, &DRIVERS, &DRIVERSKey1, 1);
  while(rcode2 == 0)
  {
    if(StatusBarAbort())
    {
      goto done;
    }
//
//
//  DriverCode (DRIVERS.recordID)
//
    sprintf(outputString, "%ld,", DRIVERS.recordID);
//
//  FirstName (DRIVERS.firstName)
//
    strncpy(tempString, DRIVERS.firstName, DRIVERS_FIRSTNAME_LENGTH);
    trim(tempString, DRIVERS_FIRSTNAME_LENGTH);
    sprintf(szarString, "\"%s\",", tempString);
    strcat(outputString, szarString);
//
//  LastName (DRIVERS.lastName)
//
    strncpy(tempString, DRIVERS.lastName, DRIVERS_LASTNAME_LENGTH);
    trim(tempString, DRIVERS_LASTNAME_LENGTH);
    sprintf(szarString, "\"%s\",", tempString);
    strcat(outputString, szarString);
//
//  Password (DRIVERS.badgeNumber, or "None" if not entered)
//
    strncpy(tempString, DRIVERS.badgeNumber, DRIVERS_BADGENUMBER_LENGTH);
    trim(tempString, DRIVERS_BADGENUMBER_LENGTH);
    if(strcmp(tempString, "") == 0)
    {
      strcpy(tempString, "None");
    }
    sprintf(szarString, "\"%s\"\r\n", tempString);
    strcat(outputString, szarString);
//
//  Write it out
//
    _lwrite(hfOutputFile, outputString, strlen(outputString));
    rcode2 = btrieve(B_GETNEXT, TMS_DRIVERS, &DRIVERS, &DRIVERSKey1, 1);
  }
//
//  Close the file
//
  _lclose(hfOutputFile);
//
//  ===========================
//  Vehicles File - Vehicle.csv
//  ===========================
//
  strcpy(outputFileName, "Vehicle.csv");
  hfOutputFile = _lcreat(outputFileName, 0);
  if(hfOutputFile == HFILE_ERROR)
  {
    LoadString(hInst, ERROR_320, szFormatString, SZFORMATSTRING_LENGTH);
    sprintf(szarString, szFormatString, outputFileName);
    MessageBeep(MB_ICONSTOP);
    MessageBox(NULL, szarString, TMS, MB_OK | MB_ICONSTOP);
    goto done;
  }
  StatusBarText(outputFileName);
//
//  Write out the column headers
//
  strcpy(outputString, "\"VehicleCode\",\"Name\"\r\n");
  _lwrite(hfOutputFile, outputString, strlen(outputString));
//
//  Traverse the Buses Table
//
  rcode2 = btrieve(B_GETFIRST, TMS_BUSES, &BUSES, &BUSESKey1, 1);
  while(rcode2 == 0)
  {
    if(StatusBarAbort())
    {
      goto done;
    }
//
//
//  VehicleCode (BUSES.recordID)
//
    sprintf(outputString, "%ld,", BUSES.recordID);
//
//  Name (BUSES.number)
//
    strncpy(tempString, BUSES.number, BUSES_NUMBER_LENGTH);
    trim(tempString, BUSES_NUMBER_LENGTH);
    sprintf(szarString, "\"%s\"\r\n", tempString);
    strcat(outputString, szarString);
//
//  Write it out
//
    _lwrite(hfOutputFile, outputString, strlen(outputString));
    rcode2 = btrieve(B_GETNEXT, TMS_BUSES, &BUSES, &BUSESKey1, 1);
  }
//
//  Close the file
//
  _lclose(hfOutputFile);
//
//  All done
//
  StatusBar(-1L, -1L);
  bFinishedOK = TRUE;
  
  done:
    chdir("..");  // Back to where we started from
    StatusBarEnd();
  
  if(!bFinishedOK)
  {
    TMSError((HWND)NULL, MB_ICONINFORMATION, ERROR_227, (HANDLE)NULL);
  }
  if(bGotError)
  {
    TMSError((HWND)NULL, MB_ICONINFORMATION, ERROR_350, (HANDLE)NULL);
  }
  _lclose(hfErrorLog);
  distanceMeasure = distanceMeasureSave;
  recordLength[TMS_COMMENTS] = COMMENTS_FIXED_LENGTH;
  return(bFinishedOK);
}

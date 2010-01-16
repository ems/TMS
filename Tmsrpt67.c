// 
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2006 Schedule Masters, Inc.
//  All rights reserved.
//
//
//  TMS Unload to the Ontira AVL System
//

#include "TMSHeader.h"
#include "cistms.h"
#include "limits.h"
#include <math.h>

int  distanceMeasureSave;

#define OUTPUT_FOLDER_NAME "Ontira Interface"

typedef struct NodesAndStopsStruct
{
  long NODESrecordID;
  long StopNumber;
  long OutboundStopNumber;
  long InboundStopNumber;
}  NodesAndStopsDef;

#define TMSRPT67_MAXNODESANDSTOPS 300

#define SERVICELEVELINFO_REPORTLABEL_LENGTH 15
#define SERVICELEVELINFO_DESCRIPTION_LENGTH 50

#define STOPINFO_REPORTLABEL_LENGTH         NODES_DESCRIPTION_LENGTH
#define STOPINFO_DESCRIPTION_LENGTH         NODES_INTERSECTION_LENGTH

#define ROUTEINFO_REPORTLABEL_LENGTH        15
#define ROUTEINFO_DESCRIPTION_LENGTH        50
#define ROUTEINFO_INTERNETNAME_LENGTH       40

#define PATTERNINFO_PATTERNNAME_LENGTH      20

#define RUNINFO_DESCRIPTION_LENGTH        50


BOOL FAR TMSRPT67(TMSRPTPassedDataDef *pPassedData)
{
  NodesAndStopsDef NodesAndStops[TMSRPT67_MAXNODESANDSTOPS];
  GetConnectionTimeDef GCTData;
  GenerateTripDef  GTResults;
  BLOCKSDef       *pTRIPSChunk;
  double  prevLat, prevLon;
  HFILE   hfOutputFile;
  HFILE   hfErrorLog;
  float   distanceToHere;
  float   tripDistances[100];
  float   distance;
  struct  tm tmED, tmUD, today;
  time_t  EDmktime;
  time_t  UDmktime;
  time_t  now;
  BOOL  bFound;
  BOOL  bGotError;
  BOOL  bFinishedOK;
  BOOL  bBikeRack;
  BOOL  bFirst;
  long  SERVICESinEffect[TMSRPT66_MAXSERVICES];
  long  year, month, day;
  long  serviceRecordID;
  long  previousPattern;
  long  timeAtStop;
  long  absRecID;
  long  stopFlags;
  long  stopNumber;
  long  assignedToNODESrecordID;
  long  blockNumber;
  long  fromNODESrecordID;
  long  toNODESrecordID;
  long  toTime;
  long  fromTime;
  long  dhd;
  long  number;
  char  outputString[1024];
  char  outputStringSave[1024];
  char  out2[1024];
  char *ptr;
  char *pszReportName;
  char  outputFileName[64];
  char  szServiceCode[SERVICES_NAME_LENGTH + 1];
  char  szTripCode[16];
  int   nI;
  int   rcode2;
  int   dayOfWeek;
  int   numServices;
  int   tripIndex;
  int   numNodes;
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
  pPassedData->nReportNumber = 66;
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
//  Get today's date
//
   time(&now);
   today = *localtime(&now);
//
//  ==============================
//  Service File - oci_service.csv
//  ==============================
//
  if(StatusBarAbort())
  {
    goto done;
  }
  strcpy(outputFileName, "oci_service.csv");
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
//  Write out the header
//
  strcpy(outputString, "\"ServiceCode\",\"Date\"\r\n");
  _lwrite(hfOutputFile, outputString, strlen(outputString));
  numServices = 0;
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
      dayOfWeek = (tmED.tm_wday == 0 ? 6 : tmED.tm_wday - 1);
      for(bFound = FALSE, nI = 0; nI < numServices; nI++)
      {
        if(ROSTERPARMS.serviceDays[dayOfWeek] == SERVICESinEffect[nI])
        {
          bFound = TRUE;
          break;
        }
      }
      if(!bFound)
      {
        SERVICESinEffect[numServices] = ROSTERPARMS.serviceDays[dayOfWeek];
        numServices++;
      }
//
//  ServiceCode (SERVICES.recordID)
//
      SERVICESKey0.recordID = ROSTERPARMS.serviceDays[dayOfWeek];
      btrieve(B_GETEQUAL, TMS_SERVICES, &SERVICES, &SERVICESKey0, 0);
      strncpy(tempString, SERVICES.name, SERVICES_NAME_LENGTH);
      trim(tempString, SERVICES_NAME_LENGTH);
      sprintf(szarString, "\"%s\",", tempString);
      strcpy(outputString, szarString);
//
//  Date
//
      sprintf(szarString, "\"%4ld%02ld%02ld\"\r\n", tmED.tm_year + 1900, tmED.tm_mon + 1, tmED.tm_mday);
      strcat(outputString, szarString);
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
//  ============================
//  Routes File - oci_routes.csv
//  ============================
//
  if(StatusBarAbort())
  {
    goto done;
  }
  strcpy(outputFileName, "oci_routes.csv");
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
//  Write out the header
//
  strcpy(outputString, "\"RouteCode\",\"RouteName\",\"DirectionCode\",\"DirectionName\",\"PatternCode\",\"PatternName\"\r\n");
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
//  Go through the directions
//
      for(nI = 0; nI < 2; nI++)
      {
        if(ROUTES.DIRECTIONSrecordID[nI] == NO_RECORD)
        {
          continue;
        }
//
//  RouteCode (ROUTES.number)
//
        strncpy(tempString, ROUTES.number, ROUTES_NUMBER_LENGTH);
        trim(tempString, ROUTES_NUMBER_LENGTH);
        ptr = tempString;
        while(ptr && *ptr == ' ')
        {
          ptr++;
        }
        sprintf(szarString, "\"%s\",", ptr);
        strcpy(outputString, szarString);
//
//  RouteName (ROUTES.name)
//
        strncpy(tempString, ROUTES.name, ROUTES_NAME_LENGTH);
        trim(tempString, ROUTES_NAME_LENGTH);
        sprintf(szarString, "\"%s\",", tempString);
        strcat(outputString, szarString);
//
//  DirectionCode (DIRECTIONS.abbrName)
//
        DIRECTIONSKey0.recordID = ROUTES.DIRECTIONSrecordID[nI];
        btrieve(B_GETEQUAL, TMS_DIRECTIONS, &DIRECTIONS, &DIRECTIONSKey0, 0);
        strncpy(tempString, DIRECTIONS.abbrName, DIRECTIONS_ABBRNAME_LENGTH);
        trim(tempString, DIRECTIONS_ABBRNAME_LENGTH);
        sprintf(szarString, "\"%s\",", tempString);
        strcat(outputString, szarString);
//
//  DirectionName (DIRECTIONS.longName)
//
        DIRECTIONSKey0.recordID = ROUTES.DIRECTIONSrecordID[nI];
        btrieve(B_GETEQUAL, TMS_DIRECTIONS, &DIRECTIONS, &DIRECTIONSKey0, 0);
        strncpy(tempString, DIRECTIONS.longName, DIRECTIONS_LONGNAME_LENGTH);
        trim(tempString, DIRECTIONS_LONGNAME_LENGTH);
        sprintf(szarString, "\"%s\",", tempString);
        strcat(outputString, szarString);
//
// Cycle through the patterns for route/service/direction
//
        strcpy(outputStringSave, outputString);
        rcode2 = btrieve(B_GETFIRST, TMS_SERVICES, &SERVICES, &SERVICESKey0, 0);
        while(rcode2 == 0)
        {
          PATTERNSKey2.ROUTESrecordID = ROUTES.recordID;
          PATTERNSKey2.SERVICESrecordID = SERVICES.recordID;
          PATTERNSKey2.PATTERNNAMESrecordID = basePatternRecordID;
          PATTERNSKey2.directionIndex = nI;
          PATTERNSKey2.nodeSequence = NO_RECORD;
          previousPattern = NO_RECORD;
          rcode2 = btrieve(B_GETGREATER, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
          while(rcode2 == 0 &&
                PATTERNS.ROUTESrecordID == ROUTES.recordID &&
                PATTERNS.SERVICESrecordID == SERVICES.recordID &&
                PATTERNS.directionIndex == nI)
          {
            if(PATTERNS.PATTERNNAMESrecordID != previousPattern)
            {
//
//  PatternCode (SERVICES.recordID-PATTERNNAMES.recordID)
//
              sprintf(szarString, "\"%ld-%ld\",", SERVICES.recordID, PATTERNS.PATTERNNAMESrecordID);
              strcat(outputString, szarString);
//
//  PatternName (PATTERNNAMES.name)
//
              PATTERNNAMESKey0.recordID = PATTERNS.PATTERNNAMESrecordID;
              btrieve(B_GETEQUAL, TMS_PATTERNNAMES, &PATTERNNAMES, &PATTERNNAMESKey0, 0);
              strncpy(tempString, PATTERNNAMES.name, PATTERNNAMES_NAME_LENGTH);
              trim(tempString, PATTERNNAMES_NAME_LENGTH);
              sprintf(szarString, "\"%s\"\r\n", tempString);
              strcat(outputString, szarString);
//
//  Write out the record and get the next pattern
//
              _lwrite(hfOutputFile, outputString, strlen(outputString));
              strcpy(outputString, outputStringSave);
              previousPattern = PATTERNS.PATTERNNAMESrecordID;
            }
            rcode2 = btrieve(B_GETNEXT, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
          }
          rcode2 = btrieve(B_GETNEXT, TMS_SERVICES, &SERVICES, &SERVICESKey0, 0);
        }
      } // nI
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
//  ==============================
//  Bus Stops File - oci_stops.csv
//  ==============================
//
  if(StatusBarAbort())
  {
    goto done;
  }
  strcpy(outputFileName, "oci_stops.csv");
  hfOutputFile = _lcreat(outputFileName, 0);
  if(hfOutputFile == HFILE_ERROR)
  {
    LoadString(hInst, ERROR_320, szFormatString, SZFORMATSTRING_LENGTH);
    sprintf(szarString, szFormatString, outputFileName);
    MessageBeep(MB_ICONSTOP);
    MessageBox(NULL, szarString, TMS, MB_OK | MB_ICONSTOP);
    goto done;
  }
//
//  Write out the header
//
  strcpy(outputString, "\"StopCode\",\"StopName\",\"Latitude\",\"Longitude\",\"Shelter\",\"Bench\",\"Accessible\",\"TimingPoint\"\r\n");
  _lwrite(hfOutputFile, outputString, strlen(outputString));
  StatusBarText(outputFileName);
//
//  Get the list of nodes
//
//
//  Cycle through the Nodes Table to get associated stop numbers
//
  rcode2 = btrieve(B_GETFIRST, TMS_NODES, &NODES, &NODESKey0, 0);
  numNodes = 0;
  while(rcode2 == 0)
  {
    if(!(NODES.flags & NODES_FLAG_STOP))
    {
      NodesAndStops[numNodes].NODESrecordID = NODES.recordID;
      NodesAndStops[numNodes].StopNumber = NODES.number;
      NodesAndStops[numNodes].OutboundStopNumber = NODES.OBStopNumber;
      NodesAndStops[numNodes].InboundStopNumber = NODES.IBStopNumber;
      numNodes++;
      if(numNodes >= TMSRPT67_MAXNODESANDSTOPS)
      {
        MessageBeep(MB_ICONSTOP);
        MessageBox(NULL, "TMSRPT67_MAXNODESANDSTOPS Exceeded.\n\nPlease contact Schedule Masters, Inc. for help", TMS, MB_OK | MB_ICONSTOP);
        goto done;
      }
    }
    rcode2 = btrieve(B_GETNEXT, TMS_NODES, &NODES, &NODESKey0, 0);
  }
//
//  Cycle through the Nodes Table
//
  rcode2 = btrieve(B_GETFIRST, TMS_NODES, &NODES, &NODESKey0, 0);
  while(rcode2 == 0)
  {
//
//  StopCode (NODES.number or NODES.abbrName)
//
    if(NODES.flags & NODES_FLAG_STOP)
    {
      sprintf(szarString, "\"%04ld\",", NODES.number);
    }
    else
    {
//      strncpy(tempString, NODES.abbrName, NODES_ABBRNAME_LENGTH);
//      trim(tempString, NODES_ABBRNAME_LENGTH);
//      sprintf(szarString, "\"%s\",", tempString);
      if(nI == 0)
      {
        number = (NODES.OBStopNumber <= 0 ? NODES.number : NODES.OBStopNumber);
      }
      else
      {
        number = (NODES.IBStopNumber <= 0 ? NODES.number : NODES.IBStopNumber);
      }
      sprintf(szarString, "\"%04ld\",", number);
    }
    strcpy(outputString, szarString);
//
//  StopName (NODES.intersection)
//
    strncpy(tempString, NODES.intersection, NODES_INTERSECTION_LENGTH);
    trim(tempString, NODES_INTERSECTION_LENGTH);
    ptr = strstr(tempString, "&&");
    if(ptr)
    {
      *ptr = 'a';
      *(ptr + 1) = 't';
    }
    sprintf(szarString, "\"%s\",", tempString);
    strcat(outputString, szarString);
//
//  Latitude (NODES.latitude)
//
    sprintf(tempString, "%12.6f", NODES.latitude);
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
    sprintf(tempString, "%12.6f", NODES.longitude);
    ptr = tempString;
    while(ptr && *ptr == ' ')
    {
      ptr++;
    }
    sprintf(szarString, "\"%s\",", ptr);
    strcat(outputString, szarString);
//
//  If this isn't a stop, look to see if we can find the stop in the Nodes Table
//
    if(NODES.flags & NODES_FLAG_STOP)
    {
      stopFlags = NODES.stopFlags;
    }
    else
    {
      stopFlags = 0;
      stopNumber = NO_RECORD;
      if(NODES.number > 0)
      {
        stopNumber = NODES.number;
      }
      if(stopNumber == NO_RECORD && NODES.OBStopNumber > 0)
      {
        stopNumber = NODES.OBStopNumber;
      }
      if(stopNumber == NO_RECORD && NODES.IBStopNumber > 0)
      {
        stopNumber = NODES.IBStopNumber;
      }
      if(stopNumber != NO_RECORD)
      {
        btrieve(B_GETPOSITION, TMS_NODES, &absRecID, &NODESKey0, 0);
        rcode2 = btrieve(B_GETFIRST, TMS_NODES, &NODES, &NODESKey0, 0);
        while(rcode2 == 0)
        {
          if(NODES.flags & NODES_FLAG_STOP)
          {
            if(NODES.number == stopNumber)
            {
              stopFlags = NODES.stopFlags;
              break;
            }
          }
          rcode2 = btrieve(B_GETNEXT, TMS_NODES, &NODES, &NODESKey0, 0);
        }
        NODES.recordID = absRecID;
        btrieve(B_GETDIRECT, TMS_NODES, &NODES, &NODESKey0, 0);
      }
    }
//
//  Shelter (item 12 from "Stop Flags.txt")
//
    sprintf(tempString, (stopFlags & (1 << 12) ? "True" : "False"));
    sprintf(szarString, "\"%s\",", tempString);
    strcat(outputString, szarString);
//
//  Bench (item 5 from "Stop Flags.txt")
//
    sprintf(tempString, (stopFlags & (1 << 5) ? "True" : "False"));
    sprintf(szarString, "\"%s\",", tempString);
    strcat(outputString, szarString);
//
//  Accessible (item 16 from "Stop Flags.txt")
//
    sprintf(tempString, (stopFlags & (1 << 6) ? "True" : "False"));
    sprintf(szarString, "\"%s\",", tempString);
    strcat(outputString, szarString);
//
//  Timing point?
//
    if(!(NODES.flags & NODES_FLAG_STOP))
    {
      bFound = TRUE;
    }
    else
    {
      bFound = FALSE;
      for(nI = 0; nI < numNodes; nI++)
      {
        if(NodesAndStops[nI].StopNumber == NODES.number ||
              NodesAndStops[nI].OutboundStopNumber == NODES.number ||
              NodesAndStops[nI].InboundStopNumber == NODES.number)
        {
          bFound = TRUE;
          break;
        }
      }
    }
    sprintf(tempString, (bFound ? "True" : "False"));
    sprintf(szarString, "\"%s\"\r\n", tempString);
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
//  Bus Patterns File - oci_patterns.csv
//  ====================================
//
  if(StatusBarAbort())
  {
    goto done;
  }
  strcpy(outputFileName, "oci_patterns.csv");
  hfOutputFile = _lcreat(outputFileName, 0);
  if(hfOutputFile == HFILE_ERROR)
  {
    LoadString(hInst, ERROR_320, szFormatString, SZFORMATSTRING_LENGTH);
    sprintf(szarString, szFormatString, outputFileName);
    MessageBeep(MB_ICONSTOP);
    MessageBox(NULL, szarString, TMS, MB_OK | MB_ICONSTOP);
    goto done;
  }
//
//  Write out the header
//
  strcpy(outputString, "\"RouteCode\",\"DirectionCode\",\"PatternCode\",\"StopCode\",\"Sequence\"\r\n");
  _lwrite(hfOutputFile, outputString, strlen(outputString));
  StatusBarText(outputFileName);
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
//  RouteCode (ROUTES.number)
//
        strncpy(tempString, ROUTES.number, ROUTES_NUMBER_LENGTH);
        trim(tempString, ROUTES_NUMBER_LENGTH);
        ptr = tempString;
        while(ptr && *ptr == ' ')
        {
          ptr++;
        }
        sprintf(szarString, "\"%s\",", ptr);
        strcpy(outputString, szarString);
//
//  DirectionCode (DIRECTIONS.abbrName)
//
        DIRECTIONSKey0.recordID = ROUTES.DIRECTIONSrecordID[nI];
        btrieve(B_GETEQUAL, TMS_DIRECTIONS, &DIRECTIONS, &DIRECTIONSKey0, 0);
        strncpy(tempString, DIRECTIONS.abbrName, DIRECTIONS_ABBRNAME_LENGTH);
        trim(tempString, DIRECTIONS_ABBRNAME_LENGTH);
        sprintf(szarString, "\"%s\",", tempString);
        strcat(outputString, szarString);
//
//  Go through all the services
//
        strcpy(outputStringSave, outputString);
        rcode2 = btrieve(B_GETFIRST, TMS_SERVICES, &SERVICES, &SERVICESKey0, 0);
        while(rcode2 == 0)
        {
//
//  Go through all the patterns
//
          PATTERNSKey2.ROUTESrecordID = ROUTES.recordID;
          PATTERNSKey2.SERVICESrecordID = SERVICES.recordID;
          PATTERNSKey2.PATTERNNAMESrecordID = basePatternRecordID;
          PATTERNSKey2.directionIndex = nI;
          PATTERNSKey2.nodeSequence = -1;
          rcode2 = btrieve(B_GETGREATER, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
          previousPattern = NO_RECORD;
          while(rcode2 == 0 &&
                PATTERNS.ROUTESrecordID == ROUTES.recordID &&
                PATTERNS.SERVICESrecordID == SERVICES.recordID && 
                PATTERNS.directionIndex == nI)
          {
            if(previousPattern != PATTERNS.PATTERNNAMESrecordID)
            {
              previousPattern = PATTERNS.PATTERNNAMESrecordID;
              seq = 1;
            }
//
//  PatternCode (SERVICES.recordID-PATTERNS.PATTERNNAMESrecordID)
//
            sprintf(szarString, "\"%ld-%ld\",", SERVICES.recordID, PATTERNS.PATTERNNAMESrecordID);
            strcat(outputString, szarString);
//
//  StopCode (NODES.number or NODES.abbrName)
//
            NODESKey0.recordID = PATTERNS.NODESrecordID;
            btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
            if(!(NODES.flags & NODES_FLAG_STOP))
            {
              if(nI == 0)
              {
                number = (NODES.OBStopNumber <= 0 ? NODES.number : NODES.OBStopNumber);
              }
              else
              {
                number = (NODES.IBStopNumber <= 0 ? NODES.number : NODES.IBStopNumber);
              }
              sprintf(tempString, "%04ld", number);
//              strncpy(tempString, NODES.abbrName, NODES_ABBRNAME_LENGTH);
//              trim(tempString, NODES_ABBRNAME_LENGTH);
            }
            else
            {
              sprintf(tempString, "%04ld", NODES.number);
            }
            sprintf(szarString, "\"%s\",", tempString);
            strcat(outputString, szarString);
//
//  Sequence
//
            sprintf(tempString, "\"%d\"\r\n", seq++);
            strcat(outputString, tempString);
//
//  Write it out and get the next node on the pattern
//
            _lwrite(hfOutputFile, outputString, strlen(outputString));
            strcpy(outputString, outputStringSave);
            rcode2 = btrieve(B_GETNEXT, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
          } // while on patterns
          rcode2 = btrieve(B_GETNEXT, TMS_SERVICES, &SERVICES, &SERVICESKey0, 0);
        }
      } // nI
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
//  ==========================================
//  Bus Trips File - oci_trips_{RouteCode}.dat
//  ==========================================
//
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
    strncpy(tempString, ROUTES.number, ROUTES_NUMBER_LENGTH);
    trim(tempString, ROUTES_NUMBER_LENGTH);
    ptr = tempString;
    while(ptr && *ptr == ' ')
    {
      ptr++;
    }
    sprintf(outputFileName, "oci_trips_%s.csv", ptr);
    hfOutputFile = _lcreat(outputFileName, 0);
    if(hfOutputFile == HFILE_ERROR)
    {
      LoadString(hInst, ERROR_320, szFormatString, SZFORMATSTRING_LENGTH);
      sprintf(szarString, szFormatString, outputFileName);
      MessageBeep(MB_ICONSTOP);
      MessageBox(NULL, szarString, TMS, MB_OK | MB_ICONSTOP);
      goto done;
    }
//
//  Write out the header
//
    strcpy(outputString, "\"RouteCode\",\"DirectionCode\",\"PatternCode\",\"TripCode\",\"StopCode\",\"Sequence\",\"ArrivalTime\",\"DepartureTime\",\"TimingPoint\",\"Bicycles\",\"ServiceCode\"\r\n");
    _lwrite(hfOutputFile, outputString, strlen(outputString));
    StatusBarText(outputFileName);
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
      strncpy(szServiceCode, SERVICES.name, SERVICES_NAME_LENGTH);
      trim(szServiceCode, SERVICES_NAME_LENGTH);
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
//  RouteCode (ROUTES.number)
//
        strncpy(tempString, ROUTES.number, ROUTES_NUMBER_LENGTH);
        trim(tempString, ROUTES_NUMBER_LENGTH);
        ptr = tempString;
        while(ptr && *ptr == ' ')
        {
          ptr++;
        }
        sprintf(szarString, "\"%s\",", ptr);
        strcpy(outputString, szarString);
//
//  DirectionCode (DIRECTIONS.abbrName)
//
        DIRECTIONSKey0.recordID = ROUTES.DIRECTIONSrecordID[nI];
        btrieve(B_GETEQUAL, TMS_DIRECTIONS, &DIRECTIONS, &DIRECTIONSKey0, 0);
        strncpy(tempString, DIRECTIONS.abbrName, DIRECTIONS_ABBRNAME_LENGTH);
        trim(tempString, DIRECTIONS_ABBRNAME_LENGTH);
        sprintf(szarString, "\"%s\",", tempString);
        strcat(outputString, szarString);
        strcpy(out2, outputString);

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
          strcpy(outputString, out2);
//
//  PatternCode (SERVICES.recordID-PATTERNNAMES.recordID)
//
          sprintf(szarString, "\"%ld-%ld\",", TRIPS.SERVICESrecordID, TRIPS.PATTERNNAMESrecordID);
          strcat(outputString, szarString);
//
//  TripCode (TRIPS.tripNumber)
//
          sprintf(szarString, "\"%ld\",", TRIPS.tripNumber);
          strcat(outputString, szarString);
//
//  Does this vehicle have a bike rack?
//
          bBikeRack = FALSE;
          if(TRIPS.BUSTYPESrecordID != NO_RECORD)
          {
            BUSTYPESKey0.recordID = TRIPS.BUSTYPESrecordID;
            rcode2 = btrieve(B_GETEQUAL, TMS_BUSTYPES, &BUSTYPES, &BUSTYPESKey0, 0);
            if(rcode2 == 0)
            {
              if(BUSTYPES.flags & BUSTYPES_FLAG_CARRIER)
              {
                bBikeRack = TRUE;
              }
            }
          }
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
//  Save outputString to this point
//
          strcpy(outputStringSave, outputString);
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
//  StopCode (NODES.number or NODES.abbrName)
//
/*
            if(NODES.flags & NODES_FLAG_STOP)
            {
              sprintf(szarString, "\"%04ld\",", NODES.number);
            }
            else
            {
              strncpy(tempString, NODES.abbrName, NODES_ABBRNAME_LENGTH);
              trim(tempString, NODES_ABBRNAME_LENGTH);
              sprintf(szarString, "\"%s\",", tempString);
            }
*/
            if(!(NODES.flags & NODES_FLAG_STOP))
            {
              if(nI == 0)
              {
                number = (NODES.OBStopNumber <= 0 ? NODES.number : NODES.OBStopNumber);
              }
              else
              {
                number = (NODES.IBStopNumber <= 0 ? NODES.number : NODES.IBStopNumber);
              }
              sprintf(szarString, "\"%04ld\",", number);
            }
            else
            {
              sprintf(szarString, "\"%04ld\",", NODES.number);
            }

            strcat(outputString, szarString);
//
//  Sequence
//
            sprintf(szarString, "\"%d\",", seq++);
            strcat(outputString, szarString);
//
//  ArrivalTime (Time at the stop, in seconds)
//
//  Now time is in HHMM (11-Aug-08)
//
//            sprintf(szarString, "\"%ld\",", timeAtStop);
            sprintf(szarString, "\"%s\",", TcharNAP(timeAtStop, FALSE));
            strcat(outputString, szarString);
//
//  DepartureTime (Time at the stop, in seconds)
//
//  Now time is in HHMM (11-Aug-08)
//
//            sprintf(szarString, "\"%ld\",", timeAtStop);
            sprintf(szarString, "\"%s\",", TcharNAP(timeAtStop, FALSE));
            strcat(outputString, szarString);
//
//  Timepoint?
//
            sprintf(tempString, "%s", (NODES.flags & NODES_FLAG_STOP ? "False" : "True"));
            sprintf(szarString, "\"%s\",", tempString);
            strcat(outputString, szarString);
//
//  Bicycles?
//
            sprintf(tempString, "%s", (bBikeRack ? "True" : "False"));
            sprintf(szarString, "\"%s\",", tempString);
            strcat(outputString, szarString);
//
//  ServiceCode
//
            sprintf(szarString, "\"%s\"\r\n", szServiceCode);
            strcat(outputString, szarString);
//
//  Write it out and go back
//
            _lwrite(hfOutputFile, outputString, strlen(outputString));
            strcpy(outputString, outputStringSave);
            prevLat = NODES.latitude;
            prevLon = NODES.longitude;
            rcode2 = btrieve(B_GETNEXT, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
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
//  Close the file
//
    _lclose(hfOutputFile);
//
//  Get the next route
//
    rcode2 = btrieve(B_GETNEXT, TMS_ROUTES, &ROUTES, &ROUTESKey1, 1);
  }
//
//  Blocks
//
//
//  ============================
//  Blocks File - oci_blocks.dat
//  ============================
//
  pTRIPSChunk = &TRIPS.standard;
  sprintf(outputFileName, "oci_blocks.csv", ptr);
  hfOutputFile = _lcreat(outputFileName, 0);
  if(hfOutputFile == HFILE_ERROR)
  {
    LoadString(hInst, ERROR_320, szFormatString, SZFORMATSTRING_LENGTH);
    sprintf(szarString, szFormatString, outputFileName);
    MessageBeep(MB_ICONSTOP);
    MessageBox(NULL, szarString, TMS, MB_OK | MB_ICONSTOP);
    goto done;
  }
//
//  Write out the header
//
  strcpy(outputString, "\"BlockCode\",\"ServiceCode\",\"TripCode\",\"Sequence\",\"StartStopCode\",\"EndStopCode\",\"InService\"\r\n");
  _lwrite(hfOutputFile, outputString, strlen(outputString));
  StatusBarText(outputFileName);
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
      strncpy(szServiceCode, SERVICES.name, SERVICES_NAME_LENGTH);
      trim(szServiceCode, SERVICES_NAME_LENGTH);
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
//  Pullout?
//
          if(pTRIPSChunk->POGNODESrecordID == NO_RECORD)
          {
            fromNODESrecordID = GTResults.firstNODESrecordID;
            fromTime = GTResults.firstNodeTime;
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
//
//  Output the pull-out deadhead record
//
//  Start and End Stop codes
//
            NODESKey0.recordID = pTRIPSChunk->POGNODESrecordID;
            btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
            strncpy(tempString, NODES.abbrName, NODES_ABBRNAME_LENGTH);
            trim(tempString, NODES_ABBRNAME_LENGTH);
            NODESKey0.recordID = GTResults.firstNODESrecordID;
            btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
            strncpy(szarString, NODES.abbrName, NODES_ABBRNAME_LENGTH);
            trim(szarString, NODES_ABBRNAME_LENGTH);
//
//  Write it
//
            sprintf(outputString, "\"%ld\",\"%s\",\"\",\"%d\",\"%s\",\"%s\",\"False\"\r\n",
                  pTRIPSChunk->blockNumber, szServiceCode, seq, tempString, szarString);
           _lwrite(hfOutputFile, outputString, strlen(outputString));
           seq++;
          }
//
//  Start/end trip details
//
          sprintf(szTripCode, "%ld", TRIPS.tripNumber);
//
//  Start and End Stop codes
//
          NODESKey0.recordID = GTResults.firstNODESrecordID;
          btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
          strncpy(tempString, NODES.abbrName, NODES_ABBRNAME_LENGTH);
          trim(tempString, NODES_ABBRNAME_LENGTH);
          NODESKey0.recordID = GTResults.lastNODESrecordID;
          btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
          strncpy(szarString, NODES.abbrName, NODES_ABBRNAME_LENGTH);
          trim(szarString, NODES_ABBRNAME_LENGTH);
//
//  Write it
//
          sprintf(outputString, "\"%ld\",\"%s\",\"%s\",\"%d\",\"%s\",\"%s\",\"True\"\r\n",
                pTRIPSChunk->blockNumber, szServiceCode, szTripCode, seq, tempString, szarString);
          _lwrite(hfOutputFile, outputString, strlen(outputString));
          seq++;
//
//  Pullin
//
          if(pTRIPSChunk->PIGNODESrecordID == NO_RECORD)
          {
            toNODESrecordID = GTResults.lastNODESrecordID;
            toTime = GTResults.lastNodeTime;
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
//
//  Output the pull-in deadhead record
//
//  Start and End Stop codes
//
            NODESKey0.recordID = GTResults.lastNODESrecordID;
            btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
            strncpy(tempString, NODES.abbrName, NODES_ABBRNAME_LENGTH);
            trim(tempString, NODES_ABBRNAME_LENGTH);
            NODESKey0.recordID = pTRIPSChunk->PIGNODESrecordID;
            btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
            strncpy(szarString, NODES.abbrName, NODES_ABBRNAME_LENGTH);
            trim(szarString, NODES_ABBRNAME_LENGTH);
//
//  Write it
//
            sprintf(outputString, "\"%ld\",\"%s\",\"\",\"%d\",\"%s\",\"%s\",\"False\"\r\n",
                  pTRIPSChunk->blockNumber, szServiceCode, seq, tempString, szarString);
           _lwrite(hfOutputFile, outputString, strlen(outputString));
           seq++;
          }
          rcode2 = btrieve(B_GETNEXT, TMS_TRIPS, &TRIPS, &TRIPSKey2, 2);
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

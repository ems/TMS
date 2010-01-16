//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
#include "TMSHeader.h"

//
//  TMSRPT42() - Download to Trapeze ATIS System
//
//  ATIS-Stops.TXT (Tab Delimited)
//
//  AbbreviatedName char[4]
//  IntersectionAddress char[64]
//  Longitude float
//  Latitude float
//  Flags long
//  StopFlags long
//
//
//  ATIS-Patterns (Tab Delimited)
//
//  RouteNumber char[8]
//  RouteName char[64]
//  ServiceName char[32]
//  DirectionName char[16]
//  PatternName char[16]
//  AbbreviatedName char[4] (repeated)
//
//
//  ATIS-Trips (Tab Delimited)
//
//  RouteNumber char[8]
//  RouteName char[64]
//  ServiceName char[32]
//  DirectionName char[16]
//  PatternName char[16]
//  NodeTime long (repeated)
//

BOOL FAR TMSRPT42(TMSRPTPassedDataDef *pPassedData)
{
  GenerateTripDef GTResults;
  HFILE hfOutputFile;
  BOOL  bKeepGoing = FALSE;
  char  dummy[256];
  char  outputString[2048];
  char  routeNumber[ROUTES_NUMBER_LENGTH + 1];
  char  routeName[ROUTES_NAME_LENGTH + 1];
  char  serviceName[SERVICES_NAME_LENGTH + 1];
  char  directionName[DIRECTIONS_LONGNAME_LENGTH + 1];
  char  *ptr;
  long  statbarTotal;
  long  prevPattern;
//  long  tempLong;
  int   nI;
  int   nJ;
  int   currentNode;
  int   currentRoute;
  int   currentService;
  int   numNodes;
  int   numRoutes;
  int   numServices;
  int   rcode2;

//
//  Open the Nodes/Stops output file
//
  strcpy(tempString, szDatabaseFileName);
  if((ptr = strrchr(tempString, '\\')) != NULL)
    *ptr = '\0';
  strcat(tempString, "\\ATIS-Stops.txt");
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
//  Get the number of nodes for the benefit of the status bar
//
  rcode2 = btrieve(B_STAT, TMS_NODES, &BSTAT, dummy, 0);
  if(rcode2 != 0 || BSTAT.numRecords == 0)
  {
    TMSError(NULL, MB_ICONSTOP, ERROR_024, (HANDLE)NULL);
    goto deallocate;
  }
  numNodes = BSTAT.numRecords;
//
//  Unload the Nodes table
//
  LoadString(hInst, TEXT_117, tempString, TEMPSTRING_LENGTH);
  StatusBarStart(hWndMain, tempString);
  statbarTotal = numNodes;
  currentNode = 0;
  rcode2 = btrieve(B_GETFIRST, TMS_NODES, &NODES, &NODESKey1, 1);
  while(rcode2 == 0)
  {
    if(StatusBarAbort())
    {
      bKeepGoing = FALSE;
      goto deallocate;
    }
    StatusBar((long)currentNode, (long)statbarTotal);
//
//  AbbrName
//
    strncpy(tempString, NODES.abbrName, NODES_ABBRNAME_LENGTH);
    trim(tempString, NODES_ABBRNAME_LENGTH);
    strcpy(outputString, tempString);
    strcat(outputString, "\t");
//
//  IntersectionAddress
//
    strncpy(tempString, NODES.intersection, NODES_INTERSECTION_LENGTH);
    trim(tempString, NODES_INTERSECTION_LENGTH);
    strcat(outputString, tempString);
    strcat(outputString, "\t");
//
//  Longitude
//
//    memcpy(&tempLong, &NODES.longitude, sizeof(long));
//    sprintf(tempString, "%9.4f\t", (float)tempLong / 10000);
    sprintf(tempString, "%12.6f\t", NODES.longitude);
    strcat(outputString, tempString);
//
//  Latitude
//
//    memcpy(&tempLong, &NODES.latitude, sizeof(long));
//    sprintf(tempString, "%9.4f\t", (float)tempLong / 10000);
    sprintf(tempString, "%12.6f\t", NODES.latitude);
    strcat(outputString, tempString);
//
//  Flags
//
    sprintf(tempString, "%ld\t", NODES.flags);
    strcat(outputString, tempString);
//
//  StopFlags
//
    sprintf(tempString, "%ld\r\n", NODES.stopFlags);
    strcat(outputString, tempString);
//
//  Write the record
//
    _lwrite(hfOutputFile, outputString, strlen(outputString));
//
//  Get the next node
//
    rcode2 = btrieve(B_GETNEXT, TMS_NODES, &NODES, &NODESKey1, 1);
    currentNode++;
  }
  _lclose(hfOutputFile);
//
//  Get the number of routes and services
//  for the benefit of the status bar
//
  rcode2 = btrieve(B_STAT, TMS_ROUTES, &BSTAT, dummy, 0);
  if(rcode2 != 0 || BSTAT.numRecords == 0)
  {
    TMSError(NULL, MB_ICONSTOP, ERROR_008, (HANDLE)NULL);
    goto deallocate;
  }
  numRoutes = BSTAT.numRecords;
  rcode2 = btrieve(B_STAT, TMS_SERVICES, &BSTAT, dummy, 0);
  if(rcode2 != 0 || BSTAT.numRecords == 0)
  {
    TMSError(NULL, MB_ICONSTOP, ERROR_007, (HANDLE)NULL);
    goto deallocate;
  }
  numServices = BSTAT.numRecords;
//
//  Open the Patterns output file
//
  strcpy(tempString, szDatabaseFileName);
  if((ptr = strrchr(tempString, '\\')) != NULL)
    *ptr = '\0';
  strcat(tempString, "\\ATIS-Patterns.txt");
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
//  Loop through the routes
//
  LoadString(hInst, TEXT_117, tempString, TEMPSTRING_LENGTH);
  statbarTotal = numRoutes * numServices;
  currentRoute = 0;
  rcode2 = btrieve(B_GETFIRST, TMS_ROUTES, &ROUTES, &ROUTESKey1, 1);
  while(rcode2 == 0)
  {
    if(StatusBarAbort())
    {
      bKeepGoing = FALSE;
      goto deallocate;
    }
    strncpy(routeNumber, ROUTES.number, ROUTES_NUMBER_LENGTH);
    trim(routeNumber, ROUTES_NUMBER_LENGTH);
    strncpy(routeName, ROUTES.name, ROUTES_NAME_LENGTH);
    trim(routeName, ROUTES_NAME_LENGTH);
//
//  Loop through the services
//
    currentService = 1;
    rcode2 = btrieve(B_GETFIRST, TMS_SERVICES, &SERVICES, &SERVICESKey1, 1);
    while(rcode2 == 0)
    {
      if(StatusBarAbort())
      {
        bKeepGoing = FALSE;
        goto deallocate;
      }
      strncpy(serviceName, SERVICES.name, SERVICES_NAME_LENGTH);
      trim(serviceName, SERVICES_NAME_LENGTH);
//
//  Loop through all the patterns on this route/service/direction
//
      for(nI = 0; nI < 2; nI++)
      {
        if(ROUTES.DIRECTIONSrecordID[nI] == NO_RECORD)
          continue;
        if(StatusBarAbort())
        {
          bKeepGoing = FALSE;
          goto deallocate;
        }
        DIRECTIONSKey0.recordID = ROUTES.DIRECTIONSrecordID[nI];
        btrieve(B_GETEQUAL, TMS_DIRECTIONS, &DIRECTIONS, &DIRECTIONSKey0, 0);
        strncpy(directionName, DIRECTIONS.longName, DIRECTIONS_LONGNAME_LENGTH);
        trim(directionName, DIRECTIONS_LONGNAME_LENGTH);
//
//  Read all the individual patterns (and nodes) for this service / route / direction
//
        PATTERNSKey2.ROUTESrecordID = ROUTES.recordID;
        PATTERNSKey2.SERVICESrecordID = SERVICES.recordID;
        PATTERNSKey2.directionIndex = nI;
        PATTERNSKey2.PATTERNNAMESrecordID = NO_RECORD;
        PATTERNSKey2.nodeSequence = NO_RECORD;
        rcode2 = btrieve(B_GETGREATER, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
        prevPattern = NO_RECORD;
        while(rcode2 == 0 &&
              PATTERNS.ROUTESrecordID == ROUTES.recordID &&
              PATTERNS.SERVICESrecordID == SERVICES.recordID &&
              PATTERNS.directionIndex == nI)
        {
          if(StatusBarAbort())
          {
            bKeepGoing = FALSE;
            goto deallocate;
          }
          if(PATTERNS.PATTERNNAMESrecordID != prevPattern)
          {
            if(prevPattern != NO_RECORD)
            {
              strcat(outputString, "\r\n");
              _lwrite(hfOutputFile, outputString, strlen(outputString));
            }
            strcpy(outputString, routeNumber);
            strcat(outputString, "\t");
            strcat(outputString, routeName);
            strcat(outputString, "\t");
            strcat(outputString, serviceName);
            strcat(outputString, "\t");
            strcat(outputString, directionName);
            strcat(outputString, "\t");
            PATTERNNAMESKey0.recordID = PATTERNS.PATTERNNAMESrecordID;
            btrieve(B_GETEQUAL, TMS_PATTERNNAMES, &PATTERNNAMES, &PATTERNNAMESKey0, 0);
            strncpy(tempString, PATTERNNAMES.name, PATTERNNAMES_NAME_LENGTH);
            trim(tempString, PATTERNNAMES_NAME_LENGTH);
            strcat(outputString, tempString);
            prevPattern = PATTERNS.PATTERNNAMESrecordID;
          }
          NODESKey0.recordID = PATTERNS.NODESrecordID;
          btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
          strncpy(tempString, NODES.abbrName, NODES_ABBRNAME_LENGTH);
          trim(tempString, NODES_ABBRNAME_LENGTH);
          strcat(outputString, "\t");
          strcat(outputString, tempString);
          rcode2 = btrieve(B_GETNEXT, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
        }
//
//  Dump out the last pattern found on this route/ser/dir
//
        if(prevPattern != NO_RECORD)
        {
          strcat(outputString, "\r\n");
          _lwrite(hfOutputFile, outputString, strlen(outputString));
        }
//
//  Next direction
//
      }  // nI
//
//  Next service
//
      currentService++;
      rcode2 = btrieve(B_GETNEXT, TMS_SERVICES, &SERVICES, &SERVICESKey1, 1);
    }
//
//  Next route
//
    currentRoute++;
    rcode2 = btrieve(B_GETNEXT, TMS_ROUTES, &ROUTES, &ROUTESKey1, 1);
  }
//
//  Open the Trips output file
//
  strcpy(tempString, szDatabaseFileName);
  if((ptr = strrchr(tempString, '\\')) != NULL)
    *ptr = '\0';
  strcat(tempString, "\\ATIS-Trips.txt");
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
//  Loop through the routes
//
  LoadString(hInst, TEXT_117, tempString, TEMPSTRING_LENGTH);
  statbarTotal = numRoutes * numServices;
  currentRoute = 0;
  rcode2 = btrieve(B_GETFIRST, TMS_ROUTES, &ROUTES, &ROUTESKey1, 1);
  while(rcode2 == 0)
  {
    if(StatusBarAbort())
    {
      bKeepGoing = FALSE;
      goto deallocate;
    }
    strncpy(routeNumber, ROUTES.number, ROUTES_NUMBER_LENGTH);
    trim(routeNumber, ROUTES_NUMBER_LENGTH);
    strncpy(routeName, ROUTES.name, ROUTES_NAME_LENGTH);
    trim(routeName, ROUTES_NAME_LENGTH);
//
//  Loop through the services
//
    currentService = 1;
    rcode2 = btrieve(B_GETFIRST, TMS_SERVICES, &SERVICES, &SERVICESKey1, 1);
    while(rcode2 == 0)
    {
      if(StatusBarAbort())
      {
        bKeepGoing = FALSE;
        goto deallocate;
      }
      strncpy(serviceName, SERVICES.name, SERVICES_NAME_LENGTH);
      trim(serviceName, SERVICES_NAME_LENGTH);
//
//  Loop through all the trips on this route/service/direction
//
      for(nI = 0; nI < 2; nI++)
      {
        if(ROUTES.DIRECTIONSrecordID[nI] == NO_RECORD)
          continue;
        if(StatusBarAbort())
        {
          bKeepGoing = FALSE;
          goto deallocate;
        }
        DIRECTIONSKey0.recordID = ROUTES.DIRECTIONSrecordID[nI];
        btrieve(B_GETEQUAL, TMS_DIRECTIONS, &DIRECTIONS, &DIRECTIONSKey0, 0);
        strncpy(directionName, DIRECTIONS.longName, DIRECTIONS_LONGNAME_LENGTH);
        trim(directionName, DIRECTIONS_LONGNAME_LENGTH);
//
//  Get the trips
//
        TRIPSKey1.ROUTESrecordID = ROUTES.recordID;
        TRIPSKey1.SERVICESrecordID = SERVICES.recordID;
        TRIPSKey1.directionIndex = nI;
        TRIPSKey1.tripSequence = NO_TIME;
        rcode2 = btrieve(B_GETGREATEROREQUAL, TMS_TRIPS, &TRIPS, &TRIPSKey1, 1);
        while(rcode2 == 0 &&
              TRIPS.ROUTESrecordID == ROUTES.recordID &&
              TRIPS.SERVICESrecordID == SERVICES.recordID &&
              TRIPS.directionIndex == nI)
        {
          if(StatusBarAbort())
          {
            bKeepGoing = FALSE;
            goto deallocate;
          }
//
//  Build up the output string
//
          strcpy(outputString, routeNumber);
          strcat(outputString, "\t");
          strcat(outputString, routeName);
          strcat(outputString, "\t");
          strcat(outputString, serviceName);
          strcat(outputString, "\t");
          strcat(outputString, directionName);
          strcat(outputString, "\t");
//
//  Pattern name
//
          PATTERNNAMESKey0.recordID = TRIPS.PATTERNNAMESrecordID;
          btrieve(B_GETEQUAL, TMS_PATTERNNAMES, &PATTERNNAMES, &PATTERNNAMESKey0, 0);
          strncpy(tempString, PATTERNNAMES.name, PATTERNNAMES_NAME_LENGTH);
          tempString[PATTERNNAMES_NAME_LENGTH] = '\0';
          strcat(outputString, tempString);
//
//  Nodes and node times
//
//  Generate the trip
//
          numNodes = GenerateTrip(TRIPS.ROUTESrecordID, TRIPS.SERVICESrecordID,
                TRIPS.directionIndex, TRIPS.PATTERNNAMESrecordID,
                TRIPS.timeAtMLP, GENERATETRIP_FLAG_DISPLAYERRORS, &GTResults);
//
//  Dump out the times
//
          for(nJ = 0; nJ < numNodes; nJ++)
          {
            sprintf(tempString, "\t%ld", GTResults.tripTimes[nJ]);
            strcat(outputString, tempString);
          }
          strcat(outputString, "\r\n");
//
//  Write it out
//
          _lwrite(hfOutputFile, outputString, strlen(outputString));
//
//  Next trip
//
          rcode2 = btrieve(B_GETNEXT, TMS_TRIPS, &TRIPS, &TRIPSKey1, 1);
        }
//
//  Next direction
//
      }  // nI
//
//  Next service
//
      currentService++;
      rcode2 = btrieve(B_GETNEXT, TMS_SERVICES, &SERVICES, &SERVICESKey1, 1);
    }
//
//  Next route
//
    currentRoute++;
    rcode2 = btrieve(B_GETNEXT, TMS_ROUTES, &ROUTES, &ROUTESKey1, 1);
  }
  bKeepGoing = TRUE;
//
//  Free allocated memory
//
  deallocate:
    _lclose(hfOutputFile);
    StatusBarEnd();
//
//  Let him know
//
  strcpy(tempString, "ATIS files were written to:\n");
  strcpy(szarString, szDatabaseFileName);
  if((ptr = strrchr(szarString, '\\')) != NULL)
    *ptr = '\0';
  strcat(tempString, szarString);
  MessageBox(hWndMain, tempString, TMS, MB_OK);

  if(!bKeepGoing)
    return(FALSE);
//
//  All done
//
  return(TRUE);
}


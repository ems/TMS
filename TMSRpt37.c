//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
//
//  TMSRPT37() - Time Checker Worksheet
//
//
//  From QETXT.INI:
//
//  [TMSRPT37]
//  FILE=TMSRPT37.txt
//  FLN=0
//  TT=Tab
//  Charset=ANSI
//  DS=.
//  FIELD1=Sequence,NUMERIC,7,0,7,0,
//  FIELD2=TripNumber,NUMERIC,7,0,7,0,
//  FIELD3=Service,VARCHAR,32,0,32,0,
//  FIELD4=Route,VARCHAR,75,0,75,0,
//  FIELD5=Direction,VARCHAR,16,0,16,0,
//  FIELD6=BlockInfo,VARCHAR,8,0,8,0,
//  FIELD7=POPIText,VARCHAR,4,0,4,0,
//  FIELD8=NodeTime,VARCHAR,8,0,8,0,
//  FIELD9=AbbrName,VARCHAR,4,0,4,0,
//  FIELD10=LongName,VARCHAR,8,0,8,0,
//  FIELD11=Intersection,VARCHAR,64,0,64,0,
//
#include "TMSHeader.h"
#include <math.h>

#define NUMCOLS 6

BOOL FAR TMSRPT37(TMSRPTPassedDataDef *pPassedData)
{
  GetConnectionTimeDef GCTData;
  GenerateTripDef GTResults;
  REPORTPARMSDef  REPORTPARMS;
  HANDLE hOutputFile;
  DWORD  dwBytesWritten;
  float  distance;
  BOOL  bKeepGoing = FALSE;
  BOOL  bFound = TRUE;
  BOOL  bRC;
  char  outputString[512];
  char  routeNumberAndName[ROUTES_NUMBER_LENGTH + 3 + ROUTES_NAME_LENGTH + 1];
  char  serviceName[SERVICES_NAME_LENGTH + 1];
  char  directionName[DIRECTIONS_LONGNAME_LENGTH + 1];
  char  szPOTime[8];
  char  szPITime[8];
  char  szAbbrName[NODES_ABBRNAME_LENGTH + 1];
  char  szLongName[NODES_LONGNAME_LENGTH + 1];
  char  szIntersection[NODES_INTERSECTION_LENGTH + 1];
  long  statbarTotal;
  long  deadheadTime;
  int   nI;
  int   nJ;
  int   nK;
  int   rcode2;
  int   seq;
  int   tripTimeIndex;

//
//  Warn him about trip numbers
//
  LoadString(hInst, TEXT_210, tempString, sizeof(tempString));
  MessageBeep(MB_ICONQUESTION);
  if(MessageBox(NULL, tempString, TMS, MB_ICONQUESTION | MB_YESNO) == IDNO)
  {
    return(FALSE);
  }
//
//  Open the output file
//
  strcpy(tempString, szReportsTempFolder);
  strcat(tempString, "\\TMSRPT37.txt");
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
//  Ok - let's go
// 
  recycle:
    pPassedData->nReportNumber = 36;
    pPassedData->numDataFiles = 1;
//
//  See what he wants
//
  REPORTPARMS.nReportNumber = pPassedData->nReportNumber;
  REPORTPARMS.ROUTESrecordID = m_RouteRecordID;
  REPORTPARMS.SERVICESrecordID = m_ServiceRecordID;
  REPORTPARMS.COMMENTSrecordID = NO_RECORD;
  REPORTPARMS.flags = RPFLAG_ROUTES | RPFLAG_SERVICES | RPFLAG_COMMENTS | RPFLAG_SPECIFICTRIPNUMBER;
  bRC = DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_RPTPARMS),
        hWndMain, (DLGPROC)RPTPARMSMsgProc, (LPARAM)&REPORTPARMS);
  if(!bRC)
  {
    return(FALSE);
  }
//
//  See if he asked for a specific trip number, and if it's valid
//
  if(REPORTPARMS.tripNumber != NO_RECORD)
  {
    bFound = FALSE;
    rcode2 = btrieve(B_GETFIRST, TMS_TRIPS, &TRIPS, &TRIPSKey0, 0);
    while(rcode2 == 0)
    {
      if(TRIPS.tripNumber == REPORTPARMS.tripNumber)
      {
        bFound = TRUE;
        break;
      }
      rcode2 = btrieve(B_GETNEXT, TMS_TRIPS, &TRIPS, &TRIPSKey0, 0);
    }
    if(!bFound)
    {
      MessageBeep(MB_ICONSTOP);
      LoadString(hInst, ERROR_353, tempString, TEMPSTRING_LENGTH);
      MessageBox(NULL, tempString, TMS, MB_ICONSTOP | MB_OK);
      goto recycle;
    }
//
//  Got the trip - deal with it
//
    seq = 0;
    GenerateTrip(TRIPS.ROUTESrecordID, TRIPS.SERVICESrecordID,
          TRIPS.directionIndex, TRIPS.PATTERNNAMESrecordID,
          TRIPS.timeAtMLP, GENERATETRIP_FLAG_DISPLAYERRORS, &GTResults);
//
//  Is this a pull-out?  If so, dump out a record.
//
    if(TRIPS.standard.POGNODESrecordID != NO_RECORD)
    {
      GCTData.fromPATTERNNAMESrecordID = TRIPS.PATTERNNAMESrecordID;
      GCTData.toPATTERNNAMESrecordID = TRIPS.PATTERNNAMESrecordID;
      GCTData.fromROUTESrecordID = TRIPS.standard.RGRPROUTESrecordID;
      GCTData.fromSERVICESrecordID = TRIPS.standard.SGRPSERVICESrecordID;
      GCTData.toROUTESrecordID = TRIPS.standard.RGRPROUTESrecordID;
      GCTData.toSERVICESrecordID = TRIPS.standard.SGRPSERVICESrecordID;
      GCTData.fromNODESrecordID = TRIPS.standard.POGNODESrecordID;
      GCTData.toNODESrecordID = GTResults.firstNODESrecordID;
      GCTData.timeOfDay = GTResults.firstNodeTime;
      deadheadTime = GetConnectionTime(GCT_FLAG_DEADHEADTIME, &GCTData, &distance);
      distance = (float)fabs((double)distance);
      if(deadheadTime == NO_TIME)
      {
        deadheadTime = 0;
      }
      strcpy(szPOTime, Tchar(GTResults.firstNodeTime - deadheadTime));
      sprintf(outputString, "%d\t%ld\t%s\t%s\t%s\t%ld\tP/O\t%s\t",
            seq++, TRIPS.tripNumber, serviceName, routeNumberAndName,
            directionName, TRIPS.standard.blockNumber, szPOTime);
      NODESKey0.recordID = TRIPS.standard.POGNODESrecordID;
      btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
      strncpy(szAbbrName, NODES.abbrName, NODES_ABBRNAME_LENGTH);
      trim(szAbbrName, NODES_ABBRNAME_LENGTH);
      strcat(outputString, szAbbrName);
      strcat(outputString, "\t");
      strncpy(szLongName, NODES.longName, NODES_LONGNAME_LENGTH);
      trim(szLongName, NODES_LONGNAME_LENGTH);
      strcat(outputString, szLongName);
      strcat(outputString, "\t");
      strncpy(szIntersection, NODES.intersection, NODES_INTERSECTION_LENGTH);
      trim(szIntersection, NODES_INTERSECTION_LENGTH);
      strcat(outputString, szIntersection);
      strcat(outputString, "\r\n");
      WriteFile(hOutputFile, (LPCVOID *)outputString,
            strlen(outputString), &dwBytesWritten, NULL);
    }
//
//  Go through the trip
//
//  Get the pattern nodes
//
    PATTERNSKey2.ROUTESrecordID = TRIPS.ROUTESrecordID;
    PATTERNSKey2.SERVICESrecordID = TRIPS.SERVICESrecordID;
    PATTERNSKey2.directionIndex = TRIPS.directionIndex;
    PATTERNSKey2.PATTERNNAMESrecordID = TRIPS.PATTERNNAMESrecordID;
    PATTERNSKey2.nodeSequence = NO_RECORD;
    rcode2 = btrieve(B_GETGREATER, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
    tripTimeIndex = 0;
    while(rcode2 == 0 &&
          PATTERNS.ROUTESrecordID == TRIPS.ROUTESrecordID &&
          PATTERNS.SERVICESrecordID == TRIPS.SERVICESrecordID &&
          PATTERNS.directionIndex == TRIPS.directionIndex &&
          PATTERNSKey2.PATTERNNAMESrecordID == TRIPS.PATTERNNAMESrecordID)
    {
      NODESKey0.recordID = PATTERNS.NODESrecordID;
      btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
//
//  Build the output string
//
//  Sequence, service, route, direction
//
      sprintf(outputString, "%d\t%ld\t%s\t%s\t%s\t%ld\t\t",
            seq++, TRIPS.tripNumber, serviceName, routeNumberAndName,
            directionName, TRIPS.standard.blockNumber);
//
//  Node time
//
      if(!(PATTERNS.flags & PATTERNS_FLAG_BUSSTOP))
      {
        strcat(outputString, Tchar(GTResults.tripTimes[tripTimeIndex]));
        tripTimeIndex++;
      }
      strcat(outputString, "\t");
//
//  Node abbreviation
//
      strncpy(tempString, NODES.abbrName, NODES_ABBRNAME_LENGTH);
      trim(tempString, NODES_ABBRNAME_LENGTH);
      strcat(outputString, tempString);
      strcat(outputString, "\t");
//
//  Node long name
//
      strncpy(tempString, NODES.longName, NODES_LONGNAME_LENGTH);
      trim(tempString, NODES_LONGNAME_LENGTH);
      strcat(outputString, tempString);
      strcat(outputString, "\t");
//
//  Node intersection
//
      strncpy(tempString, NODES.intersection, NODES_INTERSECTION_LENGTH);
      trim(tempString, NODES_INTERSECTION_LENGTH);
      strcat(outputString, tempString);
      strcat(outputString, "\r\n");
//
//  Write it out
//
      WriteFile(hOutputFile, (LPCVOID *)outputString,
            strlen(outputString), &dwBytesWritten, NULL);
//
//  Get the next node in the pattern
//
      rcode2 = btrieve(B_GETNEXT, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
    }  // while on TMS_PATTERNS
//
//  Is this a pull-in?  If so, dump out a record.
//
    if(TRIPS.standard.PIGNODESrecordID != NO_RECORD)
    {
      GCTData.fromPATTERNNAMESrecordID = TRIPS.PATTERNNAMESrecordID;
      GCTData.toPATTERNNAMESrecordID = TRIPS.PATTERNNAMESrecordID;
      GCTData.fromROUTESrecordID = TRIPS.standard.RGRPROUTESrecordID;
      GCTData.fromSERVICESrecordID = TRIPS.standard.SGRPSERVICESrecordID;
      GCTData.toROUTESrecordID = TRIPS.standard.RGRPROUTESrecordID;
      GCTData.toSERVICESrecordID = TRIPS.standard.SGRPSERVICESrecordID;
      GCTData.fromNODESrecordID = GTResults.lastNODESrecordID;
      GCTData.toNODESrecordID = TRIPS.standard.PIGNODESrecordID;
      GCTData.timeOfDay = GTResults.lastNodeTime;
      deadheadTime = GetConnectionTime(GCT_FLAG_DEADHEADTIME, &GCTData, &distance);
      distance = (float)fabs((double)distance);
      if(deadheadTime == NO_TIME)
      {
        deadheadTime = 0;
      }
      strcpy(szPITime, Tchar(GTResults.lastNodeTime + deadheadTime));
      sprintf(outputString, "%d\t%ld\t%s\t%s\t%s\t%ld\tP/I\t%s\t",
            seq++, TRIPS.tripNumber, serviceName, routeNumberAndName,
            directionName, TRIPS.standard.blockNumber, szPITime);
      NODESKey0.recordID = TRIPS.standard.PIGNODESrecordID;
      btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
      strncpy(szAbbrName, NODES.abbrName, NODES_ABBRNAME_LENGTH);
      trim(szAbbrName, NODES_ABBRNAME_LENGTH);
      strcat(outputString, szAbbrName);
      strcat(outputString, "\t");
      strncpy(szLongName, NODES.longName, NODES_LONGNAME_LENGTH);
      trim(szLongName, NODES_LONGNAME_LENGTH);
      strcat(outputString, szLongName);
      strcat(outputString, "\t");
      strncpy(szIntersection, NODES.intersection, NODES_INTERSECTION_LENGTH);
      trim(szIntersection, NODES_INTERSECTION_LENGTH);
      strcat(outputString, szIntersection);
      strcat(outputString, "\r\n");
      WriteFile(hOutputFile, (LPCVOID *)outputString,
            strlen(outputString), &dwBytesWritten, NULL);
    }
    bKeepGoing = TRUE;
    goto deallocate;
  }
//
//  Check the list of routes and services
//
  if(REPORTPARMS.numRoutes == 0 || REPORTPARMS.numServices == 0)
  {
    goto deallocate;
  }
//
//  Loop through the routes
//
  LoadString(hInst, TEXT_117, tempString, TEMPSTRING_LENGTH);
  StatusBarStart(hWndMain, tempString);
  statbarTotal = REPORTPARMS.numRoutes * REPORTPARMS.numServices;
  seq = 0;
  for(nI = 0; nI < REPORTPARMS.numRoutes; nI++)
  {
    if(StatusBarAbort())
    {
      bKeepGoing = FALSE;
      goto deallocate;
    }
    ROUTESKey0.recordID = REPORTPARMS.pRouteList[nI];
    rcode2 = btrieve(B_GETEQUAL, TMS_ROUTES, &ROUTES, &ROUTESKey0, 0);
    if(rcode2 != 0)
    {
      continue;
    }
    strncpy(routeNumberAndName, ROUTES.number, ROUTES_NUMBER_LENGTH);
    trim(routeNumberAndName, ROUTES_NUMBER_LENGTH);
    strcat(routeNumberAndName, " - ");
    strncpy(tempString, ROUTES.name, ROUTES_NAME_LENGTH);
    trim(tempString, ROUTES_NAME_LENGTH);
    strcat(routeNumberAndName, tempString);
//
//  Loop through the services
//
    for(nJ = 0; nJ < REPORTPARMS.numServices; nJ++)
    {
      if(StatusBarAbort())
      {
        bKeepGoing = FALSE;
        goto deallocate;
      }
      SERVICESKey0.recordID = REPORTPARMS.pServiceList[nJ];
      rcode2 = btrieve(B_GETEQUAL, TMS_SERVICES, &SERVICES, &SERVICESKey0, 0);
      if(rcode2 != 0)
      {
        continue;
      }
      strncpy(serviceName, SERVICES.name, SERVICES_NAME_LENGTH);
      trim(serviceName, SERVICES_NAME_LENGTH);
      LoadString(hInst, TEXT_118, szarString, sizeof(szarString));
      sprintf(tempString, szarString, routeNumberAndName, serviceName);
      StatusBarText(tempString);
      StatusBar((long)(nI * REPORTPARMS.numServices + nJ), (long)statbarTotal);
      if(StatusBarAbort())
      {
        bKeepGoing = FALSE;
        goto deallocate;
      }
//
//  Loop through the directions
//
      for(nK = 0; nK < 2; nK++)
      {
        if(StatusBarAbort())
        {
          bKeepGoing = FALSE;
          goto deallocate;
        }
        if(ROUTES.DIRECTIONSrecordID[nK] == NO_RECORD)
        {
          continue;
        }
        DIRECTIONSKey0.recordID = ROUTES.DIRECTIONSrecordID[nK];
        rcode2 = btrieve(B_GETEQUAL, TMS_DIRECTIONS, &DIRECTIONS, &DIRECTIONSKey0, 0);
        if(rcode2 != 0)
        {
          continue;
        }
        strncpy(directionName, DIRECTIONS.longName, DIRECTIONS_LONGNAME_LENGTH);
        trim(directionName, DIRECTIONS_LONGNAME_LENGTH);
//
//  Get the trips
//
        TRIPSKey1.ROUTESrecordID = ROUTES.recordID;
        TRIPSKey1.SERVICESrecordID = SERVICES.recordID;
        TRIPSKey1.directionIndex = nK;
        TRIPSKey1.tripSequence = NO_TIME;
        rcode2 = btrieve(B_GETGREATEROREQUAL, TMS_TRIPS, &TRIPS, &TRIPSKey1, 1);
        while(rcode2 == 0 &&
              TRIPS.ROUTESrecordID == ROUTES.recordID &&
              TRIPS.SERVICESrecordID == SERVICES.recordID &&
              TRIPS.directionIndex == nK)
        {
          GenerateTrip(TRIPS.ROUTESrecordID, TRIPS.SERVICESrecordID,
                TRIPS.directionIndex, TRIPS.PATTERNNAMESrecordID,
                TRIPS.timeAtMLP, GENERATETRIP_FLAG_DISPLAYERRORS, &GTResults);
//
//  Is this a pull-out?  If so, dump out a record.
//
          if(TRIPS.standard.POGNODESrecordID != NO_RECORD)
          {
            GCTData.fromPATTERNNAMESrecordID = TRIPS.PATTERNNAMESrecordID;
            GCTData.toPATTERNNAMESrecordID = TRIPS.PATTERNNAMESrecordID;
            GCTData.fromROUTESrecordID = TRIPS.standard.RGRPROUTESrecordID;
            GCTData.fromSERVICESrecordID = TRIPS.standard.SGRPSERVICESrecordID;
            GCTData.toROUTESrecordID = TRIPS.standard.RGRPROUTESrecordID;
            GCTData.toSERVICESrecordID = TRIPS.standard.SGRPSERVICESrecordID;
            GCTData.fromNODESrecordID = TRIPS.standard.POGNODESrecordID;
            GCTData.toNODESrecordID = GTResults.firstNODESrecordID;
            GCTData.timeOfDay = GTResults.firstNodeTime;
            deadheadTime = GetConnectionTime(GCT_FLAG_DEADHEADTIME, &GCTData, &distance);
            distance = (float)fabs((double)distance);
            if(deadheadTime == NO_TIME)
            {
              deadheadTime = 0;
            }
            strcpy(szPOTime, Tchar(GTResults.firstNodeTime - deadheadTime));
            sprintf(outputString, "%d\t%ld\t%s\t%s\t%s\t%ld\tP/O\t%s\t",
                  seq++, TRIPS.tripNumber, serviceName, routeNumberAndName,
                  directionName, TRIPS.standard.blockNumber, szPOTime);
            NODESKey0.recordID = TRIPS.standard.POGNODESrecordID;
            btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
            strncpy(szAbbrName, NODES.abbrName, NODES_ABBRNAME_LENGTH);
            trim(szAbbrName, NODES_ABBRNAME_LENGTH);
            strcat(outputString, szAbbrName);
            strcat(outputString, "\t");
            strncpy(szLongName, NODES.longName, NODES_LONGNAME_LENGTH);
            trim(szLongName, NODES_LONGNAME_LENGTH);
            strcat(outputString, szLongName);
            strcat(outputString, "\t");
            strncpy(szIntersection, NODES.intersection, NODES_INTERSECTION_LENGTH);
            trim(szIntersection, NODES_INTERSECTION_LENGTH);
            strcat(outputString, szIntersection);
            strcat(outputString, "\r\n");
            WriteFile(hOutputFile, (LPCVOID *)outputString,
                  strlen(outputString), &dwBytesWritten, NULL);
          }
//
//  Go through the trip
//
//  Get the pattern nodes
//
          PATTERNSKey2.ROUTESrecordID = ROUTES.recordID;
          PATTERNSKey2.SERVICESrecordID = SERVICES.recordID;
          PATTERNSKey2.directionIndex = nK;
          PATTERNSKey2.PATTERNNAMESrecordID = TRIPS.PATTERNNAMESrecordID;
          PATTERNSKey2.nodeSequence = NO_RECORD;
          rcode2 = btrieve(B_GETGREATER, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
          tripTimeIndex = 0;
          while(rcode2 == 0 &&
                PATTERNS.ROUTESrecordID == ROUTES.recordID &&
                PATTERNS.SERVICESrecordID == SERVICES.recordID &&
                PATTERNS.directionIndex == nK &&
                PATTERNSKey2.PATTERNNAMESrecordID == TRIPS.PATTERNNAMESrecordID)
          {
            NODESKey0.recordID = PATTERNS.NODESrecordID;
            btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
//
//  Build the output string
//
//  Sequence, service, route, direction
//
            sprintf(outputString, "%d\t%ld\t%s\t%s\t%s\t%ld\t\t",
                  seq++, TRIPS.tripNumber, serviceName, routeNumberAndName,
                  directionName, TRIPS.standard.blockNumber);
//
//  Node time
//
            if(!(PATTERNS.flags & PATTERNS_FLAG_BUSSTOP))
            {
              strcat(outputString, Tchar(GTResults.tripTimes[tripTimeIndex]));
              tripTimeIndex++;
            }
            strcat(outputString, "\t");
//
//  Node abbreviation
//
            strncpy(tempString, NODES.abbrName, NODES_ABBRNAME_LENGTH);
            trim(tempString, NODES_ABBRNAME_LENGTH);
            strcat(outputString, tempString);
            strcat(outputString, "\t");
//
//  Node long name
//
            strncpy(tempString, NODES.longName, NODES_LONGNAME_LENGTH);
            trim(tempString, NODES_LONGNAME_LENGTH);
            strcat(outputString, tempString);
            strcat(outputString, "\t");
//
//  Node intersection
//
            strncpy(tempString, NODES.intersection, NODES_INTERSECTION_LENGTH);
            trim(tempString, NODES_INTERSECTION_LENGTH);
            strcat(outputString, tempString);
            strcat(outputString, "\r\n");
//
//  Write it out
//
            WriteFile(hOutputFile, (LPCVOID *)outputString,
                  strlen(outputString), &dwBytesWritten, NULL);
//
//  Get the next node in the pattern
//
            rcode2 = btrieve(B_GETNEXT, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
          }  // while on TMS_PATTERNS
//
//  Is this a pull-in?  If so, dump out a record.
//
          if(TRIPS.standard.PIGNODESrecordID != NO_RECORD)
          {
            GCTData.fromPATTERNNAMESrecordID = TRIPS.PATTERNNAMESrecordID;
            GCTData.toPATTERNNAMESrecordID = TRIPS.PATTERNNAMESrecordID;
            GCTData.fromROUTESrecordID = TRIPS.standard.RGRPROUTESrecordID;
            GCTData.fromSERVICESrecordID = TRIPS.standard.SGRPSERVICESrecordID;
            GCTData.toROUTESrecordID = TRIPS.standard.RGRPROUTESrecordID;
            GCTData.toSERVICESrecordID = TRIPS.standard.SGRPSERVICESrecordID;
            GCTData.fromNODESrecordID = GTResults.lastNODESrecordID;
            GCTData.toNODESrecordID = TRIPS.standard.PIGNODESrecordID;
            GCTData.timeOfDay = GTResults.lastNodeTime;
            deadheadTime = GetConnectionTime(GCT_FLAG_DEADHEADTIME, &GCTData, &distance);
            distance = (float)fabs((double)distance);
            if(deadheadTime == NO_TIME)
            {
              deadheadTime = 0;
            }
            strcpy(szPITime, Tchar(GTResults.lastNodeTime + deadheadTime));
            sprintf(outputString, "%d\t%ld\t%s\t%s\t%s\t%ld\tP/I\t%s\t",
                  seq++, TRIPS.tripNumber, serviceName, routeNumberAndName,
                  directionName, TRIPS.standard.blockNumber, szPITime);
            NODESKey0.recordID = TRIPS.standard.PIGNODESrecordID;
            btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
            strncpy(szAbbrName, NODES.abbrName, NODES_ABBRNAME_LENGTH);
            trim(szAbbrName, NODES_ABBRNAME_LENGTH);
            strcat(outputString, szAbbrName);
            strcat(outputString, "\t");
            strncpy(szLongName, NODES.longName, NODES_LONGNAME_LENGTH);
            trim(szLongName, NODES_LONGNAME_LENGTH);
            strcat(outputString, szLongName);
            strcat(outputString, "\t");
            strncpy(szIntersection, NODES.intersection, NODES_INTERSECTION_LENGTH);
            trim(szIntersection, NODES_INTERSECTION_LENGTH);
            strcat(outputString, szIntersection);
            strcat(outputString, "\r\n");
            WriteFile(hOutputFile, (LPCVOID *)outputString,
                  strlen(outputString), &dwBytesWritten, NULL);
          }
//
//  Get the next trip
//
          rcode2 = btrieve(B_GETNEXT, TMS_TRIPS, &TRIPS, &TRIPSKey1, 1);
        }  // while on TMS_TRIPS
      }  // nK
    }  // nJ
  }  // nI
  bKeepGoing = TRUE;
//
//  Free allocated memory
//
  deallocate:
    CloseHandle(hOutputFile);
    StatusBarEnd();
    if(!bKeepGoing)
    {
      return(FALSE);
    }
//
//  All done
//
  return(TRUE);
}


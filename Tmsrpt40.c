//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
#include "TMSHeader.h"

//
//  TMSRPT40() - Connections analysis
//
//  From QETXT.INI:
//
//
//  [TMSRPT40]
//  FILE=TMSRPT40.TXT
//  FLN=0
//  TT=Tab
//  FIELD1=Sequence,NUMERIC,7,0,7,0,
//  FIELD2=FromNodeName,VARCHAR,8,0,8,0,
//  FIELD3=ToNodeName,VARCHAR,8,0,8,0,
//  FIELD4=Text,VARCHAR,256,0,256,0,
//
//  [TMSRPT40RAW]
//
//  FILE=TMSRPT40RAW.TXT
//  FLN=0
//  TT=Tab
//  FIELD1=FromNodeName,VARCHAR,4,0,4,0,
//  FIELD2=ToNodeName,VARCHAR,4,0,4,0,
//  FIELD3=RouteNumber,VARCHAR,8,0,8,0,
//  FIELD4=DirectionAbbr,VARCHAR,2,0,2,0,
//  FIELD5=ServiceName,VARCHAR,16,0,16,0,
//  FIELD6=PatternName,VARCHAR,16,0,16,0,
//  FIELD7=DepartureTime,NUMERIC,7,0,7,0,
//  FIELD8=ArrivalTime,NUMERIC,7,0,7,0,
//


typedef struct TMSRPT40NODESStruct
{
  long from;
  long to;
} TMSRPT40NODESDef;

typedef struct TMSRPT40TRIPSStruct
{
  long TRIPSrecordID;
  long actualTimeOfDay;
  long nodePositionInTrip;
} TMSRPT40TRIPSDef;

int TMSRPT40sort_nodes( const void *a, const void *b)
{
  char firstNodePair[NODES_ABBRNAME_LENGTH * 2 + 1];
  char secondNodePair[NODES_ABBRNAME_LENGTH * 2 + 1];

  TMSRPT40NODESDef *firstPair;
  TMSRPT40NODESDef *secondPair;

  firstPair = (TMSRPT40NODESDef *)a;
  secondPair = (TMSRPT40NODESDef *)b;

  NODESKey0.recordID = firstPair->from;
  btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
  strncpy(firstNodePair, NODES.abbrName, NODES_ABBRNAME_LENGTH);
  NODESKey0.recordID = firstPair->to;
  btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
  strncpy(&firstNodePair[NODES_ABBRNAME_LENGTH], NODES.abbrName, NODES_ABBRNAME_LENGTH);
  firstNodePair[NODES_ABBRNAME_LENGTH * 2] = '\0';

  NODESKey0.recordID = secondPair->from;
  btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
  strncpy(secondNodePair, NODES.abbrName, NODES_ABBRNAME_LENGTH);
  NODESKey0.recordID = secondPair->to;
  btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
  strncpy(&secondNodePair[NODES_ABBRNAME_LENGTH], NODES.abbrName, NODES_ABBRNAME_LENGTH);
  secondNodePair[NODES_ABBRNAME_LENGTH * 2] = '\0';

  return(strcmp(firstNodePair, secondNodePair));
}

int TMSRPT40sort_trips( const void *a, const void *b)
{
  TMSRPT40TRIPSDef *first;
  TMSRPT40TRIPSDef *second;

  first = (TMSRPT40TRIPSDef *)a;
  second = (TMSRPT40TRIPSDef *)b;

  return(first->actualTimeOfDay < second->actualTimeOfDay ?
    -1 : first->actualTimeOfDay > second->actualTimeOfDay ? 1 : 0);
}

#define TMSRPT40_MAXNODES 500

BOOL FAR TMSRPT40(TMSRPTPassedDataDef *pPassedData)
{
  REPORTPARMSDef  REPORTPARMS;
  TMSRPT40NODESDef NODESrecordIDs[TMSRPT40_MAXNODES];
  TMSRPT40TRIPSDef TRIPData[TMSRPT40_MAXNODES];
  HANDLE hOutputFile;
  HANDLE hOutputFileRaw;
  DWORD  dwBytesWritten;
  double distance;
  BOOL  bKeepGoing = FALSE;
  char  outputString[512];
  char  outputStringRaw[512];
  long  fromNODESrecordID;
  long  toNODESrecordID;
  long  arrivalTime;
  float longitude[2];
  float latitude[2];
  BOOL  bRC;
  BOOL  bFound;
  BOOL  bDoneTitle;
  char  fromNodeName[NODES_ABBRNAME_LENGTH * 2 + 1];
  char  toNodeName[NODES_ABBRNAME_LENGTH * 2 + 1];
  long  previousPATTERNNAMESrecordID;
  long  previousNODESrecordID;
  long  currentNodePositionInTrip;
  long  actualTime;
  int   nI;
  int   nJ;
  int   nK;
  int   nL;
  int   rcode2;
  int   seqNum;
  int   numConnections;
  int   numEntries;

//
//  Ok - let's go
// 
  pPassedData->nReportNumber = 39;
  pPassedData->numDataFiles = 1;
//
//  See what he wants
//
  REPORTPARMS.nReportNumber = pPassedData->nReportNumber;
  REPORTPARMS.ROUTESrecordID = m_RouteRecordID;
  REPORTPARMS.SERVICESrecordID = m_ServiceRecordID;
  REPORTPARMS.COMMENTSrecordID = NO_RECORD;
  REPORTPARMS.pRouteList = NULL;
  REPORTPARMS.pServiceList = NULL;
  REPORTPARMS.flags = RPFLAG_ROUTES | RPFLAG_SERVICES | RPFLAG_COMMENTS;

  bRC = DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_RPTPARMS),
        hWndMain, (DLGPROC)RPTPARMSMsgProc, (LPARAM)&REPORTPARMS);
  if(!bRC)
  {
    return(FALSE);
  }
//
//  Check the list of routes and services
//
  if(REPORTPARMS.numRoutes == 0 || REPORTPARMS.numServices == 0)
  {
    goto deallocate;
  }
//
//  Open the regular output file
//
  strcpy(tempString, szReportsTempFolder);
  strcat(tempString, "\\TMSRPT40.txt");
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
//  Open the raw output file
//
  strcpy(tempString, szReportsTempFolder);
  strcat(tempString, "\\TMSRPT40RAW.txt");
  hOutputFileRaw = CreateFile(tempString, GENERIC_WRITE, 0, NULL,
        CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
  if(hOutputFileRaw == INVALID_HANDLE_VALUE)
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
//  Build the list of node pairs (more or less nick'd from PICKCONN
//
  numConnections = 0;
  for(nI = 0; nI < REPORTPARMS.numRoutes; nI++)
  {
    if(StatusBarAbort())
    {
      bKeepGoing = FALSE;
      goto deallocate;
    }
    ROUTESKey0.recordID = REPORTPARMS.pRouteList[nI];
    btrieve(B_GETEQUAL, TMS_ROUTES, &ROUTES, &ROUTESKey0, 0);
    for(nJ = 0; nJ < REPORTPARMS.numServices; nJ++)
    {
      if(StatusBarAbort())
      {
        bKeepGoing = FALSE;
        goto deallocate;
      }
      for(nK = 0; nK < 2; nK++)
      {
        if(ROUTES.DIRECTIONSrecordID[nK] == NO_RECORD)
          continue;
        PATTERNSKey2.ROUTESrecordID = REPORTPARMS.pRouteList[nI];
        PATTERNSKey2.SERVICESrecordID = REPORTPARMS.pServiceList[nJ];
        PATTERNSKey2.directionIndex = nK;
        PATTERNSKey2.PATTERNNAMESrecordID = basePatternRecordID;
        PATTERNSKey2.nodeSequence = 0;
        rcode2 = btrieve(B_GETGREATEROREQUAL, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
//
//  Set up unique from/to node combinations.  This section of code
//  assumes that ADDPATTERN has done its job and not permitted a
//  bus stop to be the first or last node in a pattern.
//
        if(rcode2 == 0 &&
              PATTERNS.ROUTESrecordID == REPORTPARMS.pRouteList[nI] &&
              PATTERNS.SERVICESrecordID == REPORTPARMS.pServiceList[nJ] &&
              PATTERNS.directionIndex == nK)
        {
          fromNODESrecordID = PATTERNS.NODESrecordID;
          rcode2 = btrieve(B_GETNEXT, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
          while(rcode2 == 0 &&
                PATTERNS.ROUTESrecordID == REPORTPARMS.pRouteList[nI] &&
                PATTERNS.SERVICESrecordID == REPORTPARMS.pServiceList[nJ] &&
                PATTERNS.directionIndex == nK)
          {
            if(!(PATTERNS.flags & PATTERNS_FLAG_BUSSTOP))
            {
              toNODESrecordID = PATTERNS.NODESrecordID;
              for(bFound = FALSE, nL = 0; nL < numConnections; nL++)
              {
                if(fromNODESrecordID == NODESrecordIDs[nL].from &&
                      toNODESrecordID == NODESrecordIDs[nL].to)
                {
                  bFound = TRUE;
                  break;
                }
              }
              if(!bFound)
              {
                NODESrecordIDs[numConnections].from = fromNODESrecordID;
                NODESrecordIDs[numConnections++].to = toNODESrecordID;
                if(numConnections >= TMSRPT40_MAXNODES)
                  break;
              }
              fromNODESrecordID = toNODESrecordID;
            }
            previousPATTERNNAMESrecordID = PATTERNS.PATTERNNAMESrecordID;
            rcode2 = btrieve(B_GETNEXT, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
            if(PATTERNS.PATTERNNAMESrecordID != previousPATTERNNAMESrecordID)
            {
              fromNODESrecordID = PATTERNS.NODESrecordID;
              rcode2 = btrieve(B_GETNEXT, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
            }
          }
        }
      }
    }
  }
//
//  Sort the connection pairs
//
  qsort((void *)NODESrecordIDs, numConnections, sizeof(TMSRPT40NODESDef), TMSRPT40sort_nodes);

//
//  Cycle through all the connection pairs
//
  seqNum = 1;
  for(nI = 0; nI < numConnections; nI++)
  {
    if(StatusBarAbort())
    {
      bKeepGoing = FALSE;
      goto deallocate;
    }
//
//  From node
//
    NODESKey0.recordID = NODESrecordIDs[nI].from;
    btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
    strncpy(fromNodeName, NODES.abbrName, NODES_ABBRNAME_LENGTH);
    trim(fromNodeName, NODES_ABBRNAME_LENGTH);
    longitude[0] = NODES.longitude;
    latitude[0] = NODES.latitude;
//
//  To node
//
    NODESKey0.recordID = NODESrecordIDs[nI].to;
    btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
    strncpy(toNodeName, NODES.abbrName, NODES_ABBRNAME_LENGTH);
    trim(toNodeName, NODES_ABBRNAME_LENGTH);
    longitude[1] = NODES.longitude;
    latitude[1] = NODES.latitude;
//
//  Display it
//
    sprintf(tempString, "%4s - %4s", fromNodeName, toNodeName);
    StatusBarText(tempString);
    StatusBar((long)(nI + 1), (long)numConnections);
//
//  First, dump out the connections table entries for this pair under
//  the following rules: a) the entry must be a running time, and b)
//  B->A connections will only be listed if the 2-way flag is in effect.
//
//
//  Possibly From-To and To-From
//
    bDoneTitle = FALSE;
    for(nJ = 0; nJ < 2; nJ++)
    {
      if(nJ == 0)
      {
        fromNODESrecordID = NODESrecordIDs[nI].from;
        toNODESrecordID = NODESrecordIDs[nI].to;
      }
      else
      {
        fromNODESrecordID = NODESrecordIDs[nI].to;
        toNODESrecordID = NODESrecordIDs[nI].from;
      }
      CONNECTIONSKey1.fromNODESrecordID = NODESrecordIDs[nI].from;
      CONNECTIONSKey1.toNODESrecordID = NODESrecordIDs[nI].to;
      CONNECTIONSKey1.fromTimeOfDay = NO_TIME;
      rcode2 = btrieve(B_GETGREATEROREQUAL, TMS_CONNECTIONS, &CONNECTIONS, &CONNECTIONSKey1, 1);
      while(rcode2 == 0 &&
            CONNECTIONS.fromNODESrecordID == fromNODESrecordID &&
            CONNECTIONS.toNODESrecordID == toNODESrecordID)
      {
        if((nJ == 0 || (CONNECTIONS.flags & CONNECTIONS_FLAG_TWOWAY)) &&
              (CONNECTIONS.flags & CONNECTIONS_FLAG_RUNNINGTIME))
        {
//
//  Sequence + Title String
//
          if(!bDoneTitle)
          {
            sprintf(outputString, "%d\t%s\t%s\t\"", seqNum++, fromNodeName, toNodeName);
            strcat(outputString, "Data from the Connections Table:\"\r\n");
            WriteFile(hOutputFile, (LPCVOID *)outputString,
                  strlen(outputString), &dwBytesWritten, NULL);
            sprintf(outputString, "%d\t%s\t%s\t\"", seqNum++, fromNodeName, toNodeName);
            strcat(outputString, "  From   To    Route  Service  Pattern    Route  Service  Pattern Min             U/M     Dist  Spd/Hr");
            strcat(outputString, "\"\r\n");
            WriteFile(hOutputFile, (LPCVOID *)outputString,
                   strlen(outputString), &dwBytesWritten, NULL);
            bDoneTitle = TRUE;
          }
//
//  Sequence
//
          sprintf(outputString, "%d\t%s\t%s\t\"", seqNum++, fromNodeName, toNodeName);
//
//  From and to node names
//
          if(nJ == 0)
            sprintf(tempString, "  %4s %4s ", fromNodeName, toNodeName);
          else
            sprintf(tempString, "  %4s %4s ", toNodeName, fromNodeName);
          strcat(outputString, tempString);
//
//  From Route
//
          if(CONNECTIONS.fromROUTESrecordID == NO_RECORD)
            strcpy(szarString, "");
          else
          {
            ROUTESKey0.recordID = CONNECTIONS.fromROUTESrecordID;
            btrieve(B_GETEQUAL, TMS_ROUTES, &ROUTES, &ROUTESKey0, 0);
            strncpy(szarString, ROUTES.number, ROUTES_NUMBER_LENGTH);
            trim(szarString, 8);
          }
          sprintf(tempString, "%8s ", szarString);
          strcat(outputString, tempString);
//
//  From Service
//
          if(CONNECTIONS.fromSERVICESrecordID == NO_RECORD)
            strcpy(szarString, "");
          else
          {
            SERVICESKey0.recordID = CONNECTIONS.fromSERVICESrecordID;
            btrieve(B_GETEQUAL, TMS_SERVICES, &SERVICES, &SERVICESKey0, 0);
            strncpy(szarString, SERVICES.name, SERVICES_NAME_LENGTH);
            trim(szarString, 8);
          }
          sprintf(tempString, "%8s ", szarString);
          strcat(outputString, tempString);
//
//  From Pattern
//
          if(CONNECTIONS.fromPATTERNNAMESrecordID == NO_RECORD)
            strcpy(szarString, "");
          else
          {
            PATTERNNAMESKey0.recordID = CONNECTIONS.fromPATTERNNAMESrecordID;
            btrieve(B_GETEQUAL, TMS_PATTERNNAMES, &PATTERNNAMES, &PATTERNNAMESKey0, 0);
            strncpy(szarString, PATTERNNAMES.name, PATTERNNAMES_NAME_LENGTH);
            trim(szarString, 8);
          }
          sprintf(tempString, "%8s ", szarString);
          strcat(outputString, tempString);
//
//  To Route
//
          if(CONNECTIONS.toROUTESrecordID == NO_RECORD)
            strcpy(szarString, "");
          else
          {
            ROUTESKey0.recordID = CONNECTIONS.toROUTESrecordID;
            btrieve(B_GETEQUAL, TMS_ROUTES, &ROUTES, &ROUTESKey0, 0);
            strncpy(szarString, ROUTES.number, ROUTES_NUMBER_LENGTH);
            trim(szarString, 8);
          }
          sprintf(tempString, "%8s ", szarString);
          strcat(outputString, tempString);
//
//  To Service
//
          if(CONNECTIONS.toSERVICESrecordID == NO_RECORD)
            strcpy(szarString, "");
          else
          {
            SERVICESKey0.recordID = CONNECTIONS.toSERVICESrecordID;
            btrieve(B_GETEQUAL, TMS_SERVICES, &SERVICES, &SERVICESKey0, 0);
            strncpy(szarString, SERVICES.name, SERVICES_NAME_LENGTH);
            trim(szarString, 8);
          }
          sprintf(tempString, "%8s ", szarString);
          strcat(outputString, tempString);
//
//  To Pattern
//
          if(CONNECTIONS.toPATTERNNAMESrecordID == NO_RECORD)
            strcpy(szarString, "");
          else
          {
            PATTERNNAMESKey0.recordID = CONNECTIONS.toPATTERNNAMESrecordID;
            btrieve(B_GETEQUAL, TMS_PATTERNNAMES, &PATTERNNAMES, &PATTERNNAMESKey0, 0);
            strncpy(szarString, PATTERNNAMES.name, PATTERNNAMES_NAME_LENGTH);
            trim(szarString, 8);
          }
          sprintf(tempString, "%8s ", szarString);
          strcat(outputString, tempString);
//
//  Time
//
          sprintf(tempString, "%3d ", CONNECTIONS.connectionTime / 60);
          strcat(outputString, tempString);
//
//  Btwn
//
          if(CONNECTIONS.fromTimeOfDay == NO_TIME)
          {
            strcpy(tempString, "      ");
          }
          else
          {
            sprintf(tempString, "%s ", Tchar(CONNECTIONS.fromTimeOfDay));
          }
          strcat(outputString, tempString);
//
//  And
//
          if(CONNECTIONS.toTimeOfDay == NO_TIME)
          {
            strcpy(tempString, "      ");
          }
          else
          {
            sprintf(tempString, "%s ", Tchar(CONNECTIONS.toTimeOfDay));
          }
          strcat(outputString, tempString);
//
//  Distance type and distance
//
          if(CONNECTIONS.distance == NO_RECORD || CONNECTIONS.distance == 0.0)
          {
            strcpy(szarString, "Map ");
            distance = GreatCircleDistance(longitude[0], latitude[0], longitude[1], latitude[1]);
          }
          else
          {
            strcpy(szarString, "User");
            distance = CONNECTIONS.distance;
          }
          sprintf(tempString, "%s %7.2f ", szarString, distance);
          strcat(outputString, tempString);
//
//  Speed per hour
//
          if(CONNECTIONS.connectionTime == 0)
            strcpy(tempString, "-------");
          else
            sprintf(tempString, "%7.2f", distance / ((double)CONNECTIONS.connectionTime / 3600));
          strcat(outputString, tempString);
//
//  Write it out
//
          strcat(outputString, "\"\r\n");
          WriteFile(hOutputFile, (LPCVOID *)outputString,
                strlen(outputString), &dwBytesWritten, NULL);
        }  // if connection type == requested type to display
        rcode2 = btrieve(B_GETNEXT, TMS_CONNECTIONS, &CONNECTIONS, &CONNECTIONSKey1, 1);
      }  // while
    }  //  nJ
//
//  If we didn't output a title, there wasn't anything
//
    if(!bDoneTitle)
      continue;
//
//  Now get the stuff from the timechecks table
//
//  Sequence + Title String
//
    sprintf(outputString, "%d\t%s\t%s\t\"\"\r\n", seqNum++, fromNodeName, toNodeName);
    WriteFile(hOutputFile, (LPCVOID *)outputString,
          strlen(outputString), &dwBytesWritten, NULL);
    sprintf(outputString, "%d\t%s\t%s\t\"", seqNum++, fromNodeName, toNodeName);
    strcat(outputString, "Data from the Timechecks Table:\"\r\n");
    WriteFile(hOutputFile, (LPCVOID *)outputString,
          strlen(outputString), &dwBytesWritten, NULL);
//
//  Look up the "to node" on the timechecks table
//
    numEntries = 0;
    TIMECHECKSKey4.NODESrecordID = NODESrecordIDs[nI].to;
    rcode2 = btrieve(B_GETEQUAL, TMS_TIMECHECKS, &TIMECHECKS, &TIMECHECKSKey4, 4);
    while(rcode2 == 0 &&
          TIMECHECKS.NODESrecordID == NODESrecordIDs[nI].to)
    {
      if(StatusBarAbort())
      {
        bKeepGoing = FALSE;
        goto deallocate;
      }
//
//  Get the trip
//
      TRIPSKey0.recordID = TIMECHECKS.TRIPSrecordID;
      rcode2 = btrieve(B_GETEQUAL, TMS_TRIPS, &TRIPS, &TRIPSKey0, 0);
      if(rcode2 == 0)
      {
//
//  Get the pattern of the trip and find the "to node".
//
        previousNODESrecordID = NO_RECORD;
        PATTERNSKey2.ROUTESrecordID = TRIPS.ROUTESrecordID;
        PATTERNSKey2.SERVICESrecordID = TRIPS.SERVICESrecordID;
        PATTERNSKey2.directionIndex = TRIPS.directionIndex;
        PATTERNSKey2.PATTERNNAMESrecordID = TRIPS.PATTERNNAMESrecordID;
        PATTERNSKey2.nodeSequence = NO_RECORD;
        currentNodePositionInTrip = 0;
        rcode2 = btrieve(B_GETGREATER, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
        while(rcode2 == 0 &&
              PATTERNS.ROUTESrecordID == TRIPS.ROUTESrecordID &&
              PATTERNS.SERVICESrecordID == TRIPS.SERVICESrecordID &&
              PATTERNS.directionIndex == TRIPS.directionIndex &&
              PATTERNS.PATTERNNAMESrecordID == TRIPS.PATTERNNAMESrecordID)
        {
          if(!(PATTERNS.flags & PATTERNS_FLAG_BUSSTOP))
          {
            if(PATTERNS.NODESrecordID == TIMECHECKS.NODESrecordID)
            {
              if(currentNodePositionInTrip == TIMECHECKS.nodePositionInTrip)
                break;
              currentNodePositionInTrip++;
            }    
            previousNODESrecordID = PATTERNS.NODESrecordID;
          }
          rcode2 = btrieve(B_GETNEXT, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
        }
//
//  If we have a "from node" and "to node", save the trip and the actual time
//
        if(previousNODESrecordID == NODESrecordIDs[nI].from)
        {
          TRIPData[numEntries].TRIPSrecordID = TRIPS.recordID;
          TRIPData[numEntries].actualTimeOfDay = TIMECHECKS.actualTime;
          TRIPData[numEntries].nodePositionInTrip = TIMECHECKS.nodePositionInTrip;
          numEntries++;
          if(numEntries >= TMSRPT40_MAXNODES)
            break;
        }
      }
      rcode2 = btrieve(B_GETNEXT, TMS_TIMECHECKS, &TIMECHECKS, &TIMECHECKSKey4, 4);
    }
//
//  Sort TRIPData based on actualTime and print out the title
//
    if(numEntries > 0)
    {
      qsort((void *)TRIPData, numEntries, sizeof(TMSRPT40TRIPSDef), TMSRPT40sort_trips);
      sprintf(outputString, "%d\t%s\t%s\t\"", seqNum++, fromNodeName, toNodeName);
      strcat(outputString, "  Trip#    Route  Service  Pattern    At Act    Dist  Spd/Hr\"\r\n");
      WriteFile(hOutputFile, (LPCVOID *)outputString,
            strlen(outputString), &dwBytesWritten, NULL);
    }
//
//  Cycle through the saved entries
//
    for(nJ = 0; nJ < numEntries; nJ++)
    {
      if(StatusBarAbort())
      {
        bKeepGoing = FALSE;
        goto deallocate;
      }
//
//  Get the trip and generate it
//
      TRIPSKey0.recordID = TRIPData[nJ].TRIPSrecordID;
      rcode2 = btrieve(B_GETEQUAL, TMS_TRIPS, &TRIPS, &TRIPSKey0, 0);
      if(rcode2 != 0)
        continue;
      sprintf(outputString, "%d\t%s\t%s\t\"", seqNum++, fromNodeName, toNodeName);
      sprintf(outputStringRaw, "%s\t%s\t", fromNodeName, toNodeName);
//
//  Trip number
//
      sprintf(tempString, "  %5ld ", TRIPS.tripNumber);
      strcat(outputString, tempString);
//
//  Route
//
      ROUTESKey0.recordID = TRIPS.ROUTESrecordID;
      btrieve(B_GETEQUAL, TMS_ROUTES, &ROUTES, &ROUTESKey0, 0);
      strncpy(szarString, ROUTES.number, ROUTES_NUMBER_LENGTH);
      trim(szarString, 8);
      sprintf(tempString, "%8s ", szarString);
      strcat(outputString, tempString);
      strncpy(szarString, ROUTES.number, ROUTES_NUMBER_LENGTH);
      trim(szarString, ROUTES_NUMBER_LENGTH);
      strcat(outputStringRaw, szarString);
      strcat(outputStringRaw, "\t");
      DIRECTIONSKey0.recordID = ROUTES.DIRECTIONSrecordID[TRIPS.directionIndex];
      btrieve(B_GETEQUAL, TMS_DIRECTIONS, &DIRECTIONS, &DIRECTIONSKey0, 0);
      strncpy(szarString, DIRECTIONS.abbrName, DIRECTIONS_ABBRNAME_LENGTH);
      trim(szarString, DIRECTIONS_ABBRNAME_LENGTH);
      strcat(outputStringRaw, szarString);
      strcat(outputStringRaw, "\t");
//
//  Service
//
      SERVICESKey0.recordID = TRIPS.SERVICESrecordID;
      btrieve(B_GETEQUAL, TMS_SERVICES, &SERVICES, &SERVICESKey0, 0);
      strncpy(szarString, SERVICES.name, SERVICES_NAME_LENGTH);
      trim(szarString, 8);
      sprintf(tempString, "%8s ", szarString);
      strcat(outputString, tempString);
      strncpy(szarString, SERVICES.name, SERVICES_NAME_LENGTH);
      trim(szarString, SERVICES_NAME_LENGTH);
      strcat(outputStringRaw, szarString);
      strcat(outputStringRaw, "\t");
//
//  Pattern
//
      PATTERNNAMESKey0.recordID = TRIPS.PATTERNNAMESrecordID;
      btrieve(B_GETEQUAL, TMS_PATTERNNAMES, &PATTERNNAMES, &PATTERNNAMESKey0, 0);
      strncpy(szarString, PATTERNNAMES.name, PATTERNNAMES_NAME_LENGTH);
      trim(szarString, 8);
      sprintf(tempString, "%8s ", szarString);
      strcat(outputString, tempString);
      strncpy(szarString, PATTERNNAMES.name, PATTERNNAMES_NAME_LENGTH);
      trim(szarString, PATTERNNAMES_NAME_LENGTH);
      strcat(outputStringRaw, szarString);
      strcat(outputStringRaw, "\t");
//
//  At time
//
      sprintf(tempString, "%s ", Tchar(TRIPData[nJ].actualTimeOfDay));
      strcat(outputString, tempString);
      sprintf(tempString, "%ld\t", TRIPData[nJ].actualTimeOfDay);
      strcat(outputStringRaw, tempString);
//
//  Actual running time
//
//  Pass 1 - find the companion normally (nodeposition = 0) or if a pair repeats
//
      actualTime = NO_TIME;
      TIMECHECKSKey5.TRIPSrecordID = TRIPS.recordID;
      rcode2 = btrieve(B_GETEQUAL, TMS_TIMECHECKS, &TIMECHECKS, &TIMECHECKSKey5, 5);
      while(rcode2 == 0)
      {
        if(TIMECHECKS.NODESrecordID == NODESrecordIDs[nI].from  &&
              TIMECHECKS.nodePositionInTrip == TRIPData[nJ].nodePositionInTrip)
        {
          actualTime = TRIPData[nJ].actualTimeOfDay - TIMECHECKS.actualTime;
          break;
        }
        rcode2 = btrieve(B_GETNEXT, TMS_TIMECHECKS, &TIMECHECKS, &TIMECHECKSKey5, 5);
        if(TIMECHECKS.TRIPSrecordID != TRIPS.recordID)
          break;
      }
//
//  Pass 2 - find the companion when you have the same start and end node
//
      if(actualTime == NO_TIME)
      {
        TIMECHECKSKey5.TRIPSrecordID = TRIPS.recordID;
        rcode2 = btrieve(B_GETEQUAL, TMS_TIMECHECKS, &TIMECHECKS, &TIMECHECKSKey5, 5);
        while(rcode2 == 0)
        {
          if(TIMECHECKS.NODESrecordID == NODESrecordIDs[nI].from)
          {
            actualTime = TRIPData[nJ].actualTimeOfDay - TIMECHECKS.actualTime;
            break;
          }
          rcode2 = btrieve(B_GETNEXT, TMS_TIMECHECKS, &TIMECHECKS, &TIMECHECKSKey5, 5);
          if(TIMECHECKS.TRIPSrecordID != TRIPS.recordID)
            break;
        }
      }
      if(actualTime == NO_TIME)
      {
        strcpy(tempString, "    ");
        actualTime = 0;
      }
      else
      {
        sprintf(tempString, "%3d ", actualTime / 60);
      }
      strcat(outputString, tempString);
      arrivalTime = TRIPData[nJ].actualTimeOfDay + actualTime;
      sprintf(tempString, "%ld\r\n", arrivalTime);
      strcat(outputStringRaw, tempString);
//
//  Distance
//
      distance = GreatCircleDistance(longitude[0], latitude[0], longitude[1], latitude[1]);
      sprintf(tempString, "%7.2f ", distance);
      strcat(outputString, tempString);
//
//  Speed per hour
//
      if(actualTime == 0 || actualTime == NO_TIME)
        strcpy(tempString, "-------");
      else
        sprintf(tempString, "%7.2f", distance / ((double)actualTime / 3600));
      strcat(outputString, tempString);
//
//  Write it out
//
      strcat(outputString, "\"\r\n");
      WriteFile(hOutputFile, (LPCVOID *)outputString,
            strlen(outputString), &dwBytesWritten, NULL);
      WriteFile(hOutputFileRaw, (LPCVOID *)outputStringRaw,
            strlen(outputStringRaw), &dwBytesWritten, NULL);
    }
    sprintf(outputString, "%d\t%s\t%s\t\"  Total entries: %d\"\r\n",
          seqNum++, fromNodeName, toNodeName, numEntries);
    WriteFile(hOutputFile, (LPCVOID *)outputString,
          strlen(outputString), &dwBytesWritten, NULL);
  }  // nI
  bKeepGoing = TRUE;
//
//  Free allocated memory
//
  deallocate:
    TMSHeapFree(REPORTPARMS.pRouteList);
    TMSHeapFree(REPORTPARMS.pServiceList);
    CloseHandle(hOutputFile);
    CloseHandle(hOutputFileRaw);
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


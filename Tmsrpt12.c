//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
#include "TMSHeader.h"
#include "cistms.h"
#include <math.h>

#define OUTPUT_FOLDER_NAME "HP Interface Files"

BOOL  CALLBACK HPUNLOADMsgProc(HWND, UINT, WPARAM, LPARAM);
char *ConvertCTranRunNumber(long);
int sort_runs(const void *, const void *);  // Source is in tmsrpt10.c

typedef struct HPRDStruct
{
  long runNumber;
  long pieceNumber;
  long blockNumber;
  long startNODESrecordID;
  long startTRIPSrecordID;
  long startTime;
  long endNODESrecordID;
  long endTRIPSrecordID;
  long endTime;
} HPRDDef;

typedef struct HPNODESStruct
{
  long recordID;
  long flags;
} HPNODESDef;

//
//  TMS Unload to the C-Tran HP System
//
BOOL FAR TMSRPT12(TMSRPTPassedDataDef *pPassedData)
{
  GenerateTripDef GTResults;
  RPTFLAGSDef RPTFLAGS;
  HFILE hfOutputFile[5];
  HFILE hfErrorLog;
  float prevLongitude;
  float prevLatitude;
  float longitude;
  float latitude;
  BOOL  bFound;
  BOOL  bRC;
  BOOL  bGotOne;
  BOOL  bGotError;
  BOOL  bBadOnStreet;
  BOOL  bFinishedOK;
  long  ROUTErecordIDs[500];
  long  driverNumber;
  char  outputString[1024];
  char  dummy[256];
  char *ptr;
  char *pszReportName;
  int   nI;
  int   nJ;
  int   nK;
  int   nL;
  int   rcode2;
  int   numRoutes;
  int   numNodes;
  int   numRecords;
  int   serviceNumber;
  int   counter;
  int   pieceNumber;
  int   pos;

  HPNODESDef *HPNODES = NULL;
  HPRDDef    *HPRData = NULL;

  pPassedData->nReportNumber = 11;
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
//  See what he wants to unload
//
  RPTFLAGS.nReportNumber = pPassedData->nReportNumber;
  bRC = DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_HPUNLOAD),
        hWndMain, (DLGPROC)HPUNLOADMsgProc, (LPARAM)&RPTFLAGS);
  if(!bRC)
  {
    bFinishedOK = FALSE;
    goto done;
  }
//
//  Open the error log and fire up the status bar
//
  hfErrorLog = _lcreat("error.log", 0);
  if(hfErrorLog == HFILE_ERROR)
  {
    TMSError((HWND)NULL, MB_ICONSTOP, ERROR_226, (HANDLE)NULL);
    bFinishedOK = FALSE;
    goto done;
  }
  bGotError = FALSE;
  bFinishedOK = FALSE;
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
//  Get the route data in case he didn't select it
//
  numRoutes = 0;
  rcode2 = btrieve(B_GETFIRST, TMS_ROUTES, &ROUTES, &ROUTESKey1, 1);
  while(rcode2 == 0)
  {
    if(StatusBarAbort())
      goto done;
    ROUTErecordIDs[numRoutes++] = ROUTES.recordID;
    rcode2 = btrieve(B_GETNEXT, TMS_ROUTES, &ROUTES, &ROUTESKey1, 1);
  }
//
//  Get the node data in case he didn't select it
//
//  Get all the nodes
//
  rcode2 = btrieve(B_STAT, TMS_NODES, &BSTAT, dummy, 0);
  if(rcode2 != 0 || BSTAT.numRecords == 0)
    goto done;
  numNodes = BSTAT.numRecords;
  HPNODES = (HPNODESDef *)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(HPNODESDef) * numNodes); 
  if(HPNODES == NULL)
  {
    AllocationError(__FILE__, __LINE__, FALSE);
    goto done;
  }
  rcode2 = btrieve(B_GETFIRST, TMS_NODES, &NODES, &NODESKey0, 0);
  for(nI = 0; nI < numNodes; nI++)
  {
    if(StatusBarAbort())
    {
      goto done;
    }
    HPNODES[nI].recordID = NODES.recordID;
    HPNODES[nI].flags = NODES.flags;
    rcode2 = btrieve(B_GETNEXT, TMS_NODES, &NODES, &NODESKey0, 0);
  }
//
//  Route Info
//
#define ROUTENAME_LENGTH max(ROUTES_NAME_LENGTH, 30)
  if(RPTFLAGS.flags & TMSRPT12_ROUTEINFO)
  {
    int  routeNumber;
    char routeName[ROUTENAME_LENGTH + 1];

    LoadString(hInst, TEXT_139, tempString, TEMPSTRING_LENGTH);
    StatusBarText(tempString);
//
//  Open the output file
//
    hfOutputFile[0] = _lcreat("route.hp", 0);
    if(hfOutputFile[0] == HFILE_ERROR)
    {
      LoadString(hInst, ERROR_202, szFormatString, sizeof(szFormatString));
      sprintf(szarString, szFormatString, tempString);
      MessageBeep(MB_ICONSTOP);
      MessageBox((HWND)NULL, szarString, TMS, MB_ICONSTOP);
      bFinishedOK = FALSE;
      goto done;
    }
//
//  Route Info file layout
//
//  Bytes   Description
//  ~~~~~   ~~~~~~~~~~~
//   1-4    Route number, right justified
//   5-34   Route name
//
    for(nI = 0; nI < numRoutes; nI++)
    {
      if(StatusBarAbort())
      {
        goto done;
      }
      StatusBar((long)nI, (long)numRoutes);
      ROUTESKey0.recordID = ROUTErecordIDs[nI];
      btrieve(B_GETEQUAL, TMS_ROUTES, &ROUTES, &ROUTESKey0, 0);
      routeNumber = atoi(ROUTES.number);
      memset(routeName, ' ', sizeof(routeName));
      strncpy(routeName, ROUTES.name, ROUTENAME_LENGTH);
      routeName[ROUTENAME_LENGTH] = '\0';
      sprintf(outputString, "%4d%s\r\n", routeNumber, routeName);
      _lwrite(hfOutputFile[0], outputString, strlen(outputString));
    }
    _lclose(hfOutputFile[0]);
  }
  StatusBar(-1L, -1L);
//
//  Stops Info
//
#define INTERSECTION_LENGTH  max(NODES_INTERSECTION_LENGTH, 60)
#define ONSTREET_LENGTH      30
#define ATSTREET_LENGTH      30
#define SHORTONSTREET_LENGTH  5
#define SHORTATSTREET_LENGTH  5
  if(RPTFLAGS.flags & TMSRPT12_STOPSINFO)
  {
    long stopNumber;
    char intersection[INTERSECTION_LENGTH + 1];
    char onStreet[ONSTREET_LENGTH + 1];
    char atStreet[ATSTREET_LENGTH + 1];
    char shortOnStreet[SHORTONSTREET_LENGTH + 1];
    char shortAtStreet[SHORTATSTREET_LENGTH + 1];

    LoadString(hInst, TEXT_140, tempString, TEMPSTRING_LENGTH);
    StatusBarText(tempString);
//
//  Open the output file
//
    hfOutputFile[0] = _lcreat("stops.hp", 0);
    if(hfOutputFile[0] == HFILE_ERROR)
    {
      LoadString(hInst, ERROR_202, szFormatString, sizeof(szFormatString));
      sprintf(szarString, szFormatString, tempString);
      MessageBeep(MB_ICONSTOP);
      MessageBox((HWND)NULL, szarString, TMS, MB_ICONSTOP);
      bFinishedOK = FALSE;
      goto done;
    }
//
//  Stops Info file layout
//
//  Bytes   Description
//  ~~~~~   ~~~~~~~~~~~
//   1-4    Stop number
//   5-64   Location description (Intersection)
//  65-94   On street
//  95-124  At street
// 125-129  Short on street
// 130-134  Short at street
//
    for(nI = 0; nI < numNodes; nI++)
    {
      if(StatusBarAbort())
      {
        goto done;
      }
      StatusBar((long)nI, (long)numNodes);
      NODESKey0.recordID = HPNODES[nI].recordID;
      btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
//
//  Test for geocode
//
      prevLongitude = NODES.longitude;
      prevLatitude = NODES.latitude;
      if(prevLongitude == 0.0 || prevLatitude == 0.0)
      {
        bGotError = TRUE;
        strncpy(tempString, NODES.abbrName, NODES_ABBRNAME_LENGTH);
        trim(tempString, NODES_ABBRNAME_LENGTH);
        sprintf(outputString, "Node \"%s\" has not been geocoded\r\n", tempString);
        _lwrite(hfErrorLog, outputString, strlen(outputString));
      }
//
//  Stop number
//
//      stopNumber = NODES.number;
//      if(stopNumber > 10000)
//        stopNumber /= 10000;
      if(NODES.flags & NODES_FLAG_STOP)
      {
        stopNumber = NODES.number;
      }
      else
      {
        if(NODES.OBStopNumber > 0)
        {
          stopNumber = NODES.OBStopNumber;
        }
        else if(NODES.IBStopNumber > 0)
        {
          stopNumber = NODES.IBStopNumber;
        }
        else
        {
          stopNumber = 0;
        }
      }
      if(stopNumber == 0)
      {
        bGotError = TRUE;
        strncpy(tempString, NODES.abbrName, NODES_ABBRNAME_LENGTH);
        trim(tempString, NODES_ABBRNAME_LENGTH);
        sprintf(outputString, "Node \"%s\" has a number of 0\r\n", tempString);
        _lwrite(hfErrorLog, outputString, strlen(outputString));
      }
//
//  Location description (intersection)
//
      strncpy(intersection, NODES.intersection, INTERSECTION_LENGTH);
      intersection[INTERSECTION_LENGTH] = '\0';
//
//  On street
//
      ptr = strstr(intersection, " && ");
      if(ptr == NULL)
        strcpy(onStreet, "");
      else
      {
        pos = ptr - intersection + 1;
        strncpy(onStreet, intersection, pos);
        onStreet[pos] = '\0';
        strcpy(szarString, &intersection[pos + 3]);
        bBadOnStreet = FALSE;
      }
      if(strcmp(onStreet, "") == 0)
      {
        bGotError = TRUE;
        bBadOnStreet = TRUE;
        strncpy(tempString, NODES.abbrName, NODES_ABBRNAME_LENGTH);
        trim(tempString, NODES_ABBRNAME_LENGTH);
        sprintf(outputString, "Node \"%s\" has a bad intersection field\r\n", tempString);
        _lwrite(hfErrorLog, outputString, strlen(outputString));
      }
      pad(onStreet, ONSTREET_LENGTH);
      onStreet[ONSTREET_LENGTH] = '\0';
//
//  At street
//
      if(bBadOnStreet)
        strcpy(atStreet, "");
      else if(strcmp(szarString, "") == 0)
      {
        bGotError = TRUE;
        strncpy(tempString, NODES.abbrName, NODES_ABBRNAME_LENGTH);
        trim(tempString, NODES_ABBRNAME_LENGTH);
        sprintf(outputString, "Node \"%s\" has a bad intersection field\r\n", tempString);
        _lwrite(hfErrorLog, outputString, strlen(outputString));
      }
      else
      {
        trim(szarString, ATSTREET_LENGTH);
        strcpy(atStreet, szarString);
      }
      pad(atStreet, ATSTREET_LENGTH);
      atStreet[ATSTREET_LENGTH] = '\0';
//
//  Short on street
//
      strncpy(shortOnStreet, NODES.longName, 4);
      pad(shortOnStreet, 4);
      shortOnStreet[4] = ' ';
      shortOnStreet[SHORTONSTREET_LENGTH] = '\0';
//
//  Short at street
//
      strncpy(shortAtStreet, &NODES.longName[4], 4);
      pad(shortAtStreet, 4);
      shortAtStreet[4] = ' ';
      shortAtStreet[SHORTATSTREET_LENGTH] = '\0';
//
//  Write the record
//
      sprintf(outputString, "%4ld%s%s%s%s%s\r\n",
             stopNumber, intersection, onStreet, atStreet, shortOnStreet, shortAtStreet);
      _lwrite(hfOutputFile[0], outputString, strlen(outputString));
    }
    _lclose(hfOutputFile[0]);
  }
//
//  Path Info
//
#define MAX_PATTERNS         5
#define MAX_NODESPERPATTERN  9
#define BYTESPERPATTERN     11
  if(RPTFLAGS.flags & TMSRPT12_PATHINFO)
  {
    GetConnectionTimeDef GCTData;
    float distance;
    float dummy;
    BOOL bGotOne;
    char timepointData[BYTESPERPATTERN * MAX_NODESPERPATTERN + 1];
    long previousPattern;
    long stopNumber;
    long distanceI;
    long distanceF;
    long runningTime;
    long previousNode;
    int  routeNumber;
    int  pathNumber;
    int  numNodesInPattern;

    LoadString(hInst, TEXT_141, tempString, TEMPSTRING_LENGTH);
    StatusBarText(tempString);
//
//  Open the output file
//
    hfOutputFile[0] = _lcreat("path.hp", 0);
    if(hfOutputFile[0] == HFILE_ERROR)
    {
      LoadString(hInst, ERROR_202, szFormatString, sizeof(szFormatString));
      sprintf(szarString, szFormatString, tempString);
      MessageBeep(MB_ICONSTOP);
      MessageBox((HWND)NULL, szarString, TMS, MB_ICONSTOP);
      bFinishedOK = FALSE;
      goto done;
    }
//
//  Path Info file layout
//
//  Bytes   Description
//  ~~~~~   ~~~~~~~~~~~
//   1-4    Route number
//    5     Service sort number (1=Weekday, 2=Saturday, 3=Sunday)
//    6     Direction code (0 / 1)
//    7     Path number
//   8-106  Time point array
//          9 x 4 bytes - Stop number
//              2 bytes - Running time from previous (in minutes)
//              5 bytes - Distance from previous (nn.nn)
//
//  Note: Due to the HP's inability to handle more than 9 nodes on a pattern,
//        this unload, if faced with the situation, will write out the first
//        nine timepoints.
//
//  Note: Due to the HP's inability to handle more than 5 patterns, this
//        unload will only write out the first five patterns for any
//        route/service/direction combination.
//
    for(nI = 0; nI < numRoutes; nI++)
    {
      if(StatusBarAbort())
        goto done;
      StatusBar((long)nI, (long)numRoutes);
      ROUTESKey0.recordID = ROUTErecordIDs[nI];
      btrieve(B_GETEQUAL, TMS_ROUTES, &ROUTES, &ROUTESKey0, 0);
      routeNumber = atoi(ROUTES.number);
      serviceNumber = 1;
      rcode2 = btrieve(B_GETFIRST, TMS_SERVICES, &SERVICES, &SERVICESKey1, 1);
      while(rcode2 == 0 && serviceNumber <= 9)
      {
        for(nJ = 0; nJ < 2; nJ++)
        {
          if(StatusBarAbort())
            goto done;
          PATTERNSKey2.ROUTESrecordID = ROUTES.recordID;
          PATTERNSKey2.SERVICESrecordID = SERVICES.recordID;
          PATTERNSKey2.directionIndex = nJ;
          PATTERNSKey2.PATTERNNAMESrecordID = basePatternRecordID;
          PATTERNSKey2.nodeSequence = NO_RECORD;
          pathNumber = 0;
          previousPattern = NO_RECORD;
          previousNode = NO_RECORD;
          numNodesInPattern = 0;
          rcode2 = btrieve(B_GETGREATEROREQUAL, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
          bGotOne = FALSE;
          distance = (float)0.0;
          while(rcode2 == 0 &&
                PATTERNS.ROUTESrecordID == ROUTES.recordID &&
                PATTERNS.SERVICESrecordID == SERVICES.recordID &&
                PATTERNS.directionIndex == nJ &&
                pathNumber < MAX_PATTERNS)
          {
            NODESKey0.recordID = PATTERNS.NODESrecordID;
            btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
            longitude = NODES.longitude;
            latitude = NODES.latitude;
            if(PATTERNS.flags & PATTERNS_FLAG_BUSSTOP)
            {
              distance += GetStopDistance(prevLongitude, prevLatitude, longitude, latitude);
            }
            else
            {
              bGotOne = TRUE;
              if(PATTERNS.PATTERNNAMESrecordID != previousPattern || numNodesInPattern > MAX_NODESPERPATTERN)
              {
                if(previousPattern != NO_RECORD)
                {
                  pad(timepointData, BYTESPERPATTERN * MAX_NODESPERPATTERN);
                  timepointData[BYTESPERPATTERN * MAX_NODESPERPATTERN] = '\0';
                  sprintf(outputString, "%4d%1d%1d%1d%s\r\n", routeNumber, serviceNumber, nJ, pathNumber, timepointData);
                  _lwrite(hfOutputFile[0], outputString, strlen(outputString));
                }
                previousPattern = PATTERNS.PATTERNNAMESrecordID;
                previousNode = NO_RECORD;
                numNodesInPattern = 0;
                strcpy(timepointData, "");
                pathNumber++;
              }
//              stopNumber = NODES.number;
//              if(stopNumber > 10000)
//                stopNumber /= 10000;
              if(NODES.OBStopNumber > 0)
              {
                stopNumber = NODES.OBStopNumber;
              }
              else if(NODES.IBStopNumber > 0)
              {
                stopNumber = NODES.IBStopNumber;
              }
              if(numNodesInPattern == 0)
              {
                runningTime = 0;
                distance = (float)0.0;
              }
              else
              {
                GCTData.fromROUTESrecordID = ROUTES.recordID;
                GCTData.fromSERVICESrecordID = SERVICES.recordID;
                GCTData.fromPATTERNNAMESrecordID = PATTERNS.PATTERNNAMESrecordID;
                GCTData.fromNODESrecordID = previousNode;
                GCTData.toROUTESrecordID = ROUTES.recordID;
                GCTData.toSERVICESrecordID = SERVICES.recordID;
                GCTData.toPATTERNNAMESrecordID = PATTERNS.PATTERNNAMESrecordID;
                GCTData.toNODESrecordID = PATTERNS.NODESrecordID;
                GCTData.timeOfDay = NO_TIME;
                runningTime = GetConnectionTime(GCT_FLAG_RUNNINGTIME, &GCTData, &dummy);
                runningTime = runningTime == NO_TIME ? 0 : runningTime / 60;
                distance += GetStopDistance(prevLongitude, prevLatitude, longitude, latitude);
              }
              previousNode = NODES.recordID;
              if(distance == 0.0)
              {
                strcpy(szarString, "00.00");
              }
              else
              {
                distanceI = (long)distance;
                distanceF = (long)((float)((distance - distanceI) * 100));
                sprintf(szarString, "%2d.%02d", distanceI, distanceF);
              }
              sprintf(tempString, "%4ld%2ld%s", stopNumber, runningTime, szarString);
              strcat(timepointData, tempString);
              numNodesInPattern++;
              distance = (float)0.0;
            }
            prevLongitude = longitude;
            prevLatitude = latitude;
            rcode2 = btrieve(B_GETNEXT, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
          }
          if(bGotOne)
          {
            pad(timepointData, BYTESPERPATTERN * MAX_NODESPERPATTERN);
            timepointData[BYTESPERPATTERN * MAX_NODESPERPATTERN] = '\0';
            sprintf(outputString, "%4d%1d%1d%1d%s\r\n", routeNumber, serviceNumber, nJ, pathNumber, timepointData);
            _lwrite(hfOutputFile[0], outputString, strlen(outputString));
          }
        }
        rcode2 = btrieve(B_GETNEXT, TMS_SERVICES, &SERVICES, &SERVICESKey1, 1);
        serviceNumber++;
      }
    }
    _lclose(hfOutputFile[0]);
  }
  StatusBar(-1L, -1L);
//
//  Deadhead Info
//
#define MAX_DEADHEADS     6
#define BYTESPERDEADHEAD 10
  if(RPTFLAGS.flags & TMSRPT12_DEADHEADINFO)
  {
    float distance;
    long distanceI;
    long distanceF;
    BOOL bTwoWay;
    char deadheadData[MAX_DEADHEADS * BYTESPERDEADHEAD + 1];
    char fromNodeAbbr[NODES_ABBRNAME_LENGTH + 1];
    char toNodeAbbr[NODES_ABBRNAME_LENGTH + 1];
    long fromNode;
    long toNode;
    long fromStopNumber;
    long toStopNumber;
    int  numDeadheads;

    LoadString(hInst, TEXT_142, tempString, TEMPSTRING_LENGTH);
    StatusBarText(tempString);
//
//  Open the output file
//
    hfOutputFile[0] = _lcreat("deadhead.hp", 0);
    if(hfOutputFile[0] == HFILE_ERROR)
    {
      LoadString(hInst, ERROR_202, szFormatString, sizeof(szFormatString));
      sprintf(szarString, szFormatString, tempString);
      MessageBeep(MB_ICONSTOP);
      MessageBox((HWND)NULL, szarString, TMS, MB_ICONSTOP);
      bFinishedOK = FALSE;
      goto done;
    }
//
//  Deadhead Info file layout
//
//  Bytes   Description
//  ~~~~~   ~~~~~~~~~~~
//   1-4    From stop number
//   5-8    To stop number
//   9-13   Distance
//   14     Service (0 = All, 1 = Wk, 2 = Sa, 3 = Su)
//  15-74   Running time array
//          6 x 4 bytes - begin time
//              4 bytes - end time
//              2 bytes - time in minutes
//
//  Loop through the nodes looking for deadhead connections
//
    for(nI = 0; nI < numNodes - 1; nI++)
    {
      if(StatusBarAbort())
      {
        goto done;
      }
      StatusBar((long)nI, (long)numNodes);
      if(HPNODES[nI].flags & NODES_FLAG_STOP)
        continue;
      for(nJ = nI + 1; nJ < numNodes; nJ++)
      {
        if(StatusBarAbort())
          goto done;
        if(HPNODES[nJ].flags & NODES_FLAG_STOP)
          continue;
        for(nK = 0; nK < 2; nK++)
        {
          if(nK == 0)
          {
            fromNode = HPNODES[nI].recordID;
            toNode = HPNODES[nJ].recordID;
          }
          else
          {
            fromNode = HPNODES[nJ].recordID;
            toNode = HPNODES[nI].recordID;
          }
          NODESKey0.recordID = fromNode;
          btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
//          fromStopNumber = NODES.number;
//          if(fromStopNumber > 10000)
//            fromStopNumber /= 10000;
          if(NODES.flags & NODES_FLAG_STOP)
          {
            fromStopNumber = NODES.number;
          }
          else
          {
            if(NODES.OBStopNumber > 0)
            {
              fromStopNumber = NODES.OBStopNumber;
            }
            else if(NODES.IBStopNumber > 0)
            {
              fromStopNumber = NODES.IBStopNumber;
            }
          }
          prevLongitude = NODES.longitude;
          prevLatitude = NODES.latitude;
          strncpy(fromNodeAbbr, NODES.abbrName, NODES_ABBRNAME_LENGTH);
          trim(fromNodeAbbr, NODES_ABBRNAME_LENGTH);
          NODESKey0.recordID = toNode;
          btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
          longitude = NODES.longitude;
          latitude = NODES.latitude;
//          toStopNumber = NODES.number;
//          if(toStopNumber > 10000)
//            toStopNumber /= 10000;
          if(NODES.flags & NODES_FLAG_STOP)
          {
            toStopNumber = NODES.number;
          }
          else
          {
            if(NODES.OBStopNumber > 0)
            {
              toStopNumber = NODES.OBStopNumber;
            }
            else if(NODES.IBStopNumber > 0)
            {
              toStopNumber = NODES.IBStopNumber;
            }
          }
          if(fromStopNumber == toStopNumber)  // This can occur when nodes are aliased
            break;
          strncpy(toNodeAbbr, NODES.abbrName, NODES_ABBRNAME_LENGTH);
          trim(toNodeAbbr, NODES_ABBRNAME_LENGTH);
          CONNECTIONSKey1.fromNODESrecordID = fromNode;
          CONNECTIONSKey1.toNODESrecordID = toNode;
          CONNECTIONSKey1.fromTimeOfDay = NO_TIME;
          rcode2 = btrieve(B_GETGREATEROREQUAL, TMS_CONNECTIONS, &CONNECTIONS, &CONNECTIONSKey1, 1);
          numDeadheads = 0;
          bTwoWay = FALSE;
          while(rcode2 == 0 &&
                CONNECTIONS.fromNODESrecordID == fromNode &&
                CONNECTIONS.toNODESrecordID == toNode &&
                numDeadheads < MAX_DEADHEADS)
          {
            if(CONNECTIONS.flags & CONNECTIONS_FLAG_DEADHEADTIME)
            {
              if(numDeadheads == 0)
              {
                if(CONNECTIONS.distance > 0.0)
                  distance = CONNECTIONS.distance;
                else
                  distance = GetStopDistance(prevLongitude, prevLatitude, longitude, latitude);
                distanceI = (long)distance;
                distanceF = (long)((float)((distance - distanceI) * 100));
                sprintf(szarString, "%2d.%02d", distanceI, distanceF);
                pad(szarString, 5);
                szarString[5] = '\0';
                if(CONNECTIONS.distance <= 0.0)
                {
                  bGotError = TRUE;
                  strncpy(tempString, NODES.abbrName, NODES_ABBRNAME_LENGTH);
                  trim(tempString, NODES_ABBRNAME_LENGTH);
                  sprintf(outputString,
                        "No distance on deadhead record (%s to %s) - %s used.\r\n",
                        fromNodeAbbr, toNodeAbbr, szarString);
                  _lwrite(hfErrorLog, outputString, strlen(outputString));
                }
                sprintf(outputString, "%4ld%4ld%s", fromStopNumber, toStopNumber, szarString);
                if(CONNECTIONS.fromSERVICESrecordID == NO_RECORD)
                  strcat(outputString, "0");
                else
                {
                  SERVICESKey0.recordID = CONNECTIONS.fromSERVICESrecordID;
                  btrieve(B_GETEQUAL, TMS_SERVICES, &SERVICES, &SERVICESKey0, 0);
                  strcat(outputString, ltoa(SERVICES.number, tempString, 10));
                }
                strcpy(deadheadData, "");
              }
              if(CONNECTIONS.flags & CONNECTIONS_FLAG_TWOWAY)
                bTwoWay = TRUE;
              if(CONNECTIONS.fromTimeOfDay == NO_TIME)
                CONNECTIONS.fromTimeOfDay = 0L;    // 1200A
              if(CONNECTIONS.toTimeOfDay == NO_TIME)
                CONNECTIONS.toTimeOfDay =  T1159P;
              sprintf(tempString, "%4ld%4ld%2ld", CONNECTIONS.fromTimeOfDay / 60,
                    CONNECTIONS.toTimeOfDay / 60, CONNECTIONS.connectionTime / 60);
              strcat(deadheadData, tempString);
              numDeadheads++;
            }
            rcode2 = btrieve(B_GETNEXT, TMS_CONNECTIONS, &CONNECTIONS, &CONNECTIONSKey1, 1);
          }
          if(numDeadheads > 0)
          {
            for(nL = numDeadheads; nL < MAX_DEADHEADS; nL++)
            {
              strcat(deadheadData, "          ");
            }
            strcat(outputString, deadheadData);
            strcat(outputString, "\r\n");
            _lwrite(hfOutputFile[0], outputString, strlen(outputString));
            if(bTwoWay)
            {
              strncpy(&tempString[0], &outputString[0], 4);
              strncpy(&outputString[0], &outputString[4], 4);
              strncpy(&outputString[4], &tempString[0], 4);
              _lwrite(hfOutputFile[0], outputString, strlen(outputString));
            }
          }
        }
      }
    }
    _lclose(hfOutputFile[0]);
  }
  StatusBar(-1L, -1L);
//
//  Timetable Info
//
#define MAX_NODESPERTRIP 9
#define BYTESPERTRIP     4
  if(RPTFLAGS.flags & TMSRPT12_TIMETABLEINFO)
  {
    typedef struct PATDATAStruct
    {
      long PATTERNNAMESrecordID;
      int  numNodesInPattern;
    } PATDATADef;
    PATDATADef PATData[5];
    char timetableData[BYTESPERTRIP * MAX_NODESPERTRIP + 1];
    char serviceName[SERVICES_NAME_LENGTH + 1];
    long previousPattern;
    long onTime;
    long offTime;
    long fromDeadhead;
    long toDeadhead;
    long lastTime;
    int  maxRunRecords;
    int  numRunRecords;
    int  runNumber;
    int  prevRunNumber;
    int  routeNumber;
    int  pathNumber;
    int  pathToUse;

    LoadString(hInst, TEXT_143, tempString, TEMPSTRING_LENGTH);
    StatusBarText(tempString);
//
//  Open the output file
//
   hfOutputFile[0] = _lcreat("timtable.hp", 0);
    if(hfOutputFile[0] == HFILE_ERROR)
    {
      LoadString(hInst, ERROR_202, szFormatString, sizeof(szFormatString));
      sprintf(szarString, szFormatString, tempString);
      MessageBeep(MB_ICONSTOP);
      MessageBox((HWND)NULL, szarString, TMS, MB_ICONSTOP);
      bFinishedOK = FALSE;
      goto done;
    }
//
//  Timetable Info file layout
//
//  Bytes   Description
//  ~~~~~   ~~~~~~~~~~~
//   1-4    Route number
//    5     Service sort number (1=Weekday, 2=Saturday, 3=Sunday)
//    6     Direction code (0 / 1)
//    7     Path number
//   8-11   Block number
//  12-14   Run number
//  15-50   9 x 4 bytes - Time at timepoint
//  51-54   Deadhead from node (as per C-Tran, 5-Mar-99)
//  55-58   Deadhead to node (as per C-Tran, 5-Mar-99)
//
//  Note: Due to the HP's inability to handle more than 9 nodes on a pattern,
//        this unload, if faced with the situation, will write out the first
//        nine timepoints.
//
//  Loop through the services
//
    serviceNumber = 1;
    rcode2 = btrieve(B_GETFIRST, TMS_SERVICES, &SERVICES, &SERVICESKey1, 1);
    while(rcode2 == 0 && serviceNumber <= 9)
    {
      if(StatusBarAbort())
      {
        goto done;
      }
      strncpy(serviceName, SERVICES.name, SERVICES_NAME_LENGTH);
      trim(serviceName, SERVICES_NAME_LENGTH);
//
//  Get all the runs for this service.  This section of code
//  was lifted from TMSRPT10.c.
//
      rcode2 = btrieve(B_STAT, TMS_RUNS, &BSTAT, dummy, 0);
      maxRunRecords = BSTAT.numRecords;
      HPRData = (HPRDDef *)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(HPRDDef) * maxRunRecords); 
      if(HPRData == NULL)
      {
        AllocationError(__FILE__, __LINE__, FALSE);
        goto done;
      }
      numRunRecords = 0;
      RUNSKey1.DIVISIONSrecordID = m_DivisionRecordID;
      RUNSKey1.SERVICESrecordID = SERVICES.recordID;
      RUNSKey1.runNumber = NO_RECORD;
      RUNSKey1.pieceNumber = NO_RECORD;
      rcode2 = btrieve(B_GETGREATER, TMS_RUNS, &RUNS, &RUNSKey1, 1);
      while(rcode2 == 0 &&
            RUNS.DIVISIONSrecordID == m_DivisionRecordID &&
            RUNS.SERVICESrecordID == SERVICES.recordID)
      {
        if(StatusBarAbort())
        {
          goto done;
        }
        LoadString(hInst, TEXT_042, szFormatString, sizeof(szFormatString));
        sprintf(tempString, szFormatString, RUNS.runNumber);
        StatusBarText(tempString);
        HPRData[numRunRecords].runNumber = RUNS.runNumber;
        HPRData[numRunRecords].pieceNumber = RUNS.pieceNumber;
        TRIPSKey0.recordID = RUNS.start.TRIPSrecordID;
        rcode2 = btrieve(B_GETEQUAL, TMS_TRIPS, &TRIPS, &TRIPSKey0, 0);
        HPRData[numRunRecords].blockNumber = TRIPS.standard.blockNumber;
        RunStartAndEnd(RUNS.start.TRIPSrecordID, RUNS.start.NODESrecordID,
              RUNS.end.TRIPSrecordID, RUNS.end.NODESrecordID, &onTime, &offTime);
        HPRData[numRunRecords].startNODESrecordID = RUNS.start.NODESrecordID;
        HPRData[numRunRecords].startTRIPSrecordID = RUNS.start.TRIPSrecordID;
        HPRData[numRunRecords].startTime = onTime;
        HPRData[numRunRecords].endNODESrecordID = RUNS.end.NODESrecordID;
        HPRData[numRunRecords].endTRIPSrecordID = RUNS.end.TRIPSrecordID;
        HPRData[numRunRecords].endTime = offTime;
        numRunRecords++;
        rcode2 = btrieve(B_GETNEXT, TMS_RUNS, &RUNS, &RUNSKey1, 1);
      }
      qsort((void *)HPRData, numRunRecords, sizeof(HPRDDef), sort_runs);
      sprintf(tempString, "Unloading %s timetables", serviceName);
      StatusBarText(tempString);
      for(nI = 0; nI < numRoutes; nI++)
      {
        if(StatusBarAbort())
        {
          goto done;
        }
        StatusBar((long)nI, (long)numRoutes);
        ROUTESKey0.recordID = ROUTErecordIDs[nI];
        btrieve(B_GETEQUAL, TMS_ROUTES, &ROUTES, &ROUTESKey0, 0);
        routeNumber = atoi(ROUTES.number);
        for(nJ = 0; nJ < 2; nJ++)
        {
          PATTERNSKey2.ROUTESrecordID = ROUTES.recordID;
          PATTERNSKey2.SERVICESrecordID = SERVICES.recordID;
          PATTERNSKey2.directionIndex = nJ;
          PATTERNSKey2.PATTERNNAMESrecordID = basePatternRecordID;
          PATTERNSKey2.nodeSequence = NO_RECORD;
          pathNumber = 0;
          previousPattern = NO_RECORD;
          rcode2 = btrieve(B_GETGREATEROREQUAL, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
          while(rcode2 == 0 &&
                PATTERNS.ROUTESrecordID == ROUTES.recordID &&
                PATTERNS.SERVICESrecordID == SERVICES.recordID &&
                PATTERNS.directionIndex == nJ &&
                pathNumber < MAX_PATTERNS)
          {
            if(!(PATTERNS.flags & PATTERNS_FLAG_BUSSTOP))
            {
              if(PATTERNS.PATTERNNAMESrecordID == previousPattern)
                PATData[pathNumber].numNodesInPattern++;
              else
              {
                if(previousPattern != NO_RECORD)
                {
                  pathNumber++;
                  if(pathNumber >= MAX_PATTERNS)
                  {
                    break;
                  }
                }
                previousPattern = PATTERNS.PATTERNNAMESrecordID;
                PATData[pathNumber].PATTERNNAMESrecordID = PATTERNS.PATTERNNAMESrecordID;
                PATData[pathNumber].numNodesInPattern = 1;
              }
            }
            rcode2 = btrieve(B_GETNEXT, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
          }
          if(previousPattern != NO_RECORD)
          {
            pathNumber++;
          }
          TRIPSKey1.ROUTESrecordID = ROUTES.recordID;
          TRIPSKey1.SERVICESrecordID = SERVICES.recordID;
          TRIPSKey1.directionIndex = nJ;
          TRIPSKey1.tripSequence = NO_TIME;
          rcode2 = btrieve(B_GETGREATEROREQUAL, TMS_TRIPS, &TRIPS, &TRIPSKey1, 1);
          while(rcode2 == 0 &&
                TRIPS.ROUTESrecordID == ROUTES.recordID &&
                TRIPS.SERVICESrecordID == SERVICES.recordID &&
                TRIPS.directionIndex == nJ)
          {
            pathToUse = NO_RECORD;            
            for(nK = 0; nK < pathNumber; nK++)  // This assumes that it's in the list
            {
              if(TRIPS.PATTERNNAMESrecordID == PATData[nK].PATTERNNAMESrecordID)
              {
                pathToUse = nK + 1;
                break;
              }
            }
            if(pathToUse != NO_RECORD)
            {
              GenerateTrip(TRIPS.ROUTESrecordID, TRIPS.SERVICESrecordID,
                    TRIPS.directionIndex, TRIPS.PATTERNNAMESrecordID,
                    TRIPS.timeAtMLP, GENERATETRIP_FLAG_DISPLAYERRORS, &GTResults);
              strcpy(timetableData, "");
              lastTime = NO_TIME;
              for(nK = 0; nK < MAX_NODESPERTRIP; nK++)
              {
                if(nK >= PATData[pathToUse - 1].numNodesInPattern)
                  strcpy(tempString, "    ");
                else
                {
                  sprintf(tempString, "%4ld", GTResults.tripTimes[nK] / 60);
                  lastTime = GTResults.tripTimes[nK];
                }
                strcat(timetableData, tempString);
              }
              if(TRIPS.standard.POGNODESrecordID == NO_RECORD || GTResults.tripTimes[0] == NO_TIME)
                fromDeadhead = 0L;
              else
              {
                NODESKey0.recordID = TRIPS.standard.POGNODESrecordID;
                btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
//                if(NODES.number > 10000)
//                  NODES.number /= 10000;
//                fromDeadhead = NODES.number;
                if(NODES.OBStopNumber > 0)
                {
                  fromDeadhead = NODES.OBStopNumber;
                }
                else if(NODES.IBStopNumber > 0)
                {
                  fromDeadhead = NODES.IBStopNumber;
                }
              }
              if(TRIPS.standard.PIGNODESrecordID == NO_RECORD || lastTime == NO_TIME)
              {
                toDeadhead = 0L;
              }
              else
              {
                NODESKey0.recordID = TRIPS.standard.PIGNODESrecordID;
                btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
//                if(NODES.number > 10000)
//                  NODES.number /= 10000;
//                toDeadhead = NODES.number;
                if(NODES.OBStopNumber > 0)
                {
                  toDeadhead = NODES.OBStopNumber;
                }
                else if(NODES.IBStopNumber > 0)
                {
                  toDeadhead = NODES.IBStopNumber;
                }
              }
              pad(timetableData, BYTESPERTRIP * MAX_NODESPERTRIP);
              timetableData[BYTESPERTRIP * MAX_NODESPERTRIP] = '\0';
//
//  GTResults.firstNODESrecordID - First node record ID
//          [1] - Time at first node
//          [2] - Last node record ID
//          [3] - Time at last node
//
//  Iteration 5:
//
//  Having finally convinced the C-Tran MIS manager of the error of his
//  ways, this section unloads a second (or more) timetable record whenever
//  the run number changes on a trip.
//
              prevRunNumber = NO_RECORD;
              for(bGotOne = FALSE, nK = 0; nK < numRunRecords; nK++)
              {
                if(TRIPS.recordID == HPRData[nK].startTRIPSrecordID ||
                      (TRIPS.standard.blockNumber == HPRData[nK].blockNumber &&
                       GTResults.firstNodeTime >= HPRData[nK].startTime &&
                       GTResults.firstNodeTime <= HPRData[nK].endTime))
                {
                  runNumber = HPRData[nK].runNumber;
                  bGotOne = TRUE;
                  if(runNumber != prevRunNumber)
                  {
                    strcpy(tempString, ConvertCTranRunNumber(runNumber));
                    sprintf(outputString, "%4d%1d%1d%1d%4ld%s%s%4ld%4ld\r\n",
                          routeNumber, serviceNumber, nJ, pathToUse, TRIPS.standard.blockNumber,
                          tempString, timetableData, fromDeadhead, toDeadhead);
                    _lwrite(hfOutputFile[0], outputString, strlen(outputString));
                    prevRunNumber = runNumber;
                  }
                }
              }
              if(!bGotOne)
              {
                bGotError = TRUE;
                strncpy(tempString, NODES.abbrName, NODES_ABBRNAME_LENGTH);
                trim(tempString, NODES_ABBRNAME_LENGTH);
                sprintf(outputString,
                      "No run found to cover trip starting at %s on block %ld, route %d %s \r\n",
                      Tchar(GTResults.firstNodeTime), TRIPS.standard.blockNumber, routeNumber, serviceName);
                _lwrite(hfErrorLog, outputString, strlen(outputString));
              }
            }  // pathToUse != NO_RECORD
            rcode2 = btrieve(B_GETNEXT, TMS_TRIPS, &TRIPS, &TRIPSKey1, 1);
          }
        }
      }
      rcode2 = btrieve(B_GETNEXT, TMS_SERVICES, &SERVICES, &SERVICESKey1, 1);
      serviceNumber++;
    }
    _lclose(hfOutputFile[0]);
  }
  StatusBar(-1L, -1L);
//
//  Operator Bid Info
//
  if(RPTFLAGS.flags & TMSRPT12_OPERATORBIDINFO)
  {

    LoadString(hInst, TEXT_144, tempString, TEMPSTRING_LENGTH);
    StatusBarText(tempString);
//
//  Open the output file
//
    hfOutputFile[0] = _lcreat("operator.hp", 0);
    if(hfOutputFile[0] == HFILE_ERROR)
    {
      LoadString(hInst, ERROR_202, szFormatString, sizeof(szFormatString));
      sprintf(szarString, szFormatString, tempString);
      MessageBeep(MB_ICONSTOP);
      MessageBox((HWND)NULL, szarString, TMS, MB_ICONSTOP);
      bFinishedOK = FALSE;
      goto done;
    }
//
//  Operator Info file layout
//
//  Bytes   Description
//  ~~~~~   ~~~~~~~~~~~
//   1-3    Run number
//    4     Day (1=Monday, 2=Tuesday, ..., 7=Sunday)
//   5-10   Operator number
//
    rcode2 = btrieve(B_STAT, TMS_ROSTER, &BSTAT, dummy, 0);
    if(rcode2 == 0)
    {
      numRecords = (int)BSTAT.numRecords;
      nI = 0;
      rcode2 = btrieve(B_GETFIRST, TMS_ROSTER, &ROSTER, &ROSTERKey0, 0);
      while(rcode2 == 0)
      {
        DRIVERSKey0.recordID = ROSTER.DRIVERSrecordID;
        btrieve(B_GETEQUAL, TMS_DRIVERS, &DRIVERS, &DRIVERSKey0, 0);
        strncpy(tempString, DRIVERS.badgeNumber, DRIVERS_BADGENUMBER_LENGTH);
        trim(tempString, DRIVERS_BADGENUMBER_LENGTH);
        driverNumber = atol(tempString);
        for(nJ = 0; nJ < 7; nJ++)
        {
          if(StatusBarAbort())
            goto done;
          if(ROSTER.WEEK[m_RosterWeek].RUNSrecordIDs[nJ] == NO_RECORD)
            continue;
          RUNSKey0.recordID = ROSTER.WEEK[m_RosterWeek].RUNSrecordIDs[nJ];
          rcode2 = btrieve(B_GETEQUAL, TMS_RUNS, &RUNS, &RUNSKey0, 0);
          if(rcode2 != 0)
          {
            bGotError = TRUE;
            sprintf(outputString,
                  "Assigned run missing from runs table.  Driver = %ld, day = %d.\r\n",
                  driverNumber, nJ + 1);
            _lwrite(hfErrorLog, outputString, strlen(outputString));
          }
          else
          {
            strcpy(tempString, ConvertCTranRunNumber(RUNS.runNumber));
            sprintf(outputString, "%s%1d%6ld\r\n", tempString, nJ + 1, driverNumber);
            _lwrite(hfOutputFile[0], outputString, strlen(outputString));
          }
        }
        StatusBar((long)nI++, (long)numRecords);
        rcode2 = btrieve(B_GETNEXT, TMS_ROSTER, &ROSTER, &ROSTERKey0, 0);
      }
    }
    _lclose(hfOutputFile[0]);
  }
  StatusBar(-1L, -1L);
//
//  Runcut Info
//
  if(RPTFLAGS.flags & TMSRPT12_RUNCUTINFO)
  {
    GetConnectionTimeDef GCTData;
    PROPOSEDRUNDef PROPOSEDRUN;
    COSTDef COST;
    float distance;
    long  blockNumbers[MAXPIECES];
    long  onTimes[MAXPIECES];
    long  offTime;
    long  cutAsRuntype;
    long  runNumber;
    long  padTime;
    long  reportTime;
    long  travelTime;
    long  dwellTime;
    long  serviceRecordID;
    long  startNode;
    long  endNode;

//
//  Open the output file
//
    hfOutputFile[0] = _lcreat("runcut.hp", 0);
    if(hfOutputFile[0] == HFILE_ERROR)
    {
      LoadString(hInst, ERROR_202, szFormatString, sizeof(szFormatString));
      sprintf(szarString, szFormatString, tempString);
      MessageBeep(MB_ICONSTOP);
      MessageBox((HWND)NULL, szarString, TMS, MB_ICONSTOP);
      bFinishedOK = FALSE;
      goto done;
    }
//
//  Runcut Info file layout
//
//  Bytes   Description
//  ~~~~~   ~~~~~~~~~~~
//   1-3    Run number
//    4     Service sort number (1=Weekday, 2=Saturday, 3=Sunday)
//   5-8    Start time
//   9-12   Block number
//  13-14   Report time
//  15-16   Pad time
//  17-18   Travel time
//  19-20   Dwell time
//  21-24   End time (as per C-Tran, 5-Mar-99)
//  25-28   Start node number (as per C-Tran, 22-Jan-03)
//  29-32   End node number (as per C-Tran, 22-Jan-03)
//
//  Note: This code assumes that the first three services in the
//        services file are Weekday, Saturday, and Sunday, in that order
//
    StatusBarEnd();
    rcode2 = btrieve(B_GETFIRST, TMS_SERVICES, &SERVICES, &SERVICESKey1, 1);
    serviceNumber = 1;
    serviceRecordID = SERVICES.recordID;
    if(bUseDynamicTravels && bUseCISPlan)
    {
      LoadString(hInst, TEXT_154, tempString, TEMPSTRING_LENGTH);
      StatusBarText(tempString);
      CISfree();
      if(!CISbuild(FALSE, TRUE))
      {
        bFinishedOK = FALSE;
        goto done;
      }
    }
    StatusBarStart(hWndMain, pszReportName);
    while(rcode2 == 0 && serviceNumber <= 9)
    {
      if(bUseDynamicTravels && !bUseCISPlan)
      {
        LoadString(hInst, TEXT_135, tempString, TEMPSTRING_LENGTH);
        StatusBarText(tempString);
        InitTravelMatrix(serviceRecordID, FALSE);
      }
      LoadString(hInst, TEXT_145, tempString, TEMPSTRING_LENGTH);
      StatusBarText(tempString);
      RUNSKey1.DIVISIONSrecordID = m_DivisionRecordID;
      RUNSKey1.SERVICESrecordID = serviceRecordID;
      RUNSKey1.runNumber = NO_RECORD;
      RUNSKey1.pieceNumber = NO_RECORD;
      rcode2 = btrieve(B_GETGREATEROREQUAL, TMS_RUNS, &RUNS, &RUNSKey1, 1);
      while(rcode2 == 0 &&
            RUNS.DIVISIONSrecordID == m_DivisionRecordID &&
            RUNS.SERVICESrecordID == serviceRecordID)
      {
        if(StatusBarAbort())
          goto done;
        runNumber = RUNS.runNumber;
        pieceNumber = 0;
        while(rcode2 == 0 &&
              RUNS.runNumber == runNumber &&
              RUNS.SERVICESrecordID == serviceRecordID)
        {
          if(StatusBarAbort())
            goto done;
          TRIPSKey0.recordID = RUNS.start.TRIPSrecordID;
          btrieve(B_GETEQUAL, TMS_TRIPS, &TRIPS, &TRIPSKey0, 0);
//
//  Save the block number
//
          blockNumbers[pieceNumber] = TRIPS.standard.blockNumber;
//
//  Generate the start trip
//
          GenerateTrip(TRIPS.ROUTESrecordID, TRIPS.SERVICESrecordID,
                TRIPS.directionIndex, TRIPS.PATTERNNAMESrecordID,
                TRIPS.timeAtMLP, GENERATETRIP_FLAG_DISPLAYERRORS, &GTResults);
//
//  Is the node a pullout?
//
          if(TRIPS.standard.POGNODESrecordID == RUNS.start.NODESrecordID)
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
            onTimes[pieceNumber] = GTResults.firstNodeTime -
                  GetConnectionTime(GCT_FLAG_DEADHEADTIME, &GCTData, &distance);
            distance = (float)fabs((double)distance);
          }
//
//  Nope - Find the node on the pattern
//
          else
          {
            PATTERNSKey2.ROUTESrecordID = TRIPS.ROUTESrecordID;
            PATTERNSKey2.SERVICESrecordID = TRIPS.SERVICESrecordID;
            PATTERNSKey2.directionIndex = TRIPS.directionIndex;
            PATTERNSKey2.PATTERNNAMESrecordID = TRIPS.PATTERNNAMESrecordID;
            PATTERNSKey2.nodeSequence = NO_RECORD;
            rcode2 = btrieve(B_GETGREATEROREQUAL, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
            counter = 0;
            while(rcode2 == 0 &&
                  PATTERNS.ROUTESrecordID == TRIPS.ROUTESrecordID &&
                  PATTERNS.SERVICESrecordID == TRIPS.SERVICESrecordID &&
                  PATTERNS.directionIndex == TRIPS.directionIndex &&
                  PATTERNS.PATTERNNAMESrecordID == TRIPS.PATTERNNAMESrecordID)
            {
              if(StatusBarAbort())
                goto done;
              if((bFound = PATTERNS.NODESrecordID == RUNS.start.NODESrecordID) == TRUE)
                break;
              if(!(PATTERNS.flags & PATTERNS_FLAG_BUSSTOP))
                counter++;
              rcode2 = btrieve(B_GETNEXT, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
            }
            onTimes[pieceNumber] = bFound ? GTResults.tripTimes[counter] : NO_TIME;
          }
//
//  Generate the end trip
//
          TRIPSKey0.recordID = RUNS.end.TRIPSrecordID;
          btrieve(B_GETEQUAL, TMS_TRIPS, &TRIPS, &TRIPSKey0, 0);
          GenerateTrip(TRIPS.ROUTESrecordID, TRIPS.SERVICESrecordID,
                TRIPS.directionIndex, TRIPS.PATTERNNAMESrecordID,
                TRIPS.timeAtMLP, GENERATETRIP_FLAG_DISPLAYERRORS, &GTResults);
//
//  Is the node a pullin?
//
          if(TRIPS.standard.PIGNODESrecordID == RUNS.end.NODESrecordID)
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
            offTime = GTResults.lastNodeTime +
                  GetConnectionTime(GCT_FLAG_DEADHEADTIME, &GCTData, &distance);
            distance = (float)fabs((double)distance);
          }
//
//  Nope - Find the node on the pattern
//
          else
          {
            PATTERNSKey2.ROUTESrecordID = TRIPS.ROUTESrecordID;
            PATTERNSKey2.SERVICESrecordID = TRIPS.SERVICESrecordID;
            PATTERNSKey2.directionIndex = TRIPS.directionIndex;
            PATTERNSKey2.PATTERNNAMESrecordID = TRIPS.PATTERNNAMESrecordID;
            PATTERNSKey2.nodeSequence = NO_RECORD;
            rcode2 = btrieve(B_GETGREATEROREQUAL, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
            counter = 0;
            while(rcode2 == 0 &&
                  PATTERNS.ROUTESrecordID == TRIPS.ROUTESrecordID &&
                  PATTERNS.SERVICESrecordID == TRIPS.SERVICESrecordID &&
                  PATTERNS.directionIndex == TRIPS.directionIndex &&
                  PATTERNS.PATTERNNAMESrecordID == TRIPS.PATTERNNAMESrecordID)
            {
              if(StatusBarAbort())
                goto done;
              if((bFound = PATTERNS.NODESrecordID == RUNS.end.NODESrecordID) == TRUE)
                break;
              if(!(PATTERNS.flags & PATTERNS_FLAG_BUSSTOP))
                counter++;
              rcode2 = btrieve(B_GETNEXT, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
            }
            offTime = bFound ? GTResults.tripTimes[counter] : NO_TIME;
          }
//
//  Set up PROPOSEDRUN
//
          PROPOSEDRUN.piece[pieceNumber].fromTime = onTimes[pieceNumber];
          PROPOSEDRUN.piece[pieceNumber].fromNODESrecordID = RUNS.start.NODESrecordID;
          PROPOSEDRUN.piece[pieceNumber].fromTRIPSrecordID = RUNS.start.TRIPSrecordID;
          PROPOSEDRUN.piece[pieceNumber].toTime = offTime;
          PROPOSEDRUN.piece[pieceNumber].toNODESrecordID = RUNS.end.NODESrecordID;
          PROPOSEDRUN.piece[pieceNumber].toTRIPSrecordID = RUNS.end.TRIPSrecordID;
          PROPOSEDRUN.piece[pieceNumber].prior.startTime = RUNS.prior.startTime;
          PROPOSEDRUN.piece[pieceNumber].prior.endTime = RUNS.prior.endTime;
          PROPOSEDRUN.piece[pieceNumber].after.startTime = RUNS.after.startTime;
          PROPOSEDRUN.piece[pieceNumber].after.endTime = RUNS.prior.endTime;
//
//  And the runtype
//
          cutAsRuntype = RUNS.cutAsRuntype;
//
//  Get the next run record
//
          rcode2 = btrieve(B_GETNEXT, TMS_RUNS, &RUNS, &RUNSKey1, 1);
          pieceNumber++;
        }
//
//  Cost the run
//
        PROPOSEDRUN.numPieces = pieceNumber;
        RunCoster(&PROPOSEDRUN, cutAsRuntype, &COST);
//
//  And fill out and write the output record
//
        for(nI = 0; nI < PROPOSEDRUN.numPieces; nI++)
        {
//
//  Pad time
//
          padTime = nI == PROPOSEDRUN.numPieces - 1 ? COST.TOTAL.makeUpTime : 0;
//
//  Report and travel
//
          reportTime = COST.PIECECOST[nI].reportTime;
          dwellTime = COST.TRAVEL[nI].startDwellTime + COST.TRAVEL[nI].endDwellTime;
          travelTime = COST.TRAVEL[nI].startTravelTime + COST.TRAVEL[nI].endTravelTime;
          travelTime -= dwellTime;  // Split them out
//
//  Start and end locations
//
          NODESKey0.recordID = PROPOSEDRUN.piece[nI].fromNODESrecordID;
          btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
//          if(NODES.number > 10000)
//            NODES.number /= 10000;
//          startNode = NODES.number;
          if(NODES.OBStopNumber > 0)
          {
            startNode = NODES.OBStopNumber;
          }
          else if(NODES.IBStopNumber > 0)
          {
            startNode = NODES.IBStopNumber;
          }
          if(PROPOSEDRUN.piece[nI].fromNODESrecordID ==
                 PROPOSEDRUN.piece[nI].toNODESrecordID)
          {
            endNode = startNode;
          }
          else
          {
            NODESKey0.recordID = PROPOSEDRUN.piece[nI].toNODESrecordID;
            btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
//            if(NODES.number > 10000)
//              NODES.number /= 10000;
//            endNode = NODES.number;
            if(NODES.OBStopNumber > 0)
            {
              endNode = NODES.OBStopNumber;
            }
            else if(NODES.IBStopNumber > 0)
            {
              endNode = NODES.IBStopNumber;
            }
          }
          strcpy(tempString, ConvertCTranRunNumber(runNumber));
          sprintf(outputString, "%s%1d%4ld%4ld%2ld%2ld%2ld%2ld%4ld%4ld%4ld\r\n",
                tempString, serviceNumber, onTimes[nI] / 60L, blockNumbers[nI],
                reportTime / 60L, padTime / 60L, travelTime / 60L, dwellTime / 60L,
                PROPOSEDRUN.piece[nI].toTime / 60L, startNode, endNode);
          _lwrite(hfOutputFile[0], outputString, strlen(outputString));
        }
      }
//
//  Get the next service
//
      SERVICESKey0.recordID = serviceRecordID;
      btrieve(B_GETEQUAL, TMS_SERVICES, &SERVICES, &SERVICESKey0, 0);
      btrieve(B_GETPOSITION, TMS_SERVICES, &SERVICES, &SERVICESKey0, 0);
      btrieve(B_GETDIRECT, TMS_SERVICES, &SERVICES, &SERVICESKey1, 1);
      rcode2 = btrieve(B_GETNEXT, TMS_SERVICES, &SERVICES, &SERVICESKey1, 1);
      serviceNumber++;
      serviceRecordID = SERVICES.recordID;
    }
    _lclose(hfOutputFile[0]);
  }
  StatusBar(-1L, -1L);
//
//  Stop Distance Info
//
  if(RPTFLAGS.flags & TMSRPT12_STOPDISTANCEINFO)
  {
    float distance;
    BOOL bGotOne;
    long stopNumber;
    long previousNode;
    int  routeNumber;
    int  pathNumber;
    int  numNodesInPattern;

    LoadString(hInst, TEXT_146, tempString, TEMPSTRING_LENGTH);
    StatusBarText(tempString);
//
//  Open the output file
//
    hfOutputFile[0] = _lcreat("stopdist.hp", 0);
    if(hfOutputFile[0] == HFILE_ERROR)
    {
      LoadString(hInst, ERROR_202, szFormatString, sizeof(szFormatString));
      sprintf(szarString, szFormatString, tempString);
      MessageBeep(MB_ICONSTOP);
      MessageBox((HWND)NULL, szarString, TMS, MB_ICONSTOP);
      bFinishedOK = FALSE;
      goto done;
    }
//
//  Stop Distance Info file layout
//
//  Bytes   Description
//  ~~~~~   ~~~~~~~~~~~
//   1-4    Route number
//    5     Service code
//    6     Direction code
//    7     Path number
//   8-10   Sequence number
//  11-14   Stop number
//  15-19   Stop distance
//    20    Time point
//
    for(nI = 0; nI < numRoutes; nI++)
    {
      if(StatusBarAbort())
        goto done;
      StatusBar((long)nI, (long)numRoutes);
      ROUTESKey0.recordID = ROUTErecordIDs[nI];
      btrieve(B_GETEQUAL, TMS_ROUTES, &ROUTES, &ROUTESKey0, 0);
      strncpy(tempString, ROUTES.number, ROUTES_NUMBER_LENGTH);
      trim(tempString, ROUTES_NUMBER_LENGTH);
      routeNumber = atoi(tempString);
      serviceNumber = 1;
      rcode2 = btrieve(B_GETFIRST, TMS_SERVICES, &SERVICES, &SERVICESKey1, 1);
      while(rcode2 == 0 && serviceNumber <= 9)
      {
        for(nJ = 0; nJ < 2; nJ++)
        {
          if(StatusBarAbort())
            goto done;
          PATTERNSKey2.ROUTESrecordID = ROUTES.recordID;
          PATTERNSKey2.SERVICESrecordID = SERVICES.recordID;
          PATTERNSKey2.directionIndex = nJ;
          PATTERNSKey2.PATTERNNAMESrecordID = basePatternRecordID;
          PATTERNSKey2.nodeSequence = NO_RECORD;
          pathNumber = 0;
          previousNode = NO_RECORD;
          rcode2 = btrieve(B_GETGREATEROREQUAL, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
          bGotOne = FALSE;
          while(rcode2 == 0 &&
                PATTERNS.ROUTESrecordID == ROUTES.recordID &&
                PATTERNS.SERVICESrecordID == SERVICES.recordID &&
                PATTERNS.directionIndex == nJ &&
                PATTERNS.PATTERNNAMESrecordID == basePatternRecordID)
          {
            distance = (float)0.0;
            if(!bGotOne)
            {
              numNodesInPattern = 1;
              bGotOne = TRUE;
            }
            else
            {
              CONNECTIONSKey1.fromNODESrecordID = previousNode;
              CONNECTIONSKey1.toNODESrecordID = PATTERNS.NODESrecordID;
              CONNECTIONSKey1.fromTimeOfDay = NO_TIME;
              rcode2 = btrieve(B_GETGREATEROREQUAL, TMS_CONNECTIONS, &CONNECTIONS, &CONNECTIONSKey1, 1);
              while(rcode2 == 0 &&
                    CONNECTIONS.fromNODESrecordID == previousNode &&
                    CONNECTIONS.toNODESrecordID == PATTERNS.NODESrecordID)
              {
                if(CONNECTIONS.flags & CONNECTIONS_FLAG_STOPSTOP)
                {
                  distance = CONNECTIONS.distance;
                  break;
                }
                rcode2 = btrieve(B_GETNEXT, TMS_CONNECTIONS, &CONNECTIONS, &CONNECTIONSKey1, 1);
              }
              numNodesInPattern++;
            }
            NODESKey0.recordID = PATTERNS.NODESrecordID;
            btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
//            stopNumber = NODES.number;
//            if(stopNumber > 10000)
//              stopNumber /= 10000;
            if(NODES.flags & NODES_FLAG_STOP)
            {
              stopNumber = NODES.number;
            }
            else
            {
              if(NODES.OBStopNumber > 0)
              {
                stopNumber = NODES.OBStopNumber;
              }
              else if(NODES.IBStopNumber > 0)
              {
                stopNumber = NODES.IBStopNumber;
              }
            }
            sprintf(outputString, "%4d%1d%1d%1d%3d%4ld%5.2f%s\r\n",
                  routeNumber, serviceNumber, nJ, pathNumber,
                  numNodesInPattern, stopNumber, distance,
                  (NODES.flags & NODES_FLAG_STOP ? "N" : "Y"));
            _lwrite(hfOutputFile[0], outputString, strlen(outputString));
            previousNode = PATTERNS.NODESrecordID;
            rcode2 = btrieve(B_GETNEXT, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
          }
        }
        rcode2 = btrieve(B_GETNEXT, TMS_SERVICES, &SERVICES, &SERVICESKey1, 1);
        serviceNumber++;
      }
    }
    _lclose(hfOutputFile[0]);
  }
  StatusBar(-1L, -1L);
//
//  All done
//
  bFinishedOK = TRUE;

  done:
    chdir("..");
    TMSHeapFree(HPNODES);
    TMSHeapFree(HPRData);
    StatusBarEnd();
    strcpy(tempString, "HP Unload files were written to:\n");
    strcpy(szarString, szDatabaseFileName);
    if((ptr = strrchr(szarString, '\\')) != NULL)
    {
      *ptr = '\0';
    }
    strcat(tempString, szarString);
    MessageBox(hWndMain, tempString, TMS, MB_OK);
    if(!bFinishedOK)
    {
      TMSError((HWND)NULL, MB_ICONINFORMATION, ERROR_227, (HANDLE)NULL);
    }
    if(bGotError)
    {
      TMSError((HWND)NULL, MB_ICONINFORMATION, ERROR_228, (HANDLE)NULL);
    }
    _lclose(hfErrorLog);

  return(bFinishedOK);
}


BOOL CALLBACK HPUNLOADMsgProc(HWND hWndDlg, UINT Message, WPARAM wParam, LPARAM lParam)
{
  static HANDLE hCtlROUTE;
  static HANDLE hCtlSTOPS;
  static HANDLE hCtlPATH;
  static HANDLE hCtlDEADHEAD;
  static HANDLE hCtlTIMETABLE;
  static HANDLE hCtlOPERATORBID;
  static HANDLE hCtlRUNCUT;
  static HANDLE hCtlSTOPDISTANCE;
  static RPTFLAGSDef *RPTFLAGS;

  switch(Message)
  {
//
//  WM_INITDIALOG - Initialize the dialog
//
    case WM_INITDIALOG:
      RPTFLAGS = (RPTFLAGSDef *)lParam;
      if(RPTFLAGS == NULL)
      {
        SendMessage(hWndDlg, WM_COMMAND, MAKEWPARAM(IDCANCEL, 0), (LPARAM)0);
        break;
      }
//
//  Set up handles to the controls
//
      hCtlROUTE = GetDlgItem(hWndDlg, HPUNLOAD_ROUTE);
      hCtlSTOPS = GetDlgItem(hWndDlg, HPUNLOAD_STOPS);
      hCtlPATH = GetDlgItem(hWndDlg, HPUNLOAD_PATH);
      hCtlDEADHEAD = GetDlgItem(hWndDlg, HPUNLOAD_DEADHEAD);
      hCtlTIMETABLE = GetDlgItem(hWndDlg, HPUNLOAD_TIMETABLE);
      hCtlOPERATORBID = GetDlgItem(hWndDlg, HPUNLOAD_OPERATORBID);
      hCtlRUNCUT = GetDlgItem(hWndDlg, HPUNLOAD_RUNCUT);
      hCtlSTOPDISTANCE = GetDlgItem(hWndDlg, HPUNLOAD_STOPDISTANCE);
//
//  Set the dialog title
//
      SendMessage(hWndDlg, WM_GETTEXT, (WPARAM)sizeof(szFormatString), (LONG)(LPSTR)szFormatString);
      sprintf(tempString, szFormatString, TMSRPT[RPTFLAGS->nReportNumber].szReportName);
      SendMessage(hWndDlg, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)tempString);
//
//  And everything's on by default
//
      SendMessage(hCtlROUTE, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
      SendMessage(hCtlSTOPS, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
      SendMessage(hCtlPATH, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
      SendMessage(hCtlDEADHEAD, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
      SendMessage(hCtlTIMETABLE, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
      SendMessage(hCtlOPERATORBID, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
      SendMessage(hCtlRUNCUT, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
      SendMessage(hCtlSTOPDISTANCE, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
      break;

    case WM_CLOSE:
      SendMessage(hWndDlg, WM_COMMAND, MAKEWPARAM(IDCANCEL, 0), (LPARAM)0);
      break;

    case WM_COMMAND:
      switch(wParam)
      {
        case IDCANCEL:
          EndDialog(hWndDlg, FALSE);
          break;
//
//  IDHELP
//
        case IDHELP:
          WinHelp(hWndDlg, szarHelpFile, HELP_CONTEXT, Technical_Support);
          break;

        case IDOK:
          RPTFLAGS->flags = 0;
          if(SendMessage(hCtlROUTE, BM_GETCHECK, (WPARAM)0, (LPARAM)0))
            RPTFLAGS->flags |= TMSRPT12_ROUTEINFO;
          if(SendMessage(hCtlSTOPS, BM_GETCHECK, (WPARAM)0, (LPARAM)0))
            RPTFLAGS->flags |= TMSRPT12_STOPSINFO;
          if(SendMessage(hCtlPATH, BM_GETCHECK, (WPARAM)0, (LPARAM)0))
            RPTFLAGS->flags |= TMSRPT12_PATHINFO;
          if(SendMessage(hCtlDEADHEAD, BM_GETCHECK, (WPARAM)0, (LPARAM)0))
            RPTFLAGS->flags |= TMSRPT12_DEADHEADINFO;
          if(SendMessage(hCtlTIMETABLE, BM_GETCHECK, (WPARAM)0, (LPARAM)0))
            RPTFLAGS->flags |= TMSRPT12_TIMETABLEINFO;
          if(SendMessage(hCtlOPERATORBID, BM_GETCHECK, (WPARAM)0, (LPARAM)0))
            RPTFLAGS->flags |= TMSRPT12_OPERATORBIDINFO;
          if(SendMessage(hCtlRUNCUT, BM_GETCHECK, (WPARAM)0, (LPARAM)0))
            RPTFLAGS->flags |= TMSRPT12_RUNCUTINFO;
          if(SendMessage(hCtlSTOPDISTANCE, BM_GETCHECK, (WPARAM)0, (LPARAM)0))
            RPTFLAGS->flags |= TMSRPT12_STOPDISTANCEINFO;
          EndDialog(hWndDlg, TRUE);
          break;
      }
      break;

    default:
      return FALSE;
  }
  return TRUE;
}

float GetStopDistance(float fromLong, float fromLat, float toLong, float toLat)
{
  if(fromLong == 0.0 || fromLat == 0.0 || toLong == 0.0 || toLat == 0.0)
    return((float)0.0);
  else
    return((float)GreatCircleDistance(fromLong, fromLat, toLong, toLat));
}

char *ConvertCTranRunNumber(long runNumber)
{
  static char output[4];
  char   runAlpha;

  if(runNumber < 200)
    runAlpha = 'A';
  else if(runNumber < 300)
    runAlpha = 'B';
  else if(runNumber < 400)
    runAlpha = 'C';
  else if(runNumber < 500)
    runAlpha = 'D';
  else if(runNumber < 800)
    runAlpha = 'M';
  else
    runAlpha = 'U';
  sprintf(output, "%c%02ld", runAlpha, runNumber % 100);

  return(output);
}
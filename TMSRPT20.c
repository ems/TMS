//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
//
//  TMSRPT20() - Connections Overlap Report
//
#include "TMSHeader.h"

int   CheckTrip(long, long, int, long, long, GenerateTripDef *);
long  CheckRuntime(GetConnectionTimeDef *);
long  CheckRuntime1(int, int, GetConnectionTimeDef *);
HFILE hfOutputFile;

BOOL FAR TMSRPT20(TMSRPTPassedDataDef *pPassedData)
{
  GenerateTripDef GTResults;
  REPORTPARMSDef REPORTPARMS;
  BOOL  bKeepGoing = FALSE;
  BOOL  bRC;
  char  routeNumberAndName[ROUTES_NUMBER_LENGTH + 3 + ROUTES_NAME_LENGTH + 1];
  char  serviceName[SERVICES_NAME_LENGTH + 1];
  char  directionName[DIRECTIONS_LONGNAME_LENGTH + 1];
  char  firstNode[NODES_ABBRNAME_LENGTH + 1];
  char  lastNode[NODES_ABBRNAME_LENGTH + 1];
  long  statbarTotal;
  int   nI;
  int   nJ;
  int   nK;
  int   rcode2;
  int   maxNodes = 10;

  pPassedData->nReportNumber = 19;
  pPassedData->numDataFiles = 1;
//
//  See what he wants
//
  REPORTPARMS.nReportNumber = pPassedData->nReportNumber;
  REPORTPARMS.ROUTESrecordID = m_RouteRecordID;
  REPORTPARMS.SERVICESrecordID = m_ServiceRecordID;
  REPORTPARMS.pRouteList = NULL;
  REPORTPARMS.pServiceList = NULL;
  REPORTPARMS.flags = RPFLAG_ROUTES | RPFLAG_SERVICES;
  bRC = DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_RPTPARMS),
        hWndMain, (DLGPROC)RPTPARMSMsgProc, (LPARAM)&REPORTPARMS);
  if(!bRC)
  {
    return(FALSE);
  }
//
//  Build the list of routes and services
//
  if(REPORTPARMS.numRoutes == 0 || REPORTPARMS.numServices == 0)
  {
    goto deallocate;
  }
//
//  Open the output file
//
  strcpy(tempString, szReportsTempFolder);
  strcat(tempString, "\\TMSRPT20.txt");
  hfOutputFile = _lcreat(tempString, 0);
  if(hfOutputFile == HFILE_ERROR)
  {
    LoadString(hInst, ERROR_202, szFormatString, sizeof(szFormatString));
    sprintf(szarString, szFormatString, tempString);
    MessageBeep(MB_ICONSTOP);
    MessageBox((HWND)NULL, szarString, TMS, MB_ICONSTOP);
    goto deallocate;
  }
  strcpy(pPassedData->szReportDataFile[0], tempString);
//
//  Check all existing connections for bad time fields
//
  strcpy(tempString, "Performing time of day integrity check...\r\n");
  StatusBarText(tempString);
  _lwrite(hfOutputFile, tempString, strlen(tempString));
  rcode2 = btrieve(B_GETFIRST, TMS_CONNECTIONS, &CONNECTIONS, &CONNECTIONSKey1, 1);
  while(rcode2 == 0)
  {
    if(StatusBarAbort())
    {
      bKeepGoing = FALSE;
      goto deallocate;
    }
    if(CONNECTIONS.fromTimeOfDay > CONNECTIONS.toTimeOfDay)
    {
      NODESKey0.recordID = CONNECTIONS.fromNODESrecordID;
      btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
      strncpy(firstNode, NODES.abbrName, NODES_ABBRNAME_LENGTH);
      trim(firstNode, NODES_ABBRNAME_LENGTH);
      NODESKey0.recordID = CONNECTIONS.toNODESrecordID;;
      btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
      strncpy(lastNode, NODES.abbrName, NODES_ABBRNAME_LENGTH);
      trim(lastNode, NODES_ABBRNAME_LENGTH);
      sprintf(tempString, "  From time greater than to time on %s to %s:", firstNode, lastNode);
      strcat(tempString, " From time = ");
      strcat(tempString, Tchar(CONNECTIONS.fromTimeOfDay));
      strcat(tempString, " To time = ");
      strcat(tempString, Tchar(CONNECTIONS.toTimeOfDay));
      strcat(tempString, "\r\n");
      _lwrite(hfOutputFile, tempString, strlen(tempString));
      sprintf(tempString, "   RecordID is %ld\r\n", CONNECTIONS.recordID);
      _lwrite(hfOutputFile, tempString, strlen(tempString));
    }
    if(CONNECTIONS.fromTimeOfDay > T1159P ||
          CONNECTIONS.toTimeOfDay > T1159P)
    {
      NODESKey0.recordID = CONNECTIONS.fromNODESrecordID;
      btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
      strncpy(firstNode, NODES.abbrName, NODES_ABBRNAME_LENGTH);
      trim(firstNode, NODES_ABBRNAME_LENGTH);
      NODESKey0.recordID = CONNECTIONS.toNODESrecordID;;
      btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
      strncpy(lastNode, NODES.abbrName, NODES_ABBRNAME_LENGTH);
      trim(lastNode, NODES_ABBRNAME_LENGTH);
      sprintf(tempString, "  Invalid time of day on %s to %s:", firstNode, lastNode);
      if(CONNECTIONS.fromTimeOfDay > T1159P)
      {
        strcat(tempString, " From time = ");
        strcat(tempString, Tchar(CONNECTIONS.fromTimeOfDay));
      }
      if(CONNECTIONS.toTimeOfDay > T1159P)
      {
        strcat(tempString, " To time = ");
        strcat(tempString, Tchar(CONNECTIONS.toTimeOfDay));
      }
      strcat(tempString, "\r\n");
      _lwrite(hfOutputFile, tempString, strlen(tempString));
      sprintf(tempString, "   RecordID is %ld\r\n", CONNECTIONS.recordID);
      _lwrite(hfOutputFile, tempString, strlen(tempString));
    }
    rcode2 = btrieve(B_GETNEXT, TMS_CONNECTIONS, &CONNECTIONS, &CONNECTIONSKey1, 1);
  }
  strcpy(tempString, "Time of day integrity check complete\r\n");
  _lwrite(hfOutputFile, tempString, strlen(tempString));
//
//  Loop through the services
//
  LoadString(hInst, TEXT_117, tempString, TEMPSTRING_LENGTH);
  StatusBarStart(hWndMain, tempString);
  statbarTotal = REPORTPARMS.numRoutes * REPORTPARMS.numServices;
  for(nI = 0; nI < REPORTPARMS.numServices; nI++)
  {
    if(StatusBarAbort())
    {
      bKeepGoing = FALSE;
      goto deallocate;
    }
    SERVICESKey0.recordID = REPORTPARMS.pServiceList[nI];
    rcode2 = btrieve(B_GETEQUAL, TMS_SERVICES, &SERVICES, &SERVICESKey0, 0);
    if(rcode2 != 0)
      continue;
    strncpy(serviceName, SERVICES.name, SERVICES_NAME_LENGTH);
    trim(serviceName, SERVICES_NAME_LENGTH);
//
//  Loop through the routes
//
    for(nJ = 0; nJ < REPORTPARMS.numRoutes; nJ++)
    {
      ROUTESKey0.recordID = REPORTPARMS.pRouteList[nJ];
      rcode2 = btrieve(B_GETEQUAL, TMS_ROUTES, &ROUTES, &ROUTESKey0, 0);
      if(rcode2 != 0)
        continue;
      strncpy(routeNumberAndName, ROUTES.number, ROUTES_NUMBER_LENGTH);
      trim(routeNumberAndName, ROUTES_NUMBER_LENGTH);
      strcat(routeNumberAndName, " - ");
      strncpy(tempString, ROUTES.name, ROUTES_NAME_LENGTH);
      trim(tempString, ROUTES_NAME_LENGTH);
      strcat(routeNumberAndName, tempString);
      LoadString(hInst, TEXT_118, szarString, sizeof(szarString));
      sprintf(tempString, szarString, routeNumberAndName, serviceName);
      StatusBarText(tempString);
      StatusBar((long)(nI * REPORTPARMS.numRoutes + nJ), (long)statbarTotal);
      if(StatusBarAbort())
      {
        bKeepGoing = FALSE;
        goto deallocate;
      }
//
//  Loop through the (possibly) two directions to generate the trips
//
//  CheckTrip fills tripData: 0 - First node record ID
//                            1 - Time at first node
//                            2 - Last node record ID
//                            3 - Time at last node
//
      for(nK = 0; nK < 2; nK++)
      {
        if(StatusBarAbort())
        {
          bKeepGoing = FALSE;
          goto deallocate;
        }
        if(ROUTES.DIRECTIONSrecordID[nK] == NO_RECORD)
          continue;
        DIRECTIONSKey0.recordID = ROUTES.DIRECTIONSrecordID[nK];
        btrieve(B_GETEQUAL, TMS_DIRECTIONS, &DIRECTIONS, &DIRECTIONSKey0, 0);
        strncpy(directionName, DIRECTIONS.longName, DIRECTIONS_LONGNAME_LENGTH);
        trim(directionName, DIRECTIONS_LONGNAME_LENGTH);
        sprintf(tempString, "\r\n%s, %s, direction %s\r\n",
              routeNumberAndName, serviceName, directionName);
        _lwrite(hfOutputFile, tempString, strlen(tempString));
        TRIPSKey1.ROUTESrecordID = REPORTPARMS.pRouteList[nJ];
        TRIPSKey1.SERVICESrecordID = REPORTPARMS.pServiceList[nI];
        TRIPSKey1.directionIndex = nK;
        TRIPSKey1.tripSequence = NO_RECORD;
        rcode2 = btrieve(B_GETGREATEROREQUAL, TMS_TRIPS, &TRIPS, &TRIPSKey1, 1);
        while(rcode2 == 0 &&
              TRIPS.ROUTESrecordID == REPORTPARMS.pRouteList[nJ] &&
              TRIPS.SERVICESrecordID == REPORTPARMS.pServiceList[nI] &&
              TRIPS.directionIndex == nK)
        {
          if(StatusBarAbort())
          {
            bKeepGoing = FALSE;
            goto deallocate;
          }
          CheckTrip(TRIPS.ROUTESrecordID, TRIPS.SERVICESrecordID,
                TRIPS.directionIndex, TRIPS.PATTERNNAMESrecordID,
                TRIPS.timeAtMLP, &GTResults);
          rcode2 = btrieve(B_GETNEXT, TMS_TRIPS, &TRIPS, &TRIPSKey1, 1);
        }
      }  // nK
    }  // nJ
  }  // nI
  bKeepGoing = TRUE;
//
//  Free allocated memory
//
  deallocate:
    TMSHeapFree(REPORTPARMS.pRouteList);
    TMSHeapFree(REPORTPARMS.pServiceList);
    _lclose(hfOutputFile);
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

/**********/

int CheckTrip(long ROUTESrecordID, long SERVICESrecordID, int directionIndex,
      long PATTERNNAMESrecordID, long timeAtMLP, GenerateTripDef *pGTResults)
{
   GetConnectionTimeDef GCTData;
   long patternNODESrecordIDs[MAXTRIPTIMES];
   long  numNodes;
   int   MLPIndex = 0;
   long  currentTime;
   long  addTime;
   long  maxNodes = 100;
   int   fromNode;
   int   toNode;
   int   firstNode;
   int   lastNode;
   int   rcode2;
   BOOL  gotMLP;

//
//  Get the pattern nodes and establish the MLP
//
   PATTERNSKey2.ROUTESrecordID = ROUTESrecordID;
   PATTERNSKey2.SERVICESrecordID = SERVICESrecordID;
   PATTERNSKey2.directionIndex = directionIndex;
   PATTERNSKey2.PATTERNNAMESrecordID = PATTERNNAMESrecordID;
   PATTERNSKey2.nodeSequence = 0;
   rcode2 = btrieve(B_GETGREATEROREQUAL, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
   numNodes = 0;
   gotMLP = FALSE;
   while(rcode2 == 0 &&
         PATTERNS.ROUTESrecordID == ROUTESrecordID &&
         PATTERNS.SERVICESrecordID == SERVICESrecordID &&
         PATTERNS.directionIndex == directionIndex &&
         PATTERNS.PATTERNNAMESrecordID == PATTERNNAMESrecordID)
   {
     if(!(PATTERNS.flags & PATTERNS_FLAG_BUSSTOP))
     {
       patternNODESrecordIDs[numNodes] = PATTERNS.NODESrecordID;
       if((PATTERNS.flags & PATTERNS_FLAG_MLP) && !gotMLP)
       {
         MLPIndex = numNodes;
         gotMLP = TRUE;
       }
       numNodes++;
       if(numNodes > MAXTRIPTIMES)
       {
         TMSError(NULL, MB_ICONSTOP, ERROR_282, (HANDLE)NULL);
         return(NO_RECORD);
       }
     }
     rcode2 = btrieve(B_GETNEXT, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
   }
//
//  Set up the guts of the GT structure
//
   GCTData.fromROUTESrecordID = ROUTESrecordID;
   GCTData.fromSERVICESrecordID = SERVICESrecordID;
   GCTData.fromPATTERNNAMESrecordID = PATTERNNAMESrecordID;
   GCTData.toROUTESrecordID = ROUTESrecordID;
   GCTData.toSERVICESrecordID = SERVICESrecordID;
   GCTData.toPATTERNNAMESrecordID = PATTERNNAMESrecordID;
//
//  Generate the trip
//
   currentTime = timeAtMLP;
   pGTResults->tripTimes[MLPIndex] = currentTime;
   fromNode = MLPIndex;
   lastNode = fromNode;
   toNode = MLPIndex + 1;
   while(toNode < numNodes)
   {
     GCTData.fromNODESrecordID = patternNODESrecordIDs[fromNode];
     GCTData.toNODESrecordID = patternNODESrecordIDs[toNode];
     GCTData.timeOfDay = currentTime;
     if((addTime = CheckRuntime(&GCTData)) == NO_TIME)
       pGTResults->tripTimes[toNode] = NO_TIME;
     else
       pGTResults->tripTimes[toNode] = currentTime + addTime;
     currentTime = pGTResults->tripTimes[toNode];
     fromNode = toNode;
     lastNode = toNode;
     toNode++;
   }
//
//  Loop through trips from MLPIndex back to first node
//
   currentTime = timeAtMLP;
   fromNode = MLPIndex - 1;
   toNode = MLPIndex;
   firstNode = MLPIndex;
   while(fromNode >= 0)
   {
     GCTData.fromNODESrecordID = patternNODESrecordIDs[fromNode];
     GCTData.toNODESrecordID = patternNODESrecordIDs[toNode];
     GCTData.timeOfDay = currentTime;
     if((addTime = (long)CheckRuntime(&GCTData)) == NO_TIME)
       pGTResults->tripTimes[toNode] = NO_TIME;
     else
       pGTResults->tripTimes[fromNode] = currentTime - addTime;
     currentTime = pGTResults->tripTimes[fromNode];
     toNode = fromNode;
     firstNode = fromNode;
     fromNode--;
   }
//
//  All done
//
   pGTResults->firstNODESrecordID = patternNODESrecordIDs[firstNode];
   pGTResults->firstNodeTime = pGTResults->tripTimes[firstNode];
   pGTResults->lastNODESrecordID = patternNODESrecordIDs[lastNode];
   pGTResults->lastNodeTime = pGTResults->tripTimes[lastNode];

   return(numNodes);
}

long CheckRuntime(GetConnectionTimeDef *pGCTData)
{
   int   nI;
   int   rcode2;
   long  fromNode;
   long  toNode;
   long  time;
   BOOL  found = FALSE;
   int   numEntriesToCheck = 0;
   int   startPosition = NO_RECORD;

//
//  Are we establishing runtimes?
//
   if(m_bEstablishRUNTIMES)
   {
     m_numRUNTIMES = 0;
     m_bEstablishRUNTIMES = FALSE;
   }
//
//  No?  Look for the first entry in the table, and then the last
//
   else
   {
     for(nI = 0; nI < m_numRUNTIMES; nI++)
     {
       if((m_pRUNTIMES[nI].fromNODESrecordID == pGCTData->fromNODESrecordID &&
             m_pRUNTIMES[nI].toNODESrecordID == pGCTData->toNODESrecordID) ||
          (m_pRUNTIMES[nI].fromNODESrecordID == pGCTData->toNODESrecordID &&
             m_pRUNTIMES[nI].toNODESrecordID == pGCTData->fromNODESrecordID))
       {
         if(!found)
         {
           found = TRUE;
           startPosition = nI;
         }
       }
       else
       {
         if(found)
         {
           numEntriesToCheck = nI - startPosition;
           break;
         }
       }
     }
     if(found && numEntriesToCheck == 0)
       numEntriesToCheck = m_numRUNTIMES - startPosition;
   }
//
//  We're establishing the list or the connection wasn't there...
//  Read in all of the potential connections - From -> To and To -> From
//
   if(!found)
   {
     startPosition = m_numRUNTIMES;
     for(nI = 0; nI < 2; nI++)
     {
       if(nI == 0)
       {
         fromNode = pGCTData->fromNODESrecordID;
         toNode = pGCTData->toNODESrecordID;
       }
       else
       {
         if(pGCTData->toNODESrecordID == pGCTData->fromNODESrecordID)
           break;
         fromNode = pGCTData->toNODESrecordID;
         toNode = pGCTData->fromNODESrecordID;
       }
       CONNECTIONSKey1.fromNODESrecordID = fromNode;
       CONNECTIONSKey1.toNODESrecordID = toNode;
       CONNECTIONSKey1.fromTimeOfDay = NO_TIME;
       rcode2 = btrieve(B_GETGREATEROREQUAL, TMS_CONNECTIONS, &CONNECTIONS, &CONNECTIONSKey1, 1);
       while(rcode2 == 0 &&
             CONNECTIONS.fromNODESrecordID == fromNode &&
             CONNECTIONS.toNODESrecordID == toNode)
       {
         memcpy(&m_pRUNTIMES[m_numRUNTIMES++], &CONNECTIONS, sizeof(CONNECTIONS));
         numEntriesToCheck++;
         rcode2 = btrieve(B_GETNEXT, TMS_CONNECTIONS, &CONNECTIONS, &CONNECTIONSKey1, 1);
       }
     }
   }
//
//  Leave if there are no connections
//
   if(numEntriesToCheck == 0)
     return(pGCTData->fromNODESrecordID == pGCTData->toNODESrecordID ? 0L : NO_TIME);
   else
   {
     time = CheckRuntime1(startPosition, numEntriesToCheck, pGCTData);
     return(time == NO_TIME && pGCTData->fromNODESrecordID == pGCTData->toNODESrecordID ? 0L : time);
   }
}
//
// CheckRuntime1 - Get the appropriate running time
//
// Connection time examination matrix
//
// *  Have read all FROM-TO records into core.
//
// Establish FNode - Looping through incore records
// ~~~~~~~~~~~~~~~
// a) -> FNod didn't match.  If 2way, flip input and try b)
// b) -> FNod matched
//
// Once a test has succeeded, continue through the rest of the incore
// records, keeping track of those that also pass.
//
// Weight     4    8   16    2    Score
//      FNod FRte FSer FPat FTTi
//  1.   X    X    X    X    X     30
//  2.   X    X    X    X          28
//  3.   X    X    X         X     14
//  4.   X    X    X               12
//  5.   X         X         X     10
//  6.   X         X                8
//  7.   X    X              X      6
//  8.   X    X                     4
//  9.   X                   X      2
// 10.   X                          0
//
// On those records that passed the test, perform the 1-8 and 9-16
// traversal again, but compare TNod TRte TSer TPat FTTi instead.
//

typedef struct CHECKRUNTIMESStruct
{
  int  score;
  long CONNECTIONSrecordID;
} CHECKRUNTIMESDef;

long CheckRuntime1(int startPosition, int numEntriesToCheck, GetConnectionTimeDef *pGCTData)
{
  CHECKRUNTIMESDef CHECKRUNTIMES[MAXCONNECTIONS];
  GetConnectionTimeDef originalGCT;
  int   nI;
  long  tempArray[4];
  int   highest;
  BOOL  bTestTOD;
  char  fromNode[NODES_ABBRNAME_LENGTH + 1];
  char  toNode[NODES_ABBRNAME_LENGTH + 1];

  memcpy(&originalGCT, pGCTData, sizeof(GetConnectionTimeDef));
//
//  Check out the from side first
//
  bTestTOD = pGCTData->timeOfDay >= 86400L;
  if(bTestTOD)
    pGCTData->timeOfDay -= 86400L;
  for(nI = startPosition; nI < startPosition + numEntriesToCheck; nI++)
  {
    CHECKRUNTIMES[nI].score = NO_RECORD;
    CHECKRUNTIMES[nI].CONNECTIONSrecordID = m_pRUNTIMES[nI].recordID;
    if(!(m_pRUNTIMES[nI].flags & CONNECTIONS_FLAG_RUNNINGTIME))
    {
      continue;
    }
    memcpy(pGCTData, &originalGCT, sizeof(GetConnectionTimeDef));
    if(bTestTOD)
    {
      pGCTData->timeOfDay -= 86400L;
    }
//
//  If the from nodes don't match, see if we can flip them
//
    if(pGCTData->fromNODESrecordID != m_pRUNTIMES[nI].fromNODESrecordID)
    {
      if(!(m_pRUNTIMES[nI].flags & CONNECTIONS_FLAG_TWOWAY))
      {
        continue;
      }
      memcpy(tempArray, &pGCTData->fromNODESrecordID, sizeof(tempArray));
      memcpy(&pGCTData->fromNODESrecordID, &pGCTData->toNODESrecordID, sizeof(tempArray));
      memcpy(&pGCTData->toNODESrecordID, tempArray, sizeof(tempArray));
    }
//
//  If we're dealing with specifics and they don't match, just cycle through
//
    if(m_pRUNTIMES[nI].fromROUTESrecordID != pGCTData->fromROUTESrecordID &&
          m_pRUNTIMES[nI].fromROUTESrecordID != NO_RECORD)
    {
      continue;
    }
    if(m_pRUNTIMES[nI].fromSERVICESrecordID != pGCTData->fromSERVICESrecordID &&
          m_pRUNTIMES[nI].fromSERVICESrecordID != NO_RECORD)
    {
      continue;
    }
    if(m_pRUNTIMES[nI].fromPATTERNNAMESrecordID != pGCTData->fromPATTERNNAMESrecordID &&
          m_pRUNTIMES[nI].fromPATTERNNAMESrecordID != NO_RECORD)
    {
      continue;
    }
    if(m_pRUNTIMES[nI].fromTimeOfDay != NO_TIME && m_pRUNTIMES[nI].toTimeOfDay != NO_TIME &&
          (pGCTData->timeOfDay < m_pRUNTIMES[nI].fromTimeOfDay || pGCTData->timeOfDay > m_pRUNTIMES[nI].toTimeOfDay))
    {
      continue;
    }
//
//  Build a score for this component
//
    CHECKRUNTIMES[nI].score = 0;
    CHECKRUNTIMES[nI].score +=
          (pGCTData->fromPATTERNNAMESrecordID == m_pRUNTIMES[nI].fromPATTERNNAMESrecordID &&
          pGCTData->fromROUTESrecordID == m_pRUNTIMES[nI].fromROUTESrecordID &&
          pGCTData->fromSERVICESrecordID == m_pRUNTIMES[nI].fromSERVICESrecordID ? 16 : 0);
    CHECKRUNTIMES[nI].score +=
          (pGCTData->fromSERVICESrecordID == m_pRUNTIMES[nI].fromSERVICESrecordID ? 8 : 0);
    CHECKRUNTIMES[nI].score +=
          (pGCTData->fromROUTESrecordID == m_pRUNTIMES[nI].fromROUTESrecordID ? 4 : 0);
    CHECKRUNTIMES[nI].score +=
          (m_pRUNTIMES[nI].fromTimeOfDay != NO_TIME && m_pRUNTIMES[nI].toTimeOfDay != NO_TIME &&
          pGCTData->timeOfDay >= m_pRUNTIMES[nI].fromTimeOfDay &&
          pGCTData->timeOfDay <= m_pRUNTIMES[nI].toTimeOfDay ? 2 : 0);
    CHECKRUNTIMES[nI].score += (CHECKRUNTIMES[nI].score != 0 ? 1 : 0);
  }
//
//  Now do the to side
//
  highest = startPosition;
  for(nI = startPosition; nI < startPosition + numEntriesToCheck; nI++)
  {
//
//  If this connection failed a test earlier, don't bother with it now
//
    if(CHECKRUNTIMES[nI].score == NO_RECORD)
      continue;
    memcpy(pGCTData, &originalGCT, sizeof(GetConnectionTimeDef));
    if(bTestTOD)
      pGCTData->timeOfDay -= 86400L;
//
//  If the to nodes don't match, see if we can flip them
//
    if(pGCTData->toNODESrecordID != m_pRUNTIMES[nI].toNODESrecordID)
    {
      if(!(m_pRUNTIMES[nI].flags & CONNECTIONS_FLAG_TWOWAY))
      {
        continue;
      }
      memcpy(tempArray, &pGCTData->fromNODESrecordID, sizeof(tempArray));
      memcpy(&pGCTData->fromNODESrecordID, &pGCTData->toNODESrecordID, sizeof(tempArray));
      memcpy(&pGCTData->toNODESrecordID, tempArray, sizeof(tempArray));
    }
//
//  If we're dealing with specifics and they don't match, just cycle through
//
    if(m_pRUNTIMES[nI].toROUTESrecordID != pGCTData->toROUTESrecordID &&
          m_pRUNTIMES[nI].toROUTESrecordID != NO_RECORD)
    {
      continue;
    }
    if(m_pRUNTIMES[nI].toSERVICESrecordID != pGCTData->toSERVICESrecordID &&
          m_pRUNTIMES[nI].toSERVICESrecordID != NO_RECORD)
    {
      continue;
    }
    if(m_pRUNTIMES[nI].toPATTERNNAMESrecordID != pGCTData->toPATTERNNAMESrecordID &&
          m_pRUNTIMES[nI].toPATTERNNAMESrecordID != NO_RECORD)
    {
      continue;
    }
    if(m_pRUNTIMES[nI].fromTimeOfDay != NO_TIME && m_pRUNTIMES[nI].toTimeOfDay != NO_TIME &&
          (pGCTData->timeOfDay < m_pRUNTIMES[nI].fromTimeOfDay || pGCTData->timeOfDay > m_pRUNTIMES[nI].toTimeOfDay))
    {
      continue;
    }
//
//  Build a score for this component
//
    CHECKRUNTIMES[nI].score +=
         (pGCTData->toPATTERNNAMESrecordID == m_pRUNTIMES[nI].toPATTERNNAMESrecordID &&
          pGCTData->toSERVICESrecordID == m_pRUNTIMES[nI].toSERVICESrecordID &&
          pGCTData->toROUTESrecordID == m_pRUNTIMES[nI].toROUTESrecordID ? 16 : 0);
    CHECKRUNTIMES[nI].score += (pGCTData->toSERVICESrecordID == m_pRUNTIMES[nI].toSERVICESrecordID ? 8 : 0);
    CHECKRUNTIMES[nI].score += (pGCTData->toROUTESrecordID == m_pRUNTIMES[nI].toROUTESrecordID ? 4 : 0);
    if(CHECKRUNTIMES[nI].score > CHECKRUNTIMES[highest].score)
    {
      highest = nI;
    }
  }
//
//  Check the score of the highest to see if it failed a test earlier.
//  If it did, start at the beginning to find one that didn't.
//  If none paseed, send back NO_TIME.
//
  if(CHECKRUNTIMES[highest].score == NO_RECORD)
  {
    highest = NO_RECORD;
    for(nI = startPosition; nI < startPosition + numEntriesToCheck; nI++)
    {
      if(CHECKRUNTIMES[nI].score != NO_RECORD)
        if(highest == NO_RECORD || CHECKRUNTIMES[nI].score > CHECKRUNTIMES[highest].score)
          highest = nI;
    }
    if(highest == NO_RECORD)
    {
      NODESKey0.recordID = pGCTData->fromNODESrecordID;
      btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
      strncpy(fromNode, NODES.abbrName, NODES_ABBRNAME_LENGTH);
      trim(fromNode, NODES_ABBRNAME_LENGTH);
      NODESKey0.recordID = pGCTData->toNODESrecordID;
      btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
      strncpy(toNode, NODES.abbrName, NODES_ABBRNAME_LENGTH);
      trim(toNode, NODES_ABBRNAME_LENGTH);
      sprintf(tempString, "  Missing connection from %s to %s at %s\r\n",
            fromNode, toNode, Tchar(pGCTData->timeOfDay));
      _lwrite(hfOutputFile, tempString, strlen(tempString));
      return(NO_TIME);
    }
  }
//
//  Check for a duplicate of the highest amongst CHECKRUNTIMES[].scores
//
  strcpy(fromNode, "");
  for(nI = startPosition; nI < startPosition + numEntriesToCheck; nI++)
  {
    if(nI == highest)
      continue;
    if(CHECKRUNTIMES[highest].score == CHECKRUNTIMES[nI].score)
    {
      if(strcmp(fromNode, "") == 0)
      {
        NODESKey0.recordID = pGCTData->fromNODESrecordID;
        btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
        strncpy(fromNode, NODES.abbrName, NODES_ABBRNAME_LENGTH);
        trim(fromNode, NODES_ABBRNAME_LENGTH);
        NODESKey0.recordID = pGCTData->toNODESrecordID;
        btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
        strncpy(toNode, NODES.abbrName, NODES_ABBRNAME_LENGTH);
        trim(toNode, NODES_ABBRNAME_LENGTH);
        sprintf(tempString, "  Overlapping connection from %s to %s at %s\r\n",
              fromNode, toNode, Tchar(pGCTData->timeOfDay));
        _lwrite(hfOutputFile, tempString, strlen(tempString));
      }
      sprintf(tempString, "   RecordIDs are %ld and %ld\r\n",
            m_pRUNTIMES[highest].recordID, m_pRUNTIMES[nI].recordID);
      _lwrite(hfOutputFile, tempString, strlen(tempString));
    }
  }

  return(m_pRUNTIMES[highest].connectionTime);
}



//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2008 Schedule Masters, Inc.
//  All rights reserved.
//
//
//  TMSRPT77() - Jurisdictional Time and Distance
//
//  From QETXT.INI:
//  
//  [TMSRPT77]
//  FILE=Tmsrpt77.txt
//  FLN=0
//  TT=Tab
//  Charset=ANSI
//  DS=.
//  FIELD1=Jurisdiction,VARCHAR,32,0,32,0,
//  FIELD2=Route,VARCHAR,75,0,75,0,
//  FIELD3=Service,VARCHAR,32,0,32,0,
//  FIELD4=Direction,VARCHAR,32,0,32,0,
//  FIELD5=TripTime,NUMERIC,7,0,7,0,
//  FIELD6=Distance,NUMERIC,7,2,7,0,
//  
#include "TMSHeader.h"

#define TMSRPT77_MAXPATTERNNODES 200

typedef struct JurisdictionDataStruct
{
  long  recordID;
  char  szName[JURISDICTIONS_NAME_LENGTH + 1];
  long  time;
  float distance;
} JurisdictionDataDef;

int GetJurisdictionIndex(long recordID, JurisdictionDataDef *pJurData, int numInJur)
{
  int nI;

  for(nI = 0; nI < numInJur; nI++)
  {
    if(pJurData[nI].recordID == recordID)
    {
      return(nI);
    }
  }

  return(NO_RECORD);
}

BOOL FAR TMSRPT77(TMSRPTPassedDataDef *pPassedData)
{
  JurisdictionDataDef *pJurData;
  GenerateTripDef GTResults;
  REPORTPARMSDef REPORTPARMS;
  NODESDef prevNODES;
  HFILE hfOutputFile;
  float prevLon;
  float prevLat;
  float distanceToHere;
  float tripDistances[TMSRPT77_MAXPATTERNNODES];
  float distance;
  BOOL  bKeepGoing = FALSE;
  BOOL  bRC;
  BOOL  bFirstNode;
  BOOL  bFirst;
  char  szRouteNumber[ROUTES_NUMBER_LENGTH + 3 + ROUTES_NAME_LENGTH + 1];
  char  szServiceName[SERVICES_NAME_LENGTH + 1];
  char  szDirection[DIRECTIONS_LONGNAME_LENGTH + 1];
  char  outputString[512];
  char  outputStringSave[512];
  char  dummy[256];
  long  statbarTotal;
  long  prevJur;
  long  BASEPatternNODESrecordIDs[TMSRPT77_MAXPATTERNNODES];
  long  timeToHere;
  long  timeToStop;
  long  timeAtStop;
  int   nI;
  int   nJ;
  int   nK;
  int   nL;
  int   rcode2;
  int   numJur;
  int   indexToJur;
  int   prevIndexToJur;
  int   numBASEPatternNodes;
  int   tripIndex;

  pJurData = NULL;
//
//  Ensure everything's geocoded and got a jurisdiction
//
  rcode2 = btrieve(B_GETFIRST, TMS_NODES, &NODES, &NODESKey0, 0);
  while(rcode2 == 0)
  {
	  if(NODES.longitude == 0.0 || NODES.latitude == 0.0)
	  {
      strncpy(szarString, NODES.abbrName, NODES_ABBRNAME_LENGTH);
      trim(szarString, NODES_ABBRNAME_LENGTH);
      sprintf(tempString, "Node/stop \"%s\" is not geocoded\n\nThis report needs all nodes and stops to be geocoded", szarString);
      MessageBeep(MB_ICONSTOP);
      MessageBox(NULL, tempString, TMS, MB_ICONSTOP | MB_OK);
      goto deallocate;
   	}
    if(NODES.JURISDICTIONSrecordID == NO_RECORD)
    {
      strncpy(szarString, NODES.abbrName, NODES_ABBRNAME_LENGTH);
      trim(szarString, NODES_ABBRNAME_LENGTH);
      sprintf(tempString, "Node/stop \"%s\" doesn't have a jurisdiction\n\nThis report needs all nodes and stops to have jurisdictions assigned", szarString);
      MessageBeep(MB_ICONSTOP);
      MessageBox(NULL, tempString, TMS, MB_ICONSTOP | MB_OK);
      goto deallocate;
    }
    rcode2 = btrieve(B_GETNEXT, TMS_NODES, &NODES, &NODESKey0, 0);
  }
//
//  See what he wants
//
  pPassedData->nReportNumber = 76;
  pPassedData->numDataFiles = 1;
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
//  Open the output file
//
  strcpy(tempString, szReportsTempFolder);
  strcat(tempString, "\\tmsrpt77.txt");
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
//  Allocate space for the Jurisdictions structure
//
  rcode2 = btrieve(B_STAT, TMS_JURISDICTIONS, &BSTAT, dummy, 0);
  if(rcode2 != 0 || BSTAT.numRecords == 0)
  {
    LoadString(hInst, ERROR_364, szFormatString, sizeof(szFormatString));
    sprintf(szarString, szFormatString, tempString);
    MessageBeep(MB_ICONSTOP);
    MessageBox((HWND)NULL, szarString, TMS, MB_ICONSTOP);
    goto deallocate;
  }
  numJur = BSTAT.numRecords;
  pJurData = (JurisdictionDataDef *)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(JurisdictionDataDef) * numJur); 
  if(pJurData == NULL)
  {
    AllocationError(__FILE__, __LINE__, FALSE);
    goto deallocate;
  }
//
//  Get the jurisdictions
//
  for(nI = 0; nI < numJur; nI++)
  {
    rcode2 = btrieve((nI == 0 ? B_GETFIRST : B_GETNEXT), TMS_JURISDICTIONS, &JURISDICTIONS, &JURISDICTIONSKey1, 1);
    pJurData[nI].recordID = JURISDICTIONS.recordID;
    strncpy(tempString, JURISDICTIONS.name, JURISDICTIONS_NAME_LENGTH);
    trim(tempString, JURISDICTIONS_NAME_LENGTH);
    strcpy(pJurData[nI].szName, tempString);
  }
//
//  Loop through the routes
//
  LoadString(hInst, TEXT_117, tempString, TEMPSTRING_LENGTH);
  StatusBarStart(hWndMain, tempString);
  statbarTotal = REPORTPARMS.numRoutes * REPORTPARMS.numServices;
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
    strncpy(szRouteNumber, ROUTES.number, ROUTES_NUMBER_LENGTH);
    trim(szRouteNumber, ROUTES_NUMBER_LENGTH);
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
      strncpy(szServiceName, SERVICES.name, SERVICES_NAME_LENGTH);
      trim(szServiceName, SERVICES_NAME_LENGTH);
      LoadString(hInst, TEXT_118, szarString, sizeof(szarString));
      sprintf(tempString, szarString, szRouteNumber, szServiceName);
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
//
//  Zero out the variable portions of the data structure
//
        for(nL = 0; nL < numJur; nL++)
        {
          pJurData[nL].time = 0;
          pJurData[nL].distance = 0.0;
        }
//
//  Get the base pattern nodes
//
        numBASEPatternNodes = 0;
        PATTERNSKey2.ROUTESrecordID = TRIPS.ROUTESrecordID;
        PATTERNSKey2.SERVICESrecordID = TRIPS.SERVICESrecordID;
        PATTERNSKey2.PATTERNNAMESrecordID = basePatternRecordID;
        PATTERNSKey2.directionIndex = TRIPS.directionIndex;
        PATTERNSKey2.nodeSequence = -1;
        rcode2 = btrieve(B_GETGREATER, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
        bFirstNode = TRUE;
        while(rcode2 == 0 &&
              PATTERNS.ROUTESrecordID == TRIPS.ROUTESrecordID &&
              PATTERNS.SERVICESrecordID == TRIPS.SERVICESrecordID &&
              PATTERNS.PATTERNNAMESrecordID == basePatternRecordID &&
              PATTERNS.directionIndex == TRIPS.directionIndex)
        {
          if(!(PATTERNS.flags & PATTERNS_FLAG_BUSSTOP))
          {
            BASEPatternNODESrecordIDs[numBASEPatternNodes] = PATTERNS.NODESrecordID;
            numBASEPatternNodes++;
          }
          rcode2 = btrieve(B_GETNEXT, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
        }
//
//
//  Get the trips
//
        m_bEstablishRUNTIMES = TRUE;
        TRIPSKey1.ROUTESrecordID = REPORTPARMS.pRouteList[nI];
        TRIPSKey1.SERVICESrecordID = REPORTPARMS.pServiceList[nJ];
        TRIPSKey1.directionIndex = nK;
        TRIPSKey1.tripSequence = NO_TIME;
        rcode2 = btrieve(B_GETGREATEROREQUAL, TMS_TRIPS, &TRIPS, &TRIPSKey1, 1);
        while(rcode2 == 0 &&
              TRIPS.ROUTESrecordID == REPORTPARMS.pRouteList[nI] &&
              TRIPS.SERVICESrecordID == REPORTPARMS.pServiceList[nJ] &&
              TRIPS.directionIndex == nK)
        {
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
          PATTERNSKey2.ROUTESrecordID = REPORTPARMS.pRouteList[nI];
          PATTERNSKey2.SERVICESrecordID = REPORTPARMS.pServiceList[nJ];
          PATTERNSKey2.directionIndex = nK;
          PATTERNSKey2.PATTERNNAMESrecordID = TRIPS.PATTERNNAMESrecordID;
          PATTERNSKey2.nodeSequence = NO_RECORD;
          rcode2 = btrieve(B_GETGREATER, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
          tripIndex = 0;
          distanceToHere = 0.0;
          while(rcode2 == 0 &&
                PATTERNS.ROUTESrecordID == REPORTPARMS.pRouteList[nI] &&
                PATTERNS.SERVICESrecordID == REPORTPARMS.pServiceList[nJ] &&
                PATTERNS.directionIndex == nK &&
                PATTERNS.PATTERNNAMESrecordID == TRIPS.PATTERNNAMESrecordID)
          {
            if(!(PATTERNS.flags & PATTERNS_FLAG_BUSSTOP))
            {
              NODESKey0.recordID = PATTERNS.NODESrecordID;
              rcode2 = btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
              if(tripIndex != 0)
              {
                CONNECTIONSKey1.fromNODESrecordID = prevNODES.recordID;
                CONNECTIONSKey1.toNODESrecordID = NODES.recordID;
                CONNECTIONSKey1.fromTimeOfDay = NO_TIME;
                rcode2 = btrieve(B_GETGREATEROREQUAL, TMS_CONNECTIONS, &CONNECTIONS, &CONNECTIONSKey1, 1);
                if(rcode2 != 0 || CONNECTIONS.distance <= 0.0)
                {
                  CONNECTIONSKey1.fromNODESrecordID = NODES.recordID;
                  CONNECTIONSKey1.toNODESrecordID = prevNODES.recordID;
                  CONNECTIONSKey1.fromTimeOfDay = NO_TIME;
                  rcode2 = btrieve(B_GETGREATEROREQUAL, TMS_CONNECTIONS, &CONNECTIONS, &CONNECTIONSKey1, 1);
                }
                if(rcode2 == 0 && CONNECTIONS.distance > 0.0)
                {
                  distanceToHere = CONNECTIONS.distance;
                }
                else
                {
                  distanceToHere = (float)GreatCircleDistance(prevNODES.longitude, prevNODES.latitude, NODES.longitude, NODES.latitude);
                }
              }
              tripDistances[tripIndex] = distanceToHere;
              tripIndex++;
              prevNODES = NODES;
            }
            rcode2 = btrieve(B_GETNEXT, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
          }
//
//  Pass 2 - Determine time interpolations and intervening distances
//
          PATTERNSKey2.ROUTESrecordID = REPORTPARMS.pRouteList[nI];
          PATTERNSKey2.SERVICESrecordID = REPORTPARMS.pServiceList[nJ];
          PATTERNSKey2.directionIndex = nK;
          PATTERNSKey2.PATTERNNAMESrecordID = TRIPS.PATTERNNAMESrecordID;
          PATTERNSKey2.nodeSequence = NO_RECORD;
          rcode2 = btrieve(B_GETGREATER, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
          prevIndexToJur = NO_RECORD;
          bFirst = TRUE;
          while(rcode2 == 0 &&
                PATTERNS.ROUTESrecordID == REPORTPARMS.pRouteList[nI] &&
                PATTERNS.SERVICESrecordID == REPORTPARMS.pServiceList[nJ] &&
                PATTERNS.directionIndex == nK &&
                PATTERNS.PATTERNNAMESrecordID == TRIPS.PATTERNNAMESrecordID)
          {
            NODESKey0.recordID = PATTERNS.NODESrecordID;
            rcode2 = btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
            indexToJur = GetJurisdictionIndex(NODES.JURISDICTIONSrecordID, pJurData, numJur);
            if(bFirst)
            {
              timeToHere = 0;
              distanceToHere = 0.0;
              bFirst = FALSE;
            }
            else
            {
              if(indexToJur != prevIndexToJur)
              {
                pJurData[prevIndexToJur].distance += distanceToHere;
                pJurData[prevIndexToJur].time += timeToHere;
                timeToHere = 0;
                distanceToHere = 0.0;
              }
              else
              {
                CONNECTIONSKey1.fromNODESrecordID = prevNODES.recordID;
                CONNECTIONSKey1.toNODESrecordID = NODES.recordID;
                CONNECTIONSKey1.fromTimeOfDay = NO_TIME;
                rcode2 = btrieve(B_GETGREATEROREQUAL, TMS_CONNECTIONS, &CONNECTIONS, &CONNECTIONSKey1, 1);
                if(rcode2 != 0 || CONNECTIONS.distance <= 0.0)
                {
                  CONNECTIONSKey1.fromNODESrecordID = NODES.recordID;
                  CONNECTIONSKey1.toNODESrecordID = prevNODES.recordID;
                  CONNECTIONSKey1.fromTimeOfDay = NO_TIME;
                  rcode2 = btrieve(B_GETGREATEROREQUAL, TMS_CONNECTIONS, &CONNECTIONS, &CONNECTIONSKey1, 1);
                }
                if(rcode2 == 0 && CONNECTIONS.distance > 0.0)
                {
                  distanceToHere += CONNECTIONS.distance;
                }
                else
                {
                  distanceToHere += (float)GreatCircleDistance(prevNODES.longitude, prevNODES.latitude, NODES.longitude, NODES.latitude);
                }
                timeToHere = (long)((GTResults.lastNodeTime - GTResults.firstNodeTime) *
                      (distanceToHere / (tripDistances[tripIndex - 1] - tripDistances[0])));
                timeAtStop += (GTResults.tripTimes[tripIndex - 1] - GTResults.firstNodeTime);
              }
            }
            prevNODES = NODES;
            prevIndexToJur = indexToJur;
            rcode2 = btrieve(B_GETNEXT, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
          } // while on pattern
//
//  Go through the trip to summarize
//
          PATTERNSKey2.ROUTESrecordID = REPORTPARMS.pRouteList[nI];
          PATTERNSKey2.SERVICESrecordID = REPORTPARMS.pServiceList[nJ];
          PATTERNSKey2.directionIndex = nK;
          PATTERNSKey2.PATTERNNAMESrecordID = basePatternRecordID;
          PATTERNSKey2.nodeSequence = NO_RECORD;
          prevJur = NO_RECORD;
          rcode2 = btrieve(B_GETGREATER, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
          while(rcode2 == 0 &&
                PATTERNS.ROUTESrecordID == REPORTPARMS.pRouteList[nI] &&
                PATTERNS.SERVICESrecordID == REPORTPARMS.pServiceList[nJ] &&
                PATTERNS.directionIndex == nK &&
                PATTERNS.PATTERNNAMESrecordID == basePatternRecordID)
          {
            NODESKey0.recordID = PATTERNS.NODESrecordID;
            rcode2 = btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
            if(rcode2 == 0)
            {
//
//  New/undefined jurisdiction
//
              if(NODES.JURISDICTIONSrecordID == NO_RECORD || NODES.JURISDICTIONSrecordID != prevJur)
              {
                prevNODES = NODES;
              }
//
//  Is there an entry in the connections table for this?
//  (We're looking for node to node or stop to stop - either will return here)
//
              else
              {
                CONNECTIONSKey1.fromNODESrecordID = prevNODES.recordID;
                CONNECTIONSKey1.toNODESrecordID = NODES.recordID;
                CONNECTIONSKey1.fromTimeOfDay = NO_TIME;
                rcode2 = btrieve(B_GETGREATEROREQUAL, TMS_CONNECTIONS, &CONNECTIONS, &CONNECTIONSKey1, 1);
                if(rcode2 != 0 || CONNECTIONS.distance <= 0.0)
                {
                  CONNECTIONSKey1.fromNODESrecordID = NODES.recordID;
                  CONNECTIONSKey1.toNODESrecordID = prevNODES.recordID;
                  CONNECTIONSKey1.fromTimeOfDay = NO_TIME;
                  rcode2 = btrieve(B_GETGREATEROREQUAL, TMS_CONNECTIONS, &CONNECTIONS, &CONNECTIONSKey1, 1);
                }
                if(rcode2 == 0 && CONNECTIONS.distance > 0.0)
                {
                  distance = CONNECTIONS.distance;
                }
                else
                {
                  distance = (float)GreatCircleDistance(prevNODES.longitude, prevNODES.latitude, NODES.longitude, NODES.latitude);
                }
              }
            }
            rcode2 = btrieve(B_GETNEXT, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
          }
//
//  Set up the output string
//
          strcpy(outputStringSave, "");
//
//  Route number
//
          strncpy(tempString, ROUTES.number, ROUTES_NUMBER_LENGTH);
          trim(tempString, ROUTES_NUMBER_LENGTH);
          sprintf(szarString, "\"%s\"\t", tempString);
          strcat(outputStringSave, szarString);
//
//  Route name
//
          strncpy(tempString, ROUTES.name, ROUTES_NAME_LENGTH);
          trim(tempString, ROUTES_NAME_LENGTH);
          sprintf(szarString, "\"%s\"\t", tempString);
          strcat(outputStringSave, szarString);
//
//  Service Name
//
          strncpy(tempString, SERVICES.name, SERVICES_NAME_LENGTH);
          trim(tempString, SERVICES_NAME_LENGTH);
          sprintf(szarString, "\"%s\"\t", tempString);
          strcat(outputStringSave, szarString);
//
//  Direction
//
          strncpy(tempString, DIRECTIONS.longName, DIRECTIONS_LONGNAME_LENGTH);
          trim(tempString, DIRECTIONS_LONGNAME_LENGTH);
          sprintf(szarString, "\"%s\"\t", tempString);
          strcat(outputStringSave, szarString);
//
//  Cycle through the jurisdictions
//
          strcpy(outputStringSave, outputString);
          for(nL = 0; nL < numJur; nL++)
          {
            if(pJurData[nL].time == 0)
            {
              continue;
            }
            sprintf(outputString, "\"%s\"\t", pJurData[nL].szName);
            strcat(outputString, outputStringSave);
//
//  Time (in minutes)
//
            sprintf(szarString, "%ld\t", GTResults.lastNodeTime - GTResults.firstNodeTime);
            strcat(outputString, szarString);
//
//  Distance
//
            sprintf(szarString, "%7.2f\r\n", GTResults.tripDistance);
            strcat(outputString, szarString);
//
//  Write it out
//
            _lwrite(hfOutputFile, outputString, strlen(outputString));
          }
//
//  Get the next trip
//
          rcode2 = btrieve(B_GETNEXT, TMS_TRIPS, &TRIPS, &TRIPSKey1, 1);
        }  // while  (on trips)
      }  // nK  (directions)
    }  // nJ  (services)
  }  // nI  (routes)
  bKeepGoing = TRUE;
//
//  Free allocated memory
//
  deallocate:
    TMSHeapFree(REPORTPARMS.pRouteList);
    TMSHeapFree(REPORTPARMS.pServiceList);
    TMSHeapFree(pJurData);
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




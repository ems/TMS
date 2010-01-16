//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
//
//  TMSRPT39() - System Runs
//
//  From QETXT.INI
//  
//  [TMSRPT39]
//  FILE=TMSRPT39.TXT
//  FLN=0
//  TT=Tab
//  FIELD1=BlockNumber,NUMERIC,6,0,6,0,
//  FIELD2=BusType,VARCHAR,32,0,32,0,
//  FIELD3=RGRP,VARCHAR,8,0,8,0,
//  FIELD4=SGRP,VARCHAR,16,0,16,0,
//  FIELD5=Route,VARCHAR,8,0,8,0,
//  FIELD6=Service,VARCHAR,16,0,16,0,
//  FIELD7=POG,VARCHAR,4,0,4,0,
//  FIELD8=POT,VARCHAR,5,0,5,0,
//  FIELD9=FNode,VARCHAR,4,0,4,0,
//  FIELD10=FTime,VARCHAR,5,0,5,0,
//  FIELD11=TTime,VARCHAR,5,0,5,0,
//  FIELD12=TNode,VARCHAR,4,0,4,0,
//  FIELD13=PIG,VARCHAR,4,0,4,0,
//  FIELD14=PIT,VARCHAR,5,0,5,0,
//  FIELD15=TripTime,NUMERIC,4,0,4,0,
//  FIELD16=Layover,NUMERIC,4,0,4,0,
//  FIELD17=Deadhead,NUMERIC,4,0,4,0,
//  FIELD18=Distance,NUMERIC,8,2,8,0,
//  FIELD19=CumDist,NUMERIC,9,2,9,0,
//  FIELD20=GarageDhdTime,NUMERIC,4,0,4,9,
//  FIELD21=GarageDhdDist,NUMERIC,9,2,9,13,
//  FIELD22=DeadheadDist,NUMERIC,9,2,9,22,
//  FIELD23=Sequence,NUMERIC,5,0,5,31,
//  FIELD24=RunNumber,NUMERIC,6,0,6,0,
//  FIELD25=PieceNumber,NUMERIC,6,0,6,0,
//  FIELD26=TravelTime,NUMERIC,6,0,6,0,
//  FIELD27=ReportAtTime,VARCHAR,5,0,5,0,
//  FIELD28=ReportATLoc,VARCHAR,4,0,4,0,
//  FIELD29=SignCode,VARCHAR,8,0,8,0,
//  FIELD30=CommentCode,VARCHAR,8,0,8,0,
//  FIELD31=XBoardOn,VARCHAR,5,0,5,0,
//  FIELD32=XBoardOff,VARCHAR,5,0,5,0,
//  FIELD33=SpreadTime,VARCHAR,5,0,5,0,
//  FIELD34=PayTime,VARCHAR,5,0,5,0,
//
#include "TMSHeader.h"
#include "cistms.h"
#include <math.h>

BOOL FAR TMSRPT39(TMSRPTPassedDataDef *pPassedData)
{
  GetConnectionTimeDef GCTData;
  GenerateTripDef GTResults[2];  
  REPORTPARMSDef REPORTPARMS;
  PROPOSEDRUNDef PROPOSEDRUN;
  COSTDef COST;
  HANDLE hOutputFile;
  DWORD  dwBytesWritten;
  float distance;
  float cumDist;
  float PODistance, PIDistance;
  float deadheadDist;
  BOOL  bKeepGoing = FALSE;
  BOOL  bRC;
  BOOL  bGotServiceIndex[4];
  BOOL  bGotLastTrip;
  BOOL  bFirstTrip;
  char  serviceName[SERVICES_NAME_LENGTH + 1];
  char  divisionName[DIVISIONS_NAME_LENGTH + 1];
  char  outputString[TEMPSTRING_LENGTH];
  char  szNodeName[NODES_ABBRNAME_LENGTH + 1];
  long  equivalentTravelTime;
  long  layoverTime;
  long  deadheadTime;
  long  GarageDhdTime;
  long  POTime;
  long  PITime;
  long  runNumber;
  long  blockNumber;
  long  NODESrecordID;
  long  firstTime;
  long  lastTime;
  long  tempLong;
  long  COMMENTSrecordID;
  long  SIGNCODESrecordID;
  int   pos;
  int   numPieces;
  int   nI;
  int   nJ;
  int   rcode2;
  int   seqNum;
  int   serviceIndex;
  int   lastPos;

  pPassedData->nReportNumber = 38;
  pPassedData->numDataFiles = 1;
//
//  See what he wants
//
  REPORTPARMS.nReportNumber = pPassedData->nReportNumber;
  REPORTPARMS.DIVISIONSrecordID = m_DivisionRecordID;
  REPORTPARMS.SERVICESrecordID = m_ServiceRecordID;
  REPORTPARMS.pServiceList = NULL;
  REPORTPARMS.pDivisionList = NULL;
  REPORTPARMS.pRunList = NULL;
  REPORTPARMS.flags = RPFLAG_SERVICES | RPFLAG_DIVISIONS | RPFLAG_NOALLDIVISIONS | RPFLAG_RUNS;
  bRC = DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_RPTPARMS),
        hWndMain, (DLGPROC)RPTPARMSMsgProc, (LPARAM)&REPORTPARMS);
  if(!bRC)
  {
    return(FALSE);
  }

  if(REPORTPARMS.numServices == 0 || REPORTPARMS.numRuns == 0)
  {
    goto deallocate;
  }
//
//  Open the output file
//
  strcpy(tempString, szReportsTempFolder);
  strcat(tempString, "\\TMSRPT39.TXT");
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
  for(nI = 0; nI < 4; nI++)
  {
    bGotServiceIndex[nI] = FALSE;
  }
//
//  Get the division name
//
  DIVISIONSKey0.recordID = REPORTPARMS.pDivisionList[0];
  btrieve(B_GETEQUAL, TMS_DIVISIONS, &DIVISIONS, &DIVISIONSKey0, 0);
  strncpy(divisionName, DIVISIONS.name, DIVISIONS_NAME_LENGTH);
  trim(divisionName, DIVISIONS_NAME_LENGTH);
//
//  Start the status bar
//
  LoadString(hInst, TEXT_117, tempString, TEMPSTRING_LENGTH);
  StatusBarStart(hWndMain, tempString);
//
//  Loop through the runs
//
  bKeepGoing = TRUE;
  seqNum = 0;
  m_bEstablishRUNTIMES = TRUE;
  for(nI = 0; nI < REPORTPARMS.numRuns; nI++)
  {
//
//  Get the run
//
    RUNSKey0.recordID = REPORTPARMS.pRunList[nI];
    btrieve(B_GETEQUAL, TMS_RUNS, &RUNS, &RUNSKey0, 0);
//
//  Get the service name
//
    SERVICESKey0.recordID = RUNS.SERVICESrecordID;
    btrieve(B_GETEQUAL, TMS_SERVICES, &SERVICES, &SERVICESKey0, 0);
    strncpy(serviceName, SERVICES.name, SERVICES_NAME_LENGTH);
    trim(serviceName, SERVICES_NAME_LENGTH);
//
//  See if we need to load the Travel Matrix
//
    serviceIndex = SERVICES.number - 1;
    if(serviceIndex < 0 || serviceIndex > NODES_RELIEFLABELS_LENGTH - 1)
    {
      serviceIndex = 0;
    }
    if(!bGotServiceIndex[serviceIndex] && bUseDynamicTravels)
    {
      StatusBarEnd();
      bGotServiceIndex[serviceIndex] = TRUE;
      LoadString(hInst, bUseCISPlan ? TEXT_154 : TEXT_135, tempString, TEMPSTRING_LENGTH);
      StatusBarText(tempString);
      if(bUseCISPlan)
      {
        CISfree();
        if(!CISbuildService(FALSE, TRUE, SERVICES.recordID))
          goto deallocate;
      }
      else
      {
        InitTravelMatrix(SERVICES.recordID, FALSE);
      }
      LoadString(hInst, TEXT_117, tempString, TEMPSTRING_LENGTH);
      StatusBarStart(hWndMain, tempString);
    }
//
//  Cost the run
//
    runNumber = RUNS.runNumber;
    LoadString(hInst, TEXT_120, szarString, sizeof(szarString));
    sprintf(tempString, szarString, serviceName, runNumber);
    StatusBarText(tempString);
    StatusBar((long)(nI + 1), (long)REPORTPARMS.numRuns);
    if(StatusBarAbort())
    {
      bKeepGoing = FALSE;
      goto deallocate;
    }
    btrieve(B_GETPOSITION, TMS_RUNS, &RUNS, &RUNSKey0, 0);
    btrieve(B_GETDIRECT, TMS_RUNS, &RUNS, &RUNSKey1, 1);
    numPieces = GetRunElements(hWndMain, &RUNS, &PROPOSEDRUN, &COST, TRUE);
//
//  Loop through the pieces
//
    RUNSKey0.recordID = REPORTPARMS.pRunList[nI];
    btrieve(B_GETEQUAL, TMS_RUNS, &RUNS, &RUNSKey0, 0);
    btrieve(B_GETPOSITION, TMS_RUNS, &RUNS, &RUNSKey0, 0);
    btrieve(B_GETDIRECT, TMS_RUNS, &RUNS, &RUNSKey1, 1);
    for(nJ = 0; nJ < numPieces; nJ++)
    {
      if(StatusBarAbort())
      {
        bKeepGoing = FALSE;
        goto deallocate;
      }
//
//  Get all the trips on this run
//
      TRIPSKey0.recordID = RUNS.start.TRIPSrecordID;
      btrieve(B_GETEQUAL, TMS_TRIPS, &TRIPS, &TRIPSKey0, 0);
      btrieve(B_GETPOSITION, TMS_TRIPS, &TRIPS, &TRIPSKey0, 0);
      rcode2 = btrieve(B_GETDIRECT, TMS_TRIPS, &TRIPS, &TRIPSKey2, 2);
      blockNumber = TRIPS.standard.blockNumber;
//
//  Set up the output string
//
//  Block number
//
      sprintf(outputString, "%ld\t", TRIPS.standard.blockNumber);
//
//  Bustype
//
      if(TRIPS.BUSTYPESrecordID != NO_RECORD)
      {
        BUSTYPESKey0.recordID = TRIPS.BUSTYPESrecordID;
        rcode2 = btrieve(B_GETEQUAL, TMS_BUSTYPES, &BUSTYPES, &BUSTYPESKey0, 0);
        strncpy(tempString, BUSTYPES.name, BUSTYPES_NAME_LENGTH);
        trim(tempString, BUSTYPES_NAME_LENGTH);
        strcat(outputString, tempString);
      }
      strcat(outputString, "\t");
//
//  RGRP
//
      ROUTESKey0.recordID = TRIPS.standard.RGRPROUTESrecordID;
      btrieve(B_GETEQUAL, TMS_ROUTES, &ROUTES, &ROUTESKey0, 0);
      strncpy(tempString, ROUTES.number, ROUTES_NUMBER_LENGTH);
      trim(tempString, ROUTES_NUMBER_LENGTH);
      strcat(outputString, tempString);
      strcat(outputString, "\t");
//
//  SGRP
//
      SERVICESKey0.recordID = TRIPS.standard.SGRPSERVICESrecordID;
      btrieve(B_GETEQUAL, TMS_SERVICES, &SERVICES, &SERVICESKey0, 0);
      strncpy(tempString, SERVICES.name, SERVICES_NAME_LENGTH);
      trim(tempString, SERVICES_NAME_LENGTH);
      strcat(outputString, tempString);
      strcat(outputString, "\t");
//
//  Save the string length and loop through the trips
//
      lastPos = strlen(outputString);
      bGotLastTrip = FALSE;
      cumDist = 0.0;
      bFirstTrip = TRUE;
      while(rcode2 == 0 && TRIPS.standard.blockNumber == blockNumber)
      {
        if(StatusBarAbort())
        {
          bKeepGoing = FALSE;
          goto deallocate;
        }
        outputString[lastPos] = '\0';
//
//  Generate the trip
//
        GenerateTrip(TRIPS.ROUTESrecordID, TRIPS.SERVICESrecordID,
              TRIPS.directionIndex, TRIPS.PATTERNNAMESrecordID,
              TRIPS.timeAtMLP, GENERATETRIP_FLAG_DISPLAYERRORS, &GTResults[0]);
//
//  Route number
//
        ROUTESKey0.recordID = TRIPS.ROUTESrecordID;
        btrieve(B_GETEQUAL, TMS_ROUTES, &ROUTES, &ROUTESKey0, 0);
        strncpy(tempString, ROUTES.number, ROUTES_NUMBER_LENGTH);
        trim(tempString, ROUTES_NUMBER_LENGTH);
        strcat(outputString, tempString);
        strcat(outputString, "\t");
//
//  Service
//
        SERVICESKey0.recordID = TRIPS.SERVICESrecordID;
        btrieve(B_GETEQUAL, TMS_SERVICES, &SERVICES, &SERVICESKey0, 0);
        strncpy(tempString, SERVICES.name, SERVICES_NAME_LENGTH);
        trim(tempString, SERVICES_NAME_LENGTH);
        strcat(outputString, tempString);
        strcat(outputString, "\t");
//
//  POG - Check also to see that he starts there
//
        if(TRIPS.standard.POGNODESrecordID != NO_RECORD)
        {
          if(RUNS.start.NODESrecordID == TRIPS.standard.POGNODESrecordID)
          {
            NODESKey0.recordID = TRIPS.standard.POGNODESrecordID;
            btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
            strncpy(tempString, NODES.abbrName, NODES_ABBRNAME_LENGTH);
            trim(tempString, NODES_ABBRNAME_LENGTH);
            strcat(outputString, tempString);
          }
        }
        strcat(outputString, "\t");
//
//  POT
//
        if(TRIPS.standard.POGNODESrecordID == NO_RECORD ||
             TRIPS.standard.POGNODESrecordID != RUNS.start.NODESrecordID)
        {
          PODistance = (float)0.0;
          POTime = 0L;
        }
        else
        {
          GCTData.fromPATTERNNAMESrecordID = TRIPS.PATTERNNAMESrecordID;
          GCTData.toPATTERNNAMESrecordID = TRIPS.PATTERNNAMESrecordID;
          GCTData.fromROUTESrecordID = TRIPS.ROUTESrecordID;
          GCTData.fromSERVICESrecordID = TRIPS.SERVICESrecordID;
          GCTData.toROUTESrecordID = TRIPS.ROUTESrecordID;
          GCTData.toSERVICESrecordID = TRIPS.SERVICESrecordID;
          GCTData.fromNODESrecordID = TRIPS.standard.POGNODESrecordID;
          GCTData.toNODESrecordID = GTResults[0].firstNODESrecordID;
          GCTData.timeOfDay = GTResults[0].firstNodeTime;
          POTime = GTResults[0].firstNodeTime -
                GetConnectionTime(GCT_FLAG_DEADHEADTIME, &GCTData, &distance);
          distance = (float)fabs((double)distance);
          PODistance = distance;
          strcat(outputString, Tchar(POTime));
        }
        strcat(outputString, "\t");
//
//  From node
//
//  Check here to see that the first node on the first trip
//  is the first node on the run.
//
        NODESrecordID = GTResults[0].firstNODESrecordID;
        firstTime = GTResults[0].firstNodeTime;
        if(RUNS.start.TRIPSrecordID == TRIPS.recordID &&
              RUNS.start.NODESrecordID != GTResults[0].firstNODESrecordID)
        {
          pos = 0;
          PATTERNSKey2.ROUTESrecordID = TRIPS.ROUTESrecordID;
          PATTERNSKey2.SERVICESrecordID = TRIPS.SERVICESrecordID;
          PATTERNSKey2.directionIndex = TRIPS.directionIndex;
          PATTERNSKey2.PATTERNNAMESrecordID = TRIPS.PATTERNNAMESrecordID;
          PATTERNSKey2.nodeSequence = 0;
          rcode2 = btrieve(B_GETGREATEROREQUAL, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
          while(rcode2 == 0 &&
                PATTERNS.ROUTESrecordID == TRIPS.ROUTESrecordID &&
                PATTERNS.SERVICESrecordID == TRIPS.SERVICESrecordID &&
                PATTERNS.directionIndex == TRIPS.directionIndex &&
                PATTERNS.PATTERNNAMESrecordID == TRIPS.PATTERNNAMESrecordID)
          {
            if(!(PATTERNS.flags & PATTERNS_FLAG_BUSSTOP))
            {
              if(PATTERNS.NODESrecordID == RUNS.start.NODESrecordID)
              {
                NODESrecordID = PATTERNS.NODESrecordID;
                firstTime = GTResults[0].tripTimes[pos];
                break;
              }
              pos++;
            }
            rcode2 = btrieve(B_GETNEXT, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
          }
        }
        NODESKey0.recordID = NODESrecordID;
        btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
        strncpy(tempString, NODES.abbrName, NODES_ABBRNAME_LENGTH);
        trim(tempString, NODES_ABBRNAME_LENGTH);
        strcat(outputString, tempString);
        strcat(outputString, "\t");
//
//  From time
//
        strcat(outputString, Tchar(firstTime));
        strcat(outputString, "\t");
        GarageDhdTime =
              TRIPS.standard.POGNODESrecordID != NO_RECORD ? GTResults[0].firstNodeTime - POTime : 0;
//
//  To Node/Time - Same type of check as above
//
        NODESrecordID = GTResults[0].lastNODESrecordID;
        lastTime = GTResults[0].lastNodeTime;
        if(RUNS.end.TRIPSrecordID == TRIPS.recordID &&
              RUNS.end.NODESrecordID != GTResults[0].lastNODESrecordID)
        {
          pos = 0;
          PATTERNSKey2.ROUTESrecordID = TRIPS.ROUTESrecordID;
          PATTERNSKey2.SERVICESrecordID = TRIPS.SERVICESrecordID;
          PATTERNSKey2.directionIndex = TRIPS.directionIndex;
          PATTERNSKey2.PATTERNNAMESrecordID = TRIPS.PATTERNNAMESrecordID;
          PATTERNSKey2.nodeSequence = 0;
          rcode2 = btrieve(B_GETGREATEROREQUAL, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
          while(rcode2 == 0 &&
                PATTERNS.ROUTESrecordID == TRIPS.ROUTESrecordID &&
                PATTERNS.SERVICESrecordID == TRIPS.SERVICESrecordID &&
                PATTERNS.directionIndex == TRIPS.directionIndex &&
                PATTERNS.PATTERNNAMESrecordID == TRIPS.PATTERNNAMESrecordID)
          {
            if(!(PATTERNS.flags & PATTERNS_FLAG_BUSSTOP))
            {
              if(PATTERNS.NODESrecordID == RUNS.end.NODESrecordID)
              {
                NODESrecordID = PATTERNS.NODESrecordID;
                lastTime = GTResults[0].tripTimes[pos];
                break;
              }
              pos++;
            }
            rcode2 = btrieve(B_GETNEXT, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
          }
        }
//
//  To time
//
        strcat(outputString, Tchar(lastTime));
        strcat(outputString, "\t");
//
//  To node
//
        NODESKey0.recordID = NODESrecordID;
        btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
        strncpy(tempString, NODES.abbrName, NODES_ABBRNAME_LENGTH);
        trim(tempString, NODES_ABBRNAME_LENGTH);
        strcat(outputString, tempString);
        strcat(outputString, "\t");
//
//  PIG
//
        if(TRIPS.standard.PIGNODESrecordID != NO_RECORD)
        {
          NODESKey0.recordID = TRIPS.standard.PIGNODESrecordID;
          btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
          strncpy(tempString, NODES.abbrName, NODES_ABBRNAME_LENGTH);
          trim(tempString, NODES_ABBRNAME_LENGTH);
          strcat(outputString, tempString);
        }
        strcat(outputString, "\t");
//
//  PIT
//
        if(TRIPS.standard.PIGNODESrecordID == NO_RECORD)
          PIDistance = (float)0.0;
        else
        {
          GCTData.fromPATTERNNAMESrecordID = TRIPS.PATTERNNAMESrecordID;
          GCTData.toPATTERNNAMESrecordID = TRIPS.PATTERNNAMESrecordID;
          GCTData.fromROUTESrecordID = TRIPS.ROUTESrecordID;
          GCTData.fromSERVICESrecordID = TRIPS.SERVICESrecordID;
          GCTData.toROUTESrecordID = TRIPS.ROUTESrecordID;
          GCTData.toSERVICESrecordID = TRIPS.SERVICESrecordID;
          GCTData.fromNODESrecordID = GTResults[0].lastNODESrecordID;
          GCTData.toNODESrecordID = TRIPS.standard.PIGNODESrecordID;
          GCTData.timeOfDay = GTResults[0].lastNodeTime;
          PITime = GetConnectionTime(GCT_FLAG_DEADHEADTIME, &GCTData, &distance);
          distance = (float)fabs((double)distance);
          PIDistance = distance;
          strcpy(tempString, Tchar(GTResults[0].lastNodeTime + PITime));
          strcat(outputString, tempString);
          GarageDhdTime += PITime;
        }
        strcat(outputString, "\t");
//
//  Trip time (in seconds)
//
        sprintf(tempString, "%ld\t", lastTime - firstTime);
        strcat(outputString, tempString);
//
//  Save the Comment and Sign Code
//
        SIGNCODESrecordID = TRIPS.SIGNCODESrecordID;
        COMMENTSrecordID = TRIPS.COMMENTSrecordID;
//
//  Layover and deadhead
//
        if(TRIPS.recordID == RUNS.end.TRIPSrecordID)
        {
          bGotLastTrip = TRUE;
          strcpy(tempString, "\t\t");
        }
        else
        {      
          rcode2 = btrieve(B_GETNEXT, TMS_TRIPS, &TRIPS, &TRIPSKey2, 2);
          layoverTime = 0L;
          deadheadTime = 0L;
          deadheadDist = (float)0.0;
          if(rcode2 == 0 && TRIPS.standard.blockNumber == blockNumber)
          {
            GenerateTrip(TRIPS.ROUTESrecordID, TRIPS.SERVICESrecordID,
                  TRIPS.directionIndex, TRIPS.PATTERNNAMESrecordID,
                  TRIPS.timeAtMLP, GENERATETRIP_FLAG_DISPLAYERRORS, &GTResults[1]);
            if(!NodesEquivalent(GTResults[1].firstNODESrecordID,
                  GTResults[0].lastNODESrecordID, &equivalentTravelTime))
            {
              GCTData.fromPATTERNNAMESrecordID = TRIPS.PATTERNNAMESrecordID;
              GCTData.toPATTERNNAMESrecordID = TRIPS.PATTERNNAMESrecordID;
              GCTData.fromROUTESrecordID = TRIPS.ROUTESrecordID;
              GCTData.fromSERVICESrecordID = TRIPS.SERVICESrecordID;
              GCTData.toROUTESrecordID = TRIPS.ROUTESrecordID;
              GCTData.toSERVICESrecordID = TRIPS.SERVICESrecordID;
              GCTData.fromNODESrecordID = GTResults[0].lastNODESrecordID;
              GCTData.toNODESrecordID = GTResults[1].firstNODESrecordID;
              GCTData.timeOfDay = GTResults[0].lastNodeTime;
              deadheadTime = GetConnectionTime(GCT_FLAG_DEADHEADTIME, &GCTData, &distance);
              distance = (float)fabs((double)distance);
              deadheadDist = deadheadTime == NO_TIME ? (float)0.0 : distance;
              deadheadTime = deadheadTime == NO_TIME ? 0L : deadheadTime;
            }
            layoverTime = GTResults[1].firstNodeTime -
                  GTResults[0].lastNodeTime - deadheadTime;
          }
          sprintf(tempString, "%ld\t%ld\t", layoverTime, deadheadTime);
        }
        strcat(outputString, tempString);
//
//  Distance
//
        sprintf(tempString, "%7.2f\t", GTResults[0].tripDistance);
        strcat(outputString, tempString);
        cumDist += GTResults[0].tripDistance;
//
//  Cumulative distance
//
        sprintf(tempString, "%7.2f\t", cumDist);
        strcat(outputString, tempString);
//
//  Garage deadhead time
//
        sprintf(tempString, "%ld\t", GarageDhdTime);
        strcat(outputString, tempString);
//
//  Garage Distance
//
        sprintf(tempString, "%7.2f\t", PODistance + PIDistance); 
        strcat(outputString, tempString);
//
//  Deadhead distance
//
        sprintf(tempString, "%7.2f\t", deadheadDist); 
        strcat(outputString, tempString);
//
//  Sequence number
//
        sprintf(tempString, "%d\t", seqNum++);
        strcat(outputString, tempString);
//
//  Run number and piece number
//
        sprintf(tempString, "%ld\t%d\t", runNumber, nJ + 1);
        strcat(outputString, tempString);
//
//  Travel time on this piece, as well as report at time and location
//
        tempLong = RUNSVIEW[nJ].runOnTime - RUNSVIEW[nJ].reportTime - RUNSVIEW[nJ].startOfPieceTravel;
        if(RUNSVIEW[nJ].startOfPieceExtraboardStart != NO_TIME)
        {
          tempLong = RUNSVIEW[nJ].startOfPieceExtraboardStart;
        }
        if(bFirstTrip && bGotLastTrip)
        {
          sprintf(tempString, "%ld\t",
                RUNSVIEW[nJ].startOfPieceTravel + RUNSVIEW[nJ].endOfPieceTravel);
        }
        else if(bFirstTrip)
        {
          sprintf(tempString, "%ld\t", RUNSVIEW[nJ].startOfPieceTravel);
          bFirstTrip = FALSE;
        }
        else if(bGotLastTrip)
        {
          sprintf(tempString, "%ld\t", RUNSVIEW[nJ].endOfPieceTravel);
        }
        else
        {
          strcpy(tempString, "\t");
        }
//
//  Report at and time
//
        if(RUNSVIEW[nJ].reportTime == 0)
        {
          sprintf(szarString, "\t\t");
        }
        else
        {
          NODESKey0.recordID = RUNSVIEW[nJ].startOfPieceNODESrecordID;
          btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
          strncpy(szNodeName, NODES.abbrName, NODES_ABBRNAME_LENGTH);
          trim(szNodeName, NODES_ABBRNAME_LENGTH);
          sprintf(szarString, "%s\t%s\t", Tchar(tempLong), szNodeName);
        }
        strcat(tempString, szarString);
        strcat(outputString, tempString);
//
//  Sign code
//
        if(SIGNCODESrecordID == NO_RECORD)
        {
          strcpy(tempString, "\t");
        }
        else
        {
          SIGNCODESKey0.recordID = SIGNCODESrecordID;
          btrieve(B_GETEQUAL, TMS_SIGNCODES, &SIGNCODES, &SIGNCODESKey0, 0);
          strncpy(szarString, SIGNCODES.code, SIGNCODES_CODE_LENGTH);
          trim(szarString, SIGNCODES_CODE_LENGTH);
          strcpy(tempString, szarString);
          strcat(tempString, "\t");
        }
        strcat(outputString, tempString);
//
//  Comment code
//
        if(COMMENTSrecordID == NO_RECORD)
        {
          strcpy(tempString, "\t");
        }
        else
        {
          COMMENTSKey0.recordID = COMMENTSrecordID;
          recordLength[TMS_COMMENTS] = COMMENTS_TOTAL_LENGTH;
          rcode2 = btrieve(B_GETEQUAL, TMS_COMMENTS, pCommentText, &COMMENTSKey0, 0);
          recordLength[TMS_COMMENTS] = COMMENTS_FIXED_LENGTH;
          if(rcode2 == 0)
          {
            memcpy(&COMMENTS, pCommentText, COMMENTS_FIXED_LENGTH);
            strncpy(szarString, COMMENTS.code, COMMENTS_CODE_LENGTH);
            trim(szarString, COMMENTS_CODE_LENGTH);
          }
          else
          {
            strcpy(szarString, "");
          }
          strcpy(tempString, szarString);
          strcat(tempString, "\t");
        }
        strcat(outputString, tempString);
//
//  Extraboard
//
        if(RUNSVIEW[nJ].startOfPieceExtraboardStart != NO_TIME)
        {
          strcpy(tempString, Tchar(RUNSVIEW[nJ].startOfPieceExtraboardStart));
          strcat(tempString, "\t");
          strcat(tempString, Tchar(RUNSVIEW[nJ].startOfPieceExtraboardEnd));
          strcat(tempString, "\t");
        }
        else if(RUNSVIEW[nJ].endOfPieceExtraboardStart != NO_TIME)
        {
          strcpy(tempString, Tchar(RUNSVIEW[nJ].endOfPieceExtraboardStart));
          strcat(tempString, "\t");
          strcat(tempString, Tchar(RUNSVIEW[nJ].endOfPieceExtraboardEnd));
          strcat(tempString, "\t");
        }
        else
        {
          strcpy(tempString, "\t\t");
        }
        strcat(outputString, tempString);
//
//  Spread Time
//
        sprintf(tempString, "%s\t", chhmm(COST.spreadTime));
        strcat(outputString, tempString);
//
//  Pay time
//
        sprintf(tempString, "%s\r\n", chhmm(COST.TOTAL.payTime));
        strcat(outputString, tempString);
//
//  Write it out
//
        WriteFile(hOutputFile, (LPCVOID *)outputString,
              strlen(outputString), &dwBytesWritten, NULL);
//
//  Leave if need be
//
        if(bGotLastTrip)
          break;
      }  // while looping through trips
//
//  Get the next RUNS record
//
      rcode2 = btrieve(B_GETNEXT, TMS_RUNS, &RUNS, &RUNSKey1, 1);
    }  // nJ
  }  // nI
  bKeepGoing = TRUE;
//
//  Free allocated memory
//
  deallocate:
    TMSHeapFree(REPORTPARMS.pServiceList);
    TMSHeapFree(REPORTPARMS.pDivisionList);
    TMSHeapFree(REPORTPARMS.pRunList);
    StatusBarEnd();
    CloseHandle(hOutputFile);
    if(!bKeepGoing)
    {
      return(FALSE);
    }
//
//  All done
//
  return(TRUE);
}



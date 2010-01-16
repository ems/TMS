//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
//
//  TMSRPT22() - Block Paddle (2)
//
#include "TMSHeader.h"
#include <math.h>

extern int CISbuild( int, int );	// Call to initialize the trip planner.

typedef struct RSDStruct
{
  long ROUTESrecordID;
  long SERVICESrecordID;
  long directionIndex;
  long patternIndex;
  long endIndex;
} RSDDef;

typedef struct TITLENODEStruct
{
  long NODESrecordID;
  long ROUTESrecordID;
} TITLENODEDef;

#define MAXTITLENODES 20

//
//  From QETXT.INI:
//
//  [TMSRPT22]
//  FILE=TMSRPT22.txt
//  FLN=0
//  TT=Tab
//  Charset=ANSI
//  DS=.
//  FIELD1=Sequence,NUMERIC,7,0,7,0,
//  FIELD2=Division,VARCHAR,32,0,32,0,
//  FIELD3=Service,VARCHAR,32,0,32,0,
//  FIELD4=Block,NUMERIC,7,0,7,0,
//  FIELD5=Text,VARCHAR,255,0,255,0,
//

BOOL FAR TMSRPT22(TMSRPTPassedDataDef *pPassedData)
{
  GenerateTripDef GTResults;
  GetConnectionTimeDef GCTData;
  REPORTPARMSDef REPORTPARMS;
  TITLENODEDef   TITLENODES[RECORDIDS_KEPT];
  PROPOSEDRUNDef PROPOSEDRUN;
  COSTDef COST;
  RSDDef  RSD[RECORDIDS_KEPT];
  HFILE hfOutputFile;
  float distance;
  BOOL  bGotOne;
  BOOL  bKeepGoing = FALSE;
  BOOL  bRC;
  BOOL  bFound;
  BOOL  bFirst;
  BOOL  bIncludeSignCodes;
  char  outputString[512];
  char  titleString[2][512];
  char  routeNumber[ROUTES_NUMBER_LENGTH + 1];
  char  routeName[ROUTES_NAME_LENGTH + 1];
  char  serviceName[SERVICES_NAME_LENGTH + 1];
  char  divisionName[DIVISIONS_NAME_LENGTH + 1];
  char  longNames[RECORDIDS_KEPT][NODES_LONGNAME_LENGTH + 1];
  long  subPatternNodeList[RECORDIDS_KEPT];
  long  POTime;
  long  PITime;
  long  deadheadTime;
  long  runNumber[2];
  long  reliefNode;
  long  reliefTime;
  long  assignedToNODESrecordID;
  long  RGRPROUTESrecordID;
  long  SGRPSERVICESrecordID;
  long  POGNODESrecordID;
  long  blockNumber;
  long  routesInUse[RECORDIDS_KEPT];
  long  timeOfReport;
  long  outputTimes[RECORDIDS_KEPT + 1];
  long  signCodeRecordIDs[RECORDIDS_KEPT];
  int   numSignCodes;
  int   nI;
  int   nJ;
  int   nK;
  int   numLines;
  int   numNodes;
  int   numTitleNodes;
  int   rcode2;
  int   seq;
  int   numRSD;
  int   numRoutesInUse;
  int   patternIndex;
  int   previousPatternIndex;
  int   endIndex;
  int   numRunNumbers;

  pPassedData->nReportNumber = 21;
  pPassedData->numDataFiles = 1;
//
//  See what he wants
//
  REPORTPARMS.nReportNumber = pPassedData->nReportNumber;
  REPORTPARMS.ROUTESrecordID = m_RouteRecordID;
  REPORTPARMS.SERVICESrecordID = m_ServiceRecordID;
  REPORTPARMS.DIVISIONSrecordID = m_DivisionRecordID;
  REPORTPARMS.COMMENTSrecordID = NO_RECORD;
  REPORTPARMS.pRouteList = NULL;
  REPORTPARMS.pServiceList = NULL;
  REPORTPARMS.pDivisionList = NULL;
  REPORTPARMS.pBlockList = NULL;
  REPORTPARMS.flags = RPFLAG_ROUTES | RPFLAG_SERVICES | RPFLAG_NOALLSERVICES |
        RPFLAG_DIVISIONS | RPFLAG_NOALLDIVISIONS | RPFLAG_COMMENTS |
        RPFLAG_BLOCKS | RPFLAG_STANDARDBLOCKS | RPFLAG_SIGNCODES;
  bRC = DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_RPTPARMS),
        hWndMain, (DLGPROC)RPTPARMSMsgProc, (LPARAM)&REPORTPARMS);
  if(!bRC)
  {
    return(FALSE);
  }
//
//  Check the list of routes, services, and divisions
//
  if(REPORTPARMS.numRoutes == 0 ||
        REPORTPARMS.numServices == 0 ||
        REPORTPARMS.numDivisions == 0)
  {
    goto deallocate;
  }

//
//  Intialize the trip planner to be used to compute connections for the runs.
//
  if(bUseCISPlan)
  {
    CISbuild( 0, 1 );	// Build for drivers, show status bar.
  }

//
//  Does he want a summary of signcodes?
//
  bIncludeSignCodes = REPORTPARMS.returnedFlags & RPFLAG_SIGNCODES;
//
//  Open the output file
//
  strcpy(tempString, szReportsTempFolder);
  strcat(tempString, "\\TMSRPT22.txt");
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
//  Fire up the status bar
//
  LoadString(hInst, TEXT_117, tempString, TEMPSTRING_LENGTH);
  StatusBarStart(hWndMain, tempString);
//
//  Get all the runs
//
  GetRunRecords(REPORTPARMS.pDivisionList[0], REPORTPARMS.pServiceList[0]);
  if(m_numRunRecords == 0)
  {
    bKeepGoing = FALSE;
    goto deallocate;
  }
//
//  Set up the division name
//
  DIVISIONSKey0.recordID = REPORTPARMS.pDivisionList[0];
  btrieve(B_GETEQUAL, TMS_DIVISIONS, &DIVISIONS, &DIVISIONSKey0, 0);
  strncpy(divisionName, DIVISIONS.name, DIVISIONS_NAME_LENGTH);
  trim(divisionName, DIVISIONS_NAME_LENGTH);
//
//  Loop through the Blocks
//
  bKeepGoing = TRUE;
  seq = 0;
  numLines = 0;
  for(nI = 0; nI < REPORTPARMS.numBlocks; nI++)
  {
//
//  Get the first trip on the block
//
    TRIPSKey0.recordID = REPORTPARMS.pBlockList[nI];
    btrieve(B_GETEQUAL, TMS_TRIPS, &TRIPS, &TRIPSKey0, 0);
    assignedToNODESrecordID = TRIPS.standard.assignedToNODESrecordID;
    RGRPROUTESrecordID = TRIPS.standard.RGRPROUTESrecordID;
    SGRPSERVICESrecordID = TRIPS.standard.SGRPSERVICESrecordID;
    POGNODESrecordID = TRIPS.standard.POGNODESrecordID;
    blockNumber = TRIPS.standard.blockNumber;
//
//  Get the route and service name
//
    ROUTESKey0.recordID = TRIPS.standard.RGRPROUTESrecordID;
    btrieve(B_GETEQUAL, TMS_ROUTES, &ROUTES, &ROUTESKey0, 0);
    strncpy(routeNumber, ROUTES.number, ROUTES_NUMBER_LENGTH);
    trim(routeNumber, ROUTES_NUMBER_LENGTH);
    strncpy(routeName, ROUTES.name, ROUTES_NAME_LENGTH);
    trim(routeName, ROUTES_NAME_LENGTH);
    SERVICESKey0.recordID = TRIPS.standard.SGRPSERVICESrecordID;
    btrieve(B_GETEQUAL, TMS_SERVICES, &SERVICES, &SERVICESKey0, 0);
    strncpy(serviceName, SERVICES.name, SERVICES_NAME_LENGTH);
    trim(serviceName, SERVICES_NAME_LENGTH);
//
//  Display them in the status bar
//
    LoadString(hInst, TEXT_040, szFormatString, sizeof(szFormatString));
    sprintf(tempString, szFormatString, routeNumber, routeName, serviceName, blockNumber);
    StatusBarText(tempString);
    StatusBar((long)nI, (long)REPORTPARMS.numBlocks);
    if(StatusBarAbort())
    {
      bKeepGoing = FALSE;
      goto deallocate;
    }
    btrieve(B_GETPOSITION, TMS_TRIPS, &TRIPS, &TRIPSKey0, 0);
    rcode2 = btrieve(B_GETDIRECT, TMS_TRIPS, &TRIPS, &TRIPSKey2, 2);
//
//  Get the BASE pattern nodes for the trips in this block.  Keep track of
//  which route / services / directions are used.  Cancel out of this block
//  if there are more than 12 nodes used, as the title can't have more than
//  those 12 and still fit on 8.5 x 11 paper.
//
    numTitleNodes = 0;
    numRSD = 0;
    numRoutesInUse = 0;
    numSignCodes = 0;
    while(rcode2 == 0 &&
          TRIPS.standard.assignedToNODESrecordID == assignedToNODESrecordID &&
          TRIPS.standard.RGRPROUTESrecordID == RGRPROUTESrecordID &&
          TRIPS.standard.SGRPSERVICESrecordID == SGRPSERVICESrecordID &&
          TRIPS.standard.blockNumber == blockNumber)
    {
      for(bFound = FALSE, nJ = 0; nJ < numRoutesInUse; nJ++)
      {
        if(routesInUse[nJ] == TRIPS.ROUTESrecordID)
        {
          bFound = TRUE;
          break;
        }
      }
      if(!bFound)
      {
        routesInUse[numRoutesInUse] = TRIPS.ROUTESrecordID;
        numRoutesInUse++;
      }
      for(bFound = FALSE, nJ = 0; nJ < numRSD; nJ++)
      {
        if(RSD[nJ].ROUTESrecordID == TRIPS.ROUTESrecordID &&
              RSD[nJ].SERVICESrecordID == TRIPS.SERVICESrecordID &&
              RSD[nJ].directionIndex == TRIPS.directionIndex)
        {
          bFound = TRUE;
          break;
        }
      }
      if(!bFound)
      {
        RSD[numRSD].ROUTESrecordID = TRIPS.ROUTESrecordID;
        RSD[numRSD].SERVICESrecordID = TRIPS.SERVICESrecordID;
        RSD[numRSD].directionIndex = TRIPS.directionIndex;
        RSD[numRSD].patternIndex = numTitleNodes;
        numRSD++;
        PATTERNSKey2.ROUTESrecordID = TRIPS.ROUTESrecordID;
        PATTERNSKey2.SERVICESrecordID = TRIPS.SERVICESrecordID;
        PATTERNSKey2.directionIndex = TRIPS.directionIndex;
        PATTERNSKey2.PATTERNNAMESrecordID = basePatternRecordID;
        PATTERNSKey2.nodeSequence = 0;
        rcode2 = btrieve(B_GETGREATEROREQUAL, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
        while(rcode2 == 0 &&
              PATTERNS.ROUTESrecordID == TRIPS.ROUTESrecordID &&
              PATTERNS.SERVICESrecordID == TRIPS.SERVICESrecordID &&
              PATTERNS.directionIndex == TRIPS.directionIndex &&
              PATTERNS.PATTERNNAMESrecordID == basePatternRecordID)
        {
          if(!(PATTERNS.flags & PATTERNS_FLAG_BUSSTOP))
          {
            TITLENODES[numTitleNodes].NODESrecordID = PATTERNS.NODESrecordID;
            TITLENODES[numTitleNodes].ROUTESrecordID = PATTERNS.ROUTESrecordID;
            numTitleNodes++;
          }
          if(numTitleNodes > MAXTITLENODES)
            break;
          rcode2 = btrieve(B_GETNEXT, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
        }  // while rcode2 == 0 on the pattern
        if(numTitleNodes > MAXTITLENODES)
          break;
      }
      rcode2 = btrieve(B_GETNEXT, TMS_TRIPS, &TRIPS, &TRIPSKey2, 2);
    }
//
//  Make sure we're within limits
//
    if(numTitleNodes > MAXTITLENODES)
    {
      TMSError(NULL, MB_ICONINFORMATION, ERROR_249, NULL);
      continue;
    }
//
//  Ok - we're legit
//
//  Set up the ending index in RSD
//
    for(nJ = 0; nJ < numRSD - 1; nJ++)
    {
      RSD[nJ].endIndex = RSD[nJ + 1].patternIndex;
    }
    RSD[numRSD - 1].endIndex = numTitleNodes;

//
//  Set up the first output string - this one contains the route
//  numbers and names of the trips that form the block
//
    for(nJ = 0; nJ < numRoutesInUse; nJ++)
    {
      ROUTESKey0.recordID = routesInUse[nJ];
      btrieve(B_GETEQUAL, TMS_ROUTES, &ROUTES, &ROUTESKey0, 0);
      strncpy(routeNumber, ROUTES.number, sizeof(ROUTES.number));
      trim(routeNumber, sizeof(ROUTES.number));
      strncpy(routeName, ROUTES.name, sizeof(ROUTES.name));
      trim(routeName, sizeof(ROUTES.name));
      sprintf(outputString, "%ld\t%s\t%s\t%ld\t\"#%s %s\"\r\n",
            seq++, divisionName, serviceName, blockNumber, routeNumber, routeName);
      _lwrite(hfOutputFile, outputString, strlen(outputString));
      numLines++;
    }
    sprintf(outputString, "%ld\t%s\t%s\t%ld\t\" \"\r\n",
            seq++, divisionName, serviceName, blockNumber);
    _lwrite(hfOutputFile, outputString, strlen(outputString));
    numLines++;
//
//  Set up the second output string - this one contains the report
//  time of the block and the route numbers shown at each timepoint
//  along the way.
//
//  Generate the first trip
//
    TRIPSKey0.recordID = REPORTPARMS.pBlockList[nI];
    btrieve(B_GETEQUAL, TMS_TRIPS, &TRIPS, &TRIPSKey0, 0);
    GenerateTrip(TRIPS.ROUTESrecordID, TRIPS.SERVICESrecordID,
          TRIPS.directionIndex, TRIPS.PATTERNNAMESrecordID,
          TRIPS.timeAtMLP, GENERATETRIP_FLAG_DISPLAYERRORS, &GTResults);
//
//  Is this a pull-out?  (It should be)
//
    POTime = NO_TIME;
    if(TRIPS.standard.POGNODESrecordID != NO_RECORD)
    {
      GCTData.fromPATTERNNAMESrecordID = TRIPS.PATTERNNAMESrecordID;
      GCTData.toPATTERNNAMESrecordID = TRIPS.PATTERNNAMESrecordID;
      GCTData.fromROUTESrecordID = TRIPS.ROUTESrecordID;
      GCTData.fromSERVICESrecordID = TRIPS.SERVICESrecordID;
      GCTData.toROUTESrecordID = TRIPS.ROUTESrecordID;
      GCTData.toSERVICESrecordID = TRIPS.SERVICESrecordID;
      GCTData.fromNODESrecordID = TRIPS.standard.POGNODESrecordID;
      GCTData.toNODESrecordID = GTResults.firstNODESrecordID;
      GCTData.timeOfDay = GTResults.firstNodeTime;
      deadheadTime = GetConnectionTime(GCT_FLAG_DEADHEADTIME, &GCTData, &distance);
      distance = (float)fabs((double)distance);
      if(deadheadTime != NO_TIME)
      {
        POTime = GTResults.firstNodeTime - deadheadTime;
      }
    }
//
//  Get the run recordID on the first trip
//
    timeOfReport = NO_TIME;
    if(POTime != NO_TIME)
    {
      for(bGotOne = FALSE, nJ = 0; nJ < m_numRunRecords; nJ++)
      {
        if(m_pRunRecordData[nJ].startTRIPSrecordID == TRIPS.recordID)
        {
          RUNSKey0.recordID = m_pRunRecordData[nJ].recordID;
          bGotOne = TRUE;
          break;
        }
      }
      if(bGotOne)
      {
        btrieve(B_GETEQUAL, TMS_RUNS, &RUNS, &RUNSKey0, 0);
        GetRunElements(NULL, &RUNS, &PROPOSEDRUN, &COST, TRUE);
        timeOfReport = POTime - RUNSVIEW[0].reportTime;
        if(RUNSVIEW[0].startOfPieceExtraboardStart != NO_TIME)
          timeOfReport = RUNSVIEW[0].startOfPieceExtraboardStart;
        sprintf(outputString, "%ld\t%s\t%s\t%ld\t\"Report at %s\"\r\n",
              seq++, divisionName, serviceName, blockNumber, Tchar(timeOfReport));
        _lwrite(hfOutputFile, outputString, strlen(outputString));
        numLines++;
      }
    }
//
//  Loop through the route numbers
//
    sprintf(outputString, "%ld\t%s\t%s\t%ld\t\"                  ",
          seq++, divisionName, serviceName, blockNumber);
    for(nJ = 0; nJ < numTitleNodes; nJ++)
    {
      if(nJ == 0 || TITLENODES[nJ].ROUTESrecordID != TITLENODES[nJ - 1].ROUTESrecordID)
      {
        ROUTESKey0.recordID = TITLENODES[nJ].ROUTESrecordID;
        btrieve(B_GETEQUAL, TMS_ROUTES, &ROUTES, &ROUTESKey0, 0);
        strncpy(routeNumber, ROUTES.number, ROUTES_NUMBER_LENGTH);
        trim(routeNumber, ROUTES_NUMBER_LENGTH);
      }
      strcpy(szarString, "#");
      strcat(szarString, routeNumber);
      sprintf(tempString, "  %6s", szarString);
      strcat(outputString, tempString);
    }
    strcat(outputString, "\"\r\n");
    _lwrite(hfOutputFile, outputString, strlen(outputString));
    numLines++;
    sprintf(outputString, "%ld\t%s\t%s\t%ld\t\" \"\r\n",
            seq++, divisionName, serviceName, blockNumber);
    _lwrite(hfOutputFile, outputString, strlen(outputString));
    numLines++;
//
//  Set up the third and fouth output strings - the node headers
//
//
//  Store the long names
//
    for(nJ = 0; nJ < numTitleNodes; nJ++)
    {
      NODESKey0.recordID = TITLENODES[nJ].NODESrecordID;
      btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
      strncpy(longNames[nJ], NODES.longName, NODES_LONGNAME_LENGTH);
      longNames[nJ][NODES_LONGNAME_LENGTH] = '\0';
    }
//
//  Set up the title string
//
    for(nJ = 0; nJ < 2; nJ++)
    {
      if(nJ == 0)
      {
        strcpy(titleString[nJ], "            Time     ");
        for(nK = 0; nK < numTitleNodes; nK++) 
        {
          strncpy(tempString, longNames[nK], 4);
          tempString[4] = '\0';
          strcat(titleString[nJ], tempString);
          strcat(titleString[nJ], "    ");
        }
        strcat(titleString[nJ], "Time\"\r\n");
      }
      else
      {
        strcpy(titleString[nJ], "Run(s)       Out     ");
        for(nK = 0; nK < numTitleNodes; nK++)
        {
          strncpy(tempString, &longNames[nK][4], 4);
          tempString[4] = '\0';
          strcat(titleString[nJ], tempString);
          strcat(titleString[nJ], "    ");
        }
        strcat(titleString[nJ], " In\"\r\n"); 
      }
    }
    sprintf(outputString, "%ld\t%s\t%s\t%ld\t\"%s",
            seq++, divisionName, serviceName, blockNumber, titleString[0]);
    _lwrite(hfOutputFile, outputString, strlen(outputString));
    numLines++;
    sprintf(outputString, "%ld\t%s\t%s\t%ld\t\"%s",
            seq++, divisionName, serviceName, blockNumber, titleString[1]);
    _lwrite(hfOutputFile, outputString, strlen(outputString));
    numLines++;
    sprintf(outputString, "%ld\t%s\t%s\t%ld\t\" \"\r\n",
            seq++, divisionName, serviceName, blockNumber);
    _lwrite(hfOutputFile, outputString, strlen(outputString));
    numLines++;
//
//  Go through all the trips for this block
//
    bFirst = TRUE;
    for(nJ = 0; nJ < RECORDIDS_KEPT; nJ++)
    {
      outputTimes[nJ] = NO_TIME;
    }
    TRIPSKey0.recordID = REPORTPARMS.pBlockList[nI];
    btrieve(B_GETEQUAL, TMS_TRIPS, &TRIPS, &TRIPSKey0, 0);
    btrieve(B_GETPOSITION, TMS_TRIPS, &TRIPS, &TRIPSKey0, 0);
    rcode2 = btrieve(B_GETDIRECT, TMS_TRIPS, &TRIPS, &TRIPSKey2, 2);
    previousPatternIndex = NO_RECORD;
    numRunNumbers = 0;
    while(rcode2 == 0 &&
          TRIPS.standard.assignedToNODESrecordID == assignedToNODESrecordID &&
          TRIPS.standard.RGRPROUTESrecordID == RGRPROUTESrecordID &&
          TRIPS.standard.SGRPSERVICESrecordID == SGRPSERVICESrecordID &&
          TRIPS.standard.blockNumber == blockNumber)
    {
//
//  Are we keeping track of sign codes?
//
    if(bIncludeSignCodes && TRIPS.SIGNCODESrecordID != NO_RECORD)
    {
      for(bFound = FALSE, nJ = 0; nJ < numSignCodes; nJ++)
      {
        if(TRIPS.SIGNCODESrecordID == signCodeRecordIDs[nJ])
        {
          bFound = TRUE;
          break;
        }
      }
      if(!bFound)
      {
        signCodeRecordIDs[numSignCodes] = TRIPS.SIGNCODESrecordID;
        numSignCodes++;
      }
    }
//
//  Generate the trip
//
      GenerateTrip(TRIPS.ROUTESrecordID, TRIPS.SERVICESrecordID,
            TRIPS.directionIndex, TRIPS.PATTERNNAMESrecordID,
            TRIPS.timeAtMLP, GENERATETRIP_FLAG_DISPLAYERRORS, &GTResults);
//
//  Is this a pull-out?
//
      POTime = NO_TIME;
      if(TRIPS.standard.POGNODESrecordID != NO_RECORD)
      {
        GCTData.fromPATTERNNAMESrecordID = TRIPS.PATTERNNAMESrecordID;
        GCTData.toPATTERNNAMESrecordID = TRIPS.PATTERNNAMESrecordID;
        GCTData.fromROUTESrecordID = TRIPS.ROUTESrecordID;
        GCTData.fromSERVICESrecordID = TRIPS.SERVICESrecordID;
        GCTData.toROUTESrecordID = TRIPS.ROUTESrecordID;
        GCTData.toSERVICESrecordID = TRIPS.SERVICESrecordID;
        GCTData.fromNODESrecordID = TRIPS.standard.POGNODESrecordID;
        GCTData.toNODESrecordID = GTResults.firstNODESrecordID;
        GCTData.timeOfDay = GTResults.firstNodeTime;
        deadheadTime = GetConnectionTime(GCT_FLAG_DEADHEADTIME, &GCTData, &distance);
        distance = (float)fabs((double)distance);
        if(deadheadTime != NO_TIME)
        {
          POTime = GTResults.firstNodeTime - deadheadTime;
          strcpy(szarString, (timeFormat == PREFERENCES_MILITARY ? " " : ""));
          strcat(szarString, Tchar(POTime));
          strcat(szarString, "    ");
        }
      }
//
//  Find the position in outputTimes where the times start
//
      for(nJ = 0; nJ < numRSD; nJ++)
      {
        if(TRIPS.ROUTESrecordID == RSD[nJ].ROUTESrecordID &&
              TRIPS.SERVICESrecordID == RSD[nJ].SERVICESrecordID &&
              TRIPS.directionIndex == RSD[nJ].directionIndex)
        {
          patternIndex = RSD[nJ].patternIndex;
          endIndex = RSD[nJ].endIndex;
          break;
        }
      }
//
//  If the previous patternIndex is less than or equal to the current patternIndex,
//  it means that we're starting a new line - so dump out the previous one first
//
      if(patternIndex <= previousPatternIndex && !bFirst)
      {
        sprintf(outputString, "%ld\t%s\t%s\t%ld\t\"",
              seq++, divisionName, serviceName, blockNumber);
        if(runNumber[0] == runNumber[1])
          sprintf(tempString, "%4ld       ", runNumber[0]);
        else
          sprintf(tempString, "%4ld/%4ld  ", runNumber[0], runNumber[1]);
        strcat(tempString, szarString);
        strcat(outputString, tempString);
        for(bGotOne = FALSE, nJ = 0; nJ < numTitleNodes; nJ++)
        {
          if(timeFormat == PREFERENCES_MILITARY)
            strcat(outputString, " ");
          strcat(outputString, Tchar(outputTimes[nJ]));
          if(outputTimes[nJ] == NO_TIME)
          {
            strcat(outputString, "    ");
            strcat(outputString, (timeFormat == PREFERENCES_MILITARY ? "" : " "));
          }
          if(TITLENODES[nJ].NODESrecordID == reliefNode &&
                 outputTimes[nJ] == reliefTime && !bGotOne)
          {
            strcat(outputString, "*  ");
            bGotOne = TRUE;
          }
          else
            strcat(outputString, "   ");
        }
        strcat(outputString, "\"\r\n");
        _lwrite(hfOutputFile, outputString, strlen(outputString));
        numLines++;
        for(nJ = 0; nJ < RECORDIDS_KEPT; nJ++)
        {
          outputTimes[nJ] = NO_TIME;
        }
        strcpy(szarString,  "         ");
        reliefNode = NO_RECORD;
        runNumber[0] = 0;
        runNumber[1] = 0;
        numRunNumbers = 0;
      }
      bFirst = FALSE;
      previousPatternIndex = patternIndex;
//
//  Get the run number of the trip
//
      if(numRunNumbers < 2)
      {
        for(bGotOne = FALSE, nJ = 0; nJ < m_numRunRecords; nJ++)
        {
          if(m_pRunRecordData[nJ].startTRIPSrecordID == TRIPS.recordID ||
                (m_pRunRecordData[nJ].blockNumber == TRIPS.standard.blockNumber &&
                 GTResults.firstNodeTime >= m_pRunRecordData[nJ].startTime &&
                 GTResults.firstNodeTime <= m_pRunRecordData[nJ].endTime))
          {
            if(bGotOne)
            {
              runNumber[1] = m_pRunRecordData[nJ].runNumber;
              reliefNode = m_pRunRecordData[nJ].startNODESrecordID;
              reliefTime = m_pRunRecordData[nJ].startTime;
              numRunNumbers = 2;
              break;
            }
            else
            {
              runNumber[0] = m_pRunRecordData[nJ].runNumber;
              runNumber[1] = m_pRunRecordData[nJ].runNumber;
//            reliefNode = NO_RECORD;
              numRunNumbers = 1;
              bGotOne = TRUE;
            }
          }
        }
      }
//
//  Save the trip times
//
//  If necessary, get the pattern nodes on the trip
//
      if(TRIPS.PATTERNNAMESrecordID == basePatternRecordID)
      {
        nK = 0;
        for(nJ = patternIndex; nJ < endIndex; nJ++)
        {
          outputTimes[nJ] = GTResults.tripTimes[nK];
          nK++;
        }
      }
//
//  Not the BASE pattern - get the pattern and fill in the times
//
      else
      {
        PATTERNSKey2.ROUTESrecordID = TRIPS.ROUTESrecordID;
        PATTERNSKey2.SERVICESrecordID = TRIPS.SERVICESrecordID;
        PATTERNSKey2.directionIndex = TRIPS.directionIndex;
        PATTERNSKey2.PATTERNNAMESrecordID = TRIPS.PATTERNNAMESrecordID;
        PATTERNSKey2.nodeSequence = 0;
        rcode2 = btrieve(B_GETGREATEROREQUAL, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
        numNodes = 0;
        while(rcode2 == 0 &&
              PATTERNS.ROUTESrecordID == TRIPS.ROUTESrecordID &&
              PATTERNS.SERVICESrecordID == TRIPS.SERVICESrecordID &&
              PATTERNS.directionIndex == TRIPS.directionIndex &&
              PATTERNS.PATTERNNAMESrecordID == TRIPS.PATTERNNAMESrecordID)
        {
          if(!(PATTERNS.flags & PATTERNS_FLAG_BUSSTOP))
            subPatternNodeList[numNodes++] = PATTERNS.NODESrecordID;
          rcode2 = btrieve(B_GETNEXT, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
        }
//
//  Move along the pattern
//
        nK = 0;
        for(nJ = patternIndex; nJ < endIndex; nJ++)
        {
          if(nK >= numNodes)
            break;
          if(TITLENODES[nJ].NODESrecordID == subPatternNodeList[nK])
          {
            outputTimes[nJ] = GTResults.tripTimes[nK];
            nK++;
          }
        }
      }
//  
//  Is this a pull-in?
//
      PITime = NO_TIME;
      if(TRIPS.standard.PIGNODESrecordID != NO_RECORD)
      {
        GCTData.fromPATTERNNAMESrecordID = TRIPS.PATTERNNAMESrecordID;
        GCTData.toPATTERNNAMESrecordID = TRIPS.PATTERNNAMESrecordID;
        GCTData.fromROUTESrecordID = TRIPS.ROUTESrecordID;
        GCTData.fromSERVICESrecordID = TRIPS.SERVICESrecordID;
        GCTData.toROUTESrecordID = TRIPS.ROUTESrecordID;
        GCTData.toSERVICESrecordID = TRIPS.SERVICESrecordID;
        GCTData.fromNODESrecordID = GTResults.lastNODESrecordID;
        GCTData.toNODESrecordID = TRIPS.standard.PIGNODESrecordID;
        GCTData.timeOfDay = GTResults.lastNodeTime;
        deadheadTime = GetConnectionTime(GCT_FLAG_DEADHEADTIME, &GCTData, &distance);
        distance = (float)fabs((double)distance);
        if(deadheadTime != NO_TIME)
          PITime = GTResults.lastNodeTime + deadheadTime;
      }
      outputTimes[numTitleNodes] = PITime;
      rcode2 = btrieve(B_GETNEXT, TMS_TRIPS, &TRIPS, &TRIPSKey2, 2);
    }  //  while loop on block
    if(!bFirst)
    {
      sprintf(outputString, "%ld\t%s\t%s\t%ld\t\"",
            seq++, divisionName, serviceName, blockNumber);
      if(runNumber[0] == runNumber[1])
        sprintf(tempString, "%4ld       ", runNumber[0]);
      else
        sprintf(tempString, "%4ld/%4ld  ", runNumber[0], runNumber[1]);
      strcat(tempString, szarString);
      strcat(outputString, tempString);
      for(bGotOne = FALSE, nJ = 0; nJ < numTitleNodes + 1; nJ++)
      {
        if(timeFormat == PREFERENCES_MILITARY)
          strcat(outputString, " ");
        strcat(outputString, Tchar(outputTimes[nJ]));
        if(outputTimes[nJ] == NO_TIME)
        { 
          strcat(outputString, "    ");
          strcat(outputString, (timeFormat == PREFERENCES_MILITARY ? "" : " "));
        }
        if(TITLENODES[nJ].NODESrecordID == reliefNode &&
              outputTimes[nJ] == reliefTime && !bGotOne)
        {
          strcat(outputString, "*  ");
          bGotOne = TRUE;
        }
        else
          strcat(outputString, "   ");
        outputTimes[nJ] = NO_TIME;
      }
      strcat(outputString, "\"\r\n");
      _lwrite(hfOutputFile, outputString, strlen(outputString));
      numLines++;
    }
//
//  Signcodes?
//
    if(bIncludeSignCodes && numSignCodes > 0)
    {
      sprintf(outputString, "%ld\t%s\t%s\t%ld\t\" \"\r\n",
            seq++, divisionName, serviceName, blockNumber);
      _lwrite(hfOutputFile, outputString, strlen(outputString));
      numLines++;
      sprintf(outputString, "%ld\t%s\t%s\t%ld\t\"Signcodes:\"\r\n",
            seq++, divisionName, serviceName, blockNumber);
      _lwrite(hfOutputFile, outputString, strlen(outputString));
      numLines++;
      for(nJ = 0; nJ < numSignCodes; nJ++)
      {
        SIGNCODESKey0.recordID = signCodeRecordIDs[nJ];
        btrieve(B_GETEQUAL, TMS_SIGNCODES, &SIGNCODES, &SIGNCODESKey0, 0);
        strncpy(tempString, SIGNCODES.code, SIGNCODES_CODE_LENGTH);
        trim(tempString, SIGNCODES_CODE_LENGTH);
        strncpy(szarString, SIGNCODES.text, SIGNCODES_TEXT_LENGTH);
        trim(szarString, SIGNCODES_TEXT_LENGTH);
        sprintf(outputString, "%ld\t%s\t%s\t%ld\t\"  %s : %s\"\r\n",
              seq++, divisionName, serviceName, blockNumber, tempString, szarString);
        _lwrite(hfOutputFile, outputString, strlen(outputString));
        numLines++;
      }
    }
  }  // nI loop on list of blocks
//
//  Free allocated memory
//
  deallocate:
    TMSHeapFree(REPORTPARMS.pRouteList);
    TMSHeapFree(REPORTPARMS.pServiceList);
    TMSHeapFree(REPORTPARMS.pDivisionList);
    TMSHeapFree(REPORTPARMS.pBlockList);
    StatusBarEnd();
    _lclose(hfOutputFile);
    SetCursor(LoadCursor(NULL, IDC_ARROW));
    if(!bKeepGoing)
    {
      return(FALSE);
    }
//
//  All done
//
  return(TRUE);
}

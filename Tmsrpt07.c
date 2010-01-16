//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
//
//  TMSRPT07() - SmartCard Unload
//
#include "TMSHeader.h"
#include "cistms.h"
//
//  Record Layout:
//
//     Col   Description
//     ~~~   ~~~~~~~~~~~
//     1-5   Run number (front-padded with zeros)
//     6-9   Node Abbreviation
//    10-13  Time of day (Military, rolls over after 2359)
//    14-21  Route Number
//    22-23  Direction Abbreviation
//    24-39  Service Name
//    40-45  Trip Number
//

BOOL FAR TMSRPT07(TMSRPTPassedDataDef *pPassedData)
{
  GenerateTripDef GTResults;
  REPORTPARMSDef REPORTPARMS;
  HFILE hfOutputFile;
  BOOL  bGotFirstNode;
  BOOL  bKeepGoing = FALSE;
  BOOL  bRC;
  char  directionAbbr[DIRECTIONS_ABBRNAME_LENGTH + 1];
  char  routeNumber[ROUTES_NUMBER_LENGTH + 1];
  char  nodeAbbr[NODES_ABBRNAME_LENGTH + 1];
  char  prevNodeAbbr[NODES_ABBRNAME_LENGTH + 1];
  char  serviceName[SERVICES_NAME_LENGTH + 1];
  long  NODESrecordIDs[RECORDIDS_KEPT];
  long  runNumber;
  long  tripRecordID;
  long  tripSequence;
  long  timeFormatSave;
  int   nI;
  int   nJ;
  int   nK;
  int   numNodes;
  int   rcode2;


  pPassedData->nReportNumber = 6;
  pPassedData->numDataFiles = 1;
//
//  See what he wants
//
  REPORTPARMS.nReportNumber = pPassedData->nReportNumber;
  REPORTPARMS.SERVICESrecordID = m_ServiceRecordID;
  REPORTPARMS.DIVISIONSrecordID = m_DivisionRecordID;
  REPORTPARMS.pServiceList = NULL;
  REPORTPARMS.pDivisionList = NULL;
  REPORTPARMS.pRunList = NULL;
  REPORTPARMS.flags = RPFLAG_SERVICES | RPFLAG_DIVISIONS |
                      RPFLAG_NOALLDIVISIONS | RPFLAG_RUNS;
  bRC = DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_RPTPARMS),
        hWndMain, (DLGPROC)RPTPARMSMsgProc, (LPARAM)&REPORTPARMS);
  if(!bRC)
  {
    return(FALSE);
  }
//
//  Build the list of services
//
  if(REPORTPARMS.numServices == 0 || REPORTPARMS.numRuns == 0)
  {
    goto deallocate;
  }
//
//  Open the output file
//
  strcpy(tempString, szReportsTempFolder);
  strcat(tempString, "\\tmsrpt07.txt");
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
//  Loop through the services
//
  LoadString(hInst, TEXT_119, tempString, TEMPSTRING_LENGTH);
  StatusBarStart(hWndMain, tempString);
  bKeepGoing = TRUE;
  for(nI = 0; nI < REPORTPARMS.numServices; nI++)
  {
    SERVICESKey0.recordID = REPORTPARMS.pServiceList[nI];
    rcode2 = btrieve(B_GETEQUAL, TMS_SERVICES, &SERVICES, &SERVICESKey0, 0);
    if(rcode2 != 0)
      continue;
//
//  Initialize the travel matrix
//
    StatusBarEnd();
    if(bUseDynamicTravels)
    {
      if(bUseCISPlan)
      {
        CISfree();
        if(!CISbuildService(FALSE, TRUE, SERVICES.recordID))
          goto deallocate;
      }
      else
      {
        LoadString(hInst, TEXT_135, tempString, TEMPSTRING_LENGTH);
        StatusBarText(tempString);
        InitTravelMatrix(SERVICES.recordID, FALSE);
      }
    }
//
//  Get the service name
//
    strncpy(serviceName, SERVICES.name, SERVICES_NAME_LENGTH);
    trim(serviceName, SERVICES_NAME_LENGTH);
    m_bEstablishRUNTIMES = TRUE;
//
//  Loop through the runs
//
    StatusBarStart(hWndMain, "Cycling through the runs...");
    for(nJ = 0; nJ < REPORTPARMS.numRuns; nJ++)
    {
      RUNSKey0.recordID = REPORTPARMS.pRunList[nJ];
      btrieve(B_GETEQUAL, TMS_RUNS, &RUNS, &RUNSKey0, 0);
      btrieve(B_GETPOSITION, TMS_RUNS, &RUNS, &RUNSKey0, 0);
      rcode2 = btrieve(B_GETDIRECT, TMS_RUNS, &RUNS, &RUNSKey1, 1);
      runNumber = RUNS.runNumber;
      strcpy(prevNodeAbbr, "");
      LoadString(hInst, TEXT_120, szarString, sizeof(szarString));
      sprintf(tempString, szarString, serviceName, RUNS.runNumber);
      StatusBarText(tempString);
      StatusBar((long)(nJ + 1), (long)REPORTPARMS.numRuns);
      if(StatusBarAbort())
      {
        bKeepGoing = FALSE;
        break;
      }
      while(rcode2 == 0 &&
            RUNS.DIVISIONSrecordID == REPORTPARMS.pDivisionList[0] &&
            RUNS.SERVICESrecordID == REPORTPARMS.pServiceList[nI] &&
            RUNS.runNumber == runNumber)
      {
        TRIPSKey0.recordID = RUNS.start.TRIPSrecordID;
        btrieve(B_GETEQUAL, TMS_TRIPS, &TRIPS, &TRIPSKey0, 0);
        btrieve(B_GETPOSITION, TMS_TRIPS, &TRIPS, &TRIPSKey0, 0);
        rcode2 = btrieve(B_GETDIRECT, TMS_TRIPS, &TRIPS, &TRIPSKey2, 2);
        bGotFirstNode = TRIPS.standard.POGNODESrecordID != NO_RECORD;
        while(rcode2 == 0)
        {
//
//  Determine where the trip would sit on a trips table.
//  What an absolute crock of shit this is, but we got
//  Burlington Transit because we could fulfill this
//  idiotic requirement, so who am I to complain?
//
          tripRecordID = TRIPS.recordID;
          TRIPSKey1.ROUTESrecordID = TRIPS.ROUTESrecordID;
          TRIPSKey1.SERVICESrecordID = TRIPS.SERVICESrecordID;
          TRIPSKey1.directionIndex = TRIPS.directionIndex;
          TRIPSKey1.tripSequence = NO_TIME;
          rcode2 = btrieve(B_GETGREATER, TMS_TRIPS, &TRIPS, &TRIPSKey1, 1);
          tripSequence = 1;
          while(rcode2 == 0)
          {
            if(TRIPS.recordID == tripRecordID)
              break;
            tripSequence++;
            rcode2 = btrieve(B_GETNEXT, TMS_TRIPS, &TRIPS, &TRIPSKey1, 1);
          }
          TRIPSKey0.recordID = tripRecordID;
          btrieve(B_GETEQUAL, TMS_TRIPS, &TRIPS, &TRIPSKey0, 0);
          btrieve(B_GETPOSITION, TMS_TRIPS, &TRIPS, &TRIPSKey0, 0);
          btrieve(B_GETDIRECT, TMS_TRIPS, &TRIPS, &TRIPSKey2, 2);
//
//  Generate the trip
//
          GenerateTrip(TRIPS.ROUTESrecordID, TRIPS.SERVICESrecordID,
                TRIPS.directionIndex, TRIPS.PATTERNNAMESrecordID,
                  TRIPS.timeAtMLP, GENERATETRIP_FLAG_DISPLAYERRORS, &GTResults);
//
//  Get the trip pattern nodes
//
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
              NODESrecordIDs[numNodes++] = PATTERNS.NODESrecordID;
            rcode2 = btrieve(B_GETNEXT, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
          }
//
//  Get the route number of the trip
//
          ROUTESKey0.recordID = TRIPS.ROUTESrecordID;
          btrieve(B_GETEQUAL, TMS_ROUTES, &ROUTES, &ROUTESKey0, 0);
          strncpy(routeNumber, ROUTES.number, ROUTES_NUMBER_LENGTH);
          trim(routeNumber, ROUTES_NUMBER_LENGTH);
//
//  Get the direction abbreviation of the trip
//
          DIRECTIONSKey0.recordID = ROUTES.DIRECTIONSrecordID[TRIPS.directionIndex];
          btrieve(B_GETEQUAL, TMS_DIRECTIONS, &DIRECTIONS, &DIRECTIONSKey0, 0);
          strncpy(directionAbbr, DIRECTIONS.abbrName, DIRECTIONS_ABBRNAME_LENGTH);
          trim(directionAbbr, DIRECTIONS_ABBRNAME_LENGTH);
//
//  Move along the pattern and write the output line
//
//     1-5   Run number (front-padded with zeros)
//     6-9   Node Abbreviation
//    10-13  Time of day (Military, rolls over after 2359)
//    14-21  Route Number
//    22-23  Direction Abbreviation
//    24-39  Service Name
//    40-45  Trip Number (front-padded with zeros)
//
          timeFormatSave = timeFormat;
          timeFormat = PREFERENCES_MILITARY;
          for(nK = 0; nK < numNodes; nK++)
          {
            if(bGotFirstNode || NODESrecordIDs[nK] == RUNS.start.NODESrecordID)
            {
              bGotFirstNode = TRUE;
              if(GTResults.tripTimes[nK] > 86400L)
                GTResults.tripTimes[nK] -= 86400L;
              NODESKey0.recordID = NODESrecordIDs[nK];
              btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
              strncpy(nodeAbbr, NODES.abbrName, NODES_ABBRNAME_LENGTH);
              trim(nodeAbbr, NODES_ABBRNAME_LENGTH);
              sprintf(tempString, "%05ld,\"%4s\",\"%s\",\"%8s\",\"%2s\",\"%16s\",%06ld\r\n",
                    RUNS.runNumber,
                    nodeAbbr,
                    Tchar(GTResults.tripTimes[nK]),
                    routeNumber,
                    directionAbbr,
                    serviceName,
                    tripSequence);
              if(strcmp(prevNodeAbbr, nodeAbbr) == 0)
                _llseek(hfOutputFile, (long)(-1L * strlen(tempString)), 1);
              _lwrite(hfOutputFile, tempString, strlen(tempString));
              strcpy(prevNodeAbbr, nodeAbbr);
              if(TRIPS.recordID == RUNS.end.TRIPSrecordID &&
                      NODES.recordID == RUNS.end.NODESrecordID)
                break;
            }
          }
          timeFormat = timeFormatSave;
          if(TRIPS.recordID == RUNS.end.TRIPSrecordID)
            break;
          rcode2 = btrieve(B_GETNEXT, TMS_TRIPS, &TRIPS, &TRIPSKey2, 2);
        }  // while
        rcode2 = btrieve(B_GETNEXT, TMS_RUNS, &RUNS, &RUNSKey1, 1);
      }  // while
    }  // nJ
    if(!bKeepGoing)
      break;
  }  // nI
//
//  Free allocated memory
//
  deallocate:
    TMSHeapFree(REPORTPARMS.pServiceList);
    TMSHeapFree(REPORTPARMS.pDivisionList);
    TMSHeapFree(REPORTPARMS.pRunList);
    StatusBarEnd();
    _lclose(hfOutputFile);
    SetCursor(hCursorArrow);
    if(!bKeepGoing)
    {
      return(FALSE);
    }
//
//  All done
//
  return(TRUE);
}


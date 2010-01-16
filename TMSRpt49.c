//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
#include "TMSHeader.h"

//
//  TMSRPT49() - Download to MCT Tripcard System
//
//  MCT-Tripcards.TXT (Tab Delimited)
//
//  Trip number
//  Block number
//  Route name
//  Start run number
//  End run number
//  Trip first stop time
//  Trip last stop time
//

BOOL FAR TMSRPT49(TMSRPTPassedDataDef *pPassedData)
{
  GenerateTripDef GTResults;
  REPORTPARMSDef REPORTPARMS;
  HFILE hfOutputFile;
  BOOL  bGotOne;
  BOOL  bKeepGoing = FALSE;
  BOOL  bRC;
  char  outputString[512];
  char  routeNumberAndName[ROUTES_NUMBER_LENGTH + 3 + ROUTES_NAME_LENGTH + 1];
  char  serviceName[SERVICES_NAME_LENGTH + 1];
  char  divisionName[DIVISIONS_NAME_LENGTH + 1];
  char  directionName[DIRECTIONS_LONGNAME_LENGTH + 1];
  long  runNumber[2];
  long  reliefNode;
  char  *ptr;
  int   nI;
  int   nJ;
  int   nK;
  int   nL;
  int   nM;
  int   rcode2;
  int   seq;

  pPassedData->nReportNumber = 48;
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
  REPORTPARMS.flags = RPFLAG_ROUTES | RPFLAG_COMMENTS | RPFLAG_SERVICES | RPFLAG_DIVISIONS;
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
//  Open the output file
//
  strcpy(tempString, szDatabaseFileName);
  if((ptr = strrchr(tempString, '\\')) != NULL)
  {
    *ptr = '\0';
  }
  strcat(tempString, "\\MCT-Tripcards.txt");
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
//  Fire up the status bar
//
  LoadString(hInst, TEXT_117, tempString, TEMPSTRING_LENGTH);
  StatusBarStart(hWndMain, tempString);
//
//  Loop through all the divisions
//
  bKeepGoing = TRUE;
  seq = 0;
  for(nI = 0; nI < REPORTPARMS.numDivisions; nI++)
  {
    DIVISIONSKey0.recordID = REPORTPARMS.pDivisionList[nI];
    btrieve(B_GETEQUAL, TMS_DIVISIONS, &DIVISIONS, &DIVISIONSKey0, 0);
    strncpy(divisionName, DIVISIONS.name, DIVISIONS_NAME_LENGTH);
    trim(divisionName, DIVISIONS_NAME_LENGTH);
//
//  Loop through all the services
//
    for(nJ = 0; nJ < REPORTPARMS.numServices; nJ++)
    {
      SERVICESKey0.recordID = REPORTPARMS.pServiceList[nJ];
      btrieve(B_GETEQUAL, TMS_SERVICES, &SERVICES, &SERVICESKey0, 0);
      strncpy(serviceName, SERVICES.name, SERVICES_NAME_LENGTH);
      trim(serviceName, SERVICES_NAME_LENGTH);
//
//  Get all the runs for this division and service
//
      GetRunRecords(REPORTPARMS.pDivisionList[nI], REPORTPARMS.pServiceList[nJ]);
      if(m_numRunRecords == NO_RECORD)
      {
        bKeepGoing = FALSE;
        goto deallocate;
      }
//
//  Loop through all the routes
//
      for(nK = 0; nK < REPORTPARMS.numRoutes; nK++)
      {
//
//  Loop through the directions
//
        for(nL = 0; nL < 2; nL++)
        {
          ROUTESKey0.recordID = REPORTPARMS.pRouteList[nK];
          btrieve(B_GETEQUAL, TMS_ROUTES, &ROUTES, &ROUTESKey0, 0);
          if(ROUTES.DIRECTIONSrecordID[nL] == NO_RECORD)
            continue;
          strncpy(routeNumberAndName, ROUTES.number, ROUTES_NUMBER_LENGTH);
          trim(routeNumberAndName, ROUTES_NUMBER_LENGTH);
          strcat(routeNumberAndName, " - ");
          strncpy(tempString, ROUTES.name, ROUTES_NAME_LENGTH);
          trim(tempString, ROUTES_NAME_LENGTH);
          strcat(routeNumberAndName, tempString);
          DIRECTIONSKey0.recordID = ROUTES.DIRECTIONSrecordID[nL];
          btrieve(B_GETEQUAL, TMS_DIRECTIONS, &DIRECTIONS, &DIRECTIONSKey0, 0);
          strncpy(directionName, DIRECTIONS.longName, DIRECTIONS_LONGNAME_LENGTH);
          trim(directionName, DIRECTIONS_LONGNAME_LENGTH);
//
//  Go through all the trips for this route, service, and direction
//
          TRIPSKey1.ROUTESrecordID = REPORTPARMS.pRouteList[nK];
          TRIPSKey1.SERVICESrecordID = REPORTPARMS.pServiceList[nJ];
          TRIPSKey1.directionIndex = nL;
          TRIPSKey1.tripSequence = NO_RECORD;
          rcode2 = btrieve(B_GETGREATER, TMS_TRIPS, &TRIPS, &TRIPSKey1, 1);
          while(rcode2 == 0 &&
                TRIPS.ROUTESrecordID == REPORTPARMS.pRouteList[nK] &&
                TRIPS.SERVICESrecordID == REPORTPARMS.pServiceList[nJ] &&
                TRIPS.directionIndex == nL)
          {
//
//  Generate the trip
//
            GenerateTrip(TRIPS.ROUTESrecordID, TRIPS.SERVICESrecordID,
                  TRIPS.directionIndex, TRIPS.PATTERNNAMESrecordID,
                  TRIPS.timeAtMLP, GENERATETRIP_FLAG_DISPLAYERRORS, &GTResults);
//
//  Get the run number(s) on the trip
//
            runNumber[0] = 0;
            runNumber[1] = 0;
            for(bGotOne = FALSE, nM = 0; nM < m_numRunRecords; nM++)
            {
              if(bGotOne && m_pRunRecordData[nM].blockNumber != TRIPS.standard.blockNumber)
                break;
              if(m_pRunRecordData[nM].startTRIPSrecordID == TRIPS.recordID ||
                    (m_pRunRecordData[nM].blockNumber == TRIPS.standard.blockNumber &&
                     GTResults.firstNodeTime >= m_pRunRecordData[nM].startTime &&
                     GTResults.firstNodeTime <= m_pRunRecordData[nM].endTime))
              {
                if(bGotOne)
                {
                  runNumber[1] = m_pRunRecordData[nM].runNumber;
                  reliefNode = m_pRunRecordData[nM].startNODESrecordID;
                  break;
                }
                else
                {
                  runNumber[0] = m_pRunRecordData[nM].runNumber;
                  runNumber[1] = m_pRunRecordData[nM].runNumber;
                  reliefNode = NO_RECORD;
                  bGotOne = TRUE;
                }
              }
            }
//
//  Build the output string and write it out
//
            sprintf(outputString, "%ld\t%ld\t%s\t%ld\t%ld\t%ld\t%ld\r\n",
                  TRIPS.tripNumber, TRIPS.standard.blockNumber, routeNumberAndName,
                  runNumber[0], runNumber[1], GTResults.firstNodeTime, GTResults.lastNodeTime);
            _lwrite(hfOutputFile, outputString, strlen(outputString));
//
//  Get the next trip
//
            rcode2 = btrieve(B_GETNEXT, TMS_TRIPS, &TRIPS, &TRIPSKey1, 1);
          }  // while TRIPS loop
        }  //  nL loop on directions
      }  //  nK loop on routes
    }  // nJ loop on services
  }  // nI loop on divisions
  bKeepGoing = TRUE;
//
//  Free allocated memory
//
  deallocate:
    TMSHeapFree(REPORTPARMS.pRouteList);
    TMSHeapFree(REPORTPARMS.pServiceList);
    TMSHeapFree(REPORTPARMS.pDivisionList);
    StatusBarEnd();
    _lclose(hfOutputFile);
    SetCursor(hCursorArrow);
    if(!bKeepGoing)
    {
      return(FALSE);
    }
//
//  Let him know
//
    strcpy(tempString, "MCT-Tripcards.txt was written to:\n");
    strcpy(szarString, szDatabaseFileName);
    if((ptr = strrchr(tempString, '\\')) != NULL)
    {
      *ptr = '\0';
    }
    strcat(tempString, szarString);
    MessageBox(hWndMain, tempString, TMS, MB_OK);
//
//  All done
//
  return(TRUE);
}

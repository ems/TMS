//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
#include "TMSHeader.h"

//
//  TMSRPT25() - Download to Teleride Telerider system
//
//  This report simulates the Trapeze headway sheet, and is output in this
//  format because the dickheads at Teleride (may they RIP) wouldn't provide
//  a cleaner interface structure.
//
BOOL FAR TMSRPT25(TMSRPTPassedDataDef *pPassedData)
{
  GenerateTripDef GTResults;
  REPORTPARMSDef REPORTPARMS;
  time_t ltime;
  HFILE hfOutputFile;
  BOOL  bKeepGoing = FALSE;
  BOOL  bRC;
  char  outputString[512];
  char  routeNumberAndName[ROUTES_NUMBER_LENGTH + 3 + ROUTES_NAME_LENGTH + 1];
  char  serviceName[SERVICES_NAME_LENGTH + 1];
  char  directionName[DIRECTIONS_LONGNAME_LENGTH + 1];
  char  szDate[32];
  char  *ptr;
  long  BASEPatternNodeList[RECORDIDS_KEPT];
  long  subPatternNodeList[RECORDIDS_KEPT];
  long  statbarTotal;
  int   nI;
  int   nJ;
  int   nK;
  int   nL;
  int   nM;
  int   numNodes;
  int   numBASENodes;
  int   rcode2;
  int   currentRoute;
  int   currentService;

  pPassedData->nReportNumber = 24;
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
  REPORTPARMS.flags = RPFLAG_ROUTES | RPFLAG_COMMENTS | RPFLAG_SERVICES;
  bRC = DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_RPTPARMS),
        hWndMain, (DLGPROC)RPTPARMSMsgProc, (LPARAM)&REPORTPARMS);
  if(!bRC)
  {
    return(FALSE);
  }
//
//  Check the list of routes, services, and divisions
//
  if(REPORTPARMS.numRoutes == 0 || REPORTPARMS.numServices == 0)
  {
    goto deallocate;
  }
//
//  Open the output file
//
  strcpy(tempString, szDatabaseFileName);
  if((ptr = strrchr(tempString, '\\')) != NULL)
    *ptr = '\0';
  strcat(tempString, "\\TMSRPT25.txt");
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
  statbarTotal = REPORTPARMS.numRoutes * REPORTPARMS.numServices;
//
//  Get the time
//
//  From the help file:
//
//  The ctime function converts a time value stored as a time_t structure into a
//  character string. The timer value is usually obtained from a call to time,
//  which returns the number of seconds elapsed since midnight (00:00:00),
//  January 1, 1970, coordinated universal time (UTC). The string result produced
//  by ctime contains exactly 26 characters and has the form:
//
//  Wed Jan 02 02:03:55 1980\n\0
//
//  A 24-hour clock is used. All fields have a constant width. The newline character
//  ('\n') and the null character ('\0') occupy the last two positions of the string.
//
  time(&ltime);
  strcpy(szDate, ctime(&ltime));
  szDate[24] = '\0';  // We don't want the newline character
//
//  Loop through all the services
//
  currentService = 0;
  for(nI = 0; nI < REPORTPARMS.numServices; nI++)
  {
    if(StatusBarAbort())
    {
      bKeepGoing = FALSE;
      goto deallocate;
    }
    SERVICESKey0.recordID = REPORTPARMS.pServiceList[nI];
    btrieve(B_GETEQUAL, TMS_SERVICES, &SERVICES, &SERVICESKey0, 0);
    strncpy(serviceName, SERVICES.name, SERVICES_NAME_LENGTH);
    trim(serviceName, SERVICES_NAME_LENGTH);
//
//  Loop through all the routes
//
    currentRoute = 1;
    for(nJ = 0; nJ < REPORTPARMS.numRoutes; nJ++)
    {
      if(StatusBarAbort())
      {
        bKeepGoing = FALSE;
        goto deallocate;
      }
      ROUTESKey0.recordID = REPORTPARMS.pRouteList[nJ];
      btrieve(B_GETEQUAL, TMS_ROUTES, &ROUTES, &ROUTESKey0, 0);
      strncpy(routeNumberAndName, ROUTES.number, ROUTES_NUMBER_LENGTH);
      trim(routeNumberAndName, ROUTES_NUMBER_LENGTH);
      strcat(routeNumberAndName, " - ");
      strncpy(tempString, ROUTES.name, ROUTES_NAME_LENGTH);
      trim(tempString, ROUTES_NAME_LENGTH);
      strcat(routeNumberAndName, tempString);
      strcpy(szarString, routeNumberAndName);
      strcat(szarString, "\n");
      strcat(szarString, serviceName);
      StatusBarText(szarString);
      StatusBar((long)(currentService * (REPORTPARMS.numRoutes - 1) + currentRoute), (long)statbarTotal);
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
          continue;
//
//  There has to be at least one trip
//
        TRIPSKey1.ROUTESrecordID = REPORTPARMS.pRouteList[nJ];
        TRIPSKey1.SERVICESrecordID = REPORTPARMS.pServiceList[nI];
        TRIPSKey1.directionIndex = nK;
        TRIPSKey1.tripSequence = NO_RECORD;
        rcode2 = btrieve(B_GETGREATER, TMS_TRIPS, &TRIPS, &TRIPSKey1, 1);
        if(rcode2 != 0 ||
              TRIPS.ROUTESrecordID != REPORTPARMS.pRouteList[nJ] ||
              TRIPS.SERVICESrecordID != REPORTPARMS.pServiceList[nI] ||
              TRIPS.directionIndex != nK)
          continue;
//
//  Get the direction information
//
        DIRECTIONSKey0.recordID = ROUTES.DIRECTIONSrecordID[nK];
        btrieve(B_GETEQUAL, TMS_DIRECTIONS, &DIRECTIONS, &DIRECTIONSKey0, 0);
        strncpy(directionName, DIRECTIONS.longName, DIRECTIONS_LONGNAME_LENGTH);
        trim(directionName, DIRECTIONS_LONGNAME_LENGTH);
//
//  Print out the header
//
//  Line 1
//
        sprintf(outputString, " Trips Report               Printed: %s\r\n", szDate);
        _lwrite(hfOutputFile, outputString, strlen(outputString));
//
//  Line 2
//
        sprintf(outputString, " Database  :  TMS              User: TMS\r\n");
        _lwrite(hfOutputFile, outputString, strlen(outputString));
//
//  Line 3
//
        strncpy(tempString, ROUTES.number, ROUTES_NUMBER_LENGTH);
        tempString[5] = '\0';
        strncpy(szarString, ROUTES.name, ROUTES_NAME_LENGTH);
        trim(szarString, ROUTES_NAME_LENGTH);
        _strupr(szarString);
        sprintf(outputString, " Line      :  %s : %s\r\n", tempString, szarString);
        _lwrite(hfOutputFile, outputString, strlen(outputString));
//
//  Line 4
//
        if(SERVICES.number == 1)
        {
          strcpy(tempString, "WKDAY");
          strcpy(szarString, "MONDAY TO FRIDAY");
        }
        else if(SERVICES.number == 2)
        {
          strcpy(tempString, "SAT  ");
          strcpy(szarString, "SATURDAY");
        }
        else
        {
          strcpy(tempString, "SUN  ");
          strcpy(szarString, "SUNDAY");
        }
        sprintf(outputString, " Service   :  %s : %s\r\n", tempString, szarString);
        _lwrite(hfOutputFile, outputString, strlen(outputString));
//
//  Line 5 (gets an extra \r\n 'cause line 6 is blank)
//
        strncpy(tempString, DIRECTIONS.longName, DIRECTIONS_LONGNAME_LENGTH);
        trim(tempString, DIRECTIONS_LONGNAME_LENGTH);
        _strupr(tempString);
        sprintf(outputString, " Direction :  D%d    : %s\r\n\r\n", nK, tempString); 
        _lwrite(hfOutputFile, outputString, strlen(outputString));
//
//  Line 7 
//  
        strcpy(outputString, " Pat     ");
//
//  Get the BASE pattern nodes for this route, service, and direction
//
        PATTERNSKey2.ROUTESrecordID = REPORTPARMS.pRouteList[nJ];
        PATTERNSKey2.SERVICESrecordID = REPORTPARMS.pServiceList[nI];
        PATTERNSKey2.directionIndex = nK;
        PATTERNSKey2.PATTERNNAMESrecordID = basePatternRecordID;
        PATTERNSKey2.nodeSequence = 0;
        numBASENodes = 0;
        rcode2 = btrieve(B_GETGREATEROREQUAL, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
        while(rcode2 == 0 &&
              PATTERNS.ROUTESrecordID == REPORTPARMS.pRouteList[nJ] &&
              PATTERNS.SERVICESrecordID == REPORTPARMS.pServiceList[nI] &&
              PATTERNS.directionIndex == nK &&
              PATTERNS.PATTERNNAMESrecordID == basePatternRecordID)
        {
          if(!(PATTERNS.flags & PATTERNS_FLAG_BUSSTOP))
            BASEPatternNodeList[numBASENodes++] = PATTERNS.NODESrecordID;
          rcode2 = btrieve(B_GETNEXT, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
        }  // while rcode2 == 0 on the pattern
//
//  Concatenate the abbreviated names to line 7
//
        for(nL = 0; nL < numBASENodes; nL++)
        {
          NODESKey0.recordID = BASEPatternNodeList[nL];
          btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
          strncpy(tempString, NODES.abbrName, NODES_ABBRNAME_LENGTH);
          tempString[NODES_ABBRNAME_LENGTH] = '\0';
          strcat(outputString, tempString);
          strcat(outputString, "   ");
        }
//
//  And the last part of line 7.  Line 8 is blank, so 7 gets a double \r\n
//
        strcat(outputString, "Tsort   Ltime   BlkId\r\n\r\n");
        _lwrite(hfOutputFile, outputString, strlen(outputString));
//
//  Go through all the trips for this route, service, and direction
//
        TRIPSKey1.ROUTESrecordID = REPORTPARMS.pRouteList[nJ];
        TRIPSKey1.SERVICESrecordID = REPORTPARMS.pServiceList[nI];
        TRIPSKey1.directionIndex = nK;
        TRIPSKey1.tripSequence = NO_RECORD;
        rcode2 = btrieve(B_GETGREATER, TMS_TRIPS, &TRIPS, &TRIPSKey1, 1);
        while(rcode2 == 0 &&
              TRIPS.ROUTESrecordID == REPORTPARMS.pRouteList[nJ] &&
              TRIPS.SERVICESrecordID == REPORTPARMS.pServiceList[nI] &&
              TRIPS.directionIndex == nK)
        {
//
//  Lines 9 through n-1 have the trip times
//
          PATTERNNAMESKey0.recordID = TRIPS.PATTERNNAMESrecordID;
          btrieve(B_GETEQUAL, TMS_PATTERNNAMES, &PATTERNNAMES, &PATTERNNAMESKey0, 0);
          strncpy(tempString, PATTERNNAMES.name, PATTERNNAMES_NAME_LENGTH);
          tempString[8] = '\0';
          strcpy(outputString, " ");
          strcat(outputString, tempString);
//  
//  Generate the trip
//
          GenerateTrip(TRIPS.ROUTESrecordID, TRIPS.SERVICESrecordID,
                TRIPS.directionIndex, TRIPS.PATTERNNAMESrecordID,
                TRIPS.timeAtMLP, GENERATETRIP_FLAG_DISPLAYERRORS, &GTResults);
//
//  Get the trip times
//
//  BASE Pattern
//
          if(TRIPS.PATTERNNAMESrecordID == basePatternRecordID)
          {
            for(nL = 0; nL < numBASENodes; nL++)
            {
              strcat(outputString, Tchar(GTResults.tripTimes[nL]));
              strcat(outputString, "   ");
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
            nM = 0;
            for(nL = 0; nL < numBASENodes; nL++)
            {
              if(nM >= numNodes)
                break;
              if(BASEPatternNodeList[nL] == subPatternNodeList[nM])
              {
                strcat(outputString, Tchar(GTResults.tripTimes[nM]));
                nM++;
              }
              else
              {
                strcat(outputString, "    ");
              }
              strcat(outputString, "   ");
            }
//
//  Fill in some blanks if the last pattern node
//  isn't the same as the last BASE pattern node
//
            if(subPatternNodeList[numNodes - 1] != BASEPatternNodeList[numBASENodes - 1])
            {
              nM = 1;
              for(nL = numBASENodes - 2; nL >= 0 ; nL--)
              {
                if(subPatternNodeList[numNodes - 1] == BASEPatternNodeList[nL])
                  break;
                nM++;
              }
              for(nL = 0; nL < nM; nL++)
              {
                strcat(outputString, "       ");
              }
            }
          }
//
//  The last part of the line has the trip sort field,
//  the time at the last node, and the block number
//
          strcat(outputString, Tchar(TRIPS.tripSequence));
          strcat(outputString, "    ");
          strcat(outputString, Tchar(GTResults.lastNodeTime));
          strcat(outputString, "    ");
          sprintf(tempString, "%ld\r\n", TRIPS.standard.blockNumber);
          strcat(outputString, tempString);
          _lwrite(hfOutputFile, outputString, strlen(outputString));
//
//  Get the next trip
//
          rcode2 = btrieve(B_GETNEXT, TMS_TRIPS, &TRIPS, &TRIPSKey1, 1);
        }  // while TRIPS loop
//
//  After every route/ser/dir, there's a single line with x'0C' on it
//
        strcpy(outputString, "\13\r\n");
        _lwrite(hfOutputFile, outputString, strlen(outputString));
      }  //  nK loop on directions
      currentRoute++;
    }  //  nJ loop on routes
    currentService++;
  }  // nI loop on services
  bKeepGoing = TRUE;
//
//  Free allocated memory
//
  deallocate:
    TMSHeapFree(REPORTPARMS.pRouteList);
    TMSHeapFree(REPORTPARMS.pServiceList);
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
//
//  Let him know
//
  strcpy(tempString, "TMSRpt25.txt was written to:\n");
  strcpy(szarString, szDatabaseFileName);
  if((ptr = strrchr(szarString, '\\')) != NULL)
    *ptr = '\0';
  strcat(tempString, szarString);
  MessageBox(hWndMain, tempString, TMS, MB_OK);

  return(TRUE);
}

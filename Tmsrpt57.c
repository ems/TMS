//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
#include "TMSHeader.h"
#include "cistms.h"

EXTERN char *TcharAMPMAM(long);
//
//  HTML Download (2)
//
BOOL FAR TMSRPT57(TMSRPTPassedDataDef *pPassedData)
{
  GenerateTripDef GTResults;
  REPORTPARMSDef REPORTPARMS;
  HFILE hfOutputFile;
  BOOL  bRC;
  BOOL  bFinishedOK;
  BOOL  bDoSubset;
  BOOL  bFound;
  long  currentService;
  long  currentRoute;
  long  MLPIndex;
  long  statbarTotal;
  char  outputString[1024];
  char  *ptr;
  char  directionName[DIRECTIONS_LONGNAME_LENGTH + 1];
  char  routeNumber[ROUTES_NUMBER_LENGTH + 1];
  char  routeName[ROUTES_NAME_LENGTH + 1];
  char  serviceName[SERVICES_NAME_LENGTH + 1];
  int   nI;
  int   nJ;
  int   nK;
  int   nL;
  int   nM;
  int   numSelectedPatternSeqs;
  int   numTripPatternSeqs;
  int   numBASEPatternNODESrecordIDs;
  int   maxSelectedPatternSeqs = 50;
  int   maxTripPatternSeqs = 50;
  int   maxBASEPatternNODESrecordIDs = 50;
  int   rcode2;
  int   pos;
  int   rowCount;

  long  *pSelectedPatternSeqs = NULL;
  long  *pTripPatternSeqs = NULL;
  long  *pBASEPatternNODESrecordIDs = NULL;

  pPassedData->nReportNumber = 27;
//
//  See what he wants to unload
//
  bFinishedOK = FALSE;
//
//  See what he wants
//
  REPORTPARMS.nReportNumber = pPassedData->nReportNumber;
  REPORTPARMS.ROUTESrecordID = m_RouteRecordID;
  REPORTPARMS.SERVICESrecordID = m_ServiceRecordID;
  REPORTPARMS.PATTERNNAMESrecordID = NO_RECORD;
  REPORTPARMS.COMMENTSrecordID = NO_RECORD;
  REPORTPARMS.pRouteList = NULL;
  REPORTPARMS.pServiceList = NULL;
  REPORTPARMS.flags = RPFLAG_ROUTES | RPFLAG_SERVICES | RPFLAG_PATTERNNAMES | RPFLAG_COMMENTS;
  bRC = DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_RPTPARMS),
        hWndMain, (DLGPROC)RPTPARMSMsgProc, (LPARAM)&REPORTPARMS);
  if(!bRC)
    return(FALSE);
//
//  Build the list of routes and services
//
  if(REPORTPARMS.numRoutes == 0 || REPORTPARMS.numServices == 0)
    goto done;
//
//  Fire up the status bar
//
  for(nI = 0; nI < m_LastReport; nI++)
  {
    if(TMSRPT[nI].originalReportNumber == pPassedData->nReportNumber)
    {
      StatusBarStart(hWndMain, TMSRPT[nI].szReportName);
      break;
    }
  }
//
//  Allocate the trip/pattern nodes saved arrays
//
  pSelectedPatternSeqs = (long *)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(long) * maxSelectedPatternSeqs); 
  if(pSelectedPatternSeqs == NULL)
  {
    AllocationError(__FILE__, __LINE__, FALSE);
    goto done;
  }

  pTripPatternSeqs = (long *)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(long) * maxTripPatternSeqs); 
  if(pTripPatternSeqs == NULL)
  {
    AllocationError(__FILE__, __LINE__, FALSE);
    goto done;
  }

  pBASEPatternNODESrecordIDs = (long *)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(long) * maxBASEPatternNODESrecordIDs); 
  if(pBASEPatternNODESrecordIDs == NULL)
  {
    AllocationError(__FILE__, __LINE__, FALSE);
    goto done;
  }
//
//  Loop through all the services
//
  LoadString(hInst, TEXT_257, tempString, TEMPSTRING_LENGTH);
  StatusBarText(tempString);
  statbarTotal = REPORTPARMS.numRoutes * REPORTPARMS.numServices;
  currentService = 0;
  for(nI = 0; nI < REPORTPARMS.numServices; nI++)
  {
    if(StatusBarAbort())
    {
      goto done;
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
        goto done;
      }
      ROUTESKey0.recordID = REPORTPARMS.pRouteList[nJ];
      btrieve(B_GETEQUAL, TMS_ROUTES, &ROUTES, &ROUTESKey0, 0);
      strncpy(routeNumber, ROUTES.number, ROUTES_NUMBER_LENGTH);
      trim(routeNumber, ROUTES_NUMBER_LENGTH);
      strncpy(routeName, ROUTES.name, ROUTES_NAME_LENGTH);
      trim(routeName, ROUTES_NAME_LENGTH);
//
//  Loop through all the directions
//
      for(nK = 0; nK < 2; nK++)
      {
        if(StatusBarAbort())
        {
          goto done;
        }
        if(ROUTES.DIRECTIONSrecordID[nK] == NO_RECORD)
        {
          continue;
        }
//
//  There has to be at least one trip on the route
//
        TRIPSKey1.ROUTESrecordID = REPORTPARMS.pRouteList[nJ];
        TRIPSKey1.SERVICESrecordID = REPORTPARMS.pServiceList[nI];
        TRIPSKey1.directionIndex = nK;
        TRIPSKey1.tripSequence = NO_TIME;
        rcode2 = btrieve(B_GETGREATEROREQUAL, TMS_TRIPS, &TRIPS, &TRIPSKey1, 1);
        if(rcode2 != 0 ||
              TRIPS.ROUTESrecordID != REPORTPARMS.pRouteList[nJ] ||
              TRIPS.SERVICESrecordID != REPORTPARMS.pServiceList[nI] ||
              TRIPS.directionIndex != nK)
        {
          continue;
        }
//
//  Get the direction informations
//
        DIRECTIONSKey0.recordID = ROUTES.DIRECTIONSrecordID[nK];
        btrieve(B_GETEQUAL, TMS_DIRECTIONS, &DIRECTIONS, &DIRECTIONSKey0, 0);
        strncpy(directionName, DIRECTIONS.longName, DIRECTIONS_LONGNAME_LENGTH);
        trim(directionName, DIRECTIONS_LONGNAME_LENGTH);
//
//  Show the status
//
        strcpy(szarString, routeNumber);
        strcat(szarString, " - ");
        strcat(szarString, routeName);
        strcat(szarString, "\n");
        strcat(szarString, directionName);
        strcat(szarString, " - ");
        strcat(szarString, serviceName);
        StatusBarText(szarString);
        StatusBar((long)(currentService * (REPORTPARMS.numRoutes - 1) + currentRoute), (long)statbarTotal);
//
//  Get the BASE pattern for this route, service, and direction
//
//  If there isn't one, cycle through 
//
        PATTERNSKey2.ROUTESrecordID = REPORTPARMS.pRouteList[nJ];
        PATTERNSKey2.SERVICESrecordID = REPORTPARMS.pServiceList[nI];
        PATTERNSKey2.directionIndex = nK;
        PATTERNSKey2.PATTERNNAMESrecordID = basePatternRecordID;
        PATTERNSKey2.nodeSequence = NO_RECORD;
        rcode2 = btrieve(B_GETGREATER, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
        if(rcode2 != 0 ||
              PATTERNS.ROUTESrecordID != REPORTPARMS.pRouteList[nJ] ||
              PATTERNS.SERVICESrecordID != REPORTPARMS.pServiceList[nI] ||
              PATTERNS.directionIndex != nK ||
              PATTERNS.PATTERNNAMESrecordID != basePatternRecordID)
        {
          continue;
        }
//
//  Open the output file
//
        strcpy(tempString, szDatabaseFileName);
        if((ptr = strrchr(tempString, '\\')) != NULL)
        {
          *ptr = '\0';
        }
        strcat(tempString, "\\");
        strcat(tempString, serviceName);
        strcat(tempString, " ");
        strcat(tempString, routeNumber);
        strcat(tempString, " ");
        strcat(tempString, directionName);
        strcat(tempString, ".html");
        hfOutputFile = _lcreat(tempString, 0);
        if(hfOutputFile == HFILE_ERROR)
        {
          LoadString(hInst, ERROR_202, szFormatString, sizeof(szFormatString));
          sprintf(szarString, szFormatString, tempString);
          MessageBeep(MB_ICONSTOP);
          MessageBox((HWND)NULL, szarString, TMS, MB_ICONSTOP);
          bFinishedOK = FALSE;
          goto done;
        }
//
//  Passed the tests
//
//  See if the selected pattern exists on this route, service, and direction
//
        bDoSubset = FALSE;
        if(REPORTPARMS.PATTERNNAMESrecordID != basePatternRecordID)
        {
          PATTERNSKey2.ROUTESrecordID = REPORTPARMS.pRouteList[nJ];
          PATTERNSKey2.SERVICESrecordID = REPORTPARMS.pServiceList[nI];
          PATTERNSKey2.directionIndex = nK;
          PATTERNSKey2.PATTERNNAMESrecordID = REPORTPARMS.PATTERNNAMESrecordID;
          PATTERNSKey2.nodeSequence = NO_RECORD;
          rcode2 = btrieve(B_GETGREATER, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
          if(rcode2 == 0 &&
                PATTERNS.ROUTESrecordID == REPORTPARMS.pRouteList[nJ] &&
                PATTERNS.SERVICESrecordID == REPORTPARMS.pServiceList[nI] &&
                PATTERNS.directionIndex == nK &&
                PATTERNS.PATTERNNAMESrecordID == REPORTPARMS.PATTERNNAMESrecordID)
          {
            bDoSubset = TRUE;
            numSelectedPatternSeqs = 0;
            numBASEPatternNODESrecordIDs = 0;
            while(rcode2 == 0 &&
                  PATTERNS.ROUTESrecordID == REPORTPARMS.pRouteList[nJ] &&
                  PATTERNS.SERVICESrecordID == REPORTPARMS.pServiceList[nI] &&
                  PATTERNS.directionIndex == nK &&
                  PATTERNS.PATTERNNAMESrecordID == REPORTPARMS.PATTERNNAMESrecordID)
            {
              if(!(PATTERNS.flags & PATTERNS_FLAG_BUSSTOP))
              {
                if(PATTERNS.flags & PATTERNS_FLAG_MLP)
                {
                  MLPIndex = numSelectedPatternSeqs;
                }
                pBASEPatternNODESrecordIDs[numBASEPatternNODESrecordIDs++] = PATTERNS.NODESrecordID;
                pSelectedPatternSeqs[numSelectedPatternSeqs++] = PATTERNS.nodeSequence;
                if(numSelectedPatternSeqs >= maxSelectedPatternSeqs)
                {
                  maxSelectedPatternSeqs += 50;
                  pSelectedPatternSeqs = (long *)HeapReAlloc(GetProcessHeap(),
                        HEAP_ZERO_MEMORY, pSelectedPatternSeqs, sizeof(long) * maxSelectedPatternSeqs); 
                  if(pSelectedPatternSeqs == NULL)
                  {
                    AllocationError(__FILE__, __LINE__, TRUE);
                    goto done;
                  }
                  maxBASEPatternNODESrecordIDs += 50;
                  pBASEPatternNODESrecordIDs = (long *)HeapReAlloc(GetProcessHeap(),
                        HEAP_ZERO_MEMORY, pBASEPatternNODESrecordIDs, sizeof(long) * maxBASEPatternNODESrecordIDs); 
                  if(pBASEPatternNODESrecordIDs == NULL)
                  {
                    AllocationError(__FILE__, __LINE__, TRUE);
                    goto done;
                  }
                }
              }
              rcode2 = btrieve(B_GETNEXT, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
            }
          }
        }
//
//  If we're not doing a subset, set up pSelectedPatternSeqs
//  to fool the code into believing we are.
//
        if(!bDoSubset)
        {
          numSelectedPatternSeqs = 0;
          numBASEPatternNODESrecordIDs = 0;
          PATTERNSKey2.ROUTESrecordID = REPORTPARMS.pRouteList[nJ];
          PATTERNSKey2.SERVICESrecordID = REPORTPARMS.pServiceList[nI];
          PATTERNSKey2.directionIndex = nK;
          PATTERNSKey2.PATTERNNAMESrecordID = basePatternRecordID;
          PATTERNSKey2.nodeSequence = NO_RECORD;
          rcode2 = btrieve(B_GETGREATER, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
          while(rcode2 == 0 &&
                PATTERNS.ROUTESrecordID == REPORTPARMS.pRouteList[nJ] &&
                PATTERNS.SERVICESrecordID == REPORTPARMS.pServiceList[nI] &&
                PATTERNS.directionIndex == nK &&
                PATTERNS.PATTERNNAMESrecordID == basePatternRecordID)
          {
            if(!(PATTERNS.flags & PATTERNS_FLAG_BUSSTOP))
            {
              if(PATTERNS.flags & PATTERNS_FLAG_MLP)
              {
                MLPIndex = numSelectedPatternSeqs;
              }
              pBASEPatternNODESrecordIDs[numBASEPatternNODESrecordIDs++] = PATTERNS.NODESrecordID;
              pSelectedPatternSeqs[numSelectedPatternSeqs++] = PATTERNS.nodeSequence;
              if(numSelectedPatternSeqs >= maxSelectedPatternSeqs)
              {
                maxSelectedPatternSeqs += 50;
                pSelectedPatternSeqs = (long *)HeapReAlloc(GetProcessHeap(),
                      HEAP_ZERO_MEMORY, pSelectedPatternSeqs, sizeof(long) * maxSelectedPatternSeqs); 
                if(pSelectedPatternSeqs == NULL)
                {
                  AllocationError(__FILE__, __LINE__, TRUE);
                  goto done;
                }
                maxBASEPatternNODESrecordIDs += 50;
                pBASEPatternNODESrecordIDs = (long *)HeapReAlloc(GetProcessHeap(),
                      HEAP_ZERO_MEMORY, pBASEPatternNODESrecordIDs, sizeof(long) * maxBASEPatternNODESrecordIDs); 
                if(pBASEPatternNODESrecordIDs == NULL)
                {
                  AllocationError(__FILE__, __LINE__, TRUE);
                  goto done;
                }
              }
            }
            rcode2 = btrieve(B_GETNEXT, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
          }
        }
//
//  Set up the first portion of the output file
//
        for(nL = HTML_001; nL <= HTML_006; nL++)
        {
          LoadString(hInst, nL, outputString, sizeof(outputString));
          strcat(outputString, "\r\n");
          _lwrite(hfOutputFile, outputString, strlen(outputString));
        }
//
//  Title
//
        LoadString(hInst, HTML_007, szFormatString, sizeof(szFormatString));
        sprintf(outputString, szFormatString, serviceName, routeNumber, routeName, directionName);
        strcat(outputString, "\r\n");
        _lwrite(hfOutputFile, outputString, strlen(outputString));
//
//  Meta tags and the start of the body
//
        for(nL = HTML_008; nL <= HTML_013; nL++)
        {
          LoadString(hInst, nL, outputString, sizeof(outputString));
          strcat(outputString, "\r\n");
          _lwrite(hfOutputFile, outputString, strlen(outputString));
        }
//
//  Route
//
        LoadString(hInst, HTML_037, szFormatString, sizeof(szFormatString));
        sprintf(outputString, szFormatString, routeNumber, routeName);
        strcat(outputString, "\r\n");
        _lwrite(hfOutputFile, outputString, strlen(outputString));
//
//  Service/Direction
//
        LoadString(hInst, HTML_038, szFormatString, sizeof(szFormatString));
        sprintf(outputString, szFormatString, serviceName, directionName);
        strcat(outputString, "\r\n");
        _lwrite(hfOutputFile, outputString, strlen(outputString));
//
//  Verbiage
//
        LoadString(hInst, HTML_039, outputString, sizeof(outputString));
        strcat(outputString, "\r\n");
        _lwrite(hfOutputFile, outputString, strlen(outputString));
        LoadString(hInst, HTML_040, outputString, sizeof(outputString));
        strcat(outputString, "\r\n");
        _lwrite(hfOutputFile, outputString, strlen(outputString));
//
//  Table header
//
        LoadString(hInst, HTML_018, outputString, sizeof(outputString));
        strcat(outputString, "\r\n");
        _lwrite(hfOutputFile, outputString, strlen(outputString));
//
//  Column attributes
//
        LoadString(hInst, HTML_041, outputString, sizeof(outputString));
        strcat(outputString, "\r\n");
        for(nL = 0; nL < numSelectedPatternSeqs + 1; nL++)
        {
          _lwrite(hfOutputFile, outputString, strlen(outputString));
        }
//
//  Column style
//
        LoadString(hInst, HTML_042, outputString, sizeof(outputString));
        strcat(outputString, "\r\n");
        _lwrite(hfOutputFile, outputString, strlen(outputString));
//
//  "Row Number" + Node names
//
        for(nL = 0; nL < numSelectedPatternSeqs + 1; nL++)
        {
          LoadString(hInst, HTML_043, outputString, sizeof(outputString));
          strcat(outputString, "\r\n");
          _lwrite(hfOutputFile, outputString, strlen(outputString));  // Width
          if(nL == 0)
          {
            strcpy(tempString, "Row Number");
          }
          else
          {
            NODESKey0.recordID = pBASEPatternNODESrecordIDs[nL - 1];
            btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
            strncpy(tempString, NODES.description, NODES_DESCRIPTION_LENGTH);
            trim(tempString, NODES_DESCRIPTION_LENGTH);
            ptr = strstr(tempString, "&&");
            if(ptr)
            {
              *ptr = 'a';
              *(ptr + 1) = 't';
            }
          }
          LoadString(hInst, HTML_044, szFormatString, sizeof(szFormatString));
          sprintf(outputString, szFormatString, tempString);
          strcat(outputString, "\r\n");
          _lwrite(hfOutputFile, outputString, strlen(outputString));
          LoadString(hInst, HTML_045, outputString, sizeof(outputString));
          strcat(outputString, "\r\n");
          _lwrite(hfOutputFile, outputString, strlen(outputString));  // Width
        }
        LoadString(hInst, HTML_025, outputString, sizeof(outputString));  // </tr>
        strcat(outputString, "\r\n");
        _lwrite(hfOutputFile, outputString, strlen(outputString));
//
//  Loop through all the trips
//
        rowCount = 0;
        m_bEstablishRUNTIMES = TRUE;
        TRIPSKey1.ROUTESrecordID = REPORTPARMS.pRouteList[nJ];
        TRIPSKey1.SERVICESrecordID = REPORTPARMS.pServiceList[nI];
        TRIPSKey1.directionIndex = nK;
        TRIPSKey1.tripSequence = NO_TIME;
        rcode2 = btrieve(B_GETGREATEROREQUAL, TMS_TRIPS, &TRIPS, &TRIPSKey1, 1);
        while(rcode2 == 0 &&
              TRIPS.ROUTESrecordID == REPORTPARMS.pRouteList[nJ] &&
              TRIPS.SERVICESrecordID == REPORTPARMS.pServiceList[nI] &&
              TRIPS.directionIndex == nK)
        {
//
//  <tr>
//
          LoadString(hInst, HTML_033, outputString, sizeof(outputString));
          strcat(outputString, "\r\n");
          _lwrite(hfOutputFile, outputString, strlen(outputString));
//
//  Find the TRIP pattern sequences
//
          numTripPatternSeqs = 0;
          PATTERNSKey2.ROUTESrecordID = TRIPS.ROUTESrecordID;
          PATTERNSKey2.SERVICESrecordID = TRIPS.SERVICESrecordID;
          PATTERNSKey2.directionIndex = TRIPS.directionIndex;
          PATTERNSKey2.PATTERNNAMESrecordID = TRIPS.PATTERNNAMESrecordID;
          PATTERNSKey2.nodeSequence = NO_RECORD;
          rcode2 = btrieve(B_GETGREATEROREQUAL, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
          while(rcode2 == 0 &&
                PATTERNS.ROUTESrecordID == TRIPS.ROUTESrecordID &&
                PATTERNS.SERVICESrecordID == TRIPS.SERVICESrecordID &&
                PATTERNS.directionIndex == TRIPS.directionIndex &&
                PATTERNS.PATTERNNAMESrecordID == TRIPS.PATTERNNAMESrecordID)
          {
            if(!(PATTERNS.flags & PATTERNS_FLAG_BUSSTOP))
            {
              pTripPatternSeqs[numTripPatternSeqs++] = PATTERNS.nodeSequence;
              if(numTripPatternSeqs >= maxTripPatternSeqs)
              {
                maxTripPatternSeqs += 50;
                pTripPatternSeqs = (long *)HeapReAlloc(GetProcessHeap(),
                      HEAP_ZERO_MEMORY, pTripPatternSeqs, sizeof(long) * maxTripPatternSeqs); 
                if(pTripPatternSeqs == NULL)
                {
                  AllocationError(__FILE__, __LINE__, FALSE);
                  goto done;
                }
              }
            }
            rcode2 = btrieve(B_GETNEXT, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
          }
//
//  Generate the trip
//
          GenerateTrip(TRIPS.ROUTESrecordID, TRIPS.SERVICESrecordID,
                TRIPS.directionIndex, TRIPS.PATTERNNAMESrecordID,
                TRIPS.timeAtMLP, GENERATETRIP_FLAG_DISPLAYERRORS, &GTResults);
//
//  Row number
//
          LoadString(hInst, HTML_046, outputString, sizeof(outputString));  // </tr>
          strcat(outputString, "\r\n");
          _lwrite(hfOutputFile, outputString, strlen(outputString));
          LoadString(hInst, HTML_048, outputString, sizeof(outputString));  // </tr>
          strcat(outputString, "\r\n");
          _lwrite(hfOutputFile, outputString, strlen(outputString));
          LoadString(hInst, HTML_050, szFormatString, sizeof(szFormatString));
          sprintf(outputString, szFormatString, itoa(++rowCount, szarString, 10));
          strcat(outputString, "\r\n");
          _lwrite(hfOutputFile, outputString, strlen(outputString));
//
//  Trip times
//
//  We only want those times that exist on the selected pattern
//
          pos = 0;
          for(nL = 0; nL < numSelectedPatternSeqs; nL++)
          {
            for(bFound = FALSE, nM = pos; nM < numTripPatternSeqs; nM++)
            {
              if(pSelectedPatternSeqs[nL] == pTripPatternSeqs[nM])
              {
                bFound = TRUE;
                pTripPatternSeqs[nM] = NO_RECORD;
                pos = nM + 1;
                break;
              }
            }
            if(bFound)
            {
              strcpy(tempString, TcharAMPMAM(GTResults.tripTimes[nM]));
            }
            else
            {
              strcpy(tempString, "--");
            }
            LoadString(hInst, HTML_047, outputString, sizeof(outputString));  // </tr>
            strcat(outputString, "\r\n");
            _lwrite(hfOutputFile, outputString, strlen(outputString));
            LoadString(hInst, HTML_049, outputString, sizeof(outputString));  // </tr>
            strcat(outputString, "\r\n");
            _lwrite(hfOutputFile, outputString, strlen(outputString));
            LoadString(hInst, HTML_051, szFormatString, sizeof(szFormatString));
            sprintf(outputString, szFormatString, tempString);
            strcat(outputString, "\r\n");
            _lwrite(hfOutputFile, outputString, strlen(outputString));
          }
//
//  End the table row
//
          LoadString(hInst, HTML_025, outputString, sizeof(outputString));  // </tr>
          strcat(outputString, "\r\n");
          _lwrite(hfOutputFile, outputString, strlen(outputString));

//
//  Get the next trip
//
          rcode2 = btrieve(B_GETNEXT, TMS_TRIPS, &TRIPS, &TRIPSKey1, 1);
        }  // while
//
//  Table done
//
        LoadString(hInst, HTML_026, outputString, sizeof(outputString));  // </tr>
        strcat(outputString, "\r\n");
        _lwrite(hfOutputFile, outputString, strlen(outputString));
//
//  Close the file and loop back
//
        _lclose(hfOutputFile);
      }  // nK
      currentRoute++;
    }  //  nJ loop on routes
    currentService++;
  }  // nI loop on services
  StatusBar(-1L, -1L);
//
//  All done
//
  bFinishedOK = TRUE;
  done:
    TMSHeapFree(REPORTPARMS.pRouteList);
    TMSHeapFree(REPORTPARMS.pServiceList);
    TMSHeapFree(pSelectedPatternSeqs);
    TMSHeapFree(pTripPatternSeqs);
    TMSHeapFree(pBASEPatternNODESrecordIDs);
    _lclose(hfOutputFile);
    StatusBarEnd();

  if(!bFinishedOK)
  {
    TMSError((HWND)NULL, MB_ICONINFORMATION, ERROR_227, (HANDLE)NULL);
  }
  else
  {
    LoadString(hInst, TEXT_258, szFormatString, sizeof(szFormatString));
    strcpy(szarString, szDatabaseFileName);
    if((ptr = strrchr(szarString, '\\')) != NULL)
    {
      *ptr = '\0';
    }
    sprintf(tempString, szFormatString, szarString);
    MessageBox(hWndMain, tempString, TMS, MB_OK);
  }

  return(bFinishedOK);
}

//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
#include "TMSHeader.h"
#include "cistms.h"

//
//  HTML Download (1)
//
BOOL FAR TMSRPT28(TMSRPTPassedDataDef *pPassedData)
{
  GenerateTripDef GTResults;
  REPORTPARMSDef REPORTPARMS;
  HFILE hfOutputFile;
  BOOL  bRC;
  BOOL  bFinishedOK;
  BOOL  bGotWebGraphic;
  BOOL  bGotWheelchair;
  BOOL  bDoSubset;
  BOOL  bFound;
  BOOL  bAllAccessible;
  long  currentService;
  long  currentRoute;
  long  MLPIndex;
  long  statbarTotal;
  char  outputString[1024];
  char  *ptr;
  char  szWebGraphic[256];
  char  szWheelchair[256];
  char  directionName[DIRECTIONS_LONGNAME_LENGTH + 1];
  char  routeNumber[ROUTES_NUMBER_LENGTH + 1];
  char  routeName[ROUTES_NAME_LENGTH + 1];
  char  serviceName[SERVICES_NAME_LENGTH + 1];
  char  commentCode[COMMENTS_CODE_LENGTH + 1];
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
  int   maxCommentsSaved = 50;
  int   rcode2;
  int   pos;
  int   webGraphicHeight;
  int   webGraphicWidth;
  int   wheelchairHeight;
  int   wheelchairWidth;
  int   colNotesWidth;
  int   colNodesWidth;
  int   colWheelchairWidth;
  int   numCommentsSaved;

  long  *pSelectedPatternSeqs = NULL;
  long  *pTripPatternSeqs = NULL;
  long  *pBASEPatternNODESrecordIDs = NULL;
  long  *pCommentsSaved = NULL;

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
  {
    return(FALSE);
  }
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
//  Allocate the trip/pattern nodes/comments saved arrays
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
 
  pCommentsSaved = (long *)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(long) * maxCommentsSaved); 
  if(pCommentsSaved == NULL)
  {
    AllocationError(__FILE__, __LINE__, FALSE);
    goto done;
  }
//
//  Get the filename of the graphic for the pages
//
  GetPrivateProfileString("HTML", "WebGraphic", "",
        szWebGraphic, sizeof(szWebGraphic), TMSINIFile);
  bGotWebGraphic = (strcmp(szWebGraphic, "") != 0);
  if(bGotWebGraphic)
  {
    webGraphicHeight = GetPrivateProfileInt("HTML", "WebGraphicHeight", NO_RECORD, TMSINIFile);
    webGraphicWidth = GetPrivateProfileInt("HTML", "WebGraphicWidth", NO_RECORD, TMSINIFile);
  }
//
//  Get the filename of the wheelchair graphic
//
  GetPrivateProfileString("HTML", "Wheelchair", "", szWheelchair, sizeof(szWheelchair), TMSINIFile);
  bGotWheelchair = (strcmp(szWheelchair, "") != 0);
  if(bGotWheelchair)
  {
    wheelchairHeight = GetPrivateProfileInt("HTML", "WheelchairHeight", NO_RECORD, TMSINIFile);
    wheelchairWidth = GetPrivateProfileInt("HTML", "WheelchairWidth", NO_RECORD, TMSINIFile);
  }
//
//  Get the output table column widths
//
  colNotesWidth = GetPrivateProfileInt("HTML", "ColNotesWidth", NO_RECORD, TMSINIFile);
  colWheelchairWidth = GetPrivateProfileInt("HTML", "ColWheelchairWidth", NO_RECORD, TMSINIFile);
  colNodesWidth = GetPrivateProfileInt("HTML", "ColNodesWidth", NO_RECORD, TMSINIFile);
//
//  Is everything accessible?
//
  GetPrivateProfileString("HTML", "AllAccessible", "FALSE", tempString, TEMPSTRING_LENGTH, TMSINIFile);
  bAllAccessible = (strcmp(tempString, szTRUE) == 0 ? TRUE : FALSE);
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
//  If theres a graphic, put it in
//
        if(bGotWebGraphic)
        {
          LoadString(hInst, HTML_014, szFormatString, sizeof(szFormatString));
          sprintf(outputString, szFormatString, szWebGraphic,
                webGraphicWidth,  webGraphicHeight, webGraphicWidth, webGraphicHeight);
          strcat(outputString, "\r\n");
          _lwrite(hfOutputFile, outputString, strlen(outputString));
        }
//
//  Service and Route
//
        LoadString(hInst, HTML_015, szFormatString, sizeof(szFormatString));
        sprintf(outputString, szFormatString, serviceName, routeNumber);
        strcat(outputString, "\r\n");
        _lwrite(hfOutputFile, outputString, strlen(outputString));
        LoadString(hInst, HTML_016, szFormatString, sizeof(szFormatString));
        sprintf(outputString, szFormatString, routeName, directionName);
        strcat(outputString, "\r\n");
        _lwrite(hfOutputFile, outputString, strlen(outputString));
//
//  Effective date
//
        if(strcmp(szEffectiveDate, "") != 0)
        {
          LoadString(hInst, HTML_017, szFormatString, sizeof(szFormatString));
          sprintf(outputString, szFormatString, szEffectiveDate);
          strcat(outputString, "\r\n");
          _lwrite(hfOutputFile, outputString, strlen(outputString));
        }
//
//  Table header
//
        LoadString(hInst, HTML_018, outputString, sizeof(outputString));
        strcat(outputString, "\r\n");
        _lwrite(hfOutputFile, outputString, strlen(outputString));
//
//  Column attributes
//
        LoadString(hInst, HTML_019, szFormatString, sizeof(szFormatString));
        sprintf(outputString, szFormatString, colNotesWidth);
        strcat(outputString, "\r\n");
        _lwrite(hfOutputFile, outputString, strlen(outputString));
        sprintf(outputString, szFormatString, colWheelchairWidth);
        strcat(outputString, "\r\n");
        _lwrite(hfOutputFile, outputString, strlen(outputString));
        sprintf(outputString, szFormatString, colNodesWidth);
        strcat(outputString, "\r\n");
        for(nL = 0; nL < numSelectedPatternSeqs; nL++)
        {
          _lwrite(hfOutputFile, outputString, strlen(outputString));
        }
//
//  Column style
//
        LoadString(hInst, HTML_020, outputString, sizeof(outputString));
        strcat(outputString, "\r\n");
        _lwrite(hfOutputFile, outputString, strlen(outputString));
//
//  Headers
//
//  Notes
//
        LoadString(hInst, HTML_021, szFormatString, sizeof(szFormatString));
        sprintf(outputString, szFormatString, colNotesWidth);
        strcat(outputString, "\r\n");
        _lwrite(hfOutputFile, outputString, strlen(outputString));
        LoadString(hInst, HTML_022, szFormatString, sizeof(szFormatString));
        sprintf(outputString, szFormatString, "Notes");
        strcat(outputString, "\r\n");
        _lwrite(hfOutputFile, outputString, strlen(outputString));
//
//  Wheelchair symbol column
//
        LoadString(hInst, HTML_023, szFormatString, sizeof(szFormatString));
        sprintf(outputString, szFormatString, colWheelchairWidth);
        strcat(outputString, "\r\n");
        _lwrite(hfOutputFile, outputString, strlen(outputString));
        LoadString(hInst, HTML_024, szFormatString, sizeof(szFormatString));
        sprintf(outputString, szFormatString, "&nbsp;");
        strcat(outputString, "\r\n");
        _lwrite(hfOutputFile, outputString, strlen(outputString));
//
//  Node names
//
        LoadString(hInst, HTML_021, szFormatString, sizeof(szFormatString));
        sprintf(szarString, szFormatString, colNodesWidth);
        strcat(szarString, "\r\n");
        LoadString(hInst, HTML_022, szFormatString, sizeof(szFormatString));
        for(nL = 0; nL < numSelectedPatternSeqs; nL++)
        {
          _lwrite(hfOutputFile, szarString, strlen(szarString));  // Width
          NODESKey0.recordID = pBASEPatternNODESrecordIDs[nL];
          btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
          strncpy(tempString, NODES.description, NODES_DESCRIPTION_LENGTH);
          trim(tempString, NODES_DESCRIPTION_LENGTH);
          ptr = strstr(tempString, "&&");
          if(ptr)
          {
            *ptr = 'a';
            *(ptr + 1) = 't';
          }
          sprintf(outputString, szFormatString, tempString);
          strcat(outputString, "\r\n");
          _lwrite(hfOutputFile, outputString, strlen(outputString));
        }
        LoadString(hInst, HTML_025, outputString, sizeof(outputString));  // </tr>
        strcat(outputString, "\r\n");
        _lwrite(hfOutputFile, outputString, strlen(outputString));
//
//  Loop through all the trips
//
        numCommentsSaved = 0;
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
//  Customer comment code
//
          LoadString(hInst, HTML_023, szFormatString, sizeof(szFormatString));
          sprintf(outputString, szFormatString, colNotesWidth);
          strcat(outputString, "\r\n");
          _lwrite(hfOutputFile, outputString, strlen(outputString));
          LoadString(hInst, HTML_024, szFormatString, sizeof(szFormatString));
          if(TRIPS.customerCOMMENTSrecordID == NO_RECORD ||
                TRIPS.customerCOMMENTSrecordID == 0)  // Backward compatibility
          {
            sprintf(outputString, szFormatString, "&nbsp;");
          }
          else
          {
            COMMENTSKey0.recordID = TRIPS.customerCOMMENTSrecordID;
            btrieve(B_GETEQUAL, TMS_COMMENTS, &COMMENTS, &COMMENTSKey0, 0);
            strncpy(tempString, COMMENTS.code, COMMENTS_CODE_LENGTH);
            trim(tempString, COMMENTS_CODE_LENGTH);
            sprintf(outputString, szFormatString, tempString);
            for(bFound = FALSE, nL = 0; nL < numCommentsSaved; nL++)
            {
              if(TRIPS.customerCOMMENTSrecordID == pCommentsSaved[nL])
              {
                bFound = TRUE;
                break;
              }
            }
            if(!bFound)
            {
              pCommentsSaved[numCommentsSaved++] = TRIPS.customerCOMMENTSrecordID;
              if(numCommentsSaved >= maxCommentsSaved)
              {
                maxCommentsSaved += 50;
                pCommentsSaved = (long *)HeapReAlloc(GetProcessHeap(),
                      HEAP_ZERO_MEMORY, pCommentsSaved, sizeof(long) * maxCommentsSaved); 
                if(pCommentsSaved == NULL)
                {
                  AllocationError(__FILE__, __LINE__, FALSE);
                  goto done;
                }
              }
            }
          }
          strcat(outputString, "\r\n");
          _lwrite(hfOutputFile, outputString, strlen(outputString));
//
//  Vehicle type (Accessible flag, wheelchair graphic)
//
          LoadString(hInst, HTML_023, szFormatString, sizeof(szFormatString));
          sprintf(outputString, szFormatString, colWheelchairWidth);
          strcat(outputString, "\r\n");
          _lwrite(hfOutputFile, outputString, strlen(outputString));
          if(!bAllAccessible && TRIPS.BUSTYPESrecordID == NO_RECORD)
          {
            LoadString(hInst, HTML_024, szFormatString, sizeof(szFormatString));
            sprintf(outputString, szFormatString, "&nbsp;");
          }
          else
          {
            BOOL bAccessible = bAllAccessible;

            if(!bAllAccessible)
            {
              BUSTYPESKey0.recordID = TRIPS.BUSTYPESrecordID;
              btrieve(B_GETEQUAL, TMS_BUSTYPES, &BUSTYPES, &BUSTYPESKey0, 0);
              bAccessible = BUSTYPES.flags & BUSTYPES_FLAG_ACCESSIBLE;
            }
            if(bAccessible)
            {
              LoadString(hInst, HTML_034, szFormatString, sizeof(szFormatString));
              sprintf(outputString, szFormatString, szWheelchair, wheelchairWidth,
                    wheelchairHeight, wheelchairWidth, wheelchairHeight);
            }
            else
            {
              LoadString(hInst, HTML_024, szFormatString, sizeof(szFormatString));
              sprintf(outputString, szFormatString, "&nbsp;");
            }
          }
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
//  Trip times
//
//  We only want those times that exist on the selected pattern
//
          pos = 0;
          for(nL = 0; nL < numSelectedPatternSeqs; nL++)
          {
            LoadString(hInst, HTML_035, szFormatString, sizeof(szFormatString));
            sprintf(outputString, szFormatString, colNodesWidth);
            strcat(outputString, "\r\n");
            _lwrite(hfOutputFile, outputString, strlen(outputString));
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
              strcpy(tempString, Tchar(GTResults.tripTimes[nM]));
            }
            else
            {
              strcpy(tempString, "&nbsp;");
            }
            LoadString(hInst, HTML_036, szFormatString, sizeof(szFormatString));
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
//  Were there any notes?
//
        if(numCommentsSaved > 0)
        {
          LoadString(hInst, HTML_027, outputString, sizeof(outputString));  // </tr>
          strcat(outputString, "\r\n");
          _lwrite(hfOutputFile, outputString, strlen(outputString));
          LoadString(hInst, HTML_028, szFormatString, sizeof(szFormatString));
          for(nL = 0; nL < numCommentsSaved; nL++)
          {
            recordLength[TMS_COMMENTS] = COMMENTS_TOTAL_LENGTH;
            COMMENTSKey0.recordID = pCommentsSaved[nL];
            btrieve(B_GETEQUAL, TMS_COMMENTS, pCommentText, &COMMENTSKey0, 0);
            recordLength[TMS_COMMENTS] = COMMENTS_FIXED_LENGTH;
            memcpy(&COMMENTS, pCommentText, COMMENTS_FIXED_LENGTH);
            strncpy(commentCode, COMMENTS.code, COMMENTS_CODE_LENGTH);
            trim(commentCode, COMMENTS_CODE_LENGTH);
            sprintf(outputString, szFormatString, commentCode, &pCommentText[COMMENTS_FIXED_LENGTH]);
            strcat(outputString, "\r\n");
            _lwrite(hfOutputFile, outputString, strlen(outputString));
          }
        }
//
//  Standing text
//
        LoadString(hInst, HTML_029, outputString, sizeof(outputString));
        strcat(outputString, "\r\n");
        _lwrite(hfOutputFile, outputString, strlen(outputString));
        _tzset();
        _strdate(tempString);
        LoadString(hInst, HTML_030, szFormatString, sizeof(szFormatString));
        sprintf(outputString, szFormatString, tempString);
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
//  Schedule Data
//
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
    TMSHeapFree(pCommentsSaved);
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

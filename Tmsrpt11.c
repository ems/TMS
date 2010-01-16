//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
#include "TMSHeader.h"

BOOL CALLBACK DATAPUBLISHMsgProc(HWND, UINT, WPARAM, LPARAM);

//
//  TMS Data Publish - Interface to Minischeduler "Publish" Files
//
BOOL FAR TMSRPT11(TMSRPTPassedDataDef *pPassedData)
{
  RPTFLAGSDef RPTFLAGS;
  HFILE hfOutputFile[5];
  BOOL  bRC;
  char  outputString[1024];
  char  *ptr;
  int   nI;
  int   nJ;
  int   nK;
  int   rcode2;
  int   routeNumber;

//
//  See what he wants to unload
//
  pPassedData->nReportNumber = 10;
  RPTFLAGS.nReportNumber = pPassedData->nReportNumber;
  bRC = DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_DATAPUBLISH),
        hWndMain, (DLGPROC)DATAPUBLISHMsgProc, (LPARAM)&RPTFLAGS);
  if(!bRC)
  {
    return(FALSE);
  }
  for(nI = 0; nI < m_LastReport; nI++)
  {
    if(TMSRPT[nI].originalReportNumber == pPassedData->nReportNumber)
    {
      StatusBarStart(hWndMain, TMSRPT[nI].szReportName);
      break;
    }
  }
//
//  Directions
//
  if(RPTFLAGS.flags & TMSRPT11_DIRECTIONS)
  {
    char abbrName[max(DIRECTIONS_ABBRNAME_LENGTH, 2) + 1];
    char longName[max(DIRECTIONS_LONGNAME_LENGTH, 8) + 1];

    LoadString(hInst, TEXT_124, tempString, TEMPSTRING_LENGTH);
    StatusBarText(tempString);
//
//  Open the output file
//
    strcpy(tempString, szDatabaseFileName);
    if((ptr = strrchr(tempString, '\\')) != NULL)
      *ptr = '\0';
    strcat(tempString, "\\pubdir.dat");
    hfOutputFile[0] = _lcreat(tempString, 0);
    if(hfOutputFile[0] == HFILE_ERROR)
    {
      LoadString(hInst, ERROR_202, szFormatString, sizeof(szFormatString));
      sprintf(szarString, szFormatString, tempString);
      MessageBeep(MB_ICONSTOP);
      MessageBox((HWND)NULL, szarString, TMS, MB_ICONSTOP);
      return(FALSE);
    }
//
//  Directions file layout
//
//  Bytes   Description
//  ~~~~~   ~~~~~~~~~~~
//   1-5    Route number, right justified
//    6     Direction number (0 or 1)
//   7-8    Direction abbreviation
//   9-14   Blank
//  15-22   Full direction name (truncated, if necessary)
//
    rcode2 = btrieve(B_GETFIRST, TMS_ROUTES, &ROUTES, &ROUTESKey1, 1);
    while(rcode2 == 0)
    {
      for(nI = 0; nI < 2; nI++)
      {
        if(ROUTES.DIRECTIONSrecordID[nI] == NO_RECORD)
          continue;
        DIRECTIONSKey0.recordID = ROUTES.DIRECTIONSrecordID[nI];
        btrieve(B_GETEQUAL, TMS_DIRECTIONS, &DIRECTIONS, &DIRECTIONSKey0, 0);
        routeNumber = atoi(ROUTES.number);
        memset(abbrName, ' ', sizeof(abbrName));
        strncpy(abbrName, DIRECTIONS.abbrName, DIRECTIONS_ABBRNAME_LENGTH);
        abbrName[2] = '\0';
        memset(longName, ' ', sizeof(longName));
        strncpy(longName, DIRECTIONS.longName, DIRECTIONS_LONGNAME_LENGTH);
        longName[8] = '\0';
        memset(szarString, ' ', 6);
        szarString[6] = '\0';
        sprintf(outputString, "%5d%1d%s%s%s\n", routeNumber, nI, abbrName, szarString, longName);
        _lwrite(hfOutputFile[0], outputString, strlen(outputString));
      }
      rcode2 = btrieve(B_GETNEXT, TMS_ROUTES, &ROUTES, &ROUTESKey1, 1);
    }
    _lclose(hfOutputFile[0]);
  }
//
//  Routes
//
  if(RPTFLAGS.flags & TMSRPT11_ROUTES)
  {
    char longName[max(ROUTES_NAME_LENGTH, 32) + 1];

    LoadString(hInst, TEXT_125, tempString, TEMPSTRING_LENGTH);
    StatusBarText(tempString);
//
//  Open the output file
//
    strcpy(tempString, szDatabaseFileName);
    if((ptr = strrchr(tempString, '\\')) != NULL)
      *ptr = '\0';
    strcat(tempString, "\\publin.dat");
    hfOutputFile[0] = _lcreat(tempString, 0);
    if(hfOutputFile[0] == HFILE_ERROR)
    {
      LoadString(hInst, ERROR_202, szFormatString, sizeof(szFormatString));
      sprintf(szarString, szFormatString, tempString);
      MessageBeep(MB_ICONSTOP);
      MessageBox((HWND)NULL, szarString, TMS, MB_ICONSTOP);
      return(FALSE);
    }
//
//  Routes (Lines) file layout
//
//  Bytes   Description
//  ~~~~~   ~~~~~~~~~~~
//   1-5    Route number
//   6-13   Abbreviated route name (set to route number)
//  14-45   Full route name
//  46-53   Abbreviated route group name (same as abbreviated route name)
//  54-85   Full route group name (same as full route name)
//  86-88   Line flags (hard-coded to 111)
//   89     Division flag (hard-coded to 1)
//
    rcode2 = btrieve(B_GETFIRST, TMS_ROUTES, &ROUTES, &ROUTESKey1, 1);
    while(rcode2 == 0)
    {
      routeNumber = atoi(ROUTES.number);
      memset(longName, ' ', sizeof(longName));
      strncpy(longName, ROUTES.name, ROUTES_NAME_LENGTH);
      longName[32] = '\0';
      sprintf(outputString, "%5d%8d%s%8d%s1111\n", routeNumber, routeNumber, longName, routeNumber, longName);
      _lwrite(hfOutputFile[0], outputString, strlen(outputString));
      rcode2 = btrieve(B_GETNEXT, TMS_ROUTES, &ROUTES, &ROUTESKey1, 1);
    }
    _lclose(hfOutputFile[0]);
  }
//
//  Services
//
  if(RPTFLAGS.flags & TMSRPT11_SERVICES)
  {
    int  serviceNumber;
    char abbrName[max(SERVICES_NAME_LENGTH, 4) + 1];
    char longName[max(SERVICES_NAME_LENGTH, 8) + 1];

    LoadString(hInst, TEXT_126, tempString, TEMPSTRING_LENGTH);
    StatusBarText(tempString);
//
//  Open the output file
//
    strcpy(tempString, szDatabaseFileName);
    if((ptr = strrchr(tempString, '\\')) != NULL)
      *ptr = '\0';
    strcat(tempString, "\\pubser.dat");
    hfOutputFile[0] = _lcreat(tempString, 0);
    if(hfOutputFile[0] == HFILE_ERROR)
    {
      LoadString(hInst, ERROR_202, szFormatString, sizeof(szFormatString));
      sprintf(szarString, szFormatString, tempString);
      MessageBeep(MB_ICONSTOP);
      MessageBox((HWND)NULL, szarString, TMS, MB_ICONSTOP);
      return(FALSE);
    }
//
//  Services file layout
//
//  Bytes   Description
//  ~~~~~   ~~~~~~~~~~~
//    1     Service number
//   2-5    Abbreviated service name
//   6-13   Long service name
//
    rcode2 = btrieve(B_GETFIRST, TMS_SERVICES, &SERVICES, &SERVICESKey1, 1);
    while(rcode2 == 0)
    {
      serviceNumber = (int)SERVICES.number;
      memset(abbrName, ' ', sizeof(abbrName));
      strncpy(abbrName, SERVICES.name, SERVICES_NAME_LENGTH);
      abbrName[4] = '\0';
      memset(longName, ' ', sizeof(longName));
      strncpy(longName, SERVICES.name, SERVICES_NAME_LENGTH);
      longName[8] = '\0';
      sprintf(outputString, "%1d%s%s\n", serviceNumber, abbrName, longName);
      _lwrite(hfOutputFile[0], outputString, strlen(outputString));
      rcode2 = btrieve(B_GETNEXT, TMS_SERVICES, &SERVICES, &SERVICESKey1, 1);
    }
    _lclose(hfOutputFile[0]);
  }
//
//  Divisions
//
  if(RPTFLAGS.flags & TMSRPT11_DIVISIONS)
  {
    int  divisionNumber;
    char abbrName[max(DIVISIONS_NAME_LENGTH,  8) + 1];
    char longName[max(DIVISIONS_NAME_LENGTH, 15) + 1];

    LoadString(hInst, TEXT_127, tempString, TEMPSTRING_LENGTH);
    StatusBarText(tempString);
//
//  Open the output file
//
    strcpy(tempString, szDatabaseFileName);
    if((ptr = strrchr(tempString, '\\')) != NULL)
      *ptr = '\0';
    strcat(tempString, "\\pubdiv.dat");
    hfOutputFile[0] = _lcreat(tempString, 0);
    if(hfOutputFile[0] == HFILE_ERROR)
    {
      LoadString(hInst, ERROR_202, szFormatString, sizeof(szFormatString));
      sprintf(szarString, szFormatString, tempString);
      MessageBeep(MB_ICONSTOP);
      MessageBox((HWND)NULL, szarString, TMS, MB_ICONSTOP);
      return(FALSE);
    }
//
//  Divisions file layout
//
//  Bytes   Description
//  ~~~~~   ~~~~~~~~~~~
//   1-5    Division number
//   6-10   Sequence number (same as division number)
//  11-18   Abbreviated division name
//  19-33   Long division name
//
    rcode2 = btrieve(B_GETFIRST, TMS_DIVISIONS, &DIVISIONS, &DIVISIONSKey1, 1);
    while(rcode2 == 0)
    {
      divisionNumber = (int)DIVISIONS.number;
      memset(abbrName, ' ', sizeof(abbrName));
      strncpy(abbrName, DIVISIONS.name, DIVISIONS_NAME_LENGTH);
      abbrName[8] = '\0';
      memset(longName, ' ', sizeof(longName));
      strncpy(longName, DIVISIONS.name, DIVISIONS_NAME_LENGTH);
      longName[15] = '\0';
      sprintf(outputString, "%5d%5d%s%s\n", divisionNumber, divisionNumber, abbrName, longName);
      _lwrite(hfOutputFile[0], outputString, strlen(outputString));
      rcode2 = btrieve(B_GETNEXT, TMS_DIVISIONS, &DIVISIONS, &DIVISIONSKey1, 1);
    }
    _lclose(hfOutputFile[0]);
  }
//
//  Garages
//
  if(RPTFLAGS.flags & TMSRPT11_GARAGES)
  {
    int  garageNumber;
    int  divisionNumber;
    char abbrName[max(DIVISIONS_NAME_LENGTH,  8) + 1];
    char longName[max(DIVISIONS_NAME_LENGTH, 16) + 1];

    LoadString(hInst, TEXT_128, tempString, TEMPSTRING_LENGTH);
    StatusBarText(tempString);
//
//  Open the output file
//
    strcpy(tempString, szDatabaseFileName);
    if((ptr = strrchr(tempString, '\\')) != NULL)
      *ptr = '\0';
    strcat(tempString, "\\pubgar.dat");
    hfOutputFile[0] = _lcreat(tempString, 0);
    if(hfOutputFile[0] == HFILE_ERROR)
    {
      LoadString(hInst, ERROR_202, szFormatString, sizeof(szFormatString));
      sprintf(szarString, szFormatString, tempString);
      MessageBeep(MB_ICONSTOP);
      MessageBox((HWND)NULL, szarString, TMS, MB_ICONSTOP);
      return(FALSE);
    }
//
//  Garages file layout
//
//  Bytes   Description
//  ~~~~~   ~~~~~~~~~~~
//   1-5    Garage number
//   6-10   Division number
//  11-18   Abbreviated garage name
//  19-34   Long garage name
//
    NODESKey1.flags = NODES_FLAG_GARAGE;
    memset(NODESKey1.abbrName, 0x00, NODES_ABBRNAME_LENGTH);
    rcode2 = btrieve(B_GETGREATEROREQUAL, TMS_NODES, &NODES, &NODESKey1, 1);
    while(rcode2 == 0)
    {
      garageNumber = (int)NODES.number;
      memset(abbrName, ' ', sizeof(abbrName));
      strncpy(abbrName, NODES.abbrName, NODES_ABBRNAME_LENGTH);
      abbrName[8] = '\0';
      memset(longName, ' ', sizeof(longName));
      strncpy(longName, NODES.longName, NODES_LONGNAME_LENGTH);
      longName[16] = '\0';
      divisionNumber = garageNumber == 8888 ? 1 : 2;
      sprintf(outputString, "%5d%5d%s%s\n", garageNumber, divisionNumber, abbrName, longName);
      _lwrite(hfOutputFile[0], outputString, strlen(outputString));
      rcode2 = btrieve(B_GETNEXT, TMS_NODES, &NODES, &NODESKey1, 1);
    }
    _lclose(hfOutputFile[0]);
  }
//
//  Patterns / Nodes
//
//  This portion of the unload writes the following files:
//    pubtpt.dat - Timepoints
//    pubpat.dat - Patterns
//    pubptp.dat - Pattern Timepoints
//    publtp.dat - Line (Route) Timepoints
//    publts.dat - TOS Line Timepoints
//
  if(RPTFLAGS.flags & TMSRPT11_PATTERNSNODES)
  {
    typedef struct PNStruct
    {
      char mask[11];
      char patternName[max(PATTERNNAMES_NAME_LENGTH, 5) + 1];
      short int nodeNumbers[50];
    } PNDef;
    PNDef PN[10];
    short int  nodeNumber;
    char abbrName[max(NODES_ABBRNAME_LENGTH, 4) + 1];
    char longName[max(NODES_LONGNAME_LENGTH, 8) + 1];
    char intersection[max(NODES_INTERSECTION_LENGTH, 32) + 1];
    char routeName[max(ROUTES_NAME_LENGTH, 30) + 1];
    char nodeLabels[2][50][4];
    char nodeLongNames[2][52][8];
    char directionAbbr[2];
    BOOL bDoDirection[2];
    long currentPatternRecordID;
    int  patternNumber;
    int  numNodes;

//
//  Open all the output files
//
    for(nI = 0; nI < 5; nI++)
    {
      strcpy(tempString, szDatabaseFileName);
      if((ptr = strrchr(tempString, '\\')) != NULL)
        *ptr = '\0';
      switch(nI)
      {
        case 0:
          strcat(tempString, "\\pubtpt.dat");
          break;
        case 1:
          strcat(tempString, "\\pubpat.dat");
          break;
        case 2:
          strcat(tempString, "\\pubptp.dat");
          break;
        case 3:
          strcat(tempString, "\\publtp.dat");
          break;
        case 4:
          strcat(tempString, "\\publts.dat");
          break;
      }
      hfOutputFile[nI] = _lcreat(tempString, 0);
      if(hfOutputFile[nI] == HFILE_ERROR)
      {
        LoadString(hInst, ERROR_202, szFormatString, sizeof(szFormatString));
        sprintf(szarString, szFormatString, tempString);
        MessageBeep(MB_ICONSTOP);
        MessageBox((HWND)NULL, szarString, TMS, MB_ICONSTOP);
        return(FALSE);
      }
    }
//
//  Cycle through all routes.  Patterns in the publish follow the Min-Scheduler
//  convention - they're not by service.  Therefore, only service 1 patterns
//  get unloaded.
//
//  Get the service number.  We have to assume that it's the first "sort order" service.
//
    btrieve(B_GETFIRST, TMS_SERVICES, &SERVICES, &SERVICESKey1, 1);
//
//  Go through the routes
//
    rcode2 = btrieve(B_GETFIRST, TMS_ROUTES, &ROUTES, &ROUTESKey1, 1);
    while(rcode2 == 0)
    {
      if(StatusBarAbort())
        break;
      LoadString(hInst, TEXT_129, tempString, TEMPSTRING_LENGTH);
      routeNumber = atoi(ROUTES.number);
      strncpy(routeName, ROUTES.name, ROUTES_NAME_LENGTH);
      trim(routeName, ROUTES_NAME_LENGTH);
      sprintf(szarString, ": %d - %s", routeNumber, routeName);
      strcat(tempString, szarString);
      StatusBarText(tempString);
//
//  Get the nodes on the BASE pattern for this route
//
//  Nodes (pubtpt.dat: hfOutputFile[0]) file layout
//
//  Bytes   Description
//  ~~~~~   ~~~~~~~~~~~
//   1-2    Node number (binary)
//   3-6    Abbreviated name
//   7-38   Full name (Intersection)
//  39-46   Long name
//
      memset(nodeLabels, ' ', sizeof(nodeLabels));
      memset(nodeLongNames, ' ', sizeof(nodeLongNames));
      for(nI = 0; nI < 2; nI++)
      {
        bDoDirection[nI] = ROUTES.DIRECTIONSrecordID[nI] != NO_RECORD;
        if(!bDoDirection[nI])
          continue;
        PATTERNSKey2.ROUTESrecordID = ROUTES.recordID;
        PATTERNSKey2.SERVICESrecordID = SERVICES.recordID;
        PATTERNSKey2.directionIndex = nI;
        PATTERNSKey2.PATTERNNAMESrecordID = basePatternRecordID;
        PATTERNSKey2.nodeSequence = 0;
        rcode2 = btrieve(B_GETGREATEROREQUAL, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
        numNodes = 0;
        while(rcode2 == 0 &&
              PATTERNS.ROUTESrecordID == ROUTES.recordID &&
              PATTERNS.SERVICESrecordID == SERVICES.recordID &&
              PATTERNS.directionIndex == nI &&
              PATTERNSKey2.PATTERNNAMESrecordID == basePatternRecordID)
        {
          if(!(PATTERNS.flags & PATTERNS_FLAG_BUSSTOP))
          {
            NODESKey0.recordID = PATTERNS.NODESrecordID;
            btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
            nodeNumber = (short int)NODES.number;
            memset(abbrName, ' ', sizeof(abbrName));
            strncpy(abbrName, NODES.abbrName, NODES_ABBRNAME_LENGTH);
            abbrName[4] = '\0';
            memset(intersection, ' ', sizeof(intersection));
            strncpy(intersection, NODES.intersection, NODES_INTERSECTION_LENGTH);
            intersection[32] = '\0';
            memset(longName, ' ', sizeof(longName));
            strncpy(longName, NODES.longName, NODES_LONGNAME_LENGTH);
            longName[8] = '\0';
            sprintf(outputString, "  %s%s%s", abbrName, intersection, longName);
            memcpy(outputString, &nodeNumber, 2);
            _lwrite(hfOutputFile[0], outputString, 46);
            strncpy(nodeLabels[nI][numNodes], NODES.reliefLabels, 4);
            strncpy(nodeLongNames[nI][numNodes++], NODES.longName, 8);
          }
          rcode2 = btrieve(B_GETNEXT, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
        }
      }
//
//  Loop through both directions
//
      for(nI = 0; nI < 2; nI++)
      {
        if(!bDoDirection[nI])
          continue;
        PATTERNSKey2.ROUTESrecordID = ROUTES.recordID;
        PATTERNSKey2.SERVICESrecordID = SERVICES.recordID;
        PATTERNSKey2.directionIndex = nI;
        PATTERNSKey2.PATTERNNAMESrecordID = basePatternRecordID;
        PATTERNSKey2.nodeSequence = 0;
        currentPatternRecordID = NO_RECORD;
        patternNumber = 0;
        nJ = -1;
        rcode2 = btrieve(B_GETGREATEROREQUAL, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
        while(rcode2 == 0 &&
              PATTERNS.ROUTESrecordID == ROUTES.recordID &&
              PATTERNS.SERVICESrecordID == SERVICES.recordID &&
              PATTERNS.directionIndex == nI)
        {
          if(!(PATTERNS.flags & PATTERNS_FLAG_BUSSTOP))
          {
            if(PATTERNS.PATTERNNAMESrecordID != currentPatternRecordID)
            {
              nJ++;
              if(nJ > 9)
                break;
              numNodes = 0;
              memset(PN[nJ].nodeNumbers, 0x00, sizeof(PN[nJ].nodeNumbers));
              currentPatternRecordID = PATTERNS.PATTERNNAMESrecordID;
              PATTERNNAMESKey0.recordID = PATTERNS.PATTERNNAMESrecordID;
              btrieve(B_GETEQUAL, TMS_PATTERNNAMES, &PATTERNNAMES, &PATTERNNAMESKey0, 0);
              sprintf(PN[nJ].mask, "%5d%1d%5d", routeNumber, nI, ++patternNumber);
              memset(PN[nJ].patternName, ' ', sizeof(PN[0].patternName));
              strncpy(PN[nJ].patternName, PATTERNNAMES.name, 5);
              PN[nJ].patternName[5] = '\0';
            }
            NODESKey0.recordID = PATTERNS.NODESrecordID;
            btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
            PN[nJ].nodeNumbers[numNodes++] = (short int)NODES.number;
          }
          rcode2 = btrieve(B_GETNEXT, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
        }
//
//  Write the files
//
        for(nK = 0; nK < nJ; nK++)
        {
//
//  Patterns (pubpat.dat: hfOutputFile[1]) file layout
//  Bytes   Description
//  ~~~~~   ~~~~~~~~~~~
//   1-5    Route number
//    6     Direction
//   7-11   Pattern number
//  12-16   Pattern name
//  17-46   Blank
//
          memcpy(outputString, PN[nK].mask, 11);
          memcpy(&outputString[11], PN[nK].patternName, 5);
          memset(&outputString[16], ' ', 30);
          _lwrite(hfOutputFile[1], outputString, 46);
//
//  Pattern Timepoints (pubptp.dat: hfOutputFile[2]) file layout
//  Bytes   Description
//  ~~~~~   ~~~~~~~~~~~
//   1-5    Route number
//    6     Direction
//   7-11   Pattern number
//  12-111  Node numbers (Binary)
//
          memcpy(outputString, PN[nK].mask, 11);
          memcpy(&outputString[11], PN[nK].nodeNumbers, 100);
          _lwrite(hfOutputFile[2], outputString, 111);
        }
//
//  Line Timepoints (publtp.dat: hfOutputFile[3]) file layout
//  Bytes   Description
//  ~~~~~   ~~~~~~~~~~~
//   1-5    Route number
//    6     Direction
//   7-106  Node numbers (Binary)
// 107-306  Node labels
//
        memcpy(outputString, PN[0].mask, 6);
        memcpy(&outputString[6], PN[0].nodeNumbers, 100);
        memcpy(&outputString[106], nodeLabels[nI], 200);
        _lwrite(hfOutputFile[3], outputString, 306);
      }  // nI
//
//  Tos Timepoints (publts.dat: hfOutputFile[4]) file layout
//  Bytes   Description
//  ~~~~~   ~~~~~~~~~~~
//   1-4    Route number
//   5-34   Route name
//  35-36   Direction 0 abbreviated name
//  37-452  Direction 0 node long names
// 453-454  Direction 1 abbreviated name
// 455-870  Direction 1 node long names
//
      pad(routeName, 30);  // routeName is "trim"med earlied for the StatusBar
      routeName[30] = '\0';
      sprintf(outputString, "%4d%s", routeNumber, routeName);
      DIRECTIONSKey0.recordID = ROUTES.DIRECTIONSrecordID[0];
      btrieve(B_GETEQUAL, TMS_DIRECTIONS, &DIRECTIONS, &DIRECTIONSKey0, 0);
      strncpy(directionAbbr, DIRECTIONS.abbrName, 2);
      strncpy(&outputString[34], directionAbbr, 2);
      memcpy(&outputString[36], nodeLongNames[0], 416);
      if(ROUTES.DIRECTIONSrecordID[1] == NO_RECORD)
        memset(&outputString[452], ' ', 418);
      else
      {
        DIRECTIONSKey0.recordID = ROUTES.DIRECTIONSrecordID[1];
        btrieve(B_GETEQUAL, TMS_DIRECTIONS, &DIRECTIONS, &DIRECTIONSKey0, 0);
        strncpy(directionAbbr, DIRECTIONS.abbrName, 2);
        strncpy(&outputString[452], directionAbbr, 2);
        memcpy(&outputString[454], nodeLongNames[1], 416);
      }
      _lwrite(hfOutputFile[4], outputString, 870);
      rcode2 = btrieve(B_GETNEXT, TMS_ROUTES, &ROUTES, &ROUTESKey1, 1);
    }
//
//  Close all the output files
//
    for(nI = 0; nI < 5; nI++)
    {
      _lclose(hfOutputFile[nI]);
    }
  }
//
//  Blocks / Trips / Pieces
//
//  This portion of the unload writes the following files:
//    pubblk.dat - Blocks
//    pubspl.dat - TOS Pullouts
//    pubtrp.dat - Trips
//    pubtri.dat - TOS Trips
//
  if(RPTFLAGS.flags & TMSRPT11_BLOCKSTRIPSPIECES)
  {
    BOOL bQuit;
    char routeName[max(ROUTES_NAME_LENGTH, 30) + 1];
//
//  Open all the output files
//
    for(nI = 0; nI < 4; nI++)
    {
      strcpy(tempString, szDatabaseFileName);
      if((ptr = strrchr(tempString, '\\')) != NULL)
        *ptr = '\0';
      switch(nI)
      {
        case 0:
          strcat(tempString, "\\pubblk.dat");
          break;
        case 1:
          strcat(tempString, "\\pubspl.dat");
          break;
        case 2:
          strcat(tempString, "\\pubtrp.dat");
          break;
        case 3:
          strcat(tempString, "\\pubtri.dat");
          break;
      }
      hfOutputFile[nI] = _lcreat(tempString, 0);
      if(hfOutputFile[nI] == HFILE_ERROR)
      {
        LoadString(hInst, ERROR_202, szFormatString, sizeof(szFormatString));
        sprintf(szarString, szFormatString, tempString);
        MessageBeep(MB_ICONSTOP);
        MessageBox((HWND)NULL, szarString, TMS, MB_ICONSTOP);
        return(FALSE);
      }
    }
//
//  Cycle through all routes
//
    bQuit = FALSE;
    rcode2 = btrieve(B_GETFIRST, TMS_ROUTES, &ROUTES, &ROUTESKey1, 1);
    while(rcode2 == 0)
    {
      if(StatusBarAbort() || bQuit)
        break;
      LoadString(hInst, TEXT_131, tempString, TEMPSTRING_LENGTH);
      routeNumber = atoi(ROUTES.number);
      strncpy(routeName, ROUTES.name, ROUTES_NAME_LENGTH);
      trim(routeName, ROUTES_NAME_LENGTH);
      sprintf(szarString, ": %d - %s / Service: ", routeNumber, routeName);
      strcat(tempString, szarString);
//
//  Cycle through all services
//
      rcode2 = btrieve(B_GETFIRST, TMS_SERVICES, &SERVICES, &SERVICESKey1, 1);
      while(rcode2 == 0)
      {
        if(StatusBarAbort())
        {
          bQuit = TRUE;
          break;
        }
        strncpy(szarString, SERVICES.name, SERVICES_NAME_LENGTH);
        trim(szarString, SERVICES_NAME_LENGTH);
        strcat(tempString, szarString);
        StatusBarText(tempString);
        rcode2 = btrieve(B_GETNEXT, TMS_SERVICES, &SERVICES, &SERVICESKey1, 1);
      }
      rcode2 = btrieve(B_GETNEXT, TMS_ROUTES, &ROUTES, &ROUTESKey1, 1);
    }
  }
//
//  All done
//
  StatusBarEnd();
  strcpy(tempString, "Publish files were unloaded to:\n");
  strcpy(szarString, szDatabaseFileName);
  if((ptr = strrchr(szarString, '\\')) != NULL)
    *ptr = '\0';
  strcat(tempString, szarString);
  MessageBox(hWndMain, tempString, TMS, MB_OK);

  return(TRUE);
}

BOOL CALLBACK DATAPUBLISHMsgProc(HWND hWndDlg, UINT Message, WPARAM wParam, LPARAM lParam)
{
  static RPTFLAGSDef *pRPTFLAGS;
  static HANDLE hCtlDIRECTIONS;
  static HANDLE hCtlROUTES;
  static HANDLE hCtlSERVICES;
  static HANDLE hCtlDIVISIONS;
  static HANDLE hCtlGARAGES;
  static HANDLE hCtlPATTERNS;
  static HANDLE hCtlBLOCKS;
  static HANDLE hCtlRUNS;
  static HANDLE hCtlRUNTYPES;
  static HANDLE hCtlUSERS;
  short int wmId;

  switch(Message)
  {
//
//  WM_INITDIALOG - Initialize the dialog
//
    case WM_INITDIALOG:
      pRPTFLAGS = (RPTFLAGSDef *)lParam;
      if(pRPTFLAGS == NULL)
      {
        SendMessage(hWndDlg, WM_COMMAND, MAKEWPARAM(IDCANCEL, 0), (LPARAM)0);
        break;
      }
//
//  Set up the handles to the controls
//
      hCtlDIRECTIONS = GetDlgItem(hWndDlg, DATAPUBLISH_DIRECTIONS);
      hCtlROUTES = GetDlgItem(hWndDlg, DATAPUBLISH_ROUTES);
      hCtlSERVICES = GetDlgItem(hWndDlg, DATAPUBLISH_SERVICES);
      hCtlDIVISIONS = GetDlgItem(hWndDlg, DATAPUBLISH_DIVISIONS);
      hCtlGARAGES = GetDlgItem(hWndDlg, DATAPUBLISH_GARAGES);
      hCtlPATTERNS = GetDlgItem(hWndDlg, DATAPUBLISH_PATTERNS);
      hCtlBLOCKS = GetDlgItem(hWndDlg, DATAPUBLISH_BLOCKS);
      hCtlRUNS = GetDlgItem(hWndDlg, DATAPUBLISH_RUNS);
      hCtlRUNTYPES = GetDlgItem(hWndDlg, DATAPUBLISH_RUNTYPES);
      hCtlUSERS = GetDlgItem(hWndDlg, DATAPUBLISH_USERS);
//
//  Set the dialog title
//
      SendMessage(hWndDlg, WM_GETTEXT, (WPARAM)sizeof(szFormatString), (LONG)(LPSTR)szFormatString);
      sprintf(tempString, szFormatString, TMSRPT[pRPTFLAGS->nReportNumber].szReportName);
      SendMessage(hWndDlg, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)tempString);
//
//  Set all the checkboxes to "checked"
//
      SendMessage(hCtlDIRECTIONS, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
      SendMessage(hCtlROUTES, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
      SendMessage(hCtlSERVICES, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
      SendMessage(hCtlDIVISIONS, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
      SendMessage(hCtlGARAGES, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
      SendMessage(hCtlPATTERNS, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
      SendMessage(hCtlBLOCKS, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
      SendMessage(hCtlRUNS, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
      SendMessage(hCtlRUNTYPES, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
      SendMessage(hCtlUSERS, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
      break;
//
//  WM_CLOSE
//
    case WM_CLOSE:
      SendMessage(hWndDlg, WM_COMMAND, MAKEWPARAM(IDCANCEL, 0), (LPARAM)0);
      break;
//
//  WM_COMMAND
//
    case WM_COMMAND:
      wmId = LOWORD(wParam);
      switch(wmId)
      {
        case IDCANCEL:
          EndDialog(hWndDlg, FALSE);
          break;
//
//  IDOK
//
//  This one's a no-brainer.  Just figure out which ones he's checked, and leave.
//
        case IDOK:
          pRPTFLAGS->flags = 0;
//
//  Directions
//
          if(SendMessage(hCtlDIRECTIONS, BM_GETCHECK, (WPARAM)0, (LPARAM)0))
            pRPTFLAGS->flags |= TMSRPT11_DIRECTIONS;
//
//  Routes
//
          if(SendMessage(hCtlROUTES, BM_GETCHECK, (WPARAM)0, (LPARAM)0))
            pRPTFLAGS->flags |= TMSRPT11_ROUTES;
//
//  Services
//
          if(SendMessage(hCtlSERVICES, BM_GETCHECK, (WPARAM)0, (LPARAM)0))
            pRPTFLAGS->flags |= TMSRPT11_SERVICES;
//
//  Divisions
//
          if(SendMessage(hCtlDIVISIONS, BM_GETCHECK, (WPARAM)0, (LPARAM)0))
            pRPTFLAGS->flags |= TMSRPT11_DIVISIONS;
//
//  Garages
//
          if(SendMessage(hCtlGARAGES, BM_GETCHECK, (WPARAM)0, (LPARAM)0))
            pRPTFLAGS->flags |= TMSRPT11_GARAGES;

//
//  Patterns / Nodes
//
          if(SendMessage(hCtlPATTERNS, BM_GETCHECK, (WPARAM)0, (LPARAM)0))
            pRPTFLAGS->flags |= TMSRPT11_PATTERNSNODES;
//
//  Blocks / Trips / Pieces
//
          if(SendMessage(hCtlBLOCKS, BM_GETCHECK, (WPARAM)0, (LPARAM)0))
            pRPTFLAGS->flags |= TMSRPT11_BLOCKSTRIPSPIECES;
//
//  Runs
//
          if(SendMessage(hCtlRUNS, BM_GETCHECK, (WPARAM)0, (LPARAM)0))
            pRPTFLAGS->flags |= TMSRPT11_RUNS;
//
//  Runtypes
//
          if(SendMessage(hCtlRUNTYPES, BM_GETCHECK, (WPARAM)0, (LPARAM)0))
            pRPTFLAGS->flags |= TMSRPT11_RUNTYPES;
//
//  Users
//
          if(SendMessage(hCtlUSERS, BM_GETCHECK, (WPARAM)0, (LPARAM)0))
            pRPTFLAGS->flags |= TMSRPT11_USERS;

//
//  All done
//
          EndDialog(hWndDlg, TRUE);
          break;
      }
      break;

    default:
      return FALSE;
  }
  return TRUE;
}

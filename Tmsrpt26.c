//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
#include "TMSHeader.h"
#include "cistms.h"
#include <math.h>

BOOL  CALLBACK PRODRPTMsgProc(HWND, UINT, WPARAM, LPARAM);

//
//  Productivity Reports
//
//  From QETXT.INI:
//
//    [TMSRPT26]
//    FILE=Tmsrpt26.txt
//    FLN=0
//    TT=Tab
//    Charset=ANSI
//    DS=.
//    FIELD1=Conditionals,VARCHAR,128,0,128,0,
//    FIELD2=Service,VARCHAR,32,0,32,0,
//    FIELD3=Route,VARCHAR,75,0,75,0,
//    FIELD4=Direction,VARCHAR,16,0,16,0,
//    FIELD5=RevTime,NUMERIC,4,0,4,0,
//    FIELD6=RevDistance,NUMERIC,7,2,7,0,
//    FIELD7=DhdTime,NUMERIC,4,0,4,0,
//    FIELD8=DhdDistance,NUMERIC,7,2,7,0,
//

//#define DEBUGTMSRPT26

BOOL FAR TMSRPT26(TMSRPTPassedDataDef *pPassedData)
{
  GenerateTripDef GTResults;
  GetConnectionTimeDef GCTData;
  REPORTPARMSDef REPORTPARMS;
  RPTFLAGSDef RPTFLAGS;
  float distance;
  double revDistance;
  double dhdDistance;
  double tripDistance;
  HFILE hfOutputFile;
#ifdef DEBUGTMSRPT26
  HANDLE hDebugFile;
  DWORD  dwBytesWritten;
#endif
  BOOL  bKeepGoing = FALSE;
  BOOL  bFirstNode;
  BOOL  bFirstTrip;
  BOOL  bProcessedTrip;
  BOOL  bGennedNext;
  BOOL  bRC;
  BOOL  bPreviousWasStop;
  char  conditionals[128];
  char  outputString[512];
  char  routeNumberAndName[ROUTES_NUMBER_LENGTH + 3 + ROUTES_NAME_LENGTH + 1];
  char  serviceName[SERVICES_NAME_LENGTH + 1];
  char  directionName[DIRECTIONS_LONGNAME_LENGTH + 1];
  long  statbarTotal;
  long  revTime;
  long  dhdTime;
  long  POTime;
  long  PITime;
  long  deadheadTime;
  long  fromRoute;
  long  fromService;
  long  fromNode;
  long  fromTime;
  long  blockNumber;
  long  previousNode;
  long  equivalentTravelTime;
  long  flags;
  int   nI;
  int   nJ;
  int   nK;
  int   rcode2;
  int   currentRoute;
  int   currentService;

  pPassedData->nReportNumber = 25;
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
  if(REPORTPARMS.numRoutes == 0 ||REPORTPARMS.numServices == 0)
  {
    goto deallocate;
  }
//
//  Open the output file
//
  strcpy(tempString, szReportsTempFolder);
  strcat(tempString, "\\TMSRPT26.txt");
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
//  Open the debug file
//
#ifdef DEBUGTMSRPT26
  strcpy(tempString, szReportsTempFolder);
  strcat(tempString, "\\DEBUG26.txt");
  hDebugFile = CreateFile(tempString, GENERIC_WRITE, 0, NULL,
        CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
#endif
//
//  So far so good - see what's to be filtered
//
  RPTFLAGS.nReportNumber = pPassedData->nReportNumber;
  bRC = DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_PRODRPT),
        hWndMain, (DLGPROC)PRODRPTMsgProc, (LPARAM)&RPTFLAGS);
  if(!bRC)
    goto deallocate;
//
//  Make sure the runtimes are up to date
//
  m_bEstablishRUNTIMES = TRUE;
//
//  Build the header string
//
//  Before/After/All Day
//
  if(RPTFLAGS.flags & TMSRPT26_ALLDAY)
    LoadString(hInst, TEXT_254, tempString, TEMPSTRING_LENGTH);
  else
  {
    LoadString(hInst, (RPTFLAGS.flags & TMSRPT26_BEFORE) ? TEXT_248 : TEXT_249,
          tempString, TEMPSTRING_LENGTH);
    strcat(tempString, " ");
    strcat(tempString, Tchar(RPTFLAGS.time));
  }
  strcpy(conditionals, tempString);
  strcat(conditionals, ", ");
//
//  School trips
//
  LoadString(hInst, (RPTFLAGS.flags & TMSRPT26_SCHOOL) ? TEXT_250 : TEXT_251,
        tempString, TEMPSTRING_LENGTH);
  strcat(conditionals, tempString);
  strcat(conditionals, ", ");
//
//  Employee shuttles
//
  LoadString(hInst, (RPTFLAGS.flags & TMSRPT26_SHUTTLE) ? TEXT_252 : TEXT_253,
        tempString, TEMPSTRING_LENGTH);
  strcat(conditionals, tempString);
//
//  Fire up the status bar
//
  LoadString(hInst, TEXT_117, tempString, TEMPSTRING_LENGTH);
  StatusBarStart(hWndMain, tempString);
  statbarTotal = REPORTPARMS.numRoutes * REPORTPARMS.numServices;
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
//
//  Are we doing school routes?
//
      if((ROUTES.flags & ROUTES_FLAG_SCHOOL) && !(RPTFLAGS.flags & TMSRPT26_SCHOOL))
        continue;
//
//  Are we doing employee shuttles?
//
      if((ROUTES.flags & ROUTES_FLAG_EMPSHUTTLE) && !(RPTFLAGS.flags & TMSRPT26_SHUTTLE))
        continue;
//
//  Passed the tests
//
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
//  There has to be at least one trip blocked on the route
//
      TRIPSKey2.assignedToNODESrecordID = NO_RECORD;
      TRIPSKey2.RGRPROUTESrecordID = REPORTPARMS.pRouteList[nJ];
      TRIPSKey2.SGRPSERVICESrecordID = REPORTPARMS.pServiceList[nI];
      TRIPSKey2.blockNumber = 0;
      TRIPSKey2.blockSequence = NO_TIME;
      rcode2 = btrieve(B_GETGREATER, TMS_TRIPS, &TRIPS, &TRIPSKey2, 2);
      if(rcode2 != 0 ||
              TRIPS.standard.assignedToNODESrecordID != NO_RECORD ||
              TRIPS.standard.RGRPROUTESrecordID != REPORTPARMS.pRouteList[nJ] ||
              TRIPS.standard.SGRPSERVICESrecordID != REPORTPARMS.pServiceList[nI])
          continue;
//
//  Generate the trip
//
      GenerateTrip(TRIPS.ROUTESrecordID, TRIPS.SERVICESrecordID,
            TRIPS.directionIndex, TRIPS.PATTERNNAMESrecordID,
            TRIPS.timeAtMLP, GENERATETRIP_FLAG_DISPLAYERRORS, &GTResults);
//
//  Loop through the trips
//
      bFirstTrip = TRUE;
      bGennedNext = TRUE;
      while(rcode2 == 0 &&
            TRIPS.standard.assignedToNODESrecordID == NO_RECORD &&
            TRIPS.standard.RGRPROUTESrecordID == REPORTPARMS.pRouteList[nJ] &&
            TRIPS.standard.SGRPSERVICESrecordID == REPORTPARMS.pServiceList[nI])
      {
        bProcessedTrip = FALSE;
//
//  Get/verify the route
//
        ROUTESKey0.recordID = TRIPS.ROUTESrecordID;
        btrieve(B_GETEQUAL, TMS_ROUTES, &ROUTES, &ROUTESKey0, 0);
        if((ROUTES.flags & ROUTES_FLAG_SCHOOL) && !(RPTFLAGS.flags & TMSRPT26_SCHOOL))
          goto nexttrip;
        if((ROUTES.flags & ROUTES_FLAG_EMPSHUTTLE) && !(RPTFLAGS.flags & TMSRPT26_SHUTTLE))
          goto nexttrip;
        strncpy(routeNumberAndName, ROUTES.number, ROUTES_NUMBER_LENGTH);
        trim(routeNumberAndName, ROUTES_NUMBER_LENGTH);
        strcat(routeNumberAndName, " - ");
        strncpy(tempString, ROUTES.name, ROUTES_NAME_LENGTH);
        trim(tempString, ROUTES_NAME_LENGTH);
        strcat(routeNumberAndName, tempString);
//
//  Get the direction information
//
        nK = TRIPS.directionIndex;
        DIRECTIONSKey0.recordID = ROUTES.DIRECTIONSrecordID[nK];
        btrieve(B_GETEQUAL, TMS_DIRECTIONS, &DIRECTIONS, &DIRECTIONSKey0, 0);
        strncpy(directionName, DIRECTIONS.longName, DIRECTIONS_LONGNAME_LENGTH);
        trim(directionName, DIRECTIONS_LONGNAME_LENGTH);
//
//  Zero out times and miles and go
//
        blockNumber = TRIPS.standard.blockNumber;
        revTime = 0;
        revDistance = 0.0;
        dhdTime = 0;
        dhdDistance = 0.0;
//
//  Check on any before and after constraints
//
        if((RPTFLAGS.flags & TMSRPT26_ALLDAY) ||
              ((RPTFLAGS.flags & TMSRPT26_BEFORE) && RPTFLAGS.time > GTResults.firstNodeTime) ||
              ((RPTFLAGS.flags & TMSRPT26_AFTER ) && RPTFLAGS.time < GTResults.firstNodeTime))
        {
          if(!bGennedNext)
          {
            GenerateTrip(TRIPS.ROUTESrecordID, TRIPS.SERVICESrecordID,
                  TRIPS.directionIndex, TRIPS.PATTERNNAMESrecordID,
                  TRIPS.timeAtMLP, GENERATETRIP_FLAG_DISPLAYERRORS, &GTResults);
          }
//
//  Go through the pattern to get the distances, since
//  GenerateTrip only return distances betweeen timepoints
//
          bProcessedTrip = TRUE;
          PATTERNSKey2.ROUTESrecordID = TRIPS.ROUTESrecordID;
          PATTERNSKey2.SERVICESrecordID = TRIPS.SERVICESrecordID;
          PATTERNSKey2.PATTERNNAMESrecordID = TRIPS.PATTERNNAMESrecordID;
          PATTERNSKey2.directionIndex = nK;
          PATTERNSKey2.nodeSequence = -1;
          rcode2 = btrieve(B_GETGREATER, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
          tripDistance = 0.0;
          bFirstNode = TRUE;
          while(rcode2 == 0 &&
                PATTERNS.ROUTESrecordID == TRIPS.ROUTESrecordID &&
                PATTERNS.SERVICESrecordID == TRIPS.SERVICESrecordID &&
                PATTERNS.PATTERNNAMESrecordID == TRIPS.PATTERNNAMESrecordID &&
                PATTERNS.directionIndex == nK)
          {
            NODESKey0.recordID = PATTERNS.NODESrecordID;
            btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
            if(bFirstNode)
              bFirstNode = FALSE;
            else
            {
              GCTData.fromPATTERNNAMESrecordID = TRIPS.PATTERNNAMESrecordID;
              GCTData.toPATTERNNAMESrecordID = TRIPS.PATTERNNAMESrecordID;
              GCTData.fromROUTESrecordID = TRIPS.ROUTESrecordID;
              GCTData.fromSERVICESrecordID = TRIPS.SERVICESrecordID;
              GCTData.toROUTESrecordID = TRIPS.ROUTESrecordID;
              GCTData.toSERVICESrecordID = TRIPS.SERVICESrecordID;
              GCTData.fromNODESrecordID = previousNode;
              GCTData.toNODESrecordID = PATTERNS.NODESrecordID;
              GCTData.timeOfDay = GTResults.firstNodeTime;
              flags = (NODES.flags & NODES_FLAG_STOP || bPreviousWasStop) ? GCT_FLAG_ATLEASTONESTOP : 0;
              flags |= GCT_FLAG_RUNNINGTIME;
              GetConnectionTime(flags, &GCTData, &distance);
              if(distance < 0.0)
              {
                distance = -distance;
              }
              tripDistance += distance;
            }
            previousNode = NODES.recordID;
            bPreviousWasStop = NODES.flags & NODES_FLAG_STOP;
            rcode2 = btrieve(B_GETNEXT, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
          }
#ifdef DEBUGTMSRPT26
              sprintf(tempString, "%6ld\t%f\t%f\r\n", TRIPS.recordID, tripDistance, revDistance);
              WriteFile(hDebugFile, (LPCVOID *)tempString, strlen(tempString), &dwBytesWritten, NULL);
#endif
          revDistance += tripDistance;;
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
              dhdTime += deadheadTime;
              dhdDistance += distance;
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
            {
              dhdTime += deadheadTime;
              dhdDistance += distance;
            }
          }
          revTime = GTResults.lastNodeTime - GTResults.firstNodeTime;
        }
//
//  Save the pertinent information from this trip
//
        fromRoute = TRIPS.ROUTESrecordID;
        fromService = TRIPS.SERVICESrecordID;
        fromNode = GTResults.lastNODESrecordID;
        fromTime = GTResults.lastNodeTime;
//
//  Get the next trip in the block
//
      nexttrip:
        bGennedNext = FALSE;
        rcode2 = btrieve(B_GETNEXT, TMS_TRIPS, &TRIPS, &TRIPSKey2, 2);
        if(rcode2 == 0 &&
            TRIPS.standard.assignedToNODESrecordID == NO_RECORD &&
            TRIPS.standard.RGRPROUTESrecordID == REPORTPARMS.pRouteList[nJ] &&
            TRIPS.standard.SGRPSERVICESrecordID == REPORTPARMS.pServiceList[nI])
        {
//
//  If we actually looked at a trip, figure out the deadhead, then
//  the layover, and then write out the record to the output file
//
          GenerateTrip(TRIPS.ROUTESrecordID, TRIPS.SERVICESrecordID,
                TRIPS.directionIndex, TRIPS.PATTERNNAMESrecordID,
                TRIPS.timeAtMLP, GENERATETRIP_FLAG_DISPLAYERRORS, &GTResults);
          bGennedNext = TRUE;
          if(TRIPS.standard.blockNumber == blockNumber)
          {
            if(NodesEquivalent(fromNode, GTResults.firstNODESrecordID, &equivalentTravelTime))
              revTime += (GTResults.firstNodeTime - fromTime);
            else
            {
              GCTData.fromPATTERNNAMESrecordID = TRIPS.PATTERNNAMESrecordID;
              GCTData.toPATTERNNAMESrecordID = TRIPS.PATTERNNAMESrecordID;
              GCTData.fromROUTESrecordID = fromRoute;
              GCTData.fromSERVICESrecordID = fromService;
              GCTData.toROUTESrecordID = TRIPS.ROUTESrecordID;
              GCTData.toSERVICESrecordID = TRIPS.SERVICESrecordID;
              GCTData.fromNODESrecordID = fromNode;
              GCTData.toNODESrecordID = GTResults.firstNODESrecordID;
              GCTData.timeOfDay = fromTime;
              deadheadTime = GetConnectionTime(GCT_FLAG_DEADHEADTIME, &GCTData, &distance);
              distance = (float)fabs((double)distance);
              if(deadheadTime == NO_TIME)
              {
                revTime += (GTResults.firstNodeTime - fromTime);
              }
              else
              {
                dhdTime += deadheadTime;
                dhdDistance += distance;
                revTime += (GTResults.firstNodeTime - fromTime - deadheadTime);  // Layover
              }
            }
          }
        }
//
//  Write out the record
//
        if(bProcessedTrip)
        {
          sprintf(outputString, "\"%s\"\t\"%s\"\t\"%s\"\t\"%s\"\t%ld\t%6.2f\t%ld\t%6.2f\r\n",
                conditionals,
                serviceName,
                routeNumberAndName,
                directionName,
                revTime,
                (double)revDistance,
                dhdTime,
                (double)dhdDistance);
          _lwrite(hfOutputFile, outputString, strlen(outputString));
        }
      }
    }
  }
  StatusBar(-1L, -1L);
  bKeepGoing = TRUE;
//
//  Free allocated memory
//
  deallocate:
    TMSHeapFree(REPORTPARMS.pRouteList);
    TMSHeapFree(REPORTPARMS.pServiceList);
    StatusBarEnd();
    _lclose(hfOutputFile);
#ifdef DEBUGTMSRPT26
    CloseHandle(hDebugFile);
#endif
    SetCursor(hCursorArrow);

  return(bKeepGoing);
}


BOOL CALLBACK PRODRPTMsgProc(HWND hWndDlg, UINT Message, WPARAM wParam, LPARAM lParam)
{
  static HANDLE hCtlBEFORE;
  static HANDLE hCtlBEFORETIME;
  static HANDLE hCtlAFTER;
  static HANDLE hCtlAFTERTIME;
  static HANDLE hCtlSCHOOL;
  static HANDLE hCtlSHUTTLE;
  static RPTFLAGSDef *RPTFLAGS;
  HANDLE hCtl;

  switch(Message)
  {
//
//  WM_INITDIALOG - Initialize the dialog
//
    case WM_INITDIALOG:
      RPTFLAGS = (RPTFLAGSDef *)lParam;
      if(RPTFLAGS == NULL)
      {
        SendMessage(hWndDlg, WM_COMMAND, MAKEWPARAM(IDCANCEL, 0), (LPARAM)0);
        break;
      }
//
//  Set up handles to the controls
//
      hCtlBEFORE = GetDlgItem(hWndDlg, PRODRPT_BEFORE);
      hCtlBEFORETIME = GetDlgItem(hWndDlg, PRODRPT_BEFORETIME);
      hCtlAFTER = GetDlgItem(hWndDlg, PRODRPT_AFTER);
      hCtlAFTERTIME = GetDlgItem(hWndDlg, PRODRPT_AFTERTIME);
      hCtlSCHOOL = GetDlgItem(hWndDlg, PRODRPT_SCHOOL);
      hCtlSHUTTLE = GetDlgItem(hWndDlg, PRODRPT_SHUTTLE);
//
//  And set the default to "before"
//
      SendMessage(hCtlBEFORE, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
      EnableWindow(hCtlAFTERTIME, FALSE);
      break;

    case WM_CLOSE:
      SendMessage(hWndDlg, WM_COMMAND, MAKEWPARAM(IDCANCEL, 0), (LPARAM)0);
      break;

    case WM_COMMAND:
      switch(wParam)
      {
        case PRODRPT_BEFORE:
          EnableWindow(hCtlBEFORETIME, TRUE);
          SendMessage(hCtlAFTERTIME, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)"");
          EnableWindow(hCtlAFTERTIME, FALSE);
          break;

        case PRODRPT_AFTER:
          EnableWindow(hCtlAFTERTIME, TRUE);
          SendMessage(hCtlBEFORETIME, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)"");
          EnableWindow(hCtlBEFORETIME, FALSE);
          break;

        case IDCANCEL:
          EndDialog(hWndDlg, FALSE);
          break;
//
//  IDHELP
//
        case IDHELP:
          WinHelp(hWndDlg, szarHelpFile, HELP_CONTEXT, Technical_Support);
          break;
//
//  IDOK
//
        case IDOK:
          RPTFLAGS->flags = 0;
          if(SendMessage(hCtlBEFORE, BM_GETCHECK, (WPARAM)0, (LPARAM)0))
            RPTFLAGS->flags |= TMSRPT26_BEFORE;
          if(SendMessage(hCtlAFTER, BM_GETCHECK, (WPARAM)0, (LPARAM)0))
            RPTFLAGS->flags |= TMSRPT26_AFTER;
          if(SendMessage(hCtlSCHOOL, BM_GETCHECK, (WPARAM)0, (LPARAM)0))
            RPTFLAGS->flags |= TMSRPT26_SCHOOL;
          if(SendMessage(hCtlSHUTTLE, BM_GETCHECK, (WPARAM)0, (LPARAM)0))
            RPTFLAGS->flags |= TMSRPT26_SHUTTLE;
          if(SendMessage(hCtlBEFORE, BM_GETCHECK, (WPARAM)0, (LPARAM)0))
            hCtl = hCtlBEFORETIME;
          else
            hCtl = hCtlAFTERTIME;
          SendMessage(hCtl, WM_GETTEXT, (WPARAM)TEMPSTRING_LENGTH, (LONG)(LPSTR)tempString);
          if(strcmp(tempString, "") == 0)
          {
            RPTFLAGS->time = NO_TIME;
            RPTFLAGS->flags |= TMSRPT26_ALLDAY;
          }
          else
            RPTFLAGS->time = cTime(tempString);
          EndDialog(hWndDlg, TRUE);
          break;
      }
      break;

    default:
      return FALSE;
  }
  return TRUE;
}

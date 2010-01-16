//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
#include "TMSHeader.h"

BOOL CALLBACK COPYTRIPSMsgProc(HWND hWndDlg, UINT Message, WPARAM wParam, LPARAM lParam)
{
  static  PDISPLAYINFO pDI;
  static  HANDLE hCtlROUTE;
  static  HANDLE hCtlSERVICE;
  static  HANDLE hCtlOUTBOUND;
  static  HANDLE hCtlINBOUND;
  static  HANDLE hCtlCOPYBLOCKS;
  PATTERNSDef PATTERNSREC;
  TRIPSDef    TRIPSREC;  
  HCURSOR saveCursor;
  BOOL    bCopyBlocks;
  BOOL    bSame;
  BOOL    bFound;
  char    direction[DIRECTIONS_ABBRNAME_LENGTH + 1];
  char    ADDTest[sizeof(CONNECTIONSDef)];
  char    CONTest[sizeof(CONNECTIONSDef)];
  long    fromRouteRecordID;
  long    fromServiceRecordID;
  long    fromDirectionIndex;
  long    fromNodeRecordID;
  long    toNodeRecordID;
  long    insertRecordID;
  long    fromNode[500];
  long    toNode[500];
  long    recID[500];
  long    fNode;
  long    tNode;
  long    previousPattern;
  int     nI;
  int     nJ;
  int     numConnections;
  int     numRecs;
  int     rcode2;
  short int wmId;
  short int wmEvent;
  long flags;

  switch(Message)
  {
//
//  WM_INITDIALOG - Initialize the dialog
//
    case WM_INITDIALOG:
      pDI = (PDISPLAYINFO)lParam;
      if(pDI == NULL)
      {
        SendMessage(hWndDlg, WM_COMMAND, MAKEWPARAM(IDCANCEL, 0), (LPARAM)0);
        break;
      }
//
//  Set up the handles to the controls
//
      hCtlROUTE = GetDlgItem(hWndDlg, COPYTRIPS_ROUTE);
      hCtlSERVICE = GetDlgItem(hWndDlg, COPYTRIPS_SERVICE);
      hCtlOUTBOUND = GetDlgItem(hWndDlg, COPYTRIPS_OUTBOUND);
      hCtlINBOUND = GetDlgItem(hWndDlg, COPYTRIPS_INBOUND);
      hCtlCOPYBLOCKS = GetDlgItem(hWndDlg, COPYTRIPS_COPYBLOCKS);
//
//  Set up the "destination"
//
      flags = PLACEMENT_ROUTE | PLACEMENT_SERVICE | PLACEMENT_DIRECTION;
      DisplayPlacement(hWndDlg, COPYTRIPS_DESTINATION, pDI, flags);
//
//  Set up the route, service, and direciton
//
      if(SetUpRouteList(hWndDlg, COPYTRIPS_ROUTE, pDI->fileInfo.routeRecordID) == 0)
      {
        SendMessage(hWndDlg, WM_COMMAND, MAKEWPARAM(IDCANCEL, 0), (LPARAM)0);
        break;
      }
      nI = (int)SendMessage(hCtlROUTE, CB_GETCURSEL, (WPARAM)0, (LPARAM)0);
      ROUTESKey0.recordID = SendMessage(hCtlROUTE, CB_GETITEMDATA, (WPARAM)nI, (LPARAM)0);
      if(ROUTESKey0.recordID != CB_ERR)
      {
        btrieve(B_GETEQUAL, TMS_ROUTES, &ROUTES, &ROUTESKey0, 0);
        DIRECTIONSKey0.recordID = ROUTES.DIRECTIONSrecordID[0];
        btrieve(B_GETEQUAL, TMS_DIRECTIONS, &DIRECTIONS, &DIRECTIONSKey0, 0);
        strncpy(direction, DIRECTIONS.abbrName, DIRECTIONS_ABBRNAME_LENGTH);
        trim(direction, DIRECTIONS_ABBRNAME_LENGTH);
        SendMessage(hCtlOUTBOUND, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)direction);
        if(ROUTES.DIRECTIONSrecordID[1] == NO_RECORD)
        {
          LoadString(hInst, TEXT_130, tempString, TEMPSTRING_LENGTH);
          SendMessage(hCtlINBOUND, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)tempString);
          EnableWindow(hCtlINBOUND, FALSE);
        }
        else
        {
          DIRECTIONSKey0.recordID = ROUTES.DIRECTIONSrecordID[1];
          btrieve(B_GETEQUAL, TMS_DIRECTIONS, &DIRECTIONS, &DIRECTIONSKey0, 0);
          strncpy(direction, DIRECTIONS.abbrName, DIRECTIONS_ABBRNAME_LENGTH);
          trim(direction, DIRECTIONS_ABBRNAME_LENGTH);
          SendMessage(hCtlINBOUND, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)direction);
          EnableWindow(hCtlINBOUND, TRUE);
        }
      }
      if(SetUpServiceList(hWndDlg, COPYTRIPS_SERVICE, NO_RECORD) == 0)
      {
        SendMessage(hWndDlg, WM_COMMAND, MAKEWPARAM(IDCANCEL, 0), (LPARAM)0);
        break;
      }
      SendMessage(hCtlSERVICE, CB_SETCURSEL, (WPARAM)0, (LPARAM)0);
      if(pDI->fileInfo.directionIndex == 0)
        SendMessage(hCtlOUTBOUND, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
      else
        SendMessage(hCtlINBOUND, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
//
//  Default to copying block data
//
      SendMessage(hCtlCOPYBLOCKS, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
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
      wmEvent = HIWORD(wParam);
      switch(wmId)
      {
//
//  COPYTRIPS_ROUTE
//
        case COPYTRIPS_ROUTE:
          switch(wmEvent)
          {
            case CBN_SELCHANGE:
              nI = (int)SendMessage(hCtlROUTE, CB_GETCURSEL, (WPARAM)0, (LPARAM)0);
              ROUTESKey0.recordID = SendMessage(hCtlROUTE, CB_GETITEMDATA, (WPARAM)nI, (LPARAM)0);
              if(ROUTESKey0.recordID != CB_ERR)
              {
                btrieve(B_GETEQUAL, TMS_ROUTES, &ROUTES, &ROUTESKey0, 0);
                DIRECTIONSKey0.recordID = ROUTES.DIRECTIONSrecordID[0];
                btrieve(B_GETEQUAL, TMS_DIRECTIONS, &DIRECTIONS, &DIRECTIONSKey0, 0);
                strncpy(direction, DIRECTIONS.abbrName, DIRECTIONS_ABBRNAME_LENGTH);
                trim(direction, DIRECTIONS_ABBRNAME_LENGTH);
                SendMessage(hCtlOUTBOUND, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)direction);
                if(ROUTES.DIRECTIONSrecordID[1] == NO_RECORD)
                {
                  LoadString(hInst, TEXT_130, tempString, TEMPSTRING_LENGTH);
                  SendMessage(hCtlINBOUND, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)tempString);
                  EnableWindow(hCtlINBOUND, FALSE);
                }
                else
                {
                  DIRECTIONSKey0.recordID = ROUTES.DIRECTIONSrecordID[1];
                  btrieve(B_GETEQUAL, TMS_DIRECTIONS, &DIRECTIONS, &DIRECTIONSKey0, 0);
                  strncpy(direction, DIRECTIONS.abbrName, DIRECTIONS_ABBRNAME_LENGTH);
                  trim(direction, DIRECTIONS_ABBRNAME_LENGTH);
                  SendMessage(hCtlINBOUND, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)direction);
                  EnableWindow(hCtlINBOUND, TRUE);
                }
              }
              SendMessage(hCtlOUTBOUND, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)FALSE);
              SendMessage(hCtlINBOUND, BM_SETCHECK, (WPARAM)FALSE, (LPARAM)FALSE);
              break;
          }
          break;
//
//  IDOK
//
        case IDOK:
//
//  Get the route and service
//
//  Route
//
          nI = SendMessage(hCtlROUTE, CB_GETCURSEL, (WPARAM)0, (LPARAM)0);
          if(nI == CB_ERR)
          {
            TMSError(hWndDlg, MB_ICONSTOP, ERROR_010, hCtlROUTE);
            break;
          }
          fromRouteRecordID = SendMessage(hCtlROUTE, CB_GETITEMDATA, (WPARAM)nI, (LPARAM)0);
//
//  Service
//
          nI = SendMessage(hCtlSERVICE, CB_GETCURSEL, (WPARAM)0, (LPARAM)0);
          if(nI == CB_ERR)
          {
            TMSError(hWndDlg, MB_ICONSTOP, ERROR_011, hCtlSERVICE);
            break;
          }
          fromServiceRecordID = SendMessage(hCtlSERVICE, CB_GETITEMDATA, (WPARAM)nI, (LPARAM)0);
//
//  Direction
//
          fromDirectionIndex = SendMessage(hCtlOUTBOUND, BM_GETCHECK, (WPARAM)0, (LPARAM)0) ? 0 : 1;
//
//  Copy blocks?
//  
          bCopyBlocks = SendMessage(hCtlCOPYBLOCKS, BM_GETCHECK, (WPARAM)0, (LPARAM)0);
//
//  Make sure he isn't copying on top of himself
//
          if(fromRouteRecordID == pDI->fileInfo.routeRecordID &&
                fromServiceRecordID == pDI->fileInfo.serviceRecordID &&
                fromDirectionIndex == pDI->fileInfo.directionIndex)
          {
            TMSError(hWndDlg, MB_ICONSTOP, ERROR_169, hCtlROUTE);
            break;
          }
//
//  Are there existing patterns or trips?  If so, let him know
//  that we're going to shitcan them before we actually do.
//
//  Patterns
//
          PATTERNSKey2.ROUTESrecordID = pDI->fileInfo.routeRecordID;
          PATTERNSKey2.SERVICESrecordID = pDI->fileInfo.serviceRecordID;
          PATTERNSKey2.directionIndex = pDI->fileInfo.directionIndex;
          PATTERNSKey2.PATTERNNAMESrecordID = NO_RECORD;
          PATTERNSKey2.nodeSequence = NO_RECORD;
          rcode2 = btrieve(B_GETGREATEROREQUAL, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
          if(rcode2 == 0 &&
                PATTERNS.ROUTESrecordID == pDI->fileInfo.routeRecordID &&
                PATTERNS.SERVICESrecordID == pDI->fileInfo.serviceRecordID &&
                PATTERNS.directionIndex == pDI->fileInfo.directionIndex)
          {
            LoadString(hInst, ERROR_235, tempString, TEMPSTRING_LENGTH);
            MessageBeep(MB_ICONQUESTION);
            if(MessageBox(hWndDlg, tempString, TMS,
                  MB_OK | MB_ICONQUESTION | MB_YESNOCANCEL | MB_DEFBUTTON2) != IDYES)
              break;
          }
//
//  Trips
//
          TRIPSKey1.ROUTESrecordID = pDI->fileInfo.routeRecordID;
          TRIPSKey1.SERVICESrecordID = pDI->fileInfo.serviceRecordID;
          TRIPSKey1.directionIndex = pDI->fileInfo.directionIndex;
          TRIPSKey1.tripSequence = NO_TIME;
          rcode2 = btrieve(B_GETGREATEROREQUAL, TMS_TRIPS, &TRIPS, &TRIPSKey1, 1);
          if(rcode2 == 0 &&
                TRIPS.ROUTESrecordID == pDI->fileInfo.routeRecordID &&
                TRIPS.SERVICESrecordID == pDI->fileInfo.serviceRecordID &&
                TRIPS.directionIndex == pDI->fileInfo.directionIndex)
          {
            LoadString(hInst, ERROR_236, tempString, TEMPSTRING_LENGTH);
            MessageBeep(MB_ICONQUESTION);
            if(MessageBox(hWndDlg, tempString, TMS,
                  MB_OK | MB_ICONQUESTION | MB_YESNOCANCEL | MB_DEFBUTTON2) != IDYES)
              break;
          }
//
//  Kill them.  Kill them all.
//
//  Patterns
//
          PATTERNSKey2.ROUTESrecordID = pDI->fileInfo.routeRecordID;
          PATTERNSKey2.SERVICESrecordID = pDI->fileInfo.serviceRecordID;
          PATTERNSKey2.directionIndex = pDI->fileInfo.directionIndex;
          PATTERNSKey2.PATTERNNAMESrecordID = NO_RECORD;
          PATTERNSKey2.nodeSequence = NO_RECORD;
          rcode2 = btrieve(B_GETGREATEROREQUAL, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
          while(rcode2 == 0 &&
                PATTERNS.ROUTESrecordID == pDI->fileInfo.routeRecordID &&
                PATTERNS.SERVICESrecordID == pDI->fileInfo.serviceRecordID &&
                PATTERNS.directionIndex == pDI->fileInfo.directionIndex)
          {
            btrieve(B_DELETE, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
            PATTERNSKey2.ROUTESrecordID = pDI->fileInfo.routeRecordID;
            PATTERNSKey2.SERVICESrecordID = pDI->fileInfo.serviceRecordID;
            PATTERNSKey2.directionIndex = pDI->fileInfo.directionIndex;
            PATTERNSKey2.PATTERNNAMESrecordID = NO_RECORD;
            PATTERNSKey2.nodeSequence = NO_RECORD;
            rcode2 = btrieve(B_GETGREATEROREQUAL, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
          }
//
//  Trips
//
          TRIPSKey1.ROUTESrecordID = pDI->fileInfo.routeRecordID;
          TRIPSKey1.SERVICESrecordID = pDI->fileInfo.serviceRecordID;
          TRIPSKey1.directionIndex = pDI->fileInfo.directionIndex;
          TRIPSKey1.tripSequence = NO_TIME;
          rcode2 = btrieve(B_GETGREATEROREQUAL, TMS_TRIPS, &TRIPS, &TRIPSKey1, 1);
          while(rcode2 == 0 &&
                TRIPS.ROUTESrecordID == pDI->fileInfo.routeRecordID &&
                TRIPS.SERVICESrecordID == pDI->fileInfo.serviceRecordID &&
                TRIPS.directionIndex == pDI->fileInfo.directionIndex)
          {
            btrieve(B_DELETE, TMS_TRIPS, &TRIPS, &TRIPSKey1, 1);
            TRIPSKey1.ROUTESrecordID = pDI->fileInfo.routeRecordID;
            TRIPSKey1.SERVICESrecordID = pDI->fileInfo.serviceRecordID;
            TRIPSKey1.directionIndex = pDI->fileInfo.directionIndex;
            TRIPSKey1.tripSequence = NO_TIME;
            rcode2 = btrieve(B_GETGREATEROREQUAL, TMS_TRIPS, &TRIPS, &TRIPSKey1, 1);
          }
//
//  Copy them.  Copy them all.
//
//  Establish the new patterns recordID
//
          rcode2 = btrieve(B_GETLAST, TMS_PATTERNS, &PATTERNS, &PATTERNSKey0, 0);
          insertRecordID = AssignRecID(rcode2, PATTERNS.recordID);
//
//  Start the copy
//
          saveCursor = SetCursor(hCursorWait);
          PATTERNSKey2.ROUTESrecordID = fromRouteRecordID;
          PATTERNSKey2.SERVICESrecordID = fromServiceRecordID;
          PATTERNSKey2.directionIndex = fromDirectionIndex;
          PATTERNSKey2.PATTERNNAMESrecordID = NO_RECORD;
          PATTERNSKey2.nodeSequence = NO_RECORD;
          rcode2 = btrieve(B_GETGREATEROREQUAL, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
          while(rcode2 == 0 &&
                PATTERNS.ROUTESrecordID == fromRouteRecordID &&
                PATTERNS.SERVICESrecordID == fromServiceRecordID &&
                PATTERNS.directionIndex == fromDirectionIndex)
          {
            btrieve(B_GETPOSITION, TMS_PATTERNS, &PATTERNSREC, &PATTERNSKey2, 2);
            PATTERNS.recordID = insertRecordID++;
            PATTERNS.ROUTESrecordID = pDI->fileInfo.routeRecordID;
            PATTERNS.SERVICESrecordID = pDI->fileInfo.serviceRecordID;
            PATTERNS.directionIndex = pDI->fileInfo.directionIndex;
            btrieve(B_INSERT, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
            btrieve(B_GETDIRECT, TMS_PATTERNS, &PATTERNSREC, &PATTERNSKey2, 2);
            rcode2 = btrieve(B_GETNEXT, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
          }
//
//  Establish the new trips recordID
//
          rcode2 = btrieve(B_GETLAST, TMS_TRIPS, &TRIPS, &TRIPSKey0, 0);
          insertRecordID = AssignRecID(rcode2, TRIPS.recordID);
//
//  Start the copy
//
          TRIPSKey1.ROUTESrecordID = fromRouteRecordID;
          TRIPSKey1.SERVICESrecordID = fromServiceRecordID;
          TRIPSKey1.directionIndex = fromDirectionIndex;
          TRIPSKey1.tripSequence = NO_TIME;
          rcode2 = btrieve(B_GETGREATEROREQUAL, TMS_TRIPS, &TRIPS, &TRIPSKey1, 1);
          while(rcode2 == 0 &&
                TRIPS.ROUTESrecordID == fromRouteRecordID &&
                TRIPS.SERVICESrecordID == fromServiceRecordID &&
                TRIPS.directionIndex == fromDirectionIndex)
          {
            btrieve(B_GETPOSITION, TMS_TRIPS, &TRIPSREC, &TRIPSKey1, 1);
            TRIPS.recordID = insertRecordID++;
            TRIPS.ROUTESrecordID = pDI->fileInfo.routeRecordID;
            TRIPS.SERVICESrecordID = pDI->fileInfo.serviceRecordID;
            if(bCopyBlocks)
            {
              TRIPS.standard.SGRPSERVICESrecordID = pDI->fileInfo.serviceRecordID;
              TRIPS.dropback.SGRPSERVICESrecordID = pDI->fileInfo.serviceRecordID;
            }
            else
            {
              TRIPS.standard.RGRPROUTESrecordID = NO_RECORD;
              TRIPS.standard.SGRPSERVICESrecordID = NO_RECORD;
              TRIPS.standard.blockNumber = 0;
              TRIPS.standard.POGNODESrecordID = NO_RECORD;
              TRIPS.standard.PIGNODESrecordID = NO_RECORD;
              TRIPS.dropback.RGRPROUTESrecordID = NO_RECORD;
              TRIPS.dropback.SGRPSERVICESrecordID = NO_RECORD;
              TRIPS.dropback.blockNumber = 0;
              TRIPS.dropback.POGNODESrecordID = NO_RECORD;
              TRIPS.dropback.PIGNODESrecordID = NO_RECORD;
            }
            TRIPS.directionIndex = pDI->fileInfo.directionIndex;
            btrieve(B_INSERT, TMS_TRIPS, &TRIPS, &TRIPSKey1, 1);
            btrieve(B_GETDIRECT, TMS_TRIPS, &TRIPSREC, &TRIPSKey1, 1);
            rcode2 = btrieve(B_GETNEXT, TMS_TRIPS, &TRIPS, &TRIPSKey1, 1);
          }
//
//  Copy any specific route/service connections over to the new service
//
//  Get the base pattern nodes
//
          numConnections = 0;
          PATTERNSKey2.ROUTESrecordID = fromRouteRecordID;
          PATTERNSKey2.SERVICESrecordID = fromServiceRecordID;
          PATTERNSKey2.directionIndex = fromDirectionIndex;
          PATTERNSKey2.PATTERNNAMESrecordID = basePatternRecordID;
          PATTERNSKey2.nodeSequence = 0;
          previousPattern = NO_RECORD;
          rcode2 = btrieve(B_GETGREATEROREQUAL, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
//
//   Set up unique from/to node combinations
//  
//   Do the stops first
//
          while(rcode2 == 0 &&
                PATTERNS.ROUTESrecordID == fromRouteRecordID &&
                PATTERNS.SERVICESrecordID == fromServiceRecordID &&
                PATTERNS.directionIndex == fromDirectionIndex)
          {
            if(PATTERNS.PATTERNNAMESrecordID != previousPattern)
              fromNodeRecordID = PATTERNS.NODESrecordID;
            else
            {
              toNodeRecordID = PATTERNS.NODESrecordID; 
              for(bFound = FALSE, nI = 0; nI < numConnections; nI++)
              {
                if((bFound = (fromNodeRecordID == fromNode[nI] &&
                      toNodeRecordID == toNode[nI])) == TRUE)
                  break;
              }
              if(!bFound)
              {
                fromNode[numConnections] = fromNodeRecordID;
                toNode[numConnections++] = toNodeRecordID;
              }
              fromNodeRecordID = toNodeRecordID;
            }
            previousPattern = PATTERNS.PATTERNNAMESrecordID;
            rcode2 = btrieve(B_GETNEXT, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
          }
//
//  Done the stops, now do just the timepoints
//
          PATTERNSKey2.ROUTESrecordID = fromRouteRecordID;
          PATTERNSKey2.SERVICESrecordID = fromServiceRecordID;
          PATTERNSKey2.directionIndex = fromDirectionIndex;
          PATTERNSKey2.PATTERNNAMESrecordID = basePatternRecordID;
          PATTERNSKey2.nodeSequence = 0;
          previousPattern = NO_RECORD;
          rcode2 = btrieve(B_GETGREATEROREQUAL, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
//
//   Set up unique from/to node combinations
//  
          while(rcode2 == 0 &&
                PATTERNS.ROUTESrecordID == fromRouteRecordID &&
                PATTERNS.SERVICESrecordID == fromServiceRecordID &&
                PATTERNS.directionIndex == fromDirectionIndex)
          {
            if(PATTERNS.PATTERNNAMESrecordID != previousPattern)
              fromNodeRecordID = PATTERNS.NODESrecordID;
            else
            {
              if(!(PATTERNS.flags & PATTERNS_FLAG_BUSSTOP))
              {
                toNodeRecordID = PATTERNS.NODESrecordID; 
                for(bFound = FALSE, nI = 0; nI < numConnections; nI++)
                {
                  if((bFound = (fromNodeRecordID == fromNode[nI] &&
                        toNodeRecordID == toNode[nI])) == TRUE)
                    break;
                }
                if(!bFound)
                {
                  fromNode[numConnections] = fromNodeRecordID;
                  toNode[numConnections++] = toNodeRecordID;
                }
                fromNodeRecordID = toNodeRecordID;
              }
            }
            previousPattern = PATTERNS.PATTERNNAMESrecordID;
            rcode2 = btrieve(B_GETNEXT, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
          }
//
//  Now cycle through the connections
//
          numRecs = 0;
          for(nI = 0; nI < numConnections; nI++)
          {
//
//  Build the list of record ids to copy from
//
            for(nJ = 0; nJ < 2; nJ++)
            {
              fNode = nJ == 0 ? fromNode[nI] : toNode[nI];
              tNode = nJ == 0 ? toNode[nI] : fromNode[nI];
              CONNECTIONSKey1.fromNODESrecordID = fNode;
              CONNECTIONSKey1.toNODESrecordID = tNode;
              CONNECTIONSKey1.fromTimeOfDay = NO_TIME;
              rcode2 = btrieve(B_GETGREATEROREQUAL, TMS_CONNECTIONS, &CONNECTIONS, &CONNECTIONSKey1, 1);
              while(rcode2 == 0 &&
                    CONNECTIONS.fromNODESrecordID == fNode &&
                    CONNECTIONS.toNODESrecordID == tNode)
              {
//
//  Case 1: Copying same route to different service
//
//  If there's a connection that has the original route and service,
//  then copy it with the service changed.
//
                if(fromRouteRecordID == pDI->fileInfo.routeRecordID &&
                      fromServiceRecordID != pDI->fileInfo.serviceRecordID)
                {
                  if(CONNECTIONS.fromROUTESrecordID == fromRouteRecordID &&
                        CONNECTIONS.fromSERVICESrecordID == fromServiceRecordID &&
                        CONNECTIONS.toROUTESrecordID == fromRouteRecordID &&
                        CONNECTIONS.toSERVICESrecordID == fromServiceRecordID)
                    recID[numRecs++] = CONNECTIONS.recordID;
                }
//
//  Case 2: Copying different route to same service
//
//  If there's a connection that has the original route and service,
//  then copy it with the route changed.  If there's a connection that
//  has the original and no service, then copy it with the route changed.
//
                if(fromRouteRecordID != pDI->fileInfo.routeRecordID &&
                      fromServiceRecordID == pDI->fileInfo.serviceRecordID)
                {
                  if(CONNECTIONS.fromROUTESrecordID == fromRouteRecordID &&
                        CONNECTIONS.toROUTESrecordID == fromRouteRecordID &&
                        ((CONNECTIONS.fromSERVICESrecordID == fromServiceRecordID &&
                             CONNECTIONS.toSERVICESrecordID == fromServiceRecordID) ||
                         (CONNECTIONS.toSERVICESrecordID == NO_RECORD &&
                             CONNECTIONS.fromSERVICESrecordID == NO_RECORD)))
                    recID[numRecs++] = CONNECTIONS.recordID;
                }
//
//  Case 3: Copying different route to different service
//
//  If there is a connection that has the original route and service,
//  then copy it with the route and service changed.  If there is a connection
//  that has the original route and no service, then copy it with the route changed.
//
                if(fromRouteRecordID != pDI->fileInfo.routeRecordID &&
                      fromServiceRecordID != pDI->fileInfo.serviceRecordID)
                {
                  if(CONNECTIONS.fromROUTESrecordID == fromRouteRecordID &&
                        CONNECTIONS.toROUTESrecordID == fromRouteRecordID &&
                        ((CONNECTIONS.fromSERVICESrecordID == fromServiceRecordID &&
                             CONNECTIONS.toSERVICESrecordID == fromServiceRecordID) ||
                         (CONNECTIONS.toSERVICESrecordID == NO_RECORD &&
                             CONNECTIONS.fromSERVICESrecordID == NO_RECORD)))
                    recID[numRecs++] = CONNECTIONS.recordID;
                }
//
//  Case 4: Copying all routes to different service
//
//  If there is a connection that has NO_RECORD route and original service,
//  then copy it with the service changed.
//
                if(fromRouteRecordID != pDI->fileInfo.routeRecordID &&
                      fromServiceRecordID != pDI->fileInfo.serviceRecordID)
                {
                  if(CONNECTIONS.fromROUTESrecordID == NO_RECORD &&
                        CONNECTIONS.toROUTESrecordID == NO_RECORD &&
                        CONNECTIONS.fromSERVICESrecordID == fromServiceRecordID &&
                             CONNECTIONS.toSERVICESrecordID == fromServiceRecordID)
                    recID[numRecs++] = CONNECTIONS.recordID;
                }

//
//  Get the next connections record
//
                rcode2 = btrieve(B_GETNEXT, TMS_CONNECTIONS, &CONNECTIONS, &CONNECTIONSKey1, 1); 
              }
            }
          }
//
//  Drop any duplicates
//
          for(nI = 0; nI < numRecs - 1; nI++)
          {
            if(recID[nI] == NO_RECORD)
              continue;
            for(nJ = nI + 1; nJ < numRecs; nJ++)
            {
              if(recID[nI] == recID[nJ])
                recID[nJ] = NO_RECORD;
            }
          }
//
//  Go through the records
//
          for(nI = 0; nI < numRecs; nI++)
          {
            if(recID[nI] == NO_RECORD)
              continue;
//
//  Build up the new record
//
            rcode2 = btrieve(B_GETLAST, TMS_CONNECTIONS, &CONNECTIONS, &CONNECTIONSKey0, 0);
            insertRecordID = AssignRecID(rcode2, CONNECTIONS.recordID);
            CONNECTIONSKey0.recordID = recID[nI];
            btrieve(B_GETEQUAL, TMS_CONNECTIONS, &CONNECTIONS, &CONNECTIONSKey0, 0);
            CONNECTIONS.recordID = insertRecordID;
            CONNECTIONS.fromSERVICESrecordID = pDI->fileInfo.serviceRecordID;
            CONNECTIONS.toSERVICESrecordID = pDI->fileInfo.serviceRecordID;
            CONNECTIONS.fromROUTESrecordID = pDI->fileInfo.routeRecordID;
            CONNECTIONS.toROUTESrecordID = pDI->fileInfo.routeRecordID;
//
//  Make sure we're not adding a duplicate
//
            memcpy(&ADDTest, &CONNECTIONS, sizeof(CONNECTIONSDef));
            fNode = CONNECTIONS.fromNODESrecordID;
            tNode = CONNECTIONS.toNODESrecordID;
            CONNECTIONSKey1.fromNODESrecordID = fNode;
            CONNECTIONSKey1.toNODESrecordID = tNode;
            CONNECTIONSKey1.fromTimeOfDay = NO_TIME;
            bSame = FALSE;
            rcode2 = btrieve(B_GETGREATEROREQUAL, TMS_CONNECTIONS, &CONNECTIONS, &CONNECTIONSKey1, 1);
            while(rcode2 == 0 &&
                  CONNECTIONS.fromNODESrecordID == fNode &&
                  CONNECTIONS.toNODESrecordID == tNode)
            {
              memcpy(&CONTest, &CONNECTIONS, sizeof(CONNECTIONSDef));  
              bSame = TRUE;
              for(nJ = 4; nJ < sizeof(CONNECTIONSDef); nJ++)  // Start after the record ID
              {
                if(CONTest[nJ] != ADDTest[nJ])
                {
                  bSame = FALSE;
                  break;
                }
              }
              if(bSame)
                break;
              rcode2 = btrieve(B_GETNEXT, TMS_CONNECTIONS, &CONNECTIONS, &CONNECTIONSKey1, 1);
            }
//
//  If it's unique, add it
//
            if(!bSame)
            {
              memcpy(&CONNECTIONS, &ADDTest, sizeof(CONNECTIONSDef));
              btrieve(B_INSERT, TMS_CONNECTIONS, &CONNECTIONS, &CONNECTIONSKey0, 0);
            }
          }
//
//  Let him know we're done
//
          SetCursor(saveCursor);
          MessageBeep(MB_ICONINFORMATION);
          LoadString(hInst, TEXT_153, tempString, TEMPSTRING_LENGTH);
          MessageBox(hWndDlg, tempString, TMS, MB_ICONINFORMATION);
          EndDialog(hWndDlg, TRUE);
          break;
//
//  IDCANCEL
//
        case IDCANCEL:
          EndDialog(hWndDlg, FALSE);
          break;
//
//  IDHELP
//
        case IDHELP:
          WinHelp(hWndDlg, szarHelpFile, HELP_CONTEXT, The_Patterns_Table);
          break;
      }
      break;

    default:
      return FALSE;
  }
  return TRUE;
}

//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
#include "TMSHeader.h"

BOOL CALLBACK RPTCONNMsgProc(HWND, UINT, WPARAM, LPARAM);

#define NUMCOLS 18
#define TMSRPT08_MAXNODES 500
//
//  TMSRPT08() - Connections and potentials
//

int sort_nodes( const void *a, const void *b)
{
  char firstNode[NODES_ABBRNAME_LENGTH + 1];
  char secondNode[NODES_ABBRNAME_LENGTH + 1];

  NODESKey0.recordID = *((long *)a);
  btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
  strncpy(firstNode, NODES.abbrName, NODES_ABBRNAME_LENGTH);
  trim(firstNode, NODES_ABBRNAME_LENGTH);
  NODESKey0.recordID = *((long *)b);
  btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
  strncpy(secondNode, NODES.abbrName, NODES_ABBRNAME_LENGTH);
  trim(secondNode, NODES_ABBRNAME_LENGTH);

  return(strcmp(firstNode, secondNode));
}

BOOL FAR TMSRPT08(TMSRPTPassedDataDef *pPassedData)
{
  RPTFLAGSDef RPTFLAGS;
  float distance;
  float longitude[2];
  float latitude[2];
  HFILE hfOutputFile;
  BOOL  bRC;
  BOOL  bFound;
  char  fromNodeName[NODES_ABBRNAME_LENGTH + 1];
  char  toNodeName[NODES_ABBRNAME_LENGTH + 1];
  char  SBFromNodeName[NODES_ABBRNAME_LENGTH * 2 + 1];
  char  SBToNodeName[NODES_ABBRNAME_LENGTH * 2 + 1];
  long  recordIDs[TMSRPT08_MAXNODES];
  long  fromNode;
  long  toNode;
  long  prevPattern;
  long  prevLastNode;
  int   nI;
  int   nJ;
  int   nK;
  int   nL;
  int   numNodes;
  int   rcode2;
  int   seqNum;

  pPassedData->nReportNumber = 7;
  pPassedData->numDataFiles = 1;
//
//  See what he wants
//
  RPTFLAGS.nReportNumber = pPassedData->nReportNumber;
  bRC = DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_RPTCONN),
        hWndMain, (DLGPROC)RPTCONNMsgProc, (LPARAM)&RPTFLAGS);
  if(!bRC)
  {
    return(FALSE);
  }
//
//  Open the output file
//
  strcpy(tempString, szReportsTempFolder);
  strcat(tempString, "\\tmsrpt08.txt");
  hfOutputFile = _lcreat(tempString, 0);
  if(hfOutputFile == HFILE_ERROR)
  {
    LoadString(hInst, ERROR_202, szFormatString, sizeof(szFormatString));
    sprintf(szarString, szFormatString, tempString);
    MessageBeep(MB_ICONSTOP);
    MessageBox((HWND)NULL, szarString, TMS, MB_ICONSTOP);
    return(FALSE);
  }
  strcpy(pPassedData->szReportDataFile[0], tempString);
//
//  Loop through all the nodes in the system
//
  if(RPTFLAGS.flags & TMSRPT08_INCLUDEEXISTING)
  {
    rcode2 = btrieve(B_GETFIRST, TMS_NODES, &NODES, &NODESKey2, 2);
    numNodes = 0;
    while(rcode2 == 0)
    {
      if(!(NODES.flags & NODES_FLAG_STOP))
      {
        recordIDs[numNodes++] = NODES.recordID;
        if(numNodes >= TMSRPT08_MAXNODES)
          break;
      }
      rcode2 = btrieve(B_GETNEXT, TMS_NODES, &NODES, &NODESKey2, 2);
    }
  }
//
//  Determine all the start and end pattern nodes in the system.  Do this by looping
//  through all the services, then all the routes, then all the directions, then all
//  the patterns for those routes, services, and directions.
//
  else if(RPTFLAGS.flags & TMSRPT08_PATTERNNODES)
  {
    numNodes = 0;
    rcode2 = btrieve(B_GETFIRST, TMS_SERVICES, &SERVICES, &SERVICESKey0, 0);
    while(rcode2 == 0)
    {
      rcode2 = btrieve(B_GETFIRST, TMS_ROUTES, &ROUTES, &ROUTESKey0, 0);
      while(rcode2 == 0)
      {
        for(nI = 0; nI < 2; nI++)
        {
          if(ROUTES.DIRECTIONSrecordID[nI] == NO_RECORD)
            continue;
          PATTERNSKey2.ROUTESrecordID = ROUTES.recordID;
          PATTERNSKey2.SERVICESrecordID = SERVICES.recordID;
          PATTERNSKey2.directionIndex = nI;
          PATTERNSKey2.PATTERNNAMESrecordID = basePatternRecordID;
          PATTERNSKey2.nodeSequence = NO_RECORD;
          prevPattern = NO_RECORD;
          prevLastNode = NO_RECORD;
          rcode2 = btrieve(B_GETGREATER, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
          while(rcode2 == 0 &&
                PATTERNS.ROUTESrecordID == ROUTES.recordID &&
                PATTERNS.SERVICESrecordID == SERVICES.recordID &&
                PATTERNS.directionIndex == nI)
          {
            if(!(PATTERNS.flags & PATTERNS_FLAG_BUSSTOP))
            {
              if(prevPattern != PATTERNS.PATTERNNAMESrecordID)
              {
                prevPattern = PATTERNS.PATTERNNAMESrecordID;
                for(bFound = FALSE, nJ = 0; nJ < numNodes; nJ++)
                {
                  if(PATTERNS.NODESrecordID == recordIDs[nJ])
                  {
                    bFound = TRUE;
                    break;
                  }
                }
                if(!bFound)
                  recordIDs[numNodes++] = PATTERNS.NODESrecordID;
                if(prevLastNode != NO_RECORD)
                {
                  for(bFound = FALSE, nJ = 0; nJ < numNodes; nJ++)
                  {
                    if(prevLastNode == recordIDs[nJ])
                    {
                      bFound = TRUE;
                      break;
                    }
                  }
                  if(!bFound)
                    recordIDs[numNodes++] = prevLastNode;
                }
              }
              prevLastNode = PATTERNS.NODESrecordID;
            }
            rcode2 = btrieve(B_GETNEXT, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
          }
          if(prevLastNode != NO_RECORD)
          {
            for(bFound = FALSE, nJ = 0; nJ < numNodes; nJ++)
            {
              if(prevLastNode == recordIDs[nJ])
              {
                bFound = TRUE;
                break;
              }
            }
            if(!bFound)
              recordIDs[numNodes++] = prevLastNode;
          }
        }
        rcode2 = btrieve(B_GETNEXT, TMS_ROUTES, &ROUTES, &ROUTESKey0, 0);
      }
      rcode2 = btrieve(B_GETNEXT, TMS_SERVICES, &SERVICES, &SERVICESKey0, 0);
    }
    qsort((void *)recordIDs, numNodes, sizeof(long), sort_nodes);
  }
//
//  Just relief points
//
  else
  {
    rcode2 = btrieve(B_GETFIRST, TMS_NODES, &NODES, &NODESKey2, 2);
    numNodes = 0;
    while(rcode2 == 0)
    {
      if(!(NODES.flags & NODES_FLAG_STOP))
      {
        strncpy(tempString, NODES.reliefLabels, NODES_RELIEFLABELS_LENGTH);
        trim(tempString, NODES_RELIEFLABELS_LENGTH);
        if(strcmp(tempString, "") != 0 || (NODES.flags & NODES_FLAG_GARAGE))
        {
          recordIDs[numNodes++] = NODES.recordID;
          if(numNodes >= TMSRPT08_MAXNODES)
            break;
        }
      }
      rcode2 = btrieve(B_GETNEXT, TMS_NODES, &NODES, &NODESKey2, 2);
    }
  }
//
//  Generate the output file
//
  seqNum = 0;
  LoadString(hInst, TEXT_117, tempString, TEMPSTRING_LENGTH);
  StatusBarStart(hWndMain, tempString);
  for(nI = 0; nI < numNodes - 1; nI++)
  {
    NODESKey0.recordID = recordIDs[nI];
    btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
    strncpy(fromNodeName, NODES.abbrName, NODES_ABBRNAME_LENGTH);
    trim(fromNodeName, NODES_ABBRNAME_LENGTH);
    strcpy(SBFromNodeName, "");
    for(nK = 0; nK < (int)strlen(fromNodeName); nK++)
    {
      if(fromNodeName[nK] == '&')
        strcat(SBFromNodeName, "&&");
      else
      {
        szarString[0] = fromNodeName[nK];
        szarString[1] = '\0';
        strcat(SBFromNodeName, szarString);
      }
    }
    longitude[0] = NODES.longitude;
    latitude[0] = NODES.latitude;
    StatusBar((long)nI, (long)numNodes);
    if(StatusBarAbort())
      goto abort;
    for(nJ = nI + 1; nJ < numNodes; nJ++)
    {
      NODESKey0.recordID = recordIDs[nJ];
      btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
      strncpy(toNodeName, NODES.abbrName, NODES_ABBRNAME_LENGTH);
      trim(toNodeName, NODES_ABBRNAME_LENGTH);
      longitude[1] = NODES.longitude;
      latitude[1] = NODES.latitude;
      if(StatusBarAbort())
        goto abort;
      strcpy(SBToNodeName, "");
      for(nK = 0; nK < (int)strlen(toNodeName); nK++)
      {
        if(toNodeName[nK] == '&')
          strcat(SBToNodeName, "&&");
        else
        {
          szarString[0] = toNodeName[nK];
          szarString[1] = '\0';
          strcat(SBToNodeName, szarString);
        }
      }
      strcpy(szarString, "From: ");
      strcat(szarString, SBFromNodeName);
      strcat(szarString, " To: ");
      strcat(szarString, SBToNodeName);
      StatusBarText(szarString);
      bFound = FALSE;
      for(nK = 0; nK < 2; nK++)
      {
        if(StatusBarAbort())
          goto abort;
        fromNode = nK == 0 ? recordIDs[nI] : recordIDs[nJ];
        toNode = nK == 0 ? recordIDs[nJ] : recordIDs[nI];
        CONNECTIONSKey1.fromNODESrecordID = fromNode;
        CONNECTIONSKey1.toNODESrecordID = toNode;
        CONNECTIONSKey1.fromTimeOfDay = NO_TIME;
        rcode2 = btrieve(B_GETGREATEROREQUAL, TMS_CONNECTIONS, &CONNECTIONS, &CONNECTIONSKey1, 1);
        while(rcode2 == 0 &&
              CONNECTIONS.fromNODESrecordID == fromNode &&
              CONNECTIONS.toNODESrecordID == toNode)
        {
          bFound = TRUE;
          if(!(RPTFLAGS.flags & TMSRPT08_INCLUDEEXISTING))
            break;
//
//  Set up the output string
//
          if(((RPTFLAGS.flags & TMSRPT08_INCLUDERT) &&
                      (CONNECTIONS.flags & CONNECTIONS_FLAG_RUNNINGTIME)) ||
                ((RPTFLAGS.flags & TMSRPT08_INCLUDETT) &&
                      (CONNECTIONS.flags & CONNECTIONS_FLAG_TRAVELTIME)) ||
                ((RPTFLAGS.flags & TMSRPT08_INCLUDEDT) &&
                      (CONNECTIONS.flags & CONNECTIONS_FLAG_DEADHEADTIME)) ||
                ((RPTFLAGS.flags & TMSRPT08_INCLUDEEQ) &&
                      (CONNECTIONS.flags & CONNECTIONS_FLAG_EQUIVALENT)))
          {
            strcpy(tempString, "");
            for(nL = 0; nL < NUMCOLS; nL++)
            {
              switch(nL)
              {
//
//  From Node
//
                case 0:
                  strcpy(szarString, nK == 0 ? fromNodeName : toNodeName);
                  break;
//
//  To Node
//
                case 1:
                  strcpy(szarString, nK == 0 ? toNodeName : fromNodeName);
                  break;
//
//  From Route
//
                case 2:
                  if((ROUTESKey0.recordID = CONNECTIONS.fromROUTESrecordID) == NO_RECORD)
                    strcpy(szarString, "");
                  else
                  {
                    btrieve(B_GETEQUAL, TMS_ROUTES, &ROUTES, &ROUTESKey0, 0);
                    strncpy(szarString, ROUTES.number, ROUTES_NUMBER_LENGTH);
                    szarString[ROUTES_NUMBER_LENGTH] = '\0';
                  }
                  break;
//
//  From Service
//
                case 3:
                  if((SERVICESKey0.recordID = CONNECTIONS.fromSERVICESrecordID) == NO_RECORD)
                    strcpy(szarString, "");
                  else
                  {
                    btrieve(B_GETEQUAL, TMS_SERVICES, &SERVICES, &SERVICESKey0, 0);
                    strncpy(szarString, SERVICES.name, sizeof(SERVICES.name));
                    szarString[sizeof(SERVICES.name)] = '\0';
                  }
                  break;
//
//  From Pattern
//
                case 4:
                  if((PATTERNNAMESKey0.recordID = CONNECTIONS.fromPATTERNNAMESrecordID) == NO_RECORD)
                    strcpy(szarString, "");
                  else
                  {
                    btrieve(B_GETEQUAL, TMS_PATTERNNAMES, &PATTERNNAMES, &PATTERNNAMESKey0, 0);
                    strncpy(szarString, PATTERNNAMES.name, sizeof(PATTERNNAMES.name));
                    szarString[sizeof(PATTERNNAMES.name)] = '\0';
                  }
                  break;
//
//  To Route
//
                case 5:
                  if((ROUTESKey0.recordID = CONNECTIONS.toROUTESrecordID) == NO_RECORD)
                    strcpy(szarString, "");
                  else
                  {
                    btrieve(B_GETEQUAL, TMS_ROUTES, &ROUTES, &ROUTESKey0, 0);
                    strncpy(szarString, ROUTES.number, ROUTES_NUMBER_LENGTH);
                    szarString[ROUTES_NUMBER_LENGTH] = '\0';
                  }
                  break;
//
//  To Service
//
                case 6:
                  if((SERVICESKey0.recordID = CONNECTIONS.toSERVICESrecordID) == NO_RECORD)
                    strcpy(szarString, "");
                  else
                  {
                    btrieve(B_GETEQUAL, TMS_SERVICES, &SERVICES, &SERVICESKey0, 0);
                    strncpy(szarString, SERVICES.name, sizeof(SERVICES.name));
                    szarString[sizeof(SERVICES.name)] = '\0';
                  }
                  break;
//
//  To Pattern
//
                case 7:
                  if((PATTERNNAMESKey0.recordID = CONNECTIONS.toPATTERNNAMESrecordID) == NO_RECORD)
                    strcpy(szarString, "");
                  else
                  {
                    btrieve(B_GETEQUAL, TMS_PATTERNNAMES, &PATTERNNAMES, &PATTERNNAMESKey0, 0);
                    strncpy(szarString, PATTERNNAMES.name, sizeof(PATTERNNAMES.name));
                    szarString[sizeof(PATTERNNAMES.name)] = '\0';
                  }
                  break;
//
//  Time
//
                case 8:
                  sprintf(szarString, "%d", CONNECTIONS.connectionTime / 60);
                  break;
//
//  Btwn
//
                case 9:
                  strcpy(szarString, Tchar(CONNECTIONS.fromTimeOfDay));
                  break;
//
//  And
//
                case 10:
                  strcpy(szarString, Tchar(CONNECTIONS.toTimeOfDay));
                  break;
//
//  2W?
//
                case 11:
                  strcpy(szarString, CONNECTIONS.flags & CONNECTIONS_FLAG_TWOWAY ? CONST_YES : CONST_NO);
                  break;
//
//  RT?
//
                case 12:
                  strcpy(szarString, CONNECTIONS.flags & CONNECTIONS_FLAG_RUNNINGTIME ? CONST_YES : CONST_NO);
                  break;
//
//  TT?
//
                case 13:
                  strcpy(szarString, CONNECTIONS.flags & CONNECTIONS_FLAG_TRAVELTIME ? CONST_YES : CONST_NO);
                  break;
//
//  DT?
//
                case 14:
                  strcpy(szarString, CONNECTIONS.flags & CONNECTIONS_FLAG_DEADHEADTIME ? CONST_YES : CONST_NO);
                  break;
//
//  Distance type
//
                case 15:
                  if(CONNECTIONS.distance == NO_RECORD || CONNECTIONS.distance == 0.0)
                    strcpy(szarString, "Map ");
                  else
                    strcpy(szarString, "User");
                  break;
//
//  Distance
//
                case 16:
                  if(CONNECTIONS.distance == NO_RECORD || CONNECTIONS.distance == 0.0)
                  {
                    distance = (float)GreatCircleDistance(longitude[0], latitude[0],
                          longitude[1], latitude[1]);
                  }
                  else
                  {
                    distance = CONNECTIONS.distance;
                  }
                  sprintf(szarString, "%7.2f", distance);
                  break;
//
//  Sequence
//
                case 17:
                  sprintf(szarString, "%d", seqNum++);
                  break;
              }  // End of switch
              strcat(tempString, szarString);
              strcat(tempString, nL == NUMCOLS - 1 ? "\r\n" : "\t");
            }  // nL
            _lwrite(hfOutputFile, tempString, strlen(tempString));
          }  // if connection type == requested type to display
          rcode2 = btrieve(B_GETNEXT, TMS_CONNECTIONS, &CONNECTIONS, &CONNECTIONSKey1, 1);
        }  // while
      }  // nK
      if(RPTFLAGS.flags & TMSRPT08_INCLUDEPOTENTIAL)
      {
        strcpy(tempString, fromNodeName);
        strcat(tempString, "\t");
        strcat(tempString, toNodeName);
        strcat(tempString, "\r\n");
        _lwrite(hfOutputFile, tempString, strlen(tempString));
      }
    }  // nJ
  }  // nI
  abort:
    _lclose(hfOutputFile);

  StatusBarEnd();
//
//  All done
//
  return(TRUE);
}

BOOL CALLBACK RPTCONNMsgProc(HWND hWndDlg, UINT Message, WPARAM wParam, LPARAM lParam)
{
  static RPTFLAGSDef *pRPTFLAGS;
  static HANDLE hCtlINCLUDEEXISTING;
  static HANDLE hCtlCHOOSEEXISTING;
  static HANDLE hCtlEVERYTHING;
  static HANDLE hCtlJUST;
  static HANDLE hCtlRT;
  static HANDLE hCtlDT;
  static HANDLE hCtlTT;
  static HANDLE hCtlEQ;
  static HANDLE hCtlINCLUDEPOTENTIAL;
  static HANDLE hCtlCHOOSEPOTENTIAL;
  static HANDLE hCtlALLNODES;
  static HANDLE hCtlPATTERNNODES;
  static HANDLE hCtlRELIEFPOINTS;
  BOOL   bEnable;
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
      hCtlINCLUDEEXISTING = GetDlgItem(hWndDlg, RPTCONN_INCLUDEEXISTING);
      hCtlCHOOSEEXISTING = GetDlgItem(hWndDlg, RPTCONN_CHOOSEEXISTING);
      hCtlEVERYTHING = GetDlgItem(hWndDlg, RPTCONN_EVERYTHING);
      hCtlJUST = GetDlgItem(hWndDlg, RPTCONN_JUST);
      hCtlRT = GetDlgItem(hWndDlg, RPTCONN_RT);
      hCtlDT = GetDlgItem(hWndDlg, RPTCONN_DT);
      hCtlTT = GetDlgItem(hWndDlg, RPTCONN_TT);
      hCtlEQ = GetDlgItem(hWndDlg, RPTCONN_EQ);
      hCtlINCLUDEPOTENTIAL = GetDlgItem(hWndDlg, RPTCONN_INCLUDEPOTENTIAL);
      hCtlCHOOSEPOTENTIAL = GetDlgItem(hWndDlg, RPTCONN_CHOOSEPOTENTIAL);
      hCtlALLNODES = GetDlgItem(hWndDlg, RPTCONN_ALLNODES);
      hCtlPATTERNNODES = GetDlgItem(hWndDlg, RPTCONN_PATTERNNODES);
      hCtlRELIEFPOINTS = GetDlgItem(hWndDlg, RPTCONN_RELIEFPOINTS);
//
//  Default to include both
//
      SendMessage(hCtlINCLUDEEXISTING, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
      SendMessage(hCtlEVERYTHING, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
      SendMessage(hCtlINCLUDEPOTENTIAL, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
      SendMessage(hCtlALLNODES, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
//
//  Set the dialog title
//
      SendMessage(hWndDlg, WM_GETTEXT, (WPARAM)sizeof(szFormatString), (LONG)(LPSTR)szFormatString);
      sprintf(tempString, szFormatString, TMSRPT[pRPTFLAGS->nReportNumber].szReportName);
      SendMessage(hWndDlg, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)tempString);
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
        case RPTCONN_INCLUDEEXISTING:
          bEnable = (BOOL)SendMessage(hCtlINCLUDEEXISTING, BM_GETCHECK, (WPARAM)0, (LPARAM)0);
          EnableWindow(hCtlCHOOSEEXISTING, bEnable);
          EnableWindow(hCtlEVERYTHING, bEnable);
          EnableWindow(hCtlJUST, bEnable);
          EnableWindow(hCtlRT, bEnable);
          SendMessage(hCtlRT, BM_SETCHECK, (WPARAM)bEnable, (LPARAM)0);
          EnableWindow(hCtlDT, bEnable);
          SendMessage(hCtlDT, BM_SETCHECK, (WPARAM)bEnable, (LPARAM)0);
          EnableWindow(hCtlTT, bEnable);
          SendMessage(hCtlTT, BM_SETCHECK, (WPARAM)bEnable, (LPARAM)0);
          EnableWindow(hCtlEQ, bEnable);
          SendMessage(hCtlEQ, BM_SETCHECK, (WPARAM)bEnable, (LPARAM)0);
          SendMessage(hCtlEVERYTHING, BM_SETCHECK, (WPARAM)bEnable, (LPARAM)0);
          SendMessage(hCtlJUST, BM_SETCHECK, (WPARAM)FALSE, (LPARAM)0);
          break;

        case RPTCONN_INCLUDEPOTENTIAL:
          bEnable = (BOOL)SendMessage(hCtlINCLUDEPOTENTIAL, BM_GETCHECK, (WPARAM)0, (LPARAM)0);
          EnableWindow(hCtlCHOOSEPOTENTIAL, bEnable);
          EnableWindow(hCtlALLNODES, bEnable);
          EnableWindow(hCtlPATTERNNODES, bEnable);
          SendMessage(hCtlALLNODES, BM_SETCHECK, (WPARAM)bEnable, (LPARAM)0);
          SendMessage(hCtlPATTERNNODES, BM_SETCHECK, (WPARAM)FALSE, (LPARAM)0);
          SendMessage(hCtlRELIEFPOINTS, BM_SETCHECK, (WPARAM)FALSE, (LPARAM)0);
          break;
//
//  IDCANCEL
//
        case IDCANCEL:
          EndDialog(hWndDlg, FALSE);
          break;
//
//  IDOK
//
        case IDOK:
          pRPTFLAGS->flags = 0;
          if(SendMessage(hCtlINCLUDEEXISTING, BM_GETCHECK, (WPARAM)0, (LPARAM)0))
            pRPTFLAGS->flags |= TMSRPT08_INCLUDEEXISTING;
          if(SendMessage(hCtlINCLUDEPOTENTIAL, BM_GETCHECK, (WPARAM)0, (LPARAM)0))
            pRPTFLAGS->flags |= TMSRPT08_INCLUDEPOTENTIAL;
          if(!(pRPTFLAGS->flags & TMSRPT08_INCLUDEEXISTING) &&
                !(pRPTFLAGS->flags & TMSRPT08_INCLUDEPOTENTIAL))
          {
            TMSError(hWndDlg, MB_ICONSTOP, ERROR_071, hCtlINCLUDEEXISTING);
            break;
          }
          if(pRPTFLAGS->flags & TMSRPT08_INCLUDEEXISTING)
          {
            if(SendMessage(hCtlEVERYTHING, BM_GETCHECK, (WPARAM)0, (LPARAM)0))
            {
              pRPTFLAGS->flags |= TMSRPT08_INCLUDERT | TMSRPT08_INCLUDEDT |
                    TMSRPT08_INCLUDETT | TMSRPT08_INCLUDEDT;
            }
            else
            {
              if(SendMessage(hCtlRT, BM_GETCHECK, (WPARAM)0, (LPARAM)0))
                pRPTFLAGS->flags |= TMSRPT08_INCLUDERT;
              if(SendMessage(hCtlDT, BM_GETCHECK, (WPARAM)0, (LPARAM)0))
                pRPTFLAGS->flags |= TMSRPT08_INCLUDEDT;
              if(SendMessage(hCtlTT, BM_GETCHECK, (WPARAM)0, (LPARAM)0))
                pRPTFLAGS->flags |= TMSRPT08_INCLUDETT;
              if(SendMessage(hCtlEQ, BM_GETCHECK, (WPARAM)0, (LPARAM)0))
                pRPTFLAGS->flags |= TMSRPT08_INCLUDEEQ;
            }
          }
          if(pRPTFLAGS->flags & TMSRPT08_INCLUDEPOTENTIAL)
          {
            if(SendMessage(hCtlALLNODES, BM_GETCHECK, (WPARAM)0, (LPARAM)0))
              pRPTFLAGS->flags |= TMSRPT08_ALLSYSTEMNODES;
            else if(SendMessage(hCtlPATTERNNODES, BM_GETCHECK, (WPARAM)0, (LPARAM)0))
              pRPTFLAGS->flags |= TMSRPT08_PATTERNNODES;
            else
              pRPTFLAGS->flags |= TMSRPT08_RELIEFPOINTS;
          }
          EndDialog(hWndDlg, TRUE);
          break;
      }

    default:
      return FALSE;
  }

  return TRUE;
} //  End of RPTCONNMsgProc

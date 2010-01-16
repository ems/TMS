//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
#include "TMSHeader.h"

#ifdef  TABSET
#undef  TABSET
#endif
#define TABSET 7

BOOL CALLBACK UNBLOCKEDTMsgProc(HWND hWndDlg, UINT Message, WPARAM wParam, LPARAM lParam)
{
  GenerateTripDef GTResults;
  static HANDLE hCtlTITLE;
  static HANDLE hCtlTRIPS;
  static int    tabPos[TABSET] = {10, 20, 24, 34, 40, 47, 54};
  WORD   DlgWidthUnits;
  int    adjustedTabPos[TABSET];
  int    rcode2;
  int    nI;
  char   routeNumber[ROUTES_NUMBER_LENGTH + 1];
  char   serviceName[SERVICES_NAME_LENGTH + 1];
  char   directionAbbr[DIRECTIONS_ABBRNAME_LENGTH + 1];
  char   patternName[PATTERNNAMES_NAME_LENGTH + 1];
  char   fromNode[NODES_ABBRNAME_LENGTH + 1];
  char   toNode[NODES_ABBRNAME_LENGTH + 1];
  short  int wmId;

  switch(Message)
  {
//
//  WM_INITDIALOG - Initialize the dialog
//
    case WM_INITDIALOG:
//
//  Set up the handles to the controls
//
      hCtlTITLE = GetDlgItem(hWndDlg, UNBLOCKEDT_TITLE);
      hCtlTRIPS = GetDlgItem(hWndDlg, UNBLOCKEDT_TRIPS);
//
//  Set up the tab stops for ADDBLOCK_LISTBOXTITLE and ADDBLOCK_TRIPS
//
      DlgWidthUnits = LOWORD(GetDialogBaseUnits()) / 4;
      for(nI = 0; nI < TABSET; nI++)
      {
        adjustedTabPos[nI] = (DlgWidthUnits * tabPos[nI] * 2);
      }
      SendMessage(hCtlTITLE, LB_SETTABSTOPS, (WPARAM)TABSET, (LPARAM)adjustedTabPos);
      LoadString(hInst, TEXT_063, tempString, TEMPSTRING_LENGTH);
      SendMessage(hCtlTITLE, LB_ADDSTRING, (WPARAM)0, (LONG)(LPSTR)tempString);
      SendMessage(hCtlTRIPS, LB_SETTABSTOPS, (WPARAM)TABSET, (LPARAM)adjustedTabPos);
//
//  Set up the selected trips
//
//  We're not interested in dropback blocks here, so just go standard
//
      m_bEstablishRUNTIMES = TRUE;
      TRIPSKey2.assignedToNODESrecordID = NO_RECORD;
      TRIPSKey2.RGRPROUTESrecordID = NO_RECORD;
      TRIPSKey2.SGRPSERVICESrecordID = NO_RECORD;
      TRIPSKey2.blockNumber = 0;
      TRIPSKey2.blockSequence = NO_TIME;
      rcode2 = btrieve(B_GETGREATEROREQUAL, TMS_TRIPS, &TRIPS, &TRIPSKey2, 2);
      while(rcode2 == 0 &&
//            TRIPS.standard.assignedToNODESrecordID == NO_RECORD &&
//            TRIPS.standard.RGRPROUTESrecordID == NO_RECORD &&
//            TRIPS.standard.SGRPSERVICESrecordID == NO_RECORD &&
//            TRIPS.standard.blockNumber == 0)
            TRIPS.standard.RGRPROUTESrecordID == NO_RECORD)
      {
        GenerateTrip(TRIPS.ROUTESrecordID, TRIPS.SERVICESrecordID,
              TRIPS.directionIndex, TRIPS.PATTERNNAMESrecordID,
              TRIPS.timeAtMLP, GENERATETRIP_FLAG_DISPLAYERRORS, &GTResults);
//
//  Route
//
        ROUTESKey0.recordID = TRIPS.ROUTESrecordID;
        btrieve(B_GETEQUAL, TMS_ROUTES, &ROUTES, &ROUTESKey0, 0);
        strncpy(routeNumber, ROUTES.number, ROUTES_NUMBER_LENGTH);
        trim(routeNumber, ROUTES_NUMBER_LENGTH);
//
//  Service
//
        SERVICESKey0.recordID = TRIPS.SERVICESrecordID;
        btrieve(B_GETEQUAL, TMS_SERVICES, &SERVICES, &SERVICESKey0, 0);
        strncpy(serviceName, SERVICES.name, SERVICES_NAME_LENGTH);
        trim(serviceName, SERVICES_NAME_LENGTH);
//
//  Direction
//
        DIRECTIONSKey0.recordID = ROUTES.DIRECTIONSrecordID[TRIPS.directionIndex];
        btrieve(B_GETEQUAL, TMS_DIRECTIONS, &DIRECTIONS, &DIRECTIONSKey0, 0);
        strncpy(directionAbbr, DIRECTIONS.abbrName, DIRECTIONS_ABBRNAME_LENGTH);
        trim(directionAbbr, DIRECTIONS_ABBRNAME_LENGTH);
//
//  Pattern
//
        PATTERNNAMESKey0.recordID = TRIPS.PATTERNNAMESrecordID;
        btrieve(B_GETEQUAL, TMS_PATTERNNAMES, &PATTERNNAMES, &PATTERNNAMESKey0, 0);
        strncpy(patternName, PATTERNNAMES.name, PATTERNNAMES_NAME_LENGTH);
        trim(patternName, PATTERNNAMES_NAME_LENGTH);
//
//  From Node
//
        NODESKey0.recordID = GTResults.firstNODESrecordID;
        btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
        strncpy(fromNode, NODES.abbrName, NODES_ABBRNAME_LENGTH);
        trim(fromNode, NODES_ABBRNAME_LENGTH);
//
//  To Node
//
        if(GTResults.firstNODESrecordID == GTResults.lastNODESrecordID)
          strcpy(toNode, fromNode);
        else
        {
          NODESKey0.recordID = GTResults.lastNODESrecordID;
          btrieve(B_GETEQUAL, TMS_NODES, &NODES, &NODESKey0, 0);
          strncpy(toNode, NODES.abbrName, NODES_ABBRNAME_LENGTH);
          trim(toNode, NODES_ABBRNAME_LENGTH);
        }
//
//  Display it
//
        strcpy(tempString, routeNumber);
        for(nI = 0; nI < TABSET; nI++)
        {
          tempString[tabPos[nI]] = '\0';
          strcat(tempString, "\t");
          switch(nI)
          {
            case 0:
              strcat(tempString, serviceName);
              break;
            case 1:
              strcat(tempString, directionAbbr);
              break;
            case 2:
              strcat(tempString, patternName);
              break;
            case 3:
              strcat(tempString, fromNode);
              break;
            case 4:
              strcat(tempString, Tchar(GTResults.firstNodeTime));
              break;
            case 5:
              strcat(tempString, Tchar(GTResults.lastNodeTime));
              break;
            case 6:
              strcat(tempString, toNode);
              break;
          }
        }
        SendMessage(hCtlTRIPS, LB_ADDSTRING, (WPARAM)0, (LONG)(LPSTR)tempString);
        rcode2 = btrieve(B_GETNEXT, TMS_TRIPS, &TRIPS, &TRIPSKey2, 2);
      }
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
        case IDOK:
          EndDialog(hWndDlg, TRUE);
          break;

        case IDCANCEL:
          EndDialog(hWndDlg, FALSE);
          break;
//
//  IDHELP
//
        case IDHELP:
          WinHelp(hWndDlg, szarHelpFile, HELP_CONTEXT, The_Runs_Table);
          break;
      }
      break;

    default:
      return FALSE;
  }
  return TRUE;
}

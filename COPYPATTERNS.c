//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
#include "TMSHeader.h"

BOOL CALLBACK COPYPATTERNSMsgProc(HWND hWndDlg, UINT Message, WPARAM wParam, LPARAM lParam)
{
  static  PDISPLAYINFO pDI;
  static  HANDLE hCtlROUTE;
  static  HANDLE hCtlSERVICE;
  static  HANDLE hCtlOUTBOUND;
  static  HANDLE hCtlINBOUND;
  PATTERNSDef   PATTERNREC;
  HCURSOR saveCursor;
  char    direction[DIRECTIONS_ABBRNAME_LENGTH + 1];
  long    fromRouteRecordID;
  long    fromServiceRecordID;
  long    fromDirectionIndex;
  long    insertRecordID;
  long    previousPatternnameRecordID;
  int     nI;
  int     numPatterns;
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
      hCtlROUTE = GetDlgItem(hWndDlg, COPYPATTERNS_ROUTE);
      hCtlSERVICE = GetDlgItem(hWndDlg, COPYPATTERNS_SERVICE);
      hCtlOUTBOUND = GetDlgItem(hWndDlg, COPYPATTERNS_OUTBOUND);
      hCtlINBOUND = GetDlgItem(hWndDlg, COPYPATTERNS_INBOUND);
//
//  Set up the "destination"
//
      flags = PLACEMENT_ROUTE | PLACEMENT_SERVICE | PLACEMENT_DIRECTION;
      DisplayPlacement(hWndDlg, COPYPATTERNS_DESTINATION, pDI, flags);
//
//  Set up the route, service, and direciton
//
      if(SetUpRouteList(hWndDlg, COPYPATTERNS_ROUTE, pDI->fileInfo.routeRecordID) == 0)
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
      if(SetUpServiceList(hWndDlg, COPYPATTERNS_SERVICE, NO_RECORD) == 0)
      {
        SendMessage(hWndDlg, WM_COMMAND, MAKEWPARAM(IDCANCEL, 0), (LPARAM)0);
        break;
      }
      SendMessage(hCtlSERVICE, CB_SETCURSEL, (WPARAM)0, (LPARAM)0);
      if(pDI->fileInfo.directionIndex == 0)
        SendMessage(hCtlOUTBOUND, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
      else
        SendMessage(hCtlINBOUND, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
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
//  COPYPATTERNS_ROUTE
//
        case COPYPATTERNS_ROUTE:
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
//  Delete the existing patterns on this route/ser/dir
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
            rcode2 = btrieve(B_GETNEXT, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
          }
          
//
//  Establish the new patterns recordID
//
          rcode2 = btrieve(B_GETLAST, TMS_PATTERNS, &PATTERNS, &PATTERNSKey0, 0);
          insertRecordID = AssignRecID(rcode2, PATTERNS.recordID);
//
//  Start the copy
//
          saveCursor = SetCursor(hCursorWait);
          previousPatternnameRecordID = NO_RECORD;
          numPatterns = 0;
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
            if(PATTERNS.PATTERNNAMESrecordID != previousPatternnameRecordID)
            {
              previousPatternnameRecordID = PATTERNS.PATTERNNAMESrecordID;
              numPatterns++;
            }
            btrieve(B_GETPOSITION, TMS_PATTERNS, &PATTERNREC, &PATTERNSKey2, 2);
            PATTERNS.recordID = insertRecordID++;
            PATTERNS.ROUTESrecordID = pDI->fileInfo.routeRecordID;
            PATTERNS.SERVICESrecordID = pDI->fileInfo.serviceRecordID;
            PATTERNS.directionIndex = pDI->fileInfo.directionIndex;
            btrieve(B_INSERT, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
            btrieve(B_GETDIRECT, TMS_PATTERNS, &PATTERNREC, &PATTERNSKey2, 2);
            rcode2 = btrieve(B_GETNEXT, TMS_PATTERNS, &PATTERNS, &PATTERNSKey2, 2);
          }
          SetCursor(saveCursor);
//
//  Let him know
//
          MessageBeep(MB_ICONINFORMATION);
          LoadString(hInst, TEXT_094, szarString, sizeof(szarString));
          sprintf(tempString, szarString, numPatterns);
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

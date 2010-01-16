//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
#include "TMSHeader.h"

BOOL CALLBACK ASSIGNTRIPMsgProc(HWND hWndDlg, UINT Message, WPARAM wParam, LPARAM lParam)
{
  static  PDISPLAYINFO pDI;
  static  HANDLE hCtlCLEARALL;
  static  HANDLE hCtlALL;
  HCURSOR saveCursor;
  int     tripNumber;
  int     rcode2;
  short int wmId;
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
      hCtlCLEARALL = GetDlgItem(hWndDlg, ASSIGNTRIP_CLEARALL);
      hCtlALL = GetDlgItem(hWndDlg, ASSIGNTRIP_ALL);
//
//  Set up the "destination"
//
     flags = PLACEMENT_ROUTE | PLACEMENT_SERVICE;
     DisplayPlacement(hWndDlg, ASSIGNTRIP_DESTINATION, pDI, flags);
//
//  Default to clear all
//
     SendMessage(hCtlCLEARALL, BM_SETCHECK, (WPARAM)TRUE, (LPARAM)0);
     break; //  End of WM_INITDLG
//
//  WM_CLOSE
//
    case WM_CLOSE:
      SendMessage(hWndDlg, WM_COMMAND, MAKEWPARAM(IDCANCEL, 0), (LPARAM)0);
      break; //  End of WM_CLOSE
//
//  WM_COMMAND
//
    case WM_COMMAND:
      wmId = LOWORD(wParam);
      switch(wmId)
      {
//
//  IDOK
//
        case IDOK:
          saveCursor = SetCursor(hCursorWait);
          if(SendMessage(hCtlCLEARALL, BM_GETCHECK, (WPARAM)0, (LPARAM)0))
          {
            tripNumber = NO_RECORD;
            rcode2 = btrieve(B_GETFIRST, TMS_TRIPS, &TRIPS, &TRIPSKey0, 0);
            while(rcode2 == 0)
            {
              TRIPS.tripNumber = tripNumber;
              btrieve(B_UPDATE, TMS_TRIPS, &TRIPS, &TRIPSKey0, 0);
              rcode2 = btrieve(B_GETNEXT, TMS_TRIPS, &TRIPS, &TRIPSKey0, 0);
            }
          }
          else if(SendMessage(hCtlALL, BM_GETCHECK, (WPARAM)0, (LPARAM)0))
          {
            tripNumber = 1;
            rcode2 = btrieve(B_GETFIRST, TMS_TRIPS, &TRIPS, &TRIPSKey0, 0);
            while(rcode2 == 0)
            {
              TRIPS.tripNumber = tripNumber++;
              btrieve(B_UPDATE, TMS_TRIPS, &TRIPS, &TRIPSKey0, 0);
              rcode2 = btrieve(B_GETNEXT, TMS_TRIPS, &TRIPS, &TRIPSKey0, 0);
            }
          }
          else
          {
            rcode2 = btrieve(B_GETFIRST, TMS_TRIPS, &TRIPS, &TRIPSKey0, 0);
            tripNumber = NO_RECORD;
            while(rcode2 == 0)
            {
              if(TRIPS.tripNumber > tripNumber)
                tripNumber = TRIPS.tripNumber;
              rcode2 = btrieve(B_GETNEXT, TMS_TRIPS, &TRIPS, &TRIPSKey0, 0);
            }
            tripNumber++;
            TRIPSKey1.ROUTESrecordID = pDI->fileInfo.routeRecordID;
            TRIPSKey1.SERVICESrecordID = pDI->fileInfo.serviceRecordID;
            TRIPSKey1.directionIndex = NO_RECORD;
            TRIPSKey1.tripSequence = NO_RECORD;
            rcode2 = btrieve(B_GETGREATEROREQUAL, TMS_TRIPS, &TRIPS, &TRIPSKey1, 1);
            while(rcode2 == 0 &&
                  TRIPS.ROUTESrecordID == pDI->fileInfo.routeRecordID &&
                  TRIPS.SERVICESrecordID == pDI->fileInfo.serviceRecordID)
            {
              TRIPS.tripNumber = tripNumber++;
              btrieve(B_UPDATE, TMS_TRIPS, &TRIPS, &TRIPSKey1, 1);
              rcode2 = btrieve(B_GETNEXT, TMS_TRIPS, &TRIPS, &TRIPSKey1, 1);
            }
          }
          SetCursor(saveCursor);
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
          WinHelp(hWndDlg, szarHelpFile, HELP_CONTEXT, Assigning_Trip_Numbers);
          break;
      }
      break;    //  End of WM_COMMAND

    default:
      return FALSE;
  }
  return TRUE;
}
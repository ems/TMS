//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
#include "TMSHeader.h"

BOOL CALLBACK CHANGELAYOVERMsgProc(HWND hWndDlg, UINT Message, WPARAM wParam, LPARAM lParam)
{
  typedef struct PASSEDDATAStruct
  {
    long ROUTESrecordID;
    long SERVICESrecordID;
    long directionIndex;
    long *pRecordIDs;
    int  numRecords;
  } PASSEDDATADef;
  static  PASSEDDATADef *pPassedData;
  static  HANDLE hCtlMINIMUM;
  static  HANDLE hCtlMAXIMUM;
  HCURSOR hSaveCursor;
  long  TripLayoverMin;
  long  TripLayoverMax;
  int   nI;
  int   rcode2;
  short int wmId;

  switch(Message)
  {
//
//  WM_INITDIALOG - Initialize the dialog
//
    case WM_INITDIALOG:
      pPassedData = (PASSEDDATADef *)lParam;
      if(pPassedData == NULL)
      {
        SendMessage(hWndDlg, WM_COMMAND, MAKEWPARAM(IDCANCEL, 0), (LPARAM)0);
        break;
      }
      hCtlMINIMUM = GetDlgItem(hWndDlg, TRIPLAYOVER_MINIMUM);
      hCtlMAXIMUM = GetDlgItem(hWndDlg, TRIPLAYOVER_MAXIMUM);
      SendMessage(hCtlMINIMUM, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)"");
      SendMessage(hCtlMAXIMUM, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)"");
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
        case IDOK:
          SendMessage(hCtlMINIMUM, WM_GETTEXT, (WPARAM)TEMPSTRING_LENGTH, (LONG)(LPSTR)tempString);
          if(strcmp(tempString, "") == 0)
            TripLayoverMin = NO_TIME;
          else
          {
            TripLayoverMin = atol(tempString) * 60;
            if(TripLayoverMin < 0)
            {
              TMSError(hWndDlg, MB_ICONSTOP, ERROR_033, hCtlMINIMUM);
              break;
            }
          }
          SendMessage(hCtlMAXIMUM, WM_GETTEXT, (WPARAM)TEMPSTRING_LENGTH, (LONG)(LPSTR)tempString);
          if(strcmp(tempString, "") == 0)
            TripLayoverMax = NO_TIME;
          else
          {
            TripLayoverMax = atol(tempString) * 60;
            if(TripLayoverMax < 0)
            {
              TMSError(hWndDlg, MB_ICONSTOP, ERROR_034, hCtlMAXIMUM);
              break;
            }
          }
          hSaveCursor = SetCursor(hCursorWait);
          for(nI = 0; nI < pPassedData->numRecords; nI++)
          {
            TRIPSKey0.recordID = pPassedData->pRecordIDs[nI];
            rcode2 = btrieve(B_GETEQUAL, TMS_TRIPS, &TRIPS, &TRIPSKey0, 0);
            if(rcode2 == 0)
            {
              TRIPS.standard.layoverMin = TripLayoverMin;
              TRIPS.standard.layoverMax = TripLayoverMax;
              TRIPS.dropback.layoverMin = TripLayoverMin;
              TRIPS.dropback.layoverMax = TripLayoverMax;
              btrieve(B_UPDATE, TMS_TRIPS, &TRIPS, &TRIPSKey0, 0);
            }
          }
          SetCursor(hSaveCursor);
          EndDialog(hWndDlg, TRUE);
          break;

        case IDCANCEL:
          EndDialog(hWndDlg, FALSE);
          break;
//
//  IDHELP
//
        case IDHELP:
          WinHelp(hWndDlg, szarHelpFile, HELP_CONTEXT, Multiple_Modifications_on_the_Trips_Table);
          break;
      }
      break;    //  End of WM_COMMAND

    default:
      return FALSE;
  }
  return TRUE;
} //  End of CHANGELAYOVERMsgProc

//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
#include "TMSHeader.h"

BOOL CALLBACK CHANGETRIPSHIFTMsgProc(HWND hWndDlg, UINT Message, WPARAM wParam, LPARAM lParam)
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
  static  HANDLE hCtlVALUE;
  HCURSOR hSaveCursor;
  long  TripShiftValue;
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
      hCtlVALUE = GetDlgItem(hWndDlg, TRIPSHIFT_VALUE);
      SendMessage(hCtlVALUE, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)"");
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
          SendMessage(hCtlVALUE, WM_GETTEXT, (WPARAM)TEMPSTRING_LENGTH, (LONG)(LPSTR)tempString);
          if(strcmp(tempString, "") != 0)
          {
            TripShiftValue = atol(tempString) * 60;
            hSaveCursor = SetCursor(hCursorWait);
            for(nI = 0; nI < pPassedData->numRecords; nI++)
            {
              TRIPSKey0.recordID = pPassedData->pRecordIDs[nI];
              rcode2 = btrieve(B_GETEQUAL, TMS_TRIPS, &TRIPS, &TRIPSKey0, 0);
              if(rcode2 == 0)
              {
                TRIPS.timeAtMLP += TripShiftValue;
                TRIPS.tripSequence += TripShiftValue;
                btrieve(B_UPDATE, TMS_TRIPS, &TRIPS, &TRIPSKey0, 0);
              }
            }
            SetCursor(hSaveCursor);
          }
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
} //  End of CHANGETRIPSHIFTMsgProc

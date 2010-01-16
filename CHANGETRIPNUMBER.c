//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
#include "TMSHeader.h"

BOOL CALLBACK CHANGETRIPNUMBERMsgProc(HWND hWndDlg, UINT Message, WPARAM wParam, LPARAM lParam)
{

  switch(Message)
  {
    static HANDLE hCtlTO;
    BOOL   bFound;
    long   tripNumber;
    int    rcode2;
//
//  WM_INITDIALOG - Initialize the dialog
//
    case WM_INITDIALOG:
//
//  Set up the handle to the control
//
      hCtlTO = GetDlgItem(hWndDlg, RENUMBER_TO);
//
//  Get the trip
//
      TRIPSKey0.recordID = updateRecordID;
      btrieve(B_GETEQUAL, TMS_TRIPS, &TRIPS, &TRIPSKey0, 0);
//
//  Set the title
//
      SendMessage(hWndDlg, WM_GETTEXT, (WPARAM)sizeof(szFormatString), (LONG)(LPSTR)szFormatString);
      sprintf(tempString, szFormatString, "Trip Number", TRIPS.tripNumber);
      SendMessage(hWndDlg, WM_SETTEXT, (WPARAM)0, (LONG)(LPSTR)tempString);
      break;

    case WM_CLOSE:
      SendMessage(hWndDlg, WM_COMMAND, MAKEWPARAM(IDCANCEL, 0), (LPARAM)0);
      break;

    case WM_COMMAND:
      switch(wParam)
      {
        case IDCANCEL:
          EndDialog(hWndDlg, FALSE);
          break;

        case IDHELP:
          WinHelp(hWndDlg, szarHelpFile, HELP_CONTEXT, Technical_Support);
          break;

        case IDOK:
          SendMessage(hCtlTO, WM_GETTEXT, (WPARAM)TEMPSTRING_LENGTH, (LONG)(LPSTR)tempString);
          if(strcmp(tempString, "") == 0)
          {
            TMSError(hWndDlg, MB_ICONSTOP, ERROR_148, hCtlTO);
            break;
          }
          tripNumber = atol(tempString);
          if(tripNumber <= 0)
          {
            TMSError(hWndDlg, MB_ICONSTOP, ERROR_151, hCtlTO);
            break;
          }
          rcode2 = btrieve(B_GETFIRST, TMS_TRIPS, &TRIPS, &TRIPSKey0, 0);
          bFound = FALSE;
          while(rcode2 == 0)
          {
            if(TRIPS.tripNumber == tripNumber)
            {
              TMSError(hWndDlg, MB_ICONSTOP, ERROR_147, hCtlTO);
              bFound = TRUE;
              break;
            }
            rcode2 = btrieve(B_GETNEXT, TMS_TRIPS, &TRIPS, &TRIPSKey0, 0);
          }
          if(!bFound)
          {
            TRIPSKey0.recordID = updateRecordID;
            btrieve(B_GETEQUAL, TMS_TRIPS, &TRIPS, &TRIPSKey0, 0);
            TRIPS.tripNumber = tripNumber;
            btrieve(B_UPDATE, TMS_TRIPS, &TRIPS, &TRIPSKey0, 0);
            EndDialog(hWndDlg, TRUE);
          }
          break;
      }
      break;

    default:
      return FALSE;
  }
  return TRUE;
}

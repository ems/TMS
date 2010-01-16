//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
#include "TMSHeader.h"

BOOL CALLBACK SERVICEDAYSMsgProc(HWND hWndDlg, UINT Message, WPARAM wParam, LPARAM lParam)
{
  static  HANDLE hCtlDAYTITLES[ROSTER_MAX_DAYS];
  static  HANDLE hCtlDAYS[ROSTER_MAX_DAYS];
  HCURSOR saveCursor;
  char    serviceName[SERVICES_NAME_LENGTH + 1];
  int     numServices;
  int     nI;
  int     nJ;
  int     rcode2;
  short int wmId;
  short int wmEvent;

  switch(Message)
  {
//
//  WM_INITDIALOG - Initialize the dialog
//
    case WM_INITDIALOG:
//
//  Set up the handles to the controls
//
      for(nI = 0; nI < ROSTER_MAX_DAYS; nI++)
      {
        hCtlDAYTITLES[nI] = GetDlgItem(hWndDlg, SERVICEDAYS_DAY1TITLE + nI);
        hCtlDAYS[nI] = GetDlgItem(hWndDlg, SERVICEDAYS_DAY1 + nI);
      }
//
//  Set up the services list boxes
//
      rcode2 = btrieve(B_GETFIRST, TMS_SERVICES, &SERVICES, &SERVICESKey1, 1);
      if(rcode2 != 0)
      {
        TMSError(hWndDlg, MB_ICONSTOP, ERROR_007, (HANDLE)NULL);
        SendMessage(hWndDlg, WM_COMMAND, MAKEWPARAM(IDCANCEL, 0), (LPARAM)0);
        break;
      }
//
//  Add "No Service" to all the list boxes
//
      LoadString(hInst, TEXT_061, tempString, TEMPSTRING_LENGTH);
      for(nI = 0; nI < ROSTER_MAX_DAYS; nI++)
      {
        nJ = SendMessage(hCtlDAYS[nI], LB_ADDSTRING, (WPARAM)0, (LONG)(LPSTR)tempString);
        SendMessage(hCtlDAYS[nI], LB_SETITEMDATA, (WPARAM)nJ, (LONG)NO_RECORD);
      }
//
//  And the services...
//
      while(rcode2 == 0)
      {
        strncpy(serviceName, SERVICES.name, SERVICES_NAME_LENGTH);
        trim(serviceName, SERVICES_NAME_LENGTH);
        for(nI = 0; nI < ROSTER_MAX_DAYS; nI++)
        {
          nJ = SendMessage(hCtlDAYS[nI], LB_ADDSTRING, (WPARAM)0, (LONG)(LPSTR)serviceName);
          SendMessage(hCtlDAYS[nI], LB_SETITEMDATA, (WPARAM)nJ, SERVICES.recordID);
        }
        rcode2 = btrieve(B_GETNEXT, TMS_SERVICES, &SERVICES, &SERVICESKey1, 1);
      }
//
//  Set any selections out of ROSTERPARMS
//
      numServices = SendMessage(hCtlDAYS[0], LB_GETCOUNT, (WPARAM)0, (LPARAM)0);
      for(nI = 0; nI < ROSTER_MAX_DAYS; nI++)
      {
        for(nJ = 0; nJ < numServices; nJ++)
          if(SendMessage(hCtlDAYS[nI], LB_GETITEMDATA, (WPARAM)nJ, (LPARAM)0) == ROSTERPARMS.serviceDays[nI])
          {
            SendMessage(hCtlDAYS[nI], LB_SETCURSEL, (WPARAM)nJ, (LPARAM)0);
            break;
          }
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
      wmEvent = HIWORD(wParam);
      switch(wmId)
      {
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
          WinHelp(hWndDlg, szarHelpFile, HELP_CONTEXT, Setting_up_the_Roster_Service_Days);
          break;
//
//  IDOK
//
        case IDOK:
//
//  Go through the list boxes
//
          for(nI = 0; nI < ROSTER_MAX_DAYS; nI++)
          {
            nJ = SendMessage(hCtlDAYS[nI], LB_GETCURSEL, (WPARAM)0, (LPARAM)0);
            ROSTERPARMS.serviceDays[nI] =
                  nJ == LB_ERR ? NO_RECORD : SendMessage(hCtlDAYS[nI], LB_GETITEMDATA, (WPARAM)nJ, (LPARAM)0);
          }
//
//  Update WORKRULE.INI
//
          saveCursor = SetCursor(hCursorWait);
          writeWorkrules(hWndDlg);
          SetCursor(saveCursor);
          EndDialog(hWndDlg, TRUE);
          break;
      }
      break;

    default:
      return FALSE;
  }
  return TRUE;
}

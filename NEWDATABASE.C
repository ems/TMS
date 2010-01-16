//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
#include "TMSHeader.h"

BOOL CALLBACK NEWDATABASEMsgProc(HWND hWndDlg, UINT Message, WPARAM wParam, LPARAM lParam)
{
 static HANDLE hCtlDESCRIPTION;
 static HANDLE hCtlEFFECTIVEDATE;
 static HANDLE hCtlUNTILDATE;
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
     hCtlDESCRIPTION = GetDlgItem(hWndDlg, NEWDATABASE_DESCRIPTION);
     hCtlEFFECTIVEDATE = GetDlgItem(hWndDlg, NEWDATABASE_EFFECTIVEDATE);
     hCtlUNTILDATE = GetDlgItem(hWndDlg, NEWDATABASE_UNTILDATE);
     break;
//
//  WM_CLOSE - Send a CANCEL
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
//
//  IDOK
//
       case IDOK:
//
//  Description - Can't be blank
//
         SendMessage(hCtlDESCRIPTION, WM_GETTEXT,
               (WPARAM)sizeof(szDatabaseDescription), (LONG)(LPSTR)szDatabaseDescription);
         if(strcmp(szDatabaseDescription, "") == 0)
         {
           TMSError(hWndDlg, MB_ICONSTOP, ERROR_103, hCtlDESCRIPTION);
           break;
         }
//
//  Effective date - can't be blank
//
         SendMessage(hCtlEFFECTIVEDATE, WM_GETTEXT,
               (WPARAM)sizeof(szEffectiveDate), (LONG)(LPSTR)szEffectiveDate);
         if(strcmp(szEffectiveDate, "") == 0)
         {
           TMSError(hWndDlg, MB_ICONSTOP, ERROR_109, hCtlEFFECTIVEDATE);
           break;
         }
//
//  Until date - can be blank
//
         SendMessage(hCtlUNTILDATE, WM_GETTEXT,
               (WPARAM)sizeof(szUntilDate), (LONG)(LPSTR)szUntilDate);
//
//  All done
//
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
          WinHelp(hWndDlg, szarHelpFile, HELP_CONTEXT, Creating_a_New_Database);
          break;
     }
     break;    //  End of WM_COMMAND

   default:
     return FALSE;
 }
 return TRUE;
} //  End of NEWDATABASEMsgProc

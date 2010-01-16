//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
#include "TMSHeader.h"

BOOL CALLBACK USERNAMEMsgProc(HWND hWndDlg, UINT Message, WPARAM wParam, LPARAM lParam)
{
  short int wmId;

  switch(Message)
  {
//
//  WM_INITDIALOG - Initialize the dialog
//
    case WM_INITDIALOG:
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
          memset(userName, 0x00, sizeof(userName));
          GetDlgItemText(hWndDlg, USERNAME_NAME, (LPSTR)userName, sizeof(userName));
          if(strcmp(userName, "") == 0)
          {
            TMSError(hWndDlg, MB_ICONSTOP, ERROR_048, GetDlgItem(hWndDlg, USERNAME_NAME));
            break;
          }
          WritePrivateProfileString((LPSTR)"TMS", (LPSTR)"UserName", (LPSTR)userName, (LPSTR)"TMS.INI");
          EndDialog(hWndDlg, TRUE);
          break;

        case IDCANCEL:
          EndDialog(hWndDlg, FALSE);
          break;
//
//  IDHELP
//
        case IDHELP:
          WinHelp(hWndDlg, szarHelpFile, HELP_CONTEXT, Technical_Support);
          break;
      }
      break;    //  End of WM_COMMAND

    default:
      return FALSE;
  }
  return TRUE;
} //  End of USERNAMEMsgProc

//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
#include "TMSHeader.h"

BOOL CALLBACK UNDOROSTERMsgProc(HWND hWndDlg, UINT Message, WPARAM wParam, LPARAM lParam)
{
  static ROSTERDAYSUNDOSWAPDef *pUS;
  static HANDLE hCtlLIST;
  int    nI;
  short  int wmId;

  switch(Message)
  {
//
//  WM_INITDIALOG - Initialize the dialog
//
    case WM_INITDIALOG:
      pUS = (ROSTERDAYSUNDOSWAPDef *)lParam;
      if(pUS == NULL)
      {
        SendMessage(hWndDlg, WM_COMMAND, MAKEWPARAM(IDCANCEL, 0), (LPARAM)0);
        break;
      }
//
//  Set up the handle to the control
//
      hCtlLIST = GetDlgItem(hWndDlg, UNDOROSTER_LIST);
//
//  Get the days of the week from the resource file.
//
//  I know it's dangerous to set this up like this.  So sue me.
//
      for(nI = TEXT_009; nI <= TEXT_015; nI++)
      {
        LoadString(hInst, nI, tempString, TEMPSTRING_LENGTH);
        SendMessage(hCtlLIST, LB_ADDSTRING, (WPARAM)0, (LONG)(LPSTR)tempString);

      }
//
//  Set the appropriate title
//
      if(pUS->bSwap)
      {
        LoadString(hInst, TEXT_341, tempString, TEMPSTRING_LENGTH);
      }
      else
      {
        LoadString(hInst, TEXT_340, tempString, TEMPSTRING_LENGTH);
      }
      SetWindowText(hWndDlg, tempString);
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
        case IDCANCEL:
          EndDialog(hWndDlg, FALSE);
          break;
//
//  IDHELP
//
        case IDHELP:
          WinHelp(hWndDlg, szarHelpFile, HELP_CONTEXT, Undo_highlighted_rows);
          break;

        case IDOK:
          if((nI = SendMessage(hCtlLIST, LB_GETSELCOUNT, (WPARAM)0, (LPARAM)0)) == 0)
          {
            TMSError(hWndDlg, MB_ICONSTOP, ERROR_165, hCtlLIST);
            break;
          }
          for(nI = 0; nI < ROSTER_MAX_DAYS; nI++)
          {
            pUS->bDays[nI] = SendMessage(hCtlLIST, LB_GETSEL, (WPARAM)nI, (LPARAM)0);
          }
          EndDialog(hWndDlg, TRUE);
          break;
      }
      break;

    default:
      return FALSE;
  }
  return TRUE;
}
